#include "xdisplay.hpp"

#include "xlog.hpp"

#define ARRAY_ELEMS(array) (sizeof(array)/sizeof(array[0]))

static const struct TextureFormatEntry {
    enum AVPixelFormat format;
    int texture_fmt;
} sdl_texture_format_map[] = {
    { AV_PIX_FMT_RGB8,           SDL_PIXELFORMAT_RGB332 },
    { AV_PIX_FMT_RGB444,         SDL_PIXELFORMAT_RGB444 },
    { AV_PIX_FMT_RGB555,         SDL_PIXELFORMAT_RGB555 },
    { AV_PIX_FMT_BGR555,         SDL_PIXELFORMAT_BGR555 },
    { AV_PIX_FMT_RGB565,         SDL_PIXELFORMAT_RGB565 },
    { AV_PIX_FMT_BGR565,         SDL_PIXELFORMAT_BGR565 },
    { AV_PIX_FMT_RGB24,          SDL_PIXELFORMAT_RGB24 },
    { AV_PIX_FMT_BGR24,          SDL_PIXELFORMAT_BGR24 },
    { AV_PIX_FMT_0RGB32,         SDL_PIXELFORMAT_RGB888 },
    { AV_PIX_FMT_0BGR32,         SDL_PIXELFORMAT_BGR888 },
    { AV_PIX_FMT_NE(RGB0, 0BGR), SDL_PIXELFORMAT_RGBX8888 },
    { AV_PIX_FMT_NE(BGR0, 0RGB), SDL_PIXELFORMAT_BGRX8888 },
    { AV_PIX_FMT_RGB32,          SDL_PIXELFORMAT_ARGB8888 },
    { AV_PIX_FMT_RGB32_1,        SDL_PIXELFORMAT_RGBA8888 },
    { AV_PIX_FMT_BGR32,          SDL_PIXELFORMAT_ABGR8888 },
    { AV_PIX_FMT_BGR32_1,        SDL_PIXELFORMAT_BGRA8888 },
    { AV_PIX_FMT_YUV420P,        SDL_PIXELFORMAT_IYUV },
    { AV_PIX_FMT_YUYV422,        SDL_PIXELFORMAT_YUY2 },
    { AV_PIX_FMT_UYVY422,        SDL_PIXELFORMAT_UYVY },
    { AV_PIX_FMT_NONE,           SDL_PIXELFORMAT_UNKNOWN },
};

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
            st->renderer = SDL_CreateRenderer(st->window, -1, 0);
        }
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
            xlog_trc("remaining time=%.2f", remaininig_time);
            if (remaininig_time > 0.0)
            {
                std::this_thread::sleep_for(
                    std::chrono::microseconds((uint64_t)(remaininig_time * 1000000.0)));
            }
            remaininig_time = 0.01;

            videoRefresh(&remaininig_time);

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

void XDisplay::videoRefresh(double *remaining_time)
{
    /*
     can be moved to xplay as
     xplay.video_refresh();
    */

    VideoState& vs = _st->xplay->vs();

    if (vs.pictq->nb_remaining() == 0)
    {
        xlog_trc("picq empty");
    }
    else
    {
        Frame* vp = nullptr;
        Frame* last_vp = nullptr;
        double last_duration = 0.0;
        double duration = 0.0;
        double delay = 0.0;
        double time = 0.0;

        for(;;)
        {
            last_vp = vs.pictq->peek_last();
            vp = vs.pictq->peek();

            xlog_trc("serial(vp:%d,last_vp:%d,queue:%d)", 
                vp->serial, last_vp->serial, vs.videoq->serial);
            if (vp->serial != vs.videoq->serial)
            {
                xlog_trc("serial changed, next");
                vs.pictq->next();
                continue;
            }

            if (last_vp->serial != vp->serial)
            {
                _st->frame_timer = av_gettime_relative() / 1000000.0;
            }

            last_duration = vp_duration(last_vp, vp);
            delay = compute_target_delay(last_duration);

            time = av_gettime_relative() / 1000000.0;

            xlog_trc("time=%.2f,frame_timer=%.2f,delay=%.2f",
                time, _st->frame_timer, delay);
                
            if (time < _st->frame_timer + delay)
            {
                *remaining_time = std::min(_st->frame_timer + delay - time, 
                    *remaining_time);
                break;
            }

            _st->frame_timer += delay;
            if (delay > 0 && time - _st->frame_timer > AV_SYNC_THRESHOLD_MAX)
            {
                _st->frame_timer = time;
            }

            vs.pictq->next();
            _st->force_refresh = true;

            break;
        }
    }

    videoDisplay();

    _st->force_refresh = false;
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

    /* xplay.last_frame.rect */
    calDisplayRect(&rect, _st->xleft, _st->ytop, _st->width, _st->height,
        vp->width, vp->height, vp->sar);

    /* xplay.last_frame */
    setSDLYUVConversionMode(vp->frame);

    /* xplay.last_frame */
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
    AVRational aspect_ratio = pic_sar;
    int64_t width, height, x, y;

    if (av_cmp_q(aspect_ratio, av_make_q(0, 1)) <= 0)
        aspect_ratio = av_make_q(1, 1);

    aspect_ratio = av_mul_q(aspect_ratio, av_make_q(pic_width, pic_height));

    /* XXX: we suppose the screen has a 1.0 pixel ratio */
    height = scr_height;
    width = av_rescale(height, aspect_ratio.num, aspect_ratio.den) & ~1;
    if (width > scr_width) {
        width = scr_width;
        height = av_rescale(width, aspect_ratio.den, aspect_ratio.num) & ~1;
    }
    x = (scr_width - width) / 2;
    y = (scr_height - height) / 2;
    rect->x = scr_xleft + x;
    rect->y = scr_ytop + y;
    rect->w = FFMAX((int)width, 1);
    rect->h = FFMAX((int)height, 1);
}

void XDisplay::setSDLYUVConversionMode(AVFrame* frame)
{
#if SDL_VERSION_ATLEAST(2,0,8)
    SDL_YUV_CONVERSION_MODE mode = SDL_YUV_CONVERSION_AUTOMATIC;
    if (frame && (frame->format == AV_PIX_FMT_YUV420P || frame->format == AV_PIX_FMT_YUYV422 || frame->format == AV_PIX_FMT_UYVY422)) {
        if (frame->color_range == AVCOL_RANGE_JPEG)
            mode = SDL_YUV_CONVERSION_JPEG;
        else if (frame->colorspace == AVCOL_SPC_BT709)
            mode = SDL_YUV_CONVERSION_BT709;
        else if (frame->colorspace == AVCOL_SPC_BT470BG || frame->colorspace == AVCOL_SPC_SMPTE170M)
            mode = SDL_YUV_CONVERSION_BT601;
    }
    SDL_SetYUVConversionMode(mode); /* FIXME: no support for linear transfer */
#endif
}

int XDisplay::uploadTexture(SDL_Texture** tex, AVFrame* frame)
{
    int ret = 0;
    Uint32 sdl_pix_fmt;
    SDL_BlendMode sdl_blendmode;
    getSDLPixFmtAndBlendMode(frame->format, &sdl_pix_fmt, &sdl_blendmode);
    if (reallocTexture(tex, sdl_pix_fmt == SDL_PIXELFORMAT_UNKNOWN ? SDL_PIXELFORMAT_ARGB8888 : sdl_pix_fmt, frame->width, frame->height, sdl_blendmode, 0) < 0)
        return -1;
    switch (sdl_pix_fmt) {
    case SDL_PIXELFORMAT_IYUV:
        if (frame->linesize[0] > 0 && frame->linesize[1] > 0 && frame->linesize[2] > 0) {
            ret = SDL_UpdateYUVTexture(*tex, NULL, frame->data[0], frame->linesize[0],
                frame->data[1], frame->linesize[1],
                frame->data[2], frame->linesize[2]);
        }
        else if (frame->linesize[0] < 0 && frame->linesize[1] < 0 && frame->linesize[2] < 0) {
            ret = SDL_UpdateYUVTexture(*tex, NULL, frame->data[0] + frame->linesize[0] * (frame->height - 1), -frame->linesize[0],
                frame->data[1] + frame->linesize[1] * (AV_CEIL_RSHIFT(frame->height, 1) - 1), -frame->linesize[1],
                frame->data[2] + frame->linesize[2] * (AV_CEIL_RSHIFT(frame->height, 1) - 1), -frame->linesize[2]);
        }
        else {
            xlog_err("Mixed negative and positive linesizes are not supported.");
            return -1;
        }
        break;
    default:
        if (frame->linesize[0] < 0) {
            ret = SDL_UpdateTexture(*tex, NULL, frame->data[0] + frame->linesize[0] * (frame->height - 1), -frame->linesize[0]);
        }
        else {
            ret = SDL_UpdateTexture(*tex, NULL, frame->data[0], frame->linesize[0]);
        }
        break;
    }
    return ret;
}

void XDisplay::getSDLPixFmtAndBlendMode(int format, uint32_t* sdl_pix_fmt,
    SDL_BlendMode* sdl_blendmode)
{
    int i;
    *sdl_blendmode = SDL_BLENDMODE_NONE;
    *sdl_pix_fmt = SDL_PIXELFORMAT_UNKNOWN;
    if (format == AV_PIX_FMT_RGB32 ||
        format == AV_PIX_FMT_RGB32_1 ||
        format == AV_PIX_FMT_BGR32 ||
        format == AV_PIX_FMT_BGR32_1)
        *sdl_blendmode = SDL_BLENDMODE_BLEND;
    for (i = 0; i < ARRAY_ELEMS(sdl_texture_format_map) - 1; i++) {
        if (format == sdl_texture_format_map[i].format) {
            *sdl_pix_fmt = sdl_texture_format_map[i].texture_fmt;
            return;
        }
    }
}

int XDisplay::reallocTexture(SDL_Texture** texture, uint32_t new_format,
    int new_width, int new_height, SDL_BlendMode blendmode,
    int init_texture)
{
    Uint32 format;
    int access, w, h;
    if (!*texture || SDL_QueryTexture(*texture, &format, &access, &w, &h) < 0 || new_width != w || new_height != h || new_format != format) {
        void* pixels;
        int pitch;
        if (*texture)
            SDL_DestroyTexture(*texture);
        if (!(*texture = SDL_CreateTexture(_st->renderer, new_format, SDL_TEXTUREACCESS_STREAMING, new_width, new_height)))
            return -1;
        if (SDL_SetTextureBlendMode(*texture, blendmode) < 0)
            return -1;
        if (init_texture) {
            if (SDL_LockTexture(*texture, NULL, &pixels, &pitch) < 0)
                return -1;
            memset(pixels, 0, pitch * new_height);
            SDL_UnlockTexture(*texture);
        }
        av_log(NULL, AV_LOG_VERBOSE, "Created %dx%d texture with %s.\n", new_width, new_height, SDL_GetPixelFormatName(new_format));
    }
    return 0;
}

double XDisplay::vp_duration(Frame* vp, Frame* nextvp)
{
    auto max_frame_duration = _st->xplay->vs().max_frame_duration;
    if (vp->serial == nextvp->serial) {
        double duration = nextvp->pts - vp->pts;
        if (isnan(duration) || duration <= 0 || duration > max_frame_duration)
            return vp->duration;
        else
            return duration;
    }
    else {
        return 0.0;
    }
}

double XDisplay::compute_target_delay(double delay)
{
    double sync_threshold, diff = 0;

    /* update delay to follow master synchronisation source */
    //if (get_master_sync_type(is) != AV_SYNC_VIDEO_MASTER) {
    //    /* if video is slave, we try to correct big delays by
    //       duplicating or deleting a frame */
    //    diff = get_clock(&is->vidclk) - get_master_clock(is);

    //    /* skip or repeat frame. We take into account the
    //       delay to compute the threshold. I still don't know
    //       if it is the best guess */
    //    sync_threshold = FFMAX(AV_SYNC_THRESHOLD_MIN, FFMIN(AV_SYNC_THRESHOLD_MAX, delay));
    //    if (!isnan(diff) && fabs(diff) < is->max_frame_duration) {
    //        if (diff <= -sync_threshold)
    //            delay = FFMAX(0, delay + diff);
    //        else if (diff >= sync_threshold && delay > AV_SYNC_FRAMEDUP_THRESHOLD)
    //            delay = delay + diff;
    //        else if (diff >= sync_threshold)
    //            delay = 2 * delay;
    //    }
    //}

    xlog_trc("video: delay=%0.3f A-V=%f\n", delay, -diff);

    return delay;
}