#include "xdemuxer.hpp"

#include <inttypes.h>

#include "xlog.h"

extern "C" {
#include "libavcodec/avcodec.h"
}

void test_info(const char *file)
{
    do {
        XDemuxer myff(file);

        if (!myff.open()) {
            xlog_err("open failed\n");
            break;
        }

        auto info = myff.getInfo();
        if (!info) {
            xlog_err("getinfo failed\n");
            break;
        }

        auto dump = XDemuxer::dumpInfo(info);
        xlog_dbg("dump: {}\n", dump.c_str());
    } while (false);
}

void test_dump(const char *file, const char *outputfile)
{
    FILE *fpV = nullptr;
    FILE *fpA = nullptr;
    do {
        XDemuxer myff(file);
        if (!myff.open()) {
            xlog_err("open failed\n");
            break;
        }

        auto frame = std::make_shared<XDemuxerFrame>();

        std::string vfile = std::string(outputfile) + "_v";
        std::string afile = std::string(outputfile) + "_a";

        while (true) {
            if (!myff.popPacket(frame)) {
                xlog_dbg("pop end or fail\n");
                break;
            }

            if (frame->isVideo) {
                if (fpV == nullptr) {
                    fpV = fopen(vfile.c_str(), "w");
                }
                if (fpV != nullptr) {
                    fwrite(frame->buf.data(), 1, frame->buf.size(), fpV);
                    fflush(fpV);
                }
            } else {
                if (fpA == nullptr) {
                    fpA = fopen(afile.c_str(), "w");
                }
                if (fpA != nullptr) {
                    fwrite(frame->buf.data(), 1, frame->buf.size(), fpA);
                    fflush(fpA);
                }
            }

            xlog_dbg("pop size: {}, pts: %" PRIu64 ", isVideo=%d\n", frame->buf.size(), frame->pts, frame->isVideo);
        }
    } while (false);

    if (fpV != nullptr) {
        fclose(fpV);
        fpV = nullptr;
    }
    if (fpA != nullptr) {
        fclose(fpA);
        fpA = nullptr;
    }
}

void print_usage(int argc, char *argv[])
{
    xlog_err("usage: {} [info|dump] [file] [outputfile]\n", argv[0]);
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