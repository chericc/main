
#include <iostream>

#include "gentree.h"
#include "huffman_tree.h"
#include "traversetree.h"
#include "wpl.h"

void traverse() {
    Node* tree = gentree();

    std::cout << "recursive_traverse_preorder:" << std::endl;
    recursive_traverse_preorder(tree);
    std::cout << std::endl;

    std::cout << "recursive_traverse_inorder:" << std::endl;
    recursive_traverse_inorder(tree);
    std::cout << std::endl;

    std::cout << "recursive_traverse_postorder:" << std::endl;
    recursive_traverse_postorder(tree);
    std::cout << std::endl;

    std::cout << "nonrecur_traverse_preorder:" << std::endl;
    nonrecur_traverse_preorder(tree);
    std::cout << std::endl;

    std::cout << "nonrecur_traverse_inorder:" << std::endl;
    nonrecur_traverse_inorder(tree);
    std::cout << std::endl;

    std::cout << "nonrecur_traverse_postorder:" << std::endl;
    nonrecur_traverse_postorder(tree);
    std::cout << std::endl;

    std::cout << "level_traverse:" << std::endl;
    level_traverse(tree);
    std::cout << std::endl;
}

void wpl_eg() {
    Node* tree = gentree();
    int nWPL = calc_wpl(tree);
    std::cout << "WPL=" << nWPL << std::endl;
}

void huffmantree_test() {
    std::vector<int> vecValues = {1, 2, 3, 4, 5};
    Node* huffmantree = gen_hufffmantree(vecValues);

    std::cout << "preorder: " << std::endl;
    recursive_traverse_preorder(huffmantree);
    std::cout << std::endl;

    std::cout << "inorder: " << std::endl;
    recursive_traverse_inorder(huffmantree);
    std::cout << std::endl;

    delete huffmantree;
}

int main() {
    // traverse();
    // wpl_eg();
    huffmantree_test();

    return 0;
}