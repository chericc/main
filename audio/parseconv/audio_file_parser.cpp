#include "audio_file_parser.hpp"

#include <string.h>

#include "audio_file_parser_raw.hpp"
#include "audio_file_parser_wav.hpp"

std::shared_ptr<AudioFileParser> AudioFileParser::create(const AudioFileInfo &info)
{
    std::shared_ptr<AudioFileParser> audio_file_parser{nullptr};

    switch (info.codec)
    {
        case ACODEC_PCM:
        case ACODEC_G711A:
        case ACODEC_G711U:
        case ACODEC_AAC:
        {
            audio_file_parser = std::make_shared<AudioFileParserRaw>(info);
            break;
        }
        case ACODEC_UNKNOWN:
        {
            if (strstr(info.audio_file.c_str(), ".wav"))
            {
                audio_file_parser = std::make_shared<AudioFileParserWav>(info);
            }
            else if (strstr(info.audio_file.c_str(), ".pcm")
                || strstr(info.audio_file.c_str(), ".g711a"))
            {
                audio_file_parser = std::make_shared<AudioFileParserRaw>(info);
            }
            else
            {
                LOGE("filename not support");
            }
            break;
        }
        default:
        {
            LOGE("format not support");
            break;
        }
    }

    return audio_file_parser;
}

AudioFileParser::AudioFileParser(const AudioFileInfo &info)
        : info_{info}
{

}