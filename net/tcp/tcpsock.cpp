#include "tcpsock.hpp"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "net_utils.hpp"
#include "xlog.hpp"

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

int TCPSocket::open(const std::string &url)
{
    UrlSplitResult url_info;
    int port = -1;
    int ret = -1;
    struct addrinfo hints{}, *ai{nullptr}, *cur_ai{nullptr};

    do 
    {
        if (!_d)
        {
            xlog_err("null");
            break;
        }

        url_info = url_split(url);
        port = atoi(url_info.port.c_str());
        if (port <= 0 || port >= 65536)
        {
            xlog_err("port invalid(port:%d)", port);
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
    }
    while (0);

}