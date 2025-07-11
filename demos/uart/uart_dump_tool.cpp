#include "uart_dump_tool.h"

#include <cstdio>
#include <cctype>

#include "xlog.h"

void uart_dump__(const char *desc, uint8_t const*buf, int len, 
    const char *file, const char *func, int line)
{
    /* 
    In a format like xxd:

    ...
    00000000: 7365 7428 5344 4b5f 544f 4f4c 4348 4149  set(SDK_TOOLCHAI
    00000010: 4e5f 5041 5448 2022 2f6f 7074 2f6d 6970  N_PATH "/opt/mip
    ...
    */

    FILE *fp = nullptr;

    do {
        char dump_buf[512] = {};
        fp = fmemopen(dump_buf, sizeof(dump_buf), "w");
        if (!fp) {
            xlog_err("fmemopen failed\n");
            break;
        }
        
        if (len > 0) {
            for (int line = 0; line < ((len - 1) / 16 + 1); ++line) {
                int begin = line * 16 + 0;
                int end = line * 16 + 16;

                fprintf(fp, "%04x: ", begin);

                for (int k = begin; k < end; ++k) {
                    if (k < len) {
                        if (k % 2) {
                            fprintf(fp, "%02x ", buf[k]);
                        } else {
                            fprintf(fp, "%02x", buf[k]);
                        }
                    } else {
                        if (k % 2) {
                            fprintf(fp, "   ");
                        } else {
                            fprintf(fp, "  ");
                        }
                    }
                }
                for (int k = begin; k < end && k < len; ++k) {
                    if (isprint(buf[k])) {
                        fprintf(fp, "%c", buf[k]);
                    } else {
                        fprintf(fp, "%c", '.');
                    }
                }

                fprintf(fp, "\n");
            }
        }

        fflush(fp);
        
        xlog_dbg("\n%s %s/%d dump: %d bytes\n%s\n", 
            desc,
            func, line, 
            len,
            dump_buf);
    } while (0);

    if (fp) {
        fclose(fp);
        fp = nullptr;
    }

    return ;
}