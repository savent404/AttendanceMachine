# DataBase

## class::DB_Base
数据基类,封装一个固定大小的数据类型

## class::DB_Usr
将用户数据包含在一个集合里面:5个指纹(finger[5]),1个RFID(rfid),一个用户密码(pid)和一个房间号(rid).扩展了数据封装的操作

## class::DB_Sheet
STM32 Flash 每次的删除都需要删除一个整扇区, DB_Sheet以扇区为管理对象管理用户组