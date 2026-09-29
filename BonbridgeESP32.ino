#include <Arduino.h>
#include "config.h"
#include "usb_printer.h"
#include "net_manager.h"
#include "raw_server.h"
#include "web_ui.h"
#include "netwatch.h"
#include <esp_log.h>

static const char* TAG = "Main";

// Pin für die Status-LED (auf ESP32-S3 Super Mini meist GPIO 48 WS2812 RGB-LED)
#ifndef STATUS_LED_PIN
#ifdef RGB_BUILTIN
#define STATUS_LED_PIN RGB_BUILTIN
#elif defined(LED_BUILTIN)
#define STATUS_LED_PIN LED_BUILTIN
#else
#define STATUS_LED_PIN 48
#endif
#endif

static unsigned long lastHeartbeatMs = 0;
static bool ledState = false;

static void setStatusLed(bool on) {
    digitalWrite(STATUS_LED_PIN, on ? HIGH : LOW);
#ifdef RGB_BUILTIN
    rgbLedWrite(RGB_BUILTIN, 0, on ? 32 : 0, on ? 16 : 0);
#endif
    rgbLedWrite(STATUS_LED_PIN, 0, on ? 32 : 0, on ? 16 : 0);
}

void setup() {
    Serial.begin(115200);

    // Bis zu 2 Sekunden warten, falls der USB-Serielle-Monitor verbunden wird
    unsigned long startWait = millis();
    while (!Serial && (millis() - startWait < 2000)) {
        delay(10);
    }

    pinMode(STATUS_LED_PIN, OUTPUT);
    // Sofortiges optisches Lebenszeichen: 3x schnelles Aufblitzen
    setStatusLed(true); delay(80); setStatusLed(false); delay(80);
    setStatusLed(true); delay(80); setStatusLed(false); delay(80);
    setStatusLed(true); delay(80); setStatusLed(false);

    Serial.println();
    Serial.println("==================================================");
    Serial.println("        BonbridgeESP32 - Drucker-Adapter          ");
    Serial.println("==================================================");
    Serial.println("[START] Lade Konfiguration aus dem Speicher...");

    // 1. Einstellungen aus dem Speicher laden
    ConfigManager::instance().begin();
    AppConfig& conf = ConfigManager::instance().get();

    Serial.println("[START] Initialisiere Netzwerk (LAN & WLAN)...");
    // 2. Netzwerk-Steuerung starten (W5500 LAN hat Vorrang, WLAN als Ersatz)
    NetManager::instance().begin();

    // 3. Port 9100 RAW Server starten (Nimmt Druckdaten vom Kassensystem an)
    RawServer::instance().begin(conf.port9100);

    // 4. Sparsame Web-Oberfläche starten
    WebUI::instance().begin(80);

    // 5. Netzwerk-Wächter aktivieren
    NetWatch::instance().begin();

    // 6. USB-Host für ESC/POS Bondrucker (nach Netzwerk starten)
    Serial.println("[START] Initialisiere USB-Schnittstelle...");
    UsbPrinter::instance().begin();

    Serial.println("[START] System betriebsbereit.");
    Serial.println("==================================================");
    Serial.println();
}

void loop() {
    // 1. Optischer Herzschlag der Status-LED (blinkt jede Sekunde einmal)
    unsigned long now = millis();
    if (now - lastHeartbeatMs >= 1000) {
        lastHeartbeatMs = now;
        ledState = !ledState;
        setStatusLed(ledState);
    }

    // 2. Netzwerk-Verbindung überwachen & umschalten
    NetManager::instance().update();

    // 3. USB-Drucker Status überwachen
    UsbPrinter::instance().update();

    // 4. Port 9100 Daten-Tunnel verarbeiten (direktes Streaming an USB)
    RawServer::instance().update();

    // 5. Web-Oberfläche abfragen (0% CPU bei Inaktivität)
    WebUI::instance().update();

    // 6. Auf Ausfall des Netzwerks prüfen
    NetWatch::instance().update();

    // Kurzer Yield für FreeRTOS
    delay(1);
}
