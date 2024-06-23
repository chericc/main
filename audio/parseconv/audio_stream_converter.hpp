#pragma once

#include <stdint.h>

#include <vector>

#include "audio_defs.hpp"

struct AudioStreamConverterConfig {
    AudioFileCodec src_codec;  // 源音频编码
    int src_sample_rate;       // 源采样率
    int src_sample_bits;       // 源采样位数，一般为 16bit

    AudioFileCodec dst_codec;  // 目标音频编码
    int dst_sample_rate;  // 目标采样率，如果和源不同，则需要重采样
    int dst_sample_bits;  // 不作支持，和源相同
};

/**
 * 当前需要支持的转换场景：
 * - PCM,8000Hz --> G711A
 * - G711A,8000Hz --> PCM
 * - PCM,16000Hz --> PCM,8000Hz
 *
 * - PCM,8000Hz --> AAC
 *
 * 缓存策略：
 * 尽可能少占用内存；
 */

class AudioStreamConverter {
   public:
    AudioStreamConverter(const AudioStreamConverterConfig& config);
    ~AudioStreamConverter() = default;

    /**
     * 输入：源数据及大小，目标缓存及大小；
     * 输出：目标缓存中填充当前已转换数据，目标大小中填充当前已转换数据的大小；
     * 注意：保证输入的完整性（比如输入为PCM数据，则必须输入整数个sample）；
     * 注意：确保输出缓存的大小充足；
     * 返回值：-1 错误，0 结束， 1 仍有数据
     */
    int convert(const void* src_data, int src_size, void* dst_data,
                int* dst_size);

   private:
    struct ConvertInfo;

    // PCM to PCM
    int PCM2PCM_Copy(const ConvertInfo& info);
    int PCM2PCM_Resample(const ConvertInfo& info);

    // from PCM
    int PCM2G711A(const ConvertInfo& info);
    int PCM2AAC(const ConvertInfo& info);

    // to PCM
    int G711A2PCM(const ConvertInfo& info);
    int AAC2PCM(const ConvertInfo& info);

   private:
    AudioStreamConverterConfig config_{};
};