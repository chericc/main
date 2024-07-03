#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

int main() {
    int serial_port = open("/dev/ttySLB3", O_RDWR);

    if (serial_port < 0) {
        perror("Error opening serial port");
        return 1;
    }

    struct termios tty = { 0 };

    if (tcgetattr(serial_port, &tty) != 0) {
        perror("Error from tcgetattr");
        close(serial_port);
        return 1;
    }

    cfsetospeed(&tty, B115200);
    cfsetispeed(&tty, B115200);

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

    if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
        perror("Error from tcsetattr");
        close(serial_port);
        return 1;
    }

    while (1)
    {
        const char *msg = "Hello, UART!\n";
        int re_write = write(serial_port, msg, strlen(msg));
        printf("write %d bytes\n", re_write);

        char read_buf[256];
        memset(read_buf, '\0', sizeof(read_buf));

        int num_bytes = read(serial_port, read_buf, sizeof(read_buf));

        if (num_bytes < 0) {
            perror("Error reading from serial port");
            close(serial_port);
            return 1;
        }

        printf("Read %d bytes. Received message: %s\n", num_bytes, read_buf);
    }

    close(serial_port);
    return 0;
}