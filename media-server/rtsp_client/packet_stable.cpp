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

PacketStable::~PacketStable()
{
    Lock lock(_mutex_queue);
    _cond_over_low.notify_one();
    _cond_need_wait.notify_one();
    xlog_dbg("wait thread\n");
    _trd->join();
    xlog_dbg("wait thread end\n");
}

void PacketStable::push(const void *data, size_t bytes, uint32_t timestamp_ms)
{
    xlog_dbg("timestamp: %u\n", timestamp_ms);

    do {
        Lock lock(_mutex_queue);

        if (_queue.size() > QUEUE_MAX) {
            // old packets is cleared, no need to wait any more.
            // or will get old packets again(wait too long).
            xlog_err("!!!!!! queue full \n");
            _queue.clear();
            _last_ts_ms = 0;
            _last_dts = Clock::time_point::min();
            _cond_need_wait.notify_one();
            break;
        }

        if (!_queue.empty()) {
            if (_queue.back()->ts_ms == timestamp_ms) {
                _queue.back()->append(data, bytes);
                xlog_dbg("append: final size=%d\n", (int)_queue.back()->data.size());
                break;
            }
        }

        auto pkt = std::make_shared<Packet>(data, bytes, timestamp_ms);

        _queue.push_back(pkt);

        if (_queue.size() > QUEUE_LOW_LEVEL) {
            _cond_over_low.notify_one();
        }
    } while (0);

    return ;
}

void PacketStable::_trd_worker()
{
    while (_trd_run_flag) {
        Lock lock(_mutex_queue);

        xlog_dbg("waiting low level\n");

        while (_trd_run_flag && _queue.size() < QUEUE_LOW_LEVEL) {
            _cond_over_low.wait(lock);
        }

        xlog_dbg("start poping .....\n");

        while (_trd_run_flag && !_queue.empty()) {
            auto now = Clock::now();
            auto front = _queue.front();
            _queue.pop_front();
            if (_last_dts == Clock::time_point::min()) {
                _send_packet(front);
                _last_dts = now;
                _last_ts_ms = front->ts_ms;
            } else {
                auto ms_dts_diff = front->ts_ms - _last_ts_ms;
                Timepoint tp_dts = _last_dts + Ms(ms_dts_diff);
                auto wait_ms = std::chrono::duration_cast<std::chrono::milliseconds>(tp_dts - now).count();
                // xlog_dbg("wait %d ms(%d)\n", (int)wait_ms, front->ts_ms);
                _cond_need_wait.wait_until(lock, tp_dts);
                _send_packet(front);
                _last_dts = tp_dts;
                _last_ts_ms = front->ts_ms;
            }

            xlog_dbg("queue.size=%u\n", (int)_queue.size());
        }

        xlog_dbg("not enough packets\n");
    }
    
}

void PacketStable::_send_packet(PacketPtr const& packet)
{
    _cb(packet->data.data(), packet->data.size(), _user_data);
}