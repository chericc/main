#include <gtest/gtest.h>

#include "pcm.hpp"

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