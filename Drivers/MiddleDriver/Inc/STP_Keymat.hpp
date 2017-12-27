#pragma once

#include "gpio.h"
#include "main.h"

class STP_KeyMat {
public:
    enum Key {
        KEY_ID_0 = 0,
        KEY_ID_1 = 1,
        KEY_ID_2 = 2,
        KEY_ID_3 = 3,
        KEY_ID_4 = 4,
        KEY_ID_5 = 5,
        KEY_ID_6 = 6,
        KEY_ID_7 = 7,
        KEY_ID_8 = 8,
        KEY_ID_9 = 9,
        KEY_ID_UP = 10,
        KEY_ID_DOWN = 11,
        KEY_ID_RIGHT = 12,
        KEY_ID_LEFT = 13,
        KEY_ID_YES = 14,
        KEY_ID_NO = 15
    };
    STP_KeyMat()
        : subMem(0)
        , ans(0)
    {
        pins[0].port = KB_1_GPIO_Port;
        pins[0].pin = KB_1_Pin;

        pins[1].port = KB_2_GPIO_Port;
        pins[1].pin = KB_2_Pin;

        pins[2].port = KB_3_GPIO_Port;
        pins[2].pin = KB_3_Pin;

        pins[3].port = KB_4_GPIO_Port;
        pins[3].pin = KB_4_Pin;

        pins[4].port = KB_5_GPIO_Port;
        pins[4].pin = KB_5_Pin;

        pins[5].port = KB_6_GPIO_Port;
        pins[5].pin = KB_6_Pin;

        pins[6].port = KB_7_GPIO_Port;
        pins[6].pin = KB_7_Pin;

        pins[7].port = KB_8_GPIO_Port;
        pins[7].pin = KB_8_Pin;

				tick = HAL_GetTick();
    }
    ~STP_KeyMat() {}
    uint16_t scan()
    {
				// per 10ms will check again
				if (HAL_GetTick() - tick < 10) {
						return ans;
				} else {
						tick = HAL_GetTick();
				}
				
        uint8_t res = origin_scan();
        uint8_t deleteKey = subMem & (~res);
        subMem &= res;
        if (deleteKey != 0) {
            ans &= ~convert(deleteKey);
        }
        uint8_t newKey = (~subMem) & res;
        subMem |= res;
        if (newKey != 0) {
            ans |= convert(newKey);
        }
        return ans;
    }
    bool isPress(enum Key id)
    {
        scan();
        if (ans & (1 << (int)id))
            return true;
        return false;
    }

protected:
    class Pin {
    public:
        GPIO_TypeDef* port;
        uint16_t pin;
    };
		uint32_t tick;
    // 保存上次扫描的原始数据
    uint8_t subMem;
    // 每位代表一个当前按下的按钮
    uint16_t ans;
    // ARMCC不支持const static class::Pin 声明时赋值
    // 折衷处理为构造函数初始化
    Pin pins[8];
    /**
     * @brief 切换矩阵IO模式
     * @param type = 0, pin(0~3)为上拉输出, pin(4~7)为下拉输入
     *        type != 0, 情况反之
     */
    void changeMode(uint8_t type)
    {
        GPIO_InitTypeDef gpiox;
        for (int i = 0; i < 4; i++) {
            gpiox.Pin = pins[i].pin;
            gpiox.Mode = type == 0 ? GPIO_MODE_OUTPUT_PP : GPIO_MODE_INPUT;
            gpiox.Pull = type == 0 ? GPIO_PULLUP : GPIO_PULLDOWN;
            HAL_GPIO_Init(pins[i].port, &gpiox);
            HAL_GPIO_WritePin(pins[i].port, pins[i].pin, type == 0 ? GPIO_PIN_SET : GPIO_PIN_RESET);
        }
        for (int i = 4; i < 8; i++) {
            gpiox.Pin = pins[i].pin;
            gpiox.Mode = type != 0 ? GPIO_MODE_OUTPUT_PP : GPIO_MODE_INPUT;
            gpiox.Pull = type != 0 ? GPIO_PULLUP : GPIO_PULLDOWN;
            HAL_GPIO_Init(pins[i].port, &gpiox);
            HAL_GPIO_WritePin(pins[i].port, pins[i].pin, type != 0 ? GPIO_PIN_SET : GPIO_PIN_RESET);
        }
    }

    /**
     * @brief 返回原始数据等待处理
     * @retvl 返回结果
     *        | bit-7 | bit-6 | bit-5 | bit-4 | bit-3 | bit-2 | bit-1 | bit-0 |
     *        | pin-0 | pin-1 | pin-2 | pin-3 | pin-4 | pin-5 | pin-6 | pin-7 |
     */
    uint8_t origin_scan()
    {
        changeMode(0);
        uint8_t front = 0, back = 0;
        for (int i = 4; i < 8; i++) {
            front <<= 1;
            front |= HAL_GPIO_ReadPin(pins[i].port, pins[i].pin) == GPIO_PIN_SET ? 1 : 0;
        }
        if (front == 0) {
            subMem = 0;
            ans = 0;
            return 0;
        }
        changeMode(1);
        for (int i = 0; i < 4; i++) {
            back <<= 1;
            back |= HAL_GPIO_ReadPin(pins[i].port, pins[i].pin) == GPIO_PIN_SET ? 1 : 0;
        }
        if (back == 0) {
            subMem = 0;
            ans = 0;
            return 0;
        }
        return front << 4 | back;
    }
    /**
     * @brief  将原始结果转换为x-y坐标
     * @note   不能同时获取两组按键
     *         x轴为pin-0~pin-3分布的轴
     */
    uint16_t convert(const uint8_t data)
    {
        uint8_t axis_x = data >> 4;
        uint8_t axis_y = data & 0x0F;
        uint8_t x, y;
        for (x = 0; x < 4; x++) {
            if (axis_x & (1 << x))
                break;
        }
        for (y = 0; y < 4; y++) {
            if (axis_y & (1 << y))
                break;
        }

        if (x == 4 || y == 4)
            return 0x0000;
        return 1 << (x * 4 + y);
    }
};
