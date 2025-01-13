#pragma once

#include <BasicUsageEnvironment.hh>
#include <GroupsockHelper.hh>
#include <RTSPServer.hh>

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
};