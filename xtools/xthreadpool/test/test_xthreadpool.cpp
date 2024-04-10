#include <gtest/gtest.h>

#include <vector>
#include <algorithm>

#include "xthreadpool.hpp"
#include "xlog.hpp"

TEST(xthreadpool, singletask)
{
    XThreadPoolConfig config;
    std::vector<std::size_t> result;

    {
        XThreadPool pool(config);
        pool.addTask([&](){
            result.push_back(1u);
        });
        pool.waitTasks();
    }

    EXPECT_EQ(result.size(), 1u);
    if (!result.empty())
    {
        EXPECT_EQ(result[0], 1u);
    }
}

TEST(xthreadpool, multitask)
{
    XThreadPoolConfig config;
    std::vector<std::size_t> result;
    constexpr std::size_t size = 10000;
    std::mutex mutex_pushback;

    {
        XThreadPool pool(config);
        for (std::size_t i = 0; i < size; ++i)
        {
            pool.addTask([i, &mutex_pushback, &result](){
                std::lock_guard<std::mutex> lock(mutex_pushback);
                result.push_back(i);
            });
        }
        pool.waitTasks();
    }

    EXPECT_EQ(result.size(), size);
    if (result.size() >= size)
    {
        std::sort(result.begin(), result.end(), std::less<std::size_t>());

        for (std::size_t i = 0; i < size; ++i)
        {
            EXPECT_EQ(result[i], i);
        }
    }
}