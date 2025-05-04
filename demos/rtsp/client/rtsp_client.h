#pragma once

#ifndef __cplusplus
extern "C" {
#endif // __cplusplus

struct rtsp_client_param {
    char url[256];
    char username[64];
    char password[64];
};

#define rtsp_client_obj_invalid (NULL)
typedef void* rtsp_client_obj;
rtsp_client_obj rtsp_client_start(rtsp_client_param const* param);
int rtsp_client_stop(rtsp_client_obj obj);

#ifndef __cplusplus
}
#endif // __cplusplus