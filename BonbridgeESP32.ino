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

#include <driver/usb_serial_jtag.h>

static const char* TAG = "Main";

static unsigned long lastHeartbeatMs = 0;
static bool ledState = false;

// Status-LED Ansteuerung (WS2812 RGB an GPIO 48 / 38 auf ESP32-S3)
static void setStatusLed(bool heartbeat) {
    if (NetManager::instance().isWifiConnected() || NetManager::instance().isEthernetLinkUp()) {
        // Online: Sanftes, klares Gruen mit Herzschlag-Puls
        if (heartbeat) {
            rgbLedWrite(48, 0, 180, 40);
            rgbLedWrite(38, 0, 180, 40);
        } else {
            rgbLedWrite(48, 0, 25, 5);
            rgbLedWrite(38, 0, 25, 5);
        }
    } else if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA) {
        // Hotspot 'Bonbridge-Setup' aktiv: Blau
        if (heartbeat) {
            rgbLedWrite(48, 0, 50, 220);
            rgbLedWrite(38, 0, 50, 220);
        } else {
            rgbLedWrite(48, 0, 5, 40);
            rgbLedWrite(38, 0, 5, 40);
        }
    } else {
        // Verbindung wird gesucht / Offline: Gelb/Orange blinken
        if (heartbeat) {
            rgbLedWrite(48, 180, 80, 0);
            rgbLedWrite(38, 180, 80, 0);
        } else {
            rgbLedWrite(48, 0, 0, 0);
            rgbLedWrite(38, 0, 0, 0);
        }
    }
}

void setup() {
    // 0. Hardware-Brownout-Reset entschaerfen (verhindert Endlos-Reboot-Schleifen bei 
    // kurzzeitigen Millisekunden-Spannungseinbruechen waehrend WLAN-Funkspitzen)
    RTCCNTL.brown_out.rst_ena = 0;
    RTCCNTL.brown_out.ana_rst_en = 0;

    // 1. Serielle Schnittstelle ohne Blockieren (Timeout = 0)
    // Wenn das Board an einem 5V-Netzteil betrieben wird (ohne PC), darf Serial.print
    // niemals auf einen Computer warten oder das System einfrieren!
#if ARDUINO_USB_CDC_ON_BOOT
    Serial.setTxTimeoutMs(0);
#endif
    Serial.begin(115200);
#if ARDUINO_USB_CDC_ON_BOOT
    Serial.setTxTimeoutMs(0);
#endif

    // Startgrund fuer Diagnose und WebUI erfassen
    esp_reset_reason_t resetReason = esp_reset_reason();
    WebUI::instance().setResetReason((uint8_t)resetReason);

#if ARDUINO_USB_CDC_ON_BOOT
    // Nur auf den seriellen Monitor warten, wenn tatsaechlich ein PC am USB-Port angeschlossen ist
    if (usb_serial_jtag_is_connected()) {
        unsigned long startWait = millis();
        while (!Serial && (millis() - startWait < 500)) {
            delay(10);
        }
    }
#endif

    // Sofortige optische Bestaetigung beim Einschalten:
    // Violett signalisiert sofort: Strom ist da, ESP32-Prozessor laeuft!
    rgbLedWrite(48, 120, 0, 120);
    rgbLedWrite(38, 120, 0, 120);

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

    // 2. Einstellungen aus dem Speicher laden
    ConfigManager::instance().begin();
    AppConfig& conf = ConfigManager::instance().get();

    Serial.println("[START] Initialisiere Netzwerk (LAN & WLAN)...");
    // 3. Netzwerk-Steuerung starten (W5500 LAN hat Vorrang, WLAN als Ersatz)
    NetManager::instance().begin();

    // 4. Port 9100 RAW Server starten (Nimmt Druckdaten vom Kassensystem an)
    RawServer::instance().begin(conf.port9100);

    // 5. Sparsame Web-Oberfläche starten
    WebUI::instance().begin(80);

    // 6. Netzwerk-Wächter aktivieren
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
    // oder der Einrichtungs-Hotspot aktiv ist.
    // So kann die WLAN-Verbindung und DHCP-Zuweisung absolut ungestoert ablaufen!
    if (NetManager::instance().isOnline()) {
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
