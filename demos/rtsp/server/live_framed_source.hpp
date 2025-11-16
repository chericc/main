#pragma once

#include <memory>
#include <functional>

#include "liveMedia/FramedSource.hh"
#include "live_stream_provider.hpp"

// refer to ByteStreamFileSource.hh

using popBufCb = std::function<bool(size_t size, std::vector<uint8_t> &buf)>;

class LiveFramedSource : public FramedSource
{
public:
    static LiveFramedSource *createNew(UsageEnvironment& env, popBufCb cb);

    LiveFramedSource(UsageEnvironment &env, popBufCb cb);

    void doGetNextFrame() override;
    void doStopGettingFrames() override;

private:
    struct Ctx;
    std::shared_ptr<Ctx> _ctx = nullptr;
};