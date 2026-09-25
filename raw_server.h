#pragma once

#include <Arduino.h>
#include <WiFiServer.h>
#include <WiFiClient.h>

class RawServer {
public:
    static RawServer& instance();

    bool begin(uint16_t port = 9100);
    void update();

    bool isClientConnected();
    uint32_t getTotalBytesReceived();

private:
    RawServer();

    uint16_t listenPort;
    WiFiServer server;
    WiFiClient activeClient;
    uint32_t totalBytesReceived;
    bool clientActive;
    uint8_t rxBuffer[1024];

    void processClientData();
};
