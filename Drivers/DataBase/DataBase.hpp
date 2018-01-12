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
    DB_Base(const uint8_t* data, const size_t size, const uint32_t type)
    {
        _size = size;
        _data = (uint8_t*)malloc(_size);
        _type = type;
        memcpy(_data, data, _size);
    }
    DB_Base(const DB_Base& other)
    {
        _size = other.size();
        _data = (uint8_t*)malloc(_size);
        _type = other.type();
        memcpy(_data, other.data(), _size);
    }
    ~DB_Base()
    {
        free(_data);
    }
    inline size_t size() const
    {
        return _size;
    }
    inline uint8_t* data() const
    {
        return _data;
    }
    inline uint32_t type() const
    {
        return _type;
    }
    /**
     * @brief  重写数据内容
     */
    inline void overWrite(const uint8_t* data) const
    {
        memcpy(_data, data, _size);
    }
    inline void overWrite(const DB_Base& data) const
    {
        if (size() != data.size() || type() != data.type() || data.isNull())
            return;
        memcpy(_data, data.data(), _size);
    }
    /**
     * @brief  判断数据有效性
     * @retvl  如果无效返回true
     */
    bool isNull() const
    {
        const uint8_t* ptr = _data;
        for (int i = 0; i < _size; i++) {
            if (*ptr++ != 0xFF)
                return false;
        }
        return true;
    }

    /**
     * @brief  重载== 比较两个Base是否相同
     * @note   不光比较数据内容, 数据类型以及是否是有效数据也会检查
     */
    bool operator==(const DB_Base& other) const
    {
        if (type() != other.type() || size() != other.size() /*|| other.isNull()*/) {
            return false;
        }

        const uint8_t* aptr = data();
        const uint8_t* bptr = other.data();
        for (size_t i = 0; i < size(); i++) {
            if (*aptr++ != *bptr++) {
                return false;
            }
        }
        return true;
    }

    /**
     * @brief  重载== 只比较数据内容的版本
     */
    bool operator==(const uint8_t* addr) const
    {
        const uint8_t* aptr = data();
        const uint8_t* bptr = addr;
        for (size_t i = 0; i < size(); i++) {
            if (*aptr++ != *bptr++) {
                return false;
            }
        }
        return true;
    }

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
    DB_FingerPrint(const uint8_t* data = "\xFF\xFF")
        : DB_Base(data, 2, 1)
    {
    }
};

/**
 * @brief  RFID数据占4字节，数据类型为@2
 */
class DB_RFID : public DB_Base {
public:
    DB_RFID(const uint8_t* data = "\xFF\xFF\xFF\xFF")
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
    DB_RoomID(const uint8_t* data = "\xFF\xFF\xFF\xFF")
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
    DB_Password(const uint8_t* data = "\xFF\xFF\xFF\xFF\xFF\xFF")
        : DB_Base(data, 6, 4)
    {
    }
};

class DB_Usr {
public:
    DB_Usr()
    {
    }
    /**
     * @brief  通过打包好数据的数据指针初始化用户数据
     */
    DB_Usr(const uint8_t* addr)
    {
        const uint8_t* ptr = addr;
        for (int i = 0; i < 5; i++) {
            finger[i].overWrite(ptr);
            ptr += finger[i].size();
        }
        rfid.overWrite(ptr);
        ptr += rfid.size();
        rid.overWrite(ptr);
        ptr += rid.size();
        pid.overWrite(ptr);
    }
    DB_Usr(const DB_Usr& usr)
    {
        for (int i = 0; i < 5; i++) {
            finger[i].overWrite(usr.finger[i]);
        }
        rfid.overWrite(usr.rfid);
        rid.overWrite(usr.rid);
        pid.overWrite(usr.pid);
    }
    DB_Usr(const DB_FingerPrint fingers[5], const DB_RFID& rf, const DB_RoomID& r, const DB_Password& p)
    {
        for (int i = 0; i < 5; i++) {
            finger[i].overWrite(fingers[i]);
        }
        rfid.overWrite(rf);
        rid.overWrite(r);
        pid.overWrite(p);
    }

    /**
     * @brief  通过打包好的数据的数据指针重写用户数据
     */
    void overWrite(const uint8_t* data) const
    {
        const uint8_t* ptr = data;
        for (int i = 0; i < 5; i++) {
            finger[i].overWrite(ptr);
            ptr += finger[i].size();
        }
        rfid.overWrite(ptr);
        ptr += rfid.size();
        rid.overWrite(ptr);
        ptr += rid.size();
        pid.overWrite(ptr);
    }
    size_t size() const
    {
        size_t cnt = 0;
        for (int i = 0; i < 5; i++) {
            cnt += finger[i].size();
        }
        cnt += rfid.size();
        cnt += rid.size();
        cnt += pid.size();
        return cnt;
    }

    /**
     * @brief  将用户数据打包到数据指针地址中
     */
    void packet(uint8_t* addr) const
    {
        uint8_t* ptr = addr;
        for (int i = 0; i < 5; i++) {
            memcpy(ptr, finger[i].data(), finger[i].size());
            ptr += finger[i].size();
        }
        memcpy(ptr, rfid.data(), rfid.size());
        ptr += rfid.size();
        memcpy(ptr, rid.data(), rid.size());
        ptr += rid.size();
        memcpy(ptr, pid.data(), pid.size());
    }

    /**
     * @brief  比较一个数据基类，查看该用户是否拥有相同数据项
     */
    bool isThere(const DB_Base& base) const
    {
        for (int i = 0; i < 5; i++) {
            if (finger[i] == base) {
                return true;
            }
        }
        if (rfid == base) {
            return true;
        }
        if (rid == base) {
            return true;
        }
        if (pid == base) {
            return true;
        }
        return false;
    }

    /**
     * @brief  判断该用户是否为无效用户(数据都无效)
     */
    bool isAllNull() const
    {
        for (int i = 0; i < 5; i++) {
            if (!finger[i].isNull())
                return false;
        }
        if (!rfid.isNull())
            return false;
        if (!rid.isNull())
            return false;
        if (!pid.isNull())
            return false;
        return true;
    }
    bool operator==(const DB_Usr& usr) const
    {
        for (int i = 0; i < 5; i++) {
            if (!(finger[i] == usr.finger[i]))
                return false;
        }
        if (!(rfid == usr.rfid)) {
            return false;
        }
        if (!(rid == usr.rid)) {
            return false;
        }
        if (!(pid == usr.pid)) {
            return false;
        }
        return true;
    }

    // private:
    DB_FingerPrint finger[5];
    DB_RFID rfid;
    DB_RoomID rid;
    DB_Password pid;
};
