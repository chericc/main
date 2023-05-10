

/*
https://docs.fileformat.com/image/bmp/
https://www.fileformat.info/format/bmp/egff.htm
ffmpeg/libavcodec/bmp
*/

#include <string>
#include <stdint.h>
#include <vector>
#include <memory>

#include "xio.hpp"

class BmpDecoder
{
private:

    /*
    File Header
    Bitmap Header
    Color Palette
    Bitmap Data
    */

    struct BMP_FileHeader
    {
        uint16_t filetype;      /* File type, always "BM" */
        uint32_t filesize;      /* Size of the file in bytes */
        uint16_t reserved1;     /* Always 0 */
        uint16_t reserved2;     /* Always 0 */
        uint32_t bitmapoffset;  /* Starting position of image data in bytes. */
    };

    struct BMP_BitmapHeader
    {
        uint32_t size;
        uint16_t width;
        uint16_t height;
        uint16_t planes;
        uint16_t bitsperpixel;
    };

    class LoadInfo
    {
    public:
        std::shared_ptr<XIO> xio;
        size_t data_offset{};
        size_t data_size{};

        BMP_FileHeader fileheader{};
        BMP_BitmapHeader bitmapheader{};
    };
public:
    BmpDecoder(const std::string &file);
    ~BmpDecoder();

    bool loadSuccessful();

private:
    int doLoadFile();
    int doCloseFile();

    int readHeader(std::shared_ptr<LoadInfo> info);

    const std::string _filename;
    std::shared_ptr<LoadInfo> _load_info;
};