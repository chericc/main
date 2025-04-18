#include "packet_gen.h"

#include <thread>

#include <xlog.hpp>

static void gen_cb(int ch, uint64_t time_ms, uint64_t packet_id)
{
    return ;
}

int main()
{
    struct packet_gen_conf gen_conf = {};
    gen_conf.buf_ms = 2000;
    gen_conf.channel_num = 2;
    gen_conf.jitter.jitter_possibility = 5;
    gen_conf.jitter.max_jitter_ms = 200;
    packet_gen_handle packet_gen = packet_gen_create(&gen_conf);
    xlog_dbg("packet_gen_create: %p\n", packet_gen);

    xlog_dbg("packet_gen_start\n");
    packet_gen_start(packet_gen, gen_cb);

    
    xlog_dbg("wait\n");
    std::this_thread::sleep_for(std::chrono::seconds(3));
    xlog_dbg("wait end\n");

    xlog_dbg("packet_gen_destroy\n");
    packet_gen_destroy(packet_gen);

    return 0;
}
