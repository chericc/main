#pragma once

#include <chrono>
#include <functional>
#include <memory>
#include <mutex>

#include "xclass.hpp"

using XTimerID = void*;
#define XTIMER_ID_INVALID nullptr

using XTimerTask = std::function<void(void)>;
using XTimerDuration = std::chrono::milliseconds;
using XTimerTimepoint = std::chrono::time_point<std::chrono::system_clock>;

class XTimer : public XNonCopyableObject {
   public:
    virtual ~XTimer() = default;
    virtual bool ok() = 0;

    /* A task with a duration uses steady clock. */
    virtual XTimerID createTimer(XTimerTask task, XTimerDuration duration) = 0;

    /* A task with a timepoint uses system clock. */
    virtual XTimerID createTimer(XTimerTask task,
                                 XTimerTimepoint timepoint) = 0;
    virtual int destroyTimer(XTimerID id) = 0;
};

class XTimerSimple : public XTimer {
   public:
    XTimerSimple();
    ~XTimerSimple() override;
    bool ok() override;
    XTimerID createTimer(XTimerTask task, XTimerDuration duration) override;
    XTimerID createTimer(XTimerTask task, XTimerTimepoint timepoint) override;
    int destroyTimer(XTimerID id) override;

   private:
    struct PrivateData;
    struct PerThreadCtx;
    std::shared_ptr<PrivateData> _d;
    std::mutex mutex_call;

   private:
    std::shared_ptr<PrivateData> init();
    void destroy();
    void workLoop(std::shared_ptr<PerThreadCtx>);
    int destroyTimerOfQueue(XTimerID id, std::shared_ptr<PerThreadCtx>);
};
