#include "printtree.hpp"

#include <sstream>

namespace {

void r_print_tree(Tree::BTreeNode *root, int space, int height,
                  std::stringstream &ss) {
    if (!root) {
        return;
    }

    space += height;
    r_print_tree(root->right(), space, height, ss);
    ss << "\n";
    for (int i = height; i < space; ++i) {
        ss << " ";
    }

    ss << root->to_text() << "\n";
    // ss << root->to_text();

    r_print_tree(root->left(), space, height, ss);
    return;
}

}  // namespace

namespace Tree {

std::string print_tree(BTreeNode *root) {
    std::stringstream ss;
    r_print_tree(root, 0, 5, ss);
    return ss.str();
}

}  // namespace Tree