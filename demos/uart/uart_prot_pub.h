#ifndef __UART_PROT_PUB_H__
#define __UART_PROT_PUB_H__

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

enum UART_PROT_MODE {
    UART_PROT_MODE_NONE,
    UART_PROT_MODE_RAW,
    UART_PROT_MODE_MSG,
};

typedef void (*uart_prot_on_data_cb)(void const* in_data, size_t in_data_size,
    void *response_data, size_t *response_data_size);

struct uart_prot_param {
    const char *uart_dev_path;
    int baudrate; // eg, 9600
    uart_prot_on_data_cb on_reqeust_cb;
};

#define UART_PROT_BUF_MAX_SIZE      (16 * 1024)

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __UART_PROT_PUB_H__