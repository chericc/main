#ifndef __PACKET_STABLE_H__
#define __PACKET_STABLE_H__

#include <vector>
#include <deque>
#include <memory>
#include <chrono>
#include <cstring>
#include <mutex>
#include <condition_variable>
#include <thread>

typedef void (*packet_stable_data_cb)(const void *data, size_t bytes, void *user_data);

struct Packet 
{
    using clock_ = std::chrono::steady_clock;

    std::vector<uint8_t> data;
    // clock_::time_point dts;
    uint32_t ts_ms;

    Packet(const void *data_, size_t bytes_, uint32_t ts_ms_) {
        data.resize(bytes_);
        memcpy(data.data(), data_, bytes_);
        ts_ms = ts_ms_;
    };
    ~Packet() = default;
};

class PacketStable
{
public:
    const size_t QUEUE_MAX = 40;
    const size_t QUEUE_LOW_LEVEL = 20;
public:
    PacketStable(packet_stable_data_cb cb, void *user_data);
    ~PacketStable();

    // notes: packets should already be sorted with timestamp
    // (i.e. timestamp_ms always forward)
    void push(const void *data, size_t bytes, uint32_t timestamp_ms);
private:
    using PacketPtr = std::shared_ptr<Packet>;
    using Lock = std::unique_lock<std::mutex>;
    
    void _send_packet(const PacketPtr &packet);
    void _trd_worker();


    packet_stable_data_cb _cb = nullptr;
    void *_user_data = nullptr;

    std::mutex _mutex_queue;
    std::deque<PacketPtr> _queue;
    std::condition_variable _cond_over_low;
    std::condition_variable _cond_need_wait;

    bool _trd_run_flag = false;
    std::shared_ptr<std::thread> _trd = nullptr;

    using Clock = Packet::clock_;
    using Timepoint = Clock::time_point;
    using Ms = std::chrono::milliseconds;
    Timepoint _last_dts = Timepoint::min(); // 最后一次输出的帧的解码时间
    uint32_t _last_ts_ms = 0; // 最后一次输出的帧的时间戳
};

#endif // __PACKET_STABLE_H__