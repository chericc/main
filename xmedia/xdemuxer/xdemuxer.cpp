
#include "xdemuxer.h"

#include <vector>

extern "C" {
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavcodec/avcodec.h"
}

#include "xlog.h"

namespace {

constexpr size_t IO_BUF_SIZE = 128;

struct xdemuxer_ctx {
    struct xdemuxer_param param = {};

    AVFormatContext *av_format_ctx = nullptr;
};

int read_packet_cb(void *opaque, uint8_t *buf, int buf_size)
{
    xlog_dbg("read cb: buf_size=%d\n", buf_size);

    memset(buf, 0, buf_size);

    return buf_size;
}

int xdemuxer_close_imp(struct xdemuxer_ctx *ctx)
{
    if (nullptr != ctx) {
        if (nullptr != ctx->av_format_ctx) {
            avformat_free_context(ctx->av_format_ctx);
            ctx->av_format_ctx = nullptr;
        }
        delete ctx;
    }

    return 0;
}

}

xdemuxer_handle xdemuxer_open(struct xdemuxer_param const* param)
{
    bool error_flag = false;
    struct xdemuxer_ctx *ctx = nullptr;

    void *io_buf = nullptr;

    do {
        int ret = 0;

        {
            av_log_set_level(AV_LOG_TRACE);
            av_log_set_flags(AV_LOG_SKIP_REPEATED | AV_LOG_PRINT_LEVEL);
        }

        ctx = new xdemuxer_ctx();

        ctx->param = *param;

        ctx->av_format_ctx = avformat_alloc_context();
        if (ctx->av_format_ctx == nullptr) {
            xlog_err("avformat_alloc_context failed\n");
            error_flag = true;
            break;
        }

        ctx->av_format_ctx->opaque = ctx;

        io_buf = av_malloc(IO_BUF_SIZE);
        ctx->av_format_ctx->pb = avio_alloc_context((unsigned char*)io_buf, IO_BUF_SIZE, 
            0, nullptr, read_packet_cb, nullptr, nullptr);
        if (nullptr == ctx->av_format_ctx->pb) {
            xlog_err("avio_alloc_context failed\n");
            error_flag = true;
            break;
        }

        ret = avformat_open_input(&ctx->av_format_ctx, nullptr, nullptr, nullptr);
        if (ret < 0) {
            xlog_err("avformat_open_input failed\n");
            error_flag = true;
            break;
        }
    } while (0);

    if (error_flag) {
        xdemuxer_close_imp(ctx);
        ctx = nullptr;
    }

    return reinterpret_cast<xdemuxer_handle>(ctx);
}

int xdemuxer_input(xdemuxer_handle handle, const uint8_t *data, size_t size)
{
    bool error_flag = false;
    auto *obj = reinterpret_cast<xdemuxer_ctx*>(handle);

    AVPacket *pkt = nullptr;
    do {
        pkt = av_packet_alloc();
        if (pkt == nullptr) {
            error_flag = true;
            xlog_err("av_packet_alloc failed\n");
            break;
        }

        av_read_frame(obj->av_format_ctx, pkt);

        av_packet_unref(pkt);
        av_packet_free(&pkt);
    } while (false);

    return error_flag ? -1 : 0;
}

int xdemuxer_close(xdemuxer_handle handle)
{
    auto *obj = reinterpret_cast<xdemuxer_ctx*>(handle);

    xdemuxer_close_imp(obj);
    return 0;
}