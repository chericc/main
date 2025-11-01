#include "myffmpeg.hpp"

#include "xlog.h"

MyFFmpeg::MyFFmpeg(std::string const&file)
    : file_(file)
{
    // av_log_set_level(AV_LOG_ERROR);
    av_log_set_level(AV_LOG_TRACE);
}

MyFFmpeg::~MyFFmpeg()
{
    if (ffctx_ != nullptr) {
        closeImp(ffctx_);
        ffctx_ = nullptr;
    }
    // end
}

bool MyFFmpeg::open()
{
    bool error_flag = false;
    do {
        ffctx_ = openImp(file_.c_str());
        if (nullptr == ffctx_) {
            error_flag = true;
            break;
        }
    } while (false);

    return !error_flag;
}

std::shared_ptr<MyFFmpegInfo> MyFFmpeg::getInfo()
{
    bool error_flag = false;
    auto info = std::make_shared<MyFFmpegInfo>();

    do {
        int ret = 0;
        if (nullptr == ffctx_) {
            error_flag = true;
            break;
        }

        if (ffctx_->index_video < 0 || ffctx_->index_video >= ffctx_->context->nb_streams) {
            xlog_err("invalid index: %d(%d)\n", ffctx_->index_video, ffctx_->context->nb_streams);
            error_flag = true;
            break;
        }

        AVStream *stream = ffctx_->context->streams[ffctx_->index_video];

        if (nullptr == stream->codecpar) {
            xlog_err("inner error: codecpar is null\n");
            error_flag = true;
            break;
        }

        info->codec = stream->codecpar->codec_id;
        info->width = stream->codecpar->width;
        info->height = stream->codecpar->height;

        if (ffctx_->context->metadata != nullptr) {
            const AVDictionaryEntry *tag = nullptr;
            while (true) {
                tag = av_dict_iterate(ffctx_->context->metadata, tag);
                if (tag == nullptr) {
                    break;
                }
                if (strcmp(tag->key, "major_brand") == 0) {
                    info->majorBrand = tag->value;
                }
            }
        }
        
        if (ffctx_->context->iformat != nullptr) {
            if (ffctx_->context->iformat->name != nullptr) {
                info->format = ffctx_->context->iformat->name;
            }
        }
    } while (false);

    if (error_flag) {
        info = nullptr;
    }

    return info;
}

bool MyFFmpeg::popPacket(std::vector<uint8_t> &buf)
{
    bool error_flag = false;

    AVPacket *pkt = nullptr;
    bool eof = false;

    do {
        if (nullptr == ffctx_) {
            xlog_err("not opened yet\n");
            error_flag = true;
            break;
        }

        while (true) {
            int ret = popPacketOnce(ffctx_, buf);
            if (ret >= 0) {
                break;
            } 
            if (ret == AVERROR(EAGAIN)) {
                continue;
            }
            if (ret == AVERROR_EOF) {
                eof = true;
                break;
            }

            xlog_err("pop packet failed\n");
            error_flag = true;
            break;

        }

    } while (false);

    if (error_flag) {
        return false;
    }
    return !eof;
}

MyFFmpeg::FFmpegCtxPtr MyFFmpeg::openImp(const char *file)
{
    auto fftmp = std::make_shared<FFmpegCtx>();

    bool error_flag = false;
    do {
        int ret = 0;

        ret = avformat_open_input(&fftmp->context, file, nullptr, nullptr);
        if (ret < 0) {
            xlog_err("avformat_open_input failed\n");
            error_flag = true;
            break;
        }

        ret = avformat_find_stream_info(fftmp->context, nullptr);
        if (ret < 0) {
            xlog_err("avformat_find_stream_info failed\n");
            error_flag = true;
            break;
        }

        fftmp->index_video = av_find_best_stream(fftmp->context, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
        if (fftmp->index_video < 0) {
            xlog_err("av_find_best_stream failed\n");
            error_flag = true;
            break;
        }

        AVStream *stream = fftmp->context->streams[fftmp->index_video];

        if (nullptr == stream->codecpar) {
            xlog_err("inner error: codecpar is null\n");
            error_flag = true;
            break;
        }

        // 两种需要硬件解码的流格式需要考虑包格式兼容性
        if ((stream->codecpar->codec_id == AV_CODEC_ID_H264)
            || (stream->codecpar->codec_id == AV_CODEC_ID_HEVC)) {
            auto const* name = ((stream->codecpar->codec_id == AV_CODEC_ID_H264)
                 ? "h264_mp4toannexb" : "hevc_mp4toannexb");
            auto const* bsf_filter = av_bsf_get_by_name(name);
            if (nullptr == bsf_filter) {
                xlog_err("get bsf_filter failed\n");
                error_flag = true;
                break;
            }

            ret = av_bsf_alloc(bsf_filter, &fftmp->bsf_context);
            if (ret < 0) {
                xlog_err("av_bsf_alloc failed\n");
                error_flag = true;
                break;
            }

            ret = avcodec_parameters_copy(fftmp->bsf_context->par_in, stream->codecpar);
            if (ret < 0) {
                xlog_err("avcodec_parameters_copy failed\n");
                error_flag = true;
                break;
            }

            fftmp->bsf_context->time_base_in = stream->time_base;
            ret = av_bsf_init(fftmp->bsf_context);
            if (ret < 0) {
                xlog_err("av_bsf_init failed\n");
                error_flag = true;
                break;
            }

            // bsf success
        }

        // copy results
        
        xlog_dbg("open input successful, v.index=%d\n", fftmp->index_video);
    } while (false);

    if (error_flag) {
        closeImp(fftmp);
        fftmp = nullptr;
    }

    return fftmp;
}

void MyFFmpeg::closeImp(FFmpegCtxPtr ffctx)
{
    do {
        if (nullptr == ffctx) {
            break;
        }

        if (ffctx->context != nullptr) {
            avformat_close_input(&ffctx->context);
            ffctx->context = nullptr;
        }

        if (ffctx->bsf_context != nullptr) {
            av_bsf_free(&ffctx->bsf_context);
            ffctx->bsf_context = nullptr;
        }
    } while (false);

    // return 
}

int MyFFmpeg::popPacketOnceWithBSF(FFmpegCtxPtr ffctx, std::vector<uint8_t> &buf)
{
    AVPacket *pkt = nullptr;

    buf.resize(0);

    int ret = 0;
    do {

        pkt = av_packet_alloc();
        if (pkt == nullptr) {
            xlog_err("av_packet_alloc failed\n");
            ret = AVERROR(ENOMEM);
            break;
        }

        // 先尝试从bsf中取（av_bsf_send_packet要求必须先取完再塞）
        ret = av_bsf_receive_packet(ffctx->bsf_context, pkt);
        if (ret < 0) {
            if (ret == AVERROR(EAGAIN)) {
                // 
            } else if (ret == AVERROR_EOF) {
                ret = AVERROR(EAGAIN); // 第一次调用会报eof
            } else {
                xlog_err("av_bsf_receive_packet failed: %#x\n", ret);
                break;
            }
        } else {
            if (pkt->size <= MAX_PKT_SIZE) {
                buf.resize(pkt->size);
                memcpy(buf.data(), pkt->data, pkt->size);
                ret = pkt->size;
            } else {
                xlog_err("pkt too big: %d\n", pkt->size);
                ret = AVERROR(E2BIG);
            }
            break;
        }

        // 没取到，则尝试塞流

        bool got_frame = false;
        ret = av_read_frame(ffctx->context, pkt);
        if (ret < 0) {
            if (AVERROR_EOF == ret) {
                xlog_dbg("got eof\n"); 
                break; // bsf取不到，且文件为空，则停止
            } else {
                xlog_err("av_read_frame failed: %#x\n", ret);
                break;
            }
        } else {
            if (pkt->stream_index == ffctx->index_video) {
                got_frame = true;
            }
        }

        if (!got_frame) {
            ret = AVERROR(EAGAIN);
            break;
        }

        ret = av_bsf_send_packet(ffctx->bsf_context, pkt);
        if (ret < 0) {
            if (ret == AVERROR(EAGAIN)) {
                xlog_dbg("inner error: bsf is full\n");
            } else {
                xlog_err("av_bsf_send_packet failed: %#x\n", ret);
                break;
            }
        } else {
            // suc
            ret = AVERROR(EAGAIN);
        }
    } while (false);

    if (pkt != nullptr) {
        av_packet_free(&pkt);
        pkt = nullptr;
    }

    return ret;
}

int MyFFmpeg::popPacketOnceWithNoBSF(FFmpegCtxPtr ffctx, std::vector<uint8_t> &buf)
{
    AVPacket *pkt = nullptr;
    buf.resize(0);
    int ret = 0;

    do {
        pkt = av_packet_alloc();
        if (pkt == nullptr) {
            xlog_err("av_packet_alloc failed\n");
            ret = AVERROR(ENOMEM);
            break;
        }

        bool got_frame = false;
        ret = av_read_frame(ffctx->context, pkt);
        if (ret < 0) {
            if (AVERROR_EOF == ret) {
                xlog_dbg("got eof\n"); 
                break; 
            }
            
            xlog_err("av_read_frame failed: %#x\n", ret);
            break;
        }

        if (pkt->stream_index == ffctx->index_video) {
            got_frame = true;
        }

        if (!got_frame) {
            ret = AVERROR(EAGAIN);
            break;
        }

        buf.resize(pkt->size);
        memcpy(buf.data(), pkt->data, pkt->size);
        ret = pkt->size;
    } while (false);
    
    if (pkt != nullptr) {
        av_packet_free(&pkt);
        pkt = nullptr;
    }

    return ret;
}

int MyFFmpeg::popPacketOnce(FFmpegCtxPtr ffctx, std::vector<uint8_t> &buf)
{
    int ret = 0;
    if (ffctx->bsf_context != nullptr) {
        ret = popPacketOnceWithBSF(ffctx, buf);
    } else {
        ret = popPacketOnceWithNoBSF(ffctx, buf);
    }
    return ret;
}

std::string MyFFmpeg::fourcc2str(fourcc fourCC)
{
    char buf[AV_FOURCC_MAX_STRING_SIZE] = {};
    av_fourcc_make_string(buf, fourCC);
    return std::string(buf);
}

MyFFmpeg::fourcc MyFFmpeg::str2fourcc(std::string fourcc)
{
    if (fourcc.size() >= 4) {
        return MKTAG(fourcc.at(0), fourcc.at(1), fourcc.at(2), fourcc.at(3));
    } else {
        return 0;
    }
}

AVCodecID MyFFmpeg::fourcc2codecid(fourcc fourcc)
{
    AVCodecID codecid = AV_CODEC_ID_NONE;
    const struct AVCodecTag *table[] = { 
        avformat_get_riff_video_tags(), 
        avformat_get_mov_video_tags(),
        0 
    };
    codecid = av_codec_get_id(table, fourcc);
    return codecid;
}

MyFFmpeg::fourcc MyFFmpeg::codecid2fourcc(AVCodecID codecid)
{
    fourcc result = 0;
    do {
            const struct AVCodecTag *table[] = { 
                avformat_get_riff_video_tags(), 
                avformat_get_mov_video_tags(),
                0 
            };
            if (0 != av_codec_get_tag2(table, codecid, &result)) {
                break;
            }
    } while (false);
    return result;
}

std::string MyFFmpeg::dumpInfo(std::shared_ptr<MyFFmpegInfo> info)
{
    char buf[256] = {};
    FILE *fp = nullptr;

    std::string result;
    do {
        fp = fmemopen(buf, sizeof(buf), "w");
        if (nullptr == fp) {
            xlog_err("fmemopen failed\n");
            break;
        }
        fprintf(fp, "[%d/%d | %s | %s | %s]", 
            info->width, info->height,
            avcodec_get_name(info->codec),
            info->majorBrand.c_str(),
            info->format.c_str());
        fflush(fp);
        result = buf;
    } while (false);

    if (fp != nullptr) {
        fclose(fp);
    }
    
    return result;
}