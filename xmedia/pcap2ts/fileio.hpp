#include <memory>

#include "xio.hpp"

/* An xio wrapper with endian state. */
class FileIO {
   public:
    FileIO(const std::string& url, const std::string& mode);
    ~FileIO();

    bool ok();
    void setLittleEndian(bool isLittleEndian);

    uint8_t r8();
    uint16_t r16();
    uint32_t r24();
    uint32_t r32();
    uint64_t r64();
    std::vector<uint8_t> read(std::size_t size);

    int64_t tell();
    int seek(int64_t offset, int whence);
    int eof();

   private:
    bool _lendian = true;
    std::shared_ptr<XIO> _xio;
};