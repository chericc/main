# TODO

- blog/science/linux/锁的实现.md
- blog/science/linux/系统调用.md
- blog/science/cpp/右值和右值引用.md
- demos/memorypool
- demos/kernelprograming
- demos/networkprograming
- demos/ffmpeg
- net/tcp/tcpsocket
- 条件变量加锁的问题
- linux/udev
- xtools/xstring/xstringview + .arg(val) + .split(spliter) + ...
- Linux/组播转发与普通UDP包转发的区别
- cma内存与linux页迁移：
  为什么scudo分配器，在分配小内存时，无法使用剩余很多的cma内存，而分配大内存时，可以使用剩余的cma内存？
  页迁移：https://blog.csdn.net/21cnbao/article/details/108067917
- android-native-memleak https://blog.csdn.net/u012759483/article/details/122965775 https://github.com/bytedance/memory-leak-detector/blob/master/README_cn.md
  及其对linux平台开发调试的启发
  android平台的调试手段，比如 debuggerd 的原理及其对linux平台开发调试的启发
- 了解下android下调用C的方式，C下的调试手段在android下如何使用（比如符号的注入）
- c++重载和重写，子类和基类的同名不同参的非虚函数是什么关系
  示例如下：
  ```c++
  #include <iostream>
  using std::cout;
  using std::endl;

  class Parent
  {
  public:
    void FunTest()
    {
      cout << "Parent Ok!" << endl;
    }
  };
  class Child : public Parent
  {
  public:
    void FunTest(int p1) // 位置1
    {
      cout << "Child Ok " << p1 << endl;
    }
  };
  int main()
  {
    Child oc1;		  // 位置2
    oc1.FunTest();	  // 位置3
    oc1.FunTest(100); // 位置4
  }
  ```
  - lua and so
  - libevent
  - 一个类似日志的状态记录器，方便各个模块更新模块的状态，方便查询模块的状态
  - git_gen_diff_dir.sh
  - seperate debug info(objcopy) blog
  - apache2 + ssh 实现反向代理+内网穿透