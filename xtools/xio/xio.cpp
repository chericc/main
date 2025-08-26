
#include "xio.hpp"

XIO::XIO(const std::string& url, const std::string& mode) {
    ioctx_.url = url;
    ioctx_.mode = mode;
}

XIO::~XIO() {}

uint16_t XIO::rl16() {
    uint16_t value;
    value = r8();
    value |= ((uint16_t)r8() << 8);
    return value;
}

uint32_t XIO::rl24() {
    uint32_t value;
    value = rl16();
    value |= ((uint32_t)r8() << 16);
    return value;
}

uint32_t XIO::rl32() {
    uint32_t value;
    value = rl16();
    value |= ((uint32_t)rl16() << 16);
    return value;
}

uint64_t XIO::rl64() {
    uint64_t value;
    value = rl32();
    value |= ((uint64_t)rl32() << 32);
    return value;
}

uint16_t XIO::rb16() {
    uint16_t value;
    value = ((uint16_t)r8() << 8);
    value |= r8();
    return value;
}

uint32_t XIO::rb24() {
    uint32_t value;
    value = (uint32_t)rb16() << 8;
    value |= (uint32_t)r8();
    return value;
}

uint32_t XIO::rb32() {
    uint32_t value;
    value = (uint32_t)rb16() << 16;
    value |= rb16();
    return value;
}

uint64_t XIO::rb64() {
    uint64_t value;
    value = (uint64_t)rb32() << 32;
    value |= rb32();
    return value;
}

void XIO::wl16(uint16_t value) {
    w8((uint8_t)value);
    w8(value >> 8);
}

void XIO::wl24(uint32_t value) {
    wl16((uint16_t)value);
    w8(value >> 16);
}

void XIO::wl32(uint32_t value) {
    wl16((uint16_t)value);
    wl16(value >> 16);
}

void XIO::wl64(uint64_t value) {
    wl32((uint32_t)value);
    wl32(value >> 32);
}

void XIO::wb16(uint16_t value) {
    w8(value >> 8);
    w8((uint8_t)value);
}

void XIO::wb24(uint32_t value) {
    wb16((uint16_t)(value >> 8));
    w8((uint8_t)value);
}

void XIO::wb32(uint32_t value) {
    wb16((uint16_t)(value >> 16));
    wb16((uint16_t)value);
}

void XIO::wb64(uint64_t value) {
    wb32((uint32_t)(value >> 32));
    wb32((uint32_t)value);
}
