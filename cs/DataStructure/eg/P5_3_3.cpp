
#include <string.h>
#include <iostream>

/**
 
2017 计算机统考 408
将给定的表达式树转换为等价的中缀表达式（通过括号反映运算的次序）。

例如：
            *
     +             *
   a   b         c   -
                         d
转化为：
(a+b)*(c*(-d))


            +
     *             -
   a   b              -
                    c    d
转化为：
(a*b)+(-(c-d))
 
 */

/* 树节点给定如下： */
class node {
public:
    char data[10]; // 存储操作数或操作符
    struct node *l, *r;

    node(const char *s) {
        memset (data, 0, sizeof(data));
        strncpy(data, s, sizeof(data)-1);
        l= nullptr;
        r = nullptr;
    }
    node *left() {
        return l;
    }
    node *right() {
        return r;
    }
    void leftjoin(node *ln) {
        l = ln;
    }
    void rightjoin(node *rn) {
        r = rn;
    }
};

node *gennode1() {
    node *n1 = new node("+");
    node *n2 = new node("*");
    node *n3 = new node("-");
    node *n4 = new node("a");
    node *n5 = new node("b");
    node *n6 = new node("-");
    node *n7 = new node("c");
    node *n8 = new node("d");

    n2->leftjoin(n4);
    n2->rightjoin(n5);
    n6->leftjoin(n7);
    n6->rightjoin(n8);
    n3->rightjoin(n6);
    n1->leftjoin(n2);
    n1->rightjoin(n3);

    return n1;
}

void tree_exp_print(node *tree) {
    if (tree->left() != nullptr || tree->right() != nullptr) {
        std::cout << "(";
    }
    if (tree->left() != nullptr) {
        tree_exp_print(tree->left());
    }
    std::cout << tree->data;
    if (tree->right() != nullptr) {
        tree_exp_print(tree->right());
    }
    if (tree->left() != nullptr || tree->right() != nullptr) {
        std::cout << ")";
    }
}

int main() {
    node *tree = gennode1();

    if (tree->left() != nullptr) {
        tree_exp_print(tree->left());
    }
    std::cout << tree->data;
    if (tree->right() != nullptr) {
        tree_exp_print(tree->right());
    }

    return 0;
}