#pragma once

#include <cstddef>

class MaxHeap
{
public:
    using index = std::size_t;

    MaxHeap(int *data, index size);

    void MaxHeapify(index index_root);
    void BuildMaxHeap();

    static index leftIndex(index index);
    static index rightIndex(index index);
    static index parentIndex(index index);

    static void register_test();
private:
    int *data_ = nullptr;
    index size_ = 0;
};