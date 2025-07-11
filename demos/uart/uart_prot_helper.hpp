#ifndef __UART_PROT_HELPER_HPP__
#define __UART_PROT_HELPER_HPP__

#include <chrono>
#include <memory>
#include <functional>
#include <vector>
#include <mutex>
#include <cstdint>

#include "uart_prot_pub.h"

/*

This file defines protocol-specific implementations.

*/

class UartProt {
public:
    using Clock = std::chrono::steady_clock;
    using Timepoint = Clock::time_point;
    using Duration = Clock::duration;
    using Lock = std::unique_lock<std::recursive_mutex>;

    using cb_function_type = std::function<int(void const*, size_t)>;
    static constexpr size_t buf_max_size = 1024;
    
    static std::shared_ptr<UartProt> produce(enum uart_prot_mode mode, 
        struct uart_prot_param const* param, cb_function_type uart_write);

    // for caller to write data
    // will do some thing to input data(now and future) in this.
    virtual int request(const void* out_data, size_t out_data_size,
        void *resp_data, size_t *resp_data_size, Duration timeout) = 0;

    // for comming uart data to input (eg, in a callback)
    // if data used, return 0, else return 1
    virtual int input(const void *data, size_t size) = 0;

    // for caller to get this object's type
    virtual enum uart_prot_mode type() = 0;
protected:
    UartProt() = default;
    ~UartProt() = default;

    void _init(struct uart_prot_param const* param, cb_function_type uart_write);

    struct uart_prot_param _param = {};
    cb_function_type _cb_write_uart;

    std::recursive_mutex _mutex_buf;
    std::vector<uint8_t> _buf;

    // uart can only in requsting or being requested mode.
    // when requsting, input is treated as response.
    // when being requested, input is treated as request.
    bool _requesting = false;
};

/*
Communicate over uart with original data.
*/
class UartProtRaw : public UartProt {
public:
    const Duration min_interval = std::chrono::milliseconds(500);

    int request(const void* out_data, size_t out_data_size,
        void *resp_data, size_t *resp_data_size, Duration timeout) override;
    int input(const void *data, size_t size) override;
    enum uart_prot_mode type() override;

    int handle_request(const void *data, size_t size);
    int handle_response(const void *data, size_t size);
protected:
};

/*
Wrap data with header and tail to make it a data packet over uart.
*/
class UartProtMsg : public UartProt {
public:
    int request(const void* out_data, size_t out_data_size,
        void *resp_data, size_t *resp_data_size, Duration timeout) override;
    int input(const void *data, size_t size) override;
    enum uart_prot_mode type() override;
};


#endif // __UART_PROT_HELPER_HPP__