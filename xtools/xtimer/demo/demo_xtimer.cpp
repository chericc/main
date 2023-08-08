#include <stdio.h>
#include <thread>

#include "xtimer.hpp"
#include "xlog.hpp"

int main()
{
    xlog_dbg("main in");

    std::shared_ptr<XTimer> timer = std::make_shared<XTimerHeap>();

    auto fun1 = []() { printf("hello 1\n"); };
    auto fun2 = []() { printf("hello 2\n"); };

    timer->createTimer(fun1, std::chrono::milliseconds(1000));
    timer->createTimer(fun2, std::chrono::milliseconds(2000));

    std::this_thread::sleep_for(std::chrono::seconds(5));

    xlog_dbg("main out");

    return 0;
}