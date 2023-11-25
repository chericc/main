#include "packet_process.hpp"

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
        // xlog_dbg("Ipv4.")
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
        doProcessEthernetPacket(ipacket);
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
        std::shared_ptr<IPv4EthernetPacket> ipacket_new;

        switch (ipacket->type())
        {
            case PacketType::IPv4:
            {
                ipacket_new = std::make_shared<IPv4EthernetPacket>();
                if (ipacket_new->assign(ipacket->data()) < 0)
                {
                    xlog_err("Assign ipv4 packet failed");
                    error = true;
                    break;
                }
                debug_packet(ipacket);
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