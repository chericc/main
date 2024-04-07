#pragma once

#include <functional>
#include <thread>
#include <unordered_map>
#include <memory>

struct XThreadPoolConfig
{
    int core_pool_size = 0;
    int maximum_pool_size = 1;
    int keep_alive_time_ms = 0;
};

class XThreadPool
{
public:
    XThreadPool(const XThreadPoolConfig &config);
    ~XThreadPool();

    void addTask(std::function<void(void)> func);
private:
    typedef std::thread::id T_ID_;
    typedef std::shared_ptr<std::thread> T_Ptr_;
    typedef 
    std::unordered_map<T_ID_, T_Ptr_> pool_worker_;
    // std::un
};