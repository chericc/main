#pragma once

#include <memory>
#include <functional>

#include "liveMedia/FramedSource.hh"
#include "live_stream_provider.hpp"

// refer to ByteStreamFileSource.hh



class LiveFramedSource : public FramedSource
{
public:
    using PopBufCb = std::function<bool(size_t size, std::vector<uint8_t> &buf)>;
    using ForceIFrame = std::function<bool()>;
    struct Profile {
        PopBufCb popBufCb;
        ForceIFrame forceIFrame;
    };

    static LiveFramedSource *createNew(UsageEnvironment& env, Profile profile);

    LiveFramedSource(UsageEnvironment &env, Profile profile);

    void doGetNextFrame() override;
    void doStopGettingFrames() override;

private:
    struct Ctx;
    std::shared_ptr<Ctx> _ctx = nullptr;
};