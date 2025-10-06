
#include "xdemuxer.hpp"

#include <vector>

#include "xlog.h"

namespace {

constexpr size_t IO_BUF_SIZE = 128;

struct xdemuxer_ctx {
    struct xdemuxer_param param = {};

    AVFormatContext *av_format_ctx = nullptr;
};

int read_packet_cb(void *opaque, uint8_t *buf, int buf_size)
{
    auto *ctx = reinterpret_cast<xdemuxer_ctx*>(opaque);

    if (ctx->param.read_cb != nullptr) {
        buf_size = ctx->param.read_cb(buf, buf_size, ctx->param.userdata);
    }

    return buf_size;
}

int xdemuxer_close_imp(struct xdemuxer_ctx *ctx)
{
    if (nullptr != ctx) {
        if (nullptr != ctx->av_format_ctx) {
            avformat_free_context(ctx->av_format_ctx);
            ctx->av_format_ctx = nullptr;
        }
    }

    return 0;
}

int xdemuxer_open_input(struct xdemuxer_ctx *ctx)
{
    bool error_flag = false;

    void *io_buf = nullptr;

    do {
        int ret = 0;
        if (ctx->av_format_ctx != nullptr) {
            xlog_dbg("already init\n");
            break;
        }

        ctx->av_format_ctx = avformat_alloc_context();
        if (ctx->av_format_ctx == nullptr) {
            xlog_err("avformat_alloc_context failed\n");
            error_flag = true;
            break;
        }

        ctx->av_format_ctx->opaque = ctx;

        io_buf = av_malloc(IO_BUF_SIZE);
        ctx->av_format_ctx->pb = avio_alloc_context((unsigned char*)io_buf, IO_BUF_SIZE, 
            0, ctx, read_packet_cb, nullptr, nullptr);
        if (nullptr == ctx->av_format_ctx->pb) {
            xlog_err("avio_alloc_context failed\n");
            error_flag = true;
            break;
        }
        io_buf = nullptr;

        ret = avformat_open_input(&ctx->av_format_ctx, nullptr, nullptr, nullptr);
        if (ret < 0) {
            xlog_err("avformat_open_input failed\n");
            error_flag = true;
            break;
        }
    } while (false);

    if (error_flag) {
        xdemuxer_close_imp(ctx);

        if (io_buf != nullptr) {
            av_free(io_buf);
            io_buf = nullptr;
        }
    }

    return error_flag ? -1 : 0;
}

}

xdemuxer_handle xdemuxer_open(struct xdemuxer_param const* param)
{
    struct xdemuxer_ctx *ctx = nullptr;
    do {
        {
            av_log_set_level(AV_LOG_TRACE);
            av_log_set_flags(AV_LOG_SKIP_REPEATED | AV_LOG_PRINT_LEVEL);
        }

        ctx = new xdemuxer_ctx();
        ctx->param = *param;
    } while (false);

    return reinterpret_cast<xdemuxer_handle>(ctx);
}

int xdemuxer_read(xdemuxer_handle handle)
{
    bool error_flag = false;
    auto *obj = reinterpret_cast<xdemuxer_ctx*>(handle);

    AVPacket *pkt = nullptr;
    do {
        int ret = 0;
        if (obj->av_format_ctx == nullptr) {
            ret = xdemuxer_open_input(obj);
            if (ret < 0) {
                error_flag = true;
                break;
            }
        }

        pkt = av_packet_alloc();
        if (pkt == nullptr) {
            error_flag = true;
            xlog_err("av_packet_alloc failed\n");
            break;
        }

        ret = av_read_frame(obj->av_format_ctx, pkt);
        if (ret < 0) {
            xlog_err("av_read_frame failed\n");
            error_flag = true;
            break;
        }

        if (obj->param.on_packet_cb != nullptr) {
            obj->param.on_packet_cb(pkt, obj->param.userdata);
        }

        av_packet_unref(pkt);
    } while (false);

    if (pkt != nullptr) {
        av_packet_free(&pkt);
    }

    return error_flag ? -1 : 0;
}

int xdemuxer_close(xdemuxer_handle handle)
{
    auto *obj = reinterpret_cast<xdemuxer_ctx*>(handle);

    xdemuxer_close_imp(obj);

    delete obj;
    obj = nullptr;

    return 0;
}