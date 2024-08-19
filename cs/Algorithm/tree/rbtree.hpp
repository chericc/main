#pragma once

#include "treenode.hpp"

namespace Tree {

class RBTree {
    enum Color{
        Black,
        Red,
    };

    class RBTreeNode : public BTreeNode {
       public:
        ~RBTreeNode() override = default;
        BTreeNode *parent() override { return parent_; }
        void set_parent(BTreeNode *node) override { parent_ = node; }
        BTreeNode *left() override { return left_; }
        void set_left(BTreeNode *node) override { left_ = node; }
        BTreeNode *right() override { return right_; }
        void set_right(BTreeNode *node) override { right_ = node; }
        value_t const &value() override { return value_; }
        void set_value(value_t const &value) override { value_ = value; }
        std::string to_text() override { return std::to_string(value_); };

       private:
        value_t value_ = -1;
        BTreeNode *left_ = nullptr;
        BTreeNode *right_ = nullptr;
        BTreeNode *parent_ = nullptr;
    };

   public:
    RBTree();
    ~RBTree();
    void destroy(BTreeNode *subtree);
    int insert(value_t const &value);
    int exist(value_t const &value);
    int remove(value_t const &value);

private:

    void left_rotate(BTreeNode *node);
    void right_rotate(BTreeNode *node);

    void insert(BTreeNode *node);
    void insert_fix(BTreeNode *node);

    BTreeNode *null_ = nullptr;
    BTreeNode *root_ = nullptr;
};

}  // namespace Tree