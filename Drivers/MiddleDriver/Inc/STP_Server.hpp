#pragma once

#include "can.h"
#include "usart.h"

#include <stdlib.h>
#include <string.h>

extern "C" void STP_ServerCallback();
class STP_ServerBase {
public:
    STP_ServerBase();
    ~STP_ServerBase();

    bool sendMessage(const uint8_t* message, size_t size);
    bool reciMessage();
    virtual void Callback() = 0;
private:
    virtual bool send(const uint8_t* buffer, size_t size) = 0;
    virtual bool setReminder() = 0;
};

class STP_ServerRS485 : public STP_ServerBase {
public:
    STP_ServerRS485(UART_HandleTypeDef* uart);
    ~STP_ServerRS485();
    virtual void Callback();
private:
    UART_HandleTypeDef* handle;
    uint8_t recBuffer[1];
    uint8_t messageBuffer[8];
    uint8_t frameBuffer[1];
    uint8_t sizeBuffer[1];
    uint8_t dataPos;
    enum {
        waitFrame = 0,
        waitSize = 1,
        waitMessage = 2,
        waitCheck = 3
    } Recive_Status;
    
    virtual bool send(const uint8_t* buffer, size_t size);
    virtual bool setReminder();
};
