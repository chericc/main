
// 分配多个指定大小的内存，并每个一定间隔去访问这些内存

#include <stdio.h>
#include <stdlib.h>

#include <list>
#include <thread>

std::list<unsigned char*> g_plist;

void print_usage(int, const char* argv[]) {
    printf("%s mem_num mem_size touch_interval\n", argv[0]);
    return;
}

int main(int argc, const char* argv[]) {
    fflush(stdin);

    if (argc != 4) {
        print_usage(argc, argv);
        return -1;
    }

    int mem_num = atoi(argv[1]);
    int mem_size = atoi(argv[2]);
    int touch_interval = atoi(argv[3]);

    if (mem_num <= 0 || mem_size <= 0 || touch_interval <= 0) {
        printf("Parameter error\n");
        return -1;
    }

    printf("Allocating %d * %d Byte, Total: %d Byte\n", mem_num, mem_size,
           mem_num * mem_size);

    for (int i = 0; i < mem_num; ++i) {
        unsigned char* pmem = (unsigned char*)malloc(mem_size);
        if (pmem) {
            g_plist.push_back(pmem);
            pmem = nullptr;
        }
    }

    printf("Allocation finished, touching with interval: %d seconds\n",
           touch_interval);

    while (true) {
        printf("Touching...\n");
        for (auto ref : g_plist) {
            for (int k = 0; k < mem_size; k += 4 * 1024) {
                ref[k] = k;
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(touch_interval));
    }

    return 0;
}