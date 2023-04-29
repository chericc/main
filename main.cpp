#include <gtest/gtest.h>

#include "xlog.hpp"

int main(int argc, char** argv)
{
	printf("Running main() from %s\n", __FILE__);

	std::vector<FILE*> fps = { stdout };
	xlog_setoutput(fps);

	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}