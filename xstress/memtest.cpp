// 分配一定数量的指定大小的内存，并在操作后释放
// 用于测试某些内存分配器的分配和释放特性

#include <stdio.h>
#include <stdlib.h>

#include <thread>
#include <list>
#include <mutex>

std::mutex g_mutex;
std::list<unsigned char *> g_plist;
int g_touch_interval_sec = 1;

int mem_num = 0;
int mem_size = 0;
int touch_interval = 0;

void print_usage(int, const char *argv[])
{
    printf("%s mem_num mem_size touch_interval\n", argv[0]);
    return ;
}

void trd_worker()
{
    while (true)
    {
        {
            std::lock_guard<std::mutex> lock(g_mutex);

            if (!g_plist.empty())
            {
                printf("Touching...\n");
                for (auto ref : g_plist)
                {
                    for (int k = 0; k < mem_size; k += 4 * 1024)
                    {
                        ref[k] = k;
                    }
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(touch_interval));
    }
}

int main(int argc, const char *argv[])
{
    fflush(stdin);

    if (argc != 4)
    {
        print_usage(argc, argv);
        return -1;
    }

    mem_num = atoi(argv[1]);
    mem_size = atoi(argv[2]);
    touch_interval = atoi(argv[3]);

    if (mem_num <= 0 || mem_size <= 0 || touch_interval <= 0)
    {
        printf("Parameter error\n");
        return -1;
    }

    printf("Allocating %d * %d Byte, Total: %d Byte\n", mem_num, mem_size, mem_num * mem_size);

    for (int i = 0; i < mem_num; ++i)
    {
        unsigned char *pmem = (unsigned char *)malloc(mem_size);
        if (pmem)
        {
            g_plist.push_back(pmem);
            pmem = nullptr;
        }
    }

    printf("Allocation finished, touching with interval: %d seconds\n", touch_interval);

    std::thread trd(trd_worker);

    fflush(stdin);

    getchar();
    {
        printf("Freeing all...\n");
        std::lock_guard<std::mutex> lock(g_mutex);
        for (auto ref : g_plist)
        {
            free(ref);
        }
        g_plist.clear();
        printf("Freeing finished\n");
    }

    printf("Ctrl-C for exit...\n");

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
