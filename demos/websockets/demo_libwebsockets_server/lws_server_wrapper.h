#ifndef __LWS_SERVER_WRAPPER_H__
#define __LWS_SERVER_WRAPPER_H__

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif 

typedef void(*lws_server_wrapper_on_msg_cb)(const void *data, size_t size);

typedef struct {
    lws_server_wrapper_on_msg_cb cb_on_msg;
} LwsServerWrapperParam;

int lws_server_wrapper_init(LwsServerWrapperParam param);
int lws_server_wrapper_deinit();

#ifdef __cplusplus
}
#endif 

#endif // __LWS_SERVER_WRAPPER_H__