/*

模拟生成包

*/

#ifndef __PACKET_GEN_H__
#define __PACKET_GEN_H__

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdint.h>

struct packet_gen_jitter_conf {
    int max_jitter_ms;  // 最大抖动时长
    int jitter_possibility; // 抖动可能性[0-10000]
};

struct packet_gen_conf {
    int buf_ms; // 缓冲时长
    int channel_num;

    struct packet_gen_jitter_conf jitter;
};

#define packet_gen_handle_invalid (NULL)
typedef void* packet_gen_handle;
packet_gen_handle packet_gen_create(struct packet_gen_conf *conf);
int packet_gen_destroy(packet_gen_handle obj);

typedef void (*packet_gen_cb)(int ch, uint64_t time_ms, uint64_t packet_id);
int packet_gen_start(packet_gen_handle obj, packet_gen_cb cb);
int packet_gen_stop(packet_gen_handle obj);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __PACKET_GEN_H__