#pragma once

#include <string>
#include <memory>

#include "audio_defs.hpp"
#include "print.hpp"

/**
 * 由于存在fread读取阻塞的隐患，因此
 * 需要减少读取次数降低风险，方式是
 * 一次全部加载到内存中。
 * 这里定义了最大内存占用限制。
*/
#define AFP_MAX_BUFFER_SIZE (1 * 1024 * 1024) 

struct AudioFileInfo
{
    /* 音频文件的路径 */
    std::string audio_file;

    /* 音频文件的编码格式；
       如果不是封装格式，则直接提供原
       始数据，否则解封装后提供原始数据 */
    AudioFileCodec codec;
};

class AudioFileParser
{
public:
    static std::shared_ptr<AudioFileParser> create(const AudioFileInfo &info);

    AudioFileParser(const AudioFileInfo &info);
    virtual ~AudioFileParser() = default;

    virtual int prepare() = 0;
    virtual int getContent(void *buf, int size) = 0;

protected:
    AudioFileInfo info_{};
};

