
#include <stack>
#include <inttypes.h>

#include "pcap_ng_parser.hpp"

#include "xlog.hpp"
#include "xutility.hpp"

#include "file_parser.hpp"

int main(int argc, char *argv[])
{
    X_UNUSED_PARAMETER(argc);
    X_UNUSED_PARAMETER(argv);

    xlog_setmask(XLOG_MASK_LOG);

    const std::string path = "/home/test/tmp/dump.pcap";

    FileParser parser;
    if (parser.parseFile(path) < 0)
    {
        xlog_err("Parse file failed");
    }

    return 0;
}