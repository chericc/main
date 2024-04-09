#pragma once

#include <functional>
#include <thread>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <utility>

struct XThreadPoolConfig
{
    std::size_t core_pool_size = 0;
    std::size_t maximum_pool_size = 1;
    std::size_t keep_alive_time_ms = 0;
};


class XThreadPool
{
public:
    using Task = std::function<void(void)>;

    XThreadPool(const XThreadPoolConfig &config);
    ~XThreadPool();

    void addTask(const Task &task);
    void waitTasks();
private:

    struct NotifyInfo
    {
        std::thread::id job_finished_thread_id;
    };

    class ThreadContainer
    {
    public:
        using Notify = std::function<void(const NotifyInfo &)>;

        explicit ThreadContainer(Notify notify);

        void setTask(const Task &task);
        std::thread::id id();
        bool joinable();
        void exit();
        void join();
        
    private:
        void run();

        std::shared_ptr<Task> task_;
        std::shared_ptr<std::thread> trd_ptr_;
        bool idle_flag_ = false;
        bool exit_flag_ = false;
        Notify notify_;

        std::mutex mutex_call_;
        std::mutex mutex_task_;
        std::condition_variable cond_have_task_;
    };

    typedef std::thread::id TRD_ID;
    typedef std::shared_ptr<ThreadContainer> TRD_Ptr;
    typedef std::unordered_map<TRD_ID, TRD_Ptr> TRD_POOL;

    void onNotify(const NotifyInfo &info);

    // pools
    TRD_POOL pool_working_;
    TRD_POOL pool_idle_;

    // mutex of pool
    std::mutex mutex_pool_;

    // conds
    std::condition_variable cond_pool_not_all_busy_;
    std::condition_variable cond_pool_all_idle_;
    
    // local storage
    XThreadPoolConfig config_;

    // mutex of calls
    std::mutex mutex_call_;
};