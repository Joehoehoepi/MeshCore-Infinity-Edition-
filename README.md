MeshCore: Infinity Edition 🚀
Welkom bij de MeshCore: Infinity Edition. Deze repository is een onafhankelijke doorontwikkeling van de originele MeshCore-firmware, specifiek gebouwd voor snellere innovatie, bredere hardware-ondersteuning en superieure netwerkstabiliteit op ESP32-architecturen.

Waar de originele ontwikkeling stagneert, pusht de Infinity Edition de grenzen van wat mogelijk is met dynamische netwerk-switching en custom hardware-telemetrie.

✨ Waarom de Infinity Edition?
Deze versie introduceert de SmartSwitch Engine: een robuuste failover-logica waarmee ESP32-nodes naadloos, dynamisch en zonder herstart kunnen schakelen tussen Wi-Fi en Bluetooth (BLE) interfaces op basis van bereikbaarheid. Daarnaast is de C++ architectuur opgeschoond en zijn er specifieke partitie- en geheugenoptimalisaties doorgevoerd voor moderne borden.

🛠️ Ondersteunde Hardware
De Infinity Edition heeft geteste en geoptimaliseerde board-definities voor een brede vloot aan hardware:

Heltec LoRa32 V2 & V3

Heltec Wireless Tracker V4

Heltec V4 TFT

LilyGo T3-S3 (Inclusief de huge_app.csv partitie-layout voor volledige benutting van 4MB Flash).

Seeed Studio XIAO ESP32S3 (Met unieke hardware-implementaties voor batterij-uitlezing).

🚀 Kernfunctionaliteiten
1. SmartSwitch: Geautomatiseerde Failover & Recovery
Nodes zitten niet meer vast aan één interface. De interne statemachine (MeshMode) beheert de verbinding proactief:

Boot: Start in MODE_WIFI_CONNECTING (max 15s timeout).

Failover (MODE_BLE_ACTIVE): Als Wi-Fi wegvalt, wordt de Wi-Fi radio volledig uitgeschakeld (WiFi.mode(WIFI_OFF)) om stroom te besparen, waarna de BLE-interface naadloos de communicatie overneemt.

Auto-Recovery: Tijdens BLE-modus voert het systeem elke 30 seconden een onzichtbare Wi-Fi achtergrondscan (5s) uit. Zodra het thuisnetwerk weer in beeld is, wordt BLE uitgeschakeld en Wi-Fi automatisch hersteld.

2. Geavanceerde Hardware Integratie
XIAO S3 Batterij Telemetrie: De standaard XIAO S3 mist interne batterijmeting. In deze firmware is een custom XiaoS3WIOBoard klasse gebouwd die gebruik maakt van een fysieke 1M/1M spanningsdeler op PIN_VBAT (GPIO 1). De firmware leest de 12-bit ADC uit en berekent het exacte voltage.

Bootlock Beveiliging: Ingebouwde PWRMGT_VOLTAGE_BOOTLOCK (3.3V) voorkomt schadelijke bootloops bij bijna lege batterijen.

Hardware Fixes: Actieve power-management fixes (board.setInhibitSleep(true)) stabiliseren de stroomvoorziening tijdens het in- en uitschakelen van radio's (cruciaal voor uitbreidingsborden).

3. Visuele Status Feedback
Specifieke LED-patronen tonen direct de status van de node (inclusief auto-detectie voor reverse-polarity LEDs zoals op de XIAO S3 waar LOW = AAN):

🔴 Snel knipperen (100ms): Zoeken naar Wi-Fi.

🔴 Continu AAN: Stabiel verbonden met Wi-Fi.

🔴 Langzaam knipperen (500ms): Wi-Fi onbereikbaar, draait op Bluetooth-modus.

4. Architectuur Optimalisaties
Geheugen: Directe koppeling van SPIFFS aan de DataStore voor ESP32-omgevingen.

Compiler Compatibiliteit: Gelijktijdige ondersteuning voor SerialWifiInterface en SerialBLEInterface met hard-mapped interfaces om de originele serial_interface calls in de core niet te breken.

💻 Installatie & Build Instructies
