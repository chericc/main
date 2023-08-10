#include <gtest/gtest.h>

#include "xtest_clock.hpp"

#include "xlog.hpp"

TEST(xtestclock, init)
{
    XTestClock clock;
    EXPECT_TRUE(clock.ok());
}

TEST(xtestclock, now)
{
    XTestClock clock;
    
    XTestClock::Timepoint tp = xtestclock_generate_timepoint(std::string("2000-01-01 15:30:00"));
    XTestClock::Timepoint tp2 = xtestclock_generate_timepoint(std::string("2000-01-01 15:30:05"));
    clock.jump(tp);
    auto now = clock.now();

    xlog_dbg("tp=%lld,now=%lld,tp2=%lld",
        tp.time_since_epoch().count(),
        now.time_since_epoch().count(),
        tp2.time_since_epoch().count());

    EXPECT_TRUE(now >= tp);
    EXPECT_TRUE(now <= tp2);
}