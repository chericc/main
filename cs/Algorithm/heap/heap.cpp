#include "heap.hpp"

#include <algorithm>
#include <random>
#include <vector>

#include "alg.hpp"
#include "xlog.h"

void MaxHeap::registerTest() {
    auto test = []() -> void {
        std::vector<int> v;
        v.resize(10);
        for (std::size_t i = 0; i < v.size(); ++i) {
            v[i] = (std::size_t)i;
        }
        std::shuffle(v.begin(), v.end(), std::default_random_engine());
        std::string str_a = output_elements(v);
        MaxHeap maxheap(v.data(), v.size());
        maxheap.Sort();
        std::string str_b = output_elements(v);

        xlog_dbg("maxheap sort");
        xlog_dbg("before: %s", str_a.c_str());
        xlog_dbg("after: %s", str_b.c_str());
    };

    MainAlgManager::Funcs funcs;
    funcs["base"] = test;
    MainAlgManager::getInstance().add("heap", funcs);
}

MaxHeap::MaxHeap(int* data, index size) : data_(data), size_(size) {}

/**
 * @brief Make (sub) heap Maxheap again when it's root element
 * changed.
 * @param index_root The index of the root element of heap.
 */
void MaxHeap::MaxHeapify(index index_root, index index_last) {
    if (index_root >= size_) {
        xlog_err("invalid index_root: %zu", index_root);
        return;
    }

    index left_idx = leftIndex(index_root);
    index right_idx = rightIndex(index_root);
    index max_idx = index_root;
    if (left_idx <= index_last) {
        if (data_[left_idx] > data_[max_idx]) {
            max_idx = left_idx;
        }

        if (right_idx <= index_last) {
            if (data_[right_idx] > data_[max_idx]) {
                max_idx = right_idx;
            }
        }
    }
    if (max_idx != index_root) {
        std::swap(data_[max_idx], data_[index_root]);
        MaxHeapify(max_idx, index_last);
    }
}

void MaxHeap::BuildMaxHeap(index index_last) {
    if (size_ > 0) {
        index max_sub_tree_index = parentIndex(index_last);
        for (index idx = max_sub_tree_index; idx > 0; --idx) {
            MaxHeapify(idx, index_last);
        }
        MaxHeapify(0, index_last);
    }
}

void MaxHeap::Sort() {
    if (size_ > 1) {
        BuildMaxHeap(size_ - 1);
        for (index i = size_ - 1; i >= 1; --i) {
            MaxHeapify(0, i);
            std::swap(data_[0], data_[i]);
        }
    }
}

/**
        0
      1      2
    3   4  5   6

*/

MaxHeap::index MaxHeap::leftIndex(index idx) { return 2 * idx + 1; }

MaxHeap::index MaxHeap::rightIndex(index idx) { return 2 * idx + 2; }

MaxHeap::index MaxHeap::parentIndex(index idx) { return (idx - 1) / 2; }