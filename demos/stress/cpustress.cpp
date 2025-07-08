#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <thread>

#define DEFAULT_THREADS 4
#define NSEC_PER_SEC    1000000000

static int keep_running = 1;
static double s_val = 0.0;

static void cpu_load_thread() 
{
    while (keep_running) {
        for(int i = 0; i < 100000; i++){
            s_val += sqrt(i) * tan(i) * log(i+1);
        }
    }

    return ;
}

static void print_usage(int argc, char *argv[])
{
    printf("%s: [thread] [duration(seconds)]\n", argv[0]);
    return ;
}

int main(int argc, char *argv[]) 
{
    if (argc != 3) {
        print_usage(argc, argv);
        return -1;
    }

    int num_trds = atoi(argv[1]);
    int duration = atoi(argv[2]);

    std::vector<std::thread> trds;

    for (int i = 0; i < num_trds; ++i) {
        trds.emplace_back(cpu_load_thread);
    }
    
    auto now = std::chrono::steady_clock::now();
    for (int i = 0; i < duration; ++i) {
        printf("work: %d(%d)\n", i, duration);
        std::this_thread::sleep_for(std::chrono::seconds(1));

        auto now_new = std::chrono::steady_clock::now();
        if (now_new > now + std::chrono::seconds(duration)) {
            printf("end\n");
            break;
        }
    }

    keep_running = false;
    for (auto & ref : trds) {
        ref.join();
    }

    trds.clear();

    return 0;
}
