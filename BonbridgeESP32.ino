#include <Arduino.h>
#include "config.h"
#include "usb_printer.h"
#include "net_manager.h"
#include "raw_server.h"
#include "web_ui.h"
#include "netwatch.h"
#include "ota_updater.h"
#include <esp_log.h>
#include <esp_system.h>
#include "soc/rtc_cntl_struct.h"

static const char* TAG = "Main";

static unsigned long lastHeartbeatMs = 0;
static bool ledState = false;

static void setStatusLed(bool on) {
    if (on) {
        rgbLedWrite(48, 0, 255, 200); // Helles Cyan fuer WS2812 an GPIO 48
    } else {
        rgbLedWrite(48, 0, 0, 0);     // Aus
    }

    // Falls es eine normale LED ist (verschiedene Pins getestet)
    digitalWrite(48, on ? HIGH : LOW);
    digitalWrite(47, on ? HIGH : LOW);
    digitalWrite(21, on ? HIGH : LOW);
    digitalWrite(8, on ? HIGH : LOW);
    digitalWrite(1, on ? HIGH : LOW);
}

void setup() {
    // 0. Hardware-Brownout-Reset entschaerfen (verhindert Endlos-Reboot-Schleifen bei 
    // kurzzeitigen Millisekunden-Spannungseinbruechen waehrend WLAN-Funkspitzen)
    RTCCNTL.brown_out.rst_ena = 0;
    RTCCNTL.brown_out.ana_rst_en = 0;

    Serial.begin(115200);

    // Startgrund fuer Diagnose und WebUI erfassen
    esp_reset_reason_t resetReason = esp_reset_reason();
    WebUI::instance().setResetReason((uint8_t)resetReason);

    // Bis zu 2 Sekunden warten, falls der USB-Serielle-Monitor verbunden wird
    unsigned long startWait = millis();
    while (!Serial && (millis() - startWait < 2000)) {
        delay(10);
    }

    pinMode(48, OUTPUT);
    pinMode(47, OUTPUT);
    pinMode(21, OUTPUT);
    pinMode(8, OUTPUT);
    pinMode(1, OUTPUT);

    // Deutliches Farbsignal beim Einschalten (Rot -> Gruen -> Blau)
    rgbLedWrite(48, 255, 0, 0); delay(120);
    rgbLedWrite(48, 0, 255, 0); delay(120);
    rgbLedWrite(48, 0, 0, 255); delay(120);
    setStatusLed(false);

    Serial.println();
    Serial.println("==================================================");
    Serial.println("        BonbridgeESP32 - Drucker-Adapter          ");
    Serial.println("==================================================");
    Serial.print("[START] Start-Grund: ");
    switch (resetReason) {
        case ESP_RST_POWERON:   Serial.println("Normal (Einschalten / Power On)"); break;
        case ESP_RST_EXT:       Serial.println("Externer Reset-Pin"); break;
        case ESP_RST_SW:        Serial.println("Software-Neustart"); break;
        case ESP_RST_PANIC:     Serial.println("WARNUNG: Absturz / Panic-Reset!"); break;
        case ESP_RST_BROWNOUT:  Serial.println("WARNUNG: Brownout! Spannungseinbruch an 5V Stromversorgung!"); break;
        default:                Serial.printf("Code %d\n", (int)resetReason); break;
    }
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

    // 3. USB-Drucker Status überwachen:
    // USB-Host wird erst gestartet, wenn die Netzwerk-Verbindung (WLAN oder LAN) steht
    // oder der Einrichtungs-Hotspot aktiv ist (bzw. 15s Sicherheits-Fallback).
    // So kann die WLAN-Verbindung und DHCP-Zuweisung absolut ungestoert ablaufen!
    if (NetManager::instance().isOnline() || now > 15000) {
        UsbPrinter::instance().update();
    }

    // 4. Port 9100 Daten-Tunnel verarbeiten (direktes Streaming an USB)
    RawServer::instance().update();

    // 5. Web-Oberfläche abfragen (0% CPU bei Inaktivität)
    WebUI::instance().update();

    // 6. Auf Ausfall des Netzwerks prüfen
    NetWatch::instance().update();

    // 7. OTA Firmware-Update Status & Neustart verarbeiten
    OtaUpdater::instance().update();

    // Kurzer Yield für FreeRTOS
    delay(1);
}
