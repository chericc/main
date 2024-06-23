#include "image_jpeg.h"

#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>

#include "def_inner.h"

#ifdef SUPPORT_JPG
#include "jpeglib.h"
#endif  // SUPPORT_JPG

IMAGE_NAME_IMPORT;

#ifdef SUPPORT_JPG
struct my_error_mgr {
    struct jpeg_error_mgr pub; /* "public" fields */

    jmp_buf setjmp_buffer; /* for return to caller */
};

typedef struct my_error_mgr* my_error_ptr;

METHODDEF(void)
my_error_exit(j_common_ptr cinfo) {
    /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
    my_error_ptr myerr = (my_error_ptr)cinfo->err;

    /* Always display the message. */
    /* We could postpone this until after returning, if we chose. */
    (*cinfo->err->output_message)(cinfo);

    /* Return control to the setjmp point */
    longjmp(myerr->setjmp_buffer, 1);
}
#endif  // SUPPORT_JPG

bool ImageJpeg::LoadFile_override() {
    bool ret = false;

#ifdef SUPPORT_JPG
    struct jpeg_decompress_struct cinfo = {};
    struct my_error_mgr cmyerr = {};
    FILE* fp = NULL;
#endif  // SUPPORT_JPG

    do {
#ifdef SUPPORT_JPG
        /* open file */
        if ((fp = fopen(m_strFileName.c_str(), "rb")) == NULL) {
            _error("open file %s failed\n", m_strFileName.c_str());
            break;
        }

        /**
         * error handling
         * all possible error should happen in this function
         **/
        cinfo.err = jpeg_std_error(&cmyerr.pub);
        cmyerr.pub.error_exit = my_error_exit;

        if (setjmp(cmyerr.setjmp_buffer)) {
            /* if error happens in libjpeg, jump to this line */
            jpeg_destroy_decompress(&cinfo);

            fclose(fp);
            fp = NULL;

            m_vectPixelArray.resize(0);

            break;
        }

        /* create decompressor */
        jpeg_create_decompress(&cinfo);

        /* specify data source */
        jpeg_stdio_src(&cinfo, fp);

        /* read header */
        jpeg_read_header(&cinfo, TRUE);

        /* start decompressor */
        jpeg_start_decompress(&cinfo);

        /* alloc buffer */
        JSAMPARRAY buffer = NULL;
        int row_stride = cinfo.output_width * cinfo.output_components;
        buffer = (cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE,
                                           row_stride, 1);

        if (3 != cinfo.output_components) {
            _error("component != 3, [%d]\n", cinfo.output_components);
            longjmp(cmyerr.setjmp_buffer, 1);
        }

        m_vectPixelArray.resize(cinfo.output_width * cinfo.output_height);

        /* read data */
        int y = 0;
        while (cinfo.output_scanline < cinfo.output_height) {
            jpeg_read_scanlines(&cinfo, buffer, 1);

            /* copy pixels of a row */
            for (JDIMENSION x = 0; x < cinfo.output_width; ++x) {
                JDIMENSION nPixelPos = x + y * cinfo.output_width;

                JDIMENSION nByteRed = x * cinfo.output_components + 0;
                JDIMENSION nByteGreen = x * cinfo.output_components + 1;
                JDIMENSION nByteBlue = x * cinfo.output_components + 2;

                m_vectPixelArray[nPixelPos].R = buffer[0][nByteRed];
                m_vectPixelArray[nPixelPos].G = buffer[0][nByteGreen];
                m_vectPixelArray[nPixelPos].B = buffer[0][nByteBlue];
                m_vectPixelArray[nPixelPos].A = 0;
            }

            ++y;
        }

        m_nWidth = cinfo.output_width;
        m_nHeight = cinfo.output_height;

        /* finish decompressor */
        jpeg_finish_decompress(&cinfo);

        /* release object */
        jpeg_destroy_decompress(&cinfo);

        fclose(fp);
        fp = NULL;

        ret = true;
#endif  // SUPPORT_JPG
    } while (0);

    return ret;
}
