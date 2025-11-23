#include "aac.hpp"

#include <array>
#include <cstdint>

#include "xlog.h"

namespace {

int get_sample_rate_index(int sample_rate)
{
    constexpr const std::array<int, 13> sr_table = {
        96000, 88200, 64000, 48000, 44100, 32000,
        24000, 22050, 16000, 12000, 11025, 8000,
        7350
    };
    for (int i = 0; i < static_cast<int>(sr_table.size()); i++) {
        if (sr_table[i] == sample_rate) {
            return i;
        }
    }
    return -1;
}

}


bool aac_avpacket_to_adts(const AVCodecParameters *par,
                         const AVPacket *pkt,
                         std::vector<uint8_t> &adts)
{
    if (par->codec_id != AV_CODEC_ID_AAC) {
        xlog_err("not aac");
        return false;
    }

    int profile = par->profile;
    int adts_profile = profile; // 0: Main, 1: LC, 2: SSR
    if (profile == FF_PROFILE_AAC_MAIN) {
        adts_profile = 0;
    } else if (profile == FF_PROFILE_AAC_LOW) {
        adts_profile = 1; 
    } else if (profile == FF_PROFILE_AAC_SSR) {
        adts_profile = 2;
    } else if (profile == FF_PROFILE_AAC_LTP) {
        adts_profile = 3;
    } else {
        xlog_err("profile not support: %d", profile);
        return false;
    }

    xlog_dbg("sample rate: %d", par->sample_rate);

    int sr_index = get_sample_rate_index(par->sample_rate);
    if (sr_index < 0) {
        xlog_err("sample rate not supported: %d", par->sample_rate);
        return false;
    }

    int channels = par->ch_layout.nb_channels;
    if (channels < 1 || channels > 7) {
        xlog_err("channel count not supported: %d", channels);
        return false;
    }

    constexpr int adts_header_size = 7;
    int frame_length = pkt->size + adts_header_size;

    adts.resize(frame_length);

    adts[0] = 0xFF;
    adts[1] = 0xF9; // 1111 1 00 1: MPEG-4, Layer=0, no CRC
    // adts[1] = 0xF8; // 1111 1 00 1: MPEG-4, Layer=0, no CRC

    adts[2] = ((adts_profile & 0x03) << 6) |
              ((sr_index & 0x0F) << 2) |
              ((channels >> 2) & 0x01); // private_bit = 0

    adts[3] = ((channels & 0x03) << 6) |
              ((frame_length >> 11) & 0x03);

    adts[4] = (frame_length >> 3) & 0xFF;
    adts[5] = ((frame_length & 0x07) << 5) | 0x1F; // buffer fullness high part
    adts[6] = 0xFC; // buffer fullness low (0x3F << 2) | 0

    memcpy(adts.data() + adts_header_size, pkt->data, pkt->size);
    return true;
}