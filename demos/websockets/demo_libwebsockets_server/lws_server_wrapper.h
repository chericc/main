#ifndef __LWS_SERVER_WRAPPER_H__
#define __LWS_SERVER_WRAPPER_H__

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif 

typedef enum {
    LWS_SERVER_WRAPPER_CLIENT_TYPE_NONE,
    LWS_SERVER_WRAPPER_CLIENT_TYPE_SCREEN, // 车载屏
    LWS_SERVER_WRAPPER_CLIENT_TYPE_APP,     // APP
    LWS_SERVER_WRAPPER_CLIENT_TYPE_BUTT,
} LWS_SERVER_WRAPPER_CLIENT_TYPE;

typedef enum {
    LWS_SERVER_WRAPPER_CLIENT_EVENT_CONNECTED,  // 连接
    LWS_SERVER_WRAPPER_CLIENT_EVENT_DISCONNECTED,       // 断开
} LWS_SERVER_WRAPPER_CLIENT_EVENT;

typedef void(*lws_server_wrapper_on_msg_cb)(unsigned int clientID, LWS_SERVER_WRAPPER_CLIENT_TYPE type, const void *data, size_t size);
typedef void(*lws_server_wrapper_on_event_cb)(unsigned int clientID, LWS_SERVER_WRAPPER_CLIENT_TYPE type, LWS_SERVER_WRAPPER_CLIENT_EVENT event);

typedef struct {
    const char *app_lws_uri;
    const char *screen_lws_uri;
    int port;

    lws_server_wrapper_on_event_cb cb_on_event;
    lws_server_wrapper_on_msg_cb cb_on_msg;
} LwsServerWrapperParam;

int lws_server_wrapper_init(const LwsServerWrapperParam *param);
int lws_server_wrapper_deinit();
int lws_server_wrapper_send_msg(unsigned int clientID, const void *data, size_t size);

#ifdef __cplusplus
}
#endif 

#endif // __LWS_SERVER_WRAPPER_H__
