#pragma once

#define logd(x, ...)                                                         \
    do {                                                                     \
        printf("[debug][%s %d %s]" x "\n", __FILE__, __LINE__, __FUNCTION__, \
               ##__VA_ARGS__);                                               \
    } while (0)

#define logi(x, ...)                                                         \
    do {                                                                     \
        printf("[info ][%s %d %s]" x "\n", __FILE__, __LINE__, __FUNCTION__, \
               ##__VA_ARGS__);                                               \
    } while (0)

#define loge(x, ...)                                                         \
    do {                                                                     \
        printf("[error][%s %d %s]" x "\n", __FILE__, __LINE__, __FUNCTION__, \
               ##__VA_ARGS__);                                               \
    } while (0)
