#ifndef __RTSP_CLIENT_H__
#define __RTSP_CLIENT_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif 

typedef intptr_t rtsp_client_handle_t;

struct rtsp_client_param {
    const char *host;
    const char *file;
    const char *username;
    const char *password;
    const char *port;
};

rtsp_client_handle_t rtsp_client_start(const rtsp_client_param *param);
int rtsp_client_stop(rtsp_client_handle_t handle);

#ifdef __cplusplus
}
#endif 

#endif // __RTSP_CLIENT_H__
