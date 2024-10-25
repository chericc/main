#ifndef __PACKET_STABLE_H__
#define __PACKET_STABLE_H__

#include <stdint.h>
#include <stddef.h>
#include <deque>
#include <memory>

typedef void (*packet_stable_data_cb)(const void *data, size_t bytes, uint32_t timestamp, void *user_data);


struct Packet 
{
    int ch = 0;
    int payload = 0;
    void *buffer = nullptr;
    size_t size = 0;
    uint32_t timestamp = 0;
    void *user_data = nullptr;

    ~Packet() {
        if (buffer) {
            free(buffer);
            buffer = nullptr;
        }
    }
};

class PacketStable
{
public:
    PacketStable(packet_stable_data_cb cb);
    void process(int ch, int payload, const void *data, size_t bytes, uint32_t timestamp, void *user_data);
private:
    packet_stable_data_cb _cb = nullptr;
    std::deque<Packet> _queue;
};

#endif // __PACKET_STABLE_H__