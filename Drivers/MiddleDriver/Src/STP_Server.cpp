#include "STP_Server.hpp"

static STP_ServerBase* CurrentServerHandle = (STP_ServerBase*)NULL;
static STP_ServerBase* STP_GetCurrentServer()
{
    return CurrentServerHandle;
}
static void STP_SetCurrentServer(STP_ServerBase* handle)
{
    CurrentServerHandle = handle;
}
/** Public Function & Class **********************************************/
void STP_ServerCallback()
{
    STP_ServerBase* ptr = STP_GetCurrentServer();
    ptr->Callback();
}

/** Class : Base ********************************************************/
STP_ServerBase::STP_ServerBase() {}
STP_ServerBase::~STP_ServerBase() {}
bool STP_ServerBase::sendMessage(const uint8_t* message, size_t size)
{
    uint8_t ans = 0;
    uint8_t* pt = (uint8_t*)malloc(size + 4);
    const uint8_t start_frame[2] = { 0xA5, 0x5A };
    memcpy(pt, start_frame, 2);
    memcpy(pt + 2, &size, 1);
    memcpy(pt + 3, message, size);
    for (int i = 0; i < size; i++) {
        ans += message[i];
    }
    memcpy(pt + 3 + size, &ans, 1);
    return send(pt, size + 4);
}
bool STP_ServerBase::reciMessage()
{
    STP_SetCurrentServer(this);
    return setReminder();
}

/** Class : RS485 ********************************************************/
STP_ServerRS485::STP_ServerRS485(UART_HandleTypeDef* uart)
{
    handle = uart;
    Recive_Status = waitFrame;
}
STP_ServerRS485::~STP_ServerRS485()
{
    HAL_UART_AbortReceive_IT(handle);
}
bool STP_ServerRS485::send(const uint8_t* buffer, const size_t size)
{
    HAL_GPIO_WritePin(RS485_RL_GPIO_Port, RS485_RL_Pin, GPIO_PIN_SET);
    HAL_UART_AbortReceive_IT(handle);
    return HAL_UART_Transmit(handle, (uint8_t*)buffer, (uint16_t)size, 10) == HAL_OK ? true : false;
}
bool STP_ServerRS485::setReminder()
{
    HAL_GPIO_WritePin(RS485_RL_GPIO_Port, RS485_RL_Pin, GPIO_PIN_RESET);
    return HAL_UART_Receive_IT(handle, recBuffer, 1) == HAL_OK ? true : false;
}
void STP_ServerRS485::Callback()
{
    // 强制的起始帧检查，以防止发送方信息结构错误后不能正常接收下一帧
    if (frameBuffer[0] == 0xA5 && recBuffer[0] == 0x5A) {
        Recive_Status = waitSize;
    } else if (Recive_Status == waitSize) {
        sizeBuffer[0] = recBuffer[0];
        Recive_Status = waitMessage;
        dataPos = 0;
    } else if (Recive_Status == waitMessage) {
        messageBuffer[dataPos] = recBuffer[0];
        if (++dataPos >= sizeBuffer[0]) {
            Recive_Status = waitCheck;
        }
    } else if (Recive_Status == waitCheck) {
        uint8_t ans = recBuffer[0];
        uint8_t _ans = 0;
        for (int i = 0; i < sizeBuffer[0]; i++) {
            _ans += sizeBuffer[i];
        }
        if (ans == _ans) {
            goto GOT_A_MESSAGE;
        }
        Recive_Status = waitFrame;
    }
    goto NORMAL_OPERA;

// 有效接受
GOT_A_MESSAGE:

NORMAL_OPERA:
    // 将当前收到的数据填入帧头检查缓存
    frameBuffer[0] = recBuffer[0];
    // 等待下一个字符
    setReminder();
}
