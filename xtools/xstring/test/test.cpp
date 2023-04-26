#include <gtest/gtest.h>

#include "xstring.hpp"
#include "xlog.hpp"

int main(int argc, char** argv)
{
	printf("Running main() from %s\n", __FILE__);

	std::vector<FILE*> fps = { stdout };
	xlog_setoutput(fps);

	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

TEST(xstring, base)
{
	std::string source = "aaa\nbbb\nc\nd";
	std::list<std::string> list = { "aaa","bbb","c","d" };

	EXPECT_EQ(xstring_split(source, "\n"), list);
}