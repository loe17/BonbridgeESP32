#pragma once

#include <Arduino.h>

enum OtaState {
    OTA_STATE_IDLE = 0,
    OTA_STATE_STARTING = 1,
    OTA_STATE_DOWNLOADING = 2,
    OTA_STATE_FLASHING = 3,
    OTA_STATE_SUCCESS = 4,
    OTA_STATE_ERROR = 5
};

class OtaUpdater {
public:
    static OtaUpdater& instance();

    // Startet das Online-Update von der angegebenen URL (z.B. GitHub Raw oder Releases)
    bool startHttpUpdate(const String& url = "");

    // Abfrage des aktuellen Status und Fortschritts
    OtaState getState() const;
    int getProgress() const;
    String getStatusString() const;
    String getErrorMessage() const;

    // Behandlung von manuellem Datei-Upload aus dem Web-Browser
    void handleUploadStart();
    void handleUploadData(uint8_t* data, size_t len, size_t index, size_t total);
    void handleUploadEnd(bool success);

    // Wird in der Hauptschleife aufgerufen (z.B. für verzögerten Neustart)
    void update();

private:
    OtaUpdater();

    OtaState currentState;
    int progressPercent;
    String errorMessage;
    String targetUrl;
    bool taskRunning;
    bool restartPending;
    unsigned long restartTimer;

    static void otaTask(void* parameter);
    void runHttpDownload();
};
