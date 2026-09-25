#include "web_ui.h"
#include "config.h"
#include "net_manager.h"
#include "usb_printer.h"
#include "raw_server.h"
#include <esp_log.h>

static const char* TAG = "WebUI";

WebUI::WebUI() :
    server(80),
    lastMessage(""),
    messageIsError(false)
{
}

WebUI& WebUI::instance() {
    static WebUI inst;
    return inst;
}

bool WebUI::begin(uint16_t port) {
    server = WebServer(port);

    server.on("/", HTTP_GET, [this]() { handleRoot(); });
    server.on("/action", HTTP_POST, [this]() { handleAction(); });
    server.on("/save", HTTP_POST, [this]() { handleSave(); });
    server.onNotFound([this]() { handleNotFound(); });

    server.begin();
    ESP_LOGI(TAG, "Sparsame Web-Oberflaeche lauscht auf HTTP-Port %d", port);
    return true;
}

void WebUI::update() {
    // Ruft ankommende Web-Anfragen ab. Wenn kein Browser zugreift, 
    // ist diese Funktion nahezu lastfrei (0% CPU).
    server.handleClient();
}

void WebUI::handleRoot() {
    String html = generateHtmlPage();
    server.send(200, "text/html; charset=UTF-8", html);
    lastMessage = "";
}

void WebUI::handleAction() {
    String cmd = server.arg("cmd");
    if (cmd == "test_print") {
        if (UsbPrinter::instance().isConnected()) {
            AppConfig& conf = ConfigManager::instance().get();
            UsbPrinter::instance().printTestSlip(
                NetManager::instance().getIpAddress(),
                NetManager::instance().getActiveDescription(),
                ConfigManager::getProfileName(conf.printer_profile)
            );
            lastMessage = "Testbon wurde an den Drucker gesendet!";
            messageIsError = false;
        } else {
            lastMessage = "Fehler: Kein USB-Drucker angeschlossen!";
            messageIsError = true;
        }
    } else if (cmd == "kick_drawer") {
        if (UsbPrinter::instance().isConnected()) {
            UsbPrinter::instance().kickCashDrawer();
            lastMessage = "Impuls zum Oeffnen der Kassenlade wurde gesendet!";
            messageIsError = false;
        } else {
            lastMessage = "Fehler: Kein USB-Drucker angeschlossen!";
            messageIsError = true;
        }
    } else {
        lastMessage = "Unbekannte Aktion.";
        messageIsError = true;
    }

    server.sendHeader("Location", "/");
    server.send(303);
}

void WebUI::handleSave() {
    AppConfig& conf = ConfigManager::instance().get();

    if (server.hasArg("ssid")) {
        conf.wifi_ssid = server.arg("ssid");
    }
    if (server.hasArg("pass")) {
        String newPass = server.arg("pass");
        if (newPass.length() > 0) {
            conf.wifi_password = newPass;
        }
    }
    if (server.hasArg("profile")) {
        conf.printer_profile = server.arg("profile").toInt();
    }
    conf.netwatch_enabled = server.hasArg("netwatch");

    ConfigManager::instance().save();
    NetManager::instance().reloadWifiConfig();

    lastMessage = "Einstellungen erfolgreich gespeichert!";
    messageIsError = false;

    server.sendHeader("Location", "/");
    server.send(303);
}

void WebUI::handleNotFound() {
    server.send(404, "text/plain", "404 - Seite nicht gefunden");
}

String WebUI::generateHtmlPage() {
    AppConfig& conf = ConfigManager::instance().get();
    String activeDesc = NetManager::instance().getActiveDescription();
    NetActiveMode mode = NetManager::instance().getActiveMode();
    String ipStr = NetManager::instance().getIpAddress();
    String printerStatus = UsbPrinter::instance().getStatusString();
    bool printerOk = UsbPrinter::instance().isConnected();

    String badgeColor = "#6c757d"; // grau
    if (mode == NET_MODE_ETHERNET) {
        badgeColor = "#198754"; // grün
    } else if (mode == NET_MODE_WIFI) {
        badgeColor = "#0d6efd"; // blau
    } else {
        badgeColor = "#dc3545"; // rot
    }

    String html;
    html.reserve(4096);

    html += "<!DOCTYPE html><html lang=\"de\"><head>";
    html += "<meta charset=\"UTF-8\">";
    html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
    html += "<title>BonbridgeESP32 Status & Einstellungen</title>";
    html += "<style>";
    html += "body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif; background: #f4f6f8; color: #222; margin: 0; padding: 15px; }";
    html += ".container { max-width: 580px; margin: 0 auto; background: #fff; padding: 22px; border-radius: 8px; box-shadow: 0 2px 8px rgba(0,0,0,0.08); }";
    html += "h1 { margin-top: 0; font-size: 22px; color: #111; border-bottom: 2px solid #e9ecef; padding-bottom: 10px; }";
    html += ".status-box { padding: 12px 14px; border-radius: 6px; margin-bottom: 18px; color: #fff; font-weight: 500; }";
    html += ".info-row { display: flex; justify-content: space-between; padding: 8px 0; border-bottom: 1px solid #f0f0f0; font-size: 14px; }";
    html += ".info-label { color: #555; }";
    html += ".info-val { font-weight: 600; }";
    html += ".section-title { font-size: 16px; margin: 20px 0 10px 0; color: #333; font-weight: 600; }";
    html += ".btn-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; margin-bottom: 20px; }";
    html += "button { cursor: pointer; padding: 10px 14px; border: none; border-radius: 5px; font-size: 14px; font-weight: 600; }";
    html += ".btn-test { background: #0d6efd; color: #fff; }";
    html += ".btn-drawer { background: #fd7e14; color: #fff; }";
    html += ".btn-save { background: #198754; color: #fff; width: 100%; padding: 12px; margin-top: 15px; font-size: 15px; }";
    html += "button:hover { opacity: 0.9; }";
    html += "label { display: block; margin-top: 12px; font-size: 13px; font-weight: 600; color: #444; }";
    html += "input[type=\"text\"], input[type=\"password\"], select { width: 100%; box-sizing: border-box; padding: 8px 10px; border: 1px solid #ccc; border-radius: 4px; font-size: 14px; margin-top: 4px; }";
    html += ".checkbox-row { margin-top: 15px; display: flex; align-items: center; gap: 8px; font-size: 14px; font-weight: normal; }";
    html += ".alert { padding: 10px 12px; border-radius: 5px; margin-bottom: 15px; font-size: 14px; }";
    html += ".alert-success { background: #d1e7dd; color: #0f5132; }";
    html += ".alert-danger { background: #f8d7da; color: #842029; }";
    html += ".footer { text-align: center; margin-top: 25px; font-size: 12px; color: #888; }";
    html += "</style></head><body>";

    html += "<div class=\"container\">";
    html += "<h1>BonbridgeESP32</h1>";

    if (lastMessage.length() > 0) {
        html += "<div class=\"alert " + String(messageIsError ? "alert-danger" : "alert-success") + "\">" + lastMessage + "</div>";
    }

    // Verbindungs-Badge (Aktuell worüber kommuniziert wird)
    html += "<div class=\"status-box\" style=\"background:" + badgeColor + ";\">";
    html += "Aktuelle Verbindung: " + activeDesc;
    html += "</div>";

    // Status-Details
    html += "<div class=\"info-row\"><span class=\"info-label\">IP-Adresse</span><span class=\"info-val\">" + ipStr + "</span></div>";
    html += "<div class=\"info-row\"><span class=\"info-label\">RAW Kassen-Port</span><span class=\"info-val\">9100</span></div>";
    html += "<div class=\"info-row\"><span class=\"info-label\">USB-Drucker</span><span class=\"info-val\" style=\"color:" + String(printerOk ? "#198754" : "#dc3545") + "\">" + printerStatus + "</span></div>";
    html += "<div class=\"info-row\"><span class=\"info-label\">Empfangene Daten</span><span class=\"info-val\">" + String(RawServer::instance().getTotalBytesReceived()) + " Bytes</span></div>";

    // Schnell-Aktionen
    html += "<div class=\"section-title\">Schnell-Tests</div>";
    html += "<div class=\"btn-grid\">";
    html += "<form method=\"POST\" action=\"/action\"><input type=\"hidden\" name=\"cmd\" value=\"test_print\"><button type=\"submit\" class=\"btn-test\" style=\"width:100%\">Drucktest</button></form>";
    html += "<form method=\"POST\" action=\"/action\"><input type=\"hidden\" name=\"cmd\" value=\"kick_drawer\"><button type=\"submit\" class=\"btn-drawer\" style=\"width:100%\">Kassenlade testen</button></form>";
    html += "</div>";

    // Einstellungen
    html += "<div class=\"section-title\">Einstellungen</div>";
    html += "<form method=\"POST\" action=\"/save\">";
    
    html += "<label for=\"profile\">Druckprofil:</label>";
    html += "<select name=\"profile\" id=\"profile\">";
    html += "<option value=\"0\"" + String(conf.printer_profile == 0 ? " selected" : "") + ">Epson TM-T88 / Kompatible (80 mm)</option>";
    html += "<option value=\"1\"" + String(conf.printer_profile == 1 ? " selected" : "") + ">Standard ESC/POS (80 mm)</option>";
    html += "<option value=\"2\"" + String(conf.printer_profile == 2 ? " selected" : "") + ">Kompakt (58 mm)</option>";
    html += "<option value=\"3\"" + String(conf.printer_profile == 3 ? " selected" : "") + ">Star Micronics (ESC/POS Modus)</option>";
    html += "</select>";

    html += "<div class=\"checkbox-row\">";
    html += "<input type=\"checkbox\" id=\"netwatch\" name=\"netwatch\" value=\"1\"" + String(conf.netwatch_enabled ? " checked" : "") + ">";
    html += "<label for=\"netwatch\" style=\"margin:0;font-weight:normal;\">Hinweis drucken wenn Netzwerk ausfällt</label>";
    html += "</div>";

    html += "<div class=\"section-title\" style=\"margin-top:18px;\">WLAN-Ersatzverbindung</div>";
    html += "<div style=\"font-size:12px;color:#666;margin-bottom:6px;\">Wird automatisch aktiviert, falls kein LAN-Kabel eingesteckt ist.</div>";
    html += "<label for=\"ssid\">WLAN Name (SSID):</label>";
    html += "<input type=\"text\" name=\"ssid\" id=\"ssid\" value=\"" + conf.wifi_ssid + "\" placeholder=\"z.B. MeinKassenWLAN\">";
    html += "<label for=\"pass\">WLAN Passwort:</label>";
    html += "<input type=\"password\" name=\"pass\" id=\"pass\" placeholder=\"Leer lassen, um unveraendert zu bleiben\">";

    html += "<button type=\"submit\" class=\"btn-save\">Einstellungen speichern</button>";
    html += "</form>";

    html += "<div class=\"footer\">BonbridgeESP32 &middot; 1 Drucker pro Adapter &middot; Port 9100 RAW</div>";
    html += "</div></body></html>";

    return html;
}
