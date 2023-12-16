
#include "packet.hpp"

#include <inttypes.h>

#include "xlog.hpp"
#include "comm.hpp"

#define ETHERNET_TYPE_IPv4  (0x0800)
#define ETHERNET_TYPE_IPv6  (0x86dd)
#define ETHERNET_TYPE_ARP   (0x0806)

bool SharedData::valid() const
{
    if (data)
    {
        if (offset < data->size() && size <= data->size())
        {
            if (data->size() - size >= offset)
            {
                return true;
            }
        }
    }
    return false;
}

// pure function
PacketInfo::~PacketInfo() {}

const char *PacketInfo::typeName(PacketType packet_type)
{
    const char *str[] = {
        "None",
        "Ethernet",
        "ARP",
        "IPv4",
        "IPv6",
        "IPv4Data",
        "UDP",
        "UDPData",
        "TCP",
        "TCPData",
        "RTP",
        "RTPData",
        "MP2T",
        "Butt",
    };

    const std::size_t str_size = sizeof(str)/sizeof(str[0]);
    const std::size_t i_packet_type = (std::size_t)(packet_type);

    static_assert(str_size == (int)PacketType::Butt + 1, "Size check failed");

    if (i_packet_type < str_size)
    {
        return str[i_packet_type];
    }
    return "";
}

PacketType EthernetPacketInfo::convertEthType(uint16_t eth_type)
{
    PacketType type = PacketType::None;
    switch (eth_type)
    {
        case ETHERNET_TYPE_IPv4:
        {
            type = PacketType::IPv4;
            break;
        }
        case ETHERNET_TYPE_IPv6:
        {
            type = PacketType::IPv6;
            break;
        }
        case ETHERNET_TYPE_ARP:
        {
            type = PacketType::ARP;
            break;
        }
        default:
        {
            break;
        }
    }
    return type;
}

uint8_t IPv4PacketInfo::ipv4_version(uint8_t version_and_len)
{
    return (version_and_len >> 4) & 0x0f;
}

uint8_t IPv4PacketInfo::ipv4_len(uint8_t version_and_len)
{
    return (version_and_len & 0x0f) * 4;
}

uint16_t IPv4PacketInfo::ipv4_flag(uint16_t flag_and_fragment_offset)
{
    return (flag_and_fragment_offset >> 5) & 0x03;
}

uint16_t IPv4PacketInfo::ipv4_fragment_offset(uint16_t flag_and_fragment_offset)
{
    return (flag_and_fragment_offset & 0x1f);
}

PacketType IPv4PacketInfo::convertProtocol(uint8_t ipv4_protocol)
{
    PacketType protocol = PacketType::None;
    switch (ipv4_protocol)
    {
        case 6:
        {
            protocol = PacketType::TCP;
            break;   
        }
        case 17:
        {
            protocol = PacketType::UDP;
            break;
        }
        default:
        {
            xlog_err("Unknown protocol");
            break;
        }
    }
    return protocol;
}

/* https://datatracker.ietf.org/doc/html/rfc3551#page-32 */
PacketType RTPPacketInfo::rtpPayloadConvert(uint8_t pt)
{
    PacketType payload = PacketType::None;
    switch (pt)
    {
        case 33:
        {
            payload = PacketType::MP2T;
            break;
        }
        default:
        {
            xlog_err("Payload type not support(%" PRIu8 ")", pt);
            break;
        }
    }
    return payload;
}