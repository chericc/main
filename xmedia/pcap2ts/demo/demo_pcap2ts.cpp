#include "pcap_ng_parser.hpp"

#include "xlog.hpp"

int main(int argc, char *argv[])
{
    xlog_dbg("hello world");

    const std::string path = "/home/test/tmp/dump_4k_bianfuxia_25fps_hls.pcap";

    PcapngParser parser(path);

    return 0;
}