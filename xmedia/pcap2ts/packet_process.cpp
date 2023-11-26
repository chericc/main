#include "packet_process.hpp"

#include <inttypes.h>

#include "xlog.hpp"
#include "packet.hpp"
#include "comm.hpp"

static void debug_packet(std::shared_ptr<IPacket> ipacket)
{
    if (ipacket->type() == PacketType::Ethernet)
    {
        std::shared_ptr<EthernetPacket> ep = std::static_pointer_cast<EthernetPacket>(ipacket);
        xlog_dbg("Ethernet.dst_mac: %s", str_macaddr(ep->eth().mac_dst).c_str());
        xlog_dbg("Ethernet.src_mac: %s", str_macaddr(ep->eth().mac_src).c_str());
        xlog_dbg("Ethernet.sub_type: %s", ep->subTypeName(ep->convertEthType(ep->eth().type)));
        xlog_dbg("Ethernet.sub_size: %zu", ep->data().tsize());
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

        SharedPacketData tmp_data;
        tmp_data.data = data;
        tmp_data.offset = 0;
        tmp_data.size = data->size();

        std::shared_ptr<IPacket> ipacket = std::make_shared<EthernetPacket>();
        if (ipacket->assign(tmp_data) < 0)
        {
            xlog_err("Assign ethernet packet failed");
            error = true;
            break;
        }

        debug_packet(ipacket);
        if (doProcessEthernetPacket(ipacket) < 0)
        {
            xlog_err("Process ethernet packet failed");
            error = true;
            break;
        }
    }
    while (0);

    if (error)
    {
        return -1;
    }

    return 0;
}

int PacketProcess::doProcessEthernetPacket(std::shared_ptr<IPacket> ipacket)
{
    bool error = false;

    do
    {
        std::shared_ptr<IPacket> ipacket_sub;

        if (ipacket->type() != PacketType::Ethernet)
        {
            error = true;
            xlog_err("Not ethernet packet");
            break;
        }

        std::shared_ptr<EthernetPacket> ethernet_packet = 
            std::static_pointer_cast<EthernetPacket>(ipacket);
        EthernetSubType ethernet_subtype = EthernetPacket::convertEthType(ethernet_packet->eth().type);

        switch (ethernet_subtype)
        {
            case EthernetSubType::IPv4:
            {
                ipacket_sub = std::make_shared<IPv4EthernetPacket>();
                if (ipacket_sub->assign(ipacket->data()) < 0)
                {
                    xlog_err("Assign ipv4 packet failed");
                    error = true;
                    break;
                }
                debug_packet(ipacket_sub);
                break;
            }
            default:
            {
                xlog_dbg("Ignoring packet(type:%d)", (int)ipacket->type());
                break;
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