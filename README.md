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
* **Bequeme WLAN-Suche & Live-Verbindung in der Web-Oberfläche:**
  * **🔍 WLAN suchen:** Findet alle Funknetze in Reichweite mit Signalstärke (z. B. 📶 85%) und Schlosssymbol. Ein Klick genügt, um das gewünschte WLAN auszuwählen.
  * **Passwort einblenden (👁️):** Zeigt das eingegebene Passwort im Klartext an, um Tippfehler sofort zu vermeiden.
  * **Direkt verbinden mit Live-Rückmeldung:** Ein Klick auf *„Mit diesem WLAN verbinden“* stellt die Verbindung her und zeigt nach wenigen Sekunden die neue IP-Adresse als anklickbaren Link an – oder bei Fehlern (z. B. Passwort falsch) eine verständliche Erklärung.
  * **Automatischer Einrichtungs-Hotspot (`Bonbridge-Setup`):** Ist noch kein WLAN hinterlegt oder der Router nicht erreichbar, spannt der Adapter ein eigenes Hilfsnetzwerk auf (`Bonbridge-Setup` unter `http://192.168.4.1`). So kann man das WLAN kinderleicht direkt vom Smartphone oder Laptop einrichten!
* **Drahtlose Firmware-Aktualisierung (Funk-Update / OTA) mit Versionsprüfung:**
  * **🔍 Automatische Versionsprüfung:** Ein Klick auf *„Nach Versionen auf GitHub suchen“* prüft, ob eine neuere Version bereitsteht oder man bereits aktuell ist.
  * **Auswahlmenü für Updates & Downgrades:** Zeigt alle jemals veröffentlichten Versionen an. Man kann mit einem Klick auf eine neuere Version aktualisieren oder bei Bedarf auf eine ältere Version zurückspringen (Downgrade).
  * **Manuelle Datei:** Ermöglicht das Hochladen einer eigenen `.bin`-Datei direkt über den Web-Browser.
  * **Live-Balken & Sicherheit:** Zeigt den Fortschritt von 0–100 % in Echtzeit an; alle gespeicherten WLAN- und Druckereinstellungen bleiben beim Update vollständig erhalten.
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
| Netzteil / Quelle | ESP32-S3 Pin | Hinweise |
|---|---|---|
| **+5V (Plus)** | **5V** (bzw. VIN) | Stabiles 5V-Netzteil (mindestens **1,5 A bis 2 A** empfohlen) |
| **GND (Minus)** | **GND** | Gemeinsame Masse |

> [!TIP]
> **Tipps für eine stabile 5V-Stromversorgung:**
> * **5V über Pins oder USB:** Du kannst den ESP32-S3 über die Pins (**5V** und **GND**) mit einem 5V-Labornetzteil/Netzteil versorgen, oder direkt über ein 5V-USB-Netzteil (am besten mit Standard USB-A auf USB-C Kabel).
> * **Messpunkt zur Kontrolle:** Liegt an den Pins 5V an, muss am Pin **3V3** (gegen GND gemessen) eine saubere Spannung von 3,3 Volt anliegen.
> * Verwende möglichst **kurze und dicke Kabel** (keine langen, hauchdünnen Steckbrett-Drähte). Beim Senden von WLAN-Daten benötigt der Funkchip kurzzeitig Stromspitzen bis zu 450 mA – zu dünne Drähte führen zu einem Spannungsabfall (Brownout).
> * Das System verfügt über eine integrierte **Spannungsüberwachung**: Auf der Weboberfläche siehst du in der Zeile *„System-Start“* sofort, ob das Board sauber gestartet ist oder ob ein Spannungseinbruch aufgetreten ist.
> * Auch die 5V-Leitung der USB-Buchse für den Drucker muss mit 5V versorgt werden, damit der Druckeranschluss Strom hat.

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

> [!IMPORTANT]
> **Wichtig – Keinen USB-C-Hub verwenden:**
> Ein USB-Hub (Mehrfachverteiler) besitzt einen eigenen Steuerchip und verhindert, dass der ESP32-S3 den Drucker erkennt. Verwende stattdessen einen einfachen **USB-OTG-Adapter** (USB-C auf USB-A) oder ein **OTG-Y-Kabel** mit direkter Stromeinspeisung.

> [!NOTE]
> **Betrieb am Computer vs. Drucker-Betrieb:**
> * **Am Computer angeschlossen (USB-Kabel zum PC):** Der ESP32 erkennt die PC-Verbindung automatisch. Der USB-Druckermodus wird pausiert, damit der serielle Monitor und die Programmierung stabil funktionieren. Die Web-Oberfläche und das Netzwerk laufen uneingeschränkt.
> * **Im Kassen-/Drucker-Betrieb:** Betreibe das Board an einem 5V-Netzteil (z. B. 5V/GND-Pins oder zweiter Anschluss). Der Drucker wird direkt über einen einfachen USB-OTG-Adapter (USB-C auf USB-A) angesteckt.

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

1. **Einschalten & Status-LED (Lebenszeichen & Farbcode):**
   * Direkt nach dem Einschalten leuchtet die RGB-LED (GPIO 48) **Violett** (bestätigt sofort: Stromversorgung steht, Prozessor läuft!).
   * **Grün (sanfter Puls / Herzschlag):** Erfolgreich mit dem Netzwerk (WLAN oder LAN) verbunden und betriebsbereit!
   * **Blau (Puls):** Einrichtungs-Hotspot (`Bonbridge-Setup`) ist aktiv – verbinde dich mit dem WLAN und öffne `http://192.168.4.1`.
   * **Gelb / Orange (blinkend):** Das Gerät sucht das WLAN und versucht die Verbindung zum Router herzustellen.
   * **LED bleibt dunkel?** Bitte mit einem Multimeter prüfen, ob zwischen Pin **3V3** und **GND** ca. 3,3 Volt anliegen (z. B. Kabelverbindung, Polung oder Strombegrenzung am Netzteil prüfen).
   * **Hinweis zur Lade-LED:** Auf manchen Boards befindet sich zusätzlich eine winzige rote Akkulade-LED. Diese erlischt nach wenigen Sekunden, weil kein Akku angeschlossen ist – das ist völlig normal!

2. **IP-Adresse sofort ablesen (Serieller Monitor):**
   * Öffne in der Arduino IDE oben rechts die Lupe (**Serieller Monitor**).
   * Stelle unten rechts die Geschwindigkeit auf **115200 Baud**.
   * Beim Starten oder Verbinden siehst du sofort im Klartext:
     * Die vergebene **IP-Adresse**
     * Ob **LAN-Kabel** oder **WLAN** aktiv ist
     * Die Web-Adresse (`http://...`) und den Drucker-Port (`:9100`)
     * Den Status deines angeschlossenen USB-Bondruckers

3. **Web-Oberfläche öffnen:**
   * **Über LAN-Kabel:** Stecke das Netzwerkkabel ein und öffne die im Seriellen Monitor angezeigte Adresse (z. B. `http://192.168.178.50`).
   * **Über den Einrichtungs-Hotspot:** Ist kein Kabel gesteckt und kein WLAN verbunden, verbinde dein Smartphone oder deinen Laptop mit dem WLAN **`Bonbridge-Setup`** (ohne Passwort) und öffne im Browser **`http://192.168.4.1`**.

4. **WLAN suchen & verbinden:**
   * Klicke neben dem Feld *WLAN Name* auf **`🔍 Suchen`**.
   * Nach ein bis zwei Sekunden erscheint eine Liste aller gefundenen Funknetze mit Empfangsstärke und Schlosssymbol.
   * Klicke dein Netzwerk in der Liste an – der Name wird automatisch ins Textfeld eingetragen.
   * Gib dein WLAN-Passwort ein (mit dem **`👁️`**-Symbol kannst du kontrollieren, ob alles richtig geschrieben ist).
   * Klicke auf **`Mit diesem WLAN verbinden`**:
     * Der Adapter stellt nun sofort die Verbindung her.
     * Nach wenigen Sekunden erhältst du direkt im Browser die Erfolgsmeldung mit der neuen IP-Adresse und einem anklickbaren Link!
     * Falls das Passwort falsch war oder der Empfang abbricht, wird dir der Grund sofort in verständlichem Deutsch angezeigt.
   * **Drucktest & Kassenlade:** Über die Schnell-Test-Knöpfe kannst du den Drucker und die Geldschublade direkt ausprobieren.

5. **Im Kassensystem einrichten:**
   * Wähle in deiner Kassen-App (z. B. Kassensoftware) **Netzwerk-Drucker / ESC-POS**.
   * Gib die vergebene **IP-Adresse** des BonbridgeESP32 ein.
   * Gib als Port **9100** ein.
   * Fertig! Ab sofort druckt deine Kasse zuverlässig über den Adapter.

6. **Firmware drahtlos aktualisieren & Downgrades durchführen (OTA):**
   * Rufe die Weboberfläche im Browser auf.
   * Scrolle nach unten zum Bereich **Firmware-Aktualisierung (Funk-Update / OTA)**.
   * **Versionen prüfen:** Klicke auf `🔍 Nach Versionen auf GitHub suchen`.
   * **Auswählen & Installieren:**
     * Es erscheint eine Liste aller freigegebenen Versionen aus dem GitHub-Projekt.
     * Wähle eine **neuere Version** aus, um das System zu aktualisieren.
     * Wähle eine **ältere Version** aus, falls du auf einen vorherigen Softwarestand zurückspringen möchtest (Downgrade).
     * Der Knopf passt sich automatisch an (z. B. `Auf v1.2.0 aktualisieren` oder `Auf v1.0.0 zurückstufen (Downgrade)`).
   * **Manuelle Datei:** Über `Datei auswählen` kannst du weiterhin jederzeit eine eigene `.bin`-Datei vom Computer hochladen.
   * Während der Übertragung siehst du einen Echtzeit-Ladebalken. Alle gespeicherten WLAN- und Druckeinstellungen bleiben vollständig erhalten.
