# SYS 占用异常

## 一、问题描述

在进行一个嵌入式项目时，发现主程序在一定条件下会引起系统卡顿的问题。通过 top 打印分析，主要表现为 SYS 占用过高（高达 60% 以上）。本文就这个问题进行分析。

## 二、top 输出中 CPU 相关使用字段的含义

top 打印的部分内容示例：

```bash
top - 18:27:16 up 20 days,  3:58,  1 user,  load average: 0.03, 0.08, 0.03
Tasks: 223 total,   1 running, 222 sleeping,   0 stopped,   0 zombie
%Cpu(s):  0.4 us,  0.5 sy,  0.0 ni, 99.1 id,  0.1 wa,  0.0 hi,  0.0 si,  0.0 st
MiB Mem :  15638.4 total,    218.1 free,    651.2 used,  14769.0 buff/cache
MiB Swap:      0.0 total,      0.0 free,      0.0 used.  14630.8 avail Mem
```

通过查阅 top 的手册，获取 Cpu 相关字段的含义为：

```bash
us, user    : time running un-niced user processes
sy, system  : time running kernel processes
ni, nice    : time running niced user processes
id, idle    : time spent in the kernel idle handler
wa, IO-wait : time waiting for I/O completion
hi : time spent servicing hardware interrupts
si : time spent servicing software interrupts
st : time stolen from this vm by the hypervisor
```

## 三、分析

### 3.1 SYS 占用高的原因是什么？

这里我们只考虑 user 和 system 两种主要的 CPU 占用，简单的说，user 就是 CPU 花在执行进程代码上的时间，system 就是 CPU 花在内核进程代码上的时间。

一般而言，系统调用不会出现过大的 CPU 占用，因此如果一个系统出现了大的 system 引起的 CPU 占用，可以考虑是程序出现了过于频繁的系统调用。这里可以举一个例子进行试验：  

```C
#include <unistd.h>
#include <stdbool.h>

int main()
{
	while (true)
	{
		usleep (1);
	}

	return 0;
}
```

执行程序后，可以明显发现 SYS 占用相比 USR 占用高。

### 3.2 如何分析程序引起 SYS 高占用？

SYS 占用高，其本质是进程出现了不合理的系统资源使用，可以通过一些工具来进行分析。本文主要使用两个工具：strace 和 gstack 。

### 3.3 一个例子程序

为了一定程度上模拟开发场景中出现的问题，我们构建一个例子程序，这个程序中会使用两个线程，其中一个线程是一个正常的，另一个线程则会在启动后逐渐增加对系统资源的消耗。

例子程序如下：

```C
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>

unsigned short g_count = 0;
unsigned short g_start = 0;

void *trd_normal(void *arg)
{
	printf ("normal thread, sizeof(int)=%lu\n", sizeof(int));

	while (true)
	{
		g_count ++;
		if (g_start == g_count)
		{
			printf ("normal: meet\n");
		}
		usleep (1000 * 100);
	}
}

void *trd_bad (void *arg)
{
	printf ("bad thread\n");

	while (true)
	{
		g_count ++;
		if (g_start == g_count)
		{
			printf ("bad: meet\n");
		}
		usleep (1);
	}
}

int main(int argc, char *argv[])
{
	pthread_t tid_1 = 0;
	pthread_t tid_2 = 0;

	pthread_create (& tid_1, NULL, trd_normal, NULL);
	pthread_create (& tid_2, NULL, trd_bad, NULL);

	while (true)
	{
		sleep (1);
	}

	return 0;
}

```

```bash
test@test:~/test$ strace -c ./a.out
normal thread, sizeof(int)=4
bad thread
bad: meet
bad: meet
^Cstrace: Process 1697 detached
% time     seconds  usecs/call     calls    errors syscall
------ ----------- ----------- --------- --------- ----------------
 99.49    0.168995       21124         8         1 clock_nanosleep
  0.17    0.000295         295         1           execve
  0.10    0.000162          10        15           mmap
  0.05    0.000080          11         7           mprotect
  0.04    0.000061           7         8           pread64
  0.03    0.000057          28         2           clone
  0.02    0.000034          11         3           openat
  0.01    0.000025           8         3           brk
  0.01    0.000024           8         3           fstat
  0.01    0.000022           7         3           close
  0.01    0.000018          18         1           munmap
  0.01    0.000017           8         2           read
  0.01    0.000016           8         2           rt_sigaction
  0.01    0.000014           7         2         1 arch_prctl
  0.01    0.000013          13         1         1 access
  0.00    0.000008           8         1           rt_sigprocmask
  0.00    0.000008           8         1           set_robust_list
  0.00    0.000008           8         1           prlimit64
  0.00    0.000007           7         1           set_tid_address
------ ----------- ----------- --------- --------- ----------------
100.00    0.169864                    65         3 total
```