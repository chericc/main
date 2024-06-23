#pragma once

#include "depends.hpp"

class PacketQueue {
   public:
    PacketQueue();
    ~PacketQueue() = default;
    bool ok();

    int put(AVPacket* pkt);
    int put_null_packet(AVPacket* pkt, int stream_index);
    void flush();
    void abort();
    void start();
    int get(AVPacket* pkt, int block, int* serial);

    int nb_packets();
    bool empty();
    int serial();
    int size();
    int64_t duration();
    bool abort_request();

   private:
    class State {
       public:
        AVFifo* pkt_list{nullptr};
        int nb_packets{0};
        int size{0};
        int64_t duration{0};
        int abort_request{0};
        int serial{0};

        void flush();
        ~State();
        State() = default;
    };

    std::shared_ptr<State> _st;
    std::mutex mutex;
    std::condition_variable cond;

   private:
    std::shared_ptr<State> init();
    int do_put(AVPacket* pkt);
    int put_private(AVPacket* pkt);
};