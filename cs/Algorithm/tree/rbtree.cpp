#include "rbtree.hpp"

#include "xlog.hpp"

namespace Tree {

RBTree::RBTree() {
    null_ = new RBTreeNode();
    null_->set_left(null_);
    null_->set_right(null_);
    null_->set_parent(null_);
    root_ = null_;
}

RBTree::~RBTree() { destroy(root_); }

void RBTree::destroy(BTreeNode *node) {
    if (node != null_) {
        destroy(node->left());
        destroy(node->right());
        delete node;
    }
    delete null_;
    null_ = nullptr;
}

void RBTree::left_rotate(BTreeNode *node_x)
{
/*
        x
    a          y
            b     c
*/
    do {
        BTreeNode *node_y = node_x->right();

        if (node_y == null_) {
            break;
        }

        BTreeNode *parent = node_x->parent();
        BTreeNode *node_b = node_y->left();
        
        // parent
        if (parent->left() == node_x) {
            parent->set_left(node_y);
        } else if (parent->right() == node_x) {
            parent->set_right(node_y);
        } else {
            xlog_err("inner error rotating\n");
            break;
        }

        // x
        node_x->set_parent(node_y);
        node_x->set_right(node_b);

        // y
        node_y->set_parent(parent);
        node_y->set_left(node_x);

        // b
        node_b->set_parent(node_x);
    } while (0);

    return ;
}

void RBTree::right_rotate(BTreeNode *node_y)
{
/*
            y
        x          c
      a   b
*/
    do {
        BTreeNode *node_x = node_y->left();
        if (node_x == null_) {
            break;
        }

        BTreeNode *parent = node_x->parent();
        BTreeNode *node_b = node_x->right();

        // parent
        if (parent->left() == node_y) {
            parent->set_left(node_x);
        } else if (parent->right() == node_y) {
            parent->set_right(node_x);
        } else {
            xlog_err("inner error ratating\n");
            break;
        }

        // x
        node_x->set_parent(parent);
        node_x->set_right(node_y);

        // y
        node_y->set_parent(node_x);
        node_y->set_left(node_b);

        // b
        node_b->set_parent(node_y);
    } while (0);

    return ;
}

void RBTree::insert(BTreeNode *node)
{
    BTreeNode *it_p = null_;
    BTreeNode *it = root_;

    while (it != null_) {
        it_p = it;
        if (it->value() < node->value()) {
            it = it->left();
        } else {
            it = it->right();
        }
    }

    // parent
    if (it_p == null_) { // no parent
        root_ = node;
    } else {
        if (node->value() < it_p->value()) {
            it_p->set_left(node);
        } else {
            it_p->set_right(node);
        }
    }

    // node
    node->set_parent(it_p);
    node->set_left(null_);
    node->set_right(null_);

    insert_fix(node);
    return ;
}

void RBTree::insert_fix(BTreeNode *node)
{
    
}

}