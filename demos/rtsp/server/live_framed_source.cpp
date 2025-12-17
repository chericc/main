#include "live_framed_source.hpp"

#include <sys/time.h>

#include "live_stream_provider.hpp"
#include "xlog.h"

// namespace {
//     const char *h264file = "~/test.mkv";
// };

struct LiveFramedSource::Ctx {
    Profile profile;
};

LiveFramedSource *LiveFramedSource::createNew(UsageEnvironment& env, Profile profile)
{
    return new LiveFramedSource(env, std::move(profile));
}

LiveFramedSource::LiveFramedSource(UsageEnvironment &env, Profile profile) : FramedSource(env)
{
    _ctx = std::make_shared<Ctx>();
    _ctx->profile = std::move(profile);

    xlog_dbg("force I frame");
    _ctx->profile.forceIFrame();
}

void LiveFramedSource::doGetNextFrame()
{
    xlog_dbg("get next frame");

    std::vector<uint8_t> buf;
    if (!_ctx->profile.popBufCb(fMaxSize, buf)) {
        xlog_err("pop end");
        handleClosure();
    }

    xlog_dbg("pkt: size={}", buf.size());

    memcpy(fTo, buf.data(), buf.size());
    fFrameSize = buf.size();
    fNumTruncatedBytes = 0;

    // To avoid possible infinite recursion, we need to return to the event loop to do this:
    nextTask() = envir().taskScheduler().scheduleDelayedTask(0,
                (TaskFunc*)FramedSource::afterGetting, this);
}

void LiveFramedSource::doStopGettingFrames()
{
    xlog_dbg("stop getting frame");
    envir().taskScheduler().unscheduleDelayedTask(nextTask());
}