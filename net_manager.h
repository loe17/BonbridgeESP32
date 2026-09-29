#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <SPI.h>
#include <vector>
#include "config.h"

enum NetActiveMode {
    NET_MODE_NONE = 0,
    NET_MODE_ETHERNET = 1,
    NET_MODE_WIFI = 2
};

enum WifiConnectState {
    WIFI_STATE_IDLE = 0,
    WIFI_STATE_CONNECTING = 1,
    WIFI_STATE_CONNECTED = 2,
    WIFI_STATE_FAILED = 3
};

struct WifiNetworkInfo {
    String ssid;
    int32_t rssi;
    bool isSecure;
    int signalPercent;
};

class NetManager {
public:
    static NetManager& instance();

    void begin();
    void update();

    NetActiveMode getActiveMode();
    String getActiveDescription();
    String getIpAddress();
    String getMacAddress();
    bool isOnline();
    bool isEthernetLinkUp();
    bool isWifiConnected();

    // WLAN-Suchfunktion
    std::vector<WifiNetworkInfo> scanNetworks();

    // Gezielte Verbindung mit Rueckmeldung
    void connectToWifi(const String& ssid, const String& pass);
    WifiConnectState getConnectState() const;
    String getLastConnectError() const;

    // Ermöglicht es, die WLAN-Verbindung nach Speichern neuer Daten neu zu starten
    void reloadWifiConfig();
    void onDisconnectEvent(uint8_t reason);
    void onConnectEvent();

private:
    NetManager();

    NetActiveMode currentMode;
    bool ethLinkUp;
    bool wifiConnected;
    unsigned long lastCheckMs;
    unsigned long wifiConnectStartMs;
    bool wifiAttemptActive;
    uint8_t disconnectCount;

    WifiConnectState connectState;
    String lastConnectError;

    void initEthernet();
    void startWifi();
    void stopWifi();
    void checkConnections();
};
