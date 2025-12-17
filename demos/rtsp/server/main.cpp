
#include <cstdlib>

#include "rtsp_server.hpp"
#include "xlog.h"

int main(int argc, char *argv[])
{
    if ((argc != 2) && (argc != 4)) {
        xlog_err("usage: [port: 8080] [username] [password]");
        return -1;
    }
    
    struct rtsp_server_param param = {};
    param.port = atoi(argv[1]);
    if (argc == 4) {
        snprintf(param.username, sizeof(param.username), "%s", argv[2]);
        snprintf(param.password, sizeof(param.password), "%s", argv[3]);
    }

    auto server = rtsp_server_start(&param);
    (void)server;
    getchar();
    getchar();
    return 0;
}