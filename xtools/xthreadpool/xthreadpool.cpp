#include "xthreadpool.hpp"

#include "xlog.hpp"

XThreadPool::ThreadContainer::ThreadContainer(Notify notify)
{
    notify_ = notify;
}

void XThreadPool::ThreadContainer::setTask(const Task &task)
{
    std::unique_lock task_lock(mutex_task_);

    task_ = std::make_shared<Task>(task);

    if (!trd_ptr_)
    {
        trd_ptr_ = std::make_shared<std::thread>([this](){ this->run(); });
    }

    cond_have_task_.notify_one();
}

std::thread::id XThreadPool::ThreadContainer::id()
{
    if (trd_ptr_)
    {
        return trd_ptr_->get_id();
    }
    return std::thread::id();
}

void XThreadPool::ThreadContainer::run()
{
    NotifyInfo info;

    while (true)
    {
        std::unique_lock task_lock(mutex_task_);

        idle_flag_ = false;

        if (task_)
        {
            (*task_)();
            task_ = nullptr;
        }

        idle_flag_ = true;

        info.job_finished_thread_id = std::this_thread::get_id();
        notify_(info);

        cond_have_task_.wait(task_lock);
    }
}

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

void XThreadPool::addTask(const Task &task)
{
    std::lock_guard lock_call(mutex_call_);

    std::unique_lock lock_pool(mutex_pool_);

    // case 1: have idle threads
    // case 2: total threads number < max

    // case 1
    if (!pool_idle_.empty())
    {
        auto begin = pool_idle_.begin();
        auto item_trd = *begin;
        pool_idle_.erase(begin);

        item_trd.second->setTask(task);
        pool_working_.insert(item_trd);
    }
    else 
    {
        // pool_idle is empty here

        // case 2
        if (pool_working_.size() <= config_.maximum_pool_size)
        {
            auto new_trd_item = std::make_pair(TRD_ID(), TRD_Ptr());
            new_trd_item.second = std::make_shared<ThreadContainer>(
                [this](const NotifyInfo &info){ this->onNotify(info); });
            new_trd_item.second->setTask(task);
            pool_working_.insert(new_trd_item);
        }
        else 
        {
            cond_pool_not_full_.wait(lock_pool);
        }
    }

    return ;
}

void XThreadPool::onNotify(const NotifyInfo &info)
{
    std::unique_lock lock_pool(mutex_pool_);

    do 
    {
        auto finished_iterator = pool_working_.find(info.job_finished_thread_id);
        if (finished_iterator != pool_working_.end())
        {
            auto finished_item = *finished_iterator;
            pool_working_.erase(finished_iterator);
            
            pool_idle_.insert(finished_item);
        }
    }
    while (0);

    return ;
}