#include "pcap_ng_parser.hpp"

#include <stdint.h>

#include "xio.hpp"
#include "xlog.hpp"

#define BT_SHB			0x0A0D0D0A

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
    uint32_t magic_number = 0x0;

    do 
    {
        info = std::make_shared<PcapngInfo>();
        if (!info)
        {
            xlog_err("alloc failed");
            break;
        }

        info->xio = std::make_shared<XIOFile>(_d->file, "r");
        if (!info->xio)
        {
            xlog_err("open failed");
            break;
        }

        magic_number = info->xio->rl32();
        if (magic_number != BT_SHB)
        {
            xlog_err("header magic check failed(%hx != %hx)", magic_number, BT_SHB);
            break;
        }

        
    }
    while (0);
}