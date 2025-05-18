
#include "packetqueue.hpp"

#include "xlog.h"

typedef struct MyAVPacketList {
    AVPacket* pkt{nullptr};
    int serial{0};
} MyAVPacketList;

PacketQueue::State::~State() {
    flush();

    av_fifo_freep2(&pkt_list);
    return;
}

void PacketQueue::State::flush() {
    MyAVPacketList pkt1;

    while (av_fifo_read(pkt_list, &pkt1, 1) >= 0) {
        av_packet_free(&pkt1.pkt);
    }

    nb_packets = 0;
    size = 0;
    duration = 0;
    serial++;
    xlog_trc("packetqueue.flush(),serial=%d", serial);
}

PacketQueue::PacketQueue() { _st = init(); }

bool PacketQueue::ok() {
    std::unique_lock<std::mutex> lock(mutex);
    return (_st ? true : false);
}

std::shared_ptr<PacketQueue::State> PacketQueue::init() {
    std::shared_ptr<State> st;
    bool berror = false;

    do {
        st = std::make_shared<State>();

        st->pkt_list =
            av_fifo_alloc2(1, sizeof(MyAVPacketList), AV_FIFO_FLAG_AUTO_GROW);
        if (!st->pkt_list) {
            berror = true;
            break;
        }
        st->abort_request = true;

    } while (0);

    if (berror) {
        xlog_err("error");
        st.reset();
    }

    return st;
}

int PacketQueue::get(AVPacket* pkt, int block, int* serial) {
    std::unique_lock<std::mutex> lock(mutex);

    if (!_st) {
        xlog_err("null");
        return -1;
    }

    MyAVPacketList pkt1;
    int ret = 0;

    for (;;) {
        if (_st->abort_request) {
            xlog_trc("abort");
            ret = -1;
            break;
        }

        if (av_fifo_read(_st->pkt_list, &pkt1, 1) >= 0) {
            _st->nb_packets--;
            _st->size -= pkt1.pkt->size + sizeof(pkt1);
            _st->duration -= pkt1.pkt->duration;
            av_packet_move_ref(pkt, pkt1.pkt);
            if (serial) {
                *serial = pkt1.serial;
            }
            av_packet_free(&pkt1.pkt);
            ret = 1;

            xlog_trc("get packet, queue.nb=%d", _st->nb_packets);

            break;
        } else if (!block) {
            ret = 0;
            break;
        } else {
            xlog_trc("packet queue empty, wait...");
            cond.wait(lock);
        }
    }
    return ret;
}

void PacketQueue::start() {
    std::unique_lock<std::mutex> lock;

    if (!_st) {
        xlog_err("null");
        return;
    }

    _st->abort_request = 0;
    _st->serial++;
    xlog_trc("packetqueue.start,serial=%d", _st->serial);
    return;
}

void PacketQueue::abort() {
    std::unique_lock<std::mutex> lock;

    if (!_st) {
        xlog_err("null");
        return;
    }

    _st->abort_request = 1;
    cond.notify_one();
    return;
}

void PacketQueue::flush() {
    std::unique_lock<std::mutex> lock;
    _st->flush();
    return;
}

int PacketQueue::put_null_packet(AVPacket* pkt, int stream_index) {
    std::unique_lock<std::mutex> lock;
    pkt->stream_index = stream_index;
    return do_put(pkt);
}

int PacketQueue::put(AVPacket* pkt) {
    std::unique_lock<std::mutex> lock;
    return do_put(pkt);
}

int PacketQueue::do_put(AVPacket* pkt) {
    AVPacket* pkt1{};
    int ret{};

    pkt1 = av_packet_alloc();
    if (!pkt1) {
        av_packet_unref(pkt);
        return -1;
    }
    av_packet_move_ref(pkt1, pkt);

    ret = put_private(pkt1);

    if (ret < 0) {
        av_packet_free(&pkt1);
    }
    return ret;
}

int PacketQueue::put_private(AVPacket* pkt) {
    MyAVPacketList pkt1{};
    int ret{};

    if (_st->abort_request) {
        return -1;
    }

    pkt1.pkt = pkt;
    pkt1.serial = _st->serial;

    ret = av_fifo_write(_st->pkt_list, &pkt1, 1);
    if (ret < 0) {
        return ret;
    }
    _st->nb_packets++;
    _st->size += pkt1.pkt->size + sizeof(pkt1);
    _st->duration += pkt1.pkt->duration;
    cond.notify_one();

    xlog_trc("packet.put(),serial=%d,queue.nb=%d", _st->serial,
             _st->nb_packets);

    return 0;
}

int PacketQueue::nb_packets() {
    std::unique_lock<std::mutex> lock;

    if (!_st) {
        xlog_err("null");
        return -1;
    }

    return _st->nb_packets;
}

bool PacketQueue::empty() {
    std::unique_lock<std::mutex> lock;

    if (!_st) {
        xlog_err("null");
        return -1;
    }

    return (!_st->nb_packets);
}

int PacketQueue::serial() {
    std::unique_lock<std::mutex> lock;

    if (!_st) {
        xlog_err("null");
        return -1;
    }

    return _st->serial;
}

int PacketQueue::size() {
    std::unique_lock<std::mutex> lock;

    if (!_st) {
        xlog_err("null");
        return -1;
    }

    return _st->size;
}

int64_t PacketQueue::duration() {
    std::unique_lock<std::mutex> lock;

    if (!_st) {
        xlog_err("null");
        return -1;
    }

    return _st->duration;
}

bool PacketQueue::abort_request() {
    std::unique_lock<std::mutex> lock;

    if (!_st) {
        xlog_err("null");
        return -1;
    }

    return _st->abort_request;
}