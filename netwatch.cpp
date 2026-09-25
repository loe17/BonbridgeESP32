#include "netwatch.h"
#include "net_manager.h"
#include "usb_printer.h"
#include "config.h"
#include <esp_log.h>

static const char* TAG = "NetWatch";

// Wartezeit nach dem Start, damit DHCP genügend Zeit hat (15 Sekunden)
#define BOOT_GRACE_PERIOD_MS 15000
// Entprellzeit bei Verbindungsabbruch (3 Sekunden), um kurze Flackerer zu ignorieren
#define OFFLINE_DEBOUNCE_MS  3000

NetWatch::NetWatch() :
    wasOnline(false),
    alertPrinted(false),
    bootTimeMs(0),
    offlineSinceMs(0),
    offlineDetected(false)
{
}

NetWatch& NetWatch::instance() {
    static NetWatch inst;
    return inst;
}

void NetWatch::begin() {
    bootTimeMs = millis();
    wasOnline = false;
    alertPrinted = false;
    offlineDetected = false;
    ESP_LOGI(TAG, "Netzwerk-Waechter aktiv");
}

void NetWatch::update() {
    unsigned long now = millis();

    // In den ersten Sekunden nach dem Einschalten keinen Fehlalarm auslösen
    if (now - bootTimeMs < BOOT_GRACE_PERIOD_MS) {
        if (NetManager::instance().isOnline()) {
            wasOnline = true;
        }
        return;
    }

    bool currentlyOnline = NetManager::instance().isOnline();

    if (currentlyOnline) {
        if (!wasOnline) {
            ESP_LOGI(TAG, "Netzwerkverbindung wiederhergestellt!");
        }
        wasOnline = true;
        alertPrinted = false;
        offlineDetected = false;
    } else {
        // Wir waren zuvor online, nun ist die Verbindung weg
        if (wasOnline) {
            if (!offlineDetected) {
                offlineDetected = true;
                offlineSinceMs = now;
            } else if (!alertPrinted && (now - offlineSinceMs >= OFFLINE_DEBOUNCE_MS)) {
                // Entprellzeit abgelaufen -> Warnung ausgeben
                AppConfig& conf = ConfigManager::instance().get();
                if (conf.netwatch_enabled) {
                    ESP_LOGW(TAG, "Netzwerkausfall bestaetigt! Drucke Warnhinweis.");
                    UsbPrinter::instance().printNetworkAlert(
                        "Netzwerkverbindung unterbrochen!",
                        "Weder LAN-Kabel noch WLAN sind erreichbar."
                    );
                } else {
                    ESP_LOGI(TAG, "Netzwerkausfall erkannt, aber Warnbon in Einstellungen deaktiviert.");
                }
                alertPrinted = true;
                wasOnline = false;
            }
        }
    }
}

bool NetWatch::hasAlertBeenPrinted() {
    return alertPrinted;
}
