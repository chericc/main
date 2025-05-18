#include "live555wrap.hpp"

LW_Env::LW_Env(char &stop)
    : BasicUsageEnvironment(*BasicTaskScheduler::createNew()), _stop(stop)
{

}

int LW_Env::loop()
{
    taskScheduler().doEventLoop(&_stop);
    return 0;
}