#include "xlog.hpp"

#include "rtsp_server.h"

int main()
{
    struct rtsp_server_param param = {};
    param.port = 8888;

    snprintf(param.username, sizeof(param.username), "%s", "admin");
    snprintf(param.password, sizeof(param.password), "%s", "123456");

    auto server = rtsp_server_new(&param);

    getchar();
    getchar();

    rtsp_server_delete(server);

    return 0;
}