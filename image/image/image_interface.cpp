#include "image_interface.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h> 

#include <string.h>

#include "def_inner.h"

IMAGE_NAME_IMPORT;

ImageInterface::ImageInterface()
    : m_nWidth(0),m_nHeight(0),m_bFileLoaded(false)
{
    // blank
}

ImageInterface::~ImageInterface()
{
    // blank
}

bool ImageInterface::LoadFile (const char *szFileName)
{
    m_bFileLoaded = false;
    m_nWidth = 0;
    m_nHeight = 0;
    m_vectPixelArray.resize (0);
    m_strFileName = std::string(szFileName);

    bool ret = CheckOrLoadFile();
    return ret;
}

int ImageInterface::Width()
{
    CheckOrLoadFile ();

    return m_nWidth;
}

int ImageInterface::Height()
{
    CheckOrLoadFile ();

    return m_nHeight;
}

bool ImageInterface::GetPixel(int x, int y, Pixel& refPixel)
{
    if (! CheckOrLoadFile())
    {
        return false;
    }

    int nPixelPos = x + y * m_nWidth;
    if (nPixelPos >= static_cast<int>(m_vectPixelArray.size()))
    {
        _error ("[%d,%d] over [%d,%d]\n", x,y,m_nWidth,m_nHeight);
        return false;
    }

    refPixel = m_vectPixelArray[nPixelPos];
    return true;
}

int ImageInterface::SetToTwoColor (const Pixel & refLightPixel, const Pixel &refDarkPixel, const Pixel &refMidPixel)
{
    int nPixelCount = 0;

    if (! CheckOrLoadFile())
    {
        return -1;
    }

    if (sizeof(refLightPixel) != sizeof(m_vectPixelArray[0]))
    {
        _error ("inner error, size not match\n");
        return -1;
    }

    int nMidLight = refMidPixel.R + refMidPixel.G + refMidPixel.B;
    for (std::size_t i = 0; i < m_vectPixelArray.size(); ++i)
    {
        int nCurLight = m_vectPixelArray[i].R + m_vectPixelArray[i].G + m_vectPixelArray[i].B;
        if (nCurLight > nMidLight)
        {
            m_vectPixelArray[i] = refLightPixel;
        }
        else 
        {
            m_vectPixelArray[i] = refDarkPixel;
        }
        ++nPixelCount;
    }

    return nPixelCount;
}

bool ImageInterface::GuessImageType(const char *szFilename, IMAGE_FILE_TYPE_E *eType)
{
    bool bGuessOk = false;

    const uint8_t jpg_file_head[] = {0xff, 0xd8, 0xff};
    const uint8_t png_file_head[] = {0x89, 0x50, 0x4e, 0x47, 0x0d, 0xa0, 0x1a, 0x0a};
    const uint8_t bmp_file_head[] = {0x42, 0x4d};

    int fd = -1;
    int nBytesRead = 0;
    char byteFileBuf[32] = {}; // ENSURE: bigger than header

    do 
    {
        fd = open (szFilename, O_RDONLY);
        if (fd < 0)
        {
            _error ("open file failed [%s]\n", szFilename);
            break;
        }

        nBytesRead = read (fd, byteFileBuf, sizeof(byteFileBuf));
        if (nBytesRead < 0)
        {
            _error ("read failed\n");
            break;
        }

        if (memcmp(jpg_file_head, byteFileBuf, sizeof(jpg_file_head)) == 0)
        {
            *eType = IMAGE_JPEG;
            bGuessOk = true;
        }
        else if (memcmp (png_file_head, byteFileBuf, sizeof(png_file_head)) == 0)
        {
            *eType = IMAGE_PNG;
            bGuessOk = true;
        }
        else if (memcmp (bmp_file_head, byteFileBuf, sizeof(bmp_file_head)) == 0)
        {
            *eType = IMAGE_BMP;
            bGuessOk = true;
        }
        else 
        {
            _error ("file format guess failed\n");
        }
    }
    while (0);

    if (fd >= 0)
    {
        close (fd);
        fd = -1;
    }

    return bGuessOk ? true : false;
}

bool ImageInterface::CheckOrLoadFile ()
{
    if (m_bFileLoaded)
    {
        return true;
    }

    if (LoadFile_override())
    {
        m_bFileLoaded = true;
        return true;
    }
    else 
    {
        _error ("open failed\n");
    }

    return false;
}