#pragma once

#include <memory>
#include <stdint.h>
#include <vector>
#include <array>
#include <list>

enum class PacketType
{
    None,
    Ethernet,
    ARP,
    IPv4,
    IPv6,
    Butt,
};

enum class Protocol
{
    None,
    UDP,
    TCP,
    Butt,
};

/* To avoid data coping, we can use the same memory block 
 * with different offsets and sizes.
 */
class SharedData
{
public:
    SharedData() = default;
    SharedData(std::shared_ptr<std::vector<uint8_t>> adata, std::size_t aoffset, std::size_t asize)
        : data(adata), offset(aoffset), size(asize) {}
    SharedData(SharedData const&) = default;

    std::shared_ptr<std::vector<uint8_t>> data;
    std::size_t offset{0};
    std::size_t size{0};

    bool valid() const;
    
    uint8_t *offsetData()
    {
        if (data)
        {
            return data->data() + offset;
        }
        return nullptr;
    }
    std::size_t offsetSize()
    {
        return size;
    }
};

class PacketInfo
{
public:
    virtual ~PacketInfo() = 0;

    static const char *typeName(PacketType packet_type);
};

class EthernetPacketInfo : public PacketInfo
{
public:
    EthernetPacketInfo() = default;
    ~EthernetPacketInfo() override = default;

    std::array<uint8_t, 6> mac_dst;
    std::array<uint8_t, 6> mac_src;
    uint16_t type = 0;

    static PacketType convertEthType(uint16_t eth_type);
};

class IPv4PacketInfo : public PacketInfo
{
public:
    ~IPv4PacketInfo() override = default;

    /* & 0xf0 = version, & 0x0f = len */
    uint8_t version_and_len;
    uint8_t version; // not in header
    uint8_t len; // not in header

    uint8_t differentialted_services_field;
    uint16_t total_length;
    uint16_t identification;

    /* & 0xe0 = flag, & 0x1f = fragment_offset */
    uint16_t flag_and_fragment_offset;
    uint16_t flag; // not in header
    uint16_t fragment_offset; // not in header

    uint8_t time_to_live;
    uint8_t protocol;
    uint16_t header_checksum;

    std::array<uint8_t, 4> ip_addr_src;
    std::array<uint8_t, 4> ip_addr_dst;

    /* options */
    /* sub_data */

    std::size_t padding_size; // not in header
    
    static uint8_t ipv4_version(uint8_t version_and_len);
    static uint8_t ipv4_len(uint8_t version_and_len);
    static uint16_t ipv4_flag(uint16_t flag_and_fragment_offset);
    static uint16_t ipv4_fragment_offset(uint16_t flag_and_fragment_offset);
    static Protocol convertProtocol(uint8_t ipv4_protocol);
};

class SharedPacket
{
public:
    SharedPacket() = default;
    SharedPacket(SharedPacket const&) = default;
    SharedPacket(PacketType a_cur_type, SharedData a_data)
        : cur_type(a_cur_type), data(a_data) {};
    PacketType cur_type;
    SharedData data;
    std::list<std::pair<PacketType, std::shared_ptr<PacketInfo>>> parsed_info;
};

#if 0
class IPacket
{
public:
    virtual ~IPacket() = default;
    virtual int assign(SharedData data) = 0;
    virtual PacketType type() const = 0;
    virtual SharedData data() const = 0;

    static const char *typeName(PacketType packet_type);
};

class EthernetPacket : public IPacket
{
public:
    ~EthernetPacket() override = default;
    int assign(SharedData data) override;
    PacketType type() const override;
    SharedData data() const override;

    const EthernetPacketInfo &eth() const;

    static EthernetSubType convertEthType(uint16_t eth_type);
    static const char *subTypeName(EthernetSubType subtype);
private:
    /* Ethernet packet data(without Ethernet packet header) */
    SharedData _data;
    EthernetPacketInfo _eth{};
};

class IPv4EthernetPacket : public EthernetPacket
{
public:
    ~IPv4EthernetPacket() override = default;
    int assign(SharedData data) override;
    PacketType type() const override;
    SharedData data() const override;
    
    const IPv4PacketInfo& ipv4() const;
    static uint8_t ipv4_version(uint8_t version_and_len);
    static uint8_t ipv4_len(uint8_t version_and_len);
    static uint16_t ipv4_flag(uint16_t flag_and_fragment_offset);
    static uint16_t ipv4_fragment_offset(uint16_t flag_and_fragment_offset);
    static Protocol convertProtocol(uint8_t ipv4_protocol);
private:
    /* IPv4 packet data(without IPv4 packet header) */
    SharedData _data;
    IPv4PacketInfo _ipv4{};
};
#endif 