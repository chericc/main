#pragma once

using rtsp_server_handle = void*;
#define rtsp_server_handle_invalid nullptr;

struct rtsp_server_param {
    int port;
    char username[32];
    char password[32];
};
rtsp_server_handle rtsp_server_start(struct rtsp_server_param const* param);
int rtsp_server_destroy(rtsp_server_handle handle);