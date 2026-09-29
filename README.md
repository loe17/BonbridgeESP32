# BonbridgeESP32

**Mach deinen USB-Bondrucker zum Netzwerk- und WLAN-Drucker für dein Kassensystem.**

Viele Kassensysteme (wie z. B. Kassensoftware, Gastronomiesoftware oder Kassen-Apps) können Bondrucker nur über das Netzwerk über eine **IP-Adresse und Port 9100** ansprechen. USB-Drucker können sie oft nicht direkt verwenden.

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

### Mit der Arduino IDE:
1. Starte die **Arduino IDE**.
2. Klicke im Menü auf **Datei -> Öffnen...** (oder drücke `Strg + O`).
3. Wähle im Ordner `BonbridgeESP32` die Datei **`BonbridgeESP32.ino`** aus.
4. Alle Programmteile öffnen sich nun automatisch als übersichtliche Reiter (Tabs).
5. Wähle unter **Werkzeuge -> Board -> esp32** dein Board aus: **ESP32S3 Dev Module**.
6. Stelle unter **Werkzeuge** folgende Werte ein:
   * **USB Mode:** *Hardware CDC and JTAG*
   * **USB CDC On Boot:** *Enabled* (Wichtig: Nur so sendet der Chip Text an den PC-Monitor!)
   * **PSRAM:** *Disabled* (Sehr wichtig: Verhindert Start-Absturzschleifen bei Super Mini Boards!)
   * **Flash Size:** *4MB* (oder *8MB*)
   * **Flash Mode:** *QIO 80MHz*
7. Verbinde den ESP32-S3 per USB-Kabel mit dem Computer, wähle den passenden **Port** aus und klicke auf den Pfeil (**Hochladen**).

### Mit PlatformIO (VS Code):
1. Projektordner `BonbridgeESP32` in VS Code öffnen.
2. Unten in der Leiste auf den Haken (**Build**) und dann auf den Pfeil (**Upload**) klicken.

---

## Erste Schritte & Bedienung

1. **Einschalten & Status-LED (Lebenszeichen):**
   * Nach dem Anstecken an den Strom blitzt die LED 3x kurz auf als Startsignal.
   * Danach schaltet sie im ruhigen Sekundentakt um („Herzschlag“). Solange sie gleichmäßig blinkt, läuft das Gerät einwandfrei.
   * **Hinweis zur Lade-LED:** Auf dem Board befindet sich auch eine kleine rote Ladeleuchte. Diese erlischt nach wenigen Sekunden, weil kein Akku angeschlossen ist – das ist völlig normal!

2. **IP-Adresse sofort ablesen (Serieller Monitor):**
   * Öffne in der Arduino IDE oben rechts die Lupe (**Serieller Monitor**).
   * Stelle unten rechts die Geschwindigkeit auf **115200 Baud**.
   * Beim Starten oder Verbinden siehst du sofort im Klartext:
     * Die vergebene **IP-Adresse**
     * Ob **LAN-Kabel** oder **WLAN** aktiv ist
     * Die Web-Adresse (`http://...`) und den Drucker-Port (`:9100`)
     * Den Status deines angeschlossenen USB-Bondruckers

3. **Web-Oberfläche öffnen:**
   * Stecke das LAN-Kabel ein (oder nutze die im Monitor angezeigte IP-Adresse).
   * Öffne im Browser: `http://<IP-Adresse-des-ESP>/`

4. **Einstellen:**
   * **Aktuelle Verbindung:** Die Seite zeigt dir oben direkt an, ob LAN oder WLAN genutzt wird.
   * **WLAN eintragen:** Trage deinen WLAN-Namen und das Passwort ein, damit der Adapter automatisch darauf zurückgreifen kann, falls das Kabel einmal abgezogen wird.
   * **Drucktest & Kassenlade:** Klicke auf die Test-Knöpfe, um Drucker und Geldschublade direkt vom Browser aus zu testen.
   * Klicke auf **Einstellungen speichern**.

5. **Im Kassensystem einrichten:**
   * Wähle in deiner Kassen-App (z. B. Kassensoftware) **Netzwerk-Drucker / ESC-POS**.
   * Gib die **IP-Adresse** des ESP32 ein.
   * Gib als Port **9100** ein.
   * Fertig! Ab sofort druckt deine Kasse zuverlässig über den Adapter.
