#include <stdio.h>
#include <thread>

#include "xtimer.hpp"
#include "xlog.hpp"

int main()
{
    xlog_dbg("main in");

    std::shared_ptr<XTimer> timer = std::make_shared<XTimerHeap>();

    auto fun = []() { printf("hello\n"); };

    for (int i = 0; i < 10; ++i)
    {
        timer->createTimer(fun, std::chrono::milliseconds(i * 1000));
    }

    std::this_thread::sleep_for(std::chrono::seconds(50));

    timer.reset();

    xlog_dbg("main out");

    return 0;
}