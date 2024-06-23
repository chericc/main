#pragma once

#include <chrono>
#include <memory>
#include <mutex>

/**
 * A simulation for system clock.
 */

class XTestClock {
   public:
    using Clock = std::chrono::steady_clock;
    using Timepoint = Clock::time_point;
    using Duration = Clock::duration;

    XTestClock();

    bool ok();
    Timepoint now();

    void move(Duration duration);
    void jump(Timepoint timepoint);

    void waitUntil(Timepoint timepoint);
    void waitFor(Duration duration);

   private:
    struct InnerData;
    std::shared_ptr<InnerData> _d;
    std::mutex _mutex_call;

   private:
    std::shared_ptr<InnerData> init();
};

/* str eg: "2010-01-01 15:30:55" */
XTestClock::Timepoint xtestclock_generate_timepoint(const std::string& str);
