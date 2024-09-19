#include "nn_ornament_tft.h"
#include <esp_log.h>

namespace {
    constexpr char TAG[] = "TFT";
}

const std::array<LcdCommand, 14> NNOrnamentTFT::lcd_st7789v_ = {{
    {0x11, {0}, 0x80},
    {0x3A, {0X05}, 1},
    {0xB2, {0X0B, 0X0B, 0X00, 0X33, 0X33}, 5},
    {0xB7, {0X75}, 1},
    {0xBB, {0X28}, 1},
    {0xC0, {0X2C}, 1},
    {0xC2, {0X01}, 1},
    {0xC3, {0X1F}, 1},
    {0xC6, {0X13}, 1},
    {0xD0, {0XA7}, 1},
    {0xD0, {0XA4, 0XA1}, 2},
    {0xD6, {0XA1}, 1},
    {0xE0, {0XF0, 0X05, 0X0A, 0X06, 0X06, 0X03, 0X2B, 0X32, 0X43, 0X36, 0X11, 0X10, 0X2B, 0X32}, 14},
    {0xE1, {0XF0, 0X08, 0X0C, 0X0B, 0X09, 0X24, 0X2B, 0X22, 0X43, 0X38, 0X15, 0X16, 0X2F, 0X37}, 14}
}};

NNOrnamentTFT::NNOrnamentTFT() : tft_() {}

void NNOrnamentTFT::init() {
    ESP_LOGI(TAG, "Initializing TFT");
    
    pinMode(pinPowerOn_, OUTPUT);
    digitalWrite(pinPowerOn_, HIGH);

    tft_.init();

#ifdef LCD_MODULE_CMD_1
    for (const auto& cmd : lcd_st7789v_) {
        tft_.writecommand(cmd.cmd);
        for (int j = 0; j < (cmd.len & 0x7f); ++j) {
            tft_.writedata(cmd.data[j]);
        }

        if (cmd.len & 0x80) {
            delay(120);
        }
    }
#endif

#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5,0,0)
    ledcSetup(0, 2000, 8);
    ledcAttachPin(pinLcdBl_, 0);
    ledcWrite(0, 255);
#else
    ledcAttach(pinLcdBl_, 200, 8);
    ledcWrite(pinLcdBl_, 255);
#endif

    tft_.setRotation(1);
    tft_.setTextSize(1);
    tft_.fillScreen(TFT_BLACK);
    tft_.setTextColor(TFT_GREEN, TFT_BLACK);
    tft_.drawString("Welcome", 0, 0, 2);
    
    ESP_LOGI(TAG, "TFT Initialized");
}