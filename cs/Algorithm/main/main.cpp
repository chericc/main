#include "xlog.hpp"

#include "alg.hpp"
#include "heap.hpp"

int main()
{
    xlog_dbg("main start");

    MaxHeap::registerTest();

    MainAlgManager::getInstance().listDemos();

    MainAlgManager::getInstance().runAllDemo();

    return 0;
}