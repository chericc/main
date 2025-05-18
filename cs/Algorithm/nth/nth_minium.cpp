#include "nth_minium.hpp"

#include <algorithm>
#include <random>
#include <vector>

#include "alg.hpp"
#include "xlog.h"

void NthMinium::register_test() {
    auto test = []() {
        std::vector<int> v;
        v.resize(10);
        for (int i = 0; i < (int)v.size(); ++i) {
            v[i] = i;
        }
        std::shuffle(v.begin(), v.end(), std::default_random_engine());
        std::string str_a = output_elements(v);

        NthMinium::index nth_index = 4;
        NthMinium nth(v.data(), v.size(), nth_index);
        int nth_val = nth.nth();

        xlog_dbg("array: %s", str_a.c_str());
        xlog_dbg("%zuth largest number is: %d", nth_index, nth_val);
    };

    MainAlgManager::Funcs funcs;
    funcs["base"] = test;
    MainAlgManager::getInstance().add("nth largest", funcs);
}

NthMinium::NthMinium(int *data, index num, index nth)
    : data_(data), size_(num), nth_(nth) {}

NthMinium::index NthMinium::group(index begin, index last, index sep) {
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

int NthMinium::nth(index begin, index last, index n) {
    if (begin == last) {
        return data_[begin];
    }

    xlog_dbg("n: %lu", n);
    xlog_dbg("1arr: %s", output_elements(data_, begin, last + 1).c_str());

    index mid = group(begin, last, begin);
    index left_num = mid - begin;

    xlog_dbg("2arr: %s", output_elements(data_, begin, last + 1).c_str());
    xlog_dbg("begin: %lu, last: %lu, mid: %lu", begin, last, mid);

    if (n == left_num) {
        return data_[mid];
    } else if (n < left_num) {
        return nth(begin, mid - 1, n);
    } else {
        return nth(mid + 1, last, n - left_num - 1);
    }
}

int NthMinium::nth() {
    if (size_ > 0) {
        return nth(0, size_ - 1, nth_);
    }
    return -1;
}
