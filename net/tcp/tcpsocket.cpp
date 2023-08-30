#include "tcpsocket.hpp"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <netinet/tcp.h>

#include <poll.h>
#include <unistd.h>
#include <fcntl.h>

#include <chrono>

#include "net_utils.hpp"
#include "xlog.hpp"

#define MKTAG(a,b,c,d)   ((a) | ((b) << 8) | ((c) << 16) | ((unsigned)(d) << 24))
#define FFERRTAG(a, b, c, d) (-(int)MKTAG(a, b, c, d))
#define AVERROR_EXIT               FFERRTAG( 'E','X','I','T') ///< Immediate exit was requested; the called function should not be restarted

#define AVERROR(err) (-(err))
#define FF_ARRAY_ELEMS(array) (sizeof(array)/sizeof(array[0]))
#define FFMIN(a,b) std::min(a,b)

#define POLLING_TIME 100 /// Time in milliseconds between interrupt check

#define MY_ASSERT(expr) \
    do { \
        if (!(expr)) { \
            xlog_err("assertion on " #expr "failed");\
        } \
    } while (0)

#define av_assert0(expr) MY_ASSERT(expr)

struct ConnectionAttempt {
    int fd;
    int64_t deadline_us;
    struct addrinfo *addr;
};

struct TCPContext 
{
    int fd{-1};
    int open_timeout{-1};

    TCPSocketOption option;
};

struct TCPSocket::Data
{
    TCPContext tcp_ctx;

};

TCPSocket::TCPSocket(const TCPSocketOption &option)
{
    std::shared_ptr<Data> data = init(option);
    if (!data)
    {
        xlog_err("init failed");
        return ;
    }

    _d = data;
}

int TCPSocket::tcp_open(const std::string &url)
{
    UrlSplitResult url_info;
    int port = -1;
    int fd = -1;
    int ret = -1;
    struct addrinfo hints{}, *ai{nullptr}, *cur_ai{nullptr};

    bool berror = false;

    do 
    {
        if (!_d)
        {
            xlog_err("null");
            berror = true;
            break;
        }

        url_info = net_utils_url_split(url);
        port = atoi(url_info.port.c_str());
        if (port <= 0 || port >= 65536)
        {
            xlog_err("port invalid(port:%d)", port);
            berror = true;
            break;
        }

        _d->tcp_ctx.open_timeout = 5000000;

        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;

        if (_d->tcp_ctx.option.listen)
        {
            hints.ai_flags |= AI_PASSIVE;
        }
        
        if (url_info.hostname.empty())
        {
            // getaddrinfo_a 
            ret = getaddrinfo(nullptr, url_info.port.c_str(), &hints, &ai);
        }
        else 
        {
            ret = getaddrinfo(url_info.hostname.c_str(), url_info.port.c_str(), &hints, &ai);
        }
        if (ret)
        {
            xlog_err("Failed to resolve hostname: %s: %s", 
                url_info.hostname.c_str(), gai_strerror(ret));
            berror = true;
            break;
        }

        cur_ai = ai;

        if (_d->tcp_ctx.option.listen > 0)
        {
            while (cur_ai && fd < 0) {
                fd = ff_socket(cur_ai->ai_family,
                            cur_ai->ai_socktype,
                            cur_ai->ai_protocol);
                if (fd < 0) {
                    ret = ff_neterrno();
                    cur_ai = cur_ai->ai_next;
                }
            }
            if (fd < 0)
            {
                xlog_err("socket failed");
                berror = true;
                break;
            }
            customize_fd(fd, cur_ai->ai_family);
        }

        if (2 == _d->tcp_ctx.option.listen)
        {
            // multi-client
            if ((ret = ff_listen(fd, cur_ai->ai_addr, cur_ai->ai_addrlen)) < 0)
            {
                xlog_err("listen failed");
                berror = true;
                break;
            }
        }
        else if (1 == _d->tcp_ctx.option.listen)
        {
            // single client
            if ((ret = ff_listen_bind(fd, cur_ai->ai_addr, cur_ai->ai_addrlen,
                                    _d->tcp_ctx.option.listen_timeout)) < 0)
            {
                xlog_err("listen failed");
                berror = true;
                break;
            }
            // Socket descriptor already closed here. Safe to overwrite to client one.
            fd = ret;
        }
        else 
        {
            ret = ff_connect_parallel(ai, _d->tcp_ctx.open_timeout / 1000,
                3, &fd);
            if (ret < 0)
            {
                xlog_err("connect parallel failed");
                berror = true;
                break;
            }
        }
    }
    while (0);

    if (berror)
    {
        if (fd >= 0)
        {
            closesocket(fd);
            fd = -1;
        }
    }
    else 
    {
        _d->tcp_ctx.fd = fd;
    }

    freeaddrinfo(ai);
    ai = nullptr;

    return (berror ? -1 : 0);
}

int TCPSocket::tcp_accept()
{
}

// Try a new connection to another address after 200 ms, as suggested in
// RFC 8305 (or sooner if an earlier attempt fails).
#define NEXT_ATTEMPT_DELAY_MS 200

int TCPSocket::ff_connect_parallel(struct addrinfo *addrs, int timeout_ms_per_address,
                        int parallel, int *fd)
{
    struct ConnectionAttempt attempts[3];
    struct pollfd pfd[3];
    int nb_attempts = 0, i, j;
    int64_t next_attempt_us = av_gettime_relative(), next_deadline_us;
    int last_err = AVERROR(EIO);
    socklen_t optlen;
    char errbuf[100], hostbuf[100], portbuf[20];

    if (parallel > FF_ARRAY_ELEMS(attempts))
        parallel = FF_ARRAY_ELEMS(attempts);

    print_address_list(addrs, "Original list of addresses");
    // This mutates the list, but the head of the list is still the same
    // element, so the caller, who owns the list, doesn't need to get
    // an updated pointer.
    interleave_addrinfo(addrs);
    print_address_list(addrs, "Interleaved list of addresses");

    while (nb_attempts > 0 || addrs) {
        // Start a new connection attempt, if possible.
        if (nb_attempts < parallel && addrs) {
            getnameinfo(addrs->ai_addr, addrs->ai_addrlen,
                        hostbuf, sizeof(hostbuf), portbuf, sizeof(portbuf),
                        NI_NUMERICHOST | NI_NUMERICSERV);
            xlog_dbg("Starting connection attempt to %s port %s",
                                      hostbuf, portbuf);
            last_err = start_connect_attempt(&attempts[nb_attempts], &addrs,
                                             timeout_ms_per_address);
            if (last_err < 0) {
                av_strerror(last_err, errbuf, sizeof(errbuf));
                xlog_err("Connected attempt failed: %s",
                                          errbuf);
                continue;
            }
            if (last_err > 0) {
                for (i = 0; i < nb_attempts; i++)
                    closesocket(attempts[i].fd);
                *fd = attempts[nb_attempts].fd;
                return 0;
            }
            pfd[nb_attempts].fd = attempts[nb_attempts].fd;
            pfd[nb_attempts].events = POLLOUT;
            next_attempt_us = av_gettime_relative() + NEXT_ATTEMPT_DELAY_MS * 1000;
            nb_attempts++;
        }

        av_assert0(nb_attempts > 0);
        // The connection attempts are sorted from oldest to newest, so the
        // first one will have the earliest deadline.
        next_deadline_us = attempts[0].deadline_us;
        // If we can start another attempt in parallel, wait until that time.
        if (nb_attempts < parallel && addrs)
            next_deadline_us = FFMIN(next_deadline_us, next_attempt_us);
        last_err = ff_poll_interrupt(pfd, nb_attempts,
                                     (next_deadline_us - av_gettime_relative())/1000,
                                     _d->tcp_ctx.option.interrupt_callback);
        if (last_err < 0 && last_err != AVERROR(ETIMEDOUT))
            break;

        // Check the status from the poll output.
        for (i = 0; i < nb_attempts; i++) {
            last_err = 0;
            if (pfd[i].revents) {
                // Some sort of action for this socket, check its status (either
                // a successful connection or an error).
                optlen = sizeof(last_err);
                if (getsockopt(attempts[i].fd, SOL_SOCKET, SO_ERROR, &last_err, &optlen))
                    last_err = ff_neterrno();
                else if (last_err != 0)
                    last_err = AVERROR(last_err);
                if (last_err == 0) {
                    // Everything is ok, we seem to have a successful
                    // connection. Close other sockets and return this one.
                    for (j = 0; j < nb_attempts; j++)
                        if (j != i)
                            closesocket(attempts[j].fd);
                    *fd = attempts[i].fd;
                    getnameinfo(attempts[i].addr->ai_addr, attempts[i].addr->ai_addrlen,
                                hostbuf, sizeof(hostbuf), portbuf, sizeof(portbuf),
                                NI_NUMERICHOST | NI_NUMERICSERV);
                    xlog_dbg("Successfully connected to %s port %s",
                                              hostbuf, portbuf);
                    return 0;
                }
            }
            if (attempts[i].deadline_us < av_gettime_relative() && !last_err)
                last_err = AVERROR(ETIMEDOUT);
            if (!last_err)
                continue;
            // Error (or timeout) for this socket; close the socket and remove
            // it from the attempts/pfd arrays, to let a new attempt start
            // directly.
            getnameinfo(attempts[i].addr->ai_addr, attempts[i].addr->ai_addrlen,
                        hostbuf, sizeof(hostbuf), portbuf, sizeof(portbuf),
                        NI_NUMERICHOST | NI_NUMERICSERV);
            av_strerror(last_err, errbuf, sizeof(errbuf));
            xlog_dbg("Connection attempt to %s port %s "
                                      "failed: %s", hostbuf, portbuf, errbuf);
            closesocket(attempts[i].fd);
            memmove(&attempts[i], &attempts[i + 1],
                    (nb_attempts - i - 1) * sizeof(*attempts));
            memmove(&pfd[i], &pfd[i + 1],
                    (nb_attempts - i - 1) * sizeof(*pfd));
            i--;
            nb_attempts--;
        }
    }
    for (i = 0; i < nb_attempts; i++)
        closesocket(attempts[i].fd);
    if (last_err >= 0)
        last_err = AVERROR(ECONNREFUSED);
    if (last_err != AVERROR_EXIT) {
        av_strerror(last_err, errbuf, sizeof(errbuf));
        xlog_err("Connection to %s failed: %s",
               "???", errbuf);
    }
    return last_err;
}


void TCPSocket::print_address_list(const struct addrinfo *addr,
                            const char *title)
{
    char hostbuf[100], portbuf[20];
    xlog_dbg("%s: ", title);
    while (addr) 
    {
        getnameinfo(addr->ai_addr, addr->ai_addrlen,
                    hostbuf, sizeof(hostbuf), portbuf, sizeof(portbuf),
                    NI_NUMERICHOST | NI_NUMERICSERV);
        xlog_dbg("Address %s port %s", hostbuf, portbuf);
        addr = addr->ai_next;
    }
}

void TCPSocket::interleave_addrinfo(struct addrinfo *base)
{
    struct addrinfo **next = &base->ai_next;
    while (*next) {
        struct addrinfo *cur = *next;
        // Iterate forward until we find an entry of a different family.
        if (cur->ai_family == base->ai_family) {
            next = &cur->ai_next;
            continue;
        }
        if (cur == base->ai_next) {
            // If the first one following base is of a different family, just
            // move base forward one step and continue.
            base = cur;
            next = &base->ai_next;
            continue;
        }
        // Unchain cur from the rest of the list from its current spot.
        *next = cur->ai_next;
        // Hook in cur directly after base.
        cur->ai_next = base->ai_next;
        base->ai_next = cur;
        // Restart with a new base. We know that before moving the cur element,
        // everything between the previous base and cur had the same family,
        // different from cur->ai_family. Therefore, we can keep next pointing
        // where it was, and continue from there with base at the one after
        // cur.
        base = cur->ai_next;
    }
}

std::shared_ptr<TCPSocket::Data> TCPSocket::init(const TCPSocketOption &option)
{
    std::shared_ptr<Data> data;
    data = std::make_shared<Data>();
    return data;
}

int64_t TCPSocket::av_gettime_relative()
{
    using namespace std::chrono;

    auto now = steady_clock::now().time_since_epoch();
    auto usec = duration_cast<microseconds>(now);
    return usec.count();
}

int TCPSocket::start_connect_attempt(struct ConnectionAttempt *attempt,
                                 struct addrinfo **ptr, int timeout_ms)
{
    struct addrinfo *ai = *ptr;
    int ret;

    *ptr = ai->ai_next;

    attempt->fd = ff_socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
    if (attempt->fd < 0)
        return ff_neterrno();
    attempt->deadline_us = av_gettime_relative() + timeout_ms * 1000;
    attempt->addr = ai;

    ff_socket_nonblock(attempt->fd, 1);

    // if (customize_fd) {
    if (true) {        
        ret = customize_fd(attempt->fd, ai->ai_family);
        if (ret) {
            closesocket(attempt->fd);
            attempt->fd = -1;
            return ret;
        }
    }

    while ((ret = connect(attempt->fd, ai->ai_addr, ai->ai_addrlen))) {
        ret = ff_neterrno();
        switch (ret) {
        case AVERROR(EINTR):
            if (ff_check_interrupt(_d->tcp_ctx.option.interrupt_callback)) {
                closesocket(attempt->fd);
                attempt->fd = -1;
                return AVERROR_EXIT;
            }
            continue;
        case AVERROR(EINPROGRESS):
        case AVERROR(EAGAIN):
            return 0;
        default:
            closesocket(attempt->fd);
            attempt->fd = -1;
            return ret;
        }
    }
    return 1;
}

int TCPSocket::ff_check_interrupt(InterruptCB cb)
{
    if (cb)
    {
        return cb();
    }
    return 0;
}

int TCPSocket::av_strerror(int errnum, char *errbuf, size_t errbuf_size)
{
    strerror_r(errnum, errbuf, errbuf_size);
    return 0;
}

int TCPSocket::ff_socket(int af, int type, int proto)
{
    int fd;

    fd = socket(af, type | SOCK_CLOEXEC, proto);
    if (fd == -1 && errno == EINVAL)
    {
        fd = socket(af, type, proto);
        if (fd != -1) {
            if (fcntl(fd, F_SETFD, FD_CLOEXEC) == -1)
            {
                xlog_err("Failed to set close on exec");
            }
        }
    }
    return fd;
}

int TCPSocket::closesocket(int fd)
{
    return close(fd);
}

int TCPSocket::ff_socket_nonblock(int socket, int enable)
{
    if (enable)
        return fcntl(socket, F_SETFL, fcntl(socket, F_GETFL) | O_NONBLOCK);
    else
        return fcntl(socket, F_SETFL, fcntl(socket, F_GETFL) & ~O_NONBLOCK);
}

int TCPSocket::ff_neterrno()
{
    return AVERROR(errno);
}

int TCPSocket::ff_poll_interrupt(struct pollfd *p, nfds_t nfds, int timeout,
                             InterruptCB cb)
{
    int runs = timeout / POLLING_TIME;
    int ret = 0;

    do {
        if (ff_check_interrupt(cb))
            return AVERROR_EXIT;
        ret = poll(p, nfds, POLLING_TIME);
        if (ret != 0) {
            if (ret < 0)
                ret = ff_neterrno();
            if (ret == AVERROR(EINTR))
                continue;
            break;
        }
    } while (timeout <= 0 || runs-- > 0);

    if (!ret)
        return AVERROR(ETIMEDOUT);
    return ret;
}


int TCPSocket::customize_fd(int fd, int family)
{
    if (!_d->tcp_ctx.option.local_addr.empty() || 
        !_d->tcp_ctx.option.local_port.empty()) {
        struct addrinfo hints = { 0 }, *ai, *cur_ai;
        int ret;

        hints.ai_family = family;
        hints.ai_socktype = SOCK_STREAM;

        ret = getaddrinfo(_d->tcp_ctx.option.local_addr.c_str(), _d->tcp_ctx.option.local_port.c_str(), &hints, &ai);
        if (ret) {
            xlog_err("Failed to getaddrinfo local addr: %s port: %s err: %s\n",
               _d->tcp_ctx.option.local_addr.c_str(), _d->tcp_ctx.option.local_port.c_str(), 
               gai_strerror(ret));
            return ret;
        }

        cur_ai = ai;
        while (cur_ai) {
            ret = bind(fd, (struct sockaddr *)cur_ai->ai_addr, (int)cur_ai->ai_addrlen);
            if (ret)
                cur_ai = cur_ai->ai_next;
            else
                break;
        }
        freeaddrinfo(ai);

        if (ret) {
            xlog_err("bind local failed");
            return ret;
        }
    }
    /* Set the socket's send or receive buffer sizes, if specified.
       If unspecified or setting fails, system default is used. */
    if (_d->tcp_ctx.option.recv_buffer_size > 0) {
        if (setsockopt (fd, SOL_SOCKET, SO_RCVBUF, &_d->tcp_ctx.option.recv_buffer_size, sizeof (_d->tcp_ctx.option.recv_buffer_size))) {
            xlog_err("setsockopt(SO_RCVBUF)");
        }
    }
    if (_d->tcp_ctx.option.send_buffer_size > 0) {
        if (setsockopt (fd, SOL_SOCKET, SO_SNDBUF, &_d->tcp_ctx.option.send_buffer_size, sizeof (_d->tcp_ctx.option.send_buffer_size))) {
            xlog_err("setsockopt(SO_SNDBUF)");
        }
    }
    if (_d->tcp_ctx.option.tcp_nodelay > 0) {
        if (setsockopt (fd, IPPROTO_TCP, TCP_NODELAY, &_d->tcp_ctx.option.tcp_nodelay, sizeof (_d->tcp_ctx.option.tcp_nodelay))) {
            xlog_err("setsockopt(TCP_NODELAY)");
        }
    }
#if !HAVE_WINSOCK2_H
    if (_d->tcp_ctx.option.tcp_mss > 0) {
        if (setsockopt (fd, IPPROTO_TCP, TCP_MAXSEG, &_d->tcp_ctx.option.tcp_mss, sizeof (_d->tcp_ctx.option.tcp_mss))) {
            xlog_err("setsockopt(TCP_MAXSEG)");
        }
    }
#endif /* !HAVE_WINSOCK2_H */

    return 0;
}

int TCPSocket::ff_listen(int fd, const struct sockaddr *addr,
            socklen_t addrlen)
{
    int ret;
    int reuse = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse))) 
    {
        xlog_err("setsockopt(SO_REUSEADDR) failed\n");
    }
    ret = bind(fd, addr, addrlen);
    if (ret)
        return ff_neterrno();

    ret = listen(fd, 1);
    if (ret)
        return ff_neterrno();
    return ret;
}

int TCPSocket::ff_accept(int fd, int timeout)
{
    int ret;
    struct pollfd lp = { fd, POLLIN, 0 };

    ret = ff_poll_interrupt(&lp, 1, timeout, _d->tcp_ctx.option.interrupt_callback);
    if (ret < 0)
        return ret;

    ret = accept(fd, NULL, NULL);
    if (ret < 0)
        return ff_neterrno();
    if (ff_socket_nonblock(ret, 1) < 0)
    {
        xlog_err("ff_socket_nonblock failed\n");
    }

    return ret;
}

int TCPSocket::ff_listen_bind(int fd, const struct sockaddr *addr,
                socklen_t addrlen, int timeout)
{
    int ret;
    if ((ret = ff_listen(fd, addr, addrlen)) < 0)
        return ret;
    if ((ret = ff_accept(fd, timeout)) < 0)
        return ret;
    closesocket(fd);
    return ret;
}

