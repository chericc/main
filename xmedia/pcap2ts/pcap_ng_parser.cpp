#include "pcap_ng_parser.hpp"

#include <inttypes.h>
#include <cstring>

#include "fileio.hpp"
#include "xlog.hpp"


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

#define SHB_OPTION_HARDWARE         (2)
#define SHB_OPTION_OS               (3)
#define SHB_OPTION_USERAPPL         (4)

#define IDB_OPTION_NAME             (2)
#define IDB_OPTION_DESCRIPTION      (3)
#define IDB_OPTION_IPV4ADDR         (4)
#define IDB_OPTION_IPV6ADDR         (5)
#define IDB_OPTION_MACADDR          (6)
#define IDB_OPTION_EUIADDR          (7)
#define IDB_OPTION_SPEED            (8)
#define IDB_OPTION_TSRESOL          (9)
#define IDB_OPTION_TZONE            (10)
#define IDB_OPTION_FILTER           (11)
#define IDB_OPTION_OS               (12)
#define IDB_OPTION_FCSLEN           (13)
#define IDB_OPTION_TSOFFSET         (14)
#define IDB_OPTION_HARDWARE         (15)
#define IDB_OPTION_TXSPEED          (16)
#define IDB_OPTION_RXSPEED          (17)

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
    uint32_t block_length;
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
    std::shared_ptr<std::vector<uint8_t>> packet_data;
    std::shared_ptr<Options> options;
    uint32_t block_total_length_trailing;
};

/* SPB */
struct PcapngParser::SimplePacketBlock
{
    uint32_t block_type;
    uint32_t block_total_length;
    uint32_t original_packet_length;
    std::shared_ptr<std::vector<uint8_t>> packet_data;
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

    /* 用于内部将内存数据转换为整数值 */
    bool littleEndian;
};

struct PcapngParser::InnerData
{
    std::string file;
};

PcapngParser::PcapngParser(const std::string &file)
{
    _d = std::make_shared<InnerData>();
    _d->file = file;

    goThrough();
}

PcapngParser::~PcapngParser()
{

}

int PcapngParser::parse(std::function<void(const PcapngContent &)> callback)
{
    bool error = false;
    bool parse_end = false;
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
        info->littleEndian = true;
        info->fio->setLittleEndian(true);

        // iterate sections
        for(;;)
        {
            auto block_probe = doProbeBlock(info);
            if (!block_probe)
            {
                xlog_err("Probe end");
                break;
            }

            if (block_probe->block_type != BT_SHB)
            {
                xlog_err("Not shb\n");
                error = true;
                break;
            }

            PcapngContent content;

            auto shb = doParseSHB(info);
            if (!shb)
            {
                xlog_err("Parse shb failed");
                error = true;
                break;
            }

            for (auto it = shb->options->options.begin(); it != shb->options->options.end(); ++it)
            {
                switch (it->type)
                {
                    case SHB_OPTION_HARDWARE:
                    {
                        content.info.shb_hardware.assign((const char*)it->data.data(), it->data.size());
                        break;
                    }
                    case SHB_OPTION_OS:
                    {
                        content.info.shb_os.assign((const char*)it->data.data(), it->data.size());
                        break;
                    }
                    case SHB_OPTION_USERAPPL:
                    {
                        content.info.shb_userappl.assign((const char*)it->data.data(), it->data.size());
                        break;
                    }
                    default:
                    {
                        xlog_dbg("Unknown options(%" PRIu16 ")", it->type);
                        break;
                    }
                }
            }

            content.type = PcapngContentType::Info;
            callback(content);

            /* iterate idbs and packets */
            for(;;)
            {
                block_probe = doProbeBlock(info);
                if (!block_probe)
                {
                    xlog_dbg("Probe end");
                    parse_end = true;
                    break;
                }

                if (BT_IDB == block_probe->block_type)
                {
                    auto idb = doParseIDB(info);
                    if (!idb)
                    {
                        xlog_err("Parse IDB failed");
                        break;
                    }

                    for (auto it = idb->options->options.begin(); it != idb->options->options.end(); ++it)
                    {
                        switch (it->type)
                        {
                            case IDB_OPTION_NAME:
                            {
                                content.interface.if_name.assign((const char*)it->data.data(), it->data.size());
                                break;
                            }
                            case IDB_OPTION_DESCRIPTION:
                            {
                                content.interface.if_description.assign((const char*)it->data.data(), it->data.size());
                                break;
                            }
                            case IDB_OPTION_IPV4ADDR:
                            {
                                if (it->data.size() != content.interface.if_IPv4addr.size())
                                {
                                    xlog_err("option.v4 size error");
                                    error = true;
                                    break;
                                }
                                memcpy(content.interface.if_IPv4addr.data(), it->data.data(), it->data.size());
                                break;
                            }
                            case IDB_OPTION_IPV6ADDR:
                            {
                                std::array<uint8_t, 17> arr;
                                if (it->data.size() != arr.size())
                                {
                                    xlog_err("option.v6 size error");
                                    error = true;
                                    break;
                                }
                                memcpy(arr.data(), it->data.data(), it->data.size());
                                content.interface.if_IPV6addr.push_back(std::move(arr));
                                break;
                            }
                            case IDB_OPTION_MACADDR:
                            {
                                if (it->data.size() != content.interface.if_MACaddr.size())
                                {
                                    xlog_err("option.mac size error");
                                    error = true;
                                    break;
                                }
                                memcpy(content.interface.if_MACaddr.data(), it->data.data(), it->data.size());
                                break;
                            }
                            case IDB_OPTION_EUIADDR:
                            {
                                if (it->data.size() != content.interface.if_EUIaddr.size())
                                {
                                    xlog_err("option.euiaddr size error");
                                    error = true;
                                    break;
                                }
                                memcpy(content.interface.if_EUIaddr.data(), it->data.data(), it->data.size());
                                break;
                            }
                            case IDB_OPTION_SPEED:
                            {
                                if (it->data.size() != sizeof(content.interface.if_speed))
                                {
                                    xlog_err("option.speed size error");
                                    error = true;
                                    break;
                                }
                                static_assert(sizeof(content.interface.if_speed) == 8);
                                content.interface.if_speed = to_uint64(info, it->data.data());
                                break;
                            }
                            case IDB_OPTION_TSRESOL:
                            {
                                if (it->data.size() != sizeof(content.interface.if_tsresol))
                                {
                                    xlog_err("option.tsresol size error");
                                    error = true;
                                    break;
                                }
                                static_assert(sizeof(content.interface.if_tsresol) == 1);
                                content.interface.if_tsresol = to_uint8(info, it->data.data());
                                break;
                            }
                            case IDB_OPTION_TZONE:
                            {
                                if (it->data.size() != sizeof(content.interface.if_tzone))
                                {
                                    xlog_err("option.tzone size error");
                                    error = true;
                                    break;
                                }
                                static_assert(sizeof(content.interface.if_tzone) == 4);
                                content.interface.if_tzone = to_uint32(info, it->data.data());
                                break;
                            }
                            case IDB_OPTION_OS:
                            {
                                content.interface.if_os.assign((const char *)it->data.data(), it->data.size());
                                break;
                            }
                            case IDB_OPTION_TSOFFSET:
                            {
                                if (it->data.size() != sizeof(content.interface.if_tsoffset))
                                {
                                    xlog_err("option.tsoffset size error");
                                    error = true;
                                    break;
                                }
                                static_assert(sizeof(content.interface.if_tsoffset) == 8);
                                content.interface.if_tsoffset = to_uint64(info, it->data.data());
                                break;
                            }
                            case IDB_OPTION_HARDWARE:
                            {
                                content.interface.if_hardware.assign((const char *)it->data.data(), it->data.size());
                                break;
                            }
                            case IDB_OPTION_TXSPEED:
                            {
                                if (it->data.size() != sizeof(content.interface.if_txspeed))
                                {
                                    xlog_err("option.txspeed size error");
                                    error = true;
                                    break;
                                }
                                static_assert(sizeof(content.interface.if_txspeed) == 8);
                                content.interface.if_txspeed = to_uint64(info, it->data.data());
                                break;
                            }
                            case IDB_OPTION_RXSPEED:
                            {
                                if (it->data.size() != sizeof(content.interface.if_rxspeed))
                                {
                                    xlog_err("option.rxspeed size error");
                                    error = true;
                                    break;
                                }
                                static_assert(sizeof(content.interface.if_rxspeed) == 8);
                                content.interface.if_rxspeed = to_uint64(info, it->data.data());
                                break;
                            }
                            default:
                            {
                                xlog_err("Option not support(%" PRIu16 ")", it->type);
                                break;
                            }
                        }
                    }

                    if (error)
                    {
                        xlog_err("Error happened parsing options");
                        break;
                    }
                    
                    content.type = PcapngContentType::Interface;
                    callback(content);
                }
                else if (BT_SPB == block_probe->block_type)
                {
                    auto spb = doParseSPB(info);
                    if (!spb)
                    {
                        xlog_err("Parse SPB failed");
                        error = true;
                        break;
                    }
                    content.data.packet_data = spb->packet_data;
                    content.type = PcapngContentType::Data;
                    callback(content);
                }
                else if (BT_EPB == block_probe->block_type)
                {
                    auto epb = doParseEPB(info);
                    if (!epb)
                    {
                        xlog_err("Parse EPB failed");
                        error = true;
                        break;
                    }
                    content.data.packet_data = epb->packet_data;
                    content.type = PcapngContentType::Data;
                    callback(content);
                }
                else 
                {
                    xlog_dbg("Not interested block type, skipping it");
                    if (doSkipBlock(info) < 0)
                    {
                        xlog_err("Skipping block failed");
                        error = true;
                        break;
                    }
                }
            } // iterate idbs and packets

            if (parse_end)
            {
                xlog_dbg("Parse end, break");
                break;
            }
        } // iterate shbs
    }
    while (0);

    if (error)
    {
        return -1;
    }
    return 0;
}

int PcapngParser::goThrough()
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
        info->littleEndian = true;
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
                xlog_dbg("Unknown block type(%#" PRIx32 ")", block->block_type);
                if (doSkipBlock(info) < 0)
                {
                    xlog_err("Skipping block failed");
                    break;
                }
                xlog_dbg("Skipping block successful");
                break;
            }
        }
    }
    while (0);

    if (error)
    {
        return -1;
    }

    return 0;
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
        block->block_length = info->fio->r32();

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

int PcapngParser::doSkipBlock(std::shared_ptr<PcapngInfo> info)
{
    bool error = false;

    do
    {
        Block block;

        block.block_type = info->fio->r32();
        block.block_length = info->fio->r32();

        uint32_t head_len = sizeof(block.block_type) + sizeof(block.block_length);
        if (block.block_length < head_len)
        {
            xlog_err("Block length check failed");
            error = true;
            break;
        }

        uint32_t extra_head_len = block.block_length - head_len;
        if (extra_head_len > 0)
        {
            xlog_dbg("Skipping block(forward size %" PRIu32 ")", extra_head_len);
            info->fio->seek(extra_head_len, SEEK_CUR);
        }
    }
    while (0);

    if (error)
    {
        return -1;
    }
    return 0;
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
                info->littleEndian = false;
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

        epb->packet_data = std::make_shared<std::vector<uint8_t>>();
        if (!epb->packet_data)
        {
            xlog_err("Alloc failed");
            error = true;
            break;
        }

        *epb->packet_data = info->fio->read(epb->captured_packet_length);
        if (epb->packet_data->size() != epb->captured_packet_length)
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

        spb->packet_data = std::make_shared<std::vector<uint8_t>>();
        if (!spb->packet_data)
        {
            xlog_err("Alloc failed");
            error = true;
            break;
        }

        uint32_t packet_data_size = spb->block_total_length - other_data_size;
        *spb->packet_data = info->fio->read(packet_data_size);
        if (spb->packet_data->size() != packet_data_size)
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

uint8_t PcapngParser::to_uint8(std::shared_ptr<PcapngInfo> info, void *data)
{
    return *(uint8_t*)(data);
}

uint16_t PcapngParser::to_uint16(std::shared_ptr<PcapngInfo> info, void *data)
{
    uint16_t value = 0;
    uint8_t *pdata = (uint8_t*)data;
    if (info->littleEndian)
    {
        value = (uint16_t)(pdata[0])
            + ((uint16_t)(pdata[1]) << 8);
    }
    else 
    {
        value = ((uint16_t)(pdata[0]) << 8)
            + (uint16_t)(pdata[1]);
    }
    return value;
}

uint32_t PcapngParser::to_uint32(std::shared_ptr<PcapngInfo> info, void *data)
{
    uint32_t value = 0;
    uint8_t *pdata = (uint8_t*)data;
    if (info->littleEndian)
    {
        value = (uint32_t)(pdata[0])
            + ((uint32_t)(pdata[1]) << 8)
            + ((uint32_t)(pdata[2]) << 16)
            + ((uint32_t)(pdata[3]) << 32);
    }
    else 
    {
        value = ((uint32_t)(pdata[0]) << 24)
            + ((uint32_t)(pdata[1]) << 16)
            + ((uint32_t)(pdata[2]) << 8)
            + (uint32_t)(pdata[3]);
    }
    return value;
}

uint64_t PcapngParser::to_uint64(std::shared_ptr<PcapngInfo> info, void *data)
{
    uint64_t value = 0;
    uint8_t *pdata = (uint8_t*)data;
    if (info->littleEndian)
    {
        value = (uint64_t)(pdata[0])
            + ((uint64_t)(pdata[1]) << 8)
            + ((uint64_t)(pdata[2]) << 16)
            + ((uint64_t)(pdata[3]) << 24)
            + ((uint64_t)(pdata[4]) << 32)
            + ((uint64_t)(pdata[5]) << 40)
            + ((uint64_t)(pdata[6]) << 48)
            + ((uint64_t)(pdata[7]) << 56);
    }
    else 
    {
        value = ((uint64_t)(pdata[0]) << 56)
            + ((uint64_t)(pdata[1]) << 48)
            + ((uint64_t)(pdata[2]) << 40)
            + ((uint64_t)(pdata[3]) << 32)
            + ((uint64_t)(pdata[4]) << 24)
            + ((uint64_t)(pdata[5]) << 16)
            + ((uint64_t)(pdata[6]) << 8)
            + (uint64_t)(pdata[7]);
    }
    return value;
}