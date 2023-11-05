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

    struct Block;
    struct SectionHeaderBlock;
    struct InterfaceDescriptionBlock;
    struct PacketBlock;

    std::shared_ptr<PcapngInfo> doParse();

    /* Parse from current location. */
    std::shared_ptr<Block> doProbeBlock(std::shared_ptr<PcapngInfo>);
    std::shared_ptr<SectionHeaderBlock> doParseSHB(std::shared_ptr<PcapngInfo>);
    std::shared_ptr<InterfaceDescriptionBlock> doParseIDB(std::shared_ptr<PcapngInfo>);

    std::shared_ptr<InnerData> _d;
};