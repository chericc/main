#include "live_framed_source.hpp"

#include "xlog.h"

LiveFramedSource *LiveFramedSource::createNew(UsageEnvironment& env)
{
    return new LiveFramedSource(env);
}

LiveFramedSource::LiveFramedSource(UsageEnvironment &env) : FramedSource(env)
{
}

void LiveFramedSource::doGetNextFrame()
{
    xlog_dbg("get next frame\n");
}

void LiveFramedSource::doStopGettingFrames()
{
    xlog_dbg("stop getting frame\n");
}