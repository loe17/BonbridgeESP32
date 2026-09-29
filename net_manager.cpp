#include "net_manager.h"
#include <esp_log.h>
#include <ETH.h>

static const char* TAG = "NetManager";

static NetManager* s_instance = nullptr;

static void onWiFiEthEvent(WiFiEvent_t event) {
    switch (event) {
        case ARDUINO_EVENT_ETH_START:
            Serial.println("[NETZWERK] LAN-Treiber (W5500) gestartet.");
            ETH.setHostname("BonbridgeESP32");
            break;
        case ARDUINO_EVENT_ETH_CONNECTED:
            Serial.println("[NETZWERK] LAN-Kabel eingesteckt (Link Up) - warte auf IP-Adresse...");
            break;
        case ARDUINO_EVENT_ETH_GOT_IP:
            Serial.println();
            Serial.println("--------------------------------------------------");
            Serial.println("[VERBINDUNG HERGESTELLT - LAN-KABEL]");
            Serial.println("  Schnittstelle      : LAN-Kabel aktiv (WLAN ist aus)");
            Serial.print("  IP-Adresse         : "); Serial.println(ETH.localIP());
            Serial.print("  Subnetzmaske       : "); Serial.println(ETH.subnetMask());
            Serial.print("  Gateway            : "); Serial.println(ETH.gatewayIP());
            Serial.print("  Web-Oberflaeche    : http://"); Serial.println(ETH.localIP());
            Serial.print("  Kassen-Drucker-Port: "); Serial.print(ETH.localIP()); Serial.println(":9100");
            Serial.println("--------------------------------------------------");
            Serial.println();
            break;
        case ARDUINO_EVENT_ETH_DISCONNECTED:
            Serial.println("[WARNUNG] LAN-Kabel abgezogen (Link Down)! Versuche WLAN...");
            break;
        case ARDUINO_EVENT_ETH_STOP:
            Serial.println("[NETZWERK] Ethernet gestoppt.");
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            Serial.println();
            Serial.println("--------------------------------------------------");
            Serial.println("[VERBINDUNG HERGESTELLT - WLAN]");
            Serial.print("  WLAN-Name (SSID)   : "); Serial.println(WiFi.SSID());
            Serial.print("  Signalstaerke      : "); Serial.print(WiFi.RSSI()); Serial.println(" dBm");
            Serial.print("  IP-Adresse         : "); Serial.println(WiFi.localIP());
            Serial.print("  Web-Oberflaeche    : http://"); Serial.println(WiFi.localIP());
            Serial.print("  Kassen-Drucker-Port: "); Serial.print(WiFi.localIP()); Serial.println(":9100");
            Serial.println("--------------------------------------------------");
            Serial.println();
            break;
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            Serial.println("[WARNUNG] WLAN-Verbindung getrennt!");
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
    ETH.begin(ETH_PHY_W5500, 1, DEFAULT_ETH_CS, DEFAULT_ETH_INT, DEFAULT_ETH_RST, SPI);
    #else
    // Fallback falls Kern-Definition abweicht
    ETH.begin();
    #endif
}

void NetManager::startWifi() {
    AppConfig& conf = ConfigManager::instance().get();
    if (conf.wifi_ssid.length() == 0) {
        Serial.println("[NETZWERK] Kein LAN-Kabel gesteckt und keine WLAN-Zugangsdaten im Speicher.");
        Serial.println("[HINWEIS] Bitte LAN-Kabel einstecken oder WLAN-Daten im Web-Menue eintragen.");
        stopWifi();
        return;
    }

    if (wifiAttemptActive && WiFi.status() == WL_CONNECTED) {
        return; // Bereits verbunden
    }

    Serial.print("[WLAN] Aktiviere WLAN und verbinde mit: ");
    Serial.println(conf.wifi_ssid);
    WiFi.mode(WIFI_STA);
    WiFi.setHostname("BonbridgeESP32");
    WiFi.begin(conf.wifi_ssid.c_str(), conf.wifi_password.c_str());
    wifiAttemptActive = true;
    wifiConnectStartMs = millis();
}

void NetManager::stopWifi() {
    if (WiFi.getMode() != WIFI_OFF) {
        Serial.println("[WLAN] Deaktiviere WLAN-Modul (Kabelverbindung aktiv).");
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
            Serial.println("[NETZWERK] LAN-Kabelverbindung erkannt! WLAN wird deaktiviert.");
            stopWifi();
            currentMode = NET_MODE_ETHERNET;
        } else {
            Serial.println("[NETZWERK] Kein Signal auf dem LAN-Kabel. Aktiviere WLAN-Reserve...");
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
