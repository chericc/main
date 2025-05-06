#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <chrono>
#include <cstring>

// avoid compiler optimization
void *g_mem = nullptr;

static void print_usage(int argc, char *argv[])
{
    printf("Usage: %s [cache_size(KB)] [task_size(MB)] [duration(seconds)]\n", argv[0]);
    return ;
}

static void run_task(void *mem, size_t size, uint64_t task_size)
{
    uint64_t count = 0;

    auto *mem8 = (uint8_t *)mem;

    for (; count < task_size; count += size) {
        memcpy(mem8, mem8 + size / 2, size / 2);
    }
}

int main(int argc, char *argv[])
{
    if (argc < 4) {
        print_usage(argc, argv);
        return -1;
    }

    size_t cache_size_KB = atoi(argv[1]);
    uint64_t task_size_MB = atoi(argv[2]);
    int duration_sec = atoi(argv[3]);

    size_t cache_size = cache_size_KB * 1024;
    uint64_t task_size = task_size_MB * 1024 * 1024;

    g_mem = malloc(cache_size);

    int count = 0;
    auto duration_total = std::chrono::microseconds(0);
    auto tp_last_print = std::chrono::steady_clock::now();
    auto tp_tasks_start = std::chrono::steady_clock::now();

    for (count = 1; ; ++count) {
        auto tp_start = std::chrono::steady_clock::now();
        run_task(g_mem, cache_size, task_size);
        auto tp_end = std::chrono::steady_clock::now();
        auto duration = tp_end - tp_start;
        duration_total = duration_total + std::chrono::duration_cast<std::chrono::microseconds>(duration);
        
        if (tp_end - tp_last_print > std::chrono::seconds(1)) {
            tp_last_print = tp_end;
            printf("avg: %ld us/task, count=%d\n", (long)(duration_total.count() / count), count);
        }

        if (tp_end - tp_tasks_start > std::chrono::seconds(duration_sec)) {
            printf("end\n");
            break;
        }
    }
    
    return 0;
}