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

    // Goto welcome interface, waitting for "0 + down" and manager password
    while (1) {
        static uint8_t sec = 0;
        static bool refresh = true;
        uint8_t h, m, s;
        if (refresh) {
            refresh = false;
            STP_LCD::send(LCD_WELCOME, strlen(LCD_WELCOME));
        }
        rtc->getTime(h, m, s);
        if (sec != s) {
            sec = s;
            STP_LCD::showTime(h, m, s);
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
            if (DB_Sheet::checkPassword(op->getAns()) == false) {
                delete op;
                while (keyboard->scan())
                    ;
                refresh = true;
                continue;
            }
            delete op;
            while (keyboard->scan())
                ;
            break;
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
            if (subMode == Login) {
                goto GOTO_HANDLE;
            } else {
                goto CHOOSE_SUBMODE_STRING;
            }
        }
        case OPNFC: {
            NFC_Handle(subMode == Login ? false : true);
            if (subMode == Login) {
                goto GOTO_HANDLE;
            } else {
                goto CHOOSE_SUBMODE_STRING;
            }
        }
        case OPKey: {
            Key_Handle(subMode == Login ? false : true);
            if (subMode == Login) {
                goto GOTO_HANDLE;
            } else {
                goto CHOOSE_SUBMODE_STRING;
            }
        }
        default:
            continue;
        }
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
        // Output err string
        STP_LCD::showMessage(get_nfc->getErrorString());
        HAL_Delay(2000);
        goto RETURN_FALSE_NFC;
    }
    get_nfc->loop();
    get_nfc->deinit();
    if (get_key->getResCode() != Opera::OK && get_key->getResCode() != Opera::USR_Cancel) {
        // output err message
        STP_LCD::showMessage(get_nfc->getErrorString());
        HAL_Delay(1000);
        goto RETURN_FALSE_NFC;
    } else if (get_key->getResCode() == Opera::USR_Cancel) {
        goto RETURN_FALSE_NFC;
    }
    nfc_data->overWrite(get_nfc->getAns());

    // If is Regist mode, match RoomID first
    if (isRegist) {
        for (int i = 0; i <= 3; i++) {
            sheet = new DB_Sheet((enum DB_Sheet::DB_Sector)i);
            node = sheet->search(*room_id);
            // If the roomid and its rfid is exited, we should ask for replace data
            if (node != NULL) {
                break;
            }
            delete sheet;
        }
        // if found same RoomID in sheet and its RFID is exited, while this sheet is stilling alive.
        if (node != NULL && node->rfid.isNull() == false) {
            // Warnning if usr wanna replace the ID
            STP_LCD::showMessage(TEXT_REPLACE);
            get_password->init();
            do {
                get_password->loop();
            } while (get_password->getResCode() != Opera::OK && get_password->getResCode() != Opera::USR_Cancel);
            get_password->deinit();

            if (DB_Sheet::checkPassword(get_password->getAns()) && get_password->getResCode() == Opera::OK) {
                ;
            } else {
                delete sheet;
                STP_LCD::showMessage(TEXT_CANCEL);
                goto RETURN_FALSE_NFC;
            }

            // Working...
            STP_LCD::showNum((const char*)node->rid.data(), 4);
            STP_LCD::showMessage("Working....");
            node->rfid.overWrite(nfc_data->data());
            sheet->writeBack(true);
            delete sheet;
            HAL_Delay(1000);
        } else if (node != NULL) {
            // Working...
            STP_LCD::showNum((const char*)node->rid.data(), 4);
            STP_LCD::showMessage("Working....");
            node->rfid.overWrite(nfc_data->data());
            sheet->writeBack(true);
            delete sheet;
            HAL_Delay(1000);
        } else {
            DB_Usr usr;
            // Working...
            STP_LCD::showNum((const char*)room_id->data(), 4);
            STP_LCD::showMessage("Working...");
            sheet = new DB_Sheet(DB_Sheet::Sector_1);
            usr.rid.overWrite(room_id->data());
            usr.rfid.overWrite(nfc_data->data());
            sheet->add(usr);
            sheet->writeBack();
            delete sheet;
            HAL_Delay(1000);
        }

        // Log-in mode
    } else {
        // Match rfid in DB sheet
        nfc_data->overWrite(get_nfc->getAns());
        for (int i = 1; i <= 3; i++) {
            sheet = new DB_Sheet((enum DB_Sheet::DB_Sector)i);
            node = sheet->search(*nfc_data);
            if (node != NULL) {
                // TODO: send message to slaver
                STP_LCD::showNum((const char*)node->rid.data(), 4);
                STP_LCD::showMessage("Working....");
                delete sheet;
                HAL_Delay(1000);
                break;
            }
            delete sheet;
        }
        if (node == NULL) {
            // Not match message
            STP_LCD::showMessage(TEXT_UNKNOWUSER);
            HAL_Delay(2000);
            goto RETURN_FALSE_NFC;
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
        if (get_finger->getResCode() == Opera::USR_Cancel) {
            get_finger->deinit();
            STP_LCD::showMessage(TEXT_CANCEL);
            HAL_Delay(1000);
            goto RETURN_FALSE_FINGER;
        } else if (get_finger->getResCode() == Opera::OK) {
            finger_data[i++].overWrite(get_finger->getAns());
        } else {
            STP_LCD::showMessage(get_finger->getErrorString());
            HAL_Delay(2000);
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
                for (int i = 0; i < 5; i++) {
                    if (node->finger[i].isNull() == false) {
                        isNull = false;
                        break;
                    }
                }
                if (isNull == false) {
                    break;
                }
            }
            delete sheet;
        }
        // If node != NULL, sheet is still alive
        if (node != NULL && isNull == false) {
            // Warnning if usr wnna replace the ID
            STP_LCD::showMessage(TEXT_REPLACE);
            get_password->init();
            do {
                get_password->loop();
            } while (get_password->getResCode() != Opera::OK && get_password->getResCode() != Opera::USR_Cancel);
            get_password->deinit();
            if (DB_Sheet::checkPassword(get_password->getAns()) && get_password->getResCode() == Opera::OK) {
                ;
            } else {
                delete sheet;
                STP_LCD::showMessage(TEXT_CANCEL);
                HAL_Delay(1000);
                goto RETURN_FALSE_FINGER;
            }

            // Working...
            STP_LCD::clear();
            STP_LCD::showNum((const char*)(node->rid.data()), 4);
            STP_LCD::showMessage("Working....");
            HAL_Delay(2000);
            for (int i = 0; i < 5; i++) {
                node->rfid.overWrite(finger_data[i]);
            }
            sheet->writeBack(true);
            delete sheet;
        } else if (node != NULL) {
            // Working...
            STP_LCD::clear();
            STP_LCD::showNum((const char*)(node->rid.data()), 4);
            STP_LCD::showMessage("Working....");
            HAL_Delay(2000);
            for (int i = 0; i < 5; i++) {
                node->rfid.overWrite(finger_data[i]);
            }
            sheet->writeBack(true);
            delete sheet;
        } else {
            DB_Usr usr;
            STP_LCD::clear();
            STP_LCD::showNum((const char*)room_id->data(), 4);
            STP_LCD::showMessage("Working....");
            HAL_Delay(2000);
            sheet = new DB_Sheet(DB_Sheet::Sector_1);
            for (int i = 0; i < 5; i++) {
                usr.finger[i].overWrite(finger_data[i]);
            }
            usr.rid.overWrite(room_id->data());
            sheet->add(usr);
            sheet->writeBack();
            delete sheet;
        }
    } else {
        // match Fingerprint[0] in data sheet
        for (int i = 1; i <= 3; i++) {
            sheet = new DB_Sheet((enum DB_Sheet::DB_Sector)i);
            node = sheet->search(finger_data[0]);
            if (node != NULL) {
                // TODO: send message to slaver
                STP_LCD::clear();
                STP_LCD::showNum((const char*)node->rid.data(), 4);
                STP_LCD::showMessage("Working....");
                HAL_Delay(2000);
                delete sheet;
                break;
            }
            delete sheet;
        }
        if (node == NULL) {
            STP_LCD::clear();
            STP_LCD::showMessage(TEXT_UNKNOWUSER);
            HAL_Delay(1000);
            goto RETURN_FALSE_FINGER;
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
    do {
        get_password->loop();
    } while (get_password->getResCode() != Opera::OK && get_password->getResCode() != Opera::USR_Cancel);
    get_password->deinit();

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
            // Warnning if usr wanna replace the ID
            STP_LCD::showMessage(TEXT_REPLACE);
            get_password->init();
            do {
                get_password->loop();
            } while (get_password->getResCode() != Opera::OK && get_password->getResCode() != Opera::USR_Cancel);
            get_password->deinit();

            if (DB_Sheet::checkPassword(get_password->getAns()) && get_password->getResCode() == Opera::OK) {
                ;
            } else {
                delete sheet;
                STP_LCD::showMessage(TEXT_CANCEL);
                goto RETURN_FALSE_KEY;
            }

            // Working...
            STP_LCD::showNum((const char*)node->rid.data(), 4);
            STP_LCD::showMessage("Working....");
            node->pid.overWrite(*password);
            sheet->writeBack(true);
            delete sheet;
            HAL_Delay(1000);
        } else if (node != NULL) {
            // Working...
            STP_LCD::showNum((const char*)node->rid.data(), 4);
            STP_LCD::showMessage("Working....");
            node->pid.overWrite(*password);
            sheet->writeBack(true);
            delete sheet;
            HAL_Delay(1000);
        } else {
            // Working...
            DB_Usr usr;
            STP_LCD::showNum((const char*)room_id->data(), 4);
            STP_LCD::showMessage("Working...");
            sheet = new DB_Sheet(DB_Sheet::Sector_1);
            usr.rid.overWrite(room_id->data());
            usr.pid.overWrite(password->data());
            sheet->add(usr);
            sheet->writeBack();
            delete sheet;
            HAL_Delay(1000);
        }
    } else {
        for (int i = 1; i <= 3; i++) {
            sheet = new DB_Sheet((enum DB_Sheet::DB_Sector)i);
            node = sheet->search(*room_id);
            if (node != NULL && node->pid == *password) {
                // TODO: send message to slaver
                STP_LCD::showNum((const char*)node->rid.data(), 4);
                STP_LCD::showMessage("Working....");
                delete sheet;
                HAL_Delay(1000);
                break;
            }
            delete sheet;
        }
        if (node == NULL) {
            // Not match message
            STP_LCD::showMessage(TEXT_UNKNOWUSER);
            HAL_Delay(2000);
            goto RETURN_FALSE_KEY;
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

