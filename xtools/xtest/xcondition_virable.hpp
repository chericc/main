#pragma once

#include <condition_variable>

class XCondtionVariable
{
public:
    virtual ~XCondtionVariable() = default;
    virtual void notify_one() = 0;
    virtual void wait(std::unique_lock<std::mutex> &lock) = 0;
    virtual std::cv_status wait_until(std::unique_lock<std::mutex> &mutex,
        std::chrono::steady_clock::time_point &timepoint) = 0;
    virtual std::cv_status wait_until(std::unique_lock<std::mutex> &mutex,
        std::chrono::system_clock::time_point &timepoint) = 0;
};

class XCondtionVariableReal : public XCondtionVariable
{
public:
    void notify_one() override
    {
        return cond.notify_one();
    }
    void wait(std::unique_lock<std::mutex> &lock) override
    {
        return cond.wait(lock);
    }
    std::cv_status wait_until(std::unique_lock<std::mutex> &lock,
        std::chrono::steady_clock::time_point &timepoint) override
    {
        return cond.wait_until(lock, timepoint);
    }
    std::cv_status wait_until(std::unique_lock<std::mutex> &lock,
        std::chrono::system_clock::time_point &timepoint) override
    {
        return cond.wait_until(lock, timepoint);
    }
private:
    std::condition_variable cond;
};
