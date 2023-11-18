#pragma once

#include <array>
#include <string>
#include <memory>
#include <stdint.h>
#include <vector>
#include <list>
#include <string>
#include <functional>

#include "xutility.hpp"

enum class PcapngContentType
{
    Info,
    Interface,
    Data,
};

struct PcapngContentInfo
{
    std::string shb_hardware;
    std::string shb_os;
    std::string shb_userappl;
};

struct PcapngContentInterface
{
    std::string if_name;
    std::string if_description;
    std::list<std::array<uint8_t, 8>> if_IPv4addrs; // addr(4) + netmask(4)
    std::list<std::array<uint8_t, 17>> if_IPV6addrs;  // addr(16) + prefixlen(1)
    std::array<uint8_t, 6> if_MACaddr;
    std::array<uint8_t, 8> if_EUIaddr;
    uint64_t if_speed{0};  // bps
    uint8_t if_tsresol{0}; // 
    uint32_t if_tzone{0};
    std::string if_os;
    uint64_t if_tsoffset{0};
    std::string if_hardware;
    uint64_t if_txspeed{0};
    uint64_t if_rxspeed{0};
};

struct PcapngContentData
{
    std::shared_ptr<std::vector<uint8_t>> packet_data;
};

struct PcapngContent
{
    PcapngContentType type;
    
    PcapngContentInfo info;
    PcapngContentInterface interface;
    PcapngContentData data;
};

class PcapngParser : public XNonCopyableObject
{
public:
    /*
     * resolution: 
    */
    PcapngParser(const std::string &file);
    ~PcapngParser();

    int parse(std::function<void(const PcapngContent &)>);
    int goThrough();

private:
    struct InnerData;
    struct PcapngInfo;

    struct Option;
    struct Options;

    struct Block;
    struct SectionHeaderBlock;
    struct InterfaceDescriptionBlock;
    struct EnhancedPacketBlock;
    struct SimplePacketBlock;


    /* Parse options from current location. */
    std::shared_ptr<Options> doParserOptions(std::shared_ptr<PcapngInfo> info, std::size_t size);

    /* Parse from current location. */
    std::shared_ptr<Block> doProbeBlock(std::shared_ptr<PcapngInfo>);
    int doSkipBlock(std::shared_ptr<PcapngInfo>);
    std::shared_ptr<SectionHeaderBlock> doParseSHB(std::shared_ptr<PcapngInfo>);
    std::shared_ptr<InterfaceDescriptionBlock> doParseIDB(std::shared_ptr<PcapngInfo>);
    std::shared_ptr<EnhancedPacketBlock> doParseEPB(std::shared_ptr<PcapngInfo>);
    std::shared_ptr<SimplePacketBlock> doParseSPB(std::shared_ptr<PcapngInfo>);

    uint32_t round_up_4(uint32_t v32);

    uint8_t to_uint8(std::shared_ptr<PcapngInfo> info, void *data);
    uint16_t to_uint16(std::shared_ptr<PcapngInfo> info, void *data);
    uint32_t to_uint32(std::shared_ptr<PcapngInfo> info, void *data);
    uint64_t to_uint64(std::shared_ptr<PcapngInfo> info, void *data);

    std::shared_ptr<InnerData> _d;
};