
#include <gtest/gtest.h>

#include "bmp.hpp"

#include "xlog.hpp"

struct TestInfo
{
    std::string filename;
    int width{0};
    int height{0};
    BmpDecoder::PixFmt pixfmt{BmpDecoder::PIXFMT_NONE};
    std::vector<std::pair<std::pair<int,int>,std::vector<uint8_t>>> checks;
};

static void TestBmp(const TestInfo &info)
{
    const std::string path = RES_BMP_PATH;
    const std::string file_bmp = path + "/" + info.filename;

    BmpDecoder bmp(file_bmp);
    EXPECT_TRUE(bmp.loadSuccessful());

    EXPECT_EQ(bmp.width(), info.width);
    EXPECT_EQ(bmp.height(), info.height);

    EXPECT_EQ(bmp.pixfmt(), info.pixfmt);

    for (auto const& i : info.checks)
    {
        auto buf = bmp.getContent(i.first.first, i.first.second);
        EXPECT_EQ(buf, i.second);
    }
}

TEST(bmp_decoder, read_test)
{
    TestInfo info[] = {
        {
            .filename = "bmp_4_4_pureblack.bmp",
            .width = 4,
            .height = 4,
            .pixfmt = BmpDecoder::PIXFMT_BGR24,
            .checks = {
                std::make_pair(std::make_pair(0,1),
                std::vector<uint8_t>({0x0,0x0,0x0})),
                std::make_pair(std::make_pair(1,1),
                std::vector<uint8_t>({0x0,0x0,0x0})),
                std::make_pair(std::make_pair(0,-1),
                std::vector<uint8_t>(4*4*3,0x0))
            }
        },
        {
            .filename = "bmp_5_5_pureblack.bmp",
            .width = 5,
            .height = 5,
            .pixfmt = BmpDecoder::PIXFMT_BGR24,
            .checks = {
                std::make_pair(std::make_pair(0,1),
                std::vector<uint8_t>({0x0,0x0,0x0})),
                std::make_pair(std::make_pair(1,1),
                std::vector<uint8_t>({0x0,0x0,0x0})),
                std::make_pair(std::make_pair(0,-1),
                std::vector<uint8_t>(5*5*3,0x0))
            }
        },
        {
            .filename = "bmp_6_6_pureblack.bmp",
            .width = 6,
            .height = 6,
            .pixfmt = BmpDecoder::PIXFMT_BGR24,
            .checks = {
                std::make_pair(std::make_pair(0,1),
                std::vector<uint8_t>({0x0,0x0,0x0})),
                std::make_pair(std::make_pair(1,1),
                std::vector<uint8_t>({0x0,0x0,0x0})),
                std::make_pair(std::make_pair(0,-1),
                std::vector<uint8_t>(6*6*3,0x0))
            }
        },
        {
            .filename = "bmp_253_153_pureblack.bmp",
            .width = 253,
            .height = 153,
            .pixfmt = BmpDecoder::PIXFMT_BGR24,
            .checks = {
                std::make_pair(std::make_pair(0,1),
                std::vector<uint8_t>({0x0,0x0,0x0})),
                std::make_pair(std::make_pair(1,1),
                std::vector<uint8_t>({0x0,0x0,0x0})),
                std::make_pair(std::make_pair(0,-1),
                std::vector<uint8_t>(253*153*3,0x0))
            }
        },
        {
            .filename = "bmp_2_2_bwwb.bmp",
            .width = 2,
            .height = 2,
            .pixfmt = BmpDecoder::PIXFMT_BGR24,
            .checks = {
                std::make_pair(std::make_pair(0,1),
                std::vector<uint8_t>({0xff,0xff,0xff})),
                std::make_pair(std::make_pair(1,1),
                std::vector<uint8_t>({0x0,0x0,0x0})),
                std::make_pair(std::make_pair(2,1),
                std::vector<uint8_t>({0x0,0x0,0x0})),
                std::make_pair(std::make_pair(3,1),
                std::vector<uint8_t>({0xff,0xff,0xff})),
                std::make_pair(std::make_pair(0,-1),
                std::vector<uint8_t>({0xff,0xff,0xff,0x0,0x0,0x0,
                                        0x0,0x0,0x0,0xff,0xff,0xff}))
            }
        },
        {
            .filename = "bmp_7_3.bmp",
            .width = 7,
            .height = 3,
            .pixfmt = BmpDecoder::PIXFMT_BGR24,
            .checks = {
                std::make_pair(std::make_pair(0,1),
                std::vector<uint8_t>({0xff,0xff,0xff})),
                std::make_pair(std::make_pair(6,1),
                std::vector<uint8_t>({0x0,0x0,0x0})),
                std::make_pair(std::make_pair(14,1),
                std::vector<uint8_t>({0x0,0x0,0x0})),
                std::make_pair(std::make_pair(15,1),
                std::vector<uint8_t>({0x0,0x0,0xff})),
                std::make_pair(std::make_pair(16,1),
                std::vector<uint8_t>({0x0,0xff,0x0})),
                std::make_pair(std::make_pair(17,1),
                std::vector<uint8_t>({0xff,0x0,0x0})),
            }
        }
    };

    for (auto const& i : info)
    {
        TestBmp(i);
    }
}
