#include "packet_stable.hpp"

#include <stdlib.h>
#include <string.h>

#include "xlog.hpp"

PacketStable::PacketStable(packet_stable_data_cb cb)
{
    _cb = cb;
}

void PacketStable::process(int ch, int payload, const void *data, size_t bytes, uint32_t timestamp, void *user_data)
{
    do {
        if (_queue.size() > 80) {
            xlog_err("queue full\n");
            break;
        }

        void *buffer = malloc(bytes);
        if (!buffer) {
            xlog_err("malloc failed\n");
            break;
        }

        std::shared_ptr<Packet> pkt_ptr = std::make_shared<Packet>();

        pkt_ptr->ch = ch;
        pkt_ptr->payload = payload;

        memcpy(buffer, data, bytes);
        
        pkt_ptr->buffer = buffer;
        pkt_ptr->size = bytes;
        pkt_ptr->timestamp = timestamp;
        pkt_ptr->user_data = user_data;

        // _queue.add(pkt_ptr);
    } while (0);
}