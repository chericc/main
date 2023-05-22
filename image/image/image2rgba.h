/**
 * Author: x
 * Date: 20210501
 * Description: image to rgba
 * 
 * 注：
 * 对于 RGBA，ALPHA 0 表示不透明，255 表示全透明
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif 

typedef struct Pixel
{
    unsigned char R;
    unsigned char G;
    unsigned char B;
    unsigned char A;
} Image2RGBA_Pixel;

typedef void* HANDLE_IMAGE2RGBA;

HANDLE_IMAGE2RGBA       image2rgba_open(const char *szFile);
int                     image2rgba_close(HANDLE_IMAGE2RGBA handle);

int image2rgba_getsize(HANDLE_IMAGE2RGBA handle, int *pW, int *pH);
int image2rgba_getpixel(HANDLE_IMAGE2RGBA handle, int i, int j, Image2RGBA_Pixel *pPixel);

int image2rgba_settotwocolor (HANDLE_IMAGE2RGBA handle, const Image2RGBA_Pixel *cpLightPixel, const Image2RGBA_Pixel *cpDarkPixel, 
    const Image2RGBA_Pixel *cpMidPixel);

#ifdef __cplusplus
}
#endif