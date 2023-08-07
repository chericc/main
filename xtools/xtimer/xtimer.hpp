#pragma once

#include <memory>
#include <chrono>
#include <functional>

#include "xutility.hpp"

enum XTimerType 
{
    Realtime,
    Monotonic,
};

using XTimerID = int;

enum 
{
    XTIMER_ID_INVALID = -1,
};

enum 
{
    XTIMER_MAX_TASK_NB = 19200,
};

using XTimerFunc = std::function<void(void)>;
using XTimerDuration = std::chrono::milliseconds;

class XTimer
{
public:
    virtual ~XTimer() = 0;

    virtual XTimerID createTimer(XTimerFunc task, XTimerDuration duration) = 0;
    virtual int destroyTimer(XTimerID id) = 0;
    virtual XTimerDuration updateTimer(XTimerID id, XTimerDuration duration) = 0;
};

class XTimerHeap : public XTimer, public XNonCopyableObject
{
public:
    XTimerHeap(XTimerType type);
    ~XTimerHeap() override;
    bool ok();
    XTimerID createTimer(XTimerFunc task, XTimerDuration duration) override;
    int destroyTimer(XTimerID id) override;
    XTimerDuration updateTimer(XTimerID id, XTimerDuration duration) override;
private:
    struct PrivateData;
    std::shared_ptr<PrivateData> _d;
    std::mutex mutex_call;
private:
    std::shared_ptr<PrivateData> init();
    void destroy();
    void work_loop();
};
