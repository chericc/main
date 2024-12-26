#include "lws_server_wrapper.h"

#include <thread>

#include "lws_server.hpp"
// #include "fw/util/app-log.h"
#include "xlog.hpp"

#define APP_LOGW xlog_war
#define APP_LOGE xlog_err


struct LwsServerWrapperCtx {
    std::shared_ptr<LwsServer> server;

    bool worker_break_flag = false;
    std::shared_ptr<std::thread> trd_worker;
};

struct LwsServerWrapperCtx s_ctx;

namespace {

void trd_worker()
{
    // todo
    // set trd name

    while (!s_ctx.worker_break_flag) {
        auto client = s_ctx.server->waitClient(5000);

    }
}

}

int lws_server_wrapper_init(LwsServerWrapperParam param)
{
    int error_flag = 0;

    APP_LOGW("init in\n");

    do {
        int ret = 0;

        LwsServerParam param = {};
        param.app_uri = "/api/msg";
        param.screen_uri = "/api/haima";
        param.port = 7681;
        s_ctx.server = std::make_shared<LwsServer>(param);

        ret = s_ctx.server->init();
        if (ret < 0) {
            APP_LOGE("init failed\n");
            error_flag = 1;
            break;
        }

        s_ctx.worker_break_flag = false;
        s_ctx.trd_worker = std::make_shared<std::thread>(trd_worker);

        APP_LOGW("app init successful\n");
    } while (0);

    return error_flag ? -1 : 0;
}

int lws_server_wrapper_deinit()
{
    APP_LOGW("deinit in\n");
    do {
        if (s_ctx.server) {
            s_ctx.server = nullptr;
        }
        
    } while (0);
    return 0;
}