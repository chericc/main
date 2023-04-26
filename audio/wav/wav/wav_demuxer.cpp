#include "wav_demuxer.hpp"

#include <string.h>

#include "print.hpp"

#define MKTAG(a,b,c,d)   ((a) | ((b) << 8) | ((c) << 16) | ((unsigned)(d) << 24))
#define MKBETAG(a,b,c,d) ((d) | ((c) << 8) | ((b) << 16) | ((unsigned)(a) << 24))
#define BIT2BYTE(value)   ((value) / 8)

WavDemuxer::LoadInfo::~LoadInfo()
{
    if (fp)
    {
        fclose(fp);
        fp = nullptr;
    }
}

WavDemuxer::WavDemuxer(const std::string &filename)
    : _filename(filename)
{
    doLoadFile();
}

WavDemuxer::~WavDemuxer()
{
    doCloseFile();
}

bool WavDemuxer::loadSuccessful()
{
    if (_load_info)
    {
        return true;
    }
    return false;
}

int WavDemuxer::format()
{
    if (_load_info)
    {
        return _load_info->fmt.audio_format;
    }

    return -1;
}

int WavDemuxer::numChannels()
{
    if (_load_info)
    {
        return _load_info->fmt.num_channels;
    }
    return -1;
}

int WavDemuxer::sampleRate()
{
    if (_load_info)
    {
        return _load_info->fmt.sample_rate;
    }
    return -1;
}

int WavDemuxer::sampleBits()
{
    if (_load_info)
    {
        return _load_info->fmt.bits_per_sample;
    }
    return -1;
}

int WavDemuxer::numSamples()
{
    int num_samples = -1;

    do
    {
        if (!_load_info)
        {
            break;
        }

        int bytes_per_sample = BIT2BYTE(_load_info->fmt.bits_per_sample);
        if (bytes_per_sample <= 0)
        {
            break;
        }

        num_samples = _load_info->data_size / bytes_per_sample;
    }
    while(0);

    return num_samples;
}

std::vector<uint8_t> WavDemuxer::getSamples(int ch, int pos, int count)
{
    std::vector<uint8_t> buf;

    do 
    {
        if (!_load_info)
        {
            break;
        }

        if (ch < 0 || pos < 0 || count < 0)
        {
            break;
        }

        int bytes_per_sample = BIT2BYTE(_load_info->fmt.bits_per_sample);
        if (bytes_per_sample <= 0)
        {
            break;
        }

        int pos_in_bytes = pos * bytes_per_sample + _load_info->data_offset;
        int count_in_bytes = count * bytes_per_sample;
        
        buf = readBuf(_load_info, pos_in_bytes, count_in_bytes);
    }
    while (0);

    return buf;
}

int WavDemuxer::doLoadFile()
{
    int berror = false;

    std::shared_ptr<LoadInfo> info;

    info = std::make_shared<LoadInfo>();

    do 
    {
        if (!info->fp)
        {
            info->fp = fopen(_filename.c_str(), "r");
        }

        if (!info->fp)
        {
            LOGE("open file failed");
            berror = true;
            break;
        }

        if (readHeader(info) < 0)
        {
            LOGE("read header failed");
            berror = true;
            break;
        }
    }
    while (0);
    
    if (berror)
    {
        info.reset();
        return -1;
    }

    _load_info = info;

    return 0;
}

int WavDemuxer::doCloseFile()
{
    _load_info.reset();

    return 0;
}

std::vector<uint8_t> WavDemuxer::readBuf(std::shared_ptr<LoadInfo> info, size_t pos, size_t size)
{
    std::vector<uint8_t> buf;
    int berror = false;
    int ret = 0;

    do 
    {
        if (!info || !info->fp)
        {
            berror = true;
            break;
        }

        ret = fseek(info->fp, pos, SEEK_SET);
        if (ret < 0)
        {
            berror = true;
            break;
        }

        buf.resize(size);

        if (size > 0)
        {
            int ret = fread(buf.data(), 1, size, info->fp);
            if (ret != size)
            {
                berror = true;
                break;
            }
        }
    }
    while (0);

    if (berror)
    {
        buf.clear();
    }

    return buf;
}

int WavDemuxer::readHeader(std::shared_ptr<LoadInfo> info)
{
    int berror = false;

    size_t offset = 0;  // 下一个要扫描的块的起始位置
    size_t size = 0;    // 临时读大小

    bool has_riff = false;
    bool has_fmt = false;
    bool has_data = false;

    if (!info)
    {
        LOGE("null info");
        return -1;
    }

    for (;;)
    {
        if (berror)
        {
            break;
        }

        LOGD("offset=%lu", offset);

        // get next tag
        size = sizeof(NA_TAG);
        std::vector<uint8_t> buftag = readBuf(info, offset, size);
        if (!buftag.data() || buftag.size() != size)
        {
            LOGD("no next tag");
            break;
        }

        NA_TAG tag{};
        memcpy(&tag, buftag.data(), size);

        std::vector<uint8_t> buf;
        switch (tag.chunkid)
        {
            case MKTAG('R', 'I', 'F', 'F'):
            {
                LOGD("parsing chunk RIFF");
                size = sizeof(NA_RIFF);
                buf = readBuf(info, offset, size);
                NA_RIFF *nariff = (NA_RIFF*)buf.data();
                if (!nariff || buf.size() != size)
                {
                    LOGE("parse chunk riff failed");
                    berror = true;
                    break;
                }
                
                memcpy(&info->riff, nariff, size);
                offset += size;

                has_riff = true;

                break;
            }
            case MKTAG('f','m','t',' '):
            {
                LOGD("parsing chunk fmt");
                size = sizeof(NA_SubChunkFmt);
                buf = readBuf(info, offset, size);
                NA_SubChunkFmt* nafmt = (NA_SubChunkFmt*)buf.data();
                if (!nafmt || buf.size() != size)
                {
                    LOGE("parse chunk fmt failed");
                    berror = true;
                    break;
                }

                memcpy(&info->fmt, nafmt, size);
                offset += tag.chunksize + sizeof(NA_TAG);

                has_fmt = true;

                break;
            }
            case MKTAG('d', 'a', 't', 'a'):
            {
                LOGD("parsing chunk data");
                size = sizeof(NA_SubChunkData);
                buf = readBuf(info, offset, size);
                NA_SubChunkData* data = (NA_SubChunkData*)buf.data();
                if(!data || buf.size() != size)
                {
                    LOGE("parse chunk data failed");
                    berror = true;
                    break;
                }

                info->data_offset = offset + sizeof(NA_TAG);
                info->data_size = data->tag.chunksize;
                offset += tag.chunksize + sizeof(NA_TAG);

                has_data = true;

                break;
            }
            default:
            {
                LOGD("parsing chunk unknown");
                offset += tag.chunksize + sizeof(NA_TAG);
                break;
            }
        } // switch
    } // for()

    if (!has_riff || !has_fmt || !has_data)
    {
        LOGE("chunk incomplete(riff=%d,fmt=%d,data=%d)", 
            has_riff, has_fmt, has_data);
        berror = true;
    }

    // 为了读取的方便，做出一些限制
    if ((info->fmt.bits_per_sample % 8 != 0)
        || info->fmt.num_channels != 1)
    {
        LOGE("bits_per_sample=%d, num_channels=%d", 
            info->fmt.bits_per_sample,
            info->fmt.num_channels);
        berror = true;
    }

    if (berror)
    {
        return -1;
    }
    return 0;
}
