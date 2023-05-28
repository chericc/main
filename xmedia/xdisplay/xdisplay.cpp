#include "xdisplay.hpp"

#include "xlog.hpp"

State::~State()
{
    if (renderer)
    {
        xlog_trc("destroy renderer");
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }

    if (window)
    {
        xlog_trc("destroy window");
        SDL_DestroyWindow(window);
        window = nullptr;
    }
}

XDisplay::XDisplay()
{
    _st = init();
}

XDisplay::~XDisplay()
{

}

std::shared_ptr<State> XDisplay::init()
{
    int berror = false;
    int flags = 0;

    std::shared_ptr<State> st;

    do
    {
        flags = SDL_INIT_VIDEO | SDL_INIT_TIMER;

        st = std::make_shared<State>();

        if (SDL_Init(flags))
        {
            xlog_err("SDL_Init failed");
            berror = true;
            break;
        }

        flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
        st->window = SDL_CreateWindow("xdisplay", 
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            640, 360, flags);

        if (!st->window)
        {
            xlog_err("SDL_CreateWindow failed");
            berror = true;
            break;
        }

        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

        st->renderer = SDL_CreateRenderer(st->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (!st->renderer)
        {
            xlog_err("SDL_CreateRenderer failed");
            berror = true;
            break;
        }

        st->xplay = std::make_shared<XPlay>();

        SDL_PumpEvents();
    }
    while (0);

    return (berror ? nullptr : st);
}

int XDisplay::open(const std::string& url)
{
    int berror = false;

    do
    {
        if (!_st)
        {
            xlog_err("null");
            berror = true;
            break;
        }

        OptValues opt{};
        opt.filename = url;
        _st->xplay->open(opt);
    } 
    while (0);

    return (berror ? -1 : 0);
}

int XDisplay::close()
{
    xlog_inf("close");

    xlog_setoutput(std::vector<FILE*>());

    exit(0);

    return 0;
}

int XDisplay::exec()
{
    double remaininig_time = 0.0; // s
    SDL_Event event{};

    for (;;)
    {
        SDL_PumpEvents();
        while (!SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT))
        {
            if (remaininig_time > 0.0)
            {
                std::this_thread::sleep_for(
                    std::chrono::microseconds((uint64_t)(remaininig_time * 1000000.0)));
            }
            remaininig_time = 0.01;

            videoRefresh(remaininig_time);

            SDL_PumpEvents();
        }

        switch (event.type)
        {
        case SDL_QUIT:
        {
            close();
            break;
        }
        default:
        {
            break;
        }
        }
    }
}

void XDisplay::videoRefresh(double& remaining_time)
{
    VideoState& vs = _st->xplay->vs();

    if (vs.pictq->numRemaining() == 0)
    {
        xlog_trc("picq empty");
    }
    else
    {
        videoDisplay();
    }
}

void XDisplay::videoDisplay()
{
    if (!_st->width)
    {
        videoOpen();
    }

    SDL_SetRenderDrawColor(_st->renderer, 0, 0, 0, 255);
    SDL_RenderClear(_st->renderer);
    videoImageDisplay();
    SDL_RenderPresent(_st->renderer);
}

int XDisplay::videoOpen()
{
    int w = 0;
    int h = 0;

    int default_width = 640;
    int default_height = 360;

    w = _st->screen_width ? _st->screen_width : default_width;
    h = _st->screen_height ? _st->screen_height : default_height;

    xlog_trc("open: [w=%d,h=%d]", w, h);

    SDL_SetWindowTitle(_st->window, "hello");
    SDL_SetWindowSize(_st->window, w, h);
    SDL_SetWindowPosition(_st->window, 100, 100);
    SDL_ShowWindow(_st->window);

    _st->width = w;
    _st->height = h;

    return 0;
}

void XDisplay::videoImageDisplay()
{
    xlog_trc("display");

    Frame* vp = nullptr;
    Frame* sp = nullptr;
    SDL_Rect rect{};

    VideoState& vs = _st->xplay->vs();

    vp = vs.pictq->peek_last();

    calDisplayRect(&rect, _st->xleft, _st->ytop, _st->width, _st->height,
        vp->width, vp->height, vp->sar);

    setSDLYUVConversionMode(vp->frame);

    if (!vp->uploaded)
    {
        if (uploadTexture(&_st->video_texture, vp->frame) < 0)
        {
            setSDLYUVConversionMode(nullptr);
            return;
        }
        vp->uploaded = true;
        vp->flip_v = vp->frame->linesize[0] < 0;
    }

    SDL_RenderCopyEx(_st->renderer, _st->video_texture, nullptr, &rect,
        0, nullptr, vp->flip_v ? SDL_FLIP_VERTICAL : SDL_FLIP_NONE);

    setSDLYUVConversionMode(nullptr);
}


void XDisplay::calDisplayRect(SDL_Rect* rect,
    int scr_xleft, int scr_ytop, int scr_width, int scr_height,
    int pic_width, int pic_height, AVRational pic_sar)
{

}

void XDisplay::setSDLYUVConversionMode(AVFrame* frame)
{

}

int XDisplay::uploadTexture(SDL_Texture** tex, AVFrame* frame)
{
    return 0;
}

