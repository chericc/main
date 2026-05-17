#pragma once

#include <string>
#include <memory>
#include <cstdint>
#include <vector>

extern "C" {
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavcodec/avcodec.h"
#include "libavcodec/bsf.h"
#include "libavutil/log.h"
#include "libavutil/common.h"
}

/*

目的：
给出一个媒体文件，找出这个媒体文件主视频流的相关信息，并将主视频流的包给出来。

*/

// 对应best视频流
// 对于heic，只能反映其中一个块的信息
struct XDemuxerInfo {
    int widthV = 0;
    int heightV = 0;
    AVCodecID codecV = AV_CODEC_ID_NONE;
    AVCodecID codecA = AV_CODEC_ID_NONE;
    std::string majorBrandV; // heic
    std::string formatV; // mp4,...
};
using MyFFmpegInfoPtr = std::shared_ptr<XDemuxerInfo>;

struct XDemuxerFrame {
    std::vector<uint8_t> buf;
    bool isVideo = false;
    uint64_t pts = 0;
};
using XDemuxerFramePtr = std::shared_ptr<XDemuxerFrame>;

class XDemuxer {
public:
    enum {
        MAX_PKT_SIZE = 1024 * 1024,
    };

    explicit XDemuxer(std::string file);
    ~XDemuxer();

    bool open();
    MyFFmpegInfoPtr getInfo();
    bool popPacket(XDemuxerFramePtr &framePtr);
    bool forceIFrame();

    using fourcc = uint32_t;
    static std::string fourcc2str(fourcc);
    static fourcc str2fourcc(std::string);
    static AVCodecID fourcc2codecid(fourcc);
    static fourcc codecid2fourcc(AVCodecID);

    static std::string dumpInfo(MyFFmpegInfoPtr info);
private:
    struct FFmpegCtx;
    using FFmpegCtxPtr = std::shared_ptr<FFmpegCtx>;

    struct Ctx;
    std::shared_ptr<Ctx> _ctx = nullptr;

    static FFmpegCtxPtr openImp(std::string const& file);
    static void closeImp(FFmpegCtxPtr ctx);
    static int popPacketOnce(FFmpegCtxPtr ffctx, XDemuxerFramePtr &framePtr);
    static bool handlePacket(AVPacket *pkt, AVStream *stream, XDemuxerFramePtr &framePtr);
};