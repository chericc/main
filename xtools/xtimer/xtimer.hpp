#pragma once

#include <memory>
#include <chrono>
#include <functional>
#include <mutex>

#include "xutility.hpp"

using XTimerID = void*;
#define XTIMER_ID_INVALID nullptr

using XTimerFunc = std::function<void(void)>;
using XTimerDuration = std::chrono::milliseconds;

class XTimer : public XNonCopyableObject
{
public:
    virtual ~XTimer() = default;
    virtual bool ok() = 0;
    virtual XTimerID createTimer(XTimerFunc task, XTimerDuration duration) = 0;
    virtual int destroyTimer(XTimerID id) = 0;
};

class XTimerHeap : public XTimer
{
public:
    XTimerHeap();
    ~XTimerHeap() override;
    bool ok() override;
    XTimerID createTimer(XTimerFunc task, XTimerDuration duration) override;
    int destroyTimer(XTimerID id) override;
private:
    struct PrivateData;
    std::shared_ptr<PrivateData> _d;
    std::mutex mutex_call;
private:
    std::shared_ptr<PrivateData> init();
    void destroy();
    void work_loop();
};
