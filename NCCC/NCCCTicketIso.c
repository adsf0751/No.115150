#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <sqlite3.h>
#include "../SOURCE/INCLUDES/Define_1.h"
#include "../SOURCE/INCLUDES/Define_2.h"
#include "../SOURCE/INCLUDES/TransType.h"
#include "../SOURCE/INCLUDES/Transaction.h"
#include "../SOURCE/INCLUDES/AllStruct.h"
#include "../SOURCE/DISPLAY/Display.h"
#include "../SOURCE/DISPLAY/DispMsg.h"
#include "../SOURCE/DISPLAY/DisTouch.h"
#include "../SOURCE/PRINT/Print.h"
#include "../SOURCE/FUNCTION/Function.h"
#include "../SOURCE/FUNCTION/FuncTable.h"
#include "../SOURCE/FUNCTION/File.h"
#include "../SOURCE/FUNCTION/Sqlite.h"
#include "../SOURCE/FUNCTION/Batch.h"
#include "../SOURCE/EVENT/MenuMsg.h"
#include "../SOURCE/EVENT/Flow.h"
#include "../SOURCE/FUNCTION/Accum.h"
#include "../SOURCE/FUNCTION/EDC.h"
#include "../SOURCE/FUNCTION/HDT.h"
#include "../SOURCE/FUNCTION/HDPT.h"
#include "../SOURCE/FUNCTION/CPT.h"
#include "../SOURCE/FUNCTION/CFGT.h"
#include "../SOURCE/FUNCTION/MVT.h"
#include "../SOURCE/FUNCTION/KMS.h"
#include "../SOURCE/FUNCTION/TDT.h"
#include "../SOURCE/FUNCTION/IPASSDT.h"
#include "../SOURCE/FUNCTION/ICASHDT.h"
#include "../SOURCE/FUNCTION/Utility.h"
#include "../SOURCE/FUNCTION/ECR.h"
#include "../SOURCE/COMM/Comm.h"
#include "../CREDIT/Creditfunc.h"
#include "../CTLS/CTLS.h"
#include "../FISC/NCCCfisc.h"
#include "../EMVSRC/EMVsrc.h"
#include "../IPASS/IPASSFunc.h"
#include "NCCCTicketIso.h"
#include "NCCCsrc.h"
#include "NCCCtSAM.h"
#include "NCCCtmk.h"
#include "NCCCats.h"
#include "NCCCesc.h"
#include "NCCCdcc.h"
#include "NCCCtmsCPT.h"
#include "NCCCloyalty.h"
#include "NCCCTicketSrc.h"
#include "TAKAsrc.h"

unsigned char		guszNCCC_TICKET_ISO_Field03[_NCCC_TICKET_PCODE_SIZE_ + 1];
unsigned char		guszTICKET_MTI[4 + 1];
extern	int		ginECC_F57_Len;
extern	int		ginECC_F59_ET_Len;
extern	int		ginECC_F63_Len;
extern	int		ginDebug; /* Debug使用 extern */
extern	int		ginISODebug;
extern	int		ginFindRunTime;
extern	int		ginMachineType;
extern	int		ginMacError;
extern	int		ginAPVersionType;
extern	char		gszTermVersionID[16 + 1];
extern	char		gszMAC_F_03[2 + 1];	/* Field_3:	Processing Code */
extern	char		gszMAC_F_04[12 + 1];	/* Field_4:	Amount, Transaction */
extern	char		gszMAC_F_11[2 + 1];	/* Field_11:	System Trace Audit Number */
extern	char		gszMAC_F_35[40 + 1];	/* Field_35:	TRACK 2 Data */
extern	char		gszMAC_F_59[18 + 1];	/* Field_59:	F _59 之 Table ID “ 電票交易訊 )18 碼。 */
extern	char		gszMAC_F_63[45 + 1];	/* Field_63:	Reserved-Private Data */
extern	unsigned char	guszEnormousNoNeedResetBit;	/* 資料庫是否須從頭找 */
extern	unsigned char	guszField_35;
extern	unsigned char	guszField_56;
extern	unsigned char	gusztSAMKeyIndex;
extern	unsigned char	gusztSAMCheckSum_35[4 + 1];
extern	unsigned char	gusztSAMCheckSum_56[4 + 1];
extern	int		ginSpecialSituation;
extern	int		ginBeforeIndex;
/* 傳回主機結果用 */
extern EMV_CONFIG	EMVGlobConfig;


ISO_FIELD_NCCC_TICKET_TABLE srNCCC_TICKET_ATS_ISOFieldPack[] =
{
        {3,             inNCCC_TICKET_ATS_Pack03},
        {4,             inNCCC_TICKET_Pack04},
        {11,            inNCCC_TICKET_Pack11},
        {12,            inNCCC_TICKET_Pack12},
        {13,            inNCCC_TICKET_Pack13},
        {22,            inNCCC_TICKET_Pack22},
        {24,            inNCCC_TICKET_Pack24},
        {25,            inNCCC_TICKET_Pack25},
        {32,            inNCCC_TICKET_Pack32},
        {35,            inNCCC_TICKET_Pack35},
        {37,            inNCCC_TICKET_Pack37},
        {38,            inNCCC_TICKET_Pack38},
        {41,            inNCCC_TICKET_Pack41},
        {42,            inNCCC_TICKET_Pack42},
        {48,            inNCCC_TICKET_Pack48},
        {56,            inNCCC_TICKET_Pack56},
        {59,            inNCCC_TICKET_ATS_Pack59},
        {60,            inNCCC_TICKET_Pack60},
        {62,            inNCCC_TICKET_Pack62},
        {63,            inNCCC_TICKET_Pack63},
	{64,            inNCCC_TICKET_Pack64},
        {0,             NULL},  /* 最後一組一定要放 0 */
};

ISO_FIELD_NCCC_TICKET_TABLE srNCCC_TICKET_MFES_ISOFieldPack[] =
{
        {3,             inNCCC_TICKET_MFES_Pack03},
        {4,             inNCCC_TICKET_Pack04},
        {11,            inNCCC_TICKET_Pack11},
        {12,            inNCCC_TICKET_Pack12},
        {13,            inNCCC_TICKET_Pack13},
        {22,            inNCCC_TICKET_Pack22},
        {24,            inNCCC_TICKET_Pack24},
        {25,            inNCCC_TICKET_Pack25},
        {32,            inNCCC_TICKET_Pack32},
        {35,            inNCCC_TICKET_Pack35},
        {37,            inNCCC_TICKET_Pack37},
        {38,            inNCCC_TICKET_Pack38},
        {41,            inNCCC_TICKET_Pack41},
        {42,            inNCCC_TICKET_Pack42},
        {48,            inNCCC_TICKET_Pack48},
        {56,            inNCCC_TICKET_Pack56},
        {59,            inNCCC_TICKET_MFES_Pack59},
        {60,            inNCCC_TICKET_Pack60},
        {62,            inNCCC_TICKET_Pack62},
        {63,            inNCCC_TICKET_Pack63},
	{64,            inNCCC_TICKET_Pack64},
        {0,             NULL},  /* 最後一組一定要放 0 */
};

ISO_FIELD_NCCC_TICKET_TABLE srNCCC_TICKET_ATS_ISOFieldPack_tSAM[] =
{
        {3,             inNCCC_TICKET_ATS_Pack03},
        {4,             inNCCC_TICKET_Pack04},
        {11,            inNCCC_TICKET_Pack11},
        {12,            inNCCC_TICKET_Pack12},
        {13,            inNCCC_TICKET_Pack13},
        {22,            inNCCC_TICKET_Pack22},
        {24,            inNCCC_TICKET_Pack24},
        {25,            inNCCC_TICKET_Pack25},
        {32,            inNCCC_TICKET_Pack32},
        {35,            inNCCC_TICKET_Pack35_tSAM},
        {37,            inNCCC_TICKET_Pack37},
        {38,            inNCCC_TICKET_Pack38},
        {41,            inNCCC_TICKET_Pack41},
        {42,            inNCCC_TICKET_Pack42},
        {48,            inNCCC_TICKET_Pack48},
        {56,            inNCCC_TICKET_Pack56_tSAM},
	{57,		inNCCC_TICKET_Pack57_tSAM},
        {59,            inNCCC_TICKET_ATS_Pack59},
        {60,            inNCCC_TICKET_Pack60},
        {62,            inNCCC_TICKET_Pack62},
        {63,            inNCCC_TICKET_Pack63},
	{64,            inNCCC_TICKET_Pack64},
        {0,             NULL},  /* 最後一組一定要放 0 */
};

ISO_FIELD_NCCC_TICKET_TABLE srNCCC_TICKET_ATS_ISOFieldUnPack[] =
{
        {24,            inNCCC_TICKET_UnPack24},
	{32,            inNCCC_TICKET_UnPack32},
        {38,            inNCCC_TICKET_UnPack38},
        {39,            inNCCC_TICKET_UnPack39},
        {56,            inNCCC_TICKET_UnPack56},
        {59,            inNCCC_TICKET_UnPack59},
	{60,            inNCCC_TICKET_UnPack60},
        {0,             NULL}, /* 最後一組一定要放【0】!! */
};

ISO_FIELD_NCCC_TICKET_TABLE srNCCC_TICKET_MFES_ISOFieldUnPack[] =
{
        {24,            inNCCC_TICKET_UnPack24},
	{32,            inNCCC_TICKET_UnPack32},
        {38,            inNCCC_TICKET_UnPack38},
        {39,            inNCCC_TICKET_UnPack39},
        {56,            inNCCC_TICKET_UnPack56},
        {59,            inNCCC_TICKET_UnPack59},
	{60,            inNCCC_TICKET_UnPack60},
        {0,             NULL}, /* 最後一組一定要放【0】!! */
};

ISO_CHECK_NCCC_TICKET_TABLE srNCCC_TICKET_ATS_ISOFieldCheck[] =
{
        {3,             inNCCC_TICKET_Check03},
        {4,             inNCCC_TICKET_Check04},
	{32,		inNCCC_TICKET_Check32},
        {41,            inNCCC_TICKET_Check41},
        {0,		NULL}, /* 最後一組一定要放【0】!! */
};

ISO_CHECK_NCCC_TICKET_TABLE srNCCC_TICKET_MFES_ISOFieldCheck[] =
{
        {3,             inNCCC_TICKET_Check03},
        {4,             inNCCC_TICKET_Check04},
	{32,		inNCCC_TICKET_Check32},
        {41,            inNCCC_TICKET_Check41},
        {0,		NULL}, /* 最後一組一定要放【0】!! */
};

ISO_FIELD_TYPE_NCCC_TICKET_TABLE srNCCC_TICKET_ATS_ISOFieldType[] =
{
        {3,             _NCCC_TICKET_ISO_BCD_,         VS_FALSE,	6},
        {4,             _NCCC_TICKET_ISO_BCD_,         VS_FALSE,	12},
        {11,            _NCCC_TICKET_ISO_BCD_,         VS_FALSE,	6},
        {12,            _NCCC_TICKET_ISO_BCD_,         VS_FALSE,	6},
        {13,            _NCCC_TICKET_ISO_BCD_,         VS_FALSE,	4},
        {22,            _NCCC_TICKET_ISO_BCD_,         VS_FALSE,	4},
        {24,            _NCCC_TICKET_ISO_BCD_,         VS_FALSE,	4},
        {25,            _NCCC_TICKET_ISO_BCD_,         VS_FALSE,	2},
        {32,            _NCCC_TICKET_ISO_BYTE_1_,      VS_FALSE,	0},
        {35,            _NCCC_TICKET_ETICKET_CARD_,    VS_FALSE,	0},
        {37,            _NCCC_TICKET_ISO_ASC_,         VS_FALSE,	12},
        {38,            _NCCC_TICKET_ISO_ASC_,         VS_FALSE,	6},
        {39,            _NCCC_TICKET_ISO_ASC_,         VS_FALSE,	2},
        {41,            _NCCC_TICKET_ISO_ASC_,         VS_FALSE,	8},
        {42,            _NCCC_TICKET_ISO_ASC_,         VS_FALSE,	15},
        {48,            _NCCC_TICKET_ISO_BYTE_3_,      VS_TRUE,		0},
        {56,            _NCCC_TICKET_ISO_BYTE_3_,      VS_FALSE,	0},
	{57,            _NCCC_TICKET_ISO_BYTE_3_,      VS_FALSE,	0},
        {59,            _NCCC_TICKET_ISO_BYTE_3_,      VS_TRUE,		0},
        {60,            _NCCC_TICKET_ISO_BYTE_3_,      VS_TRUE,		0},
        {62,            _NCCC_TICKET_ISO_BYTE_3_,      VS_TRUE,		0},
        {63,            _NCCC_TICKET_ISO_BYTE_3_,      VS_TRUE,		0},
	{64,            _NCCC_TICKET_ISO_BCD_,         VS_FALSE,       16},
        {0,             _NCCC_TICKET_ISO_BCD_,         VS_FALSE,	0},  /* 最後一組一定要放 0 */
};

ISO_FIELD_TYPE_NCCC_TICKET_TABLE srNCCC_TICKET_MFES_ISOFieldType[] =
{
        {3,             _NCCC_TICKET_ISO_BCD_,         VS_FALSE,	6},
        {4,             _NCCC_TICKET_ISO_BCD_,         VS_FALSE,	12},
        {11,            _NCCC_TICKET_ISO_BCD_,         VS_FALSE,	6},
        {12,            _NCCC_TICKET_ISO_BCD_,         VS_FALSE,	6},
        {13,            _NCCC_TICKET_ISO_BCD_,         VS_FALSE,	4},
        {22,            _NCCC_TICKET_ISO_BCD_,         VS_FALSE,	4},
        {24,            _NCCC_TICKET_ISO_BCD_,         VS_FALSE,	4},
        {25,            _NCCC_TICKET_ISO_BCD_,         VS_FALSE,	2},
        {32,            _NCCC_TICKET_ISO_BYTE_1_,      VS_FALSE,	0},
        {35,            _NCCC_TICKET_ETICKET_CARD_,    VS_TRUE,		0},
        {37,            _NCCC_TICKET_ISO_ASC_,         VS_FALSE,	12},
        {38,            _NCCC_TICKET_ISO_ASC_,         VS_FALSE,	6},
        {39,            _NCCC_TICKET_ISO_ASC_,         VS_FALSE,	2},
        {41,            _NCCC_TICKET_ISO_ASC_,         VS_FALSE,	8},
        {42,            _NCCC_TICKET_ISO_ASC_,         VS_FALSE,	15},
        {48,            _NCCC_TICKET_ISO_BYTE_3_,      VS_TRUE,		0},
        {56,            _NCCC_TICKET_ISO_BYTE_3_,      VS_FALSE,	0},
	{57,            _NCCC_TICKET_ISO_BYTE_3_,      VS_FALSE,	0},
        {59,            _NCCC_TICKET_ISO_BYTE_3_,      VS_TRUE,		0},
        {60,            _NCCC_TICKET_ISO_BYTE_3_,      VS_TRUE,		0},
        {62,            _NCCC_TICKET_ISO_BYTE_3_,      VS_TRUE,		0},
        {63,            _NCCC_TICKET_ISO_BYTE_3_,      VS_TRUE,		0},
	{64,            _NCCC_TICKET_ISO_BCD_,         VS_FALSE,       16},
        {0,             _NCCC_TICKET_ISO_BCD_,         VS_FALSE,	0},  /* 最後一組一定要放 0 */
};

int inTICKET_ATS_IPASS_Logon[] = {3, 11, 24, 25, 32, 37, 41, 42, 56, 59, 60, 0}; 
int inTICKET_ATS_IPASS_Register[] = {3, 11, 24, 25, 32, 37, 41, 42, 56, 59, 60, 0}; 
int inTICKET_ATS_IPASS_Query[] = {3, 4, 11, 22, 24, 25, 32, 35, 37, 41, 42, 56, 59, 60, 62, 64, 0}; 
int inTICKET_ATS_IPASS_Deduct[] = {3, 4, 11, 22, 24, 25, 32, 35, 37, 41, 42, 56, 59, 60, 62, 64, 0}; 
int inTICKET_ATS_IPASS_Refund[] = {3, 4, 11, 22, 24, 25, 32, 35, 37, 41, 42, 56, 59, 60, 62, 64, 0}; 
int inTICKET_ATS_IPASS_Void_Top_Up[] = {3, 4, 11, 22, 24, 25, 32, 35, 37, 38, 41, 42, 56, 59, 60, 62, 64, 0}; 
int inTICKET_ATS_IPASS_Auto_Top_Up[] = {3, 4, 11, 22, 24, 25, 32, 35, 37, 41, 42, 56, 59, 60, 62, 64, 0}; 
int inTICKET_ATS_IPASS_Top_Up[] = {3, 4, 11, 22, 24, 25, 32, 35, 37, 41, 42, 56, 59, 60, 62, 64, 0}; 
int inTICKET_ATS_Send_Advice[] = {3, 4, 11, 12, 13, 22, 24, 25, 32, 35, 37, 41, 42, 56, 59, 60, 62, 64, 0}; 
int inTICKET_ATS_SETTLE[] = {3, 11, 24, 32, 41, 42, 59, 60, 63, 64, 0}; 
int inTICKET_ATS_CLS_SETTLE[] = {3, 4, 11, 24, 32, 41, 42, 56, 59, 60, 63, 64, 0}; 
int inTICKET_ATS_REVERSAL[] = {3, 4, 11, 22, 24, 25, 32, 35, 37, 41, 42, 56, 59, 60, 62, 0}; 

int inTICKET_ATS_EASYCARD_Logon[] = {3, 11, 24, 25, 32, 37, 41, 42, 56, 59, 60, 0}; 
int inTICKET_ATS_EASYCARD_Register[] = {3, 11, 24, 25, 32, 37, 41, 42, 56, 59, 60, 0}; 
int inTICKET_ATS_EASYCARD_Reg_REV[] = {3, 11, 24, 25, 32, 37, 41, 42, 56, 59, 60, 0}; 
int inTICKET_ATS_EASYCARD_Deduct[] = {3, 4, 11, 22, 24, 25, 32, 35, 37, 41, 42, 56, 59, 60, 62, 64, 0}; 
int inTICKET_ATS_EASYCARD_Refund[] = {3, 4, 11, 22, 24, 25, 32, 35, 37, 41, 42, 56, 59, 60, 62, 64, 0}; 
int inTICKET_ATS_EASYCARD_Top_Up[] = {3, 4, 11, 22, 24, 25, 32, 35, 37, 41, 42, 56, 59, 60, 62, 64, 0}; 
int inTICKET_ATS_EASYCARD_Auto_Top_Up[] = {3, 4, 11, 22, 24, 25, 32, 35, 37, 41, 42, 56, 59, 60, 62, 64, 0}; 
int inTICKET_ATS_EASYCARD_Void_Top_Up[] = {3, 4, 11, 22, 24, 25, 32, 35, 37, 38, 41, 42, 56, 59, 60, 62, 64, 0};

int inTICKET_ATS_ICASH_Unlock[] = {3, 11, 24, 25, 32, 37, 41, 42, 56, 59, 60, 0};
int inTICKET_ATS_ICASH_GetPIN[] = {3, 11, 24, 25, 32, 37, 41, 42, 56, 59, 60, 0};
int inTICKET_ATS_ICASH_Logon[] = {3, 11, 24, 25, 32, 37, 41, 42, 56, 59, 60, 0};
int inTICKET_ATS_ICASH_Deduct[] = {3, 4, 11, 22, 24, 25, 32, 35, 37, 41, 42, 56, 59, 60, 62, 64, 0};
int inTICKET_ATS_ICASH_Refund[] = {3, 4, 11, 22, 24, 25, 32, 35, 37, 41, 42, 56, 59, 60, 62, 64, 0};
int inTICKET_ATS_ICASH_Top_Up[] = {3, 4, 11, 22, 24, 25, 32, 35, 37, 41, 42, 56, 59, 60, 62, 64, 0};
int inTICKET_ATS_ICASH_Auto_Top_Up[] = {3, 4, 11, 22, 24, 25, 32, 35, 37, 41, 42, 56, 59, 60, 62, 64, 0};
int inTICKET_ATS_ICASH_Void_Top_Up[] = {3, 4, 11, 22, 24, 25, 32, 35, 37, 38, 41, 42, 56, 59, 60, 62, 64, 0};

int inTICKET_MFES_IPASS_Logon[] = {3, 11, 24, 25, 37, 41, 42, 56, 59, 60, 0}; 
int inTICKET_MFES_IPASS_Register[] = {3, 11, 24, 25, 37, 41, 42, 56, 59, 60, 0}; 
int inTICKET_MFES_IPASS_Query[] = {3, 4, 11, 22, 24, 25, 35, 37, 41, 42, 56, 59, 60, 62, 0}; 
int inTICKET_MFES_IPASS_Deduct[] = {3, 4, 11, 22, 24, 25, 35, 37, 41, 42, 56, 59, 60, 62, 0}; 
int inTICKET_MFES_IPASS_Refund[] = {3, 4, 11, 22, 24, 25, 35, 37, 41, 42, 56, 59, 60, 62, 0}; 
int inTICKET_MFES_IPASS_Void_Top_Up[] = {3, 4, 11, 22, 24, 25, 35, 37, 38, 41, 42, 56, 59, 60, 62, 0}; 
int inTICKET_MFES_IPASS_Auto_Top_Up[] = {3, 4, 11, 22, 24, 25, 35, 37, 41, 42, 56, 59, 60, 62, 0}; 
int inTICKET_MFES_IPASS_Top_Up[] = {3, 4, 11, 22, 24, 25, 35, 37, 41, 42, 56, 59, 60, 62, 0}; 
int inTICKET_MFES_Send_Advice[] = {3, 4, 11, 12, 13, 22, 24, 25, 35, 37, 41, 42, 56, 59, 60, 62, 0}; 
int inTICKET_MFES_SETTLE[] = {3, 11, 24, 41, 42, 60, 63, 0}; 
int inTICKET_MFES_CLS_SETTLE[] = {3, 4, 11, 24, 41, 42, 56, 59, 60, 63, 0}; 
int inTICKET_MFES_REVERSAL[] = {3, 4, 11, 22, 24, 25, 35, 37, 41, 42, 56, 59, 60, 62, 0}; 

int inTICKET_MFES_EASYCARD_Logon[] = {3, 11, 24, 25, 37, 41, 42, 56, 59, 60, 0}; 
int inTICKET_MFES_EASYCARD_Register[] = {3, 11, 24, 25, 37, 41, 42, 56, 59, 60, 0}; 
int inTICKET_MFES_EASYCARD_Reg_REV[] = {3, 11, 24, 25, 37, 41, 42, 56, 59, 60, 0};  
int inTICKET_MFES_EASYCARD_Deduct[] = {3, 4, 11, 22, 24, 25, 35, 37, 41, 42, 56, 59, 60, 62, 0}; 
int inTICKET_MFES_EASYCARD_Refund[] = {3, 4, 11, 22, 24, 25, 35, 37, 41, 42, 56, 59, 60, 62, 0}; 
int inTICKET_MFES_EASYCARD_Top_Up[] = {3, 4, 11, 22, 24, 25, 35, 37, 41, 42, 56, 59, 60, 62, 0}; 
int inTICKET_MFES_EASYCARD_Auto_Top_Up[] = {3, 4, 11, 22, 24, 25, 35, 37, 41, 42, 56, 59, 60, 62, 0}; 
int inTICKET_MFES_EASYCARD_Void_Top_Up[] = {3, 4, 11, 22, 24, 25, 35, 37, 38, 41, 42, 56, 59, 60, 62, 0};

int inTICKET_MFES_ICASH_Unlock[] = {3, 11, 24, 25, 37, 41, 42, 56, 60, 0};
int inTICKET_MFES_ICASH_GetPIN[] = {3, 11, 24, 25, 37, 41, 42, 56, 60, 0};
int inTICKET_MFES_ICASH_Logon[] = {3, 11, 24, 25, 37, 41, 42, 56, 60, 0};
int inTICKET_MFES_ICASH_Deduct[] = {3, 4, 11, 22, 24, 25, 35, 37, 41, 42, 56, 59, 60, 62, 0};
int inTICKET_MFES_ICASH_Refund[] = {3, 4, 11, 22, 24, 25, 35, 37, 41, 42, 56, 59, 60, 62, 0};
int inTICKET_MFES_ICASH_Top_Up[] = {3, 4, 11, 22, 24, 25, 35, 37, 41, 42, 56, 59, 60, 62, 0};
int inTICKET_MFES_ICASH_Auto_Top_Up[] = {3, 4, 11, 22, 24, 25, 35, 37, 41, 42, 56, 59, 60, 62, 0};
int inTICKET_MFES_ICASH_Void_Top_Up[] = {3, 4, 11, 22, 24, 25, 35, 37, 38, 41, 42, 56, 59, 60, 62, 0};

/* 最後一組一定要放 0!! */


BIT_MAP_NCCC_TICKET_TABLE srNCCC_TICKET_ATS_ISOBitMap[] =
{
	{_TICKET_IPASS_LOGON_,		inTICKET_ATS_IPASS_Logon,		"0800",		"535000"},      /* 開機認證 */
	{_TICKET_IPASS_REGISTER_,	inTICKET_ATS_IPASS_Register,		"0800",		"535200"},      /* 開機主機註冊 */
        {_TICKET_IPASS_QUERY_,		inTICKET_ATS_IPASS_Query,		"0100",         "538000"},      /* 詢卡 */
        {_TICKET_IPASS_INQUIRY_,	inTICKET_ATS_IPASS_Query,		"0100",         "538000"},      /* 查詢餘額 */
        {_TICKET_IPASS_DEDUCT_,		inTICKET_ATS_IPASS_Deduct,		"0100",         "530000"},      /* 扣款 */
        {_TICKET_IPASS_REFUND_,		inTICKET_ATS_IPASS_Refund,		"0100",         "532000"},      /* 退貨 */
	{_TICKET_IPASS_AUTO_TOP_UP_,	inTICKET_ATS_IPASS_Auto_Top_Up,		"0100",         "531000"},      /* 自動加值 */
        {_TICKET_IPASS_TOP_UP_,		inTICKET_ATS_IPASS_Top_Up,		"0100",         "533000"},      /* 手動加值 */
        {_TICKET_IPASS_VOID_TOP_UP_,	inTICKET_ATS_IPASS_Void_Top_Up,		"0100",         "533200"},      /* 取消加值 */
        {_ADVICE_,			inTICKET_ATS_Send_Advice,		"0220",         "000000"},      /* Advice */
        {_SETTLE_,			inTICKET_ATS_SETTLE,			"0500",		"519200"},      /* Initial Settlement */
        {_CLS_SETTLE_,			inTICKET_ATS_CLS_SETTLE,		"0500",		"519600"},      /* End Settlement */
        {_REVERSAL_,			inTICKET_ATS_REVERSAL,			"0400",		"530000"},      /* Reversal */   
        
        {_TICKET_EASYCARD_LOGON_,	inTICKET_ATS_EASYCARD_Logon,		"0800",		"515000"},      /* 開機認證 */
        {_TICKET_EASYCARD_REGISTER_,	inTICKET_ATS_EASYCARD_Register,		"0800",		"515200"},      /* 開機主機註冊 */
        {_TICKET_EASYCARD_REG_REV_,	inTICKET_ATS_EASYCARD_Reg_REV,		"0400",		"515200"},      /* 註冊Reversal */   
        {_TICKET_EASYCARD_DEDUCT_,	inTICKET_ATS_EASYCARD_Deduct,		"0100",         "510000"},      /* 扣款 */
        {_TICKET_EASYCARD_REFUND_,	inTICKET_ATS_EASYCARD_Refund,		"0100",         "512000"},      /* 退貨 */
        {_TICKET_EASYCARD_TOP_UP_,	inTICKET_ATS_EASYCARD_Top_Up,		"0100",         "513000"},      /* 手動加值 */
        {_TICKET_EASYCARD_AUTO_TOP_UP_,	inTICKET_ATS_EASYCARD_Auto_Top_Up,	"0100",         "511000"},      /* 自動加值 */
        {_TICKET_EASYCARD_VOID_TOP_UP_,	inTICKET_ATS_EASYCARD_Void_Top_Up,	"0100",         "513200"},      /* 取消加值 */

	{_TICKET_ICASH_UNLOCK_,	        inTICKET_ATS_ICASH_Unlock,	        "0800",		"585200"},      /* 開機 sam unlock */
        {_TICKET_ICASH_GETPIN_,	        inTICKET_ATS_ICASH_GetPIN,	        "0800",		"585000"},      /* 開機 Get PIN */
        {_TICKET_ICASH_LOGON_,	        inTICKET_ATS_ICASH_Logon,	        "0800",		"585100"},      /* 開機 logon */
        {_TICKET_ICASH_DEDUCT_,		inTICKET_ATS_ICASH_Deduct,		"0100",         "580000"},      /* 扣款 */
        {_TICKET_ICASH_REFUND_,		inTICKET_ATS_ICASH_Refund,		"0100",         "582000"},      /* 退貨 */
        {_TICKET_ICASH_TOP_UP_,		inTICKET_ATS_ICASH_Top_Up,		"0100",         "583000"},      /* 手動加值 */
	{_TICKET_ICASH_AUTO_TOP_UP_,	inTICKET_ATS_ICASH_Auto_Top_Up,		"0100",         "581000"},      /* 自動加值 */
        {_TICKET_ICASH_VOID_TOP_UP_,	inTICKET_ATS_ICASH_Void_Top_Up,		"0100",         "583200"},      /* 取消加值 */
        
	{_NCCC_TICKET_NULL_TX_,	NULL,						"0000",		"000000"},	/* 最後一組一定要放 TMS_NULL_TX!! */
};

BIT_MAP_NCCC_TICKET_TABLE srNCCC_TICKET_MFES_ISOBitMap[] =
{
	{_TICKET_IPASS_LOGON_,		inTICKET_MFES_IPASS_Logon,		"0800",		"535000"},      /* 開機認證 */
	{_TICKET_IPASS_REGISTER_,	inTICKET_MFES_IPASS_Register,		"0800",		"535200"},      /* 開機主機註冊 */
        {_TICKET_IPASS_QUERY_,		inTICKET_MFES_IPASS_Query,		"0100",         "538000"},      /* 詢卡 */
        {_TICKET_IPASS_INQUIRY_,	inTICKET_MFES_IPASS_Query,		"0100",         "538000"},      /* 查詢餘額 */
        {_TICKET_IPASS_DEDUCT_,		inTICKET_MFES_IPASS_Deduct,		"0100",         "530000"},      /* 扣款 */
        {_TICKET_IPASS_REFUND_,		inTICKET_MFES_IPASS_Refund,		"0100",         "532000"},      /* 退貨 */
	{_TICKET_IPASS_AUTO_TOP_UP_,	inTICKET_MFES_IPASS_Auto_Top_Up,	"0100",         "531000"},      /* 自動加值 */
        {_TICKET_IPASS_TOP_UP_,		inTICKET_MFES_IPASS_Top_Up,		"0100",         "533000"},      /* 手動加值 */
        {_TICKET_IPASS_VOID_TOP_UP_,	inTICKET_MFES_IPASS_Void_Top_Up,	"0100",         "533200"},      /* 取消加值 */
        {_ADVICE_,			inTICKET_MFES_Send_Advice,		"0220",         "000000"},      /* Advice */
        {_SETTLE_,			inTICKET_MFES_SETTLE,			"0500",		"519200"},      /* Initial Settlement */
        {_CLS_SETTLE_,			inTICKET_MFES_CLS_SETTLE,		"0500",		"519600"},      /* End Settlement */
        {_REVERSAL_,			inTICKET_MFES_REVERSAL,			"0400",		"530000"},      /* Reversal */   
        
        {_TICKET_EASYCARD_LOGON_,	inTICKET_MFES_EASYCARD_Logon,		"0800",		"515000"},      /* 開機認證 */
        {_TICKET_EASYCARD_REGISTER_,	inTICKET_MFES_EASYCARD_Register,	"0800",		"515200"},      /* 開機主機註冊 */
        {_TICKET_EASYCARD_REG_REV_,	inTICKET_MFES_EASYCARD_Reg_REV,		"0400",		"515200"},      /* 註冊Reversal */   
        {_TICKET_EASYCARD_DEDUCT_,	inTICKET_MFES_EASYCARD_Deduct,		"0100",         "510000"},      /* 扣款 */
        {_TICKET_EASYCARD_REFUND_,	inTICKET_MFES_EASYCARD_Refund,		"0100",         "512000"},      /* 退貨 */
        {_TICKET_EASYCARD_TOP_UP_,	inTICKET_MFES_EASYCARD_Top_Up,		"0100",         "513000"},      /* 手動加值 */
        {_TICKET_EASYCARD_AUTO_TOP_UP_,	inTICKET_MFES_EASYCARD_Auto_Top_Up,	"0100",         "511000"},      /* 自動加值 */
        {_TICKET_EASYCARD_VOID_TOP_UP_,	inTICKET_MFES_EASYCARD_Void_Top_Up,	"0100",         "513200"},      /* 取消加值 */

	{_TICKET_ICASH_UNLOCK_,	        inTICKET_MFES_ICASH_Unlock,	        "0800",		"585200"},      /* 開機 sam unlock */
        {_TICKET_ICASH_GETPIN_,	        inTICKET_MFES_ICASH_GetPIN,	        "0800",		"585000"},      /* 開機 Get PIN */
        {_TICKET_ICASH_LOGON_,	        inTICKET_MFES_ICASH_Logon,	        "0800",		"585100"},      /* 開機 logon */
        {_TICKET_ICASH_DEDUCT_,		inTICKET_MFES_ICASH_Deduct,		"0100",         "580000"},      /* 扣款 */
        {_TICKET_ICASH_REFUND_,		inTICKET_MFES_ICASH_Refund,		"0100",         "582000"},      /* 退貨 */
        {_TICKET_ICASH_TOP_UP_,		inTICKET_MFES_ICASH_Top_Up,		"0100",         "583000"},      /* 手動加值 */
	{_TICKET_ICASH_AUTO_TOP_UP_,	inTICKET_MFES_ICASH_Auto_Top_Up,	"0100",         "581000"},      /* 自動加值 */
        {_TICKET_ICASH_VOID_TOP_UP_,	inTICKET_MFES_ICASH_Void_Top_Up,	"0100",         "583200"},      /* 取消加值 */
        
	{_NCCC_TICKET_NULL_TX_,	NULL,						"0000",		"000000"},	/* 最後一組一定要放 TMS_NULL_TX!! */
};

ISO_TYPE_NCCC_TICKET_TABLE srNCCC_TICKET_ISOFunc[] =
{
        {
		srNCCC_TICKET_ATS_ISOFieldPack,
		srNCCC_TICKET_ATS_ISOFieldUnPack,
		srNCCC_TICKET_ATS_ISOFieldCheck,
		srNCCC_TICKET_ATS_ISOFieldType,
		srNCCC_TICKET_ATS_ISOBitMap,
		inNCCC_TICKET_ISOPackMessageType,
		inNCCC_TICKET_ATS_ISOModifyBitMap,
		inNCCC_TICKET_ATS_ISOModifyPackData,
		inNCCC_TICKET_ISOCheckHeader,
		inNCCC_TICKET_ISOOnlineAnalyse,
		inNCCC_TICKET_ISOAdviceAnalyse
        },
	
	{
		srNCCC_TICKET_MFES_ISOFieldPack,
		srNCCC_TICKET_MFES_ISOFieldUnPack,
		srNCCC_TICKET_MFES_ISOFieldCheck,
		srNCCC_TICKET_MFES_ISOFieldType,
		srNCCC_TICKET_MFES_ISOBitMap,
		inNCCC_TICKET_ISOPackMessageType,
		inNCCC_TICKET_MFES_ISOModifyBitMap,
		inNCCC_TICKET_MFES_ISOModifyPackData,
		inNCCC_TICKET_ISOCheckHeader,
		inNCCC_TICKET_ISOOnlineAnalyse,
		inNCCC_TICKET_ISOAdviceAnalyse
        },
	
	{
		srNCCC_TICKET_ATS_ISOFieldPack_tSAM,
		srNCCC_TICKET_ATS_ISOFieldUnPack,
		srNCCC_TICKET_ATS_ISOFieldCheck,
		srNCCC_TICKET_ATS_ISOFieldType,
		srNCCC_TICKET_ATS_ISOBitMap,
		inNCCC_TICKET_ISOPackMessageType,
		inNCCC_TICKET_ATS_ISOModifyBitMap,
		inNCCC_TICKET_ATS_ISOModifyPackData,
		inNCCC_TICKET_ISOCheckHeader,
		inNCCC_TICKET_ISOOnlineAnalyse,
		inNCCC_TICKET_ISOAdviceAnalyse
        },
};

/*
Function        :inNCCC_TICKET_Func_SetTxnOnlineOffline
Date&Time       :2018/1/31 下午 5:15
Describe        :根據交易別決定是否Online
*/
int inNCCC_TICKET_Func_SetTxnOnlineOffline(TRANSACTION_OBJECT *pobTran)
{	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_TICKET_Func_SetTxnOnlineOffline() START !");
	}
	
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
	}
	
        /* offline交易及settle不需要做reversal */
        pobTran->uszReversalBit = VS_TRUE;

	/* _TICKET_IPASS_LOGON_ _TICKET_IPASS_REGISTER_ 都不跑SetOnlineOffline，但還是加入邏輯避免不小心組到reversal */
	/* ICASH一律不組reversal，他們用Txnlog控制 */
	if (pobTran->inTransactionCode == _SETTLE_			||
	    pobTran->inTransactionCode == _CLS_SETTLE_			||
	    pobTran->inTransactionCode == _TICKET_IPASS_LOGON_		||	
	    pobTran->inTransactionCode == _TICKET_IPASS_REGISTER_	||	
	    pobTran->srTRec.srIPASSRec.inStepNum == 2			||
	    pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
	{
		pobTran->uszReversalBit = VS_FALSE;
	}
	
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inNCCC_TICKET_Func_SetTxnOnlineOffline() END!");
        }    
        
        return (VS_SUCCESS);
}

/*
Function        :inNCCC_TICKET_Func_BuildAndSendPacket
Date&Time       :2016/9/14 下午 12:00
Describe        :處理交易流程斷線
*/
int inNCCC_TICKET_Func_BuildAndSendPacket(TRANSACTION_OBJECT *pobTran)
{
	int	i = 0;
	int	inRetVal = VS_SUCCESS;
	char	szTemplate[10 + 1] = {0};
	char	szTemplate1[10 + 1] = {0};
	char	szTemplate2[10 + 1] = {0};
	char	szDemoMode[2 + 1] = {0};
	
	/* 教育訓練模式 */
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memset(szTemplate2, 0x00, sizeof(szTemplate2));

		inGetBatchNum(szTemplate1);
		inGetInvoiceNum(szTemplate2);

		sprintf(szTemplate, "%03d%03d", atoi(szTemplate1), atoi(szTemplate2));
		strcpy(pobTran->srTRec.szTicketRefundCode, szTemplate);

		for (i = 0; i < 2; i++)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_SEND_, 0, _COORDINATE_Y_LINE_8_7_);/* 傳送中... */
		}

		for (i = 0; i < 1; i++)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_PROCESS_, 0, _COORDINATE_Y_LINE_8_7_);/* 處理中... */
		}

		for (i = 0; i < 1; i++)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_RECEIVE_, 0, _COORDINATE_Y_LINE_8_7_);/* 接收中... */
		}	

		for (i = 0; i < 1; i++)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_PROCESS_, 0, _COORDINATE_Y_LINE_8_7_);/* 處理中... */
		}
		
		/* DEMO版一卡通且超過100元要出優惠 */
		if (pobTran->srTRec.lnTxnAmount >= 100 &&  pobTran->srTRec.inCode == _TICKET_IPASS_DEDUCT_ &&
		    inNCCC_Loyalty_ASM_Flag() == VS_SUCCESS)
		{
			memset(pobTran->srTRec.szAwardNum, 0x00, sizeof(pobTran->srTRec.szAwardNum));
			memcpy(pobTran->srTRec.szAwardNum, "1", 1);
			pobTran->srTRec.uszRewardL1Bit = VS_TRUE;
			pobTran->srTRec.uszRewardL2Bit = VS_TRUE;
			pobTran->srTRec.uszRewardL5Bit = VS_TRUE;
		}
		
		inRetVal = VS_SUCCESS;
                
                memset(pobTran->srTRec.szAuthCode, 0x00, sizeof(pobTran->srTRec.szAuthCode)); 
		memcpy(&pobTran->srTRec.szAuthCode[0], "MCD123", 6);  

		return (inRetVal);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inNCCC_TICKET_Func_BuildAndSendPacket() START!");
		}
		
		if (ginFindRunTime == VS_TRUE)
		{
			inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
		}
	

		inRetVal = inNCCC_TICKET_BuildAndSendPacket(pobTran);
		if (inRetVal != VS_SUCCESS)
		{
			inFunc_Display_Error(pobTran);
			inNCCC_Ticket_DisConnect(pobTran);
		}

		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inNCCC_TICKET_Func_BuildAndSendPacket() END !");
			inLogPrintf(AT, "----------------------------------------");
		}
	
		return (inRetVal);
	}
}

/*
Function        :inNCCC_TICKET_BuildAndSendPacket
Date&Time       :2018/1/3 下午 12:00
Describe        :真正的Bulid and Send
*/
int inNCCC_TICKET_BuildAndSendPacket(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	char	szCLS_SettleBit[2 + 1];
	
        /* 步驟 1 Get System Trans Number */
        if (inNCCC_TICKET_GetSTAN(pobTran) == VS_ERROR)
                return (VS_ERROR);
	
        /* 步驟 1.1 處理online交易 */
        if (pobTran->srTRec.uszOfflineBit == VS_FALSE)
        {		
                /* 步驟 1.4 */
                if (pobTran->inTransactionCode == _SETTLE_)
                {
			/* 處理reversal */
			/* IPASS */
			pobTran->srTRec.inTDTIndex = _TDT_INDEX_00_IPASS_;
			inLoadTDTRec(pobTran->srTRec.inTDTIndex);
			if ((inRetVal = inNCCC_TICKET_ProcessReversal(pobTran)) != VS_SUCCESS)
			{
				return (inRetVal);
			}
			
			/* ICASH */
			pobTran->srTRec.inTDTIndex = _TDT_INDEX_02_ICASH_;
			inLoadTDTRec(pobTran->srTRec.inTDTIndex);
			if ((inRetVal = inNCCC_TICKET_ProcessReversal(pobTran)) != VS_SUCCESS)
			{
				return (inRetVal);
			}
			
			/* 處理advice */
			/* IPASS */
			pobTran->srTRec.inTDTIndex = _TDT_INDEX_00_IPASS_;
			inLoadTDTRec(pobTran->srTRec.inTDTIndex);
			if ((inRetVal = inNCCC_TICKET_ProcessAdvice(pobTran)) != VS_SUCCESS)
			{
				/* 只有結帳時前送advice錯誤時要顯示錯誤訊息，在inNCCC_TICKET_ISOAdviceAnalyse裡顯示錯誤訊息 */
				pobTran->inErrorMsg = _ERROR_CODE_V3_SETTLE_NOT_SUCCESS_;
				return (inRetVal);
			}
			
			/* ICASH */
			pobTran->srTRec.inTDTIndex = _TDT_INDEX_02_ICASH_;
			inLoadTDTRec(pobTran->srTRec.inTDTIndex);
			if ((inRetVal = inNCCC_TICKET_ProcessAdvice(pobTran)) != VS_SUCCESS)
			{
				/* 只有結帳時前送advice錯誤時要顯示錯誤訊息，在inNCCC_TICKET_ISOAdviceAnalyse裡顯示錯誤訊息 */
				pobTran->inErrorMsg = _ERROR_CODE_V3_SETTLE_NOT_SUCCESS_;
				return (inRetVal);
			}
				
//			if ((inRetVal = inNCCC_TICKET_ProcessAdvice_ESC(pobTran)) != VS_SUCCESS)
//			{
//				/* 只有結帳時前送advice錯誤時要顯示錯誤訊息，在inNCCC_TICKET_ISOAdviceAnalyse裡顯示錯誤訊息 */
//				pobTran->inErrorMsg = _ERROR_CODE_V3_SETTLE_NOT_SUCCESS_;
//				return (inRetVal);
//			}
			
			memset(szCLS_SettleBit, 0x00, sizeof(szCLS_SettleBit));
			inGetCLS_SettleBit(szCLS_SettleBit);
			
			if (memcmp(szCLS_SettleBit, "Y", strlen("Y")) == 0)
			{
				pobTran->inTransactionCode = _CLS_SETTLE_;
				pobTran->srBRec.inCode = _CLS_SETTLE_;
			}
			else
			{
				pobTran->inTransactionCode = _SETTLE_;
				pobTran->srBRec.inCode = _SETTLE_;
			}
			
                }
		else
		{
			/* 先送上筆該卡別交易失敗的 Reversal 及產生當筆交易 Reversal */
			if ((inRetVal = inNCCC_TICKET_ProcessReversal(pobTran)) != VS_SUCCESS)
			{
				return (inRetVal);
			}
		}
		
		/* 處理【ONLINE】交易 */
                if (pobTran->inTransactionCode == _SETTLE_ || pobTran->inTransactionCode == _CLS_SETTLE_)
                {
			inRetVal = inNCCC_TICKET_ProcessOnline(pobTran);
			inNCCC_TICKET_SetSTAN(pobTran); /* 成功或失敗 System_Trace_Number 都要加一 */
			
                        if (inRetVal != VS_SUCCESS)
        		{
                                pobTran->inTransactionResult = _TRAN_RESULT_COMM_ERROR_;
				
        			return (inRetVal); 
        		}
			
			inRetVal = inNCCC_TICKET_AnalysePacket(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				inUtility_StoreTraceLog_OneStep("inNCCC_TICKET_AnalysePacket Error");

				return (inRetVal);
			}
        		
                }        
                else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
                {        
                        if (pobTran->srTRec.srIPASSRec.inStepNum == 1)
                        {        
                		inRetVal = inNCCC_TICKET_ProcessOnline(pobTran);
				inNCCC_TICKET_SetSTAN(pobTran); /* 成功或失敗 System_Trace_Number 都要加一 */

				if (inRetVal != VS_SUCCESS)
				{
					pobTran->inTransactionResult = _TRAN_RESULT_COMM_ERROR_;

					return (inRetVal); 
				}
        	        }	
        		else
        		{
        		        /* 一次送完所有的advice */
        		        inNCCC_TICKET_ProcessAdvice(pobTran);
        	        }  
        	}
		else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
                {        
                        if (pobTran->srTRec.srICASHRec.inStepNum == 1	||
			    pobTran->srTRec.srICASHRec.inStepNum == 2)
                        {        
                		inRetVal = inNCCC_TICKET_ProcessOnline(pobTran);
				inNCCC_TICKET_SetSTAN(pobTran); /* 成功或失敗 System_Trace_Number 都要加一 */

				if (inRetVal != VS_SUCCESS)
				{
					pobTran->inTransactionResult = _TRAN_RESULT_COMM_ERROR_;

					return (inRetVal); 
				}
        	        }	
        		else
        		{
        		        /* 一次送完所有的advice */
        		        inNCCC_TICKET_ProcessAdvice(pobTran);
        	        }  
        	}
        }
	else
        {
               /* 不處理offline */
        }
	
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_BuildAndSendPacket() END!");
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_TICKET_ATS_Pack03
Date&Time       :2018/1/4 下午 4:35
Describe        :Field_3:       Processing Code
*/
int inNCCC_TICKET_ATS_Pack03(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 	inCnt = 0;
	char	szDebugMsg[100 + 1];
	char	szAscii[6 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_ATS_Pack03() START!");
	
	
	memcpy(&uszPackBuf[inCnt], &guszNCCC_TICKET_ISO_Field03[0], _NCCC_TICKET_PCODE_SIZE_);
	
        inCnt += _NCCC_TICKET_PCODE_SIZE_;
	
	/* for Mac */
	inFunc_BCD_to_ASCII(&gszMAC_F_03[0], &guszNCCC_TICKET_ISO_Field03[1], 1);
	
	if (ginDebug == VS_TRUE)
	{
		memset(szAscii, 0x00, sizeof(szAscii));
		inFunc_BCD_to_ASCII(szAscii, guszNCCC_TICKET_ISO_Field03, _NCCC_TICKET_PCODE_SIZE_);
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "F_03 [P_CODE %s]", szAscii);
		inLogPrintf(AT, szDebugMsg);
	}
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_ATS_Pack03() END!");
	
        return (inCnt);
}

/*
Function        :inNCCC_TICKET_MFES_Pack03
Date&Time       :2018/1/4 下午 4:35
Describe        :Field_3:       Processing Code
*/
int inNCCC_TICKET_MFES_Pack03(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 	inCnt = 0;
	char	szDebugMsg[100 + 1];
	char	szAscii[6 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_MFES_Pack03() START!");
	
	
	memcpy(&uszPackBuf[inCnt], &guszNCCC_TICKET_ISO_Field03[0], _NCCC_TICKET_PCODE_SIZE_);
	
        inCnt += _NCCC_TICKET_PCODE_SIZE_;
	
	/* for Mac */
	inFunc_BCD_to_ASCII(&gszMAC_F_03[0], &guszNCCC_TICKET_ISO_Field03[1], 1);
	
	if (ginDebug == VS_TRUE)
	{
		memset(szAscii, 0x00, sizeof(szAscii));
		inFunc_BCD_to_ASCII(szAscii, guszNCCC_TICKET_ISO_Field03, _NCCC_TICKET_PCODE_SIZE_);
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "F_03 [P_CODE %s]", szAscii);
		inLogPrintf(AT, szDebugMsg);
	}
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_MFES_Pack03() END!");
	
        return (inCnt);
}

/*
Function        :inNCCC_TICKET_Pack04
Date&Time       :
Describe        :Field_4:       Amount, Transaction
*/
int inNCCC_TICKET_Pack04(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 		inCnt = 0;
        char 		szTemplate[100 + 1], szDebugMsg[100 + 1];
	char		szAscii[12 + 1];
        unsigned char 	uszBCD[6 + 1];

	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_Pack04() START!");
	
	
        memset(szTemplate, 0x00, sizeof(szTemplate));
        switch (pobTran->srTRec.inCode)
	{
		case _TICKET_IPASS_AUTO_TOP_UP_ :
		case _TICKET_ICASH_AUTO_TOP_UP_ :
			sprintf(szTemplate, "%010ld00", pobTran->srTRec.lnTotalTopUpAmount);
			break;
		case _TICKET_IPASS_QUERY_ :
		case _TICKET_IPASS_INQUIRY_ :		/* 兩段式電票一卡通查餘額時要帶0*/
		case _TICKET_EASYCARD_DEDUCT_ :
		case _TICKET_EASYCARD_REFUND_ :
		case _TICKET_EASYCARD_TOP_UP_ :
		case _TICKET_EASYCARD_AUTO_TOP_UP_ :    
		case _TICKET_EASYCARD_VOID_TOP_UP_ :            
		        sprintf(szTemplate, "%010ld00", 0L);   
			break;	
		default :
			sprintf(szTemplate, "%010ld00", pobTran->srTRec.lnTxnAmount);
			break;
	}
	
	if (pobTran->inISOTxnCode == _ADVICE_)
	{
                if (pobTran->srTRec.uszBlackListBit == VS_TRUE || pobTran->srTRec.uszCloseAutoTopUpBit == VS_TRUE)  
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "%010ld00", 0L);  
                } 
	} 

        memset(uszBCD, 0x00, sizeof(uszBCD));
        inFunc_ASCII_to_BCD(&uszBCD[0], &szTemplate[0], 6);
        memcpy((char *) &uszPackBuf[inCnt], (char *) &uszBCD[0], 6);
        inCnt += 6;

	/* 壓Mac使用 */
	memset(gszMAC_F_04, 0x00, sizeof(gszMAC_F_04));
	memcpy(&gszMAC_F_04[0], szTemplate, 12);
	
	if (ginDebug == VS_TRUE)
	{
		memset(szAscii, 0x00, sizeof(szAscii));
		inFunc_BCD_to_ASCII(szAscii, uszBCD, 6);
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "F_04 [AMT %s]", szAscii);
		inLogPrintf(AT, szDebugMsg);
	}
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_Pack04() END!");
	
        return (inCnt);
}

/*
Function        :inNCCC_TICKET_Pack11
Date&Time       :
Describe        :Field_11:      System Trace Audit Number
*/
int inNCCC_TICKET_Pack11(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 		inCnt = 0;
        char 		szSTAN[6 + 1];
	char		szDebugMsg[100 + 1];
	char		szAscii[6 + 1];
        unsigned char 	uszBCD[3 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_Pack11() START!");
	

        memset(szSTAN, 0x00, sizeof(szSTAN));
	/* STAN */
        if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
                sprintf(szSTAN, "%06ld", 0L);   
        else        
		sprintf(szSTAN, "%06ld", pobTran->srTRec.lnSTAN);

        memset(uszBCD, 0x00, sizeof(uszBCD));
        inFunc_ASCII_to_BCD(&uszBCD[0], &szSTAN[0], 3);
        memcpy((char *) &uszPackBuf[inCnt], (char *) &uszBCD[0], 3);
        inCnt += 3;

	/* 壓Mac使用 */
	memset(gszMAC_F_11, 0x00, sizeof(gszMAC_F_11));
	memcpy(&gszMAC_F_11[0], &szSTAN[4], 2);
	
	if (ginDebug == VS_TRUE)
	{
		memset(szAscii, 0x00, sizeof(szAscii));
		inFunc_BCD_to_ASCII(szAscii, uszBCD, 3);
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "F_011 [STAN %s]", szAscii);
		inLogPrintf(AT, szDebugMsg);
	}
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_Pack11() END!");
	
        return (inCnt);
}

/*
Function        :inNCCC_TICKET_Pack12
Date&Time       :
Describe        :Field_12:      Time, Local Transaction
*/
int inNCCC_TICKET_Pack12(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 		inCnt = 0;
	char		szDebugMsg[100 + 1];
	char		szAscii[6 + 1];
        unsigned char 	uszBCD[3 + 1];
	RTC_NEXSYS	srRTC = {0};
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_Pack12() START!");
	
	if (pobTran->inISOTxnCode == _REVERSAL_ || pobTran->inISOTxnCode == _ADVICE_ ||
	    pobTran->inISOTxnCode == _TICKET_EASYCARD_REG_REV_)
	{
		memset(&srRTC, 0x00, sizeof(srRTC));
		inFunc_GetSystemDateAndTime(&srRTC);
	        
	        memset(pobTran->srTRec.szDate, 0x00, sizeof(pobTran->srTRec.szDate));
                memset(pobTran->srTRec.szTime, 0x00, sizeof(pobTran->srTRec.szTime));
		sprintf(pobTran->srTRec.szDate, "%02d%02d%02d",  srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
		sprintf(pobTran->srTRec.szTime, "%02d%02d%02d",  srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
	}   

        memset(uszBCD, 0x00, sizeof(uszBCD));
        inFunc_ASCII_to_BCD(&uszBCD[0], &pobTran->srTRec.szTime[0], 3);
        memcpy((char *) &uszPackBuf[inCnt], (char *) &uszBCD[0], 3);
        inCnt += 3;
	
	if (ginDebug == VS_TRUE)
	{
		memset(szAscii, 0x00, sizeof(szAscii));
		inFunc_BCD_to_ASCII(szAscii, uszBCD, 3);
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "F_012 [TIME %s]", szAscii);
		inLogPrintf(AT, szDebugMsg);
	}

	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_Pack12() END!");
	
        return (inCnt);
}

/*
Function        :inNCCC_TICKET_Pack13
Date&Time       :
Describe        :Field_13:      Date, Local Transaction
*/
int inNCCC_TICKET_Pack13(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 		inCnt = 0;
	char		szDebugMsg[100 + 1];
	char		szAscii[4 + 1];
        unsigned char 	uszBCD[2 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_Pack13() START!");

        memset(uszBCD, 0x00, sizeof(uszBCD));
        inFunc_ASCII_to_BCD(&uszBCD[0], &pobTran->srTRec.szDate[2], 2);
        memcpy((char *) &uszPackBuf[inCnt], (char *) &uszBCD[0], 2);
        inCnt += 2;
	
	if (ginDebug == VS_TRUE)
	{
		memset(szAscii, 0x00, sizeof(szAscii));
		inFunc_BCD_to_ASCII(szAscii, uszBCD, 2);
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "F_013 [DATE %s]", szAscii);
		inLogPrintf(AT, szDebugMsg);
	}
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_Pack13() END!");

        return (inCnt);
}

/*
Function        :inNCCC_TICKET_Pack22
Date&Time       :2018/1/4 下午 5:59
Describe        :Field_22:   Point of Service Entry Mode
*/
int inNCCC_TICKET_Pack22(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int             inCnt = 0;
	char		szDebugMsg[100 + 1];			
	char		szAscii[4 + 1];				/* 放Debug中轉出來的Ascii用 */
        char            szPOSmode[4 + 1];			/* Field 22的Ascii值 */
        unsigned char   uszBCD[2 + 1];				/* Field 22的Hex值 */
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_Pack22() START!");
	
        /*
         Position 1       |     PAN Entry Mode
        ------------------------------------------------------------
             5            |     EMV compliance terminal
        -------------------------------------------------------------
         Position 2 & 3   |     PAN Entry Mode
        ------------------------------------------------------------
             01           |     PAN entered manually
        ------------------------------------------------------------
             02           |     PAN auto-entry via magnetic stripe
		          |     Fallback
                          |     (Smart Pay不支援磁條卡或Fallback)
        ------------------------------------------------------------
             05           |     PAN auto-entry via chip
        ------------------------------------------------------------
             07           |     PAN auto-entry via contactless;
                          |     MasterCard PayPass Chip
                          |     VISA Paywave 1
                          |     VISA Paywave 3
                          |     JCB J/Speedy
			  |	07AMEX Expresspay = 07
                          |     (Smart Pay Contactless)
        -------------------------------------------------------------
             91           |     PAN auto-entry via contactless
                          |     PayPass Magnetic Stripe only
         ------------------------------------------------------------
         Position 4       |     PAN Entry Mode
        -------------------------------------------------------------
             1            |     With PIN entry capability
        -------------------------------------------------------------
             2            |     Without PIN entry capability
        -------------------------------------------------------------
         */
        memset(uszBCD, 0x00, sizeof(uszBCD));
        memset(szPOSmode, 0x00, sizeof(szPOSmode));

	/* Position 1 */
	strcpy(szPOSmode, "5");

	/* Position 2 & 3 */
	/* 除了特例外，只要contactless就是 07 */
	strcat(szPOSmode, "07");

	/* Position 4 */
	strcat(szPOSmode, "2");

	inFunc_ASCII_to_BCD(&uszBCD[0], &szPOSmode[0], 2);
        memcpy((char *) &uszPackBuf[inCnt], (char *) &uszBCD[0], 2);
        inCnt += 2;
	
	if (ginDebug == VS_TRUE)
	{
		memset(szAscii, 0x00, sizeof(szAscii));
		inFunc_BCD_to_ASCII(szAscii, uszBCD, 2);
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "F_022 [POSMode %s]", szAscii);
		inLogPrintf(AT, szDebugMsg);
	}

	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_Pack22() END!");
	
        return (inCnt);
}

/*
Function        :inNCCC_TICKET_Pack24
Date&Time       :2018/1/4 下午 5:59
Describe        :Field_24:      Network International Identifier (NII)
*/
int inNCCC_TICKET_Pack24(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 		inCnt = 0;
        char 		szTemplate[100 + 1];
	char		szDebugMsg[100 + 1];
	char		szAscii[4 + 1];
        unsigned char 	uszBCD[2 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_Pack24() START!");
	

        memset(szTemplate, 0x00, sizeof(szTemplate));
        if (inGetNII(&szTemplate[1]) == VS_ERROR)
                return (VS_ERROR);

        /* NII 只有三碼所以前面補0 */
        szTemplate[0] = '0' ;

        memset(uszBCD, 0x00, sizeof(uszBCD));
        inFunc_ASCII_to_BCD(&uszBCD[0], &szTemplate[0], 2);

        memcpy((char *) &uszPackBuf[inCnt], (char *) &uszBCD[0], 2);
        inCnt += 2;
	
	if (ginDebug == VS_TRUE)
	{
		memset(szAscii, 0x00, sizeof(szAscii));
		inFunc_BCD_to_ASCII(szAscii, uszBCD, 2);
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "F_024 [NII %s]", szAscii);
		inLogPrintf(AT, szDebugMsg);
	}
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_Pack24() END!");

        return (inCnt);
}

/*
Function        :inNCCC_TICKET_Pack25
Date&Time       :2018/1/4 下午 6:01
Describe        :Field_25:   Point of Service Condition Code
*/
int inNCCC_TICKET_Pack25(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 		inCnt = 0;
	char		szTemplate[100 + 1];
	char		szDebugMsg[100 + 1];
	char		szAscii[2 + 1];
        unsigned char 	uszBCD[1 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_Pack25() START!");

        /*
                    Code        |       Meaning
                ---------------------------------------------------
                    00          |       Normal presentment
                                |       Smart Pay所有交易
         */
	/* 初值設為00 */
	inFunc_ASCII_to_BCD(&uszPackBuf[inCnt], "00", 1);
	
	/* 為了印debug訊息，所以都先copy到template內，最後再轉bcd */
        memset(uszBCD, 0x00, sizeof(uszBCD));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	
	memcpy(szTemplate, "00", 2);
	
	inFunc_ASCII_to_BCD(uszBCD, szTemplate, 1);
        memcpy((char *) &uszPackBuf[inCnt], (char *) &uszBCD[0], 1);
        inCnt++;
	
	if (ginDebug == VS_TRUE)
	{
		memset(szAscii, 0x00, sizeof(szAscii));
		inFunc_BCD_to_ASCII(szAscii, uszBCD, 1);
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "F_025 [POSCondition code %s]", szAscii);
		inLogPrintf(AT, szDebugMsg);
	}
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_Pack25() END!");

        return (inCnt);
}

/*
Function        :inNCCC_TICKET_Pack32
Date&Time       :2018/1/4 下午 6:01
Describe        :Field_32:    Acquiring Institution Identification Code(金融代碼)
*/
int inNCCC_TICKET_Pack32(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int	inCnt = 0;
	char	szAscii[6 + 1];
	char	szDebugMsg[100 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_Pack32() START!");

        /*
                                補充說明：
                1. 本欄位請依收單銀行之金融機構代碼上傳(3碼)。
                2. 範例：金融代碼=956
                        Length (BCD)+ Data(BCD)= 0x02 0x09 0x56
                3. 吳升文 E-Mail Reply : 2014-04-03 (週四) AM 09:44
                        Field_32是變動長度，所以定義最多11 Betes，但是TICKET系統只用2 Bytes (3 Nibble)。所以端末機開發可以Hard Code填入範例的值即可。
         */
	memcpy((char *)&uszPackBuf[inCnt], "\x02\x09\x56", 3);
	inCnt += 3;
	
	if (ginDebug == VS_TRUE)
	{
		memset(szAscii, 0x00, sizeof(szAscii));
		inFunc_BCD_to_ASCII(szAscii, (unsigned char*)"\x02\x09\x56", 3);
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "F_032 [AIIC %s %s]", szAscii, "hard code");
		inLogPrintf(AT, szDebugMsg);
	}
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_Pack32() END!");
	
        return (inCnt);
}

/*
Function        :inNCCC_TICKET_Pack35
Date&Time       :2018/1/4 下午 6:18
Describe        :Field_35:   電票卡號
*/
int inNCCC_TICKET_Pack35(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        /* Field_35:	電票卡號 */
	int	i = 0;
	int	inUIDLen = 0;           
	int 	inCnt = 0;
	char    szTemplate[50 + 1];
	char	szDebugMsg[100 + 1];
	char	szUID[50 + 1] = {0};
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_Pack35() START!");

        if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_	||
	    pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
        {
		memset(szUID, 0x00, sizeof(szUID));
		strcpy(szUID, pobTran->srTRec.szUID);
		
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inUIDLen = strlen(szUID);
		sprintf(szTemplate, "%02d", inUIDLen);
		inFunc_ASCII_to_BCD(&uszPackBuf[inCnt], &szTemplate[0], 1);
		inCnt ++;
	}
	else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
	{
		memset(szUID, 0x00, sizeof(szUID));
		inUIDLen = 34;
		memcpy(szUID, "                                  ", inUIDLen);
		
	        uszPackBuf[inCnt] = 0x34;
		inCnt ++;
	}
	
	memcpy((char *)&uszPackBuf[inCnt], szUID, inUIDLen);
	inCnt += inUIDLen;
	
	/* 組MAC使用 F_35前22碼ASCII 不足22碼左靠右補 0*/
	memset(gszMAC_F_35, 0x00, sizeof(gszMAC_F_35));
	memcpy(&gszMAC_F_35[0], szUID, 22);
	for (i = 0; i < 22; i++)
	{
		if (gszMAC_F_35[i] == 0x00)
		{
			gszMAC_F_35[i] = '0';
		}
	}
		
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "F_35 [%d %s]", inUIDLen, szUID);
		inLogPrintf(AT, szDebugMsg);
	}
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_Pack35() END!");

        return (inCnt);
}

/*
Function        :inNCCC_TICKET_Pack35_tSAM
Date&Time       :2020/8/21 下午 5:17
Describe        :Field_35:   電票卡號
*/
int inNCCC_TICKET_Pack35_tSAM(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        /* Field_35:	電票卡號 */
	int		i = 0;
	int		inUIDLen = 0;
	int		inCnt = 0;
	int		inEncryptLen = 0;
	int		inRetVal = VS_SUCCESS;
	int		inRetVal_SelectAID = VS_SUCCESS;
	char		szTemplate[50 + 1] = {0};
	char		szDebugMsg[100 + 1] = {0};
	char		szUID[50 + 1] = {0};
	char		szField37[8 + 1] = {0}, szField41[4 + 1] = {0};
	char		szAscii[100 + 1] = {0};
	unsigned char	uszNCCCSlot = 0;
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_Pack35_tSAM() START!");

        if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_	||
	    pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
        {
		memset(szUID, 0x00, sizeof(szUID));
		strcpy(szUID, pobTran->srTRec.szUID);
		
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inUIDLen = strlen(szUID);
		sprintf(szTemplate, "%02d", inUIDLen);
		inFunc_ASCII_to_BCD(&uszPackBuf[inCnt], &szTemplate[0], 1);
		inCnt ++;
	}
	else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
	{
		memset(szUID, 0x00, sizeof(szUID));
		inUIDLen = 34;
		memcpy(szUID, "                                  ", inUIDLen);
		
	        uszPackBuf[inCnt] = 0x34;
		inCnt ++;
	}
	
	/* 加密 Start */
	/* 抓tSAM Slot */
	inRetVal = inNCCC_tSAM_Decide_tSAM_Slot(&uszNCCCSlot);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inNCCC_tSAM_Decide_tSAM_Slot Failed");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	/* 取加密長度，
	 * 1. F_35 Data Length= 01~ 15 Bytes ，取前 8 Bytes 加密。
	 * 2. F_35 Data Length= 16~ 23 Bytes ，取前 16 Bytes 加密。
	 * 3. F_35 Data Length= 24~ 31 Bytes ，取前 24 Bytes 加密。
	 * 4. F_35 Data Length= 32~ 39 Bytes ，取前 32 Bytes 加密。
	 * 5. F_35 Data Length= 40~ 57 Bytes ，取前 40 Bytes 加密。
	 *  */
	if (inUIDLen >= 1 && inUIDLen <= 15)
	{
		inEncryptLen = 8;
	}
	else if (inUIDLen >= 16 && inUIDLen <= 23)
	{
		inEncryptLen = 16;
	}
	else if (inUIDLen >= 24 && inUIDLen <= 31)
	{
		inEncryptLen = 24;
	}
	else if (inUIDLen >= 32 && inUIDLen <= 39)
	{
		inEncryptLen = 32;
	}
	else if (inUIDLen >= 40 && inUIDLen <= 57)
	{
		inEncryptLen = 40;
	}
	
	/* 組 Field_37 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(szTemplate, pobTran->srTRec.szRefNo, 11);
	inFunc_PAD_ASCII(szTemplate, szTemplate, 'F', 16, _PADDING_RIGHT_); /* 補【F】 */
	memset(szField37, 0x00, sizeof(szField37));
	inFunc_ASCII_to_BCD((unsigned char*)szField37, szTemplate, 8);
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "F_37:\"%s\"", szTemplate);
	}
	/* 組 Field_41 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTerminalID(szTemplate);
	memset(szField41, 0x00, sizeof(szField41));
	inFunc_ASCII_to_BCD((unsigned char*)szField41, szTemplate, 4);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "F_41:\"%s\"", szTemplate);
		inLogPrintf(AT, "Before Encrypt(%d):\"%s\"", inEncryptLen, szUID);
	}
	
	/* 組MAC使用 F_35前22碼ASCII 不足22碼左靠右補 0*/
	memset(gszMAC_F_35, 0x00, sizeof(gszMAC_F_35));
	memcpy(&gszMAC_F_35[0], szUID, 22);
	for (i = 0; i < 22; i++)
	{
		if (gszMAC_F_35[i] == 0x00)
		{
			gszMAC_F_35[i] = '0';
		}
	}
	
	/* 切回NCCC */
	inNCCC_tSAM_SelectAID_NCCC(uszNCCCSlot);
	
	/* 加密 */
	inRetVal = inNCCC_tSAM_Encrypt_ESVC(uszNCCCSlot,
				 inEncryptLen,
				 szUID,
				 (unsigned char*)&szField37,
				 (unsigned char*)&szField41,
				 &gusztSAMKeyIndex,
				 gusztSAMCheckSum_35);
	/* 切回票證AID */
	if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
	{
		inRetVal_SelectAID = inNCCC_tSAM_SelectAID_IPASS_Flow();
	}
	else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
	{
		inRetVal_SelectAID = inNCCC_tSAM_SelectAID_ECC_Flow();
	}
	else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
	{
		inRetVal_SelectAID = inNCCC_tSAM_SelectAID_ICASH_Flow();
	}
	else
	{
		inRetVal_SelectAID = VS_ERROR;
	}
	
	if (inRetVal_SelectAID != VS_SUCCESS)
	{
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, _ERR_SAM_ENCRYPTED_FAIL_);
		srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
		srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
		srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
		strcpy(srDispMsgObj.szErrMsg1, "無法切回票證AID");
		srDispMsgObj.inErrMsg1Line = _LINE_8_5_;
		srDispMsgObj.inBeepTimes = 1;
		srDispMsgObj.inBeepInterval = 0;
		inDISP_Msg_BMP(&srDispMsgObj);

		return (VS_ERROR);
	}
	
	
	if (inRetVal == VS_ERROR)
	{
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, _ERR_SAM_ENCRYPTED_FAIL_);
		srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
		srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
		srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
		strcpy(srDispMsgObj.szErrMsg1, "");
		srDispMsgObj.inErrMsg1Line = 0;
		srDispMsgObj.inBeepTimes = 1;
		srDispMsgObj.inBeepInterval = 0;
		inDISP_Msg_BMP(&srDispMsgObj);

		return (VS_ERROR);
	}
	else
	{
		guszField_35 = VS_TRUE;
	}
	
	memcpy((char *)&uszPackBuf[inCnt], szUID, inUIDLen);
	inCnt += inUIDLen;
		
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		memset(szAscii, 0x00, sizeof(szAscii));
		inFunc_BCD_to_ASCII(szAscii, (unsigned char*)szUID, inUIDLen);
		sprintf(szDebugMsg, "F_35 [%d %s]", inUIDLen, szAscii);
		inLogPrintf(AT, szDebugMsg);
	}
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_Pack35_tSAM() END!");

        return (inCnt);
}

/*
Function        :inNCCC_TICKET_Pack37
Date&Time       :2018/1/4 下午 6:19
Describe        :Field_37:	Retrieval Reference Number (RRN)
*/
int inNCCC_TICKET_Pack37(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 	inCnt = 0;
	char	szDebugMsg[100 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_Pack37() START!");
	
        memcpy((char *) &uszPackBuf[inCnt], pobTran->srTRec.szRefNo, 12);
        inCnt += 12;

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "F_37 [RRN %s]", pobTran->srTRec.szRefNo);
		inLogPrintf(AT, szDebugMsg);
	}
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_Pack37() END!");
	
        return (inCnt);
}

/*
Function        :inNCCC_TICKET_Pack38
Date&Time       :2018/1/4 下午 6:20
Describe        :Field_38:	Authorization Identification Response（SmartPay交易無授權碼）
*/
int inNCCC_TICKET_Pack38(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 	inCnt = 0;
        char    szAuthCode[_AUTH_CODE_SIZE_ + 1];
	char	szDebugMsg[100 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_Pack38() START!");

        memset(szAuthCode, 0x00, sizeof(szAuthCode));
	strcpy(szAuthCode, pobTran->srTRec.szAuthCode);
	/* 若沒輸入滿6個字元，則右邊用空白填滿*/
        inFunc_PAD_ASCII(szAuthCode, szAuthCode, ' ', 6, _PADDING_RIGHT_);

        memcpy((char *) &uszPackBuf[inCnt], &szAuthCode[0], _AUTH_CODE_SIZE_);
        inCnt += _AUTH_CODE_SIZE_;

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "F_38 [AUTH CODE %s]", pobTran->srTRec.szAuthCode);
		inLogPrintf(AT, szDebugMsg);
	}
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_Pack38() END!");
	
        return (inCnt);
}

/*
Function        :inNCCC_TICKET_Pack41
Date&Time       :2018/1/4 下午 6:21
Describe        :Field_41:	Card Acceptor Terminal Identification（TID）補充說明：端末機代號需可支援英、數字。
*/
int inNCCC_TICKET_Pack41(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 	inCnt = 0;
        char 	szTemplate[8 + 1];
	char	szDebugMsg[100 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_Pack41() START!");
	

        memset(szTemplate, 0x00, sizeof(szTemplate));
        if (inGetTerminalID(szTemplate) == VS_ERROR)
                return (VS_ERROR);
	
        memcpy((char *) &uszPackBuf[inCnt], szTemplate, strlen(szTemplate));
        inCnt += strlen(szTemplate);
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "F_41 [TID %s]", szTemplate);
		inLogPrintf(AT, szDebugMsg);
	}
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_Pack41() END!");

        return (inCnt);
}

/*
Function        :inNCCC_TICKET_Pack42
Date&Time       :2018/1/4 下午 6:22
Describe        :Field_42:	Card Acceptor Identification Code(MID)
 		補充說明：
		1. 商店代號需左靠右補空白。
		2. 語音開卡交易須使用固定商店代號 0122900410 。
*/
int inNCCC_TICKET_Pack42(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 	inCnt = 0;
        char	szTemplate[16 + 1];
	char 	szDebugMsg[100 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_Pack42() START!");
	
	
        memset(szTemplate, 0x00, sizeof(szTemplate));
        if (inGetMerchantID(szTemplate) == VS_ERROR)
                return (VS_ERROR);

        inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 15, _PADDING_RIGHT_);
        memcpy((char *) &uszPackBuf[inCnt], szTemplate, strlen(szTemplate));
        inCnt += strlen(szTemplate);
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "F_42 [MID %s]", szTemplate);
		inLogPrintf(AT, szDebugMsg);
	}
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_Pack42() END!");

        return (inCnt);
}

/*
Function        :inNCCC_TICKET_Pack48
Date&Time       :2018/1/4 下午 6:22
Describe        :Field_48:	Additional Data - Private Use
*/
int inNCCC_TICKET_Pack48(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int	i;
        int	inCnt = 0;
	char	szDebugMsg[100 + 1];
	char	szTemplate[30 + 1];
	char	szAscii[100 + 1];
	char	szCustomerIndicator[3 + 1] = {0};
	char	szPOS_ID[6 + 1] = {0};
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_Pack48() START!");

	/*
		      Field	  |  Attribute	|		Comment
		------------------------------------------------------------------------------------
		Length		  |	n3	|	Fix length = 19, BCD format => 0x00 0x19
		------------------------------------------------------------------------------------
		SubField Length	  |	b8	|	Fix length = 18, Binary format => 0x12
		------------------------------------------------------------------------------------
		Store ID	  |	an6	|
		User Define	  |	an6	|
		Other		  |	an6	|
	*/
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
	{
		memset(szPOS_ID, 0x00, sizeof(szPOS_ID));
		inGetPOS_ID(szPOS_ID);
		
		uszPackBuf[inCnt ++] = 0x00;
		uszPackBuf[inCnt ++] = 0x19;
		uszPackBuf[inCnt ++] = 0x12;

		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(szTemplate, _HARDCODE_STORE_ID_CUS_021_TAKAWEL_, 12);
		memcpy(&szTemplate[12], szPOS_ID, 6);
		for (i = 0; i < 18; i ++)
		{
			if ((szTemplate[i] < 0x20) || (szTemplate[i] > 0x7e))
			{
				szTemplate[i] = 0x20;
			}
		}

		memcpy((char *)&uszPackBuf[inCnt], szTemplate, 18);
		inCnt += 18;
	}
	else
	{
		uszPackBuf[inCnt ++] = 0x00;
		uszPackBuf[inCnt ++] = 0x19;
		uszPackBuf[inCnt ++] = 0x12;

		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(szTemplate, pobTran->srTRec.szStoreID, 18);
		for (i = 0; i < 18; i ++)
		{
			if ((szTemplate[i] < 0x20) || (szTemplate[i] > 0x7e))
			{
				szTemplate[i] = 0x20;
			}
		}

		memcpy((char *)&uszPackBuf[inCnt], szTemplate, 18);
		inCnt += 18;
	}
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(szAscii, 0x00, sizeof(szAscii));
		
		memcpy(szTemplate, &uszPackBuf[0], inCnt);
		inFunc_BCD_to_ASCII(szAscii, (unsigned char*)szTemplate, inCnt);
		sprintf(szDebugMsg, "F_48 [StoreID %s]", szAscii);
		inLogPrintf(AT, szDebugMsg);
	}
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_Pack48() END!");

	return (inCnt);
}


/*
Function        :inNCCC_TICKET_Pack56
Date&Time       :2018/1/4 下午 4:32
Describe        :票證專用
*/
int inNCCC_TICKET_Pack56(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int		inCnt = 0, inPacketCnt = 0;
	char		szPacket[1000 + 1] = {0};
	char		szASCII[4 + 1] = {0};
	char		szTemplate[50 + 1] = {0};
	unsigned char	uszBCD[2 + 1];
	
	/*      Field_56:	電子票證交易資料 
		  SEQ   |  Position(s)	|  Description
		----------------------------------------------------------------------------------------------
		   1	|	1	|    1-2	Field total length.
		----------------------------------------------------------------------------------------------
		   2	|	2	|    3-6	Table ID
		----------------------------------------------------------------------------------------------
		   3	|	3	|    7-8	First sub-element length; the value of the “length”
		   					sub-field is always one;. BCD length for
							範例:20 bytes  0x00 0x20
		----------------------------------------------------------------------------------------------
		   4	|	4	|    9..xxxx	Table ID Data (ISO8583封包總長度不可超過999 bytes)
	*/

	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_Pack56() START!");
	
	
	memset(szPacket, 0x00, sizeof(szPacket));
	
	if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
        {
                memcpy(&szPacket[inPacketCnt], "@@@@@", 5); 
                inPacketCnt += 5;
		ginECC_F57_Len = 0;
		ginECC_F59_ET_Len = 0;
		ginECC_F63_Len = 0;
        }
        else        
        {        
                if ((pobTran->inTransactionCode == _SETTLE_ || pobTran->inTransactionCode == _CLS_SETTLE_) && pobTran->inISOTxnCode != _ADVICE_)
                {
                        /* EZ01 */ 
                        memcpy(&szPacket[inPacketCnt], "EZ01", 4); /* Table ID */
                        inPacketCnt += 4;
                	/* Sub Total Length */
                	inPacketCnt ++;
                	szPacket[inPacketCnt] = 0x02; 	/* Sub-Data Total Length */
                	inPacketCnt ++;
                
                        memcpy(&szPacket[inPacketCnt], "00", 2);         /* TEST */                       
                        inPacketCnt += 2;     
                }        
                else
                {        
                        if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
                        {       
                                /* IP00 - BMS or TMS */ 
                                memcpy(&szPacket[inPacketCnt], "IP00", 4); /* Table ID */
                                inPacketCnt += 4;
                        	/* Sub Total Length */
                        	inPacketCnt ++;
                        	szPacket[inPacketCnt] = 0x02; 	/* Sub-Data Total Length */
                        	inPacketCnt ++;
                        
                                if (pobTran->srTRec.srIPASSRec.uszQueryBit == VS_TRUE && !memcmp(guszTICKET_MTI, "0100", 4))
                                        memcpy(&szPacket[inPacketCnt], "01", 2);         
                                else        
                                        memcpy(&szPacket[inPacketCnt], "02", 2);         /* 01 = BMS, 02 = TMS/AMS */
                                        
                                inPacketCnt += 2;
                                
                                /* IP01 - 簽章 */ 
                                if (pobTran->srTRec.srIPASSRec.lnSign_Len > 0)
                                {       
                                        if (pobTran->inISOTxnCode == _ADVICE_)
                                        {           
                                                /* First階段感應的Advice不送IP01 */
                                                if (pobTran->srTRec.srIPASSRec.inStepNum == 2)
                                                {
                                                        memcpy(&szPacket[inPacketCnt], "IP01", 4); /* Table ID */
                                                        inPacketCnt += 4;
                                                	/* Sub Total Length */
                                                	memset(szASCII, 0x00, sizeof(szASCII));
                                                	sprintf(szASCII, "%04ld", pobTran->srTRec.srIPASSRec.lnSign_Len);  
                                                        inFunc_ASCII_to_BCD((unsigned char*)&szPacket[inPacketCnt], szASCII, 2);
                                                	inPacketCnt += 2;
                                                
                                                        memcpy(&szPacket[inPacketCnt], &pobTran->srTRec.srIPASSRec.szSign_Data[0], pobTran->srTRec.srIPASSRec.lnSign_Len);     
                                                        inPacketCnt += pobTran->srTRec.srIPASSRec.lnSign_Len;         
                                                }  
                                        }
                                        else 
                                        {
                                                memcpy(&szPacket[inPacketCnt], "IP01", 4); /* Table ID */
                                                inPacketCnt += 4;
                                        	/* Sub Total Length */
                                        	memset(szASCII, 0x00, sizeof(szASCII));
                                        	sprintf(szASCII, "%04ld", pobTran->srTRec.srIPASSRec.lnSign_Len);  
                                                inFunc_ASCII_to_BCD((unsigned char*)&szPacket[inPacketCnt], szASCII, 2);
                                        	inPacketCnt += 2;
                                        
                                                memcpy(&szPacket[inPacketCnt], &pobTran->srTRec.srIPASSRec.szSign_Data[0], pobTran->srTRec.srIPASSRec.lnSign_Len);     
                                                inPacketCnt += pobTran->srTRec.srIPASSRec.lnSign_Len;             
                                        }          
                                }
                                
                                if (pobTran->srTRec.srIPASSRec.szDAVTITxn[0] == 'D')
                                {
                                        if (pobTran->inISOTxnCode == _ADVICE_)
                                        {
                                                memcpy(&szPacket[inPacketCnt], "IP02", 4); /* Table ID */
                                                inPacketCnt += 4;
                                        	/* Sub Total Length */
                                        	memset(szASCII, 0x00, sizeof(szASCII));
                                        	sprintf(szASCII, "%04d", _IPASS_DAVTI_SIZE_);  
                                                inFunc_ASCII_to_BCD((unsigned char*)&szPacket[inPacketCnt], szASCII, 2);
                                        	inPacketCnt += 2;
                                        
                                                memcpy(&szPacket[inPacketCnt], &pobTran->srTRec.srIPASSRec.szDAVTITxn[0], _IPASS_DAVTI_SIZE_);     
                                                inPacketCnt += _IPASS_DAVTI_SIZE_;     
                                        }
                                        else 
                                        {
                                                if (pobTran->srTRec.srIPASSRec.inStepNum == 2)
                                                {
                                                        memcpy(&szPacket[inPacketCnt], "IP02", 4); /* Table ID */
                                                        inPacketCnt += 4;
                                                	/* Sub Total Length */
                                                	memset(szASCII, 0x00, sizeof(szASCII));
                                                	sprintf(szASCII, "%04d", _IPASS_DAVTI_SIZE_);  
                                                        inFunc_ASCII_to_BCD((unsigned char*)&szPacket[inPacketCnt], szASCII, 2);
                                                	inPacketCnt += 2;
                                        
                                                        memcpy(&szPacket[inPacketCnt], &pobTran->srTRec.srIPASSRec.szDAVTITxn[0], _IPASS_DAVTI_SIZE_);     
                                                        inPacketCnt += _IPASS_DAVTI_SIZE_;  
                                                }        
                                        }        
                                }        
                        }
			else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
                        {
                                if (pobTran->inISOTxnCode == _ADVICE_)
                                {
                                        if (pobTran->srTRec.srICASHRec.uszAPI_FailBit == VS_TRUE)
                                        {
                                                /* IC02 */
                                                memcpy(&szPacket[inPacketCnt], "IC02", 4); /* Table ID */
                                                inPacketCnt += 4;
                                        }
                                        else
                                        {
                                                /* IC01 */
                                                memcpy(&szPacket[inPacketCnt], "IC01", 4); /* Table ID */
                                                inPacketCnt += 4;
                                        }

                                        /* Sub Total Length */
                                	memset(szASCII, 0x00, sizeof(szASCII));
                                	sprintf(szASCII, "%04d", 339);
                                        inFunc_ASCII_to_BCD((unsigned char*)&szPacket[inPacketCnt], &szASCII[0], 2);
                                	inPacketCnt += 2;

					/* 特約機構編號 ACQUIRE_NO */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					inGetICASH_Special_Agency_Code(szTemplate);
                                	memcpy(&szPacket[inPacketCnt], szTemplate, 8);
                                        inPacketCnt += 8;

                                        if (pobTran->srTRec.srICASHRec.uszTxLogBit == VS_TRUE)
                                                memcpy(&szPacket[inPacketCnt], &pobTran->srTRec.srICASHRec.szTxLog_Other[16], 331);
                                        else
                                                memcpy(&szPacket[inPacketCnt], &pobTran->srTRec.srICASHRec.szTxLog[16], 331);

                                        inPacketCnt += 331;
                                }
                                else
                        	{
                        	        /* IC01 */
                                        memcpy(&szPacket[inPacketCnt], "IC01", 4); /* Table ID */
                                        inPacketCnt += 4;

                        	        /* Sub Total Length */
					memset(szASCII, 0x00, sizeof(szASCII));
                                	sprintf(szASCII, "%04ld", pobTran->srTRec.srICASHRec.lnSign_Len);
                                        inFunc_ASCII_to_BCD((unsigned char*)&szPacket[inPacketCnt], &szASCII[0], 2);
                                	inPacketCnt += 2;

                                        memcpy(&szPacket[inPacketCnt], &pobTran->srTRec.srICASHRec.szSign_Data[0], pobTran->srTRec.srICASHRec.lnSign_Len);
                                        inPacketCnt += pobTran->srTRec.srICASHRec.lnSign_Len;
                                }
                        }
                }
        }
	
	/* Packet Data Length */
	memset(szASCII, 0x00, sizeof(szASCII));
	sprintf(szASCII, "%04d", inPacketCnt);
	memset(uszBCD, 0x00, sizeof(uszBCD));
	inFunc_ASCII_to_BCD(uszBCD, szASCII, 2);
	memcpy((char *)&uszPackBuf[inCnt], &uszBCD[0], 2);
	inCnt += 2;
	/* Packet Data */
	memcpy((char *)&uszPackBuf[inCnt], &szPacket[0], inPacketCnt);
	inCnt += inPacketCnt;
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_Pack56() END!");
	
	return (inCnt);
}

/*
Function        :inNCCC_TICKET_Pack56_tSAM
Date&Time       :2020/8/21 下午 5:20
Describe        :票證專用
*/
int inNCCC_TICKET_Pack56_tSAM(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int		i = 0;
        int		inCnt = 0, inPacketCnt = 0;
	int		inRetVal = VS_SUCCESS;
	char		szPacket[1000 + 1] = {0};
	char		szASCII[4 + 1] = {0};
	char		szTemplate[50 + 1] = {0};
	char		szPath[200 + 1] = {0};
	unsigned char	uszBCD[2 + 1] = {0};
	unsigned char	uszLRC[1 + 1] = {0};
	unsigned long	ulHandle = 0;
	
	/*      Field_56:	電子票證交易資料 
		  SEQ   |  Position(s)	|  Description
		----------------------------------------------------------------------------------------------
		   1	|	1	|    1-2	Field total length.
		----------------------------------------------------------------------------------------------
		   2	|	2	|    3-6	Table ID
		----------------------------------------------------------------------------------------------
		   3	|	3	|    7-8	First sub-element length; the value of the “length”
		   					sub-field is always one;. BCD length for
							範例:20 bytes  0x00 0x20
		----------------------------------------------------------------------------------------------
		   4	|	4	|    9..xxxx	Table ID Data (ISO8583封包總長度不可超過999 bytes)
	*/

	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_Pack56_tSAM() START!");
	
	
	memset(szPacket, 0x00, sizeof(szPacket));
	
	if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
        {
                memcpy(&szPacket[inPacketCnt], "@@@@@", 5); 
                inPacketCnt += 5;
		ginECC_F57_Len = 0;
		ginECC_F59_ET_Len = 0;
		ginECC_F63_Len = 0;
        }
        else        
        {        
                if ((pobTran->inTransactionCode == _SETTLE_ || pobTran->inTransactionCode == _CLS_SETTLE_) && pobTran->inISOTxnCode != _ADVICE_)
                {
                        /* EZ01 */ 
                        memcpy(&szPacket[inPacketCnt], "EZ01", 4); /* Table ID */
                        inPacketCnt += 4;
                	/* Sub Total Length */
                	inPacketCnt ++;
                	szPacket[inPacketCnt] = 0x02; 	/* Sub-Data Total Length */
                	inPacketCnt ++;
                
                        memcpy(&szPacket[inPacketCnt], "00", 2);         /* TEST */                       
                        inPacketCnt += 2;     
                }        
                else
                {        
                        if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
                        {       
                                /* IP00 - BMS or TMS */ 
                                memcpy(&szPacket[inPacketCnt], "IP00", 4); /* Table ID */
                                inPacketCnt += 4;
                        	/* Sub Total Length */
                        	inPacketCnt ++;
                        	szPacket[inPacketCnt] = 0x02; 	/* Sub-Data Total Length */
                        	inPacketCnt ++;
                        
                                if (pobTran->srTRec.srIPASSRec.uszQueryBit == VS_TRUE && !memcmp(guszTICKET_MTI, "0100", 4))
                                        memcpy(&szPacket[inPacketCnt], "01", 2);         
                                else        
                                        memcpy(&szPacket[inPacketCnt], "02", 2);         /* 01 = BMS, 02 = TMS/AMS */
                                        
                                inPacketCnt += 2;
                                
                                /* IP01 - 簽章 */ 
                                if (pobTran->srTRec.srIPASSRec.lnSign_Len > 0)
                                {       
                                        if (pobTran->inISOTxnCode == _ADVICE_)
                                        {           
                                                /* First階段感應的Advice不送IP01 */
                                                if (pobTran->srTRec.srIPASSRec.inStepNum == 2)
                                                {
                                                        memcpy(&szPacket[inPacketCnt], "IP01", 4); /* Table ID */
                                                        inPacketCnt += 4;
                                                	/* Sub Total Length */
                                                	memset(szASCII, 0x00, sizeof(szASCII));
                                                	sprintf(szASCII, "%04ld", pobTran->srTRec.srIPASSRec.lnSign_Len);  
                                                        inFunc_ASCII_to_BCD((unsigned char*)&szPacket[inPacketCnt], szASCII, 2);
                                                	inPacketCnt += 2;
                                                
                                                        memcpy(&szPacket[inPacketCnt], &pobTran->srTRec.srIPASSRec.szSign_Data[0], pobTran->srTRec.srIPASSRec.lnSign_Len);     
                                                        inPacketCnt += pobTran->srTRec.srIPASSRec.lnSign_Len;         
                                                }  
                                        }
                                        else 
                                        {
                                                memcpy(&szPacket[inPacketCnt], "IP01", 4); /* Table ID */
                                                inPacketCnt += 4;
                                        	/* Sub Total Length */
                                        	memset(szASCII, 0x00, sizeof(szASCII));
                                        	sprintf(szASCII, "%04ld", pobTran->srTRec.srIPASSRec.lnSign_Len);  
                                                inFunc_ASCII_to_BCD((unsigned char*)&szPacket[inPacketCnt], szASCII, 2);
                                        	inPacketCnt += 2;
                                        
                                                memcpy(&szPacket[inPacketCnt], &pobTran->srTRec.srIPASSRec.szSign_Data[0], pobTran->srTRec.srIPASSRec.lnSign_Len);     
                                                inPacketCnt += pobTran->srTRec.srIPASSRec.lnSign_Len;             
                                        }          
                                }
                                
                                if (pobTran->srTRec.srIPASSRec.szDAVTITxn[0] == 'D')
                                {
                                        if (pobTran->inISOTxnCode == _ADVICE_)
                                        {
                                                memcpy(&szPacket[inPacketCnt], "IP02", 4); /* Table ID */
                                                inPacketCnt += 4;
                                        	/* Sub Total Length */
                                        	memset(szASCII, 0x00, sizeof(szASCII));
                                        	sprintf(szASCII, "%04d", _IPASS_DAVTI_SIZE_);  
                                                inFunc_ASCII_to_BCD((unsigned char*)&szPacket[inPacketCnt], szASCII, 2);
                                        	inPacketCnt += 2;
                                        
                                                memcpy(&szPacket[inPacketCnt], &pobTran->srTRec.srIPASSRec.szDAVTITxn[0], _IPASS_DAVTI_SIZE_);     
                                                inPacketCnt += _IPASS_DAVTI_SIZE_;     
                                        }
                                        else 
                                        {
                                                if (pobTran->srTRec.srIPASSRec.inStepNum == 2)
                                                {
                                                        memcpy(&szPacket[inPacketCnt], "IP02", 4); /* Table ID */
                                                        inPacketCnt += 4;
                                                	/* Sub Total Length */
                                                	memset(szASCII, 0x00, sizeof(szASCII));
                                                	sprintf(szASCII, "%04d", _IPASS_DAVTI_SIZE_);  
                                                        inFunc_ASCII_to_BCD((unsigned char*)&szPacket[inPacketCnt], szASCII, 2);
                                                	inPacketCnt += 2;
                                        
                                                        memcpy(&szPacket[inPacketCnt], &pobTran->srTRec.srIPASSRec.szDAVTITxn[0], _IPASS_DAVTI_SIZE_);     
                                                        inPacketCnt += _IPASS_DAVTI_SIZE_;  
                                                }        
                                        }        
                                }        
                        }
			else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
                        {
                                if (pobTran->inISOTxnCode == _ADVICE_)
                                {
                                        if (pobTran->srTRec.srICASHRec.uszAPI_FailBit == VS_TRUE)
                                        {
                                                /* IC02 */
                                                memcpy(&szPacket[inPacketCnt], "IC02", 4); /* Table ID */
                                                inPacketCnt += 4;
                                        }
                                        else
                                        {
                                                /* IC01 */
                                                memcpy(&szPacket[inPacketCnt], "IC01", 4); /* Table ID */
                                                inPacketCnt += 4;
                                        }

                                        /* Sub Total Length */
                                	memset(szASCII, 0x00, sizeof(szASCII));
                                	sprintf(szASCII, "%04d", 339);
                                        inFunc_ASCII_to_BCD((unsigned char*)&szPacket[inPacketCnt], &szASCII[0], 2);
                                	inPacketCnt += 2;

					/* 特約機構編號 ACQUIRE_NO */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					inGetICASH_Special_Agency_Code(szTemplate);
                                	memcpy(&szPacket[inPacketCnt], szTemplate, 8);
                                        inPacketCnt += 8;

                                        if (pobTran->srTRec.srICASHRec.uszTxLogBit == VS_TRUE)
                                                memcpy(&szPacket[inPacketCnt], &pobTran->srTRec.srICASHRec.szTxLog_Other[16], 331);
                                        else
                                                memcpy(&szPacket[inPacketCnt], &pobTran->srTRec.srICASHRec.szTxLog[16], 331);

                                        inPacketCnt += 331;
                                }
                                else
                        	{
                        	        /* IC01 */
                                        memcpy(&szPacket[inPacketCnt], "IC01", 4); /* Table ID */
                                        inPacketCnt += 4;

                        	        /* Sub Total Length */
					memset(szASCII, 0x00, sizeof(szASCII));
                                	sprintf(szASCII, "%04ld", pobTran->srTRec.srICASHRec.lnSign_Len);
                                        inFunc_ASCII_to_BCD((unsigned char*)&szPacket[inPacketCnt], &szASCII[0], 2);
                                	inPacketCnt += 2;

                                        memcpy(&szPacket[inPacketCnt], &pobTran->srTRec.srICASHRec.szSign_Data[0], pobTran->srTRec.srICASHRec.lnSign_Len);
                                        inPacketCnt += pobTran->srTRec.srICASHRec.lnSign_Len;
                                }
                        }
                }
        }
	
	/* 這兩種交易別不加密 */
	/* 悠遊卡用API加密 */
	if (memcmp(guszTICKET_MTI, "0500", 4) == 0	||
	    memcmp(guszTICKET_MTI, "0800", 4) == 0	||
	    pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
	{
		guszField_56 = VS_TRUE;
	}
	else
	{
		/* 先Gzip */
		inRetVal = inNCCC_Ticket_Gen_F_56_File(szPacket, inPacketCnt);
		if (inRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "ininNCCC_Ticket_Gen_F_56_File Fail");
			}
		}

		inRetVal = inFunc_Data_GZip("", _ESVC_FILE_F_56_, _FS_DATA_PATH_);
		if (inRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "inFunc_Data_GZip Fail");
			}
		}
		/* 加密第17~32 byte */
		/* 置換 */
		inRetVal = inNCCC_Ticket_Data_Compress_Encryption(pobTran);
		if (inRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "inNCCC_Ticket_Data_Compress_Encryption Fail");
			}
		}
		else
		{
			guszField_56 = VS_TRUE;
		}

		/* 讀出來，貼上 */
		memset(szPacket, 0x00, sizeof(szPacket));
		inPacketCnt = 0;
		memset(szPath, 0x00, sizeof(szPath));
		sprintf(szPath, "%s%s", _FS_DATA_PATH_, _ESVC_FILE_F_56_GZ_ENCRYPTED_);
		inFile_Linux_Get_FileSize_By_Stat(szPath, &inPacketCnt);
		inRetVal = inFILE_OpenReadOnly(&ulHandle, (unsigned char*)_ESVC_FILE_F_56_GZ_ENCRYPTED_);
		if (inRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "inFILE_OpenReadOnly Fail");
			}
		}
		else
		{
			inFILE_Read(&ulHandle, (unsigned char*)szPacket, (unsigned long)inPacketCnt);

			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "inPacketCnt : %d", inPacketCnt);
			}

			inFILE_Close(&ulHandle);

			/* 補上檢查碼LRC */
			memset(uszLRC, 0x00, sizeof(uszLRC));
			uszLRC[0] = 0x00;
			for (i = 0; i < inPacketCnt; i++)
			{
				uszLRC[0] = uszLRC[0] ^ szPacket[i];
			}
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "LRC : %X", uszLRC[0]);
			}

			memcpy(&szPacket[inPacketCnt], uszLRC, 1);
			inPacketCnt++;
		}
	}
	
	/* Packet Data Length */
	memset(szASCII, 0x00, sizeof(szASCII));
	sprintf(szASCII, "%04d", inPacketCnt);
	memset(uszBCD, 0x00, sizeof(uszBCD));
	inFunc_ASCII_to_BCD(uszBCD, szASCII, 2);
	memcpy((char *)&uszPackBuf[inCnt], &uszBCD[0], 2);
	inCnt += 2;
	/* Packet Data */
	memcpy((char *)&uszPackBuf[inCnt], &szPacket[0], inPacketCnt);
	inCnt += inPacketCnt;
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_Pack56_tSAM() END!");
	
	return (inCnt);
}

/*
Function        :inNCCC_TICKET_Pack57_tSAM
Date&Time       :
Describe        :Field_57:	T-SAM Decryption Relation Data
		加密之交易，EDC需送Filed_57，Decryption Server藉此判斷為加密交易。
		前8 bytes(64 bits)為BitMap，將加密之欄位Bit On起來；
		另每個加密之欄位資料需有相對應4 bytes的檢查碼，依序置放於Field_57 BitMap之後。

		範例：
		Field_35 加密，則資料內容應為：
		00 16 XX XX XX XX XX XX XX XX YY YY YY YY ZZ ZZ ZZ ZZ
		其中1~2 bytes為欄位長度，3~10 bytes為BitMap，11~14 bytes為Field_35之檢查碼，15~18 bytes為Field_55之檢查碼。	
*/
int inNCCC_TICKET_Pack57_tSAM(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int		inCnt = 0, inPacketCnt = 0;
	char		szASCII[4 + 1];
	char		szPacket[100 + 1];
	char		szPacket_Ascii[100 + 1];
	char		szTemplate[100 + 1];
	unsigned char	uszBCD[2 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_Pack57_tSAM() START!");
	
	
	/* BitMap 8 Byte */
	memset(szPacket, 0x00, sizeof(szPacket));
	szPacket[inPacketCnt ++] = 0x00;
	szPacket[inPacketCnt ++] = 0x00;
	szPacket[inPacketCnt ++] = 0x00;
	szPacket[inPacketCnt ++] = 0x00;

	/* 第5個byte */
	szPacket[inPacketCnt] = 0x00;
	if (guszField_35 == VS_TRUE)
	{
		szPacket[inPacketCnt] += 0x20;
	}
	inPacketCnt ++;

	/* 第6個byte */
	szPacket[inPacketCnt ++] = 0x00;
	
	/* 第7個byte */
	szPacket[inPacketCnt] = 0x00;
	if (guszField_56 == VS_TRUE)
	{
		szPacket[inPacketCnt] += 0x01;
	}
	inPacketCnt ++;

	/* 第8個byte */
	szPacket[inPacketCnt ++] = 0x00;
	
	/* 計算位置用 */
	if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
	{
		/* 2是最前面的Packet Data Length，這邊要預先計算 */
		ginECC_F57_Len += inPacketCnt + 2;
	}
	
	if (guszField_35 == VS_TRUE)
	{
		memcpy((char *)&szPacket[inPacketCnt], (char *)&gusztSAMCheckSum_35[0], 4);
		inPacketCnt += 4;
	}
	
	if (guszField_56 == VS_TRUE)
	{
		memcpy((char *)&szPacket[inPacketCnt], (char *)&gusztSAMCheckSum_56[0], 4);
		inPacketCnt += 4;
	}

	/* Packet Data Length */
	memset(szASCII, 0x00, sizeof(szASCII));
	sprintf(szASCII, "%04d", inPacketCnt);
	memset(uszBCD, 0x00, sizeof(uszBCD));
	inFunc_ASCII_to_BCD(uszBCD, szASCII, 2);
	memcpy((char *)&uszPackBuf[inCnt], &uszBCD[0], 2);
	inCnt += 2;
	
	/* Packet Data */
	memcpy((char *)&uszPackBuf[inCnt], &szPacket[0], inPacketCnt);
	inCnt += inPacketCnt;
	
	if (ginDebug == VS_TRUE)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(szPacket_Ascii, 0x00, sizeof(szPacket_Ascii));
		
		inFunc_BCD_to_ASCII(szPacket_Ascii, (unsigned char*)szPacket, inPacketCnt);
		inLogPrintf(AT, "F_57 [CheckSum]");
		sprintf(szTemplate, "%s %s", szASCII, szPacket_Ascii);
		inLogPrintf(AT, szTemplate);
	}
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_Pack57_tSAM() END!");
	
	/* 計算位置用 */
	if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
	{
		ginECC_F59_ET_Len += inCnt;
		ginECC_F63_Len += inCnt;
	}
	
        return (inCnt);
}

/*
Function        :inNCCC_TICKET_ATS_Pack59
Date&Time       :2018/1/4 下午 4:40
Describe        : by bai
		Field_59:	Reserved-Private Data
		  SEQ   |  Position(s)	|  Description
		----------------------------------------------------------------------------------------------
		   1	|	1	|    1-2	Field total length.
		----------------------------------------------------------------------------------------------
		   2	|	2	|    3-4	Table ID
		----------------------------------------------------------------------------------------------
		   3	|	3	|    5-6	First sub-element length; the value of the “length”
		   					sub-field is always one;. BCD length for
							範例:20 bytes  0x00 0x20
		----------------------------------------------------------------------------------------------
		   4		4	|    6..750	Table ID Data (ISO8583封包總長度不可超過999 bytes)

*/
int inNCCC_TICKET_ATS_Pack59(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int		inCnt = 0, inPacketCnt = 0;
	long		lnAmt = 0;
	char		szASCII[4 + 1] = {0};
	char		szTemplate[100 + 1] = {0}, szPacket[768 + 1] = {0};
	char		szTemp[8 + 1] = {0};
	char		szTemp2[8 + 1] = {0};
	char		szFESMode[2 + 1] = {0};
	char		szFES_ID[3 + 1] = {0};
	char		szCFESMode[2 + 1] = {0};
	unsigned char	uszBCD[2 + 1] = {0};
	RTC_NEXSYS	srRTC = {};
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_ATS_Pack59() START!");

	memset(szPacket, 0x00, sizeof(szPacket));

	if ((pobTran->inTransactionCode == _SETTLE_ || pobTran->inTransactionCode == _CLS_SETTLE_) && pobTran->inISOTxnCode != _ADVICE_)
        {
		/* (需求單 - 107276)自助交易標準做法 MFES雲端化 by Russell 2019/3/5 下午 4:39 */
		/* 
		 *  Table ID “ND”：FES ID (FES 請款代碼)
		 *  端末機依TMS設定，若NCCC_FES_MODE = "03"(MFES)，則本欄位填入TMS下載的FES_ID值。其餘NCCC_FES_MODE，則預設上傳3BYTE（空白）
		 */
		memset(szFESMode, 0x00, sizeof(szFESMode));
		inGetNCCCFESMode(szFESMode);
		memset(szCFESMode, 0x00, sizeof(szCFESMode));
		inGetCloud_MFES(szCFESMode);
		if (memcmp(szFESMode, _NCCC_03_MFES_MODE_, strlen(_NCCC_03_MFES_MODE_)) == 0)
		{
			if (memcmp(szCFESMode, "Y", strlen("Y")) == 0)
			{
				memcpy(&szPacket[inPacketCnt], "ND", 2); /* Table ID */
				inPacketCnt += 2;

				/* Sub Total Length */
				szPacket[inPacketCnt] = 0x00;
				inPacketCnt ++;
				szPacket[inPacketCnt] = 0x03;
				inPacketCnt ++;

				memset(szFES_ID, 0x00, sizeof(szFES_ID));
				inGetFES_ID(szFES_ID);

				/* 沒值就給三個空白，讓主機回30 */
				/* ATS判斷端末機上傳Table ID "NF" = CFES
				而Table ID = "ND"為3 Bytes(空白)時，需回覆Response code = 30 */
				if (strlen(szFES_ID) > 0)
				{
					memcpy(&szPacket[inPacketCnt], szFES_ID, 3);
				}
				else
				{
					memcpy(&szPacket[inPacketCnt], "   ", 3);
				}
				inPacketCnt += 3;
			}
			else
			{
				/* MFES不送ND */
			}
		}
		else
		{
			memcpy(&szPacket[inPacketCnt], "ND", 2); /* Table ID */
			inPacketCnt += 2;

			/* Sub Total Length */
			szPacket[inPacketCnt] = 0x00;
			inPacketCnt ++;
			szPacket[inPacketCnt] = 0x03;
			inPacketCnt ++;

			memcpy(&szPacket[inPacketCnt], "   ", 3);
			inPacketCnt += 3;
		}
		
                /*
			●	Table ID “NF”: FES Indicator
			依TMS Config.txt中NCCC_FES_Mode參數設定上傳值。
			當NCCC_FES_Mode=03時，表示此為MFES端末機，本欄位值為”MFxx”。
			(端末機預設xx=0x20 0x20。xx由MFES填入新值。
			當MFES已經進行基本查核後SAF的交易，ATS不需再查，則MFES填入xx=”SF”。
			反之則維持0x20,0x20)
			當NCCC_FES_Mode=05時，表示此非MFES端末機，本欄位值為”ATS”。(表0x20)
			ATS以此判斷若為”MFES”端末機，則ATS不產生該端末機的請款檔。
		*/
		memcpy(&szPacket[inPacketCnt], "NF", 2); /* Table ID */
		inPacketCnt += 2;
		/* Sub Total Length */
		inPacketCnt ++;
		szPacket[inPacketCnt] = 0x04;
		inPacketCnt ++;
		
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetNCCCFESMode(szTemplate);
		memset(szCFESMode, 0x00, sizeof(szCFESMode));
		inGetCloud_MFES(szCFESMode);

		if (memcmp(szTemplate, _NCCC_04_MPAS_MODE_, strlen(_NCCC_04_MPAS_MODE_)) == 0)
		{
			memcpy(&szPacket[inPacketCnt], "MPAS", 4);
		}
		else if (memcmp(szFESMode, _NCCC_03_MFES_MODE_, strlen(_NCCC_03_MFES_MODE_)) == 0)
		{
			if (memcmp(szCFESMode, "Y", strlen("Y")) == 0)
			{
				memcpy(&szPacket[inPacketCnt], "CFES", 4);
			}
			else
			{
				memcpy(&szPacket[inPacketCnt], "MF  ", 4);
			}
		}
		else
		{
			memcpy(&szPacket[inPacketCnt], "ATS ", 4);
		}
		inPacketCnt += 4;
        }
        else
        {
        	if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
                {
			/* 0500 部份和上面結帳交易重複，故移除 by Russell 2019/11/18 上午 10:03 */
                	if (!memcmp(&guszTICKET_MTI[0], "0800", 4))
                        {
                                /* ●	Table ID “N3”: Host AD “YYYY” (系統西元年) */
                		memcpy(&szPacket[inPacketCnt], "N3", 2); /* Table ID */
                		inPacketCnt += 2;
                		/* Sub Total Length */
                		inPacketCnt ++;
                		szPacket[inPacketCnt] = 0x04;
                		inPacketCnt ++;
                		memcpy(&szPacket[inPacketCnt], "0000", 4); /* YYYY，上傳時，固定為'0000'，並由授權主機更新回傳 */
                		inPacketCnt += 4;
                
				/* (需求單 - 107276)自助交易標準做法 MFES雲端化 by Russell 2019/3/5 下午 4:39 */
				/* 
				 *  Table ID “ND”：FES ID (FES 請款代碼)
				 *  端末機依TMS設定，若NCCC_FES_MODE = "03"(MFES)，則本欄位填入TMS下載的FES_ID值。其餘NCCC_FES_MODE，則預設上傳3BYTE（空白）
				 */
				memset(szFESMode, 0x00, sizeof(szFESMode));
				inGetNCCCFESMode(szFESMode);
				memset(szCFESMode, 0x00, sizeof(szCFESMode));
				inGetCloud_MFES(szCFESMode);
				if (memcmp(szFESMode, _NCCC_03_MFES_MODE_, strlen(_NCCC_03_MFES_MODE_)) == 0)
				{
					if (memcmp(szCFESMode, "Y", strlen("Y")) == 0)
					{
						memcpy(&szPacket[inPacketCnt], "ND", 2); /* Table ID */
						inPacketCnt += 2;

						/* Sub Total Length */
						szPacket[inPacketCnt] = 0x00;
						inPacketCnt ++;
						szPacket[inPacketCnt] = 0x03;
						inPacketCnt ++;

						memset(szFES_ID, 0x00, sizeof(szFES_ID));
						inGetFES_ID(szFES_ID);

						/* 沒值就給三個空白，讓主機回30 */
						/* ATS判斷端末機上傳Table ID "NF" = CFES
						而Table ID = "ND"為3 Bytes(空白)時，需回覆Response code = 30 */
						if (strlen(szFES_ID) > 0)
						{
							memcpy(&szPacket[inPacketCnt], szFES_ID, 3);
						}
						else
						{
							memcpy(&szPacket[inPacketCnt], "   ", 3);
						}
						inPacketCnt += 3;
					}
					else
					{
						/* MFES不送ND */
					}
				}
				else
				{
					memcpy(&szPacket[inPacketCnt], "ND", 2); /* Table ID */
					inPacketCnt += 2;

					/* Sub Total Length */
					szPacket[inPacketCnt] = 0x00;
					inPacketCnt ++;
					szPacket[inPacketCnt] = 0x03;
					inPacketCnt ++;

					memcpy(&szPacket[inPacketCnt], "   ", 3);
					inPacketCnt += 3;
				}
				
                		/* ●	Table ID “NF”: FES Indicator */
                		memcpy(&szPacket[inPacketCnt], "NF", 2); /* Table ID */
                		inPacketCnt += 2;
                		/* Sub Total Length */
                		inPacketCnt ++;
                		szPacket[inPacketCnt] = 0x04;
                		inPacketCnt ++;
				
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetNCCCFESMode(szTemplate);
				memset(szCFESMode, 0x00, sizeof(szCFESMode));
				inGetCloud_MFES(szCFESMode);
				
				if (memcmp(szTemplate, _NCCC_04_MPAS_MODE_, strlen(_NCCC_04_MPAS_MODE_)) == 0)
				{
					memcpy(&szPacket[inPacketCnt], "MPAS", 4);
				}
				else if (memcmp(szFESMode, _NCCC_03_MFES_MODE_, strlen(_NCCC_03_MFES_MODE_)) == 0)
				{
					if (memcmp(szCFESMode, "Y", strlen("Y")) == 0)
					{
						memcpy(&szPacket[inPacketCnt], "CFES", 4);
					}
					else
					{
						memcpy(&szPacket[inPacketCnt], "MF  ", 4);
					}
				}
				else
				{
					memcpy(&szPacket[inPacketCnt], "ATS ", 4);
				}
                		inPacketCnt += 4;
                        }
                        else
                        {	
                                /* ●	Table ID “N1”: UnionPay (原CUP)交易 */
        			memset(szPacket, 0x00, sizeof(szPacket));
        			/* UnionPay(原CUP) data information */
        			memcpy(&szPacket[inPacketCnt], "N1", 2); /* Table ID */
        			inPacketCnt += 2;
        			/* Sub Total Length */
        			inPacketCnt ++;
        			szPacket[inPacketCnt] = 0x02;
        			inPacketCnt ++;
        			/*
        				Function number.
        				The terminal support UnionPay (原CUP) function.
        				0 = Normal.
        				1 = Support.
        			*/
        			szPacket[inPacketCnt] = '1';
        			inPacketCnt ++;
        			/*
        				Acquire Indicator.
        				Terminal hot key control.
        				0 = Normal.
        			*/
        			szPacket[inPacketCnt] = '0';
        			inPacketCnt ++;
        			
        			/*
                			●	Table ID “N2”: Settlement Flag
                			Settlement Flag
                			依TMS下載的Settlement Flag填入此欄位
                			1=”ST” (ATS清算)
                			2=”AU” (ATS不清算)
                		*/
        			memcpy(&szPacket[inPacketCnt], "N2", 2); /* Table ID */
        			inPacketCnt += 2;
        			/* Sub Total Length */
        			inPacketCnt ++;
        			szPacket[inPacketCnt] = 0x02;
        			inPacketCnt ++;
        
        			/*
        				吳升文(Angus Wu) <angus.wu@nccc.com.tw> 2014-05-26 (週一) PM 03:43 通知
        				ATS針對Table ID “N2” Settlement Flag的值，請調整為當收到TMS客製化參數042(建設公司)及043(保險公司)時，填入”AU”。
        				其餘填入”ST”。
        			*/
        			memcpy(&szPacket[inPacketCnt], "ST", 2);
        			inPacketCnt += 2;
        			
        			/*
                			●	Table ID “N3”: Host AD “YYYY” (系統西元年)
                			1. 為解決ISO8583無法更新西元年的問題，故增加此Table ID N3，端末機可依此欄位更新端末機系統年份(西元)。
                			2. 避免端末機因Clock故障或是跨年的秒差造成交易年份錯誤。
                		*/
				if (memcmp(guszTICKET_MTI, "0220", strlen("0220")) == 0)
				{
					/* 0220不帶N3 */
				}
				else
				{
					memcpy(&szPacket[inPacketCnt], "N3", 2); /* Table ID */
					inPacketCnt += 2;
					/* Sub Total Length */
					inPacketCnt ++;
					szPacket[inPacketCnt] = 0x04;
					inPacketCnt ++;
					memcpy(&szPacket[inPacketCnt], "0000", 4); /* YYYY，上傳時，固定為'0000'，並由授權主機更新回傳 */
					inPacketCnt += 4;
				}
        			
				/* (需求單 - 107276)自助交易標準做法 MFES雲端化 by Russell 2019/3/5 下午 4:39 */
				/* 
				 *  Table ID “ND”：FES ID (FES 請款代碼)
				 *  端末機依TMS設定，若NCCC_FES_MODE = "03"(MFES)，則本欄位填入TMS下載的FES_ID值。其餘NCCC_FES_MODE，則預設上傳3BYTE（空白）
				 */
				memset(szFESMode, 0x00, sizeof(szFESMode));
				inGetNCCCFESMode(szFESMode);
				memset(szCFESMode, 0x00, sizeof(szCFESMode));
				inGetCloud_MFES(szCFESMode);
				if (memcmp(szFESMode, _NCCC_03_MFES_MODE_, strlen(_NCCC_03_MFES_MODE_)) == 0)
				{
					if (memcmp(szCFESMode, "Y", strlen("Y")) == 0)
					{
						memcpy(&szPacket[inPacketCnt], "ND", 2); /* Table ID */
						inPacketCnt += 2;

						/* Sub Total Length */
						szPacket[inPacketCnt] = 0x00;
						inPacketCnt ++;
						szPacket[inPacketCnt] = 0x03;
						inPacketCnt ++;

						memset(szFES_ID, 0x00, sizeof(szFES_ID));
						inGetFES_ID(szFES_ID);

						/* 沒值就給三個空白，讓主機回30 */
						/* ATS判斷端末機上傳Table ID "NF" = CFES
						而Table ID = "ND"為3 Bytes(空白)時，需回覆Response code = 30 */
						if (strlen(szFES_ID) > 0)
						{
							memcpy(&szPacket[inPacketCnt], szFES_ID, 3);
						}
						else
						{
							memcpy(&szPacket[inPacketCnt], "   ", 3);
						}
						inPacketCnt += 3;
					}
					else
					{
						/* MFES不送ND */
					}
				}
				else
				{
					memcpy(&szPacket[inPacketCnt], "ND", 2); /* Table ID */
					inPacketCnt += 2;

					/* Sub Total Length */
					szPacket[inPacketCnt] = 0x00;
					inPacketCnt ++;
					szPacket[inPacketCnt] = 0x03;
					inPacketCnt ++;

					memcpy(&szPacket[inPacketCnt], "   ", 3);
					inPacketCnt += 3;
				}
			       
        			/*
                			●	Table ID “NF”: FES Indicator
                			依TMS Config.txt中NCCC_FES_Mode參數設定上傳值。
                			當NCCC_FES_Mode=03時，表示此為MFES端末機，本欄位值為”MFxx”。
                			(端末機預設xx=0x20 0x20。xx由MFES填入新值。
                			當MFES已經進行基本查核後SAF的交易，ATS不需再查，則MFES填入xx=”SF”。
                			反之則維持0x20,0x20)
                			當NCCC_FES_Mode=05時，表示此非MFES端末機，本欄位值為”ATS”。(表0x20)
                			ATS以此判斷若為”MFES”端末機，則ATS不產生該端末機的請款檔。
                		*/
                		memcpy(&szPacket[inPacketCnt], "NF", 2); /* Table ID */
                		inPacketCnt += 2;
                		/* Sub Total Length */
                		inPacketCnt ++;
                		szPacket[inPacketCnt] = 0x04;
                		inPacketCnt ++;
				
                		memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetNCCCFESMode(szTemplate);
				memset(szCFESMode, 0x00, sizeof(szCFESMode));
				inGetCloud_MFES(szCFESMode);
				
				if (memcmp(szTemplate, _NCCC_04_MPAS_MODE_, strlen(_NCCC_04_MPAS_MODE_)) == 0)
				{
					memcpy(&szPacket[inPacketCnt], "MPAS", 4);
				}
				else if (memcmp(szFESMode, _NCCC_03_MFES_MODE_, strlen(_NCCC_03_MFES_MODE_)) == 0)
				{
					if (memcmp(szCFESMode, "Y", strlen("Y")) == 0)
					{
						memcpy(&szPacket[inPacketCnt], "CFES", 4);
					}
					else
					{
						memcpy(&szPacket[inPacketCnt], "MF  ", 4);
					}
				}
				else
				{
					memcpy(&szPacket[inPacketCnt], "ATS ", 4);
				}
                		inPacketCnt += 4;
        				
                		/* 
                		        ●	Table ID “ET”: 電票交易資訊 
                		*/ 
                                memcpy(&szPacket[inPacketCnt], "ET", 2); /* Table ID */
                                inPacketCnt += 2;
                        	/* Sub Total Length */
                        	inPacketCnt ++;
                        	szPacket[inPacketCnt] = 0x18; 	/* Sub-Data Total Length */
                        	inPacketCnt ++;
                        	
                        	/* Before Amt */
                        	if (pobTran->srTRec.lnCardRemainAmount < 0)
                        	{      
                        	        szPacket[inPacketCnt] = 0x2D;  
                        	        inPacketCnt ++;
                        	        memset(szTemplate, 0x00, sizeof(szTemplate)); 
                        	        sprintf(szTemplate, "%08ld", (0 - pobTran->srTRec.lnCardRemainAmount));
                        	}
                        	else     
                        	{
                        	        szPacket[inPacketCnt] = 0x20; 
                        	        inPacketCnt ++;
                        	        memset(szTemplate, 0x00, sizeof(szTemplate));    
                                        sprintf(szTemplate, "%08ld", pobTran->srTRec.lnCardRemainAmount);
                                }
                                          
                                memcpy(&szPacket[inPacketCnt], &szTemplate[0], 8);           
                                inPacketCnt += 8;
                                
                                /* After Amt */
                                if (pobTran->srTRec.inCode == _TICKET_IPASS_QUERY_	||
				    pobTran->srTRec.inCode == _TICKET_IPASS_INQUIRY_)
                                        lnAmt = pobTran->srTRec.lnCardRemainAmount;     
                                else if (pobTran->srTRec.inCode == _TICKET_IPASS_DEDUCT_)
                                        lnAmt = pobTran->srTRec.lnCardRemainAmount - pobTran->srTRec.lnTxnAmount;
                                else if (pobTran->srTRec.inCode == _TICKET_IPASS_AUTO_TOP_UP_)
                                        lnAmt = pobTran->srTRec.lnCardRemainAmount + pobTran->srTRec.lnTotalTopUpAmount;    
                                else if (pobTran->srTRec.inCode == _TICKET_IPASS_TOP_UP_)
                                        lnAmt = pobTran->srTRec.lnCardRemainAmount + pobTran->srTRec.lnTxnAmount;              
                                else if (pobTran->srTRec.inCode == _TICKET_IPASS_VOID_TOP_UP_)
                                        lnAmt = pobTran->srTRec.lnCardRemainAmount - pobTran->srTRec.lnTxnAmount;                 
                                else              
                                        lnAmt = pobTran->srTRec.lnCardRemainAmount + pobTran->srTRec.lnTxnAmount;
                                
                                if (lnAmt < 0)
                        	{
                        	        szPacket[inPacketCnt] = '-';  
                        	        inPacketCnt ++;
                        	        memset(szTemplate, 0x00, sizeof(szTemplate)); 
                        	        sprintf(szTemplate, "%08ld", (0 - lnAmt));
                        	}
                        	else     
                        	{
                        	        szPacket[inPacketCnt] = ' '; 
                        	        inPacketCnt ++;
                        	        memset(szTemplate, 0x00, sizeof(szTemplate));    
                                        sprintf(szTemplate, "%08ld", lnAmt);
                                }
                                
                                memcpy(&szPacket[inPacketCnt], &szTemplate[0], 8);           
                                inPacketCnt += 8;
				
				/* 組MAC使用 */
				memset(gszMAC_F_59, 0x00, sizeof(gszMAC_F_59));
				memcpy(gszMAC_F_59, &szPacket[inPacketCnt - 18], 18);
                                
                                /* 
                		        ●	Table ID “ES”: 電票交易序號 
                		*/ 
                                memcpy(&szPacket[inPacketCnt], "ES", 2); /* Table ID */
                                inPacketCnt += 2;
                        	/* Sub Total Length */
                        	inPacketCnt ++;
                        	szPacket[inPacketCnt] = 0x06; 	/* Sub-Data Total Length */
                        	inPacketCnt ++;
                        	
                        	/* RF序號 */      
                                memcpy(&szPacket[inPacketCnt], &pobTran->srTRec.szTicketRefundCode[0], 6);           
                                inPacketCnt += 6;
                                
				/* 
					●	Table ID “ED”: 電票原交易日期 
				*/ 
                                if (pobTran->srTRec.inCode == _TICKET_IPASS_REFUND_ && 
				   (!memcmp(&guszTICKET_MTI[0], "0100", 4) || !memcmp(&guszTICKET_MTI[0], "0400", 4)))
                                {        
                                        
                                        memcpy(&szPacket[inPacketCnt], "ED", 2); /* Table ID */
                                        inPacketCnt += 2;
                                	/* Sub Total Length */
                                	inPacketCnt ++;
                                	szPacket[inPacketCnt] = 0x08; 	/* Sub-Data Total Length */
                                	inPacketCnt ++;
                                	
                                	/* 原交易日期 - 輸入日期大於目前日期，判定為去年 */
					memset(szTemp, 0x00, sizeof(szTemp));
					memset(szTemp2, 0x00, sizeof(szTemp2));
					/* [20251219_BUG_MDF][ECC] 修改電票退貨的判斷日期,由 2位MM改為 4位 MMDD */
					memcpy(szTemp2, pobTran->srTRec.szTicketRefundDate, 4);
					
					inNCCC_TICKET_Decide_Year(szTemp, szTemp2);
					memcpy(&szPacket[inPacketCnt], szTemp, 4); 
					inPacketCnt += 4;
                                        
                                        memcpy(&szPacket[inPacketCnt], &pobTran->srTRec.szTicketRefundDate[0], 4); 
                                        inPacketCnt += 4;
                                }
                        }
                        
                        /* ●	Table ID “PS”: 電票交易資訊 */ 
                        memcpy(&szPacket[inPacketCnt], "PS", 2); /* Table ID */
                        inPacketCnt += 2;
                	/* Sub Total Length */
                	inPacketCnt ++;
                	szPacket[inPacketCnt] = 0x08; 	/* Sub-Data Total Length */
                	inPacketCnt ++;
               
                        /* 系統代碼 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetIPASS_System_ID(szTemplate);
                        memcpy(&szPacket[inPacketCnt], szTemplate, 2);    
                        inPacketCnt += 2;
                        
                        /* 業者代碼 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetIPASS_SP_ID(szTemplate);
                        memcpy(&szPacket[inPacketCnt], szTemplate, 2);    
                        inPacketCnt += 2;
                        
                        /* 次業者代碼 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetIPASS_Sub_Company_ID(szTemplate);
                        memcpy(&szPacket[inPacketCnt], szTemplate, 4);    
                        inPacketCnt += 4;
			
			/* 【需求單-108215】電票交易支援優惠平台需求 by Russell 2020/5/11 下午 3:53 */
                        if (pobTran->srTRec.inCode == _TICKET_IPASS_DEDUCT_ && (!memcmp(&guszTICKET_MTI[0], "0100", 4) || !memcmp(&guszTICKET_MTI[0], "0400", 4)))
                        {
                        	if (pobTran->inISOTxnCode != _ADVICE_ && inNCCC_Loyalty_ASM_Flag() == VS_SUCCESS)
                        	{
                        		/*
						優惠序號(Award S/N)
						註1. 本序號為唯一值，由端末機產生邏輯為TID(8Bytes)+YYYYMMDDhhmmss
						註2. 取消(含沖銷取消)須上傳原交易之優惠序號。
						註3. 主機回覆原端末機上傳之值。
					*/
					memset(pobTran->srTRec.szAwardSN, 0x00, sizeof(pobTran->srTRec.szAwardSN));
					memset(szTemplate, 0x00, sizeof(szTemplate));
					inGetTerminalID(szTemplate);
					strcpy(pobTran->srTRec.szAwardSN, szTemplate);
					
					memset(szTemplate, 0x00, sizeof(szTemplate));
					inFunc_GetSystemDateAndTime(&srRTC);
					sprintf(szTemplate, "20%02d%02d%02d%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay, srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
					memcpy(&pobTran->srTRec.szAwardSN[8], &szTemplate[0], 14);
					
					memcpy(&szPacket[inPacketCnt], "L1", 2); /* Table ID */
					inPacketCnt += 2;
					/* Sub Total Length */
					inPacketCnt ++;
					szPacket[inPacketCnt] = 0x23;
					inPacketCnt ++;
					
					/*
						列印優惠或廣告資訊之個數。
						‘0’=表示無優惠活動
						‘1’=表示僅有一個優惠活動
						‘2’=表示有兩個優惠活動
						註1. 若是只有一個優惠活動，則只需下載【第一個優惠活動】欄位的資料。每次下載之優惠活動不超過2個
						註2. 請求電文需傳送至優惠序號，預設上傳’0’。FES依此判斷端末機是否為支援優惠平台之端末機版本，FES需進行優惠查詢。
						註3. 回覆電文中，若本欄位=’0’，則僅回傳至優惠序號。
					*/
					szPacket[inPacketCnt] = '0';
					inPacketCnt ++;
					/*
						優惠序號(Award S/N)
						註1. 本序號為唯一值，由端末機產生邏輯為TID(8Bytes)+YYYYMMDDhhmmss
						註2. 取消(含沖銷取消)須上傳原交易之優惠序號。
						註3. 主機回覆原端末機上傳之值。
					*/
					memcpy(&szPacket[inPacketCnt], pobTran->srTRec.szAwardSN, 22);
					inPacketCnt += 22;
                        	}	
                        }
                }
                else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
                { 
                	/* 0500 部份和上面結帳交易重複，故移除 by Russell 2019/11/18 上午 10:03 */
			if (!memcmp(&guszTICKET_MTI[0], "0800", 4) || pobTran->inISOTxnCode == _TICKET_EASYCARD_REG_REV_)
                        {       
                                /* ●	Table ID “N3”: Host AD “YYYY” (系統西元年) */
                		memcpy(&szPacket[inPacketCnt], "N3", 2); /* Table ID */
                		inPacketCnt += 2;
                		/* Sub Total Length */
                		inPacketCnt ++;
                		szPacket[inPacketCnt] = 0x04;
                		inPacketCnt ++;
                		memcpy(&szPacket[inPacketCnt], "0000", 4); /* YYYY，上傳時，固定為'0000'，並由授權主機更新回傳 */
                		inPacketCnt += 4;
                
				/* (需求單 - 107276)自助交易標準做法 MFES雲端化 by Russell 2019/3/5 下午 4:39 */
				/* 
				 *  Table ID “ND”：FES ID (FES 請款代碼)
				 *  端末機依TMS設定，若NCCC_FES_MODE = "03"(MFES)，則本欄位填入TMS下載的FES_ID值。其餘NCCC_FES_MODE，則預設上傳3BYTE（空白）
				 */
				memset(szFESMode, 0x00, sizeof(szFESMode));
				inGetNCCCFESMode(szFESMode);
				memset(szCFESMode, 0x00, sizeof(szCFESMode));
				inGetCloud_MFES(szCFESMode);
				if (memcmp(szFESMode, _NCCC_03_MFES_MODE_, strlen(_NCCC_03_MFES_MODE_)) == 0)
				{
					if (memcmp(szCFESMode, "Y", strlen("Y")) == 0)
					{
						memcpy(&szPacket[inPacketCnt], "ND", 2); /* Table ID */
						inPacketCnt += 2;

						/* Sub Total Length */
						szPacket[inPacketCnt] = 0x00;
						inPacketCnt ++;
						szPacket[inPacketCnt] = 0x03;
						inPacketCnt ++;

						memset(szFES_ID, 0x00, sizeof(szFES_ID));
						inGetFES_ID(szFES_ID);

						/* 沒值就給三個空白，讓主機回30 */
						/* ATS判斷端末機上傳Table ID "NF" = CFES
						而Table ID = "ND"為3 Bytes(空白)時，需回覆Response code = 30 */
						if (strlen(szFES_ID) > 0)
						{
							memcpy(&szPacket[inPacketCnt], szFES_ID, 3);
						}
						else
						{
							memcpy(&szPacket[inPacketCnt], "   ", 3);
						}
						inPacketCnt += 3;
					}
					else
					{
						/* MFES不送ND */
					}
				}
				else
				{
					memcpy(&szPacket[inPacketCnt], "ND", 2); /* Table ID */
					inPacketCnt += 2;

					/* Sub Total Length */
					szPacket[inPacketCnt] = 0x00;
					inPacketCnt ++;
					szPacket[inPacketCnt] = 0x03;
					inPacketCnt ++;

					memcpy(&szPacket[inPacketCnt], "   ", 3);
					inPacketCnt += 3;
				}
				
                		/* ●	Table ID “NF”: FES Indicator */
                		memcpy(&szPacket[inPacketCnt], "NF", 2); /* Table ID */
                		inPacketCnt += 2;
                		/* Sub Total Length */
                		inPacketCnt ++;
                		szPacket[inPacketCnt] = 0x04;
                		inPacketCnt ++;
                		
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetNCCCFESMode(szTemplate);
				memset(szCFESMode, 0x00, sizeof(szCFESMode));
				inGetCloud_MFES(szCFESMode);
				if (memcmp(szTemplate, _NCCC_04_MPAS_MODE_, strlen(_NCCC_04_MPAS_MODE_)) == 0)
				{
					memcpy(&szPacket[inPacketCnt], "MPAS", 4);
				}
				else if (memcmp(szFESMode, _NCCC_03_MFES_MODE_, strlen(_NCCC_03_MFES_MODE_)) == 0)
				{
					if (memcmp(szCFESMode, "Y", strlen("Y")) == 0)
					{
						memcpy(&szPacket[inPacketCnt], "CFES", 4);
					}
					else
					{
						memcpy(&szPacket[inPacketCnt], "MF  ", 4);
					}
				}
				else
				{
					memcpy(&szPacket[inPacketCnt], "ATS ", 4);
				}
                		inPacketCnt += 4;
                        }
                        else
                        {	
                                /* ●	Table ID “N1”: UnionPay (原CUP)交易 */
        			memset(szTemplate, 0x00, sizeof(szTemplate));
        			memset(szPacket, 0x00, sizeof(szPacket));
        			strcpy(szTemplate, "000000000000");
        			/* UnionPay(原CUP) data information */
        			memcpy(&szPacket[inPacketCnt], "N1", 2); /* Table ID */
        			inPacketCnt += 2;
        			/* Sub Total Length */
        			inPacketCnt ++;
        			szPacket[inPacketCnt] = 0x02;
        			inPacketCnt ++;
        			/*
        				Function number.
        				The terminal support UnionPay (原CUP) function.
        				0 = Normal.
        				1 = Support.
        			*/
        			szPacket[inPacketCnt] = '1';
        			inPacketCnt ++;
        			/*
        				Acquire Indicator.
        				Terminal hot key control.
        				0 = Normal.
        			*/
        			szPacket[inPacketCnt] = '0';
        			inPacketCnt ++;
        			
        			/*
                			●	Table ID “N2”: Settlement Flag
                			Settlement Flag
                			依TMS下載的Settlement Flag填入此欄位
                			1=”ST” (ATS清算)
                			2=”AU” (ATS不清算)
                		*/
        			memcpy(&szPacket[inPacketCnt], "N2", 2); /* Table ID */
        			inPacketCnt += 2;
        			/* Sub Total Length */
        			inPacketCnt ++;
        			szPacket[inPacketCnt] = 0x02;
        			inPacketCnt ++;
        
        			/*
        				吳升文(Angus Wu) <angus.wu@nccc.com.tw> 2014-05-26 (週一) PM 03:43 通知
        				ATS針對Table ID “N2” Settlement Flag的值，請調整為當收到TMS客製化參數042(建設公司)及043(保險公司)時，填入”AU”。
        				其餘填入”ST”。
        			*/
        			memcpy(&szPacket[inPacketCnt], "ST", 2);
        			inPacketCnt += 2;
        			
        			/*
                			●	Table ID “N3”: Host AD “YYYY” (系統西元年)
                			1. 為解決ISO8583無法更新西元年的問題，故增加此Table ID N3，端末機可依此欄位更新端末機系統年份(西元)。
                			2. 避免端末機因Clock故障或是跨年的秒差造成交易年份錯誤。
                		*/
        			if (memcmp(guszTICKET_MTI, "0220", strlen("0220")) == 0)
				{
					/* 0220不帶N3 */
				}
				else
				{
					memcpy(&szPacket[inPacketCnt], "N3", 2); /* Table ID */
					inPacketCnt += 2;
					/* Sub Total Length */
					inPacketCnt ++;
					szPacket[inPacketCnt] = 0x04;
					inPacketCnt ++;
					memcpy(&szPacket[inPacketCnt], "0000", 4); /* YYYY，上傳時，固定為'0000'，並由授權主機更新回傳 */
					inPacketCnt += 4;
				}
        			
				/* (需求單 - 107276)自助交易標準做法 MFES雲端化 by Russell 2019/3/5 下午 4:39 */
				/* 
				 *  Table ID “ND”：FES ID (FES 請款代碼)
				 *  端末機依TMS設定，若NCCC_FES_MODE = "03"(MFES)，則本欄位填入TMS下載的FES_ID值。其餘NCCC_FES_MODE，則預設上傳3BYTE（空白）
				 */
				memset(szFESMode, 0x00, sizeof(szFESMode));
				inGetNCCCFESMode(szFESMode);
				memset(szCFESMode, 0x00, sizeof(szCFESMode));
				inGetCloud_MFES(szCFESMode);
				if (memcmp(szFESMode, _NCCC_03_MFES_MODE_, strlen(_NCCC_03_MFES_MODE_)) == 0)
				{
					if (memcmp(szCFESMode, "Y", strlen("Y")) == 0)
					{
						memcpy(&szPacket[inPacketCnt], "ND", 2); /* Table ID */
						inPacketCnt += 2;

						/* Sub Total Length */
						szPacket[inPacketCnt] = 0x00;
						inPacketCnt ++;
						szPacket[inPacketCnt] = 0x03;
						inPacketCnt ++;

						memset(szFES_ID, 0x00, sizeof(szFES_ID));
						inGetFES_ID(szFES_ID);

						/* 沒值就給三個空白，讓主機回30 */
						/* ATS判斷端末機上傳Table ID "NF" = CFES
						而Table ID = "ND"為3 Bytes(空白)時，需回覆Response code = 30 */
						if (strlen(szFES_ID) > 0)
						{
							memcpy(&szPacket[inPacketCnt], szFES_ID, 3);
						}
						else
						{
							memcpy(&szPacket[inPacketCnt], "   ", 3);
						}
						inPacketCnt += 3;
					}
					else
					{
						/* MFES不送ND */
					}
				}
				else
				{
					memcpy(&szPacket[inPacketCnt], "ND", 2); /* Table ID */
					inPacketCnt += 2;

					/* Sub Total Length */
					szPacket[inPacketCnt] = 0x00;
					inPacketCnt ++;
					szPacket[inPacketCnt] = 0x03;
					inPacketCnt ++;

					memcpy(&szPacket[inPacketCnt], "   ", 3);
					inPacketCnt += 3;
				}
				
        			/*
                			●	Table ID “NF”: FES Indicator
                			依TMS Config.txt中NCCC_FES_Mode參數設定上傳值。
                			當NCCC_FES_Mode=03時，表示此為MFES端末機，本欄位值為”MFxx”。
                			(端末機預設xx=0x20 0x20。xx由MFES填入新值。
                			當MFES已經進行基本查核後SAF的交易，ATS不需再查，則MFES填入xx=”SF”。
                			反之則維持0x20,0x20)
                			當NCCC_FES_Mode=05時，表示此非MFES端末機，本欄位值為”ATS”。(表0x20)
                			ATS以此判斷若為”MFES”端末機，則ATS不產生該端末機的請款檔。
                		*/
                		memcpy(&szPacket[inPacketCnt], "NF", 2); /* Table ID */
                		inPacketCnt += 2;
                		/* Sub Total Length */
                		inPacketCnt ++;
                		szPacket[inPacketCnt] = 0x04;
                		inPacketCnt ++;
				
                		memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetNCCCFESMode(szTemplate);
				memset(szCFESMode, 0x00, sizeof(szCFESMode));
				inGetCloud_MFES(szCFESMode);
				
				if (memcmp(szTemplate, _NCCC_04_MPAS_MODE_, strlen(_NCCC_04_MPAS_MODE_)) == 0)
				{
					memcpy(&szPacket[inPacketCnt], "MPAS", 4);
				}
				else if (memcmp(szFESMode, _NCCC_03_MFES_MODE_, strlen(_NCCC_03_MFES_MODE_)) == 0)
				{
					if (memcmp(szCFESMode, "Y", strlen("Y")) == 0)
					{
						memcpy(&szPacket[inPacketCnt], "CFES", 4);
					}
					else
					{
						memcpy(&szPacket[inPacketCnt], "MF  ", 4);
					}
				}
				else
				{
					memcpy(&szPacket[inPacketCnt], "ATS ", 4);
				}
                		inPacketCnt += 4;
                		
                		/* 
                		        ●	Table ID “ET”: 電票交易資訊 
                		*/ 
                                memcpy(&szPacket[inPacketCnt], "ET", 2); /* Table ID */
                                inPacketCnt += 2;
                        	/* Sub Total Length */
                        	inPacketCnt ++;
                        	szPacket[inPacketCnt] = 0x18; 	/* Sub-Data Total Length */
                        	inPacketCnt ++;
				/* 加2是因為要加上F59總長度 */
				ginECC_F59_ET_Len += inPacketCnt + 2;
				
                                memcpy(&szPacket[inPacketCnt], "                  ", 18);           
                                inPacketCnt += 18;
				
				/* 組MAC使用 */
				memset(gszMAC_F_59, 0x00, sizeof(gszMAC_F_59));
				memcpy(gszMAC_F_59, &szPacket[inPacketCnt - 18], 18);
                        
                                /* 
                		        ●	Table ID “ES”: 電票交易序號 
                		*/ 
                                memcpy(&szPacket[inPacketCnt], "ES", 2); /* Table ID */
                                inPacketCnt += 2;
                        	/* Sub Total Length */
                        	inPacketCnt ++;
                        	szPacket[inPacketCnt] = 0x06; 	/* Sub-Data Total Length */
                        	inPacketCnt ++;
                        	
                        	/* RF序號 */      
                                memcpy(&szPacket[inPacketCnt], pobTran->srTRec.szTicketRefundCode, 6);           
                                inPacketCnt += 6;
                                
				/* 
					●	Table ID “ED”: 電票原交易日期 
				*/ 
                                if (pobTran->srTRec.inCode == _TICKET_EASYCARD_REFUND_ && 
				   (!memcmp(&guszTICKET_MTI[0], "0100", 4) || !memcmp(&guszTICKET_MTI[0], "0400", 4)))
                                {        
                                        
                                        memcpy(&szPacket[inPacketCnt], "ED", 2); /* Table ID */
                                        inPacketCnt += 2;
                                	/* Sub Total Length */
                                	inPacketCnt ++;
                                	szPacket[inPacketCnt] = 0x08; 	/* Sub-Data Total Length */
                                	inPacketCnt ++;
                                	
                                	/* 原交易日期 - 輸入日期大於目前日期，判定為去年 */ 
					memset(szTemp, 0x00, sizeof(szTemp));
					memset(szTemp2, 0x00, sizeof(szTemp2));
					/* [20251219_BUG_MDF][ECC] 修改電票退貨的判斷日期,由 2位MM改為 4位 MMDD */
					memcpy(szTemp2, pobTran->srTRec.szTicketRefundDate, 4);
					
					inNCCC_TICKET_Decide_Year(szTemp, szTemp2);
					memcpy(&szPacket[inPacketCnt], szTemp, 4); 
					inPacketCnt += 4;
                                        
                                        memcpy(&szPacket[inPacketCnt], &pobTran->srTRec.szTicketRefundDate[0], 4); 
                                        inPacketCnt += 4;
                                }
                        }
			
			/* 【需求單-108215】電票交易支援優惠平台需求 by Russell 2020/5/11 下午 3:53 */
                        if (pobTran->srTRec.inCode == _TICKET_EASYCARD_DEDUCT_ && (!memcmp(&guszTICKET_MTI[0], "0100", 4) || !memcmp(&guszTICKET_MTI[0], "0400", 4)))
                        {
                        	if (pobTran->inISOTxnCode != _ADVICE_ && inNCCC_Loyalty_ASM_Flag() == VS_SUCCESS)
                        	{
                        		/*
						優惠序號(Award S/N)
						註1. 本序號為唯一值，由端末機產生邏輯為TID(8Bytes)+YYYYMMDDhhmmss
						註2. 取消(含沖銷取消)須上傳原交易之優惠序號。
						註3. 主機回覆原端末機上傳之值。
					*/
					memset(pobTran->srTRec.szAwardSN, 0x00, sizeof(pobTran->srTRec.szAwardSN));
					memset(szTemplate, 0x00, sizeof(szTemplate));
					inGetTerminalID(szTemplate);
					strcpy(pobTran->srTRec.szAwardSN, szTemplate);
					
					memset(szTemplate, 0x00, sizeof(szTemplate));
					inFunc_GetSystemDateAndTime(&srRTC);
					sprintf(szTemplate, "20%02d%02d%02d%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay, srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
					memcpy(&pobTran->srTRec.szAwardSN[8], &szTemplate[0], 14);
					
					memcpy(&szPacket[inPacketCnt], "L1", 2); /* Table ID */
					inPacketCnt += 2;
					/* Sub Total Length */
					inPacketCnt ++;
					szPacket[inPacketCnt] = 0x23;
					inPacketCnt ++;
					
					/*
						列印優惠或廣告資訊之個數。
						‘0’=表示無優惠活動
						‘1’=表示僅有一個優惠活動
						‘2’=表示有兩個優惠活動
						註1. 若是只有一個優惠活動，則只需下載【第一個優惠活動】欄位的資料。每次下載之優惠活動不超過2個
						註2. 請求電文需傳送至優惠序號，預設上傳’0’。FES依此判斷端末機是否為支援優惠平台之端末機版本，FES需進行優惠查詢。
						註3. 回覆電文中，若本欄位=’0’，則僅回傳至優惠序號。
					*/
					szPacket[inPacketCnt] = '0';
					inPacketCnt ++;
					/*
						優惠序號(Award S/N)
						註1. 本序號為唯一值，由端末機產生邏輯為TID(8Bytes)+YYYYMMDDhhmmss
						註2. 取消(含沖銷取消)須上傳原交易之優惠序號。
						註3. 主機回覆原端末機上傳之值。
					*/
					memcpy(&szPacket[inPacketCnt], pobTran->srTRec.szAwardSN, 22);
					inPacketCnt += 22;
                        	}	
                        }
                }
		else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
                { 
                	if (!memcmp(&guszTICKET_MTI[0], "0800", 4))
                        {
                                /* ●	Table ID “N3”: Host AD “YYYY” (系統西元年) */
                		memcpy(&szPacket[inPacketCnt], "N3", 2); /* Table ID */
                		inPacketCnt += 2;
                		/* Sub Total Length */
                		inPacketCnt ++;
                		szPacket[inPacketCnt] = 0x04;
                		inPacketCnt ++;
                		memcpy(&szPacket[inPacketCnt], "0000", 4); /* YYYY，上傳時，固定為'0000'，並由授權主機更新回傳 */
                		inPacketCnt += 4;
                
				/* (需求單 - 107276)自助交易標準做法 MFES雲端化 by Russell 2019/3/5 下午 4:39 */
				/* 
				 *  Table ID “ND”：FES ID (FES 請款代碼)
				 *  端末機依TMS設定，若NCCC_FES_MODE = "03"(MFES)，則本欄位填入TMS下載的FES_ID值。其餘NCCC_FES_MODE，則預設上傳3BYTE（空白）
				 */
				memset(szFESMode, 0x00, sizeof(szFESMode));
				inGetNCCCFESMode(szFESMode);
				memset(szCFESMode, 0x00, sizeof(szCFESMode));
				inGetCloud_MFES(szCFESMode);
				if (memcmp(szFESMode, _NCCC_03_MFES_MODE_, strlen(_NCCC_03_MFES_MODE_)) == 0)
				{
					if (memcmp(szCFESMode, "Y", strlen("Y")) == 0)
					{
						memcpy(&szPacket[inPacketCnt], "ND", 2); /* Table ID */
						inPacketCnt += 2;

						/* Sub Total Length */
						szPacket[inPacketCnt] = 0x00;
						inPacketCnt ++;
						szPacket[inPacketCnt] = 0x03;
						inPacketCnt ++;

						memset(szFES_ID, 0x00, sizeof(szFES_ID));
						inGetFES_ID(szFES_ID);

						/* 沒值就給三個空白，讓主機回30 */
						/* ATS判斷端末機上傳Table ID "NF" = CFES
						而Table ID = "ND"為3 Bytes(空白)時，需回覆Response code = 30 */
						if (strlen(szFES_ID) > 0)
						{
							memcpy(&szPacket[inPacketCnt], szFES_ID, 3);
						}
						else
						{
							memcpy(&szPacket[inPacketCnt], "   ", 3);
						}
						inPacketCnt += 3;
					}
					else
					{
						/* MFES不送ND */
					}
				}
				else
				{
					memcpy(&szPacket[inPacketCnt], "ND", 2); /* Table ID */
					inPacketCnt += 2;

					/* Sub Total Length */
					szPacket[inPacketCnt] = 0x00;
					inPacketCnt ++;
					szPacket[inPacketCnt] = 0x03;
					inPacketCnt ++;

					memcpy(&szPacket[inPacketCnt], "   ", 3);
					inPacketCnt += 3;
				}
				
                		/* ●	Table ID “NF”: FES Indicator */
                		memcpy(&szPacket[inPacketCnt], "NF", 2); /* Table ID */
                		inPacketCnt += 2;
                		/* Sub Total Length */
                		inPacketCnt ++;
                		szPacket[inPacketCnt] = 0x04;
                		inPacketCnt ++;
				
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetNCCCFESMode(szTemplate);
				memset(szCFESMode, 0x00, sizeof(szCFESMode));
				inGetCloud_MFES(szCFESMode);
				
				if (memcmp(szTemplate, _NCCC_04_MPAS_MODE_, strlen(_NCCC_04_MPAS_MODE_)) == 0)
				{
					memcpy(&szPacket[inPacketCnt], "MPAS", 4);
				}
				else if (memcmp(szFESMode, _NCCC_03_MFES_MODE_, strlen(_NCCC_03_MFES_MODE_)) == 0)
				{
					if (memcmp(szCFESMode, "Y", strlen("Y")) == 0)
					{
						memcpy(&szPacket[inPacketCnt], "CFES", 4);
					}
					else
					{
						memcpy(&szPacket[inPacketCnt], "MF  ", 4);
					}
				}
				else
				{
					memcpy(&szPacket[inPacketCnt], "ATS ", 4);
				}
                		inPacketCnt += 4;
                        }
                        else
                        {	
                                /* ●	Table ID “N1”: UnionPay (原CUP)交易 */
        			memset(szPacket, 0x00, sizeof(szPacket));
        			/* UnionPay(原CUP) data information */
        			memcpy(&szPacket[inPacketCnt], "N1", 2); /* Table ID */
        			inPacketCnt += 2;
        			/* Sub Total Length */
        			inPacketCnt ++;
        			szPacket[inPacketCnt] = 0x02;
        			inPacketCnt ++;
        			/*
        				Function number.
        				The terminal support UnionPay (原CUP) function.
        				0 = Normal.
        				1 = Support.
        			*/
        			szPacket[inPacketCnt] = '1';
        			inPacketCnt ++;
        			/*
        				Acquire Indicator.
        				Terminal hot key control.
        				0 = Normal.
        			*/
        			szPacket[inPacketCnt] = '0';
        			inPacketCnt ++;
        			
        			/*
                			●	Table ID “N2”: Settlement Flag
                			Settlement Flag
                			依TMS下載的Settlement Flag填入此欄位
                			1=”ST” (ATS清算)
                			2=”AU” (ATS不清算)
                		*/
        			memcpy(&szPacket[inPacketCnt], "N2", 2); /* Table ID */
        			inPacketCnt += 2;
        			/* Sub Total Length */
        			inPacketCnt ++;
        			szPacket[inPacketCnt] = 0x02;
        			inPacketCnt ++;
        
        			/*
        				吳升文(Angus Wu) <angus.wu@nccc.com.tw> 2014-05-26 (週一) PM 03:43 通知
        				ATS針對Table ID “N2” Settlement Flag的值，請調整為當收到TMS客製化參數042(建設公司)及043(保險公司)時，填入”AU”。
        				其餘填入”ST”。
        			*/
        			memcpy(&szPacket[inPacketCnt], "ST", 2);
        			inPacketCnt += 2;
        			
        			/*
                			●	Table ID “N3”: Host AD “YYYY” (系統西元年)
                			1. 為解決ISO8583無法更新西元年的問題，故增加此Table ID N3，端末機可依此欄位更新端末機系統年份(西元)。
                			2. 避免端末機因Clock故障或是跨年的秒差造成交易年份錯誤。
                		*/
				if (memcmp(guszTICKET_MTI, "0220", strlen("0220")) == 0)
				{
					/* 0220不帶N3 */
				}
				else
				{
					memcpy(&szPacket[inPacketCnt], "N3", 2); /* Table ID */
					inPacketCnt += 2;
					/* Sub Total Length */
					inPacketCnt ++;
					szPacket[inPacketCnt] = 0x04;
					inPacketCnt ++;
					memcpy(&szPacket[inPacketCnt], "0000", 4); /* YYYY，上傳時，固定為'0000'，並由授權主機更新回傳 */
					inPacketCnt += 4;
				}
        			
				/* (需求單 - 107276)自助交易標準做法 MFES雲端化 by Russell 2019/3/5 下午 4:39 */
				/* 
				 *  Table ID “ND”：FES ID (FES 請款代碼)
				 *  端末機依TMS設定，若NCCC_FES_MODE = "03"(MFES)，則本欄位填入TMS下載的FES_ID值。其餘NCCC_FES_MODE，則預設上傳3BYTE（空白）
				 */
				memset(szFESMode, 0x00, sizeof(szFESMode));
				inGetNCCCFESMode(szFESMode);
				memset(szCFESMode, 0x00, sizeof(szCFESMode));
				inGetCloud_MFES(szCFESMode);
				if (memcmp(szFESMode, _NCCC_03_MFES_MODE_, strlen(_NCCC_03_MFES_MODE_)) == 0)
				{
					if (memcmp(szCFESMode, "Y", strlen("Y")) == 0)
					{
						memcpy(&szPacket[inPacketCnt], "ND", 2); /* Table ID */
						inPacketCnt += 2;

						/* Sub Total Length */
						szPacket[inPacketCnt] = 0x00;
						inPacketCnt ++;
						szPacket[inPacketCnt] = 0x03;
						inPacketCnt ++;

						memset(szFES_ID, 0x00, sizeof(szFES_ID));
						inGetFES_ID(szFES_ID);

						/* 沒值就給三個空白，讓主機回30 */
						/* ATS判斷端末機上傳Table ID "NF" = CFES
						而Table ID = "ND"為3 Bytes(空白)時，需回覆Response code = 30 */
						if (strlen(szFES_ID) > 0)
						{
							memcpy(&szPacket[inPacketCnt], szFES_ID, 3);
						}
						else
						{
							memcpy(&szPacket[inPacketCnt], "   ", 3);
						}
						inPacketCnt += 3;
					}
					else
					{
						/* MFES不送ND */
					}
				}
				else
				{
					memcpy(&szPacket[inPacketCnt], "ND", 2); /* Table ID */
					inPacketCnt += 2;

					/* Sub Total Length */
					szPacket[inPacketCnt] = 0x00;
					inPacketCnt ++;
					szPacket[inPacketCnt] = 0x03;
					inPacketCnt ++;

					memcpy(&szPacket[inPacketCnt], "   ", 3);
					inPacketCnt += 3;
				}
			       
        			/*
                			●	Table ID “NF”: FES Indicator
                			依TMS Config.txt中NCCC_FES_Mode參數設定上傳值。
                			當NCCC_FES_Mode=03時，表示此為MFES端末機，本欄位值為”MFxx”。
                			(端末機預設xx=0x20 0x20。xx由MFES填入新值。
                			當MFES已經進行基本查核後SAF的交易，ATS不需再查，則MFES填入xx=”SF”。
                			反之則維持0x20,0x20)
                			當NCCC_FES_Mode=05時，表示此非MFES端末機，本欄位值為”ATS”。(表0x20)
                			ATS以此判斷若為”MFES”端末機，則ATS不產生該端末機的請款檔。
                		*/
                		memcpy(&szPacket[inPacketCnt], "NF", 2); /* Table ID */
                		inPacketCnt += 2;
                		/* Sub Total Length */
                		inPacketCnt ++;
                		szPacket[inPacketCnt] = 0x04;
                		inPacketCnt ++;
				
                		memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetNCCCFESMode(szTemplate);
				memset(szCFESMode, 0x00, sizeof(szCFESMode));
				inGetCloud_MFES(szCFESMode);
				
				if (memcmp(szTemplate, _NCCC_04_MPAS_MODE_, strlen(_NCCC_04_MPAS_MODE_)) == 0)
				{
					memcpy(&szPacket[inPacketCnt], "MPAS", 4);
				}
				else if (memcmp(szFESMode, _NCCC_03_MFES_MODE_, strlen(_NCCC_03_MFES_MODE_)) == 0)
				{
					if (memcmp(szCFESMode, "Y", strlen("Y")) == 0)
					{
						memcpy(&szPacket[inPacketCnt], "CFES", 4);
					}
					else
					{
						memcpy(&szPacket[inPacketCnt], "MF  ", 4);
					}
				}
				else
				{
					memcpy(&szPacket[inPacketCnt], "ATS ", 4);
				}
                		inPacketCnt += 4;
        				
                		/* 
                		        ●	Table ID “ET”: 電票交易資訊 
                		*/ 
                                memcpy(&szPacket[inPacketCnt], "ET", 2); /* Table ID */
                                inPacketCnt += 2;
                        	/* Sub Total Length */
                        	inPacketCnt ++;
                        	szPacket[inPacketCnt] = 0x18; 	/* Sub-Data Total Length */
                        	inPacketCnt ++;
                        	
                        	/* Before Amt */
                        	if (pobTran->srTRec.lnCardRemainAmount < 0)
                        	{      
                        	        szPacket[inPacketCnt] = 0x2D;  
                        	        inPacketCnt ++;
                        	        memset(szTemplate, 0x00, sizeof(szTemplate)); 
                        	        sprintf(szTemplate, "%08ld", (0 - pobTran->srTRec.lnCardRemainAmount));
                        	}
                        	else     
                        	{
                        	        szPacket[inPacketCnt] = 0x20; 
                        	        inPacketCnt ++;
                        	        memset(szTemplate, 0x00, sizeof(szTemplate));    
                                        sprintf(szTemplate, "%08ld", pobTran->srTRec.lnCardRemainAmount);
                                }
                                          
                                memcpy(&szPacket[inPacketCnt], &szTemplate[0], 8);           
                                inPacketCnt += 8;
                                
                                /* After Amt */
                                if (pobTran->srTRec.inCode == _TICKET_ICASH_DEDUCT_)
                                        lnAmt = pobTran->srTRec.lnCardRemainAmount - pobTran->srTRec.lnTxnAmount;
                                else if (pobTran->srTRec.inCode == _TICKET_ICASH_AUTO_TOP_UP_)
                                        lnAmt = pobTran->srTRec.lnCardRemainAmount + pobTran->srTRec.lnTotalTopUpAmount;    
                                else if (pobTran->srTRec.inCode == _TICKET_ICASH_TOP_UP_)
                                        lnAmt = pobTran->srTRec.lnCardRemainAmount + pobTran->srTRec.lnTxnAmount;              
                                else if (pobTran->srTRec.inCode == _TICKET_ICASH_VOID_TOP_UP_)
                                        lnAmt = pobTran->srTRec.lnCardRemainAmount - pobTran->srTRec.lnTxnAmount;                 
                                else              
                                        lnAmt = pobTran->srTRec.lnCardRemainAmount + pobTran->srTRec.lnTxnAmount;
                                
                                if (lnAmt < 0)
                        	{
                        	        szPacket[inPacketCnt] = '-';  
                        	        inPacketCnt ++;
                        	        memset(szTemplate, 0x00, sizeof(szTemplate)); 
                        	        sprintf(szTemplate, "%08ld", (0 - lnAmt));
                        	}
                        	else     
                        	{
                        	        szPacket[inPacketCnt] = ' '; 
                        	        inPacketCnt ++;
                        	        memset(szTemplate, 0x00, sizeof(szTemplate));    
                                        sprintf(szTemplate, "%08ld", lnAmt);
                                }
                                
                                memcpy(&szPacket[inPacketCnt], &szTemplate[0], 8);           
                                inPacketCnt += 8;
				
				/* 組MAC使用 */
				memset(gszMAC_F_59, 0x00, sizeof(gszMAC_F_59));
				memcpy(gszMAC_F_59, &szPacket[inPacketCnt - 18], 18);
                                
                                /* 
                		        ●	Table ID “ES”: 電票交易序號 
                		*/ 
                                memcpy(&szPacket[inPacketCnt], "ES", 2); /* Table ID */
                                inPacketCnt += 2;
                        	/* Sub Total Length */
                        	inPacketCnt ++;
                        	szPacket[inPacketCnt] = 0x06; 	/* Sub-Data Total Length */
                        	inPacketCnt ++;
                        	
                        	/* RF序號 */      
                                memcpy(&szPacket[inPacketCnt], &pobTran->srTRec.szTicketRefundCode[0], 6);           
                                inPacketCnt += 6;
                                
				/* 
					●	Table ID “ED”: 電票原交易日期 
				*/ 
                                if (pobTran->srTRec.inCode == _TICKET_ICASH_REFUND_ && 
				   (!memcmp(&guszTICKET_MTI[0], "0100", 4)	||
				    !memcmp(&guszTICKET_MTI[0], "0102", 4)))
                                {        
                                        
                                        memcpy(&szPacket[inPacketCnt], "ED", 2); /* Table ID */
                                        inPacketCnt += 2;
                                	/* Sub Total Length */
                                	inPacketCnt ++;
                                	szPacket[inPacketCnt] = 0x08; 	/* Sub-Data Total Length */
                                	inPacketCnt ++;
                                	
                                	/* 原交易日期 - 輸入日期大於目前日期，判定為去年 */ 
					memset(szTemp, 0x00, sizeof(szTemp));
					memset(szTemp2, 0x00, sizeof(szTemp2));
					/* [20251219_BUG_MDF][ECC] 修改電票退貨的判斷日期,由 2位MM改為 4位 MMDD */
					memcpy(szTemp2, pobTran->srTRec.szTicketRefundDate, 4);
					
					inNCCC_TICKET_Decide_Year(szTemp, szTemp2);
					memcpy(&szPacket[inPacketCnt], szTemp, 4); 
					inPacketCnt += 4;
                                        
                                        memcpy(&szPacket[inPacketCnt], &pobTran->srTRec.szTicketRefundDate[0], 4); 
                                        inPacketCnt += 4;
                                }
                        }
			
			/* 【需求單-108215】電票交易支援優惠平台需求 by Russell 2020/5/11 下午 3:53 */
                        if (pobTran->srTRec.inCode == _TICKET_ICASH_DEDUCT_ && (!memcmp(&guszTICKET_MTI[0], "0100", 4) || !memcmp(&guszTICKET_MTI[0], "0400", 4)))
                        {
                        	if (pobTran->inISOTxnCode != _ADVICE_ && inNCCC_Loyalty_ASM_Flag() == VS_SUCCESS)
                        	{
                        		/*
						優惠序號(Award S/N)
						註1. 本序號為唯一值，由端末機產生邏輯為TID(8Bytes)+YYYYMMDDhhmmss
						註2. 取消(含沖銷取消)須上傳原交易之優惠序號。
						註3. 主機回覆原端末機上傳之值。
					*/
					memset(pobTran->srTRec.szAwardSN, 0x00, sizeof(pobTran->srTRec.szAwardSN));
					memset(szTemplate, 0x00, sizeof(szTemplate));
					inGetTerminalID(szTemplate);
					strcpy(pobTran->srTRec.szAwardSN, szTemplate);
					
					memset(szTemplate, 0x00, sizeof(szTemplate));
					inFunc_GetSystemDateAndTime(&srRTC);
					sprintf(szTemplate, "20%02d%02d%02d%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay, srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
					memcpy(&pobTran->srTRec.szAwardSN[8], &szTemplate[0], 14);
					
					memcpy(&szPacket[inPacketCnt], "L1", 2); /* Table ID */
					inPacketCnt += 2;
					/* Sub Total Length */
					inPacketCnt ++;
					szPacket[inPacketCnt] = 0x23;
					inPacketCnt ++;
					
					/*
						列印優惠或廣告資訊之個數。
						‘0’=表示無優惠活動
						‘1’=表示僅有一個優惠活動
						‘2’=表示有兩個優惠活動
						註1. 若是只有一個優惠活動，則只需下載【第一個優惠活動】欄位的資料。每次下載之優惠活動不超過2個
						註2. 請求電文需傳送至優惠序號，預設上傳’0’。FES依此判斷端末機是否為支援優惠平台之端末機版本，FES需進行優惠查詢。
						註3. 回覆電文中，若本欄位=’0’，則僅回傳至優惠序號。
					*/
					szPacket[inPacketCnt] = '0';
					inPacketCnt ++;
					/*
						優惠序號(Award S/N)
						註1. 本序號為唯一值，由端末機產生邏輯為TID(8Bytes)+YYYYMMDDhhmmss
						註2. 取消(含沖銷取消)須上傳原交易之優惠序號。
						註3. 主機回覆原端末機上傳之值。
					*/
					memcpy(&szPacket[inPacketCnt], pobTran->srTRec.szAwardSN, 22);
					inPacketCnt += 22;
                        	}	
                        }
        	}
        }

	/* Packet Data Length */
	memset(szASCII, 0x00, sizeof(szASCII));
	sprintf(szASCII, "%04d", inPacketCnt);
	memset(uszBCD, 0x00, sizeof(uszBCD));
	inFunc_ASCII_to_BCD(uszBCD, szASCII, 2);
	memcpy((char *)&uszPackBuf[inCnt], &uszBCD[0], 2);
	inCnt += 2;
	/* Packet Data */
	memcpy((char *)&uszPackBuf[inCnt], &szPacket[0], inPacketCnt);
	inCnt += inPacketCnt;
	
	/* 計算位置用 */
	if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
	{
		ginECC_F63_Len += inCnt;
	}

	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_ATS_Pack59() END!");
	
        return (inCnt);
}

/*
Function        :inNCCC_TICKET_MFES_Pack59
Date&Time       :2018/1/4 下午 4:40
Describe        : by bai
		Field_59:	Reserved-Private Data
		  SEQ   |  Position(s)	|  Description
		----------------------------------------------------------------------------------------------
		   1	|	1	|    1-2	Field total length.
		----------------------------------------------------------------------------------------------
		   2	|	2	|    3-4	Table ID
		----------------------------------------------------------------------------------------------
		   3	|	3	|    5-6	First sub-element length; the value of the “length”
		   					sub-field is always one;. BCD length for
							範例:20 bytes  0x00 0x20
		----------------------------------------------------------------------------------------------
		   4		4	|    6..750	Table ID Data (ISO8583封包總長度不可超過999 bytes)

*/
int inNCCC_TICKET_MFES_Pack59(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int		inCnt = 0, inPacketCnt = 0;
	LONG		lnAmt = 0;
	char		szASCII[4 + 1] = {0};
	char		szTemplate[100 + 1] = {0}, szPacket[768 + 1] = {0};
	char		szTemp[8 + 1] = {0};
	char		szTemp2[8 + 1] = {0};
//	char		szFESMode[2 + 1] = {0};
//	char		szFES_ID[3 + 1] = {0};
	unsigned char	uszBCD[2 + 1] = {0};
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_MFES_Pack59() START!");

	memset(szPacket, 0x00, sizeof(szPacket));

	if ((pobTran->inTransactionCode == _SETTLE_ || pobTran->inTransactionCode == _CLS_SETTLE_) && pobTran->inISOTxnCode != _ADVICE_)
        {
		
        }
        else
        {
        	if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
                {
			/* 0500 部份和上面結帳交易重複，故移除 by Russell 2019/12/2 上午 10:15 */
                	if (!memcmp(&guszTICKET_MTI[0], "0800", 4))
                        {
                        }
                        else
                        {	
                                /* ●	Table ID “N1”: UnionPay (原CUP)交易 */
        			memset(szPacket, 0x00, sizeof(szPacket));
        			/* UnionPay(原CUP) data information */
        			memcpy(&szPacket[inPacketCnt], "N1", 2); /* Table ID */
        			inPacketCnt += 2;
        			/* Sub Total Length */
        			inPacketCnt ++;
        			szPacket[inPacketCnt] = 0x02;
        			inPacketCnt ++;
        			/*
        				Function number.
        				The terminal support UnionPay (原CUP) function.
        				0 = Normal.
        				1 = Support.
        			*/
        			szPacket[inPacketCnt] = '1';
        			inPacketCnt ++;
        			/*
        				Acquire Indicator.
        				Terminal hot key control.
        				0 = Normal.
        			*/
        			szPacket[inPacketCnt] = '0';
        			inPacketCnt ++;
        			
                		/* 
                		        ●	Table ID “ET”: 電票交易資訊 
                		*/ 
                                memcpy(&szPacket[inPacketCnt], "ET", 2); /* Table ID */
                                inPacketCnt += 2;
                        	/* Sub Total Length */
                        	inPacketCnt ++;
                        	szPacket[inPacketCnt] = 0x18; 	/* Sub-Data Total Length */
                        	inPacketCnt ++;
                        	
                        	/* Before Amt */
                        	if (pobTran->srTRec.lnCardRemainAmount < 0)
                        	{      
                        	        szPacket[inPacketCnt] = 0x2D;  
                        	        inPacketCnt ++;
                        	        memset(szTemplate, 0x00, sizeof(szTemplate)); 
                        	        sprintf(szTemplate, "%08ld", (0 - pobTran->srTRec.lnCardRemainAmount));
                        	}
                        	else     
                        	{
                        	        szPacket[inPacketCnt] = 0x20; 
                        	        inPacketCnt ++;
                        	        memset(szTemplate, 0x00, sizeof(szTemplate));    
                                        sprintf(szTemplate, "%08ld", pobTran->srTRec.lnCardRemainAmount);
                                }
                                          
                                memcpy(&szPacket[inPacketCnt], &szTemplate[0], 8);           
                                inPacketCnt += 8;
                                
                                /* After Amt */
                                if (pobTran->srTRec.inCode == _TICKET_IPASS_QUERY_)
                                        lnAmt = pobTran->srTRec.lnCardRemainAmount;     
                                else if (pobTran->srTRec.inCode == _TICKET_IPASS_DEDUCT_)
                                        lnAmt = pobTran->srTRec.lnCardRemainAmount - pobTran->srTRec.lnTxnAmount;
                                else if (pobTran->srTRec.inCode == _TICKET_IPASS_AUTO_TOP_UP_)
                                        lnAmt = pobTran->srTRec.lnCardRemainAmount + pobTran->srTRec.lnTotalTopUpAmount;    
                                else if (pobTran->srTRec.inCode == _TICKET_IPASS_TOP_UP_)
                                        lnAmt = pobTran->srTRec.lnCardRemainAmount + pobTran->srTRec.lnTxnAmount;              
                                else if (pobTran->srTRec.inCode == _TICKET_IPASS_VOID_TOP_UP_)
                                        lnAmt = pobTran->srTRec.lnCardRemainAmount - pobTran->srTRec.lnTxnAmount;                 
                                else              
                                        lnAmt = pobTran->srTRec.lnCardRemainAmount + pobTran->srTRec.lnTxnAmount;
                                
                                if (lnAmt < 0)
                        	{
                        	        szPacket[inPacketCnt] = '-';  
                        	        inPacketCnt ++;
                        	        memset(szTemplate, 0x00, sizeof(szTemplate)); 
                        	        sprintf(szTemplate, "%08ld", (0 - lnAmt));
                        	}
                        	else     
                        	{
                        	        szPacket[inPacketCnt] = ' '; 
                        	        inPacketCnt ++;
                        	        memset(szTemplate, 0x00, sizeof(szTemplate));    
                                        sprintf(szTemplate, "%08ld", lnAmt);
                                }
                                
                                memcpy(&szPacket[inPacketCnt], &szTemplate[0], 8);           
                                inPacketCnt += 8;
                                
				/* 組MAC使用 */
				memset(gszMAC_F_59, 0x00, sizeof(gszMAC_F_59));
				memcpy(gszMAC_F_59, &szPacket[inPacketCnt - 18], 18);
				
                                /* 
                		        ●	Table ID “ES”: 電票交易序號 
                		*/
                                memcpy(&szPacket[inPacketCnt], "ES", 2); /* Table ID */
                                inPacketCnt += 2;
                        	/* Sub Total Length */
                        	inPacketCnt ++;
                        	szPacket[inPacketCnt] = 0x06; 	/* Sub-Data Total Length */
                        	inPacketCnt ++;
                        	
                        	/* RF序號 */      
                                memcpy(&szPacket[inPacketCnt], &pobTran->srTRec.szTicketRefundCode[0], 6);           
                                inPacketCnt += 6;
                                
				/* 
					●	Table ID “ED”: 電票原交易日期 
				*/ 
                                if (pobTran->srTRec.inCode == _TICKET_IPASS_REFUND_ &&
				   (!memcmp(&guszTICKET_MTI[0], "0100", 4) || !memcmp(&guszTICKET_MTI[0], "0400", 4)))
                                {
                                        
                                        memcpy(&szPacket[inPacketCnt], "ED", 2); /* Table ID */
                                        inPacketCnt += 2;
                                	/* Sub Total Length */
                                	inPacketCnt ++;
                                	szPacket[inPacketCnt] = 0x08; 	/* Sub-Data Total Length */
                                	inPacketCnt ++;
                                	
                                	/* 原交易日期 - 輸入日期大於目前日期，判定為去年 */ 
					memset(szTemp, 0x00, sizeof(szTemp));
					memset(szTemp2, 0x00, sizeof(szTemp2));
					/* [20251219_BUG_MDF][ECC] 修改電票退貨的判斷日期,由 2位MM改為 4位 MMDD */
					memcpy(szTemp2, pobTran->srTRec.szTicketRefundDate, 4);
					
					inNCCC_TICKET_Decide_Year(szTemp, szTemp2);
					memcpy(&szPacket[inPacketCnt], szTemp, 4); 
					inPacketCnt += 4;
                                        
                                        memcpy(&szPacket[inPacketCnt], &pobTran->srTRec.szTicketRefundDate[0], 4); 
                                        inPacketCnt += 4;
                                }
                        }
                        
                        /* ●	Table ID “PS”: 電票交易資訊 */ 
                        memcpy(&szPacket[inPacketCnt], "PS", 2); /* Table ID */
                        inPacketCnt += 2;
                	/* Sub Total Length */
                	inPacketCnt ++;
                	szPacket[inPacketCnt] = 0x08; 	/* Sub-Data Total Length */
                	inPacketCnt ++;
               
                        /* 系統代碼 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetIPASS_System_ID(szTemplate);
                        memcpy(&szPacket[inPacketCnt], szTemplate, 2);    
                        inPacketCnt += 2;
                        
                        /* 業者代碼 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetIPASS_SP_ID(szTemplate);
                        memcpy(&szPacket[inPacketCnt], szTemplate, 2);    
                        inPacketCnt += 2;
                        
                        /* 次業者代碼 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetIPASS_Sub_Company_ID(szTemplate);
                        memcpy(&szPacket[inPacketCnt], szTemplate, 4);    
                        inPacketCnt += 4;
                }
                else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
                { 
                	if (!memcmp(&guszTICKET_MTI[0], "0800", 4))
                        {                  

                        }
                        else
                        {
                                /* ●	Table ID “N1”: UnionPay (原CUP)交易 */
        			memset(szPacket, 0x00, sizeof(szPacket));
        			/* UnionPay(原CUP) data information */
        			memcpy(&szPacket[inPacketCnt], "N1", 2); /* Table ID */
        			inPacketCnt += 2;
        			/* Sub Total Length */
        			inPacketCnt ++;
        			szPacket[inPacketCnt] = 0x02;
        			inPacketCnt ++;
        			/*
        				Function number.
        				The terminal support UnionPay (原CUP) function.
        				0 = Normal.
        				1 = Support.
        			*/
        			szPacket[inPacketCnt] = '1';
        			inPacketCnt ++;
        			/*
        				Acquire Indicator.
        				Terminal hot key control.
        				0 = Normal.
        			*/
        			szPacket[inPacketCnt] = '0';
        			inPacketCnt ++;
                		
                		/* 
                		        ●	Table ID “ET”: 電票交易資訊 
                		*/ 
                                memcpy(&szPacket[inPacketCnt], "ET", 2); /* Table ID */
                                inPacketCnt += 2;
                        	/* Sub Total Length */
                        	inPacketCnt ++;
                        	szPacket[inPacketCnt] = 0x18; 	/* Sub-Data Total Length */
                        	inPacketCnt ++;
                                /* 加2是因為要加上F59總長度 */
				ginECC_F59_ET_Len += inPacketCnt + 2;
				
                                memcpy(&szPacket[inPacketCnt], "                  ", 18);           
                                inPacketCnt += 18;
				
				/* 組MAC使用 */
				memset(gszMAC_F_59, 0x00, sizeof(gszMAC_F_59));
				memcpy(gszMAC_F_59, &szPacket[inPacketCnt - 18], 18);
				
                                /* 
                		        ●	Table ID “ES”: 電票交易序號 
                		*/ 
                                memcpy(&szPacket[inPacketCnt], "ES", 2); /* Table ID */
                                inPacketCnt += 2;
                        	/* Sub Total Length */
                        	inPacketCnt ++;
                        	szPacket[inPacketCnt] = 0x06; 	/* Sub-Data Total Length */
                        	inPacketCnt ++;
                        	
                        	/* RF序號 */      
                                memcpy(&szPacket[inPacketCnt], pobTran->srTRec.szTicketRefundCode, 6);           
                                inPacketCnt += 6;
				
				/* 
					●	Table ID “ED”: 電票原交易日期 
				*/ 
                                if (pobTran->srTRec.inCode == _TICKET_EASYCARD_REFUND_ && 
				   (!memcmp(&guszTICKET_MTI[0], "0100", 4) || !memcmp(&guszTICKET_MTI[0], "0400", 4)))
                                {        
                                        
                                        memcpy(&szPacket[inPacketCnt], "ED", 2); /* Table ID */
                                        inPacketCnt += 2;
                                	/* Sub Total Length */
                                	inPacketCnt ++;
                                	szPacket[inPacketCnt] = 0x08; 	/* Sub-Data Total Length */
                                	inPacketCnt ++;
                                	
                                	/* 原交易日期 - 輸入日期大於目前日期，判定為去年 */ 
					memset(szTemp, 0x00, sizeof(szTemp));
					memset(szTemp2, 0x00, sizeof(szTemp2));
					/* [20251219_BUG_MDF][ECC] 修改電票退貨的判斷日期,由 2位MM改為 4位 MMDD */
					memcpy(szTemp2, pobTran->srTRec.szTicketRefundDate, 4);
					
					inNCCC_TICKET_Decide_Year(szTemp, szTemp2);
					memcpy(&szPacket[inPacketCnt], szTemp, 4); 
					inPacketCnt += 4;
                                        
                                        memcpy(&szPacket[inPacketCnt], &pobTran->srTRec.szTicketRefundDate[0], 4); 
                                        inPacketCnt += 4;
                                }
                        }
                }
		else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
                {
                	/* 0500 部份和上面結帳交易重複，故移除 by Russell 2019/12/2 上午 10:15 */
			if (!memcmp(&guszTICKET_MTI[0], "0800", 4))
                	{

                	}
                        else
                        {
                                memset(szPacket, 0x00, sizeof(szPacket));

                                /* ●	Table ID “N1”: UnionPay (原CUP)交易 */
        			/* UnionPay(原CUP) data information */
        			memcpy(&szPacket[inPacketCnt], "N1", 2); /* Table ID */
        			inPacketCnt += 2;
        			/* Sub Total Length */
        			inPacketCnt ++;
        			szPacket[inPacketCnt] = 0x02;
        			inPacketCnt ++;
        			/*
        				Function number.
        				The terminal support UnionPay (原CUP) function.
        				0 = Normal.
        				1 = Support.
        			*/
        			szPacket[inPacketCnt] = '1';
        			inPacketCnt ++;
        			/*
        				Acquire Indicator.
        				Terminal hot key control.
        				0 = Normal.
        			*/
        			szPacket[inPacketCnt] = '0';
        			inPacketCnt ++;

                		/*
                		        ●	Table ID “ET”: 電票交易資訊
                		*/
                                memcpy(&szPacket[inPacketCnt], "ET", 2); /* Table ID */
                                inPacketCnt += 2;
                        	/* Sub Total Length */
                        	inPacketCnt ++;
                        	szPacket[inPacketCnt] = 0x18; 	/* Sub-Data Total Length */
                        	inPacketCnt ++;

                        	/* Before Amt */
                        	if (pobTran->srTRec.lnCardRemainAmount < 0)
                        	{
                        	        szPacket[inPacketCnt] = 0x2D;
                        	        inPacketCnt ++;
                        	        memset(szTemplate, 0x00, sizeof(szTemplate));
                        	        sprintf(szTemplate, "%08ld", (0 - pobTran->srTRec.lnCardRemainAmount));
                        	}
                        	else
                        	{
                        	        szPacket[inPacketCnt] = 0x20;
                        	        inPacketCnt ++;
                        	        memset(szTemplate, 0x00, sizeof(szTemplate));
                                        sprintf(szTemplate, "%08ld", pobTran->srTRec.lnCardRemainAmount);
                                }

                                memcpy(&szPacket[inPacketCnt], &szTemplate[0], 8);
                                inPacketCnt += 8;

                                /* After Amt */
                                if (pobTran->srTRec.inCode == _TICKET_ICASH_DEDUCT_)
                                        lnAmt = pobTran->srTRec.lnCardRemainAmount - pobTran->srTRec.lnTxnAmount;
                                else if (pobTran->srTRec.inCode == _TICKET_ICASH_AUTO_TOP_UP_)
                                        lnAmt = pobTran->srTRec.lnCardRemainAmount + pobTran->srTRec.lnTotalTopUpAmount;
                                else if (pobTran->srTRec.inCode == _TICKET_ICASH_TOP_UP_)
                                        lnAmt = pobTran->srTRec.lnCardRemainAmount + pobTran->srTRec.lnTxnAmount;
                                else if (pobTran->srTRec.inCode == _TICKET_ICASH_VOID_TOP_UP_)
                                        lnAmt = pobTran->srTRec.lnCardRemainAmount - pobTran->srTRec.lnTxnAmount;
                                else
                                        lnAmt = pobTran->srTRec.lnCardRemainAmount + pobTran->srTRec.lnTxnAmount;

                                if (lnAmt < 0)
                        	{
                        	        szPacket[inPacketCnt] = '-';
                        	        inPacketCnt ++;
                        	        memset(szTemplate, 0x00, sizeof(szTemplate));
                        	        sprintf(szTemplate, "%08ld", (0 - lnAmt));
                        	}
                        	else
                        	{
                        	        szPacket[inPacketCnt] =  ' ';
                        	        inPacketCnt ++;
                        	        memset(szTemplate, 0x00, sizeof(szTemplate));
                                        sprintf(szTemplate, "%08ld", lnAmt);
                                }

                                memcpy(&szPacket[inPacketCnt], &szTemplate[0], 8);
                                inPacketCnt += 8;

				/* 組MAC使用 */
				memset(gszMAC_F_59, 0x00, sizeof(gszMAC_F_59));
				memcpy(gszMAC_F_59, &szPacket[inPacketCnt - 18], 18);
				
                                /*
                		        ●	Table ID “ES”: 電票交易序號
                		*/
                                memcpy(&szPacket[inPacketCnt], "ES", 2); /* Table ID */
                                inPacketCnt += 2;
                        	/* Sub Total Length */
                        	inPacketCnt ++;
                        	szPacket[inPacketCnt] = 0x06; 	/* Sub-Data Total Length */
                        	inPacketCnt ++;

                        	/* RF序號 */
                                memcpy(&szPacket[inPacketCnt], &pobTran->srTRec.szTicketRefundCode[0], 6);
                                inPacketCnt += 6;

				if (pobTran->srTRec.inCode == _TICKET_ICASH_REFUND_ && 
				   (!memcmp(&guszTICKET_MTI[0], "0100", 4)	||
				    !memcmp(&guszTICKET_MTI[0], "0102", 4)))
                                {
                                        /*
                        		        ●	Table ID “ED”: 電票原交易日期
                        		*/
                                        memcpy(&szPacket[inPacketCnt], "ED", 2); /* Table ID */
                                        inPacketCnt += 2;
                                	/* Sub Total Length */
                                	inPacketCnt ++;
                                	szPacket[inPacketCnt] = 0x08; 	/* Sub-Data Total Length */
                                	inPacketCnt ++;
                                	
                                	/* 原交易日期 - 輸入日期大於目前日期，判定為去年 */ 
					memset(szTemp, 0x00, sizeof(szTemp));
					memset(szTemp2, 0x00, sizeof(szTemp2));
					/* [20251219_BUG_MDF][ECC] 修改電票退貨的判斷日期,由 2位MM改為 4位 MMDD */
					memcpy(szTemp2, pobTran->srTRec.szTicketRefundDate, 4);
					
					inNCCC_TICKET_Decide_Year(szTemp, szTemp2);
					memcpy(&szPacket[inPacketCnt], szTemp, 4); 
					inPacketCnt += 4;
                                        
                                        memcpy(&szPacket[inPacketCnt], &pobTran->srTRec.szTicketRefundDate[0], 4); 
                                        inPacketCnt += 4;
                                }
                        }
                }
        }

	/* Packet Data Length */
	memset(szASCII, 0x00, sizeof(szASCII));
	sprintf(szASCII, "%04d", inPacketCnt);
	memset(uszBCD, 0x00, sizeof(uszBCD));
	inFunc_ASCII_to_BCD(uszBCD, szASCII, 2);
	memcpy((char *)&uszPackBuf[inCnt], &uszBCD[0], 2);
	inCnt += 2;
	/* Packet Data */
	memcpy((char *)&uszPackBuf[inCnt], &szPacket[0], inPacketCnt);
	inCnt += inPacketCnt;
	
	/* 計算位置用 */
	if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
	{
		ginECC_F63_Len += inCnt;
	}

	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_MFES_Pack59() END!");
	
        return (inCnt);
}

/*
Function        :inNCCC_TICKET_Pack60
Date&Time       :
Describe        :Field_60:	Reserved-Private Data
		Record of Count must be Unique in the batch.(Batch Number)
*/
int inNCCC_TICKET_Pack60(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 	inCnt = 0;
        long 	lnBatchNum;
        char 	szTemplate[110 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_Pack60() START!");

	/*
	 For all reconciliation messages, this field will contain the 6 digit batch number.
	 This number may not be zero and will be represented in six bytes.
	 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	if (inGetBatchNum(szTemplate) == VS_ERROR)
		return (VS_ERROR);

	lnBatchNum = atol(szTemplate);
	uszPackBuf[inCnt++] = 0x00;
	uszPackBuf[inCnt++] = 0x06;
	sprintf((char *) &uszPackBuf[inCnt], "%06ld", lnBatchNum);
	inCnt += 6;
	
	if (ginDebug == VS_TRUE)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "F_60 [BatchNumber %06ld]", lnBatchNum);
		inLogPrintf(AT, szTemplate);
	}
	
	/* 計算位置用 */
	if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
	{
		ginECC_F63_Len += inCnt;
	}
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_Pack60() END!");

        return (inCnt);
}

/*
Function        :inNCCC_TICKET_Pack62
Date&Time       :
Describe        :Field_62:	Reserved-Private Data(Invoice Number)
*/
int inNCCC_TICKET_Pack62(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 	inCnt = 0;
	long	lnInvNum = 0;
	char	szTemplate[100 + 1];

	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_Pack62() START!");
	
	
        uszPackBuf[inCnt++] = 0x00;
        uszPackBuf[inCnt++] = 0x06;
	
	if (pobTran->inISOTxnCode == _ADVICE_)
	{       
	        if (pobTran->srTRec.uszConfirmBit == VS_TRUE)
		{
			lnInvNum = pobTran->srTRec.lnMainInvNum;
		}
	        else
		{
			lnInvNum = pobTran->srTRec.lnCountInvNum;
		}
	}          
	else
	{
		lnInvNum = pobTran->srTRec.lnMainInvNum;
	}
	
	sprintf((char *)&uszPackBuf[inCnt], "%06ld", lnInvNum);
	
        inCnt += 6;
	
	if (ginDebug == VS_TRUE)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "F_62 [InvoiceNumber %06ld]",  lnInvNum);
		inLogPrintf(AT, szTemplate);
	}
	
	/* 計算位置用 */
	if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
	{
		ginECC_F63_Len += inCnt;
	}
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_Pack62() END!");

        return (inCnt);
}

/*
Function        :inNCCC_TICKET_Pack63
Date&Time       :
Describe        :Field_63:	Reserved-Private Data

		Field_63:	Reserved-Private Data
		--------------------------------------------------------------------------------------------------------------
		Sale Count      |   an	 |   3	 |	Right justified with zero filled on the left
		--------------------------------------------------------------------------------------------------------------
		Sale Amount	|   an	 |   12	 |	Right justified and zeros filled on the left with 2 decimals. No decimal point.
		--------------------------------------------------------------------------------------------------------------
		Refund Count	|   an	 |   3	 |	Right justified with zero filled on the left
		--------------------------------------------------------------------------------------------------------------
		Refund Amount	|   an	 |   12	 |	Right justified and zeros filled on the left with 2 decimals. No decimal point.
		--------------------------------------------------------------------------------------------------------------
		Reserved	|   an	 |   60	 |	Zero filled
		--------------------------------------------------------------------------------------------------------------
	
*/
int inNCCC_TICKET_Pack63(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int			i = 0;
        int			inCnt = 0;
        int			inBatchCnt;
        char			szTemplate[100 + 1];
	char			szMACData[60 + 1] = {0};
        TICKET_ACCUM_TOTAL_REC	srAccumRec;

	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_Pack63() START!");
	
	if (pobTran->inTransactionCode == _SETTLE_ || pobTran->inTransactionCode == _CLS_SETTLE_)
        {
		inBatchCnt = inBATCH_GetTotalCountFromBakFile_By_Sqlite(pobTran);
		if (inBatchCnt == 0)
		{
			/* Total Len */
			uszPackBuf[inCnt++] = 0x03;
			uszPackBuf[inCnt++] = 0x15;
			
			for (i = 0; i < 315; i++)
			{
				uszPackBuf[inCnt] = '0';
				inCnt++;
			};
			
			/* For MAC */
			memset(szMACData, 0x00, sizeof(szMACData));
			for (i = 0; i < 45; i++)
			{
				szMACData[i] = '0';
			};
		}
		else
		{
			/* Total Len */
			uszPackBuf[inCnt++] = 0x03;
			uszPackBuf[inCnt++] = 0x15;
			
			memset(&srAccumRec, 0x00, sizeof(srAccumRec));
			if (inACCUM_GetRecord_ESVC(pobTran, &srAccumRec) != VS_SUCCESS)
			{
				return (VS_ERROR);
			}
			
			/* 悠遊卡 */
			/* 購貨筆數 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%03lu", srAccumRec.lnEASYCARD_DeductTotalCount);
			strcpy((char*)&uszPackBuf[inCnt], szTemplate);
			inCnt += 3;
			/* 購貨金額 */
			sprintf(szTemplate, "%010lld00", (srAccumRec.llEASYCARD_DeductTotalAmount));
			strcpy((char*)&uszPackBuf[inCnt], szTemplate);
			inCnt += 12;
			
			/* 購貨取消筆數 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%03lu", srAccumRec.lnEASYCARD_VoidDeductTotalCount);
			strcpy((char*)&uszPackBuf[inCnt], szTemplate);
			inCnt += 3;
			/* 購貨取消金額 */
			sprintf(szTemplate, "%010lld00", (srAccumRec.llEASYCARD_VoidDeductTotalAmount));
			strcpy((char*)&uszPackBuf[inCnt], szTemplate);
			inCnt += 12;
			
			/* 現金加值筆數 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%03lu", srAccumRec.lnEASYCARD_ADDTotalCount);
			strcpy((char*)&uszPackBuf[inCnt], szTemplate);
			inCnt += 3;
			/* 現金加值金額 */
			sprintf(szTemplate, "%010lld00", (srAccumRec.llEASYCARD_ADDTotalAmount));
			strcpy((char*)&uszPackBuf[inCnt], szTemplate);
			inCnt += 12;
			
			/* 現金加值取消筆數 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%03lu", srAccumRec.lnEASYCARD_VoidADDTotalCount);
			strcpy((char*)&uszPackBuf[inCnt], szTemplate);
			inCnt += 3;
			/* 現金加值取消金額 */
			sprintf(szTemplate, "%010lld00", (srAccumRec.llEASYCARD_VoidADDTotalAmount));
			strcpy((char*)&uszPackBuf[inCnt], szTemplate);
			inCnt += 12;
			
			/* 自動加值筆數 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%03lu", srAccumRec.lnEASYCARD_AutoADDTotalCount);
			strcpy((char*)&uszPackBuf[inCnt], szTemplate);
			inCnt += 3;
			/* 自動加值金額 */
			sprintf(szTemplate, "%010lld00", (srAccumRec.llEASYCARD_AutoADDTotalAmount));
			strcpy((char*)&uszPackBuf[inCnt], szTemplate);
			inCnt += 12;
			
			/* 退貨筆數 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%03lu", srAccumRec.lnEASYCARD_RefundTotalCount);
			strcpy((char*)&uszPackBuf[inCnt], szTemplate);
			inCnt += 3;
			/* 退貨金額 */
			sprintf(szTemplate, "%010lld00", (srAccumRec.llEASYCARD_RefundTotalAmount));
			strcpy((char*)&uszPackBuf[inCnt], szTemplate);
			inCnt += 12;
			
			/* Reserved */
			strcpy((char*)&uszPackBuf[inCnt], "000000000000000");
			inCnt += 15;
			
			/* 一卡通 */
			/* 購貨筆數 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%03lu", srAccumRec.lnIPASS_DeductTotalCount);
			strcpy((char*)&uszPackBuf[inCnt], szTemplate);
			inCnt += 3;
			/* 購貨金額 */
			sprintf(szTemplate, "%010lld00", (srAccumRec.llIPASS_DeductTotalAmount));
			strcpy((char*)&uszPackBuf[inCnt], szTemplate);
			inCnt += 12;
			
			/* 購貨取消筆數 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%03lu", srAccumRec.lnIPASS_VoidDeductTotalCount);
			strcpy((char*)&uszPackBuf[inCnt], szTemplate);
			inCnt += 3;
			/* 購貨取消金額 */
			sprintf(szTemplate, "%010lld00", (srAccumRec.llIPASS_VoidDeductTotalAmount));
			strcpy((char*)&uszPackBuf[inCnt], szTemplate);
			inCnt += 12;
			
			/* 現金加值筆數 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%03lu", srAccumRec.lnIPASS_ADDTotalCount);
			strcpy((char*)&uszPackBuf[inCnt], szTemplate);
			inCnt += 3;
			/* 現金加值金額 */
			sprintf(szTemplate, "%010lld00", (srAccumRec.llIPASS_ADDTotalAmount));
			strcpy((char*)&uszPackBuf[inCnt], szTemplate);
			inCnt += 12;
			
			/* 現金加值取消筆數 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%03lu", srAccumRec.lnIPASS_VoidADDTotalCount);
			strcpy((char*)&uszPackBuf[inCnt], szTemplate);
			inCnt += 3;
			/* 現金加值取消金額 */
			sprintf(szTemplate, "%010lld00", (srAccumRec.llIPASS_VoidADDTotalAmount));
			strcpy((char*)&uszPackBuf[inCnt], szTemplate);
			inCnt += 12;
			
			/* 自動加值筆數 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%03lu", srAccumRec.lnIPASS_AutoADDTotalCount);
			strcpy((char*)&uszPackBuf[inCnt], szTemplate);
			inCnt += 3;
			/* 自動加值金額 */
			sprintf(szTemplate, "%010lld00", (srAccumRec.llIPASS_AutoADDTotalAmount));
			strcpy((char*)&uszPackBuf[inCnt], szTemplate);
			inCnt += 12;
			
			/* 退貨筆數 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%03lu", srAccumRec.lnIPASS_RefundTotalCount);
			strcpy((char*)&uszPackBuf[inCnt], szTemplate);
			inCnt += 3;
			/* 退貨金額 */
			sprintf(szTemplate, "%010lld00", (srAccumRec.llIPASS_RefundTotalAmount));
			strcpy((char*)&uszPackBuf[inCnt], szTemplate);
			inCnt += 12;
			
			/* Reserved */
			strcpy((char*)&uszPackBuf[inCnt], "000000000000000");
			inCnt += 15;
			
			/* 愛金卡 */
			/* 購貨筆數 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%03lu", srAccumRec.lnICASH_DeductTotalCount);
			strcpy((char*)&uszPackBuf[inCnt], szTemplate);
			inCnt += 3;
			/* 購貨金額 */
			sprintf(szTemplate, "%010lld00", (srAccumRec.llICASH_DeductTotalAmount));
			strcpy((char*)&uszPackBuf[inCnt], szTemplate);
			inCnt += 12;
			
			/* 購貨取消筆數 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%03lu", srAccumRec.lnICASH_VoidDeductTotalCount);
			strcpy((char*)&uszPackBuf[inCnt], szTemplate);
			inCnt += 3;
			/* 購貨取消金額 */
			sprintf(szTemplate, "%010lld00", (srAccumRec.llICASH_VoidDeductTotalAmount));
			strcpy((char*)&uszPackBuf[inCnt], szTemplate);
			inCnt += 12;
			
			/* 現金加值筆數 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%03lu", srAccumRec.lnICASH_ADDTotalCount);
			strcpy((char*)&uszPackBuf[inCnt], szTemplate);
			inCnt += 3;
			/* 現金加值金額 */
			sprintf(szTemplate, "%010lld00", (srAccumRec.llICASH_ADDTotalAmount));
			strcpy((char*)&uszPackBuf[inCnt], szTemplate);
			inCnt += 12;
			
			/* 現金加值取消筆數 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%03lu", srAccumRec.lnICASH_VoidADDTotalCount);
			strcpy((char*)&uszPackBuf[inCnt], szTemplate);
			inCnt += 3;
			/* 現金加值取消金額 */
			sprintf(szTemplate, "%010lld00", (srAccumRec.llICASH_VoidADDTotalAmount));
			strcpy((char*)&uszPackBuf[inCnt], szTemplate);
			inCnt += 12;
			
			/* 自動加值筆數 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%03lu", srAccumRec.lnICASH_AutoADDTotalCount);
			strcpy((char*)&uszPackBuf[inCnt], szTemplate);
			inCnt += 3;
			/* 自動加值金額 */
			sprintf(szTemplate, "%010lld00", (srAccumRec.llICASH_AutoADDTotalAmount));
			strcpy((char*)&uszPackBuf[inCnt], szTemplate);
			inCnt += 12;
			
			/* 退貨筆數 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%03lu", srAccumRec.lnICASH_RefundTotalCount);
			strcpy((char*)&uszPackBuf[inCnt], szTemplate);
			inCnt += 3;
			/* 退貨金額 */
			sprintf(szTemplate, "%010lld00", (srAccumRec.llICASH_RefundTotalAmount));
			strcpy((char*)&uszPackBuf[inCnt], szTemplate);
			inCnt += 12;
			
			/* Reserved */
			strcpy((char*)&uszPackBuf[inCnt], "000000000000000");
			inCnt += 15;
			
			/* For MAC */
			memset(szMACData, 0x00, sizeof(szMACData));
			sprintf(szMACData, "%03lu%010lld00""%03lu%010lld00""%03lu%010lld00", 
				srAccumRec.lnEASYCARD_DeductTotalCount, 
				srAccumRec.llEASYCARD_DeductTotalAmount, 
				srAccumRec.lnIPASS_DeductTotalCount, 
				srAccumRec.llIPASS_DeductTotalAmount, 
				srAccumRec.lnICASH_DeductTotalCount, 
				srAccumRec.llICASH_DeductTotalAmount);
		}
	}
	
	/* 算〈MAC〉使用 */
	memset(gszMAC_F_63, 0x00, sizeof(gszMAC_F_63));
	memcpy(&gszMAC_F_63[0], szMACData, 45);
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_Pack63() END!");

        return (inCnt);
}

/*
Function        :inNCCC_TICKET_Pack64
Date&Time       :
Describe        :Field_64	Reserved-Private Data
		1. 除了0800及0400之外，其餘交易電文皆要上傳MAC。
		2. 若連續三次MAC Error則EDC先自動關閉MAC上傳(含無輸入PIN的CUP交易)。
 *		3.不支援舊MFES
*/
int inNCCC_TICKET_Pack64(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int			inCnt = 0;
	char			szTerminalID[8 + 1];
	char			szTemplate[100 + 1];
	char			szAscii[16 + 1];
	TICKET_NCCC_MAC_STRUCT	srMAC;

	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_Pack64() START!");
	
	/* 信用卡 */
	memset(szTerminalID, 0x00, sizeof(szTerminalID));
	inGetTerminalID(szTerminalID);
	
	memset(&srMAC, 0x00, sizeof(TICKET_NCCC_MAC_STRUCT));
	strcpy(srMAC.szF_03, gszMAC_F_03);
	strcpy(srMAC.szF_04, gszMAC_F_04);
	strcpy(srMAC.szF_11, gszMAC_F_11);
	strcpy(srMAC.szF_35, gszMAC_F_35);
	strcpy(srMAC.szF_41, szTerminalID);
	/* 若該筆交易電文沒有 Table ID “ 則此欄位補滿 0 */
	if (strlen(gszMAC_F_59)  > 0)
	{
		strcpy(srMAC.szF_59, gszMAC_F_59);
	}
	else
	{
		strcpy(srMAC.szF_59, "000000000000000000");
	}
	strcpy(srMAC.szF_63, gszMAC_F_63);
	
	if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
	{
		inCnt += 8;
	}
	else
	{
		/* MIT = 〈0800〉 〈0400〉不用送(在Bit Map處理) */
		inNCCC_Ticket_GenMAC(pobTran, &srMAC);

		memcpy((char *)&uszPackBuf[inCnt], &pobTran->szMAC_HEX[0], 8);
		inCnt += 8;
	}

	if (ginDebug == VS_TRUE)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(szAscii, 0x00, sizeof(szAscii));
		inFunc_BCD_to_ASCII(szAscii, (unsigned char*)pobTran->szMAC_HEX, 8);
		sprintf(szTemplate, "F_64 [MAC %s]", szAscii);
		inLogPrintf(AT, szTemplate);
	}
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ATS_Pack64() END!");
	
        return (inCnt);
}

/*
Function        :inNCCC_TICKET_Check03
Date&Time       :
Describe        :送和收的processing Code要一致
*/
int inNCCC_TICKET_Check03(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, unsigned char *uszRecePacket)
{
        if (memcmp((char *)uszSendPacket, (char *)uszRecePacket, 3))
        {
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_TICKET_Check32
Date&Time       :
Describe        :
*/
int inNCCC_TICKET_Check32(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, unsigned char *uszRecePacket)
{
        return (VS_SUCCESS);
}

/*
Function        :inNCCC_TICKET_Check04
Date&Time       :
Describe        :送和收的Amount要一致
*/
int inNCCC_TICKET_Check04(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, unsigned char *uszRecePacket)
{
        if (memcmp((char *)uszSendPacket, (char *)uszRecePacket, 6))
        {
                //vdSGErrorMessage(NCCC_CHECK_ISO_FILED04_ERROR); /* 140 = 電文錯誤請重試 */
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}


/*
Function        :inNCCC_TICKET_Check41
Date&Time       :
Describe        :送和收的TID要一致
*/
int inNCCC_TICKET_Check41(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, unsigned char *uszRecePacket)
{
        if (memcmp((char *)uszSendPacket, (char *)uszRecePacket, 8))
        {
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_TICKET_UnPack24
Date&Time       :
Describe        : 1. NPS 需求要判斷 Field_24 第一個 Byte 是否為【9】
		2. 此 Flag 是由主機做控管，端末機不管例外處理
*/
int inNCCC_TICKET_UnPack24(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
        char	szNPS[8 + 1];

	memset(szNPS, 0x00, sizeof(szNPS));
	inFunc_BCD_to_ASCII(&szNPS[0], uszUnPackBuf, 2);
	if (szNPS[0] == 0x39)
	{
		pobTran->srBRec.uszField24NPSBit = VS_TRUE;
		pobTran->srBRec.uszNoSignatureBit = VS_TRUE;
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inNCCC_TICKET_UnPack32
Date&Time       :2018/1/4 下午 4:50
Describe        :
*/
int inNCCC_TICKET_UnPack32(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
        return (VS_SUCCESS);
}

/*
Function        :inNCCC_TICKET_UnPack38
Date&Time       :2018/1/4 下午 4:50
Describe        :
*/
int inNCCC_TICKET_UnPack38(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
        if (pobTran->inISOTxnCode == _REVERSAL_)
	{
                return (VS_SUCCESS);
	}
	else
	{
		memset(pobTran->srTRec.szAuthCode, 0x00, sizeof(pobTran->srTRec.szAuthCode));
		memcpy(&pobTran->srTRec.szAuthCode[0], (char *) &uszUnPackBuf[0], _AUTH_CODE_SIZE_);
	}

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_TICKET_UnPack39
Date&Time       :2018/1/4 下午 4:50
Describe        :Response code
*/
int inNCCC_TICKET_UnPack39(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{	
	char	szTemplate[6 + 1];

	memset(&szTemplate, 0x00, sizeof(szTemplate));
	inFunc_BCD_to_ASCII(&szTemplate[0], &guszNCCC_TICKET_ISO_Field03[0], 3);

	memset(pobTran->srTRec.szRespCode, 0x00, sizeof(pobTran->srTRec.szRespCode));
	memcpy(&pobTran->srTRec.szRespCode[0], (char *)&uszUnPackBuf[0], 2);

        /* 2011-05-24 PM 05:45:19 銀聯一般交易 01不能被改成05 */
	/* 【需求單 - 106306】	補登交易回覆Call Bank需求 by Russell 2019/10/5 上午 11:46 */
	if (pobTran->inISOTxnCode != _SALE_		&&
	    pobTran->inISOTxnCode != _CUP_SALE_		&&
	    pobTran->inISOTxnCode != _SALE_OFFLINE_	&&
	    ((!memcmp(&pobTran->srTRec.szRespCode[0], "01", 2)) || (!memcmp(&pobTran->srTRec.szRespCode[0], "02", 2))))
	{
		memset(pobTran->srTRec.szRespCode, 0x00, sizeof(pobTran->srTRec.szRespCode));
		strcpy(pobTran->srTRec.szRespCode, "05");
	}
	else if (pobTran->inISOTxnCode == _CLS_BATCH_ &&
		 !memcmp(&pobTran->srTRec.szRespCode[0], "95", 2) &&
		 !memcmp(&szTemplate[0], "960000", 6))
	{
		/*
			若是MTI:0510 + PCode:960000 TICKET 仍回覆Response Code=95，則端末機視為結帳失敗。端末機下次執行結帳時，
			需從頭開始(MTI:0500 +PCode:920000)。
			此時需要重新BatchUpload，
		*/
		/* CLS SETTLE會在外面檢查回應碼，這邊不做動作 */

	}

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_TICKET_UnPack56
Date&Time       :2018/1/4 下午 4:50
Describe        :
*/
int inNCCC_TICKET_UnPack56(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
	int 	inLen, inCnt = 0, inSubTotalLength;
        char    szTemp[10] = {0};
        
	inLen = ((uszUnPackBuf[0] / 16 * 10 + uszUnPackBuf[0] % 16) * 100);
	inLen += (uszUnPackBuf[1] / 16 * 10) + uszUnPackBuf[1] % 16;
	
	if (inLen > 0)
	{
		inCnt += 2;

		while (inLen > inCnt)
		{
			if (!memcmp(&uszUnPackBuf[inCnt], "IP00", 4))
			{      
				inCnt += 4; /* Table ID */
				/* Sub Total Length */
				inSubTotalLength = ((uszUnPackBuf[inCnt] / 16) * 1000) + ((uszUnPackBuf[inCnt] % 16) * 100);
				inSubTotalLength += ((uszUnPackBuf[inCnt + 1] / 16) * 10) + (uszUnPackBuf[inCnt + 1] % 16);
				inCnt += 2;
				inCnt += inSubTotalLength;
			}
			else if (!memcmp(&uszUnPackBuf[inCnt], "IP01", 4))
			{      
				inCnt += 4; /* Table ID */
				/* Sub Total Length */
				inSubTotalLength = ((uszUnPackBuf[inCnt] / 16) * 1000) + ((uszUnPackBuf[inCnt] % 16) * 100);
				inSubTotalLength += ((uszUnPackBuf[inCnt + 1] / 16) * 10) + (uszUnPackBuf[inCnt + 1] % 16);
				inCnt += 2;
				
				if (pobTran->inISOTxnCode != _REVERSAL_)
				{        
        				pobTran->srTRec.srIPASSRec.lnSign_Len = inSubTotalLength;
        				memset(pobTran->srTRec.srIPASSRec.szSign_Data, 0x00, sizeof(pobTran->srTRec.srIPASSRec.szSign_Data)); 
        				memcpy(pobTran->srTRec.srIPASSRec.szSign_Data, (char *)&uszUnPackBuf[inCnt], inSubTotalLength);
				}
				
				inCnt += inSubTotalLength;
			}
			else if (!memcmp(&uszUnPackBuf[inCnt], "IP02", 4))
			{      
				inCnt += 4; /* Table ID */
				/* Sub Total Length */
				inSubTotalLength = ((uszUnPackBuf[inCnt] / 16) * 1000) + ((uszUnPackBuf[inCnt] % 16) * 100);
				inSubTotalLength += ((uszUnPackBuf[inCnt + 1] / 16) * 10) + (uszUnPackBuf[inCnt + 1] % 16);
				inCnt += 2;
				inCnt += inSubTotalLength;
			}
			else if (!memcmp(&uszUnPackBuf[inCnt], "IC01", 4) || !memcmp(&uszUnPackBuf[inCnt], "IC02", 4))
			{
				inCnt += 4; /* Table ID */
				/* Sub Total Length */
				inSubTotalLength = ((uszUnPackBuf[inCnt] / 16) * 1000) + ((uszUnPackBuf[inCnt] % 16) * 100);
				inSubTotalLength += ((uszUnPackBuf[inCnt + 1] / 16) * 10) + (uszUnPackBuf[inCnt + 1] % 16);
				inCnt += 2;

				if (pobTran->inISOTxnCode != _REVERSAL_)
				{
        				pobTran->srTRec.srICASHRec.lnSign_Len = inSubTotalLength;
        				memset(pobTran->srTRec.srICASHRec.szSign_Data, 0x00, sizeof(pobTran->srTRec.srICASHRec.szSign_Data));
        				memcpy(&pobTran->srTRec.srICASHRec.szSign_Data[0], (char *)&uszUnPackBuf[inCnt], inSubTotalLength);

        				memset(szTemp, 0x00, sizeof(szTemp));
        				memcpy(&szTemp[0], &pobTran->srTRec.srICASHRec.szSign_Data[44], 6);

        				if (!memcmp(szTemp, "990003", 6))
					{
        				        pobTran->srTRec.uszBlackListBit = VS_TRUE;
						
						if (ginDebug == VS_TRUE)
						{
							inLogPrintf(AT, "判定為黑名單");
						}
					}
				}

				inCnt += inSubTotalLength;
			}
			else
			        inCnt++; 
                }
        }
        else
	{
		inLogPrintf(AT, "inTICKETUnPack56()_ERROR");
		return (VS_ERROR);
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inNCCC_TICKET_UnPack59
Date&Time       :2018/1/4 下午 4:51
Describe        :
*/
int inNCCC_TICKET_UnPack59(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
        int	inLen, inCnt = 0, inSubTotalLength;
	char	szTemplate[1024 + 1];
	char	szFesMode[2 + 1] = {0};

        if (pobTran->inISOTxnCode == _REVERSAL_)
                return (VS_SUCCESS);
	
	/* 因為票證不會回"MP"，所以只要有開MPAS都on uszMPASTransBit by Russell 2020/7/14 上午 11:54 */
	memset(szFesMode, 0x00, sizeof(szFesMode));
	inGetNCCCFESMode(szFesMode);
	if (memcmp(szFesMode, _NCCC_04_MPAS_MODE_, 2) == 0)
	{
		pobTran->srTRec.uszMPASTransBit = VS_TRUE;
		/* 重印簽單的判斷 可能有斷電的例外狀況 收完電文預設可重印 單機操作可重印 */
		/* 悠遊卡不走online analyze所以放這裡 */
		pobTran->srTRec.uszMPASReprintBit = VS_TRUE;
	}
	
	inLen = ((uszUnPackBuf[0] / 16 * 10 + uszUnPackBuf[0] % 16) * 100);
	inLen += (uszUnPackBuf[1] / 16 * 10) + uszUnPackBuf[1] % 16;

	if (inLen > 0)
	{
		inCnt += 2;

		while (inLen > inCnt)
		{
			if (!memcmp(&uszUnPackBuf[inCnt], "N1", 2))
			{
				/* ●	Table ID “N1”: UnionPay (原CUP)交易 */
				inCnt += 2; /* Table ID */
				/* Sub Total Length */
				inSubTotalLength = ((uszUnPackBuf[inCnt] % 16) * 100) + ((uszUnPackBuf[inCnt + 1] / 16) * 10) + (uszUnPackBuf[inCnt + 1] % 16);
				inCnt += 2;
				/*
					Function number.
					The terminal support UnionPay (原CUP) function.
					0 = Normal.
					1 = Support.
				*/
				inCnt ++;
				/*
					Acquire Indicator.
					Terminal hot key control.
					0 = Normal.
					1 = UnionPay (原CUP) card.
				*/
				inCnt ++;

				/* ================================================== */
				/* 當 "Acquire Indicator" = 1時，需要傳送下述所有欄位 */
				if (uszUnPackBuf[inCnt - 1] == '1')
				{
					/*
						UnionPay (原CUP) Trace Number（TN）
						主機回傳時，下述交易由授權主機更新回傳：
						1. Sale
						2. Void
						3. Pre-Auth
						4. Void Pre-Auth
					*/
					memcpy(&pobTran->srBRec.szCUP_TN[0], &uszUnPackBuf[inCnt], 6);
					inCnt += 6;
					/*
						UnionPay (原CUP) Transaction Date(MMDD)
						主機回傳時，下述交易由授權主機更新回傳：
						1. Sale
						2. Pre-Auth
					*/
					memcpy(&pobTran->srBRec.szCUP_TD[0], &uszUnPackBuf[inCnt], 4);
					inCnt += 4;
					/*
						UnionPay (原CUP) Transaction Time(hhmmss)
						主機回傳時，下述交易由授權主機更新回傳：
						1. Sale
						2. Void
						3. Pre-Auth
						4. Void Pre-Auth
						5. Pre-Auth Complete
						6. Void Pre-Auth Complete
					*/
					memcpy(&pobTran->srBRec.szCUP_TT[0], &uszUnPackBuf[inCnt], 6);
					inCnt += 6;
					/*
						UnionPay (原CUP) Retrieve Reference Number(CRRN)
						主機回傳時，下述交易由授權主機更新回傳：
						1. Sale
						2. Pre-Auth
					*/
					memcpy(&pobTran->srBRec.szCUP_RRN[0], &uszUnPackBuf[inCnt], 12);
					inCnt += 12;
					/*
						Settlement date(MMDD)
						主機回傳時，下述交易由授權主機更新回傳：
						1. Sale
						2. Void
						3. Pre-Auth
						4. Void Pre-Auth
					*/
					memcpy(&pobTran->srBRec.szCUP_STD[0], &uszUnPackBuf[inCnt], 4);
					inCnt += 4;
				}
			}
			else if (!memcmp(&uszUnPackBuf[inCnt], "N2", 2))
			{
				/* ●	Table ID “N2”: Settlement Flag */
				inCnt += 2; /* Table ID */
				/* Sub Total Length */
				inSubTotalLength = ((uszUnPackBuf[inCnt] % 16) * 100) + ((uszUnPackBuf[inCnt + 1] / 16) * 10) + (uszUnPackBuf[inCnt + 1] % 16);
				inCnt += 2;
				inCnt += inSubTotalLength;
			}
			else if (!memcmp(&uszUnPackBuf[inCnt], "N3", 2))
			{
				/* ●	Table ID “N3”: Host AD “YYYY” (系統西元年) */
				inCnt += 2; /* Table ID */
				/* Sub Total Length */
				inSubTotalLength = ((uszUnPackBuf[inCnt] % 16) * 100) + ((uszUnPackBuf[inCnt + 1] / 16) * 10) + (uszUnPackBuf[inCnt + 1] % 16);
				inCnt += 2;
				
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &uszUnPackBuf[inCnt], 4);
				
				memcpy(pobTran->srTRec.szDate, &szTemplate[2], 2);
				inCnt += inSubTotalLength;
			}
			else if (!memcmp(&uszUnPackBuf[inCnt], "ND", 2))
			{
				/* ●	Table ID “ND”: FES ID */
				inCnt += 2; /* Table ID */
				/* Sub Total Length */
				inSubTotalLength = ((uszUnPackBuf[inCnt] % 16) * 100) + ((uszUnPackBuf[inCnt + 1] / 16) * 10) + (uszUnPackBuf[inCnt + 1] % 16);
				inCnt += 2;
				inCnt += inSubTotalLength;
			}
			else if (!memcmp(&uszUnPackBuf[inCnt], "NF", 2))
			{
				/* ●	Table ID “NF”: FES Indicator */
				inCnt += 2; /* Table ID */
				/* Sub Total Length */
				inSubTotalLength = ((uszUnPackBuf[inCnt] % 16) * 100) + ((uszUnPackBuf[inCnt + 1] / 16) * 10) + (uszUnPackBuf[inCnt + 1] % 16);
				inCnt += 2;
				inCnt += inSubTotalLength;
			}
			else if (!memcmp(&uszUnPackBuf[inCnt], "NS", 2))
			{
				/* ●	Table ID “NS”: Unattended Gas Station Relation Data(持卡人自助交易) */
				inCnt += 2; /* Table ID */
				/* Sub Total Length */
				inSubTotalLength = ((uszUnPackBuf[inCnt] % 16) * 100) + ((uszUnPackBuf[inCnt + 1] / 16) * 10) + (uszUnPackBuf[inCnt + 1] % 16);
				inCnt += 2;
				inCnt += inSubTotalLength;
			}
			else if (!memcmp(&uszUnPackBuf[inCnt], "ES", 2))
			{
				/* ●	Table ID “ES”: 電票交易序號 */
				inCnt += 2; /* Table ID */
				/* Sub Total Length */
				inSubTotalLength = ((uszUnPackBuf[inCnt] % 16) * 100) + ((uszUnPackBuf[inCnt + 1] / 16) * 10) + (uszUnPackBuf[inCnt + 1] % 16);
				inCnt += 2;
				memcpy(&pobTran->srTRec.szTicketRefundCode[0], &uszUnPackBuf[inCnt], inSubTotalLength);
				inCnt += inSubTotalLength;
			}
			else if (!memcmp(&uszUnPackBuf[inCnt], "ED", 2))
			{
				/* ●	Table ID “ED”: 電票原交易日期 */
				inCnt += 2; /* Table ID */
				/* Sub Total Length */
				inSubTotalLength = ((uszUnPackBuf[inCnt] % 16) * 100) + ((uszUnPackBuf[inCnt + 1] / 16) * 10) + (uszUnPackBuf[inCnt + 1] % 16);
				inCnt += 2;
				inCnt += inSubTotalLength;
			}
			else if (!memcmp(&uszUnPackBuf[inCnt], "L1", 2))
			{
				/*
					●	Table ID “L1”: Reward (優惠活動)
					取消時，若是原正項交易無任何優惠訊息(Header第一欄位=0)，則端末機不需上傳Table ID “L1”。
				*/
				inCnt += 2; /* Table ID */
				/* Sub Total Length */
				inSubTotalLength = ((uszUnPackBuf[inCnt] % 16) * 100) + ((uszUnPackBuf[inCnt + 1] / 16) * 10) + (uszUnPackBuf[inCnt + 1] % 16);
				inCnt += 2;
				/*
					列印優惠或廣告資訊之個數。
					‘0’=表示無優惠活動
					‘1’=表示僅有一個優惠活動
					‘2’=表示有兩個優惠活動
					註1. 若是只有一個優惠活動，則只需下載【第一個優惠活動】欄位的資料。每次下載之優惠活動不超過2個
					註2. 請求電文需傳送至優惠序號，預設上傳’0’。FES依此判斷端末機是否為支援優惠平台之端末機版本，FES需進行優惠查詢。
					註3. 回覆電文中，若本欄位=’0’，則僅回傳至優惠序號。
				*/
				/* 儲存優惠資訊 */
				if (inNCCC_Loyalty_Save_Reward_Data(inSubTotalLength + 4, (char*)&uszUnPackBuf[inCnt - 4], _REWARD_ESVC_FILE_NAME_) != VS_SUCCESS)
				{
					return (VS_ERROR);
				}
				
				/* 優惠個數(因為取消要送，所以還是必須存) */
				memcpy(pobTran->srTRec.szAwardNum, &uszUnPackBuf[inCnt], 1);
				
				if (uszUnPackBuf[inCnt] == '0')
				{
					/* 沒有要列印優惠或廣告資訊，有可能要列印補充資訊(暫停優惠服務) */
					if (uszUnPackBuf[inCnt + 23] == '1')
					{
						pobTran->srTRec.uszRewardL1Bit = VS_TRUE;
						pobTran->srTRec.uszRewardSuspendBit = VS_TRUE;		/* 暫停優惠服務(優惠活動為0且要印補充資訊) */
					}
					else
						pobTran->srTRec.uszRewardL1Bit = VS_FALSE;
				}
				else
				{
					pobTran->srTRec.uszRewardL1Bit = VS_TRUE;
				}

				inCnt += inSubTotalLength;

			}
			else if (!memcmp(&uszUnPackBuf[inCnt], "L2", 2))
			{
				/*
					Table ID 'L2': Reward or Advertisement (優惠活動||累計訊息)及廣告資訊
					取消時，若是原正項交易無任何優惠訊息(Header第一欄位=0)，則端末機不需上傳Table ID “L2”。
					取消時，若是原正項交易只有廣告，則端末機不需上傳Table ID “L2”。
				 */
				inCnt += 2; /* Table ID */
				/* Sub Total Length */
				inSubTotalLength = ((uszUnPackBuf[inCnt] % 16) * 100) + ((uszUnPackBuf[inCnt + 1] / 16) * 10) + (uszUnPackBuf[inCnt + 1] % 16);
				inCnt += 2;
				/*
					列印(優惠活動||累計訊息)及廣告資訊之個數。
					‘0’=表示無(優惠活動||累計訊息)及廣告資訊
					‘1’=表示僅有一個(累計訊息)或廣告資訊
					‘2’=表示有(優惠活動||累計訊息)及廣告資訊
					註1.	本欄位值=’0’，則僅回傳至優惠序號。
					註2.	取消(含沖銷取消)須上傳原交易。
					註3.	本欄位值=’1’是指僅有一個，但仍需判斷第一段與第二段電文。
				*/
				/* 儲存優惠資訊 */
				if (inNCCC_Loyalty_Save_Reward_Data(inSubTotalLength + 4, (char*)&uszUnPackBuf[inCnt - 4], _REWARD_ESVC_FILE_NAME_) != VS_SUCCESS)
				{
					return (VS_ERROR);
				}
				
				/* 優惠個數(因為取消要送，所以還是必須存) */
				memcpy(pobTran->srTRec.szAwardNum, &uszUnPackBuf[inCnt], 1);
				
				if (uszUnPackBuf[inCnt] == '0')
				{
					/* 沒有要列印優惠或廣告資訊，有可能要列印補充資訊(暫停優惠服務) */
					if (uszUnPackBuf[inCnt + 23] == '1')
					{
						pobTran->srTRec.uszRewardL2Bit = VS_TRUE;
						pobTran->srTRec.uszRewardSuspendBit = VS_TRUE;		/* 暫停優惠服務(優惠活動為0且要印補充資訊) */
					}
					else
						pobTran->srTRec.uszRewardL2Bit = VS_FALSE;
				}
				else
				{
					pobTran->srTRec.uszRewardL2Bit = VS_TRUE;
				}

				inCnt += inSubTotalLength;

			}
			else if (!memcmp(&uszUnPackBuf[inCnt], "L5", 2))
			{
				/*
					●	Table ID “L5”: Reward (優惠活動)
					取消時，若是原正項交易無任何優惠訊息(Header第一欄位=0)，則端末機不需上傳Table ID “L5”。
				*/
				inCnt += 2; /* Table ID */
				/* Sub Total Length */
				inSubTotalLength = ((uszUnPackBuf[inCnt] % 16) * 100) + ((uszUnPackBuf[inCnt + 1] / 16) * 10) + (uszUnPackBuf[inCnt + 1] % 16);
				inCnt += 2;
				/*
					列印優惠或廣告資訊之個數。
					‘0’=表示無優惠活動
					‘1’=表示僅有一個優惠活動
					‘2’=表示有兩個優惠活動
					註1. 若是只有一個優惠活動，則只需下載【第一個優惠活動】欄位的資料。每次下載之優惠活動不超過2個
					註2. 請求電文需傳送至優惠序號，預設上傳’0’。FES依此判斷端末機是否為支援優惠平台之端末機版本，FES需進行優惠查詢。
					註3. 回覆電文中，若本欄位=’0’，則僅回傳至優惠序號。
				*/
				/* 儲存優惠資訊 */
				if (inNCCC_Loyalty_Save_Reward_Data_L5(inSubTotalLength + 4, (char*)&uszUnPackBuf[inCnt - 4], _REWARD_ESVC_FILE_NAME_) != VS_SUCCESS)
				{
					return (VS_ERROR);
				}
				
				/* 優惠個數(因為取消要送，所以還是必須存) */
				memcpy(pobTran->srTRec.szAwardNum, &uszUnPackBuf[inCnt], 1);
				
				if (uszUnPackBuf[inCnt] == '0')
				{
					/* 沒有要列印優惠或廣告資訊，有可能要列印補充資訊(暫停優惠服務) */
					if (uszUnPackBuf[inCnt + 23] == '1')
					{
						pobTran->srTRec.uszRewardL5Bit = VS_TRUE;
						pobTran->srTRec.uszRewardSuspendBit = VS_TRUE;		/* 暫停優惠服務(優惠活動為0且要印補充資訊) */
					}
					else
						pobTran->srTRec.uszRewardL5Bit = VS_FALSE;
				}
				else
				{
					pobTran->srTRec.uszRewardL5Bit = VS_TRUE;
				}

				inCnt += inSubTotalLength;

			}
			else if (!memcmp(&uszUnPackBuf[inCnt], "L3", 2))
			{
				/* ●	Table ID “L3”: Reward (優惠活動) */
				inCnt += 2; /* Table ID */
				/* Sub Total Length */
				inSubTotalLength = ((uszUnPackBuf[inCnt] % 16) * 100) + ((uszUnPackBuf[inCnt + 1] / 16) * 10) + (uszUnPackBuf[inCnt + 1] % 16);
				inCnt += 2;
				
				/* 儲存優惠資訊 */
				if (inNCCC_Loyalty_Save_Reward_Data(inSubTotalLength + 4, (char*)&uszUnPackBuf[inCnt - 4], _REWARD_ESVC_FILE_NAME_) != VS_SUCCESS)
				{
					return (VS_ERROR);
				}

				inCnt += inSubTotalLength;
			}
			else
			{
				inCnt ++;
			}
		}
	}
	else
	{
		inLogPrintf(AT, "inNCCC_TICKET_UnPack59()_ERROR");
		return (VS_ERROR);
	}

	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_TICKET_UnPack60
Date&Time       :2020/8/19 下午 2:29
Describe        :
*/
int inNCCC_TICKET_UnPack60(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
	int	inRetVal = VS_SUCCESS;

	if (!memcmp(pobTran->srTRec.szRespCode, "A0", 2)	||
	    !memcmp(pobTran->srTRec.szRespCode, "A1", 2))
	{
		/* 格式與ATS相同，所以直接照搬 */
		inRetVal = inNCCC_ATS_TMKKeyExchange(pobTran, uszUnPackBuf);
	}

	return (inRetVal);
}

/*
Function        :inNCCC_TICKET_ISOPackMessageType
Date&Time       :2016/9/14 下午 1:40
Describe        :把MTI放到PackData中，並放到guszTICKET_MTI中以便在pack其他欄位時判斷
*/
int inNCCC_TICKET_ISOPackMessageType(TRANSACTION_OBJECT *pobTran, int inISOTxnCode, unsigned char *uszPackData, char *szMTI)
{
        int 		inCnt = 0;
	char		szTemp[4 + 1] = {0};
        unsigned char 	uszBCD[10 + 1];
	
	memset(szTemp, 0x00, sizeof(szTemp));
        memcpy(&szTemp[0], &szMTI[0], 4);
	if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
	{
	        /* 0100 = > 0102 */
	        if (pobTran->srTRec.srICASHRec.inStepNum == 2)
	                szTemp[3] = 0x32;
        }
	
	/* 放到global中方便判斷 */
	memcpy((char*)guszTICKET_MTI, szTemp, 4);
	
	/* 放到把MTI放到PackData中 */
        memset(uszBCD, 0x00, sizeof(uszBCD));
        inFunc_ASCII_to_BCD(&uszBCD[0], &szTemp[0], _NCCC_TICKET_MTI_SIZE_);
        memcpy((char *) &uszPackData[inCnt], (char *) &uszBCD[0], _NCCC_TICKET_MTI_SIZE_);
        inCnt += _NCCC_TICKET_MTI_SIZE_;

        return (inCnt);
}

/*
Function        :inNCCC_TICKET_ATS_ISOModifyBitMap
Date&Time       :2018/1/4 下午 5:10
Describe        :
*/
int inNCCC_TICKET_ATS_ISOModifyBitMap(TRANSACTION_OBJECT *pobTran, int inISOTxnCode, int *inBitMap)
{
	char	szHostEnable[1 + 1] = {0};
	char	szMACEnable[1 + 1] = {0};
	char	szCommMode[1 + 1] = {0};
	char	szIFESMode[1 + 1] = {0};
	char	szEncryptMode[1 + 1] = {0};
        	
	/* F_38 */
	if (pobTran->srTRec.inCode == _TICKET_IPASS_VOID_TOP_UP_ 	|| 
	    pobTran->srTRec.inCode == _TICKET_EASYCARD_VOID_TOP_UP_	||
	    pobTran->srTRec.inCode == _TICKET_ICASH_VOID_TOP_UP_)
	{
		inNCCC_TICKET_BitMapSet(inBitMap, 38);
	}
                
	/* F_48 */
        if (strlen(pobTran->srTRec.szStoreID) > 0)
	{
		inNCCC_TICKET_BitMapSet(inBitMap, 48);
	}
		
        /* 結帳無ECC就不帶F_56 */  
        if (pobTran->srTRec.inCode == _SETTLE_)      
        {
                inLoadTDTRec(_TDT_INDEX_01_ECC_);
		memset(szHostEnable, 0x00, sizeof(szHostEnable));
		inGetTicket_HostEnable(szHostEnable);
                if (memcmp(szHostEnable, "Y", strlen("Y")) == 0)
		{
                        inNCCC_TICKET_BitMapSet(inBitMap, 56);
		}
        }
	
	/* F_57 */
	memset(szCommMode, 0x00, sizeof(szCommMode));
	inGetCommMode(szCommMode);
	memset(szIFESMode, 0x00, sizeof(szIFESMode));
	inGetI_FES_Mode(szIFESMode);
	memset(szEncryptMode, 0x00, sizeof(szEncryptMode));
	inGetEncryptMode(szEncryptMode);

	if (memcmp(szCommMode, _COMM_ETHERNET_MODE_, 1) == 0 || 
	    memcmp(szCommMode, _COMM_GPRS_MODE_, 1) == 0 ||
	    memcmp(szCommMode, _COMM_3G_MODE_, 1) == 0	||
	    memcmp(szCommMode, _COMM_4G_MODE_, 1) == 0	||
	    memcmp(szCommMode, _COMM_WIFI_MODE_, 1) == 0)
	{
		if (memcmp(guszTICKET_MTI, "0500", 4) == 0	||
		    memcmp(guszTICKET_MTI, "0800", 4) == 0)
		{

		}
		else
		{
			/* 只要走I-FES，且不是MFES，就要tSAM加密 */
			if (memcmp(szIFESMode, "Y", 1) == 0)
			{
				inNCCC_TICKET_BitMapSet(inBitMap, 57);
			}
		}
	}
	
	/* F_64 */
	/* 預設除0400、0800外全部要送MAC，但若安全認證沒通過，信用卡交易不送F_64 */
	/* 這邊採用跟Verifone Code一樣的邏輯，安全認證時先砍Working Key，若找不到Key代表安全認證失敗 */
	/* CFGT 的 MacEnable沒On 或是 沒有Mac Key 或是 0220的交易 不送Mac */
	/* MAC換key後，回3次A0或A1 */
	memset(szMACEnable, 0x00, sizeof(szMACEnable));
	inGetMACEnable(szMACEnable);
	if ((memcmp(szMACEnable, "Y", 1) != 0)						||	/* CFGT沒開 */
	    (inKMS_CheckKey(_TWK_KEYSET_NCCC_, _TWK_KEYINDEX_NCCC_MAC_) != VS_SUCCESS)	||
	     ginMacError >= 3								||	/* 安全認證沒過 */
	     pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)					/* ECC 在ICER流程中決定 */
	{
		inNCCC_TICKET_BitMapReset(inBitMap, 64);
	}

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_TICKET_MFES_ISOModifyBitMap
Date&Time       :2018/1/4 下午 5:10
Describe        :
*/
int inNCCC_TICKET_MFES_ISOModifyBitMap(TRANSACTION_OBJECT *pobTran, int inISOTxnCode, int *inBitMap)
{
	char	szHostEnable[2 + 1];
        	
	/* F_38 */
	if (pobTran->srTRec.inCode == _TICKET_IPASS_VOID_TOP_UP_ 	|| 
	    pobTran->srTRec.inCode == _TICKET_EASYCARD_VOID_TOP_UP_ 	||
	    pobTran->srTRec.inCode == _TICKET_ICASH_VOID_TOP_UP_)
	{
		inNCCC_TICKET_BitMapSet(inBitMap, 38);
	}
                
	/* F_48 */
        if (strlen(pobTran->srTRec.szStoreID) > 0)
	{
		inNCCC_TICKET_BitMapSet(inBitMap, 48);
	}
		
        /* 結帳無ECC就不帶F_56 */  
        if (pobTran->srTRec.inCode == _SETTLE_)      
        {
                inLoadTDTRec(_TDT_INDEX_01_ECC_);
		memset(szHostEnable, 0x00, sizeof(szHostEnable));
		inGetTicket_HostEnable(szHostEnable);
                if (memcmp(szHostEnable, "Y", strlen("Y")) == 0)
		{
                        inNCCC_TICKET_BitMapSet(inBitMap, 56);
		}
        } 
	
	/* 【需求單-109057】電票交易電文新增tSAM加密及MAC需求不支援舊MFES，必定拿掉F_64 */
	inNCCC_TICKET_BitMapReset(inBitMap, 64);
	
        return (VS_SUCCESS);
}


int inNCCC_TICKET_ATS_ISOModifyPackData(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackData, int *inPackLen)
{
	char	szCommMode[1 + 1] = {0};
	char	szIFESMode[1 + 1] = {0};
	char	szEncryptMode[1 + 1] = {0};
	
	/* F_57 */
	/* 有Tsam 要加送 F_57，除了0500和CUP LOGON */
	memset(szCommMode, 0x00, sizeof(szCommMode));
	inGetCommMode(szCommMode);
	memset(szIFESMode, 0x00, sizeof(szIFESMode));
	inGetI_FES_Mode(szIFESMode);
	memset(szEncryptMode, 0x00, sizeof(szEncryptMode));
	inGetEncryptMode(szEncryptMode);

	if (memcmp(szCommMode, _COMM_ETHERNET_MODE_, 1) == 0	|| 
	    memcmp(szCommMode, _COMM_GPRS_MODE_, 1) == 0	||
	    memcmp(szCommMode, _COMM_3G_MODE_, 1) == 0		||
	    memcmp(szCommMode, _COMM_4G_MODE_, 1) == 0		||
	    memcmp(szCommMode, _COMM_WIFI_MODE_, 1) == 0)
	{
		/* 只要走I-FES，且不是MFES，就要tSAM加密 */
		if (memcmp(szIFESMode, "Y", 1) == 0)
		{
			uszPackData[4] = gusztSAMKeyIndex; /* TPDU 最後一個 Byte */
		}
	}
	
        return (VS_SUCCESS);
}

int inNCCC_TICKET_MFES_ISOModifyPackData(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackData, int *inPackLen)
{
        return (VS_SUCCESS);
}

/*
Function        :inNCCC_TICKET_ISOCheckHeader
Date&Time       :2016/9/14 上午 10:17
Describe        :
*/
int inNCCC_TICKET_ISOCheckHeader(TRANSACTION_OBJECT *pobTran, char *szSendISOHeader, char *szReceISOHeader)
{
	int	inCnt = 0;

	/* 檢查TPDU */
	inCnt += 5;
	/* 檢查MTI */
	szSendISOHeader[inCnt + 1] += 0x10;
	if (memcmp(&szSendISOHeader[inCnt], &szReceISOHeader[inCnt], 2))
		return (VS_ERROR);

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_TICKET_ISOOnlineAnalyse
Date&Time       :2016/9/14 上午 10:17
Describe        :
*/
int inNCCC_TICKET_ISOOnlineAnalyse(TRANSACTION_OBJECT *pobTran)
{
	if (pobTran->inISOTxnCode == _SETTLE_		||
	    pobTran->inTransactionCode == _CLS_SETTLE_)
        {
		if (inNCCC_TICKET_SetReversalCnt(pobTran, _RESET_) == VS_ERROR)
		{
			return (VS_ERROR);
		}

		/* 結帳成功 把請先結帳的bit關掉 */
		inNCCC_TICKET_SetMustSettleBit(pobTran, "N");
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inNCCC_TICKET_ISOAdviceAnalyse
Date&Time       :2016/9/14 上午 10:17
Describe        :
*/
int inNCCC_TICKET_ISOAdviceAnalyse(TRANSACTION_OBJECT *pobTran, unsigned char *uszTcUpload)
{
	if (!memcmp(&pobTran->srTRec.szRespCode[0], "00", 2))
        {
		return (VS_SUCCESS);
        }
        else
        {
		/* 只有結帳時前送advice錯誤時要顯示錯誤訊息，在inNCCC_ATS_ISOAdviceAnalyse裡顯示錯誤訊息 */
		/* 結帳交易流程中，若於前帳前補送電文，補送電文有拒絕或其他回覆碼的狀況，畫面皆顯示結帳失敗即可，不須顯示補送電文之回覆碼訊息。 */
		
		return (VS_ERROR);
        }
	
}

int inNCCC_TICKET_BitMapSet(int *inBitMap, int inFeild)
{
        int 	i, inBMapCnt, inBitMapCnt;
        int 	inBMap[_NCCC_TICKET_MAX_BIT_MAP_CNT_];

        memset((char *) inBMap, 0x00, sizeof(inBMap));
        inBMapCnt = inBitMapCnt = 0;

        for (i = 0; i < 64; i++)
        {
                if (inBitMap[inBitMapCnt] > inFeild)
                {
                        if (i == 0)
                        {
                                /* 第一個BitMap */
                                inBMap[inBMapCnt++] = inFeild;
                                break;
                        }
                }
                else if (inBitMap[inBitMapCnt] < inFeild)
                {
                        inBMap[inBMapCnt++] = inBitMap[inBitMapCnt++];
                        if (inBitMap[inBitMapCnt] == 0 || inBitMap[inBitMapCnt] > inFeild) /* 最後一個 BitMap */
                        {
                                inBMap[inBMapCnt++] = inFeild;
                                break;
                        }
                }
                else
                        break; /* if (*inBitMap == inFeild) */
        }

        for (i = 0;; i++)
        {
                if (inBitMap[inBitMapCnt] != 0)
                        inBMap[inBMapCnt++] = inBitMap[inBitMapCnt++];
                else
                        break;
        }

        memcpy((char *) inBitMap, (char *) inBMap, sizeof(inBMap));

        return (VS_SUCCESS);
}

int inNCCC_TICKET_BitMapReset(int *inBitMap, int inFeild)
{
        int 	i, inBMapCnt;
        int 	inBMap[_NCCC_TICKET_MAX_BIT_MAP_CNT_];

        memset((char *) inBMap, 0x00, sizeof(inBMap));
        inBMapCnt = 0;

        for (i = 0; i < 64; i++)
        {
                if (inBitMap[i] == 0)
                        break;
                else if (inBitMap[i] != inFeild)
                        inBMap[inBMapCnt++] = inBitMap[i];
        }

        memcpy((char *) inBitMap, (char *) inBMap, sizeof(inBMap));

        return (VS_SUCCESS);
}

int inNCCC_TICKET_BitMapCheck(unsigned char *inBitMap, int inFeild)
{
        int 	inByteIndex, inBitIndex;

        inFeild--;
        inByteIndex = inFeild / 8;
        inBitIndex = 7 - (inFeild - inByteIndex * 8);

        if (_NCCC_TICKET_CHECK_BIT_MAP_(inBitMap[inByteIndex], inBitIndex))
        {
                return (VS_TRUE);
        }

        return (VS_FALSE);
}

/*
Function        :inNCCC_TICKET_CopyBitMap
Date&Time       :
Describe        :
*/
int inNCCC_TICKET_CopyBitMap(int *inBitMap, int *inSearchBitMap)
{
        int 	i;

        for (i = 0; i < _NCCC_TICKET_MAX_BIT_MAP_CNT_; i++)
        {
                if (inSearchBitMap[i] == 0)
                        break;
                else
                        inBitMap[i] = inSearchBitMap[i];
        }

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_TICKET_GetBitMapTableIndex
Date&Time       :
Describe        :
*/
int inNCCC_TICKET_GetBitMapTableIndex(ISO_TYPE_NCCC_TICKET_TABLE *srISOFunc, int inBitMapTxnCode)
{
        int 	inBitMapIndex;

        for (inBitMapIndex = 0;; inBitMapIndex++)
        {
                if (srISOFunc->srBitMap[inBitMapIndex].inTxnID == _NCCC_TICKET_NULL_TX_)
                {
                        return (VS_ERROR); /* 找不到相對應交易別的Bit Map */
                }

                if (srISOFunc->srBitMap[inBitMapIndex].inTxnID == inBitMapTxnCode)
                        break; /* 找到一樣的交易類別 */
        }

        return (inBitMapIndex);
}

/*
Function        :inNCCC_TICKET_GetBitMapMessagegTypeField03
Date&Time       :2016/9/14 下午 1:34
Describe        :
*/
int inNCCC_TICKET_GetBitMapMessagegTypeField03(TRANSACTION_OBJECT *pobTran, ISO_TYPE_NCCC_TICKET_TABLE *srISOFunc, int inIsoType, int *inTxnBitMap, unsigned char *uszSendBuf)
{
        int 		i, j, k, inBitMapIndex, *inBitMap, inCnt, inBitMapTxnCode = -1;
        char		szTemplate[64 + 1];
	unsigned char 	uszBuf;
       

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_GetBitMapMessagegTypeField03() START!");
        
        if (ginDebug == VS_TRUE)
        {
                sprintf(szTemplate, "inIsoType = %d", inIsoType);
                inLogPrintf(AT, szTemplate);
        }      
        
        /* 設定交易別 */
        inBitMapTxnCode = inIsoType;
      
        /* 要搜尋 BIT_MAP_NCCC_TICKET_TABLE srNCCC_TICKET_ISOBitMap 相對應的 inTxnID */
        if ((inBitMapIndex = inNCCC_TICKET_GetBitMapTableIndex(srISOFunc, inBitMapTxnCode)) == VS_ERROR)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inNCCC_TICKET_GetBitMapTableIndex == VS_ERROR");
                
                return (VS_ERROR);
        }

        /* Pack Message Type */
        inCnt = 0;
        inCnt += srISOFunc->inPackMTI(pobTran, inIsoType, &uszSendBuf[inCnt], srISOFunc->srBitMap[inBitMapIndex].szMTI);

        /* 要搜尋 BIT_MAP_NCCC_TICKET_TABLE srNCCC_TICKET_ISOBitMap 相對應的 inBitMap */
        inNCCC_TICKET_CopyBitMap(inTxnBitMap, srISOFunc->srBitMap[inBitMapIndex].inBitMap);

        /* 修改 Bit Map */
        srISOFunc->inModifyBitMap(pobTran, inIsoType, inTxnBitMap);
        /* Pack Bit Map */
        inBitMap = inTxnBitMap;
        for (i = 0;; i++)
        {
                uszBuf = 0x80;
                if (*inBitMap == 0)
                        break;

                j = *inBitMap / 8;
                k = *inBitMap % 8;

                if (k == 0)
                {
                        j--;
                        k = 8;
                }

                k--;

                while (k)
                {
                        uszBuf = uszBuf >> 1;
                        k--;
                }

                uszSendBuf[inCnt + j] += uszBuf;
                inBitMap++;
        }

        inCnt += _NCCC_TICKET_BIT_MAP_SIZE_;

        /* Process Code */
	/* 將MAC初始化 */
	memset(gszMAC_F_03, 0x00, sizeof(gszMAC_F_03));
	memset(gszMAC_F_04, 0x00, sizeof(gszMAC_F_04));
	memset(gszMAC_F_11, 0x00, sizeof(gszMAC_F_11));
	memset(gszMAC_F_35, 0x00, sizeof(gszMAC_F_35));
	memset(gszMAC_F_59, 0x00, sizeof(gszMAC_F_59));
	memset(gszMAC_F_63, 0x00, sizeof(gszMAC_F_63));
	
        memset(guszNCCC_TICKET_ISO_Field03, 0x00, sizeof(guszNCCC_TICKET_ISO_Field03));
        inFunc_ASCII_to_BCD(&guszNCCC_TICKET_ISO_Field03[0], srISOFunc->srBitMap[inBitMapIndex].szPCode, 3);

        if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
        {
                /* 
                        0800 :
                                535000 LogOn            535200  SAM註冊
                        
                        0100 :        
        	                530000	購貨            531000	自動加值
        	                530200  取消購貨(X)     532000	退貨            
        	                533000	現金加值        533200  現金加值取消
                                538000  詢卡
                                
                        0220 :
                                538100  鎖卡            538200	票值回覆    
                                538300  關閉自動加值
                                其餘同0100  
                                
                        0400 :   
                                同0100   
                                
                        0500 :
                                519200  Close Batch     519600  Repeat Close Batch after Batch upload             
                */
                
                if (inBitMapTxnCode == _ADVICE_)
                {
                        guszNCCC_TICKET_ISO_Field03[0] = 0x53;
                        
                        if (pobTran->srTRec.uszBlackListBit == VS_TRUE) 
                                guszNCCC_TICKET_ISO_Field03[1] = 0x81; 
                        else if (pobTran->srTRec.uszResponseBit == VS_TRUE) 
                                guszNCCC_TICKET_ISO_Field03[1] = 0x82;   
                        else if (pobTran->srTRec.uszCloseAutoTopUpBit == VS_TRUE)  
                                guszNCCC_TICKET_ISO_Field03[1] = 0x83;     
                        else
                        {
                                switch (pobTran->srTRec.inCode)
			        {
			                case _TICKET_IPASS_DEDUCT_ :
			                        guszNCCC_TICKET_ISO_Field03[1] = 0x00; 
			                        break;
			                case _TICKET_IPASS_REFUND_ :
			                        guszNCCC_TICKET_ISO_Field03[1] = 0x20; 
			                        break;
			                case _TICKET_IPASS_INQUIRY_ :
			                case _TICKET_IPASS_QUERY_ :        
			                        break;
			                case _TICKET_IPASS_AUTO_TOP_UP_ : 
			                        guszNCCC_TICKET_ISO_Field03[1] = 0x10; 
			                        break; 
			                case _TICKET_IPASS_TOP_UP_ : 
			                        guszNCCC_TICKET_ISO_Field03[1] = 0x30; 
			                        break;     
			                case _TICKET_IPASS_VOID_TOP_UP_ : 
			                        guszNCCC_TICKET_ISO_Field03[1] = 0x32; 
			                        break;            
			                default :
			                        break;         
			        }  
                        }                            
                } 
                else if (inBitMapTxnCode == _REVERSAL_)    
                {
                        guszNCCC_TICKET_ISO_Field03[0] = 0x53;
                        
                        switch (pobTran->srTRec.inCode)
		        {
		                case _TICKET_IPASS_DEDUCT_ :
		                        guszNCCC_TICKET_ISO_Field03[1] = 0x00; 
		                        break;
		                case _TICKET_IPASS_REFUND_ :
		                        guszNCCC_TICKET_ISO_Field03[1] = 0x20; 
		                        break;
		                case _TICKET_IPASS_AUTO_TOP_UP_ : 
		                        guszNCCC_TICKET_ISO_Field03[1] = 0x10; 
		                        break; 
		                case _TICKET_IPASS_TOP_UP_ : 
		                        guszNCCC_TICKET_ISO_Field03[1] = 0x30; 
		                        break;     
		                case _TICKET_IPASS_VOID_TOP_UP_ : 
		                        guszNCCC_TICKET_ISO_Field03[1] = 0x32; 
		                        break;            
		                default :
		                        break;         
		        }  
                }           
        }        
        else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
        {
                guszNCCC_TICKET_ISO_Field03[0] = 0x51;
                        
                if (inBitMapTxnCode == _ADVICE_ || inBitMapTxnCode == _REVERSAL_)
                {
			if (pobTran->srTRec.uszBlackListBit == VS_TRUE)
			{
                                guszNCCC_TICKET_ISO_Field03[1] = 0x81;
			}
                        else
                        {
				switch (pobTran->srTRec.inCode)
				{
					case _TICKET_EASYCARD_DEDUCT_ :
						guszNCCC_TICKET_ISO_Field03[1] = 0x00; 
						break;
					case _TICKET_EASYCARD_REFUND_ :
						guszNCCC_TICKET_ISO_Field03[1] = 0x20; 
						break;
					case _TICKET_EASYCARD_TOP_UP_ :
						guszNCCC_TICKET_ISO_Field03[1] = 0x30; 
						break;
					case _TICKET_EASYCARD_AUTO_TOP_UP_ :
						guszNCCC_TICKET_ISO_Field03[1] = 0x10; 
						break;
					case _TICKET_EASYCARD_VOID_TOP_UP_ : 
						guszNCCC_TICKET_ISO_Field03[1] = 0x32; 
						break;
					default :
						break;         
				}
			}
		}
        }
	else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
        {
                /*
                        0800 :
                                585200  SAM Unlock      585000  Get PIN
                                585100  LogOn

                        0100 :
        	                580000	購貨            581000	自動加值
        	                580200  取消購貨(X)     582000	退貨
        	                583000	現金加值        583200  現金加值取消
                                588000  詢卡

                        0102 :
                                581000	自動加值        582000	退貨
                                583000	現金加值

                        0220 :
                                588100  鎖卡            588400	聯名卡開卡資料(自動加值設定)
                                其餘同0100

                        0400 :
                                無Reversal

                        0500 :
                                519200  Close Batch     519600  Repeat Close Batch after Batch upload
                */

                if (inBitMapTxnCode == _ADVICE_)
                {
                        guszNCCC_TICKET_ISO_Field03[0] = 0x58;

                        if (pobTran->srTRec.uszBlackListBit == VS_TRUE)
                                guszNCCC_TICKET_ISO_Field03[1] = 0x81;
                        else if (pobTran->srTRec.srICASHRec.uszTxLogBit == VS_TRUE)
                                guszNCCC_TICKET_ISO_Field03[1] = 0x84;
                        else
                        {
                                switch (pobTran->srTRec.inCode)
			        {
			                case _TICKET_ICASH_DEDUCT_ :
			                        guszNCCC_TICKET_ISO_Field03[1] = 0x00;
			                        break;
			                case _TICKET_ICASH_REFUND_ :
			                        guszNCCC_TICKET_ISO_Field03[1] = 0x20;
			                        break;
			                case _TICKET_ICASH_INQUIRY_ :
			                        break;
			                case _TICKET_ICASH_AUTO_TOP_UP_ :
			                        guszNCCC_TICKET_ISO_Field03[1] = 0x10;
			                        break;
			                case _TICKET_ICASH_TOP_UP_ :
			                        guszNCCC_TICKET_ISO_Field03[1] = 0x30;
			                        break;
			                case _TICKET_ICASH_VOID_TOP_UP_ :
			                        guszNCCC_TICKET_ISO_Field03[1] = 0x32;
			                        break;
			                default :
			                        break;
			        }
                        }
                }
                else if (inBitMapTxnCode == _REVERSAL_)
                {
                        guszNCCC_TICKET_ISO_Field03[0] = 0x58;

                        switch (pobTran->srTRec.inCode)
		        {
		                case _TICKET_ICASH_DEDUCT_ :
		                        guszNCCC_TICKET_ISO_Field03[1] = 0x00;
		                        break;
		                case _TICKET_ICASH_REFUND_ :
		                        guszNCCC_TICKET_ISO_Field03[1] = 0x20;
		                        break;
		                case _TICKET_ICASH_AUTO_TOP_UP_ :
		                        guszNCCC_TICKET_ISO_Field03[1] = 0x10;
		                        break;
		                case _TICKET_ICASH_TOP_UP_ :
		                        guszNCCC_TICKET_ISO_Field03[1] = 0x30;
		                        break;
		                case _TICKET_ICASH_VOID_TOP_UP_ :
		                        guszNCCC_TICKET_ISO_Field03[1] = 0x32;
		                        break;
		                default :
		                        break;
		        }
                }
        }

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_GetBitMapMessagegTypeField03() END!");
        
        return (inCnt);
}

int inNCCC_TICKET_PackISO(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendBuf, int inTxnCode)
{
        int				i = 0, inSendCnt = 0, inField = 0, inCnt = 0;
        int				inBitMap[_NCCC_TICKET_MAX_BIT_MAP_CNT_ + 1];
        int				inRetVal, inISOFuncIndex = -1;
	char				szTemplate[42 + 1] = {0};
        char				szDebugMsg[100 + 1] = {0};
	char				szCommMode[1 + 1] = {0};
	char				szNCCCFesMode[2 + 1] = {0};
	char				szCFESMode[2 + 1] = {0};
	char				szIFESMode[2 + 1] = {0};
        unsigned char			uszBCD[20 + 1] = {0};
        ISO_TYPE_NCCC_TICKET_TABLE 	srISOFunc;		
	
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_PackISO() START!");

        inSendCnt = 0;
        inField = 0;

        /* 加密模式，預設值 = 0。【0 = 不加密，1 = tSAM 加密】 */
	memset(szCommMode, 0x00, sizeof(szCommMode));
	inGetCommMode(szCommMode);
	memset(szNCCCFesMode, 0x00, sizeof(szNCCCFesMode));
	inGetNCCCFESMode(szNCCCFesMode);
	memset(szCFESMode, 0x00, sizeof(szCFESMode));
	inGetCloud_MFES(szCFESMode);
	memset(szIFESMode, 0x00, sizeof(szIFESMode));
	inGetI_FES_Mode(szIFESMode);
	
	if (memcmp(szCommMode, _COMM_MODEM_MODE_, strlen(_COMM_MODEM_MODE_)) == 0)
	{
		/* 初始化tSAM加密用欄位 */
		inNCCC_Func_Init_tSAM_Data();
		
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Ticket Not Support CommMode : %s", szCommMode);
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	else if (memcmp(szCommMode, _COMM_ETHERNET_MODE_, strlen(_COMM_ETHERNET_MODE_)) == 0)
	{
		/* 初始化tSAM加密用欄位 */
		inNCCC_Func_Init_tSAM_Data();
		
		if (memcmp(szNCCCFesMode, _NCCC_05_ATS_MODE_, strlen(_NCCC_05_ATS_MODE_)) == 0	||
		    memcmp(szNCCCFesMode, _NCCC_04_MPAS_MODE_, strlen(_NCCC_04_MPAS_MODE_)) == 0)
		{
			/* 【需求單-109057】電票交易電文新增tSAM加密及MAC需求 只有IFES要tSAM加密 */
			if (memcmp(szIFESMode, "Y", 1) == 0)
			{
				inISOFuncIndex = 2;
			}
			else
			{
				inISOFuncIndex = 0;
			}
		}
		else if (memcmp(szNCCCFesMode, _NCCC_03_MFES_MODE_, strlen(_NCCC_03_MFES_MODE_)) == 0)
		{
			/* 【需求單-109057】電票交易電文新增tSAM加密及MAC需求 只有IFES要tSAM加密 */
			if (memcmp(szIFESMode, "Y", 1) == 0)
			{
				if (memcmp(szCFESMode, "Y", strlen("Y")) == 0)
				{
					inISOFuncIndex = 2;
				}
				else
				{
					inISOFuncIndex = 1;
				}
			}
			else
			{
				if (memcmp(szCFESMode, "Y", strlen("Y")) == 0)
				{
					inISOFuncIndex = 0;
				}
				else
				{
					inISOFuncIndex = 1;
				}
			}
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Ticket Not Support FesMode : %s", szNCCCFesMode);
				inLogPrintf(AT, szDebugMsg);
			}
			return (VS_ERROR);
		}
	}
	else if (memcmp(szCommMode, _COMM_GPRS_MODE_, 1) == 0	||
		 memcmp(szCommMode, _COMM_3G_MODE_, 1) == 0	||
		 memcmp(szCommMode, _COMM_4G_MODE_, 1) == 0)
	{
		/* 初始化tSAM加密用欄位 */
		inNCCC_Func_Init_tSAM_Data();
		
		if (memcmp(szNCCCFesMode, _NCCC_05_ATS_MODE_, strlen(_NCCC_05_ATS_MODE_)) == 0	||
		    memcmp(szNCCCFesMode, _NCCC_04_MPAS_MODE_, strlen(_NCCC_04_MPAS_MODE_)) == 0)
		{
			/* 【需求單-109057】電票交易電文新增tSAM加密及MAC需求 只有IFES要tSAM加密 */
			if (memcmp(szIFESMode, "Y", 1) == 0)
			{
				inISOFuncIndex = 2;
			}
			else
			{
				inISOFuncIndex = 0;
			}
		}
		else if (memcmp(szNCCCFesMode, _NCCC_03_MFES_MODE_, strlen(_NCCC_03_MFES_MODE_)) == 0)
		{
			/* 【需求單-109057】電票交易電文新增tSAM加密及MAC需求 只有IFES要tSAM加密 */
			if (memcmp(szIFESMode, "Y", 1) == 0)
			{
				if (memcmp(szCFESMode, "Y", strlen("Y")) == 0)
				{
					inISOFuncIndex = 2;
				}
				else
				{
					inISOFuncIndex = 1;
				}
			}
			else
			{
				if (memcmp(szCFESMode, "Y", strlen("Y")) == 0)
				{
					inISOFuncIndex = 0;
				}
				else
				{
					inISOFuncIndex = 1;
				}
			}
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Ticket Not Support FesMode : %s", szNCCCFesMode);
				inLogPrintf(AT, szDebugMsg);
			}
			return (VS_ERROR);
		}
	}
	else if (memcmp(szCommMode, _COMM_WIFI_MODE_, strlen(_COMM_WIFI_MODE_)) == 0)
	{
		/* 初始化tSAM加密用欄位 */
		inNCCC_Func_Init_tSAM_Data();
		
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Ticket Not Support CommMode : %s", szCommMode);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		/* 初始化tSAM加密用欄位 */
		inNCCC_Func_Init_tSAM_Data();
		
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg,"No such CommMode! szCommMode = %d", atoi(szCommMode));
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR); /* 防呆 */
	}
	
	if (inISOFuncIndex >= 3 || inISOFuncIndex < 0)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg,"szEncryptMode ERROR!! szEncryptMode = %d",inISOFuncIndex);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR); /* 防呆 */
	}
	
        /* 決定要執行第幾個 Function Index */
        memset((char *) &srISOFunc, 0x00, sizeof(srISOFunc));
        memcpy((char *) &srISOFunc, (char *) &srNCCC_TICKET_ISOFunc[inISOFuncIndex], sizeof(srISOFunc));
        memset((char *) inBitMap, 0x00, sizeof(inBitMap));

        /* 開始組 ISO 電文 */
        /* Pack TPDU */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        if (inGetTPDU(szTemplate) != VS_SUCCESS)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inGetTPDU() ERROR!");

                return (VS_ERROR);
        }

        memset(uszBCD, 0x00, sizeof(uszBCD));
        inFunc_ASCII_to_BCD(uszBCD, szTemplate, 5);
        memcpy((char *) &uszSendBuf[inSendCnt], (char *) uszBCD, _NCCC_TICKET_TPDU_SIZE_);
        inSendCnt += _NCCC_TICKET_TPDU_SIZE_;
        /* Get Bit Map / Mesage Type / Processing Code */
        inRetVal = inNCCC_TICKET_GetBitMapMessagegTypeField03(pobTran, &srISOFunc, inTxnCode, inBitMap, &uszSendBuf[inSendCnt]);
        if (inRetVal == VS_ERROR)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inNCCC_TICKET_GetBitMapMessagegTypeField03() ERROR!");

                return (VS_ERROR);
        }
        else
                inSendCnt += inRetVal; /* Bit Map 長度 */

        for (i = 0;; i++)
        {
		/* BitMap的最後一個0 */
                if (inBitMap[i] == 0)
                        break;

		/* srPackISO會跳過BitMap沒有的欄位 */
                while (inBitMap[i] > srISOFunc.srPackISO[inField].inFieldNum)
                {
                        inField++;
                }

		/* BitMap中有的欄位就pack進去 */
                if (inBitMap[i] == srISOFunc.srPackISO[inField].inFieldNum)
                {
			/* 抓Before的Index */
			if (srISOFunc.srPackISO[inField].inFieldNum == 56)
			{
				ginBeforeIndex = inSendCnt;
			}
			
                        inCnt = srISOFunc.srPackISO[inField].inISOLoad(pobTran, &uszSendBuf[inSendCnt]);
                        
			/* pack的字數小於等於0，一定出錯 */
                        if (inCnt <= 0)
                        {
                                if (ginDebug == VS_TRUE)
                                {
                                        memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                                        sprintf(szDebugMsg, "inField = %d Error", srISOFunc.srPackISO[inField].inFieldNum);
                                        inLogPrintf(AT, szDebugMsg);
                                }

                                return (VS_ERROR);
                        }
                        else
                                inSendCnt += inCnt;
                }
		
        }

        if (srISOFunc.inModifyPackData != _NCCC_TICKET_NULL_TX_)
                srISOFunc.inModifyPackData(pobTran, uszSendBuf, &inSendCnt);
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_PackISO() END!");

        return (inSendCnt);
}

int inNCCC_TICKET_CheckUnPackField(int inField, ISO_FIELD_NCCC_TICKET_TABLE *srCheckUnPackField)
{
        int 	i;

        for (i = 0; i < 64; i++)
        {
                if (srCheckUnPackField[i].inFieldNum == 0)
                        break;
                else if (srCheckUnPackField[i].inFieldNum == inField)
                {
                        return (VS_SUCCESS); /* i 是 ISO_FIELD_TABLE 裡的第幾個 */
                }
        }

        return (VS_ERROR);
}

int inNCCC_TICKET_GetCheckField(int inField, ISO_CHECK_NCCC_TICKET_TABLE *ISOFieldCheck)
{
        int 	i;

        for (i = 0; i < 64; i++)
        {
                if (ISOFieldCheck[i].inFieldNum == 0)
                        break;
                else if (ISOFieldCheck[i].inFieldNum == inField)
                {
                        return (i); /* i 是 ISO_CHECK_NCCC_TICKET_TABLE 裡的第幾個 */
                }
        }

        return (VS_ERROR);
}

int inNCCC_TICKET_GetFieldLen(int inField, unsigned char *uszSendData, ISO_FIELD_TYPE_NCCC_TICKET_TABLE *srFieldType, int inTicketType)
{
        int 	inCnt = 0, i, inLen;

        for (i = 0;; i++)
        {
                if (srFieldType[i].inFieldNum == 0)
                        break;

                if (srFieldType[i].inFieldNum != inField)
                        continue;

                switch (srFieldType[i].inFieldType)
                {
                        case _NCCC_TICKET_ISO_ASC_:
                                inCnt += srFieldType[i].inFieldLen;
                                break;
                        case _NCCC_TICKET_ISO_BCD_:
                                inCnt += srFieldType[i].inFieldLen / 2;
                                break;
                        case _NCCC_TICKET_ISO_NIBBLE_2_:
                                inLen = (uszSendData[0] / 16 * 10) + (uszSendData[0] % 16);
				/* Smart Pay卡號上傳格式(Field_35)不同一般信用卡。一般信用卡的卡號需要Pack成 BCD Code，Smart pay的卡號/帳號直接上傳ASCII Code不進行Pack。 */
				/* 因為reversal不讀batch，沒辦法知道fiscBit，所以還是只能強制用長度判斷 */
				if (inLen == 54 && srFieldType[i].inFieldNum == 35)
					inCnt += inLen + 1;
				else
					inCnt += ((inLen + 1) / 2) + 1;
                                break;
                        case _NCCC_TICKET_ISO_NIBBLE_3_:
                                inLen = (uszSendData[0] / 16 * 1000) + (uszSendData[0] % 16 * 100) + (uszSendData[1] / 16 * 10) + (uszSendData[1] % 16);
                                inCnt += ((inLen + 1) / 2) + 2;
                                break;
                        case _NCCC_TICKET_ISO_BYTE_2_:
                                inLen = (uszSendData[0] / 16 * 10) + (uszSendData[0] % 16);
                                inCnt += inLen + 1;
                                break;
                        case _NCCC_TICKET_ISO_BYTE_3_:
                                inLen = (uszSendData[0] / 16 * 1000) + (uszSendData[0] % 16 * 100) + (uszSendData[1] / 16 * 10) + (uszSendData[1] % 16);
                                inCnt += inLen + 2;
                                break;
                        case _NCCC_TICKET_ISO_BYTE_2_H_:
                                inLen = (int) uszSendData[0];
                                inCnt += inLen + 1;
                                break;
                        case _NCCC_TICKET_ISO_BYTE_3_H_:
                                inLen = ((int) uszSendData[0] * 0xFF) + (int) uszSendData[1];
                                inCnt += inLen + 1;
                                break;
			case _NCCC_TICKET_ISO_BYTE_1_:
				inLen = (uszSendData[0] / 16 * 10) + (uszSendData[0] % 16);
				inCnt += inLen + 1;
				break;
			case _NCCC_TICKET_ETICKET_CARD_:
				if (inTicketType == _TICKET_TYPE_IPASS_	||
				    inTicketType == _TICKET_TYPE_ECC_ 	||
				    inTicketType == _TICKET_TYPE_ICASH_)
			        {
			                inLen = (uszSendData[0] / 16 * 10) + (uszSendData[0] % 16);
				        inCnt += inLen + 1;
				}
				else
				{
					inLen = (uszSendData[0] / 16 * 10) + (uszSendData[0] % 16);
				        inCnt += inLen + 1;
				}
				break;
                        default:
                                break;
                }

                break;
        }

        return (inCnt);
}

int inNCCC_TICKET_GetFieldIndex(int inField, ISO_FIELD_TYPE_NCCC_TICKET_TABLE *srFieldType)
{
	int 	i;

	for (i = 0 ;; i ++)
	{
		if (srFieldType[i].inFieldNum == 0)
			break;

		if (srFieldType[i].inFieldNum == inField)
			return (i);
	}

	return (VS_ERROR);
}

int inNCCC_TICKET_UnPackISO(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendBuf, unsigned char *uszRecvBuf)
{
	int				inRetVal = VS_ERROR;
        int				i, inRecvCnt, inCnt, inSendCnt, inSendField, inRecvField, inISOFuncIndex = -1;
        char				szBuf[_NCCC_TICKET_TPDU_SIZE_ + _NCCC_TICKET_MTI_SIZE_ + _NCCC_TICKET_BIT_MAP_SIZE_ + 1];
        char				szDebugMsg[100 + 1] = {0};
	char				szNCCCFesMode[2 + 1] = {0};
	char				szCommMode[1 + 1] = {0};
	char				szCFESMode[2 + 1] = {0};
        unsigned char			uszSendMap[_NCCC_TICKET_BIT_MAP_SIZE_ + 1], uszReceMap[_NCCC_TICKET_BIT_MAP_SIZE_ + 1];
        ISO_TYPE_NCCC_TICKET_TABLE 	srISOFunc;


        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_UnPackISO() START!");

        inSendField = inRecvField = 0;
        inSendCnt = inRecvCnt = 0;

        memset((char *) uszSendMap, 0x00, sizeof(uszSendMap));
        memset((char *) uszReceMap, 0x00, sizeof(uszReceMap));
        memset((char *) szBuf, 0x00, sizeof(szBuf));
	
	/* 加密模式，預設值 = 0。【0 = 不加密，1 = tSAM 加密】 */
	memset(szCommMode, 0x00, sizeof(szCommMode));
	inGetCommMode(szCommMode);
	if (memcmp(szCommMode, _COMM_MODEM_MODE_, strlen(_COMM_MODEM_MODE_)) == 0)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Ticket Not Support CommMode : %s", szCommMode);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	else if (memcmp(szCommMode, _COMM_ETHERNET_MODE_, strlen(_COMM_ETHERNET_MODE_)) == 0)
	{
		memset(szNCCCFesMode, 0x00, sizeof(szNCCCFesMode));
		inGetNCCCFESMode(szNCCCFesMode);
		if (memcmp(szNCCCFesMode, _NCCC_05_ATS_MODE_, strlen(_NCCC_05_ATS_MODE_)) == 0	||
		    memcmp(szNCCCFesMode, _NCCC_04_MPAS_MODE_, strlen(_NCCC_04_MPAS_MODE_)) == 0)
		{
			inISOFuncIndex = 0;
		}
		else if (memcmp(szNCCCFesMode, _NCCC_03_MFES_MODE_, strlen(_NCCC_03_MFES_MODE_)) == 0)
		{
			memset(szCFESMode, 0x00, sizeof(szCFESMode));
			inGetCloud_MFES(szCFESMode);
			if (memcmp(szCFESMode, "Y", strlen("Y")) == 0)
			{
				inISOFuncIndex = 0;
			}
			else
			{
				inISOFuncIndex = 1;
			}
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Ticket Not Support FesMode : %s", szNCCCFesMode);
				inLogPrintf(AT, szDebugMsg);
			}
		}
	}
	else if (memcmp(szCommMode, _COMM_GPRS_MODE_, 1) == 0 ||
		 memcmp(szCommMode, _COMM_3G_MODE_, 1) == 0 ||
		 memcmp(szCommMode, _COMM_4G_MODE_, 1) == 0)
	{
		memset(szNCCCFesMode, 0x00, sizeof(szNCCCFesMode));
		inGetNCCCFESMode(szNCCCFesMode);
		if (memcmp(szNCCCFesMode, _NCCC_05_ATS_MODE_, strlen(_NCCC_05_ATS_MODE_)) == 0	||
		    memcmp(szNCCCFesMode, _NCCC_04_MPAS_MODE_, strlen(_NCCC_04_MPAS_MODE_)) == 0)
		{
			inISOFuncIndex = 0;
		}
		else if (memcmp(szNCCCFesMode, _NCCC_03_MFES_MODE_, strlen(_NCCC_03_MFES_MODE_)) == 0)
		{
			memset(szCFESMode, 0x00, sizeof(szCFESMode));
			inGetCloud_MFES(szCFESMode);
			if (memcmp(szCFESMode, "Y", strlen("Y")) == 0)
			{
				inISOFuncIndex = 0;
			}
			else
			{
				inISOFuncIndex = 1;
			}
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Ticket Not Support FesMode : %s", szNCCCFesMode);
				inLogPrintf(AT, szDebugMsg);
			}
			return (VS_ERROR);
		}
	}
	else if (memcmp(szCommMode, _COMM_WIFI_MODE_, strlen(_COMM_WIFI_MODE_)) == 0)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Ticket Not Support CommMode : %s", szCommMode);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg,"No such CommMode! szCommMode = %d", atoi(szCommMode));
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR); /* 防呆 */
	}

	/*  */
	if (inISOFuncIndex >= 2 || inISOFuncIndex < 0)
	{
                if (ginDebug == VS_TRUE)
                {
                        memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                        sprintf(szDebugMsg, "inISOFuncIndex : %d", inISOFuncIndex);
                        inLogPrintf(AT, szDebugMsg);
                }
                
		return (VS_ERROR); /* 防呆 */
	}
	
        memset((char *) &srISOFunc, 0x00, sizeof(srISOFunc));
        memcpy((char *) &srISOFunc, (char *) &srNCCC_TICKET_ISOFunc[inISOFuncIndex], sizeof(srISOFunc));

        if (srISOFunc.inCheckISOHeader != NULL)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "srISOFunc.inCheckISOHeader != NULL");

                if (srISOFunc.inCheckISOHeader(pobTran, (char *) &uszSendBuf[inRecvCnt], (char *) &uszRecvBuf[inRecvCnt]) != VS_SUCCESS)
                {
                        if (ginDebug == VS_TRUE)
                                inLogPrintf(AT, "srISOFunc.inCheckISOHeader ERROR");

                        return (_TRAN_RESULT_UNPACK_ERR_);
                }
        }

        inSendCnt += _NCCC_TICKET_TPDU_SIZE_;
        inRecvCnt += _NCCC_TICKET_TPDU_SIZE_;
        inSendCnt += _NCCC_TICKET_MTI_SIZE_;
        inRecvCnt += _NCCC_TICKET_MTI_SIZE_;

        memcpy((char *) uszSendMap, (char *) &uszSendBuf[inSendCnt], _NCCC_TICKET_BIT_MAP_SIZE_);
        memcpy((char *) uszReceMap, (char *) &uszRecvBuf[inRecvCnt], _NCCC_TICKET_BIT_MAP_SIZE_);

        inSendCnt += _NCCC_TICKET_BIT_MAP_SIZE_;
        inRecvCnt += _NCCC_TICKET_BIT_MAP_SIZE_;

        /* 先檢查 ISO Field_39 */
        if (inNCCC_TICKET_BitMapCheck(uszReceMap, 39) == VS_FALSE)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inNCCC_TICKET_BitMapCheck(39) ERROR");

                return (VS_ERROR);
        }

        if (inNCCC_TICKET_BitMapCheck(uszReceMap, 41) == VS_FALSE)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inNCCC_TICKET_BitMapCheck(41) ERROR");

                return (VS_ERROR);
        }

        for (i = 1; i <= 64; i++)
        {
                /* 有送出去的 Field 但沒有收回來的 Field */
                if (inNCCC_TICKET_BitMapCheck(uszSendMap, i) && !inNCCC_TICKET_BitMapCheck(uszReceMap, i))
                {
                        inSendCnt += inNCCC_TICKET_GetFieldLen(i, &uszSendBuf[inSendCnt], srISOFunc.srISOFieldType, pobTran->srTRec.inTicketType);
                }
                else if (inNCCC_TICKET_BitMapCheck(uszReceMap, i)) /* 收的 BitMap */
                {
                        if (inNCCC_TICKET_BitMapCheck(uszSendMap, i)) /* 送的 BitMap 都有 */
                        {
                                /* 是否要進行檢查封包資料包含【送】【收】 */
                                if ((inSendField = inNCCC_TICKET_GetCheckField(i, srISOFunc.srCheckISO)) != VS_ERROR)
                                {
                                        if (srISOFunc.srCheckISO[inSendField].inISOCheck(pobTran, &uszSendBuf[inSendCnt], &uszRecvBuf[inRecvCnt]) != VS_SUCCESS)
                                        {
                                                if (ginDebug == VS_TRUE)
                                                {
                                                        memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                                                        sprintf(szDebugMsg, "inSendField = %d Error!", srISOFunc.srCheckISO[inSendField].inFieldNum);
                                                        inLogPrintf(AT, szDebugMsg);
                                                }

                                                return (_TRAN_RESULT_UNPACK_ERR_);
                                        }
                                }

                                inSendCnt += inNCCC_TICKET_GetFieldLen(i, &uszSendBuf[inSendCnt], srISOFunc.srISOFieldType, pobTran->srTRec.inTicketType);
                        }

                        while (i > srISOFunc.srUnPackISO[inRecvField].inFieldNum) /* 只有收 */
                        {
                                inRecvField++;
                        }

                        if (i == srISOFunc.srUnPackISO[inRecvField].inFieldNum)
                        {
                                /* 要先檢查 UnpackISO 是否存在 */
                                if (inNCCC_TICKET_CheckUnPackField(i, srISOFunc.srUnPackISO) == VS_SUCCESS)
				{
                                        inRetVal =srISOFunc.srUnPackISO[inRecvField].inISOLoad(pobTran, &uszRecvBuf[inRecvCnt]);
					
					if (inRetVal != VS_SUCCESS)
					{
						if (ginDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
							sprintf(szDebugMsg, "inRecvField = %d Error!", srISOFunc.srUnPackISO[inRecvField].inFieldNum);
							inLogPrintf(AT, szDebugMsg);
						}

						return (_TRAN_RESULT_UNPACK_ERR_);
					}
				}
                        }
			
                        inCnt = inNCCC_TICKET_GetFieldLen(i, &uszRecvBuf[inRecvCnt], srISOFunc.srISOFieldType, pobTran->srTRec.inTicketType);
                        if (inCnt == VS_ERROR)
                        {
                                if (ginDebug == VS_TRUE)
                                {
                                        memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                                        sprintf(szDebugMsg, "inRecvField = %d Error!", srISOFunc.srUnPackISO[inRecvField].inFieldNum);
                                        inLogPrintf(AT, szDebugMsg);
                                }

                                return (_TRAN_RESULT_UNPACK_ERR_);
                        }

                        inRecvCnt += inCnt;
                }
        }

        /* 這裡表示已經解完電文要檢查是否有回 ISO Field_38 */
        if (!memcmp(&pobTran->srTRec.szRespCode[0], "00", 2))
        {
                switch (pobTran->inISOTxnCode)
                {
			case _CUP_LOGON_:
				if (inNCCC_TICKET_BitMapCheck(uszReceMap, 59) == VS_FALSE)
                                {
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "inNCCC_TICKET_BitMapCheck 59 Error!");
                                        }

                                        return (VS_ERROR);
                                }
				
				if (inNCCC_TICKET_BitMapCheck(uszReceMap, 60) == VS_FALSE)
                                {
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "inNCCC_TICKET_BitMapCheck 60 Error!");
                                        }

                                        return (VS_ERROR);
                                }

                                break;
				
			case _CUP_SALE_ :
			case _CUP_PRE_AUTH_ :
				if (inNCCC_TICKET_BitMapCheck(uszReceMap, 04) == VS_FALSE)
                                {
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "inNCCC_TICKET_BitMapCheck 04 Error!");
                                        }

                                        return (VS_ERROR);
                                }
				
				if (inNCCC_TICKET_BitMapCheck(uszReceMap, 59) == VS_FALSE)
                                {
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "inNCCC_TICKET_BitMapCheck 59 Error!");
                                        }

                                        return (VS_ERROR);
                                }

                                break;
				
			case _CUP_REFUND_:
			case _CUP_MAIL_ORDER_REFUND_:
			case _CUP_PRE_COMP_:
			case _CUP_VOID_:
			case _CUP_PRE_AUTH_VOID_:
			case _CUP_PRE_COMP_VOID_:
			case _INST_SALE_:
			case _INST_ADJUST_:
			case _INST_REFUND_:
			case _REDEEM_SALE_:
			case _REDEEM_ADJUST_:
			case _REDEEM_REFUND_:
				if (inNCCC_TICKET_BitMapCheck(uszReceMap, 04) == VS_FALSE)
                                {
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "inNCCC_TICKET_BitMapCheck 04 Error!");
                                        }

                                        return (VS_ERROR);
                                }
				
				if (inNCCC_TICKET_BitMapCheck(uszReceMap, 38) == VS_FALSE)
                                {
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "inNCCC_TICKET_BitMapCheck 38 Error!");
                                        }

                                        return (VS_ERROR);
                                }
				
				if (inNCCC_TICKET_BitMapCheck(uszReceMap, 59) == VS_FALSE)
                                {
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "inNCCC_TICKET_BitMapCheck 59 Error!");
                                        }

                                        return (VS_ERROR);
                                }

                                break;
				
                        case _SALE_:
                        case _REFUND_:
                        case _VOID_:
			case _TIP_:
                        case _PRE_AUTH_:
			case _PRE_COMP_:
			case _MAIL_ORDER_:
			case _CASH_ADVANCE_:
				if (inNCCC_TICKET_BitMapCheck(uszReceMap, 04) == VS_FALSE)
                                {
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "inNCCC_TICKET_BitMapCheck 04 Error!");
                                        }

                                        return (VS_ERROR);
                                }
				
                                if (inNCCC_TICKET_BitMapCheck(uszReceMap, 38) == VS_FALSE)
                                {
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "inNCCC_TICKET_BitMapCheck 38 Error!");
                                        }

                                        return (VS_ERROR);
                                }

                                break;
				
			case _FISC_SALE_:
			case _FISC_VOID_:
			case _FISC_REFUND_:
				if (inNCCC_TICKET_BitMapCheck(uszReceMap, 04) == VS_FALSE)
                                {
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "inNCCC_TICKET_BitMapCheck 04 Error!");
                                        }

                                        return (VS_ERROR);
                                }
				
                                if (inNCCC_TICKET_BitMapCheck(uszReceMap, 58) == VS_FALSE)
                                {
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "inNCCC_TICKET_BitMapCheck 58 Error!");
                                        }

                                        return (VS_ERROR);
                                }
				if (inNCCC_TICKET_BitMapCheck(uszReceMap, 59) == VS_FALSE)
                                {
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "inNCCC_TICKET_BitMapCheck 59 Error!");
                                        }

                                        return (VS_ERROR);
                                }

                                break;
                        default:
                                break;
                }
        }

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_UnPackISO() END!");

        return (VS_SUCCESS);
}

int inNCCC_TICKET_GetSTAN(TRANSACTION_OBJECT *pobTran)
{
        char 	szSTANNum[12 + 1];

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_GetSTAN() START!");

        memset(szSTANNum, 0x00, sizeof(szSTANNum));
        if (inGetSTANNum(szSTANNum) == VS_ERROR)
                return (VS_ERROR);

        pobTran->srTRec.lnSTAN = atol(szSTANNum);

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_TICKET_SetSTAN
Date&Time       :2015/12/24 早上 10:25
Describe        :STAN++
*/
int inNCCC_TICKET_SetSTAN(TRANSACTION_OBJECT *pobTran)
{
        long 	lnSTAN;
        char 	szSTANNum[12 + 1];

        memset(szSTANNum, 0x00, sizeof(szSTANNum));
        if (inGetSTANNum(szSTANNum) == VS_ERROR)
                return (VS_ERROR);

        lnSTAN = atol(szSTANNum);
        if (ginAPVersionType == _APVERSION_TYPE_NCCC_)
	{
		lnSTAN += 1;
		/* SPDH通訊編號只有兩碼，遇到100倍數要多跳一號 */
		if ((lnSTAN % 100) == 0)
		{
			lnSTAN += 1;
		}
			
		if (lnSTAN > 999999)
			lnSTAN = 1;
	}
        else
	{
		lnSTAN += 1;
		if (lnSTAN > 999999)
			lnSTAN = 1;
	}

        memset(szSTANNum, 0x00, sizeof(szSTANNum));
        sprintf(szSTANNum, "%06ld", lnSTAN);
        if (inSetSTANNum(szSTANNum) == VS_ERROR)
                return (VS_ERROR);

        if (inSaveHDPTRec(pobTran->srBRec.inHDTIndex) < 0)
        {
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_TICKET_ProcessReversal
Date&Time       :2016/9/13 下午 4:48
Describe        :
*/
int inNCCC_TICKET_ProcessReversal(TRANSACTION_OBJECT *pobTran)
{
        int 	inRetVal = VS_ERROR;
        char 	szTicket_ReversalBit[2 + 1] = {0};
	
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_ProcessReversal() START!");

	memset(szTicket_ReversalBit, 0x00, sizeof(szTicket_ReversalBit));
	if (inGetTicket_ReversalBit(szTicket_ReversalBit) == VS_ERROR)
                return (VS_ERROR);

	if (!memcmp(szTicket_ReversalBit, "Y", 1))
        {
                if ((inRetVal = inNCCC_TICKET_ReversalSendRecvPacket(pobTran)) != VS_SUCCESS)
                {
                        return (VS_ERROR);
                }

                if (inNCCC_TICKET_SetReversalCnt(pobTran, _ADD_) == VS_ERROR)
                {
                        return (VS_ERROR);
                }
        }
	
        /* 檢查是否要產生 Reversal */
        if (pobTran->uszReversalBit == VS_TRUE)
        {
		inRetVal = inNCCC_TICKET_ReversalSave_Flow(pobTran);
		if (inRetVal != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
        }
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_ProcessReversal() END!");
	
        return (VS_SUCCESS);
}

/*
Function        :inNCCC_TICKET_AdviceSendRecvPacket
Date&Time       :2016/9/14 上午 9:42
Describe        :
*/
int inNCCC_TICKET_AdviceSendRecvPacket(TRANSACTION_OBJECT *pobTran, int inAdvCnt)
{
	int				inCnt, inRetVal = VS_SUCCESS, inISOFuncIndex = -1;
	char				szDebugMsg[100 + 1];
	unsigned char			uszTCUpload = 0;
        TRANSACTION_OBJECT		ADVpobTran;
        ISO_TYPE_NCCC_TICKET_TABLE 	srISOFunc;

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_AdviceSendRecvPacket() START!");
        
        /* 加密模式，預設值 = 0。【0 = 不加密，1 = tSAM 加密，2 = 軟體加密】 */
        inISOFuncIndex = 0; /* 不加密 */
        memset((char *) &srISOFunc, 0x00, sizeof(srISOFunc));
        memcpy((char *) &srISOFunc, (char *) &srNCCC_TICKET_ISOFunc[inISOFuncIndex], sizeof(srISOFunc));

        memset((char *) &ADVpobTran, 0x00, sizeof(TRANSACTION_OBJECT));
        memcpy((char *) &ADVpobTran, (char *) pobTran, sizeof(TRANSACTION_OBJECT));

        /* 這裡的 for () 不可以在裡面直接call Return () ....  */
        for (inCnt = 0; inCnt < inAdvCnt; inCnt++)
        {
		/* 這裡要開始逐一將【0220】交易上傳 */
		if (inBATCH_GetAdvice_ESVC_DetailRecord_By_Sqlite(&ADVpobTran) == VS_SUCCESS)
		{
			inRetVal = VS_SUCCESS;
		}
		else
		{
			inRetVal = VS_ERROR;
			break;
		}
		
		ADVpobTran.inISOTxnCode = _ADVICE_;
		inNCCC_TICKET_GetSTAN(&ADVpobTran);
		ADVpobTran.inTransactionCode = ADVpobTran.srTRec.inCode;

		if (inRetVal == VS_SUCCESS)
			inRetVal = inNCCC_TICKET_SendPackRecvUnPack(&ADVpobTran);

		inNCCC_TICKET_SetSTAN(&ADVpobTran);
			
		/* 不回寫到batch */
		if (inRetVal == VS_ERROR || inRetVal == VS_ISO_PACK_ERR || inRetVal == VS_ISO_UNPACK_ERROR || inRetVal == VS_COMM_ERROR)
		{
			break;
		}
		else
		{
			if (ginISODebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Send Advice Inv:%ld", ADVpobTran.srTRec.lnCountInvNum);
				inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
			}
			
			if (srISOFunc.inAdviceAnalyse != NULL)
			{
				inRetVal = srISOFunc.inAdviceAnalyse(&ADVpobTran, &uszTCUpload);

				if (inRetVal == VS_SUCCESS)
				{
					/* 如果【ADVICE】刪除失敗會鎖機 */
					if (inNCCC_Ticket_ESVC_Delete_TRec_Top_Flow(&ADVpobTran, _TN_BATCH_TABLE_TICKET_ADVICE_) != VS_SUCCESS)
					{
						inFunc_EDCLock(AT);
						break;
					}
				}
			}
			else
			{
				inRetVal = VS_ERROR;
			}
			
		}

                if (inRetVal != VS_SUCCESS)
                        break;
        }

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_AdviceSendRecvPacket() END!");
        
        return (inRetVal);
}

/*
Function        :inNCCC_TICKET_ProcessAdvice
Date&Time       :2016/9/13 下午 5:13
Describe        :
*/
int inNCCC_TICKET_ProcessAdvice(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;
        int 	inSendAdviceCnt = 0;

	/* 改以卡別紀錄 START */
	pobTran->uszESVCFileNameByCardBit = VS_TRUE;
	inRetVal = inSqlite_Get_Table_Count_Flow(pobTran, _TN_BATCH_TABLE_TICKET_ADVICE_, &inSendAdviceCnt);
	/* 改以卡別紀錄 END */
	pobTran->uszESVCFileNameByCardBit = VS_FALSE;
        if (inRetVal == VS_NO_RECORD)
        {
                return (VS_SUCCESS);
        }
        else
        {
		/* 改以卡別紀錄 START */
		pobTran->uszESVCFileNameByCardBit = VS_TRUE;
		inRetVal = inNCCC_TICKET_AdviceSendRecvPacket(pobTran, inSendAdviceCnt);
		/* 改以卡別紀錄 END */
		pobTran->uszESVCFileNameByCardBit = VS_FALSE;
                if (inRetVal != VS_SUCCESS)
                {
                        return (inRetVal);
                }
        }

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_TICKET_ProcessOnline
Date&Time       :2016/9/14 上午 9:36
Describe        :
*/
int inNCCC_TICKET_ProcessOnline(TRANSACTION_OBJECT *pobTran)
{
        int 	inRetVal;
        char    szTemplate[512 + 1];
	char	szCustomIndicator[3 + 1] = {0};

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_ProcessOnline() START!");
	
	memset(szCustomIndicator, 0x00, sizeof(szCustomIndicator));
	inGetCustomIndicator(szCustomIndicator);
		
        /* 開始組交易封包，送、收、組、解 */
	pobTran->inISOTxnCode = pobTran->inTransactionCode;
	
        inRetVal = inNCCC_TICKET_SendPackRecvUnPack(pobTran);

        if (inRetVal == VS_COMM_ERROR)
        {
		/* 這裡不用再產生reversal */
		/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/5/20 下午 2:26
		 * 客製化075、103要送當筆reversal */
		if (!memcmp(szCustomIndicator, _CUSTOMER_INDICATOR_075_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
		    !memcmp(szCustomIndicator, _CUSTOMER_INDICATOR_103_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inCOMM_End(pobTran);
			inFLOW_RunFunction(pobTran, _COMM_START_);
			pobTran->uszReversalBit = VS_FALSE;
			inNCCC_TICKET_ProcessReversal(pobTran);
		}
		
                /* 這裡是通訊失敗，所以不管是不是晶片卡先定義為通訊失敗 */
                pobTran->inTransactionResult = _TRAN_RESULT_COMM_ERROR_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_COMM_;
                
                if (ginDebug == VS_TRUE)
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "pobTran->inTransactionResult = %d", _TRAN_RESULT_COMM_ERROR_);
                        inLogPrintf(AT, szTemplate);
                }
                
                return (inRetVal);
        }
        else if (inRetVal == VS_ISO_PACK_ERR || inRetVal == VS_ISO_UNPACK_ERROR)
        {
		if (inRetVal == VS_ISO_PACK_ERR)
		{
			pobTran->inTransactionResult = _TRAN_RESULT_PACK_ERR_;
			pobTran->inErrorMsg = _ERROR_CODE_V3_ISO_PACK_;
		}
		else if (inRetVal == VS_ISO_UNPACK_ERROR)
		{
			pobTran->inTransactionResult = _TRAN_RESULT_UNPACK_ERR_;
		}
		
                if (ginDebug == VS_TRUE)
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "pobTran->inTransactionResult = %d", inRetVal);
                        inLogPrintf(AT, szTemplate);
                }
                
                return (inRetVal);
        }
        else
        {
		if (pobTran->srTRec.inCode == _TICKET_IPASS_LOGON_)
	        {
	                /* SAM未註冊時會05拒絕，但會帶IP01 */
	                if (pobTran->srTRec.srIPASSRec.lnSign_Len > 0)
	                        return (VS_SUCCESS);                
	        } 
		
                /* 決定後續交易的流程先看【Field_39】再看【Field_38】 */
                pobTran->inTransactionResult = inNCCC_TICKET_CheckRespCode(pobTran); /* 【Field_39】 */
                
                if (ginDebug == VS_TRUE)
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "pobTran->inTransactionResult = %d", pobTran->inTransactionResult);
                        inLogPrintf(AT, szTemplate);
                }
                
                if (pobTran->inTransactionResult == _TRAN_RESULT_UNPACK_ERR_)
		{
                        return (VS_ISO_UNPACK_ERROR);				/* 主機沒有回回覆碼 */
		}
		else if (pobTran->inTransactionResult == _TRAN_RESULT_CANCELLED_)
                {
                        if (pobTran->inISOTxnCode == _TICKET_IPASS_REGISTER_	|| 
			    pobTran->inISOTxnCode == _TICKET_IPASS_LOGON_	||
			    pobTran->inISOTxnCode == _TICKET_ICASH_UNLOCK_	||
			    pobTran->inISOTxnCode == _TICKET_ICASH_GETPIN_	||
			    pobTran->inISOTxnCode == _TICKET_ICASH_LOGON_)
        		{
        		        /* Sign On不動Reversal flag */
        		}
			else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_	&& 
				 pobTran->srTRec.uszBlackListBit == VS_TRUE		&&
        		         pobTran->srTRec.srICASHRec.inStepNum == 1)
        		{
        		        /* ICASH 黑名單繼續往下 */
        		        pobTran->inErrorMsg = _ERROR_CODE_V3_NONE_;
				
        		        return (VS_SUCCESS);
        		}
        		else
                        {
				inSetTicket_ReversalBit("N");
				if (inSaveTDTRec(pobTran->srTRec.inTDTIndex) != VS_SUCCESS)
				{
					return (VS_ERROR);
				}
                        }
                	
			/* 開機跟ICASH Comfirm不Show */
                        if (pobTran->inRunOperationID == _OPERATION_EDC_BOOTING_		||
			    pobTran->inRunOperationID == _OPERATION_EDC_SDK_INITIAL_BOOTING_	||
			    pobTran->srTRec.srICASHRec.inStepNum == 2)
			{
				
			}
			else
			{
                                inNCCC_TICKET_DispHostResponseCode(pobTran);
			}
                                
                        return (VS_ERROR);
                }
		
		if (pobTran->inISOTxnCode == _TICKET_IPASS_REGISTER_	|| 
		    pobTran->inISOTxnCode == _TICKET_IPASS_LOGON_	||
		    pobTran->inISOTxnCode == _TICKET_ICASH_UNLOCK_	||
		    pobTran->inISOTxnCode == _TICKET_ICASH_GETPIN_	||
		    pobTran->inISOTxnCode == _TICKET_ICASH_LOGON_	||
		    pobTran->inISOTxnCode == _SETTLE_)
		{
		        /* Sign On不動Reversal flag */
		}
		else        
                {
        		inSetTicket_ReversalBit("N");
			if (inSaveTDTRec(pobTran->srTRec.inTDTIndex) != VS_SUCCESS)
			{
				return (VS_ERROR);
			}
                }
        }

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_ProcessOnline() END!");

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_TICKET_ProcessOffline
Date&Time       :2016/9/14 上午 10:02
Describe        :
*/
int inNCCC_TICKET_ProcessOffline(TRANSACTION_OBJECT *pobTran)
{
        int 	inRetVal = VS_SUCCESS;

        return (inRetVal);
}

/*
Function        :inNCCC_TICKET_AnalysePacket
Date&Time       :2016/9/14 上午 10:03
Describe        :
*/
int inNCCC_TICKET_AnalysePacket(TRANSACTION_OBJECT *pobTran)
{
        int				inRetVal = VS_SUCCESS, inISOFuncIndex = -1;
        ISO_TYPE_NCCC_TICKET_TABLE 	srISOFunc;

        if (pobTran->srTRec.uszOfflineBit == VS_FALSE)
        {
		/* 加密模式，預設值 = 0。【0 = 不加密，1 = tSAM 加密，2 = 軟體加密】 */
		inISOFuncIndex = 0; /* 不加密 */
		memset((char *) &srISOFunc, 0x00, sizeof(srISOFunc));
		memcpy((char *) &srISOFunc, (char *) &srNCCC_TICKET_ISOFunc[inISOFuncIndex], sizeof(srISOFunc));

		if (srISOFunc.inOnAnalyse != NULL)
			inRetVal = srISOFunc.inOnAnalyse(pobTran);
		else
			inRetVal = VS_ERROR;
        }

        return (inRetVal);
}

/*
Function        :inNCCC_TICKET_ReversalSendRecvPacket
Date&Time       :2016/9/13 下午 4:52
Describe        :
*/
int inNCCC_TICKET_ReversalSendRecvPacket(TRANSACTION_OBJECT *pobTran)
{
        int 		inRetVal = VS_ERROR, inSendCnt = 0;
        long 		lnREVCnt = 0;
        unsigned char 	uszFileName[20 + 1] = {0};
        unsigned char 	uszSendPacket[_NCCC_TICKET_ISO_SEND_ + 1] = {0}, uszRecvPacket[_NCCC_TICKET_ISO_RECV_ + 1] = {0};
        unsigned long 	ulREVFHandle = 0;
	
        memset(uszSendPacket, 0x00, sizeof(uszSendPacket));
        memset(uszRecvPacket, 0x00, sizeof(uszRecvPacket));

	/* 如果有DialBackup */
	
	/* 票證沒有撥接備援，暫時移除 */
	memset(uszFileName, 0x00, sizeof(uszFileName));
	pobTran->uszESVCFileNameByCardBit = VS_TRUE;
	inRetVal = inFunc_ComposeFileName(pobTran, (char*)uszFileName, _REVERSAL_FILE_EXTENSION_, 6);
	pobTran->uszESVCFileNameByCardBit = VS_FALSE;
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

        /* 開檔案 */
        if ((inRetVal = inFILE_OpenReadOnly(&ulREVFHandle, uszFileName)) == VS_ERROR)
        {
                return (VS_ERROR);
        }

        /* 取得 Reversal 檔案大小 */
        lnREVCnt = lnFILE_GetSize(&ulREVFHandle, uszFileName);
        if (lnREVCnt == 0)
        {
                inFILE_Close(&ulREVFHandle);

                return (VS_ERROR);
        }

        /* 指到第一個 BYTE */
        if ((inRetVal = inFILE_Seek(ulREVFHandle, 0, _SEEK_BEGIN_)) == VS_ERROR)
        {
                inFILE_Close(&ulREVFHandle);

                return (VS_ERROR);
        }

        /* 讀檔案 */
        if ((inRetVal = inFILE_Read(&ulREVFHandle, &uszSendPacket[0], lnREVCnt)) == VS_ERROR)
        {
                inFILE_Close(&ulREVFHandle);

                return (VS_ERROR);
        }

        /* 關檔案 */
        inFILE_Close(&ulREVFHandle);

        /* 傳送及接收 ISO 電文 */
        inSendCnt = lnREVCnt % 65536; /* Add by li for ln2int */

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_CommSendRecvToHost() Before");

        if ((inRetVal = inNCCC_TICKET_CommSendRecvToHost(pobTran, uszSendPacket, inSendCnt, uszRecvPacket)) != VS_SUCCESS)
        {
		pobTran->inTransactionResult = _TRAN_RESULT_COMM_ERROR_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_COMM_;
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inNCCC_TICKET_CommSendRecvToHost() Error");

                return (VS_COMM_ERROR);
        }

        /* 解 ISO 電文 */
	pobTran->inISOTxnCode = _REVERSAL_;
        if ((inRetVal = inNCCC_TICKET_UnPackISO(pobTran, uszSendPacket, uszRecvPacket)) != VS_SUCCESS)
        {
		pobTran->inTransactionResult = _TRAN_RESULT_UNPACK_ERR_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_ISO_UNPACK_;
		
                return (VS_ISO_UNPACK_ERROR);
        }

        /* memcmp兩字串相同回傳0 */
        if (memcmp(pobTran->srTRec.szRespCode, "00", 2) != 0)
        {
		pobTran->inTransactionResult = _TRAN_RESULT_CANCELLED_;
		
                return (VS_ERROR);
        }
        else
        {
                inSetTicket_ReversalBit("N");
		if (inSaveTDTRec(pobTran->srTRec.inTDTIndex) != VS_SUCCESS)
                {
                        return (VS_ERROR);
                }
                
		/* 票證沒有撥接備援，暫時移除 by Russell 2019/11/18 下午 3:49 */
		memset(uszFileName, 0x00, sizeof(uszFileName));
		pobTran->uszESVCFileNameByCardBit = VS_TRUE;
		inRetVal = inFunc_ComposeFileName(pobTran, (char*)uszFileName, _REVERSAL_FILE_EXTENSION_, 6);
		pobTran->uszESVCFileNameByCardBit = VS_FALSE;
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		inFILE_Delete(uszFileName);
        }

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_TICKET_ReversalSave_Flow
Date&Time       :2017/3/30 上午 11:06
Describe        :
*/
int inNCCC_TICKET_ReversalSave_Flow(TRANSACTION_OBJECT *pobTran)
{
	int 	inRetVal = VS_ERROR;
	
	inRetVal = inNCCC_TICKET_ReversalSave(pobTran);
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	/* 票證沒有撥接備援，暫時移除 */
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_TICKET_ReversalSave
Date&Time       :2016/9/13 下午 5:06
Describe        :
*/
int inNCCC_TICKET_ReversalSave(TRANSACTION_OBJECT *pobTran)
{
        int 		inRetVal, inPacketCnt;
        unsigned char 	uszReversalPacket[_NCCC_TICKET_ISO_SEND_ + 1];
        unsigned char 	uszFileName[20 + 1];
        unsigned long 	srFHandle;


        memset(uszReversalPacket, 0x00, sizeof(uszReversalPacket));
        pobTran->inISOTxnCode = _REVERSAL_; /* 沖銷 */

        /* 組_REVERSAL_ 封包 */
        inPacketCnt = inNCCC_TICKET_PackISO(pobTran, &uszReversalPacket[0], pobTran->inISOTxnCode);
        if (inPacketCnt <= 0)
	{
		pobTran->inTransactionResult = _TRAN_RESULT_PACK_ERR_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_ISO_PACK_;
		
                return (VS_ERROR);
	}

        memset(uszFileName, 0x00, sizeof(uszFileName));
	pobTran->uszESVCFileNameByCardBit = VS_TRUE;
	inRetVal = inFunc_ComposeFileName(pobTran, (char*)uszFileName, _REVERSAL_FILE_EXTENSION_, 6);
	pobTran->uszESVCFileNameByCardBit = VS_FALSE;
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
        /* 先刪除上一筆 Reversal，確保是當筆的 Reversal */
        inFILE_Delete(uszFileName);

        inRetVal = inFILE_Create(&srFHandle, uszFileName);
        if (inRetVal == VS_ERROR)
        {
                inFILE_Close(&srFHandle);
                inFILE_Delete(uszFileName);

                return (VS_ERROR);
        }

        inRetVal = inFILE_Seek(srFHandle, 0, _SEEK_BEGIN_); /* 從頭開始 */
        if (inRetVal == VS_ERROR)
        {
                inFILE_Close(&srFHandle);
                inFILE_Delete(uszFileName);

                return (VS_ERROR);
        }

        inRetVal = inFILE_Write(&srFHandle, &uszReversalPacket[0], inPacketCnt);
        if (inRetVal == VS_ERROR)
        {
                inFILE_Close(&srFHandle);
                inFILE_Delete(uszFileName);

                return (VS_ERROR);
        }

        inFILE_Close(&srFHandle);
	
	/* 標記為哪個卡別要送Reversal */
	inLoadTDTRec(pobTran->srTRec.inTDTIndex);
	inSetTicket_ReversalBit("Y");
	inSaveTDTRec(pobTran->srTRec.inTDTIndex);

        return (VS_SUCCESS);
}

int inNCCC_TICKET_CommSendRecvToHost(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, int inSendLen, unsigned char *uszRecvPacket)
{
	/* 【需求單-108265】修改電票交易timeout時間 by Russell 2020/6/4 下午 2:02 */
        int		inRetVal;
        int		inSendTimeout = _NCCC_TICKET_COMM_TIMEOUT_;
        int		inReceiveTimeout = _NCCC_TICKET_COMM_TIMEOUT_;
	int		inReceiveSize = _COMM_RECEIVE_MAX_LENGTH_;		/* 配合Ingenico */
	char		szTimeOut[2 + 1] = {0};
        char		szHostResponseTimeOut[2 + 1] = {0};
	unsigned char	uszDispBit = VS_TRUE;

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_CommSendRecvToHost() START!");
        
	/* 如果沒設定TimeOut，就用EDC.dat內的TimeOut */
	if (inSendTimeout <= 0)
	{
		memset(szTimeOut, 0x00, sizeof(szTimeOut));
		inGetIPSendTimeout(szTimeOut);
		inSendTimeout = atoi(szTimeOut);
	}
	
	/* 顯示ISO Debug */
	if (ginDebug == VS_TRUE)
	{
		vdNCCC_TICKET_ISO_FormatDebug_DISP(pobTran, &uszSendPacket[0], inSendLen);
	}
	/* 列印ISO Debug */
        if (ginISODebug == VS_TRUE)
	{
                vdNCCC_TICKET_ISO_FormatDebug_PRINT(pobTran, &uszSendPacket[0], inSendLen);
	}
	
	if (memcmp(gszTermVersionID, "MD731UAGAS001", strlen("MD731UAGAS001")) == 0)
	{
		/* 不顯示訊息 */
		uszDispBit = VS_FALSE;
	}
	else
	{
		/* 不顯示訊息 */
		uszDispBit = VS_TRUE;
	}
	
        if ((inRetVal = inCOMM_Send(uszSendPacket, inSendLen, inSendTimeout, uszDispBit)) != VS_SUCCESS)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inCOMM_Send() ERROR");
                return (VS_ERROR);
        }
	
	/* 如果沒設定TimeOut，就用EDC.dat內的TimeOut */
	if (inReceiveTimeout <= 0)
	{
		memset(szHostResponseTimeOut, 0x00, sizeof(szHostResponseTimeOut));
		inRetVal = inGetHostResponseTimeOut(szHostResponseTimeOut);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		inReceiveTimeout = atoi(szHostResponseTimeOut);
	}
	
	if (memcmp(gszTermVersionID, "MD731UAGAS001", strlen("MD731UAGAS001")) == 0)
	{
		/* 不顯示訊息 */
		uszDispBit = VS_FALSE;
	}
	else
	{
		/* 不顯示訊息 */
		uszDispBit = VS_TRUE;
	}
	
        if ((inRetVal = inCOMM_Receive(uszRecvPacket, inReceiveSize, inReceiveTimeout, uszDispBit)) != VS_SUCCESS)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inCOMM_Receive() ERROR");

                return (VS_ERROR);
        }

	/* 顯示ISO Debug */
	if (ginDebug == VS_TRUE)
	{
		vdNCCC_TICKET_ISO_FormatDebug_DISP(pobTran, &uszRecvPacket[0], inRetVal);
	}
        /* 列印ISO Debug */
        if (ginISODebug == VS_TRUE)
                vdNCCC_TICKET_ISO_FormatDebug_PRINT(pobTran, &uszRecvPacket[0], inRetVal);

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_CommSendRecvToHost() END!");
        
        return (VS_SUCCESS);
}

int inNCCC_TICKET_SendPackRecvUnPack(TRANSACTION_OBJECT *pobTran)
{
        int 		inSendCnt = 0, inRecvCnt = 0, inRetVal;
        unsigned char 	uszSendPacket[_NCCC_TICKET_ISO_SEND_ + 1], uszRecvPacket[_NCCC_TICKET_ISO_RECV_ + 1];

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_SendPackRecvUnPack() START!");

        memset(uszSendPacket, 0x00, sizeof(uszSendPacket));
        memset(uszRecvPacket, 0x00, sizeof(uszRecvPacket));

        /* 組 ISO 電文 */
        if ((inSendCnt = inNCCC_TICKET_PackISO(pobTran, uszSendPacket, pobTran->inISOTxnCode)) <= 0)
        {
		pobTran->inTransactionResult = _TRAN_RESULT_PACK_ERR_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_ISO_PACK_;
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inNCCC_TICKET_PackISO() Error!");

                return (VS_ISO_PACK_ERR); /* 組交易電文錯誤，不用組_REVERSAL_ */
        }
        
        /* 傳送及接收 ISO 電文 */
        if ((inRecvCnt = inNCCC_TICKET_CommSendRecvToHost(pobTran, uszSendPacket, inSendCnt, uszRecvPacket)) != VS_SUCCESS)
        {
		pobTran->inTransactionResult = _TRAN_RESULT_COMM_ERROR_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_COMM_;
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inNCCC_TICKET_CommSendRecvToHost() Error");

                if (pobTran->inISOTxnCode != _ADVICE_)
                        memset(pobTran->srTRec.szRespCode, 0x00, sizeof(pobTran->srTRec.szRespCode));

                return (VS_COMM_ERROR);
        }

        /* 解 ISO 電文 */
        inRetVal = inNCCC_TICKET_UnPackISO(pobTran, uszSendPacket, uszRecvPacket);
        
        if (inRetVal != VS_SUCCESS)
        {	
		pobTran->inTransactionResult = _TRAN_RESULT_UNPACK_ERR_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_ISO_UNPACK_;
		
                inRetVal = VS_ISO_UNPACK_ERROR;
        }
	
	/* 模擬ESVC advice回19的情形 */
	if ((ginSpecialSituation & _SPECIAL_SITUATION_ESVC_ADVICE_RESPONSE_19_) == _SPECIAL_SITUATION_ESVC_ADVICE_RESPONSE_19_)
	{
		if (memcmp(&uszRecvPacket[5], "\x02\x30", 2) == 0)
		{
			sprintf(pobTran->srTRec.szRespCode, "19");
		}
	}
	
        return (inRetVal);
}

/*
Function        :inNCCC_TICKET_CheckRespCode
Date&Time       :2016/9/14 上午 9:48
Describe        :
*/
int inNCCC_TICKET_CheckRespCode(TRANSACTION_OBJECT *pobTran)
{
        int 	inRetVal = _TRAN_RESULT_COMM_ERROR_;
        char	szCustomerIndicator[3 + 1] = {0};
        
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);

	if (!memcmp(pobTran->srTRec.szRespCode, "00", 2))
	{
		inRetVal = _TRAN_RESULT_AUTHORIZED_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_NONE_;
		
		/* 未滿三次，授權成功要重置次數 */
		if (ginMacError < 3)
		{
			ginMacError = 0;
		}
	}
	else if (!memcmp(pobTran->srTRec.szRespCode, "01", 2) || !memcmp(pobTran->srTRec.szRespCode, "02", 2))
	{
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_)	||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)		||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
		{
			/* 【需求單 - 104014】發卡機構on-us繳費平台需求 +【需求單 - 104152】發卡機構on-us繳費平台 Payitem Code */
			/* 	on-us交易不支援Call bank交易，提示「XX 拒絕交易」。 */
			/* 維持原回應碼 */
		}
		else
		{
			if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
			{
				strcpy(pobTran->srBRec.szRespCode, "05");
			}
		}
		
		/* 感應交易若被主機回CALL BANK，EDC請提示「05拒絕交易」。*/
		if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
		{
			inRetVal = _TRAN_RESULT_CANCELLED_;
		}
		/* (需求單-110202)-修改客製化參數111卡人自助EDC UI訊息需求 by Russell 2021/12/23 下午 1:57 */
		/* 這邊直接照520，01、02直接拒絕 */
		else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_)	||
			 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)			||
			 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inRetVal = _TRAN_RESULT_CANCELLED_;
		}
		else
			inRetVal = _TRAN_RESULT_REFERRAL_;
		
		/* 即使沒有跑call bank流程，仍然要回傳為callbank */
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)		 ||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
		{
			pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_CALLBANK_;
		}
	}
	else if (!memcmp(pobTran->srTRec.szRespCode, "  ", 2))
	{
		inRetVal = _TRAN_RESULT_UNPACK_ERR_; /* 不是定義的 Response Code */
		pobTran->inErrorMsg = _ERROR_CODE_V3_ISO_UNPACK_;
	}
	else if (pobTran->srTRec.szRespCode[0] == 0x00)
	{
		inRetVal = _TRAN_RESULT_COMM_ERROR_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_COMM_;
	}
	else if (!memcmp(&pobTran->srTRec.szRespCode[0], "0X", 2) ||
		 !memcmp(&pobTran->srTRec.szRespCode[0], "C8", 2) ||
		 !memcmp(&pobTran->srTRec.szRespCode[0], "C9", 2) ||
	         !memcmp(&pobTran->srTRec.szRespCode[0], "XA", 2) ||
	         !memcmp(&pobTran->srTRec.szRespCode[0], "XB", 2) ||
	         !memcmp(&pobTran->srTRec.szRespCode[0], "XC", 2) ||
		 !memcmp(&pobTran->srTRec.szRespCode[0], "XD", 2) ||
	         !memcmp(&pobTran->srTRec.szRespCode[0], "XI", 2) ||
	         !memcmp(&pobTran->srTRec.szRespCode[0], "XN", 2) ||
	         !memcmp(&pobTran->srTRec.szRespCode[0], "XV", 2) ||
	         !memcmp(&pobTran->srTRec.szRespCode[0], "XX", 2) ||
	         !memcmp(&pobTran->srTRec.szRespCode[0], "XZ", 2) ||
	         !memcmp(&pobTran->srTRec.szRespCode[0], "L1", 2) ||
	         !memcmp(&pobTran->srTRec.szRespCode[0], "L3", 2) ||
	         !memcmp(&pobTran->srTRec.szRespCode[0], "L4", 2) ||
	         !memcmp(&pobTran->srTRec.szRespCode[0], "L5", 2) ||
	         !memcmp(&pobTran->srTRec.szRespCode[0], "L6", 2) ||
	         !memcmp(&pobTran->srTRec.szRespCode[0], "L7", 2) ||
	         !memcmp(&pobTran->srTRec.szRespCode[0], "L8", 2) ||
	         !memcmp(&pobTran->srTRec.szRespCode[0], "L9", 2) ||
	         !memcmp(&pobTran->srTRec.szRespCode[0], "ID", 2))
	{
		inRetVal = _TRAN_RESULT_CANCELLED_;
	}
	else if (!memcmp(&pobTran->srTRec.szRespCode[0], "XT", 2))
	{
	        memset(pobTran->szTicket_ErrorCode, 0x00, sizeof(pobTran->szTicket_ErrorCode));
                
                /* 客製化098，0010以上的回應碼都與標準版少1 */
                if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)     ||
                    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_))  
		{
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0013");
                }
                else
                {
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0014");
                }    

	        inRetVal = _TRAN_RESULT_CANCELLED_;
	}
	else if (!memcmp(&pobTran->srTRec.szRespCode[0], "A0", 2) ||
		 !memcmp(&pobTran->srTRec.szRespCode[0], "A1", 2))
	{
		ginMacError++;
		inRetVal = _TRAN_RESULT_CANCELLED_;
	}
        else
        {
                if ((pobTran->srTRec.szRespCode[0] >= '0' && pobTran->srTRec.szRespCode[0] <= '9') && (pobTran->srTRec.szRespCode[1] >= '0' && pobTran->srTRec.szRespCode[1] <= '9'))
                {
                        /* 要增加結帳的判斷 */
                        if ((pobTran->inISOTxnCode == _SETTLE_		||
			     pobTran->inISOTxnCode == _CLS_SETTLE_)	&& 
			    !memcmp(&pobTran->srTRec.szRespCode[0], "95", 2))
                                inRetVal = _TRAN_RESULT_SETTLE_UPLOAD_BATCH_; /* 要當成功進行不平帳上傳 */
                        else
                                inRetVal = _TRAN_RESULT_CANCELLED_;
                }
                else
		{
                        inRetVal = _TRAN_RESULT_UNPACK_ERR_; /* 不是定義的 Response Code */
			pobTran->inErrorMsg = _ERROR_CODE_V3_ISO_UNPACK_;
		}
        }

        return (inRetVal);
}

/*
Function        :inNCCC_TICKET_CheckAuthCode
Date&Time       :2016/9/14 上午 9:48
Describe        :
*/
int inNCCC_TICKET_CheckAuthCode(TRANSACTION_OBJECT *pobTran)
{
        int 	inRetVal = VS_SUCCESS;
	char	szDebugMsg[100 + 1];
	
        switch (pobTran->inISOTxnCode)
        {
                case _SETTLE_:
		case _CLS_SETTLE_:
                case _BATCH_UPLOAD_:
                case _CLS_BATCH_:
		case _CUP_SALE_ :
		case _CUP_REFUND_ :
		case _CUP_MAIL_ORDER_REFUND_ :
		case _CUP_PRE_AUTH_ :
		case _CUP_PRE_COMP_ :
		case _CUP_VOID_ :
		case _CUP_PRE_AUTH_VOID_ :
		case _CUP_PRE_COMP_VOID_ :
		case _LOYALTY_REDEEM_ :
		case _VOID_LOYALTY_REDEEM_ :
                        break; /* 不檢核 */
                default:
                        /* 修改主機授權碼帶空白TC Upload失敗，卡Advice的問題 */
                        if ((!memcmp(&pobTran->srTRec.szAuthCode[0], "000000", 6) || !memcmp(&pobTran->srTRec.szAuthCode[0], "      ", 6)) && pobTran->srBRec.uszCUPTransBit != VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "inNCCC_TICKET_CheckAuthCode Error, %s", pobTran->srTRec.szAuthCode);
					inLogPrintf(AT, szDebugMsg);
				}
				
                                inRetVal = VS_ERROR;
			}

                        break;
        }

        return (inRetVal);
}

/*
Function        :inNCCC_TICKET_ProcessSettleBatchUpload
Date&Time       :2016/9/14 上午 10:08
Describe        :
*/
int inNCCC_TICKET_ProcessSettleBatchUpload(TRANSACTION_OBJECT *pobTran)
{
	int			inRetVal;
        int			inBatchTotalCnt, i;
	int			inBatchValidCnt = 0, inBatchValidTotalCnt = 0;
        int			inRunCLS_BATCH = VS_TRUE;
        TRANSACTION_OBJECT	OrgpobTran;

        /* 先算出來有多少筆交易資料，如果有會回傳交易總筆數 */
        inBatchTotalCnt = inBATCH_GetTotalCountFromBakFile_By_Sqlite(pobTran);
        if (inBatchTotalCnt == VS_ERROR)
                return (VS_ERROR);
        else if (inBatchTotalCnt == 0)
        {
                /* 主機回《95》但是沒有帳，還是要做一次結帳 */
                inRunCLS_BATCH = VS_TRUE;
        }
        else if (inBatchTotalCnt > 0)
        {
		inBatchValidTotalCnt = inBATCH_GetTotalCount_BatchUpload_By_Sqlite(pobTran);
		
                /* 有記錄要開始執行 BATCH UPLOAD */
                pobTran->uszLastBatchUploadBit = VS_FALSE;

                memset((char *) &OrgpobTran, 0x00, sizeof(TRANSACTION_OBJECT));
                memcpy((char *) &OrgpobTran, (char *) pobTran, sizeof(TRANSACTION_OBJECT));

		/* 開始讀取 */
		inBATCH_GetDetailRecords_By_Sqlite_Enormous_START(pobTran);
		/* 預設為無須重找 */
		guszEnormousNoNeedResetBit = VS_TRUE;
		
                for (i = 0; i < inBatchTotalCnt; i++)
                {
                        /* 這裡要顯示【批次上傳】訊息 */

			pobTran->inISOTxnCode = _BATCH_UPLOAD_;
                        /* 開始讀批次檔記錄*/
                        if (inBATCH_GetDetailRecords_By_Sqlite_Enormous_Read(pobTran, i) != VS_SUCCESS)
                        {
                                inRunCLS_BATCH = VS_FALSE;
                                break;
                        }
			
			/* NCCC 取消(void)不會上傳，batch PreAuth也不會，優惠兌換也不會 */
                        /* 因為取消incode有很多種，所以改以 已取消的flag來判斷 */
                        if (pobTran->srBRec.uszVOIDBit == VS_TRUE		||
			    pobTran->srBRec.inCode == _PRE_AUTH_		||
			    pobTran->srBRec.inCode == _CUP_PRE_AUTH_		||
			    pobTran->srBRec.inCode == _LOYALTY_REDEEM_		||
			    pobTran->srBRec.inCode == _VOID_LOYALTY_REDEEM_)
                                continue;
			
			/* 表示要上傳的合法筆數 */
			inBatchValidCnt ++;

                        if (inBatchValidCnt == inBatchValidTotalCnt)
                                pobTran->uszLastBatchUploadBit = VS_TRUE; /* 最後一筆交易 */
			
			/* 抓最新的STAN */
			inNCCC_TICKET_GetSTAN(pobTran);

			inRetVal = inNCCC_TICKET_SendPackRecvUnPack(pobTran);
			/* 成功或失敗 System Trace Number 都要加一 */
			inNCCC_TICKET_SetSTAN(pobTran);
			
			if (inRetVal != VS_SUCCESS)
                        {
                                inRunCLS_BATCH = VS_FALSE;
                                break;
                        }
                        else
                        {
				/* 回傳非00，表示失敗 */
                                if (memcmp(pobTran->srTRec.szRespCode, "00", 2))
                                {
					inNCCC_TICKET_DispHostResponseCode(pobTran);		/* 顯示主機回的錯誤訊息 */
                                        inRunCLS_BATCH = VS_FALSE;
                                        break;
                                }
				
                        }

                }
		/* 結束讀取 */
		inBATCH_GetDetailRecords_By_Sqlite_Enormous_END(pobTran);
		
		/* 還原成原來的【 pobTran 】 */
		memcpy((char *) pobTran, (char *) &OrgpobTran, sizeof(TRANSACTION_OBJECT));
        }

        /* 抓最新的STAN */
	inNCCC_TICKET_GetSTAN(pobTran);

        if (inRunCLS_BATCH == VS_TRUE)
        {
                /* 暫時保留
                 pobTran->srBRec.uszManualBit = VS_FALSE;
                 pobTran->srBRec.uszVOIDBit = VS_FALSE;
                 pobTran->srBRec.uszSignatureBit = VS_FALSE;
                 pobTran->srTRec.uszOfflineBit = VS_FALSE;
                 */
		
		/* 930000 – 前次不平帳上傳失敗 */
		inSetCLS_SettleBit("N");
		inSaveHDPTRec(pobTran->srBRec.inHDTIndex);

                pobTran->inISOTxnCode = _CLS_BATCH_;

		inRetVal = inNCCC_TICKET_SendPackRecvUnPack(pobTran);
		/* 成功或失敗 System Trace Number 都要加一 */
		inNCCC_TICKET_SetSTAN(pobTran);
			
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
                else
                {
			/* 回傳非00，表示失敗 */
                        if (memcmp(pobTran->srTRec.szRespCode, "00", 2))
                                return (VS_ERROR); /* 一定要回【00】 */
                }
        }
        else
        {
		/* 930000 – 前次不平帳上傳失敗 */
		inSetCLS_SettleBit("Y");
		inSaveHDPTRec(pobTran->srBRec.inHDTIndex);
		
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_TICKET_DispHostResponseCode
Date&Time       :2016/11/15 下午 5:44
Describe        :顯示錯誤代碼
*/
int inNCCC_TICKET_DispHostResponseCode(TRANSACTION_OBJECT *pobTran)
{
	int		inChoice = _DisTouch_No_Event_;
	int		inRetVal = VS_SUCCESS;
	char		szResponseCode[10 + 1] = {0};
	char		szMsg[42 + 1] = {0};
	char		szCustomerIndicator[3 + 1] = {0};
	char		szFuncEnable[2 + 1] = {0};
        unsigned char   uszKey = 0x00;
	
	/* Host回覆錯誤時也是亮紅燈 */
	memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
	inGetIntegrate_Device(szFuncEnable);
	if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
	{

	}
	else
	{
		inNCCC_Ticket_Set_CTLS_Light(&pobTran->inCTLSLightStatus, _CTLS_LIGHT_RED_);
	}
	
	memset(szMsg, 0x00, sizeof(szMsg));
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	if ((pobTran->srTRec.szRespCode[0] >= '0' && pobTran->srTRec.szRespCode[0] <= '9') &&
	    (pobTran->srTRec.szRespCode[1] >= '0' && pobTran->srTRec.szRespCode[1] <= '9'))
	{
		if (!memcmp(&pobTran->srTRec.szRespCode[0], "13", 2))
			sprintf(szMsg, "%s", "請聯絡發卡銀行");			/* 請聯絡發卡銀行 */
		else if (!memcmp(&pobTran->srTRec.szRespCode[0], "25", 2))
			sprintf(szMsg, "%s", "無原授權記錄");			/* 無原授權記錄 */
		else if (!memcmp(&pobTran->srTRec.szRespCode[0], "30", 2))
			sprintf(szMsg, "%s", "請重新交易");			/* 請重新交易 */
		else if (!memcmp(&pobTran->srTRec.szRespCode[0], "38", 2))
			sprintf(szMsg, "%s", "密碼錯誤次數超出");		/* 密碼錯誤次數超出 */
		else if (!memcmp(&pobTran->srTRec.szRespCode[0], "51", 2))
			sprintf(szMsg, "%s", "本交易不接受");			/* 本交易不接受 */
		else if (!memcmp(&pobTran->srTRec.szRespCode[0], "55", 2))
			sprintf(szMsg, "%s", "密碼錯誤");			/* 密碼錯誤 */
		else if (!memcmp(&pobTran->srTRec.szRespCode[0], "40", 2))
			sprintf(szMsg, "%s", "拒絕交易");			/* 拒絕交易 */
		else if (!memcmp(&pobTran->srTRec.szRespCode[0], "98", 2))
			sprintf(szMsg, "%s", "請聯絡發卡銀行");			/* 請聯絡發卡銀行 */
		else if (!memcmp(&pobTran->srTRec.szRespCode[0], "05", 2))
		{
			/* 如果是愛金卡顯示卡片異常，驗測中要求修改 2019/12/31  */
			if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
			{
				sprintf(szMsg, "%s", "卡片異常");			/* 卡片異常 */
			}
			else
			{
				sprintf(szMsg, "%s", "拒絕交易");			/* 拒絕交易 */
			}
		}
		else
			sprintf(szMsg, "%s", "拒絕交易");			/* 拒絕交易 */
	}
	else
	{
		if (!memcmp(&pobTran->srTRec.szRespCode[0], "0X", 2))
			sprintf(szMsg, "%s", "拒絕交易");			/* 拒絕交易 */
		else if (!memcmp(&pobTran->srTRec.szRespCode[0], "A0", 2))
			sprintf(szMsg, "%s", "驗証錯誤請重試");			/* 驗証錯誤請重試 */
		else if (!memcmp(&pobTran->srTRec.szRespCode[0], "A1", 2))
			sprintf(szMsg, "%s", "驗証錯誤請重試");			/* 驗証錯誤請重試 */
		else if (!memcmp(&pobTran->srTRec.szRespCode[0], "C8", 2))
			sprintf(szMsg, "%s", "感應交易超額");			/* 感應交易超額 */
		else if (!memcmp(&pobTran->srTRec.szRespCode[0], "C9", 2))
			sprintf(szMsg, "%s", "感應交易超次");			/* 感應交易超次 */
		else if (!memcmp(&pobTran->srTRec.szRespCode[0], "XA", 2))
			sprintf(szMsg, "%s", "拒絕交易");			/* 拒絕交易 */
		else if (!memcmp(&pobTran->srTRec.szRespCode[0], "XB", 2))
			sprintf(szMsg, "%s", "拒絕交易");			/* 拒絕交易 */
		else if (!memcmp(&pobTran->srTRec.szRespCode[0], "XC", 2))
			sprintf(szMsg, "%s", "請勿按銀聯鍵");			/* 請勿按銀聯鍵 */
		else if (!memcmp(&pobTran->srTRec.szRespCode[0], "XN", 2))
			sprintf(szMsg, "%s", "請改按銀聯鍵");			/* 請改按銀聯鍵 */
		else if (!memcmp(&pobTran->srTRec.szRespCode[0], "XT", 2))
		{
			sprintf(szMsg, "%s", "無法比對原交易");			/* 無法比對原交易 */
                        
                        /* 客製化098，0010以上的回應碼都與標準版少1 */
                        if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)     ||
                            !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_))  
                        {
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0013");
                        }
                        else
                        {
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0014");
                        }
		}
		else if (!memcmp(&pobTran->srTRec.szRespCode[0], "XV", 2))
			sprintf(szMsg, "%s", "輸入銀聯背面三碼");		/* 輸入銀聯背面三碼 */
		else if (!memcmp(&pobTran->srTRec.szRespCode[0], "XX", 2))
			sprintf(szMsg, "%s", "請改刷磁條");			/* 請改刷磁條 */
		else if (!memcmp(&pobTran->srTRec.szRespCode[0], "XY", 2))
			sprintf(szMsg, "%s", "NPS拆帳單");			/* NPS拆帳單 */
		else if (!memcmp(&pobTran->srTRec.szRespCode[0], "XZ", 2))
			sprintf(szMsg, "%s", "拒絕交易");			/* 拒絕交易 */
		else if (!memcmp(&pobTran->srTRec.szRespCode[0], "XI", 2))
			sprintf(szMsg, "%s", "拒絕交易");			/* 拒絕交易 */
		else if (!memcmp(&pobTran->srTRec.szRespCode[0], "L1", 2))
			sprintf(szMsg, "%s", "無優惠可兌換");			/* 無優惠可兌換 */
		else if (!memcmp(&pobTran->srTRec.szRespCode[0], "L3", 2))
			sprintf(szMsg, "%s", "優惠已兌換");			/* 優惠已兌換 */
		else if (!memcmp(&pobTran->srTRec.szRespCode[0], "L4", 2))
			sprintf(szMsg, "%s", "兌換期限已過");			/* 兌換期限已過 */
		else if (!memcmp(&pobTran->srTRec.szRespCode[0], "L5", 2))
			sprintf(szMsg, "%s", "請以刷卡作兌換");			/* 請以刷卡作兌換 */
		else if (!memcmp(&pobTran->srTRec.szRespCode[0], "L6", 2))
			sprintf(szMsg, "%s", "請掃描條碼作兌換");		/* 請掃描條碼作兌換 */
		else if (!memcmp(&pobTran->srTRec.szRespCode[0], "L7", 2))
			sprintf(szMsg, "%s", "無兌換可取消");			/* 無兌換可取消 */
		else if (!memcmp(&pobTran->srTRec.szRespCode[0], "L8", 2))
			sprintf(szMsg, "%s", "重複取消");			/* 重複取消 */
		else if (!memcmp(&pobTran->srTRec.szRespCode[0], "L9", 2))
			sprintf(szMsg, "%s", "取消期限已過");			/* 取消期限已過 */
		else if (!memcmp(&pobTran->srTRec.szRespCode[0], "ID", 2))
			sprintf(szMsg, "%s", "ID身分字號錯誤");			/* ID身分字號錯誤 */
		else if (!memcmp(&pobTran->srTRec.szRespCode[0], "Y1", 2))
			sprintf(szMsg, "%s", "拒絕交易");			
		else if (!memcmp(&pobTran->srTRec.szRespCode[0], "Z1", 2))
			sprintf(szMsg, "%s", "拒絕交易");			
		else if (!memcmp(&pobTran->srTRec.szRespCode[0], "Y3", 2))
			sprintf(szMsg, "%s", "拒絕交易");			
		else if (!memcmp(&pobTran->srTRec.szRespCode[0], "Z3", 2))
			sprintf(szMsg, "%s", "拒絕交易");
		else if (!memcmp(&pobTran->srTRec.szRespCode[0], "OL", 2))
			sprintf(szMsg, "%s", "拒絕交易");
		else
		{
			sprintf(szMsg, "%s", "拒絕交易");			/* Display 【拒絕交易】 */
		}
	}
	
	/* MPAS的規則 */
	if (!memcmp(&pobTran->srTRec.szAuthCode[0], "REJB01", 6) ||
	    !memcmp(&pobTran->srTRec.szAuthCode[0], "REJB02", 6) ||
	    !memcmp(&pobTran->srTRec.szAuthCode[0], "REJB03", 6))
	{
		sprintf(szMsg, "%s", "非參加機構卡片");				/* 非參加機構卡片 */
	}
		
	memset(szResponseCode, 0x00, sizeof(szResponseCode));	
	sprintf(szResponseCode, "%s", pobTran->srTRec.szRespCode);		/* 錯誤代碼 */
	
	/* (需求單-110202)-修改客製化參數111卡人自助EDC UI訊息需求 by Russell 2021/12/23 下午 1:57 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_)	||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)		 ||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
	{
		if (memcmp(szMsg, "請聯絡發卡銀行", strlen("請聯絡發卡銀行")) == 0)
		{
			sprintf(szMsg, "%s", "拒絕交易");	/* 拒絕交易 */
		}
	}
	
	/* 如果當SDK，不用顯示 */
	if (memcmp(gszTermVersionID, "MD731UAGAS001", strlen("MD731UAGAS001")) == 0)
	{
		
	}
	/* 在客製化107.111的時候將請按確認鍵跟請按清除鍵這兩個字樣移除 */
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)		||
		 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_)	||
		 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)			||
		 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
	{
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)	||
		 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
		{
			srDispMsgObj.inTimeout = _CUSTOMER_111_KIOSK_STANDARD_DISPLAY_TIMEOUT_;
		}
		else
		{
			srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
		}
		
		strcpy(srDispMsgObj.szDispPic1Name, "");
		srDispMsgObj.inDispPic1YPosition = 0;
		srDispMsgObj.inMsgType = _NO_KEY_MSG_;
		strcpy(srDispMsgObj.szErrMsg1, szResponseCode);
		srDispMsgObj.inErrMsg1Line = _LINE_8_5_;
		strcpy(srDispMsgObj.szErrMsg2, szMsg);
		srDispMsgObj.inErrMsg2Line = _LINE_8_6_;
		srDispMsgObj.inBeepTimes = 1;
		srDispMsgObj.inBeepInterval = 0;
		inDISP_Msg_BMP(&srDispMsgObj);

		/* 為了強調Timeout時間 */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	}
        else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)     ||
                 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)) 
	{
		/* 清下排 */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);

		inDISP_ChineseFont(szResponseCode, _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
		inDISP_ChineseFont(szMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);

		inDISP_PutGraphic(_ERR_CLEAR_, 0, _COORDINATE_Y_LINE_8_7_);
		inDISP_BEEP(1, 0);

		inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_105_MCDONALDS_DISPLAY_TIMEOUT_);
		while (1)
		{
			inChoice = inDisTouch_TouchSensor_Click_Slide(_Touch_BATCH_END_);
			uszKey = uszKBD_Key();
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				uszKey = _KEY_TIMEOUT_;
			}

			if (uszKey == _KEY_CANCEL_	||
			    inChoice == _BATCH_END_Touch_ENTER_BUTTON_)
			{
				inRetVal = VS_SUCCESS;
				break;
			}
			else if (uszKey == _KEY_TIMEOUT_)
			{
				inRetVal = VS_SUCCESS;
				break;
			}
			else
			{
				continue;
			}
		}
		/* 為了強調Timeout時間 */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDisTouch_Flush_TouchFile();
	}
	else
	{
		/* 清下排 */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);

		inDISP_ChineseFont(szResponseCode, _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
		inDISP_ChineseFont(szMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);

		inDISP_PutGraphic(_ERR_CLEAR_, 0, _COORDINATE_Y_LINE_8_7_);
		inDISP_BEEP(1, 0);

		inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
		while (1)
		{
			inChoice = inDisTouch_TouchSensor_Click_Slide(_Touch_BATCH_END_);
			uszKey = uszKBD_Key();
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				uszKey = _KEY_TIMEOUT_;
			}

			if (uszKey == _KEY_CANCEL_	||
			    inChoice == _BATCH_END_Touch_ENTER_BUTTON_)
			{
				inRetVal = VS_SUCCESS;
				break;
			}
			else if (uszKey == _KEY_TIMEOUT_)
			{
				inRetVal = VS_SUCCESS;
				break;
			}
			else
			{
				continue;
			}
		}
		/* 為了強調Timeout時間 */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDisTouch_Flush_TouchFile();
	}
	
        return (inRetVal);
}

/*
Function        :inNCCC_TICKET_SyncHostTerminalDateTime
Date&Time       :2016/9/14 上午 10:14
Describe        :
*/
int inNCCC_TICKET_SyncHostTerminalDateTime(TRANSACTION_OBJECT *pobTran)
{	
	char	szDebugMsg[100 + 1];
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "%s", pobTran->srBRec.szDate);
		inLogPrintf(AT, szDebugMsg);
		
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "%s", pobTran->srBRec.szTime);
		inLogPrintf(AT, szDebugMsg);
	}
	
	inFunc_SetEDCDateTime(pobTran->srBRec.szDate, pobTran->srBRec.szTime);

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_TICKET_ProcessReferral
Date&Time       :2016/9/14 上午 10:14
Describe        :
*/
int inNCCC_TICKET_ProcessReferral(TRANSACTION_OBJECT *pobTran)
{
        /* 輸入授權碼 */
        if (inFunc_REFERRAL_GetManualApproval(pobTran) != VS_SUCCESS)
                return (VS_ERROR);

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_TICKET_GetReversalCnt
Date&Time       :2016/9/13 下午 5:02
Describe        :
*/
int inNCCC_TICKET_GetReversalCnt(TRANSACTION_OBJECT *pobTran)
{
        int 	inCnt;
        char 	szReversalCnt[6 + 1];

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_GetReversalCnt() START!");

        memset(szReversalCnt, 0x00, sizeof(szReversalCnt));
        if (inGetReversalCnt(szReversalCnt) == VS_ERROR)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inGetReversalCnt ERROR!");
                return (VS_ERROR);
        }
        inCnt = atoi(szReversalCnt);

        return (inCnt);
}

/*
Function        :inNCCC_TICKET_SetReversalCnt
Date&Time       :2016/9/13 下午 5:02
Describe        :
*/
int inNCCC_TICKET_SetReversalCnt(TRANSACTION_OBJECT *pobTran, long lnSetMode)
{
        long 	lnCnt;
        char 	szReversalCnt[6 + 1];

        memset(szReversalCnt, 0x00, sizeof(szReversalCnt));
        if (inGetReversalCnt(szReversalCnt) == VS_ERROR)
                return (VS_ERROR);

        if (lnSetMode == _ADD_)
        {
                lnCnt = atol(szReversalCnt);
                if (lnCnt++ > 999)
                        lnCnt = 1;
        }
        else if (lnSetMode == _RESET_)
        {
                lnCnt = 0;
        }
        else
        {
                return (VS_ERROR);
        }

        memset(szReversalCnt, 0x00, sizeof(szReversalCnt));
        sprintf(szReversalCnt, "%06ld", lnCnt);
        if (inSetReversalCnt(szReversalCnt) == VS_ERROR)
                return (VS_ERROR);

        if (inSaveHDPTRec(pobTran->srBRec.inHDTIndex) < 0)
        {
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_TICKET_GetMustSettleBit
Date&Time       :2016/9/14 上午 10:12
Describe        :
*/
int inNCCC_TICKET_GetMustSettleBit(TRANSACTION_OBJECT *pobTran, char *szMustSettleBit)
{
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_TICKET_GetMustSettleBit() START!");

	/* 刷完卡就LoadHDPT，不另在Load */
        memset(szMustSettleBit, 0x00, sizeof(szMustSettleBit));
        if (inGetMustSettleBit(szMustSettleBit) == VS_ERROR)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inNCCC_TICKET_GetMustSettleBit ERROR!");
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_TICKET_SetMustSettleBit
Date&Time       :2016/9/14 上午 10:12
Describe        :
*/
int inNCCC_TICKET_SetMustSettleBit(TRANSACTION_OBJECT *pobTran, char *szMustSettleBit)
{
        if (inSetMustSettleBit(&szMustSettleBit[0]) == VS_ERROR)
                return (VS_ERROR);

        if (inSaveHDPTRec(pobTran->srBRec.inHDTIndex) < 0)
        {
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

/*
Function        :vdNCCC_TICKET_ISO_FormatDebug_DISP
Date&Time       :2016/11/30 下午 4:19
Describe        :顯示ISO Debug 
*/
void vdNCCC_TICKET_ISO_FormatDebug_DISP(TRANSACTION_OBJECT *pobTran, unsigned char *uszDebugBuf, int inSendLen)
{
	int				inOneLineLen = 34;
	int				i, inPrintLineCnt = 0, inCnt = 0, inField, inLen, inFieldLen;
	int				inBitMapCnt1, inBitMapCnt2;
	char				szPrtBuf[50 + 1], szBuf[5120 + 1], szBitMap[8 + 1];
	char				szPrintLineData[36 + 1];
	unsigned char			uszBitMap[9], uszBitMapDisp[66];
	ISO_TYPE_NCCC_TICKET_TABLE	srISOTypeTable;

	memset((char *)&srISOTypeTable, 0x00, sizeof(srISOTypeTable));
	memcpy((char *)&srISOTypeTable, (char *)&srNCCC_TICKET_ISOFunc[0], sizeof(srISOTypeTable));

        inLogPrintf(AT, "==========================================");

        if (uszDebugBuf[6] == 0x00 || uszDebugBuf[6] == 0x20 || uszDebugBuf[6] == 0x40)
                inLogPrintf(AT, "ISO8583 Format  <<Send Data>>");
	else
                inLogPrintf(AT, "ISO8583 Format  <<Recvive Data>>");

	memset(szBuf, 0x00, sizeof(szBuf));
	sprintf(szBuf, "TPDU = [");
        inFunc_BCD_to_ASCII(&szBuf[8], &uszDebugBuf[inCnt], 5);
	strcat(szBuf,"]");
        inLogPrintf(AT, szBuf);
	inCnt += 5;

	memset(szBuf, 0x00, sizeof(szBuf));
	sprintf(szBuf,"MTI  = [");
        inFunc_BCD_to_ASCII(&szBuf[8], &uszDebugBuf[inCnt], 2);
	strcat(szBuf,"]");
        inLogPrintf(AT, szBuf);
	inCnt += 2;

	memset(szBuf, 0x00, sizeof(szBuf));
	memset(szBitMap, 0x00, sizeof(szBitMap));
	memcpy(szBitMap, (char *)&uszDebugBuf[inCnt], 8);
	sprintf(szBuf, "BMap = [%02X %02X %02X %02X %02X %02X %02X %02X]",
						uszDebugBuf[inCnt + 0],
						uszDebugBuf[inCnt + 1],
						uszDebugBuf[inCnt + 2],
						uszDebugBuf[inCnt + 3],
						uszDebugBuf[inCnt + 4],
						uszDebugBuf[inCnt + 5],
						uszDebugBuf[inCnt + 6],
						uszDebugBuf[inCnt + 7]);
        inLogPrintf(AT, szBuf);
	memset(uszBitMapDisp, 0x00, sizeof(uszBitMapDisp));
	memset(uszBitMap, 0x00, sizeof(uszBitMap));
	memcpy(uszBitMap, (char *)&uszDebugBuf[inCnt], 8);

	for (inBitMapCnt1 = 0; inBitMapCnt1 < 8; inBitMapCnt1 ++)
        {
                for (inBitMapCnt2 = 0; inBitMapCnt2 < 8; inBitMapCnt2 ++)
                {
                        if (uszBitMap[inBitMapCnt1] & 0x80)
                                uszBitMapDisp[(inBitMapCnt1*8) + inBitMapCnt2 + 1] = '1';
                        else
                                uszBitMapDisp[(inBitMapCnt1*8) + inBitMapCnt2 + 1] = '0';

                        uszBitMap[inBitMapCnt1] = uszBitMap[inBitMapCnt1] << 1;
            }
        }

        inLogPrintf(AT, "  1234567890   1234567890   1234567890");
	memset(szBuf, 0x00, sizeof(szBuf));
	sprintf(szBuf, "0>%10.10s 1>%10.10s 2>%10.10s", &uszBitMapDisp[1], &uszBitMapDisp[11], &uszBitMapDisp[21]);
        inLogPrintf(AT, szBuf);
        sprintf(szBuf, "3>%10.10s 4>%10.10s 5>%10.10s", &uszBitMapDisp[31], &uszBitMapDisp[41], &uszBitMapDisp[51]);
        inLogPrintf(AT, szBuf);
        sprintf(szBuf, "6>%4.4s", &uszBitMapDisp[61]);
        inLogPrintf(AT, szBuf);

	inCnt += 8;

	for (i = 1; i <= 64; i ++)
	{
		if (!inNCCC_TICKET_BitMapCheck((unsigned char *)szBitMap, i))
			continue;

		inLen = 0;
		inFieldLen = 0;
		memset(szBuf, 0x00, sizeof(szBuf));
		sprintf(szBuf, "F_%02d ", i);
		inLen = 5;
                strcat(&szBuf[inLen ++], "[");
                
		switch (i)
		{
			case 3 :
				strcat(szBuf, "P_CODE ");
				inLen += 7;
				break;
			case 4 :
				strcat(szBuf, "AMT ");
				inLen += 4;
				break;
			case 11 :
				strcat(szBuf, "STAN ");
				inLen += 5;
				break;
			case 12 :
				strcat(szBuf, "TIME ");
				inLen += 5;
				break;
			case 13 :
				strcat(szBuf, "DATE ");
				inLen += 5;
				break;
			case 24 :
				strcat(szBuf, "NII ");
				inLen += 4;
				break;
			case 32 :
				strcat(szBuf, "AIIC ");
				inLen += 5;
				break;
			case 35 :
				break;
			case 37 :
				strcat(szBuf, "RRN ");
				inLen += 4;
				break;
			case 38 :
				strcat(szBuf, "AUTH_ID ");
				inLen += 8;
				break;
			case 39 :
				strcat(szBuf, "RESP_CODE ");
				inLen += 10;
				break;
			case 41 :
				strcat(szBuf, "TID ");
				inLen += 4;
				break;
			case 42 :
				strcat(szBuf, "MID ");
				inLen += 4;
				break;
			case 48 :
				strcat(szBuf, "STORE_ID ");
				inLen += 9;
				break;
			case 52 :
				strcat(szBuf, "PIN_BLOCK ");
				inLen += 10;
				break;
			case 54 :
				strcat(szBuf, "TIP_AMT ");
				inLen += 8;
				break;
			case 55 :
				strcat(szBuf, "EMV ");
				inLen += 4;
				break;
			case 56 :
				strcat(szBuf, "TICKET ");
				inLen += 7;
				break;
			case 57 :
				strcat(szBuf, "CSUM ");
				inLen += 5;
				break;
			case 58 :
				strcat(szBuf, "TABLE ");
				inLen += 6;
				break;
			case 59 :
				strcat(szBuf, "TABLE ");
				inLen += 6;
				break;
			case 64 :
				strcat(szBuf, "MAC ");
				inLen += 4;
				break;
			default :
				break;
		}       
		
		inField = inNCCC_TICKET_GetFieldIndex(i, srISOTypeTable.srISOFieldType);
		inFieldLen = inNCCC_TICKET_GetFieldLen(i, &uszDebugBuf[inCnt], srISOTypeTable.srISOFieldType, pobTran->srTRec.inTicketType);

		switch (srISOTypeTable.srISOFieldType[inField].inFieldType)
		{
			case _NCCC_TICKET_ISO_ASC_  :
				memcpy(&szBuf[inLen], &uszDebugBuf[inCnt], inFieldLen);
				break;
                        case _NCCC_TICKET_ISO_BYTE_3_  :
                                inFunc_BCD_to_ASCII(&szBuf[inLen], &uszDebugBuf[inCnt], 2);
				inCnt += 2;
				inLen += 4;
				strcat(szBuf, " ");
				inLen ++;
				inFieldLen --;
				inFieldLen --;

				if (srISOTypeTable.srISOFieldType[inField].uszDispAscii == VS_TRUE)
				{
					if (i != 58 && i != 59)
						memcpy(&szBuf[inLen], (char *)&uszDebugBuf[inCnt], inFieldLen);
				}
				else
				{
					if (i != 55 && i != 56)
                                                inFunc_BCD_to_ASCII(&szBuf[inLen], &uszDebugBuf[inCnt], inFieldLen);
				}

				break;
                        case _NCCC_TICKET_ISO_BYTE_1_ :
                                inFunc_BCD_to_ASCII(&szBuf[inLen], &uszDebugBuf[inCnt ++], 1);
				inLen ++;
				strcat(szBuf, " ");
				inLen ++;

				inFieldLen --;
                                inFunc_BCD_to_ASCII(&szBuf[inLen], &uszDebugBuf[inCnt], inFieldLen);
				break;
			case _NCCC_TICKET_ISO_NIBBLE_2_  :
			case _NCCC_TICKET_ISO_BYTE_2_  :
                                inFunc_BCD_to_ASCII(&szBuf[inLen], &uszDebugBuf[inCnt ++], 1);
				inLen += 2;
				strcat(szBuf, " ");
				inLen ++;

				inFieldLen --;
				if (srISOTypeTable.srISOFieldType[inField].uszDispAscii == VS_TRUE)
					memcpy(&szBuf[inLen], (char *)&uszDebugBuf[inCnt], inFieldLen);
				else
                                        inFunc_BCD_to_ASCII(&szBuf[inLen], &uszDebugBuf[inCnt], inFieldLen);

				break;
			case _NCCC_TICKET_ISO_BCD_  :
				if (srISOTypeTable.srISOFieldType[inField].uszDispAscii == VS_TRUE)
					memcpy(&szBuf[inLen], (char *)&uszDebugBuf[inCnt], inFieldLen);
				else
                                        inFunc_BCD_to_ASCII(&szBuf[inLen], &uszDebugBuf[inCnt], inFieldLen);

				break;
			case _NCCC_TICKET_ETICKET_CARD_:
				if (srISOTypeTable.srISOFieldType[inField].uszDispAscii == VS_TRUE)
					memcpy(&szBuf[inLen], (char *)&uszDebugBuf[inCnt], inFieldLen);
				else
                                        inFunc_BCD_to_ASCII(&szBuf[inLen], &uszDebugBuf[inCnt], inFieldLen);

				break;
			default :
				break;
		}

                strcat(&szBuf[inLen ++], "]");
                
		inPrintLineCnt = 0;
		while ((inPrintLineCnt * inOneLineLen) < strlen(szBuf))
		{
			memset(szPrintLineData, 0x00, sizeof(szPrintLineData));
			memset(szPrtBuf, 0x00, sizeof(szPrtBuf));
			if (((inPrintLineCnt + 1) * inOneLineLen) > strlen(szBuf))
			{
				strcat(szPrintLineData, &szBuf[inPrintLineCnt * inOneLineLen]);
			}
			else
			{
				memcpy(szPrintLineData, &szBuf[inPrintLineCnt * inOneLineLen], inOneLineLen);
			}
			
			sprintf(szPrtBuf, "  %s", szPrintLineData);
			
			inLogPrintf(AT, szPrtBuf);
			inPrintLineCnt ++;
		};

		if (i == 56)
			vdNCCC_TICKET_ISO_FormatDebug_DISP_56(&uszDebugBuf[inCnt], inFieldLen);
		else if (i == 59)
			vdNCCC_TICKET_ISO_FormatDebug_DISP_59(&uszDebugBuf[inCnt], inFieldLen);
                
		inCnt += inFieldLen;
	}

        inLogPrintf(AT, "==========================================");
}

void vdNCCC_TICKET_ISO_FormatDebug_DISP_56(unsigned char *uszDebugBuf, int inFieldLen)
{
	int	inCnt = 0, inTableLen = 0;
	int	inPrintLineCnt = 0, inOneLineLen = 34;
	char	szTagData[38 + 1];
	char	szPrintBuf[5120 + 1], szTemplate[5120 + 1];
	char	szTemp[50 + 1];
	char	szI_FESMode[1 + 1] = {0};
	
	memset(szI_FESMode, 0x00, sizeof(szI_FESMode));
	inGetI_FES_Mode(szI_FESMode);

	while (inFieldLen > inCnt)
	{
		if (!memcmp(&uszDebugBuf[inCnt], "IP00", 4) ||
		    !memcmp(&uszDebugBuf[inCnt], "IP01", 4) ||
		    !memcmp(&uszDebugBuf[inCnt], "IP02", 4))
		{
			/* Table ID */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 4);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, " Table ID   [%s]", szTemplate);
                        inLogPrintf(AT, szPrintBuf);
			inCnt += 4;
			/* Table Length */
			inTableLen = ((uszDebugBuf[inCnt] / 16 * 10 + uszDebugBuf[inCnt] % 16) * 100);
			inTableLen += (uszDebugBuf[inCnt + 1] / 16 * 10) + uszDebugBuf[inCnt + 1] % 16;
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, " Table Len  [%02X %02X][%d]", uszDebugBuf[inCnt], uszDebugBuf[inCnt + 1], inTableLen);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 2;
			
			/* Table Data */
			if (!memcmp(&szTemplate[0], "IP01", 4))
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inFunc_BCD_to_ASCII(&szTemplate[0], &uszDebugBuf[inCnt], inTableLen);
				
				if (inTableLen <= 8)
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, " Table Data [%s]", szTemplate);
					inLogPrintf(AT, szPrintBuf);
				}
				else
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, " Table Data ");
					inLogPrintf(AT, szPrintBuf);

					inPrintLineCnt = 0;
					while ((inPrintLineCnt * inOneLineLen) < inTableLen)
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						memset(szTagData, 0x00, sizeof(szTagData));
						if (((inPrintLineCnt + 1) * inOneLineLen) > inTableLen)
						{
							memcpy(szTagData, &szTemplate[inPrintLineCnt * inOneLineLen], (inTableLen - (inPrintLineCnt + 1) * inOneLineLen));
						}
						else
						{
							memcpy(szTagData, &szTemplate[inPrintLineCnt * inOneLineLen], inOneLineLen);
						}
						sprintf(szPrintBuf, "  [%s]", szTagData);
						inLogPrintf(AT, szPrintBuf);
						inPrintLineCnt ++;
					}

				}
			}
			else if (!memcmp(&szTemplate[0], "IP02", 4))
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], inTableLen);
				
				if (inTableLen <= 8)
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, " Table Data [%s]", szTemplate);
					inLogPrintf(AT, szPrintBuf);
				}
				else
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, " Table Data ");
					inLogPrintf(AT, szPrintBuf);

					inPrintLineCnt = 0;
					while ((inPrintLineCnt * inOneLineLen) < inTableLen)
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						memset(szTagData, 0x00, sizeof(szTagData));
						if (((inPrintLineCnt + 1) * inOneLineLen) > inTableLen)
						{
							memset(szTemp, 0x00, sizeof(szTemp));
							strcat(szTemp, &szTemplate[inPrintLineCnt * inOneLineLen]);
							memcpy(szTagData, szTemp, strlen(szTemp) - 2);
							sprintf(szPrintBuf, "  [%s]", szTagData);
							inLogPrintf(AT, szPrintBuf);
							
							memset(szTagData, 0x00, sizeof(szTagData));
							sprintf(szTagData, "\"%02X\"\"%02X\"", szTemp[strlen(szTemp) - 2], szTemp[strlen(szTemp) - 1]);
							sprintf(szPrintBuf, "  [%s]", szTagData);
							inLogPrintf(AT, szPrintBuf);
							
							inPrintLineCnt ++;
						}
						else
						{
							memcpy(szTagData, &szTemplate[inPrintLineCnt * inOneLineLen], inOneLineLen);
							sprintf(szPrintBuf, "  [%s]", szTagData);
							inLogPrintf(AT, szPrintBuf);
							inPrintLineCnt ++;
						}
					}

				}
			}
			else
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], inTableLen);
				
				if (inTableLen <= 8)
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, " Table Data [%s]", szTemplate);
					inLogPrintf(AT, szPrintBuf);
				}
				else
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, " Table Data ");
					inLogPrintf(AT, szPrintBuf);

					inPrintLineCnt = 0;
					while ((inPrintLineCnt * inOneLineLen) < inTableLen)
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						memset(szTagData, 0x00, sizeof(szTagData));
						if (((inPrintLineCnt + 1) * inOneLineLen) > inTableLen)
						{
							memcpy(szTagData, &szTemplate[inPrintLineCnt * inOneLineLen], (inTableLen - (inPrintLineCnt + 1) * inOneLineLen));
						}
						else
						{
							memcpy(szTagData, &szTemplate[inPrintLineCnt * inOneLineLen], inOneLineLen);
						}
						sprintf(szPrintBuf, "  [%s]", szTagData);
						inLogPrintf(AT, szPrintBuf);
						inPrintLineCnt ++;
					}

				}
			}
			
			inCnt += inTableLen;
                        inLogPrintf(AT, " -----------------------------------------");
		}
		else if (!memcmp(&uszDebugBuf[inCnt], "EZ01", 4))
		{
			/* Table ID */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 4);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, " Table ID   [%s]", szTemplate);
                        inLogPrintf(AT, szPrintBuf);
			inCnt += 4;
			/* Table Length */
			inTableLen = ((uszDebugBuf[inCnt] / 16 * 10 + uszDebugBuf[inCnt] % 16) * 100);
			inTableLen += (uszDebugBuf[inCnt + 1] / 16 * 10) + uszDebugBuf[inCnt + 1] % 16;
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, " Table Len  [%02X %02X][%d]", uszDebugBuf[inCnt], uszDebugBuf[inCnt + 1], inTableLen);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 2;
			if (inTableLen <= 4)
			{
				/* Table Data */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], inTableLen);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, " Table Data [%s]", szTemplate);
				inLogPrintf(AT, szPrintBuf);
			}
			else
			{
				/* Table Data */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], inTableLen);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, " Table Data ");
				inLogPrintf(AT, szPrintBuf);
				
				inPrintLineCnt = 0;
				while ((inPrintLineCnt * inOneLineLen) < inTableLen)
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					memset(szTagData, 0x00, sizeof(szTagData));
					if (((inPrintLineCnt + 1) * inOneLineLen) > inTableLen)
					{
						memcpy(szTagData, &szTemplate[inPrintLineCnt * inOneLineLen], (inTableLen - (inPrintLineCnt + 1) * inOneLineLen));
					}
					else
					{
						memcpy(szTagData, &szTemplate[inPrintLineCnt * inOneLineLen], inOneLineLen);
					}
					sprintf(szPrintBuf, "  [%s]", szTagData);
//					inLogPrintf(AT, szPrintBuf);
					inPrintLineCnt ++;
				}
				
			}
			
			inCnt += inTableLen;
                        inLogPrintf(AT, " -----------------------------------------");
		}
		else  if (!memcmp(&uszDebugBuf[inCnt], "IC01", 4) ||
		          !memcmp(&uszDebugBuf[inCnt], "IC02", 4))
		{
			/* Table ID */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 4);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, " Table ID   [%s]", szTemplate);
                        inLogPrintf(AT, szPrintBuf);
			inCnt += 4;
			/* Table Length */
			inTableLen = ((uszDebugBuf[inCnt] / 16 * 10 + uszDebugBuf[inCnt] % 16) * 100);
			inTableLen += (uszDebugBuf[inCnt + 1] / 16 * 10) + uszDebugBuf[inCnt + 1] % 16;
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, " Table Len  [%02X %02X][%d]", uszDebugBuf[inCnt], uszDebugBuf[inCnt + 1], inTableLen);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 2;
			
			/* Table Data */
			if (!memcmp(&szTemplate[0], "IC01", 4))
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inFunc_BCD_to_ASCII(&szTemplate[0], &uszDebugBuf[inCnt], inTableLen);
				
				if (inTableLen <= 8)
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, " Table Data [%s]", szTemplate);
					inLogPrintf(AT, szPrintBuf);
				}
				else
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, " Table Data ");
					inLogPrintf(AT, szPrintBuf);

					inPrintLineCnt = 0;
					while ((inPrintLineCnt * inOneLineLen) < inTableLen)
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						memset(szTagData, 0x00, sizeof(szTagData));
						if (((inPrintLineCnt + 1) * inOneLineLen) > inTableLen)
						{
							memcpy(szTagData, &szTemplate[inPrintLineCnt * inOneLineLen], (inTableLen - (inPrintLineCnt + 1) * inOneLineLen));
						}
						else
						{
							memcpy(szTagData, &szTemplate[inPrintLineCnt * inOneLineLen], inOneLineLen);
						}
						sprintf(szPrintBuf, "  [%s]", szTagData);
						inLogPrintf(AT, szPrintBuf);
						inPrintLineCnt ++;
					}

				}
			}
			else if (!memcmp(&szTemplate[0], "IC02", 4))
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], inTableLen);
				
				if (inTableLen <= 8)
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, " Table Data [%s]", szTemplate);
					inLogPrintf(AT, szPrintBuf);
				}
				else
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, " Table Data ");
					inLogPrintf(AT, szPrintBuf);

					inPrintLineCnt = 0;
					while ((inPrintLineCnt * inOneLineLen) < inTableLen)
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						memset(szTagData, 0x00, sizeof(szTagData));
						if (((inPrintLineCnt + 1) * inOneLineLen) > inTableLen)
						{
							memset(szTemp, 0x00, sizeof(szTemp));
							strcat(szTemp, &szTemplate[inPrintLineCnt * inOneLineLen]);
							memcpy(szTagData, szTemp, strlen(szTemp) - 2);
							sprintf(szPrintBuf, "  [%s]", szTagData);
							inLogPrintf(AT, szPrintBuf);
							
							memset(szTagData, 0x00, sizeof(szTagData));
							sprintf(szTagData, "\"%02X\"\"%02X\"", szTemp[strlen(szTemp) - 2], szTemp[strlen(szTemp) - 1]);
							sprintf(szPrintBuf, "  [%s]", szTagData);
							inLogPrintf(AT, szPrintBuf);
							
							inPrintLineCnt ++;
						}
						else
						{
							memcpy(szTagData, &szTemplate[inPrintLineCnt * inOneLineLen], inOneLineLen);
							sprintf(szPrintBuf, "  [%s]", szTagData);
							inLogPrintf(AT, szPrintBuf);
							inPrintLineCnt ++;
						}
					}

				}
			}
			else
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], inTableLen);
				
				if (inTableLen <= 8)
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, " Table Data [%s]", szTemplate);
					inLogPrintf(AT, szPrintBuf);
				}
				else
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, " Table Data ");
					inLogPrintf(AT, szPrintBuf);

					inPrintLineCnt = 0;
					while ((inPrintLineCnt * inOneLineLen) < inTableLen)
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						memset(szTagData, 0x00, sizeof(szTagData));
						if (((inPrintLineCnt + 1) * inOneLineLen) > inTableLen)
						{
							memcpy(szTagData, &szTemplate[inPrintLineCnt * inOneLineLen], (inTableLen - (inPrintLineCnt + 1) * inOneLineLen));
						}
						else
						{
							memcpy(szTagData, &szTemplate[inPrintLineCnt * inOneLineLen], inOneLineLen);
						}
						sprintf(szPrintBuf, "  [%s]", szTagData);
						inLogPrintf(AT, szPrintBuf);
						inPrintLineCnt ++;
					}

				}
			}
			
			inCnt += inTableLen;
                        inLogPrintf(AT, " -----------------------------------------");
		}
		else
		{
			/* IFES會tSAM加密 */
			if (memcmp(szI_FESMode, "Y", 1)  == 0)
			{
				/* Table Data */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inFunc_BCD_to_ASCII(&szTemplate[0], &uszDebugBuf[inCnt], inFieldLen);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, " Table Data ");
				inLogPrintf(AT, szPrintBuf);

				inPrintLineCnt = 0;
				inOneLineLen = 42;

				while ((inPrintLineCnt * inOneLineLen) < inFieldLen)
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					memset(szTagData, 0x00, sizeof(szTagData));
					if (((inPrintLineCnt + 1) * inOneLineLen) > inFieldLen)
					{
						memcpy(szTagData, &szTemplate[inPrintLineCnt * inOneLineLen], (inTableLen - (inPrintLineCnt + 1) * inOneLineLen));
					}
					else
					{
						memcpy(szTagData, &szTemplate[inPrintLineCnt * inOneLineLen], inOneLineLen);
					}
					sprintf(szPrintBuf, "  [%s]", szTagData);
					inLogPrintf(AT, szPrintBuf);
					inPrintLineCnt ++;
				}

				inCnt += inTableLen;
				inLogPrintf(AT, " -----------------------------------------");
			}
			else
			{
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, " F_56 OTHER DATA [%02X]", uszDebugBuf[inCnt]);
				inLogPrintf(AT, szPrintBuf);
				inCnt ++;
			}
		}
		
	}
	
}

void vdNCCC_TICKET_ISO_FormatDebug_DISP_59(unsigned char *uszDebugBuf, int inFieldLen)
{
	int	inCnt = 0, inTableLen;
	int	inPrintLineCnt = 0, inOneLineLen = 34;
	char	szTagData[38 + 1];
	char	szPrintBuf[1024 + 1], szTemplate[1024 + 1], szBig5[1024 + 1] = {0};

	while (inFieldLen > inCnt)
	{
		if (!memcmp(&uszDebugBuf[inCnt], "N1", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "N2", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "N3", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "N6", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "NA", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "ND", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "NF", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "ET", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "ES", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "ED", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "PS", 2))
		{
			/* Table ID */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 2);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, " Table ID   [%s]", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 2;
			/* Table Length */
			inTableLen = ((uszDebugBuf[inCnt] / 16 * 10 + uszDebugBuf[inCnt] % 16) * 100);
			inTableLen += (uszDebugBuf[inCnt + 1] / 16 * 10) + uszDebugBuf[inCnt + 1] % 16;
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, " Table Len  [%02X %02X][%d]", uszDebugBuf[inCnt], uszDebugBuf[inCnt + 1], inTableLen);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 2;
			if (inTableLen <= 8)
			{
				/* Table Data */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], inTableLen);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, " Table Data [%s]", szTemplate);
				inLogPrintf(AT, szPrintBuf);
			}
			else
			{
				/* Table Data */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], inTableLen);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, " Table Data ");
				inLogPrintf(AT, szPrintBuf);
				
				inPrintLineCnt = 0;
				while ((inPrintLineCnt * inOneLineLen) < inTableLen)
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					memset(szTagData, 0x00, sizeof(szTagData));
					if (((inPrintLineCnt + 1) * inOneLineLen) > inTableLen)
					{
						memcpy(szTagData, &szTemplate[inPrintLineCnt * inOneLineLen], (inTableLen - (inPrintLineCnt + 1) * inOneLineLen));
					}
					else
					{
						memcpy(szTagData, &szTemplate[inPrintLineCnt * inOneLineLen], inOneLineLen);
					}
					sprintf(szPrintBuf, "  [%s]", szTagData);
					inLogPrintf(AT, szPrintBuf);
					inPrintLineCnt ++;
				}
				
			}

			inCnt += inTableLen;
                        inLogPrintf(AT, " -----------------------------------------");
		}
		else if (!memcmp(&uszDebugBuf[inCnt], "L1", 2) || !memcmp(&uszDebugBuf[inCnt], "L5", 2))
		{
			/* Table ID */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 2);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, " Table ID   [%s]", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 2;
			/* Table Length */
			inTableLen = ((uszDebugBuf[inCnt] / 16 * 1000 + uszDebugBuf[inCnt] % 16) * 100);
			inTableLen += (uszDebugBuf[inCnt + 1] / 16 * 10) + uszDebugBuf[inCnt + 1] % 16;
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, " Table Len  [%02X %02X][%d]", uszDebugBuf[inCnt], uszDebugBuf[inCnt + 1], inTableLen);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 2;
			/* 列印優惠或廣告資訊之個數 (1 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			szTemplate[0] = uszDebugBuf[inCnt];
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  列印優惠個數 = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt ++;
			/* 優惠活動SN (22 Bytes) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 22);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  優惠活動SN = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 22;

			if (inTableLen == 23)
				continue;

			/* 是否列印補充資訊 (1 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  是否列印補充資訊 = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt ++;
			/* 補充資訊長度 (2 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 2);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  補充資訊長度 = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 2;
			/* 列印補充資訊位置 (1 Byte) */ /* 2014/1/22 下午 03:26:24 add by kakab NCCC優惠平台端末機連線規格_V1_15_20140109 新增參數列印位置 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  列印補充資訊位置 = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt ++;
			/* 補充資訊內容 (60 Byte) */ /* 2014/1/22 下午 03:26:24 add by kakab NCCC優惠平台端末機連線規格_V1_15_20140109 增大為60Bytes */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szBig5, 0x00, sizeof(szBig5));
			memcpy(szBig5, (char*)&uszDebugBuf[inCnt], 60);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			inFunc_Big5toUTF8(szTemplate, szBig5);
			sprintf(szPrintBuf, "  補充資訊內容 = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 60;

			if (inTableLen == 87)
				continue;

			/* 是否列印優惠活動之標題 (1 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  是否列印優惠活動之標題 = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt ++;
			/* 優惠活動之標題長度 (2 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 2);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  優惠活動之標題長度 = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 2;
			/* 優惠活動之標題內容 (50 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szBig5, 0x00, sizeof(szBig5));
			memcpy(szBig5, (char*)&uszDebugBuf[inCnt], 50);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			inFunc_Big5toUTF8(szTemplate, szBig5);
			sprintf(szPrintBuf, "  優惠活動之標題內容 = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 50;
			/* 是否列印一維條碼(一) (1 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  是否列印一維條碼(一) = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt ++;
			/* 一維條碼(一)長度 (2 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 2);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  一維條碼(一)長度 = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 2;
			/* 一維條碼(一) (20 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 20);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  一維條碼(一) = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 20;
			/* 是否列印一維條碼(二) (1 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  是否列印一維條碼(二) = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt ++;
			/* 一維條碼(二)長度 (2 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 2);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  一維條碼(二)長度 = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 2;
			/* 一維條碼(二) (20 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 20);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  一維條碼(二) = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 20;
			/* 保留欄位(預設空白) */
			inCnt ++;
			/* 是否列印優惠活動之內容 (1 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  是否列印優惠活動之內容 = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt ++;
			/* 優惠活動之內容長度 (3 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 3);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  優惠活動之內容長度 = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 3;
			/* 優惠活動之內容 (200 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szBig5, 0x00, sizeof(szBig5));
			memcpy(szBig5, (char*)&uszDebugBuf[inCnt], 200);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			inFunc_Big5toUTF8(szTemplate, szBig5);
			sprintf(szPrintBuf, "  優惠活動之內容 = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 200;
			/* 檢查碼(Checksum) (1 Byte) */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  檢查碼 = %02x", uszDebugBuf[inCnt]);
			inLogPrintf(AT, szPrintBuf);
			inCnt ++;
			
			inLogPrintf(AT, " -----------------------------------------");
			
			if (inTableLen == 392)
				continue;
			
			/* 是否列印優惠活動之標題 (1 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  是否列印優惠活動之標題 = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt ++;
			/* 優惠活動之標題長度 (2 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 2);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  優惠活動之標題長度 = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 2;
			/* 優惠活動之標題內容 (50 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szBig5, 0x00, sizeof(szBig5));
			memcpy(szBig5, (char*)&uszDebugBuf[inCnt], 50);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			inFunc_Big5toUTF8(szTemplate, szBig5);
			sprintf(szPrintBuf, "  優惠活動之標題內容 = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 50;
			/* 是否列印一維條碼(一) (1 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  是否列印一維條碼(一) = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt ++;
			/* 一維條碼(一)長度 (2 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 2);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  一維條碼(一)長度 = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 2;
			/* 一維條碼(一) (20 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 20);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  一維條碼(一) = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 20;
			/* 是否列印一維條碼(二) (1 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  是否列印一維條碼(二) = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt ++;
			/* 一維條碼(二)長度 (2 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 2);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  一維條碼(二)長度 = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 2;
			/* 一維條碼(二) (20 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 20);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  一維條碼(二) = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 20;
			/* 保留欄位(預設空白) */
			inCnt ++;
			/* 是否列印優惠活動之內容 (1 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  是否列印優惠活動之內容 = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt ++;
			/* 優惠活動之內容長度 (3 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 3);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  優惠活動之內容長度 = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 3;
			/* 優惠活動之內容 (200 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szBig5, 0x00, sizeof(szBig5));
			memcpy(szBig5, (char*)&uszDebugBuf[inCnt], 200);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			inFunc_Big5toUTF8(szTemplate, szBig5);
			sprintf(szPrintBuf, "  優惠活動之內容 = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 200;
			/* 檢查碼(Checksum) (1 Byte) */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  檢查碼 = %02x", uszDebugBuf[inCnt]);
			inLogPrintf(AT, szPrintBuf);
			inCnt ++;
			inLogPrintf(AT, " -----------------------------------------");
		}
		else if (!memcmp(&uszDebugBuf[inCnt], "L2", 2))
		{
			/* Table ID */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 2);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, " Table ID   [%s]", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 2;
			/* Table Length */
			inTableLen = ((uszDebugBuf[inCnt] / 16 * 1000 + uszDebugBuf[inCnt] % 16) * 100);
			inTableLen += (uszDebugBuf[inCnt + 1] / 16 * 10) + uszDebugBuf[inCnt + 1] % 16;
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, " Table Len  [%02X %02X][%d]", uszDebugBuf[inCnt], uszDebugBuf[inCnt + 1], inTableLen);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 2;
			/* 列印優惠或廣告資訊之個數 (1 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			szTemplate[0] = uszDebugBuf[inCnt];
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  列印優惠個數 = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt ++;
			/* 優惠活動SN (22 Bytes) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 22);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  優惠活動SN = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 22;

			if (inTableLen == 23)
				continue;

			/* 是否列印補充資訊 (1 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  是否列印補充資訊 = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt ++;
			/* 補充資訊長度 (2 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 2);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  補充資訊長度 = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 2;
			/* 列印補充資訊位置 (1 Byte) */  /* 2014/1/22 下午 03:26:24 add by kakab NCCC優惠平台端末機連線規格_V1_15_20140109 新增參數列印位置 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  列印補充資訊位置 = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt ++;
			/* 補充資訊內容 (60 Byte) */ /* 2014/1/22 下午 03:26:24 add by kakab NCCC優惠平台端末機連線規格_V1_15_20140109 增大為60Bytes */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szBig5, 0x00, sizeof(szBig5));
			memcpy(szBig5, (char*)&uszDebugBuf[inCnt], 60);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			inFunc_Big5toUTF8(szTemplate, szBig5);
			sprintf(szPrintBuf, "  補充資訊內容 = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 60;

			if (inTableLen == 87)
				continue;

			/* 是否列印(優惠活動||累計訊息)之標題 (1 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  是否列印優惠or訊息標題 = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt ++;
			/* (優惠活動||累計訊息)之標題長度。 (2 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 2);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  優惠or訊息標題長度 = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 2;
			/* (優惠活動||累計訊息)之標題內容。(端末機列印時需自動置中) (50 Bytes) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szBig5, 0x00, sizeof(szBig5));
			memcpy(szBig5, (char*)&uszDebugBuf[inCnt], 50);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			inFunc_Big5toUTF8(szTemplate, szBig5);
			sprintf(szPrintBuf, "  優惠or訊息內容 = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 50;
			/* 是否列印一維條碼(一) (1 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  是否列印一維條碼(一) = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt ++;
			/* 一維條碼(一)長度 (2 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 2);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  一維條碼(一)長度 = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 2;
			/* 一維條碼(一) (20 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 20);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  一維條碼(一) = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 20;
			/* 是否列印一維條碼(二) (1 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  是否列印一維條碼(二) = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt ++;
			/* 一維條碼(二)長度 (2 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 2);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  一維條碼(二)長度 = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 2;
			/* 一維條碼(二) (20 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 20);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  一維條碼(二) = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 20;
			/* 保留欄位(預設空白) */
			inCnt ++;
			/* 是否列印(優惠活動||累計訊息)之內容。 (1 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  是否列印優惠or訊息)之內容 = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt ++;
			/* (優惠活動||累計訊息)之內容長度 (3 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 3);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  優惠or訊息內容長度 = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 3;
			/* (優惠活動||累計訊息)之內容 (200 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szBig5, 0x00, sizeof(szBig5));
			memcpy(szBig5, (char*)&uszDebugBuf[inCnt], 200);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			inFunc_Big5toUTF8(szTemplate, szBig5);
			sprintf(szPrintBuf, "  優惠or訊息內容 = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 200;
			/* 檢查碼(Checksum) (1 Byte) */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  檢查碼 = %02x", uszDebugBuf[inCnt]);
			inLogPrintf(AT, szPrintBuf);
			inCnt ++;
			/* 廣告資訊 */
			/* 是否列印廣告資訊之標題。 (1 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  是否列印廣告標題 = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt ++;
			/* 廣告資訊之標題長度。 (2 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 2);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  廣告標題長度 = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 2;
			/* 廣告資訊之標題內容。(端末機列印時需自動置中) (50 Bytes) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szBig5, 0x00, sizeof(szBig5));
			memcpy(szBig5, (char*)&uszDebugBuf[inCnt], 50);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			inFunc_Big5toUTF8(szTemplate, szBig5);
			sprintf(szPrintBuf, "  廣告標題內容 = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 50;
			/* 是否列印廣告資訊。 (1 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  是否列印廣告資訊 = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt ++;
			/* 廣告資訊長度。 (3 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 3);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  廣告資訊長度 = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 3;
			/* 廣告資訊之內容 (150 Bytes) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szBig5, 0x00, sizeof(szBig5));
			memcpy(szBig5, (char*)&uszDebugBuf[inCnt], 150);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			inFunc_Big5toUTF8(szTemplate, szBig5);
			sprintf(szPrintBuf, "  廣告資訊內容 = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 150;
			/* 是否列印URL資訊之QR Code。 (1 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  是否列印QRCode = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt ++;
			/* URL資訊長度。 (3 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 3);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  QRCode長度 = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 3;
			/* URL資訊之內容 (100 Bytes) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 100);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  QRcode內容 = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 100;
			/* 檢查碼(Checksum) (1 Byte) */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  檢查碼 = %02x", uszDebugBuf[inCnt]);
			inLogPrintf(AT, szPrintBuf);
			inCnt ++;
			inLogPrintf(AT, " -----------------------------------------");
		}
		else if (!memcmp(&uszDebugBuf[inCnt], "L3", 2))
		{
			/* Table ID */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 2);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, " Table ID   [%s]", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 2;
			/* Table Length */
			inTableLen = ((uszDebugBuf[inCnt] / 16 * 1000 + uszDebugBuf[inCnt] % 16) * 100);
			inTableLen += (uszDebugBuf[inCnt + 1] / 16 * 10) + uszDebugBuf[inCnt + 1] % 16;
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, " Table Len  [%02X %02X][%d]", uszDebugBuf[inCnt], uszDebugBuf[inCnt + 1], inTableLen);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 2;
			/* 兌換方式 (1 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  兌換方式  = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt ++;
			/* 優惠活動SN (22 Bytes) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 22);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  優惠活動SN = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 22;
			if (inTableLen == 23)
				continue;

			/* 是否列印補充資訊 (1 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  是否列印補充資訊 = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt ++;
			/* 補充資訊長度 (2 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 2);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  補充資訊長度 = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 2;
			/* 補充資訊內容 (20 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szBig5, 0x00, sizeof(szBig5));
			memcpy(szBig5, (char*)&uszDebugBuf[inCnt], 20);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			inFunc_Big5toUTF8(szTemplate, szBig5);
			sprintf(szPrintBuf, "  補充資訊內容 = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 20;

			if (inTableLen == 46)
				continue;

			/* 是否上傳一維條碼(一) (1 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  是否上傳一維條碼(一) = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt ++;
			/* 一維條碼(一)長度 (2 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 2);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  一維條碼(一)長度 = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 2;
			/* 一維條碼(一) (20 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 20);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  一維條碼(一) = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 20;
			/* 是否上傳一維條碼(二) (1 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  是否上傳一維條碼(二) = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt ++;
			/* 一維條碼(二)長度 (2 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 2);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  一維條碼(二)長度 = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 2;
			/* 一維條碼(二) (20 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 20);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  一維條碼(二) = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 20;

			if (inTableLen == 92)
				continue;
			/* 第一段核銷資訊 */
			/* 是否列印兌換核銷資訊(一)(1 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  是否列印核銷資訊(一) = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt ++;
			/* 核銷資訊(一)長度 (3 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 3);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  核銷資訊(一)長度 = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 3;
			/* 核銷資訊(一) (100 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szBig5, 0x00, sizeof(szBig5));
			memcpy(szBig5, (char*)&uszDebugBuf[inCnt], 100);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			inFunc_Big5toUTF8(szTemplate, szBig5);
			sprintf(szPrintBuf, "  核銷資訊(一) = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 100;
			/* 第二段核銷資訊 */
			/* 是否列印兌換核銷資訊(二)(1 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  是否列印核銷資訊(二) = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt ++;
			/* 核銷資訊(二)長度 (3 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 3);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  核銷資訊(二)長度 = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 3;
			/* 核銷資訊(二) (100 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szBig5, 0x00, sizeof(szBig5));
			memcpy(szBig5, (char*)&uszDebugBuf[inCnt], 100);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			inFunc_Big5toUTF8(szTemplate, szBig5);
			sprintf(szPrintBuf, "  核銷資訊(二) = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 100;
			/* 檢查碼(Checksum) (1 Byte) */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  檢查碼 = %02x", uszDebugBuf[inCnt]);
			inLogPrintf(AT, szPrintBuf);
			inCnt ++;
			inLogPrintf(AT, " -----------------------------------------");
		}
		else
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, " F_59 OTHER DATA [%02X]", uszDebugBuf[inCnt]);
			inLogPrintf(AT, szPrintBuf);
			inCnt ++;
		}
		
	}
	
}

/*
Function        :vdNCCC_TICKET_ISO_FormatDebug_PRINT
Date&Time       :2016/11/30 下午 4:20
Describe        :列印ISO Debug
*/
void vdNCCC_TICKET_ISO_FormatDebug_PRINT(TRANSACTION_OBJECT *pobTran, unsigned char *uszDebugBuf, int inSendLen)
{
	int			inOneLineLen = 34, inPrintLineCnt = 0;
	int			i, inCnt = 0, inField, inLen, inFieldLen;
	int			inBitMapCnt1, inBitMapCnt2;
	char			szPrtBuf[50 + 1], szBuf[5120 + 1], szBitMap[8 + 1];
	char			szPrintLineData[36 + 1];
	unsigned char		uszBitMap[9], uszBitMapDisp[66];
	unsigned char		uszBuffer[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	BufferHandle		srBhandle;
	FONT_ATTRIB		srFont_Attrib;
	ISO_TYPE_NCCC_TICKET_TABLE srISOTypeTable;

	/* 是否有列印功能 */
	if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
	{
		
	}
	else
	{
		/* 初始化 */
		inPRINT_Buffer_Initial(uszBuffer, _BUFFER_MAX_LINE_, &srFont_Attrib, &srBhandle);

		memset((char *)&srISOTypeTable, 0x00, sizeof(srISOTypeTable));
		memcpy((char *)&srISOTypeTable, (char *)&srNCCC_TICKET_ISOFunc[0], sizeof(srISOTypeTable));

		inPRINT_Buffer_PutIn("==========================================", _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		if (uszDebugBuf[6] == 0x00 || uszDebugBuf[6] == 0x20 || uszDebugBuf[6] == 0x40)
			inPRINT_Buffer_PutIn("ISO8583 Format  <<Send Data>>", _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else
			inPRINT_Buffer_PutIn("ISO8583 Format  <<Recvive Data>>", _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szBuf, 0x00, sizeof(szBuf));
		sprintf(szBuf, "TPDU = [");
		inFunc_BCD_to_ASCII(&szBuf[8], &uszDebugBuf[inCnt], 5);
		strcat(szBuf,"]");
		inPRINT_Buffer_PutIn(szBuf, _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inCnt += 5;

		memset(szBuf, 0x00, sizeof(szBuf));
		sprintf(szBuf,"MTI  = [");
		inFunc_BCD_to_ASCII(&szBuf[8], &uszDebugBuf[inCnt], 2);
		strcat(szBuf,"]");
		inPRINT_Buffer_PutIn(szBuf, _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inCnt += 2;

		memset(szBuf, 0x00, sizeof(szBuf));
		memset(szBitMap, 0x00, sizeof(szBitMap));
		memcpy(szBitMap, (char *)&uszDebugBuf[inCnt], 8);
		sprintf(szBuf, "BMap = [%02X %02X %02X %02X %02X %02X %02X %02X]",
							uszDebugBuf[inCnt + 0],
							uszDebugBuf[inCnt + 1],
							uszDebugBuf[inCnt + 2],
							uszDebugBuf[inCnt + 3],
							uszDebugBuf[inCnt + 4],
							uszDebugBuf[inCnt + 5],
							uszDebugBuf[inCnt + 6],
							uszDebugBuf[inCnt + 7]);
		inPRINT_Buffer_PutIn(szBuf, _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		memset(uszBitMapDisp, 0x00, sizeof(uszBitMapDisp));
		memset(uszBitMap, 0x00, sizeof(uszBitMap));
		memcpy(uszBitMap, (char *)&uszDebugBuf[inCnt], 8);

		for (inBitMapCnt1 = 0; inBitMapCnt1 < 8; inBitMapCnt1 ++)
		{
			for (inBitMapCnt2 = 0; inBitMapCnt2 < 8; inBitMapCnt2 ++)
			{
				if (uszBitMap[inBitMapCnt1] & 0x80)
					uszBitMapDisp[(inBitMapCnt1*8) + inBitMapCnt2 + 1] = '1';
				else
					uszBitMapDisp[(inBitMapCnt1*8) + inBitMapCnt2 + 1] = '0';

				uszBitMap[inBitMapCnt1] = uszBitMap[inBitMapCnt1] << 1;
		    }
		}

		inPRINT_Buffer_PutIn("  1234567890   1234567890   1234567890", _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		memset(szBuf, 0x00, sizeof(szBuf));
		sprintf(szBuf, "0>%10.10s 1>%10.10s 2>%10.10s", &uszBitMapDisp[1], &uszBitMapDisp[11], &uszBitMapDisp[21]);
		inPRINT_Buffer_PutIn(szBuf, _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		sprintf(szBuf, "3>%10.10s 4>%10.10s 5>%10.10s", &uszBitMapDisp[31], &uszBitMapDisp[41], &uszBitMapDisp[51]);
		inPRINT_Buffer_PutIn(szBuf, _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		sprintf(szBuf, "6>%4.4s", &uszBitMapDisp[61]);
		inPRINT_Buffer_PutIn(szBuf, _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		inCnt += 8;

		for (i = 1; i <= 64; i ++)
		{
			if (!inNCCC_TICKET_BitMapCheck((unsigned char *)szBitMap, i))
				continue;

			inLen = 0;
			inFieldLen = 0;
			memset(szBuf, 0x00, sizeof(szBuf));
			sprintf(szBuf, "F_%02d ", i);
			inLen = 5;
			strcat(&szBuf[inLen ++], "[");

			switch (i)
			{
				case 3 :
					strcat(szBuf, "P_CODE ");
					inLen += 7;
					break;
				case 4 :
					strcat(szBuf, "AMT ");
					inLen += 4;
					break;
				case 11 :
					strcat(szBuf, "STAN ");
					inLen += 5;
					break;
				case 12 :
					strcat(szBuf, "TIME ");
					inLen += 5;
					break;
				case 13 :
					strcat(szBuf, "DATE ");
					inLen += 5;
					break;
				case 24 :
					strcat(szBuf, "NII ");
					inLen += 4;
					break;
				case 32 :
					strcat(szBuf, "AIIC ");
					inLen += 5;
					break;
				case 35 :
					break;
				case 37 :
					strcat(szBuf, "RRN ");
					inLen += 4;
					break;
				case 38 :
					strcat(szBuf, "AUTH_ID ");
					inLen += 8;
					break;
				case 39 :
					strcat(szBuf, "RESP_CODE ");
					inLen += 10;
					break;
				case 41 :
					strcat(szBuf, "TID ");
					inLen += 4;
					break;
				case 42 :
					strcat(szBuf, "MID ");
					inLen += 4;
					break;
				case 48 :
					strcat(szBuf, "STORE_ID ");
					inLen += 9;
					break;
				case 52 :
					strcat(szBuf, "PIN_BLOCK ");
					inLen += 10;
					break;
				case 54 :
					strcat(szBuf, "TIP_AMT ");
					inLen += 8;
					break;
				case 55 :
					strcat(szBuf, "EMV ");
					inLen += 4;
					break;
				case 56 :
					strcat(szBuf, "TICKET ");
					inLen += 7;
					break;
				case 57 :
					strcat(szBuf, "CSUM ");
					inLen += 5;
					break;
				case 58 :
					strcat(szBuf, "TABLE ");
					inLen += 6;
					break;
				case 59 :
					strcat(szBuf, "TABLE ");
					inLen += 6;
					break;
				case 64 :
					strcat(szBuf, "MAC ");
					inLen += 4;
					break;
				default :
					break;
			}       

			inField = inNCCC_TICKET_GetFieldIndex(i, srISOTypeTable.srISOFieldType);
			inFieldLen = inNCCC_TICKET_GetFieldLen(i, &uszDebugBuf[inCnt], srISOTypeTable.srISOFieldType, pobTran->srTRec.inTicketType);

			switch (srISOTypeTable.srISOFieldType[inField].inFieldType)
			{
				case _NCCC_TICKET_ISO_ASC_  :
					memcpy(&szBuf[inLen], &uszDebugBuf[inCnt], inFieldLen);
					inLen += inFieldLen;
					break;
				case _NCCC_TICKET_ISO_BYTE_3_  :
					inFunc_BCD_to_ASCII(&szBuf[inLen], &uszDebugBuf[inCnt], 2);
					inCnt += 2;
					inLen += 4;
					strcat(szBuf, " ");
					inLen ++;
					inFieldLen --;
					inFieldLen --;

					if (srISOTypeTable.srISOFieldType[inField].uszDispAscii == VS_TRUE)
					{
						if (i != 58 && i != 59)
						{
							memcpy(&szBuf[inLen], (char *)&uszDebugBuf[inCnt], inFieldLen);
							inLen += inFieldLen;
						}
					}
					else
					{
						if (i != 55 && i != 56)
						{
							inFunc_BCD_to_ASCII(&szBuf[inLen], &uszDebugBuf[inCnt], inFieldLen);
							inLen += inFieldLen * 2;
						}
					}

					break;
				case _NCCC_TICKET_ISO_BYTE_1_ :
					inFunc_BCD_to_ASCII(&szBuf[inLen], &uszDebugBuf[inCnt ++], 1);
					inLen ++;
					strcat(szBuf, " ");
					inLen ++;

					inFieldLen --;
					inFunc_BCD_to_ASCII(&szBuf[inLen], &uszDebugBuf[inCnt], inFieldLen);
					inLen += inFieldLen;
					break;
				case _NCCC_TICKET_ISO_NIBBLE_2_  :
				case _NCCC_TICKET_ISO_BYTE_2_  :
					inFunc_BCD_to_ASCII(&szBuf[inLen], &uszDebugBuf[inCnt ++], 1);
					inLen += 2;
					strcat(szBuf, " ");
					inLen ++;

					inFieldLen --;
					if (srISOTypeTable.srISOFieldType[inField].uszDispAscii == VS_TRUE)
					{
						memcpy(&szBuf[inLen], (char *)&uszDebugBuf[inCnt], inFieldLen);
						inLen += inFieldLen;
					}
					else
					{
						inFunc_BCD_to_ASCII(&szBuf[inLen], &uszDebugBuf[inCnt], inFieldLen);
						inLen += inFieldLen * 2;
					}

					break;
				case _NCCC_TICKET_ISO_BCD_  :
					if (srISOTypeTable.srISOFieldType[inField].uszDispAscii == VS_TRUE)
					{
						memcpy(&szBuf[inLen], (char *)&uszDebugBuf[inCnt], inFieldLen);
						inLen += inFieldLen;
					}
					else
					{
						inFunc_BCD_to_ASCII(&szBuf[inLen], &uszDebugBuf[inCnt], inFieldLen);
						inLen += inFieldLen * 2;
					}

					break;
				case _NCCC_TICKET_ETICKET_CARD_:
					if (srISOTypeTable.srISOFieldType[inField].uszDispAscii == VS_TRUE)
					{
						memcpy(&szBuf[inLen], (char *)&uszDebugBuf[inCnt], inFieldLen);
						inLen += inFieldLen;
					}
					else
					{
						inFunc_BCD_to_ASCII(&szBuf[inLen], &uszDebugBuf[inCnt], inFieldLen);
						inLen += inFieldLen * 2;
					}

					break;
				default :
					break;
			}
			
			/* 模擬ESVC advice回19的情形 */
			if ((ginSpecialSituation & _SPECIAL_SITUATION_ESVC_ADVICE_RESPONSE_19_) == _SPECIAL_SITUATION_ESVC_ADVICE_RESPONSE_19_	&&
			     memcmp(&uszDebugBuf[5], "\x02\x30", 2) == 0									&&
			     i == 39)
			{
				memcpy(&szBuf[inLen - 2], "19", 2);
			}

			strcat(&szBuf[inLen ++], "]");

			inPrintLineCnt = 0;
			while ((inPrintLineCnt * inOneLineLen) < strlen(szBuf))
			{
				memset(szPrintLineData, 0x00, sizeof(szPrintLineData));
				memset(szPrtBuf, 0x00, sizeof(szPrtBuf));
				if (((inPrintLineCnt + 1) * inOneLineLen) > strlen(szBuf))
				{
					strcat(szPrintLineData, &szBuf[inPrintLineCnt * inOneLineLen]);
				}
				else
				{
					memcpy(szPrintLineData, &szBuf[inPrintLineCnt * inOneLineLen], inOneLineLen);
				}

				sprintf(szPrtBuf, "  %s", szPrintLineData);

				inPRINT_Buffer_PutIn(szPrtBuf, _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPrintLineCnt ++;
			};

			if (i == 56)
				vdNCCC_TICKET_ISO_FormatDebug_PRINT_56(&uszDebugBuf[inCnt], inFieldLen, uszBuffer, &srFont_Attrib, &srBhandle);
			else if (i == 59)
				vdNCCC_TICKET_ISO_FormatDebug_PRINT_59(&uszDebugBuf[inCnt], inFieldLen, uszBuffer, &srFont_Attrib, &srBhandle);

			inCnt += inFieldLen;
		}

		inPRINT_Buffer_PutIn("==========================================", _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		inPRINT_Buffer_OutPut(uszBuffer, &srBhandle);
	}
}

void vdNCCC_TICKET_ISO_FormatDebug_PRINT_56(unsigned char *uszDebugBuf, int inFieldLen, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inCnt = 0, inTableLen = 0;
	int	inPrintLineCnt = 0, inOneLineLen = 34;
	char	szTagData[38 + 1];
	char	szPrintBuf[5120 + 1], szTemplate[5120 + 1];
	char	szTemp[50 + 1];
	char	szI_FESMode[1 + 1] = {0};
	
	memset(szI_FESMode, 0x00, sizeof(szI_FESMode));
	inGetI_FES_Mode(szI_FESMode);

	while (inFieldLen > inCnt)
	{
		if (!memcmp(&uszDebugBuf[inCnt], "IP00", 4) ||
		    !memcmp(&uszDebugBuf[inCnt], "IP01", 4) ||
		    !memcmp(&uszDebugBuf[inCnt], "IP02", 4))
		{
			/* Table ID */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 4);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, " Table ID   [%s]", szTemplate);
                        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 4;
			/* Table Length */
			inTableLen = ((uszDebugBuf[inCnt] / 16 * 10 + uszDebugBuf[inCnt] % 16) * 100);
			inTableLen += (uszDebugBuf[inCnt + 1] / 16 * 10) + uszDebugBuf[inCnt + 1] % 16;
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, " Table Len  [%02X %02X][%d]", uszDebugBuf[inCnt], uszDebugBuf[inCnt + 1], inTableLen);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 2;
			
			/* Table Data */
			if (!memcmp(&szTemplate[0], "IP01", 4))
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inFunc_BCD_to_ASCII(&szTemplate[0], &uszDebugBuf[inCnt], inTableLen);
				
				if (inTableLen <= 8)
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, " Table Data [%s]", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				}
				else
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, " Table Data ");
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

					inPrintLineCnt = 0;
					while ((inPrintLineCnt * inOneLineLen) < inTableLen)
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						memset(szTagData, 0x00, sizeof(szTagData));
						if (((inPrintLineCnt + 1) * inOneLineLen) > inTableLen)
						{
							memcpy(szTagData, &szTemplate[inPrintLineCnt * inOneLineLen], (inTableLen - (inPrintLineCnt + 1) * inOneLineLen));
						}
						else
						{
							memcpy(szTagData, &szTemplate[inPrintLineCnt * inOneLineLen], inOneLineLen);
						}
						sprintf(szPrintBuf, "  [%s]", szTagData);
						inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						inPrintLineCnt ++;
					}

				}
			}
			else if (!memcmp(&szTemplate[0], "IP02", 4))
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], inTableLen);
				
				if (inTableLen <= 8)
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, " Table Data [%s]", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				}
				else
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, " Table Data ");
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

					inPrintLineCnt = 0;
					while ((inPrintLineCnt * inOneLineLen) < inTableLen)
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						memset(szTagData, 0x00, sizeof(szTagData));
						if (((inPrintLineCnt + 1) * inOneLineLen) > inTableLen)
						{
							memset(szTemp, 0x00, sizeof(szTemp));
							strcat(szTemp, &szTemplate[inPrintLineCnt * inOneLineLen]);
							memcpy(szTagData, szTemp, strlen(szTemp) - 2);
							sprintf(szPrintBuf, "  [%s]", szTagData);
							inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
							
							memset(szTagData, 0x00, sizeof(szTagData));
							sprintf(szTagData, "\"%02X\"\"%02X\"", szTemp[strlen(szTemp) - 2], szTemp[strlen(szTemp) - 1]);
							sprintf(szPrintBuf, "  [%s]", szTagData);
							inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
							
							inPrintLineCnt ++;
						}
						else
						{
							memcpy(szTagData, &szTemplate[inPrintLineCnt * inOneLineLen], inOneLineLen);
							sprintf(szPrintBuf, "  [%s]", szTagData);
							inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
							inPrintLineCnt ++;
						}
					}

				}
			}
			else
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], inTableLen);
				
				if (inTableLen <= 8)
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, " Table Data [%s]", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				}
				else
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, " Table Data ");
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

					inPrintLineCnt = 0;
					while ((inPrintLineCnt * inOneLineLen) < inTableLen)
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						memset(szTagData, 0x00, sizeof(szTagData));
						if (((inPrintLineCnt + 1) * inOneLineLen) > inTableLen)
						{
							memcpy(szTagData, &szTemplate[inPrintLineCnt * inOneLineLen], (inTableLen - (inPrintLineCnt + 1) * inOneLineLen));
						}
						else
						{
							memcpy(szTagData, &szTemplate[inPrintLineCnt * inOneLineLen], inOneLineLen);
						}
						sprintf(szPrintBuf, "  [%s]", szTagData);
						inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						inPrintLineCnt ++;
					}

				}
			}
					
			inCnt += inTableLen;
                        inPRINT_Buffer_PutIn(" -----------------------------------------", _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		else if (!memcmp(&uszDebugBuf[inCnt], "EZ01", 4))
		{
			/* Table ID */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 4);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, " Table ID   [%s]", szTemplate);
                        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 4;
			/* Table Length */
			inTableLen = ((uszDebugBuf[inCnt] / 16 * 10 + uszDebugBuf[inCnt] % 16) * 100);
			inTableLen += (uszDebugBuf[inCnt + 1] / 16 * 10) + uszDebugBuf[inCnt + 1] % 16;
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, " Table Len  [%02X %02X][%d]", uszDebugBuf[inCnt], uszDebugBuf[inCnt + 1], inTableLen);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 2;
			if (inTableLen <= 4)
			{
				/* Table Data */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], inTableLen);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, " Table Data [%s]", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			else
			{
				/* Table Data */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], inTableLen);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, " Table Data ");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				
				inPrintLineCnt = 0;
				inOneLineLen = 42;
				
				while ((inPrintLineCnt * inOneLineLen) < inTableLen)
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					memset(szTagData, 0x00, sizeof(szTagData));
					if (((inPrintLineCnt + 1) * inOneLineLen) > inTableLen)
					{
						memcpy(szTagData, &szTemplate[inPrintLineCnt * inOneLineLen], (inTableLen - (inPrintLineCnt + 1) * inOneLineLen));
					}
					else
					{
						memcpy(szTagData, &szTemplate[inPrintLineCnt * inOneLineLen], inOneLineLen);
					}
					sprintf(szPrintBuf, "  [%s]", szTagData);
//					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					inPrintLineCnt ++;
				}
				
			}
			
			inCnt += inTableLen;
                        inPRINT_Buffer_PutIn(" -----------------------------------------", _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		else if (!memcmp(&uszDebugBuf[inCnt], "IC01", 4) ||
		         !memcmp(&uszDebugBuf[inCnt], "IC02", 4))
		{
			/* Table ID */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 4);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, " Table ID   [%s]", szTemplate);
                        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 4;
			/* Table Length */
			inTableLen = ((uszDebugBuf[inCnt] / 16 * 10 + uszDebugBuf[inCnt] % 16) * 100);
			inTableLen += (uszDebugBuf[inCnt + 1] / 16 * 10) + uszDebugBuf[inCnt + 1] % 16;
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, " Table Len  [%02X %02X][%d]", uszDebugBuf[inCnt], uszDebugBuf[inCnt + 1], inTableLen);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 2;
			
			/* Table Data */
			if (!memcmp(&szTemplate[0], "IC01", 4))
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inFunc_BCD_to_ASCII(&szTemplate[0], &uszDebugBuf[inCnt], inTableLen);
				
				if (inTableLen <= 8)
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, " Table Data [%s]", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				}
				else
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, " Table Data ");
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

					inPrintLineCnt = 0;
					while ((inPrintLineCnt * inOneLineLen) < inTableLen)
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						memset(szTagData, 0x00, sizeof(szTagData));
						if (((inPrintLineCnt + 1) * inOneLineLen) > inTableLen)
						{
							memcpy(szTagData, &szTemplate[inPrintLineCnt * inOneLineLen], (inTableLen - (inPrintLineCnt + 1) * inOneLineLen));
						}
						else
						{
							memcpy(szTagData, &szTemplate[inPrintLineCnt * inOneLineLen], inOneLineLen);
						}
						sprintf(szPrintBuf, "  [%s]", szTagData);
						inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						inPrintLineCnt ++;
					}

				}
			}
			else if (!memcmp(&szTemplate[0], "IC02", 4))
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], inTableLen);
				
				if (inTableLen <= 8)
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, " Table Data [%s]", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				}
				else
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, " Table Data ");
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

					inPrintLineCnt = 0;
					while ((inPrintLineCnt * inOneLineLen) < inTableLen)
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						memset(szTagData, 0x00, sizeof(szTagData));
						if (((inPrintLineCnt + 1) * inOneLineLen) > inTableLen)
						{
							memset(szTemp, 0x00, sizeof(szTemp));
							strcat(szTemp, &szTemplate[inPrintLineCnt * inOneLineLen]);
							memcpy(szTagData, szTemp, strlen(szTemp) - 2);
							sprintf(szPrintBuf, "  [%s]", szTagData);
							inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
							
							memset(szTagData, 0x00, sizeof(szTagData));
							sprintf(szTagData, "\"%02X\"\"%02X\"", szTemp[strlen(szTemp) - 2], szTemp[strlen(szTemp) - 1]);
							sprintf(szPrintBuf, "  [%s]", szTagData);
							inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
							
							inPrintLineCnt ++;
						}
						else
						{
							memcpy(szTagData, &szTemplate[inPrintLineCnt * inOneLineLen], inOneLineLen);
							sprintf(szPrintBuf, "  [%s]", szTagData);
							inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
							inPrintLineCnt ++;
						}
					}

				}
			}
			else
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], inTableLen);
				
				if (inTableLen <= 8)
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, " Table Data [%s]", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				}
				else
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, " Table Data ");
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

					inPrintLineCnt = 0;
					while ((inPrintLineCnt * inOneLineLen) < inTableLen)
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						memset(szTagData, 0x00, sizeof(szTagData));
						if (((inPrintLineCnt + 1) * inOneLineLen) > inTableLen)
						{
							memcpy(szTagData, &szTemplate[inPrintLineCnt * inOneLineLen], (inTableLen - (inPrintLineCnt + 1) * inOneLineLen));
						}
						else
						{
							memcpy(szTagData, &szTemplate[inPrintLineCnt * inOneLineLen], inOneLineLen);
						}
						sprintf(szPrintBuf, "  [%s]", szTagData);
						inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						inPrintLineCnt ++;
					}

				}
			}
					
			inCnt += inTableLen;
                        inPRINT_Buffer_PutIn(" -----------------------------------------", _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		else
		{
			/* IFES會tSAM加密 */
			if (memcmp(szI_FESMode, "Y", 1)  == 0)
			{
				/* Table Data */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inFunc_BCD_to_ASCII(&szTemplate[0], &uszDebugBuf[inCnt], inFieldLen);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, " Table Data ");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				inPrintLineCnt = 0;
				inOneLineLen = 42;

				while ((inPrintLineCnt * inOneLineLen) < inTableLen)
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					memset(szTagData, 0x00, sizeof(szTagData));
					if (((inPrintLineCnt + 1) * inOneLineLen) > inTableLen)
					{
						memcpy(szTagData, &szTemplate[inPrintLineCnt * inOneLineLen], (inTableLen - (inPrintLineCnt + 1) * inOneLineLen));
					}
					else
					{
						memcpy(szTagData, &szTemplate[inPrintLineCnt * inOneLineLen], inOneLineLen);
					}
					sprintf(szPrintBuf, "  [%s]", szTagData);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					inPrintLineCnt ++;
				}

				inCnt += inFieldLen;
				inPRINT_Buffer_PutIn(" -----------------------------------------", _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			else
			{
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, " F_56 OTHER DATA [%02X]", uszDebugBuf[inCnt]);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt ++;
			}
		}
		
	}
	
}

void vdNCCC_TICKET_ISO_FormatDebug_PRINT_59(unsigned char *uszDebugBuf, int inFieldLen, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inCnt = 0, inTableLen;
	int	inPrintLineCnt = 0, inOneLineLen = 34;
	char	szTagData[38 + 1];
	char	szPrintBuf[1024 + 1], szTemplate[1024 + 1], szBig5[1024 + 1] = {0};

	while (inFieldLen > inCnt)
	{
		if (!memcmp(&uszDebugBuf[inCnt], "N1", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "N2", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "N3", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "N6", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "NA", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "ND", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "NF", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "ET", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "ES", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "ED", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "PS", 2))
		{
			/* Table ID */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 2);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, " Table ID   [%s]", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 2;
			/* Table Length */
			inTableLen = ((uszDebugBuf[inCnt] / 16 * 10 + uszDebugBuf[inCnt] % 16) * 100);
			inTableLen += (uszDebugBuf[inCnt + 1] / 16 * 10) + uszDebugBuf[inCnt + 1] % 16;
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, " Table Len  [%02X %02X][%d]", uszDebugBuf[inCnt], uszDebugBuf[inCnt + 1], inTableLen);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 2;
			if (inTableLen <= 8)
			{
				/* Table Data */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], inTableLen);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, " Table Data [%s]", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			else
			{
				/* Table Data */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], inTableLen);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, " Table Data ");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				
				inPrintLineCnt = 0;
				while ((inPrintLineCnt * inOneLineLen) < inTableLen)
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					memset(szTagData, 0x00, sizeof(szTagData));
					if (((inPrintLineCnt + 1) * inOneLineLen) > inTableLen)
					{
						memcpy(szTagData, &szTemplate[inPrintLineCnt * inOneLineLen], (inTableLen - (inPrintLineCnt + 1) * inOneLineLen));
					}
					else
					{
						memcpy(szTagData, &szTemplate[inPrintLineCnt * inOneLineLen], inOneLineLen);
					}
					sprintf(szPrintBuf, "  [%s]", szTagData);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					inPrintLineCnt ++;
				}
				
			}

			inCnt += inTableLen;
                        inPRINT_Buffer_PutIn(" -----------------------------------------", _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		else if (!memcmp(&uszDebugBuf[inCnt], "L1", 2) || !memcmp(&uszDebugBuf[inCnt], "L5", 2))
		{
			/* Table ID */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 2);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, " Table ID   [%s]", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 2;
			/* Table Length */
			inTableLen = ((uszDebugBuf[inCnt] / 16 * 1000 + uszDebugBuf[inCnt] % 16) * 100);
			inTableLen += (uszDebugBuf[inCnt + 1] / 16 * 10) + uszDebugBuf[inCnt + 1] % 16;
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, " Table Len  [%02X %02X][%d]", uszDebugBuf[inCnt], uszDebugBuf[inCnt + 1], inTableLen);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 2;
			/* 列印優惠或廣告資訊之個數 (1 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			szTemplate[0] = uszDebugBuf[inCnt];
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  列印優惠個數 = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt ++;
			/* 優惠活動SN (22 Bytes) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 22);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  優惠活動SN = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 22;

			if (inTableLen == 23)
				continue;

			/* 是否列印補充資訊 (1 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  是否列印補充資訊 = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt ++;
			/* 補充資訊長度 (2 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 2);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  補充資訊長度 = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 2;
			/* 列印補充資訊位置 (1 Byte) */ /* 2014/1/22 下午 03:26:24 add by kakab NCCC優惠平台端末機連線規格_V1_15_20140109 新增參數列印位置 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  列印補充資訊位置 = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt ++;
			/* 補充資訊內容 (60 Byte) */ /* 2014/1/22 下午 03:26:24 add by kakab NCCC優惠平台端末機連線規格_V1_15_20140109 增大為60Bytes */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szBig5, 0x00, sizeof(szBig5));
			memcpy(szBig5, (char*)&uszDebugBuf[inCnt], 60);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			inFunc_Big5toUTF8(szTemplate, szBig5);
			sprintf(szPrintBuf, "  補充資訊內容 = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 60;

			if (inTableLen == 87)
				continue;

			/* 是否列印優惠活動之標題 (1 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  是否列印優惠活動之標題 = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt ++;
			/* 優惠活動之標題長度 (2 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 2);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  優惠活動之標題長度 = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 2;
			/* 優惠活動之標題內容 (50 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szBig5, 0x00, sizeof(szBig5));
			memcpy(szBig5, (char*)&uszDebugBuf[inCnt], 50);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			inFunc_Big5toUTF8(szTemplate, szBig5);
			sprintf(szPrintBuf, "  優惠活動之標題內容 = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 50;
			/* 是否列印一維條碼(一) (1 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  是否列印一維條碼(一) = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt ++;
			/* 一維條碼(一)長度 (2 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 2);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  一維條碼(一)長度 = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 2;
			/* 一維條碼(一) (20 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 20);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  一維條碼(一) = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 20;
			/* 是否列印一維條碼(二) (1 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  是否列印一維條碼(二) = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt ++;
			/* 一維條碼(二)長度 (2 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 2);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  一維條碼(二)長度 = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 2;
			/* 一維條碼(二) (20 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 20);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  一維條碼(二) = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 20;
			/* 保留欄位(預設空白) */
			inCnt ++;
			/* 是否列印優惠活動之內容 (1 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  是否列印優惠活動之內容 = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt ++;
			/* 優惠活動之內容長度 (3 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 3);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  優惠活動之內容長度 = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 3;
			/* 優惠活動之內容 (200 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szBig5, 0x00, sizeof(szBig5));
			memcpy(szBig5, (char*)&uszDebugBuf[inCnt], 200);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			inFunc_Big5toUTF8(szTemplate, szBig5);
			sprintf(szPrintBuf, "  優惠活動之內容 = ");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szTemplate, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 200;
			/* 檢查碼(Checksum) (1 Byte) */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  檢查碼 = %02x", uszDebugBuf[inCnt]);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt ++;

			inPRINT_Buffer_PutIn(" -----------------------------------------", _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			
			if (inTableLen == 392)
				continue;
			
			/* 是否列印優惠活動之標題 (1 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  是否列印優惠活動之標題 = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt ++;
			/* 優惠活動之標題長度 (2 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 2);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  優惠活動之標題長度 = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 2;
			/* 優惠活動之標題內容 (50 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szBig5, 0x00, sizeof(szBig5));
			memcpy(szBig5, (char*)&uszDebugBuf[inCnt], 50);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			inFunc_Big5toUTF8(szTemplate, szBig5);
			sprintf(szPrintBuf, "  優惠活動之標題內容 = ");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szTemplate, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 50;
			/* 是否列印一維條碼(一) (1 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  是否列印一維條碼(一) = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt ++;
			/* 一維條碼(一)長度 (2 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 2);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  一維條碼(一)長度 = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 2;
			/* 一維條碼(一) (20 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 20);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  一維條碼(一) = ");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szTemplate, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 20;
			/* 是否列印一維條碼(二) (1 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  是否列印一維條碼(二) = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt ++;
			/* 一維條碼(二)長度 (2 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 2);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  一維條碼(二)長度 = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 2;
			/* 一維條碼(二) (20 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 20);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  一維條碼(二) = ");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szTemplate, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 20;
			/* 保留欄位(預設空白) */
			inCnt ++;
			/* 是否列印優惠活動之內容 (1 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  是否列印優惠活動之內容 = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt ++;
			/* 優惠活動之內容長度 (3 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 3);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  優惠活動之內容長度 = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 3;
			/* 優惠活動之內容 (200 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szBig5, 0x00, sizeof(szBig5));
			memcpy(szBig5, (char*)&uszDebugBuf[inCnt], 200);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			inFunc_Big5toUTF8(szTemplate, szBig5);
			sprintf(szPrintBuf, "  優惠活動之內容 = ");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szTemplate, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 200;
			/* 檢查碼(Checksum) (1 Byte) */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  檢查碼 = %02x", uszDebugBuf[inCnt]);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt ++;
			inPRINT_Buffer_PutIn(" -----------------------------------------", _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		else if (!memcmp(&uszDebugBuf[inCnt], "L2", 2))
		{
			/* Table ID */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 2);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, " Table ID   [%s]", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 2;
			/* Table Length */
			inTableLen = ((uszDebugBuf[inCnt] / 16 * 1000 + uszDebugBuf[inCnt] % 16) * 100);
			inTableLen += (uszDebugBuf[inCnt + 1] / 16 * 10) + uszDebugBuf[inCnt + 1] % 16;
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, " Table Len  [%02X %02X][%d]", uszDebugBuf[inCnt], uszDebugBuf[inCnt + 1], inTableLen);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 2;
			/* 列印優惠或廣告資訊之個數 (1 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			szTemplate[0] = uszDebugBuf[inCnt];
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  列印優惠個數 = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt ++;
			/* 優惠活動SN (22 Bytes) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 22);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  優惠活動SN = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 22;

			if (inTableLen == 23)
				continue;

			/* 是否列印補充資訊 (1 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  是否列印補充資訊 = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt ++;
			/* 補充資訊長度 (2 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 2);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  補充資訊長度 = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 2;
			/* 列印補充資訊位置 (1 Byte) */  /* 2014/1/22 下午 03:26:24 add by kakab NCCC優惠平台端末機連線規格_V1_15_20140109 新增參數列印位置 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  列印補充資訊位置 = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt ++;
			/* 補充資訊內容 (60 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szBig5, 0x00, sizeof(szBig5));
			memcpy(szBig5, (char*)&uszDebugBuf[inCnt], 60);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			inFunc_Big5toUTF8(szTemplate, szBig5);
			sprintf(szPrintBuf, "  補充資訊內容 = ");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szTemplate, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 60;

			if (inTableLen == 87)
				continue;

			/* 是否列印(優惠活動||累計訊息)之標題 (1 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  是否列印優惠or訊息標題 = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt ++;
			/* (優惠活動||累計訊息)之標題長度。 (2 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 2);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  優惠or訊息標題長度 = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 2;
			/* (優惠活動||累計訊息)之標題內容。(端末機列印時需自動置中) (50 Bytes) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szBig5, 0x00, sizeof(szBig5));
			memcpy(szBig5, (char*)&uszDebugBuf[inCnt], 50);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			inFunc_Big5toUTF8(szTemplate, szBig5);
			sprintf(szPrintBuf, "  優惠or訊息內容 = ");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szTemplate, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 50;
			/* 是否列印一維條碼(一) (1 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  是否列印一維條碼(一) = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt ++;
			/* 一維條碼(一)長度 (2 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 2);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  一維條碼(一)長度 = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 2;
			/* 一維條碼(一) (20 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 20);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  一維條碼(一) = ");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szTemplate, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 20;
			/* 是否列印一維條碼(二) (1 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  是否列印一維條碼(二) = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt ++;
			/* 一維條碼(二)長度 (2 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 2);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  一維條碼(二)長度 = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 2;
			/* 一維條碼(二) (20 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 20);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  一維條碼(二) = ");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szTemplate, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 20;
			/* 保留欄位(預設空白) */
			inCnt ++;
			/* 是否列印(優惠活動||累計訊息)之內容。 (1 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  是否列印優惠or訊息)之內容 = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt ++;
			/* (優惠活動||累計訊息)之內容長度 (3 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 3);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  優惠or訊息內容長度 = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 3;
			/* (優惠活動||累計訊息)之內容 (200 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szBig5, 0x00, sizeof(szBig5));
			memcpy(szBig5, (char*)&uszDebugBuf[inCnt], 200);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			inFunc_Big5toUTF8(szTemplate, szBig5);
			sprintf(szPrintBuf, "  優惠or訊息內容 = ");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szTemplate, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 200;
			/* 檢查碼(Checksum) (1 Byte) */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  檢查碼 = %02x", uszDebugBuf[inCnt]);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt ++;
			/* 廣告資訊 */
			/* 是否列印廣告資訊之標題。 (1 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  是否列印廣告標題 = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt ++;
			/* 廣告資訊之標題長度。 (2 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 2);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  廣告標題長度 = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 2;
			/* 廣告資訊之標題內容。(端末機列印時需自動置中) (50 Bytes) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szBig5, 0x00, sizeof(szBig5));
			memcpy(szBig5, (char*)&uszDebugBuf[inCnt], 50);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			inFunc_Big5toUTF8(szTemplate, szBig5);
			sprintf(szPrintBuf, "  廣告標題內容 = ");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szTemplate, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 50;
			/* 是否列印廣告資訊。 (1 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  是否列印廣告資訊 = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt ++;
			/* 廣告資訊長度。 (3 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 3);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  廣告資訊長度 = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 3;
			/* 廣告資訊之內容 (150 Bytes) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szBig5, 0x00, sizeof(szBig5));
			memcpy(szBig5, (char*)&uszDebugBuf[inCnt], 150);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			inFunc_Big5toUTF8(szTemplate, szBig5);
			sprintf(szPrintBuf, "  廣告資訊內容 = ");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szTemplate, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 150;
			/* 是否列印URL資訊之QR Code。 (1 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  是否列印QRCode = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt ++;
			/* URL資訊長度。 (3 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 3);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  QRCode長度 = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 3;
			/* URL資訊之內容 (100 Bytes) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 100);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  QRcode內容 = ");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szTemplate, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 100;
			/* 檢查碼(Checksum) (1 Byte) */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  檢查碼 = %02x", uszDebugBuf[inCnt]);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt ++;
			inPRINT_Buffer_PutIn(" -----------------------------------------", _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		else if (!memcmp(&uszDebugBuf[inCnt], "L3", 2))
		{
			/* Table ID */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 2);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, " Table ID   [%s]", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 2;
			/* Table Length */
			inTableLen = ((uszDebugBuf[inCnt] / 16 * 1000 + uszDebugBuf[inCnt] % 16) * 100);
			inTableLen += (uszDebugBuf[inCnt + 1] / 16 * 10) + uszDebugBuf[inCnt + 1] % 16;
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, " Table Len  [%02X %02X][%d]", uszDebugBuf[inCnt], uszDebugBuf[inCnt + 1], inTableLen);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 2;
			/* 兌換方式 (1 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  兌換方式  = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt ++;
			/* 優惠活動SN (22 Bytes) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 22);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  優惠活動SN = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 22;
			if (inTableLen == 23)
				continue;

			/* 是否列印補充資訊 (1 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  是否列印補充資訊 = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt ++;
			/* 補充資訊長度 (2 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 2);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  補充資訊長度 = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 2;
			/* 補充資訊內容 (20 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szBig5, 0x00, sizeof(szBig5));
			memcpy(szBig5, (char*)&uszDebugBuf[inCnt], 20);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			inFunc_Big5toUTF8(szTemplate, szBig5);
			sprintf(szPrintBuf, "  補充資訊內容 = ");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szTemplate, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 20;

			if (inTableLen == 46)
				continue;

			/* 是否上傳一維條碼(一) (1 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  是否上傳一維條碼(一) = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt ++;
			/* 一維條碼(一)長度 (2 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 2);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  一維條碼(一)長度 = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 2;
			/* 一維條碼(一) (20 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 20);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  一維條碼(一) = ");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szTemplate, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 20;
			/* 是否上傳一維條碼(二) (1 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  是否上傳一維條碼(二) = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt ++;
			/* 一維條碼(二)長度 (2 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 2);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  一維條碼(二)長度 = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 2;
			/* 一維條碼(二) (20 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 20);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  一維條碼(二) = ");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szTemplate, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 20;

			if (inTableLen == 92)
				continue;
			/* 第一段核銷資訊 */
			/* 是否列印兌換核銷資訊(一)(1 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  是否列印核銷資訊(一) = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt ++;
			/* 核銷資訊(一)長度 (3 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 3);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  核銷資訊(一)長度 = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 3;
			/* 核銷資訊(一) (100 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szBig5, 0x00, sizeof(szBig5));
			memcpy(szBig5, (char*)&uszDebugBuf[inCnt], 100);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			inFunc_Big5toUTF8(szTemplate, szBig5);
			sprintf(szPrintBuf, "  核銷資訊(一) = ");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szTemplate, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 100;
			/* 第二段核銷資訊 */
			/* 是否列印兌換核銷資訊(二)(1 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  是否列印核銷資訊(二) = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt ++;
			/* 核銷資訊(二)長度 (3 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 3);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  核銷資訊(二)長度 = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 3;
			/* 核銷資訊(二) (100 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szBig5, 0x00, sizeof(szBig5));
			memcpy(szBig5, (char*)&uszDebugBuf[inCnt], 100);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			inFunc_Big5toUTF8(szTemplate, szBig5);
			sprintf(szPrintBuf, "  核銷資訊(二) = ");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szTemplate, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 100;
			/* 檢查碼(Checksum) (1 Byte) */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  檢查碼 = %02x", uszDebugBuf[inCnt]);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt ++;
			inPRINT_Buffer_PutIn(" -----------------------------------------", _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		else
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, " F_59 OTHER DATA [%02X]", uszDebugBuf[inCnt]);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt ++;
		}
		
	}
	
}

/*
Function        :inNCCC_TICKET_Decide_Year
Date&Time       :2019/12/16 上午 11:00
Describe        :
*/
int inNCCC_TICKET_Decide_Year(char* szYear, char* szMonth)
{
	int		inRetVal = VS_SUCCESS;
	char		szTemp[8 + 1] = {0};
	RTC_NEXSYS	srRTC;
	
	/* 原交易日期 - 輸入日期大於目前日期，判定為去年 */ 
	inFunc_GetSystemDateAndTime(&srRTC);

	memset(szTemp, 0x00, sizeof(szTemp));
	memcpy(szTemp, szMonth, 2);

	/* 日期比較 */
	if (atoi(szTemp) > srRTC.uszMonth)
	{
		/* 輸入月份大於現在月份 */
		/* 判定為去年 */
		memset(szTemp, 0x00, sizeof(szTemp));
		sprintf(szTemp, "%04d", (2000 + srRTC.uszYear) - 1);      

		memcpy(szYear, szTemp, 4); 
	}
	else if (atoi(szTemp) == srRTC.uszMonth)
	{
		/* 輸入月份等於現在月份 */
		memset(szTemp, 0x00, sizeof(szTemp));
		memcpy(szTemp, &szMonth[2], 2);

		if (atoi(szTemp) > srRTC.uszDay)
		{
			/* 輸入日期大於現在日期判定為去年 */
			memset(szTemp, 0x00, sizeof(szTemp));
			sprintf(szTemp, "%04d", (2000 + srRTC.uszYear) -1);
		}
		else
		{
			/* 輸入日期小於等於現在日期判定為今年 */
			memset(szTemp, 0x00, sizeof(szTemp));
			sprintf(szTemp, "%04d", (2000 + srRTC.uszYear));
		}

		memcpy(szYear, szTemp, 4);
	} 
	else
	{
		/* 判定為今年 */
		memset(szTemp, 0x00, sizeof(szTemp));
		sprintf(szTemp, "%04d", (2000 + srRTC.uszYear));  

		memcpy(szYear, szTemp, 4);
	}
	
	return (inRetVal);
}
