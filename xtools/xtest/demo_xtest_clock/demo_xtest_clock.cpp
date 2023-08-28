#include <thread>
#include "xtest_clock.hpp"
#include "xlog.hpp"
#include "xthread.hpp"

#include "inttypes.h"

XTestClock s_clock;

void funA()
{
    xlog_dbg("in");

    for (int i = 0; i < 3; ++i)
    {
        int seconds = 1;
        xlog_dbg("wait for begin: (%d seconds)", seconds);
        s_clock.waitFor(std::chrono::seconds(seconds));
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
        XTestClock::Timepoint tp = now + std::chrono::seconds(i + 1);
        xlog_dbg("wait until begin: (%" PRId64 ")", (int64_t)std::chrono::time_point_cast<std::chrono::seconds>(tp).time_since_epoch().count());
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