#include "raw_server.h"
#include "usb_printer.h"
#include <esp_log.h>

static const char* TAG = "RawServer";

RawServer::RawServer() :
    listenPort(9100),
    server(9100),
    totalBytesReceived(0),
    clientActive(false)
{
}

RawServer& RawServer::instance() {
    static RawServer inst;
    return inst;
}

bool RawServer::begin(uint16_t port) {
    listenPort = port;
    server = WiFiServer(listenPort);
    server.begin();
    server.setNoDelay(true);
    ESP_LOGI(TAG, "RAW-Druckserver lauscht auf TCP-Port %d", listenPort);
    return true;
}

void RawServer::update() {
    // 1. Wenn kein aktiver Client da ist, auf neue Verbindung warten
    if (!clientActive || !activeClient.connected()) {
        WiFiClient newClient = server.available();
        if (newClient) {
            activeClient = newClient;
            activeClient.setNoDelay(true);
            clientActive = true;
            ESP_LOGI(TAG, "Kassensystem verbunden (%s:%d)", 
                     activeClient.remoteIP().toString().c_str(), activeClient.remotePort());
        }
    }

    // 2. Daten vom Kassensystem empfangen und direkt zum Drucker streamen
    if (clientActive && activeClient.connected()) {
        int avail = activeClient.available();
        if (avail > 0) {
            int toRead = avail > (int)sizeof(rxBuffer) ? sizeof(rxBuffer) : avail;
            int bytesRead = activeClient.read(rxBuffer, toRead);
            if (bytesRead > 0) {
                totalBytesReceived += bytesRead;
                size_t written = UsbPrinter::instance().write(rxBuffer, bytesRead);
                if (written < (size_t)bytesRead) {
                    ESP_LOGW(TAG, "USB-Drucker konnte nicht alle Daten aufnehmen (%u von %d)", written, bytesRead);
                }
            }
        }
    }

    // 3. Wenn die Kasse die Verbindung beendet hat
    if (clientActive && !activeClient.connected()) {
        activeClient.stop();
        clientActive = false;
        ESP_LOGI(TAG, "Druckauftrag beendet, Verbindung geschlossen.");
    }
}

bool RawServer::isClientConnected() {
    return clientActive && activeClient.connected();
}

uint32_t RawServer::getTotalBytesReceived() {
    return totalBytesReceived;
}
