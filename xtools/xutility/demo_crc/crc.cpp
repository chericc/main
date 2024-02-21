#include "xlog.hpp"

int main()
{
    std::vector<FILE*> output{stdout};
    xlog_setoutput(output);

    xlog_dbg("hello world");

    return 0;
}