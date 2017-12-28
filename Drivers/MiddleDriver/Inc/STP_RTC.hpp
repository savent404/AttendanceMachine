#pragma once

#include "rtc.h"

class STP_RTC {
public:
    STP_RTC(RTC_HandleTypeDef* rtc)
        : handle(rtc)
    {
    }
    ~STP_RTC() {}

    HAL_StatusTypeDef getTime(uint8_t& hour, uint8_t& min, uint8_t& sec)
    {
        static RTC_TimeTypeDef time;
        HAL_StatusTypeDef res = HAL_RTC_GetTime(handle, &time, RTC_FORMAT_BIN);
        hour = time.Hours;
        min = time.Minutes;
        sec = time.Seconds;
        return res;
    }
    HAL_StatusTypeDef setTime(const uint8_t& hour, const uint8_t& min, const uint8_t& sec)
    {
        RTC_TimeTypeDef time;

        time.Hours = hour + 1;
        time.Minutes = min;
        time.Seconds = sec;
        if (time.Hours > 24) {
            time.Hours = 0;
        }
        if (time.Minutes >= 60) {
            time.Minutes = 0;
        }
        if (time.Seconds >= 60) {
            time.Seconds = 0;
        }

        return HAL_RTC_SetTime(handle, &time, RTC_FORMAT_BIN);
    }

private:
    RTC_HandleTypeDef* handle;
};
