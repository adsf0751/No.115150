#ifndef __TCPIP_H__
#define __TCPIP_H__

#ifdef __cplusplus
	extern "C" {
#endif

#include "XMLFunc.h"

typedef struct
{
	int inSendLen;//傳送資料長度
	int inRecvLen;//接收資料長度
	int inRespCodeAdr;//Response Code位址
	int inCardIDAdr;//Card ID位址,鎖卡用
}STRUCT_TCPIP_PAR;

//某些平台的檔名有長度限制,注意檔名別太長!!
#define		FILE_REQ			"ICERAPI.REQ"				//file
#define		FILE_REQ_OK			"ICERAPI.REQ.OK"			//file
#define		FILE_RES			"ICERAPI.RES"				//file
#define		FILE_RES_OK			"ICERAPI.RES.OK"			//file
#define		FILE_INI			"ICERINI.xml"				//file
#define		FILE_ICER_REV		"ICERAPI_ICER.rev"			//flash		1024 * 10
#define		FILE_ICER_REV_BAK	"ICERAPI_ICER.rev.bak"		//flash		1024 * 10
#define		FILE_ICER_ADV		"ICERAPI_ICER.adv"			//flash		Max
#define 	FILE_CMAS_REV		"ICERAPI_CMAS.rev"			//flash		1024 * 10
#define 	FILE_CMAS_REV_BAK	"ICERAPI_CMAS.rev.bak"		//flash		1024 * 10
#define 	FILE_CMAS_ADV		"ICERAPI_CMAS.adv"			//flash		Max
#define		FILE_TMP			"ICERAPI.tmp"				//flash		512
#define		FILE2_REQ			"ICERAPI2.REQ"				//file
#define		FILE2_RES			"ICERAPI2.RES"				//file
#define		FILE_REQ_CMAS_BAT	"ICERAPI_REQ_CMAS.upl"		//file
#define		FILE_RES_CMAS_BAT	"ICERAPI_RES_CMAS.upl"		//file
#define		FILE_RES_CMAS2_BAT	"ICERAPI_RES_CMAS2.upl"		//file
#define		FILE_LOG			"ICERAPI.Log"				//file
#define		FILE_COMMAND1_BAK	"ICERAPI_Comm1.bak"			//file
#define		FILE_REQ_BAK		"ICERAPI.REQ.bak"			//file
#define		FILE_PERMISSION     "PERMISSION"       			//file

#ifdef	FLASH_SYSTEM

#define		MAX_FILE_CNT		5//不算OK檔,要算Log檔

enum
{
	//STORAGE_REQ = 0,
	//STORAGE_REQ_OK,
	//STORAGE_RES,
	//STORAGE_RES_OK,
	//STORAGE_INI,
	//STORAGE_ICER_REV,
	//STORAGE_ICER_REV_BAK,
	//STORAGE_ICER_ADV,
	STORAGE_CMAS_REV = 0,
	STORAGE_CMAS_REV_BAK,
	STORAGE_CMAS_ADV,
	STORAGE_TMP,
	//STORAGE2_REQ,
	//STORAGE2_RES,
	//STORAGE_REQ_CMAS_BAT,
	//STORAGE_RES_CMAS_BAT,
	//STORAGE_RES_CMAS2_BAT,
	STORAGE_LOG
};

typedef struct
{
	BYTE bFileName[30];
	unsigned long ulStartAddress;
	unsigned long ulFlashMaxSize;
	unsigned long ulEndAddress;
}STRUCT_FLASH_PAR;

typedef struct
{
	unsigned long ulFlashDataSize[MAX_FILE_CNT];
	unsigned long ulFlashEndDataAddr[MAX_FILE_CNT];//Flash絕對位置
}STRUCT_FLASH_DATA;

#define		FLASH_PAR						0x50000//0 ~ 768
#define		FLASH_START						FLASH_PAR + 0x300
//#define		FLASH_ICER_REV				FLASH_START + 0x0000//1024 * 10
//#define		FLASH_ICER_REV_BAK			FLASH_START + 0x2800//1024 * 10
#define		FLASH_START_CMAS_REV			FLASH_START + 0x0000//0 ~ 10240
#define		FLASH_START_CMAS_REV_BAK		FLASH_START + 0x2800//10240 ~ 20480
#define		FLASH_START_TMP					FLASH_START + 0x5000//20480 ~ 20992
#define 	FLASH_START_CMAS_ADV			FLASH_START + 0x5200//20992 ~ 5140992,一筆需10240,500筆為5120000
#define 	FLASH_START_LOG					FLASH_START + 0x4E7200//5140992 ~ 10260992

#define		FLASH_CMAS_REV_SIZE				1024 * 10
#define		FLASH_CMAS_REV_BAK_SIZE			1024 * 10
#define		FLASH_TMP_SIZE					512
#define 	FLASH_CMAS_ADV_SIZE				1024 * 10 * 500//(5 M)
#define 	FLASH_LOG_SIZE					1024 * 10 * 500//(5 M)

#define		STORAGE_FLASH					TRUE
#define		STORAGE_FILE					FALSE

#endif//FLASH_SYSTEM

#define		REVERSAL_ON						TRUE
#define		REVERSAL_OFF					FALSE
#define		ADVICE_ON						TRUE
#define		ADVICE_OFF						FALSE
#define		STORE_BATCH_ON					TRUE
#define		STORE_BATCH_OFF					FALSE

int inTCPSendAndReceive(int inDLLType,unsigned char * sendbuf ,unsigned char * recebuf,char fConfirm);
int inSendRecvXML(int inHostType,STRUCT_XML_DOC *srXML,int inTxnType);
int inReversalProcess(int inHostType);
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
int inTCPIPSendData(BYTE *bSendData,int inSendLen);
#else
int inTCPIPSendData(unsigned char *IP,unsigned short Port,BYTE *bSendData,int inSendLen);
#endif
int inTCPIPRecvData(BYTE *bRecvData,int inTCPHeaderLen);
int inTCPIPSendRecv(BYTE *bPCode,unsigned char *IP,unsigned short Port,BYTE *bSendBuf,int inSendLen,BYTE *bRecvData,char fConfirm);
int inTXMLSendRecv(BYTE *bPCode,int inMTI,unsigned char *IP,unsigned short Port,BYTE *bSendBuf,int inSendLen,BYTE *bRecvData,char fConfirm);
int inBuildSendPackage(int inHostType,STRUCT_XML_DOC *srXML,BYTE *bOutData,BOOL fReversalFlag,BOOL fAdviceFlag,BOOL fStoreBatchFlag);
int inTCPIPAnalyzePackage(int inHostType,int inTCPIPTxnType,BYTE *bInData,int inLen,STRUCT_XML_DOC *srXMLOut);
int inStoreAdviceFile(int inHostType,BYTE *bInData,int inLen,BOOL fStoreBatchFlag,int inMTIAdr);
int inTCPIPAdviceProcess(int inHostType,BOOL fSendAllAdvFile);
void vdReversalOn(int inHostType);
void vdReversalOff(int inHostType);
//int inGetNowAdvFileName(char *chOutName);
//int inGetOldAdvFileName(char *chOutName);
int inCheckHostSTAC(STRUCT_XML_DOC *srXMLOut,int inSTACIndex);
int inHelloTest(int inHostType);
int inCMASBatchUploadProcess(BYTE *bSendData,long lnSendLen);
char chGetchPacketLenFlag(void);
int inTCPIPRecvByLen(BYTE *lpBuf,unsigned long dwToRead,clock_t tTimeout);
int inCheckICERT6410(STRUCT_XML_DOC *srXML);
int inTCPIPServerListen(unsigned short Port);

#ifdef __cplusplus
	}
#endif // __cplusplus

#endif
