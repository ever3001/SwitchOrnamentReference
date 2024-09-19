#pragma once

#include <SD.h>
#include <json11.hpp>

class ConfigManager
{
public:
    bool loadConfig(std::string filename = "/config.json");
    bool getShowLog() const;
    const char *getSSID() const;
    const char *getPassword() const;
    const char *getTimezone() const;

private:
    esp_err_t assertJson() const;
    json11::Json json_ = nullptr;
};