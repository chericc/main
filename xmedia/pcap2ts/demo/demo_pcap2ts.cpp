
#include <stack>
#include <inttypes.h>

#include "pcap_ng_parser.hpp"

#include "xlog.hpp"
#include "xutility.hpp"

static std::pair<std::string,std::string> ipv4addr(const std::array<uint8_t, 8> &ipv4)
{
    std::string ipstr;
    std::string maskstr;
    char str_tmp[64];

    snprintf(str_tmp, sizeof(str_tmp), "%" PRIu8 ".%" PRIu8 ".%" PRIu8 ".%" PRIu8, 
        ipv4[0], ipv4[1], ipv4[2], ipv4[3]);
    ipstr.assign(str_tmp);

    snprintf(str_tmp, sizeof(str_tmp), "%" PRIu8 ".%" PRIu8 ".%" PRIu8 ".%" PRIu8, 
        ipv4[4], ipv4[5], ipv4[6], ipv4[7]);
    maskstr.assign(str_tmp);

    return std::make_pair(ipstr, maskstr);
}

std::string ipv6addr(const std::array<uint8_t, 17> &ipv6)
{
    char str_tmp[256];
    snprintf(str_tmp, sizeof(str_tmp), 
        "%" PRIu8 "%" PRIu8 ".%" PRIu8 "%" PRIu8 ".%" PRIu8 "%" PRIu8 ".%" PRIu8 "%" PRIu8 
        ".%" PRIu8 "%" PRIu8 ".%" PRIu8 "%" PRIu8 ".%" PRIu8 "%" PRIu8 ".%" PRIu8 "%" PRIu8 
        "/%" PRIu8,
        ipv6[0], ipv6[1], ipv6[2], ipv6[3], ipv6[4], ipv6[5], ipv6[6], ipv6[7], 
        ipv6[8], ipv6[9], ipv6[10], ipv6[11], ipv6[12], ipv6[13], ipv6[14], ipv6[15], 
        ipv6[16]);
    return std::string(str_tmp);
}

static void deal_pcapngcontent(const PcapngContent &content)
{
    bool error = false;

    do 
    {
        std::stack<PcapngContent> content_stack;

        if (PcapngContentType::Info == content.type)
        {
            auto const& info = content.info;
            xlog_inf("hardware: %s", info.shb_hardware.c_str());
            xlog_inf("os: %s", info.shb_os.c_str());
            xlog_inf("userappl: %s", info.shb_userappl.c_str());

            content_stack.push(content);
        }
        else if (PcapngContentType::Interface == content.type)
        {
            auto const& intf = content.interface;
            xlog_inf("if_name: %s", intf.if_name.c_str());
            xlog_inf("if_description: %s", intf.if_description.c_str());
            for (auto const& it : intf.if_IPv4addrs)
            {
                auto pair_ipv4 = ipv4addr(it);
                xlog_inf("ipv4/mask: %s/%s", pair_ipv4.first.c_str(), pair_ipv4.second.c_str());
            }
            for (auto const& it : intf.if_IPV6addrs)
            {
                auto ipv6str = ipv6addr(it);
                xlog_inf("ipv6/prefix: %s", ipv6str.c_str());
            }
        }
    }
    while (0);

}

int main(int argc, char *argv[])
{
    X_UNUSED_PARAMETER(argc);
    X_UNUSED_PARAMETER(argv);

    xlog_setmask(XLOG_MASK_LOG);

    const std::string path = "/home/test/tmp/dump.pcap";

    PcapngParser parser(path);

    parser.parse(deal_pcapngcontent);

    // if (parser.goThrough() < 0)
    // {
    //     xlog_err("Go through failed");
    // }
    // xlog_dbg("Go through successful");

    return 0;
}