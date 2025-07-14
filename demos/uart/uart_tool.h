#ifndef __UART_DUMP_TOOL_H__
#define __UART_DUMP_TOOL_H__

#include <stdint.h>
#include <stddef.h>

// dump
void uart_dump__(const char *desc, uint8_t const*buf, int len, const char *file, const char *func, int line);
#define uart_dump(desc,buf,len) uart_dump__(desc,buf,len,__FILE__,__func__,__LINE__)

// escape / un-escape
void uart_unescape(const char *src, char *dst, size_t dst_size);

#endif // __UART_DUMP_TOOL_H__
