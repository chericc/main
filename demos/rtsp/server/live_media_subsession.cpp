#include "live_media_subsession.hpp"

#include "H264VideoRTPSink.hh"
#include "live_framed_source.hpp"
#include "H264VideoStreamFramer.hh"
#include "H265VideoStreamFramer.hh"

#include "xlog.h"

LiveMediaSubsession *LiveMediaSubsession::createNew(UsageEnvironment &env, LiveMediaSubsessionProfile const &profile)
{
    return new LiveMediaSubsession(env, profile);
}

LiveMediaSubsession::LiveMediaSubsession(UsageEnvironment &env, LiveMediaSubsessionProfile const &profile)
    : OnDemandServerMediaSubsession(env, True)
{
    _profile = std::make_shared<LiveMediaSubsessionProfile>();
    *_profile = profile;
}

LiveMediaSubsession::~LiveMediaSubsession()
{

}

void LiveMediaSubsession::afterPlaying(void *clientData)
{
    auto *sess = reinterpret_cast<LiveMediaSubsession*>(clientData);
    sess->afterPlaying();
}

void LiveMediaSubsession::afterPlaying()
{
    envir().taskScheduler().unscheduleDelayedTask(nextTask());
    setDoneFlag();
}

void LiveMediaSubsession::checkForAuxSDPLine(void *clientData)
{
    auto *sess = reinterpret_cast<LiveMediaSubsession*>(clientData);
    sess->checkForAuxSDPLine();
}

void LiveMediaSubsession::checkForAuxSDPLine()
{
    nextTask() = nullptr;

    const char *dasl = nullptr;
    if (_sdp != nullptr) {
        setDoneFlag();
    } else if ((_sink != nullptr) && ((dasl = _sink->auxSDPLine()) != nullptr)) {
        _sdp = std::make_shared<std::string>(dasl);
        _sink = nullptr;
        setDoneFlag();
    } else if (0 == _fDoneFlag) {
        int usecs_delay = 100000; // 100 ms
        nextTask() = envir().taskScheduler().scheduleDelayedTask(usecs_delay, checkForAuxSDPLine, this);
    }
}

const char *LiveMediaSubsession::getAuxSDPLine(RTPSink *sink, FramedSource *source)
{
    if (_sdp != nullptr) {
        return _sdp->c_str();
    }

    if (_sink == nullptr) {
        _sink = sink;
        _sink->startPlaying(*source, afterPlaying, this);
        
        checkForAuxSDPLine();
    }

    envir().taskScheduler().doEventLoop(&_fDoneFlag);
    return _sdp->c_str();
}

FramedSource *LiveMediaSubsession::createNewStreamSource(unsigned clientSessionId, 
    unsigned &estBitrate)
{
        estBitrate = 500;

        return  _profile->genSource();
}

RTPSink* LiveMediaSubsession::createNewRTPSink(Groupsock* rtpGroupsock,
                                    unsigned char rtpPayloadTypeIfDynamic,
                    FramedSource* inputSource)
{
    return _profile->genRTPSink(rtpGroupsock, rtpPayloadTypeIfDynamic);
};