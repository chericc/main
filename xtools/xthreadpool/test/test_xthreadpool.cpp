#include <gtest/gtest.h>

#include <algorithm>
#include <vector>

#include "xlog.h"
#include "xthreadpool.hpp"

TEST(xthreadpool, singletask) {
    XThreadPoolConfig config;
    std::vector<std::size_t> result;

    {
        XThreadPool pool(config);
        pool.addTask([&]() { result.push_back(1u); });
        pool.waitTasks();
    }

    EXPECT_EQ(result.size(), 1u);
    if (!result.empty()) {
        EXPECT_EQ(result[0], 1u);
    }
}

TEST(xthreadpool, multitask) {
    XThreadPoolConfig config;

    constexpr std::size_t size = 1000;
    std::vector<std::size_t> result(size);

    {
        XThreadPool pool(config);
        for (std::size_t i = 0; i < size; ++i) {
            pool.addTask([i, &result]() {
                std::this_thread::sleep_for(std::chrono::microseconds(1));
                result[i] = i;
            });
        }
        pool.waitTasks();
    }

    EXPECT_EQ(result.size(), size);
    if (result.size() >= size) {
        std::sort(result.begin(), result.end(), std::less<std::size_t>());

        for (std::size_t i = 0; i < size; ++i) {
            EXPECT_EQ(result[i], i);
        }
    }
}

TEST(xthreadpool, multiworker) {
    XThreadPoolConfig config;
    config.core_pool_size = 1;
    config.maximum_pool_size = 10;

    constexpr std::size_t size = 1000;
    std::vector<std::size_t> result(size);

    {
        XThreadPool pool(config);
        for (std::size_t i = 0; i < size; ++i) {
            pool.addTask([i, &result]() {
                std::this_thread::sleep_for(std::chrono::microseconds(1));
                result[i] = i;
            });
        }
        pool.waitTasks();
    }

    EXPECT_EQ(result.size(), size);
    if (result.size() >= size) {
        std::sort(result.begin(), result.end(), std::less<std::size_t>());

        for (std::size_t i = 0; i < size; ++i) {
            EXPECT_EQ(result[i], i);
        }
    }
}