/**
 * compile commands:
 * g++ -I.. -std=c++11 heapsort.cpp -g
 */

#include <algorithm>
#include <chrono>
#include <random>
#include <thread>
#include <vector>

#include "utility.h"

using TEST::print_elements;

namespace {

/**
 * @brief Return the index of left node.
 */
size_t Left(size_t i) { return 2 * i; }

/**
 * @brief Return the index of right node.
 */
size_t Right(size_t i) { return 2 * i + 1; }

/**
 * @brief Make this heap MAXHEAP again when the
 * heap's root element is changed.
 * @param indexEnd The index of the last element of heap.
 * @param indexRoot The index of root.
 */
void MaxHeapify(std::vector<int>& vecData, size_t indexEnd, size_t indexRoot) {
    if (indexEnd >= vecData.size()) {
        return;
    }

    if (indexRoot >= indexEnd) {
        return;
    }

    /**
     * Find out the max element among root node, left node and right node.
     **/
    size_t indexLeft = Left(indexRoot);
    size_t indexRight = Right(indexRoot);
    size_t indexLargest = indexRoot;

    if (indexLeft <= indexEnd && vecData[indexLeft] > vecData[indexLargest]) {
        indexLargest = indexLeft;
    }

    if (indexRight <= indexEnd && vecData[indexRight] > vecData[indexLargest]) {
        indexLargest = indexRight;
    }

    /* If the max node is not root, than swap root with it. */
    if (indexLargest != indexRoot) {
        std::swap(vecData[indexRoot], vecData[indexLargest]);
        MaxHeapify(vecData, indexEnd, indexLargest);
    }

    return;
}

void BuildMaxHeap(std::vector<int>& vecData) {
    /**
     * In order to build a MaxHeapTree, we MaxHeapify
     * all sub trees from bottom to top.
     */
    for (size_t i = vecData.size() / 2; i != (size_t)(-1); --i) {
        MaxHeapify(vecData, vecData.size() - 1, i);
    }

    return;
}

void HeapSort(std::vector<int>& vecData) {
    /**
     * Use max-heap to generate ONE element per time.
     */
    BuildMaxHeap(vecData);

    for (size_t i = 0; i < vecData.size() - 1; ++i) {
        std::swap(vecData[0], vecData[vecData.size() - 1 - i]);
        MaxHeapify(vecData, vecData.size() - 1 - (i + 1), 0);
    }

    return;
}

}  // namespace

int main() {
    int nSize = 1000 * 1000;
    std::vector<int> vecData(nSize);
    for (int i = 0; i < nSize; ++i) {
        vecData[i] = i;
    }

    std::shuffle(vecData.begin(), vecData.end(), std::default_random_engine());

    // BuildMaxHeap (vecData);

    {
        TIME_POINT_BEGIN(tTemp);
        HeapSort(vecData);
        TIME_POINT_END(tTemp);
    }

    return 0;
}
