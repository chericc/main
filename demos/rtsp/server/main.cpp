#include "rtsp_server.h"
#include "xdemuxer.hpp"

#include <cstdio>
#include <csignal>
#include <cstdlib>
#include <thread>
#include <chrono>

extern "C" {
#include "libavcodec/avcodec.h"
}

static volatile bool g_quit = false;

static void on_signal(int sig) {
    if (sig == SIGINT || sig == SIGTERM)
        g_quit = true;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s <media_file> [port]\n", argv[0]);
        return 1;
    }

    const char *file = argv[1];
    int port = 8554;
    if (argc > 2)
        port = std::atoi(argv[2]);

    rtsp_server::Config cfg;
    cfg.port = port;
    cfg.streamName = "live";

    signal(SIGINT, on_signal);
    signal(SIGTERM, on_signal);

    // init RTSP server
    if (!rtsp_server::init(cfg)) {
        fprintf(stderr, "rtsp_server::init failed\n");
        return 1;
    }
    printf("RTSP server listening on port %d ...\n", port);

    // open media file via xdemuxer
    XDemuxer demuxer(file);
    if (!demuxer.open()) {
        fprintf(stderr, "failed to open: %s\n", file);
        rtsp_server::shutdown();
        return 1;
    }

    auto info = demuxer.getInfo();
    if (info) {
        printf("Media: %dx%d, codec=%s\n",
               info->widthV, info->heightV,
               XDemuxer::dumpInfo(info).c_str());
    }

    // push frames in a loop, simulating a live stream
    auto frame = std::make_shared<XDemuxerFrame>();
    uint64_t base_pts = 0;
    uint64_t last_pts_us = 0;
    bool first = true;

    while (!g_quit) {
        if (!demuxer.popPacket(frame)) {
            // EOF — restart from the beginning
            demuxer.forceIFrame();
            first = true;
            last_pts_us = 0;
            printf("EOF, restarting from beginning ...\n");
            continue;
        }

        if (!frame->isVideo)
            continue;

        if (first) {
            base_pts = frame->pts;
            first = false;
        }

        uint64_t pts_us = (frame->pts - base_pts) * 1000; // ms → us
        if (pts_us > last_pts_us) {
            std::this_thread::sleep_for(std::chrono::microseconds(pts_us - last_pts_us));
        }
        last_pts_us = pts_us;

        rtsp_server::pushFrame(frame->buf.data(), frame->buf.size(), pts_us);
    }

    printf("\nShutting down ...\n");
    rtsp_server::shutdown();
    printf("RTSP server stopped.\n");
    return 0;
}
