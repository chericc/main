#include <gtest/gtest.h>

#include "demo1.hpp"

TEST(demo1, add) {
    EXPECT_EQ(2, add(1, 1));
    EXPECT_EQ(0, add(-1, 1));
    EXPECT_EQ(0, add(1, -1));
    EXPECT_EQ(-2, add(-1, -1));
}