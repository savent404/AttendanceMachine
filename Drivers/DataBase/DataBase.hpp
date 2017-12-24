#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

class DB_Base {
public:
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
    bool isNull() const
    {
        const uint8_t* ptr = _data;
        for (int i = 0; i < _size; i++) {
            if (*ptr++ != 'a')
                return false;
        }
        return true;
    }
    bool operator==(const DB_Base& other) const
    {
        if (type() != other.type() || size() != other.size() || other.isNull()) {
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

class DB_FingerPrint : public DB_Base {
public:
    DB_FingerPrint(const uint8_t* data = "aa")
        : DB_Base(data, 2, 1)
    {
    }
};
class DB_RFID : public DB_Base {
public:
    DB_RFID(const uint8_t* data = "aaaa")
        : DB_Base(data, 4, 2)
    {
    }
};
class DB_RoomID : public DB_Base {
public:
    DB_RoomID(const uint8_t* data = "aa")
        : DB_Base(data, 2, 3)
    {
    }

    static inline void convert(const uint8_t level, const uint8_t room, uint8_t buf[2])
    {
        buf[0] = level;
        buf[1] = room;
    }
};
class DB_Password : public DB_Base {
public:
    DB_Password(const uint8_t *data = "aaaaaa")
        : DB_Base(data, 6, 4)
    {
    }
};

class DB_Usr {
public:
    DB_Usr() {}
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
