#include "rtsp_client.h"

#include "xlog.hpp"
#include "rtsp_client.h"

int main()
{
    xlog_dbg("hello\n");

    struct rtsp_client_param param = {};
    snprintf(param.username, sizeof(param.username), "admin");
    snprintf(param.password, sizeof(param.password), "123456");
    snprintf(param.url, sizeof(param.url), "%s", "rtsp://10.0.0.3:8888/test.264");

    rtsp_client_start(&param);

    return 0;
}