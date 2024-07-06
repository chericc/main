#pragma once

#include <string>

namespace Tree {

using value_t = int;

// binary tree node
class BTreeNode {
   public:
    virtual ~BTreeNode() = default;
    virtual BTreeNode *parent() = 0;
    virtual void set_parent(BTreeNode *node) = 0;
    virtual BTreeNode *left() = 0;
    virtual void set_left(BTreeNode *node) = 0;
    virtual BTreeNode *right() = 0;
    virtual void set_right(BTreeNode *node) = 0;
    virtual value_t const &value() = 0;
    virtual void set_value(value_t const &value) = 0;
    virtual std::string to_text() = 0;
};

}  // namespace Tree
