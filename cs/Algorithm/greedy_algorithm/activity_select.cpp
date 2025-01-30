#include "activity_select.hpp"

#include <vector>
#include "alg.hpp"
#include "xlog.hpp"

/*

活动选择问题的最优子结构性质：

见 greedy_algorithm.md 。


*/

namespace {

struct Job {
    int start_time;
    int fini_time;
};

std::vector<Job> select_largest_job_set(std::vector<Job> jobs)
{
    // 我们直接用优化的单层循环替换递归的形式实现
    std::vector<Job> sel;

    size_t i = 0;
    while (i < jobs.size()) {
        sel.push_back(jobs[i]);
        int fin_time = jobs[i].fini_time;
        int got_flag = 0;
        while (i < jobs.size()) {
            if (jobs[i].start_time >= fin_time) {
                got_flag = 1;
                break;
            } else {
                ++i;
                continue;
            }
        }
        if (got_flag) {
            continue;
        } else {
            break;
        }
    }

    return sel;
}

}

void ActivitySelect::registerTest()
{
    auto fun = [](){
        // fini_time is already sorted
        std::vector<Job> jobs = {
            {1,4},
            {3,5},
            {0,6},
            {5,7},
            {3,9},
            {5,9},
            {6,10},
            {8,11},
            {8,12},
            {2,14},
            {12,16},
        };

        auto dump_job = [](std::vector<Job> const& jobs) {
            for (size_t i = 0; i < jobs.size(); ++i) {
                xlog_dbg("job[%zu]:[%d,%d]\n", i, jobs[i].start_time, jobs[i].fini_time);
            }
        };

        dump_job(jobs);
        auto largest = select_largest_job_set(jobs);
        dump_job(largest);
    };

    MainAlgManager::getInstance().add("greddy", {{"activity_select", fun}});
}