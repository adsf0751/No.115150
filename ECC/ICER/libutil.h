/*
 * File:   libutil.h
 * Author: PeyJiun
 *
 * Created on 2010.12.6 8:57
 */

#ifndef _LIBUTIL_H
#define _LIBUTIL_H

#ifdef    __cplusplus
extern "C" {
#endif
//#include "Common.h"

// #define PACKED  __attribute__ ((packed))
// typedef struct {
//     BYTE    baYear[2];
//     BYTE    baMonth[2];
//     BYTE    baDay[2];
//     BYTE    baHour[2];
//     BYTE    baMin[2];
//     BYTE    baSec[2];
//     BYTE    bDateOfWeek;
// } PACKED tDTG_t;

/*
 * Ethernet error code
 */
#define BASE_ERROR                                  0x1000
#define d_ETHERNET_ERR_INVALID_PARAM               BASE_ERROR+1
#define d_ETHERNET_ERR_TRANSMISSION_TIMEOUT        BASE_ERROR+2

//ETHERNET
USHORT ETHERNET_Open(void);
USHORT ETHERNET_Close(void);
USHORT ETHERNET_Connect(BYTE *pbaDestIP, BYTE bIPLen, BYTE *pbaPort, BYTE bPortLen);
USHORT ETHERNET_Disconnect(void);
USHORT ETHERNET_SendData(BYTE* baData, USHORT usLen, USHORT usTimeout);
USHORT ETHERNET_RecvData(BYTE* baData, USHORT *pusLen, USHORT usTimeout);

//Debug
void   DebugEnable(BYTE byDbg);     //byDgb, TURE: Enable Debug, FALSE: Disable Debug
void   DebugEntry(const char* sourceFileName, int iLine, const char* str);
void   DebugInfo(const char* sourceFileName, int iLineNo, const char* pFormat, ...);
USHORT DebugExit(const char* sourceFileName, int iLine, const char* str, USHORT uRet);
void   DebugRetCode(const char* sourceFileName, int iLine, const char* str, USHORT uRet);
ULONG  DebugUlongExit(const char* sourceFileName, int iLine, const char* str, ULONG uRet);
void   DebugUlongRetCode(const char* pszFileName, int iLine, const char* str, ULONG uRet);
#define DEBUG_MSG(fmt,...)          DebugInfo(__FILE__,__LINE__, fmt, ## __VA_ARGS__ );
#define DEBUG_ENTRY(str)            DebugEntry(__FILE__,__LINE__, str);
#define DEBUG_EXIT(str, ret)        DebugExit(__FILE__,__LINE__, str, ret);
#define DEBUG_RET_CODE(str, ret)    DebugRetCode(__FILE__,__LINE__, str, ret);
#define DEBUG_ULONG_EXIT(str, ret)        DebugUlongExit(__FILE__,__LINE__, str, ret);
#define DEBUG_ULONG_RET_CODE(str, ret)    DebugUlongRetCode(__FILE__,__LINE__, str, ret);

//StrFun
unsigned int UTIL_FindStrEndPt(unsigned char *str);
void UTIL_StrCat(unsigned char *dest, unsigned char *sour);
void UTIL_StrNCat(unsigned char *dest, unsigned char *sour, unsigned int n);
void UTIL_StrCpy(unsigned char *dest, unsigned char *sour);
unsigned int UTIL_StrLen(unsigned char *str);
void UTIL_MemCpy(unsigned char *dest, unsigned char *sour, unsigned int nLen);
void UTIL_MemSet(unsigned char *addr, unsigned char ch, unsigned int nLen);
unsigned char UTIL_Hex2Ascii(unsigned char chHex);
void UTIL_StrAppendByteHex(unsigned char *str, unsigned char ch);
void UTIL_StrAppendShortHex(unsigned char *str, unsigned short usData);
void UTIL_StrAppendLongHex(unsigned char *str, unsigned long lData);
void UTIL_StrAppend2dDec(unsigned char *str, unsigned char ch);
void UTIL_StrAppendLongDec(unsigned char *str, unsigned long lData);
unsigned char UTIL_Long2Bcd(unsigned long lData, unsigned char *pszBuf, unsigned char *pszyte_len);
unsigned char UTIL_Toupper(unsigned char ch);
unsigned char UTIL_PackByte(unsigned char h, unsigned char l);
unsigned int UTIL_Hex2Str(unsigned char *hex, unsigned char *str, unsigned int nLen);
unsigned int UTIL_Str2Hex(unsigned char *str, unsigned char *hex, unsigned int nLen);
unsigned long UTIL_Str2Long(unsigned char *str, unsigned int nLen);
unsigned long UTIL_Array2Long(unsigned char *para);
void UTIL_Long2Array(unsigned long u, unsigned char *psz);
unsigned long UTIL_Bcd2Long(unsigned char *pszBuf, unsigned char len);
unsigned char UTIL_CompareBits(unsigned char *A, unsigned char *psz, unsigned int nLen);
unsigned char UTIL_Lrc(unsigned char *pszBuf, unsigned int nLen);
unsigned char UTIL_MemCmp(unsigned char *addr1, unsigned char *addr2, unsigned int nLen);
unsigned short UTIL_TLVGetValue(unsigned char *pszBuf, unsigned long *tag_value, unsigned short *len_value);
void UTIL_Xor(unsigned char *pszSData1, unsigned char *pszSData2, unsigned char *pszSXorData, int iLen);

void UTIL_StrSplit(unsigned char *dest_ptr, unsigned char *pszSrc_ptr, unsigned int nLen);
void UTIL_StrCompress(unsigned char *pbDest, unsigned char *pbSrc, unsigned int destLen);
//void UTIL_StrRtcToUnixTime(unsigned int *punixTime, unsigned int century, IN tDTG_t *ptDTG);
//void UTIL_StrUnixTimeToRtc(unsigned int *pCentury, OUT tDTG_t *ptDTG, unsigned int unixTime);

unsigned short UTIL_TLVGetTagLength(unsigned char *pbTag);
unsigned short UTIL_TLVGetLengthLength(unsigned char *pbLength, unsigned short *pusValueLen);
unsigned short UTIL_TLVListCheckFormat(unsigned char *baTlvListBuf, unsigned short usTlvListLen);
unsigned short UTIL_TLVListReadTagPkt(unsigned char *baTagName, unsigned char *baTlvListBuf, unsigned short usTlvListLen, unsigned char **pbTlvPkt, unsigned short *pusTlvPktLen);
unsigned short UTIL_TLVListReadTagValue(char *caTagName,unsigned char *baTlvListBuf, unsigned short usTlvListLen, unsigned char **pbTlvValue, unsigned short *pusTlvValueLen);
unsigned short UTIL_TLVListAddTag(unsigned char *baTlvListBuf, unsigned short *pusTlvListLen, unsigned char *baTlvPkt, unsigned short usTlvPktLen);
unsigned short UTIL_TLVFormatTlv(char *caTagName, unsigned short usTagLen, unsigned char *baTagData, unsigned short *pusTlvPktLen, unsigned char *baTlvPktBuf);
unsigned short UTIL_TLVFormatAndUpdateList(char *caTagName, unsigned short usValueLen, unsigned char *baValueData, unsigned short *pusTlvListLen, unsigned char *baTlvListBuf);


#ifdef    __cplusplus
}
#endif

#endif    /* _LIBUTIL_H */
