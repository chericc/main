#include <stdio.h>
#include <thread>

#include "xtimer.hpp"
#include "xlog.hpp"

int main()
{
    xlog_dbg("main in");

    std::shared_ptr<XTimer> timer = std::make_shared<XTimerSimple>();

    auto fun1 = []() { printf("fun1\n"); };
    auto fun2 = []() { printf("fun2\n"); };

    auto now = std::chrono::system_clock::now();

    for (int i = 0; i < 10; ++i)
    {
        timer->createTimer(fun1, std::chrono::milliseconds(i * 1000));
        timer->createTimer(fun2, now + std::chrono::milliseconds(i * 1000));
    }

    std::this_thread::sleep_for(std::chrono::seconds(50));

    timer.reset();

    xlog_dbg("main out");

    return 0;
}