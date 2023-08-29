#pragma once

#include <memory>
#include <string>

struct TCPSocketOption
{
    int listen{0};
    std::string local_port;
    std::string local_addr;
    int timeout{-1};
    int listen_timeout{-1};
    int send_buffer_size{-1};
    int recv_buffer_size{-1};
    int tcp_nodelay{0};
    int tcp_mss{-1};
};

class TCPSocket
{
    TCPSocket(const TCPSocketOption &option);
    ~TCPSocket();

    int open(const std::string &url);
    
private:
    struct Data;
    std::shared_ptr<Data> _d;

    std::shared_ptr<Data> init(const TCPSocketOption &option);
};