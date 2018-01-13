#include "STP_Server.hpp"

extern void rec_callback(enum STP_ServerBase::CMD cmd, const uint8_t* buffer, size_t size);

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
bool STP_ServerBase::sendMessage(enum CMD cmd, const uint8_t* message, size_t size)
{
    uint8_t ans = 0;
    uint8_t* buffer = (uint8_t*)malloc(sizeof(uint8_t) * (size + 5));
    buffer[0] = START_FRAME0;
    buffer[1] = START_FRAME1;
    buffer[2] = (uint8_t)cmd;
    buffer[3] = (uint8_t)size;
    ans += (uint8_t)cmd;
    ans += (uint8_t)size;
    for (int i = 0; i < size; i++) {
        buffer[4 + i] = *message;
        ans += *message++;
    }
    buffer[4 + size] = ans;
    bool res = send(buffer, size + 5);
    free(buffer);
    return res;
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
    if (frameBuffer[0] == START_FRAME0 && *recBuffer == START_FRAME1) {
        Recive_Status = waitCMD;
    } else if (Recive_Status == waitCMD) {
        CMDBuffer[0] = recBuffer[0];
        Recive_Status = waitSize;
    } else if (Recive_Status == waitSize) {
        sizeBuffer[0] = recBuffer[0];
        dataPos = 0;
        if (sizeBuffer[0] != 0) {
            Recive_Status = waitMessage;
        } else {
            Recive_Status = waitCheck;
        }
    } else if (Recive_Status == waitMessage) {
        messageBuffer[dataPos] = recBuffer[0];
        if (++dataPos >= sizeBuffer[0]) {
            Recive_Status = waitCheck;
        }
    } else if (Recive_Status == waitCheck) {
        uint8_t ans = recBuffer[0];
        uint8_t _ans = 0;
        for (int i = 0; i < sizeBuffer[0]; i++) {
            _ans += messageBuffer[i];
        }
        _ans += CMDBuffer[0];
        _ans += sizeBuffer[0];
        if (ans == _ans) {
            goto GOT_A_MESSAGE;
        }
        Recive_Status = waitFrame;
    }
    frameBuffer[0] = *recBuffer;
    goto NORMAL_OPERA;

// 有效接受
GOT_A_MESSAGE:
    rec_callback((enum CMD)CMDBuffer[0], messageBuffer, sizeBuffer[0]);
NORMAL_OPERA:
    // 等待下一个字符
    setReminder();
}
