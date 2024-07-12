#include "demo_util.h"

#include <stdio.h>

void hex_dump_a(uint8_t const*buf, int len, const char *func, int line)
{
    char outputbuf[512];
    outputbuf[0] = 0;
    FILE* fp = NULL;

    do {
        if (len <= 0) {
            APP_LOGE("empty buf\n");
            break;
        } 

        fp = fmemopen(outputbuf, sizeof(outputbuf), "w");
        if (!fp) {
            APP_LOGE("fmemopen failed\n");
            break;
        }

        fprintf(fp, "[%d]", len);

        int dump_len = (len > 16 ? 16 : len);

        int bytes_write = 0;
        for (int i = 0; i < dump_len; ++i) {
            bytes_write = fprintf(fp, "%02x ", buf[i]);
            if (bytes_write <= 0) {
                break;
            }
        }

        if (dump_len != len) {
            fprintf(fp, "...");
        }
        fflush(fp);

        APP_LOGE("%s(in: %s,%d)\n", outputbuf, func, line);
    } while (0);
    
    if (fp) {
        fclose(fp);
        fp = NULL;
    }
}
