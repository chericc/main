#include "rtsp_client.h"
#include "xlog.hpp"

int main()
{
    xlog_dbg("hello\n");

    rtsp_client_param param = {};
    param.host = "10.0.0.103";
    param.file = "0/1";
    param.port = "554";
    param.username = "test";
    param.password = "test";
    rtsp_client_handle_t handle = rtsp_client_start(&param);

    getchar();
    getchar();

    rtsp_client_stop(handle);

    return 0;
}