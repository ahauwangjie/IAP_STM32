# 网络远程烧录stm32

功能
---
网络远程烧录stm32

开发平台
---
* ide：keil
* platform： stm32

实现原理
---
1. 移植uip到stm32，满足网络功能
2. stm32运行iap程序，并检查是否有升级包，
   如远端无升级包，则进入3；
   如远端有升级包，则下载升级包，下载完成后重启，判断无升级包的情况，进入3。
3. 进入app程序

参考资料
---
material/第五十七章ENC28J60网络实验战舰STM32开发板.pdf
