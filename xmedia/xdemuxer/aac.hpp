#pragma once

#include <vector>

extern "C" {
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavcodec/avcodec.h"
#include "libavutil/log.h"
#include "libavutil/common.h"
}

bool aac_avpacket_to_adts(const AVCodecParameters *par,
                         const AVPacket *pkt,
                         std::vector<uint8_t> &buf);