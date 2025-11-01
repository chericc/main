#include "live_framed_source.hpp"

#include <sys/time.h>

#include "xdemuxer.hpp"
#include "xlog.h"

// namespace {
//     const char *h264file = "~/test.mkv";
// };

struct LiveFramedSource::Ctx {
    std::shared_ptr<XDemuxer> myff = nullptr;
};

LiveFramedSource *LiveFramedSource::createNew(UsageEnvironment& env)
{
    return new LiveFramedSource(env);
}

LiveFramedSource::LiveFramedSource(UsageEnvironment &env) : FramedSource(env)
{
    _ctx = std::make_shared<Ctx>();
    _ctx->myff = std::make_shared<XDemuxer>("/home/test/test.mkv");
    _ctx->myff->open();
}

void LiveFramedSource::doGetNextFrame()
{
    xlog_dbg("get next frame\n");

    auto frame = std::make_shared<XDemuxerFrame>();
    if (_ctx->myff->popPacket(frame)) {
        fFrameSize = frame->buf.size();
        if (frame->buf.size() <= fMaxSize) {
            fNumTruncatedBytes = 0;
        } else {
            fNumTruncatedBytes = fFrameSize - fMaxSize;
        }

        fPresentationTime.tv_sec = frame->pts / 1000;
        fPresentationTime.tv_usec = (frame->pts % 1000) * 1000;
        memcpy(fTo, frame->buf.data(), frame->buf.size());

        LiveFramedSource::afterGetting(this);
    } else {
        xlog_err("pop failed\n");
    }
    

    // fPresentationTime.tv_sec
}

void LiveFramedSource::doStopGettingFrames()
{
    xlog_dbg("stop getting frame\n");
}