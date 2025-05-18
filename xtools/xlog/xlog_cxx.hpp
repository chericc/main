
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
#include <ostream>
#include <vector>

#include "xlog.h"

struct XLogMessageData;

class XLogMessage {
   public:
    XLogMessage(const char* file, int line, const char* function,
                XLOG_LEVEL severity);
    ~XLogMessage();
    std::ostream& stream();
    XLogMessageData* data_;

   private:
    XLogMessage(const XLogMessage&);
};

// XLOG(INFO) --> XLOGINFO
// XLOG(INFO) << "This is amazing"
#define XLOG(level) XLOG##level.stream()

#define XLOGTRC XLogMessage(__FILE__, __LINE__, __FUNCTION__, XLOG_LEVEL_TRACE)
#define XLOGDBG XLogMessage(__FILE__, __LINE__, __FUNCTION__, XLOG_LEVEL_DEBUG)
#define XLOGLOG XLogMessage(__FILE__, __LINE__, __FUNCTION__, XLOG_LEVEL_LOG)
#define XLOGINF \
    XLogMessage(__FILE__, __LINE__, __FUNCTION__, XLOG_LEVEL_INFORMATION)
#define XLOGWAR \
    XLogMessage(__FILE__, __LINE__, __FUNCTION__, XLOG_LEVEL_WARNING)
#define XLOGERR XLogMessage(__FILE__, __LINE__, __FUNCTION__, XLOG_LEVEL_ERROR)
#define XLOGCRI \
    XLogMessage(__FILE__, __LINE__, __FUNCTION__, XLOG_LEVEL_CRITICAL)
