#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <SPI.h>
#include "config.h"

enum NetActiveMode {
    NET_MODE_NONE = 0,
    NET_MODE_ETHERNET = 1,
    NET_MODE_WIFI = 2
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

    // Ermöglicht es, die WLAN-Verbindung nach Speichern neuer Daten neu zu starten
    void reloadWifiConfig();

private:
    NetManager();

    NetActiveMode currentMode;
    bool ethLinkUp;
    bool wifiConnected;
    unsigned long lastCheckMs;
    unsigned long wifiConnectStartMs;
    bool wifiAttemptActive;

    void initEthernet();
    void startWifi();
    void stopWifi();
    void checkConnections();
};
