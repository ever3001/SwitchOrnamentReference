#pragma once

#include <cstdint>
#include <array>
#include <TFT_eSPI.h>

// Check for tft pins
#ifndef PIN_POWER_ON
    #error "PIN_POWER_ON is not defined. Please define it before including this file."
#endif

#ifndef TFT_BL
    #error "TFT_BL is not defined. Please define it before including this file."
#endif

#ifndef TFT_WR
    #error "TFT_WR is not defined. Please define it before including this file."
#endif

#ifndef TFT_RD
    #error "TFT_RD is not defined. Please define it before including this file."
#endif

#ifndef TFT_CS
    #error "TFT_CS is not defined. Please define it before including this file."
#endif

#ifndef TFT_DC
    #error "TFT_DC is not defined. Please define it before including this file."
#endif

#ifndef TFT_RST
    #error "TFT_RST is not defined. Please define it before including this file."
#endif

#ifndef TFT_D0
    #error "TFT_D0 is not defined. Please define it before including this file."
#endif

#ifndef TFT_D1
    #error "TFT_D1 is not defined. Please define it before including this file."
#endif

#ifndef TFT_D2
    #error "TFT_D2 is not defined. Please define it before including this file."
#endif

#ifndef TFT_D3
    #error "TFT_D3 is not defined. Please define it before including this file."
#endif

#ifndef TFT_D4
    #error "TFT_D4 is not defined. Please define it before including this file."
#endif

#ifndef TFT_D5
    #error "TFT_D5 is not defined. Please define it before including this file."
#endif

#ifndef TFT_D6
    #error "TFT_D6 is not defined. Please define it before including this file."
#endif

#ifndef TFT_D7
    #error "TFT_D7 is not defined. Please define it before including this file."
#endif

#ifndef TFT_BL
    #error "TFT_BL is not defined. Please define it before including this file."
#endif

#ifndef TFT_WIDTH
    #error "TFT_WIDTH is not defined. Please define it before including this file."
#endif

#ifndef TFT_HEIGHT
    #error "TFT_HEIGHT is not defined. Please define it before including this file."
#endif



struct LcdCommand {
    uint8_t cmd;
    std::array<uint8_t, 14> data;
    uint8_t len;
};

class NNOrnamentTFTWrapper {
public:
    NNOrnamentTFTWrapper();
    ~NNOrnamentTFTWrapper() = default;

    NNOrnamentTFTWrapper(const NNOrnamentTFTWrapper&) = delete;
    NNOrnamentTFTWrapper& operator=(const NNOrnamentTFTWrapper&) = delete;
    NNOrnamentTFTWrapper(NNOrnamentTFTWrapper&&) = delete;
    NNOrnamentTFTWrapper& operator=(NNOrnamentTFTWrapper&&) = delete;

    void init();

    TFT_eSPI* getTFT() { return &tft_; }
    
    void startWrite() { tft_.startWrite(); }
    void endWrite() { tft_.endWrite(); }

private:
    TFT_eSPI tft_;
    static constexpr uint16_t pinPowerOn_ = PIN_POWER_ON;
    static constexpr uint16_t pinLcdBl_ = TFT_BL;

    static const std::array<LcdCommand, 14> lcd_st7789v_;
};

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5,0,0)
#error  "The current version is not supported for the time being, please use a version below Arduino ESP32 3.0"
#endif