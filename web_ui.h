#pragma once

#include <Arduino.h>
#include <WebServer.h>
#include <memory>

class WebUI {
public:
    static WebUI& instance();

    bool begin(uint16_t port = 80);
    void update();

    void setResetReason(uint8_t reason);
    String getResetReasonString() const;

private:
    WebUI();

    std::unique_ptr<WebServer> server;
    String lastMessage;
    bool messageIsError;
    uint8_t resetReasonCode;

    void handleRoot();
    void handleAction();
    void handleSave();
    void handleScan();
    void handleConnect();
    void handleStatus();
    void handleOtaStart();
    void handleOtaStatus();
    void handleOtaUpload();
    void handleNotFound();

    String generateHtmlPage();
};
