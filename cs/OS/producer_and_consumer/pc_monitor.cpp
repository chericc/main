/**
 * 生产者消费者的信号量实现
 */

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>

#include <list>

static std::list<int> s_list;
pthread_cond_t s_cond_notfull = PTHREAD_COND_INITIALIZER;
pthread_cond_t s_cond_notempty = PTHREAD_COND_INITIALIZER;
pthread_mutex_t s_mutex = PTHREAD_MUTEX_INITIALIZER;

void* trd_producer(void*) {
    pthread_detach(pthread_self());

    while (1) {
        int product = 0;
        scanf("%d", &product);

        pthread_mutex_lock(&s_mutex);  // use mutex to simulate Monitor

        if (s_list.size() >= 5) {
            pthread_cond_wait(&s_cond_notfull, &s_mutex);
        }

        s_list.push_back(product);
        printf("+ %d\n", product);

        printf("array: ");
        for (auto i : s_list) {
            printf("%d ", i);
        }
        printf("\n");

        pthread_cond_signal(&s_cond_notempty);

        pthread_mutex_unlock(&s_mutex);
    }
}

void* trd_consumer(void*) {
    pthread_detach(pthread_self());

    while (1) {
        int product = 0;

        pthread_mutex_lock(&s_mutex);

        if (s_list.size() == 0) {
            pthread_cond_wait(&s_cond_notempty, &s_mutex);
        }

        product = s_list.front();
        s_list.pop_front();
        printf("- %d\n", product);

        pthread_cond_signal(&s_cond_notfull);
        pthread_mutex_unlock(&s_mutex);

        sleep(1);  // simulate consume process
    }
}

int main() {
    pthread_t tid_producer, tid_consumer;

    pthread_create(&tid_producer, NULL, trd_producer, NULL);
    pthread_create(&tid_consumer, NULL, trd_consumer, NULL);

    printf("main finished\n");

    sleep(100);

    return 0;
}
