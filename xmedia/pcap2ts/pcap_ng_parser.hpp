#pragma once

#include <string>
#include <memory>
#include <stdint.h>

#include "xutility.hpp" 

enum class PcapngParserResolution
{
    MICRO,
    NANO
};

class PcapngParser : public XNonCopyableObject
{
public:
    /*
     * resolution: 
    */
    PcapngParser(const std::string &file, PcapngParserResolution resolution = PcapngParserResolution::NANO);
    ~PcapngParser();

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

    std::shared_ptr<PcapngInfo> doParse();

    /* Parse options from current location. */
    std::shared_ptr<Options> doParserOptions(std::shared_ptr<PcapngInfo> info, std::size_t size);

    /* Parse from current location. */
    std::shared_ptr<Block> doProbeBlock(std::shared_ptr<PcapngInfo>);
    std::shared_ptr<SectionHeaderBlock> doParseSHB(std::shared_ptr<PcapngInfo>);
    std::shared_ptr<InterfaceDescriptionBlock> doParseIDB(std::shared_ptr<PcapngInfo>);
    std::shared_ptr<EnhancedPacketBlock> doParseEPB(std::shared_ptr<PcapngInfo>);
    std::shared_ptr<SimplePacketBlock> doParseSPB(std::shared_ptr<PcapngInfo>);

    uint32_t round_up_4(uint32_t v32);

    std::shared_ptr<InnerData> _d;
};