#include "xlog.h"

#include "spdlog/sinks/stdout_color_sinks.h"

namespace {
}

#if false
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <chrono>
#include <cstring>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <thread>
#include <vector>

static unsigned int s_log_mask = 0xffffffff;
static std::vector<FILE*> s_fps = {stdout};
static std::mutex s_call_mutex;

const char* xlog_basename(const char* filepath) {
    const char* base = strrchr(filepath, '/');
    if (!base) base = strrchr(filepath, '\\');
    return base ? (base + 1) : filepath;
}

static const char* xlog_getlevel(XLOG_LEVEL level) {
    int i = 0;

    /* Make sure this two arrays has the same number of items. */

    const char* array_name[] = {"trc", "dbg", "log", "inf",
                                "war", "err", "cri"};
    const XLOG_LEVEL array_level[] = {
        XLOG_LEVEL_TRACE,       XLOG_LEVEL_DEBUG,   XLOG_LEVEL_LOG,
        XLOG_LEVEL_INFORMATION, XLOG_LEVEL_WARNING, XLOG_LEVEL_ERROR,
        XLOG_LEVEL_CRITICAL,
    };

    static_assert(sizeof(array_name) / sizeof(array_name[0]) ==
                      sizeof(array_level) / sizeof(array_level[0]),
                  "size error");

    for (i = 0; i < XLOG_LEVEL_BUTT; ++i) {
        if (array_level[i] == level) {
            return array_name[i];
        }
    }

    return "unknown";
}

static std::string now_str() {
    auto now = std::chrono::system_clock::now();
    auto secs = std::chrono::time_point_cast<std::chrono::seconds>(now)
                    .time_since_epoch()
                    .count();
    auto msecs = std::chrono::time_point_cast<std::chrono::milliseconds>(now)
                     .time_since_epoch()
                     .count() %
                 1000;

    std::stringstream ss;
    ss << secs << ":" << std::setw(3) << std::setfill('0') << msecs;
    return ss.str();
}

static std::string trd_str() {
    std::stringstream ss;
    std::thread::id tid = std::this_thread::get_id();
    ss << tid;
    return ss.str();
}

void xlog_setmask(unsigned int mask) {
    std::unique_lock<std::mutex> lock(s_call_mutex);
    s_log_mask = mask;
    return;
}

unsigned int xlog_getmask() {
    std::unique_lock<std::mutex> lock(s_call_mutex);
    return s_log_mask;
}

void xlog_setoutput(FILE *fp[], size_t fp_num) {
    std::unique_lock<std::mutex> lock(s_call_mutex);

    // close all output
    for (auto& it : s_fps) {
        if (it != stdin && it != stdout && it != stderr) {
            fclose(it);
        }
    }
    s_fps.clear();
    s_fps.shrink_to_fit();

    for (size_t i = 0; i < fp_num; ++i) {
        auto ref = fp[i];
        if (!ref) {
            continue;
        }
        s_fps.push_back(ref);
    }

    return;
}

void xlog(XLOG_LEVEL level, const char* file, int line, const char* func,
          const char* format, ...) {
    va_list ap;

    if (!(level & s_log_mask)) {
        return;
    }

    std::unique_lock<std::mutex> lock(s_call_mutex);

    for (auto& it : s_fps) {
        fprintf(it, "[%s]", now_str().c_str());
        fprintf(it, "[%s]", trd_str().c_str());
        fprintf(it, "[%s]", xlog_getlevel(level));
        fprintf(it, "[%s %d %s] ", xlog_basename(file), line, func);
        va_start(ap, format);
        vfprintf(it, format, ap);
        va_end(ap);
        size_t format_len = strlen(format);
        int has_new_line = !!(format_len && format[format_len - 1] == '\n');
        if (!has_new_line) {
            fprintf(it, "\n");
        }
        fflush(it);
    }

    return;
}


#endif 


spdlog::logger *xlog_default_logger_raw()
{
    static std::shared_ptr<spdlog::logger> logger;
    static std::once_flag once;
    std::call_once(once, [](){
        auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        consoleSink->set_level(spdlog::level::debug);
        consoleSink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] %n %^%l%$ [%s:%#:%!] %v");
        consoleSink->set_level(spdlog::level::debug);
        logger = std::make_shared<spdlog::logger>("main", consoleSink);
        logger->set_level(spdlog::level::debug);
    });
    return logger.get();
}