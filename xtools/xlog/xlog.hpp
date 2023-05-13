
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
#include <string>

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

void xlog(XLOG_LEVEL level, const char *format, ...);
void xlog_setmask(unsigned int mask);
void xlog_setoutput(const std::vector<FILE*> &fps);
std::string xlog_shortfilepath(const std::string &path);

#define XLOG_FILE__ xlog_shortfilepath(std::string(__FILE__)).c_str()

#define xlog_trc(format, ...) xlog(XLOG_LEVEL_TRACE, "[%s %d %s]" format, XLOG_FILE__,__LINE__,__FUNCTION__,##__VA_ARGS__)
#define xlog_dbg(format, ...) xlog(XLOG_LEVEL_DEBUG, "[%s %d %s]" format, XLOG_FILE__,__LINE__,__FUNCTION__,##__VA_ARGS__)
#define xlog_log(format, ...) xlog(XLOG_LEVEL_LOG, "[%s %d %s]" format, XLOG_FILE__,__LINE__,__FUNCTION__,##__VA_ARGS__)
#define xlog_inf(format, ...) xlog(XLOG_LEVEL_INFORMATION, "[%s %d %s]" format, XLOG_FILE__,__LINE__,__FUNCTION__,##__VA_ARGS__)
#define xlog_err(format, ...) xlog(XLOG_LEVEL_ERROR, "[%s %d %s]" format, XLOG_FILE__,__LINE__,__FUNCTION__,##__VA_ARGS__)
#define xlog_cri(format, ...) xlog(XLOG_LEVEL_CRITICAL, "[%s %d %s]" format, XLOG_FILE__,__LINE__,__FUNCTION__,##__VA_ARGS__)

