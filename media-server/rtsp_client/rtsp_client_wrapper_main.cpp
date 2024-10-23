#include "rtsp_client_wrapper.h"
#include "xlog.hpp"

static void data_cb(int ch, int payload, const void *data, size_t bytes, uint32_t timestamp)
{
    if (ch == 0) {
        xlog_war("ch:%d, payload:%d, bytes:%d, timestamp=%u\n", ch, payload, (int)bytes, timestamp);
    }
    
    constexpr int ch_num = 4;

    static FILE *fp_array[ch_num];

    const char *encode_name = "UNKNOWN";
    if (payload == 98) {
        encode_name = "H264";
    } else if (payload == 100) {
        encode_name = "H265";
    } else if (payload == 0) {
        encode_name = "PCMU";
    }

    if (ch >= 0 && ch < ch_num) {
        FILE *&fp = fp_array[ch];
        if (!fp) {
            char filename[64];
            snprintf(filename, sizeof(filename), "./ch_%d.%s", ch, encode_name);
            fp = fopen(filename, "w");
        }
        if (fp) {
            fwrite(data, 1, bytes, fp);
            fflush(fp);
        }
    }

    return ;
}

int main()
{
    rtsp_client_wrapper_param param = {};
    param.host = "10.0.0.103";
    param.file = "0/1";
    param.port = "554";
    param.username = "test";
    param.password = "test";
    param.data_cb = data_cb;
    rtsp_client_wrapper_handle_t handle = rtsp_client_wrapper_start(&param);

    getchar();
    getchar();

    rtsp_client_wrapper_destroy(handle);

    return 0;
}