#pragma once

#include <TFT_eSPI.h>
#include <vector>
#include <string>
#include "nn_ornament_gif.h"
#include "nn_ornament_sd.h"
#include "nn_ornament_tft.h"

class GifManager {
public:
    GifManager(TFT_eSPI& tft);
    void playBootAnimation(std::string path_to_boot_gif = "/gifs/boot.gif");
    void chooseNextGif(bool is_christmas);
    bool playFrame();

private:
    void enumerateGifs(const char* basePath, std::vector<std::string>& out_files);

    TFT_eSPI& tft_;
    NNOrnamentSDWrapper sd_;
    NNOrnamentTFTWrapper tft_wrapper_;
    NNOrnamentGIFWrapper gif_;
    std::vector<std::string> main_gifs_;
    std::vector<std::string> christmas_gifs_;
    int current_file_;
    std::string current_file_name_;
};