#include "opera.hpp"

/**
 * 测试从机定义
 */
#ifndef SLAVE_DEBUG
#define SLAVE_DEBUG 0
#endif

/** 说明
 *  - 所有c++ class 尽量使用指针 new/delete
 *      原因是只占用heap,方便查找问题以及内存控制
 *  - 三种登入模式独立一个函数
 *      操作相对独立，且便于后期更改
 */

// 全局对象, 在OP_Handle开头初始化
STP_ServerBase* server;
STP_KeyMat* keyboard;
STP_RTC* rtc;

/**
 * @brief  获取RFID的子流程
 * @param  isRegist:true 注册流程，否则为登入流程
 * @retvl  返回上一级为true
 */
static bool NFC_Handle(bool isRegist);

/**
 * @brief  获取FingerPrint的子流程
 * @param  isRegist:true 注册流程，否则为登入流程
 * @retvl  返回上一级为true
 */
static bool Finger_Handle(bool isRegist);

/**
 * @brief  获取key的子流程
 * @param  isRegist:true 注册流程，否则为登入流程
 * @retvl  返回上一级为true
 */
static bool Key_Handle(bool isRegist);

/**
 * @brief  下位机串口回调函数
 * @note   处于中断中，由于都是异常消息，默认触发软件复位
 */
#if SLAVE_DEBUG == 0
void rec_callback(enum STP_ServerBase::CMD cmd, const uint8_t* buffer, size_t size)
{
    STP_LCD::clear();
    switch (cmd) {
    case STP_ServerBase::CMD_ERROR_UNKNOW: {
        STP_LCD::showLabel("Error unkown", 0, 0, 32);
    } break;
    case STP_ServerBase::CMD_ERROR_BREAKIN: {
        STP_LCD::showLabel("Error break in", 0, 0, 32);
    } break;
    case STP_ServerBase::CMD_ERROR_LIMIT: {
        STP_LCD::showLabel("Error Limit", 0, 0, 32);
    } break;
    case STP_ServerBase::CMD_ERROR_CHAT: {
        STP_LCD::showLabel("Error Chat", 0, 0, 32);
    } break;
    case STP_ServerBase::CMD_TIMEOUT:
        STP_LCD::showLabel("Time out", 0, 0, 32);
        break;
    default:
        char buf[20];
        sprintf(buf, "unkown meesage:%x", (int)cmd);
        STP_LCD::showLabel(buf, 0, 0, 32);
    }
    HAL_GPIO_WritePin(BELL_GPIO_Port, BELL_Pin, GPIO_PIN_SET);
    while (1) {
        if (keyboard->isPress(STP_KeyMat::KEY_ID_NO, false)) {
            HAL_NVIC_SystemReset();
        }
    }
}
#else
void rec_callback(enum STP_ServerBase::CMD cmd, const uint8_t* buffer, size_t size)
{
    const char* title = "";
    char buff[50];
    switch (cmd) {
    case STP_ServerBase::CMD_ID_FINGER:
        title = "CMD_ID_FINGER";
        break;
    case STP_ServerBase::CMD_ID_RFID:
        title = "CMD_ID_RFID";
        break;
    case STP_ServerBase::CMD_ID_KEY:
        title = "CMD_ID_KEY";
        break;
    case STP_ServerBase::CMD_UP_Start:
        title = "CMD_UP_Start";
        break;
    case STP_ServerBase::CMD_UP_Stop:
        title = "CMD_UP_Stop";
        break;
    case STP_ServerBase::CMD_DOWN_Start:
        title = "CMD_DOWN_Start";
        break;
    case STP_ServerBase::CMD_DOWN_Stop:
        title = "CMD_DOWN_Stop";
        break;
    case STP_ServerBase::CMD_RIGHT_Start:
        title = "CMD_RIGHT_Start";
        break;
    case STP_ServerBase::CMD_RIGHT_Stop:
        title = "CMD_RIGHT_Stop";
        break;
    case STP_ServerBase::CMD_LEFT_Start:
        title = "CMD_LEFT_Stop";
        break;
    case STP_ServerBase::CMD_SECURITY:
        title = "CMD_SECURITY";
        break;
    case STP_ServerBase::CMD_ERROR_UNKNOW:
        title = "ERROR UNKNOW";
        break;
    case STP_ServerBase::CMD_ERROR_BREAKIN:
        title = "ERROR Break in";
        break;
    case STP_ServerBase::CMD_ERROR_LIMIT:
        title = "ERROR Limit";
        break;
    case STP_ServerBase::CMD_ERROR_CHAT:
        title = "ERROR Chat";
        break;
    case STP_ServerBase::CMD_ASK:
        title = "Ask";
        break;
    case STP_ServerBase::CMD_ACK:
        title = "Ack";
        break;
    case STP_ServerBase::CMD_TIMEOUT:
        title = "Time out";
        break;
    default:
        title = "CMD::Unkown";
    }
    strcpy(buff, title);
    if (size != 0) {
        strcat(buff, " ");
        for (int i = 0; i < size; i++) {
            char a[3] = "  ";
            a[0] = buffer[i];
            strcat(buff, a);
        }
    }
    STP_LCD::send(buff, strlen(buff), false);
}
#endif

/**
 * CM4定义的EXTI中断回调函数
 */
extern "C" void EXTI2_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);
}

/**
 * STM32库函数定义的EXTI中断回调函数
 * 执行安全按钮的操作
 */
extern "C" void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    server->sendMessage(STP_ServerBase::CMD_SECURITY, "", 0);
    HAL_GPIO_WritePin(BELL_GPIO_Port, BELL_Pin, GPIO_PIN_SET);
    STP_LCD::clear();
    STP_LCD::showLabel(TEXT_SECUR);
    while (keyboard->isPress(STP_KeyMat::KEY_ID_NO, false) == false) {
    }
    HAL_NVIC_SystemReset();
}
/*********************************************
 * @name   OP_Handle
 * @brief  操作执行实体
 */
#if SLAVE_DEBUG == 0
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

    // Enable recive slave's message, and check slave is avalibel first
    server->sendMessage(STP_ServerBase::CMD_ASK, "", 0);
    server->reciMessage();

    // Software Reset flag
    // 若软件复位,该标志为1。代表当前为重启，不需要重新设定时间
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

WELCOME_FRESH:
    GUI_Welcome(*rtc, -1);
    while (1) {
        GUI_Welcome(*rtc, 1);
        if (keyboard->isPress(STP_KeyMat::KEY_ID_0) && keyboard->isPress(STP_KeyMat::KEY_ID_DOWN)) {
          TRY_AGAIN:
            while (keyboard->scan())
                ;
            op = new Opera_getUsrKey(*keyboard, Opera_getUsrKey::getPassword_Root);
            op->init();
            op->loop();
            op->deinit();
            if (DB_Sheet::checkPassword(op->getAns()) == true) {
                delete op;
                break;
              } else if (op->getResCode() == Opera::USR_Cancel) {
                delete op;
                goto WELCOME_FRESH;
            } else {
              delete op;
              goto TRY_AGAIN;
            }
        }
    }

    while (1) {
        // Choose Mode
        GUI_ChooseMode(*rtc, -1);
    CHOOSE_MODE:
        if (keyboard->isPress(STP_KeyMat::KEY_ID_0)) {
            Mode = OPFinger;
        } else if (keyboard->isPress(STP_KeyMat::KEY_ID_1)) {
            Mode = OPNFC;
        } else if (keyboard->isPress(STP_KeyMat::KEY_ID_2)) {
            Mode = OPKey;
        } else if (keyboard->isPress(STP_KeyMat::KEY_ID_3)) {
            while (keyboard->scan())
                ;
            Opera* get = new Opera_getUsrKey(*keyboard, Opera_getUsrKey::getPassword_Root);
            do {
                get->init();
                get->loop();
            } while (get->getResCode() != Opera::OK && get->getResCode() != Opera::USR_Cancel);
            get->deinit();
            if (get->getResCode() == Opera::USR_Cancel) {
                delete get;
                continue;
            }
            DB_Sheet* sheet = new DB_Sheet(DB_Sheet::Sector_1);
            sheet->changePassword(get->getAns());
            sheet->writeBack();
            delete sheet;
            delete get;
            continue;
        } else if (keyboard->isPress(STP_KeyMat::KEY_ID_4)) {
            uint8_t h, m, s;
            uint8_t sh, sm, ss;
            rtc->getTime(h, m, s);
            STP_LCD::showLabel("Press 5 sec to erase flash", 390, 290, 24);
            while (keyboard->isPress(STP_KeyMat::KEY_ID_4)) {
                rtc->getTime(sh, sm, ss);
                if (sm * 60 + ss - m * 60 - s > 5)
                    break;
            }
            if (sm * 60 + ss - m * 60 - s > 5) {
                for (int i = 0; i < 3; i++) {
                    DB_Sheet* sheet = new DB_Sheet((enum DB_Sheet::DB_Sector)i);
                    sheet->clear();
                    sheet->writeBack();
                    delete sheet;
                }
                STP_LCD::showLabel("Flash erase OK                  ", 390, 290, 24);
                HAL_Delay(2000);
            }
            while (keyboard->scan())
                ;
            continue;
        } else if (keyboard->isPress(STP_KeyMat::KEY_ID_NO)) {
            while (keyboard->scan())
                ;
            // continue;
            goto WELCOME_FRESH;
        } else {
            goto CHOOSE_MODE;
        }
        while (keyboard->scan())
            ;
        // Choose submode
    CHOOSE_SUBMODE_STRING:
        GUI_ChooseSubMode(*rtc, -1, (int)Mode);
    CHOOSE_SUBMODE:
        if (keyboard->isPress(STP_KeyMat::KEY_ID_0)) {
            subMode = Login;
        } else if (keyboard->isPress(STP_KeyMat::KEY_ID_1)) {
            subMode = Regist;
        } else if (keyboard->isPress(STP_KeyMat::KEY_ID_NO)) {
            while (keyboard->scan())
                ;
            continue;
        } else {
            goto CHOOSE_SUBMODE;
        }
        while (keyboard->scan())
            ;
    GOTO_HANDLE:
        // Goto handle
        switch (Mode) {
        case OPFinger: {
            if (Finger_Handle(subMode == Login ? false : true)) {
                goto CHOOSE_SUBMODE_STRING;
            }
        } break;
        case OPNFC: {
            if (NFC_Handle(subMode == Login ? false : true)) {
                goto CHOOSE_SUBMODE_STRING;
            }
        } break;
        case OPKey: {
            if (Key_Handle(subMode == Login ? false : true)) {
                goto CHOOSE_SUBMODE_STRING;
            }
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

#else
void OP_Handle(void)
{
    STP_LCD::send("TERM;", strlen("TERM"), false);
    server = new STP_ServerRS485(&huart1);
  keyboard = new STP_KeyMat;
    server->reciMessage();
    HAL_Delay(500);
    STP_LCD::send("HELLO", strlen("HELLO"), false);
    while (1) {
        if (keyboard->isPress(STP_KeyMat::KEY_ID_0)) {
            while (keyboard->scan())
                ;
            server->sendMessage(STP_ServerBase::CMD_ERROR_UNKNOW, "", 0);
            server->reciMessage();
        }
        if (keyboard->isPress(STP_KeyMat::KEY_ID_1)) {
            while (keyboard->scan())
                ;
            server->sendMessage(STP_ServerBase::CMD_ERROR_BREAKIN);
            server->reciMessage();
        }
        if (keyboard->isPress(STP_KeyMat::KEY_ID_2)) {
            while (keyboard->scan())
                ;
            server->sendMessage(STP_ServerBase::CMD_ERROR_LIMIT);
            server->reciMessage();
        }
        if (keyboard->isPress(STP_KeyMat::KEY_ID_3)) {
            while (keyboard->scan())
                ;
            server->sendMessage(STP_ServerBase::CMD_ERROR_CHAT);
            server->reciMessage();
        }
    }
}
#endif
static bool NFC_Handle(bool isRegist)
{
    Opera* get_key = new Opera_getUsrKey(*keyboard, Opera_getUsrKey::getRoomID_NFC);
    Opera* get_password = new Opera_getUsrKey(*keyboard, Opera_getUsrKey::getPassword_Usr);
    Opera* get_nfc = new Opera_getNFC(*keyboard, !isRegist);
    DB_Base* nfc_data = new DB_RFID;
    DB_Base* room_id = new DB_RoomID;
    DB_Sheet* sheet;
    std::list<DB_Usr>::iterator node;

    // If is Regist mode, input RoomID first
    if (isRegist) {
        get_key->init();
        get_key->loop();
        get_key->deinit();
        if (get_key->getResCode() == Opera::USR_Cancel) {
            goto RETURN_EXIT_NFC;
        } else if (get_key->getResCode() != Opera::OK) {
            STP_LCD::showLabel(get_key->getErrorString());
            HAL_Delay(1000);
            goto RETURN_FALSE_NFC;
        }
        room_id->overWrite(get_key->getAns());
    }

    // Then wait for RFID
    if (get_nfc->init() == false) {
        STP_LCD::showLabel(get_nfc->getErrorString());
        HAL_Delay(2000);
        goto RETURN_FALSE_NFC;
    }
    get_nfc->loop();
    get_nfc->deinit();
    if (get_nfc->getResCode() == Opera::USR_Cancel) {
        goto RETURN_EXIT_NFC;
    } else if (get_nfc->getResCode() != Opera::OK) {
        STP_LCD::showLabel(get_nfc->getErrorString());
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
            get_password->init();
            STP_LCD::showLabel(TEXT_REPLACE);
            get_password->loop();
            get_password->deinit();
            if (get_password->getResCode() == Opera::OK && DB_Sheet::checkPassword(get_password->getAns())) {
                ;
            } else if (get_password->getResCode() == Opera::OK) {
                STP_LCD::showLabel("Wrong Password");
                HAL_Delay(1000);
                goto TRY_PASSWORD;
            } else if (get_password->getResCode() == Opera::USR_Cancel) {
                delete sheet;
                goto RETURN_FALSE_NFC;
            } else {
                STP_LCD::showLabel(get_password->getErrorString());
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
        GUI_Working(*rtc, -1, (const char*)(room_id->data()));
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
            STP_LCD::showLabel(TEXT_UNKNOWUSER);
            HAL_Delay(1000);
            goto RETURN_FALSE_NFC;
        } else {
            // send message to slaver
            server->sendMessage(STP_ServerBase::CMD_ID_RFID, node->rid.data(), 4);
            server->reciMessage();
            GUI_Working(*rtc, -1, (char*)node->rid.data());
            HAL_Delay(1000);
            delete sheet;
        }
    }

    delete get_key;
    delete get_password;
    delete get_nfc;
    delete nfc_data;
    delete room_id;
    return false;

RETURN_FALSE_NFC:
    delete get_key;
    delete get_password;
    delete get_nfc;
    delete nfc_data;
    delete room_id;
    return false;
RETURN_EXIT_NFC:
    delete get_key;
    delete get_password;
    delete get_nfc;
    delete nfc_data;
    delete room_id;
    return true;
}

static bool Finger_Handle(bool isRegist)
{
    Opera* get_key = new Opera_getUsrKey(*keyboard, Opera_getUsrKey::getRoomID_Finger);
    Opera* get_password = new Opera_getUsrKey(*keyboard, Opera_getUsrKey::getPassword_Usr);
    //Opera* get_finger; // = new Opera_getFinger(*keyboard, isRegist == true ? false : true);
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
        if (get_key->getResCode() == Opera::USR_Cancel) {
            goto RETURN_EXIT_FINGER;
        } else if (get_key->getResCode() != Opera::OK) {
            STP_LCD::showLabel(get_key->getErrorString());
            HAL_Delay(1000);
            goto RETURN_FALSE_FINGER;
        }
        room_id->overWrite(get_key->getAns());
    }

    // Wait for Fingerprints
    for (int i = 0; i < ((isRegist == true) ? 5 : 1);) {
        Opera_getFinger get_finger(*keyboard, isRegist == true ? i + 1 : 0);
        get_finger.init();
        get_finger.loop();
        get_finger.deinit();
        if (get_finger.getResCode() == Opera::OK) {
            finger_data[i++].overWrite(get_finger.getAns());
        } else if (get_finger.getResCode() == Opera::USR_Cancel) {
            goto RETURN_EXIT_FINGER;
        } else {
            STP_LCD::showLabel(get_finger.getErrorString());
            HAL_Delay(1000);
            continue;
        }
    }
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
            get_password->init();
            STP_LCD::showLabel(TEXT_REPLACE);
            get_password->loop();
            get_password->deinit();
            if (get_password->getResCode() == Opera::OK && DB_Sheet::checkPassword(get_password->getAns())) {
                ;
            } else if (get_password->getResCode() == Opera::OK) {
                STP_LCD::showLabel("Wrong Password");
                HAL_Delay(1000);
                goto TRY_PASSWORD;
            } else if (get_password->getResCode() == Opera::USR_Cancel) {
                delete sheet;
                goto RETURN_FALSE_FINGER;
            } else {
                STP_LCD::showLabel(get_password->getErrorString());
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
        GUI_Working(*rtc, -1, (const char*)(room_id->data()));
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
            STP_LCD::showLabel(TEXT_UNKNOWUSER);
            HAL_Delay(1000);
            goto RETURN_FALSE_FINGER;
        } else {
            // send message to slaver
            server->sendMessage(STP_ServerBase::CMD_ID_FINGER, (node->rid.data()), 4);
            server->reciMessage();
            GUI_Working(*rtc, -1, (const char*)(node->rid.data()));
            HAL_Delay(2000);
            delete sheet;
        }
    }
    delete get_key;
    delete get_password;
    delete finger_data;
    delete room_id;
    return false;
RETURN_FALSE_FINGER:
    for (int i = 0; i < 5; i++) {
        uint8_t* buf = finger_data[i].data();
        uint16_t id = *buf | (*(buf + 1) << 8);
        GR307_Delete(id);
    }
    delete get_key;
    delete get_password;
    delete finger_data;
    delete room_id;
    return false;
RETURN_EXIT_FINGER:
    for (int i = 0; i < 5; i++) {
        uint8_t* buf = finger_data[i].data();
        uint16_t id = *buf | (*(buf + 1) << 8);
        GR307_Delete(id);
    }
    delete get_key;
    delete get_password;
    delete finger_data;
    delete room_id;
    return true;
}

bool Key_Handle(bool isRegist)
{
    Opera* get_key = new Opera_getUsrKey(*keyboard, Opera_getUsrKey::getRoomID_Password);
    Opera* get_password = new Opera_getUsrKey(*keyboard, Opera_getUsrKey::getPassword_Usr);
    DB_Base* room_id = new DB_RoomID;
    DB_Base* password = new DB_Password;
    DB_Sheet* sheet;
    std::list<DB_Usr>::iterator node;

    // Input room id first
    get_key->init();
    get_key->loop();
    if (get_key->getResCode() == Opera::USR_Cancel) {
        goto RETURN_EXIT_KEY;
    } else if (get_key->getResCode() != Opera::OK) {
        STP_LCD::showLabel(get_key->getErrorString());
        HAL_Delay(1000);
        goto RETURN_FALSE_KEY;
    }
    room_id->overWrite(get_key->getAns());
    STP_LCD::clear();
    // Input room's passowrd
    get_password->init();
    get_password->loop();
    get_password->deinit();
    if (get_password->getResCode() == Opera::USR_Cancel) {
        goto RETURN_EXIT_KEY;
    } else if (get_password->getResCode() != Opera::OK) {
        STP_LCD::showLabel(get_password->getErrorString());
        HAL_Delay(1000);
        goto RETURN_FALSE_KEY;
    }
    password->overWrite(get_password->getAns());
    STP_LCD::clear();

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
            get_password->init();
            STP_LCD::showLabel(TEXT_REPLACE);
            get_password->loop();
            get_password->deinit();
            if (get_password->getResCode() == Opera::OK && DB_Sheet::checkPassword(get_password->getAns())) {
                ;
            } else if (get_password->getResCode() == Opera::OK) {
                STP_LCD::showLabel("Wrong Password");
                HAL_Delay(1000);
                goto TRY_PASSWORD;
            } else if (get_password->getResCode() == Opera::USR_Cancel) {
                delete sheet;
                goto RETURN_FALSE_KEY;
            } else {
                STP_LCD::showLabel(get_password->getErrorString());
                HAL_Delay(1000);
                delete sheet;
                goto RETURN_FALSE_KEY;
            }

            node->pid.overWrite(*password);
        } else if (node != NULL) {
            node->pid.overWrite(password->data());
        } else {
            DB_Usr usr;
            sheet = new DB_Sheet(DB_Sheet::Sector_1);
            usr.rid.overWrite(room_id->data());
            usr.pid.overWrite(password->data());
            sheet->add(usr);
        }
        // Working...
        GUI_Working(*rtc, -1, (const char*)(room_id->data()));
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
            STP_LCD::showLabel(TEXT_UNKNOWUSER);
            HAL_Delay(2000);
            goto RETURN_FALSE_KEY;
        } else {
            if (node->pid == *password) {
                server->sendMessage(STP_ServerBase::CMD_ID_KEY, node->rid.data(), 4);
                server->reciMessage();
                GUI_Operation(*rtc, -1, 0);
                GUI_Operation(*rtc, 1, 0);
                while (1) {
                    static uint16_t key_status = 0;
                    static uint8_t output = 0;
                    uint16_t buf = keyboard->scan();
                    uint16_t xbuf = key_status ^ buf;

                    if (xbuf & (1 << STP_KeyMat::KEY_ID_UP)) {
                        if (buf & (1 << STP_KeyMat::KEY_ID_UP)) {
                            server->sendMessage(STP_ServerBase::CMD_UP_Start);
                            server->reciMessage();
                            output |= 0x01;
                        } else {
                            server->sendMessage(STP_ServerBase::CMD_UP_Stop);
                            server->reciMessage();
                            output &= ~0x01;
                        }
                    }
                    if (xbuf & (1 << STP_KeyMat::KEY_ID_DOWN)) {
                        if (buf & (1 << STP_KeyMat::KEY_ID_DOWN)) {
                            server->sendMessage(STP_ServerBase::CMD_DOWN_Start);
                            server->reciMessage();
                            output |= 0x02;
                        } else {
                            server->sendMessage(STP_ServerBase::CMD_DOWN_Stop);
                            server->reciMessage();
                            output &= ~0x02;
                        }
                    }
                    if (xbuf & (1 << STP_KeyMat::KEY_ID_LEFT)) {
                        if (buf & (1 << STP_KeyMat::KEY_ID_LEFT)) {
                            server->sendMessage(STP_ServerBase::CMD_LEFT_Start);
                            server->reciMessage();
                            output |= 0x04;
                        } else {
                            server->sendMessage(STP_ServerBase::CMD_LEFT_Stop);
                            server->reciMessage();
                            output &= ~0x04;
                        }
                    }
                    if (xbuf & (1 << STP_KeyMat::KEY_ID_RIGHT)) {
                        if (buf & (1 << STP_KeyMat::KEY_ID_RIGHT)) {
                            server->sendMessage(STP_ServerBase::CMD_RIGHT_Start);
                            server->reciMessage();
                            output |= 0x08;
                        } else {
                            server->sendMessage(STP_ServerBase::CMD_RIGHT_Stop);
                            server->reciMessage();
                            output &= ~0x08;
                        }
                    }
                    if (keyboard->isPress(STP_KeyMat::KEY_ID_YES)) {
                        // release all key
                        if (buf & (1 << STP_KeyMat::KEY_ID_UP)) {
                            server->sendMessage(STP_ServerBase::CMD_UP_Stop);
                            server->reciMessage();
                        }
                        if (buf & (1 << STP_KeyMat::KEY_ID_DOWN)) {
                            server->sendMessage(STP_ServerBase::CMD_DOWN_Stop);
                            server->reciMessage();
                        }
                        if (buf & (1 << STP_KeyMat::KEY_ID_LEFT)) {
                            server->sendMessage(STP_ServerBase::CMD_LEFT_Stop);
                            server->reciMessage();
                        }
                        if (buf & (1 << STP_KeyMat::KEY_ID_RIGHT)) {
                            server->sendMessage(STP_ServerBase::CMD_RIGHT_Stop);
                            server->reciMessage();
                        }
                        while (keyboard->scan())
                            ;
                        break;
                    }
                    // if (xbuf & ((1 << STP_KeyMat::KEY_ID_UP) | (1 << STP_KeyMat::KEY_ID_DOWN) | (1 << STP_KeyMat::KEY_ID_LEFT) | (1 << STP_KeyMat::KEY_ID_RIGHT))) {
                    if (1) { // LCD show
                        GUI_Operation(*rtc, 0, output);
                    }
                    key_status = buf;
                }
            } else {
                STP_LCD::showLabel("Wrong Password");
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
    return false;
RETURN_FALSE_KEY:
    delete get_key;
    delete get_password;
    delete room_id;
    delete password;
    return false;
RETURN_EXIT_KEY:
    delete get_key;
    delete get_password;
    delete room_id;
    delete password;
    return true;
}
