#include <stdio.h>
#include <fstream>

#include "xconfig.hpp"
#include "xlog.hpp"

#include <gtest/gtest.h>

int main(int argc, char** argv)
{
	printf("Running main() from %s\n", __FILE__);

	std::vector<FILE*> fps = { stdout };
	xlog_setoutput(fps);

	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

TEST(xconfig, base)
{
	std::string file("config.ini");

	std::ofstream of;
	of.open(file, std::ios_base::out);
	ASSERT_EQ(of.good(), true);
	of.close();

	XConfig config(file, false);

	ASSERT_EQ(0, config.LoadFile());

	std::string section = "base";
	std::string key_width = "width";
	std::string value_width = "800";
	std::string key_height = "height";
	std::string value_height = "600";

	config.SetValue(section, key_width, value_width);
	config.SetValue(section, key_height, value_height);

	EXPECT_EQ(config.Exist(section, key_width), true);
	EXPECT_EQ(config.GetValue(section, key_width), value_width);

	EXPECT_EQ(config.Exist(section, key_height), true);
	EXPECT_EQ(config.GetValue(section, key_height), value_height);
}