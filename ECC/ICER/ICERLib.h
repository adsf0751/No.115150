#ifndef __ICERLIB_H__
#define __ICERLIB_H__

#ifdef __cplusplus
		 extern "C" {
#endif

//#ifdef ICERAPI_ONLY
#define FULL_FILE_NAME_MAX         500

int ECC_FileOpen(BYTE* bFileName,BYTE* bFileMode);
int ECC_FileRead(int FileHandle,BYTE* bFileData,unsigned short usRLen);
int ECC_FileWrite(int FileHandle,BYTE* bFileData,unsigned short usWLen);
int ECC_FileClose(int FileHandle);
int ECC_FileSeek(int FileHandle,long lnOffset,unsigned char bFormWhere);
int ECC_FileSize(BYTE* bFileName);
int ECC_FileRemove(BYTE* bFileName);
int ECC_FileRename(BYTE* bFileName1,BYTE* bFileName2);
int ECC_FileCopy(BYTE* bFileName1,BYTE* bFileName2);
int ECC_CreateDirectory_Lib(char *bFileName);

short ECC_TCPIPConnect(BYTE* bIP,unsigned short usPort,unsigned short usTimeOut);
short ECC_TCPIPSend(BYTE* bSendData,unsigned short usSendLen,unsigned short usTimeOut);
short ECC_TCPIPRecv(BYTE* bRecvData,unsigned short* usRecvLen,unsigned short usTimeOut);
short ECC_TCPIPClose(void);

short ECC_UnixToDateTime(unsigned char *bUnixTime,unsigned char *bDate);
short ECC_DateTimeToUnix(unsigned char *bDateuchar,unsigned char *bUnixTime);
short ECC_INT64ToASCII(unsigned char *bData,unsigned char *bAsciiData);
short ECC_UINT64ToASCII(unsigned char *bData,unsigned char *bAsciiData);
short ECC_ASCIIToINT64(unsigned char *bAsciiData,unsigned short usLen,unsigned char *bData);
short ECC_ASCIIToUINT64(unsigned char *bAsciiData,unsigned short usLen,unsigned char *bData);

//#endif

int inFileDelete(char *bFileName,char chPathFlag);
int inFileRename(char *bOldName,char *NewName);
int inFileGetSize(char *bFileName,char chPathFlag);
int inFileRead(char *bFileName,BYTE *bFileData,int inFileDataLen,int inFileReadCnt);
int inFileRead2(char *bFileName,BYTE *bFileData,int inFileDataLen,int inFileReadCnt,int inOffset);
int inFileWrite(char *bFileName,BYTE *bFileData,int inFileWriteCnt);
int inFileAppend(char *bFileName,BYTE *bFileData,int inFileWriteCnt);
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API
int inFileModify(FILE *FileHandle,BOOL bOpened,char *bFileName,BYTE *bFileData,int inFileWriteCnt,int inFileSeek);
#else
int inFileModify(int FileHandle,BOOL bOpened,char *bFileName,BYTE *bFileData,int inFileWriteCnt,int inFileSeek);
#endif
int inFileCopy(char *bOldName,char *NewName);

short ECC_SSLConnect(BYTE* bIP,unsigned short usPort,unsigned short usTimeOut);
short ECC_SSLSend(BYTE* bSendData,unsigned short usSendLen,unsigned short usTimeOut);
short ECC_SSLRecv(BYTE* bRecvData,unsigned short* usRecvLen,unsigned short usTimeOut);
short ECC_SSLClose(void);
short ECC_NCCC_Encryption_TSAM(BYTE *bData, unsigned int *dataLen, BYTE fieldNo, BYTE *keyField37, BYTE *checksumField57, BYTE *keyIndex);
short ECC_NCCC_MAC(unsigned int msgType, BYTE *bData, unsigned int idxField3, unsigned idxField4, unsigned int idxField11, unsigned int idxField35, unsigned int idxField59, unsigned int idxField63, BYTE *MACData);
#ifdef	FLASH_SYSTEM
short ICER_WriteDataFlash(unsigned long ulAddress, unsigned char *bpData, unsigned short usDataLen);
short ICER_ReadDataFlash(unsigned long ulAddress, unsigned char *bpData, unsigned short usDataLen);

BOOL fCheckStorageMode(BYTE* bFileName);

int inGetICERLogSize(unsigned long *ulOutSize);
unsigned long inGetICERLog(unsigned long ulStartAdress,BYTE *bOutLogData,unsigned long ulOutSize,BOOL fClearFlag);

#endif

int inReadFlasfData(void);
int inWriteFlasfData(void);
//void vdReversalTest();

//short ECC_AES(unsigned char bMode,unsigned char* bpKey,unsigned char bKeyType,unsigned char* bpInData,unsigned short usInLen, unsigned char* bpOutData);

BYTE *ECC_calloc(unsigned long ulSize1,unsigned long ulSize2);
void ECC_free(BYTE *bFreeAdr);

#ifdef __cplusplus
		 }
#endif // __cplusplus

#endif
