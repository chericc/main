#include "xthreadpool.hpp"

#include <iomanip>

#include "xlog_cxx.hpp"

XThreadPool::ThreadContainer::ThreadContainer(Notify notify) {
    notify_ = notify;
}

void XThreadPool::ThreadContainer::setTask(const Task& task) {
    std::unique_lock<std::mutex> call_lock(mutex_call_);
    std::unique_lock<std::mutex> task_lock(mutex_task_);

    task_ = std::make_shared<Task>(task);

    if (!trd_ptr_) {
        trd_ptr_ = std::make_shared<std::thread>([this]() { this->run(); });
    }

    cond_have_task_.notify_one();
}

std::thread::id XThreadPool::ThreadContainer::id() {
    std::unique_lock<std::mutex> call_lock(mutex_call_);

    if (trd_ptr_) {
        return trd_ptr_->get_id();
    }
    return std::thread::id();
}

bool XThreadPool::ThreadContainer::joinable() {
    std::unique_lock<std::mutex> call_lock(mutex_call_);

    if (trd_ptr_) {
        return true;
    }
    return false;
}

void XThreadPool::ThreadContainer::exit() {
    std::unique_lock<std::mutex> call_lock(mutex_call_);
    std::unique_lock<std::mutex> task_lock(mutex_task_);

    exit_flag_ = true;
    cond_have_task_.notify_one();
}

void XThreadPool::ThreadContainer::join() {
    std::unique_lock<std::mutex> call_lock(mutex_call_);

    if (trd_ptr_) {
        return trd_ptr_->join();
    }
    return;
}

void XThreadPool::ThreadContainer::run() {
    NotifyInfo info;
    auto this_id = std::this_thread::get_id();

    xlog_dbg("thread begin");

    while (true) {
        std::unique_lock<std::mutex> task_lock(mutex_task_);

        if (exit_flag_) {
            break;
        }

        idle_flag_ = false;

        if (task_) {
            xlog_dbg("work begin");
            (*task_)();
            task_ = nullptr;
            xlog_dbg("work end");
        }

        idle_flag_ = true;

        info.job_finished_thread_id = this_id;
        notify_(info);

        cond_have_task_.wait(task_lock);
    }

    xlog_dbg("thread end");
}

XThreadPool::XThreadPool(const XThreadPoolConfig& config) { config_ = config; }

XThreadPool::~XThreadPool() {
    std::lock_guard<std::mutex> lock_call(mutex_call_);

    for (auto& ref : pool_idle_) {
        if (ref.second && ref.second->joinable()) {
            ref.second->exit();
            ref.second->join();
        }
    }
    for (auto& ref : pool_working_) {
        if (ref.second && ref.second->joinable()) {
            ref.second->exit();
            ref.second->join();
        }
    }
    pool_idle_.clear();
    pool_working_.clear();
}

void XThreadPool::addTask(const Task& task) {
    std::lock_guard<std::mutex> lock_call(mutex_call_);
    std::unique_lock<std::mutex> lock_pool(mutex_pool_);

    // case 1: have idle threads, use idle threads
    // case 2: total threads number < max, create new thread
    // case 3: wait for idle threads

    while (true) {
        // case 1
        if (!pool_idle_.empty()) {
            xlog_dbg("use idle pool");

            auto begin = pool_idle_.begin();
            auto item_trd = *begin;
            pool_idle_.erase(begin);

            item_trd.second->setTask(task);
            pool_working_.insert(item_trd);
            break;
        }

        // case 2
        if (pool_working_.size() + pool_idle_.size() <
            config_.maximum_pool_size) {
            xlog_dbg("create worker to working pool");

            auto new_trd_item = std::make_pair(TRD_ID(), TRD_Ptr());
            new_trd_item.second = std::make_shared<ThreadContainer>(
                [this](const NotifyInfo& info) { this->onNotify(info); });
            new_trd_item.second->setTask(task);
            new_trd_item.first = new_trd_item.second->id();
            pool_working_.insert(new_trd_item);
            break;
        }

        // pool_idle is empty here

        while (pool_idle_.empty()) {
            xlog_dbg("idle empty, wait");
            cond_pool_not_all_busy_.wait(lock_pool);
        }

        xlog_dbg("idle not empty, wait end");
    }

    return;
}

void XThreadPool::waitTasks() {
    std::lock_guard<std::mutex> lock_call(mutex_call_);
    std::unique_lock<std::mutex> lock_pool(mutex_pool_);

    while (!pool_working_.empty()) {
        cond_pool_all_idle_.wait(lock_pool);
    }
}

void XThreadPool::onNotify(const NotifyInfo& info) {
    std::unique_lock<std::mutex> lock_pool(mutex_pool_);

    do {
        XLOG(DBG) << "notify: " << info.job_finished_thread_id;

        auto finished_iterator =
            pool_working_.find(info.job_finished_thread_id);
        if (finished_iterator != pool_working_.end()) {
            auto finished_item = *finished_iterator;
            pool_working_.erase(finished_iterator);
            if (pool_working_.empty()) {
                cond_pool_all_idle_.notify_one();
            }

            pool_idle_.insert(finished_item);
            cond_pool_not_all_busy_.notify_one();
        }
    } while (0);

    return;
}