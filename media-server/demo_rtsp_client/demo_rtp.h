#ifndef __DEMO_RTP_H__
#define __DEMO_RTP_H__

#include <stdint.h>

#include "sockutil.h"

void rtp_receiver_tcp_input(uint8_t channel, const void* data, uint16_t bytes);
void rtp_receiver_test(socket_t rtp[2], const char* peer, int peerport[2], int payload, const char* encoding);
void* rtp_receiver_tcp_test(uint8_t interleave1, uint8_t interleave2, int payload, const char* encoding);

#endif // __DEMO_RTP_H__
