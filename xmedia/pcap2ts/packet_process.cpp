#include "packet_process.hpp"

#include <inttypes.h>

#include "xlog.hpp"
#include "packet.hpp"
#include "comm.hpp"

static void debug_packet(std::shared_ptr<SharedPacket> packet)
{
    xlog_dbg("PacketType: %s", PacketInfo::typeName(packet->cur_type));
    xlog_dbg("PacketData: [%zu,%zu] in [%zu]", packet->data.offset, packet->data.size, packet->data.data->size());
    xlog_dbg("PacketInfo: ");
    for (auto it : packet->parsed_info)
    {
        switch(it.first)
        {
            case PacketType::Ethernet:
            {
                std::shared_ptr<EthernetPacketInfo> ep = std::static_pointer_cast<EthernetPacketInfo>(it.second);
                xlog_dbg("Ethernet.dst_mac: %s", str_macaddr(ep->mac_dst).c_str());
                xlog_dbg("Ethernet.src_mac: %s", str_macaddr(ep->mac_src).c_str());
                xlog_dbg("Ethernet.sub_type: %s", PacketInfo::typeName(ep->convertEthType(ep->type)));
                break;
            }
            case PacketType::IPv4:
            {
                std::shared_ptr<IPv4PacketInfo> ipv4p = std::static_pointer_cast<IPv4PacketInfo>(it.second);
                xlog_dbg("ipv4->version: %" PRIu8, ipv4p->version);
                xlog_dbg("ipv4->len: %" PRIu8, ipv4p->len);
                xlog_dbg("ipv4->differentialted_service_field: %" PRIu8, ipv4p->differentialted_services_field);
                xlog_dbg("ipv4->total_length: %" PRIu16, ipv4p->total_length);
                xlog_dbg("ipv4->identification: %#" PRIx16, ipv4p->identification);
                xlog_dbg("ipv4->flag: %" PRIu16, ipv4p->flag);
                xlog_dbg("ipv4->fragment_offset: %" PRIu16, ipv4p->fragment_offset);
                xlog_dbg("ipv4->time_to_live: %" PRIu8, ipv4p->time_to_live);
                xlog_dbg("ipv4->protocol: %" PRIu8, ipv4p->protocol);
                xlog_dbg("ipv4->header_checksum: %#" PRIx16, ipv4p->header_checksum);
                xlog_dbg("ipv4->ip_src: %s", str_ipv4(ipv4p->ip_addr_src).c_str());
                xlog_dbg("ipv4->ip_dst: %s", str_ipv4(ipv4p->ip_addr_dst).c_str());
                xlog_dbg("ipv4->padding_size: %zu", ipv4p->padding_size);
                break;
            }
            default:
            {
                break;
            }
        }
    }
#if 0
    if (ipacket->type() == PacketType::Ethernet)
    {
        std::shared_ptr<EthernetPacket> ep = std::static_pointer_cast<EthernetPacket>(ipacket);
        xlog_dbg("Ethernet.dst_mac: %s", str_macaddr(ep->mac_dst).c_str());
        xlog_dbg("Ethernet.src_mac: %s", str_macaddr(ep->mac_src).c_str());
        xlog_dbg("Ethernet.sub_type: %s", ep->subTypeName(ep->convertEthType(ep->type)));
        xlog_dbg("Ethernet.sub_size: %zu", ep->data().offsetSize());
    }
    else if(ipacket->type() == PacketType::IPv4)
    {
        std::shared_ptr<IPv4EthernetPacket> ipv4p = std::static_pointer_cast<IPv4EthernetPacket>(ipacket);
        xlog_dbg("ipv4->version: %" PRIu8, ipv4p->ipv4().version);
        xlog_dbg("ipv4->len: %" PRIu8, ipv4p->ipv4().len);
        xlog_dbg("ipv4->differentialted_service_field: %" PRIu8, ipv4p->ipv4().differentialted_services_field);
        xlog_dbg("ipv4->total_length: %" PRIu16, ipv4p->ipv4().total_length);
        xlog_dbg("ipv4->identification: %#" PRIx16, ipv4p->ipv4().identification);
        xlog_dbg("ipv4->flag: %" PRIu16, ipv4p->ipv4().flag);
        xlog_dbg("ipv4->fragment_offset: %" PRIu16, ipv4p->ipv4().fragment_offset);
        xlog_dbg("ipv4->time_to_live: %" PRIu8, ipv4p->ipv4().time_to_live);
        xlog_dbg("ipv4->protocol: %" PRIu8, ipv4p->ipv4().protocol);
        xlog_dbg("ipv4->header_checksum: %#" PRIx16, ipv4p->ipv4().header_checksum);
        xlog_dbg("ipv4->ip_src: %s", str_ipv4(ipv4p->ipv4().ip_addr_src).c_str());
        xlog_dbg("ipv4->ip_dst: %s", str_ipv4(ipv4p->ipv4().ip_addr_dst).c_str());
        xlog_dbg("ipv4->padding_size: %zu", ipv4p->ipv4().padding_size);
    }
    else 
    {
        xlog_dbg("Type not support(%d)", (int)ipacket->type());
    }
#endif 
}

PacketProcess::PacketProcess()
{

}

PacketProcess::~PacketProcess()
{

}

int PacketProcess::processEthernetData(std::shared_ptr<std::vector<uint8_t>> data)
{
    bool error = false;

    do 
    {
        if (!data)
        {
            break;
        }

        xlog_dbg("packet.size(%zu)", data->size());

        std::shared_ptr<SharedPacket> shared_packet = std::make_shared<SharedPacket>
            (PacketType::Ethernet, SharedData{data, 0, data->size()});

        bool end_flag = false;
        for ( ; !error && !end_flag; )
        {
            switch (shared_packet->cur_type)
            {
                case PacketType::Ethernet:
                {
                    if (processEthernetPacket(shared_packet) < 0)
                    {
                        xlog_err("Process ethernet packet failed");
                        error = true;
                    }
                    break;
                }
                case PacketType::IPv4:
                {
                    if (processIPv4Packet(shared_packet) < 0)
                    {
                        xlog_err("Process ipv4 packet failed");
                        error = true;
                    }
                    break;
                }
                default:
                {
                    debug_packet(shared_packet);
                    end_flag = true;
                    break;
                }
            }
        }
    }
    while (0);

    if (error)
    {
        return -1;
    }

    return 0;
}

int PacketProcess::processEthernetPacket(std::shared_ptr<SharedPacket> packet)
{
    bool error = false;
    do 
    {
        if (!packet)
        {
            xlog_err("Null packet");
            error = true;
            break;
        }

        SharedData& data = packet->data;
        std::shared_ptr<EthernetPacketInfo> eth = std::make_shared<EthernetPacketInfo>();

        if (!data.valid())
        {
            xlog_err("Invalid data");
            error = true;
            break;
        }

        uint8_t *pdata = data.offsetData();
        std::size_t size = data.offsetSize();

        std::size_t ethernet_header_size = eth->mac_dst.size()
            + eth->mac_src.size()
            + sizeof(eth->type);

        if (size < ethernet_header_size)
        {
            xlog_err("Size error");
            error = true;
            break;
        }

        // mac_dst
        net_copy(eth->mac_dst.data(), eth->mac_dst.size(),
            pdata, size);
        
        // mac_src
        pdata += eth->mac_dst.size();
        size -= eth->mac_dst.size();
        net_copy(eth->mac_src.data(), eth->mac_src.size(),
            pdata, size);

        // type
        pdata += eth->mac_src.size();
        size -= eth->mac_src.size();
        eth->type = net_u16(pdata, size);

        // copy
        SharedData data_tmp = data;
        data_tmp.offset += ethernet_header_size;
        data_tmp.size -= ethernet_header_size;

        // integer limit check
        if (data_tmp.offset < data.offset
            || data_tmp.size > data.size)
        {
            xlog_err("Inner error");
            error = true;
            break;
        }

        data = data_tmp;
        packet->parsed_info.push_back(std::make_pair(PacketType::Ethernet, eth));
        packet->cur_type = EthernetPacketInfo::convertEthType(eth->type);
    }
    while (0);

    if (error)
    {
        return -1;
    }
    return 0;
}

int PacketProcess::processIPv4Packet(std::shared_ptr<SharedPacket> packet)
{
    bool error = false;

    do 
    {
        std::shared_ptr<IPv4PacketInfo> ipv4 = std::make_shared<IPv4PacketInfo>();

        if (!packet)
        {
            xlog_err("Null packet");
            error = true;
            break;
        }

        SharedData &data = packet->data;

        uint8_t *pdata = data.offsetData();
        std::size_t size = data.offsetSize();

        const std::size_t head_size_without_options = sizeof(ipv4->version_and_len)
            + sizeof(ipv4->differentialted_services_field) + sizeof(ipv4->total_length) + sizeof(ipv4->identification)
            + sizeof(ipv4->flag_and_fragment_offset) + sizeof(ipv4->time_to_live)
            + sizeof(ipv4->protocol) + sizeof(ipv4->header_checksum)
            + ipv4->ip_addr_src.size() + ipv4->ip_addr_dst.size();

        if (size < head_size_without_options)
        {
            xlog_err("Size check failed(%zu < %zu)", size, head_size_without_options);
            error = true;
            break;
        }

        static_assert(sizeof(ipv4->version_and_len) == 1, "");
        ipv4->version_and_len = net_u8(pdata, size);
        pdata += sizeof(ipv4->version_and_len);
        size -= sizeof(ipv4->version_and_len);

        ipv4->version = ipv4->ipv4_version(ipv4->version_and_len);
        ipv4->len = ipv4->ipv4_len(ipv4->version_and_len);

        if (ipv4->version != 4) // version = 4 --> IPv4
        {
            xlog_err("Verson check failed(%" PRIu8 ")", ipv4->version);
            error = true;
            break;
        }

        if (ipv4->len < head_size_without_options)
        {
            xlog_err("Length check failed(%" PRIu8 ", %zu)", ipv4->len, head_size_without_options);
            error = true;
            break;
        }

        if (size < ipv4->len)
        {
            xlog_err("Size check failed(%zu < %" PRIu8 ")", size, ipv4->len);
            error = true;
            break;
        }
        
        std::size_t option_len = ipv4->len - head_size_without_options;

        static_assert(sizeof(ipv4->differentialted_services_field) == 1, "");
        ipv4->differentialted_services_field = net_u8(pdata, size);
        pdata += sizeof(ipv4->differentialted_services_field);
        size -= sizeof(ipv4->differentialted_services_field);

        static_assert(sizeof(ipv4->total_length) == 2, "");
        ipv4->total_length = net_u16(pdata, size);
        pdata += sizeof(ipv4->total_length);
        size -= sizeof(ipv4->total_length);

        if (ipv4->total_length > data.offsetSize())
        {
            xlog_err("Total length check failed(%" PRIu16 "> %zu)", ipv4->total_length, data.offsetSize());
            error = true;
            break;
        }

        ipv4->padding_size = data.offsetSize() - ipv4->total_length;

        static_assert(sizeof(ipv4->identification) == 2, "");
        ipv4->identification = net_u16(pdata, size);
        pdata += sizeof(ipv4->identification);
        size -= sizeof(ipv4->identification);

        static_assert(sizeof(ipv4->flag_and_fragment_offset) == 2, "");
        ipv4->flag_and_fragment_offset = net_u16(pdata, size);
        pdata += sizeof(ipv4->flag_and_fragment_offset);
        size -= sizeof(ipv4->flag_and_fragment_offset);

        ipv4->flag = ipv4->ipv4_flag(ipv4->flag_and_fragment_offset);
        ipv4->fragment_offset = ipv4->ipv4_fragment_offset(ipv4->flag_and_fragment_offset);

        static_assert(sizeof(ipv4->time_to_live) == 1, "");
        ipv4->time_to_live = net_u8(pdata, size);
        pdata += sizeof(ipv4->time_to_live);
        size -= sizeof(ipv4->time_to_live);

        static_assert(sizeof(ipv4->protocol) == 1, "");
        ipv4->protocol = net_u8(pdata, size);
        pdata += sizeof(ipv4->protocol);
        size -= sizeof(ipv4->protocol);

        static_assert(sizeof(ipv4->header_checksum) == 2, "");
        ipv4->header_checksum = net_u16(pdata, size);
        pdata += sizeof(ipv4->header_checksum);
        size -= sizeof(ipv4->header_checksum);

        net_copy(ipv4->ip_addr_src.data(), ipv4->ip_addr_src.size(), 
            pdata, size);
        pdata += ipv4->ip_addr_src.size();
        size -= ipv4->ip_addr_src.size();

        net_copy(ipv4->ip_addr_dst.data(), ipv4->ip_addr_dst.size(), 
            pdata, size);
        pdata += ipv4->ip_addr_dst.size();
        size -= ipv4->ip_addr_dst.size();

        // skip ipv4 options
        if (option_len > 0)
        {
            pdata += option_len;
            size -= option_len;
        }

        // EthernetPacketData = IPv4Packet + [padding]
        SharedData data_tmp = data;
        data_tmp.offset += ipv4->len;
        data_tmp.size = ipv4->total_length;
        if (data_tmp.offset < data.offset
            || data_tmp.size > data.size)
        {
            xlog_err("Inner error");
            error = true;
            break;
        }

        // copy
        data = data_tmp;
        packet->parsed_info.push_back(std::make_pair(PacketType::IPv4, ipv4));
        packet->cur_type = PacketType::IPv4Done;
    }
    while (0);

    if (error)
    {
        return -1;
    }

    return 0;
}