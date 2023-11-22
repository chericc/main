
#include "packet.hpp"
#include "xlog.hpp"
#include "comm.hpp"

#define ETHERNET_TYPE_IPv4  (0x0800)
#define ETHERNET_TYPE_IPv6  (0x86dd)
#define ETHERNET_TYPE_ARP   (0x0806)

bool SharedPacketData::valid() const
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
int EthernetPacket::assign(SharedPacketData data)
{
    bool error = false;
    do 
    {
        EthernetStructure eth{};

        if (!data.valid())
        {
            xlog_err("Invalid data");
            error = true;
            break;
        }

        uint8_t *pdata = data.tdata();
        std::size_t size = data.tsize();

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

SharedPacketData EthernetPacket::data() const
{
    const std::size_t before_data_size = _eth.mac_dst.size() + _eth.mac_src.size()
        + sizeof(_eth.type);

    SharedPacketData tmp_data = _data;

    tmp_data.offset += before_data_size;
    tmp_data.size -= before_data_size;

    if (!tmp_data.valid())
    {
        return SharedPacketData();
    }
    
    return tmp_data;
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

int IPv4EthernetPacket::assign(SharedPacketData data)
{
    bool error = false;

    do 
    {
        IPv4Structure ipv4{};

        if (!data.valid())
        {
            xlog_err("Invalid data");
            error = true;
            break;
        }

        uint8_t *pdata = data.tdata();
        std::size_t size = data.tsize();

        const std::size_t min_head_size = sizeof(ipv4.version_and_len)
            + sizeof(ipv4.differentialted_services_field) + sizeof(ipv4.total_length) + sizeof(ipv4.identification)
            + sizeof(ipv4.flag_and_fragment_offset) + sizeof(ipv4.time_to_live)
            + sizeof(ipv4.protocol) + sizeof(ipv4.header_checksum)
            + ipv4.ip_addr_src.size() + ipv4.ip_addr_dst.size();

        if (size < min_head_size)
        {
            xlog_err("Size check failed");
            error = true;
            break;
        }

        static_assert(sizeof(ipv4.version_and_len) == 1, "");
        ipv4.version_and_len = net_u8(pdata, size);
        pdata += sizeof(ipv4.version_and_len);
        size -= sizeof(ipv4.version_and_len);
        
        static_assert(sizeof(ipv4.differentialted_services_field) == 1, "");
        ipv4.differentialted_services_field = net_u8(pdata, size);
        pdata += sizeof(ipv4.differentialted_services_field);
        size -= sizeof(ipv4.differentialted_services_field);

        static_assert(sizeof(ipv4.total_length) == 2, "");
        ipv4.total_length = net_u16(pdata, size);
        pdata += sizeof(ipv4.total_length);
        size -= sizeof(ipv4.total_length);

        static_assert(sizeof(ipv4.identification) == 2, "");
        ipv4.identification = net_u16(pdata, size);
        pdata += sizeof(ipv4.identification);
        size -= sizeof(ipv4.identification);

        static_assert(sizeof(ipv4.flag_and_fragment_offset) == 2, "");
        ipv4.flag_and_fragment_offset = net_u16(pdata, size);
        pdata += sizeof(ipv4.flag_and_fragment_offset);
        size -= sizeof(ipv4.flag_and_fragment_offset);

        static_assert(sizeof(ipv4.time_to_live) == 1, "");
        ipv4.time_to_live = net_u8(pdata, size);
        pdata += sizeof(ipv4.time_to_live);
        size -= sizeof(ipv4.time_to_live);

        static_assert(sizeof(ipv4.protocol) == 1, "");
        ipv4.protocol = net_u8(pdata, size);
        pdata += sizeof(ipv4.protocol);
        size -= sizeof(ipv4.protocol);

        static_assert(sizeof(ipv4.header_checksum) == 2, "");
        ipv4.header_checksum = net_u16(pdata, size);
        pdata += sizeof(ipv4.header_checksum);
        size -= sizeof(ipv4.header_checksum);

        net_copy(ipv4.ip_addr_src.data(), ipv4.ip_addr_src.size(), 
            pdata, size);
        pdata += ipv4.ip_addr_src.size();
        size -= ipv4.ip_addr_src.size();

        net_copy(ipv4.ip_addr_dst.data(), ipv4.ip_addr_dst.size(), 
            pdata, size);
        pdata += ipv4.ip_addr_dst.size();
        size -= ipv4.ip_addr_dst.size();

        // todo
    }
    while (0);

    if (error)
    {
        return -1;
    }

    return 0;
}

PacketType IPv4EthernetPacket::type() const
{
    return PacketType::IPv4;
}

SharedPacketData IPv4EthernetPacket::data() const
{
    return SharedPacketData();
}

const IPv4Structure& IPv4EthernetPacket::ipv4() const
{
    return _ipv4;
}