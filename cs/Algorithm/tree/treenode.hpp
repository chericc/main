#pragma once

#include <string>

namespace Tree {

using value_t = int;

// binary tree node
class BTreeNode {
   public:
    virtual BTreeNode *prt() = 0;
    virtual BTreeNode *left() = 0;
    virtual BTreeNode *right() = 0;
    virtual value_t &value() = 0;
    virtual std::string text() = 0;
};

}  // namespace Tree
