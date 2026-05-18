#pragma once

namespace rtsp_client {

/// Initialize and connect to an RTSP stream.
/// The stream will be written to the specified file as raw H264/H265 ES
/// (AnnexB format with 0x00000001 start codes).
/// Must be called once before shutdown().
/// Returns false on initial setup failure.
bool init(const char *rtspURL, const char *outputFile);

bool isRunning();

/// Shutdown the RTSP client, close the output file, and release all resources.
void shutdown();

} // namespace rtsp_client
