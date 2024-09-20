#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <stdlib.h>

static int gen_uuid(const char *token, char *output, int output_size)
{
    static unsigned int call_count = 0;
    struct timespec ts_now = {};
    clock_gettime(CLOCK_REALTIME, &ts_now);

    char ms[16] = { 0 };
    char timestamp[16] = { 0 };
    char call_count_str[16] = { 0 };

    snprintf(ms, sizeof(ms), "%03d", (int)ts_now.tv_nsec / 1000 / 1000);
    snprintf(timestamp, sizeof(timestamp), "%012d", (int)ts_now.tv_sec);
    snprintf(call_count_str, sizeof(call_count_str), "%03d", (int)call_count);

    snprintf(output, output_size, "%.3s-%.12s-%.3s-%.16s", ms, timestamp, call_count_str, token);

    ++call_count;
    call_count = call_count % 1000;
    return 0;
}

int main()
{
    char buf[128];
    gen_uuid("token", buf, sizeof(buf));
    printf("uuid: <%s>\n", buf);
    return 0;
}