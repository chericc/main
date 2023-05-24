#include "freetypewrapper.hpp"

#include "xlog.hpp"
#include "utf8_enc.hpp"

#define INT_TO_FP_26_6(integer) ((integer) * 64)
#define FP_26_6_TO_INT(FP26_6)  ((FP26_6) / 64)

FreeTypeWrapper::State::~State()
{
    if (ft_face)
    {
        FT_Done_Face(ft_face);
        ft_face = nullptr;
    }

    if (ft_library)
    {
        FT_Done_FreeType(ft_library);
        ft_library = nullptr;
    }
}

FreeTypeWrapper::FreeTypeWrapper(const std::string &font_path)
    :_font_path(font_path)
{
    _state = init();
}

FreeTypeWrapper::~FreeTypeWrapper()
{
    _state.reset();
}

int FreeTypeWrapper::setColorMap(FuncColorMap color_map)
{
    int berror = false;

    do 
    {
        if (!_state)
        {
            xlog_err("null");
            berror = true;
            break;
        }

        _state->color_map = std::make_shared<FuncColorMap>(color_map);
    }
    while(0);
}

int FreeTypeWrapper::drawString(const std::string &utf8_str, int font_size, int x, int y,
    std::shared_ptr<ImageView> iv)
{
    int berror = false;
    FT_Error err{};

    do
    {
        if (x < 0 || y < 0 || !iv)
        {
            xlog_err("invalid args");
            berror = true;
            break;
        }

        if (!_state)
        {
            xlog_err("null");
            berror = true;
            break;
        }

        std::vector<uint32_t> utf32_str;

        utf32_str = enc_utf8_2_utf32(utf8_str);

        err = FT_Set_Pixel_Sizes(_state->ft_face, 0, font_size);
        if (err)
        {
            xlog_err("FT_Set_Char_Size failed");
            berror = true;
            break;
        }

        FT_GlyphSlot slot = nullptr;
        FT_Vector pen = {0, 0};
        slot = _state->ft_face->glyph;

        for (int i = 0; i < (int)utf32_str.size(); ++i)
        {
            FT_Set_Transform(_state->ft_face, 0, &pen);

            err = FT_Load_Char(_state->ft_face, utf32_str[i], FT_LOAD_RENDER);
            if (err)
            {
                xlog_err("FT_Load_Char failed");
                berror = true;
                break;
            }

            FT_Pos ascender = _state->ft_face->size->metrics.ascender >> 6;
            drawBitmap(iv, 
                x + slot->bitmap_left, 
                y + ascender - slot->bitmap_top,
                &slot->bitmap);

            pen.x += slot->advance.x;
            pen.y += slot->advance.y;
        }
    }
    while (0);

    return (berror ? -1 : 0);
}

int FreeTypeWrapper::drawStringMonochrome(const std::string &utf8_str, int font_size, int x, int y,
    std::shared_ptr<ImageView> iv)
{

    int berror = false;
    FT_Error err{};

    do
    {
        if (x < 0 || y < 0 || !iv)
        {
            xlog_err("invalid args");
            berror = true;
            break;
        }

        if (!_state)
        {
            xlog_err("null");
            berror = true;
            break;
        }

        std::vector<uint32_t> utf32_str;

        utf32_str = enc_utf8_2_utf32(utf8_str);

        err = FT_Set_Pixel_Sizes(_state->ft_face, 0, font_size);
        if (err)
        {
            xlog_err("FT_Set_Char_Size failed");
            berror = true;
            break;
        }

        FT_GlyphSlot slot = nullptr;
        FT_Vector pen = {0, 0};
        slot = _state->ft_face->glyph;

        for (int i = 0; i < (int)utf32_str.size(); ++i)
        {
            FT_Set_Transform(_state->ft_face, 0, &pen);

            err = FT_Load_Char(_state->ft_face, utf32_str[i], FT_LOAD_RENDER | FT_LOAD_MONOCHROME);
            if (err)
            {
                xlog_err("FT_Load_Char failed");
                berror = true;
                break;
            }

            FT_Pos ascender = _state->ft_face->size->metrics.ascender >> 6;
            drawBitmap(iv, 
                x + slot->bitmap_left, 
                y + ascender - slot->bitmap_top,
                &slot->bitmap);

            pen.x += slot->advance.x;
            pen.y += slot->advance.y;
        }
    }
    while (0);

    return (berror ? -1 : 0);
}

int FreeTypeWrapper::drawStringNormal(const std::string &utf8_str, int font_size, int x, int y,
    std::shared_ptr<ImageView> iv)
{
    int berror = false;
    FT_Error err{};

    do
    {
        if (x < 0 || y < 0 || !iv)
        {
            xlog_err("invalid args");
            berror = true;
            break;
        }

        if (!_state)
        {
            xlog_err("null");
            berror = true;
            break;
        }

        std::vector<uint32_t> utf32_str;

        utf32_str = enc_utf8_2_utf32(utf8_str);

        err = FT_Set_Pixel_Sizes(_state->ft_face, 0, font_size);
        if (err)
        {
            xlog_err("FT_Set_Char_Size failed");
            berror = true;
            break;
        }

        FT_GlyphSlot slot = nullptr;
        FT_Vector pen = {0, 0};
        slot = _state->ft_face->glyph;

        int iv_width = iv->width();
        int iv_height = iv->height();

        /* 注意：freetype 坐标系为笛卡尔坐标系 */
        
        pen.x = INT_TO_FP_26_6(x);
        pen.y = INT_TO_FP_26_6(iv_height - y);

        for (int i = 0; i < (int)utf32_str.size(); ++i)
        {
            FT_Set_Transform(_state->ft_face, nullptr, &pen);

            err = FT_Load_Char(_state->ft_face, utf32_str[i], FT_LOAD_RENDER);
            if (err)
            {
                xlog_err("FT_Load_Char failed");
                continue;
            }

            // FT_Pos ascender = _state->ft_face->size->metrics.ascender >> 6;

            auto &metrics = _state->ft_face->size->metrics;
            xlog_trc("[ascender=%d, descender=%d, height=%d]",
                FP_26_6_TO_INT(metrics.ascender), 
                FP_26_6_TO_INT(metrics.descender),
                FP_26_6_TO_INT(metrics.height));

            drawBitmap(iv, 
                slot->bitmap_left, 
                iv_height - 1 - slot->bitmap_top + (_state->ft_face->size->metrics.height >> 6),
                &slot->bitmap);

            pen.x += slot->advance.x;
            pen.y += slot->advance.y;
        }
    }
    while (0);

    return (berror ? -1 : 0);
}

std::shared_ptr<FreeTypeWrapper::State> FreeTypeWrapper::init()
{
    std::shared_ptr<State> state;
    int berror = false;

    do
    {
        FT_Error err{};
        
        state = std::make_shared<State>();

        err = FT_Init_FreeType(&state->ft_library);
        if (err)
        {
            xlog_err("FT_Init_FreeType failed");
            berror = true;
            break;
        }

        err = FT_New_Face(state->ft_library, _font_path.c_str(), 0, &state->ft_face);
        if (err)
        {
            xlog_err("FT_New_Face failed");
            berror = true;
            break;
        }
    }
    while (0);

    if (berror)
    {
        state.reset();
    }

    return state;
}

/* 注：这里以左上角为原点 */
int FreeTypeWrapper::drawBitmap(std::shared_ptr<ImageView> iv, int x, int y, FT_Bitmap *bitmap)
{
    xlog_trc("[x=%d,y=%d][w=%d, h=%d]", 
        x, y, 
        (int)bitmap->width, (int)bitmap->rows);

    int berror = false;

    if (!bitmap || !iv)
    {
        xlog_err("null");
        berror = true;
        return -1;
    }

    std::vector<uint8_t> pixel(iv->depth());

    for (int j = 0; j < bitmap->rows; ++j)
    {
        for (int i = 0; i < bitmap->width; ++i)
        {
            int dst_x = x + i;
            int dst_y = y + j;

            if (dst_x < 0 || dst_x >= iv->width())
            {
                xlog_err("x over");
                continue;
            }

            if (dst_y < 0 || dst_y >= iv->height())
            {
                xlog_err("y over"); 
                continue;
            }

            uint8_t color = 0x0;

            switch (bitmap->pixel_mode)
            {
                case FT_PIXEL_MODE_MONO:
                {
                    int byte_index = (bitmap->pitch * j + i / 8);
                    int bit_index = i % 8;
                    if (bitmap->buffer[byte_index] & (0x80 >> bit_index))
                    {
                        color = 0xff;
                    }
                    else 
                    {
                        color = 0x0;
                    }
                    break;
                }
                case FT_PIXEL_MODE_GRAY:
                {
                    color = bitmap->buffer[j * bitmap->width + i];
                    break;
                }
                default:
                {
                    xlog_err("");
                    break;
                }
            }

            if (!_state->color_map)
            {
                for (auto & r : pixel)
                {
                    r = color;
                }
            }
            else 
            {
                auto pixel_map = (*_state->color_map)(color);
                if (pixel_map)
                {
                    pixel = *pixel_map;
                }
                else 
                {
                    xlog_err("color map null");
                }
            }

            iv->drawPixels(dst_x, dst_y, pixel);
        }
    }
    
}