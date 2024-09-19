#include "nn_ornament_gif.h"

NNOrnamentGIF* NNOrnamentGIF::instance_ = nullptr;

NNOrnamentGIF::NNOrnamentGIF(NNOrnamentSD& sd, NNOrnamentTFT& tft)
    : sd_(sd), tft_(tft) {
    gif_.begin(LITTLE_ENDIAN_PIXELS);
    instance_ = this;
}

bool NNOrnamentGIF::openFile(const char* filename) {
    closeFile();
    file_ = sd_.getSD().open(filename, FILE_READ);
    ESP_LOGD("NNOrnamentGIF", "openFile: %s", filename);
    ESP_LOGD("NNOrnamentGIF", "File: %s", file_ ? "true" : "false");
    if (!file_) {
        return false;
    }
    
    int32_t size = file_.size();
    uint8_t *gifData = new uint8_t[size];
    file_.read(gifData, size);

    return gif_.open(gifData, size, drawCallback);
}

void NNOrnamentGIF::closeFile() {
    if (file_) {
        file_.close();
    }
    gif_.close();
}

bool NNOrnamentGIF::playFrame() {
    ESP_LOGD("NNOrnamentGIF", "isOpen: %d", isOpen());
    if (!isOpen()) {
        return false;
    }
    return gif_.playFrame(true, NULL);
}

void NNOrnamentGIF::drawCallback(GIFDRAW *pDraw) {
    if (instance_) {
        instance_->draw(pDraw);
    }
}

void NNOrnamentGIF::draw(GIFDRAW *pDraw) {
    TFT_eSPI* tft = tft_.getTFT();
    uint8_t *s;
    uint16_t *d, *usPalette, usTemp[320];
    int x, y, iWidth;

    iWidth = pDraw->iWidth;
    if (iWidth > tft->width())
        iWidth = tft->width();
    usPalette = pDraw->pPalette;
    y = pDraw->iY + pDraw->y; // current line
    
    s = pDraw->pPixels;
    if (pDraw->ucDisposalMethod == 2) // restore to background color
    {
        for (x=0; x<iWidth; x++)
        {
            if (s[x] == pDraw->ucTransparent)
                s[x] = pDraw->ucBackground;
        }
        pDraw->ucHasTransparency = 0;
    }
    // Apply the new pixels to the main image
    if (pDraw->ucHasTransparency) // if transparency used
    {
        uint8_t *pEnd, c, ucTransparent = pDraw->ucTransparent;
        int x, iCount;
        pEnd = s + iWidth;
        x = 0;
        iCount = 0; // count non-transparent pixels
        while(x < iWidth)
        {
            c = ucTransparent-1;
            d = usTemp;
            while (c != ucTransparent && s < pEnd)
            {
                c = *s++;
                if (c == ucTransparent) // done, stop
                {
                    s--; // back up to treat it like transparent
                }
                else // opaque
                {
                    *d++ = usPalette[c];
                    iCount++;
                }
            } // while looking for opaque pixels
            if (iCount) // any opaque pixels?
            {
                tft->setAddrWindow(pDraw->iX+x, y, iCount, 1);
                tft->pushColors(usTemp, iCount, true);
                x += iCount;
                iCount = 0;
            }
            // no, look for a run of transparent pixels
            c = ucTransparent;
            while (c == ucTransparent && s < pEnd)
            {
                c = *s++;
                if (c == ucTransparent)
                    x++;
                else
                    s--; 
            }
        }
    }
    else
    {
        s = pDraw->pPixels;
        // Translate the 8-bit pixels through the RGB565 palette (already byte reversed)
        for (x=0; x<iWidth; x++)
            usTemp[x] = usPalette[*s++];
        tft->setAddrWindow(pDraw->iX, y, iWidth, 1);
        tft->pushColors(usTemp, iWidth, true);
    }
}