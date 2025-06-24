#include <cstring>
#include <cstdio>

#include <time.h>

#include "rtsp_client.h"
#include "xlog.h"

static FILE *fps[8] = {};

void my_rtsp_client_cb(int channel_id, uint8_t *data, size_t size, 
    const char *medium_name, const char *codec_name, 
    struct timeval presentationTime, unsigned int durationInMs)
{
    
    xlog_dbg("[%d]%s/%s: frame: %zu\n", channel_id, medium_name, 
        codec_name, size);

    // if (strcmp(codec_name, "H264") == 0) {
        
    // }
}

int main(int argc, char *argv[])
{
    struct rtsp_client_param param = {};
    
    snprintf(param.url, sizeof(param.url), "%s", 
        "rtsp://10.0.0.3:8888/test.mkv");
    snprintf(param.username, sizeof(param.username), "admin");
    snprintf(param.password, sizeof(param.password), "123456");
    param.cb = my_rtsp_client_cb;

    auto client = rtsp_client_start(&param);

    getchar();
    getchar();

    rtsp_client_stop(client);

    return 0;
}