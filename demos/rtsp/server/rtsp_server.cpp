#include "rtsp_server.hpp"

#include <memory>
#include <thread>

#include <BasicUsageEnvironment.hh>
#include <GroupsockHelper.hh>
#include <liveMedia.hh>

#include "my_rtsp_server.hpp"
#include "xlog.h"

namespace {

struct rtsp_server_ctx {
    std::shared_ptr<std::thread> trd;

    struct rtsp_server_param param = {};

    EventLoopWatchVariable loopVariable;
};


void rtsp_server_trd(struct rtsp_server_ctx *ctx)
{
    TaskScheduler *scheduler = nullptr;
    UsageEnvironment *env = nullptr;
    UserAuthenticationDatabase *authDB = nullptr;
    RTSPServer *rtspServer = nullptr;

    do {
        scheduler = BasicTaskScheduler::createNew();
        env = BasicUsageEnvironment::createNew(*scheduler);

        xlog_dbg("server: [%s/%s], port=%d\n", 
            ctx->param.username, ctx->param.password,
            ctx->param.port);

        if (strlen(ctx->param.username) > 0) {
            authDB = new UserAuthenticationDatabase;
            authDB->addUserRecord(ctx->param.username, ctx->param.password);
        }

        portNumBits port = ctx->param.port;
        rtspServer = MyRTSPServer::createNew(*env, port, authDB);
    
        if (nullptr == rtspServer) {
            xlog_err("create rtsp server failed\n");
            break;
        }

        xlog_dbg("rtsp server started\n");
        env->taskScheduler().doEventLoop(&ctx->loopVariable);
        xlog_dbg("rtsp server stopped\n");
    } while (false);

    if (rtspServer != nullptr) {
        Medium::close(rtspServer);
        rtspServer = nullptr;
    }

    if (authDB != nullptr) {
        delete authDB;
        authDB = nullptr;
    }

    if (env != nullptr) {
        if (!static_cast<bool>(env->reclaim())) {
            xlog_err("delete env failed\n");
        }
    }

    if (scheduler != nullptr) {
        delete scheduler;
        scheduler = nullptr;
    }
}

}

rtsp_server_handle rtsp_server_start(struct rtsp_server_param const* param)
{
    struct rtsp_server_ctx *ctx = nullptr;
    do {

        OutPacketBuffer::maxSize = 500 * 1024;

        ctx = new rtsp_server_ctx{};
        ctx->param = *param;
        ctx->loopVariable = 0;
        ctx->trd = std::make_shared<std::thread>(rtsp_server_trd, ctx);

    } while (false);

    return reinterpret_cast<rtsp_server_handle>(ctx);
}

int rtsp_server_destroy(rtsp_server_handle handle)
{
    return 0;
}