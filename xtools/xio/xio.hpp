/*

Buffered I/O Layer

ref: ffmpeg/libavformat/avio

*/

#pragma once

#include <string>
#include <stdint.h>
#include <vector>

class XIO
{
public:
    XIO(const std::string &url, int flags);
    virtual ~XIO() = 0;

    virtual void feof() = 0;
    virtual int error() = 0;

    virtual int seek(std::size_t offset, int whence) = 0;
    virtual std::size_t size() = 0;

    virtual void flush() = 0;

    // read
    virtual uint8_t r8() = 0;
    uint16_t rl16();
    uint32_t rl24();
    uint32_t rl32();
    uint64_t rl64();
    virtual std::vector<uint8_t> read(std::size_t size) = 0;

    // write
    virtual void w8(uint8_t b) = 0;
    void wl16(uint16_t value);
    void wl24(uint32_t value);
    void wl32(uint32_t value);
    void wl64(uint64_t value);
    virtual void write(std::vector<uint8_t> buffer) = 0;

protected:
    struct IOContext
    {
        std::string url;
        int flags{0};

        std::size_t buf_size{64 * 1024};
        std::vector<uint8_t> buffer;
        std::size_t buf_content_size{0};
    };

    IOContext ioctx_;
};

class XIOFile : public XIO
{
public:
    XIOFile(const std::string &url, int flags);
    ~XIOFile() override;

    void feof() override;
    int error() override;

    int seek(std::size_t offset, int whence) override;
    std::size_t size() override;

    void flush() override;

    uint8_t r8() override;
    std::vector<uint8_t> read(std::size_t size) override;

    void w8(uint8_t b) override;
    void write(std::vector<uint8_t> buffer) override;
private:
    struct IOFileContenxt
    {
        FILE *fp{nullptr};
    };
    IOFileContenxt iofctx_;
};