#include "freetypewrapper.hpp"

#include "xlog.hpp"
#include "utf8_enc.hpp"

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

void FreeTypeWrapper::drawString(const std::string &utf8_str, int font_size, int x, int y,
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

        // err = FT_Set_Char_Size(_state->ft_face, 50 << 6, 0, 100, 0);
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
            drawBitmap(&slot->bitmap, 
                x + slot->bitmap_left, 
                y + ascender - slot->bitmap_top,
                iv);

            pen.x += slot->advance.x;
            pen.y += slot->advance.y;
        }
    }
    while (0);

    return ;
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

void FreeTypeWrapper::drawBitmap(FT_Bitmap *bitmap, int x, int y, std::shared_ptr<ImageView> iv)
{
    xlog_trc("[x=%d,y=%d][w=%d, h=%d]", 
        x, y, 
        (int)bitmap->width, (int)bitmap->rows);

    int view_width = iv->width();
    int view_height = iv->height();

    std::vector<uint8_t> pixel(iv->pixelBytes());

    for (int j = 0; j < bitmap->rows; ++j)
    {
        for (int i = 0; i < bitmap->width; ++i)
        {
            int dst_x = x + i;
            int dst_y = y + j;

            if (dst_x < 0 || dst_x >= view_width)
            {
                xlog_err("x over");
                continue;
            }

            if (dst_y < 0 || dst_y >= view_height)
            {
                xlog_err("y over"); 
                continue;
            }

            for (auto & r : pixel)
            {
                r = bitmap->buffer[j * bitmap->width + i];
            }
            iv->drawPixels(dst_x, dst_y, pixel);
        }
    }
    
}