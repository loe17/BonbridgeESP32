#pragma once

#include <Arduino.h>
#include <WebServer.h>

class WebUI {
public:
    static WebUI& instance();

    bool begin(uint16_t port = 80);
    void update();

private:
    WebUI();

    WebServer server;
    String lastMessage;
    bool messageIsError;

    void handleRoot();
    void handleAction();
    void handleSave();
    void handleNotFound();

    String generateHtmlPage();
};
