#pragma once

#include "DataBase.hpp"
#include "stm32f4xx_hal.h"
#include <list>

// Sector 1 0x0800 4000 - 0x0800 7FFF 16 Kbytes
// Sector 2 0x0800 8000 - 0x0800 BFFF 16 Kbytes
// Sector 3 0x0800 C000 - 0x0800 FFFF 16 Kbytes
class DB_Sheet {
public:
    enum DB_Sector {
        Sector_1 = 1,
        Sector_2 = 2,
        Sector_3 = 3,
    };
    DB_Sheet(const enum DB_Sector& sector)
    {
        base_addr = getSectorAddr(sector);
        uint32_t cnt = getNodeNum(base_addr);
        const uint8_t* ptr = (const uint8_t*)base_addr;
        ptr += sizeof(uint32_t) * 2;
        for (auto node : *arry) {
            node.overWrite(ptr);
            ptr += node.size();
        }
    }
    DB_Sheet(const uint32_t* addr)
    {
        base_addr = addr;
        uint32_t cnt = getNodeNum(base_addr);
        const uint8_t* ptr = (const uint8_t*)base_addr;
        ptr += sizeof(uint32_t) * 2;
        for (auto node : *arry) {
            node.overWrite(ptr);
            ptr += node.size();
        }
    }
    DB_Sheet(const DB_Sheet& other)
        : base_addr(other.base_addr)
        , modi(other.modi)
    {
        arry = new std::list<DB_Usr>(other.arry->begin(), other.arry->end());
    }
    ~DB_Sheet()
    {
        // writeBack();
        delete arry;
    }
    std::list<DB_Usr>::iterator search(const DB_Usr& usr) const
    {
        for (auto node = arry->begin(); node != arry->end(); node++) {
            if (*node == usr)
                return node;
        }
        return NULL;
    }
    std::list<DB_Usr>::iterator search(const DB_Base& db) const
    {
        for (auto node = arry->begin(); node != arry->end(); node++) {
            if (node->isThere(db))
                return node;
        }
        return NULL;
    }
    void add(const DB_Usr& usr)
    {
        arry->push_back(usr);
        modi = true;
    }
    void add(std::list<DB_Usr>::iterator begin, std::list<DB_Usr>::iterator end)
    {
        arry->insert(arry->end(), begin, end);
        modi = true;
    }
    uint32_t size()
    {
        return arry->size();
    }
    void clear()
    {
        if (arry->size())
            modi = true;
        arry->clear();
    }
    bool writeBack()
    {

        if (modi == false) {
            return true;
        }

        uint32_t cnt = sizeof(uint32_t) * 2;
        uint32_t num = 0;
        auto node = arry->begin();
        for (node = arry->begin(); node != arry->end(); node++) {
            cnt += node->size();
            num += 1;
            if (cnt >= getMaxSize(base_addr)) {
                break;
            }
        }

        if (node != arry->end()) {
            const uint32_t* ptr = base_addr;
            while (getNodeNum(ptr) <= num)
                ptr = getNextSector(ptr);
            if (ptr == NULL) {
                return false;
                ;
            }
            DB_Sheet* pt = new DB_Sheet(ptr);
            pt->writeBack();
            delete pt;
            modi = false;
            return true;
            ;
        }
        // flash erase
        FLASH_EraseInitTypeDef EraseInitStruct;
        uint32_t SectorError;
        EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
        EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
        EraseInitStruct.Sector = ((uint32_t)base_addr & 0xFFFFF) / (0x4000); // 只能是1,2,3
        EraseInitStruct.NbSectors = 1;
        HAL_FLASH_Unlock();
        if (HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) != HAL_OK) {
            return false;
        }
        // flash write
        uint32_t u32ptr = (uint32_t)base_addr;
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t)u32ptr, 0xA55A);
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, u32ptr + 4, size());
        uint32_t u8ptr = (uint32_t)base_addr + 8;
        uint8_t* pack = (uint8_t*)malloc(arry->begin()->size());
        for (auto node = arry->begin(); node != arry->end(); node++) {
            node->packet(pack);
            int cnt = node->size();
            uint8_t* buf_ptr = pack;
            while (cnt--) {
                HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, u8ptr, *buf_ptr++);
                u8ptr += 1;
            }
        }
        HAL_FLASH_Lock();
        modi = false;
        return true;
    }
    static const uint32_t* getSectorAddr(const enum DB_Sector& sector)
    {
        switch (sector) {
        case Sector_1:
            return (const uint32_t*)0x08004000;
        case Sector_2:
            return (const uint32_t*)0x08008000;
        case Sector_3:
            return (const uint32_t*)0x0800C000;
        default:
            return (const uint32_t*)0x00;
        }
    }
    static uint32_t getNodeNum(const uint32_t* baseAddr)
    {
        if (*baseAddr != 0xA55A)
            return 0;
        uint32_t num = *(baseAddr + 1);
        if (num > 150)
            return 150;
        return num;
    }
    static uint32_t getMaxSize(const uint32_t* baseAddr)
    {
        // return 0x4000;
        return 0x1000;
    }
    static const uint32_t* getNextSector(const uint32_t* baseAddr)
    {
        return baseAddr + 0x4000 / sizeof(uint32_t);
    }

private:
    std::list<DB_Usr>* arry;
    const uint32_t* base_addr;
    bool modi;
};
