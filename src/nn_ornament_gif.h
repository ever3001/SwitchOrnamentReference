#pragma once

#include <AnimatedGIF.h>
#include "nn_ornament_sd.h"
#include "nn_ornament_tft.h"

#define BUFFER_SIZE 256 // Optimum is >= GIF width or integral division of width

class NNOrnamentGIFWrapper {
public:
    NNOrnamentGIFWrapper(NNOrnamentSDWrapper& sd, NNOrnamentTFTWrapper& tft);
    ~NNOrnamentGIFWrapper() = default;

    NNOrnamentGIFWrapper(const NNOrnamentGIFWrapper&) = delete;
    NNOrnamentGIFWrapper& operator=(const NNOrnamentGIFWrapper&) = delete;
    NNOrnamentGIFWrapper(NNOrnamentGIFWrapper&&) = delete;
    NNOrnamentGIFWrapper& operator=(NNOrnamentGIFWrapper&&) = delete;

    bool openFile(const char* filename);
    void closeFile();
    bool playFrame();
    bool isOpen() const { return file_; }

private:
    static void drawCallback(GIFDRAW *pDraw);
    void draw(GIFDRAW *pDraw);

    NNOrnamentSDWrapper& sd_;
    NNOrnamentTFTWrapper& tft_;
    AnimatedGIF gif_;
    File file_;
    uint16_t usTemp_[BUFFER_SIZE];

    static NNOrnamentGIFWrapper* instance_;
};