#include "net_manager.h"
#include <esp_log.h>
#include <ETH.h>

static const char* TAG = "NetManager";

static NetManager* s_instance = nullptr;

static void onWiFiEthEvent(WiFiEvent_t event) {
    switch (event) {
        case ARDUINO_EVENT_ETH_START:
            ESP_LOGI(TAG, "ETH Gestartet");
            ETH.setHostname("BonbridgeESP32");
            break;
        case ARDUINO_EVENT_ETH_CONNECTED:
            ESP_LOGI(TAG, "ETH Kabel eingesteckt (Link Up)");
            break;
        case ARDUINO_EVENT_ETH_GOT_IP:
            ESP_LOGI(TAG, "ETH IP erhalten: %s", ETH.localIP().toString().c_str());
            break;
        case ARDUINO_EVENT_ETH_DISCONNECTED:
            ESP_LOGW(TAG, "ETH Kabel abgezogen (Link Down)");
            break;
        case ARDUINO_EVENT_ETH_STOP:
            ESP_LOGI(TAG, "ETH Gestoppt");
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            ESP_LOGI(TAG, "WLAN IP erhalten: %s", WiFi.localIP().toString().c_str());
            break;
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            ESP_LOGW(TAG, "WLAN getrennt");
            break;
        default:
            break;
    }
}

NetManager::NetManager() :
    currentMode(NET_MODE_NONE),
    ethLinkUp(false),
    wifiConnected(false),
    lastCheckMs(0),
    wifiConnectStartMs(0),
    wifiAttemptActive(false)
{
    s_instance = this;
}

NetManager& NetManager::instance() {
    static NetManager inst;
    return inst;
}

void NetManager::begin() {
    WiFi.onEvent(onWiFiEthEvent);

    initEthernet();

    // Kurze Pause, um W5500 Link Zeit zu geben
    delay(200);

    checkConnections();

    // Falls kein Ethernet-Kabel steckt, direkt WLAN aktivieren
    if (!ethLinkUp) {
        startWifi();
    }
}

void NetManager::initEthernet() {
    ESP_LOGI(TAG, "Initialisiere W5500 SPI Ethernet...");
    SPI.begin(DEFAULT_ETH_SCLK, DEFAULT_ETH_MISO, DEFAULT_ETH_MOSI);

    // Initialisierung des W5500 SPI Ethernet Treibers
    #if defined(ETH_PHY_W5500)
    ETH.begin(ETH_PHY_W5500, 1, DEFAULT_ETH_CS, DEFAULT_ETH_INT, DEFAULT_ETH_RST, SPI2_HOST);
    #else
    // Fallback falls Kern-Definition abweicht
    ETH.begin();
    #endif
}

void NetManager::startWifi() {
    AppConfig& conf = ConfigManager::instance().get();
    if (conf.wifi_ssid.length() == 0) {
        ESP_LOGI(TAG, "Keine WLAN-Zugangsdaten hinterlegt.");
        stopWifi();
        return;
    }

    if (wifiAttemptActive && WiFi.status() == WL_CONNECTED) {
        return; // Bereits verbunden
    }

    ESP_LOGI(TAG, "Aktiviere WLAN und verbinde mit SSID: %s", conf.wifi_ssid.c_str());
    WiFi.mode(WIFI_STA);
    WiFi.setHostname("BonbridgeESP32");
    WiFi.begin(conf.wifi_ssid.c_str(), conf.wifi_password.c_str());
    wifiAttemptActive = true;
    wifiConnectStartMs = millis();
}

void NetManager::stopWifi() {
    if (WiFi.getMode() != WIFI_OFF) {
        ESP_LOGI(TAG, "Deaktiviere WLAN (Kabelverbindung aktiv oder keine Zugangsdaten)");
        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
        wifiAttemptActive = false;
        wifiConnected = false;
    }
}

void NetManager::reloadWifiConfig() {
    if (!ethLinkUp) {
        stopWifi();
        startWifi();
    }
}

void NetManager::checkConnections() {
    // Prüfe Link-Status des W5500
    bool currentEthLink = ETH.linkUp();

    if (currentEthLink != ethLinkUp) {
        ethLinkUp = currentEthLink;
        if (ethLinkUp) {
            ESP_LOGI(TAG, "LAN-Kabel erkannt! Schalte WLAN ab.");
            stopWifi();
            currentMode = NET_MODE_ETHERNET;
        } else {
            ESP_LOGW(TAG, "LAN-Kabel verloren! Versuche WLAN zu aktivieren...");
            startWifi();
        }
    }

    // Wenn LAN aktiv ist
    if (ethLinkUp) {
        currentMode = NET_MODE_ETHERNET;
        return;
    }

    // Wenn kein LAN da ist, prüfen wir den WLAN-Status
    if (WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
        currentMode = NET_MODE_WIFI;
    } else {
        wifiConnected = false;
        currentMode = NET_MODE_NONE;
    }
}

void NetManager::update() {
    unsigned long now = millis();
    if (now - lastCheckMs >= 1000) {
        lastCheckMs = now;
        checkConnections();
    }
}

NetActiveMode NetManager::getActiveMode() {
    return currentMode;
}

String NetManager::getActiveDescription() {
    switch (currentMode) {
        case NET_MODE_ETHERNET:
            return "LAN-Kabel aktiv (WLAN ist aus)";
        case NET_MODE_WIFI: {
            int rssi = WiFi.RSSI();
            return "WLAN aktiv (" + WiFi.SSID() + ", Signal: " + String(rssi) + " dBm)";
        }
        case NET_MODE_NONE:
        default:
            if (wifiAttemptActive) {
                return "Keine Verbindung (Verbinde mit WLAN...)";
            }
            return "Keine Verbindung (Kein LAN-Kabel, WLAN aus)";
    }
}

String NetManager::getIpAddress() {
    if (currentMode == NET_MODE_ETHERNET) {
        return ETH.localIP().toString();
    } else if (currentMode == NET_MODE_WIFI) {
        return WiFi.localIP().toString();
    }
    return "0.0.0.0 (Nicht verbunden)";
}

String NetManager::getMacAddress() {
    if (currentMode == NET_MODE_ETHERNET) {
        return ETH.macAddress();
    }
    return WiFi.macAddress();
}

bool NetManager::isOnline() {
    if (currentMode == NET_MODE_ETHERNET && ETH.linkUp()) {
        return ETH.localIP() != IPAddress(0, 0, 0, 0);
    }
    if (currentMode == NET_MODE_WIFI && WiFi.status() == WL_CONNECTED) {
        return WiFi.localIP() != IPAddress(0, 0, 0, 0);
    }
    return false;
}

bool NetManager::isEthernetLinkUp() {
    return ethLinkUp;
}

bool NetManager::isWifiConnected() {
    return wifiConnected;
}
