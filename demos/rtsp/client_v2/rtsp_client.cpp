#include "rtsp_client.h"

#include <thread>
#include <memory>
#include <string>
#include <vector>

#include "RTSPClient.hh"
#include "BasicUsageEnvironment.hh"

#include "xlog.h"

using std::string;

struct rtsp_client_ctx {
    std::string progName;
    rtsp_client_param param;

    std::shared_ptr<std::thread> trd;
    EventLoopWatchVariable eventLoopWatchVariable;
    RTSPClient *rtsp_client = nullptr;
};

class RtspClientImp : public RTSPClient
{
public:
    struct param {
        string url;
        int verbosity;
        string app_name;
        int port;
    };
    static RtspClientImp *createNew();
    static int destroy(RtspClientImp *);
protected:
    RtspClientImp(UsageEnvironment &env, param *param);
    ~RtspClientImp();

private:
    param *_param;
};

class RtspClientWrapper 
{
public:
    RtspClientWrapper();
    ~RtspClientWrapper();
    int openUrl(string const& url);
private:
    UsageEnvironment *_env = nullptr;
    BasicTaskScheduler *_scheduler = nullptr;
    RtspClientImp *_client = nullptr;
};

RtspClientImp *RtspClientImp::createNew()
{

}

int RtspClientImp::destroy(RtspClientImp *client)
{
    if (client) {
        // todo
    }
}

RtspClientImp::RtspClientImp(UsageEnvironment &env, param *param)
    : RTSPClient(env, param->url.c_str(), param->verbosity, 
        param->app_name.c_str(), param->port, -1)
{
}

RtspClientImp::~RtspClientImp()
{
}

RtspClientWrapper::RtspClientWrapper()
{
    _scheduler = BasicTaskScheduler::createNew();
    _env = BasicUsageEnvironment::createNew(*_scheduler);
}

RtspClientWrapper::~RtspClientWrapper()
{

}

int RtspClientWrapper::openUrl(string const& url)
{
    auto client = RtspClientImp::createNew()
}

void trd_func(rtsp_client_ctx *ctx)
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
    RtspClientWrapper *ctx = nullptr;

    do {
        ctx = new RtspClientWrapper{};
        ctx->param = *param;

        ctx->eventLoopWatchVariable = 0;

        ctx->trd = std::make_shared<std::thread>(trd_func, ctx);
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

        ctx->eventLoopWatchVariable = 1;
        if (ctx->trd->joinable()) {
            ctx->trd->join();
        }

        delete ctx;
        ctx = nullptr;
    } while (0);
    
    return 0;
}