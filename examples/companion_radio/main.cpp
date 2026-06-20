#include <Arduino.h>   // needed for PlatformIO
#include <Mesh.h>
#include "MyMesh.h"
#include <WiFi.h>      // Expliciet nodig voor de status-checks

// Believe it or not, this std C function is busted on some platforms!
static uint32_t _atoi(const char* sp) {
  uint32_t n = 0;
  while (*sp && *sp >= '0' && *sp <= '9') {
    n *= 10;
    n += (*sp++ - '0');
  }
  return n;
}

#if defined(NRF52_PLATFORM) || defined(STM32_PLATFORM)
  #include <InternalFileSystem.h>
  #if defined(QSPIFLASH)
    #include <CustomLFS_QSPIFlash.h>
    DataStore store(InternalFS, QSPIFlash, rtc_clock);
  #else
  #if defined(EXTRAFS)
    #include <CustomLFS.h>
    CustomLFS ExtraFS(0xD4000, 0x19000, 128);
    DataStore store(InternalFS, ExtraFS, rtc_clock);
  #else
    DataStore store(InternalFS, rtc_clock);
  #endif
  #endif
#elif defined(RP2040_PLATFORM)
  #include <LittleFS.h>
  DataStore store(LittleFS, rtc_clock);
#elif defined(ESP32)
  #include <SPIFFS.h>
  DataStore store(SPIFFS, rtc_clock);
#endif

/* DYNAMISCHE INTERFACES VOOR DE ESP32 SWITCH */
#ifdef ESP32
  #include <helpers/esp32/SerialWifiInterface.h>
  #include <helpers/esp32/SerialBLEInterface.h>
  
  SerialWifiInterface wifi_interface;
  SerialBLEInterface  ble_interface;
  
  // We mappen de originele naam hard op de wifi_interface voor compiler-compatibiliteit
  #define serial_interface wifi_interface

  #ifndef TCP_PORT
    #define TCP_PORT 5000
  #endif

  // Statemachine variabelen
  enum MeshMode { MODE_WIFI_CONNECTING, MODE_WIFI_ACTIVE, MODE_BLE_ACTIVE };
  MeshMode current_mode = MODE_WIFI_CONNECTING;
  
  unsigned long wifi_timer = 0;
  unsigned long last_wifi_check = 0;
  const unsigned long WIFI_TIMEOUT = 15000;       // 15 seconden proberen te verbinden met Wi-Fi
  const unsigned long WIFI_RETRY_INTERVAL = 30000; // Elke 30s buiten bereik checken of Wi-Fi er weer is

#else
  // Originele afhandeling voor andere platformen (niet gewijzigd)
  #if defined(RP2040_PLATFORM) || defined(NRF52_PLATFORM) || defined(STM32_PLATFORM)
    #include <helpers/ArduinoSerialInterface.h>
    ArduinoSerialInterface serial_interface;
  #endif
#endif

/* GLOBAL OBJECTS */
#ifdef DISPLAY_CLASS
  #include "UITask.h"
  UITask ui_task(&board, &serial_interface);
#endif

StdRNG fast_rng;
SimpleMeshTables tables;
MyMesh the_mesh(radio_driver, fast_rng, rtc_clock, tables, store
   #ifdef DISPLAY_CLASS
      , &ui_task
   #endif
);

/* END GLOBAL OBJECTS */

void halt() {
  while (1) ;
}

void setup() {
  Serial.begin(115200);
  board.begin();

  // Initialiseer de enkele ingebouwde rode User LED
  pinMode(LED_BUILTIN, OUTPUT);
  // Bij de XIAO S3 is LOW = AAN en HIGH = UIT. We zetten hem eerst uit.
  digitalWrite(LED_BUILTIN, HIGH);

#ifdef DISPLAY_CLASS
  DisplayDriver* disp = NULL;
  if (display.begin()) {
    disp = &display;
    disp->startFrame();
  #ifdef ST7789
    disp->setTextSize(2);
  #endif
    disp->drawTextCentered(disp->width() / 2, 28, "Loading...");
    disp->endFrame();
  }
#endif

  if (!radio_init()) { halt(); }
  fast_rng.begin(radio_driver.getRngSeed());

#if defined(ESP32)
  SPIFFS.begin(true);
  store.begin();
  the_mesh.begin(disp != NULL);

  // STARTUPLOOGIC ENERGIE EN WI-FI VOOR ESP32
  board.setInhibitSleep(true); 
  Serial.println("[SmartSwitch] Opstarten: Wi-Fi verbinding zoeken...");
  WiFi.begin(WIFI_SSID, WIFI_PWD);
  wifi_interface.begin(TCP_PORT);
  
  the_mesh.startInterface(wifi_interface);
  current_mode = MODE_WIFI_CONNECTING;
  wifi_timer = millis();

#else
  // Originele setup voor Non-ESP32 platformen
  #if defined(NRF52_PLATFORM) || defined(STM32_PLATFORM)
    InternalFS.begin();
    store.begin();
    the_mesh.begin(disp != NULL);
    serial_interface.begin(Serial);
    the_mesh.startInterface(serial_interface);
  #elif defined(RP2040_PLATFORM)
    LittleFS.begin();
    store.begin();
    the_mesh.begin(disp != NULL);
    serial_interface.begin(Serial);
    the_mesh.startInterface(serial_interface);
  #endif
#endif

  sensors.begin();
#if ENV_INCLUDE_GPS == 1
  the_mesh.applyGpsPrefs();
#endif
#ifdef DISPLAY_CLASS
  ui_task.begin(disp, &sensors, the_mesh.getNodePrefs());
#endif
}

void loop() {
  the_mesh.loop();
  sensors.loop();

#ifdef DISPLAY_CLASS
  ui_task.loop();
#endif

  rtc_clock.tick();

  // DE SMART SWITCH LOGICA (ALLEEN VOOR ESP32 CORES)
#ifdef ESP32
  // HARDWARE FIX: Houdt het Seeed Expansion Board en de voeding stabiel in de lucht
  board.setInhibitSleep(true); 

  unsigned long current_time = millis();
  static unsigned long last_led_blink = 0;
  static bool led_state = false;

  switch (current_mode) {
    
    case MODE_WIFI_CONNECTING:
      // Tijdens het zoeken naar Wi-Fi: Rode led knippert heel snel (elke 100ms)
      if (current_time - last_led_blink > 100) {
        led_state = !led_state;
        digitalWrite(LED_BUILTIN, led_state ? LOW : HIGH); 
        last_led_blink = current_time;
      }

      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("[SmartSwitch] Wi-Fi succesvol verbonden! Schakelen naar Wi-Fi Modus.");
        digitalWrite(LED_BUILTIN, LOW); // LOW is AAN: led permanent rood branden
        current_mode = MODE_WIFI_ACTIVE;
      } 
      else if (current_time - wifi_timer > WIFI_TIMEOUT) {
        Serial.println("[SmartSwitch] Wi-Fi time-out. Schakelen naar Bluetooth...");
        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
        delay(100);
        
        ble_interface.begin(BLE_NAME_PREFIX, the_mesh.getNodePrefs()->node_name, the_mesh.getBLEPin());
        the_mesh.startInterface(ble_interface);
        
        current_mode = MODE_BLE_ACTIVE;
        last_wifi_check = current_time;
      }
      break;

    case MODE_WIFI_ACTIVE:
      // Wi-Fi is stabiel: Hou de rode led constant BRANDEND
      digitalWrite(LED_BUILTIN, LOW); 

      if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[SmartSwitch] Wi-Fi verbinding verloren! Overschakelen naar Bluetooth.");
        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
        delay(100);
        
        ble_interface.begin(BLE_NAME_PREFIX, the_mesh.getNodePrefs()->node_name, the_mesh.getBLEPin());
        the_mesh.startInterface(ble_interface);
        
        current_mode = MODE_BLE_ACTIVE;
        last_wifi_check = current_time;
      }
      break;

    case MODE_BLE_ACTIVE:
      // Bluetooth modus: Laat de rode led rustig knipperen (elke 500ms)
      if (current_time - last_led_blink > 500) {
        led_state = !led_state;
        digitalWrite(LED_BUILTIN, led_state ? LOW : HIGH); 
        last_led_blink = current_time;
      }

      if (current_time - last_wifi_check > WIFI_RETRY_INTERVAL) {
        Serial.println("[SmartSwitch] Periodieke scan: Kijken of thuis-Wi-Fi weer in de buurt is...");
        WiFi.mode(WIFI_STA);
        WiFi.begin(WIFI_SSID, WIFI_PWD);
        
        unsigned long scan_start = millis();
        bool found_wifi = false;
        while (millis() - scan_start < 5000) {
          if (WiFi.status() == WL_CONNECTED) {
            found_wifi = true;
            break;
          }
          delay(100);
        }
        
        if (found_wifi) {
          Serial.println("[SmartSwitch] Thuis-Wi-Fi weer gevonden! Bluetooth uitschakelen...");
          digitalWrite(LED_BUILTIN, LOW); // Gelijk weer permanent rood aan
          the_mesh.startInterface(wifi_interface);
          current_mode = MODE_WIFI_ACTIVE;
        } else {
          Serial.println("[SmartSwitch] Thuis-Wi-Fi nog niet in de buurt. We blijven op Bluetooth.");
          WiFi.disconnect(true);
          WiFi.mode(WIFI_OFF);
          last_wifi_check = millis();
        }
      }
      break;
  }
#endif
}