#include "image_tools.h"

#include <stdint.h>
#include <string.h>
#include <math.h>

#include "def_inner.h"

typedef uint8_t UBYTE;
typedef uint32_t U32_RGBA;
typedef uint16_t U16_1555;

typedef struct C_RGBA
{
    UBYTE red;
    UBYTE green;
    UBYTE blue;
    UBYTE alpha;
} __attribute__((packed)) C_RGBA;

typedef struct C_1555_SmallEndian
{
    UBYTE blue:5;
    UBYTE green:5;
    UBYTE red:5;
    UBYTE alpha:1;
}  __attribute__((packed)) C_1555_SmallEndian;

typedef struct C_1555_BigEndian
{
    UBYTE alpha:1;
    UBYTE red:5;
    UBYTE green:5;
    UBYTE blue:5;
}  __attribute__((packed)) C_1555_BigEndian;

static int imagetools_smallendian()
{
    unsigned int u32 = 0x1;
    char *pu8 = (char*)&u32;
    if (*pu8)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/**
 * @param bLowAlpha: 为真，则 1555 中，0 表示透明，1 表示不透明
 */
static int imagetools_rawconvert_RGBA_to_1555(const void *pSrc, unsigned int nSrcSize, 
    void *pDst, unsigned int nDstSize, int bLowAlpha)
{
    const U32_RGBA *p32Src = (const U32_RGBA*) pSrc;
    U16_1555 *p16Dst = (U16_1555*) pDst;
    int n32SrcSize = nSrcSize / sizeof(U32_RGBA);
    int n16DstSize = nDstSize / sizeof(U16_1555);

    int bSmallEndian = imagetools_smallendian();

    UBYTE nTransparent = (bLowAlpha ? 0 : 1);
    UBYTE nDisTransparent = (bLowAlpha ? 1 : 0);

    int i = 0;
    int j = 0;
    for (i = 0, j = 0; i < n32SrcSize && j < n16DstSize; ++i, ++j)
    {
        const C_RGBA *pCRGBA = (const C_RGBA *)(p32Src + i);
        if (bSmallEndian)
        {
            C_1555_SmallEndian *pC1555 = (C_1555_SmallEndian *)(p16Dst + j);
            pC1555->red = (pCRGBA->red & 0xF8) >> 3;
            pC1555->green = (pCRGBA->green & 0xF8) >> 3;
            pC1555->blue = (pCRGBA->blue & 0xF8) >> 3;
            pC1555->alpha = (pCRGBA->alpha > 0x80 ? nTransparent : nDisTransparent);
        }
        else 
        {
            C_1555_BigEndian *pC1555 = (C_1555_BigEndian *)(p16Dst + j);
            pC1555->red = (pCRGBA->red & 0xF8) >> 3;
            pC1555->green = (pCRGBA->green & 0xF8) >> 3;
            pC1555->blue = (pCRGBA->blue & 0xF8) >> 3;
            pC1555->alpha = (pCRGBA->alpha > 0x80 ? nTransparent : nDisTransparent);
        }
    }

    return 0;
}

int imagetools_rawconvert_HBIT_to_1555(const void *pSrc, unsigned int nSrcSize, 
    void *pDst, unsigned int nDstSize, const void *pPixel1555_0, const void *pPixel1555_1)
{
    const unsigned char bitmap[8] = {0x80,0x40,0x20,0x10,0x8,0x4,0x2,0x1};

    UBYTE *p8Src = (UBYTE *)pSrc;
    U16_1555 *p16Dst = (U16_1555 *)pDst;
    U16_1555 *p16Pixel1555_0 = (U16_1555*)pPixel1555_0;
    U16_1555 *p16Pixel1555_1 = (U16_1555*)pPixel1555_1;
    unsigned int n8SrcSize = nSrcSize;
    unsigned int n16DstSize = nDstSize / 2;

    unsigned int i = 0;     
    unsigned int j = 0;
    int k = 0;
    for (i = 0,j = 0; i < n8SrcSize && j < n16DstSize; ++i)
    {
        for (k = 0; k < 8 && j < n16DstSize; ++k, ++j)
        {
            if (p8Src[i] & bitmap[k])
            {
                p16Dst[j] = *(p16Pixel1555_1);
            }
            else 
            {
                p16Dst[j] = *(p16Pixel1555_0);
            }
        }
    }

    return 0;
}

int imagetools_rawconvert_HBIT_to_RGBA(const void *pSrc, unsigned int nSrcSize, 
    void *pDst, unsigned int nDstSize, const void *pPixelRGBA_0, const void *pPixelRGBA_1)
{
    const unsigned char bitmap[8] = {0x80,0x40,0x20,0x10,0x8,0x4,0x2,0x1};

    UBYTE *p8Src = (UBYTE *)pSrc;
    U32_RGBA *p32Dst = (U32_RGBA *)pDst;
    U32_RGBA *p32PixelRGBA_0 = (U32_RGBA*)pPixelRGBA_0;
    U32_RGBA *p32PixelRGBA_1 = (U32_RGBA*)pPixelRGBA_1;
    unsigned int n8SrcSize = nSrcSize;
    unsigned int n32DstSize = nDstSize / sizeof(U32_RGBA);

    unsigned int i = 0;     
    unsigned int j = 0;
    int k = 0;
    for (i = 0,j = 0; i < n8SrcSize && j < n32DstSize; ++i)
    {
        for (k = 0; k < 8 && j < n32DstSize; ++k, ++j)
        {
            if (p8Src[i] & bitmap[k])
            {
                p32Dst[j] = *(p32PixelRGBA_1);
            }
            else 
            {
                p32Dst[j] = *(p32PixelRGBA_0);
            }
        }
    }

    return 0;
}

int imagetools_scale_rgba(const void *pSrc, unsigned int nSrcW, unsigned int nSrcH, 
    void *pDst, unsigned int nDstW, unsigned int nDstH)
{
    unsigned char *pre_lcd = (unsigned char *)pSrc;
    unsigned int src_width = nSrcW;
    unsigned int src_height = nSrcH;
    unsigned int dst_width = nDstW;
    unsigned int dst_height = nDstH;

	int byte_size = sizeof(C_RGBA);
 
	// int hadd, wadd; //用于图像精确四舍五入的参数
	unsigned char *buffer = (unsigned char *)pDst; //存储转化后的图像数据
	memset(buffer, 0, dst_width * dst_height * byte_size); //对数组清零
 
	double fw = (double)dst_width / src_width; //计算图像宽度缩放比例
	double fh = (double)dst_height / src_height; //计算图像高度缩放比例
 
	// double fw_yu = fw - (int)fw; //求出缩放比例的小数部分
	// double fh_yu = fh - (int)fh;
 
	unsigned int x=0,y=0;
    unsigned int hnum = 0;
    unsigned int wnum = 0;
 
	for (hnum = 0; hnum < dst_height; ++hnum) //按照从左到右，从上到下的顺序进行转换
	{
		y = round(hnum / fh);   //计算当前临近坐标的y值
 
        if (y >= src_height)
        {
            y = src_height - 1;
        }
 
		for (wnum = 0; wnum < dst_width; ++wnum)
		{
			x = round(wnum / fw); //计算当前临近坐标的x值
 
            if (x >= src_width)
            {
                x = src_width - 1;
            }
 
			long pixel_point = hnum*dst_width*byte_size + wnum*byte_size; //计算转化后图像的数组偏移量
 
			//RGB数据依次进行数据变换
			buffer[pixel_point] = pre_lcd[y * src_width * byte_size + x * byte_size];
			buffer[pixel_point + 1] = pre_lcd[y * src_width * byte_size + x * byte_size + 1];
			buffer[pixel_point + 2] = pre_lcd[y * src_width * byte_size + x * byte_size + 2];
            buffer[pixel_point + 3] = pre_lcd[y * src_width * byte_size + x * byte_size + 3];
		}
	}
 
	return 0;
}

static int imagetools_rawreplace_RGBA(void *pPixels, unsigned int nPixelCount, 
    const void *pOldPixel, const void *pNewPixel)
{
    U32_RGBA *p32Src = (U32_RGBA *)pPixels;

    unsigned int i = 0;
    unsigned int nCount = 0;
    for (i = 0; i < nPixelCount; ++i)
    {
        if (memcmp (& p32Src[i], pOldPixel, sizeof(U32_RGBA)) == 0)
        {
            memcpy (& p32Src[i], pNewPixel, sizeof(U32_RGBA));
            ++nCount;
        }
    }

    return nCount;
}

static int imagetools_rawreplace_1555(void *pPixels, unsigned int nPixelCount, 
    const void *pOldPixel, const void *pNewPixel)
{
    U16_1555 *p16Src = (U16_1555 *)pPixels;

    unsigned int i = 0;
    unsigned int nCount = 0;
    for (i = 0; i < nPixelCount; ++i)
    {
        if (memcmp (& p16Src[i], pOldPixel, sizeof(U16_1555)) == 0)
        {
            memcpy (& p16Src[i], pNewPixel, sizeof(U16_1555));
            ++nCount;
        }
    }

    return nCount;
}

int imagetools_rawconvert (const void *pSrc, unsigned int nSrcSize, 
    void *pDst, unsigned int nDstSize, IMAGE_RAW_CONVERT_TYPE_E eConvType)
{
    int ret = -1;
    switch (eConvType)
    {
        case IMAGE_RAW_RGBA_1555:
        {
            ret = imagetools_rawconvert_RGBA_to_1555 (pSrc, nSrcSize, pDst, nDstSize, FALSE);
            break;
        }
        case IMAGE_RAW_RGBA_1555_0ALPHA:
        {
            ret = imagetools_rawconvert_RGBA_to_1555 (pSrc, nSrcSize, pDst, nDstSize, TRUE);
            break;
        }
        case IMAGE_RAW_RGBA_1555_1ALPHA:
        {
            ret = imagetools_rawconvert_RGBA_to_1555 (pSrc, nSrcSize, pDst, nDstSize, FALSE);
            break;
        }
        case IMAGE_RAW_HBIT_1555_1BLACK_1ALPHA:
        {
            U16_1555 pixel_1 = 0x8000;
            U16_1555 pixel_0 = 0xFFFF;
            ret = imagetools_rawconvert_HBIT_to_1555 (pSrc, nSrcSize, pDst, nDstSize, &pixel_0, &pixel_1);
            break;
        }
        case IMAGE_RAW_HBIT_1555_1BLACK_0ALPHA:
        {
            U16_1555 pixel_1 = 0x0000;
            U16_1555 pixel_0 = 0x7FFF;
            ret = imagetools_rawconvert_HBIT_to_1555 (pSrc, nSrcSize, pDst, nDstSize, &pixel_0, &pixel_1);
            break;
        }
        case IMAGE_RAW_HBIT_1555_1WHITE_1ALPHA:
        {
            U16_1555 pixel_1 = 0xFFFF;
            U16_1555 pixel_0 = 0x8000;
            ret = imagetools_rawconvert_HBIT_to_1555 (pSrc, nSrcSize, pDst, nDstSize, &pixel_0, &pixel_1);
            break;
        }
        case IMAGE_RAW_HBIT_1555_1WHITE_0ALPHA:
        {
            U16_1555 pixel_1 = 0x7FFF;
            U16_1555 pixel_0 = 0x0000;
            ret = imagetools_rawconvert_HBIT_to_1555 (pSrc, nSrcSize, pDst, nDstSize, &pixel_0, &pixel_1);
            break;
        }
        default:
        {
            _error ("unexpected type=%d\n", eConvType);
            break;
        }
    }
    return ret;
}


int imagetools_scale(const void *pSrc, unsigned int nSrcW, unsigned int nSrcH, 
    void *pDst, unsigned int nDstW, unsigned int nDstH, IMAGE_RAW_TYPE_E eImageRawType)
{
    int ret = -1;
    switch (eImageRawType)
    {
        case IMAGE_RAW_RGBA:
        {
            ret = imagetools_scale_rgba (pSrc, nSrcW, nSrcH, pDst, nDstW, nDstH);
            break;  
        }
        default:
        {
            _error ("unexpected type=%d\n", eImageRawType);
            break;
        }
    }
    return ret;
}

int imagetools_replaceall (void *pPixels, unsigned int nPixelCount, 
    const void *pOldPixel, const void *pNewPixel, IMAGE_RAW_TYPE_E eImageRawType)
{
    int ret = -1;
    switch (eImageRawType)
    {
        case IMAGE_RAW_1555:
        {
            ret = imagetools_rawreplace_1555 (pPixels, nPixelCount, pOldPixel, pNewPixel);
            break;   
        }
        case IMAGE_RAW_RGBA:
        {
            ret = imagetools_rawreplace_RGBA (pPixels, nPixelCount, pOldPixel, pNewPixel);
            break;  
        }
        default:
        {
            _error ("unexpected type=%d\n", eImageRawType);
            break;
        }
    }
    return ret;
}

static int imagetools_settwocolor_rgba (void *pPixels, unsigned int nPixelCount, 
    const void *pBackColor, const void *pBackToColor, const void* pFrontToColor)
{
    U32_RGBA *pRGBAPixel = (U32_RGBA *)pPixels;
    U32_RGBA *pRGBAPixel_Back = (U32_RGBA *)pBackColor;
    U32_RGBA *pRGBAPixel_BackTo = (U32_RGBA *)pBackToColor;
    U32_RGBA *pRGBAPixel_FrontTo = (U32_RGBA *)pFrontToColor;

    unsigned int i = 0;

    for (i = 0; i < nPixelCount; ++i)
    {
        if (pRGBAPixel[i] == *pRGBAPixel_Back)
        {
            pRGBAPixel[i] = *pRGBAPixel_BackTo;
        }
        else 
        {
            pRGBAPixel[i] = *pRGBAPixel_FrontTo;
        }
    }

    return 0;
}

int imagetools_settwocolor (void *pPixels, unsigned int nPixelCount, 
    const void *pBackColor, const void *pBackToColor, const void* pFrontToColor, IMAGE_RAW_TYPE_E eImageRawType)
{
    int ret = -1;
    switch (eImageRawType)
    {
        case IMAGE_RAW_RGBA:
        {
            ret = imagetools_settwocolor_rgba (pPixels, nPixelCount, pBackColor, pBackToColor, pFrontToColor);
            break;
        }
        default:
        {
            _error ("unexpected type=%d\n", eImageRawType);
            break;
        }
    }
    return ret;
}

/**
 * @param bLowAlpha: 为真，则 0 表示透明，1 表示不透明
 */
static U16_1555 imagetools_mixto_1555(U16_1555 pixel1555_Board, U16_1555 pixel1555_Paint, int bLowAlpha)
{
    int bSmallEndian = imagetools_smallendian();
    U16_1555 pixelMixed = 0;

    int n1555Transparent = (bLowAlpha ? 0 : 1);

    int bPaintTransparent = FALSE;

    if (bSmallEndian)
    {
        const C_1555_SmallEndian* p1555Paint = (const C_1555_SmallEndian*)& pixel1555_Paint;
        if (n1555Transparent == p1555Paint->alpha)
        {
            bPaintTransparent = TRUE;
        }
        else 
        {
            bPaintTransparent = FALSE;
        }
    }
    else 
    {
        const C_1555_BigEndian* p1555Paint = (const C_1555_BigEndian*)& pixel1555_Paint;
        if (n1555Transparent == p1555Paint->alpha)
        {
            bPaintTransparent = TRUE;
        }
        else 
        {
            bPaintTransparent = FALSE;
        }
    }

    if (bPaintTransparent)
    {
        pixelMixed = pixel1555_Board;
    }
    else 
    {
        pixelMixed = pixel1555_Paint;
    }

    return pixelMixed;
}

static int imagetools_drawimage_1555 (void *pDstPixels, unsigned int nDstWidth, unsigned int nDstHeight, 
    const void *pSrcPixels, unsigned int nSrcWidth, unsigned int nSrcHeight, 
    int nX, int nY, IMAGE_DRAW_MODE eDrawMode, int bLowAlpha)
{
    U16_1555 *p16DstPixels1555 = (U16_1555 *)pDstPixels;
    U16_1555 *p16SrcPixels1555 = (U16_1555 *)pSrcPixels;

    unsigned int xSrc = 0;
    unsigned int ySrc = 0;

    for (ySrc = 0; ySrc < nSrcHeight; ++ySrc)
    {
        for (xSrc = 0; xSrc < nSrcWidth; ++xSrc)
        {
            int xDst = (int)(xSrc + nX);
            int yDst = (int)(ySrc + nY);

            if (xDst < 0 || xDst >= (int)nDstWidth
                    || yDst < 0 || yDst >= (int)nDstHeight)
            {
                continue ;
            }

            int nDstIndex = xDst + yDst * nDstWidth;
            int nSrcIndex = xSrc + ySrc * nSrcWidth;

            if (nDstIndex >= (int)(nDstWidth * nDstHeight) ||
                nSrcIndex >= (int)(nSrcWidth * nSrcHeight))
            {
                _error ("inner error\n");
                continue;
            }

            switch (eDrawMode)
            {
                case IMAGE_DRAW_MIX:
                {
                    p16DstPixels1555[nDstIndex] = imagetools_mixto_1555(p16DstPixels1555[nDstIndex],
                                            p16SrcPixels1555[nSrcIndex], bLowAlpha);
                    break;
                }
                case IMAGE_DRAW_COVER:
                default:
                {
                    p16DstPixels1555[nDstIndex] = p16SrcPixels1555[nSrcIndex];
                    break;
                }
            }
        }
    }

    return 0;
}

static int imagetools_brushrect_1555 (void *pDstPixels, unsigned int nDstWidth, unsigned int nDstHeight,
    const void *pSrcPixels, unsigned int nSrcWidth, unsigned int nSrcHeight, 
    int nX, int nY, IMAGE_DRAW_MODE eDrawMode, int bLowAlpha)
{
    U16_1555 *p16DstPixels1555 = (U16_1555 *)pDstPixels;
    U16_1555 u16SrcPixel1555 = *(U16_1555 *)pSrcPixels;

    unsigned int xSrc = 0;
    unsigned int ySrc = 0;

    for (ySrc = 0; ySrc < nSrcHeight; ++ySrc)
    {
        for (xSrc = 0; xSrc < nSrcWidth; ++xSrc)
        {
            int xDst = (int)(xSrc + nX);
            int yDst = (int)(ySrc + nY);

            if (xDst < 0 || xDst >= (int)nDstWidth
                    || yDst < 0 || yDst >= (int)nDstHeight)
            {
                continue ;
            }

            int nDstIndex = xDst + yDst * nDstWidth;
            int nSrcIndex = xSrc + ySrc * nSrcWidth;

            if (nDstIndex >= (int)(nDstWidth * nDstHeight) ||
                nSrcIndex >= (int)(nSrcWidth * nSrcHeight))
            {
                _error ("inner error\n");
                continue;
            }

            switch (eDrawMode)
            {
                case IMAGE_DRAW_MIX:
                {
                    p16DstPixels1555[nDstIndex] = imagetools_mixto_1555(p16DstPixels1555[nDstIndex],
                                            u16SrcPixel1555, bLowAlpha);
                    break;
                }
                case IMAGE_DRAW_COVER:
                default:
                {
                    p16DstPixels1555[nDstIndex] = u16SrcPixel1555;
                    break;
                }
            }
        }
    }

    return 0;
}

int imagetools_drawimage (void *pDstPixels, unsigned int nDstWidth, unsigned int nDstHeight, 
    const void *pSrcPixels, unsigned int nSrcWidth, unsigned int nSrcHeight, 
    int nX, int nY, IMAGE_DRAW_MODE eDrawMode, IMAGE_RAW_TYPE_E eImageRawType)
{
    int ret = -1;
    switch (eImageRawType)
    {
        case IMAGE_RAW_1555:
        {
            ret = imagetools_drawimage_1555 (pDstPixels, nDstWidth, nDstHeight, 
                            pSrcPixels, nSrcWidth, nSrcHeight, nX, nY, eDrawMode, FALSE);
            break;
        }
        case IMAGE_RAW_1555_1ALPHA:
        {
            ret = imagetools_drawimage_1555 (pDstPixels, nDstWidth, nDstHeight, 
                            pSrcPixels, nSrcWidth, nSrcHeight, nX, nY, eDrawMode, FALSE);
            break;
        }
        case IMAGE_RAW_1555_0ALPHA:
        {
            ret = imagetools_drawimage_1555 (pDstPixels, nDstWidth, nDstHeight, 
                            pSrcPixels, nSrcWidth, nSrcHeight, nX, nY, eDrawMode, TRUE);
            break;
        }
        default:
        {
            _error ("unexpected type=%d\n", eImageRawType);
            break;
        }
    }
    return ret;
}

int imagetools_brushrect (void *pDstPixels, unsigned int nDstWidth, unsigned int nDstHeight,
    const void *pSrcPixels, unsigned int nSrcWidth, unsigned int nSrcHeight, 
    int nX, int nY, IMAGE_DRAW_MODE eDrawMode, IMAGE_RAW_TYPE_E eImageRawType)
{
    int ret = -1;
    switch (eImageRawType)
    {
        case IMAGE_RAW_1555:
        {
            ret = imagetools_brushrect_1555 (pDstPixels, nDstWidth, nDstHeight, 
                            pSrcPixels, nSrcWidth, nSrcHeight, nX, nY, eDrawMode, FALSE);
            break;
        }
        default:
        {
            _error ("unexpected type=%d\n", eImageRawType);
            break;
        }
    }
    return ret;
}