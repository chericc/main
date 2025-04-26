#pragma once

#include "inc.hpp"
#include "libwebsockets.h"
#include "lws_server_client.hpp"

using std::string;
using std::mutex;
using std::condition_variable;
using UniLock = std::unique_lock<mutex>;
using UniRecuLock = std::unique_lock<std::recursive_mutex>;

struct LwsServerParam {
    int heartbeat_interval_usec;
    int port = 0;
    string screen_uri;
    string app_uri;
};

class LwsServer {
public:
    LwsServer(const LwsServerParam& param);
    ~LwsServer();
    int init();
    int deinit();

    // 这个接口上设计为裸指针
    // 是因为libwebsockets回调机制设计问题(user)
    // 注意，如果client断开连接了，则对象就会被马上销毁掉
    // 返回的client在使用时应注意如果断开，则应清除
    LwsServerClient* waitClient(int timeout_ms);
    int destroyClient(LwsServerClient *client);

    int cbOnWebSocket(struct lws *wsi, enum lws_callback_reasons reason,
                              void *user, void *in, size_t len);
    int cbOnHttp(struct lws *wsi, enum lws_callback_reasons reason,
                              void *user, void *in, size_t len);
private:
    int _init();
    int _deinit();
    void _trdWorkder();
    LwsServerClient* _waitClient(UniRecuLock &lock, int timeout_ms);

    void _onClientConnected(LwsServerClient *client);
    void _onClientDisconnected(LwsServerClient *client);

    int _writeData(struct lws *wsi, const void *data, size_t size, lws_write_protocol protocol);

    // weak ref
    // 涉及到资源泄漏，封装一下
    LwsServerClient *_produceClient();
    int _restoreClient(LwsServerClient* client);

    std::recursive_mutex _mutex;
    std::shared_ptr<std::thread> _trd_worker;
    bool _trd_interrupted_flag = false;
    
    struct lws_context *_lws_context;

    const LwsServerParam _param;

    mutex _mutex_client_connected;
    condition_variable _cond_client_connected_changed;
    std::vector<LwsServerClient*> _client_connected;

    // 分配池，用于跟踪可能的泄漏
    mutex _mutex_allocated_client_trace_pool;
    std::vector<LwsServerClient*> _allocated_client_trace_pool;

    struct msg_data_t {
        uint8_t _internal_head[LWS_PRE];
        uint8_t payload[16 * 1024 - LWS_PRE];
    };
    msg_data_t* _s_msg_data = nullptr;
};