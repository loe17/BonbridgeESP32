#pragma once

#include <Arduino.h>
#include <WebServer.h>
#include <memory>

class WebUI {
public:
    static WebUI& instance();

    bool begin(uint16_t port = 80);
    void update();

private:
    WebUI();

    std::unique_ptr<WebServer> server;
    String lastMessage;
    bool messageIsError;

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
