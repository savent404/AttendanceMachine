#include "STP_LCD.hpp"
#include "STP_RTC.hpp"

void GUI_Welcom(STP_RTC& rtc, int32_t timeDelay)
{
    static uint8_t s_sec = 0;
    uint8_t, h, m, s;
    rtc.getTime(h, m, s);
    if (!timeDelay) {
        STP_LCD::clear();
        STP_LCD::showLabel(32, 230, 75, 640, "上海xxx设备制造有限公司", 1);
        STP_LCD::showLabel(48, 230, 335, 640, "智能家居，安全出行, 1");
        STP_LCD::showLabel(16, 0, 0, 300, "按下'0'+'DOWN'输入管理员密码");
    }
    if ((s - s_sec) % 60 < timeDelay)
        continue;
    STP_LCD::showTime(48, 330, 200, 530, h, m, s);
    s_sec = s;
}

void GUI_InputFinger(STP_RTC& rtc, int32_t timeDelay, bool isPress, uint32_t idth = 0)
{
    static uint8_t s_sec = 0;
    uint8_t h, m, s;
    rtc.getTime(h, m, s);
    if (!timeDelay) {
        STP_LCD::clear();
        char buffer[20];
        sprintf(buffer, "请按下指纹(%01d)", idth);
        if (!idth)
            STP_LCD::showLabel(32, 230, 75, 640, "请按下指纹", 1);
        else
            STP_LCD::showLabel(32, 230, 75, 640, buffer, 1);
        STP_LCD::showLabel(16, 0, 0, 300, "按下'no'取消");
    }
    if ((s - s_sec) % 60 < timeDelay)
        continue;
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

    if (!timeDelay) {
        STP_LCD::clear();
        STP_LCD::showLabel(32, 230, 75, 640, "请刷卡", 1);
        STP_LCD::showLabel(16, 0, 0, 300, "按下'no'取消");
    }
    if ((s - s_sec) % 60 < timeDelay)
        continue;
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
        STP_LCD::showLabel(16, 0, 0, 300, "按下'no'取消");
    }
    if ((s - s_sec) % 60 < timeDelay)
        continue;
    STP_LCD::showTime(48, 330, 200, 530, h, m, s);
    STP_LCD::showLabel(32, 0, 300, 850, meesage, 1);
    s_sec = s;
}

void GUI_InputPassword(STP_RTC& rtc, int32_t timeDelay, char intputLen = 0)
{
    static uint8_t s_sec = 0;
    uint8_t h, m, s;
    rtc.getTime(h, m, s);
    if (!timeDelay) {
        STP_LCD::clear();
        STP_LCD::showLabel(32, 230, 75, 640, "请按下指纹", 1);
        STP_LCD::showLabel(16, 0, 0, 300, "按下'no'取消");
        STP_LCD::showLabel(16, 0, 30, 300, "按下指纹时请尽量使用手指中心按下");
    }
    if ((s - s_sec) % 60 < timeDelay)
        continue;
    // STP_LCD::showTime(48, 330, 200, 530, h, m, s);
    STP_LCD::showLabel(32, 0, 300, 850, STP_LCD::passwordLen(intputLen));
    s_sec = s;
}

void GUI_ChooseMode(STP_RTC& rtc, int32_t timeDelay)
{
    static uint8_t s_sec = 0;
    uint8_t h, m, s;
    rtc.getTime(h, m, s);
    if (!timeDelay) {
        STP_LCD::clear();
        STP_LCD::showLabel(32, 230, 75, 640, "请选择登入模式", 1);
        STP_LCD::showLabel(16, 0, 0, 300, "按下'no'取消");
        STP_LCD::showLabel(16, 0, 30, 300, "按键 0:指纹 1:刷卡 2:密码模式");
    }
    if ((s - s_sec) % 60 < timeDelay)
        continue;
    s_sec = s;
}

void GUI_ChooseSubMode(STP_RTC& rtc, int32_t timeDelay)
{
    static uint8_t s_sec = 0;
    uint8_t h, m, s;
    rtc.getTime(h, m, s);
    if (!timeDelay) {
        STP_LCD::clear();
        STP_LCD::showLabel(32, 230, 75, 640, "请选择登入/注册", 1);
        STP_LCD::showLabel(16, 0, 0, 300, "按下'no'取消");
        STP_LCD::showLabel(16, 0, 30, 300, "按键 0:登入 1:注册");
    }
    if ((s - s_sec) % 60 < timeDelay)
        continue;
    s_sec = s;
}

void GUI_InputTime(STP_RTC& rtc, int32_t timeDelay, const char* inputChar)
{
    char time[3][3] = { "", "", "" };
    uint8_t h, m, s;
    static uint8_t s_sec = 0;
    rtc.getTime(h, m, s);

    if (timeDelay < 0) {
        STP_LCD::clear();
        STP_LCD::showLabel(32, 230, 75, 640, "请输入当前时间", 1);
    }

    if (strlen(inputChar) >= 2) {
        time[0][0] = inputChar[0];
        time[0][1] = inputChar[1];
        time[0][2] = '\0';
    }
    if (strlen(inputChar) >= 4) {
        time[1][0] = inputChar[2];
        time[1][1] = inputChar[3];
        time[1][2] = '\0';
    }
    if (strlen(inputChar) >= 6) {
        time[2][0] = inputChar[4];
        time[2][1] = inputChar[5];
        time[2][2] = '\0';
    }
    char buffer[10] = "";
    strcat(buffer, time[0][0] == '\0' ? "  " : time[0]);
    strcat(buffer, ":");
    strcat(buffer, time[1][0] == '\0' ? "  " : time[1]);
    strcat(buffer, ":");
    strcat(buffer, time[2][0] == '\0' ? "  " : time[2]);
    STP_LCD::showLabel(48, 0, 300, 850, buffer, 1);
}

void GUI_Working(STP_RTC& rtc, int32_t timeDelay, const char* roomID)
{
    static uint8_t s_sec = 0;
    uint8_t h, m, s;
    rtc.getTime(h, m, s);

    if (timeDelay < 0) {
        STP_LCD::clear();
        STP_LCD::showLabel(32, 230, 75, 640, "正在操作...");
    }
    if ((s - s_sec) % 60 < timeDelay)
        continue;
    STP_LCD::showLabel(32, 230, 300, 640, roomID);
}
