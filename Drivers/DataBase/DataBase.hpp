#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

class DB_Base {
public:
    /**
     * @brief 创建一个基础数据类型
     * @param data 指向需要初始化数据的内容
     * @param size 数据大小
     * @param type 数据类型
     */
    DB_Base(const uint8_t* data, const size_t size, const uint32_t type);
    DB_Base(const DB_Base& other);
    ~DB_Base();
    size_t size() const;
    uint8_t* data() const;
    uint32_t type() const;
    /**
     * @brief  重写数据内容
     */
    void overWrite(const uint8_t* data) const;
    void overWrite(const DB_Base& data) const;
    /**
     * @brief  判断数据有效性
     * @retvl  如果无效返回true
     */
    bool isNull() const;

    /**
     * @brief  重载== 比较两个Base是否相同
     * @note   不光比较数据内容, 数据类型以及是否是有效数据也会检查
     */
    bool operator==(const DB_Base& other) const;

    /**
     * @brief  重载== 只比较数据内容的版本
     */
    bool operator==(const uint8_t* addr) const;

private:
    uint8_t* _data;
    size_t _size;
    uint32_t _type;
};

/**
 * @brief  指纹数据占2Bytes,数据类型为@1
 */
class DB_FingerPrint : public DB_Base {
public:
    DB_FingerPrint(const uint8_t* data = (uint8_t*)"\xFF\xFF")
        : DB_Base(data, 2, 1)
    {
    }
};

/**
 * @brief  RFID数据占4字节，数据类型为@2
 */
class DB_RFID : public DB_Base {
public:
    DB_RFID(const uint8_t* data = (uint8_t*)"\xFF\xFF\xFF\xFF")
        : DB_Base(data, 4, 2)
    {
    }
};

/**
 * @brief  房间ID数据占4字节，数据类型为@3
 * @note   数据以ASCII方式保存
 */
class DB_RoomID : public DB_Base {
public:
    DB_RoomID(const uint8_t* data = (uint8_t*)"\xFF\xFF\xFF\xFF")
        : DB_Base(data, 4, 3)
    {
    }
};

/**
 * @brief  密码数据占6字节，数据类型为@4
 * @note   密码以明文保存，为提供额外保护需自行添加convert函数进行加密
 */
class DB_Password : public DB_Base {
public:
    DB_Password(const uint8_t* data = (uint8_t*)"\xFF\xFF\xFF\xFF\xFF\xFF")
        : DB_Base(data, 6, 4)
    {
    }
};

class DB_Usr {
public:
    DB_Usr();
    /**
     * @brief  通过打包好数据的数据指针初始化用户数据
     */
    DB_Usr(const uint8_t* addr);
    DB_Usr(const DB_Usr& usr);
    DB_Usr(const DB_FingerPrint fingers[5], const DB_RFID& rf, const DB_RoomID& r, const DB_Password& p);

    /**
     * @brief  通过打包好的数据的数据指针重写用户数据
     */
    void overWrite(const uint8_t* data) const;
    size_t size() const;

    /**
     * @brief  将用户数据打包到数据指针地址中
     */
    void packet(uint8_t* addr) const;

    /**
     * @brief  比较一个数据基类，查看该用户是否拥有相同数据项
     */
    bool isThere(const DB_Base& base) const;

    /**
     * @brief  判断该用户是否为无效用户(数据都无效)
     */
    bool isAllNull() const;
    bool operator==(const DB_Usr& usr) const;

    // private:
    DB_FingerPrint finger[5];
    DB_RFID rfid;
    DB_RoomID rid;
    DB_Password pid;
};
