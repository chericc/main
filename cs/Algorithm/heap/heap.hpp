#pragma once

#include <cstddef>

class MaxHeap
{
public:
    using index = std::size_t;

    MaxHeap(int *data, index size);

    /**
    @brief Make (sub) heap Maxheap again when it's root element
    changed.
    @param index_root The index of the root element of heap.
    @param index_last The index of the last element of heap.
    */
    void MaxHeapify(index index_root, index index_last);
    void BuildMaxHeap();

    static index leftIndex(index index);
    static index rightIndex(index index);
    static index parentIndex(index index);
private:
    int *data = nullptr;
    index size = 0;
};