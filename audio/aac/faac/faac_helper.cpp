#include "faac_helper.hpp"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mutex>

#include "faac.h"

#ifdef AAC_USE_ASSERT
#define ASSERT(expr) assert(expr)
#else
#define ASSERT(expr) \
    if (expr) {      \
        ;            \
    }
#endif

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

using namespace AAC;

class FAACHelper::InnerData {
   public:
    bool init_flag = false;

    std::mutex mutex;

    faacEncHandle handle = nullptr;

    unsigned int mpeg_version = MPEG2;
    unsigned int object_type = LOW;
    unsigned long sample_rate = 8000;  // 采样率
    unsigned int channels = 1;
    unsigned long vbr = 1000;       // 0,10-5000
    unsigned long bitrate = 96000;  // 非 VBR 时有效
    unsigned int output_format = ADTS_STREAM;
    unsigned int input_format = FAAC_INPUT_16BIT;
    unsigned int use_tns = 0;
    unsigned int use_lfe = 0;
    // unsigned int band_width = sample_rate / 2;  // 频率 = 采样率 / 2

    unsigned long samples_input = 0;     // SPF, samples/frame
    unsigned long max_bytes_output = 0;  // 最大输出字节

    uint8_t* pcm_buf = nullptr;  // PCM 缓存，用于缓存小于一个 FRAME 的输入
    unsigned int pcm_buf_size = 0;  // PCM 缓存大小
    unsigned int pcm_buf_data_size = 0;  // PCM 缓存数据大小（从起始位置开始）
};

FAACHelper::FAACHelper(AACConfig* config) {
    _data = std::make_shared<InnerData>();

    if (config != nullptr) {
        _data->sample_rate = config->sample_rate;
        _data->vbr = config->vbr;
        _data->bitrate = config->bitrate;
    }
}

FAACHelper::~FAACHelper() {
    std::lock_guard<std::mutex> lock_guard(_data->mutex);

    deinit();
}

int FAACHelper::init() {
    std::lock_guard<std::mutex> lock_guard(_data->mutex);

    bool& init_flag = _data->init_flag;
    faacEncHandle& handle = _data->handle;
    const unsigned int& mpeg_version = _data->mpeg_version;
    const unsigned int& object_type = _data->object_type;
    const unsigned long& sample_rate = _data->sample_rate;
    const unsigned int& channels = _data->channels;
    const unsigned long& vbr = _data->vbr;
    const unsigned long& bitrate = _data->bitrate;
    const unsigned int& output_format = _data->output_format;
    const unsigned int& use_lfe = _data->use_lfe;
    const unsigned int& use_tns = _data->use_tns;
    const unsigned int& input_format = _data->input_format;
    // const unsigned int &band_width = _data->band_width;

    unsigned long& samples_input = _data->samples_input;
    unsigned long& max_bytes_output = _data->max_bytes_output;

    uint8_t*& pcm_buf = _data->pcm_buf;
    unsigned int& pcm_buf_size = _data->pcm_buf_size;
    unsigned int& pcm_buf_data_size = _data->pcm_buf_data_size;

    faacEncConfigurationPtr config = nullptr;

    unsigned int tmp_pcm_buf_size = 0;

    if (init_flag) {
        xerror("init flag is true\n");
        return -1;
    }

    if (handle != nullptr) {
        xerror("handle not null\n");
        return -1;
    }

    if (faacEncGetVersion(nullptr, nullptr) != FAAC_CFG_VERSION) {
        xerror("Version error\n");
        return -1;
    }

    if ((handle = faacEncOpen(sample_rate, channels, &samples_input,
                              &max_bytes_output)) == nullptr) {
        xerror("Faac open failed\n");
        return -1;
    } else {
        xinfo("Faac open: samples_input=%lu,maxbytesoutput=%lu\n",
              samples_input, max_bytes_output);
    }

    /* 参考值：samplesinput=1024,maxbytesoutput=8192 */
    if (samples_input > 1 * 1024 * 1024) {
        xerror("Faac error, samples_input too big\n");
        return -1;
    }
    if (max_bytes_output > 1 * 1024 * 1024) {
        xerror("Faac error, maxbytesoutput too big\n");
        return -1;
    }

    // 2 倍最大帧大小，足够缓存不足的输入帧
    tmp_pcm_buf_size = samples_input * PCM_ELEMENT_SIZE * 2;

    if (nullptr == config) {
        config = faacEncGetCurrentConfiguration(handle);
        if (nullptr == config) {
            xerror("Get config failed\n");
            return -1;
        }
    }

    config->mpegVersion = mpeg_version;
    config->aacObjectType = object_type;
    config->useLfe = use_lfe;
    config->useTns = use_tns;
    // config->bandWidth = band_width;
    if (vbr) {
        config->quantqual = vbr;
        config->bitRate = 0;
    } else {
        config->quantqual = 0;
        config->bitRate = bitrate;
    }
    config->outputFormat = output_format;
    config->inputFormat = input_format;

    if (!faacEncSetConfiguration(handle, config)) {
        xerror("Faac config failed\n");
        return -1;
    }

    if (pcm_buf != nullptr) {
        free(pcm_buf);
        pcm_buf = nullptr;
        pcm_buf_size = 0;
        pcm_buf_data_size = 0;
    }

    if (nullptr == pcm_buf) {
        pcm_buf = static_cast<uint8_t*>(malloc(tmp_pcm_buf_size));
        if (nullptr == pcm_buf) {
            xerror("Malloc failed\n");
            return -1;
        } else {
            pcm_buf_size = tmp_pcm_buf_size;
            pcm_buf_data_size = 0;

            memset(pcm_buf, 0, sizeof(tmp_pcm_buf_size));
        }
    }

    init_flag = true;

    return 0;
}

bool FAACHelper::isInit() const { return _data->init_flag; }

unsigned int FAACHelper::oneFramePCMSize() const {
    ASSERT(isInit());

    return _data->samples_input * PCM_ELEMENT_SIZE;
}

int FAACHelper::deinit() {
    faacEncHandle& handle = _data->handle;
    uint8_t* pcm_buf = _data->pcm_buf;
    unsigned int& pcm_buf_size = _data->pcm_buf_size;
    unsigned int& pcm_buf_data_size = _data->pcm_buf_data_size;

    if (handle != nullptr) {
        faacEncClose(handle);
        handle = nullptr;
    }

    if (nullptr != pcm_buf) {
        free(pcm_buf);
        pcm_buf_size = 0;
        pcm_buf_data_size = 0;
    }

    return 0;
}

int FAACHelper::checkEncArgs(const AudioInputPCMData* input,
                             const InputArgs* input_args,
                             AudioOutputAACData* output,
                             OutputArgs* output_args) {
    unsigned long& maxbytesoutput = _data->max_bytes_output;

    if (nullptr == input || nullptr == input_args || nullptr == output ||
        nullptr == output_args) {
        xerror("null argument\n");
        return -1;
    }

    if ((input->audio_data_size % PCM_ELEMENT_SIZE) != 0) {
        xerror("pcm size not valid: size=%d\n", input->audio_data_size);
        return -1;
    }

    if (input->audio_data_size > oneFramePCMSize()) {
        xerror("data size over: %u>%u\n", input->audio_data_size,
               oneFramePCMSize());
        return -1;
    }

    if (output->audio_data_size < maxbytesoutput) {
        xerror("output buffer too small(%u < %lu)\n", output->audio_data_size,
               maxbytesoutput);
        return -1;
    }

    if (input->audio_data_size > 0 && input_args->flush_flag) {
        xerror("flush with input data\n");
        return -1;
    }

    return 0;
}

int FAACHelper::encodeStream(const void* data_input,
                             unsigned int data_input_size, void* data_output,
                             unsigned int* data_output_size) {
    /* 锁由 encode 接口保证 */

    int ret = 0;

    AudioInputPCMData input{};
    AudioOutputAACData output{};
    InputArgs input_args{};
    OutputArgs output_args{};

    if (nullptr == data_input || nullptr == data_output_size) {
        xerror("input error\n");
        return -1;
    }

    input.audio_data = (unsigned char*)data_input;
    input.audio_data_size = data_input_size;
    input_args.flush_flag = false;
    output.audio_data = (unsigned char*)data_output;
    output.audio_data_size = *data_output_size;

    ret = encode(&input, &input_args, &output, &output_args);

    if (0 == ret) {
        *data_output_size = output_args.packet_size;
    }

    return ret;
}

int FAACHelper::encode(const AudioInputPCMData* input,
                       const InputArgs* input_args, AudioOutputAACData* output,
                       OutputArgs* output_args) {
    std::lock_guard<std::mutex> lock_guard(_data->mutex);

    uint8_t*& pcm_buf = _data->pcm_buf;
    unsigned int& pcm_buf_size = _data->pcm_buf_size;
    unsigned int& pcm_buf_data_size = _data->pcm_buf_data_size;

    AudioInputPCMData inputex = {};
    const unsigned int one_packet_pcm_size = oneFramePCMSize();

    if (!isInit()) {
        xerror("not init\n");
        return -1;
    }

    if (checkEncArgs(input, input_args, output, output_args) < 0) {
        xerror("args check failed\n");
        return -1;
    }

    ASSERT(input->audio_data_size + pcm_buf_data_size <= pcm_buf_size);

    if (!input_args->flush_flag) {
        memcpy(pcm_buf + pcm_buf_data_size, input->audio_data,
               input->audio_data_size);
        pcm_buf_data_size += input->audio_data_size;
    } else {
        if (pcm_buf_data_size > 0 && pcm_buf_data_size < one_packet_pcm_size) {
            xinfo("Fill the remaining buffer\n");
            memset(pcm_buf + pcm_buf_data_size, 0,
                   one_packet_pcm_size - pcm_buf_data_size);
            pcm_buf_data_size = one_packet_pcm_size;
        }
    }

    if (pcm_buf_data_size >= one_packet_pcm_size) {
        inputex.audio_data = pcm_buf;
        inputex.audio_data_size = one_packet_pcm_size;
    } else {
        inputex.audio_data = pcm_buf;
        inputex.audio_data_size = 0;
    }

    // xdebug ("input: %d bytes\n", inputex.audio_data_size);

    if (input_args->flush_flag || inputex.audio_data_size > 0) {
        if (encodeFrame(&inputex, input_args, output, output_args) < 0) {
            xerror("Encode failed\n");
            return -1;
        }
    }

    /*
    关于 pcm_buf 移动的示例:
    + 表示已经送去编码的 PCM 数据
    - 表示剩余的 PCM 数据
    * 表示无效数据
    移动前:  ++++++++++++++---***********
    移动后:  ---*************************
    */
    if (inputex.audio_data_size > 0) {
        ASSERT(pcm_buf_data_size - inputex.audio_data_size >= 0);
        memmove(pcm_buf, pcm_buf + one_packet_pcm_size,
                pcm_buf_data_size - inputex.audio_data_size);

        pcm_buf_data_size -= inputex.audio_data_size;
    }

    return 0;
}

int FAACHelper::encodeFrame(const AudioInputPCMData* input,
                            const InputArgs* input_args,
                            AudioOutputAACData* output,
                            OutputArgs* output_args) {
    int ret_encode = 0;

    faacEncHandle& handle = _data->handle;

    if (prepareEncodeEnv(input, input_args, output, output_args) < 0) {
        xerror("Prepare env failed\n");
        return -1;
    }

    if (input_args->flush_flag) {
        ASSERT(0 == input->audio_data_size);
    }

    ret_encode = faacEncEncode(handle, (int32_t*)input->audio_data,
                               input->audio_data_size / PCM_ELEMENT_SIZE,
                               output->audio_data, output->audio_data_size);

    if (ret_encode < 0) {
        xerror("Encode failed\n");
        return -1;
    } else if (0 == ret_encode) {
        if (0 == input->audio_data_size && input_args->flush_flag) {
            xinfo("Eof\n");
            output_args->eof_flag = true;
        }
    } else {
        output_args->packet_size = ret_encode;
    }

    return 0;
}

int FAACHelper::getInfo(AacEncInfo& info) const {
    std::lock_guard<std::mutex> lock_guard(_data->mutex);

    if (!isInit()) {
        xerror("Not init\n");
        return -1;
    }

    info.max_in_buf_bytes = _data->samples_input * PCM_ELEMENT_SIZE;
    info.max_out_buf_bytes = _data->max_bytes_output;

    return 0;
}

// 一些处理
int FAACHelper::prepareEncodeEnv(const AudioInputPCMData* input,
                                 const InputArgs* input_args,
                                 AudioOutputAACData* output,
                                 OutputArgs* output_args) {
    memset(output_args, 0, sizeof(*output_args));

    return 0;
}
