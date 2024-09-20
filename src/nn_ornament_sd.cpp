#include "nn_ornament_sd.h"
#include <esp_log.h>
#include <sstream>
#include <iomanip>

namespace {
    constexpr char TAG[] = "SD";
}

NNOrnamentSDWrapper::NNOrnamentSDWrapper() : cardType_(CARD_NONE), cardSize_(0), sd_(&SD) {}

NNOrnamentSDWrapper::InitResult NNOrnamentSDWrapper::init() {
    ESP_LOGI(TAG, "Initializing SD Card");
    SPI.begin(sclk_, miso_, mosi_, cs_);
    
    if (!sd_->begin(cs_)) {
        ESP_LOGE(TAG, "Card Mount Failed");
        return InitResult::FAIL_MOUNT;
    }
    
    cardType_ = sd_->cardType();
    
    if (cardType_ == CARD_NONE) {
        ESP_LOGE(TAG, "No SD card attached");
        return InitResult::FAIL_ATTACH;
    }
    
    cardSize_ = sd_->cardSize() / (1024 * 1024);
    
    ESP_LOGD(TAG, "SD Card Type: %s", getCardTypeString().c_str());
    ESP_LOGD(TAG, "SD Card Size: %lluMB", cardSize_);
    ESP_LOGI(TAG, "SD Card Initialized");
    
    return InitResult::SUCCESS;
}

uint64_t NNOrnamentSDWrapper::getCardSize() const {
    return cardSize_;
}

std::string NNOrnamentSDWrapper::getCardTypeString() const {
    switch (cardType_) {
        case CARD_MMC: return "MMC";
        case CARD_SD: return "SDSC";
        case CARD_SDHC: return "SDHC";
        default: return "UNKNOWN";
    }
}

void NNOrnamentSDWrapper::listDir(const char* dirname, uint8_t levels) {
    ESP_LOGI(TAG, "Listing directory: %s", dirname);

    File root = sd_->open(dirname);
    if (!root) {
        ESP_LOGE(TAG, "Failed to open directory");
        return;
    }
    if (!root.isDirectory()) {
        ESP_LOGE(TAG, "Not a directory");
        return;
    }

    File file = root.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            ESP_LOGI(TAG, "  DIR : %s", file.name());
            if (levels) {
                listDir(file.path(), levels - 1);
            }
        } else {
            ESP_LOGI(TAG, "  FILE: %s  SIZE: %d", file.name(), file.size());
        }
        file = root.openNextFile();
    }
}

bool NNOrnamentSDWrapper::createDir(const char* path) {
    ESP_LOGI(TAG, "Creating Dir: %s", path);
    if (sd_->mkdir(path)) {
        ESP_LOGI(TAG, "Dir created");
        return true;
    } else {
        ESP_LOGE(TAG, "mkdir failed");
        return false;
    }
}

bool NNOrnamentSDWrapper::removeDir(const char* path) {
    ESP_LOGI(TAG, "Removing Dir: %s", path);
    if (sd_->rmdir(path)) {
        ESP_LOGI(TAG, "Dir removed");
        return true;
    } else {
        ESP_LOGE(TAG, "rmdir failed");
        return false;
    }
}

std::string NNOrnamentSDWrapper::readFile(const char* path) {
    ESP_LOGI(TAG, "Reading file: %s", path);

    File file = sd_->open(path);
    if (!file) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return "";
    }

    std::string content;
    while (file.available()) {
        content += (char)file.read();
    }
    file.close();
    return content;
}

bool NNOrnamentSDWrapper::writeFile(const char* path, const char* message) {
    ESP_LOGI(TAG, "Writing file: %s", path);

    File file = sd_->open(path, FILE_WRITE);
    if (!file) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return false;
    }
    if (file.print(message)) {
        ESP_LOGI(TAG, "File written");
        file.close();
        return true;
    } else {
        ESP_LOGE(TAG, "Write failed");
        file.close();
        return false;
    }
}

bool NNOrnamentSDWrapper::appendFile(const char* path, const char* message) {
    ESP_LOGI(TAG, "Appending to file: %s", path);

    File file = sd_->open(path, FILE_APPEND);
    if (!file) {
        ESP_LOGE(TAG, "Failed to open file for appending");
        return false;
    }
    if (file.print(message)) {
        ESP_LOGI(TAG, "Message appended");
        file.close();
        return true;
    } else {
        ESP_LOGE(TAG, "Append failed");
        file.close();
        return false;
    }
}

bool NNOrnamentSDWrapper::renameFile(const char* path1, const char* path2) {
    ESP_LOGI(TAG, "Renaming file %s to %s", path1, path2);
    if (sd_->rename(path1, path2)) {
        ESP_LOGI(TAG, "File renamed");
        return true;
    } else {
        ESP_LOGE(TAG, "Rename failed");
        return false;
    }
}

bool NNOrnamentSDWrapper::deleteFile(const char* path) {
    ESP_LOGI(TAG, "Deleting file: %s", path);
    if (sd_->remove(path)) {
        ESP_LOGI(TAG, "File deleted");
        return true;
    } else {
        ESP_LOGE(TAG, "Delete failed");
        return false;
    }
}