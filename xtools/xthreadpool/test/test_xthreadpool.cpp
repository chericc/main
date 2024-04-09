#include <gtest/gtest.h>

#include <vector>

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
    constexpr std::size_t size = 1000;

    {
        XThreadPool pool(config);
        for (std::size_t i = 0; i < size; ++i)
        {
            pool.addTask([&](){
                result.push_back(i);
            });
        }
        pool.waitTasks();
    }

    EXPECT_EQ(result.size(), size);
    if (result.size() >= size)
    {
        for (std::size_t i = 0; i < size; ++i)
        {
            EXPECT_EQ(result[i], i);
        }
    }
}