#pragma once

#include <stdint.h>
#include <stdio.h>

#include <vector>

#include "audio_file_parser.hpp"

class AudioFileParserRaw : public AudioFileParser {
   public:
    AudioFileParserRaw(const AudioFileInfo& info);
    ~AudioFileParserRaw();

    int prepare() override;
    int getContent(void* buf, int size) override;

   private:
    int read_size_{0};  // 已获取的数据
    std::vector<uint8_t> file_buffer_;
};
