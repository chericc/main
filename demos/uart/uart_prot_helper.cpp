#include "uart_prot_helper.hpp"

#include <cstring>
#include <thread>

#include "xlog.h"
#include "uart_tool.h"

std::shared_ptr<UartProt> UartProt::produce(enum UART_PROT_MODE mode,
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
        
        auto start = Clock::now();
        auto dead = start + timeout;
        for (;;) {
            auto now = Clock::now();
            if (now >= dead) {
                break;
            }
            std::this_thread::sleep_for(wake_up_interval);
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

        std::array<uint8_t, buf_max_size> response_buf = {};
        size_t response_buf_size = response_buf.size();

        if (_param.on_reqeust_cb) {

            // tell user we have a request
            _param.on_reqeust_cb(data, size, response_buf.data(), &response_buf_size);

            if (response_buf_size > response_buf.size()) {
                xlog_err("inner error: response size over\n");
                error_flag = true;
                break;
            }

            // send user's response 
            if (response_buf_size > 0) {
                ret = _cb_write_uart(response_buf.data(), response_buf_size);
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
            // xlog_dbg("handle request\n");
            // ret = handle_request(data, size);
            // if (ret < 0) {
            //     xlog_err("handle_request failed\n");
            //     error_flag = true;
            // }
            
            xlog_dbg("raw mode not answering request\n");
            // not an error (just ignored)
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

enum UART_PROT_MODE UartProtRaw::type()
{
    return UART_PROT_MODE_RAW;
}

void TF_WriteImpl(TinyFrame *tf, const uint8_t *buff, uint32_t len)
{
    do {
        auto obj = (UartProtMsg*)(tf->userdata);
        if (!obj) {
            xlog_err("null obj\n");
            break;
        }

        obj->tf_writeimpl(tf, buff, len);
    } while (0);
    return ;
}

UartProtMsg::UartProtMsg()
{
    _tf = TF_Init(TF_MASTER);
    _tf->userdata = this;
    TF_AddGenericListener(_tf, generic_listener_static);

    return ;
}

UartProtMsg::~UartProtMsg()
{
    if (_tf) {
        TF_RemoveGenericListener(_tf, generic_listener_static);
        TF_DeInit(_tf);
        _tf = nullptr;
    }
}

TF_Result UartProtMsg::id_listener_static(TinyFrame *tf, TF_Msg *msg)
{
    TF_Result result = TF_STAY;
    do {
        auto obj = static_cast<UartProtMsg*>(tf->userdata);
        if (!obj) {
            xlog_err("null obj\n");
            break;
        }
        result = obj->id_listener(tf, msg);
    } while (0);
    return result;
}

TF_Result UartProtMsg::id_listener(TinyFrame *tf, TF_Msg *msg)
{
    TF_Result result = TF_STAY;
    do {
        // we don't check id, for request should be called in 
        // single thread
        if (msg->len > buf_max_size) {
            xlog_err("len over max size\n");
            break;
        }

        Lock lock_buf(_mutex_buf);
        _buf.resize(msg->len);
        memcpy(_buf.data(), static_cast<const void*>(msg->data), msg->len);
        _cond_buf_changed.notify_one();
    } while (0);

    return result;
}

TF_Result UartProtMsg::generic_listener_static(TinyFrame *tf, TF_Msg *msg)
{
    TF_Result result = TF_STAY;
    do {
        auto obj = static_cast<UartProtMsg*>(tf->userdata);
        if (!obj) {
            xlog_err("null obj\n");
            break;
        }
        result = obj->generic_listener(tf, msg);
    } while (0);
    return result;
}

TF_Result UartProtMsg::generic_listener(TinyFrame *tf, TF_Msg *msg)
{
    TF_Result result = TF_STAY;

    do {
        int ret = 0;

        std::array<uint8_t, buf_max_size> response_buf = {};
        size_t response_buf_size = response_buf.size();

        if (!_param.on_reqeust_cb) {
            xlog_err("null cb\n");
            break;
        }

        _param.on_reqeust_cb(tf->data, tf->len, 
            response_buf.data(), &response_buf_size);
        
        if (response_buf_size <= 0) {
            xlog_dbg("no response\n");
            break;
        }

        if (response_buf_size > response_buf.size()) {
            xlog_err("inner error: response size over\n");
            break;
        }

        auto const len_max = std::numeric_limits<decltype(msg->len)>::max();
        if (response_buf_size > len_max) {
            xlog_err("response too big for msg(%d > %d)\n", response_buf_size, len_max);
            break;
        }
        
        TF_Msg msg_resp;
        TF_ClearMsg(&msg_resp);
        msg_resp.data = response_buf.data();
        msg_resp.len = response_buf_size;
        msg_resp.frame_id = msg->frame_id;

        xlog_dbg("respond with id: %d\n", (int)msg_resp.frame_id);

        ret = TF_Respond(tf, &msg_resp);
        if (ret < 0) {
            xlog_err("respond failed\n");
            break;
        }
        
    } while (0);

    return result;
}

void UartProtMsg::tf_writeimpl(TinyFrame *tf, const uint8_t *buff, uint32_t len)
{
    // bool error_flag = false;

    do {
        int ret = 0;
        ret = _cb_write_uart(buff, len);
        if (ret < 0) {
            xlog_err("write uart failed\n");
            // error_flag = true;
            break;
        }

    } while (0);

    return ;
}

int UartProtMsg::request(const void* out_data, size_t out_data_size,
        void *resp_data, size_t *resp_data_size, Duration timeout)
{
    bool error_flag = false;

    int listener_id = -1;

    do {

        TF_Msg msg = {};
        TF_ClearMsg(&msg);
        msg.data = static_cast<const uint8_t *>(out_data);
        msg.len = out_data_size;
        msg.type = MSG_TYPE_REQ;

        bool need_response = false;

        if (resp_data 
                && resp_data_size 
                && (*resp_data_size > 0) ) {
            need_response = true;
            xlog_dbg("need response\n");
        } else {
            xlog_dbg("no need response\n");
        }

        Lock lock_buf(_mutex_buf);
        _buf.clear();
        TF_Query(_tf, &msg, id_listener_static, 5);
        listener_id = msg.frame_id;
            
        if (need_response) {

            auto start = Clock::now();
            auto dead = start + timeout;

            for (;;) {
                auto now = Clock::now();
                if (now >= dead) {
                    error_flag = true;
                    xlog_err("timeout\n");
                    break;
                }
                if (!_buf.empty()) {
                    break;
                }
                _cond_buf_changed.wait_for(lock_buf, wake_up_interval);
            }

            if (error_flag) {
                break;
            }

            if (!_buf.empty()) {
                size_t resp_data_size_tmp = *resp_data_size;
                if (resp_data_size_tmp < _buf.size()) {
                    xlog_err("resp buf small: %zu < %zu)\n", 
                        resp_data_size_tmp, _buf.size());
                    error_flag = true;
                    break;
                }
                memcpy(resp_data, _buf.data(), _buf.size());
                *resp_data_size = _buf.size();
            } else {
                xlog_war("got no content\n");
            }
        }

        // success
    } while (0);

    if (listener_id >= 0) {
        TF_RemoveIdListener(_tf, listener_id);
    }
    

    return error_flag ? -1 : 0;
}

int UartProtMsg::input(const void *data, size_t size)
{
    // uart_dump("input", (uint8_t const* )data, size);
    TF_Accept(_tf, static_cast<const uint8_t*>(data), size);
    return 0;
}

enum UART_PROT_MODE UartProtMsg::type()
{
    return UART_PROT_MODE_MSG;
}