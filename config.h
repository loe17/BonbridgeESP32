#pragma once

#include <Arduino.h>

// =========================================================================
// W5500 SPI Ethernet Pinbelegung
// =========================================================================
// Standardmäßig für das kompakte Seeed Studio XIAO ESP32-S3 (14-Pin Mini-Board):
#define BOARD_XIAO_ESP32S3

#if defined(BOARD_XIAO_ESP32S3) || defined(ARDUINO_XIAO_ESP32S3)
  // Pinbelegung für Seeed Studio XIAO ESP32-S3 (Amazon Modul):
  #define DEFAULT_ETH_SCLK  7   // Pin D8  (gelb beschriftet: SCK)
  #define DEFAULT_ETH_MISO  8   // Pin D9  (gelb beschriftet: MISO)
  #define DEFAULT_ETH_MOSI  9   // Pin D10 (gelb beschriftet: MOSI)
  #define DEFAULT_ETH_CS    3   // Pin D2  (Chip Select)
  #define DEFAULT_ETH_INT   2   // Pin D1  (Interrupt)
  #define DEFAULT_ETH_RST   1   // Pin D0  (Hardware-Reset)
#else
  // Pinbelegung für großes ESP32-S3 DevKit / Bonbridge Custom Board:
  #define DEFAULT_ETH_SCLK 10
  #define DEFAULT_ETH_MISO 12
  #define DEFAULT_ETH_MOSI 11
  #define DEFAULT_ETH_CS    9
  #define DEFAULT_ETH_INT  14
  #define DEFAULT_ETH_RST  13
#endif

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
