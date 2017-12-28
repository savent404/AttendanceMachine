
# SlaveMessage
## SequencDiagram::User-Master-Slave 
```mermaid
sequenceDiagram
  User->>Master: 设定时间
  User->>Master: 输入管理者密码
  User->>Master: 选择模式
  loop 注册
    User-->>Master: [ROOMID] + [其他信息]
    alt 可直接添加
      Master-->>Master: 写Flash
    else 需root权限替换已存在信息
      User-->>Master: root密码
      Master-->>Master: 写Flash
    end
  end 

  loop 登入
    User-->>Master: [其他信息]
    alt 存在该信息
      Master-->>Slave: [MODE] + [RoomID]
    else 不存在该信息
      Master-->>User: Error message
    end
  end

  opt 紧急停止
    User-->>Master: 按下紧急按钮
    Master-->>Slave: CMD:Security
    loop 报警
      Master-->>Master: 蜂鸣器响、显示错误信息
    end
    User-->>Master: 按下'NO'按键
    Master-->>Master:触发软件复位，并跳过时间设定阶段
  end

  opt 从机异常
    Slave-->>Master: CMD(0x41~0x43)
    loop 报警
      Master-->>Master: 蜂鸣器响、显示错误信息
    end
    User-->>Master: 按下'NO'按键
    Master-->>Master:触发软件复位，并跳过时间设定阶段
  end
```
***
## Message::Frame
当Data Length为0时，不存在Data且Check也为0

```mermaid
gantt
dateFormat  SSS
title Message Frame

section Master
StartFrame(0xEF)  :done, 000, 001
CMD(1 Byte) :done, 001, 002
Data Length(1 Byte) : done, 002, 003
Data(Length Bytes) : done, 003, 004
Check(1 Byte) :done, 004, 005
```

## Message::CMD
name | value | description
---- | ----- | -----------
`ID_FINGER`| 0x10|`指纹模式`-发送ID
`ID_RFID`|0x11|`刷卡模式`-发送ID
`ID_KEY`|0x12|`按键模式`-发送ID
`UP_Start`|0x20|`操作模式`-按下`UP`
`DOWN_Start`|0x21|`操作模式`-按下`DOWN`
`RIGHT_Start`|0x22|`操作模式`-按下`RIGHT`
`LEFT_Start`|0x23|`操作模式`-按下`LEFT`
`UP_Stop`|0x30|`操作模式`-松开`UP`
`DOWN_Stop`|0x31|`操作模式`-松开`DOWN`
`RIGHT_Stop`|0x32|`操作模式`-松开`RIGHT`
`LEFT_Stop`|0x33|`操作模式`-松开`LEFT`
`SECURITY`|0x40|紧急制动
`ERROR_UNKNOW`|0x41|从机发送的`未知错误`
`ERROR_BREAKIN`|0x42|从机发送的`闯入警告`
`ERROR_LIMIT`|0x43|从机发送的`触发极限`
`ERROR_CHAT`|0x44|从机发送的`通讯错误`
