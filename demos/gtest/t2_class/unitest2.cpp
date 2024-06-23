#include <gtest/gtest-spi.h>
#include <gtest/gtest.h>

#include "demo2.hpp"

TEST(Main, DefaultConstructor) {
    Calculator cal;
    EXPECT_EQ(0, cal.add());
    EXPECT_EQ(0, cal.minus());
    EXPECT_EQ(0, cal.multiply());
}

TEST(Main, MemberFunctions) {
    Calculator cal;
    cal.setValue(1, 1);

    EXPECT_EQ(2, cal.add());
    EXPECT_EQ(0, cal.minus());
    EXPECT_EQ(1, cal.multiply());
    EXPECT_EQ(1, cal.divide());
}