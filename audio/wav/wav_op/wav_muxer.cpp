
#include "wav_muxer.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <memory>
#include <vector>

#include "wav_private.h"

#include "xlog.h"
#include "xio_fp.hpp"

namespace {

/* Ref: wav/fmt: ffmpeg/libavformat/riff.c:ff_codec_wav_tags */
enum class RiffFmt {
    PCM = 0x1,
    PCM_ALAW = 0x6,
    PCM_MULAW = 0x7,
};

struct WAV_TAG {
    uint32_t chunkid;
    uint32_t chunksize;
};

struct WAV_RIFF {
    WAV_TAG tag;
    uint32_t format;
};

struct WAV_SubChunkFmt {
    WAV_TAG tag;
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t align;
    uint16_t bits_per_sample;
};

struct WAV_SubChunkData {
    WAV_TAG tag;
};

class wav_muxer_context {
public:
    struct wav_muxer_info info_backup = {};
    bool header_flag = 0;
    std::shared_ptr<XIOFp> xio_fp = nullptr;
    std::vector<uint8_t> buf;

    size_t input_data_size;

    int64_t pos_riff_chunksize = -1;
    int64_t pos_data_chunksize = -1;
};

int wav_muxer_destroy_imp(wav_muxer_context *ctx)
{
    if (ctx != nullptr) {
        free(ctx);
        ctx = nullptr;
    }

    return 0;
}

int wav_muxer_write_header(wav_muxer_context *ctx)
{
    bool error_flag = false;

    do {
        if (nullptr == ctx) {
            xlog_err("null obj\n");
            error_flag = true;
            break;
        }

        // riff
        uint32_t chunkid = MKTAG('R', 'I', 'F', 'F');
        ctx->xio_fp->wl32(chunkid);
        ctx->pos_riff_chunksize = ctx->xio_fp->tell();
        ctx->xio_fp->wl32(-1); // chunksize: file size - 8
        uint32_t format = MKTAG('W', 'A', 'V', 'E');
        ctx->xio_fp->wl32(format);

        auto &wav_info = ctx->info_backup;

        // fmt 
        chunkid = MKTAG('f', 'm', 't', ' ');
        ctx->xio_fp->wl32(chunkid);
        uint32_t chunksize = 16; // 8 + others(16)
        ctx->xio_fp->wl32(chunksize);
        ctx->xio_fp->wl16(wav_info.info.audio_type);
        ctx->xio_fp->wl16(wav_info.info.channels);
        ctx->xio_fp->wl32(wav_info.info.sample_rate);
        ctx->xio_fp->wl32(wav_info.info.sample_rate * 
            wav_info.info.bits_per_sample * wav_info.info.channels / 8); // byte rate
        ctx->xio_fp->wl16(wav_info.info.channels * 
            wav_info.info.bits_per_sample / 8); // block align
        ctx->xio_fp->wl16(wav_info.info.bits_per_sample);

        // data
        chunkid = MKTAG('d', 'a', 't', 'a');
        ctx->xio_fp->wl32(chunkid);
        ctx->pos_data_chunksize = ctx->xio_fp->tell();
        ctx->xio_fp->wl32(-1);
    } while (0);

    return error_flag ? -1 : 0;
}

}

wav_muxer_handle wav_muxer_create(struct wav_muxer_info const *info)
{
    bool error_flag = false;
    wav_muxer_context *ctx = nullptr;

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

        ctx = new wav_muxer_context();

        ctx->info_backup = *info;
        ctx->header_flag = false;
        ctx->xio_fp = std::make_shared<XIOFp>(ctx->info_backup.fp);
        if (ctx->xio_fp->error()) {
            xlog_err("io error\n");
            error_flag = true;
            break;
        }
    } while (0);

    if (error_flag) {
        if (ctx) {
            wav_muxer_destroy_imp(ctx);
            ctx = nullptr;
        }
    }

    return reinterpret_cast<wav_muxer_context*>(ctx);
}

int wav_muxer_input(wav_muxer_handle handle, const void *chunk, size_t chunksize, size_t count)
{
    bool error_flag = false;

    do {
        int ret = 0;

        auto *ctx = reinterpret_cast<wav_muxer_context*>(handle);
        if (nullptr == ctx) {
            xlog_err("null obj\n");
            error_flag = true;
            break;
        }

        if (nullptr == ctx->xio_fp) {
            xlog_err("io null\n");
            error_flag = true;
            break;
        }

        if (!ctx->header_flag) {
            ret = wav_muxer_write_header(ctx);
            if (ret < 0) {
                xlog_err("write header failed\n");
                error_flag = true;
                break;
            }
            ctx->header_flag = true;
        }

        // todo: check chunk size

        for (size_t i = 0; i < count; ++i) {
            if (ctx->buf.size() != chunksize) {
                ctx->buf.resize(chunksize);
            }

            memcpy(ctx->buf.data(), (uint8_t*)chunk + (i * chunksize), chunksize);
            ctx->xio_fp->write(ctx->buf);
            ctx->input_data_size += chunksize;
        }

        if (ctx->xio_fp->error()) {
            xlog_err("io error\n");
            error_flag = true;
            break;
        }
    } while (false);

    return error_flag ? -1 : 0;
}

int wav_muxer_close(wav_muxer_handle handle)
{
    bool error_flag = true;
    auto *ctx = reinterpret_cast<wav_muxer_context*>(handle);

    do {
        if (nullptr == ctx) {
            xlog_err("null obj\n");
            error_flag = true;
            break;
        }

        if (ctx->xio_fp == nullptr) {
            xlog_err("io null\n");
            error_flag = true;
            break;
        }

        if (ctx->pos_data_chunksize < 0
            || ctx->pos_riff_chunksize < 0) {
            xlog_err("pos empty\n");
            error_flag = true;
            break;
        }

        ctx->xio_fp->seek(0, SEEK_END);
        auto filesize = ctx->xio_fp->tell();

        uint32_t chunksize = static_cast<uint32_t>(filesize - 8);
        ctx->xio_fp->seek(ctx->pos_riff_chunksize, SEEK_SET);
        ctx->xio_fp->wl32(chunksize);

        chunksize = static_cast<uint32_t>(ctx->input_data_size);
        ctx->xio_fp->seek(ctx->pos_data_chunksize, SEEK_SET);
        ctx->xio_fp->wl32(chunksize);

        ctx->xio_fp.reset();

    } while (false);

    if (ctx != nullptr) {
        delete ctx;
        ctx = nullptr;
    }

    return error_flag ? -1 : 0;
}