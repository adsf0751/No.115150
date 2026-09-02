#ifndef __BANKDATA_H__
#define __BANKDATA_H__

#ifdef __cplusplus
	extern "C" {
#endif

#include "SubStruct.h"
#include "XMLFunc.h"

#define IN
#define OUT

//#define	MAX_ISO8583_FLAG_CNT		32
#define	MAX_ISO8583_FLAG_CNT		36

#define	MTI_NORMAL_TXN				0
#define	MTI_ADVICE_TXN				1
#define	MTI_REVERSAL_TXN			2
#define	MTI_UPLOAD_TXN				3

typedef struct
{
	IN  BYTE bPCode[6 + 1];
	OUT BYTE bTxnTagName[20];
	IN  int inMTI;
	OUT int inBefLen;
	OUT int inAftLen;
	OUT int inF11Adr;
	OUT int inF39Adr;
	OUT int inF62Adr;
	OUT int inECCAdr;
	OUT int inNCCCEncryptFlag;
	OUT int inF03Adr;
	OUT int inF04Adr;
	OUT int inF35Adr;
	OUT int inF37Adr;
	OUT int inF57Adr;
	OUT int inF59Adr;
	OUT int inF63Adr;
}STRUCT_ADD_DATA;

typedef struct
{
	IN int inF11AddFlag;
	IN int inF62AddFlag;
}STRUCT_ISO8583_BANK_FLAG;

typedef struct
{
	IN BYTE bPCode[6 + 1];
	IN int inMTI;
	STRUCT_ISO8583_BANK_FLAG st_ISO8583BankFlag[4];//NO_ADD_DATA + NCCC_ADD_DATA + CTCB_ADD_DATA  + FISC_ADD_DATA
}STRUCT_ISO8583_FLAG;

#define STRUCT_ADD_DATA_LEN		sizeof(STRUCT_ADD_DATA)

int inAddTcpipDataSend(STRUCT_XML_DOC *srBankXML,STRUCT_ADD_DATA *srAddData,BYTE *bSendData,BYTE *bSendBuf,int inSendLen);
int inAddTcpipDataRecv(STRUCT_XML_DOC *srBankXML,STRUCT_ADD_DATA *srAddData,BYTE *bRecvData,BYTE *bRecvBuf,int inRecvLen);
int inAddTcpipDataModify(STRUCT_ADD_DATA *srAddData,BYTE *bSendBuf,int inSendLen);
void vdGetISO8583FlagIndex(BYTE *bPCode,int inMTI);
void vdIcnBankData(int inIncType);
long lnGetEVByPCode(STRUCT_XML_DOC *srXML,long lnEVBef);
int inNCCCEncryptionData(STRUCT_ADD_DATA *srAddData, BYTE *bData);

#ifdef __cplusplus
	}
#endif // __cplusplus

#endif

