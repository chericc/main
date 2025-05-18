#include <gtest/gtest.h>

#include <thread>

#include "xlog.h"
#include "xtest_clock.hpp"

TEST(xtestclock, init) {
    XTestClock clock;
    EXPECT_TRUE(clock.ok());

    // EXPECT_TRUE(clock.now() == XTestClock::Timepoint{});
}

TEST(xtestclock, now) {
    XTestClock clock;

    XTestClock::Timepoint tp =
        xtestclock_generate_timepoint(std::string("2000-01-01 15:30:00"));

    auto start_tp = std::chrono::steady_clock::now();
    clock.jump(tp);
    auto now = clock.now();
    auto end_tp = std::chrono::steady_clock::now();

    auto dur_timepass = end_tp - start_tp;

    XTestClock::Timepoint tp2 = tp + dur_timepass;

    EXPECT_TRUE(now >= tp);
    EXPECT_TRUE(now <= tp2);
}

TEST(xtestclock, move) {
    XTestClock clock;

    auto dur_move_forward = std::chrono::seconds(5);
    auto tp_op_begin = std::chrono::steady_clock::now();
    auto tp_begin = clock.now();
    clock.move(dur_move_forward);
    auto tp_moved = clock.now();
    auto tp_op_end = std::chrono::steady_clock::now();

    EXPECT_TRUE(tp_moved - tp_begin >= dur_move_forward);
    EXPECT_TRUE(tp_moved - tp_begin <=
                dur_move_forward + (tp_op_end - tp_op_begin));
}

TEST(xtestclock, jump) {
    XTestClock clock;

    XTestClock::Timepoint tp =
        xtestclock_generate_timepoint(std::string("2000-01-01 15:30:00"));
    auto tp_op_begin = std::chrono::steady_clock::now();
    clock.jump(tp);
    auto tp_now = clock.now();
    auto tp_op_end = std::chrono::steady_clock::now();

    EXPECT_TRUE(tp_now >= tp);
    EXPECT_TRUE(tp_now <= tp_now + (tp_op_end - tp_op_begin));
}

TEST(xtestclock, jumpandmove) {
    XTestClock clock;

    XTestClock::Timepoint tp1 =
        xtestclock_generate_timepoint(std::string("2000-01-01 15:30:00"));
    XTestClock::Timepoint tp2 =
        xtestclock_generate_timepoint(std::string("2000-01-01 15:30:05"));

    auto tp_op_begin = std::chrono::steady_clock::now();
    clock.jump(tp1);
    clock.move(tp2 - tp1);
    auto tp_now = clock.now();
    auto tp_op_end = std::chrono::steady_clock::now();

    EXPECT_TRUE(tp_now >= tp2);
    EXPECT_TRUE(tp_now <= tp2 + (tp_op_end - tp_op_begin));
}

TEST(xtestclock, waituntil) {
    XTestClock clock;
    XTestClock::Duration wait_op_duration;
    XTestClock::Duration wait_length(std::chrono::seconds(5));
    XTestClock::Timepoint now = clock.now();

    auto fun1 = [&]() {
        auto op_start = std::chrono::steady_clock::now();
        clock.waitUntil(now + wait_length);
        auto op_end = std::chrono::steady_clock::now();
        wait_op_duration = op_end - op_start;
    };

    std::thread trd(fun1);
    clock.move(wait_length);
    trd.join();

    EXPECT_TRUE(wait_op_duration >= XTestClock::Duration(0));

    //!!! NOTE: for OS's scheduling reason, this test
    // may fail.
    EXPECT_TRUE(wait_op_duration < wait_length);
}

TEST(xtestclock, waitfor) {
    XTestClock clock;
    XTestClock::Duration wait_op_duration;
    XTestClock::Duration wait_length(std::chrono::seconds(5));
    XTestClock::Timepoint now = clock.now();

    auto fun1 = [&]() {
        auto op_start = std::chrono::steady_clock::now();
        clock.waitUntil(now + wait_length);
        auto op_end = std::chrono::steady_clock::now();
        wait_op_duration = op_end - op_start;
    };

    std::thread trd(fun1);

    // Block this thread for `thd` thread to run into blocked state.
    //!!! NOTE:  The duration below may not be enough.
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    clock.move(wait_length);
    trd.join();

    EXPECT_TRUE(wait_op_duration >= XTestClock::Duration(0));

    //!!! NOTE: for OS's scheduling reason, this test
    // may fail.
    EXPECT_TRUE(wait_op_duration < wait_length);
}
