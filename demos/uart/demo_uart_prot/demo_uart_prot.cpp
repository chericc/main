#include "uart_prot.h"

#include <cstdlib>
#include <cstring>
#include <thread>

#include "xlog.h"

void on_request_cb(void const* in_data, size_t in_data_size,
    void *response_data, size_t *response_data_size)
{
    // as example, we reply with reqeust content
    if (response_data && response_data_size && *response_data_size > 0) {
        int ret = snprintf((char*)response_data, *response_data_size, "%.*s", (int)in_data_size, (const char *)in_data);
        *response_data_size = ret;
    }

    xlog_dbg("request: <%.*s>, response: same\n", 
        (int)in_data_size, (const char *)in_data);

    return ;
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        xlog_dbg("usage: %s [uart_dev_path] [boudrate]\n", argv[0]);
        return 1;
    }

    uart_prot_handle prot = uart_prot_handle_invalid;

    do {
        struct uart_prot_param param = {};
        param.uart_dev_path = argv[1];
        param.baudrate = atoi(argv[2]);
        param.on_reqeust_cb = on_request_cb;
        prot = uart_prot_init(&param);
        if (prot == uart_prot_handle_invalid) {
            xlog_err("uart_prot_init failed\n");
            break;
        }

        uart_prot_switch_mode(prot, UART_PROT_MODE_RAW);

        std::this_thread::sleep_for(std::chrono::seconds(1));

        while (1) {

            xlog_dbg("\n"
                "choice: \n"
                "switch raw|prot\n"
                "send [content: helloworld] [need response: 0|1] [timeout_ms: 1000]\n"
                "quit\n");
            char input_buf[128] = {};
            fgets(input_buf, sizeof(input_buf), stdin);
            char c1[64], c2[64], c3[64], c4[64];
            int ret = sscanf(input_buf, "%s %s %s %s", c1, c2, c3, c4);
            if (ret == 1) {
                if (strstr(c1, "quit")) {
                    break;
                }
            } else if (ret == 2) {
                if (strstr(c1, "switch")) {
                    uart_prot_mode mode = UART_PROT_MODE_NONE;
                    int ok = 0;
                    if (strstr(c2, "raw")) {
                        mode = UART_PROT_MODE_RAW;
                        ok = 1;
                    } else if (strstr(c2, "prot")) {
                        mode = UART_PROT_MODE_RAW;
                        ok = 1;
                    }
                    if (ok) {
                        ret = uart_prot_switch_mode(prot, mode);
                        if (ret < 0) {
                            xlog_err("uart_prot_switch_mode failed\n");
                        }
                    }
                }
            } else if (ret == 4) {
                if (strstr(c1, "send")) {
                    int timeout_ms = atoi(c4);
                    char res_buf[256] = {};
                    size_t res_size = sizeof(res_buf);
                    ret = uart_prot_send(prot, c2, strlen(c2), res_buf, &res_size, timeout_ms);
                    if (ret < 0) {
                        xlog_err("uart_prot_send failed\n");
                    } else {
                        xlog_dbg("resp: <%.*s>, size=%zu\n", (int)res_size, res_buf, res_size);
                    }
                }
            }
        }

    } while (0);

    if (prot != uart_prot_handle_invalid) {
        uart_prot_deinit(prot);
        prot = uart_prot_handle_invalid;
    }

    return 0;
}