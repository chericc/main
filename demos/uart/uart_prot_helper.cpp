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

void UartProt::_init(struct uart_prot_param const* param, cb_function_type uart_write)
{
    _param = *param;
    _cb_write_uart = uart_write;
    _buf.reserve(buf_max_size);
}

int UartProtRaw::request(const void* out_data, size_t out_data_size,
        void *resp_data, size_t *resp_data_size, Duration timeout)
{
    bool error_flag = false;

    xlog_dbg("request begin\n");
    _requesting = true;

    do {
        int ret = 0;

        // clear input buf
        Lock lock(_mutex_buf);
        _buf.clear();
        lock.unlock();

        // send request
        ret = _cb_write_uart(out_data, out_data_size);
        if (ret < 0) {
            xlog_err("write uart failed\n");
            error_flag = true;
            break;
        }

        xlog_dbg("wait response begin\n");

        // wait response

        /*
        When we need response, wait a fixed time for the response
        because we don't know end flag. To avoid invalid response data treat 
        as request, we always wait no matter if response is needed.
        */

        if (timeout < min_interval) {
            timeout = min_interval;
        }
        
        auto start = Clock::now();
        auto dead = start + timeout;
        for (;;) {
            auto now = Clock::now();
            if (now >= dead) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

        if (resp_data 
                && resp_data_size 
                && (*resp_data_size > 0) ) {
            size_t resp_size_tmp = *resp_data_size;
            
            lock.lock();
            xlog_dbg("wait response end: size=%zu\n", _buf.size());

            size_t min_resp_size = std::min(resp_size_tmp, _buf.size());
            memcpy(resp_data, _buf.data(), min_resp_size);
            *resp_data_size = min_resp_size;
        } else {
            xlog_dbg("no need response\n");
        }
    } while(0);

    xlog_dbg("request end\n");
    _requesting = false;

    return error_flag ? -1 : 0;
}

int UartProtRaw::handle_request(const void *data, size_t size)
{
    bool error_flag = false;

    do {
        int ret = 0;

        std::array<uint8_t, 256> response_buf = {};
        size_t response_buf_size = response_buf.size();

        if (_param.on_reqeust_cb) {

            // tell user we have a request
            _param.on_reqeust_cb(data, size, response_buf.data(), &response_buf_size);

            size_t checked_res_buf_size = std::min(response_buf.size(), response_buf_size);
            if (checked_res_buf_size != response_buf_size) {
                xlog_err("inner error: response size over\n");
                error_flag = true;
                break;
            }

            // send user's response 
            if (response_buf_size > 0) {
                ret = _cb_write_uart(response_buf.data(), checked_res_buf_size);
                if (ret < 0) {
                    xlog_err("response failed\n");
                    error_flag = true;
                    break;
                }
            }
        }
    } while (0);

    return error_flag ? -1 : 0;
}

int UartProtRaw::handle_response(const void *data, size_t size)
{
    bool error_flag = false;

    do {
        size_t now_size = _buf.size();
        if (now_size > buf_max_size) {
            xlog_err("inner error, buf over\n");
            error_flag = true;
            break;
        }

        size_t left_size = buf_max_size - now_size;
        if (left_size < size) {
            xlog_err("buf full(input: %zu, now: %zu)\n", size, now_size);
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

int UartProtRaw::input(const void *data, size_t size)
{
    bool error_flag = false;

    do {
        int ret = 0;

        Lock lock(_mutex_buf);

        // not requesting: i.e. input is request
        if (!_requesting) {
            xlog_dbg("handle request\n");
            ret = handle_request(data, size);
            if (ret < 0) {
                xlog_err("handle_request failed\n");
                error_flag = true;
            }
            break;
        }

        xlog_dbg("handle response\n");
        ret = handle_response(data, size);
        if (ret < 0) {
            xlog_err("handle_response failed\n");
            error_flag = true;
            break;
        }

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
