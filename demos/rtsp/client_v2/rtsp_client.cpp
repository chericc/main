#include "rtsp_client.h"

#include <thread>
#include <memory>
#include <string>
#include <vector>

#include "RTSPClient.hh"
#include "BasicUsageEnvironment.hh"

#include "xlog.h"

using std::string;

namespace {

constexpr int audio_recv_buf = (100 * 1000);
constexpr int video_recv_buf = (2000 * 1000);
constexpr int ivp4_timeout_ms = (1000);

class RtspClientWrapper;

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
    int open(string const& url);
    int close();

private:
    int connect();
    int session_setup();
    int play();
private:
    static void continueAfterDescribe(RTSPClient *client, int resultCode, char *resultString);
    static void default_live555_callback(RTSPClient *client, int resultCode, char *resultString);
    static void TaskInterruptRTSP(void *user);
    void continueAfterDescribe(int resultCode, char *resultString);
    void default_live555_callback(int resultCode, char *resultString);
    void TaskInterruptRTSP();

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

    struct param {
        std::string url;
        std::string username;
        std::string password;
    };
    param _param;
};

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
    } while (0);
}

void RtspClientWrapper::default_live555_callback(RTSPClient *client, int resultCode, char *resultString)
{
    auto cli = reinterpret_cast<MyRtspClient*>(client);
    return cli->wrapper()->default_live555_callback(resultCode, resultString);
}

void RtspClientWrapper::default_live555_callback(int resultCode, char *resultString)
{
    return ;
}

void RtspClientWrapper::TaskInterruptRTSP(void *user)
{
    auto wrapper = reinterpret_cast<RtspClientWrapper*>(user);
    return wrapper->TaskInterruptRTSP();
}

void RtspClientWrapper::TaskInterruptRTSP()
{
    xlog_dbg("interrupt rtsp loop\n");
    _loop = static_cast<char>(0xff);
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

static void continueAfterDescribe(RTSPClient *client, int resultCode, char *resultString)
{

}

int RtspClientWrapper::connect()
{
    int success = 0;
    do {
        if (_rtsp) {
            xlog_err("inner error, not null\n");
            break;
        }

        _rtsp = new MyRtspClient(*_env, _param.url.c_str(), 255, "test", 0, this);

        Authenticator auth;
        auth.setUsernameAndPassword(_param.password.c_str(), _param.password.c_str());
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
    int track_num = 0;

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

        iter = new MediaSubsessionIterator(*_ms);
        while (1) {
            sub = iter->next();
            if (!sub) {
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

            if (!sub->readSource()) {
                xlog_war("sub read failed\n");
                continue;
            }

            if (sub->rtcpInstance()) {
                sub->rtcpInstance()->setByeHandler(nullptr, nullptr);
            }
            ++track_num;
        }
    } while (0);

    if (iter) {
        delete iter;
        iter = nullptr;
    }

    if (track_num <= 0) {
        xlog_err("no track\n");
        error_flag = true;
    }

    // todo: error handling
    return error_flag ? -1 : 0;
}

int RtspClientWrapper::play()
{
    do {
        _rtsp->sendPlayCommand(*_ms, default_live555_callback);
        if (!wait_live555_response()) {
            xlog_err("rtsp play failed: %s\n", _env->getResultMsg());
            break;
        }

        auto start_time =_ms->playStartTime();
        auto end_time = _ms->playEndTime();

        xlog_inf("play: starttime=%g, endtime=%g\n", start_time, end_time);
    } while (0);
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

        if (session_setup()) {
            xlog_err("nothing to play: no track\n");
            break;
        }

        if (play()) {
            xlog_err("play failed\n");
            break;
        }

        _env->taskScheduler().doEventLoop(&_loop);
    } while (0);
}

int RtspClientWrapper::open(string const& url)
{
    auto trd_func = [this](){
        return this->trd();
    };

    _param.url = url;

    _trd = std::make_shared<std::thread>(trd_func);
    
    
    return 0;
}

}

rtsp_client_obj rtsp_client_start(rtsp_client_param const* param)
{
    RtspClientWrapper *ctx = nullptr;

    do {
        ctx = new RtspClientWrapper{};
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