#include "xutility.hpp"

uint16_t xswap16(uint16_t value)
{
    return ((value & 0xff) << 8)
        | ((value >> 8) & 0xff);
}

uint32_t xswap32(uint32_t value)
{
    return (xswap16(value & 0xffff) << 16)
        | (xswap16(value >> 16));
}

uint64_t xswap64(uint64_t value)
{
    return (((uint64_t)(xswap32(value & 0xffffffff))) << 32)
        | (xswap32((value >> 32)));
}