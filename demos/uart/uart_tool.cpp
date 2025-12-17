#include "uart_tool.h"

#include <cstdio>
#include <cctype>

#include "xlog.h"

// #define xlog_dbg printf
// #define xlog_err printf

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
            xlog_err("fmemopen failed");
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
        
        xlog_dbg("\n{} bytes in {}/{}: dump({}): \n{}\n", 
            len,
            func, line, 
            desc,
            dump_buf);
    } while (0);

    if (fp) {
        fclose(fp);
        fp = nullptr;
    }

    return ;
}

void uart_unescape(const char *src, char *dst, size_t dst_size)
{
    size_t dst_off = 0;

    while (*src) {
        if (dst_off >= dst_size) {
            break;
        }

        if (*src == '\\') {
            ++src;
            switch (*src) {
                case 'n': {
                    *(dst + dst_off) = '\n';
                    ++dst_off;
                    break;
                }
                case 'r': {
                    *(dst + dst_off) = '\r';
                    ++dst_off;
                    break;
                }
                default: {
                    break;
                }
            }
        } else {
            *(dst + dst_off) = *src;
            ++dst_off;
        }
        ++src;
    }
    if (dst_off < dst_size) {
        dst[dst_off] = 0;
    }
    return ;
}

