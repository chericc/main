#include "rbtreedemo.hpp"

#include "alg.hpp"
#include "xlog.h"
#include "rbtree.hpp"

namespace Tree {

void rbtreedemo_registertest()
{
    auto test_insert = [](){
        RBTree tree;
        for (int i = 10; i > 0; --i) {
            tree.insert(i);
        }
        xlog_dbg("dump: \n\n{}\n\n", tree.dump().c_str());
    };

    auto test_remove = [](){
        RBTree tree;
        for (int i = 1; i < 11; ++i) {
            tree.insert(i);
        }
        xlog_dbg("dump: \n\n{}\n\n", tree.dump().c_str());
        // for (int i = 1; i < 11; ++i) {
        //     xlog_dbg("remove value {}", i);
        //     tree.remove(i);
        //     xlog_dbg("dump: \n\n{}\n\n", tree.dump().c_str());
        //     // break;
        // }
        for (int i = 10; i >= 1; --i) {
            xlog_dbg("remove value {}", i);
            tree.remove(i);
            xlog_dbg("dump: \n\n{}\n\n", tree.dump().c_str());
            // break;
        }
    };

    MainAlgManager::Funcs funcs;
    funcs["insert"] = test_insert;
    funcs["remove"] = test_remove;
    MainAlgManager::getInstance().add("rbtree", funcs);
}



}