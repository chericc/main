#include <cstdio>
#include <list>
#include <array>

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <signal.h>

#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

#include "xlog.hpp"
#include "xos_independent.hpp"

std::string sockaddr_str(struct sockaddr *paddr, int socklen)
{
    FILE *fp = nullptr;
    char *streambuf = nullptr;
    size_t size = 0;
    std::string str;

    if (paddr)
    {
        std::array<char, 128> buf;
        fp = open_memstream(&streambuf, &size);

        if (socklen == sizeof(sockaddr_in))
        {
            auto sin = reinterpret_cast<sockaddr_in *>(paddr);
            evutil_inet_ntop(AF_INET, &sin->sin_addr, buf.data(),
                buf.size());
            fprintf(fp, "<IPv4: %s:%hu>", buf.data(), ntohs(sin->sin_port));
        }
        else if (socklen == sizeof(sockaddr_in6))
        {
            auto sin6 = reinterpret_cast<sockaddr_in6 *>(paddr);
            evutil_inet_ntop(AF_INET6, &sin6->sin6_addr, buf.data(),
                buf.size());
            fprintf(fp, "<IPv6: %s:%hu>", buf.data(), ntohs(sin6->sin6_port));
        }
    }

    if (fp)
    {
        fclose(fp);
        fp = nullptr;
    }

    if (streambuf)
    {
        if (size > 0)
        {
            str.assign(streambuf);
        }
        
        free(streambuf);
        streambuf = nullptr;
    }

    return str;
}

std::string addrinfo_str(const addrinfo *paddr)
{
    FILE *fp = nullptr;
    char *streambuf = nullptr;
    size_t size = 0;
    std::string str;

    fp = open_memstream(&streambuf, &size);

    if (paddr)
    {
        std::array<char, 128> buf;
        buf[0] = 0;

        if (paddr->ai_canonname)
        {
            fprintf(fp, "<name: %s>", paddr->ai_canonname);
        }

        fprintf(fp, "%s", sockaddr_str(paddr->ai_addr, paddr->ai_addrlen).c_str());
    }

    if (fp)
    {
        fclose(fp);
        fp = nullptr;
    }

    if (streambuf)
    {
        str.assign(streambuf);
        free(streambuf);
        streambuf = nullptr;
    }

    return str;
}

void read_cb(struct bufferevent *bev, void *ctx)
{
    xlog_dbg("read cb");
    struct evbuffer *output = bufferevent_get_output(bev);

}

void write_cb(struct bufferevent *bev, void *ctx)
{
    xlog_dbg("write cb");

}

void event_cb(struct bufferevent *bev, short event, void *ctx)
{
    xlog_dbg("event cb(%#hx)", event);
    if (event & BEV_EVENT_CONNECTED) 
    {
        xlog_dbg("Connection established");
    }

    if (event & BEV_EVENT_EOF)
    {
        xlog_dbg("Connection eof");
    }

    if (event & BEV_EVENT_ERROR)
    {
        std::array<char, 128> errstr;
        x_strerror(errno, errstr.data(), errstr.size());
        xlog_dbg("Got an error on the connection: %s", errstr.data());
    }

    xlog_dbg("Closing connection");
    bufferevent_free(bev);
}

void listener_cb(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *addr, int socklen, void *user)
{
    auto base = reinterpret_cast<struct event_base *>(user);
    struct bufferevent *bev = nullptr;

    xlog_dbg("got a connection: %s", sockaddr_str(addr, socklen).c_str());

    do 
    {
        int options = BEV_OPT_CLOSE_ON_FREE;
        bev = bufferevent_socket_new(base, fd, options);
        if (!bev)
        {
            xlog_err("bufferevent_socket_new failed");
            break;
        }

        bufferevent_setcb(bev, read_cb, write_cb, event_cb, nullptr);
        bufferevent_enable(bev, EV_WRITE);
        bufferevent_enable(bev, EV_READ);

        std::string msg = "Welcome!\n";
        bufferevent_write(bev, msg.data(), msg.size());
    }
    while (false);
}

void signal_cb(evutil_socket_t, short, void *user)
{
    auto *base = (struct event_base *)user;

    xlog_dbg("Interrupt");
    event_base_loopexit(base, nullptr);
}

int main()
{
    xlog_dbg("start");

    do 
    {
        struct event_base *base = nullptr;
        std::list<evconnlistener*> listener_list;
        struct addrinfo hints = {};
        struct event *signal_event = nullptr;

        base = event_base_new();
        if (!base)
        {
            xlog_err("failed");
            break;
        }

        signal_event = evsignal_new(base, SIGINT, signal_cb, base);
        if (!signal_event)
        {
            xlog_err("evsignal_new failed");
            break;
        }

        if (event_add(signal_event, nullptr) < 0)
        {
            xlog_err("event_add failed");
            break;
        }

        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;
        hints.ai_protocol = IPPROTO_TCP;
        struct addrinfo *result = nullptr;

        if (getaddrinfo(nullptr, "20000", &hints, &result) != 0)
        {
            xlog_err("getaddrinfo failed");
            break;
        }

        for (; result != nullptr; result = result->ai_next)
        {
            if (result->ai_family == PF_INET
                || result->ai_family == PF_INET6)
            {
                xlog_dbg("bind: %s", addrinfo_str(result).c_str());

                unsigned int flags = LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE;

                /* not using ipv4 mapped ipv6 address feature for compatibility. */
                if (result->ai_family == PF_INET6)
                {
                    flags |= LEV_OPT_BIND_IPV6ONLY;
                }

                auto listener = evconnlistener_new_bind(base, listener_cb, base, flags,
                    -1, result->ai_addr, static_cast<int>(result->ai_addrlen));

                if (!listener)
                {
                    xlog_err("bind failed");
                    continue;
                }
                listener_list.push_back(listener);
            }
        }

        freeaddrinfo(result);
        result = nullptr;

        event_base_dispatch(base);

        while (!listener_list.empty()) 
        {
            auto const& front = listener_list.front();
            evconnlistener_free(front);
            listener_list.pop_front();
        }

        event_free(signal_event);
        signal_event = nullptr;
        event_base_free(base);
        base = nullptr;
    }
    while (false);

    xlog_dbg("end");

    return 0;
}