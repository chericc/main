/**
 * 生产者消费者的管程实现
 */

#include <unistd.h>

#include <list>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>

class Monitor
{
public:
    Monitor();
    void put(int x);
    int get();
private:
    pthread_cond_t m_cond_notfull;
    pthread_cond_t m_cond_notempty;
    pthread_mutex_t m_mutex;

    std::list<int> s_list;
};

Monitor::Monitor() : 
    m_cond_notfull(PTHREAD_COND_INITIALIZER),
    m_cond_notempty(PTHREAD_COND_INITIALIZER),
    m_mutex(PTHREAD_MUTEX_INITIALIZER)
{
}

void Monitor::put(int x)
{
    pthread_mutex_lock (& m_mutex);
    if (s_list.size() >= 5) 
    {
        pthread_cond_wait (& m_cond_notfull, & m_mutex);
    }

    s_list.push_back (x);
    printf ("+ %d\n", x);

    printf ("array: ");
    for (auto i : s_list) 
    {
        printf ("%d ", i);
    }
    printf ("\n");

    pthread_cond_signal (& m_cond_notempty);

    pthread_mutex_unlock (& m_mutex);
}

int Monitor::get()
{
    pthread_mutex_lock (& m_mutex);

    if (s_list.size() == 0)
    {
        pthread_cond_wait (& m_cond_notempty, & m_mutex);
    }


    int x = s_list.front();
    s_list.pop_front();
    printf ("- %d\n", x);

    pthread_cond_signal (& m_cond_notfull);

    pthread_mutex_unlock (& m_mutex);
    return x;
}

static Monitor s_monitor;

void *trd_producer(void *)
{
    pthread_detach (pthread_self());

    while (1)
    {
        int product = 0;
        scanf ("%d", &product);

        s_monitor.put(product);
    }
}

void *trd_consumer(void *)
{
    pthread_detach (pthread_self());

    while (1)
    {
        int product = 0;

        product = s_monitor.get();
        sleep (1); // simulate consume process
    }
}

int main()
{
    pthread_t tid_producer, tid_consumer;

    pthread_create (&tid_producer, NULL, trd_producer, NULL);
    pthread_create (&tid_consumer, NULL, trd_consumer, NULL);

    printf("main finished\n");

    sleep (100);

    return 0;
}
