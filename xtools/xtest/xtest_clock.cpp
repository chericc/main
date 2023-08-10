#include "xtest_clock.hpp"

#include <condition_variable>
#include <time.h>

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
    if (!strptime(str.c_str(), "%F %T", &stm))
    {
        return XTestClock::Timepoint{};
    }
    
    time_t time_input = mktime(&stm);
    XTestClock::Clock::duration dur(
        std::chrono::duration_cast<XTestClock::Duration>(std::chrono::seconds(time_input)));
    return XTestClock::Timepoint{} + dur;
}
