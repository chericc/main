#include <cstddef>
#include <stdio.h>

extern "C"
{
#include <libavformat/avformat.h>
}

#include "xlog.hpp"

#define array_nb(array) (sizeof(array)/sizeof(array[0]))

static int open(const char *file)
{
    bool berror = false;
    AVFormatContext *fc = nullptr;
    bool open_suc_flag = false;
    int st_index[AVMEDIA_TYPE_NB]{};

    do 
    {
        fc = avformat_alloc_context();
        if (!fc)
        {
            xlog_err("avformat_alloc_context failed");
            berror = true;
            break;
        }

        if (avformat_open_input(&fc, file, nullptr, nullptr) < 0)
        {
            xlog_err("avformat_open_input failed");
            berror = true;
            break;
        }
        open_suc_flag = true;

        for (auto &ref : st_index)
        {
            ref = -1;
        }

        for (int i = 0; i < fc->nb_streams; ++i)
        {
            AVMediaType type = fc->streams[i]->codecpar->codec_type;
            if (type >= 0)
            {
                const char *type_name = av_get_media_type_string(type);
                xlog_dbg("stream[%d]: %s", i, type_name);
            }
        }
    }
    while (0);

    if (fc)
    {
        avformat_close_input(&fc);
    }

    return (berror ? -1 : 0);
}

int main(int argc, char *argv[])
{
    xlog_setoutput({stdout});

    if (argc != 2)
    {
        xlog_err("Usage: %s filename", argv[0]);
        return -1;
    }

    const char *filename = argv[1];
    int ret = open (filename);
    return ret;
}