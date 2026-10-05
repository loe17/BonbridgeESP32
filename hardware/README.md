# BonbridgeESP32 - Eigene Hardware-Entwicklung (KiCad)

Vollständige Anleitung und Projektdaten für eine maßgeschneiderte All-in-One-Platine für den Festeinbau in den **Epson TM-T88 Bondrucker**.

---

## 1. Spezifikationen & Mechanik

* **Abmessungen:** Exakt **55,0 mm × 25,0 mm** (mit abgerundeten Ecken $R = 2,0\text{ mm}$).
* **Befestigung:** 4× M2,5-Montagebohrungen (Bohrung 2,7 mm, Kupfer-Pad 4,8 mm, mit Masse/GND verbunden).
* **Ausrichtung & Platzierung der Anschlüsse:**
  * **Obere Längsseite (55 mm Kante, oben links):**
    * **JST-XH 2-Pin (J3):** 24V Stromeingang vom internen Druckernetzteil (+24V, GND).
    * **JST-XH 4-Pin (J4):** Direkt daneben: 4-Pin USB-Verbindung zum Drucker-Mainboard (+5V, D-, D+, GND).
  * **Obere Längsseite (55 mm Kante, oben rechts):**
    * **USB-C Buchse (J2):** Zum Programmieren/Flashen und Diagnose (öffnet nach oben).
  * **Untere Längsseite (55 mm Kante, unten links/mitte):**
    * **RJ45 MagJack (J1):** Netzwerkbuchse (10/100Mbit, öffnet nach unten, gegenüber von USB-C).
* **Externe Antenne:** Anschluss über integrierte **U.FL / IPEX Buchse** direkt auf dem ESP32-S3-WROOM-1U Modul.

---

## 2. Abbildung: Platinen-Layout (55 mm × 25 mm)

```text
       ┌─── 55 mm Längsseite (Oben) ──────────────────────────────────────┐
       │ (H1)    [J3] 24V IN     [J4] PRINTER USB            [J2]   (H2)  │
       │ M2.5    [+24V GND]     [+5V D- D+ GND]             USB-C   M2.5  │
       │                                                    PROG          │
       │                                     ┌─────────────┐              │
       │ ┌──────────────┐    ┌───────────┐   │  ESP32-S3   │              │
       │ │              │    │   W5500   │   │  WROOM-1U   │    [D1]      │
       │ │ RJ45 MagJack │    │  QFN-48   │   │             │   WS2812B    │
       │ │ (10/100Mbit) │    ├───────────┤   │ (IPEX Ant.) │     RGB      │
       │ │              │    │  TPS54302 │   │             │              │
       │ └──────────────┘    │ Step-Down │   └─────────────┘        (H4)  │
       │ (H3)                └───────────┘                          M2.5  │
       └─▼──▼──▼──▼───────────────────────────────────────────────────────┘
          RJ45 Buchse öffnet nach UNTEN (Längsseite)
```

---

## 3. Bauteilauswahl & Stückliste (BOM)

| Komponente | Bauteil / Typ | Gehäuse | Funktion & Begründung |
|---|---|---|---|
| **U1 (MCU)** | **ESP32-S3-WROOM-1U-N8** | SMD-Modul (18×19,2 mm) | **Mit U.FL/IPEX-Buchse für externe Antenne.** Spart 6 mm Baulänge gegenüber der PCB-Antennen-Version! Voll CE/FCC-zertifiziert. |
| **U2 (Ethernet)** | **WIZnet W5500** | QFN-48 (7×7 mm) | Hardware-TCP/IP-Chip. Wird über SPI mit dem ESP32 verbunden. |
| **J1 (LAN-Buchse)**| **RJ45 MagJack (10/100M)** | THT (z.B. HanRun HR911105A / Amphenol) | Buchse mit **integrierten Übertragern** und Status-LEDs (Link/Act). Auf der unteren Längsseite platziert. |
| **J2 (USB-C)** | **USB-C 16-Pin Receptacle** | SMD/THT Hybrid (z.B. GCT USB4085) | Auf oberer Längsseite gegenüber RJ45. Zum Programmieren, Flashen und Auslesen des Seriellen Monitors am PC. |
| **J3 (24V In)** | **JST-XH 2-Pin (2,50 mm)** | THT vertikal | Oben links: Stromeingang direkt von der 24V-Schiene des Epson-Druckers. |
| **J4 (Drucker-USB)**| **JST-XH 4-Pin (2,50 mm)** | THT vertikal | Oben links neben J3: Führt 5V, D-, D+, GND direkt zu den 4 USB-Pins des Epson TM-T88. |
| **U3 (Step-Down)**| **TI TPS54302** oder **MP1584** | SOT-23-6 / SOIC-8 | Wandelt 24V DC hocheffizient (Buck) auf **saubere 5,0V (bis 3A)** für ESP32 und Drucker-USB. |
| **U4 (3.3V LDO)** | **AP2112K-3.3** oder **AMS1117-3.3** | SOT-23-5 / SOT-223 | Erzeugt saubere 3,3V (bis 600 mA / 1 A) für ESP32-S3 und W5500. |
| **D1 (Status-LED)**| **WS2812B RGB** | SMD 5050 | Adressierbare RGB-Status-LED (Puls Grün = Online, Blau = Setup, Orange = Suche). |
| **Y1 (Quarz)** | **25.000 MHz Quarz (18pF)** | SMD 3225 | Taktgeber für den W5500. |
| **D2 (Schottky)** | **B5819W** oder **SS14** | SOD-123 / SMA | Verhindert Rückspeisung zwischen USB-C VBUS und 24V-Step-Down. |

---

## 4. Verdrahtungs- & Pinbelegung

### 4.1 ESP32-S3 zu W5500 (SPI-Schnittstelle)
* `GPIO 10` ➔ `W5500 SCLK` (Takt)
* `GPIO 11` ➔ `W5500 MOSI` (Daten zum Chip)
* `GPIO 12` ➔ `W5500 MISO` (Daten vom Chip)
* `GPIO 9`  ➔ `W5500 CS` (Chip Select)
* `GPIO 14` ➔ `W5500 INT` (Interrupt)
* `GPIO 13` ➔ `W5500 RST` (Hardware-Reset)

### 4.2 ESP32-S3 zum Drucker-Mainboard (JST-XH 4-Pin - J4)
* `Pin 1` ➔ `+5,0V` (Ausgang vom Step-Down-Wandler)
* `Pin 2` ➔ `GPIO 19` (USB D-)
* `Pin 3` ➔ `GPIO 20` (USB D+)
* `Pin 4` ➔ `GND` (Gemeinsame Masse)

### 4.3 USB-C Programmierbuchse (J2)
* `A6 / B6` (D+) ➔ `GPIO 20`
* `A7 / B7` (D-) ➔ `GPIO 19`
* `CC1` ➔ 5,1 kΩ Widerstand nach GND
* `CC2` ➔ 5,1 kΩ Widerstand nach GND
* `VBUS` ➔ Anode von Schottky-Diode D2 ➔ Kathode an +5V-Schiene

### 4.4 Status-LED
* `GPIO 48` ➔ DIN der WS2812B RGB-LED

---

## 5. Das KiCad-Projekt verwenden

Die Projektdateien befinden sich direkt in diesem Ordner:
* **`BonbridgeESP32_CustomBoard.kicad_pro`**: KiCad 7 / 8 Projektdatei.
* **`BonbridgeESP32_CustomBoard.kicad_pcb`**: Vollständig vordefiniertes Board-Layout mit exakter Außenkontur (55×25 mm), Bohrungen und Footprint-Platzierung.
* **`generate_kicad_project.py`**: Python-Generatorskript, das das Board jederzeit neu erzeugen oder anpassen kann.

### Schritte in KiCad:
1. Starte **KiCad** und öffne `BonbridgeESP32_CustomBoard.kicad_pro`.
2. Öffne den **Leiterplatten-Editor (PCB)**:
   * Die 55 mm × 25 mm Kontur mit abgerundeten Ecken ist bereits fertig gezeichnet.
   * Die 4 Befestigungslöcher (M2.5) sind exakt platziert.
   * RJ45 befindet sich an der unteren Längsseite, USB-C gegenüber an der oberen Längsseite, und beide JST-Stecker sitzen links oben nebeneinander.
3. **Leiterbahnen routen:**
   * **Empfehlung:** 4-Lagen-Platine (z. B. bei JLCPCB oder Aisler für wenige Euro):
     * *Lage 1 (Top):* Signale & Bauteile
     * *Lage 2 (In1):* Durchgehende GND-Massefläche
     * *Lage 3 (In2):* 3,3V und 5,0V Stromversorgungs-Flächen
     * *Lage 4 (Bottom):* Signale & Masse
   * Die USB-Leitungen (D+ und D-) parallel als 90-Ohm-Differenzialpaar führen.
4. **Gerber-Dateien exportieren** und bei einem Platinenhersteller bestellen.
