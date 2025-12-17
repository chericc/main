#include "rtsp_client.h"

#include <thread>
#include <memory>
#include <string>
#include <vector>
#include <array>

#include "liveMedia/MediaSession.hh"
#include "liveMedia/RTSPClient.hh"
#include "BasicUsageEnvironment/BasicUsageEnvironment.hh"
#include "liveMedia/H264VideoRTPSource.hh"
#include "liveMedia/ADTSAudioStreamDiscreteFramer.hh"

#include "xlog.h"

using std::string;

namespace {

constexpr int audio_recv_buf = (100 * 1000);
constexpr int video_recv_buf = (100 * 1000);
constexpr int ivp4_timeout_ms = (1000);

class RtspClientWrapper;

struct Track {
    int track_id;
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

class MySink : public MediaSink {
public:
    static MySink *createNew(UsageEnvironment &env, 
        TrackPtr track, rtsp_client_cb cb);
private:
    MySink(UsageEnvironment& env, TrackPtr track, rtsp_client_cb cb);
    ~MySink() override = default;

    static void afterGettingFrame(void *clientData, unsigned int frameSize,
        unsigned int numTruncatedBytes, struct timeval presentationTime, unsigned int durationInMs);
    void afterGettingFrame(unsigned int frameSize,
        unsigned int numTruncatedBytes, struct timeval presentationTime, unsigned int durationInMs);
private:
    Boolean continuePlaying() override;

    rtsp_client_cb _cb = nullptr;
    TrackPtr _track = nullptr;
    std::vector<uint8_t> _recv_buf;
};

class MyH264or5MediaSink : public MediaSink {
public:
    static MyH264or5MediaSink *createNew(UsageEnvironment &env, 
        TrackPtr track, rtsp_client_cb cb);
private:
    MyH264or5MediaSink(UsageEnvironment& env, TrackPtr track, rtsp_client_cb cb);
    ~MyH264or5MediaSink() override = default;

    static void afterGettingFrame(void *clientData, unsigned int frameSize,
        unsigned int numTruncatedBytes, struct timeval presentationTime, unsigned int durationInMs);
    void afterGettingFrame(unsigned int frameSize,
        unsigned int numTruncatedBytes, struct timeval presentationTime, unsigned int durationInMs);
private:
    Boolean continuePlaying() override;

private:
    TrackPtr _track = nullptr;
    std::vector<uint8_t> _recv_buf;

    int _sps_pps_written = false;
    // std::vector<uint8_t> _sps_pps_info;
    // FILE *fp = nullptr;
    rtsp_client_cb _cb = nullptr;
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
    int close_imp();
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
    int _close_flag = 0;
    int _close_fin_flag = 0;

    std::vector<std::shared_ptr<Track>> _tracks;

    struct param {
        std::string url;
        std::string username;
        std::string password;
        rtsp_client_cb cb = nullptr;
    };
    param _param;
};


MySink *MySink::createNew(UsageEnvironment& env, TrackPtr track, rtsp_client_cb cb)
{
    return new MySink(env, track, cb);
}

MySink::MySink(UsageEnvironment &env, TrackPtr track, rtsp_client_cb cb)
    : MediaSink(env)
{
    xlog_dbg("sink created\n");
    _recv_buf.resize(audio_recv_buf);
    _track = std::move(track);
    _cb = cb;
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
    // xlog_dbg("{}/{}: frame: {}\n", _track->sub->mediumName(), 
    //     _track->sub->codecName(), frameSize);

    if (_cb) {
        int channel_id = _track->track_id;
        const char *medium_name = _track->sub->mediumName();
        const char *codec_name = _track->sub->codecName();
        _cb(channel_id, _recv_buf.data(), frameSize, medium_name, codec_name, 
            presentationTime, durationInMs);
    }
    
    continuePlaying();
}

Boolean MySink::continuePlaying()
{
    if (!fSource) {
        xlog_err("error\n");
        return False;
    }

    // xlog_dbg("before get frame\n");
    fSource->getNextFrame(_recv_buf.data(), _recv_buf.size(), 
        afterGettingFrame, this, onSourceClosure, this);
    // xlog_dbg("after get frame\n");
    return True;
}

MyH264or5MediaSink *MyH264or5MediaSink::createNew(UsageEnvironment& env, TrackPtr track, rtsp_client_cb cb)
{
    return new MyH264or5MediaSink(env, track, cb);
}

MyH264or5MediaSink::MyH264or5MediaSink(UsageEnvironment &env, TrackPtr track, rtsp_client_cb cb)
    : MediaSink(env)
{
    xlog_dbg("sink created\n");
    _recv_buf.resize(video_recv_buf);
    _track = std::move(track);
    _cb = cb;
}

void MyH264or5MediaSink::afterGettingFrame(void *clientData, unsigned int frameSize,
    unsigned int numTruncatedBytes, struct timeval presentationTime, unsigned int durationInMs)
{
    MyH264or5MediaSink* sink = (MyH264or5MediaSink*)clientData;
    sink->afterGettingFrame(frameSize, numTruncatedBytes, presentationTime, durationInMs);

    return ;
}

void MyH264or5MediaSink::afterGettingFrame(unsigned int frameSize,
    unsigned int numTruncatedBytes, struct timeval presentationTime, unsigned int durationInMs)
{
    // xlog_dbg("{}/{}: frame: {}\n", _track->sub->mediumName(), 
    //     _track->sub->codecName(), frameSize);

    if ( (strcmp(_track->sub->codecName(), "H264") == 0)
        || (strcmp(_track->sub->codecName(), "H265") == 0)) {

        MyPacket packet = {};
        std::array<uint8_t, 4> nalu_sep = {0x0,0x0,0x0,0x1};

        if (!_sps_pps_written) {
            std::vector<uint8_t> buf_sps_pps;
            const char *prop = _track->sub->fmtp_spropparametersets();
            
            unsigned int numSPropRecords = 0;
            SPropRecord* sPropRecords = parseSPropParameterSets(prop, numSPropRecords);
            for (unsigned int i = 0; i < numSPropRecords; ++i) {
                if (sPropRecords[i].sPropLength > 0) {
                    packet.append(nalu_sep.data(), nalu_sep.size());
                    xlog_dbg("spspps: {}\n", sPropRecords[i].sPropLength);
                    packet.append(sPropRecords[i].sPropBytes, sPropRecords[i].sPropLength);
                }
            }
            if (sPropRecords) {
                delete [] sPropRecords;
                sPropRecords = nullptr;
            }

            _sps_pps_written = true;
        }

        packet.append(nalu_sep.data(), nalu_sep.size());
        packet.append(_recv_buf.data(), frameSize);

        if (_cb) {
            int channel_id = _track->track_id;
            uint8_t *data = packet._data->data();
            size_t size = packet._data->size();
            const char *medium_name = _track->sub->mediumName();
            const char *codec_name = _track->sub->codecName();

            _cb(channel_id, data, size, medium_name, codec_name, presentationTime, durationInMs);
        }
    }

    continuePlaying();
    return ;
}

Boolean MyH264or5MediaSink::continuePlaying()
{
    // xlog_dbg("continue playing\n");

    if (!fSource) {
        xlog_err("error\n");
        return False;
    }

    // xlog_dbg("before get frame\n");
    fSource->getNextFrame(_recv_buf.data(), _recv_buf.size(), 
        afterGettingFrame, this, onSourceClosure, this);
    // xlog_dbg("after get frame\n");
    
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

        xlog_dbg("sdp: {}\n", _sdp.c_str());

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
    xlog_dbg("cb: code:{}\n", resultCode);

    _live555_ret = resultCode;
    _loop = 1;

    xlog_dbg("set live555 ret: {}\n", _live555_ret);

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
    // xlog_dbg("TaskInterruptData\n");
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
        xlog_dbg("reason: {}\n", reason);
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
        xlog_dbg("set live555 ret to {}\n", _live555_ret);
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

        int verbosityLevel = 0;
        _rtsp = new MyRtspClient(*_env, _param.url.c_str(), verbosityLevel, "test", 0, this);

        xlog_dbg("connect {} using {}/{}\n", _param.url.c_str(), 
            _param.username.c_str(), 
            _param.password.c_str());

        Authenticator auth{};
        if (!_param.username.empty()) {
            auth.setUsernameAndPassword(_param.username.c_str(), _param.password.c_str());
        }
        _rtsp->sendDescribeCommand(RtspClientWrapper::continueAfterDescribe, &auth);

        // may add some timeout control here
        // eg. wait_Live555_response
        // and check for errors in previous calls
        int live555_ret = wait_live555_response(ivp4_timeout_ms);
        if (live555_ret) {
            if (live555_ret == 401) {
                xlog_err("authentication failed\n");
            } else {
                xlog_err("connect error: {}\n", live555_ret);
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
            xlog_err("create rtsp session failed: {}\n",
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
                xlog_err("rtp subsession {}/{} failed: {}\n", 
                    sub->mediumName(), sub->codecName(),
                    _env->getResultMsg());
                continue;
            }

            xlog_dbg("rtp subsesion: {}/{}\n", 
                sub->mediumName(), sub->codecName());

            if (sub->rtcpIsMuxed()) {
                xlog_dbg("client port: {}\n", sub->clientPortNum());
            } else {
                xlog_dbg("client ports: {}-{}\n", sub->clientPortNum(),
                    sub->clientPortNum() + 1);
            }

            int stream_out_going = 0;
            int stream_using_tcp = 1;
            int stream_force_multicast = 0;
            _rtsp->sendSetupCommand(*sub, RtspClientWrapper::default_live555_callback, 
                stream_out_going, stream_using_tcp, stream_force_multicast);

            ret = wait_live555_response();
            if (ret) {
                xlog_err("ret: {}\n", ret);
                break;
            }

            if (!sub->readSource()) {
                xlog_war("sub read failed\n");
                continue;
            }

            if (sub->rtcpInstance()) {
                sub->rtcpInstance()->setByeHandler(nullptr, nullptr);
            }

            auto track = std::make_shared<Track>();
            track->sub = sub;
            track->buf.resize(video_recv_buf);
            track->wrapper = this;
            track->track_id = static_cast<int>(_tracks.size());

            // refer to playCommon.cpp for examples.
            
            if (strcmp(sub->mediumName(), "video") == 0) {
                if ( (strcmp(sub->codecName(), "H264") == 0)
                    || (strcmp(sub->codecName(), "H265") == 0) ) {
                    xlog_dbg("create sink for: {}/{}\n", sub->mediumName(), sub->codecName());
                    sub->sink = MyH264or5MediaSink::createNew(*_env, track, _param.cb);
                }
            } else if (strcmp(sub->mediumName(), "audio") == 0) {
                if (strcmp(sub->codecName(), "MPEG4-GENERIC") == 0) {
                    xlog_dbg("create sink for: {}/{}\n", sub->mediumName(), sub->codecName());
                    FramedFilter *adtsFramer = ADTSAudioStreamDiscreteFramer::createNew(*_env, 
                        sub->readSource(), sub->fmtp_config());
                    sub->addFilter(adtsFramer);
                    sub->sink = MySink::createNew(*_env, track, _param.cb);
                }
            }
            
            if (!sub->sink) {
                xlog_err("no sutable sink, or create sink failed: {}/{}\n",
                    sub->mediumName(), sub->codecName());
                break;
            }
            sub->miscPtr = _rtsp;

            sub->sink->startPlaying(*sub->readSource(), afterPlaying, track.get());
            if (sub->rtcpInstance()) {
                sub->rtcpInstance()->setByeWithReasonHandler(byeHandler, track.get());
            }

            _tracks.push_back(track);
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
            xlog_err("rtsp play failed: {}\n", _env->getResultMsg());
            error_flag = 1;
            break;
        }

        auto start_time =_ms->playStartTime();
        auto end_time = _ms->playEndTime();

        xlog_inf("play: starttime={}, endtime={}\n", start_time, end_time);
    } while (0);

    return error_flag ? -1 : 0;
}

int RtspClientWrapper::demux()
{
    do {
        auto task = _scheduler->scheduleDelayedTask(300 * 1000, TaskInterruptData, this);
        _loop = 0;
        _scheduler->doEventLoop(&_loop);
        _scheduler->unscheduleDelayedTask(task);
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

        while (!_close_flag) {
            demux();
        }

        xlog_dbg("demux end, closing\n");

    } while (0);

    close_imp();

    _close_fin_flag = 1;

    xlog_dbg("close ok\n");

    return ;
}

int RtspClientWrapper::open(rtsp_client_param const *param)
{
    auto trd_func = [this](){
        return this->trd();
    };

    _param.url = param->url;
    _param.username = param->username;
    _param.password = param->password;
    _param.cb = param->cb;

    _trd = std::make_shared<std::thread>(trd_func);
    
    return 0;
}

int RtspClientWrapper::close_imp()
{
    do {

        int some_subsessions_active = false;

        for (size_t i = 0; i < _tracks.size(); ++i) {
            auto & track = _tracks[i];
            if (track) {
                if (track->sub) {
                    if(track->sub->sink) {
                        Medium::close(track->sub->sink);
                        track->sub->sink = nullptr;
                    }
                    if (track->sub->rtcpInstance()) {
                        track->sub->rtcpInstance()->setByeHandler(nullptr, nullptr);
                    }
                    some_subsessions_active = true;
                }

            }
        }

        if (some_subsessions_active) {
            if (_rtsp) {
                xlog_dbg("sending teardown command\n");
                _rtsp->sendTeardownCommand(*_ms, nullptr);
            }
        }

        if (_ms) {
            Medium::close(_ms);
            _ms = nullptr;
        }

        if (_rtsp) {
            RTSPClient::close(_rtsp);
            _rtsp = nullptr;
        }

        if (_env) {
            _env->reclaim();
        }

        if (_scheduler) {
            delete _scheduler;
            _scheduler = nullptr;
        }

    } while (0);

    return 0;
}

int RtspClientWrapper::close()
{
    _close_flag = true;
    xlog_dbg("wait close fin\n");
    while (1) {
        if (_close_fin_flag) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    xlog_dbg("wait close fin done\n");

    xlog_dbg("before join\n");
    if (_trd->joinable()) {
        _trd->join();
    }
    xlog_dbg("join end\n");

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

        ctx->close();
        
        delete ctx;
        ctx = nullptr;
    } while (0);
    
    return 0;
}