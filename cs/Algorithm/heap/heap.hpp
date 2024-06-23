#pragma once

#include <cstddef>

class MaxHeap {
   public:
    using index = std::size_t;

    MaxHeap(int* data, index num);

    void MaxHeapify(index index_root, index index_last);
    void BuildMaxHeap(index index_last);
    void Sort();

    static index leftIndex(index index);
    static index rightIndex(index index);
    static index parentIndex(index index);

    static void registerTest();

   private:
    int* data_ = nullptr;
    index size_ = 0;
};