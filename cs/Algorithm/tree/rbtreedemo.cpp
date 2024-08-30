#include "rbtreedemo.hpp"

#include "alg.hpp"
#include "printtree.hpp"
#include "xlog.hpp"
#include "rbtree.hpp"

namespace Tree {

void rbtreedemo_registertest()
{
    auto test_insert = [](){
        RBTree tree;
        for (int i = 10; i > 0; --i) {
            tree.insert(i);
        }
        auto dump = tree.dump();
        xlog_dbg("dump: \n\n%s\n\n", dump.c_str());
    };

    MainAlgManager::Funcs funcs;
    funcs["insert"] = test_insert;
    MainAlgManager::getInstance().add("rbtree", funcs);
}



}