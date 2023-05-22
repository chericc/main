/**
 * Author: x
 * Date: 20210503
 * Description: rgba --> 1555,...
 * 
 * 注：
 * 对于 RGBA，ALPHA 0 表示不透明，255 表示全透明
 * 
 * RGBA
 * 透明 [x,x,x,255]
 * 红色 [255,0,0,0]
 * 绿色 [0,255,0,0]
 * 蓝色 [0,0,255,0]
 * 白色 [255,255,255,0]
 * 黑色 [0,0,0,0]
 * 
 * 1555 格式的字节排列按照书写的顺序排列，或者说是大端字节序（即地址从低到高依次是alpha,red,green,blue）。
 * 0x0000 表示黑色
 * 0x8C00 表示红色
 */

#pragma once

typedef enum 
{
    /* rgba --> 1555 */
    IMAGE_RAW_RGBA_1555 = 0x1,  // 1555 中 1 表示透明
    IMAGE_RAW_RGBA_1555_1ALPHA, // 1555 中 1 表示透明
    IMAGE_RAW_RGBA_1555_0ALPHA, // 1555 中 0 表示透明

    /* 位图 --> 1555 */
    IMAGE_RAW_HBIT_1555_1BLACK_1ALPHA = 0x10,        // HBIT: BYTE & 0x80 表示第一个像素，1 表示 黑色，alpha 为 1
    IMAGE_RAW_HBIT_1555_1BLACK_0ALPHA,        // HBIT: BYTE & 0x80 表示第一个像素，1 表示 黑色，alpha 为 0
    IMAGE_RAW_HBIT_1555_1WHITE_1ALPHA,        // HBIT: BYTE & 0x80 表示第一个像素，1 表示 白色，alpha 为 1
    IMAGE_RAW_HBIT_1555_1WHITE_0ALPHA,        // HBIT: BYTE & 0x80 表示第一个像素，1 表示 白色，alpha 为 0
} IMAGE_RAW_CONVERT_TYPE_E;

typedef enum
{
    IMAGE_RAW_RGBA = 0x1,
    IMAGE_RAW_1555, // 1555 中 1 表示透明
    IMAGE_RAW_1555_1ALPHA,  // 1555 中 1 表示透明
    IMAGE_RAW_1555_0ALPHA,  // 1555 中 0 表示透明
} IMAGE_RAW_TYPE_E;

typedef enum
{
    IMAGE_DRAW_COVER = 0, // 总是覆盖掉底色
    IMAGE_DRAW_MIX,     // 如果绘制的颜色为透明色，则不覆盖底色
} IMAGE_DRAW_MODE;

#ifdef __cplusplus
extern "C" {
#endif 

int imagetools_rawconvert (const void *pSrc, unsigned int nSrcSize, 
    void *pDst, unsigned int nDstSize, IMAGE_RAW_CONVERT_TYPE_E eConvType);

int imagetools_rawconvert_HBIT_to_1555(const void *pSrc, unsigned int nSrcSize, 
    void *pDst, unsigned int nDstSize, const void *pPixel1555_0, const void *pPixel1555_1);

int imagetools_rawconvert_HBIT_to_RGBA(const void *pSrc, unsigned int nSrcSize, 
    void *pDst, unsigned int nDstSize, const void *pPixelRGBA_0, const void *pPixelRGBA_1);

int imagetools_scale(const void *pSrc, unsigned int nSrcW, unsigned int nSrcH, 
    void *pDst, unsigned int nDstW, unsigned int nDstH, IMAGE_RAW_TYPE_E eImageRawType);

int imagetools_replaceall (void *pPixels, unsigned int nPixelCount, 
    const void *pOldPixel, const void *pNewPixel, IMAGE_RAW_TYPE_E eImageRawType);

int imagetools_settwocolor (void *pPixels, unsigned int nPixelCount, 
    const void *pBackColor, const void *pBackToColor, const void* pFrontToColor, IMAGE_RAW_TYPE_E eImageRawType);

int imagetools_drawimage (void *pDstPixels, unsigned int nDstWidth, unsigned int nDstHeight, 
    const void *pSrcPixels, unsigned int nSrcWidth, unsigned int nSrcHeight, 
    int nX, int nY, IMAGE_DRAW_MODE eDrawMode, IMAGE_RAW_TYPE_E eImageRawType);

int imagetools_brushrect (void *pDstPixels, unsigned int nDstWidth, unsigned int nDstHeight,
    const void *pSrcPixels, unsigned int nSrcWidth, unsigned int nSrcHeight, 
    int nX, int nY, IMAGE_DRAW_MODE eDrawMode, IMAGE_RAW_TYPE_E eImageRawType);

#ifdef __cplusplus
}
#endif