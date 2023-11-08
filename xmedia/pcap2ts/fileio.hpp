#include <memory>

#include "xio.hpp"

/* An xio wrapper with endian state. */
class FileIO
{
public:
    FileIO(const std::string &url, const std::string &mode);
    ~FileIO();

    bool ok();
    void setLittleEndian(bool isLittleEndian);

    uint8_t r8();
    uint16_t r16();
    uint32_t r24();
    uint32_t r32();
    uint64_t r64();

    uint64_t tell();
    int seek(uint64_t offset, int whence);
private:
    bool _lendian = true;
    std::shared_ptr<XIO> _xio;
};