/**
 * 生产者和消费者
 * 信号量
 * 
 * 问题描述：
 * 生产者P1：生产正整数，并放入缓存
 * 消费者P2：从缓存中取奇数，并统计个数
 * 消费者P3：从缓存中取偶数，并统计个数
 * 
 * 题目原型：
 * 计算机408全国，2009统考真题
 * 
 */

#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>

#include <stdio.h>
#include <assert.h>

#include <list>
#include <random>

#define BUF_SIZE 4

static std::list<int> s_buffer; // 缓存

sem_t sema_mutex;        // 访问缓存锁
sem_t sema_empty;        // 空位置信号量
sem_t sema_full_even;    // 偶数满位置信号量，even: n % 2 == 0
sem_t sema_full_odd;     // 奇数满位置信号量，odd: n % 2 == 1


std::default_random_engine engine;
std::uniform_int_distribution<int> dis(1,100);

int produce()
{
    int r = dis(engine);
    sleep (1);
    printf ("produce %d\n", r);
    return r;
}

void count_odd(int odd)
{
    static int n_countodd = 0;
    ++n_countodd;
    sleep (2);
    printf ("new odd(%d), odd count=%d\n", odd, n_countodd);
    return ;
}

void count_even(int even)
{
    static int n_counteven = 0;
    ++n_counteven;
    sleep (2);
    printf ("new even(%d), even count=%d\n", even, n_counteven);
    return ;
}

int get_odd(std::list<int> & ref_list)
{
    auto it = ref_list.begin();
    for (; it != ref_list.end(); ++it)
    {
        if (*it % 2 == 1)
        {
            break;
        }
    }
    assert (it != ref_list.end());
    ref_list.erase (it);
    return *it;
}

int get_even(std::list<int> & ref_list)
{
    auto it = ref_list.begin();
    for (; it != ref_list.end(); ++it)
    {
        if (*it % 2 == 0)
        {
            break;
        }
    }
    assert (it != ref_list.end());
    ref_list.erase(it);
    return *it;
}

void *thread_p1(void *arg)
{
    printf ("enter p1\n");
    while (true)
    {
        int k = produce();
        sem_wait (& sema_empty); // 获取一个空位置
        sem_wait (& sema_mutex); // 进入访缓临界区
        s_buffer.push_back(k);   // 入缓
        assert (s_buffer.size() <= BUF_SIZE);
        sem_post (& sema_mutex); // 离开访缓临界区
        if (k % 2 == 0) // even
        {
            sem_post (& sema_full_even);
        }
        else 
        {
            sem_post (& sema_full_odd);
        }
    }
}

void *thread_p2(void *arg)
{
    printf ("enter p2\n");
    while (true)
    {
        sem_wait (& sema_full_odd);
        sem_wait (& sema_mutex);
        int k = get_odd (s_buffer);
        sem_post (& sema_mutex);
        sem_post (& sema_empty);
        count_odd (k);
    }
}

void *thread_p3(void *arg)
{
    printf ("enter p3\n");
    while (true)
    {
        sem_wait (& sema_full_even);
        sem_wait (& sema_mutex);
        int k = get_even (s_buffer);
        sem_post (& sema_mutex);
        sem_post (& sema_empty);
        count_even(k);
    }
}

int main()
{
    pthread_t tid_p1;
    pthread_t tid_p2;
    pthread_t tid_p3;

    sem_init (&sema_mutex, false, 1);
    sem_init(&sema_empty, false, BUF_SIZE);  // 
    sem_init(&sema_full_even, false, 0);
    sem_init(&sema_full_odd, false, 0);

    pthread_create (&tid_p1, NULL, thread_p1, NULL);
    pthread_create (&tid_p2, NULL, thread_p2, NULL);
    pthread_create (&tid_p3, NULL, thread_p3, NULL);

    while (1) 
        sleep (5);
}