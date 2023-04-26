/**
 * compile commands:
 * g++ -I.. -std=c++11 largest_sub_array.cpp
 */

#include <vector>

#include "utility.h"

using TEST::print_elements;


/**
 * Largest sub-array algorithm of Exhaustive implemention.
 */
static int alg_largest_sub_array_exhaustive(const std::vector<int> &v, std::pair<size_t,size_t> &index)
{
    int nMax = 0;
    int bFound = 0;
    const size_t nSize = v.size();

    if (v.size() < 2)
    {
        printf ("array too small\n");
        return -1;
    }

    nMax = v[1] - v[0];

    /**
     * Here we use two level depth of for loops to go through
     * all sub-arrays of v.
     */
    for (size_t i = 0; i < nSize; ++i)
    {
        for (size_t j = i + 1; j < nSize; ++j)
        {
            /**
             * In this inner statement, i is the head index of sub-array and j 
             * is the tail index of sub-array.
             * j > i
             */
            if (v[j] - v[i] > nMax)
            {
                index.first = i;
                index.second = j;
                nMax = v[j] - v[i];
            }
        }
    }

    return bFound ? 0 : -1;
}

/**
 * be means begin and end.
 */
static int alg_largest_sub_array_dc_helper(const std::vector<int> &v, std::pair<size_t,size_t> &be, std::pair<size_t,size_t> &index)
{
    if (0 == be.second - be.first)
    {
        index.second = be.second;
        index.first = be.first;
        return 0;
    }
    else if (1 == be.second - be.first)
    {
        if (v[be.second] - v[be.first] > 0)
        {
            index.first = be.first;
            index.second = be.second;
        }
        else 
        {
            index.first = be.first;
            index.second = be.first;
        }
        return 0;
    }
    
    /** 
     * If we divide the array into two parts, there are only three conditions. First, the largest-sub-array 
     * is in the left part. Second, the largest-sub-array is in the right part. Third, the largest-sub-array's
     * beginning is in the left part and endding in the right part.
     */

    /**
     * size=3(0,2)
     * midindex=1
     * size=4(0,3)
     * midindex=1
     */
    const size_t indexMid = (be.first + be.second) / 2;
    std::pair<size_t,size_t> pairFirstBE;
    std::pair<size_t,size_t> pairFirstIndex;
    std::pair<size_t,size_t> pairSecondBE;
    std::pair<size_t,size_t> pairSecondIndex;
    std::pair<size_t,size_t> pairMiddleBE;
    std::pair<size_t,size_t> pairMiddleIndex;
    int nTemp = 0;

    pairFirstBE.first = be.first;
    pairFirstBE.second = indexMid;
    pairSecondBE.first = indexMid;
    pairSecondBE.second = be.second;

    /* first */
    alg_largest_sub_array_dc_helper (v, pairFirstBE, pairFirstIndex);

    /* second */
    alg_largest_sub_array_dc_helper (v, pairSecondBE, pairSecondIndex);

    /* middle */

    /* mid left */
    nTemp = v[be.first];
    pairMiddleIndex.first = be.first;
    for (size_t i = be.first; i < indexMid; ++i)
    {
        if (v[i] < nTemp)
        {
            nTemp = v[i];
            pairMiddleIndex.first = i;
        }
    }
    /* mid right */
    nTemp = v[indexMid];
    pairMiddleIndex.second = indexMid;
    for (size_t i = indexMid; i < be.second; ++i)
    {
        if (v[i] > nTemp)
        {
            nTemp = v[i];
            pairMiddleIndex.second = i;
        }
    }

    /* conquer */
    int nLeftMax = v[pairFirstIndex.second] - v[pairFirstIndex.first];
    int nRightMax = v[pairSecondIndex.second] - v[pairSecondIndex.first];
    int nMidMax = v[pairMiddleIndex.second] - v[pairMiddleIndex.first];

    if (nLeftMax > nRightMax && nLeftMax > nMidMax)
    {
        index = pairFirstIndex;
    }
    else if (nMidMax > nLeftMax && nMidMax > nRightMax)
    {
        index = pairMiddleIndex;
    }
    else if (nRightMax > nLeftMax && nRightMax > nMidMax)
    {
        index = pairSecondIndex;
    }

    return 0;
}

/**
 * Largest sub-array algorithm of Divide & Conquer implemention.
 */
static int alg_largest_sub_array_divide_and_conquer(const std::vector<int> &v, std::pair<size_t,size_t> &index)
{
    std::pair<size_t,size_t> be(0,v.size());

    return alg_largest_sub_array_dc_helper (v, be, index);
}

int main()
{
    /** 
     * 17 days stock price. 
     * The best choice is to buy at day 7 and sell at day 11. We will get a difference of
     * 106 - 63 = 43.
     */
    std::vector<int> v = {100,113,110,85,105,102,86,63,81,101,94,106,101,79,94,90,97};
    std::pair<size_t,size_t> pairResult1;
    std::pair<size_t,size_t> pairResult2;

    print_elements (v, "original array:");

    alg_largest_sub_array_exhaustive (v, pairResult1);
    std::cout << "result: " << pairResult1.first << "," << pairResult1.second << std::endl;
    std::cout << "low=" << v.at(pairResult1.first) << "," << 
        "high=" << v.at(pairResult1.second) << std::endl;

    alg_largest_sub_array_divide_and_conquer (v, pairResult2);
    std::cout << "result: " << pairResult2.first << "," << pairResult2.second << std::endl;
    std::cout << "low=" << v.at(pairResult2.first) << "," << 
        "high=" << v.at(pairResult2.second) << std::endl;

    return 0;
}