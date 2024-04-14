#include <gtest/gtest.h>

#include "xbswap.hpp"

TEST(xutility, xbswap_n)
{
    uint16_t u16a = 0x1234;
    uint16_t u16b = 0x3412;
    EXPECT_EQ(xbswap(u16a), u16b);
    EXPECT_EQ(xbswap(u16b), u16a);

    uint32_t u32a = 0x12345678;
    uint32_t u32b = 0x78563412;

    EXPECT_EQ(xbswap(u32a), u32b);
    EXPECT_EQ(xbswap(u32b), u32a);

    uint64_t u64a = 0x1234567887654321;
    uint64_t u64b = 0x2143658778563412;

    EXPECT_EQ(xbswap(u64a), u64b);
    EXPECT_EQ(xbswap(u64b), u64a);
}