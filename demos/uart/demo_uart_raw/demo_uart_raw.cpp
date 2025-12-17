#include <cstdlib>
#include <cstring>

#include "uart_raw.h"

#include "xlog.h"

static void uart_raw_read_on_data_imp(void const* data, size_t size, void *user)
{
    xlog_dbg("on data: user={}, <{2:.{1}}>\n", user, (int)size, (const char *)data);
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        xlog_dbg("usage: {} [uart_dev_path] [boudrate]\n", argv[0]);
        return 1;
    }

    struct uart_raw_param param = {};
    param.uart_dev_path = argv[1];
    param.baudrate = atoi(argv[2]);
    param.read_cb = uart_raw_read_on_data_imp;
    param.user = (void*)0x12;

    uart_raw_handle uart = uart_raw_open(&param);

    if (uart == uart_raw_handle_invalid) {
        xlog_err("open failed\n");
        return -1;
    }

    while (1) {
        char buf[32] = {};
        fgets(buf, sizeof(buf), stdin);

        char *end = strstr(buf, "\n");
        if (end) {
            *end = 0;
        }

        if (strstr(buf, "exit")) {
            xlog_dbg("break\n");
            break;
        }

        xlog_dbg("input: <{}>\n", buf);
        char buf_with_tail[64] = {};
        // snprintf(buf_with_tail, sizeof(buf_with_tail), "%s\n", buf);
        snprintf(buf_with_tail, sizeof(buf_with_tail), "%s", buf);
        uart_raw_write(uart, buf_with_tail, strlen(buf_with_tail));
    }

    uart_raw_close(uart);

    return 0;
}
