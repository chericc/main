#include "nanomsg/nn.h"
#include "nanomsg/pubsub.h"

#include "xlog.h"

int start_client(const char *url)
{
    int fd = -1;

    do {
        int ret = -1;

        fd = nn_socket(AF_SP, NN_SUB);
        if (fd < 0) {
            xlog_err("nn_socket failed: %s\n", nn_strerror(nn_errno()));
            break;
        }

        ret = nn_connect(fd, url);
        if (ret < 0) {
            xlog_err("nn_connect failed: %s\n", nn_strerror(nn_errno()));
            break;
        }

        ret = nn_setsockopt(fd, NN_SUB, NN_SUB_SUBSCRIBE, "", 0);
        if (ret < 0) {
            xlog_err("nn_setsockopt failed: %s\n", nn_strerror(nn_errno()));
            break;
        }

        while (true) {
            char buf[256] = {};
            ret = nn_recv(fd, buf, sizeof(buf), 0);
            if (ret < 0) {
                xlog_err("nn_recv: %s\n", nn_strerror(nn_errno()));
                break;
            }

            xlog_dbg("msg: [%.*s]\n", 
                static_cast<int>(sizeof(buf)), buf);
        }
        
    } while(0);

    if (fd >= 0) {
        nn_close(fd);
        fd = -1;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    xlog_dbg("client in\n");

    if (argc < 2) {
        xlog_err("Usage: %s [url]\n", argv[0]);
        return -1;
    }

    start_client(argv[1]);
    return 0;
}