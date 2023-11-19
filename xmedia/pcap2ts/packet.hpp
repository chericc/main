#pragma once

#include <memory>
#include <stdint.h>
#include <vector>
#include <array>

enum class PacketType
{
    None,
    Ethernet,
    IPv4,
    IPv6,
    Butt,
};

enum class EthernetSubType
{
    None,
    ARP,
    IPv4,
    IPv6,
    Butt,
};

class IPacket
{
public:
    virtual ~IPacket() = default;
    virtual int assign(std::shared_ptr<std::vector<uint8_t>> data) = 0;
    virtual PacketType type() const = 0;
    virtual uint8_t *data() const = 0;
    virtual std::size_t size() const = 0;

    static const char *typeName(PacketType packet_type);
};

struct EthernetStructure
{
    std::array<uint8_t, 6> mac_dst;
    std::array<uint8_t, 6> mac_src;
    uint16_t type;
};

class EthernetPacket : public IPacket
{
public:
    ~EthernetPacket() override = default;
    int assign(std::shared_ptr<std::vector<uint8_t>> data) override;
    PacketType type() const override;
    uint8_t *data() const override;
    std::size_t size() const override;

    const EthernetStructure &eth() const;

    static EthernetSubType convertEthType(uint16_t eth_type);
    static const char *subTypeName(EthernetSubType subtype);
private:
    std::shared_ptr<std::vector<uint8_t>> _data;
    EthernetStructure _eth{};
};

struct IPv4Structure
{
    /* & 0xf0 = version, & 0x0f = len */
    uint8_t version_and_len;

    uint8_t differentialted_services_field;
    uint16_t total_length;
    uint16_t identification;

    /* & 0xe0 = flag, & 0x1f = fragment_offset */
    uint16_t flag_and_fragment_offset;
    uint8_t time_to_live;
    uint8_t protocol;
    uint16_t header_checksum;

    std::array<uint8_t, 4> ip_addr_src;
    std::array<uint8_t, 4> ip_addr_dst;

    /* options */
    /* sub_data */
};

class IPv4EthernetPacket : public EthernetPacket
{
public:
    ~IPv4EthernetPacket() override = default;
    int assign(std::shared_ptr<std::vector<uint8_t>> data) override;
    PacketType type() const override;
    uint8_t *data() const override;
    std::size_t size() const override;
    
    const IPv4Structure& ipv4() const;
private:
    IPv4Structure _ipv4;
};