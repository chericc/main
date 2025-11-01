#include "live_framed_source.hpp"

#include <sys/time.h>

#include "myffmpeg.hpp"
#include "xlog.h"

// namespace {
//     const char *h264file = "~/test.mkv";
// };

struct LiveFramedSource::Ctx {
    std::shared_ptr<MyFFmpeg> myff = nullptr;
};

LiveFramedSource *LiveFramedSource::createNew(UsageEnvironment& env)
{
    return new LiveFramedSource(env);
}

LiveFramedSource::LiveFramedSource(UsageEnvironment &env) : FramedSource(env)
{
    _ctx = std::make_shared<Ctx>();
    _ctx->myff = std::make_shared<MyFFmpeg>("/home/test/test.mkv");
    _ctx->myff->open();
}

void LiveFramedSource::doGetNextFrame()
{
    xlog_dbg("get next frame\n");

    std::vector<uint8_t> buf;
    if (_ctx->myff->popPacket(buf)) {
        if (buf.size() <= fMaxSize) {
            memcpy(fTo, buf.data(), buf.size());

            gettimeofday(&fPresentationTime, nullptr);

            LiveFramedSource::afterGetting(this);
        } else {
            xlog_err("packet size over(%zu > %zu)\n", buf.size(), (size_t)fMaxSize);
        }
    } else {
        xlog_err("pop failed\n");
    }
    

    // fPresentationTime.tv_sec
}

void LiveFramedSource::doStopGettingFrames()
{
    xlog_dbg("stop getting frame\n");
}