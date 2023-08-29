#include "tcpsock.hpp"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "xlog.hpp"

struct TCPSocket::Data
{
    TCPSocketOption option;
};

TCPSocket::TCPSocket(const TCPSocketOption &option)
{
    std::shared_ptr<Data> data = init();
    if (!data)
    {
        xlog_err("init failed");
        return ;
    }

    _d = data;
}