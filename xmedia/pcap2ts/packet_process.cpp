#include "packet_process.hpp"

#include "xlog.hpp"

PacketProcess::PacketProcess()
{

}

PacketProcess::~PacketProcess()
{

}

int PacketProcess::processEthenetPacket(std::shared_ptr<std::vector<uint8_t>> packet)
{
    do 
    {
        if (!packet)
        {
            break;
        }

        xlog_inf("packet.size(%zu)", packet->size());
    }
    while (0);

    return 0;
}