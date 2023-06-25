#include <gtest/gtest.h>

#include "xlog.hpp"

#define UNUSED_PARAMETER(x) do{(void)x;}while(0)

int main(int argc, char** argv)
{
	std::vector<FILE*> fps = { stdout };
	xlog_setoutput(fps);
	xlog_setmask(XLOG_MASK_ERR);

	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}