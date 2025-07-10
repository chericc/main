#include "uart_prot_helper.hpp"

#include <cstring>
#include <thread>

#include "xlog.h"

std::shared_ptr<UartProt> UartProt::produce(enum uart_prot_mode mode,
    struct uart_prot_param const* param, std::function<int(void const*, size_t)> uart_write)
{
    std::shared_ptr<UartProt> prot = nullptr;

    switch (mode) {
        case UART_PROT_MODE_RAW: {
            prot = std::make_shared<UartProtRaw>();
            break;
        }
        case UART_PROT_MODE_MSG: {
            prot = std::make_shared<UartProtMsg>();
            break;
        }
        case UART_PROT_MODE_NONE: 
        default: {
            xlog_err("none prot\n");
            break;
        }
    }

    if (prot) {
        prot->_init(param, uart_write);
    }

    return prot;
}

int UartProtRaw::request(const void* out_data, size_t out_data_size,
        void *resp_data, size_t *resp_data_size, Duration timeout)
{
    bool error_flag = false;

    do {
        int ret = 0;

        // clear input buf
        _buf.clear();

        // send request
        ret = _cb_write_uart(out_data, out_data_size);
        if (ret < 0) {
            xlog_err("write uart failed\n");
            error_flag = true;
            break;
        }

        // wait response
        if (resp_data 
                && resp_data_size 
                && (*resp_data_size > 0) ) {    
            size_t resp_size_tmp = *resp_data_size;

            // when we need response, we just wait a fixed time for the response
            // because we don't know end flag

            auto start = Clock::now();
            auto dead = start + timeout;

            for (;;) {
                auto now = Clock::now();
                if (now >= dead) {
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }

            size_t min_resp_size = std::min(resp_size_tmp, _buf.size());
            memcpy(resp_data, _buf.data(), min_resp_size);
            *resp_data_size = min_resp_size;
        }
    } while(0);

    return error_flag ? -1 : 0;
}

int UartProtRaw::input(const void *data, size_t size)
{
    bool error_flag = false;

    do {
        Lock lock(_mutex_buf);

        size_t now_size = _buf.size();
        if (now_size > buf_max_size) {
            xlog_err("inner error, buf over\n");
            error_flag = true;
            break;
        }

        size_t left_size = buf_max_size - now_size;
        if (left_size < size) {
            xlog_err("buf small(input: %zu, now: %zu)\n", size, now_size);
            error_flag = true;
            break;
        }

        size_t new_size = now_size + size;
        _buf.resize(new_size);
        memcpy(_buf.data() + now_size, data, size);

        xlog_dbg("new size: %zu\n", new_size);
    } while (0);

    return error_flag ? -1 : 0;
}

enum uart_prot_mode UartProtRaw::type()
{
    return UART_PROT_MODE_RAW;
}


int UartProtMsg::request(const void* out_data, size_t out_data_size,
        void *resp_data, size_t *resp_data_size, Duration timeout)
{

}

int UartProtMsg::input(const void *data, size_t size)
{

}

enum uart_prot_mode UartProtMsg::type()
{

}
