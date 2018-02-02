#pragma once

#include "DataBase.hpp"
#include "GR307.h"
#include "stm32f4xx_hal.h"
#include <list>

// FLASH 使用范围
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
    DB_Sheet(const enum DB_Sector& sector);
    DB_Sheet(const uint32_t* addr);
    DB_Sheet(const DB_Sheet& other);
    void init(const uint32_t* base);
    ~DB_Sheet();

    /**
     * @brief  检查sheet是否含有该用户
     * @retvl  找到返回该用户在list中的节点地址，否则返回NULL
     */
    std::list<DB_Usr>::iterator search(const DB_Usr& usr) const;

    /**
     * @brief  检查sheet是否有用户含有某项数据(密码、房间号、指纹等)
     * @retvl  找到返回该用户在list中的节点地址，否则返回NULL
     */
    std::list<DB_Usr>::iterator search(const DB_Base& db) const;

    /**
     * @brief  添加一个用户
     * @note   [不安全] 未检查该用户的房间ID已存在,需调用时额外有代码进行检查
     */
    void add(const DB_Usr& usr);

    /**
     * @brief  添加多个用户
     * @note   [不安全] 未检查该用户的房间ID已存在,需调用时额外有代码进行检查
     * @note   主要用于sheet在writeBack时将不能保存的用户放入下一个sector
     */
    void add(std::list<DB_Usr>::iterator begin, std::list<DB_Usr>::iterator end);

    /**
     * @brief  删除一个用户
     * @param  node 该用户在sheet的list中的位置
     * @note   由于使用数据类型为链表，每次删除操作会重写整个sheet内容
     */
    void del(std::list<DB_Usr>::iterator node);
    uint32_t size();
    void clear();
    
    /**
     * @brief  检查标志位，看flash中是否是有效数据结构
     * @retvl  true-无效结构
     */
    static bool isInvalid(const uint32_t* addr);

    /**
     * @brief  验证管理员密码，该密码始终保存在0x08004000 + 2的地址上
     */
    static bool checkPassword(const uint8_t* pass);

    /**
     * @brief  将新密码写入结构体
     * @note   将密码立即写入flash是危险的操作
     * @note   管理员密码只对sector1有效
     */
    bool changePassword(const uint8_t* pass);

    /**
     * @brief  将数据结构写回flash
     * @note   由于search方法可能导致隐形的数据改动(而这是允许的), modi不会标识当前数据的改动
     *         所以提供输入参数force为程序员提供强制写回的方法
     * @retvl  写入是否成功
     */
    bool writeBack(bool force = false);

    /**
     * @brief  将枚举量转化为真实flash地址
     */
    static const uint32_t* getSectorAddr(const enum DB_Sector& sector);

    /**
     * @brief  基于基地址获取当前sector容纳的用户数据条数
     * @note   未初始化的sector将返回结果:0
     */
    static uint32_t getNodeNum(const uint32_t* baseAddr);

    /**
     * @brief  返回每个sector最大容量
     * @note   测试起见,0x4000的sector只使用0x1000Bytes
     */
    static uint32_t getMaxSize(const uint32_t* baseAddr);

    /**
     * @brief  基于目前sector地址返回下一个sector地址
     * @note   该函数的前提为Sheet使用sector是地址连续的
     */
    static const uint32_t* getNextSector(const uint32_t* baseAddr);

private:
    std::list<DB_Usr>* arry;
    const uint32_t* base_addr;
    uint8_t password[6];
    bool modi; // 标志用户信息是否改变(#1 search方法可能导致隐形的数据改动,该标志不会改变)
};
