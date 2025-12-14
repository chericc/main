
#include <inttypes.h>

#include <stack>

#include "file_parser.hpp"
#include "pcap_ng_parser.hpp"
#include "xlog.h"
#include "xutility.hpp"

int main(int argc, char* argv[]) {
    X_UNUSED_PARAMETER(argc);
    X_UNUSED_PARAMETER(argv);

    const std::string path = "/home/test/tmp/dump.pcap";

    FileParser parser;
    if (parser.parseFile(path) < 0) {
        xlog_err("Parse file failed");
    }

    return 0;
}