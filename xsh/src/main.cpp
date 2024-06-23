#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>

#include <vector>

#include "defs.hpp"

static void config_termios() {
    int ret = 0;
    int fd = STDIN_FILENO;
    struct termios stermios {};

    ret = tcgetattr(fd, &stermios);
    if (ret < 0) {
        printf("tcgetattr failed\n");
        return;
    }

    stermios.c_lflag &= ~(ICANON | ECHO | ECHONL | ICRNL);

    ret = tcsetattr(fd, TCSANOW, &stermios);
    if (ret < 0) {
        printf("tcsetattr failed\n");
        return;
    }

    return;
}

/**
 * 带转义的输出：
 * - 可显示字符：直接输出
 * - 控制字符：
 *    - 支持的，按控制行为输出
 *    - 不支持的，输出转义字符
 */
static int putchar_escape(int c) {
    int ret = 0;
    uchar c_copy = (c % 256);

    if ((uchar)ASCII::LF == c_copy || (uchar)ASCII::BS == c_copy) {
        putchar(c_copy);
    } else if ((uchar)ASCII::DEL == c_copy) {
        putchar((uchar)ASCII::BS);
        putchar((uchar)ASCII::SPACE);
        putchar((uchar)ASCII::BS);
    } else if (c_copy < (uchar)ASCII::C_START || c_copy > (uchar)ASCII::C_END) {
        ret = printf("\\%hho", c_copy);
    } else {
        ret = putchar(c_copy);
    }
    return ret;
}

static void deal_special_input(const input_buffer_type& input) {}

static void run(int argc, char* argv[]) {
    while (true) {
        fd_set rfds{};
        struct timeval tv {};
        int retval = 0;

        /* Watch stdin (fd 0) to see when it has input. */

        FD_ZERO(&rfds);
        FD_SET(STDIN_FILENO, &rfds);

        /* Wait up to five seconds. */

        tv.tv_sec = 5;
        tv.tv_usec = 0;

        retval = select(1, &rfds, NULL, NULL, &tv);
        /* Don't rely on the value of tv now! */

        if (retval == -1) {
            printf("select error");
            break;
        } else if (retval) {
            printf("Data is available now.\n");
            /* FD_ISSET(0, &rfds) will be true. */

        } else {
            printf("No data within five seconds.\n");
            continue;
        }
    }
}

int main(int argc, char* argv[]) {
    config_termios();

#if 0
    uchar c_input = 0;
    bool special_flag = false;
    input_buffer_type input_buffer;

    while (true)
    {
        c_input = getchar();

        // printf("%hhx\n", c_input);
        // continue;

        if ((uchar)ASCII::LF == c_input)
        {
            special_flag = false;
        }
        else
        {
            if (special_flag)
            {
                input_buffer.push_back(c_input);

                deal_special_input(input_buffer);

                continue;
            }

            if ((uchar)ASCII::ESC == c_input)
            {
                special_flag = true;
                input_buffer.push_back(c_input);
                continue;
            }
        }

        putchar_escape(c_input);
    }
#endif

    run(argc, argv);

    return 0;
}