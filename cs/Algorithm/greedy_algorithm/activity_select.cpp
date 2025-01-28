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
                xlog_dbg("job[%d]:[%d,%d]\n", i, jobs[i].start_time, jobs[i].fini_time);
            }
        };

        dump_job(jobs);
        auto largest = select_largest_job_set(jobs);
        dump_job(largest);
    };

    MainAlgManager::getInstance().add("greddy", {{"activity_select", fun}});
}