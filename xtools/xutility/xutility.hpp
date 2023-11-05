/**
 * Some utilitys
 * 
 * Note: Should all be header-only implementations.
*/

#pragma once

#include <cstdint>

class XNonCopyableObject
{
public:
    XNonCopyableObject() = default;
    XNonCopyableObject(XNonCopyableObject const&) = delete;
    XNonCopyableObject& operator=(XNonCopyableObject const&) = delete;
};

uint16_t xswap16(uint16_t value);
uint32_t xswap32(uint32_t value);
uint64_t xswap64(uint64_t value);