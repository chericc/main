#include <gtest/gtest.h>

#include "xcrc.hpp"
#include "xlog.hpp"

/* demo: refs to ffmpeg/libavutil/tests/crc.c */
TEST(crc, base)
{
    uint8_t buf[1999];

#if 0
    static const unsigned p[7][3] = {
        { (int)XCRCID::CRC_32_IEEE_LE, 0xEDB88320, 0x3D5CDD04 },
        { (int)XCRCID::CRC_32_IEEE   , 0x04C11DB7, 0xC0F5BAE0 },
        { (int)XCRCID::CRC_24_IEEE   , 0x864CFB  , 0xB704CE   },
        { (int)XCRCID::CRC_16_ANSI_LE, 0xA001    , 0xBFD8     },
        { (int)XCRCID::CRC_16_ANSI   , 0x8005    , 0x1FBB     },
        { (int)XCRCID::CRC_8_ATM     , 0x07      , 0xE3       },
        { (int)XCRCID::CRC_8_EBU     , 0x1D      , 0xD6       },
    };
#endif 

    for (size_t i = 0; i < sizeof(buf); i++)
    {
        buf[i] = i + i * i;
    }
    
    {
        XCrc crc(XCRCID::CRC_32_IEEE_LE);
        EXPECT_EQ(crc.crc(0, buf, sizeof(buf)), 0x3D5CDD04UL);
    }

    {
        XCrc crc(XCRCID::CRC_32_IEEE);
        EXPECT_EQ(crc.crc(0, buf, sizeof(buf)), 0xE0BAF5C0UL);
    }

    {
        XCrc crc(XCRCID::CRC_24_IEEE);
        EXPECT_EQ(crc.crc(0, buf, sizeof(buf)), 0x326039UL);
    }

    {
        XCrc crc(XCRCID::CRC_16_ANSI_LE);
        EXPECT_EQ(crc.crc(0, buf, sizeof(buf)), 0xBFD8UL);
    }

    {
        XCrc crc(XCRCID::CRC_16_ANSI);
        EXPECT_EQ(crc.crc(0, buf, sizeof(buf)), 0xBB1FUL);
    }

    {
        XCrc crc(XCRCID::CRC_8_ATM);
        EXPECT_EQ(crc.crc(0, buf, sizeof(buf)), 0xE3UL);
    }

    {
        XCrc crc(XCRCID::CRC_8_EBU);
        EXPECT_EQ(crc.crc(0, buf, sizeof(buf)), 0xD6UL);
    }
}