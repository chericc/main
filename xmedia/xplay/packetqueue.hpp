#pragma once

#include "depends.hpp"


class PacketQueue
{
public:
    AVFifo* pkt_list{ nullptr };
    int nb_packets{ 0 };
    int size{ 0 };
    int64_t duration{ 0 };
    int abort_request{ 0 };
    int serial{ 0 };
    std::mutex mutex;
    std::condition_variable cond;

    int init();
    int put(AVPacket* pkt);
    int put_private(AVPacket* pkt);
    int put_null_packet(AVPacket* pkt, int stream_index);
    void flush();
    void destroy();
    void abort();
    void start();
    int get(AVPacket* pkt, int block, int* serial);
};