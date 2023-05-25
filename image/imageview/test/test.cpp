#include <gtest/gtest.h>

#include "imageview.hpp"

class ImageViewTest : public testing::Test
{
protected:
    int width = 7;
    int height = 4;
    int depth = 3;

    std::shared_ptr<ImageView> imageview;

    void SetUp() override
    {
        imageview = std::make_shared<ImageView>(width, height, depth);

        ASSERT_TRUE(imageview->ok());
    }
};

TEST_F(ImageViewTest, base)
{
    EXPECT_EQ(imageview->mem().size(), (std::size_t)(width * height * depth));

    EXPECT_EQ(imageview->width(), width);
    EXPECT_EQ(imageview->height(), height);
    EXPECT_EQ(imageview->depth(), depth);

    for (auto const & i : imageview->mem())
    {
        EXPECT_EQ(i, 0x0);
    }
}

TEST_F(ImageViewTest, pixels)
{
    {
        auto data = imageview->pixels(0, 0, 0);
        EXPECT_TRUE(data.empty());
    }

    {
        auto data = imageview->pixels(0, 0, 1);
        EXPECT_EQ(data, std::vector<uint8_t>({0x0,0x0,0x0}));
    }

    {
        auto data = imageview->pixels(0, 0, 3);
        std::vector<uint8_t> ref;
        ref.resize(3 * depth);
        EXPECT_EQ(data, ref);
    }

    {
        auto data = imageview->pixels(width - 2, height - 1, 1);
        EXPECT_EQ(data, std::vector<uint8_t>({0x0,0x0,0x0}));
    }
}

TEST_F(ImageViewTest, drawrect1)
{
    std::vector<uint8_t> pixels(depth * 1, 0xff);

    int x = 0;
    int y = 0;
    EXPECT_EQ(0, imageview->drawPixels(x, y, pixels));

    int pos = (y * width + x) * depth;
    for (int i = 0; i < (int)imageview->mem().size(); ++i)
    {
        if (i >= pos && i < pos + depth)
        {
            EXPECT_EQ(imageview->mem().at(i), 0xff);
        }
        else 
        {
            EXPECT_EQ(imageview->mem().at(i), 0x0);
        }
    }

    {
        auto data = imageview->pixels(x, y, 1);
        EXPECT_EQ(data, pixels);
    }

    {
        auto data = imageview->pixels(x + 1, y, 1);
        EXPECT_EQ(data, std::vector<uint8_t>({0x0,0x0,0x0}));
    }
}