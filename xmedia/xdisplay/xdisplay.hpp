#pragma once

#include <memory>
#include <SDL.h>

#include "xplay.hpp"

struct State
{
    SDL_Window *window{nullptr};
    SDL_Renderer *renderer{nullptr};

    std::shared_ptr<XPlay> xplay;

    int screen_width{ 640 };
    int screen_height{ 360 };

    int xleft{ 0 };
    int ytop{ 0 };
    int width{ 0 };
    int height{ 0 };

    SDL_Texture* video_texture{ nullptr };

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

    void videoRefresh(double &remaining_time);
    void videoDisplay();
    int videoOpen();
    void videoImageDisplay();
    void calDisplayRect(SDL_Rect* rect,
        int scr_xleft, int scr_ytop, int scr_width, int scr_height,
        int pic_width, int pic_height, AVRational pic_sar);
    void setSDLYUVConversionMode(AVFrame* frame);
    int uploadTexture(SDL_Texture** tex, AVFrame* frame);
    
    std::shared_ptr<State> init();

    std::shared_ptr<State> _st;
};