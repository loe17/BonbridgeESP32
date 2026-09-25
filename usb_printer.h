#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include "esp_err.h"
#include "usb/usb_host.h"

class UsbPrinter {
public:
    static UsbPrinter& instance();

    // Startet den USB Host Treiber und die Hintergrund-Aufgaben
    bool begin();

    // Zustand des Druckers
    bool isConnected();
    String getStatusString();
    String getDeviceName();

    // Direkte Datenübertragung (wird von Port 9100 genutzt)
    size_t write(const uint8_t* data, size_t len);

    // Kassenlade ansteuern (Kassenladen-Impuls Pin 2 & 5)
    void kickCashDrawer();

    // Testbon drucken
    void printTestSlip(const String& ipAddress, const String& activeInterface, const String& profileName);

    // Warnbon bei Netzwerkausfall drucken
    void printNetworkAlert(const String& title, const String& message);

    // Intern: Callback-Handler für den USB Host
    void handleDeviceEvent(usb_host_client_event_msg_t* event_msg);
    void setDeviceConnected(bool connected, const String& name, uint8_t outEp, uint16_t maxPacketSize, usb_device_handle_t devHandle);

private:
    UsbPrinter();
    
    bool initialized;
    bool deviceConnected;
    String deviceName;
    uint8_t outEndpoint;
    uint16_t outMaxPacketSize;
    usb_device_handle_t dev_hdl;
    usb_host_client_handle_t client_hdl;
    SemaphoreHandle_t writeMutex;

    void sendEscPosCommand(const uint8_t* cmd, size_t len);
};
