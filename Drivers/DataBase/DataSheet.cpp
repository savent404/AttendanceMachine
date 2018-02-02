#include "DataSheet.hpp"

DB_Sheet::DB_Sheet(const enum DB_Sector& sector)
{
    base_addr = getSectorAddr(sector);
    init(base_addr);
}
DB_Sheet::DB_Sheet(const uint32_t* addr)
{
    init(addr);
}
DB_Sheet::DB_Sheet(const DB_Sheet& other)
    : base_addr(other.base_addr)
    , modi(other.modi)
{
    memcpy(password, other.password, 6);
    arry = new std::list<DB_Usr>(other.arry->begin(), other.arry->end());
}
DB_Sheet::~DB_Sheet()
{
    // writeBack();
    delete arry;
}
void DB_Sheet::init(const uint32_t* base)
{
    base_addr = base;
    modi = false;
    if (isInvalid(base) == false) {
        int cnt = getNodeNum(base);
        memcpy(password, (uint8_t*)(base_addr) + 2, 6);
        arry = new std::list<DB_Usr>(cnt);
        const uint8_t* ptr = (const uint8_t*)base;
        ptr += sizeof(uint32_t) * 3;
        for (auto node = arry->begin(); node != arry->end(); node++) {
            node->overWrite(ptr);
            ptr += node->size();
        }
    } else {
        memcpy(password, "000000", 6);
        arry = new std::list<DB_Usr>(0);
    }
}
std::list<DB_Usr>::iterator DB_Sheet::search(const DB_Usr& usr) const
{
    for (auto node = arry->begin(); node != arry->end(); node++) {
        if (*node == usr)
            return node;
    }
    return NULL;
}
std::list<DB_Usr>::iterator DB_Sheet::search(const DB_Base& db) const
{
    for (auto node = arry->begin(); node != arry->end(); node++) {
        if (node->isThere(db))
            return node;
    }
    return NULL;
}
void DB_Sheet::add(const DB_Usr& usr)
{
    arry->push_back(usr);
    modi = true;
}
void DB_Sheet::add(std::list<DB_Usr>::iterator begin, std::list<DB_Usr>::iterator end)
{
    arry->insert(arry->end(), begin, end);
    modi = true;
}
void DB_Sheet::del(std::list<DB_Usr>::iterator node)
{
    bool isExit = false;
    for (auto n = arry->begin(); n != arry->end(); n++) {
        if (n == node) {
            isExit = true;
            break;
        }
    }
    if (!isExit)
        return;
    // Delete data in GR307
    for (int i = 0; i < 5; i++) {
        uint8_t* buf = node->finger[i].data();
        uint16_t id = *buf | (*(buf + 1) << 8);
        GR307_Delete(id);
    }
    std::list<DB_Usr>* new_arry = new std::list<DB_Usr>;
    for (auto n = arry->begin(); n != arry->end(); n++) {
        if (n != node) {
            new_arry->push_back(*n);
        }
    }
    delete arry;
    arry = new_arry;
}
uint32_t DB_Sheet::size()
{
    return arry->size();
}
void DB_Sheet::clear()
{
    if (arry->size())
        modi = true;
    GR307_Clear();
    arry->clear();
}
bool DB_Sheet::isInvalid(const uint32_t* addr)
{
    const uint8_t* a = (const uint8_t*)addr;
    if (*a++ == 0xA5 && *a == 0x5A)
        return false;
    return true;
}
bool DB_Sheet::checkPassword(const uint8_t* pass)
{
    const uint8_t default_pass[] = "000000";
    const uint8_t* my = NULL;

    if (isInvalid(getSectorAddr(Sector_1)))
        my = default_pass;
    else
        my = (uint8_t*)(0x08004000 + 2);
    for (int i = 0; i < 6; i++)
        if (*my++ != *pass++)
            return false;
    return true;
}
bool DB_Sheet::changePassword(const uint8_t* pass)
{
    if (base_addr == getSectorAddr(Sector_1)) {
        uint8_t* my = password;
        for (int i = 0; i < 6; i++) {
            *my++ = *pass++;
        }
        modi = true;
        return true;
    } else {
        return false;
    }
}
bool DB_Sheet::writeBack(bool force)
{

    if (force == false && modi == false) {
        return true;
    }

    uint32_t cnt = sizeof(uint32_t) * 3;
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
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_PGAERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGSERR | FLASH_FLAG_PGPERR | FLASH_FLAG_OPERR);
    //FLASH_WaitForLastOperation(1000);
    if (HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) != HAL_OK) {
        HAL_FLASH_Lock();
        return false;
    }

    // flash write
    uint32_t u32ptr = (uint32_t)base_addr;
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, u32ptr++, 0xA5);
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, u32ptr++, 0x5A);
    for (int i = 0; i < 6; i++)
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, u32ptr++, password[i]);
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, u32ptr, size());
    u32ptr += 4;
    size_t pack_size = arry->begin()->size();
    uint8_t* pack = (uint8_t*)malloc(pack_size);
    for (auto node = arry->begin(); node != arry->end(); node++) {
        node->packet(pack);
        size_t cnt = pack_size;
        uint8_t* buf_ptr = pack;
        while (cnt--) {
            HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, u32ptr++, *buf_ptr++);
        }
    }
    free(pack);
    HAL_FLASH_Lock();
    modi = false;
    return true;
}
const uint32_t* DB_Sheet::getSectorAddr(const enum DB_Sector& sector)
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
uint32_t DB_Sheet::getNodeNum(const uint32_t* baseAddr)
{
    uint32_t num = *(baseAddr + 2);
    if (num > 150)
        return 150;
    return num;
}
uint32_t DB_Sheet::getMaxSize(const uint32_t* baseAddr)
{
    // return 0x4000;
    return 0x1000;
}
const uint32_t* DB_Sheet::getNextSector(const uint32_t* baseAddr)
{
    return baseAddr + 0x4000 / sizeof(uint32_t);
}
