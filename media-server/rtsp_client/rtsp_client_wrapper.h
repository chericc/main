#ifndef __RTSP_CLIENT_WRAPPER_H__
#define __RTSP_CLIENT_WRAPPER_H__

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif 

#define rtsp_client_wrapper_handle_invalid 0
typedef intptr_t rtsp_client_wrapper_handle_t;
/* 
payload: rtp-profile.h / RTP_PAYLOAD_xxx
 */
typedef void(*rtsp_client_wrapper_data_cb)(int ch, int payload, const void *data, size_t bytes);

struct rtsp_client_wrapper_param {
    const char *host;
    const char *file;
    const char *username;
    const char *password;
    const char *port;

    rtsp_client_wrapper_data_cb data_cb;
};

rtsp_client_wrapper_handle_t rtsp_client_wrapper_start(const struct rtsp_client_wrapper_param *param);
int rtsp_client_wrapper_destroy(rtsp_client_wrapper_handle_t handle);

#ifdef __cplusplus
}
#endif 

#endif // __RTSP_CLIENT_H__
