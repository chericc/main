#include <thread>
#include "xtest_clock.hpp"
#include "xlog.hpp"
#include "xthread.hpp"

XTestClock s_clock;

void funA()
{
    xlog_dbg("in");

    for (int i = 0; i < 3; ++i)
    {
        xlog_dbg("wait begin");
        s_clock.waitFor(std::chrono::seconds(1));
        xlog_dbg("wait end");
    }

    xlog_dbg("out");
}

void funB()
{
    xlog_dbg("in");

    auto now = XTestClock::Clock::now();
    for (int i = 0; i < 3; ++i)
    {
        XTestClock::Timepoint tp = now + std::chrono::seconds(1);
        xlog_dbg("wait begin");
        s_clock.waitUntil(std::move(tp));
        xlog_dbg("wait end");
    }

    xlog_dbg("out");
}

int main()
{
    XThread trd1(funA);
    XThread trd2(funB);

    trd1.start();
    trd1.join();

    trd2.start();
    trd2.join();

    return 0;
}