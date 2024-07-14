#ifndef UTIL_H__
#define UTIL_H__

#include <stdint.h>

#define APP_LOG(t,...) \
    do { \
        printf("[%s %d] [" #t "] ", __func__, __LINE__); \
        printf(__VA_ARGS__); \
    } while (0);

#define APP_LOGW(...) APP_LOG(W,__VA_ARGS__)
#define APP_LOGE(...) APP_LOG(E,__VA_ARGS__)

void hex_dump_a(const char *desc, uint8_t const*buf, int len, const char *func, int line);
#define hex_dump(desc,buf,len) hex_dump_a(desc,buf,len,__func__,__LINE__)

#endif // UTIL_H__
