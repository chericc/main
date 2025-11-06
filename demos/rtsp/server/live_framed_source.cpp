#include "live_framed_source.hpp"

#include <sys/time.h>

#include "xdemuxer.hpp"
#include "xlog.h"

// namespace {
//     const char *h264file = "~/test.mkv";
// };

struct LiveFramedSource::Ctx {
    std::shared_ptr<XDemuxer> myff = nullptr;

    std::vector<uint8_t> buf;
};

LiveFramedSource *LiveFramedSource::createNew(UsageEnvironment& env)
{
    return new LiveFramedSource(env);
}

LiveFramedSource::LiveFramedSource(UsageEnvironment &env) : FramedSource(env)
{
    _ctx = std::make_shared<Ctx>();
    _ctx->myff = std::make_shared<XDemuxer>("/home/test/test.h264");
    _ctx->myff->open();
}

void LiveFramedSource::doGetNextFrame()
{
    xlog_dbg("get next frame\n");

    if (!_ctx->buf.empty()) {
        xlog_dbg("user buf: %zu\n", _ctx->buf.size());
        if (_ctx->buf.size() <= fMaxSize) {
            memcpy(fTo, _ctx->buf.data(), _ctx->buf.size());
            fFrameSize = _ctx->buf.size();
            _ctx->buf.clear();
        } else {
            memcpy(fTo, _ctx->buf.data(), fMaxSize);
            memcpy(_ctx->buf.data(), _ctx->buf.data() + fMaxSize, _ctx->buf.size() - fMaxSize);
            _ctx->buf.resize(_ctx->buf.size() - fMaxSize);
            fFrameSize = fMaxSize;
        }
        fNumTruncatedBytes = 0;
        xlog_dbg("frameSize=%d\n", fFrameSize);
        // To avoid possible infinite recursion, we need to return to the event loop to do this:
        nextTask() = envir().taskScheduler().scheduleDelayedTask(0,
                        (TaskFunc*)FramedSource::afterGetting, this);
    } else {
        auto frame = std::make_shared<XDemuxerFrame>();
        if (_ctx->myff->popPacket(frame)) {
            xlog_dbg("pop packet: %zu\n", frame->buf.size());
            if (frame->buf.size() <= fMaxSize) {
                memcpy(fTo, frame->buf.data(), frame->buf.size());
                fFrameSize = frame->buf.size();
            } else {
                memcpy(fTo, frame->buf.data(), fMaxSize);
                _ctx->buf.resize(frame->buf.size() - fMaxSize);
                memcpy(_ctx->buf.data(), frame->buf.data() + fMaxSize, frame->buf.size() - fMaxSize);
                fFrameSize = fMaxSize;
            }
            fNumTruncatedBytes = 0;
            // To avoid possible infinite recursion, we need to return to the event loop to do this:
            nextTask() = envir().taskScheduler().scheduleDelayedTask(0,
                            (TaskFunc*)FramedSource::afterGetting, this);
        } else {
            xlog_dbg("pop eof\n");
            handleClosure();
        }
    }
}

void LiveFramedSource::doStopGettingFrames()
{
    xlog_dbg("stop getting frame\n");
    envir().taskScheduler().unscheduleDelayedTask(nextTask());
}