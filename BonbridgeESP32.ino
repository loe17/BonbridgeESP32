#include <Arduino.h>
#include "config.h"
#include "usb_printer.h"
#include "net_manager.h"
#include "raw_server.h"
#include "web_ui.h"
#include "netwatch.h"
#include <esp_log.h>

static const char* TAG = "Main";

// Pin für die Status-LED (auf vielen ESP32-S3 Super Mini Boards GPIO 48, 47 oder 21)
#ifndef STATUS_LED_PIN
#ifdef LED_BUILTIN
#define STATUS_LED_PIN LED_BUILTIN
#else
#define STATUS_LED_PIN 48
#endif
#endif

static unsigned long lastHeartbeatMs = 0;
static bool ledState = false;

void setup() {
    Serial.begin(115200);

    // Bis zu 1,5 Sekunden warten, falls der USB-Serielle-Monitor verbunden wird
    unsigned long startWait = millis();
    while (!Serial && (millis() - startWait < 1500)) {
        delay(10);
    }

    pinMode(STATUS_LED_PIN, OUTPUT);
    digitalWrite(STATUS_LED_PIN, LOW);

    Serial.println();
    Serial.println("==================================================");
    Serial.println("        BonbridgeESP32 - Drucker-Adapter          ");
    Serial.println("==================================================");
    Serial.println("[START] Lade Konfiguration aus dem Speicher...");

    // 1. Einstellungen aus dem Speicher laden
    ConfigManager::instance().begin();
    AppConfig& conf = ConfigManager::instance().get();

    Serial.println("[START] Starte USB-Host fuer Bondrucker...");
    // 2. USB-Host für ESC/POS Bondrucker starten
    if (!UsbPrinter::instance().begin()) {
        Serial.println("[FEHLER] Konnte USB-Host nicht initialisieren!");
    }

    Serial.println("[START] Initialisiere Netzwerk (LAN & WLAN)...");
    // 3. Netzwerk-Steuerung starten (W5500 LAN hat Vorrang, WLAN als Ersatz)
    NetManager::instance().begin();

    // 4. Port 9100 RAW Server starten (Nimmt Druckdaten vom Kassensystem an)
    RawServer::instance().begin(conf.port9100);

    // 5. Sparsame Web-Oberfläche starten
    WebUI::instance().begin(80);

    // 6. Netzwerk-Wächter aktivieren
    NetWatch::instance().begin();

    Serial.println("[START] System bereit. Warte auf Netzwerk und Drucker...");
    Serial.println("==================================================");
    Serial.println();
}

void loop() {
    // 1. Optischer Herzschlag der Status-LED (blinkt jede Sekunde einmal)
    unsigned long now = millis();
    if (now - lastHeartbeatMs >= 1000) {
        lastHeartbeatMs = now;
        ledState = !ledState;
        digitalWrite(STATUS_LED_PIN, ledState ? HIGH : LOW);
    }

    // 2. Netzwerk-Verbindung überwachen & umschalten
    NetManager::instance().update();

    // 3. Port 9100 Daten-Tunnel verarbeiten (direktes Streaming an USB)
    RawServer::instance().update();

    // 4. Web-Oberfläche abfragen (0% CPU bei Inaktivität)
    WebUI::instance().update();

    // 5. Auf Ausfall des Netzwerks prüfen
    NetWatch::instance().update();

    // Kurzer Yield für FreeRTOS
    delay(1);
}
