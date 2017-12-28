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

void OP_Handle();

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
            ans[i] = 'a';
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
    Opera_getNFC(STP_KeyMat& kb);
    virtual bool init();
    virtual bool deinit();
    virtual bool exitCheck();
    virtual bool loop();

private:
    Adafruit_NFCShield_I2C* nfc;
    STP_KeyMat* keymat;
};

class Opera_getFinger : public Opera {
public:
    Opera_getFinger(STP_KeyMat& kb, bool isCheck);
    virtual bool init();
    virtual bool deinit();
    virtual bool exitCheck();
    virtual bool loop();

private:
    STP_KeyMat* keymat;
    bool _isCheck;
};

class Opera_getUsrKey : public Opera {
public:
    enum getMode {
        getPassword = 0,
        getTime = 1,
        getRoomID = 2
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
