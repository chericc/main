#include "uart_prot.h"

#include <memory>
#include <chrono>
#include <mutex>
#include <array>

#include "uart_raw.h"
#include "uart_prot_helper.hpp"
#include "xlog.h"
#include "uart_tool.h"

using Lock = std::unique_lock<std::mutex>;

struct uart_prot_obj {
    uart_prot_param param = {};
    uart_raw_handle uart = uart_raw_handle_invalid;
    UART_PROT_MODE mode = UART_PROT_MODE_NONE;

    std::mutex mutex_prot;
    std::shared_ptr<UartProt> prot = nullptr;

    // 串行通信，每次只允许一个通信任务
    std::mutex mutex_call;
};

static void uart_prot_on_data_imp(void const* data, size_t size, void *user)
{
    auto obj = static_cast<struct uart_prot_obj*>(user);

    do {
        int ret = 0;

        if (!obj) {
            xlog_err("inner error, null obj\n");
            break;
        }

        Lock lock(obj->mutex_prot);
        if (!obj->prot) {
            xlog_err("inner error, null prot\n");
            break;
        }

        // feed uart data to prot
        ret = obj->prot->input(data, size);
        if (ret < 0) {
            xlog_err("input failed\n");
            break;
        }
    } while (0);
    
    return ;
}

static int uart_prot_deinit_imp(struct uart_prot_obj *obj)
{
    do {
        if (!obj) {
            xlog_war("null obj\n");
            break;
        }

        Lock lock(obj->mutex_prot);
        obj->prot.reset(); // explicit release
        lock.unlock();

        if (obj->uart != uart_raw_handle_invalid) {
            uart_raw_close(obj->uart);
            obj->uart = uart_raw_handle_invalid;
        }

        delete obj;
        obj = nullptr;
    } while (0);

    return 0;
}

uart_prot_handle uart_prot_init(const struct uart_prot_param *param)
{
    struct uart_prot_obj *obj = nullptr;

    bool error_flag = false;

    do {
        obj = new uart_prot_obj();

        obj->param = *param;

        uart_raw_param param_raw = {};
        param_raw.uart_dev_path = param->uart_dev_path;
        param_raw.baudrate = param->baudrate;
        param_raw.read_cb = uart_prot_on_data_imp;
        param_raw.user = obj;
        obj->uart = uart_raw_open(&param_raw);

        if (obj->uart == uart_raw_handle_invalid) {
            xlog_err("uart_raw_open failed\n");
            error_flag = true;
            break;
        }

        uart_raw_flush(obj->uart, UART_RAW_FLUSH_IN_OUT);
    } while (0);

    if (error_flag) {
        if (obj) {
            uart_prot_deinit_imp(obj);
            obj = nullptr;
        }
    }

    return static_cast<uart_prot_handle>(obj);
}

int uart_prot_deinit(uart_prot_handle handle)
{
    auto obj = static_cast<struct uart_prot_obj*>(handle);
    uart_prot_deinit_imp(obj);
    return 0;
}

int uart_prot_switch_mode(uart_prot_handle handle, UART_PROT_MODE mode)
{
    auto obj = static_cast<struct uart_prot_obj*>(handle);

    xlog_dbg("switch mode\n");

    bool error_flag = false;
    do {
        obj->mode = mode;

        Lock lock_call(obj->mutex_call);

        Lock lock(obj->mutex_prot);

        if ( (!obj->prot)
            || (obj->prot->type() != obj->mode)) {
            auto write_cb = [obj](void const* data, size_t size) {
                return uart_raw_write(obj->uart, data, size);
            };
            obj->prot = UartProt::produce(obj->mode, &obj->param, write_cb);
            if (!obj->prot) {
                xlog_err("switch failed\n");
            } else {
                xlog_dbg("switch to %s ok\n", uart_prot_mode_name(mode));
            }
        } else {
            xlog_dbg("mode same, no action\n");
        }
    } while (0);
    
    return error_flag ? -1 : 0;
}

int uart_prot_send(uart_prot_handle handle, 
        void const* request_data, size_t request_data_size, 
        void *response_data, size_t *response_data_size,
        int timeout_ms)
{
    xlog_dbg("send: %zu bytes\n", request_data_size);

    auto obj = static_cast<struct uart_prot_obj*>(handle);

    bool error_flag = false;
    
    do {
        int ret = 0;

        if (!obj) {
            xlog_err("null obj\n");
            error_flag = true;
            break;
        }

        Lock lock_send(obj->mutex_call);

        // obj->prot will not change in this call(lock_send protected)

        if (!obj->prot) {
            xlog_err("prot is null\n");
            error_flag = true;
            break;
        }

        // clear input and output for request
        ret = uart_raw_flush(obj->uart, UART_RAW_FLUSH_IN_OUT);
        if (ret < 0) {
            xlog_err("flush failed\n");
            error_flag = true;
            break;
        }

        ret = obj->prot->request(request_data, request_data_size, 
            response_data, response_data_size, 
            std::chrono::milliseconds(timeout_ms));
        if (ret < 0) {
            xlog_err("request failed\n");
            error_flag = true;
            break;
        }

        // uart_dump("request", (const uint8_t*)request_data, request_data_size);
        // if (response_data_size != nullptr) {
        //     uart_dump("response", (const uint8_t*)response_data, *response_data_size);
        // } else {
        //     APP_LOGW("no need response\n");
        // }
    } while (0);

    return error_flag ? -1 : 0;
}

const char *uart_prot_mode_name(UART_PROT_MODE mode)
{
    const char *name = "null";
    switch (mode) {
        case UART_PROT_MODE_NONE: {
            name = "none";
            break;
        }
        case UART_PROT_MODE_RAW: {
            name = "raw";
            break;
        }
        case UART_PROT_MODE_MSG: {
            name = "msg";
            break;
        }
        default: {
            break;
        }
    }
    return name;
}