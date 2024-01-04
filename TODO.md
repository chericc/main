# TODO

- blog/science/cs/计算机操作系统-进程同步.md
- blog/science/linux/锁的实现.md
- blog/science/linux/系统调用.md
- blog/science/linux/终端开发环境.md
- blog/science/cpp/右值和右值引用.md
- demos/threadpool
- demos/memorypool
- demos/kernelprograming
- demos/networkprograming
- demos/linux/pipe
- demos/ffmpeg
- net/tcp/tcpsocket
- gRPC
  
  写一个分层结构时，底层的模块一般只能由上层的模块调用。这引入了耦合。

  可以通过一些水平的通信方法，让这些模块取得水平的地位。这样，高层模块和底层模块之间就去除了层次带来的耦合性。

  例如，程序目标是生成告警事件。底层模块负责搜集告警，上层负责对各种告警信息进行集中并附带一些业务数据和实现一些业务要求。如果直接让上层调用下层，则上层直接依赖了下层，为了测试上层的逻辑，则必须对下层进行修改。如果采用水平结构，则可以另外构造一个工具用于给上层输出数据（因为这些数据是松耦合的，不强调来源），从而更好的对上层进行测试。

  可以找一下不同的RPC实现框架，对比它们的优劣（比如性能、场景、原理）。

  后续可以在某些小程序上应用RPC进行重构。

- xtimer

  定时器（可以理解为一个闹钟）。

  搜索定时器原理，并实现一个简单的定时器；

- 松排序算法

  考虑定时器场景，我们每次都需要取出当前最近的一个或几个节点，并且会频繁地新增节点。如果使用普通的排序算法（比如快速排序），或者一些有序数据结构（比如红黑树），当节点规模很大时，新增一个节点带来的计算量会很大。

  以快速排序为例，

- 编译器的处理

  const int a = 2; int *pa = (int*)&a; *pa = 1; a = ? ; *pa = ?

  注意：g++ 和 gcc 在这里的处理还不一样，gcc更符合直觉（能修改）

- 条件变量加锁的问题

- linux/udev

- xtools/xstring/xstringview + .arg(val) + .split(spliter) + ...

- linux/net

  网络数据包在两个进程之间的流动路径；UDP方式发送数据包时，哪些位置可能出现丢包；出现丢包时，如何确认丢包的原因；

  CPU中断在丢包中扮演的角色

- CPU中断与网络负载，CPU中断的分配，CPU中断的分析
- 组播；组播的转发特征；ffmpeg与组播流；RTSP与组播流在UDP协议上的区别；
- C++正则匹配是栈上展开
- https://github.com/gteall/pcap-parser
- rtp推流服务器（可以参考ffmpeg）
- cma内存与linux页迁移：
  为什么scudo分配器，在分配小内存时，无法使用剩余很多的cma内存，而分配大内存时，可以使用剩余的cma内存？
  页迁移：https://blog.csdn.net/21cnbao/article/details/108067917

- 写一个git补丁合并工具，支持宽松的合并策略（默认的合并方式大多数情况下都会失败），比如，不考虑行数，只考虑上下文；
- system() 函数的风险完善 fork 和 vfork 的区别  是否有占用内存的风险  什么时候（内核版本）有风险？（占用2倍的内存）
- rsync用法（场景：一个服务器上编辑代码，另一个服务器上编译。需要关注增量传输，文件更新时间）