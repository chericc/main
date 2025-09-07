#include <cerrno>
#include <gtest/gtest.h>

#include <list>

#include "test_comm.hpp"
#include "wav_def.h"
#include "wav_demuxer.h"
#include "xlog.h"

struct TestDemux {
    std::string file;
    std::string file_raw;

    int format;
    int channel;
    int samplerate;
    int samplebits;
    int samples;
};

static void testExtractRaw(const std::string& wavFile,
                           const std::string& rawFile, int format, int channels,
                           int samplerate, int samplebits, int samples) {
    FILE *fp = nullptr;
    wav_demuxer_handle demuxer = wav_demuxer_handle_invalid;
    do {
        int ret = 0;
        fp = fopen(wavFile.c_str(), "r");
        EXPECT_NE(fp, nullptr);
        if (nullptr == fp) {
            xlog_err("open file failed: %s\n", wavFile.c_str());
            break;
        }
        struct wav_demuxer_info demuxer_info = {};
        demuxer_info.fp = fp;
        demuxer = wav_demuxer_create(&demuxer_info);
        EXPECT_NE(demuxer, wav_demuxer_handle_invalid);
        if (demuxer == wav_demuxer_handle_invalid) {
            xlog_err("demuxer create failed\n");
            break;
        }

        struct wav_info wav_info = {};
        ret = wav_demuxer_get_info(demuxer, &wav_info);
        EXPECT_EQ(ret, 0);
        if (ret < 0) {
            xlog_err("get wav info failed\n");
            break;
        }

        EXPECT_EQ(static_cast<int>(wav_info.audio_type), format);
        EXPECT_EQ(wav_info.channels, channels);
        EXPECT_EQ(wav_info.bits_per_sample, samplebits);
        EXPECT_EQ(wav_info.sample_rate, samplerate);

        size_t data_size = 0;
        ret = wav_demuxer_get_data_size(demuxer, &data_size);
        EXPECT_EQ(ret, 0);
        if (ret < 0) {
            xlog_err("wav_demuxer_get_data_size failed\n");
            break;
        }

        if ((wav_info.channels != 0)
                && (wav_info.bits_per_sample != 0)) {
            size_t sample_num = data_size / (wav_info.channels * wav_info.bits_per_sample / 8);
            EXPECT_EQ(sample_num, samples);
        }

        std::vector<uint8_t> data;
        std::vector<uint8_t> buf;
        buf.resize(32);
        data.resize(data_size);
        size_t offset = 0;
        while (true) {
            size_t size = buf.size();
            ret = wav_demuxer_get_data(demuxer, offset, buf.data(), &size);
            if (ret < 0) {
                break;
            } else {
                if (0 == size) {
                    break;
                }
                EXPECT_LE(size, buf.size());
                EXPECT_LE(offset + size, data.size());
                if (offset + size <= data.size()) {
                    memcpy(data.data() + offset, buf.data(), size);
                    offset += size;
                }
            }
        }

        auto file_buffer = readFile(rawFile);
        EXPECT_EQ(file_buffer, data);

        // saveFile("dump", data);
        // exit(0);

    } while (false);
}

TEST(wav_demuxer, demux) {
    std::string path = std::string(RES_AUDIO_PATH) + "/";

    std::list<TestDemux> demux;

    {
        TestDemux item{};
        item.file = path + "demo_8000hz_1ch.wav";
        item.file_raw = path + "demo_8000hz_1ch.raw";
        item.format = (int)WAV_AUDIO_TYPE_PCM;
        item.channel = 1;
        item.samplerate = 8000;
        item.samplebits = 16;
        item.samples = 8000 * 2;
        demux.push_back(item);
    }
    {
        TestDemux item{};
        item.file = path + "demo_8000hz_1ch_alaw.wav";
        item.file_raw = path + "demo_8000hz_1ch_alaw.raw";
        item.format = (int)WAV_AUDIO_TYPE_ALAW;
        item.channel = 1;
        item.samplerate = 8000;
        item.samplebits = 8;
        item.samples = 8000 * 2;
        demux.push_back(item);
    }
    {
        TestDemux item{};
        item.file = path + "demo_8000hz_1ch_ulaw.wav";
        item.file_raw = path + "demo_8000hz_1ch_ulaw.raw";
        item.format = (int)WAV_AUDIO_TYPE_MULAW;
        item.channel = 1;
        item.samplerate = 8000;
        item.samplebits = 8;
        item.samples = 8000 * 2;
        demux.push_back(item);
    }
    {
        TestDemux item{};
        item.file = path + "demo_16000hz_1ch.wav";
        item.file_raw = path + "demo_16000hz_1ch.raw";
        item.format = (int)WAV_AUDIO_TYPE_PCM;
        item.channel = 1;
        item.samplerate = 16000;
        item.samplebits = 16;
        item.samples = 16000 * 2;
        demux.push_back(item);
    }
    {
        TestDemux item{};
        item.file = path + "demo_16000hz_1ch_alaw.wav";
        item.file_raw = path + "demo_16000hz_1ch_alaw.raw";
        item.format = (int)WAV_AUDIO_TYPE_ALAW;
        item.channel = 1;
        item.samplerate = 16000;
        item.samplebits = 8;
        item.samples = 16000 * 2;
        demux.push_back(item);
    }
    {
        TestDemux item{};
        item.file = path + "demo_16000hz_1ch_ulaw.wav";
        item.file_raw = path + "demo_16000hz_1ch_ulaw.raw";
        item.format = (int)WAV_AUDIO_TYPE_MULAW;
        item.channel = 1;
        item.samplerate = 16000;
        item.samplebits = 8;
        item.samples = 16000 * 2;
        demux.push_back(item);
    }

    for (auto const& ref : demux) {
        testExtractRaw(ref.file, ref.file_raw, ref.format, ref.channel,
                       ref.samplerate, ref.samplebits, ref.samples);
    }
}