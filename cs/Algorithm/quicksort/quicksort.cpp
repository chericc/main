#include "quicksort.hpp"

#include <algorithm>
#include <random>
#include <vector>

#include "alg.hpp"
#include "xlog.hpp"

void QuickSort::register_test() {
    auto test = []() {
        std::vector<int> v;
        v.resize(10);
        for (std::size_t i = 0; i < v.size(); ++i) {
            v[i] = (std::size_t)i;
        }
        std::shuffle(v.begin(), v.end(), std::default_random_engine());
        std::string str_a = output_elements(v);

        QuickSort qs(v.data(), v.size());
        qs.sort();
        std::string str_b = output_elements(v);
        xlog_dbg("before: %s", str_a.c_str());
        xlog_dbg("after: %s", str_b.c_str());
    };

    MainAlgManager::Funcs funcs;
    funcs["base"] = test;
    MainAlgManager::getInstance().add("quicksort", funcs);
}

QuickSort::QuickSort(int *data, std::size_t num) : data_(data), num_(num) {}

void QuickSort::sort() { sort(0, num_ > 0 ? num_ - 1 : 0); }

/**
 * @return index of mid
 */
QuickSort::index QuickSort::group(index begin, index last, index sep) {
    /**
     * i1 points the next element of left part
     * i2 points the next element of right part
     * -----+++++
     *      i1   i2
     */
    index i1 = begin;
    index i2 = begin;
    int sep_val = data_[sep];
    std::swap(data_[last], data_[sep]);

    for (; i2 < last; ++i2) {
        if (data_[i2] <= sep_val) {
            std::swap(data_[i1], data_[i2]);
            ++i1;
        }
    }

    std::swap(data_[i1], data_[last]);

    return i1;
}

void QuickSort::sort(index begin, index last) {
    index idx_mid = group(begin, last, begin);
    if (begin + 1 < idx_mid) {
        sort(begin, idx_mid - 1);
    }
    if (idx_mid + 1 < last) {
        sort(idx_mid + 1, last);
    }
}