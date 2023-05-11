
#include <gtest/gtest.h>

#include "bmp.hpp"

TEST(bmp_decoder, base)
{
    std::string path = RES_BMP_PATH;
    std::string file_bmp = path + "/" + "test.bmp";
    std::vector<uint8_t> buf;

    BmpDecoder bmp(file_bmp);
    EXPECT_TRUE(bmp.loadSuccessful());

    EXPECT_EQ(bmp.width(), 16);
    EXPECT_EQ(bmp.height(), 16);

    EXPECT_EQ(bmp.pixfmt(), BmpDecoder::PIXFMT_BGR24);

    buf = bmp.getContent(0, 1);

    EXPECT_EQ(buf.size(), 3);
    EXPECT_EQ(buf, std::vector<uint8_t>({0xff,0xff,0xff}));

    // buf = bmp.getContent();
}
