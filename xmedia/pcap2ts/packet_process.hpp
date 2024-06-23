#pragma once

#include <stdint.h>

#include <memory>
#include <vector>

#include "packet.hpp"

class PacketProcess {
   public:
    PacketProcess();
    ~PacketProcess();
    int processEthernetData(std::shared_ptr<std::vector<uint8_t>> data);

   private:
    //
    int processEthernetPacket(std::shared_ptr<SharedPacket> packet);
    int processIPv4Packet(std::shared_ptr<SharedPacket> packet);
    int processUDPPacket(std::shared_ptr<SharedPacket> packet);
    int processRTPPacket(std::shared_ptr<SharedPacket> packet);
};
