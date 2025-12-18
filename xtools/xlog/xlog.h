
/**
 * @file xlog.h
 * @date 2021-12-27
 */

#pragma once

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#include "spdlog/spdlog.h"

spdlog::logger *xlog_default_logger_raw();
#define xlog_trc(...) SPDLOG_LOGGER_TRACE(xlog_default_logger_raw(), __VA_ARGS__)
#define xlog_dbg(...) SPDLOG_LOGGER_DEBUG(xlog_default_logger_raw(), __VA_ARGS__)
#define xlog_inf(...) SPDLOG_LOGGER_INFO(xlog_default_logger_raw(), __VA_ARGS__)
#define xlog_war(...) SPDLOG_LOGGER_WARN(xlog_default_logger_raw(), __VA_ARGS__)
#define xlog_err(...) SPDLOG_LOGGER_ERROR(xlog_default_logger_raw(), __VA_ARGS__)
#define xlog_cri(...) SPDLOG_LOGGER_CRITICAL(xlog_default_logger_raw(), __VA_ARGS__)
