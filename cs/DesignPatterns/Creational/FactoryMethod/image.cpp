#include "image.hpp"

#include <stdio.h>

#define xdebug(x...) do {printf("[debug][%s %d %s]", \
	__FILE__,__LINE__,__FUNCTION__);printf(x);} while (0)

void Image::openImageFile(const std::string &filename)
{
    if (std::string::npos != filename.find(".jpg")
        || std::string::npos != filename.find(".jpeg"))
    {
        decoder = std::make_shared<ImageDecoderJpeg>();
    }
    else if (std::string::npos != filename.find(".bmp"))
    {
        decoder = std::make_shared<ImageDecoderBMP>();
    }
    else if (std::string::npos != filename.find(".png"))
    {
        decoder = std::make_shared<ImageDecoderPNG>();
    }
    else 
    {
        xdebug ("Image format not support\n");
    }

    if (decoder)
    {
        decoder->decodeImage(filename);
    }
    

    return ;
}

void Image::drawLine()
{
    xdebug ("Drawing line on image\n");
}

void Image::brushRect()
{
    xdebug ("Brush rect on image\n");
}

void ImageDecoderJpeg::decodeImage(const std::string &filename)
{
    xdebug ("Decoding file <%s> with jpeg lib\n", filename.c_str());
}

void ImageDecoderBMP::decodeImage(const std::string &filename)
{
    xdebug ("Decoding file <%s> with bmp lib\n", filename.c_str());
}

void ImageDecoderPNG::decodeImage(const std::string &filename)
{
    xdebug ("Decoding file <%s> with png lib\n", filename.c_str());
}