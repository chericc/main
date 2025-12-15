#include "file_parser.hpp"

#include <inttypes.h>

#include "comm.hpp"
#include "xlog.h"

FileParser::FileParser() {}

FileParser::~FileParser() {}

int FileParser::parseFile(const std::string& file) {
    bool error = false;

    do {
        bool parse_success_flag = false;

        /* Release all contexts. */
        _pcapng_ctx.reset();

        /* Try pcapng */
        do {
            std::shared_ptr<PcapngParser> _pcapng_parser =
                std::make_shared<PcapngParser>();
            _pcapng_ctx = std::make_shared<PcapngContext>();
            _pcapng_ctx->pp = std::make_shared<PacketProcess>();
            auto lambda_func = [this](const PcapngContent& content) -> int {
                return this->dealPcapngContent(content);
            };
            int ret_parse = _pcapng_parser->parse(file, lambda_func);
            if (ret_parse) {
                xlog_dbg("Try pcapng failed");
                break;
            }

            xlog_inf("ctx: ");
            xlog_inf("packet_count: %" PRIu64, _pcapng_ctx->packet_count);

            parse_success_flag = true;
        } while (0);

        /* Try pcap */
        { xlog_dbg("Try pcap: not support yet"); }

        if (!parse_success_flag) {
            xlog_err("Parse file failed({})", file.c_str());
            error = true;
        }
    } while (0);

    if (error) {
        return -1;
    }
    return 0;
}

int FileParser::dealPcapngContent(const PcapngContent& content) {
    bool error = false;
    do {
        if (!_pcapng_ctx) {
            xlog_err("Null ctx");
            error = true;
            break;
        }

        if (PcapngContentType::Info == content.type) {
            auto const& info = content.info;

            xlog_inf("hardware: %s", info.shb_hardware.c_str());
            xlog_inf("os: %s", info.shb_os.c_str());
            xlog_inf("userappl: %s", info.shb_userappl.c_str());

            _pcapng_ctx->content_stack.push(content);
        } else if (PcapngContentType::Interface == content.type) {
            auto const& intf = content.interface;
            xlog_inf("if_name: %s", intf.if_name.c_str());
            xlog_inf("if_description: %s", intf.if_description.c_str());
            xlog_inf("ipv4: ");
            for (auto const& it : intf.if_IPv4addrs) {
                auto pair_ipv4 = str_ipv4addr(it);
                xlog_inf("ipv4/mask: %s/%s", pair_ipv4.first.c_str(),
                         pair_ipv4.second.c_str());
            }
            xlog_inf("ipv6: ");
            for (auto const& it : intf.if_IPV6addrs) {
                auto ipv6str = str_ipv6addr(it);
                xlog_inf("ipv6/prefix: %s", ipv6str.c_str());
            }
            xlog_inf("mac: %s", str_macaddr(intf.if_MACaddr).c_str());
            xlog_inf("eui: %s", str_euiaddr(intf.if_EUIaddr).c_str());
            xlog_inf("speed: %s", str_bps(intf.if_speed).c_str());
            xlog_inf("os: %s", intf.if_os.c_str());
            xlog_inf("hardware: %s", intf.if_hardware.c_str());
            xlog_inf("tx: %s", str_bps(intf.if_txspeed).c_str());
            xlog_inf("rx: %s", str_bps(intf.if_rxspeed).c_str());

            _pcapng_ctx->content_stack.push(content);
        } else if (PcapngContentType::Data == content.type) {
            auto const& data = content.data;

            if (data.packet_data) {
                _pcapng_ctx->packet_count += 1;
                xlog_dbg("Processing packet(%" PRIu64 ")",
                         _pcapng_ctx->packet_count);
                if (_pcapng_ctx->pp->processEthernetData(data.packet_data) <
                    0) {
                    xlog_err("Process ethernet data failed");
                    error = true;
                    break;
                }
            } else {
                xlog_inf("data: null");
            }
        }
    } while (0);

    if (error) {
        return -1;
    }
    return 0;
}