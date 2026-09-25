#pragma once

#include <Arduino.h>

// Standard-Pins für W5500 SPI Ethernet-Aufsatz auf dem ESP32-S3
#define DEFAULT_ETH_MISO 12
#define DEFAULT_ETH_MOSI 11
#define DEFAULT_ETH_SCLK 10
#define DEFAULT_ETH_CS   9
#define DEFAULT_ETH_INT  14
#define DEFAULT_ETH_RST  13

// Verfügbare Druckprofile
enum PrinterProfile {
    PROFILE_EPSON_80MM = 0,    // Epson TM-T88 und vollkompatible (Standard 80 mm)
    PROFILE_GENERIC_80MM = 1,  // Generischer ESC/POS Bondrucker (80 mm)
    PROFILE_COMPACT_58MM = 2,  // Kompakter Bondrucker (58 mm)
    PROFILE_STAR_ESC = 3       // Star Micronics im ESC/POS Emulationsmodus
};

struct AppConfig {
    String wifi_ssid;
    String wifi_password;
    int printer_profile;
    bool netwatch_enabled;
    uint16_t port9100;
};

class ConfigManager {
public:
    static ConfigManager& instance();

    void begin();
    AppConfig& get();
    void save();
    
    static const char* getProfileName(int profile);

private:
    ConfigManager();
    AppConfig config;
};
