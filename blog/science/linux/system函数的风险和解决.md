# system函数的风险和解决

## 源码摘录

```c
/* Execute LINE as a shell command, returning its status.  */
static int
do_system (const char *line)
{
  int status = -1;
  int ret;
  pid_t pid;
  struct sigaction sa;
#ifndef _LIBC_REENTRANT
  struct sigaction intr, quit;
#endif
  sigset_t omask;
  sigset_t reset;

  sa.sa_handler = SIG_IGN;
  sa.sa_flags = 0;
  __sigemptyset (&sa.sa_mask);

  DO_LOCK ();
  if (ADD_REF () == 0)
    {
      /* sigaction can not fail with SIGINT/SIGQUIT used with SIG_IGN.  */
      __sigaction (SIGINT, &sa, &intr);
      __sigaction (SIGQUIT, &sa, &quit);
    }
  DO_UNLOCK ();

  __sigaddset (&sa.sa_mask, SIGCHLD);
  /* sigprocmask can not fail with SIG_BLOCK used with valid input
     arguments.  */
  __sigprocmask (SIG_BLOCK, &sa.sa_mask, &omask);

  __sigemptyset (&reset);
  if (intr.sa_handler != SIG_IGN)
    __sigaddset(&reset, SIGINT);
  if (quit.sa_handler != SIG_IGN)
    __sigaddset(&reset, SIGQUIT);

  posix_spawnattr_t spawn_attr;
  /* None of the posix_spawnattr_* function returns an error, including
     posix_spawnattr_setflags for the follow specific usage (using valid
     flags).  */
  __posix_spawnattr_init (&spawn_attr);
  __posix_spawnattr_setsigmask (&spawn_attr, &omask);
  __posix_spawnattr_setsigdefault (&spawn_attr, &reset);
  __posix_spawnattr_setflags (&spawn_attr,
			      POSIX_SPAWN_SETSIGDEF | POSIX_SPAWN_SETSIGMASK);

  ret = __posix_spawn (&pid, SHELL_PATH, 0, &spawn_attr,
		       (char *const[]){ (char *) SHELL_NAME,
					(char *) "-c",
					(char *) line, NULL },
		       __environ);
  __posix_spawnattr_destroy (&spawn_attr);

  if (ret == 0)
    {
      /* Cancellation results in cleanup handlers running as exceptions in
	 the block where they were installed, so it is safe to reference
	 stack variable allocate in the broader scope.  */
#if defined(_LIBC_REENTRANT) && defined(SIGCANCEL)
      struct cancel_handler_args cancel_args =
      {
	.quit = &quit,
	.intr = &intr,
	.pid = pid
      };
      __libc_cleanup_region_start (1, cancel_handler, &cancel_args);
#endif
      /* Note the system() is a cancellation point.  But since we call
	 waitpid() which itself is a cancellation point we do not
	 have to do anything here.  */
      if (TEMP_FAILURE_RETRY (__waitpid (pid, &status, 0)) != pid)
	status = -1;
#if defined(_LIBC_REENTRANT) && defined(SIGCANCEL)
      __libc_cleanup_region_end (0);
#endif
    }
  else
   /* POSIX states that failure to execute the shell should return
      as if the shell had terminated using _exit(127).  */
   status = W_EXITCODE (127, 0);

  DO_LOCK ();
  if (SUB_REF () == 0)
    {
      /* sigaction can not fail with SIGINT/SIGQUIT used with old
	 disposition.  Same applies for sigprocmask.  */
      __sigaction (SIGINT, &intr, NULL);
      __sigaction (SIGQUIT, &quit, NULL);
      __sigprocmask (SIG_SETMASK, &omask, NULL);
    }
  DO_UNLOCK ();

  if (ret != 0)
    __set_errno (ret);

  return status;
}
```

主要步骤有：忽略信号SIGINT和SIGQUIT，阻塞信号SIGCHLD，创建子进程执行shell并执行命令，等待子进程（退出），恢复信号；

## 主要问题和注意事项

### 低效

```c++
#include <cstdlib>
#include <chrono>
#include <stdio.h>

int main()
{
	auto lambda_now_ms = []()
	{ 
		return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	};
	
	auto start1 = lambda_now_ms();
	auto start2 = lambda_now_ms();
	for (int i = 0; i < 1000; ++i)
	{
		std::system("touch test_file");
		std::system("rm test_file");
	}
	auto end = lambda_now_ms();

	int diff1 = start2 - start1;
	int diff2 = end - start2;
	printf("diff: %d/%d\n", diff2, diff1);
	return 0;
}
```

```bash
$ g++ test.cpp
$ ./a.out
diff: 3372/0
```

可以看到，通过system函数执行文件系统命令的耗时达到了毫秒级别。

将创建文件和删除文件的操作改为直接实现的方式，如：

```c
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
...
open(file, O_CREAT | O_RDWR | O_TRUNC, 0666);
unlink(file);
```

此时，执行的结果为：

```bash
$ g++ test.cpp
$ ./a.out
diff: 22/0
```

可以看到，两者速度的差别达到了两个数量级；

### 信号处理

#### 为什么要忽略信号SIGINT和SIGQUIT

一个解释：这两个信号在终端产生，并且是发送给所有前台进程组中的进程的。当调用system()运行一个前台程序时，如果此时触发了信号，则这个信号只应该影响system()调用的程序，而不应该影响调用system()的程序。

一个示例如下：

```c++
#include <stdlib.h>
#include <stdio.h>
#include <thread>

int main()
{
    system("vim");
    printf("end\n");

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
```

程序运行后，按下"Ctrl C"，退出vim后，发现主程序继续运行。如果system()没有处理SIGINT信号，则当vim退出后，主程序也会一并退出。

#### 为什么要阻塞信号SIGCHLD

这是因为system()的实现中使用了waitpid()来获取进程执行的结果。可以参考man 2 waitpid。

主要原因是如果一个进程忽略了SIGCHLD（这也是默认行为），则其子进程在结束后，waitpid会等待子进程的结束，然后返回错误（不会得到正常的返回值）。

### 返回值

### 环境变量

## 解决

