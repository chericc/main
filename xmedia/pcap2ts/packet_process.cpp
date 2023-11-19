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
        xlog_dbg("Ethernet.sub_size: %zu", ep->size());
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

        std::shared_ptr<IPacket> ipacket = std::make_shared<EthernetPacket>();
        if (ipacket->assign(data) < 0)
        {
            xlog_err("Assign ethernet packet failed");
            error = true;
            break;
        }

        ipacket = doProcessEthernetPacket(ipacket);
    }
    while (0);

    if (error)
    {
        return -1;
    }

    return 0;
}

std::shared_ptr<IPacket> PacketProcess::doProcessEthernetPacket(std::shared_ptr<IPacket> ipacket)
{
    debug_packet(ipacket);
    return nullptr;
}