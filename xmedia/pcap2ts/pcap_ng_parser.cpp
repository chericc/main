#include "pcap_ng_parser.hpp"

#include "xio.hpp"
#include "xlog.hpp"

struct PcapngParser::PcapngInfo
{
    std::shared_ptr<XIO> xio;
};

struct PcapngParser::InnerData
{
    std::string file;
};

PcapngParser::PcapngParser(const std::string &file)
{
    _d = std::make_shared<InnerData>();
    _d->file = file;
}

PcapngParser::~PcapngParser()
{

}

std::shared_ptr<PcapngParser::PcapngInfo> 
PcapngParser::doParseHeader()
{
    std::shared_ptr<PcapngInfo> info;

    do 
    {
        info = std::make_shared<PcapngInfo>();
        if (!info)
        {
            
        }
    }
    while (0);
}