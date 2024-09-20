#pragma once

#include <cstdint>
#include <SD.h>
#include <SPI.h>
#include <FS.h>
#include <string>

// Check for SD pin definitions
#ifndef SD_SCLK
    #error "SD_SCLK is not defined. Please define it before including this file."
#endif

#ifndef SD_MISO
    #error "SD_MISO is not defined. Please define it before including this file."
#endif

#ifndef SD_MOSI
    #error "SD_MOSI is not defined. Please define it before including this file."
#endif

#ifndef SD_CS
    #error "SD_CS is not defined. Please define it before including this file."
#endif

class NNOrnamentSDWrapper {
public:
    enum class InitResult {
        SUCCESS,
        FAIL_MOUNT,
        FAIL_ATTACH,
        UNKNOWN_CARD_TYPE
    };

    NNOrnamentSDWrapper();
    ~NNOrnamentSDWrapper() = default;

    NNOrnamentSDWrapper(const NNOrnamentSDWrapper&) = delete;
    NNOrnamentSDWrapper& operator=(const NNOrnamentSDWrapper&) = delete;
    NNOrnamentSDWrapper(NNOrnamentSDWrapper&&) = delete;
    NNOrnamentSDWrapper& operator=(NNOrnamentSDWrapper&&) = delete;

    InitResult init();
    uint64_t getCardSize() const;
    std::string getCardTypeString() const;
    SDFS& getSD() { return *sd_; }

    // New file and directory operations
    void listDir(const char* dirname, uint8_t levels);
    bool createDir(const char* path);
    bool removeDir(const char* path);
    std::string readFile(const char* path);
    bool writeFile(const char* path, const char* message);
    bool appendFile(const char* path, const char* message);
    bool renameFile(const char* path1, const char* path2);
    bool deleteFile(const char* path);

private:
    static constexpr uint8_t sclk_ = SD_SCLK;
    static constexpr uint8_t miso_ = SD_MISO;
    static constexpr uint8_t mosi_ = SD_MOSI;
    static constexpr uint8_t cs_ = SD_CS;

    uint8_t cardType_;
    uint64_t cardSize_;
    SDFS* sd_;
};