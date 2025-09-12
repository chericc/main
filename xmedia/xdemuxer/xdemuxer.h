#pragma once

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif 

#ifdef __cplusplus
#define xdemuxer_handle_invalid nullptr
#else
#define xdemuxer_handle_invalid NULL
#endif 

typedef void* xdemuxer_handle;

typedef void xdemuxer_packet_cb(uint8_t const *packet, size_t size, void *user);

struct xdemuxer_param {
    void* user;
};

xdemuxer_handle xdemuxer_open();
int xdemuxer_input(xdemuxer_handle obj, const uint8_t *data, size_t size);
int xdemuxer_close(xdemuxer_handle obj);

#ifdef __cplusplus
}
#endif 