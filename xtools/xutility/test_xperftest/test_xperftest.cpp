#include <gtest/gtest.h>

#include <thread>

#include "xlog.hpp"
#include "xperftest.hpp"

TEST(xperftest, base) {
    auto mask = xlog_getmask();
    xlog_setmask(XLOG_ALLOW_INF);

    { X_PERIOD_PRINT(INF, "No sleep", tmp); }

    {
        X_PERIOD_PRINT(INF, "Sleep 1ms", tmp);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    {
        X_PERIOD_PRINT(INF, "Sleep 1s", tmp);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    xlog_setmask(mask);
}