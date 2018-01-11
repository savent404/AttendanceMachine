#include "STP_LCD.hpp"
#include "STP_RTC.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
void GUI_Welcome(STP_RTC& rtc, int32_t timeDelay)
{
    static uint8_t s_sec = 0;
    uint8_t h, m, s;
    rtc.getTime(h, m, s);
    if (timeDelay < 0) {
        STP_LCD::clear();
        STP_LCD::showLabel(32, 230, 75, 640, "上海xxx设备制造有限公司", 1);
        STP_LCD::showLabel(48, 230, 335, 640, "智能家居，安全出行", 1);
        STP_LCD::showLabel(16, 0, 20, 300, "按下 0 + DOWN 输入管理员密码", 0);
    } else if (((s - s_sec) % 60) < timeDelay)
        return;
    STP_LCD::showTime(48, 330, 200, 530, h, m, s);
    s_sec = s;
}

void GUI_InputFinger(STP_RTC& rtc, int32_t timeDelay, bool isPress, uint32_t idth = 0)
{
    static uint8_t s_sec = 0;
    uint8_t h, m, s;
    rtc.getTime(h, m, s);
    if (timeDelay < 0) {
        STP_LCD::clear();
        char buffer[20];
        sprintf(buffer, "请按下指纹(%01d)", idth);
        if (!idth)
            STP_LCD::showLabel(32, 230, 75, 640, "请按下指纹", 1);
        else
            STP_LCD::showLabel(32, 230, 75, 640, buffer, 1);
        STP_LCD::showLabel(16, 0, 20, 300, "按下 no 取消", 0);
    } else if (((s - s_sec) % 60) < timeDelay)
        return;

    if (isPress) {
        STP_LCD::showLabel(32, 0, 300, 850, "OK!请松开", 1);
    }
    STP_LCD::showTime(48, 330, 200, 530, h, m, s);
    s_sec = s;
}

void GUI_InputNFC(STP_RTC& rtc, int32_t timeDelay)
{
    static uint8_t s_sec = 0;
    uint8_t h, m, s;
    rtc.getTime(h, m, s);

    if (timeDelay < 0) {
        STP_LCD::clear();
        STP_LCD::showLabel(32, 230, 75, 640, "请刷卡", 1);
        STP_LCD::showLabel(16, 0, 20, 300, "按下 no 取消", 0);
    } else if (((s - s_sec) % 60) < timeDelay)
        return;
    STP_LCD::showTime(48, 330, 200, 530, h, m, s);
    s_sec = s;
}

void GUI_InputRoomID(STP_RTC& rtc, int32_t timeDelay, const char* meesage)
{
    static uint8_t s_sec = 0;
    uint8_t h, m, s;
    rtc.getTime(h, m, s);

    if (timeDelay < 0) {
        STP_LCD::clear();
        STP_LCD::showLabel(32, 230, 75, 640, "请输入房间ID", 1);
        STP_LCD::showLabel(16, 0, 20, 300, "按下 no 取消", 0);
    } else if (((s - s_sec) % 60) < timeDelay)
        return;
    STP_LCD::showTime(48, 330, 200, 530, h, m, s);
    STP_LCD::showLabel(32, 0, 300, 850, meesage, 1);
    s_sec = s;
}

void GUI_InputPassword(STP_RTC& rtc, int32_t timeDelay, char intputLen = 0)
{
    static uint8_t s_sec = 0;
    uint8_t h, m, s;
    rtc.getTime(h, m, s);
    if (timeDelay < 0) {
        STP_LCD::clear();
        STP_LCD::showLabel(32, 230, 75, 640, "请输入密码", 1);
        STP_LCD::showLabel(16, 0, 20, 300, "按下 no 取消", 0);
    } else if (((s - s_sec) % 60) < timeDelay)
        return;
    // STP_LCD::showTime(48, 330, 200, 530, h, m, s);
    STP_LCD::showLabel(32, 0, 300, 850, STP_LCD::passwordLen(intputLen), 1);
    s_sec = s;
}

void GUI_ChooseMode(STP_RTC& rtc, int32_t timeDelay)
{
    static uint8_t s_sec = 0;
    uint8_t h, m, s;
    rtc.getTime(h, m, s);
    if (timeDelay < 0) {
        STP_LCD::clear();
        STP_LCD::showLabel(32, 230, 75, 640, "请选择登入模式", 1);
        STP_LCD::showLabel(16, 0, 20, 300, "按下 no 取消", 0);
        STP_LCD::showLabel(16, 0, 40, 300, "按键 0:指纹 1:刷卡 2:密码模式", 0);
    } else if (((s - s_sec) % 60) < timeDelay)
        return;
    s_sec = s;
}

void GUI_ChooseSubMode(STP_RTC& rtc, int32_t timeDelay)
{
    static uint8_t s_sec = 0;
    uint8_t h, m, s;
    rtc.getTime(h, m, s);
    if (timeDelay < 0) {
        STP_LCD::clear();
        STP_LCD::showLabel(32, 230, 75, 640, "请选择登入/注册", 1);
        STP_LCD::showLabel(16, 0, 20, 300, "按下 no 取消", 0);
        STP_LCD::showLabel(16, 0, 40, 300, "按键 0:登入 1:注册", 0);
    } else if (((s - s_sec) % 60) < timeDelay)
        return;
    s_sec = s;
}

void GUI_InputTime(STP_RTC& rtc, int32_t timeDelay, const char* inputChar)
{
    uint8_t h, m, s;
    static uint8_t s_sec = 0;
    rtc.getTime(h, m, s);

    if (timeDelay < 0) {
        STP_LCD::clear();
        STP_LCD::showLabel(32, 230, 75, 640, "请输入时间", 1);
    } else if (((s - s_sec) % 60) < timeDelay)
        return;

    char buffer[9] = "";
    for (int i = 0; i < 6; i++) {
        if (*(inputChar + i) == '\0')
            strcat(buffer, " ");
        else {
            char b[2] = { *(inputChar + i), '\0' };
            strcat(buffer, b);
        }
        if (i < 5 && (i % 2) == 1) {
            strcat(buffer, ":");
        }
    }
    STP_LCD::showLabel(48, 0, 300, 850, buffer, 1);
}

void GUI_Working(STP_RTC& rtc, int32_t timeDelay, const char* roomID)
{
    static uint8_t s_sec = 0;
    uint8_t h, m, s;
    rtc.getTime(h, m, s);

    if (timeDelay < 0) {
        STP_LCD::clear();
        STP_LCD::showLabel(32, 230, 75, 640, "正在操作中", 1);
    } else if (((s - s_sec) % 60) < timeDelay)
        return;
    char buffer[5] = { roomID[0], roomID[1], roomID[2], roomID[3], '\0' };
    STP_LCD::showLabel(32, 230, 300, 640, buffer, 1);
}
void GUI_Operation(STP_RTC& rtc, int32_t timeDelay, uint8_t up_down_left_right)
{
    static uint8_t s_sec = 0;
    uint8_t h, m, s;
    rtc.getTime(h, m, s);

    if (timeDelay < 0) {
        STP_LCD::clear();
        STP_LCD::showLabel(32, 230, 75, 640, "请小心操作", 1);
        STP_LCD::showLabel(16, 0, 20, 300, "按下上下左右操作,点击 yes 确认", 0);
    } else if (((s - s_sec) % 60) < timeDelay)
        return;

    bool flag = false;
    char buffer[20] = "";

    if (up_down_left_right & 0x01) {
        flag = true;
        strcat(buffer, "上");
    }

    if (up_down_left_right & 0x02) {
        if (flag == false) {
            flag = true;
        } else {
            strcat(buffer, "+");
        }
        strcat(buffer, "下");
    }

    if (up_down_left_right & 0x04) {
        if (flag == false) {
            flag = true;
        } else {
            strcat(buffer, "+");
        }
        strcat(buffer, "左");
    }

    if (up_down_left_right & 0x08) {
        if (flag == false) {
            flag = true;
        } else {
            strcat(buffer, "+");
        }
        strcat(buffer, "右");
    }
    STP_LCD::showLabel(32, 0, 300, 850, buffer, 1);
}
