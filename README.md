# BonbridgeESP32

**Mach deinen USB-Bondrucker zum Netzwerk- und WLAN-Drucker für dein Kassensystem.**

Viele Kassensysteme (wie z. B. OrderAssist, Gastronomiesoftware oder Kassen-Apps) können Bondrucker nur über das Netzwerk über eine **IP-Adresse und Port 9100** ansprechen. USB-Drucker können sie oft nicht direkt verwenden.

**BonbridgeESP32** löst dieses Problem mit einem kleinen, sparsamen Mikrocontroller (ESP32-S3):
Er nimmt Druckaufträge über ein **Netzwerkkabel (LAN)** oder über **WLAN** entgegen und leitet sie ohne Verzögerung direkt an deinen USB-Bondrucker weiter.

---

## Funktionen im Überblick

* **LAN & WLAN mit automatischer Umschaltung:**
  * **Kabel hat Vorrang:** Sobald ein Netzwerkkabel eingesteckt ist, nutzt der Adapter ausschließlich das Kabel. Das WLAN-Funkmodul wird komplett ausgeschaltet (spart Strom und schont das Funknetz).
  * **WLAN als Reserve:** Wird das Kabel abgezogen oder ist keins gesteckt, schaltet der Adapter automatisch das WLAN ein und verbindet sich mit deinem hinterlegten Funknetz.
* **Port 9100 RAW-Drucker-Server:** Der weltweite Standard für Netzwerk-Bondrucker. Druckdaten werden 1:1 an den USB-Drucker gestreamt.
* **1 Drucker pro Adapter:** Maximale Zuverlässigkeit – keine Vermischung von Bons.
* **Extrem sparsame Einstellungsseite (Web-Oberfläche):**
  * Im Browser erreichbar über die IP-Adresse des Geräts (`http://<ip>/`).
  * Verbraucht im Hintergrund **0 % Rechenleistung**, wenn sie nicht aufgerufen wird.
  * Zeigt sofort an, **worüber aktuell kommuniziert wird** (🟢 *LAN-Kabel aktiv* oder 🔵 *WLAN aktiv*).
  * Test-Knöpfe für **Drucktest** und **Kassenlade öffnen**.
  * Auswahl des **Druckprofils** (Epson TM-T88, Standard 80 mm, Kompakt 58 mm, Star).
  * Schalter: **Hinweis drucken wenn Netzwerk ausfällt**.
* **Netzwerk-Wächter (Ausfall-Alarm):**
  * Bricht die Netzwerkverbindung komplett ab, druckt der Drucker selbstständig einen Warnzettel aus:  
    *„Achtung: Netzwerkverbindung unterbrochen! Bitte LAN-Kabel oder Router prüfen.“*

---

## Benötigte Hardware

1. **Mikrocontroller:** **ESP32-S3** (z. B. ESP32-S3-DevKitC-1).  
   *(Wichtig: Ein älterer, klassischer ESP32 kann keine USB-Geräte als Host ansteuern – daher muss es ein ESP32-S3 sein!)*
2. **Netzwerk-Aufsatz:** **WIZnet W5500** SPI-Ethernet-Modul (für den LAN-Kabelanschluss).
3. **USB-OTG-Kabel / Buchse:** Zum Anstecken des Druckers an den ESP32-S3.
4. **Stromversorgung:** Über die Pins (5V und GND), z. B. von einem stabilen 5V-Netzteil oder dem 5V-Ausgang des Druckers.
5. **Bondrucker:** Beliebiger USB-ESC/POS-Bondrucker (z. B. Epson TM-T88V/VI, Munbyn, Bixolon, etc.).

---

## Verkabelung (Pin-Belegung)

### 1. Stromversorgung
| Netzteil / Quelle | ESP32-S3 Pin |
|---|---|
| +5V (Plus) | **5V** (bzw. VIN) |
| GND (Minus) | **GND** |

*(Wichtig: Auch die 5V-Leitung der USB-Buchse für den Drucker muss mit 5V versorgt werden, damit der Druckeranschluss Strom hat.)*

### 2. W5500 Netzwerk-Modul (LAN-Kabel)
Das W5500-Modul wird über die SPI-Leitungen mit dem ESP32-S3 verbunden:

| W5500 Pin | ESP32-S3 Pin | Funktion |
|---|---|---|
| **VCC** | **3.3V** | Stromversorgung Modul |
| **GND** | **GND** | Masse / Minus |
| **MOSI** | **GPIO 11** | Datenleitung zum Modul |
| **MISO** | **GPIO 12** | Datenleitung vom Modul |
| **SCLK** | **GPIO 10** | Taktleitung |
| **CS / SCS** | **GPIO 9** | Modulauswahl (Chip Select) |
| **INT** | **GPIO 14** | Unterbrechungssignal |
| **RST** | **GPIO 13** | Rücksetzleitung (Reset) |

### 3. USB-Buchse (Drucker-Anschluss)
| USB-Buchse | ESP32-S3 Pin |
|---|---|
| **D-** (Weiß) | **GPIO 19** |
| **D+** (Grün) | **GPIO 20** |
| **+5V** (Rot) | **5V Stromversorgung** |
| **GND** (Schwarz) | **GND** |

---

## Software auf den ESP32 übertragen

Am einfachsten geht das mit **PlatformIO** (in VS Code) oder der **Arduino IDE**:

### Mit PlatformIO:
1. Projektordner `BonbridgeESP32` in VS Code mit der PlatformIO-Erweiterung öffnen.
2. ESP32-S3 per Programmier-USB-Kabel an den Computer anschließen.
3. Unten in der Leiste auf den Haken (**Build**) und dann auf den Pfeil (**Upload**) klicken.

### Mit der Arduino IDE:
1. Datei `src/main.cpp` öffnen (oder in ein `.ino`-Projekt kopieren).
2. Board auswählen: **ESP32S3 Dev Module**.
3. Einstellungen:
   * **USB Mode:** *Hardware CDC and JTAG* (oder *OTG*)
   * **USB CDC On Boot:** *Enabled*
4. Auf **Hochladen** klicken.

---

## Erste Schritte & Bedienung

1. **Einschalten:** Nach dem Anschließen an 5V startet der Adapter in weniger als 2 Sekunden.
2. **Web-Oberfläche öffnen:**
   * Stecke das LAN-Kabel ein.
   * Schau im Router nach der vergebenen IP-Adresse (oder drücke nach dem Start einmal kurz auf den Drucker, falls eingerichtet).
   * Öffne im Browser: `http://<IP-Adresse-des-ESP>/`
3. **Einstellen:**
   * **Aktuelle Verbindung:** Die Seite zeigt dir oben direkt an, ob LAN oder WLAN genutzt wird.
   * **WLAN eintragen:** Trage deinen WLAN-Namen und das Passwort ein, damit der Adapter automatisch darauf zurückgreifen kann, falls das Kabel einmal abgezogen wird.
   * **Drucktest & Kassenlade:** Klicke auf die Test-Buttons, um Drucker und Geldschublade zu prüfen.
   * Klicke auf **Einstellungen speichern**.
4. **Im Kassensystem einrichten:**
   * Wähle in deiner Kassen-App (z. B. OrderAssist) **Netzwerk-Drucker / ESC-POS**.
   * Gib die **IP-Adresse** des ESP32 ein.
   * Gib als Port **9100** ein.
   * Fertig! Ab sofort druckt deine Kasse über den Adapter.
