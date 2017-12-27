# NFC::GR307
***
# 文件说明
驱动文件: `Adafruit_NFCShield_I2C.h`, `Adafruit_NFCShield_I2C.cpp`

# 移植说明
[原代码库](https://github.com/adafruit/Adafruit_NFCShield_I2C)是用于`ARDUINO`的I2C驱动库
这里将底层操作函数修改后直接使用
由于是C++代码，索性将整个工程以及所有驱动都写成C++驱动，也是因为使用C++方便封装用户信息(GR307驱动这时已完成)

# 使用说明
参见[原代码库](https://github.com/adafruit/Adafruit_NFCShield_I2C)的[EXAMPLES](https://github.com/adafruit/Adafruit_NFCShield_I2C/tree/master/examples)
