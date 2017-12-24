#pragma once

#include "gpio.h"

class STP_Bell {
    public:
    STP_Bell() {
        mute();
    }
    ~STP_Bell() {
        mute();
    }
    void beep() {
        HAL_GPIO_WritePin(BELL_GPIO_Port, BELL_Pin, GPIO_PIN_SET);
    }
    void mute() {
        HAL_GPIO_WritePin(BELL_GPIO_Port, BELL_Pin, GPIO_PIN_RESET);
    }
};
