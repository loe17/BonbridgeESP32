#include "config.h"
#include <Preferences.h>

static Preferences prefs;

ConfigManager::ConfigManager() {
    config.wifi_ssid = "";
    config.wifi_password = "";
    config.printer_profile = PROFILE_EPSON_80MM;
    config.netwatch_enabled = true;
    config.port9100 = 9100;
}

ConfigManager& ConfigManager::instance() {
    static ConfigManager inst;
    return inst;
}

void ConfigManager::begin() {
    prefs.begin("bonbridge", false);
    config.wifi_ssid = prefs.getString("ssid", "");
    config.wifi_password = prefs.getString("pass", "");
    config.printer_profile = prefs.getInt("profile", PROFILE_EPSON_80MM);
    config.netwatch_enabled = prefs.getBool("netwatch", true);
    config.port9100 = prefs.getUShort("port9100", 9100);
}

AppConfig& ConfigManager::get() {
    return config;
}

void ConfigManager::save() {
    prefs.putString("ssid", config.wifi_ssid);
    prefs.putString("pass", config.wifi_password);
    prefs.putInt("profile", config.printer_profile);
    prefs.putBool("netwatch", config.netwatch_enabled);
    prefs.putUShort("port9100", config.port9100);
}

const char* ConfigManager::getProfileName(int profile) {
    switch (profile) {
        case PROFILE_EPSON_80MM:
            return "Epson TM-T88 / Kompatible (80 mm)";
        case PROFILE_GENERIC_80MM:
            return "Standard ESC/POS (80 mm)";
        case PROFILE_COMPACT_58MM:
            return "Kompakt (58 mm)";
        case PROFILE_STAR_ESC:
            return "Star Micronics (ESC/POS Modus)";
        default:
            return "Unbekanntes Profil";
    }
}
