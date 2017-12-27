
```flow
st=>start: 启动
OpPass1=>operation: 输入管理员密码
OpMode1=>operation: 选择模式(指纹、NFC、键盘)
OpMode20=>operation: 选择指纹模式(登入/注册)
OpMode21=>operation: 选择NFC模式(登入/注册)
OpMode22=>operation: 选择键入模式(登入/注册)

c_pass1=>condition: 密码正确?
c20=>condition: 是否为指纹模式?
c21=>condition: 是否为NFC模式?
c22=>condition: 是否为键入模式?
e=>end: 关闭

st->OpPass1->c_pass1
c_pass1(yes)->OpMode1
c_pass1(no)->OpPass1
OpMode1->c20(no)->c21(no)->c22(no)->OpMode1
c20(yes)->OpMode20
c21(yes)->OpMode21
c22(yes)->OpMode22
```
