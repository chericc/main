#include "rtsp_client.hpp"

#include "xlog.h"

// live555 headers
#include "BasicUsageEnvironment.hh"
#include "liveMedia.hh"

#include <atomic>
#include <cstring>
#include <string>
#include <thread>

// ============================================================
// operator<< overloads for RTSPClient / MediaSubsession logging
// ============================================================

static UsageEnvironment &operator<<(UsageEnvironment &env,
                                     const RTSPClient &c) {
    return env << "[URL:\"" << c.url() << "\"]: ";
}

static UsageEnvironment &operator<<(UsageEnvironment &env,
                                     const MediaSubsession &s) {
    return env << s.mediumName() << "/" << s.codecName();
}

// ============================================================
// Everything below is internal linkage
// ============================================================

namespace {

// ============================================================
// Per-stream state attached to each ourRTSPClient
// ============================================================

class StreamClientState {
public:
    MediaSubsessionIterator *iter      = nullptr;
    MediaSession            *session   = nullptr;
    MediaSubsession         *subsession = nullptr;

    ~StreamClientState() {
        delete iter;
        Medium::close(session);
    }
};

class ourRTSPClient : public RTSPClient {
public:
    static ourRTSPClient *createNew(UsageEnvironment &env, char const *rtspURL,
                                    int verbosityLevel = 0,
                                    char const *applicationName = nullptr) {
        return new ourRTSPClient(env, rtspURL, verbosityLevel, applicationName);
    }

    StreamClientState scs;

protected:
    ourRTSPClient(UsageEnvironment &env, char const *rtspURL,
                  int verbosityLevel, char const *applicationName)
        : RTSPClient(env, rtspURL, verbosityLevel, applicationName, 0, -1) {}
    ~ourRTSPClient() override = default;
};

// ============================================================
// RTSP response handlers (forward declarations)
// ============================================================

static void continueAfterDESCRIBE(RTSPClient *, int, char *);
static void continueAfterSETUP(RTSPClient *, int, char *);
static void continueAfterPLAY(RTSPClient *, int, char *);
static void setupNextSubsession(RTSPClient *);
static void subsessionAfterPlaying(void *);
static void shutdownStream(RTSPClient *);

// ============================================================
// ClientContext — singleton managing the live555 event loop
// ============================================================

struct ClientContext {
    TaskScheduler    *scheduler   = nullptr;
    UsageEnvironment *env         = nullptr;
    ourRTSPClient    *rtspClient  = nullptr;

    std::string       outputFile;

    std::thread       eventLoopThread;
    EventLoopWatchVariable quitFlag = 0;
    std::atomic<bool> initialized{false};
    std::atomic<bool> clientAlive{false};

    bool init(const char *rtspURL, const char *file) {
        do {
            scheduler = BasicTaskScheduler::createNew();
            if (!scheduler) {
                xlog_err("BasicTaskScheduler::createNew failed");
                break;
            }

            env = BasicUsageEnvironment::createNew(*scheduler);
            if (!env) {
                xlog_err("BasicUsageEnvironment::createNew failed");
                break;
            }

            rtspClient = ourRTSPClient::createNew(*env, rtspURL,
                                                   1, "rtsp_client");
            if (!rtspClient) {
                xlog_err("ourRTSPClient::createNew failed");
                break;
            }

            outputFile  = file;
            clientAlive = true;

            // Initiate the RTSP handshake (handled async in the event loop)
            rtspClient->sendDescribeCommand(continueAfterDESCRIBE);

            // Start event loop in background thread
            quitFlag = 0;
            eventLoopThread = std::thread([this]() {
                env->taskScheduler().doEventLoop(&quitFlag);
                xlog_inf("RTSP client event loop ended");
            });

            initialized = true;
            xlog_inf("RTSP client initialised, connecting to {} ...", rtspURL);
            return true;
        } while (false);

        cleanup_();
        return false;
    }

    void shutdown() {
        if (!initialized.exchange(false)) {
            xlog_inf("RTSP client shutdown: not initialised, skipped");
            return;
        }

        xlog_inf("RTSP client shutdown: stopping event loop...");
        quitFlag = 1;
        if (eventLoopThread.joinable()) {
            eventLoopThread.join();
            xlog_inf("RTSP client event loop thread joined");
        }

        // If the event-loop thread didn't already close the client
        // (e.g. stream ended normally), close it now.
        if (clientAlive.exchange(false) && rtspClient) {
            Medium::close(rtspClient);
            rtspClient = nullptr;
        }

        cleanup_();
        xlog_inf("RTSP client shutdown complete");
    }

private:
    void cleanup_() {
        if (env) {
            env->reclaim();
            env = nullptr;
        }
        if (scheduler) {
            delete scheduler;
            scheduler = nullptr;
        }
        rtspClient = nullptr;
    }
};

static ClientContext gCtx;

// ============================================================
// RTSP response handlers
// ============================================================

static void continueAfterDESCRIBE(RTSPClient *rtspClient, int resultCode,
                                   char *resultString) {
    do {
        UsageEnvironment  &env = rtspClient->envir();
        StreamClientState &scs = ((ourRTSPClient *)rtspClient)->scs;

        if (resultCode != 0) {
            env << *rtspClient << "DESCRIBE failed: " << resultString << "\n";
            delete[] resultString;
            break;
        }

        char *sdp = resultString;
        xlog_inf("Got SDP, creating MediaSession...");

        scs.session = MediaSession::createNew(env, sdp);
        delete[] sdp;
        if (!scs.session) {
            env << *rtspClient << "MediaSession::createNew failed\n";
            break;
        }
        if (!scs.session->hasSubsessions()) {
            env << *rtspClient << "Session has no subsessions\n";
            break;
        }

        scs.iter = new MediaSubsessionIterator(*scs.session);
        setupNextSubsession(rtspClient);
        return;
    } while (0);

    shutdownStream(rtspClient);
}

static void setupNextSubsession(RTSPClient *rtspClient) {
    UsageEnvironment  &env = rtspClient->envir();
    StreamClientState &scs = ((ourRTSPClient *)rtspClient)->scs;

    scs.subsession = scs.iter->next();
    if (!scs.subsession) {
        xlog_inf("All subsessions set up, sending PLAY...");
        rtspClient->sendPlayCommand(*scs.session, continueAfterPLAY);
        return;
    }

    // Only video (H264/H265)
    if (strcmp(scs.subsession->mediumName(), "video") != 0) {
        xlog_inf("Skipping non-video subsession: {}/{}",
                 scs.subsession->mediumName(), scs.subsession->codecName());
        setupNextSubsession(rtspClient);
        return;
    }

    const char *codec = scs.subsession->codecName();
    if (strcmp(codec, "H264") != 0 && strcmp(codec, "H265") != 0) {
        xlog_war("Unsupported video codec: {}, skipping", codec);
        setupNextSubsession(rtspClient);
        return;
    }

    if (!scs.subsession->initiate()) {
        env << *rtspClient << "Failed to initiate \""
            << *scs.subsession << "\" subsession\n";
        setupNextSubsession(rtspClient);
        return;
    }

    xlog_inf("Initiated {}/{}", scs.subsession->mediumName(), codec);

    rtspClient->sendSetupCommand(*scs.subsession, continueAfterSETUP,
                                 False /*streamOutgoing*/, True /*streamUsingTCP*/);
}

static void continueAfterSETUP(RTSPClient *rtspClient, int resultCode,
                                char *resultString) {
    do {
        UsageEnvironment  &env = rtspClient->envir();
        StreamClientState &scs = ((ourRTSPClient *)rtspClient)->scs;

        if (resultCode != 0) {
            env << *rtspClient << "SETUP failed: " << resultString << "\n";
            break;
        }

        env << *rtspClient << "SETUP OK: \"" << *scs.subsession << "\"\n";

        // Create the appropriate file sink
        const char *codec = scs.subsession->codecName();
        MediaSink  *sink  = nullptr;

        if (strcmp(codec, "H264") == 0) {
            char const *sprop =
                scs.subsession->attrVal_str("sprop-parameter-sets");
            sink = H264VideoFileSink::createNew(env, gCtx.outputFile.c_str(),
                                                 sprop, 200000);
        } else if (strcmp(codec, "H265") == 0) {
            char const *spropVPS = scs.subsession->attrVal_str("sprop-vps");
            char const *spropSPS = scs.subsession->attrVal_str("sprop-sps");
            char const *spropPPS = scs.subsession->attrVal_str("sprop-pps");
            sink = H265VideoFileSink::createNew(env, gCtx.outputFile.c_str(),
                                                 spropVPS, spropSPS, spropPPS,
                                                 200000);
        } else {
            break;
        }

        if (!sink) {
            env << *rtspClient << "Failed to create file sink\n";
            break;
        }

        xlog_inf("Created file sink -> {}", gCtx.outputFile);

        scs.subsession->sink = sink;
        scs.subsession->miscPtr = rtspClient;
        sink->startPlaying(*(scs.subsession->readSource()),
                           subsessionAfterPlaying, scs.subsession);

        if (scs.subsession->rtcpInstance()) {
            scs.subsession->rtcpInstance()->setByeHandler(
                subsessionAfterPlaying, scs.subsession);
        }
    } while (0);

    delete[] resultString;
    setupNextSubsession(rtspClient);
}

static void continueAfterPLAY(RTSPClient *rtspClient, int resultCode,
                               char *resultString) {
    if (resultCode != 0) {
        UsageEnvironment &env = rtspClient->envir();
        env << *rtspClient << "PLAY failed: " << resultString << "\n";
        delete[] resultString;
        shutdownStream(rtspClient);
        return;
    }

    xlog_inf("Started playing, receiving stream...");
    delete[] resultString;
}

// ============================================================
// Stream-end handler
// ============================================================

static void subsessionAfterPlaying(void *clientData) {
    auto *subsession = static_cast<MediaSubsession *>(clientData);
    auto *rtspClient = static_cast<RTSPClient *>(subsession->miscPtr);

    xlog_inf("Subsession ended: {}/{}",
             subsession->mediumName(), subsession->codecName());

    Medium::close(subsession->sink);
    subsession->sink = nullptr;

    MediaSession           &session = subsession->parentSession();
    MediaSubsessionIterator iter(session);
    while ((subsession = iter.next()) != nullptr) {
        if (subsession->sink != nullptr)
            return;
    }

    xlog_inf("All subsessions ended");
    shutdownStream(rtspClient);
}

static void shutdownStream(RTSPClient *rtspClient) {
    StreamClientState &scs = ((ourRTSPClient *)rtspClient)->scs;

    if (scs.session) {
        MediaSubsessionIterator iter(*scs.session);
        MediaSubsession *subsession;
        while ((subsession = iter.next()) != nullptr) {
            if (subsession->sink) {
                Medium::close(subsession->sink);
                subsession->sink = nullptr;
                if (subsession->rtcpInstance())
                    subsession->rtcpInstance()->setByeHandler(nullptr, nullptr);
            }
        }
        rtspClient->sendTeardownCommand(*scs.session, nullptr);
    }

    xlog_inf("Closing RTSP client");
    Medium::close(rtspClient);

    gCtx.rtspClient  = nullptr;
    gCtx.clientAlive = false;
    gCtx.quitFlag    = 1;
}

} // anonymous namespace

// ============================================================
// Public API
// ============================================================

namespace rtsp_client {

bool init(const char *rtspURL, const char *outputFile) {
    if (gCtx.initialized) {
        xlog_war("RTSP client already initialised, shutting down first");
        gCtx.shutdown();
    }
    return gCtx.init(rtspURL, outputFile);
}

bool isRunning() {
    return gCtx.initialized;
}

void shutdown() {
    gCtx.shutdown();
}

} // namespace rtsp_client
