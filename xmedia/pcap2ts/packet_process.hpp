#pragma once

#include <memory>
#include <vector>
#include <stdint.h>

#include "packet.hpp"

class PacketProcess
{
public:
    PacketProcess();
    ~PacketProcess();
    int processEthernetData(std::shared_ptr<std::vector<uint8_t>> data);
private:
    // 
    int doProcessEthernetPacket(std::shared_ptr<IPacket> packet);
};
