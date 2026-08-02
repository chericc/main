#include "rot_cutting_problem.hpp"

#include <cstddef>
#include <vector>

#include "alg.hpp"
#include "xlog.hpp"

namespace {

/// 价格表：p[1..10]，索引 0 不使用（长度为 0 的价格为 0）。
/// 例：kPrice[2] = 5 表示一整根长度为 2 的钢条可以卖 5 元。
constexpr int kPrice[] = {0, 1, 5, 8, 9, 10, 17, 17, 20, 24, 30};

/// 价格表覆盖的最大长度（= 10）。
/// 长度超过 10 的钢条无法整段定价，只能切成不超过 10 的小段来卖。
constexpr size_t kPriceSize = sizeof(kPrice) / sizeof(kPrice[0]) - 1;

/// 求解结果：最大收益 + 对应的切割方案。
struct Result {
    int revenue = 0;        // 最大收益（元）
    std::vector<int> plan;  // 切割方案：每段的长度，从左往右排列
};

/// 求长度为 n 的钢条的最大收益和切割方案。
///
/// 递推式（推导详见 rot_cutting_problem.hpp）：
///     r(0) = 0
///     r(j) = max( p[i] + r(j-i) ),   1 <= i <= j
/// 其中 r(j) 是长度为 j 的钢条的最大收益，p[i] 是长度为 i 的整段价格。
///
/// 思路：任意切割方案从左往右看，最左段长度 i 必在 [1, j] 内；
/// 左段整根卖出，右段是更小的同类型子问题。穷举 i 取最大值即可。
Result cutting_plan(int n)
{
    // ------------------------------------------------------------------
    // 第 1 步：准备两张表
    // ------------------------------------------------------------------
    // revenue[j]：长度为 j 的钢条的最大收益，即递推式中的 r(j)。
    // first[j]  ：revenue[j] 取到最大值时，最左段的长度 i。
    //             它相当于"决策表"：记录长度 j 的最优方案第一刀切多长，
    //             后面回溯还原完整切割方案时要用。
    // 初始全 0：
    //   revenue[0] = 0 就是递推式的边界条件（长度 0 的收益为 0）；
    //   first[0] 不会被使用（回溯在 left > 0 时结束）。
    std::vector<int> revenue(n + 1, 0);
    std::vector<int> first(n + 1, 0);

    // ------------------------------------------------------------------
    // 第 2 步：自底向上填表（核心）
    // ------------------------------------------------------------------
    // 外层循环 j = 1..n：按长度从小到大依次求解每个子问题。
    // 自底向上的关键：算 revenue[j] 时，所有更小的子问题 revenue[j-i]
    // 都已经算好，直接查表即可，无需递归重算（这是 DP 与暴力递归的区别）。
    for (size_t j = 1; j <= static_cast<size_t>(n); ++j) {
        // 内层循环 i = 1..j：穷举"最左段切多长"的所有可能。
        // 任意切割方案的最左段长度必在 [1, j] 中，因此这样穷举不漏方案。
        // 同时限制 i <= kPriceSize：不能切出价格表之外的长度。
        for (size_t i = 1; i <= j && i <= kPriceSize; ++i) {
            // candidate 表示"最左段长度为 i"这种方案的总收益：
            //   左段是一整根，卖 kPrice[i] 元；
            //   右段长度为 j-i，继续最优切割，收益为 revenue[j-i]。
            int candidate = kPrice[i] + revenue[j - i];

            // 对 i = 1..j 的所有方案取最大值。
            // 用严格大于比较：相等时不更新，因此并列最优时
            // 打印的是"最左段尽量短"的方案（只是打印偏好，收益相同）。
            if (candidate > revenue[j]) {
                revenue[j] = candidate;          // 更新长度 j 的最大收益
                first[j] = static_cast<int>(i);  // 记住最优时最左段长度
            }
        }
    }
    // 循环结束后：
    //   revenue[n] 就是长度为 n 的钢条的最大收益；
    //   first[1..n] 记录了每个长度的最优方案第一刀怎么切。

    // ------------------------------------------------------------------
    // 第 3 步：回溯还原切割方案
    // ------------------------------------------------------------------
    Result result;
    result.revenue = revenue[n];  // 直接取表尾，即长度 n 的最大收益

    // 从长度为 n 的钢条出发，反复"切掉最左段"：
    //   1. 查表 first[left]，得到当前长度最优时最左段应切多长；
    //   2. 把这段长度记入方案；
    //   3. 剩余长度 left 减去这段，进入下一个子问题；
    //   4. 直到剩余长度为 0，整根钢条被切完。
    // 例：n = 8 时，first[8] = 2 → 记 2，left = 6；
    //     再查 first[6] = 6 → 记 6，left = 0。方案 = [2, 6]，即切成 2 和 6 两段。
    for (int left = n; left > 0; left -= first[left]) {
        result.plan.push_back(first[left]);
    }

    return result;
}

}  // namespace

void RotCuttingProblem::registerTest()
{
    auto test = []() {
        constexpr int kMaxLength = 10;  // 价格表最大长度，依次测试 1..10
        for (int n = 1; n <= kMaxLength; ++n) {
            Result result = cutting_plan(n);
            // 打印：长度 n、最大收益、切割方案（每段长度）
            xlog_dbg("n = {}: revenue = {}, plan = {}", n, result.revenue,
                     output_elements(result.plan));
        }
    };

    // 注册到框架：模块名 "rot_cutting_problem"，测试函数名 "base"
    MainAlgManager::Funcs funcs;
    funcs["base"] = test;
    MainAlgManager::getInstance().add("rot_cutting_problem", funcs);

    return ;
}

static StaticRegistrant _reg_rot_cutting_problem(RotCuttingProblem::registerTest);
