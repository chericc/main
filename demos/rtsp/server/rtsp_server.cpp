#include "rtsp_server.h"

#include "xlog.h"

// live555 headers (sysroot)
#include "BasicUsageEnvironment.hh"
#include "H264VideoRTPSink.hh"
#include "H264VideoStreamDiscreteFramer.hh"
#include "H265VideoRTPSink.hh"
#include "H265VideoStreamDiscreteFramer.hh"
#include "liveMedia.hh"

#include <atomic>
#include <cstring>
#include <deque>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

namespace {

constexpr uint64_t kUsecPerSec = 1000000;

} // anonymous namespace

// ============================================================
// LiveSource -- a FramedSource that accepts complete AnnexB
// frames (with start codes) and outputs individual NAL units
// (without start codes) one at a time.  Downstream
// H264VideoStreamDiscreteFramer / H265VideoStreamDiscreteFramer
// handles SPS/PPS tracking and RTP marker bit setting.
// ============================================================

class LiveSource : public FramedSource {
public:
    struct Frame {
        std::vector<uint8_t> data;
        uint64_t ptsUs{0};
    };

    static LiveSource *createNew(UsageEnvironment &env, unsigned bufferSize);

    // pushFrame: accepts AnnexB stream (with start codes) and
    // splits it into individual NAL units internally.
    void pushFrame(const uint8_t *data, size_t size, uint64_t ptsUs);

    using DestroyCb = std::function<void(LiveSource *)>;
    void onDestroy(DestroyCb cb) { onDestroy_ = std::move(cb); }

protected:
    LiveSource(UsageEnvironment &env, unsigned bufferSize);
    virtual ~LiveSource();

private:
    void doGetNextFrame() override;

    static void deliverFrame0_(void *clientData);
    void deliverFrame_();

    std::mutex              mutex_;
    std::deque<Frame>       queue_;
    EventTriggerId          eventTriggerId_{0};
    DestroyCb               onDestroy_;
};

LiveSource *LiveSource::createNew(UsageEnvironment &env, unsigned bufferSize)
{
    return new LiveSource(env, bufferSize);
}

LiveSource::LiveSource(UsageEnvironment &env, unsigned bufferSize)
    : FramedSource(env)
{
    fMaxSize = bufferSize;
    eventTriggerId_ = envir().taskScheduler().createEventTrigger(deliverFrame0_);
    xlog_inf("LiveSource created, triggerId={}, fMaxSize={}", eventTriggerId_, fMaxSize);
}

LiveSource::~LiveSource()
{
    xlog_inf("LiveSource destroyed, triggerId={}", eventTriggerId_);
    if (onDestroy_) {
        onDestroy_(this);
    }
    if (eventTriggerId_ != 0) {
        envir().taskScheduler().deleteEventTrigger(eventTriggerId_);
    }
}

void LiveSource::pushFrame(const uint8_t *data, size_t size, uint64_t ptsUs)
{
    if (!data || size < 4) return;

    const uint8_t *end = data + size;
    const uint8_t *nal_start = nullptr;

    for (const uint8_t *p = data; p < end; ) {
        bool found = false;
        size_t skip = 0;
        if (p + 3 < end && p[0]==0x00 && p[1]==0x00 && p[2]==0x01) {
            skip = 3; found = true;
        } else if (p + 4 < end && p[0]==0x00 && p[1]==0x00 && p[2]==0x00 && p[3]==0x01) {
            skip = 4; found = true;
        }

        if (found) {
            if (nal_start) {
                size_t nal_size = p - nal_start;
                if (nal_size > 0) {
                    std::lock_guard<std::mutex> lock(mutex_);
                    queue_.push_back({std::vector<uint8_t>(nal_start, nal_start+nal_size), ptsUs});
                }
            }
            nal_start = p + skip;
            p += skip;
        } else {
            ++p;
        }
    }

    if (nal_start && nal_start < end) {
        size_t nal_size = end - nal_start;
        if (nal_size > 0) {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push_back({std::vector<uint8_t>(nal_start, nal_start+nal_size), ptsUs});
        }
    }

    envir().taskScheduler().triggerEvent(eventTriggerId_, this);
}

void LiveSource::doGetNextFrame()
{
    deliverFrame_();
}

void LiveSource::deliverFrame0_(void *clientData)
{
    static_cast<LiveSource *>(clientData)->deliverFrame_();
}

void LiveSource::deliverFrame_()
{
    if (isCurrentlyAwaitingData() == False) {
        return;
    }

    Frame frame;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty()) {
            return;
        }
        frame = std::move(queue_.front());
        queue_.pop_front();
    }

    size_t size = frame.data.size();

    if (size > fMaxSize) {
        fFrameSize         = fMaxSize;
        fNumTruncatedBytes = size - fMaxSize;
    } else {
        fFrameSize = size;
        fNumTruncatedBytes = 0;
    }
    std::memcpy(fTo, frame.data.data(), fFrameSize);

    fPresentationTime.tv_sec  = static_cast<long>(frame.ptsUs / kUsecPerSec);
    fPresentationTime.tv_usec = static_cast<long>(frame.ptsUs % kUsecPerSec);
    fDurationInMicroseconds   = 0;

    afterGetting(this);
}

// ============================================================
// LiveServerMediaSubsession
// ============================================================

class LiveServerMediaSubsession : public OnDemandServerMediaSubsession {
public:
    static LiveServerMediaSubsession *createNew(UsageEnvironment &env,
                                                 rtsp_server::Codec codec,
                                                 unsigned bufferSize);

    void broadcastFrame(const uint8_t *data, size_t size, uint64_t ptsUs);

protected:
    LiveServerMediaSubsession(UsageEnvironment &env, rtsp_server::Codec codec,
                               unsigned bufferSize);
    virtual ~LiveServerMediaSubsession();

private:
    FramedSource *createNewStreamSource(unsigned clientSessionId,
                                                 unsigned &estBitrate) override;
    RTPSink *createNewRTPSink(Groupsock *rtpGroupsock,
                                       unsigned char rtpPayloadTypeIfDynamic,
                                       FramedSource *inputSource) override;

    rtsp_server::Codec fCodec_;
    unsigned          fBufferSize_;
    std::mutex         sourceMutex_;
    std::vector<LiveSource *> sources_;
};

LiveServerMediaSubsession *
LiveServerMediaSubsession::createNew(UsageEnvironment &env, rtsp_server::Codec codec,
                                      unsigned bufferSize)
{
    return new LiveServerMediaSubsession(env, codec, bufferSize);
}

LiveServerMediaSubsession::LiveServerMediaSubsession(UsageEnvironment &env,
                                                       rtsp_server::Codec codec,
                                                       unsigned bufferSize)
    : OnDemandServerMediaSubsession(env, False)
    , fCodec_(codec)
    , fBufferSize_(bufferSize)
{
}

LiveServerMediaSubsession::~LiveServerMediaSubsession() {}

FramedSource *
LiveServerMediaSubsession::createNewStreamSource(unsigned /*clientSessionId*/,
                                                  unsigned &estBitrate)
{
    estBitrate = 5000;
    xlog_inf("createNewStreamSource");

    auto *source = LiveSource::createNew(envir(), fBufferSize_);

    source->onDestroy([this](LiveSource *s) {
        std::lock_guard<std::mutex> lock(sourceMutex_);
        for (auto it = sources_.begin(); it != sources_.end(); ++it) {
            if (*it == s) {
                sources_.erase(it);
                break;
            }
        }
    });

    {
        std::lock_guard<std::mutex> lock(sourceMutex_);
        sources_.push_back(source);
    }

    if (fCodec_ == rtsp_server::Codec::H264) {
        return H264VideoStreamDiscreteFramer::createNew(envir(), source);
    }
    return H265VideoStreamDiscreteFramer::createNew(envir(), source);
}

RTPSink *
LiveServerMediaSubsession::createNewRTPSink(Groupsock *rtpGroupsock,
                                             unsigned char rtpPayloadTypeIfDynamic,
                                             FramedSource * /*inputSource*/)
{
    xlog_inf("createNewRTPSink: payloadType={}", rtpPayloadTypeIfDynamic);

    OutPacketBuffer::maxSize = fBufferSize_;

    if (fCodec_ == rtsp_server::Codec::H264) {
        return H264VideoRTPSink::createNew(envir(), rtpGroupsock,
                                           rtpPayloadTypeIfDynamic);
    }
    return H265VideoRTPSink::createNew(envir(), rtpGroupsock,
                                       rtpPayloadTypeIfDynamic);
}

void LiveServerMediaSubsession::broadcastFrame(const uint8_t *data, size_t size,
                                                 uint64_t ptsUs)
{
    std::lock_guard<std::mutex> lock(sourceMutex_);
    for (auto *s : sources_) {
        s->pushFrame(data, size, ptsUs);
    }
}

// ============================================================
// ServerContext -- singleton managing the live555 event loop
// ============================================================

struct ServerContext {
    TaskScheduler    *scheduler_{nullptr};
    UsageEnvironment *env_{nullptr};
    RTSPServer       *rtspServer_{nullptr};
    ServerMediaSession     *sms_{nullptr};
    LiveServerMediaSubsession *subsession_{nullptr};

    std::thread       eventLoopThread_;
    std::atomic<char> quitFlag_{0};
    std::atomic<bool> initialized_{false};

    bool init(const rtsp_server::Config &cfg)
    {
        do {
            scheduler_ = BasicTaskScheduler::createNew();
            if (scheduler_ == nullptr) {
                xlog_err("BasicTaskScheduler::createNew failed");
                break;
            }

            env_ = BasicUsageEnvironment::createNew(*scheduler_);
            if (env_ == nullptr) {
                xlog_err("BasicUsageEnvironment::createNew failed");
                break;
            }

            Port port(cfg.port);
            rtspServer_ = RTSPServer::createNew(*env_, port, nullptr);
            if (rtspServer_ == nullptr) {
                xlog_err("RTSPServer::createNew failed on port {}", cfg.port);
                break;
            }

            sms_ = ServerMediaSession::createNew(*env_,
                                                   cfg.streamName.c_str(),
                                                   cfg.streamName.c_str(),
                                                   "Live Stream");
            if (sms_ == nullptr) {
                xlog_err("ServerMediaSession::createNew failed");
                break;
            }

            subsession_ = LiveServerMediaSubsession::createNew(*env_, cfg.codec, cfg.bufferSize);
            if (subsession_ == nullptr) {
                xlog_err("LiveServerMediaSubsession::createNew failed");
                break;
            }
            sms_->addSubsession(subsession_);

            rtspServer_->addServerMediaSession(sms_);

            char *url = rtspServer_->rtspURL(sms_);
            if (url != nullptr) {
                xlog_inf("RTSP stream ready at {}", url);
                delete[] url;
            }
            xlog_inf("RTSP server listening on port {}, stream='{}'",
                      cfg.port, cfg.streamName);

            quitFlag_ = false;
            eventLoopThread_ = std::thread([this]() {
                env_->taskScheduler().doEventLoop(&quitFlag_);
            });

            initialized_ = true;
            return true;
        } while (false);

        cleanup_();
        return false;
    }

    void shutdown()
    {
        if (!initialized_.exchange(false)) {
            xlog_inf("RTSP server shutdown: not initialized, skipped");
            return;
        }

        xlog_inf("RTSP server shutdown: stopping event loop...");
        quitFlag_ = true;
        if (eventLoopThread_.joinable()) {
            eventLoopThread_.join();
            xlog_inf("RTSP server event loop thread joined");
        }
        cleanup_();
        xlog_inf("RTSP server shutdown complete");
    }

private:
    void cleanup_()
    {
        if (rtspServer_ != nullptr) {
            Medium::close(rtspServer_);
            rtspServer_ = nullptr;
        }
        sms_        = nullptr;
        subsession_ = nullptr;

        if (env_ != nullptr) {
            env_->reclaim();
            env_ = nullptr;
        }
        if (scheduler_ != nullptr) {
            delete scheduler_;
            scheduler_ = nullptr;
        }
    }
};

// ============================================================
// Module-level singleton
// ============================================================

namespace {

ServerContext gServerCtx;

} // anonymous namespace

// ============================================================
// Public API
// ============================================================

namespace rtsp_server {

bool init(const Config &cfg)
{
    if (gServerCtx.initialized_) {
        xlog_war("RTSP server already initialized, shutting down first");
        gServerCtx.shutdown();
    }
    return gServerCtx.init(cfg);
}

void pushFrame(const uint8_t *data, size_t size, uint64_t timestampUs)
{
    if (data == nullptr || size == 0) {
        xlog_war("pushFrame: null or empty frame");
        return;
    }
    if (gServerCtx.subsession_ == nullptr) {
        return;
    }
    xlog_inf("pushFrame: {} bytes, ts={} us", size, timestampUs);

    gServerCtx.subsession_->broadcastFrame(data, size, timestampUs);
}

bool isRunning()
{
    return gServerCtx.initialized_;
}

void shutdown()
{
    gServerCtx.shutdown();
}

} // namespace rtsp_server
