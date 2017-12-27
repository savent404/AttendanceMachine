#include "opera.hpp"

static STP_ServerBase* server;
static STP_KeyMat* keyboard;
static STP_RTC* rtc;

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
    static bool init = false;
    
    if (init) {
      DB_Sheet *sheet = new DB_Sheet(DB_Sheet::Sector_1);
      sheet->clear();
      sheet->writeBack();
      delete sheet;
      //GR307_Clear();
    }
    goto DEBUG_TAG;
    // Setting the RTC parameter
    // TODO: GUI interface
    op = new Opera_getUsrKey(*keyboard, Opera_getUsrKey::getTime);
    op->init();
    do {
        op->loop();
    } while (op->getResCode() != Opera::OK);
    op->deinit();
    {
      char buffer[3*3];
      memcpy(buffer, op->getAns(), 2);
      memcpy(buffer + 3, op->getAns() + 2, 2);
      memcpy(buffer + 6, op->getAns() + 4, 2);
      buffer[2] = 0, buffer[5] = 0, buffer[8] = 0;
      uint8_t h, m, s;
      h = atoi(buffer);
      m = atoi(buffer+3);
      s = atoi(buffer+6);
      rtc->setTime(h, m, s);
    }
    delete op;

    // Goto welcome interface, waitting for "0 + down" and manager password
    // TODO: GUI interface
    op = new Opera_getUsrKey(*keyboard, Opera_getUsrKey::getPassword);
    op->init();
    do {
        op->loop();
    } while (op->getResCode() != Opera::OK && DB_Sheet::checkPassword(op->getAns()));
    op->deinit();
    delete op;
    DEBUG_TAG:
    while (1) {
        // Choose Mode
        // TODO: GUI interface
        if (keyboard->isPress(STP_KeyMat::KEY_ID_0)) {
            Mode = OPFinger;
        } else if (keyboard->isPress(STP_KeyMat::KEY_ID_1)) {
            Mode = OPNFC;
        } else if (keyboard->isPress(STP_KeyMat::KEY_ID_2)) {
            Mode = OPKey;
        } else {
            continue;
        }
        while (keyboard->scan())
            ;
    CHOOSE_SUBMODE:
        // Choose submode
        // TODO: GUI interface
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
                goto CHOOSE_SUBMODE;
            }
        }
        case OPNFC: {
            NFC_Handle(subMode == Login ? false : true);
            if (subMode == Login) {
                goto GOTO_HANDLE;
            } else {
                goto CHOOSE_SUBMODE;
            }
        }
        case OPKey: {
            Key_Handle(subMode == Login ? false : true);
            if (subMode == Login) {
                goto GOTO_HANDLE;
            } else {
                goto CHOOSE_SUBMODE;
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
            // TODO:Output err message
            goto RETURN_FALSE_NFC;
        }
        room_id->overWrite(get_key->getAns());
    }

    // Then wait for RFID
    if (get_nfc->init() == false) {
      // TODO: Output err string
      get_nfc->getErrorString();
      goto RETURN_FALSE_NFC;
    }
    get_nfc->loop();
    get_nfc->deinit();
    if (get_key->getResCode() != Opera::OK && get_key->getResCode() != Opera::USR_Cancel) {
        // TODO: output err message
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
            if (node != NULL && node->rfid.isNull() == false) {
                break;
            }
            node = NULL;
            delete sheet;
        }
        // if found same RoomID in sheet and its RFID is exited, while this sheet is stilling alive.
        if (node != NULL) {
            // TODO: Warnning if usr wnna replace the ID
            get_password->init();
            do {
                get_password->loop();
            } while (get_password->getResCode() != Opera::OK && get_password->getResCode() != Opera::USR_Cancel);
            get_password->deinit();

            if (DB_Sheet::checkPassword(get_password->getAns()) && get_password->getResCode() == Opera::OK) {
                ;
            } else {
                delete sheet;
                goto RETURN_FALSE_NFC;
            }

            // TODO: Working...
            node->rfid.overWrite(nfc_data->data());
            sheet->writeBack(true);
            delete sheet;
        } else {
            DB_Usr usr;
            // TODO: Working...
            sheet = new DB_Sheet(DB_Sheet::Sector_1);
            usr.rid.overWrite(room_id->data());
            usr.rfid.overWrite(nfc_data->data());
            sheet->add(usr);
            sheet->writeBack();
            delete sheet;
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
                delete sheet;
                break;
            }
            delete sheet;
        }
        if (node == NULL) {
            // TODO: Not match message
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
    std::list<DB_Usr>::iterator node;

    // If is Regist mode, input RoomID first
    if (isRegist) {
        get_key->init();
        get_key->loop();
        get_key->deinit();
        if (get_key->getResCode() != Opera::OK) {
            // TODO:Output err message
            goto RETURN_FALSE_FINGER;
        }
        room_id->overWrite(get_key->getAns());
    }

    // Wait for Fingerprints
    get_finger->init();
    for (int i = 0; i < ((isRegist == true) ? 5 : 1);) {
        // TODO: Output sequence of fingers
        get_finger->loop();
        if (get_finger->getResCode() == Opera::USR_Cancel) {
            get_finger->deinit();
            goto RETURN_FALSE_FINGER;
        } else if (get_finger->getResCode() == Opera::OK) {
            finger_data[i++].overWrite(get_finger->getAns());
        } else {
            // TODO: Output err message
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
                bool isNull = true;
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
            node = NULL;
            delete sheet;
        }
        // If node != NULL, sheet is still alive
        if (node != NULL) {
            // TODO: Warnning if usr wnna replace the ID
            get_password->init();
            do {
                get_password->loop();
            } while (get_password->getResCode() != Opera::OK && get_password->getResCode() != Opera::USR_Cancel);
            get_password->deinit();

            if (DB_Sheet::checkPassword(get_password->getAns()) && get_password->getResCode() == Opera::OK) {
                ;
            } else {
                delete sheet;
                goto RETURN_FALSE_FINGER;
            }

            // TODO: Working...
            for (int i = 0; i < 5; i++) {
                node->rfid.overWrite(finger_data[i]);
            }
            sheet->writeBack(true);
            delete sheet;
        } else {
            DB_Usr usr;
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
                delete sheet;
                break;
            }
            delete sheet;
        }
        if (node == NULL) {
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
    return true;
}
