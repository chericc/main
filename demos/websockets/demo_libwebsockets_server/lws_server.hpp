#ifndef __LWS_SERVER_H__
#define __LWS_SERVER_H__

#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <cstdint>
#include <thread>
#include <condition_variable>
// #include <list>
#include <chrono>
#include <functional>

#include "libwebsockets.h"

using std::string;
using std::mutex;
using std::condition_variable;
using UniLock = std::unique_lock<mutex>;

struct LwsServerParam {
    int heartbeat_interval_usec;
    int port = 0;
    string screen_uri;
    string app_uri;
};

typedef void(*lws_server_client_cb_on_msg)(const char *cmd, const char *args, const char *in_msg, int *out_code, char *out_msg, size_t out_msg_size);
typedef void(*lws_server_client_cb_on_heartbeat_msg)(const char *msg);

struct LwsServerClientParam {
    // lws_server_client_cb_on_msg cb_on_msg;
    // lws_server_client_cb_on_heartbeat_msg cb_on_heartbeat;
};

class LwsServer;

class LwsServerClient {
public:
    struct Request {
        string cmd_id;
        string msg_id;
        string args;
        string json_to_send;

        int ms_wait = 0;
    };
    struct Response {
        std::vector<uint8_t> data;
        int code = 0;
    };
    struct ClientCallbacks {
        std::function<void(void)> cbOnDisconnected;
    };
    int setDisconnectCb(std::function<void(void)>);
    std::string const &info();
    int request(Request const& req, Response &resp);

    using NeedWriteCb = void();
    int setNeedWriteCb(NeedWriteCb);
    int setInfo(std::string const& info);
    using WriteCb = int(struct lws *wsi, const void *data, size_t size, lws_write_protocol protocol);
    int onWritable(struct lws *wsi, std::function<WriteCb>);
    int onReceive(const void *data, size_t size);
    int onDisconnected();
private:
    // not create or destroy by user
    LwsServerClient() = default;
    ~LwsServerClient() = default;

    enum class State {
        Init,
        Connected,
        Disconnected,
    };
    State _state = State::Init;
    string _info = "null";
    std::function<void(void)> _disconnectCb;
    mutex _mutex;

    friend class LwsServer;
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
    void destroyClient(LwsServerClient *client);

    int cbOnWebSocket(struct lws *wsi, enum lws_callback_reasons reason,
                              void *user, void *in, size_t len);
private:
    int _init();
    int _deinit();
    void _trdWorkder();
    LwsServerClient* _waitClient(int timeout_ms);

    void _onClientConnected(LwsServerClient *client);
    void _onClientDisconnected(LwsServerClient *client);

    int _writeData(struct lws *wsi, const void *data, size_t size, lws_write_protocol protocol);

    // weak ref
    // 涉及到资源泄漏，封装一下
    LwsServerClient *_produceClient();
    void _restoreClient(LwsServerClient* client);

    mutex _mutex;
    std::shared_ptr<std::thread> _trd_worker;
    bool _trd_interrupted_flag = false;
    
    struct lws_context *_lws_context;

    const LwsServerParam _param;

    mutex _mutex_client_connected;
    condition_variable _cond_client_connected_changed;
    std::vector<LwsServerClient*> _client_connected;

    // 分配池，用于跟踪可能的泄漏
    std::vector<LwsServerClient*> _client_pool;


    struct msg_data_t {
        uint8_t _internal_head[LWS_PRE];
        uint8_t payload[16 * 1024 - LWS_PRE];
    };
    msg_data_t* _s_msg_data = nullptr;
};

#endif // __LWS_SERVER_H__