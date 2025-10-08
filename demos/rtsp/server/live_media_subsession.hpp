#pragma once

#include "OnDemandServerMediaSubsession.hh"

class LiveMediaSubsession : public OnDemandServerMediaSubsession
{
public:
    static LiveMediaSubsession* createNew(UsageEnvironment& env);
protected:
    LiveMediaSubsession(UsageEnvironment& env);
    ~LiveMediaSubsession() override;

    // redefined virtual funcs
    const char *getAuxSDPLine(RTPSink *sink, FramedSource *source) override;
    FramedSource *createNewStreamSource(unsigned clientSessionId, 
        unsigned &estBitrate) override;
    RTPSink* createNewRTPSink(Groupsock* rtpGroupsock,
                                        unsigned char rtpPayloadTypeIfDynamic,
                        FramedSource* inputSource) override;
};