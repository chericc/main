#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include <chrono>
#include <thread>

#include "aac_fdkaac.hpp"

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

int run(const char* input_file, const char* output_file) {
    AacFDKAac aac;
    AacEncInfo aac_info = {};

    FILE* fp_in = nullptr;
    FILE* fp_out = nullptr;

    int ret = 0;

    unsigned char* buffer_output = nullptr;
    unsigned char* buffer_input = nullptr;
    unsigned int buffer_output_size = 0;
    unsigned int buffer_input_size = 0;

    if (aac.Init() < 0) {
        xerror("Init failed\n");
        return -1;
    } else {
        xinfo("Init successful\n");
    }

    if (aac.GetInfo(aac_info) < 0) {
        xerror("Get info failed\n");
        return -1;
    }

    xinfo("in:%d bytes, out:%d bytes\n", aac_info.maxInBufBytes,
          aac_info.maxOutBufBytes);

    buffer_output_size = aac_info.maxOutBufBytes;
    buffer_input_size = aac_info.maxInBufBytes;

    if (true) {
        const unsigned int CUSTOM_INPUT_SIZE = buffer_input_size;
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
        AudioInputPCMData inputdata = {};
        InputArgs inputarg = {};
        AudioOutputAACData outputdata = {};
        OutputArgs outputarg = {};

        ret = fread(buffer_input, 1, buffer_input_size, fp_in);

        if (ret > 0) {
            inputdata.audio_data = buffer_input;
            inputdata.audio_data_size = ret;
            inputarg.flush_flag = false;
        } else {
            inputarg.flush_flag = true;
        }

        outputdata.audio_data = buffer_output;
        outputdata.audio_data_size = buffer_output_size;

        if (aac.Encode(&inputdata, &inputarg, &outputdata, &outputarg) < 0) {
            xerror("Encode failed\n");
            break;
        }

        debug_timeprint_x("in: %d bytes, out: %d bytes\n", ret,
                          outputarg.packet_size);

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

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s <input_file> <output_file>\n", argv[0]);
        return 0;
    }

    run(argv[1], argv[2]);

    return 0;
}