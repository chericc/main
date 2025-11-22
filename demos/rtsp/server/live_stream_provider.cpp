#include "live_stream_provider.hpp"

#include <string>
#include <memory>

#include "xdemuxer.hpp"
#include "xlog.h"

struct LiveStreamProviderFile::Ctx {
    std::string file;

    std::shared_ptr<XDemuxer> demuxer = nullptr;

    std::list<std::vector<uint8_t>> pktListVideo;
    std::list<std::vector<uint8_t>> pktListAudio;
};

LiveStreamProviderFile::LiveStreamProviderFile(std::string file)
{
    _ctx = std::make_shared<Ctx>();

    _ctx->file = std::move(file);

    auto demuxer = std::make_shared<XDemuxer>(_ctx->file);
    if (demuxer->open()) {
        _ctx->demuxer = demuxer;
    } else {
        xlog_err("open failed\n");
    }
}

LiveStreamProviderFile::~LiveStreamProviderFile()
{

}

bool LiveStreamProviderFile::info(Info &info)
{
    bool okFlag = false;
    do {
        if (nullptr == _ctx->demuxer) {
            xlog_err("not opened\n");
            break;
        }

        auto infoPtr = _ctx->demuxer->getInfo();
        if (!infoPtr) {
            xlog_err("get info failed\n");
            break;
        }

        if (infoPtr->codecA == AV_CODEC_ID_AAC) {
            info.codecA = AAC;
        }

        if (infoPtr->codecV == AV_CODEC_ID_HEVC) {
            info.codecV = HEVC;
        } else if (infoPtr->codecV == AV_CODEC_ID_H264) {
            info.codecV = H264;
        }

        info.hasVStream = (infoPtr->codecV != AV_CODEC_ID_NONE);
        info.hasAStream = (infoPtr->codecA != AV_CODEC_ID_NONE);
        okFlag = true;
    } while (false);
    return okFlag;
}

bool LiveStreamProviderFile::forceIFrame()
{
    bool okFlag = false;
    do {
        if (nullptr == _ctx->demuxer) {
            xlog_err("not opened");
            break;
        }

        okFlag = _ctx->demuxer->forceIFrame();
    } while (false);
    return okFlag;
}

bool LiveStreamProviderFile::popVBuf(size_t size, std::vector<uint8_t> &buf)
{
    auto ret = popBuf(size, buf, _ctx->pktListVideo);
    return ret;
}

bool LiveStreamProviderFile::popABuf(size_t size, std::vector<uint8_t> &buf)
{
    auto ret = popBuf(size, buf, _ctx->pktListAudio);
    return ret;
}

bool LiveStreamProviderFile::popBuf(size_t size, std::vector<uint8_t> &buf, std::list<std::vector<uint8_t>> &pktList)
{
    bool okFlag = false;
    do {
        if (nullptr == _ctx->demuxer) {
            xlog_err("not opened\n");
            break;
        }

        while (true) {
            if (!_ctx->pktListVideo.empty()) {
                auto & ref = _ctx->pktListVideo.front();
                if (ref.size() > size) {
                    buf.resize(size);
                    memcpy(buf.data(), ref.data(), size);
                    memmove(ref.data(), ref.data() + size, ref.size() - size);
                } else {
                    buf.resize(ref.size());
                    memcpy(buf.data(), ref.data(), ref.size());
                    _ctx->pktListVideo.pop_front();
                }

                xlog_dbg("size=%zu, pop=%zu\n", size, buf.size());

                okFlag = true;
                break;
            }

            if (!tryPopBuf()) {
                xlog_err("pop buf failed\n");
                break;
            }
        }
    } while (false);
    return okFlag;
}

bool LiveStreamProviderFile::tryPopBuf()
{
    bool okFlag = false;
    do {
        XDemuxerFramePtr frame = std::make_shared<XDemuxerFrame>();

        _ctx->demuxer->popPacket(frame);

        if (frame->isVideo) {
            if (_ctx->pktListVideo.size() > MAX_BUF_CACHE_NUM) {
                // break;
            }

            _ctx->pktListVideo.push_back(std::move(frame->buf));
        } else {
            if (_ctx->pktListAudio.size() > MAX_BUF_CACHE_NUM) {
                // break;
            }

            _ctx->pktListAudio.push_back(std::move(frame->buf));
        }
        okFlag = true;
    } while (false);
    return okFlag;
}