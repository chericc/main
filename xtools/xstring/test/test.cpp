#include <gtest/gtest.h>

#include "xstring.hpp"
#include "xlog.hpp"

TEST(xstring, base)
{
	std::string source = "aaa\nbbb\nc\nd";
	std::list<std::string> list = { "aaa","bbb","c","d" };

	EXPECT_EQ(xstring_split(source, "\n"), list);
}