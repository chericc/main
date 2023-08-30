#pragma once

#include <memory>
#include <string>
#include <functional>
#include <sys/socket.h>

#include <poll.h>

using InterruptCB = std::function<int(void)>;

struct TCPSocketOption
{
    int listen{0}; // 2: multi-client 1: single client 0: connect
    std::string local_port;
    std::string local_addr;
    int timeout{-1};
    int listen_timeout{-1};
    int send_buffer_size{-1};
    int recv_buffer_size{-1};
    int tcp_nodelay{0};
    int tcp_mss{-1};

    InterruptCB interrupt_callback;
};

class TCPSocket
{
public:
    TCPSocket(const TCPSocketOption &option);
    ~TCPSocket() = default;

    int open(const std::string &url);
    int accept();
    
private:
    struct Data;
    std::shared_ptr<Data> _d;

    std::shared_ptr<Data> init(const TCPSocketOption &option);

    int customize_fd(int fd, int family);
    
    int ff_connect_parallel(struct addrinfo *addrs, int timeout_ms_per_address,
                            int parallel, int *fd);
    void print_address_list(const struct addrinfo *addr,
                               const char *title);

    int start_connect_attempt(struct ConnectionAttempt *attempt,
                                 struct addrinfo **ptr, int timeout_ms);

    static int64_t av_gettime_relative();
    static int ff_socket(int af, int type, int proto);
    static int closesocket(int fd);
    static int ff_socket_nonblock(int socket, int enable);
    static int ff_neterrno();
    static int ff_check_interrupt(InterruptCB cb);
    static void interleave_addrinfo(struct addrinfo *base);
    static int ff_poll_interrupt(struct pollfd *p, nfds_t nfds, int timeout,
                             InterruptCB cb);
    static int av_strerror(int errnum, char *errbuf, size_t errbuf_size);
    static int ff_listen(int fd, const struct sockaddr *addr,
              socklen_t addrlen);
    static int ff_accept(int fd, int timeout);
    static int ff_listen_bind(int fd, const struct sockaddr *addr,
                   socklen_t addrlen, int timeout);
};
