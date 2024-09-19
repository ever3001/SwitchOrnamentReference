#pragma once

#include <Arduino.h>

#include <AceButton.h>

#include "event.h"
#include "task.h"
#include <vector>

class MainTask : public Task<MainTask>, public ace_button::IEventHandler {
    friend class Task<MainTask>; // Allow base Task to invoke protected run()

    public:
        MainTask(const uint8_t task_core);
        virtual ~MainTask();

        void setConfig(const char* wifi_ssid, const char* wifi_password, const char* timezone);
        bool getLocalTime(tm* t);
        void setOtaEnabled(bool enabled);
        void registerEventQueue(QueueHandle_t queue);

        void handleEvent(ace_button::AceButton* button, uint8_t event_type, uint8_t button_state) override;

    protected:
        void run();

    private:

        void publishEvent(Event event);
        
        SemaphoreHandle_t semaphore_;

        String wifi_ssid_;
        String wifi_password_;
        String timezone_;

        bool ntp_synced_ = false;

        std::vector<QueueHandle_t> event_queues_;
};
