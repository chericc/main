#include "optimal_binary_search_tree.hpp"

#include <vector>
#include <cstdint>

#include "alg.hpp"
#include "xlog.hpp"

namespace {

// 实现待补充

}

void OptimalBinarySearchTree::registerTest()
{
    auto test = []() {
        // 测试代码待补充
    };

    MainAlgManager::Funcs funcs;
    funcs["base"] = test;
    MainAlgManager::getInstance().add("optimal_binary_search_tree", funcs);

    return ;
}

static StaticRegistrant _reg_optimal_binary_search_tree(OptimalBinarySearchTree::registerTest);
