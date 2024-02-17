/*
 * CRC 
 *
 * copy from ffmpeg/crc
*/

#pragma once

#include <cstddef>
#include <cstdint>

enum class XCRCID
{
    CRC_8_ATM,
    CRC_16_ANSI,
    CRC_16_CCITT,
    CRC_32_IEEE,
    CRC_32_IEEE_LE,  /*< reversed bitorder version of CRC_32_IEEE */
    CRC_16_ANSI_LE,  /*< reversed bitorder version of CRC_16_ANSI */
    CRC_24_IEEE,
    CRC_8_EBU,
    CRC_MAX,         
};

class XCrc
{
public:
    XCrc(XCRCID crc_id);
    uint32_t crc(uint32_t last_crc, const uint8_t *buffer, size_t length);
private:
    const uint32_t *_table = nullptr;
};