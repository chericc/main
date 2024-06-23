#include <algorithm>
#include <iostream>
#include <random>
#include <vector>

#include "bst.h"

void print_helper(const BSTree& tree) {
    // tree.preorder_print ();
    // tree.inorder_print ();
    std::cout << "tree.height=" << tree.height() << std::endl;
    std::cout << "tree.size=" << tree.size() << std::endl;
}

void test_suite() {
    std::random_device oRandomDevice;
    std::default_random_engine oRandomEngine(oRandomDevice());

    std::size_t nsize = 1000 * 100;
    std::vector<int> vecdata(nsize);

    for (std::size_t i = 0; i < nsize; ++i) {
        vecdata[i] = (int)i;
    }

    std::shuffle(vecdata.begin(), vecdata.end(), oRandomEngine);

    BSTree tree;

    for (std::size_t i = 0; i < nsize; ++i) {
        tree.insert(vecdata[i]);
    }

    print_helper(tree);

    for (std::size_t i = 0; i < nsize; ++i) {
        int* p = tree.find(vecdata[i]);
        if (nullptr == p || *p != vecdata[i]) {
            std::cout << "error" << std::endl;
        }
    }
}

int main() {
    std::vector<KEY_TYPE> keys = {4, 2, 1, 3, 6, 5, 7};

    BSTree tree;

    for (std::size_t i = 0; i < keys.size(); ++i) {
        tree.insert(keys[i]);
    }

    print_helper(tree);

    tree.erase(4);

    print_helper(tree);

    test_suite();

    return 0;
}
