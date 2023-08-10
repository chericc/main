#include "xtest_clock.hpp"

#include <condition_variable>
#include <time.h>

#include <stdio.h>

#include "xlog.hpp"

struct XTestClock::InnerData
{
    // dur_diff越大，则表示虚拟时间越快，或者说等待越提前返回；
    // XTestClock::now() = Clock::now() + dur_diff
    Duration dur_diff;

    std::condition_variable cond;
};

XTestClock::XTestClock()
{
    _d = init();
    if (!_d)
    {
        xlog_err("init failed");
    }
}

bool XTestClock::ok()
{
    std::lock_guard<std::mutex> lock_call(_mutex_call);

    return (_d ? true : false);
}

XTestClock::Timepoint XTestClock::now()
{
    std::lock_guard<std::mutex> lock_call(_mutex_call);

    if (!_d)
    {
        xlog_err("null");
        return Timepoint{};
    }

    return Clock::now() + _d->dur_diff;
}

void XTestClock::move(Duration duration)
{
    std::lock_guard<std::mutex> lock_call(_mutex_call);

    if (!_d)
    {
        xlog_err("null");
        return ;
    }

    _d->dur_diff += duration;

    _d->cond.notify_all();
}

void XTestClock::jump(Timepoint timepoint)
{
    std::lock_guard<std::mutex> lock_call(_mutex_call);

    if (!_d)
    {
        xlog_err("null");
        return ;
    }

    _d->dur_diff = std::chrono::duration_cast<Duration>(timepoint - Clock::now());

    _d->cond.notify_all();
}

void XTestClock::waitUntil(Timepoint timepoint)
{
    std::unique_lock<std::mutex> lock_call(_mutex_call);

    if (!_d)
    {
        xlog_err("null");
        return ;
    }

    while (true)
    {
        auto real_timepoint = timepoint - _d->dur_diff;
        auto ret = _d->cond.wait_until(lock_call, real_timepoint);
        if (ret == std::cv_status::timeout)
        {
            break;
        }
    }
}

void XTestClock::waitFor(Duration duration)
{
    std::unique_lock<std::mutex> lock_call(_mutex_call);

    if (!_d)
    {
        xlog_err("null");
        return ;
    }

    Duration old_dur_diff = _d->dur_diff;

    while (true)
    {
        Duration new_dur_diff = _d->dur_diff;
        Duration diff = new_dur_diff - old_dur_diff;

        auto real_timepoint = Clock::now() + duration - diff;
        auto ret = _d->cond.wait_until(lock_call, real_timepoint);
        if (ret == std::cv_status::timeout)
        {
            break;
        }
    }
}

std::shared_ptr<XTestClock::InnerData> XTestClock::init()
{
    std::shared_ptr<XTestClock::InnerData> data = 
        std::make_shared<XTestClock::InnerData>();
    
    return data;
}

XTestClock::Timepoint xtestclock_generate_timepoint(const std::string &str)
{
    struct tm stm{};

    /* str eg: "2010-01-01 15:30:55" */

    int year = 0, month = 0, day = 0;
    int hour = 0, minute = 0, second = 0;
    int ret = sscanf(str.c_str(), "%d-%d-%d %d:%d:%d",
        &year, &month, &day, &hour, &minute, &second);
    if (ret != 6)
    {
        xlog_err("invalid input string");
        return XTestClock::Timepoint{};
    }

    stm.tm_year = 1900 + year;
    stm.tm_mon = month - 1;
    stm.tm_mday = day;
    stm.tm_hour = hour;
    stm.tm_min = minute;
    stm.tm_sec = second;
    
    time_t time_input = mktime(&stm);

    if (time_input == (time_t)(-1))
    {
        xlog_err("invalid input string");
        return XTestClock::Timepoint{};
    }

    XTestClock::Clock::duration dur(
        std::chrono::duration_cast<XTestClock::Duration>(std::chrono::seconds(time_input)));
    return XTestClock::Timepoint{} + dur;
}
