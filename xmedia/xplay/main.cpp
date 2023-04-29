
#include <string>

#include "xplay.hpp"
#include "xlog.hpp"

static void print_usage(int argc, char **argv)
{
    printf("%s [filename]\n", argv[0]);
    return ;
}

static int parse_opts(int argc, char **argv, OptValues &opts)
{
    int error = false;

    if (argc != 2)
    {
        print_usage(argc, argv);
        return -1;
    }

    opts.filename = argv[1];

    return error ? -1 : 0;
}

int main(int argc, char **argv)
{
    OptValues opts{};
    XPlay player;
    FILE *fp = nullptr;

    do 
    {
        fp = fopen("xplay.log", "w");
        std::vector<FILE *> fps = {stdout, fp};
        xlog_setoutput(fps);

        if (parse_opts(argc, argv, opts) < 0)
        {
            xlog_err("get opts failed");
            break;
        }

        xlog_dbg("opts: filename:%s", opts.filename.c_str());

        if (player.open(opts) < 0)
        {
            xlog_err("open failed");
            break;
        }
    }
    while (0);

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}