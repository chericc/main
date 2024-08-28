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
    
    auto rbnode = static_cast<RBTreeNode*>(node);
    rbnode->set_color(Red);

    insert_fix(node);
    return ;
}

void RBTree::insert_fix(BTreeNode *node)
{

/*

INIT CONDITION:

- z is red node added.
- if z.p is root, then z.p is black.
- rb-tree quality 1(i.e. q1), q3, q5 is ok.

if q2 is not ok, then new node is root.
if q4 is not ok, then new node is child of red node.

only one rule can be broke at init condition.

TERMINATE CONDITION: 

- if z.p is black, then terminate.

KEEP:

- cond1: uncle node is red.
 - make z's parent and uncle black
 - make z's grandparent red
 - only z's grandparent broke q4 or q2;
- cond2: uncle node is black and z is right child.
 - make z to z's parent, left rotate z. into cond3.
- cond3: uncle node is black and z is left child.
 - right rotate z's grandparent.
 - make z's parent black.
 - make z's sibling red.
 - z'parent is black. meet terminate condition.

*/

    

}

}