#include "huffman_tree.h"

#include <algorithm>

Node* gen_hufffmantree(std::vector<int>& refValues) {
    /* 生成以给定值列为值的森林 */
    std::vector<Node*> vecTrees;
    for (std::size_t i = 0; i < refValues.size(); ++i) {
        Node* pNewNode = new Node(refValues[i]);
        vecTrees.push_back(pNewNode);
    }

    while (vecTrees.size() > 1) {
        /* 排序法找出权最小的两棵树 */
        std::sort(vecTrees.begin(), vecTrees.end(),
                  [](Node* node1, Node* node2) -> bool {
                      return node1->data > node2->data;
                  });

        Node* pNodeLeft = vecTrees[vecTrees.size() - 2];
        Node* pNodeRight = vecTrees[vecTrees.size() - 1];
        Node* pNodeTop = new Node(pNodeLeft->data + pNodeRight->data);

        /* 将权最小的两棵树合并为一棵新树 */
        pNodeTop->leftjoin(pNodeLeft);
        pNodeTop->rightjoin(pNodeRight);

        vecTrees.pop_back();
        vecTrees.pop_back();
        vecTrees.push_back(pNodeTop);
    }

    return vecTrees.empty() ? nullptr : vecTrees[0];
}