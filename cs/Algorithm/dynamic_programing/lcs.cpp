#include "lcs.hpp"

#include <vector>
#include <cstdint>
#include <list>

#include "alg.hpp"
#include "xlog.hpp"

namespace {

using seq = std::string;

struct Result {
    seq lcs;
};

seq longest_common_sequence(const seq &seqa, const seq &seqb)
{
/*

使用递归的方式解决最长公共子序列并不好，因为子问题每次最多只能将当前问题的规模
减少1，因此递归的深度和序列的长度成线性相关，递归栈太深。

如果将两个序列的前缀大小作为坐标构造一个二维表，左上角表示只有一个元素的两个前缀
对应问题，则表中的任何一个问题只依赖其左上方问题。

可以从第一行从左到右依次求解所有问题。

*/

    std::vector<std::vector<Result>> maps;
    maps.resize(seqa.size());
    for (auto & ref : maps) {
        ref.resize(seqb.size());
    }

    for (size_t ia = 0; ia < seqa.size(); ++ia) {
        for (size_t ib = 0; ib < seqb.size(); ++ib) {
            if (seqa[ia] == seqb[ib]) {
                if (ia > 0 && ib > 0) {
                    Result new_result = maps[ia - 1][ib - 1];
                    new_result.lcs.push_back(seqa[ia]);
                    maps[ia][ib] = new_result;
                } else {
                    Result new_result;
                    new_result.lcs.push_back(seqa[ia]);
                }
            } else {
                Result new_result;
                if (ia > 0) {
                    new_result = maps[ia - 1][ib];
                }
                if (ib > 0) {
                    if (maps[ia][ib - 1].lcs.size() > new_result.lcs.size()) {
                        new_result = maps[ia][ib-1];
                    }
                }
                maps[ia][ib] = new_result;
            }
        }
    }

    return maps[maps.size()-1][maps[0].size()-1].lcs;
}


}

void Lcs::registerTest()
{
    auto test = []() {
        seq seqa = "abcdefg";
        seq seqb = "tbdh";

        seq lcs = longest_common_sequence(seqa, seqb);
        xlog_dbg("seqa: %s\n", seqa.c_str());
        xlog_dbg("seqb: %s\n", seqb.c_str());
        xlog_dbg("lcs : %s\n", lcs.c_str());
    };

    MainAlgManager::Funcs funcs;
    funcs["base"] = test;
    MainAlgManager::getInstance().add("lcs", funcs);

    return ;
}