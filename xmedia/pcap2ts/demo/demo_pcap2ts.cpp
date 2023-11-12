#include "pcap_ng_parser.hpp"

#include "xlog.hpp"

int main(int argc, char *argv[])
{
    xlog_setmask(XLOG_MASK_ERR);
    xlog_dbg("hello world");

    const std::string path = "/home/test/tmp/dump_4k_bianfuxia_25fps_hls.pcap";

    PcapngParser parser(path);
    if (parser.goThrough() < 0)
    {
        xlog_err("Go through failed");
    }
    xlog_dbg("Go through successful");

    return 0;
}