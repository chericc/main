#ifndef GBK2GB2312_H__
#define GBK2GB2312_H__

/**
 * @brief ����ת����
 * �� GBK ת��Ϊ GB2312
 * �������޷�ת�����ַ�������
 * @cReplaceToken �滻
 * @param pSrc GBK �����ִ�
 * @param nSrcSize Դ�ִ��Ĵ�С(byte)
 * @param pDst Ԥ�ȷ�����ڴ�
 * @param nDstSize (byte)
 * @return ����0
 */
int enc_conv_gbk2gb2312(void* pSrc, int nSrcSize, void* pDst, int nDstSize,
                        char cReplaceToken);

#endif  // GBK2GB2312_H__