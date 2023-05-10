#include "bmp.hpp"

#include "xlog.hpp"

#define MKLE16(a,b) ((a) | ((uint16_t)b << 8))

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
        if (!info->xio)
        {
            info->xio = std::make_shared<XIOFile>(_filename, 0);
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

    _load_info = info;

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
    std::vector<uint8_t> buf;

    size_t offset = 0;  // 下一个要扫描的快的起始位置
    size_t size = 0;    // 

    if (!info || !info->xio)
    {
        xlog_err("null");
        return -1;
    }

    do
    {
        info->xio->seek(0, SEEK_SET);

        if (info->xio->size() < 14)
        {
            berror = true;
            xlog_err("size too small");
            break;
        }

        info->fileheader.filetype = info->xio->rl16();
        if (info->fileheader.filetype != MKLE16('B','M'))
        {
            berror = true;
            xlog_err("magic number error");
            break;
        }

        info->fileheader.filesize = info->xio->rl32();
        if (info->xio->size() < info->fileheader.filesize)
        {
            berror = true;
            xlog_err("size check failed");
            break;
        }

        info->fileheader.reserved1 = info->xio->rl16();
        info->fileheader.reserved2 = info->xio->rl16();

        info->fileheader.bitmapoffset = info->xio->rl32();
        info->bitmapheader.size = info->xio->rl32();

        xlog_trc("filesize=%d,headersize=%d", 
            (int)info->fileheader.filesize, 
            (int)info->bitmapheader.size);
        
        switch (info->bitmapheader.size)
        {
            case  40: // windib
            case  56: // windib v3
            case  64: // OS/2 v2
            case 108: // windib v4
            case 124: // windib v5
            {
                info->bitmapheader.width = info->xio->rl32();
                info->bitmapheader.height = info->xio->rl32();
                break;
            }
            case 12: // OS/2 v1
            {
                info->bitmapheader.width = info->xio->rl16();
                info->bitmapheader.height = info->xio->rl16();
                break;
            }
            default:
            {
                berror = true;
                break;
            }
        } // end of switch

        if (berror)
        {
            xlog_err("header not support(size=%d)", (int)info->bitmapheader.size);
            break;
        }

        xlog_trc("width=%d,height=%d", 
            (int)info->bitmapheader.width,
            (int)info->bitmapheader.height);

        /* planes */
        if (info->xio->rl16() != 1)
        {
            xlog_err("invalid BMP header");
            berror = true;
            break;
        }

        info->bitmapheader.bitsperpixel = info->xio->rl16();

        if (info->bitmapheader.size >= 40)
        {
            
        }
    }
    while (0);

    if (berror)
    {
        return -1;
    }

    return 0;
}
