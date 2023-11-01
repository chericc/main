#pragma once

#include <string>
#include <memory>

class PcapngParser
{
public:
    PcapngParser(const std::string &file);
    ~PcapngParser();

private:
    struct PcapngInfo;
    struct InnerData;
    std::shared_ptr<PcapngInfo> doParseHeader();

    std::shared_ptr<InnerData> _d;
};