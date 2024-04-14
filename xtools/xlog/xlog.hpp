
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

#include <cstdio>
#include <vector>
#include <ostream>

using XLOG_LEVEL = enum XLOG_LEVEL
{
    XLOG_LEVEL_TRACE        = 1 << 0,
    XLOG_LEVEL_DEBUG        = 1 << 1,
    XLOG_LEVEL_LOG          = 1 << 2,
    XLOG_LEVEL_INFORMATION  = 1 << 3,
    XLOG_LEVEL_ERROR        = 1 << 4,
    XLOG_LEVEL_CRITICAL     = 1 << 5,
    XLOG_LEVEL_BUTT         = 1 << 6,
};

#define XLOG_ALLOW_ALL (~0U)
#define XLOG_ALLOW_DBG (~((unsigned int)XLOG_LEVEL_TRACE))
#define XLOG_ALLOW_LOG (~((unsigned int)XLOG_LEVEL_TRACE | XLOG_LEVEL_DEBUG))
#define XLOG_ALLOW_INF (~((unsigned int)XLOG_LEVEL_TRACE | XLOG_LEVEL_DEBUG \
                        | XLOG_LEVEL_LOG))
#define XLOG_ALLOW_ERR (~((unsigned int)XLOG_LEVEL_TRACE | XLOG_LEVEL_DEBUG \
                        | XLOG_LEVEL_LOG | XLOG_LEVEL_INFORMATION))


#if defined(X_PLATFORM_GNU)
#define XLOG_VAR_CHECK __attribute__((format(printf,2,3)))
#define XLOG_VAR_CHECK_EX __attribute__((format(printf,5,6)))
#else 
#define XLOG_VAR_CHECK 
#define XLOG_VAR_CHECK_EX
#endif

void xlog(XLOG_LEVEL level, const char *file, int line, const char *func, const char *format, ...) XLOG_VAR_CHECK_EX;

void xlog_setmask(unsigned int mask);
unsigned int xlog_getmask();

void xlog_setoutput(const std::vector<FILE*> &fps);

struct XLogMessageData;

class XLogMessage {
public:
    XLogMessage(const char* file, int line, const char *function, XLOG_LEVEL severity);
    ~XLogMessage();
    std::ostream& stream();
    XLogMessageData* data_;
private:
    XLogMessage(const XLogMessage&);
};

// xlog_inf("This is amazing");
#define xlog_trc(...) xlog(XLOG_LEVEL_TRACE, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define xlog_dbg(...) xlog(XLOG_LEVEL_DEBUG, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define xlog_log(...) xlog(XLOG_LEVEL_LOG, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define xlog_inf(...) xlog(XLOG_LEVEL_INFORMATION, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define xlog_err(...) xlog(XLOG_LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define xlog_cri(...) xlog(XLOG_LEVEL_CRITICAL, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)

// XLOG(INFO) --> XLOGINFO
// XLOG(INFO) << "This is amazing"
#define XLOG(level) XLOG##level.stream()

#define XLOGTRC \
    XLogMessage(__FILE__,__LINE__,__FUNCTION__,XLOG_LEVEL_TRACE)
#define XLOGDBG \
    XLogMessage(__FILE__,__LINE__,__FUNCTION__,XLOG_LEVEL_DEBUG)
#define XLOGLOG \
    XLogMessage(__FILE__,__LINE__,__FUNCTION__,XLOG_LEVEL_LOG)
#define XLOGINF \
    XLogMessage(__FILE__,__LINE__,__FUNCTION__,XLOG_LEVEL_INFORMATION)
#define XLOGERR \
    XLogMessage(__FILE__,__LINE__,__FUNCTION__,XLOG_LEVEL_ERROR)
#define XLOGCRI \
    XLogMessage(__FILE__,__LINE__,__FUNCTION__,XLOG_LEVEL_CRITICAL)
