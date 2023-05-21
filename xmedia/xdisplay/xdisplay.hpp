#pragma once

#include <memory>
#include <SDL.h>

#include "xplay.hpp"

struct State
{
    SDL_Window *window{nullptr};
    SDL_Renderer *renderer{nullptr};

    std::shared_ptr<XPlay> xplay;

    ~State();
};

class XDisplay
{
public:
    XDisplay();
    ~XDisplay();

private:
    
    std::shared_ptr<State> init();

    std::shared_ptr<State> st;
};