#include "xthreadpool.hpp"

XThreadPool::XThreadPool(const XThreadPoolConfig &config)
{
    config_ = config;
}

XThreadPool::~XThreadPool()
{
    std::lock_guard lock_call(mutex_call_);

    for (auto &ref : pool_idle_)
    {
        if (ref.second && ref.second->joinable()) 
        {
            ref.second->join();
        }
    }
    for (auto &ref : pool_working_)
    {
        if (ref.second && ref.second->joinable())
        {
            ref.second->join();
        }
    }
    pool_idle_.clear();
    pool_working_.clear();
}

void XThreadPool::addTask(std::function<void(void)> task)
{
    std::lock_guard lock_call(mutex_call_);

    
}