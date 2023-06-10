#pragma once

#include "depends.hpp"
#include "frame.hpp"
#include "packetqueue.hpp"

class FrameQueue
{
public:
    std::vector<Frame> queue;
    int rindex{ 0 };
    int windex{ 0 };
    int size{ 0 };
    int max_size{ 0 };
    int keep_last{ 0 };
    int rindex_shown{ 0 };
    std::mutex mutex;
    std::condition_variable cond;
    std::shared_ptr<PacketQueue> pktq;

    void unref_item(Frame* vp);
    int init(std::shared_ptr<PacketQueue> pktq, int max_size, int keep_last);
    void destroy();
    void signal();
    Frame* peek();
    Frame* peek_next();
    Frame* peek_last();
    Frame* peek_writable();
    Frame* peek_readable();
    void push();
    void next();
    int nb_remaining();
    int64_t last_pos();
};