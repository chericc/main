#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <array>

class CircleBuffer
{
public:
    CircleBuffer();
    int get();
    void put(int x);
private:
    int circle(int pos);
    int distance();
private:
    std::array<int, 8> m_array;
    int m_nEmpty; // point to first empty element
    int m_nFull; // point to first full element
    pthread_cond_t m_cond_notfull;
    pthread_cond_t m_cond_notempty;
    pthread_mutex_t m_mutex;
};

CircleBuffer::CircleBuffer() :
    m_nEmpty(0), m_nFull(0), m_array({}),
    m_cond_notfull(PTHREAD_COND_INITIALIZER),
    m_cond_notempty(PTHREAD_COND_INITIALIZER)
{
}

int CircleBuffer::get()
{
    int value = 0;
    pthread_mutex_lock(& m_mutex);
    if (distance() == 0)
    {
        pthread_cond_wait(& m_cond_notempty, & m_mutex);
    }
    value = m_array[m_nFull];
    m_nFull = circle (m_nFull + 1);
    pthread_cond_signal(& m_cond_notfull);

    pthread_mutex_unlock (& m_mutex);
    return value;
}

void CircleBuffer::put(int x)
{
    pthread_mutex_lock(& m_mutex);
    if (distance() == m_array.size() - 1)
    {
        pthread_cond_wait (& m_cond_notfull, & m_mutex);
    }
    m_array[m_nEmpty] = x;
    m_nEmpty = circle(m_nEmpty + 1);
    pthread_cond_signal(& m_cond_notempty);
    pthread_mutex_unlock (& m_mutex);
}

int CircleBuffer::distance()
{
    return circle(m_nFull - m_nEmpty);
}

int CircleBuffer::circle(int pos)
{
    int N = m_array.size();
    return pos % N;
}

CircleBuffer s_circlebuf;

void *trd_producer(void *)
{
    while (1)
    {
        int x = 0;
        scanf ("%d", &x);
        s_circlebuf.put (x);
        printf ("put %d\n", x);
    }
}

void *trd_consumer(void *)
{
    while (1)
    {
        int x = 0;
        x = s_circlebuf.get();
        printf ("get %d\n", x);
        sleep (1);
    }
}

int main()
{
    pthread_t tid_producer;
    pthread_t tid_consumer;
    pthread_create (&tid_producer, NULL, trd_producer, NULL);
    pthread_create (&tid_producer, NULL, trd_consumer, NULL);
    sleep (600);
    return 0;
}