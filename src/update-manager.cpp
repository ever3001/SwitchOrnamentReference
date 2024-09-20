#include "update_manager.h"
#include <Arduino.h>

UpdateManager::UpdateManager(TFT_eSPI* tft) : tft_(tft) {}

bool UpdateManager::checkForUpdate() {
    return updateFromFS(SD);
}

bool UpdateManager::performUpdate(Stream& updateSource, size_t updateSize) {
    if (Update.begin(updateSize)) {
        size_t written = Update.writeStream(updateSource);
        if (written == updateSize) {
            ESP_LOGI("UPDATE_MANAGER", "Written: %d successfully", written);
        } else {
            ESP_LOGI("UPDATE_MANAGER", "Written only: %d/%d. Retry?", written, updateSize);
        }
        if (Update.end()) {
            ESP_LOGI("UPDATE_MANAGER", "OTA done!");
            if (Update.isFinished()) {
                ESP_LOGI("UPDATE_MANAGER", "Update successful. Rebooting...");
                tft_->fillScreen(TFT_BLACK);
                tft_->drawString("Update successful!", 0, 0);
                return true;
            } else {
                ESP_LOGE("UPDATE_MANAGER", "Update not finished? Something went wrong!");
            }
        } else {
            ESP_LOGE("UPDATE_MANAGER", "Error Occurred. Error #: %d", Update.getError());
        }
    } else {
        ESP_LOGE("UPDATE_MANAGER", "Not enough space to begin OTA");
    }
    return false;
}

bool UpdateManager::updateFromFS(fs::FS& fs) {
    File updateBin = fs.open("/firmware.bin");
    if (updateBin) {
        if (updateBin.isDirectory()) {
            ESP_LOGE("UPDATE_MANAGER", "Error, firmware.bin is not a file");
            updateBin.close();
            return false;
        }

        size_t updateSize = updateBin.size();

        if (updateSize > 0) {
            ESP_LOGI("UPDATE_MANAGER", "Try to start update");
            tft_->fillScreen(TFT_BLACK);
            tft_->drawString("Starting update...", 0, 0);
            bool result = performUpdate(updateBin, updateSize);
            updateBin.close();
            fs.remove("/firmware.bin");
            return result;
        } else {
            ESP_LOGE("UPDATE_MANAGER", "Error, file is empty");
        }

        updateBin.close();
    } else {
        ESP_LOGE("UPDATE_MANAGER", "Could not load firmware.bin from sd root");
    }
    return false;
}
