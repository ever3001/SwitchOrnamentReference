#pragma once

#include <AnimatedGIF.h>
#include "nn_ornament_sd.h"
#include "nn_ornament_tft.h"

#define BUFFER_SIZE 256 // Optimum is >= GIF width or integral division of width

class NNOrnamentGIF {
public:
    NNOrnamentGIF(NNOrnamentSD& sd, NNOrnamentTFT& tft);
    ~NNOrnamentGIF() = default;

    NNOrnamentGIF(const NNOrnamentGIF&) = delete;
    NNOrnamentGIF& operator=(const NNOrnamentGIF&) = delete;
    NNOrnamentGIF(NNOrnamentGIF&&) = delete;
    NNOrnamentGIF& operator=(NNOrnamentGIF&&) = delete;

    bool openFile(const char* filename);
    void closeFile();
    bool playFrame();
    bool isOpen() const { return file_; }

private:
    static void drawCallback(GIFDRAW *pDraw);
    void draw(GIFDRAW *pDraw);

    NNOrnamentSD& sd_;
    NNOrnamentTFT& tft_;
    AnimatedGIF gif_;
    File file_;
    uint16_t usTemp_[BUFFER_SIZE];

    static NNOrnamentGIF* instance_;
};