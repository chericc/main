#pragma once

#include <memory>
#include <chrono>
#include <functional>

enum XTimerType 
{
    Realtime,
    Monotonic,
};

using XTimerID = nullptr_t;
using XTimerTask = std::function<void(void)>;
using XTimerDuration = std::chrono::milliseconds;

class XTimer
{
public:

    virtual ~XTimer() = 0;

    virtual XTimerID createTimer(XTimerTask task, XTimerDuration interval) = 0;
    virtual int destroyTimer(XTimerID XTimerID) = 0;
    virtual XTimerDuration updateTimer(XTimerID XTimerID, XTimerDuration interval) = 0;
};

class XTimerIndependent : public XTimer
{
public:
    XTimerIndependent(XTimerType type);
    ~XTimerIndependent() override;
    XTimerID createTimer(XTimerTask task, XTimerDuration interval) override;
    int destroyTimer(XTimerID XTimerID) override;
    XTimerDuration updateTimer(XTimerID XTimerID, XTimerDuration interval) override;
private:
    struct PrivateData;
    std::unique_ptr<PrivateData> _d;
};
