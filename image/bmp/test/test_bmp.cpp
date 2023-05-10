
#include <gtest/gtest.h>

#include "bmp.hpp"

TEST(bmp, base)
{
    std::string path = "/home/test/code/main/resources/bmp/test.bmp";

    BmpDecoder bmp(path);
    EXPECT_TRUE(bmp.loadSuccessful());
}