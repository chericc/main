
#include "packet.hpp"
#include "xlog.hpp"
#include "comm.hpp"

#define ETHERNET_TYPE_IPv4  (0x0800)
#define ETHERNET_TYPE_IPv6  (0x86dd)
#define ETHERNET_TYPE_ARP   (0x0806)

const char *IPacket::typeName(PacketType packet_type)
{
    const char *str[] = {
        "None",
        "Ethernet",
        "IPv4",
        "IPv6",
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

/*

| mac_dst(6) | mac_src(6) | type(2) | sub_data |

*/
int EthernetPacket::assign(std::shared_ptr<std::vector<uint8_t>> data)
{
    bool error = false;
    do 
    {
        EthernetStructure eth{};

        if (!data)
        {
            xlog_err("Null");
            error = true;
            break;
        }

        uint8_t *pdata = data->data();
        std::size_t size = data->size();

        std::size_t min_size = eth.mac_dst.size()
            + eth.mac_src.size()
            + sizeof(eth.type);

        if (size < min_size)
        {
            xlog_err("Size error");
            error = true;
            break;
        }

        // mac_dst
        net_copy(eth.mac_dst.data(), eth.mac_dst.size(),
            pdata, size);
        
        // mac_src
        pdata += eth.mac_dst.size();
        size -= eth.mac_dst.size();
        net_copy(eth.mac_src.data(), eth.mac_src.size(),
            pdata, size);

        // type
        pdata += eth.mac_src.size();
        size -= eth.mac_src.size();
        eth.type = net_u16(pdata, size);

        // copy
        _data = data;
        _eth = eth;
    }
    while (0);

    if (error)
    {
        return -1;
    }
    return 0;
}

PacketType EthernetPacket::type() const
{
    return PacketType::Ethernet;
}

uint8_t *EthernetPacket::data() const
{
    const std::size_t before_data_size = _eth.mac_dst.size() + _eth.mac_src.size()
        + sizeof(_eth.type);
    return static_cast<uint8_t*>(_data->data()) + before_data_size;
}

std::size_t EthernetPacket::size() const
{
    const std::size_t before_data_size = _eth.mac_dst.size() + _eth.mac_src.size()
        + sizeof(_eth.type);
    if (_data->size() < before_data_size)
    {
        return 0;
    }
    return _data->size() - before_data_size;
}

const EthernetStructure& EthernetPacket::eth() const
{
    return _eth;
}

EthernetSubType EthernetPacket::convertEthType(uint16_t eth_type)
{
    EthernetSubType type = EthernetSubType::None;
    switch (eth_type)
    {
        case ETHERNET_TYPE_IPv4:
        {
            type = EthernetSubType::IPv4;
            break;
        }
        case ETHERNET_TYPE_IPv6:
        {
            type = EthernetSubType::IPv6;
            break;
        }
        case ETHERNET_TYPE_ARP:
        {
            type = EthernetSubType::ARP;
            break;
        }
        default:
        {
            break;
        }
    }
    return type;
}

const char *EthernetPacket::subTypeName(EthernetSubType subtype)
{
    const char *str[] = {
        "None",
        "ARP",
        "IPv4",
        "IPv6",
        "Butt",
    };

    const std::size_t str_size = sizeof(str)/sizeof(str[0]);
    const std::size_t i_sub_type = (std::size_t)(subtype);

    static_assert(str_size == (int)EthernetSubType::Butt + 1, "Size check failed");

    if (i_sub_type < str_size)
    {
        return str[i_sub_type];
    }
    return "";
}

int IPv4EthernetPacket::assign(std::shared_ptr<std::vector<uint8_t>> data)
{
    return 0;
}

PacketType IPv4EthernetPacket::type() const
{
    return PacketType::IPv4;
}

uint8_t *IPv4EthernetPacket::data() const
{
    return nullptr;
}

std::size_t IPv4EthernetPacket::size() const
{
    return 0;
}

const IPv4Structure& IPv4EthernetPacket::ipv4() const
{
    return _ipv4;
}