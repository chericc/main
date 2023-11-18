#pragma once

#include <memory>
#include <vector>
#include <stdint.h>

enum class PacketType
{
    None,
    Ethenet,
    ARP,
    IPv4,
    IPv6,
};

enum class EthenetPacketType
{
    None,
    ARP,
    IPv4,
    IPv6,
};

class IPacket
{
public:
    virtual int assign(std::shared_ptr<std::vector<uint8_t>> data);
    virtual PacketType type() const = 0;
    virtual uint8_t *data() const = 0;
    virtual std::size_t size() const = 0;
};

class EthenetPacket : public IPacket
{
public:
    std::array<uint8_t, 6> mac_src;
    std::array<uint8_t, 6> mac_dst;
    EthenetPacketType type{EthenetPacketType::None};

    std::size_t offset_ethenet{0};
    std::size_t size_ethenet{0};
};

class IPv4Packet
{

};

class IPv4EthenetPacket : public EthenetPacket, public IPv4Packet
{
public:

};

class PacketProcess
{
public:
    PacketProcess();
    ~PacketProcess();
    int processEthenetPacket(std::shared_ptr<std::vector<uint8_t>> packet);
private:
    // 
    std::shared_ptr<IPacket> doProcessEthenetPacket(std::shared_ptr<IPacket> packet);
};
