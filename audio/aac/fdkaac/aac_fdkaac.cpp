#include "aac_fdkaac.hpp"

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <mutex>
#include <string.h>
#include <stdlib.h>

#include "aacenc_lib.h"

#ifdef AAC_USE_ASSERT
#define ASSERT(expr) assert(expr)
#else
#define ASSERT(expr) if(expr){;}
#endif 

#define xdebug(x...) do {printf("[debug][%s %d %s]", \
	__FILE__,__LINE__,__FUNCTION__);printf(x);} while (0)
#define xinfo(x...) do {printf("[info][%s %d %s]", \
	__FILE__,__LINE__,__FUNCTION__);printf(x);} while (0)
#define xerror(x...) do {printf("[error][%s %d %s]", \
	__FILE__,__LINE__,__FUNCTION__);printf(x);} while (0)




NAMESPACE_AAC_BEGIN

class AacFDKAac::InnerData
{
public:
    bool init_flag = false;

    std::mutex mutex;

    HANDLE_AACENCODER handle = nullptr;

    const int bitrate = 64000;
    const int aot = AOT_MP2_AAC_LC;
    const int channels = 1;
    const int sample_rate = 8000;
    const int mode = MODE_1;
    const int channel_order = 0;
    const int vbr = 3;  // vbr = 0: cbr; vbr = 3: medium
    const int transporttype = TT_MP4_ADTS;
    const int afterburner = 0;

    AACENC_InfoStruct aacenc_info = {};

    // need free
    int16_t *convert_buf = nullptr;
    unsigned int convert_buf_size = 0;
    
    uint8_t *pcm_buf = nullptr;
    unsigned int pcm_buf_size = 0;
    unsigned int pcm_buf_data_size = 0;
};

AacFDKAac::AacFDKAac()
{
    _data = std::make_shared<InnerData>();
}

AacFDKAac::~AacFDKAac()
{
    std::lock_guard<std::mutex> lock_guard(_data->mutex);

    Deinit__();
}

int AacFDKAac::Init()
{
    std::lock_guard<std::mutex> lock_guard(_data->mutex);

    bool &init_flag = _data->init_flag;
    HANDLE_AACENCODER &handle = _data->handle;
    const int &bitrate = _data->bitrate;
    const int &aot = _data->aot;
    const int &channels = _data->channels;
    const int &sample_rate = _data->sample_rate;
    const int &mode = _data->mode;
    const int &channel_order = _data->channel_order;
    const int &vbr = _data->vbr;
    const int &transporttype = _data->transporttype;
    const int &afterburner = _data->afterburner;
    AACENC_InfoStruct &aacenc_info = _data->aacenc_info;

    int16_t* &convert_buf = _data->convert_buf;
    uint32_t &convert_buf_size = _data->convert_buf_size;
    unsigned int &pcm_buf_data_size = _data->pcm_buf_data_size;

    uint8_t* &pcm_buf = _data->pcm_buf;
    unsigned int &pcm_buf_size = _data->pcm_buf_size;

    unsigned int tmp_pcm_max_packet_size = 0;
    unsigned int tmp_pcm_buf_size = 0;

    if (init_flag)
    {
        xerror ("init flag is true\n");
        return -1;
    }

    if (handle != nullptr)
    {
        xerror("handle not null\n");
        return -1;
    }

	if (aacEncOpen(&handle, 1, channels) != AACENC_OK) 
    {
		xerror("Unable to open encoder\n");
		return -1;
	}
	if (aacEncoder_SetParam(handle, AACENC_AOT, aot) != AACENC_OK) 
    {
		xerror("Unable to set the AOT\n");
		return -1;
	}
	if (aacEncoder_SetParam(handle, AACENC_SAMPLERATE, sample_rate) != AACENC_OK) 
    {
		xerror("Unable to set the AOT\n");
		return -1;
	}
	if (aacEncoder_SetParam(handle, AACENC_CHANNELMODE, mode) != AACENC_OK) 
    {
		xerror("Unable to set the channel mode\n");
		return -1;
	}
	if (aacEncoder_SetParam(handle, AACENC_CHANNELORDER, channel_order) != AACENC_OK) 
    {
		xerror("Unable to set the wav channel order\n");
		return -1;
	}
	if (vbr) {
		if (aacEncoder_SetParam(handle, AACENC_BITRATEMODE, vbr) != AACENC_OK) 
        {
			xerror("Unable to set the VBR bitrate mode\n");
			return -1;
		}
	} 
    else 
    {
		if (aacEncoder_SetParam(handle, AACENC_BITRATE, bitrate) != AACENC_OK) 
        {
			xerror("Unable to set the bitrate\n");
			return -1;
		}
	}
	if (aacEncoder_SetParam(handle, AACENC_TRANSMUX, transporttype) != AACENC_OK) 
    {
		xerror("Unable to set the ADTS transmux\n");
		return 1;
	}
	if (aacEncoder_SetParam(handle, AACENC_AFTERBURNER, afterburner) != AACENC_OK) 
    {
		xerror("Unable to set the afterburner mode\n");
		return -1;
	}
	if (aacEncEncode(handle, NULL, NULL, NULL, NULL) != AACENC_OK) 
    {
		xerror("Unable to initialize the encoder\n");
		return -1;
	}
	if (aacEncInfo(handle, &aacenc_info) != AACENC_OK) 
    {
		xerror("Unable to get the encoder info\n");
		return -1;
	}

    if (nullptr != convert_buf)
    {
        free (convert_buf);
        convert_buf = nullptr;
        convert_buf_size = 0;
    }

    if (nullptr != pcm_buf)
    {
        free (pcm_buf);
        pcm_buf = nullptr;
        pcm_buf_size = 0;
    }

    tmp_pcm_max_packet_size = aacenc_info.frameLength * PCM_ELEMENT_SIZE;
    tmp_pcm_buf_size = aacenc_info.frameLength * PCM_ELEMENT_SIZE * 2;  // 2 倍最大帧大小，足够缓存不足的输入帧

    if (tmp_pcm_max_packet_size > 1024 * 1024)
    {
        xerror ("Invalid size(%d)\n", tmp_pcm_max_packet_size);
        return -1;
    }

    if (nullptr == convert_buf)
    {
        convert_buf = static_cast<int16_t*>(malloc(tmp_pcm_max_packet_size));
        if (nullptr == convert_buf)
        {
            xerror ("Malloc failed\n");
            return -1;
        }
        else
        {
            convert_buf_size = tmp_pcm_max_packet_size;

            memset (convert_buf, 0, tmp_pcm_max_packet_size);
        }
    }

    if (nullptr == pcm_buf)
    {
        pcm_buf = static_cast<uint8_t*>(malloc(tmp_pcm_buf_size));
        if (nullptr == pcm_buf)
        {
            xerror ("Malloc failed\n");
            return -1;
        }
        else
        {
            pcm_buf_size = tmp_pcm_buf_size;

            memset (pcm_buf, 0, tmp_pcm_buf_size);
            pcm_buf_data_size = 0;
        }
    }

    init_flag = true;

    return 0;
}

bool AacFDKAac::IsInit__() const
{
    return _data->init_flag;
}

unsigned int AacFDKAac::OnePacketPCMSize__() const
{
    ASSERT (IsInit__());

    return _data->aacenc_info.frameLength * PCM_ELEMENT_SIZE;
}

int AacFDKAac::Deinit__()
{
    HANDLE_AACENCODER &handle = _data->handle;

    if (handle != nullptr)
    {
        aacEncClose (& handle);
        handle = nullptr;
    }

    if (nullptr != _data->convert_buf)
    {
        free (_data->convert_buf);
        _data->convert_buf = nullptr;
        _data->convert_buf_size = 0;
    }

    if (nullptr != _data->pcm_buf)
    {
        free (_data->pcm_buf);
        _data->pcm_buf = nullptr;
        _data->pcm_buf_size = 0;
    }

    return 0;
}

int AacFDKAac::CheckEncArgs__ (const AudioInputPCMData *input, 
        const InputArgs *input_args,
        AudioOutputAACData *output,
        OutputArgs *output_args)
{
    const AACENC_InfoStruct &aacenc_info = _data->aacenc_info;

    if (nullptr == input
        || nullptr == input_args
        || nullptr == output
        || nullptr == output_args)
    {
        xerror ("null argument\n");
        return -1;
    }

    if ((input->audio_data_size % PCM_ELEMENT_SIZE) != 0)
    {
        xerror ("pcm size not valid: size=%d\n",
            input->audio_data_size);
        return -1;
    }

    if ((unsigned int)input->audio_data_size > OnePacketPCMSize__())
    {
        xerror ("data size over: %u>%u\n",
            input->audio_data_size, OnePacketPCMSize__());
        return -1;
    }

    if ((unsigned int)output->audio_data_size < aacenc_info.maxOutBufBytes)
    {
        xerror ("output buffer too small(%u < %u)\n",
            output->audio_data_size, aacenc_info.maxOutBufBytes);
        return -1;
    }

    if (input->audio_data_size > 0 && input_args->flush_flag)
    {
        xerror ("flush with input data\n");
        return -1;
    }

    return 0;
}

int AacFDKAac::Encode(const AudioInputPCMData *input, 
        const InputArgs *input_args,
        AudioOutputAACData *output,
        OutputArgs *output_args)
{
    std::lock_guard<std::mutex> lock_guard(_data->mutex);

    uint8_t* &pcm_buf = _data->pcm_buf;
    unsigned int &pcm_buf_size = _data->pcm_buf_size;
    unsigned int &pcm_buf_data_size = _data->pcm_buf_data_size;

    AudioInputPCMData inputex = {};
    const unsigned int one_packet_pcm_size = OnePacketPCMSize__();

    if (!IsInit__())
    {
        xerror ("not init\n");
        return -1;
    }

    if (CheckEncArgs__ (input, input_args, output, output_args) < 0)
    {
        xerror ("args check failed\n");
        return -1;
    }

    ASSERT (input->audio_data_size + pcm_buf_data_size <= pcm_buf_size);

    if (! input_args->flush_flag)
    {
        memcpy (pcm_buf + pcm_buf_data_size, input->audio_data, input->audio_data_size);
        pcm_buf_data_size += input->audio_data_size;
    }
    else
    {
        if (pcm_buf_data_size > 0 
            && pcm_buf_data_size < one_packet_pcm_size)
        {
            xinfo ("fill the remaining buffer\n");
            memset (pcm_buf + pcm_buf_data_size, 0, one_packet_pcm_size - pcm_buf_data_size);
            pcm_buf_data_size = one_packet_pcm_size;
        }
    }

    if (pcm_buf_data_size >= one_packet_pcm_size)
    {
        inputex.audio_data = pcm_buf;
        inputex.audio_data_size = one_packet_pcm_size;
    }
    else
    {
        inputex.audio_data = pcm_buf;
        inputex.audio_data_size = 0;
    }

    if (Encode__ (&inputex, input_args, output, output_args) < 0)
    {
        xerror ("Encode failed\n");
        return -1;
    }

    /*
    关于 pcm_buf 移动的示例:
    + 表示已经送去编码的 PCM 数据
    - 表示剩余的 PCM 数据
    * 表示无效数据
    移动前:  ++++++++++++++---***********
    移动后:  ---*************************
    */
    if (inputex.audio_data_size > 0)
    {
        ASSERT (pcm_buf_data_size - inputex.audio_data_size >= 0);
        memmove (pcm_buf, pcm_buf + one_packet_pcm_size, 
            pcm_buf_data_size - inputex.audio_data_size);

        pcm_buf_data_size -= inputex.audio_data_size;
    }

    return 0;
}

int AacFDKAac::Encode__(const AudioInputPCMData *input, 
        const InputArgs *input_args,
        AudioOutputAACData *output,
        OutputArgs *output_args)
{
    if (PrepareEncodeEnv__(input, input_args, output, output_args) < 0)
    {
        xerror ("Prepare env failed\n");
        return -1;
    }

    HANDLE_AACENCODER &handle = _data->handle;

    int16_t* &convert_buf = _data->convert_buf;

    AACENC_ERROR err;

    AACENC_BufDesc in_buf = { 0 }, out_buf = { 0 };
    AACENC_InArgs in_args = { 0 };
    AACENC_OutArgs out_args = { 0 };

    void* in_bufs[] = {convert_buf};
    int in_identifier = IN_AUDIO_DATA;
    int in_bufs_size[] = {(int)input->audio_data_size};
    int in_bufs_elesizes[] = {PCM_ELEMENT_SIZE};

    void* out_bufs[] = {output->audio_data};
    int out_identifer = OUT_BITSTREAM_DATA;
    int out_bufs_size[] = {(int)output->audio_data_size};
    int out_bufs_elesizes[] = {AAC_ELEMENT_SIZE};

    in_buf.numBufs = sizeof(in_bufs)/sizeof(in_bufs[0]);
    in_buf.bufs = in_bufs;
    in_buf.bufferIdentifiers = &in_identifier;
    in_buf.bufSizes = in_bufs_size;
    in_buf.bufElSizes = in_bufs_elesizes;

    out_buf.numBufs = sizeof(out_bufs)/sizeof(out_bufs[0]);
    out_buf.bufs = out_bufs;
    out_buf.bufferIdentifiers = &out_identifer;
    out_buf.bufSizes = out_bufs_size;
    out_buf.bufElSizes = out_bufs_elesizes;

    in_args.numInSamples = input->audio_data_size / PCM_ELEMENT_SIZE;
    if (input_args->flush_flag && (0 == input->audio_data_size))
    {
        xinfo ("flush\n");
        in_args.numInSamples = -1;
    }

    if ((err = aacEncEncode(handle, &in_buf, &out_buf, &in_args, &out_args))
        != AACENC_OK)
    {
        if (err == AACENC_ENCODE_EOF)
        {
            xinfo ("aac eof\n");
            output_args->eof_flag = true;
        }
        else
        {
            xerror ("Encoding failed, err=%#x\n", err);
            return -1;
        }
    }
    else
    {
        output_args->packet_size = out_args.numOutBytes;
    }

    return 0;
}

int AacFDKAac::GetInfo(AacEncInfo &info) const
{
    std::lock_guard<std::mutex> lock_guard(_data->mutex);
    
    AACENC_InfoStruct &aacenc_info = _data->aacenc_info;

    if (! IsInit__())
    {
        xerror ("Not init\n");
        return -1;
    }

    info.maxInBufBytes = aacenc_info.frameLength * PCM_ELEMENT_SIZE;
    info.maxOutBufBytes = aacenc_info.maxOutBufBytes;

    return 0;
}

// 一些处理
int AacFDKAac::PrepareEncodeEnv__(const AudioInputPCMData *input, 
        const InputArgs *input_args,
        AudioOutputAACData *output,
        OutputArgs *output_args)
{
    unsigned int i = 0;
    
    int16_t* &convert_buf = _data->convert_buf;
    uint32_t &convert_buf_size = _data->convert_buf_size;

    ASSERT ((uint32_t)input->audio_data_size <= convert_buf_size);

    /* PCM 16bit: small endian --> big endian */
    for (i = 0; i < input->audio_data_size / 2; ++i)
    {
        const uint8_t* it_input = &input->audio_data[2*i];
        convert_buf[i] = it_input[0] | (it_input[1] << 8);
    }

    memset (output_args, 0, sizeof(*output_args));

    return 0;
}

NAMESPACE_AAC_END