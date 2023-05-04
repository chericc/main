#include "bmp.hpp"

#include "xlog.hpp"

BmpDecoder::LoadInfo::~LoadInfo()
{
    if (fp)
    {
        fclose(fp);
        fp = nullptr;
    }
}

BmpDecoder::BmpDecoder(const std::string &filename)
    : _filename(filename)
{
    doLoadFile();
}

BmpDecoder::~BmpDecoder()
{
    doCloseFile();
}

bool BmpDecoder::loadSuccessful()
{
    if (_load_info)
    {
        return true;
    }
    return false;
}

int BmpDecoder::doLoadFile()
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
            xlog_err("open file failed");
            berror = true;
            break;
        }

        if (readHeader(info) < 0)
        {
            xlog_err("read header failed");
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

    return 0;
}

int BmpDecoder::doCloseFile()
{
    _load_info.reset();
    return 0;
}

int BmpDecoder::readHeader(std::shared_ptr<LoadInfo> info)
{
    int berror = false;

    size_t offset = 0;  // 下一个要扫描的快的起始位置

    if (!info)
    {
        xlog_err("null info");
        return -1;
    }

    for (;;)
    {
        if (berror)
        {
            break;
        }

        
    }
}

std::vector<uint8_t> BmpDecoder::readBuf(std::shared_ptr<LoadInfo> info, size_t pos, size_t size)
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