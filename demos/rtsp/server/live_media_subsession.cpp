#include "live_media_subsession.hpp"

#include "H264VideoRTPSink.hh"

LiveMediaSubsession *LiveMediaSubsession::createNew(UsageEnvironment &env)
{
    return new LiveMediaSubsession(env);
}

LiveMediaSubsession::LiveMediaSubsession(UsageEnvironment &env)
    : OnDemandServerMediaSubsession(env, True)
{

}

LiveMediaSubsession::~LiveMediaSubsession()
{

}

const char *LiveMediaSubsession::getAuxSDPLine(RTPSink *sink, FramedSource *source)
{

}

FramedSource *LiveMediaSubsession::createNewStreamSource(unsigned clientSessionId, 
    unsigned &estBitrate)
{
    estBitrate = 500;
    
}

RTPSink* LiveMediaSubsession::createNewRTPSink(Groupsock* rtpGroupsock,
                                    unsigned char rtpPayloadTypeIfDynamic,
                    FramedSource* inputSource)
{
    return H264VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
}