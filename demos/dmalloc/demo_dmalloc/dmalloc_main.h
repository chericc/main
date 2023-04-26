#pragma once

#include <thread>

/* export DMALLOC_OPTIONS=debug=0x4000503,log=/tmp/logfile */

class DmallocImp
{
public:
    DmallocImp();
    ~DmallocImp();
private:
    std::thread thread__;
};