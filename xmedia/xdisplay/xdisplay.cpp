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
    st = init();
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

    return st;
}