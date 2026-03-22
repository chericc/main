#include "ffmpeg-file-source.hpp"

extern "C" {
#include "libavformat/avformat.h"
}

#include "xlog.h"

enum class Status {
    INIT = 0,
    PLAY = 1,
    PAUSE = 2,
    END = 3,
};

struct FFFileSource::Context {
    AVFormatContext *avformat_ctx = nullptr;
    AVPacket pkt{};
    Status status = Status::INIT;
};

FFFileSource::FFFileSource(const char *file)
{
    bool okFlag = false;

    do {
        _ctx = std::make_shared<Context>();

        _ctx->avformat_ctx = avformat_alloc_context();
        if (nullptr == _ctx->avformat_ctx) {
            xlog_err("avformat_alloc_context failed");
            break;
        }

        int ret = avformat_open_input(&_ctx->avformat_ctx, file, nullptr, nullptr);
        if (ret != 0) {
            xlog_err("avformat_open_input failed");
            break;
        }

        ret = avformat_find_stream_info(_ctx->avformat_ctx, nullptr);
        if (ret < 0) {
            xlog_err("avformat_find_stream_info failed");
            break;
        }

        xlog_dbg("open done");
        okFlag = true;
    } while (false);

    if (!okFlag) {
        xlog_err("open failed");
    }

}

int FFFileSource::Play()
{
    bool sendFrame = false;
    do {
        if (_ctx->status == Status::END) {
            break;
        }

        
    } while (false);
    return sendFrame;
}