LPB100_UPGRADE_V1.0.04a-23_2M_201406271810	---升级工具/网页升级文件,(适用于2MB Flash模块,HF-LPB100,HF-LPT100）
LPB100_V1.0.04a-23_2M_201406271810		---串口升级文件(适用于2MB Flash模块,HF-LPB100,HF-LPT100）
LPB100_UPGRADE_V1.0.04a-23_1M_201406271810.bin	---升级工具/网页升级文件(适用于1MB Flash模块,HF-LPT200)
LPB100_V1.0.04a-23_1M_201406271810.bin		---串口升级文件(适用于1MB Flash模块,HF-LPT200）

版本更新记录：

V1.0.04a-23：
1、去掉UDPLCPT功能，防止升级后没有做reload导致UDP收不到数据；
2、修复SmartLink log没有带IP问题；

V1.0.04a-21
1. 修改：smartlink如果周围一个AP也搜不到的情况下，按reload可以重新进入；
2. 修改：smartlink过程中可能意外重启，修改重启后继续进入smartlink模式；
3. 改进串口分包；
4. 修改网页登录名点击3次就会进入的bug；
5. 修改波特率去掉300；
6. 支持Firmware8.0 /9.0；

V1.0.04a-17
1、解决-16版本串口分包的问题。

V1.0.04a-16
更新说明：
1， 解决连接极路由重启现象；
2、增加本地UDP端口设置命令AT+UDPLCPT；例：AT+UDPLCPT=1234,2345将socketA本地端口设置为1234，将socketB的本地端口设置为2345，目标端口仍由NETP、SOCKB设置；
3、增加流程处理SmartLink Fail情况(配置模块成功，APP显示失败问题)，配合新版APP使用(暂时只有Anadroid版本，增加扫描机制)，提高成功率；
(注：升级后必须做恢复出厂设置，不然NTP、UDP端口是错误的)

Firmware V8.0更新记录：

实现N信道抓包，优化SmartLink