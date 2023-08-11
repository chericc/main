#include <gtest/gtest.h>

#include "xtest_clock.hpp"

#include "xlog.hpp"

TEST(xtestclock, init)
{
    XTestClock clock;
    EXPECT_TRUE(clock.ok());

    // EXPECT_TRUE(clock.now() == XTestClock::Timepoint{});
}

TEST(xtestclock, now)
{
    XTestClock clock;

    XTestClock::Timepoint tp = xtestclock_generate_timepoint(std::string("2000-01-01 15:30:00"));
    
    auto start_tp = std::chrono::steady_clock::now();
    clock.jump(tp);
    auto now = clock.now();
    auto end_tp = std::chrono::steady_clock::now();

    auto dur_timepass = end_tp - start_tp;

    XTestClock::Timepoint tp2 = tp + dur_timepass;

    EXPECT_TRUE(now >= tp);
    EXPECT_TRUE(now <= tp2);
}
