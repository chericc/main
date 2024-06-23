#include <gtest/gtest.h>

#include "xlog.hpp"

int main(int argc, char** argv) {
    std::vector<FILE*> fps = {stdout};
    xlog_setoutput(fps);
    xlog_setmask(XLOG_ALLOW_ERR);

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}