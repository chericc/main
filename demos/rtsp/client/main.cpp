#include "rtsp_client.hpp"

#include <cstdio>
#include <csignal>
#include <cstdlib>
#include <chrono>
#include <thread>

static volatile bool g_quit = false;

static void on_signal(int sig) {
    if (sig == SIGINT || sig == SIGTERM)
        g_quit = true;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "usage: %s <rtsp_url> <output_file>\n", argv[0]);
        fprintf(stderr, "  eg: %s rtsp://localhost:8554/live output.h264\n", argv[0]);
        return 1;
    }

    signal(SIGINT, on_signal);
    signal(SIGTERM, on_signal);

    const char *rtspURL    = argv[1];
    const char *outputFile = argv[2];

    if (!rtsp_client::init(rtspURL, outputFile)) {
        fprintf(stderr, "rtsp_client::init failed\n");
        return 1;
    }

    printf("Receiving RTSP stream from %s, writing to %s ...\n",
           rtspURL, outputFile);
    printf("Press Ctrl+C to stop.\n");

    while (!g_quit && rtsp_client::isRunning()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    printf("\nShutting down ...\n");
    rtsp_client::shutdown();
    printf("RTSP client stopped.\n");
    return 0;
}
