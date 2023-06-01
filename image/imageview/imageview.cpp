#include "imageview.hpp"

#include "xlog.hpp"

ImageView::ImageView(int w, int h, int depth, std::vector<uint8_t> init_pixel)
{
    if (init_pixel.empty())
    {
        init_pixel.resize(depth, 0x0);
    }

    _state = init(w, h, depth, init_pixel);
}

ImageView::~ImageView()
{

}

int ImageView::ok() const
{
    return (_state ? true : false);
}

int ImageView::width() const
{
    if (_state)
    {
        return _state->w;
    }

    xlog_err("null");
    return 0;
}

int ImageView::height() const
{
    if (_state)
    {
        return _state->h;
    }

    xlog_err("null");
    return 0;
}

int ImageView::depth() const
{
    if (_state)
    {
        return _state->depth;
    }

    xlog_err("null");
    return 0;
}

const std::vector<uint8_t> &ImageView::mem() const
{
    if (_state && _state->mem)
    {}
    else 
    {
        xlog_cri("null");
    }
    
    return *_state->mem;
}

std::vector<uint8_t> ImageView::pixels(int x, int y, int num_pixels) const
{
    int berror = false;

    std::vector<uint8_t> data;

    do 
    {
        if (!_state)
        {
            xlog_err("null");
            berror = true;
            break;
        }

        if (x < 0 || y < 0 || num_pixels <= 0)
        {
            xlog_err("invalid arg");
            berror = true;
            break;
        }

        int total_bytes = num_pixels * _state->depth;
        if (total_bytes < 0)
        {
            xlog_err("invalid arg");
            berror = true;
            break;
        }

        if (!pixelAt(x, y, num_pixels - 1))
        {
            xlog_err("over[%d,%d,%d]", x, y, num_pixels);
            berror = true;
            break;
        }

        data.reserve(total_bytes);

        const uint8_t *src = pixelAt(x, y, 0);
        for (int i = 0; i < total_bytes; ++i)
        {
            data.push_back(src[i]);
        }
    }
    while (0);

    return (berror ? std::vector<uint8_t>() : data);
}

int ImageView::drawPixels(int x, int y, const std::vector<uint8_t> &pixels)
{
    int berror = false;

    do 
    {
        if (!_state)
        {
            xlog_err("not ok");
            berror = true;
            break;
        }

        if (pixels.size() % _state->depth)
        {
            xlog_err("invalid args");
            berror = true;
            break;
        }

        int num_pixels = pixels.size() / _state->depth;

        if (num_pixels <= 0)
        {
            xlog_err("invalid args");
            berror = true;
            break;
        }

        if (!pixelAt(x, y, num_pixels - 1))
        {
            xlog_err("invalid args[over]");
            berror = true;
            break;
        }

        for (int i = 0; i < num_pixels; ++i)
        {
            const uint8_t *psrc = &pixels[i * _state->depth];
            uint8_t *pdst = pixelAt(x, y, i);

            for (int k = 0; k < _state->depth; ++k)
            {
                pdst[k] = psrc[k];
            }
        }
    }
    while (0);

    return (berror ? -1 : 0);
}

#if 0
int ImageView::scale(int w, int h)
{
    xlog_err("not implement");
    return -1;
}

int ImageView::drawRect(int x, int y, int w, int h, const std::vector<uint8_t> &pixels)
{
    xlog_err("not implement");
    return -1;
}
#endif 

std::shared_ptr<ImageView::State> ImageView::init(int w, int h, int depth, std::vector<uint8_t> init_pixel)
{
    std::shared_ptr<State> state;
    int berror = false;

    do
    {
        if (w <= 0 || h <= 0 || depth <= 0)
        {
            xlog_err("invalid args");
            berror = true;
            break;
        }

        int num_pixel = w * h;
        int total_bytes = num_pixel * depth;
        if (num_pixel <= 0 || total_bytes <= 0)
        {
            xlog_err("invalid args(w,h,depth over)");
            berror = true;
            break;
        }

        if (init_pixel.size() != (std::size_t)depth)
        {
            xlog_err("invalid args(init_pixel.size != depth)");
            berror = true;
            break;
        }

        std::shared_ptr<std::vector<uint8_t>> mem = 
            std::make_shared<std::vector<uint8_t>>();
        mem->reserve(total_bytes);

        for (int i = 0; i < num_pixel; ++i)
        {
            for (int k = 0; k < depth; ++k)
            {
                mem->push_back(init_pixel[k]);
            }
        }

        state = std::make_shared<State>();

        state->mem = mem;
        state->w = w;
        state->h = h;
        state->depth = depth;

        if (!state->depth)
        {
            xlog_err("pixel depth is 0");
            berror = true;
            break;
        }
    }
    while (0);

    return (berror ? nullptr : state);
}

uint8_t* ImageView::pixelAt(int x, int y, int offset_pixels)
{
    if (!_state)
    {
        return nullptr;
    }

    if (x < 0 || x >= _state->w
        || y < 0 || y >= _state->h)
    {
        return nullptr;
    }

    int pixel_offset = y * _state->w + x + offset_pixels;
    int bytes_offset = pixel_offset * _state->depth;

    if (bytes_offset < 0 || (std::size_t)bytes_offset >= _state->mem->size())
    {
        return nullptr;
    }

    return _state->mem->data() + bytes_offset;
}

const uint8_t* ImageView::pixelAt(int x, int y, int offset_pixels) const
{
    if (!_state)
    {
        return nullptr;
    }

    if (x < 0 || x >= _state->w
        || y < 0 || y >= _state->h)
    {
        return nullptr;
    }

    int pixel_offset = y * _state->w + x + offset_pixels;
    int bytes_offset = pixel_offset * _state->depth;

    if (bytes_offset < 0 || (std::size_t)bytes_offset >= _state->mem->size())
    {
        return nullptr;
    }

    return _state->mem->data() + bytes_offset;
}
