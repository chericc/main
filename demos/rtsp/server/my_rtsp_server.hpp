#pragma once

#include <BasicUsageEnvironment.hh>
#include <GroupsockHelper.hh>
#include <liveMedia.hh>

class MyRTSPServer : public RTSPServer {
public:
    static MyRTSPServer* createNew(UsageEnvironment &env, Port port,
        UserAuthenticationDatabase *authDB, 
        unsigned int reclamationSeconds = 10);
protected:
    MyRTSPServer(UsageEnvironment &env, int ipv4Socket, int ipv6Socket, Port port,
        UserAuthenticationDatabase *authDB, unsigned int reclamationSeconds);
    ~MyRTSPServer() override;

protected:
    void lookupServerMediaSession(const char *streamName,
        lookupServerMediaSessionCompletionFunc *completionFunc,
        void *completionClientData,
        Boolean isFirstLoopupInSession) override;

    void onVod(const char *path, const char *streamname, 
        lookupServerMediaSessionCompletionFunc *completionFunc,
        void *completionClientData,
        Boolean isFirstLoopupInSession);
    void onLive(const char *path, const char *streamname,
        lookupServerMediaSessionCompletionFunc *completionFunc,
        void *completionClientData,
        Boolean isFirstLoopupInSession);
};