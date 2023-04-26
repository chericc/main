
#include <string>
#include <iostream>
#include <list>
#include <vector>
#include <memory>
#include <tuple>
#include <map>

#include <cassert>

template<typename T>
class Array2
{
public:
    Array2() = default;
    Array2(int x, int y, T const& v)
    {
        a2.resize(y);
        for (int i = 0; i < y; ++i)
        {
            a2[i].resize(x, v);
        }
    }
    std::vector<std::vector<T>> a2;
};

enum Direction
{
    Left,
    Up,
    UpLeft,
};

struct LCSMapItem
{
    int lcslen{0};
    int dir{0}; // Direction
    int len1{0};
    int len2{0};
};

struct LCSInfo
{
    Array2<LCSMapItem> array2_lcs_dict;
};

std::ostream& operator <<(std::ostream &os, const Array2<int> &a)
{
    return os;
}

/**

Understanding of direction:

Left: 0
Up: 1
UpLeft: 2

  1    A    C    B
2 0 
A      2
B      1
C           2
B                2

*/

int lcs(const std::string &str1, int len1, const std::string &str2, int len2, LCSInfo &info)
{
    // std::cout << "c[" << len1 << "," << len2 << "]" << std::endl;

    assert(len1 >= 0);
    assert(len2 >= 0);

    LCSMapItem item{};

    if (len1 == 0 || len2 == 0)
    {
        return 0;
    }
    else
    {

        if (info.array2_lcs_dict.a2[len2 - 1][len1 - 1].lcslen >= 0)
        {
            return info.array2_lcs_dict.a2[len2 - 1][len1 - 1].lcslen;
        }

        if (str1.at(len1 - 1) == str2.at(len2 - 1))
        {
            int len_tmp = lcs(str1, len1 - 1, str2, len2 - 1, info) + 1;

            item.dir = UpLeft;
            item.lcslen = len_tmp;
            item.len1 = len1;
            item.len2 = len2;
            info.array2_lcs_dict.a2[len2 - 1][len1 - 1] = item;
            return len_tmp;
        }
        else
        {
            int len_tmp1 = lcs(str1, len1 - 1, str2, len2, info);
            int len_tmp2 = lcs(str1, len1, str2, len2 - 1, info);

            if (len_tmp1 >= len_tmp2)
            {
                item.dir = Left;
                item.lcslen = len_tmp1;
                item.len1 = len1;
                item.len2 = len2;
                info.array2_lcs_dict.a2[len2 - 1][len1 - 1] = item;
                return len_tmp1;
            }
            else
            {
                item.dir = Up;
                item.lcslen = len_tmp2;
                item.len1 = len1;
                item.len2 = len2;
                info.array2_lcs_dict.a2[len2 - 1][len1 - 1] = item;
                return len_tmp2;
            }
        }
    }

    std::cout << "error" << std::endl;

    return 0;
}

int main()
{
    std::string s1 = "ACCGGTCGAGTGCGCGGAAGCCGGCCGAA";
    std::string s2 = "GTCGTTCGGAATGCCGTTGCTCTGTAAA";
    std::string s3 = "GTCGTCGGAAGCCGGCCGAA";

    LCSInfo info{};
    LCSMapItem item{};
    item.lcslen = -1;
    info.array2_lcs_dict = Array2<LCSMapItem>(s1.length(), s2.length(), item);

    int len = lcs(s1, (int)s1.size(), s2, (int)s2.size(), info);

    int map_size = 0;

    for (int y = 0; y < info.array2_lcs_dict.a2.size(); ++y)
    {
        for (int x = 0; x < info.array2_lcs_dict.a2[y].size(); ++x)
        {
            LCSMapItem const& r = info.array2_lcs_dict.a2[y][x];
            if (r.lcslen >= 0)
            {
                // std::cout << "[" << r.len1 << "," << r.len2 << "]" << "len=" << r.lcslen << ",dir=" << r.dir << std::endl;
                ++map_size;
            }
        }
    }

    {
        std::string output;

        int x = s1.length() - 1;
        int y = s2.length() - 1;

        std::cout << "lcs:";

        Array2<LCSMapItem> const& map = info.array2_lcs_dict;
        while (x >= 0 && y >= 0)
        {
            assert(map.a2[y][x].lcslen >= 0);
            switch(map.a2[y][x].dir)
            {
                case Left:
                {
                    --x;
                    break;
                }
                case Up:
                {
                    --y;
                    break;
                }
                case UpLeft:
                default:
                {
                    output.insert(output.begin(), s1[x]);
                    --x;
                    --y;
                }
            }
        }

        std::cout << output << std::endl;
        std::cout << "len=" << len << std::endl;

        if (output == s3)
        {
            std::cout << "result is right!!!" << std::endl;
        }
        else 
        {
            std::cout << "result is wrong!!!" << std::endl;
        }
    }

    std::cout << "map_size=" << map_size << std::endl;

    return 0;
}