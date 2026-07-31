#pragma once

/**
 * Simple header-only logging utility.
 *
 * Provides: xlog_dbg(), xlog_warn(), xlog_err()
 * Format: uses {} placeholders (like fmtlib / Python str.format)
 *
 * Example:
 *   xlog_dbg("value = {}, name = {}", 42, "hello");
 */

#include <chrono>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>

namespace detail {

/// Convert any printable type to string via operator<<.
template <typename T>
std::string to_string(T&& val) {
    std::ostringstream os;
    os << std::forward<T>(val);
    return os.str();
}

// Base case: no more args, flush remaining literal after last {}.
inline void substitute(std::string& /*out*/, std::string::size_type /*pos*/) {}

/// Recursively replace the first {} with the next argument, then continue.
template <typename T, typename... Args>
void substitute(std::string& out, std::string::size_type pos, T&& head,
                Args&&... tail) {
    pos = out.find("{}", pos);
    if (pos == std::string::npos) {
        // No more placeholders; append remaining args as a suffix.
        out += " [unused: ";
        out += to_string(std::forward<T>(head));
        auto append_remaining = [&](auto const&... rest) {
            ((out += ", ", out += to_string(rest)), ...);
        };
        append_remaining(std::forward<Args>(tail)...);
        out += "]";
        return;
    }
    auto replacement = to_string(std::forward<T>(head));
    out.replace(pos, 2, replacement);
    substitute(out, pos + replacement.size(), std::forward<Args>(tail)...);
}

/// Build the final formatted string.
template <typename... Args>
std::string format(std::string_view fmt, Args&&... args) {
    std::string out(fmt);
    substitute(out, 0, std::forward<Args>(args)...);
    return out;
}

/// Prefix with current timestamp.
inline std::string timestamp() {
    auto now = std::chrono::system_clock::now();
    auto tt = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                  now.time_since_epoch()) %
              1000;

    std::tm tm_buf;
    localtime_r(&tt, &tm_buf);

    std::ostringstream os;
    os << "[" << std::put_time(&tm_buf, "%H:%M:%S") << "." << std::setfill('0')
       << std::setw(3) << ms.count() << "]";
    return os.str();
}

}  // namespace detail

// ------------------------------------------------------------------
// Public API
// ------------------------------------------------------------------

template <typename... Args>
void xlog_dbg(std::string_view fmt, Args&&... args) {
    std::cerr << detail::timestamp() << " [DBG] "
              << detail::format(fmt, std::forward<Args>(args)...) << std::endl;
}

template <typename... Args>
void xlog_warn(std::string_view fmt, Args&&... args) {
    std::cerr << detail::timestamp() << " [WRN] "
              << detail::format(fmt, std::forward<Args>(args)...) << std::endl;
}

template <typename... Args>
void xlog_err(std::string_view fmt, Args&&... args) {
    std::cerr << detail::timestamp() << " [ERR] "
              << detail::format(fmt, std::forward<Args>(args)...) << std::endl;
}
