
#include <gtest/gtest.h>

#include <list>

#include "bmp.hpp"
#include "test_comm.hpp"
#include "xlog.h"

struct TestInfo {
    std::string filename;
    int width{0};
    int height{0};
    BmpDecoder::PixFmt pixfmt{BmpDecoder::PIXFMT_NONE};
    std::vector<std::pair<std::pair<int, int>, std::vector<uint8_t>>> checks;
};

static void TestBmp(const TestInfo& info) {
    const std::string path = RES_BMP_PATH;
    const std::string file_bmp = path + "/" + info.filename;

    BmpDecoder bmp(file_bmp);
    EXPECT_TRUE(bmp.loadSuccessful());

    EXPECT_EQ(bmp.width(), info.width);
    EXPECT_EQ(bmp.height(), info.height);

    EXPECT_EQ(bmp.pixfmt(), info.pixfmt);

    for (auto const& i : info.checks) {
        auto buf = bmp.getContent(i.first.first, i.first.second);
        EXPECT_NE(buf, nullptr);
        if (buf) {
            EXPECT_EQ(*buf, i.second);
        }
    }

    {
        std::string file_bmp_save = std::string("save_") + info.filename;
        BmpDecoder::BmpInfo info{};
        auto buf = bmp.getContent(0, bmp.width() * bmp.height());
        EXPECT_NE(buf, nullptr);
        if (buf) {
            info.data = buf;
            info.file = file_bmp_save;
            info.width = bmp.width();
            info.height = bmp.height();
            info.pixfmt = bmp.pixfmt();
            info.invert_y = true;
            BmpDecoder::saveBmp(info);

            EXPECT_EQ(readFile(file_bmp), readFile(file_bmp_save));
        }
    }
}

TEST(bmp_decoder, read_test) {
    std::list<TestInfo> info;
#if 0
    {
        TestInfo item{};
        item.filename = "bmp_4_4_pureblack.bmp";
        item.width = 4;
        item.height = 4;
        item.pixfmt = BmpDecoder::PIXFMT_BGR24;
        item.checks = {
            std::make_pair(std::make_pair(0,1),
            std::vector<uint8_t>({0x0,0x0,0x0})),
            std::make_pair(std::make_pair(1,1),
            std::vector<uint8_t>({0x0,0x0,0x0})),
            std::make_pair(std::make_pair(0,-1),
            std::vector<uint8_t>(4*4*3,0x0))
        };
        info.push_back(item);
    }

    {
        TestInfo item{};
        item.filename = "bmp_5_5_pureblack.bmp";
        item.width = 5;
        item.height = 5;
        item.pixfmt = BmpDecoder::PIXFMT_BGR24;
        item.checks = {
            std::make_pair(std::make_pair(0,1),
            std::vector<uint8_t>({0x0,0x0,0x0})),
            std::make_pair(std::make_pair(1,1),
            std::vector<uint8_t>({0x0,0x0,0x0})),
            std::make_pair(std::make_pair(0,-1),
            std::vector<uint8_t>(5*5*3,0x0))
        };
        info.push_back(item);
    }

    {
        TestInfo item{};
        item.filename = "bmp_6_6_pureblack.bmp";
        item.width = 6;
        item.height = 6;
        item.pixfmt = BmpDecoder::PIXFMT_BGR24;
        item.checks = {
            std::make_pair(std::make_pair(0,1),
            std::vector<uint8_t>({0x0,0x0,0x0})),
            std::make_pair(std::make_pair(1,1),
            std::vector<uint8_t>({0x0,0x0,0x0})),
            std::make_pair(std::make_pair(0,-1),
            std::vector<uint8_t>(6*6*3,0x0))
        };
        info.push_back(item);
    }
#endif
    {
        TestInfo item{};
        item.filename = "bmp_253_153_pureblack.bmp";
        item.width = 253;
        item.height = 153;
        item.pixfmt = BmpDecoder::PIXFMT_BGR24;
        item.checks = {
            std::make_pair(std::make_pair(0, 1),
                           std::vector<uint8_t>({0x0, 0x0, 0x0})),
            std::make_pair(std::make_pair(1, 1),
                           std::vector<uint8_t>({0x0, 0x0, 0x0})),
            std::make_pair(std::make_pair(0, -1),
                           std::vector<uint8_t>(253 * 153 * 3, 0x0))};
        info.push_back(item);
    }
    {
        TestInfo item{};
        item.filename = "bmp_2_2_bwwb.bmp";
        item.width = 2;
        item.height = 2;
        item.pixfmt = BmpDecoder::PIXFMT_BGR24;
        item.checks = {std::make_pair(std::make_pair(0, 1),
                                      std::vector<uint8_t>({0xff, 0xff, 0xff})),
                       std::make_pair(std::make_pair(1, 1),
                                      std::vector<uint8_t>({0x0, 0x0, 0x0})),
                       std::make_pair(std::make_pair(2, 1),
                                      std::vector<uint8_t>({0x0, 0x0, 0x0})),
                       std::make_pair(std::make_pair(3, 1),
                                      std::vector<uint8_t>({0xff, 0xff, 0xff})),
                       std::make_pair(std::make_pair(0, -1),
                                      std::vector<uint8_t>(
                                          {0xff, 0xff, 0xff, 0x0, 0x0, 0x0, 0x0,
                                           0x0, 0x0, 0xff, 0xff, 0xff}))};
        info.push_back(item);
    }

    {
        TestInfo item{};
        item.filename = "bmp_7_3.bmp";
        item.width = 7;
        item.height = 3;
        item.pixfmt = BmpDecoder::PIXFMT_BGR24;
        item.checks = {
            std::make_pair(std::make_pair(0, 1),
                           std::vector<uint8_t>({0xff, 0xff, 0xff})),
            std::make_pair(std::make_pair(6, 1),
                           std::vector<uint8_t>({0x0, 0x0, 0x0})),
            std::make_pair(std::make_pair(14, 1),  // black
                           std::vector<uint8_t>({0x0, 0x0, 0x0})),
            std::make_pair(std::make_pair(15, 1),
                           std::vector<uint8_t>({0x0, 0x0, 0xff})),  // red
            std::make_pair(std::make_pair(16, 1),
                           std::vector<uint8_t>({0x0, 0xff, 0x0})),  // green
            std::make_pair(std::make_pair(17, 1),
                           std::vector<uint8_t>({0xff, 0x0, 0x0})),  // blue
        };
        info.push_back(item);
    }

    for (auto const& i : info) {
        TestBmp(i);
    }
}
