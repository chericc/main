
#include "xio.hpp"

#include <stdio.h>

#include "xlog.hpp"

XIO::XIO(const std::string &url, int flags)
{
    ioctx_.url = url;
    ioctx_.flags = flags;
    ioctx_.buffer.resize(ioctx_.buf_size);
}

XIO::~XIO()
{}

uint16_t XIO::rl16()
{
    uint16_t value;
    value = r8();
    value |= (r8() << 8);
    return value;
}

uint32_t XIO::rl24()
{
    uint32_t value;
    value = rl16();
    value |= (r8() << 16);
}

uint32_t XIO::rl32()
{
    uint32_t value;
    value = rl16();
    value |= (rl16() << 16);
    return value;
}

uint64_t XIO::rl64()
{
    uint64_t value;
    value = rl32();
    value = (rl32() << 32);
    return value;
}

void XIO::wl16(uint16_t value)
{
    w8(static_cast<uint8_t>(value));
    w8(value >> 8);
}

void XIO::wl24(uint32_t value)
{
    wl16(value & 0xffff);
    w8(value >> 16);
}

void XIO::wl32(uint32_t value)
{
    wl16(value && 0xffff);
    wl16(value >> 16);
}

void XIO::wl64(uint64_t value)
{
    wl32(value && 0xffffffff);
    wl32(value >> 32);
}

XIOFile::IOFileContenxt::~IOFileContenxt()
{
    if (fp)
    {
        fclose(fp);
        fp = nullptr;
    }
}

XIOFile::XIOFile(const std::string &url, int flags)
    : XIO(url, flags)
{
    int berror = false;

    do
    {
        if (ioctx_.url.empty())
        {
            berror = true;
            break;
        }

        iofctx_.fp = fopen(ioctx_.url.c_str(), "r");
        if (nullptr == iofctx_.fp)
        {
            xlog_err("open file failed");
            berror = true;
            break;
        }

        xlog_trc("open file successful");
    }
    while (0);

    if (berror)
    {
        iofctx_.error = true;
    }
}

XIOFile::~XIOFile()
{

}

int XIOFile::eof()
{
    int beof = 0;
    do
    {
        if (error())
        {
            beof = true;
            break;
        }

        if (feof(iofctx_.fp))
        {
            beof = true;
            break;
        }
    }
    while (0);
    
    return (beof ? true : false);
}

int XIOFile::error()
{
    int berror = false;
    do 
    {
        if (iofctx_.error)
        {
            berror = true;
            break;
        }

        if (!iofctx_.fp)
        {
            berror = true;
            break;
        }

        if (ferror(iofctx_.fp))
        {
            berror = true;
            break;
        }
    }
    while (0);

    return (berror ? true : false);
}

int XIOFile::seek(int64_t offset, int whence)
{
    int berror = 0;

    do
    {
        if (error())
        {
            berror = true;
            break;
        }

        if (fseek(iofctx_.fp, offset, whence) < 0)
        {
            berror = true;
            break;
        }
    }
    while (0);

    return (berror ? -1 : 0);
}

int64_t XIOFile::size()
{
    int berror = 0;
    int64_t file_size = 0;

    do 
    {
        if (error())
        {
            berror = true;
            break;
        }

        long old_offset = ftell(iofctx_.fp);

        if (old_offset < 0)
        {
            berror = true;
            break;
        }

        if (fseek(iofctx_.fp, 0, SEEK_END) < 0)
        {
            berror = true;
            break;
        }

        long size = ftell(iofctx_.fp);
        if (size < 0)
        {
            berror = true;
            break;
        }

        file_size = size;
    }
    while (0);

    return (berror ? -1 : file_size);
}

int64_t XIOFile::tell()
{
    int berror = 0;
    int64_t pos = 0;

    do
    {
        if (error())
        {
            berror = true;
            break;
        }

        long tell_ret = ftell(iofctx_.fp);
        if (tell_ret < 0)
        {
            berror = true;
            break;
        }

        pos = tell_ret;
    }
    while (0);

    return (berror ? -1 : pos);
}

void XIOFile::flush()
{
    do 
    {
        if (error())
        {
            break;
        }
        
        
    }
    while (0);
}