#pragma once

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


#define MY_LOG(fmt, ...)\
	do { \
		if (g_enable_log) { \
			printf(fmt, ##__VA_ARGS__); \
		} \
	} while (0)


extern int g_enable_log;

int mwu_enable_log(int enable);

#ifdef __cplusplus
}
#endif // __cplusplus