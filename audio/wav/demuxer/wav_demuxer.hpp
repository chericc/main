#pragma once

#include <string>
#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <memory>
#include "xio.hpp"

/* 参考 wav/fmt: ffmpeg/libavformat/riff.c:ff_codec_wav_tags */
enum class RiffFmt
{
    PCM = 0x1,
    PCM_ALAW = 0x6,
    PCM_MULAW = 0x7,
};

class WavDemuxer
{
private:

    struct WAV_TAG
    {
        uint32_t chunkid;
        uint32_t chunksize;
    };

    struct WAV_RIFF
    {
        WAV_TAG tag;
        uint32_t format;
    };

    struct WAV_SubChunkFmt
    {
        WAV_TAG tag;
        uint16_t audio_format;
        uint16_t num_channels;
        uint32_t sample_rate;
        uint32_t byte_rate;
        uint16_t align;
        uint16_t bits_per_sample;
    };

    struct WAV_SubChunkData
    {
        WAV_TAG tag;
    };

    class LoadInfo
    {
    public:
        WAV_RIFF        riff{};
        WAV_SubChunkFmt fmt{};

        // FILE *fp{nullptr};
        std::shared_ptr<XIO> xio;

        size_t data_offset{};   // start of audio data
        size_t data_size{};     // size of audio data
    };
public:
    WavDemuxer(const std::string &filename);
    ~WavDemuxer();
    bool loadSuccessful();
    int format(); // RiffFmt
    int numChannels();
    int sampleRate();
    int sampleBits();
    int numSamples();
    std::vector<uint8_t> getSamples(int ch, int pos, int count);
private:
    int doLoadFile();
    int doCloseFile();

    /* read header into mem */
    int readHeader(std::shared_ptr<LoadInfo> info);

    const std::string _filename;
    std::shared_ptr<LoadInfo> _load_info;
};