#include "pcap_ng_parser.hpp"

#include <stdint.h>
#include <limits>
#include <inttypes.h>
#include <list>

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

#define BT_SHB			    0x0A0D0D0A      /* Secton Header Block */
#define BT_IDB              0x00000001      /* Interface Description Block */
#define BT_EPB              0x00000006      /* Enhanced Packet Block */
#define BT_SPB              0x00000003      /* Sinple Packet Block */
#define BT_NRB              0x00000004      /* Name Resolution Block */
#define BT_ISB              0x00000005      /* Interface Statistics Block */
#define BT_DSB              0x0000000A      /* Decryption Secrets Block */
#define BT_CB_1             0x00000BAD      /* Custom Block 1 */
#define BT_CB_2             0x40000BAD      /* Csutom Block 2 */
#define BT_PB               0x00000002      /* Packet Block. (obsolete) */

#define BT_SHB_INSANE_MAX       (1024U*1024U*1U)  /* 1MB should be enough */
#define PACKET_INSACE_MAX       (16U*1024U*1024U)

struct PcapngParser::Option
{
    uint16_t type;
    uint16_t option_len;

    /* May be store offset only is also OK. */
    std::vector<uint8_t> data;
};

struct PcapngParser::Options
{
    std::list<Option> options;
};

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
    std::shared_ptr<Options> options;
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
    std::shared_ptr<Options> options;
    uint32_t block_total_length_trailing;
};

/* EPB */
struct PcapngParser::EnhancedPacketBlock
{
    uint32_t block_type;
    uint32_t block_total_length;
    uint32_t interface_id;
    uint32_t timestamp_high;
    uint32_t timestamp_low;
    uint32_t captured_packet_length;
    uint32_t original_packet_length;
    std::vector<uint8_t> packet_data;
    std::shared_ptr<Options> options;
    uint32_t block_total_length_trailing;
};

/* SPB */
struct PcapngParser::SimplePacketBlock
{
    uint32_t block_type;
    uint32_t block_total_length;
    uint32_t original_packet_length;
    std::vector<uint8_t> packet_data;
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
            auto offset_now = info->fio->tell();
            xlog_dbg("offset_now=%" PRIu64, offset_now);

            auto block = doProbeBlock(info);
            if (!block)
            {
                xlog_dbg("Probe end");
                break;
            }

            if (BT_SHB == block->block_type)
            {
                xlog_dbg("Parsing SHB...");
                auto shb = doParseSHB(info);
                if (!shb)
                {
                    xlog_err("Parse SHB failed");
                    break;
                }
                xlog_dbg("Parsing SHB successful");
            }
            else if (BT_IDB == block->block_type)
            {
                xlog_dbg("Parsing IDB...");
                auto idb = doParseIDB(info);
                if (!idb)
                {
                    xlog_err("Parsing IDB failed");
                    break;
                }
                xlog_dbg("Parsing IDB successful");
            }
            else if (BT_EPB == block->block_type)
            {
                xlog_dbg("Parsing EPG...");
                auto epb = doParseEPB(info);
                if (!epb)
                {
                    xlog_err("Parsing EPB failed");
                    break;
                }
                xlog_dbg("Parsing EPG successful");
            }
            else if (BT_SPB == block->block_type)
            {
                xlog_dbg("Parsing SPB...");
                auto spb = doParseSPB(info);
                if (!spb)
                {
                    xlog_err("Parsing SPB failed");
                    break;
                }
                xlog_dbg("Parsing SPB successful");
            }
            else 
            {
                xlog_err("Unknown block type(%#" PRIx32 ")", block->block_type);
                break;
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

std::shared_ptr<PcapngParser::Options> 
PcapngParser::doParserOptions(std::shared_ptr<PcapngParser::PcapngInfo> info, std::size_t options_size)
{
    xlog_dbg("Parsing options");

    bool error = false;
    std::shared_ptr<PcapngParser::Options> options;

    int64_t pos_initial = info->fio->tell();

    do 
    {
        options = std::make_shared<Options>();
        if (!options)
        {
            xlog_err("Alloc failed");
            break;
        }

        if (!options_size)
        {
            xlog_dbg("Empty option size");
            break;
        }

        for (int i = 0;; ++i)
        {
            Option option;
            std::size_t minium_option_size = sizeof(option.type) + sizeof(option.option_len);
            int64_t pos_now = info->fio->tell();

            xlog_dbg("option: (posnow:%" PRId64 ", end:%" PRId64 ")", 
                pos_now, pos_initial + options_size);

            if (pos_initial + (int64_t)options_size == pos_now)
            {
                xlog_dbg("Option size end met");
                break;
            }

            if (pos_now + (int64_t)minium_option_size > pos_initial + (int64_t)options_size)
            {
                xlog_dbg("Option parse with no end");
                error = true;
                break;
            }

            option.type = info->fio->r16();
            option.option_len = info->fio->r16();

            /* opt_endofopt */
            if (!option.type && !option.option_len)
            {
                pos_now = info->fio->tell();
                if (pos_now != (int64_t)(pos_initial + options_size))
                {
                    xlog_err("Option end unexpectly");
                    error = true;
                    break;
                }

                xlog_dbg("Option end tag met");
                break;
            }

            xlog_dbg("Option(%d): type=%" PRIu16 ", len=%" PRIu16, i, option.type, option.option_len);

            option.data = info->fio->read(option.option_len);
            if (option.data.size() != option.option_len)
            {
                xlog_err("Read option data failed");
                error = true;
                break;
            }

            uint32_t option_len_round_up_4 = round_up_4(option.option_len);
            int32_t option_padding_size = option_len_round_up_4 - option.option_len;
            if (option_padding_size < 0)
            {
                xlog_err("Inner error");
                error = true;
                break;
            }
            if (option_padding_size > 0)
            {
                xlog_dbg("Skip padding size(%" PRId32 ")", option_padding_size);
                info->fio->seek(option_padding_size, SEEK_CUR);
            }

            options->options.push_back(std::move(option));
        }
    }
    while (0);

    if (error)
    {
        xlog_err("Parsing options failed");
        return nullptr;
    }
    xlog_dbg("Parsing options successful(options num=%zu)", options->options.size());

    return options;
}

std::shared_ptr<PcapngParser::Block> PcapngParser::doProbeBlock(std::shared_ptr<PcapngInfo> info)
{
    bool error = false;
    std::shared_ptr<PcapngParser::Block> block;
    uint64_t offset = 0;

    do
    {
        block = std::make_shared<Block>();
        if (!block)
        {
            xlog_err("Alloc failed");
            error = true;
            break;
        }

        offset = info->fio->tell();

        block->block_type = info->fio->r32();

        /* Restore offset. */
        info->fio->seek(offset, SEEK_SET);
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

        /* size without options */
        constexpr uint32_t other_option_len = 
            (sizeof(shb->block_type) + sizeof(shb->block_length)
                + sizeof(shb->byte_order_magic) + sizeof(shb->major_version)
                + sizeof(shb->minor_version) + sizeof(shb->section_length)
                + sizeof(shb->block_length_trailing));

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
            /* Default case, do nothing */
        }

        if (shb->block_length < other_option_len
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

        /* Not important. */
        shb->section_length = info->fio->r64();
        
        /* block_length >= other_option_len has been promised */
        uint32_t option_len = shb->block_length - other_option_len;
        shb->options = doParserOptions(info, option_len);
        if (!shb->options)
        {
            xlog_err("Parse options failed");
            error = true;
            break;
        }

        shb->block_length_trailing = info->fio->r32();
        if (shb->block_length_trailing != shb->block_length)
        {
            xlog_err("Trailing length != length");
            error = true;
            break;
        }

    }
    while (0);

    if (error)
    {
        return nullptr;
    }

    xlog_dbg("Parsing SHB successful(block_len=%" PRIu32 ")", shb->block_length);

    return shb;
}

std::shared_ptr<PcapngParser::InterfaceDescriptionBlock> PcapngParser::doParseIDB(std::shared_ptr<PcapngInfo> info)
{
    bool error = false;
    std::shared_ptr<InterfaceDescriptionBlock> idb;

    do 
    {

        idb = std::make_shared<InterfaceDescriptionBlock>();
        if (!idb)
        {
            xlog_err("Alloc failed");
            error = true;
            break;
        }

        /* size without options */
        constexpr uint32_t other_option_len = 
            (sizeof(idb->block_type) + sizeof(idb->block_total_length) + sizeof(idb->link_type)
                + sizeof(idb->reserved) + sizeof(idb->snap_length) + sizeof(idb->block_total_length_trailing));

        idb->block_type = info->fio->r32();
        if (idb->block_type != BT_IDB)
        {
            xlog_err("Block type error");
            error = true;
            break;
        }

        idb->block_total_length = info->fio->r32();
        idb->link_type = info->fio->r16();
        idb->reserved = info->fio->r16();
        idb->snap_length = info->fio->r32();

        if (idb->block_total_length < other_option_len)
        {
            xlog_err("Block length too small");
            error = true;
            break;
        }

        uint32_t option_len = idb->block_total_length - other_option_len;
        idb->options = doParserOptions(info, option_len);
        if (!idb->options)
        {
            xlog_err("Parse options failed");
            error = true;
            break;
        }

        idb->block_total_length_trailing = info->fio->r32();
        if (idb->block_total_length_trailing != idb->block_total_length)
        {
            xlog_err("IDB trailing length != length");
            error = true;
            break;
        }
    }
    while (0);

    if (error)
    {
        return nullptr;
    }
    return idb;
}

/* 整数向上4对齐 */
uint32_t PcapngParser::round_up_4(uint32_t v32)
{
    uint32_t ret = (v32 / 4) * 4 + ((v32 % 4) ? 4 : 0);
    return ret;
}

std::shared_ptr<PcapngParser::EnhancedPacketBlock> PcapngParser::doParseEPB(std::shared_ptr<PcapngInfo> info)
{
    bool error = false;
    std::shared_ptr<EnhancedPacketBlock> epb;

    do 
    {
        epb = std::make_shared<EnhancedPacketBlock>();
        if (!epb)
        {
            xlog_err("Alloc failed");
            error = true;
            break;
        }

        epb->block_type = info->fio->r32();
        epb->block_total_length = info->fio->r32();

        if (epb->block_type != BT_EPB)
        {
            xlog_err("Block type check failed");
            error = true;
            break;
        }

        epb->interface_id = info->fio->r32();
        epb->timestamp_high = info->fio->r32();
        epb->timestamp_low = info->fio->r32();
        epb->captured_packet_length = info->fio->r32();
        epb->original_packet_length = info->fio->r32();

        if (epb->captured_packet_length != epb->original_packet_length)
        {
            xlog_err("Packet truncated: (%" PRIu32" != %" PRIu32 ")", 
                epb->captured_packet_length, epb->original_packet_length);
        }

        if (epb->captured_packet_length > PACKET_INSACE_MAX)
        {
            xlog_err("Packet too big(%" PRIu32 ")", epb->captured_packet_length);
            error = true;
            break;
        }

        epb->packet_data = info->fio->read(epb->captured_packet_length);
        if (epb->packet_data.size() != epb->captured_packet_length)
        {
            xlog_err("Read packet data failed");
            error = true;
            break;
        }

        uint32_t data_with_padding_4_size = round_up_4(epb->captured_packet_length);
        if (data_with_padding_4_size < epb->captured_packet_length)
        {
            xlog_err("Inner error");
            error = true;
            break;
        }

        uint32_t padding_size = data_with_padding_4_size - epb->captured_packet_length;
        if (padding_size > 0)
        {
            xlog_dbg("Skipping data padding(%" PRIu32 ")", padding_size);
            info->fio->seek(padding_size, SEEK_CUR);
        }

        uint32_t other_option_len = sizeof(epb->block_type) + sizeof(epb->block_total_length)
            + sizeof(epb->interface_id) + sizeof(epb->timestamp_high) + sizeof(epb->timestamp_low)
            + sizeof(epb->captured_packet_length) + sizeof(epb->original_packet_length)
            + data_with_padding_4_size + sizeof(epb->block_total_length_trailing);
        if (other_option_len > epb->block_total_length)
        {
            xlog_err("Total length check failed");
            error = true;
            break;
        }

        uint32_t option_len = epb->block_total_length - other_option_len;

        epb->options = doParserOptions(info, option_len);
        if (!epb->options)
        {
            xlog_err("Parse options failed");
            error = true;
            break;
        }

        epb->block_total_length_trailing = info->fio->r32();
        if (epb->block_total_length != epb->block_total_length_trailing)
        {
            xlog_err("Total length trailing error(%" PRIu32 ", %" PRIu32 ")", 
                epb->block_total_length, epb->block_total_length_trailing);
            error = true;
            break;
        }
    }
    while (0);

    if (error)
    {
        return nullptr;
    }
    return epb;
}

std::shared_ptr<PcapngParser::SimplePacketBlock> PcapngParser::doParseSPB(std::shared_ptr<PcapngInfo> info)
{
    bool error = false;
    std::shared_ptr<SimplePacketBlock> spb;

    do 
    {
        spb = std::make_shared<SimplePacketBlock>();
        if (!spb)
        {
            xlog_err("Alloc failed");
            error = true;
            break;
        }

        spb->block_type = info->fio->r32();
        spb->block_total_length = info->fio->r32();
        spb->original_packet_length = info->fio->r32();

        if (spb->block_total_length > PACKET_INSACE_MAX)
        {
            xlog_err("Total length check failed: too big(%" PRIu32 ")", spb->block_total_length);
            error = true;
            break;
        }

        uint32_t other_data_size = sizeof(spb->block_type) + sizeof(spb->block_total_length)
            + sizeof(spb->original_packet_length) + sizeof(spb->block_total_length_trailing);

        if (spb->block_total_length < other_data_size)
        {
            xlog_err("Total length check failed: too small(%" PRIu32 ")", spb->block_total_length);
            error = true;
            break;
        }

        uint32_t packet_data_size = spb->block_total_length - other_data_size;
        spb->packet_data = info->fio->read(packet_data_size);
        if (spb->packet_data.size() != packet_data_size)
        {
            xlog_err("Read failed");
            error = true;
            break;
        }

        spb->block_total_length_trailing = info->fio->r32();
        if (spb->block_total_length_trailing != spb->block_total_length)
        {
            xlog_err("Trailing length not equal to length");
            error = true;
            break;
        }
    }
    while (0);

    if (error)
    {
        return nullptr;
    }
    return spb;
}