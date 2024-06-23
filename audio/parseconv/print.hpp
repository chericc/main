#pragma once

#include <stdio.h>

#define LOGD(...)                                                      \
    do {                                                               \
        printf("[debug][%s %d %s]", __FILE__, __LINE__, __FUNCTION__); \
        printf(__VA_ARGS__);                                           \
        printf("\n");                                                  \
    } while (0)
#define LOGI(...)                                                     \
    do {                                                              \
        printf("[info][%s %d %s]", __FILE__, __LINE__, __FUNCTION__); \
        printf(__VA_ARGS__);                                          \
        printf("\n");                                                 \
    } while (0)
#define LOGE(...)                                                      \
    do {                                                               \
        printf("[error][%s %d %s]", __FILE__, __LINE__, __FUNCTION__); \
        printf(__VA_ARGS__);                                           \
        printf("\n");                                                  \
    } while (0)
