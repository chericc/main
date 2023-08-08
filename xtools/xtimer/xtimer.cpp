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

struct XTimerTaskCtx
{
    XTimerFunc task{};
    XTimerDuration duration{};

    bool valid_flag{false};

    std::chrono::time_point<std::chrono::steady_clock> tp_start_steady{};
};

using XTimerTaskCtxPtr = std::shared_ptr<XTimerTaskCtx>;

struct XTimerTaskCtxCmp
{
    bool operator()(XTimerTaskCtxPtr r1, XTimerTaskCtxPtr r2)
    {
        auto tp1 = r1->tp_start_steady + r1->duration;
        auto tp2 = r2->tp_start_steady + r2->duration;
        return tp1 < tp2;
    }
};

using XTimerTaskQueueT = std::priority_queue<XTimerTaskCtxPtr, std::vector<XTimerTaskCtxPtr>, XTimerTaskCtxCmp>;

struct XTimerHeap::PrivateData
{
    std::shared_ptr<XTimerTaskQueueT> queue_task;
    std::mutex mutex_task;
    std::condition_variable cond_task;

    std::shared_ptr<XThread> trd_worker;
    bool break_flag{false};
};

/*********************************** DEFS *********************************/

XTimerHeap::XTimerHeap()
{
    std::lock_guard<std::mutex> lock_call(mutex_call);
    _d = init();
    if (!_d)
    {
        xlog_err("init failed");
    }
    else 
    {
        _d->trd_worker->start();
    }
}

XTimerHeap::~XTimerHeap()
{
    std::lock_guard<std::mutex> lock_call(mutex_call);
    destroy();
}

bool XTimerHeap::ok()
{
    std::lock_guard<std::mutex> lock_call(mutex_call);
    return (_d != nullptr);
}

XTimerID XTimerHeap::createTimer(XTimerFunc task, XTimerDuration duration)
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
        ctx.duration = duration;
        ctx.tp_start_steady = std::chrono::steady_clock::now();
        ctx.valid_flag = true;

        auto ctx_ptr = std::make_shared<XTimerTaskCtx>(ctx);

        {
            std::lock_guard<std::mutex> lock_task(_d->mutex_task);
            _d->queue_task->push(ctx_ptr);
            _d->cond_task.notify_one();
        }

        id = ctx_ptr.get();
    }
    while (0);

    return id;
}

int XTimerHeap::destroyTimer(XTimerID id)
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

        std::size_t size_queue_old = _d->queue_task->size();
        auto newq = std::make_shared<XTimerTaskQueueT>();

        std::lock_guard<std::mutex> lock_task(_d->mutex_task);
        while (!_d->queue_task->empty())
        {
            if (_d->queue_task->top().get() != id)
            {
                newq->push(_d->queue_task->top());
            }
            _d->queue_task->pop();
        }
        _d->queue_task = newq;

        if (newq->size() == size_queue_old)
        {
            xlog_err("id(%p) not found", id);
            berror_flag = true;
        }
        
        // 令loop重新选择任务
        _d->cond_task.notify_one();
    }
    while(0);

    return berror_flag ? -1 : 0;
}

std::shared_ptr<XTimerHeap::PrivateData> XTimerHeap::init()
{
    std::shared_ptr<PrivateData> data;

    do 
    {
        data = std::make_shared<PrivateData>();
        data->queue_task = std::make_shared<XTimerTaskQueueT>();

        auto lambda_func = [&](){return this->work_loop();};
        data->trd_worker = std::make_shared<XThread>(lambda_func);
    }
    while (0);
    
    return data;
}

void XTimerHeap::destroy()
{
    do 
    {
        if (!_d)
        {
            break;
        }

        xlog_dbg("destroying");

        _d->break_flag = true;
        _d->cond_task.notify_one();
        _d->trd_worker->join();

        _d.reset();
    }
    while (0);

    return ;
}

void XTimerHeap::work_loop()
{
    while (true)
    {
        if (_d->break_flag)
        {
            xlog_dbg("break");
            break;
        }

        std::unique_lock<std::mutex> lock_task(_d->mutex_task);
        while (_d->queue_task->empty())
        {
            xlog_trc("no job, waiting");
            _d->cond_task.wait(lock_task);
        }

        XTimerTaskCtxPtr top_task = _d->queue_task->top();
        auto timepoint_task = top_task->tp_start_steady + top_task->duration;

        auto wait_ret = _d->cond_task.wait_until(lock_task, timepoint_task);

        if (wait_ret == std::cv_status::timeout)
        {
            xlog_trc("run job");
            top_task->task();
            _d->queue_task->pop();
        }
    }
}