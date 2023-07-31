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
- gRPC
  
  写一个分层结构时，底层的模块一般只能由上层的模块调用。这引入了耦合。

  可以通过一些水平的通信方法，让这些模块取得水平的地位。这样，高层模块和底层模块之间就去除了层次带来的耦合性。

  例如，程序目标是生成告警事件。底层模块负责搜集告警，上层负责对各种告警信息进行集中并附带一些业务数据和实现一些业务要求。如果直接让上层调用下层，则上层直接依赖了下层，为了测试上层的逻辑，则必须对下层进行修改。如果采用水平结构，则可以另外构造一个工具用于给上层输出数据（因为这些数据是松耦合的，不强调来源），从而更好的对上层进行测试。

  可以找一下不同的RPC实现框架，对比它们的优劣（比如性能、场景、原理）。

  后续可以在某些小程序上应用RPC进行重构。