

#include "alg.hpp"
#include "heap.hpp"
#include "quicksort.hpp"
#include "xlog.hpp"

int main() {
    xlog_dbg("main start");

    MaxHeap::registerTest();
    QuickSort::register_test();

    MainAlgManager::getInstance().listDemos();

    MainAlgManager::getInstance().runAllDemo();

    return 0;
}