#pragma once

#include "FramedSource.hh"

// refer to ByteStreamFileSource.hh

class LiveFramedSource : public FramedSource
{
public:
    static LiveFramedSource *createNew(UsageEnvironment& env);
    void doGetNextFrame() override;
    void doStopGettingFrames() override;
};