# 内存泄漏调试

本文主要介绍利用dmalloc进行内存调试的方法。

## 1 dmalloc

### 1.1 dmalloc环境准备

系统环境（demo）：

```bash
Ubuntu22.04,x86_64
```

#### 1.1.1 内容下载

dmalloc 源码：
<https://dmalloc.com/releases/dmalloc-5.6.5.tgz>

dmalloc 文档：
<https://dmalloc.com/docs/dmalloc.pdf>

#### 1.1.2 编译

在下载好`dmalloc-5.6.5.tgz`文件之后，可按如下步骤编译：

```bash
$ ls
dmalloc-5.6.5.tgz

$ tar xf dmalloc-5.6.5.tgz 

$ ls
dmalloc-5.6.5  dmalloc-5.6.5.tgz

$ mkdir build

$ ls
build  dmalloc-5.6.5  dmalloc-5.6.5.tgz

$ cd build/

$ ../dmalloc-5.6.5/configure \
    --prefix=$(pwd)/output \
    --host=x86_64-linux-gnu \
    CC=gcc \
    CXX=g++ \
    CFALGS=fPIC
$ make threadscxx
$ make installthcxx

$ ls output/lib/
libdmallocthcxx.a

$ ls | grep dmalloc\\.h$
dmalloc.h
```

编译结束后，`./dmalloc.h`和`./output/lib/libdmallocthcxx.a`文件即为我们需要的头文件和库文件。

#### 1.1.3 编译的一些注意事项

在实际的项目运用中，比较实用的方式是将dmalloc编译成一个静态库（打开了-fPIC选项的），然后将这个静态库集成到被目标程序会链接的某个动态库上。在编译目标程序时，要将dmalloc所在的库放在最后。

### 1.2 demo演示

通过一个demo来演示如何利用dmalloc来跟踪程序中存在的内存泄漏问题。

#### 1.2.1 介绍

将dmalloc编译得到的头文件和库文件拷贝到demo_dmalloc目录中，如下：

```bash
$ ls
dmalloc.h  dmalloc_main.cpp  dmalloc_main.h  libdmallocthcxx.a  main.cpp  Makefile
```

其中：

- `dmalloc.h`、`libdmallocthcxx.a`为dmalloc编译后得到的文件；

- `dmalloc_main.h/cpp`为方便使用dmalloc编写的一个工具类，这个工具类会自动运行（通过全局对象的自动构造实现）；在实际项目使用时，可以直接复用这个工具类。

- `main.cpp`为demo的实现部分，包含了一些存在资源泄漏的函数。

#### 1.2.2 演示

##### 1.2.2.1 找出泄漏特征

```bash



# 给定dmalloc环境变量
$ export DMALLOC_OPTIONS=debug=0x4000503,log=/tmp/logfile

# 编译
$ make clean; make debug;

# 执行
$ ./debug/a.out -a

# demo运行后，回车一次调用一次资源泄漏的函数。

# 创建标记文件，会记录当前未释放的内存。
$ touch /tmp/malloc_unfreed

# 回车一次
$ ^Enter

# 泄漏几次之后，再次创建标记文件，记录当前未释放的内存。
$ touch /tmp/malloc_unfreed

# 一段时间后（/tmp/malloc_unfreed被删除掉），结束程序。
$ ^C

# 查看dmalloc日志文件
$ cat /tmp/logfile
1677916302: 18: Dmalloc version '5.6.5' from 'http://dmalloc.com/'
1677916302: 18: flags = 0x4000503, logfile '/tmp/logfile'
1677916302: 18: interval = 0, addr = 0x0, seen # = 0, limit = 0
1677916302: 18: threads enabled, lock-on = 0, lock-init = 2
1677916302: 18: starting time = 1677916301
1677916302: 18: process pid = 20176
1677916302: 18: WARNING: tried to free(0) from 'unknown'
1677916302: 21: WARNING: tried to free(0) from 'unknown'
1677916302: 29: WARNING: tried to free(0) from 'unknown'
1677916321: 32: -------------- log unfreed begin -------------
1677916321: 33: dumping the unfreed pointers
1677916321: 33: Dumping Not-Freed Pointers Changed Since Start:
1677916321: 33:  not freed: '0x7fe4132e4008|s1' (1048576 bytes) from 'unknown'
1677916321: 33:  not freed: '0x7fe4133e5008|s1' (1048576 bytes) from 'unknown'
1677916321: 33:  not freed: '0x7fe4134e6008|s1' (4096 bytes) from 'unknown'
1677916321: 33:  not freed: '0x7fe4134ea008|s1' (1048576 bytes) from 'unknown'
1677916321: 33:  not freed: '0x7fe4135edf68|s1' (20 bytes) from 'unknown'
1677916321: 33:  not freed: '0x7fe4135edf88|s1' (20 bytes) from 'unknown'
1677916321: 33:  not freed: '0x7fe4135edfa8|s1' (20 bytes) from 'unknown'
1677916321: 33:  not freed: '0x7fe4135edfc8|s1' (15 bytes) from 'unknown'
1677916321: 33:  not freed: '0x7fe4135edfe8|s1' (18 bytes) from 'unknown'
1677916321: 33:  not freed: '0x7fe4135eeb08|s1' (80 bytes) from 'unknown'
1677916321: 33:  not freed: '0x7fe4135eeb88|s1' (80 bytes) from 'unknown'
1677916321: 33:  not freed: '0x7fe4135eec08|s1' (80 bytes) from 'unknown'
1677916321: 33:  not freed: '0x7fe4135eec88|s1' (80 bytes) from 'unknown'
1677916321: 33:  not freed: '0x7fe4135eed08|s1' (80 bytes) from 'unknown'
1677916321: 33:  not freed: '0x7fe4135eed88|s1' (80 bytes) from 'unknown'
1677916321: 33:  not freed: '0x7fe4135eee08|s1' (80 bytes) from 'unknown'
1677916321: 33:  not freed: '0x7fe4135eee88|s1' (80 bytes) from 'unknown'
1677916321: 33:  not freed: '0x7fe4135eef08|s1' (80 bytes) from 'unknown'
1677916321: 33:  not freed: '0x7fe4135eef88|s1' (80 bytes) from 'unknown'
1677916321: 33:  not freed: '0x7fe4135ef808|s1' (1024 bytes) from 'unknown'
1677916321: 33:  not freed: '0x7fe4135f0808|s1' (303 bytes) from 'unknown'
1677916321: 33:  not freed: '0x7fe4135f0c08|s1' (472 bytes) from 'unknown'
1677916321: 33:  not freed: '0x7fe4135f0e08|s1' (288 bytes) from 'unknown'
1677916321: 33:  not freed: '0x7fe413df2008|s1' (72704 bytes) from 'unknown'
1677916321: 33:  not freed: '0x7fe414367fc8|s1' (24 bytes) from 'unknown'
1677916321: 33:  total-size  count  source
1677916321: 33:           0      0  Total of 0
1677916321: 33: -------------- log unfreed end -------------
1677916333: 34: -------------- log unfreed begin -------------
1677916333: 35: dumping the unfreed pointers
1677916333: 35: Dumping Not-Freed Pointers Changed Since Start:
1677916333: 35:  not freed: '0x7fe4131e3008|s1' (1048576 bytes) from 'unknown'
1677916333: 35:  not freed: '0x7fe4132e4008|s1' (1048576 bytes) from 'unknown'
1677916333: 35:  not freed: '0x7fe4133e5008|s1' (1048576 bytes) from 'unknown'
1677916333: 35:  not freed: '0x7fe4134e6008|s1' (4096 bytes) from 'unknown'
1677916333: 35:  not freed: '0x7fe4134ea008|s1' (1048576 bytes) from 'unknown'
1677916333: 35:  not freed: '0x7fe4135edf68|s1' (20 bytes) from 'unknown'
1677916333: 35:  not freed: '0x7fe4135edf88|s1' (20 bytes) from 'unknown'
1677916333: 35:  not freed: '0x7fe4135edfa8|s1' (20 bytes) from 'unknown'
1677916333: 35:  not freed: '0x7fe4135edfc8|s1' (15 bytes) from 'unknown'
1677916333: 35:  not freed: '0x7fe4135edfe8|s1' (18 bytes) from 'unknown'
1677916333: 35:  not freed: '0x7fe4135eeb08|s1' (80 bytes) from 'unknown'
1677916333: 35:  not freed: '0x7fe4135eeb88|s1' (80 bytes) from 'unknown'
1677916333: 35:  not freed: '0x7fe4135eec08|s1' (80 bytes) from 'unknown'
1677916333: 35:  not freed: '0x7fe4135eec88|s1' (80 bytes) from 'unknown'
1677916333: 35:  not freed: '0x7fe4135eed08|s1' (80 bytes) from 'unknown'
1677916333: 35:  not freed: '0x7fe4135eed88|s1' (80 bytes) from 'unknown'
1677916333: 35:  not freed: '0x7fe4135eee08|s1' (80 bytes) from 'unknown'
1677916333: 35:  not freed: '0x7fe4135eee88|s1' (80 bytes) from 'unknown'
1677916333: 35:  not freed: '0x7fe4135eef08|s1' (80 bytes) from 'unknown'
1677916333: 35:  not freed: '0x7fe4135eef88|s1' (80 bytes) from 'unknown'
1677916333: 35:  not freed: '0x7fe4135ef808|s1' (1024 bytes) from 'unknown'
1677916333: 35:  not freed: '0x7fe4135f0808|s1' (303 bytes) from 'unknown'
1677916333: 35:  not freed: '0x7fe4135f0c08|s1' (472 bytes) from 'unknown'
1677916333: 35:  not freed: '0x7fe4135f0e08|s1' (288 bytes) from 'unknown'
1677916333: 35:  not freed: '0x7fe413df2008|s1' (72704 bytes) from 'unknown'
1677916333: 35:  not freed: '0x7fe414367fc8|s1' (24 bytes) from 'unknown'
1677916333: 35:  total-size  count  source
1677916333: 35:           0      0  Total of 0
1677916333: 35: -------------- log unfreed end -------------

# 观察日志的内容：
# 程序运行过程中一共记录了两次内存情况，因此日志中一共有两段内存记录，如下
$ cat /tmp/logfile | grep -n "log unfreed"
10:1677916321: 32: -------------- log unfreed begin -------------
40:1677916321: 33: -------------- log unfreed end -------------
41:1677916333: 34: -------------- log unfreed begin -------------
72:1677916333: 35: -------------- log unfreed end -------------

# 将这两段内存记录分别提取出来，排序，对比，观察变化的部分；
# 这里用命令行工具演示，也可以用excel进行数据处理，用文件对比工具比较差异；
$ sed -n '10,40p' /tmp/logfile > record1.txt
$ sed -n '41,72p' /tmp/logfile > record2.txt
$ cat record1.txt | grep "not freed:" | sed -E 's/.*\(([0-9]*).*/\1/g' | sort -nr > record1_sort.txt
$ cat record2.txt | grep "not freed:" | sed -E 's/.*\(([0-9]*).*/\1/g' | sort -nr > record2_sort.txt
$ diff -y record1_sort.txt record2_sort.txt
1048576                                                         1048576
1048576                                                         1048576
1048576                                                         1048576
                                                              > 1048576
72704                                                           72704
4096                                                            4096
1024                                                            1024
472                                                             472
303                                                             303
288                                                             288
80                                                              80
80                                                              80
80                                                              80
80                                                              80
80                                                              80
80                                                              80
80                                                              80
80                                                              80
80                                                              80
80                                                              80
24                                                              24
20                                                              20
20                                                              20
20                                                              20
18                                                              18
15                                                              15

# 通过对比结果，发现第二次相比第一次，多了一个1048576。联系到demo每次会申请1024*1024=1048576的内存，表现一致。
```

至此，已经找到了demo的泄漏特征（泄漏大小为1048576）。

##### 1.2.2.2 找出泄漏位置

当前dmalloc在demo演示环境下，不能提供调用栈信息或者给出调用者地址（因为获取这些信息的方式与具体的编译器和平台有关，dmalloc支持不完善）。可以根据单次泄漏的大小利用GDB找出内存申请的可能位置，演示如下：

```bash
# 定位到dmalloc_main.cpp的my_track函数，这个函数是dmalloc的一个钩子函数，每次执行内存分配时，这个函数都会被调用；
# 将其中的uSizeIntrested修改为1048576，并重新编译，这样，程序在运行时如果出现了一个对应大小的分配，就会在这个函数中走进对应逻辑产生段错误，生成core文件，然后利用GDB载入这个core文件，就能知道对应的调用栈的情况；
# static unsigned int uSizeIntrested = 1048576;
$ make clean; make debug;

# 使能core
$ sudo sh -c "echo '/tmp/core' > /proc/sys/kernel/core_pattern"
$ ulimit -c unlimited

$ rm /tmp/core*
$ ./debug/dmalloc_test.out -a
[debug][dmalloc_main.cpp 153 DmallocImp]DmallocImp::construction
cicling



**************   break: size=1048576,count=1   **************


Segmentation fault (core dumped)
$ gdb debug/dmalloc_test.out /tmp/core.21056
$ (gdb) bt
#0  0x0000565082634be6 in my_track (file=0x0, line=0, func_id=10, byte_size=1048576, alignment=0, old_addr=0x0, new_addr=0x7f248f6ab008)
    at dmalloc_main.cpp:43
#1  0x000056508263a011 in dmalloc_malloc (file=0x0, line=0, size=1048576, func_id=10, alignment=0, xalloc_b=0) at ../dmalloc-5.6.5/user_malloc.c:766
#2  0x0000565082635b21 in leak_malloc_big () at main.cpp:44
#3  0x00005650826365c7 in main (argc=2, argv=0x7ffc9f14cdd8) at main.cpp:183

# 这样，就找到了泄漏的可能位置。
```

### 1.3 其它用途

#### 1.3.1 内存占用优化

找出程序的所有内存信息后，确定哪些内存申请占比较大，并利用上述方法找出其中占用比较大的内存的位置。在代码中检查这些位置是否存在内存优化可能。

### 1.4 dmalloc检查内存泄漏的原理

dmalloc实现了一套内存动态申请和释放的接口，可以完全替换掉C库中的内存申请和释放相关接口，同时重写了new和delete，可以跟踪C和C++中所有的内存申请和释放。

```bash
#include <stdlib.h>
int main()
{
	// [p1,64]
	void *p1 = malloc(64);
	
	// [p1,64],[p2,64]
	void *p2 = malloc(64);
	
	// [p2,64]
	free(p1);
	
	return 0;
}
```

在重写了malloc和free之后，可以在每次调用后记录得到的地址和对应的大小，在释放时抹掉对应地址项，这样就得到了一个动态的当前所有未释放内存的记录。

### 1.5 一个网损引起内存泄漏的例子

#### 1.5.1 前期现象

部分被测对象出现了功能问题，经观察为内存泄漏引起。同版本部分设备没问题。

#### 1.5.2 前期分析

选择性关闭部分功能，并运行观察，发现内存泄漏存在白天泄漏、晚上不泄露的特征，因此分析内存泄漏可能与告警触发有关。

尝试在研发环境复现泄漏问题，没有复现（测试时间有2天以上）。

尝试用研发设备在测试环境复现，没有复现（测试时间只有1小时左右，时间太短）。

#### 1.5.3 dmalloc分析

使用一台测试环境中能稳定复现的设备，利用dmalloc制作版本之后进行测试。测试过程中，每隔一段时间记录一次当前的所有未释放的内存。经过对分配的内存进行进行分析，发现存在较多内存泄漏。

其表现为：

```bash
15:32 --> 16:46
count * mem
38 * 16717
20 * 2891
20 * 516
40 * 392
40 * 312
40 * 280
40 * 256
20 * 208
20 * 176
40 * 172
20 * 128
220 * 32
640 * 16
40 * 4
...

15:32 --> 09:23
count * mem
292 * 16717
146 * 2891
146 * 1133
146 * 516
292 * 392
292 * 312
292 * 280
292 * 256
146 * 208
146 * 176
292 * 172
146 * 128
1606 * 32
4673 * 16
292 * 4
...

```

经过对16717的多次跟踪，为`mbedtls_ssl_setup`。考虑到16717这个值的特殊性，因此可以认为所有的内存泄漏点均为`mbedtls_ssl_setup`引起。

#### 1.5.4 补充分析

1. 经过查看外部依赖库的更新日志（考虑到ssl调用的可能源），发现对应库存在内存泄漏相关更新，并且更新说明上有关于网络问题的描述。
2. 从top上看，内存泄漏主要体现在DIRTY上。经查wolfssl中`mbedtls_ssl_setup`的实现，泄漏源为`calloc`（与dmalloc泄漏特征一致）。经demo试验，`calloc`分配的内存确实为DIRTY内存（可以针对这一点具体分析一下Linux下分配内存对应的具体系统行为）。
3. 最终结论为：网损环境（丢包）上触发告警事件，就会引起内存泄漏。

## 2 内存泄漏的两种情况

- 如果一个程序即将结束时，所有申请的内存都被释放掉了，是不是就不存在内存泄漏？

  大致可以把内存泄漏归为两种，一种是申请内存之后不再释放引起的泄漏（A），一种是申请之后在某个阶段会释放，但是释放时机太晚从而占用了大量未使用内存的泄漏（B）。总的说，如果程序超出了实际需要，过多的占用内存，以至于影响了业务正常运行，就认为是出现了内存泄漏。

  对于第一种泄漏的情况，通过记录进程整个生命期申请和释放的内存，找出进程即将结束时仍然没有释放的内存，就是泄漏的内存。

  对于第二种泄漏的情况，需要观察进程在两个时间点之间的内存占用的差异，找出额外大量增加的内存，这些额外增加的内存就是泄漏的内存。

## 3 内存泄漏的测试

考虑主进程为常驻进程的情况，可以通过统计一些信息判断是否存在内存泄漏。

比如在环境中跑一个脚本，定期打印信息。

### 3.1 /proc/meminfo

```bash
$ cat /proc/meminfo 
MemTotal:          59656 kB
MemFree:            1376 kB
MemAvailable:      31116 kB
Buffers:            8448 kB
Cached:            22816 kB
SwapCached:            0 kB
Active:            28796 kB
Inactive:          15656 kB
Active(anon):      13528 kB
Inactive(anon):      124 kB
Active(file):      15268 kB
Inactive(file):    15532 kB
Unevictable:           0 kB
Mlocked:               0 kB
SwapTotal:             0 kB
SwapFree:              0 kB
Dirty:                64 kB
Writeback:             0 kB
AnonPages:         13212 kB
Mapped:             9472 kB
Shmem:               464 kB
Slab:               7184 kB
SReclaimable:       2816 kB
SUnreclaim:         4368 kB
KernelStack:         976 kB
PageTables:          552 kB
NFS_Unstable:          0 kB
Bounce:                0 kB
WritebackTmp:          0 kB
CommitLimit:       29828 kB
Committed_AS:     416396 kB
VmallocTotal:     966656 kB
VmallocUsed:           0 kB
VmallocChunk:          0 kB
```

MemAvailable为系统估算出来的当前系统可用内存。如果烤机过程中这个值持续降低，则出现了内存泄漏。此时需要进一步定位是否为主进程引起的内存泄漏。

### 3.2 /proc/pid/status

```bash
$ cat /proc/1019/status 
Name:   mainapp
Umask:  0000
State:  S (sleeping)
Tgid:   1019
Ngid:   0
Pid:    1019
PPid:   1
TracerPid:      0
Uid:    0       0       0       0
Gid:    0       0       0       0
FDSize: 256
Groups:  
NStgid: 1019
NSpid:  1019
NSpgid: 1017
NSsid:  1017
VmPeak:   335432 kB
VmSize:   335380 kB
VmLck:         0 kB
VmPin:         0 kB
VmHWM:     16464 kB
VmRSS:     16424 kB
RssAnon:            8432 kB
RssFile:            7992 kB
RssShmem:              0 kB
VmData:   304624 kB
VmStk:       132 kB
VmExe:      3172 kB
VmLib:      8360 kB
VmPTE:       116 kB
VmPMD:         0 kB
VmSwap:        0 kB
Threads:        39
SigQ:   1/464
SigPnd: 0000000000000000
ShdPnd: 0000000000000000
SigBlk: 0000000000000000
SigIgn: 0000000000001006
SigCgt: 0000000180014800
CapInh: 0000000000000000
CapPrm: 0000003fffffffff
CapEff: 0000003fffffffff
CapBnd: 0000003fffffffff
CapAmb: 0000000000000000
Cpus_allowed:   1
Cpus_allowed_list:      0
voluntary_ctxt_switches:        96
nonvoluntary_ctxt_switches:     50
```

VmSize：进程当前使用的虚拟内存大小。

VmRSS：进程实际占用的物理内存大小。

如果进程存在malloc之后没有free的情况，并且频繁发生，最直观的反映是VmSize会不断增加（这个大小和malloc具有直接关系）。VmRSS表示的物理内存占用也会不断增加。

### 3.3 /proc/pid/maps

```bash
$ cat /proc/1019/maps 
00010000-00329000 r-xp 00000000 1f:06 178        /fhrom/bin/mainapp
00339000-00341000 r--p 00319000 1f:06 178        /fhrom/bin/mainapp
00341000-00345000 rw-p 00321000 1f:06 178        /fhrom/bin/mainapp
00345000-00375000 rw-p 00000000 00:00 0          [heap]
00375000-00587000 rw-p 00000000 00:00 0          [heap]
a2d18000-a2d19000 ---p 00000000 00:00 0 
a2d19000-a2d58000 rw-p 00000000 00:00 0 
a2d58000-a2d59000 ---p 00000000 00:00 0 
a2d59000-a2dd8000 rw-p 00000000 00:00 0 
a2dd8000-a2dd9000 ---p 00000000 00:00 0 
a2dd9000-a2e58000 rw-p 00000000 00:00 0 
a2e58000-a2e59000 ---p 00000000 00:00 0 
a2e59000-a2ef9000 rw-p 00000000 00:00 0 
a2ef9000-a2efa000 ---p 00000000 00:00 0 
a2efa000-a2f19000 rw-p 00000000 00:00 0 
a2f19000-a2f1a000 ---p 00000000 00:00 0 
a2f1a000-a2f32000 rw-p 00000000 00:00 0 
a2f32000-a2f33000 ---p 00000000 00:00 0 
a2f33000-a37b3000 rw-p 00000000 00:00 0 
a37b3000-a37b4000 ---p 00000000 00:00 0 
a37b4000-a3fb3000 rw-p 00000000 00:00 0 
a3fb3000-a3fb4000 ---p 00000000 00:00 0 
a3fb4000-a47b3000 rw-p 00000000 00:00 0 
a47b3000-a47b4000 ---p 00000000 00:00 0 
a47b4000-a4fb3000 rw-p 00000000 00:00 0 
a4fb3000-a4fb4000 ---p 00000000 00:00 0 
a4fb4000-a57b3000 rw-p 00000000 00:00 0 
a57b3000-a57b4000 ---p 00000000 00:00 0 
a57b4000-a5fb3000 rw-p 00000000 00:00 0 
a5fb3000-a5fb4000 ---p 00000000 00:00 0 
a5fb4000-a67b3000 rw-p 00000000 00:00 0 
a67b3000-a67b4000 ---p 00000000 00:00 0 
a67b4000-a6fb3000 rw-p 00000000 00:00 0 
a6fb3000-a6fb4000 ---p 00000000 00:00 0 
a6fb4000-a77b3000 rw-p 00000000 00:00 0 
a77b3000-a77b4000 ---p 00000000 00:00 0 
a77b4000-a7fb3000 rw-p 00000000 00:00 0 
a7fb3000-a7fb4000 ---p 00000000 00:00 0 
a7fb4000-a87b3000 rw-p 00000000 00:00 0 
a87b3000-a87b4000 ---p 00000000 00:00 0 
a87b4000-a87b7000 rw-p 00000000 00:00 0 
a87b7000-a87b8000 ---p 00000000 00:00 0 
a87b8000-a8fb7000 rw-p 00000000 00:00 0 
a8fb7000-a8fb8000 ---p 00000000 00:00 0 
a8fb8000-a97b7000 rw-p 00000000 00:00 0 
a97b7000-a97b8000 ---p 00000000 00:00 0 
a97b8000-a9fb7000 rw-p 00000000 00:00 0 
a9fb7000-a9fb8000 ---p 00000000 00:00 0 
a9fb8000-aa7b7000 rw-p 00000000 00:00 0 
aa7b7000-aa7b8000 ---p 00000000 00:00 0 
aa7b8000-aafb7000 rw-p 00000000 00:00 0 
aafb7000-aafb8000 ---p 00000000 00:00 0 
aafb8000-ab7b7000 rw-p 00000000 00:00 0 
ab7b7000-ab7b8000 ---p 00000000 00:00 0 
ab7b8000-abfb7000 rw-p 00000000 00:00 0 
abfb7000-abfbc000 rw-s 47241000 00:06 215        /dev/mmz_userdev
abfbc000-abfbd000 ---p 00000000 00:00 0 
abfbd000-ac7bc000 rw-p 00000000 00:00 0 
ac7bc000-ac7bd000 ---p 00000000 00:00 0 
ac7bd000-acfbc000 rw-p 00000000 00:00 0 
acfbc000-acfbd000 ---p 00000000 00:00 0 
acfbd000-ad7bc000 rw-p 00000000 00:00 0 
ad7bc000-ad89d000 rw-s 4732c000 00:06 215        /dev/mmz_userdev
ad89d000-ad8a3000 rw-s 47245000 00:06 215        /dev/mmz_userdev
ad8a3000-adaa7000 rw-p 00000000 00:00 0 
adaa7000-adfed000 rw-s 46472000 00:06 225        /dev/venc
adfed000-adfee000 rw-s 46467000 00:06 215        /dev/mmz_userdev
adfee000-adff1000 rw-s 46464000 00:06 215        /dev/mmz_userdev
adff1000-adff2000 rw-s 46463000 00:06 215        /dev/mmz_userdev
adff2000-ae064000 rw-s 463f1000 00:06 215        /dev/mmz_userdev
ae064000-ae2d1000 rw-s 46184000 00:06 215        /dev/mmz_userdev
ae2d1000-ae2d2000 ---p 00000000 00:00 0 
ae2d2000-aead1000 rw-p 00000000 00:00 0 
aead1000-aebce000 rw-s 46087000 00:06 215        /dev/mmz_userdev
aebce000-aebcf000 rw-s 44092000 00:06 215        /dev/mmz_userdev
aebcf000-aebd0000 rw-s 44091000 00:06 215        /dev/mmz_userdev
aebd0000-aec45000 rw-s 46012000 00:06 215        /dev/mmz_userdev
aec45000-aec46000 rw-s 46011000 00:06 215        /dev/mmz_userdev
aec46000-aec47000 rw-s 46010000 00:06 215        /dev/mmz_userdev
aec47000-aec80000 rw-s 45fd7000 00:06 215        /dev/mmz_userdev
aec80000-aecb9000 rw-s 45f9e000 00:06 215        /dev/mmz_userdev
aecb9000-aed2d000 rw-s 45f62000 00:06 225        /dev/venc
aed2d000-aed2e000 ---p 00000000 00:00 0 
aed2e000-af52d000 rw-p 00000000 00:00 0 
af52d000-af52e000 ---p 00000000 00:00 0 
af52e000-afd2d000 rw-p 00000000 00:00 0 
afd2d000-afd2e000 ---p 00000000 00:00 0 
afd2e000-b052d000 rw-p 00000000 00:00 0 
b052d000-b0535000 rw-s 45f5a000 00:06 215        /dev/mmz_userdev
b0535000-b0536000 ---p 00000000 00:00 0 
b0536000-b0d35000 rw-p 00000000 00:00 0 
b0d35000-b0d36000 ---p 00000000 00:00 0 
b0d36000-b1535000 rw-p 00000000 00:00 0 
b1535000-b153d000 rw-s 45f52000 00:06 215        /dev/mmz_userdev
b153d000-b153e000 ---p 00000000 00:00 0 
b153e000-b1e45000 rw-p 00000000 00:00 0 
b1e45000-b1e46000 ---p 00000000 00:00 0 
b1e46000-b2645000 rw-p 00000000 00:00 0 
b2645000-b2671000 rw-s 45ef5000 00:06 215        /dev/mmz_userdev
b2671000-b2672000 ---p 00000000 00:00 0 
b2672000-b2e71000 rw-p 00000000 00:00 0 
b2e71000-b2e72000 ---p 00000000 00:00 0 
b2e72000-b3671000 rw-p 00000000 00:00 0 
b3671000-b3672000 ---p 00000000 00:00 0 
b3672000-b3e71000 rw-p 00000000 00:00 0 
b3e71000-b3e72000 ---p 00000000 00:00 0 
b3e72000-b4671000 rw-p 00000000 00:00 0 
b4671000-b4672000 ---p 00000000 00:00 0 
b4672000-b4e71000 rw-p 00000000 00:00 0 
b4e71000-b4e91000 rw-s 11020000 00:06 10         /dev/mem
b4e91000-b4e92000 rw-s 45ab0000 00:06 215        /dev/mmz_userdev
b4e92000-b4e94000 rw-s 45aae000 00:06 215        /dev/mmz_userdev
b4e94000-b4e9b000 rw-s 45aa7000 00:06 215        /dev/mmz_userdev
b4e9b000-b4e9c000 rw-s 45aa7000 00:06 215        /dev/mmz_userdev
b4e9c000-b4e9d000 rw-s 45aa6000 00:06 215        /dev/mmz_userdev
b4e9d000-b4e9e000 rw-s 45aa6000 00:06 215        /dev/mmz_userdev
b4e9e000-b4e9f000 rw-s 45aa6000 00:06 215        /dev/mmz_userdev
b4e9f000-b4eb8000 rw-s 45a8b000 00:06 218        /dev/sys
b4eb8000-b4f1c000 rw-s 45a19000 00:06 218        /dev/sys
b4f1c000-b4f2c000 rw-s 45a08000 00:06 215        /dev/mmz_userdev
b4f2c000-b4f2d000 ---p 00000000 00:00 0 
b4f2d000-b572c000 rw-p 00000000 00:00 0 
b572c000-b572d000 ---p 00000000 00:00 0 
b572d000-b5f2c000 rw-p 00000000 00:00 0 
b5f2c000-b5f2d000 r-xp 00000000 1f:07 86         /gokesdk/lib/libhi_vqe_wnr.so
b5f2d000-b5f3c000 ---p 00000000 00:00 0 
b5f3c000-b5f3d000 r--p 00000000 1f:07 86         /gokesdk/lib/libhi_vqe_wnr.so
b5f3d000-b5f3e000 rw-p 00001000 1f:07 86         /gokesdk/lib/libhi_vqe_wnr.so
b5f3e000-b5f3f000 r-xp 00000000 1f:07 84         /gokesdk/lib/libhi_vqe_res.so
b5f3f000-b5f4e000 ---p 00000000 00:00 0 
b5f4e000-b5f4f000 r--p 00000000 1f:07 84         /gokesdk/lib/libhi_vqe_res.so
b5f4f000-b5f50000 rw-p 00001000 1f:07 84         /gokesdk/lib/libhi_vqe_res.so
b5f50000-b5f51000 r-xp 00000000 1f:07 82         /gokesdk/lib/libhi_vqe_hpf.so
b5f51000-b5f60000 ---p 00000000 00:00 0 
b5f60000-b5f61000 r--p 00000000 1f:07 82         /gokesdk/lib/libhi_vqe_hpf.so
b5f61000-b5f62000 rw-p 00001000 1f:07 82         /gokesdk/lib/libhi_vqe_hpf.so
b5f62000-b5f63000 r-xp 00000000 1f:07 80         /gokesdk/lib/libhi_vqe_anr.so
b5f63000-b5f72000 ---p 00000000 00:00 0 
b5f72000-b5f73000 r--p 00000000 1f:07 80         /gokesdk/lib/libhi_vqe_anr.so
b5f73000-b5f74000 rw-p 00001000 1f:07 80         /gokesdk/lib/libhi_vqe_anr.so
b5f74000-b5f75000 r-xp 00000000 1f:07 78         /gokesdk/lib/libhi_vqe_aec.so
b5f75000-b5f84000 ---p 00000000 00:00 0 
b5f84000-b5f85000 r--p 00000000 1f:07 78         /gokesdk/lib/libhi_vqe_aec.so
b5f85000-b5f86000 rw-p 00001000 1f:07 78         /gokesdk/lib/libhi_vqe_aec.so
b5f86000-b5f87000 r-xp 00000000 1f:07 76         /gokesdk/lib/libhi_qr.so
b5f87000-b5f96000 ---p 00000000 00:00 0 
b5f96000-b5f97000 r--p 00000000 1f:07 76         /gokesdk/lib/libhi_qr.so
b5f97000-b5f98000 rw-p 00001000 1f:07 76         /gokesdk/lib/libhi_qr.so
b5f98000-b5f9a000 r-xp 00000000 1f:07 70         /gokesdk/lib/libhi_cipher.so
b5f9a000-b5fa9000 ---p 00000000 00:00 0 
b5fa9000-b5faa000 r--p 00001000 1f:07 70         /gokesdk/lib/libhi_cipher.so
b5faa000-b5fab000 rw-p 00002000 1f:07 70         /gokesdk/lib/libhi_cipher.so
b5fab000-b5fac000 r-xp 00000000 1f:07 65         /gokesdk/lib/libhi_aacsbrenc.so
b5fac000-b5fbb000 ---p 00000000 00:00 0 
b5fbb000-b5fbc000 r--p 00000000 1f:07 65         /gokesdk/lib/libhi_aacsbrenc.so
b5fbc000-b5fbd000 rw-p 00001000 1f:07 65         /gokesdk/lib/libhi_aacsbrenc.so
b5fbd000-b5fbe000 r-xp 00000000 1f:07 63         /gokesdk/lib/libhi_aacenc.so
b5fbe000-b5fcd000 ---p 00000000 00:00 0 
b5fcd000-b5fce000 r--p 00000000 1f:07 63         /gokesdk/lib/libhi_aacenc.so
b5fce000-b5fcf000 rw-p 00001000 1f:07 63         /gokesdk/lib/libhi_aacenc.so
b5fcf000-b5fd0000 r-xp 00000000 1f:07 85         /gokesdk/lib/libhi_vqe_talkv2.so
b5fd0000-b5fdf000 ---p 00000000 00:00 0 
b5fdf000-b5fe0000 r--p 00000000 1f:07 85         /gokesdk/lib/libhi_vqe_talkv2.so
b5fe0000-b5fe1000 rw-p 00001000 1f:07 85         /gokesdk/lib/libhi_vqe_talkv2.so
b5fe1000-b5fe2000 r-xp 00000000 1f:07 83         /gokesdk/lib/libhi_vqe_record.so
b5fe2000-b5ff1000 ---p 00000000 00:00 0 
b5ff1000-b5ff2000 r--p 00000000 1f:07 83         /gokesdk/lib/libhi_vqe_record.so
b5ff2000-b5ff3000 rw-p 00001000 1f:07 83         /gokesdk/lib/libhi_vqe_record.so
b5ff3000-b5ff4000 r-xp 00000000 1f:07 81         /gokesdk/lib/libhi_vqe_eq.so
b5ff4000-b6003000 ---p 00000000 00:00 0 
b6003000-b6004000 r--p 00000000 1f:07 81         /gokesdk/lib/libhi_vqe_eq.so
b6004000-b6005000 rw-p 00001000 1f:07 81         /gokesdk/lib/libhi_vqe_eq.so
b6005000-b6006000 r-xp 00000000 1f:07 79         /gokesdk/lib/libhi_vqe_agc.so
b6006000-b6015000 ---p 00000000 00:00 0 
b6015000-b6016000 r--p 00000000 1f:07 79         /gokesdk/lib/libhi_vqe_agc.so
b6016000-b6017000 rw-p 00001000 1f:07 79         /gokesdk/lib/libhi_vqe_agc.so
b6017000-b6019000 r-xp 00000000 1f:07 77         /gokesdk/lib/libhi_tde.so
b6019000-b6028000 ---p 00000000 00:00 0 
b6028000-b6029000 r--p 00001000 1f:07 77         /gokesdk/lib/libhi_tde.so
b6029000-b602a000 rw-p 00002000 1f:07 77         /gokesdk/lib/libhi_tde.so
b602a000-b602b000 r-xp 00000000 1f:07 69         /gokesdk/lib/libhi_bcd.so
b602b000-b603a000 ---p 00000000 00:00 0 
b603a000-b603b000 r--p 00000000 1f:07 69         /gokesdk/lib/libhi_bcd.so
b603b000-b603c000 rw-p 00001000 1f:07 69         /gokesdk/lib/libhi_bcd.so
b603c000-b603d000 r-xp 00000000 1f:07 64         /gokesdk/lib/libhi_aacsbrdec.so
b603d000-b604c000 ---p 00000000 00:00 0 
b604c000-b604d000 r--p 00000000 1f:07 64         /gokesdk/lib/libhi_aacsbrdec.so
b604d000-b604e000 rw-p 00001000 1f:07 64         /gokesdk/lib/libhi_aacsbrdec.so
b604e000-b604f000 r-xp 00000000 1f:07 62         /gokesdk/lib/libhi_aacdec.so
b604f000-b605e000 ---p 00000000 00:00 0 
b605e000-b605f000 r--p 00000000 1f:07 62         /gokesdk/lib/libhi_aacdec.so
b605f000-b6060000 rw-p 00001000 1f:07 62         /gokesdk/lib/libhi_aacdec.so
b6060000-b60e7000 r-xp 00000000 1f:07 108        /gokesdk/lib/libvqe_wnr.so
b60e7000-b60f6000 ---p 00000000 00:00 0 
b60f6000-b60f7000 r--p 00086000 1f:07 108        /gokesdk/lib/libvqe_wnr.so
b60f7000-b60f8000 rw-p 00087000 1f:07 108        /gokesdk/lib/libvqe_wnr.so
b60f8000-b6127000 r-xp 00000000 1f:07 107        /gokesdk/lib/libvqe_talkv2.so
b6127000-b6136000 ---p 00000000 00:00 0 
b6136000-b6137000 r--p 0002e000 1f:07 107        /gokesdk/lib/libvqe_talkv2.so
b6137000-b6138000 rw-p 0002f000 1f:07 107        /gokesdk/lib/libvqe_talkv2.so
b6138000-b615b000 r-xp 00000000 1f:07 106        /gokesdk/lib/libvqe_res.so
b615b000-b616a000 ---p 00000000 00:00 0 
b616a000-b616b000 r--p 00022000 1f:07 106        /gokesdk/lib/libvqe_res.so
b616b000-b616c000 rw-p 00023000 1f:07 106        /gokesdk/lib/libvqe_res.so
b616c000-b6194000 r-xp 00000000 1f:07 105        /gokesdk/lib/libvqe_record.so
b6194000-b61a4000 ---p 00000000 00:00 0 
b61a4000-b61a5000 r--p 00028000 1f:07 105        /gokesdk/lib/libvqe_record.so
b61a5000-b61a6000 rw-p 00029000 1f:07 105        /gokesdk/lib/libvqe_record.so
b61a6000-b61a8000 r-xp 00000000 1f:07 104        /gokesdk/lib/libvqe_hpf.so
b61a8000-b61b7000 ---p 00000000 00:00 0 
b61b7000-b61b8000 r--p 00001000 1f:07 104        /gokesdk/lib/libvqe_hpf.so
b61b8000-b61b9000 rw-p 00002000 1f:07 104        /gokesdk/lib/libvqe_hpf.so
b61b9000-b61c2000 r-xp 00000000 1f:07 103        /gokesdk/lib/libvqe_eq.so
b61c2000-b61d2000 ---p 00000000 00:00 0 
b61d2000-b61d3000 r--p 00009000 1f:07 103        /gokesdk/lib/libvqe_eq.so
b61d3000-b61d4000 rw-p 0000a000 1f:07 103        /gokesdk/lib/libvqe_eq.so
b61d4000-b61e0000 r-xp 00000000 1f:07 101        /gokesdk/lib/libvqe_anr.so
b61e0000-b61ef000 ---p 00000000 00:00 0 
b61ef000-b61f0000 r--p 0000b000 1f:07 101        /gokesdk/lib/libvqe_anr.so
b61f0000-b61f1000 rw-p 0000c000 1f:07 101        /gokesdk/lib/libvqe_anr.so
b61f1000-b61fd000 r-xp 00000000 1f:07 100        /gokesdk/lib/libvqe_agc.so
b61fd000-b620c000 ---p 00000000 00:00 0 
b620c000-b620d000 r--p 0000b000 1f:07 100        /gokesdk/lib/libvqe_agc.so
b620d000-b620e000 rw-p 0000c000 1f:07 100        /gokesdk/lib/libvqe_agc.so
b620e000-b621c000 r-xp 00000000 1f:07 99         /gokesdk/lib/libvqe_aec.so
b621c000-b622b000 ---p 00000000 00:00 0 
b622b000-b622c000 r--p 0000d000 1f:07 99         /gokesdk/lib/libvqe_aec.so
b622c000-b622d000 rw-p 0000e000 1f:07 99         /gokesdk/lib/libvqe_aec.so
b622d000-b6231000 r-xp 00000000 1f:07 61         /gokesdk/lib/libgk_tde.so
b6231000-b6240000 ---p 00000000 00:00 0 
b6240000-b6241000 r--p 00003000 1f:07 61         /gokesdk/lib/libgk_tde.so
b6241000-b6242000 rw-p 00004000 1f:07 61         /gokesdk/lib/libgk_tde.so
b6242000-b624d000 r-xp 00000000 1f:07 60         /gokesdk/lib/libgk_qr.so
b624d000-b625c000 ---p 00000000 00:00 0 
b625c000-b625d000 r--p 0000a000 1f:07 60         /gokesdk/lib/libgk_qr.so
b625d000-b625e000 rw-p 0000b000 1f:07 60         /gokesdk/lib/libgk_qr.so
b625e000-b6268000 r-xp 00000000 1f:07 55         /gokesdk/lib/libgk_cipher.so
b6268000-b6277000 ---p 00000000 00:00 0 
b6277000-b6278000 r--p 00009000 1f:07 55         /gokesdk/lib/libgk_cipher.so
b6278000-b6279000 rw-p 0000a000 1f:07 55         /gokesdk/lib/libgk_cipher.so
b6279000-b628b000 r-xp 00000000 1f:07 54         /gokesdk/lib/libgk_bcd.so
b628b000-b629a000 ---p 00000000 00:00 0 
b629a000-b629b000 r--p 00011000 1f:07 54         /gokesdk/lib/libgk_bcd.so
b629b000-b629f000 rw-p 00012000 1f:07 54         /gokesdk/lib/libgk_bcd.so
b629f000-b62bd000 r-xp 00000000 1f:07 41         /gokesdk/lib/libaac_sbr_enc.so
b62bd000-b62cc000 ---p 00000000 00:00 0 
b62cc000-b62cd000 r--p 0001d000 1f:07 41         /gokesdk/lib/libaac_sbr_enc.so
b62cd000-b62ce000 rw-p 0001e000 1f:07 41         /gokesdk/lib/libaac_sbr_enc.so
b62ce000-b62e7000 r-xp 00000000 1f:07 40         /gokesdk/lib/libaac_sbr_dec.so
b62e7000-b62f6000 ---p 00000000 00:00 0 
b62f6000-b62f7000 r--p 00018000 1f:07 40         /gokesdk/lib/libaac_sbr_dec.so
b62f7000-b62f8000 rw-p 00019000 1f:07 40         /gokesdk/lib/libaac_sbr_dec.so
b62f8000-b6330000 r-xp 00000000 1f:07 39         /gokesdk/lib/libaac_enc.so
b6330000-b633f000 ---p 00000000 00:00 0 
b633f000-b6340000 r--p 00037000 1f:07 39         /gokesdk/lib/libaac_enc.so
b6340000-b6341000 rw-p 00038000 1f:07 39         /gokesdk/lib/libaac_enc.so
b6341000-b636a000 r-xp 00000000 1f:07 38         /gokesdk/lib/libaac_dec.so
b636a000-b637a000 ---p 00000000 00:00 0 
b637a000-b637b000 r--p 00029000 1f:07 38         /gokesdk/lib/libaac_dec.so
b637b000-b637c000 rw-p 0002a000 1f:07 38         /gokesdk/lib/libaac_dec.so
b637c000-b6395000 r-xp 00000000 1f:07 37         /gokesdk/lib/libaac_comm.so
b6395000-b63a4000 ---p 00000000 00:00 0 
b63a4000-b63a5000 r--p 00018000 1f:07 37         /gokesdk/lib/libaac_comm.so
b63a5000-b63a6000 rw-p 00019000 1f:07 37         /gokesdk/lib/libaac_comm.so
b63a6000-b63ae000 r-xp 00000000 1f:06 265        /fhrom/lib/libfhfile.so
b63ae000-b63bd000 ---p 00000000 00:00 0 
b63bd000-b63be000 r--p 00007000 1f:06 265        /fhrom/lib/libfhfile.so
b63be000-b63bf000 rw-p 00008000 1f:06 265        /fhrom/lib/libfhfile.so
b63bf000-b63cb000 r-xp 00000000 1f:06 263        /fhrom/lib/libfhdev.so
b63cb000-b63db000 ---p 00000000 00:00 0 
b63db000-b63dc000 r--p 0000c000 1f:06 263        /fhrom/lib/libfhdev.so
b63dc000-b63dd000 rw-p 0000d000 1f:06 263        /fhrom/lib/libfhdev.so
b63dd000-b63e0000 r-xp 00000000 1f:06 343        /lib/libdl-0.9.33.2.so
b63e0000-b63ef000 ---p 00000000 00:00 0 
b63ef000-b63f0000 r--p 00002000 1f:06 343        /lib/libdl-0.9.33.2.so
b63f0000-b63f1000 rw-p 00003000 1f:06 343        /lib/libdl-0.9.33.2.so
b63f1000-b6488000 r-xp 00000000 1f:06 370        /lib/libuClibc-0.9.33.2.so
b6488000-b6497000 ---p 00000000 00:00 0 
b6497000-b6498000 r--p 00096000 1f:06 370        /lib/libuClibc-0.9.33.2.so
b6498000-b6499000 rw-p 00097000 1f:06 370        /lib/libuClibc-0.9.33.2.so
b6499000-b649e000 rw-p 00000000 00:00 0 
b649e000-b64a7000 r-xp 00000000 1f:06 348        /lib/libgcc_s.so.1
b64a7000-b64b6000 ---p 00000000 00:00 0 
b64b6000-b64b7000 r--p 00008000 1f:06 348        /lib/libgcc_s.so.1
b64b7000-b64b8000 rw-p 00009000 1f:06 348        /lib/libgcc_s.so.1
b64b8000-b64c5000 r-xp 00000000 1f:06 355        /lib/libm-0.9.33.2.so
b64c5000-b64d4000 ---p 00000000 00:00 0 
b64d4000-b64d5000 r--p 0000c000 1f:06 355        /lib/libm-0.9.33.2.so
b64d5000-b64d6000 rw-p 0000d000 1f:06 355        /lib/libm-0.9.33.2.so
b64d6000-b64d8000 r-xp 00000000 1f:06 254        /fhrom/lib/libfh_file_opt.so
b64d8000-b64e7000 ---p 00000000 00:00 0 
b64e7000-b64e8000 r--p 00001000 1f:06 254        /fhrom/lib/libfh_file_opt.so
b64e8000-b64e9000 rw-p 00002000 1f:06 254        /fhrom/lib/libfh_file_opt.so
b64e9000-b6507000 r-xp 00000000 1f:06 278        /fhrom/lib/librecorder.so
b6507000-b6516000 ---p 00000000 00:00 0 
b6516000-b6517000 r--p 0001d000 1f:06 278        /fhrom/lib/librecorder.so
b6517000-b6518000 rw-p 0001e000 1f:06 278        /fhrom/lib/librecorder.so
b6518000-b654a000 r-xp 00000000 1f:07 89         /gokesdk/lib/libiconv.so.2.1.0
b654a000-b6559000 ---p 00000000 00:00 0 
b6559000-b655a000 r--p 00031000 1f:07 89         /gokesdk/lib/libiconv.so.2.1.0
b655a000-b655b000 rw-p 00032000 1f:07 89         /gokesdk/lib/libiconv.so.2.1.0
b655b000-b655c000 r-xp 00000000 1f:06 257        /fhrom/lib/libfh_led_ctrl.so
b655c000-b656b000 ---p 00000000 00:00 0 
b656b000-b656c000 r--p 00000000 1f:06 257        /fhrom/lib/libfh_led_ctrl.so
b656c000-b656d000 rw-p 00001000 1f:06 257        /fhrom/lib/libfh_led_ctrl.so
b656d000-b6571000 r-xp 00000000 1f:06 258        /fhrom/lib/libfh_pwm_ctrl.so
b6571000-b6580000 ---p 00000000 00:00 0 
b6580000-b6581000 r--p 00003000 1f:06 258        /fhrom/lib/libfh_pwm_ctrl.so
b6581000-b6582000 rw-p 00004000 1f:06 258        /fhrom/lib/libfh_pwm_ctrl.so
b6582000-b6583000 r-xp 00000000 1f:07 90         /gokesdk/lib/libir_auto.so
b6583000-b6592000 ---p 00000000 00:00 0 
b6592000-b6593000 r--p 00000000 1f:07 90         /gokesdk/lib/libir_auto.so
b6593000-b6594000 rw-p 00001000 1f:07 90         /gokesdk/lib/libir_auto.so
b6594000-b65bf000 r-xp 00000000 1f:07 50         /gokesdk/lib/libgk_ae.so
b65bf000-b65cf000 ---p 00000000 00:00 0 
b65cf000-b65d0000 r--p 0002b000 1f:07 50         /gokesdk/lib/libgk_ae.so
b65d0000-b65d1000 rw-p 0002c000 1f:07 50         /gokesdk/lib/libgk_ae.so
b65d1000-b65de000 rw-p 00000000 00:00 0 
b65de000-b65f0000 r-xp 00000000 1f:07 52         /gokesdk/lib/libgk_awb.so
b65f0000-b65ff000 ---p 00000000 00:00 0 
b65ff000-b6600000 r--p 00011000 1f:07 52         /gokesdk/lib/libgk_awb.so
b6600000-b6601000 rw-p 00012000 1f:07 52         /gokesdk/lib/libgk_awb.so
b6601000-b6603000 rw-p 00000000 00:00 0 
b6603000-b6653000 r-xp 00000000 1f:07 56         /gokesdk/lib/libgk_isp.so
b6653000-b6663000 ---p 00000000 00:00 0 
b6663000-b6664000 r--p 00050000 1f:07 56         /gokesdk/lib/libgk_isp.so
b6664000-b6667000 rw-p 00051000 1f:07 56         /gokesdk/lib/libgk_isp.so
b6667000-b666d000 rw-p 00000000 00:00 0 
b666d000-b667a000 r-xp 00000000 1f:06 253        /fhrom/lib/libfaac.so.0.0.0
b667a000-b6689000 ---p 00000000 00:00 0 
b6689000-b668a000 r--p 0000c000 1f:06 253        /fhrom/lib/libfaac.so.0.0.0
b668a000-b668d000 rw-p 0000d000 1f:06 253        /fhrom/lib/libfaac.so.0.0.0
b668d000-b6690000 r-xp 00000000 1f:07 59         /gokesdk/lib/libgk_md.so
b6690000-b669f000 ---p 00000000 00:00 0 
b669f000-b66a0000 r--p 00002000 1f:07 59         /gokesdk/lib/libgk_md.so
b66a0000-b66a1000 rw-p 00003000 1f:07 59         /gokesdk/lib/libgk_md.so
b66a1000-b66a5000 rw-p 00000000 00:00 0 
b66a5000-b66b5000 r-xp 00000000 1f:07 58         /gokesdk/lib/libgk_ivp.so
b66b5000-b66c4000 ---p 00000000 00:00 0 
b66c4000-b66c5000 r--p 0000f000 1f:07 58         /gokesdk/lib/libgk_ivp.so
b66c5000-b66c6000 rw-p 00010000 1f:07 58         /gokesdk/lib/libgk_ivp.so
b66c6000-b66c8000 rw-p 00000000 00:00 0 
b66c8000-b66d3000 r-xp 00000000 1f:07 57         /gokesdk/lib/libgk_ive.so
b66d3000-b66e2000 ---p 00000000 00:00 0 
b66e2000-b66e3000 r--p 0000a000 1f:07 57         /gokesdk/lib/libgk_ive.so
b66e3000-b66e4000 rw-p 0000b000 1f:07 57         /gokesdk/lib/libgk_ive.so
b66e4000-b6719000 r-xp 00000000 1f:07 51         /gokesdk/lib/libgk_api.so
b6719000-b6729000 ---p 00000000 00:00 0 
b6729000-b672a000 r--p 00035000 1f:07 51         /gokesdk/lib/libgk_api.so
b672a000-b672b000 rw-p 00036000 1f:07 51         /gokesdk/lib/libgk_api.so
b672b000-b6738000 rw-p 00000000 00:00 0 
b6738000-b677d000 r-xp 00000000 1f:06 256        /fhrom/lib/libfh_isp_hw_adp.so
b677d000-b678c000 ---p 00000000 00:00 0 
b678c000-b678d000 r--p 00044000 1f:06 256        /fhrom/lib/libfh_isp_hw_adp.so
b678d000-b678e000 rw-p 00045000 1f:06 256        /fhrom/lib/libfh_isp_hw_adp.so
b678e000-b6815000 rw-p 00000000 00:00 0 
b6815000-b681c000 r-xp 00000000 1f:06 255        /fhrom/lib/libfh_isp_api.so
b681c000-b682b000 ---p 00000000 00:00 0 
b682b000-b682c000 r--p 00006000 1f:06 255        /fhrom/lib/libfh_isp_api.so
b682c000-b682d000 rw-p 00007000 1f:06 255        /fhrom/lib/libfh_isp_api.so
b682d000-b6877000 r-xp 00000000 1f:07 49         /gokesdk/lib/libfreetype.so.6.17.2
b6877000-b6887000 ---p 00000000 00:00 0 
b6887000-b6888000 r--p 0004a000 1f:07 49         /gokesdk/lib/libfreetype.so.6.17.2
b6888000-b6889000 rw-p 0004b000 1f:07 49         /gokesdk/lib/libfreetype.so.6.17.2
b6889000-b688a000 r-xp 00000000 1f:07 73         /gokesdk/lib/libhi_ivp.so
b688a000-b6899000 ---p 00000000 00:00 0 
b6899000-b689a000 r--p 00000000 1f:07 73         /gokesdk/lib/libhi_ivp.so
b689a000-b689b000 rw-p 00001000 1f:07 73         /gokesdk/lib/libhi_ivp.so
b689b000-b689f000 r-xp 00000000 1f:06 241        /fhrom/lib/libaudiovqe.so
b689f000-b68ae000 ---p 00000000 00:00 0 
b68ae000-b68af000 r--p 00003000 1f:06 241        /fhrom/lib/libaudiovqe.so
b68af000-b68b0000 rw-p 00004000 1f:06 241        /fhrom/lib/libaudiovqe.so
b68b0000-b692b000 r-xp 00000000 1f:06 276        /fhrom/lib/libmedia_server.so
b692b000-b693a000 ---p 00000000 00:00 0 
b693a000-b693b000 r--p 0007a000 1f:06 276        /fhrom/lib/libmedia_server.so
b693b000-b693d000 rw-p 0007b000 1f:06 276        /fhrom/lib/libmedia_server.so
b693d000-b6954000 r-xp 00000000 1f:06 273        /fhrom/lib/libipc_media_api_cmcc.so
b6954000-b6963000 ---p 00000000 00:00 0 
b6963000-b6964000 r--p 00016000 1f:06 273        /fhrom/lib/libipc_media_api_cmcc.so
b6964000-b6965000 rw-p 00017000 1f:06 273        /fhrom/lib/libipc_media_api_cmcc.so
b6965000-b6966000 rw-p 00000000 00:00 0 
b6966000-b6967000 r-xp 00000000 1f:07 74         /gokesdk/lib/libhi_md.so
b6967000-b6976000 ---p 00000000 00:00 0 
b6976000-b6977000 r--p 00000000 1f:07 74         /gokesdk/lib/libhi_md.so
b6977000-b6978000 rw-p 00001000 1f:07 74         /gokesdk/lib/libhi_md.so
b6978000-b6979000 r-xp 00000000 1f:07 68         /gokesdk/lib/libhi_awb_natura.so
b6979000-b6988000 ---p 00000000 00:00 0 
b6988000-b6989000 r--p 00000000 1f:07 68         /gokesdk/lib/libhi_awb_natura.so
b6989000-b698a000 rw-p 00001000 1f:07 68         /gokesdk/lib/libhi_awb_natura.so
b698a000-b698c000 r-xp 00000000 1f:07 72         /gokesdk/lib/libhi_ive.so
b698c000-b699b000 ---p 00000000 00:00 0 
b699b000-b699c000 r--p 00001000 1f:07 72         /gokesdk/lib/libhi_ive.so
b699c000-b699d000 rw-p 00002000 1f:07 72         /gokesdk/lib/libhi_ive.so
b699d000-b69ac000 r-xp 00000000 1f:07 95         /gokesdk/lib/libsecurec.so
b69ac000-b69bc000 ---p 00000000 00:00 0 
b69bc000-b69bd000 r--p 0000f000 1f:07 95         /gokesdk/lib/libsecurec.so
b69bd000-b69be000 rw-p 00010000 1f:07 95         /gokesdk/lib/libsecurec.so
b69be000-b69c3000 r-xp 00000000 1f:07 45         /gokesdk/lib/libdnvqe.so
b69c3000-b69d2000 ---p 00000000 00:00 0 
b69d2000-b69d3000 r--p 00004000 1f:07 45         /gokesdk/lib/libdnvqe.so
b69d3000-b69d4000 rw-p 00005000 1f:07 45         /gokesdk/lib/libdnvqe.so
b69d4000-b69db000 r-xp 00000000 1f:07 97         /gokesdk/lib/libupvqe.so
b69db000-b69ea000 ---p 00000000 00:00 0 
b69ea000-b69eb000 r--p 00006000 1f:07 97         /gokesdk/lib/libupvqe.so
b69eb000-b69ec000 rw-p 00007000 1f:07 97         /gokesdk/lib/libupvqe.so
b69ec000-b69f2000 r-xp 00000000 1f:07 98         /gokesdk/lib/libvoice_engine.so
b69f2000-b6a02000 ---p 00000000 00:00 0 
b6a02000-b6a03000 r--p 00006000 1f:07 98         /gokesdk/lib/libvoice_engine.so
b6a03000-b6a04000 rw-p 00007000 1f:07 98         /gokesdk/lib/libvoice_engine.so
b6a04000-b6a0c000 r-xp 00000000 1f:06 279        /fhrom/lib/libsns_gc4023.so
b6a0c000-b6a1b000 ---p 00000000 00:00 0 
b6a1b000-b6a1c000 r--p 00007000 1f:06 279        /fhrom/lib/libsns_gc4023.so
b6a1c000-b6a1d000 rw-p 00008000 1f:06 279        /fhrom/lib/libsns_gc4023.so
b6a1d000-b6a24000 r-xp 00000000 1f:07 96         /gokesdk/lib/libsns_gc4653_2l.so
b6a24000-b6a33000 ---p 00000000 00:00 0 
b6a33000-b6a34000 r--p 00006000 1f:07 96         /gokesdk/lib/libsns_gc4653_2l.so
b6a34000-b6a35000 rw-p 00007000 1f:07 96         /gokesdk/lib/libsns_gc4653_2l.so
b6a35000-b6a36000 r-xp 00000000 1f:07 67         /gokesdk/lib/libhi_awb.so
b6a36000-b6a45000 ---p 00000000 00:00 0 
b6a45000-b6a46000 r--p 00000000 1f:07 67         /gokesdk/lib/libhi_awb.so
b6a46000-b6a47000 rw-p 00001000 1f:07 67         /gokesdk/lib/libhi_awb.so
b6a47000-b6a4b000 r-xp 00000000 1f:07 91         /gokesdk/lib/libldci.so
b6a4b000-b6a5a000 ---p 00000000 00:00 0 
b6a5a000-b6a5b000 r--p 00003000 1f:07 91         /gokesdk/lib/libldci.so
b6a5b000-b6a5c000 rw-p 00004000 1f:07 91         /gokesdk/lib/libldci.so
b6a5c000-b6a63000 r-xp 00000000 1f:07 44         /gokesdk/lib/libdehaze.so
b6a63000-b6a72000 ---p 00000000 00:00 0 
b6a72000-b6a73000 r--p 00006000 1f:07 44         /gokesdk/lib/libdehaze.so
b6a73000-b6a74000 rw-p 00007000 1f:07 44         /gokesdk/lib/libdehaze.so
b6a74000-b6a78000 r-xp 00000000 1f:07 71         /gokesdk/lib/libhi_isp.so
b6a78000-b6a88000 ---p 00000000 00:00 0 
b6a88000-b6a89000 r--p 00004000 1f:07 71         /gokesdk/lib/libhi_isp.so
b6a89000-b6a8a000 rw-p 00005000 1f:07 71         /gokesdk/lib/libhi_isp.so
b6a8a000-b6a8c000 r-xp 00000000 1f:07 66         /gokesdk/lib/libhi_ae.so
b6a8c000-b6a9b000 ---p 00000000 00:00 0 
b6a9b000-b6a9c000 r--p 00001000 1f:07 66         /gokesdk/lib/libhi_ae.so
b6a9c000-b6a9d000 rw-p 00002000 1f:07 66         /gokesdk/lib/libhi_ae.so
b6a9d000-b6aae000 r-xp 00000000 1f:07 75         /gokesdk/lib/libhi_mpi.so
b6aae000-b6abd000 ---p 00000000 00:00 0 
b6abd000-b6abe000 r--p 00010000 1f:07 75         /gokesdk/lib/libhi_mpi.so
b6abe000-b6abf000 rw-p 00011000 1f:07 75         /gokesdk/lib/libhi_mpi.so
b6abf000-b6ac7000 r-xp 00000000 1f:07 46         /gokesdk/lib/libdrc.so
b6ac7000-b6ad6000 ---p 00000000 00:00 0 
b6ad6000-b6ad7000 r--p 00007000 1f:07 46         /gokesdk/lib/libdrc.so
b6ad7000-b6ad8000 rw-p 00008000 1f:07 46         /gokesdk/lib/libdrc.so
b6ad8000-b6ae9000 r-xp 00000000 1f:06 259        /fhrom/lib/libfhapi.so
b6ae9000-b6af8000 ---p 00000000 00:00 0 
b6af8000-b6af9000 r--p 00010000 1f:06 259        /fhrom/lib/libfhapi.so
b6af9000-b6afa000 rw-p 00011000 1f:06 259        /fhrom/lib/libfhapi.so
b6afa000-b6b8c000 r-xp 00000000 1f:06 378        /lib/libwolfssl.so.24.1.0
b6b8c000-b6b9b000 ---p 00000000 00:00 0 
b6b9b000-b6b9d000 r--p 00091000 1f:06 378        /lib/libwolfssl.so.24.1.0
b6b9d000-b6b9e000 rw-p 00093000 1f:06 378        /lib/libwolfssl.so.24.1.0
b6b9e000-b6c37000 rw-p 00000000 00:00 0 
b6c37000-b6c3b000 r-xp 00000000 1f:06 262        /fhrom/lib/libfhcrypto.so
b6c3b000-b6c4a000 ---p 00000000 00:00 0 
b6c4a000-b6c4b000 r--p 00003000 1f:06 262        /fhrom/lib/libfhcrypto.so
b6c4b000-b6c4c000 rw-p 00004000 1f:06 262        /fhrom/lib/libfhcrypto.so
b6c4c000-b6c66000 r-xp 00000000 1f:06 260        /fhrom/lib/libfhcfg.so
b6c66000-b6c75000 ---p 00000000 00:00 0 
b6c75000-b6c76000 r--p 00019000 1f:06 260        /fhrom/lib/libfhcfg.so
b6c76000-b6c77000 rw-p 0001a000 1f:06 260        /fhrom/lib/libfhcfg.so
b6c77000-b6c7f000 r-xp 00000000 1f:06 274        /fhrom/lib/libled_interface.so
b6c7f000-b6c8e000 ---p 00000000 00:00 0 
b6c8e000-b6c8f000 r--p 00007000 1f:06 274        /fhrom/lib/libled_interface.so
b6c8f000-b6c90000 rw-p 00008000 1f:06 274        /fhrom/lib/libled_interface.so
b6c90000-b6cab000 r-xp 00000000 1f:06 345        /lib/libfhdrv_kdrv_board.so
b6cab000-b6cba000 ---p 00000000 00:00 0 
b6cba000-b6cbb000 r--p 0001a000 1f:06 345        /lib/libfhdrv_kdrv_board.so
b6cbb000-b6cbc000 rw-p 0001b000 1f:06 345        /lib/libfhdrv_kdrv_board.so
b6cbc000-b6cc4000 r-xp 00000000 1f:06 269        /fhrom/lib/libfhsysmgr.so
b6cc4000-b6cd3000 ---p 00000000 00:00 0 
b6cd3000-b6cd4000 r--p 00007000 1f:06 269        /fhrom/lib/libfhsysmgr.so
b6cd4000-b6cd5000 rw-p 00008000 1f:06 269        /fhrom/lib/libfhsysmgr.so
b6cd5000-b6cdc000 r-xp 00000000 1f:06 373        /lib/libuci.so
b6cdc000-b6ceb000 ---p 00000000 00:00 0 
b6ceb000-b6cec000 r--p 00006000 1f:06 373        /lib/libuci.so
b6cec000-b6ced000 rw-p 00007000 1f:06 373        /lib/libuci.so
b6ced000-b6cfe000 r-xp 00000000 1f:06 371        /lib/libubox.so
b6cfe000-b6d0d000 ---p 00000000 00:00 0 
b6d0d000-b6d0e000 r--p 00010000 1f:06 371        /lib/libubox.so
b6d0e000-b6d0f000 rw-p 00011000 1f:06 371        /lib/libubox.so
b6d0f000-b6d13000 r-xp 00000000 1f:06 372        /lib/libubus.so
b6d13000-b6d22000 ---p 00000000 00:00 0 
b6d22000-b6d23000 r--p 00003000 1f:06 372        /lib/libubus.so
b6d23000-b6d24000 rw-p 00004000 1f:06 372        /lib/libubus.so
b6d24000-b6d2b000 r-xp 00000000 1f:06 270        /fhrom/lib/libfhubus.so
b6d2b000-b6d3b000 ---p 00000000 00:00 0 
b6d3b000-b6d3c000 r--p 00007000 1f:06 270        /fhrom/lib/libfhubus.so
b6d3c000-b6d3d000 rw-p 00008000 1f:06 270        /fhrom/lib/libfhubus.so
b6d3d000-b6d45000 r-xp 00000000 1f:06 351        /lib/libjson-c.so.2.0.1
b6d45000-b6d54000 ---p 00000000 00:00 0 
b6d54000-b6d55000 r--p 00007000 1f:06 351        /lib/libjson-c.so.2.0.1
b6d55000-b6d56000 rw-p 00008000 1f:06 351        /lib/libjson-c.so.2.0.1
b6d56000-b6d57000 r-xp 00000000 1f:06 354        /lib/libjson.so.0.1.0
b6d57000-b6d66000 ---p 00000000 00:00 0 
b6d66000-b6d67000 r--p 00000000 1f:06 354        /lib/libjson.so.0.1.0
b6d67000-b6d68000 rw-p 00001000 1f:06 354        /lib/libjson.so.0.1.0
b6d68000-b6d6b000 r-xp 00000000 1f:06 267        /fhrom/lib/libfhlogmgr.so
b6d6b000-b6d7a000 ---p 00000000 00:00 0 
b6d7a000-b6d7b000 r--p 00002000 1f:06 267        /fhrom/lib/libfhlogmgr.so
b6d7b000-b6d7c000 rw-p 00003000 1f:06 267        /fhrom/lib/libfhlogmgr.so
b6d7c000-b6eb4000 r-xp 00000000 1f:06 367        /lib/libstdc++.so.6.0.23
b6eb4000-b6ec4000 ---p 00000000 00:00 0 
b6ec4000-b6eca000 r--p 00138000 1f:06 367        /lib/libstdc++.so.6.0.23
b6eca000-b6ecb000 rw-p 0013e000 1f:06 367        /lib/libstdc++.so.6.0.23
b6ecb000-b6ecd000 rw-p 00000000 00:00 0 
b6ecd000-b6ee2000 r-xp 00000000 1f:06 359        /lib/libpthread-0.9.33.2.so
b6ee2000-b6ef1000 ---p 00000000 00:00 0 
b6ef1000-b6ef2000 r--p 00014000 1f:06 359        /lib/libpthread-0.9.33.2.so
b6ef2000-b6ef3000 rw-p 00015000 1f:06 359        /lib/libpthread-0.9.33.2.so
b6ef3000-b6ef5000 rw-p 00000000 00:00 0 
b6ef5000-b6ef8000 r-xp 00000000 1f:06 363        /lib/librt-0.9.33.2.so
b6ef8000-b6f07000 ---p 00000000 00:00 0 
b6f07000-b6f08000 r--p 00002000 1f:06 363        /lib/librt-0.9.33.2.so
b6f08000-b6f09000 rw-p 00003000 1f:06 363        /lib/librt-0.9.33.2.so
b6f09000-b6f10000 r-xp 00000000 1f:06 338        /lib/ld-uClibc-0.9.33.2.so
b6f10000-b6f11000 rw-s 45aa6000 00:06 215        /dev/mmz_userdev
b6f11000-b6f12000 rw-s 45aa6000 00:06 215        /dev/mmz_userdev
b6f12000-b6f14000 rw-s 45aa4000 00:06 215        /dev/mmz_userdev
b6f14000-b6f1f000 rw-p 00000000 00:00 0 
b6f1f000-b6f20000 r--p 00006000 1f:06 338        /lib/ld-uClibc-0.9.33.2.so
b6f20000-b6f21000 rw-p 00007000 1f:06 338        /lib/ld-uClibc-0.9.33.2.so
bec5e000-bec7f000 rw-p 00000000 00:00 0          [stack]
bef16000-bef17000 r-xp 00000000 00:00 0          [sigpage]
bef17000-bef18000 r--p 00000000 00:00 0          [vvar]
bef18000-bef19000 r-xp 00000000 00:00 0          [vdso]
ffff0000-ffff1000 r-xp 00000000 00:00 0          [vectors]
```

对比内存泄漏前后的maps内容，可以对内存泄漏出现的原因有一定了解。maps的内容和程序环境有关（C库，线程库等）。

```bash
$ cat /proc/1019/maps 
00010000-00329000 r-xp 00000000 1f:06 178        /fhrom/bin/mainapp
00339000-00341000 r--p 00319000 1f:06 178        /fhrom/bin/mainapp
00341000-00345000 rw-p 00321000 1f:06 178        /fhrom/bin/mainapp
00345000-00375000 rw-p 00000000 00:00 0          [heap]
00375000-00587000 rw-p 00000000 00:00 0          [heap]
a2d18000-a2d19000 ---p 00000000 00:00 0 
a2d19000-a2d58000 rw-p 00000000 00:00 0 
a2d58000-a2d59000 ---p 00000000 00:00 0 
a2d59000-a2dd8000 rw-p 00000000 00:00 0 
a2dd8000-a2dd9000 ---p 00000000 00:00 0 
```

如果程序中存在比较小的内存分配的泄漏，一般表现为heap的大小不断增加（调用brk分配内存）。如果程序中存在比较大的内存分配的泄漏（比如1M以上），一般表现为heap以下地址空间会出现新的地址段（调用mmap分配内存）。如果程序中存在线程泄漏，也表现为heap以下地址空间会出现新的地址段。

### 3.4 一个示例脚本

以下是一个用来监测内存的脚本示例。

```bash
#!/bin/sh

TMP_FILE="/tmp/ps.tmp"
PROCESS_NAME="mainapp"

ps > $TMP_FILE

v_process=$(cat $TMP_FILE | grep $PROCESS_NAME | awk '{print $1}')

while true
do
        date
        cat /proc/$v_process/maps
        cat /proc/$v_process/status
        cat /proc/meminfo
        top -b -n 1
        netstat -an
        sleep 10
done

```

设备运行时，可以在监测电脑上通过telnet登录到设备，并执行该脚本。将脚本的输出写到文件中，用于后续分析。

### 3.5 数据处理

泄漏速度较慢并且有一定触发条件的内存泄漏需要较长时间的挂机，此时内存检测脚本会产生大量的数据输出。需要对数据进行一定的处理以得到需要的信息。

#### 3.5.1 例子：提取MemAvailable

可以结合linux命令行工具和excel进行数据处理（如果数据量不是很大，用notepad++的搜索功能和excel的替换功能可以替代linux命令行工具）。

假设要获取日志中的/proc/meminfo中的MemAvailable变化情况，处理步骤如下：

```bash
# 将日志文件top.log拷贝到linux系统中
$ ls
top.log

# top.log中有大量的cat /proc/meminfo输出的信息
$ cat top.log
...
MemTotal:          59628 kB
MemFree:            1272 kB
MemAvailable:      26652 kB
Buffers:            1712 kB
Cached:            24924 kB
SwapCached:            0 kB
Active:            29000 kB
...

# 提取日志文件中的MemAvailable行中的值
cat top.log | grep MemAvai | sed -E 's/[^0-9]*([0-9]+).*/\1/g' > memavai.txt

# 将memavai.txt中的数据拷贝到excel中，然后以这些数据为源生成图表
```

## 4. ASan

### 4.1 已有文档

《ASAN工具使用.docx》

### 4.2 介绍

全称：Address Sanitizer；

译名：地址消毒器、地址完整性检查工具（没有找到译名）；

来源：LLVM的sanitizers项目；https://github.com/google/sanitizers；目前gcc、vc、linux kernel均已引入了这个项目中的技术

演示：https://www.usenix.org/conference/atc12/technical-sessions/presentation/serebryany，2012USENIX ATC（2012年6月USENIX年度技术会议）。

文档：https://github.com/google/sanitizers/wiki，https://clang.llvm.org/docs/AddressSanitizer.html，https://github.com/google/sanitizers/wiki/AddressSanitizerLeakSanitizerDesignDocument

相关论文：AddressSanitizer: A Fast Address Sanity Checker，https://static.googleusercontent.com/media/research.google.com/en//pubs/archive/37752.pdf；

sanity: 健全

sanitizer: 消毒剂

### 4.3 功能

- 释放后使用
- 重复释放
- 堆越界
- 栈越界
- 函数返回后访问局部变量
- 局部变量离开作用域后访问
- 程序结束时堆上未释放的内存

### 4.4 使用

编译选项增加`-fsanitize=address -fno-omit-frame-pointer`即可。

### 4.5 demo演示

```bash
# 编译之后，程序执行结束后会自动生成泄漏统计内容

$ ./release/a.out -a

# 回车几次，触发几次内存泄漏
cicling
0x7f69d58ff800

cicling
0x7f69d57fd800

cicling
0x7f69d56fb800

# 输入 'q' 后回车，退出程序
q
break

=================================================================
==5260==ERROR: LeakSanitizer: detected memory leaks

Direct leak of 3145728 byte(s) in 3 object(s) allocated from:
    #0 0x7f69d947b867 in __interceptor_malloc ../../../../src/libsanitizer/asan/asan_malloc_linux.cpp:145
    #1 0x55873acf09de in leak_malloc_big() (/home/test/code/log/dmalloc/demo_asan/release/a.out+0x39de)
    #2 0x55873acf25c3 in main (/home/test/code/log/dmalloc/demo_asan/release/a.out+0x55c3)
    #3 0x7f69d8e97d8f in __libc_start_call_main ../sysdeps/nptl/libc_start_call_main.h:58

SUMMARY: AddressSanitizer: 3145728 byte(s) leaked in 3 allocation(s).
```

### 4.6 原理

- 如何侦测程序的内存泄漏？

  参考文档：

  https://github.com/google/sanitizers/wiki/AddressSanitizerAlgorithm

  https://github.com/google/sanitizers/wiki/AddressSanitizerLeakSanitizerDesignDocument

  ASan使用LSan检查内存泄漏检查。其特征有：

  - 和dmalloc一样，LSan是一个运行时工具（不需要重新编译）。
  - 作为一个独立进程通过ptrace附加到目标进程上
  - 暂停目标进程的执行，搜集所有线程、全局变量等信息，并找出不再被引用的内存块。不被引用的内存块就认为是泄漏的内存。（不是简单的跟踪malloc和free等内存申请的调用）

- 如何检测异常的内存访问？

  对于不应该访问的内存，设置“毒”标记。对于程序中的内存访问相关代码，在编译期对其替换，添加地址检查逻辑。

### 4.7 和dmalloc的比较

- 和GCC版本相关（组件不完全独立）
- ASan是GCC集成的，不需要对代码进行额外修改，更方便。
- ASan支持的检查项更多（堆内存、栈内存、全局内存等等）。
- ASan作为内存检查工具更成熟（目前仍在更新）。
- 检查内存泄漏时，ASan文档给出的方式是在进程退出时检查。
  - 对于常驻进程，可能需要支持进程退出。
  - 对于内存的过多使用引起的泄漏（第二种内存泄漏方式），是否能检测暂未知。
