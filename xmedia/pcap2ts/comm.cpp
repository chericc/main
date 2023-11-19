
#include "comm.hpp"

#include <string.h>
#include <inttypes.h>

std::pair<std::string,std::string> str_ipv4addr(const std::array<uint8_t, 8> &ipv4)
{
    std::string ipstr;
    std::string maskstr;
    char str_tmp[256];

    snprintf(str_tmp, sizeof(str_tmp), "%" PRIu8 ".%" PRIu8 ".%" PRIu8 ".%" PRIu8, 
        ipv4[0], ipv4[1], ipv4[2], ipv4[3]);
    ipstr.assign(str_tmp);

    snprintf(str_tmp, sizeof(str_tmp), "%" PRIu8 ".%" PRIu8 ".%" PRIu8 ".%" PRIu8, 
        ipv4[4], ipv4[5], ipv4[6], ipv4[7]);
    maskstr.assign(str_tmp);

    return std::make_pair(ipstr, maskstr);
}

std::string str_ipv6addr(const std::array<uint8_t, 17> &ipv6)
{
    char str_tmp[256];
    snprintf(str_tmp, sizeof(str_tmp), 
        "%02" PRIx8 "%02" PRIx8 ".%02" PRIx8 "%02" PRIx8 ".%02" PRIx8 "%02" PRIx8 ".%02" PRIx8 "%02" PRIx8 
        ".%02" PRIx8 "%02" PRIx8 ".%02" PRIx8 "%02" PRIx8 ".%02" PRIx8 "%02" PRIx8 ".%02" PRIx8 "%02" PRIx8 
        "/%" PRIu8,
        ipv6[0], ipv6[1], ipv6[2], ipv6[3], ipv6[4], ipv6[5], ipv6[6], ipv6[7], 
        ipv6[8], ipv6[9], ipv6[10], ipv6[11], ipv6[12], ipv6[13], ipv6[14], ipv6[15], 
        ipv6[16]);
    return std::string(str_tmp);
}

std::string str_macaddr(const std::array<uint8_t, 6> &mac)
{
    char str_tmp[256];
    snprintf(str_tmp, sizeof(str_tmp),
        "%02" PRIx8 ".%02" PRIx8 ".%02" PRIx8 ".%02" PRIx8 ".%02" PRIx8 ".%02" PRIx8,
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return std::string(str_tmp);
}

std::string str_euiaddr(const std::array<uint8_t, 8> &euiaddr)
{
    char str_tmp[256];
    snprintf(str_tmp, sizeof(str_tmp),
        "%02" PRIx8 ".%02" PRIx8 ".%02" PRIx8 ".%02" PRIx8 ".%02" PRIx8 ".%02" PRIx8 ".%02" PRIx8 ".%02" PRIx8,
        euiaddr[0], euiaddr[1], euiaddr[2], euiaddr[3], 
        euiaddr[4], euiaddr[5], euiaddr[6], euiaddr[7]);
    return std::string(str_tmp);
}

std::string str_bps(double bps)
{
    std::array<std::string, 4> speed_name = {"bps", "kbps", "Mbps", "Gbps"};

    double bitrate = bps;
    std::size_t i_level = 0;
    char str[64];
    for (i_level = 0; i_level < speed_name.size(); ++i_level)
    {
        if (bitrate < 1000.0)
        {
            break;
        }
        bitrate = bitrate / 1000;
    }
    
    snprintf(str, sizeof(str), "%.1g %s", bitrate, speed_name[i_level].c_str());
    return std::string(str);
}


uint8_t net_u8(const void *ptr, std::size_t size)
{
    const uint8_t *pi = (const uint8_t *)ptr;
    if (size < sizeof(uint8_t))
    {
        return 0;
    }
    return pi[0];
}

uint16_t net_u16(const void *ptr, std::size_t size)
{
    const uint8_t *pi = (const uint8_t *)ptr;
    if (size < sizeof(uint16_t))
    {
        return 0;
    }
    return  pi[1] 
        + ((uint16_t)pi[0] << 8);
}

uint32_t net_u32(const void *ptr, std::size_t size)
{
    const uint8_t *pi = (const uint8_t *)ptr;
    if (size < sizeof(uint32_t))
    {
        return 0;
    }
    return  pi[3] 
        + ((uint32_t)pi[2] << 8)
        + ((uint32_t)pi[1] << 16)
        + ((uint32_t)pi[0] << 24);
}

uint64_t net_u64(const void *ptr, std::size_t size)
{
    const uint8_t *pi = (const uint8_t *)ptr;
    if (size < sizeof(uint64_t))
    {
        return 0;
    }
    return  pi[7] 
        + ((uint64_t)pi[6] << 8)
        + ((uint64_t)pi[5] << 16)
        + ((uint64_t)pi[4] << 24)
        + ((uint64_t)pi[3] << 32)
        + ((uint64_t)pi[2] << 40)
        + ((uint64_t)pi[1] << 48)
        + ((uint64_t)pi[0] << 56);
}

std::size_t net_copy(void *dst, std::size_t dst_size, void *src, std::size_t src_size)
{
    std::size_t min_size = std::min(dst_size, src_size);
    memcpy(dst, src, min_size);
    return min_size;
}