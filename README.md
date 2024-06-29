# Grbl移植到51单片机。
Grbl是一个非常优秀的CNC固件，在业余CNC爱好者中被广泛使用，但是并没有人完整的将Grbl移植到51单片机上，因此我把Grbl在51单片机上进行了移植，希望能对想了解Grbl或对Grbl进行移植的小伙伴们提供一些帮助。本固件基于我中文注解的最新版Grbl，花费了很大精力，如果您觉得有帮助，不妨在下面的二维码打赏。

***
_点击 `Release` 页签下载编译好的 `.hex` 文件 或 [点击这里](https://github.com/MillerRen/grbl-stc/releases)_
***

## 使用说明
在grbl的基础上添加了USB支持，由于P3.0和P3.1被占用，串口引脚重映射到P3.6、P3.7，USB和串口共用一个缓冲器。默认典型器件是STC的STC8H8K64U,LQFP32封装。引脚定义请参见源码配置文件`config.h`。

* [许可证](https://github.com/gnea/grbl/wiki/Licensing): Grbl是自由软件, 在 GPLv3 许可证下发布。

* 更多信息或帮助, 查看我们的WiKi **[WiKi页面!](https://github.com/MillerRen/grbl/wiki)** 如果上面的信息过时了, 请编辑它保持最新并和我们沟通! 谢谢!

## 路线图
- 支持USB    √
- 支持舵机
- 支持多轴
- 控制板

## 支持及赞助
<img src="./images/donate.png">   

## CNC技术讨论
<img src="./images/cncnc.png">

