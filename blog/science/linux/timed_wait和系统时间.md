# timed_wait和系统时间

## 问题的提出

在linux中，有几种timed_wait的形式，如：

```c
#include <semaphore.h>
int sem_timedwait(sem_t *sem, const struct timespec *abs_timeout);

// The abs_timeout argument points to a structure that specifies an absolute timeout in seconds and
//       nanoseconds since the Epoch.

#include <pthread.h>
#include <time.h>
int pthread_mutex_timedlock(pthread_mutex_t *restrict mutex,
           const struct timespec *restrict abstime);

// The timeout shall be based on the CLOCK_REALTIME clock.

int pthread_cond_timedwait(pthread_cond_t *restrict cond,
           pthread_mutex_t *restrict mutex,
           const struct timespec *restrict abstime);

// The condition variable shall have a clock attribute which specifies the clock 
// that shall be used to measure the time specified by the abstime argument.
```

其中，等待超时的形式都和系统时间相关，那么，如果在调用接口之前，修改了系统时间，这些接口会有什么表现？

## sem_timedwait

```c++
/**
 * This is a demo showing what will happen when changing 
 * system time while doing sem_timedwait.
*/

#include <semaphore.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <chrono>
#include <string>

int main()
{
    sem_t sem;
    int ret = 0;
    struct timespec sts;
    constexpr const int wait_duration = 30;

    auto now = []()
    {
        using namespace std;
        auto now_ms = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now().time_since_epoch()).count();
        return now_ms;
    };
    auto now_str = [](time_t now)
    {
        char buffer[64]{};
        struct tm stm{};
        localtime_r(&now, &stm);
        strftime(buffer, sizeof(buffer), "%F %T", &stm);
        return std::string(buffer);
    };

    sem_init(&sem, 0, 0);
    clock_gettime(CLOCK_REALTIME, &sts);

    printf("Now: %s\n", now_str(sts.tv_sec).c_str());

    sts.tv_sec += wait_duration;

    printf("Waiting %d secs\n", wait_duration);

    auto begin_ms = now();
    sem_timedwait(&sem, &sts);
    auto end_ms = now();

    printf("Time passed: %d ms\n", (int)(end_ms - begin_ms));
    return 0;
}
```

程序运行后，通过系统命令修改系统时间（比如将系统时间提前1min），此时能够得到如下结果：

```bash
$ ./a.out
Now: 2023-07-26 17:24:29
Waiting 30 secs
Time passed: 13087 ms
```

因此有结论，