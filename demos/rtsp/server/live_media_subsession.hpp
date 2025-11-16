#pragma once

#include <string>
#include <memory>
#include <functional>
#include "OnDemandServerMediaSubsession.hh"

// refer to H264VideoFileServerMediaSubsession.hpp

struct LiveMediaSubsessionProfile {
    std::function<FramedSource*()> genSource;
    std::function<RTPSink*(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic)> genRTPSink;
};

class LiveMediaSubsession : public OnDemandServerMediaSubsession
{
public:
    static LiveMediaSubsession* createNew(UsageEnvironment& env, LiveMediaSubsessionProfile const &profile);
protected:
    LiveMediaSubsession(UsageEnvironment& env, LiveMediaSubsessionProfile const &profile);
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

    std::shared_ptr<LiveMediaSubsessionProfile> _profile;
};