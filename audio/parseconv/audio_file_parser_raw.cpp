#include "audio_file_parser_raw.hpp"

#include <string.h>

AudioFileParserRaw::AudioFileParserRaw(const AudioFileInfo& info)
    : AudioFileParser(info) {}

AudioFileParserRaw::~AudioFileParserRaw() {}

int AudioFileParserRaw::prepare() {
    bool error_flag = false;

    long file_size = 0;
    int ret = 0;
    FILE* fp = nullptr;

    do {
        fp = fopen(info_.audio_file.c_str(), "r");
        if (!fp) {
            LOGE("open file failed");
            error_flag = true;
            break;
        }

        fseek(fp, 0, SEEK_END);
        file_size = ftell(fp);
        if (file_size <= 0) {
            LOGE("file size not valid(%ld)", file_size);
            error_flag = true;
            break;
        }

        if (file_size > AFP_MAX_BUFFER_SIZE) {
            LOGE("file size over, trimmed(%ld->%d)", file_size,
                 AFP_MAX_BUFFER_SIZE);
            file_size = AFP_MAX_BUFFER_SIZE;
        }

        rewind(fp);

        LOGD("load file to mem, allocating:%ld", file_size);
        file_buffer_.clear();
        file_buffer_.shrink_to_fit();
        file_buffer_.resize(file_size);

        ret = fread(file_buffer_.data(), 1, file_size, fp);
        if (ret != file_size) {
            LOGE("fread failed");
            error_flag = true;
            break;
        }
    } while (0);

    if (fp) {
        fclose(fp);
        fp = nullptr;
    }

    // 可以多次读取
    read_size_ = 0;

    return (error_flag ? -1 : 0);
}

int AudioFileParserRaw::getContent(void* buf, int size) {
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