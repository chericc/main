#pragma once

#include <cstdint>
#include <cstddef>

extern "C" {
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavcodec/avcodec.h"
}

#define xdemuxer_handle_invalid nullptr

typedef void* xdemuxer_handle;

using xdemuxer_on_packet_cb = void (*)(AVPacket const* pkt, void *user);
using xdemuxer_read_cb = int(*)(uint8_t *buf, int buf_size, void *user);
// using xdemuxer_write_cb = int(*)(xdemuxer_handle handle, uint8_t *buf, int buf_size);
// using xdemuxer_seek_cb = int64_t(*)(xdemuxer_handle handle, int64_t pos, int flag);

struct xdemuxer_param {
    void* userdata;
    xdemuxer_on_packet_cb on_packet_cb;
    xdemuxer_read_cb read_cb;
    // xdemuxer_write_cb write_cb;
    // xdemuxer_seek_cb seek_cb;
};

xdemuxer_handle xdemuxer_open(struct xdemuxer_param const* param);
int xdemuxer_read(xdemuxer_handle handle);
int xdemuxer_close(xdemuxer_handle handle);
