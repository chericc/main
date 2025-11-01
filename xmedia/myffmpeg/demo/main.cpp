#include "myffmpeg.hpp"

#include "xlog.h"

extern "C" {
#include "libavcodec/avcodec.h"
}

void test_info(const char *file)
{
    do {
        MyFFmpeg myff(file);

        if (myff.open() < 0) {
            xlog_err("open failed\n");
            break;
        }

        auto info = myff.getInfo();
        if (!info) {
            xlog_err("getinfo failed\n");
            break;
        }

        xlog_dbg("file: %s\n"
            "width: %d\n"
            "height: %d\n"
            "codec: %s\n", 
            file, 
            info->width, 
            info->height, 
            avcodec_get_name(info->codec));
    } while (false);
}

void test_dump(const char *file, const char *outputfile)
{
    FILE *fp = nullptr;
    do {
        MyFFmpeg myff(file);
        if (myff.open() < 0) {
            xlog_err("open failed\n");
            break;
        }

        std::vector<uint8_t> buf;
        buf.reserve(MyFFmpeg::MAX_PKT_SIZE);

        while (true) {
            int ret = myff.popPacket(buf);
            if (ret < 0) {
                xlog_err("pop failed\n");
                break;
            }
            if (ret == 0) {
                xlog_dbg("pop end\n");
                break;
            }

            if (fp == nullptr) {
                fp = fopen(outputfile, "w");
            }
            if (fp != nullptr) {
                fwrite(buf.data(), 1, buf.size(), fp);
                fflush(fp);
            }

            xlog_dbg("pop size: %zd\n", buf.size());
        }
    } while (false);

    if (fp != nullptr) {
        fclose(fp);
        fp = nullptr;
    }
}

void print_usage(int argc, char *argv[])
{
    xlog_err("usage: %s [info|dump] [file] [outputfile]\n", argv[0]);
}

int main(int argc, char *argv[])
{
    if (argc < 3) {
        print_usage(argc, argv);
        return -1;
    }

    const char *choice = argv[1];
    const char *file = argv[2];

    if (strcmp(choice, "info") == 0) {
        test_info(file);
    } else if (strcmp(choice, "dump") == 0) {
        const char *outputfile = argv[3];
        if (argc < 4) {
            print_usage(argc, argv);
            return -1;
        }
        test_dump(file, outputfile);
    } else {
        xlog_err("unknown choice\n");
    }

    return 0;
}