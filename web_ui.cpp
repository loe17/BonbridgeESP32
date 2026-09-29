#include "web_ui.h"
#include "config.h"
#include "net_manager.h"
#include "usb_printer.h"
#include "raw_server.h"
#include <esp_log.h>

static const char* TAG = "WebUI";

WebUI::WebUI() :
    server(nullptr),
    lastMessage(""),
    messageIsError(false)
{
}

WebUI& WebUI::instance() {
    static WebUI inst;
    return inst;
}

bool WebUI::begin(uint16_t port) {
    server.reset(new WebServer(port));

    server->on("/", HTTP_GET, [this]() { handleRoot(); });
    server->on("/action", HTTP_POST, [this]() { handleAction(); });
    server->on("/save", HTTP_POST, [this]() { handleSave(); });
    server->on("/api/scan", HTTP_GET, [this]() { handleScan(); });
    server->on("/api/connect", HTTP_POST, [this]() { handleConnect(); });
    server->on("/api/status", HTTP_GET, [this]() { handleStatus(); });
    server->onNotFound([this]() { handleNotFound(); });

    server->begin();
    ESP_LOGI(TAG, "Sparsame Web-Oberflaeche lauscht auf HTTP-Port %d", port);
    return true;
}

void WebUI::update() {
    // Ruft ankommende Web-Anfragen ab. Wenn kein Browser zugreift, 
    // ist diese Funktion nahezu lastfrei (0% CPU).
    if (server) {
        server->handleClient();
    }
}

void WebUI::handleRoot() {
    if (!server) return;
    String html = generateHtmlPage();
    server->send(200, "text/html; charset=UTF-8", html);
    lastMessage = "";
}

void WebUI::handleAction() {
    if (!server) return;
    String cmd = server->arg("cmd");
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

    server->sendHeader("Location", "/");
    server->send(303);
}

void WebUI::handleSave() {
    if (!server) return;
    AppConfig& conf = ConfigManager::instance().get();

    if (server->hasArg("ssid")) {
        conf.wifi_ssid = server->arg("ssid");
    }
    if (server->hasArg("pass")) {
        String newPass = server->arg("pass");
        if (newPass.length() > 0) {
            conf.wifi_password = newPass;
        }
    }
    if (server->hasArg("profile")) {
        conf.printer_profile = server->arg("profile").toInt();
    }
    conf.netwatch_enabled = server->hasArg("netwatch");

    ConfigManager::instance().save();
    NetManager::instance().reloadWifiConfig();

    lastMessage = "Einstellungen erfolgreich gespeichert!";
    messageIsError = false;

    server->sendHeader("Location", "/");
    server->send(303);
}

void WebUI::handleScan() {
    if (!server) return;
    auto networks = NetManager::instance().scanNetworks();
    String json = "[";
    for (size_t i = 0; i < networks.size(); ++i) {
        if (i > 0) json += ",";
        json += "{\"ssid\":\"";
        for (char c : networks[i].ssid) {
            if (c == '"' || c == '\\') json += '\\';
            json += c;
        }
        json += "\",\"rssi\":";
        json += networks[i].rssi;
        json += ",\"signal\":";
        json += networks[i].signalPercent;
        json += ",\"secure\":";
        json += networks[i].isSecure ? "true" : "false";
        json += "}";
    }
    json += "]";
    server->send(200, "application/json", json);
}

void WebUI::handleConnect() {
    if (!server) return;
    String ssid = server->hasArg("ssid") ? server->arg("ssid") : "";
    String pass = server->hasArg("pass") ? server->arg("pass") : "";

    if (ssid.length() == 0) {
        server->send(400, "application/json", "{\"error\":\"Keine SSID angegeben\"}");
        return;
    }

    NetManager::instance().connectToWifi(ssid, pass);
    server->send(200, "application/json", "{\"status\":\"started\"}");
}

void WebUI::handleStatus() {
    if (!server) return;
    WifiConnectState st = NetManager::instance().getConnectState();
    String stStr = "idle";
    if (st == WIFI_STATE_CONNECTING) stStr = "connecting";
    else if (st == WIFI_STATE_CONNECTED) stStr = "connected";
    else if (st == WIFI_STATE_FAILED) stStr = "failed";

    String json = "{";
    json += "\"state\":\"" + stStr + "\",";
    json += "\"ip\":\"" + NetManager::instance().getIpAddress() + "\",";
    json += "\"mode\":\"" + NetManager::instance().getActiveDescription() + "\",";
    json += "\"error\":\"" + NetManager::instance().getLastConnectError() + "\"";
    json += "}";
    server->send(200, "application/json", json);
}

void WebUI::handleNotFound() {
    if (server) {
        server->send(404, "text/plain", "404 - Seite nicht gefunden");
    }
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
    html.reserve(10240);

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
    html += ".btn-scan { background: #6c757d; color: #fff; padding: 8px 14px; font-size: 13px; border-radius: 4px; white-space: nowrap; }";
    html += ".btn-connect { background: #0d6efd; color: #fff; width: 100%; padding: 11px; margin-top: 10px; font-size: 14px; border-radius: 5px; }";
    html += ".btn-toggle { background: #e9ecef; color: #333; border: 1px solid #ced4da; border-radius: 4px; padding: 8px 12px; font-size: 14px; }";
    html += ".scan-list { max-height: 220px; overflow-y: auto; border: 1px solid #ced4da; border-radius: 5px; margin-top: 8px; background: #fff; box-shadow: 0 2px 4px rgba(0,0,0,0.05); }";
    html += ".scan-item { display: flex; justify-content: space-between; align-items: center; padding: 9px 12px; border-bottom: 1px solid #f0f0f0; cursor: pointer; transition: background 0.15s; font-size: 13px; }";
    html += ".scan-item:hover { background: #e9f2ff; }";
    html += ".scan-item:last-child { border-bottom: none; }";
    html += ".scan-item-ssid { font-weight: 600; color: #111; }";
    html += ".scan-item-meta { color: #666; font-size: 12px; display: flex; gap: 8px; align-items: center; }";
    html += ".wifi-card { padding: 12px 14px; border-radius: 6px; margin-top: 12px; font-size: 14px; line-height: 1.5; }";
    html += ".wifi-card-info { background: #cff4fc; color: #055160; border: 1px solid #b6effb; }";
    html += ".wifi-card-success { background: #d1e7dd; color: #0f5132; border: 1px solid #badbcc; }";
    html += ".wifi-card-danger { background: #f8d7da; color: #842029; border: 1px solid #f5c2c7; }";
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

    html += "<div class=\"section-title\" style=\"margin-top:20px;\">WLAN-Ersatzverbindung</div>";
    html += "<div style=\"font-size:12px;color:#666;margin-bottom:8px;\">Wird automatisch aktiviert, falls kein LAN-Kabel eingesteckt ist.</div>";

    // WLAN Name mit Such-Button
    html += "<label for=\"ssid\">WLAN Name (SSID):</label>";
    html += "<div style=\"display:flex;gap:6px;margin-top:4px;\">";
    html += "<input type=\"text\" name=\"ssid\" id=\"ssid\" value=\"" + conf.wifi_ssid + "\" placeholder=\"z.B. WLAN-Name\" style=\"margin-top:0;flex:1;\">";
    html += "<button type=\"button\" id=\"btn-scan\" onclick=\"scanWifi()\" class=\"btn-scan\">🔍 Suchen</button>";
    html += "</div>";
    html += "<div id=\"scan-status\" style=\"display:none;margin-top:6px;font-size:13px;color:#0d6efd;\">⏳ Suche nach WLAN-Netzen in Reichweite...</div>";
    html += "<div id=\"scan-list\" class=\"scan-list\" style=\"display:none;\"></div>";

    // WLAN Passwort mit Anzeige-Button
    html += "<label for=\"pass\">WLAN Passwort:</label>";
    html += "<div style=\"display:flex;gap:6px;margin-top:4px;\">";
    html += "<input type=\"password\" name=\"pass\" id=\"pass\" placeholder=\"Leer lassen, um unveraendert zu bleiben\" style=\"margin-top:0;flex:1;\">";
    html += "<button type=\"button\" onclick=\"togglePass()\" class=\"btn-toggle\" title=\"Passwort anzeigen/verbergen\">👁️</button>";
    html += "</div>";

    // Direkt verbinden Button + Rückmeldungs-Box
    html += "<button type=\"button\" id=\"btn-connect\" onclick=\"connectWifi()\" class=\"btn-connect\">Mit diesem WLAN verbinden</button>";
    html += "<div id=\"wifi-feedback\" style=\"display:none;\"></div>";

    html += "<button type=\"submit\" class=\"btn-save\">Einstellungen dauerhaft speichern</button>";
    html += "</form>";

    html += "<div class=\"footer\">BonbridgeESP32 &middot; 1 Drucker pro Adapter &middot; Port 9100 RAW</div>";
    html += "</div>";

    // JavaScript für Suche, Netzauswahl und Live-Verbindungsstatus
    html += "<script>";
    html += "function togglePass(){var p=document.getElementById('pass');p.type=(p.type==='password')?'text':'password';}";
    html += "function scanWifi(){";
    html += "var btn=document.getElementById('btn-scan');var st=document.getElementById('scan-status');var list=document.getElementById('scan-list');";
    html += "btn.disabled=true;st.style.display='block';list.style.display='none';";
    html += "fetch('/api/scan').then(function(r){return r.json();}).then(function(nets){";
    html += "btn.disabled=false;st.style.display='none';";
    html += "if(!nets||nets.length===0){list.innerHTML='<div style=\"padding:10px;color:#888;text-align:center;\">Keine WLAN-Netze gefunden</div>';list.style.display='block';return;}";
    html += "nets.sort(function(a,b){return b.signal-a.signal;});";
    html += "var h='';for(var i=0;i<nets.size?nets.size():nets.length;i++){";
    html += "var n=nets[i];var lock=n.secure?'🔒':'🔓';";
    html += "h+='<div class=\"scan-item\" onclick=\"selectNet(\\''+encodeURIComponent(n.ssid)+'\\')\">';";
    html += "h+='<span class=\"scan-item-ssid\">'+escapeHtml(n.ssid)+'</span>';";
    html += "h+='<span class=\"scan-item-meta\"><span>'+lock+'</span><span>📶 '+n.signal+'%</span></span>';";
    html += "h+='</div>';}";
    html += "list.innerHTML=h;list.style.display='block';";
    html += "}).catch(function(e){btn.disabled=false;st.style.display='none';list.innerHTML='<div style=\"padding:10px;color:#dc3545;text-align:center;\">Fehler beim Suchen</div>';list.style.display='block';});";
    html += "}";
    html += "function selectNet(enc){document.getElementById('ssid').value=decodeURIComponent(enc);document.getElementById('scan-list').style.display='none';document.getElementById('pass').focus();}";
    html += "function escapeHtml(t){var d=document.createElement('div');d.textContent=t;return d.innerHTML;}";
    html += "var pollTimer=null;var pollCount=0;";
    html += "function connectWifi(){";
    html += "var ssid=document.getElementById('ssid').value.trim();var pass=document.getElementById('pass').value;";
    html += "var fb=document.getElementById('wifi-feedback');var btn=document.getElementById('btn-connect');";
    html += "if(!ssid){fb.className='wifi-card wifi-card-danger';fb.innerHTML='Bitte zuerst ein WLAN auswaehlen oder den Namen eingeben.';fb.style.display='block';return;}";
    html += "if(pollTimer)clearInterval(pollTimer);pollCount=0;btn.disabled=true;";
    html += "fb.className='wifi-card wifi-card-info';fb.style.display='block';";
    html += "fb.innerHTML='⏳ Verbindungsversuch mit <strong>'+escapeHtml(ssid)+'</strong> laeuft... Bitte kurz warten...';";
    html += "var body='ssid='+encodeURIComponent(ssid)+'&pass='+encodeURIComponent(pass);";
    html += "fetch('/api/connect',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:body})";
    html += ".then(function(r){return r.json();}).then(function(d){pollTimer=setInterval(pollStatus,1000);})";
    html += ".catch(function(e){btn.disabled=false;fb.className='wifi-card wifi-card-danger';fb.innerHTML='Konnte Befehl nicht an den ESP senden.';});";
    html += "}";
    html += "function pollStatus(){";
    html += "pollCount++;var fb=document.getElementById('wifi-feedback');var btn=document.getElementById('btn-connect');";
    html += "fetch('/api/status').then(function(r){return r.json();}).then(function(st){";
    html += "if(st.state==='connected'){clearInterval(pollTimer);btn.disabled=false;fb.className='wifi-card wifi-card-success';";
    html += "var ip=st.ip;var url='http://'+ip;";
    html += "fb.innerHTML='✅ <strong>Erfolgreich verbunden!</strong><br>Neue IP-Adresse: <strong>'+ip+'</strong><br>Weboberflaeche: <a href=\"'+url+'\" target=\"_blank\" style=\"color:#0f5132;font-weight:bold;text-decoration:underline;\">'+url+'</a><div style=\"margin-top:6px;font-size:12px;color:#155724;\">Hinweis: Du kannst dieses Fenster schliessen und die Seite unter der neuen Adresse aufrufen.</div>';";
    html += "}else if(st.state==='failed'){clearInterval(pollTimer);btn.disabled=false;fb.className='wifi-card wifi-card-danger';";
    html += "var reason=st.error||'Verbindung fehlgeschlagen';";
    html += "fb.innerHTML='❌ <strong>Verbindung nicht moeglich</strong><br>Grund: '+escapeHtml(reason)+'<div style=\"margin-top:6px;font-size:12px;\">Bitte pruefe das WLAN-Passwort oder stelle sicher, dass 2.4 GHz aktiv ist.</div>';";
    html += "}else if(pollCount>=18){clearInterval(pollTimer);btn.disabled=false;fb.className='wifi-card wifi-card-danger';";
    html += "fb.innerHTML='⚠️ <strong>Zeitueberschreitung</strong><br>Keine Rueckmeldung vom Router erhalten. Bitte pruefe den WLAN-Namen und das Passwort.';";
    html += "}";
    html += "}).catch(function(e){if(pollCount>=8){clearInterval(pollTimer);btn.disabled=false;fb.className='wifi-card wifi-card-success';fb.innerHTML='ℹ️ <strong>ESP hat sich verbunden!</strong><br>Die Verbindung zum Hotspot wurde beendet. Bitte schaue im WLAN-Router nach der vergebenen IP-Adresse.';}});";
    html += "}";
    html += "</script>";
    html += "</body></html>";

    return html;
}
