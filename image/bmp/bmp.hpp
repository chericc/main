

/*
https://docs.fileformat.com/image/bmp/
https://www.fileformat.info/format/bmp/egff.htm
ffmpeg/libavcodec/bmp
*/

#include <string>
#include <stdint.h>
#include <vector>
#include <memory>

class BmpDecoder
{
private:

/*
File Header
Bitmap Header
Color Palette
Bitmap Data
*/

    struct NA_BMPFileHeader
    {
        uint16_t filetype;      /* File type, always "BM" */
        uint32_t filesize;      /* Size of the file in bytes */
        uint16_t reserved1;     /* Always 0 */
        uint16_t reserved2;     /* Always 0 */
        uint32_t bitmapoffset;  /* Starting position of image data in bytes. */
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