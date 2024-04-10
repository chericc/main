
/**
 * @file xlog.h
 * @author xlog's author
 * @brief 
 * @version x.x
 * @date 2021-12-27
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#pragma once

#include <stdio.h>
#include <vector>
#include <ostream>

typedef enum XLOG_LEVEL
{
    XLOG_LEVEL_TRACE        = 1 << 0,
    XLOG_LEVEL_DEBUG        = 1 << 1,
    XLOG_LEVEL_LOG          = 1 << 2,
    XLOG_LEVEL_INFORMATION  = 1 << 3,
    XLOG_LEVEL_ERROR        = 1 << 4,
    XLOG_LEVEL_CRITICAL     = 1 << 5,
    XLOG_LEVEL_BUTT         = 1 << 6,
} XLOG_LEVEL;

#define XLOG_MASK_LOG (~((unsigned int)XLOG_LEVEL_DEBUG | XLOG_LEVEL_TRACE))
#define XLOG_MASK_ERR (~((unsigned int)XLOG_LEVEL_INFORMATION | XLOG_LEVEL_LOG \
                        | XLOG_LEVEL_DEBUG | XLOG_LEVEL_TRACE))


#if defined(X_PLATFORM_GNU)
#define XLOG_VAR_CHECK __attribute__((format(printf,2,3)))
#define XLOG_VAR_CHECK_EX __attribute__((format(printf,5,6)))
#else 
#define XLOG_VAR_CHECK 
#define XLOG_VAR_CHECK_EX
#endif

void xlog(XLOG_LEVEL level, const char *format, ...) XLOG_VAR_CHECK;
void xlog_ex(XLOG_LEVEL level, const char *file, int line, const char *func, const char *format, ...) XLOG_VAR_CHECK_EX;
void xlog_setmask(unsigned int mask);
void xlog_setoutput(const std::vector<FILE*> &fps);

struct LogMessageData;

class LogMessage {
public:
    LogMessage(const char* file, int line, const char *function, XLOG_LEVEL severity);
    ~LogMessage();
    std::ostream& stream();
    LogMessageData* data_;
private:
    LogMessage(const LogMessage&);
};

// xlog_inf("This is amazing");
#define xlog_trc(...) xlog_ex(XLOG_LEVEL_TRACE, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define xlog_dbg(...) xlog_ex(XLOG_LEVEL_DEBUG, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define xlog_log(...) xlog_ex(XLOG_LEVEL_LOG, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define xlog_inf(...) xlog_ex(XLOG_LEVEL_INFORMATION, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define xlog_err(...) xlog_ex(XLOG_LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define xlog_cri(...) xlog_ex(XLOG_LEVEL_CRITICAL, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)

// XLOG(INFO) --> XLOGINFO
// XLOG(INFO) << "This is amazing"
#define XLOG(level) XLOG##level.stream()

#define XLOGTRC \
    LogMessage(__FILE__,__LINE__,__FUNCTION__,XLOG_LEVEL_TRACE)
#define XLOGDBG \
    LogMessage(__FILE__,__LINE__,__FUNCTION__,XLOG_LEVEL_DEBUG)
#define XLOGLOG \
    LogMessage(__FILE__,__LINE__,__FUNCTION__,XLOG_LEVEL_LOG)
#define XLOGINF \
    LogMessage(__FILE__,__LINE__,__FUNCTION__,XLOG_LEVEL_INFORMATION)
#define XLOGERR \
    LogMessage(__FILE__,__LINE__,__FUNCTION__,XLOG_LEVEL_ERROR)
#define XLOGCRI \
    LogMessage(__FILE__,__LINE__,__FUNCTION__,XLOG_LEVEL_CRITICAL)
