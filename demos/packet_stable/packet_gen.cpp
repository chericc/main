
#include "packet_gen.h"

#include <memory>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <random>

#include "xlog.hpp"

using UniClock = std::unique_lock<std::mutex>;
using Clock = std::chrono::steady_clock;
using Timepoint = std::chrono::time_point<Clock>;
using Duration = Clock::duration;

enum thread_state {
    SLEEP,
    RUN,
    DEAD,
};

struct packet {
    uint64_t packet_id;
    Timepoint dec_time; // decode time
    Timepoint out_time; // output time, can be affected by jitter
};

struct jitter_ctx {
    std::default_random_engine random_engine;
};

struct order_ctx {
    Timepoint last_packet_tp;
};

struct packet_map_time_ctx {
    bool sent_flag;
    Duration packet_time_to_real_dur;
};

struct thread_ctx {
    struct packet_gen_conf conf;

    int channel_id;

    enum thread_state state;
    std::mutex mutex_state;
    std::condition_variable cond_state_changed;
    std::condition_variable cond_state_activated;

    std::shared_ptr<std::thread> trd;

    uint64_t generated_packet_count;
    Timepoint last_new_packet_tp;

    struct jitter_ctx jitter_ctx;
    struct order_ctx order_ctx;
    struct packet_map_time_ctx map_time_ctx;
};

struct packet_gen_ctx {
    struct packet_gen_conf conf = {};
    std::vector<std::shared_ptr<thread_ctx>> trds;
};

namespace {

/*
生成正常的包
*/
struct packet packet_new(std::shared_ptr<thread_ctx> ctx)
{
    struct packet packet = {};
    packet.packet_id = ctx->generated_packet_count;
    
    packet.dec_time = ctx->last_new_packet_tp;
    packet.out_time = ctx->last_new_packet_tp;

    ctx->last_new_packet_tp += std::chrono::milliseconds(ctx->conf.packet_interval_ms);
    ++ctx->generated_packet_count;
    return packet;
}

/*
对每个包按顺序应用随机的jitter
*/
void packet_apply_jitter(std::shared_ptr<thread_ctx> ctx, struct packet *packet)
{
    std::uniform_int_distribution<int> prob_dist(PACKET_GEN_PROB_MIN, PACKET_GEN_PROB_MAX - 1);
    
    int prob_value = prob_dist(ctx->jitter_ctx.random_engine);
    if (prob_value < ctx->conf.jitter.jitter_possibility) {
        int jitter_ms = prob_value * ctx->conf.jitter.max_jitter_ms / ctx->conf.jitter.jitter_possibility;
        packet->out_time += std::chrono::milliseconds(jitter_ms);
    }
}

/*
对每个包应用固定的延迟
*/
void packet_apply_delay(std::shared_ptr<thread_ctx> ctx, struct packet *packet)
{
    packet->out_time += std::chrono::milliseconds(ctx->conf.delay_ms);
}

/*
乱序处理
确保所有的包
*/
void packet_apply_order(std::shared_ptr<thread_ctx> ctx, struct packet *packet)
{
    if (packet->out_time < ctx->order_ctx.last_packet_tp) {
        packet->out_time = ctx->order_ctx.last_packet_tp;
    } else {
        ctx->order_ctx.last_packet_tp = packet->out_time;
    }
}

Timepoint packet_map_to_real_time(std::shared_ptr<thread_ctx> ctx, struct packet const* packet)
{
    if (!ctx->map_time_ctx.sent_flag) {
        auto now = Clock::now();
        ctx->map_time_ctx.sent_flag = true;
        ctx->map_time_ctx.packet_time_to_real_dur = now - packet->out_time;
    }

    return packet->out_time + ctx->map_time_ctx.packet_time_to_real_dur;
}

void trd_worker_push_packets(std::shared_ptr<thread_ctx> ctx)
{
    while (true) {
        switch (ctx->state) {
            case SLEEP: {
                UniClock lock(ctx->mutex_state);
                if (ctx->state == SLEEP) {
                    ctx->cond_state_activated.notify_one();
                    ctx->cond_state_changed.wait(lock);
                }
                break;
            }
            case DEAD: {
                UniClock lock(ctx->mutex_state);
                if (ctx->state == DEAD) {
                    ctx->cond_state_activated.notify_one();
                    goto dead;
                }
                break;
            }
            default: break;
        }

        auto pkt = packet_new(ctx);
        packet_apply_delay(ctx, &pkt);
        packet_apply_jitter(ctx, &pkt);
        if (!ctx->conf.enable_out_order) {
            packet_apply_order(ctx, &pkt);
        }
        
        auto out_tp = packet_map_to_real_time(ctx, &pkt);
        std::this_thread::sleep_until(out_tp);
        if (ctx->conf.packet_cb) {
            auto time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(pkt.dec_time.time_since_epoch()).count();
            ctx->conf.packet_cb(ctx->channel_id, time_ms, pkt.packet_id, ctx->conf.user);
        }
        
    }

dead: 
    return ;
}

}

packet_gen_handle packet_gen_create(struct packet_gen_conf *conf)
{
    packet_gen_handle handle = packet_gen_handle_invalid;

    struct packet_gen_ctx *ctx = nullptr;
    do {
        ctx = new struct packet_gen_ctx;

        ctx->conf = *conf;

        for (int i = 0; i < ctx->conf.channel_num; ++i) {
            auto trd_ctx = std::make_shared<thread_ctx>();
            trd_ctx->state = SLEEP;
            trd_ctx->channel_id = i;
            trd_ctx->conf = *conf;
            trd_ctx->trd = std::make_shared<std::thread>(trd_worker_push_packets, trd_ctx);
            ctx->trds.push_back(trd_ctx);
        }
    } while (0);

    if (ctx) {
        handle = static_cast<packet_gen_handle>(ctx);
    }
    return handle;
}

int packet_gen_destroy(packet_gen_handle obj)
{
    auto *ctx = static_cast<packet_gen_ctx*>(obj);
    do {
        if (!ctx) {
            xlog_err("null obj\n");
            break;
        }

        for (int i = 0; i < (int)ctx->trds.size(); ++i) {
            if (ctx->trds[i]->trd->joinable()) {
                if (ctx->trds[i]->state == SLEEP) {
                    UniClock lock(ctx->trds[i]->mutex_state);
                    ctx->trds[i]->state = DEAD;
                    ctx->trds[i]->cond_state_changed.notify_one();
                } else {
                    ctx->trds[i]->state = DEAD;
                }
                xlog_dbg("join trd begin: %d\n", i);
                ctx->trds[i]->trd->join();
                xlog_dbg("join trd end: %d\n", i);
            }
        }
    } while (0);
    return 0;
}

int packet_gen_start(packet_gen_handle obj)
{
    auto *ctx = static_cast<packet_gen_ctx*>(obj);
    do {
        if (!ctx) {
            xlog_err("null obj\n");
            break;
        }

        for (int i = 0; i < (int)ctx->trds.size(); ++i) {
            if (ctx->trds[i]->trd->joinable()) {
                if (ctx->trds[i]->state == SLEEP) {
                    UniClock lock(ctx->trds[i]->mutex_state);
                    ctx->trds[i]->state = RUN;
                    ctx->trds[i]->cond_state_changed.notify_one();
                } else if (ctx->trds[i]->state == RUN) {
                    xlog_dbg("already running\n");
                } else if (ctx->trds[i]->state == DEAD) {
                    xlog_err("already dead\n");
                } else {
                    xlog_err("not handled\n");
                }
            }
        }
    } while (0);
    return 0;
}

int packet_gen_stop(packet_gen_handle obj)
{
    auto *ctx = static_cast<packet_gen_ctx*>(obj);
    do {
        if (!ctx) {
            xlog_err("null obj\n");
            break;
        }

        for (int i = 0; i < (int)ctx->trds.size(); ++i) {
            if (ctx->trds[i]->trd->joinable()) {
                if (ctx->trds[i]->state == SLEEP) {
                    xlog_dbg("already stopped\n");
                } else if (ctx->trds[i]->state == RUN) {
                    UniClock lock(ctx->trds[i]->mutex_state);
                    ctx->trds[i]->state = SLEEP;
                    xlog_dbg("wait stop begin: %d\n", i);
                    ctx->trds[i]->cond_state_activated.wait(lock);
                    xlog_dbg("wait stop end: %d\n", i);
                } else if (ctx->trds[i]->state == DEAD) {
                    xlog_err("already dead\n");
                } else {
                    xlog_err("not handled\n");
                }
            }
        }
    } while (0);
    return 0;
}