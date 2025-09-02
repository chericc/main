#include "wav_demuxer.h"

#include <memory>

#include "xio_fp.hpp"
#include "wav_private.h"
#include "xlog.h"

namespace {

struct wav_demuxer_ctx {
    struct wav_demuxer_info info_backup;
    std::shared_ptr<XIO> xio;
    struct wav_info wav_info;
};

int load_demuxer_info(struct wav_demuxer_ctx *ctx, struct wav_info *info)
{
    bool error_flag = false;

    do {
        if (nullptr == info) {
            error_flag = true;
            xlog_err("null info\n");
            break;
        }

        bool has_riff_flag = false;
        bool has_fmt_flag = false;
        bool has_data_flag = false;

        auto &xio = ctx->xio;
        while (true) {
            if (error_flag) {
                xlog_err("error\n");
                error_flag = true;
                break;
            }

            if (xio->eof()) {
                xlog_dbg("eof\n");
                break;
            }

            if (xio->error()) {
                xlog_err("io error\n");
                error_flag = true;
                break;
            }

            uint32_t chunkid = xio->rl32();
            uint32_t chunksize = xio->rl32();

            switch (chunkid) {
                case MKTAG('R', 'I', 'F', 'F'): {
                    ctx->wav_info.audio_type = static_cast<enum WAV_AUDIO_TYPE>(xio->rl32());
                    has_riff_flag = true;
                    break;
                }
            }
        }
    } while (false);
}

int wav_demuxer_destroy_imp(struct wav_demuxer_ctx *ctx)
{
    if (nullptr != ctx) {
        if (ctx->xio) {
            ctx->xio.reset();
        }

        delete ctx;
        ctx = nullptr;
    }

    return 0;
}

}

wav_demuxer_handle wav_demuxer_create(struct wav_demuxer_info const *info)
{
    bool error_flag = false;
    struct wav_demuxer_ctx *ctx = nullptr;
    do {
        if (nullptr == info) {
            xlog_err("null info\n");
            error_flag = true;
            break;
        }

        if (nullptr == info->fp) {
            xlog_err("null fp\n");
            error_flag = true;
            break;
        }

        ctx = new struct wav_demuxer_ctx();

        ctx->info_backup = *info;

        ctx->xio = std::make_shared<XIOFp>(ctx->info_backup.fp);

        if (ctx->xio->error()) {
            error_flag = true;
            break;
        }
    } while (false);

    if (error_flag) {
        wav_demuxer_destroy_imp(ctx);
        ctx = nullptr;
    }

    return static_cast<wav_demuxer_handle>(ctx);
}

int wav_demuxer_get_info(struct wav_info *info)
{

}

int wav_demuxer_get_data(size_t offset, size_t size, void *output_data, size_t output_data_size)
{

}

int wav_demuxer_close(wav_demuxer_handle handle)
{

}