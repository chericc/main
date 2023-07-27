# timed_wait和系统时间

## 环境的准备

本文中的试验涉及到手动修改系统时间，因此需要临时禁用自动时间同步服务；

对于ubuntu24.04，可以执行

```bash
sudo service systemd-timesyncd stop
```

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

#include <pthread.h>
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
    
    clock_gettime(CLOCK_REALTIME, &sts);
    printf("Now: %s\n", now_str(sts.tv_sec).c_str());
    return 0;
}
```

程序运行后，通过系统命令修改系统时间将系统时间加快，此时能够得到如下结果：

```bash
$ ./a.out
Now: 2023-07-27 19:43:03
Waiting 30 secs
Time passed: 8697 ms
Now: 2023-07-27 19:43:33
```

如果将系统时间减慢，此时能够得到如下结果：

```bash
$ ./a.out
Now: 2023-07-27 19:00:20
Waiting 30 secs
Time passed: 65905 ms
Now: 2023-07-27 19:00:50
```

注意，前后打印的系统时间显示，`sem_timedwait`总是按照系统时间来确定结束时间。

结论为，`sem_timedwait`受系统时间影响。如果系统时间加快，则调用提前结束，如果系统时间减慢，则调用延迟结束。

## pthread_mutex_timedlock

```c++
/**
 * This is a demo showing what will happen when changing 
 * system time while doing pthread_mutex_timedlock.
*/

#include <mutex>
#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <chrono>
#include <string>

int main()
{
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
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

    pthread_mutex_lock(&mutex);

    clock_gettime(CLOCK_REALTIME, &sts);

    printf("Now: %s\n", now_str(sts.tv_sec).c_str());

    sts.tv_sec += wait_duration;

    printf("Waiting %d secs\n", wait_duration);

    auto begin_ms = now();
    pthread_mutex_timedlock(&mutex, &sts);
    auto end_ms = now();

    printf("Time passed: %d ms\n", (int)(end_ms - begin_ms));

    clock_gettime(CLOCK_REALTIME, &sts);
    printf("Now: %s\n", now_str(sts.tv_sec).c_str());
    return 0;
}
```

若调整系统时间，将时间减慢，则结果如下：

```bash
$ ./a.out
Now: 2023-07-27 19:11:52
Waiting 30 secs
Time passed: 63194 ms
Now: 2023-07-27 19:12:22
```

若调整系统时间，将时间加快，则结果如下：

```bash
$ ./a.out
Now: 2023-07-27 19:15:03
Waiting 30 secs
Time passed: 18646 ms
Now: 2023-07-27 19:15:34
```

试验结果表明，`pthread_mutex_timedlock`和`sem_timedwait`具有一致的行为。

## pthread_cond_timedwait

```c++
/**
 * This is a demo showing what will happen when changing 
 * system time while doing pthread_cond_timedwait.
*/

#include <mutex>
#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <chrono>
#include <string>

int main()
{
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
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

    clock_gettime(CLOCK_REALTIME, &sts);

    printf("Now: %s\n", now_str(sts.tv_sec).c_str());

    sts.tv_sec += wait_duration;

    printf("Waiting %d secs\n", wait_duration);

    auto begin_ms = now();
    pthread_mutex_lock(&mutex);
    pthread_cond_timedwait(&cond, &mutex, &sts);
    pthread_mutex_unlock(&mutex);
    auto end_ms = now();

    printf("Time passed: %d ms\n", (int)(end_ms - begin_ms));

    clock_gettime(CLOCK_REALTIME, &sts);
    printf("Now: %s\n", now_str(sts.tv_sec).c_str());
    return 0;
}
```

系统时间减慢：

```bash
$ ./a.out
Now: 2023-07-27 19:24:56
Waiting 30 secs
Time passed: 58713 ms
Now: 2023-07-27 19:25:26
```

系统时间加快：

```bash
$ ./a.out
Now: 2023-07-27 19:26:54
Waiting 30 secs
Time passed: 23803 ms
Now: 2023-07-27 19:27:24
```

## 一些解决办法

### 使用c++11/timed_mutex

可以利用c++提供的一些库来实现：

```c++
/**
 * This is a demo showing what will happen when changing 
 * system time while doing sem_timedwait.
*/

#include <mutex>
#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <chrono>
#include <string>

int main()
{
    int ret = 0;
    std::timed_mutex timed_mutex;
    struct timespec sts;
    constexpr const int wait_duration = 30;

    auto now = []()
    {
        using namespace std;
        auto now_ms = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now().time_since_epoch()).count();
        return now_ms;
    };
    auto now_str = []()
    {
        time_t now = time(nullptr);
        char buffer[64]{};
        struct tm stm{};
        localtime_r(&now, &stm);
        strftime(buffer, sizeof(buffer), "%F %T", &stm);
        return std::string(buffer);
    };

    printf("Now: %s\n", now_str().c_str());

    printf("Waiting %d secs\n", wait_duration);

	timed_mutex.lock();

    auto begin_ms = now();
	auto now_tp = std::chrono::steady_clock::now();
	timed_mutex.try_lock_until(now_tp + std::chrono::seconds(wait_duration));
    auto end_ms = now();

    printf("Time passed: %d ms\n", (int)(end_ms - begin_ms));

    printf("Now: %s\n", now_str().c_str());

    return 0;
}
```

将系统时间减慢，输出如下：

```bash
$ ./a.out
Now: 2023-07-27 08:00:29
Waiting 30 secs
Time passed: 30000 ms
Now: 2023-07-27 08:00:44
```

将系统时间加快，输出如下：

```bash
$ ./a.out
Now: 2023-07-27 08:01:43
Waiting 30 secs
Time passed: 30000 ms
Now: 2023-07-27 08:02:13
```

查阅`std::timed_mutex`的源码，可以发现其使用了`pthread_mutex_clocklock`。

类似的还有`std::condition_variable::wait_until`，查阅源代码可以发现其使用了`pthread_cond_clockwait`。