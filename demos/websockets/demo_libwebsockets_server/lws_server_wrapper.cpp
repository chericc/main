#include "lws_server_wrapper.h"

#include <thread>

#include <string.h>
#include <sys/prctl.h>

#include "lws_server.hpp"
// #include "fw/util/app-log.h"
#include "xlog.h"

#include "lws_server_config.h"

struct LwsServerWrapperClientInfo {
    LwsServerClient* client;
    unsigned int id;
};

struct LwsServerWrapperCtx {
    LwsServerWrapperParam param_copy;

    std::shared_ptr<LwsServer> server;

    mutex mutex_client_id_gen;
    unsigned int client_id_gen;

    bool worker_break_flag = false;
    std::shared_ptr<std::thread> lws_serv_wrap_trd_worker;

    std::recursive_mutex mutex_client_connected;
    std::vector<LwsServerWrapperClientInfo> vec_client_connected; // 这里只是缓存拿到的client，失效的client需要给server销毁
};

struct LwsServerWrapperCtx s_ctx;

namespace {

LWS_SERVER_WRAPPER_CLIENT_TYPE get_client_type(LwsServerClient *client)
{
    LWS_SERVER_WRAPPER_CLIENT_TYPE client_type = LWS_SERVER_WRAPPER_CLIENT_TYPE_NONE;
    string client_uri = client->uri();
    if (client_uri.find(s_ctx.param_copy.screen_lws_uri) != client_uri.npos) {
        client_type = LWS_SERVER_WRAPPER_CLIENT_TYPE_SCREEN;
    } else if (client_uri.find(s_ctx.param_copy.app_lws_uri) != client_uri.npos) {
        client_type = LWS_SERVER_WRAPPER_CLIENT_TYPE_APP;
    } else {
        xlog_dbg("client type not recognized(uri:{})", client_uri.c_str());
    }
    return client_type;
}

void on_client_disconnect(LwsServerClient *client)
{
    do {
        if (!client) {
            xlog_err("null client");
            break;
        }

        UniRecuLock lock(s_ctx.mutex_client_connected);
        bool found_flag = false;
        string client_info;
        for (auto it = s_ctx.vec_client_connected.begin(); it != s_ctx.vec_client_connected.end();) {
            if (it->client == client) {
                found_flag = true;
                xlog_dbg("got client, destroy it");
                
                if (s_ctx.param_copy.cb_on_event) {
                    LWS_SERVER_WRAPPER_CLIENT_TYPE client_type = get_client_type(client);
                    s_ctx.param_copy.cb_on_event(it->id, client_type, LWS_SERVER_WRAPPER_CLIENT_EVENT_DISCONNECTED);
                } else {
                    xlog_err("cb on event is null");
                }
                client_info = client->info();
                s_ctx.server->destroyClient(client);
                s_ctx.vec_client_connected.erase(it);
            } else {
                ++it;
            }
        }

        // note: client is already freed 
        xlog_dbg("client disconnected: {}({}), num:{}", 
            (void*)client, client_info.c_str(), s_ctx.vec_client_connected.size());

        if (!found_flag) {
            xlog_err("client not found");
        }
    } while (0);
    
    return ;
}

void on_client_recv_data(LwsServerClient *client, const void *data, size_t size)
{
    // xlog_dbg("on recv data");
    do {
        if (!client) {
            xlog_err("null clinet");
            break;
        }

        // xlog_dbg("client recv data: {}({})", client, client->info().c_str());

        UniRecuLock lock(s_ctx.mutex_client_connected);
        bool found_flag = false;
        for (size_t i = 0; i < s_ctx.vec_client_connected.size(); ++i) {
            if (s_ctx.vec_client_connected[i].client == client) {
                found_flag = true;

                if (s_ctx.param_copy.cb_on_msg) {
                    LWS_SERVER_WRAPPER_CLIENT_TYPE client_type = get_client_type(client);
                    s_ctx.param_copy.cb_on_msg(s_ctx.vec_client_connected[i].id, client_type, data, size);
                } else {
                    xlog_dbg("cb_on_msg is null");
                }
            }
        }

        if (!found_flag) {
            xlog_err("recv data while client not found");
        }
    } while (0);
    return ;
}

void lws_serv_wrap_trd_worker()
{
    // todo
    // set trd name
    prctl(PR_SET_NAME, "lws_serv_wrap");

    xlog_dbg("begin");
    while (!s_ctx.worker_break_flag) {
        // xlog_dbg("begin wait client to connect");
        auto client = s_ctx.server->waitClient(5 * 1000);

        if (client) {
            LwsServerWrapperClientInfo cli_info = {};
            cli_info.client = client;

            {
                UniLock lock(s_ctx.mutex_client_id_gen);
                cli_info.id = s_ctx.client_id_gen;
                ++s_ctx.client_id_gen;
            }
            
            {
                UniRecuLock lock(s_ctx.mutex_client_connected);
                if (s_ctx.vec_client_connected.size() > LWS_SERVER_MAX_CLIENT_NUM) {
                    xlog_err("vec num over({})", s_ctx.vec_client_connected.size());
                    continue;
                }
                s_ctx.vec_client_connected.push_back(cli_info);
            }

            // 必须先通知上线，再通知离线
            if (s_ctx.param_copy.cb_on_event) {
                LWS_SERVER_WRAPPER_CLIENT_TYPE client_type = get_client_type(client);
                s_ctx.param_copy.cb_on_event(cli_info.id, client_type, LWS_SERVER_WRAPPER_CLIENT_EVENT_CONNECTED);
            } else {
                xlog_err("cb_on_event is null");
            }

            LwsServerClient::ClientCallbacks cbs = {};
            cbs.cbOnDisconnected = on_client_disconnect;
            cbs.cbOnData = on_client_recv_data;

            // NOTE:
            // 可能在配置回调的时候直接触发回调
            // 因此必须先把client放到缓存中
            client->setNotifyCb(cbs);
            xlog_dbg("got connected client({}): num={}", client->info().c_str(), s_ctx.vec_client_connected.size());
        } else {
            // xlog_dbg("no client");
        }

        // check connected clients
    }
    xlog_dbg("end");
}

}

int lws_server_wrapper_init(const LwsServerWrapperParam *param)
{
    int error_flag = 0;

    xlog_dbg("init in");

    do {
        int ret = 0;

        s_ctx.param_copy = *param;

        LwsServerParam server_param = {};
        server_param.app_uri = param->app_lws_uri;
        server_param.screen_uri = param->screen_lws_uri;
        server_param.port = param->port;
        s_ctx.server = std::make_shared<LwsServer>(server_param);

        ret = s_ctx.server->init();
        if (ret < 0) {
            xlog_err("init failed");
            error_flag = 1;
            break;
        }

        s_ctx.worker_break_flag = false;
        s_ctx.lws_serv_wrap_trd_worker = std::make_shared<std::thread>(lws_serv_wrap_trd_worker);

        xlog_dbg("app init successful");
    } while (0);

    return error_flag ? -1 : 0;
}

int lws_server_wrapper_deinit()
{
    xlog_dbg("deinit in");
    do {
        s_ctx.worker_break_flag = true;
        if (s_ctx.lws_serv_wrap_trd_worker && s_ctx.lws_serv_wrap_trd_worker->joinable()) {
            xlog_dbg("wait for trd worker");
            s_ctx.lws_serv_wrap_trd_worker->join();
            s_ctx.lws_serv_wrap_trd_worker.reset(); // free thread
            xlog_dbg("wait for trd worker end");
        }

        if (s_ctx.server) {
            xlog_dbg("destroy server begin");
            s_ctx.server.reset(); // free server
            xlog_dbg("destroy server end");
        }
        
    } while (0);
    return 0;
}

int lws_server_wrapper_send_msg(unsigned int clientID, const void *data, size_t size)
{
    bool error_flag = false;

    // xlog_dbg("send msg: {}", (const char *)data);

    do {
        bool found_flag = false;

        UniRecuLock lock(s_ctx.mutex_client_connected);
        for (size_t i = 0; i < s_ctx.vec_client_connected.size(); ++i) {
            if (s_ctx.vec_client_connected[i].id == clientID) {
                found_flag = true;
                int ret = s_ctx.vec_client_connected[i].client->sendData(data, size);
                if (ret != 0) {
                    error_flag = true;
                }
                break;
            }
        }

        if (!found_flag) {
            xlog_err("client not found for id[{}]", clientID);
            error_flag = true;
        }
    } while (0);

    return error_flag ? -1 : 0;
}