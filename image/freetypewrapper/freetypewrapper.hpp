#pragma once

#include <functional>
#include <vector>
#include <stdint.h>
#include <memory>
#include <string>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#include FT_STROKER_H
#include FT_BITMAP_H

#include "imageview.hpp"

class FreeTypeWrapper
{
public:

    enum class DrawMode
    {
        Normal,
        Monochrome,
        Outline,
    };

    using PixelColor = std::vector<uint8_t>;

    struct DrawInfo
    {
        std::shared_ptr<std::string> utf8_str;
        std::shared_ptr<ImageView> iv;
        int x{0};
        int y{0};
        int font_size{0};
        double outline_width{0.0};
        std::shared_ptr<PixelColor> foreground;
        std::shared_ptr<PixelColor> background;
        std::shared_ptr<PixelColor> outline;
        DrawMode mode{DrawMode::Normal};
    };

    FreeTypeWrapper(const std::string &font_path);
    ~FreeTypeWrapper();

    int drawString(const DrawInfo &info);

private:
    int drawString_Normal_Monochrome(const DrawInfo &info);
    int drawString_Outline(const DrawInfo &info);
    
private:
    struct State
    {
        FT_Library ft_library{nullptr};
        FT_Face ft_face{nullptr};

        ~State();
    };

    std::shared_ptr<State> init();
    int drawBitmap(std::shared_ptr<ImageView> iv, int x, int y, FT_Bitmap *bitmap, 
        std::shared_ptr<std::vector<uint8_t>> foreground,
        std::shared_ptr<std::vector<uint8_t>> background);

    /* color 0-->background 255-->foreground */
    std::shared_ptr<PixelColor> mid(std::shared_ptr<PixelColor> background, std::shared_ptr<PixelColor> foreground, uint8_t color);

    const std::string _font_path;

    std::shared_ptr<State> _state;
};