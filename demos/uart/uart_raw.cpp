#include "uart_raw.h"

#include <thread>
#include <memory>
#include <cerrno>
#include <cstring>
#include <mutex>

#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#include <sys/prctl.h>

#include "xlog.h"
#include "uart_dump_tool.h"

using Lock = std::unique_lock<std::mutex>;

struct uart_raw_obj {
    int uart_fd = -1;
    int break_flag = 0;
    std::shared_ptr<std::thread> trd = nullptr;

    struct uart_raw_param param = {};

    // 串口只允许逐个通信
    std::mutex mutex_uart;
};

static void uart_raw_trd_worker(uart_raw_obj *obj)
{
    prctl(PR_SET_NAME, "uartworker");

    xlog_war("trd in\n");

    using Clock = std::chrono::steady_clock;
    auto last_tp = Clock::now();

    while (!obj->break_flag) {
        
        char buf[128] = {};
        ssize_t ret = read(obj->uart_fd, buf, sizeof(buf));
        if (ret > 0) {
            if (obj->param.read_cb) {
                uart_dump("read", (uint8_t*)buf, ret);
                obj->param.read_cb(buf, ret, obj->param.user);
            }
        } else if (ret == 0) {
            auto now = Clock::now();
            auto dur_pass = now - last_tp;
            last_tp = now;
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(dur_pass).count();
            if (ms < 500) {
                xlog_err("inner err: too frequent call(%d)", static_cast<int>(ms));
            }
        } else {
            xlog_err("read failed\n");
            break;
        }
    }

    xlog_war("trd out\n");

    return ;
}

static int convert_baudrate(int baudrate, speed_t *speed)
{
    bool error_flag = false;

    // just some common convert here
    switch (baudrate) {
        case 9600: {
            *speed = B9600;
            break;
        }
        case 115200: {
            *speed = B115200;
            break;
        }
        case 921600: {
            *speed = B921600;
            break;
        }
        default: {
            xlog_err("not found: baudrate=%d\n", baudrate);
            error_flag = true;
            break;
        }
    }

    return error_flag ? -1 : 0;
}

static int uart_raw_open_uart_fd(struct uart_raw_param const* param)
{
    int serial_port_fd = -1;
    bool error_flag = false;
    char buf[64] = {};

    do {
        int ret = 0;

        xlog_dbg("open uart: dev:%s,baudrate:%d\n", param->uart_dev_path, param->baudrate);

        serial_port_fd = open(param->uart_dev_path, O_RDWR);
        if (serial_port_fd < 0) {
            strerror_r(errno, buf, sizeof(buf));
            xlog_err("open uart failed: %s\n", buf);
            error_flag = true;
            break;
        }

        struct termios tty = {};

        if (tcgetattr(serial_port_fd, &tty) != 0) {
            strerror_r(errno, buf, sizeof(buf));
            xlog_err("tcgetattr failed: %s\n", buf);
            error_flag = true;
            break;
        }

        speed_t baudrate = 0;
        ret = convert_baudrate(param->baudrate, &baudrate);
        if (ret < 0) {
            xlog_err("convert failed\n");
            error_flag = true;
            break;
        }

        cfsetospeed(&tty, baudrate);
        cfsetispeed(&tty, baudrate);

        tty.c_cflag &= ~PARENB; // No parity bit
        tty.c_cflag &= ~CSTOPB; // Only one stop bit
        tty.c_cflag &= ~CSIZE;  // Clear the character size bits
        tty.c_cflag |= CS8;     // 8 bits per byte

        tty.c_cflag &= ~CRTSCTS; // No hardware flow control
        tty.c_cflag |= CREAD | CLOCAL; // Turn on the receiver and set local mode

        tty.c_lflag &= ~ICANON;
        tty.c_lflag &= ~ECHO;
        tty.c_lflag &= ~ECHOE;
        tty.c_lflag &= ~ECHONL;
        tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP

        tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Disable XON/XOFF flow control

        tty.c_iflag &= ~(ICRNL | INLCR | IGNCR); // Disable CR-to-NL, NL-to-CR and ignore CR

        tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
        tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed

        tty.c_cc[VTIME] = 10; // Wait for up to 1s (10 deciseconds) for data
        tty.c_cc[VMIN] = 0;

        if (tcsetattr(serial_port_fd, TCSANOW, &tty) != 0) {
            strerror_r(errno, buf, sizeof(buf));
            xlog_err("tcsetattr failed: %s\n", buf);
            error_flag = true;
            break;
        }
    } while (0);

    if (error_flag) {
        xlog_war("cleaning\n");
        if (serial_port_fd >= 0) {
            close(serial_port_fd);
            serial_port_fd = -1;
        }
    }

    return serial_port_fd;
}

static void uart_raw_close_imp(uart_raw_obj *obj)
{
    do {
        if (!obj) {
            xlog_war("null obj\n");
            break;
        }

        if (obj->trd) {
            obj->break_flag = true;
            if (obj->trd->joinable()) {
                xlog_war("before join\n");
                obj->trd->join();
                xlog_war("after join\n");
            }
        }

        if (obj->uart_fd >= 0) {
            close(obj->uart_fd);
            obj->uart_fd = -1;
        }

        delete obj;
        obj = nullptr;
    } while (0);

    return ;
}

uart_raw_handle uart_raw_open(struct uart_raw_param const* param)
{
    uart_raw_obj *obj = nullptr;

    bool error_flag = false;

    do {
        obj = new uart_raw_obj();

        obj->param = *param;
        
        obj->uart_fd = uart_raw_open_uart_fd(param);
        if (obj->uart_fd < 0) {
            xlog_err("uart_raw_open_uart_fd failed\n");
            error_flag = true;
            break;
        }

        obj->break_flag = false;
        obj->trd = std::make_shared<std::thread>(uart_raw_trd_worker, obj);

        // success
    } while (0);

    if (error_flag) {
        uart_raw_close_imp(obj);
        obj = nullptr;
    }

    return static_cast<uart_raw_handle>(obj);
}

int uart_raw_close(uart_raw_handle handle)
{
    auto obj = static_cast<uart_raw_obj*>(handle);

    if (obj) {
        uart_raw_close_imp(obj);
    }
    return 0;
}

int uart_raw_write(uart_raw_handle handle, void const* data, size_t size)
{
    bool error_flag = false;

    do {
        auto obj = static_cast<uart_raw_obj*>(handle);
        if (!obj) {
            xlog_err("null obj\n");
            error_flag = true;
            break;
        }
        Lock lock_uart(obj->mutex_uart);

        // xlog_dbg("write: %zu bytes begin\n", size);

        ssize_t ret = write(obj->uart_fd, data, size);
        if (ret < 0) {
            xlog_err("write failed\n");
            error_flag = true;
            break;
        } else {
            if (static_cast<size_t>(ret) != size) {
                xlog_err("partial write\n");
                error_flag = true;
                break;
            } else {
                uart_dump("write", (uint8_t const*)data, size);
            }
        }
        ret = tcdrain(obj->uart_fd);
        if (ret < 0) {
            xlog_err("tcdrain on fd:%d failed\n", obj->uart_fd);
            error_flag = true;
            break;
        }

        // xlog_dbg("write: %zu bytes end\n", size);
    } while (0);

    return error_flag ? -1 : 0;
}

int uart_raw_flush(uart_raw_handle handle, enum UART_RAW_FLUSH_TARGET target)
{
    bool error_flag = false;
    do {
        int ret = 0;

        auto obj = static_cast<uart_raw_obj*>(handle);
        if (!obj) {
            xlog_err("null obj\n");
            error_flag = true;
            break;
        }

        Lock lock_uart(obj->mutex_uart);
        int queue_sel = -1;
        if (UART_RAW_FLUSH_IN == target) {
            queue_sel = TCIFLUSH;
        } else if (UART_RAW_FLUSH_OUT == target) {
            queue_sel = TCOFLUSH;
        } else if (UART_RAW_FLUSH_IN_OUT == target) {
            queue_sel = TCIOFLUSH;
        }
        if (queue_sel < 0) {
            xlog_err("invalid target: (%d)\n", static_cast<int>(target));
            error_flag = true;
            break;
        }

        ret = tcflush(obj->uart_fd, queue_sel);
        if (ret < 0) {
            xlog_err("tcflush failed\n");
            error_flag = 1;
            break;
        }
    } while (0);

    return error_flag ? -1 : 0;
}