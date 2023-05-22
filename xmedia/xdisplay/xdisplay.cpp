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