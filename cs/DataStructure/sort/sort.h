
#include <vector>

/* 直接插入排序 */
void sort_insert(std::vector<int>& ref);

/* 折半插入排序 */
void sort_insert_half(std::vector<int>& ref);

/* 希尔排序 */
void sort_shell(std::vector<int>& ref);

/* 冒泡排序 */
void sort_bubble(std::vector<int>& ref);

/* 快速排序 */
void sort_fast(std::vector<int>& ref);

/* 简单选择排序 */
void sort_select(std::vector<int>& ref);

/* 堆排序 */
void sort_heap(std::vector<int>& ref);

/* 归并排序 */
void sort_merge(std::vector<int>& ref);

/* 基数排序 */
void sort_radix(std::vector<int>& ref);

/* 外排 */
void sort_outside(const char* szInputFile, const char* szOutputFile);