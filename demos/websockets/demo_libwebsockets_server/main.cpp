

#include <unistd.h>
#include "lws_server_wrapper.h"
#include "xlog.h"

void on_msg_cb(unsigned int clientID, LWS_SERVER_WRAPPER_CLIENT_TYPE type, const void *data, size_t size)
{
    xlog_dbg("client:{},type:{},msg:{2:.{1}},size:{}\n", 
        clientID, type, (int)size, (const char*)data, size);
}

void on_event_cb(unsigned int clientID, LWS_SERVER_WRAPPER_CLIENT_TYPE type, LWS_SERVER_WRAPPER_CLIENT_EVENT event)
{
    xlog_dbg("client:{},type:{},event:{}\n", 
        clientID, type, event);
}

int main()
{
    LwsServerWrapperParam param = {};
    param.app_lws_uri = "/app";
    param.screen_lws_uri = "/screen";
    param.port = 9008;
    param.cb_on_msg = on_msg_cb;
    param.cb_on_event = on_event_cb;
    lws_server_wrapper_init(&param);

    getchar();
    getchar();

    lws_server_wrapper_deinit();

    return 0;
}