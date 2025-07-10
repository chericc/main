#include "uart_prot.h"

#include <memory>
#include <chrono>
#include <mutex>
#include <array>

#include "uart_raw.h"
#include "uart_prot_helper.hpp"
#include "xlog.h"

using Lock = std::unique_lock<std::recursive_mutex>;

struct uart_prot_obj {
    uart_prot_param param = {};
    uart_raw_handle uart = uart_raw_handle_invalid;
    uart_prot_mode mode = UART_PROT_MODE_NONE;

    std::recursive_mutex mutex_prot;
    std::shared_ptr<UartProt> prot = nullptr;
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

        obj->prot.reset(); // explicit release

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
        }
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

int uart_prot_switch_mode(uart_prot_handle handle, uart_prot_mode mode)
{
    auto obj = static_cast<struct uart_prot_obj*>(handle);

    xlog_dbg("switch mode: %s\n", uart_prot_mode_name(mode));

    bool error_flag = false;
    do {
        obj->mode = mode;

        Lock lock(obj->mutex_prot);
        if ( (!obj->prot)
            || (obj->prot->type() != obj->mode)) {
            auto write_cb = [obj](void const* data, size_t size) {
                return uart_raw_write(obj->uart, data, size);
            };
            obj->prot = UartProt::produce(obj->mode, &obj->param, write_cb);
        }
        if (!obj->prot) {
            xlog_err("null prot\n");
            error_flag = true;
        } else {
            xlog_dbg("mode switch suc\n");
        }
    } while (0);
    
    return error_flag ? -1 : 0;
}

int uart_prot_send(uart_prot_handle handle, 
        void const* out_data, size_t out_data_size, 
        void *in_data, size_t *in_data_size,
        int timeout_ms)
{
    auto obj = static_cast<struct uart_prot_obj*>(handle);

    bool error_flag = false;
    
    do {
        int ret = 0;

        if (!obj) {
            xlog_err("null obj\n");
            error_flag = true;
            break;
        }

        Lock lock(obj->mutex_prot);

        if (!obj->prot) {
            xlog_err("prot is null\n");
            error_flag = true;
            break;
        }

        ret = obj->prot->request(out_data, out_data_size, 
            in_data, in_data_size, 
            std::chrono::milliseconds(timeout_ms));
        if (ret < 0) {
            xlog_err("request failed\n");
            error_flag = true;
            break;
        }
    } while (0);

    return error_flag ? -1 : 0;
}

const char *uart_prot_mode_name(uart_prot_mode mode)
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