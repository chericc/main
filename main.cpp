#include <gtest/gtest.h>

#include "xlog.hpp"

int main(int argc, char** argv)
{
	std::vector<FILE*> fps = { stdout };
	xlog_setoutput(fps);
	// xlog_setmask(0xffffffff & (!XLOG_LEVEL_TRACE) & (!XLOG_LEVEL_DEBUG));

	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}