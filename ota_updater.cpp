#include "ota_updater.h"
#include <WiFi.h>
#include <Update.h>
#include <HTTPClient.h>
#include <NetworkClientSecure.h>

OtaUpdater& OtaUpdater::instance() {
    static OtaUpdater inst;
    return inst;
}

OtaUpdater::OtaUpdater() :
    currentState(OTA_STATE_IDLE),
    progressPercent(0),
    errorMessage(""),
    targetUrl(""),
    taskRunning(false),
    restartPending(false),
    restartTimer(0)
{
}

bool OtaUpdater::startHttpUpdate(const String& url) {
    if (taskRunning || currentState == OTA_STATE_DOWNLOADING || currentState == OTA_STATE_FLASHING) {
        return false;
    }

    targetUrl = url;
    if (targetUrl.length() == 0) {
        targetUrl = "https://raw.githubusercontent.com/loe17/BonbridgeESP32/main/firmware.bin";
    }

    currentState = OTA_STATE_STARTING;
    progressPercent = 0;
    errorMessage = "";
    taskRunning = true;

    // Starte Download in separatem FreeRTOS-Task
    xTaskCreatePinnedToCore(otaTask, "ota_task", 8192, this, 5, NULL, 0);
    return true;
}

void OtaUpdater::otaTask(void* parameter) {
    OtaUpdater* self = (OtaUpdater*)parameter;
    self->runHttpDownload();
    self->taskRunning = false;
    vTaskDelete(NULL);
}

void OtaUpdater::runHttpDownload() {
    Serial.printf("[OTA] Starte Firmware-Download von: %s\n", targetUrl.c_str());

    NetworkClientSecure client;
    client.setInsecure(); // GitHub Zertifikate ohne Root-CA Bundle akzeptieren

    HTTPClient http;
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.setTimeout(15000);

    if (!http.begin(client, targetUrl)) {
        currentState = OTA_STATE_ERROR;
        errorMessage = "Verbindung zu GitHub konnte nicht aufgebaut werden.";
        Serial.println("[OTA-FEHLER] http.begin fehlgeschlagen.");
        return;
    }

    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
        currentState = OTA_STATE_ERROR;
        if (httpCode == 404) {
            errorMessage = "Datei im GitHub-Repository nicht gefunden (404).";
        } else if (httpCode < 0) {
            errorMessage = "Netzwerkfehler: " + http.errorToString(httpCode);
        } else {
            errorMessage = "Server meldet Fehlercode: " + String(httpCode);
        }
        Serial.printf("[OTA-FEHLER] HTTP-Code: %d (%s)\n", httpCode, errorMessage.c_str());
        http.end();
        return;
    }

    int contentLength = http.getSize();
    Serial.printf("[OTA] Dateigroesse: %d Bytes\n", contentLength);

    if (contentLength <= 0) {
        currentState = OTA_STATE_ERROR;
        errorMessage = "Server hat keine Dateigroesse gemeldet.";
        http.end();
        return;
    }

    bool canBegin = Update.begin(contentLength);
    if (!canBegin) {
        currentState = OTA_STATE_ERROR;
        errorMessage = "Nicht genuegend freier Flash-Speicher verfuegbar.";
        Serial.printf("[OTA-FEHLER] Update.begin fehlgeschlagen. Code: %u\n", Update.getError());
        http.end();
        return;
    }

    currentState = OTA_STATE_DOWNLOADING;
    WiFiClient* stream = http.getStreamPtr();
    size_t written = 0;
    uint8_t buff[1024];

    while (http.connected() && (written < (size_t)contentLength)) {
        size_t available = stream->available();
        if (available) {
            int toRead = available > sizeof(buff) ? sizeof(buff) : available;
            int bytesRead = stream->readBytes(buff, toRead);
            if (bytesRead > 0) {
                size_t w = Update.write(buff, bytesRead);
                if (w != (size_t)bytesRead) {
                    currentState = OTA_STATE_ERROR;
                    errorMessage = "Fehler beim Schreiben in den Flash-Speicher.";
                    Update.abort();
                    http.end();
                    return;
                }
                written += w;
                progressPercent = (int)((written * 100) / contentLength);
            }
        }
        vTaskDelay(1);
    }

    if (Update.end(true)) {
        if (Update.isFinished()) {
            Serial.println("[OTA] Firmware-Update erfolgreich geschrieben!");
            currentState = OTA_STATE_SUCCESS;
            progressPercent = 100;
            restartPending = true;
            restartTimer = millis();
        } else {
            currentState = OTA_STATE_ERROR;
            errorMessage = "Update nicht vollstaendig beendet.";
            Serial.println("[OTA-FEHLER] Update.isFinished() ist false.");
        }
    } else {
        currentState = OTA_STATE_ERROR;
        errorMessage = "Fehler beim Abschliessen des Updates (Code: " + String(Update.getError()) + ")";
        Serial.printf("[OTA-FEHLER] Update.end() fehlgeschlagen. Code: %u\n", Update.getError());
    }

    http.end();
}

void OtaUpdater::handleUploadStart() {
    currentState = OTA_STATE_DOWNLOADING;
    progressPercent = 0;
    errorMessage = "";
    Serial.println("[OTA-UPLOAD] Manueller Datei-Upload gestartet...");
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
        currentState = OTA_STATE_ERROR;
        errorMessage = "Nicht genuegend Speicherplatz vorhanden.";
        Serial.printf("[OTA-FEHLER] Update.begin fehlgeschlagen: %u\n", Update.getError());
    }
}

void OtaUpdater::handleUploadData(uint8_t* data, size_t len, size_t index, size_t total) {
    if (currentState == OTA_STATE_ERROR) return;

    if (Update.write(data, len) != len) {
        currentState = OTA_STATE_ERROR;
        errorMessage = "Schreibfehler beim Datei-Upload.";
        Update.abort();
        Serial.printf("[OTA-FEHLER] Update.write fehlgeschlagen: %u\n", Update.getError());
        return;
    }

    if (total > 0) {
        progressPercent = (int)(((index + len) * 100) / total);
    }
}

void OtaUpdater::handleUploadEnd(bool success) {
    if (!success || currentState == OTA_STATE_ERROR) {
        currentState = OTA_STATE_ERROR;
        if (errorMessage.length() == 0) {
            errorMessage = "Upload wurde abgebrochen oder unterbrochen.";
        }
        Update.abort();
        return;
    }

    if (Update.end(true)) {
        if (Update.isFinished()) {
            Serial.println("[OTA-UPLOAD] Datei erfolgreich installiert!");
            currentState = OTA_STATE_SUCCESS;
            progressPercent = 100;
            restartPending = true;
            restartTimer = millis();
        } else {
            currentState = OTA_STATE_ERROR;
            errorMessage = "Datei unvollstaendig.";
        }
    } else {
        currentState = OTA_STATE_ERROR;
        errorMessage = "Fehler beim Pruefen der Firmware (Code: " + String(Update.getError()) + ")";
    }
}

void OtaUpdater::update() {
    if (restartPending && (millis() - restartTimer >= 2000)) {
        Serial.println("[OTA] Starte ESP32 neu...");
        delay(100);
        esp_restart();
    }
}

OtaState OtaUpdater::getState() const {
    return currentState;
}

int OtaUpdater::getProgress() const {
    return progressPercent;
}

String OtaUpdater::getErrorMessage() const {
    return errorMessage;
}

String OtaUpdater::getStatusString() const {
    switch (currentState) {
        case OTA_STATE_IDLE: return "Bereit";
        case OTA_STATE_STARTING: return "Verbinde mit Server...";
        case OTA_STATE_DOWNLOADING: return "Herunterladen & Schreiben...";
        case OTA_STATE_FLASHING: return "Installieren...";
        case OTA_STATE_SUCCESS: return "Erfolgreich! Neustart laeuft...";
        case OTA_STATE_ERROR: return "Fehler: " + errorMessage;
        default: return "";
    }
}
