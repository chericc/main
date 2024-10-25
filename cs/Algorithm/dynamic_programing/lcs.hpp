#pragma once

#include <cstddef>

/*

LCS

longest-common-subsequence

公共子序列：
对两个串，其公共子序列的所有符号均出现在两个串中，并且公共子序列中这些符号的顺序和这两个串中这些符号的顺序相同。

最长公共子序列：
两个串的最长的公共子序列。

LCS的特征：

把序列的前缀序列作为子问题处理。

定义X的第i前缀为Xi=[x1,x2,...,xi]。
有以下定理：
令X=[x1,x2,...,xm]和Y=[y1,y2,...,yn]为两个序列，Z=[z1,z2,...,zk]为X和Y的任意LCS。
1. 如果xm == yn，则 xm == yn == zk ，且 Xm-1 和 Yn-1 有 LCS 为 Zk-1 。
2. 如果xm != yn，
 - 如果 xm != zk ，则 Xm-1 和 Yn 有 LCS 为 Zk 。
 - 如果 yn != zk ，则 Xm 和 Yn-1 有 LCS 为 Zk 。

证明：
1. 若 xm == yn 且 xm != zk ，则 xm 可以附加到 Z 的末尾，产生一个长度为 k+1 的公共子序列，
这与 Z 是 LCS 矛盾（最长子序列的长度为 k），则必有 xm == yn == zk。 由前面的结论和公共子
序列的定义，zk 为 X 和 Y 中的最后一个相同的元素（也是最后一个元素），因此有 Zk-1 为 Xm-1 
和 Yn-1 的公共子序列。再证明 Zk-1 是 Xm-1 和 Yn-1 的LCS。假设存在更长的公共子序列，则将
zk 加到这个公共子序列的尾部，会得到长度比 k 更长的LCS，这与 Z 为LCS矛盾。因此得证。
2. -
2.1 对第一种情况，容易得到 Xm-1 和 Yn 有公共子序列 Z。如果 Z 不是其 LCS，则其 LCS 长度大于
 k，这与 Z 是 X 和 Y 的 LCS 矛盾。
2.2 对称可证。

*/

class Lcs {
public:
    static void registerTest();
};