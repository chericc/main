#include "matrix_chain_multiplication.hpp"

#include <cstddef>
#include <functional>
#include <limits>
#include <string>
#include <vector>

#include "alg.hpp"
#include "xlog.hpp"

namespace {

/// 求解结果：最少乘法次数 + 对应的加括号方案。
struct Result {
    int cost = 0;       // 最少乘法次数
    std::string plan;   // 加括号方案，如 "((A1A2)A3)"
};

/// 求矩阵链 <A1, A2, ..., An> 的最少乘法次数和加括号方案。
///
/// 递推式（推导详见 matrix_chain_multiplication.hpp）：
///     m[i][j] = 0,                                          i == j
///     m[i][j] = min( m[i][k] + m[k+1][j] + p[i-1]*p[k]*p[j] ),  i <= k < j
/// 其中 m[i][j] 是计算 A_i..A_j 的最少乘法次数，
/// p[i-1]*p[k]*p[j] 是最后一步把左右两部分结果相乘的代价。
///
/// 思路：任意运算顺序，最后一步总在某个位置 k 把链分成左右两段；
/// 左右两段又各自是更小的同类型子问题。穷举 k 取最小值即可。
///
/// 参数 p：矩阵链的规模数组，p[0..n]；矩阵 A_i 的规模为 p[i-1] x p[i]。
/// 例：p = {1, 2, 3, 4} 表示 A1: 1x2、A2: 2x3、A3: 3x4，共 3 个矩阵。
Result matrix_chain_order(std::vector<int> const& p)
{
    int n = static_cast<int>(p.size()) - 1;  // 矩阵个数

    // ------------------------------------------------------------------
    // 第 1 步：准备两张表
    // ------------------------------------------------------------------
    // m[i][j]：计算 A_i..A_j 的最少乘法次数，即递推式中的 m[i][j]。
    //          只用 i <= j 的区域，其余位置不使用（初始全 0）。
    // s[i][j]：m[i][j] 取到最小值时，最后一步的分割点 k。
    //          它相当于"决策表"：记录子链 A_i..A_j 最优时从哪分开，
    //          后面回溯还原完整加括号方案时要用。
    // 边界条件 m[i][i] = 0（单个矩阵不需要乘法），已由初始全 0 满足。
    std::vector<std::vector<int>> m(n + 1, std::vector<int>(n + 1, 0));
    std::vector<std::vector<int>> s(n + 1, std::vector<int>(n + 1, 0));

    // ------------------------------------------------------------------
    // 第 2 步：自底向上填表（核心）
    // ------------------------------------------------------------------
    // 外层循环 L = 2..n：按链的长度从小到大依次求解每个子问题。
    // 自底向上的关键：长度为 L 的链 m[i][j] 被 k 分成两段后，左右两段
    // 的长度都小于 L，即 m[i][k] 和 m[k+1][j] 都已经算好，直接查表即可，
    // 无需递归重算（这是 DP 与暴力递归的区别）。
    for (int L = 2; L <= n; ++L) {
        // 链 A_i..A_j 的长度为 L，则 j = i + L - 1，i 的取值范围 [1, n-L+1]。
        for (int i = 1; i <= n - L + 1; ++i) {
            int j = i + L - 1;
            // 内层循环 k = i..j-1：穷举"最后一步从哪分开"的所有可能。
            // 任意运算顺序的最后一步分割点必在 [i, j-1] 中，因此穷举不漏方案。
            m[i][j] = std::numeric_limits<int>::max();  // 先置为无穷大，便于取最小
            for (int k = i; k <= j - 1; ++k) {
                // candidate 表示"最后一步在 k 处分开"这种方案的总代价：
                //   左段 A_i..A_k 的最少代价 m[i][k]；
                //   右段 A_{k+1}..A_j 的最少代价 m[k+1][j]；
                //   左右结果相乘的代价 p[i-1]*p[k]*p[j]
                //   （左结果规模 p[i-1] x p[k]，右结果规模 p[k] x p[j]）。
                int candidate = m[i][k] + m[k + 1][j] + p[i - 1] * p[k] * p[j];

                // 对 k = i..j-1 的所有方案取最小值。
                // 用严格小于比较：相等时不更新，因此并列最优时
                // 记住的是"分割点尽量靠左"的方案（只是打印偏好，代价相同）。
                if (candidate < m[i][j]) {
                    m[i][j] = candidate;          // 更新子链 A_i..A_j 的最少代价
                    s[i][j] = k;                  // 记住最优时最后一步的分割点
                }
            }
        }
    }
    // 循环结束后：
    //   m[1][n] 就是整个矩阵链的最少乘法次数；
    //   s[i][j] 记录了每个子链最优时最后一步的分割点。

    // ------------------------------------------------------------------
    // 第 3 步：回溯还原加括号方案
    // ------------------------------------------------------------------
    Result result;
    result.cost = m[1][n];  // 直接取表尾，即整条链的最少乘法次数

    // 从子链 A_i..A_j 出发，反复"在分割点切开"：
    //   1. 查表 s[i][j]，得到最优时最后一步的分割点 k；
    //   2. 左段 A_i..A_k、右段 A_{k+1}..A_j 各自递归处理；
    //   3. 直到 i == j，子链只剩单个矩阵，直接输出矩阵名。
    // 例：s[1][3] = 1 → 左段 A1、右段 A2..A3，方案 = "(A1(A2A3))"。
    std::function<std::string(int, int)> build =
        [&build, &s](int i, int j) -> std::string {
        if (i == j) {
            return "A" + std::to_string(i);
        }
        int k = s[i][j];
        return "(" + build(i, k) + build(k + 1, j) + ")";
    };
    result.plan = build(1, n);

    return result;
}

}  // namespace

void MatrixChainMultiplication::registerTest()
{
    auto test = []() {
        // 示例 1：md 文档中的例子，p = {1, 2, 3, 4}，
        // 即 A1: 1x2、A2: 2x3、A3: 3x4。
        // 顺序相乘需 1*2*3 + 1*3*4 = 18 次，后两个先乘需 32 次，
        // 最少应为 18 次，方案 ((A1A2)A3)。
        std::vector<int> p1 = {1, 2, 3, 4};
        Result result1 = matrix_chain_order(p1);
        xlog_dbg("p = {}: cost = {}, plan = {}", output_elements(p1),
                 result1.cost, result1.plan);

        // 示例 2：CLRS 教材经典例子，p = {30, 35, 15, 5, 10, 20, 25}，
        // 即 6 个矩阵 A1..A6。最少乘法次数为 15125 次，
        // 加括号方案为 ((A1(A2A3))((A4A5)A6))。
        std::vector<int> p2 = {30, 35, 15, 5, 10, 20, 25};
        Result result2 = matrix_chain_order(p2);
        xlog_dbg("p = {}: cost = {}, plan = {}", output_elements(p2),
                 result2.cost, result2.plan);
    };

    // 注册到框架：模块名 "matrix_chain_multiplication"，测试函数名 "base"
    MainAlgManager::Funcs funcs;
    funcs["base"] = test;
    MainAlgManager::getInstance().add("matrix_chain_multiplication", funcs);

    return ;
}

static StaticRegistrant _reg_matrix_chain_multiplication(MatrixChainMultiplication::registerTest);
