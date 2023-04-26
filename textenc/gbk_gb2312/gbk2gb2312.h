#ifndef GBK2GB2312_H__
#define GBK2GB2312_H__

/**
 * @brief 编码转换：
 * 将 GBK 转换为 GB2312 ，对于无法转换的字符，则用 @cReplaceToken 替换
 * @param pSrc GBK 编码字串
 * @param nSrcSize 源字串的大小(byte)
 * @param pDst 预先分配的内存
 * @param nDstSize (byte)
 * @return 返回0
 */
int enc_conv_gbk2gb2312 (void *pSrc, int nSrcSize, void *pDst, int nDstSize, char cReplaceToken);

#endif // GBK2GB2312_H__