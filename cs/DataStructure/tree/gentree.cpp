
#include "gentree.h"

#include <random>
#include <chrono>
#include <algorithm>

/******

       1
    2     3
   4 5

w  3 2    7
WPL = 3*3 + 2*3 + 7*2 = 29

******/

Node *gentree() {
    Node *root = new Node(1);

    Node *a2 = new Node(2);
    Node *a4 = new Node(4, 3);
    Node *a5 = new Node(5, 2);
    Node *a3 = new Node(3, 7);

    a2->leftjoin(a4);
    a2->rightjoin(a5);
    root->leftjoin(a2);
    root->rightjoin(a3);

    return root;
}

Node *gentree_random (const std::vector<int> &refValues)
{
    return nullptr;
}