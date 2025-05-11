#pragma once

#ifdef __cplusplus
extern "C" {
#endif 

struct rtsp_server_param {
    int port;
    char username[256];
    char password[256];
};

typedef void* rtsp_server_obj;
rtsp_server_obj rtsp_server_new(const struct rtsp_server_param *param);
int rtsp_server_delete(rtsp_server_obj obj);

#ifdef __cplusplus
}
#endif 