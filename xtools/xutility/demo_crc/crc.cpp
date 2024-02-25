#include <cstdio>
#include <cstring>
#include <string>
#include <cstdint>
#include <unistd.h>

#include "xlog.hpp"
#include "xcrc.hpp"

static std::string crc_file(const std::string file, XCRCID id)
{
    FILE *fp = nullptr;
    XCrc crc(id);
    uint32_t crc_value = 0;
    std::string crc_str;

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

        for(;;)
        {
            uint8_t buf[512];
            size_t ret_fread = fread(buf, 1, sizeof(buf), fp);
            if (ret_fread > 0)
            {
                crc_value = crc.crc(crc_value, buf, ret_fread);
            }
            else 
            {
                if (ferror(fp))
                {
                    xlog_err("Read failed");
                    break;
                }
                else if (feof(fp))
                {
                    xlog_dbg("Read end");
                    break;
                }
                xlog_err("Unknown error");
                break;
            }
        }

        char str_crc[64];
        snprintf(str_crc, sizeof(str_crc), "%x", crc_value);
        crc_str.assign(str_crc);
    }
    while (0);

    if (fp)
    {
        fclose(fp);
        fp = nullptr;
    }

    return crc_str;
}

struct opt_info
{
    std::string input_filename;
    XCRCID id = XCRCID::CRC_32_IEEE;
};

int parse_args(int argc, char *argv[], opt_info &option)
{
    int opt = 0;
    while ((opt = getopt(argc, argv, "i:c:")) != -1)
    {
        switch (opt)
        {
            case 'i':
            {
                option.input_filename.assign(optarg);
                break;
            }
            case 'c':
            {
                option.id = (XCRCID)atoi(optarg);
                break;
            }
            default:
            {
                fprintf(stderr, "Usage: %s [-i file] [-c crc_algo]\n", 
                    argv[0]);
                break;
            }
        }
    }
    return 0;
}

int main(int argc, char *argv[])
{
    std::string crc_str;
    do
    {
        std::vector<FILE*> output{stdout};
        xlog_setoutput(output);

        opt_info option;
        parse_args(argc, argv, option);

        if (option.input_filename.empty())
        {
            xlog_err("No input file");
            break;
        }

        crc_str = crc_file(option.input_filename, option.id);

        fprintf(stdout, "crc: %s\n", crc_str.c_str());
    }
    while (0);

    return 0;
}