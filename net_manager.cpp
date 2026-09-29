#include "net_manager.h"
#include <esp_log.h>
#include <ETH.h>

static const char* TAG = "NetManager";

static NetManager* s_instance = nullptr;

static const char* getWifiReasonText(uint8_t reason) {
    switch (reason) {
        case 1:   return "UNSPECIFIED";
        case 2:   return "AUTH_EXPIRE";
        case 15:  return "4WAY_HANDSHAKE_TIMEOUT (Passwort pruefen!)";
        case 200: return "BEACON_TIMEOUT (Signal zu schwach / außer Reichweite)";
        case 201: return "NO_AP_FOUND (WLAN-Router nicht gefunden! Bitte 2.4 GHz pruefen)";
        case 202: return "AUTH_FAIL (Passwort falsch oder WPA3/PMF Inkompatibilitaet)";
        case 203: return "ASSOC_FAIL";
        case 204: return "HANDSHAKE_TIMEOUT";
        case 205: return "CONNECTION_FAIL";
        default:  return "Verbindung fehlgeschlagen";
    }
}

static void onWiFiEthEvent(WiFiEvent_t event, WiFiEventInfo_t info) {
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
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            Serial.println("[WLAN] Funkverbindung mit WLAN-Router hergestellt! Warte auf IP-Adresse...");
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
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED: {
            uint8_t reason = info.wifi_sta_disconnected.reason;
            Serial.printf("[WLAN-INFO] Verbindungsversuch fehlgeschlagen (Code %d: %s)\n",
                          reason, getWifiReasonText(reason));
            if (s_instance) {
                s_instance->onDisconnectEvent();
            }
            break;
        }
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
    wifiAttemptActive(false),
    disconnectCount(0)
{
    s_instance = this;
}

void NetManager::onDisconnectEvent() {
    disconnectCount++;
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
        Serial.println("[NETZWERK] Kein LAN-Kabel gesteckt und keine WLAN-Daten im Speicher.");
        Serial.println("[WLAN-HOTSPOT] Starte Einrichtungs-Hotspot: 'Bonbridge-Setup'");
        WiFi.mode(WIFI_AP);
        WiFi.softAP("Bonbridge-Setup");
        Serial.print("[WLAN-HOTSPOT] Hotspot IP-Adresse: ");
        Serial.println(WiFi.softAPIP());
        Serial.println("[WLAN-HOTSPOT] Verbinde dich mit 'Bonbridge-Setup' und oeffne http://192.168.4.1 im Browser.");
        wifiAttemptActive = false;
        wifiConnected = false;
        currentMode = NET_MODE_WIFI;
        return;
    }

    if (wifiAttemptActive && WiFi.status() == WL_CONNECTED) {
        return; // Bereits verbunden
    }

    Serial.printf("[WLAN] Verbinde mit: '%s' (Passwort: %d Zeichen)\n", conf.wifi_ssid.c_str(), conf.wifi_password.length());
    WiFi.mode(WIFI_STA);
    WiFi.setHostname("BonbridgeESP32");
    WiFi.setAutoReconnect(true);
    WiFi.setTxPower(WIFI_POWER_15dBm);
    WiFi.begin(conf.wifi_ssid.c_str(), conf.wifi_password.c_str());
    wifiAttemptActive = true;
    wifiConnectStartMs = millis();
    disconnectCount = 0;
}

void NetManager::stopWifi() {
    if (WiFi.getMode() != WIFI_OFF) {
        Serial.println("[WLAN] Deaktiviere WLAN-Modul (Kabelverbindung aktiv).");
        WiFi.disconnect(true);
        WiFi.softAPdisconnect(true);
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
    // Wenn WLAN-Verbindung versucht wird, aber nach 12s oder 4 Fehlversuchen nicht klappt
    if (wifiAttemptActive && !wifiConnected && !ethLinkUp) {
        if ((millis() - wifiConnectStartMs > 12000) || disconnectCount >= 4) {
            Serial.println();
            Serial.println("==================================================");
            Serial.println("[WLAN-HINWEIS] Verbindung zum WLAN-Router nicht moeglich.");
            Serial.println("[WLAN-HOTSPOT] Eigener Einrichtungs-Hotspot wird zusaetzlich gestartet!");
            Serial.println("  Name (SSID): Bonbridge-Setup");
            Serial.println("  Passwort   : keines (offen)");
            Serial.println("  Web-Menue  : http://192.168.4.1");
            Serial.println("==================================================");
            Serial.println();
            WiFi.mode(WIFI_AP_STA);
            WiFi.softAP("Bonbridge-Setup");
            Serial.print("[WLAN-HOTSPOT] Hotspot IP: ");
            Serial.println(WiFi.softAPIP());
            wifiAttemptActive = false;
        }
    }

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
            if (WiFi.getMode() == WIFI_AP) {
                return "Einrichtungs-Hotspot aktiv ('Bonbridge-Setup')";
            }
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
        if (WiFi.getMode() == WIFI_AP) {
            return WiFi.softAPIP().toString();
        }
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
    if (currentMode == NET_MODE_WIFI) {
        if (WiFi.getMode() == WIFI_AP) return true;
        if (WiFi.status() == WL_CONNECTED) {
            return WiFi.localIP() != IPAddress(0, 0, 0, 0);
        }
    }
    return false;
}

bool NetManager::isEthernetLinkUp() {
    return ethLinkUp;
}

bool NetManager::isWifiConnected() {
    return wifiConnected;
}
