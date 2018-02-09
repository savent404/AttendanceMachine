#pragma once

#include "usart.h"
#include <stdio.h>
#include <string.h>

#define LCD_UART (&huart3)

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
    static void showLabel(const char* message, int x = 350, int y = 250, uint8_t size = 48)
    {
        char buffer[50];
        sprintf(buffer, "PS%d(1,%d,%d,'%s');", size, x, y, message);
        send(buffer, strlen(buffer));
    }
    static void clear(uint8_t color = 8)
    {
        char buffer[10];
        sprintf(buffer, "CLS(%d);", color);
        send(buffer, strlen(buffer));
    }
    static void showTime(uint8_t h,
        uint8_t m, uint8_t s,
        int x1 = 345, int y1 = 230,
        uint8_t size = 48)
    {
        char time[30];
        sprintf(time, "PS%d(1,%d,%d,'%02d:%02d:%02d');", size, x1, y1, h, m, s);
        send(time, strlen(time));
    }
    static void showPIC(uint8_t id) {
        char buffer[10];
        sprintf(buffer, "BPIC(1,0,0,%d);", id);
        send(buffer, strlen(buffer));
    }
    static const char* passwordLen(size_t len)
    {
        switch (len) {
        case 0:
            return "      ";
        case 1:
            return "*     ";
        case 2:
            return "**    ";
        case 3:
            return "***   ";
        case 4:
            return "****  ";
        case 5:
            return "***** ";
        case 6:
            return "******";
        default:
            return "over 6";
        }
    }
};

extern char TEXT_SECUR[];
extern char TEXT_REPLACE[];
extern char TEXT_UNKNOWUSER[];
