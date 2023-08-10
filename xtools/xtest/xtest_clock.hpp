#pragma once

#include <chrono>
#include <memory>
#include <mutex>

/**
 * 模拟一个系统时钟，这个时钟可以调整时间；
 * 
 * 这个时钟提供了基于其时间的等待接口，对于时间的修改会影响到等待接口的行为；
*/

class XTestClock
{
public:
    using Clock = std::chrono::steady_clock;
    using Timepoint = Clock::time_point;
    using Duration = std::chrono::milliseconds;

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
XTestClock::Timepoint xtestclock_generate_timepoint(const std::string &str);
