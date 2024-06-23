#include <gtest/gtest.h>

#include "xlog.hpp"
#include "xstring.hpp"

TEST(xstring, base) {
    std::string source = "aaa\nbbb\nc\nd";
    std::list<std::string> list = {"aaa", "bbb", "c", "d"};

    EXPECT_EQ(xstring_split(source, "\n"), list);
}