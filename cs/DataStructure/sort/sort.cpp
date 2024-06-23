
#include "sort.h"

#include <assert.h>

namespace {

/**
 * 折半查找，返回应插入的位置
 * 2 3 5 7 1
 * begin = 0
 * end = 4
 * ele = 1
 **/
int half_locate(const std::vector<int>& ref, int begin, int end, int ele) {
    if (begin >= end) {
        return -1;
    }

    int ret = -1;
    while (true) {
        if (begin == end) {
            break;
        }
        int mid = (begin + end) / 2;
        if (ref[mid] < ele) {
            if (begin == mid) {
                break;
            }
            begin = mid;
        } else if (ref[mid] == ele) {
            ret = mid;
            break;
        } else {
            end = mid;
        }
    }

    if (ref[begin] < ele) {
        ret = begin + 1;
    } else if (ref[begin] == ele) {
        while (ref[begin] == ele && begin < (int)ref.size()) {
            ++begin;
        }
        ret = begin;
    } else {
        ret = begin;
    }

    return ret;
}

/**
 * 参考 <https://www.runoob.com/w3cnote/quick-sort.html>
 * 用挖坑填坑的方式来理解。
 **/
void sort_fast_div(std::vector<int>& ref, int low, int high) {
    if (!(low < high)) {
        return;
    }
    int l = low;
    int r = high;
    int midele = ref[low];  // 挖坑
    while (low < high) {
        while (ref[high] >= midele && low < high) --high;
        ref[low] = ref[high];  // 挖坑填坑，新坑为 high
        while (ref[low] < midele && low < high) ++low;
        ref[high] = ref[low];  // 挖坑填坑，新坑为 low
    }
    ref[low] = midele;  // 将会合处的坑恢复为最初挖出的元素

    sort_fast_div(ref, l, low - 1);
    sort_fast_div(ref, low + 1, r);
}

/**
 * 堆
 * 0 1 2 3 4 5 6
 * -->
 *           0
 *      1           2
 *    3   4      5     6
 **/

#define HEAP_LEFT(i) (2 * i + 1)
#define HEAP_RIGHT(i) (2 * i + 2)
#define HEAP_PARENT(i) ((i - 1) / 2)
#define HEAP_HAS_LEFT(i, size) (HEAP_LEFT(i) < size)
#define HEAP_HAS_RIGHT(i, size) (HEAP_RIGHT(i) < size)

void sort_heap_adjust(std::vector<int>& ref, int size, int i) {
    while (HEAP_HAS_LEFT(i, size)) {
        int j = i;
        if (HEAP_HAS_RIGHT(i, size)) {
            if (ref[HEAP_RIGHT(i)] > ref[j]) {
                j = HEAP_RIGHT(i);
            }
        }
        if (HEAP_HAS_LEFT(i, size)) {
            if (ref[HEAP_LEFT(i)] > ref[j]) {
                j = HEAP_LEFT(i);
            }
        }
        if (j == i) {
            break;
        }
        std::swap(ref[j], ref[i]);
        i = j;
    }
}

void sort_merge_div(std::vector<int>& ref, int begin, int end) {
    if (begin >= end) {
        return;
    }

    int mid = (begin + end) / 2;

    sort_merge_div(ref, begin, mid);
    sort_merge_div(ref, mid, end);

    std::vector<int> vbuf(end - begin);
}

}  // end of anonymous namespace

/* 直接插入排序 */
void sort_insert(std::vector<int>& ref) {
    for (int i = 1; i < (int)ref.size(); ++i) {
        for (int j = i; j - 1 >= 0; --j) {
            if (ref[j - 1] > ref[j]) {
                std::swap(ref[j - 1], ref[j]);
            } else {
                break;
            }
        }
    }
}

/* 折半插入排序 */
void sort_insert_half(std::vector<int>& ref) {
    for (int i = 1; i < (int)ref.size(); ++i) {
        int ret = half_locate(ref, 0, i, ref[i]);
        assert(ret >= 0);
        for (int j = i; j - 1 >= ret; --j) {
            std::swap(ref[j - 1], ref[j]);
        }
    }
}

/* 希尔排序 */
void sort_shell(std::vector<int>& ref) {
    std::vector<int> arr = {5, 3, 1}; /* 这里直接取确定的一个增量序列 */
    for (int m = 0; m < (int)arr.size(); ++m) { /* m表示增量序列序号 */
        for (int k = 0; k < arr[m]; ++k) {      /* k表示趟内增量 */
            /* 简单插入排序 */
            for (int i = 1; i * arr[m] + k < (int)ref.size(); ++i) {
                // int iindex = i*arr[m]+k;
                for (int j = i; (j - 1) * arr[m] + k >= 0; --j) {
                    int jindexpre = (j - 1) * arr[m] + k;
                    int jindex = j * arr[m] + k;
                    if (ref[jindexpre] > ref[jindex]) {
                        std::swap(ref[jindexpre], ref[jindex]);
                    } else {
                        break;
                    }
                }
            }
        }
    }
}

/* 冒泡排序 */
void sort_bubble(std::vector<int>& ref) {
    bool sort_flag = false;

    for (int i = ref.size() - 1; i > 0; --i) {
        sort_flag = false;
        for (int j = 0; j < i; ++j) {
            if (ref[j] > ref[j + 1]) {
                std::swap(ref[j], ref[j + 1]);
                sort_flag = true;
            }
        }
        if (!sort_flag) {
            break;
        }
    }
}

/* 快速排序 */
void sort_fast(std::vector<int>& ref) { sort_fast_div(ref, 0, ref.size() - 1); }

/* 简单选择排序 */
void sort_select(std::vector<int>& ref) {
    for (int i = ref.size() - 1; i >= 1; --i) {
        int select = 0;
        for (int j = 0; j < i; ++j) {
            if (ref[j] > ref[select]) {
                select = j;
            }
        }
        std::swap(ref[select], ref[i]);
    }
}

void sort_heap(std::vector<int>& ref) {
    int size = ref.size();

    for (int k = size; k > 1; --k) {
        /* 建堆 */
        for (int i = k / 2 - 1; i >= 0; --i) {
            sort_heap_adjust(ref, k, i);
        }
        /* 取出根节点 */
        std::swap(ref[0], ref[k - 1]);
    }
    return;
}

/* 归并排序 */
void sort_merge(std::vector<int>& ref) {}

/* 基数排序 */
void sort_radix(std::vector<int>& ref);

/* 外排 */
void sort_outside(const char* szInputFile, const char* szOutputFile);