#pragma once

#include "usart.h"
#include <stdio.h>
#include <string.h>

#define LCD_UART (&huart3)

#define LCD_CLEAR "CLS(0);"
extern char LCD_WELCOME[];
extern char TEXT_NFC_001[];
extern char TEST_FINGER_001[];
extern char TEXT_FINGER_002[];
extern char TEXT_TIME[];
extern char TEXT_PASSWORD[];
extern char TEXT_ROOMID[];
extern char TEXT_UNKNOWUSER[];
extern char TEXT_REPLACE[];
extern char TEXT_CANCEL[];
extern char TEXT_CHOOSE_MODE_1[];
extern char TEXT_CHOOSE_MODE_2[];
extern char TEXT_ERROR_UNKNOW[];
extern char TEXT_ERROR_BREAKIN[];
extern char TEXT_ERROR_LIMIT[];
extern char TEXT_ERROR_CHAT[];

class STP_LCD {
public:
    STP_LCD() {}
    ~STP_LCD() {}
    static void send(const char* buffer, const size_t cnt, bool wait = true)
    {
        if (wait)
            while (HAL_GPIO_ReadPin(LCD_BUSY_GPIO_Port, LCD_BUSY_Pin) == GPIO_PIN_SET)
                ;
        HAL_UART_Transmit(LCD_UART, (uint8_t*)buffer, cnt, 100);
        HAL_UART_Transmit(LCD_UART, (uint8_t*)"\r\n", 2, 10);
        if (wait)
            while (HAL_GPIO_ReadPin(LCD_BUSY_GPIO_Port, LCD_BUSY_Pin) == GPIO_PIN_RESET)
                ;
        if (!wait)
            for (int i = 0; i < 100; i++)
                for (int j = 0; j < 2; j++)
                    ;
    }
    static void showLabel(uint8_t size, int x1, int y1, int x2, const char* str, uint8_t mode)
    {
        char buffer[100];
        sprintf(buffer, "LABL(%d,%d,%d,%d,\'%s\',7,%d);", size, x1, y1, x2, str, mode);
        send(buffer, strlen(buffer));
    }
    static void clear()
    {
        send(LCD_CLEAR, strlen(LCD_CLEAR));
    }
    static void showTime(uint8_t size, int x1, int y1, int x2, uint8_t h, uint8_t m, uint8_t s)
    {
        char time[6];
        sprintf(time, "%02d:%02d:%02d", h, m, s);
        showLabel(size, x1, y1, x2, time, 1);
    }
    static void showMessage(const char* message)
    {
        showLabel(32, 0, 250, 850, message, 1);
    }
    static const char* passwordLen(size_t len)
    {
        switch (len) {
        case 0:
            return "";
        case 1:
            return "*";
        case 2:
            return "**";
        case 3:
            return "***";
        case 4:
            return "****";
        case 5:
            return "*****";
        case 6:
            return "******";
        default:
            return "over 6";
        }
    }
};
