#include <list>
#include <string>

#include <gtest/gtest.h>

#include "xlog.hpp"

TEST(xlog, base)
{
    std::list<std::pair<std::string,std::string>> test_suite = 
    {
            std::make_pair("a.txt","a.txt"),
            std::make_pair("path/a.txt", "a.txt"),
            std::make_pair("", ""),
            std::make_pair("/", ""),
            std::make_pair("a\\b\\c", "c"),
            std::make_pair("a.txt","a.txt"),
            std::make_pair("path\\a.txt", "a.txt"),
            std::make_pair("", ""),
            std::make_pair("\\", ""),
            std::make_pair("a\\b\\c", "c")
    };

    for (auto const& i : test_suite)
    {
        ASSERT_EQ(xlog_shortfilepath(i.first), i.second);
    }
}