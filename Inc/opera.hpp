#pragma once
#include "main.h"
/** Drivers *********************/
#include "Adafruit_NFCShield_I2C.h"
#include "DataSheet.hpp"
#include "GR307.h"
#include "STP_Bell.hpp"
#include "STP_Keymat.hpp"
#include "STP_LCD.hpp"
#include "STP_RTC.hpp"
#include "STP_Server.hpp"

extern "C" void OP_Handle();

extern void GUI_Welcome(STP_RTC& rtc, int32_t timeDelay);
extern void GUI_Working(STP_RTC& rtc, int32_t timeDelay, const char* roomID);
extern void GUI_InputFinger(STP_RTC& rtc, int32_t timeDelay, bool isPress, uint32_t idth = 0);
extern void GUI_InputNFC(STP_RTC& rtc, int32_t timeDelay, bool);
extern void GUI_InputPassword(STP_RTC& rtc, int32_t timeDelay, bool isRoot, char intputLen = 0);
extern void GUI_InputRoomID(STP_RTC& rtc, int32_t timeDelay, const char* meesage, uint8_t id);
extern void GUI_InputTime(STP_RTC& rtc, int32_t timeDelay, const char* inputChar);
extern void GUI_ChooseMode(STP_RTC& rtc, int32_t timeDelay);
extern void GUI_ChooseSubMode(STP_RTC& rtc, int32_t timeDelay, uint8_t id);
extern void GUI_Operation(STP_RTC & rtc, int32_t timeDelay, uint8_t up_down_left_right);
class Opera {
public:
    enum ErrorType {
        OK = 0,
        MEM_Error = 1,
        DRIVER_Error = 2,
        USR_Error = 3,
        USR_Cancel = 4,
    };
    Opera(uint32_t ans_size)
    {
        ErrorCode = OK;
        ErrorStr = (const char*)"Unknow Error";
        ans = (uint8_t*)malloc(ans_size);
        for (int i = 0; i < ans_size; i++) {
            ans[i] = '\0';
        }
    }
    virtual ~Opera() { free(ans); }

    virtual bool init() = 0;
    virtual bool deinit() = 0;
    virtual bool exitCheck() = 0;
    virtual bool loop() = 0;

    enum ErrorType getResCode() const
    {
        return ErrorCode;
    }
    const char* getErrorString() const
    {
        return ErrorStr;
    }
    uint8_t* getAns() const
    {
        return ans;
    }

protected:
    enum ErrorType ErrorCode;
    const char* ErrorStr;
    uint8_t* ans;
};

class Opera_getNFC : public Opera {
public:
    Opera_getNFC(STP_KeyMat& kb, bool islogin);
    virtual bool init();
    virtual bool deinit();
    virtual bool exitCheck();
    virtual bool loop();

private:
    Adafruit_NFCShield_I2C* nfc;
    STP_KeyMat* keymat;
    bool isLogin;
};

class Opera_getFinger : public Opera {
public:
    Opera_getFinger(STP_KeyMat& kb, uint8_t th);
    virtual bool init();
    virtual bool deinit();
    virtual bool exitCheck();
    virtual bool loop();

private:
    STP_KeyMat* keymat;
    uint8_t _th;
};

class Opera_getUsrKey : public Opera {
public:
    enum getMode {
        getPassword_Root = 0,
        getPassword_Usr,
        getTime,
        getRoomID_Finger,
        getRoomID_NFC,
        getRoomID_Password,
    };
    Opera_getUsrKey(STP_KeyMat& kb, enum getMode mode);
    virtual bool init();
    virtual bool deinit();
    virtual bool exitCheck();
    virtual bool loop();

private:
    STP_KeyMat* keymat;
    uint8_t pos;
    enum getMode _mode;
    int maxNum;
};
