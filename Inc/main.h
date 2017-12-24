/**
  ******************************************************************************
  * File Name          : main.hpp
  * Description        : This file contains the common defines of the application
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2017 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H
  /* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Includes ------------------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/

#define KB_1_Pin GPIO_PIN_4
#define KB_1_GPIO_Port GPIOA
#define KB_2_Pin GPIO_PIN_5
#define KB_2_GPIO_Port GPIOA
#define KB_3_Pin GPIO_PIN_6
#define KB_3_GPIO_Port GPIOA
#define KB_4_Pin GPIO_PIN_7
#define KB_4_GPIO_Port GPIOA
#define KB_5_Pin GPIO_PIN_4
#define KB_5_GPIO_Port GPIOC
#define KB_6_Pin GPIO_PIN_5
#define KB_6_GPIO_Port GPIOC
#define KB_7_Pin GPIO_PIN_0
#define KB_7_GPIO_Port GPIOB
#define KB_8_Pin GPIO_PIN_1
#define KB_8_GPIO_Port GPIOB
#define PN532_SCL_Pin GPIO_PIN_10
#define PN532_SCL_GPIO_Port GPIOB
#define PN532_SDA_Pin GPIO_PIN_11
#define PN532_SDA_GPIO_Port GPIOB
#define W25Q_SPI2_NSS_Pin GPIO_PIN_12
#define W25Q_SPI2_NSS_GPIO_Port GPIOB
#define W25Q_SPI2_SCK_Pin GPIO_PIN_13
#define W25Q_SPI2_SCK_GPIO_Port GPIOB
#define W25Q_SPI2_MISO_Pin GPIO_PIN_14
#define W25Q_SPI2_MISO_GPIO_Port GPIOB
#define W25Q_SPI2_MOSI_Pin GPIO_PIN_15
#define W25Q_SPI2_MOSI_GPIO_Port GPIOB
#define LCD_USART3_TX_Pin GPIO_PIN_8
#define LCD_USART3_TX_GPIO_Port GPIOD
#define LCD_USART3_RX_Pin GPIO_PIN_9
#define LCD_USART3_RX_GPIO_Port GPIOD
#define LCD_BUSY_Pin GPIO_PIN_10
#define LCD_BUSY_GPIO_Port GPIOD
#define R307_USART6_TX_Pin GPIO_PIN_6
#define R307_USART6_TX_GPIO_Port GPIOC
#define R307_USART6_RX_Pin GPIO_PIN_7
#define R307_USART6_RX_GPIO_Port GPIOC
#define R307_INT_Pin GPIO_PIN_8
#define R307_INT_GPIO_Port GPIOC
#define BELL_Pin GPIO_PIN_9
#define BELL_GPIO_Port GPIOC
#define PN532_RDY_Pin GPIO_PIN_3
#define PN532_RDY_GPIO_Port GPIOB
#define RS485_RL_Pin GPIO_PIN_5
#define RS485_RL_GPIO_Port GPIOB
#define RS485_USART1_TX_Pin GPIO_PIN_6
#define RS485_USART1_TX_GPIO_Port GPIOB
#define RS485_USART1_RX_Pin GPIO_PIN_7
#define RS485_USART1_RX_GPIO_Port GPIOB

/* ########################## Assert Selection ############################## */
/**
  * @brief Uncomment the line below to expanse the "assert_param" macro in the 
  *        HAL drivers code
  */
/* #define USE_FULL_ASSERT    1U */

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
 extern "C" {
#endif
void _Error_Handler(char *, int);

#define Error_Handler() _Error_Handler(__FILE__, __LINE__)
#ifdef __cplusplus
}
#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

#endif /* __MAIN_H */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
