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

    int open(const std::string& url);
    int close();

    int exec();

private:
    
    std::shared_ptr<State> init();

    std::shared_ptr<State> _st;
};