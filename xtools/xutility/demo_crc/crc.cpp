#include <cstdio>
#include <cstring>
#include <string>

#include "xlog.hpp"
#include "xcrc.hpp"

static std::string crc_filestream(FILE *fp)
{
    
}

static std::string crc_file(const std::string file)
{
    FILE *fp = nullptr;

    do 
    {
        if (file.empty())
        {
            xlog_err("Filename empty");
            break;
        }

        fp = fopen(file.c_str(), "rb");
        if (!fp)
        {
            char err_buf[64];
            int error_num = errno;
            strerror_r(error_num, err_buf, sizeof(err_buf));
            xlog_err("Open file failed(%s,%s)", file.c_str(), err_buf);
            break;
        }
    }
    while (0);
}

int main()
{
    std::vector<FILE*> output{stdout};
    xlog_setoutput(output);

    xlog_dbg("hello world");



    return 0;
}