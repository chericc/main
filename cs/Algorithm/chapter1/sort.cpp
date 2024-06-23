
/**
 * compile commands:
 * g++ -I.. -std=c++11 sort.cpp
 */

#include <algorithm>
#include <random>
#include <vector>

#include "utility.h"

using TEST::print_elements;

/**
 * This is the sort algorithm using insertion method.
 * Though the method is simple, but here I still want to
 * show the details. That is, to show how this algorithm
 * works.
 */
static int sort_insert(std::vector<int>& v) {
    for (std::size_t i = 2; i < v.size();
         ++i) { /* i stands for current processing element index */
        for (std::size_t j = i; j > 0;
             --j) { /* Purpose:
                     * insert element of index i to the left
                     * position by swaping adjacing elements.
                     **/
            if (v[j] < v[j - 1]) {
                std::swap(v[j - 1], v[j]);
            } else {
                break;
            }
        }
    }

    return 0;
}

int main(int argc, char* argv[]) {
    std::vector<int> v = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    std::shuffle(v.begin(), v.end(), std::default_random_engine());
    print_elements(v, "original array:");

    sort_insert(v);
    print_elements(v, "after insert-sort:");

    return 0;
}