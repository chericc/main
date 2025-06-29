#pragma once

#include <stdint.h>
#include <stddef.h>

#ifndef __cplusplus
extern "C" {
#endif // __cplusplus

typedef void (*rtsp_client_cb)(int channel_id, uint8_t *data, size_t size, 
    const char *medium_name, const char *codec_name, 
    struct timeval presentationTime, unsigned int durationInMs);

struct rtsp_client_param {
    char url[256];
    char username[64];
    char password[64];
    rtsp_client_cb cb;
};

#define rtsp_client_obj_invalid (NULL)
typedef void* rtsp_client_obj;
rtsp_client_obj rtsp_client_start(rtsp_client_param const* param);
int rtsp_client_stop(rtsp_client_obj obj);

#ifndef __cplusplus
}
#endif // __cplusplus
