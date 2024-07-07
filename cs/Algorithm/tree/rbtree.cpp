#include "rbtree.hpp"

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
    if (node != root_) {
        destroy(node->left());
        destroy(node->right());
        delete node;
    }
}
// namespace Tree
}  // namespace Tree