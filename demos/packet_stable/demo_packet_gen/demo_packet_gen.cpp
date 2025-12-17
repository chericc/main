#include "packet_gen.h"

#include <thread>
#include <chrono>

#include "xlog.h"

static void gen_cb(int ch, uint64_t time_ms, uint64_t packet_id, void *user)
{
    using clock = std::chrono::steady_clock;
    using timepoint = clock::time_point;

    {
        auto now = clock::now();
        
        static timepoint last_tp;
        auto dur = now - last_tp;
        last_tp = now;

        xlog_dbg("duration: {:4d} ms", (int)std::chrono::duration_cast
            <std::chrono::milliseconds>(dur).count());
    }

    return ;
}

int main()
{
    struct packet_gen_conf gen_conf = {};
    gen_conf.buf_ms = 2000;
    gen_conf.channel_num = 1;
    gen_conf.packet_interval_ms = 50;
    gen_conf.jitter.jitter_possibility = 5;
    gen_conf.jitter.max_jitter_ms = 200;
    gen_conf.delay_ms = 500;
    gen_conf.enable_out_order = 0;
    gen_conf.packet_cb = gen_cb;
    packet_gen_handle packet_gen = packet_gen_create(&gen_conf);
    xlog_dbg("packet_gen_create: {}", packet_gen);

    xlog_dbg("packet_gen_start");
    packet_gen_start(packet_gen);

    xlog_dbg("wait");
    std::this_thread::sleep_for(std::chrono::seconds(3));
    xlog_dbg("wait end");

    xlog_dbg("packet_gen_destroy");
    packet_gen_destroy(packet_gen);

    return 0;
}
