
#include "freetypewrapper.hpp"
#include "imageview.hpp"
#include "xlog.hpp"
#include "bmp.hpp"

int main()
{
    {
        std::vector<FILE*> fps = {stdout};
        xlog_setoutput(fps);
    }

    std::string font_path = std::string() + RES_FONT_PATH + "/song.subset.TTF";

    int width = 1000;
    int height = 100;
    int depth = 3;

    FreeTypeWrapper ft(font_path);
    std::shared_ptr<std::vector<uint8_t>> mem;
    mem = std::make_shared<std::vector<uint8_t>>(width * height * depth);

    ImageView view(width, height, mem);
    std::shared_ptr<ImageView> iv = std::make_shared<ImageView>(width, height, mem);

    std::string utf8_str = "星期天12345";
    ft.drawString(utf8_str, 50, 0, 0, iv);

    {
        BmpDecoder::BmpInfo info{};
        info.data = view.mem();
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