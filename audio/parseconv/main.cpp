#include <cstdint>
#include <iostream>

#include "audio_file_parser.hpp"
#include "audio_stream_converter.hpp"

int parse_file(const char* filename) {
    AudioFileInfo info{};
    info.audio_file = std::string(filename);
    info.codec = ACODEC_UNKNOWN;

    auto afp = AudioFileParser::create(info);
    afp->prepare();

    FILE* fp = nullptr;
    fp = fopen("output.pcm", "w");

    while (true) {
        char buffer[64];
        int ret = afp->getContent(buffer, sizeof(buffer));
        if (ret > 0) {
            fwrite(buffer, 1, ret, fp);
        } else if (0 == ret) {
            break;
        } else {
            LOGE("get failed");
            break;
        }
    }

    if (fp) {
        fclose(fp);
        fp = nullptr;
    }

    return 0;
}

int parse_file_and_convert(const char* filename) {
    AudioStreamConverterConfig config{};

    config.src_codec = ACODEC_PCM;
    config.src_sample_rate = 16000;
    config.src_sample_bits = 16;

    config.dst_codec = ACODEC_PCM;
    config.dst_sample_rate = 8000;
    config.dst_sample_bits = 16;

    AudioStreamConverter asc{config};
    AudioFileInfo info{};

    info.audio_file = std::string(filename);
    info.codec = ACODEC_UNKNOWN;

    auto afp = AudioFileParser::create(info);
    afp->prepare();

    FILE* fp = nullptr;
    fp = fopen("output.pcm", "w");

    while (true) {
        char buffer[64];
        char outputbuffer[128];
        int ret = afp->getContent(buffer, sizeof(buffer));
        if (ret > 0) {
            int dst_size = sizeof(outputbuffer);
            int ret_convert = asc.convert(buffer, ret, outputbuffer, &dst_size);
            if (ret_convert >= 0) {
                LOGD("output:%d", dst_size);
                fwrite(outputbuffer, 1, dst_size, fp);
            }
        } else if (0 == ret) {
            break;
        } else {
            LOGE("get failed");
            break;
        }
    }

    if (fp) {
        fclose(fp);
        fp = nullptr;
    }

    return 0;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "usage:" << std::endl;
        return -1;
    }

    const char* filename = argv[1];
    // parse_file(filename);
    parse_file_and_convert(filename);

    return 0;
}