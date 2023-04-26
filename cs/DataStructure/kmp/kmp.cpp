
/**
 * @brief 串的模式匹配
 * 
 * 暴力搜索算法的两种写法。
 * 
 * KMP算法。
 */

#include <iostream>
#include <string>
#include <string.h>
#include <vector>

/**
 * @brief 字串模式匹配的暴力搜索法
 * 
 * 算法描述：
 * 遍历@szSrc的每个位置并依次和@szToken比较，直到
 * 找到完全匹配。
 * 
 * 算法特点：双层循环
 * 
 * 性能分析：
 * 记源串长度为n，模式串长度为m。
 * 时间复杂度：O(nm)
 * 空间复杂度：O(1)
 */
const char *strstr_normal_v1(const char *szSrc, const char *szToken) {
    bool bFound = false;
    const char *szResult = nullptr;
    const char *pItSrc = szSrc;
    while (*pItSrc != '\0') {
        const char *pItSrcTemp = pItSrc;
        const char *pItTokenTemp = szToken;
        while (*pItSrcTemp != '\0' && *pItTokenTemp != '\0') {
            if (*pItSrcTemp == *pItTokenTemp) {
                ++pItSrcTemp;
                ++pItTokenTemp;
            }
            else {
                break;
            }
        }
        if (*pItTokenTemp == '\0') {
            bFound = true;
            szResult = pItSrc;
            break;
        }
        ++pItSrc;
    }
    return bFound ? szResult : nullptr;
}

/**
 * @brief 子串模式匹配的暴力搜索方法v2
 * 
 * 算法特点：指针回退。
 * 算法描述：两个指针分别指向源串和模式串，如果匹配失败，则回退
 * 已匹配的长度。
 */
const char *strstr_normal_v2(const char *szSrc, const char *szToken) {
    int i = 0, j = 0;

    if (*szToken == '\0') {
        return nullptr;
    }

    while (szSrc[i] != '\0' && szToken[j] != '\0') {
        if (szSrc[i] == szToken[j]) {
            ++i;
            ++j;
        }
        else {
            i = i - j + 1; /* 回退 */
            j = 0;
        }
    }

    if (szToken[j] == '\0') {
        return szSrc + i - j;
    }
    else {
        return nullptr;
    }
}

/**
 * KMP算法作者
 * D.E.Knuth, V.R.Pratt, J.H.Morris
 * 
 * 有修改
 * 

求PM表：

i         0 1 2 3 4 5 6 7 8
对于模式串 1 2 1 1 2 3 1 2 1
PM        0 0 1 1 2 0 1 2 3
next     -1 0 0 1 1 2 0 1 2
流程：
PM[0] = 0
i = 1, i表示下一个处理的子串
j = 0, j表示上一个处理的子串的PM值
p[j]!=p[i],而j==0,取pm[i]=j,即pm[1]=0
i = i+1 = 2
j = 0
p[j]==p[i],取pm[i]=j+1,即pm[2]=1
i = i+1 = 3
j = j+1 = 1
p[j]!=p[i],取j=pm[j-1],即j=pm[0]=0,
p[j]==p[i],取pm[i]=j+1,即pm[3]=1
i = i+1 = 4
j = 1
p[j]==p[i],取pm[i]=j+1,即pm[4]=2
i = i+1 = 5
j = 2
p[j]!=p[i],取j=pm[j-1],即j=pm[1]=0
p[j]!=p[i],而j==0,取pm[i]=j,即pm[5]=0

...

利用next表进行模式匹配
1 2 1 3 ... 目标字符串
1 2 1 1 2 3 ... 模式字符串
i 为目标字符串指针
j 为模式字符串指针
p[3]匹配失败，查next[3]=1，即
取j=1,继续进行匹配。
如果next为-1，则整体向前一步
（-1代表第一个字符就匹配失败）

 */
const char *strstr_kmp(const char *szSrc, const char *szToken) {
    int nLenToken = strlen (szToken);

    std::vector<int> pm(nLenToken); /* pm表 */

    /* 求模式串的部分匹配（PM）表 */
    pm[0] = 0;
    int i = 1; /* [0,i]表示当前计算pm值的头部子串 */ 
    int j = 0; /* j为上一个已处理字符串的PM值 */
    while (i < nLenToken) {
        if (szToken[j] != szToken[i]) {
            if (j == 0) {
                pm[i]=j;
                ++i;
            }
            else {
                j = pm[j-1];
            }
        }
        else {
            pm[i] = j+1;
            ++i;
            ++j; /* j=pm[i] */
        }
    }

    std::cout << "pm:" << std::endl;
    for (auto i : pm) {
        std::cout << i << " ";
    }
    std::cout << std::endl;

    /* PM表转next表 */
    std::vector<int> next(nLenToken); /* next表 */

    next[0] = -1;
    for (std::size_t k = 1; k < next.size(); ++k) {
        next[k] = pm[k-1];
    }

    std::cout << "next:" << std::endl;
    for (auto i : next) {
        std::cout << i << " ";
    }
    std::cout << std::endl;

    /* 利用next表进行模式匹配 */
    i = 0; /* i表示szSrc指针 */
    j = 0; /* j表示szToken指针 */
    while (szSrc[i] != '\0' && szToken[j] != '\0') {
        if (szSrc[i] == szToken[j]) {
            ++i;
            ++j;
        }
        else {
            j = next[j];
            if (j < 0) {
                ++i;
                j = 0; /* ++j */
            }
        }
    }

    if (j == nLenToken) {
        return szSrc + i - j;
    }
    else {
        return nullptr;
    }
}

/**
 * 在v1基础上，避免了p[j]=p[next[j]]引起重复比较的情况
 * 也即 p[j+1]=p[pm[j]] 的情况
 * 
对于模式串 1 1 1 1 2
PM        0 0 0 0 0
流程：
PM[0]=0
i = 1
j = 0
p[j]==p[i],尝试取pm[i]=j+1,此时p[i+1]==p[pm[i]]==p[j+1],
故尝试取pm[i]=pm[j-1],而j==0,故取pm[i]=j,即pm[i]=0
i = i+1 = 2
j = 0
p[j]==p[i],尝试取pm[i]=j+1,此时p[i+1]==p[pm[i]]==p[j+1],
故尝试取pm[i]=pm[j-1],而j==0,故取pm[i]=j,即pm[i]=0

...

 */
const char *strstr_kmp_v2(const char *szSrc, const char *szToken) {
    int nLenToken = strlen (szToken);

    std::vector<int> pm(nLenToken); /* pm表 */

    /* 求模式串的部分匹配（PM）表 */
    pm[0] = 0;
    int i = 1; /* [0,i]表示当前计算pm值的头部子串 */ 
    int j = 0; /* j为上一个已处理字符串的PM值 */
    while (i < nLenToken) {
        if (szToken[j] != szToken[i]) {
            if (j == 0) {
                pm[i]=j;
                ++i;
            }
            else {
                j = pm[j-1];
            }
        }
        else {
            if (szToken[i+1] == szToken[j+1]) {
                if (j == 0) {
                    pm[i] = 0;
                }
                else {
                    pm[i] = pm[j-1];
                }
            }
            else {
                pm[i] = j+1;
            }
            
            ++i;
            ++j; /* j=pm[i] */
        }
    }

    std::cout << "pm:" << std::endl;
    for (auto i : pm) {
        std::cout << i << " ";
    }
    std::cout << std::endl;

    /* PM表转next表 */
    std::vector<int> next(nLenToken); /* next表 */

    next[0] = -1;
    for (std::size_t k = 1; k < next.size(); ++k) {
        next[k] = pm[k-1];
    }

    std::cout << "next:" << std::endl;
    for (auto i : next) {
        std::cout << i << " ";
    }
    std::cout << std::endl;

    /* 利用next表进行模式匹配 */
    i = 0; /* i表示szSrc指针 */
    j = 0; /* j表示szToken指针 */
    while (szSrc[i] != '\0' && szToken[j] != '\0') {
        if (szSrc[i] == szToken[j]) {
            ++i;
            ++j;
        }
        else {
            j = next[j];
            if (j < 0) {
                ++i;
                j = 0; /* ++j */
            }
        }
    }

    if (j == nLenToken) {
        return szSrc + i - j;
    }
    else {
        return nullptr;
    }
}

int main() {
    while (true) {
        std::cout << "************" << std::endl;
        std::string strSrc, strToken;
        std::cout << "src: " << std::endl;
        std::cin >> strSrc;
        std::cout << "token: " << std::endl;
        std::cin >> strToken;

        std::cout << "src=" << strSrc << std::endl;
        std::cout << "token=" << strToken << std::endl;

        const char *szResult = nullptr;
        // szResult = strstr_normal_v1(strSrc.c_str(), strToken.c_str());
        // szResult = strstr_normal_v2(strSrc.c_str(), strToken.c_str());
        // szResult = strstr_kmp(strSrc.c_str(), strToken.c_str());
        szResult = strstr_kmp_v2(strSrc.c_str(), strToken.c_str());
        if (szResult != nullptr) {
            std::cout << "i=" << szResult - strSrc.c_str() << std::endl;
        }
        else {
            std::cout << "not found!" << std::endl;
        }
        std::cout << "************\n" << std::endl;
    }
}