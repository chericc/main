
#include "wav_muxer.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "xlog.h"

#define MKTAG(a, b, c, d) \
    ((a) | ((b) << 8) | ((c) << 16) | ((unsigned)(d) << 24))

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

struct wav_muxer_context {
    struct wav_muxer_info info_backup;
    int header_flag;
};

int wav_muxer_destroy_imp(struct wav_muxer_context *ctx)
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
        uint32_t chinkid = MKTAG('R', 'I', 'F', 'F');
        
    } while (0);
}

}

wav_muxer_handle wav_muxer_create(struct wav_muxer_info const *info)
{
    bool error_flag = false;
    struct wav_muxer_context *ctx = nullptr;

    do {
        ctx = (struct wav_muxer_context*)malloc(sizeof(struct wav_muxer_context));
        if (ctx == nullptr) {
            xlog_err("malloc failed\n");
            error_flag = true;
            break;
        }

        memset(ctx, 0, sizeof(*ctx));

        ctx->info_backup = *info;
    } while (0);

    if (error_flag) {
        if (ctx) {
            wav_muxer_destroy_imp(ctx);
            ctx = nullptr;
        }
    }

    return reinterpret_cast<struct wav_muxer_context*>(ctx);
}

int wav_muxer_input(wav_muxer_handle handle, const void *chunk, size_t count)
{

}

int wav_muxer_close(wav_muxer_handle handle)
{

}