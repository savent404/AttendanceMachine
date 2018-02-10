#include "STP_LCD.hpp"
#include "STP_RTC.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief  ͼ�ν�����ش���
 * @brief  �������� timeDelay >0 ʱΪͼ���������, =0��ÿ�ε��ö������
 *                 timeDelay <0 ʱ�����ʼ������(�������±���ͼƬ�Ⱦ�̬��Ϣ)
 */

// ECHODE: GBK

char TEXT_SECUR[] = "��ȫ����,����no������";
char TEXT_REPLACE[] = "�����Ѵ��ڣ��滻���������Ա����";
char TEXT_UNKNOWUSER[] = "δ�ҵ����û�";

static uint32_t fullTime(uint8_t h, uint8_t m, uint8_t s)
{
    return h * 3600 + m * 60 + s;
}

void GUI_Welcome(STP_RTC& rtc, int32_t timeDelay)
{
    static uint32_t s_sec = 0;
    uint8_t h, m, s;
    rtc.getTime(h, m, s);
    if (timeDelay < 0) {
        STP_LCD::showPIC(3);
        STP_LCD::showLabel("�Ϻ�����", 130, 50);
        STP_LCD::showLabel("�豸�������޹�˾", 325, 50);
        STP_LCD::showLabel("0+\"��\" �������ģʽ", 20, 450, 16);
        STP_LCD::showLabel("���ܳ��� ��ȫ���", 240, 420);
    } else if (fullTime(h, m, s) - s_sec < timeDelay)
        return;
    STP_LCD::showTime(h, m, s);
    s_sec = fullTime(h, m, s);
}

// note id==0 �����ǵ��� id != 0����ע�ỷ��
void GUI_InputFinger(STP_RTC& rtc, int32_t timeDelay, bool isPress, uint32_t idth = 0)
{
    static uint32_t s_sec = 0;
    uint8_t h, m, s;
    rtc.getTime(h, m, s);
    if (timeDelay < 0) {
        STP_LCD::showPIC(1);
        if (!idth) {
            STP_LCD::showLabel("\"ȡ��\"��������һ��", 20, 20, 16);
            STP_LCD::showLabel("�Ϻ�����", 130, 50);
            STP_LCD::showLabel("�豸�������޹�˾", 325, 50);
            STP_LCD::showLabel("0+\"��\" �������ģʽ", 20, 450, 16);
            STP_LCD::showLabel("���ܳ��� ��ȫ���", 240, 420);
            STP_LCD::showLabel("�밴��ָ��", 310, 120);
        } else {
            char buffer[20];
            sprintf(buffer, "�밴��ָ��#%1d", idth);
            STP_LCD::showLabel(buffer, 290, 120);
            STP_LCD::showLabel("\"ȡ��\"��������һ��", 20, 20, 16);
        }
    } else if (fullTime(h, m, s) - s_sec < timeDelay) {
        return;
    }
    if (isPress) {
        STP_LCD::showLabel("���ɿ���ָ", 300, 330);
    }
    STP_LCD::showTime(h, m, s);
    s_sec = fullTime(h, m, s);
}

void GUI_InputNFC(STP_RTC& rtc, int32_t timeDelay, bool isLogin)
{
    static uint32_t s_sec = 0;
    uint8_t h, m, s;
    rtc.getTime(h, m, s);

    if (timeDelay < 0) {
        STP_LCD::showPIC(2);
        STP_LCD::showLabel("��ˢ��", 370, 120);
        STP_LCD::showLabel("\"ȡ��\"��������һ��", 20, 20, 16);
        if (isLogin) {
            STP_LCD::showLabel("�Ϻ�����", 130, 50);
            STP_LCD::showLabel("�豸�������޹�˾", 325, 50);
            STP_LCD::showLabel("���ܳ��� ��ȫ���", 240, 420);
        }
    } else if (fullTime(h, m, s) - s_sec < timeDelay)
        return;
    STP_LCD::showTime(h, m, s);
    s_sec = fullTime(h, m, s);
}

// note:
// 0 - finger
// 1 - rfid
// 2 - password
void GUI_InputRoomID(STP_RTC& rtc, int32_t timeDelay, const char* meesage, uint8_t id)
{
    static uint32_t s_sec = 0;
    uint8_t h, m, s;
    rtc.getTime(h, m, s);

    if (timeDelay < 0) {

        switch (id) {
        case 0:
            STP_LCD::showPIC(1);
            break;
        case 1:
            STP_LCD::showPIC(2);
            break;
        case 2:
            STP_LCD::showPIC(3);
            break;
        default:
            STP_LCD::clear();
        }
        STP_LCD::showLabel("�����복��ID", 290, 120);
        STP_LCD::showLabel("\"ȡ��\"��������һ��'", 20, 20, 16);
        STP_LCD::send("PL(390,335,410,335,0);", strlen("PL(390,335,410,335,0);"));
        STP_LCD::send("PL(415,335,435,335,0);", strlen("PL(415,335,435,335,0);"));
        STP_LCD::send("PL(440,335,460,335,0);", strlen("PL(440,335,460,335,0);"));
        STP_LCD::send("PL(465,335,485,335,0);", strlen("PL(465,335,485,335,0);"));
    } else if (fullTime(h, m, s) - s_sec < timeDelay)
        return;
    // �����&ʱ��
    // ��message����4λ�ַ�,���ÿ��в���
    char* _ptr = NULL;
    bool flag = false;
    if (strlen(meesage) < 4) {
        _ptr = (char*)malloc(5);
        flag = true;
        for (int i = 0; i < 4; i++) {
            if (i < strlen(meesage))
                _ptr[i] = meesage[i];
            else
                _ptr[i] = ' ';
        }
        _ptr[4] = '\0';
    } else {
        _ptr = (char*)meesage;
    }
    STP_LCD::showTime(h, m, s);
    STP_LCD::showLabel(_ptr, 390, 290);
    STP_LCD::send("PL(390,335,410,335,0);", strlen("PL(390,335,410,335,0);"));
    STP_LCD::send("PL(415,335,435,335,0);", strlen("PL(415,335,435,335,0);"));
    STP_LCD::send("PL(440,335,460,335,0);", strlen("PL(440,335,460,335,0);"));
    STP_LCD::send("PL(465,335,485,335,0);", strlen("PL(465,335,485,335,0);"));
    s_sec = fullTime(h, m, s);
    if (flag == true)
        free(_ptr);
}

void GUI_InputPassword(STP_RTC& rtc, int32_t timeDelay, bool isRoot, char intputLen = 0)
{
    static uint32_t s_sec = 0;
    uint8_t h, m, s;
    const char buffer[] = "PL(355,245,375,245,0);PL(380,245,400,245,0);PL(405,245,425,245,0);PL(430,245,450,245,0);PL(455,245,475,245,0);PL(480,245,500,245,0);";

    rtc.getTime(h, m, s);
    if (timeDelay < 0) {
        // ����Ա����
        if (isRoot) {
            STP_LCD::clear();
            STP_LCD::showLabel("���������Ա����", 250, 55);
        } else {
            STP_LCD::showPIC(3);
            STP_LCD::showLabel("�������û�����", 270, 55);
        }
        STP_LCD::showLabel("����6λ", 20, 410, 16);
        STP_LCD::showLabel("����\"ȷ��\"ȷ��", 20, 430, 16);
        STP_LCD::showLabel("����\"ȡ��\"������", 20, 450, 16);
        STP_LCD::send(buffer, strlen(buffer));
    } else if (fullTime(h, m, s) - s_sec < timeDelay)
        return;
    STP_LCD::showLabel(STP_LCD::passwordLen(intputLen), 355, 200);
    STP_LCD::send(buffer, strlen(buffer));
    s_sec = fullTime(h, m, s);
}

void GUI_ChooseMode(STP_RTC& rtc, int32_t timeDelay)
{
    static uint32_t s_sec = 0;
    uint8_t h, m, s;
    rtc.getTime(h, m, s);
    if (timeDelay < 0) {
        STP_LCD::showPIC(3);
        STP_LCD::showLabel("��ѡ��ʹ��ģʽ", 60, 10);
        STP_LCD::showLabel("����0:ָ��ģʽ", 20, 80, 32);
        STP_LCD::showLabel("����1:ˢ��ģʽ", 20, 120, 32);
        STP_LCD::showLabel("����2:�ֶ�ģʽ", 20, 160, 32);
        STP_LCD::showLabel("����3:��������", 20, 200, 32);
        STP_LCD::showLabel("����4:ɾ������", 20, 240, 32);
        STP_LCD::showLabel("\"ȡ��\"��������һ��", 20, 280, 32);
    } else if (fullTime(h, m, s) - s_sec < timeDelay)
        return;
    s_sec = fullTime(h, m, s);
}

// note: id
// 0 - finger
// 1 - nfc
// 2 - password
void GUI_ChooseSubMode(STP_RTC& rtc, int32_t timeDelay, uint8_t id)
{
    static uint32_t s_sec = 0;
    uint8_t h, m, s;
    rtc.getTime(h, m, s);
    if (timeDelay < 0) {
        switch (id) {
        case 0:
            STP_LCD::showPIC(1);
            break;
        case 1:
            STP_LCD::showPIC(2);
            break;
        case 2:
            STP_LCD::showPIC(3);
            break;
        default:
            STP_LCD::clear();
        }
        STP_LCD::showLabel("��ѡ���½/ע��", 265, 120);
        STP_LCD::showLabel("����0:��½", 320, 200, 32);
        STP_LCD::showLabel("����1:ע��", 320, 240, 32);
        STP_LCD::showLabel("����\"ȡ��\"������", 320, 280, 32);
    } else if (fullTime(h, m, s) - s_sec < timeDelay)
        return;
    s_sec = fullTime(h, m, s);
}

void GUI_InputTime(STP_RTC& rtc, int32_t timeDelay, const char* inputChar)
{
    uint8_t h, m, s;
    static uint8_t s_sec = 0;
    rtc.getTime(h, m, s);

    if (timeDelay < 0) {
        STP_LCD::clear();
        STP_LCD::showLabel("������ʱ��", 350, 120, 32);
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
    STP_LCD::showLabel(buffer, 330, 290);
}

void GUI_Working(STP_RTC& rtc, int32_t timeDelay, const char* roomID)
{
    static uint32_t s_sec = 0;
    uint8_t h, m, s;
    rtc.getTime(h, m, s);
    char buffer[5];
    for (int i = 0; i < 4; i++)
        buffer[i] = roomID[i];
    buffer[4] = '\0';
    if (timeDelay < 0) {
        char buf[30] = "���ڲ�������";
        strcat(buf, buffer);
        strcat(buf, "��...");
        STP_LCD::clear();
        STP_LCD::showLabel(buf, 210, 120);
    } else if (fullTime(h, m, s) - s_sec < timeDelay)
        return;
    s_sec = fullTime(h, m, s);
}
void GUI_Operation(STP_RTC& rtc, int32_t timeDelay, uint8_t up_down_left_right)
{
    static uint32_t s_sec = 0;
    uint8_t h, m, s;
    rtc.getTime(h, m, s);

    if (timeDelay < 0) {
        STP_LCD::showPIC(3);
        STP_LCD::showLabel("��С�Ĳ���", 320, 75, 48);
        STP_LCD::showLabel("�����������Ҳ���,��� yes ȷ��", 20, 20, 16);
    } else if (fullTime(h, m, s) - s_sec < timeDelay)
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
    strcat(buffer, "       ");
    STP_LCD::showLabel(buffer, 350, 290, 32);
    s_sec = fullTime(h, m, s);
}
