#pragma once

#include "image_interface.h"

IMAGE_NAME_START

class ImageJpeg : public ImageInterface {
   protected:
    bool LoadFile_override() override;
};

IMAGE_NAME_END