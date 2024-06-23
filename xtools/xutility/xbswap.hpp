#pragma once

#include <cstdint>

#define xbswap16c(x) ((((x) << 8) & 0xff00) | (((x) >> 8) & 0x00ff))
#define xbswap32c(x) (xbswap16c(x) << 16 | xbswap16c(x >> 16))
#define xbswap64c(x) (xbswap32c(x) << 32 | xbswap32c(x >> 32))

#define xbswapc(bits, x) xbswap##bits##c(x)

// be2ne: big endian to native endian
// le2ne: little endian to native endian

#ifdef X_BIGENDIAN
#define x_be2ne16(x) (x)
#define x_be2ne32(x) (x)
#define x_be2ne64(x) (x)
#define x_le2ne16(x) xbswap16c(16, c)
#define x_le2ne32(x) xbswap32c(32, c)
#define x_le2ne64(x) xbswap64c(64, c)
#else
#define x_be2ne16(x) xbswap16c(16, c)
#define x_be2ne32(x) xbswap32c(32, c)
#define x_be2ne64(x) xbswap64c(64, c)
#define x_le2ne16(x) (x)
#define x_le2ne32(x) (x)
#define x_le2ne64(x) (x)
#endif

uint16_t xbswap(uint16_t value) { return xbswap16c(value); }
uint32_t xbswap(uint32_t value) { return xbswap32c(value); }
uint64_t xbswap(uint64_t value) { return xbswap64c(value); }
