#ifndef _GR307_H_
#define _GR307_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "usart.h"

#ifdef __cplusplus
extern "C" {
#endif

/** 基本操作 ******************************************************
 * [0] 初始化
 *     * 使用GR307_Init函数
 * [1] 注册
 *     * 使用GR307_Register函数
 *     * 检查返回结果，若不正确可使用GR307_getErrorMsg获取帮助字符串
 * [2] 验证
 *     * 使用GR307_Check函数
 *     * 检查返回结果，若不正确可使用GR307_getErrorMsg获取帮助字符串
 * [3] 删除
 *     * 使用Delete删除特定指纹ID，Clear删除所有
 * 
 ****************************************************************/

// 定义使用串口
#define GR307_UART_ID (huart6)

/**
 * @brief  初始化GR307默认设置
 * @retvl  GR307错误代码
 */
uint8_t GR307_Init(void);

/**
 * @brief  注册一个新的指纹
 * @param  ID[out]:模块内部分配的pageID
 * @retvl  GR307错误代码
 */
uint8_t GR307_Register(uint16_t *ID);

/**
 * @brief  验证指纹库中是否有相同指纹
 * @param  ID[out]:匹配到的指纹pageID
 * @retvl  GR307错误代码
 */
uint8_t GR307_Check(uint16_t *ID);

/**
 * @brief  删除一个指纹
 * @param  ID[in]:指纹保存位置(pageID)
 * @retvl  GR307错误代码
 */
uint8_t GR307_Delete(uint16_t ID);

/**
 * @brief  清空指纹库
 * @retvl  GR307错误代码
 */
uint8_t GR307_Clear(void);

/**
 * @brief  返回模块错误信息
 * @param  errorID[in]:错误ID
 * @retvl  错误信息
 */
const char *GR307_getErrorMsg(uint8_t errorID);


#ifdef __cplusplus
}
#endif
#endif
