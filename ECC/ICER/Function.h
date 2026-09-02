#ifndef __FUNCTION_H__
#define __FUNCTION_H__

#ifdef __cplusplus
	extern "C" {
#endif

#include "ICERAPI.h"
#include "Function.h"
#include "TM.h"

//#define ICERAPI_ONLY			1//define this in SDK
//#define ICERAPI_READERAPI		1

#define MAX_MSG			256

/*#if READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
#define DEFAULT_LOG_FILENO (STDOUT_FILENO)
#define MAX_PATH 		256
#elif READER_MANUFACTURERS==NE_PAXSXX
#define MAX_PATH 		256
#elif READER_MANUFACTURERS==CS_PAXSXX
#define MAX_PATH 		256
#elif READER_MANUFACTURERS==SYSTEX_CASTLES
#define MAX_PATH 		256
#elif READER_MANUFACTURERS==NE_CASTLES_API
#define MAX_PATH 		256
#else*/
#if READER_MANUFACTURERS!=WINDOWS_API && READER_MANUFACTURERS!=WINDOWS_CE_API
#define MAX_PATH 		256
#endif

#define	WRITE_DLL_REQ_LOG			1
#define	WRITE_DLL_RES_LOG			2
#define	WRITE_R6_SEND_LOG			3
#define	WRITE_R6_RECV_LOG			4
#define	WRITE_XML_SEND_LOG			5
#define	WRITE_XML_RECV_LOG			6
#define	WRITE_TM_REQ_LOG			7
#define	WRITE_TM_RES_LOG			8

#define	TXN_NORMAL					0
#define	TXN_REVERSAL				1
#define	TXN_ADVICE					2
#define	TXN_UPLOAD					3

#define	CARD_DEDUCT					TRUE
#define	CARD_ADD					FALSE

#define	LOG_LEVEL_ERROR				0
#define	LOG_LEVEL_FLOW				1

typedef union
{
	long Value;
	unsigned char Buf[4];
}UnionLong;

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
typedef __int64 		INT64;
typedef unsigned __int64	UINT64;
#elif READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
#define u32 uint32_t;
typedef long long INT64;
typedef unsigned long long UINT64;
#else
#endif

int compare(const void *arg1, const void *arg2);
int compare3(const void *arg1, const void *arg2);
int fnLogFileProcedure(void);

//#if READER_MANUFACTURERS==NE_PAXSXX
//#define log_msg			CPI_Tracef
//#else
void log_msg(int inLogLevel,const char* fmt, ...);
//#endif

void fnUnPack(BYTE *BINData,int BINSize,BYTE *ASCDATA);
void fnPack(char *chASCDATA,int ASCSize,BYTE *BINData);
void vdWriteLog(BYTE *bLogMsg,int inLogMsgLen,int inLogType,int inTxnType);
void GetCurrentDateTime(unsigned char *bDate);
void UnixToDateTime(unsigned char *bUnixDate,unsigned char *bDate,int inSize);
//#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API
//#if  READER_MANUFACTURERS==WINDOWS_CE_API
int inICERChkSpecialYear(int inYear);
//#endif
long lnDateTimeToUnix(unsigned char *bDate,unsigned char *bTime);
int inInitialTxn(void);
void vdPad(char *bInData,char *bOutData,int inOutSize,char fFlag,char chFiller);
void vdReplace(char *bInData,int inInLen,char *bOutData,char chOldFiller,char chNewFiller);
int inQueryPoint(int inTxnType);
int inDeductOrRefundProcess(int inTxnType);
int inECCDeductOrRefund(BOOL fDeductFlag,int inTxnType,long lnTxnAmt);
//#if READER_MANUFACTURERS==WINDOWS_API
int ECC_strncasecmp(char *chString1,char *chString2,int inLen);
void ECC_LCase(char *InStr,char *OutStr);
//#endif
BOOL fIsOnlineTxn(void);
int inGetREQData(void);
int inECCstrchr(BYTE *bInData,int inInLen,char chWord);
int inDefaultDataCheck(long lnProcCode);
void vdTxnEndProcess(int inTxnType,char *chSettleDate);
//int inSettleProcess();
void vdStoreDeviceID(BYTE *bDeviceID,BYTE bSPID,BYTE bLocID,BYTE *bCPUDeviceID,BYTE *bCPUSPID,BYTE *bCPULocID,BYTE *bReader,BYTE *bReaderFWVersion,BYTE *bCPUSAMID);
void vdStoreTMLocationID(BYTE *bTMLocationID);
int inGetDeviceID(BYTE *bDeviceID/*,STRUCT_TMP_DATA *srTmpData*/);
int inGetTMLocationID(BYTE *bTMLocationID);
void vdStoreAutoloadData(BYTE *bAutoloadData, int iDataLen);
int inGetAutoloadData(BYTE *bAutoloadData);
int inGetSettleDate(BYTE *bSettleDate);
int inGetSettleData(void);
int inGetFileDataByPassEnterKey(char *chFileName,BYTE *bMsg,int inMsgLen,BYTE *bOutData);
void vdTrimData(char *chOutData,char *chInData,int inInLen);
void vdTrimDataRight(char *chOutData,char *chInData,int inInLen);
void vdBatchDataProcess(BYTE *bOutData,BYTE *bInData1,int inInDataLen1,BYTE *bInData2,int inInDataLen2);
void vdBatchDataTransfer(BYTE *bTransferData,int inDataLen);
int inCheckECCEVEnough(int inTxnType);
int inCheckCanAutoload(long lnTxnAmt);
BOOL fIsMustReversal(void);
void vdUpdateCardEV(BYTE *bCardEV,BYTE *bOrgCardEV,int inEVLen,int inOrgEVLen,char *chTagName);
int inASCToLONG(char *chASC,int inASCLen);
BOOL fIsRetry(int inRetVal);
BOOL fIsECCTxn(int inTransType);
int inECCTxnProcess(int inTxnType);
int vdUpdateECCDataAfterTxn(int inTxnType,BYTE *bOutData);
BOOL fIsRetryTxn(int inTxnType,int inRetVal);
void vdIntToAsc(BYTE *bInData,int inInSize,BYTE *bOutData,int inOutSize,char fFlag,char chFiller,int inRadix);
void vdUIntToAsc(BYTE *bInData,int inInSize,BYTE *bOutData,int inOutSize,char fFlag,char chFiller,int inRadix);
void vdUpdateCardInfo(int inTxnType,BYTE *anCardID,BYTE *unPID,BYTE *unCardType,BYTE *unPersonalProfile,BYTE *anEV,BYTE *unNewExpiryDate,BYTE *unOrgExpiryDate,BYTE *unPurseVersionNumber,BYTE *unBankCode,BYTE *unAreaCode,BYTE *unSubAreaCode,BYTE *unDeposit,BYTE *bDeviceID,BYTE *bCPUDeviceID,BYTE *bTxnSN);
void vdUpdateCardInfo2(int inTxnType,BYTE *ucCardID,BYTE *ucPID,BYTE bAutoLoad,BYTE ucCardType,BYTE ucPersonalProfile,BYTE *ucAutoLoadAmt,BYTE *ucEV,BYTE *ucExpiryDate,BYTE ucPurseVersionNumber,BYTE ucBankCode,BYTE ucAreaCode,BYTE *ucSubAreaCode,BYTE *ucDeposit,BYTE *ucTxnSN);
void vdMaskCardID(BYTE *anCardID,BYTE *unPID,BYTE *unPurseVersionNumber,BYTE *bOutData);
void vdGetECCEVBefore(BYTE *anEVBeforeTxn,int inEVLen);
int inCheckProcess(void);
void vdProcessingFlag(BYTE bFlag);
int inECCSettle(int inTxnType);
void vdECCFolderProcess(void);
long lnAmt3ByteToLong(BYTE *b3ByteAmt);
void vdUpdateCardInfo3(int inTxnType,BYTE *anCardID,BYTE *anEVBeforeTxn,BYTE *unNewExpiryDate,BYTE *unOrgExpiryDate);
long lnConvertAutoloadAmt(long lnECCAmt,BYTE *bInALAmt,int inInLen,BYTE *bEVAmt,int inEVLen,BYTE *bOutALAmt);
void vdToUpper(char *szOutData,char *szInData);
int inGetBLCName(BYTE *bBLCName);
int inSetBLCName(BYTE *bBLCName);
void vdGetTmpData(STRUCT_TMP_DATA *srTmpData);
//void vdTest(int inCnt,BYTE *srTmpData);
BYTE bGetLCDControlFlag(void);
void log_msg2(const char* fmt, ...);
void vdSaveReaderDebugLog(BYTE *bReaderDebugLog,int inLen);
int inCMASSettle(int inTxnType);
void vd640EProcess(int inReaderSW,BYTE *bReaderOutData,BYTE *bReaderOutData2);
void vdBuildDeviceIDResponse(STRUCT_XML_DOC *srXML);
void vdCMASBuildRRNProcess(STRUCT_XML_DOC *srXML,unsigned long ulSerialNumber,BYTE *ucTxnSN);
void vdReSendR6ReaderAVR(BYTE *bReaderOutData);

#if READER_MANUFACTURERS==SYSTEX_CASTLES
char toupper(char chInData);
#endif

int inECCBatchUpload(void);
int inCMASBatchUpload(void);

/*#if READER_MANUFACTURERS!=WINDOWS_API && READER_MANUFACTURERS!=LINUX_API && READER_MANUFACTURERS!=ANDROID_API
char toupper(char chInData);
#endif*/

void vdSetCMASTagDefault(STRUCT_XML_DOC *srXML,int inTransType);
BOOL fIsDeductTxn(int inTransType);
int inECCAdvice(void);
BOOL fIsETxn(int inTransType);
int inETxnProcess(int inTxnType);
void log_msg_debug(int inLogLevel,char fDebugFlag,char* chMsg,int inMsgLen,BYTE* chData,int inDataLen);
void vdCloseAntenn(void);
void PKCS5Padding(BYTE *bInData,int inInLen,int inPadBase);
void vdICERTest(int inCnt);
void vdBuildT4213Tag(STRUCT_XML_DOC *srXML);
void vdURTDataProcess(STRUCT_XML_DOC *srXML,BYTE ucPurseVersionNumber,TRANSPORT_DATA *URT);
int inCheckDiscountCanVoid(BYTE *bOutData);
void vdSetICERTxnParameter(char *chOnlineFlag,char *chBatchFlag,char *chTCPIP_SSL,char *PacketLenFlag);
void vdGetICERTxnParameter(char chOnlineFlag,char chBatchFlag,char chTCPIP_SSL,char PacketLenFlag);
void vdICERBuildETxnPCode(BYTE *ucMsgType,BYTE *ucSubType);

#ifdef __cplusplus
	}
#endif // __cplusplus

#endif
