#include "xlog.h"

#include "exiftool.hpp"

namespace {

void printUsage(int argc, char *argv[])
{
    xlog_dbg("usage: {} [file1] [file2] ...", argv[0]);
}

}

int main(int argc, char *argv[])
{
    xlog_dbg("hello");

    if (argc < 2) {
        printUsage(argc, argv);
        return 1;
    }

    ExifTool tool;
    tool.parse(argv[1], 1000);

    return 0;
}