#include <cstring>
#include <cstdio>

#include <sys/time.h>

#include "rtsp_client.h"
#include "xlog.h"

static FILE *fps[8] = {};

void my_rtsp_client_cb(int channel_id, uint8_t *data, size_t size, 
    const char *medium_name, const char *codec_name, 
    struct timeval presentationTime, unsigned int durationInMs)
{
    
    xlog_dbg("[{}]{}/{}: frame: {}\n", channel_id, medium_name, 
        codec_name, size);

    if ( (channel_id < 0) || (channel_id >= static_cast<int>(sizeof(fps) / sizeof(fps[0]))) ) {
        xlog_err("channel id over\n");
        return ;
    }

    char filename[64] = {};
    snprintf(filename, sizeof(filename), "%d_%s.%s", channel_id, medium_name, codec_name);

    if (!fps[channel_id]) {
        fps[channel_id] = fopen(filename, "w");
    }

    if ( (strcmp(codec_name, "H264") == 0)
        || (strcmp(codec_name, "H265") == 0)) {
        fwrite(data, 1, size, fps[channel_id]);
    } else if (strcmp(codec_name, "MPEG4-GENERIC") == 0) {
        fwrite(data, 1, size, fps[channel_id]);
    } else {
        fwrite(data, 1, size, fps[channel_id]);
    }
    
    fflush(fps[channel_id]);
}

int main(int argc, char *argv[])
{
    struct rtsp_client_param param = {};
    
    snprintf(param.url, sizeof(param.url), "%s", 
        "rtsp://10.0.0.3:8554/test.mkv");
    // snprintf(param.username, sizeof(param.username), "admin");
    // snprintf(param.password, sizeof(param.password), "123456");
    param.cb = my_rtsp_client_cb;

    auto client = rtsp_client_start(&param);

    getchar();
    getchar();

    rtsp_client_stop(client);

    return 0;
}