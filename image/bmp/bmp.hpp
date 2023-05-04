

/*
https://docs.fileformat.com/image/bmp/
ffmpeg/libavcodec/bmp
*/

#include <string>
#include <stdint.h>
#include <vector>
#include <memory>

class BmpDecoder
{
private:
    struct NA_BitmapFileHeader
    {
        uint8_t tag[2];
        uint32_t size;
        
    } __attribute__((packed));

    class LoadInfo
    {
    public:
        FILE *fp{nullptr};
        size_t data_offset{};
        size_t data_size{};

        ~LoadInfo();
    };
public:
    BmpDecoder(const std::string &file);
    ~BmpDecoder();

    bool loadSuccessful();

private:
    int doLoadFile();
    int doCloseFile();

    int readHeader(std::shared_ptr<LoadInfo> info);

    std::vector<uint8_t> readBuf(std::shared_ptr<LoadInfo> info, size_t pos, size_t size);

    const std::string _filename;
    std::shared_ptr<LoadInfo> _load_info;
};