#include <iostream>
#include <vector>

class Tree {
   public:
    void print_1(int a) { std::cout << "print 1" << std::endl; }
    void print_2(int a) { std::cout << "print 2" << std::endl; }
    void print_3(int a) { std::cout << "print 3" << std::endl; }
};

int main() {
    typedef void (Tree::*func)(int);

    std::vector<func> vec_func = {&Tree::print_1, &Tree::print_2,
                                  &Tree::print_3};

    Tree tree;
    for (auto it : vec_func) {
        (tree.*it)(0);
    }
}