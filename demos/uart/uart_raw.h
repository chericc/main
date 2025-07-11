#ifndef __UART_RAW_H__
#define __UART_RAW_H__

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define uart_raw_handle_invalid NULL
typedef void* uart_raw_handle;

typedef void (*uart_raw_on_data_cb)(void const* data, size_t size, void *user);

struct uart_raw_param {
    const char *uart_dev_path;
    int baudrate; // eg, 9600
    uart_raw_on_data_cb read_cb;
    void *user;
};

uart_raw_handle uart_raw_open(struct uart_raw_param const* param);
int uart_raw_close(uart_raw_handle handle);
enum UART_RAW_FLUSH_TARGET {
    UART_RAW_FLUSH_NONE = 0,
    UART_RAW_FLUSH_IN = 1,
    UART_RAW_FLUSH_OUT = 2,
    UART_RAW_FLUSH_IN_OUT = 3,
};
int uart_raw_flush(uart_raw_handle handle, enum UART_RAW_FLUSH_TARGET target);
int uart_raw_write(uart_raw_handle handle, void const* data, size_t size);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __UART_RAW_H__