/**
 * Some utilitys
 * 
 * Note: Should all be header-only implementations.
*/

#pragma once

class XNonCopyableObject
{
public:
    XNonCopyableObject() = default;
    XNonCopyableObject(XNonCopyableObject const&) = delete;
    XNonCopyableObject& operator=(XNonCopyableObject const&) = delete;
};

static uint32_t xswap32(uint32_t value);