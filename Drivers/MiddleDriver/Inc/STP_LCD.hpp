#pragma once

#include "usart.h"
#include <stdio.h>
#include <string.h>

#define LCD_UART (&huart3)

#define LCD_CLEAR "CLS(0);"
char LCD_WELCOMDE[];
char LCD_WELCOME[];
char TEXT_NFC_001[];
char TEST_FINGER_001[];
char TEXT_FINGER_002[];
char TEXT_TIME[];
char TEXT_PASSWORD[];
char TEXT_ROOMID[];
char TEXT_UNKNOWUSER[];
char TEXT_REPLACE[];
char TEXT_CANCEL[];
char TEXT_CHOOSE_MODE_1[];
char TEXT_CHOOSE_MODE_2[];

class STP_LCD {
public:
    STP_LCD() {}
    ~STP_LCD() {}
    static void send(const char* buffer, const size_t cnt)
    {
        while (HAL_GPIO_ReadPin(LCD_BUSY_GPIO_Port, LCD_BUSY_Pin) == GPIO_PIN_SET)
            ;
        HAL_UART_Transmit(LCD_UART, (uint8_t*)buffer, cnt, 100);
        HAL_UART_Transmit(LCD_UART, (uint8_t*)"\r\n", 2, 10);
        while (HAL_GPIO_ReadPin(LCD_BUSY_GPIO_Port, LCD_BUSY_Pin) == GPIO_PIN_RESET)
            ;
    }
    static void showLabel(uint8_t size, int x1, int y1, int x2, const char* str, uint8_t mode)
    {
        char buffer[50];
        sprintf(buffer, "LABL(%d,%d,%d,%d,\'%s\',7,%d);", size, x1, y1, x2, str, mode);
        send(buffer, strlen(buffer));
    }
    static void clear()
    {
        send(LCD_CLEAR, strlen(LCD_CLEAR));
    }
    static void showTime(uint8_t h, uint8_t m, uint8_t s)
    {
        char time[6];
        sprintf(time, "%02d:%02d:%02d", h, m, s);
        showLabel(48, 330, 200, 530, time, 1);
    }
    static void setTitle(const char* title)
    {
        showLabel(48, 230, 70, 640, title, 1);
    }
    static void showNum(const char* str, size_t maxlen = 0)
    {
        if (maxlen != 0) {
            char buffer[50];
            strncpy(buffer, str, maxlen);
            buffer[maxlen] = '\0';
            showLabel(48, 350, 200, 510, buffer, 0);

        } else
            showLabel(48, 350, 200, 510, str, 0);
    }

    static void showMessage(const char* message)
    {
        showLabel(48, 0, 250, 850, message, 1);
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
