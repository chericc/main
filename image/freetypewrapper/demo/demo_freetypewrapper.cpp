
#include "freetypewrapper.hpp"
#include "imageview.hpp"
#include "xlog.hpp"
#include "bmp.hpp"

static const int s_pixel_depth = 3;

int main()
{
    {
        std::vector<FILE*> fps = {stdout};
        xlog_setoutput(fps);
    }

    std::string font_path = std::string() + RES_FONT_PATH + "/song.subset.TTF";
    // std::string font_path = std::string() + RES_FONT_PATH + "/fangzheng.subset.TTF";
    // std::string font_path = std::string() + RES_FONT_PATH + "/simsun.ttc";
    // std::string font_path = std::string() + RES_FONT_PATH + "/STHUPO.TTF";

    int width = 1440;
    int height = 360;
    std::vector<uint8_t> pixel_init(s_pixel_depth, 0xff);

    std::shared_ptr<ImageView> iv;

    {
        iv = std::make_shared<ImageView>(width, height, s_pixel_depth, pixel_init);
    }

    {
        FreeTypeWrapper ft(font_path);
        
        // std::string utf8_str = "2023-05-22 星期一 14:14:43";
        std::string utf8_str = "0123456789星期一123:-";
        // std::string utf8_str = "China 中国，全球 01";
        // std::string utf8_str = "A";
        // std::string utf8_str = "星";

        // 72 * 360 / 1440 = 18
        // ft.drawString(utf8_str, 72 * 360 / 1440, 0, 0, iv);
        // ft.drawStringMonochrome(utf8_str, 72 * 360 / 1440, 0, 0, iv);

        std::vector<uint8_t> pixel_white({0xff,0xff,0xff});
        std::vector<uint8_t> pixel_black({0x0,0x0,0x0});

        FreeTypeWrapper::DrawInfo drawinfo{};
        drawinfo.utf8_str = std::make_shared<std::string>(utf8_str);
        drawinfo.iv = iv;
        drawinfo.x = 0;
        drawinfo.y = 0;
        drawinfo.font_size = 72 * 360 / 1440;
        // drawinfo.font_size = 72;
        drawinfo.outline_width = 1.0;
        drawinfo.foreground = std::make_shared<std::vector<uint8_t>>(pixel_white);
        drawinfo.background = std::make_shared<std::vector<uint8_t>>(pixel_white);
        drawinfo.outline = std::make_shared<std::vector<uint8_t>>(pixel_black);
        drawinfo.mode = FreeTypeWrapper::DrawMode::Outline;

        ft.drawString(drawinfo);
    }

    // convert to 1555
    if (true)
    {
        for (int y = 0; y < iv->height(); ++y)
        {
            for (int x = 0; x < iv->width(); ++x)
            {
                auto pixel = iv->pixels(x, y, 1);
                for (auto &ref : pixel)
                {
                    ref = (ref & 0xf8);
                }
                iv->drawPixels(x, y, pixel);
            }
        }
    }

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