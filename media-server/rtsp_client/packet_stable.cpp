#include "packet_stable.hpp"

#include <stdlib.h>
#include <string.h>

#include "xlog.hpp"

PacketStable::PacketStable(packet_stable_data_cb cb, void *user_data)
{
    _cb = cb;
    _user_data = user_data;

    auto trd = [&](){
        return this->_trd_worker();
    };

    _trd_run_flag = true;
    _trd = std::make_shared<std::thread>(trd);
}

void PacketStable::push(const void *data, size_t bytes, uint32_t timestamp_ms)
{
    do {
        Lock lock(_mutex_queue);

        if (_queue.size() > QUEUE_MAX) {
            xlog_err("queue full\n");
            break;
        }

        auto pkt = std::make_shared<Packet>(data, bytes, timestamp_ms);

        _queue.push_back(pkt);
        _cond_not_empty.notify_one();
    } while (0);

    return ;
}

void PacketStable::_trd_worker()
{
    while (true) {
        Lock lock(_mutex_queue);

        if (_queue.empty()) {
            // xlog_dbg("wait empty\n");
            _cond_not_empty.wait(lock);
            continue;
        }

        // xlog_dbg("not empty\n");

        auto front = _queue.front();

        // calculate dts

        // first packet
        if (_last_dts == Clock::time_point::min()) {
            xlog_dbg("first packet\n");
            _send_packet(front);
            _queue.pop_front();
            continue;
        } 

        // not first packet
        if (front->ts_ms >= _last_ts_ms) { // timestamp not reversed
            auto ms_dts_diff = front->ts_ms - _last_ts_ms;
            Timepoint tp_dts = _last_dts + Ms(ms_dts_diff);
            _cond_need_wait.wait_until(lock, tp_dts);
            _send_packet(front);
        } else { // timestamp reversed 2^32 / 90 / 1000 / 3600 = 13.26 hours
            xlog_dbg("timestamp reverse detected\n");
            // calculation is same as not reversed
            auto ms_dts_diff = front->ts_ms - _last_ts_ms;
            Timepoint tp_dts = _last_dts + Ms(ms_dts_diff);
            _cond_need_wait.wait_until(lock, tp_dts);
            _send_packet(front);
        }

        _queue.pop_front();

        // xlog_dbg("queue.size=%u\n", (int)_queue.size());
    }
    
}

void PacketStable::_send_packet(PacketPtr const& packet)
{
    _last_dts = Clock::now();
    _last_ts_ms = packet->ts_ms;
    _cb(packet->data.data(), packet->data.size(), _user_data);
}