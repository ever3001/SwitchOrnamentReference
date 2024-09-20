#include "display_task.h"

#include <AceButton.h>
#include <SD.h>
#include <TFT_eSPI.h>
#include <Update.h>
#include <WiFi.h>

#include <json11.hpp>

#include "nn_ornament_gif.h"
#include "nn_ornament_sd.h"
#include "nn_ornament_tft.h"

#include "config_manager.h"
#include "update_manager.h"

using namespace json11;

#if defined(LCD_MODULE_CMD_1)
typedef struct
{
    uint8_t cmd;
    uint8_t data[14];
    uint8_t len;
} lcd_cmd_t;

lcd_cmd_t lcd_st7789v[] = {
    {0x11, {0}, 0 | 0x80},
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
    {0xE1, {0XF0, 0X08, 0X0C, 0X0B, 0X09, 0X24, 0X2B, 0X22, 0X43, 0X38, 0X15, 0X16, 0X2F, 0X37}, 14},
};
#endif

NNOrnamentSDWrapper sdGlobal;
NNOrnamentTFTWrapper tftGlobal;
NNOrnamentGIFWrapper gifGlobal(sdGlobal, tftGlobal);

DisplayTask::DisplayTask(MainTask &main_task, const uint8_t task_core) : Task{"Display", 8192, 1, task_core}, main_task_(main_task)
{
    log_queue_ = xQueueCreate(10, sizeof(std::string *));
    assert(log_queue_ != NULL);

    event_queue_ = xQueueCreate(10, sizeof(Event));
    assert(event_queue_ != NULL);
}

int DisplayTask::enumerateGifs(const char *basePath, std::vector<std::string> &out_files)
{
    int amount = 0;
    File GifRootFolder = sdGlobal.getSD().open(basePath);
    if (!GifRootFolder)
    {
        ESP_LOGE("DISPLAY_TASK", "Failed to open directory");
        return 0;
    }

    if (!GifRootFolder.isDirectory())
    {
        ESP_LOGE("DISPLAY_TASK", "Not a directory");
        return 0;
    }

    File file = GifRootFolder.openNextFile();

    while (file)
    {
        if (!file.isDirectory())
        {
            out_files.push_back(file.name());
            amount++;
            file.close();
        }
        file = GifRootFolder.openNextFile();
    }
    GifRootFolder.close();
    ESP_LOGD("DISPLAY_TASK", "Found %d GIF files", amount);
    return amount;
}

// perform the actual update from a given stream
bool DisplayTask::performUpdate(Stream &updateSource, size_t updateSize)
{
    if (Update.begin(updateSize))
    {
        size_t written = Update.writeStream(updateSource);
        if (written == updateSize)
        {
            ESP_LOGI("MAIN_TASK", "Written : %d successfully", written);
        }
        else
        {
            ESP_LOGI("MAIN_TASK", "Written only : %d/%d. Retry?", written, updateSize);
        }
        if (Update.end())
        {
            ESP_LOGI("MAIN_TASK", "OTA done!");
            if (Update.isFinished())
            {
                ESP_LOGI("MAIN_TASK", "Update Success. Rebooting...");
                tft_.fillScreen(TFT_BLACK);
                tft_.drawString("Update successful!", 0, 0);
                return true;
            }
            else
            {
                ESP_LOGE("MAIN_TASK", "Update not finished? Something went wrong!");
                tft_.fillScreen(TFT_BLACK);
                tft_.drawString("Update error: unknown", 0, 0);
            }
        }
        else
        {
            uint8_t error = Update.getError();
            ESP_LOGE("MAIN_TASK", "Error Occurred. Error #: %d", error);
            tft_.fillScreen(TFT_BLACK);
            tft_.drawString("Update error: " + String(error), 0, 0);
        }
    }
    else
    {
        ESP_LOGE("MAIN_TASK", "Not enough space to begin OTA");
        tft_.fillScreen(TFT_BLACK);
        tft_.drawString("Not enough space", 0, 0);
    }
    return false;
}

// check given FS for valid firmware.bin and perform update if available
bool DisplayTask::updateFromFS(fs::FS &fs)
{
    tft_.fillScreen(TFT_BLACK);
    tft_.setTextDatum(TL_DATUM);

    File updateBin = fs.open("/firmware.bin");
    if (updateBin)
    {
        if (updateBin.isDirectory())
        {
            ESP_LOGE("MAIN_TASK", "Error, firmware.bin is not a file");
            updateBin.close();
            return false;
        }

        size_t updateSize = updateBin.size();

        bool update_successful = false;
        if (updateSize > 0)
        {
            ESP_LOGI("MAIN_TASK", "Try to start update");
            tft_.fillScreen(TFT_BLACK);
            tft_.drawString("Starting update...", 0, 0);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            update_successful = performUpdate(updateBin, updateSize);
        }
        else
        {
            ESP_LOGE("MAIN_TASK", "Error, file is empty");
        }

        updateBin.close();
        fs.remove("/firmware.bin");

        // Leave some time to read the update result message
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        return update_successful;
    }
    else
    {
        ESP_LOGE("MAIN_TASK", "No firmware.bin at sd root");
        return false;
    }
}

void DisplayTask::run()
{

    tftGlobal.init();

    bool isblinked = false;
    while (sdGlobal.init() != NNOrnamentSDWrapper::InitResult::SUCCESS)
    {
        isblinked = !isblinked;
        if (isblinked)
        {
            tft_.setTextColor(TFT_WHITE, TFT_BLACK);
        }
        else
        {
            tft_.setTextColor(TFT_BLACK, TFT_WHITE);
        }
        tft_.setTextDatum(TC_DATUM);
        tft_.drawString("INSERT SD", tft_.width() / 2, tft_.height() / 2);

        vTaskDelay(300 / portTICK_PERIOD_MS);
    }

    UpdateManager updateManager = UpdateManager(tftGlobal.getTFT());

    if (updateManager.checkForUpdate())
    {
        ESP.restart();
    }

    // #####################################################
    // CHANGES ABOVE THIS LINE MAY BREAK FIRMWARE UPDATES!!!
    // #####################################################

    vTaskDelay(3000 / portTICK_PERIOD_MS);

    // Load config from SD card
    ConfigManager config = ConfigManager();
    if (config.loadConfig("/config.json"))
    {
        main_task_.setConfig(config.getSSID(), config.getPassword(), config.getTimezone());
    }

    // Delay to avoid brownout while wifi is starting
    vTaskDelay(500 / portTICK_PERIOD_MS);

    // Open and play a GIF file
    if (gifGlobal.openFile("/gifs/boot.gif"))
    {
        tftGlobal.getTFT()->startWrite(); // Lock TFT chip select if using DMA
        while (gifGlobal.isOpen())
        {
            int playResult = gifGlobal.playFrame();
            if (!playResult)
            {
                ESP_LOGD("MAIN", "Closing /gifs/boot.gif");
                gifGlobal.closeFile();
            }
            yield(); // Allow other tasks to run
        }
        tftGlobal.getTFT()->endWrite(); // Release TFT chip select
    }
    else
    {
        ESP_LOGE("MAIN", "Failed to open GIF file");
    }

    std::vector<std::string> main_gifs;
    std::vector<std::string> christmas_gifs;

    int num_main_gifs = enumerateGifs("/gifs/main", main_gifs);
    int num_christmas_gifs = enumerateGifs("/gifs/christmas", christmas_gifs);
    int current_file = -1;
    std::string current_file_name = "";
    uint32_t minimum_loop_duration = 0;
    uint32_t start_millis = UINT32_MAX;

    bool last_christmas; // I gave you my heart...

    main_task_.registerEventQueue(event_queue_);

    State state = State::CHOOSE_GIF;
    int frame_delay = 0;
    uint32_t last_frame = 0;
    while (1)
    {
        bool left_button = false;
        bool right_button = false;
        Event event;
        if (xQueueReceive(event_queue_, &event, 0))
        {
            switch (event.type)
            {
            case EventType::BUTTON:
                if (event.button.event == ace_button::AceButton::kEventPressed)
                {
                    if (event.button.button_id == BUTTON_ID_LEFT)
                    {
                        left_button = true;
                    }
                    else if (event.button.button_id == BUTTON_ID_RIGHT)
                    {
                        right_button = true;
                    }
                }
                break;
            }
        }
        handleLogRendering();
        switch (state)
        {
        case State::CHOOSE_GIF:
            ESP_LOGI("MAIN_TASK", "Choosing gif");
            if (millis() - start_millis > minimum_loop_duration)
            {
                // Only change the file if we've exceeded the minimum loop duration
                if (isChristmas())
                {
                    if (num_christmas_gifs > 0)
                    {
                        current_file_name = "/gifs/christmas/" + christmas_gifs[current_file++ % num_christmas_gifs];
                        minimum_loop_duration = 30000;
                        ESP_LOGD("MAIN_TASK", "Chose christmas gif: %s\n", current_file_name.c_str());
                    }
                    else
                    {
                        continue;
                    }
                }
                else
                {
                    if (num_main_gifs > 0)
                    {
                        int next_file = current_file;
                        while (num_main_gifs > 1 && next_file == current_file)
                        {
                            next_file = random(num_main_gifs);
                        }
                        current_file = next_file;
                        current_file_name = "/gifs/main/" + main_gifs[current_file];
                        minimum_loop_duration = 0;
                        ESP_LOGD("MAIN_TASK", "Chose main gif: %s\n", current_file_name.c_str());
                    }
                    else
                    {
                        continue;
                    }
                }
                start_millis = millis();
            }
            if (!gifGlobal.openFile(current_file_name.c_str()))
            {
                continue;
            }
            last_frame = millis();
            vTaskDelay(50 / portTICK_PERIOD_MS);
            state = State::PLAY_GIF;
            break;
        case State::PLAY_GIF:
        {
            if (right_button)
            {
                gifGlobal.closeFile();
                int center = tft_.width() / 2;
                tft_.fillScreen(TFT_BLACK);
                tft_.setTextSize(2);
                tft_.setTextDatum(TC_DATUM);
                tft_.drawString("Merry Christmas!", center, 10);
                tft_.setTextSize(1);
                tft_.drawString("Designed and handmade", center, 50);
                tft_.drawString("by Scott Bezek", center, 60);
                tft_.drawString("Oakland, 2021", center, 80);

                if (WiFi.status() == WL_CONNECTED)
                {
                    tft_.setTextDatum(BL_DATUM);
                    tft_.drawString(String("IP: ") + WiFi.localIP().toString(), 5, tft_.height());
                }
                main_task_.setOtaEnabled(true);
                vTaskDelay(200 / portTICK_PERIOD_MS);
                state = State::SHOW_CREDITS;
                break;
            }
            bool is_christmas = isChristmas();
            bool christmas_changed = false;
            if (is_christmas != last_christmas)
            {
                last_christmas = is_christmas;
                christmas_changed = true;
            }

            if (left_button || christmas_changed)
            {
                // Force select new gif, even if we hadn't met the minimum loop duration yet
                minimum_loop_duration = 0;
                gifGlobal.closeFile();
                state = State::CHOOSE_GIF;
                break;
            }
            uint32_t time_since_last_frame = millis() - last_frame;
            if (time_since_last_frame > frame_delay)
            {
                // Time for the next frame; play it
                last_frame = millis();
                if (!gifGlobal.playFrame())
                {
                    gifGlobal.closeFile();
                    state = State::CHOOSE_GIF;
                    break;
                }
            }
            else
            {
                // Wait until it's time for the next frame, but up to 50ms max at a time to avoid stalling UI thread
                uint32_t minDelay = min((uint32_t)50, frame_delay - time_since_last_frame);
                vTaskDelay(minDelay / portTICK_PERIOD_MS);
            }

            break;
        }
        case State::SHOW_CREDITS:
            if (right_button)
            {
                // Exit credits
                main_task_.setOtaEnabled(false);
                state = State::CHOOSE_GIF;
                tft_.fillScreen(TFT_BLACK);
                vTaskDelay(200 / portTICK_PERIOD_MS);
            }
            break;
        }
    }
}

bool DisplayTask::isChristmas()
{
    tm local;
    return main_task_.getLocalTime(&local) && local.tm_mon == 11 && local.tm_mday == 25;
}

void DisplayTask::handleLogRendering()
{
    uint32_t now = millis();
    // Check for new message
    bool force_redraw = false;
    if (now - last_message_millis_ > 100)
    {
        std::string *log_string;
        if (xQueueReceive(log_queue_, &log_string, 0) == pdTRUE)
        {
            last_message_millis_ = now;
            force_redraw = true;
            strncpy(current_message_, log_string->c_str(), sizeof(current_message_));
            delete log_string;
        }
    }

    bool show = show_log_ && (now - last_message_millis_ < 3000);

    // if (show && (!message_visible_ || force_redraw))
    // {
    //     GifPlayer::set_max_line(124);
    //     tft_.fillRect(0, 124, DISPLAY_WIDTH, 11, TFT_BLACK);
    //     tft_.setTextSize(1);
    //     tft_.setTextDatum(TL_DATUM);
    //     tft_.drawString(current_message_, 3, 126);
    // }
    // else if (!show && message_visible_)
    // {
    //     tft_.fillRect(0, 124, DISPLAY_WIDTH, 11, TFT_BLACK);
    //     GifPlayer::set_max_line(-1);
    // }
    // message_visible_ = show;
}
