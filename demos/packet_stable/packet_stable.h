#ifndef __PACKET_STABLE_H__
#define __PACKET_STABLE_H__

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stddef.h>
#include <stdint.h>

#define packet_stable_handle_invalid (NULL)
typedef void* packet_stable_handle;

// 为了避免翻转判断不一致，要求time类型大小和实际使用的时间类型大小一致(uint32 --> uint32, ...)
typedef uint16_t packet_stable_timestamp_ms;

struct packet_stable_packet {
    // 这里要求输入的帧已经实现了排序，即靠前传入的帧的解码时间一定不在后续传入的帧的解码时间之后
    packet_stable_timestamp_ms dts;
    void *other_data;
    size_t other_data_size;
};
typedef void (*packet_stable_cb)(struct packet_stable_packet const *packet, void *user);

struct packet_stable_param {
    size_t queue_max;
    size_t queue_low_level;
    int max_jitter_ms;    // 如果两个帧的时间差距很大，则直接丢弃已有的缓存
    packet_stable_cb cb;
    void *user;
};
packet_stable_handle packet_stable_new(struct packet_stable_param *param);
int packet_stable_destroy(packet_stable_handle obj);
int packet_stable_push(packet_stable_handle obj, struct packet_stable_packet const* packet);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __PACKET_STABLE_H__