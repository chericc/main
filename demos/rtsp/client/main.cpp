#include "rtsp_client.h"

#include <thread>

#include "xlog.h"
#include "rtsp_client.h"

int main()
{
    xlog_dbg("hello\n");

    struct rtsp_client_param param = {};
    snprintf(param.username, sizeof(param.username), "admin");
    snprintf(param.password, sizeof(param.password), "123456");
    snprintf(param.url, sizeof(param.url), "%s", "rtsp://10.0.0.3:8888/test.264");

    auto client = rtsp_client_start(&param);

    // getchar();
    // getchar();
    std::this_thread::sleep_for(std::chrono::seconds(2));

    rtsp_client_stop(client);

    return 0;
}