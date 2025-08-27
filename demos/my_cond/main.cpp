#include <chrono>
#include <mutex>
#include <thread>

#if defined (USE_STD_COND)
#include <condition_variable>
#else
#include "my_cond.hpp"
#endif 

#include "xlog.h"

using Unilock = std::unique_lock<std::mutex>;
using Clock = std::chrono::steady_clock;

namespace {
std::mutex s_mutex;
#if defined (USE_STD_COND)
std::condition_variable s_cond;
#else 
cyan::condition_variable s_cond;
#endif 
}

void trd()
{
    Unilock lock(s_mutex);

    auto now = Clock::now();
    auto dead = now + std::chrono::seconds(10);

    xlog_dbg("wait begin\n");
    s_cond.wait_until(lock, dead);
    xlog_dbg("wait end\n");
}

int main()
{
    std::thread run(trd);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    getchar();

    s_cond.notify_one();

    run.join();

    return 0;
}