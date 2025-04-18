
#include "packet_gen.h"

#include <memory>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>

#include "xlog.hpp"

using unilock = std::unique_lock<std::mutex>;

enum thread_state {
    SLEEP,
    RUN,
    DEAD,
};

struct thread_ctx {
    int channel_id;

    enum thread_state state;
    std::mutex mutex_state;
    std::condition_variable cond_state_changed;
    std::condition_variable cond_state_activated;

    std::shared_ptr<std::thread> trd;
};

struct packet_gen_ctx {
    struct packet_gen_conf conf = {};
    std::vector<std::shared_ptr<thread_ctx>> trds;
};

namespace {

void trd_worker_push_packets(std::shared_ptr<thread_ctx> ctx)
{
    while (true) {
        switch (ctx->state) {
            case SLEEP: {
                unilock lock(ctx->mutex_state);
                if (ctx->state == SLEEP) {
                    ctx->cond_state_activated.notify_one();
                    ctx->cond_state_changed.wait(lock);
                }
                break;
            }
            case DEAD: {
                unilock lock(ctx->mutex_state);
                if (ctx->state == DEAD) {
                    ctx->cond_state_activated.notify_one();
                    goto dead;
                }
                break;
            }
            default: break;
        }

        std::this_thread::sleep_for(std::chrono::seconds(2));
        xlog_dbg("run %d\n", ctx->channel_id);
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
                    unilock lock(ctx->trds[i]->mutex_state);
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

int packet_gen_start(packet_gen_handle obj, packet_gen_cb cb)
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
                    unilock lock(ctx->trds[i]->mutex_state);
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
                    unilock lock(ctx->trds[i]->mutex_state);
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