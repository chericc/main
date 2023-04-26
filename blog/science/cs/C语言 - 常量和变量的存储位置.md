# C语言 - 常量和变量的存储位置

[toc]

## 〇、环境

项 | 值
:--- | :---
语言 | C
编译器 | gcc
编译环境 | x86_64-linux-gnu

## 一、常量和变量的存储位置

### 1.1 实验方法

静态观察：对编译得到的可执行文件进行分析  
动态观察：运行时获取变量的地址，并分析变量在进程地址空间的位置  

### 1.2 示例程序

#### 示例程序1

```C
#include <stdio.h>
#include <unistd.h>

static int local_static_uninit;
static int local_static_init = 0x88;
static int localarr_static_uninit[1024];
static int localarr_static_init[1024] = {1,2,3};

int global_uninit;
int gloabl_init = 0x88;
int globalarr_uninit[1024];
int globalarr_init[1024] = {1,2,3};

int add(int a, int b)
{
  static int infun_static_uninit;
  static int infun_static_init = 0x88;
  static int infunarr_static_uninit[1024];
  static int infunarr_static_init[1024] = {1,2,3};

  int c;
  int d = 0;
  int e = a + b;

  printf (
    "&local_static_uninit=%p\n"
    "&local_static_init=%p\n"
    "&localarr_static_uninit[0]=%p\n"
    "&localarr_static_init[0]=%p\n"
    "&global_uninit=%p\n"
    "&gloabl_init=%p\n"
    "&globalarr_uninit[0]=%p\n"
    "&globalarr_init[0]=%p\n"
    "&infun_static_uninit=%p\n"
    "&infun_static_init=%p\n"
    "&infunarr_static_uninit[0]=%p\n"
    "&infunarr_static_init[0]=%p\n"
    "&a=%p\n"
    "&b=%p\n"
    "&c=%p\n"
    "&d=%p\n"
    ,
    &local_static_uninit,
    &local_static_init,
    &localarr_static_uninit[0],
    &localarr_static_init[0],
    &global_uninit,
    &gloabl_init,
    &globalarr_uninit[0],
    &globalarr_init[0],
    &infun_static_uninit,
    &infun_static_init,
    &infunarr_static_uninit[0],
    &infunarr_static_init[0],
    &a, &b, &c, &d
  );

  while (1)
  {
    sleep (5);
  }

  return e;
}

int main()
{
  add(1,2);
  return 0;
}
```

文件布局：

运行输出：
> ./a.out
&local_static_uninit=0x6040e0
&local_static_init=0x601060
&localarr_static_uninit[0]=0x604100
&localarr_static_init[0]=0x601080
&global_uninit=0x606120
&gloabl_init=0x602080
&globalarr_uninit[0]=0x606140
&globalarr_init[0]=0x6020a0
&infun_static_uninit=0x605100
&infun_static_init=0x6030a0
&infunarr_static_uninit[0]=0x605120
&infunarr_static_init[0]=0x6030c0
&a=0x7ffd0ee30b1c
&b=0x7ffd0ee30b18
&c=0x7ffd0ee30b2c
&d=0x7ffd0ee30b30

进程地址布局：
>cat /proc/2446/maps 
00400000-00401000 r-xp 00000000 fc:00 130819                             /home/ttt/test/a.out
00600000-00601000 r--p 00000000 fc:00 130819                             /home/ttt/test/a.out
00601000-00605000 rw-p 00001000 fc:00 130819                             /home/ttt/test/a.out
00605000-00608000 rw-p 00000000 00:00 0 
007cf000-007f0000 rw-p 00000000 00:00 0                                  [heap]
7f54a3458000-7f54a3618000 r-xp 00000000 fc:00 659982                     /lib/x86_64-linux-gnu/libc-2.23.so
7f54a3618000-7f54a3818000 ---p 001c0000 fc:00 659982                     /lib/x86_64-linux-gnu/libc-2.23.so
7f54a3818000-7f54a381c000 r--p 001c0000 fc:00 659982                     /lib/x86_64-linux-gnu/libc-2.23.so
7f54a381c000-7f54a381e000 rw-p 001c4000 fc:00 659982                     /lib/x86_64-linux-gnu/libc-2.23.so
7f54a381e000-7f54a3822000 rw-p 00000000 00:00 0 
7f54a3822000-7f54a3848000 r-xp 00000000 fc:00 659993                     /lib/x86_64-linux-gnu/ld-2.23.so
7f54a3a3c000-7f54a3a3f000 rw-p 00000000 00:00 0 
7f54a3a47000-7f54a3a48000 r--p 00025000 fc:00 659993                     /lib/x86_64-linux-gnu/ld-2.23.so
7f54a3a48000-7f54a3a49000 rw-p 00026000 fc:00 659993                     /lib/x86_64-linux-gnu/ld-2.23.so
7f54a3a49000-7f54a3a4a000 rw-p 00000000 00:00 0 
7ffd0ee11000-7ffd0ee32000 rw-p 00000000 00:00 0                          [stack]
7ffd0eff3000-7ffd0eff6000 r--p 00000000 00:00 0                          [vvar]
7ffd0eff6000-7ffd0eff8000 r-xp 00000000 00:00 0                          [vdso]
ffffffffff600000-ffffffffff601000 r-xp 00000000 00:00 0                  [vsyscall]

### 1.3 结果分析

#### 1.3.1 运行时结果

- 源文件中的全局变量，包括普通全局变量、静态全局变量，不论是否初始化，均在程序段（可写数据段）
- 函数内部的静态变量，不论是否初始化，均在程序段（可写数据段）。（和全局变量一致）
- 函数内部的局部变量，函数传参，均在栈段。

#### 1.3.2 静态分析结果

说明一点，未初始化的全局变量（包括函数内静态变量）位于.bss段，并不占用可执行文件的空间。以后再表。
