#include <cstring>
#include <string>
#include <algorithm>
#include <thread>

#include "my_nng.hpp"

#include "xlog.h"

namespace {

void cb_on_req(void const *req, size_t req_size, void *rsp, size_t *rsp_size)
{
    xlog_dbg("cb on req\n");
    size_t rsp_size_cache = *rsp_size;
    size_t min_size = std::min(req_size, rsp_size_cache);
    memcpy(rsp, req, min_size);
    *rsp_size = min_size;
}

void server(const char *url)
{
    struct my_nm_start_param param = {};
    param.cb_on_req = cb_on_req;
    int ret = my_nng_start(url, &param);
    xlog_dbg("ret: %d\n", ret);
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void client(const char *url)
{

    while (true) {
        char buf[128] = {};
        xlog_dbg("input to send: \n");
        fgets(buf, sizeof(buf), stdin);
        
        my_nm_req_param param = {};
        param.req = buf;
        if (strlen(buf) > 2) {
            param.req_size = strlen(buf) - 1;
        }
        
        char buf_recv[128] = {};
        size_t buf_recv_size = sizeof(buf_recv);
        param.rsp = buf_recv;
        param.rsp_size = &buf_recv_size;

        int ret = my_nng_req(url, &param);
        xlog_dbg("ret: %d, size=%zd\n", ret, buf_recv_size);
        xlog_dbg("msg: %.*s\n", (int)buf_recv_size, buf_recv);
    }
}

}

int main(int argc, char *argv[])
{
    xlog_dbg("in\n");

    if (argc < 3) {
        xlog_err("usage: %s [server|client] [url: ipc:///tmp/test]\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "server") == 0) {
        server(argv[2]);
    } else if (strcmp(argv[1], "client") == 0) {
        client(argv[2]);
    } else {
        xlog_err("unknown role\n");
    }

    return 0;
}
