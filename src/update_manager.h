#pragma once

#include <Update.h>
#include <SD.h>
#include <TFT_eSPI.h>

class UpdateManager {
public:
    UpdateManager(TFT_eSPI* tft);
    bool checkForUpdate();

private:
    bool performUpdate(Stream& updateSource, size_t updateSize);
    bool updateFromFS(fs::FS& fs);

    TFT_eSPI* tft_;
};