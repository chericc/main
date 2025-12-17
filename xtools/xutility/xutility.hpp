#pragma once

#include <string>
#include "xlog.h"

#define X_UNUSED_PARAMETER(x) \
    do {                      \
        (void)x;              \
    } while (0)

template <typename... Args>
std::string x_safe_format_(const char *file, int line, const char *function, fmt::format_string<> fmt_str, Args&&... args) {
    try {
        return fmt::format(fmt_str, std::forward<decltype(args)>(args)...);
    } catch (const std::exception& e) {
        xlog_err("error: {}", e.what());
        return "";
    } catch (...) {
        xlog_err("error");
        return "";
    }
}

#define x_safe_format(fmt, ...) x_safe_format_(__FILE__,__LINE__,__func__,fmt,##__VA_ARGS__)
