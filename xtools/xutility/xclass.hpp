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
