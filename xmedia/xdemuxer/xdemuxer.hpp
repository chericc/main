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
    int width;  
    int height;
    AVCodecID codec;
    std::string majorBrand; // heic
    std::string format; // mp4,...
};
using MyFFmpegInfoPtr = std::shared_ptr<XDemuxerInfo>;

struct XDemuxerFrame {
    std::vector<uint8_t> buf;
    uint64_t pts;
};
using XDemuxerFramePtr = std::shared_ptr<XDemuxerFrame>;

class XDemuxer {
public:
    enum {
        MAX_PKT_SIZE = 1024 * 1024,
    };

    explicit XDemuxer(std::string const&file);
    ~XDemuxer();

    bool open();
    MyFFmpegInfoPtr getInfo();
    bool popPacket(XDemuxerFramePtr &framePtr);

    using fourcc = uint32_t;
    static std::string fourcc2str(fourcc);
    static fourcc str2fourcc(std::string);
    static AVCodecID fourcc2codecid(fourcc);
    static fourcc codecid2fourcc(AVCodecID);

    static std::string dumpInfo(MyFFmpegInfoPtr info);
private:
    struct FFmpegCtx {
        AVFormatContext *context = nullptr;
        int index_video = 0;

        AVBSFContext *bsf_context = nullptr;
    };
    using FFmpegCtxPtr = std::shared_ptr<FFmpegCtx>;

    static FFmpegCtxPtr openImp(const char *file);
    static void closeImp(FFmpegCtxPtr ctx);
    static int popPacketOnce(FFmpegCtxPtr ffctx, XDemuxerFramePtr &framePtr);
    static int popPacketOnceWithBSF(FFmpegCtxPtr ffctx, XDemuxerFramePtr &framePtr);
    static int popPacketOnceWithNoBSF(FFmpegCtxPtr ffctx, XDemuxerFramePtr &framePtr);

    std::string file_;
    FFmpegCtxPtr ffctx_;
};