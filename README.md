# MeshCore Companion Infinity Edition 🚀

Welkom bij de **MeshCore: Infinity Edition**. Deze repository is een onafhankelijke doorontwikkeling van de originele MeshCore-firmware, specifiek gebouwd voor snellere innovatie, bredere hardware-ondersteuning en superieure netwerkstabiliteit op ESP32-architecturen.

Waar de originele ontwikkeling stagneert, pusht de Infinity Edition de grenzen van wat mogelijk is met dynamische netwerk-switching en custom hardware-telemetrie.

## ✨ Waarom de Infinity Edition?

Deze versie introduceert de **SmartSwitch Engine**: een robuuste failover-logica waarmee ESP32-nodes naadloos, dynamisch en zonder herstart kunnen schakelen tussen Wi-Fi en Bluetooth (BLE) interfaces op basis van bereikbaarheid. Daarnaast is de C++ architectuur opgeschoond en zijn er specifieke partitie- en geheugenoptimalisaties doorgevoerd voor moderne borden.

## 🛠️ Ondersteunde Hardware

De Infinity Edition heeft geteste en geoptimaliseerde board-definities voor een brede vloot aan hardware:

* **Heltec LoRa32 V2**
* **Heltec LoRa32 V3**
* **Heltec V4 OLED**
* **Heltec V4 TFT**
* **LilyGo T3-S3** (Inclusief de `huge_app.csv` partitie-layout voor volledige benutting van 4MB Flash, helaas geen ota meer door gebrek aan ruimte).
* **Seeed Studio XIAO ESP32S3** (Met unieke hardware-implementaties voor batterij-uitlezing).

## 🚀 Kernfunctionaliteiten

### 1. SmartSwitch: Geautomatiseerde Failover & Recovery

Nodes zitten niet meer vast aan één interface. De interne statemachine (`MeshMode`) beheert de verbinding proactief:

* **Boot:** Start in `MODE_WIFI_CONNECTING` (max 15s timeout).
* **Failover (`MODE_BLE_ACTIVE`):** Als Wi-Fi wegvalt, wordt de Wi-Fi radio volledig uitgeschakeld (`WiFi.mode(WIFI_OFF)`) om stroom te besparen, waarna de BLE-interface naadloos de communicatie overneemt.
* **Auto-Recovery:** Tijdens BLE-modus voert het systeem elke 30 seconden een onzichtbare Wi-Fi achtergrondscan (5s) uit. Zodra het thuisnetwerk weer in beeld is, wordt BLE uitgeschakeld en Wi-Fi automatisch hersteld.

### 2. Geavanceerde Hardware Integratie

* **XIAO S3 Batterij Telemetrie:** De standaard XIAO S3 mist interne batterijmeting. In deze firmware is een custom `XiaoS3WIOBoard` klasse gebouwd die gebruik maakt van een fysieke 1M/1M spanningsdeler op `PIN_VBAT` (GPIO 1). De firmware leest de 12-bit ADC uit en berekent het exacte voltage.
* **Bootlock Beveiliging:** Ingebouwde `PWRMGT_VOLTAGE_BOOTLOCK` (3.3V) voorkomt schadelijke bootloops bij bijna lege batterijen.
* **Hardware Fixes:** Actieve power-management fixes (`board.setInhibitSleep(true)`) stabiliseren de stroomvoorziening tijdens het in- en uitschakelen van radio's (cruciaal voor uitbreidingsborden).

### 3. Visuele Status Feedback

Specifieke LED-patronen tonen direct de status van de node (inclusief auto-detectie voor reverse-polarity LEDs zoals op de XIAO S3 waar `LOW = AAN`):

* 🔴 **Snel knipperen (100ms):** Zoeken naar Wi-Fi.
* 🔴 **Continu AAN:** Stabiel verbonden met Wi-Fi.
* 🔴 **Langzaam knipperen (500ms):** Wi-Fi onbereikbaar, draait op Bluetooth-modus.

### 4. Architectuur Optimalisaties

* **Geheugen:** Directe koppeling van `SPIFFS` aan de `DataStore` voor ESP32-omgevingen.
* **Compiler Compatibiliteit:** Gelijktijdige ondersteuning voor `SerialWifiInterface` en `SerialBLEInterface` met hard-mapped interfaces om de originele `serial_interface` calls in de core niet te breken.

## 💻 Installatie & Build Instructies

Volg deze stappen om de firmware te compileren en naar je nodes te uploaden:

1. **Kloon de repository:** Download of kloon deze repository naar je lokale machine.
2. **Open in PlatformIO:** Open de gedownloade projectmap in VS Code met de PlatformIO-extensie geïnstalleerd.
3. **Configureer je Wi-Fi credentials:** * Open het `platformio.ini` bestand in de root van het project.
* Navigeer naar de specifieke omgeving die je wilt gebruiken (bijvoorbeeld `[env:Xiao_S3_WIO_companion_radio_wifi]` voor de Wi-Fi en Companion App functionaliteit).
* Pas de volgende `build_flags` aan met de gegevens van jouw eigen thuisnetwerk:
```ini
-D WIFI_SSID='"JOUW_SSID_HIER"'
-D WIFI_PWD='"JOUW_WACHTWOORD_HIER"'

```
4. **Kies je functionaliteit:** * Als je de node wilt koppelen met de **Companion App**, kies dan een van de `companion_radio` omgevingen (zoals BLE of Wi-Fi). Deze versies activeren de juiste UI-taken en interfaces om naadloos met de app te communiceren.
5. **Clean, Build & Upload (CLEAN IS ALLEEN NODIG VOOR DE LILYGO T3S3!):** Maak een backup van je private key voor je de clean uitvoert zodat je dezelde Meshcore identiteit kunt behouden indien je dit wenst.
* Klik in de blauwe balk van PlatformIO op het **Prullenbak-icoon** (Clean) om eventuele oude builds te wissen.
* Klik op het **Vinkje** (Build) om de firmware te compileren.
* Klik op het **Pijltje naar rechts** (Upload) om de firmware naar je aangesloten board te flashen.
Zodra de upload is voltooid, start de node op, zoekt hij automatisch naar het hardcoded Wi-Fi netwerk en activeert hij de SmartSwitch engine. Alles is direct klaar voor gebruik!

### 🌐 Captive Portal: Netwerkconfiguratie zonder Hardcoden via Acces Point

De Infinity Edition maakt een einde aan het hardcoden van netwerkgegevens in de broncode. Als de node geen bekende Wi-Fi-netwerken in zijn geheugen (`Preferences`) heeft staan (of als het de allereerste keer is dat de firmware start), treedt de **Captive Portal** in werking.

1. **Allereerste opstart:** Bij de allereerste boot blijft de node (als deze een scherm heeft) op **"Loading..."** staan. Dit is normaal! De node zoekt op de achtergrond naar opgeslagen inloggegevens.
2. **Verbind met het Access Point:** Omdat er geen gegevens zijn, zendt de node zijn eigen Wi-Fi netwerk uit. Zoek op je telefoon of laptop naar het netwerk:
<img width="1080" height="1950" alt="image" src="https://github.com/user-attachments/assets/956b47f4-48b4-4f91-abf9-e2163c27c0df" />
* **SSID:** `MeshCore-Configurator`
* **Wachtwoord:** (Geen wachtwoord)
3. **Gegevens invullen:** Open je browser en ga naar `http://192.168.4.1` (of de Captive Portal opent automatisch). Je krijgt nu het zwart/neon-groene **Infinity Configurator** scherm te zien. Vul hier je SSID en Wachtwoord in en druk op opslaan.
4. **Reboot & Verbinding:** De node herstart zichzelf en zal nu direct verbinden met je netwerk en overschakelen naar de reguliere MeshCore- en Bluetooth-logica.

---

## ⚡ Firmware Flashen (Twee methodes)

Je kunt de Infinity Edition op twee manieren flashen: via een gebruiksvriendelijke Web Flasher of handmatig via Visual Studio Code (PlatformIO).
Je kunt de gecompileerde firmware met captive portal en acces point vinden in de map `compiled firmware`.

### Methode 1: Snel flashen via de MeshCore Web Flasher (Aangeraden)

Je kunt een gecompileerde `.bin` file bestand direct vanuit je browser flashen met de officiële flasher.

1. Zorg ervoor dat je de juiste `.bin` file voor jouw specifieke board hebt gedownload.
2. **Bootloader Modus:** Zet je node eerst fysiek in de 'Boot' (Download) modus. Voor veel ESP32-S3 borden (zoals de XIAO S3 of LilyGo) doe je dit zo:
* Houd de **BOOT**-knop ingedrukt.
* Druk kort op de **RESET**-knop.
* Laat de **BOOT**-knop los. (Het scherm blijft nu zwart).
3. Ga naar [https://flasher.meshcore.io/](https://flasher.meshcore.io/).
4. Kies in de interface voor **Custom Firmware** en selecteer jouw `firmware.bin` bestand.
5. Klik op Flash en wacht tot het proces voltooid is.

⚠️ **Belangrijk voor de LilyGo T3-S3:** Voordat je de nieuwe firmware naar een LilyGo T3-S3 flasht, **moet** je eerst een "Erase Flash" uitvoeren om geheugencorruptie (vooral in de partitietabellen) te voorkomen.

* **Let op:** Een Erase Flash wist ook je Node Identiteit! Maak vooraf een back-up van je `private key` als je je huidige identiteit in het mesh-netwerk wilt behouden.

### Methode 2: Geavanceerd flashen via Visual Studio Code (PlatformIO)

Voor ontwikkelaars of gebruikers die de broncode willen inzien of wijzigen.

1. **Kloon de repository:** Download of kloon deze repository naar je lokale machine. Zorg voor een **kort bestandspad** (bijv. `C:\Projects\MeshCore`) om compiler-fouten door de Windows pad-limiet te voorkomen.
2. **Open in PlatformIO:** Open de projectmap in VS Code. PlatformIO zal automatisch beginnen met het downloaden van de benodigde libraries.
3. **Selecteer je Board:** Klik onderin de blauwe balk op het geselecteerde `env` (bijv. `[env:LilyGo_T3S3...]`) en kies de juiste hardware omgeving (Wi-Fi of BLE Companion).
4. **Clean (Alleen LilyGo T3-S3):** Voor de LilyGo: voer altijd eerst de taak `Erase Flash` uit via de PlatformIO navigatiebalk aan de linkerkant (Platform > Erase Flash). *Vergeet je private key back-up niet!*
5. **Bouwen & Flashen:** * Klik op het **Vinkje** (Build) om te controleren of de firmware compileert.
* Zet de node in Bootloader-modus (zie Methode 1).
* Klik op het **Pijltje naar rechts** (Upload) om de code naar je board te flashen. Zodra de upload start, kun je de node resetten of laten opstarten.

*(Zodra de flash is afgerond, wacht je tot het scherm op "Loading..." blijft staan en verbind je met de acces point `MeshCore-Configurator` Wi-Fi om de installatie af te ronden!)*
