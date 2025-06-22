#include "rtsp_client.h"

#include <thread>
#include <memory>
#include <string>
#include <vector>

#include "MediaSession.hh"
#include "RTSPClient.hh"
#include "BasicUsageEnvironment.hh"
#include "H264VideoRTPSource.hh"

#include "xlog.h"

using std::string;

namespace {

constexpr int audio_recv_buf = (100 * 1000);
constexpr int video_recv_buf = (100 * 1000);
constexpr int ivp4_timeout_ms = (1000);

class RtspClientWrapper;

struct Track {
    std::vector<uint8_t> buf;
    MediaSubsession *sub = nullptr;
    RtspClientWrapper *wrapper = nullptr;
};

using TrackPtr = std::shared_ptr<Track>;

struct MyPacket 
{
public:
    // MyPacket();
    int append(uint8_t *data, size_t size);

    std::shared_ptr<std::vector<uint8_t>> _data;
};

int MyPacket::append(uint8_t *data, size_t size)
{
    if (!_data) {
        _data = std::make_shared<std::vector<uint8_t> >();
    }
    auto old_size = _data->size();
    _data->resize(old_size + size);
    memcpy(_data->data() + old_size, data, size);

    return size;
}

class MyMediaSink : public MediaSink {
public:
    static MyMediaSink *createNew(UsageEnvironment &env, 
        TrackPtr track,
        const char *streamId = nullptr);
private:
    MyMediaSink(UsageEnvironment& env, TrackPtr track, const char *streamId);
    ~MyMediaSink() override;

    static void afterGettingFrame(void *clientData, unsigned int frameSize,
        unsigned int numTruncatedBytes, struct timeval presentationTime, unsigned int durationInMs);
    void afterGettingFrame(unsigned int frameSize,
        unsigned int numTruncatedBytes, struct timeval presentationTime, unsigned int durationInMs);
private:
    Boolean continuePlaying() override;

private:
    TrackPtr _track = nullptr;
    std::vector<uint8_t> _recv_buf;
    std::string _streamid;

    int _sps_pps_written = false;
    std::vector<uint8_t> _sps_pps_info;
    FILE *fp = nullptr;
};

class MyRtspClient : public RTSPClient
{
public:
    MyRtspClient(UsageEnvironment& env, char const* rtspURL, int verbosityLevel,
                   char const* applicationName, portNumBits tunnelOverHTTPPortNum, 
                   RtspClientWrapper *wrapper) : 
                   RTSPClient(env, rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum, -1) {
        this->_wrapper = wrapper;
    }
    RtspClientWrapper* wrapper() {
        return _wrapper;
    }
private:

private:
    RtspClientWrapper *_wrapper = nullptr;
};

class RtspClientWrapper 
{
public:
    RtspClientWrapper();
    ~RtspClientWrapper();
    int open(rtsp_client_param const *param);
    int close();

private:
    int connect();
    int session_setup();
    int play();
    int demux();
private:
    static void continueAfterDescribe(RTSPClient *client, int resultCode, char *resultString);
    static void default_live555_callback(RTSPClient *client, int resultCode, char *resultString);
    static void TaskInterruptRTSP(void *user);
    static void TaskInterruptData(void *user);
    static void afterPlaying(void *user);
    static void byeHandler(void *user, const char *reason);

    void continueAfterDescribe(int resultCode, char *resultString);
    void default_live555_callback(int resultCode, char *resultString);
    void TaskInterruptRTSP();
    void TaskInterruptData();
    void afterPlaying(Track *track);
    void byeHandler(Track *track, const char *reason);

    int wait_live555_response(int timeout_ms = 0 /* ms */);

private:
    void trd();

    UsageEnvironment *_env = nullptr;
    BasicTaskScheduler *_scheduler = nullptr;
    MyRtspClient *_rtsp = nullptr;
    std::shared_ptr<std::thread> _trd = nullptr;

    std::string _sdp; // session description 
    EventLoopWatchVariable _loop = 0;
    MediaSession *_ms = nullptr;
    int _live555_ret = 0;

    std::vector<std::shared_ptr<Track>> _tracks;

    struct param {
        std::string url;
        std::string username;
        std::string password;
    };
    param _param;
};


MyMediaSink *MyMediaSink::createNew(UsageEnvironment& env, TrackPtr track, char const* streamId)
{
    return new MyMediaSink(env, track, streamId);
}

MyMediaSink::MyMediaSink(UsageEnvironment &env, TrackPtr track, const char *streamId)
    : MediaSink(env)
{
    _streamid = std::string(streamId);
    _recv_buf.resize(video_recv_buf);
    _track = track;
}

MyMediaSink::~MyMediaSink() 
{
    // 
}

void MyMediaSink::afterGettingFrame(void *clientData, unsigned int frameSize,
    unsigned int numTruncatedBytes, struct timeval presentationTime, unsigned int durationInMs)
{
    MyMediaSink* sink = (MyMediaSink*)clientData;
    sink->afterGettingFrame(frameSize, numTruncatedBytes, presentationTime, durationInMs);

    return ;
}

void MyMediaSink::afterGettingFrame(unsigned int frameSize,
    unsigned int numTruncatedBytes, struct timeval presentationTime, unsigned int durationInMs)
{
    xlog_dbg("%s/%s: frame: %d\n", _track->sub->mediumName(), 
        _track->sub->codecName(), frameSize);

    if (strcmp(_track->sub->codecName(), "H264") == 0) {

        MyPacket packet = {};
        std::array<uint8_t, 4> nalu_sep = {0x0,0x0,0x0,0x1};

        if (!_sps_pps_written) {
            std::vector<uint8_t> buf_sps_pps;
            const char *prop = _track->sub->fmtp_spropparametersets();
            if (_sps_pps_info.empty()) {
                unsigned int numSPropRecords = 0;
                SPropRecord* sPropRecords = parseSPropParameterSets(prop, numSPropRecords);
                for (unsigned int i = 0; i < numSPropRecords; ++i) {
                    if (sPropRecords[i].sPropLength > 0) {
                        packet.append(nalu_sep.data(), nalu_sep.size());
                        // xlog_dbg("spspps: %zu\n", sPropRecords)
                        packet.append(sPropRecords[i].sPropBytes, sPropRecords[i].sPropLength);
                    }
                }
                if (sPropRecords) {
                    delete [] sPropRecords;
                    sPropRecords = nullptr;
                }
            }

            _sps_pps_written = true;
        }

        packet.append(nalu_sep.data(), nalu_sep.size());
        packet.append(_recv_buf.data(), frameSize);

        if (!fp) {
            char filename[64] = {};
            snprintf(filename, sizeof(filename), "dump_%s_%s", _track->sub->mediumName(),
                _track->sub->codecName());
            fp = fopen(filename, "w");
        }
        if (fp) {
            fwrite(packet._data->data(), 1, packet._data->size(), fp);
            fflush(fp);
            xlog_dbg("write: %zu bytes(%s)\n", packet._data->size(), _track->sub->codecName());

            // if (frameSize > 8)
            // {
            //     char buf[256] = {};
            //     size_t off = 0;
            //     for (size_t i = 0; i < 8; ++i) {
            //         if (off >= sizeof(buf)) {
            //             break;
            //         }
            //         off += snprintf(buf + off, sizeof(buf) - off, "%02hhx ", _recv_buf[i]);
            //     }
            //     xlog_dbg("%s: %s, type: %d\n", _track->sub->codecName(), buf, _recv_buf[0] & 0x1f);
            // }
        }
    }

    continuePlaying();
    return ;
}

Boolean MyMediaSink::continuePlaying()
{
    // xlog_dbg("continue playing\n");

    if (!fSource) {
        xlog_err("error\n");
        return False;
    }

    xlog_dbg("before get frame\n");
    fSource->getNextFrame(_recv_buf.data(), _recv_buf.size(), 
        afterGettingFrame, this, onSourceClosure, this);
    xlog_dbg("after get frame\n");
    
    return True;
}

void RtspClientWrapper::continueAfterDescribe(RTSPClient *client, int resultCode, char *resultString)
{
    auto cli = reinterpret_cast<MyRtspClient*>(client);
    return cli->wrapper()->continueAfterDescribe(resultCode, resultString);
}

void RtspClientWrapper::continueAfterDescribe(int resultCode, char *resultString)
{
    // todo
    // check result code and return to callers
    // like: _wrapper->rtsp_code = resultCode;
    
    do {
        std::string result_str;
        if (resultString) {
            result_str.assign(resultString);
            delete [] resultString;
            resultString = nullptr;
        }

        _live555_ret = resultCode;

        if (resultCode != 0) {
            xlog_err("failed to get SDP description\n");
            break;
        }

        _sdp = result_str;

        xlog_dbg("sdp: %s\n", _sdp.c_str());

    } while (0);

    _loop = 1;
    return ;
}

void RtspClientWrapper::default_live555_callback(RTSPClient *client, int resultCode, char *resultString)
{
    auto cli = reinterpret_cast<MyRtspClient*>(client);
    return cli->wrapper()->default_live555_callback(resultCode, resultString);
}

void RtspClientWrapper::default_live555_callback(int resultCode, char *resultString)
{
    xlog_dbg("cb: code:%d\n", resultCode);

    _live555_ret = resultCode;
    _loop = 1;

    xlog_dbg("set live555 ret: %d\n", _live555_ret);

    return ;
}

void RtspClientWrapper::TaskInterruptRTSP(void *user)
{
    auto wrapper = reinterpret_cast<RtspClientWrapper*>(user);
    return wrapper->TaskInterruptRTSP();
}

void RtspClientWrapper::TaskInterruptData(void *user)
{
    auto wrapper = reinterpret_cast<RtspClientWrapper*>(user);
    return wrapper->TaskInterruptData();
}

void RtspClientWrapper::TaskInterruptData()
{
    xlog_dbg("TaskInterruptData\n");
    _loop = static_cast<char>(0xff);
    return ;
}

void RtspClientWrapper::TaskInterruptRTSP()
{
    xlog_dbg("interrupt rtsp loop\n");
    _loop = static_cast<char>(0xff);
    return ;
}

void RtspClientWrapper::afterPlaying(void *user)
{
    auto track = reinterpret_cast<Track*>(user);
    return track->wrapper->afterPlaying(track);
}

void RtspClientWrapper::byeHandler(void *user, const char *reason)
{
    auto track = reinterpret_cast<Track*>(user);
    return track->wrapper->byeHandler(track, reason);
}

void RtspClientWrapper::afterPlaying(Track *track)
{
    xlog_err("close subsession stream not implemented yet\n");
}

void RtspClientWrapper::byeHandler(Track *track, const char *reason)
{
    xlog_dbg("bye handler of track\n");

    if (reason) {
        xlog_dbg("reason: %s\n", reason);
        delete [] reason;
    }

    return ;
}

int RtspClientWrapper::wait_live555_response(int timeout_ms)
{
    do {
        TaskToken task = {};
        
        _loop = 0;
        if (timeout_ms > 0) {
            xlog_dbg("schedule rtsp interrupt\n");
            task = _scheduler->scheduleDelayedTask((int64_t)timeout_ms * 1000, 
                TaskInterruptRTSP, this);
        }
        _live555_ret = -1;
        xlog_dbg("set live555 ret to %d\n", _live555_ret);
        _scheduler->doEventLoop(&_loop);
        if (timeout_ms > 0) {
            xlog_dbg("unschedule rtsp interrupt\n");
            _scheduler->unscheduleDelayedTask(task);
        }
    } while (0);

    return _live555_ret;
}

RtspClientWrapper::RtspClientWrapper()
{
    _scheduler = BasicTaskScheduler::createNew();
    _env = BasicUsageEnvironment::createNew(*_scheduler);
}

RtspClientWrapper::~RtspClientWrapper()
{
    xlog_dbg("do nothing\n");
    return ;
}

int RtspClientWrapper::connect()
{
    int success = 0;
    do {
        if (_rtsp) {
            xlog_err("inner error, not null\n");
            break;
        }

        int verbosityLevel = 1;
        // _rtsp = new MyRtspClient(*_env, _param.url.c_str(), 255, "test", 0, this);
        _rtsp = new MyRtspClient(*_env, _param.url.c_str(), verbosityLevel, "test", 0, this);

        xlog_dbg("connect %s using %s/%s\n", _param.url.c_str(), 
            _param.username.c_str(), 
            _param.password.c_str());

        Authenticator auth{};
        auth.setUsernameAndPassword(_param.username.c_str(), _param.password.c_str());
        _rtsp->sendDescribeCommand(RtspClientWrapper::continueAfterDescribe, &auth);

        // may add some timeout control here
        // eg. wait_Live555_response
        // and check for errors in previous calls
        int live555_ret = wait_live555_response(ivp4_timeout_ms);
        if (live555_ret) {
            if (live555_ret == 401) {
                xlog_err("authentication failed\n");
            } else {
                xlog_err("connect error: %d\n", live555_ret);
            }
            break;
        }

        success = 1;
    } while (0);

    return success ? 0 : -1;
}

int RtspClientWrapper::session_setup()
{
    int error_flag = false;

    MediaSubsessionIterator *iter = nullptr;
    MediaSubsession *sub = nullptr;

    do {
        int ret = 0;

        if (_ms) {
            xlog_err("inner error, not null\n");
            error_flag = true;
            break;
        }

        _ms = MediaSession::createNew(*_env, _sdp.c_str());
        if (!this->_ms) {
            xlog_err("create rtsp session failed: %s\n",
                _env->getResultMsg());
            error_flag = true;
            break;
        }

        if (!_ms->hasSubsessions()) {
            xlog_err("has no sub session\n");
            break;
        }

        iter = new MediaSubsessionIterator(*_ms);
        while (1) {
            sub = iter->next();
            if (!sub) {
                xlog_dbg("no more sub, break\n");
                break;
            }

            // todo
            // adjust buffer for stream

            int binit = sub->initiate();
            if (!binit) {
                xlog_err("rtp subsession %s/%s failed: %s\n", 
                    sub->mediumName(), sub->codecName(),
                    _env->getResultMsg());
                continue;
            }

            xlog_dbg("rtp subsesion: %s/%s\n", 
                sub->mediumName(), sub->codecName());

            if (sub->rtcpIsMuxed()) {
                xlog_dbg("client port: %d\n", sub->clientPortNum());
            } else {
                xlog_dbg("client ports: %d-%d\n", sub->clientPortNum(),
                    sub->clientPortNum() + 1);
            }

            int stream_out_going = 0;
            int stream_using_tcp = 1;
            int stream_force_multicast = 0;
            _rtsp->sendSetupCommand(*sub, RtspClientWrapper::default_live555_callback, 
                stream_out_going, stream_using_tcp, stream_force_multicast);

            ret = wait_live555_response();
            if (ret) {
                xlog_err("ret: %d\n", ret);
                break;
            }

            // if (!sub->readSource()) {
            //     xlog_war("sub read failed\n");
            //     continue;
            // }

            if (sub->rtcpInstance()) {
                sub->rtcpInstance()->setByeHandler(nullptr, nullptr);
            }

            auto track = std::make_shared<Track>();
            track->sub = sub;
            track->buf.resize(video_recv_buf);
            track->wrapper = this;
            _tracks.push_back(track);

            sub->sink = MyMediaSink::createNew(*_env, track, _rtsp->url());
            if (!sub->sink) {
                xlog_err("create sink failed\n");
                break;
            }
            sub->miscPtr = _rtsp;

            sub->sink->startPlaying(*sub->readSource(), afterPlaying, track.get());
            if (sub->rtcpInstance()) {
                sub->rtcpInstance()->setByeWithReasonHandler(byeHandler, track.get());
            }
        }
    } while (0);

    if (iter) {
        delete iter;
        iter = nullptr;
    }

    if (_tracks.empty()) {
        xlog_err("no track\n");
        error_flag = true;
    }

    return error_flag ? -1 : 0;
}

int RtspClientWrapper::play()
{
    int error_flag = 0;

    do {
        _rtsp->sendPlayCommand(*_ms, default_live555_callback);
        if (wait_live555_response()) {
            xlog_err("rtsp play failed: %s\n", _env->getResultMsg());
            error_flag = 1;
            break;
        }

        auto start_time =_ms->playStartTime();
        auto end_time = _ms->playEndTime();

        xlog_inf("play: starttime=%g, endtime=%g\n", start_time, end_time);
    } while (0);

    return error_flag ? -1 : 0;
}

int RtspClientWrapper::demux()
{
    do {
        xlog_dbg("demux in\n");
        for (size_t i = 0; i < _tracks.size(); ++i) {
            // auto &track = _tracks[i];
        }

        // auto task = _scheduler->scheduleDelayedTask(300 * 1000, TaskInterruptData, this);
        _loop = 0;
        _scheduler->doEventLoop(&_loop);
        // _scheduler->unscheduleDelayedTask(task);
        xlog_dbg("demux out\n");
    } while(0);

    return 0;
}

void RtspClientWrapper::trd()
{
    // refer to live555.cpp::Open in vlc

    do {
        if (connect()) {
            xlog_err("connect failed\n");
            break;
        }

        if (_sdp.empty()) {
            xlog_err("failed to retrieve the RTSP Session Description");
            break;
        }

        xlog_dbg("connect ok\n");

        if (session_setup()) {
            xlog_err("nothing to play: no track\n");
            break;
        }

        xlog_dbg("session setup ok\n");

        if (play()) {
            xlog_err("play failed\n");
            break;
        }

        xlog_dbg("play ok\n");

        while (!demux());

        xlog_dbg("demux end\n");
    } while (0);
}

int RtspClientWrapper::open(rtsp_client_param const *param)
{
    auto trd_func = [this](){
        return this->trd();
    };

    _param.url = param->url;
    _param.username = param->username;
    _param.password = param->password;

    _trd = std::make_shared<std::thread>(trd_func);
    
    return 0;
}

}

rtsp_client_obj rtsp_client_start(rtsp_client_param const* param)
{
    RtspClientWrapper *ctx = nullptr;

    do {
        ctx = new RtspClientWrapper{};
        ctx->open(param);
    } while (0);

    return reinterpret_cast<rtsp_client_obj>(ctx);
}

int rtsp_client_stop(rtsp_client_obj obj)
{
    do {
        auto *ctx = reinterpret_cast<RtspClientWrapper*>(obj);
        if (!ctx) {
            xlog_dbg("null obj\n");
            break;
        }

        delete ctx;
        ctx = nullptr;
    } while (0);
    
    return 0;
}