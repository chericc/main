#include "xutility.hpp"

uint16_t xswap(uint16_t value)
{
    return ((value & 0xff) << 8)
        | ((value >> 8) & 0xff);
}

uint32_t xswap(uint32_t value)
{
    return (xswap((uint16_t)(value & 0xffff)) << 16)
        | (xswap((uint16_t)(value >> 16)));
}

uint64_t xswap(uint64_t value)
{
    return (((uint64_t)(xswap((uint32_t)(value & 0xffffffff)))) << 32)
        | (xswap((uint32_t)(value >> 32)));
}