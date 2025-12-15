#include "xdemuxer.hpp"

#include <array>
#include <thread>

#include "aac.hpp"

#include "xlog.h"

struct XDemuxer::FFmpegCtx {
    AVFormatContext *context = nullptr;
    int index_video = 0;
    int index_audio = 0;

    AVBSFContext *bsf_context_v = nullptr;
};

struct XDemuxer::Ctx {
    std::string file;
    FFmpegCtxPtr ffctx;
};

namespace {

}

XDemuxer::XDemuxer(std::string file)
{
    av_log_set_level(AV_LOG_ERROR);
    // av_log_set_level(AV_LOG_TRACE);

    xlog_dbg("file: {}\n", file.c_str());

    _ctx = std::make_shared<Ctx>();
    _ctx->file = std::move(file);
    _ctx->ffctx = nullptr;
}

XDemuxer::~XDemuxer()
{
    if (_ctx != nullptr) {
        if (_ctx->ffctx != nullptr) {
            closeImp(_ctx->ffctx);
            _ctx->ffctx = nullptr;
        }
    }
    // end
}

bool XDemuxer::open()
{
    bool error_flag = false;
    do {
        if (_ctx->ffctx) {
            xlog_err("not null\n");
            break;
        }


        _ctx->ffctx = openImp(_ctx->file);;
        if (!_ctx->ffctx) {
            error_flag = true;
            xlog_err("open failed\n");
            break;
        }
    } while (false);

    return !error_flag;
}

std::shared_ptr<XDemuxerInfo> XDemuxer::getInfo()
{
    bool error_flag = false;
    auto info = std::make_shared<XDemuxerInfo>();

    do {
        if (!_ctx->ffctx) {
            error_flag = true;
            xlog_err("null\n");
            break;
        }

        if (_ctx->ffctx->index_video >= 0) {
            if (static_cast<unsigned int>(_ctx->ffctx->index_video) >= _ctx->ffctx->context->nb_streams) {
                xlog_err("invalid index: {}({})\n", _ctx->ffctx->index_video, _ctx->ffctx->context->nb_streams);
                error_flag = true;
                break;
            }

            AVStream *streamV = _ctx->ffctx->context->streams[_ctx->ffctx->index_video];

            if (nullptr == streamV->codecpar) {
                xlog_err("inner error: codecpar is null\n");
                error_flag = true;
                break;
            }

            info->codecV = streamV->codecpar->codec_id;
            info->widthV = streamV->codecpar->width;
            info->heightV = streamV->codecpar->height;

            if (_ctx->ffctx->context->metadata != nullptr) {
                const AVDictionaryEntry *tag = nullptr;
                while (true) {
                    tag = av_dict_iterate(_ctx->ffctx->context->metadata, tag);
                    if (tag == nullptr) {
                        break;
                    }
                    if (strcmp(tag->key, "major_brand") == 0) {
                        info->majorBrandV = tag->value;
                    }
                }
            }
            
            if (_ctx->ffctx->context->iformat != nullptr) {
                if (_ctx->ffctx->context->iformat->name != nullptr) {
                    info->formatV = _ctx->ffctx->context->iformat->name;
                }
            }
        }

        if (_ctx->ffctx->index_audio >= 0) {
            if (static_cast<unsigned int>(_ctx->ffctx->index_audio) >= _ctx->ffctx->context->nb_streams) {
                xlog_err("invalid index: {}({})\n", _ctx->ffctx->index_audio, _ctx->ffctx->context->nb_streams);
                error_flag = true;
                break;
            }

            AVStream *streamA = _ctx->ffctx->context->streams[_ctx->ffctx->index_audio];
            if (nullptr == streamA->codecpar) {
                xlog_err("codecpar is null\n");
                error_flag = true;
                break;
            }

            info->codecA = streamA->codecpar->codec_id;
        }

    } while (false);

    if (error_flag) {
        info = nullptr;
    }

    return info;
}

bool XDemuxer::popPacket(XDemuxerFramePtr &framePtr)
{
    bool error_flag = false;
    bool eof = false;

    do {
        if (nullptr == _ctx->ffctx) {
            xlog_err("not opened yet\n");
            error_flag = true;
            break;
        }

        while (true) {
            int ret = popPacketOnce(_ctx->ffctx, framePtr);
            if (ret >= 0) {
                if (framePtr->isVideo) {
                    xlog_dbg("pop frame: size={}, video={}\n", framePtr->buf.size(), framePtr->isVideo);
                }
                
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

bool XDemuxer::forceIFrame()
{
    bool okFlag = false;

    xlog_dbg("force I Frame");

    do {
        int ret = 0;
        int64_t dts = 0;

        if (!_ctx->ffctx) {
            xlog_err("null");
            break;
        }

        ret = av_seek_frame(_ctx->ffctx->context, _ctx->ffctx->index_video, dts, 0);
        if (ret < 0) {
            xlog_err("av_seek_frame failed");
            break;
        }

        xlog_dbg("seek ok\n");
        okFlag = true;
    } while (false);
    return okFlag;
}

bool XDemuxer::getInfo(AVCodecParameters &param, bool isVideo)
{
    bool okFlag = false;
    do {
        if (!_ctx->ffctx) {
            xlog_err("not opened yet");
            break;
        }

        int index = 0;
        if (isVideo) {
            index = _ctx->ffctx->index_video;
        } else {
            index = _ctx->ffctx->index_audio;
        }

        auto codecpar = _ctx->ffctx->context->streams[index]->codecpar;
        if (nullptr == codecpar) {
            xlog_err("null codec par");
            break;
        }

        param = *codecpar;
        okFlag = true;
    } while (false);
    return okFlag;
}

XDemuxer::FFmpegCtxPtr XDemuxer::openImp(std::string const& file)
{
    auto fftmp = std::make_shared<FFmpegCtx>();

    bool error_flag = false;
    do {
        int ret = 0;

        ret = avformat_open_input(&fftmp->context, file.c_str(), nullptr, nullptr);
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

        fftmp->index_audio = av_find_best_stream(fftmp->context, AVMEDIA_TYPE_AUDIO, -1, fftmp->index_video, nullptr, 0);

        AVStream *streamV = fftmp->context->streams[fftmp->index_video];

        if (nullptr == streamV->codecpar) {
            xlog_err("inner error: codecpar is null\n");
            error_flag = true;
            break;
        }

        // 两种需要硬件解码的流格式需要考虑包格式兼容性
        if ((streamV->codecpar->codec_id == AV_CODEC_ID_H264)
            || (streamV->codecpar->codec_id == AV_CODEC_ID_HEVC)) {
            auto const* name = ((streamV->codecpar->codec_id == AV_CODEC_ID_H264)
                 ? "h264_mp4toannexb" : "hevc_mp4toannexb");
            auto const* bsf_filter = av_bsf_get_by_name(name);
            if (nullptr == bsf_filter) {
                xlog_err("get bsf_filter failed\n");
                error_flag = true;
                break;
            }

            ret = av_bsf_alloc(bsf_filter, &fftmp->bsf_context_v);
            if (ret < 0) {
                xlog_err("av_bsf_alloc failed\n");
                error_flag = true;
                break;
            }

            ret = avcodec_parameters_copy(fftmp->bsf_context_v->par_in, streamV->codecpar);
            if (ret < 0) {
                xlog_err("avcodec_parameters_copy failed\n");
                error_flag = true;
                break;
            }

            fftmp->bsf_context_v->time_base_in = streamV->time_base;
            ret = av_bsf_init(fftmp->bsf_context_v);
            if (ret < 0) {
                xlog_err("av_bsf_init failed\n");
                error_flag = true;
                break;
            }

            // bsf success
        }

        // copy results
        
        xlog_dbg("open input successful, v.index={}\n", fftmp->index_video);
    } while (false);

    if (error_flag) {
        closeImp(fftmp);
        fftmp = nullptr;
    }

    return fftmp;
}

void XDemuxer::closeImp(FFmpegCtxPtr ffctx)
{
    do {
        if (nullptr == ffctx) {
            break;
        }

        if (ffctx->context != nullptr) {
            avformat_close_input(&ffctx->context);
            ffctx->context = nullptr;
        }

        if (ffctx->bsf_context_v != nullptr) {
            av_bsf_free(&ffctx->bsf_context_v);
            ffctx->bsf_context_v = nullptr;
        }
    } while (false);

    // return 
}

bool XDemuxer::handlePacket(AVPacket *pkt, AVStream *stream, XDemuxerFramePtr &framePtr)
{
    bool okFlag = false;
    do {
        if (pkt->size > MAX_PKT_SIZE) {
            xlog_err("pkt too big: {}\n", pkt->size);
            break;
        }
        
        if (stream->codecpar->codec_id == AV_CODEC_ID_AAC) {
            xlog_dbg("pktsize={}", pkt->size);
            if (!aac_avpacket_to_adts(stream->codecpar, pkt, framePtr->buf)) {
                break;
            }
        } else {
            framePtr->buf.resize(pkt->size);
            memcpy(framePtr->buf.data(), pkt->data, pkt->size);
        }

        if (pkt->pts != AV_NOPTS_VALUE) {
            framePtr->pts = av_rescale_q(pkt->pts, stream->time_base, AVRational{1, 1000}); // ms
        } else {
            framePtr->pts = AV_NOPTS_VALUE;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        okFlag = true;
    } while (false);
    return okFlag;
}

int XDemuxer::popPacketOnce(FFmpegCtxPtr ffctx, XDemuxerFramePtr &framePtr)
{
    int ret = 0;

    AVPacket *pktFilter = nullptr;
    AVPacket *pktRead = nullptr;

    do {
        pktFilter = av_packet_alloc();
        pktRead = av_packet_alloc();
        
        if ((nullptr == pktFilter)
            || (nullptr == pktRead)) {
            xlog_err("alloc packet failed\n");
            ret = AVERROR(ENOMEM);
            break;
        }

        while (true) {

            // filter processing
            if (ffctx->bsf_context_v != nullptr) {

                ret = av_bsf_receive_packet(ffctx->bsf_context_v, pktFilter);
                if (ret < 0) {
                    if (ret == AVERROR(EAGAIN)) {
                        //
                    } else if (ret == AVERROR_EOF) {
                        ret = AVERROR(EAGAIN); // 认为没有取到帧
                    } else {
                        xlog_err("av_bsf_receive_packet failed\n");
                        break;
                    }
                } else {
                    // got pkt from bsf

                    continue;
                
                    AVStream *stream = ffctx->context->streams[ffctx->index_video];
                    if (!handlePacket(pktFilter, stream, framePtr)) {
                        xlog_err("handlePacket failed\n");
                        ret = AVERROR(E2BIG);
                    }
                    framePtr->isVideo = true;
                    ret = 0;
                    break;
                }
            }

            // not got pkt from fiter

            ret = av_read_frame(ffctx->context, pktRead);
            if (ret < 0) {
                if (AVERROR_EOF == ret) {
                    xlog_dbg("got eof\n"); 
                } else {
                    xlog_err("av_read_frame failed: {}\n", ret);
                }
                break; // 读不到，并且bsf也取不到，则认为结束了
            }

            // got pkt here

            if (pktRead->stream_index == ffctx->index_video) {
                ret = av_bsf_send_packet(ffctx->bsf_context_v, pktRead);
                if(ret < 0) {
                    if (ret == AVERROR(EAGAIN)) {
                        xlog_err("bsf if full (should not happen)\n");
                    } else {
                        xlog_err("av_bsf_send_packet failed\n");
                    }
                    break;
                }
                ret = AVERROR(EAGAIN); // pkt sent to the filter. we need call again to get it.
                break;
            }

            // not video pkt here

            if (pktRead->stream_index == ffctx->index_audio) {
                AVStream *stream = ffctx->context->streams[pktRead->stream_index];
                if (!handlePacket(pktRead, stream, framePtr)) {
                    xlog_err("handlePacket failed\n");
                    ret = AVERROR(E2BIG);
                }
                framePtr->isVideo = false;
                ret = 0;
                break;
            }

            // if other pkts, need try again.

            ret = AVERROR(EAGAIN);
        }
    } while (false);

    if (nullptr != pktFilter) {
        av_packet_free(&pktFilter);
        pktFilter = nullptr;
    }
    if (nullptr != pktRead) {
        av_packet_free(&pktRead);
        pktRead = nullptr;
    }

    return ret;
}

std::string XDemuxer::fourcc2str(fourcc fourCC)
{
    char buf[AV_FOURCC_MAX_STRING_SIZE] = {};
    av_fourcc_make_string(buf, fourCC);
    return std::string(buf);
}

XDemuxer::fourcc XDemuxer::str2fourcc(std::string fourcc)
{
    if (fourcc.size() >= 4) {
        return MKTAG(fourcc.at(0), fourcc.at(1), fourcc.at(2), fourcc.at(3));
    } else {
        return 0;
    }
}

AVCodecID XDemuxer::fourcc2codecid(fourcc fourcc)
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

XDemuxer::fourcc XDemuxer::codecid2fourcc(AVCodecID codecid)
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

std::string XDemuxer::dumpInfo(std::shared_ptr<XDemuxerInfo> info)
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
        fprintf(fp, "[%d/%d | %s | %s | %s | %s]", 
            info->widthV, info->heightV,
            avcodec_get_name(info->codecV),
            avcodec_get_name(info->codecA),
            info->majorBrandV.c_str(),
            info->formatV.c_str());
        fflush(fp);
        result = buf;
    } while (false);

    if (fp != nullptr) {
        fclose(fp);
    }
    
    return result;
}