# gperftools

## doc

gperftools-2.10/docs/index.html

## CPU profiler

### 功能

```
CPU profiler
```

### demo

```c++
// test.cpp
int g_a = 0;

void fun3()
{
    for (int i = 0; i < 50; ++i)
    {
        g_a += (i % 5);
    }
}

void fun2()
{
    for (int i = 0; i < 100; ++i)
    {
        g_a += (i % 5);
    }
}

void fun1()
{
    for (int i = 0; i < 1 * 1000 * 1000; ++i)
    {
        g_a += (i % 5);

        fun2();
        fun3();
    }
}

int main()
{
    fun1();

    return g_a;
}
```

### steps

```bash
# how to build gperftools?
# refer to /README.md

g++ test.cpp -c -g -o test.o
g++ test.o -o test.out 
LD_PRELOAD=/home/test/opensrc/gperftools/build/output/lib/libprofiler.so CPUPROFILE=/tmp/prof.out ./test.out

# CPUPROFILE_FREQUENCY:interrupts/second
LD_PRELOAD=/home/test/opensrc/gperftools/build/output/lib/libprofiler.so CPUPROFILE=/tmp/prof.out CPUPROFILE_FREQUENCY=100 ./test.out

/home/test/opensrc/gperftools/build/output/bin/pprof-symbolize --help
/home/test/opensrc/gperftools/build/output/bin/pprof-symbolize --text ./test.out /tmp/prof.out
/home/test/opensrc/gperftools/build/output/bin/pprof-symbolize --svg ./test.out /tmp/prof.out > output.svg
/home/test/opensrc/gperftools/build/output/bin/pprof-symbolize --svg --lines ./test.out /tmp/prof.out > output.svg
```

## CPU profiler FAQ

### 看不到有效的信息或者信息不足

```
因为统计的方式是采样，程序本身的负荷、运行时间和工具的采样频率，都会影响采样点的数量。可以调整CPUPROFILE_FREQUENCY来增加采样频率（每秒的采样点数）；
```

### 如果程序被strip或者没有加-g会如何

```
单纯没有-g选项，则不能显示除程序中函数对应的文件和行信息；
如果程序被strip，则统计信息中涉及到程序部分的，将只会有地址信息，并且分析得到的调用层次及函数的CPU占用将混乱（不可读）；
```

### sleep时间是否被计入？

```c++
// test.cpp
#include <thread>

void fun3()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
}

void fun2()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

void fun1()
{
    for (int i = 0; i < 500; ++i)
    {
        fun2();
        fun3();
    }
}

int main()
{
    fun1();

    return 0;
}
```

经试验，结论为CPU Profiler不计入sleep的时间，也即其分析的是程序占用CPU的来源；

### 如果程序持续运行，如何控制CPU Profiler生成信息（包括多线程的处理）？

```c++
// test.cpp
#include <thread>

static int s_value = 0;
static int s_break_flag = 0;
static int s_round = 40 * 1000 * 1000;

void fun3()
{
    while (true)
    {
        for (int i = 0; i < s_round; ++i)
        {
            s_value += (i % 5);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        if (s_break_flag)
        {
            break;
        }
    }
}

void fun2()
{
    while (true)
    {
        for (int i = 0; i < s_round; ++i)
        {
            s_value += (i % 5);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
        if (s_break_flag)
        {
            break;
        }
    }
}

void fun1()
{
    while (true)
    {
        for (int i = 0; i < s_round; ++i)
        {
            s_value += (i % 5);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(4));
        if (s_break_flag)
        {
            break;
        }
    }
}

int main()
{
    std::thread trd1(fun1);
    std::thread trd2(fun2);
    std::thread trd3(fun3);

    trd1.join();
    trd2.join();
    trd3.join();

    return s_value;
}
```

```bash
g++ test.cpp -g -o test.out
LD_PRELOAD=/home/test/opensrc/gperftools/build/output/lib/libprofiler.so CPUPROFILE=/tmp/prof.out CPUPROFILESIGNAL=12 CPUPROFILE_FREQUENCY=1000 ./test.out

# start analysis
sudo killall -12 test.out 

# stop analysis
sudo killall -12 test.out 

# output
/home/test/opensrc/gperftools/build/output/bin/pprof-symbolize --text ./test.out /tmp/prof.out
Using local file ./test.out.
Using local file /tmp/prof.out.
Removing _fini from all stack traces.
Removing _fini from all stack traces.
Removing _fini from all stack traces.
Removing _fini from all stack traces.
Removing _fini from all stack traces.
Removing std::error_code::default_error_condition@@GLIBCXX_3.4.11 from all stack traces.
Total: 3534 samples
    1211  34.3%  34.3%     1211  34.3% fun1
    1202  34.0%  68.3%     1202  34.0% fun3
    1121  31.7% 100.0%     1121  31.7% fun2
```

### 只对程序中的某一些代码进行分析如何实现？

```
#include "profiler.h"
ProfilerStart()
code...
ProfilerStop()
```

## heap-profiling

### 功能

```
1. 给出当前堆内存里有什么；
2. 定位内存泄漏；
3. 找出申请大量内存的位置；
```

### demo

```c++
#include <stdlib.h>
#include <list>

static std::list<void *> s_list;

void fun1()
{
    // leak
    void *p = malloc(64 * 1024);
    p = nullptr;
}

void fun2()
{
    // mem bomb
    for (int i = 0; i < 64; ++i)
    {
        void *p = malloc(64 * 1024);
        s_list.push_back(p);
    }
}

void fun3()
{
    // leak
    unsigned char *pmem = new unsigned char [64 * 1024];
    pmem = nullptr;
}

int main()
{
    fun1();
    fun2();
    fun3();
    return 0;
}
```

### steps

```bash
# compile
g++ test.cpp -g -o test.out

# run
LD_PRELOAD=/home/test/opensrc/gperftools/build/output/lib/libtcmalloc.so HEAPPROFILE=/tmp/prof.out HEAP_PROFILE_TIME_INTERVAL=1 ./test.out

# here, some files like "/tmp/prof.out.0001.heap" will be created.

# analyze
# svg
/home/test/opensrc/gperftools/build/output/bin/pprof-symbolize --svg --lines ./test.out /tmp/prof.out.0002.heap > output.svg
# text
/home/test/opensrc/gperftools/build/output/bin/pprof-symbolize --text --lines ./test.out /tmp/prof.out.0002.heap
```

