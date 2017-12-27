# 流程图

# 主机上电流程
```flow
st=>start: 开始
op1=>operation: 设定时间
op2=>operation: 开始画面
op3=>operation: 选择模式(RFID/指纹/键入)
io1=>inputoutput: "0+down"
io2=>inputoutput: "管理员密码"
io3=>inputoutput: "键入数字"
c1=>condition: "==0"
c2=>condition: "==1"
c3=>condition: "==2"

sub10=>subroutine: "指纹" 选择模式(登入/注册)
sub11=>subroutine: "RFID" 选择模式(登入/注册)
sub12=>subroutine: "键入" 选择模式(登入/注册)

st->op1->op2->io1->io2->op3
op3->io3->c1(no, right)->c2(no, right)->c3(no)->op3
c1(yes)->sub10
c2(yes)->sub11
c3(yes)->sub12
```
***
# 指纹模式(Mode:Finger)子流程(RFID(Mode:NFC)流程类似)
```flow
sub=>subroutine: "指纹" 选择模式(登入/注册)
io1=>inputoutput: "登入/注册" 模式输入
io2=>inputoutput: "按下指纹"
io3=>inputoutput: "输入房间号"
io4=>inputoutput: "按下指纹(5次)"
io5=>inputoutput: "管理员密码"
c1=>condition: "==0"
c2=>condition: "==1"
c3=>condition: 数据存在?
c4=>condition: 房间ID已存在?
c5=>condition: 管理员密码正确?
op1=>operation: 等待指纹按下
op2=>operation: 对比数据库
op3=>operation: 正在操作
op4=>operation: 等待输入房间ID
op5=>operation: 对比数据库
op6=>operation: 写入数据库

sub->io1->c1(no)->c2(no)->io1
c1(yes)->op1->io2->op2->op3->op1
c2(yes)->op4->io3->io4->op5->c4(no)->op6
c4(yes, right)->io5->c5(no)->io1
c5(yes)->op6(right)->io1
```
***
# 键入模式(Mode:Key)子流程
```flow
sub=>subroutine: "键入" 选择模式(登入/注册)
io1=>inputoutput: "登入/注册" 模式输入
io2=>inputoutput: "输入房间号"
io3=>inputoutput: "输入密码"
io4=>inputoutput: "输入确定"
io5=>inputoutput: "输入房间号"
io6=>inputoutput: "输入密码"
io7=>inputoutput: "输入管理员密码"
c1=>condition: "==0"
c2=>condition: "==1"
c3=>condition: "房间号已存在?"
sub->io1->c1(no)->c2(no)->io1
c1(yes)->io2->io3->io4->io2
c2(yes)->io5->io6->c3(yes)->io7(right)->io1
c3(no)->io1
```
