#include "xlog.hpp"

#define __STDC_WANT_LIB_EXT2__ 1

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <mutex>
#include <vector>
#include <algorithm>
#include <chrono>
#include <sstream>
#include <iomanip>

static int s_log_mask = 0xffffffff;
static std::vector<FILE *> s_fps;
static std::mutex s_call_mutex;

static const char *xlog_getlevel (XLOG_LEVEL level)
{
    int i = 0;

    /* Make sure this two arrays has the same number of items. */

    const char *array_name[XLOG_LEVEL_BUTT] = {
        "trc",
        "dbg",
        "log",
        "inf",
        "err",
        "cri",
    };
    const XLOG_LEVEL array_level[XLOG_LEVEL_BUTT] = {
        XLOG_LEVEL_TRACE,
        XLOG_LEVEL_DEBUG,
        XLOG_LEVEL_LOG,
        XLOG_LEVEL_INFORMATION,
        XLOG_LEVEL_ERROR,
        XLOG_LEVEL_CRITICAL,
    };

    for (i = 0; i < XLOG_LEVEL_BUTT; ++i)
    {
        if (array_level[i] == level)
        {
            return array_name[i];
        }
    }

    return "unknown";
}

static std::string now_str()
{
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    auto msecs = std::chrono::time_point_cast<std::chrono::milliseconds>(now).time_since_epoch().count() % 1000;
    
    std::stringstream ss;
    ss << now_time << ":" << std::setw(3) << std::setfill('0') << msecs;
    return ss.str();
}

void xlog_setmask(unsigned int mask)
{
    std::unique_lock<std::mutex> lock(s_call_mutex);
    s_log_mask = mask;
    return ;
}

void xlog_setoutput(const std::vector<FILE*> &fps)
{
    std::unique_lock<std::mutex> lock(s_call_mutex);
    
    // close all output
    for (auto &it : s_fps)
    {
        if (it != stdin
            && it != stdout
            && it != stderr)
        {
            fclose(it);
        }
    }
    s_fps.clear();
    s_fps.shrink_to_fit();

    for (auto const& it : fps)
    {
        if (!it)
        {
            continue;
        }
        s_fps.push_back(it);
    }

    return ;
}

void xlog(XLOG_LEVEL level, const char *format, ...)
{
    va_list ap;

    std::unique_lock<std::mutex> lock(s_call_mutex);

    if (! (level & s_log_mask))
    {
        return ;
    }

    for (auto &it : s_fps)
    {
        fprintf(it, "[%s]", now_str().c_str());
        fprintf(it, "[%s]", xlog_getlevel(level));
        va_start (ap, format);
        vfprintf(it, format, ap);
        va_end (ap);
        fprintf(it, "\n");
        fflush(it);
    }

    return ;
}


std::string xlog_shortfilepath(const std::string &path)
{
    char token[] = { '/', '\\'};
    std::size_t token_pos = 0;
    bool found = false;
    for (auto const& r : token)
    {
        std::size_t pos = path.rfind(r);
        if (pos != std::string::npos && pos >= token_pos)
        {
            found = true;
            token_pos = pos;
        }
    }
    
    if (!found)
    {
        return path;
    }

    return std::string(path, token_pos + 1);

}