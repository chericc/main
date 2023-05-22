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
        std::shared_ptr<std::vector<uint8_t>> mem = 
            std::make_shared<std::vector<uint8_t>>(width * height * depth);

        imageview = std::make_shared<ImageView>(width, height, mem);

        ASSERT_TRUE(imageview->ok());
    }
};

TEST_F(ImageViewTest, base)
{
    EXPECT_EQ(imageview->mem().size(), width * height * depth);

    for (auto const & i : imageview->mem())
    {
        EXPECT_EQ(i, 0x0);
    }
}

TEST_F(ImageViewTest, drawrect)
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
            EXPECT_EQ(imageview->mem()[i], 0xff);
        }
        else 
        {
            EXPECT_EQ(imageview->mem()[i], 0x0);
        }
    }

    EXPECT_EQ(0, imageview->drawPixels(5, 3, pixels));
}