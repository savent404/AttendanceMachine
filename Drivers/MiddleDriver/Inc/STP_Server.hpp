#pragma once

#include "can.h"
#include "usart.h"

#include <stdlib.h>
#include <string.h>

#define START_FRAME ((uint8_t)0xEF)

extern "C" void STP_ServerCallback();
class STP_ServerBase {
public:
    enum CMD {
        CMD_ID_FINGER = 0x10, // 指纹模式，发送ID
        CMD_ID_RFID = 0x11, // RFID模式，发送ID
        CMD_ID_KEY = 0x12, // KEY模式，发送ID

        CMD_UP_Start = 0x20,
        CMD_UP_Stop = 0x30,
        CMD_DOWN_Start = 0x21,
        CMD_DOWN_Stop = 0x31,
        CMD_RIGHT_Start = 0x22,
        CMD_RIGHT_Stop = 0x32,
        CMD_LEFT_Start = 0x23,
        CMD_LEFT_Stop = 0x33,

        CMD_SECURITY = 0x40, // 主机紧急停止按钮
        CMD_ERROR_UNKNOW = 0x41, //从机未知错误
        CMD_ERROR_BREAKIN = 0x42, //未授权进入
        CMD_ERROR_LIMIT = 0x43 // 极限位置
        CMD_ERROR_CHAT
        = 0x44
    };
    STP_ServerBase();
    virtual ~STP_ServerBase();

    /**
     * @brief  发送消息
     * @note   size不大于4个字节
     */
    bool sendMessage(enum CMD cmd, const uint8_t* message = NULL, size_t size = 0);
    /**
     * @brief  启动中断接受消息
     * @note   接收到有效消息后会在当前中断优先级下调用Callback函数响应中断
     */
    bool reciMessage();
    /**
     * @brief  中断响应函数
     * @note   由于处于中断阶段，尽量设置处理标志而不是直接调用高负载函数
     */
    virtual void Callback() = 0;

private:
    /**
     * @brief  底层IO,发送打包好的消息
     */
    virtual bool send(const uint8_t* buffer, size_t size) = 0;

    /**
     * @brief  底层IO,使能接收中断
     */
    virtual bool setReminder() = 0;
};

/**
 * @brief  由于是字符设备，中断只接收一个字节
 *         打包的消息通过多次中断接收
 */
class STP_ServerRS485 : public STP_ServerBase {
public:
    STP_ServerRS485(UART_HandleTypeDef* uart);
    virtual ~STP_ServerRS485();
    virtual void Callback();

private:
    UART_HandleTypeDef* handle;
    uint8_t recBuffer[1];
    uint8_t CMDBuffer[1];
    uint8_t messageBuffer[4];
    uint8_t sizeBuffer[1];
    uint8_t dataPos;
    enum {
        waitFrame = 0,
        waitCMD = 1,
        waitSize = 2,
        waitMessage = 3,
        waitCheck = 4
    } Recive_Status;

    virtual bool send(const uint8_t* buffer, size_t size);
    virtual bool setReminder();
};
