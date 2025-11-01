#pragma once

#include <memory>
#include "liveMedia/FramedSource.hh"

// refer to ByteStreamFileSource.hh

class LiveFramedSource : public FramedSource
{
public:
    static LiveFramedSource *createNew(UsageEnvironment& env);

    LiveFramedSource(UsageEnvironment &env);

    void doGetNextFrame() override;
    void doStopGettingFrames() override;

private:
    struct Ctx;
    std::shared_ptr<Ctx> _ctx = nullptr;
};