#pragma once

#include "inc.hpp"


class LwsServerClient {
public:
    enum {
        MAX_MSG_CACHE_NUM = 5,
    };

    using DisconnectCb = std::function<void(LwsServerClient*)>;
    using OnDataCb = std::function<void(LwsServerClient*, const void *data, size_t size)>;
    struct ClientCallbacks {
        DisconnectCb cbOnDisconnected;
        OnDataCb cbOnData;
    };
    int setNotifyCb(ClientCallbacks cbs);
    std::string const &info();
    std::string const &uri();
    int sendData(const void *data, size_t size);

    using NeedWriteCb = std::function<void()>;
    int withNeedWriteCb(NeedWriteCb);
    int withInfo(std::string const& info);
    int withUri(std::string const& uri);
    using WriteCb = std::function<int(struct lws *wsi, const void *data, size_t size, lws_write_protocol protocol)>;
    int onWritable(struct lws *wsi, WriteCb);
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
    string _uri = "null";

    NeedWriteCb _need_write_cb;
    ClientCallbacks _notify_cbs;
    std::vector<std::vector<uint8_t>> _msgs_to_send;
    std::vector<std::vector<uint8_t>> _msgs_received;

    std::recursive_mutex _mutex;

    friend class LwsServer;
};