#include "wav_demuxer.h"

#include <memory>
#include <cstring>

#include "xio_fp.hpp"
#include "wav_private.h"
#include "xlog.h"

namespace {

struct wav_demuxer_ctx {
    struct wav_demuxer_info info_backup;
    std::shared_ptr<XIO> xio;
    struct wav_info wav_info;
    size_t pos_data;
    size_t size_data;
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

            if (static_cast<bool>(xio->eof())) {
                xlog_dbg("eof\n");
                break;
            }

            if (static_cast<bool>(xio->error())) {
                xlog_err("io error\n");
                error_flag = true;
                break;
            }

            uint32_t chunkid = xio->rl32();
            uint32_t chunksize = xio->rl32();

            switch (chunkid) {
                case MKTAG('R', 'I', 'F', 'F'): {
                    auto format = xio->rl32();
                    if (format != MKTAG('W', 'A', 'V', 'E')) {
                        xlog_err("format not match: %#x\n", (unsigned int)format);
                        error_flag = true;
                        break;
                    }
                    has_riff_flag = true;
                    break;
                }
                case MKTAG('f', 'm', 't', ' '): {
                    int64_t pos_next_tag = xio->tell() + chunksize;
                    ctx->wav_info.audio_type = static_cast<WAV_AUDIO_TYPE>(xio->rl16());
                    ctx->wav_info.channels = xio->rl16();
                    ctx->wav_info.sample_rate = static_cast<int>(xio->rl32());
                    uint32_t byte_rate = xio->rl32();
                    uint32_t align = xio->rl16();
                    ctx->wav_info.bits_per_sample = xio->rl16();
                    if (xio->tell() != pos_next_tag) {
                        xlog_dbg("additional info in fmt, ignored\n");
                        if (xio->seek(pos_next_tag, SEEK_SET) < 0) {
                            xlog_err("seek failed\n");
                            error_flag = true;
                            break;
                        }
                    }
                    // checks
                    constexpr uint32_t bitsperbyte = 8;
                    uint32_t cal_byte_rate = ctx->wav_info.sample_rate * 
                        ctx->wav_info.bits_per_sample * ctx->wav_info.channels / bitsperbyte;
                    uint32_t cal_align = ctx->wav_info.bits_per_sample * ctx->wav_info.channels / bitsperbyte;
                    if (byte_rate != cal_byte_rate) {
                        xlog_err("bytes rate check failed(%u != %u)\n", 
                            (unsigned int)byte_rate, (unsigned int)cal_byte_rate);
                        error_flag = true;
                        break;
                    }
                    if (align != cal_align) {
                        xlog_err("align check failed(%u != %u)\n",
                            (unsigned int)align, (unsigned int)cal_align);
                        error_flag = true;
                        break;
                    }

                    has_fmt_flag = true;
                    break;
                }
                case MKTAG('d', 'a', 't', 'a'): {
                    ctx->pos_data = xio->tell();
                    ctx->size_data = chunksize;
                    if (xio->seek(chunksize, SEEK_CUR) < 0) {
                        xlog_err("seek omit data chunk failed\n");
                        error_flag = true;
                        break;
                    }
                    has_data_flag = true;
                    break;
                }
                default: {
                    uint8_t chunkname[5] = {};
                    static_assert(sizeof(chunkid) < sizeof(chunkname), "chunkid check failed");
                    memcpy(chunkname, &chunkid, sizeof(chunkname) - 1);
                    xlog_dbg("chunk ignored: %s\n", (char*)chunkname);

                    if (xio->seek(chunksize, SEEK_CUR) < 0) {
                        xlog_err("seek omit chunk failed(%s)\n", chunkname);
                        error_flag = true;
                        break;
                    }
                    break;
                }
            }
        }

        if (error_flag) {
            break;
        }
        if (!has_data_flag || !has_fmt_flag || !has_riff_flag) {
            xlog_err("chunk incomplete(riff=%d,fmt=%d,data=%d)\n", 
                has_riff_flag, has_fmt_flag, has_data_flag);
            error_flag = true;
            break;
        }
    } while (false);

    return error_flag ? -1 : 0;
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

int wav_demuxer_get_info(wav_demuxer_handle handle, struct wav_info *info)
{

}

int wav_demuxer_get_data(wav_demuxer_handle handle, size_t offset, size_t size, void *output_data, size_t output_data_size)
{

}

int wav_demuxer_close(wav_demuxer_handle handle)
{

}