#include "GR307.h"
/* Encode : GBK */
/** Private var ************************************/
#define GR307_DEFAULT_TIMEOUT 200
#define GR307_WAITFIGURE_TIMEOUT 30000
static const uint8_t start_flag[6] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF};
static UART_HandleTypeDef *uart = &GR307_UART_ID;
/** Private func ***********************************/
/**
 * @brief  У���
 * @param  buff[in]:��ҪУ�������
 * @param  len[in]:У�����ݵĳ���
 * @retvl  16λУ����
 */
static uint16_t sum(uint8_t *buff, size_t len)
{
  uint16_t res = 0;
  while (len--)
    res += *buff++;
  return res;
}

/**
 * @brief  ����һ����(����/����)
 * @param  cmd_id[in]:����ʶ
 * @param  payload[in]:����ָ�����Լ���Ӧ�Ĳ���
 * @param  len[in]:payload�ĳ���
 * @note   |��ͷ(00:2Bytes)|ģ���ַ(02:4Bytes)|����ʶ(06:1Byte)|������(07:2Bytes)|payload(09:len)|У��(09+len:2Bytes)|
 */
static void send(uint8_t cmd_id, uint8_t *payload, size_t len)
{
  uint8_t *send_buffer = malloc(len + 11);
  if (send_buffer == 0)
    return;

  memcpy(send_buffer, start_flag, 6);
  send_buffer[6] = cmd_id;
  send_buffer[7] = (len + 2) >> 8;   // ����У�鳤��
  send_buffer[8] = (len + 2) & 0xFF; // ����У�鳤��
  memcpy(send_buffer + 9, payload, len);

  uint16_t ans = sum(send_buffer + 6, len + 3); // ���ϰ���ʶ�Լ�����
  send_buffer[9 + len] = ans >> 8;
  send_buffer[10 + len] = ans & 0xFF;

  // while (HAL_UART_GetState(uart) != HAL_UART_STATE_READY)
  //   ;
  HAL_UART_Transmit(uart, send_buffer, len + 11, 100);

  free(send_buffer);
}

/**
 * @brief  ����һ��Ӧ���
 * @brief  ������������Ч����
 * @brief  ��Ч���س���
 * @retvl  G307������Ϣ
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

  // ��LED
  buffer[0] = 0x35;
  buffer[1] = 0x01;
  send(0x01, buffer, 2);
  res = checkAck(NULL, 0, GR307_DEFAULT_TIMEOUT);
  if (res != 0)
    return res;

  buffer[0] = 0x33; //ָ����:ע��
  send(0x01, buffer, 1);
  res = checkAck(buffer, 2, GR307_WAITFIGURE_TIMEOUT);

  if (res == 0)
  {
    *ID = buffer[0] << 8 | buffer[1];
  }

  // �ر�LED
  // note: ��LEDʱ�Ѽ����������LED�����򲻻�ִ�е��˴�
  buffer[0] = 0x35;
  buffer[1] = 0x00;
  send(0x01, buffer, 2);

  // ����ע����
  return res;
}

uint8_t GR307_Check(uint16_t *ID)
{
  uint8_t buffer[4];
  uint8_t res;
  uint8_t code = 0;
  // ��LED
  buffer[0] = 0x35;
  buffer[1] = 0x01;
  send(0x01, buffer, 2);
  res = checkAck(NULL, 0, GR307_DEFAULT_TIMEOUT);
  if (res != 0)
    return res;

  buffer[0] = 0x34; //ָ����:��֤
  send(0x01, buffer, 1);
  res = checkAck(buffer, 4, GR307_WAITFIGURE_TIMEOUT);

  if (res == 0)
  {
    *ID = buffer[0] << 8 | buffer[1];
    code = buffer[2] << 8 | buffer[3];
    // ����С��30����Ϊʶ��ƥ��
    if (code < 30)
      res = 0x09;
  }

  // �ر�LED
  // note: ��LEDʱ�Ѽ����������LED�����򲻻�ִ�е��˴�
  buffer[0] = 0x35;
  buffer[1] = 0x00;
  send(0x01, buffer, 2);

  // ������֤���
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
    pt = "ָ��ִ����ϻ�OK";
    break;
  case 0x01:
    pt = "���ݰ����մ���";
    break;
  case 0x02:
    pt = "��������û����ָ";
    break;
  case 0x03:
    pt = "¼��ָ��ͼ��ʧ��";
    break;
  case 0x04:
    pt = "ָ��ͼ��̫�ɡ�̫�����޷���������";
    break;
  case 0x05:
    pt = "ָ��ͼ��̫ʪ��̫�����޷���������";
    break;
  case 0x06:
    pt = "ָ��ͼ��̫�ң��޷���������";
    break;
  case 0x07:
    pt = "ָ��ͼ����������������̫�ٻ����̫С���޷���������";
    break;
  case 0x08:
    pt = "ָ�Ʋ�ƥ��";
    break;
  case 0x09:
    pt = "û������ָ��";
    break;
  case 0x0a:
    pt = "�����ϲ�ʧ��";
    break;
  case 0x0b:
    pt = "����ָ�ƿ��ַ��ų�����Χ";
    break;
  case 0x0c:
    pt = "ָ�ƿ��ȡģ��������Ч";
    break;
  case 0x0d:
    pt = "�ϴ�����ʧ��";
    break;
  case 0x0e:
    pt = "ģ���޷����պ������ݰ�";
    break;
  case 0x0f:
    pt = "�ϴ�ͼ��ʧ��";
    break;
  case 0x10:
    pt = "ɾ��ģ��ʧ��";
    break;
  case 0x11:
    pt = "���ָ�ƿ�ʧ��";
    break;
  case 0x13:
    pt = "�����ȷ";
    break;
  case 0x15:
    pt = "������û����Чԭʼͼ��������ͼ��";
    break;
  case 0x18:
    pt = "��дFLASH����";
    break;
  case 0x19:
    pt = "δ�������";
    break;
  case 0x1a:
    pt = "��Ч�Ĵ�����";
    break;
  case 0x1b:
    pt = "�Ĵ����趨���ݴ���";
    break;
  case 0x1c:
    pt = "���±�ҳ���ƶ�����";
    break;
  case 0x1d:
    pt = "�˿ڲ���ʧ��";
    break;
  case 0x1e:
    pt = "�Զ�ע��(enroll)ʧ��";
    break;
  case 0x1f:
    pt = "ָ�ƿ���";
    break;
  case 0x41:
    pt = "�ڶ���¼��ָʱ������������ָ";
    break;
  case 0x42:
    pt = "�ڶ���¼��ָʱ���ɹ�";
    break;
  case 0x43:
    pt = "�ڶ���¼����ָͼƬ��������������̫��";
    break;
  case 0x44:
    pt = "�ڶ���¼��ָ��ͼ��̫��";
    break;
  case 0x45:
    pt = "ָ�ƿ����ظ�ָ����";
    break;
  case 0xFF:
    pt = "ͨѶ�쳣";
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
