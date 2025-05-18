#include "wav_demuxer.hpp"

#include <gtest/gtest.h>

#include <list>

#include "test_comm.hpp"
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
    WavDemuxer wavd(wavFile);

    xlog_dbg("testExtractRaw: file:%s", wavFile.c_str());

    EXPECT_TRUE(wavd.loadSuccessful());

    if (wavd.loadSuccessful()) {
        EXPECT_EQ(wavd.format(), format);
        EXPECT_EQ(wavd.numChannels(), channels);
        EXPECT_EQ(wavd.sampleRate(), samplerate);
        EXPECT_EQ(wavd.sampleBits(), samplebits);
        EXPECT_EQ(wavd.numSamples(), samples);

        int nb_samples = wavd.numSamples();
        auto get_buffer = wavd.getSamples(0, 0, nb_samples);
        auto file_buffer = readFile(rawFile);

        // saveFile("get_buffer", get_buffer);
        // saveFile("file_buffer", file_buffer);

        EXPECT_EQ(get_buffer.size(), file_buffer.size());
        EXPECT_EQ(get_buffer, file_buffer);
    }

    xlog_dbg("testExtractRaw fin");
}

TEST(wav_demuxer, demux) {
    std::string path = std::string(RES_AUDIO_PATH) + "/";

    std::list<TestDemux> demux;

    {
        TestDemux item{};
        item.file = path + "demo_8000hz_1ch.wav";
        item.file_raw = path + "demo_8000hz_1ch.raw";
        item.format = (int)RiffFmt::PCM;
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
        item.format = (int)RiffFmt::PCM_ALAW;
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
        item.format = (int)RiffFmt::PCM_MULAW;
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
        item.format = (int)RiffFmt::PCM;
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
        item.format = (int)RiffFmt::PCM_ALAW;
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
        item.format = (int)RiffFmt::PCM_MULAW;
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