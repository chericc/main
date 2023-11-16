
#include <stack>
#include <inttypes.h>

#include "pcap_ng_parser.hpp"

#include "xlog.hpp"
#include "xutility.hpp"

static std::pair<std::string,std::string> str_ipv4addr(const std::array<uint8_t, 8> &ipv4)
{
    std::string ipstr;
    std::string maskstr;
    char str_tmp[256];

    snprintf(str_tmp, sizeof(str_tmp), "%" PRIu8 ".%" PRIu8 ".%" PRIu8 ".%" PRIu8, 
        ipv4[0], ipv4[1], ipv4[2], ipv4[3]);
    ipstr.assign(str_tmp);

    snprintf(str_tmp, sizeof(str_tmp), "%" PRIu8 ".%" PRIu8 ".%" PRIu8 ".%" PRIu8, 
        ipv4[4], ipv4[5], ipv4[6], ipv4[7]);
    maskstr.assign(str_tmp);

    return std::make_pair(ipstr, maskstr);
}

static std::string str_ipv6addr(const std::array<uint8_t, 17> &ipv6)
{
    char str_tmp[256];
    snprintf(str_tmp, sizeof(str_tmp), 
        "%02" PRIx8 "%02" PRIx8 ".%02" PRIx8 "%02" PRIx8 ".%02" PRIx8 "%02" PRIx8 ".%02" PRIx8 "%02" PRIx8 
        ".%02" PRIx8 "%02" PRIx8 ".%02" PRIx8 "%02" PRIx8 ".%02" PRIx8 "%02" PRIx8 ".%02" PRIx8 "%02" PRIx8 
        "/%" PRIu8,
        ipv6[0], ipv6[1], ipv6[2], ipv6[3], ipv6[4], ipv6[5], ipv6[6], ipv6[7], 
        ipv6[8], ipv6[9], ipv6[10], ipv6[11], ipv6[12], ipv6[13], ipv6[14], ipv6[15], 
        ipv6[16]);
    return std::string(str_tmp);
}

static std::string str_macaddr(const std::array<uint8_t, 6> &mac)
{
    char str_tmp[256];
    snprintf(str_tmp, sizeof(str_tmp),
        "%02" PRIx8 ".%02" PRIx8 ".%02" PRIx8 ".%02" PRIx8 ".%02" PRIx8 ".%02" PRIx8,
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return std::string(str_tmp);
}

static std::string str_euiaddr(const std::array<uint8_t, 8> &euiaddr)
{
    char str_tmp[256];
    snprintf(str_tmp, sizeof(str_tmp),
        "%02" PRIx8 ".%02" PRIx8 ".%02" PRIx8 ".%02" PRIx8 ".%02" PRIx8 ".%02" PRIx8 ".%02" PRIx8 ".%02" PRIx8,
        euiaddr[0], euiaddr[1], euiaddr[2], euiaddr[3], 
        euiaddr[4], euiaddr[5], euiaddr[6], euiaddr[7]);
    return std::string(str_tmp);
}

static std::string str_bps(double bps)
{
    std::array<std::string, 4> speed_name = {"bps", "kbps", "Mbps", "Gbps"};

    double bitrate = bps;
    std::size_t i_level = 0;
    char str[64];
    for (i_level = 0; i_level < speed_name.size(); ++i_level)
    {
        if (bitrate < 1000.0)
        {
            break;
        }
        bitrate = bitrate / 1000;
    }
    
    snprintf(str, sizeof(str), "%.1g %s", bitrate, speed_name[i_level].c_str());
    return std::string(str);
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
            xlog_inf("ipv4: ");
            for (auto const& it : intf.if_IPv4addrs)
            {
                auto pair_ipv4 = str_ipv4addr(it);
                xlog_inf("ipv4/mask: %s/%s", pair_ipv4.first.c_str(), pair_ipv4.second.c_str());
            }
            xlog_inf("ipv6: ");
            for (auto const& it : intf.if_IPV6addrs)
            {
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

            content_stack.push(content);
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