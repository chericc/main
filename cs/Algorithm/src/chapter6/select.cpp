/**
 * compile commands:
 * g++ -I.. -std=c++11 select.cpp -g
 */

#include <vector>
#include <algorithm>
#include <random>

#include "utility.h"

/**
 * @brief Group vec into two parts by a seperator. 
 * All elements in left part are not bigger than seperator. All elements in
 * right part are not smaller than seperator. Seperator is between left
 * part and right part.
 */
std::size_t select_nth_helper_group (std::vector<int> &vec, std::size_t nBegin, std::size_t nEnd, std::size_t nSeperator)
{
    /**
     * Index x points to the next element of left part.
     * Index y points to the next element of right part.
     */
    std::size_t x = nBegin;
    std::size_t y = nBegin;

    /* Move seperator to end. */
    std::swap (vec[nSeperator], vec[nEnd]);
    nSeperator = nEnd;

    while (y < nEnd)
    {
        if (vec[y] >= vec[nSeperator])
        {
            ++y;
        }
        else 
        {
            std::swap (vec[x], vec[y]);
            ++x;
            ++y;
        }
    }

    std::swap (vec[x], vec[nSeperator]);
    nSeperator = x;

    return nSeperator;
}

int select_nth_helper (std::vector<int> &vec, std::size_t nBegin, std::size_t nEnd, std::size_t nNth)
{
    std::size_t nSeperator = select_nth_helper_group (vec, nBegin, nEnd, nBegin);

    if (nNth < nBegin || nNth > nEnd)
    {
        return -1;
    }

    if (nNth == nSeperator)
    {
        return vec[nSeperator];
    }
    else if (nNth < nSeperator)
    {
        return select_nth_helper (vec, nBegin, nSeperator - 1, nNth);
    }
    else 
    {
        return select_nth_helper (vec, nSeperator + 1, nEnd, nNth);
    }

    return 0;
}

/**
 * @brief Find out the nth element of input vec.
 * nNth begins at 0.
 */
int select_nth (std::vector<int> &vec, size_t nNth)
{
    if (vec.size() > 0 && nNth >= 0 && nNth < vec.size())
    {
        return select_nth_helper (vec, 0, vec.size() - 1, nNth);
    }

    return 0;
}

/**
 * @brief A non-recursive implementation.
 */
int select_nth_v2(std::vector<int> &vec, size_t nNth)
{
    if (vec.empty() || nNth < 0 || nNth >= vec.size())
    {
        return -1;
    }

    std::size_t nBegin = 0;
    std::size_t nEnd = vec.size() - 1;
    std::size_t nSeperator = nBegin;

    while (nSeperator != nNth)
    {
        std::size_t nNewSeperator = select_nth_helper_group (vec, nBegin, nEnd, nSeperator);
        if (nNth == nNewSeperator)
        {
            nSeperator = nNewSeperator;
            break;
        }
        else if (nNth < nNewSeperator)
        {
            nEnd = nNewSeperator - 1;
            nSeperator = nBegin;
        }
        else 
        {
            nBegin = nNewSeperator + 1;
            nSeperator = nBegin;
        }
    }

    return nSeperator;
}

int main()
{
    const std::size_t nSize = 1000 * 1000 * 1;
    std::vector<int> vecData (nSize);
    for (std::size_t i = 0; i < nSize; ++i)
    {
        vecData[i] = i;
    }
    std::vector<int> vecBackup;
    std::vector<int> vecBackupA;

    TIME_POINT_BEGIN(A);
    std::shuffle (vecData.begin(), vecData.end(), std::default_random_engine());
    TIME_POINT_END(A);

    vecBackup = vecData;
    vecBackupA = vecData;

    TIME_POINT_BEGIN(B);
    int nNthValue = select_nth (vecData, vecData.size() / 2);
    TIME_POINT_END(B);
    
    TIME_POINT_BEGIN(C);
    int nNthValueA = select_nth_v2 (vecBackupA, vecBackupA.size() / 2);
    TIME_POINT_END(C);

    TIME_POINT_BEGIN(D);
    std::nth_element (vecBackup.begin(), vecBackup.begin() + vecBackup.size() / 2, vecBackup.end());
    TIME_POINT_END(D);

    std::cout << "value=" << nNthValue << " " << vecData[vecData.size() / 2] << std::endl;
    std::cout << "value=" << nNthValueA << " " << vecBackupA[vecBackupA.size() / 2] << std::endl;
    std::cout << "value=" << vecBackup[vecBackup.size() / 2] << std::endl;

    return 0;
}
