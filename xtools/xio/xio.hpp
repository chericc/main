/*

Buffered I/O Layer

ref: ffmpeg/libavformat/avio

*/

#pragma once

#include <stdint.h>

#include <string>
#include <vector>

class XIO {
   public:
    XIO(const std::string& url, const std::string& mode);
    virtual ~XIO() = 0;

    virtual int eof() = 0;
    virtual int error() = 0;

    virtual int seek(int64_t offset, int whence) = 0;
    virtual int64_t size() = 0;
    virtual int64_t tell() = 0;

    virtual void flush() = 0;

    // 'l' stands for little endian(in the io device)
    // 'b' stands for big endian

    // read
    virtual uint8_t r8() = 0;
    uint16_t rl16();
    uint32_t rl24();
    uint32_t rl32();
    uint64_t rl64();
    uint16_t rb16();
    uint32_t rb24();
    uint32_t rb32();
    uint64_t rb64();
    virtual std::vector<uint8_t> read(std::size_t size) = 0;

    // write
    virtual void w8(uint8_t b) = 0;
    void wl16(uint16_t value);
    void wl24(uint32_t value);
    void wl32(uint32_t value);
    void wl64(uint64_t value);
    void wb16(uint16_t value);
    void wb24(uint32_t value);
    void wb32(uint32_t value);
    void wb64(uint64_t value);
    virtual void write(const std::vector<uint8_t>& buffer) = 0;

   protected:
    XIO(XIO&) = delete;
    XIO& operator=(XIO&) = delete;
    class IOContext {
       public:
        std::string url;
        std::string mode;
    };

    IOContext ioctx_;
};

class XIOFile : public XIO {
   public:
    XIOFile(const std::string& url, const std::string& mode);
    ~XIOFile() override;

    int eof() override;
    int error() override;

    int seek(int64_t offset, int whence) override;
    int64_t size() override;
    int64_t tell() override;

    void flush() override;

    uint8_t r8() override;
    std::vector<uint8_t> read(std::size_t size) override;

    void w8(uint8_t b) override;
    void write(const std::vector<uint8_t>& buffer) override;

   private:
    class IOFileContenxt {
       public:
        ~IOFileContenxt();
        FILE* fp{nullptr};
    };
    IOFileContenxt iofctx_;
};