
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

