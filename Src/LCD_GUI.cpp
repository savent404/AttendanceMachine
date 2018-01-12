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
        STP_LCD::showLabel(32, 230, 75, 640, "�Ϻ�xxx�豸�������޹�˾", 1);
        STP_LCD::showLabel(48, 230, 335, 640, "���ܼҾӣ���ȫ����", 1);
        STP_LCD::showLabel(16, 0, 20, 300, "���� 0 + DOWN �������Ա����", 0);
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
        sprintf(buffer, "�밴��ָ��(%01d)", idth);
        if (!idth)
            STP_LCD::showLabel(32, 230, 75, 640, "�밴��ָ��", 1);
        else
            STP_LCD::showLabel(32, 230, 75, 640, buffer, 1);
        STP_LCD::showLabel(16, 0, 20, 300, "���� no ȡ��", 0);
    } else if (((s - s_sec) % 60) < timeDelay)
        return;

    if (isPress) {
        STP_LCD::showLabel(32, 0, 300, 850, "OK!���ɿ�", 1);
    } else {
        STP_LCD::showLabel(32, 0, 300, 850, "", 1);
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
        STP_LCD::showLabel(32, 230, 75, 640, "��ˢ��", 1);
        STP_LCD::showLabel(16, 0, 20, 300, "���� no ȡ��", 0);
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
        STP_LCD::showLabel(32, 230, 75, 640, "�����뷿��ID", 1);
        STP_LCD::showLabel(16, 0, 20, 300, "���� no ȡ��", 0);
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
        STP_LCD::showLabel(32, 230, 75, 640, "����������", 1);
        STP_LCD::showLabel(16, 0, 20, 300, "���� no ȡ��", 0);
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
        STP_LCD::showLabel(32, 230, 75, 640, "��ѡ�����ģʽ", 1);
        STP_LCD::showLabel(16, 0, 20, 300, "���� no ȡ��", 0);
        STP_LCD::showLabel(16, 0, 40, 300, "���� 0:ָ�� 1:ˢ�� 2:����ģʽ", 0);
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
        STP_LCD::showLabel(32, 230, 75, 640, "��ѡ�����/ע��", 1);
        STP_LCD::showLabel(16, 0, 20, 300, "���� no ȡ��", 0);
        STP_LCD::showLabel(16, 0, 40, 300, "���� 0:���� 1:ע��", 0);
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
        STP_LCD::showLabel(32, 230, 75, 640, "������ʱ��", 1);
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
        STP_LCD::showLabel(32, 230, 75, 640, "���ڲ�����", 1);
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
        STP_LCD::showLabel(32, 230, 75, 640, "��С�Ĳ���", 1);
        STP_LCD::showLabel(16, 0, 20, 300, "�����������Ҳ���,��� yes ȷ��", 0);
    } else if (((s - s_sec) % 60) < timeDelay)
        return;

    bool flag = false;
    char buffer[20] = "";

    if (up_down_left_right & 0x01) {
        flag = true;
        strcat(buffer, "��");
    }

    if (up_down_left_right & 0x02) {
        if (flag == false) {
            flag = true;
        } else {
            strcat(buffer, "+");
        }
        strcat(buffer, "��");
    }

    if (up_down_left_right & 0x04) {
        if (flag == false) {
            flag = true;
        } else {
            strcat(buffer, "+");
        }
        strcat(buffer, "��");
    }

    if (up_down_left_right & 0x08) {
        if (flag == false) {
            flag = true;
        } else {
            strcat(buffer, "+");
        }
        strcat(buffer, "��");
    }
    STP_LCD::showLabel(32, 0, 300, 850, buffer, 1);
}
