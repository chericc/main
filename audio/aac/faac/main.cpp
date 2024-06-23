#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include <chrono>
#include <thread>

#include "faac_helper.hpp"

#define xdebug(x...)                                                   \
    do {                                                               \
        printf("[debug][%s %d %s]", __FILE__, __LINE__, __FUNCTION__); \
        printf(x);                                                     \
    } while (0)
#define xinfo(x...)                                                   \
    do {                                                              \
        printf("[info][%s %d %s]", __FILE__, __LINE__, __FUNCTION__); \
        printf(x);                                                    \
    } while (0)
#define xerror(x...)                                                   \
    do {                                                               \
        printf("[error][%s %d %s]", __FILE__, __LINE__, __FUNCTION__); \
        printf(x);                                                     \
    } while (0)

#define debug_timeprint_x(x...)                                               \
    do {                                                                      \
        struct timeval tvTmp = {};                                            \
        struct tm stmTmp = {};                                                \
        char szDTime[48] = {};                                                \
        char szResult[64] = {};                                               \
                                                                              \
        gettimeofday(&tvTmp, NULL);                                           \
        gmtime_r(&tvTmp.tv_sec, &stmTmp);                                     \
        strftime(szDTime, sizeof(szDTime), "%F %T", &stmTmp);                 \
        snprintf(szResult, sizeof(szResult), "%s.%03d", szDTime,              \
                 (int)(tvTmp.tv_usec / 1000));                                \
                                                                              \
        printf("timestamp[%s %d %s] [%s] ", __FILE__, __LINE__, __FUNCTION__, \
               szResult);                                                     \
        printf(x);                                                            \
    } while (0)

typedef struct {
    unsigned int vbr;
    unsigned int bitrate;
    unsigned int samplerate;
} Config;

int run(const char* input_file, const char* output_file, const Config* config,
        int repeat) {
    std::shared_ptr<AAC::FAACHelper> aac;
    AAC::AacEncInfo aac_info = {};

    FILE* fp_in = nullptr;
    FILE* fp_out = nullptr;

    int ret = 0;

    unsigned char* buffer_output = nullptr;
    unsigned char* buffer_input = nullptr;
    unsigned int buffer_output_size = 0;
    unsigned int buffer_input_size = 0;

    if (nullptr == config) {
        aac = std::make_shared<AAC::FAACHelper>();
    } else {
        AAC::AACConfig aac_config = {};
        aac_config.bitrate = config->bitrate;
        aac_config.vbr = config->vbr;
        aac_config.sample_rate = config->samplerate;

        aac = std::make_shared<AAC::FAACHelper>(&aac_config);
    }

    if (nullptr == config) {
        xinfo("test: default construction\n");
    } else {
        xinfo("test: vbr=%u, bitrate=%u, samplerate=%u\n", config->vbr,
              config->bitrate, config->samplerate);
    }

    if (aac->init() < 0) {
        xerror("Init failed\n");
        return -1;
    }

    if (aac->getInfo(aac_info) < 0) {
        xerror("Get info failed\n");
        return -1;
    }

    buffer_output_size = aac_info.max_out_buf_bytes;
    buffer_input_size = aac_info.max_in_buf_bytes;

    debug_timeprint_x("max_out_buf_bytes=%u, max_in_buf_bytes=%u\n",
                      aac_info.max_out_buf_bytes, aac_info.max_in_buf_bytes);

    if (true) {
        const unsigned int CUSTOM_INPUT_SIZE = 638;
        if (CUSTOM_INPUT_SIZE <= buffer_input_size) {
            buffer_input_size = CUSTOM_INPUT_SIZE;
        }
    }

    buffer_output = (unsigned char*)malloc(buffer_output_size);
    buffer_input = (unsigned char*)malloc(buffer_input_size);

    fp_in = fopen(input_file, "r");
    fp_out = fopen(output_file, "w");

    if (nullptr == fp_in || nullptr == fp_out) {
        debug_timeprint_x("open file <%s,%s> failed\n", input_file,
                          output_file);

        if (fp_in != nullptr) {
            fclose(fp_in);
            fp_in = nullptr;
        }
        if (fp_out != nullptr) {
            fclose(fp_out);
            fp_out = nullptr;
        }

        return -1;
    }

    while (true) {
        AAC::AudioInputPCMData inputdata = {};
        AAC::InputArgs inputarg = {};
        AAC::AudioOutputAACData outputdata = {};
        AAC::OutputArgs outputarg = {};

        ret = fread(buffer_input, 1, buffer_input_size, fp_in);

        if (0 == ret) {
            if (repeat > 0) {
                xinfo("repeat=%d\n", repeat);
                repeat -= 1;
                rewind(fp_in);
            }
        }

        if (ret > 0) {
            inputdata.audio_data = buffer_input;
            inputdata.audio_data_size = ret;
            inputarg.flush_flag = false;
        } else {
            inputarg.flush_flag = true;
        }

        outputdata.audio_data = buffer_output;
        outputdata.audio_data_size = buffer_output_size;

        if (aac->encode(&inputdata, &inputarg, &outputdata, &outputarg) < 0) {
            xerror("Encode failed\n");
            break;
        }

        // debug_timeprint_x ("in: %d bytes, out: %d bytes\n",
        //     ret, outputarg.packet_size);

        if (outputarg.packet_size > 0) {
            fwrite(outputdata.audio_data, 1, outputarg.packet_size, fp_out);
            fflush(fp_out);
        }

        if (outputarg.eof_flag) {
            xinfo("eof, break\n");
            break;
        }
    }

    fclose(fp_in);
    fp_in = nullptr;
    fclose(fp_out);
    fp_out = nullptr;

    if (nullptr != buffer_output) {
        free(buffer_output);
        buffer_output = nullptr;
    }

    if (nullptr != buffer_input) {
        free(buffer_input);
        buffer_input = nullptr;
    }

    return 0;
}

void test(const char* file_in, const char* file_out, int test_type) {
    char file_in_real[64];
    char file_out_real[64];

    Config config[] = {
        {.vbr = 1000, .bitrate = 0, .samplerate = 8000},
        {.vbr = 10, .bitrate = 0, .samplerate = 8000},
        {.vbr = 5000, .bitrate = 0, .samplerate = 8000},
        {.vbr = 0, .bitrate = 32000, .samplerate = 8000},
        {.vbr = 0, .bitrate = 64000, .samplerate = 8000},
        {.vbr = 0, .bitrate = 96000, .samplerate = 8000},
        {.vbr = 1000, .bitrate = 0, .samplerate = 44100},
        {.vbr = 10, .bitrate = 0, .samplerate = 44100},
        {.vbr = 5000, .bitrate = 0, .samplerate = 44100},
        {.vbr = 0, .bitrate = 32000, .samplerate = 44100},
        {.vbr = 0, .bitrate = 64000, .samplerate = 44100},
        {.vbr = 0, .bitrate = 96000, .samplerate = 44100},
    };

    if (0 == test_type) {
        snprintf(file_in_real, sizeof(file_in_real), "%s_%u.pcm", file_in,
                 config[0].samplerate);
        snprintf(file_out_real, sizeof(file_out_real),
                 "%s_vbr%u_bitrate%u_default.aac", file_out, config[0].vbr,
                 config[0].bitrate);
        if (run(file_in_real, file_out_real, nullptr, 1) < 0) {
            xerror("test failed\n");
        }
    } else if (1 == test_type) {
        for (unsigned int i = 0; i < sizeof(config) / sizeof(config[0]); ++i) {
            snprintf(file_in_real, sizeof(file_in_real), "%s_%u.pcm", file_in,
                     config[i].samplerate);
            if (config[i].vbr > 0) {
                snprintf(file_out_real, sizeof(file_out_real),
                         "%s_vbr%u_bitrate%u_sample%u.aac", file_out,
                         config[i].vbr, config[i].bitrate,
                         config[i].samplerate);
            } else {
                snprintf(file_out_real, sizeof(file_out_real),
                         "%s_abr%u_bitrate%u_sample%u.aac", file_out,
                         config[i].vbr, config[i].bitrate,
                         config[i].samplerate);
            }

            if (run(file_in_real, file_out_real, &config[i], 1) < 0) {
                xerror("test failed\n");
                break;
            }
        }
    } else if (2 == test_type) {
        int repeat = 100;
        snprintf(file_in_real, sizeof(file_in_real), "%s_%u.pcm", file_in,
                 config[0].samplerate);
        snprintf(file_out_real, sizeof(file_out_real),
                 "%s_vbr%u_bitrate%u_repeat%d.aac", file_out, config[0].vbr,
                 config[0].bitrate, repeat);
        if (run(file_in_real, file_out_real, nullptr, repeat) < 0) {
            xerror("test failed\n");
        }
    } else {
        xerror("unsupport test_type\n");
    }

    return;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        printf("Usage: %s <input_file> <output_file> <type>\n", argv[0]);
        return 0;
    }

    test(argv[1], argv[2], atoi(argv[3]));

    return 0;
}