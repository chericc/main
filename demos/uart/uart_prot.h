#ifndef __UART_PROT_H__
#define __UART_PROT_H__

#include <stdint.h>
#include <stddef.h>

#include "uart_prot_pub.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define uart_prot_handle_invalid NULL
typedef void* uart_prot_handle;

uart_prot_handle uart_prot_init(const struct uart_prot_param *param);
int uart_prot_deinit(uart_prot_handle handle);

int uart_prot_switch_mode(uart_prot_handle handle, UART_PROT_MODE mode);
int uart_prot_send(uart_prot_handle handle, 
        void const* out_data, size_t out_data_size, 
        void *in_data, size_t *in_data_size,
        int timeout_ms);

const char *uart_prot_mode_name(UART_PROT_MODE mode);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __UART_PROT_H__
