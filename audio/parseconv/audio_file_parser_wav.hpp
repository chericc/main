#pragma once

#include <stdint.h>

#include <vector>

#include "audio_file_parser.hpp"

typedef struct wave_reader wave_reader;

class AudioFileParserWav : public AudioFileParser {
   public:
    AudioFileParserWav(const AudioFileInfo& info);
    ~AudioFileParserWav();

    int prepare() override;
    int getContent(void* buf, int size) override;

    // additional info of wav
    int getChannels();
    int getSampleBits();
    int getFormat();
    int getSampleRates();
    int getNumSamples();

   private:
    std::vector<uint8_t> file_buffer_;
    int read_size_{0};  // 已获取的数据

    int channels_{0};
    int sample_bits_{0};
    int format_{0};
    int sample_rates_{0};
    int num_samples_{0};
};
