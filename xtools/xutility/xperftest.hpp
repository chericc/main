#pragma once

#include <chrono>
#include <string>

#include "xlog.hpp"

class XPeriodPrint {
   public:
    XPeriodPrint(const char* file, int line, const char* function,
                 XLOG_LEVEL severity, const char* description);
    ~XPeriodPrint();

   private:
    using Clock = std::chrono::steady_clock;
    using Timepoint = std::chrono::time_point<Clock>;

    Timepoint tp_;
    std::string file_;
    int line_ = 0;
    std::string function_;
    XLOG_LEVEL level_ = XLOG_LEVEL_BUTT;
    std::string desc_;
};

#define X_PERIOD_PRINT(level, desc, objname) \
    X_PERIOD_PRINT_##level(desc, objname)

#define X_PERIOD_PRINT_TRC(desc, objname)                                    \
    XPeriodPrint objname(__FILE__, __LINE__, __FUNCTION__, XLOG_LEVEL_TRACE, \
                         desc)
#define X_PERIOD_PRINT_DBG(desc, objname)                                    \
    XPeriodPrint objname(__FILE__, __LINE__, __FUNCTION__, XLOG_LEVEL_DEBUG, \
                         desc)
#define X_PERIOD_PRINT_LOG(desc, objname) \
    XPeriodPrint objname(__FILE__, __LINE__, __FUNCTION__, XLOG_LEVEL_LOG, desc)
#define X_PERIOD_PRINT_INF(desc, objname)                  \
    XPeriodPrint objname(__FILE__, __LINE__, __FUNCTION__, \
                         XLOG_LEVEL_INFORMATION, desc)
#define X_PERIOD_PRINT_ERR(desc, objname)                                    \
    XPeriodPrint objname(__FILE__, __LINE__, __FUNCTION__, XLOG_LEVEL_ERROR, \
                         desc)
#define X_PERIOD_PRINT_CRI(desc, objname)                  \
    XPeriodPrint objname(__FILE__, __LINE__, __FUNCTION__, \
                         XLOG_LEVEL_CRITICAL, desc)
