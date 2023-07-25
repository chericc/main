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

