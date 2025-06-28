#include <chrono>
#include <thread>
#include <cinttypes>

#include "nanomsg/nn.h"
#include "nanomsg/pubsub.h"

#include "xlog.h"

int start_server(const char *url)
{
    int fd = -1;

    do {
        int ret = 0;

        fd = nn_socket(AF_SP, NN_PUB);
        if (fd < 0) {
            xlog_err("nn_socket failed: %s\n", nn_strerror(nn_errno()));
            break;
        }

        ret = nn_bind(fd, url);
        if (ret < 0) {
            xlog_err("nn_bind failed: %s\n", nn_strerror(nn_errno()));
            break;
        }

        while (true) {
            auto now_dur = std::chrono::system_clock::now().time_since_epoch();
            auto secs = std::chrono::duration_cast<std::chrono::seconds>(now_dur).count();
            auto usecs = std::chrono::duration_cast<std::chrono::microseconds>(now_dur).count();

            auto secs_uint64 = static_cast<uint64_t>(secs);
            auto usecs_uint64 = static_cast<uint64_t>(usecs);

            char buf[128] = {};
            snprintf(buf, sizeof(buf), "time: %" PRIu64 ":%" PRIu64, secs_uint64, usecs_uint64);

            ret = nn_send(fd, buf, sizeof(buf), 0);
            if (ret < 0) {
                xlog_err("nn_send failed: %s\n", nn_strerror(nn_errno()));
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    } while (0);

    if (fd >= 0) {
        nn_close(fd);
        fd = -1;
    }
    
    return 0;
}

int main(int argc, char *argv[])
{
    xlog_dbg("server in\n");

    if (argc < 2) {
        xlog_err("Usage: %s [url]\n", argv[0]);
        return -1;
    }

    start_server(argv[1]);

    return 0;
}