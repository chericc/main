#include <cstdint>

#include "xlog.h"
#include "spdlog/fmt/chrono.h"
#include "spdlog/fmt/ranges.h"

int main(int argc, char *argv[])
{
    xlog_dbg("hello");

    {
        int i = 123;
        xlog_dbg("{}", i);
    }

    {
        int i = 0x123;
        xlog_dbg("{:#x}", i);
    }

    {
        char str[16] = "hello123";
        xlog_dbg("{:.{}}", str, 5);
    }

    {
        float f = 3.14159;
        xlog_dbg("{:.3f}", f);
    }

    {
        char str[16] = "hell0";
        str[1] = 0x12;
        xlog_dbg("{:?}", str);
    }

    {
        uint32_t number = 0x1234;
        xlog_dbg("{:#b}", number);
    }

    {
        char *p = nullptr;
        xlog_dbg("{:p}", p);
        xlog_dbg("{}", fmt::ptr(p));
    }

    {
        xlog_dbg("a={a},b={b},c={c}",
            fmt::arg("c", 3), 
            fmt::arg("b", 2),
            fmt::arg("a", 1));
    }

    {
        std::vector<int> v{1,2,3};
        xlog_dbg("{}", v);
    }

    {
        using SysClock = std::chrono::system_clock;
        auto now = SysClock::now();
        xlog_dbg("{:%F %T}", now);
    }

    return 0;
}