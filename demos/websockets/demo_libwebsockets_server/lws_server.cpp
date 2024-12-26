#include "lws_server.hpp"

// #include "fw/util/app-log.h"
#include "xlog.hpp"

// #define xlog_war xlog_war
// #define xlog_err xlog_err

#define RX_BUF_SIZE (64 * 1024)
#define CLIENT_MAX_NUM  2
#define PING_PACKET_SIZE    (16 * 1024)

namespace {

struct ClientCtx {
    LwsServerClient *client;
    // std::shared_ptr<LwsServerClient> client;
};

void my_lws_log_emit_t(int level, const char *line)
{
    xlog_war("lws log: %s\n", line);
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

}

int LwsServerClient::setDisconnectCb(std::function<void(void)> cbDisconnect)
{
    UniLock lock(_mutex);
    do {
        if (!cbDisconnect) {
            xlog_err("null cb\n");
            break;
        }

        _disconnectCb = cbDisconnect;
        
        if (_state == State::Disconnected) {
            xlog_war("already disconnected, notify directly\n");
            _disconnectCb();
        }
    } while (0);
    return 0;
}

std::string const&LwsServerClient::info()
{
    UniLock lock(_mutex);
    return _info;
}

int LwsServerClient::request(Request const& req, Response &resp)
{
    UniLock lock(_mutex);
    return 0;
}

int LwsServerClient::setInfo(std::string const& info)
{
    UniLock lock(_mutex);
    _info = info;
    return 0;
}

int LwsServerClient::onWritable(struct lws *wsi, std::function<WriteCb> cb)
{
    UniLock lock(_mutex);
    char msg[64] = "hello from server\n";
    int ret = cb(wsi, msg, sizeof(msg), LWS_WRITE_TEXT);
    return ret;
}

int LwsServerClient::onReceive(const void *data, size_t size)
{
    UniLock lock(_mutex);
    xlog_war("on receive: %s(size:%zu)\n", (const char *)data, size);
    return 0;
}

int LwsServerClient::onDisconnected()
{
    UniLock lock(_mutex);
    _state = State::Disconnected;
    if (_disconnectCb) {
        _disconnectCb();
    }
    return 0;
}

LwsServer::LwsServer(const LwsServerParam& param)
 : _param(param)
{
    UniLock lock(_mutex);

    _s_msg_data = (msg_data_t*)malloc(sizeof(*_s_msg_data));
    if (!_s_msg_data) {
        xlog_err("malloc failed\n");
    }

    _init();
}

LwsServer::~LwsServer()
{
    UniLock lock(_mutex);
    _deinit();

    if (_s_msg_data) {
        free(_s_msg_data);
        _s_msg_data = nullptr;
    }
}

int LwsServer::init()
{
    UniLock lock(_mutex);
    int ret = _init();
    return ret;
}

int LwsServer::deinit()
{
    UniLock lock(_mutex);
    int ret = _deinit();
    return ret;
}

LwsServerClient* LwsServer::waitClient(int timeout_ms)
{
    UniLock lock(_mutex);
    auto client = _waitClient(timeout_ms);
    return client;
}

int LwsServer::cbOnWebSocket(struct lws *wsi, enum lws_callback_reasons reason,
                            void *user, void *in, size_t len)
{
    int ret = 0;

    ClientCtx* per_conn_cctx = reinterpret_cast<ClientCtx*>(user);

    switch (reason) {
        case LWS_CALLBACK_WSI_CREATE: {
            xlog_war("wsi create\n");
            break;
        }
        case LWS_CALLBACK_WSI_DESTROY: {
            xlog_war("wsi destroy\n");
            break;
        }
        case LWS_CALLBACK_PROTOCOL_INIT: {
            xlog_war("protocol init\n");
            // per_conn_cctx
            break;
        }
        case LWS_CALLBACK_PROTOCOL_DESTROY: {
            xlog_war("protocol destroy\n");
            break;
        }
        case LWS_CALLBACK_ESTABLISHED: {
            xlog_war("established\n");
            per_conn_cctx->client = _produceClient();
            if (per_conn_cctx->client) {
                
                char name[64] = {};
                lws_get_peer_simple(wsi, name, sizeof(name));

                char client_info[128] = {};
                snprintf(client_info, sizeof(client_info), "name:%s", name);
                per_conn_cctx->client->setInfo(client_info);
                
                _onClientConnected(per_conn_cctx->client);
            } else {
                xlog_err("produce client failed\n");
            }
            break;
        }
        case LWS_CALLBACK_SERVER_WRITEABLE: {
            xlog_war("server writable\n");
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
            xlog_war("receive msg: <%s>\n", (const char*)in);
            if (per_conn_cctx->client) {
                per_conn_cctx->client->onReceive(in, len);
            } else {
                xlog_err("writable no client\n");
            }
            break;
        }
        case LWS_CALLBACK_CLOSED: {
            xlog_war("close\n");
            xlog_war("client is: %p\n", (void*)per_conn_cctx->client);
            if (per_conn_cctx->client) {
                _onClientDisconnected(per_conn_cctx->client);
                // _restoreClient(per_conn_cctx->client);
            } else {
                xlog_war("client is null\n");
            }
            break;
        }
        default: {
            break;
        }
    }
    return ret;
}

int LwsServer::_init()
{
    bool error_flag = false;

    do {
        auto cb_trd = [this]() { return this->_trdWorkder(); };

        if (_trd_worker) {
            xlog_err("worker already started\n");
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
    bool error_flag = false;
	struct lws_context *context = nullptr;
	struct lws_context_creation_info info = {};

    // todo
    // set trd name

    do {

        // lws_set_log_level(LLL_ERR | LLL_WARN | LLL_NOTICE | LLL_INFO | LLL_DEBUG, my_lws_log_emit_t);
        lws_set_log_level(LLL_ERR | LLL_WARN, my_lws_log_emit_t);

        info.port = _param.port;
        lws_protocols protocols[] = {
            { "lws-minimal-server-echo", 
                callback_websocket, sizeof(ClientCtx), 
                RX_BUF_SIZE, 0, NULL, 0 },
                LWS_PROTOCOL_LIST_TERM,
        };
        info.protocols = protocols;
        // info.options = LWS_SERVER_OPTION_SKIP_SERVER_CANONICAL_NAME 
        //      | LWS_SERVER_OPTION_VALIDATE_UTF8 | LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE;
        info.options = 0;
        info.user = this;
        xlog_war("this: %p\n", (void*)this);
        context = lws_create_context(&info);
        if (!context) {
            xlog_err("lws_create_context failed\n");
            error_flag = true;
            break;
        }
        _lws_context = context;

        xlog_war("lws service begin\n");
        int ret = 0;
        while (ret >= 0 && !_trd_interrupted_flag) {
            ret = lws_service(context, 0);
        }
        xlog_war("lws service end\n");
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
    int error_flag = 0;
    do {
        if (!_trd_worker) {
            xlog_war("trd_worker not started\n");
            error_flag = 1;
            break;
        }
        if (!_trd_worker->joinable()) {
            xlog_err("trd not joinable\n");
            error_flag = 1;
            break;
        }

        if (_lws_context) {
            xlog_war("cancel service\n");
            lws_cancel_service(_lws_context);
            _lws_context = nullptr;
        }

        _trd_interrupted_flag = true;

        xlog_war("join start\n");
        _trd_worker->join();
        xlog_war("join fin\n");
        _trd_worker = nullptr;
    } while (0);

    return error_flag ? -1 : 0;
}

LwsServerClient* LwsServer::_waitClient(int timeout_ms)
{
    xlog_err("not implemented yet\n");

    LwsServerClient *client_ret = nullptr;

    do {
        UniLock _mutex_client_connected;
        if (!_client_connected.empty()) {
            client_ret = _client_connected.back();
            _client_connected.pop_back();
            break;
        }

        _cond_client_connected_changed.wait_for(_mutex_client_connected, 
            std::chrono::milliseconds(timeout_ms));
        
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
    LwsServerClient *client_ret = nullptr;
    do {
        if (_client_pool.size() > CLIENT_MAX_NUM) {
            xlog_err("client num over\n");
            break;
        }

        auto client = new LwsServerClient();
        _client_pool.push_back(client);
        client_ret = client;
    } while (0);

    xlog_war("pool size:%zu\n", _client_pool.size());
    return client_ret;
}

void LwsServer::_restoreClient(LwsServerClient* client)
{
    do {
        bool find_flag = false;

        for (auto it = _client_pool.begin(); it != _client_pool.end(); ) {
            if (*it == client) {
                xlog_war("deleting client: %s\n", (*it)->info().c_str());
                delete *it;
                it = _client_pool.erase(it); // delete this client
                find_flag = true;
            } else {
                ++it;
            }
        }

        if (!find_flag) {
            xlog_err("client not found, may leak\n");
        }
    } while (0);

    xlog_war("pool size:%zu\n", _client_pool.size());
    return ;
}

void LwsServer::_onClientConnected(LwsServerClient *client)
{
    do {
        if (!client) {
            xlog_err("client is null\n");
            break;
        }

        UniLock lock(_mutex_client_connected);

        if (_client_connected.size() > CLIENT_MAX_NUM) {
            xlog_err("inner error, client num over\n");
            break;
        }

        _client_connected.push_back(client);
        _cond_client_connected_changed.notify_one();
        
        xlog_war("connected client num:%zu\n", _client_connected.size());
    } while (0);
    return ;
}

void LwsServer::_onClientDisconnected(LwsServerClient *client)
{
    bool found_flag = false;
    do {
        if (!client) {
            xlog_err("client is null\n");
            break;
        }

        // notify client of disconnected state
        client->onDisconnected();

        UniLock lock(_mutex_client_connected);
        for (auto it = _client_connected.begin(); it != _client_connected.end(); ) {
            if (*it == client) {
                it = _client_connected.erase(it);
                found_flag = true;
            } else {
                ++it;
            }
        }
    } while (0);

    if (!found_flag) {
        xlog_err("client not found\n");
    }
    xlog_war("connected client num:%zu\n", _client_connected.size());
    return ;
}

int LwsServer::_writeData(struct lws *wsi, const void *data, size_t size, lws_write_protocol protocol)
{
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

        xlog_war("sent: %d bytes\n", ret);
    } while (0);

    return error_flag ? -1 : 0;
}