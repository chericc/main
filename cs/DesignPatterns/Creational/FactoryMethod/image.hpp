#pragma once

#include <memory>
#include <string>

class Image;
class ImageDecoder;
class ImageDecoderJpeg;
class ImageDecoderBMP;
class ImageDecoderPNG;

class Image
{
public:
    void openImageFile(const std::string &filename);
    void drawLine();
    void brushRect();
private:
    std::shared_ptr<ImageDecoder> decoder;
};

class ImageDecoder
{
public:
    virtual ~ImageDecoder() = default;
    virtual void decodeImage(const std::string &filename) = 0;
};

class ImageDecoderJpeg : public ImageDecoder
{
public:
    void decodeImage(const std::string &filename) override;
};

class ImageDecoderBMP : public ImageDecoder
{
public:
    void decodeImage(const std::string &filename) override;
};

class ImageDecoderPNG : public ImageDecoder
{
public:
    void decodeImage(const std::string &filename) override;
};