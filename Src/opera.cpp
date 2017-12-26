#include "opera.hpp"
#define OP_Finger 0
#define OP_RFID 1
#define OP_Room 2
#define OP_Manager 3
#define OP_Unknow 4

STP_ServerBase* server;
STP_KeyMat* keyboard;
STP_RTC* rtc;

static void NFC_Handle(bool isRegist)
{
    Opera* get_key = new Opera_getUsrKey(*keyboard, Opera_getUsrKey::getRoomID);
    Opera* get_password = new Opera_getUsrKey(*keyboard, Opera_getUsrKey::getPassword);
    Opera* get_nfc = new Opera_getNFC(*keyboard);
    DB_Base* nfc_data = new DB_RFID;
    DB_Base* room_id = new DB_RoomID;
    DB_Sheet* sheet;
    std::list<DB_Usr>::iterator node;
    while (1) {
        const uint8_t reset_data[] = "000000";
        nfc_data->overWrite(reset_data);
        room_id->overWrite(reset_data);

        if (isRegist == true) {
            // Wait for input ROOM ID
            get_key->init();
            get_key->loop();
            get_key->deinit();
            if (get_key->getResCode() != Opera::OK) {
                // TODO: output error message
                continue;
            }
            // Wait for press RFID
            get_nfc->init();
            get_nfc->loop();
            get_nfc->deinit();
            if (get_key->getResCode() != Opera::OK && get_key->getResCode() != Opera::USR_Cancel) {
                // TODO: output error message
                continue;
            }
            // Match DB in Sheet
            bool isFound = false;
            room_id->overWrite(get_key->getAns());
            nfc_data->overWrite(get_nfc->getAns());
            for (int i = 1; i <= 3; i++) {
                sheet = new DB_Sheet((enum DB_Sheet::DB_Sector)i);
                node = sheet->search(*room_id);
                if (node != NULL) {
                    isFound = true;
                    break;
                }
                delete sheet;
            }
            // Regist a new DB_Usr in exited room
            if (isFound) {
                // TODO: Warning if you replace the ID
                get_password->init();
                do {
                    get_password->loop();
                } while (get_password->getResCode() != Opera::OK && get_password->getResCode() != Opera::USR_Cancel);
                get_password->deinit();
                if (DB_Sheet::checkPassword(get_password->getAns())) {
                    ;
                } else {
                    // TODO : output error message
                    delete sheet;
                    continue;
                }
                node->rfid.overWrite(nfc_data->data());
                sheet->writeBack(true);
                delete sheet;
            } else {
                DB_Usr usr;
                sheet = new DB_Sheet(DB_Sheet::Sector_1);
                usr.rid.overWrite(room_id->data());
                usr.rfid.overWrite(nfc_data->data());
                sheet->add(usr);
                sheet->writeBack();
                delete sheet;
            }
        } else {
            // Wait for press nfc card
            get_nfc->init();
            get_nfc->loop();
            get_nfc->deinit();
            if (get_key->getResCode() != Opera::OK && get_key->getResCode() != Opera::USR_Cancel) {
                // TODO: output error message
                continue;
            }
            // Match in DB sheet
            nfc_data->overWrite(get_nfc->getAns());
            for (int i = 1; i <= 3; i++) {
                sheet = new DB_Sheet(DB_Sheet::Sector_1);
                auto node = sheet->search(*nfc_data);
                if (node != node) {
                    // TODO: send message to slaver
                    delete sheet;
                    break;
                }
                delete sheet;
            }
        }
    }
}
/*********************************************
 * @name   OP_Handle
 * @brief  操作执行实体
 */
static bool isErase = false;
void OP_Handle()
{
    if (isErase) {
        DB_Sheet* sheet = new DB_Sheet(DB_Sheet::Sector_1);
        sheet->clear();
        sheet->writeBack();
        delete sheet;
    }
    server = new STP_ServerRS485(&huart6);
    keyboard = new STP_KeyMat;
    rtc = new STP_RTC(&hrtc);

    server->reciMessage();

    NFC_Handle(true);
}
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
    keymat = &kb;
}
bool Opera_getFinger::init()
{
    uint8_t error = GR307_Init();
    if (error != 0) {
        ErrorCode = DRIVER_Error;
        ErrorStr = GR307_getErrorMsg(error);
        return false;
    }
    return true;
}
bool Opera_getFinger::deinit()
{
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
    , maxNum((int)mode)
{
    keymat = &kb;
    pos = 0;
}
bool Opera_getUsrKey::init()
{
    memset(ans, 0, maxNum);
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
            if (keymat->isPress(STP_KeyMat::KEY_ID_0)) {
                ans[pos++] = '0';
            } else if (keymat->isPress(STP_KeyMat::KEY_ID_1)) {
                ans[pos++] = '1';
            } else if (keymat->isPress(STP_KeyMat::KEY_ID_2)) {
                ans[pos++] = '2';
            } else if (keymat->isPress(STP_KeyMat::KEY_ID_3)) {
                ans[pos++] = '3';
            } else if (keymat->isPress(STP_KeyMat::KEY_ID_4)) {
                ans[pos++] = '4';
            } else if (keymat->isPress(STP_KeyMat::KEY_ID_5)) {
                ans[pos++] = '5';
            } else if (keymat->isPress(STP_KeyMat::KEY_ID_6)) {
                ans[pos++] = '6';
            } else if (keymat->isPress(STP_KeyMat::KEY_ID_7)) {
                ans[pos++] = '7';
            } else if (keymat->isPress(STP_KeyMat::KEY_ID_8)) {
                ans[pos++] = '8';
            } else if (keymat->isPress(STP_KeyMat::KEY_ID_9)) {
                ans[pos++] = '9';
            } else if (keymat->isPress(STP_KeyMat::KEY_ID_LEFT)) {
                if (pos > 0) {
                    ans[pos--] = '\0';
                }
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
