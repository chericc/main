#include "nth_largest.hpp"

#include <algorithm>
#include <random>
#include <vector>

#include "alg.hpp"
#include "xlog.hpp"

void NthLargest::register_test() {
    auto test = []() {
        std::vector<int> v;
        v.resize(10);
        for (int i = 0; i < v.size(); ++i) {
            v[i] = (std::size_t)i;
        }
        std::shuffle(v.begin(), v.end(), std::default_random_engine());
        std::string str_a = output_elements(v);

        NthLargest::index nth_index = 5;
        NthLargest nth(v.data(), v.size(), nth_index);
        int nth_val = nth.nth();

        xlog_dbg("array: %s", str_a.c_str());
        xlog_dbg("%zuth largest number is: %d", nth_index, nth_val);
    };

    MainAlgManager::Funcs funcs;
    funcs["base"] = test;
    MainAlgManager::getInstance().add("nth largest", funcs);
}

NthLargest::NthLargest(int *data, index num, index nth)
    : data_(data), size_(num), nth_(nth) {}

NthLargest::index NthLargest::group(index begin, index last, index sep) {
    index i1 = begin;
    index i2 = begin;

    int sep_val = data_[sep];
    std::swap(data_[sep], data_[last]);

    for (; i2 < last; ++i2) {
        if (data_[i2] <= sep_val) {
            std::swap(data_[i1], data_[i2]);
            ++i1;
        }
    }

    std::swap(data_[i1], data_[last]);
    return i1;
}

int NthLargest::nth(index begin, index last, index n) {
    if (begin == last) {
        return data_[begin];
    }

    index mid = group(begin, last, 0);
    index left_num = mid - begin + 1;
    index right_num = last - mid + 1;
    if (n == left_num) {
        return data_[n];
    } else if (n < left_num) {
        return nth(begin, mid - 1, n);
    } else {
        return nth(mid + 1, last, n - left_num);
    }
}

int NthLargest::nth() {
    if (size_ > 0) {
        return nth(0, size_ - 1, nth_);
    }
    return -1;
}
