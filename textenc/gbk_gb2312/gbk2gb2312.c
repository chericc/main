#include "gbk2gb2312.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * BIG ENDIAN / SMALL ENDIAN
 * 只考虑小端情况 
 **/

#define SMALL_ENDIAN

#ifdef SMALL_ENDIAN

typedef struct 
{
    unsigned char uc;
} BYTE;

typedef struct 
{
    unsigned char uc1;
    unsigned char uc2;
} DBYTE;

#endif 

/**
 * @brief 统计变字节字串的字符数
 * 适用于双字节编码和 ascii 编码组合的情况
 */
static int enc_conv_count_mb_1_2 (const void *pSrc, int nSrcSize)
{
    int i = 0;
    int nCount = 0;

    for (i = 0; i < nSrcSize; )
    {
        if ( 0 == ((BYTE*)pSrc)[i].uc )
        {
            break;
        }
        if ( ((BYTE*)pSrc)[i].uc <= 0x7f )
        {
            ++i;
            ++nCount;
        }
        else 
        {
            i += 2;
            ++nCount;
        }
    }

    return nCount;
}

/**
 * @brief GBK 变字节转宽字符
 * @note 输出大小需求最大为源大小的 2 倍
 */
static int enc_conv_gbk_mb2wc(const void *pSrc, int nSrcSize, void *pDst, int nDstSize)
{
    int i = 0;
    int j = 0;

    for (i = 0, j = 0; i < nSrcSize && j < nDstSize / 2; )
    {
        if ( 0 == ((BYTE*)pSrc)[i].uc )
        {
            ((DBYTE*)pDst)[j].uc1 = 0x0;
            ((DBYTE*)pDst)[j].uc2 = 0x0;
            ++i;
            ++j;
            break;
        }
        else if ( ((BYTE*)pSrc)[i].uc <= 0x7f )
        {
            ((DBYTE*)pDst)[j].uc1 = ((BYTE*)pSrc)[i].uc;
            ((DBYTE*)pDst)[j].uc2 = 0x00;
            ++i;
            ++j;
        }
        else 
        {
            if (i + 1 < nSrcSize)
            {
                ((DBYTE*)pDst)[j].uc1 = ((BYTE*)pSrc)[i].uc;
                ((DBYTE*)pDst)[j].uc2 = ((BYTE*)pSrc)[i + 1].uc;
                i += 2;
                ++j;
            }
            else 
            {
                break;
            }
        }
    }

    return 0;
}

/**
 * @brief GBK 转 GB2312 宽字符
 * @note 输出大小需求和输入大小相同
 */
static int enc_conv_gbk2gb2312_wc(const void *pSrc, int nSrcSize, void *pDst, int nDstSize, DBYTE dbyteReplaceToken)
{
    int i = 0;

    for (i = 0; i < nSrcSize / 2 && i < nDstSize / 2; ++i)
    {
        printf ("replace [%d]\n", i);

        if ( 0 == ((DBYTE*)pSrc)[i].uc1 && 0 == ((DBYTE*)pSrc)[i].uc2 )
        {
            ((DBYTE*)pDst)[i].uc1 = 0x0;
            ((DBYTE*)pDst)[i].uc2 = 0x0;
            break;
        }
        else if ( ((DBYTE*)pSrc)[i].uc1 <= 0x7f )
        {
            // ((DBYTE*)pDst)[i] = ((DBYTE*)pSrc)[i];
            ((DBYTE*)pDst)[i].uc1 = ((DBYTE*)pSrc)[i].uc1;
            ((DBYTE*)pDst)[i].uc2 = ((DBYTE*)pSrc)[i].uc2;
        }
        else if ( ((DBYTE*)pSrc)[i].uc1 >= 0xa1 && ((DBYTE*)pSrc)[i].uc1 <= 0xfe )
        {
            ((DBYTE*)pDst)[i] = ((DBYTE*)pSrc)[i];
        }
        else 
        {
            ((DBYTE*)pDst)[i] = dbyteReplaceToken;
        }
    }

    return 0;
}

/**
 * @brief GB2312 宽字符转 GB2312 变字长
 * @note 输出大小需求不大于输入大小
 */
int enc_conv_gb2312_wc2mb (const void *pSrc, int nSrcSize, void *pDst, int nDstSize)
{
    int i = 0;
    int j = 0;

    for (i = 0, j = 0; i < nSrcSize / 2 && j < nDstSize; )
    {
        if ( 0 == ((BYTE*)pSrc)[i].uc && 0 == ((BYTE*)pSrc) )
        {
            ((DBYTE*)pDst)[j].uc1 = 0x0;
            ((DBYTE*)pDst)[j].uc2 = 0x0;
            ++i;
            ++j;
            break;
        }
        else if ( ((BYTE*)pSrc)[i].uc <= 0x7f )
        {
            ((DBYTE*)pDst)[j].uc1 = ((BYTE*)pSrc)[i].uc;
            ((DBYTE*)pDst)[j].uc2 = 0x00;
            ++i;
            ++j;
        }
        else 
        {
            if (i + 1 < nSrcSize)
            {
                ((DBYTE*)pDst)[j].uc1 = ((BYTE*)pSrc)[i].uc;
                ((DBYTE*)pDst)[j].uc2 = ((BYTE*)pSrc)[i + 1].uc;
                i += 2;
                ++j;
            }
            else 
            {
                break;
            }
        }
    }

    return 0;
}

int enc_conv_gbk2gb2312 (void *pSrc, int nSrcSize, void *pDst, int nDstSize, char cReplaceToken)
{
    int i = 0;
    int j = 0;

    unsigned char *pByteSrc = (unsigned char *)pSrc;
    unsigned char *pByteDst = (unsigned char *)pDst;

    for (i = 0, j = 0; i < nSrcSize && j < nDstSize; )
    {
        if ( 0x0 == pByteSrc[i] )
        { // end
            pByteDst[j] = 0x0;
            ++i;
            ++j;
            break;
        }
        else if ( pByteSrc[i] < 0x7f )
        { // ascii
            pByteDst[j] = pByteSrc[i];
            ++i;
            ++j;
        }
        else if ( pByteSrc[i] >= 0x81 && pByteSrc[i] <= 0xfe )
        { // in gbk
            if ( pByteSrc[i] >= 0xa1 && pByteSrc[i] <= 0xfe )
            { // in gb2312
                if ( i + 1 < nSrcSize && j + 1 < nDstSize )
                {
                    pByteDst[i] = pByteSrc[i];
                    pByteDst[i + 1] = pByteSrc[i + 1];

                    ++i;
                    ++i;
                    ++j;
                    ++j;
                }
                else 
                {
                    break;
                }
            }
            else 
            { // not in gb2312
                if ( i + 1 < nSrcSize && j + 1 < nDstSize )
                {
                    pByteDst[j] = cReplaceToken;
                    ++i;
                    ++i;
                    ++j;
                }
                else 
                {
                    break;
                }
            }
        }
        else 
        { // not in gbk

        }
    }

    return 0;
}
