#include "usb_printer.h"
#include <esp_log.h>
#include <string.h>

static const char* TAG = "UsbPrinter";

#define CLIENT_NUM_EVENT_MSG   5
#define ACTION_OPEN_DEV        0x01
#define ACTION_GET_DEV_INFO    0x02
#define ACTION_CLAIM_INTF      0x04
#define ACTION_TRANSFER        0x08
#define ACTION_CLOSE_DEV       0x10

static void usb_host_lib_task(void *pvParameters) {
    while (1) {
        uint32_t event_flags;
        esp_err_t err = usb_host_lib_handle_events(portMAX_DELAY, &event_flags);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "usb_host_lib_handle_events error: %d", err);
        }
        if (event_flags & USB_HOST_LIB_EVENT_FLAGS_NO_CLIENTS) {
            ESP_LOGI(TAG, "No clients in USB host");
        }
        if (event_flags & USB_HOST_LIB_EVENT_FLAGS_ALL_FREE) {
            ESP_LOGI(TAG, "USB host all free");
        }
    }
}

static void client_event_cb(const usb_host_client_event_msg_t *event_msg, void *arg) {
    UsbPrinter *printer = (UsbPrinter *)arg;
    if (event_msg && printer) {
        printer->handleDeviceEvent((usb_host_client_event_msg_t *)event_msg);
    }
}

static void usb_client_task(void *pvParameters) {
    UsbPrinter *printer = (UsbPrinter *)pvParameters;
    while (1) {
        usb_host_client_handle_events(printer->instance().client_hdl, portMAX_DELAY);
    }
}

UsbPrinter::UsbPrinter() :
    initialized(false),
    deviceConnected(false),
    deviceName("Kein Drucker angeschlossen"),
    outEndpoint(0),
    outMaxPacketSize(64),
    dev_hdl(NULL),
    client_hdl(NULL),
    writeMutex(NULL)
{
}

UsbPrinter& UsbPrinter::instance() {
    static UsbPrinter inst;
    return inst;
}

bool UsbPrinter::begin() {
    if (initialized) return true;

    writeMutex = xSemaphoreCreateMutex();

    ESP_LOGI(TAG, "Installing USB Host Library...");
    const usb_host_config_t host_config = {
        .skip_phy_setup = false,
        .intr_flags = ESP_INTR_FLAG_LEVEL1,
    };
    esp_err_t err = usb_host_install(&host_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to install USB Host: %s", esp_err_to_name(err));
        return false;
    }

    xTaskCreatePinnedToCore(usb_host_lib_task, "usb_lib", 4096, NULL, 5, NULL, 0);

    const usb_host_client_config_t client_config = {
        .is_async = false,
        .max_num_event_msg = CLIENT_NUM_EVENT_MSG,
        .async = {
            .client_event_callback = client_event_cb,
            .callback_arg = this
        }
    };

    err = usb_host_client_register(&client_config, &client_hdl);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register USB client: %s", esp_err_to_name(err));
        return false;
    }

    xTaskCreatePinnedToCore(usb_client_task, "usb_client", 4096, this, 5, NULL, 0);

    initialized = true;
    ESP_LOGI(TAG, "USB Host ready and waiting for ESC/POS printer.");
    return true;
}

void UsbPrinter::handleDeviceEvent(usb_host_client_event_msg_t* event_msg) {
    switch (event_msg->event) {
        case USB_HOST_CLIENT_EVENT_NEW_DEV: {
            uint8_t dev_addr = event_msg->new_dev.address;
            ESP_LOGI(TAG, "New USB device detected at address %d", dev_addr);

            usb_device_handle_t new_dev_hdl = NULL;
            esp_err_t err = usb_host_device_open(client_hdl, dev_addr, &new_dev_hdl);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Failed to open USB device: %s", esp_err_to_name(err));
                return;
            }

            const usb_device_desc_t *dev_desc;
            usb_host_get_device_descriptor(new_dev_hdl, &dev_desc);

            const usb_config_desc_t *config_desc;
            usb_host_get_active_config_descriptor(new_dev_hdl, &config_desc);

            uint8_t bulkOutEp = 0;
            uint16_t maxPacket = 64;
            uint8_t intfNum = 0;
            bool foundPrinter = false;

            // Suche nach Drucker-Schnittstelle oder Bulk-OUT-Endpunkt
            int offset = 0;
            const uint8_t *p = (const uint8_t *)config_desc;
            while (offset < config_desc->wTotalLength) {
                const usb_standard_desc_t *desc = (const usb_standard_desc_t *)(p + offset);
                if (desc->bDescriptorType == USB_B_DESCRIPTOR_TYPE_INTERFACE) {
                    const usb_intf_desc_t *intf = (const usb_intf_desc_t *)desc;
                    intfNum = intf->bInterfaceNumber;
                    // Class 0x07 = Printer, 0xFF = Vendor Specific, oder 0x00
                    if (intf->bInterfaceClass == 0x07 || intf->bInterfaceClass == 0xFF || intf->bInterfaceClass == 0x00) {
                        foundPrinter = true;
                    }
                } else if (desc->bDescriptorType == USB_B_DESCRIPTOR_TYPE_ENDPOINT) {
                    const usb_ep_desc_t *ep = (const usb_ep_desc_t *)desc;
                    if ((ep->bmAttributes & USB_BM_ATTRIBUTES_XFERTYPE_MASK) == USB_BM_ATTRIBUTES_XFER_BULK) {
                        if ((ep->bEndpointAddress & USB_B_ENDPOINT_ADDRESS_DIRECTION_MASK) == USB_B_ENDPOINT_ADDRESS_DIR_OUT) {
                            bulkOutEp = ep->bEndpointAddress;
                            maxPacket = ep->wMaxPacketSize;
                        }
                    }
                }
                offset += desc->bLength;
            }

            if (bulkOutEp != 0) {
                err = usb_host_interface_claim(client_hdl, new_dev_hdl, intfNum, 0);
                if (err == ESP_OK) {
                    char nameBuf[64];
                    snprintf(nameBuf, sizeof(nameBuf), "ESC/POS Drucker (VID:%04X PID:%04X)", 
                             dev_desc->idVendor, dev_desc->idProduct);
                    setDeviceConnected(true, String(nameBuf), bulkOutEp, maxPacket, new_dev_hdl);
                    ESP_LOGI(TAG, "Printer claimed successfully: %s, EP OUT: 0x%02X, MaxPacket: %d", 
                             nameBuf, bulkOutEp, maxPacket);
                } else {
                    ESP_LOGE(TAG, "Could not claim interface: %s", esp_err_to_name(err));
                    usb_host_device_close(client_hdl, new_dev_hdl);
                }
            } else {
                ESP_LOGW(TAG, "No Bulk OUT endpoint found on device.");
                usb_host_device_close(client_hdl, new_dev_hdl);
            }
            break;
        }

        case USB_HOST_CLIENT_EVENT_DEV_GONE: {
            ESP_LOGW(TAG, "USB device disconnected!");
            setDeviceConnected(false, "Kein Drucker angeschlossen", 0, 64, NULL);
            break;
        }

        default:
            break;
    }
}

void UsbPrinter::setDeviceConnected(bool connected, const String& name, uint8_t outEp, uint16_t maxPacketSize, usb_device_handle_t devHandle) {
    if (writeMutex) xSemaphoreTake(writeMutex, portMAX_DELAY);
    deviceConnected = connected;
    deviceName = name;
    outEndpoint = outEp;
    outMaxPacketSize = maxPacketSize;
    dev_hdl = devHandle;
    if (writeMutex) xSemaphoreGive(writeMutex);
}

bool UsbPrinter::isConnected() {
    return deviceConnected;
}

String UsbPrinter::getStatusString() {
    if (deviceConnected) {
        return "Bereit (" + deviceName + ")";
    }
    return "Nicht verbunden (USB-Kabel pruefen)";
}

String UsbPrinter::getDeviceName() {
    return deviceName;
}

static void transfer_cb(usb_transfer_t *transfer) {
    SemaphoreHandle_t done_sem = (SemaphoreHandle_t)transfer->context;
    if (done_sem) {
        xSemaphoreGive(done_sem);
    }
}

size_t UsbPrinter::write(const uint8_t* data, size_t len) {
    if (!deviceConnected || dev_hdl == NULL || outEndpoint == 0 || len == 0) {
        return 0;
    }

    if (writeMutex) xSemaphoreTake(writeMutex, portMAX_DELAY);

    size_t totalWritten = 0;
    SemaphoreHandle_t transferDone = xSemaphoreCreateBinary();

    while (totalWritten < len && deviceConnected) {
        size_t chunkSize = len - totalWritten;
        if (chunkSize > 512) {
            chunkSize = 512;
        }

        usb_transfer_t *transfer = NULL;
        esp_err_t err = usb_host_transfer_alloc(chunkSize, 0, &transfer);
        if (err != ESP_OK || transfer == NULL) {
            ESP_LOGE(TAG, "Failed to allocate USB transfer: %s", esp_err_to_name(err));
            break;
        }

        memcpy(transfer->data_buffer, data + totalWritten, chunkSize);
        transfer->num_bytes = chunkSize;
        transfer->device_handle = dev_hdl;
        transfer->bEndpointAddress = outEndpoint;
        transfer->callback = transfer_cb;
        transfer->context = (void*)transferDone;
        transfer->timeout_ms = 2000;

        err = usb_host_transfer_submit(transfer);
        if (err == ESP_OK) {
            if (xSemaphoreTake(transferDone, pdMS_TO_TICKS(2500)) == pdTRUE) {
                totalWritten += transfer->actual_num_bytes;
            } else {
                ESP_LOGE(TAG, "USB transfer timed out!");
                usb_host_endpoint_clear(dev_hdl, outEndpoint);
                usb_host_transfer_free(transfer);
                break;
            }
        } else {
            ESP_LOGE(TAG, "USB submit error: %s", esp_err_to_name(err));
            usb_host_transfer_free(transfer);
            break;
        }

        usb_host_transfer_free(transfer);
    }

    vSemaphoreDelete(transferDone);
    if (writeMutex) xSemaphoreGive(writeMutex);

    return totalWritten;
}

void UsbPrinter::sendEscPosCommand(const uint8_t* cmd, size_t len) {
    write(cmd, len);
}

void UsbPrinter::kickCashDrawer() {
    ESP_LOGI(TAG, "Sende Kassenladen-Oeffnungsimpuls...");
    // ESC p 0 25 250 (Pin 2)
    const uint8_t cmd1[] = { 0x1B, 0x70, 0x00, 0x19, 0xFA };
    write(cmd1, sizeof(cmd1));
    vTaskDelay(pdMS_TO_TICKS(50));
    // ESC p 1 25 250 (Pin 5)
    const uint8_t cmd2[] = { 0x1B, 0x70, 0x01, 0x19, 0xFA };
    write(cmd2, sizeof(cmd2));
}

void UsbPrinter::printTestSlip(const String& ipAddress, const String& activeInterface, const String& profileName) {
    ESP_LOGI(TAG, "Drucke Testbon...");
    String slip = "";
    // ESC @ - Reset Drucker
    const uint8_t initCmd[] = { 0x1B, 0x40 };
    write(initCmd, sizeof(initCmd));

    // Zentriert, Fett, Doppelte Groesse
    const uint8_t headerStyle[] = { 0x1B, 0x61, 0x01, 0x1B, 0x45, 0x01, 0x1B, 0x21, 0x30 };
    write(headerStyle, sizeof(headerStyle));
    slip += "BONBRIDGE ESP32\n";
    write((const uint8_t*)slip.c_str(), slip.length());
    slip = "";

    // Normaler Stil
    const uint8_t normalStyle[] = { 0x1B, 0x45, 0x00, 0x1B, 0x21, 0x00 };
    write(normalStyle, sizeof(normalStyle));
    slip += "Drucker-Bruecke fuer POS & Gastro\n";
    slip += "==========================================\n";
    write((const uint8_t*)slip.c_str(), slip.length());
    slip = "";

    // Linksbuendig
    const uint8_t leftStyle[] = { 0x1B, 0x61, 0x00 };
    write(leftStyle, sizeof(leftStyle));

    slip += "Status        : " + getStatusString() + "\n";
    slip += "Verbindung    : " + activeInterface + "\n";
    slip += "IP-Adresse    : " + ipAddress + "\n";
    slip += "RAW-Port      : 9100\n";
    slip += "Druckprofil   : " + profileName + "\n";
    
    unsigned long sec = millis() / 1000;
    char upBuf[32];
    snprintf(upBuf, sizeof(upBuf), "%luh %lum %lus", sec / 3600, (sec % 3600) / 60, sec % 60);
    slip += "Laufzeit      : " + String(upBuf) + "\n";
    slip += "==========================================\n";
    write((const uint8_t*)slip.c_str(), slip.length());
    slip = "";

    // Zentriert, Abschluss & Schnitt
    const uint8_t centerStyle[] = { 0x1B, 0x61, 0x01 };
    write(centerStyle, sizeof(centerStyle));
    slip += "Testdruck erfolgreich!\n\n\n\n\n";
    write((const uint8_t*)slip.c_str(), slip.length());

    // GS V 66 3 - Papier vorschieben und abschneiden
    const uint8_t cutCmd[] = { 0x1D, 0x56, 0x42, 0x03 };
    write(cutCmd, sizeof(cutCmd));
}

void UsbPrinter::printNetworkAlert(const String& title, const String& message) {
    ESP_LOGW(TAG, "Drucke Netzwerk-Warnung!");
    // ESC @ - Reset Drucker
    const uint8_t initCmd[] = { 0x1B, 0x40 };
    write(initCmd, sizeof(initCmd));

    // Zentriert, doppelte Hoehe
    const uint8_t warnHeader[] = { 0x1B, 0x61, 0x01, 0x1B, 0x45, 0x01, 0x1B, 0x21, 0x20 };
    write(warnHeader, sizeof(warnHeader));
    
    String text = "!!! NETZWERK-WARNUNG !!!\n";
    write((const uint8_t*)text.c_str(), text.length());

    // Normaler Stil
    const uint8_t normalStyle[] = { 0x1B, 0x45, 0x00, 0x1B, 0x21, 0x00 };
    write(normalStyle, sizeof(normalStyle));
    text = "==========================================\n";
    text += title + "\n";
    text += message + "\n";
    text += "Bitte LAN-Kabel oder WLAN/Router pruefen.\n";
    text += "==========================================\n\n\n\n";
    write((const uint8_t*)text.c_str(), text.length());

    // Papier abschneiden
    const uint8_t cutCmd[] = { 0x1D, 0x56, 0x42, 0x03 };
    write(cutCmd, sizeof(cutCmd));
}
