#include "packet_stable.h"

#include <vector>
#include <memory>
#include <thread>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <cstring>

#include <xlog.hpp>

using namespace std::chrono;
using Clock = steady_clock;
using Timepoint = Clock::time_point;
using Duration = Clock::duration;
using Unilock = std::unique_lock<std::mutex>;

struct packet_stable_packet_inner {
    packet_stable_timestamp_ms dts;
    std::vector<uint8_t> other_data;
};
using packet_stable_packet_inner_ptr = std::shared_ptr<packet_stable_packet_inner>;

enum class packet_stable_thread_state {
    RUN,
    EXIT,
};

struct packet_stable_ctx {
    struct packet_stable_param param_back;

    enum packet_stable_thread_state trd_state;
    std::shared_ptr<std::thread> trd;

    std::mutex mutex_queue;
    std::deque<packet_stable_packet_inner_ptr> queue;
    std::condition_variable cond_queue_over_low;

    std::condition_variable cond_need_wait;

    Timepoint last_dts; // 上一帧的解码时间
    packet_stable_timestamp_ms last_packet_ts; // 上一个包的时间戳
};

namespace {

void send_packet(struct packet_stable_ctx *ctx, packet_stable_packet_inner_ptr packet)
{
    if (ctx->param_back.cb) {
        struct packet_stable_packet pkt = {};
        pkt.dts = packet->dts;
        pkt.other_data = packet->other_data.data();
        pkt.other_data_size = packet->other_data.size();
        ctx->param_back.cb(&pkt, ctx->param_back.user);
    }
}

void trd_worker(struct packet_stable_ctx *ctx)
{
    constexpr auto tp_invalid = Timepoint::min();

    while (ctx->trd_state == packet_stable_thread_state::RUN) {
        Unilock lock(ctx->mutex_queue);

        xlog_dbg("wait low level...\n");
        while ( (ctx->trd_state == packet_stable_thread_state::RUN) 
                    && (ctx->queue.size() < ctx->param_back.queue_low_level)) {
            ctx->cond_queue_over_low.wait(lock);
        }

        xlog_dbg("start poping...\n");
        ctx->last_dts = tp_invalid; // re-start
        while ( (ctx->trd_state == packet_stable_thread_state::RUN) 
                    && !ctx->queue.empty()) {
            auto now = Clock::now();
            auto front = ctx->queue.front(); // must copy here
            ctx->queue.pop_front();

            if (ctx->last_dts == tp_invalid) {
                send_packet(ctx, front);
                ctx->last_dts = now;
                ctx->last_packet_ts = front->dts;
            } else {
                packet_stable_timestamp_ms dts_diff = front->dts - ctx->last_packet_ts;

                // xlog_dbg("dts_diff: %d\n", (int)dts_diff);
                if (dts_diff > ctx->param_back.max_jitter_ms) {
                    xlog_dbg("over jitter, reset\n");
                    break;
                }

                Timepoint tp_dts = ctx->last_dts + milliseconds(dts_diff);
                ctx->cond_need_wait.wait_until(lock, tp_dts);
                send_packet(ctx, front);
                ctx->last_dts = tp_dts;
                ctx->last_packet_ts = front->dts;
            }

        }
    }

    xlog_dbg("end\n");
}

}

packet_stable_handle packet_stable_new(struct packet_stable_param *param)
{
    packet_stable_ctx *ctx = nullptr;

    do {
        ctx = new packet_stable_ctx{};
        ctx->param_back = *param;

        ctx->trd_state = packet_stable_thread_state::RUN;
        ctx->trd = std::make_shared<std::thread>(trd_worker, ctx);
    } while (0);

    return (packet_stable_handle)ctx;
}

int packet_stable_destroy(packet_stable_handle obj)
{
    if (obj == packet_stable_handle_invalid) {
        xlog_err("null obj\n");
        return 0;
    }
    
    auto *ctx = reinterpret_cast<packet_stable_ctx*>(obj);
    ctx->trd_state = packet_stable_thread_state::EXIT;

    ctx->mutex_queue.lock(); // sync
    ctx->mutex_queue.unlock();
    ctx->cond_queue_over_low.notify_one();
    ctx->cond_need_wait.notify_one();
    xlog_dbg("join begin\n");
    ctx->trd->join();
    xlog_dbg("join end\n");
    return 0;
}

int packet_stable_push(packet_stable_handle obj, struct packet_stable_packet const* packet)
{
    int suc_flag = 0;
    auto *ctx = reinterpret_cast<packet_stable_ctx*>(obj);

    do {
        if (!ctx) {
            xlog_err("null obj\n");
            break;
        }

        Unilock lock(ctx->mutex_queue);
        if (ctx->queue.size() > ctx->param_back.queue_max) {
            xlog_err("queue full, reset!!!\n");
            ctx->queue.clear();
            ctx->last_packet_ts = 0;
            ctx->last_dts = Timepoint::min();
        }

        auto pkt_inner = std::make_shared<packet_stable_packet_inner>();
        pkt_inner->dts = packet->dts;
        pkt_inner->other_data.resize(packet->other_data_size);
        memcpy(pkt_inner->other_data.data(), packet->other_data, packet->other_data_size);

        // xlog_dbg("dts: %d\n", (int)pkt_inner->dts);

        ctx->queue.push_back(pkt_inner);
        if (ctx->queue.size() > ctx->param_back.queue_low_level) {
            ctx->cond_queue_over_low.notify_one();
        }

        suc_flag = 1;
    } while (0);

    return suc_flag ? 0 : -1;
}