/**
 * 定义一个在图片上进行操作的类。
 */

#pragma once

#include <memory>
#include <vector>
#include <stdint.h>

class ImageView
{
public:
    ImageView(int w, int h, std::shared_ptr<std::vector<uint8_t>> mem);
    ~ImageView();
    int ok();

    int width();
    int height();
    int pixelBytes();
    std::vector<uint8_t> &mem();

    int drawPixels(int x, int y, const std::vector<uint8_t> &pixels);
    int scale(int w, int h);
    int drawRect(int x, int y, int w, int h, const std::vector<uint8_t> &pixels);
private:

    struct State
    {
        std::shared_ptr<std::vector<uint8_t>> mem;
        int w{0};
        int h{0};
        int pixel_bytes{0};
    };

    std::shared_ptr<State> init(int w, int h, std::shared_ptr<std::vector<uint8_t>> mem);
    uint8_t* pixelAt(int x, int y, int offset_pixels);

    std::shared_ptr<State> _state;
};