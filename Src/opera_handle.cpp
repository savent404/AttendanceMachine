#include "opera.hpp"

STP_ServerBase* server;
STP_KeyMat* keyboard;
STP_RTC* rtc;

/**
 * @brief  获取RFID的子流程
 * @param  isRegist:true 注册流程，否则为登入流程
 */
static bool NFC_Handle(bool isRegist);

/**
 * @brief  获取FingerPrint的子流程
 * @param  isRegist:true 注册流程，否则为登入流程
 */
static bool Finger_Handle(bool isRegist);

/**
 * @brief  获取key的子流程
 * @param  isRegist:true 注册流程，否则为登入流程
 */
static bool Key_Handle(bool isRegist);
void rec_callback(enum STP_ServerBase::CMD cmd)
{
    switch (cmd) {
    case STP_ServerBase::CMD_ERROR_UNKNOW: {
        STP_LCD::showMessage(TEXT_ERROR_UNKNOW);
    } break;
    case STP_ServerBase::CMD_ERROR_BREAKIN: {
        STP_LCD::showMessage(TEXT_ERROR_BREAKIN);
    } break;
    case STP_ServerBase::CMD_ERROR_LIMIT: {
        STP_LCD::showMessage(TEXT_ERROR_LIMIT);
    } break;
    case STP_ServerBase::CMD_ERROR_CHAT: {
        STP_LCD::showMessage(TEXT_ERROR_CHAT);
    } break;
    }
    HAL_GPIO_WritePin(BELL_GPIO_Port, BELL_Pin, GPIO_PIN_SET);
    while (1) {
        if (keyboard->isPress(STP_KeyMat::KEY_ID_NO)) {
            HAL_NVIC_SystemReset();
        }
    }
}
/*********************************************
 * @name   OP_Handle
 * @brief  操作执行实体
 */
void OP_Handle(void)
{
    server = new STP_ServerRS485(&huart1);
    keyboard = new STP_KeyMat;
    rtc = new STP_RTC(&hrtc);
    Opera* op;
    enum {
        OPFinger = 0,
        OPNFC = 1,
        OPKey = 2
    } Mode
        = OPFinger;
    enum {
        Login = 0,
        Regist = 1
    } subMode
        = Login;

    server->reciMessage();

    if (__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST) == 0) {
    // Setting the RTC parameter
    op = new Opera_getUsrKey(*keyboard, Opera_getUsrKey::getTime);
    op->init();
    do {
        op->loop();
    } while (op->getResCode() != Opera::OK);
    op->deinit();
    {
        char buffer[3 * 3];
        memcpy(buffer, op->getAns(), 2);
        memcpy(buffer + 3, op->getAns() + 2, 2);
        memcpy(buffer + 6, op->getAns() + 4, 2);
        buffer[2] = 0, buffer[5] = 0, buffer[8] = 0;
        uint8_t h, m, s;
        h = atoi(buffer);
        m = atoi(buffer + 3);
        s = atoi(buffer + 6);
        rtc->setTime(h, m, s);
    }
    delete op;
    }

    // Goto welcome interface, waitting for "0 + down" and manager password
    while (1) {
        static uint8_t sec = 0;
        static bool refresh = true;
        uint8_t h, m, s;

        rtc->getTime(h, m, s);
        if (sec != s) {
            sec = s;
            STP_LCD::showTime(h, m, s);
        }
        if (refresh) {
            refresh = false;
            STP_LCD::send(LCD_WELCOME, strlen(LCD_WELCOME));
        }
        if (keyboard->isPress(STP_KeyMat::KEY_ID_0) && keyboard->isPress(STP_KeyMat::KEY_ID_DOWN)) {
            STP_LCD::clear();
            op = new Opera_getUsrKey(*keyboard, Opera_getUsrKey::getPassword);
            op->init();
            while (keyboard->scan())
                ;
            do {
                op->loop();
            } while (op->getResCode() != Opera::OK && DB_Sheet::checkPassword(op->getAns()));
            op->deinit();
            delete op;
            while (keyboard->scan())
                ;
            if (DB_Sheet::checkPassword(op->getAns()) == false) {
                continue;
            } else {
                break;
            }
        }
    }

    while (1) {
        // Choose Mode
        STP_LCD::clear();
        STP_LCD::showMessage(TEXT_CHOOSE_MODE_1);
    CHOOSE_MODE:
        if (keyboard->isPress(STP_KeyMat::KEY_ID_0)) {
            Mode = OPFinger;
        } else if (keyboard->isPress(STP_KeyMat::KEY_ID_1)) {
            Mode = OPNFC;
        } else if (keyboard->isPress(STP_KeyMat::KEY_ID_2)) {
            Mode = OPKey;
        } else {
            goto CHOOSE_MODE;
        }
        while (keyboard->scan())
            ;
        // Choose submode
    CHOOSE_SUBMODE_STRING:
        STP_LCD::clear();
        STP_LCD::showMessage(TEXT_CHOOSE_MODE_2);
    CHOOSE_SUBMODE:
        if (keyboard->isPress(STP_KeyMat::KEY_ID_0)) {
            subMode = Login;
        } else if (keyboard->isPress(STP_KeyMat::KEY_ID_1)) {
            subMode = Regist;
        } else {
            goto CHOOSE_SUBMODE;
        }
        while (keyboard->scan())
            ;
    GOTO_HANDLE:
        // Goto handle
        switch (Mode) {
        case OPFinger: {
            Finger_Handle(subMode == Login ? false : true);
        } break;
        case OPNFC: {
            NFC_Handle(subMode == Login ? false : true);
        } break;
        case OPKey: {
            Key_Handle(subMode == Login ? false : true);
        } break;
        default:
            continue;
        }
        // Default option: Login->Handle, Regist->Choose sub mode then goto handle
        if (subMode == Login)
            goto GOTO_HANDLE;
        else if (subMode == Regist)
            goto CHOOSE_SUBMODE_STRING;
    }
}

static bool NFC_Handle(bool isRegist)
{
    Opera* get_key = new Opera_getUsrKey(*keyboard, Opera_getUsrKey::getRoomID);
    Opera* get_password = new Opera_getUsrKey(*keyboard, Opera_getUsrKey::getPassword);
    Opera* get_nfc = new Opera_getNFC(*keyboard);
    DB_Base* nfc_data = new DB_RFID;
    DB_Base* room_id = new DB_RoomID;
    DB_Sheet* sheet;
    std::list<DB_Usr>::iterator node;

    // If is Regist mode, input RoomID first
    if (isRegist) {
        get_key->init();
        get_key->loop();
        get_key->deinit();
        if (get_key->getResCode() != Opera::OK) {
            STP_LCD::showMessage(get_key->getErrorString());
            HAL_Delay(1000);
            goto RETURN_FALSE_NFC;
        }
        room_id->overWrite(get_key->getAns());
    }

    // Then wait for RFID
    if (get_nfc->init() == false) {
        STP_LCD::showMessage(get_nfc->getErrorString());
        HAL_Delay(2000);
        goto RETURN_FALSE_NFC;
    }
    get_nfc->loop();
    get_nfc->deinit();
    if (get_key->getResCode() != Opera::OK) {
        STP_LCD::showMessage(get_nfc->getErrorString());
        HAL_Delay(1000);
        goto RETURN_FALSE_NFC;
    }
    nfc_data->overWrite(get_nfc->getAns());

    // If is Regist mode, match RoomID first
    if (isRegist) {
        for (int i = 0; i <= 3; i++) {
            sheet = new DB_Sheet((enum DB_Sheet::DB_Sector)i);
            node = sheet->search(*room_id);
            if (node != NULL) {
                break;
            }
            delete sheet;
        }
        // If found same ROOMID, check if this usr's rfid is empty.
        if (node != NULL && node->rfid.isNull() == false) {
        TRY_PASSWORD:
            // Warnning if usr wanna replace the ID
            STP_LCD::showMessage(TEXT_REPLACE);
            get_password->init();
            get_password->loop();
            get_password->deinit();
            if (get_password->getResCode() == Opera::OK && DB_Sheet::checkPassword(get_password->getAns())) {
                ;
            } else if (get_password->getResCode() == Opera::OK) {
                STP_LCD::showMessage("Wrong Password");
                HAL_Delay(1000);
                goto TRY_PASSWORD;
            } else {
                STP_LCD::showMessage(get_password->getErrorString());
                HAL_Delay(1000);
                delete sheet;
                goto RETURN_FALSE_NFC;
            }

            node->rfid.overWrite(nfc_data->data());
        } else if (node != NULL) {
            node->rfid.overWrite(nfc_data->data());
        } else {
            DB_Usr usr;
            sheet = new DB_Sheet(DB_Sheet::Sector_1);
            usr.rid.overWrite(room_id->data());
            usr.rfid.overWrite(nfc_data->data());
            sheet->add(usr);
        }
        // Working...
        STP_LCD::showNum((const char*)node->rid.data(), 4);
        STP_LCD::showMessage("Working....");
        sheet->writeBack(true);
        delete sheet;
        HAL_Delay(1000);
        // Log-in mode
    } else {
        // Match rfid in DB sheet
        for (int i = 1; i <= 3; i++) {
            sheet = new DB_Sheet((enum DB_Sheet::DB_Sector)i);
            node = sheet->search(*nfc_data);
            if (node != NULL) {
                break;
            }
            delete sheet;
        }
        if (node == NULL) {
            // Not match message
            STP_LCD::showMessage(TEXT_UNKNOWUSER);
            HAL_Delay(1000);
            goto RETURN_FALSE_NFC;
        } else {
            // TODO: send message to slaver
            STP_LCD::showNum((const char*)node->rid.data(), 4);
            STP_LCD::showMessage("Working...");
            HAL_Delay(1000);
            delete sheet;
        }
    }

    delete get_key;
    delete get_password;
    delete get_nfc;
    delete nfc_data;
    delete room_id;
    return true;

RETURN_FALSE_NFC:
    delete get_key;
    delete get_password;
    delete get_nfc;
    delete nfc_data;
    delete room_id;
    return false;
}

static bool Finger_Handle(bool isRegist)
{
    Opera* get_key = new Opera_getUsrKey(*keyboard, Opera_getUsrKey::getRoomID);
    Opera* get_password = new Opera_getUsrKey(*keyboard, Opera_getUsrKey::getPassword);
    Opera* get_finger = new Opera_getFinger(*keyboard, isRegist == true ? false : true);
    DB_Base* finger_data = new DB_FingerPrint[5];
    DB_Base* room_id = new DB_RoomID;
    DB_Sheet* sheet;
    bool isNull = true;
    std::list<DB_Usr>::iterator node;

    // If is Regist mode, input RoomID first
    if (isRegist) {
        get_key->init();
        get_key->loop();
        get_key->deinit();
        if (get_key->getResCode() != Opera::OK) {
            STP_LCD::showMessage(get_key->getErrorString());
            HAL_Delay(1000);
            goto RETURN_FALSE_FINGER;
        }
        room_id->overWrite(get_key->getAns());
    }

    // Wait for Fingerprints
    get_finger->init();
    for (int i = 0; i < ((isRegist == true) ? 5 : 1);) {
        if (isRegist) {
            char buffer[2] = { 0x00, 0x00 };
            buffer[0] = '1' + i;
            STP_LCD::showMessage(buffer);
        } else {
            STP_LCD::showMessage("");
        }
        get_finger->loop();
        if (get_finger->getResCode() == Opera::OK) {
            finger_data[i++].overWrite(get_finger->getAns());
        } else {
            get_finger->deinit();
            STP_LCD::showMessage(get_finger->getErrorString());
            HAL_Delay(1000);
            if (get_finger->getResCode() == Opera::USR_Cancel) {
                goto RETURN_FALSE_FINGER;
            }
            get_finger->init();
            continue;
        }
    }
    get_finger->deinit();

    if (isRegist) {
        // match RoomID in data sheet
        for (int i = 0; i <= 3; i++) {
            sheet = new DB_Sheet((enum DB_Sheet::DB_Sector)i);
            node = sheet->search(*room_id);

            // if RoomID and Fingerprint is exited(any Fingerprint is not null)
            if (node != NULL) {
                for (int i = 0; i < 5; i++)
                    if (node->finger[i].isNull() == false) {
                        isNull = false;
                        break;
                    }
                break;
            }
            delete sheet;
        }
        // If node != NULL, sheet is still alive
        if (node != NULL && isNull == false) {
        TRY_PASSWORD:
            // Warnning if usr wanna replace the ID
            STP_LCD::showMessage(TEXT_REPLACE);
            get_password->init();
            get_password->loop();
            get_password->deinit();
            if (get_password->getResCode() == Opera::OK && DB_Sheet::checkPassword(get_password->getAns())) {
                ;
            } else if (get_password->getResCode() == Opera::OK) {
                STP_LCD::showMessage("Wrong Password");
                HAL_Delay(1000);
                goto TRY_PASSWORD;
            } else {
                STP_LCD::showMessage(get_password->getErrorString());
                HAL_Delay(1000);
                delete sheet;
                goto RETURN_FALSE_FINGER;
            }

            for (int i = 0; i < 5; i++)
                node->finger[i].overWrite(finger_data[i]);
        } else if (node != NULL) {
            for (int i = 0; i < 5; i++)
                node->finger[i].overWrite(finger_data[i]);
        } else {
            DB_Usr usr;
            sheet = new DB_Sheet(DB_Sheet::Sector_1);
            for (int i = 0; i < 5; i++)
                usr.finger[i].overWrite(finger_data[i]);
            usr.rid.overWrite(room_id->data());
            sheet->add(usr);
        }
        // Working...
        STP_LCD::clear();
        STP_LCD::showNum((const char*)(node->rid.data()), 4);
        STP_LCD::showMessage("Working....");
        sheet->writeBack(true);
        delete sheet;
        HAL_Delay(1000);
    } else {
        // match Fingerprint[0] in data sheet
        for (int i = 1; i <= 3; i++) {
            sheet = new DB_Sheet((enum DB_Sheet::DB_Sector)i);
            node = sheet->search(finger_data[0]);
            if (node != NULL) {
                break;
            }
            delete sheet;
        }
        if (node == NULL) {
            STP_LCD::clear();
            STP_LCD::showMessage(TEXT_UNKNOWUSER);
            HAL_Delay(1000);
            goto RETURN_FALSE_FINGER;
        } else {
            // TODO: send message to slaver
            STP_LCD::clear();
            STP_LCD::showNum((const char*)node->rid.data(), 4);
            STP_LCD::showMessage("Working....");
            HAL_Delay(2000);
            delete sheet;
        }
    }
    delete get_key;
    delete get_password;
    delete get_finger;
    delete finger_data;
    delete room_id;
    return true;
RETURN_FALSE_FINGER:
    for (int i = 0; i < 5; i++) {
        uint8_t* buf = finger_data[i].data();
        uint16_t id = *buf | (*(buf + 1) << 8);
        GR307_Delete(id);
    }
    delete get_key;
    delete get_password;
    delete get_finger;
    delete finger_data;
    delete room_id;
    return false;
}

bool Key_Handle(bool isRegist)
{
    Opera* get_key = new Opera_getUsrKey(*keyboard, Opera_getUsrKey::getRoomID);
    Opera* get_password = new Opera_getUsrKey(*keyboard, Opera_getUsrKey::getPassword);
    DB_Base* room_id = new DB_RoomID;
    DB_Base* password = new DB_Password;
    DB_Sheet* sheet;
    std::list<DB_Usr>::iterator node;

    // Input room id first
    get_key->init();
    get_key->loop();
    if (get_key->getResCode() != Opera::OK) {
        STP_LCD::showMessage(get_key->getErrorString());
        HAL_Delay(1000);
        goto RETURN_FALSE_KEY;
    }
    room_id->overWrite(get_key->getAns());

    // Input room's passowrd
    STP_LCD::showMessage(TEXT_ROOMID);
    get_password->init();
    get_password->loop();
    get_password->deinit();
    if (get_password->getResCode() != Opera::OK) {
        STP_LCD::showMessage(get_password->getErrorString());
        HAL_Delay(1000);
        goto RETURN_FALSE_KEY;
    }

    // if it's register, check if roomid and passowrd is exit
    if (isRegist) {
        for (int i = 1; i <= 3; i++) {
            sheet = new DB_Sheet((enum DB_Sheet::DB_Sector)i);
            node = sheet->search(*room_id);
            if (node != NULL) {
                break;
            }
            delete sheet;
        }

        // if found a exit data, ask for manager password to replace it
        if (node != NULL && node->pid.isNull() == false) {
        TRY_PASSWORD:
            // Warnning if usr wanna replace the ID
            STP_LCD::showMessage(TEXT_REPLACE);
            get_password->init();
            get_password->loop();
            get_password->deinit();
            if (get_password->getResCode() == Opera::OK && DB_Sheet::checkPassword(get_password->getAns())) {
                ;
            } else if (get_password->getResCode() == Opera::OK) {
                STP_LCD::showMessage("Wrong Password");
                HAL_Delay(1000);
                goto TRY_PASSWORD;
            } else {
                STP_LCD::showMessage(get_password->getErrorString());
                HAL_Delay(1000);
                delete sheet;
                goto RETURN_FALSE_KEY;
            }

            node->pid.overWrite(*password);
        } else if (node != NULL) {
            sheet->writeBack(true);
        } else {
            DB_Usr usr;
            sheet = new DB_Sheet(DB_Sheet::Sector_1);
            usr.rid.overWrite(room_id->data());
            usr.pid.overWrite(password->data());
            sheet->add(usr);
        }
        // Working...
        STP_LCD::showNum((const char*)node->rid.data(), 4);
        STP_LCD::showMessage("Working....");
        sheet->writeBack(true);
        delete sheet;
        HAL_Delay(1000);
    } else {
        for (int i = 1; i <= 3; i++) {
            sheet = new DB_Sheet((enum DB_Sheet::DB_Sector)i);
            node = sheet->search(*room_id);
            if (node != NULL) {
                break;
            }
            delete sheet;
        }
        if (node == NULL) {
            // Not match message
            STP_LCD::showMessage(TEXT_UNKNOWUSER);
            HAL_Delay(2000);
            goto RETURN_FALSE_KEY;
        } else {
            if (node->pid == *password) {
                // TODO: send message to slaver
                STP_LCD::showNum((const char*)node->rid.data(), 4);
                STP_LCD::showMessage("Working....");
                HAL_Delay(1000);
            } else {
                STP_LCD::showNum("");
                STP_LCD::showMessage("Wrong Password");
                HAL_Delay(1000);
                goto RETURN_FALSE_KEY;
            }
            delete sheet;
        }
    }

    delete get_key;
    delete get_password;
    delete room_id;
    delete password;
    return true;
RETURN_FALSE_KEY:
    delete get_key;
    delete get_password;
    delete room_id;
    delete password;
    return false;
}
