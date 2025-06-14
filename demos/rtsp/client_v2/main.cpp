#include <cstring>
#include <cstdio>

#include "rtsp_client.h"
#include "xlog.h"

int main(int argc, char *argv[])
{
    xlog_dbg("hello world\n");

    struct rtsp_client_param param = {};
    
    snprintf(param.url, sizeof(param.url), "%s", 
        "rtsp://10.0.0.3:8888/test.mkv");
    snprintf(param.username, sizeof(param.username), "admin");
    snprintf(param.password, sizeof(param.password), "123456");

    auto client = rtsp_client_start(&param);
    

    getchar();
    getchar();

    rtsp_client_stop(client);

    return 0;
}