# TODO

- blog/science/linux/锁的实现.md
- blog/science/linux/系统调用.md
- blog/science/cpp/右值和右值引用.md
- demos/threadpool
- demos/memorypool
- demos/kernelprograming
- demos/networkprograming
- demos/linux/pipe
- demos/ffmpeg
- net/tcp/tcpsocket
- xtimer
  定时器（可以理解为一个闹钟）。
  搜索定时器原理，并实现一个简单的定时器；
- 条件变量加锁的问题
- linux/udev
- xtools/xstring/xstringview + .arg(val) + .split(spliter) + ...
- linux/net
  网络数据包在两个进程之间的流动路径；UDP方式发送数据包时，哪些位置可能出现丢包；出现丢包时，如何确认丢包的原因；
  CPU中断在丢包中扮演的角色
- CPU中断与网络负载，CPU中断的分配，CPU中断的分析
- 组播；组播的转发特征；ffmpeg与组播流；RTSP与组播流在UDP协议上的区别；
- cma内存与linux页迁移：
  为什么scudo分配器，在分配小内存时，无法使用剩余很多的cma内存，而分配大内存时，可以使用剩余的cma内存？
  页迁移：https://blog.csdn.net/21cnbao/article/details/108067917
- docker用法
- android-native-memleak https://blog.csdn.net/u012759483/article/details/122965775 https://github.com/bytedance/memory-leak-detector/blob/master/README_cn.md
  及其对linux平台开发调试的启发
  android平台的调试手段，比如 debuggerd 的原理及其对linux平台开发调试的启发
- printf() 变参的原理 printf("%d", char) 的效果
- select及其替代品的原理