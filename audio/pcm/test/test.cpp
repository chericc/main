#include <gtest/gtest.h>

#include "pcm.hpp"

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

template <typename T>
static std::vector<T> readFile(const std::string &filename)
{
    std::vector<T> buffer;
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

TEST(pcm, convert)
{
    std::string path = "/home/test/code/log/audio/sources/wav";

    std::string alaw = path + "demo_8000hz_1ch_alaw.raw";
    std::string ulaw = path + "demo_8000hz_1ch_ulaw.raw";
    std::string raw = path + "demo_8000hz_1ch.raw";

    {
        auto buf_file_alaw = readFile<uint8_t>(alaw);
        auto buf_file_pcm = readFile<uint16_t>(raw);
        auto buf_convert = pcm_s16le_to_alaw(buf_file_pcm);

        EXPECT_EQ(buf_file_alaw, buf_convert);
    }

    {
        auto buf_file_ulaw = readFile<uint8_t>(ulaw);
        auto buf_file_pcm = readFile<uint16_t>(raw);
        auto buf_convert = pcm_s16le_to_mulaw(buf_file_pcm);

        EXPECT_EQ(buf_file_ulaw, buf_convert);
    }
    
}

int main(int argc, char** argv)
{
	printf("Running main() from %s\n", __FILE__);

	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}