#pragma once

#include <memory>


#include "SDL2/SDL.h"
#include "xplay.hpp"

struct State {
    SDL_Window* window{nullptr};
    SDL_Renderer* renderer{nullptr};

    std::shared_ptr<XPlay> xplay;

    int screen_width{640};
    int screen_height{360};

    int xleft{0};
    int ytop{0};
    int width{0};
    int height{0};

    bool force_refresh{false};

    double frame_timer{0.0};

    SDL_Texture* video_texture{nullptr};

    RefreshState state{};

    ~State();
};

class XDisplay {
   public:
    XDisplay();
    ~XDisplay();

    int open(const std::string& url);
    int close();

    int exec();

   private:
    void videoRefresh(double* remaining_time);
    void videoDisplay();
    int videoOpen();
    void videoImageDisplay();
    void calDisplayRect(SDL_Rect* rect, int scr_xleft, int scr_ytop,
                        int scr_width, int scr_height, int pic_width,
                        int pic_height, AVRational pic_sar);
    void setSDLYUVConversionMode(const AVFrame* frame);
    int uploadTexture(SDL_Texture** tex, AVFrame* frame);
    void getSDLPixFmtAndBlendMode(int format, uint32_t* sdl_pix_fmt,
                                  SDL_BlendMode* sdl_blendmode);
    int reallocTexture(SDL_Texture** texture, uint32_t new_format,
                       int new_width, int new_height, SDL_BlendMode blendmode,
                       int init_texture);

    double vp_duration(Frame* vp, Frame* vp_last);
    double compute_target_delay(double delay);

    std::shared_ptr<State> init();

    std::shared_ptr<State> _st;

    const double AV_SYNC_THRESHOLD_MAX{0.1};
};