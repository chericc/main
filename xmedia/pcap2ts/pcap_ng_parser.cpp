#include "pcap_ng_parser.hpp"

#include <stdint.h>
#include <limits>
#include <inttypes.h>

#include "fileio.hpp"
#include "xlog.hpp"
#include "xutility.hpp"


#define BYTE_ORDER_MAGIC    0x1A2B3C4D

/*
 * Current version number.  If major_version isn't PCAP_NG_VERSION_MAJOR,
 * or if minor_version isn't PCAP_NG_VERSION_MINOR or 2, that means that
 * this code can't read the file.
 */
#define PCAP_NG_VERSION_MAJOR	1
#define PCAP_NG_VERSION_MINOR	0

/*
 * Section Header Block.
 * Note: BT_SHB is symmetric
 */
#define BT_SHB			    0x0A0D0D0A
#define BT_SHB_INSANE_MAX       1024U*1024U*1U  /* 1MB should be enough */

struct PcapngParser::Block
{
    uint32_t block_type;
};

/* SHB */
struct PcapngParser::SectionHeaderBlock
{
    uint32_t block_type;
    uint32_t block_length;
    uint32_t byte_order_magic;
    uint16_t major_version;
    uint16_t minor_version;
    uint64_t section_length;
    /* options.. */
    uint32_t block_length_trailing;
};

/* IDB */
struct PcapngParser::InterfaceDescriptionBlock
{
    uint32_t block_type;
    uint32_t block_total_length;
    uint16_t link_type;
    uint16_t reserved;
    uint32_t snap_length;
    /* options.. */
    uint32_t block_total_length_trailing;
};

/* PB */
struct PcapngParser::PacketBlock
{
    uint32_t block_type;
    uint32_t block_total_length;
    uint32_t interface_id;
    uint32_t timestamp_upper;
    uint32_t timestamp_lower;
    uint32_t captured_packet_length;
    uint32_t original_packet_length;
    /* packet data.. */
    /* options.. */
    uint32_t block_total_length_trailing;
};

/*
 * The maximum block size we start with; we use an arbitrary value of
 * 16 MiB.
 */
#define INITIAL_MAX_BLOCKSIZE	(16*1024*1024)

struct PcapngParser::PcapngInfo
{
    std::shared_ptr<FileIO> fio;
};

struct PcapngParser::InnerData
{
    std::string file;
    PcapngParserResolution res{};
};

PcapngParser::PcapngParser(const std::string &file, PcapngParserResolution resolution)
{
    _d = std::make_shared<InnerData>();
    _d->file = file;
    _d->res = resolution;

    doParse();
}

PcapngParser::~PcapngParser()
{

}

std::shared_ptr<PcapngParser::PcapngInfo> 
PcapngParser::doParse()
{
    bool error = false;
    std::shared_ptr<PcapngInfo> info;

    do 
    {
        info = std::make_shared<PcapngInfo>();
        if (!info)
        {
            xlog_err("Alloc failed");
            error = true;
            break;
        }

        info->fio = std::make_shared<FileIO>(_d->file, "rb");
        if (!info->fio || !info->fio->ok())
        {
            xlog_err("Open failed");
            error = true;
            break;
        }
        info->fio->setLittleEndian(true);

        for (;;)
        {
            auto block = doProbeBlock(info);
            if (!block)
            {
                xlog_dbg("Probe end");
                break;
            }

            if (BT_SHB == block->block_type)
            {
                xlog_trc("Parsing SHB...");
                auto shb = doParseSHB(info);
                if (!shb)
                {
                    xlog_err("Parse SHB failed");
                    continue;
                }
            }
        }
    }
    while (0);

    if (error)
    {
        return nullptr;
    }

    return info;
}

std::shared_ptr<PcapngParser::Block> PcapngParser::doProbeBlock(std::shared_ptr<PcapngInfo> info)
{
    bool error= false;
    std::shared_ptr<PcapngParser::Block> block;

    do
    {
        block = std::make_shared<Block>();
        if (!block)
        {
            xlog_err("Alloc failed");
            error = true;
            break;
        }

        block->block_type = info->fio->r32();
    }
    while(0);

    if (error)
    {
        return nullptr;
    }
    return block;
}

std::shared_ptr<PcapngParser::SectionHeaderBlock> PcapngParser::doParseSHB(std::shared_ptr<PcapngInfo> info)
{
    bool error = false;
    std::shared_ptr<SectionHeaderBlock> shb;

    do
    {
        shb = std::make_shared<SectionHeaderBlock>();
        if (!shb)
        {
            xlog_err("Alloc failed");
            error = true;
            break;
        }

        shb->block_type = info->fio->r32();
        shb->block_length = info->fio->r32();
        shb->byte_order_magic = info->fio->r32();
        if (BYTE_ORDER_MAGIC != shb->byte_order_magic)
        {
            if (BYTE_ORDER_MAGIC != xswap(shb->byte_order_magic))
            {
                xlog_err("Magic check failed");
                error = true;
                break;
            }
            else 
            {
                info->fio->setLittleEndian(false);
                shb->block_length = xswap(shb->block_length);
                shb->byte_order_magic = xswap(shb->byte_order_magic);
            }
        }
        else 
        {
            /* default case, do nothing */
        }

        if (shb->block_length < sizeof(SectionHeaderBlock)
            || shb->block_length > BT_SHB_INSANE_MAX)
        {
            xlog_err("SHB.block_length invalid: %" PRIu32, shb->block_length);
            error = true;
            break;
        }

        shb->major_version = info->fio->r16();
        shb->minor_version = info->fio->r16();

        if (! (shb->major_version == PCAP_NG_VERSION_MAJOR 
                && (shb->minor_version == PCAP_NG_VERSION_MINOR
                    || shb->minor_version == 2)))
        {
            xlog_err("Version not support: %" PRIu16 ".%" PRIu16, 
                shb->major_version, shb->minor_version);
            error = true;
            break;
        }


    }
    while (0);

    if (error)
    {
        return nullptr;
    }

    return shb;
}