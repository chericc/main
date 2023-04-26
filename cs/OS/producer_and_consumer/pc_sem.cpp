/**
 * 生产者消费者的信号量实现
 */

#include <unistd.h>

#include <list>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>

static std::list<int> s_list;
sem_t sem_full;
sem_t sem_empty;
sem_t sem_mutex;

void *trd_producer(void *)
{
    pthread_detach (pthread_self());

    while (1)
    {
        int product = 0;
        scanf ("%d", &product);

        sem_wait (& sem_empty); // 获取一个空位
        sem_wait (& sem_mutex); // 进入临界区

        s_list.push_back (product);
        printf ("+ %d\n", product);

        printf ("array: ");
        for (auto i : s_list) 
        {
            printf ("%d ", i);
        }
        printf ("\n");

        sem_post (& sem_mutex); // 离开临界区
        sem_post (& sem_full); // 记录一个填充位
    }
}

void *trd_consumer(void *)
{
    pthread_detach (pthread_self());

    while (1)
    {
        int product = 0;

        sem_wait (& sem_full); // 获取一个填充位
        sem_wait (& sem_mutex); // 进入临界区

        product = s_list.front();
        s_list.pop_front();
        printf ("- %d\n", product);

        sem_post (& sem_mutex); // 离开临界区
        sem_post (& sem_empty); // 记录一个空位
        
        sleep (1); // simulate consume process
    }
}

int main()
{
    pthread_t tid_producer, tid_consumer;

    sem_init (& sem_empty, 0, 5);
    sem_init (& sem_full, 0, 0);
    sem_init (& sem_mutex, 0, 1);

    pthread_create (&tid_producer, NULL, trd_producer, NULL);
    pthread_create (&tid_consumer, NULL, trd_consumer, NULL);

    printf("main finished\n");

    sleep (100);

    return 0;
}
