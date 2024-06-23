#include "image2rgba.h"

#include <string.h>

#include "image_bmp.h"
#include "image_jpeg.h"

IMAGE_NAME_IMPORT;

#define BMP_SUFFIX ".bmp"
#define JPEG_SUFFIX ".jpeg"
#define JPEG_SUFFIX_2 ".jpg"

HANDLE_IMAGE2RGBA image2rgba_open(const char* szFile) {
    ImageInterface* pObj = nullptr;

    do {
        IMAGE_FILE_TYPE_E eType = IMAGE_UNKNOWN;
        if (!ImageInterface::GuessImageType(szFile, &eType)) {
            _error("guess failed\n");
            break;
        } else {
            switch (eType) {
                case IMAGE_BMP: {
                    pObj = new ImageBMP();
                } break;
                case IMAGE_JPEG: {
                    pObj = new ImageJpeg();
                } break;
                default: {
                    // blank
                } break;
            }
        }

        if (pObj) {
            pObj->LoadFile(szFile);
        }
    } while (0);

    return pObj;
}

int image2rgba_close(HANDLE_IMAGE2RGBA handle) {
    ImageInterface* pObj = static_cast<ImageInterface*>(handle);
    if (nullptr != pObj) {
        delete pObj;
        pObj = nullptr;
    }

    return 0;
}

int image2rgba_getsize(HANDLE_IMAGE2RGBA handle, int* pW, int* pH) {
    ImageInterface* pObj = static_cast<ImageInterface*>(handle);

    *pW = pObj->Width();
    *pH = pObj->Height();
    return 0;
}

int image2rgba_getpixel(HANDLE_IMAGE2RGBA handle, int i, int j,
                        Image2RGBA_Pixel* pPixel) {
    ImageInterface* pObj = static_cast<ImageInterface*>(handle);

    bool ret = pObj->GetPixel(i, j, *pPixel);

    return ret ? 0 : -1;
}

int image2rgba_settotwocolor(HANDLE_IMAGE2RGBA handle,
                             const Image2RGBA_Pixel* cpLightPixel,
                             const Image2RGBA_Pixel* cpDarkPixel,
                             const Image2RGBA_Pixel* cpMidPixel) {
    ImageInterface* pObj = static_cast<ImageInterface*>(handle);

    int ret = pObj->SetToTwoColor(*cpLightPixel, *cpDarkPixel, *cpMidPixel);

    _debug("replace %d pixels\n", ret);

    return (ret < 0) ? -1 : 0;
}