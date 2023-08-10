#include "xtimer.hpp"

#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <limits>

#include "xlog.hpp"
#include "xthread.hpp"

// https://www.cnblogs.com/sunsky303/p/14154190.html

/*********************************** TYPES *********************************/

enum class TaskType
{
    Steady,
    System,
};

struct SteadyTimepoint
{
    XTimerDuration duration{};
    std::chrono::time_point<std::chrono::steady_clock> tp_start_steady{};
};

struct SystemTimepoint
{
    std::chrono::time_point<std::chrono::system_clock> tp_start_system{};
};

union Timepoint
{
    SteadyTimepoint steady;
    SystemTimepoint system;
};

struct XTimerTaskCtx
{
    XTimerTask task{};

    TaskType type{TaskType::Steady};
    Timepoint timepoint{};
};

using XTimerTaskCtxPtr = std::shared_ptr<XTimerTaskCtx>;

struct XTimerTaskCtxGTCmp
{
    bool operator()(XTimerTaskCtxPtr r1, XTimerTaskCtxPtr r2)
    {
        // Note: priority queue returns largest element first
        // if we use less comparator.
        if (TaskType::Steady == r1->type)
        {
            auto tp1 = r1->timepoint.steady.tp_start_steady + 
                r1->timepoint.steady.duration;
            auto tp2 = r2->timepoint.steady.tp_start_steady + 
                r2->timepoint.steady.duration;
            return std::greater<decltype(tp1)>()(tp1, tp2);
        }
        else if (TaskType::System == r1->type)
        {
            auto tp1 = r1->timepoint.system.tp_start_system;
            auto tp2 = r2->timepoint.system.tp_start_system;
            return std::greater<decltype(tp1)>()(tp1, tp2);
        }
        
        xlog_cri("unexpected");
        return true;
    }
};

using XTimerTaskQueueT = std::priority_queue<XTimerTaskCtxPtr, std::vector<XTimerTaskCtxPtr>, XTimerTaskCtxGTCmp>;

struct XTimerSimple::PerThreadCtx
{
    std::shared_ptr<XTimerTaskQueueT> queue_task;
    std::mutex mutex_task;
    std::condition_variable cond_task;
    std::shared_ptr<XThread> trd_worker;
};

struct XTimerSimple::PrivateData
{
    std::shared_ptr<PerThreadCtx> trd_ctx_steady;
    std::shared_ptr<PerThreadCtx> trd_ctx_system;

    bool break_flag{false};
};

/*********************************** DEFS *********************************/

XTimerSimple::XTimerSimple()
{
    std::lock_guard<std::mutex> lock_call(mutex_call);
    _d = init();
    if (!_d)
    {
        xlog_err("init failed");
    }
    else 
    {
        _d->trd_ctx_steady->trd_worker->start();
        _d->trd_ctx_system->trd_worker->start();
    }
}

XTimerSimple::~XTimerSimple()
{
    std::lock_guard<std::mutex> lock_call(mutex_call);
    destroy();
}

bool XTimerSimple::ok()
{
    std::lock_guard<std::mutex> lock_call(mutex_call);
    return (_d != nullptr);
}

XTimerID XTimerSimple::createTimer(XTimerTask &&task, XTimerDuration &&duration)
{
    std::lock_guard<std::mutex> lock_call(mutex_call);

    XTimerID id = XTIMER_ID_INVALID;

    do 
    {
        if (!_d)
        {
            xlog_err("null");
            break;
        }

        XTimerTaskCtx ctx;
        ctx.task = task;
        ctx.type = TaskType::Steady;
        ctx.timepoint.steady.duration = duration;
        ctx.timepoint.steady.tp_start_steady = std::chrono::steady_clock::now();

        auto ctx_ptr = std::make_shared<XTimerTaskCtx>(ctx);

        {
            std::lock_guard<std::mutex> lock_task(_d->trd_ctx_steady->mutex_task);
            _d->trd_ctx_steady->queue_task->push(ctx_ptr);
            _d->trd_ctx_steady->cond_task.notify_one();
        }

        id = ctx_ptr.get();
    }
    while (0);

    return id;
}

XTimerID XTimerSimple::createTimer(XTimerTask &&task, XTimerTimepoint &&timepoint)
{
    std::lock_guard<std::mutex> lock_call(mutex_call);

    XTimerID id = XTIMER_ID_INVALID;

    do 
    {
        if (!_d)
        {
            xlog_err("null");
            break;
        }

        XTimerTaskCtx ctx;
        ctx.task = task;
        ctx.type = TaskType::System;
        ctx.timepoint.system.tp_start_system = timepoint;

        auto ctx_ptr = std::make_shared<XTimerTaskCtx>(ctx);

        {
            std::lock_guard<std::mutex> lock_task(_d->trd_ctx_system->mutex_task);
            _d->trd_ctx_system->queue_task->push(ctx_ptr);
            _d->trd_ctx_system->cond_task.notify_one();
        }

        id = ctx_ptr.get();
    }
    while (0);

    return id;
}

int XTimerSimple::destroyTimer(XTimerID id)
{
    std::lock_guard<std::mutex> lock_call(mutex_call);

    bool berror_flag = false;

    do 
    {
        if (!_d)
        {
            xlog_err("null");
            berror_flag = true;
            break;
        }

        if (!destroyTimerOfQueue(id, _d->trd_ctx_steady))
        {
            break;
        }

        if (!destroyTimerOfQueue(id, _d->trd_ctx_system))
        {
            break;
        }

        berror_flag = true;
    }
    while(0);

    return berror_flag ? -1 : 0;
}

int XTimerSimple::destroyTimerOfQueue(XTimerID id, std::shared_ptr<PerThreadCtx> ctx)
{
    bool berror_flag = false;

    do 
    {
        if (!_d)
        {
            xlog_err("null");
            berror_flag = true;
            break;
        }

        std::size_t size_queue_old = ctx->queue_task->size();
        auto newq = std::make_shared<XTimerTaskQueueT>();

        std::lock_guard<std::mutex> lock_task(ctx->mutex_task);
        while (!ctx->queue_task->empty())
        {
            if (ctx->queue_task->top().get() != id)
            {
                newq->push(ctx->queue_task->top());
            }
            ctx->queue_task->pop();
        }
        ctx->queue_task = newq;

        if (newq->size() == size_queue_old)
        {
            xlog_err("id(%p) not found", id);
            berror_flag = true;
        }
        
        // 令loop重新选择任务
        ctx->cond_task.notify_one();
    }
    while(0);

    return berror_flag ? -1 : 0;
}

std::shared_ptr<XTimerSimple::PrivateData> XTimerSimple::init()
{
    std::shared_ptr<PrivateData> data;

    do 
    {
        data = std::make_shared<PrivateData>();
        data->trd_ctx_steady = std::make_shared<PerThreadCtx>();
        data->trd_ctx_system = std::make_shared<PerThreadCtx>();
        data->trd_ctx_steady->queue_task = std::make_shared<XTimerTaskQueueT>();
        data->trd_ctx_system->queue_task = std::make_shared<XTimerTaskQueueT>();

        auto lambda_func_steady = [this](){return this->workLoop(this->_d->trd_ctx_steady);};
        auto lambda_func_system = [this](){return this->workLoop(this->_d->trd_ctx_system);};
        data->trd_ctx_steady->trd_worker = std::make_shared<XThread>(lambda_func_steady);
        data->trd_ctx_system->trd_worker = std::make_shared<XThread>(lambda_func_system);
    }
    while (0);
    
    return data;
}

void XTimerSimple::destroy()
{
    do 
    {
        if (!_d)
        {
            break;
        }

        xlog_dbg("destroying");

        _d->break_flag = true;
        _d->trd_ctx_steady->cond_task.notify_one();
        _d->trd_ctx_system->cond_task.notify_one();
        _d->trd_ctx_steady->trd_worker->join();
        _d->trd_ctx_system->trd_worker->join();

        _d.reset();
    }
    while (0);

    return ;
}

void XTimerSimple::workLoop(std::shared_ptr<PerThreadCtx> ctx)
{
    while (true)
    {
        if (_d->break_flag)
        {
            xlog_dbg("break");
            break;
        }

        std::unique_lock<std::mutex> lock_task(ctx->mutex_task);
        if (ctx->queue_task->empty())
        {
            xlog_trc("no job, waiting");
            ctx->cond_task.wait(lock_task);
            continue;
        }

        XTimerTaskCtxPtr top_task = ctx->queue_task->top();
        std::cv_status wait_ret{};
        if (TaskType::Steady == top_task->type)
        {
            auto timepoint_task = top_task->timepoint.steady.tp_start_steady + 
                top_task->timepoint.steady.duration;
            wait_ret = ctx->cond_task.wait_until(lock_task, timepoint_task);
        }
        else if (TaskType::System == top_task->type)
        {
            auto timepoint_task = top_task->timepoint.system.tp_start_system;
            wait_ret = ctx->cond_task.wait_until(lock_task, timepoint_task);
        }
        else 
        {
            xlog_cri("unexpected type");
            break;
        }

        if (wait_ret == std::cv_status::timeout)
        {
            xlog_trc("job timeout, running it");
            top_task->task();
            ctx->queue_task->pop();
        }
    }
}