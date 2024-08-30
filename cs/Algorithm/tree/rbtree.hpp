#pragma once

#include "treenode.hpp"

/*

rb-tree quality:

1. node is black or red.
2. root is black.
3. leaf is black, leaf is null node.
4. children node of red node is black.
5. black height is all equal.

*/

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
        std::string to_text() override { 
            return std::to_string(value_) + "(" + (color_ == Red ? "r" : "b")
                + ")"; 
        }
        void set_color(Color color) { color_ = color; }
        Color color() { return color_; }

       private:
        value_t value_ = -1;
        Color color_ = Red;
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
    std::string dump();

private:

    void left_rotate(BTreeNode *node);
    void right_rotate(BTreeNode *node);

    void insert(BTreeNode *node);
    void insert_fix(BTreeNode *node);

    Color color(BTreeNode *node);
    void set_color(BTreeNode *node, Color color);

    BTreeNode *null_ = nullptr;
    BTreeNode *root_ = nullptr;
};

}  // namespace Tree