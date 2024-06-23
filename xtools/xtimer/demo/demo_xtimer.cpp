#include <stdio.h>

#include <thread>

#include "xlog.hpp"
#include "xtimer.hpp"

int main() {
    xlog_dbg("main in");

    std::shared_ptr<XTimer> timer = std::make_shared<XTimerSimple>();

    auto fun1 = []() { xlog_dbg("steady fun"); };
    auto fun2 = []() { xlog_dbg("system fun"); };

    auto now = std::chrono::system_clock::now();

    for (int i = 0; i < 5; ++i) {
        timer->createTimer(fun1, std::chrono::milliseconds(i * 1000));
        timer->createTimer(fun2,
                           now + std::chrono::milliseconds(i * 1000 + 500));
    }

    std::this_thread::sleep_for(std::chrono::seconds(10));

    timer.reset();

    xlog_dbg("main out");

    return 0;
}