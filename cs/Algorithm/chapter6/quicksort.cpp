/**
 * compile commands:
 * g++ -I.. -std=c++11 quicksort.cpp -g
 */

#include <vector>
#include <algorithm>
#include <random>

#include "utility.h"

using TEST::print_elements;

/**
 * @brief Group vec into two parts. Elements of left part are all smaller than vec[nSeperator],
 * and elements of right part are all not smaller than vec[nSeperator]. The vec[nSeperator] will
 * be put int the middle beween left part and right part.
 * @return Return the index where seperator is located.
 */
std::size_t quick_sort_helper_group(std::vector<int> &vec, std::size_t nSeperator, std::size_t nBegin, std::size_t nEnd)
{
    std::size_t x = nBegin, y = nBegin;

    std::swap (vec[nEnd], vec[nSeperator]);
    nSeperator = nEnd;

    /**
     * x points to the next element of left part.
     * y points to the next element of right part.
     */

    while (y < nEnd)
    {
        if (vec[y] >= vec[nEnd])
        {
            ++y;
        }
        else if (vec[y] < vec[nEnd])
        {
            std::swap (vec[x], vec[y]);
            ++x;
            ++y;
        }
    }

    if (x < y)
    { /* right part not empty */
        std::swap (vec[x], vec[nSeperator]);
        return x;
    }

    return nSeperator;
}

void quick_sort_helper(std::vector<int> &vec, std::size_t nBegin, std::size_t nEnd)
{
    /* Choose the one third of the vec as seperator for NO REASON. */
    // int nSeperatorIndex = nBegin + (nEnd - nBegin) / 3;
    int nSeperatorIndex = nBegin;
    int nSeperatorNew = quick_sort_helper_group (vec, nSeperatorIndex, nBegin, nEnd);

    if (nSeperatorNew > nBegin + 1)
    {
        quick_sort_helper (vec, nBegin, nSeperatorNew - 1);
    }

    if (nEnd > nSeperatorNew + 1)
    {
        quick_sort_helper (vec, nSeperatorNew + 1, nEnd);
    }

    return ;
}

void quick_sort(std::vector<int> &vec)
{
    if (! vec.empty())
    {
        quick_sort_helper (vec, 0, vec.size() - 1);
    }
    return ;
}

int main()
{
    const int nSize = 1000 * 10;
    std::vector<int> vecData (nSize);
    for (int i = 0; i < nSize; ++i)
    {
        vecData[i] = i;
    }
    std::vector<int> vecBackup;

    TIME_POINT_BEGIN(A);
    std::shuffle (vecData.begin(), vecData.end(), std::default_random_engine());
    TIME_POINT_END(A);

    TIME_POINT_BEGIN(B);
    vecBackup = vecData;
    TIME_POINT_END(B);

    TIME_POINT_BEGIN(C);
    quick_sort (vecData);
    TIME_POINT_END(C);

    TIME_POINT_BEGIN(D);
    std::sort (vecBackup.begin(), vecBackup.end());
    TIME_POINT_END(D);

    TIME_POINT_BEGIN(E);
    if (vecData != vecBackup)
    {
        std::cout << "error!" << std::endl;
    }
    TIME_POINT_END(E);

    return 0;
}
