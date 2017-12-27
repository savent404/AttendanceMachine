# FingerPrint::GR307
***
# 文件说明
驱动文件为`GR307.h`,`GR307.c`

# 移植说明
参考该模块手册编程，精简了API到5个
因为使用的STM32 HAL库函数, 调用串口会用到一个`usart handle`结构体(定义在`huart.h`中)，更换串口时记得将`GR307_UART_ID`的值修改

# 使用说明
参见GR307.h函数说明
