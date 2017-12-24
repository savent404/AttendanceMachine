#include "GR307.h"
/* Encode : GBK */
/** Private var ************************************/
#define GR307_DEFAULT_TIMEOUT 200
#define GR307_WAITFIGURE_TIMEOUT 30000
static const uint8_t start_flag[6] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF};
static UART_HandleTypeDef *uart = &GR307_UART_ID;
/** Private func ***********************************/
/**
 * @brief  校验合
 * @param  buff[in]:需要校验的数据
 * @param  len[in]:校验数据的长度
 * @retvl  16位校验结果
 */
static uint16_t sum(uint8_t *buff, size_t len)
{
  uint16_t res = 0;
  while (len--)
    res += *buff++;
  return res;
}

/**
 * @brief  发送一个包(数据/命令)
 * @param  cmd_id[in]:包标识
 * @param  payload[in]:包括指令码以及相应的参数
 * @param  len[in]:payload的长度
 * @note   |包头(00:2Bytes)|模块地址(02:4Bytes)|包标识(06:1Byte)|包长度(07:2Bytes)|payload(09:len)|校验(09+len:2Bytes)|
 */
static void send(uint8_t cmd_id, uint8_t *payload, size_t len)
{
  uint8_t *send_buffer = malloc(len + 11);
  if (send_buffer == 0)
    return;

  memcpy(send_buffer, start_flag, 6);
  send_buffer[6] = cmd_id;
  send_buffer[7] = (len + 2) >> 8;   // 加上校验长度
  send_buffer[8] = (len + 2) & 0xFF; // 加上校验长度
  memcpy(send_buffer + 9, payload, len);

  uint16_t ans = sum(send_buffer + 6, len + 3); // 加上包标识以及长度
  send_buffer[9 + len] = ans >> 8;
  send_buffer[10 + len] = ans & 0xFF;

  // while (HAL_UART_GetState(uart) != HAL_UART_STATE_READY)
  //   ;
  HAL_UART_Transmit(uart, send_buffer, len + 11, 100);

  free(send_buffer);
}

/**
 * @brief  接收一个应答包
 * @brief  除错误代码的有效负载
 * @brief  有效负载长度
 * @retvl  G307错误信息
 */
uint8_t checkAck(uint8_t *payload, size_t len, uint32_t timeout)
{
  uint8_t *buffer = malloc(sizeof(uint8_t) * (12 + len));
  uint8_t ans;
  uint16_t length;

  buffer[10] = 0xFF;
  // while (HAL_UART_GetState(uart) != HAL_UART_STATE_READY)
  //   ;
  HAL_UART_Receive(uart, buffer, 12 + len, timeout);
  // while (HAL_UART_GetState(uart) != HAL_UART_STATE_READY &&
  //   ((HAL_GetTick() - timeout) < 8e3))
  //   ;
  uint16_t get_sum = buffer[10 + len] << 8 | buffer[11 + len];
  if (get_sum != sum(buffer + 6, 4 + len))
  {
    ans = 0xFF;
    goto _CHECKACK_RETURN;
  }
  length = buffer[7] << 8 | buffer[8];
  if (length > 3)
  {
    if (payload == NULL)
    {
      goto _CHECKACK_RETURN;
    }
    memcpy(payload, buffer + 10, length - 3);
  }
  ans = buffer[10];
_CHECKACK_RETURN:
  free(buffer);
  return ans;
}

uint8_t GR307_Init(void)
{
  return 0x00;
}

uint8_t GR307_Register(uint16_t *ID)
{
  uint8_t buffer[4];
  uint8_t res;

  // 打开LED
  buffer[0] = 0x35;
  buffer[1] = 0x01;
  send(0x01, buffer, 2);
  res = checkAck(NULL, 0, GR307_DEFAULT_TIMEOUT);
  if (res != 0)
    return res;

  buffer[0] = 0x33; //指令码:注册
  send(0x01, buffer, 1);
  res = checkAck(buffer, 2, GR307_WAITFIGURE_TIMEOUT);

  if (res == 0)
  {
    *ID = buffer[0] << 8 | buffer[1];
  }

  // 关闭LED
  // note: 打开LED时已检查结果，若打开LED错误则不会执行到此处
  buffer[0] = 0x35;
  buffer[1] = 0x00;
  send(0x01, buffer, 2);

  // 返回注册结果
  return res;
}

uint8_t GR307_Check(uint16_t *ID)
{
  uint8_t buffer[4];
  uint8_t res;
  uint8_t code = 0;
  // 打开LED
  buffer[0] = 0x35;
  buffer[1] = 0x01;
  send(0x01, buffer, 2);
  res = checkAck(NULL, 0, GR307_DEFAULT_TIMEOUT);
  if (res != 0)
    return res;

  buffer[0] = 0x34; //指令码:验证
  send(0x01, buffer, 1);
  res = checkAck(buffer, 4, GR307_WAITFIGURE_TIMEOUT);

  if (res == 0)
  {
    *ID = buffer[0] << 8 | buffer[1];
    code = buffer[2] << 8 | buffer[3];
    // 分数小于30则认为识别不匹配
    if (code < 30)
      res = 0x09;
  }

  // 关闭LED
  // note: 打开LED时已检查结果，若打开LED错误则不会执行到此处
  buffer[0] = 0x35;
  buffer[1] = 0x00;
  send(0x01, buffer, 2);

  // 返回验证结果
  return res;
}

uint8_t GR307_Delete(uint16_t ID)
{
  uint8_t buffer[5];
  buffer[0] = 0x0c;
  buffer[1] = ID >> 8;
  buffer[2] = ID & 0xFF;
  buffer[3] = 0x00;
  buffer[4] = 0x01;
  send(0x01, buffer, 5);
  return checkAck(NULL, 0, GR307_DEFAULT_TIMEOUT);
}

uint8_t GR307_Clear(void)
{
  uint8_t buffer[1] = {0x0D};
  send(0x01, buffer, 1);
  return checkAck(NULL, 0, GR307_DEFAULT_TIMEOUT);
}

const char *GR307_getErrorMsg(uint8_t errorID)
{
  const char *pt = "";
  switch (errorID)
  {
  case 0x00:
    pt = "指令执行完毕或OK";
    break;
  case 0x01:
    pt = "数据包接收错误";
    break;
  case 0x02:
    pt = "传感器上没有手指";
    break;
  case 0x03:
    pt = "录入指纹图像失败";
    break;
  case 0x04:
    pt = "指纹图像太干、太淡，无法生成特征";
    break;
  case 0x05:
    pt = "指纹图像太湿、太糊，无法生成特征";
    break;
  case 0x06:
    pt = "指纹图像太乱，无法生成特征";
    break;
  case 0x07:
    pt = "指纹图像正常，但特征点太少或面积太小，无法生成特征";
    break;
  case 0x08:
    pt = "指纹不匹配";
    break;
  case 0x09:
    pt = "没搜索到指纹";
    break;
  case 0x0a:
    pt = "特征合并失败";
    break;
  case 0x0b:
    pt = "访问指纹库地址序号超出范围";
    break;
  case 0x0c:
    pt = "指纹库读取模板出错或无效";
    break;
  case 0x0d:
    pt = "上传特征失败";
    break;
  case 0x0e:
    pt = "模块无法接收后续数据包";
    break;
  case 0x0f:
    pt = "上传图像失败";
    break;
  case 0x10:
    pt = "删除模板失败";
    break;
  case 0x11:
    pt = "清空指纹库失败";
    break;
  case 0x13:
    pt = "口令不正确";
    break;
  case 0x15:
    pt = "缓存区没有有效原始图而不生成图像";
    break;
  case 0x18:
    pt = "读写FLASH出错";
    break;
  case 0x19:
    pt = "未定义错误";
    break;
  case 0x1a:
    pt = "无效寄存器号";
    break;
  case 0x1b:
    pt = "寄存器设定内容错误";
    break;
  case 0x1c:
    pt = "记事本页码制定错误";
    break;
  case 0x1d:
    pt = "端口操作失败";
    break;
  case 0x1e:
    pt = "自动注册(enroll)失败";
    break;
  case 0x1f:
    pt = "指纹库满";
    break;
  case 0x41:
    pt = "第二次录手指时传感器上无手指";
    break;
  case 0x42:
    pt = "第二次录手指时不成功";
    break;
  case 0x43:
    pt = "第二次录入手指图片正常，但特征点太少";
    break;
  case 0x44:
    pt = "第二次录入指纹图像太乱";
    break;
  case 0x45:
    pt = "指纹库有重复指纹了";
    break;
  case 0xFF:
    pt = "通讯异常";
    break;
  default:
  {
    char *buffer = malloc(sizeof(char) * 20);
    sprintf(buffer, "GR307 Error Code: %02x", errorID);
    pt = (const char *)buffer;
  }
  }
  return pt;
}
