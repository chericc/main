#include "uart_prot.h"

#include <cstdlib>
#include <cstring>
#include <thread>

#include "xlog.h"

void on_request_cb(void const* in_data, size_t in_data_size,
    void *response_data, size_t *response_data_size)
{
    const char *response_header = "response for";

    // as example, we reply with reqeust content
    // only response when it's a request

    if ( (in_data_size > 0)
        && (strncmp((const char *)in_data, response_header, strlen(response_header)) != 0) ) {
    
        char buf_response[1024] = {};
        snprintf(buf_response, sizeof(buf_response), "%s : %.*s", response_header, (int)in_data_size, (const char *)in_data);

        size_t response_data_size_tmp = *response_data_size;
        int ret = snprintf((char*)response_data, response_data_size_tmp, "%s", buf_response);
        *response_data_size = ret;

        xlog_dbg("request: <%.*s>(%zu), response: <%.*s>(%d)\n", 
            (int)in_data_size, (const char *)in_data, in_data_size,
            ret, (const char *)response_data, ret);
    } else {
        xlog_dbg("not respond for response\n");
        *response_data_size = 0;
    }

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
                "switch [raw|msg]\n"
                "send [content: helloworld] [need response: 0|1] [timeout_ms: 1000]\n"
                "quit\n");
            char input_buf[128] = {};
            fgets(input_buf, sizeof(input_buf), stdin);
            char c1[64], c2[64], c3[64], c4[64], c5[64];
            int ret = sscanf(input_buf, "%s %s %s %s %s", c1, c2, c3, c4, c5);
            if (ret == 1) {
                if (strstr(c1, "quit")) {
                    break;
                } else {
                    xlog_err("unknown\n");
                }
            } else if (ret == 2) {
                if (strstr(c1, "switch")) {
                    UART_PROT_MODE mode = UART_PROT_MODE_NONE;
                    int ok = 0;
                    if (strstr(c2, "raw")) {
                        mode = UART_PROT_MODE_RAW;
                        ok = 1;
                    } else if (strstr(c2, "msg")) {
                        mode = UART_PROT_MODE_MSG;
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
                    const char *content = c2;
                    size_t content_len = strlen(content);
                    int need_response = atoi(c3);
                    int timeout_ms = atoi(c4);

                    char res_buf[256] = {};
                    size_t res_size = sizeof(res_buf);

                    if (!need_response) {
                        res_size = 0;
                    }

                    ret = uart_prot_send(prot, content, content_len, 
                        res_buf, &res_size, timeout_ms);
                    if (ret < 0) {
                        xlog_err("uart_prot_send failed\n");
                    } else {
                        xlog_dbg("resp: <%.*s>, size=%zu\n", (int)res_size, res_buf, res_size);
                    }
                } else {
                    xlog_err("unknown\n");
                }
            } else {
                xlog_err("unknown\n");
            }
        }

    } while (0);

    if (prot != uart_prot_handle_invalid) {
        uart_prot_deinit(prot);
        prot = uart_prot_handle_invalid;
    }

    return 0;
}