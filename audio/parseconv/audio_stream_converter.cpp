#include "audio_stream_converter.hpp"

#include <string.h>

#include "print.hpp"

#define EQUAL(a, b) ((a) == (b))

struct AudioStreamConverter::ConvertInfo {
    const void* src_data;
    int src_size;
    void* dst_data;
    int* dst_size;
};

AudioStreamConverter::AudioStreamConverter(
    const AudioStreamConverterConfig& config)
    : config_(config) {}

int AudioStreamConverter::convert(const void* src_data, int src_size,
                                  void* dst_data, int* dst_size) {
    int ret = 0;

    ConvertInfo info{};
    info.src_data = src_data;
    info.src_size = src_size;
    info.dst_data = dst_data;
    info.dst_size = dst_size;

    if (nullptr == info.src_data || nullptr == info.dst_data) {
        LOGE("null pointer");
        return -1;
    }

    if (0 == config_.src_sample_bits || 0 == config_.dst_sample_bits) {
        LOGE("sample bits invalid");
        return -1;
    }

    do {
        if (!EQUAL(config_.src_sample_bits, config_.dst_sample_bits)) {
            LOGE("sample bits not equal");
            ret = -1;
            break;
        }

        if (EQUAL(config_.src_codec, ACODEC_PCM) &&
            EQUAL(config_.dst_codec, ACODEC_PCM)) {
            if (EQUAL(config_.src_sample_rate, config_.dst_sample_rate)) {
                ret = PCM2PCM_Copy(info);
            } else {
                ret = PCM2PCM_Resample(info);
            }
        } else if (EQUAL(config_.src_codec, ACODEC_PCM)) {
            if (!EQUAL(config_.src_sample_rate, config_.dst_sample_rate)) {
                LOGE("sample rate not equal");
                break;
            }

            if (EQUAL(config_.dst_codec, ACODEC_G711A)) {
                ret = PCM2G711A(info);
            } else if (EQUAL(config_.dst_codec, ACODEC_AAC)) {
                ret = PCM2AAC(info);
            } else {
                LOGE("convert not support");
                ret = -1;
                break;
            }
        } else if (EQUAL(config_.dst_codec, ACODEC_PCM)) {
            if (!EQUAL(config_.src_sample_rate, config_.dst_sample_rate)) {
                LOGE("sample rate not equal");
                return -1;
                break;
            }

            if (EQUAL(config_.src_codec, ACODEC_G711A)) {
                ret = G711A2PCM(info);
            } else if (EQUAL(config_.src_codec, ACODEC_AAC)) {
                ret = AAC2PCM(info);
            } else {
                LOGE("convert not support");
                ret = -1;
                break;
            }
        } else {
            LOGE("convert not support");
            ret = -1;
            break;
        }
    } while (0);

    return ret;
}

int AudioStreamConverter::PCM2PCM_Copy(const ConvertInfo& info) {
    int dst_size = *info.dst_size;
    int transfer_bytes = std::min(info.src_size, dst_size);

    if (transfer_bytes < info.src_size) {
        LOGE("data loss");
    }

    memcpy(info.dst_data, info.src_data, transfer_bytes);
    *info.dst_size = transfer_bytes;

    return 0;
}

int AudioStreamConverter::PCM2PCM_Resample(const ConvertInfo& info) {
    do {
        int dst_size = *info.dst_size;

        int transfer_samples = 0;
        uint8_t* dst_data = (uint8_t*)info.dst_data;
        uint8_t* src_data = (uint8_t*)info.src_data;

        int sample_size = config_.dst_sample_bits / 8;
        int src_sample_num = 0;
        int dst_sample_num_max = 0;

        /* 2倍降采样 */
        const int DOWN_SCALE_2 = 2;

        if (sample_size <= 0) {
            LOGE("invalid sample size");
            break;
        }

        src_sample_num = info.src_size / sample_size;
        dst_sample_num_max = dst_size / sample_size;

        static_assert(DOWN_SCALE_2 != 0, "down scale can't be 0");

        if (EQUAL(config_.dst_sample_rate * DOWN_SCALE_2,
                  config_.src_sample_rate)) {
            int i = 0;
            for (i = 0;
                 i < src_sample_num && transfer_samples < dst_sample_num_max;
                 ++i) {
                if (0 == i % DOWN_SCALE_2) {
                    AUDIO_ASSERT(transfer_samples * sample_size < dst_size);
                    AUDIO_ASSERT(i * sample_size < info.src_size);

                    memcpy(dst_data + transfer_samples * sample_size,
                           src_data + i * sample_size, sample_size);
                    ++transfer_samples;
                }
            }

            if (i < src_sample_num) {
                LOGE("data loss");
            }

            *info.dst_size = transfer_samples * sample_size;
            break;
        }
    } while (0);

    return 0;
}

int AudioStreamConverter::PCM2G711A(const ConvertInfo& info) { return 0; }

int AudioStreamConverter::PCM2AAC(const ConvertInfo& info) { return 0; }

int AudioStreamConverter::G711A2PCM(const ConvertInfo& info) { return 0; }

int AudioStreamConverter::AAC2PCM(const ConvertInfo& info) { return 0; }