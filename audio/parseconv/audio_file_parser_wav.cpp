#include "audio_file_parser_wav.hpp"

#include <string.h>

#include "wave_reader.h"

AudioFileParserWav::AudioFileParserWav(const AudioFileInfo& info)
    : AudioFileParser(info) {}

AudioFileParserWav::~AudioFileParserWav() {}

int AudioFileParserWav::prepare() {
    wave_reader* reader = nullptr;
    wave_reader_error error = WR_NO_ERROR;
    int audio_size = 0;         // 音频数据的大小
    uint8_t sample_buffer[16];  // 确保足够缓存一个sample即可

    int read_size = 0;    // 当前已读取的大小
    int sample_size = 0;  // 一个采样点的大小

    int ret = 0;

    do {
        reader = wave_reader_open(info_.audio_file.c_str(), &error);
        if (nullptr == reader) {
            LOGE("open file failed");
            break;
        }

        channels_ = wave_reader_get_num_channels(reader);
        sample_bits_ = wave_reader_get_sample_bits(reader);
        format_ = wave_reader_get_format(reader);
        sample_rates_ = wave_reader_get_sample_rate(reader);
        num_samples_ = wave_reader_get_num_samples(reader);
        sample_size = sample_bits_ / 8;

        audio_size = sample_size * num_samples_;

        file_buffer_.clear();
        file_buffer_.shrink_to_fit();
        file_buffer_.resize(audio_size);

        /* 对sample大小的校验 */
        if (sample_bits_ > (int)sizeof(sample_buffer) * 8) {
            LOGE("sample too big");
            break;
        }

        /* 取出所有内容，并存储到缓存中 */
        while (true) {
            if (audio_size <= read_size) {
                break;
            }

            int left_size = audio_size - read_size;  // 缓存剩余多少
            uint8_t* writep = file_buffer_.data() + read_size;  // 写指针

            if (sample_size <= 0) {
                LOGE("invalid sample size");
                break;
            }

            ret = wave_reader_get_samples(reader, 1, sample_buffer);
            if (ret > 0) {
                if (sample_size <= left_size) {
                    memcpy(writep, sample_buffer, sample_size);
                    read_size += sample_size;
                } else {
                    LOGE("buffer full");
                    break;
                }
            } else if (0 == ret) {
                break;
            } else {
                LOGE("get samples failed");
                break;
            }
        }
    } while (0);

    if (read_size != audio_size) {
        LOGE("read size not equal to audio size(%d,%d)", read_size, audio_size);
    }

    file_buffer_.resize(read_size);

    if (reader != nullptr) {
        wave_reader_close(reader);
        reader = nullptr;
    }

    return -1;
}

int AudioFileParserWav::getContent(void* buf, int size) {
    int buffer_size = 0;       // 总的数据大小
    int left_size = 0;         // 剩余数据大小
    uint8_t* readp = nullptr;  // 读指针
    int ret_size = 0;          // 本次读取的数据大小

    readp = file_buffer_.data() + read_size_;

    buffer_size = file_buffer_.size();

    if (buffer_size > read_size_) {
        left_size = buffer_size - read_size_;

        if (size <= left_size) {
            memcpy(buf, readp, size);
            read_size_ += size;
            ret_size = size;
        } else {
            memcpy(buf, readp, left_size);
            read_size_ += left_size;
            ret_size = left_size;
        }
    }

    return ret_size;
}