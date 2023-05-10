#pragma once

#include <memory>

/************************** STRUCTURE **************************/

typedef struct
{
    unsigned char *audio_data;
    unsigned int audio_data_size;    // 8bit bytes
} AudioData;

typedef AudioData AudioInputPCMData;    // 作为输入数据时，取有效数据大小
typedef AudioData AudioOutputAACData;   // 作为输出数据时，取缓存大小

typedef struct
{
    int flush_flag;    // 如果没有后续输入了，则将此标志置真，将剩余数据取出（多次调用至 eof_flag 为真）
} InputArgs;

typedef struct
{
    int packet_size;    // 8bit bytes
    int eof_flag;
} OutputArgs;

typedef struct 
{
    /* 最大输入帧大小（提供的输入缓存大小最大为这个值） */
    unsigned int maxInBufBytes;
    /* 最大输出帧大小（提供的输出缓存大小至少为这个值） */
    unsigned int maxOutBufBytes;
} AacEncInfo;

/************************** CLASS **************************/

class AacFDKAac
{
public:
    AacFDKAac();
    ~AacFDKAac();
    int Init();
    int GetInfo(AacEncInfo &info) const;
    int Encode(const AudioInputPCMData *input, 
        const InputArgs *input_args,
        AudioOutputAACData *output,
        OutputArgs *output_args);
private:
    bool IsInit__() const;
    int Deinit__();
    unsigned int OnePacketPCMSize__() const;
    int Encode__(const AudioInputPCMData *input, 
        const InputArgs *input_args,
        AudioOutputAACData *output,
        OutputArgs *output_args);
    int CheckEncArgs__(const AudioInputPCMData *input, 
        const InputArgs *input_args,
        AudioOutputAACData *output,
        OutputArgs *output_args);
    int PrepareEncodeEnv__ (const AudioInputPCMData *input, 
        const InputArgs *input_args,
        AudioOutputAACData *output,
        OutputArgs *output_args);
private:
    class InnerData;
    std::shared_ptr<InnerData> _data;
        
    static const int PCM_ELEMENT_SIZE = 2; // 16bit default
    static const int AAC_ELEMENT_SIZE = 1; // 8bit default
};
