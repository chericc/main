

#include "alg.hpp"
#include "heap.hpp"
#include "nth_minium.hpp"
#include "printtreedemo.hpp"
#include "quicksort.hpp"
#include "xlog.hpp"
#include "rbtreedemo.hpp"
#include "lcs.hpp"

int main() {
    xlog_dbg("main start");

    // MaxHeap::registerTest();
    // QuickSort::register_test();
    // NthMinium::register_test();
    // Tree::printtreedemo_registertest();
    // Tree::rbtreedemo_registertest();
    Lcs::registerTest();

    MainAlgManager::getInstance().listDemos();
    MainAlgManager::getInstance().runAllDemo();

    return 0;
}