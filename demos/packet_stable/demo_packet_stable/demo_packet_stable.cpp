#include <stdio.h>

#include <chrono>
#include <thread>

#include "packet_gen.h"
#include "packet_stable.h"
#include "xlog.h"

static void packet_cb(struct packet_stable_packet const *packet, void *user)
{
    {
        using clock = std::chrono::steady_clock;
        using timepoint = clock::time_point;
        auto now = clock::now();
        
        static timepoint last_tp;
        auto dur = now - last_tp;
        last_tp = now;

        xlog_dbg("dts: {}, duration: %4d ms\n", 
            (packet_stable_timestamp_ms)packet->dts,
            (int)std::chrono::duration_cast<std::chrono::milliseconds>(dur).count());
    }
}

static void gen_cb(int ch, uint64_t time_ms, uint64_t packet_id, void *user)
{
    using clock = std::chrono::steady_clock;
    using timepoint = clock::time_point;

    {
        auto now = clock::now();
        
        static timepoint last_tp;
        auto dur = now - last_tp;
        last_tp = now;

        xlog_dbg("dts: {}, duration: %4d ms\n", 
            (packet_stable_timestamp_ms)time_ms,
            (int)std::chrono::duration_cast<std::chrono::milliseconds>(dur).count());
    }

    struct packet_stable_packet pkt = {};
    pkt.dts = time_ms;

    auto handle = reinterpret_cast<packet_stable_handle>(user);
    packet_stable_push(handle, &pkt);

    return ;
}

int main()
{
    struct packet_stable_param param = {};
    param.queue_max = 10;
    param.queue_low_level = 5;
    param.max_jitter_ms = 2000;
    param.user = NULL;
    param.cb = packet_cb;
    auto packet_stable = packet_stable_new(&param);

    struct packet_gen_conf gen_conf = {};
    gen_conf.buf_ms = 2000;
    gen_conf.channel_num = 1;
    gen_conf.packet_interval_ms = 50;
    gen_conf.jitter.jitter_possibility = 1000;
    gen_conf.jitter.max_jitter_ms = 200;
    gen_conf.delay_ms = 500;
    gen_conf.enable_out_order = 0;
    gen_conf.packet_cb = gen_cb;
    gen_conf.user = packet_stable;
    packet_gen_handle packet_gen = packet_gen_create(&gen_conf);
    packet_gen_start(packet_gen);

    for (int i = 0; i < 10; ++i) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    packet_gen_destroy(packet_gen);
    packet_stable_destroy(packet_stable);

    return 0;
}