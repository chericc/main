#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace rtsp_server {

enum class Codec {
    H264,
    H265,
};

struct Config {
    int         port       = 554;
    std::string streamName = "live";
    Codec       codec      = Codec::H264;
    int         bufferSize = 512 * 1024; ///< Max frame size in bytes
};

/// Initialize and start the RTSP server.
/// Must be called once before pushFrame().
bool init(const Config &cfg);

/// Push an encoded video frame (H264/H265 ES with start codes) into the live stream.
/// Thread-safe: can be called from any thread.
void pushFrame(const uint8_t *data, size_t size, uint64_t timestampUs);

bool isRunning();

/// Shutdown the RTSP server and release all resources.
void shutdown();

} // namespace rtsp_server
