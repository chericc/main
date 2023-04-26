
#include "wpl.h"

#include <iostream>

/**
 * 递归函数
 * 计算@node对应树的WPL值，@depth为@node所在
 * 的深度（根节点为1）
 * 返回WPL值
 */
int wpl_recursive(Node *node, int depth) {
    int nWPL = 0;
    if (node->left() != nullptr) {
        nWPL += wpl_recursive(node->left(), depth + 1);
    }
    if (node->right() != nullptr) {
        nWPL += wpl_recursive(node->right(), depth + 1);
    }
    if (node->left() == nullptr && 
        node->right() == nullptr) {
        std::cout << node->w << "*" << depth << std::endl;
        nWPL += node->w * depth;
    }
    return nWPL;
}

int calc_wpl(Node *tree) {
    return wpl_recursive(tree, 1);
}