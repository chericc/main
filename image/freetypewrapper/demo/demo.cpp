
#include "freetypewrapper.hpp"
#include "imageview.hpp"
#include "xlog.hpp"
#include "bmp.hpp"

static const int s_pixel_depth = 3;

std::shared_ptr<std::vector<uint8_t>> color_mapper(uint8_t color)
{
    std::shared_ptr<std::vector<uint8_t>> pixel = std::make_shared<std::vector<uint8_t>>(s_pixel_depth);

    for (auto & r : *pixel)
    {
        r = 255 - color;
    }

    return pixel;
}

int main()
{
    {
        std::vector<FILE*> fps = {stdout};
        xlog_setoutput(fps);
    }

    // std::string font_path = std::string() + RES_FONT_PATH + "/song.subset.TTF";
    std::string font_path = std::string() + RES_FONT_PATH + "/fangzheng.subset.TTF";

    int width = 500;
    int height = 100;
    std::vector<uint8_t> pixel_init(s_pixel_depth, 0xff);

    std::shared_ptr<ImageView> iv = std::make_shared<ImageView>(width, height, s_pixel_depth, pixel_init);

    std::string utf8_str = "2023-05-22 星期一 14:14:43";
    FreeTypeWrapper ft(font_path);
    ft.setColorMap(color_mapper);

    // 72 * 360 / 1440 = 18
    // ft.drawString(utf8_str, 72 * 360 / 1440, 0, 0, iv);
    // ft.drawStringMonochrome(utf8_str, 72 * 360 / 1440, 0, 0, iv);
    ft.drawStringNormal(utf8_str, 72 * 360 / 1440, 0, 50, iv);

    {
        BmpDecoder::BmpInfo info{};
        info.data = std::make_shared<std::vector<uint8_t>>(iv->mem());
        info.file = "output.bmp";
        info.height = height;
        info.width = width;
        info.invert_y = 0;
        info.pixfmt = BmpDecoder::PIXFMT_BGR24;
        BmpDecoder::saveBmp(info);
    }

    xlog_dbg("end");

    return 0;
}