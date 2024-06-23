#pragma once

#include <cstddef>

class QuickSort {
   public:
    using index = std::size_t;
    QuickSort(int *data, index num);
    static void register_test();
    void sort();

   private:
    void sort(index begin, index end);
    index group(index begin, index end, index sep);
    int *data_ = nullptr;
    index num_ = 0;
};
