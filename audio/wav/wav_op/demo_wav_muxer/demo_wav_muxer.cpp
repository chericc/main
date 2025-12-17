#include <cstdlib>
#include <vector>
#include <cstdint>

#include "wav_muxer.h"
#include "xlog.h"

namespace {

int wav_mux(const char *rawfile, const char *wavfile, int channel, WAV_AUDIO_TYPE type, 
    int samplerate, int samplebits)
{
    bool error_flag = false;
    FILE *fp_raw = nullptr;
    FILE *fp_wav = nullptr;
    wav_muxer_handle wav_muxer = wav_muxer_handle_invalid;

    do {
        int ret = 0;
        fp_raw = fopen(rawfile, "r");
        if (nullptr == fp_raw) {
            xlog_err("error open {}", rawfile);
            error_flag = true;
            break;
        }

        fp_wav = fopen(wavfile, "w");
        if (nullptr == wavfile) {
            xlog_err("error open {}", wavfile);
            error_flag = true;
            break;
        }

        struct wav_muxer_info info = {};
        info.info.audio_type = type;
        info.info.bits_per_sample = samplebits;
        info.info.sample_rate = samplerate;
        info.info.channels = channel;
        info.fp = fp_wav;
        wav_muxer = wav_muxer_create(&info);
        if (wav_muxer_handle_invalid == wav_muxer) {
            xlog_err("wav_muxer_create failed");
            error_flag = true;
            break;
        }

        std::vector<uint8_t> buf;
        
        size_t chunk_size = samplebits * samplerate / 8;
        buf.resize(chunk_size);
        for (;;) {
            size_t ret_fread = fread(buf.data(), 1, chunk_size, fp_raw);
            if (ret_fread != chunk_size) {
                if (ret_fread != 0) {
                    xlog_err("tail not full chunk");
                }
                break;
            }
            ret = wav_muxer_input(wav_muxer, buf.data(), chunk_size, 1);
            if (ret < 0) {
                xlog_err("wav_muxer_input failed");
                error_flag = true;
                break;
            }
        }

    } while (false);

    if (wav_muxer != wav_muxer_handle_invalid) {
        wav_muxer_close(wav_muxer);
        wav_muxer = wav_muxer_handle_invalid;
    }
    if (fp_raw != nullptr) {
        fclose(fp_raw);
        fp_raw = nullptr;
    }
    if (fp_wav != nullptr) {
        fclose(fp_wav);
        fp_wav = nullptr;
    }

    return error_flag ? -1 : 0;
}

}

int main(int argc, char *argv[])
{
    if (argc != 7) {
        xlog_err("Usage: {} [rawfile] [wavfile] [channel] [format] [samplerate] [samplebits]\n"
            "eg: %s input.pcm output.wav 1 1 16000 16\n", 
            argv[0], argv[0]);
        return 1;
    }

    const char *rawfile = argv[1];
    const char *wavfile = argv[2];
    int channel = atoi(argv[3]);
    int type = atoi(argv[4]);
    int samplerate = atoi(argv[5]);
    int samplebits = atoi(argv[6]);

    int ret = wav_mux(rawfile, wavfile, channel, static_cast<WAV_AUDIO_TYPE>(type), samplerate, samplebits);
    return ret < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}