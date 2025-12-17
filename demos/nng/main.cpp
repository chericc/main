#include <cstring>
#include <string>
#include <algorithm>
#include <thread>

#include "my_nng.hpp"

#include "xlog.h"

namespace {

bool exit_flag = false;

void cb_on_req(const char *url, void const *req, size_t req_size, void *rsp, size_t *rsp_size)
{
    xlog_dbg("cb on req: url: {}", url);

    int ret = snprintf((char*)rsp, *rsp_size, "%s: %.*s", url, (int)req_size, (const char *)req);
    *rsp_size = ret;

    std::string str((const char *)req, req_size);
    if (str == "exit") {
        exit_flag = true;
    }
}

void server(std::vector<std::string> urls)
{
    struct my_nm_start_param param = {};
    param.cb_on_req = cb_on_req;
    for (auto const& ref : urls) {
        int ret = my_nng_start(ref.c_str(), &param);
        if (ret != 0) {
            xlog_err("start failed");
            break;
        }
    }
    while (!exit_flag) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    for (auto const& ref : urls) {
        my_nng_stop(ref.c_str());
    }
}

void client(std::vector<std::string> urls)
{
    while (true) {
        char buf[128] = {};
        xlog_dbg("input to send, eg: 0 hello");
        for (size_t i = 0; i < urls.size(); ++i) {
            xlog_dbg("[{}]: {}", i, urls[i].c_str());
        }
        fgets(buf, sizeof(buf), stdin);

        int choice = -1;
        char text[32] = {};
        sscanf(buf, "%d %[^\n]", &choice, text);

        if (choice >= 0 && choice < (int)urls.size()) {
            my_nm_req_param param = {};
            param.req = text;
            param.req_size = strlen(text);

            char buf_recv[128] = {};
            size_t buf_recv_size = sizeof(buf_recv);
            param.rsp = buf_recv;
            param.rsp_size = &buf_recv_size;

            int ret = my_nng_req(urls[choice].c_str(), &param);
            xlog_dbg("ret: {}, size={}", ret, buf_recv_size);
            xlog_dbg("msg: {:.{}}", buf_recv, buf_recv_size);

            std::string str((const char *)param.req, param.req_size);
            if (str == "exit") {
                break;
            }
        } else {
            xlog_err("unknown choice: {}", choice);
        }
    }

    for (auto const& ref : urls) {
        my_nng_stop(ref.c_str());
    }
}

}

int main(int argc, char *argv[])
{
    xlog_dbg("in");

    if (argc < 3) {
        xlog_err("usage: {} [server|client] [url: ipc:///tmp/test]", argv[0]);
        return 1;
    }

    std::vector<std::string> urls = {};
    for (int i = 0; i < argc - 2; ++i) {
        urls.push_back(argv[2 + i]);
    }

    if (strcmp(argv[1], "server") == 0) {
        server(urls);
    } else if (strcmp(argv[1], "client") == 0) {
        client(urls);
    } else {
        xlog_err("unknown role");
    }

    return 0;
}
