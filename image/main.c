
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if.h>

#include <string.h>
#include <stdlib.h>

#include "image2rgba.h"
#include "image_tools.h"

#define DEBUG

#ifdef DEBUG
#define _debug(x...) do {printf("[debug][%s %d %s]", \
	__FILE__,__LINE__,__FUNCTION__);printf(x);} while (0)
#define _info(x...) do {printf("[info][%s %d %s]", \
	__FILE__,__LINE__,__FUNCTION__);printf(x);} while (0)
#define _error(x...) do {printf("[error][%s %d %s]", \
	__FILE__,__LINE__,__FUNCTION__);printf(x);} while (0)
#else 
#define _debug(x...) do {;} while (0)
#define _info(x...) do {printf("[info][%s %d %s]", \
	__FILE__,__LINE__,__FUNCTION__);printf(x);} while (0)
#define _error(x...) do {printf("[error][%s %d %s]", \
	__FILE__,__LINE__,__FUNCTION__);printf(x);} while (0)
#endif 

static void bitprint(const void *pData, int nSize)
{
    unsigned char bitmap[8] = {0x1,0x2,0x4,0x8,0x10,0x20,0x40,0x80};

    for (int i = 0; i < nSize; ++i)
    {
        if ((i) % 4 == 0)
            printf ("0x%04x: ", i);
        for (int j = sizeof(bitmap) - 1; j >= 0; --j)
        {
            if (((char*)pData)[i] & bitmap[j])
            {
                printf ("1");
            }
            else 
            {
                printf ("0");
            }
        }
        printf (" ");
        
        if ((i+1) % 4 == 0)
            printf ("\n");
    }

    printf ("\n");
}

static void RGBAprint(const Image2RGBA_Pixel *pPixel, int nPixelCount, int nWidth)
{
    for (int i = 0; i < nPixelCount; ++i)
    {
        printf ("[%02x %02x %02x %02x] ", 
            pPixel[i].R, 
            pPixel[i].G,
            pPixel[i].B,
            pPixel[i].A);
        if (nWidth <= 0)
        {
            printf ("\n");
        }
        else if ((i + 1) % nWidth == 0)
        {
            printf ("\n");
        }
    }
    printf ("\n");
}

static void C1555print(const void *pData, int nSize, int nWidth)
{
    int nPixelCount = nSize / 2;
    const unsigned short *phData = (const unsigned short *)pData;
    for (int i = 0; i < nPixelCount; ++i)
    {
        printf ("[%04x] ", phData[i]);
        if (nWidth <= 0)
        {
            printf ("\n");
        }
        else if ((i + 1) % nWidth == 0)
        {
            printf ("\n");
        }
    }
    printf ("\n");
}

static void fun_with_drawA (Image2RGBA_Pixel *pixels, int nWidth, int nHeight)
{
    int n1555Size = nWidth * nHeight * 2;
    void *p1555Buf = malloc (n1555Size);

    imagetools_rawconvert (pixels, nWidth * nHeight * sizeof(Image2RGBA_Pixel), p1555Buf, n1555Size, IMAGE_RAW_RGBA_1555);
    C1555print (p1555Buf, n1555Size, nWidth);

    unsigned short sPixel1555[36] = {};
    for (int i = 0; i < 36; ++i)
    {
        sPixel1555[i] = 0x1111;
    }
    C1555print (sPixel1555, sizeof(sPixel1555), 6);

    imagetools_drawimage (& sPixel1555, 6, 6, p1555Buf, nWidth, nHeight, 1, 1, IMAGE_DRAW_COVER, IMAGE_RAW_1555);
    C1555print (sPixel1555, sizeof(sPixel1555), 6);
}

static void fun_with_drawB ()
{
    unsigned short sPixel1555_Src[4] = {
        0x8111,0x7111,0x8111,0x7111
    };
    C1555print (sPixel1555_Src, sizeof(sPixel1555_Src), 2);

    unsigned short sPixel1555_Board[36] = {};
    for (int i = 0; i < 36; ++i)
    {
        sPixel1555_Board[i] = 0x1111;
    }
    
    C1555print (sPixel1555_Board, sizeof(sPixel1555_Board), 6);

    imagetools_drawimage (& sPixel1555_Board, 6, 6, &sPixel1555_Src, 2, 2, 1, 1, IMAGE_DRAW_MIX, IMAGE_RAW_1555);
    C1555print (sPixel1555_Board, sizeof(sPixel1555_Board), 6);
}

static void fun_with_drawC()
{
    unsigned short sPixel1555_Brush = 0x2222;
    unsigned short sPixel1555_Board[36] = {};
    for (int i = 0; i < 36; ++i)
    {
        sPixel1555_Board[i] = 0x1111;
    }
    
    C1555print (sPixel1555_Board, sizeof(sPixel1555_Board), 6);
    imagetools_brushrect (& sPixel1555_Board, 6, 6, & sPixel1555_Brush, 2, 2, 0, 0, IMAGE_DRAW_COVER, IMAGE_RAW_1555);
    C1555print (sPixel1555_Board, sizeof(sPixel1555_Board), 6);
}

static void fun_with_imageA(int argc, char **argv)
{
    if (argc != 2)
    {
        _error ("usage: %s [filename]\n", argv[0]);
        return ;   
    }

    const char *szFile = argv[1];

    HANDLE_IMAGE2RGBA handle = NULL;

    _debug ("open file %s\n", szFile);

    handle = image2rgba_open (szFile);
    if (handle == NULL)
    {
        _error ("open failed\n");
        return ;
    }

    int nW = 0;
    int nH = 0;
    image2rgba_getsize (handle, &nW, &nH);

    _debug ("[%d,%d]\n", nW, nH);

    int nPixelSize = nW * nH;
    int nByteSize = nPixelSize * sizeof(Image2RGBA_Pixel);
    Image2RGBA_Pixel *pPixel = (Image2RGBA_Pixel*)malloc (nByteSize);
    for (int y = 0; y < nH; ++y)
    {
        for (int x = 0; x < nW; ++x)
        {
            image2rgba_getpixel (handle, x, y, & pPixel[x + y * nH]);
        }
    }
    _debug ("original:\n");
    RGBAprint (pPixel, nPixelSize, nW);

    image2rgba_close (handle);

    free (pPixel);
    pPixel = NULL;
}

static void fun_with_convertA()
{
    Image2RGBA_Pixel arrayPixel[4] = {{0,0,0,0},{0,0,0,255},{0,0,0,0},{0,0,0,255}};

    unsigned short array1555[4] = {};

    imagetools_rawconvert (& arrayPixel, sizeof(arrayPixel),
        & array1555, sizeof(array1555), IMAGE_RAW_RGBA_1555_0ALPHA);

    RGBAprint ((const Image2RGBA_Pixel *)(& arrayPixel), 4, 2);
    C1555print (& array1555, sizeof(array1555), 2);

    return ;
}

int main(int argc, char *argv[])
{


    // fun_with_drawB ();
    // fun_with_drawC ();
    fun_with_convertA ();

    return 0;
}