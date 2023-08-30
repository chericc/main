#include <gtest/gtest.h>

#include "tcpsocket.hpp"

TEST(tcpsocket, base)
{
    TCPSocketOption opt;

    opt.listen = 0;

    TCPSocket tcp(opt);
    tcp.open("tcp://baidu.com:80");
}