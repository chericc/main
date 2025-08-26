/*

Buffered I/O Layer

ref: ffmpeg/libavformat/avio

*/

#pragma once

#include "xio.hpp"

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