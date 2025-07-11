#ifndef __UART_PROT_HELPER_HPP__
#define __UART_PROT_HELPER_HPP__

#include <chrono>
#include <memory>
#include <functional>
#include <vector>
#include <mutex>
#include <cstdint>
#include <condition_variable>

#include "uart_prot_pub.h"
#include "TinyFrameWrapper.h"

/*

This file defines protocol-specific implementations.

*/

class UartProt {
public:
    using Clock = std::chrono::steady_clock;
    using Timepoint = Clock::time_point;
    using Duration = Clock::duration;
    using Lock = std::unique_lock<std::mutex>;

    using cb_function_type = std::function<int(void const*, size_t)>;
    static constexpr size_t buf_max_size = 1024;
    
    static std::shared_ptr<UartProt> produce(enum UART_PROT_MODE mode, 
        struct uart_prot_param const* param, cb_function_type uart_write);

    // for caller to write data
    // will do some thing to input data(now and future) in this.
    virtual int request(const void* out_data, size_t out_data_size,
        void *resp_data, size_t *resp_data_size, Duration timeout) = 0;

    // for comming uart data to input (eg, in a callback)
    // if data used, return 0, else return 1
    virtual int input(const void *data, size_t size) = 0;

    // for caller to get this object's type
    virtual enum UART_PROT_MODE type() = 0;
    
    UartProt() = default;
    virtual ~UartProt() = default;
protected:

    void _init(struct uart_prot_param const* param, cb_function_type uart_write);

    struct uart_prot_param _param = {};
    cb_function_type _cb_write_uart;

    std::mutex _mutex_buf;
    std::condition_variable  _cond_buf_changed;
    std::vector<uint8_t> _buf;

    // uart can only in requsting or being requested mode.
    // when requsting, input is treated as response.
    // when being requested, input is treated as request.
    bool _requesting = false;

    const Duration min_interval = std::chrono::milliseconds(500);
};

/*
Communicate over uart with original data.
*/
class UartProtRaw : public UartProt {
public:

    int request(const void* out_data, size_t out_data_size,
        void *resp_data, size_t *resp_data_size, Duration timeout) override;
    int input(const void *data, size_t size) override;
    enum UART_PROT_MODE type() override;

protected:
    int handle_request(const void *data, size_t size);
    int handle_response(const void *data, size_t size);
};

/*
Wrap data with header and tail to make it a data packet over uart.
*/
class UartProtMsg : public UartProt {
public:
    UartProtMsg();
    ~UartProtMsg() override;
    
    int request(const void* out_data, size_t out_data_size,
        void *resp_data, size_t *resp_data_size, Duration timeout) override;
    int input(const void *data, size_t size) override;
    enum UART_PROT_MODE type() override;

    static TF_Result id_listener_static(TinyFrame *tf, TF_Msg *msg);
    TF_Result id_listener(TinyFrame *tf, TF_Msg *msg);

    static TF_Result generic_listener_static(TinyFrame *tf, TF_Msg *msg);
    TF_Result generic_listener(TinyFrame *tf, TF_Msg *msg);

    void tf_writeimpl(TinyFrame *tf, const uint8_t *buff, uint32_t len);

    int gen_frame_id();

    enum MSG_TYPE {
        MSG_TYPE_REQ = 0,
    };
protected:
private:
    TinyFrame *_tf = nullptr;

    static TF_ID s_frame_id;
};


#endif // __UART_PROT_HELPER_HPP__