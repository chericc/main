#pragma once

#include <string>
#include <memory>
#include "OnDemandServerMediaSubsession.hh"

// refer to H264VideoFileServerMediaSubsession.hpp

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

    static void afterPlaying(void *clientData);
    void afterPlaying();
    static void checkForAuxSDPLine(void *clientData);
    void checkForAuxSDPLine();

    void setDoneFlag() { _fDoneFlag = ~0; }
private:
    RTPSink *_sink = nullptr;
    std::shared_ptr<std::string> _sdp = nullptr;
    EventLoopWatchVariable _fDoneFlag = 0; // used when setting up "fAuxSDPLine"
};