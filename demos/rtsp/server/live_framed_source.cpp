#include "live_framed_source.hpp"

#include <sys/time.h>

#include "live_stream_provider.hpp"
#include "xlog.h"

// namespace {
//     const char *h264file = "~/test.mkv";
// };

struct LiveFramedSource::Ctx {
    popBufCb getPacketCb = nullptr;
};

LiveFramedSource *LiveFramedSource::createNew(UsageEnvironment& env, popBufCb cb)
{
    return new LiveFramedSource(env, cb);
}

LiveFramedSource::LiveFramedSource(UsageEnvironment &env, popBufCb cb) : FramedSource(env)
{
    _ctx = std::make_shared<Ctx>();
    _ctx->getPacketCb = cb;
}

void LiveFramedSource::doGetNextFrame()
{
    xlog_dbg("get next frame\n");

    std::vector<uint8_t> buf;
    if (!_ctx->getPacketCb(fMaxSize, buf)) {
        xlog_err("pop end\n");
        handleClosure();
    }

    memcpy(fTo, buf.data(), buf.size());
    fFrameSize = buf.size();
    fNumTruncatedBytes = 0;

    // To avoid possible infinite recursion, we need to return to the event loop to do this:
    nextTask() = envir().taskScheduler().scheduleDelayedTask(0,
                (TaskFunc*)FramedSource::afterGetting, this);
}

void LiveFramedSource::doStopGettingFrames()
{
    xlog_dbg("stop getting frame\n");
    envir().taskScheduler().unscheduleDelayedTask(nextTask());
}