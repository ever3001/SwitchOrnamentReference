#include "config_manager.h"
#include <Arduino.h>

bool ConfigManager::loadConfig(std::string filename)
{
    File configFile = SD.open(filename.c_str());
    if (!configFile)
    {
        ESP_LOGE("CONFIG_MANAGER", "Failed to open config file");
        return false;
    }

    size_t size = configFile.size();
    std::unique_ptr<char[]> buf(new char[size]);
    configFile.readBytes(buf.get(), size);
    configFile.close();

    std::string err;
    json_ = json11::Json::parse(buf.get(), err);
    if (!err.empty())
    {
        ESP_LOGE("CONFIG_MANAGER", "Failed to parse config file: %s", err.c_str());
        return false;
    }

    ESP_LOGI("CONFIG_MANAGER", "Config loaded successfully");
    return true;
}

esp_err_t ConfigManager::assertJson() const
{
    if (json_ == nullptr){
        return ESP_OK;
    }
    return ESP_FAIL;
}

bool ConfigManager::getShowLog() const
{
    ESP_ERROR_CHECK(assertJson());
    ESP_LOGD("CONFIG_MANAGER", "Show log: %s", json_["show_log"].string_value().c_str());
    return json_["show_log"].bool_value();
}

const char *ConfigManager::getSSID() const
{
    ESP_LOGD("CONFIG_MANAGER", "SSID: %s", json_["ssid"].string_value().c_str());
    return json_["ssid"].string_value().c_str();
}
const char *ConfigManager::getPassword() const
{
    ESP_LOGD("CONFIG_MANAGER", "Password: %s", json_["password"].string_value().c_str());
    return json_["password"].string_value().c_str();
}
const char *ConfigManager::getTimezone() const
{
    ESP_LOGD("CONFIG_MANAGER", "Timezone: %s", json_["timezone"].string_value().c_str());
    return json_["timezone"].string_value().c_str();
}
