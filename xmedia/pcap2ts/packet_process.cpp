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
        xlog_dbg("IPv4.version: %" PRIu8, ipv4p->ipv4().version);
        xlog_dbg("IPv4.len: %" PRIu8, ipv4p->ipv4().len);
        xlog_dbg("IPv4.differentialted_service_field: %" PRIu8, ipv4p->ipv4().differentialted_services_field);
        xlog_dbg("IPv4.total_length: %" PRIu16, ipv4p->ipv4().total_length);
        xlog_dbg("IPv4.identification: %#" PRIx16, ipv4p->ipv4().identification);
        xlog_dbg("IPv4.flag: %" PRIu16, ipv4p->ipv4().flag);
        xlog_dbg("IPv4.fragment_offset: %" PRIu16, ipv4p->ipv4().fragment_offset);
        xlog_dbg("IPv4.time_to_live: %" PRIu8, ipv4p->ipv4().time_to_live);
        xlog_dbg("IPv4.protocol: %" PRIu8, ipv4p->ipv4().protocol);
        xlog_dbg("IPv4.header_checksum: %#" PRIx16, ipv4p->ipv4().header_checksum);
        xlog_dbg("IPv4.ip_src: %s", str_ipv4(ipv4p->ipv4().ip_addr_src).c_str());
        xlog_dbg("IPv4.ip_dst: %s", str_ipv4(ipv4p->ipv4().ip_addr_dst).c_str());
        xlog_dbg("IPv4.padding_size: %zu", ipv4p->ipv4().padding_size);
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
                    debug_packet(shared_packet);
                    break;
                }
                case PacketType::IPv4:
                {
                    end_flag = true;
                    break;
                }
                default:
                {
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