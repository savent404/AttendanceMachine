#include "opera.hpp"
extern STP_RTC* rtc;

/*********************************************
 * @name   Opera_getNFC
 * @brief  获取4字节的RFID
 * @note   过程中响应退出事件
 */
Opera_getNFC::Opera_getNFC(STP_KeyMat& kb, bool logic)
    : Opera(4)
    , isLogin(logic)
{
    keymat = &kb;
}
bool Opera_getNFC::init()
{
    GUI_InputNFC(*rtc, -1, isLogin);
    nfc = new Adafruit_NFCShield_I2C(
        Adafruit_NFCShield_I2C::STM32_Pin(PN532_RDY_GPIO_Port, PN532_RDY_Pin),
        Adafruit_NFCShield_I2C::STM32_Pin((GPIO_TypeDef*)NULL, 0),
        &hi2c2);
    if (nfc == NULL) {
        ErrorCode = MEM_Error;
        ErrorStr = "Can not offer mem for class:Adafruit_NFCShield_I2C";
        return false;
    }
    nfc->begin();
    if (nfc->getFirmwareVersion() == 0) {
        ErrorCode = DRIVER_Error;
        ErrorStr = "Can not find NFC module";
        return false;
    }
    nfc->setPassiveActivationRetries(0x02);
    nfc->SAMConfig();
    return true;
}
bool Opera_getNFC::deinit()
{
    delete nfc;
    return true;
}
bool Opera_getNFC::exitCheck()
{
    if (keymat->isPress(STP_KeyMat::KEY_ID_NO)) {
        while (keymat->scan())
            GUI_InputNFC(*rtc, 1, isLogin);
        ErrorCode = USR_Cancel;
        ErrorStr = "Usr Cancel";
        return true;
    }
    return false;
}
bool Opera_getNFC::loop()
{
    uint8_t uid[10];
    uint8_t uidLen;
    while (1) {
        if (nfc->readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLen)) {
            memcpy(ans, uid, 4);
            return true;
        }
        GUI_InputNFC(*rtc, 1, isLogin);
        if (exitCheck()) {
            return false;
        }
    }
}

/*********************************************
 * @name   Opera_getFinger
 * @brief  对比一个指纹或者注册一个新的到指纹库
 * @note   过程中响应退出事件
 */
Opera_getFinger::Opera_getFinger(STP_KeyMat& kb, uint8_t th)
    : Opera(2)
    , _th(th)
{
    keymat = &kb;
}
bool Opera_getFinger::init()
{
    uint8_t error = GR307_Init();
    GUI_InputFinger(*rtc, -1, false, _th);
    if (error != 0) {
        ErrorCode = DRIVER_Error;
        ErrorStr = GR307_getErrorMsg(error);
        return false;
    }
    return true;
}
bool Opera_getFinger::deinit()
{
    STP_LCD::clear();
    return true;
}
bool Opera_getFinger::exitCheck()
{
    if (keymat->isPress(STP_KeyMat::KEY_ID_NO)) {
        while (keymat->scan())
            GUI_InputFinger(*rtc, 1, false, _th);
        ErrorCode = USR_Cancel;
        ErrorStr = "Usr Cancel";
        return true;
    }
    return false;
}
bool Opera_getFinger::loop()
{
    uint16_t id;
    while (1) {
        if (HAL_GPIO_ReadPin(R307_INT_GPIO_Port, R307_INT_Pin) == GPIO_PIN_RESET) {
            uint8_t error = 0;
            if (_th == 0) {
                error = GR307_Check(&id);
            } else {
                error = GR307_Register(&id);
            }
            if (error != 0) {
                ErrorCode = DRIVER_Error;
                ErrorStr = GR307_getErrorMsg(error);
                return false;
            }
            memcpy(ans, &id, 2);
            while (HAL_GPIO_ReadPin(R307_INT_GPIO_Port, R307_INT_Pin) == GPIO_PIN_RESET)
                GUI_InputFinger(*rtc, 1, true, _th);
            return true;
        }
        GUI_InputFinger(*rtc, 1, false, _th);
        if (exitCheck()) {
            return false;
        }
    }
}

Opera_getUsrKey::Opera_getUsrKey(STP_KeyMat& kb, enum getMode mode)
    : Opera(6)
    , _mode(mode)
{
    keymat = &kb;
    pos = 0;
    switch (mode) {
    case getRoomID_Finger:
    case getRoomID_NFC:
    case getRoomID_Password:
        maxNum = 4;
        break;
    case getPassword_Root:
    case getPassword_Usr:
        maxNum = 6;
        break;
    case getTime:
        maxNum = 6;
        break;
    default:
        maxNum = 6;
    }
}
bool Opera_getUsrKey::init()
{
    memset(ans, 0, maxNum);
    if (_mode == getTime)
        GUI_InputTime(*rtc, -1, "\0\0\0\0\0\0");
    if (_mode == getPassword_Root)
        GUI_InputPassword(*rtc, -1, true, 0);
    if (_mode == getPassword_Usr)
        GUI_InputPassword(*rtc, -1, false, 0);
    if (_mode >= getRoomID_Finger && _mode <= getRoomID_Password)
        GUI_InputRoomID(*rtc, -1, "    ", _mode - getRoomID_Finger);
    pos = 0;
    return true;
}
bool Opera_getUsrKey::deinit()
{
    return true;
}
bool Opera_getUsrKey::exitCheck()
{
    if (keymat->isPress(STP_KeyMat::KEY_ID_NO)) {
        while (keymat->scan())
            ;
        ErrorCode = USR_Cancel;
        ErrorStr = "Usr Cancel";
        memset(ans, 0, 6);
        pos = 0;
        return true;
    } else if (keymat->isPress(STP_KeyMat::KEY_ID_YES) && pos == maxNum) {
        while (keymat->scan())
            ;
        ErrorCode = OK;
        ErrorStr = "Comfirme";
        return true;
    } else if (keymat->isPress(STP_KeyMat::KEY_ID_YES) && pos != maxNum) {
        while (keymat->scan())
            ;
        ErrorCode = USR_Error;
        if (maxNum == 6)
            ErrorStr = "Please input 6 char";
        else if (maxNum == 4)
            ErrorStr = "Please input 4 char";
        else
            ErrorStr = "Please input right number of char";
        memset(ans, 0, 6);
        pos = 0;
        // opt1:用户错误需要提示的话应return true,在上一级函数中给予提示
        // opt2:直接在当前位置输出字符串并延时保证显示时间
        // opt3:不显示直接继续循环
        return false;
    }
    return false;
}
bool Opera_getUsrKey::loop()
{
    pos = 0;
    while (1) {
        if (keymat->scan()) {
            if (keymat->isPress(STP_KeyMat::KEY_ID_0) && pos < maxNum) {
                ans[pos++] = '0';
            } else if (keymat->isPress(STP_KeyMat::KEY_ID_1) && pos < maxNum) {
                ans[pos++] = '1';
            } else if (keymat->isPress(STP_KeyMat::KEY_ID_2) && pos < maxNum) {
                ans[pos++] = '2';
            } else if (keymat->isPress(STP_KeyMat::KEY_ID_3) && pos < maxNum) {
                ans[pos++] = '3';
            } else if (keymat->isPress(STP_KeyMat::KEY_ID_4) && pos < maxNum) {
                ans[pos++] = '4';
            } else if (keymat->isPress(STP_KeyMat::KEY_ID_5) && pos < maxNum) {
                ans[pos++] = '5';
            } else if (keymat->isPress(STP_KeyMat::KEY_ID_6) && pos < maxNum) {
                ans[pos++] = '6';
            } else if (keymat->isPress(STP_KeyMat::KEY_ID_7) && pos < maxNum) {
                ans[pos++] = '7';
            } else if (keymat->isPress(STP_KeyMat::KEY_ID_8) && pos < maxNum) {
                ans[pos++] = '8';
            } else if (keymat->isPress(STP_KeyMat::KEY_ID_9) && pos < maxNum) {
                ans[pos++] = '9';
            } else if (keymat->isPress(STP_KeyMat::KEY_ID_LEFT)) {
                if (pos > 0) {
                    ans[--pos] = '\0';
                }
            }
            switch (_mode) {
            case getRoomID_Finger:
            case getRoomID_NFC:
            case getRoomID_Password:
                GUI_InputRoomID(*rtc, 0, (char*)ans, _mode - getRoomID_Finger);
                break;
            case getPassword_Root:
                GUI_InputPassword(*rtc, 0, true, pos);
                break;
            case getPassword_Usr:
                GUI_InputPassword(*rtc, 0, false, pos);
                break;
            case getTime:
                GUI_InputTime(*rtc, 0, (char*)ans);
                break;
            }
            while (keymat->scan()) {
                if (exitCheck())
                    return false;
            }
        }
        switch (_mode) {
        case getRoomID_Finger:
        case getRoomID_NFC:
        case getRoomID_Password:
            GUI_InputRoomID(*rtc, 1, (char*)ans, _mode - getRoomID_Finger);
            break;
        case getPassword_Root:
            GUI_InputPassword(*rtc, 1, true, pos);
            break;
        case getPassword_Usr:
            GUI_InputPassword(*rtc, 1, false, pos);
            break;
        case getTime:
            GUI_InputTime(*rtc, 1, (char*)ans);
            break;
        }

        if (exitCheck()) {
            return false;
        }
    }
}
