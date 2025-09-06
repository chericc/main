#include <gtest/gtest.h>

#include "xlog.h"

int main(int argc, char** argv) {
    // xlog_setmask(XLOG_ALLOW_ERR);

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}