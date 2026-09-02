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
#include "../SOURCE/FUNCTION/Signpad.h"
#include "../SOURCE/FUNCTION/Utility.h"
#include "../SOURCE/FUNCTION/ECR.h"
#include "../SOURCE/COMM/Comm.h"
#include "../CREDIT/Creditfunc.h"
#include "../CTLS/CTLS.h"
#include "../FISC/NCCCfisc.h"
#include "../EMVSRC/EMVsrc.h"
#include "NCCCsrc.h"
#include "NCCCtSAM.h"
#include "NCCCtmk.h"
#include "NCCCesc.h"
#include "NCCCdcc.h"
#include "NCCCtmsSCT.h"
#include "NCCCtmsCPT.h"
#include "NCCCloyalty.h"
#include "NCCCmfes.h"
#include "TAKAsrc.h"

unsigned char		guszNCCC_MFES_ISO_Field03[_NCCC_MFES_PCODE_SIZE_ + 1];
unsigned char		guszMFES_MTI[4 + 1];
extern	int		ginMacError;
extern	int		ginDebug; /* Debug使用 extern */
extern	int		ginISODebug;
extern	int		ginMachineType;
extern	int		ginAPVersionType;
extern	char		gszTermVersionID[16 + 1];
extern	char		gszMAC_F_03[2 + 1];	/* Field_3:	Processing Code */
extern	char		gszMAC_F_04[12 + 1];	/* Field_4:	Amount, Transaction */
extern	char		gszMAC_F_11[2 + 1];	/* Field_11:	System Trace Audit Number */
extern	char		gszMAC_F_35[40 + 1];	/* Field_35:	TRACK 2 Data */
extern	char		gszMAC_F_59[18 + 1];	/* Field_59:	F _59 之 Table ID “ 電票交易訊 )18 碼。 */
extern	char		gszMAC_F_63[45 + 1];	/* Field_63:	Reserved-Private Data */
extern	unsigned char	guszField_35;
extern	unsigned char	guszField_55;
extern	unsigned char	gusztSAMKeyIndex;
extern	unsigned char	gusztSAMCheckSum_35[4 + 1];
extern	unsigned char	gusztSAMCheckSum_55[4 + 1];
extern	unsigned char	guszEnormousNoNeedResetBit;	/* 資料庫是否須從頭找 */

/* 傳回主機結果用 */
extern EMV_CONFIG	EMVGlobConfig;


ISO_FIELD_NCCC_MFES_TABLE srNCCC_MFES_ISOFieldPack[] =
{
        {3,             inNCCC_MFES_Pack03},
        {4,             inNCCC_MFES_Pack04},
        {11,            inNCCC_MFES_Pack11},
        {12,            inNCCC_MFES_Pack12},
        {13,            inNCCC_MFES_Pack13},
        {22,            inNCCC_MFES_Pack22},
        {24,            inNCCC_MFES_Pack24},
        {25,            inNCCC_MFES_Pack25},
        {35,            inNCCC_MFES_Pack35},
        {37,            inNCCC_MFES_Pack37},
        {38,            inNCCC_MFES_Pack38},
        {41,            inNCCC_MFES_Pack41},
        {42,            inNCCC_MFES_Pack42},
        {48,            inNCCC_MFES_Pack48},
        {52,            inNCCC_MFES_Pack52},
        {54,            inNCCC_MFES_Pack54},
        {55,            inNCCC_MFES_Pack55},
        {56,            inNCCC_MFES_Pack56},
        {59,            inNCCC_MFES_Pack59},
        {60,            inNCCC_MFES_Pack60},
        {62,            inNCCC_MFES_Pack62},
        {63,            inNCCC_MFES_Pack63},
        {64,            inNCCC_MFES_Pack64},
        {0,             NULL},  /* 最後一組一定要放 0 */
};

ISO_FIELD_NCCC_MFES_TABLE srNCCC_MFES_ISOFieldPack_tSAM[] =
{
        {3,             inNCCC_MFES_Pack03},
        {4,             inNCCC_MFES_Pack04},
        {11,            inNCCC_MFES_Pack11},
        {12,            inNCCC_MFES_Pack12},
        {13,            inNCCC_MFES_Pack13},
        {22,            inNCCC_MFES_Pack22},
        {24,            inNCCC_MFES_Pack24},
        {25,            inNCCC_MFES_Pack25},
        {35,            inNCCC_MFES_Pack35_tSAM},
        {37,            inNCCC_MFES_Pack37},
        {38,            inNCCC_MFES_Pack38},
        {41,            inNCCC_MFES_Pack41},
        {42,            inNCCC_MFES_Pack42},
        {48,            inNCCC_MFES_Pack48},
        {52,            inNCCC_MFES_Pack52},
        {54,            inNCCC_MFES_Pack54},
        {55,            inNCCC_MFES_Pack55_tSAM},
	{56,            inNCCC_MFES_Pack56},
        {57,            inNCCC_MFES_Pack57_tSAM},
        {59,            inNCCC_MFES_Pack59},
        {60,            inNCCC_MFES_Pack60},
        {62,            inNCCC_MFES_Pack62},
        {63,            inNCCC_MFES_Pack63},
        {64,            inNCCC_MFES_Pack64},
        {0,             NULL},  /* 最後一組一定要放 0 */
};

ISO_FIELD_NCCC_MFES_TABLE srNCCC_MFES_ISOFieldPack_SoftWare[] =
{
        {3,             inNCCC_MFES_Pack03},
        {4,             inNCCC_MFES_Pack04},
        {11,            inNCCC_MFES_Pack11},
        {12,            inNCCC_MFES_Pack12},
        {13,            inNCCC_MFES_Pack13},
        {22,            inNCCC_MFES_Pack22},
        {24,            inNCCC_MFES_Pack24},
        {25,            inNCCC_MFES_Pack25_Software},
        {35,            inNCCC_MFES_Pack35_Software},
        {37,            inNCCC_MFES_Pack37},
        {38,            inNCCC_MFES_Pack38},
        {41,            inNCCC_MFES_Pack41},
        {42,            inNCCC_MFES_Pack42},
        {48,            inNCCC_MFES_Pack48},
        {52,            inNCCC_MFES_Pack52},
        {54,            inNCCC_MFES_Pack54},
        {55,            inNCCC_MFES_Pack55_Software},
        {56,            inNCCC_MFES_Pack56},
        {59,            inNCCC_MFES_Pack59},
        {60,            inNCCC_MFES_Pack60},
        {62,            inNCCC_MFES_Pack62},
        {63,            inNCCC_MFES_Pack63},
        {64,            inNCCC_MFES_Pack64},
        {0,             NULL},  /* 最後一組一定要放 0 */
};

ISO_FIELD_NCCC_MFES_TABLE srNCCC_MFES_ISOFieldUnPack[] =
{
        {12,            inNCCC_MFES_UnPack12},
        {13,            inNCCC_MFES_UnPack13},
        {24,            inNCCC_MFES_UnPack24},
        {38,            inNCCC_MFES_UnPack38},
        {39,            inNCCC_MFES_UnPack39},
        {55,            inNCCC_MFES_UnPack55},
        {58,            inNCCC_MFES_UnPack58},
        {59,            inNCCC_MFES_UnPack59},
        {60,            inNCCC_MFES_UnPack60},
        {63,            inNCCC_MFES_UnPack63},
        {0,             NULL}, /* 最後一組一定要放【0】!! */
};

ISO_CHECK_NCCC_MFES_TABLE srNCCC_MFES_ISOFieldCheck[] =
{
        {3,             inNCCC_MFES_Check03},
        {4,             inNCCC_MFES_Check04},
        {41,            inNCCC_MFES_Check41},
        {0,		NULL}, /* 最後一組一定要放【0】!! */
};

ISO_FIELD_TYPE_NCCC_MFES_TABLE srNCCC_MFES_ISOFieldType[] =
{
        {3,             _NCCC_MFES_ISO_BCD_,         VS_FALSE,       6},
        {4,             _NCCC_MFES_ISO_BCD_,         VS_FALSE,       12},
        {11,            _NCCC_MFES_ISO_BCD_,         VS_FALSE,       6},
        {12,            _NCCC_MFES_ISO_BCD_,         VS_FALSE,       6},
        {13,            _NCCC_MFES_ISO_BCD_,         VS_FALSE,       4},
        {22,            _NCCC_MFES_ISO_BCD_,         VS_FALSE,       4},
        {24,            _NCCC_MFES_ISO_BCD_,         VS_FALSE,       4},
        {25,            _NCCC_MFES_ISO_BCD_,         VS_FALSE,       2},
        {35,            _NCCC_MFES_ISO_NIBBLE_2_,    VS_FALSE,       0},
        {37,            _NCCC_MFES_ISO_ASC_,         VS_FALSE,       12},
        {38,            _NCCC_MFES_ISO_ASC_,         VS_FALSE,       6},
        {39,            _NCCC_MFES_ISO_ASC_,         VS_FALSE,       2},
        {41,            _NCCC_MFES_ISO_ASC_,         VS_FALSE,       8},
        {42,            _NCCC_MFES_ISO_ASC_,         VS_FALSE,       15},
        {48,            _NCCC_MFES_ISO_BYTE_3_,      VS_TRUE,        0},
        {52,            _NCCC_MFES_ISO_BCD_,         VS_FALSE,       16},
        {54,            _NCCC_MFES_ISO_BYTE_3_,      VS_TRUE,        0},
        {55,            _NCCC_MFES_ISO_BYTE_3_,      VS_FALSE,       0},
	{56,            _NCCC_MFES_ISO_BYTE_3_,      VS_FALSE,       0},
        {57,            _NCCC_MFES_ISO_BYTE_3_,      VS_FALSE,       0},
        {58,            _NCCC_MFES_ISO_BYTE_3_,      VS_TRUE,        0},
        {59,            _NCCC_MFES_ISO_BYTE_3_,      VS_TRUE,        0},
        {60,            _NCCC_MFES_ISO_BYTE_3_,      VS_TRUE,        0},
        {62,            _NCCC_MFES_ISO_BYTE_3_,      VS_TRUE,        0},
        {63,            _NCCC_MFES_ISO_BYTE_3_,      VS_TRUE,        0},
        {64,            _NCCC_MFES_ISO_BCD_,         VS_FALSE,       16},
        {0,             _NCCC_MFES_ISO_BCD_,         VS_FALSE,       0},  /* 最後一組一定要放 0 */
};

/* pre-auth */
int inNCCC_MFES_PRE_AUTH[] = {3, 4, 11, 22, 24, 25, 35, 37, 41, 42, 59, 60, 62, 64, 0}; /* 最後一組一定要放 0!! */
int inNCCC_MFES_CUP_PRE_AUTH[] = {3, 4, 11, 22, 24, 25, 35, 37, 41, 42, 59, 60, 62, 64, 0}; /* 最後一組一定要放 0!! */
int inNCCC_MFES_CUP_PRE_AUTH_VOID[] = {3, 4, 11, 22, 24, 25, 35, 37, 38, 41, 42, 59, 60, 62, 64, 0}; /* 最後一組一定要放 0!! */
/* sale */
int inNCCC_MFES_SALE[] = {3, 4, 11, 22, 24, 25, 35, 37, 41, 42, 59, 60, 62, 64, 0}; /* 最後一組一定要放 0!! */
int inNCCC_MFES_INST_SALE[] = {3, 4, 11, 22, 24, 25, 35, 37, 41, 42, 59, 60, 62, 63, 64, 0}; /* 最後一組一定要放 0!! */
int inNCCC_MFES_REDEEM_SALE[] = {3, 4, 11, 22, 24, 25, 35, 37, 41, 42, 59, 60, 62, 63, 64, 0}; /* 最後一組一定要放 0!! */
int inNCCC_MFES_CUP_SALE[] = {3, 4, 11, 22, 24, 25, 35, 37, 41, 42, 59, 60, 62, 64, 0}; /* 最後一組一定要放 0!! */
int inNCCC_MFES_CUP_INST_SALE[] = {3, 4, 11, 22, 24, 25, 35, 37, 41, 42, 59, 60, 62, 63, 64, 0}; /* 最後一組一定要放 0!! */
int inNCCC_MFES_CUP_REDEEM_SALE[] = {3, 4, 11, 22, 24, 25, 35, 37, 41, 42, 59, 60, 62, 63, 64, 0}; /* 最後一組一定要放 0!! */
/* refund */
int inNCCC_MFES_REFUND[] = {3, 4, 11, 22, 24, 25, 35, 37, 38, 41, 42, 59, 60, 62, 64, 0}; /* 最後一組一定要放 0!! */
int inNCCC_MFES_REDEEM_REFUND[] = {3, 4, 11, 22, 24, 25, 35, 37, 38, 41, 42, 59, 60, 62, 63, 64, 0}; /* 最後一組一定要放 0!! */
int inNCCC_MFES_INST_REFUND[] = {3, 4, 11, 22, 24, 25, 35, 37, 38, 41, 42, 59, 60, 62, 63, 64, 0}; /* 最後一組一定要放 0!! */
int inNCCC_MFES_CUP_REFUND[] = {3, 4, 11, 22, 24, 25, 35, 37, 38, 41, 42, 59, 60, 62, 64, 0}; /* 最後一組一定要放 0!! */
int inNCCC_MFES_CUP_REDEEM_REFUND[] = {3, 4, 11, 22, 24, 25, 35, 37, 38, 41, 42, 59, 60, 62, 63, 64, 0}; /* 最後一組一定要放 0!! */
int inNCCC_MFES_CUP_INST_REFUND[] = {3, 4, 11, 22, 24, 25, 35, 37, 38, 41, 42, 59, 60, 62, 63, 64, 0}; /* 最後一組一定要放 0!! */
/* pre_auth compelete*/
int inNCCC_MFES_PRE_COMP[] = {3, 4, 11, 12, 13, 22, 24, 25, 35, 37, 38, 41, 42, 59, 60, 62, 64, 0}; /* 最後一組一定要放 0!! */
int inNCCC_MFES_CUP_PRE_COMP[] = {3, 4, 11, 12, 13, 22, 24, 25, 35, 37, 38, 41, 42, 59, 60, 62, 64, 0}; /* 最後一組一定要放 0!! */
/* 優惠兌換 */
int inNCCC_MFES_LOYALTY_REDEEM[] = {3, 4, 11, 24, 25, 37, 41, 42, 59, 60, 62, 0}; /* 最後一組一定要放 0!! */
int inNCCC_MFES_VOID_LOYALTY_REDEEM[] = {3, 4, 11, 24, 25, 37, 41, 42, 59, 60, 62, 0}; /* 最後一組一定要放 0!! */
/* adjust (補登強制Online時要一起Online)*/
int inNCCC_MFES_REDEEM_ADJUST[] = {3, 4, 11, 12, 13, 22, 24, 25, 35, 37, 38, 41, 42, 59, 60, 62, 63, 64, 0}; /* 最後一組一定要放 0!! */
int inNCCC_MFES_INST_ADJUST[] = {3, 4, 11, 12, 13, 22, 24, 25, 35, 37, 38, 41, 42, 59, 60, 62, 63, 64, 0}; /* 最後一組一定要放 0!! */
/* void */
int inNCCC_MFES_VOID[] = {3, 4, 11, 22, 24, 25, 35, 37, 38, 41, 42, 59, 60, 62, 64, 0}; /* 最後一組一定要放 0!! */
int inNCCC_MFES_CUP_VOID[] = {3, 4, 11, 22, 24, 25, 35, 37, 38, 41, 42, 59, 60, 62, 64, 0}; /* 最後一組一定要放 0!! */
int inNCCC_MFES_CUP_PRE_COMP_VOID[] = {3, 4, 11, 12, 13, 22, 24, 25, 35, 37, 38, 41, 42, 59, 60, 62, 0}; /* 最後一組一定要放 0!! */
/* offline (offline補登要送12、13，但online補登不送，所以預設不送，當成advice送時才加F_12、F_13) */
int inNCCC_MFES_SALE_OFFLINE[] = {3, 4, 11, 12, 13, 22, 24, 25, 35, 37, 38, 41, 42, 59, 60, 62, 64, 0}; /* 最後一組一定要放 0!! */
/* advice */
int inNCCC_MFES_ADVICE[] = {3, 4, 11, 12, 13, 22, 24, 25, 35, 37, 38, 41, 42, 59, 60, 62, 64, 0}; /* 最後一組一定要放 0!! */
/* TC-UPLOAD */
int inNCCC_MFES_ICCTcUpload[] = {3, 4, 11, 12, 13, 22, 24, 25, 35, 37, 38, 41, 42, 55, 56, 59, 60, 62, 0}; /* 最後一組一定要放 0!! */
/* tip */
int inNCCC_MFES_TIP[] = {3, 4, 11, 12, 13, 22, 24, 25, 35, 37, 38, 41, 42, 54, 59, 60, 62, 64, 0}; /* 最後一組一定要放 0!! */
/* batch upload */
int inNCCC_MFES_BATCH_UPLOAD[] = {3, 4, 11, 12, 13, 22, 24, 25, 35, 37, 38, 41, 42, 59, 60, 62, 0}; /* 最後一組一定要放 0!! */
/* reversal */
int inNCCC_MFES_REVERSAL[] = {3, 4, 11, 22, 24, 25, 35, 37, 41, 42, 59, 60, 62, 0}; /* 最後一組一定要放 0!! */
/* settle */
int inNCCC_MFES_SETTLE[] = {3, 11, 24, 41, 42, 60, 63, 0}; /* 最後一組一定要放 0!! */
int inNCCC_MFES_CLS_BATCH[] = {3, 11, 24, 41, 42, 60, 63, 0}; /* 最後一組一定要放 0!! */
/* TMK LOGON */
int inNCCC_MFES_TMK_LOGON[] = {3, 11, 24, 25, 41, 42, 59, 0}; /* 最後一組一定要放 0!! */
/* ESC_ADVICE */
int inNCCC_MFES_ESC_SEND_ADVICE[] = {3, 4, 11, 12, 13, 22, 24, 25, 35, 37, 38, 41, 42, 59, 60, 62, 0}; /* 最後一組一定要放 0!! */
/* 查詢金融卡代碼 */
int inNCCC_MFES_INQUERY_ISSUE_BANK[] = {3, 11, 24, 25, 35, 37, 41, 42, 59, 0}; /* 最後一組一定要放 0!! */

BIT_MAP_NCCC_MFES_TABLE srNCCC_MFES_ISOBitMap[] =
{
	{_SALE_,		inNCCC_MFES_SALE,		"0200",		"000000"},
	{_INST_SALE_,		inNCCC_MFES_INST_SALE,		"0200",		"000000"},
	{_REDEEM_SALE_,		inNCCC_MFES_REDEEM_SALE,	"0200",		"000000"},
	{_PRE_AUTH_,		inNCCC_MFES_PRE_AUTH,		"0100",		"300000"},
	{_PRE_COMP_,		inNCCC_MFES_PRE_COMP,		"0220",		"000000"},
	{_TC_UPLOAD_,		inNCCC_MFES_ICCTcUpload,	"0220",		"250000"}, 
	{_ADVICE_,		inNCCC_MFES_ADVICE,		"0220",		"000000"},
	{_REVERSAL_,		inNCCC_MFES_REVERSAL,		"0400",		"000000"},
	{_REFUND_,		inNCCC_MFES_REFUND,		"0200",		"200000"},
	{_INST_REFUND_,		inNCCC_MFES_INST_REFUND,	"0200",		"200000"},
	{_REDEEM_REFUND_,	inNCCC_MFES_REDEEM_REFUND,	"0200",		"200000"},
	{_VOID_,		inNCCC_MFES_VOID,		"0200",		"020000"},
	{_TIP_,			inNCCC_MFES_TIP,		"0220",		"020000"},
	{_REDEEM_ADJUST_,	inNCCC_MFES_REDEEM_ADJUST,	"0220",		"000000"},
	{_INST_ADJUST_,		inNCCC_MFES_INST_ADJUST,	"0220",		"000000"},
	{_SALE_OFFLINE_,	inNCCC_MFES_SALE_OFFLINE,	"0220",		"000000"},
	{_CUP_LOGON_,		inNCCC_MFES_TMK_LOGON,		"0800",		"000000"},
	{_CUP_SALE_,		inNCCC_MFES_CUP_SALE,		"0200",		"000000"}, /* CUP_銷售 */
	{_CUP_REFUND_,		inNCCC_MFES_CUP_REFUND,		"0200",		"200000"}, /* CUP_退貨 */
	{_CUP_VOID_,		inNCCC_MFES_CUP_VOID,		"0200",		"020000"}, /* CUP_取消 */
	{_CUP_PRE_AUTH_,	inNCCC_MFES_CUP_PRE_AUTH,	"0100",		"300000"}, /* CUP_預先授權 */
	{_CUP_PRE_COMP_,	inNCCC_MFES_CUP_PRE_COMP,	"0220",		"000000"}, /* CUP_預先授權完成 */
	{_CUP_PRE_AUTH_VOID_,	inNCCC_MFES_CUP_PRE_AUTH_VOID,	"0200",		"320000"}, /* Void Pre-Auth (CUP only )：銀聯取消預先授權 */
	{_CUP_PRE_COMP_VOID_,	inNCCC_MFES_CUP_PRE_COMP_VOID,	"0220",		"420000"}, /* CUP_取消預先授權完成 */
	{_CUP_INST_SALE_,	inNCCC_MFES_CUP_INST_SALE,	"0200",		"000000"},
	{_CUP_REDEEM_SALE_,	inNCCC_MFES_CUP_REDEEM_SALE,	"0200",		"000000"},
	{_CUP_INST_REFUND_,	inNCCC_MFES_CUP_INST_REFUND,	"0200",		"200000"},
	{_CUP_REDEEM_REFUND_,	inNCCC_MFES_CUP_REDEEM_REFUND,	"0200",		"200000"},
	{_SETTLE_,		inNCCC_MFES_SETTLE,		"0500",		"920000"}, /* Initial Settlement */
	{_BATCH_UPLOAD_,	inNCCC_MFES_BATCH_UPLOAD,	"0320",		"000001"}, /* (Unbalance Upload */
	{_CLS_BATCH_,		inNCCC_MFES_CLS_BATCH,		"0500",		"960000"}, /* Repeat Settlement(after transactions uploaded) */
	{_LOYALTY_REDEEM_,	inNCCC_MFES_LOYALTY_REDEEM,	"0200",		"700000"}, /* Loyalty Redeem：優惠兌換 */
	{_VOID_LOYALTY_REDEEM_,	inNCCC_MFES_VOID_LOYALTY_REDEEM,"0200",		"720000"}, /* Loyalty Redeem：優惠兌換 */
	{_SEND_ESC_ADVICE_,	inNCCC_MFES_ESC_SEND_ADVICE,	"0220",		"690000"}, /* ESC Advice */
	{_INQUIRY_ISSUER_BANK_,	inNCCC_MFES_INQUERY_ISSUE_BANK,	"0800",		"006000"}, /* 查詢發卡機構金融代碼 */
	{_NCCC_MFES_NULL_TX_,	NULL,				"0000",		"000000"}, /* 最後一組一定要放 TMS_NULL_TX!! */
};

ISO_TYPE_NCCC_MFES_TABLE srNCCC_MFES_ISOFunc[] =
{
        {
           srNCCC_MFES_ISOFieldPack,
           srNCCC_MFES_ISOFieldUnPack,
           srNCCC_MFES_ISOFieldCheck,
           srNCCC_MFES_ISOFieldType,
           srNCCC_MFES_ISOBitMap,
           inNCCC_MFES_ISOPackMessageType,
           inNCCC_MFES_ISOModifyBitMap,
           inNCCC_MFES_ISOModifyPackData,
           inNCCC_MFES_ISOCheckHeader,
           inNCCC_MFES_ISOOnlineAnalyse,
           inNCCC_MFES_ISOAdviceAnalyse
        },

        /* tSAM用 */
        {
           srNCCC_MFES_ISOFieldPack_tSAM,
           srNCCC_MFES_ISOFieldUnPack,
           srNCCC_MFES_ISOFieldCheck,
           srNCCC_MFES_ISOFieldType,
           srNCCC_MFES_ISOBitMap,
           inNCCC_MFES_ISOPackMessageType,
           inNCCC_MFES_ISOModifyBitMap,
           inNCCC_MFES_ISOModifyPackData,
           inNCCC_MFES_ISOCheckHeader,
           inNCCC_MFES_ISOOnlineAnalyse,
           inNCCC_MFES_ISOAdviceAnalyse
        },
	
	/* 軟加用 */
	{
           srNCCC_MFES_ISOFieldPack_SoftWare,
           srNCCC_MFES_ISOFieldUnPack,
           srNCCC_MFES_ISOFieldCheck,
           srNCCC_MFES_ISOFieldType,
           srNCCC_MFES_ISOBitMap,
           inNCCC_MFES_ISOPackMessageType,
           inNCCC_MFES_ISOModifyBitMap,
           inNCCC_MFES_ISOModifyPackData,
           inNCCC_MFES_ISOCheckHeader,
           inNCCC_MFES_ISOOnlineAnalyse,
           inNCCC_MFES_ISOAdviceAnalyse
        }
};

/*
Function        :inNCCC_MFES_Func_SetTxnOnlineOffline
Date&Time       :2016/9/14 上午 11:52
Describe        :根據交易別決定是否Online
Log				:Done
*/
int inNCCC_MFES_Func_SetTxnOnlineOffline(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;
	char	szExpectApplicationIndex[2 + 1] = {0};
	char	szTemplate[12 + 1] = {0};
	char	szEMVFloorLimit[10 + 1] = {0};
	long	lnEMVFloorLimit = 0;
    
        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_MFES_Func_SetTxnOnlineOffline START!");
        
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inNCCC_MFES_Func_SetTxnOnlineOffline START!");
        }
	
	if (pobTran->inTransactionCode == _SALE_)
	{
		/* EMV Card 且 EMV Kernel 決定 Force Online */
		if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
		{
			pobTran->srBRec.uszOfflineBit = VS_FALSE;
		}
		
		/* VLP JCB Y1 要OFFLINE */
		if ((!memcmp(pobTran->srBRec.szAuthCode, "VLP", 3) && pobTran->srBRec.uszContactlessBit == VS_TRUE) ||
		    (!memcmp(pobTran->srBRec.szAuthCode, "JCB", 3) && pobTran->srBRec.uszContactlessBit == VS_TRUE) ||
		    (!memcmp(pobTran->srBRec.szAuthCode, "Y1", 2) && pobTran->srBRec.uszContactlessBit  == VS_TRUE) ||
		    (!memcmp(pobTran->srBRec.szAuthCode, "Y1", 2) && pobTran->srBRec.inChipStatus == _EMV_CARD_))
		{
			/* Add by hanlin 2011.9.29 PM 3:40 若是EMV Y1超過Floor Limit則要拒絕 (START) */
			if (pobTran->srBRec.inChipStatus == _EMV_CARD_ && !memcmp(pobTran->srBRec.szAuthCode, "Y1", 2))
			{
				/* EMV Kernel 6.0.1 修正【MVT.txt】Index 【VISA = 1】【MASTERCARD = 2】【JCB = 3】
				   #define VISA_MVT_INDEX	1
				   #define MCHIP_MVT_INDEX	2
				   #define JSMART_MVT_INDEX	3 */
				memset(szExpectApplicationIndex, 0x00, sizeof(szExpectApplicationIndex));
				if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_VISA_, strlen(_CARD_TYPE_VISA_)))
				{
					strcpy(szExpectApplicationIndex, _MVT_VISA_NCCC_INDEX_);	/* VISA */
				}
				else if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)))
				{
					strcpy(szExpectApplicationIndex, _MVT_MCHIP_NCCC_INDEX_);	/* MASTER CARD */
				}
				else if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_JCB_, strlen(_CARD_TYPE_JCB_)))
				{
					strcpy(szExpectApplicationIndex, _MVT_JSMART_NCCC_INDEX_);	/* J SMART */
				}
				else
				{
					vdUtility_SYSFIN_LogMessage(AT, "inNCCC_MFES_Func_SetTxnOnlineOffline no such flow(%s)", pobTran->srBRec.szCardLabel);
					/* 晶片卡流程有誤 */
					/* 請重新交易 */
					DISPLAY_OBJECT	srDispMsgObj;
					memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
					strcpy(srDispMsgObj.szDispPic1Name, _ERR_EMV_FLOW_);
					srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_5_;
					srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
					srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
					strcpy(srDispMsgObj.szErrMsg1, "");
					srDispMsgObj.inErrMsg1Line = 0;
					srDispMsgObj.inBeepTimes = 1;
					srDispMsgObj.inBeepInterval = 0;
					inDISP_Msg_BMP(&srDispMsgObj);
					
					return (VS_ERROR);
				}
				
				inRetVal = inFunc_Load_MVT_By_Index(szExpectApplicationIndex);
				if (inRetVal != VS_SUCCESS)
				{
					vdUtility_SYSFIN_LogMessage(AT,"inFunc_Load_MVT_By_Index(%s):no matching MVTRecord found.",szExpectApplicationIndex);
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "MVT 中無此感應卡");
					}

					return (VS_WAVE_ERROR);
				}
				
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szEMVFloorLimit, 0x00, sizeof(szEMVFloorLimit));
				
				inGetEMVFloorLimit(szTemplate);
				memcpy(szEMVFloorLimit, szTemplate, 10);
				
				lnEMVFloorLimit = atol(szEMVFloorLimit);
				if (pobTran->srBRec.lnTxnAmount >= lnEMVFloorLimit)
				{
					vdUtility_SYSFIN_LogMessage(AT, "srBRec.lnTxnAmount(%ld) exceeds the EMVFloorLimit(%ld)", pobTran->srBRec.lnTxnAmount, lnEMVFloorLimit);	
					/* 晶片卡流程有誤 */
					/* 請重新交易 */
					DISPLAY_OBJECT	srDispMsgObj;
					memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
					strcpy(srDispMsgObj.szDispPic1Name, _ERR_EMV_FLOW_);
					srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_5_;
					srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
					srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
					strcpy(srDispMsgObj.szErrMsg1, "");
					srDispMsgObj.inErrMsg1Line = 0;
					srDispMsgObj.inBeepTimes = 1;
					srDispMsgObj.inBeepInterval = 0;
					inDISP_Msg_BMP(&srDispMsgObj);
					
					return (VS_ERROR);
				}
				/* 若是EMV Y1超過Floor Limit則要拒絕 (END) */
			}

			/* VLP JCB Y1 要OFFLINE */
			pobTran->srBRec.uszOfflineBit = VS_TRUE;

			pobTran->srBRec.uszUpload1Bit = VS_TRUE; /* 交易補登【ADVICE】未上傳 */
			pobTran->srBRec.uszUpload2Bit = VS_FALSE;
			pobTran->srBRec.uszUpload3Bit = VS_FALSE;
		}/* VLP JCB Y1 要OFFLINE (END) */
		
	}
	/* 聯合有online補登所以要特別判斷 */
        else if (pobTran->inTransactionCode == _SALE_OFFLINE_ || pobTran->inTransactionCode == _INST_ADJUST_ || pobTran->inTransactionCode == _REDEEM_ADJUST_ || pobTran->inTransactionCode == _FORCE_CASH_ADVANCE_)
        {
//		/* 抓交易功能開關 */
//		memset(szTxnType, 0x00, sizeof(szTxnType));
//		inGetTransFunc(szTxnType);
//		
//		if (szTxnType[4] == 0x31)
//		{
		/* MFES雲端化 在改回來 */
			pobTran->srBRec.uszForceOnlineBit = VS_TRUE;
			pobTran->srBRec.uszOfflineBit = VS_FALSE; /* 強制Online */
			
			pobTran->srBRec.uszUpload1Bit = VS_FALSE; /* 強制Online */
			pobTran->srBRec.uszUpload2Bit = VS_FALSE;
			pobTran->srBRec.uszUpload3Bit = VS_FALSE;
//		}
//		else
//		{
//			pobTran->srBRec.uszForceOnlineBit = VS_FALSE;
//			pobTran->srBRec.uszOfflineBit = VS_TRUE; /* 改成離線交易 */
//			
//			pobTran->srBRec.uszUpload1Bit = VS_TRUE; /* 交易補登【ADVICE】未上傳 */
//			pobTran->srBRec.uszUpload2Bit = VS_FALSE;
//			pobTran->srBRec.uszUpload3Bit = VS_FALSE;
//		}

        }
        else if (pobTran->inTransactionCode == _TIP_)
        {
		/* 只有這些交易別支援小費 */
                if (pobTran->srBRec.inOrgCode == _SALE_		|| pobTran->srBRec.inOrgCode == _SALE_OFFLINE_	||
		    pobTran->srBRec.inOrgCode == _INST_SALE_	|| pobTran->srBRec.inOrgCode == _REDEEM_SALE_	)
                {
			/* ATS小費交易, 一律Go Offline */
			/* MFES小費交易則要看原交易是Online還是offline */
			/* 原交易是離線交易而且未上傳 */
                        if (pobTran->srBRec.uszOfflineBit == VS_TRUE && pobTran->srBRec.uszUpload1Bit == VS_TRUE)
                        {
				pobTran->srBRec.uszOfflineBit = VS_TRUE;	/* 原交易是離線交易 */
                                pobTran->srBRec.uszUpload1Bit = VS_TRUE;	/* 原交易【ADVICE】未上傳 */
                                pobTran->srBRec.uszUpload2Bit = VS_TRUE;	/* 小費交易以【ADVICE】處理 */
                                pobTran->srBRec.uszUpload3Bit = VS_FALSE;	
                        }
			/* 原交易是離線交易而且已經上傳 */
                        else if (pobTran->srBRec.uszOfflineBit == VS_TRUE && pobTran->srBRec.uszUpload1Bit == VS_FALSE)
                        {
				pobTran->srBRec.uszOfflineBit = VS_FALSE;	/* 當筆交易要 Online */
                                pobTran->srBRec.uszUpload1Bit = VS_FALSE;	/* 原交易上傳 */
                                pobTran->srBRec.uszUpload2Bit = VS_FALSE;	/* 小費交易以【ADVICE】處理 */
                                pobTran->srBRec.uszUpload3Bit = VS_FALSE;
                        }
			else
			{
				pobTran->srBRec.uszOfflineBit = VS_FALSE;	/* 當筆交易要 Online */
			}
			
                }
                else
                {
						vdUtility_SYSFIN_LogMessage(AT, "inNCCC_MFES_Func_SetTxnOnlineOffline no such flow(%d)", pobTran->srBRec.inOrgCode);
                        return (VS_ERROR);
                }
        }
        else if (pobTran->inTransactionCode == _VOID_)
        {
                if (pobTran->srBRec.inOrgCode == _SALE_OFFLINE_)
                {
			/* 原交易未上傳，要等原交易上傳 */
                        if (pobTran->srBRec.uszUpload1Bit == VS_TRUE)
                        {
                                pobTran->srBRec.uszOfflineBit = VS_TRUE; /* 改成離線交易 */
                                pobTran->srBRec.uszUpload1Bit = VS_TRUE; /* 交易補登【ADVICE】還未上傳 */
                                pobTran->srBRec.uszUpload2Bit = VS_TRUE; /* 取消交易要以【ADVICE】處理 */
                                pobTran->srBRec.uszUpload3Bit = VS_FALSE;
                        }
			/* 原交易已上傳，則這筆取消可以Online */
                        else
                        {
                                /* 開始預設值 */
                                pobTran->srBRec.uszOfflineBit = VS_FALSE; /* 當筆交易要 Online */
                                pobTran->srBRec.uszUpload1Bit = VS_FALSE; /* 不會產生任何 Advice */
                                pobTran->srBRec.uszUpload2Bit = VS_FALSE; /* 不會產生任何 Advice */
                                pobTran->srBRec.uszUpload3Bit = VS_FALSE; /* 不會產生任何 Advice */
                        }
                }
		/* 原晶片卡或感應卡為離線交易 */
		else if (pobTran->srBRec.inOrgCode == _SALE_ && 
			((!memcmp(pobTran->srBRec.szAuthCode, "VLP", 3) && pobTran->srBRec.uszContactlessBit == VS_TRUE) ||
			 (!memcmp(pobTran->srBRec.szAuthCode, "JCB", 3) && pobTran->srBRec.uszContactlessBit == VS_TRUE) ||
			 (!memcmp(pobTran->srBRec.szAuthCode, "Y1", 2) && pobTran->srBRec.uszContactlessBit  == VS_TRUE) ||
			 (!memcmp(pobTran->srBRec.szAuthCode, "Y1", 2) && pobTran->srBRec.inChipStatus == _EMV_CARD_)))
		{
			/* 原交易未上傳，要等原交易上傳 */
                        if (pobTran->srBRec.uszUpload1Bit == VS_TRUE)
                        {
                                pobTran->srBRec.uszOfflineBit = VS_TRUE; /* 改成離線交易 */
                                pobTran->srBRec.uszUpload1Bit = VS_TRUE; /* 交易補登【ADVICE】還未上傳 */
                                pobTran->srBRec.uszUpload2Bit = VS_TRUE; /* 取消交易要以【ADVICE】處理 */
                                pobTran->srBRec.uszUpload3Bit = VS_FALSE;
                        }
			/* 原交易已上傳，則這筆取消可以Online */
                        else
                        {
                                /* 開始預設值 */
                                pobTran->srBRec.uszOfflineBit = VS_FALSE; /* 當筆交易要 Online */
                                pobTran->srBRec.uszUpload1Bit = VS_FALSE; /* 不會產生任何 Advice */
                                pobTran->srBRec.uszUpload2Bit = VS_FALSE; /* 不會產生任何 Advice */
                                pobTran->srBRec.uszUpload3Bit = VS_FALSE; /* 不會產生任何 Advice */
                        }
			
		}
		/* 原交易為分期調帳或紅利調帳離線交易 */
		else if (pobTran->srBRec.inOrgCode == _INST_ADJUST_	||
			 pobTran->srBRec.inOrgCode == _REDEEM_ADJUST_)
		{
			/* 原交易未上傳，要等原交易上傳 */
                        if (pobTran->srBRec.uszUpload1Bit == VS_TRUE)
                        {
                                pobTran->srBRec.uszOfflineBit = VS_TRUE; /* 改成離線交易 */
                                pobTran->srBRec.uszUpload1Bit = VS_TRUE; /* 交易補登【ADVICE】還未上傳 */
                                pobTran->srBRec.uszUpload2Bit = VS_TRUE; /* 取消交易要以【ADVICE】處理 */
                                pobTran->srBRec.uszUpload3Bit = VS_FALSE;
                        }
			/* 原交易已上傳，則這筆取消可以Online */
                        else
                        {
                                /* 開始預設值 */
                                pobTran->srBRec.uszOfflineBit = VS_FALSE; /* 當筆交易要 Online */
                                pobTran->srBRec.uszUpload1Bit = VS_FALSE; /* 不會產生任何 Advice */
                                pobTran->srBRec.uszUpload2Bit = VS_FALSE; /* 不會產生任何 Advice */
                                pobTran->srBRec.uszUpload3Bit = VS_FALSE; /* 不會產生任何 Advice */
                        }
			
		}
                else if (pobTran->srBRec.inOrgCode == _TIP_)
                {
			/* 原交易未上傳，要等原交易上傳 */
                        if (pobTran->srBRec.uszUpload1Bit == VS_TRUE && pobTran->srBRec.uszUpload2Bit == VS_TRUE)
                        {
                                pobTran->srBRec.uszOfflineBit = VS_TRUE; /* 改成離線交易 */

                                pobTran->srBRec.uszUpload1Bit = VS_TRUE; /* 原交易【ADVICE】還未上傳 */
                                pobTran->srBRec.uszUpload2Bit = VS_TRUE; /* 小費交易【ADVICE】還未上傳 */
                                pobTran->srBRec.uszUpload3Bit = VS_TRUE; /* 取消-小費交易要以【ADVICE】處理 */
                        }
			/* 原交易已上傳，原交易調帳未上傳，要等原交易調帳上傳 */
                        else if (pobTran->srBRec.uszUpload1Bit == VS_FALSE && pobTran->srBRec.uszUpload2Bit == VS_TRUE)
                        {
                                pobTran->srBRec.uszOfflineBit = VS_TRUE; /* 改成離線交易 */

                                pobTran->srBRec.uszUpload1Bit = VS_FALSE; /* 原交易【ADVICE】已上傳 */
                                pobTran->srBRec.uszUpload2Bit = VS_TRUE; /* 小費交易【ADVICE】還未上傳 */
                                pobTran->srBRec.uszUpload3Bit = VS_TRUE; /* 取消-小費交易要以【ADVICE】處理 */
                        }
			/* 原交易已上傳，原交易調帳已上傳，原交易調帳取消可以Online */
                        else
                        {
                                /* 開始預設值 */
                                pobTran->srBRec.uszOfflineBit = VS_FALSE; /* 當筆交易要 Online */
                                pobTran->srBRec.uszUpload1Bit = VS_FALSE; /* 不會產生任何 Advice */
                                pobTran->srBRec.uszUpload2Bit = VS_FALSE; /* 不會產生任何 Advice */
                                pobTran->srBRec.uszUpload3Bit = VS_FALSE; /* 不會產生任何 Advice */
                        }
                }
                else if (pobTran->srBRec.inOrgCode == _ADJUST_)
                {
			/* 原交易未上傳，要等原交易上傳 */
                        if (pobTran->srBRec.uszUpload1Bit == VS_TRUE && pobTran->srBRec.uszUpload2Bit == VS_TRUE)
                        {
                                pobTran->srBRec.uszOfflineBit = VS_TRUE; /* 改成離線交易 */

                                pobTran->srBRec.uszUpload1Bit = VS_TRUE; /* 原交易【ADVICE】還未上傳 */
                                pobTran->srBRec.uszUpload2Bit = VS_TRUE; /* 調帳交易【ADVICE】還未上傳 */
                                pobTran->srBRec.uszUpload3Bit = VS_TRUE; /* 取消-小費交易要以【ADVICE】處理 */
                        }
			/* 原交易已上傳，原交易調帳未上傳，要等原交易調帳上傳 */
                        else if (pobTran->srBRec.uszUpload1Bit == VS_FALSE && pobTran->srBRec.uszUpload2Bit == VS_TRUE)
                        {
                                pobTran->srBRec.uszOfflineBit = VS_TRUE; /* 改成離線交易 */

                                pobTran->srBRec.uszUpload1Bit = VS_FALSE; /* 原交易【ADVICE】已上傳 */
                                pobTran->srBRec.uszUpload2Bit = VS_TRUE; /* 調帳交易【ADVICE】還未上傳 */
                                pobTran->srBRec.uszUpload3Bit = VS_TRUE; /* 取消-小費交易要以【ADVICE】處理 */
                        }
			/* 原交易已上傳，原交易調帳已上傳，原交易調帳取消可以Online */
                        else
                        {
                                /* 開始預設值 */
                                pobTran->srBRec.uszOfflineBit = VS_FALSE; /* 當筆交易要 Online */
                                pobTran->srBRec.uszUpload1Bit = VS_FALSE; /* 不會產生任何 Advice */
                                pobTran->srBRec.uszUpload2Bit = VS_FALSE; /* 不會產生任何 Advice */
                                pobTran->srBRec.uszUpload3Bit = VS_FALSE; /* 不會產生任何 Advice */
                        }
                }
		/* 其他原交易會Online的狀況 */
                else
                {
                        /* 開始預設值 */
                        pobTran->srBRec.uszOfflineBit = VS_FALSE; /* 當筆交易要 Online */
                        pobTran->srBRec.uszUpload1Bit = VS_FALSE; /* 不會產生任何 Advice */
                        pobTran->srBRec.uszUpload2Bit = VS_FALSE; /* 不會產生任何 Advice */
                        pobTran->srBRec.uszUpload3Bit = VS_FALSE; /* 不會產生任何 Advice */
                }
		
        }
        else
        {
                /* 開始預設值 */
                pobTran->srBRec.uszOfflineBit = VS_FALSE; /* 當筆交易要 Online */
                pobTran->srBRec.uszUpload1Bit = VS_FALSE; /* 不會產生任何 Advice */
                pobTran->srBRec.uszUpload2Bit = VS_FALSE; /* 不會產生任何 Advice */
                pobTran->srBRec.uszUpload3Bit = VS_FALSE; /* 不會產生任何 Advice */
        }
	
	/* 新增感應及晶片go online防呆機制，若卡片進入offline流程，畫面提示”OL 拒絕交易”。*/
	if ((pobTran->srBRec.inChipStatus == _EMV_CARD_	|| pobTran->srBRec.uszContactlessBit  == VS_TRUE)	&& 
	     pobTran->srBRec.uszOfflineBit == VS_TRUE								&&
	    (pobTran->inTransactionCode != _TIP_		&& 
	     pobTran->inTransactionCode != _SALE_OFFLINE_	&&
	     pobTran->inTransactionCode != _VOID_))
	{
		memcpy(pobTran->srBRec.szRespCode, "OL", strlen("OL"));
		inNCCC_MFES_DispHostResponseCode(pobTran);

		return (VS_ERROR);
	}
	
        /* offline交易及settle不需要做reversal */
        pobTran->uszReversalBit = VS_TRUE;

        if (pobTran->srBRec.uszOfflineBit == VS_TRUE)
        {
                pobTran->uszReversalBit = VS_FALSE;
        }
        else
        {
                if (pobTran->inTransactionCode == _SETTLE_)
                {
                        pobTran->uszReversalBit = VS_FALSE;
                }
		/* 同520的TIP邏輯，即使MFES小費online，也不送reversal */
		else if (pobTran->inTransactionCode == _TIP_)
		{
			pobTran->uszReversalBit = VS_FALSE;
		}
        }
	
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inNCCC_MFES_Func_SetTxnOnlineOffline END!");
        }    
        
        return (VS_SUCCESS);
}

/*
Function        :inNCCC_MFES_Func_BuildAndSendPacket
Date&Time       :2016/9/14 下午 12:00
Describe        :處理交易，發送並分析
*/
int inNCCC_MFES_Func_BuildAndSendPacket(TRANSACTION_OBJECT *pobTran)
{
        int	inRetVal = VS_ERROR;
	int	inBatchCnt = 0;
	char	szDemoMode[2 + 1] = {0};
	
        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_MFES_Func_BuildAndSendPacket START!");
        
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_Func_BuildAndSendPacket() START!");

	/* 教育訓練模式 */
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		inRetVal = inNCCC_Func_BuildAndSendPacket_Demo_Flow(pobTran);
		
		/* 步驟 1 Get System Trans Number */
		inNCCC_MFES_GetSTAN(pobTran);
		/* Set STAN */
		inNCCC_MFES_SetSTAN(pobTran);
		
		/* 交易失敗 */
		if (inRetVal != VS_SUCCESS)
		{
			return (inRetVal);
		}
		
		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "inNCCC_MFES_Func_BuildAndSendPacket() END!");
		
		return (VS_SUCCESS);
	}
	else
	{
		if (pobTran->inTransactionCode == _SETTLE_)
		{
			/* 如果是結帳交易，要把請先結帳的Bit On起來，直到接收到host的回覆才Off*/
			inBatchCnt = inBATCH_GetTotalCountFromBakFile_By_Sqlite(pobTran);
			if (inBatchCnt >= 0)
			{
				inNCCC_MFES_SetMustSettleBit(pobTran, "Y");
			}
			else
			{
				/* 開啟失敗 */
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "inBATCH_GetTotalCountFromBakFile_By_Sqlite Fail:%d", inBatchCnt);
				}
				inUtility_StoreTraceLog_OneStep("inBATCH_GetTotalCountFromBakFile_By_Sqlite Fail:%d", inBatchCnt);

				inFunc_EDCLock(AT);
			}
		}
		
		/* 步驟 1 Get System Trans Number */
		if (inNCCC_MFES_GetSTAN(pobTran) == VS_ERROR)
			return (VS_ERROR);

		/* 步驟 1.1 處理online交易 */
		if (pobTran->srBRec.uszOfflineBit == VS_FALSE)
		{
			/* Reset title(只有Online的交易才需要Reset) */
			inFunc_ResetTitle(pobTran);

			/* 步驟 1.2 開始連線 */
			if (pobTran->uszConnectionBit != VS_TRUE)
			{
				inRetVal = inFLOW_RunFunction(pobTran, _COMM_START_);
				if (inRetVal != VS_SUCCESS)
				{
					/* 通訊失敗‧‧‧‧ */
					pobTran->inTransactionResult = _TRAN_RESULT_COMM_ERROR_;
					pobTran->inErrorMsg = _ERROR_CODE_V3_COMM_;
					inFunc_Display_Error(pobTran);			/* 通訊失敗 */

					return (VS_COMM_ERROR);
				}
			}

			/* 步驟 1.3 檢查是否為 Online 交易，先送上筆交易失敗的 Reversal 及產生當筆交易 Reversal */
			if ((inRetVal = inNCCC_MFES_ProcessReversal(pobTran)) != VS_SUCCESS)
			{
				return (inRetVal);
			}

			/* 步驟 1.4 */
			if (pobTran->inTransactionCode == _SETTLE_)
			{
				if ((inRetVal = inNCCC_MFES_ProcessAdvice(pobTran)) != VS_SUCCESS)
				{
					/* 只有結帳時前送advice錯誤時要顯示錯誤訊息，在inNCCC_MFES_ISOAdviceAnalyse裡顯示錯誤訊息 */
					pobTran->inErrorMsg = _ERROR_CODE_V3_SETTLE_NOT_SUCCESS_;
					return (inRetVal);
				}

				if ((inRetVal = inNCCC_MFES_ProcessAdvice_ESC(pobTran)) != VS_SUCCESS)
				{
					/* 只有結帳時前送advice錯誤時要顯示錯誤訊息，在inNCCC_MFES_ISOAdviceAnalyse裡顯示錯誤訊息 */
					pobTran->inErrorMsg = _ERROR_CODE_V3_SETTLE_NOT_SUCCESS_;
					return (inRetVal);
				}

			}

		}

		/* 處理【ONLINE】交易 */
		if (pobTran->srBRec.uszOfflineBit == VS_FALSE)
		{
			if ((inRetVal = inNCCC_MFES_ProcessOnline(pobTran)) != VS_SUCCESS)
			{
                                if (pobTran->srBRec.inChipStatus == _EMV_CARD_ &&
                                    pobTran->inTransactionResult == _TRAN_RESULT_COMM_ERROR_)
				{
					
				}
                                else
                                {
                                        inNCCC_MFES_SetSTAN(pobTran); /* 成功或失敗 System_Trace_Number 都要加一 */

                                        return (inRetVal); /* Return 【VS_ERROR】【VS_SUCCESS】 */
                                }
			}
		}
		else
		{
			/* 處理【OFFLINE】交易 */
			inRetVal = inNCCC_MFES_ProcessOffline(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
		}

		inNCCC_MFES_SetSTAN(pobTran); /* 成功或失敗 System_Trace_Number 都要加一 */

		/* 1. 表示有收到完整的資料後的分析
		   2. 處理 Online & Offline 交易
		 */
		inRetVal = inNCCC_MFES_AnalysePacket(pobTran);
		if (inRetVal != VS_SUCCESS)
		{
			return (inRetVal);
		}

		if (pobTran->inTransactionCode == _SETTLE_)
		{
			/* 安全認證失敗不影響結帳 */
			inRetVal = inNCCC_MFES_CUP_LogOn(pobTran);
			
			if (inRetVal == VS_SUCCESS)
			{
				/* 安全認證成功後重置 */
				ginMacError = 0;
			}
		}

		/* Reset Ttile */
		inFunc_ResetTitle(pobTran);

		/* 避免撥接太久，收送完就斷線 */
		/* 這裡判斷是否要送TC Upload再決定是否斷線 */
		if (pobTran->uszEMVProcessDisconectBit != VS_TRUE)
		{
			inCOMM_End(pobTran);
		}

		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "inNCCC_MFES_Func_BuildAndSendPacket() END!");

		return (VS_SUCCESS);
	}
}

/*
Function        :inNCCC_MFES_Pack03
Date&Time       :2016/12/21 下午 5:30
Describe        :Field_3:       Processing Code
*/
int inNCCC_MFES_Pack03(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 	inCnt = 0;
	char	szDebugMsg[100 + 1];
	char	szAscii[6 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_Pack03() START!");
	
	
	memcpy(&uszPackBuf[inCnt], &guszNCCC_MFES_ISO_Field03[0], _NCCC_MFES_PCODE_SIZE_);
	
        inCnt += _NCCC_MFES_PCODE_SIZE_;
	
	/* for Mac */
	inFunc_BCD_to_ASCII(&gszMAC_F_03[0], &guszNCCC_MFES_ISO_Field03[0], 1);

	if (ginDebug == VS_TRUE)
	{
		memset(szAscii, 0x00, sizeof(szAscii));
		inFunc_BCD_to_ASCII(szAscii, guszNCCC_MFES_ISO_Field03, _NCCC_MFES_PCODE_SIZE_);
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "F_03 [P_CODE %s]", szAscii);
		inLogPrintf(AT, szDebugMsg);
	}
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_Pack03() END!");
	
        return (inCnt);
}

/*
Function        :inNCCC_MFES_Pack04
Date&Time       :2016/12/21 下午 5:30
Describe        :Field_4:       Amount, Transaction
*/
int inNCCC_MFES_Pack04(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 		inCnt = 0;
        char 		szTemplate[100 + 1], szDebugMsg[100 + 1];
	char		szAscii[12 + 1];
        unsigned char 	uszBCD[6 + 1];

	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_Pack04() START!");
	
	
        memset(szTemplate, 0x00, sizeof(szTemplate));

        switch (pobTran->inISOTxnCode)
        {
                case _BATCH_UPLOAD_:
		case _REVERSAL_:
		case _TIP_:
		case _SEND_ESC_ADVICE_ :
			sprintf(szTemplate, "%010ld00", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
                        break;
                default:
                        sprintf(szTemplate, "%010ld00", pobTran->srBRec.lnTxnAmount);
                        break;
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
                inLogPrintf(AT, "inNCCC_MFES_Pack04() END!");
	
        return (inCnt);
}

/*
Function        :inNCCC_MFES_Pack11
Date&Time       :2016/12/21 下午 5:30
Describe        :Field_11:      System Trace Audit Number
*/
int inNCCC_MFES_Pack11(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 		inCnt = 0;
        char 		szSTAN[6 + 1];
	char		szDebugMsg[100 + 1];
	char		szAscii[6 + 1];
        unsigned char 	uszBCD[3 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_Pack11() START!");
	

        memset(szSTAN, 0x00, sizeof(szSTAN));
        sprintf(szSTAN, "%06ld", pobTran->srBRec.lnSTANNum);

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
                inLogPrintf(AT, "inNCCC_MFES_Pack11() END!");
	
        return (inCnt);
}

/*
Function        :inNCCC_MFES_Pack12
Date&Time       :2016/12/21 下午 5:30
Describe        :Field_12:      Time, Local Transaction
*/
int inNCCC_MFES_Pack12(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 		inCnt = 0;
	char		szDebugMsg[100 + 1];
	char		szAscii[6 + 1];
        unsigned char 	uszBCD[3 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_Pack12() START!");
	

        memset(uszBCD, 0x00, sizeof(uszBCD));
        inFunc_ASCII_to_BCD(&uszBCD[0], &pobTran->srBRec.szTime[0], 3);
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
                inLogPrintf(AT, "inNCCC_MFES_Pack12() END!");
	
        return (inCnt);
}

/*
Function        :inNCCC_MFES_Pack13
Date&Time       :2016/12/21 下午 5:30
Describe        :Field_13:      Date, Local Transaction
*/
int inNCCC_MFES_Pack13(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 		inCnt = 0;
	char		szDebugMsg[100 + 1];
	char		szAscii[4 + 1];
        unsigned char 	uszBCD[2 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_Pack13() START!");

        memset(uszBCD, 0x00, sizeof(uszBCD));
        inFunc_ASCII_to_BCD(&uszBCD[0], &pobTran->srBRec.szDate[4], 2);
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
                inLogPrintf(AT, "inNCCC_MFES_Pack13() END!");

        return (inCnt);
}

/*
Function        :inNCCC_MFES_Pack22
Date&Time       :2016/12/21 下午 5:30
Describe        :Field_22:   Point of Service Entry Mode
*/
int inNCCC_MFES_Pack22(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int		inRetVal = VS_ERROR;
        int             inCnt = 0;
	char		szDebugMsg[100 + 1];			
	char		szAscii[4 + 1];				/* 放Debug中轉出來的Ascii用 */
        char            szPOSmode[4 + 1];			/* Field 22的Ascii值 */
	char		szTerminalCapbilityAscii[6 + 1];	/* TerminalCapbility的Ascii值 */
	char		szTerminalCapbility[3 + 1];		/* TerminalCapbility的Hex值 */
	char		szExpectApplicationIndex[2 + 1] = {0};
	unsigned char	uszPinBit = VS_FALSE;
        unsigned char   uszBCD[2 + 1];				/* Field 22的Hex值 */
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_Pack22() START!");
	
	/* 1.比對TerminalCapbility */
	memset(szTerminalCapbility, 0x00, sizeof(szTerminalCapbility));
	memset(szExpectApplicationIndex, 0x00, sizeof(szExpectApplicationIndex));
	if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_VISA_, strlen(_CARD_TYPE_VISA_)) == 0)
	{
		strcpy(szExpectApplicationIndex, _MVT_VISA_NCCC_INDEX_);	/* VISA */
	}
	else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)) == 0)
	{
		strcpy(szExpectApplicationIndex, _MVT_MCHIP_NCCC_INDEX_);	/* MASTERCARD */
	}
	else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_JCB_, strlen(_CARD_TYPE_JCB_)) == 0)
	{
		strcpy(szExpectApplicationIndex, _MVT_JSMART_NCCC_INDEX_);		/* JCB */
	}
	else if (memcmp(pobTran->srBRec.szCUP_EMVAID, "A000000333010101", strlen("A000000333010101")) == 0)
	{
		strcpy(szExpectApplicationIndex, _MVT_CUP_DEBIT_NCCC_INDEX_);
	}
	else if (memcmp(pobTran->srBRec.szCUP_EMVAID, "A000000333010102", strlen("A000000333010102")) == 0)
	{
		strcpy(szExpectApplicationIndex, _MVT_CUP_CREDIT_NCCC_INDEX_);
	}
	else if (memcmp(pobTran->srBRec.szCUP_EMVAID, "A000000333010103", strlen("A000000333010103")) == 0)
	{
		strcpy(szExpectApplicationIndex, _MVT_CUP_QUASI_CREDIT_NCCC_INDEX_);
	}
	else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)) == 0)
	{
		strcpy(szExpectApplicationIndex, _MVT_AEIPS_NCCC_INDEX_);	/* AEIPS */
	}
	else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)) == 0 ||
		 pobTran->srBRec.uszUCARDTransBit == VS_TRUE)
	{
		/* (需求單 - 106349) 2018/10/31 下午 3:41 by Russell */
		if (pobTran->srBRec.uszUCARDTransBit == VS_TRUE	||
		    pobTran->uszUCardBit == VS_TRUE)
		{
			strcpy(szExpectApplicationIndex, _MVT_NCCC_NCCC_INDEX_);
		}
		else
		{
			strcpy(szExpectApplicationIndex, _MVT_D_PAS_NCCC_INDEX_);
		}
	}
	else
	{
		
	}
	
	memset(szTerminalCapbilityAscii, 0x00, sizeof(szTerminalCapbilityAscii));
	memset(szTerminalCapbility, 0x00, sizeof(szTerminalCapbility));
	if (strlen(szExpectApplicationIndex) > 0)
	{
		inRetVal = inFunc_Load_MVT_By_Index(szExpectApplicationIndex);
		if (inRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "MVT 中無此感應卡");
			}

			/* 此處無須跳出，讓Pin Bit則無須判斷 */
		}
		else
		{
			inGetTerminalCapabilities(szTerminalCapbilityAscii);

			/* 有值才繼續 */
			if (strlen(szTerminalCapbilityAscii) > 0)
			{
				inFunc_ASCII_to_BCD((unsigned char*)szTerminalCapbility, szTerminalCapbilityAscii, 3);

				/* If EMVTermCapabilities enable  a , b , c ,mean PIN Entry Capbility enable
				   a. Plaintext PIN for ICC verification
				   b. Enciphered PIN for online verification
				   c. Enciphered PIN for offline verification
				 */
				if ((szTerminalCapbility[1] & 0xD0) == 0xD0)
					uszPinBit = VS_TRUE;
			}
		}
		
	}
	
	
	/* 2.是CUP而且為sale或Pre_Auth(目前只有CUP SALE和CUP PREAUTH要輸入密碼)，有輸入密碼的話就設為true */
	if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
	{
		if (pobTran->srBRec.inCode == _CUP_SALE_ || pobTran->srBRec.inCode == _CUP_PRE_AUTH_)
		{
			/* 因為ESC後組，而且不會存PIN，所以改用uszPinEnterBit記錄 */
			if (pobTran->srBRec.uszPinEnterBit == VS_TRUE)
				uszPinBit = VS_TRUE;
		}
		/* 若銀聯取消交易採輸入調閱編號方式進行，不輸入密碼 */
		else
		{
			uszPinBit = VS_FALSE;
		}
		
	}
	
        /*
         Position 1	|	PAN Entry Mode
        ------------------------------------------------------------
		0	|	Unspecified
		5	|	EMV compliance terminal
        -------------------------------------------------------------
         Position 2 & 3	|	PAN Entry Mode
        ------------------------------------------------------------
		00	|	Unspecified
		01	|	PAN entered manually
		02	|	PAN auto-entry via magnetic stripe
			|	UICC Fallback
		05	|	PAN auto-entry via chip
		07	|	PAN auto-entry via contactless;
			|	MasterCard PayPass Chip
			|	VISA Paywave 1
			|	VISA Paywave 3
			|	JCB J/Speedy
		80	|	M/Chip Fallback
		90	|	VSDC Fallback
		91	|	PAN auto-entry via contactless
			|	PayPass Magnetic Stripe only
		97	|	J/Smart Fallback
         ------------------------------------------------------------
         Position 4	|	PAN Entry Mode
        -------------------------------------------------------------
		1	|	With PIN entry capability
		2	|	Without PIN entry capability
		8	|	Terminal has PIN entry capability but currently PIN PAD
			|	is not operative(M/Chip only)
		9	|	PIN verified by terminal device(M/Chip only)
        -------------------------------------------------------------
         */
        memset(uszBCD, 0x00, sizeof(uszBCD));
        memset(szPOSmode, 0x00, sizeof(szPOSmode));

	/*
		例外狀況處理
		As same as original transaction
		若銀聯取消交易採輸入調閱編號方式進行，則pos entry mode需放”012”
	*/
	if (pobTran->srBRec.uszCUPTransBit == VS_TRUE && pobTran->srBRec.uszVOIDBit == VS_TRUE)
	{
		memset(szPOSmode, 0x00, sizeof(szPOSmode));
		strcpy(szPOSmode, "5012");
	}
	else
	{
		/* Position 1 */
		strcpy(szPOSmode, "5");

		/* Position 2 & 3 */
		/* 晶片卡一律 05 */
		if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
			strcat(szPOSmode, "05");
		else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
		{
			/* MASTERCARD 和 NewJPeedy 感應磁條卡送91 */
			if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE	|| 
			    pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_64_NEWJSPEEDY_MSD		||
			    pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE)
			{
				strcat(szPOSmode, "91");
			}
			/* 除了特例外，只要contactless就是 07 */
			else
				strcat(szPOSmode, "07");
		}
		/* fallback */
		else if (pobTran->srBRec.uszEMVFallBackBit == VS_TRUE)
		{
			if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_VISA_, strlen(_CARD_TYPE_VISA_)))
			{
				strcat(szPOSmode, "90");
			}
			else if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)))
			{
				strcat(szPOSmode, "80");
			}
			else if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_JCB_, strlen(_CARD_TYPE_JCB_)))
			{
				strcat(szPOSmode, "97");
			}
			else
			{
				strcat(szPOSmode, "02");
			}

		}
		/* 01是manual keyin */
		else if (pobTran->srBRec.uszManualBit == VS_TRUE)
		{
			strcat(szPOSmode, "01");
		}
		/* 02是磁卡 */
		else
		{
			strcat(szPOSmode, "02");
		}

		/* Position 4 */
		if (uszPinBit == VS_TRUE)
			strcat(szPOSmode, "1");
		else
			strcat(szPOSmode, "2");
	}

	
	
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
                inLogPrintf(AT, "inNCCC_MFES_Pack22() END!");
	
        return (inCnt);
}

/*
Function        :inNCCC_MFES_Pack24
Date&Time       :2016/12/21 下午 5:30
Describe        :Field_24:      Network International Identifier (NII)
*/
int inNCCC_MFES_Pack24(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 		inCnt = 0;
        char 		szTemplate[100 + 1];
	char		szDebugMsg[100 + 1];
	char		szAscii[4 + 1];
        unsigned char 	uszBCD[2 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_Pack24() START!");
	

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
                inLogPrintf(AT, "inNCCC_MFES_Pack24() END!");

        return (inCnt);
}

/*
Function        :inNCCC_MFES_Pack25
Date&Time       :2016/12/21 下午 5:30
Describe        :Field_25:   Point of Service Condition Code
*/
int inNCCC_MFES_Pack25(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 		inCnt = 0;
	char		szTemplate[100 + 1];
	char		szDebugMsg[100 + 1];
	char		szAscii[2 + 1];
        unsigned char 	uszBCD[1 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_Pack25() START!");

        /*
			Code	|       Meaning
                ---------------------------------------------------
			00	|	Normal presentment
			06	|	Pre-Auth Complete
         */
	/* 初值設為00 */
	inFunc_ASCII_to_BCD(&uszPackBuf[inCnt], "00", 1);
	
	/* 為了印debug訊息，所以都先copy到template內，最後再轉bcd */
        memset(uszBCD, 0x00, sizeof(uszBCD));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	
	if (pobTran->srBRec.inCode == _PRE_COMP_	||
	    pobTran->srBRec.inCode == _CUP_PRE_COMP_)
	{
		memcpy(szTemplate, "06", 2);
	}
	else
	{
		memcpy(szTemplate, "00", 2);
	}
	
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
                inLogPrintf(AT, "inNCCC_MFES_Pack25() END!");

        return (inCnt);
}

/*
Function	:inNCCC_MFES_Pack25_Software
Date&Time	:2016/12/21 下午 5:29
Describe	:Field_25:   Point of Service Condition Code
*/
int inNCCC_MFES_Pack25_Software(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 		inCnt = 0;
	char		szTemplate[2 + 1];
	char		szDebugMsg[100 + 1];
	char		szAscii[2 + 1];
	char		szCommMode[1 + 1];
        unsigned char 	uszBCD[1 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_Pack25_Software() START!");

        /*
                    Code        |       Meaning
                ---------------------------------------------------
                    00          |       Normal presentment
		    06		|	PreAuth Compelete
				|
		    同原交易	|	Batch Upload
         */
	/* 初值設為00 */
	inFunc_ASCII_to_BCD(&uszPackBuf[inCnt], "00", 1);
	
        memset(uszBCD, 0x00, sizeof(uszBCD));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	
	/* Logon固定帶00 */
	if (pobTran->inISOTxnCode == _CUP_LOGON_)
	{
		memcpy(szTemplate, "00", 2);
	}
	else
	{
		inGetCommMode(szCommMode);
		if (memcmp(szCommMode, _COMM_ETHERNET_MODE_, 1) == 0)
			memcpy(szTemplate, "98", 2);
		else if (memcmp(szCommMode, _COMM_GPRS_MODE_, 1) == 0	||
			 memcmp(szCommMode, _COMM_3G_MODE_, 1) == 0	||
			 memcmp(szCommMode, _COMM_4G_MODE_, 1) == 0)
		{
			memcpy(szTemplate, "98", 2);
		}
		else
		{	
			/* Pre compelete是06，Batch Upload要跟原交易一樣 */
			if (pobTran->srBRec.inCode == _PRE_COMP_ ||
			    pobTran->srBRec.inCode == _CUP_PRE_COMP_)
					memcpy(szTemplate, "06", 2);
				else
					memcpy(szTemplate, "00", 2);
		}
	}
		
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
                inLogPrintf(AT, "inNCCC_MFES_Pack25_Software() END!");

        return (inCnt);
}

/*
Function        :inNCCC_MFES_Pack35
Date&Time       :2016/12/21 下午 5:29
Describe        :Field_35:   TRACK 2 Data
*/
int inNCCC_MFES_Pack35(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int     inCnt = 0, inPacketCnt = 0, i;
        char    szT2Data[50 + 1];		/* 目前track2最長到32 */
	char	szFinalPAN[20 + 1];		/* 為了U CARD */
	char	szDebugMsg[100 + 1];
	short   shLen = 0;
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_Pack35() START!");

        /*
                        補充說明：
        1. 本系統所有卡號一律透過ISO8583 Field_35上傳，不使用Field_02及Field_14。
        2. Message Type = 0100、0200
                A. 刷卡或插卡的交易：需上傳【完整Track 2】
                B. 原交易為刷卡或插卡的調整交易(例：取消或小費)：需上傳【卡號】=【有效期】+【Service Code】
                C. 人工輸入卡號的交易：只需上傳【卡號】=【有效期】
                D. 原交易為人工輸入卡號的調整交易：只需上傳【卡號】=【有效期】
        3. Message Type = 0220、0320、0400
                A. 刷卡或插卡的交易：需上傳【卡號】=【有效期】+【Service Code】
                B. 原交易為刷卡或插卡的調整交易(例：取消或小費)：需上傳【卡號】=【有效期】+【Service Code】
                C. 人工輸入卡號的交易：只需上傳【卡號】=【有效期】
                D. 原交易為人工輸入卡號的調整交易：只需上傳【卡號】=【有效期】
        5. 本欄位為敏感性資料須符合PCI規範，若以TCP/IP 協定上傳之交易電文，本欄位須依照本中心規範透過tSAM進行卡號加密，加密方式請參閱附錄4.3
        6. 優惠兌換交易 送完整的Track2 Data，過U Card時，手輸仍帶11碼，過卡請FES帶連同授權9000給優惠平台
        7. Smart Pay卡號/帳號上傳格式(Field_35)不同一般信用卡。一般信用卡的卡號需要Pack成BCD code，Smart Pay的卡號/帳號直接上傳ASCII Code不進行Pack。

		Fiedld          Attribute       Byte    Value

		Length          n  2            1       Length of the data
		IssuerID        an 8            8       發卡單位代號
		PAN             an 16           16      帳號
		Remarks         b  240          30      備註欄

        e.g.:
	信用卡卡號 ＝ 4761739001010119 有效期（YYMM）=1512

	F_35=[0x21][0x47 0x61 0x73 0x90 0x01 0x01 0x01 0x19 0xD1 0x51 0x2F]
	(第 1 Byte[0x21]為卡號pack前的長度)

	Smart Pay 卡號=000009561234567890123456FFFFFFFFFFFFFFFFFFFFFFFFFFFFFF

	F_35 = [0x54][000009561234567890123456FFFFFFFFFFFFFFFFFFFFFFFFFFFFFF]
	(第 1 Byte[0x54]為ASCII卡號長度)

		交易上傳格式：【發卡單位代號】+【卡號/帳號】+【備註欄】
        */
	
	memset(szT2Data, 0x00, sizeof(szT2Data));
	memset(szFinalPAN, 0x00, sizeof(szFinalPAN));
	/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
	strcpy(szFinalPAN, pobTran->srBRec.szPAN);
	
	/* Manual keyin -> PAN + 'D' + Expire Date */
	if (pobTran->srBRec.uszManualBit == VS_TRUE)
	{
		strcpy(szT2Data, szFinalPAN);
		inPacketCnt += strlen(szFinalPAN);

		strcat(szT2Data, "D");
		
		inPacketCnt ++;
		memcpy(&szT2Data[inPacketCnt], &pobTran->srBRec.szExpDate[0], 4);
	}
	else
	{
		/* Message Type = 0220、0320、0400的交易或者是0100、0200的調整交易(e.g.:取消和小費)*/
		switch (pobTran->inISOTxnCode)
		{
			case _TIP_:
			case _TC_UPLOAD_ :
			case _ADVICE_ :
			case _VOID_ :
			case _CUP_PRE_COMP_ :
			case _REVERSAL_:
			case _PRE_COMP_ :
			case _REDEEM_ADJUST_ :
			case _INST_ADJUST_ :
			case _SALE_OFFLINE_:
			case _CUP_PRE_COMP_VOID_ :
			case _BATCH_UPLOAD_ :
			case _CUP_VOID_ :
			case _CUP_PRE_AUTH_VOID_ :
			case _SEND_ESC_ADVICE_ :
				/* 這邊只看Isotxncode，但是Loyaltu redeem的reversal仍然只送卡號和有效期，所以為特例 */
				if (pobTran->srBRec.inCode == _LOYALTY_REDEEM_ || pobTran->srBRec.inCode == _VOID_LOYALTY_REDEEM_)
				{
					strcpy(szT2Data, szFinalPAN);
					inPacketCnt += strlen(szFinalPAN);
					strcat(szT2Data, "D");
					inPacketCnt ++;
					memcpy(&szT2Data[inPacketCnt], &pobTran->srBRec.szExpDate[0], 4);
					inPacketCnt += 4;
				}
				else
				{
					strcpy(szT2Data, szFinalPAN);
					inPacketCnt += strlen(szFinalPAN);
					strcat(szT2Data, "D");
					inPacketCnt ++;
					memcpy(&szT2Data[inPacketCnt], &pobTran->srBRec.szExpDate[0], 4);
					inPacketCnt += 4;
					memcpy(&szT2Data[inPacketCnt], &pobTran->srBRec.szServiceCode[0], 3);
					inPacketCnt += 3;
				}
				break;
			case _LOYALTY_REDEEM_ :
				strcpy(szT2Data, szFinalPAN);
				inPacketCnt += strlen(szFinalPAN);
				strcat(szT2Data, "D");
				inPacketCnt ++;
				memcpy(&szT2Data[inPacketCnt], &pobTran->srBRec.szExpDate[0], 4);
				inPacketCnt += 4;
				break;
			default:
				memcpy(szT2Data, pobTran->szTrack2, strlen(pobTran->szTrack2));

				for (i = 0; i < strlen(szT2Data); i++)
				{
					if (szT2Data[i] == '=')
						szT2Data[i] = 'D';
				}

				break;
		} /* End switch () ... */

	}

	/* 先放長度再補0 */
	shLen = strlen(szT2Data);
	uszPackBuf[inCnt ++] = (shLen / 10 * 16) + (shLen % 10);

	/* 組MAC使用 */
	memset(gszMAC_F_35, 0x00, sizeof(gszMAC_F_35));
	memcpy(&gszMAC_F_35[0], &szT2Data[0], strlen(szT2Data));

	if (shLen % 2)
	{
		shLen ++;
		strcat(szT2Data, "0");
	}

	inFunc_ASCII_to_BCD(&uszPackBuf[inCnt], &szT2Data[0], (shLen / 2));
	inCnt += (shLen / 2);
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "F_35 [%d %s]", shLen, szT2Data);
		inLogPrintf(AT, szDebugMsg);
	}
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_Pack35() END!");

        return (inCnt);
}

/*
Function        :inNCCC_MFES_Pack35_tSAM
Date&Time       :2016/12/22 上午 10:49
Describe        :Field_35:   TRACK 2 Data
*/
int inNCCC_MFES_Pack35_tSAM(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        /* Field_35:	TRACK 2 Data */
	int		inCnt = 0, inPacketCnt = 0, i, inRetVal = -1;
	int		inLen;
	char		szT2Data[50 + 1];				/* 目前track2最長到32 */
	char		szFinalPAN[20 + 1];		/* 為了U CARD */
	char		szTemplate[100 + 1];
	char		szDebugMsg[100 + 1];
	char		szEncrypt[50 + 1];
	char		szField37[8 + 1], szField41[4 + 1];
	char		szAscii[84 + 1];
	unsigned char	uszSlot = 0;
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_Pack35_tSAM() START!");
	
	/* 抓tSAM Slot */
	inRetVal = inNCCC_tSAM_Decide_tSAM_Slot(&uszSlot);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inNCCC_tSAM_Decide_tSAM_Slot Failed");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	/*
                        補充說明：
        1. 本系統所有卡號一律透過ISO8583 Field_35上傳，不使用Field_02及Field_14。
        2. Message Type = 0100、0200
                A. 刷卡或插卡的交易：需上傳【完整Track 2】
                B. 原交易為刷卡或插卡的調整交易(例：取消或小費)：需上傳【卡號】=【有效期】+【Service Code】
                C. 人工輸入卡號的交易：只需上傳【卡號】=【有效期】
                D. 原交易為人工輸入卡號的調整交易：只需上傳【卡號】=【有效期】
        3. Message Type = 0220、0320、0400
                A. 刷卡或插卡的交易：需上傳【卡號】=【有效期】+【Service Code】
                B. 原交易為刷卡或插卡的調整交易(例：取消或小費)：需上傳【卡號】=【有效期】+【Service Code】
                C. 人工輸入卡號的交易：只需上傳【卡號】=【有效期】
                D. 原交易為人工輸入卡號的調整交易：只需上傳【卡號】=【有效期】
        5. 本欄位為敏感性資料須符合PCI規範，若以TCP/IP 協定上傳之交易電文，本欄位須依照本中心規範透過tSAM進行卡號加密，加密方式請參閱附錄4.3
        6. 優惠兌換交易 送完整的Track2 Data，過U Card時，手輸仍帶11碼，過卡請FES帶連同授權9000給優惠平台
        7. Smart Pay卡號/帳號上傳格式(Field_35)不同一般信用卡。一般信用卡的卡號需要Pack成BCD code，Smart Pay的卡號/帳號直接上傳ASCII Code不進行Pack。

		Fiedld          Attribute       Byte    Value

		Length          n  2            1       Length of the data
		IssuerID        an 8            8       發卡單位代號
		PAN             an 16           16      帳號
		Remarks         b  240          30      備註欄

        e.g.:
	信用卡卡號 ＝ 4761739001010119 有效期（YYMM）=1512

	F_35=[0x21][0x47 0x61 0x73 0x90 0x01 0x01 0x01 0x19 0xD1 0x51 0x2F]
	(第 1 Byte[0x21]為卡號pack前的長度)

	Smart Pay 卡號=000009561234567890123456FFFFFFFFFFFFFFFFFFFFFFFFFFFFFF

	F_35 = [0x54][000009561234567890123456FFFFFFFFFFFFFFFFFFFFFFFFFFFFFF]
	(第 1 Byte[0x54]為ASCII卡號長度)

		交易上傳格式：【發卡單位代號】+【卡號/帳號】+【備註欄】

	2015/1/14 下午 02:15:12 修正
	From: 吳升文
	Sent: Monday, January 12, 2015 7:40 PM

	近期發現Vx570集訊機ISO版有一個問題，同樣是16碼卡號的晶片，TC Upload電文中 F_35卡號長度有的交易帶24碼(16碼+等於符號1碼+有效期4碼+Service Code 3碼)是合理的，但是有些卻是帶25碼(Service Code多一碼)。
	因集訊機ISO會判斷卡號中等於符號後面的資料 <=7碼會就以manual keyin卡號的邏輯轉給Base24驗證MAC。
	而有問題交易因為卡號中等於符號後面的資料 為8碼，導致集訊機會誤判為有完整卡號，而轉完整卡號的邏輯給Base24驗證MAC，因此發生MAC Error的問題。
	問題是，TC UPLOAD交易等於符號後面應該是7碼怎麼會跑出8碼的資料。

	使用以下方式修正:
		strcpy(szT2Data, pobTran->srBRec.szPAN);
		inPacketCnt += strlen(pobTran->srBRec.szPAN);
		strcat(szT2Data, "D");
		inPacketCnt ++;
		memcpy(&szT2Data[inPacketCnt], &pobTran->srBRec.szExpDate[0], 4);
		inPacketCnt += 4;
		memcpy(&szT2Data[inPacketCnt], &pobTran->srBRec.szServiceCode[0], 3);
        */
 
	memset(szT2Data, 0x00, sizeof(szT2Data));
	memset(szFinalPAN, 0x00, sizeof(szFinalPAN));
	/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
	strcpy(szFinalPAN, pobTran->srBRec.szPAN);
	
	/* Manual keyin -> PAN + 'D' + Expire Date */
	if (pobTran->srBRec.uszManualBit == VS_TRUE)
	{
		
		strcpy(szT2Data, szFinalPAN);
		inPacketCnt += strlen(szFinalPAN);

		strcat(szT2Data, "D");
		inPacketCnt ++;
		memcpy(&szT2Data[inPacketCnt], &pobTran->srBRec.szExpDate[0], 4);
		inPacketCnt += 4;
	}
	else
	{
		/* Message Type = 0220、0320、0400的交易或者是0100、0200的調整交易(e.g.:取消和小費)*/
		switch (pobTran->inISOTxnCode)
		{
			case _TIP_:
			case _TC_UPLOAD_ :
			case _ADVICE_ :
			case _VOID_ :
			case _CUP_PRE_COMP_ :
			case _REVERSAL_:
			case _PRE_COMP_ :
			case _REDEEM_ADJUST_ :
			case _INST_ADJUST_ :
			case _SALE_OFFLINE_:
			case _CUP_PRE_COMP_VOID_ :
			case _BATCH_UPLOAD_ :
			case _CUP_VOID_ :
			case _CUP_PRE_AUTH_VOID_ :
			case _SEND_ESC_ADVICE_ :
				/* 這邊只看Isotxncode，但是Loyaltu redeem的reversal仍然只送卡號和有效期，所以為特例 */
				if (pobTran->srBRec.inCode == _LOYALTY_REDEEM_ || pobTran->srBRec.inCode == _VOID_LOYALTY_REDEEM_)
				{
					strcpy(szT2Data, szFinalPAN);
					inPacketCnt += strlen(szFinalPAN);
					strcat(szT2Data, "D");
					inPacketCnt ++;
					memcpy(&szT2Data[inPacketCnt], &pobTran->srBRec.szExpDate[0], 4);
					inPacketCnt += 4;
				}
				else
				{
					strcpy(szT2Data, szFinalPAN);
					inPacketCnt += strlen(szFinalPAN);
					strcat(szT2Data, "D");
					inPacketCnt ++;
					memcpy(&szT2Data[inPacketCnt], &pobTran->srBRec.szExpDate[0], 4);
					inPacketCnt += 4;
					memcpy(&szT2Data[inPacketCnt], &pobTran->srBRec.szServiceCode[0], 3);
					inPacketCnt += 3;
				}
				break;
			case _LOYALTY_REDEEM_ :
				strcpy(szT2Data, szFinalPAN);
				inPacketCnt += strlen(szFinalPAN);
				strcat(szT2Data, "D");
				inPacketCnt ++;
				memcpy(&szT2Data[inPacketCnt], &pobTran->srBRec.szExpDate[0], 4);
				inPacketCnt += 4;
				break;
			default:
				memcpy(szT2Data, pobTran->szTrack2, strlen(pobTran->szTrack2));

				for (i = 0; i < strlen(szT2Data); i++)
				{
					if (szT2Data[i] == '=')
						szT2Data[i] = 'D';
				}

				break;
		} /* End switch () ... */
		
	}

	inLen = strlen(szT2Data);
	/* 先放長度再補0 */
	uszPackBuf[inCnt ++] = (inLen / 10 * 16) + (inLen % 10);
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		inLogPrintf(AT, "F_35 [T2Data: ]");
		sprintf(szDebugMsg, "%s", szT2Data);
		inLogPrintf(AT, szDebugMsg);

		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "F_35 [T2Len: %d]", inLen);
		inLogPrintf(AT, szDebugMsg);
	}

	/* 組MAC使用 */
	memset(gszMAC_F_35, 0x00, sizeof(gszMAC_F_35));
	memcpy(&gszMAC_F_35[0], &szT2Data[0], strlen(szT2Data));

	if (inLen % 2)
	{
		inLen ++;
		strcat(szT2Data, "0");
	}

	if (ginDebug == VS_TRUE)
		inLogPrintf(AT, "inNCCC_MFES_Pack35_tSAM() 開始準備加密");

	/* 開始加密 */
	/* 組 Field_37 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(szTemplate, pobTran->srBRec.szRefNo, 11);
	inFunc_PAD_ASCII(szTemplate, szTemplate, 'F', 16, _PADDING_RIGHT_); /* 補【F】 */
	memset(szField37, 0x00, sizeof(szField37));
	inFunc_ASCII_to_BCD((unsigned char*)szField37, szTemplate, 8);

	if (ginDebug == VS_TRUE)
	{
		memset(szAscii, 0x00, sizeof(szAscii));	
		inFunc_BCD_to_ASCII(szAscii, (unsigned char*)szField37, 8);
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "F_35 [field37: %s] %d", szAscii, 8);
		inLogPrintf(AT, szDebugMsg);
	}

	/* 組 Field_41 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTerminalID(szTemplate);
	memset(szField41, 0x00, sizeof(szField41));
	inFunc_ASCII_to_BCD((unsigned char*)szField41, szTemplate, 4);

	if (ginDebug == VS_TRUE)
	{
		memset(szAscii, 0x00, sizeof(szAscii));
		inFunc_BCD_to_ASCII(szAscii, (unsigned char*)szField41, 4);
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "F_35 [field41: %s] %d", szAscii, 4);
		inLogPrintf(AT, szDebugMsg);
	}

	/* Field_35 */
	memset(gusztSAMCheckSum_35, 0x00, sizeof(gusztSAMCheckSum_35));
	memset(szEncrypt, 0x00, sizeof(szEncrypt));

	/* 空的部份全補F */
	if (inLen < 16)
	{
		for (i = inLen; i < 16; i ++)
		{
			szT2Data[i] = 0x46;
		}
		inLen = 16;

		inFunc_ASCII_to_BCD((unsigned char*)szEncrypt, szT2Data, inLen / 2);
	}
	else
		inFunc_ASCII_to_BCD((unsigned char*)szEncrypt, szT2Data, inLen / 2);

	if (ginDebug == VS_TRUE)
	{
		memset(szAscii, 0x00, sizeof(szAscii));
		inFunc_BCD_to_ASCII(szAscii, (unsigned char*)szEncrypt, inLen / 2);
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		inLogPrintf(AT, "F_35 [szEncrypt ]");
		sprintf(szDebugMsg, "%s L:%d", szAscii, inLen);
		inLogPrintf(AT, szDebugMsg);
	}

	memcpy((char *)&uszPackBuf[inCnt], szEncrypt, (inLen / 2));

	if (ginDebug == VS_TRUE)
		inLogPrintf(AT, "inNCCC_MFES_Pack35_tSAM() 開始加密");

	inRetVal = inNCCC_tSAM_Encrypt(uszSlot, 
				       inLen,
				       szEncrypt,
				       (unsigned char*)&szField37,
				       (unsigned char*)&szField41,
				       &gusztSAMKeyIndex,
				       gusztSAMCheckSum_35);

	if (inRetVal == VS_ERROR)
	{
		pobTran->inErrorMsg = _ERROR_CODE_V3_TSAM_ENCRYPT_FAIL_;
		return (VS_ERROR);
	}
	else
	{
		guszField_35 = VS_TRUE;
		memcpy((char *)&uszPackBuf[inCnt], szEncrypt, inRetVal);
		inCnt += (inLen / 2);
		if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof(szAscii));
			inFunc_BCD_to_ASCII(szAscii, (unsigned char*)uszPackBuf, strlen((char*)uszPackBuf));

			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "F_35 [szEncrypted  %s]", szAscii);
			inLogPrintf(AT, szDebugMsg);
		}

	}
		
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_Pack35_tSAM() END!");
	
	return (inCnt);
}

/*
Function        :inNCCC_MFES_Pack35_Software
Date&Time       :2016/10/4 上午 9:23
Describe        :Field_35:   TRACK 2 Data
*/
int inNCCC_MFES_Pack35_Software(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        /* Field_35:	TRACK 2 Data */
	int		inSoftwareKey = 0;
	int		inCnt = 0, inPacketCnt = 0, i;
	short		shLen;
	char		szT2Data[50 + 1];		/* 目前track2最長到32 */
	char		szFinalPAN[20 + 1];		/* 為了U CARD */
	char		szDebugMsg[100 + 1];
	char		szEncrypt[50 + 1];
	char		szAscii[84 + 1];
	char		szField11[3 + 1];
	char		szSTAN[6 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_Pack35_Software() START!");
	
	
	/*
                        補充說明：
        1. 本系統所有卡號一律透過ISO8583 Field_35上傳，不使用Field_02及Field_14。
        2. Message Type = 0100、0200
                A. 刷卡或插卡的交易：需上傳【完整Track 2】
                B. 原交易為刷卡或插卡的調整交易(例：取消或小費)：需上傳【卡號】=【有效期】+【Service Code】
                C. 人工輸入卡號的交易：只需上傳【卡號】=【有效期】
                D. 原交易為人工輸入卡號的調整交易：只需上傳【卡號】=【有效期】
        3. Message Type = 0220、0320、0400
                A. 刷卡或插卡的交易：需上傳【卡號】=【有效期】+【Service Code】
                B. 原交易為刷卡或插卡的調整交易(例：取消或小費)：需上傳【卡號】=【有效期】+【Service Code】
                C. 人工輸入卡號的交易：只需上傳【卡號】=【有效期】
                D. 原交易為人工輸入卡號的調整交易：只需上傳【卡號】=【有效期】
        5. 本欄位為敏感性資料須符合PCI規範，若以TCP/IP 協定上傳之交易電文，本欄位須依照本中心規範透過tSAM進行卡號加密，加密方式請參閱附錄4.3
        6. 優惠兌換交易 送完整的Track2 Data，過U Card時，手輸仍帶11碼，過卡請FES帶連同授權9000給優惠平台
        7. Smart Pay卡號/帳號上傳格式(Field_35)不同一般信用卡。一般信用卡的卡號需要Pack成BCD code，Smart Pay的卡號/帳號直接上傳ASCII Code不進行Pack。

		Fiedld          Attribute       Byte    Value

		Length          n  2            1       Length of the data
		IssuerID        an 8            8       發卡單位代號
		PAN             an 16           16      帳號
		Remarks         b  240          30      備註欄

        e.g.:
	信用卡卡號 ＝ 4761739001010119 有效期（YYMM）=1512

	F_35=[0x21][0x47 0x61 0x73 0x90 0x01 0x01 0x01 0x19 0xD1 0x51 0x2F]
	(第 1 Byte[0x21]為卡號pack前的長度)

	Smart Pay 卡號=000009561234567890123456FFFFFFFFFFFFFFFFFFFFFFFFFFFFFF

	F_35 = [0x54][000009561234567890123456FFFFFFFFFFFFFFFFFFFFFFFFFFFFFF]
	(第 1 Byte[0x54]為ASCII卡號長度)

		交易上傳格式：【發卡單位代號】+【卡號/帳號】+【備註欄】

	2015/1/14 下午 02:15:12 修正
	From: 吳升文
	Sent: Monday, January 12, 2015 7:40 PM

	近期發現Vx570集訊機ISO版有一個問題，同樣是16碼卡號的晶片，TC Upload電文中 F_35卡號長度有的交易帶24碼(16碼+等於符號1碼+有效期4碼+Service Code 3碼)是合理的，但是有些卻是帶25碼(Service Code多一碼)。
	因集訊機ISO會判斷卡號中等於符號後面的資料 <=7碼會就以manual keyin卡號的邏輯轉給Base24驗證MAC。
	而有問題交易因為卡號中等於符號後面的資料 為8碼，導致集訊機會誤判為有完整卡號，而轉完整卡號的邏輯給Base24驗證MAC，因此發生MAC Error的問題。
	問題是，TC UPLOAD交易等於符號後面應該是7碼怎麼會跑出8碼的資料。

	使用以下方式修正:
		strcpy(szT2Data, pobTran->srBRec.szPAN);
		inPacketCnt += strlen(pobTran->srBRec.szPAN);
		strcat(szT2Data, "D");
		inPacketCnt ++;
		memcpy(&szT2Data[inPacketCnt], &pobTran->srBRec.szExpDate[0], 4);
		inPacketCnt += 4;
		memcpy(&szT2Data[inPacketCnt], &pobTran->srBRec.szServiceCode[0], 3);
        */	
        
	memset(szT2Data, 0x00, sizeof(szT2Data));
	memset(szFinalPAN, 0x00, sizeof(szFinalPAN));
	/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
	strcpy(szFinalPAN, pobTran->srBRec.szPAN);
	
	/* Manual keyin -> PAN + 'D' + Expire Date */
	if (pobTran->srBRec.uszManualBit == VS_TRUE)
	{
		
		strcpy(szT2Data, szFinalPAN);
		inPacketCnt += strlen(szFinalPAN);

		strcat(szT2Data, "D");
		inPacketCnt ++;
		memcpy(&szT2Data[inPacketCnt], &pobTran->srBRec.szExpDate[0], 4);
		inPacketCnt += 4;
	}
	else
	{
		/* Message Type = 0220、0320、0400的交易或者是0100、0200的調整交易(e.g.:取消和小費)*/
		switch (pobTran->inISOTxnCode)
		{
			case _TIP_:
			case _TC_UPLOAD_ :
			case _ADVICE_ :
			case _VOID_ :
			case _CUP_PRE_COMP_ :
			case _REVERSAL_:
			case _PRE_COMP_ :
			case _REDEEM_ADJUST_ :
			case _INST_ADJUST_ :
			case _SALE_OFFLINE_:
			case _CUP_PRE_COMP_VOID_ :
			case _BATCH_UPLOAD_ :
			case _CUP_VOID_ :
			case _CUP_PRE_AUTH_VOID_ :
			case _SEND_ESC_ADVICE_ :
				/* 這邊只看Isotxncode，但是Loyaltu redeem的reversal仍然只送卡號和有效期，所以為特例 */
				if (pobTran->srBRec.inCode == _LOYALTY_REDEEM_ || pobTran->srBRec.inCode == _VOID_LOYALTY_REDEEM_)
				{
					strcpy(szT2Data, szFinalPAN);
					inPacketCnt += strlen(szFinalPAN);
					strcat(szT2Data, "D");
					inPacketCnt ++;
					memcpy(&szT2Data[inPacketCnt], &pobTran->srBRec.szExpDate[0], 4);
					inPacketCnt += 4;
				}
				else
				{
					strcpy(szT2Data, szFinalPAN);
					inPacketCnt += strlen(szFinalPAN);
					strcat(szT2Data, "D");
					inPacketCnt ++;
					memcpy(&szT2Data[inPacketCnt], &pobTran->srBRec.szExpDate[0], 4);
					inPacketCnt += 4;
					memcpy(&szT2Data[inPacketCnt], &pobTran->srBRec.szServiceCode[0], 3);
					inPacketCnt += 3;
				}
				break;
			case _LOYALTY_REDEEM_ :
				strcpy(szT2Data, szFinalPAN);
				inPacketCnt += strlen(szFinalPAN);
				strcat(szT2Data, "D");
				inPacketCnt ++;
				memcpy(&szT2Data[inPacketCnt], &pobTran->srBRec.szExpDate[0], 4);
				inPacketCnt += 4;
				break;
			default:
				memcpy(szT2Data, pobTran->szTrack2, strlen(pobTran->szTrack2));

				for (i = 0; i < strlen(szT2Data); i++)
				{
					if (szT2Data[i] == '=')
						szT2Data[i] = 'D';
				}

				break;
		} /* End switch () ... */
		
	}

	shLen = strlen(szT2Data);
	/* 先放長度再補0 */
	uszPackBuf[inCnt ++] = (shLen / 10 * 16) + (shLen % 10);
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		inLogPrintf(AT, "F_35 [T2Data: ]");
		sprintf(szDebugMsg, "%s", szT2Data);
		inLogPrintf(AT, szDebugMsg);

		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "F_35 [T2Len: %d]", shLen);
		inLogPrintf(AT, szDebugMsg);
	}
	
	/* 組MAC使用 */
	memset(gszMAC_F_35, 0x00, sizeof(gszMAC_F_35));
	memcpy(&gszMAC_F_35[0], &szT2Data[0], strlen(szT2Data));

	if (shLen % 2)
	{
		shLen ++;
		strcat(szT2Data, "0");
	}

	if (ginDebug == VS_TRUE)
		inLogPrintf(AT, "inNCCC_MFES_Pack35_Software() 開始準備加密");

	/* 開始加密 */
	/* Field_35 */
	memset(szEncrypt, 0x00, sizeof(szEncrypt));
	inFunc_ASCII_to_BCD((unsigned char*)szEncrypt, szT2Data, shLen / 2);
	memset(szField11, 0x00, sizeof(szField11));
	memset(szSTAN, 0x00, sizeof(szSTAN));

        if (pobTran->inISOTxnCode == _SEND_ESC_ADVICE_)
	{
                /* For ESC電子簽單，Advice 使用當下STAN */
		inGetSTANNum(szSTAN);
	}
	else
	{
		sprintf(szSTAN, "%06ld", pobTran->srBRec.lnSTANNum);
        }
	
	inFunc_ASCII_to_BCD((unsigned char*)szField11, szSTAN, 3);
	inSoftwareKey = (unsigned char)szField11[2] & 0x0F;
	
	if (ginDebug == VS_TRUE)
		inLogPrintf(AT, "inNCCC_MFES_Pack35_Software() 開始加密");
	
	for (i = 0; i < (shLen / 2); i ++)
	{
		szEncrypt[i] = szEncrypt[i] ^ inSoftwareKey;
		inSoftwareKey ++;

		if (inSoftwareKey > 9)
			inSoftwareKey = 0;
	}
	
	memcpy((char *)&uszPackBuf[inCnt], szEncrypt, (shLen / 2));

	inCnt += (shLen / 2);
	
	if (ginDebug == VS_TRUE)
	{
		memset(szAscii, 0x00, sizeof(szAscii));
		inFunc_BCD_to_ASCII(szAscii, (unsigned char*)uszPackBuf, strlen((char*)uszPackBuf));

		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "F_35 [szEncrypted  %s]", szAscii);
		inLogPrintf(AT, szDebugMsg);
	}

	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_Pack35_Software() END!");
	
	return (inCnt);
}

/*
Function        :inNCCC_MFES_Pack37
Date&Time       :2016/12/22 上午 10:49
Describe        :Field_37:	Retrieval Reference Number (RRN)
*/
int inNCCC_MFES_Pack37(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 	inCnt = 0;
	char	szDebugMsg[100 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_Pack37() START!");
	
	/* 靠inNCCC_Func_MakeRefNo來組 */
	
        memcpy((char *) &uszPackBuf[inCnt], pobTran->srBRec.szRefNo, 12);
        inCnt += 12;

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "F_37 [RRN %s]", pobTran->srBRec.szRefNo);
		inLogPrintf(AT, szDebugMsg);
	}
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_Pack37() END!");
	
        return (inCnt);
}

/*
Function        :inNCCC_MFES_Pack38
Date&Time       :2016/12/22 上午 10:49
Describe        :Field_38:	Authorization Identification Response（SmartPay交易無授權碼）
*/
int inNCCC_MFES_Pack38(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 	inCnt = 0;
        char    szAuthCode[_AUTH_CODE_SIZE_ + 1];
	char	szDebugMsg[100 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_Pack38() START!");


        /* 確認是否授權碼是合法字元 */
        if (inFunc_Check_AuthCode_Validate(pobTran->srBRec.szAuthCode) != VS_SUCCESS)
	{
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, _ERR_AUTHCODE_);
		srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
		srDispMsgObj.inMsgType = _NO_KEY_MSG_;
		srDispMsgObj.inTimeout = 2;
		strcpy(srDispMsgObj.szErrMsg1, "");
		srDispMsgObj.inErrMsg1Line = 0;
		inDISP_Msg_BMP(&srDispMsgObj);
		return (VS_ERROR);
	}

	/* 【需求單-112268】1.避免特店因退貨交易未輸入授權碼導致剔退 僅退貨不檢核 by Russell 2024/3/21 下午 5:10 */
	if (pobTran->srBRec.inCode == _REFUND_)
	{
		
	}
	else
	{
		/* 授權碼不能全0或全空白，除了CUP */
		if ((!memcmp(pobTran->srBRec.szAuthCode, "000000", 6) || !memcmp(pobTran->srBRec.szAuthCode, "      ", 6)) && pobTran->srBRec.uszCUPTransBit != VS_TRUE)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Auth code全零或全空白%s", pobTran->srBRec.szAuthCode);
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
	}

        memset(szAuthCode, 0x00, sizeof(szAuthCode));
	strcpy(szAuthCode, pobTran->srBRec.szAuthCode);
	/* 若沒輸入滿6個字元，則右邊用空白填滿*/
        inFunc_PAD_ASCII(szAuthCode, szAuthCode, ' ', 6, _PADDING_RIGHT_);

        memcpy((char *) &uszPackBuf[inCnt], &szAuthCode[0], _AUTH_CODE_SIZE_);
        inCnt += _AUTH_CODE_SIZE_;

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "F_38 [AUTH CODE %s]", pobTran->srBRec.szAuthCode);
		inLogPrintf(AT, szDebugMsg);
	}
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_Pack38() END!");
	
        return (inCnt);
}

/*
Function        :inNCCC_MFES_Pack41
Date&Time       :2016/12/22 上午 10:48
Describe        :Field_41:	Card Acceptor Terminal Identification（TID）補充說明：端末機代號需可支援英、數字。
*/
int inNCCC_MFES_Pack41(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 	inCnt = 0;
        char 	szTemplate[8 + 1];
	char	szDebugMsg[100 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_Pack41() START!");
	

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
                inLogPrintf(AT, "inNCCC_MFES_Pack41() END!");

        return (inCnt);
}

/*
Function        :inNCCC_MFES_Pack42
Date&Time       :2016/12/22 上午 10:48
Describe        :Field_42:	Card Acceptor Identification Code(MID)
 		補充說明：
		1. 商店代號需左靠右補空白。
		2. 語音開卡交易須使用固定商店代號 0122900410 。
*/
int inNCCC_MFES_Pack42(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 	inCnt = 0;
        char	szTemplate[16 + 1];
	char 	szDebugMsg[100 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_Pack42() START!");
	
	
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
                inLogPrintf(AT, "inNCCC_MFES_Pack42() END!");

        return (inCnt);
}

/*
Function        :inNCCC_MFES_Pack48
Date&Time       :2016/12/22 上午 10:48
Describe        :Field_48:	Additional Data - Private Use
*/
int inNCCC_MFES_Pack48(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int	i;
        int	inCnt = 0;
	char	szDebugMsg[100 + 1];
	char	szTemplate[30 + 1];
	char	szAscii[100 + 1];
	char	szCustomerIndicator[3 + 1] = {0};
	char	szPOS_ID[6 + 1] = {0};
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_Pack48() START!");

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
		memcpy(szTemplate, pobTran->srBRec.szStoreID, 18);
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
                inLogPrintf(AT, "inNCCC_MFES_Pack48() END!");

	return (inCnt);
}

/*
Function        :inNCCC_MFES_Pack52
Date&Time       :2016/12/22 上午 10:48
Describe        :Field_52:	PinBlock Data
*/
int inNCCC_MFES_Pack52(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int     inCnt = 0;
	char	szDebugMsg[100 + 1];

	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_Pack52() START!");
	
	memcpy(&uszPackBuf[inCnt], pobTran->szPIN, _PIN_SIZE_);
	inCnt += 8;

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "F_52 [PinDATA %s]", pobTran->szPIN);
		inLogPrintf(AT, szDebugMsg);
	}
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_Pack52() END!");
	
	return (inCnt);
}

/*
Function        :inNCCC_MFES_Pack54
Date&Time       :2016/12/22 上午 10:48
Describe        :Field_54:	Additional Amounts(此規格放Tip)
*/
int inNCCC_MFES_Pack54(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 	inCnt = 0;
	char	szAscii[16 + 1];
	char	szDebugMsg[100 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_Pack54() START!");

        /* Tip Amount */
        uszPackBuf[inCnt++] = 0x00;
        uszPackBuf[inCnt++] = 0x12;
        sprintf((char *) &uszPackBuf[inCnt], "%010ld00", pobTran->srBRec.lnTipTxnAmount);
        inCnt += 12;
	
	if (ginDebug == VS_TRUE)
	{
		memset(szAscii, 0x00, sizeof(szAscii));
		inFunc_BCD_to_ASCII(szAscii, &uszPackBuf[inCnt - 14], 2);
			
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "F_54 [Tip %s %010ld00]", szAscii, pobTran->srBRec.lnTipTxnAmount);
		inLogPrintf(AT, szDebugMsg);
	}
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_Pack54() END!");
	
        return (inCnt);
}

/*
Function        :inNCCC_MFES_Pack55
Date&Time       :2016/12/23 上午 11:54
Describe        :Field_55:	EMV Chip Data
*/
int inNCCC_MFES_Pack55(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int		inBitMapIndex;
	int		inCnt = 0, inPacketCnt = 0;
	int		inContactlessType = 0;
	char		szASCII[128 + 1];
	char		szPacket[512 + 1];
	char		szDebugMsg[100 + 1];
        unsigned char	uszBCD[2 + 1];

	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_Pack55() START!");

	memset(szPacket, 0x00, sizeof(szPacket));
	/*
		(1) Tag 9F5B若該筆晶片交易有執行Issure Script Update，除原本TC Upload要上傳 9F5B(ISR)之外，該筆交易的取消或小費交易也要上傳此 Tag。
		(2) Fallback交易DE55僅需上傳Tag DFED、Tag DFEE及Tag DFEF。
		(3) C1:晶片卡 Fallback 的小費交易比照原交易需上傳Tag DFED、Tag DFEE及Tag DFEF。
		(4) Tag 9F63 若UICC卡片包含此Tag時，EDC則應支援該Tag上傳。

		感應交易上傳之 Tag 9F33 的 Value 規則如下：
		(1) VISA Paywave 1/VISA Paywave 3/JSpeedy 請上傳Terminal 之 EMV 參數（因為 V & J 感應交易Reader 不會回 Tag 9F33，故以端末機EMV參數上傳）。
		(2) Paypass 因為 Reader 會回覆 Tag 9F33 ，所以依Reader 回覆資料上傳。
		(3) (O) option 欄位，若是Reader有回覆則必須上傳。
		(4) GHL V3R6 Reader感應Paypass不會回覆Tag9F03，所以此Tag依Reader實際回覆資料上傳。
		UICC的void sale 和void preauth 不上傳此欄位。
	*/
	
	/* 因為把F55的檢查寫的較嚴格，所以沒考慮到Reversal的狀況，所以若Reversal跑到F55時，先換回原MTI (START) */
	if (pobTran->inISOTxnCode == _REVERSAL_)
	{
		for (inBitMapIndex = 0;; inBitMapIndex++)
		{
			if (srNCCC_MFES_ISOBitMap[inBitMapIndex].inTxnID == _NCCC_MFES_NULL_TX_)
			{
				return (VS_ERROR); /* 找不到相對應交易別的Bit Map */
			}

			if (srNCCC_MFES_ISOBitMap[inBitMapIndex].inTxnID == pobTran->inTransactionCode)
				break; /* 找到一樣的交易類別 */
		}
		
		memcpy(guszMFES_MTI, srNCCC_MFES_ISOBitMap[inBitMapIndex].szMTI, 4);
	}
	/* 因為把F55的檢查寫的較嚴格，所以沒考慮到Reversal的狀況，所以若Reversal跑到F55時，先換回原MTI (END) */
		
	if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
	{
		switch (pobTran->srBRec.uszWAVESchemeID)
		{
			case SCHEME_ID_20_PAYPASS_MAG_STRIPE:
			case SCHEME_ID_21_PAYPASS_MCHIP:
				inContactlessType = 1;
				break;
			case SCHEME_ID_16_WAVE1:
				inContactlessType = 2;
				break;
			case SCHEME_ID_17_WAVE3:
				inContactlessType = 3;
				break;
			case SCHEME_ID_61_JSPEEDY:
			case SCHEME_ID_65_NEWJSPEEDY_LEGACY:	/* NCCC OLDJ 和NEW J C卡分同一類 a*/
				inContactlessType = 4;
				break;
			case SCHEME_ID_63_NEWJSPEEDY_EMV:
				inContactlessType = 5;
				break;
			case SCHEME_ID_91_QUICKPASS :	/* 0x91 */
				inContactlessType = 6;
				break;
			case SCHEME_ID_50_EXPRESSSPAY :
			case SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE:
				inContactlessType = 7;
				break;
			case SCHEME_ID_42_DPAS :
			case SCHEME_ID_43_DISCOVER_DPAS_MAG_STRIPE :
				inContactlessType = 8;
				break;
			default :
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "F55_未支援的感應卡別");
				}
				inUtility_StoreTraceLog_OneStep("F55_未支援的感應卡別");
				return (VS_ERROR);
				break;
		}
		
		/* 感應只有paypass要送，正向送57，其他符合0200、0220、0320、0400的話送5A和5F24 */
		if (inContactlessType == 1)
		{
			/* 57 */
			/* 比照520 2020/2/7 下午 4:50 */
			if (pobTran->uszTCUploadBit != VS_TRUE		&&
		            (pobTran->inISOTxnCode == _SALE_		||	/* 原來只判斷MTI，但退貨和取消不用，所以下面用加了用ISOTXNCODE來判別 */
		             pobTran->inISOTxnCode == _PRE_AUTH_	||
			     pobTran->inISOTxnCode == _REDEEM_SALE_	||
		             pobTran->inISOTxnCode == _INST_SALE_	||
	                     pobTran->inISOTxnCode == _CUP_SALE_	||
			     pobTran->inISOTxnCode == _CUP_PRE_AUTH_))
			{
				if (pobTran->in57_Track2Len <= 0)
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "F 55 TAG 57 ERROR!!");
						inLogPrintf(AT, szDebugMsg);
					}
					inUtility_StoreTraceLog_OneStep("F 55 TAG 57 ERROR!!");

					return (VS_ERROR);
				}

				szPacket[inPacketCnt ++] = 0x57;
				szPacket[inPacketCnt ++] = pobTran->in57_Track2Len;
				memcpy(&szPacket[inPacketCnt], pobTran->usz57_Track2, pobTran->in57_Track2Len);
				inPacketCnt += pobTran->in57_Track2Len;
			}
			else
			{
				/* 5A */
				/* 比照520 2020/2/7 下午 4:50 */
				if (!memcmp(guszMFES_MTI, "0200", 4)	||
				    !memcmp(guszMFES_MTI, "0220", 4)	||
				    !memcmp(guszMFES_MTI, "0320", 4)	||
				    !memcmp(guszMFES_MTI, "0400", 4))
				{
					if (pobTran->srEMVRec.in5A_ApplPanLen <= 0)
					{
						if (ginDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
							sprintf(szDebugMsg, "F 55 TAG 5A ERROR!!");
							inLogPrintf(AT, szDebugMsg);
						}
						inUtility_StoreTraceLog_OneStep("F 55 TAG 5A ERROR!!");

						return (VS_ERROR);
					}

					szPacket[inPacketCnt ++] = 0x5A;
					szPacket[inPacketCnt ++] = pobTran->srEMVRec.in5A_ApplPanLen;
					memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz5A_ApplPan[0], pobTran->srEMVRec.in5A_ApplPanLen);
					inPacketCnt += pobTran->srEMVRec.in5A_ApplPanLen;
				}

				/* 5F24 */
				/* 比照520 2020/2/7 下午 4:50 */
				if (inContactlessType == 1)
				{
					if (!memcmp(guszMFES_MTI, "0200", 4)	||
					    !memcmp(guszMFES_MTI, "0220", 4)	||
					    !memcmp(guszMFES_MTI, "0320", 4)	||
					    !memcmp(guszMFES_MTI, "0400", 4))
					{
						if (pobTran->srEMVRec.in5F24_ExpireDateLen <= 0)
						{
							if (ginDebug == VS_TRUE)
							{
								memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
								sprintf(szDebugMsg, "F 55 TAG 5F24 ERROR!!");
								inLogPrintf(AT, szDebugMsg);
							}
							inUtility_StoreTraceLog_OneStep("F 55 TAG 5F24 ERROR!!");

							return (VS_ERROR);
						}

						szPacket[inPacketCnt ++] = 0x5F;
						szPacket[inPacketCnt ++] = 0x24;
						szPacket[inPacketCnt ++] = pobTran->srEMVRec.in5F24_ExpireDateLen;
						memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz5F24_ExpireDate[0], pobTran->srEMVRec.in5F24_ExpireDateLen);
						inPacketCnt += pobTran->srEMVRec.in5F24_ExpireDateLen;
					}
				}
			}
		}
		
		/* 5F2A */
		if (pobTran->srEMVRec.in5F2A_TransCurrCodeLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 5F2A ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}
			inUtility_StoreTraceLog_OneStep("F 55 TAG 5F2A ERROR!!");

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x5F;
		szPacket[inPacketCnt ++] = 0x2A;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in5F2A_TransCurrCodeLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz5F2A_TransCurrCode[0], pobTran->srEMVRec.in5F2A_TransCurrCodeLen);
		inPacketCnt += pobTran->srEMVRec.in5F2A_TransCurrCodeLen;
                
		/* 5F34 */
		if (inContactlessType == 1	||
		    inContactlessType == 2	||
		    inContactlessType == 4)
		{
			if (pobTran->srEMVRec.in5F34_ApplPanSeqnumLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x5F;
				szPacket[inPacketCnt ++] = 0x34;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in5F34_ApplPanSeqnumLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz5F34_ApplPanSeqnum[0], pobTran->srEMVRec.in5F34_ApplPanSeqnumLen);
				inPacketCnt += pobTran->srEMVRec.in5F34_ApplPanSeqnumLen;
			}
		}
		else if (inContactlessType == 3)
		{
			if (pobTran->srEMVRec.in5F34_ApplPanSeqnumLen <= 0)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "F 55 TAG 5F34 ERROR!!");
					inLogPrintf(AT, szDebugMsg);
				}
				inUtility_StoreTraceLog_OneStep("F 55 TAG 5F34 ERROR!!");

				return (VS_ERROR);
			}
			szPacket[inPacketCnt ++] = 0x5F;
			szPacket[inPacketCnt ++] = 0x34;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in5F34_ApplPanSeqnumLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz5F34_ApplPanSeqnum[0], pobTran->srEMVRec.in5F34_ApplPanSeqnumLen);
			inPacketCnt += pobTran->srEMVRec.in5F34_ApplPanSeqnumLen;
		}
		/* 規格沒寫抄Verfone的，有才送 */
		else
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "SPEC not specific");
			}
			
			if (pobTran->srEMVRec.in5F34_ApplPanSeqnumLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x5F;
				szPacket[inPacketCnt ++] = 0x34;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in5F34_ApplPanSeqnumLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz5F34_ApplPanSeqnum[0], pobTran->srEMVRec.in5F34_ApplPanSeqnumLen);
				inPacketCnt += pobTran->srEMVRec.in5F34_ApplPanSeqnumLen;
			}
		}
		
		/* 82 */
		if (pobTran->srEMVRec.in82_AIPLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 82 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}
			inUtility_StoreTraceLog_OneStep("F 55 TAG 82 ERROR!!");

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x82;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in82_AIPLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz82_AIP[0], pobTran->srEMVRec.in82_AIPLen);
		inPacketCnt += pobTran->srEMVRec.in82_AIPLen;
	
		/* 84 */
		if (inContactlessType == 1)
		{
			if (pobTran->srEMVRec.in84_DFNameLen <= 0)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "F 55 TAG 84 ERROR!!");
					inLogPrintf(AT, szDebugMsg);
				}
				inUtility_StoreTraceLog_OneStep("F 55 TAG 84 ERROR!!");

				return (VS_ERROR);
			}
			szPacket[inPacketCnt ++] = 0x84;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in84_DFNameLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz84_DF_NAME[0], pobTran->srEMVRec.in84_DFNameLen);
			inPacketCnt += pobTran->srEMVRec.in84_DFNameLen;
		}
		else if (inContactlessType == 2	||
			 inContactlessType == 4 ||
			 inContactlessType == 5)
		{
			if (!memcmp(guszMFES_MTI, "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
			{
				/* JSPEEDY NEWJ WAVE1 OFFLINE APPROVED不送 */
			}
			else
			{
				if (pobTran->srEMVRec.in84_DFNameLen > 0)
				{
					szPacket[inPacketCnt ++] = 0x84;
					szPacket[inPacketCnt ++] = pobTran->srEMVRec.in84_DFNameLen;
					memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz84_DF_NAME[0], pobTran->srEMVRec.in84_DFNameLen);
					inPacketCnt += pobTran->srEMVRec.in84_DFNameLen;
				}
			}
		}
		else if (inContactlessType == 3)
		{
			if (pobTran->srEMVRec.in84_DFNameLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x84;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in84_DFNameLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz84_DF_NAME[0], pobTran->srEMVRec.in84_DFNameLen);
				inPacketCnt += pobTran->srEMVRec.in84_DFNameLen;
			}
		}
		/* 規格沒寫抄Verfone的，有才送 */
		else
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "SPEC not specific");
			}
			
			if (pobTran->srEMVRec.in84_DFNameLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x84;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in84_DFNameLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz84_DF_NAME[0], pobTran->srEMVRec.in84_DFNameLen);
				inPacketCnt += pobTran->srEMVRec.in84_DFNameLen;
			}
		}

		/* 8A */
		if (inContactlessType == 1)
		{
			if (!memcmp(guszMFES_MTI, "0220", 4) || !memcmp(guszMFES_MTI, "0320", 4))
			{
				if (pobTran->srEMVRec.in8A_AuthRespCodeLen <= 0)
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "F 55 TAG 8A ERROR!!");
						inLogPrintf(AT, szDebugMsg);
					}
					inUtility_StoreTraceLog_OneStep("F 55 TAG 8A ERROR!!");

					return (VS_ERROR);
				}
				szPacket[inPacketCnt ++] = 0x8A;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in8A_AuthRespCodeLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz8A_AuthRespCode[0], pobTran->srEMVRec.in8A_AuthRespCodeLen);
				inPacketCnt += pobTran->srEMVRec.in8A_AuthRespCodeLen;
			}
		
		}
		else if (inContactlessType == 2	||
			 inContactlessType == 4 ||
			 inContactlessType == 5)
		{
			if (!memcmp(guszMFES_MTI, "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
			{
				/* WAVE1 JSPEEDY NEWJ OFFLINE APPROVED不送 */
			}
			else if (!memcmp(guszMFES_MTI, "0200", 4))
			{
				/* WAVE1 JSPEEDY NEWJ 0200不送 */
			}
			else
			{
				if (pobTran->srEMVRec.in8A_AuthRespCodeLen > 0)
				{
					szPacket[inPacketCnt ++] = 0x8A;
					szPacket[inPacketCnt ++] = pobTran->srEMVRec.in8A_AuthRespCodeLen;
					memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz8A_AuthRespCode[0], pobTran->srEMVRec.in8A_AuthRespCodeLen);
					inPacketCnt += pobTran->srEMVRec.in8A_AuthRespCodeLen;
				}
			}
			
		}
		else if (inContactlessType == 3	||
			 inContactlessType == 6	||
			 inContactlessType == 7	||
			 inContactlessType == 8)
		{
			if (!memcmp(guszMFES_MTI, "0200", 4))
			{
				/* WAVE 3 QUICKPASS 不送 */
			}
			else
			{
				if (pobTran->srEMVRec.in8A_AuthRespCodeLen > 0)
				{
					szPacket[inPacketCnt ++] = 0x8A;
					szPacket[inPacketCnt ++] = pobTran->srEMVRec.in8A_AuthRespCodeLen;
					memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz8A_AuthRespCode[0], pobTran->srEMVRec.in8A_AuthRespCodeLen);
					inPacketCnt += pobTran->srEMVRec.in8A_AuthRespCodeLen;
				}
			}
		}
		/* 規格沒寫抄Verfone的，有才送 */
		else
		{	
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "SPEC not specific");
			}
			if (pobTran->srEMVRec.in8A_AuthRespCodeLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x8A;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in8A_AuthRespCodeLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz8A_AuthRespCode[0], pobTran->srEMVRec.in8A_AuthRespCodeLen);
				inPacketCnt += pobTran->srEMVRec.in8A_AuthRespCodeLen;
			}
		}
		
		/* 95 */
		if (pobTran->srEMVRec.in95_TVRLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 95 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}
			inUtility_StoreTraceLog_OneStep("F 55 TAG 95 ERROR!!");

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x95;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in95_TVRLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz95_TVR[0], pobTran->srEMVRec.in95_TVRLen);
		inPacketCnt += pobTran->srEMVRec.in95_TVRLen;

		/* 9A */
		if (pobTran->srEMVRec.in9A_TranDateLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9A ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}
			inUtility_StoreTraceLog_OneStep("F 55 TAG 9A ERROR!!");

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9A;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9A_TranDateLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9A_TranDate[0], pobTran->srEMVRec.in9A_TranDateLen);
		inPacketCnt += pobTran->srEMVRec.in9A_TranDateLen;
		
		/* 9B */
		if (inContactlessType == 1)
		{
//			if (pobTran->srEMVRec.in9B_TSILen <= 0)
//			{
//				if (ginDebug == VS_TRUE)
//				{
//					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
//					sprintf(szDebugMsg, "F 55 TAG 9B ERROR!!");
//					inLogPrintf(AT, szDebugMsg);
//				}
//				inUtility_StoreTraceLog_OneStep("F 55 TAG 9B ERROR!!");
//
//				return (VS_ERROR);
//			}
			/* 抄Verifone有才送，ATS(含MCAP_ATS)非接觸式交易，目前不會檢查Tag 9B為必要欄位 */
			if (pobTran->srEMVRec.in9B_TSILen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9B;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9B_TSILen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9B_TSI[0], pobTran->srEMVRec.in9B_TSILen);
				inPacketCnt += pobTran->srEMVRec.in9B_TSILen;
			}
		}
		else if (inContactlessType == 2	||
			 inContactlessType == 4 ||
			 inContactlessType == 5)
		{
			if (!memcmp(guszMFES_MTI, "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
			{
				/* JSPEEDY NEWJ WAVE1 OFFLINE APPROVED不送 */
			}
			else
			{
				if (pobTran->srEMVRec.in9B_TSILen > 0)
				{
					szPacket[inPacketCnt ++] = 0x9B;
					szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9B_TSILen;
					memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9B_TSI[0], pobTran->srEMVRec.in9B_TSILen);
					inPacketCnt += pobTran->srEMVRec.in9B_TSILen;
				}
			}
		}
		else if (inContactlessType == 3)
		{
			if (pobTran->srEMVRec.in9B_TSILen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9B;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9B_TSILen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9B_TSI[0], pobTran->srEMVRec.in9B_TSILen);
				inPacketCnt += pobTran->srEMVRec.in9B_TSILen;
			}
		}
		/* 規格沒寫抄Verfone的，有才送 */
		else
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "SPEC not specific");
			}
			
			if (pobTran->srEMVRec.in9B_TSILen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9B;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9B_TSILen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9B_TSI[0], pobTran->srEMVRec.in9B_TSILen);
				inPacketCnt += pobTran->srEMVRec.in9B_TSILen;
			}
		}
		
		/* 9C */
		if (pobTran->srEMVRec.in9C_TranTypeLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9C ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}
			inUtility_StoreTraceLog_OneStep("F 55 TAG 9C ERROR!!");

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9C;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9C_TranTypeLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9C_TranType[0], pobTran->srEMVRec.in9C_TranTypeLen);
		inPacketCnt += pobTran->srEMVRec.in9C_TranTypeLen;

		/* 9F02 */
		if (pobTran->srEMVRec.in9F02_AmtAuthNumLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9F02 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}
			inUtility_StoreTraceLog_OneStep("F 55 TAG 9F02 ERROR!!");

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9F;
		szPacket[inPacketCnt ++] = 0x02;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F02_AmtAuthNumLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F02_AmtAuthNum[0], pobTran->srEMVRec.in9F02_AmtAuthNumLen);
		inPacketCnt += pobTran->srEMVRec.in9F02_AmtAuthNumLen;
		
		/* 9F03 */
		if (pobTran->srEMVRec.in9F03_AmtOtherNumLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x03;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F03_AmtOtherNumLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F03_AmtOtherNum[0], pobTran->srEMVRec.in9F03_AmtOtherNumLen);
			inPacketCnt += pobTran->srEMVRec.in9F03_AmtOtherNumLen;
		}

		/* 9F10 */
		if (pobTran->srEMVRec.in9F10_IssuerAppDataLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9F10 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}
			inUtility_StoreTraceLog_OneStep("F 55 TAG 9F10 ERROR!!");

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9F;
		szPacket[inPacketCnt ++] = 0x10;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F10_IssuerAppDataLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F10_IssuerAppData[0], pobTran->srEMVRec.in9F10_IssuerAppDataLen);
		inPacketCnt += pobTran->srEMVRec.in9F10_IssuerAppDataLen;
		

		/* 9F1A */
		if (pobTran->srEMVRec.in9F1A_TermCountryCodeLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9F1A ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}
			inUtility_StoreTraceLog_OneStep("F 55 TAG 9F1A ERROR!!");

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9F;
		szPacket[inPacketCnt ++] = 0x1A;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F1A_TermCountryCodeLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F1A_TermCountryCode[0], pobTran->srEMVRec.in9F1A_TermCountryCodeLen);
		inPacketCnt += pobTran->srEMVRec.in9F1A_TermCountryCodeLen;
		
		/* 9F1E */
		if (inContactlessType == 1)
		{
			if (pobTran->srEMVRec.in9F1E_IFDNumLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x1E;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F1E_IFDNumLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F1E_IFDNum[0], pobTran->srEMVRec.in9F1E_IFDNumLen);
				inPacketCnt += pobTran->srEMVRec.in9F1E_IFDNumLen;
			}
			
		}
		else if (inContactlessType == 2	||
			 inContactlessType == 4 ||
			 inContactlessType == 5)
		{
			if (!memcmp(guszMFES_MTI, "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
			{
				/* JSPEEDY NEWJ WAVE1 OFFLINE APPROVED不送 */
			}
			else
			{
				if (pobTran->srEMVRec.in9F1E_IFDNumLen > 0)
				{
					szPacket[inPacketCnt ++] = 0x9F;
					szPacket[inPacketCnt ++] = 0x1E;
					szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F1E_IFDNumLen;
					memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F1E_IFDNum[0], pobTran->srEMVRec.in9F1E_IFDNumLen);
					inPacketCnt += pobTran->srEMVRec.in9F1E_IFDNumLen;
				}
			}
		}
		else if (inContactlessType == 3)
		{
			if (pobTran->srEMVRec.in9F1E_IFDNumLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x1E;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F1E_IFDNumLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F1E_IFDNum[0], pobTran->srEMVRec.in9F1E_IFDNumLen);
				inPacketCnt += pobTran->srEMVRec.in9F1E_IFDNumLen;
			}
		}
		/* 規格沒寫抄Verfone的，有才送 */
		else
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "SPEC not specific");
			}
			
			if (pobTran->srEMVRec.in9F1E_IFDNumLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x1E;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F1E_IFDNumLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F1E_IFDNum[0], pobTran->srEMVRec.in9F1E_IFDNumLen);
				inPacketCnt += pobTran->srEMVRec.in9F1E_IFDNumLen;
			}
		
		}

		/* 9F26 */
		if (pobTran->srEMVRec.in9F26_ApplCryptogramLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9F26 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}
			inUtility_StoreTraceLog_OneStep("F 55 TAG 9F26 ERROR!!");

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9F;
		szPacket[inPacketCnt ++] = 0x26;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F26_ApplCryptogramLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F26_ApplCryptogram[0], pobTran->srEMVRec.in9F26_ApplCryptogramLen);
		inPacketCnt += pobTran->srEMVRec.in9F26_ApplCryptogramLen;
		
		/* 9F27 */
		if (inContactlessType == 1)
		{
			if (pobTran->srEMVRec.in9F27_CIDLen <= 0)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "F 55 TAG 9F27 ERROR!!");
					inLogPrintf(AT, szDebugMsg);
				}
				inUtility_StoreTraceLog_OneStep("F 55 TAG 9F27 ERROR!!");

				return (VS_ERROR);
			}
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x27;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F27_CIDLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F27_CID[0], pobTran->srEMVRec.in9F27_CIDLen);
			inPacketCnt += pobTran->srEMVRec.in9F27_CIDLen;
		}
		else
		{
			if (pobTran->srEMVRec.in9F27_CIDLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x27;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F27_CIDLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F27_CID[0], pobTran->srEMVRec.in9F27_CIDLen);
				inPacketCnt += pobTran->srEMVRec.in9F27_CIDLen;
			}
		}
		
		/* 9F33 */
		if (pobTran->srEMVRec.in9F33_TermCapabilitiesLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9F33 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}
			inUtility_StoreTraceLog_OneStep("F 55 TAG 9F33 ERROR!!");

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9F;
		szPacket[inPacketCnt ++] = 0x33;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F33_TermCapabilitiesLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F33_TermCapabilities[0], pobTran->srEMVRec.in9F33_TermCapabilitiesLen);
		inPacketCnt += pobTran->srEMVRec.in9F33_TermCapabilitiesLen;

		/* 9F34 */
		if (inContactlessType == 1)
		{
			if (pobTran->srEMVRec.in9F34_CVMLen <= 0)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "F 55 TAG 9F34 ERROR!!");
					inLogPrintf(AT, szDebugMsg);
				}
				inUtility_StoreTraceLog_OneStep("F 55 TAG 9F34 ERROR!!");

				return (VS_ERROR);
			}
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x34;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F34_CVMLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F34_CVM[0], pobTran->srEMVRec.in9F34_CVMLen);
			inPacketCnt += pobTran->srEMVRec.in9F34_CVMLen;
			
		}
		else if (inContactlessType == 2	||
			 inContactlessType == 4 ||
			 inContactlessType == 5)
		{
			if (!memcmp(guszMFES_MTI, "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
			{
				/* JSPEEDY NEWJ WAVE1 OFFLINE APPROVED不送 */
			}
			else
			{
				if (pobTran->srEMVRec.in9F34_CVMLen > 0)
				{
					szPacket[inPacketCnt ++] = 0x9F;
					szPacket[inPacketCnt ++] = 0x34;
					szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F34_CVMLen;
					memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F34_CVM[0], pobTran->srEMVRec.in9F34_CVMLen);
					inPacketCnt += pobTran->srEMVRec.in9F34_CVMLen;
				}
			}
		}
		else if (inContactlessType == 3)
		{
			if (pobTran->srEMVRec.in9F34_CVMLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x34;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F34_CVMLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F34_CVM[0], pobTran->srEMVRec.in9F34_CVMLen);
				inPacketCnt += pobTran->srEMVRec.in9F34_CVMLen;
			}
		}
		/* 規格沒寫抄Verfone的，有才送 */
		else
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "SPEC not specific");
			}
			
			if (pobTran->srEMVRec.in9F34_CVMLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x34;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F34_CVMLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F34_CVM[0], pobTran->srEMVRec.in9F34_CVMLen);
				inPacketCnt += pobTran->srEMVRec.in9F34_CVMLen;
			}
		
		}
		
		/* 9F35 */
		if (inContactlessType == 1)
		{
			if (pobTran->srEMVRec.in9F35_TermTypeLen <= 0)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "F 55 TAG 9F35 ERROR!!");
					inLogPrintf(AT, szDebugMsg);
				}
				inUtility_StoreTraceLog_OneStep("F 55 TAG 9F35 ERROR!!");

				return (VS_ERROR);
			}
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x35;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F35_TermTypeLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F35_TermType[0], pobTran->srEMVRec.in9F35_TermTypeLen);
			inPacketCnt += pobTran->srEMVRec.in9F35_TermTypeLen;

		}
		else if (inContactlessType == 2	||
			 inContactlessType == 4 ||
			 inContactlessType == 5)
		{
			if (!memcmp(guszMFES_MTI, "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
			{
				/* JSPEEDY NEWJ WAVE1 OFFLINE APPROVED不送 */
			}
			else
			{
				if (pobTran->srEMVRec.in9F35_TermTypeLen > 0)
				{
					szPacket[inPacketCnt ++] = 0x9F;
					szPacket[inPacketCnt ++] = 0x35;
					szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F35_TermTypeLen;
					memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F35_TermType[0], pobTran->srEMVRec.in9F35_TermTypeLen);
					inPacketCnt += pobTran->srEMVRec.in9F35_TermTypeLen;
				}
			}
		}
		else if (inContactlessType == 3)
		{
			if (pobTran->srEMVRec.in9F35_TermTypeLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x35;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F35_TermTypeLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F35_TermType[0], pobTran->srEMVRec.in9F35_TermTypeLen);
				inPacketCnt += pobTran->srEMVRec.in9F35_TermTypeLen;
			}
		}
		/* 規格沒寫抄Verfone的，有才送 */
		else
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "SPEC not specific");
			}
			
			if (pobTran->srEMVRec.in9F35_TermTypeLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x35;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F35_TermTypeLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F35_TermType[0], pobTran->srEMVRec.in9F35_TermTypeLen);
				inPacketCnt += pobTran->srEMVRec.in9F35_TermTypeLen;
			}
		
		}

		/* 9F36 */
		if (pobTran->srEMVRec.in9F36_ATCLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9F36 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}
			inUtility_StoreTraceLog_OneStep("F 55 TAG 9F36 ERROR!!");

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9F;
		szPacket[inPacketCnt ++] = 0x36;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F36_ATCLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F36_ATC[0], pobTran->srEMVRec.in9F36_ATCLen);
		inPacketCnt += pobTran->srEMVRec.in9F36_ATCLen;
		
		/* 9F37 */
		if (pobTran->srEMVRec.in9F37_UnpredictNumLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9F37 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}
			inUtility_StoreTraceLog_OneStep("F 55 TAG 9F37 ERROR!!");

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9F;
		szPacket[inPacketCnt ++] = 0x37;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F37_UnpredictNumLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F37_UnpredictNum[0], pobTran->srEMVRec.in9F37_UnpredictNumLen);
		inPacketCnt += pobTran->srEMVRec.in9F37_UnpredictNumLen;

		/* 9F41 */
		if (inContactlessType == 1)
		{
			if (pobTran->srEMVRec.in9F41_TransSeqCounterLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x41;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F41_TransSeqCounterLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F41_TransSeqCounter[0], pobTran->srEMVRec.in9F41_TransSeqCounterLen);
				inPacketCnt += pobTran->srEMVRec.in9F41_TransSeqCounterLen;
			}
			
		}
		else if (inContactlessType == 2	||
			 inContactlessType == 4 ||
			 inContactlessType == 5)
		{
			if (!memcmp(guszMFES_MTI, "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
			{
				/* JSPEEDY NEWJ WAVE1 OFFLINE APPROVED不送 */
			}
			else
			{
				if (pobTran->srEMVRec.in9F41_TransSeqCounterLen > 0)
				{
					szPacket[inPacketCnt ++] = 0x9F;
					szPacket[inPacketCnt ++] = 0x41;
					szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F41_TransSeqCounterLen;
					memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F41_TransSeqCounter[0], pobTran->srEMVRec.in9F41_TransSeqCounterLen);
					inPacketCnt += pobTran->srEMVRec.in9F41_TransSeqCounterLen;
				}
			}
		}
		else if (inContactlessType == 3)
		{
			if (pobTran->srEMVRec.in9F41_TransSeqCounterLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x41;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F41_TransSeqCounterLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F41_TransSeqCounter[0], pobTran->srEMVRec.in9F41_TransSeqCounterLen);
				inPacketCnt += pobTran->srEMVRec.in9F41_TransSeqCounterLen;
			}
		}
		/* 規格沒寫抄Verfone的，有才送 */
		else
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "SPEC not specific");
			}
			
			if (pobTran->srEMVRec.in9F41_TransSeqCounterLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x41;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F41_TransSeqCounterLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F41_TransSeqCounter[0], pobTran->srEMVRec.in9F41_TransSeqCounterLen);
				inPacketCnt += pobTran->srEMVRec.in9F41_TransSeqCounterLen;
			}
		
		}
				
                /* 【需求單103399】VCPS2.1.3規範， DE55新增上傳 FFI (Tag 9F6E) */
		if (pobTran->srEMVRec.in9F6E_From_Factor_IndicatorLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x6E;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F6E_From_Factor_IndicatorLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F6E_From_Factor_Indicator[0], pobTran->srEMVRec.in9F6E_From_Factor_IndicatorLen);
			inPacketCnt += pobTran->srEMVRec.in9F6E_From_Factor_IndicatorLen;
		}
		
		/* (需求單-110315)-新增支援晶片9F6E及9F7C欄位需求 by Russell 2022/1/17 上午 11:21 */
		if (pobTran->srEMVRec.in9F7C_PartnerDiscretionaryDataLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x7C;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F7C_PartnerDiscretionaryDataLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F7C_PartnerDiscretionaryData[0], pobTran->srEMVRec.in9F7C_PartnerDiscretionaryDataLen);
			inPacketCnt += pobTran->srEMVRec.in9F7C_PartnerDiscretionaryDataLen;
		}
	}
	else if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
	{
		/* 接觸式 正向Mastercard帶57 其他狀況不論卡別都帶5A和5F24 */
		/* 57 */
		if (pobTran->uszTCUploadBit != VS_TRUE		&&
		    (pobTran->inISOTxnCode == _SALE_		||	/* 原來只判斷MTI，但退貨和取消不用，所以下面用加了用ISOTXNCODE來判別 */
		     pobTran->inISOTxnCode == _PRE_AUTH_	||
		     pobTran->inISOTxnCode == _REDEEM_SALE_	||
		     pobTran->inISOTxnCode == _INST_SALE_	||
	             pobTran->inISOTxnCode == _CUP_SALE_	||
		     pobTran->inISOTxnCode == _CUP_PRE_AUTH_))
		{
			if (pobTran->in57_Track2Len <= 0)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "F 55 TAG 57 ERROR!!");
					inLogPrintf(AT, szDebugMsg);
				}

				return (VS_ERROR);
			}

			szPacket[inPacketCnt ++] = 0x57;
			szPacket[inPacketCnt ++] = pobTran->in57_Track2Len;
			memcpy(&szPacket[inPacketCnt], pobTran->usz57_Track2, pobTran->in57_Track2Len);
			inPacketCnt += pobTran->in57_Track2Len;
		}
		else
		{
			/* 5A */
			/* 比照520 2020/2/7 下午 4:50 */
			if (pobTran->srEMVRec.in5A_ApplPanLen <= 0)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "F 55 TAG 5A ERROR!!");
					inLogPrintf(AT, szDebugMsg);
				}

				return (VS_ERROR);
			}

			szPacket[inPacketCnt ++] = 0x5A;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in5A_ApplPanLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz5A_ApplPan[0], pobTran->srEMVRec.in5A_ApplPanLen);
			inPacketCnt += pobTran->srEMVRec.in5A_ApplPanLen;

			/* 5F24 */
			/* 比照520 2020/2/7 下午 4:50 */
			if (pobTran->srEMVRec.in5F24_ExpireDateLen <= 0)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "F 55 TAG 5F24 ERROR!!");
					inLogPrintf(AT, szDebugMsg);
				}

				return (VS_ERROR);
			}

			szPacket[inPacketCnt ++] = 0x5F;
			szPacket[inPacketCnt ++] = 0x24;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in5F24_ExpireDateLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz5F24_ExpireDate[0], pobTran->srEMVRec.in5F24_ExpireDateLen);
			inPacketCnt += pobTran->srEMVRec.in5F24_ExpireDateLen;
		}
		
		/* 5F2A */
		if (pobTran->srEMVRec.in5F2A_TransCurrCodeLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 5F2A ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x5F;
		szPacket[inPacketCnt ++] = 0x2A;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in5F2A_TransCurrCodeLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz5F2A_TransCurrCode[0], pobTran->srEMVRec.in5F2A_TransCurrCodeLen);
		inPacketCnt += pobTran->srEMVRec.in5F2A_TransCurrCodeLen;

		/* 5F34 */
		if (pobTran->srEMVRec.in5F34_ApplPanSeqnumLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x5F;
			szPacket[inPacketCnt ++] = 0x34;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in5F34_ApplPanSeqnumLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz5F34_ApplPanSeqnum[0], pobTran->srEMVRec.in5F34_ApplPanSeqnumLen);
			inPacketCnt += pobTran->srEMVRec.in5F34_ApplPanSeqnumLen;
		}

		/* 82 */
		if (pobTran->srEMVRec.in82_AIPLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 82 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x82;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in82_AIPLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz82_AIP[0], pobTran->srEMVRec.in82_AIPLen);
		inPacketCnt += pobTran->srEMVRec.in82_AIPLen;

		/* 84 */
		if (pobTran->srEMVRec.in84_DFNameLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 84 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x84;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in84_DFNameLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz84_DF_NAME[0], pobTran->srEMVRec.in84_DFNameLen);
		inPacketCnt += pobTran->srEMVRec.in84_DFNameLen;

		/* 8A */
		/* MIT = 〈0220〉 〈0320〉要送 */
		if (!memcmp(&guszMFES_MTI[0], "0220", 4) || !memcmp(&guszMFES_MTI[0], "0320", 4))
		{
			if (pobTran->srEMVRec.in8A_AuthRespCodeLen <= 0)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "F 55 TAG 8A ERROR!!");
					inLogPrintf(AT, szDebugMsg);
				}

				return (VS_ERROR);
			}
			szPacket[inPacketCnt ++] = 0x8A;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in8A_AuthRespCodeLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz8A_AuthRespCode[0], pobTran->srEMVRec.in8A_AuthRespCodeLen);
			inPacketCnt += pobTran->srEMVRec.in8A_AuthRespCodeLen;
			
		}

		/* 95 */
		if (pobTran->srEMVRec.in95_TVRLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 95 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x95;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in95_TVRLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz95_TVR[0], pobTran->srEMVRec.in95_TVRLen);
		inPacketCnt += pobTran->srEMVRec.in95_TVRLen;
		

		/* 9A */
		if (pobTran->srEMVRec.in9A_TranDateLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9A ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9A;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9A_TranDateLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9A_TranDate[0], pobTran->srEMVRec.in9A_TranDateLen);
		inPacketCnt += pobTran->srEMVRec.in9A_TranDateLen;

		/* 9B */
		if (pobTran->srEMVRec.in9B_TSILen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9B ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9B;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9B_TSILen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9B_TSI[0], pobTran->srEMVRec.in9B_TSILen);
		inPacketCnt += pobTran->srEMVRec.in9B_TSILen;

		/* 9C */
		if (pobTran->srEMVRec.in9C_TranTypeLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9C ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9C;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9C_TranTypeLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9C_TranType[0], pobTran->srEMVRec.in9C_TranTypeLen);
		inPacketCnt += pobTran->srEMVRec.in9C_TranTypeLen;

		/* 9F02 */
		if (pobTran->srEMVRec.in9F02_AmtAuthNumLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9F02 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9F;
		szPacket[inPacketCnt ++] = 0x02;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F02_AmtAuthNumLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F02_AmtAuthNum[0], pobTran->srEMVRec.in9F02_AmtAuthNumLen);
		inPacketCnt += pobTran->srEMVRec.in9F02_AmtAuthNumLen;

		/* 9F03 */
		if (pobTran->srEMVRec.in9F03_AmtOtherNumLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9F03 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9F;
		szPacket[inPacketCnt ++] = 0x03;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F03_AmtOtherNumLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F03_AmtOtherNum[0], pobTran->srEMVRec.in9F03_AmtOtherNumLen);
		inPacketCnt += pobTran->srEMVRec.in9F03_AmtOtherNumLen;

		/* 9F09 */
		if (pobTran->srEMVRec.in9F09_TermVerNumLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9F09 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9F;
		szPacket[inPacketCnt ++] = 0x09;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F09_TermVerNumLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F09_TermVerNum[0], pobTran->srEMVRec.in9F09_TermVerNumLen);
		inPacketCnt += pobTran->srEMVRec.in9F09_TermVerNumLen;

		/* 9F10 */
		if (pobTran->srEMVRec.in9F10_IssuerAppDataLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9F10 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9F;
		szPacket[inPacketCnt ++] = 0x10;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F10_IssuerAppDataLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F10_IssuerAppData[0], pobTran->srEMVRec.in9F10_IssuerAppDataLen);
		inPacketCnt += pobTran->srEMVRec.in9F10_IssuerAppDataLen;

		/* 9F1A */
		if (pobTran->srEMVRec.in9F1A_TermCountryCodeLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9F1A ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9F;
		szPacket[inPacketCnt ++] = 0x1A;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F1A_TermCountryCodeLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F1A_TermCountryCode[0], pobTran->srEMVRec.in9F1A_TermCountryCodeLen);
		inPacketCnt += pobTran->srEMVRec.in9F1A_TermCountryCodeLen;

		/* 9F1E */
		if (pobTran->srEMVRec.in9F1E_IFDNumLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x1E;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F1E_IFDNumLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F1E_IFDNum[0], pobTran->srEMVRec.in9F1E_IFDNumLen);
			inPacketCnt += pobTran->srEMVRec.in9F1E_IFDNumLen;
		}

		/* 9F26 */
		if (pobTran->srEMVRec.in9F26_ApplCryptogramLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9F26 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9F;
		szPacket[inPacketCnt ++] = 0x26;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F26_ApplCryptogramLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F26_ApplCryptogram[0], pobTran->srEMVRec.in9F26_ApplCryptogramLen);
		inPacketCnt += pobTran->srEMVRec.in9F26_ApplCryptogramLen;

		/* 9F27 */
		if (pobTran->srEMVRec.in9F27_CIDLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9F27 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9F;
		szPacket[inPacketCnt ++] = 0x27;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F27_CIDLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F27_CID[0], pobTran->srEMVRec.in9F27_CIDLen);
		inPacketCnt += pobTran->srEMVRec.in9F27_CIDLen;

		/* 9F33 */
		if (pobTran->srEMVRec.in9F33_TermCapabilitiesLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9F33 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9F;
		szPacket[inPacketCnt ++] = 0x33;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F33_TermCapabilitiesLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F33_TermCapabilities[0], pobTran->srEMVRec.in9F33_TermCapabilitiesLen);
		inPacketCnt += pobTran->srEMVRec.in9F33_TermCapabilitiesLen;

		/* 9F34 */
		if (pobTran->srEMVRec.in9F34_CVMLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9F34 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9F;
		szPacket[inPacketCnt ++] = 0x34;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F34_CVMLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F34_CVM[0], pobTran->srEMVRec.in9F34_CVMLen);
		inPacketCnt += pobTran->srEMVRec.in9F34_CVMLen;

		/* 9F35 */
		if (pobTran->srEMVRec.in9F35_TermTypeLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9F35 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9F;
		szPacket[inPacketCnt ++] = 0x35;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F35_TermTypeLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F35_TermType[0], pobTran->srEMVRec.in9F35_TermTypeLen);
		inPacketCnt += pobTran->srEMVRec.in9F35_TermTypeLen;

		/* 9F36 */
		if (pobTran->srEMVRec.in9F36_ATCLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9F36 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9F;
		szPacket[inPacketCnt ++] = 0x36;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F36_ATCLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F36_ATC[0], pobTran->srEMVRec.in9F36_ATCLen);
		inPacketCnt += pobTran->srEMVRec.in9F36_ATCLen;

		/* 9F37 */
		if (pobTran->srEMVRec.in9F37_UnpredictNumLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9F37 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9F;
		szPacket[inPacketCnt ++] = 0x37;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F37_UnpredictNumLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F37_UnpredictNum[0], pobTran->srEMVRec.in9F37_UnpredictNumLen);
		inPacketCnt += pobTran->srEMVRec.in9F37_UnpredictNumLen;

		/* 9F41 */
		if (pobTran->srEMVRec.in9F41_TransSeqCounterLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x41;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F41_TransSeqCounterLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F41_TransSeqCounter[0], pobTran->srEMVRec.in9F41_TransSeqCounterLen);
			inPacketCnt += pobTran->srEMVRec.in9F41_TransSeqCounterLen;
		}

		/* 9F5B */
		if (!memcmp(&guszMFES_MTI[0], "0200", 4)	||
		    !memcmp(&guszMFES_MTI[0], "0220", 4)	||
		    !memcmp(&guszMFES_MTI[0], "0320", 4))
		{
			if (pobTran->srEMVRec.in9F5B_ISRLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x5B;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F5B_ISRLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F5B_ISR[0], pobTran->srEMVRec.in9F5B_ISRLen);
				inPacketCnt += pobTran->srEMVRec.in9F5B_ISRLen;
			}
		}
			
		/* 9F63 */
		if (pobTran->srEMVRec.in9F63_CardProductLabelInformationLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x63;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F63_CardProductLabelInformationLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F63_CardProductLabelInformation[0], pobTran->srEMVRec.in9F63_CardProductLabelInformationLen);
			inPacketCnt += pobTran->srEMVRec.in9F63_CardProductLabelInformationLen;
		}

	}
	
	/* 回復原MTI (START) */
	if (pobTran->inISOTxnCode == _REVERSAL_)
	{
		for (inBitMapIndex = 0;; inBitMapIndex++)
		{
			if (srNCCC_MFES_ISOBitMap[inBitMapIndex].inTxnID == _NCCC_MFES_NULL_TX_)
			{
				return (VS_ERROR); /* 找不到相對應交易別的Bit Map */
			}

			if (srNCCC_MFES_ISOBitMap[inBitMapIndex].inTxnID == pobTran->inISOTxnCode)
				break; /* 找到一樣的交易類別 */
		}
		
		memcpy(guszMFES_MTI, srNCCC_MFES_ISOBitMap[inBitMapIndex].szMTI, 4);
	}
	/* 回復原MTI (END) */

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
                inLogPrintf(AT, "inNCCC_MFES_Pack55() END!");
	
	return (inCnt);
}

/*
Function        :inNCCC_MFES_Pack55_tSAM
Date&Time       :2016/12/23 上午 11:54
Describe        :Field_55:	EMV Chip Data
*/
int inNCCC_MFES_Pack55_tSAM(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int		inBitMapIndex;
	int		i;
	int		inRetVal;
	int		inCnt = 0, inPacketCnt = 0;
	int		inContactlessType = 0;
	char		szTemplate[50 + 1];
	char		szField37[8 + 1], szField41[4 + 1];
	char		szASCII[128 + 1];
	char		szPacket[512 + 1];
	char		szDebugMsg[100 + 1];
        unsigned char	uszBCD[2 + 1];
	unsigned char	szEncrypt[200 + 1];
	unsigned char	uszSlot = 0;
	unsigned short	usLen = 0;

	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_Pack55_tSAM() START!");
	
	/* 抓tSAM Slot */
	inRetVal = inNCCC_tSAM_Decide_tSAM_Slot(&uszSlot);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inNCCC_tSAM_Decide_tSAM_Slot Failed");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	/* 開始加密 */
	/* 組 Field_37 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	if (pobTran->inISOTxnCode == _DCC_RATE_)
	{
		/* DCC 詢價因無RRN所以對tSAM 執行卡號加密指令時，Diversified Data則填入8個NULL(0x00)取代原本RRN的前8個Bytes。 */
	}
	else
	{
		memcpy(szTemplate, pobTran->srBRec.szRefNo, 11);
		inFunc_PAD_ASCII(szTemplate, szTemplate, 'F', 16, _PADDING_RIGHT_); /* 補【F】 */
	}
	memset(szField37, 0x00, sizeof(szField37));
	inFunc_ASCII_to_BCD((unsigned char*)szField37, szTemplate, 8);

	if (ginDebug == VS_TRUE)
	{
		memset(szASCII, 0x00, sizeof(szASCII));	
		inFunc_BCD_to_ASCII(szASCII, (unsigned char*)szField37, 8);
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "F_35 [field37: %s] %d", szASCII, 8);
		inLogPrintf(AT, szDebugMsg);
	}

	/* 組 Field_41 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTerminalID(szTemplate);
	memset(szField41, 0x00, sizeof(szField41));
	inFunc_ASCII_to_BCD((unsigned char*)szField41, szTemplate, 4);

	if (ginDebug == VS_TRUE)
	{
		memset(szASCII, 0x00, sizeof(szASCII));
		inFunc_BCD_to_ASCII(szASCII, (unsigned char*)szField41, 4);
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "F_35 [field41: %s] %d", szASCII, 4);
		inLogPrintf(AT, szDebugMsg);
	}

	memset(szPacket, 0x00, sizeof(szPacket));
	
	/* 因為把F55的檢查寫的較嚴格，所以沒考慮到Reversal的狀況，所以若Reversal跑到F55時，先換回原MTI (START) */
	if (pobTran->inISOTxnCode == _REVERSAL_)
	{
		for (inBitMapIndex = 0;; inBitMapIndex++)
		{
			if (srNCCC_MFES_ISOBitMap[inBitMapIndex].inTxnID == _NCCC_MFES_NULL_TX_)
			{
				return (VS_ERROR); /* 找不到相對應交易別的Bit Map */
			}

			if (srNCCC_MFES_ISOBitMap[inBitMapIndex].inTxnID == pobTran->inTransactionCode)
				break; /* 找到一樣的交易類別 */
		}
		
		memcpy(guszMFES_MTI, srNCCC_MFES_ISOBitMap[inBitMapIndex].szMTI, 4);
	}
	/* 因為把F55的檢查寫的較嚴格，所以沒考慮到Reversal的狀況，所以若Reversal跑到F55時，先換回原MTI (END) */
	
	/*
		(1) Tag 9F5B若該筆晶片交易有執行Issure Script Update，除原本TC Upload要上傳 9F5B(ISR)之外，該筆交易的取消或小費交易也要上傳此 Tag。
		(2) Fallback交易DE55僅需上傳Tag DFED、Tag DFEE及Tag DFEF。
		(3) C1:晶片卡 Fallback 的小費交易比照原交易需上傳Tag DFED、Tag DFEE及Tag DFEF。
		(4) Tag 9F63 若UICC卡片包含此Tag時，EDC則應支援該Tag上傳。

		感應交易上傳之 Tag 9F33 的 Value 規則如下：
		(1) VISA Paywave 1/VISA Paywave 3/JSpeedy 請上傳Terminal 之 EMV 參數（因為 V & J 感應交易Reader 不會回 Tag 9F33，故以端末機EMV參數上傳）。
		(2) Paypass 因為 Reader 會回覆 Tag 9F33 ，所以依Reader 回覆資料上傳。
		(3) (O) option 欄位，若是Reader有回覆則必須上傳。
		(4) GHL V3R6 Reader感應Paypass不會回覆Tag9F03，所以此Tag依Reader實際回覆資料上傳。
		UICC的void sale 和void preauth 不上傳此欄位。
	*/
	if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
	{
		switch (pobTran->srBRec.uszWAVESchemeID)
		{
			case SCHEME_ID_20_PAYPASS_MAG_STRIPE:
			case SCHEME_ID_21_PAYPASS_MCHIP:
				inContactlessType = 1;
				break;
			case SCHEME_ID_16_WAVE1:
				inContactlessType = 2;
				break;
			case SCHEME_ID_17_WAVE3:
				inContactlessType = 3;
				break;
			case SCHEME_ID_61_JSPEEDY:
			case SCHEME_ID_65_NEWJSPEEDY_LEGACY:	/* NCCC OLDJ 和NEW J C卡分同一類 a*/
				inContactlessType = 4;
				break;
			case SCHEME_ID_63_NEWJSPEEDY_EMV:
				inContactlessType = 5;
				break;
			case SCHEME_ID_91_QUICKPASS :	/* 0x91 */
				inContactlessType = 6;
				break;
			case SCHEME_ID_50_EXPRESSSPAY :
			case SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE:
				inContactlessType = 7;
				break;
			case SCHEME_ID_42_DPAS :
			case SCHEME_ID_43_DISCOVER_DPAS_MAG_STRIPE :
				inContactlessType = 8;
				break;
			default :
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "F55_未支援的感應卡別");
				}
				inUtility_StoreTraceLog_OneStep("F55_未支援的感應卡別");
				return (VS_ERROR);
				break;
		}
		
		/* 57 */
		/* 感應只有paypass要送，正向送57，其他符合0200、0220、0320、0400的話送5A和5F24 */
		if (inContactlessType == 1)
		{
			/* 比照520 2020/2/7 下午 4:50 */
			if (pobTran->uszTCUploadBit != VS_TRUE		&&
		            (pobTran->inISOTxnCode == _SALE_		||	/* 原來只判斷MTI，但退貨和取消不用，所以下面用加了用ISOTXNCODE來判別 */
		             pobTran->inISOTxnCode == _PRE_AUTH_	||
			     pobTran->inISOTxnCode == _REDEEM_SALE_	||
		             pobTran->inISOTxnCode == _INST_SALE_	||
	                     pobTran->inISOTxnCode == _CUP_SALE_	||
			     pobTran->inISOTxnCode == _CUP_PRE_AUTH_))
			{
				if (pobTran->in57_Track2Len <= 0)
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "F 55 TAG 57 ERROR!!");
						inLogPrintf(AT, szDebugMsg);
					}
					inUtility_StoreTraceLog_OneStep("F 55 TAG 57 ERROR!!");

					return (VS_ERROR);
				}

				memset(szEncrypt, 0x00, sizeof(szEncrypt));
				memcpy(szEncrypt, pobTran->usz57_Track2, pobTran->in57_Track2Len);
				usLen = pobTran->in57_Track2Len;

				/* 不滿16要補 */
				if (usLen < 8)
				{
					for (i = usLen; i < 16; i ++)
					{
						szEncrypt[i] = 0xFF;
					}
				}

				szPacket[inPacketCnt ++] = 0x57;
				szPacket[inPacketCnt ++] = usLen;
				memcpy(&szPacket[inPacketCnt], szEncrypt, usLen);
				inRetVal = inNCCC_tSAM_Encrypt(uszSlot,
							       usLen * 2,
							       (char*)szEncrypt,
							       (unsigned char*)szField37,
							       (unsigned char*)szField41,
							       &gusztSAMKeyIndex,
							       gusztSAMCheckSum_55);
				if (inRetVal == VS_ERROR)
				{
					pobTran->inErrorMsg = _ERROR_CODE_V3_TSAM_ENCRYPT_FAIL_;

					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "F 55 TAG 57 ERROR!!");
						inLogPrintf(AT, szDebugMsg);
					}
					inUtility_StoreTraceLog_OneStep("F 55 TAG 57 ERROR!!");

					return (VS_ERROR);
				}
				else
				{
					guszField_55 = VS_TRUE;
					memcpy(&szPacket[inPacketCnt], szEncrypt, inRetVal);
					inPacketCnt += usLen;
				}
		
			}
			else
			{
				/* 5A */
				/* 比照520 2020/2/7 下午 4:50 */
				if (!memcmp(guszMFES_MTI, "0200", 4)	||
				    !memcmp(guszMFES_MTI, "0220", 4)	||
				    !memcmp(guszMFES_MTI, "0320", 4)	||
				    !memcmp(guszMFES_MTI, "0400", 4))
				{
					if (pobTran->srEMVRec.in5A_ApplPanLen <= 0)
					{
						if (ginDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
							sprintf(szDebugMsg, "F 55 TAG 5A ERROR!!");
							inLogPrintf(AT, szDebugMsg);
						}
						inUtility_StoreTraceLog_OneStep("F 55 TAG 5A ERROR!!");

						return (VS_ERROR);
					}

					szPacket[inPacketCnt ++] = 0x5A;
					szPacket[inPacketCnt ++] = pobTran->srEMVRec.in5A_ApplPanLen;
					memcpy(&szPacket[inPacketCnt], szEncrypt, usLen);
					inRetVal = inNCCC_tSAM_Encrypt(uszSlot,
									usLen * 2,
									(char*)szEncrypt,
									(unsigned char*)szField37,
									(unsigned char*)szField41,
									&gusztSAMKeyIndex,
									gusztSAMCheckSum_55);
					if (inRetVal == VS_ERROR)
					{
						pobTran->inErrorMsg = _ERROR_CODE_V3_TSAM_ENCRYPT_FAIL_;

						if (ginDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
							sprintf(szDebugMsg, "F 55 TAG 5A ERROR!!");
							inLogPrintf(AT, szDebugMsg);
						}
						inUtility_StoreTraceLog_OneStep("F 55 TAG 5A ERROR!!");

						return (VS_ERROR);
					}
					else
					{
						guszField_55 = VS_TRUE;
						memcpy(&szPacket[inPacketCnt], szEncrypt, inRetVal);
						inPacketCnt += usLen;
					}
				}

				/* 5F24 */
				/* 比照520 2020/2/7 下午 4:50 */
				if (!memcmp(guszMFES_MTI, "0200", 4)	||
				    !memcmp(guszMFES_MTI, "0220", 4)	||
				    !memcmp(guszMFES_MTI, "0320", 4)	||
				    !memcmp(guszMFES_MTI, "0400", 4))
				{
					if (pobTran->srEMVRec.in5F24_ExpireDateLen <= 0)
					{
						if (ginDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
							sprintf(szDebugMsg, "F 55 TAG 5F24 ERROR!!");
							inLogPrintf(AT, szDebugMsg);
						}
						inUtility_StoreTraceLog_OneStep("F 55 TAG 5F24 ERROR!!");

						return (VS_ERROR);
					}

					szPacket[inPacketCnt ++] = 0x5F;
					szPacket[inPacketCnt ++] = 0x24;
					szPacket[inPacketCnt ++] = pobTran->srEMVRec.in5F24_ExpireDateLen;
					memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz5F24_ExpireDate[0], pobTran->srEMVRec.in5F24_ExpireDateLen);
					inPacketCnt += pobTran->srEMVRec.in5F24_ExpireDateLen;
				}
			}
		}
		
		/* 5F2A */
		if (pobTran->srEMVRec.in5F2A_TransCurrCodeLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 5F2A ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}
			inUtility_StoreTraceLog_OneStep("F 55 TAG 5F2A ERROR!!");

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x5F;
		szPacket[inPacketCnt ++] = 0x2A;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in5F2A_TransCurrCodeLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz5F2A_TransCurrCode[0], pobTran->srEMVRec.in5F2A_TransCurrCodeLen);
		inPacketCnt += pobTran->srEMVRec.in5F2A_TransCurrCodeLen;
                
		/* 5F34 */
		if (inContactlessType == 1	||
		    inContactlessType == 2	||
		    inContactlessType == 4)
		{
			if (pobTran->srEMVRec.in5F34_ApplPanSeqnumLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x5F;
				szPacket[inPacketCnt ++] = 0x34;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in5F34_ApplPanSeqnumLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz5F34_ApplPanSeqnum[0], pobTran->srEMVRec.in5F34_ApplPanSeqnumLen);
				inPacketCnt += pobTran->srEMVRec.in5F34_ApplPanSeqnumLen;
			}
		}
		else if (inContactlessType == 3)
		{
			if (pobTran->srEMVRec.in5F34_ApplPanSeqnumLen <= 0)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "F 55 TAG 5F34 ERROR!!");
					inLogPrintf(AT, szDebugMsg);
				}
				inUtility_StoreTraceLog_OneStep("F 55 TAG 5F34 ERROR!!");

				return (VS_ERROR);
			}
			szPacket[inPacketCnt ++] = 0x5F;
			szPacket[inPacketCnt ++] = 0x34;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in5F34_ApplPanSeqnumLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz5F34_ApplPanSeqnum[0], pobTran->srEMVRec.in5F34_ApplPanSeqnumLen);
			inPacketCnt += pobTran->srEMVRec.in5F34_ApplPanSeqnumLen;
		}
		/* 規格沒寫抄Verfone的，有才送 */
		else
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "SPEC not specific");
			}
			
			if (pobTran->srEMVRec.in5F34_ApplPanSeqnumLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x5F;
				szPacket[inPacketCnt ++] = 0x34;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in5F34_ApplPanSeqnumLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz5F34_ApplPanSeqnum[0], pobTran->srEMVRec.in5F34_ApplPanSeqnumLen);
				inPacketCnt += pobTran->srEMVRec.in5F34_ApplPanSeqnumLen;
			}
		}
		
		/* 82 */
		if (pobTran->srEMVRec.in82_AIPLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 82 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}
			inUtility_StoreTraceLog_OneStep("F 55 TAG 82 ERROR!!");

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x82;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in82_AIPLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz82_AIP[0], pobTran->srEMVRec.in82_AIPLen);
		inPacketCnt += pobTran->srEMVRec.in82_AIPLen;
	
		/* 84 */
		if (inContactlessType == 1)
		{
			if (pobTran->srEMVRec.in84_DFNameLen <= 0)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "F 55 TAG 84 ERROR!!");
					inLogPrintf(AT, szDebugMsg);
				}
				inUtility_StoreTraceLog_OneStep("F 55 TAG 84 ERROR!!");

				return (VS_ERROR);
			}
			szPacket[inPacketCnt ++] = 0x84;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in84_DFNameLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz84_DF_NAME[0], pobTran->srEMVRec.in84_DFNameLen);
			inPacketCnt += pobTran->srEMVRec.in84_DFNameLen;
		}
		else if (inContactlessType == 2	||
			 inContactlessType == 4 ||
			 inContactlessType == 5)
		{
			if (!memcmp(guszMFES_MTI, "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
			{
				/* JSPEEDY NEWJ WAVE1 OFFLINE APPROVED不送 */
			}
			else
			{
				if (pobTran->srEMVRec.in84_DFNameLen > 0)
				{
					szPacket[inPacketCnt ++] = 0x84;
					szPacket[inPacketCnt ++] = pobTran->srEMVRec.in84_DFNameLen;
					memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz84_DF_NAME[0], pobTran->srEMVRec.in84_DFNameLen);
					inPacketCnt += pobTran->srEMVRec.in84_DFNameLen;
				}
			}
		}
		else if (inContactlessType == 3)
		{
			if (pobTran->srEMVRec.in84_DFNameLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x84;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in84_DFNameLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz84_DF_NAME[0], pobTran->srEMVRec.in84_DFNameLen);
				inPacketCnt += pobTran->srEMVRec.in84_DFNameLen;
			}
		}
		/* 規格沒寫抄Verfone的，有才送 */
		else
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "SPEC not specific");
			}
			
			if (pobTran->srEMVRec.in84_DFNameLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x84;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in84_DFNameLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz84_DF_NAME[0], pobTran->srEMVRec.in84_DFNameLen);
				inPacketCnt += pobTran->srEMVRec.in84_DFNameLen;
			}
		}

		/* 8A */
		if (inContactlessType == 1)
		{
			if (!memcmp(guszMFES_MTI, "0220", 4) || !memcmp(guszMFES_MTI, "0320", 4))
			{
				if (pobTran->srEMVRec.in8A_AuthRespCodeLen <= 0)
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "F 55 TAG 8A ERROR!!");
						inLogPrintf(AT, szDebugMsg);
					}
					inUtility_StoreTraceLog_OneStep("F 55 TAG 8A ERROR!!");

					return (VS_ERROR);
				}
				szPacket[inPacketCnt ++] = 0x8A;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in8A_AuthRespCodeLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz8A_AuthRespCode[0], pobTran->srEMVRec.in8A_AuthRespCodeLen);
				inPacketCnt += pobTran->srEMVRec.in8A_AuthRespCodeLen;
			}
		
		}
		else if (inContactlessType == 2	||
			 inContactlessType == 4 ||
			 inContactlessType == 5)
		{
			if (!memcmp(guszMFES_MTI, "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
			{
				/* WAVE1 JSPEEDY NEWJ OFFLINE APPROVED不送 */
			}
			else if (!memcmp(guszMFES_MTI, "0200", 4))
			{
				/* WAVE1 JSPEEDY NEWJ 0200不送 */
			}
			else
			{
				if (pobTran->srEMVRec.in8A_AuthRespCodeLen > 0)
				{
					szPacket[inPacketCnt ++] = 0x8A;
					szPacket[inPacketCnt ++] = pobTran->srEMVRec.in8A_AuthRespCodeLen;
					memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz8A_AuthRespCode[0], pobTran->srEMVRec.in8A_AuthRespCodeLen);
					inPacketCnt += pobTran->srEMVRec.in8A_AuthRespCodeLen;
				}
			}
			
		}
		else if (inContactlessType == 3	||
			 inContactlessType == 6	||
			 inContactlessType == 7	||
			 inContactlessType == 8)
		{
			if (!memcmp(guszMFES_MTI, "0200", 4))
			{
				/* WAVE 3 QUICKPASS 不送 */
			}
			else
			{
				if (pobTran->srEMVRec.in8A_AuthRespCodeLen > 0)
				{
					szPacket[inPacketCnt ++] = 0x8A;
					szPacket[inPacketCnt ++] = pobTran->srEMVRec.in8A_AuthRespCodeLen;
					memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz8A_AuthRespCode[0], pobTran->srEMVRec.in8A_AuthRespCodeLen);
					inPacketCnt += pobTran->srEMVRec.in8A_AuthRespCodeLen;
				}
			}
		}
		/* 規格沒寫抄Verfone的，有才送 */
		else
		{	
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "SPEC not specific");
			}
			if (pobTran->srEMVRec.in8A_AuthRespCodeLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x8A;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in8A_AuthRespCodeLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz8A_AuthRespCode[0], pobTran->srEMVRec.in8A_AuthRespCodeLen);
				inPacketCnt += pobTran->srEMVRec.in8A_AuthRespCodeLen;
			}
		}
		
		/* 95 */
		if (pobTran->srEMVRec.in95_TVRLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 95 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}
			inUtility_StoreTraceLog_OneStep("F 55 TAG 95 ERROR!!");

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x95;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in95_TVRLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz95_TVR[0], pobTran->srEMVRec.in95_TVRLen);
		inPacketCnt += pobTran->srEMVRec.in95_TVRLen;

		/* 9A */
		if (pobTran->srEMVRec.in9A_TranDateLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9A ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}
			inUtility_StoreTraceLog_OneStep("F 55 TAG 9A ERROR!!");

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9A;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9A_TranDateLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9A_TranDate[0], pobTran->srEMVRec.in9A_TranDateLen);
		inPacketCnt += pobTran->srEMVRec.in9A_TranDateLen;
		
		/* 9B */
		if (inContactlessType == 1)
		{
//			if (pobTran->srEMVRec.in9B_TSILen <= 0)
//			{
//				if (ginDebug == VS_TRUE)
//				{
//					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
//					sprintf(szDebugMsg, "F 55 TAG 9B ERROR!!");
//					inLogPrintf(AT, szDebugMsg);
//				}
//				inUtility_StoreTraceLog_OneStep("F 55 TAG 9B ERROR!!");
//
//				return (VS_ERROR);
//			}
			/* 抄Verifone有才送，ATS(含MCAP_ATS)非接觸式交易，目前不會檢查Tag 9B為必要欄位 */
			if (pobTran->srEMVRec.in9B_TSILen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9B;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9B_TSILen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9B_TSI[0], pobTran->srEMVRec.in9B_TSILen);
				inPacketCnt += pobTran->srEMVRec.in9B_TSILen;
			}
		}
		else if (inContactlessType == 2	||
			 inContactlessType == 4 ||
			 inContactlessType == 5)
		{
			if (!memcmp(guszMFES_MTI, "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
			{
				/* JSPEEDY NEWJ WAVE1 OFFLINE APPROVED不送 */
			}
			else
			{
				if (pobTran->srEMVRec.in9B_TSILen > 0)
				{
					szPacket[inPacketCnt ++] = 0x9B;
					szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9B_TSILen;
					memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9B_TSI[0], pobTran->srEMVRec.in9B_TSILen);
					inPacketCnt += pobTran->srEMVRec.in9B_TSILen;
				}
			}
		}
		else if (inContactlessType == 3)
		{
			if (pobTran->srEMVRec.in9B_TSILen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9B;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9B_TSILen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9B_TSI[0], pobTran->srEMVRec.in9B_TSILen);
				inPacketCnt += pobTran->srEMVRec.in9B_TSILen;
			}
		}
		/* 規格沒寫抄Verfone的，有才送 */
		else
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "SPEC not specific");
			}
			
			if (pobTran->srEMVRec.in9B_TSILen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9B;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9B_TSILen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9B_TSI[0], pobTran->srEMVRec.in9B_TSILen);
				inPacketCnt += pobTran->srEMVRec.in9B_TSILen;
			}
		}
		
		/* 9C */
		if (pobTran->srEMVRec.in9C_TranTypeLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9C ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}
			inUtility_StoreTraceLog_OneStep("F 55 TAG 9C ERROR!!");

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9C;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9C_TranTypeLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9C_TranType[0], pobTran->srEMVRec.in9C_TranTypeLen);
		inPacketCnt += pobTran->srEMVRec.in9C_TranTypeLen;

		/* 9F02 */
		if (pobTran->srEMVRec.in9F02_AmtAuthNumLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9F02 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}
			inUtility_StoreTraceLog_OneStep("F 55 TAG 9F02 ERROR!!");

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9F;
		szPacket[inPacketCnt ++] = 0x02;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F02_AmtAuthNumLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F02_AmtAuthNum[0], pobTran->srEMVRec.in9F02_AmtAuthNumLen);
		inPacketCnt += pobTran->srEMVRec.in9F02_AmtAuthNumLen;
		
		/* 9F03 */
		if (pobTran->srEMVRec.in9F03_AmtOtherNumLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x03;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F03_AmtOtherNumLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F03_AmtOtherNum[0], pobTran->srEMVRec.in9F03_AmtOtherNumLen);
			inPacketCnt += pobTran->srEMVRec.in9F03_AmtOtherNumLen;
		}

		/* 9F10 */
		if (pobTran->srEMVRec.in9F10_IssuerAppDataLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9F10 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}
			inUtility_StoreTraceLog_OneStep("F 55 TAG 9F10 ERROR!!");

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9F;
		szPacket[inPacketCnt ++] = 0x10;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F10_IssuerAppDataLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F10_IssuerAppData[0], pobTran->srEMVRec.in9F10_IssuerAppDataLen);
		inPacketCnt += pobTran->srEMVRec.in9F10_IssuerAppDataLen;
		

		/* 9F1A */
		if (pobTran->srEMVRec.in9F1A_TermCountryCodeLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9F1A ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}
			inUtility_StoreTraceLog_OneStep("F 55 TAG 9F1A ERROR!!");

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9F;
		szPacket[inPacketCnt ++] = 0x1A;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F1A_TermCountryCodeLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F1A_TermCountryCode[0], pobTran->srEMVRec.in9F1A_TermCountryCodeLen);
		inPacketCnt += pobTran->srEMVRec.in9F1A_TermCountryCodeLen;
		
		/* 9F1E */
		if (inContactlessType == 1)
		{
			if (pobTran->srEMVRec.in9F1E_IFDNumLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x1E;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F1E_IFDNumLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F1E_IFDNum[0], pobTran->srEMVRec.in9F1E_IFDNumLen);
				inPacketCnt += pobTran->srEMVRec.in9F1E_IFDNumLen;
			}
			
		}
		else if (inContactlessType == 2	||
			 inContactlessType == 4 ||
			 inContactlessType == 5)
		{
			if (!memcmp(guszMFES_MTI, "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
			{
				/* JSPEEDY NEWJ WAVE1 OFFLINE APPROVED不送 */
			}
			else
			{
				if (pobTran->srEMVRec.in9F1E_IFDNumLen > 0)
				{
					szPacket[inPacketCnt ++] = 0x9F;
					szPacket[inPacketCnt ++] = 0x1E;
					szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F1E_IFDNumLen;
					memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F1E_IFDNum[0], pobTran->srEMVRec.in9F1E_IFDNumLen);
					inPacketCnt += pobTran->srEMVRec.in9F1E_IFDNumLen;
				}
			}
		}
		else if (inContactlessType == 3)
		{
			if (pobTran->srEMVRec.in9F1E_IFDNumLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x1E;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F1E_IFDNumLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F1E_IFDNum[0], pobTran->srEMVRec.in9F1E_IFDNumLen);
				inPacketCnt += pobTran->srEMVRec.in9F1E_IFDNumLen;
			}
		}
		/* 規格沒寫抄Verfone的，有才送 */
		else
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "SPEC not specific");
			}
			
			if (pobTran->srEMVRec.in9F1E_IFDNumLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x1E;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F1E_IFDNumLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F1E_IFDNum[0], pobTran->srEMVRec.in9F1E_IFDNumLen);
				inPacketCnt += pobTran->srEMVRec.in9F1E_IFDNumLen;
			}
		
		}

		/* 9F26 */
		if (pobTran->srEMVRec.in9F26_ApplCryptogramLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9F26 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}
			inUtility_StoreTraceLog_OneStep("F 55 TAG 9F26 ERROR!!");

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9F;
		szPacket[inPacketCnt ++] = 0x26;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F26_ApplCryptogramLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F26_ApplCryptogram[0], pobTran->srEMVRec.in9F26_ApplCryptogramLen);
		inPacketCnt += pobTran->srEMVRec.in9F26_ApplCryptogramLen;
		
		/* 9F27 */
		if (inContactlessType == 1)
		{
			if (pobTran->srEMVRec.in9F27_CIDLen <= 0)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "F 55 TAG 9F27 ERROR!!");
					inLogPrintf(AT, szDebugMsg);
				}
				inUtility_StoreTraceLog_OneStep("F 55 TAG 9F27 ERROR!!");

				return (VS_ERROR);
			}
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x27;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F27_CIDLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F27_CID[0], pobTran->srEMVRec.in9F27_CIDLen);
			inPacketCnt += pobTran->srEMVRec.in9F27_CIDLen;
		}
		else
		{
			if (pobTran->srEMVRec.in9F27_CIDLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x27;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F27_CIDLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F27_CID[0], pobTran->srEMVRec.in9F27_CIDLen);
				inPacketCnt += pobTran->srEMVRec.in9F27_CIDLen;
			}
		}
		
		/* 9F33 */
		if (pobTran->srEMVRec.in9F33_TermCapabilitiesLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9F33 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}
			inUtility_StoreTraceLog_OneStep("F 55 TAG 9F33 ERROR!!");

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9F;
		szPacket[inPacketCnt ++] = 0x33;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F33_TermCapabilitiesLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F33_TermCapabilities[0], pobTran->srEMVRec.in9F33_TermCapabilitiesLen);
		inPacketCnt += pobTran->srEMVRec.in9F33_TermCapabilitiesLen;

		/* 9F34 */
		if (inContactlessType == 1)
		{
			if (pobTran->srEMVRec.in9F34_CVMLen <= 0)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "F 55 TAG 9F34 ERROR!!");
					inLogPrintf(AT, szDebugMsg);
				}
				inUtility_StoreTraceLog_OneStep("F 55 TAG 9F34 ERROR!!");

				return (VS_ERROR);
			}
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x34;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F34_CVMLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F34_CVM[0], pobTran->srEMVRec.in9F34_CVMLen);
			inPacketCnt += pobTran->srEMVRec.in9F34_CVMLen;
			
		}
		else if (inContactlessType == 2	||
			 inContactlessType == 4 ||
			 inContactlessType == 5)
		{
			if (!memcmp(guszMFES_MTI, "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
			{
				/* JSPEEDY NEWJ WAVE1 OFFLINE APPROVED不送 */
			}
			else
			{
				if (pobTran->srEMVRec.in9F34_CVMLen > 0)
				{
					szPacket[inPacketCnt ++] = 0x9F;
					szPacket[inPacketCnt ++] = 0x34;
					szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F34_CVMLen;
					memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F34_CVM[0], pobTran->srEMVRec.in9F34_CVMLen);
					inPacketCnt += pobTran->srEMVRec.in9F34_CVMLen;
				}
			}
		}
		else if (inContactlessType == 3)
		{
			if (pobTran->srEMVRec.in9F34_CVMLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x34;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F34_CVMLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F34_CVM[0], pobTran->srEMVRec.in9F34_CVMLen);
				inPacketCnt += pobTran->srEMVRec.in9F34_CVMLen;
			}
		}
		/* 規格沒寫抄Verfone的，有才送 */
		else
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "SPEC not specific");
			}
			
			if (pobTran->srEMVRec.in9F34_CVMLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x34;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F34_CVMLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F34_CVM[0], pobTran->srEMVRec.in9F34_CVMLen);
				inPacketCnt += pobTran->srEMVRec.in9F34_CVMLen;
			}
		
		}
		
		/* 9F35 */
		if (inContactlessType == 1)
		{
			if (pobTran->srEMVRec.in9F35_TermTypeLen <= 0)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "F 55 TAG 9F35 ERROR!!");
					inLogPrintf(AT, szDebugMsg);
				}
				inUtility_StoreTraceLog_OneStep("F 55 TAG 9F35 ERROR!!");

				return (VS_ERROR);
			}
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x35;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F35_TermTypeLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F35_TermType[0], pobTran->srEMVRec.in9F35_TermTypeLen);
			inPacketCnt += pobTran->srEMVRec.in9F35_TermTypeLen;

		}
		else if (inContactlessType == 2	||
			 inContactlessType == 4 ||
			 inContactlessType == 5)
		{
			if (!memcmp(guszMFES_MTI, "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
			{
				/* JSPEEDY NEWJ WAVE1 OFFLINE APPROVED不送 */
			}
			else
			{
				if (pobTran->srEMVRec.in9F35_TermTypeLen > 0)
				{
					szPacket[inPacketCnt ++] = 0x9F;
					szPacket[inPacketCnt ++] = 0x35;
					szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F35_TermTypeLen;
					memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F35_TermType[0], pobTran->srEMVRec.in9F35_TermTypeLen);
					inPacketCnt += pobTran->srEMVRec.in9F35_TermTypeLen;
				}
			}
		}
		else if (inContactlessType == 3)
		{
			if (pobTran->srEMVRec.in9F35_TermTypeLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x35;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F35_TermTypeLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F35_TermType[0], pobTran->srEMVRec.in9F35_TermTypeLen);
				inPacketCnt += pobTran->srEMVRec.in9F35_TermTypeLen;
			}
		}
		/* 規格沒寫抄Verfone的，有才送 */
		else
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "SPEC not specific");
			}
			
			if (pobTran->srEMVRec.in9F35_TermTypeLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x35;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F35_TermTypeLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F35_TermType[0], pobTran->srEMVRec.in9F35_TermTypeLen);
				inPacketCnt += pobTran->srEMVRec.in9F35_TermTypeLen;
			}
		
		}

		/* 9F36 */
		if (pobTran->srEMVRec.in9F36_ATCLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9F36 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}
			inUtility_StoreTraceLog_OneStep("F 55 TAG 9F36 ERROR!!");

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9F;
		szPacket[inPacketCnt ++] = 0x36;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F36_ATCLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F36_ATC[0], pobTran->srEMVRec.in9F36_ATCLen);
		inPacketCnt += pobTran->srEMVRec.in9F36_ATCLen;
		
		/* 9F37 */
		if (pobTran->srEMVRec.in9F37_UnpredictNumLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9F37 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}
			inUtility_StoreTraceLog_OneStep("F 55 TAG 9F37 ERROR!!");

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9F;
		szPacket[inPacketCnt ++] = 0x37;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F37_UnpredictNumLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F37_UnpredictNum[0], pobTran->srEMVRec.in9F37_UnpredictNumLen);
		inPacketCnt += pobTran->srEMVRec.in9F37_UnpredictNumLen;

		/* 9F41 */
		if (inContactlessType == 1)
		{
			if (pobTran->srEMVRec.in9F41_TransSeqCounterLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x41;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F41_TransSeqCounterLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F41_TransSeqCounter[0], pobTran->srEMVRec.in9F41_TransSeqCounterLen);
				inPacketCnt += pobTran->srEMVRec.in9F41_TransSeqCounterLen;
			}
			
		}
		else if (inContactlessType == 2	||
			 inContactlessType == 4 ||
			 inContactlessType == 5)
		{
			if (!memcmp(guszMFES_MTI, "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
			{
				/* JSPEEDY NEWJ WAVE1 OFFLINE APPROVED不送 */
			}
			else
			{
				if (pobTran->srEMVRec.in9F41_TransSeqCounterLen > 0)
				{
					szPacket[inPacketCnt ++] = 0x9F;
					szPacket[inPacketCnt ++] = 0x41;
					szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F41_TransSeqCounterLen;
					memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F41_TransSeqCounter[0], pobTran->srEMVRec.in9F41_TransSeqCounterLen);
					inPacketCnt += pobTran->srEMVRec.in9F41_TransSeqCounterLen;
				}
			}
		}
		else if (inContactlessType == 3)
		{
			if (pobTran->srEMVRec.in9F41_TransSeqCounterLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x41;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F41_TransSeqCounterLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F41_TransSeqCounter[0], pobTran->srEMVRec.in9F41_TransSeqCounterLen);
				inPacketCnt += pobTran->srEMVRec.in9F41_TransSeqCounterLen;
			}
		}
		/* 規格沒寫抄Verfone的，有才送 */
		else
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "SPEC not specific");
			}
			
			if (pobTran->srEMVRec.in9F41_TransSeqCounterLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x41;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F41_TransSeqCounterLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F41_TransSeqCounter[0], pobTran->srEMVRec.in9F41_TransSeqCounterLen);
				inPacketCnt += pobTran->srEMVRec.in9F41_TransSeqCounterLen;
			}
		
		}
				
                /* 【需求單103399】VCPS2.1.3規範， DE55新增上傳 FFI (Tag 9F6E) */
		if (pobTran->srEMVRec.in9F6E_From_Factor_IndicatorLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x6E;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F6E_From_Factor_IndicatorLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F6E_From_Factor_Indicator[0], pobTran->srEMVRec.in9F6E_From_Factor_IndicatorLen);
			inPacketCnt += pobTran->srEMVRec.in9F6E_From_Factor_IndicatorLen;
		}
		
		/* (需求單-110315)-新增支援晶片9F6E及9F7C欄位需求 by Russell 2022/1/17 上午 11:21 */
		if (pobTran->srEMVRec.in9F7C_PartnerDiscretionaryDataLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x7C;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F7C_PartnerDiscretionaryDataLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F7C_PartnerDiscretionaryData[0], pobTran->srEMVRec.in9F7C_PartnerDiscretionaryDataLen);
			inPacketCnt += pobTran->srEMVRec.in9F7C_PartnerDiscretionaryDataLen;
		}
	}
	else if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
	{
		/* 接觸式 正向Mastercard帶57 其他狀況不論卡別都帶5A和5F24 */
		/* 57 */
		if (pobTran->uszTCUploadBit != VS_TRUE		&&
		    (pobTran->inISOTxnCode == _SALE_		||	/* 原來只判斷MTI，但退貨和取消不用，所以下面用加了用ISOTXNCODE來判別 */
		     pobTran->inISOTxnCode == _PRE_AUTH_	||
		     pobTran->inISOTxnCode == _REDEEM_SALE_	||
		     pobTran->inISOTxnCode == _INST_SALE_	||
	             pobTran->inISOTxnCode == _CUP_SALE_	||
		     pobTran->inISOTxnCode == _CUP_PRE_AUTH_))
		{
			if (pobTran->in57_Track2Len <= 0)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "F 55 TAG 57 ERROR!!");
					inLogPrintf(AT, szDebugMsg);
				}

				return (VS_ERROR);
			}

			memset(szEncrypt, 0x00, sizeof(szEncrypt));
			memcpy(szEncrypt, pobTran->usz57_Track2, pobTran->in57_Track2Len);
			usLen = pobTran->in57_Track2Len;

			/* 不滿16要補 */
			if (usLen < 8)
			{
				for (i = usLen; i < 16; i ++)
				{
					szEncrypt[i] = 0xFF;
				}
			}

			szPacket[inPacketCnt ++] = 0x57;
			szPacket[inPacketCnt ++] = usLen;
			memcpy(&szPacket[inPacketCnt], szEncrypt, usLen);
			inRetVal = inNCCC_tSAM_Encrypt(uszSlot,
						       usLen * 2,
						       (char*)szEncrypt,
						       (unsigned char*)szField37,
						       (unsigned char*)szField41,
						       &gusztSAMKeyIndex,
						       gusztSAMCheckSum_55);
			if (inRetVal == VS_ERROR)
			{
				pobTran->inErrorMsg = _ERROR_CODE_V3_TSAM_ENCRYPT_FAIL_;

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "F 55 TAG 57 ERROR!!");
					inLogPrintf(AT, szDebugMsg);
				}

				return (VS_ERROR);
			}
			else
			{
				guszField_55 = VS_TRUE;
				memcpy(&szPacket[inPacketCnt], szEncrypt, inRetVal);
				inPacketCnt += usLen;
			}
		}
		else
		{
			/* 5A */
			/* 比照520 2020/2/7 下午 4:50 */
			if (pobTran->srEMVRec.in5A_ApplPanLen <= 0)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "F 55 TAG 5A ERROR!!");
					inLogPrintf(AT, szDebugMsg);
				}

				return (VS_ERROR);
			}

			memset(szEncrypt, 0x00, sizeof(szEncrypt));
			memcpy(szEncrypt, &pobTran->srEMVRec.usz5A_ApplPan[0], pobTran->srEMVRec.in5A_ApplPanLen);
			usLen = pobTran->srEMVRec.in5A_ApplPanLen;

			/* 不滿16要補 */
			if (pobTran->srEMVRec.in5A_ApplPanLen < 8)
			{
				for (i = pobTran->srEMVRec.in5A_ApplPanLen; i < 16; i ++)
				{
					szEncrypt[i] = 0xFF;
				}
			}

			szPacket[inPacketCnt ++] = 0x5A;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in5A_ApplPanLen;
			memcpy(&szPacket[inPacketCnt], szEncrypt, usLen);
			inRetVal = inNCCC_tSAM_Encrypt(uszSlot,
							usLen * 2,
							(char*)szEncrypt,
							(unsigned char*)szField37,
							(unsigned char*)szField41,
							&gusztSAMKeyIndex,
							gusztSAMCheckSum_55);
			if (inRetVal == VS_ERROR)
			{
				pobTran->inErrorMsg = _ERROR_CODE_V3_TSAM_ENCRYPT_FAIL_;

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "F 55 TAG 5A ERROR!!");
					inLogPrintf(AT, szDebugMsg);
				}

				return (VS_ERROR);
			}
			else
			{
				guszField_55 = VS_TRUE;
				memcpy(&szPacket[inPacketCnt], szEncrypt, inRetVal);
				inPacketCnt += usLen;
			}

			/* 5F24 */
			/* 比照520 2020/2/7 下午 4:50 */
			if (pobTran->srEMVRec.in5F24_ExpireDateLen <= 0)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "F 55 TAG 5F24 ERROR!!");
					inLogPrintf(AT, szDebugMsg);
				}

				return (VS_ERROR);
			}

			szPacket[inPacketCnt ++] = 0x5F;
			szPacket[inPacketCnt ++] = 0x24;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in5F24_ExpireDateLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz5F24_ExpireDate[0], pobTran->srEMVRec.in5F24_ExpireDateLen);
			inPacketCnt += pobTran->srEMVRec.in5F24_ExpireDateLen;
		}
		
		/* 5F2A */
		if (pobTran->srEMVRec.in5F2A_TransCurrCodeLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 5F2A ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x5F;
		szPacket[inPacketCnt ++] = 0x2A;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in5F2A_TransCurrCodeLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz5F2A_TransCurrCode[0], pobTran->srEMVRec.in5F2A_TransCurrCodeLen);
		inPacketCnt += pobTran->srEMVRec.in5F2A_TransCurrCodeLen;

		/* 5F34 */
		if (pobTran->srEMVRec.in5F34_ApplPanSeqnumLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x5F;
			szPacket[inPacketCnt ++] = 0x34;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in5F34_ApplPanSeqnumLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz5F34_ApplPanSeqnum[0], pobTran->srEMVRec.in5F34_ApplPanSeqnumLen);
			inPacketCnt += pobTran->srEMVRec.in5F34_ApplPanSeqnumLen;
		}

		/* 82 */
		if (pobTran->srEMVRec.in82_AIPLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 82 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x82;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in82_AIPLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz82_AIP[0], pobTran->srEMVRec.in82_AIPLen);
		inPacketCnt += pobTran->srEMVRec.in82_AIPLen;

		/* 84 */
		if (pobTran->srEMVRec.in84_DFNameLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 84 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x84;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in84_DFNameLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz84_DF_NAME[0], pobTran->srEMVRec.in84_DFNameLen);
		inPacketCnt += pobTran->srEMVRec.in84_DFNameLen;

		/* 8A */
		/* MIT = 〈0220〉 〈0320〉要送 */
		if (!memcmp(&guszMFES_MTI[0], "0220", 4) || !memcmp(&guszMFES_MTI[0], "0320", 4))
		{
			if (pobTran->srEMVRec.in8A_AuthRespCodeLen <= 0)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "F 55 TAG 8A ERROR!!");
					inLogPrintf(AT, szDebugMsg);
				}

				return (VS_ERROR);
			}
			szPacket[inPacketCnt ++] = 0x8A;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in8A_AuthRespCodeLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz8A_AuthRespCode[0], pobTran->srEMVRec.in8A_AuthRespCodeLen);
			inPacketCnt += pobTran->srEMVRec.in8A_AuthRespCodeLen;
			
		}

		/* 95 */
		if (pobTran->srEMVRec.in95_TVRLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 95 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x95;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in95_TVRLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz95_TVR[0], pobTran->srEMVRec.in95_TVRLen);
		inPacketCnt += pobTran->srEMVRec.in95_TVRLen;
		

		/* 9A */
		if (pobTran->srEMVRec.in9A_TranDateLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9A ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9A;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9A_TranDateLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9A_TranDate[0], pobTran->srEMVRec.in9A_TranDateLen);
		inPacketCnt += pobTran->srEMVRec.in9A_TranDateLen;

		/* 9B */
		if (pobTran->srEMVRec.in9B_TSILen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9B ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9B;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9B_TSILen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9B_TSI[0], pobTran->srEMVRec.in9B_TSILen);
		inPacketCnt += pobTran->srEMVRec.in9B_TSILen;

		/* 9C */
		if (pobTran->srEMVRec.in9C_TranTypeLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9C ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9C;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9C_TranTypeLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9C_TranType[0], pobTran->srEMVRec.in9C_TranTypeLen);
		inPacketCnt += pobTran->srEMVRec.in9C_TranTypeLen;

		/* 9F02 */
		if (pobTran->srEMVRec.in9F02_AmtAuthNumLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9F02 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9F;
		szPacket[inPacketCnt ++] = 0x02;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F02_AmtAuthNumLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F02_AmtAuthNum[0], pobTran->srEMVRec.in9F02_AmtAuthNumLen);
		inPacketCnt += pobTran->srEMVRec.in9F02_AmtAuthNumLen;

		/* 9F03 */
		if (pobTran->srEMVRec.in9F03_AmtOtherNumLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9F03 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9F;
		szPacket[inPacketCnt ++] = 0x03;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F03_AmtOtherNumLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F03_AmtOtherNum[0], pobTran->srEMVRec.in9F03_AmtOtherNumLen);
		inPacketCnt += pobTran->srEMVRec.in9F03_AmtOtherNumLen;

		/* 9F09 */
		if (pobTran->srEMVRec.in9F09_TermVerNumLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9F09 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9F;
		szPacket[inPacketCnt ++] = 0x09;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F09_TermVerNumLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F09_TermVerNum[0], pobTran->srEMVRec.in9F09_TermVerNumLen);
		inPacketCnt += pobTran->srEMVRec.in9F09_TermVerNumLen;

		/* 9F10 */
		if (pobTran->srEMVRec.in9F10_IssuerAppDataLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9F10 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9F;
		szPacket[inPacketCnt ++] = 0x10;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F10_IssuerAppDataLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F10_IssuerAppData[0], pobTran->srEMVRec.in9F10_IssuerAppDataLen);
		inPacketCnt += pobTran->srEMVRec.in9F10_IssuerAppDataLen;

		/* 9F1A */
		if (pobTran->srEMVRec.in9F1A_TermCountryCodeLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9F1A ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9F;
		szPacket[inPacketCnt ++] = 0x1A;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F1A_TermCountryCodeLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F1A_TermCountryCode[0], pobTran->srEMVRec.in9F1A_TermCountryCodeLen);
		inPacketCnt += pobTran->srEMVRec.in9F1A_TermCountryCodeLen;

		/* 9F1E */
		if (pobTran->srEMVRec.in9F1E_IFDNumLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x1E;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F1E_IFDNumLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F1E_IFDNum[0], pobTran->srEMVRec.in9F1E_IFDNumLen);
			inPacketCnt += pobTran->srEMVRec.in9F1E_IFDNumLen;
		}

		/* 9F26 */
		if (pobTran->srEMVRec.in9F26_ApplCryptogramLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9F26 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9F;
		szPacket[inPacketCnt ++] = 0x26;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F26_ApplCryptogramLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F26_ApplCryptogram[0], pobTran->srEMVRec.in9F26_ApplCryptogramLen);
		inPacketCnt += pobTran->srEMVRec.in9F26_ApplCryptogramLen;

		/* 9F27 */
		if (pobTran->srEMVRec.in9F27_CIDLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9F27 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9F;
		szPacket[inPacketCnt ++] = 0x27;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F27_CIDLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F27_CID[0], pobTran->srEMVRec.in9F27_CIDLen);
		inPacketCnt += pobTran->srEMVRec.in9F27_CIDLen;

		/* 9F33 */
		if (pobTran->srEMVRec.in9F33_TermCapabilitiesLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9F33 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9F;
		szPacket[inPacketCnt ++] = 0x33;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F33_TermCapabilitiesLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F33_TermCapabilities[0], pobTran->srEMVRec.in9F33_TermCapabilitiesLen);
		inPacketCnt += pobTran->srEMVRec.in9F33_TermCapabilitiesLen;

		/* 9F34 */
		if (pobTran->srEMVRec.in9F34_CVMLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9F34 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9F;
		szPacket[inPacketCnt ++] = 0x34;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F34_CVMLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F34_CVM[0], pobTran->srEMVRec.in9F34_CVMLen);
		inPacketCnt += pobTran->srEMVRec.in9F34_CVMLen;

		/* 9F35 */
		if (pobTran->srEMVRec.in9F35_TermTypeLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9F35 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9F;
		szPacket[inPacketCnt ++] = 0x35;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F35_TermTypeLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F35_TermType[0], pobTran->srEMVRec.in9F35_TermTypeLen);
		inPacketCnt += pobTran->srEMVRec.in9F35_TermTypeLen;

		/* 9F36 */
		if (pobTran->srEMVRec.in9F36_ATCLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9F36 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9F;
		szPacket[inPacketCnt ++] = 0x36;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F36_ATCLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F36_ATC[0], pobTran->srEMVRec.in9F36_ATCLen);
		inPacketCnt += pobTran->srEMVRec.in9F36_ATCLen;

		/* 9F37 */
		if (pobTran->srEMVRec.in9F37_UnpredictNumLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9F37 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9F;
		szPacket[inPacketCnt ++] = 0x37;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F37_UnpredictNumLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F37_UnpredictNum[0], pobTran->srEMVRec.in9F37_UnpredictNumLen);
		inPacketCnt += pobTran->srEMVRec.in9F37_UnpredictNumLen;

		/* 9F41 */
		if (pobTran->srEMVRec.in9F41_TransSeqCounterLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x41;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F41_TransSeqCounterLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F41_TransSeqCounter[0], pobTran->srEMVRec.in9F41_TransSeqCounterLen);
			inPacketCnt += pobTran->srEMVRec.in9F41_TransSeqCounterLen;
		}

		/* 9F5B */
		if (!memcmp(&guszMFES_MTI[0], "0200", 4)	||
		    !memcmp(&guszMFES_MTI[0], "0220", 4)	||
		    !memcmp(&guszMFES_MTI[0], "0320", 4))
		{
			if (pobTran->srEMVRec.in9F5B_ISRLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x5B;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F5B_ISRLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F5B_ISR[0], pobTran->srEMVRec.in9F5B_ISRLen);
				inPacketCnt += pobTran->srEMVRec.in9F5B_ISRLen;
			}
		}
			
		/* 9F63 */
		if (pobTran->srEMVRec.in9F63_CardProductLabelInformationLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x63;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F63_CardProductLabelInformationLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F63_CardProductLabelInformation[0], pobTran->srEMVRec.in9F63_CardProductLabelInformationLen);
			inPacketCnt += pobTran->srEMVRec.in9F63_CardProductLabelInformationLen;
		}

	}
	
	/* 回復原MTI (START) */
	if (pobTran->inISOTxnCode == _REVERSAL_)
	{
		for (inBitMapIndex = 0;; inBitMapIndex++)
		{
			if (srNCCC_MFES_ISOBitMap[inBitMapIndex].inTxnID == _NCCC_MFES_NULL_TX_)
			{
				return (VS_ERROR); /* 找不到相對應交易別的Bit Map */
			}

			if (srNCCC_MFES_ISOBitMap[inBitMapIndex].inTxnID == pobTran->inISOTxnCode)
				break; /* 找到一樣的交易類別 */
		}
		
		memcpy(guszMFES_MTI, srNCCC_MFES_ISOBitMap[inBitMapIndex].szMTI, 4);
	}
	/* 回復原MTI (END) */

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
                inLogPrintf(AT, "inNCCC_MFES_Pack55_tSAM() END!");
	
	return (inCnt);
}

/*
Function        :inNCCC_MFES_Pack55_Software
Date&Time       :2016/12/23 上午 11:54
Describe        :Field_55:	EMV Chip Data
*/
int inNCCC_MFES_Pack55_Software(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int		inBitMapIndex;
	int		inSoftwareKey;
	int		i;
	int		inCnt = 0, inPacketCnt = 0;
	int		inContactlessType = 0;
	char		szASCII[128 + 1];
	char		szSTAN[6 + 1], szField11[3 + 1];
	char		szPacket[512 + 1];
	char		szDebugMsg[100 + 1];
        unsigned char	uszBCD[2 + 1];
	unsigned char	szEncrypt[200 + 1];
	unsigned short	usLen = 0;

	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_Pack55_Software() START!");

	/* 開始加密 */
	memset(szField11, 0x00, sizeof(szField11));
	memset(szSTAN, 0x00, sizeof(szSTAN));
	
	if (pobTran->inISOTxnCode == _SEND_ESC_ADVICE_)
	{
                /* For ESC電子簽單，Advice 使用當下STAN */
		inGetSTANNum(szSTAN);
	}
	else
	{
		sprintf(szSTAN, "%06ld", pobTran->srBRec.lnSTANNum);
        }
	inFunc_ASCII_to_BCD((unsigned char*)szSTAN, szField11, 3);
	
	
	memset(szPacket, 0x00, sizeof(szPacket));
	
	/* 因為把F55的檢查寫的較嚴格，所以沒考慮到Reversal的狀況，所以若Reversal跑到F55時，先換回原MTI (START) */
	if (pobTran->inISOTxnCode == _REVERSAL_)
	{
		for (inBitMapIndex = 0;; inBitMapIndex++)
		{
			if (srNCCC_MFES_ISOBitMap[inBitMapIndex].inTxnID == _NCCC_MFES_NULL_TX_)
			{
				return (VS_ERROR); /* 找不到相對應交易別的Bit Map */
			}

			if (srNCCC_MFES_ISOBitMap[inBitMapIndex].inTxnID == pobTran->inTransactionCode)
				break; /* 找到一樣的交易類別 */
		}
		
		memcpy(guszMFES_MTI, srNCCC_MFES_ISOBitMap[inBitMapIndex].szMTI, 4);
	}
	/* 因為把F55的檢查寫的較嚴格，所以沒考慮到Reversal的狀況，所以若Reversal跑到F55時，先換回原MTI (END) */
	
	/*
		(1) Tag 9F5B若該筆晶片交易有執行Issure Script Update，除原本TC Upload要上傳 9F5B(ISR)之外，該筆交易的取消或小費交易也要上傳此 Tag。
		(2) Fallback交易DE55僅需上傳Tag DFED、Tag DFEE及Tag DFEF。
		(3) C1:晶片卡 Fallback 的小費交易比照原交易需上傳Tag DFED、Tag DFEE及Tag DFEF。
		(4) Tag 9F63 若UICC卡片包含此Tag時，EDC則應支援該Tag上傳。

		感應交易上傳之 Tag 9F33 的 Value 規則如下：
		(1) VISA Paywave 1/VISA Paywave 3/JSpeedy 請上傳Terminal 之 EMV 參數（因為 V & J 感應交易Reader 不會回 Tag 9F33，故以端末機EMV參數上傳）。
		(2) Paypass 因為 Reader 會回覆 Tag 9F33 ，所以依Reader 回覆資料上傳。
		(3) (O) option 欄位，若是Reader有回覆則必須上傳。
		(4) GHL V3R6 Reader感應Paypass不會回覆Tag9F03，所以此Tag依Reader實際回覆資料上傳。
		UICC的void sale 和void preauth 不上傳此欄位。
	*/
	
	if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
	{
		switch (pobTran->srBRec.uszWAVESchemeID)
		{
			case SCHEME_ID_20_PAYPASS_MAG_STRIPE:
			case SCHEME_ID_21_PAYPASS_MCHIP:
				inContactlessType = 1;
				break;
			case SCHEME_ID_16_WAVE1:
				inContactlessType = 2;
				break;
			case SCHEME_ID_17_WAVE3:
				inContactlessType = 3;
				break;
			case SCHEME_ID_61_JSPEEDY:
			case SCHEME_ID_65_NEWJSPEEDY_LEGACY:	/* NCCC OLDJ 和NEW J C卡分同一類 a*/
				inContactlessType = 4;
				break;
			case SCHEME_ID_63_NEWJSPEEDY_EMV:
				inContactlessType = 5;
				break;
			case SCHEME_ID_91_QUICKPASS :	/* 0x91 */
				inContactlessType = 6;
				break;
			case SCHEME_ID_50_EXPRESSSPAY :
			case SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE:
				inContactlessType = 7;
				break;
			case SCHEME_ID_42_DPAS :
			case SCHEME_ID_43_DISCOVER_DPAS_MAG_STRIPE :
				inContactlessType = 8;
				break;
			default :
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "F55_未支援的感應卡別");
				}
				inUtility_StoreTraceLog_OneStep("F55_未支援的感應卡別");
				return (VS_ERROR);
				break;
		}
		
		/* 感應只有paypass要送，正向送57，其他符合0200、0220、0320、0400的話送5A和5F24 */
		if (inContactlessType == 1)
		{
			/* 比照520 2020/2/7 下午 4:50 */
			if (pobTran->uszTCUploadBit != VS_TRUE		&&
		            (pobTran->inISOTxnCode == _SALE_		||	/* 原來只判斷MTI，但退貨和取消不用，所以下面用加了用ISOTXNCODE來判別 */
		             pobTran->inISOTxnCode == _PRE_AUTH_	||
			     pobTran->inISOTxnCode == _REDEEM_SALE_	||
		             pobTran->inISOTxnCode == _INST_SALE_	||
	                     pobTran->inISOTxnCode == _CUP_SALE_	||
			     pobTran->inISOTxnCode == _CUP_PRE_AUTH_))
			{
				if (pobTran->in57_Track2Len <= 0)
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "F 55 TAG 57 ERROR!!");
						inLogPrintf(AT, szDebugMsg);
					}
					inUtility_StoreTraceLog_OneStep("F 55 TAG 57 ERROR!!");

					return (VS_ERROR);
				}

				memset(szEncrypt, 0x00, sizeof(szEncrypt));
				memcpy(szEncrypt, pobTran->usz57_Track2, pobTran->in57_Track2Len);
				usLen = pobTran->in57_Track2Len;

				inSoftwareKey = (unsigned char)szField11[2] & 0x0F;

				for (i = 0; i < (usLen / 2); i ++)
				{
					szEncrypt[i] = szEncrypt[i] ^ inSoftwareKey;
					inSoftwareKey ++;

					if (inSoftwareKey > 9)
						inSoftwareKey = 0;
				}

				szPacket[inPacketCnt ++] = 0x57;
				szPacket[inPacketCnt ++] = usLen;
				memcpy(&szPacket[inPacketCnt], szEncrypt, usLen);
				inPacketCnt += usLen;
			}
			else
			{
				/* 5A */
				/* 比照520 2020/2/7 下午 4:50 */
				if (!memcmp(guszMFES_MTI, "0200", 4)	||
				    !memcmp(guszMFES_MTI, "0220", 4)	||
				    !memcmp(guszMFES_MTI, "0320", 4)	||
				    !memcmp(guszMFES_MTI, "0400", 4))
				{
					if (pobTran->srEMVRec.in5A_ApplPanLen <= 0)
					{
						if (ginDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
							sprintf(szDebugMsg, "F 55 TAG 5A ERROR!!");
							inLogPrintf(AT, szDebugMsg);
						}
						inUtility_StoreTraceLog_OneStep("F 55 TAG 5A ERROR!!");

						return (VS_ERROR);
					}

					memset(szEncrypt, 0x00, sizeof(szEncrypt));
					memcpy(szEncrypt, &pobTran->srEMVRec.usz5A_ApplPan[0], pobTran->srEMVRec.in5A_ApplPanLen);
					usLen = pobTran->srEMVRec.in5A_ApplPanLen;

					inSoftwareKey = (unsigned char)szField11[2] & 0x0F;
					for (i = 0; i < (usLen / 2); i ++)
					{
						szEncrypt[i] = szEncrypt[i] ^ inSoftwareKey;
						inSoftwareKey ++;

						if (inSoftwareKey > 9)
							inSoftwareKey = 0;
					}

					szPacket[inPacketCnt ++] = 0x5A;
					szPacket[inPacketCnt ++] = usLen;
					memcpy(&szPacket[inPacketCnt], szEncrypt, usLen);
					inPacketCnt += usLen;
				}

				/* 5F24 */
				/* 比照520 2020/2/7 下午 4:50 */
				if (!memcmp(guszMFES_MTI, "0200", 4)	||
				    !memcmp(guszMFES_MTI, "0220", 4)	||
				    !memcmp(guszMFES_MTI, "0320", 4)	||
				    !memcmp(guszMFES_MTI, "0400", 4))
				{
					if (pobTran->srEMVRec.in5F24_ExpireDateLen <= 0)
					{
						if (ginDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
							sprintf(szDebugMsg, "F 55 TAG 5F24 ERROR!!");
							inLogPrintf(AT, szDebugMsg);
						}
						inUtility_StoreTraceLog_OneStep("F 55 TAG 5F24 ERROR!!");

						return (VS_ERROR);
					}

					memset(szEncrypt, 0x00, sizeof(szEncrypt));
					memcpy(szEncrypt, &pobTran->srEMVRec.usz5F24_ExpireDate[0], pobTran->srEMVRec.in5F24_ExpireDateLen);
					usLen = pobTran->srEMVRec.in5F24_ExpireDateLen;

					inSoftwareKey = (unsigned char)szField11[2] & 0x0F;
					for (i = 0; i < (usLen / 2); i ++)
					{
						szEncrypt[i] = szEncrypt[i] ^ inSoftwareKey;
						inSoftwareKey ++;

						if (inSoftwareKey > 9)
							inSoftwareKey = 0;
					}

					szPacket[inPacketCnt ++] = 0x5F;
					szPacket[inPacketCnt ++] = 0x24;
					szPacket[inPacketCnt ++] = usLen;
					memcpy(&szPacket[inPacketCnt], szEncrypt, usLen);
					inPacketCnt += usLen;
				}
			}
		}
		
		/* 5F2A */
		if (pobTran->srEMVRec.in5F2A_TransCurrCodeLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 5F2A ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}
			inUtility_StoreTraceLog_OneStep("F 55 TAG 5F2A ERROR!!");

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x5F;
		szPacket[inPacketCnt ++] = 0x2A;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in5F2A_TransCurrCodeLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz5F2A_TransCurrCode[0], pobTran->srEMVRec.in5F2A_TransCurrCodeLen);
		inPacketCnt += pobTran->srEMVRec.in5F2A_TransCurrCodeLen;
                
		/* 5F34 */
		if (inContactlessType == 1	||
		    inContactlessType == 2	||
		    inContactlessType == 4)
		{
			if (pobTran->srEMVRec.in5F34_ApplPanSeqnumLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x5F;
				szPacket[inPacketCnt ++] = 0x34;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in5F34_ApplPanSeqnumLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz5F34_ApplPanSeqnum[0], pobTran->srEMVRec.in5F34_ApplPanSeqnumLen);
				inPacketCnt += pobTran->srEMVRec.in5F34_ApplPanSeqnumLen;
			}
		}
		else if (inContactlessType == 3)
		{
			if (pobTran->srEMVRec.in5F34_ApplPanSeqnumLen <= 0)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "F 55 TAG 5F34 ERROR!!");
					inLogPrintf(AT, szDebugMsg);
				}
				inUtility_StoreTraceLog_OneStep("F 55 TAG 5F34 ERROR!!");

				return (VS_ERROR);
			}
			szPacket[inPacketCnt ++] = 0x5F;
			szPacket[inPacketCnt ++] = 0x34;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in5F34_ApplPanSeqnumLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz5F34_ApplPanSeqnum[0], pobTran->srEMVRec.in5F34_ApplPanSeqnumLen);
			inPacketCnt += pobTran->srEMVRec.in5F34_ApplPanSeqnumLen;
		}
		/* 規格沒寫抄Verfone的，有才送 */
		else
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "SPEC not specific");
			}
			
			if (pobTran->srEMVRec.in5F34_ApplPanSeqnumLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x5F;
				szPacket[inPacketCnt ++] = 0x34;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in5F34_ApplPanSeqnumLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz5F34_ApplPanSeqnum[0], pobTran->srEMVRec.in5F34_ApplPanSeqnumLen);
				inPacketCnt += pobTran->srEMVRec.in5F34_ApplPanSeqnumLen;
			}
		}
		
		/* 82 */
		if (pobTran->srEMVRec.in82_AIPLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 82 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}
			inUtility_StoreTraceLog_OneStep("F 55 TAG 82 ERROR!!");

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x82;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in82_AIPLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz82_AIP[0], pobTran->srEMVRec.in82_AIPLen);
		inPacketCnt += pobTran->srEMVRec.in82_AIPLen;
	
		/* 84 */
		if (inContactlessType == 1)
		{
			if (pobTran->srEMVRec.in84_DFNameLen <= 0)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "F 55 TAG 84 ERROR!!");
					inLogPrintf(AT, szDebugMsg);
				}
				inUtility_StoreTraceLog_OneStep("F 55 TAG 84 ERROR!!");

				return (VS_ERROR);
			}
			szPacket[inPacketCnt ++] = 0x84;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in84_DFNameLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz84_DF_NAME[0], pobTran->srEMVRec.in84_DFNameLen);
			inPacketCnt += pobTran->srEMVRec.in84_DFNameLen;
		}
		else if (inContactlessType == 2	||
			 inContactlessType == 4 ||
			 inContactlessType == 5)
		{
			if (!memcmp(guszMFES_MTI, "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
			{
				/* JSPEEDY NEWJ WAVE1 OFFLINE APPROVED不送 */
			}
			else
			{
				if (pobTran->srEMVRec.in84_DFNameLen > 0)
				{
					szPacket[inPacketCnt ++] = 0x84;
					szPacket[inPacketCnt ++] = pobTran->srEMVRec.in84_DFNameLen;
					memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz84_DF_NAME[0], pobTran->srEMVRec.in84_DFNameLen);
					inPacketCnt += pobTran->srEMVRec.in84_DFNameLen;
				}
			}
		}
		else if (inContactlessType == 3)
		{
			if (pobTran->srEMVRec.in84_DFNameLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x84;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in84_DFNameLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz84_DF_NAME[0], pobTran->srEMVRec.in84_DFNameLen);
				inPacketCnt += pobTran->srEMVRec.in84_DFNameLen;
			}
		}
		/* 規格沒寫抄Verfone的，有才送 */
		else
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "SPEC not specific");
			}
			
			if (pobTran->srEMVRec.in84_DFNameLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x84;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in84_DFNameLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz84_DF_NAME[0], pobTran->srEMVRec.in84_DFNameLen);
				inPacketCnt += pobTran->srEMVRec.in84_DFNameLen;
			}
		}

		/* 8A */
		if (inContactlessType == 1)
		{
			if (!memcmp(guszMFES_MTI, "0220", 4) || !memcmp(guszMFES_MTI, "0320", 4))
			{
				if (pobTran->srEMVRec.in8A_AuthRespCodeLen <= 0)
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "F 55 TAG 8A ERROR!!");
						inLogPrintf(AT, szDebugMsg);
					}
					inUtility_StoreTraceLog_OneStep("F 55 TAG 8A ERROR!!");

					return (VS_ERROR);
				}
				szPacket[inPacketCnt ++] = 0x8A;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in8A_AuthRespCodeLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz8A_AuthRespCode[0], pobTran->srEMVRec.in8A_AuthRespCodeLen);
				inPacketCnt += pobTran->srEMVRec.in8A_AuthRespCodeLen;
			}
		
		}
		else if (inContactlessType == 2	||
			 inContactlessType == 4 ||
			 inContactlessType == 5)
		{
			if (!memcmp(guszMFES_MTI, "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
			{
				/* WAVE1 JSPEEDY NEWJ OFFLINE APPROVED不送 */
			}
			else if (!memcmp(guszMFES_MTI, "0200", 4))
			{
				/* WAVE1 JSPEEDY NEWJ 0200不送 */
			}
			else
			{
				if (pobTran->srEMVRec.in8A_AuthRespCodeLen > 0)
				{
					szPacket[inPacketCnt ++] = 0x8A;
					szPacket[inPacketCnt ++] = pobTran->srEMVRec.in8A_AuthRespCodeLen;
					memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz8A_AuthRespCode[0], pobTran->srEMVRec.in8A_AuthRespCodeLen);
					inPacketCnt += pobTran->srEMVRec.in8A_AuthRespCodeLen;
				}
			}
			
		}
		else if (inContactlessType == 3	||
			 inContactlessType == 6	||
			 inContactlessType == 7	||
			 inContactlessType == 8)
		{
			if (!memcmp(guszMFES_MTI, "0200", 4))
			{
				/* WAVE 3 QUICKPASS 不送 */
			}
			else
			{
				if (pobTran->srEMVRec.in8A_AuthRespCodeLen > 0)
				{
					szPacket[inPacketCnt ++] = 0x8A;
					szPacket[inPacketCnt ++] = pobTran->srEMVRec.in8A_AuthRespCodeLen;
					memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz8A_AuthRespCode[0], pobTran->srEMVRec.in8A_AuthRespCodeLen);
					inPacketCnt += pobTran->srEMVRec.in8A_AuthRespCodeLen;
				}
			}
		}
		/* 規格沒寫抄Verfone的，有才送 */
		else
		{	
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "SPEC not specific");
			}
			if (pobTran->srEMVRec.in8A_AuthRespCodeLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x8A;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in8A_AuthRespCodeLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz8A_AuthRespCode[0], pobTran->srEMVRec.in8A_AuthRespCodeLen);
				inPacketCnt += pobTran->srEMVRec.in8A_AuthRespCodeLen;
			}
		}
		
		/* 95 */
		if (pobTran->srEMVRec.in95_TVRLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 95 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}
			inUtility_StoreTraceLog_OneStep("F 55 TAG 95 ERROR!!");

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x95;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in95_TVRLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz95_TVR[0], pobTran->srEMVRec.in95_TVRLen);
		inPacketCnt += pobTran->srEMVRec.in95_TVRLen;

		/* 9A */
		if (pobTran->srEMVRec.in9A_TranDateLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9A ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}
			inUtility_StoreTraceLog_OneStep("F 55 TAG 9A ERROR!!");

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9A;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9A_TranDateLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9A_TranDate[0], pobTran->srEMVRec.in9A_TranDateLen);
		inPacketCnt += pobTran->srEMVRec.in9A_TranDateLen;
		
		/* 9B */
		if (inContactlessType == 1)
		{
//			if (pobTran->srEMVRec.in9B_TSILen <= 0)
//			{
//				if (ginDebug == VS_TRUE)
//				{
//					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
//					sprintf(szDebugMsg, "F 55 TAG 9B ERROR!!");
//					inLogPrintf(AT, szDebugMsg);
//				}
//				inUtility_StoreTraceLog_OneStep("F 55 TAG 9B ERROR!!");
//
//				return (VS_ERROR);
//			}
			/* 抄Verifone有才送，ATS(含MCAP_ATS)非接觸式交易，目前不會檢查Tag 9B為必要欄位 */
			if (pobTran->srEMVRec.in9B_TSILen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9B;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9B_TSILen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9B_TSI[0], pobTran->srEMVRec.in9B_TSILen);
				inPacketCnt += pobTran->srEMVRec.in9B_TSILen;
			}
		}
		else if (inContactlessType == 2	||
			 inContactlessType == 4 ||
			 inContactlessType == 5)
		{
			if (!memcmp(guszMFES_MTI, "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
			{
				/* JSPEEDY NEWJ WAVE1 OFFLINE APPROVED不送 */
			}
			else
			{
				if (pobTran->srEMVRec.in9B_TSILen > 0)
				{
					szPacket[inPacketCnt ++] = 0x9B;
					szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9B_TSILen;
					memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9B_TSI[0], pobTran->srEMVRec.in9B_TSILen);
					inPacketCnt += pobTran->srEMVRec.in9B_TSILen;
				}
			}
		}
		else if (inContactlessType == 3)
		{
			if (pobTran->srEMVRec.in9B_TSILen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9B;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9B_TSILen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9B_TSI[0], pobTran->srEMVRec.in9B_TSILen);
				inPacketCnt += pobTran->srEMVRec.in9B_TSILen;
			}
		}
		/* 規格沒寫抄Verfone的，有才送 */
		else
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "SPEC not specific");
			}
			
			if (pobTran->srEMVRec.in9B_TSILen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9B;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9B_TSILen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9B_TSI[0], pobTran->srEMVRec.in9B_TSILen);
				inPacketCnt += pobTran->srEMVRec.in9B_TSILen;
			}
		}
		
		/* 9C */
		if (pobTran->srEMVRec.in9C_TranTypeLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9C ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}
			inUtility_StoreTraceLog_OneStep("F 55 TAG 9C ERROR!!");

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9C;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9C_TranTypeLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9C_TranType[0], pobTran->srEMVRec.in9C_TranTypeLen);
		inPacketCnt += pobTran->srEMVRec.in9C_TranTypeLen;

		/* 9F02 */
		if (pobTran->srEMVRec.in9F02_AmtAuthNumLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9F02 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}
			inUtility_StoreTraceLog_OneStep("F 55 TAG 9F02 ERROR!!");

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9F;
		szPacket[inPacketCnt ++] = 0x02;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F02_AmtAuthNumLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F02_AmtAuthNum[0], pobTran->srEMVRec.in9F02_AmtAuthNumLen);
		inPacketCnt += pobTran->srEMVRec.in9F02_AmtAuthNumLen;
		
		/* 9F03 */
		if (pobTran->srEMVRec.in9F03_AmtOtherNumLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x03;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F03_AmtOtherNumLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F03_AmtOtherNum[0], pobTran->srEMVRec.in9F03_AmtOtherNumLen);
			inPacketCnt += pobTran->srEMVRec.in9F03_AmtOtherNumLen;
		}

		/* 9F10 */
		if (pobTran->srEMVRec.in9F10_IssuerAppDataLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9F10 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}
			inUtility_StoreTraceLog_OneStep("F 55 TAG 9F10 ERROR!!");

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9F;
		szPacket[inPacketCnt ++] = 0x10;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F10_IssuerAppDataLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F10_IssuerAppData[0], pobTran->srEMVRec.in9F10_IssuerAppDataLen);
		inPacketCnt += pobTran->srEMVRec.in9F10_IssuerAppDataLen;
		

		/* 9F1A */
		if (pobTran->srEMVRec.in9F1A_TermCountryCodeLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9F1A ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}
			inUtility_StoreTraceLog_OneStep("F 55 TAG 9F1A ERROR!!");

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9F;
		szPacket[inPacketCnt ++] = 0x1A;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F1A_TermCountryCodeLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F1A_TermCountryCode[0], pobTran->srEMVRec.in9F1A_TermCountryCodeLen);
		inPacketCnt += pobTran->srEMVRec.in9F1A_TermCountryCodeLen;
		
		/* 9F1E */
		if (inContactlessType == 1)
		{
			if (pobTran->srEMVRec.in9F1E_IFDNumLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x1E;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F1E_IFDNumLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F1E_IFDNum[0], pobTran->srEMVRec.in9F1E_IFDNumLen);
				inPacketCnt += pobTran->srEMVRec.in9F1E_IFDNumLen;
			}
			
		}
		else if (inContactlessType == 2	||
			 inContactlessType == 4 ||
			 inContactlessType == 5)
		{
			if (!memcmp(guszMFES_MTI, "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
			{
				/* JSPEEDY NEWJ WAVE1 OFFLINE APPROVED不送 */
			}
			else
			{
				if (pobTran->srEMVRec.in9F1E_IFDNumLen > 0)
				{
					szPacket[inPacketCnt ++] = 0x9F;
					szPacket[inPacketCnt ++] = 0x1E;
					szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F1E_IFDNumLen;
					memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F1E_IFDNum[0], pobTran->srEMVRec.in9F1E_IFDNumLen);
					inPacketCnt += pobTran->srEMVRec.in9F1E_IFDNumLen;
				}
			}
		}
		else if (inContactlessType == 3)
		{
			if (pobTran->srEMVRec.in9F1E_IFDNumLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x1E;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F1E_IFDNumLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F1E_IFDNum[0], pobTran->srEMVRec.in9F1E_IFDNumLen);
				inPacketCnt += pobTran->srEMVRec.in9F1E_IFDNumLen;
			}
		}
		/* 規格沒寫抄Verfone的，有才送 */
		else
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "SPEC not specific");
			}
			
			if (pobTran->srEMVRec.in9F1E_IFDNumLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x1E;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F1E_IFDNumLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F1E_IFDNum[0], pobTran->srEMVRec.in9F1E_IFDNumLen);
				inPacketCnt += pobTran->srEMVRec.in9F1E_IFDNumLen;
			}
		
		}

		/* 9F26 */
		if (pobTran->srEMVRec.in9F26_ApplCryptogramLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9F26 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}
			inUtility_StoreTraceLog_OneStep("F 55 TAG 9F26 ERROR!!");

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9F;
		szPacket[inPacketCnt ++] = 0x26;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F26_ApplCryptogramLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F26_ApplCryptogram[0], pobTran->srEMVRec.in9F26_ApplCryptogramLen);
		inPacketCnt += pobTran->srEMVRec.in9F26_ApplCryptogramLen;
		
		/* 9F27 */
		if (inContactlessType == 1)
		{
			if (pobTran->srEMVRec.in9F27_CIDLen <= 0)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "F 55 TAG 9F27 ERROR!!");
					inLogPrintf(AT, szDebugMsg);
				}
				inUtility_StoreTraceLog_OneStep("F 55 TAG 9F27 ERROR!!");

				return (VS_ERROR);
			}
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x27;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F27_CIDLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F27_CID[0], pobTran->srEMVRec.in9F27_CIDLen);
			inPacketCnt += pobTran->srEMVRec.in9F27_CIDLen;
		}
		else
		{
			if (pobTran->srEMVRec.in9F27_CIDLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x27;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F27_CIDLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F27_CID[0], pobTran->srEMVRec.in9F27_CIDLen);
				inPacketCnt += pobTran->srEMVRec.in9F27_CIDLen;
			}
		}
		
		/* 9F33 */
		if (pobTran->srEMVRec.in9F33_TermCapabilitiesLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9F33 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}
			inUtility_StoreTraceLog_OneStep("F 55 TAG 9F33 ERROR!!");

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9F;
		szPacket[inPacketCnt ++] = 0x33;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F33_TermCapabilitiesLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F33_TermCapabilities[0], pobTran->srEMVRec.in9F33_TermCapabilitiesLen);
		inPacketCnt += pobTran->srEMVRec.in9F33_TermCapabilitiesLen;

		/* 9F34 */
		if (inContactlessType == 1)
		{
			if (pobTran->srEMVRec.in9F34_CVMLen <= 0)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "F 55 TAG 9F34 ERROR!!");
					inLogPrintf(AT, szDebugMsg);
				}
				inUtility_StoreTraceLog_OneStep("F 55 TAG 9F34 ERROR!!");

				return (VS_ERROR);
			}
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x34;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F34_CVMLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F34_CVM[0], pobTran->srEMVRec.in9F34_CVMLen);
			inPacketCnt += pobTran->srEMVRec.in9F34_CVMLen;
			
		}
		else if (inContactlessType == 2	||
			 inContactlessType == 4 ||
			 inContactlessType == 5)
		{
			if (!memcmp(guszMFES_MTI, "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
			{
				/* JSPEEDY NEWJ WAVE1 OFFLINE APPROVED不送 */
			}
			else
			{
				if (pobTran->srEMVRec.in9F34_CVMLen > 0)
				{
					szPacket[inPacketCnt ++] = 0x9F;
					szPacket[inPacketCnt ++] = 0x34;
					szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F34_CVMLen;
					memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F34_CVM[0], pobTran->srEMVRec.in9F34_CVMLen);
					inPacketCnt += pobTran->srEMVRec.in9F34_CVMLen;
				}
			}
		}
		else if (inContactlessType == 3)
		{
			if (pobTran->srEMVRec.in9F34_CVMLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x34;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F34_CVMLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F34_CVM[0], pobTran->srEMVRec.in9F34_CVMLen);
				inPacketCnt += pobTran->srEMVRec.in9F34_CVMLen;
			}
		}
		/* 規格沒寫抄Verfone的，有才送 */
		else
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "SPEC not specific");
			}
			
			if (pobTran->srEMVRec.in9F34_CVMLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x34;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F34_CVMLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F34_CVM[0], pobTran->srEMVRec.in9F34_CVMLen);
				inPacketCnt += pobTran->srEMVRec.in9F34_CVMLen;
			}
		
		}
		
		/* 9F35 */
		if (inContactlessType == 1)
		{
			if (pobTran->srEMVRec.in9F35_TermTypeLen <= 0)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "F 55 TAG 9F35 ERROR!!");
					inLogPrintf(AT, szDebugMsg);
				}
				inUtility_StoreTraceLog_OneStep("F 55 TAG 9F35 ERROR!!");

				return (VS_ERROR);
			}
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x35;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F35_TermTypeLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F35_TermType[0], pobTran->srEMVRec.in9F35_TermTypeLen);
			inPacketCnt += pobTran->srEMVRec.in9F35_TermTypeLen;

		}
		else if (inContactlessType == 2	||
			 inContactlessType == 4 ||
			 inContactlessType == 5)
		{
			if (!memcmp(guszMFES_MTI, "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
			{
				/* JSPEEDY NEWJ WAVE1 OFFLINE APPROVED不送 */
			}
			else
			{
				if (pobTran->srEMVRec.in9F35_TermTypeLen > 0)
				{
					szPacket[inPacketCnt ++] = 0x9F;
					szPacket[inPacketCnt ++] = 0x35;
					szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F35_TermTypeLen;
					memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F35_TermType[0], pobTran->srEMVRec.in9F35_TermTypeLen);
					inPacketCnt += pobTran->srEMVRec.in9F35_TermTypeLen;
				}
			}
		}
		else if (inContactlessType == 3)
		{
			if (pobTran->srEMVRec.in9F35_TermTypeLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x35;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F35_TermTypeLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F35_TermType[0], pobTran->srEMVRec.in9F35_TermTypeLen);
				inPacketCnt += pobTran->srEMVRec.in9F35_TermTypeLen;
			}
		}
		/* 規格沒寫抄Verfone的，有才送 */
		else
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "SPEC not specific");
			}
			
			if (pobTran->srEMVRec.in9F35_TermTypeLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x35;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F35_TermTypeLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F35_TermType[0], pobTran->srEMVRec.in9F35_TermTypeLen);
				inPacketCnt += pobTran->srEMVRec.in9F35_TermTypeLen;
			}
		
		}

		/* 9F36 */
		if (pobTran->srEMVRec.in9F36_ATCLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9F36 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}
			inUtility_StoreTraceLog_OneStep("F 55 TAG 9F36 ERROR!!");

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9F;
		szPacket[inPacketCnt ++] = 0x36;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F36_ATCLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F36_ATC[0], pobTran->srEMVRec.in9F36_ATCLen);
		inPacketCnt += pobTran->srEMVRec.in9F36_ATCLen;
		
		/* 9F37 */
		if (pobTran->srEMVRec.in9F37_UnpredictNumLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9F37 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}
			inUtility_StoreTraceLog_OneStep("F 55 TAG 9F37 ERROR!!");

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9F;
		szPacket[inPacketCnt ++] = 0x37;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F37_UnpredictNumLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F37_UnpredictNum[0], pobTran->srEMVRec.in9F37_UnpredictNumLen);
		inPacketCnt += pobTran->srEMVRec.in9F37_UnpredictNumLen;

		/* 9F41 */
		if (inContactlessType == 1)
		{
			if (pobTran->srEMVRec.in9F41_TransSeqCounterLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x41;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F41_TransSeqCounterLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F41_TransSeqCounter[0], pobTran->srEMVRec.in9F41_TransSeqCounterLen);
				inPacketCnt += pobTran->srEMVRec.in9F41_TransSeqCounterLen;
			}
			
		}
		else if (inContactlessType == 2	||
			 inContactlessType == 4 ||
			 inContactlessType == 5)
		{
			if (!memcmp(guszMFES_MTI, "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
			{
				/* JSPEEDY NEWJ WAVE1 OFFLINE APPROVED不送 */
			}
			else
			{
				if (pobTran->srEMVRec.in9F41_TransSeqCounterLen > 0)
				{
					szPacket[inPacketCnt ++] = 0x9F;
					szPacket[inPacketCnt ++] = 0x41;
					szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F41_TransSeqCounterLen;
					memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F41_TransSeqCounter[0], pobTran->srEMVRec.in9F41_TransSeqCounterLen);
					inPacketCnt += pobTran->srEMVRec.in9F41_TransSeqCounterLen;
				}
			}
		}
		else if (inContactlessType == 3)
		{
			if (pobTran->srEMVRec.in9F41_TransSeqCounterLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x41;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F41_TransSeqCounterLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F41_TransSeqCounter[0], pobTran->srEMVRec.in9F41_TransSeqCounterLen);
				inPacketCnt += pobTran->srEMVRec.in9F41_TransSeqCounterLen;
			}
		}
		/* 規格沒寫抄Verfone的，有才送 */
		else
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "SPEC not specific");
			}
			
			if (pobTran->srEMVRec.in9F41_TransSeqCounterLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x41;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F41_TransSeqCounterLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F41_TransSeqCounter[0], pobTran->srEMVRec.in9F41_TransSeqCounterLen);
				inPacketCnt += pobTran->srEMVRec.in9F41_TransSeqCounterLen;
			}
		
		}
				
                /* 【需求單103399】VCPS2.1.3規範， DE55新增上傳 FFI (Tag 9F6E) */
		if (pobTran->srEMVRec.in9F6E_From_Factor_IndicatorLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x6E;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F6E_From_Factor_IndicatorLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F6E_From_Factor_Indicator[0], pobTran->srEMVRec.in9F6E_From_Factor_IndicatorLen);
			inPacketCnt += pobTran->srEMVRec.in9F6E_From_Factor_IndicatorLen;
		}
		
		/* (需求單-110315)-新增支援晶片9F6E及9F7C欄位需求 by Russell 2022/1/17 上午 11:21 */
		if (pobTran->srEMVRec.in9F7C_PartnerDiscretionaryDataLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x7C;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F7C_PartnerDiscretionaryDataLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F7C_PartnerDiscretionaryData[0], pobTran->srEMVRec.in9F7C_PartnerDiscretionaryDataLen);
			inPacketCnt += pobTran->srEMVRec.in9F7C_PartnerDiscretionaryDataLen;
		}
	}
	else if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
	{
		/* 接觸式 正向Mastercard帶57 其他狀況不論卡別都帶5A和5F24 */
		/* 57 */
		if (pobTran->uszTCUploadBit != VS_TRUE		&&
		    (pobTran->inISOTxnCode == _SALE_		||	/* 原來只判斷MTI，但退貨和取消不用，所以下面用加了用ISOTXNCODE來判別 */
		     pobTran->inISOTxnCode == _PRE_AUTH_	||
		     pobTran->inISOTxnCode == _REDEEM_SALE_	||
		     pobTran->inISOTxnCode == _INST_SALE_	||
	             pobTran->inISOTxnCode == _CUP_SALE_	||
		     pobTran->inISOTxnCode == _CUP_PRE_AUTH_))
		{
			if (pobTran->in57_Track2Len <= 0)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "F 55 TAG 57 ERROR!!");
					inLogPrintf(AT, szDebugMsg);
				}

				return (VS_ERROR);
			}

			memset(szEncrypt, 0x00, sizeof(szEncrypt));
			memcpy(szEncrypt, pobTran->usz57_Track2, pobTran->in57_Track2Len);
			usLen = pobTran->in57_Track2Len;

			inSoftwareKey = (unsigned char)szField11[2] & 0x0F;

			for (i = 0; i < (usLen / 2); i ++)
			{
				szEncrypt[i] = szEncrypt[i] ^ inSoftwareKey;
				inSoftwareKey ++;

				if (inSoftwareKey > 9)
					inSoftwareKey = 0;
			}

			szPacket[inPacketCnt ++] = 0x57;
			szPacket[inPacketCnt ++] = usLen;
			memcpy(&szPacket[inPacketCnt], szEncrypt, usLen);
			inPacketCnt += usLen;
		}
		else
		{
			/* 5A */
			/* 比照520 2020/2/7 下午 4:50 */
			if (pobTran->srEMVRec.in5A_ApplPanLen <= 0)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "F 55 TAG 5A ERROR!!");
					inLogPrintf(AT, szDebugMsg);
				}

				return (VS_ERROR);
			}

			memset(szEncrypt, 0x00, sizeof(szEncrypt));
			memcpy(szEncrypt, &pobTran->srEMVRec.usz5A_ApplPan[0], pobTran->srEMVRec.in5A_ApplPanLen);
			usLen = pobTran->srEMVRec.in5A_ApplPanLen;

			inSoftwareKey = (unsigned char)szField11[2] & 0x0F;
			for (i = 0; i < (usLen / 2); i ++)
			{
				szEncrypt[i] = szEncrypt[i] ^ inSoftwareKey;
				inSoftwareKey ++;

				if (inSoftwareKey > 9)
					inSoftwareKey = 0;
			}

			szPacket[inPacketCnt ++] = 0x5A;
			szPacket[inPacketCnt ++] = usLen;
			memcpy(&szPacket[inPacketCnt], szEncrypt, usLen);
			inPacketCnt += usLen;

			/* 5F24 */
			/* 比照520 2020/2/7 下午 4:50 */
			if (pobTran->srEMVRec.in5F24_ExpireDateLen <= 0)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "F 55 TAG 5F24 ERROR!!");
					inLogPrintf(AT, szDebugMsg);
				}

				return (VS_ERROR);
			}

			memset(szEncrypt, 0x00, sizeof(szEncrypt));
			memcpy(szEncrypt, &pobTran->srEMVRec.usz5F24_ExpireDate[0], pobTran->srEMVRec.in5F24_ExpireDateLen);
			usLen = pobTran->srEMVRec.in5F24_ExpireDateLen;

			inSoftwareKey = (unsigned char)szField11[2] & 0x0F;
			for (i = 0; i < (usLen / 2); i ++)
			{
				szEncrypt[i] = szEncrypt[i] ^ inSoftwareKey;
				inSoftwareKey ++;

				if (inSoftwareKey > 9)
					inSoftwareKey = 0;
			}

			szPacket[inPacketCnt ++] = 0x5F;
			szPacket[inPacketCnt ++] = 0x24;
			szPacket[inPacketCnt ++] = usLen;
			memcpy(&szPacket[inPacketCnt], szEncrypt, usLen);
			inPacketCnt += usLen;
		}
		
		/* 5F2A */
		if (pobTran->srEMVRec.in5F2A_TransCurrCodeLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 5F2A ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x5F;
		szPacket[inPacketCnt ++] = 0x2A;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in5F2A_TransCurrCodeLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz5F2A_TransCurrCode[0], pobTran->srEMVRec.in5F2A_TransCurrCodeLen);
		inPacketCnt += pobTran->srEMVRec.in5F2A_TransCurrCodeLen;

		/* 5F34 */
		if (pobTran->srEMVRec.in5F34_ApplPanSeqnumLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x5F;
			szPacket[inPacketCnt ++] = 0x34;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in5F34_ApplPanSeqnumLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz5F34_ApplPanSeqnum[0], pobTran->srEMVRec.in5F34_ApplPanSeqnumLen);
			inPacketCnt += pobTran->srEMVRec.in5F34_ApplPanSeqnumLen;
		}

		/* 82 */
		if (pobTran->srEMVRec.in82_AIPLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 82 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x82;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in82_AIPLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz82_AIP[0], pobTran->srEMVRec.in82_AIPLen);
		inPacketCnt += pobTran->srEMVRec.in82_AIPLen;

		/* 84 */
		if (pobTran->srEMVRec.in84_DFNameLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 84 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x84;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in84_DFNameLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz84_DF_NAME[0], pobTran->srEMVRec.in84_DFNameLen);
		inPacketCnt += pobTran->srEMVRec.in84_DFNameLen;

		/* 8A */
		/* MIT = 〈0220〉 〈0320〉要送 */
		if (!memcmp(&guszMFES_MTI[0], "0220", 4) || !memcmp(&guszMFES_MTI[0], "0320", 4))
		{
			if (pobTran->srEMVRec.in8A_AuthRespCodeLen <= 0)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "F 55 TAG 8A ERROR!!");
					inLogPrintf(AT, szDebugMsg);
				}

				return (VS_ERROR);
			}
			szPacket[inPacketCnt ++] = 0x8A;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in8A_AuthRespCodeLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz8A_AuthRespCode[0], pobTran->srEMVRec.in8A_AuthRespCodeLen);
			inPacketCnt += pobTran->srEMVRec.in8A_AuthRespCodeLen;
			
		}

		/* 95 */
		if (pobTran->srEMVRec.in95_TVRLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 95 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x95;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in95_TVRLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz95_TVR[0], pobTran->srEMVRec.in95_TVRLen);
		inPacketCnt += pobTran->srEMVRec.in95_TVRLen;
		

		/* 9A */
		if (pobTran->srEMVRec.in9A_TranDateLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9A ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9A;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9A_TranDateLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9A_TranDate[0], pobTran->srEMVRec.in9A_TranDateLen);
		inPacketCnt += pobTran->srEMVRec.in9A_TranDateLen;

		/* 9B */
		if (pobTran->srEMVRec.in9B_TSILen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9B ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9B;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9B_TSILen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9B_TSI[0], pobTran->srEMVRec.in9B_TSILen);
		inPacketCnt += pobTran->srEMVRec.in9B_TSILen;

		/* 9C */
		if (pobTran->srEMVRec.in9C_TranTypeLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9C ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9C;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9C_TranTypeLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9C_TranType[0], pobTran->srEMVRec.in9C_TranTypeLen);
		inPacketCnt += pobTran->srEMVRec.in9C_TranTypeLen;

		/* 9F02 */
		if (pobTran->srEMVRec.in9F02_AmtAuthNumLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9F02 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9F;
		szPacket[inPacketCnt ++] = 0x02;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F02_AmtAuthNumLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F02_AmtAuthNum[0], pobTran->srEMVRec.in9F02_AmtAuthNumLen);
		inPacketCnt += pobTran->srEMVRec.in9F02_AmtAuthNumLen;

		/* 9F03 */
		if (pobTran->srEMVRec.in9F03_AmtOtherNumLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9F03 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9F;
		szPacket[inPacketCnt ++] = 0x03;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F03_AmtOtherNumLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F03_AmtOtherNum[0], pobTran->srEMVRec.in9F03_AmtOtherNumLen);
		inPacketCnt += pobTran->srEMVRec.in9F03_AmtOtherNumLen;

		/* 9F09 */
		if (pobTran->srEMVRec.in9F09_TermVerNumLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9F09 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9F;
		szPacket[inPacketCnt ++] = 0x09;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F09_TermVerNumLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F09_TermVerNum[0], pobTran->srEMVRec.in9F09_TermVerNumLen);
		inPacketCnt += pobTran->srEMVRec.in9F09_TermVerNumLen;

		/* 9F10 */
		if (pobTran->srEMVRec.in9F10_IssuerAppDataLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9F10 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9F;
		szPacket[inPacketCnt ++] = 0x10;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F10_IssuerAppDataLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F10_IssuerAppData[0], pobTran->srEMVRec.in9F10_IssuerAppDataLen);
		inPacketCnt += pobTran->srEMVRec.in9F10_IssuerAppDataLen;

		/* 9F1A */
		if (pobTran->srEMVRec.in9F1A_TermCountryCodeLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9F1A ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9F;
		szPacket[inPacketCnt ++] = 0x1A;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F1A_TermCountryCodeLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F1A_TermCountryCode[0], pobTran->srEMVRec.in9F1A_TermCountryCodeLen);
		inPacketCnt += pobTran->srEMVRec.in9F1A_TermCountryCodeLen;

		/* 9F1E */
		if (pobTran->srEMVRec.in9F1E_IFDNumLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x1E;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F1E_IFDNumLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F1E_IFDNum[0], pobTran->srEMVRec.in9F1E_IFDNumLen);
			inPacketCnt += pobTran->srEMVRec.in9F1E_IFDNumLen;
		}

		/* 9F26 */
		if (pobTran->srEMVRec.in9F26_ApplCryptogramLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9F26 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9F;
		szPacket[inPacketCnt ++] = 0x26;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F26_ApplCryptogramLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F26_ApplCryptogram[0], pobTran->srEMVRec.in9F26_ApplCryptogramLen);
		inPacketCnt += pobTran->srEMVRec.in9F26_ApplCryptogramLen;

		/* 9F27 */
		if (pobTran->srEMVRec.in9F27_CIDLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9F27 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9F;
		szPacket[inPacketCnt ++] = 0x27;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F27_CIDLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F27_CID[0], pobTran->srEMVRec.in9F27_CIDLen);
		inPacketCnt += pobTran->srEMVRec.in9F27_CIDLen;

		/* 9F33 */
		if (pobTran->srEMVRec.in9F33_TermCapabilitiesLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9F33 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9F;
		szPacket[inPacketCnt ++] = 0x33;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F33_TermCapabilitiesLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F33_TermCapabilities[0], pobTran->srEMVRec.in9F33_TermCapabilitiesLen);
		inPacketCnt += pobTran->srEMVRec.in9F33_TermCapabilitiesLen;

		/* 9F34 */
		if (pobTran->srEMVRec.in9F34_CVMLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9F34 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9F;
		szPacket[inPacketCnt ++] = 0x34;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F34_CVMLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F34_CVM[0], pobTran->srEMVRec.in9F34_CVMLen);
		inPacketCnt += pobTran->srEMVRec.in9F34_CVMLen;

		/* 9F35 */
		if (pobTran->srEMVRec.in9F35_TermTypeLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9F35 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9F;
		szPacket[inPacketCnt ++] = 0x35;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F35_TermTypeLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F35_TermType[0], pobTran->srEMVRec.in9F35_TermTypeLen);
		inPacketCnt += pobTran->srEMVRec.in9F35_TermTypeLen;

		/* 9F36 */
		if (pobTran->srEMVRec.in9F36_ATCLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9F36 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9F;
		szPacket[inPacketCnt ++] = 0x36;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F36_ATCLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F36_ATC[0], pobTran->srEMVRec.in9F36_ATCLen);
		inPacketCnt += pobTran->srEMVRec.in9F36_ATCLen;

		/* 9F37 */
		if (pobTran->srEMVRec.in9F37_UnpredictNumLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 55 TAG 9F37 ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0x9F;
		szPacket[inPacketCnt ++] = 0x37;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F37_UnpredictNumLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F37_UnpredictNum[0], pobTran->srEMVRec.in9F37_UnpredictNumLen);
		inPacketCnt += pobTran->srEMVRec.in9F37_UnpredictNumLen;

		/* 9F41 */
		if (pobTran->srEMVRec.in9F41_TransSeqCounterLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x41;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F41_TransSeqCounterLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F41_TransSeqCounter[0], pobTran->srEMVRec.in9F41_TransSeqCounterLen);
			inPacketCnt += pobTran->srEMVRec.in9F41_TransSeqCounterLen;
		}

		/* 9F5B */
		if (!memcmp(&guszMFES_MTI[0], "0200", 4)	||
		    !memcmp(&guszMFES_MTI[0], "0220", 4)	||
		    !memcmp(&guszMFES_MTI[0], "0320", 4))
		{
			if (pobTran->srEMVRec.in9F5B_ISRLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x5B;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F5B_ISRLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F5B_ISR[0], pobTran->srEMVRec.in9F5B_ISRLen);
				inPacketCnt += pobTran->srEMVRec.in9F5B_ISRLen;
			}
		}
			
		/* 9F63 */
		if (pobTran->srEMVRec.in9F63_CardProductLabelInformationLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x63;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F63_CardProductLabelInformationLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F63_CardProductLabelInformation[0], pobTran->srEMVRec.in9F63_CardProductLabelInformationLen);
			inPacketCnt += pobTran->srEMVRec.in9F63_CardProductLabelInformationLen;
		}

	}
	
	/* 回復原MTI (START) */
	if (pobTran->inISOTxnCode == _REVERSAL_)
	{
		for (inBitMapIndex = 0;; inBitMapIndex++)
		{
			if (srNCCC_MFES_ISOBitMap[inBitMapIndex].inTxnID == _NCCC_MFES_NULL_TX_)
			{
				return (VS_ERROR); /* 找不到相對應交易別的Bit Map */
			}

			if (srNCCC_MFES_ISOBitMap[inBitMapIndex].inTxnID == pobTran->inISOTxnCode)
				break; /* 找到一樣的交易類別 */
		}
		
		memcpy(guszMFES_MTI, srNCCC_MFES_ISOBitMap[inBitMapIndex].szMTI, 4);
	}
	/* 回復原MTI (END) */

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
                inLogPrintf(AT, "inNCCC_MFES_Pack55_Software() END!");
	
	return (inCnt);
}

/*
Function        :inNCCC_MFES_Pack56
Date&Time       :2016/12/23 上午 11:46
Describe        :EMV Related Data – Private use for NCCC
*/
int inNCCC_MFES_Pack56(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int		inCnt = 0, inPacketCnt = 0;
	char		szASCII[128 + 1];
	char		szTemplate[20 + 1];
	char		szPacket[512 + 1];
	char		szDebugMsg[100 + 1];
        unsigned char	uszBCD[2 + 1];

	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_Pack56() START!");

	memset(szPacket, 0x00, sizeof(szPacket));
	
	/* 晶片Fallback 要多組 */
	if (pobTran->srBRec.uszEMVFallBackBit == VS_TRUE)
	{
		/* DFED */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		if (inFunc_TWNAddDataToEMVPacket(pobTran, TAG_DFED_CHIP_CONDITION_CODE, (unsigned char *)&szTemplate[0]) > 0)
		{
			memset(pobTran->srEMVRec.uszDFED_ChipConditionCode, 0x00, sizeof(pobTran->srEMVRec.uszDFED_ChipConditionCode));
			pobTran->srEMVRec.inDFED_ChipConditionCodeLen = szTemplate[2];
			memcpy((char *)&pobTran->srEMVRec.uszDFED_ChipConditionCode[0], &szTemplate[3], pobTran->srEMVRec.inDFED_ChipConditionCodeLen);
		}
		/* ISO Display Debug */
		if (ginDebug == VS_TRUE)
		{
			memset(szASCII, 0x00, sizeof(szASCII));
			inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.uszDFED_ChipConditionCode[0], pobTran->srEMVRec.inDFED_ChipConditionCodeLen);

			inCTLS_ISOFormatDebug_DISP("DFED", pobTran->srEMVRec.inDFED_ChipConditionCodeLen, szASCII);
		}
		/* ISO Print Debug */
		if (ginISODebug == VS_TRUE)
		{
			memset(szASCII, 0x00, sizeof(szASCII));
			inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.uszDFED_ChipConditionCode[0], pobTran->srEMVRec.inDFED_ChipConditionCodeLen);

			inCTLS_ISOFormatDebug_PRINT("DFED", pobTran->srEMVRec.inDFED_ChipConditionCodeLen, szASCII);
		}

		/* DFEE */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		if (inFunc_TWNAddDataToEMVPacket(pobTran, TAG_DFEE_TERMINAL_ENTRY_CAPABILITY, (unsigned char *)&szTemplate[0]) > 0)
		{
			memset(pobTran->srEMVRec.uszDFEE_TerEntryCap, 0x00, sizeof(pobTran->srEMVRec.uszDFEE_TerEntryCap));
			pobTran->srEMVRec.inDFEE_TerEntryCapLen = szTemplate[2];
			memcpy((char *)&pobTran->srEMVRec.uszDFEE_TerEntryCap[0], &szTemplate[3], pobTran->srEMVRec.inDFEE_TerEntryCapLen);
		}
		/* ISO Display Debug */
		if (ginDebug == VS_TRUE)
		{
			memset(szASCII, 0x00, sizeof(szASCII));
			inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.uszDFEE_TerEntryCap[0], pobTran->srEMVRec.inDFED_ChipConditionCodeLen);

			inCTLS_ISOFormatDebug_DISP("DFEE", pobTran->srEMVRec.inDFED_ChipConditionCodeLen, szASCII);
		}
		/* ISO Print Debug */
		if (ginISODebug == VS_TRUE)
		{
			memset(szASCII, 0x00, sizeof(szASCII));
			inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.uszDFEE_TerEntryCap[0], pobTran->srEMVRec.inDFED_ChipConditionCodeLen);

			inCTLS_ISOFormatDebug_PRINT("DFEE", pobTran->srEMVRec.inDFED_ChipConditionCodeLen, szASCII);
		}

		/* DFEF */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		if (inFunc_TWNAddDataToEMVPacket(pobTran, TAG_DFEF_REASON_ONLINE_CODE, (unsigned char *)&szTemplate[0]) > 0)
		{
			memset(pobTran->srEMVRec.uszDFEF_ReasonOnlineCode, 0x00, sizeof(pobTran->srEMVRec.uszDFEF_ReasonOnlineCode));
			pobTran->srEMVRec.inDFEF_ReasonOnlineCodeLen = szTemplate[2];
			memcpy((char *)&pobTran->srEMVRec.uszDFEF_ReasonOnlineCode[0], &szTemplate[3], pobTran->srEMVRec.inDFEF_ReasonOnlineCodeLen);
		}
		/* ISO Display Debug */
		if (ginDebug == VS_TRUE)
		{
			memset(szASCII, 0x00, sizeof(szASCII));
			inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.uszDFEF_ReasonOnlineCode[0], pobTran->srEMVRec.inDFEF_ReasonOnlineCodeLen);

			inCTLS_ISOFormatDebug_DISP("DFEF", pobTran->srEMVRec.inDFEF_ReasonOnlineCodeLen, szASCII);
		}
		/* ISO Print Debug */
		if (ginISODebug == VS_TRUE)
		{
			memset(szASCII, 0x00, sizeof(szASCII));
			inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.uszDFEF_ReasonOnlineCode[0], pobTran->srEMVRec.inDFEF_ReasonOnlineCodeLen);

			inCTLS_ISOFormatDebug_PRINT("DFEF", pobTran->srEMVRec.inDFEF_ReasonOnlineCodeLen, szASCII);
		}

		if (pobTran->srEMVRec.inDFED_ChipConditionCodeLen > 0)
		{
			szPacket[inPacketCnt ++] = 0xDF;
			szPacket[inPacketCnt ++] = 0xED;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.inDFED_ChipConditionCodeLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.uszDFED_ChipConditionCode[0], pobTran->srEMVRec.inDFED_ChipConditionCodeLen);
			inPacketCnt += pobTran->srEMVRec.inDFED_ChipConditionCodeLen;
		}

		if (pobTran->srEMVRec.inDFEE_TerEntryCapLen > 0)
		{
			szPacket[inPacketCnt ++] = 0xDF;
			szPacket[inPacketCnt ++] = 0xEE;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.inDFEE_TerEntryCapLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.uszDFEE_TerEntryCap[0], pobTran->srEMVRec.inDFEE_TerEntryCapLen);
			inPacketCnt += pobTran->srEMVRec.inDFEE_TerEntryCapLen;
		}

		if (pobTran->srEMVRec.inDFEF_ReasonOnlineCodeLen > 0)
		{
			szPacket[inPacketCnt ++] = 0xDF;
			szPacket[inPacketCnt ++] = 0xEF;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.inDFEF_ReasonOnlineCodeLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.uszDFEF_ReasonOnlineCode[0], pobTran->srEMVRec.inDFEF_ReasonOnlineCodeLen);
			inPacketCnt += pobTran->srEMVRec.inDFEF_ReasonOnlineCodeLen;
		}

	}
	else
	{
		/* DFEE */
		if (pobTran->srEMVRec.inDFEE_TerEntryCapLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 56 TAG DFEE ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0xDF;
		szPacket[inPacketCnt ++] = 0xEE;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.inDFEE_TerEntryCapLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.uszDFEE_TerEntryCap[0], pobTran->srEMVRec.inDFEE_TerEntryCapLen);
		inPacketCnt += pobTran->srEMVRec.inDFEE_TerEntryCapLen;
		

		/* DFEF */
		if (pobTran->srEMVRec.inDFEF_ReasonOnlineCodeLen <= 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "F 56 TAG DFEF ERROR!!");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}
		szPacket[inPacketCnt ++] = 0xDF;
		szPacket[inPacketCnt ++] = 0xEF;
		szPacket[inPacketCnt ++] = pobTran->srEMVRec.inDFEF_ReasonOnlineCodeLen;
		memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.uszDFEF_ReasonOnlineCode[0], pobTran->srEMVRec.inDFEF_ReasonOnlineCodeLen);
		inPacketCnt += pobTran->srEMVRec.inDFEF_ReasonOnlineCodeLen;
		
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
                inLogPrintf(AT, "inNCCC_MFES_Pack56() END!");
	
	return (inCnt);
}

/*
Function        :inNCCC_MFES_Pack57_tSAM
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
int inNCCC_MFES_Pack57_tSAM(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int		inCnt = 0, inPacketCnt = 0;
	char		szASCII[4 + 1];
	char		szPacket[100 + 1];
	char		szPacket_Ascii[100 + 1];
	char		szTemplate[100 + 1];
	unsigned char	uszBCD[2 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_Pack57_tSAM() START!");
	
	
	/* BitMap 8 Byte */
	memset(szPacket, 0x00, sizeof(szPacket));
	szPacket[inPacketCnt ++] = 0x00;
	szPacket[inPacketCnt ++] = 0x00;
	szPacket[inPacketCnt ++] = 0x00;
	szPacket[inPacketCnt ++] = 0x00;

	/* 第5個byte的第三個Bit */
	if (guszField_35 == VS_TRUE)
		szPacket[inPacketCnt ++] = 0x20;
	else
		szPacket[inPacketCnt ++] = 0x00;

	szPacket[inPacketCnt ++] = 0x00;

	/* 第7個byte的第七個Bit */
	if (guszField_55 == VS_TRUE)
	{
		szPacket[inPacketCnt ++] = 0x02;
	}
	else
	{
		szPacket[inPacketCnt ++] = 0x00;
	}

	szPacket[inPacketCnt ++] = 0x00;
	
	if (guszField_35 == VS_TRUE)
	{
		memcpy((char *)&szPacket[inPacketCnt], (char *)&gusztSAMCheckSum_35[0], 4);
		inPacketCnt += 4;
	}

	if (guszField_55 == VS_TRUE)
	{
		memcpy((char *)&szPacket[inPacketCnt], (char *)&gusztSAMCheckSum_55[0], 4);
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
		
		inFunc_BCD_to_ASCII(szPacket_Ascii, (unsigned char*)szPacket, 14);
		inLogPrintf(AT, "F_57 [CheckSum]");
		sprintf(szTemplate, "%s %s", szASCII, szPacket_Ascii);
		inLogPrintf(AT, szTemplate);
	}
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_Pack57_tSAM() END!");
	
        return (inCnt);
}

/*
Function        :inNCCC_MFES_Pack59
Date&Time       :2015/12/3 上午 11:00
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

/*
Function        :inNCCC_MFES_Pack59
Date&Time       :2016/12/23 下午 4:35
Describe        :This field is used to AE card 4DBC for AE transactions, or table id
		 for something transactions
*/
int inNCCC_MFES_Pack59(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int		inNCLen = 0;
	int		inEIVILen = 0;
	int		inCnt = 0, inPacketCnt = 0;
	char		szASCII[4 + 1];
	char		szTemplate[100 + 1], szTemplate1[100 + 1], szPacket[768 + 1]; /* sztemplae for debug message,szTemplate1 for temprary*/
	char		sztemp[8 + 1];
	char		szESCMode[2 + 1];
	unsigned char	uszBCD[2 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_Pack59() START!");

        memset(szTemplate1, 0x00, sizeof(szTemplate1));
	memset(szPacket, 0x00, sizeof(szPacket));

	/* 以下共區分3類使用規格，
	 * 第(1)類規格無Table ID，
	 * 第(2)類規格Table ID為1碼，
	 * 第(3)類規格Table ID為2碼。
	 * 第(1)類規格不可與(2)、(3)類規格混合使用，第(2)、(3)類規格可以混合使用
	 *  */
	
	if ((pobTran->srBRec.inCode == _LOYALTY_REDEEM_		||
	     pobTran->srBRec.inCode == _VOID_LOYALTY_REDEEM_)	&&
	    (memcmp(guszMFES_MTI, "0220", strlen("0220")) != 0)	&&	/* ISOTxncode == _ADVICE_、_TC_UPLOAD_、_TIP_*/
	    (memcmp(guszMFES_MTI, "0320", strlen("0320")) != 0))	/* ISOTxncode == _BatchUpload_ */
	{
		memcpy(&szPacket[inPacketCnt], "L3", 2); /* Table ID */
		inPacketCnt += 2;
		/* Sub Total Length */
		inPacketCnt ++;
		szPacket[inPacketCnt] = 0x92;
		inPacketCnt ++;

		/* Data */
		/* 兌換方式 */
		memcpy(&szPacket[inPacketCnt], pobTran->szL3_AwardWay, 1);
		inPacketCnt ++;

		/* S/N = TID(8 bytes) + YYYYMMDDhhmmss (共22 bytes) */
		memcpy(&szPacket[inPacketCnt], pobTran->srBRec.szAwardSN, 22);
		inPacketCnt += 22;

		/* 是否列印補充資訊 */
		szPacket[inPacketCnt] = '0';
		inPacketCnt ++;

		/* 補充資訊實際長度 */
		memcpy(&szPacket[inPacketCnt], "00", 2);
		inPacketCnt += 2;

		/* 補充內容 雖然不送資料  還是要空下來  */
		memset(szTemplate1, ' ', sizeof(szTemplate1));
		memcpy(&szPacket[inPacketCnt], &szTemplate1[0], 20);
		inPacketCnt += 20;

		/* 以條碼方式兌換 */
		if (!memcmp(pobTran->szL3_AwardWay, "1", 1)	||
		    !memcmp(pobTran->szL3_AwardWay, "2", 1)	||
		    !memcmp(pobTran->szL3_AwardWay, "3", 1))
		{
			/* 第一段條碼 */
			/* 是否上傳一維條碼 */
			szPacket[inPacketCnt] = '1';
			inPacketCnt ++;

			/* 一維條碼長度 */
			memcpy(&szPacket[inPacketCnt], pobTran->szL3_Barcode1Len, 2);
			inPacketCnt += 2;

			/* 一維條碼 */
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			memcpy(szTemplate1, pobTran->szL3_Barcode1, atoi(pobTran->szL3_Barcode1Len));
			/* 左靠右補空白 */
			inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 20, _PADDING_RIGHT_);
			memcpy(&szPacket[inPacketCnt], szTemplate1, 20);
			inPacketCnt += 20;

			/* 第二段條碼 */
			if (atoi(pobTran->szL3_Barcode2Len) > 0)
			{
				/* 是否上傳一維條碼(二) */
				szPacket[inPacketCnt] = '1';
				inPacketCnt ++;

				/* 一維條碼(二)長度 */
				memcpy(&szPacket[inPacketCnt], pobTran->szL3_Barcode2Len, 2);
				inPacketCnt += 2;

				/* 一維條碼(二) */
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				memcpy(szTemplate1, pobTran->szL3_Barcode2, atoi(pobTran->szL3_Barcode2Len));
				/* 左靠右補空白 */
				inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 20, _PADDING_RIGHT_);
				memcpy(&szPacket[inPacketCnt], szTemplate1, 20);
				inPacketCnt += 20;
			}
			else
			{
				/* Q2. 上傳L3的時候, 若只上傳一組條碼, 那EDC在第二組條碼的欄位是flag = 0 + 其餘欄位補空白嗎?
				       【是的】 */

				/* 是否上傳一維條碼 */
				szPacket[inPacketCnt] = '0';
				inPacketCnt ++;

				/* 一維條碼長度 */
				memcpy(&szPacket[inPacketCnt], "00", 2);
				inPacketCnt += 2;

				/* 一維條碼 */
				memset(szTemplate1, ' ', 20);
				memcpy(&szPacket[inPacketCnt], szTemplate1, 20);
				inPacketCnt += 20;
			}

		}
		/* 以卡號方式兌換*/
		else if (!memcmp(pobTran->szL3_AwardWay, "4", 1)	||
			 !memcmp(pobTran->szL3_AwardWay, "5", 1))
		{
			/* 是否上傳一維條碼(一) */
			szPacket[inPacketCnt] = '0';
			inPacketCnt ++;

			/* 一維條碼長度(一) */
			memcpy(&szPacket[inPacketCnt], "00", 2);
			inPacketCnt += 2;

			/* 一維條碼(一) */
			memset(szTemplate1, ' ', 20);
			memcpy(&szPacket[inPacketCnt], szTemplate1, 20);
			inPacketCnt += 20;

			/* 是否上傳一維條碼(二) */
			memcpy(&szPacket[inPacketCnt], "0", 1);
			inPacketCnt ++;

			/* 一維條碼長度(二) */
			memcpy(&szPacket[inPacketCnt], "00", 2);
			inPacketCnt += 2;

			/* 一維條碼(二) */
			memset(szTemplate1, 0x20, 20);
			memcpy(&szPacket[inPacketCnt], szTemplate1, 20);
			inPacketCnt += 20;
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
			inLogPrintf(AT, "inNCCC_MFES_Pack59() END!");

		return (inCnt);
	}
	
	if (pobTran->inISOTxnCode == _CUP_LOGON_)
	{
		/* 只會上傳【Y Table】 */
	}
	/* AE規則很麻煩，所以獨立出來 */
	else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)) == 0)
	{
		/* AE卡但不輸入4DBC */
		if (strlen(pobTran->srBRec.szAMEX4DBC) == 0)
		{
			/* CUP Data Information */
			szPacket[inPacketCnt] = 'C'; /* Table ID */
			inPacketCnt ++;
			szPacket[inPacketCnt] = 2; /* Sub-Data Total Length */
			inPacketCnt ++;

			memset(sztemp, 0x00, sizeof(sztemp));
			inGetCUPFuncEnable(sztemp);
			if (sztemp[0] == 'Y')
			{
				szPacket[inPacketCnt] = '1';
			}
			else
			{
				szPacket[inPacketCnt] = '0';
			}
			inPacketCnt ++;
			szPacket[inPacketCnt] = '0';
			inPacketCnt ++;
			
			if  (pobTran->srBRec.inCode == _PRE_COMP_	|| 
			     pobTran->inISOTxnCode == _PRE_COMP_	|| 
			     pobTran->srBRec.inOrgCode == _PRE_COMP_)
			{
				/* Pre Authorization Complete Transaction */
				szPacket[inPacketCnt] = 'O'; /* Table ID */
				inPacketCnt ++;
				szPacket[inPacketCnt] = 16; /* Sub-Data Total Length */
				inPacketCnt ++;
				memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szCUP_TD[0], 4);		/* Original Transaction Amount (MMDD) */
				inPacketCnt += 4;
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%010ld00", pobTran->srBRec.lnOrgTxnAmount);
				memcpy(&szPacket[inPacketCnt], &szTemplate[0], 12);				/* Original Transaction Amount */
				inPacketCnt += 12;
			}
			else
			{
				if (memcmp(guszMFES_MTI, "0220", 4) != 0	&&
				    memcmp(guszMFES_MTI, "0320", 4) != 0)
				{
					
					szPacket[inPacketCnt] = 'Y'; /* Table ID */
					inPacketCnt ++;
					szPacket[inPacketCnt] = 4; /* Sub-Data Total Length */
					inPacketCnt ++;
					memcpy(&szPacket[inPacketCnt], "0000", 4);
					inPacketCnt += 4;
				}
				
			}
			
		}
		/* AE卡且有輸入4DBC */
		else
		{
		        if (pobTran->srBRec.inOrgCode == _CUP_SALE_  ||
			    pobTran->srBRec.inOrgCode == _CUP_PRE_AUTH_)
		        {
		                /* CUP Data Information */
				szPacket[inPacketCnt] = 'C'; /* Table ID */
				inPacketCnt ++;
				szPacket[inPacketCnt] = 2; /* Sub-Data Total Length */
				inPacketCnt ++;

				memset(sztemp, 0x00, sizeof(sztemp));
				inGetCUPFuncEnable(sztemp);
				if (sztemp[0] == 'Y')
				{
					szPacket[inPacketCnt] = '1';
				}
				else
				{
					szPacket[inPacketCnt] = '0';
				}
				inPacketCnt ++;
				szPacket[inPacketCnt] = '0';
				inPacketCnt ++;

				szPacket[inPacketCnt] = 'Y'; /* Table ID */
				inPacketCnt ++;
				szPacket[inPacketCnt] = 4; /* Sub-Data Total Length */
				inPacketCnt ++;
				memcpy(&szPacket[inPacketCnt], "0000", 4);
				inPacketCnt += 4;
			}
			else
			{
			        /* AE批次上傳及reversal及小費不送4dbc */
				/* reversal 移到下面else if 判斷 */
				/* 取消也不能送4DBC */
				if (pobTran->inISOTxnCode == _BATCH_UPLOAD_	|| 
				    pobTran->inISOTxnCode == _TIP_		|| 
				    pobTran->inISOTxnCode == _VOID_)
				{
					/* CUP Data Information */
					szPacket[inPacketCnt] = 'C'; /* Table ID */
					inPacketCnt ++;
					szPacket[inPacketCnt] = 2; /* Sub-Data Total Length */
					inPacketCnt ++;

					memset(sztemp, 0x00, sizeof(sztemp));
					inGetCUPFuncEnable(sztemp);
					if (sztemp[0] == 'Y')
					{
						szPacket[inPacketCnt] = '1';
					}
					else
					{
						szPacket[inPacketCnt] = '0';
					}
					inPacketCnt ++;
					szPacket[inPacketCnt] = '0';
					inPacketCnt ++;
				}
				else if ( pobTran->inISOTxnCode == _REVERSAL_ )
				{
					/* reversal照原交易送tabke Y */
					/* CUP Data Information */
					szPacket[inPacketCnt] = 'C'; /* Table ID */
					inPacketCnt ++;
					szPacket[inPacketCnt] = 2; /* Sub-Data Total Length */
					inPacketCnt ++;

					memset(sztemp, 0x00, sizeof(sztemp));
					inGetCUPFuncEnable(sztemp);
					if (sztemp[0] == 'Y')
					{
						szPacket[inPacketCnt] = '1';
					}
					else
					{
						szPacket[inPacketCnt] = '0';
					}
					inPacketCnt ++;
					szPacket[inPacketCnt] = '0';
					inPacketCnt ++;

					szPacket[inPacketCnt] = 'Y'; /* Table ID */
					inPacketCnt ++;
					szPacket[inPacketCnt] = 4; /* Sub-Data Total Length */
					inPacketCnt ++;
					memcpy(&szPacket[inPacketCnt], "0000", 4);
					inPacketCnt += 4;
				}
				else
				{
					memcpy(&szPacket[inPacketCnt], "N8", 2);  /* 2013-12-09 AM 10:41:27 add by kakab R-FES EDC Specifications v1.48.doc 4DBC新增table ID N8 */
					inPacketCnt += 2;
					szPacket[inPacketCnt] = 0x00; /* Sub-Data Total Length */
					szPacket[inPacketCnt + 1] = 4; /* Sub-Data Total Length */
					inPacketCnt += 2;
                			memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szAMEX4DBC[0], 4);
                			inPacketCnt += 4;
                		}
        		}/* 非CUP交易END */
			
		}/* 有輸入4DBC END */
	}
	else
	{
		/* CUP交易 */
		if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szPacket, 0x00, sizeof(szPacket));
			strcpy(szTemplate1, "000000000000");
			/* CUP Data Information */
			szPacket[inPacketCnt] = 'C'; /* Table ID */
			inPacketCnt ++;
			szPacket[inPacketCnt] = 34; /* Sub-Data Total Length */
			inPacketCnt ++;
			szPacket[inPacketCnt] = '1';
			inPacketCnt ++;
			szPacket[inPacketCnt] = '1';
			inPacketCnt ++;

			/* CUP Trace Number（TN）
			 * 上傳時，下述交易時帶授權主機回傳值，其餘交易帶'000000'：
			 * Void Sale
			 * Void Pre-Auth
			 * Reversal
			 * Batch Upload
			 * 主機回傳時，下述交易由授權主機更新回傳：
			 * Sale
			 * Void Sale
			 * Pre-Auth
			 * Void Pre-Auth
			 */
			if (pobTran->inISOTxnCode == _BATCH_UPLOAD_											||	/* 4. Batch Upload */
			    pobTran->inISOTxnCode == _TC_UPLOAD_											||	/* 5. TC Upload*/
			    (pobTran->inISOTxnCode == _CUP_VOID_  && pobTran->srBRec.inOrgCode == _CUP_SALE_)						||	/* 1. Void (Refund N1 回傳同送出，所以只包含CUP_SALE)*/
			    pobTran->inISOTxnCode == _CUP_PRE_AUTH_VOID_										||	/* 2. Void Pre-Auth */
			    (pobTran->inISOTxnCode == _REVERSAL_ && (pobTran->srBRec.inCode == _CUP_VOID_ && pobTran->srBRec.inOrgCode == _CUP_SALE_))	||	/* 3. Reversal(Void Sal) */
			    (pobTran->inISOTxnCode == _REVERSAL_ && pobTran->srBRec.inCode == _CUP_PRE_AUTH_VOID_))						/* 3. Reversal(Void Pre-Auth) */
			{
				memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szCUP_TN[0], 6);
				if (ginDebug == VS_TRUE)
				{
					memset(szTemplate, 0x00, sizeof(szTemplate));
					sprintf(szTemplate, "   [%s]", pobTran->srBRec.szCUP_TN);
					inLogPrintf(AT, szTemplate);
				}

			}
			/* advice有的交易有值，有得要全0，所以做此判斷做區別 */
			else if (pobTran->inISOTxnCode == _ADVICE_		||
				 pobTran->inISOTxnCode == _SEND_ESC_ADVICE_)
			{
				if (strlen(pobTran->srBRec.szCUP_TN) > 0)
					memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szCUP_TN[0], 6);

				if (ginDebug == VS_TRUE)
				{
					memset(szTemplate, 0x00, sizeof(szTemplate));
					sprintf(szTemplate, "   [%s]", pobTran->srBRec.szCUP_TN);
					inLogPrintf(AT, szTemplate);
				}
				/* 沒有就全塞0，而不能有空白 */
				else
				{
					memcpy(&szPacket[inPacketCnt], &szTemplate1[0], 6);
					if (ginDebug == VS_TRUE)
					{
						memset(szTemplate, 0x00, sizeof(szTemplate));
						sprintf(szTemplate, "   [%s]", "000000");
						inLogPrintf(AT, szTemplate);
					}
				}
			}
			else
			{
				memcpy(&szPacket[inPacketCnt], &szTemplate1[0], 6);

				if (ginDebug == VS_TRUE)
				{
					memset(szTemplate, 0x00, sizeof(szTemplate));
					sprintf(szTemplate, "   [%s]", "000000");
					inLogPrintf(AT, szTemplate);
				}
			}

			inPacketCnt += 6;

			/* CUP Transaction Date (MMDD) */
			/*
				UnionPay (原CUP) Transaction Date(MMDD)
				上傳時，下述交易時帶授權主機回傳值，其餘交易帶'0000'：
				1. Void
				2. Void Pre-Auth
				3. Pre-Auth Complete
				4. Void Pre-Auth Complete
				5. Refund
				6. Batch Upload
				7. TC Upload
			*/
			if (pobTran->inISOTxnCode == _BATCH_UPLOAD_	||	/* 6. Batch Upload */
			    pobTran->inISOTxnCode == _TC_UPLOAD_	||	/* 7. TC Upload */
			    pobTran->inISOTxnCode == _CUP_REFUND_	||	/* 5. Refund */
			    pobTran->inISOTxnCode == _CUP_PRE_COMP_	||	/* 3. Pre-Auth Complete */
			    pobTran->inISOTxnCode == _CUP_VOID_		||	/* 1. Void */
			    pobTran->inISOTxnCode == _CUP_PRE_AUTH_VOID_||	/* 2. Void Pre-Auth */
			    pobTran->inISOTxnCode == _CUP_PRE_COMP_VOID_	/* 4. Void Pre-Auth Complete */
			    )
			{
				memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szCUP_TD[0], 4);

				if (ginDebug == VS_TRUE)
				{
					memset(szTemplate, 0x00, sizeof(szTemplate));
					sprintf(szTemplate, "   [%s]", pobTran->srBRec.szCUP_TD);
					inLogPrintf(AT, szTemplate);
				}
			}
			else if (pobTran->inISOTxnCode == _ADVICE_)
			{
				if (strlen(pobTran->srBRec.szCUP_TN) > 0)
				{
					memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szCUP_TD[0], 4);

					if (ginDebug == VS_TRUE)
					{
						memset(szTemplate, 0x00, sizeof(szTemplate));
						sprintf(szTemplate, "   [%s]", pobTran->srBRec.szCUP_TD);
						inLogPrintf(AT, szTemplate);
					}
				}
				else
				{
					memcpy(&szPacket[inPacketCnt], &szTemplate[0], 4);

					if (ginDebug == VS_TRUE)
					{
						memset(szTemplate, 0x00, sizeof(szTemplate));
						sprintf(szTemplate, "   [%s]", "0000");
						inLogPrintf(AT, szTemplate);
					}
				}
			}
			else
			{
				memcpy(&szPacket[inPacketCnt], &szTemplate1[0], 4);

				if (ginDebug == VS_TRUE)
				{
					memset(szTemplate, 0x00, sizeof(szTemplate));
					sprintf(szTemplate, "   [%s]", "0000");
					inLogPrintf(AT, szTemplate);
				}
			}

			inPacketCnt += 4;

			/*
				UnionPay (原CUP) Transaction Time(hhmmss)
				上傳時，下述交易時帶授權主機回傳值，其餘交易帶'000000'：
				1. Void
				2. Void Pre-Auth
				3. Batch Upload
				4. TC Upload
			*/
			if (pobTran->inISOTxnCode == _BATCH_UPLOAD_		||	/* 3. Batch Upload */
			    pobTran->inISOTxnCode == _TC_UPLOAD_		||	/* 4. TC Upload */
			    pobTran->inISOTxnCode == _CUP_VOID_			||	/* 1. Void */
			    pobTran->inISOTxnCode == _CUP_PRE_AUTH_VOID_)		/* 2. Void Pre-Auth */
			{
				memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szCUP_TT[0], 6);

				if (ginDebug == VS_TRUE)
				{
					memset(szTemplate, 0x00, sizeof(szTemplate));
					sprintf(szTemplate, "   [%s]", pobTran->srBRec.szCUP_TT);
					inLogPrintf(AT, szTemplate);
				}
			}
			else if (pobTran->inISOTxnCode == _ADVICE_)
			{
				if (strlen(pobTran->srBRec.szCUP_TN) > 0)
				{
					memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szCUP_TT[0], 6);

					if (ginDebug == VS_TRUE)
					{
						memset(szTemplate, 0x00, sizeof(szTemplate));
						sprintf(szTemplate, "   [%s]", pobTran->srBRec.szCUP_TT);
						inLogPrintf(AT, szTemplate);
					}
				}
				else
				{
					memcpy(&szPacket[inPacketCnt], &szTemplate1[0], 6);

					if (ginDebug == VS_TRUE)
					{
						memset(szTemplate, 0x00, sizeof(szTemplate));
						sprintf(szTemplate, "   [%s]", "000000");
						inLogPrintf(AT, szTemplate);
					}
				}
			}
			else
			{
				memcpy(&szPacket[inPacketCnt], &szTemplate1[0], 6);

				if (ginDebug == VS_TRUE)
				{
					memset(szTemplate, 0x00, sizeof(szTemplate));
					sprintf(szTemplate, "   [%s]", "000000");
					inLogPrintf(AT, szTemplate);
				}
			}

			inPacketCnt += 6;

			/*
				UnionPay (原CUP) Retrieve Reference Number(CRRN)
				上傳時，下述交易時帶授權主機回傳值，其餘交易帶'000000000000'：
				1. Void
				2. Void Pre-Auth
			 *	3. Void Pre-Auth COMP
				4. Batch Upload
				5. TC Upload
			*/
			if (pobTran->inISOTxnCode == _BATCH_UPLOAD_	||	/* 3. Batch Upload */
			    pobTran->inISOTxnCode == _TC_UPLOAD_	||	/* 4. TC Upload */
			    pobTran->inISOTxnCode == _CUP_VOID_		||	/* 1. Void */
			    pobTran->inISOTxnCode == _CUP_PRE_AUTH_VOID_||	/* 2. Void Pre-Auth */
			    pobTran->inISOTxnCode == _CUP_PRE_COMP_VOID_)
			{
				memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szCUP_RRN[0], 12);

				if (ginDebug == VS_TRUE)
				{
					memset(szTemplate, 0x00, sizeof(szTemplate));
					sprintf(szTemplate, "   [%s]", pobTran->srBRec.szCUP_RRN);
					inLogPrintf(AT, szTemplate);
				}
			}
			else if (pobTran->inISOTxnCode == _ADVICE_)
			{
				if (strlen(pobTran->srBRec.szCUP_TN) > 0)
				{
					memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szCUP_RRN[0], 12);

					if (ginDebug == VS_TRUE)
					{
						memset(szTemplate, 0x00, sizeof(szTemplate));
						sprintf(szTemplate, "   [%s]", pobTran->srBRec.szCUP_RRN);
						inLogPrintf(AT, szTemplate);
					}
				}
				else
				{
					memcpy(&szPacket[inPacketCnt], &szTemplate1[0], 12);

					if (ginDebug == VS_TRUE)
					{
						memset(szTemplate, 0x00, sizeof(szTemplate));
						sprintf(szTemplate, "   [%s]", "000000000000");
						inLogPrintf(AT, szTemplate);
					}
				}
			}
			else
			{
				memcpy(&szPacket[inPacketCnt], &szTemplate1[0], 12);
				if (ginDebug == VS_TRUE)
				{
					memset(szTemplate, 0x00, sizeof(szTemplate));
					sprintf(szTemplate, "   [%s]", "000000000000");
					inLogPrintf(AT, szTemplate);
				}
			}

			inPacketCnt += 12;

			/*
				Settlement date(MMDD)
				上傳時，下述交易時帶授權主機回傳值，其餘交易帶'0000'
				1. Void
				2. Void Pre-Auth
				3. Void Pre-Auth Complete
				4. Batch Upload
				5. TC Upload
			*/
			if (pobTran->inISOTxnCode == _BATCH_UPLOAD_		||	/* 4. Batch Upload */
			    pobTran->inISOTxnCode == _TC_UPLOAD_		||	/* 5. TC Upload */
			    pobTran->inISOTxnCode == _CUP_VOID_			||	/* 1. Void */
			    pobTran->inISOTxnCode == _CUP_PRE_AUTH_VOID_	||	/* 2. Void Pre-Auth */
			    pobTran->inISOTxnCode == _CUP_PRE_COMP_VOID_)		/* 3. Void Pre-Auth Complete */
			{
				memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szCUP_STD[0], 4);

				if (ginDebug == VS_TRUE)
				{
					memset(szTemplate, 0x00, sizeof(szTemplate));
					sprintf(szTemplate, "   [%s]", pobTran->srBRec.szCUP_STD);
					inLogPrintf(AT, szTemplate);
				}
			}
			else if (pobTran->inISOTxnCode == _ADVICE_)
			{
				if (strlen(pobTran->srBRec.szCUP_TN) > 0)
				{
					memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szCUP_STD[0], 4);

					if (ginDebug == VS_TRUE)
					{
						memset(szTemplate, 0x00, sizeof(szTemplate));
						sprintf(szTemplate, "   [%s]", pobTran->srBRec.szCUP_STD);
						inLogPrintf(AT, szTemplate);
					}
				}
				else
				{
					memcpy(&szPacket[inPacketCnt], &szTemplate1[0], 4);

					if (ginDebug == VS_TRUE)
					{
						memset(szTemplate, 0x00, sizeof(szTemplate));
						sprintf(szTemplate, "   [%s]", "0000");
						inLogPrintf(AT, szTemplate);
					}
				}
			}
			else
			{
				memcpy(&szPacket[inPacketCnt], &szTemplate1[0], 4);

				if (ginDebug == VS_TRUE)
				{
					memset(szTemplate, 0x00, sizeof(szTemplate));
					sprintf(szTemplate, "   [%s]", "0000");
					inLogPrintf(AT, szTemplate);
				}
			}

			inPacketCnt += 4;
			
			if (pobTran->inISOTxnCode == _CUP_PRE_COMP_											||
			   (pobTran->inISOTxnCode == _REVERSAL_ && pobTran->srBRec.inCode == _CUP_PRE_COMP_)						|| 
			   (pobTran->srBRec.uszVOIDBit == VS_TRUE && pobTran->srBRec.inOrgCode == _CUP_PRE_COMP_)					||
			   (pobTran->inISOTxnCode == _REVERSAL_ && pobTran->srBRec.uszVOIDBit == VS_TRUE && pobTran->srBRec.inOrgCode == _PRE_COMP_)	||
			   pobTran->inISOTxnCode == _BATCH_UPLOAD_)
			{
				/* Pre Authorization Complete Transaction */
				szPacket[inPacketCnt] = 'O'; /* Table ID */
				inPacketCnt ++;
				szPacket[inPacketCnt] = 16; /* Sub-Data Total Length */
				inPacketCnt ++;
				memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szCUP_TD[0], 4);		/* Original Transaction Amount (MMDD) */
				inPacketCnt += 4;
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%010ld00", pobTran->srBRec.lnOrgTxnAmount);
				memcpy(&szPacket[inPacketCnt], &szTemplate[0], 12);				/* Original Transaction Amount */
				inPacketCnt += 12;
			}

		}
		/* 非CUP 交易 */
		else
		{
			if (pobTran->srBRec.inCode == _LOYALTY_REDEEM_		||
			    pobTran->srBRec.inCode == _VOID_LOYALTY_REDEEM_)		/* 優惠兌換不送F64 */
			{
				/* 優惠兌換不送 */
			}
			else
			{
				/* CUP Data Information */
				szPacket[inPacketCnt] = 'C'; /* Table ID */
				inPacketCnt ++;
				szPacket[inPacketCnt] = 2; /* Sub-Data Total Length */
				inPacketCnt ++;

				memset(sztemp, 0x00, sizeof(sztemp));
				inGetCUPFuncEnable(sztemp);
				if (sztemp[0] == 'Y')
				{
					szPacket[inPacketCnt] = '1';
				}
				else
				{
					szPacket[inPacketCnt] = '0';
				}
				inPacketCnt ++;
				szPacket[inPacketCnt] = '0';
				inPacketCnt ++;

				if (pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE)
				{
					szPacket[inPacketCnt] = 'D'; /* Table ID */
					inPacketCnt ++;
					szPacket[inPacketCnt] = 0x01; /* Sub-Data Total Length */
					inPacketCnt ++;
					/* 轉台幣sale送D 轉台幣PreAuth送P */
					if (pobTran->srBRec.inOrgCode == _PRE_COMP_)
						memcpy(&szPacket[inPacketCnt], "P", 1);
					else
						memcpy(&szPacket[inPacketCnt], "D", 1);
					inPacketCnt ++;
				}

				/*  Table ID 'O': Pre-Auth Complete交易 */
				/*  下傳此述交欄位易必：須上傳，主機不回
				 *  Pre-Auth Complete
				 *  Reversal Pre-Auth Complete
				 *  Void Pre-Auth Complete
				 *  Reversal Void Pre-Auth Complete
				 *  Batch Upload
				 */
				if  (pobTran->inISOTxnCode == _PRE_COMP_										 ||
				     (pobTran->inISOTxnCode == _REVERSAL_ && pobTran->srBRec.inCode == _PRE_COMP_)					 || 
				     (pobTran->inISOTxnCode == _VOID_ && pobTran->srBRec.inOrgCode == _PRE_COMP_)					 ||
				     (pobTran->inISOTxnCode == _REVERSAL_ && pobTran->srBRec.inCode == _VOID_ && pobTran->srBRec.inOrgCode == _PRE_COMP_)||
				     pobTran->inISOTxnCode == _BATCH_UPLOAD_)
				{
					/* Pre Authorization Complete Transaction */
					szPacket[inPacketCnt] = 'O'; /* Table ID */
					inPacketCnt ++;
					szPacket[inPacketCnt] = 16; /* Sub-Data Total Length */
					inPacketCnt ++;
					memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szCUP_TD[0], 4); /* Original Transaction Amount (MMDD) */
					inPacketCnt += 4;
					memset(szTemplate, 0x00, sizeof(szTemplate));
					sprintf(szTemplate, "%010ld00", pobTran->srBRec.lnOrgTxnAmount);
					memcpy(&szPacket[inPacketCnt], &szTemplate[0], 12); /* Original Transaction Amount */
					inPacketCnt += 12;
				}
				
			}

		}/* 是否為CUP交易END */
		
	}

	/* Table 'Y '取得主機的交易西元年 */
	if ((!memcmp(guszMFES_MTI, "0100", 4)	||
	     !memcmp(guszMFES_MTI, "0200", 4)	||
	     !memcmp(guszMFES_MTI, "0400", 4)	||
	     !memcmp(guszMFES_MTI, "0800", 4))	&&
	      memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)) != 0)		/* AE獨立判斷，所以這裡不檢核 */
	{
		/* 預先授權完成取消不送Table Y */
		if ((pobTran->inISOTxnCode == _VOID_  && pobTran->srBRec.inOrgCode == _PRE_COMP_)	||
		     pobTran->srBRec.inCode == _LOYALTY_REDEEM_						||
		     pobTran->srBRec.inCode == _VOID_LOYALTY_REDEEM_)						/* 優惠兌換不送Y */
		{

		}
		else
		{
			szPacket[inPacketCnt] = 'Y'; /* Table ID */
			inPacketCnt ++;
			szPacket[inPacketCnt] = 0x04; /* Sub-Data Total Length */
			inPacketCnt ++;
			memcpy(&szPacket[inPacketCnt], "0000", 4);
			inPacketCnt += 4;
		}
	}

	/*
		●	Table ID “NC”: MCP Indicator & Issue Bank ID (行動支付標記及金融機構代碼)
		發卡行金融機構代碼。(請求電文填空白)
		(1) 國內卡->金融機構代碼
		    (NSSMBINR. NSSMBINR_FID的前三碼即為金融機構代碼)
		(2) 國外卡->固定值"999"
		    (NSSMBINR查無BIN即為外國卡)
		(3) ATS不管是否為MCP(行動支付)皆要回傳發卡行金融機構代碼。
	*/
	/* 0100、0200及 0400的交易電文 (上述不含 Smart Pay交易)， 端末機預設上傳 Table ID“NC ”*/
	/* 0220但是online的交易，0320但原交易不是offline的交易 */
	/* 行動支付標記及金融機構代碼 */
	if ((!memcmp(&guszMFES_MTI[0], "0100", 4)		||
	     !memcmp(&guszMFES_MTI[0], "0200", 4)		||
	    (!memcmp(&guszMFES_MTI[0], "0220", 4)		&&
	     (pobTran->srBRec.inOrgCode == _SALE_OFFLINE_	|| 
	      pobTran->srBRec.inOrgCode == _INST_ADJUST_	|| 
	      pobTran->srBRec.inOrgCode == _REDEEM_ADJUST_	||
	      pobTran->srBRec.inOrgCode == _PRE_COMP_		||
	      pobTran->srBRec.inOrgCode == _CUP_PRE_COMP_	||
	      pobTran->srBRec.inOrgCode == _CUP_PRE_COMP_VOID_))||
	    (!memcmp(&guszMFES_MTI[0], "0320", 4)		&&
	      pobTran->srBRec.uszOfflineBit != VS_TRUE)		||	
	     !memcmp(&guszMFES_MTI[0], "0400", 4)		||
	    (!memcmp(&guszMFES_MTI[0], "0800", 4)		&& 
	     pobTran->srBRec.inCode == _INQUIRY_ISSUER_BANK_))	&&
	     pobTran->srBRec.uszFiscTransBit != VS_TRUE		&&
	     pobTran->srBRec.inCode != _LOYALTY_REDEEM_		&&
	     pobTran->srBRec.inCode != _VOID_LOYALTY_REDEEM_)
	{
		/* 取消時，端末機上傳原正項交易ATS回覆值 */
		/* batchupload 也要送原交易值 */
		if (pobTran->srBRec.uszVOIDBit == VS_TRUE	||
		   (!memcmp(&guszMFES_MTI[0], "0320", 4)		&&
		     pobTran->srBRec.uszOfflineBit != VS_TRUE))
		{
			/* 變動長度 8 or Max 31 */
			inNCLen = strlen(pobTran->srBRec.szMCP_BANKID);
			/* 長度大於0才送原交易 DCC轉台幣會傳錯 */
			if (inNCLen > 0)
			{
				memcpy(&szPacket[inPacketCnt], "NC", 2); /* Table ID */
				inPacketCnt += 2;
				/* Sub Total Length */
				inPacketCnt ++;
				
				memset(szASCII, 0x00, sizeof(szASCII));
				sprintf(szASCII, "%02d", inNCLen);
				memset(uszBCD, 0x00, sizeof(uszBCD));
				inFunc_ASCII_to_BCD(uszBCD, szASCII, 1);
				szPacket[inPacketCnt] = uszBCD[0];
				inPacketCnt ++;

				memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szMCP_BANKID[0], inNCLen);
				inPacketCnt += inNCLen;
			}
			else
			{
				/* 原交易沒送就不用送 */
			}
		}
		else
		{
			/* 請求電文預設空白 */
			memcpy(&szPacket[inPacketCnt], "NC", 2); /* Table ID */
			inPacketCnt += 2;
			/* Sub Total Length */
			inPacketCnt ++;
			szPacket[inPacketCnt] = 0x07;
			inPacketCnt ++;
			memcpy(&szPacket[inPacketCnt], "    ", 4);
			inPacketCnt += 4;
			memcpy(&szPacket[inPacketCnt], "   ", 3);
			inPacketCnt += 3;
		}
	}
	
	/* 【需求單 - 108046】電子發票BIN大於6碼需求 Table"NI" by Russell 2019/7/8 上午 11:44 */
	/*
		●	Table ID “NI”: E-Invoice vehicle Issue Bank ID 
		支援電子發票載具之發卡行代碼。(請求電文填空白)
		上傳規則比照Table ID “NC”
	*/
	if ((!memcmp(&guszMFES_MTI[0], "0100", 4)		||
	     !memcmp(&guszMFES_MTI[0], "0200", 4)		||
	    (!memcmp(&guszMFES_MTI[0], "0220", 4)		&&
	     (pobTran->srBRec.inOrgCode == _SALE_OFFLINE_	|| 
	      pobTran->srBRec.inOrgCode == _INST_ADJUST_	|| 
	      pobTran->srBRec.inOrgCode == _REDEEM_ADJUST_	||
	      pobTran->srBRec.inOrgCode == _PRE_COMP_		||
	      pobTran->srBRec.inOrgCode == _CUP_PRE_COMP_	||
	      pobTran->srBRec.inOrgCode == _CUP_PRE_COMP_VOID_))||
	    (!memcmp(&guszMFES_MTI[0], "0320", 4)		&&
	      pobTran->srBRec.uszOfflineBit != VS_TRUE)		||	
	     !memcmp(&guszMFES_MTI[0], "0400", 4)		||
	    (!memcmp(&guszMFES_MTI[0], "0800", 4)		&& 
	     pobTran->srBRec.inCode == _INQUIRY_ISSUER_BANK_))	&&
	     pobTran->srBRec.uszFiscTransBit != VS_TRUE		&&
	     pobTran->srBRec.inCode != _LOYALTY_REDEEM_		&&
	     pobTran->srBRec.inCode != _VOID_LOYALTY_REDEEM_)
	{
		/* 取消時，端末機上傳原正項交易ATS回覆值 */
		/* batchupload 也要送原交易值 */
		if (pobTran->srBRec.uszVOIDBit == VS_TRUE	||
		   (!memcmp(&guszMFES_MTI[0], "0320", 4)		&&
		     pobTran->srBRec.uszOfflineBit != VS_TRUE))
		{
			/* 變動長度 8 or Max 31 */
			inEIVILen = strlen(pobTran->srBRec.szEIVI_BANKID);
			/* 長度大於0才送原交易 DCC轉台幣會傳錯 */
			if (inEIVILen > 0)
			{
				memcpy(&szPacket[inPacketCnt], "NI", 2); /* Table ID */
				inPacketCnt += 2;
				/* Sub Total Length */
				inPacketCnt ++;

				memset(szASCII, 0x00, sizeof(szASCII));
				sprintf(szASCII, "%02d", inEIVILen);
				memset(uszBCD, 0x00, sizeof(uszBCD));
				inFunc_ASCII_to_BCD(uszBCD, szASCII, 1);
				szPacket[inPacketCnt] = uszBCD[0];
				inPacketCnt ++;

				memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szEIVI_BANKID[0], inEIVILen);
				inPacketCnt += inEIVILen;
			}
			else
			{
				/* 原交易沒送就不用送 */
			}
		}
		else
		{
			/* ●	Table ID “NI”: E-Invoice vehicle Issue Bank ID */
			memcpy(&szPacket[inPacketCnt], "NI", 2); /* Table ID */
			inPacketCnt += 2;
			/* Sub Total Length */
			inPacketCnt ++;
			szPacket[inPacketCnt] = 0x06;
			inPacketCnt ++;
			memcpy(&szPacket[inPacketCnt], "      ", 6); /* 請求電文預設空白 */
			inPacketCnt += 6;
		}
	}
	
	/* 要送詢問電文的，回覆詢問電文的，一定會回L1 or L2 任一個，所以不用擋 */
	/* (需求單 - 107367)優惠查核改由ATS判斷 移除ASMCofig判斷 by Russell 2019/7/1 下午 1:46 */
	if (((memcmp(guszMFES_MTI, "0220", strlen("0220")) != 0) || (pobTran->inTransactionCode == _PRE_COMP_ || pobTran->inTransactionCode == _CUP_PRE_COMP_))	&&	/* ISOTxncode == _ADVICE_、_TC_UPLOAD_、_TIP_ (預先授權完成0220要可以做)*/
	    (memcmp(guszMFES_MTI, "0320", strlen("0320")) != 0)		&&	/* ISOTxncode == _BatchUpload_ */
	    (memcmp(guszMFES_MTI, "0800", strlen("0800")) != 0)		&&	/* ISOTxncode == CUP_LOGON */
	     pobTran->srBRec.uszFiscTransBit != VS_TRUE			&&	/* SMARTPAY 交易不送 */
	     pobTran->srBRec.inOrgCode != _REFUND_			&&	/* 退貨不送 */
	     pobTran->srBRec.inOrgCode != _INST_REFUND_			&&	/* 分期退貨不送 */
	     pobTran->srBRec.inOrgCode != _REDEEM_REFUND_		&&	/* 紅利退貨不送 */
	     pobTran->srBRec.inOrgCode != _PRE_AUTH_			&&	/* 預先授權不送 */
	     pobTran->srBRec.uszVOIDBit != VS_TRUE			&&	/* 取消不送(各式取消)*/
	     pobTran->srBRec.inOrgCode != _MAIL_ORDER_			&&	/* 郵購不送 */
	     pobTran->srBRec.inOrgCode != _LOYALTY_REDEEM_		&&	/* 優惠兌換 */
	     pobTran->srBRec.inOrgCode != _VOID_LOYALTY_REDEEM_		&&	/* 取消優惠兌換 */
	     pobTran->srBRec.inOrgCode != _CUP_REFUND_			&&	/* 銀聯退貨不送 */
	     pobTran->srBRec.inOrgCode != _CUP_PRE_AUTH_		&&	/* 銀聯預先授權不送 */
	     pobTran->srBRec.inOrgCode != _CUP_PRE_AUTH_VOID_		&&	/* 銀聯預先授權取消不送 */
	     pobTran->srBRec.inOrgCode != _CUP_PRE_COMP_VOID_		&&	/* 銀聯預先授權完成取消不送 */
	     pobTran->srBRec.inOrgCode != _CUP_MAIL_ORDER_		&&	/* 銀聯郵購不送 */
	     pobTran->srBRec.inOrgCode != _CUP_MAIL_ORDER_REFUND_	&&	/* 銀聯郵購退貨不送 */
	     pobTran->srBRec.inOrgCode != _CASH_ADVANCE_		&&	/* 預借現金不送 */
	     pobTran->srBRec.inOrgCode != _FORCE_CASH_ADVANCE_		&&	/* 預借現金補登不送 */
	   !(pobTran->srBRec.uszForceOnlineBit == VS_TRUE		&& 
	    (pobTran->srBRec.inOrgCode == _SALE_OFFLINE_	|| 
	     pobTran->srBRec.inOrgCode == _INST_ADJUST_		|| 
	     pobTran->srBRec.inOrgCode == _REDEEM_ADJUST_))		&&	/* _SALE_OFFLINE_、_INST_ADJUST_、_REDEEM_ADJUST_原為0220但若強制online則為0200，但還是不送 */
	     inNCCC_Loyalty_ASM_Flag() == VS_SUCCESS			&&
	     pobTran->srBRec.uszRewardL1Bit != VS_TRUE			&&
	     pobTran->srBRec.uszRewardL2Bit != VS_TRUE			&&
	     pobTran->srBRec.uszRewardL5Bit != VS_TRUE)
	{
		/* Table ID */
		memcpy(&szPacket[inPacketCnt], "L1", 2);
		inPacketCnt += 2;

		/* Sub Total Length */
		szPacket[inPacketCnt] = 0x00;
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
		memcpy(&szPacket[inPacketCnt], pobTran->srBRec.szAwardSN, 22);
		inPacketCnt += 22;
	}
	
	/* 原交易有收到L1 */
	if (pobTran->srBRec.uszRewardL1Bit == VS_TRUE)
	{
		/* 暫停優惠服務(取消就不送L1) */
		if (pobTran->srBRec.uszRewardSuspendBit == VS_TRUE)
		{
			pobTran->srBRec.uszRewardL1Bit = VS_FALSE;
		}
		else
		{
			if ((memcmp(guszMFES_MTI, "0220", strlen("0220")) == 0 && (pobTran->srBRec.inOrgCode != _PRE_COMP_ && pobTran->srBRec.inOrgCode != _CUP_PRE_COMP_))	||	/* ISOTxncode == _ADVICE_、_TC_UPLOAD_、_TIP_*/
			    (memcmp(guszMFES_MTI, "0320", strlen("0320")) == 0))	/* ISOTxncode == _BatchUpload_ */
			{
				/* 即使原交易有也不送 */
			}
			else
			{
				memcpy(&szPacket[inPacketCnt], "L1", 2); /* Table ID */
				inPacketCnt += 2;
				/* Sub Total Length */
				inPacketCnt ++;
				szPacket[inPacketCnt] = 0x23;
				inPacketCnt ++;

				/* Data */
				/* 優惠個數 */
				memcpy(&szPacket[inPacketCnt], pobTran->srBRec.szAwardNum, 1);
				inPacketCnt += 1;
				/* 優惠序號 */
				memcpy(&szPacket[inPacketCnt], pobTran->srBRec.szAwardSN, 22);
				inPacketCnt += 22;
			}
		}

	}
	
	/* 原交易有L2 */
	if (pobTran->srBRec.uszRewardL2Bit == VS_TRUE)
	{
		/*
			取消時，若是原正項交易只有廣告，則端末機不需上傳Table ID “L2”。
			取消時，原交易的優惠資訊為暫停優惠服務，則端末機不需上傳Table ID “L2”。
		*/
		if ((pobTran->srBRec.uszVOIDBit == VS_TRUE						&& 
		     atoi(pobTran->srBRec.szAwardNum) == 1						&& 
		     pobTran->srBRec.uszL2PrintADBit == VS_TRUE)					||	/* 取消時，只須送一個，而且要送的是廣告就不送了 */
		    (pobTran->srBRec.uszRewardSuspendBit == VS_TRUE))						/* 暫停優惠服務 */
		{
			pobTran->srBRec.uszRewardL2Bit = VS_FALSE;
		}
		else
		{
			if ((memcmp(guszMFES_MTI, "0220", strlen("0220")) != 0 || (pobTran->srBRec.inOrgCode == _PRE_COMP_ || pobTran->srBRec.inOrgCode == _CUP_PRE_COMP_))	&&	/* ISOTxncode == _ADVICE_、_TC_UPLOAD_、_TIP_*/
			    (memcmp(guszMFES_MTI, "0320", strlen("0320")) != 0))	/* ISOTxncode == _BatchUpload_ */
			{
				memcpy(&szPacket[inPacketCnt], "L2", 2); /* Table ID */
				inPacketCnt += 2;
				/* Sub Total Length */
				inPacketCnt ++;
				szPacket[inPacketCnt] = 0x23;
				inPacketCnt ++;
				/* Data */
				/* 優惠個數 */
				memcpy(&szPacket[inPacketCnt], pobTran->srBRec.szAwardNum, 1);
				inPacketCnt += 1;
				/* 優惠序號 */
				memcpy(&szPacket[inPacketCnt], pobTran->srBRec.szAwardSN, 22);
				inPacketCnt += 22;
			}

		}

	}

	/* 原交易有收到L5 */
	if (pobTran->srBRec.uszRewardL5Bit == VS_TRUE)
	{
		/* 暫停優惠服務(取消就不送L5) */
		if (pobTran->srBRec.uszRewardSuspendBit == VS_TRUE)
		{
			pobTran->srBRec.uszRewardL5Bit = VS_FALSE;
		}
		else
		{
			if ((memcmp(guszMFES_MTI, "0220", strlen("0220")) == 0 && (pobTran->srBRec.inOrgCode != _PRE_COMP_ && pobTran->srBRec.inOrgCode != _CUP_PRE_COMP_))	||	/* ISOTxncode == _ADVICE_、_TC_UPLOAD_、_TIP_*/
			    (memcmp(guszMFES_MTI, "0320", strlen("0320")) == 0))	/* ISOTxncode == _BatchUpload_ */
			{
				/* 即使原交易有也不送 */
			}
			else
			{
				memcpy(&szPacket[inPacketCnt], "L5", 2); /* Table ID */
				inPacketCnt += 2;
				/* Sub Total Length */
				inPacketCnt ++;
				szPacket[inPacketCnt] = 0x23;
				inPacketCnt ++;

				/* Data */
				/* 優惠個數 */
				memcpy(&szPacket[inPacketCnt], pobTran->srBRec.szAwardNum, 1);
				inPacketCnt += 1;
				/* 優惠序號 */
				memcpy(&szPacket[inPacketCnt], pobTran->srBRec.szAwardSN, 22);
				inPacketCnt += 22;
			}
		}

	}
	
	/* TABLE "NE" */
	memset(szESCMode, 0x00, sizeof(szESCMode));
	inGetESCMode(szESCMode);
	if (memcmp(szESCMode, "Y", 1) == 0)
	{
		/* 安全認證、金融查詢、_TC_UPLOAD_、結帳都不送NE */
	        if (!memcmp(&guszMFES_MTI[0], "0800", 4)	||
		    !memcmp(&guszMFES_MTI[0], "0500", 4)	||
		    pobTran->inISOTxnCode == _TC_UPLOAD_)
		{
			
		}
		else
		{
			memcpy(&szPacket[inPacketCnt], "NE", 2);	/* Table ID */
			inPacketCnt += 2;
			inPacketCnt ++;
			szPacket[inPacketCnt] = 0x01; 	/* Sub-Data Total Length */
			inPacketCnt ++;

			if (pobTran->srBRec.inESCUploadMode == _ESC_STATUS_NOT_SUPPORTED_	||
			    pobTran->srBRec.inESCUploadMode == _ESC_STATUS_OVERLIMIT_		||
			   (pobTran->srBRec.inESCUploadMode == _ESC_STATUS_SUPPORTED_		&&
			    pobTran->srBRec.inSignStatus == _SIGN_BYPASS_))
			{
				memcpy(&szPacket[inPacketCnt], "N", 1);
			}
			else
			{       
				if (pobTran->inISOTxnCode == _SEND_ESC_ADVICE_)     
					memcpy(&szPacket[inPacketCnt], "N", 1); 
				else             
					memcpy(&szPacket[inPacketCnt], "Y", 1);
			}

			inPacketCnt ++;
		}
	}
	
	/* 免簽名需求《依照ATS、MFES授權主機回覆交易電文的Indicator判斷帳單是否列印免簽名 */
	/*
	 * 註：
	 * 	端末機支援Table ID “NQ”後，一併移除原端末機Table ID“NI”（有NQ沒NI）
	 * 	之功能及其端末機依TMS參數判斷免簽名之邏輯。
	 * 
	 * 	● Table ID “NQ”: ATS Quick Pay (快速支付免簽名) Flag
	 *	Message Type=0100 及 0200 的交易電文，端末機預設上傳
	 * 	Table ID “NQ”，且預設值=’N’。端末機依ATS回覆之值，
	 * 	決定該筆交易是否符合中心VEPS免簽名條件。
	*/
	if ((!memcmp(&guszMFES_MTI[0], "0100", 4)	||
	     !memcmp(&guszMFES_MTI[0], "0200", 4))	&&
	    (pobTran->srBRec.inCode != _LOYALTY_REDEEM_ && pobTran->srBRec.inCode != _VOID_LOYALTY_REDEEM_))
	{
		memcpy(&szPacket[inPacketCnt], "NQ", 2); /* Table ID */
		inPacketCnt += 2;
		/* Sub Total Length */
		inPacketCnt ++;
		szPacket[inPacketCnt] = 0x01;
		inPacketCnt ++;
		memcpy(&szPacket[inPacketCnt], "N", 1);
		inPacketCnt ++;
	}
	else if (pobTran->inISOTxnCode == _REVERSAL_)
	{
		/* MFES主機，預先授權完成、交易補登、紅利調帳、分期調帳Reversal，Field_59 Table ID NQ需同原交易不上傳。 */
		/* Reversal送NQ欄位另外做判斷，MFES與ATS不同，下面交易別MTI不是0100及0200 */
		if (pobTran->srBRec.inCode != _PRE_COMP_		&&
		    pobTran->srBRec.inCode != _TC_UPLOAD_		&&
		    pobTran->srBRec.inCode != _ADVICE_			&&
		    pobTran->srBRec.inCode != _TIP_			&&
		    pobTran->srBRec.inCode != _REDEEM_ADJUST_		&& 
		    pobTran->srBRec.inCode != _INST_ADJUST_		&&
		    pobTran->srBRec.inCode != _SALE_OFFLINE_		&&
		    pobTran->srBRec.inCode != _CUP_PRE_COMP_		&&
		    pobTran->srBRec.inCode != _CUP_PRE_COMP_VOID_	&&
		    pobTran->srBRec.inCode != _BATCH_UPLOAD_		&&
		    pobTran->srBRec.inOrgCode != _PRE_COMP_		&&
		    pobTran->srBRec.inOrgCode != _CUP_PRE_COMP_		&&
		    pobTran->srBRec.inOrgCode != _CUP_PRE_COMP_VOID_	&&
		    pobTran->srBRec.inOrgCode != _SALE_OFFLINE_)
		{
			memcpy(&szPacket[inPacketCnt], "NQ", 2); /* Table ID */
			inPacketCnt += 2;
			/* Sub Total Length */
			inPacketCnt ++;
			szPacket[inPacketCnt] = 0x01;
			inPacketCnt ++;
			memcpy(&szPacket[inPacketCnt], "N", 1);
			inPacketCnt ++;
		}
	}
	
	/*
	* ● Table ID “TD”： Trace ID 
	* (清算交易追蹤號及DFS 交易系統追蹤號)
	*/
	/* SMARTPAY不用帶"TD" */
	/* ESC status update一律不用帶 by Russell 2019/3/27 上午 10:09 */
	if (pobTran->srBRec.uszFiscTransBit == VS_TRUE		||
	    pobTran->srBRec.inCode == _LOYALTY_REDEEM_		||
	    pobTran->srBRec.inCode == _VOID_LOYALTY_REDEEM_	||
	    !memcmp(&guszMFES_MTI[0], "0500", 4)		||
	    !memcmp(&guszMFES_MTI[0], "0800", 4)		||
	    pobTran->inISOTxnCode == _SEND_ESC_ADVICE_)
	{

	}
	else if (pobTran->inISOTxnCode == _REVERSAL_)
	{
		memcpy(&szPacket[inPacketCnt], "TD", 2); 
		inPacketCnt += 2;	
		/* Sub Total Length */
		szPacket[inPacketCnt] = 0x00;
		inPacketCnt ++;
		szPacket[inPacketCnt] = 0x21;
		inPacketCnt ++;

		/* 清算交易追蹤號 */
		if (strlen(pobTran->srBRec.szTableTD_Data) > 0)
		{
			memcpy(&szPacket[inPacketCnt], pobTran->srBRec.szTableTD_Data, 15);
			inPacketCnt += 15;
		}
		else
		{
			memcpy(&szPacket[inPacketCnt], "               ", 15);
			inPacketCnt += 15;
		}

		/* DFS交易系統追蹤號 */
		if (strlen(pobTran->srBRec.szDFSTraceNum) > 0)
		{
			memcpy(&szPacket[inPacketCnt], pobTran->srBRec.szDFSTraceNum, 6);
			inPacketCnt += 6;
		}
		else
		{
			memcpy(&szPacket[inPacketCnt], "      ", 6);
			inPacketCnt += 6;
		}
	}
	else if (pobTran->inISOTxnCode == _BATCH_UPLOAD_)
	{
		/* 若沒值不補空白 */
		/* 清算交易追蹤號 */
		/* DFS交易系統追蹤號 */
		if (strlen(pobTran->srBRec.szTableTD_Data) > 0	&&
		    strlen(pobTran->srBRec.szDFSTraceNum) > 0)
		{
			memcpy(&szPacket[inPacketCnt], "TD", 2); 
			inPacketCnt += 2;	
			/* Sub Total Length */
			szPacket[inPacketCnt] = 0x00;
			inPacketCnt ++;
			szPacket[inPacketCnt] = 0x21;
			inPacketCnt ++;

			memcpy(&szPacket[inPacketCnt], pobTran->srBRec.szTableTD_Data, 15);
			inPacketCnt += 15;

			memcpy(&szPacket[inPacketCnt], pobTran->srBRec.szDFSTraceNum, 6);
			inPacketCnt += 6;
		}
	}
	else
	{
		if (!memcmp(&guszMFES_MTI[0], "0220", 4))
		{
			/* 修訂3.11 Advice(Offline Sale)不需要上傳Table ID"TD" */
			if (pobTran->inISOTxnCode == _TC_UPLOAD_ || pobTran->srBRec.inCode == _TIP_)
			{
				/* 若沒值不補空白 */
				/* 清算交易追蹤號 */
				/* DFS交易系統追蹤號 */
				if (strlen(pobTran->srBRec.szTableTD_Data) > 0	&&
				    strlen(pobTran->srBRec.szDFSTraceNum) > 0)
				{
					memcpy(&szPacket[inPacketCnt], "TD", 2); 
					inPacketCnt += 2;	
					/* Sub Total Length */
					szPacket[inPacketCnt] = 0x00;
					inPacketCnt ++;
					szPacket[inPacketCnt] = 0x21;
					inPacketCnt ++;

					memcpy(&szPacket[inPacketCnt], pobTran->srBRec.szTableTD_Data, 15);
					inPacketCnt += 15;

					memcpy(&szPacket[inPacketCnt], pobTran->srBRec.szDFSTraceNum, 6);
					inPacketCnt += 6;
				}
			}
			else if (pobTran->srBRec.inCode == _PRE_COMP_		||
				 pobTran->srBRec.inCode == _CUP_PRE_COMP_	||
				 pobTran->srBRec.inCode == _CUP_PRE_COMP_VOID_)
			{
				memcpy(&szPacket[inPacketCnt], "TD", 2); /* Table ID */
				inPacketCnt += 2;
				/* Sub Total Length */
				inPacketCnt ++;
				szPacket[inPacketCnt] = 0x21;
				inPacketCnt ++;
				memcpy(&szPacket[inPacketCnt], "                     ", 21);
				inPacketCnt += 21;
			}
		}
		else if (pobTran->srBRec.inCode == _REDEEM_ADJUST_ || pobTran->srBRec.inCode == _INST_ADJUST_)
		{
			/* 不送 */
		}
		else
		{
			if (pobTran->srBRec.uszVOIDBit == VS_TRUE)
			{
				if (strlen(pobTran->srBRec.szTableTD_Data) > 0	&&
				    strlen(pobTran->srBRec.szDFSTraceNum) > 0)
				{
					memcpy(&szPacket[inPacketCnt], "TD", 2); 
					inPacketCnt += 2;	
					/* Sub Total Length */
					szPacket[inPacketCnt] = 0x00;
					inPacketCnt ++;
					szPacket[inPacketCnt] = 0x21;
					inPacketCnt ++;

					memcpy(&szPacket[inPacketCnt], pobTran->srBRec.szTableTD_Data, 15);
					inPacketCnt += 15;

					memcpy(&szPacket[inPacketCnt], pobTran->srBRec.szDFSTraceNum, 6);
					inPacketCnt += 6;
				}
			}
			else
			{
				memcpy(&szPacket[inPacketCnt], "TD", 2); /* Table ID */
				inPacketCnt += 2;
				/* Sub Total Length */
				inPacketCnt ++;
				szPacket[inPacketCnt] = 0x21;
				inPacketCnt ++;
				memcpy(&szPacket[inPacketCnt], "                     ", 21);
				inPacketCnt += 21;
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
                inLogPrintf(AT, "inNCCC_MFES_Pack59() END!");
	
        return (inCnt);
}

/*
Function        :inNCCC_MFES_Pack60
Date&Time       :2016/12/27 上午 9:21
Describe        :Field_60:	Reserved-Private Data
		Record of Count must be Unique in the batch.(Batch Number)
*/
int inNCCC_MFES_Pack60(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 	inCnt = 0;
        long 	lnBatchNum = 0;
        char 	szTemplate[110 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_Pack60() START!");

	/*
	 For all reconciliation messages, this field will contain the 6 digit batch number.
	 This number may not be zero and will be represented in six bytes.
	 */
	/* 補充說明： TIPS的Reversal及Batch Upload，本欄位維持送Original Amount。 */
	if ((pobTran->inISOTxnCode == _TIP_) ||
	    (pobTran->inISOTxnCode == _BATCH_UPLOAD_ && pobTran->srBRec.inCode == _TIP_) ||
	    (pobTran->inISOTxnCode == _REVERSAL_ && pobTran->srBRec.inCode == _TIP_))
	{
		uszPackBuf[inCnt++] = 0x00;
		uszPackBuf[inCnt++] = 0x012;
		sprintf((char *) &uszPackBuf[inCnt], "%010ld00", pobTran->srBRec.lnTxnAmount);
		inCnt += 12;
	}
	else
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		if (inGetBatchNum(szTemplate) == VS_ERROR)
			return (VS_ERROR);

		lnBatchNum = atol(szTemplate);
		uszPackBuf[inCnt++] = 0x00;
		uszPackBuf[inCnt++] = 0x06;
		sprintf((char *) &uszPackBuf[inCnt], "%06ld", lnBatchNum);
		inCnt += 6;
	}
	
	if (ginDebug == VS_TRUE)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "F_60 [BatchNumber %06ld]", lnBatchNum);
		inLogPrintf(AT, szTemplate);
	}
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_Pack60() END!");

        return (inCnt);
}

/*
Function        :inNCCC_MFES_Pack62
Date&Time       :2016/12/27 上午 9:21
Describe        :Field_62:	Reserved-Private Data(Invoice Number)
*/
int inNCCC_MFES_Pack62(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 	inCnt = 0;
	char	szTemplate[100 + 1];

	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_Pack62() START!");
	
	
        uszPackBuf[inCnt++] = 0x00;
        uszPackBuf[inCnt++] = 0x06;
        sprintf((char *) &uszPackBuf[inCnt], "%06ld", pobTran->srBRec.lnOrgInvNum);
        inCnt += 6;
	
	if (ginDebug == VS_TRUE)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "F_62 [InvoiceNumber %06ld]",  pobTran->srBRec.lnOrgInvNum);
		inLogPrintf(AT, szTemplate);
	}
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_Pack62() END!");

        return (inCnt);
}

/*
Function        :inNCCC_MFES_Pack63
Date&Time       :2016/12/27 上午 9:24
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
int inNCCC_MFES_Pack63(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int		inCnt = 0;
        int		inAccumCnt, inBatchCnt, inPacketCnt = 0;
	int		inTxnCode;
	char		szASCII[4 + 1];
        char		szTemplate[100 + 1], szPacket[92 + 1];
	unsigned char	uszBCD[2 + 1];
        ACCUM_TOTAL_REC srAccumRec;

	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_Pack63() START!");
	
	/* 先分類，分期和紅利要送分期和紅利的資料 */
	if (pobTran->inISOTxnCode == _REVERSAL_)
	{
		/* _INST_SALE_和_REDEEM_SALE_ request 不用送很多資料，所以要分出來 */
		if (pobTran->srBRec.uszInstallmentBit == VS_TRUE && pobTran->inTransactionCode != _INST_SALE_)
			inTxnCode = _INST_DETAIL_;
		else if (pobTran->srBRec.uszRedeemBit == VS_TRUE && pobTran->inTransactionCode != _REDEEM_SALE_)
			inTxnCode = _REDEEM_DETAIL_;
		else
			inTxnCode = pobTran->inTransactionCode;
	}
	else if (pobTran->inISOTxnCode == _VOID_	||
		 pobTran->inISOTxnCode == _BATCH_UPLOAD_||
		 pobTran->inISOTxnCode == _TIP_		||
		 pobTran->inISOTxnCode == _TC_UPLOAD_	||
		 pobTran->inISOTxnCode == _SEND_ESC_ADVICE_)
	{
		if (pobTran->srBRec.uszInstallmentBit == VS_TRUE)
			inTxnCode = _INST_DETAIL_;
		else if (pobTran->srBRec.uszRedeemBit == VS_TRUE)
			inTxnCode = _REDEEM_DETAIL_;
		else
			inTxnCode = pobTran->srBRec.inOrgCode;
	}   
	else
		inTxnCode = pobTran->inISOTxnCode;
	
	memset(szPacket, 0x00, sizeof(szPacket));
	switch (inTxnCode)
	{
		case _SETTLE_ :
		case _CLS_BATCH_ :
			inBatchCnt = inBATCH_GetTotalCountFromBakFile_By_Sqlite(pobTran);
			if (inBatchCnt == 0)
			{
				strcpy(szPacket, "000000000000000");
				strcat(szPacket, "000000000000000");
				inPacketCnt += 30;
			}
			else
			{
				memset(&srAccumRec, 0x00, sizeof(srAccumRec));
				inAccumCnt = inACCUM_GetRecord(pobTran, &srAccumRec);
				if (inAccumCnt == VS_ERROR)
				{
					return (VS_ERROR);
				}
				else if (inAccumCnt == VS_NO_RECORD)
				{
					strcpy(szPacket, "000000000000000");
					strcat(szPacket, "000000000000000");
					inPacketCnt += 30;
				}
				else
				{
					/*  Sale Count +  Sale Amount */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					sprintf(szTemplate, "%03lu", srAccumRec.lnTotalSaleCount);
					strcpy(szPacket, szTemplate);
					inPacketCnt += 3;
					sprintf(szTemplate, "%010lld00", (srAccumRec.llTotalSaleAmount + srAccumRec.llTotalTipsAmount));
					strcat(szPacket, szTemplate);
					inPacketCnt += 12;
					/* Refund Count +  Refund Amount */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					sprintf(szTemplate, "%03lu", srAccumRec.lnTotalRefundCount);
					strcat(szPacket, szTemplate);
					inPacketCnt += 3;
					sprintf(szTemplate, "%010lld00", (0 - srAccumRec.llTotalRefundAmount));
					strcat(szPacket, szTemplate);
					inPacketCnt += 12;
				}
			}

			strcat(szPacket, "000000000000000000000000000000");
	                strcat(szPacket, "000000000000000000000000000000");
	                inPacketCnt += 60;
	                break;
		case _INST_SALE_ :
			/* Indicator & Period */
			sprintf(szPacket, "I%02lu", pobTran->srBRec.lnInstallmentPeriod);
			inPacketCnt = 3;
			break;
		case _INST_REFUND_ :
		case _INST_ADJUST_ :
		case _INST_DETAIL_ :
			/* Indicator (1 Byte) */
			memcpy(&szPacket[0], &pobTran->srBRec.szInstallmentIndicator[0], 1);
			inPacketCnt ++;
			/* 【Period (2 Byte)】【Down Payment (8 Byte)】
			   【Payment (8 Byte)】【Formality Fee (6 Byte)】【Response Code (2 Byte)】 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%02lu%08lu%08lu%06lu00",
						pobTran->srBRec.lnInstallmentPeriod,
						pobTran->srBRec.lnInstallmentDownPayment + pobTran->srBRec.lnTipTxnAmount,  /* 首期金額須加小費 */
						pobTran->srBRec.lnInstallmentPayment,
						pobTran->srBRec.lnInstallmentFormalityFee);
			strcat(szPacket, szTemplate);
			inPacketCnt += 26;
			break;
		case _REDEEM_SALE_ :
			/* Indicator */
			strcpy(szPacket, "1");
			inPacketCnt = 1;
			break;
		case _REDEEM_REFUND_ :
		case _REDEEM_ADJUST_ :
		case _REDEEM_DETAIL_ :
			/* Indicator (1 Byte) */
			memcpy(&szPacket[0], &pobTran->srBRec.szRedeemIndicator[0], 1);
			inPacketCnt ++;
			/* 【Points Of Redemption (8 Byte)】【Sign Of Balance (1 Byte)】
			   【Points of Balance (8 Byte)】【Paid Credit Amount (10 Byte)】
			   【Response Code (2 Byte)】 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%08lu %08lu%010lu00",
						pobTran->srBRec.lnRedemptionPoints,
						pobTran->srBRec.lnRedemptionPointsBalance,
						(pobTran->srBRec.lnRedemptionPaidCreditAmount + pobTran->srBRec.lnTipTxnAmount));
			strcat(szPacket, szTemplate);
			inPacketCnt += 29;
			break;
		case _VOID_ :
			break;
		default :
			return (VS_ERROR);
	} /* end switch() */
	
	/* 算〈MAC〉使用 */
	memset(gszMAC_F_63, 0x00, sizeof(gszMAC_F_63));
	memcpy(&gszMAC_F_63[0], &szPacket[0], 30);

	/* Pack Length */
	memset(szASCII, 0x00, sizeof(szASCII));
	sprintf(szASCII, "%04d", inPacketCnt);
	memset(uszBCD, 0x00, sizeof(uszBCD));
	inFunc_ASCII_to_BCD(uszBCD, szASCII, 2);
	memcpy((char *)&uszPackBuf[inCnt], &uszBCD[0], 2);
	inCnt += 2;
        /* Packet Data */
        memcpy((char *) &uszPackBuf[inCnt], &szPacket[0], inPacketCnt);
        inCnt += inPacketCnt;
	
	if (ginDebug == VS_TRUE)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "F_63 [InvoiceNumber %06ld]",  pobTran->srBRec.lnOrgInvNum);
		inLogPrintf(AT, szTemplate);
	}
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_Pack63() END!");

        return (inCnt);
}

/*
Function        :inNCCC_MFES_Pack64
Date&Time       :2016/12/27 上午 10:49
Describe        :Field_64	Reserved-Private Data(MFES無使用)
		1. 除了0800及0400之外，其餘交易電文皆要上傳MAC。
		2. 若連續三次MAC Error則EDC先自動關閉MAC上傳(含無輸入PIN的CUP交易)。
*/
int inNCCC_MFES_Pack64(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int     inCnt = 0;
	char	szTerminalID[8 + 1];
	char	szTemplate[100 + 1];
	char	szAscii[16 + 1];

	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_Pack64() START!");
		
	/* MIT = 〈0800〉 〈0400〉不用送 */
	if (memcmp(&guszMFES_MTI[0], "0800", 4) && memcmp(&guszMFES_MTI[0], "0400", 4))
	{
		memset(szTerminalID, 0x00, sizeof(szTerminalID));
		inGetTerminalID(szTerminalID);
		inNCCC_Func_MFES_GenMAC(pobTran, gszMAC_F_03, gszMAC_F_04, gszMAC_F_11, gszMAC_F_35, &szTerminalID[0], gszMAC_F_63);


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
                inLogPrintf(AT, "inNCCC_MFES_Pack64() END!");
	
        return (inCnt);
}

/*
Function        :inNCCC_MFES_Check03
Date&Time       :
Describe        :送和收的processing Code要一致
*/
int inNCCC_MFES_Check03(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, unsigned char *uszRecePacket)
{
        if (memcmp((char *)uszSendPacket, (char *)uszRecePacket, 3))
        {
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_MFES_Check04
Date&Time       :
Describe        :送和收的Amount要一致
*/
int inNCCC_MFES_Check04(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, unsigned char *uszRecePacket)
{
        if (memcmp((char *)uszSendPacket, (char *)uszRecePacket, 6))
        {
                //vdSGErrorMessage(NCCC_CHECK_ISO_FILED04_ERROR); /* 140 = 電文錯誤請重試 */
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_MFES_Check41
Date&Time       :
Describe        :送和收的TID要一致
*/
int inNCCC_MFES_Check41(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, unsigned char *uszRecePacket)
{
        if (memcmp((char *)uszSendPacket, (char *)uszRecePacket, 8))
        {
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_MFES_UnPack12
Date&Time       :
Describe        :
*/
int inNCCC_MFES_UnPack12(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
	if (pobTran->inISOTxnCode == _REVERSAL_ || pobTran->inISOTxnCode == _ADVICE_) /* 送 Advice 不要 Update 時間 */
		return (VS_SUCCESS);
	else
	{
		memset(pobTran->srBRec.szTime, 0x00, sizeof(pobTran->srBRec.szTime));
		inFunc_BCD_to_ASCII(&pobTran->srBRec.szTime[0], &uszUnPackBuf[0], 3);
	}

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_MFES_UnPack13
Date&Time       :
Describe        :
*/
int inNCCC_MFES_UnPack13(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
        /* 只改變月份和日期 */
	if (pobTran->inISOTxnCode == _REVERSAL_ || pobTran->inISOTxnCode == _ADVICE_) /* 送 Advice 不要 Update 時間 */
		return (VS_SUCCESS);
	else
	{
		memset(&pobTran->srBRec.szDate[4], 0x00, 4);
		inFunc_BCD_to_ASCII(&pobTran->srBRec.szDate[4], &uszUnPackBuf[0], 2);
	}

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_MFES_UnPack24
Date&Time       :2016/12/23 下午 1:47
Describe        : 1. NPS 需求要判斷 Field_24 第一個 Byte 是否為【9】
		2. 此 Flag 是由主機做控管，端末機不管例外處理
*/
int inNCCC_MFES_UnPack24(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
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
Function        :inNCCC_MFES_UnPack38
Date&Time       :2016/12/23 下午 1:47
Describe        :
*/
int inNCCC_MFES_UnPack38(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
        if (pobTran->inISOTxnCode == _REVERSAL_ || pobTran->inISOTxnCode == _CUP_LOGON_)
                return (VS_SUCCESS);
	else
	{
		memset(pobTran->srBRec.szAuthCode, 0x00, sizeof(pobTran->srBRec.szAuthCode));
		memcpy(&pobTran->srBRec.szAuthCode[0], (char *) &uszUnPackBuf[0], _AUTH_CODE_SIZE_);
	}

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_MFES_UnPack39
Date&Time       :2016/12/23 下午 1:47
Describe        :Response code
*/
int inNCCC_MFES_UnPack39(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{	
	char	szTemplate[6 + 1];

	memset(&szTemplate, 0x00, sizeof(szTemplate));
	inFunc_BCD_to_ASCII(&szTemplate[0], &guszNCCC_MFES_ISO_Field03[0], 3);

	memset(pobTran->srBRec.szRespCode, 0x00, sizeof(pobTran->srBRec.szRespCode));
	memcpy(&pobTran->srBRec.szRespCode[0], (char *)&uszUnPackBuf[0], 2);

        /* 2011-05-24 PM 05:45:19 銀聯一般交易 01不能被改成05 */
	/* 【需求單 - 106306】	補登交易回覆Call Bank需求 by Russell 2019/10/5 上午 11:46 */
	if (pobTran->inISOTxnCode != _SALE_		&&
	    pobTran->inISOTxnCode != _CUP_SALE_		&&
	    pobTran->inISOTxnCode != _SALE_OFFLINE_	&&
	    ((!memcmp(&pobTran->srBRec.szRespCode[0], "01", 2)) || (!memcmp(&pobTran->srBRec.szRespCode[0], "02", 2))))
	{
		memset(pobTran->srBRec.szRespCode, 0x00, sizeof(pobTran->srBRec.szRespCode));
		strcpy(pobTran->srBRec.szRespCode, "05");
	}
	else if (pobTran->inISOTxnCode == _CLS_BATCH_ &&
		 !memcmp(&pobTran->srBRec.szRespCode[0], "95", 2) &&
		 !memcmp(&szTemplate[0], "960000", 6))
	{
		/*
			若是MTI:0510 + PCode:960000 MFES 仍回覆Response Code=95，則端末機視為結帳失敗。端末機下次執行結帳時，
			需從頭開始(MTI:0500 +PCode:920000)。
			此時需要重新BatchUpload，
		*/
		/* CLS SETTLE會在外面檢查回應碼，這邊不做動作 */

	}

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_MFES_UnPack55
Date&Time       :2016/12/23 下午 1:47
Describe        :
*/
int inNCCC_MFES_UnPack55(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
        int 	inLen;

	if (pobTran->inISOTxnCode == _REVERSAL_)
		return (VS_SUCCESS);

	inLen = ((uszUnPackBuf[0] / 16 * 10 + uszUnPackBuf[0] % 16) * 100);
	inLen += (uszUnPackBuf[1] / 16 * 10) + uszUnPackBuf[1] % 16;

	if (inLen > 0)
	{
		inNCCC_EMVUnPackData55(pobTran, &uszUnPackBuf[2], inLen);
	}
	else
	{
		return (VS_ERROR);
	}

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_MFES_UnPack58
Date&Time       :2016/12/23 下午 1:48
Describe        :
*/
int inNCCC_MFES_UnPack58(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
        int 	inCnt = 0, inSubTotalLength;
	char	szTemplate[20 + 1], szTemplate2[8 + 1];

	if (pobTran->inISOTxnCode == _REVERSAL_)
		return (VS_SUCCESS);

	/* Field total length. 2bytes*/
	inCnt += 2;
		
	/* Table 'A'*/
	inCnt += 1; /* Table ID */
	
	/* Sub Total Length */
	inSubTotalLength = uszUnPackBuf[inCnt];
	inCnt += 1;

	if (inSubTotalLength > 0)
	{
		/*
			●	Table ID 'NT': TMS 排程資訊
			補充說明：結帳成功時，MFES才需回覆Table ID 'NT'
			(即Process Code=920000或960000平帳(Response code =00)時，
			MFES才回覆Table ID 'NT',若不平帳(Response code =95)，
			則不需回覆Table ID 'NT')。
		*/
		
		if (inLoadTMSCPTRec(0) < 0)
		{
			inCnt += inSubTotalLength; /* 排程下載 SCHEDULE，不要影響結帳交易進行 */
		}
		else
		{
			/* Schedule 會以最後一個為主 如果之前有 Schedlue 一律當失敗 */
			/* 先清空排程日期時間 */
			inResetTMSCPT_Schedule();
			
			/* Host Date(TMS)	YYYYMMDD */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszUnPackBuf[inCnt], 8);
			inCnt += 8;
			/* Host Time(TMS)	HHMMSS */
			memset(szTemplate2, 0x00, sizeof(szTemplate2));
			memcpy(szTemplate2, &uszUnPackBuf[inCnt], 6);
			inCnt += 6;
			
			inFunc_SetEDCDateTime(szTemplate, szTemplate2);
			/*
				Download Flag
				0 = 無需下載
				1 = 立即下載
				2 = 排程下載

			*/
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszUnPackBuf[inCnt], 1);
			inSetTMSDownloadFlag(szTemplate);
			inCnt ++;
			/*
				Download Scope
				1 = 應用程式下載與Full參數下載
				2 = Full參數下載
				3 = Partial參數下載
			*/
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszUnPackBuf[inCnt], 1);
			inSetTMSDownloadScope(szTemplate);
			inCnt ++;
			/*
				Download TEL	撥接TMS電話號碼
				空白 = 以端末機的TMS電話號碼預設值，進行下載。
				注意：此電話話碼不更新端末機的TMS電話號碼預設值。

			*/
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszUnPackBuf[inCnt], 15);
			if (strlen(szTemplate) > 0 && inFunc_CheckFullSpace(szTemplate) != VS_TRUE)
			{
				inSetTMSPhoneNumber(szTemplate);
			}
			inCnt += 15;
			/* Schedule Date	YYYYMMDD */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszUnPackBuf[inCnt], 8);
			inSetTMSScheduleDate(szTemplate);
			inCnt += 8;
			/* Schedule Time	HHMMSS */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszUnPackBuf[inCnt], 6);
			inSetTMSScheduleTime(szTemplate);
			inCnt += 6;

			if (inSaveTMSCPTRec(0) < 0)
				inLogPrintf(AT, "inNCCC_MFES_UnPack58()_ERROR"); /* 排程下載 SCHEDULE，不要影響結帳交易進行 */
		}
		
	}
	else
	{
		/* 這是防止主機沒有給 Field_58 */
		inResetTMSCPT_Schedule();
		
		/* 下載失敗 DCC 排程時間也要歸零 */
		inSetDCCDownloadMode(_NCCC_DCC_DOWNLOAD_MODE_NOARMAL_);
		inSaveEDCRec(0);
			
		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

/*
Function        :inNCCC_MFES_UnPack59
Date&Time       :2016/12/23 下午 1:48
Describe        :
*/
int inNCCC_MFES_UnPack59(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
        int 	inLen, inCnt = 0, inSubTotalLength;

        if (pobTran->inISOTxnCode == _REVERSAL_)
                return (VS_SUCCESS);

	inLen = ((uszUnPackBuf[0] / 16 * 10 + uszUnPackBuf[0] % 16) * 100);
	inLen += (uszUnPackBuf[1] / 16 * 10) + uszUnPackBuf[1] % 16;

	if (inLen > 0)
	{
		inCnt += 2;

		while (inLen > inCnt)
		{
			if (!memcmp(&uszUnPackBuf[inCnt], "C", 1))
			{
				/* Table ID “C” */
				inCnt += 1;		/* Table ID */
				/* Sub Total Length */
				inSubTotalLength = ((uszUnPackBuf[inCnt] / 16) * 10) + (uszUnPackBuf[inCnt] % 16);
				inCnt += 1;
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
			else if (!memcmp(&uszUnPackBuf[inCnt], "O", 1))
			{
				/* Table ID “O” */
				inCnt += 1;			/* Table ID */
				/* Sub Total Length */
				inSubTotalLength = ((uszUnPackBuf[inCnt + 1] / 16) * 10) + (uszUnPackBuf[inCnt + 1] % 16);
				inCnt += 1;
				/* Original Transaction Date(MMDD) */
				memset(pobTran->srBRec.szCUP_TD, 0x00, sizeof(pobTran->srBRec.szCUP_TD));
				memcpy(&pobTran->srBRec.szCUP_TD[0], &uszUnPackBuf[inCnt], 4);
				inCnt += 4;
				/* Original Amount Right justified and zeros filled on the left with 2 decimals. No decimal point. */
				inCnt += 12;
			}
			else if (!memcmp(&uszUnPackBuf[inCnt], "Y", 1))
			{
				/* Table ID “N3”: Host AD “YYYY” (系統西元年) */
				inCnt += 1; /* Table ID */
				/* Sub Total Length */
				inSubTotalLength = ((uszUnPackBuf[inCnt] / 16) * 10) + (uszUnPackBuf[inCnt] % 16);
				inCnt += 1;
				
				memcpy(&pobTran->srBRec.szDate[0], &uszUnPackBuf[inCnt], 4);
				
				inCnt += 4;
			}
			else if (!memcmp(&uszUnPackBuf[inCnt], "N8", 2))
			{
				/* ●	Table ID “N8”: CVV2、CVC2或4DBC */
				inCnt += 2; /* Table ID */
				/* Sub Total Length */
				inSubTotalLength = ((uszUnPackBuf[inCnt] % 16) * 100) + ((uszUnPackBuf[inCnt + 1] / 16) * 10) + (uszUnPackBuf[inCnt + 1] % 16);
				inCnt += 2;
				inCnt += inSubTotalLength;
			}
			/* 免簽名需求《依照ATS、MFES授權主機回覆交易電文的Indicator判斷帳單是否列印免簽名》 */
			/*
				註：
					端末機支援Table ID “NQ”後，一併移除原端末機Table ID“NI”
					之功能及其端末機依TMS參數判斷免簽名之邏輯。
			*/
			else if (!memcmp(&uszUnPackBuf[inCnt], "NQ", 2))
			{
				/* Table ID “NQ”: MFES Quick Pay (快速支付免簽名) Flag */
				inCnt += 2; /* Table ID */
				/* Sub Total Length */
				inSubTotalLength = ((uszUnPackBuf[inCnt] % 16) * 100) + ((uszUnPackBuf[inCnt + 1] / 16) * 10) + (uszUnPackBuf[inCnt + 1] % 16);
				inCnt += 2;
				/*
					MFES 主機判斷免簽名邏輯
					‘Y’=符合Quick Pay本交易可以免簽名。
					‘N’=不符合Quick Pay本交易需要簽名。(要印簽名欄)
				*/
				/* 只判斷是否免簽 */
				if (!memcmp(&uszUnPackBuf[inCnt], "Y", 1))
				{
					if (ginDebug == VS_TRUE)
					{
						char	szDebugMsg[100 + 1];

						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "回傳NQ判斷免簽名");
						inLogPrintf(AT, szDebugMsg);
					}
					pobTran->srBRec.uszVEPS_SignatureBit = VS_TRUE;
					/* 分期付款交易，使用感應卡進行交易，皆要簽名，不允許分期付款交易"免簽名"。 */
					if (pobTran->srBRec.uszInstallmentBit == VS_TRUE)
					{
						pobTran->srBRec.uszNoSignatureBit = VS_FALSE; /* 免簽名條件 */
					}
					else
					{
						pobTran->srBRec.uszNoSignatureBit = VS_TRUE;
					}
				}

				inCnt ++;
			}
			else if (!memcmp(&uszUnPackBuf[inCnt], "NC", 2))
			{
				/*
					●	Table ID “NC”: MCP Indicator & Issue Bank ID (行動支付標記及金融機構代碼)
					發卡行金融機構代碼。(請求電文填空白)
					(1) 國內卡->金融機構代碼
					    (NSSMBINR. NSSMBINR_FID的前三碼即為金融機構代碼)
					(2) 國外卡->固定值"999"
					    (NSSMBINR查無BIN即為外國卡)
					(3) ATS不管是否為MCP(行動支付)皆要回傳發卡行金融機構代碼。
				*/
				/* 【需求單 - 170608】行動支付標記及金融機構代碼 add by LingHsiung 2017-08-23 上午 11:48:57 */
				inCnt += 2; /* Table ID */
				/* Sub Total Length */
				inSubTotalLength = ((uszUnPackBuf[inCnt] % 16) * 100) + ((uszUnPackBuf[inCnt + 1] / 16) * 10) + (uszUnPackBuf[inCnt + 1] % 16);
				inCnt += 2;
				
				if (inSubTotalLength == 4)
				{
					/* MCP Indicator */
					memcpy(&pobTran->srBRec.szMCP_BANKID[0], &uszUnPackBuf[inCnt], inSubTotalLength);
					inCnt += 1;

					if (pobTran->srBRec.szMCP_BANKID[0] == 'T')
						pobTran->srBRec.uszMobilePayBit = VS_TRUE;
					else
						pobTran->srBRec.uszMobilePayBit = VS_FALSE;

					/* 【需求單 - 170608】行動支付標記及金融機構代碼 */
					/* 簡汶婷(Natasha Chien) <natasha.chien@nccc.com.tw> 2017-09-29 (週五) 下午 04:48 分期付款要簽名 */
					if (pobTran->srBRec.uszMobilePayBit == VS_TRUE && pobTran->srBRec.uszInstallmentBit != VS_TRUE)
					{
						pobTran->srBRec.uszNoSignatureBit = VS_TRUE;
					}
					
					/* 發卡行金融機構代碼 */
					inCnt += 3;
				}
				else
				{
					/* MCP Indicator */
					memcpy(&pobTran->srBRec.szMCP_BANKID[0], &uszUnPackBuf[inCnt], inSubTotalLength);
					inCnt += 1;

					if (pobTran->srBRec.szMCP_BANKID[0] == 'T')
						pobTran->srBRec.uszMobilePayBit = VS_TRUE;
					else
						pobTran->srBRec.uszMobilePayBit = VS_FALSE;

					/* 【需求單 - 170608】行動支付標記及金融機構代碼 */
					/* 簡汶婷(Natasha Chien) <natasha.chien@nccc.com.tw> 2017-09-29 (週五) 下午 04:48 分期付款要簽名 */
					if (pobTran->srBRec.uszMobilePayBit == VS_TRUE && pobTran->srBRec.uszInstallmentBit != VS_TRUE)
					{
						pobTran->srBRec.uszNoSignatureBit = VS_TRUE;
					}
					/* 【需求單 - 106349】自有品牌判斷需求 */
					/* 卡別簡碼，非國內自有品牌卡片一律回空白 */
					if (pobTran->srBRec.szMCP_BANKID[4] != 0x20)
						pobTran->srBRec.uszUCARDTransBit = VS_TRUE;
					
					/* 發卡行金融機構代碼 */
					inCnt += 3;
					
					/* 卡別簡碼 只有國內自有品牌卡片需要回覆卡別簡碼 */
					inCnt ++;
					
					/* 卡別名稱長度 只有國內自有品牌卡片需要回覆長度 非國內自有品牌卡片一律回覆00 */
					inCnt += 2;
					
					/* 卡別名稱 只有國內自有品牌卡片需要回覆此欄位 */
					inCnt += (inSubTotalLength - 7);
				}
			}
			/* 【需求單 - 108046】電子發票BIN大於6碼需求 Table"NI" by Russell 2019/7/8 上午 11:44 */
			else if (!memcmp(&uszUnPackBuf[inCnt], "NI", 2))
			{
				/*
					●	Table ID “NI”: E-Invoice vehicle Issue Bank ID 
					支援電子發票載具之發卡行代碼。(請求電文填空白)
					上傳規則比照Table ID “NC”
				*/
				inCnt += 2; /* Table ID */
				/* Sub Total Length */
				inSubTotalLength = ((uszUnPackBuf[inCnt] % 16) * 100) + ((uszUnPackBuf[inCnt + 1] / 16) * 10) + (uszUnPackBuf[inCnt + 1] % 16);
				inCnt += 2;
				memset(pobTran->srBRec.szEIVI_BANKID, 0x00, sizeof(pobTran->srBRec.szEIVI_BANKID)); 
				memcpy(&pobTran->srBRec.szEIVI_BANKID[0], &uszUnPackBuf[inCnt], inSubTotalLength);
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
				if (inNCCC_Loyalty_Save_Reward_Data(inSubTotalLength + 4, (char*)&uszUnPackBuf[inCnt - 4], _REWARD_FILE_NAME_) != VS_SUCCESS)
				{
					return (VS_ERROR);
				}
				
				/* 優惠個數(因為取消要送，所以還是必須存) */
				memcpy(pobTran->srBRec.szAwardNum, &uszUnPackBuf[inCnt], 1);
				
				if (uszUnPackBuf[inCnt] == '0')
				{
					/* 沒有要列印優惠或廣告資訊，有可能要列印補充資訊(暫停優惠服務) */
					if (uszUnPackBuf[inCnt + 23] == '1')
					{
						pobTran->srBRec.uszRewardL1Bit = VS_TRUE;
						pobTran->srBRec.uszRewardSuspendBit = VS_TRUE;		/* 暫停優惠服務(優惠活動為0且要印補充資訊) */
					}
					else
						pobTran->srBRec.uszRewardL1Bit = VS_FALSE;
				}
				else
				{
					pobTran->srBRec.uszRewardL1Bit = VS_TRUE;
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
				if (inNCCC_Loyalty_Save_Reward_Data(inSubTotalLength + 4, (char*)&uszUnPackBuf[inCnt - 4], _REWARD_FILE_NAME_) != VS_SUCCESS)
				{
					return (VS_ERROR);
				}
				
				/* 優惠個數(因為取消要送，所以還是必須存) */
				memcpy(pobTran->srBRec.szAwardNum, &uszUnPackBuf[inCnt], 1);

				/* 是否有AD，L2是否印AD，因L2原交易取消要判斷，只好增加 */
				if (uszUnPackBuf[inCnt + 392] == '1')
					pobTran->srBRec.uszL2PrintADBit = VS_TRUE;

				if (uszUnPackBuf[inCnt] == '0')
				{
					/* 沒有要列印優惠或廣告資訊，有可能要列印補充資訊(暫停優惠服務) */
					if (uszUnPackBuf[inCnt + 23] == '1')
					{
						pobTran->srBRec.uszRewardL2Bit = VS_TRUE;
						pobTran->srBRec.uszRewardSuspendBit = VS_TRUE;		/* 暫停優惠服務(優惠活動為0且要印補充資訊) */
					}
					else
						pobTran->srBRec.uszRewardL2Bit = VS_FALSE;
				}
				else
				{
					pobTran->srBRec.uszRewardL2Bit = VS_TRUE;
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
				if (inNCCC_Loyalty_Save_Reward_Data_L5(inSubTotalLength + 4, (char*)&uszUnPackBuf[inCnt - 4], _REWARD_FILE_NAME_) != VS_SUCCESS)
				{
					return (VS_ERROR);
				}
				
				/* 優惠個數(因為取消要送，所以還是必須存) */
				memcpy(pobTran->srBRec.szAwardNum, &uszUnPackBuf[inCnt], 1);
				
				if (uszUnPackBuf[inCnt] == '0')
				{
					/* 沒有要列印優惠或廣告資訊，有可能要列印補充資訊(暫停優惠服務) */
					if (uszUnPackBuf[inCnt + 23] == '1')
					{
						pobTran->srBRec.uszRewardL5Bit = VS_TRUE;
						pobTran->srBRec.uszRewardSuspendBit = VS_TRUE;		/* 暫停優惠服務(優惠活動為0且要印補充資訊) */
					}
					else
						pobTran->srBRec.uszRewardL5Bit = VS_FALSE;
				}
				else
				{
					pobTran->srBRec.uszRewardL5Bit = VS_TRUE;
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
				if (inNCCC_Loyalty_Save_Reward_Data(inSubTotalLength + 4, (char*)&uszUnPackBuf[inCnt - 4], _REWARD_FILE_NAME_) != VS_SUCCESS)
				{
					return (VS_ERROR);
				}
				
				inCnt += inSubTotalLength;
				
			}
			else if (!memcmp(&uszUnPackBuf[inCnt], "TD", 2))
			{
				inCnt += 2; /* Table ID */
				/* Sub Total Length */
				inSubTotalLength = ((uszUnPackBuf[inCnt] % 16) * 100) + ((uszUnPackBuf[inCnt + 1] / 16) * 10) + (uszUnPackBuf[inCnt + 1] % 16);
				inCnt += 2;
				
				memcpy(pobTran->srBRec.szTableTD_Data, &uszUnPackBuf[inCnt], 15);
				inCnt += 15;
				inSubTotalLength -= 15;
				
				if (inSubTotalLength > 0)
				{
					memcpy(pobTran->srBRec.szDFSTraceNum, &uszUnPackBuf[inCnt], 6);
					inCnt += 6;
				}
			}
			else
			{
				inCnt ++;
			}
		}
	}
	/* 接受F_59欄位為0(ex:優惠兌換時) */
	else if (inLen == 0)
	{
		
	}
	else
	{
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_MFES_UnPack60
Date&Time       :2016/12/23 下午 1:48
Describe        :
*/
int inNCCC_MFES_UnPack60(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
	int	inRetVal = VS_SUCCESS;
	
	if (pobTran->inISOTxnCode == _REVERSAL_)
		return (VS_SUCCESS);
	else if (memcmp(pobTran->srBRec.szRespCode, "A0", strlen("A0")) == 0 || pobTran->inISOTxnCode == _CUP_LOGON_)
	{
		inRetVal = inNCCC_MFES_TMKKeyExchange(pobTran, uszUnPackBuf);
	}
	else
	{
		/* Batch Upload 送BatchNum */
	}

	return (inRetVal);
}

/*
Function        :inNCCC_MFES_UnPack63
Date&Time       :2016/12/23 下午 1:48
Describe        :
*/
int inNCCC_MFES_UnPack63(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
        int	inRetVal = VS_ERROR, inCnt = 0, i;
	int	inSwitchTxnCode;
	char	szTemplate[100 + 1];

	if (pobTran->inISOTxnCode == _REVERSAL_)
		return (VS_SUCCESS);
	
	if (!memcmp(&pobTran->srBRec.szRespCode[0], "00", 2) || !memcmp(&pobTran->srBRec.szRespCode[0], "11", 2))
	{
		if (pobTran->inTransactionCode == _VOID_)
			inSwitchTxnCode = pobTran->srBRec.inOrgCode;
		else
			inSwitchTxnCode = pobTran->inTransactionCode;

		switch (inSwitchTxnCode)
		{
			case _INST_SALE_ :
			case _INST_REFUND_ :
			case _INST_ADJUST_ :
				inCnt += 2;		/* Length (2 Byte) */
				/* Indicator (1 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], (char *)&uszUnPackBuf[inCnt], 1);
				if (!memcmp(&szTemplate[0], "I", 1) || !memcmp(&szTemplate[0], "E", 1))
				{
					memset(pobTran->srBRec.szInstallmentIndicator, 0x00, sizeof(pobTran->srBRec.szInstallmentIndicator));
					memcpy(&pobTran->srBRec.szInstallmentIndicator[0], (char *)&uszUnPackBuf[inCnt], 1);
					inCnt ++;
				}
				else
				{
					memcpy(&pobTran->srBRec.szRespCode[0], "XI", 2);		/* 要顯示【拒絕交易XI】 */
					
					return (inRetVal);
				}

				/* Installment Period (2 Byte) 一定要是數字，不能是空白 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], (char *)&uszUnPackBuf[inCnt], 2);

				if (memcmp(&szTemplate[0], "  ", 2))
				{
					for (i = 0; i < strlen(szTemplate); i ++)
					{
						if ((szTemplate[i] >= '0') && (szTemplate[i] <= '9'))
							continue;
						else
						{
							memcpy(&pobTran->srBRec.szRespCode[0], "I0", 2);/* 要顯示【分期電文錯誤】 */
							return (inRetVal);
						}
					}

					pobTran->srBRec.lnInstallmentPeriod = atol(szTemplate);
					inCnt += 2;
				}
				else
				{/* 要顯示【拒絕交易XI】 */
					memcpy(&pobTran->srBRec.szRespCode[0], "XI", 2);		/* 要顯示【拒絕交易XI】 */
					
					return (inRetVal);
				}

				/* Down Payment (8 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], (char *)&uszUnPackBuf[inCnt], 8);

				if (memcmp(&szTemplate[0], "        ", 8))
				{
					for (i = 0; i < strlen(szTemplate); i ++)
					{
						if ((szTemplate[i] >= '0') && (szTemplate[i] <= '9'))
							continue;
						else
						{
							memcpy(&pobTran->srBRec.szRespCode[0], "I0", 2);/* 要顯示【分期電文錯誤】 */
							
							return (inRetVal);
						}
					}

					pobTran->srBRec.lnInstallmentDownPayment = atol(szTemplate);
					inCnt += 8;
				}
				else
				{
					memcpy(&pobTran->srBRec.szRespCode[0], "XI", 2);	/* 要顯示【拒絕交易XI】 */
					
					return (inRetVal);
				}

				/* Installment Payment (8 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], (char *)&uszUnPackBuf[inCnt], 8);

				if (memcmp(&szTemplate[0], "        ", 8))
				{
					for (i = 0; i < strlen(szTemplate); i ++)
					{
						if ((szTemplate[i] >= '0') && (szTemplate[i] <= '9'))
							continue;
						else
						{
							memcpy(&pobTran->srBRec.szRespCode[0], "I0", 2);	/* 要顯示【分期電文錯誤】 */
							
							return (inRetVal);
						}
					}

					pobTran->srBRec.lnInstallmentPayment = atol(szTemplate);
					inCnt += 8;
				}
				else
				{
					memcpy(&pobTran->srBRec.szRespCode[0], "XI", 2);			/* 要顯示【拒絕交易XI】 */
					return (inRetVal);
				}

				/* Formality Fee (6 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], (char *)&uszUnPackBuf[inCnt], 6);

				if (memcmp(&szTemplate[0], "      ", 6))
				{
					for (i = 0; i < strlen(szTemplate); i ++)
					{
						if ((szTemplate[i] >= '0') && (szTemplate[i] <= '9'))
							continue;
						else
						{
							memcpy(&pobTran->srBRec.szRespCode[0], "I0", 2);	/* 要顯示【分期電文錯誤】 */
							return (inRetVal);
						}
					}

					pobTran->srBRec.lnInstallmentFormalityFee = atol(szTemplate);
					inCnt += 6;
				}
				else
				{
					memcpy(&pobTran->srBRec.szRespCode[0], "XI", 2);	/* 要顯示【拒絕交易XI】 */
					
					return (inRetVal);
				}

//				/* Response Code 【先暫時忽略】 */
//				if (gszISODebug == VS_TRUE)
//				{
//					vdVERIX_PRINT_ChineseFont(" ", PRT_NORMAL, PRT_COLUMN_42);
//					vdVERIX_PRINT_ChineseFont("Installment Relative Date", PRT_NORMAL, PRT_COLUMN_42);
//					/* Indicator */
//					memset(szTemplate, 0x00, sizeof(szTemplate));
//					sprintf(szTemplate, "  Indicator = %s", pobTran->srBRec.szInstallmentIndicator);
//					vdVERIX_PRINT_ChineseFont(szTemplate, PRT_NORMAL, PRT_COLUMN_42);
//					/* Installment Period */
//					memset(szTemplate, 0x00, sizeof(szTemplate));
//					sprintf(szTemplate, "  Installment Period = %d", pobTran->srBRec.lnInstallmentPeriod);
//					vdVERIX_PRINT_ChineseFont(szTemplate, PRT_NORMAL, PRT_COLUMN_42);
//					/* Down Payment */
//					memset(szTemplate, 0x00, sizeof(szTemplate));
//					sprintf(szTemplate, "  Down Payment = %d", pobTran->srBRec.lnInstallmentDownPayment);
//					vdVERIX_PRINT_ChineseFont(szTemplate, PRT_NORMAL, PRT_COLUMN_42);
//					/* Installment Payment */
//					memset(szTemplate, 0x00, sizeof(szTemplate));
//					sprintf(szTemplate, "  Installment Payment = %d", pobTran->srBRec.lnInstallmentPayment);
//					vdVERIX_PRINT_ChineseFont(szTemplate, PRT_NORMAL, PRT_COLUMN_42);
//					/* Formality Fee */
//					memset(szTemplate, 0x00, sizeof(szTemplate));
//					sprintf(szTemplate, "  Formality Fee = %d", pobTran->srBRec.lnInstallmentFormalityFee);
//					vdVERIX_PRINT_ChineseFont(szTemplate, PRT_NORMAL, PRT_COLUMN_42);
//					vdVERIX_PRINT_ChineseFont(" ", PRT_NORMAL, PRT_COLUMN_42);
//					vdVERIX_PRINT_ChineseFont("==========================================", PRT_NORMAL, PRT_COLUMN_42);
//				}

				break;
			case _REDEEM_SALE_ :
			case _REDEEM_REFUND_ :
			case _REDEEM_ADJUST_ :
				inCnt += 2;		/* Length (2 Byte) */
				/* Indicator (1 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], (char *)&uszUnPackBuf[inCnt], 1);
				if (!memcmp(&szTemplate[0], "1", 1) || !memcmp(&szTemplate[0], "2", 1))
				{
					memset(pobTran->srBRec.szRedeemIndicator, 0x00, sizeof(pobTran->srBRec.szRedeemIndicator));
					memcpy(&pobTran->srBRec.szRedeemIndicator[0], (char *)&uszUnPackBuf[inCnt], 1);
					inCnt ++;
				}
				else
				{
					memcpy(&pobTran->srBRec.szRespCode[0], "XI", 2);	/* 要顯示【拒絕交易XI】 */
					
					return (inRetVal);
				}

				/* Points Of Redemption (8 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], (char *)&uszUnPackBuf[inCnt], 8);

				if (memcmp(&szTemplate[0], "        ", 8))
				{
					for (i = 0; i < strlen(szTemplate); i ++)
					{
						if ((szTemplate[i] >= '0') && (szTemplate[i] <= '9'))
							continue;
						else
						{
							memcpy(&pobTran->srBRec.szRespCode[0], "R0", 2);	/* 要顯示【紅利電文錯誤】 */
							
							return (inRetVal);
						}
					}

					pobTran->srBRec.lnRedemptionPoints = atol(szTemplate);
					inCnt += 8;
				}
				else
				{
					memcpy(&pobTran->srBRec.szRespCode[0], "XI", 2);			/* 要顯示【拒絕交易XI】 */
					
					return (inRetVal);
				}

				/* Sign Of Balance (1 Byte) */
				inCnt ++;
				/* Points of Balance (8 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], (char *)&uszUnPackBuf[inCnt], 8);

				if (memcmp(&szTemplate[0], "        ", 8))
				{
					for (i = 0; i < strlen(szTemplate); i ++)
					{
						if ((szTemplate[i] >= '0') && (szTemplate[i] <= '9'))
							continue;
						else
						{
							memcpy(&pobTran->srBRec.szRespCode[0], "R0", 2);	/* 要顯示【紅利電文錯誤】 */
							return (inRetVal);
						}
					}

					pobTran->srBRec.lnRedemptionPointsBalance = atol(szTemplate);
					inCnt += 8;
				}
				else
				{
					memcpy(&pobTran->srBRec.szRespCode[0], "XI", 2);			/* 要顯示【拒絕交易XI】 */
					return (inRetVal);
				}

				/* Paid Credit Amount (10 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], (char *)&uszUnPackBuf[inCnt], 10);

				if (memcmp(&szTemplate[0], "          ", 10))
				{
					for (i = 0; i < strlen(szTemplate); i ++)
					{
						if ((szTemplate[i] >= '0') && (szTemplate[i] <= '9'))
							continue;
						else
						{
							memcpy(&pobTran->srBRec.szRespCode[0], "R0", 2);	/* 要顯示【紅利電文錯誤】 */
							
							return (inRetVal);
						}
					}

					pobTran->srBRec.lnRedemptionPaidCreditAmount = atol(szTemplate);
					inCnt += 8;
				}
				else
				{
					memcpy(&pobTran->srBRec.szRespCode[0], "XI", 2);			/* 要顯示【拒絕交易XI】 */
					
					return (inRetVal);
				}

//				/* Response Code 【先暫時忽略】 */
//				if (gszISODebug == VS_TRUE)
//				{
//					vdVERIX_PRINT_ChineseFont(" ", PRT_NORMAL, PRT_COLUMN_42);
//					vdVERIX_PRINT_ChineseFont("Online Redemption Relative Date", PRT_NORMAL, PRT_COLUMN_42);
//					/* Indicator */
//					memset(szTemplate, 0x00, sizeof(szTemplate));
//					sprintf(szTemplate, "  Indicator = %s", pobTran->srBRec.szRedemptionIndicator);
//					vdVERIX_PRINT_ChineseFont(szTemplate, PRT_NORMAL, PRT_COLUMN_42);
//					/* Points Of Redemption */
//					memset(szTemplate, 0x00, sizeof(szTemplate));
//					sprintf(szTemplate, "  Points Of Redemption = %d", pobTran->srBRec.lnRedemptionPoints);
//					vdVERIX_PRINT_ChineseFont(szTemplate, PRT_NORMAL, PRT_COLUMN_42);
//					/* Points of Balance */
//					memset(szTemplate, 0x00, sizeof(szTemplate));
//					sprintf(szTemplate, "  Points of Balance = %d", pobTran->srBRec.lnRedemptionPointsBalance);
//					vdVERIX_PRINT_ChineseFont(szTemplate, PRT_NORMAL, PRT_COLUMN_42);
//					/* Paid Credit Amount */
//					memset(szTemplate, 0x00, sizeof(szTemplate));
//					sprintf(szTemplate, "  Paid Credit Amount = %d", pobTran->srBRec.lnRedemptionPaidCreditAmount);
//					vdVERIX_PRINT_ChineseFont(szTemplate, PRT_NORMAL, PRT_COLUMN_42);
//					vdVERIX_PRINT_ChineseFont(" ", PRT_NORMAL, PRT_COLUMN_42);
//					vdVERIX_PRINT_ChineseFont("==========================================", PRT_NORMAL, PRT_COLUMN_42);
//				}
				break;
			default :
				break;
		}
		
	}
//	else if (!memcmp(&pobTran->srBRec.szRespCode[0], "01", 2) || !memcmp(&pobTran->srBRec.szRespCode[0], "02", 2))
//	{
//		if (!memcmp(szGetTRTFileName(), "NCCCTRT", 7))
//		{
//			memset(pobTran->szReferralPhoneNum, 0x00, sizeof(pobTran->szReferralPhoneNum));
//			memset(szTemplate, 0x00, sizeof(szTemplate));
//			sprintf(szTemplate, "%02x%02x" , uszUnPackBuf[0], uszUnPackBuf[1]);
//			memcpy(&pobTran->szReferralPhoneNum[0], (char *)&uszUnPackBuf[2], atoi(szTemplate)); /* Call Bank 電話 */
//		}
//	}
//	if (!memcmp(&uszUnPackBuf[inCnt], "N4", 2))
//	{
//		/* ●	Table ID “N4”: Installment Relation Data(分期付款資料) */
//		inCnt += 2; /* Table ID */
//		/* Sub Total Length */
//		inSubTotalLength = ((uszUnPackBuf[inCnt] % 16) * 100) + ((uszUnPackBuf[inCnt + 1] / 16) * 10) + (uszUnPackBuf[inCnt + 1] % 16);
//		inCnt += 2;
//		/* Installmet Indicator (分期標記)
//			‘I’=分期-手續費內含
//			‘E’=分期-手續費外加
//			請求電文預設值為’I’。
//		*/
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		memcpy(&szTemplate[0], (char *)&uszUnPackBuf[inCnt], 1);
//		if (!memcmp(&szTemplate[0], "I", 1) || !memcmp(&szTemplate[0], "E", 1))
//		{
//			memset(pobTran->srBRec.szInstallmentIndicator, 0x00, sizeof(pobTran->srBRec.szInstallmentIndicator));
//			memcpy(&pobTran->srBRec.szInstallmentIndicator[0], (char *)&uszUnPackBuf[inCnt], 1);
//			inCnt ++;
//		}
//		else
//		{
//			memcpy(&pobTran->srBRec.szRespCode[0], "XI", 2);
//			pobTran->srBRec.szRespCode[2] = 0x00; /* 要顯示【拒絕交易XI】 */
//			return (inRetVal);
//		}
//
//		/* Installment Period (期數)，右靠左補0 “01” ~ “99” */
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		memcpy(&szTemplate[0], (char *)&uszUnPackBuf[inCnt], 2);
//
//		if (memcmp(&szTemplate[0], "  ", 2))
//		{
//			for (i = 0; i < strlen(szTemplate); i ++)
//			{
//				if ((szTemplate[i] >= '0') && (szTemplate[i] <= '9'))
//					continue;
//				else
//				{
//					memcpy(&pobTran->srBRec.szRespCode[0], "I0", 2);
//					pobTran->srBRec.szRespCode[2] = 0x00; /* 要顯示【分期電文錯誤】 */
//					return (inRetVal);
//				}
//			}
//
//			pobTran->srBRec.lnInstallmentPeriod = atol(szTemplate);
//			inCnt += 2;
//		}
//		else
//		{
//			memcpy(&pobTran->srBRec.szRespCode[0], "XI", 2);
//			pobTran->srBRec.szRespCode[2] = 0x00; /* 要顯示【拒絕交易XI】 */
//			return (inRetVal);
//		}
//
//		/* Down Payment(首期金額)，右靠左補0 */
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		memcpy(&szTemplate[0], (char *)&uszUnPackBuf[inCnt], 8);
//
//		if (memcmp(&szTemplate[0], "        ", 8))
//		{
//			for (i = 0; i < strlen(szTemplate); i ++)
//			{
//				if ((szTemplate[i] >= '0') && (szTemplate[i] <= '9'))
//					continue;
//				else
//				{
//					memcpy(&pobTran->srBRec.szRespCode[0], "I0", 2);
//					pobTran->srBRec.szRespCode[2] = 0x00; /* 要顯示【分期電文錯誤】 */
//					return (inRetVal);
//				}
//			}
//
//			/* 2014/12/1 下午 03:26:52 修正小費重複加的問題 */
//			if (pobTran->srBRec.lnTipTxnAmount == 0L)
//				pobTran->srBRec.lnInstallmentDownPayment = atol(szTemplate);
//			else
//				pobTran->srBRec.lnInstallmentDownPayment = (atol(szTemplate) - pobTran->srBRec.lnTipTxnAmount);
//
//			inCnt += 8;
//		}
//		else
//		{
//			memcpy(&pobTran->srBRec.szRespCode[0], "XI", 2);
//			pobTran->srBRec.szRespCode[2] = 0x00; /* 要顯示【拒絕交易XI】 */
//			return (inRetVal);
//		}
//
//		/* Installment Payment(每期金額)，右靠左補0 */
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		memcpy(&szTemplate[0], (char *)&uszUnPackBuf[inCnt], 8);
//
//		if (memcmp(&szTemplate[0], "        ", 8))
//		{
//			for (i = 0; i < strlen(szTemplate); i ++)
//			{
//				if ((szTemplate[i] >= '0') && (szTemplate[i] <= '9'))
//					continue;
//				else
//				{
//					memcpy(&pobTran->srBRec.szRespCode[0], "I0", 2);
//					pobTran->srBRec.szRespCode[2] = 0x00; /* 要顯示【分期電文錯誤】 */
//					return (inRetVal);
//				}
//			}
//
//			pobTran->srBRec.lnInstallmentPayment = atol(szTemplate);
//			inCnt += 8;
//		}
//		else
//		{
//			memcpy(&pobTran->srBRec.szRespCode[0], "XI", 2);
//			pobTran->srBRec.szRespCode[2] = 0x00; /* 要顯示【拒絕交易XI】 */
//			return (inRetVal);
//		}
//
//		/* Formality Fee(手續費)，右靠左補0 */
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		memcpy(&szTemplate[0], (char *)&uszUnPackBuf[inCnt], 6);
//
//		if (memcmp(&szTemplate[0], "      ", 6))
//		{
//			for (i = 0; i < strlen(szTemplate); i ++)
//			{
//				if ((szTemplate[i] >= '0') && (szTemplate[i] <= '9'))
//					continue;
//				else
//				{
//					memcpy(&pobTran->srBRec.szRespCode[0], "I0", 2);
//					pobTran->srBRec.szRespCode[2] = 0x00; /* 要顯示【分期電文錯誤】 */
//					return (inRetVal);
//				}
//			}
//
//			pobTran->srBRec.lnInstallmentFormalityFee = atol(szTemplate);
//			inCnt += 6;
//		}
//		else
//		{
//			memcpy(&pobTran->srBRec.szRespCode[0], "XI", 2);
//			pobTran->srBRec.szRespCode[2] = 0x00; /* 要顯示【拒絕交易XI】 */
//			return (inRetVal);
//		}
//
//		/* Response Code 同原分期交易主機回覆值。“00”=Approved */
//		inCnt += 2;
//	}
//	else if (!memcmp(&uszUnPackBuf[inCnt], "N5", 2))
//	{
//		/* ●	Table ID “N5”: Redeem Relation Data(紅利扣抵資料) */
//		inCnt += 2; /* Table ID */
//		/* Sub Total Length */
//		inSubTotalLength = ((uszUnPackBuf[inCnt] % 16) * 100) + ((uszUnPackBuf[inCnt + 1] / 16) * 10) + (uszUnPackBuf[inCnt + 1] % 16);
//		inCnt += 2;
//		/*
//			Redeem Indicator (紅利標記)
//			‘1’=全額扣抵
//			‘2’=部分扣抵
//			‘3’=CLM全額扣抵
//			‘4’=CLM部分扣抵
//			‘6’= CLM部分扣抵，但不清算
//			請求電文預設值為’1’。
//		*/
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		memcpy(&szTemplate[0], (char *)&uszUnPackBuf[inCnt], 1);
//		if (!memcmp(&szTemplate[0], "1", 1) || !memcmp(&szTemplate[0], "2", 1))
//		{
//			memset(pobTran->srBRec.szRedeemIndicator, 0x00, sizeof(pobTran->srBRec.szRedeemIndicator));
//			memcpy(&pobTran->srBRec.szRedeemIndicator, (char *)&uszUnPackBuf[inCnt], 1);
//			inCnt ++;
//		}
//		else
//		{
//			memcpy(&pobTran->srBRec.szRespCode[0], "XI", 2);
//			pobTran->srBRec.szRespCode[2] = 0x00; /* 要顯示【拒絕交易XI】 */
//			return (inRetVal);
//		}
//
//		/* Point of Redemption (紅利扣抵點數)，右靠左補0。 */
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		memcpy(&szTemplate[0], (char *)&uszUnPackBuf[inCnt], 8);
//
//		if (memcmp(&szTemplate[0], "        ", 8))
//		{
//			for (i = 0; i < strlen(szTemplate); i ++)
//			{
//				if ((szTemplate[i] >= '0') && (szTemplate[i] <= '9'))
//					continue;
//				else
//				{
//					memcpy(&pobTran->srBRec.szRespCode[0], "R0", 2);
//					pobTran->srBRec.szRespCode[2] = 0x00; /* 要顯示【紅利電文錯誤】 */
//					return (inRetVal);
//				}
//			}
//			pobTran->srBRec.lnRedemptionPoints = atol(szTemplate);
//
//			inCnt += 8;
//		}
//		else
//		{
//			memcpy(&pobTran->srBRec.szRespCode[0], "XI", 2);
//			pobTran->srBRec.szRespCode[2] = 0x00; /* 要顯示【拒絕交易XI】 */
//			return (inRetVal);
//		}
//
//		/*
//			Sign of Balance(剩餘點數之正負值)
//			‘-‘，表負值 ‘ ’，表正值
//		*/
//		memset(pobTran->srBRec.szRedeemSignOfBalance, 0x00, sizeof(pobTran->srBRec.szRedeemSignOfBalance));
//		memcpy(&pobTran->srBRec.szRedeemSignOfBalance[0], (char *)&uszUnPackBuf[inCnt], 1);
//		inCnt ++;
//		/* Point of Balance(紅利剩餘點數)，右靠左補0。 */
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		memcpy(&szTemplate[0], (char *)&uszUnPackBuf[inCnt], 8);
//
//		if (memcmp(&szTemplate[0], "        ", 8))
//		{
//			for (i = 0; i < strlen(szTemplate); i ++)
//			{
//				if ((szTemplate[i] >= '0') && (szTemplate[i] <= '9'))
//					continue;
//				else
//				{
//					memcpy(&pobTran->srBRec.szRespCode[0], "R0", 2);
//					pobTran->srBRec.szRespCode[2] = 0x00; /* 要顯示【紅利電文錯誤】 */
//					return (inRetVal);
//				}
//			}
//
//			pobTran->srBRec.lnRedemptionPointsBalance = atol(szTemplate);
//			inCnt += 8;
//		}
//		else
//		{
//			memcpy(&pobTran->srBRec.szRespCode[0], "XI", 2);
//			pobTran->srBRec.szRespCode[2] = 0x00; /* 要顯示【拒絕交易XI】 */
//			return (inRetVal);
//		}
//
//		/* Paid Credit Amount(紅利支付金額)，右靠左補0。 */
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		memcpy(&szTemplate[0], (char *)&uszUnPackBuf[inCnt], 10);
//
//		if (memcmp(&szTemplate[0], "          ", 10))
//		{
//			for (i = 0; i < strlen(szTemplate); i ++)
//			{
//				if ((szTemplate[i] >= '0') && (szTemplate[i] <= '9'))
//					continue;
//				else
//				{
//					memcpy(&pobTran->srBRec.szRespCode[0], "R0", 2);
//					pobTran->srBRec.szRespCode[2] = 0x00; /* 要顯示【紅利電文錯誤】 */
//					return (inRetVal);
//				}
//			}
//
//			/* 2014/12/1 下午 03:26:52 修正小費重複加的問題 */
//			if (pobTran->srBRec.lnTipTxnAmount == 0L)
//				pobTran->srBRec.lnRedemptionPaidCreditAmount = atol(szTemplate);
//			else
//				pobTran->srBRec.lnRedemptionPaidCreditAmount = (atol(szTemplate) - pobTran->srBRec.lnTipTxnAmount);
//
//			inCnt += 8;
//		}
//		else
//		{
//			memcpy(&pobTran->srBRec.szRespCode[0], "XI", 2);
//			pobTran->srBRec.szRespCode[2] = 0x00; /* 要顯示【拒絕交易XI】 */
//			return (inRetVal);
//		}
//
//		/* Response Code 同原紅利交易主機回覆值。“00”=Approved */
//		inCnt += 2;
//	}
        return (VS_SUCCESS);
}

/*
Function        :inNCCC_MFES_ISOPackMessageType
Date&Time       :2016/9/14 下午 1:40
Describe        :把MTI放到PackData中，並放到guszMFES_MTI中以便在pack其他欄位時判斷
*/
int inNCCC_MFES_ISOPackMessageType(TRANSACTION_OBJECT *pobTran, int inISOTxnCode, unsigned char *uszPackData, char *szMTI)
{
        int 		inCnt = 0;
        unsigned char 	uszBCD[10 + 1];
	
	/* Force Sale(強制授權交易)，Field_25=”06”。Force交易定義為Online的補登交易。
	 * 分期調帳及紅利調帳處理邏輯比照【Force Sale(強制授權交易)】
	 * MFES Online補登交易，仍維持送0220 */

	/* 放到global中方便判斷 */
	memcpy((char*)guszMFES_MTI, szMTI, 4);
	
	/* 放到把MTI放到PackData中 */
        memset(uszBCD, 0x00, sizeof(uszBCD));
        inFunc_ASCII_to_BCD(&uszBCD[0], &szMTI[0], _NCCC_MFES_MTI_SIZE_);
        memcpy((char *) &uszPackData[inCnt], (char *) &uszBCD[0], _NCCC_MFES_MTI_SIZE_);
        inCnt += _NCCC_MFES_MTI_SIZE_;

        return (inCnt);
}

/*
Function        :inNCCC_MFES_ISOModifyBitMap
Date&Time       :2016/9/14 下午 1:40
Describe        :
*/
int inNCCC_MFES_ISOModifyBitMap(TRANSACTION_OBJECT *pobTran, int inISOTxnCode, int *inBitMap)
{
        char	szTRTFileName[12 + 1];
	char	szCommMode[2 + 1], szEncryptMode[2 + 1];
	char	szMACEnable[2 + 1];
        	
	
	/* Reversal要根據原交易別，決定要多送哪些欄位 */
	if (inISOTxnCode == _REVERSAL_)
	{
		/* 原交易有上傳F_38 Reversal才會上傳 */
		if (pobTran->inTransactionCode == _PRE_COMP_		|| pobTran->inTransactionCode == _CUP_PRE_COMP_	||
		    pobTran->inTransactionCode == _REDEEM_ADJUST_	|| pobTran->inTransactionCode == _INST_ADJUST_	||
		    pobTran->inTransactionCode == _SALE_OFFLINE_	|| pobTran->inTransactionCode == _TIP_		||
		    pobTran->inTransactionCode == _REFUND_		|| pobTran->inTransactionCode == _REDEEM_REFUND_||
		    pobTran->inTransactionCode == _INST_REFUND_		|| pobTran->inTransactionCode == _CUP_REFUND_	||
		    pobTran->inTransactionCode == _VOID_		|| pobTran->inTransactionCode == _CUP_VOID_	||
		    pobTran->inTransactionCode == _CUP_PRE_AUTH_VOID_	|| pobTran->inTransactionCode == _CUP_PRE_COMP_VOID_ ||
		    pobTran->inTransactionCode == _FORCE_CASH_ADVANCE_)
		{
			inNCCC_MFES_BitMapSet(inBitMap, 38);
		}
		/* SmartPay 要多送58但不送38 */
		else if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
		{
			inNCCC_MFES_BitMapSet(inBitMap, 58);
		}

	}

	/* BatchUpload 的特殊處理 */
	/* BatchUpload TIP要送F_54：Tip的金額 */
        if (pobTran->inISOTxnCode == _BATCH_UPLOAD_)
	{
		/* SmartPay 要多送58但不送38 */
		if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
		{
			inNCCC_MFES_BitMapReset(inBitMap, 38);
			inNCCC_MFES_BitMapSet(inBitMap, 58);
		}
		else if (pobTran->srBRec.inCode == _TIP_)
		{
			inNCCC_MFES_BitMapSet(inBitMap, 54);
		}
		else
		{
			/* 一般batch Upload不做處理 */
		}
		
	}
        
	/* 優惠兌換交易方式 使用條碼掃描不送 F_35 */
	if (pobTran->srBRec.inCode == _LOYALTY_REDEEM_ || pobTran->srBRec.inCode == _VOID_LOYALTY_REDEEM_)
	{
		/* 兌換方式用卡號才要送35 */
		if (strlen(pobTran->srBRec.szPAN) > 0)
			inNCCC_MFES_BitMapSet(inBitMap, 35);
		else
			inNCCC_MFES_BitMapReset(inBitMap, 35);

		/* 優惠兌換及取消優惠兌換的reveral不送Field 22 */
		inNCCC_MFES_BitMapReset(inBitMap, 22);
	}
	
	/* TIPS、 Advice (Offline Sale )、 EMV TC Upload、 Unbalance Upload要送F12、13
	   F_12、F_13 It does not change on subsequent transactions(i.e. adjustment). */
	if (pobTran->inTransactionCode == _SALE_OFFLINE_ ||
	    (pobTran->inISOTxnCode == _FORCE_CASH_ADVANCE_ && pobTran->srBRec.uszOfflineBit == VS_TRUE) ||
	    pobTran->inTransactionCode == _REDEEM_ADJUST_ ||
	    pobTran->inTransactionCode == _INST_ADJUST_)
        {
                memset(szTRTFileName, 0x00, sizeof(szTRTFileName));
                inGetTRTFileName(szTRTFileName);
        
		if (!memcmp(szTRTFileName, _TRT_FILE_NAME_CREDIT_, 1) && pobTran->srBRec.uszForceOnlineBit != VS_TRUE)
		{
			inNCCC_MFES_BitMapSet(inBitMap, 12);
			inNCCC_MFES_BitMapSet(inBitMap, 13);
		}
		
        }
	
	/* 有輸入櫃號才加送F_48 */
	/* 卡號查詢不送櫃號 */
	if (strlen(pobTran->srBRec.szStoreID) > 0		&&
	    pobTran->inTransactionCode != _INQUIRY_ISSUER_BANK_	&&
	    inISOTxnCode != _TIP_)
	{
		inNCCC_MFES_BitMapSet(inBitMap, 48);
	}

	/* 銀聯若有輸入PIN，要送PIN BLOCK */
	if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
	{
		/* REVERSAL 不用送 PIN_BLOCK */
		if (inISOTxnCode == _CUP_SALE_ || inISOTxnCode == _CUP_PRE_AUTH_)
		{
			if (strlen(pobTran->szPIN) > 0)
				inNCCC_MFES_BitMapSet(inBitMap, 52);
		}
	}

	/* F_55 判斷*/
	/* ESC轉紙本不須帶F_55 */
	if (pobTran->inISOTxnCode == _SEND_ESC_ADVICE_)
	{

	}
	/* 【需求單 - 106128】行動支付標記及金融機構代碼 */
	/* 查詢金融卡代碼不用帶55、56 */
	/* 不分卡別交易，所以加在最前面 */
	else if (pobTran->inTransactionCode == _INQUIRY_ISSUER_BANK_)
	{

	}
	/* 銀聯卡*/
	else if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
	{
		/* 銀聯取消用調閱編號時，不送F_55 */
		if (pobTran->srBRec.inChipStatus == _EMV_CARD_	&&
		    pobTran->srBRec.uszVOIDBit != VS_TRUE)
		{
			if (pobTran->srBRec.uszEMVFallBackBit == VS_TRUE)
			{
				inNCCC_MFES_BitMapSet(inBitMap, 56);
			}
			else
			{
				inNCCC_MFES_BitMapSet(inBitMap, 55);
				inNCCC_MFES_BitMapSet(inBitMap, 56);	
			}
			
		}
		else if (pobTran->srBRec.uszContactlessBit == VS_TRUE	&&
		         pobTran->srBRec.uszVOIDBit != VS_TRUE)
		{
			inNCCC_MFES_BitMapSet(inBitMap, 55);
			inNCCC_MFES_BitMapSet(inBitMap, 56);
		}
			
	}
	/* Fallback要帶F_56，除了銀聯取消 */
	else if (pobTran->srBRec.uszEMVFallBackBit == VS_TRUE)
	{
		inNCCC_MFES_BitMapSet(inBitMap, 56);
	}
	/* 一般晶片卡、感應卡、金融卡、金融感應卡 */
	else
	{
		if (pobTran->srBRec.inChipStatus == _EMV_CARD_ || pobTran->srBRec.uszContactlessBit == VS_TRUE)
		{
			if (!memcmp(&pobTran->srBRec.szAuthCode[0], "VLP", 3)				||
			    !memcmp(&pobTran->srBRec.szAuthCode[0], "JCB", 3)				||
			    pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE	||
			    pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_64_NEWJSPEEDY_MSD		||
			    pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE)
			{
				/* 吳升文 2015-05-25 (週一) 上午 11:26 信件說:
				   NewJspeedy 磁條感應卡，請比照 Paypass磁條 送 F_22 091X , 不送F_55。ATS及RFES 皆同。
				*/

				/* 如果是授權碼是【VLP】【JCB】及【SCHEME_ID_20_PAYPASS_MAG_STRIPE】【NewJspeedy MSG Mode】不帶晶片資料 */
			}
			else if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				/* 2015/4/7 上午 11:15:10 add by luko 新增Smart Pay交易不用送Field_55 */
			}
			else
			{
				inNCCC_MFES_BitMapSet(inBitMap, 55);
				inNCCC_MFES_BitMapSet(inBitMap, 56);
			}
			
		}
	
	}
	
	
	/* 有Tsam 要加送 F_57，除了0500和CUP LOGON */
	/* 撥被備援轉成撥接時，要送不加密的電文 */
	if (pobTran->uszDialBackup == VS_TRUE)
	{
		
	}
	else
	{
		memset(szCommMode, 0x00, sizeof(szCommMode));
		inGetCommMode(szCommMode);

		if (memcmp(szCommMode, _COMM_ETHERNET_MODE_, 1) == 0	|| 
		    memcmp(szCommMode, _COMM_GPRS_MODE_, 1) == 0	||
		    memcmp(szCommMode, _COMM_3G_MODE_, 1) == 0		||
		    memcmp(szCommMode, _COMM_4G_MODE_, 1) == 0)
		{
			if (inISOTxnCode == _SETTLE_ || inISOTxnCode == _CLS_BATCH_ || inISOTxnCode == _CUP_LOGON_)
			{

			}
			else
			{
				memset(szEncryptMode, 0x00, sizeof(szEncryptMode));
				inGetEncryptMode(szEncryptMode);

				if (memcmp(szEncryptMode, _NCCC_ENCRYPTION_TSAM_, 1) == 0)
					inNCCC_MFES_BitMapSet(inBitMap, 57);
			}
			
		}
	
	}
	
	/* 分期紅利對F_63的判斷(補強TC Upload和取消沒送的問題) */
	/* 【需求單 - 106128】行動支付標記及金融機構代碼 不帶63 */
	if ((pobTran->srBRec.uszRedeemBit == VS_TRUE		|| 
	     pobTran->srBRec.uszInstallmentBit == VS_TRUE)	&&
	     pobTran->inTransactionCode != _INQUIRY_ISSUER_BANK_)
		inNCCC_MFES_BitMapSet(inBitMap, 63);

	/* 預設除0400、0800外全部要送MAC，但若安全認證沒通過，信用卡交易不送F_64 */
	/* 這邊採用跟Verifone Code一樣的邏輯，安全認證時先砍Working Key，若找不到Key代表安全認證失敗 */
	/* CFGT 的 MacEnable沒On 或是 沒有Mac Key 或是 0220的交易 不送Mac */
	/* MFES F_64和ATS不同的地方有 ATS的0220和BatchUpload會送F_64 */
	/* MAC換key後，回3次A0或A1 */
	memset(szMACEnable, 0x00, sizeof(szMACEnable));
	inGetMACEnable(szMACEnable);
	if ((memcmp(szMACEnable, "Y", 1) != 0)						||	/* CFGT沒開 */
	    (inKMS_CheckKey(_TWK_KEYSET_NCCC_, _TWK_KEYINDEX_NCCC_MAC_) != VS_SUCCESS)	||	/* 安全認證沒過 */
	    (memcmp(guszMFES_MTI, "0220", 4) == 0)					||	/* 聯合說 0220 的交易不用送 MAC By Vx520 Code CUPproject.c line 4017 */
	     ginMacError >= 3)
	{
		inNCCC_MFES_BitMapReset(inBitMap, 64);
	}

        return (VS_SUCCESS);
}

int inNCCC_MFES_ISOModifyPackData(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackData, int *inPackLen)
{
	char	szCommMode[2 + 1];
	char	szEncryptionMode[2 + 1];
	
	/* 撥被備援轉成撥接時，要送不加密的電文 */
	if (pobTran->uszDialBackup == VS_TRUE)
	{
		
	}
	else
	{
		/* 用TSAM加密，要放TSam key index到最後一個Bit */
		memset(szCommMode, 0x00, sizeof(szCommMode));
		inGetCommMode(szCommMode);
		if (memcmp(szCommMode, _COMM_ETHERNET_MODE_, 1) == 0	|| 
		    memcmp(szCommMode, _COMM_GPRS_MODE_, 1) == 0	||
		    memcmp(szCommMode, _COMM_3G_MODE_, 1) == 0		||
		    memcmp(szCommMode, _COMM_4G_MODE_, 1) == 0)
		{
			memset(szEncryptionMode, 0x00, sizeof(szEncryptionMode));
			inGetEncryptMode(szEncryptionMode);
			
			if (memcmp(szEncryptionMode, _NCCC_ENCRYPTION_TSAM_, 1) == 0)
				uszPackData[4] = gusztSAMKeyIndex; /* TPDU 最後一個 Byte */
		}
		
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inNCCC_MFES_ISOCheckHeader
Date&Time       :2016/9/14 上午 10:17
Describe        :
*/
int inNCCC_MFES_ISOCheckHeader(TRANSACTION_OBJECT *pobTran, char *szSendISOHeader, char *szReceISOHeader)
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
Function        :inNCCC_MFES_ISOOnlineAnalyse
Date&Time       :2016/9/14 上午 10:17
Describe        :
*/
int inNCCC_MFES_ISOOnlineAnalyse(TRANSACTION_OBJECT *pobTran)
{
        int	inRetVal = VS_ERROR;

	if (pobTran->srBRec.inChipStatus == _EMV_CARD_ && (pobTran->inTransactionCode == _SALE_		||
							   pobTran->inTransactionCode == _INST_SALE_	||
							   pobTran->inTransactionCode == _REDEEM_SALE_	||
							   pobTran->inTransactionCode == _PRE_AUTH_	||
							   pobTran->inTransactionCode == _CUP_SALE_	||
							   pobTran->inTransactionCode == _CUP_PRE_AUTH_	||
							   pobTran->inTransactionCode == _CASH_ADVANCE_))
	{
		inRetVal = inNCCC_MFES_OnlineAnalyseEMV(pobTran); /* NCCCfunc.c */
	}
	else if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
        {
                inRetVal = inNCCC_MFES_OnlineAnalyseFISC(pobTran);
		
		if (inRetVal == VS_SUCCESS)
		{
			/* 紀錄已送到主機的調閱編號 */
			inNCCC_Func_Update_Memory_Invoice(pobTran);
		}
        }
	else
	{
		inRetVal = inNCCC_MFES_OnlineAnalyseMagneticManual(pobTran);
		
		/* 因為感應不會回存Response Code到8A 所以在這裡回存 */
		/* 解決 paypass感應交易完 0220 0320 F_55 沒有帶 TAG 8A 的問題，如果沒有收到 Response Code 則【8A】塞 "00" (START) */
		if ((pobTran->srBRec.uszContactlessBit == VS_TRUE) &&
		    (pobTran->srEMVRec.in8A_AuthRespCodeLen == 0) &&
		    ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_21_PAYPASS_MCHIP)	||
		     (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE)))
		{
			if (strlen(pobTran->srBRec.szRespCode) > 0)
			{
				memset(pobTran->srEMVRec.usz8A_AuthRespCode, 0x00, sizeof(pobTran->srEMVRec.usz8A_AuthRespCode));
				pobTran->srEMVRec.in8A_AuthRespCodeLen = strlen(pobTran->srBRec.szRespCode);
				memcpy(&pobTran->srEMVRec.usz8A_AuthRespCode[0], &pobTran->srBRec.szRespCode[0], pobTran->srEMVRec.in8A_AuthRespCodeLen);
			}
			else
			{
				pobTran->srEMVRec.in8A_AuthRespCodeLen = 2;
				memcpy(&pobTran->srEMVRec.usz8A_AuthRespCode[0], "00", 2);
			}
		}
		/* 解決 paypass感應交易完 0220 0320 F_55 沒有帶 TAG 8A 的問題，如果沒有收到 Response Code 則【8A】塞 "00" (END) */
		
		if (inRetVal == VS_SUCCESS)
		{
			/* 紀錄已送到主機的調閱編號 */
			inNCCC_Func_Update_Memory_Invoice(pobTran);
		}
	}
	
	if (inRetVal != VS_SUCCESS)
	{
		inUtility_StoreTraceLog_OneStep("Analyse Failed");
		inUtility_StoreTraceLog_OneStep("Response Code:%s", pobTran->srBRec.szRespCode);
		inUtility_StoreTraceLog_OneStep("Auth Code:%s", pobTran->srBRec.szAuthCode);
	}

        return (inRetVal);
}

/*
Function        :inNCCC_MFES_ISOAdviceAnalyse
Date&Time       :2016/9/14 上午 10:17
Describe        :
*/
int inNCCC_MFES_ISOAdviceAnalyse(TRANSACTION_OBJECT *pobTran, unsigned char *uszTcUpload)
{
        if (!memcmp(&pobTran->srBRec.szRespCode[0], "00", 2))
        {
                if (pobTran->srBRec.inCode == _TIP_)
                {
                        if (pobTran->srBRec.uszUpload1Bit == VS_TRUE && pobTran->srBRec.uszUpload2Bit == VS_TRUE && pobTran->srBRec.uszUpload3Bit == VS_FALSE)
                        {
                                pobTran->srBRec.uszUpload1Bit = VS_FALSE;
                                pobTran->srBRec.uszUpload2Bit = VS_TRUE;
                                pobTran->srBRec.uszUpload3Bit = VS_FALSE;
                        }
                        else
                        {
                                pobTran->srBRec.uszUpload1Bit = VS_FALSE;
                                pobTran->srBRec.uszUpload2Bit = VS_FALSE;
                                pobTran->srBRec.uszUpload3Bit = VS_FALSE;
                        }
                }
                else if (pobTran->srBRec.inCode == _ADJUST_)
                {
                        if (pobTran->srBRec.uszUpload1Bit == VS_TRUE && pobTran->srBRec.uszUpload2Bit == VS_TRUE && pobTran->srBRec.uszUpload3Bit == VS_FALSE)
                        {
                                pobTran->srBRec.uszUpload1Bit = VS_FALSE;
                                pobTran->srBRec.uszUpload2Bit = VS_TRUE;
                                pobTran->srBRec.uszUpload3Bit = VS_FALSE;
                        }
                        else
                        {
                                pobTran->srBRec.uszUpload1Bit = VS_FALSE;
                                pobTran->srBRec.uszUpload2Bit = VS_FALSE;
                                pobTran->srBRec.uszUpload3Bit = VS_FALSE;
                        }
                }
                else if (pobTran->srBRec.inCode == _VOID_)
                {
                        if (pobTran->srBRec.uszUpload1Bit == VS_TRUE && pobTran->srBRec.uszUpload2Bit == VS_TRUE && pobTran->srBRec.uszUpload3Bit == VS_TRUE)
                        {
                                pobTran->srBRec.uszUpload1Bit = VS_FALSE;
                                pobTran->srBRec.uszUpload2Bit = VS_TRUE;
                                pobTran->srBRec.uszUpload3Bit = VS_TRUE;
                        }
                        else if (pobTran->srBRec.uszUpload1Bit == VS_FALSE && pobTran->srBRec.uszUpload2Bit == VS_TRUE && pobTran->srBRec.uszUpload3Bit == VS_TRUE)
                        {
                                pobTran->srBRec.uszUpload1Bit = VS_FALSE;
                                pobTran->srBRec.uszUpload2Bit = VS_FALSE;
                                pobTran->srBRec.uszUpload3Bit = VS_TRUE;
                        }
                        else if (pobTran->srBRec.uszUpload1Bit == VS_TRUE && pobTran->srBRec.uszUpload2Bit == VS_TRUE && pobTran->srBRec.uszUpload3Bit == VS_FALSE)
                        {
                                pobTran->srBRec.uszUpload1Bit = VS_FALSE;
                                pobTran->srBRec.uszUpload2Bit = VS_TRUE;
                                pobTran->srBRec.uszUpload3Bit = VS_FALSE;
                        }
                        else
                        {
                                pobTran->srBRec.uszUpload1Bit = VS_FALSE;
                                pobTran->srBRec.uszUpload2Bit = VS_FALSE;
                                pobTran->srBRec.uszUpload3Bit = VS_FALSE;
                        }
                }
                else
                {
                        pobTran->srBRec.uszUpload1Bit = VS_FALSE;
                        pobTran->srBRec.uszUpload2Bit = VS_FALSE;
                        pobTran->srBRec.uszUpload3Bit = VS_FALSE;
                }

		/* 這裡是For非當筆，因為這裡的pobTran是ADVPobTran，所以TRT的update Batch不會存到這裡的改動 */
		/* 在這裡把bit On起來，避免TC重送 */
		if (pobTran->srBRec.inChipStatus == _EMV_CARD_ && pobTran->inISOTxnCode == _TC_UPLOAD_)
			pobTran->srBRec.uszTCUploadBit = VS_TRUE;
		
		/* 在這裡把bit On起來，避免Confirm重送 */
		if (pobTran->srBRec.inChipStatus == _EMV_CARD_ && pobTran->inISOTxnCode == _FISC_ADVICE_)
		{
			if (pobTran->srBRec.uszVOIDBit == VS_TRUE)
			{
				pobTran->srBRec.uszFiscVoidConfirmBit = VS_TRUE;
			}
			else
			{
				pobTran->srBRec.uszFiscConfirmBit = VS_TRUE;
			}
		}
		
                /* pobTran->uszUpdateBatchBit 表示 uszUpdateBatchBit / TRANS_BATCH_KEY】是要更新記錄 */
                pobTran->uszUpdateBatchBit = VS_TRUE;
	
		/* 若不是當筆TCUpload，而是送一般advice，要做batch更新和更新advice檔 */
		if (pobTran->uszTCUploadBit != VS_TRUE && pobTran->uszFiscConfirmBit != VS_TRUE)
		{
			/* 存檔 */
			if (inFLOW_RunFunction(pobTran, _UPDATE_BATCH_) != VS_SUCCESS)
				return (VS_ERROR);

			/* 如果【ADVICE】刪除失敗會鎖機，使用Global的Handle */
			if (inADVICE_Update(pobTran) != VS_SUCCESS)
			{
				/* 鎖機 */
				inFunc_EDCLock(AT);
				
				return (VS_ERROR);
			}
			
		}
		else
		{
			/* 當筆TCUplaod不更新batch檔和advice檔 */
		}
		
		return (VS_SUCCESS);
        }
        else
        {
		/* 只有結帳時前送advice錯誤時要顯示錯誤訊息，在inNCCC_MFES_ISOAdviceAnalyse裡顯示錯誤訊息 */
                /* 結帳交易流程中，若於前帳前補送電文，補送電文有拒絕或其他回覆碼的狀況，畫面皆顯示結帳失敗即可，不須顯示補送電文之回覆碼訊息。 */
        }

        return (VS_ERROR);
}

int inNCCC_MFES_BitMapSet(int *inBitMap, int inFeild)
{
        int 	i, inBMapCnt, inBitMapCnt;
        int 	inBMap[_NCCC_MFES_MAX_BIT_MAP_CNT_];

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

int inNCCC_MFES_BitMapReset(int *inBitMap, int inFeild)
{
        int 	i, inBMapCnt;
        int 	inBMap[_NCCC_MFES_MAX_BIT_MAP_CNT_];

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

int inNCCC_MFES_BitMapCheck(unsigned char *inBitMap, int inFeild)
{
        int 	inByteIndex, inBitIndex;

        inFeild--;
        inByteIndex = inFeild / 8;
        inBitIndex = 7 - (inFeild - inByteIndex * 8);

        if (_NCCC_MFES_CHECK_BIT_MAP_(inBitMap[inByteIndex], inBitIndex))
        {
                return (VS_TRUE);
        }

        return (VS_FALSE);
}

/*
Function        :inNCCC_MFES_CopyBitMap
Date&Time       :
Describe        :
*/
int inNCCC_MFES_CopyBitMap(int *inBitMap, int *inSearchBitMap)
{
        int 	i;

        for (i = 0; i < _NCCC_MFES_MAX_BIT_MAP_CNT_; i++)
        {
                if (inSearchBitMap[i] == 0)
                        break;
                else
                        inBitMap[i] = inSearchBitMap[i];
        }

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_MFES_GetBitMapTableIndex
Date&Time       :
Describe        :
*/
int inNCCC_MFES_GetBitMapTableIndex(ISO_TYPE_NCCC_MFES_TABLE *srISOFunc, int inBitMapTxnCode)
{
        int 	inBitMapIndex;

        for (inBitMapIndex = 0;; inBitMapIndex++)
        {
                if (srISOFunc->srBitMap[inBitMapIndex].inTxnID == _NCCC_MFES_NULL_TX_)
                {
                        return (VS_ERROR); /* 找不到相對應交易別的Bit Map */
                }

                if (srISOFunc->srBitMap[inBitMapIndex].inTxnID == inBitMapTxnCode)
                        break; /* 找到一樣的交易類別 */
        }

        return (inBitMapIndex);
}

/*
Function        :inNCCC_MFES_GetBitMapMessagegTypeField03
Date&Time       :2016/9/14 下午 1:34
Describe        :
*/
int inNCCC_MFES_GetBitMapMessagegTypeField03(TRANSACTION_OBJECT *pobTran, ISO_TYPE_NCCC_MFES_TABLE *srISOFunc, int inIsoType, int *inTxnBitMap, unsigned char *uszSendBuf)
{
        int 		i, j, k, inBitMapIndex, *inBitMap, inCnt, inBitMapTxnCode = -1;
        unsigned char 	uszBuf;
        char		szTemplate[64 + 1];

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_GetBitMapMessagegTypeField03() START!");
        
        if (ginDebug == VS_TRUE)
        {
                sprintf(szTemplate, "inIsoType = %d", inIsoType);
                inLogPrintf(AT, szTemplate);
        }      
        
        /* 設定交易別 */
        inBitMapTxnCode = inIsoType;
      
        /* 要搜尋 BIT_MAP_NCCC_MFES_TABLE srNCCC_MFES_ISOBitMap 相對應的 inTxnID */
        if ((inBitMapIndex = inNCCC_MFES_GetBitMapTableIndex(srISOFunc, inBitMapTxnCode)) == VS_ERROR)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inNCCC_MFES_GetBitMapTableIndex == VS_ERROR");
                
                return (VS_ERROR);
        }

        /* Pack Message Type */
        inCnt = 0;
        inCnt += srISOFunc->inPackMTI(pobTran, inIsoType, &uszSendBuf[inCnt], srISOFunc->srBitMap[inBitMapIndex].szMTI);

        /* 要搜尋 BIT_MAP_NCCC_MFES_TABLE srNCCC_MFES_ISOBitMap 相對應的 inBitMap */
        inNCCC_MFES_CopyBitMap(inTxnBitMap, srISOFunc->srBitMap[inBitMapIndex].inBitMap);

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

        inCnt += _NCCC_MFES_BIT_MAP_SIZE_;

        /* Process Code */
	/* 將MAC初始化 */
	memset(gszMAC_F_03, 0x00, sizeof(gszMAC_F_03));
	memset(gszMAC_F_04, 0x00, sizeof(gszMAC_F_04));
	memset(gszMAC_F_11, 0x00, sizeof(gszMAC_F_11));
	memset(gszMAC_F_35, 0x00, sizeof(gszMAC_F_35));
	memset(gszMAC_F_63, 0x00, sizeof(gszMAC_F_63));
		
        memset(guszNCCC_MFES_ISO_Field03, 0x00, sizeof(guszNCCC_MFES_ISO_Field03));
        inFunc_ASCII_to_BCD(&guszNCCC_MFES_ISO_Field03[0], srISOFunc->srBitMap[inBitMapIndex].szPCode, 3);

        if (inBitMapTxnCode == _REVERSAL_)
        {
                if (pobTran->inTransactionCode == _VOID_ || pobTran->inTransactionCode == _CUP_VOID_ ||
		    pobTran->inTransactionCode == _CUP_PRE_AUTH_VOID_ || pobTran->inTransactionCode == _CUP_PRE_COMP_VOID_)
                {
                        /*
			Processing Code - Activities
			020000 - Reversal Void Sale
			220000 - Reversal Void Refund
			320000 - Reversal Void Pre-Auth
			420000 - Reversal Void Pre-Auth Comp
                         */
                        switch (pobTran->srBRec.inOrgCode)
                        {
                                case _SALE_:
				/* 小費不能取消 */
				case _CUP_SALE_:
				case _INST_SALE_:
				case _REDEEM_SALE_:
                                case _SALE_OFFLINE_:
				case _INST_ADJUST_:
				case _REDEEM_ADJUST_:
                                        guszNCCC_MFES_ISO_Field03[0] = 0x02;
                                        guszNCCC_MFES_ISO_Field03[1] = 0x00;
                                        break;
					break;
                                case _REFUND_:
				case _INST_REFUND_:
				case _REDEEM_REFUND_:
				case _CUP_REFUND_:
                                        guszNCCC_MFES_ISO_Field03[0] = 0x22;
                                        guszNCCC_MFES_ISO_Field03[1] = 0x00;
                                        break;
				case _PRE_AUTH_:
				case _CUP_PRE_AUTH_:
					guszNCCC_MFES_ISO_Field03[0] = 0x32;
                                        guszNCCC_MFES_ISO_Field03[1] = 0x00;
					break;
				case _PRE_COMP_:
				case _CUP_PRE_COMP_:
					guszNCCC_MFES_ISO_Field03[0] = 0x42;
                                        guszNCCC_MFES_ISO_Field03[1] = 0x00;
					break;
                                default:
                                        guszNCCC_MFES_ISO_Field03[0] = 0x00;
                                        break;
                        }
                }
                else
                {
                        /*
			Processing Code - Activities
			000000 - Reversal Sale
			200000 - Reversal Refund
			300000 - Reversal Pre-Auth
			400000 - Reversal Pre-Auth Comp
                         */
                        switch (pobTran->inTransactionCode)
                        {
                                case _SALE_:
				case _TIP_:
				case _CUP_SALE_:
				case _INST_SALE_:
				case _REDEEM_SALE_:
                                case _SALE_OFFLINE_:
				case _INST_ADJUST_:
				case _REDEEM_ADJUST_:
                                        guszNCCC_MFES_ISO_Field03[0] = 0x00;
                                        guszNCCC_MFES_ISO_Field03[1] = 0x00;
                                        break;
                                case _REFUND_:
				case _INST_REFUND_:
				case _REDEEM_REFUND_:
				case _CUP_REFUND_:
                                        guszNCCC_MFES_ISO_Field03[0] = 0x20;
                                        guszNCCC_MFES_ISO_Field03[1] = 0x00;
                                        break;
				case _PRE_AUTH_:
				case _CUP_PRE_AUTH_:
					guszNCCC_MFES_ISO_Field03[0] = 0x30;
                                        guszNCCC_MFES_ISO_Field03[1] = 0x00;
					break;
				case _PRE_COMP_:
				case _CUP_PRE_COMP_:
					guszNCCC_MFES_ISO_Field03[0] = 0x40;
                                        guszNCCC_MFES_ISO_Field03[1] = 0x00;
					break;
				case _LOYALTY_REDEEM_:
					guszNCCC_MFES_ISO_Field03[0] = 0x70;
                                        guszNCCC_MFES_ISO_Field03[1] = 0x00;
					break;
				case _VOID_LOYALTY_REDEEM_:
					guszNCCC_MFES_ISO_Field03[0] = 0x72;
                                        guszNCCC_MFES_ISO_Field03[1] = 0x00;
					break;
                                default:
                                        guszNCCC_MFES_ISO_Field03[0] = 0x00;
                                        break;
                        }
			
                }
		
        }
        else if (inBitMapTxnCode == _BATCH_UPLOAD_)
        {
                /*
		XX000A
		XX:
		“00”  Sale
		“20”  Refund
		A: Don’t check this data.
		“0”  last transaction
		“1”  coming with next transaction
                 */
                switch (pobTran->srBRec.inOrgCode)
                {
			case _REFUND_:
			case _INST_REFUND_:
			case _REDEEM_REFUND_:
			case _CUP_REFUND_:
				guszNCCC_MFES_ISO_Field03[0] = 0x20;
				guszNCCC_MFES_ISO_Field03[1] = 0x00;
				break;
                        default:
				guszNCCC_MFES_ISO_Field03[0] = 0x00;
				guszNCCC_MFES_ISO_Field03[1] = 0x00;
                                break;
                }

                if (pobTran->uszLastBatchUploadBit == VS_TRUE)
                        guszNCCC_MFES_ISO_Field03[2] = 0x00;
                else
                        guszNCCC_MFES_ISO_Field03[2] = 0x01;

        }
	else if (inBitMapTxnCode == _SEND_ESC_ADVICE_)
	{
		/* ESC ADVICE照原交易 */
	}
        else if (inBitMapTxnCode == _VOID_ || inBitMapTxnCode == _CUP_VOID_ || pobTran->inTransactionCode == _CUP_PRE_AUTH_VOID_ || pobTran->inTransactionCode == _CUP_PRE_COMP_VOID_)
        {
                /*
                Processing Code - Activities
                020000 - Void Sale
		220000 - Void Refund
		320000 - Void Pre-Auth
		020000 - Void Pre-Auth Complete
                 */
                switch (pobTran->srBRec.inOrgCode)
                {
                        case _SALE_:
                        case _SALE_OFFLINE_:
                        case _TIP_:
			case _CUP_SALE_:
			case _INST_SALE_:
			case _REDEEM_SALE_:
			case _INST_ADJUST_:
			case _REDEEM_ADJUST_:
			case _MAIL_ORDER_:
                                guszNCCC_MFES_ISO_Field03[0] = 0x02;
				guszNCCC_MFES_ISO_Field03[1] = 0x00;
				break;
			case _REFUND_:
			case _INST_REFUND_:
			case _REDEEM_REFUND_:
			case _CUP_REFUND_:
				guszNCCC_MFES_ISO_Field03[0] = 0x22;
				guszNCCC_MFES_ISO_Field03[1] = 0x00;
				break;
			case _PRE_AUTH_:
			case _CUP_PRE_AUTH_:
				guszNCCC_MFES_ISO_Field03[0] = 0x32;
				guszNCCC_MFES_ISO_Field03[1] = 0x00;
				break;
			case _PRE_COMP_:	
			case _CUP_PRE_COMP_:
				guszNCCC_MFES_ISO_Field03[0] = 0x02;
				guszNCCC_MFES_ISO_Field03[1] = 0x00;
				break;
			case _LOYALTY_REDEEM_:
				guszNCCC_MFES_ISO_Field03[0] = 0x72;
				guszNCCC_MFES_ISO_Field03[1] = 0x00;
				break;
			default:
                                break;
                }
		
        }
	else if (inBitMapTxnCode == _ADVICE_ && pobTran->srBRec.inCode == _PRE_COMP_ && pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE)
	{
		/*  Advice(Offline Sale)新增DCC預先授權完成轉台幣支付的Processing Code=400000。 */
		guszNCCC_MFES_ISO_Field03[0] = 0x40;
		guszNCCC_MFES_ISO_Field03[1] = 0x00;
	}

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_GetBitMapMessagegTypeField03() END!");
        
        return (inCnt);
}

int inNCCC_MFES_PackISO(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendBuf, int inTxnCode)
{
        int				i, inSendCnt, inField, inCnt;
        int				inBitMap[_NCCC_MFES_MAX_BIT_MAP_CNT_ + 1];
        int				inRetVal, inISOFuncIndex = -1;
	char				szTemplate[42 + 1];
        char				szLogMessage[40 + 1];
	char				szCommMode[1 + 1];
	char				szEncryptMode[1 +1];
        unsigned char			uszBCD[20 + 1];
        ISO_TYPE_NCCC_MFES_TABLE 	srISOFunc;		
	
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_PackISO() START!");

        inSendCnt = 0;
        inField = 0;

        /* 加密模式，預設值 = 0。【0 = 不加密，1 = tSAM 加密】 */
	memset(szCommMode, 0x00, sizeof(szCommMode));
	inGetCommMode(szCommMode);

	if (memcmp(szCommMode, _COMM_MODEM_MODE_, 1) == 0)
		inISOFuncIndex = 0; /* 不加密 */
	else if (memcmp(szCommMode, _COMM_ETHERNET_MODE_, 1) == 0)
	{
		/* 初始化tSAM加密用欄位 */
		inNCCC_Func_Init_tSAM_Data();
		
		inGetEncryptMode(szEncryptMode);
		if (memcmp(szEncryptMode, _NCCC_ENCRYPTION_TSAM_, strlen(_NCCC_ENCRYPTION_TSAM_)) == 0)
		{
			inISOFuncIndex = 1;
		}
		else if (memcmp(szEncryptMode, _NCCC_ENCRYPTION_SOFTWARE_, strlen(_NCCC_ENCRYPTION_SOFTWARE_)) == 0)
		{
			inISOFuncIndex = 2;
		}
		else
		{
			inISOFuncIndex = 0;
		}
	}
	else if (memcmp(szCommMode, _COMM_GPRS_MODE_, 1) == 0	||
		 memcmp(szCommMode, _COMM_3G_MODE_, 1) == 0	||
		 memcmp(szCommMode, _COMM_4G_MODE_, 1) == 0)
	{
		/* 初始化tSAM加密用欄位 */
		inNCCC_Func_Init_tSAM_Data();
		
		inGetEncryptMode(szEncryptMode);
		if (memcmp(szEncryptMode, _NCCC_ENCRYPTION_TSAM_, strlen(_NCCC_ENCRYPTION_TSAM_)) == 0)
		{
			inISOFuncIndex = 1;
		}
		else if (memcmp(szEncryptMode, _NCCC_ENCRYPTION_SOFTWARE_, strlen(_NCCC_ENCRYPTION_SOFTWARE_)) == 0)
		{
			inISOFuncIndex = 2;
		}
		else
		{
			inISOFuncIndex = 0;
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szLogMessage, 0x00, sizeof(szLogMessage));
			sprintf(szLogMessage,"No such CommMode! szCommMode = %d", atoi(szCommMode));
			inLogPrintf(AT, szLogMessage);
		}
		return (VS_ERROR); /* 防呆 */
	}
	
	/* mfes有軟加密 */
	if (inISOFuncIndex >= 3 || inISOFuncIndex < 0)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szLogMessage, 0x00, sizeof(szLogMessage));
			sprintf(szLogMessage,"szEncryptMode ERROR!! szEncryptMode = %d",inISOFuncIndex);
			inLogPrintf(AT, szLogMessage);
		}
		return (VS_ERROR); /* 防呆 */
	}
	
        /* 決定要執行第幾個 Function Index */
        memset((char *) &srISOFunc, 0x00, sizeof(srISOFunc));
        memcpy((char *) &srISOFunc, (char *) &srNCCC_MFES_ISOFunc[inISOFuncIndex], sizeof(srISOFunc));
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
        memcpy((char *) &uszSendBuf[inSendCnt], (char *) uszBCD, _NCCC_MFES_TPDU_SIZE_);
        inSendCnt += _NCCC_MFES_TPDU_SIZE_;
        /* Get Bit Map / Mesage Type / Processing Code */
        inRetVal = inNCCC_MFES_GetBitMapMessagegTypeField03(pobTran, &srISOFunc, inTxnCode, inBitMap, &uszSendBuf[inSendCnt]);
        if (inRetVal == VS_ERROR)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inNCCC_MFES_GetBitMapMessagegTypeField03() ERROR!");

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
                        inCnt = srISOFunc.srPackISO[inField].inISOLoad(pobTran, &uszSendBuf[inSendCnt]);
                        
			/* pack的字數小於等於0，一定出錯 */
                        if (inCnt <= 0)
                        {
                                if (ginDebug == VS_TRUE)
                                {
                                        memset(szLogMessage, 0x00, sizeof(szLogMessage));
                                        sprintf(szLogMessage, "inField = %d Error", srISOFunc.srPackISO[inField].inFieldNum);
                                        inLogPrintf(AT, szLogMessage);
                                }

                                return (VS_ERROR);
                        }
                        else
                                inSendCnt += inCnt;
                }
		
        }

        if (srISOFunc.inModifyPackData != _NCCC_MFES_NULL_TX_)
                srISOFunc.inModifyPackData(pobTran, uszSendBuf, &inSendCnt);
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_PackISO() END!");

        return (inSendCnt);
}

int inNCCC_MFES_CheckUnPackField(int inField, ISO_FIELD_NCCC_MFES_TABLE *srCheckUnPackField)
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

int inNCCC_MFES_GetCheckField(int inField, ISO_CHECK_NCCC_MFES_TABLE *ISOFieldCheck)
{
        int 	i;

        for (i = 0; i < 64; i++)
        {
                if (ISOFieldCheck[i].inFieldNum == 0)
                        break;
                else if (ISOFieldCheck[i].inFieldNum == inField)
                {
                        return (i); /* i 是 ISO_CHECK_NCCC_MFES_TABLE 裡的第幾個 */
                }
        }

        return (VS_ERROR);
}

int inNCCC_MFES_GetFieldLen(int inField, unsigned char *uszSendData, ISO_FIELD_TYPE_NCCC_MFES_TABLE *srFieldType)
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
                        case _NCCC_MFES_ISO_ASC_:
                                inCnt += srFieldType[i].inFieldLen;
                                break;
                        case _NCCC_MFES_ISO_BCD_:
                                inCnt += srFieldType[i].inFieldLen / 2;
                                break;
                        case _NCCC_MFES_ISO_NIBBLE_2_:
                                inLen = (uszSendData[0] / 16 * 10) + (uszSendData[0] % 16);
				/* Smart Pay卡號上傳格式(Field_35)不同一般信用卡。一般信用卡的卡號需要Pack成 BCD Code，Smart pay的卡號/帳號直接上傳ASCII Code不進行Pack。 */
				/* 因為reversal不讀batch，沒辦法知道fiscBit，所以還是只能強制用長度判斷 */
				if (inLen == 54 && srFieldType[i].inFieldNum == 35)
					inCnt += inLen + 1;
				else
					inCnt += ((inLen + 1) / 2) + 1;
                                break;
                        case _NCCC_MFES_ISO_NIBBLE_3_:
                                inLen = (uszSendData[0] / 16 * 1000) + (uszSendData[0] % 16 * 100) + (uszSendData[1] / 16 * 10) + (uszSendData[1] % 16);
                                inCnt += ((inLen + 1) / 2) + 2;
                                break;
                        case _NCCC_MFES_ISO_BYTE_2_:
                                inLen = (uszSendData[0] / 16 * 10) + (uszSendData[0] % 16);
                                inCnt += inLen + 1;
                                break;
                        case _NCCC_MFES_ISO_BYTE_3_:
                                inLen = (uszSendData[0] / 16 * 1000) + (uszSendData[0] % 16 * 100) + (uszSendData[1] / 16 * 10) + (uszSendData[1] % 16);
                                inCnt += inLen + 2;
                                break;
                        case _NCCC_MFES_ISO_BYTE_2_H_:
                                inLen = (int) uszSendData[0];
                                inCnt += inLen + 1;
                                break;
                        case _NCCC_MFES_ISO_BYTE_3_H_:
                                inLen = ((int) uszSendData[0] * 0xFF) + (int) uszSendData[1];
                                inCnt += inLen + 1;
                                break;
			case _NCCC_MFES_ISO_BYTE_1_:
				inLen = (uszSendData[0] / 16 * 10) + (uszSendData[0] % 16);
				inCnt += inLen + 1;
				break;
                        default:
                                break;
                }

                break;
        }

        return (inCnt);
}

int inNCCC_MFES_GetFieldIndex(int inField, ISO_FIELD_TYPE_NCCC_MFES_TABLE *srFieldType)
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

int inNCCC_MFES_UnPackISO(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendBuf, unsigned char *uszRecvBuf)
{
	int				inRetVal;
        int				i, inRecvCnt, inCnt, inSendCnt, inSendField, inRecvField, inISOFuncIndex = -1;
        char				szBuf[_NCCC_MFES_TPDU_SIZE_ + _NCCC_MFES_MTI_SIZE_ + _NCCC_MFES_BIT_MAP_SIZE_ + 1];
        char				szErrorMessage[40 + 1];
	char				szCommMode[1 + 1];
	char				szEncryptMode[1 +1];
        unsigned char			uszSendMap[_NCCC_MFES_BIT_MAP_SIZE_ + 1], uszReceMap[_NCCC_MFES_BIT_MAP_SIZE_ + 1];
        ISO_TYPE_NCCC_MFES_TABLE 	srISOFunc;


        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_UnPackISO() START!");

        inSendField = inRecvField = 0;
        inSendCnt = inRecvCnt = 0;

        memset((char *) uszSendMap, 0x00, sizeof(uszSendMap));
        memset((char *) uszReceMap, 0x00, sizeof(uszReceMap));
        memset((char *) szBuf, 0x00, sizeof(szBuf));
	
	/* 加密模式，預設值 = 0。【0 = 不加密，1 = tSAM 加密】 */
	memset(szCommMode, 0x00, sizeof(szCommMode));
	inGetCommMode(szCommMode);
	if (memcmp(szCommMode, _COMM_MODEM_MODE_, 1) == 0)
		inISOFuncIndex = 0; /* 不加密 */
	else if (memcmp(szCommMode, _COMM_ETHERNET_MODE_, 1) == 0)
	{
		memset(szEncryptMode, 0x00, sizeof(szEncryptMode));
		inGetEncryptMode(szEncryptMode);
		inISOFuncIndex = atoi(szEncryptMode);
	}
	else if (memcmp(szCommMode, _COMM_GPRS_MODE_, 1) == 0	||
		 memcmp(szCommMode, _COMM_3G_MODE_, 1) == 0	||
		 memcmp(szCommMode, _COMM_4G_MODE_, 1) == 0)
	{
		memset(szEncryptMode, 0x00, sizeof(szEncryptMode));
		inGetEncryptMode(szEncryptMode);
		inISOFuncIndex = atoi(szEncryptMode);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szErrorMessage, 0x00, sizeof(szErrorMessage));
			sprintf(szErrorMessage,"No such CommMode! szCommMode = %d", atoi(szCommMode));
			inLogPrintf(AT, szErrorMessage);
		}
		return (VS_ERROR); /* 防呆 */
	}
	
	/* 新的〈MFES〉不支援軟加密 */
	if (inISOFuncIndex >= 3 || inISOFuncIndex < 0)
	{
                if (ginDebug == VS_TRUE)
                {
                        memset(szErrorMessage, 0x00, sizeof(szErrorMessage));
                        sprintf(szErrorMessage, "inISOFuncIndex : %d", inISOFuncIndex);
                        inLogPrintf(AT, szErrorMessage);
                }
                
		return (VS_ERROR); /* 防呆 */
	}
	
        memset((char *) &srISOFunc, 0x00, sizeof(srISOFunc));
        memcpy((char *) &srISOFunc, (char *) &srNCCC_MFES_ISOFunc[inISOFuncIndex], sizeof(srISOFunc));

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

        inSendCnt += _NCCC_MFES_TPDU_SIZE_;
        inRecvCnt += _NCCC_MFES_TPDU_SIZE_;
        inSendCnt += _NCCC_MFES_MTI_SIZE_;
        inRecvCnt += _NCCC_MFES_MTI_SIZE_;

        memcpy((char *) uszSendMap, (char *) &uszSendBuf[inSendCnt], _NCCC_MFES_BIT_MAP_SIZE_);
        memcpy((char *) uszReceMap, (char *) &uszRecvBuf[inRecvCnt], _NCCC_MFES_BIT_MAP_SIZE_);

        inSendCnt += _NCCC_MFES_BIT_MAP_SIZE_;
        inRecvCnt += _NCCC_MFES_BIT_MAP_SIZE_;

        /* 先檢查 ISO Field_39 */
        if (inNCCC_MFES_BitMapCheck(uszReceMap, 39) == VS_FALSE)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inNCCC_MFES_BitMapCheck(39) ERROR");

                return (VS_ERROR);
        }

        if (inNCCC_MFES_BitMapCheck(uszReceMap, 41) == VS_FALSE)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inNCCC_MFES_BitMapCheck(41) ERROR");

                return (VS_ERROR);
        }

        for (i = 1; i <= 64; i++)
        {
                /* 有送出去的 Field 但沒有收回來的 Field */
                if (inNCCC_MFES_BitMapCheck(uszSendMap, i) && !inNCCC_MFES_BitMapCheck(uszReceMap, i))
                {
                        inSendCnt += inNCCC_MFES_GetFieldLen(i, &uszSendBuf[inSendCnt], srISOFunc.srISOFieldType);
                }
                else if (inNCCC_MFES_BitMapCheck(uszReceMap, i)) /* 收的 BitMap */
                {
                        if (inNCCC_MFES_BitMapCheck(uszSendMap, i)) /* 送的 BitMap 都有 */
                        {
                                /* 是否要進行檢查封包資料包含【送】【收】 */
                                if ((inSendField = inNCCC_MFES_GetCheckField(i, srISOFunc.srCheckISO)) != VS_ERROR)
                                {
                                        if (srISOFunc.srCheckISO[inSendField].inISOCheck(pobTran, &uszSendBuf[inSendCnt], &uszRecvBuf[inRecvCnt]) != VS_SUCCESS)
                                        {
                                                if (ginDebug == VS_TRUE)
                                                {
                                                        memset(szErrorMessage, 0x00, sizeof(szErrorMessage));
                                                        sprintf(szErrorMessage, "inSendField = %d Error!", srISOFunc.srCheckISO[inSendField].inFieldNum);
                                                        inLogPrintf(AT, szErrorMessage);
                                                }

                                                return (_TRAN_RESULT_UNPACK_ERR_);
                                        }
                                }

                                inSendCnt += inNCCC_MFES_GetFieldLen(i, &uszSendBuf[inSendCnt], srISOFunc.srISOFieldType);
                        }

                        while (i > srISOFunc.srUnPackISO[inRecvField].inFieldNum) /* 只有收 */
                        {
                                inRecvField++;
                        }

                        if (i == srISOFunc.srUnPackISO[inRecvField].inFieldNum)
                        {
                                /* 要先檢查 UnpackISO 是否存在 */
                                if (inNCCC_MFES_CheckUnPackField(i, srISOFunc.srUnPackISO) == VS_SUCCESS)
				{
                                        inRetVal =srISOFunc.srUnPackISO[inRecvField].inISOLoad(pobTran, &uszRecvBuf[inRecvCnt]);
					
					if (inRetVal != VS_SUCCESS)
					{
						if (ginDebug == VS_TRUE)
						{
							memset(szErrorMessage, 0x00, sizeof(szErrorMessage));
							sprintf(szErrorMessage, "inRecvField = %d Error!", srISOFunc.srUnPackISO[inRecvField].inFieldNum);
							inLogPrintf(AT, szErrorMessage);
						}

						if (inRetVal == VS_WRITE_KEY_ERROR)
						{
							return (VS_WRITE_KEY_ERROR);
						}
						else
						{
							return (_TRAN_RESULT_UNPACK_ERR_);
						}
					}
				}
				
                        }
			
                        inCnt = inNCCC_MFES_GetFieldLen(i, &uszRecvBuf[inRecvCnt], srISOFunc.srISOFieldType);
                        if (inCnt == VS_ERROR)
                        {
                                if (ginDebug == VS_TRUE)
                                {
                                        memset(szErrorMessage, 0x00, sizeof(szErrorMessage));
                                        sprintf(szErrorMessage, "inRecvField = %d Error!", srISOFunc.srUnPackISO[inRecvField].inFieldNum);
                                        inLogPrintf(AT, szErrorMessage);
                                }

                                return (_TRAN_RESULT_UNPACK_ERR_);
                        }

                        inRecvCnt += inCnt;
                }
        }

        /* 這裡表示已經解完電文要檢查是否有回 ISO Field_38 */
        if (!memcmp(&pobTran->srBRec.szRespCode[0], "00", 2))
        {
                switch (pobTran->inISOTxnCode)
                {
			case _CUP_LOGON_:
				if (inNCCC_MFES_BitMapCheck(uszReceMap, 59) == VS_FALSE)
                                {
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "inNCCC_MFES_BitMapCheck 59 Error!");
                                        }

                                        return (VS_ERROR);
                                }
				
				if (inNCCC_MFES_BitMapCheck(uszReceMap, 60) == VS_FALSE)
                                {
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "inNCCC_MFES_BitMapCheck 60 Error!");
                                        }

                                        return (VS_ERROR);
                                }

                                break;
				
			case _CUP_SALE_ :
			case _CUP_PRE_AUTH_ :
				if (inNCCC_MFES_BitMapCheck(uszReceMap, 04) == VS_FALSE)
                                {
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "inNCCC_MFES_BitMapCheck 04 Error!");
                                        }

                                        return (VS_ERROR);
                                }
				
				if (inNCCC_MFES_BitMapCheck(uszReceMap, 59) == VS_FALSE)
                                {
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "inNCCC_MFES_BitMapCheck 59 Error!");
                                        }

                                        return (VS_ERROR);
                                }

                                break;
				
			case _CUP_REFUND_:
			case _CUP_PRE_COMP_:
				if (inNCCC_MFES_BitMapCheck(uszReceMap, 04) == VS_FALSE)
                                {
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "inNCCC_MFES_BitMapCheck 04 Error!");
                                        }

                                        return (VS_ERROR);
                                }
				
				if (inNCCC_MFES_BitMapCheck(uszReceMap, 38) == VS_FALSE)
                                {
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "inNCCC_MFES_BitMapCheck 38 Error!");
                                        }

                                        return (VS_ERROR);
                                }
				
				if (inNCCC_MFES_BitMapCheck(uszReceMap, 59) == VS_FALSE)
                                {
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "inNCCC_MFES_BitMapCheck 59 Error!");
                                        }

                                        return (VS_ERROR);
                                }

                                break;
			/* By R-Fes規格 1.39 不用回38 */
			case _CUP_VOID_:
			case _CUP_PRE_AUTH_VOID_:
			case _CUP_PRE_COMP_VOID_:
				
				if (inNCCC_MFES_BitMapCheck(uszReceMap, 04) == VS_FALSE)
                                {
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "inNCCC_MFES_BitMapCheck 04 Error!");
                                        }

                                        return (VS_ERROR);
                                }
				
				if (inNCCC_MFES_BitMapCheck(uszReceMap, 59) == VS_FALSE)
                                {
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "inNCCC_MFES_BitMapCheck 59 Error!");
                                        }

                                        return (VS_ERROR);
                                }

                                break;
			case _INST_SALE_:
			case _INST_ADJUST_:
			case _INST_REFUND_:
			case _REDEEM_SALE_:
			case _REDEEM_ADJUST_:
			case _REDEEM_REFUND_:
				
				if (inNCCC_MFES_BitMapCheck(uszReceMap, 04) == VS_FALSE)
                                {
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "inNCCC_MFES_BitMapCheck 04 Error!");
                                        }

                                        return (VS_ERROR);
                                }
				
				if (inNCCC_MFES_BitMapCheck(uszReceMap, 38) == VS_FALSE)
                                {
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "inNCCC_MFES_BitMapCheck 38 Error!");
                                        }

                                        return (VS_ERROR);
                                }
				
				if (inNCCC_MFES_BitMapCheck(uszReceMap, 63) == VS_FALSE)
                                {
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "inNCCC_MFES_BitMapCheck 63 Error!");
                                        }

                                        return (VS_ERROR);
                                }

                                break;
                        case _SALE_:
                        case _REFUND_:
			case _TIP_:
                        case _PRE_AUTH_:
			case _PRE_COMP_:
				if (inNCCC_MFES_BitMapCheck(uszReceMap, 04) == VS_FALSE)
                                {
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "inNCCC_MFES_BitMapCheck 04 Error!");
                                        }

                                        return (VS_ERROR);
                                }
				
                                if (inNCCC_MFES_BitMapCheck(uszReceMap, 38) == VS_FALSE)
                                {
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "inNCCC_MFES_BitMapCheck 38 Error!");
                                        }

                                        return (VS_ERROR);
                                }

                                break;
			/* By R-Fes規格 1.39 不用回38 */
			case _VOID_:
				if (inNCCC_MFES_BitMapCheck(uszReceMap, 04) == VS_FALSE)
                                {
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "inNCCC_MFES_BitMapCheck 04 Error!");
                                        }

                                        return (VS_ERROR);
                                }
				
				break;
                        default:
                                break;
                }
        }

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_UnPackISO() END!");

        return (VS_SUCCESS);
}

int inNCCC_MFES_GetSTAN(TRANSACTION_OBJECT *pobTran)
{
        char 	szSTANNum[12 + 1];

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_GetSTAN() START!");

        memset(szSTANNum, 0x00, sizeof(szSTANNum));
        if (inGetSTANNum(szSTANNum) == VS_ERROR)
                return (VS_ERROR);

        pobTran->srBRec.lnSTANNum = atol(szSTANNum);
        pobTran->srBRec.lnOrgSTANNum = atol(szSTANNum);

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_MFES_SetSTAN
Date&Time       :2015/12/24 早上 10:25
Describe        :STAN++
*/
int inNCCC_MFES_SetSTAN(TRANSACTION_OBJECT *pobTran)
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
Function        :inNCCC_MFES_ProcessReversal
Date&Time       :2016/9/13 下午 4:48
Describe        :
*/
int inNCCC_MFES_ProcessReversal(TRANSACTION_OBJECT *pobTran)
{
        int 	inRetVal;
        char 	szSendReversalBit[2 + 1];

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_ProcessReversal() START!");

        memset(szSendReversalBit, 0x00, sizeof(szSendReversalBit));
        if (inGetSendReversalBit(szSendReversalBit) == VS_ERROR)
                return (VS_ERROR);

        if (!memcmp(szSendReversalBit, "Y", 1))
        {
                if ((inRetVal = inNCCC_MFES_ReversalSendRecvPacket(pobTran)) != VS_SUCCESS)
                {
                        return (VS_ERROR);
                }

                if (inNCCC_MFES_SetReversalCnt(pobTran, _ADD_) == VS_ERROR)
                {
                        return (VS_ERROR);
                }
        }
	
        /* 檢查是否要產生 Reversal */
        if (pobTran->uszReversalBit == VS_TRUE)
        {
                if ((inRetVal = inNCCC_MFES_ReversalSave(pobTran)) != VS_SUCCESS)
                {
                        return (VS_ERROR);
                }
        }
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_ProcessReversal() END!");
	
        return (VS_SUCCESS);
}

/*
Function        :inNCCC_MFES_AdviceSendRecvPacket
Date&Time       :2016/9/14 上午 9:42
Describe        :
*/
int inNCCC_MFES_AdviceSendRecvPacket(TRANSACTION_OBJECT *pobTran, int inAdvCnt)
{
	int				inCheckTransactionCode;	/* 存inTransaction code用 For DCC 轉台幣預先授權完成失敗要Reversal用 */
        int				inCnt, inRetVal = VS_ERROR, inISOFuncIndex = -1;
        unsigned char			uszTCUpload = 0;
        TRANSACTION_OBJECT		ADVpobTran;
        ISO_TYPE_NCCC_MFES_TABLE 	srISOFunc;

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_AdviceSendRecvPacket() START!");
        
        /* 加密模式，預設值 = 0。【0 = 不加密，1 = tSAM 加密，2 = 軟體加密】 */
        inISOFuncIndex = 0; /* 不加密 */
        memset((char *) &srISOFunc, 0x00, sizeof(srISOFunc));
        memcpy((char *) &srISOFunc, (char *) &srNCCC_MFES_ISOFunc[inISOFuncIndex], sizeof(srISOFunc));

        memset((char *) &ADVpobTran, 0x00, sizeof(TRANSACTION_OBJECT));
        memcpy((char *) &ADVpobTran, (char *) pobTran, sizeof(TRANSACTION_OBJECT));

	/* 若批次第一筆TCupload會因為還沒存batch檔而失敗，所以TCUpload不做開啟動作 */
	if (ADVpobTran.uszTCUploadBit != VS_TRUE && ADVpobTran.uszFiscConfirmBit != VS_TRUE)
	{
		/* 開啟【.bat】【.bkey】【.adv】三個檔 */
		if (inBATCH_AdviceHandleReadOnly_By_Sqlite(pobTran) != VS_SUCCESS)
			return (VS_NO_RECORD);
	}

        /* 這裡的 for () 不可以在裡面直接call Return () ....  */
        for (inCnt = 0; inCnt < inAdvCnt; inCnt++)
        {
                if (ADVpobTran.uszTCUploadBit != VS_TRUE && ADVpobTran.uszFiscConfirmBit != VS_TRUE)
                {
			/* 讀最後一筆，最先進去會在最後一筆 */
			ADVpobTran.srBRec.lnOrgInvNum = _BATCH_LAST_RECORD_;
                        /* 這裡要開始逐一將【0220】交易上傳 */
			inRetVal = inBATCH_GetAdviceDetailRecord_By_Sqlite(&ADVpobTran, inCnt);
                        if (inRetVal == VS_SUCCESS)
                        {
                                inRetVal = VS_SUCCESS;
                        }
			else if (inRetVal == VS_NO_RECORD)
			{
				inRetVal = VS_NO_RECORD;
                                break;
			}
                        else
                        {
                                inRetVal = VS_ERROR;
                                break;
                        }
			
                }
                else
                {
                        inRetVal = VS_SUCCESS;
                }
		
		/* 信用卡當筆TC */
		if (ADVpobTran.uszTCUploadBit == VS_TRUE		&&
		    ADVpobTran.srBRec.inChipStatus == _EMV_CARD_	&&
		   (ADVpobTran.inTransactionCode == _SALE_		||
		    ADVpobTran.inTransactionCode == _INST_SALE_		|| 
		    ADVpobTran.inTransactionCode == _REDEEM_SALE_	|| 
                    ADVpobTran.inTransactionCode == _CUP_SALE_))
		{
			ADVpobTran.inISOTxnCode = _TC_UPLOAD_;		/* 當筆 */
		}
		/* MFES沒有SmartPay */
		else
		{
			/* 這裡因為銀聯和SmartPay交易別特殊，所以先分開 */
			if (ADVpobTran.srBRec.uszCUPTransBit == VS_TRUE)
			{
				if (ADVpobTran.srBRec.inChipStatus == _EMV_CARD_ && ADVpobTran.srBRec.uszTCUploadBit != VS_TRUE)
				{
					/* 銀聯卡沒有 Y1 or Y3 or Call Bank */
					/* 非特殊情況而且，TCUpload 未上傳，先送TCUpload */
					ADVpobTran.inISOTxnCode = _TC_UPLOAD_;

				}
				/* 晶片卡且TC已上傳 */
				else
				{
					/* 銀聯卡必online */
					inRetVal = VS_ERROR;
				}
			}/* 銀聯卡End */
			/* MFES沒有SmartPay*/
			else
			{
				if (ADVpobTran.srBRec.inChipStatus == _EMV_CARD_ && ADVpobTran.srBRec.uszTCUploadBit != VS_TRUE)
				{
					/* Y1 or Y3 or Call Bank */
					if (!memcmp(ADVpobTran.srBRec.szAuthCode, "Y1", 2) ||
					    !memcmp(ADVpobTran.srBRec.szAuthCode, "Y3", 2) ||
					    ADVpobTran.srBRec.uszReferralBit == VS_TRUE)
					{
						if (ADVpobTran.srBRec.inCode == _TIP_)
						{
							if (ADVpobTran.srBRec.uszUpload1Bit == VS_TRUE && ADVpobTran.srBRec.uszUpload2Bit == VS_TRUE && ADVpobTran.srBRec.uszUpload3Bit == VS_FALSE)
								ADVpobTran.inISOTxnCode = _ADVICE_; /* 先送原交易 */
							else
								ADVpobTran.inISOTxnCode = _TIP_; /* 送小費交易 */
						}
						else if (ADVpobTran.srBRec.inCode == _ADJUST_)
						{
							if (ADVpobTran.srBRec.uszUpload1Bit == VS_TRUE && ADVpobTran.srBRec.uszUpload2Bit == VS_TRUE && ADVpobTran.srBRec.uszUpload3Bit == VS_FALSE)
								ADVpobTran.inISOTxnCode = _ADVICE_; /* 先送原交易 */
							else
								ADVpobTran.inISOTxnCode = _ADJUST_; /* 送調帳交易 */
						}
						else if (ADVpobTran.srBRec.inCode == _VOID_)
						{
							if (ADVpobTran.srBRec.uszUpload1Bit == VS_TRUE && ADVpobTran.srBRec.uszUpload2Bit == VS_TRUE && ADVpobTran.srBRec.uszUpload3Bit == VS_TRUE)
							{
								ADVpobTran.inISOTxnCode = _ADVICE_;
							}
							else if (ADVpobTran.srBRec.uszUpload1Bit == VS_FALSE && ADVpobTran.srBRec.uszUpload2Bit == VS_TRUE && ADVpobTran.srBRec.uszUpload3Bit == VS_TRUE)
							{
								if (ADVpobTran.srBRec.inOrgCode == _TIP_)
									ADVpobTran.inISOTxnCode = _TIP_;
								else if (ADVpobTran.srBRec.inOrgCode == _ADJUST_)
									ADVpobTran.inISOTxnCode = _ADJUST_;
								else
									inRetVal = VS_ERROR;
							}
							else if (ADVpobTran.srBRec.uszUpload1Bit == VS_TRUE && ADVpobTran.srBRec.uszUpload2Bit == VS_TRUE && ADVpobTran.srBRec.uszUpload3Bit == VS_FALSE)
							{
								if (ADVpobTran.srBRec.inOrgCode == _SALE_OFFLINE_)
									ADVpobTran.inISOTxnCode = _ADVICE_;
								else
									inRetVal = VS_ERROR;
							}
							else
							{
								ADVpobTran.inISOTxnCode = _VOID_; /* 送取消交易 */
							}
						}
						else
						{
							/* 交易補登或是REFERRAL */
							ADVpobTran.inISOTxnCode = _ADVICE_; /* 表示做結帳前要先送【Advice】所以交易日期、時間不應該被更新 */
						}
					}
					else
					{
						/* 非特殊情況而且，TCUpload 未上傳，先送TCUpload */
						ADVpobTran.inISOTxnCode = _TC_UPLOAD_;
					}

				}
				/* 晶片卡且TC已上傳 */
				else
				{
					if (ADVpobTran.srBRec.inCode == _TIP_)
					{
						if (ADVpobTran.srBRec.uszUpload1Bit == VS_TRUE && ADVpobTran.srBRec.uszUpload2Bit == VS_TRUE && ADVpobTran.srBRec.uszUpload3Bit == VS_FALSE)
							ADVpobTran.inISOTxnCode = _ADVICE_; /* 先送原交易 */
						else
							ADVpobTran.inISOTxnCode = _TIP_; /* 送小費交易 */
					}
					else if (ADVpobTran.srBRec.inCode == _ADJUST_)
					{
						if (ADVpobTran.srBRec.uszUpload1Bit == VS_TRUE && ADVpobTran.srBRec.uszUpload2Bit == VS_TRUE && ADVpobTran.srBRec.uszUpload3Bit == VS_FALSE)
							ADVpobTran.inISOTxnCode = _ADVICE_; /* 先送原交易 */
						else
							ADVpobTran.inISOTxnCode = _ADJUST_; /* 送調帳交易 */
					}
					else if (ADVpobTran.srBRec.inCode == _VOID_)
					{
						if (ADVpobTran.srBRec.uszUpload1Bit == VS_TRUE && ADVpobTran.srBRec.uszUpload2Bit == VS_TRUE && ADVpobTran.srBRec.uszUpload3Bit == VS_TRUE)
						{
							ADVpobTran.inISOTxnCode = _ADVICE_;
						}
						else if (ADVpobTran.srBRec.uszUpload1Bit == VS_FALSE && ADVpobTran.srBRec.uszUpload2Bit == VS_TRUE && ADVpobTran.srBRec.uszUpload3Bit == VS_TRUE)
						{
							if (ADVpobTran.srBRec.inOrgCode == _TIP_)
								ADVpobTran.inISOTxnCode = _TIP_;
							else if (ADVpobTran.srBRec.inOrgCode == _ADJUST_)
								ADVpobTran.inISOTxnCode = _ADJUST_;
							else
								inRetVal = VS_ERROR;
						}
						else if (ADVpobTran.srBRec.uszUpload1Bit == VS_TRUE && ADVpobTran.srBRec.uszUpload2Bit == VS_TRUE && ADVpobTran.srBRec.uszUpload3Bit == VS_FALSE)
						{
							if (ADVpobTran.srBRec.inOrgCode == _SALE_OFFLINE_)
								ADVpobTran.inISOTxnCode = _ADVICE_;
							else
								inRetVal = VS_ERROR;
						}
						else
						{
							ADVpobTran.inISOTxnCode = _VOID_; /* 送取消交易 */
						}
					}
					else
						ADVpobTran.inISOTxnCode = _ADVICE_; /* 表示做結帳前要先送【Advice】所以交易日期、時間不應該被更新 */
				}
				
			}/* 一般信用卡End*/
			
		}

		/* 動態貨幣轉換_DCC_端末機規格-一段式標準版-20121004 
		 * 如果預先授權完成外幣轉台幣advice 需先組reversal，若advice送失敗要reversal */
		if (ADVpobTran.srBRec.uszNCCCDCCRateBit == VS_TRUE && ADVpobTran.srBRec.inOrgCode == _PRE_COMP_)
		{
			/* 因為讀advice不會更新inTransactionCode，所以這裡要給 */
			inCheckTransactionCode = ADVpobTran.inTransactionCode;
			ADVpobTran.inTransactionCode = _PRE_COMP_;
			inRetVal = inNCCC_MFES_ReversalSave(pobTran);
			ADVpobTran.inISOTxnCode = _ADVICE_;
			ADVpobTran.inTransactionCode = inCheckTransactionCode;
			memset(ADVpobTran.srBRec.szRespCode, 0x00, sizeof(ADVpobTran.srBRec.szRespCode));
			
			if (inRetVal == VS_SUCCESS)
				inRetVal = inNCCC_MFES_SendPackRecvUnPack(&ADVpobTran);

			if (inRetVal == VS_SUCCESS)
				if (srISOFunc.inAdviceAnalyse != NULL)
					inRetVal = srISOFunc.inAdviceAnalyse(&ADVpobTran, &uszTCUpload);
			
			/* send Advice成功就把reversal flag off */
			if (!memcmp(ADVpobTran.srBRec.szRespCode, "00", 2) || !memcmp(ADVpobTran.srBRec.szRespCode, "11", 2))
			{
				inSetSendReversalBit("N");
                                inRetVal = inSaveHDPTRec(ADVpobTran.srBRec.inHDTIndex);
			}
				
		}
		else
		{
			if (inRetVal == VS_SUCCESS)
				inRetVal = inNCCC_MFES_SendPackRecvUnPack(&ADVpobTran);

			if (inRetVal == VS_SUCCESS)
				if (srISOFunc.inAdviceAnalyse != NULL)
					inRetVal = srISOFunc.inAdviceAnalyse(&ADVpobTran, &uszTCUpload);
		}

                if (inRetVal != VS_SUCCESS)
                        break;
		
        }

        /* 關閉【.bat】【.bket】【.adv】三個檔 */
	/* 若批次第一筆TCupload會因為還沒存batch檔而失敗，所以TCUpload不做關閉動作 */
	if (ADVpobTran.uszTCUploadBit != VS_TRUE && pobTran->uszFiscConfirmBit != VS_TRUE)
	{
		inBATCH_GlobalAdviceHandleClose_By_Sqlite();
	}

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_AdviceSendRecvPacket() END!");
        
        return (inRetVal);
}

/*
Function        :inNCCC_MFES_Advice_ESC_SendRecvPacket
Date&Time       :2017/3/29 上午 11:05
Describe        :
*/
int inNCCC_MFES_Advice_ESC_SendRecvPacket(TRANSACTION_OBJECT *pobTran, int inAdvCnt)
{
        int				inCnt, inRetVal = VS_ERROR, inISOFuncIndex = -1;
        TRANSACTION_OBJECT		ADVpobTran;
        ISO_TYPE_NCCC_MFES_TABLE 	srISOFunc;

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_Advice_ESC_SendRecvPacket() START!");
        
        /* 加密模式，預設值 = 0。【0 = 不加密，1 = tSAM 加密，2 = 軟體加密】 */
        inISOFuncIndex = 0; /* 不加密 */
        memset((char *) &srISOFunc, 0x00, sizeof(srISOFunc));
        memcpy((char *) &srISOFunc, (char *) &srNCCC_MFES_ISOFunc[inISOFuncIndex], sizeof(srISOFunc));

        memset((char *) &ADVpobTran, 0x00, sizeof(TRANSACTION_OBJECT));
        memcpy((char *) &ADVpobTran, (char *) pobTran, sizeof(TRANSACTION_OBJECT));

	/* 開啟【.bat】【.bkey】【.adv】三個檔 */
	if (inBATCH_Advice_ESC_HandleReadOnly_Flow(pobTran) == VS_ERROR)
		return (VS_ERROR);

        /* 這裡的 for () 不可以在裡面直接call Return () ....  */
        for (inCnt = 0; inCnt < inAdvCnt; inCnt++)
        {
		/* 讀最後一筆，最先進去會在最後一筆 */
		ADVpobTran.srBRec.lnOrgInvNum = _BATCH_LAST_RECORD_;
		/* 這裡要開始逐一將【0220】交易上傳 */
		if (inBATCH_GetAdvice_ESC_DetailRecord_Flow(&ADVpobTran, inCnt) == VS_SUCCESS)
		{
			inRetVal = VS_SUCCESS;
		}
		else
		{
			inRetVal = VS_ERROR;
			break;
		}
		
		ADVpobTran.inISOTxnCode = _SEND_ESC_ADVICE_;
		ADVpobTran.inTransactionCode = ADVpobTran.srBRec.inCode;

		if (inRetVal == VS_SUCCESS)
			inRetVal = inNCCC_MFES_SendPackRecvUnPack(&ADVpobTran);

		/* 不回寫到batch */
		if (inRetVal == VS_ERROR || inRetVal == VS_ISO_PACK_ERR || inRetVal == VS_ISO_UNPACK_ERROR)
			break;
		else
		{
			if (inNCCC_MFES_CheckRespCode(&ADVpobTran) != _TRAN_RESULT_AUTHORIZED_)
			{
				inRetVal = VS_ERROR;
				break;
			}
			
			/* 如果【ADVICE】刪除失敗會鎖機 */
			if (inADVICE_ESC_DeleteRecordFlow(&ADVpobTran, ADVpobTran.srBRec.lnOrgInvNum) != VS_SUCCESS)
			{
				inFunc_EDCLock(AT);
				break;
			}
			
		}

                if (inRetVal != VS_SUCCESS)
                        break;
		
        }

        /* 關閉【.bat】【.bket】【.adv】三個檔 */
	/* 若批次第一筆TCupload會因為還沒存batch檔而失敗，所以TCUpload不做關閉動作 */
	inBATCH_GlobalAdvice_ESC_HandleClose_Flow();

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_Advice_ESC_SendRecvPacket() END!");
        
        return (inRetVal);
}

/*
Function        :inNCCC_MFES_ProcessAdvice
Date&Time       :2016/9/13 下午 5:13
Describe        :
*/
int inNCCC_MFES_ProcessAdvice(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;
        int 	inSendAdviceCnt;

        if ((inSendAdviceCnt = inADVICE_GetTotalCount(pobTran)) == VS_NO_RECORD)
        {
                return (VS_SUCCESS);
        }
        else
        {
		inRetVal = inNCCC_MFES_AdviceSendRecvPacket(pobTran, inSendAdviceCnt);
                if (inRetVal != VS_SUCCESS)
                {
                        return (inRetVal);
                }
        }

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_MFES_ProcessAdvice_ESC
Date&Time       :2017/3/29 上午 10:59
Describe        :處理ESC advice
*/
int inNCCC_MFES_ProcessAdvice_ESC(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;
        int 	inSendAdviceCnt;

        if ((inSendAdviceCnt = inADVICE_ESC_GetTotalCount_Flow(pobTran)) == VS_NO_RECORD)
        {
                return (VS_SUCCESS);
        }
        else
        {
		inRetVal = inNCCC_MFES_Advice_ESC_SendRecvPacket(pobTran, inSendAdviceCnt);
                if (inRetVal != VS_SUCCESS)
                {
                        return (inRetVal);
                }
        }

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_MFES_ProcessOnline
Date&Time       :2016/9/14 上午 9:36
Describe        :
*/
int inNCCC_MFES_ProcessOnline(TRANSACTION_OBJECT *pobTran)
{
        int 	inRetVal;
        char    szTemplate[512 + 1];
	char	szCustomIndicator[3 + 1] = {0};

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_ProcessOnline() START!");

	memset(szCustomIndicator, 0x00, sizeof(szCustomIndicator));
	inGetCustomIndicator(szCustomIndicator);

        /* 開始組交易封包，送、收、組、解 */
        pobTran->inISOTxnCode = pobTran->inTransactionCode; /* 以 srEventMenuItem.inCode Index */
        inRetVal = inNCCC_MFES_SendPackRecvUnPack(pobTran);

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
			inNCCC_MFES_ProcessReversal(pobTran);
		}
		else if (!memcmp(szCustomIndicator, _CUSTOMER_INDICATOR_099_SINYA_, _CUSTOMER_INDICATOR_SIZE_))
		{
			if (pobTran->inISOTxnCode != _SETTLE_)
			{
				inNCCC_Func_Process_Reversal_Inform(pobTran);
				
				return (inRetVal);
			}
		}
		/* 026印兩聯，099只印一聯 */
		else if (!memcmp(szCustomIndicator, _CUSTOMER_INDICATOR_026_TAKA_, _CUSTOMER_INDICATOR_SIZE_) ||
			 !memcmp(szCustomIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
		{
			if (pobTran->inISOTxnCode != _SETTLE_)
			{
				inNCCC_Func_Process_Reversal_Inform(pobTran);
				
				return (inRetVal);
			}
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
			pobTran->inErrorMsg = _ERROR_CODE_V3_ISO_UNPACK_;
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
                /* 決定後續交易的流程先看【Field_39】再看【Field_38】 */
                pobTran->inTransactionResult = inNCCC_MFES_CheckRespCode(pobTran); /* 【Field_39】 */
                
                if (ginDebug == VS_TRUE)
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "pobTran->inTransactionResult = %d", pobTran->inTransactionResult);
                        inLogPrintf(AT, szTemplate);
                }
                
                if (pobTran->inTransactionResult == _TRAN_RESULT_AUTHORIZED_)
                {
                        if (inNCCC_MFES_CheckAuthCode(pobTran) != VS_SUCCESS)
			{
				/* 拒絕交易 */
				pobTran->inErrorMsg = _ERROR_CODE_V3_AUTH_CODE_NOT_VALID_;
				pobTran->inTransactionResult = _TRAN_RESULT_CANCELLED_;
				
                                return (VS_ISO_UNPACK_ERROR); /* 主機沒有回覆授權碼 */
			}
                }
                else if (pobTran->inTransactionResult == _TRAN_RESULT_UNPACK_ERR_)
                        return (VS_ISO_UNPACK_ERROR); /* 主機沒有回回覆碼 */
        }

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_ProcessOnline() END!");

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_MFES_ProcessOffline
Date&Time       :2016/9/14 上午 10:02
Describe        :
*/
int inNCCC_MFES_ProcessOffline(TRANSACTION_OBJECT *pobTran)
{
        int 	inRetVal = VS_SUCCESS;

	/* 離線取消(原交易未上傳直接取消) */
	if (pobTran->srBRec.inCode == _VOID_ && pobTran->srBRec.uszOfflineBit == VS_TRUE && pobTran->srBRec.uszUpload1Bit == VS_TRUE)
	{
		if (inADVICE_DeleteRecordFlow(pobTran, pobTran->srBRec.lnOrgInvNum) != VS_SUCCESS)
		{
			/* 鎖機 */
			inFunc_EDCLock(AT);
			
			inRetVal = VS_ERROR;
		}
		
	}
	else
	{
		/* Save Advice */
		inRetVal = inADVICE_SaveAppend(pobTran, pobTran->srBRec.lnOrgInvNum);
		
		if (inRetVal != VS_ERROR)
		{
			
		}
		else
		{
			pobTran->inTransactionResult = _TRAN_RESULT_AUTHORIZED_;

			memset(pobTran->srBRec.szRespCode, 0x00, sizeof(pobTran->srBRec.szRespCode));
			strcpy(pobTran->srBRec.szRespCode, "00");
			pobTran->srBRec.szRespCode[2] = 0x00;
		}
		
	}

        return (inRetVal);
}

/*
Function        :inNCCC_MFES_AnalysePacket
Date&Time       :2016/9/14 上午 10:03
Describe        :
*/
int inNCCC_MFES_AnalysePacket(TRANSACTION_OBJECT *pobTran)
{
        int				inRetVal = VS_SUCCESS, inISOFuncIndex = -1;
        ISO_TYPE_NCCC_MFES_TABLE 	srISOFunc;

        if (pobTran->srBRec.uszOfflineBit == VS_FALSE)
        {
		/* 加密模式，預設值 = 0。【0 = 不加密，1 = tSAM 加密，2 = 軟體加密】 */
		inISOFuncIndex = 0; /* 不加密 */
		memset((char *) &srISOFunc, 0x00, sizeof(srISOFunc));
		memcpy((char *) &srISOFunc, (char *) &srNCCC_MFES_ISOFunc[inISOFuncIndex], sizeof(srISOFunc));

		if (srISOFunc.inOnAnalyse != NULL)
			inRetVal = srISOFunc.inOnAnalyse(pobTran);
		else
			inRetVal = VS_ERROR;
        }

        return (inRetVal);
}

/*
Function        :inNCCC_MFES_ReversalSendRecvPacket
Date&Time       :2016/9/13 下午 4:52
Describe        :
*/
int inNCCC_MFES_ReversalSendRecvPacket(TRANSACTION_OBJECT *pobTran)
{
        int 		inRetVal, inSendCnt;
        long 		lnREVCnt;
	char		szDialBackupEnable[2 + 1];
	char		szCommMode[2 + 1];
        unsigned char 	uszFileName[20 + 1];
        unsigned char 	uszSendPacket[_NCCC_MFES_ISO_SEND_ + 1], uszRecvPacket[_NCCC_MFES_ISO_RECV_ + 1];
        unsigned long 	ulREVFHandle;
	
        memset(uszSendPacket, 0x00, sizeof(uszSendPacket));
        memset(uszRecvPacket, 0x00, sizeof(uszRecvPacket));

        /* 如果有DialBackup */
	memset(szDialBackupEnable, 0x00, sizeof(szDialBackupEnable));
	inGetDialBackupEnable(szDialBackupEnable);
	memset(szCommMode, 0x00, sizeof(szCommMode));
	inGetCommMode(szCommMode);
	
	if (memcmp(szDialBackupEnable, "Y", strlen("Y")) == 0 && memcmp(szCommMode, _COMM_MODEM_MODE_, strlen(_COMM_MODEM_MODE_)) == 0)
	{
		memset(uszFileName, 0x00, sizeof(uszFileName));
		if (inFunc_ComposeFileName(pobTran, (char*)uszFileName, _REVERSAL_DIALBACKUP_FILE_EXTENSION_, 6) != VS_SUCCESS)
			return (VS_ERROR);
	}
	else
	{
		memset(uszFileName, 0x00, sizeof(uszFileName));
		if (inFunc_ComposeFileName(pobTran, (char*)uszFileName, _REVERSAL_FILE_EXTENSION_, 6) != VS_SUCCESS)
			return (VS_ERROR);
	}

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
                inLogPrintf(AT, "inNCCC_MFES_CommSendRecvToHost() Before");

        if ((inRetVal = inNCCC_MFES_CommSendRecvToHost(pobTran, uszSendPacket, inSendCnt, uszRecvPacket)) != VS_SUCCESS)
        {
		pobTran->inTransactionResult = _TRAN_RESULT_COMM_ERROR_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_COMM_;
		
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inNCCC_MFES_CommSendRecvToHost() Error");

                return (VS_COMM_ERROR);
        }

        /* 解 ISO 電文 */
	pobTran->inISOTxnCode = _REVERSAL_;
        if ((inRetVal = inNCCC_MFES_UnPackISO(pobTran, uszSendPacket, uszRecvPacket)) != VS_SUCCESS)
        {
		pobTran->inTransactionResult = _TRAN_RESULT_UNPACK_ERR_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_ISO_UNPACK_;
		
                return (VS_ISO_UNPACK_ERROR);
        }

        /* memcmp兩字串相同回傳0 */
        if (memcmp(pobTran->srBRec.szRespCode, "00", 2) != 0)
        {
		pobTran->inTransactionResult = _TRAN_RESULT_CANCELLED_;
		
                return (VS_ERROR);
        }
        else
        {
                inSetSendReversalBit("N");
                if (inSaveHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
                {
                        return (VS_ERROR);
                }
                
		/* 如果有DialBackup，要砍兩個Reversal */
		memset(szDialBackupEnable, 0x00, sizeof(szDialBackupEnable));
		inGetDialBackupEnable(szDialBackupEnable);
		memset(szCommMode, 0x00, sizeof(szCommMode));
		inGetCommMode(szCommMode);

		if (memcmp(szDialBackupEnable, "Y", strlen("Y")) == 0 && memcmp(szCommMode, _COMM_MODEM_MODE_, strlen(_COMM_MODEM_MODE_)) == 0)
		{
			/* 刪除撥接reersal */
			memset(uszFileName, 0x00, sizeof(uszFileName));
			if (inFunc_ComposeFileName(pobTran, (char*)uszFileName, _REVERSAL_DIALBACKUP_FILE_EXTENSION_, 6) != VS_SUCCESS)
				return (VS_ERROR);
			
			inFILE_Delete(uszFileName);
			
			/* 刪除非撥接Reversal */
			memset(uszFileName, 0x00, sizeof(uszFileName));
			if (inFunc_ComposeFileName(pobTran, (char*)uszFileName, _REVERSAL_FILE_EXTENSION_, 6) != VS_SUCCESS)
				return (VS_ERROR);
			
			inFILE_Delete(uszFileName);
		}
		else
		{
			memset(uszFileName, 0x00, sizeof(uszFileName));
			if (inFunc_ComposeFileName(pobTran, (char*)uszFileName, _REVERSAL_FILE_EXTENSION_, 6) != VS_SUCCESS)
				return (VS_ERROR);
			
			inFILE_Delete(uszFileName);
		}
		
        }

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_MFES_ReversalSave_Flow
Date&Time       :2017/3/30 上午 11:06
Describe        :
*/
int inNCCC_MFES_ReversalSave_Flow(TRANSACTION_OBJECT *pobTran)
{
	int 	inRetVal;
	char	szDialBackupEnable[2 + 1];
	
	inRetVal = inNCCC_MFES_ReversalSave(pobTran);
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	memset(szDialBackupEnable, 0x00, sizeof(szDialBackupEnable));
	inGetDialBackupEnable(szDialBackupEnable);

	if (memcmp(szDialBackupEnable, "Y", strlen("Y")) == 0)
	{

		inRetVal = inNCCC_MFES_ReversalSave_For_DialBeckUp(pobTran);
		if (inRetVal != VS_SUCCESS)
		{
			return (VS_ERROR);
		}

	}
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_MFES_ReversalSave
Date&Time       :2016/9/13 下午 5:06
Describe        :
*/
int inNCCC_MFES_ReversalSave(TRANSACTION_OBJECT *pobTran)
{
        int 		inRetVal, inPacketCnt;
        unsigned char 	uszReversalPacket[_NCCC_MFES_ISO_SEND_ + 1];
        unsigned char 	uszFileName[20 + 1];
        unsigned long 	srFHandle;


        memset(uszReversalPacket, 0x00, sizeof(uszReversalPacket));
        pobTran->inISOTxnCode = _REVERSAL_; /* 沖銷 */

        /* 組_REVERSAL_ 封包 */
        inPacketCnt = inNCCC_MFES_PackISO(pobTran, &uszReversalPacket[0], pobTran->inISOTxnCode);
        if (inPacketCnt <= 0)
	{
		pobTran->inTransactionResult = _TRAN_RESULT_PACK_ERR_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_ISO_PACK_;
		
                return (VS_ERROR);
	}

        memset(uszFileName, 0x00, sizeof(uszFileName));
	if (inFunc_ComposeFileName(pobTran, (char*)uszFileName, _REVERSAL_FILE_EXTENSION_, 6) != VS_SUCCESS)
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
        inSetSendReversalBit("Y");
        if (inSaveHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
        {
                inFILE_Close(&srFHandle);
                inFILE_Delete(uszFileName);

                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_MFES_ReversalSave_For_DialBeckUp
Date&Time       :2017/3/30 上午 10:24
Describe        :
*/
int inNCCC_MFES_ReversalSave_For_DialBeckUp(TRANSACTION_OBJECT *pobTran)
{
        int 		inRetVal, inPacketCnt;
        unsigned char 	uszReversalPacket[_NCCC_MFES_ISO_SEND_ + 1];
        unsigned char 	uszFileName[20 + 1];
        unsigned long 	srFHandle;


        memset(uszReversalPacket, 0x00, sizeof(uszReversalPacket));
        pobTran->inISOTxnCode = _REVERSAL_; /* 沖銷 */

        /* 組_REVERSAL_ 封包 */
        inPacketCnt = inNCCC_MFES_PackISO(pobTran, &uszReversalPacket[0], pobTran->inISOTxnCode);
        if (inPacketCnt <= 0)
	{
		pobTran->inTransactionResult = _TRAN_RESULT_PACK_ERR_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_ISO_PACK_;
		
                return (VS_ERROR);
	}

        memset(uszFileName, 0x00, sizeof(uszFileName));
	if (inFunc_ComposeFileName(pobTran, (char*)uszFileName, _REVERSAL_DIALBACKUP_FILE_EXTENSION_, 6) != VS_SUCCESS)
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
        inSetSendReversalBit("Y");
        if (inSaveHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
        {
                inFILE_Close(&srFHandle);
                inFILE_Delete(uszFileName);

                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}


int inNCCC_MFES_CommSendRecvToHost(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, int inSendLen, unsigned char *uszRecvPacket)
{
        int		inRetVal;
        int		inSendTimeout = 10;
        int		inReceiveTimeout = 10;
        int		inReceiveSize = _COMM_RECEIVE_MAX_LENGTH_;		/* 配合Ingenico */
	char		szTimeOut[2 + 1] = {0};
        char		szHostResponseTimeOut[2 + 1] = {0};
	unsigned char	uszDispBit = VS_TRUE;

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_CommSendRecvToHost() START!");
        
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
		vdNCCC_MFES_ISO_FormatDebug_DISP(pobTran, &uszSendPacket[0], inSendLen);
	}
	/* 列印ISO Debug */
        if (ginISODebug == VS_TRUE)
	{
                vdNCCC_MFES_ISO_FormatDebug_PRINT(pobTran, &uszSendPacket[0], inSendLen);
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
	
	/* 這裡是用CPT.DAT的東西，所以一定要用 */
	memset(szHostResponseTimeOut, 0x00, sizeof(szHostResponseTimeOut));
        inRetVal = inGetHostResponseTimeOut(szHostResponseTimeOut);

        if (inRetVal != VS_SUCCESS)
                return (VS_ERROR);

        inReceiveTimeout = atoi(szHostResponseTimeOut);

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
		vdNCCC_MFES_ISO_FormatDebug_DISP(pobTran, &uszRecvPacket[0], inRetVal);
	}
        /* 列印ISO Debug */
        if (ginISODebug == VS_TRUE)
                vdNCCC_MFES_ISO_FormatDebug_PRINT(pobTran, &uszRecvPacket[0], inRetVal);

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_CommSendRecvToHost() END!");
        
        return (VS_SUCCESS);
}

int inNCCC_MFES_SendPackRecvUnPack(TRANSACTION_OBJECT *pobTran)
{
        int 		inSendCnt = 0, inRecvCnt = 0, inRetVal;
        unsigned char 	uszSendPacket[_NCCC_MFES_ISO_SEND_ + 1], uszRecvPacket[_NCCC_MFES_ISO_RECV_ + 1];

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_SendPackRecvUnPack() START!");

        memset(uszSendPacket, 0x00, sizeof(uszSendPacket));
        memset(uszRecvPacket, 0x00, sizeof(uszRecvPacket));

        /* 組 ISO 電文 */
        if ((inSendCnt = inNCCC_MFES_PackISO(pobTran, uszSendPacket, pobTran->inISOTxnCode)) <= 0)
        {
		pobTran->inTransactionResult = _TRAN_RESULT_PACK_ERR_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_ISO_PACK_;
		
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inNCCC_MFES_PackISO() Error!");

                return (VS_ISO_PACK_ERR); /* 組交易電文錯誤，不用組_REVERSAL_ */
        }
        
        /* 傳送及接收 ISO 電文 */
        if ((inRecvCnt = inNCCC_MFES_CommSendRecvToHost(pobTran, uszSendPacket, inSendCnt, uszRecvPacket)) != VS_SUCCESS)
        {
		pobTran->inTransactionResult = _TRAN_RESULT_COMM_ERROR_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_COMM_;
		
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inNCCC_MFES_CommSendRecvToHost() Error");

                if (pobTran->inISOTxnCode != _ADVICE_)
                        memset(pobTran->srBRec.szRespCode, 0x00, sizeof(pobTran->srBRec.szRespCode));

                return (VS_COMM_ERROR);
        }

        /* 解 ISO 電文 */
        inRetVal = inNCCC_MFES_UnPackISO(pobTran, uszSendPacket, uszRecvPacket);
        
        if (inRetVal != VS_SUCCESS)
        {
		if (inRetVal == VS_WRITE_KEY_ERROR)
		{
			pobTran->inTransactionResult = _TRAN_RESULT_AUTHORIZED_;
			pobTran->inErrorMsg = _ERROR_CODE_V3_WRITE_KEY_INIT_FAIL_;
			inRetVal = VS_WRITE_KEY_ERROR;
			
		}
		else
		{
			pobTran->inTransactionResult = _TRAN_RESULT_UNPACK_ERR_;
			pobTran->inErrorMsg = _ERROR_CODE_V3_ISO_UNPACK_;

			inRetVal = VS_ISO_UNPACK_ERROR;
		}
        }
	
        return (inRetVal);
}

/*
Function        :inNCCC_MFES_CheckRespCode
Date&Time       :2016/9/14 上午 9:48
Describe        :
*/
int inNCCC_MFES_CheckRespCode(TRANSACTION_OBJECT *pobTran)
{
	int 	inRetVal = _TRAN_RESULT_COMM_ERROR_;
	char	szCustomerIndicator[3 + 1] = {0};
        
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	/* 如果是DFS卡回覆授權碼空白時，EDC轉為05拒絕交易並送Reversal */
	if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)) == 0	&&
	    memcmp(pobTran->srBRec.szAuthCode, "      ", strlen("      ") == 0))
	{
		/* 查詢卡號，不適用此限制 */
		if (pobTran->inISOTxnCode == _INQUIRY_ISSUER_BANK_)
		{
			
		}
		else
		{
			strcpy(pobTran->srBRec.szRespCode, "05");
			pobTran->uszDFSNoAuthCodeBit = VS_TRUE;
		}
	}

	if (!memcmp(pobTran->srBRec.szRespCode, "00", 2))
	{
		inRetVal = _TRAN_RESULT_AUTHORIZED_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_NONE_;
		
		/* 未滿三次，授權成功要重置次數 */
		if (ginMacError < 3)
		{
			ginMacError = 0;
		}
	}
	else if (!memcmp(pobTran->srBRec.szRespCode, "01", 2) || !memcmp(pobTran->srBRec.szRespCode, "02", 2))
	{
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_)	||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)		||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
		{
			/* 維持原回應碼 */
		}
		else
		{
			if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
			{
				strcpy(pobTran->srBRec.szRespCode, "05");
			}
			/* 分期紅利CallBank回05 */
			else if (pobTran->srBRec.uszInstallmentBit == VS_TRUE	||
				 pobTran->srBRec.uszRedeemBit == VS_TRUE)
			{
				strcpy(pobTran->srBRec.szRespCode, "05");
			}
		}
		
		if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
		{
			inRetVal = _TRAN_RESULT_CANCELLED_;
		}
		/* 	DISCOVER交易不支援Call bank交易，提示「XX 拒絕交易」。 */
		/* 要排除TWIN卡的狀況 */
		else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)) == 0 &&
			 pobTran->srBRec.uszUCARDTransBit != VS_TRUE)
		{
			inRetVal = _TRAN_RESULT_CANCELLED_;
		}
		/* 分期紅利CallBank回05 */
		else if (pobTran->srBRec.uszInstallmentBit == VS_TRUE	||
			 pobTran->srBRec.uszRedeemBit == VS_TRUE)
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
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)	||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
		{
			pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_CALLBANK_;
		}
	}
	else if (!memcmp(pobTran->srBRec.szRespCode, "  ", 2))
	{
		inRetVal = _TRAN_RESULT_UNPACK_ERR_; /* 不是定義的 Response Code */
		pobTran->inErrorMsg = _ERROR_CODE_V3_ISO_UNPACK_;
	}
	else if (pobTran->srBRec.szRespCode[0] == 0x00)
	{
		inRetVal = _TRAN_RESULT_COMM_ERROR_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_COMM_;
	}
	else if (!memcmp(&pobTran->srBRec.szRespCode[0], "A0", 2) ||
		 !memcmp(&pobTran->srBRec.szRespCode[0], "A1", 2))
	{
		ginMacError++;
		inRetVal = _TRAN_RESULT_CANCELLED_;
	}
	else if (!memcmp(&pobTran->srBRec.szRespCode[0], "0X", 2) ||
		 !memcmp(&pobTran->srBRec.szRespCode[0], "C8", 2) ||
		 !memcmp(&pobTran->srBRec.szRespCode[0], "C9", 2) ||
	         !memcmp(&pobTran->srBRec.szRespCode[0], "XA", 2) ||
	         !memcmp(&pobTran->srBRec.szRespCode[0], "XB", 2) ||
	         !memcmp(&pobTran->srBRec.szRespCode[0], "XC", 2) ||
	         !memcmp(&pobTran->srBRec.szRespCode[0], "XI", 2) ||
	         !memcmp(&pobTran->srBRec.szRespCode[0], "XN", 2) ||
	         !memcmp(&pobTran->srBRec.szRespCode[0], "XV", 2) ||
	         !memcmp(&pobTran->srBRec.szRespCode[0], "XX", 2) ||
	         !memcmp(&pobTran->srBRec.szRespCode[0], "XZ", 2) ||
	         !memcmp(&pobTran->srBRec.szRespCode[0], "L1", 2) ||
	         !memcmp(&pobTran->srBRec.szRespCode[0], "L3", 2) ||
	         !memcmp(&pobTran->srBRec.szRespCode[0], "L4", 2) ||
	         !memcmp(&pobTran->srBRec.szRespCode[0], "L5", 2) ||
	         !memcmp(&pobTran->srBRec.szRespCode[0], "L6", 2) ||
	         !memcmp(&pobTran->srBRec.szRespCode[0], "L7", 2) ||
	         !memcmp(&pobTran->srBRec.szRespCode[0], "L8", 2) ||
	         !memcmp(&pobTran->srBRec.szRespCode[0], "L9", 2) ||
	         !memcmp(&pobTran->srBRec.szRespCode[0], "ID", 2))
	{
		inRetVal = _TRAN_RESULT_CANCELLED_;
	}
        else
        {
                if ((pobTran->srBRec.szRespCode[0] >= '0' && pobTran->srBRec.szRespCode[0] <= '9') && (pobTran->srBRec.szRespCode[1] >= '0' && pobTran->srBRec.szRespCode[1] <= '9'))
                {
                        /* 要增加結帳的判斷 */
                        if (pobTran->inISOTxnCode == _SETTLE_ && !memcmp(&pobTran->srBRec.szRespCode[0], "95", 2))
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
Function        :inNCCC_MFES_CheckAuthCode
Date&Time       :2016/9/14 上午 9:48
Describe        :
*/
int inNCCC_MFES_CheckAuthCode(TRANSACTION_OBJECT *pobTran)
{
        int 	inRetVal = VS_SUCCESS;
	char	szDebugMsg[100 + 1];
	
        switch (pobTran->inISOTxnCode)
        {
                case _SETTLE_:
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
                        if ((!memcmp(&pobTran->srBRec.szAuthCode[0], "000000", 6) || !memcmp(&pobTran->srBRec.szAuthCode[0], "      ", 6)) && pobTran->srBRec.uszCUPTransBit != VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "inNCCC_MFES_CheckAuthCode Error, %s", pobTran->srBRec.szAuthCode);
					inLogPrintf(AT, szDebugMsg);
				}
				
                                inRetVal = VS_ERROR;
			}

                        break;
        }

        return (inRetVal);
}

/*
Function        :inNCCC_MFES_OnlineAnalyseMagneticManual
Date&Time       :2016/9/14 上午 9:49
Describe        :
*/
int inNCCC_MFES_OnlineAnalyseMagneticManual(TRANSACTION_OBJECT *pobTran)
{
        int		inRetVal = VS_SUCCESS;
	char		szTransFunEnable[20 + 1];
	RTC_NEXSYS	srRTC;				/* Date & Time */

        if (pobTran->inISOTxnCode == _SETTLE_)
        {
		/* Settle時非95、非00的話 */
                if (memcmp(pobTran->srBRec.szRespCode, "95", 2) && memcmp(pobTran->srBRec.szRespCode, "00", 2))
                        inRetVal = VS_ERROR;
                else
                {
			/* 需要執行BatchUpload時 */
                        if (pobTran->inTransactionResult == _TRAN_RESULT_SETTLE_UPLOAD_BATCH_ && !memcmp(pobTran->srBRec.szRespCode, "95", 2))
                                inRetVal = inNCCC_MFES_ProcessSettleBatchUpload(pobTran);
			
                }

		/* 結帳成功 */
                if (inRetVal == VS_SUCCESS)
                {
                        if (inNCCC_MFES_SetReversalCnt(pobTran, _RESET_) == VS_ERROR)
                        {
                                return (VS_ERROR);
                        }
			
			/* 結帳成功 把請先結帳的bit關掉 */
			inNCCC_MFES_SetMustSettleBit(pobTran, "N");
                }
		else
		{
			/* BatchUpload失敗 */
			/* 在inNCCC_MFES_ProcessSettleBatchUpload裡面顯示主機回的錯誤訊息 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_SETTLE_NOT_SUCCESS_;
			inRetVal = VS_ERROR;
		}

        }
        else
        {
                if (pobTran->inTransactionResult == _TRAN_RESULT_AUTHORIZED_)
                {
                        /* 要更新端末機的日期及時間 */
                        if (inNCCC_MFES_SyncHostTerminalDateTime(pobTran) != VS_SUCCESS)
                                return (VS_ERROR);

                        if (pobTran->srBRec.uszOfflineBit == VS_FALSE)
                        {
                                inSetSendReversalBit("N");
                                if (inSaveHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
                                {
                                        return (VS_ERROR);
                                }

				/* 因為是【Online】交易在這裡送【Advice】 */
                                inRetVal = inNCCC_MFES_AdviceSendRecvPacket(pobTran, 1);
				if (inRetVal == VS_NO_RECORD)
				{
					inNCCC_MFES_Advice_ESC_SendRecvPacket(pobTran, 1);
				}
				
				/* 不管advice最後如何，只要原交易授權就回傳成功 */
				inRetVal = VS_SUCCESS;
                        }
                }
                else if (pobTran->inTransactionResult == _TRAN_RESULT_REFERRAL_ && pobTran->inTransactionCode == _SALE_)
                {
                        /* 要更新端末機的日期及時間 */
                        if (inNCCC_MFES_SyncHostTerminalDateTime(pobTran) != VS_SUCCESS)
                                return (VS_ERROR);

                        if (pobTran->srBRec.uszOfflineBit == VS_FALSE)
                        {
                                inSetSendReversalBit("N");
                                if (inSaveHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
                                {
                                        return (VS_ERROR);
                                }
                        }

                        if ((inRetVal = inNCCC_MFES_ProcessReferral(pobTran)) == VS_SUCCESS)
                        {
				/* 修改Call Bank交易補登Online問題 */
				memset(szTransFunEnable, 0x00, sizeof(szTransFunEnable));
				inGetTransFunc(szTransFunEnable);

				/* 交易補登Online */
				/* 當EMS開啟授權碼補登功能，交易電文預設go online by Russell 2018/12/7 下午 6:10 */
				if (1)
				{
					pobTran->inTransactionResult = _TRAN_RESULT_AUTHORIZED_;
					pobTran->srBRec.uszReferralBit = VS_TRUE;
					pobTran->srBRec.uszForceOnlineBit = VS_TRUE;
					pobTran->srBRec.uszOfflineBit = VS_FALSE;
					pobTran->srBRec.uszUpload1Bit = VS_FALSE;
					pobTran->srBRec.uszUpload2Bit = VS_FALSE;
					pobTran->srBRec.uszUpload3Bit = VS_FALSE;

					/* 取得EDC時間日期 */
					memset(&srRTC, 0x00, sizeof(RTC_NEXSYS));
					if (inFunc_GetSystemDateAndTime(&srRTC) != VS_SUCCESS)
					{
						return (VS_ERROR);
					}
	
					/* 更新pobTran內日期時間 */
					if (inFunc_Sync_BRec_Date_Time(pobTran, &srRTC) != VS_SUCCESS)
					{
						return (VS_ERROR);
					}

					/* 送CallBank的Online交易補登 要加一 */
                                        inNCCC_MFES_GetSTAN(pobTran);

					/* inFunc_REFERRAL_GetManualApproval會斷線，因此要重新連一次 */
					if (inFLOW_RunFunction(pobTran, _COMM_START_) != VS_SUCCESS)
					{
						/* 連線失敗 */
						return (VS_ERROR);
					}
					
					/* 連線成功*/
					/* 第一行顯示 <交易補登> */
					inDISP_ClearAll();
					inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
					inDISP_PutGraphic(_MENU_CALL_BANK_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 授權碼補登 */

					pobTran->srBRec.inCode = _SALE_OFFLINE_;	/* 交易補登 */
					pobTran->inTransactionCode = _SALE_OFFLINE_;	/* 交易補登 */
					pobTran->inISOTxnCode = _SALE_OFFLINE_;
					pobTran->uszReversalBit = VS_TRUE;

					/* 步驟 1.先組Reversal */
					if ((inRetVal = inNCCC_MFES_ProcessReversal(pobTran)) != VS_SUCCESS)
					{
						return (inRetVal);
					}
					
					/* 步驟 2.送Online 交易 MTI 0200 */
					inRetVal = inNCCC_MFES_ProcessOnline(pobTran);
					inNCCC_MFES_SetSTAN(pobTran); /* 成功或失敗 System_Trace_Number 都要加一 */
					
					if (inRetVal != VS_SUCCESS)
					{
						return (inRetVal); /* Return 【VS_ERROR】【VS_SUCCESS】 */
					}
					
					/* 步驟 3.分析資料 */
					if (inNCCC_MFES_AnalysePacket(pobTran) == VS_ERROR)
					{
						return (VS_ERROR);
					}
					
					/* 避免撥接太久，收送完就斷線 */
					inCOMM_End(pobTran);
					
					return (VS_SUCCESS);
				}
				else
				{
					/* 要變成一筆【ADVICE】交易 */
					pobTran->inTransactionResult = _TRAN_RESULT_AUTHORIZED_;
					pobTran->srBRec.uszReferralBit = VS_TRUE;
					pobTran->srBRec.uszForceOnlineBit = VS_TRUE;
					pobTran->srBRec.uszOfflineBit = VS_TRUE;
					pobTran->srBRec.uszUpload1Bit = VS_TRUE;
					pobTran->srBRec.uszUpload2Bit = VS_FALSE;
					pobTran->srBRec.uszUpload3Bit = VS_FALSE;

					/* 更新交易時間 */
					/* Save Advice */
					if (inADVICE_SaveAppend(pobTran, pobTran->srBRec.lnOrgInvNum) != VS_SUCCESS)
					{
						inRetVal = VS_ERROR;
					}

					pobTran->srBRec.inCode = _SALE_OFFLINE_; /* 交易補登 */
					pobTran->srBRec.inOrgCode = _SALE_OFFLINE_;
					pobTran->inTransactionCode = _SALE_OFFLINE_; /* 交易補登 */
				}
				
                        }
                        else
                        {
                                inRetVal = VS_ERROR;
                        }
                }
                else if (pobTran->inTransactionResult == _TRAN_RESULT_CANCELLED_)
                {
                        if (pobTran->srBRec.uszOfflineBit == VS_FALSE)
                        {
				/* 如果是DFS卡回覆授權碼空白時，EDC轉為05拒絕交易並送Reversal */
				if (pobTran->uszDFSNoAuthCodeBit == VS_TRUE)
				{
					/* 不把送reversal的bit關掉 */
				}
				else
				{
					inSetSendReversalBit("N");
					if (inSaveHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
					{
						return (VS_ERROR);
					}
				}
                        }

			inNCCC_MFES_DispHostResponseCode(pobTran);		/* 顯示主機回的錯誤訊息 */
			
			inRetVal = VS_ERROR;
                }
                else
                {
			inNCCC_MFES_DispHostResponseCode(pobTran);		/* 顯示主機回的錯誤訊息 */
			
                        inRetVal = VS_ERROR;
                }
        }

        return (inRetVal);
}

/*
Function        :inNCCC_MFES_OnlineAnalyseEMV
Date&Time       :2016/11/17 下午 4:57
Describe        :分析結果
*/
int inNCCC_MFES_OnlineAnalyseEMV(TRANSACTION_OBJECT *pobTran)
{
        char            szCustomerIndicator[3 + 1] = {0};
	int	inRetVal = VS_SUCCESS;
	
	/* 初始化主機回傳結果 */
	EMVGlobConfig.uszAction = 0;
	
	/* 若通訊失敗則要產生Y3，須在這裡填入bAction回傳給Kernal */
	/*
	Mail 2013/09/27 RE: V5S開發EMV流程相關問題
	5.在OnTxnOnline流程中，若是晶片通訊失敗我要如何回傳給Kernal產生Y3?(因沒有Response Code無法給pAuthorizationCode)
	Ans: 若要Y3，則在EMV_ONLINE_RESPONSE_DATA結構中的bAction 變數，填上 d_ONLINE_ACTION_UNABLE (03h)。
	*/
	if (pobTran->inTransactionResult == _TRAN_RESULT_COMM_ERROR_ && (pobTran->inTransactionCode == _SALE_ || pobTran->inTransactionCode == _CUP_SALE_))
	{
                memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
                inGetCustomIndicator(szCustomerIndicator);

                // Tusin_071
                if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_071_FORCE_ONLINE_, _CUSTOMER_INDICATOR_SIZE_))
                {
                        if (pobTran->srBRec.uszOfflineBit == VS_FALSE)
                        {
                                inSetSendReversalBit("N");
                                inSaveHDPTRec(pobTran->srBRec.inHDTIndex);
                        }
                        /* 將主機下來的回覆資料依照Kernel的要求塞入指定的參數中 */
                        EMVGlobConfig.uszAction = d_ONLINE_ACTION_UNABLE ;
			inRetVal = VS_ERROR;
                }
                else
                {
                        if (pobTran->srBRec.uszOfflineBit == VS_FALSE)
                        {
                                inSetSendReversalBit("N");
                                inSaveHDPTRec(pobTran->srBRec.inHDTIndex);
                        }
                        /* 將主機下來的回覆資料依照Kernel的要求塞入指定的參數中 */
                        EMVGlobConfig.uszAction = d_ONLINE_ACTION_UNABLE ;
			inRetVal = VS_ERROR;
                }
	}
	else if (pobTran->inTransactionResult == _TRAN_RESULT_REFERRAL_ && (pobTran->inTransactionCode == _SALE_ || pobTran->inTransactionCode == _CUP_SALE_))
	{
		/* 要更新端末機的日期及時間 */
		if (inFunc_SetEDCDateTime(pobTran->srBRec.szDate, pobTran->srBRec.szTime) != VS_SUCCESS)
			return (VS_ERROR);

		/* 開始執行【Call Bank】流程 */
		if ((inRetVal = inFunc_REFERRAL_GetManualApproval(pobTran)) == VS_SUCCESS)
		{
			/* 主機回傳結果 */
			EMVGlobConfig.uszAction = d_ONLINE_ACTION_ISSUER_REFERRAL_APPR;
		}
		/* Call Bank 失敗，拿不到授權碼 */
		else
		{
			/* 主機回傳結果 */
			EMVGlobConfig.uszAction = d_ONLINE_ACTION_ISSUER_REFERRAL_DENY;
		}

		/* Second Gen AC會在Online後執行，執行完Second Gen AC的流程在OnTxnResult中執行 */
	}
	else if (pobTran->inTransactionCode == _PRE_AUTH_ || pobTran->inTransactionCode == _CUP_PRE_AUTH_)
	{
		/* 2011-05-27 AM 09:58:33 修正晶片做預先授權會送TC-upload */
		if (pobTran->inTransactionResult == _TRAN_RESULT_AUTHORIZED_)
		{
			/* 主機回傳結果 */
			EMVGlobConfig.uszAction = d_ONLINE_ACTION_APPROVAL;

			/* 要更新端末機的日期及時間 */
			if (inFunc_SetEDCDateTime(pobTran->srBRec.szDate, pobTran->srBRec.szTime) != VS_SUCCESS)
				 return (VS_ERROR);

			inSetSendReversalBit("N");
			inSaveHDPTRec(pobTran->srBRec.inHDTIndex);

		}
		/* 主機拒絕交易 */
		else
		{	
			/* 主機回傳結果 */
			EMVGlobConfig.uszAction = d_ONLINE_ACTION_DECLINE;

			if (pobTran->inTransactionResult == _TRAN_RESULT_CANCELLED_)
			{
				/* Offline被主機拒絕交易的話，就不送Reversal */
				if (pobTran->srBRec.uszOfflineBit == VS_FALSE)
				{
					inSetSendReversalBit("N");
					inSaveHDPTRec(pobTran->srBRec.inHDTIndex);
				}
				inNCCC_MFES_DispHostResponseCode(pobTran);	/* 顯示主機回的錯誤訊息 */

				return (VS_ERROR);
			}
			else
			{
				inNCCC_MFES_DispHostResponseCode(pobTran);	/* 顯示主機回的錯誤訊息 */
				
				return (VS_ERROR);
			}

		}

	}
	/* 主機授權 */
	else if (pobTran->inTransactionResult == _TRAN_RESULT_AUTHORIZED_)
	{
		/* 主機回傳結果 */
		EMVGlobConfig.uszAction = d_ONLINE_ACTION_APPROVAL;

		/* 要更新端末機的日期及時間 */
		if (inFunc_SetEDCDateTime(pobTran->srBRec.szDate, pobTran->srBRec.szTime) != VS_SUCCESS)
			return (VS_ERROR);
	}
	/* 主機拒絕 */
	else
	{
		/* 主機回傳結果 */
		EMVGlobConfig.uszAction = d_ONLINE_ACTION_DECLINE;
		
		if (pobTran->inTransactionResult == _TRAN_RESULT_CANCELLED_)
		{
			/* Offline被主機拒絕交易的話，就不送Reversal */
			if (pobTran->srBRec.uszOfflineBit == VS_FALSE)
			{
				/* 如果是DFS卡回覆授權碼空白時，EDC轉為05拒絕交易並送Reversal */
				if (pobTran->uszDFSNoAuthCodeBit == VS_TRUE)
				{
					/* 不把送reversal的bit關掉 */
				}
				else
				{
					inSetSendReversalBit("N");
					inSaveHDPTRec(pobTran->srBRec.inHDTIndex);
				}
			}
			inNCCC_MFES_DispHostResponseCode(pobTran);	/* 顯示主機回的錯誤訊息 */

			return (VS_ERROR);
		}
		else
		{
			inNCCC_MFES_DispHostResponseCode(pobTran);	/* 顯示主機回的錯誤訊息 */

			return (VS_ERROR);
		}
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_MFES_OnlineEMV_Complete
Date&Time       :2016/11/17 下午 4:55
Describe        :處理Second Gen AC之後TC or Reversal
*/
int inNCCC_MFES_OnlineEMV_Complete(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
        char		szTemplate[512 + 1];
        char            szCustomerIndicator[3 + 1] = {0};
	unsigned char 	uszValue[128 + 1];
	unsigned short	usTagLen;
	RTC_NEXSYS	srRTC;
	
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
        inGetCustomIndicator(szCustomerIndicator);
        
	/* 8A */
	usTagLen = sizeof(uszValue);
	memset(uszValue, 0x00, sizeof(uszValue));
	inEMV_Get_Tag_Value(0x8A, &usTagLen, uszValue);
        
	/*  先檢核first gen AC的結果 若是Y1 或 Z1就不用在做Second Gen AC */
	if (!memcmp(uszValue, "Z1", 2))
	{
		/* Z1= declined offline (first Gen AC , CID=AAC) */
		/* 拒絕交易 */
		
		memset(pobTran->srBRec.szRespCode, 0x00, sizeof(pobTran->srBRec.szRespCode));
		strcpy(pobTran->srBRec.szRespCode, "Z1");
		
		inNCCC_MFES_DispHostResponseCode(pobTran);
		
		return (VS_ERROR);
	}
	else if (!memcmp(uszValue, "Y1", 2))
	{
		/* EMV First Gen AC產生Y1 */
		if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "EMV First Gen AC產生Y1");
		
		/* 新增感應及晶片go online防呆機制，若卡片進入offline流程，畫面提示”OL 拒絕交易”。*/
		if (pobTran->inEMVDecision == _EMV_DECESION_OFFLINE)
		{
			memcpy(pobTran->srBRec.szRespCode, "OL", strlen("OL"));
			inNCCC_MFES_DispHostResponseCode(pobTran);

			return (VS_ERROR);
		}
		
		memset(pobTran->srBRec.szRespCode, 0x00, sizeof(pobTran->srBRec.szRespCode));
		memset(pobTran->srBRec.szAuthCode, 0x00, sizeof(pobTran->srBRec.szAuthCode));
		strcpy(pobTran->srBRec.szRespCode, "Y1");
		strcpy(pobTran->srBRec.szAuthCode, "Y1");
		
		/* 卡片自行授權時（授權碼為 Y1/Y3），端末設備以 Offline Sale 電文上傳，
		 * 但 EMV Tag 資料視為 TC Upload。*/
		/* 將EMV參數更新存檔 */
		if (inNCCC_FuncEMVPrepareBatch(pobTran) != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
				inLogPrintf(AT, "inNCCC_FuncEMVPrepareBatch Error!");

			inFunc_EDCLock(AT);

			return (VS_ERROR);
		}
		
		/* 這裡是晶片卡offline 要疊加STAN */
		inNCCC_MFES_GetSTAN(pobTran);
		inNCCC_MFES_SetSTAN(pobTran);
		
		/* Y1存 Advice */
		if (inADVICE_SaveTop(pobTran, pobTran->srBRec.lnOrgInvNum) != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
				inLogPrintf(AT, "inADVICE_SaveTop() Error!");

			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
				inLogPrintf(AT, "inADVICE_SaveTop() SUCCESS!");
		}
		
		/* 紀錄已送到主機的調閱編號 */
		inNCCC_Func_Update_Memory_Invoice(pobTran);
			
		return (VS_SUCCESS);
	}
	else
	{
		/* 新增感應及晶片go online防呆機制，若卡片進入offline流程，畫面提示”OL 拒絕交易”。*/
		if (pobTran->inEMVDecision == _EMV_DECESION_OFFLINE)
		{
			memcpy(pobTran->srBRec.szRespCode, "OL", strlen("OL"));
			inNCCC_MFES_DispHostResponseCode(pobTran);

			return (VS_ERROR);
		}
		
		/* 要處理Second Gen AC之後的流程，含TC Upload（Call Bank的狀況） */
		if (pobTran->inTransactionResult == _TRAN_RESULT_REFERRAL_ && pobTran->inTransactionCode == _SALE_)
		{
			pobTran->inTransactionResult = _TRAN_RESULT_AUTHORIZED_;
			pobTran->srBRec.uszReferralBit = VS_TRUE;
			pobTran->srBRec.uszForceOnlineBit = VS_TRUE;
			pobTran->srBRec.uszOfflineBit = VS_FALSE;
			pobTran->srBRec.uszUpload1Bit = VS_FALSE;
			pobTran->srBRec.uszUpload2Bit = VS_FALSE;
			pobTran->srBRec.uszUpload3Bit = VS_FALSE;

			/* 取得EDC時間日期 */
			memset(&srRTC, 0x00, sizeof(RTC_NEXSYS));
			if (inFunc_GetSystemDateAndTime(&srRTC) != VS_SUCCESS)
			{
				return (VS_ERROR);
			}

			/* 更新pobTran內日期時間 */
			if (inFunc_Sync_BRec_Date_Time(pobTran, &srRTC) != VS_SUCCESS)
			{
				return (VS_ERROR);
			}

			/* Second Generate AC失敗但是銀行授權成功還是要算成功，
			* 因Second Gen AC在OnTxnResult前由API自己改晶片卡資料，
			* 而且call bank 以補登送要送原AuthCode，所以不再call inEMV_SecondGenerateAC來改pobTran的資料 */

			/*
			2014-08-14
			From: 吳升文(Angus Wu) [mailto:angus.wu@nccc.com.tw]
			Sent: Tuesday, August 12, 2014 9:01 PM

			若為CallBank完成之晶片卡交易，端末機需上傳Field_55，其Tag 8A之值須為”0x30 0x30”。
			補充說明：晶片卡Callbank時，卡片需要插在EDC中直到輸入授權碼後，執行EMV 2nd AC(Request TC)。
			故該筆Callbank交易完成交易需上傳Field_55。
			*/
			if (inEMV_Set_TagValue_During_Txn(d_TAG_ARC, 2, (unsigned char*)"\x30\x30") != VS_SUCCESS)
				return (VS_ERROR);

			/* 將EMV參數更新存檔 */
			if (inNCCC_FuncEMVPrepareBatch(pobTran) != VS_SUCCESS)
			{
				if (ginDebug == VS_TRUE)
					inLogPrintf(AT, "inNCCC_FuncEMVPrepareBatch Error!");

				inFunc_EDCLock(AT);

				return (VS_ERROR);
			}

			/* 送CallBank的Online交易補登 STAN要加一 */
			inNCCC_MFES_GetSTAN(pobTran);

			/* inFunc_REFERRAL_GetManualApproval會斷線，因此要重新連一次 */
			if (inFLOW_RunFunction(pobTran, _COMM_START_) != VS_SUCCESS)
			{
				/* 連線失敗 */
				return (VS_ERROR);
			}

			/* 連線成功*/
			/* 第一行顯示 <交易補登> */
			inDISP_ClearAll();
			inFunc_Display_LOGO(0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_CALL_BANK_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 授權碼補登 */

			pobTran->srBRec.inCode = _SALE_OFFLINE_;	/* 交易補登 */
			pobTran->inTransactionCode = _SALE_OFFLINE_;	/* 交易補登 */
			pobTran->inISOTxnCode = _SALE_OFFLINE_;
			pobTran->uszReversalBit = VS_TRUE;

			/* 步驟 1.先組Reversal */
			if ((inRetVal = inNCCC_MFES_ProcessReversal(pobTran)) != VS_SUCCESS)
			{
				return (inRetVal);
			}

			/* 步驟 2.送Online 交易 MTI 0200 */
			inRetVal = inNCCC_MFES_ProcessOnline(pobTran);
			inNCCC_MFES_SetSTAN(pobTran); /* 成功或失敗 System_Trace_Number 都要加一 */

			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal); /* Return 【VS_ERROR】【VS_SUCCESS】 */
			}

			/* 步驟 3.分析資料 */
			if (inNCCC_MFES_AnalysePacket(pobTran) == VS_ERROR)
			{
				return (VS_ERROR);
			}

			/* 避免撥接太久，收送完就斷線 */
			inCOMM_End(pobTran);

			/* 紀錄已送到主機的調閱編號 */
			inNCCC_Func_Update_Memory_Invoice(pobTran);
			
			return (VS_SUCCESS);
		}
		else if (pobTran->inTransactionCode == _PRE_AUTH_ ||pobTran->inTransactionCode == _CUP_PRE_AUTH_)
		{
			/* 開始執行 Second Generate AC */
			inRetVal = inEMV_SecondGenerateAC(pobTran);
			
			/* 將EMV參數更新存檔 */
			if (inNCCC_FuncEMVPrepareBatch(pobTran) != VS_SUCCESS)
			{
				if (ginDebug == VS_TRUE)
					inLogPrintf(AT, "inNCCC_FuncEMVPrepareBatch Error!");

				inFunc_EDCLock(AT);

				return (VS_ERROR);
			}
			
			if (inRetVal == VS_SUCCESS)
			{
				/* 這裡成功交易有2種【通訊成功】【通訊失敗產生Y3】 */
				/* Pre Auth 通訊成功後，不用送TC */
				if (!memcmp(&pobTran->srBRec.szAuthCode[0], "Y3", 2))
				{
					inSetSendReversalBit("N");
					if (inSaveHDPTRec(pobTran->srBRec.inHDTIndex) != VS_SUCCESS)
					{
						inFunc_EDCLock(AT);

						return (VS_ERROR);
					}

					/* 紀錄已送到主機的調閱編號 */
					inNCCC_Func_Update_Memory_Invoice(pobTran);
			
					return (VS_SUCCESS);
				}
				/* Pre Auth 沒有Y3 */
				else
				{
                                        /* 紅利、分期交易也支援「OL 拒絕交易 */
					memset(pobTran->srBRec.szRespCode, 0x00, sizeof(pobTran->srBRec.szRespCode));
					strcpy(pobTran->srBRec.szRespCode, "OL");
					inNCCC_MFES_DispHostResponseCode(pobTran);
					
					return (VS_ERROR);
				}

			}
			/* Second Gen AC Fail */
			else
			{
				/* 因為主機回成功或是通訊失敗但是最後卡片拒絕交易所以要組重新組【REVERSAL】 */
				if (pobTran->inTransactionResult == _TRAN_RESULT_AUTHORIZED_ || pobTran->inTransactionResult == _TRAN_RESULT_COMM_ERROR_)
				{
					if (pobTran->uszCus099_Already_Decide_Reversal_Bit == VS_TRUE)
					{
						/* 已送過reversal，不必再組 */
					}
					else
					{
						/* 重組【REVERSAL】 */
						if (inNCCC_MFES_ReversalSave(pobTran) != VS_SUCCESS)
						{
							if (ginDebug == VS_TRUE)
								inLogPrintf(AT, "inNCCC_MFES_ReversalSave() Error!");

							inFunc_EDCLock(AT);

							return (VS_ERROR);
						}
					}
                                        
                                        if (pobTran->inTransactionResult == _TRAN_RESULT_COMM_ERROR_)
					{
						/* 通訊失敗 */
					}
					else
					{
						inNCCC_MFES_DispHostResponseCode(pobTran);
					}
					
					if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_099_SINYA_, _CUSTOMER_INDICATOR_SIZE_))
					{
						inNCCC_Func_Process_Reversal_Inform(pobTran);
					}
					/* 026印兩聯，099只印一聯 */
					else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_026_TAKA_, _CUSTOMER_INDICATOR_SIZE_) ||
						 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
					{
						inNCCC_Func_Process_Reversal_Inform(pobTran);
					}
				}
				else
				{
					/* 如果是DFS卡回覆授權碼空白時，EDC轉為05拒絕交易並送Reversal */
					if (pobTran->uszDFSNoAuthCodeBit == VS_TRUE)
					{
						/* 不把送reversal的bit關掉 */
						if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_099_SINYA_, _CUSTOMER_INDICATOR_SIZE_))
						{
							inNCCC_Func_Process_Reversal_Inform(pobTran);
						}
						/* 026印兩聯，099只印一聯 */
						else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_026_TAKA_, _CUSTOMER_INDICATOR_SIZE_) ||
							 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
						{
							inNCCC_Func_Process_Reversal_Inform(pobTran);
						}
					}
					else
					{
						inSetSendReversalBit("N");
						if (inSaveHDPTRec(pobTran->srBRec.inHDTIndex) != VS_SUCCESS)
						{
							inFunc_EDCLock(AT);

							return (VS_ERROR);
						}
					}
                                        
//					/* 主機有回拒絕，不顯示晶片錯誤碼 */
//					inNCCC_MFES_DispHostResponseCode(pobTran);
				}

				return (VS_ERROR);
			}
		}
		else
		{
			/* 開始執行 Second Generate AC */
			inRetVal = inEMV_SecondGenerateAC(pobTran);
                        
                        if (ginDebug == VS_TRUE)
                        {
                                sprintf(szTemplate, "inEMVAPISecondGenerateAC = %d", inRetVal);
                                inLogPrintf(AT, szTemplate);
                        }

			/* 將EMV參數更新存檔 */
			if (inNCCC_FuncEMVPrepareBatch(pobTran) != VS_SUCCESS)
                        {
                                if (ginDebug == VS_TRUE)
                                        inLogPrintf(AT, "inNCCC_FuncEMVPrepareBatch Error!");
				
                                inFunc_EDCLock(AT);
				
				return (VS_ERROR);
                        }
			
			/*
			若是預借現金通訊失敗直接Return
			端末機還是要執行Second Gen AC，不過請端末機判斷若是因Unable to go Online產生Y3 (TC)，端末機請直接轉為通訊錯誤(或失敗)之訊息
			*/
			if (pobTran->inTransactionCode == _CASH_ADVANCE_ && pobTran->inTransactionResult == _TRAN_RESULT_COMM_ERROR_)
			{
				inRetVal = VS_ERROR;
			}

			/* Second Gen AC Approve */
			if (inRetVal == VS_SUCCESS)
			{
				if (pobTran->inTransactionResult == _TRAN_RESULT_AUTHORIZED_)
				{
					/* 這裡成功交易有2種【通訊成功】【通訊失敗產生Y3】 */
					if (!memcmp(&pobTran->srBRec.szAuthCode[0], "Y3", 2))
					{
						inSetSendReversalBit("N");
						if (inSaveHDPTRec(pobTran->srBRec.inHDTIndex) != VS_SUCCESS)
						{
							inFunc_EDCLock(AT);

							return (VS_ERROR);
						}
					}
					/* Y3 */
					else
					{
						/* 防呆紅利分期不會有【Y3】 */
						if (pobTran->inTransactionCode == _INST_SALE_	||
						    pobTran->inTransactionCode == _REDEEM_SALE_	||
						    pobTran->inTransactionCode == _CUP_SALE_)
						{
                                                        /* 紅利、分期交易也支援「OL 拒絕交易 */
							memset(pobTran->srBRec.szRespCode, 0x00, sizeof(pobTran->srBRec.szRespCode));
							strcpy(pobTran->srBRec.szRespCode, "OL");
							inNCCC_MFES_DispHostResponseCode(pobTran);
							
							return (VS_ERROR);
						}
						
					}
					/* 在這裡送【TC UPLOAD】或組【Y3 Advice】 */
					pobTran->uszTCUploadBit = VS_TRUE;
					pobTran->srBRec.uszTCUploadBit = VS_FALSE;
					/* TC_UPLOAD 要加一 */
					inNCCC_MFES_GetSTAN(pobTran);
                                        
					inRetVal = inFLOW_RunFunction(pobTran, _COMM_START_);
                                        inNCCC_MFES_SetSTAN(pobTran);

					/* Add by hanlin 2013/9/10 PM 03:38:48 因為EMV流程會在inMCAP_ISO_BuildAndSendPacket完之後再跑OnlineAnalyse，因此要重新連一次 */
					if (inRetVal != VS_SUCCESS)
					{
						/* 失敗要存 Advice */
						if (inADVICE_SaveTop(pobTran, pobTran->srBRec.lnOrgInvNum) != VS_SUCCESS)
						{
							if (ginDebug == VS_TRUE)
								inLogPrintf(AT, "inADVICE_SaveTop() Error!");

							inFunc_EDCLock(AT);

							return (VS_ERROR);
						}
						else
						{
							if (ginDebug == VS_TRUE)
								inLogPrintf(AT, "inADVICE_SaveTop() SUCCESS!");

						}

						/* 紀錄已送到主機的調閱編號 */
						inNCCC_Func_Update_Memory_Invoice(pobTran);
						
						return (VS_SUCCESS);
					}
					/* 連線成功*/
					else
					{
						/* 送TC UPLOAD或advice*/
						inRetVal = inNCCC_MFES_AdviceSendRecvPacket(pobTran, 1);
						
						if (inRetVal != VS_SUCCESS)
						{
							if (ginDebug == VS_TRUE)
								inLogPrintf(AT, "inNCCC_MFES_AdviceSendRecvPacket() Error!");

							/* 失敗要存 Advice */
							if (inADVICE_SaveTop(pobTran, pobTran->srBRec.lnOrgInvNum) != VS_SUCCESS)
							{
								if (ginDebug == VS_TRUE)
									inLogPrintf(AT, "inADVICE_SaveTop() Error!");

								inFunc_EDCLock(AT);

								return (VS_ERROR);
							}
							else
							{
								if (ginDebug == VS_TRUE)
									inLogPrintf(AT, "inADVICE_SaveTop() SUCCESS!");

							}

							/* 紀錄已送到主機的調閱編號 */
							inNCCC_Func_Update_Memory_Invoice(pobTran);
			
							return (VS_SUCCESS);
						}
						else
						{
							if (ginDebug == VS_TRUE)
								inLogPrintf(AT, "inNCCC_MFES_AdviceSendRecvPacket() SUCCESS!");

							/* 這裡是For當筆，因為inNCCC_MFES_ISOAdviceAnalyse的pobTran是ADVPobTran，所以TRT的update Batch不會存到在那裡的改動 */
							/* 在這裡把bit On起來，避免TC重送 */
							pobTran->srBRec.uszTCUploadBit = VS_TRUE;

							/* 紀錄已送到主機的調閱編號 */
							inNCCC_Func_Update_Memory_Invoice(pobTran);
							
							return (VS_SUCCESS);
						}
						
					}
					
				}
				/* 主機拒絕，不用多做處理 */
				else
				{
					return (VS_ERROR);
				}
				
			}
			else
			{	
				/* 因為主機回成功或是通訊失敗但是最後卡片拒絕交易所以要組重新組【REVERSAL】 */
				if (pobTran->inTransactionResult == _TRAN_RESULT_AUTHORIZED_ || pobTran->inTransactionResult == _TRAN_RESULT_COMM_ERROR_)
				{
					if (pobTran->uszCus099_Already_Decide_Reversal_Bit == VS_TRUE)
					{
						/* 已送過reversal，不必再組 */
					}
					else
					{
						/* 重組【REVERSAL】 */
						if (inNCCC_MFES_ReversalSave(pobTran) != VS_SUCCESS)
						{
							if (ginDebug == VS_TRUE)
								inLogPrintf(AT, "inNCCC_MFES_ReversalSave() Error!");

							inFunc_EDCLock(AT);

							return (VS_ERROR);
						}
					}
                                        
                                        /* 拒絕交易Z3 */
					if (pobTran->inTransactionResult == _TRAN_RESULT_COMM_ERROR_)
					{
					}
					else
					{
						inNCCC_MFES_DispHostResponseCode(pobTran);			/* 拒絕交易Z3 */
					}
					
					if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_099_SINYA_, _CUSTOMER_INDICATOR_SIZE_))
					{
						inNCCC_Func_Process_Reversal_Inform(pobTran);
					}
					/* 026印兩聯，099只印一聯 */
					else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_026_TAKA_, _CUSTOMER_INDICATOR_SIZE_) ||
						 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
					{
						inNCCC_Func_Process_Reversal_Inform(pobTran);
					}
				}
				else
				{
					/* 如果是DFS卡回覆授權碼空白時，EDC轉為05拒絕交易並送Reversal */
					if (pobTran->uszDFSNoAuthCodeBit == VS_TRUE)
					{
						/* 不把送reversal的bit關掉 */
						if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_099_SINYA_, _CUSTOMER_INDICATOR_SIZE_))
						{
							inNCCC_Func_Process_Reversal_Inform(pobTran);
						}
						/* 026印兩聯，099只印一聯 */
						else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_026_TAKA_, _CUSTOMER_INDICATOR_SIZE_) ||
							 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
						{
							inNCCC_Func_Process_Reversal_Inform(pobTran);
						}
					}
					else
					{
						inSetSendReversalBit("N");
						if (inSaveHDPTRec(pobTran->srBRec.inHDTIndex) != VS_SUCCESS)
						{
							inFunc_EDCLock(AT);

							return (VS_ERROR);
						}
					}
                                        
//					/* 主機有回拒絕，不顯示晶片錯誤碼 */
//                                        inNCCC_MFES_DispHostResponseCode(pobTran);				/* 拒絕交易 */
				}

				return (VS_ERROR);
			}
			
		}
		
	}
}

/*
Function        :inNCCC_MFES_ProcessSettleBatchUpload
Date&Time       :2016/9/14 上午 10:08
Describe        :
*/
int inNCCC_MFES_ProcessSettleBatchUpload(TRANSACTION_OBJECT *pobTran)
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
			
                        /* NCCC 取消(void)不會上傳 batch */
			/* 因為取消incode有很多種，所以改以 已取消的flag來判斷 */
                        if (pobTran->srBRec.inCode == _PRE_AUTH_ || pobTran->srBRec.uszVOIDBit == VS_TRUE)
                                continue;
			
			/* 表示要上傳的合法筆數 */
			inBatchValidCnt ++;

                        if (inBatchValidCnt == inBatchValidTotalCnt)
                                pobTran->uszLastBatchUploadBit = VS_TRUE; /* 最後一筆交易 */
			
			/* 抓最新的STAN */
			inNCCC_MFES_GetSTAN(pobTran);

			inRetVal = inNCCC_MFES_SendPackRecvUnPack(pobTran);
			/* 成功或失敗 System Trace Number 都要加一 */
			inNCCC_MFES_SetSTAN(pobTran);
			
			if (inRetVal != VS_SUCCESS)
                        {
                                inRunCLS_BATCH = VS_FALSE;
                                break;
                        }
                        else
                        {
				/* 回傳非00，表示失敗 */
                                if (memcmp(pobTran->srBRec.szRespCode, "00", 2))
                                {
					inNCCC_MFES_DispHostResponseCode(pobTran);		/* 顯示主機回的錯誤訊息 */
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
	inNCCC_MFES_GetSTAN(pobTran);

        if (inRunCLS_BATCH == VS_TRUE)
        {
                /* 暫時保留
                 pobTran->srBRec.uszManualBit = VS_FALSE;
                 pobTran->srBRec.uszVOIDBit = VS_FALSE;
                 pobTran->srBRec.uszSignatureBit = VS_FALSE;
                 pobTran->srBRec.uszOfflineBit = VS_FALSE;
                 */
		/* 930000 – 前次不平帳上傳失敗 */
		inSetCLS_SettleBit("N");
		inSaveHDPTRec(pobTran->srBRec.inHDTIndex);

                pobTran->inISOTxnCode = _CLS_BATCH_;

		inRetVal = inNCCC_MFES_SendPackRecvUnPack(pobTran);
		/* 成功或失敗 System Trace Number 都要加一 */
		inNCCC_MFES_SetSTAN(pobTran);
			
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
                else
                {
			/* 回傳非00，表示失敗 */
                        if (memcmp(pobTran->srBRec.szRespCode, "00", 2))
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
Function        :inNCCC_MFES_DispHostResponseCode
Date&Time       :2016/11/15 下午 5:44
Describe        :顯示錯誤代碼
*/
int inNCCC_MFES_DispHostResponseCode(TRANSACTION_OBJECT *pobTran)
{
	int		inChoice = _DisTouch_No_Event_;
	int		inRetVal = VS_SUCCESS;
	char		szResponseCode[10 + 1] = {0};
	char		szMsg[42 + 1] = {0};
	char		szCustomerIndicator[3 + 1] = {0};
        unsigned char   uszKey = 0x00;
	
	memset(szMsg, 0x00, sizeof(szMsg));
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	if ((pobTran->srBRec.szRespCode[0] >= '0' && pobTran->srBRec.szRespCode[0] <= '9') &&
	    (pobTran->srBRec.szRespCode[1] >= '0' && pobTran->srBRec.szRespCode[1] <= '9'))
	{
		if (!memcmp(&pobTran->srBRec.szRespCode[0], "13", 2))
			sprintf(szMsg, "%s", "請聯絡發卡銀行");			/* 請聯絡發卡銀行 */
		else if (!memcmp(&pobTran->srBRec.szRespCode[0], "25", 2))
			sprintf(szMsg, "%s", "無原授權記錄");			/* 無原授權記錄 */
		else if (!memcmp(&pobTran->srBRec.szRespCode[0], "30", 2))
			sprintf(szMsg, "%s", "請重新交易");			/* 請重新交易 */
		else if (!memcmp(&pobTran->srBRec.szRespCode[0], "38", 2))
			sprintf(szMsg, "%s", "密碼錯誤次數超出");		/* 密碼錯誤次數超出 */
		else if (!memcmp(&pobTran->srBRec.szRespCode[0], "51", 2))
			sprintf(szMsg, "%s", "本交易不接受");			/* 本交易不接受 */
		else if (!memcmp(&pobTran->srBRec.szRespCode[0], "55", 2))
			sprintf(szMsg, "%s", "密碼錯誤");			/* 密碼錯誤 */
		else if (!memcmp(&pobTran->srBRec.szRespCode[0], "40", 2))
			sprintf(szMsg, "%s", "拒絕交易");			/* 拒絕交易 */
		else if (!memcmp(&pobTran->srBRec.szRespCode[0], "98", 2))
			sprintf(szMsg, "%s", "請聯絡發卡銀行");			/* 請聯絡發卡銀行 */
		else if (!memcmp(&pobTran->srBRec.szRespCode[0], "05", 2))
			sprintf(szMsg, "%s", "拒絕交易");			/* 拒絕交易 */
		else if (!memcmp(&pobTran->srBRec.szRespCode[0], "01", 2) ||
			 !memcmp(&pobTran->srBRec.szRespCode[0], "02", 2))
		{
			/* 	DISCOVER交易不支援Call bank交易，提示「XX 拒絕交易」。 */
			/* 要排除TWIN卡的狀況 */
			if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)) == 0 &&
			    pobTran->srBRec.uszUCARDTransBit != VS_TRUE)
			{
				sprintf(szMsg, "%s", "拒絕交易");	/* 拒絕交易 */
			}
			else
			{
				sprintf(szMsg, "%s", "請聯絡發卡銀行");
			}
		}
		else
		{
			/* 轉換回應訊息 */
			inNCCC_Func_ResponseCode_Transform(atoi(pobTran->srBRec.szRespCode), szMsg);
		}
	}
	else
	{
		if (!memcmp(&pobTran->srBRec.szRespCode[0], "0X", 2))
			sprintf(szMsg, "%s", "拒絕交易");			/* 拒絕交易 */
		else if (!memcmp(&pobTran->srBRec.szRespCode[0], "A0", 2))
			sprintf(szMsg, "%s", "驗証錯誤請重試");			/* 驗証錯誤請重試 */
		else if (!memcmp(&pobTran->srBRec.szRespCode[0], "A1", 2))
			sprintf(szMsg, "%s", "驗証錯誤請重試");			/* 驗証錯誤請重試 */
		else if (!memcmp(&pobTran->srBRec.szRespCode[0], "C8", 2))
			sprintf(szMsg, "%s", "感應交易超額");			/* 感應交易超額 */
		else if (!memcmp(&pobTran->srBRec.szRespCode[0], "C9", 2))
			sprintf(szMsg, "%s", "感應交易超次");			/* 感應交易超次 */
		else if (!memcmp(&pobTran->srBRec.szRespCode[0], "XA", 2))
			sprintf(szMsg, "%s", "拒絕交易");			/* 拒絕交易 */
		else if (!memcmp(&pobTran->srBRec.szRespCode[0], "XB", 2))
			sprintf(szMsg, "%s", "拒絕交易");			/* 拒絕交易 */
		else if (!memcmp(&pobTran->srBRec.szRespCode[0], "XC", 2))
			sprintf(szMsg, "%s", "請勿按銀聯鍵");			/* 請勿按銀聯鍵 */
		else if (!memcmp(&pobTran->srBRec.szRespCode[0], "XN", 2))
			sprintf(szMsg, "%s", "請改按銀聯鍵");			/* 請改按銀聯鍵 */
		else if (!memcmp(&pobTran->srBRec.szRespCode[0], "XV", 2))
			sprintf(szMsg, "%s", "輸入銀聯背面三碼");		/* 輸入銀聯背面三碼 */
		else if (!memcmp(&pobTran->srBRec.szRespCode[0], "XX", 2))
			sprintf(szMsg, "%s", "請改刷磁條");			/* 請改刷磁條 */
		else if (!memcmp(&pobTran->srBRec.szRespCode[0], "XY", 2))
			sprintf(szMsg, "%s", "NPS拆帳單");			/* NPS拆帳單 */
		else if (!memcmp(&pobTran->srBRec.szRespCode[0], "XZ", 2))
			sprintf(szMsg, "%s", "拒絕交易");			/* 拒絕交易 */
		else if (!memcmp(&pobTran->srBRec.szRespCode[0], "XI", 2))
			sprintf(szMsg, "%s", "拒絕交易");			/* 拒絕交易 */
		else if (!memcmp(&pobTran->srBRec.szRespCode[0], "L1", 2))
			sprintf(szMsg, "%s", "無優惠可兌換");			/* 無優惠可兌換 */
		else if (!memcmp(&pobTran->srBRec.szRespCode[0], "L3", 2))
			sprintf(szMsg, "%s", "優惠已兌換");			/* 優惠已兌換 */
		else if (!memcmp(&pobTran->srBRec.szRespCode[0], "L4", 2))
			sprintf(szMsg, "%s", "兌換期限已過");			/* 兌換期限已過 */
		else if (!memcmp(&pobTran->srBRec.szRespCode[0], "L5", 2))
			sprintf(szMsg, "%s", "請以刷卡作兌換");			/* 請以刷卡作兌換 */
		else if (!memcmp(&pobTran->srBRec.szRespCode[0], "L6", 2))
			sprintf(szMsg, "%s", "請掃描條碼作兌換");		/* 請掃描條碼作兌換 */
		else if (!memcmp(&pobTran->srBRec.szRespCode[0], "L7", 2))
			sprintf(szMsg, "%s", "無兌換可取消");			/* 無兌換可取消 */
		else if (!memcmp(&pobTran->srBRec.szRespCode[0], "L8", 2))
			sprintf(szMsg, "%s", "重複取消");			/* 重複取消 */
		else if (!memcmp(&pobTran->srBRec.szRespCode[0], "L9", 2))
			sprintf(szMsg, "%s", "取消期限已過");			/* 取消期限已過 */
		else if (!memcmp(&pobTran->srBRec.szRespCode[0], "ID", 2))
			sprintf(szMsg, "%s", "ID身分字號錯誤");			/* ID身分字號錯誤 */
		else if (!memcmp(&pobTran->srBRec.szRespCode[0], "I0", 2))
			sprintf(szMsg, "%s", "分期電文錯誤");			/* 分期電文錯誤 */
		else if (!memcmp(&pobTran->srBRec.szRespCode[0], "R0", 2))
			sprintf(szMsg, "%s", "紅利電文錯誤");			/* 紅利電文錯誤 */
		else if (!memcmp(&pobTran->srBRec.szRespCode[0], "Y1", 2))
			sprintf(szMsg, "%s", "拒絕交易");			
		else if (!memcmp(&pobTran->srBRec.szRespCode[0], "Z1", 2))
			sprintf(szMsg, "%s", "拒絕交易");			
		else if (!memcmp(&pobTran->srBRec.szRespCode[0], "Y3", 2))
			sprintf(szMsg, "%s", "拒絕交易");			
		else if (!memcmp(&pobTran->srBRec.szRespCode[0], "Z3", 2))
			sprintf(szMsg, "%s", "拒絕交易");
		else if (!memcmp(&pobTran->srBRec.szRespCode[0], "OL", 2))
			sprintf(szMsg, "%s", "拒絕交易");
		else
		{
			strcpy(pobTran->srBRec.szRespCode, "05");
			sprintf(szMsg, "%s", "拒絕交易");			/* Display 【拒絕交易】 */
		}
	}
		
	memset(szResponseCode, 0x00, sizeof(szResponseCode));	
	sprintf(szResponseCode, "%s", pobTran->srBRec.szRespCode);		/* 錯誤代碼 */
	
	/* (需求單-110202)-修改客製化參數111卡人自助EDC UI訊息需求 by Russell 2021/12/23 下午 1:57 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_)	||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)		||
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
	else
	{
                /* Mirror Message */
                if (pobTran->uszECRBit == VS_TRUE)
                {
                        inECR_SendMirror(pobTran, _MIRROR_MSG_DECLINED_);
                }
                
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
Function        :inNCCC_MFES_SyncHostTerminalDateTime
Date&Time       :2016/9/14 上午 10:14
Describe        :
*/
int inNCCC_MFES_SyncHostTerminalDateTime(TRANSACTION_OBJECT *pobTran)
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
Function        :inNCCC_MFES_ProcessReferral
Date&Time       :2016/9/14 上午 10:14
Describe        :
*/
int inNCCC_MFES_ProcessReferral(TRANSACTION_OBJECT *pobTran)
{
        /* 輸入授權碼 */
        if (inFunc_REFERRAL_GetManualApproval(pobTran) != VS_SUCCESS)
                return (VS_ERROR);

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_MFES_GetReversalCnt
Date&Time       :2016/9/13 下午 5:02
Describe        :
*/
int inNCCC_MFES_GetReversalCnt(TRANSACTION_OBJECT *pobTran)
{
        int 	inCnt;
        char 	szReversalCnt[6 + 1];

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_GetReversalCnt() START!");

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
Function        :inNCCC_MFES_SetReversalCnt
Date&Time       :2016/9/13 下午 5:02
Describe        :
*/
int inNCCC_MFES_SetReversalCnt(TRANSACTION_OBJECT *pobTran, long lnSetMode)
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
Function        :inNCCC_MFES_GetMustSettleBit
Date&Time       :2016/9/14 上午 10:12
Describe        :
*/
int inNCCC_MFES_GetMustSettleBit(TRANSACTION_OBJECT *pobTran, char *szMustSettleBit)
{
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_GetMustSettleBit() START!");

	/* 刷完卡就LoadHDPT，不另在Load */
        memset(szMustSettleBit, 0x00, sizeof(szMustSettleBit));
        if (inGetMustSettleBit(szMustSettleBit) == VS_ERROR)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inNCCC_MFES_GetMustSettleBit ERROR!");
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_MFES_SetMustSettleBit
Date&Time       :2016/9/14 上午 10:12
Describe        :
*/
int inNCCC_MFES_SetMustSettleBit(TRANSACTION_OBJECT *pobTran, char *szMustSettleBit)
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
Function        :inNCCC_MFES_CUP_LogOn
Date&Time       :2015/12/23 早上 10:25
Describe        :這裡傳進的pobTran只做連線狀態確認
*/
int inNCCC_MFES_CUP_LogOn(TRANSACTION_OBJECT* pobTran)
{
	int			i = 0, inRetVal = VS_ERROR;
	int			inCUPKeyExchangeTimes = 0;
	char			szSendReversalBit[2 + 1] = {0};
	char			szCUPKeyExchangeTimes[1 + 1] = {0};
        char 			szTMSOK[2 + 1] = {0};
	char			szBatchNum[6 + 1] = {0};
	char			szDemoMode[2 + 1] = {0};
	TRANSACTION_OBJECT 	pobCUPLogOnTran;
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_CUP_PowerOnLogon() START!");
	
	memset(&pobCUPLogOnTran, 0x00, sizeof(TRANSACTION_OBJECT));
        memset(szTMSOK, 0x00, sizeof(inGetTMSOK));
	
	pobCUPLogOnTran.uszConnectionBit = pobTran->uszConnectionBit;
	
	/* 安全認證 */
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_LOGON_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第一層顯示 <安全認證> */

	/* 教育訓練模式 */
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		return (VS_SUCCESS);
	}
	else
	{
		inGetTMSOK(szTMSOK);

		if (szTMSOK[0] != 'Y')
		{
			if (ginDebug == VS_TRUE)
				inLogPrintf(AT, "TMSOK != Y");

			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_TMS_DWL_);
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

		/* 安全認證前先檢查鎖機狀態 */
		if (inFunc_Check_EDCLock() != VS_SUCCESS)
			return (VS_ERROR);

		if (inLoadHDTRec(0) < 0)
		{
			return (VS_ERROR);
		}

		if (inLoadHDPTRec(0) < 0)
		{
			return (VS_ERROR);
		}

		memset(szBatchNum, 0x00, sizeof(szBatchNum));
		inGetBatchNum(szBatchNum);
		pobCUPLogOnTran.srBRec.lnBatchNum = atol(szBatchNum);

		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_); /* 先清除螢幕顯示 */
		inDISP_PutGraphic(_PROCESS_, 0, _COORDINATE_Y_LINE_8_7_); /* 處理中‧‧‧‧‧ */

		/* 這邊採用跟Verifone Code一樣的邏輯，安全認證時先砍Working Key，若找不到Key代表安全認證失敗 */
		inKMS_DeleteKey(_TWK_KEYSET_NCCC_, _TWK_KEYINDEX_NCCC_PIN_ONLINE_);
		inKMS_DeleteKey(_TWK_KEYSET_NCCC_, _TWK_KEYINDEX_NCCC_MAC_);

		if (pobCUPLogOnTran.uszConnectionBit != VS_TRUE)
		{
			if (inFLOW_RunFunction(&pobCUPLogOnTran, _COMM_START_) != VS_SUCCESS)
			{
				/* 通訊失敗‧‧‧‧ */
				pobCUPLogOnTran.inTransactionResult = _TRAN_RESULT_COMM_ERROR_;
				pobCUPLogOnTran.inErrorMsg = _ERROR_CODE_V3_COMM_;
				inFunc_Display_Error(pobTran);			/* 通訊失敗 */

				inCOMM_End(&pobCUPLogOnTran);

				return (VS_COMM_ERROR);
			}
		}

		memset(szSendReversalBit, 0x00, sizeof(szSendReversalBit));
		if (inGetSendReversalBit(szSendReversalBit) == VS_ERROR)
			return (VS_ERROR);

		/* 檢查是否要先送 Reversal */
		if (!memcmp(szSendReversalBit, "Y", 1))
		{
			/* 開機時沒過卡，不會存inHDTIndex，但ReversalBit 儲存會用到，CUP logon一定是NCCC，所以hardcode為0 */
			pobCUPLogOnTran.srBRec.inHDTIndex = 0;

			if ((inRetVal = inNCCC_MFES_ReversalSendRecvPacket(&pobCUPLogOnTran)) != VS_SUCCESS)
			{
				/* 斷線，將連線狀態給外面的pobTran */
				inCOMM_End(&pobCUPLogOnTran);
				pobTran->uszConnectionBit = pobCUPLogOnTran.uszConnectionBit;

				return (inRetVal);
			}

			if (inNCCC_MFES_SetReversalCnt(&pobCUPLogOnTran, _ADD_) == VS_ERROR)
			{
				/* 斷線，將連線狀態給外面的pobTran */
				inCOMM_End(&pobCUPLogOnTran);
				pobTran->uszConnectionBit = pobCUPLogOnTran.uszConnectionBit;

				return (VS_ERROR);
			}
		}

		pobCUPLogOnTran.srBRec.inCode = _CUP_LOGON_; /* 設定交易類別 */
		pobCUPLogOnTran.inTransactionCode = _CUP_LOGON_; /* 設定交易類別 */
		pobCUPLogOnTran.inISOTxnCode = _CUP_LOGON_; /* 組 ISO 電文 */
		pobCUPLogOnTran.srBRec.uszRewardL1Bit = VS_FALSE; /* CUP_LOGON 不送優惠資訊 */
		pobCUPLogOnTran.srBRec.uszRewardL2Bit = VS_FALSE; /* CUP_LOGON 不送優惠資訊 */
		pobCUPLogOnTran.srBRec.uszRewardL5Bit = VS_FALSE; /* CUP_LOGON 不送優惠資訊 */

		/* 要送 Field_37 Retrieval Reference Number */
		inNCCC_Func_MakeRefNo(&pobCUPLogOnTran);

		/* 組_送_收_解 */
		memset(szCUPKeyExchangeTimes, 0x00, sizeof(szCUPKeyExchangeTimes));
		inGetCUPKeyExchangeTimes(szCUPKeyExchangeTimes);
		inCUPKeyExchangeTimes = atoi(szCUPKeyExchangeTimes);

		/* 先預設認證失敗，直到收到00才把inRetVal設為VS_SUCCESS */
		inRetVal = VS_ERROR;
		for (i = 0; i < (inCUPKeyExchangeTimes + 1); i ++)
		{
			/* 交易結果初始化 */
			pobCUPLogOnTran.inTransactionResult = 0;

			if (pobCUPLogOnTran.uszConnectionBit != VS_TRUE)
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_PutGraphic(_PROCESS_, 0, _COORDINATE_Y_LINE_8_7_); /* 處理中‧‧‧‧‧ */

				if (inFLOW_RunFunction(&pobCUPLogOnTran, _COMM_START_) != VS_SUCCESS)
				{
					/* 通訊失敗‧‧‧‧ */
					pobCUPLogOnTran.inTransactionResult = _TRAN_RESULT_COMM_ERROR_;
					pobCUPLogOnTran.inErrorMsg = _ERROR_CODE_V3_COMM_;

					inRetVal = VS_COMM_ERROR;
					continue;
				}
				else
				{

				}
			}

			inNCCC_MFES_GetSTAN(&pobCUPLogOnTran);

			inRetVal = inNCCC_MFES_SendPackRecvUnPack(&pobCUPLogOnTran);
			inNCCC_MFES_SetSTAN(&pobCUPLogOnTran); /* 成功或失敗 System Trace Number 都要加一 */

			if (inRetVal != VS_SUCCESS)
			{
				/* 通訊失敗‧‧‧‧ */
				if (inRetVal == VS_WRITE_KEY_ERROR)
				{
					pobCUPLogOnTran.inTransactionResult = _TRAN_RESULT_AUTHORIZED_;
					pobCUPLogOnTran.inErrorMsg = _ERROR_CODE_V3_WRITE_KEY_INIT_FAIL_;
				}
				else
				{
					pobCUPLogOnTran.inTransactionResult = _TRAN_RESULT_COMM_ERROR_;
					pobCUPLogOnTran.inErrorMsg = _ERROR_CODE_V3_COMM_;
				}

				inCOMM_End(&pobCUPLogOnTran);

				continue;
			}
			else
			{
				pobCUPLogOnTran.inTransactionResult = inNCCC_MFES_CheckRespCode(&pobCUPLogOnTran);

				if (pobCUPLogOnTran.inTransactionResult != _TRAN_RESULT_AUTHORIZED_)
				{
					inRetVal = VS_ERROR;
					continue;
				}

				pobCUPLogOnTran.inErrorMsg = _ERROR_CODE_V3_NONE_;
				inRetVal = VS_SUCCESS;
				break;
			}

		}

		if (inRetVal != VS_SUCCESS)
		{
			/* 結帳時，安全認證失敗不提示錯誤訊息 */
			if (pobTran->inTransactionCode == _SETTLE_	||
			    pobTran->inTransactionCode == _CLS_SETTLE_)
			{
				pobTran->uszSettleLOGONFailedBit = VS_TRUE;
			}
			else
			{
				inFunc_Display_Error(&pobCUPLogOnTran);			/* 通訊失敗 */
			}
		}
		else
		{
			/* 要更新端末機的日期及時間 */
			if (inNCCC_MFES_SyncHostTerminalDateTime(&pobCUPLogOnTran) != VS_SUCCESS)
				return (VS_ERROR);
		}

		/* 第一次斷線，將連線狀態給外面的pobTran */
		inCOMM_End(&pobCUPLogOnTran);

		pobTran->uszConnectionBit = pobCUPLogOnTran.uszConnectionBit;

		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "inNCCC_MFES_CUP_PowerOnLogon() END!");

		return (inRetVal);
	}
}

/*
Function        :inNCCC_MFES_TMKKeyExchange_Software
Date&Time       :2016/12/30 下午 1:38
Describe        :
*/
int inNCCC_MFES_TMKKeyExchange(TRANSACTION_OBJECT *pobTran,unsigned char *uszUnPackBuf)
{
	int	inCnt, inLen, inTMKindex, inPINLen, inMACLen;
	char	szLen[2 + 1], szTemplate[96 + 1];
	char	szKeyExChange[48 + 1];
	
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_TMKKeyExchange() Start!");
	
	/* 初始化 */
	inCnt = 0;
	inLen = 0;
	inTMKindex = 0;
	inPINLen = 0;
	inMACLen = 0;
	
	memset(szLen, 0x00, sizeof(szLen));
	sprintf(szLen, "%x", uszUnPackBuf[1]);
	
	inLen = atoi(szLen);
	inCnt += 2;
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "Field_60 Key Exchange");
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "Length [%d][0x%02X][0x%02X]", inLen, uszUnPackBuf[0], uszUnPackBuf[1]);
		inLogPrintf(AT, szTemplate);
	}
	
	if (ginISODebug == VS_TRUE)
	{
		inPRINT_ChineseFont("Field_60 Key Exchange", _PRT_ISO_);
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "Length [%d][0x%02X][0x%02X]", inLen, uszUnPackBuf[0], uszUnPackBuf[1]);
		inPRINT_ChineseFont(szTemplate, _PRT_ISO_);
	}
	
	/* Key Set : Set = _TMK_KEYSET_*/
	/* Key Index : Index = 0x01~0x0F */
	/* mfes預設為第一把Key() */
	inTMKindex = 1;

	if (ginDebug == VS_TRUE)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "TMK Index [%d]", inTMKindex);
		inLogPrintf(AT, szTemplate);
	}

	if (ginISODebug == VS_TRUE)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "TMK Index [%d]", inTMKindex);
		inPRINT_ChineseFont(szTemplate, _PRT_ISO_);
	}
	
	while (inCnt < inLen)
	{
		switch (uszUnPackBuf[inCnt])
		{
			case 'P' :
				inCnt ++; /* Key ID */
				/*
					長度僅含Key Length，不含Chack Value)
					First sub-element length; the value of the “length”
					sub-field is always one;.
					BCD length for 範例 : 32 bytes 0x20
				*/
				
				inPINLen = uszUnPackBuf[inCnt];
				inCnt ++;			/* Sub Length */
				
				if (ginDebug == VS_TRUE)
				{
					memset(szTemplate, 0x00, sizeof(szTemplate));
					sprintf(szTemplate, "PIN KEY BCD Lenght [%d][0x%02X]", inPINLen, uszUnPackBuf[inCnt - 1]);
					inDISP_LogPrintf_Format(szTemplate, "  ", 34);
					
				}
				
				if (ginISODebug == VS_TRUE)
				{
					memset(szTemplate, 0x00, sizeof(szTemplate));
					sprintf(szTemplate, "PIN KEY BCD Lenght [%d][0x%02X]", inPINLen, uszUnPackBuf[inCnt - 1]);
					inPRINT_ChineseFont_Format(szTemplate, "  ", 34, _PRT_ISO_);
					
				}

				if (inPINLen == 32 || inPINLen == 48)
				{
					/* Unpack TPK under TMK, 16(DES)/32(2DES)/48(3DES) */
					memset(szKeyExChange, 0x00, sizeof(szKeyExChange));
					memcpy(szKeyExChange, (char *)&uszUnPackBuf[inCnt], inPINLen);
					inCnt += inPINLen;
					
					if (ginDebug == VS_TRUE)
					{
						memset(szTemplate, 0x00, sizeof(szTemplate));
						sprintf(szTemplate, "PIN KEY [%s]", szKeyExChange);
						inDISP_LogPrintf_Format(szTemplate, "  ", 34);
						
					}

					if (ginISODebug == VS_TRUE)
					{
						memset(szTemplate, 0x00, sizeof(szTemplate));
						sprintf(szTemplate, "PIN KEY [%s]", szKeyExChange);
						inPRINT_ChineseFont_Format(szTemplate, "  ", 34, _PRT_ISO_);
					}

					/* Write PINKey */
					if (inNCCC_TMK_Write_PINKey(inTMKindex, inPINLen / 2, szKeyExChange, "") != VS_SUCCESS)
						return (VS_WRITE_KEY_ERROR);
				}
				else
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "key值長度不合法:%d", inPINLen);
					}
					if (ginISODebug == VS_TRUE)
					{
						memset(szTemplate, 0x00, sizeof(szTemplate));
						sprintf(szTemplate, "key值長度不合法:%d", inPINLen);
						inPRINT_ChineseFont_Format(szTemplate, "  ", 34, _PRT_ISO_);
					}
					return (VS_ERROR);
				}

				break;
			case 'M' :
				inCnt ++; /* Key ID */
				/*
					Sub Length
					(長度僅含Key Length，不含Chack Value)
					First sub-element length; the value of the “length”
					sub-field is always one;.
					BCD length for 範例 : 32 bytes 0x20
				*/
				inMACLen = uszUnPackBuf[inCnt];
				inCnt ++;				/* Sub Length */
				
				if (ginDebug == VS_TRUE)
				{
					memset(szTemplate, 0x00, sizeof(szTemplate));
					sprintf(szTemplate, "MAC KEY BCD Lenght [%d][0x%02X]", inMACLen, uszUnPackBuf[inCnt - 1]);
					inDISP_LogPrintf_Format(szTemplate, "  ", 34);
				}
				
				if (ginISODebug == VS_TRUE)
				{
					memset(szTemplate, 0x00, sizeof(szTemplate));
					sprintf(szTemplate, "MAC KEY BCD Lenght [%d][0x%02X]", inMACLen, uszUnPackBuf[inCnt - 1]);
					inPRINT_ChineseFont_Format(szTemplate, "  ", 34, _PRT_ISO_);
				}

				if (inMACLen == 32 || inMACLen == 48)
				{
					/* Unpack TPK under TMK, 16(DES)/32(2DES)/48(3DES) */
					memset(szKeyExChange, 0x00, sizeof(szKeyExChange));
					memcpy(&szKeyExChange[0], (char *)&uszUnPackBuf[inCnt], inMACLen);
					inCnt += inMACLen;

					if (ginDebug == VS_TRUE)
					{
						memset(szTemplate, 0x00, sizeof(szTemplate));
						sprintf(szTemplate, "MAC KEY [%s]", szKeyExChange);
						inDISP_LogPrintf_Format(szTemplate, "  ", 34);
					}
					
					if (ginISODebug == VS_TRUE)
					{
						memset(szTemplate, 0x00, sizeof(szTemplate));
						sprintf(szTemplate, "MAC KEY [%s]", szKeyExChange);
						inPRINT_ChineseFont_Format(szTemplate, "  ", 34, _PRT_ISO_);
					}
                                        
                                        /* Write MAC Key */
                                        if (inNCCC_TMK_Write_MACKey(inTMKindex, inMACLen / 2, szKeyExChange, "") != VS_SUCCESS)
						return (VS_WRITE_KEY_ERROR);
				}
				else
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "key值長度不合法:%d", inPINLen);
					}
					if (ginISODebug == VS_TRUE)
					{
						memset(szTemplate, 0x00, sizeof(szTemplate));
						sprintf(szTemplate, "key值長度不合法:%d", inPINLen);
						inPRINT_ChineseFont_Format(szTemplate, "  ", 34, _PRT_ISO_);
					}
					return (VS_ERROR);
				}

				break;
			default :
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "key值內容非預設:%02X", uszUnPackBuf[inCnt]);
				}
				if (ginISODebug == VS_TRUE)
				{
					memset(szTemplate, 0x00, sizeof(szTemplate));
					sprintf(szTemplate, "key值內容非預設:%02X", uszUnPackBuf[inCnt]);
					inPRINT_ChineseFont_Format(szTemplate, "  ", 34, _PRT_ISO_);
				}
				return (VS_ERROR);
		}
	}
	
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_MFES_TMKKeyExchange() End!");
	
	return (VS_SUCCESS);
}

/*
Function        :vdNCCC_MFES_ISO_FormatDebug_DISP
Date&Time       :2016/11/30 下午 4:19
Describe        :顯示ISO Debug 
*/
void vdNCCC_MFES_ISO_FormatDebug_DISP(TRANSACTION_OBJECT *pobTran, unsigned char *uszDebugBuf, int inSendLen)
{
	int			inOneLineLen = 34;
	int			i, inPrintLineCnt = 0, inCnt = 0, inField, inLen, inFieldLen;
	int			inBitMapCnt1, inBitMapCnt2;
	char			szPrtBuf[50 + 1], szBuf[1536 + 1], szBitMap[8 + 1];
	char			szPrintLineData[36 + 1];
	unsigned char		uszBitMap[9], uszBitMapDisp[66];
	ISO_TYPE_NCCC_MFES_TABLE srISOTypeTable;

	memset((char *)&srISOTypeTable, 0x00, sizeof(srISOTypeTable));
	memcpy((char *)&srISOTypeTable, (char *)&srNCCC_MFES_ISOFunc[0], sizeof(srISOTypeTable));

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
		if (!inNCCC_MFES_BitMapCheck((unsigned char *)szBitMap, i))
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
				strcat(szBuf, "EMV ");
				inLen += 4;
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
		
		inField = inNCCC_MFES_GetFieldIndex(i, srISOTypeTable.srISOFieldType);
		inFieldLen = inNCCC_MFES_GetFieldLen(i, &uszDebugBuf[inCnt], srISOTypeTable.srISOFieldType);

		switch (srISOTypeTable.srISOFieldType[inField].inFieldType)
		{
			case _NCCC_MFES_ISO_ASC_  :
				memcpy(&szBuf[inLen], &uszDebugBuf[inCnt], inFieldLen);
				break;
                        case _NCCC_MFES_ISO_BYTE_3_  :
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
                        case _NCCC_MFES_ISO_BYTE_1_ :
                                inFunc_BCD_to_ASCII(&szBuf[inLen], &uszDebugBuf[inCnt ++], 1);
				inLen ++;
				strcat(szBuf, " ");
				inLen ++;

				inFieldLen --;
                                inFunc_BCD_to_ASCII(&szBuf[inLen], &uszDebugBuf[inCnt], inFieldLen);
				break;
			case _NCCC_MFES_ISO_NIBBLE_2_  :
			case _NCCC_MFES_ISO_BYTE_2_  :
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
			case _NCCC_MFES_ISO_BCD_  :
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

		if (i == 55 || i == 56)
			vdNCCC_MFES_ISO_FormatDebug_DISP_EMV(&uszDebugBuf[inCnt], inFieldLen);
		else if (i == 58)
			vdNCCC_MFES_ISO_FormatDebug_DISP_58(&uszDebugBuf[inCnt], inFieldLen);
		else if (i == 59)
			vdNCCC_MFES_ISO_FormatDebug_DISP_59(&uszDebugBuf[inCnt], inFieldLen);
                
		inCnt += inFieldLen;
	}

        inLogPrintf(AT, "==========================================");
}

void vdNCCC_MFES_ISO_FormatDebug_DISP_EMV(unsigned char *uszDebugBuf, int inFieldLen)
{
	int	inOneLineLen = 34;
	int	i, inTagLen;
	int	inPrintLineCnt = 0;
	char	szTag[512 + 1], szPrintTag[38 + 1];
	char	szTagData[34 + 1];

	for (i = 0; i < inFieldLen; i ++)
	{
		if ((uszDebugBuf[i] & 0x1F) == 0x1F)
		{
			/* 2-Byte Tag */
			memset(szTag, 0x00, sizeof(szTag));
			memset(szPrintTag, 0x00, sizeof(szPrintTag));
			inTagLen = uszDebugBuf[i + 2];
                        inFunc_BCD_to_ASCII(&szTag[0], &uszDebugBuf[i + 3], inTagLen);
			sprintf(&szPrintTag[0], "  Tag %02X", uszDebugBuf[i]);
			i ++;
			sprintf(&szPrintTag[8], "%02X", uszDebugBuf[i]);
			i ++;
			if (inTagLen <= 8)
			{
				sprintf(&szPrintTag[10], " (%02X = %d)(%s)", uszDebugBuf[i], inTagLen, szTag);
				inLogPrintf(AT, szPrintTag);
			}
			else
			{
				sprintf(&szPrintTag[10], " (%02X = %d)", uszDebugBuf[i], inTagLen);
				inLogPrintf(AT, szPrintTag);
				
				inPrintLineCnt = 0;
				while ((inPrintLineCnt * inOneLineLen) < strlen(szTag))
				{
					memset(szPrintTag, 0x00, sizeof(szPrintTag));
					memset(szTagData, 0x00, sizeof(szTagData));
					if (((inPrintLineCnt + 1) * inOneLineLen) > strlen(szTag))
					{
						strcat(szTagData, &szTag[inPrintLineCnt * inOneLineLen]);
					}
					else
					{
						memcpy(szTagData, &szTag[inPrintLineCnt * inOneLineLen], inOneLineLen);
					}
					sprintf(szPrintTag, "  (%s)", szTagData);
					inLogPrintf(AT, szPrintTag);
					inPrintLineCnt ++;
				}
				
			}
			i += inTagLen;
		}
		else
		{
			/* 1-Byte Tag */
			memset(szTag, 0x00, sizeof(szTag));
			memset(szPrintTag, 0x00, sizeof(szPrintTag));
			inTagLen = uszDebugBuf[i + 1];
                        inFunc_BCD_to_ASCII(&szTag[0], &uszDebugBuf[i + 2], inTagLen);
			sprintf(&szPrintTag[0], "  Tag %02X", uszDebugBuf[i]);
			i ++;
			if (inTagLen <= 8)
			{
				sprintf(&szPrintTag[8], " (%02X = %d)(%s)", uszDebugBuf[i], inTagLen, szTag);
				inLogPrintf(AT, szPrintTag);
			}
			else
			{
				sprintf(&szPrintTag[8], " (%02X = %d)", uszDebugBuf[i], inTagLen);
				inLogPrintf(AT, szPrintTag);
				
				inPrintLineCnt = 0;
				while ((inPrintLineCnt * inOneLineLen) < strlen(szTag))
				{
					memset(szPrintTag, 0x00, sizeof(szPrintTag));
					memset(szTagData, 0x00, sizeof(szTagData));
					if (((inPrintLineCnt + 1) * inOneLineLen) > strlen(szTag))
					{
						strcat(szTagData, &szTag[inPrintLineCnt * inOneLineLen]);
					}
					else
					{
						memcpy(szTagData, &szTag[inPrintLineCnt * inOneLineLen], inOneLineLen);
					}
					sprintf(szPrintTag, "  (%s)", szTagData);
					inLogPrintf(AT, szPrintTag);
					inPrintLineCnt ++;
				}
			}
			i += inTagLen;
                        
		}
		
	}
	
}

/*
Function        :vdNCCC_MFES_ISO_FormatDebug_DISP_58
Date&Time       :2017/1/25 下午 6:13
Describe        :看TMS資訊
*/
void vdNCCC_MFES_ISO_FormatDebug_DISP_58(unsigned char *uszDebugBuf, int inFieldLen)
{
	int	inCnt = 0, inTableLen;
	char	szPrintBuf[1024 + 1], szTemplate[1024 + 1];

	/* Table ID */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	szTemplate[0] = uszDebugBuf[inCnt];
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "  Table ID = %x / %s", uszDebugBuf[inCnt], szTemplate);
	inLogPrintf(AT, szPrintBuf);
	inCnt ++;
	/* Table Length */
	inTableLen = uszDebugBuf[inCnt];
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "  Table Len = %x / %d", uszDebugBuf[inCnt], inTableLen);
	inLogPrintf(AT, szPrintBuf);
	inCnt ++;
	
	/* Host Date (8 Byte) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(szTemplate, &uszDebugBuf[inCnt], 8);
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "    Host Date = %s", szTemplate);
	inLogPrintf(AT, szPrintBuf);
	inCnt += 8;
	
	/* Host Time (6 Byte) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(szTemplate, &uszDebugBuf[inCnt], 6);
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "    Host Time = %s", szTemplate);
	inLogPrintf(AT, szPrintBuf);
	inCnt += 6;
	
	/* Download Flag (1 Byte) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "    Download Flag = %s", szTemplate);
	inLogPrintf(AT, szPrintBuf);
	inCnt += 1;
	
	/* Download Scope (1 Byte) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "    Download Scope = %s", szTemplate);
	inLogPrintf(AT, szPrintBuf);
	inCnt += 1;
	
	/* Download TEL (1 Byte) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(szTemplate, &uszDebugBuf[inCnt], 15);
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "    Download TEL = %s", szTemplate);
	inLogPrintf(AT, szPrintBuf);
	inCnt += 15;
	
	/* Schedule Date (8 Byte) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(szTemplate, &uszDebugBuf[inCnt], 8);
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "    Schedule Date = %s", szTemplate);
	inLogPrintf(AT, szPrintBuf);
	inCnt += 8;
	
	/* Schedule Time (6 Byte) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(szTemplate, &uszDebugBuf[inCnt], 6);
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "    Schedule Time = %s", szTemplate);
	inLogPrintf(AT, szPrintBuf);
	inCnt += 6;
	
}

/*
Function        :vdNCCC_MFES_ISO_FormatDebug_DISP_59
Date&Time       :2016/12/27 下午 4:25
Describe        :
*/
void vdNCCC_MFES_ISO_FormatDebug_DISP_59(unsigned char *uszDebugBuf, int inFieldLen)
{
	int	inCnt = 0, inTableLen = 0;
	int	inTempLen = 0;
	int	inAwardNum = 0;
	char	szPrintBuf[1024 + 1], szTemplate[1024 + 1];

	while (inFieldLen > inCnt)
	{
		if (!memcmp(&uszDebugBuf[inCnt], "C", 1))
		{
			/* Table ID */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			szTemplate[0] = uszDebugBuf[inCnt];
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Table ID = %x / %s", uszDebugBuf[inCnt], szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt ++;
			/* Table Length */
			inTableLen = uszDebugBuf[inCnt];
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Table Len = %x / %d", uszDebugBuf[inCnt], inTableLen);
			inLogPrintf(AT, szPrintBuf);
			inCnt ++;
			/* Function Number / The Terminal Support CUP Function (1 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			szTemplate[0] = uszDebugBuf[inCnt];
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "    Function Number = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt ++;
			/* Acquire Indicator / Terminal Hot Key Control (1 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			szTemplate[0] = uszDebugBuf[inCnt];
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "    Acquire Indicator = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt ++;

			if (szTemplate[0] == '1')
			{
				/* CUP Trace Number (6 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 6);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "    CUP Trace Number = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 6;
				/* CUP Transaction Date(MMDD) (4 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 4);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "    CUP Transaction Date(MMDD) = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 4;
				/* CUP Transaction Time(hhmmss) (6 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 6);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "    CUP Transaction Time(hhmmss) = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 6;
				/* CUP Retrieve Reference Number(CRRN) (12 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 12);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "    CUP Retrieve Reference Number(CRRN) = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 12;
				/* Settlement Date(MMDD) (4 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 4);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "    Settlement Date(MMDD) = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 4;
			}
			
		}
		else if (!memcmp(&uszDebugBuf[inCnt], "D", 1))
		{
			/* Table ID */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			szTemplate[0] = uszDebugBuf[inCnt];
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Table ID = %x / %s", uszDebugBuf[inCnt], szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt ++;
			/* Table Length */
			inTableLen = uszDebugBuf[inCnt];
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Table Len = %x / %d", uszDebugBuf[inCnt], inTableLen);
			inLogPrintf(AT, szPrintBuf);
			inCnt ++;
			/* Online Rate DCC (1 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			szTemplate[0] = uszDebugBuf[inCnt];
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "   Online Rate DCC = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt ++;
		}
		else if (!memcmp(&uszDebugBuf[inCnt], "O", 1))
		{
			/* Table ID */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			szTemplate[0] = uszDebugBuf[inCnt];
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Table ID = %x / %s", uszDebugBuf[inCnt], szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt ++;
			/* Table Length */
			inTableLen = uszDebugBuf[inCnt];
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Table Len = %x / %d", uszDebugBuf[inCnt], inTableLen);
			inLogPrintf(AT, szPrintBuf);
			inCnt ++;
			/* Original Transaction Date(MMDD) (4 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 4);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "    Original Transaction Date(MMDD) = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 4;
			/* Original Amount (12 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 12);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "    Original Amount = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 12;
		}
		else if (!memcmp(&uszDebugBuf[inCnt], "Y", 1))
		{
			/* Table ID */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			szTemplate[0] = uszDebugBuf[inCnt];
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Table ID = %x / %s", uszDebugBuf[inCnt], szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt ++;
			/* Table Length */
			inTableLen = uszDebugBuf[inCnt];
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Table Len = %x / %d", uszDebugBuf[inCnt], inTableLen);
			inLogPrintf(AT, szPrintBuf);
			inCnt ++;
			/* 授權主機的西元年 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 4);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "    Host YYYY = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 4;
		}
		else if (!memcmp(&uszDebugBuf[inCnt], "N8", 2))
		{
			/* Table ID */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			szTemplate[0] = uszDebugBuf[inCnt];
			szTemplate[1] = uszDebugBuf[inCnt + 1];
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Table ID = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 2;
			/* Table Length */
			inTableLen = ((uszDebugBuf[inCnt] % 16) * 100) +
				      ((uszDebugBuf[inCnt + 1] / 16) * 10) + (uszDebugBuf[inCnt + 1] % 16);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Table Len = %d", inTableLen);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 2;
			/* 4DBC (4 bytes) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 4);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  4DBC = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 4;
		}
		else if (!memcmp(&uszDebugBuf[inCnt], "NE", 2))
		{
			/* Table ID */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			szTemplate[0] = uszDebugBuf[inCnt];
			szTemplate[1] = uszDebugBuf[inCnt + 1];
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Table ID = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 2;
			/* Table Length */
			inTableLen = ((uszDebugBuf[inCnt] % 16) * 100) +
				      ((uszDebugBuf[inCnt + 1] / 16) * 10) + (uszDebugBuf[inCnt + 1] % 16);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Table Len = %d", inTableLen);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 2;
			/* VEPS (1 bytes) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Need Upload ESC = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt ++;
		}
		else if (!memcmp(&uszDebugBuf[inCnt], "NQ", 2))
		{
			/* 免簽名需求《依照ATS、MFES授權主機回覆交易電文的Indicator判斷帳單是否列印免簽名》 add by LingHsiung 2015-07-20 上午 11:36:30 */
			/* Table ID */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			szTemplate[0] = uszDebugBuf[inCnt];
			szTemplate[1] = uszDebugBuf[inCnt + 1];
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Table ID = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 2;
			/* Table Length */
			inTableLen = ((uszDebugBuf[inCnt] % 16) * 100) +
				      ((uszDebugBuf[inCnt + 1] / 16) * 10) + (uszDebugBuf[inCnt + 1] % 16);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Table Len = %d", inTableLen);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 2;
			/* VEPS (1 bytes) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Quick Pay = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt ++;
		}
		else if (!memcmp(&uszDebugBuf[inCnt], "NC", 2))
		{
			/* Table ID */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			szTemplate[0] = uszDebugBuf[inCnt];
			szTemplate[1] = uszDebugBuf[inCnt + 1];
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Table ID = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 2;
			/* Table Length */
			inTableLen = ((uszDebugBuf[inCnt] % 16) * 100) +
				      ((uszDebugBuf[inCnt + 1] / 16) * 10) + (uszDebugBuf[inCnt + 1] % 16);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Table Len = %d", inTableLen);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 2;
			/* MCP Indicator (1 bytes) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  MCP Indicator = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt ++;
			
			/* Issue Bank ID (3 bytes) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 3);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Issue Bank ID = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 3;
			
			if (inTableLen > 4)
			{
				if (inTableLen == 7)
				{
					/* 卡別簡碼 */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "  Card SC = %s", szTemplate);
					inLogPrintf(AT, szPrintBuf);
					inCnt += 1;
					
					/* 卡別名稱長度 */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memcpy(szTemplate, &uszDebugBuf[inCnt], 2);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "  Card LabelLen = %s", szTemplate);
					inLogPrintf(AT, szPrintBuf);
					inCnt += 2;
				}
				/* 有回傳卡別名稱的情況*/
				else
				{
					/* 卡別簡碼 */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "  Card SC = %s", szTemplate);
					inLogPrintf(AT, szPrintBuf);
					inCnt += 1;
					
					/* 卡別名稱長度 */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memcpy(szTemplate, &uszDebugBuf[inCnt], 2);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "  Card LabelLen = %s", szTemplate);
					inLogPrintf(AT, szPrintBuf);
					inCnt += 2;
					
					inTempLen = atoi(szTemplate);
					/* 卡別名稱 */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "  Card LabelLen = %s", szTemplate);
					inLogPrintf(AT, szPrintBuf);
					inCnt += inTempLen;
				}
			}
		}
		else if (!memcmp(&uszDebugBuf[inCnt], "NI", 2))
		{
			/* 【需求單 - 108046】電子發票BIN大於6碼需求 Table"NI" by Russell 2019/7/8 上午 11:44 */
			/* Table ID */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			szTemplate[0] = uszDebugBuf[inCnt];
			szTemplate[1] = uszDebugBuf[inCnt + 1];
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Table ID = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 2;
			/* Table Length */
			inTableLen = ((uszDebugBuf[inCnt] % 16) * 100) +
				      ((uszDebugBuf[inCnt + 1] / 16) * 10) + (uszDebugBuf[inCnt + 1] % 16);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Table Len = %d", inTableLen);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 2;
			
			/* E-IVI Bank ID (6 bytes) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTableLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  E-IVI Bank ID = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += inTableLen;
		}
		else if (!memcmp(&uszDebugBuf[inCnt], "L1", 2))
		{
			/* Table ID */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			szTemplate[0] = uszDebugBuf[inCnt];
			szTemplate[1] = uszDebugBuf[inCnt + 1];
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Table ID = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 2;
			/* Table Length */
			inTableLen = ((uszDebugBuf[inCnt] % 16) * 100) +
				      ((uszDebugBuf[inCnt + 1] / 16) * 10) + (uszDebugBuf[inCnt + 1] % 16);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Table Len = %d", inTableLen);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 2;
			/* 列印優惠或廣告資訊之個數 (1 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			szTemplate[0] = uszDebugBuf[inCnt];
			inAwardNum = atoi(szTemplate);
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
			memcpy(szTemplate, &uszDebugBuf[inCnt], 60);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
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
			memcpy(szTemplate, &uszDebugBuf[inCnt], 50);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
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
			memcpy(szTemplate, &uszDebugBuf[inCnt], 200);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  優惠活動之內容 = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 200;
			/* 檢查碼(Checksum) (1 Byte) */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  檢查碼 = %02x", uszDebugBuf[inCnt]);
			inLogPrintf(AT, szPrintBuf);
			inCnt ++;
			
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
			memcpy(szTemplate, &uszDebugBuf[inCnt], 50);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
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
			memcpy(szTemplate, &uszDebugBuf[inCnt], 200);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  優惠活動之內容 = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 200;
			/* 檢查碼(Checksum) (1 Byte) */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  檢查碼 = %02x", uszDebugBuf[inCnt]);
			inLogPrintf(AT, szPrintBuf);
			inCnt ++;
		}
		else if (!memcmp(&uszDebugBuf[inCnt], "L2", 2))
		{
			/* Table ID */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			szTemplate[0] = uszDebugBuf[inCnt];
			szTemplate[1] = uszDebugBuf[inCnt + 1];
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Table ID = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 2;

			/* Table Length */
			inTableLen = ((uszDebugBuf[inCnt] % 16) * 100) +
				      ((uszDebugBuf[inCnt + 1] / 16) * 10) + (uszDebugBuf[inCnt + 1] % 16);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Table Len = %d", inTableLen);
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
			memcpy(szTemplate, &uszDebugBuf[inCnt], 60);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
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
			memcpy(szTemplate, &uszDebugBuf[inCnt], 50);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
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
			memcpy(szTemplate, &uszDebugBuf[inCnt], 200);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
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
			memcpy(szTemplate, &uszDebugBuf[inCnt], 50);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
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
			memcpy(szTemplate, &uszDebugBuf[inCnt], 150);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
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
		}
		else if (!memcmp(&uszDebugBuf[inCnt], "L3", 2))
		{
			/* Table ID */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			szTemplate[0] = uszDebugBuf[inCnt];
			szTemplate[1] = uszDebugBuf[inCnt + 1];
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Table ID = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 2;
			/* Table Length */
			inTableLen = ((uszDebugBuf[inCnt] % 16) * 100) +
				      ((uszDebugBuf[inCnt + 1] / 16) * 10) + (uszDebugBuf[inCnt + 1] % 16);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Table Len = %d", inTableLen);
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
			memcpy(szTemplate, &uszDebugBuf[inCnt], 20);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
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
			memcpy(szTemplate, &uszDebugBuf[inCnt], 100);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
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
			memcpy(szTemplate, &uszDebugBuf[inCnt], 100);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  核銷資訊(二) = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 100;
			/* 檢查碼(Checksum) (1 Byte) */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  檢查碼 = %02x", uszDebugBuf[inCnt]);
			inLogPrintf(AT, szPrintBuf);
			inCnt ++;
		}
		else if (!memcmp(&uszDebugBuf[inCnt], "TD", 2))
		{
			/* Table ID */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			szTemplate[0] = uszDebugBuf[inCnt];
			szTemplate[1] = uszDebugBuf[inCnt + 1];
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Table ID = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 2;
			/* Table Length */
			inTableLen = ((uszDebugBuf[inCnt] % 16) * 100) +
				      ((uszDebugBuf[inCnt + 1] / 16) * 10) + (uszDebugBuf[inCnt + 1] % 16);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Table Len = %d", inTableLen);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 2;
			/* TransactionID (15 bytes) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 15);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  TransactionID = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += 15;
			
			if ((inTableLen - 15) > 0)
			{
				/* DFS TraceNum (6 bytes) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &uszDebugBuf[inCnt], 6);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "  DFS TraceNum = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 6;
			}
		}
		/* Table 長度為1時使用 */
//		else if (!memcmp(&uszDebugBuf[inCnt], "Y", 1))
//		{
//			/* Table ID */
//			memset(szTemplate, 0x00, sizeof(szTemplate));
//			memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 1);
//			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
//			sprintf(szPrintBuf, " Table ID   [%s]", szTemplate);
//			inLogPrintf(AT, szPrintBuf);
//			inCnt += 1;
//			/* Table Length */
//			inTableLen = (uszDebugBuf[inCnt] / 16 * 10) + uszDebugBuf[inCnt] % 16;
//			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
//			sprintf(szPrintBuf, " Table Len  [%02X][%d]", uszDebugBuf[inCnt], inTableLen);
//			inLogPrintf(AT, szPrintBuf);
//			inCnt += 1;
//			if (inTableLen <= 8)
//			{
//				/* Table Data */
//				memset(szTemplate, 0x00, sizeof(szTemplate));
//				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], inTableLen);
//				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
//				sprintf(szPrintBuf, " Table Data [%s]", szTemplate);
//				inLogPrintf(AT, szPrintBuf);
//			}
//			else
//			{
//				/* Table Data */
//				memset(szTemplate, 0x00, sizeof(szTemplate));
//				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], inTableLen);
//				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
//				sprintf(szPrintBuf, " Table Data ");
//				inLogPrintf(AT, szPrintBuf);
//				
//				inPrintLineCnt = 0;
//				while ((inPrintLineCnt * inOneLineLen) < strlen(szTemplate))
//				{
//					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
//					memset(szTagData, 0x00, sizeof(szTagData));
//					if (((inPrintLineCnt + 1) * inOneLineLen) > strlen(szTemplate))
//					{
//						strcat(szTagData, &szTemplate[inPrintLineCnt * inOneLineLen]);
//					}
//					else
//					{
//						memcpy(szTagData, &szTemplate[inPrintLineCnt * inOneLineLen], inOneLineLen);
//					}
//					sprintf(szPrintBuf, "  [%s]", szTagData);
//					inLogPrintf(AT, szPrintBuf);
//					inPrintLineCnt ++;
//				}
//				
//			}
//
//			inCnt += inTableLen;
//                        inLogPrintf(AT, " -----------------------------------------");
//		}
//		/* Table 長度為2時使用 */
//		else if (!memcmp(&uszDebugBuf[inCnt], "L3", 2))
//		{
//			/* Table ID */
//			memset(szTemplate, 0x00, sizeof(szTemplate));
//			memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 2);
//			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
//			sprintf(szPrintBuf, " Table ID   [%s]", szTemplate);
//			inLogPrintf(AT, szPrintBuf);
//			inCnt += 2;
//			/* Table Length */
//			inTableLen = ((uszDebugBuf[inCnt] / 16 * 10 + uszDebugBuf[inCnt] % 16) * 100);
//			inTableLen += (uszDebugBuf[inCnt + 1] / 16 * 10) + uszDebugBuf[inCnt + 1] % 16;
//			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
//			sprintf(szPrintBuf, " Table Len  [%02X %02X][%d]", uszDebugBuf[inCnt], uszDebugBuf[inCnt + 1], inTableLen);
//			inLogPrintf(AT, szPrintBuf);
//			inCnt += 2;
//			if (inTableLen <= 8)
//			{
//				/* Table Data */
//				memset(szTemplate, 0x00, sizeof(szTemplate));
//				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], inTableLen);
//				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
//				sprintf(szPrintBuf, " Table Data [%s]", szTemplate);
//				inLogPrintf(AT, szPrintBuf);
//			}
//			else
//			{
//				/* Table Data */
//				memset(szTemplate, 0x00, sizeof(szTemplate));
//				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], inTableLen);
//				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
//				sprintf(szPrintBuf, " Table Data ");
//				inLogPrintf(AT, szPrintBuf);
//				
//				inPrintLineCnt = 0;
//				while ((inPrintLineCnt * inOneLineLen) < strlen(szTemplate))
//				{
//					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
//					memset(szTagData, 0x00, sizeof(szTagData));
//					if (((inPrintLineCnt + 1) * inOneLineLen) > strlen(szTemplate))
//					{
//						strcat(szTagData, &szTemplate[inPrintLineCnt * inOneLineLen]);
//					}
//					else
//					{
//						memcpy(szTagData, &szTemplate[inPrintLineCnt * inOneLineLen], inOneLineLen);
//					}
//					sprintf(szPrintBuf, "  [%s]", szTagData);
//					inLogPrintf(AT, szPrintBuf);
//					inPrintLineCnt ++;
//				}
//				
//			}
//
//			inCnt += inTableLen;
//                        inLogPrintf(AT, " -----------------------------------------");
//		}
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
Function        :vdNCCC_MFES_ISO_FormatDebug_PRINT
Date&Time       :2016/11/30 下午 4:20
Describe        :列印ISO Debug
*/
void vdNCCC_MFES_ISO_FormatDebug_PRINT(TRANSACTION_OBJECT *pobTran, unsigned char *uszDebugBuf, int inSendLen)
{
	int			inOneLineLen = 34;
	int			i, inPrintLineCnt = 0, inCnt = 0, inField, inLen, inFieldLen;
	int			inBitMapCnt1, inBitMapCnt2;
	char			szPrtBuf[50 + 1], szBuf[1536 + 1], szBitMap[8 + 1];
	char			szPrintLineData[36 + 1];
	unsigned char		uszBitMap[9], uszBitMapDisp[66];
	unsigned char		uszBuffer[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	BufferHandle		srBhandle;
	FONT_ATTRIB		srFont_Attrib;
	ISO_TYPE_NCCC_MFES_TABLE srISOTypeTable;

	/* 是否有列印功能 */
	if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
	{
		
	}
	else
	{
		/* 初始化 */
		inPRINT_Buffer_Initial(uszBuffer, _BUFFER_MAX_LINE_, &srFont_Attrib, &srBhandle);

		memset((char *)&srISOTypeTable, 0x00, sizeof(srISOTypeTable));
		memcpy((char *)&srISOTypeTable, (char *)&srNCCC_MFES_ISOFunc[0], sizeof(srISOTypeTable));

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
			if (!inNCCC_MFES_BitMapCheck((unsigned char *)szBitMap, i))
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
					strcat(szBuf, "EMV ");
					inLen += 4;
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

			inField = inNCCC_MFES_GetFieldIndex(i, srISOTypeTable.srISOFieldType);
			inFieldLen = inNCCC_MFES_GetFieldLen(i, &uszDebugBuf[inCnt], srISOTypeTable.srISOFieldType);

			switch (srISOTypeTable.srISOFieldType[inField].inFieldType)
			{
				case _NCCC_MFES_ISO_ASC_  :
					memcpy(&szBuf[inLen], &uszDebugBuf[inCnt], inFieldLen);
					break;
				case _NCCC_MFES_ISO_BYTE_3_  :
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
				case _NCCC_MFES_ISO_BYTE_1_ :
					inFunc_BCD_to_ASCII(&szBuf[inLen], &uszDebugBuf[inCnt ++], 1);
					inLen ++;
					strcat(szBuf, " ");
					inLen ++;

					inFieldLen --;
					inFunc_BCD_to_ASCII(&szBuf[inLen], &uszDebugBuf[inCnt], inFieldLen);
					break;
				case _NCCC_MFES_ISO_NIBBLE_2_  :
				case _NCCC_MFES_ISO_BYTE_2_  :
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
				case _NCCC_MFES_ISO_BCD_  :
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

				inPRINT_Buffer_PutIn(szPrtBuf, _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPrintLineCnt ++;
			};

			if (i == 55 || i == 56)
				vdNCCC_MFES_ISO_FormatDebug_PRINT_EMV(&uszDebugBuf[inCnt], inFieldLen, uszBuffer, &srFont_Attrib, &srBhandle);
			else if (i == 58)
				vdNCCC_MFES_ISO_FormatDebug_PRINT_58(&uszDebugBuf[inCnt], inFieldLen, uszBuffer, &srFont_Attrib, &srBhandle);
			else if (i == 59)
				vdNCCC_MFES_ISO_FormatDebug_PRINT_59(&uszDebugBuf[inCnt], inFieldLen, uszBuffer, &srFont_Attrib, &srBhandle);

			inCnt += inFieldLen;
		}

		inPRINT_Buffer_PutIn("==========================================", _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		inPRINT_Buffer_OutPut(uszBuffer, &srBhandle);
	}
}

void vdNCCC_MFES_ISO_FormatDebug_PRINT_EMV(unsigned char *uszDebugBuf, int inFieldLen, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inOneLineLen = 34;
	int	i, inTagLen;
	int	inPrintLineCnt = 0;
	char	szTag[512 + 1], szPrintTag[38 + 1];
	char	szTagData[34 + 1];

	for (i = 0; i < inFieldLen; i ++)
	{
		if ((uszDebugBuf[i] & 0x1F) == 0x1F)
		{
			/* 2-Byte Tag */
			memset(szTag, 0x00, sizeof(szTag));
			memset(szPrintTag, 0x00, sizeof(szPrintTag));
			inTagLen = uszDebugBuf[i + 2];
                        inFunc_BCD_to_ASCII(&szTag[0], &uszDebugBuf[i + 3], inTagLen);
			sprintf(&szPrintTag[0], "  Tag %02X", uszDebugBuf[i]);
			i ++;
			sprintf(&szPrintTag[8], "%02X", uszDebugBuf[i]);
			i ++;
			if (inTagLen <= 8)
			{
				sprintf(&szPrintTag[10], " (%02X = %d)(%s)", uszDebugBuf[i], inTagLen, szTag);
				inPRINT_Buffer_PutIn(szPrintTag, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			else
			{
				sprintf(&szPrintTag[10], " (%02X = %d)", uszDebugBuf[i], inTagLen);
				inPRINT_Buffer_PutIn(szPrintTag, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				
				inPrintLineCnt = 0;
				while ((inPrintLineCnt * inOneLineLen) < strlen(szTag))
				{
					memset(szPrintTag, 0x00, sizeof(szPrintTag));
					memset(szTagData, 0x00, sizeof(szTagData));
					if (((inPrintLineCnt + 1) * inOneLineLen) > strlen(szTag))
					{
						strcat(szTagData, &szTag[inPrintLineCnt * inOneLineLen]);
					}
					else
					{
						memcpy(szTagData, &szTag[inPrintLineCnt * inOneLineLen], inOneLineLen);
					}
					sprintf(szPrintTag, "  (%s)", szTagData);
					inPRINT_Buffer_PutIn(szPrintTag, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					inPrintLineCnt ++;
				}
				
			}
			i += inTagLen;
		}
		else
		{
			/* 1-Byte Tag */
			memset(szTag, 0x00, sizeof(szTag));
			memset(szPrintTag, 0x00, sizeof(szPrintTag));
			inTagLen = uszDebugBuf[i + 1];
                        inFunc_BCD_to_ASCII(&szTag[0], &uszDebugBuf[i + 2], inTagLen);
			sprintf(&szPrintTag[0], "  Tag %02X", uszDebugBuf[i]);
			i ++;
			if (inTagLen <= 8)
			{
				sprintf(&szPrintTag[8], " (%02X = %d)(%s)", uszDebugBuf[i], inTagLen, szTag);
				inPRINT_Buffer_PutIn(szPrintTag, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			else
			{
				sprintf(&szPrintTag[8], " (%02X = %d)", uszDebugBuf[i], inTagLen);
				inPRINT_Buffer_PutIn(szPrintTag, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				
				inPrintLineCnt = 0;
				while ((inPrintLineCnt * inOneLineLen) < strlen(szTag))
				{
					memset(szPrintTag, 0x00, sizeof(szPrintTag));
					memset(szTagData, 0x00, sizeof(szTagData));
					if (((inPrintLineCnt + 1) * inOneLineLen) > strlen(szTag))
					{
						strcat(szTagData, &szTag[inPrintLineCnt * inOneLineLen]);
					}
					else
					{
						memcpy(szTagData, &szTag[inPrintLineCnt * inOneLineLen], inOneLineLen);
					}
					sprintf(szPrintTag, "  (%s)", szTagData);
					inPRINT_Buffer_PutIn(szPrintTag, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					inPrintLineCnt ++;
				}
				
			}
			i += inTagLen;
                        
		}
		
	}
	
}

/*
Function        :vdNCCC_MFES_ISO_FormatDebug_PRINT_58
Date&Time       :2017/1/25 下午 5:59
Describe        :看TMS資訊
*/
void vdNCCC_MFES_ISO_FormatDebug_PRINT_58(unsigned char *uszDebugBuf, int inFieldLen, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inCnt = 0, inTableLen;
	char	szPrintBuf[1024 + 1], szTemplate[1024 + 1];

	/* Table ID */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	szTemplate[0] = uszDebugBuf[inCnt];
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "  Table ID = %x / %s", uszDebugBuf[inCnt], szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inCnt ++;
	/* Table Length */
	inTableLen = uszDebugBuf[inCnt];
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "  Table Len = %x / %d", uszDebugBuf[inCnt], inTableLen);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inCnt ++;
	
	/* Host Date (8 Byte) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(szTemplate, &uszDebugBuf[inCnt], 8);
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "    Host Date = %s", szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inCnt += 8;
	
	/* Host Time (6 Byte) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(szTemplate, &uszDebugBuf[inCnt], 6);
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "    Host Time = %s", szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inCnt += 6;
	
	/* Download Flag (1 Byte) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "    Download Flag = %s", szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inCnt += 1;
	
	/* Download Scope (1 Byte) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "    Download Scope = %s", szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inCnt += 1;
	
	/* Download TEL (1 Byte) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(szTemplate, &uszDebugBuf[inCnt], 15);
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "    Download TEL = %s", szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inCnt += 15;
	
	/* Schedule Date (8 Byte) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(szTemplate, &uszDebugBuf[inCnt], 8);
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "    Schedule Date = %s", szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inCnt += 8;
	
	/* Schedule Time (6 Byte) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(szTemplate, &uszDebugBuf[inCnt], 6);
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "    Schedule Time = %s", szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inCnt += 6;
	
}

void vdNCCC_MFES_ISO_FormatDebug_PRINT_59(unsigned char *uszDebugBuf, int inFieldLen, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inCnt = 0, inTableLen = 0;
	int	inTempLen = 0;
	char	szPrintBuf[1024 + 1], szTemplate[1024 + 1];

	while (inFieldLen > inCnt)
	{
		if (!memcmp(&uszDebugBuf[inCnt], "C", 1))
		{
			/* Table ID */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			szTemplate[0] = uszDebugBuf[inCnt];
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Table ID = %x / %s", uszDebugBuf[inCnt], szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt ++;
			/* Table Length */
			inTableLen = uszDebugBuf[inCnt];
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Table Len = %x / %d", uszDebugBuf[inCnt], inTableLen);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt ++;
			/* Function Number / The Terminal Support CUP Function (1 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			szTemplate[0] = uszDebugBuf[inCnt];
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "    Function Number = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt ++;
			/* Acquire Indicator / Terminal Hot Key Control (1 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			szTemplate[0] = uszDebugBuf[inCnt];
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "    Acquire Indicator = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt ++;

			if (szTemplate[0] == '1')
			{
				/* CUP Trace Number (6 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 6);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "    CUP Trace Number = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 6;
				/* CUP Transaction Date(MMDD) (4 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 4);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "    CUP TD(MMDD) = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 4;
				/* CUP Transaction Time(hhmmss) (6 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 6);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "    CUP TT(hhmmss) = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 6;
				/* CUP Retrieve Reference Number(CRRN) (12 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 12);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "    CUP RRN(CRRN) = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 12;
				/* Settlement Date(MMDD) (4 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 4);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "    Settlement Date(MMDD) = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 4;
			}
			
		}
		else if (!memcmp(&uszDebugBuf[inCnt], "D", 1))
		{
			/* Table ID */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			szTemplate[0] = uszDebugBuf[inCnt];
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Table ID = %x / %s", uszDebugBuf[inCnt], szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt ++;
			/* Table Length */
			inTableLen = uszDebugBuf[inCnt];
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Table Len = %x / %d", uszDebugBuf[inCnt], inTableLen);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt ++;
			/* Online Rate DCC (1 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			szTemplate[0] = uszDebugBuf[inCnt];
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "   Online Rate DCC = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt ++;
		}
		else if (!memcmp(&uszDebugBuf[inCnt], "O", 1))
		{
			/* Table ID */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			szTemplate[0] = uszDebugBuf[inCnt];
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Table ID = %x / %s", uszDebugBuf[inCnt], szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt ++;
			/* Table Length */
			inTableLen = uszDebugBuf[inCnt];
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Table Len = %x / %d", uszDebugBuf[inCnt], inTableLen);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt ++;
			/* Original Transaction Date(MMDD) (4 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 4);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "    Org TD(MMDD) =  %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 4;
			/* Original Amount (12 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 12);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "    Original Amount = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 12;
		}
		else if (!memcmp(&uszDebugBuf[inCnt], "Y", 1))
		{
			/* Table ID */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			szTemplate[0] = uszDebugBuf[inCnt];
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Table ID = %x / %s", uszDebugBuf[inCnt], szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt ++;
			/* Table Length */
			inTableLen = uszDebugBuf[inCnt];
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Table Len = %x / %d", uszDebugBuf[inCnt], inTableLen);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt ++;
			/* 授權主機的西元年 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 4);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "    Host YYYY = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 4;
		}
		else if (!memcmp(&uszDebugBuf[inCnt], "N8", 2))
		{
			/* Table ID */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			szTemplate[0] = uszDebugBuf[inCnt];
			szTemplate[1] = uszDebugBuf[inCnt + 1];
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Table ID = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 2;
			/* Table Length */
			inTableLen = ((uszDebugBuf[inCnt] % 16) * 100) +
				      ((uszDebugBuf[inCnt + 1] / 16) * 10) + (uszDebugBuf[inCnt + 1] % 16);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Table Len = %d", inTableLen);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 2;
			/* 4DBC (4 bytes) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 4);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  4DBC = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 4;
		}
		else if (!memcmp(&uszDebugBuf[inCnt], "NE", 2))
		{
			/* Table ID */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			szTemplate[0] = uszDebugBuf[inCnt];
			szTemplate[1] = uszDebugBuf[inCnt + 1];
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Table ID = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 2;
			/* Table Length */
			inTableLen = ((uszDebugBuf[inCnt] % 16) * 100) +
				      ((uszDebugBuf[inCnt + 1] / 16) * 10) + (uszDebugBuf[inCnt + 1] % 16);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Table Len = %d", inTableLen);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 2;
			/* VEPS (1 bytes) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Need Upload ESC = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt ++;
		}
		else if (!memcmp(&uszDebugBuf[inCnt], "NQ", 2))
		{
			/* 免簽名需求《依照ATS、MFES授權主機回覆交易電文的Indicator判斷帳單是否列印免簽名》 add by LingHsiung 2015-07-20 上午 11:36:30 */
			/* Table ID */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			szTemplate[0] = uszDebugBuf[inCnt];
			szTemplate[1] = uszDebugBuf[inCnt + 1];
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Table ID = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 2;
			/* Table Length */
			inTableLen = ((uszDebugBuf[inCnt] % 16) * 100) +
				      ((uszDebugBuf[inCnt + 1] / 16) * 10) + (uszDebugBuf[inCnt + 1] % 16);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Table Len = %d", inTableLen);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 2;
			/* VEPS (1 bytes) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Quick Pay = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt ++;
		}
		else if (!memcmp(&uszDebugBuf[inCnt], "NC", 2))
		{
			/* Table ID */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			szTemplate[0] = uszDebugBuf[inCnt];
			szTemplate[1] = uszDebugBuf[inCnt + 1];
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Table ID = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 2;
			/* Table Length */
			inTableLen = ((uszDebugBuf[inCnt] % 16) * 100) +
				      ((uszDebugBuf[inCnt + 1] / 16) * 10) + (uszDebugBuf[inCnt + 1] % 16);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Table Len = %d", inTableLen);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 2;
			/* MCP Indicator (1 bytes) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  MCP Indicator = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt ++;
			
			/* Issue Bank ID (3 bytes) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 3);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Issue Bank ID = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 3;
			
			if (inTableLen > 4)
			{
				if (inTableLen == 7)
				{
					/* 卡別簡碼 */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "  Card SC = %s", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					inCnt += 1;
					
					/* 卡別名稱長度 */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memcpy(szTemplate, &uszDebugBuf[inCnt], 2);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "  Card LabelLen = %s", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					inCnt += 2;
				}
				/* 有回傳卡別名稱的情況*/
				else
				{
					/* 卡別簡碼 */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memcpy(szTemplate, &uszDebugBuf[inCnt], 1);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "  Card SC = %s", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					inCnt += 1;
					
					/* 卡別名稱長度 */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memcpy(szTemplate, &uszDebugBuf[inCnt], 2);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "  Card LabelLen = %s", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					inCnt += 2;
					
					inTempLen = atoi(szTemplate);
					/* 卡別名稱 */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "  Card LabelLen = %s", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					inCnt += inTempLen;
				}
			}
		}
		else if (!memcmp(&uszDebugBuf[inCnt], "NI", 2))
		{
			/* 【需求單 - 108046】電子發票BIN大於6碼需求 Table"NI" by Russell 2019/7/8 上午 11:44 */
			/* Table ID */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			szTemplate[0] = uszDebugBuf[inCnt];
			szTemplate[1] = uszDebugBuf[inCnt + 1];
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Table ID = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 2;
			/* Table Length */
			inTableLen = ((uszDebugBuf[inCnt] % 16) * 100) +
				      ((uszDebugBuf[inCnt + 1] / 16) * 10) + (uszDebugBuf[inCnt + 1] % 16);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Table Len = %d", inTableLen);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 2;
			
			/* E-IVI Bank ID (6 bytes) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTableLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  E-IVI Bank ID = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += inTableLen;
		}
		else if (!memcmp(&uszDebugBuf[inCnt], "L1", 2))
		{
			/* Table ID */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			szTemplate[0] = uszDebugBuf[inCnt];
			szTemplate[1] = uszDebugBuf[inCnt + 1];
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Table ID = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 2;
			/* Table Length */
			inTableLen = ((uszDebugBuf[inCnt] % 16) * 100) +
				      ((uszDebugBuf[inCnt + 1] / 16) * 10) + (uszDebugBuf[inCnt + 1] % 16);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Table Len = %d", inTableLen);
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
			memcpy(szTemplate, &uszDebugBuf[inCnt], 60);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
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
			memcpy(szTemplate, &uszDebugBuf[inCnt], 50);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
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
			memcpy(szTemplate, &uszDebugBuf[inCnt], 200);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  優惠活動之內容 = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 200;
			/* 檢查碼(Checksum) (1 Byte) */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  檢查碼 = %02x", uszDebugBuf[inCnt]);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt ++;
			
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
			memcpy(szTemplate, &uszDebugBuf[inCnt], 50);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
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
			memcpy(szTemplate, &uszDebugBuf[inCnt], 200);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  優惠活動之內容 = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 200;
			/* 檢查碼(Checksum) (1 Byte) */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  檢查碼 = %02x", uszDebugBuf[inCnt]);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt ++;
		}
		else if (!memcmp(&uszDebugBuf[inCnt], "L2", 2))
		{
			/* Table ID */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			szTemplate[0] = uszDebugBuf[inCnt];
			szTemplate[1] = uszDebugBuf[inCnt + 1];
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Table ID = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 2;

			/* Table Length */
			inTableLen = ((uszDebugBuf[inCnt] % 16) * 100) +
				      ((uszDebugBuf[inCnt + 1] / 16) * 10) + (uszDebugBuf[inCnt + 1] % 16);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Table Len = %d", inTableLen);
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
			/* 補充資訊內容 (60 Byte) */ /* 2014/1/22 下午 03:26:24 add by kakab NCCC優惠平台端末機連線規格_V1_15_20140109 增大為60Bytes */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 60);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  補充資訊內容 = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
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
			memcpy(szTemplate, &uszDebugBuf[inCnt], 50);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  優惠or訊息內容 = %s", szTemplate);
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
			memcpy(szTemplate, &uszDebugBuf[inCnt], 200);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  優惠or訊息內容 = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
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
			memcpy(szTemplate, &uszDebugBuf[inCnt], 50);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  廣告標題內容 = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
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
			memcpy(szTemplate, &uszDebugBuf[inCnt], 150);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  廣告資訊內容 = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
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
			sprintf(szPrintBuf, "  QRcode內容 = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 100;
			/* 檢查碼(Checksum) (1 Byte) */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  檢查碼 = %02x", uszDebugBuf[inCnt]);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt ++;
		}
		else if (!memcmp(&uszDebugBuf[inCnt], "L3", 2))
		{
			/* Table ID */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			szTemplate[0] = uszDebugBuf[inCnt];
			szTemplate[1] = uszDebugBuf[inCnt + 1];
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Table ID = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 2;
			/* Table Length */
			inTableLen = ((uszDebugBuf[inCnt] % 16) * 100) +
				      ((uszDebugBuf[inCnt + 1] / 16) * 10) + (uszDebugBuf[inCnt + 1] % 16);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Table Len = %d", inTableLen);
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
			memcpy(szTemplate, &uszDebugBuf[inCnt], 20);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  補充資訊內容 = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
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
			sprintf(szPrintBuf, "  一維條碼(一) = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
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
			sprintf(szPrintBuf, "  一維條碼(二) = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
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
			memcpy(szTemplate, &uszDebugBuf[inCnt], 100);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  核銷資訊(一) = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
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
			memcpy(szTemplate, &uszDebugBuf[inCnt], 100);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  核銷資訊(二) = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 100;
			/* 檢查碼(Checksum) (1 Byte) */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  檢查碼 = %02x", uszDebugBuf[inCnt]);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt ++;
			
		}
		else if (!memcmp(&uszDebugBuf[inCnt], "TD", 2))
		{
			/* Table ID */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			szTemplate[0] = uszDebugBuf[inCnt];
			szTemplate[1] = uszDebugBuf[inCnt + 1];
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Table ID = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 2;
			/* Table Length */
			inTableLen = ((uszDebugBuf[inCnt] % 16) * 100) +
				      ((uszDebugBuf[inCnt + 1] / 16) * 10) + (uszDebugBuf[inCnt + 1] % 16);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  Table Len = %d", inTableLen);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 2;
			/* TransactionID (15 bytes) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], 15);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  TransactionID = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 15;
			
			if ((inTableLen - 15) > 0)
			{
				/* DFS TraceNum (6 bytes) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &uszDebugBuf[inCnt], 6);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "  DFS TraceNum = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 6;
			}
		}
		/* Table 長度為1時使用 */
//		else if (!memcmp(&uszDebugBuf[inCnt], "Y", 1))
//		{
//			/* Table ID */
//			memset(szTemplate, 0x00, sizeof(szTemplate));
//			memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 1);
//			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
//			sprintf(szPrintBuf, " Table ID   [%s]", szTemplate);
//			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
//			inCnt += 1;
//			/* Table Length */
//			inTableLen = (uszDebugBuf[inCnt] / 16 * 10) + uszDebugBuf[inCnt] % 16;
//			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
//			sprintf(szPrintBuf, " Table Len  [%02X][%d]", uszDebugBuf[inCnt], inTableLen);
//			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
//			inCnt += 1;
//			if (inTableLen <= 8)
//			{
//				/* Table Data */
//				memset(szTemplate, 0x00, sizeof(szTemplate));
//				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], inTableLen);
//				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
//				sprintf(szPrintBuf, " Table Data [%s]", szTemplate);
//				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
//			}
//			else
//			{
//				/* Table Data */
//				memset(szTemplate, 0x00, sizeof(szTemplate));
//				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], inTableLen);
//				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
//				sprintf(szPrintBuf, " Table Data ");
//				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
//				
//				inPrintLineCnt = 0;
//				while ((inPrintLineCnt * inOneLineLen) < strlen(szTemplate))
//				{
//					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
//					memset(szTagData, 0x00, sizeof(szTagData));
//					if (((inPrintLineCnt + 1) * inOneLineLen) > strlen(szTemplate))
//					{
//						strcat(szTagData, &szTemplate[inPrintLineCnt * inOneLineLen]);
//					}
//					else
//					{
//						memcpy(szTagData, &szTemplate[inPrintLineCnt * inOneLineLen], inOneLineLen);
//					}
//					sprintf(szPrintBuf, "  [%s]", szTagData);
//					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
//					inPrintLineCnt ++;
//				}
//				
//			}
//
//			inCnt += inTableLen;
//                       inPRINT_Buffer_PutIn(" -----------------------------------------, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
//		}
//		/* Table 長度為2時使用 */
//		else if (!memcmp(&uszDebugBuf[inCnt], "L3", 2))
//		{
//			/* Table ID */
//			memset(szTemplate, 0x00, sizeof(szTemplate));
//			memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 2);
//			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
//			sprintf(szPrintBuf, " Table ID   [%s]", szTemplate);
//			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
//			inCnt += 2;
//			/* Table Length */
//			inTableLen = ((uszDebugBuf[inCnt] / 16 * 10 + uszDebugBuf[inCnt] % 16) * 100);
//			inTableLen += (uszDebugBuf[inCnt + 1] / 16 * 10) + uszDebugBuf[inCnt + 1] % 16;
//			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
//			sprintf(szPrintBuf, " Table Len  [%02X %02X][%d]", uszDebugBuf[inCnt], uszDebugBuf[inCnt + 1], inTableLen);
//			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
//			inCnt += 2;
//			if (inTableLen <= 8)
//			{
//				/* Table Data */
//				memset(szTemplate, 0x00, sizeof(szTemplate));
//				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], inTableLen);
//				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
//				sprintf(szPrintBuf, " Table Data [%s]", szTemplate);
//				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
//			}
//			else
//			{
//				/* Table Data */
//				memset(szTemplate, 0x00, sizeof(szTemplate));
//				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], inTableLen);
//				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
//				sprintf(szPrintBuf, " Table Data ");
//				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
//				
//				inPrintLineCnt = 0;
//				while ((inPrintLineCnt * inOneLineLen) < strlen(szTemplate))
//				{
//					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
//					memset(szTagData, 0x00, sizeof(szTagData));
//					if (((inPrintLineCnt + 1) * inOneLineLen) > strlen(szTemplate))
//					{
//						strcat(szTagData, &szTemplate[inPrintLineCnt * inOneLineLen]);
//					}
//					else
//					{
//						memcpy(szTagData, &szTemplate[inPrintLineCnt * inOneLineLen], inOneLineLen);
//					}
//					sprintf(szPrintBuf, "  [%s]", szTagData);
//					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
//					inPrintLineCnt ++;
//				}
//				
//			}
//
//			inCnt += inTableLen;
//		inPRINT_Buffer_PutIn(" -----------------------------------------, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
//		}
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
Function        :inNCCC_MFES_OnlineAnalyseFISC
Date&Time       :2016/11/23 上午 9:59
Describe        :在這做FISC Confirm交易或reversal
 *		 MFES沒有FISC交易
*/
int inNCCC_MFES_OnlineAnalyseFISC(TRANSACTION_OBJECT *pobTran)
{
        int 	inRetVal = VS_SUCCESS;

	/* 要更新端末機的日期及時間 */
	if (inNCCC_MFES_SyncHostTerminalDateTime(pobTran) != VS_SUCCESS)
		return (VS_ERROR);
	
        if (pobTran->inTransactionResult == _TRAN_RESULT_AUTHORIZED_)
        {
		/* 授權，不用送Reversal */
		inSetSendReversalBit("N");
		if (inSaveHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
		{
			/* 鎖機 */
			inFunc_EDCLock(AT);
			
			return (VS_ERROR);
		}
		
		/* 在這裡送【FISC CONFIRM】或組【Y3 Advice】 */
		pobTran->uszFiscConfirmBit = VS_TRUE;
		/* TC_UPLOAD 要加一 */
		inNCCC_MFES_GetSTAN(pobTran);
		inRetVal = inFLOW_RunFunction(pobTran, _COMM_START_);
		
		inNCCC_MFES_SetSTAN(pobTran);
		/* 因為EMV流程會在inMCAP_ISO_BuildAndSendPacket完之後再跑OnlineAnalyse，因此要重新連一次 */
		if (inRetVal != VS_SUCCESS)
		{
			/* 失敗要存 Advice */
			if (inADVICE_SaveTop(pobTran, pobTran->srBRec.lnOrgInvNum) != VS_SUCCESS)
			{
				if (ginDebug == VS_TRUE)
					inLogPrintf(AT, "inADVICE_SaveTop() Error!");

				inFunc_EDCLock(AT);

				return (VS_ERROR);
			}
			else
			{
				if (ginDebug == VS_TRUE)
					inLogPrintf(AT, "inADVICE_SaveTop() SUCCESS!");
			}

			inRetVal = VS_SUCCESS;
		}
		/* 連線成功*/
		else
		{
			inRetVal = inNCCC_MFES_AdviceSendRecvPacket(pobTran, 1);
			/* 送TC UPLOAD或advice*/
			if (inRetVal != VS_SUCCESS)
			{
				if (ginDebug == VS_TRUE)
					inLogPrintf(AT, "inNCCC_MFES_AdviceSendRecvPacket() Error!");

				/* 失敗要存 Advice */
				if (inADVICE_SaveTop(pobTran, pobTran->srBRec.lnOrgInvNum) != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
						inLogPrintf(AT, "inADVICE_SaveTop() Error!");

					inFunc_EDCLock(AT);

					return (VS_ERROR);
				}
				else
				{
					if (ginDebug == VS_TRUE)
						inLogPrintf(AT, "inADVICE_SaveTop() SUCCESS!");

				}

				inRetVal = VS_SUCCESS;
			}
			else
			{
				if (ginDebug == VS_TRUE)
					inLogPrintf(AT, "inNCCC_MFES_AdviceSendRecvPacket() SUCCESS!");
				
				/* 在這裡把bit On起來，避免Confirm重送 */
				if (pobTran->srBRec.uszVOIDBit == VS_TRUE)
				{
					pobTran->srBRec.uszFiscVoidConfirmBit = VS_TRUE;
				}
				else
				{
					pobTran->srBRec.uszFiscConfirmBit = VS_TRUE;
				}

				inRetVal = VS_SUCCESS;
			}

		}
		
        }
        else if (pobTran->inTransactionResult == _TRAN_RESULT_CANCELLED_)
        {
		/* 已被主機拒絕，不用送Reversal */
		inSetSendReversalBit("N");
		if (inSaveHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
		{
			/* 鎖機 */
			inFunc_EDCLock(AT);
			
			return (VS_ERROR);
		}

		inNCCC_MFES_DispHostResponseCode(pobTran);
		
                inRetVal = VS_ERROR;
        }
        else
        {
		/* 不是授權也不是拒絕，送Reversal，ReversalBit不關 */
		
                inRetVal = VS_ERROR;
        }

        return (inRetVal);
}

/*
Function        :inNCCC_MFES_FuncInquiryIssueBank
Date&Time       :2018/5/29 上午 11:33
Describe        :【需求單 - 106128】行動支付標記及金融機構代碼
*/
int inNCCC_MFES_FuncInquiryIssueBank(TRANSACTION_OBJECT *pobTran)
{
	int	i = 0, inRetVal = VS_ERROR;
	int	inHDTIndex = 0;
	int	inRetryTimes = 0;
	char	szTemplate[20 + 1] = {0};
	char	szDEMOMode[2 + 1] = {0};

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_MFES_FuncInquiryIssueBank() START !");
	}
	
	memset(szDEMOMode, 0x00, sizeof(szDEMOMode));
	inGetDemoMode(szDEMOMode);
	
	inDISP_ClearAll();								/* 先清除螢幕顯示 */
	inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_);
	inDISP_PutGraphic(_MENU_CARD_INQUIRY_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);	/* <卡號查詢> */

	inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_CREDIT_NCCC_, &inHDTIndex);
	if (inLoadHDTRec(inHDTIndex) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	if (inLoadHDPTRec(inHDTIndex) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_PutGraphic(_PROCESS_, 0, _COORDINATE_Y_LINE_8_7_);			/* 處理中‧‧‧‧‧ */

	if (inFLOW_RunFunction(pobTran, _COMM_START_) != VS_SUCCESS)
	{
		pobTran->inErrorMsg = _ERROR_CODE_V3_COMM_;				/* 通訊失敗‧‧‧‧ */
		return (VS_ERROR);
	}

	pobTran->srBRec.inCode = _INQUIRY_ISSUER_BANK_;					/* 設定交易類別 */
	pobTran->inTransactionCode = _INQUIRY_ISSUER_BANK_;				/* 設定交易類別 */
	pobTran->inISOTxnCode = _INQUIRY_ISSUER_BANK_;					/* 組 ISO 電文 */

	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetInvoiceNum(szTemplate);
	pobTran->srBRec.lnOrgInvNum = atol(szTemplate);

	/* 要送 Field_37 Retrieval Reference Number */
	inFLOW_RunFunction(pobTran, _NCCC_FUNCTION_MAKE_REFNO_);

	/* 組_送_收_解 */
	inRetryTimes = 1 + 1;	/* 重試三次 = 送四次 */
	
	for (i = 0; i < inRetryTimes; i ++)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inNCCC_MFES_FuncInqueryIssueBank(CNT = %i)", i);
		}
		

		if (pobTran->uszConnectionBit != VS_TRUE)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_PROCESS_, 0, _COORDINATE_Y_LINE_8_7_);			/* 處理中‧‧‧‧‧ */

			if (inFLOW_RunFunction(pobTran, _COMM_START_) != VS_SUCCESS)
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "inNCCC_MFES_FuncInquiryIssueBank() RETRY_COMM_ERROR!!");
				}
				inRetVal = VS_ERROR;
				continue;
			}
		}

		inNCCC_MFES_GetSTAN(pobTran);

		if (memcmp(szDEMOMode, "Y", strlen("Y")) == 0)
		{
			inRetVal = inNCCC_Func_BuildAndSendPacket_Demo_Flow(pobTran);
		}
		else
		{
			inRetVal = inNCCC_MFES_SendPackRecvUnPack(pobTran);
		}
		
		inNCCC_MFES_SetSTAN(pobTran);				/* 成功或失敗 System Trace Number 都要加一 */
		if (inRetVal != VS_SUCCESS)
		{
			inCOMM_End(pobTran);
			inRetVal = VS_ERROR;
			continue;
		}
		else
		{
			if (inNCCC_MFES_CheckRespCode(pobTran) != _TRAN_RESULT_AUTHORIZED_)
			{
				inCOMM_End(pobTran);
				inRetVal = VS_ERROR;
				continue;
			}
			pobTran->inErrorMsg = _ERROR_CODE_V3_NONE_;
			inRetVal = VS_SUCCESS;
			break;
		}
	}

	/* 第一次斷線 */
	inCOMM_End(pobTran);

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_MFES_FuncInquiryIssueBank() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}
