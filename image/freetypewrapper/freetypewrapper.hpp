#pragma once

#include <string>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#include FT_STROKER_H

#include <functional>

#include "imageview.hpp"

class FreeTypeWrapper
{
public:
    FreeTypeWrapper(const std::string &font_path);
    ~FreeTypeWrapper();

    using FuncColorMap = std::function<std::shared_ptr<std::vector<uint8_t>>(uint8_t)>;

    /* 将 uint8_t 映射为一个具体的像素 */
    int setColorMap(FuncColorMap colorMap);

    int drawString(const std::string &utf8_str, int font_size, int x, int y,
        std::shared_ptr<ImageView> iv);
    int drawStringMonochrome(const std::string &utf8_str, int font_size, int x, int y,
        std::shared_ptr<ImageView> iv);
    int drawStringNormal(const std::string &utf8_str, int font_size, int x, int y,
        std::shared_ptr<ImageView> iv);
    
private:
    struct State
    {
        FT_Library ft_library{nullptr};
        FT_Face ft_face{nullptr};

        std::shared_ptr<FuncColorMap> color_map;

        ~State();
    };

    std::shared_ptr<State> init();
    int drawBitmap(std::shared_ptr<ImageView> iv, int x, int y, FT_Bitmap *bitmap);

    const std::string _font_path;

    std::shared_ptr<State> _state;
};