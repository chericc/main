#include <stdio.h>
#include <string.h>

#include "iconv.h"

#ifndef DEBUG
#define DEBUG
#endif 

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

void my_iconv_unicode_mb_to_uc_fallback
             (const char* inbuf, size_t inbufsize,
              void (*write_replacement) (const unsigned int *buf, size_t buflen,
                                         void* callback_arg),
              void* callback_arg,
              void* data)
{
    _debug ("replace\n");

    for (size_t i = 0; i < inbufsize; ++i)
    {
        _debug ("inbuf[%u]=%x\n", i, (unsigned char)inbuf[i]);
    }

    unsigned char buf[] = {0x6A, 0x0, 0x0, 0x0};
    const unsigned int *cbuf = (const unsigned int *)buf;
    write_replacement(cbuf, sizeof(buf), callback_arg);
}

void my_iconv_unicode_uc_to_mb_fallback
             (unsigned int code,
              void (*write_replacement) (const char *buf, size_t buflen,
                                         void* callback_arg),
              void* callback_arg,
              void* data)
{
    _debug ("replace\n");
    
    _debug ("inbuf = %x\n", code);
    unsigned char buf[] = {0x6A};
    const char *pbuf = (const char *)buf;
    write_replacement (pbuf, sizeof(buf), callback_arg);
}

void my_iconv_wchar_mb_to_wc_fallback
             (const char* inbuf, size_t inbufsize,
              void (*write_replacement) (const wchar_t *buf, size_t buflen,
                                         void* callback_arg),
              void* callback_arg,
              void* data)
{
    _debug ("replace\n");
}

void my_iconv_wchar_wc_to_mb_fallback
             (wchar_t code,
              void (*write_replacement) (const char *buf, size_t buflen,
                                         void* callback_arg),
              void* callback_arg,
              void* data)
{
    _debug ("replace\n");
}

int main()
{
    // unsigned char byteBuf[] = {
    //     0x81,0x40, // GBK 起始编码，不在 GB2312 中
    //     0xBD,0xF1, // “今”
    // };
    unsigned char byteBuf[16] = {
        0x81,0x40, // GBK 起始编码，不在 GB2312 中
        0xBD,0xF1, // “今”
    };
    char *pByteBufIn = (char*)byteBuf;
    size_t nInSize = strlen(pByteBufIn);

    unsigned char byteOutBuf[256];
    char *pOutBuf = (char*)byteOutBuf;
    size_t nOutSize = sizeof(byteOutBuf);

    _debug ("\n");
    iconv_t hIconv = iconv_open ("GB2312", "GBK");
    _debug ("\n");
    if (hIconv == (iconv_t)-1)
    {
        _error ("open failed\n");
        return -1;
    }
    _debug ("pByteBufIn=%p, nInSize=%d, pOutBuf=%p, nOutSize=%d\n", 
        pByteBufIn, nInSize, pOutBuf, nOutSize);
    
    // int nSet = 1;
    // iconvctl (hIconv, ICONV_SET_DISCARD_ILSEQ, &nSet);

    struct iconv_fallbacks fallb = {};
    // fallb.mb_to_wc_fallback = my_iconv_wchar_mb_to_wc_fallback;
    // fallb.wc_to_mb_fallback = my_iconv_wchar_wc_to_mb_fallback;
    // fallb.mb_to_uc_fallback = my_iconv_unicode_mb_to_uc_fallback;
    fallb.uc_to_mb_fallback = my_iconv_unicode_uc_to_mb_fallback;
    iconvctl (hIconv, ICONV_SET_FALLBACKS, &fallb);

    size_t ret = iconv (hIconv, &pByteBufIn, &nInSize, &pOutBuf, &nOutSize);
    _debug ("ret=%u\n", ret);

    iconv_close (hIconv);
    _debug ("pByteBufIn=%p, nInSize=%d, pOutBuf=%p, nOutSize=%d\n", 
        pByteBufIn, nInSize, pOutBuf, nOutSize);

    for (size_t i = 0; i < sizeof(byteOutBuf) - nOutSize; ++i)
    {
        _debug ("byteOutBuf[%u]=%x\n", i, byteOutBuf[i]);
    }

    return 0;
}