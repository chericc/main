#include <cstring>
#include <cstdio>

#include "xdemuxer.hpp"
#include "xlog.h"

extern "C" {
#include "libavutil/avutil.h"
}

struct ctx {
    FILE *fp = nullptr;
};

void packet_cb(AVPacket const* pkt, void *user)
{
    xlog_dbg("pkt: stream: %d, size=%d\n", pkt->stream_index, pkt->size);
}

int read_cb(uint8_t *buf, int buf_size, void *user)
{
    auto *ctx = reinterpret_cast<struct ctx*>(user);
    size_t ret_fread = fread(buf, 1, buf_size, ctx->fp);

    // xlog_dbg("read: size=%zd\n", ret_fread);
    if (ret_fread == 0) {
        ret_fread = AVERROR_EOF;
    }

    return (int)ret_fread;
}

void test(const char *file)
{
    xdemuxer_handle handle = xdemuxer_handle_invalid;
    FILE *fp = nullptr;
    do {
        int ret = 0;

        fp = fopen(file, "r");
        if (nullptr == fp) {
            xlog_err("fopen failed: %s\n", file);
            break;
        }
        xlog_dbg("file open successful: %s\n", file);

        struct ctx ctx = {};
        ctx.fp = fp;

        struct xdemuxer_param param = {};
        param.on_packet_cb = packet_cb;
        param.read_cb = read_cb;
        param.userdata = &ctx;
        handle = xdemuxer_open(&param);
        if (handle == xdemuxer_handle_invalid) {
            xlog_err("xdemuxer_open failed\n");
            break;
        }

        while (true) {
            // xlog_dbg("read\n");
            ret = xdemuxer_read(handle);
            if (ret < 0) {
                xlog_err("xdemuxer_read failed\n");
                break;
            }
        }
    } while (0);

    if (handle != xdemuxer_handle_invalid) {
        xdemuxer_close(handle);
        handle = xdemuxer_handle_invalid;
    }

    if (fp != nullptr) {
        fclose(fp);
        fp = nullptr;
    }
    
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        xlog_err("usage: %s [file]\n", argv[0]);
        return -1;
    }

    const char *file = argv[1];
    test(file);
    return 0;
}