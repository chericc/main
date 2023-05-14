

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
public:


    typedef enum {
        PIXFMT_NONE     = -1,
        PIXFMT_BGRA     = 0,
        PIXFMT_BGR24,
        PIXFMT_RGB555,
    } PixFmt;
private:

    
    typedef enum {
        BMP_RGB         =0,
        BMP_RLE8        =1,
        BMP_RLE4        =2,
        BMP_BITFIELDS   =3,
    } BiCompression;

    /*
    File Header
    Bitmap Header
    Color Palette
    Bitmap Data
    */

    struct BMP_FileHeader
    {
        /* file header size = 14 */

        uint16_t filetype;      /* File type, always "BM" */
        uint32_t filesize;      /* Size of the file in bytes */
        uint16_t reserved1;     /* Always 0 */
        uint16_t reserved2;     /* Always 0 */
        uint32_t bitmapoffset;  /* Starting position of image data in bytes. */
    };

    struct BMP_BitmapHeader
    {
        /* v2: header size = 12 */

        uint32_t size;          /* size of this header in bytes */
        union
        {
        int16_t width_v2;         /* image width in pixels */
        int32_t width_v3;         /* image width in pixels */
        };
        union
        {
        int16_t height_v2;        /* image height in pixels */
        int32_t height_v3;        /* image height in pixels */
        };
        uint16_t planes;        /* number of color planes */
        uint16_t bitsperpixel;  /* number of bits per pixel */

        /* v3: header size = 16 + 24 = 40 */

        uint32_t compression;   /* >=v3: compression methods used */
        uint32_t sizeofbitmap;  /* size of bitmap in bytes */
        uint32_t horzresolution;    /* h resolution in pixels per meter */
        uint32_t vertresolution;    /* v resolution in pixels per meter */
        uint32_t colorsused;    /* number of colors in the image */
        uint32_t colorsimportant;   /* minimum number of important colors */

        /* v4: header size = 40 + 68 = 108 */
        
        uint32_t redmask;       /* mask identifying bits of red component */
        uint32_t greenmask;     /* mask identifying bits of green component */
        uint32_t bluemask;      /* mask identifying bits of blue component */
        uint32_t alphamask;     /* mask identifying bits of alpha component */
        uint32_t cstype;        /* color space type */
        uint32_t redx;
        uint32_t redy;
        uint32_t redz;
        uint32_t greenx;
        uint32_t greeny;
        uint32_t greenz;
        uint32_t bluex;
        uint32_t bluey;
        uint32_t bluez;
        uint32_t gammared;
        uint32_t gammagreen;
        uint32_t gammablue;

    };

    class LoadInfo
    {
    public:
        std::shared_ptr<XIO> xio;
        int32_t width{};
        int32_t height{};
        size_t data_offset{};
        size_t data_size{};
        PixFmt pixfmt{};

        BMP_FileHeader fileheader{};
        BMP_BitmapHeader bitmapheader{};
    };
public:
    BmpDecoder(const std::string &file);
    ~BmpDecoder();

    bool loadSuccessful();

    int width();
    int height();

    PixFmt pixfmt();

    /* 注：BMP图像数据的扫描方式是从左至右，从下到上的 */
    std::vector<uint8_t> getContent(int pos, int numPixels);

private:
    int doLoadFile();
    int doCloseFile();

    int readHeader(std::shared_ptr<LoadInfo> info);

    int pixelDepthBytes(PixFmt pixfmt);

    const std::string _filename;
    std::shared_ptr<LoadInfo> _load_info;
};