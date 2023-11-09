
#include "xio.hpp"

#include <stdio.h>

#include "xlog.hpp"
#include "xos_independent.hpp"

XIO::XIO(const std::string &url, const std::string &mode)
{
    ioctx_.url = url;
    ioctx_.mode = mode;
}

XIO::~XIO()
{}

uint16_t XIO::rl16()
{
    uint16_t value;
    value = r8();
    value |= ((uint16_t)r8() << 8);
    return value;
}

uint32_t XIO::rl24()
{
    uint32_t value;
    value = rl16();
    value |= ((uint32_t)r8() << 16);
    return value;
}

uint32_t XIO::rl32()
{
    uint32_t value;
    value = rl16();
    value |= ((uint32_t)rl16() << 16);
    return value;
}

uint64_t XIO::rl64()
{
    uint64_t value;
    value = rl32();
    value |= ((uint64_t)rl32() << 32);
    return value;
}

uint16_t XIO::rb16()
{
    uint16_t value;
    value = ((uint16_t)r8() << 8);
    value |= r8();
    return value;
}

uint32_t XIO::rb24()
{
    uint32_t value;
    value = (uint32_t)rb16() << 8;
    value |= (uint32_t)r8();
    return value;
}

uint32_t XIO::rb32()
{
    uint32_t value;
    value = (uint32_t)rb16() << 16;
    value |= rb16();
    return value;
}

uint64_t XIO::rb64()
{
    uint64_t value;
    value = (uint64_t)rb32() << 32;
    value |= rb32();
    return value;
}

void XIO::wl16(uint16_t value)
{
    w8((uint8_t)value);
    w8(value >> 8);
}

void XIO::wl24(uint32_t value)
{
    wl16((uint16_t)value);
    w8(value >> 16);
}

void XIO::wl32(uint32_t value)
{
    wl16((uint16_t)value);
    wl16(value >> 16);
}

void XIO::wl64(uint64_t value)
{
    wl32((uint32_t)value);
    wl32(value >> 32);
}

void XIO::wb16(uint16_t value)
{
    w8(value >> 8);
    w8((uint8_t)value);
}

void XIO::wb24(uint32_t value)
{
    wb16((uint16_t)(value >> 8));
    w8((uint8_t)value);
}

void XIO::wb32(uint32_t value)
{
    wb16((uint16_t)(value >> 16));
    wb16((uint16_t)value);
}

void XIO::wb64(uint64_t value)
{
    wb32((uint32_t)(value >> 32));
    wb32((uint32_t)value);
}

XIOFile::IOFileContenxt::~IOFileContenxt()
{
    if (fp)
    {
        fclose(fp);
        fp = nullptr;
    }
}

XIOFile::XIOFile(const std::string &url, const std::string &mode)
    : XIO(url, mode)
{
    int berror = false;

    do
    {
        if (ioctx_.url.empty())
        {
            berror = true;
            break;
        }

        iofctx_.fp = fopen(ioctx_.url.c_str(), mode.c_str());
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
        xlog_err("init failed");
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

        if (x_fseek64(iofctx_.fp, offset, whence) < 0)
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

        long old_offset = x_ftell64(iofctx_.fp);

        if (old_offset < 0)
        {
            berror = true;
            break;
        }

        if (x_fseek64(iofctx_.fp, 0, SEEK_END) < 0)
        {
            berror = true;
            break;
        }

        long size = x_ftell64(iofctx_.fp);
        if (size < 0)
        {
            berror = true;
            break;
        }

        if (x_fseek64(iofctx_.fp, old_offset, SEEK_SET) < 0)
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

        long tell_ret = x_ftell64(iofctx_.fp);
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
        
        fflush(iofctx_.fp);
    }
    while (0);
}

uint8_t XIOFile::r8()
{
    int berror = false;
    uint8_t buf[1] = { 0 };

    do 
    {
        if (error())
        {
            berror = true; 
            break;
        }
        
        if (fread(buf, 1, sizeof(buf), iofctx_.fp) != sizeof(buf))
        {
            return 0;
        }
    }
    while (0);

    if (berror)
    {
        xlog_err("read with error");
    }
    
    return buf[0];
}

std::vector<uint8_t> XIOFile::read(std::size_t size)
{
    int berror = false;
    std::vector<uint8_t> buf;

    do
    {
        if (error())
        {
            berror = true;
            break;
        }

        buf.resize(size);

        if (fread(buf.data(), 1, size, iofctx_.fp) != size)
        {
            berror = true;
            break;
        }
    }
    while (0);

    return (berror ? std::vector<uint8_t>() : buf);
}

void XIOFile::w8(uint8_t b)
{
    int berror = false;
    uint8_t buf[1] = {b};

    do 
    {
        if (error())
        {
            berror = true;
            break;
        }

        if (fwrite(buf, 1, sizeof(buf), iofctx_.fp) != sizeof(buf))
        {
            break;
        }
    }
    while (0);

    if (berror)
    {
        xlog_err("write with error");
    }

    return ;
}

void XIOFile::write(const std::vector<uint8_t> &buffer)
{
    int berror = false;

    do 
    {
        if (error())
        {
            berror = true;
            break;
        }

        if (fwrite(buffer.data(), 1, buffer.size(), iofctx_.fp) != buffer.size())
        {
            berror = true;
            break;
        }
    }
    while(0);

    if (berror)
    {
        xlog_err("write failed");
    }

    return ;
}