# gprof

## code

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

## steps

```bash
g++ -c -pg test.cpp -O0
g++ test.o -pg
./a.out
```

```bash
gprof a.out gmon.out -b
Flat profile:

Each sample counts as 0.01 seconds.
  %   cumulative   self              self     total
 time   seconds   seconds    calls  ms/call  ms/call  name
 72.22      0.26     0.26  1000000     0.00     0.00  fun2()
 25.00      0.35     0.09  1000000     0.00     0.00  fun3()
  2.78      0.36     0.01        1    10.00   360.00  fun1()


                        Call graph


granularity: each sample hit covers 4 byte(s) for 2.78% of 0.36 seconds

index % time    self  children    called     name
                                                 <spontaneous>
[1]    100.0    0.00    0.36                 main [1]
                0.01    0.35       1/1           fun1() [2]
-----------------------------------------------
                0.01    0.35       1/1           main [1]
[2]    100.0    0.01    0.35       1         fun1() [2]
                0.26    0.00 1000000/1000000     fun2() [3]
                0.09    0.00 1000000/1000000     fun3() [4]
-----------------------------------------------
                0.26    0.00 1000000/1000000     fun1() [2]
[3]     72.2    0.26    0.00 1000000         fun2() [3]
-----------------------------------------------
                0.09    0.00 1000000/1000000     fun1() [2]
[4]     25.0    0.09    0.00 1000000         fun3() [4]
-----------------------------------------------


Index by function name

   [2] fun1()                  [3] fun2()                  [4] fun3()
```

## explain

一共输出两张表：第一张为flat profile，即所有函数的耗时及调用次数；第二张为call graph，展示每个函数被谁调用，调用了谁，以及次数，并且也有每个函数的子调用的估计耗时；

### flat profile

% time 函数总的运行时间；

cumulative seconds 该函数及之上函数的总时长；

self seconds 该函数总时长；

calls 调用次数；

self ms/call 函数的每次调用平均耗时（毫秒）；

total ms/call 函数及其子调用的每次调用平均耗时（毫秒）；

### call graph

index 表的索引编号

% time 函数及其子调用占用的时间百分比

self 函数自身花费的总时间

children 函数子调用花费的总时间

called 函数被调用的次数，如果为递归调用，a+b，其中b为调用调用次数；

对于某个函数的调用函数和被调用函数，结果中仅显示调用路径中的花费时间和次数；

## example

### 初步分析

结合对 test_bmp 的一个分析结果进行优化：

内容参考 output.txt

由 flat profile 可知，大部分都与 vector<char...>有关；

然后观察 call graph ，

第一个函数：

对于 bmp_decoder_read_test_Test ，其时间占用为 71.4% ，并且其子函数调用次数不多，并且self耗时均为0（很小），因此判定这个调用路径耗时不大；

第二个函数：

首先，self有不为0的值，并且调用次数很大，因此判定，这个是第一个要解决的性能瓶颈位置；

### 找到第二个函数的位置

直接搜索 [2] ，就可以找到输出中的所有使用第二个函数的位置，其调用者包括 vector::resize, vector比较，vector拷贝构造