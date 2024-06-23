#include <gtest/gtest.h>

#include "xthread.hpp"

TEST(xthread, base) {
    auto lam = []() -> void { ; };

    XThread trd(lam);
    trd.join();
}