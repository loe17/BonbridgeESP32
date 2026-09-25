#include <Arduino.h>
#include "config.h"
#include "usb_printer.h"
#include "net_manager.h"
#include "raw_server.h"
#include "web_ui.h"
#include "netwatch.h"
#include <esp_log.h>

static const char* TAG = "Main";

void setup() {
    Serial.begin(115200);
    delay(500);

    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "   BonbridgeESP32 - Drucker-Adapter     ");
    ESP_LOGI(TAG, "========================================");

    // 1. Einstellungen aus dem Speicher laden
    ConfigManager::instance().begin();
    AppConfig& conf = ConfigManager::instance().get();

    // 2. USB-Host für ESC/POS Bondrucker starten
    if (!UsbPrinter::instance().begin()) {
        ESP_LOGE(TAG, "Konnte USB-Host nicht initialisieren!");
    }

    // 3. Netzwerk-Steuerung starten (W5500 LAN hat Vorrang, WLAN als Ersatz)
    NetManager::instance().begin();

    // 4. Port 9100 RAW Server starten (Nimmt Druckdaten vom Kassensystem an)
    RawServer::instance().begin(conf.port9100);

    // 5. Sparsame Web-Oberfläche starten
    WebUI::instance().begin(80);

    // 6. Netzwerk-Wächter aktivieren
    NetWatch::instance().begin();

    ESP_LOGI(TAG, "System erfolgreich gestartet und bereit.");
}

void loop() {
    // 1. Netzwerk-Verbindung überwachen & umschalten
    NetManager::instance().update();

    // 2. Port 9100 Daten-Tunnel verarbeiten (direktes Streaming an USB)
    RawServer::instance().update();

    // 3. Web-Oberfläche abfragen (0% CPU bei Inaktivität)
    WebUI::instance().update();

    // 4. Auf Ausfall des Netzwerks prüfen
    NetWatch::instance().update();

    // Kurzer Yield für FreeRTOS
    delay(1);
}
