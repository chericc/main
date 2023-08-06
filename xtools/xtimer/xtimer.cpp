#include "xtimer.hpp"

#include <queue>
#include <map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <limits>

#include "xlog.hpp"
#include "xthread.hpp"

/*********************************** TYPES *********************************/

struct XTimerTaskCtx
{
    XTimerFunc task{};
    XTimerDuration duration{};

    std::chrono::time_point<std::chrono::system_clock> tp_start_system{};
    std::chrono::time_point<std::chrono::steady_clock> tp_start_steady{};
};

using XTimerTaskCtxPtr = std::shared_ptr<XTimerTaskCtx>;

struct XTimerIndependent::PrivateData
{
public:
    XTimerType timer_type{Realtime};

    std::shared_ptr<std::priority_queue<XTimerTaskCtxPtr>> task_queue_systemclock;
    std::shared_ptr<std::priority_queue<XTimerTaskCtxPtr>> task_queue_steadyclock;
    std::map<XTimerID, XTimerTaskCtxPtr> task_map;
    std::mutex mutex_task;
    std::condition_variable cond_task;

    XTimerID current_id{0};

    std::shared_ptr<XThread> trd_worker;
    bool break_flag{false};
};

/*********************************** DEFS *********************************/

XTimerIndependent::XTimerIndependent(XTimerType type)
{
    std::lock_guard<std::mutex> lock_call(mutex_call);
    _d = init();
    if (!_d)
    {
        xlog_err("init failed");
    }
}

XTimerIndependent::~XTimerIndependent()
{
    std::lock_guard<std::mutex> lock_call(mutex_call);
    destroy();
}

bool XTimerIndependent::ok()
{
    std::lock_guard<std::mutex> lock_call(mutex_call);
    return (_d != nullptr);
}

XTimerID XTimerIndependent::createTimer(XTimerFunc task, XTimerDuration duration)
{
    XTimerID id = XTIMER_ID_INVALID;

    do 
    {
        if (!_d)
        {
            xlog_err("null");
            break;
        }

        XTimerTaskCtx ctx = {
            .task = task,
            .duration = duration,
            .tp_start_system = std::chrono::system_clock::now(),
            .tp_start_steady = std::chrono::steady_clock::now()
        };
        auto ctx_ptr = std::make_shared<XTimerTaskCtx>(ctx);

        std::lock_guard<std::mutex> lock_task(_d->mutex_task);

        if (_d->task_map.size() >= XTIMER_MAX_TASK_NB)
        {
            xlog_err("task full");
            break;
        }

        // generate id
        // !!! could consume a lot cpu time
        // may refer to linux_kernel.pid alg
        XTimerID idtmp = _d->current_id;
        bool id_found_flag = false;
        for (int k = 0; k <= std::numeric_limits<XTimerID>().max(); ++k)
        {
            if (_d->task_map.find(idtmp) != _d->task_map.end())
            {
                ++idtmp;
                if (idtmp < 0)
                {
                    idtmp = 0;
                }
            }
            else 
            {
                id_found_flag = true;
            }
        }

        if (!id_found_flag)
        {
            xlog_err("no id available");
            break;
        }

        xlog_dbg("id=%d", (int)idtmp);

        _d->current_id = idtmp;
        _d->task_map.emplace(idtmp, ctx_ptr);
        _d->task_queue_steadyclock->push(ctx_ptr);
        _d->task_queue_systemclock->push(ctx_ptr);

        id = idtmp;
    }
    while (0);

    return id;
}

int XTimerIndependent::destroyTimer(XTimerID id)
{
    bool berror_flag = false;

    do 
    {
        
    }
    while(0);


}

std::shared_ptr<XTimerIndependent::PrivateData> XTimerIndependent::init()
{
    std::shared_ptr<PrivateData> data;

    do 
    {
        data = std::make_shared<PrivateData>();

        auto lambda_comp_system_clock = [](const XTimerTaskCtxPtr &r1, const XTimerTaskCtxPtr &r2)
        {
            auto tp1 = r1->tp_start_system + r1->duration;
            auto tp2 = r2->tp_start_system + r2->duration;
            return tp1 < tp2;
        };
        auto lambda_comp_steady_clock = [](const XTimerTaskCtxPtr &r1, const XTimerTaskCtxPtr &r2)
        {
            auto tp1 = r1->tp_start_steady + r1->duration;
            auto tp2 = r2->tp_start_steady + r2->duration;
            return tp1 < tp2;
        };

        data->task_queue_systemclock = std::make_shared<std::priority_queue<XTimerTaskCtxPtr>>(lambda_comp_system_clock);
        data->task_queue_systemclock = std::make_shared<std::priority_queue<XTimerTaskCtxPtr>>(lambda_comp_steady_clock);

        auto lambda_func = [&](){return this->work_loop();};

        data->trd_worker = std::make_shared<XThread>(lambda_func);
        data->trd_worker->start();
    }
    while (0);
    
    return data;
}

