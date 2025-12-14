
/**
 * @file xlog.h
 * @date 2021-12-27
 */

#pragma once

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG

#include "spdlog/spdlog.h"

#define xlog_trc(...) SPDLOG_TRACE(__VA_ARGS__)
#define xlog_dbg(...) SPDLOG_DEBUG(__VA_ARGS__)
#define xlog_inf(...) SPDLOG_INFO(__VA_ARGS__)
#define xlog_war(...) SPDLOG_WARN(__VA_ARGS__)
#define xlog_err(...) SPDLOG_ERROR(__VA_ARGS__)
#define xlog_cri(...) SPDLOG_CRITICAL(__VA_ARGS__)
