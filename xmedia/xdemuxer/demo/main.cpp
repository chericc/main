#include "xdemuxer.h"

#include "xlog.h"

void packet_cb(uint8_t const *packet, size_t size, void *user)
{

}

void test()
{
    xdemuxer_handle handle = xdemuxer_handle_invalid;
    do {
        int ret = 0;

        struct xdemuxer_param param = {};
        param.packet_cb = packet_cb;
        param.userdata = nullptr;
        handle = xdemuxer_open(&param);
        if (handle == xdemuxer_handle_invalid) {
            xlog_err("xdemuxer_open failed\n");
            break;
        }

        uint8_t buf[64] = {};
        ret = xdemuxer_input(handle, buf, sizeof(buf));
        if (ret < 0) {
            xlog_err("xdemuxer_input failed\n");
            break;
        }
    } while (0);

    if (handle != xdemuxer_handle_invalid) {
        xdemuxer_close(handle);
        handle = xdemuxer_handle_invalid;
    }

    
}

int main()
{
    test();
    return 0;
}