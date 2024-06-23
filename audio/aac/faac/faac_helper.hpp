#ifndef FAAC_HELPER__
#define FAAC_HELPER__

#include <memory>

namespace AAC {

/************************** STRUCTURE **************************/

typedef struct {
    unsigned int sample_rate;  // 采样率
    unsigned int vbr;          // 可变码率（0,10-5000）
    unsigned int bitrate;      // 比特率（固定码率）
} AACConfig;

typedef struct {
    unsigned char* audio_data;
    unsigned int audio_data_size;  // 8bit bytes
} AACAudioData;

typedef AACAudioData AudioInputPCMData;  // 作为输入数据时，取有效数据大小
typedef AACAudioData AudioOutputAACData;  // 作为输出数据时，取缓存大小

typedef struct {
    int flush_flag;  // 如果没有后续输入了，则将此标志置真，将剩余数据取出（多次调用至
                     // eof_flag 为真）
} InputArgs;

typedef struct {
    int packet_size;  // 8bit bytes
    int eof_flag;
} OutputArgs;

typedef struct {
    /* 最大输入帧大小（提供的输入缓存大小最大为这个值） */
    unsigned int max_in_buf_bytes;
    /* 最大输出帧大小（提供的输出缓存大小至少为这个值） */
    unsigned int max_out_buf_bytes;
} AacEncInfo;

/************************** CLASS **************************/

class FAACHelper {
   public:
    FAACHelper(AACConfig* config = nullptr);
    ~FAACHelper();
    FAACHelper(FAACHelper&) = delete;
    FAACHelper& operator=(FAACHelper&) = delete;

    int init();
    int getInfo(AacEncInfo& info) const;
    int encodeStream(const void* data_input, unsigned int data_input_size,
                     void* data_output, unsigned int* data_output_size);
    int encode(const AudioInputPCMData* input, const InputArgs* input_args,
               AudioOutputAACData* output, OutputArgs* output_args);

   private:
    bool isInit() const;
    int deinit();
    unsigned int oneFramePCMSize() const;
    int encodeFrame(const AudioInputPCMData* input, const InputArgs* input_args,
                    AudioOutputAACData* output, OutputArgs* output_args);
    int checkEncArgs(const AudioInputPCMData* input,
                     const InputArgs* input_args, AudioOutputAACData* output,
                     OutputArgs* output_args);
    int prepareEncodeEnv(const AudioInputPCMData* input,
                         const InputArgs* input_args,
                         AudioOutputAACData* output, OutputArgs* output_args);

   private:
    class InnerData;
    std::shared_ptr<InnerData> _data;

    static const int PCM_ELEMENT_SIZE = 2;  // 16bit default
    static const int AAC_ELEMENT_SIZE = 1;  // 8bit default
};

}  // namespace AAC

#endif  // FAAC_HELPER__
