#pragma once

#include <string>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#include FT_STROKER_H

#include "imageview.hpp"

class FreeTypeWrapper
{
public:
    FreeTypeWrapper(const std::string &font_path);
    ~FreeTypeWrapper();

    void drawString(const std::string &utf8_str, int font_size, int x, int y,
        std::shared_ptr<ImageView> iv);
    
private:
    struct State
    {
        FT_Library ft_library{nullptr};
        FT_Face ft_face{nullptr};

        ~State();
    };

    std::shared_ptr<State> init();
    void drawBitmap(FT_Bitmap *bitmap, int x, int y, std::shared_ptr<ImageView> iv);

    const std::string _font_path;
    std::shared_ptr<State> _state;
};