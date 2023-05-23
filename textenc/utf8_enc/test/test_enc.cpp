
#include <gtest/gtest.h>
#include <stdio.h>

#include "utf8_enc.hpp"
#include "content_utf8.hpp"

TEST(utf8, enc_utf8_strlen)
{
    EXPECT_EQ(20, enc_utf8_length(g_utf8_20_words));
}

TEST(utf8, enc_utf8_substr)
{
    EXPECT_EQ(g_utf8_20_words_0_1, enc_utf8_substr(g_utf8_20_words, 0, 1));
    EXPECT_EQ(g_utf8_20_words_0_0, enc_utf8_substr(g_utf8_20_words, 0, 0));
    EXPECT_EQ(g_utf8_20_words_5_6, enc_utf8_substr(g_utf8_20_words, 5, 6));
    EXPECT_EQ(g_utf8_20_words_19_1, enc_utf8_substr(g_utf8_20_words, 19, 1));
}

TEST(utf8, enc_utf8_2_utf32)
{
    EXPECT_EQ(g_utf32_20_words, enc_utf8_2_utf32(g_utf8_20_words));
}

TEST(utf8, enc_utf32_2_utf8)
{
    EXPECT_EQ(g_utf8_20_words, enc_utf32_2_utf8(g_utf32_20_words));
}

