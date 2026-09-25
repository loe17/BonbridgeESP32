#pragma once

#include <Arduino.h>

class NetWatch {
public:
    static NetWatch& instance();

    void begin();
    void update();

    bool hasAlertBeenPrinted();

private:
    NetWatch();

    bool wasOnline;
    bool alertPrinted;
    unsigned long bootTimeMs;
    unsigned long offlineSinceMs;
    bool offlineDetected;
};
