#include "lws_server.hpp"

#include <sys/prctl.h>

// #include "fw/util/app-log.h"
#include "xlog.h"

#include "lws_server_config.h"

// #define xlog_dbg xlog_dbg
// #define xlog_err xlog_err

#define RX_BUF_SIZE (64 * 1024)  
#define PING_PACKET_SIZE    (16 * 1024)

#define FUNC_SCOPE_LOG 
// #define FUNC_SCOPE_LOG FuncScopeLog log__(__func__);

namespace {

class FuncScopeLog {
public:
    FuncScopeLog(std::string func) : _func(func) {
        xlog_dbg("{} in\n", _func.c_str());
    }
    ~FuncScopeLog() {
        xlog_dbg("{} leave\n", _func.c_str());
    }
    std::string _func;
};

struct ClientCtx {
    LwsServerClient *client;
    // std::shared_ptr<LwsServerClient> client;
};

struct HttpClientCtx {
    string path;
};

void my_lws_log_emit_t(int level, const char *line)
{
    xlog_dbg("lws log: {}", line);
}

int callback_websocket(struct lws *wsi, enum lws_callback_reasons reason,
                              void *user, void *in, size_t len) 
{
    void *user_data = lws_context_user(lws_get_context(wsi));

    LwsServer *server = reinterpret_cast<LwsServer*>(user_data);

    int ret = 0;
    ret = server->cbOnWebSocket(wsi, reason, user, in, len);

    return ret;
}

int callback_http(struct lws *wsi, enum lws_callback_reasons reason,
                              void *user, void *in, size_t len) 
{
    void *user_data = lws_context_user(lws_get_context(wsi));

    LwsServer *server = reinterpret_cast<LwsServer*>(user_data);

    int ret = 0;
    ret = server->cbOnHttp(wsi, reason, user, in, len);

    return ret;
}

}

int LwsServerClient::setNotifyCb(ClientCallbacks cbs)
{
    UniRecuLock lock(_mutex);
    do {
        if (!cbs.cbOnData || !cbs.cbOnDisconnected) {
            xlog_err("null cb\n");
            break;
        }

        _notify_cbs = cbs;
        
        if (!_msgs_received.empty()) {
            for (size_t i = 0; i < _msgs_received.size(); ++i) {
                xlog_dbg("already have received data for client(%p,%s)\n", (void*)this, _info.c_str());
                cbs.cbOnData(this, _msgs_received[i].data(), _msgs_received[i].size());
            }
            _msgs_received.clear();
        }

        if (_state == State::Disconnected) {
            xlog_dbg("already disconnected, notify directly\n");
            cbs.cbOnDisconnected(this);
        }
    } while (0);
    return 0;
}

std::string const&LwsServerClient::info()
{
    UniRecuLock lock(_mutex);
    return _info;
}

std::string const&LwsServerClient::uri()
{
    UniRecuLock lock(_mutex);
    return _uri;
}

int LwsServerClient::sendData(const void *data, size_t size)
{
    bool error_flag = false;
    UniRecuLock lock(_mutex);

    do {
        if (!_need_write_cb) {
            xlog_err("cb null\n");
            error_flag = true;
            break;
        }

        if (_msgs_to_send.size() > MAX_MSG_CACHE_NUM) {
            xlog_err("msg cache full\n");
            error_flag = true;
            break;
        }

        _msgs_to_send.emplace_back(std::vector<uint8_t>((uint8_t*)data, (uint8_t*)data + size));
        _need_write_cb();
    } while (0);

    return error_flag ? -1 : 0;
}

int LwsServerClient::withNeedWriteCb(NeedWriteCb need_write_cb)
{
    UniRecuLock lock(_mutex);
    _need_write_cb = need_write_cb;
    return 0;
}

int LwsServerClient::withInfo(std::string const& info)
{
    UniRecuLock lock(_mutex);
    _info = info;
    return 0;
}

int LwsServerClient::withUri(std::string const& uri)
{
    UniRecuLock lock(_mutex);
    _uri = uri;
    return 0;
}

int LwsServerClient::onWritable(struct lws *wsi, WriteCb cb)
{
    UniRecuLock lock(_mutex);
    int ret = 0;
    for (size_t i = 0; i < _msgs_to_send.size(); ++i) {
        ret = cb(wsi, _msgs_to_send[i].data(), _msgs_to_send[i].size(), LWS_WRITE_TEXT);
    }
    _msgs_to_send.clear();
    return ret;
}

int LwsServerClient::onReceive(const void *data, size_t size)
{
    UniRecuLock lock(_mutex);
    // xlog_dbg("on receive: {}(size:{})\n", (const char *)data, size);
    if (_notify_cbs.cbOnData) {
        _notify_cbs.cbOnData(this, data, size);
    } else {
        xlog_err("cbOnData null\n");

    }
    return 0;
}

int LwsServerClient::onDisconnected()
{
    UniRecuLock lock(_mutex);
    _state = State::Disconnected;
    if (_notify_cbs.cbOnDisconnected) {
        _notify_cbs.cbOnDisconnected(this);
    } else {
        xlog_err("disconnect cb is null\n");
    }
    return 0;
}

LwsServer::LwsServer(const LwsServerParam& param)
 : _param(param)
{
    FUNC_SCOPE_LOG
    UniRecuLock lock(_mutex);

    _s_msg_data = (msg_data_t*)malloc(sizeof(*_s_msg_data));
    if (!_s_msg_data) {
        xlog_err("malloc failed\n");
    }
}

LwsServer::~LwsServer()
{
    FUNC_SCOPE_LOG
    UniRecuLock lock(_mutex);
    _deinit();

    if (_s_msg_data) {
        free(_s_msg_data);
        _s_msg_data = nullptr;
    }
}

int LwsServer::init()
{
    FUNC_SCOPE_LOG
    UniRecuLock lock(_mutex);
    int ret = _init();
    return ret;
}

int LwsServer::deinit()
{
    FUNC_SCOPE_LOG
    UniRecuLock lock(_mutex);
    int ret = _deinit();
    return ret;
}

LwsServerClient* LwsServer::waitClient(int timeout_ms)
{
    FUNC_SCOPE_LOG
    UniRecuLock lock(_mutex);
    auto client = _waitClient(lock, timeout_ms);
    return client;
}

int LwsServer::destroyClient(LwsServerClient *client)
{
    FUNC_SCOPE_LOG
    UniRecuLock lock(_mutex);
    int ret = _restoreClient(client);
    return ret;
}

int LwsServer::cbOnWebSocket(struct lws *wsi, enum lws_callback_reasons reason,
                            void *user, void *in, size_t len)
{
    int ret = 0;

    ClientCtx* per_conn_cctx = reinterpret_cast<ClientCtx*>(user);

    switch (reason) {
        case LWS_CALLBACK_WSI_CREATE: {
            xlog_dbg("wsi create\n");
            break;
        }
        case LWS_CALLBACK_WSI_DESTROY: {
            xlog_dbg("wsi destroy\n");
            break;
        }
        case LWS_CALLBACK_PROTOCOL_INIT: {
            xlog_dbg("protocol init\n");
            // per_conn_cctx
            break;
        }
        case LWS_CALLBACK_PROTOCOL_DESTROY: {
            xlog_dbg("protocol destroy\n");
            break;
        }
        case LWS_CALLBACK_ESTABLISHED: {
            xlog_dbg("established\n");

            per_conn_cctx->client = _produceClient();
            if (per_conn_cctx->client) {

                auto needWriteCb = [wsi](){
                    // 注意wsi生命周期（捕获的wsi是client连接时的wsi）
                    // client在disconnected之后，不能调用
                    // 
                    // xlog_dbg("trigger writable\n");
                    lws_callback_on_writable(wsi);
                };

                char name[64] = {};
                lws_get_peer_simple(wsi, name, sizeof(name));

                char uri[64] = {};
                if (lws_hdr_copy(wsi, uri, sizeof(uri), WSI_TOKEN_GET_URI) < 0) {
                    xlog_err("get uri failed\n");
                }

                char client_info[256] = {};
                snprintf(client_info, sizeof(client_info), "name:%s,path:%s", name, uri);

                xlog_dbg("name: {}, uri: {}\n", name, uri);

                per_conn_cctx->client->withUri(uri);
                per_conn_cctx->client->withInfo(client_info);
                per_conn_cctx->client->withNeedWriteCb(needWriteCb);
                
                _onClientConnected(per_conn_cctx->client);
            } else {
                xlog_err("produce client failed\n");
            }
            break;
        }
        case LWS_CALLBACK_SERVER_WRITEABLE: {
            // xlog_dbg("server writable\n");
            if (per_conn_cctx->client) {
                auto write_cb = [this](struct lws *wsi, const void *data, size_t size, lws_write_protocol protocol) {
                    return _writeData(wsi, data, size, protocol);
                };
                per_conn_cctx->client->onWritable(wsi, write_cb);
            } else {
                xlog_err("writable no client\n");
            }
            break;
        }
        case LWS_CALLBACK_RECEIVE: {
            // xlog_dbg("receive msg: <{}>\n", (const char*)in);
            // xlog_dbg("receive msg\n");
            if (per_conn_cctx->client) {
                per_conn_cctx->client->onReceive(in, len);
            } else {
                xlog_err("writable no client\n");
            }
            break;
        }
        case LWS_CALLBACK_CLOSED: {
            xlog_dbg("close, client is: %p(%s)\n", (void*)per_conn_cctx->client, 
                per_conn_cctx->client->info().c_str());

            if (per_conn_cctx->client) {
                _onClientDisconnected(per_conn_cctx->client);
                // _restoreClient(per_conn_cctx->client);
            } else {
                xlog_dbg("client is null\n");
            }

            // lws_close_reason(wsi, LWS_CLOSE_STATUS_NORMAL, (unsigned char*)"", 0);
            // lws_close_free_wsi(wsi, LWS_CLOSE_STATUS_NORMAL, "server");
            ret = -1;
            break;
        }
        default: {
            break;
        }
    }
    return ret;
}

int LwsServer::cbOnHttp(struct lws *wsi, enum lws_callback_reasons reason,
                            void *user, void *in, size_t len)
{
    // int ret = 0;
    HttpClientCtx *per_conn_cctx = reinterpret_cast<HttpClientCtx*>(user);

    switch (reason) {
        case LWS_CALLBACK_HTTP: {
            xlog_dbg("LWS_CALLBACK_HTTP, in: {}\n", (const char *)in);

            per_conn_cctx->path = (const char *)in;
            break;
        }
        case LWS_CALLBACK_CLOSED_HTTP: {
            xlog_dbg("LWS_CALLBACK_CLOSED_HTTP\n");
            break;
        }
        default:
        {
            break;
        }
    }

    return lws_callback_http_dummy(wsi, reason, user, in, len);
}

int LwsServer::_init()
{
    FUNC_SCOPE_LOG
    bool error_flag = false;

    do {
        auto cb_trd = [this]() { return this->_trdWorkder(); };

        if (_trd_worker) {
            xlog_dbg("worker already started\n");
            error_flag = 1;
            break;
        }

        _trd_interrupted_flag = false;
        _trd_worker = std::make_shared<std::thread>(cb_trd);
    } while (0);

    return error_flag ? -1 : 0;
}

void LwsServer::_trdWorkder()
{
    prctl(PR_SET_NAME, "lws_server");

    bool error_flag = false;
	struct lws_context *context = nullptr;
	struct lws_context_creation_info info = {};

    do {
        lws_set_log_level(LLL_ERR | LLL_WARN, my_lws_log_emit_t);

        info.port = _param.port;
        lws_protocols protocols[] = {
            {
                "http",
                callback_http, sizeof(HttpClientCtx), 
                RX_BUF_SIZE, 0, NULL, 0
            },
            { "my-test-lws-server", 
                callback_websocket, sizeof(ClientCtx), 
                RX_BUF_SIZE, 0, NULL, 0 },
            LWS_PROTOCOL_LIST_TERM,
        };
        info.protocols = protocols;
        // info.options = LWS_SERVER_OPTION_SKIP_SERVER_CANONICAL_NAME 
        //      | LWS_SERVER_OPTION_VALIDATE_UTF8 | LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE;
        info.options = 0;
        info.user = this;
        xlog_dbg("this: %p\n", (void*)this);
        context = lws_create_context(&info);
        if (!context) {
            xlog_err("lws_create_context failed\n");
            error_flag = true;
            break;
        }
        _lws_context = context;

        xlog_dbg("lws service begin\n");
        int ret = 0;
        while (ret >= 0 && !_trd_interrupted_flag) {
            ret = lws_service(context, 0);
        }
        xlog_dbg("lws service end\n");
    } while (0);

    if (error_flag) {
        xlog_err("error in trd\n");
    }

    if (context) {
        lws_context_destroy(context);
        context = nullptr;
    }

    return ;
}

int LwsServer::_deinit()
{
    FUNC_SCOPE_LOG
    int error_flag = 0;
    do {
        if (!_trd_worker) {
            xlog_dbg("trd_worker not started\n");
            error_flag = 1;
            break;
        }
        if (!_trd_worker->joinable()) {
            xlog_err("trd not joinable\n");
            error_flag = 1;
            break;
        }

        if (_lws_context) {
            xlog_dbg("cancel service\n");
            lws_cancel_service(_lws_context);
            _lws_context = nullptr;
        }

        _trd_interrupted_flag = true;

        xlog_dbg("join start\n");
        _trd_worker->join();
        xlog_dbg("join fin\n");
        _trd_worker = nullptr;

        UniLock lock(_mutex_allocated_client_trace_pool);
        if (!_allocated_client_trace_pool.empty()) {
            xlog_err("deinit while pool not empty\n");
            for (size_t i = 0; i < _allocated_client_trace_pool.size(); ++i) {
                xlog_err("delete client: {}\n", i);
                delete _allocated_client_trace_pool[i];
                _allocated_client_trace_pool[i] = nullptr;
            }
        }
    } while (0);

    return error_flag ? -1 : 0;
}

LwsServerClient* LwsServer::_waitClient(UniRecuLock &call_lock, int timeout_ms)
{
    FUNC_SCOPE_LOG

    LwsServerClient *client_ret = nullptr;

    do {
        UniLock lock(_mutex_client_connected);
        if (!_client_connected.empty()) {
            xlog_dbg("got on directly\n");
            client_ret = _client_connected.back();
            _client_connected.pop_back();
            break;
        }

        // xlog_dbg("no client, need wait\n");
        call_lock.unlock();
        _cond_client_connected_changed.wait_for(lock, 
            std::chrono::milliseconds(timeout_ms));
        call_lock.lock();
        // xlog_dbg("wait end\n");
        
        if (!_client_connected.empty()) {
            client_ret = _client_connected.back();
            _client_connected.pop_back();
            break;
        }
    } while (0);

    return client_ret;
}

LwsServerClient *LwsServer::_produceClient()
{
    FUNC_SCOPE_LOG
    UniLock lock(_mutex_allocated_client_trace_pool);
    LwsServerClient *client_ret = nullptr;
    do {
        if (_allocated_client_trace_pool.size() > LWS_SERVER_MAX_CLIENT_NUM) {
            xlog_err("client num over(num={})\n", _allocated_client_trace_pool.size());
            break;
        }

        auto client = new LwsServerClient();
        _allocated_client_trace_pool.push_back(client);
        client_ret = client;
    } while (0);

    xlog_dbg("pool size:{}\n", _allocated_client_trace_pool.size());
    return client_ret;
}

int LwsServer::_restoreClient(LwsServerClient* client)
{
    FUNC_SCOPE_LOG
    bool error_flag = false;
    UniLock lock(_mutex_allocated_client_trace_pool);
    do {
        bool find_flag = false;

        for (auto it = _allocated_client_trace_pool.begin(); it != _allocated_client_trace_pool.end(); ) {
            if (*it == client) {
                xlog_dbg("deleting client: {}\n", (*it)->info().c_str());
                delete *it;
                it = _allocated_client_trace_pool.erase(it); // delete this client
                find_flag = true;
            } else {
                ++it;
            }
        }

        if (!find_flag) {
            xlog_err("client not found, may leak\n");
            error_flag = true;
        }
    } while (0);

    xlog_dbg("pool size:{}\n", _allocated_client_trace_pool.size());
    return error_flag ? -1 : 0;
}

void LwsServer::_onClientConnected(LwsServerClient *client)
{
    FUNC_SCOPE_LOG
    do {
        if (!client) {
            xlog_err("client is null\n");
            break;
        }

        UniLock lock(_mutex_client_connected);

        if (_client_connected.size() > LWS_SERVER_MAX_CLIENT_NUM) {
            xlog_err("inner error, client num over\n");
            break;
        }

        _client_connected.push_back(client);
        _cond_client_connected_changed.notify_one();
        
        xlog_dbg("connected client num:{}\n", _client_connected.size());
    } while (0);
    return ;
}

void LwsServer::_onClientDisconnected(LwsServerClient *client)
{
    FUNC_SCOPE_LOG
    do {
        if (!client) {
            xlog_err("client is null\n");
            break;
        }

        // notify client of disconnected state
        client->onDisconnected();
    } while (0);

    xlog_dbg("connected client num:{}\n", _client_connected.size());
    return ;
}

int LwsServer::_writeData(struct lws *wsi, const void *data, size_t size, lws_write_protocol protocol)
{
    FUNC_SCOPE_LOG
    bool error_flag = false;
    do {
        if (!_s_msg_data) {
            xlog_err("buffer malloc failed\n");
            error_flag = true;
            break;
        }

        size_t min_size = std::min(sizeof(_s_msg_data->payload), size);
        if (min_size != size) {
            xlog_err("buffer too small\n");
            error_flag = true;
            break;
        }

        memcpy(_s_msg_data->payload, data, min_size);
        int ret = lws_write(wsi, (unsigned char*)_s_msg_data->payload, min_size, protocol);
        if (ret != (int)min_size) {
            xlog_err("lws_write failed\n");
            error_flag = true;
            break;
        }

        // xlog_dbg("sent: {} bytes\n", ret);
    } while (0);

    return error_flag ? -1 : 0;
}