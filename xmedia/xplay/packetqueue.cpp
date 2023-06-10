
#include "packetqueue.hpp"

#include "xlog.hpp"

typedef struct MyAVPacketList {
    AVPacket* pkt{ nullptr };
    int serial{ 0 };
} MyAVPacketList;

int PacketQueue::get(AVPacket* pkt, int block, int* serial)
{
    std::unique_lock<std::mutex> lock(mutex);

    MyAVPacketList pkt1;
    int ret = 0;

    for (;;)
    {
        if (abort_request)
        {
            ret = -1;
            break;
        }

        if (av_fifo_read(pkt_list, &pkt1, 1) >= 0)
        {
            nb_packets--;
            size -= pkt1.pkt->size + sizeof(pkt1);
            duration -= pkt1.pkt->duration;
            av_packet_move_ref(pkt, pkt1.pkt);
            if (serial)
            {
                *serial = pkt1.serial;
            }
            av_packet_free(&pkt1.pkt);
            ret = 1;

            xlog_trc("get packet, queue.nb=%d", nb_packets);

            break;
        }
        else if (!block)
        {
            ret = 0;
            break;
        }
        else
        {
            xlog_trc("packet queue empty, wait...");
            cond.wait(lock);
        }
    }
    return ret;
}

void PacketQueue::start()
{
    std::unique_lock<std::mutex> lock;
    abort_request = 0;
    serial++;
    xlog_trc("packetqueue.start,serial=%d", serial);
    return;
}

void PacketQueue::abort()
{
    std::unique_lock<std::mutex> lock;
    abort_request = 1;
    cond.notify_one();
    return;
}

void PacketQueue::destroy()
{
    flush();
    av_fifo_freep2(&pkt_list);
    return;
}

void PacketQueue::flush()
{
    std::unique_lock<std::mutex> lock;
    MyAVPacketList pkt1;

    while (av_fifo_read(pkt_list, &pkt1, 1) >= 0)
    {
        av_packet_free(&pkt1.pkt);
    }
    nb_packets = 0;
    size = 0;
    duration = 0;
    serial++;
    xlog_trc("packetqueue.flush(),serial=%d", serial);
    return;
}

int PacketQueue::init()
{
    pkt_list = av_fifo_alloc2(1, sizeof(MyAVPacketList), AV_FIFO_FLAG_AUTO_GROW);
    if (!pkt_list)
    {
        return AVERROR(ENOMEM);
    }
    abort_request = 1;
    return 0;
}

int PacketQueue::put_null_packet(AVPacket* pkt, int stream_index)
{
    pkt->stream_index = stream_index;
    return put(pkt);
}

int PacketQueue::put(AVPacket* pkt)
{
    AVPacket* pkt1{};
    int ret{};

    pkt1 = av_packet_alloc();
    if (!pkt1)
    {
        av_packet_unref(pkt);
        return -1;
    }
    av_packet_move_ref(pkt1, pkt);

    std::unique_lock<std::mutex> lock(mutex);
    ret = put_private(pkt1);
    lock.unlock();

    if (ret < 0)
    {
        av_packet_free(&pkt1);
    }
    return ret;
}

int PacketQueue::put_private(AVPacket* pkt)
{
    MyAVPacketList pkt1{};
    int ret{};

    if (abort_request)
    {
        return -1;
    }

    pkt1.pkt = pkt;
    pkt1.serial = serial;

    ret = av_fifo_write(pkt_list, &pkt1, 1);
    if (ret < 0)
    {
        return ret;
    }
    nb_packets++;
    size += pkt1.pkt->size + sizeof(pkt1);
    duration += pkt1.pkt->duration;
    cond.notify_one();

    xlog_trc("packet.put(),serial=%d,queue.nb=%d", serial, nb_packets);

    return 0;
}