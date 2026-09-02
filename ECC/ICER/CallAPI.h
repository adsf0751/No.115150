#ifndef __CALLAPI_H__
#define __CALLAPI_H__

#ifdef __cplusplus
	extern "C" {
#endif

#include "Function.h"
#include "XMLFunc.h"

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
#define WM_DONGLE_GETDATA		WM_APP+9001
#define WM_DONGLE_ERROR			WM_APP+9002
#endif

enum
{
	DLL_OPENCOM = 0,
	DLL_SIGNONQUERY,
	DLL_RESET,
	DLL_RESET_OFF,
	DLL_SIGNON,
	DLL_READ_CODE_VERSION,
	DLL_READ_CARD_BASIC_DATA,
	DLL_DEDUCT_VALUE,
	DLL_DEDUCT_VALUE_AUTH,
	DLL_ADD_VALUE,
	DLL_ADD_VALUE_AUTH,//10
	DLL_VOID_TXN,
	DLL_AUTOLOAD,
	DLL_AUTOLOAD_AUTH,
	DLL_READ_DONGLE_LOG,
	DLL_READ_CARD_LOG,
	DLL_TX_REFUND,
	DLL_TX_REFUND_AUTH,
	DLL_TX_ADD_VALUE,
	DLL_TX_ADD_VALUE_AUTH,
	DLL_SET_VALUE,//20
	DLL_SET_VALUE_AUTH,
	DLL_AUTOLOAD_ENABLE,
	DLL_AUTOLOAD_ENABLE_AUTH,
	DLL_ACCUNT_LINK,
	DLL_ACCUNT_LINK_AUTH,
	DLL_LOCK_CARD,
	DLL_READ_CARD_NUMBER,
	DLL_ADD_VALUE2,
	DLL_MULTI_SELECT,
	DLL_ANTENN_CONTROL,//30
	DLL_TAXI_READ,
	DLL_TAXI_DEDUCT,
	DLL_ADD_OFFLINE,
	DLL_MATCH,
	DLL_MATCH_AUTH,
	DLL_FAST_READ_CARD,
	DLL_FAST_READ_CARD2,
	DLL_RESET2,
	DLL_MATCH2,
	DLL_S_SET_VALUE,//40
	DLL_S_SET_VALUE_AUTH,
	DLL_THSRC_READ,
	DLL_E_ADD_VALUE,
	DLL_VERIFY_HOST_CRYPT,
	DLL_E_QR_TXN,
	DLL_QR_VERIFY_HOST_CRYPT,
	DLL_MATCH_OFF,
	DLL_CBIKE_READ,
	DLL_CBIKE_DEDUCT,
	DLL_EDCA_READ,//50
	DLL_EDCA_DEDUCT,
	DLL_TLRT_AUTH,
	DLL_ADD_VALUE1,//代收售退貨用
	DLL_ADD_VALUE_AUTH1,//代收售退貨用
	DLL_REFUND_CARD_AUTH,//連線退卡
	DLL_DEDUCT_VALUE1, // 後台加值查詢
	DDL_DEDUCT_VALUE2, // 購貨(含菸酒)
	DLL_GET_CMASMODE,//下載CMAS MODE
	DLL_ONLINE_CHECK_CARD,//連線卡片查詢
	DLL_CLOSECOM
};

#define	DLL_TOTAL_CNT				DLL_CLOSECOM + 1

#define MIFARE		0x00
#define LEVEL1		0x01
#define LEVEL2		0x02

#define READER_ECC			'0'
#define READER_INFOCHAMP	'1'//中冠
#define READER_CASTLES		'2'//虹堡
#define READER_SYMLINK		'3'//瑞士普210

#define READER_BUFFER		1024

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
typedef int (WINAPI *pFuncType1)(int,DWORD);//OpenCom指令專用型態
typedef void(WINAPI *pFuncType2)(); 		//Request-無傳入參數指令型態
typedef void(WINAPI *pFuncType3)(BYTE *);		//Request-有傳入參數指令型態
typedef int (WINAPI *pFuncType4)(BYTE *);		//Response--有回傳參數指令型態
typedef int (WINAPI *pFuncType5)(); 		//Response--無回傳參數指令型態
typedef int (WINAPI *pFuncType6)(int,DWORD,long);//OpenCom3指令專用型態

typedef struct
{
	BYTE bReaderCommand[50];//API指令
	int inInFuncType;//Request指令型態
	int inOutFuncType;//Response指令型態
	int inTMInLen;//TM InPut_Data長度
	//int inTMOutLen6304;//TM OutPut_Data長度 6304 6305
	int inTMOutLen9000;//TM OutPut_Data長度 9000 6403
	int inTMOutLen6415;//TM OutPut_Data長度 6415
	int inTMOutLen6103;//TM OutPut_Data長度 6103 610F 6406 640E 6418
}STRUCT_READER_INFO;

typedef struct
{
	int inTxnType;
	STRUCT_READER_INFO srReaderInfoL1;//一代指令參數
	STRUCT_READER_INFO srReaderInfoL2;//二代指令參數
}STRUCT_READER_COMMAND;

#else//READER_MANUFACTURERS==LINUX_API

typedef struct
{
	int inTxnType;
	BYTE bReaderCommand[50];//API指令
	int inFuncType;
	int inTMInLen;//TM InPut_Data長度
	int inTMOutLen9000;//TM OutPut_Data長度 9000 6403
	int inTMOutLen6415;//TM OutPut_Data長度 6415
	int inTMOutLen6103;//TM OutPut_Data長度 6103 610F 6406 640E 6418
}STRUCT_READER_COMMAND;

#endif

typedef int(*pTMFunction)(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *APIOut,BYTE *DongleIn,BYTE *DongleOut,BYTE *DongleReqOut);
typedef int(*pAPDUFunction)(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);

typedef struct
{
	BYTE bReaderCommand[50];//API指令
	BYTE CLA;//CLA
	BYTE INS;//INS
	BYTE P1;//P1
	BYTE P2;//P2
	int LC;
	//BYTE bInData[256];
	int LE;
	pAPDUFunction pR6_APDUFun;
	pTMFunction   pR6_TMFun_OK1;//9000 or 6403
	pTMFunction   pR6_TMFun_OK2;//6415 or 6308
	pTMFunction   pR6_TMFun_Lock1;
	pTMFunction   pR6_TMFun_Lock2;
	pAPDUFunction pSIS2_APDUFun;
	pTMFunction   pSIS2_TMFun_OK;
	pTMFunction   pSIS2_TMFun_Lock1;
	pTMFunction   pSIS2_TMFun_Lock2;
	pAPDUFunction pCMAS_APDUFun;
	pTMFunction   pCMAS_TMFun_OK;
	pTMFunction   pCMAS_TMFun_Lock1;
	pTMFunction   pCMAS_TMFun_Lock2;
	pAPDUFunction pICER_APDUFun;
	pTMFunction   pICER_TMFun_OK;
	pTMFunction   pICER_TMFun_Lock1;
	pTMFunction   pICER_TMFun_Lock2;
}STRUCT_READER_INFO2;

typedef struct
{
	int inTxnType;
	unsigned long ulTimeOut;//Time Out
	char chCardAccess;
	//STRUCT_READER_INFO srReaderInfoL1;//一代指令參數
	STRUCT_READER_INFO2 srReaderInfoL2;//二代指令參數
}STRUCT_READER_COMMAND2;

BYTE checksum(int length,BYTE data[]);
int inLoadLibrary(void);
void vdFreeLibrary(void);
int inReaderCommand(int inDLLType,STRUCT_XML_DOC *srXML,BYTE *bInData,BYTE *bOutData,BOOL bBatchFlag,int inLC,BOOL bAPILock);
int inReaderCommand2(int inDLLType,STRUCT_XML_DOC *srXML,BYTE *bInData,BYTE *bOutData,BOOL bBatchFlag,int inLC,BOOL bAPILock);
int inSignOnQueryProcess(BYTE *bInData,BYTE *bOutData);
int inSignOnQueryProcess2(int inSW,BYTE *bInData,BYTE *bOutData);
int inSignOnProcess(int inTxnType,BYTE *bOutData,BOOL fClearData);
void vdSignOnReturn(BOOL bRetFlag,BOOL fClearData,STRUCT_XML_DOC *srTmpXML,STRUCT_XML_DOC *srXML,BYTE *bInData,BYTE *bOutDataa);
int inQueryCardData(int inTxnType);
int inBuildAPIDefaultData(int inDLLType,unsigned long ulTMSerialNo,BYTE *bOutData,int *inDateTimeAdr);
int inGetCardData(int inTxnType,BOOL fHostMACCheckFlag);
BOOL fIsAutoloadProcess(BYTE *bAPIOut,long *lnOutALAmt);
int inAutoloadProcess(BYTE *bAPIOut);
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
int inGetTMOutLen(int inDLLRet,STRUCT_READER_INFO *srReaderInfo,int inDLLType);
#else//READER_MANUFACTURERS==LINUX_API
int inGetTMOutLen(int inDLLRet,STRUCT_READER_COMMAND *srReaderInfo,int inDLLType);
#endif
char *chGetDLLName(int inDLLType);
void vdMakeAData2(char chPurseVersion,BYTE *bInData);
int inBuildTxnAmt(BYTE *bOutData,long lnTxnAmt);
int inDoECCAmtTxn(int inTxnType);
int inDoECCQueryTxn(int inTxnType);
int inBuildTxRefundInData(BYTE *bOutData,unsigned long ulTMSerialNo,int inAddCnt);
int inLockCard(BYTE *bCardID,BOOL bAPILock);
int inBuildNullAPDU(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In);
int inBuildNullTM(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut);
int inBuildRefundCardInData(BYTE *bOutData);
int inECCReadCardBasicCheck(int inTxnType,BYTE *bInOutData);
BOOL bIsResetOfflineTxn(int inTransType);
BOOL f6403Check(STRUCT_XML_DOC *srXML,BYTE *bAPDUInData,BYTE *bAPDUOutData,int inAPDURet);
int inDoICERTxn(int inTxnType);
int inDoICERQRTxn(int inTxnType);
int inResetOfflineProcess(int inDLLType,STRUCT_XML_DOC *srXML,BYTE *bInData,BYTE *bOutData);
BOOL fECCCheckETxnCard(BYTE *bOutData);

#ifdef __cplusplus
	}
#endif // __cplusplus

#endif
