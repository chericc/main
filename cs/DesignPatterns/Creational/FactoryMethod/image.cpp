#include "image.hpp"

#include "xlog.hpp"

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
        xlog_dbg("Image format not support");
    }

    if (decoder)
    {
        decoder->decodeImage(filename);
    }
    

    return ;
}

void Image::drawLine()
{
    xlog_dbg("Drawing line on image");
}

void Image::brushRect()
{
    xlog_dbg("Brush rect on image");
}

void ImageDecoderJpeg::decodeImage(const std::string &filename)
{
    xlog_dbg("Decoding file <%s> with jpeg lib", filename.c_str());
}

void ImageDecoderBMP::decodeImage(const std::string &filename)
{
    xlog_dbg("Decoding file <%s> with bmp lib", filename.c_str());
}

void ImageDecoderPNG::decodeImage(const std::string &filename)
{
    xlog_dbg("Decoding file <%s> with png lib", filename.c_str());
}