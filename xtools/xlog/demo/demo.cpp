#include "xlog.hpp"

#include <thread>

int main()
{
    std::vector<FILE *> fps = { stdout };
    xlog_setoutput(fps);

    xlog_setmask(XLOG_LEVEL_ERROR | XLOG_LEVEL_CRITICAL);

    for (int i = 0; i < 10; ++i)
    {
        xlog_trc("this is a trace log");
        xlog_dbg("this is a debug log");
        xlog_log("this is a log log");
        xlog_inf("this is a information log");
        xlog_err("this is a error log");
        xlog_cri("this is a critical log");


        xlog_trc("this is a trace log: %d", 0);
        xlog_dbg("this is a debug log: %d", 0);
        xlog_log("this is a log log: %d", 0);
        xlog_inf("this is a information log: %d", 0);
        xlog_err("this is a error log: %d", 0);
        xlog_cri("this is a critical log: %d", 0);

        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 10));
    }
    
    XLOG(ERR) << std::string("Cpp style log");

    return 0;
}