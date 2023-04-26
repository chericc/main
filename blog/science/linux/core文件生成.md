# core 文件生成

## 原理

core 文件是系统对收到信号的进程的一种动作方式。这里列举几种能引起生成 core 动作的信号：  

```bash
SIGABRT
SIGBUS
SIGFPE
SIGILL
SIGIOT
SIGQUIT
SIGSEGV
SIGSYS
SIGTRAP
SIGUNUSED
SIGXCPU
SIGXFSZ
```

进程收到生成 core 文件信号之后，是否生成 core 文件，需要进程指定 RLIMIT_CORE 。可以通过 setrlimit 修改。  

core 文件的文件名还受 /proc/sys/kernel/core_pattern 影响。  

一个例子：  

```bash
echo "core-%e-%t-%s" > /proc/sys/kernel/core_pattern
```

## 通过 shell 修改

shell 一般都支持通过 ulimit 修改进程资源，例如 ulimit -c unlimited。  

shell 修改了进程资源后，将通过进程的资源信息传递传递给子进程（man setrlimit）。  
