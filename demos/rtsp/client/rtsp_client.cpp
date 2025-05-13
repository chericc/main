#include "rtsp_client.h"

#include <thread>
#include <memory>
#include <string>

#include "RTSPClient.hh"
#include "BasicUsageEnvironment.hh"

#include "xlog.hpp"

#define RTSP_CLIENT_VERVOSITY_LEVEL     0
#define REQUEST_STREAMING_OVER_TCP False
#define DUMMY_SINK_RECEIVE_BUFFER_SIZE 100000

namespace {

class StreamClientState {
public:
    // StreamClientState();
    ~StreamClientState();

    MediaSubsessionIterator *iter = nullptr;
    MediaSession *session = nullptr;
    MediaSubsession *subsession = nullptr;
    TaskToken streamTimerTask = nullptr;
    double duration = 0.0f;


};

StreamClientState::~StreamClientState()
{
    xlog_dbg("delete iter\n");
    delete iter;

    if (session) {
        xlog_dbg("close session\n");
        auto &env = session->envir();
        env.taskScheduler().unscheduleDelayedTask(streamTimerTask);
        Medium::close(session);
        session = nullptr;
    }
}

class MySink : public MediaSink {
public:
    static MySink *createNew(UsageEnvironment &env, 
        MediaSubsession &subsession,
        const char *streamId = nullptr);
private:
    MySink(UsageEnvironment& env, MediaSubsession &subsession, const char *streamId);
    ~MySink() override;

    static void afterGettingFrame(void *clientData, unsigned int frameSize,
        unsigned int numTruncatedBytes, struct timeval presentationTime, unsigned int durationInMs);
    void afterGettingFrame(unsigned int frameSize,
        unsigned int numTruncatedBytes, struct timeval presentationTime, unsigned int durationInMs);
private:
    Boolean continuePlaying() override;

private:
    MediaSubsession &_sub_session;
    std::vector<uint8_t> _recv_buf;
    std::string _streamid;
};

struct RtspClientCtx {
    std::string progName;
    rtsp_client_param param;

    std::shared_ptr<std::thread> trd;
    EventLoopWatchVariable eventLoopWatchVariable;
    RTSPClient *rtsp_client = nullptr;
};

class MyRTSPClient : public RTSPClient
{
public:
    static MyRTSPClient* createNew(UsageEnvironment& env, const char *rtspURL,
        int verbosityLevel = 0, 
        const char *applicationName = nullptr,
        portNumBits tunnelOverHTTPPortNum = 0);
protected:
    MyRTSPClient(UsageEnvironment &env, const char *rtspUrl,
        int verbosityLevel, const char* applicationName, 
        portNumBits tunnelOverHTTPPortNum);
    ~MyRTSPClient() override;
public:
    StreamClientState scs = {};
};

void continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char *resultString);
void setupNextSubsession(RTSPClient *rtspClient);

MyRTSPClient* MyRTSPClient::createNew(UsageEnvironment& env, const char *rtspURL,
    int verbosityLevel, 
    const char *applicationName,
    portNumBits tunnelOverHTTPPortNum)
{
    return new MyRTSPClient(env, rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum);
}

MyRTSPClient::MyRTSPClient(UsageEnvironment& env, char const* rtspURL,
    int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum)
: RTSPClient(env,rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum, -1)
{
}

MyRTSPClient::~MyRTSPClient() 
{
    
}

MySink *MySink::createNew(UsageEnvironment& env, MediaSubsession& subsession, char const* streamId)
{
    return new MySink(env, subsession, streamId);
}

MySink::MySink(UsageEnvironment &env, MediaSubsession &subsession, const char *streamId)
    : MediaSink(env), _sub_session(subsession)
{
    _streamid = std::string(streamId);
    _recv_buf.resize(DUMMY_SINK_RECEIVE_BUFFER_SIZE);
}

MySink::~MySink() 
{
    // 
}

void MySink::afterGettingFrame(void *clientData, unsigned int frameSize,
    unsigned int numTruncatedBytes, struct timeval presentationTime, unsigned int durationInMs)
{
    MySink* sink = (MySink*)clientData;
    sink->afterGettingFrame(frameSize, numTruncatedBytes, presentationTime, durationInMs);

    return ;
}

void MySink::afterGettingFrame(unsigned int frameSize,
    unsigned int numTruncatedBytes, struct timeval presentationTime, unsigned int durationInMs)
{
    xlog_dbg("frame: %d\n", frameSize);

    continuePlaying();
    return ;
}

Boolean MySink::continuePlaying()
{
    if (!fSource) {
        xlog_err("error\n");
        return False;
    }

    fSource->getNextFrame(_recv_buf.data(), _recv_buf.size(), 
        afterGettingFrame, this, onSourceClosure, this);
    
    return True;
}

void shutdownStream(RTSPClient *rtspClient, int exitCode = 1)
{
    // UsageEnvironment &env = rtspClient->envir();
    StreamClientState &scs = ((MyRTSPClient*)rtspClient)->scs;

    if (scs.session) {
        Boolean someSubsessionWereActive = False;
        MediaSubsessionIterator iter(*scs.session);
        MediaSubsession *subsession = nullptr;

        while ((subsession = iter.next()) != nullptr) {
            if (subsession->sink) {
                Medium::close(subsession->sink);
                subsession->sink = nullptr;
                
                if (subsession->rtcpInstance()) {
                    subsession->rtcpInstance()->setByeHandler(nullptr, nullptr);
                }
                someSubsessionWereActive = True;
            }
        }

        if (someSubsessionWereActive) {
            rtspClient->sendTeardownCommand(*scs.session, nullptr);
        }
    }

    xlog_dbg("closing the stream\n");
    Medium::close(rtspClient);

    /// todo
    /// exit eventloop here
}

void subsessionAfterPlaying(void *clientData) 
{
    MediaSubsession *subsession = (MediaSubsession*)clientData;
    RTSPClient *rtspClient = (RTSPClient*)(subsession->miscPtr);

    xlog_dbg("close subsession stream\n");
    Medium::close(subsession->sink);
    subsession->sink = nullptr;

    MediaSession &session = subsession->parentSession();
    MediaSubsessionIterator iter(session);
    while ((subsession = iter.next()) != nullptr) {
        if (subsession->sink) {
            xlog_dbg("check: session not closed\n");
        }
    }

    shutdownStream(rtspClient);

    return ;
}

void subsessionByeHandler(void *clientData, const char *reason) 
{
    MediaSubsession* subsession = (MediaSubsession*)clientData;
    // RTSPClient *rtspClient = (RTSPClient*)subsession->miscPtr;
    // UsageEnvironment &env = rtspClient->envir();

    xlog_dbg("received rtcp 'BYE'\n");
    if (reason) {
        xlog_dbg("reason: %s\n", reason);
        delete [] reason;
    }
    xlog_dbg("on subsession %s\n", subsession->mediumName());
}

void continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char *resultString)
{
    do {
        UsageEnvironment &env = rtspClient->envir();
        StreamClientState &scs = ((MyRTSPClient*)rtspClient)->scs;

        if (resultCode != 0) {
            xlog_err("failed to set up subsession: %s\n", resultString);
            break;
        }

        xlog_dbg("set up subsession\n");
        if (scs.subsession->rtcpIsMuxed()) {
            xlog_dbg("client port: %d\n", scs.subsession->clientPortNum());
        } else {
            xlog_dbg("client ports: %d-%d\n", scs.subsession->clientPortNum(), 
                scs.subsession->clientPortNum() + 1);
        }

        scs.subsession->sink = MySink::createNew(env, *scs.subsession, rtspClient->url());
        if (!scs.subsession->sink) {
            xlog_err("failed to create data sink for subsession\n");
            break;
        }

        xlog_dbg("created a data sink for subsession\n");
        scs.subsession->miscPtr = rtspClient;
        scs.subsession->sink->startPlaying(*(scs.subsession->readSource()),
            subsessionAfterPlaying, scs.subsession);
        if (scs.subsession->rtcpInstance()) {
            scs.subsession->rtcpInstance()->setByeWithReasonHandler(subsessionByeHandler, scs.subsession);
        }
    } while (0);

    delete [] resultString;
    resultString = nullptr;

    setupNextSubsession(rtspClient);
}

void streamTimerHandler(void *clientData)
{
    do {
        MyRTSPClient* rtspClient = (MyRTSPClient*)clientData;
        StreamClientState &scs = ((MyRTSPClient*)rtspClient)->scs;

        scs.streamTimerTask = nullptr;
        shutdownStream(rtspClient);
    } while (0);

    return ;
}

void continueAfterPLAY(RTSPClient* rtspClient, int resultCode, char* resultString)
{
    Boolean success = False;
    do {
        UsageEnvironment &env = rtspClient->envir();
        StreamClientState &scs = ((MyRTSPClient*)rtspClient)->scs;

        if (resultCode) {
            xlog_err("failed to start playing session: %s\n", resultString);
            break;
        }

        // Set a timer to be handled at the end of the stream's expected duration (if the stream does not already signal its end
        // using a RTCP "BYE").  This is optional.  If, instead, you want to keep the stream active - e.g., so you can later
        // 'seek' back within it and do another RTSP "PLAY" - then you can omit this code.
        // (Alternatively, if you don't want to receive the entire stream, you could set this timer for some shorter value.)
        if (scs.duration > 0) {
            unsigned int delaySlop = 2;
            scs.duration += delaySlop;
            int64_t usecs_to_delay = (int64_t)scs.duration * 1000 * 1000;
            scs.streamTimerTask = env.taskScheduler().scheduleDelayedTask(usecs_to_delay, streamTimerHandler, rtspClient);
        }

        xlog_dbg("start playing session\n");
        if (scs.duration > 0) {
            xlog_dbg("up to %.2f seconds\n", scs.duration);
        }

        success = True;
    } while (0);

    if (!success) {
        shutdownStream(rtspClient);
    }

    return ;
}

void setupNextSubsession(RTSPClient *rtspClient)
{
    do {
        UsageEnvironment &env = rtspClient->envir();
        StreamClientState &scs = ((MyRTSPClient*)rtspClient)->scs;
    
        scs.subsession = scs.iter->next();
        if (scs.subsession != nullptr) {
            if (!scs.subsession->initiate()) {
                xlog_err("failed to initiate subssion: %s\n", env.getResultMsg());
                setupNextSubsession(rtspClient);
            } else {
                xlog_dbg("initiated subsession\n");
                if (scs.subsession->rtcpIsMuxed()) {
                    xlog_dbg("client port: %d\n", scs.subsession->clientPortNum());
                } else {
                    xlog_dbg("client ports: %d-%d\n", scs.subsession->clientPortNum(),
                        scs.subsession->clientPortNum() + 1);
                }
                rtspClient->sendSetupCommand(*scs.subsession, continueAfterSETUP, false, REQUEST_STREAMING_OVER_TCP);
            }

            break;
        }

        if (scs.session->absStartTime()) {
            rtspClient->sendPlayCommand(*scs.session, continueAfterSETUP, scs.session->absStartTime(),
                scs.session->absEndTime());
        } else {
            scs.duration = scs.session->playEndTime() - scs.session->playStartTime();
            rtspClient->sendPlayCommand(*scs.session, continueAfterPLAY);
        }
    } while(0);

    return ;
}

void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char *resultString)
{
    do {
        UsageEnvironment& env = rtspClient->envir();
        
        if (resultCode != 0) {
            xlog_err("failed to get SDP description\n");
            delete [] resultString;
            break;
        }

        const char *sdpDescription = resultString;
        resultString = nullptr;
        // xlog_dbg("got a SDP description: %s\n", sdpDescription);
        xlog_dbg("got a SDP description\n");

        auto& scs = ((MyRTSPClient*)rtspClient)->scs;
        scs.session = MediaSession::createNew(env, sdpDescription);
        delete[] sdpDescription;
        sdpDescription = nullptr;
        
        if (!scs.session) {
            xlog_err("Failed to create MediaSession object from SDP description: %s\n", env.getResultMsg());
            break;
        } else if (!scs.session->hasSubsessions()) {
            xlog_err("This session has not media subsessions\n");
            break;
        }

        scs.iter = new MediaSubsessionIterator(*scs.session);
        setupNextSubsession(rtspClient);

        return ;
    } while (0);

    shutdownStream(rtspClient);

    return ;
}

int openUrl(UsageEnvironment& env, RtspClientCtx &ctx)
{
    do {
        xlog_dbg("openurl: %s\n", ctx.param.url);

        auto rtsp_client = MyRTSPClient::createNew(env, ctx.param.url, 
            RTSP_CLIENT_VERVOSITY_LEVEL, ctx.progName.c_str());
        if (!rtsp_client) {
            xlog_err("failed to create rtsp client for url: %s\n", ctx.param.url);
            break;
        }

        xlog_dbg("rtsp client: %s\n", rtsp_client->name());

        ctx.rtsp_client = rtsp_client;

        Authenticator auth(ctx.param.username, ctx.param.password);
        rtsp_client->sendDescribeCommand(continueAfterDESCRIBE, &auth);
    } while (0);

    return 0;
}

void trd_func(RtspClientCtx *ctx)
{
    TaskScheduler *scheduler = nullptr;
    UsageEnvironment *env = nullptr;
    
    do {
        int ret = 0;

        scheduler = BasicTaskScheduler::createNew();
        env = BasicUsageEnvironment::createNew(*scheduler);

        ret = openUrl(*env, *ctx);
        if (ret < 0) {
            xlog_err("open url failed\n");
            break;
        }

        xlog_dbg("rtsp client started\n");
        env->taskScheduler().doEventLoop(&ctx->eventLoopWatchVariable);
        xlog_dbg("rtsp client stopped\n");

        auto &session = ((MyRTSPClient*)ctx->rtsp_client)->scs.session;
        if(session) {
            ctx->rtsp_client->sendTeardownCommand(*session, nullptr);
        }
    } while (0);

    if (ctx->rtsp_client) {
        xlog_dbg("close rtsp client: %s\n", ctx->rtsp_client->name());
        shutdownStream(ctx->rtsp_client);
        ctx->rtsp_client = nullptr;
    }

    if (env) {
        if (!env->reclaim()) {
            xlog_err("reclaim failed\n");
        }
        env = nullptr;
    }

    if (scheduler) {
        delete scheduler;
        scheduler = nullptr;
    }

    return ;
}

}

rtsp_client_obj rtsp_client_start(rtsp_client_param const* param)
{
    RtspClientCtx *ctx = nullptr;

    do {
        ctx = new RtspClientCtx{};
        ctx->param = *param;

        ctx->eventLoopWatchVariable = 0;

        ctx->trd = std::make_shared<std::thread>(trd_func, ctx);
    } while (0);

    return reinterpret_cast<rtsp_client_obj>(ctx);
}

int rtsp_client_stop(rtsp_client_obj obj)
{
    do {
        auto *ctx = reinterpret_cast<RtspClientCtx*>(obj);
        if (!ctx) {
            xlog_dbg("null obj\n");
            break;
        }

        ctx->eventLoopWatchVariable = 1;
        if (ctx->trd->joinable()) {
            ctx->trd->join();
        }

        delete ctx;
        ctx = nullptr;
    } while (0);
    
    return 0;
}