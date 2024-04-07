#pragma once

#include <functional>
#include <thread>
#include <unordered_map>
#include <memory>
#include <mutex>

struct XThreadPoolConfig
{
    std::size_t core_pool_size = 0;
    std::size_t maximum_pool_size = 1;
    std::size_t keep_alive_time_ms = 0;
};

class XThreadPool
{
public:
    XThreadPool(const XThreadPoolConfig &config);
    ~XThreadPool();

    void addTask(std::function<void(void)> task);
private:
    bool prepareIdleWorker();

    typedef std::thread::id TRD_ID;
    typedef std::shared_ptr<std::thread> TRD_Ptr;
    typedef std::unordered_map<TRD_ID, TRD_Ptr> TRD_POOL;
    TRD_POOL pool_working_;
    TRD_POOL pool_idle_;
    std::mutex mutex_call_;
    std::mutex mutex_pool_;
    XThreadPoolConfig config_;
};