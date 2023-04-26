#include <gtest/gtest.h>

#include <list>

#include "print.hpp"
#include "wav_demuxer.hpp"

struct TestDemux
{
    std::string file;
    std::string file_raw;

    int format;
    int channel;
    int samplerate;
    int samplebits;
    int samples;
};

static size_t fileSize(FILE *fp)
{
    size_t size_file = 0;
    size_t size_origin = 0;
    if (fp)
    {
        size_origin = ftell(fp);
        fseek(fp, 0, SEEK_END);
        size_file = ftell(fp);
        fseek(fp, size_origin, SEEK_SET);
    }

    return size_file;
}

static std::vector<uint8_t> readFile(const std::string &filename)
{
    std::vector<uint8_t> buffer;
    FILE *fp = fopen(filename.c_str(), "r");

    if (fp)
    {
        size_t pos = 0;
        size_t filesize = fileSize(fp);

        
        buffer.resize(filesize);

        int ret = fread(buffer.data(), 1, filesize, fp);

        fclose(fp);
        fp = nullptr;
    }

    return buffer;
}

static void saveFile(const std::string &filename, std::vector<uint8_t> data)
{
    FILE *fp = fopen(filename.c_str(), "w");
    if (fp)
    {
        int ret = fwrite(data.data(), 1, data.size(), fp);
        if (fp)
        {
            fclose(fp);
            fp = nullptr;
        }
    }
}

static void testExtractRaw(const std::string &wavFile, const std::string &rawFile,
    int format, int channels, int samplerate, int samplebits, int samples)
{
    WavDemuxer wavd(wavFile);

    LOGD("testExtractRaw: file:%s", wavFile.c_str());

    EXPECT_TRUE(wavd.loadSuccessful());

    if (wavd.loadSuccessful())
    {
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

    LOGD("testExtractRaw fin");
}

TEST(wav_demuxer, demux)
{
    std::string path = "/home/test/code/log/audio/sources/wav/";

    TestDemux demux[] = {
        {
            .file = path + "demo_8000hz_1ch.wav",
            .file_raw = path + "demo_8000hz_1ch.raw",
            .format = (int)RiffFmt::PCM,
            .channel = 1,
            .samplerate = 8000,
            .samplebits = 16,
            .samples = 8000 * 2
        },
        {
            .file = path + "demo_8000hz_1ch_alaw.wav",
            .file_raw = path + "demo_8000hz_1ch_alaw.raw",
            .format = (int)RiffFmt::PCM_ALAW,
            .channel = 1,
            .samplerate = 8000,
            .samplebits = 8,
            .samples = 8000 * 2
        },
        {
            .file = path + "demo_8000hz_1ch_ulaw.wav",
            .file_raw = path + "demo_8000hz_1ch_ulaw.raw",
            .format = (int)RiffFmt::PCM_MULAW,
            .channel = 1,
            .samplerate = 8000,
            .samplebits = 8,
            .samples = 8000 * 2
        },
        {
            .file = path + "demo_16000hz_1ch.wav",
            .file_raw = path + "demo_16000hz_1ch.raw",
            .format = (int)RiffFmt::PCM,
            .channel = 1,
            .samplerate = 16000,
            .samplebits = 16,
            .samples = 16000 * 2
        },
        {
            .file = path + "demo_16000hz_1ch_alaw.wav",
            .file_raw = path + "demo_16000hz_1ch_alaw.raw",
            .format = (int)RiffFmt::PCM_ALAW,
            .channel = 1,
            .samplerate = 16000,
            .samplebits = 8,
            .samples = 16000 * 2
        },
        {
            .file = path + "demo_16000hz_1ch_ulaw.wav",
            .file_raw = path + "demo_16000hz_1ch_ulaw.raw",
            .format = (int)RiffFmt::PCM_MULAW,
            .channel = 1,
            .samplerate = 16000,
            .samplebits = 8,
            .samples = 16000 * 2
        },
    };

    for (int i = 0; i < sizeof(demux)/sizeof(demux[0]); ++i)
    {
        testExtractRaw(demux[i].file, demux[i].file_raw, 
            demux[i].format, demux[i].channel, demux[i].samplerate, 
            demux[i].samplebits, demux[i].samples);
    }
}

int main(int argc, char** argv)
{
	printf("Running main() from %s\n", __FILE__);

	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}