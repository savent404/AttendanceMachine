#include "opera.hpp"
/*********************************************
 * @name   Opera_getNFC
 * @brief  获取4字节的RFID
 * @note   过程中响应退出事件
 */
Opera_getNFC::Opera_getNFC(STP_KeyMat& kb)
    : Opera(4)
{
    keymat = &kb;
}
bool Opera_getNFC::init()
{
    STP_LCD::clear();
    nfc = new Adafruit_NFCShield_I2C(
        Adafruit_NFCShield_I2C::STM32_Pin(PN532_RDY_GPIO_Port, PN532_RDY_Pin),
        Adafruit_NFCShield_I2C::STM32_Pin((GPIO_TypeDef*)NULL, 0),
        &hi2c2);
    if (nfc == NULL) {
        ErrorCode = MEM_Error;
        ErrorStr = "Can't offer mem for class:Adafruit_NFCShield_I2C";
        return false;
    }
    nfc->begin();
    if (nfc->getFirmwareVersion() == 0) {
        ErrorCode = DRIVER_Error;
        ErrorStr = "Can't find NFC module";
        return false;
    }
    nfc->setPassiveActivationRetries(0x02);
    nfc->SAMConfig();
    return true;
}
bool Opera_getNFC::deinit()
{
    delete nfc;
    STP_LCD::clear();
    return true;
}
bool Opera_getNFC::exitCheck()
{
    if (keymat->isPress(STP_KeyMat::KEY_ID_NO)) {
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
    STP_LCD::setTitle("请刷卡");
    while (1) {
        if (nfc->readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLen)) {
            memcpy(ans, uid, 4);
            return true;
        }
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
Opera_getFinger::Opera_getFinger(STP_KeyMat& kb, bool isCheck = true)
    : Opera(2)
    , _isCheck(isCheck)
{
    STP_LCD::clear();
    keymat = &kb;
}
bool Opera_getFinger::init()
{
    uint8_t error = GR307_Init();
    STP_LCD::clear();
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
        ErrorCode = USR_Cancel;
        ErrorStr = "Usr Cancel";
        return true;
    }
    return false;
}
bool Opera_getFinger::loop()
{
    uint16_t id;
    STP_LCD::setTitle("请按下手指");
    while (1) {
        if (HAL_GPIO_ReadPin(R307_INT_GPIO_Port, R307_INT_Pin) == GPIO_PIN_RESET) {
            uint8_t error = 0;
            if (_isCheck) {
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
            STP_LCD::showMessage("OK!松开手指");
            while (HAL_GPIO_ReadPin(R307_INT_GPIO_Port, R307_INT_Pin) == GPIO_PIN_RESET)
                ;
            return true;
        }
        if (exitCheck()) {
            return false;
        }
    }
}

Opera_getUsrKey::Opera_getUsrKey(STP_KeyMat& kb, enum getMode mode)
    : Opera((int)mode)
    , _mode(mode)
{
    keymat = &kb;
    pos = 0;
    if (_mode != getRoomID)
        maxNum = 6;
    else
        maxNum = 4;
}
bool Opera_getUsrKey::init()
{
    memset(ans, 0, maxNum);
    STP_LCD::clear();
    switch (_mode) {
    case Opera_getUsrKey::getPassword: {
        STP_LCD::setTitle("请输入密码");
    } break;
    case Opera_getUsrKey::getTime: {
        STP_LCD::setTitle("请输入时间(hhmmss");
    } break;
    case Opera_getUsrKey::getRoomID: {
        STP_LCD::setTitle("请输入房间号(aabb)");
    } break;
    }
    return true;
}
bool Opera_getUsrKey::deinit()
{
    STP_LCD::showNum("");
    return true;
}
bool Opera_getUsrKey::exitCheck()
{
    if (keymat->isPress(STP_KeyMat::KEY_ID_NO)) {
        while (keymat->scan())
            ;
        ErrorCode = USR_Cancel;
        ErrorStr = "Usr Cancel";
        return true;
    } else if (keymat->isPress(STP_KeyMat::KEY_ID_YES) && pos == maxNum) {
        while (keymat->scan())
            ;
        ErrorCode = OK;
        ErrorStr = "Comfirme";
        return true;
    } else if (keymat->isPress(STP_KeyMat::KEY_ID_YES)) {
        while (keymat->scan())
            ;
        ErrorCode = USR_Error;
        if (maxNum == 6)
            ErrorStr = "Please input 6 char";
        else if (maxNum == 4)
            ErrorStr = "Please input 4 char";
        else
            ErrorStr = "Please input right number of char";
        return true;
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
            ans[pos] = '\0';
            if (_mode != getPassword) {
                STP_LCD::showNum((const char*)ans);
            } else {
                STP_LCD::showNum(STP_LCD::passwordLen(pos));
            }
            while (keymat->scan()) {
                if (exitCheck())
                    return false;
            }
        }
        if (exitCheck()) {
            return false;
        }
    }
}
