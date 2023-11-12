/**
 * Some utilitys
 * 
 * Note: Should all be header-only implementations.
*/

#pragma once

#include <cstdint>

#define X_UNUSED_PARAMETER(x) do{(void)x;}while(0)

class XNonCopyableObject
{
public:
    XNonCopyableObject() = default;
    XNonCopyableObject(XNonCopyableObject const&) = delete;
    XNonCopyableObject& operator=(XNonCopyableObject const&) = delete;
};

uint16_t xswap(uint16_t value);
uint32_t xswap(uint32_t value);
uint64_t xswap(uint64_t value);