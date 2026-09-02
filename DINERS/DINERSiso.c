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
#include "../SOURCE/FUNCTION/Sqlite.h"
#include "../SOURCE/FUNCTION/Function.h"
#include "../SOURCE/FUNCTION/FuncTable.h"
#include "../SOURCE/FUNCTION/File.h"
#include "../SOURCE/FUNCTION/Batch.h"
#include "../SOURCE/EVENT/MenuMsg.h"
#include "../SOURCE/EVENT/Flow.h"
#include "../SOURCE/FUNCTION/Accum.h"
#include "../SOURCE/FUNCTION/HDT.h"
#include "../SOURCE/FUNCTION/HDPT.h"
#include "../SOURCE/FUNCTION/CPT.h"
#include "../SOURCE/FUNCTION/CFGT.h"
#include "../SOURCE/FUNCTION/EDC.h"
#include "../SOURCE/FUNCTION/Utility.h"
#include "../SOURCE/COMM/Comm.h"
#include "../CREDIT/Creditfunc.h"
#include "DINERSiso.h"
#include "DINERSsrc.h"

unsigned char		guszDINERS_ISO_Field03[_DINERS_PCODE_SIZE_ + 1];
extern	int		ginDebug; /* Debug使用 extern */
extern	int		ginISODebug;
extern	int		ginMachineType;
extern	char		gszTermVersionID[16 + 1];
extern	unsigned char	guszEnormousNoNeedResetBit;	/* 資料庫是否須從頭找 */

ISO_FIELD_DINERS_TABLE	srDINERS_ISOFieldPack[] =
{
        {2,             inDINERS_Pack02},
        {3,             inDINERS_Pack03},
        {4,             inDINERS_Pack04},
        {11,            inDINERS_Pack11},
        {12,            inDINERS_Pack12},
        {13,            inDINERS_Pack13},
        {14,            inDINERS_Pack14},
        {22,            inDINERS_Pack22},
        {24,            inDINERS_Pack24},
        {25,            inDINERS_Pack25},
        {35,            inDINERS_Pack35},
        {37,            inDINERS_Pack37},
        {38,            inDINERS_Pack38},
        {39,            inDINERS_Pack39},
        {41,            inDINERS_Pack41},
        {42,            inDINERS_Pack42},
        {48,            inDINERS_Pack48},
        {54,            inDINERS_Pack54},
        {60,            inDINERS_Pack60},
        {62,            inDINERS_Pack62},
        {63,            inDINERS_Pack63},
        {0,             NULL},  /* 最後一組一定要放 0 */
};

ISO_FIELD_DINERS_TABLE srDINERS_ISOFieldPack_tSAM[] =
{
        {2,             inDINERS_Pack02},
        {3,             inDINERS_Pack03},
        {4,             inDINERS_Pack04},
        {11,            inDINERS_Pack11},
        {12,            inDINERS_Pack12},
        {13,            inDINERS_Pack13},
        {14,            inDINERS_Pack14},
        {22,            inDINERS_Pack22},
        {24,            inDINERS_Pack24},
        {25,            inDINERS_Pack25},
        {35,            inDINERS_Pack35},
        {37,            inDINERS_Pack37},
        {38,            inDINERS_Pack38},
        {39,            inDINERS_Pack39},
        {41,            inDINERS_Pack41},
        {42,            inDINERS_Pack42},
        {48,            inDINERS_Pack48},
        {54,            inDINERS_Pack54},
        {60,            inDINERS_Pack60},
        {62,            inDINERS_Pack62},
        {63,            inDINERS_Pack63},
        {0,             NULL},   /* 最後一組一定要放 0 */
};

ISO_FIELD_DINERS_TABLE srDINERS_ISOFieldPack_SoftWare[] =
{
        {2,             inDINERS_Pack02},
        {3,             inDINERS_Pack03},
        {4,             inDINERS_Pack04},
        {11,            inDINERS_Pack11},
        {12,            inDINERS_Pack12},
        {13,            inDINERS_Pack13},
        {14,            inDINERS_Pack14},
        {22,            inDINERS_Pack22},
        {24,            inDINERS_Pack24},
        {25,            inDINERS_Pack25},
        {35,            inDINERS_Pack35},
        {37,            inDINERS_Pack37},
        {38,            inDINERS_Pack38},
        {39,            inDINERS_Pack39},
        {41,            inDINERS_Pack41},
        {42,            inDINERS_Pack42},
        {48,            inDINERS_Pack48},
        {54,            inDINERS_Pack54},
        {60,            inDINERS_Pack60},
        {62,            inDINERS_Pack62},
        {63,            inDINERS_Pack63},
        {0,             NULL},  /* 最後一組一定要放 0 */
};

ISO_FIELD_DINERS_TABLE srDINERS_ISOFieldUnPack[] =
{
        {12,            inDINERS_UnPack12},
        {13,            inDINERS_UnPack13},
        {37,            inDINERS_UnPack37},
        {38,            inDINERS_UnPack38},
        {39,            inDINERS_UnPack39},
        {60,            inDINERS_UnPack60},
        {0,             NULL},  /* 最後一組一定要放 0 */
};

ISO_CHECK_DINERS_TABLE srDINERS_ISOFieldCheck[] =
{
        {0,             NULL},  /* 最後一組一定要放 0 */
};

ISO_FIELD_TYPE_DINERS_TABLE srDINERS_ISOFieldType[] =
{
        {2,     _DINERS_ISO_NIBBLE_2_,      VS_FALSE,       0},
        {3,     _DINERS_ISO_BCD_,           VS_FALSE,       6},
        {4,     _DINERS_ISO_BCD_,           VS_FALSE,       12},
        {11,    _DINERS_ISO_BCD_,           VS_FALSE,       6},
        {12,    _DINERS_ISO_BCD_,           VS_FALSE,       6},
        {13,    _DINERS_ISO_BCD_,           VS_FALSE,       4},
        {14,    _DINERS_ISO_BCD_,           VS_FALSE,       4},
        {22,    _DINERS_ISO_BCD_,           VS_FALSE,       4},
        {24,    _DINERS_ISO_BCD_,           VS_FALSE,       4},
        {25,    _DINERS_ISO_BCD_,           VS_FALSE,       2},
        {27,    _DINERS_ISO_BCD_,           VS_FALSE,       2},
        {35,    _DINERS_ISO_NIBBLE_2_,      VS_FALSE,       0},
        {37,    _DINERS_ISO_ASC_,           VS_FALSE,       12},
        {38,    _DINERS_ISO_ASC_,           VS_FALSE,       6},
        {39,    _DINERS_ISO_ASC_,           VS_FALSE,       2},
        {41,    _DINERS_ISO_ASC_,           VS_FALSE,       8},
        {42,    _DINERS_ISO_ASC_,           VS_FALSE,       15},
        {48,    _DINERS_ISO_BYTE_3_,        VS_TRUE,        0},
        {54,    _DINERS_ISO_BYTE_3_,        VS_TRUE,        0},
        {55,    _DINERS_ISO_BYTE_3_,        VS_FALSE,       0},
        {56,    _DINERS_ISO_BYTE_3_,        VS_FALSE,       0},
        {57,    _DINERS_ISO_BYTE_3_,        VS_FALSE,       0},
        {58,    _DINERS_ISO_BYTE_3_,        VS_TRUE,        0},
        {59,    _DINERS_ISO_BYTE_3_,        VS_TRUE,        0},
        {60,    _DINERS_ISO_BYTE_3_,        VS_TRUE,        0},
        {61,    _DINERS_ISO_BYTE_3_,        VS_TRUE,        0},
        {62,    _DINERS_ISO_BYTE_3_,        VS_TRUE,        0},
        {63,    _DINERS_ISO_BYTE_3_,        VS_TRUE,        0},
        {0,     _DINERS_ISO_BCD_,           VS_FALSE,       0},  /* 最後一組一定要放 0 */
};

int inDINERS_SALE[] = {3, 4, 11, 22, 24, 25, 35, 41, 42, 62, 0}; /* 最後一組一定要放 0!! */
int inDINERS_TIP[] = {2, 3, 4, 11, 12, 13, 14, 22, 24, 25, 38, 41, 42, 54, 60, 62, 0}; /* 最後一組一定要放 0!! */
int inDINERS_SALE_ADJUST[] = {2, 3, 4, 11, 12, 13, 14, 22, 24, 25, 37, 38, 41, 42, 60, 62, 0}; /* 最後一組一定要放 0!! */
int inDINERS_REFUND[] = {3, 4, 11, 22, 24, 25, 35, 41, 42, 62, 0}; /* 最後一組一定要放 0!! */
int inDINERS_VOID[] = {2, 3, 4, 11, 12, 13, 14, 22, 24, 25, 38, 41, 42, 62, 0}; /* 最後一組一定要放 0!! */
int inDINERS_PRE_AUTH[] = {3, 4, 11, 22, 24, 25, 35, 41, 42, 62, 0}; /* 最後一組一定要放 0!! */
int inDINERS_SALE_OFFLINE[] = {2, 3, 4, 11, 12, 13, 14, 22, 24, 25, 38, 41, 42, 62, 0}; /* 最後一組一定要放 0!! */
int inDINERS_REVERSAL[] = {2, 3, 4, 11, 14, 22, 24, 25, 41, 42, 62, 0}; /* 最後一組一定要放 0!! */
int inDINERS_SETTLE[] = {3, 11, 24, 41, 42, 60, 63, 0}; /* 最後一組一定要放 0!! */
int inDINERS_CLS_BATCH[] = {3, 11, 24, 41, 42, 60, 63, 0}; /* 最後一組一定要放 0!! */
int inDINERS_BATCH_UPLOAD[] = {2, 3, 4, 11, 12, 13, 14, 22, 24, 25, 38, 39, 41, 42, 60, 62, 0}; /* 最後一組一定要放 0!! */


BIT_MAP_DINERS_TABLE srDINERS_ISOBitMap[] =
{
        {_SALE_,		inDINERS_SALE,		"0200",         "000000"}, /* 銷售 */
        {_TIP_,                 inDINERS_TIP,           "0220",         "020000"}, /* 小費 */
	{_ADJUST_,		inDINERS_SALE_ADJUST,	"0220",         "020000"}, /* 調帳 */
	{_REFUND_,		inDINERS_REFUND,	"0200",         "200000"}, /* 退貨 */
	{_VOID_,		inDINERS_VOID,		"0200",         "020000"}, /* 取消 */
	{_PRE_AUTH_,		inDINERS_PRE_AUTH,	"0100",         "380000"}, /* 預先授權 */
	{_SALE_OFFLINE_,	inDINERS_SALE_OFFLINE,	"0220",         "000000"}, /* 交易補登 */
	{_REVERSAL_,		inDINERS_REVERSAL,	"0400",         "000000"}, /* 沖銷 */
	{_SETTLE_,		inDINERS_SETTLE,	"0500",         "920000"}, /* 結帳_1 */
	{_CLS_BATCH_,		inDINERS_CLS_BATCH,	"0500",         "960000"}, /* 結帳_2 */
        {_BATCH_UPLOAD_,	inDINERS_BATCH_UPLOAD,	"0320",         "000000"}, /* 批次上傳 */
        {_DINERS_NULL_TX_,      NULL,                   "0000",         "000000"}, /* 最後一組一定要放 TMS_NULL_TX!! */
};

ISO_TYPE_DINERS_TABLE srDINERS_ISOFunc[] =
{
        {
                srDINERS_ISOFieldPack,
                srDINERS_ISOFieldUnPack,
                srDINERS_ISOFieldCheck,
                srDINERS_ISOFieldType,
                srDINERS_ISOBitMap,
                inDINERS_ISOGetBitMapCode,
                inDINERS_ISOPackMessageType,
                inDINERS_ISOModifyBitMap,
                inDINERS_ISOModifyPackData,
                inDINERS_ISOCheckHeader,
                inDINERS_ISOOnlineAnalyse,
                inDINERS_ISOAdviceAnalyse
        },

        {
                srDINERS_ISOFieldPack_tSAM,
                srDINERS_ISOFieldUnPack,
                srDINERS_ISOFieldCheck,
                srDINERS_ISOFieldType,
                srDINERS_ISOBitMap,
                inDINERS_ISOGetBitMapCode,
                inDINERS_ISOPackMessageType,
                inDINERS_ISOModifyBitMap,
                inDINERS_ISOModifyPackData,
                inDINERS_ISOCheckHeader,
                inDINERS_ISOOnlineAnalyse,
                inDINERS_ISOAdviceAnalyse
        },

        {
                srDINERS_ISOFieldPack_SoftWare,
                srDINERS_ISOFieldUnPack,
                srDINERS_ISOFieldCheck,
                srDINERS_ISOFieldType,
                srDINERS_ISOBitMap,
                inDINERS_ISOGetBitMapCode,
                inDINERS_ISOPackMessageType,
                inDINERS_ISOModifyBitMap,
                inDINERS_ISOModifyPackData,
                inDINERS_ISOCheckHeader,
                inDINERS_ISOOnlineAnalyse,
                inDINERS_ISOAdviceAnalyse
        }
};

int inDINERS_Func_SetTxnOnlineOffline(TRANSACTION_OBJECT *pobTran)
{
        if (pobTran->inTransactionCode == _SALE_OFFLINE_)
        {
                pobTran->srBRec.uszOfflineBit = VS_TRUE;	/* 改成離線交易 */

                pobTran->srBRec.uszUpload1Bit = VS_TRUE;	/* 交易補登【ADVICE】未上傳 */
                pobTran->srBRec.uszUpload2Bit = VS_FALSE;
                pobTran->srBRec.uszUpload3Bit = VS_FALSE;
        }
        else if (pobTran->inTransactionCode == _TIP_)
        {
                if (pobTran->srBRec.inOrgCode == _SALE_ || pobTran->srBRec.inOrgCode == _SALE_OFFLINE_)
                {
                        pobTran->srBRec.uszOfflineBit = VS_TRUE;	/* 改成離線交易 */

                        if (pobTran->srBRec.uszUpload1Bit == VS_TRUE)
                        {
                                pobTran->srBRec.uszUpload1Bit = VS_TRUE;	/* 原交易【ADVICE】未上傳 */
                                pobTran->srBRec.uszUpload2Bit = VS_TRUE;	/* 小費交易以【ADVICE】處理 */
                                pobTran->srBRec.uszUpload3Bit = VS_FALSE;
                        }
                        else
                        {
                                pobTran->srBRec.uszUpload1Bit = VS_FALSE;	/* 原交易上傳 */
                                pobTran->srBRec.uszUpload2Bit = VS_TRUE;	/* 小費交易以【ADVICE】處理 */
                                pobTran->srBRec.uszUpload3Bit = VS_FALSE;
                        }
                }
                else
                {
                        return (VS_ERROR);
                }
        }
        else if (pobTran->inTransactionCode == _ADJUST_)
        {
                if (pobTran->srBRec.inOrgCode == _SALE_ || pobTran->srBRec.inOrgCode == _SALE_OFFLINE_)
                {
                        pobTran->srBRec.uszOfflineBit = VS_TRUE;	/* 改成離線交易 */

                        if (pobTran->srBRec.uszUpload1Bit == VS_TRUE)
                        {
                                pobTran->srBRec.uszUpload1Bit = VS_TRUE;	/* 原交易【ADVICE】未上傳 */
                                pobTran->srBRec.uszUpload2Bit = VS_TRUE;	/* 調帳交易以【ADVICE】處理 */
                                pobTran->srBRec.uszUpload3Bit = VS_FALSE;
                        }
                        else
                        {
                                pobTran->srBRec.uszUpload1Bit = VS_FALSE;	/* 原交易上傳 */
                                pobTran->srBRec.uszUpload2Bit = VS_TRUE;	/* 調帳交易以【ADVICE】處理 */
                                pobTran->srBRec.uszUpload3Bit = VS_FALSE;
                        }
                }
                else
                {
                        return (VS_ERROR);
                }
        }
        else if (pobTran->inTransactionCode == _VOID_)
        {
                if (pobTran->srBRec.inOrgCode == _SALE_OFFLINE_)
                {
                        if (pobTran->srBRec.uszUpload1Bit == VS_TRUE)
                        {
                                pobTran->srBRec.uszOfflineBit = VS_TRUE;	/* 改成離線交易 */

                                pobTran->srBRec.uszUpload1Bit = VS_TRUE;	/* 交易補登【ADVICE】還未上傳 */
                                pobTran->srBRec.uszUpload2Bit = VS_TRUE;	/* 取消交易要以【ADVICE】處理 */
                                pobTran->srBRec.uszUpload3Bit = VS_FALSE;
                        }
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
                        if (pobTran->srBRec.uszUpload1Bit == VS_TRUE && pobTran->srBRec.uszUpload2Bit == VS_TRUE)
                        {
                                pobTran->srBRec.uszOfflineBit = VS_TRUE;	/* 改成離線交易 */

                                pobTran->srBRec.uszUpload1Bit = VS_TRUE;	/* 原交易【ADVICE】還未上傳 */
                                pobTran->srBRec.uszUpload2Bit = VS_TRUE;	/* 小費交易【ADVICE】還未上傳 */
                                pobTran->srBRec.uszUpload3Bit = VS_TRUE;	/* 取消-小費交易要以【ADVICE】處理 */
                        }
                        else if (pobTran->srBRec.uszUpload1Bit == VS_FALSE && pobTran->srBRec.uszUpload2Bit == VS_TRUE)
                        {
                                pobTran->srBRec.uszOfflineBit = VS_TRUE; 	/* 改成離線交易 */

                                pobTran->srBRec.uszUpload1Bit = VS_FALSE;	/* 原交易【ADVICE】已上傳 */
                                pobTran->srBRec.uszUpload2Bit = VS_TRUE;	/* 小費交易【ADVICE】還未上傳 */
                                pobTran->srBRec.uszUpload3Bit = VS_TRUE;	/* 取消-小費交易要以【ADVICE】處理 */
                        }
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
                        if (pobTran->srBRec.uszUpload1Bit == VS_TRUE && pobTran->srBRec.uszUpload2Bit == VS_TRUE)
                        {
                                pobTran->srBRec.uszOfflineBit = VS_TRUE;	/* 改成離線交易 */

                                pobTran->srBRec.uszUpload1Bit = VS_TRUE;	/* 原交易【ADVICE】還未上傳 */
                                pobTran->srBRec.uszUpload2Bit = VS_TRUE;	/* 調帳交易【ADVICE】還未上傳 */
                                pobTran->srBRec.uszUpload3Bit = VS_TRUE;	/* 取消-小費交易要以【ADVICE】處理 */
                        }
                        else if (pobTran->srBRec.uszUpload1Bit == VS_FALSE && pobTran->srBRec.uszUpload2Bit == VS_TRUE)
                        {
                                pobTran->srBRec.uszOfflineBit = VS_TRUE; 	/* 改成離線交易 */

                                pobTran->srBRec.uszUpload1Bit = VS_FALSE;	/* 原交易【ADVICE】已上傳 */
                                pobTran->srBRec.uszUpload2Bit = VS_TRUE;	/* 調帳交易【ADVICE】還未上傳 */
                                pobTran->srBRec.uszUpload3Bit = VS_TRUE;	/* 取消-小費交易要以【ADVICE】處理 */
                        }
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
        }
        else
        {
                /* 開始預設值 */
                pobTran->srBRec.uszOfflineBit = VS_FALSE; /* 當筆交易要 Online */
                pobTran->srBRec.uszUpload1Bit = VS_FALSE; /* 不會產生任何 Advice */
                pobTran->srBRec.uszUpload2Bit = VS_FALSE; /* 不會產生任何 Advice */
                pobTran->srBRec.uszUpload3Bit = VS_FALSE; /* 不會產生任何 Advice */
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
        }

        return (VS_SUCCESS);
}

int inDINERS_Func_BuildAndSendPacket(TRANSACTION_OBJECT *pobTran)
{
        int     inRetVal = VS_ERROR;
	int	inBatchCnt = 0;
	char	szDemoMode[2 + 1] = {0};
	char	szTemplate[42 + 1] = {0}, szTemplate1[42 + 1] = {0}, szTemplate2[42 + 1] = {0};
	
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inDINERS_Func_BuildAndSendPacket() START!");
	
	/* 教育訓練模式 */
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{	
		if (pobTran->inTransactionCode == _INST_SALE_)
		{
			if (pobTran->srBRec.lnInstallmentPeriod == 0)
			{
				/* 聯合分期付款_頭期款 */
				pobTran->srBRec.lnInstallmentDownPayment = pobTran->srBRec.lnTxnAmount;
				pobTran->srBRec.lnInstallmentPayment = 0L; /* 聯合分期付款_每期款 */
			}
			else
			{
				/* 聯合分期付款_頭期款 */
				pobTran->srBRec.lnInstallmentDownPayment = (pobTran->srBRec.lnTxnAmount / pobTran->srBRec.lnInstallmentPeriod);
				/* 聯合分期付款_每期款 */
				pobTran->srBRec.lnInstallmentPayment = pobTran->srBRec.lnInstallmentDownPayment;
				/* 餘數加到首期 */
				pobTran->srBRec.lnInstallmentDownPayment += (pobTran->srBRec.lnTxnAmount % pobTran->srBRec.lnInstallmentPeriod);
			}

			pobTran->srBRec.lnInstallmentFormalityFee = 0L; /* 聯合分期付款_手續費 */

		}
		else if (pobTran->inTransactionCode == _REDEEM_SALE_)
		{
			/* 聯合紅利扣抵_扣抵紅利點數 */
			pobTran->srBRec.lnRedemptionPoints = (pobTran->srBRec.lnTxnAmount / 10);
			/* 聯合紅利扣抵_支付金額 */
			pobTran->srBRec.lnRedemptionPaidCreditAmount = (pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnRedemptionPoints);
			/* 聯合紅利扣抵_剩餘紅利點數 */
			pobTran->srBRec.lnRedemptionPointsBalance = (pobTran->srBRec.lnTxnAmount * 999);
			if (pobTran->srBRec.lnRedemptionPointsBalance > 9999999)
				pobTran->srBRec.lnRedemptionPointsBalance = 9999999;
		}

		/* 授權碼 = Batch Number + Invoice Number */
		if (pobTran->srBRec.uszOfflineBit == VS_FALSE)
		{
			if (strlen(pobTran->srBRec.szAuthCode) > 0)
			{

			}
			else
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				memset(szTemplate2, 0x00, sizeof(szTemplate2));

				inGetBatchNum(szTemplate1);
				inGetInvoiceNum(szTemplate2);

				sprintf(szTemplate, "%03d%03d", atoi(szTemplate1), atoi(szTemplate2));
				strcpy(pobTran->srBRec.szAuthCode, szTemplate);
			}

		}

		/* 主機回應碼 */
		strcpy(pobTran->srBRec.szRespCode, "00");

		memset(pobTran->srBRec.szRefNo, 0x00, sizeof(pobTran->srBRec.szRefNo));
		strcpy(pobTran->srBRec.szRefNo, "9");

		memset(szTemplate, 0x00, 12);
		inGetTerminalID(szTemplate);
		memcpy(&pobTran->srBRec.szRefNo[1], &szTemplate[3], 5);

		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetBatchNum(szTemplate);
		sprintf(szTemplate, "%ld", pobTran->srBRec.lnBatchNum);
		memcpy(&pobTran->srBRec.szRefNo[6], &szTemplate[4], 3);

		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetInvoiceNum(szTemplate);
		sprintf(szTemplate, "%ld", pobTran->srBRec.lnOrgInvNum);
		memcpy(&pobTran->srBRec.szRefNo[8], &szTemplate[3], 3);

		memset(&pobTran->srBRec.szRefNo[11], 0x20, 1);

		inDINERS_SetSTAN(pobTran);
		
		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "inDINERS_Func_BuildAndSendPacket() END!");

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
				inDINERS_SetMustSettleBit(pobTran, "Y");
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
		if (inDINERS_GetSTAN(pobTran) == VS_ERROR)
			return (VS_ERROR);

		/* 步驟 1.1 處理online交易 */
		if (pobTran->srBRec.uszOfflineBit == VS_FALSE)
		{
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

					return (VS_ERROR);
				}
			}

			/* 步驟 1.3 檢查是否為 Online 交易，先送上筆交易失敗的 Reversal 及產生當筆交易 Reversal */
			if ((inRetVal = inDINERS_ProcessReversal(pobTran)) != VS_SUCCESS)
			{
				return (VS_ERROR);
			}

			/* 步驟 1.4 */
			if (pobTran->inTransactionCode == _SETTLE_)
			{
				if (inDINERS_ProcessAdvice(pobTran) != VS_SUCCESS)
				{
					return (VS_ERROR);
				}

			}
		}

		/* 處理【ONLINE】交易 */
		if (pobTran->srBRec.uszOfflineBit == VS_FALSE)
		{
			if ((inRetVal = inDINERS_ProcessOnline(pobTran)) != VS_SUCCESS)
			{
				inDINERS_SetSTAN(pobTran); /* 成功或失敗 System_Trace_Number 都要加一 */

				return (VS_ERROR); /* Return 【VS_ERROR】【VS_SUCCESS】 */
			}
		}
		else
		{
			/* 處理【OFFLINE】交易 */
			inRetVal = inDINERS_ProcessOffline(pobTran);
			if (inRetVal != VS_SUCCESS)
				return (inRetVal);
		}

		inDINERS_SetSTAN(pobTran); /* 成功或失敗 System_Trace_Number 都要加一 */

		/* 1. 表示有收到完整的資料後的分析
		   2. 處理 Online & Offline 交易 */
		inRetVal = inDINERS_AnalysePacket(pobTran);
		if (inRetVal != VS_SUCCESS)
		{
			return (inRetVal);
		}

		/* 避免撥接太久，收送完就斷線 */
		inCOMM_End(pobTran);

		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "inDINERS_Func_BuildAndSendPacket() END!");

		return (VS_SUCCESS);
	}
}

int inDINERS_Pack02(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int		inCnt = 0, inPANLen;
        char		szPAN[40 + 1];
        unsigned char	uszBCD[20 + 1];

        memset(szPAN, 0x00, sizeof(szPAN));
        inPANLen = strlen(pobTran->srBRec.szPAN);
        uszPackBuf[inCnt++] = (inPANLen / 10 * 16) + (inPANLen % 10);
        memcpy(szPAN, pobTran->srBRec.szPAN, inPANLen);
        if (inPANLen % 2)
                szPAN[inPANLen++] = '0';

        memset(uszBCD, 0x00, sizeof(uszBCD));
        inFunc_ASCII_to_BCD(&uszBCD[0], &szPAN[0], inPANLen/2);
        memcpy((char *)&uszPackBuf[inCnt], (char *)&uszBCD[0], (inPANLen / 2));

        inCnt += (inPANLen / 2);

        return (inCnt);
}

int inDINERS_Pack03(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int     inCnt = 0;

        memcpy(&uszPackBuf[inCnt], &guszDINERS_ISO_Field03[0], _DINERS_PCODE_SIZE_);
        inCnt += _DINERS_PCODE_SIZE_;

        return (inCnt);
}

int inDINERS_Pack04(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int     inCnt = 0;
        char    szTemplate[12 + 1];

        memset(szTemplate, 0x00, sizeof(szTemplate));

        switch (pobTran->inISOTxnCode)
        {
                case _TIP_:
                case _BATCH_UPLOAD_:
                        sprintf(szTemplate, "%010ld00", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
                        break;
                case _SALE_OFFLINE_:
                        if (pobTran->srBRec.lnAdjustTxnAmount != 0)
                        {
				 /* Add by li for 由於輸入調帳金額時會影響到儲存原交易金額的變數內容 所以需特別處理才能在送SALE_OFFLINE封包時帶原交易金額 20150210 */
                                sprintf(szTemplate, "%010ld00", pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnAdjustTxnAmount);
                        }
                        else
                        {
                                sprintf(szTemplate, "%010ld00", pobTran->srBRec.lnTxnAmount);
                        }
                        break;
                default:
                        sprintf(szTemplate, "%010ld00", pobTran->srBRec.lnTxnAmount);
                        break;
        }

        inFunc_ASCII_to_BCD( &uszPackBuf[inCnt], szTemplate, 6);
        inCnt += 6;

        return (inCnt);
}

int inDINERS_Pack11(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int		inCnt = 0;
        char		szSTAN[6 + 1];
        unsigned char	uszBCD[20 + 1];

        memset(szSTAN, 0x00, sizeof(szSTAN));
        sprintf(szSTAN, "%06ld", pobTran->srBRec.lnSTANNum);

        memset(uszBCD, 0x00, sizeof(uszBCD));
        inFunc_ASCII_to_BCD (&uszBCD[0], &szSTAN[0], 6);
        memcpy((char *)&uszPackBuf[inCnt], (char *)&uszBCD[0], 3);
        inCnt += 3;

        return (inCnt);
}

int inDINERS_Pack12(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int		inCnt = 0;
        unsigned char	uszBCD[20 + 1];

        memset(uszBCD, 0x00, sizeof(uszBCD));
        inFunc_ASCII_to_BCD (&uszBCD[0], &pobTran->srBRec.szTime[0], 6);
        memcpy((char *)&uszPackBuf[inCnt], (char *)&uszBCD[0], 3);
        inCnt += 3;

        return (inCnt);
}

int inDINERS_Pack13(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int		inCnt = 0;
        unsigned char	uszBCD[20 + 1];

        memset(uszBCD, 0x00, sizeof(uszBCD));
        inFunc_ASCII_to_BCD (&uszBCD[0], &pobTran->srBRec.szDate[4], 2);
        memcpy((char *)&uszPackBuf[inCnt], (char *)&uszBCD[0], 2);
        inCnt += 2;

        return (inCnt);
}

int inDINERS_Pack14(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int		inCnt = 0;
        unsigned char	uszBCD[20 + 1];

        memset(uszBCD, 0x00, sizeof(uszBCD));
        inFunc_ASCII_to_BCD(&uszBCD[0], &pobTran->srBRec.szExpDate[0], 2);
        memcpy((char *)&uszPackBuf[inCnt], (char *)&uszBCD[0], 2);
        inCnt += 2;

        return (inCnt);
}

int inDINERS_Pack22(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int		inCnt = 0;
        unsigned char	uszBCD[20 + 1];

        if (pobTran->srBRec.uszManualBit == VS_TRUE)
                inFunc_ASCII_to_BCD(&uszBCD[0], (char*)"0012", 4);
        else
                inFunc_ASCII_to_BCD(&uszBCD[0], (char*)"0022", 4);

        memcpy((char *)&uszPackBuf[inCnt], (char *)&uszBCD[0], 2);

        inCnt += 2;

        return (inCnt);
}

int inDINERS_Pack24(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int		inCnt = 0;
        char		szTemplate[10 + 1];
        unsigned char	uszBCD[10 + 1];

        memset(szTemplate, 0x00, sizeof(szTemplate));
        if (inGetNII(szTemplate) == VS_ERROR)
                return (VS_ERROR);

        memset(uszBCD, 0x00, sizeof(uszBCD));
        inFunc_ASCII_to_BCD (&uszBCD[0], &szTemplate[0], 4);
        memcpy((char *)&uszPackBuf[inCnt], (char *)&uszBCD[0], 2);
        inCnt += 2;

        return (inCnt);
}

int inDINERS_Pack25(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int     inCnt = 0;

        if (pobTran->srBRec.uszReferralBit == VS_TRUE)
                inFunc_ASCII_to_BCD((unsigned char*)&uszPackBuf[inCnt], (char *) "06", 1);
        else
                inFunc_ASCII_to_BCD((unsigned char*)&uszPackBuf[inCnt], (char *) "00", 1);
        inCnt += 1;

        return (inCnt);
}

int inDINERS_Pack35(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int     inCnt = 0, i;
        char    szT2Data[38 + 1];
	short   shLen;

        memset(szT2Data, 0x00, sizeof(szT2Data));

        switch (pobTran->inISOTxnCode)
        {
                case _TIP_:
                case _SALE_OFFLINE_:
                case _REVERSAL_:
                        strcpy(szT2Data, pobTran->srBRec.szPAN);
                        strcat(szT2Data, "D");
                        strcat(szT2Data, pobTran->srBRec.szExpDate);
                        strcat(szT2Data, pobTran->srBRec.szServiceCode);
                        break;
                default:
                        if (pobTran->srBRec.uszManualBit == VS_TRUE)
                        {
                                strcpy(szT2Data, pobTran->srBRec.szPAN);
                                strcat(szT2Data, "D");
                                strcat(szT2Data, pobTran->srBRec.szExpDate);
                                strcat(szT2Data, pobTran->srBRec.szServiceCode);
                        }
                        else
                        {
                                strcpy(szT2Data, pobTran->szTrack2);

                                for (i = 0; i < strlen(szT2Data); i++)
                                {
                                        if (szT2Data[i] == '=')
                                                szT2Data[i] = 'D';
                                }
                        }

                        break;
        } /* End switch () ... */

        shLen = strlen(szT2Data);
        uszPackBuf[inCnt++] = (shLen / 10 * 16) + (shLen % 10);

        if (shLen % 2)
        {
                shLen++;
                strcat(szT2Data, "0");
        }

        inFunc_ASCII_to_BCD((unsigned char*)&szT2Data[0], (char *) &uszPackBuf[inCnt], (shLen / 2));
        inCnt += (shLen / 2);

        return (inCnt);
}

int inDINERS_Pack37(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int     inCnt = 0;

        memcpy((char *) &uszPackBuf[inCnt], &pobTran->srBRec.szRefNo[0], 12);
        inCnt += 12;

        return (inCnt);
}

int inDINERS_Pack38(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int     inCnt = 0;
        char 	szAuthCode[_AUTH_CODE_SIZE_ + 1];


        /* 確認是否授權碼是合法字元 */
        if (inFunc_Check_AuthCode_Validate(pobTran->srBRec.szAuthCode) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

        if (!memcmp(pobTran->srBRec.szAuthCode, "000000", 6))
        {
                return (VS_ERROR);
        }

        memset(szAuthCode, 0x00, sizeof(szAuthCode));
        strcpy(szAuthCode, pobTran->srBRec.szAuthCode);
        inFunc_PAD_ASCII(szAuthCode, szAuthCode, ' ', 6, _PADDING_RIGHT_);
        memcpy((char *) &uszPackBuf[inCnt], &szAuthCode[0], 6);
        inCnt += 6;

        return (inCnt);
}

int inDINERS_Pack39(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int     inCnt = 0;

        memcpy((char *) &uszPackBuf[inCnt], &pobTran->srBRec.szRespCode[0], 2);
        inCnt += 2;

        return (inCnt);
}

int inDINERS_Pack41(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int     inCnt = 0;
        char    szTid[8 + 1];

        memset(szTid, 0x00, sizeof(szTid));
        if (inGetTerminalID(szTid) == VS_ERROR)
                return (VS_ERROR);
        memcpy((char *) &uszPackBuf[inCnt], szTid, strlen(szTid));
        inCnt += strlen(szTid);

        return (inCnt);
}

int inDINERS_Pack42(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int     inCnt = 0;
        char    szMid[15 + 1];

        memset(szMid, 0x00, sizeof(szMid));
        if (inGetMerchantID(szMid) == VS_ERROR)
                return (VS_ERROR);
	inFunc_PAD_ASCII(szMid, szMid, ' ', 15, _PADDING_RIGHT_);
        memcpy((char *) &uszPackBuf[inCnt], szMid, strlen(szMid));
        inCnt += strlen(szMid);

        return (inCnt);
}

int inDINERS_Pack48(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	return (VS_SUCCESS);	
}

int inDINERS_Pack54(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int     inCnt = 0;

        uszPackBuf[inCnt++] = 0x00;
        uszPackBuf[inCnt++] = 0x12;
        sprintf((char *) &uszPackBuf[inCnt], "%010ld00", (long) pobTran->srBRec.lnTipTxnAmount);
        inCnt += 12;

        return (inCnt);
}

int inDINERS_Pack60(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int     inCnt = 0;
        char    szMsgTypeId[4 + 1];
        char    szScratch[23];
        char    szBatchNum[6+1];

        if (pobTran->inISOTxnCode == _BATCH_UPLOAD_)
        {
                memset(szMsgTypeId, 0x00, sizeof(szMsgTypeId));
                memset(szScratch, 0x00, sizeof(szScratch));

                switch (pobTran->srBRec.inCode)
                {
                        case _SALE_:
                        case _REFUND_:
                                memcpy(szMsgTypeId, "\x02\x00", 2);
                                break;
                        case _TIP_:
                        case _SALE_OFFLINE_:
                                memcpy(szMsgTypeId, "\x02\x20", 2);
                                break;
                        default:
                                break;
                }

                sprintf(szScratch, "%02x%02x%06ld%12s", szMsgTypeId[0], szMsgTypeId[1], pobTran->srBRec.lnSTANNum, pobTran->srBRec.szRefNo);
                inFunc_PAD_ASCII(szScratch, szScratch, ' ', 22, _PADDING_RIGHT_);

                uszPackBuf[inCnt++] = 0x00;
                uszPackBuf[inCnt++] = 0x22;
                memcpy((char *) &uszPackBuf[inCnt], szScratch, 22);
                inCnt += 22;
        }
        else
        {
                if (pobTran->inISOTxnCode == _TIP_)
                {
                        /* 小費原始交易 */
                        uszPackBuf[inCnt++] = 0x00;
                        uszPackBuf[inCnt++] = 0x12;
                        sprintf((char *) &uszPackBuf[inCnt], "%010ld00", pobTran->srBRec.lnTxnAmount);
                        inCnt += 12;
                }
                else if (pobTran->inISOTxnCode == _ADJUST_)
                {
                        /* 調帳原始交易 */
                        uszPackBuf[inCnt++] = 0x00;
                        uszPackBuf[inCnt++] = 0x12;
                        sprintf((char *) &uszPackBuf[inCnt], "%010ld00", pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnAdjustTxnAmount);
                        inCnt += 12;
                }
                else
                {
                        uszPackBuf[inCnt++] = 0x00;
                        uszPackBuf[inCnt++] = 0x06;
                        memset(szBatchNum,0x00,sizeof(szBatchNum));
                        if (inGetBatchNum(szBatchNum) == VS_ERROR)
                                return (VS_ERROR);
                        memcpy((char *) &uszPackBuf[inCnt],szBatchNum,6);
                        inCnt += 6;
                }
        }

        return (inCnt);
}

int inDINERS_Pack62(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int     inCnt = 0;

        uszPackBuf[inCnt++] = 0x00;
        uszPackBuf[inCnt++] = 0x06;
        sprintf((char *) &uszPackBuf[inCnt], "%06ld", pobTran->srBRec.lnOrgInvNum);
        inCnt += 6;

        return (inCnt);
}

int inDINERS_Pack63(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int		inCnt = 0;
        int		inAccumCnt, inBatchCnt = 0, inPacketCnt = 0;
        char		szASCII[4 + 1], szBCD[2 + 1];
        char		szTemplate[100], szPacket[100 + 1];
        ACCUM_TOTAL_REC srAccumRec;

        if (pobTran->inTransactionCode == _SETTLE_ || pobTran->inTransactionCode == _CLS_BATCH_)
        {
                memset(szPacket, 0x00, sizeof(szPacket));
/*
                inBatchCnt = inBatch_CheckBatchKeyFile(pobTran);
*/
                if (inBatchCnt == VS_ERROR)
                        return (VS_ERROR);
                else if (inBatchCnt == VS_NO_RECORD)
                {
                        strcpy(szPacket, "000000000000000");
                        strcat(szPacket, "000000000000000");
                        inPacketCnt += 30;
                }
                else
                {
                        memset(&srAccumRec, 0x00, sizeof(srAccumRec));
                        inAccumCnt = inACCUM_GetRecord(pobTran, &srAccumRec);
                        if (inAccumCnt == VS_NO_RECORD)
                        {
                                strcpy(szPacket, "000000000000000");
                                strcat(szPacket, "000000000000000");
                                inPacketCnt += 30;
                        }
                        else if (inAccumCnt == VS_ERROR)
                                return (VS_ERROR);
                        else
                        {
                                /* SALE */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                sprintf(szTemplate, "%03lu", srAccumRec.lnTotalSaleCount);
                                strcpy(szPacket, szTemplate);
                                inPacketCnt += 3;
                                sprintf(szTemplate, "%010lld00", (srAccumRec.llTotalSaleAmount + srAccumRec.llTotalTipsAmount));
                                strcat(szPacket, szTemplate);
                                inPacketCnt += 12;
                                /* REFUND */
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
        }
        else
        {
                return (VS_ERROR);
        }

        /* Packet Data Length */
        memset(szASCII, 0x00, sizeof(szASCII));
        sprintf(szASCII, "%04d", inPacketCnt);
        memset(szBCD, 0x00, sizeof(szBCD));
        inFunc_ASCII_to_BCD((unsigned char*)szASCII, szBCD, 2);
        memcpy((char *) &uszPackBuf[inCnt], &szBCD[0], 2);
        inCnt += 2;
        /* Packet Data */
        memcpy((char *) &uszPackBuf[inCnt], &szPacket[0], inPacketCnt);
        inCnt += inPacketCnt;

        return (inCnt);
}

int inDINERS_UnPack12(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
        memset(pobTran->srBRec.szTime, 0x00, sizeof(pobTran->srBRec.szTime));
        inFunc_BCD_to_ASCII(&pobTran->srBRec.szTime[0], &uszUnPackBuf[0], 3);

	return (VS_SUCCESS);	
}

int inDINERS_UnPack13(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
        /* 只改變月份和日期 */
        memset(&pobTran->srBRec.szDate[4], 0x00, 4);
        inFunc_BCD_to_ASCII(&pobTran->srBRec.szDate[4], &uszUnPackBuf[0], 2);

	return (VS_SUCCESS);	
}

int inDINERS_UnPack37(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
        if (pobTran->inISOTxnCode == _REVERSAL_)
                return (VS_SUCCESS);

        memset(pobTran->srBRec.szRefNo, 0x00, sizeof(pobTran->srBRec.szRefNo));
        memcpy(&pobTran->srBRec.szRefNo[0], (char *) &uszUnPackBuf[0], 12);

	return (VS_SUCCESS);	
}

int inDINERS_UnPack38(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
        if (pobTran->inISOTxnCode == _REVERSAL_)
                return (VS_SUCCESS);

        memset(pobTran->srBRec.szAuthCode, 0x00, sizeof(pobTran->srBRec.szAuthCode));
        memcpy(&pobTran->srBRec.szAuthCode[0], (char *) &uszUnPackBuf[0], _AUTH_CODE_SIZE_);

	return (VS_SUCCESS);	
}

int inDINERS_UnPack39(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
        memcpy(&pobTran->srBRec.szRespCode[0], (char *) uszUnPackBuf, 2);
        pobTran->srBRec.szRespCode[2] = 0x00;

	return (VS_SUCCESS);
}

int inDINERS_UnPack60(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
	return (VS_SUCCESS);	
}

int inDINERS_ISOGetBitMapCode(TRANSACTION_OBJECT *pobTran, int inTxnType)
{
        int	inBitMapTxnCode = -1;

        switch (inTxnType)
        {
                case _REVERSAL_:
                case _SALE_:
                case _SALE_OFFLINE_:
                case _TIP_:
                case _ADJUST_ :
                case _ADVICE_ :
                case _REFUND_:
                case _PRE_AUTH_:
                case _SETTLE_:
                case _CLS_BATCH_:
                case _BATCH_UPLOAD_:
                case _VOID_:
                        inBitMapTxnCode = inTxnType;
                        break;
                default:
                        break;
        }

        return (inBitMapTxnCode);
}

int inDINERS_ISOPackMessageType(TRANSACTION_OBJECT *pobTran, int inTxnCode, unsigned char *uszPackData, char *szMTI)
{
        int		inCnt = 0;
        unsigned char	uszBCD[10 + 1];

        memset(uszBCD, 0x00, sizeof(uszBCD));
        inFunc_ASCII_to_BCD (&uszBCD[0], &szMTI[0], 4);
        memcpy((char *)&uszPackData[inCnt], (char *)&uszBCD[0], _DINERS_MTI_SIZE_);
        inCnt += _DINERS_MTI_SIZE_;
        return (inCnt);
}

int inDINERS_ISOModifyBitMap(TRANSACTION_OBJECT *pobTran, int inTxnType, int *inBitMap)
{
        if (pobTran->inISOTxnCode == _SETTLE_ || pobTran->inISOTxnCode == _CLS_BATCH_)
                return (VS_SUCCESS); /* 因為測試到【Batch UpLoad】會亂送 */

        if (pobTran->srBRec.uszManualBit == VS_TRUE)
        {
                inDINERS_BitMapSet(inBitMap, 2);
                inDINERS_BitMapSet(inBitMap, 14);
                inDINERS_BitMapReset(inBitMap, 35);
        }

        if (strlen(pobTran->srBRec.szRefNo) > 0)
                inDINERS_BitMapSet(inBitMap, 37);

        return (VS_SUCCESS);
}

int inDINERS_ISOModifyPackData(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackData, int *inPackLen)
{
        return (VS_SUCCESS);
}

int inDINERS_ISOCheckHeader(TRANSACTION_OBJECT *pobTran, char *szSendISOHeader, char *szReceISOHeader)
{
        int     inCnt = 0;

        /* 檢查TPDU */
        inCnt += 5;
        /* 檢查MTI */
        szSendISOHeader[inCnt + 1] += 0x10;
        if (memcmp(&szSendISOHeader[inCnt], &szReceISOHeader[inCnt], 2))
                return (VS_ERROR);

        return (VS_SUCCESS);
}

int inDINERS_ISOOnlineAnalyse(TRANSACTION_OBJECT *pobTran)
{
        int     inRetVal;

        inRetVal = inDINERS_OnlineAnalyseMagneticManual(pobTran);
        return (inRetVal);
}

int inDINERS_ISOAdviceAnalyse(TRANSACTION_OBJECT *pobTran, unsigned char * uszTcUpload)
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

                /* pobTran->uszUpdateBatchBit 表示 uszUpdateBatchBit / TRANS_BATCH_KEY】是要更新記錄 */
                pobTran->uszUpdateBatchBit = VS_TRUE;

                /* 存檔 */
                if (inFLOW_RunFunction(pobTran, _UPDATE_BATCH_) != VS_SUCCESS)
                        return (VS_ERROR);

                /* 如果【ADVICE】刪除失敗會鎖機 */
                if (inADVICE_Update(pobTran) != VS_SUCCESS)
		{
			/* 鎖機 */
			inFunc_EDCLock(AT);
			
                        return (VS_ERROR);
		}
                return (VS_SUCCESS);
        }
        else
        {
                /* 結帳交易流程中，若於前帳前補送電文，補送電文有拒絕或其他回覆碼的狀況，畫面皆顯示結帳失敗即可，不須顯示補送電文之回覆碼訊息。 */
        }

        return (VS_ERROR);
}

int inDINERS_BitMapSet(int *inBitMap, int inFeild)
{
        int 	i, inBMapCnt, inBitMapCnt;
        int 	inBMap[_DINERS_MAX_BIT_MAP_CNT_];

        memset((char *)inBMap, 0x00, sizeof(inBMap));
        inBMapCnt = inBitMapCnt = 0;

        for (i = 0; i < 64; i ++)
        {
                if (inBitMap[inBitMapCnt] > inFeild)
                {
                        if (i == 0)
                        {
                                /* 第一個BitMap */
                                inBMap[inBMapCnt ++] = inFeild;
                                break;
                        }
                }
                else if (inBitMap[inBitMapCnt] < inFeild)
                {
                        inBMap[inBMapCnt ++] = inBitMap[inBitMapCnt ++];
                        if (inBitMap[inBitMapCnt] == 0 || inBitMap[inBitMapCnt] > inFeild) /* 最後一個 BitMap */
                        {
                                inBMap[inBMapCnt ++] = inFeild;
                                break;
                        }
                }
                else
                        break; /* if (*inBitMap == inFeild) */
        }

        for (i = 0 ;; i ++)
        {
                if (inBitMap[inBitMapCnt] != 0)
                        inBMap[inBMapCnt ++] = inBitMap[inBitMapCnt ++];
                else
                        break;
        }

        memcpy((char *)inBitMap, (char *)inBMap, sizeof(inBMap));
        return (VS_SUCCESS);
}

int inDINERS_BitMapReset(int *inBitMap, int inFeild)
{
        int 	i, inBMapCnt;
        int 	inBMap[_DINERS_MAX_BIT_MAP_CNT_];

        memset((char *)inBMap, 0x00, sizeof(inBMap));
        inBMapCnt = 0;

        for (i = 0; i < 64; i ++)
        {
                if (inBitMap[i] == 0)
                        break;
                else if (inBitMap[i] != inFeild)
                        inBMap[inBMapCnt ++] = inBitMap[i];
        }

        memcpy((char *)inBitMap, (char *)inBMap, sizeof(inBMap));
        return (VS_SUCCESS);
}

int inDINERS_BitMapCheck(unsigned char *inBitMap, int inFeild)
{
        int	inByteIndex, inBitIndex;

        inFeild --;
        inByteIndex = inFeild / 8;
        inBitIndex  = 7 - (inFeild - inByteIndex * 8);

        if (_DINERS_CHECK_BIT_MAP_(inBitMap[inByteIndex], inBitIndex))
        {
                return (VS_TRUE);
        }

        return (VS_FALSE);
}

int inDINERS_CopyBitMap(int *inBitMap, int *inSearchBitMap)
{
        int     i;

        for (i = 0; i < _DINERS_MAX_BIT_MAP_CNT_; i ++)
        {
                if (inSearchBitMap[i] == 0)
                        break;
                else
                        inBitMap[i] = inSearchBitMap[i];
        }

        return (VS_SUCCESS);
}

int inDINERS_GetBitMapTableIndex(ISO_TYPE_DINERS_TABLE *srISOFunc, int inBitMapTxnCode)
{
        int     inBitMapIndex;

        for (inBitMapIndex = 0 ;; inBitMapIndex ++)
        {
                if (srISOFunc->srBitMap[inBitMapIndex].inTxnID == _DINERS_NULL_TX_)
                {
                        return (VS_ERROR); /* 找不到相對應交易別的Bit Map */
                }

                if (srISOFunc->srBitMap[inBitMapIndex].inTxnID == inBitMapTxnCode)
                        break; /* 找到一樣的交易類別 */
        }

        return (inBitMapIndex);
}

int inDINERS_GetBitMapMessagegTypeField03(TRANSACTION_OBJECT *pobTran, ISO_TYPE_DINERS_TABLE *srISOFunc, int inTxnType, int *inTxnBitMap, unsigned char *uszSendBuf)
{
        int		i, j, k, inBitMapIndex, *inBitMap, inCnt, inBitMapTxnCode = -1;
        char		szErrorMessage[256 + 1];
        unsigned char 	uszBuf;

        /* 設定交易別 */
        inBitMapTxnCode = srISOFunc->inGetBitMapCode(pobTran, inTxnType);
        if (inBitMapTxnCode == -1)
                return (VS_ERROR);

        /* 要搜尋 BIT_MAP_DINERS_TABLE srDINERS_ISOBitMap 相對應的 inTxnID */
        if ((inBitMapIndex = inDINERS_GetBitMapTableIndex(srISOFunc, inBitMapTxnCode)) == VS_ERROR)
                return (VS_ERROR);

        /* Pack Message Type */
        inCnt = 0;
        inCnt += srISOFunc->inPackMTI(pobTran, inTxnType, &uszSendBuf[inCnt], srISOFunc->srBitMap[inBitMapIndex].szMTI);


        if (ginDebug == VS_TRUE)
        {
                for(i = 0 ; i < 20 ; i ++)
                {
                        memset(szErrorMessage, 0x00, sizeof(szErrorMessage));
                        sprintf(szErrorMessage, "1.inBitMap = %d", srISOFunc->srBitMap[inBitMapIndex].inBitMap[i]);
                        inLogPrintf(AT, szErrorMessage);
                }
        }

        /* 要搜尋 BIT_MAP_DINERS_TABLE srDINERS_ISOBitMap 相對應的 inBitMap */
        inDINERS_CopyBitMap(inTxnBitMap, srISOFunc->srBitMap[inBitMapIndex].inBitMap);


        if (ginDebug == VS_TRUE)
        {
                for(i = 0 ; i < 20 ; i ++)
                {
                        memset(szErrorMessage, 0x00, sizeof(szErrorMessage));
                        sprintf(szErrorMessage, "2.inBitMap = %d", srISOFunc->srBitMap[inBitMapIndex].inBitMap[i]);
                        inLogPrintf(AT, szErrorMessage);
                }
        }


        /* 修改 Bit Map */
        srISOFunc->inModifyBitMap(pobTran, inTxnType, inTxnBitMap);
        /* Pack Bit Map */
        inBitMap = inTxnBitMap;
        for (i = 0 ;; i ++)
        {
                uszBuf = 0x80;
                if (*inBitMap == 0)
                        break;

                j = *inBitMap / 8;
                k = *inBitMap % 8;

                if (k == 0)
                {
                        j --;
                        k = 8;
                }

                k --;

                while (k)
                {
                        uszBuf = uszBuf >> 1;
                        k --;
                }

                uszSendBuf[inCnt + j] += uszBuf;
                inBitMap ++;
        }

        inCnt += _DINERS_BIT_MAP_SIZE_;

        /* Process Code */
        memset(guszDINERS_ISO_Field03, 0x00, sizeof(guszDINERS_ISO_Field03));
        inFunc_ASCII_to_BCD (&guszDINERS_ISO_Field03[0], srISOFunc->srBitMap[inBitMapIndex].szPCode, 6);

        if (inBitMapTxnCode == _REVERSAL_)
        {
                if (pobTran->inTransactionCode == _VOID_)
                {
                        /*
                                Processing Code         Activities
                                --------------------------------------------------------
                                    000000      |       Reversal Void Sale
                                    220000      |       Reversal Void Refund
                        */
                        memset(guszDINERS_ISO_Field03, 0x00, sizeof(guszDINERS_ISO_Field03));
                        switch (pobTran->srBRec.inOrgCode)
                        {
                                case _SALE_ :
                                case _SALE_OFFLINE_ :
                                        guszDINERS_ISO_Field03[0] = 0x00;
                                        guszDINERS_ISO_Field03[1] = 0x00;
                                        break;
                                case _REFUND_ :
                                        guszDINERS_ISO_Field03[0] = 0x22;
                                        guszDINERS_ISO_Field03[1] = 0x00;
                                        break;
                                default :
                                        guszDINERS_ISO_Field03[0] = 0x00;
                                        break;
                        }
                }
                else
                {
                        /*
                                Processing Code         Activities
                                --------------------------------------------------------
                                    000000      |       Reversal Sale
                                    200000      |       Reversal Refund
                                    380000      |       Reversal Pre-Auth
                        */
                        memset(guszDINERS_ISO_Field03, 0x00, sizeof(guszDINERS_ISO_Field03));
                        switch (pobTran->inTransactionCode)
                        {
                                case _SALE_ :
                                case _SALE_OFFLINE_ :
                                        guszDINERS_ISO_Field03[0] = 0x00;
                                        guszDINERS_ISO_Field03[1] = 0x00;
                                        break;
                                case _REFUND_ :
                                        guszDINERS_ISO_Field03[0] = 0x20;
                                        guszDINERS_ISO_Field03[1] = 0x00;
                                        break;
                                case _PRE_AUTH_ :
                                        guszDINERS_ISO_Field03[0] = 0x38;
                                        guszDINERS_ISO_Field03[1] = 0x00;
                                        break;
                                default :
                                        guszDINERS_ISO_Field03[0] = 0x00;
                                        break;
                        }
                }
        }
        else if (inBitMapTxnCode == _BATCH_UPLOAD_)
        {
                /*
                        Processing Code         Activities
                        --------------------------------------------------------
                            000000      |       Reversal Sale
                            200000      |       Reversal Refund
                */
                memset(guszDINERS_ISO_Field03, 0x00, sizeof(guszDINERS_ISO_Field03));
                switch (pobTran->srBRec.inOrgCode)
                {
                        case _SALE_ :
                        case _SALE_OFFLINE_ :
                                guszDINERS_ISO_Field03[0] = 0x00;
                                guszDINERS_ISO_Field03[1] = 0x00;
                                break;
                        case _REFUND_ :
                                guszDINERS_ISO_Field03[0] = 0x20;
                                guszDINERS_ISO_Field03[1] = 0x00;
                                break;
                        default :
                                break;
                }

                if (pobTran->uszLastBatchUploadBit == VS_TRUE)
                        guszDINERS_ISO_Field03[2] = 0x00;
                else
                        guszDINERS_ISO_Field03[2] = 0x01;

        }
        else if (inBitMapTxnCode == _VOID_)
        {
                /*
                        Processing Code         Activities
                        --------------------------------------------------------
                            020000      |       Void Sale
                            220000      |       Void Refund
                */
                memset(guszDINERS_ISO_Field03, 0x00, sizeof(guszDINERS_ISO_Field03));
                switch (pobTran->srBRec.inOrgCode)
                {
                        case _SALE_ :
                        case _SALE_OFFLINE_ :
                        case _TIP_ :
                        case _ADJUST_ :
                                guszDINERS_ISO_Field03[0] = 0x02;
                                guszDINERS_ISO_Field03[1] = 0x00;
                                break;
                        case _REFUND_ :
                                guszDINERS_ISO_Field03[0] = 0x22;
                                guszDINERS_ISO_Field03[1] = 0x00;
                                break;
                        default :
                                guszDINERS_ISO_Field03[0] = 0x00;
                                break;
                }
        }
        else if (pobTran->inTransactionCode == _TIP_ || pobTran->inTransactionCode == _ADJUST_)
        {
                /*
                        Processing Code         Activities
                        --------------------------------------------------------------
                            000000      |       When original sale not seen by host
                            020000      |       When original sale seen by host
                */
                memset(guszDINERS_ISO_Field03, 0x00, sizeof(guszDINERS_ISO_Field03));
                if (pobTran->srBRec.inOrgCode == _SALE_OFFLINE_)
                {
                        guszDINERS_ISO_Field03[0] = 0x00;
                        guszDINERS_ISO_Field03[1] = 0x00;
                }
        }

        return (inCnt);
}

int inDINERS_PackISO(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendBuf, int inTxnCode)
{
        int			i, inSendCnt, inField, inCnt;
        int			inBitMap[_DINERS_MAX_BIT_MAP_CNT_ + 1];
        int			inRetVal, inISOFuncIndex = -1;
        char			szTemplate[40 + 1];
        char			szLogMessage[40 + 1];
        unsigned char		uszBCD[20 + 1];
        ISO_TYPE_DINERS_TABLE	srISOFunc;

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inDINERS_PackISO() START!");

        inSendCnt = 0;
        inField = 0;

        /* 加密模式，預設值 = 0。【0 = 不加密，1 = tSAM 加密，2 = 軟體加密】 */
        inISOFuncIndex = 0; /* 不加密 */
        /* 決定要執行第幾個 Function Index */
        memset((char *)&srISOFunc, 0x00, sizeof(srISOFunc));
        memcpy((char *)&srISOFunc, (char *)&srDINERS_ISOFunc[inISOFuncIndex], sizeof(srISOFunc));
        memset((char *)inBitMap, 0x00, sizeof(inBitMap));

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
        inFunc_ASCII_to_BCD (&uszBCD[0], &szTemplate[0], 10);
        memcpy((char *)&uszSendBuf[inSendCnt], (char *)&uszBCD[0], _DINERS_TPDU_SIZE_);
        inSendCnt += _DINERS_TPDU_SIZE_;
        /* Get Bit Map / Mesage Type / Processing Code */
        inRetVal = inDINERS_GetBitMapMessagegTypeField03(pobTran, &srISOFunc, inTxnCode, inBitMap, &uszSendBuf[inSendCnt]);
        if (inRetVal == VS_ERROR)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inDINERS_GetBitMapMessagegTypeField03() ERROR!");

                return (VS_ERROR);
        }
        else
                inSendCnt += inRetVal; /* Bit Map 長度 */

        for (i = 0 ;; i ++)
        {
                if (inBitMap[i] == 0)
                        break;

                while (inBitMap[i] > srISOFunc.srPackISO[inField].inFieldNum)
                {
                        inField ++;
                }

                if (inBitMap[i] == srISOFunc.srPackISO[inField].inFieldNum)
                {
                        inCnt = srISOFunc.srPackISO[inField].inISOLoad(pobTran, &uszSendBuf[inSendCnt]);
                        if (inCnt <= 0)
                        {
                                if (ginDebug == VS_TRUE)
                                {
                                        memset(szLogMessage, 0x00, sizeof(szLogMessage));
                                        sprintf(szLogMessage, "inField = %d Error", inField);
                                        inLogPrintf(AT, szLogMessage);
                                }

                                return (VS_ERROR);
                        }
                        else
                                inSendCnt += inCnt;
                }
        }

        if (srISOFunc.inModifyPackData != _DINERS_NULL_TX_)
                srISOFunc.inModifyPackData(pobTran, uszSendBuf, &inSendCnt);

        return (inSendCnt);
}

int inDINERS_CheckUnPackField(int inField, ISO_FIELD_DINERS_TABLE *srCheckUnPackField)
{
        int     i;

        for (i = 0; i < 64; i ++)
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

int inDINERS_GetCheckField(int inField, ISO_CHECK_DINERS_TABLE *ISOFieldCheck)
{
        int     i;

        for (i = 0; i < 64; i ++)
        {
                if (ISOFieldCheck[i].inFieldNum == 0)
                        break;
                else if (ISOFieldCheck[i].inFieldNum == inField)
                {
                        return (i); /* i 是 ISO_CHECK_DINERS_TABLE 裡的第幾個 */
                }
        }

        return (VS_ERROR);
}

int inDINERS_GetFieldLen(int inField, unsigned char *uszSendData, ISO_FIELD_TYPE_DINERS_TABLE *srFieldType)
{
        int     inCnt = 0, i, inLen;

        for (i = 0 ;; i ++)
        {
                if (srFieldType[i].inFieldNum == 0)
                        break;

                if (srFieldType[i].inFieldNum != inField)
                        continue;

                switch (srFieldType[i].inFieldType)
                {
                        case _DINERS_ISO_ASC_ :
                                inCnt += srFieldType[i].inFieldLen;
                                break;
                        case _DINERS_ISO_BCD_ :
                                inCnt += srFieldType[i].inFieldLen / 2;
                                break;
                        case _DINERS_ISO_NIBBLE_2_ :
                                inLen = (uszSendData[0] / 16 * 10) + (uszSendData[0] % 16);
                                inCnt += ((inLen + 1) / 2) + 1;
                                break;
                        case _DINERS_ISO_NIBBLE_3_ :
                                inLen = (uszSendData[0] / 16 * 1000) + (uszSendData[0] % 16 * 100) + (uszSendData[1] / 16 * 10) + (uszSendData[1] % 16);
                                inCnt += ((inLen + 1) / 2) + 2;
                                break;
                        case _DINERS_ISO_BYTE_2_ :
                                inLen = (uszSendData[0] / 16 * 10) + (uszSendData[0] % 16);
                                inCnt += inLen + 1;
                                break;
                        case _DINERS_ISO_BYTE_3_ :
                                inLen = (uszSendData[0] / 16 * 1000) + (uszSendData[0] % 16 * 100) + (uszSendData[1] / 16 * 10) + (uszSendData[1] % 16);
                                inCnt += inLen + 2;
                                break;
                        case _DINERS_ISO_BYTE_2_H_ :
                                inLen = (int)uszSendData[0];
                                inCnt += inLen + 1;
                                break;
                        case _DINERS_ISO_BYTE_3_H_ :
                                inLen = ((int)uszSendData[0] * 0xFF) + (int)uszSendData[1];
                                inCnt += inLen + 1;
                                break;
                        default :
                                break;
                }

                break;
        }

        return (inCnt);
}

int inDINERS_GetFieldIndex(int inField, ISO_FIELD_TYPE_DINERS_TABLE *srFieldType)
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

int inDINERS_UnPackISO(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendBuf, unsigned char *uszRecvBuf)
{
        int			i, inRecvCnt, inCnt, inSendCnt, inSendField, inRecvField, inISOFuncIndex = -1;
        char			szBuf[_DINERS_TPDU_SIZE_ + _DINERS_MTI_SIZE_ + _DINERS_BIT_MAP_SIZE_ + 1];
        char			szErrorMessage[40 + 1];
        unsigned char		uszSendMap[_DINERS_BIT_MAP_SIZE_ + 1], uszReceMap[_DINERS_BIT_MAP_SIZE_ + 1];
        ISO_TYPE_DINERS_TABLE	srISOFunc;


        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inDINERS_UnPackISO() START!");

        inSendField = inRecvField = 0;
        inSendCnt = inRecvCnt = 0;

        memset((char *)uszSendMap, 0x00, sizeof(uszSendMap));
        memset((char *)uszReceMap, 0x00, sizeof(uszReceMap));
        memset((char *)szBuf, 0x00, sizeof(szBuf));
        /* 加密模式，預設值 = 0。【0 = 不加密，1 = tSAM 加密，2 = 軟體加密】 */
        inISOFuncIndex = 0; /* 不加密 */
        memset((char *)&srISOFunc, 0x00, sizeof(srISOFunc));
        memcpy((char *)&srISOFunc, (char *)&srDINERS_ISOFunc[inISOFuncIndex], sizeof(srISOFunc));

        if (srISOFunc.inCheckISOHeader != NULL)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "srISOFunc.inCheckISOHeader != NULL");

                if (srISOFunc.inCheckISOHeader(pobTran, (char *)&uszSendBuf[inRecvCnt], (char *)&uszRecvBuf[inRecvCnt]) != VS_SUCCESS)
                {
                        if (ginDebug == VS_TRUE)
                                inLogPrintf(AT, "srISOFunc.inCheckISOHeader ERROR");

                        return (_TRAN_RESULT_UNPACK_ERR_);
        }
        }

        inSendCnt += _DINERS_TPDU_SIZE_;
        inRecvCnt += _DINERS_TPDU_SIZE_;
        inSendCnt += _DINERS_MTI_SIZE_;
        inRecvCnt += _DINERS_MTI_SIZE_;

        memcpy((char *)uszSendMap, (char *)&uszSendBuf[inSendCnt], _DINERS_BIT_MAP_SIZE_);
        memcpy((char *)uszReceMap, (char *)&uszRecvBuf[inRecvCnt], _DINERS_BIT_MAP_SIZE_);

        inSendCnt += _DINERS_BIT_MAP_SIZE_;
        inRecvCnt += _DINERS_BIT_MAP_SIZE_;

        /* 先檢查 ISO Field_39 */
        if (inDINERS_BitMapCheck(uszReceMap, 39) == VS_FALSE)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inDINERS_BitMapCheck(39) ERROR");

                return (VS_ERROR);
        }

        if (inDINERS_BitMapCheck(uszReceMap, 41) == VS_FALSE)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inDINERS_BitMapCheck(41) ERROR");

                return (VS_ERROR);
        }

        for (i = 1; i <= 64; i ++)
        {
                /* 有送出去的 Field 但沒有收回來的 Field */
                if (inDINERS_BitMapCheck(uszSendMap, i) && !inDINERS_BitMapCheck(uszReceMap, i))
                {
                        inSendCnt += inDINERS_GetFieldLen(i, &uszSendBuf[inSendCnt], srISOFunc.srISOFieldType);
                }
                else if (inDINERS_BitMapCheck(uszReceMap, i)) /* 收的 BitMap */
                {
                        if (inDINERS_BitMapCheck(uszSendMap, i)) /* 送的 BitMap 都有 */
                        {
                                /* 是否要進行檢查封包資料包含【送】【收】 */
                                if ((inSendField = inDINERS_GetCheckField(i, srISOFunc.srCheckISO)) != VS_ERROR)
                                {
                                        if (srISOFunc.srCheckISO[inSendField].inISOCheck(pobTran, &uszSendBuf[inSendCnt], &uszRecvBuf[inRecvCnt]) != VS_SUCCESS)
                                        {
                                                if (ginDebug == VS_TRUE)
                                                {
                                                        memset(szErrorMessage, 0x00, sizeof(szErrorMessage));
                                                        sprintf(szErrorMessage, "inSendField = %d Error!", inSendField);
                                                        inLogPrintf(AT, szErrorMessage);
                                                }

                                                return (_TRAN_RESULT_UNPACK_ERR_);
                                }
                                }

                                inSendCnt += inDINERS_GetFieldLen(i, &uszSendBuf[inSendCnt], srISOFunc.srISOFieldType);
                        }

                        while (i > srISOFunc.srUnPackISO[inRecvField].inFieldNum) /* 只有收 */
                        {
                                inRecvField ++;
                        }

                        if (i == srISOFunc.srUnPackISO[inRecvField].inFieldNum)
                        {
                                /* 要先檢查 UnpackISO 是否存在 */
                                if (inDINERS_CheckUnPackField(i, srISOFunc.srUnPackISO) == VS_SUCCESS)
                                        srISOFunc.srUnPackISO[inRecvField].inISOLoad(pobTran, &uszRecvBuf[inRecvCnt]);
                        }

                        inCnt = inDINERS_GetFieldLen(i, &uszRecvBuf[inRecvCnt], srISOFunc.srISOFieldType);
                        if (inCnt == VS_ERROR)
                        {
                                if (ginDebug == VS_TRUE)
                                {
                                        memset(szErrorMessage, 0x00, sizeof(szErrorMessage));
                                        sprintf(szErrorMessage, "inRecvField = %d Error!", inRecvField);
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
                        case _SALE_ :
                        case _REFUND_ :
                        case _VOID_ :
                        case _PRE_AUTH_ :
                                if (inDINERS_BitMapCheck(uszReceMap, 38) == VS_FALSE)
{
                                        if (ginDebug == VS_TRUE)
        {
                                                inLogPrintf(AT, "inDINERS_BitMapCheck 38 Error!");
                                        }

                                return (VS_ERROR);
                }

                                break;
                        default :
                                break;
        }
        }

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inDINERS_UnPackISO() END!");

        return (VS_SUCCESS);
}



int inDINERS_GetSTAN(TRANSACTION_OBJECT *pobTran)
{
        char	szSTANNum[12 + 1];

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inDINERS_GetSTAN() START!");

        memset(szSTANNum, 0x00, sizeof(szSTANNum));
        if (inGetSTANNum(szSTANNum) == VS_ERROR)
                return (VS_ERROR);

        pobTran->srBRec.lnSTANNum = atol(szSTANNum);

        return (VS_SUCCESS);
}

int inDINERS_SetSTAN(TRANSACTION_OBJECT *pobTran)
{
        long    lnSTAN;
        char    szSTANNum[12 + 1];

        memset(szSTANNum, 0x00, sizeof(szSTANNum));
        if (inGetSTANNum(szSTANNum) == VS_ERROR)
                return (VS_ERROR);

        lnSTAN = atol(szSTANNum);
        if (lnSTAN ++ > 999999)
                lnSTAN = 1;

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

int inDINERS_ProcessReversal(TRANSACTION_OBJECT *pobTran)
{
        int     inRetVal;
        char    szSendReversalBit[2 + 1];

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inDINERS_ProcessReversal() START!");

        memset(szSendReversalBit, 0x00, sizeof(szSendReversalBit));
        if (inGetSendReversalBit(szSendReversalBit) == VS_ERROR)
                return (VS_ERROR);

        if (!memcmp(szSendReversalBit, "Y", 1))
        {
                if ((inRetVal = inDINERS_ReversalSendRecvPacket(pobTran)) != VS_SUCCESS)
                {
                        return (VS_ERROR);
                }
        }

        /* 檢查是否要產生 Reversal */
        if (pobTran->uszReversalBit == VS_TRUE)
        {
                if ((inRetVal = inDINERS_ReversalSave_Flow(pobTran)) != VS_SUCCESS)
                {
                        return (VS_ERROR);
                }
        }

        return (VS_SUCCESS);
}

int inDINERS_AdviceSendRecvPacket(TRANSACTION_OBJECT *pobTran, int inAdvCnt)
{
        int			inCnt, inRetVal = VS_ERROR, inISOFuncIndex = -1;
        unsigned char		uszTCUpload = 0;
        TRANSACTION_OBJECT	ADVpobTran;
        ISO_TYPE_DINERS_TABLE	srISOFunc;

        /* 加密模式，預設值 = 0。【0 = 不加密，1 = tSAM 加密，2 = 軟體加密】 */
        inISOFuncIndex = 0; /* 不加密 */
        memset((char *)&srISOFunc, 0x00, sizeof(srISOFunc));
        memcpy((char *)&srISOFunc, (char *)&srDINERS_ISOFunc[inISOFuncIndex], sizeof(srISOFunc));

        memset((char *)&ADVpobTran, 0x00, sizeof(TRANSACTION_OBJECT));
        memcpy((char *)&ADVpobTran, (char *)pobTran, sizeof(TRANSACTION_OBJECT));

        /* 開啟【.bat】【.bkey】【.adv】三個檔 */
        if (inBATCH_AdviceHandleReadOnly_By_Sqlite(pobTran) != VS_SUCCESS)
	{
                return (VS_NO_RECORD);
	}

        /* 這裡的 for () 不可以在裡面直接call Return () ....  */
        for (inCnt = 0; inCnt < inAdvCnt; inCnt ++)
        {
                ADVpobTran.srBRec.lnOrgInvNum = -1;

                if (ADVpobTran.uszTCUploadBit != VS_TRUE)
                {
                        /* 這裡要開始逐一將【0220】交易上傳 */
                        if (inBATCH_GetAdviceDetailRecord_By_Sqlite(&ADVpobTran, inCnt) == VS_SUCCESS)
                        {
                                inRetVal = VS_SUCCESS;
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

                if (ADVpobTran.srBRec.inCode == _TIP_)
                {
                        if (ADVpobTran.srBRec.uszUpload1Bit == VS_TRUE && ADVpobTran.srBRec.uszUpload2Bit == VS_TRUE && ADVpobTran.srBRec.uszUpload3Bit == VS_FALSE)
                                ADVpobTran.inISOTxnCode = _SALE_OFFLINE_; /* 先送原交易 */
                        else
                                ADVpobTran.inISOTxnCode = _TIP_; /* 送小費交易 */
                }
                else if (ADVpobTran.srBRec.inCode == _ADJUST_)
                {
                        if (ADVpobTran.srBRec.uszUpload1Bit == VS_TRUE && ADVpobTran.srBRec.uszUpload2Bit == VS_TRUE && ADVpobTran.srBRec.uszUpload3Bit == VS_FALSE)
                                ADVpobTran.inISOTxnCode = _SALE_OFFLINE_; /* 先送原交易 */
                        else
                                ADVpobTran.inISOTxnCode = _ADJUST_; /* 送調帳交易 */
                }
                else if (ADVpobTran.srBRec.inCode == _VOID_)
                {
                        if (ADVpobTran.srBRec.uszUpload1Bit == VS_TRUE && ADVpobTran.srBRec.uszUpload2Bit == VS_TRUE && ADVpobTran.srBRec.uszUpload3Bit == VS_TRUE)
                        {
                                ADVpobTran.inISOTxnCode = _SALE_OFFLINE_;
                        }
                        else if (ADVpobTran.srBRec.uszUpload1Bit == VS_FALSE && ADVpobTran.srBRec.uszUpload2Bit == VS_TRUE && ADVpobTran.srBRec.uszUpload3Bit == VS_TRUE)
                        {
                                if (ADVpobTran.srBRec.inOrgCode == _TIP_)
                                        ADVpobTran.inISOTxnCode = _TIP_;
                                else if (ADVpobTran.srBRec.inOrgCode == _ADJUST_)
                                        ADVpobTran.inISOTxnCode = _ADJUST_;
                                else
                                        return (VS_ERROR);
                        }
                        else if (ADVpobTran.srBRec.uszUpload1Bit == VS_TRUE && ADVpobTran.srBRec.uszUpload2Bit == VS_TRUE && ADVpobTran.srBRec.uszUpload3Bit == VS_FALSE)
                        {
                                if (ADVpobTran.srBRec.inOrgCode == _SALE_OFFLINE_)
                                        ADVpobTran.inISOTxnCode = _SALE_OFFLINE_;
                                else
                                        return (VS_ERROR);
                        }
                        else
                        {
                                ADVpobTran.inISOTxnCode = _VOID_; /* 送取消交易 */
                        }
                }
                else
                {
                        /* 交易補登或是REFERRAL */
                        ADVpobTran.inISOTxnCode = _SALE_OFFLINE_; /* 表示做結帳前要先送【Advice】所以交易日期、時間不應該被更新 */
                }

                if (inRetVal == VS_SUCCESS)
                        inRetVal = inDINERS_SendPackRecvUnPack(&ADVpobTran);

                if (inRetVal == VS_SUCCESS)
                        if (srISOFunc.inAdviceAnalyse != NULL)
                                inRetVal = srISOFunc.inAdviceAnalyse(&ADVpobTran, &uszTCUpload);

                if (inRetVal != VS_SUCCESS)
                        break;
        }

        /* 關閉【.bat】【.bket】【.adv】三個檔 */
        inBATCH_GlobalAdviceHandleClose_By_Sqlite();

        return (inRetVal);
}

int inDINERS_ProcessAdvice(TRANSACTION_OBJECT *pobTran)
{
        int     inSendAdviceCnt;

        if ((inSendAdviceCnt = inADVICE_GetTotalCount(pobTran)) == VS_NO_RECORD)
        {
                return (VS_SUCCESS);
        }
        else
        {
                if (inDINERS_AdviceSendRecvPacket(pobTran, inSendAdviceCnt) != VS_SUCCESS)
                {
                        return (VS_ERROR);
                }
        }


        return (VS_SUCCESS);
}

int inDINERS_ProcessOnline(TRANSACTION_OBJECT *pobTran)
{
        int     inRetVal;
	char	szTemplate[512 + 1];

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inDINERS_ProcessOnline() START!");

        /* 開始組交易封包，送、收、組、解 */
        pobTran->inISOTxnCode = pobTran->inTransactionCode; /* 以 srEventMenuItem.inCode Index */
        inRetVal = inDINERS_SendPackRecvUnPack(pobTran);
        if (inRetVal == VS_COMM_ERROR)
        {
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
                pobTran->inTransactionResult = inDINERS_CheckRespCode(pobTran); /* 【Field_39】 */
                if (pobTran->inTransactionResult == _TRAN_RESULT_AUTHORIZED_)
                {
                        if (inDINERS_CheckAuthCode(pobTran) != VS_SUCCESS)
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
                inLogPrintf(AT, "inDINERS_ProcessOnline() END!");

        return (VS_SUCCESS);
}

int inDINERS_ProcessOffline(TRANSACTION_OBJECT *pobTran)
{
        int     inRetVal = VS_SUCCESS;

        /* Save Advice */
        if (inADVICE_SaveAppend(pobTran, pobTran->srBRec.lnOrgInvNum) != VS_SUCCESS)
        {
                inRetVal = VS_ERROR;
        }
        else
        {
                pobTran->inTransactionResult = _TRAN_RESULT_AUTHORIZED_;

                memset(pobTran->srBRec.szRespCode, 0x00, sizeof(pobTran->srBRec.szRespCode));
                strcpy(pobTran->srBRec.szRespCode, "00");
                pobTran->srBRec.szRespCode[2] = 0x00;
        }

        return (inRetVal);
}

int inDINERS_AnalysePacket(TRANSACTION_OBJECT *pobTran)
{
        int     inRetVal = VS_SUCCESS, inISOFuncIndex = -1;
        ISO_TYPE_DINERS_TABLE srISOFunc;

        if (pobTran->srBRec.uszOfflineBit == VS_FALSE)
        {
                if (pobTran->inTransactionResult == _TRAN_RESULT_AUTHORIZED_ || pobTran->inTransactionResult == _TRAN_RESULT_REFERRAL_)
                {
                        /* 加密模式，預設值 = 0。【0 = 不加密，1 = tSAM 加密，2 = 軟體加密】 */
                        inISOFuncIndex = 0; /* 不加密 */
                        memset((char *)&srISOFunc, 0x00, sizeof(srISOFunc));
                        memcpy((char *)&srISOFunc, (char *)&srDINERS_ISOFunc[inISOFuncIndex], sizeof(srISOFunc));

                        if (srISOFunc.inOnAnalyse != NULL)
                                inRetVal = srISOFunc.inOnAnalyse(pobTran);
                }
                else
                {
                        inRetVal = VS_ERROR;
                }
        }

        return (inRetVal);
}

int inDINERS_ReversalSendRecvPacket(TRANSACTION_OBJECT *pobTran)
{
        int		inRetVal, inSendCnt;
        long		lnREVCnt;
	char		szDialBackupEnable[2 + 1];
	char		szCommMode[2 + 1];
        unsigned char   uszFileName[20 + 1];
        unsigned char   uszSendPacket[_DINERS_ISO_SEND_ + 1], uszRecvPacket[_DINERS_ISO_RECV_ + 1];
        unsigned long   ulREVFHandle;

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
                inLogPrintf(AT, "inDINERS_CommSendRecvToHost() Before");

        if ((inRetVal = inDINERS_CommSendRecvToHost(pobTran, uszSendPacket, inSendCnt, uszRecvPacket)) != VS_SUCCESS)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inDINERS_CommSendRecvToHost() Error");

                return (VS_ERROR);
        }

        /* 解 ISO 電文 */
        if ((inRetVal = inDINERS_UnPackISO(pobTran, uszSendPacket, uszRecvPacket)) != VS_SUCCESS)
        {
		pobTran->inTransactionResult = _TRAN_RESULT_UNPACK_ERR_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_ISO_UNPACK_;
		
                return (VS_ERROR);
	}

        if (memcmp(pobTran->srBRec.szRespCode, "00", 2))
	{
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
Function        :inDINERS_ReversalSave_Flow
Date&Time       :2017/3/30 上午 11:06
Describe        :
*/
int inDINERS_ReversalSave_Flow(TRANSACTION_OBJECT *pobTran)
{
	int 	inRetVal;
	char	szDialBackupEnable[2 + 1];
	
	inRetVal = inDINERS_ReversalSave(pobTran);
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	memset(szDialBackupEnable, 0x00, sizeof(szDialBackupEnable));
	inGetDialBackupEnable(szDialBackupEnable);

	if (memcmp(szDialBackupEnable, "Y", strlen("Y")) == 0)
	{

		inRetVal = inDINERS_ReversalSave_For_DialBeckUp(pobTran);
		if (inRetVal != VS_SUCCESS)
		{
			return (VS_ERROR);
		}

	}
	
	return (VS_SUCCESS);
}

int inDINERS_ReversalSave(TRANSACTION_OBJECT *pobTran)
{
        int		inRetVal, inPacketCnt;
        long		lnBatchNum;
        char		szTemplate[20 + 1];
        unsigned char	uszReversalPacket[_DINERS_ISO_SEND_ + 1];
        unsigned char   uszFileName[20 + 1];
        unsigned long	ulFHandle;

        memset(uszReversalPacket, 0x00, sizeof(uszReversalPacket));
        pobTran->inISOTxnCode = _REVERSAL_; /* 沖銷 */

        /* 組 REVERSAL 封包 */
        inPacketCnt = inDINERS_PackISO(pobTran, &uszReversalPacket[0], pobTran->inISOTxnCode);
        if (inPacketCnt <= 0)
	{
		pobTran->inTransactionResult = _TRAN_RESULT_PACK_ERR_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_ISO_PACK_;
		
                return (VS_ERROR);
	}

        memset(szTemplate, 0x00, sizeof(szTemplate));
        if (inGetBatchNum(szTemplate) == VS_ERROR)
                return (VS_ERROR);

        lnBatchNum = atol(szTemplate);
        memset(uszFileName, 0x00, sizeof(uszFileName));
        sprintf((char *)uszFileName, "%s%06lu%s", _FILE_NAME_DINERS_, lnBatchNum, _REVERSAL_FILE_EXTENSION_);

        /* 先刪除上一筆 Reversal，確保是當筆的 Reversal */
        inFILE_Delete(uszFileName);

        inRetVal = inFILE_Create(&ulFHandle, uszFileName);
        if (inRetVal == VS_ERROR)
        {
                inFILE_Close(&ulFHandle);
                inFILE_Delete(uszFileName);
                return (VS_ERROR);
        }

        inRetVal = inFILE_Seek(ulFHandle, 0, _SEEK_BEGIN_); /* 從頭開始 */
        if (inRetVal == VS_ERROR)
        {
                inFILE_Close(&ulFHandle);
                inFILE_Delete(uszFileName);
                return (VS_ERROR);
        }

        inRetVal = inFILE_Write(&ulFHandle, &uszReversalPacket[0], inPacketCnt);
        if (inRetVal == VS_ERROR)
        {
                inFILE_Close(&ulFHandle);
                inFILE_Delete(uszFileName);
                return (VS_ERROR);
        }

        inFILE_Close(&ulFHandle);
        inSetSendReversalBit("Y");
        if (inSaveHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
        {
                inFILE_Close(&ulFHandle);
                inFILE_Delete(uszFileName);
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

/*
Function        :inDINERS_ReversalSave_For_DialBeckUp
Date&Time       :2017/3/30 上午 10:24
Describe        :
*/
int inDINERS_ReversalSave_For_DialBeckUp(TRANSACTION_OBJECT *pobTran)
{
        int 		inRetVal, inPacketCnt;
        unsigned char 	uszReversalPacket[_DINERS_ISO_SEND_ + 1];
        unsigned char 	uszFileName[20 + 1];
        unsigned long 	srFHandle;


        memset(uszReversalPacket, 0x00, sizeof(uszReversalPacket));
        pobTran->inISOTxnCode = _REVERSAL_; /* 沖銷 */

        /* 組_REVERSAL_ 封包 */
        inPacketCnt = inDINERS_PackISO(pobTran, &uszReversalPacket[0], pobTran->inISOTxnCode);
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

int inDINERS_CommSendRecvToHost(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, int inSendLen, unsigned char *uszRecvPacket)
{
        int		inRetVal;
        int		inSendTimeout = 0;
        int		inReceiveTimeout = 10;
        int		inReceiveSize = _COMM_RECEIVE_MAX_LENGTH_;	/* 配合Ingenico */
	char		szTimeOut[2 + 1] = {0};
        char		szHostResponseTimeOut[2 + 1] = {0};
	unsigned char	uszDispBit = VS_TRUE;
        DISPLAY_OBJECT	srDisplay;

        memset(&srDisplay, 0x00, sizeof(DISPLAY_OBJECT));
        srDisplay.inY = _LINE_8_4_;
        srDisplay.inR_L = _DISP_LEFT_;
        strcpy(srDisplay.szPromptMsg, "傳送中...");
	
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
		vdDINERS_ISO_FormatDebug_DISP(pobTran, &uszSendPacket[0], inSendLen);
	}
	/* 列印ISO Debug */
        if (ginISODebug == VS_TRUE)
	{
                vdDINERS_ISO_FormatDebug_PRINT(pobTran, &uszSendPacket[0], inSendLen);
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
        if ((inRetVal = inCOMM_Send(uszSendPacket, inSendLen, inSendTimeout , uszDispBit)) != VS_SUCCESS)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inCOMM_ConnectStart() START!");
                return (VS_ERROR);
        }
	
	/* 這裡是用CPT.DAT的東西，所以一定要用 */
	memset(szHostResponseTimeOut, 0x00, sizeof(szHostResponseTimeOut));
        inRetVal = inGetHostResponseTimeOut(szHostResponseTimeOut);

        if (inRetVal != VS_SUCCESS)
                return (VS_ERROR);

        inReceiveTimeout = atoi(szHostResponseTimeOut);

        memset(&srDisplay, 0x00, sizeof(DISPLAY_OBJECT));
        srDisplay.inY = _LINE_8_4_;
        srDisplay.inR_L = _DISP_LEFT_;
        strcpy(srDisplay.szPromptMsg, "接收中...");

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
                        inLogPrintf(AT, "inCOMM_ConnectStart() START!");

                return (VS_ERROR);
        }
	
	/* 顯示ISO Debug */
	if (ginDebug == VS_TRUE)
	{
		vdDINERS_ISO_FormatDebug_DISP(pobTran, &uszRecvPacket[0], inRetVal);
	}
        /* 列印ISO Debug */
        if (ginISODebug == VS_TRUE)
                vdDINERS_ISO_FormatDebug_PRINT(pobTran, &uszRecvPacket[0], inRetVal);

        return (VS_SUCCESS);
}

int inDINERS_SendPackRecvUnPack(TRANSACTION_OBJECT *pobTran)
{
        int		inSendCnt = 0, inRecvCnt = 0, inRetVal;
        unsigned char	uszSendPacket[_DINERS_ISO_SEND_ + 1], uszRecvPacket[_DINERS_ISO_RECV_ + 1];

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inDINERS_SendPackRecvUnPack() START!");

        memset(uszSendPacket, 0x00, sizeof(uszSendPacket));
        memset(uszRecvPacket, 0x00, sizeof(uszRecvPacket));

        /* 組 ISO 電文 */
        if ((inSendCnt = inDINERS_PackISO(pobTran, uszSendPacket, pobTran->inISOTxnCode)) <= 0)
        {
		pobTran->inTransactionResult = _TRAN_RESULT_PACK_ERR_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_ISO_PACK_;
		
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inDINERS_PackISO() Error!");

                return (VS_ISO_PACK_ERR); /* 組交易電文錯誤，不用組 REVERSAL */
        }

        /* 傳送及接收 ISO 電文 */
        if ((inRecvCnt = inDINERS_CommSendRecvToHost(pobTran, uszSendPacket, inSendCnt, uszRecvPacket)) != VS_SUCCESS)
        {
		pobTran->inTransactionResult = _TRAN_RESULT_COMM_ERROR_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_COMM_;
		
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inDINERS_CommSendRecvToHost() Error");

                if (pobTran->inISOTxnCode != _ADVICE_)
                        memset(pobTran->srBRec.szRespCode, 0x00, sizeof(pobTran->srBRec.szRespCode));

                return (VS_COMM_ERROR);
        }

        /* 解 ISO 電文 */
        inRetVal = inDINERS_UnPackISO(pobTran, uszSendPacket, uszRecvPacket);
        if (inRetVal != VS_SUCCESS)
        {
		pobTran->inTransactionResult = _TRAN_RESULT_UNPACK_ERR_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_ISO_UNPACK_;
		
                inRetVal = VS_ISO_UNPACK_ERROR;
        }

        return (inRetVal);
}

int inDINERS_CheckRespCode(TRANSACTION_OBJECT *pobTran)
{
	int     inRetVal = _TRAN_RESULT_COMM_ERROR_;

	if (!memcmp(pobTran->srBRec.szRespCode, "00", 2))
	{
		inRetVal = _TRAN_RESULT_AUTHORIZED_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_NONE_;
	}
	else if (!memcmp(pobTran->srBRec.szRespCode, "01", 2) ||
		 !memcmp(pobTran->srBRec.szRespCode, "02", 2))
	{
		inRetVal = _TRAN_RESULT_REFERRAL_;
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
	else
	{
		if ((pobTran->srBRec.szRespCode[0] >= '0' && pobTran->srBRec.szRespCode[0] <= '9') &&
		    (pobTran->srBRec.szRespCode[1] >= '0' && pobTran->srBRec.szRespCode[1] <= '9'))
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

int inDINERS_CheckAuthCode(TRANSACTION_OBJECT *pobTran)
{
        int     inRetVal = VS_SUCCESS;

        switch (pobTran->inISOTxnCode)
        {
                case _SETTLE_ :
                case _BATCH_UPLOAD_ :
                case _CLS_BATCH_ :
                        break; /* 不檢核 */
                default :
                        if (!memcmp(&pobTran->srBRec.szAuthCode[0], "000000", 6) || !memcmp(&pobTran->srBRec.szAuthCode[0], "      ", 6))
                                inRetVal = VS_ERROR;

                        break;
        }

        return (inRetVal);
}

int inDINERS_OnlineAnalyseMagneticManual(TRANSACTION_OBJECT *pobTran)
{
        int     inRetVal = VS_SUCCESS;

        if (pobTran->inISOTxnCode == _SETTLE_)
        {
                if (memcmp(pobTran->srBRec.szRespCode, "95", 2) && memcmp(pobTran->srBRec.szRespCode, "00", 2))
                        inRetVal = VS_ERROR;
                else
		{
                        if (pobTran->inTransactionResult == _TRAN_RESULT_SETTLE_UPLOAD_BATCH_ && !memcmp(pobTran->srBRec.szRespCode, "95", 2))
                                inRetVal = inDINERS_ProcessSettleBatchUpload(pobTran);
		}
		
		/* 結帳成功 */
                if (inRetVal == VS_SUCCESS)
                {
                        if (inDINERS_SetReversalCnt(pobTran, _RESET_) == VS_ERROR)
                        {
                                return (VS_ERROR);
                        }
			
			/* 結帳成功 把請先結帳的bit關掉 */
			inDINERS_SetMustSettleBit(pobTran, "N");
                }
		else
		{
			/* BatchUpload失敗 */
			/* 在inDINERS_ProcessSettleBatchUpload裡面顯示主機回的錯誤訊息 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_SETTLE_NOT_SUCCESS_;
			inRetVal = VS_ERROR;
		}
		
        }
        else
        {
                if (pobTran->inTransactionResult == _TRAN_RESULT_AUTHORIZED_)
                {
                        /* 要更新端末機的日期及時間 */
                        if (inDINERS_SyncHostTerminalDateTime(pobTran) != VS_SUCCESS)
                                return (VS_ERROR);

                        if (pobTran->srBRec.uszOfflineBit == VS_FALSE)
                        {
                                inSetSendReversalBit("N");
                                if (inSaveHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
                                {
                                        return (VS_ERROR);
                                }

                                /* 因為是【Online】交易在這裡送【Advice】 */
                                inDINERS_AdviceSendRecvPacket(pobTran, 1);
                        }
                }
                else if (pobTran->inTransactionResult == _TRAN_RESULT_REFERRAL_ && pobTran->inTransactionCode == _SALE_)
                {
                        /* 要更新端末機的日期及時間 */
                        if (inDINERS_SyncHostTerminalDateTime(pobTran) != VS_SUCCESS)
                                return (VS_ERROR);

                        if (pobTran->srBRec.uszOfflineBit == VS_FALSE)
                        {
                                inSetSendReversalBit("N");
                                if (inSaveHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
                                {
                                        return (VS_ERROR);
                                }
                        }

                        if ((inRetVal = inDINERS_ProcessReferral(pobTran)) == VS_SUCCESS)
                        {
                                /* 要變成一筆【ADVICE】交易 */
                                pobTran->inTransactionResult = _TRAN_RESULT_AUTHORIZED_;
                                pobTran->srBRec.uszReferralBit = VS_TRUE;
				pobTran->srBRec.uszForceOnlineBit = VS_TRUE;
                                pobTran->srBRec.uszOfflineBit = VS_TRUE;
                                pobTran->srBRec.uszUpload1Bit = VS_TRUE;
                                pobTran->srBRec.uszUpload2Bit = VS_FALSE;
                                pobTran->srBRec.uszUpload3Bit = VS_FALSE;

                                /* Save Advice */
                                if (inADVICE_SaveAppend(pobTran, pobTran->srBRec.lnOrgInvNum) != VS_SUCCESS)
                                {
                                        inRetVal = VS_ERROR;
                                }

                                pobTran->srBRec.inCode = _SALE_OFFLINE_; /* 交易補登 */
                                pobTran->srBRec.inOrgCode = _SALE_OFFLINE_;
                                pobTran->inTransactionCode = _SALE_OFFLINE_; /* 交易補登 */
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
                                inSetSendReversalBit("N");
                                if (inSaveHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
                                {
                                        return (VS_ERROR);
                                }
                        }

			inDINERS_DispHostResponseCode(pobTran);			/* 顯示主機回的錯誤訊息 */
		      
			inRetVal = VS_ERROR;
                }
                else
                {
			inDINERS_DispHostResponseCode(pobTran);			/* 顯示主機回的錯誤訊息 */
			
                        inRetVal = VS_ERROR;
                }
        }

        return (inRetVal);
}

int inDINERS_ProcessSettleBatchUpload(TRANSACTION_OBJECT *pobTran)
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
                /* 主機回《95》但是沒有帳，還要做一次結帳 */
                inRunCLS_BATCH = VS_TRUE;
        }
        else if (inBatchTotalCnt > 0)
        {
		inBatchValidTotalCnt = inBATCH_GetTotalCount_BatchUpload_By_Sqlite(pobTran);
		
                /* 有記錄要開始執行 BATCH UPLOAD */
                pobTran->uszLastBatchUploadBit = VS_FALSE;

                memset((char *)&OrgpobTran, 0x00, sizeof(TRANSACTION_OBJECT));
                memcpy((char *)&OrgpobTran, (char *)pobTran, sizeof(TRANSACTION_OBJECT));

		/* 開始讀取 */
		inBATCH_GetDetailRecords_By_Sqlite_Enormous_START(pobTran);
		/* 預設為無須重找 */
		guszEnormousNoNeedResetBit = VS_TRUE;
		
                for (i = 0; i < inBatchTotalCnt; i ++)
                {
                        /* 這裡要顯示【批次上傳】訊息 */

			pobTran->inISOTxnCode = _BATCH_UPLOAD_;
                        /* 開始讀批次檔記錄*/
                        if (inBATCH_GetDetailRecords_By_Sqlite_Enormous_Read(pobTran, i) != VS_SUCCESS)
                        {
                                inRunCLS_BATCH = VS_FALSE;
                                break;
                        }
			
			/* 抓最新的STAN */
			inDINERS_GetSTAN(pobTran);
			
                        if (pobTran->srBRec.inCode == _VOID_ || pobTran->srBRec.inCode == _PRE_AUTH_)
                                continue;
			
			/* 表示要上傳的合法筆數 */
			inBatchValidCnt ++;


                        if (inBatchValidCnt == inBatchValidTotalCnt)
                                pobTran->uszLastBatchUploadBit = VS_TRUE; /* 最後一筆交易 */

			inRetVal = inDINERS_SendPackRecvUnPack(pobTran);
			/* 成功或失敗 System Trace Number 都要加一 */
			inDINERS_SetSTAN(pobTran);
			
                        if (inRetVal != VS_SUCCESS)
                        {
                                inRunCLS_BATCH = VS_FALSE;
                                break;
                        }
                        else
                        {
                                if (memcmp(pobTran->srBRec.szRespCode, "00", 2))
                                {
					inDINERS_DispHostResponseCode(pobTran);		/* 顯示主機回的錯誤訊息 */
                                        inRunCLS_BATCH = VS_FALSE;
                                        break;
                                }
                        }

                }
		
		/* 結束讀取 */
		inBATCH_GetDetailRecords_By_Sqlite_Enormous_END(pobTran);
		
		/* 還原成原來的【 pobTran 】 */
		memcpy((char *)pobTran, (char *)&OrgpobTran, sizeof(TRANSACTION_OBJECT));
        }

	/* 抓最新的STAN */
	inDINERS_GetSTAN(pobTran);

        if (inRunCLS_BATCH == VS_TRUE)
        {
                /* 暫時保留
                pobTran->srBRec.uszManualBit = VS_FALSE;
                pobTran->srBRec.uszVOIDBit = VS_FALSE;
                pobTran->srBRec.uszSignatureBit = VS_FALSE;
                pobTran->srBRec.uszOfflineBit = VS_FALSE;
                */

                pobTran->inISOTxnCode = _CLS_BATCH_;

                if (inDINERS_SendPackRecvUnPack(pobTran) != VS_SUCCESS)
                        return (VS_ERROR);
                else
                {
                        if (memcmp(pobTran->srBRec.szRespCode, "00", 2))
                                return (VS_ERROR); /* 一定要回【00】 */
                }
        }
        else
        {
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

/*
Function        :inDINERS_DispHostResponseCode
Date&Time       :2016/11/15 下午 6:12
Describe        :顯示錯誤代碼
*/
int inDINERS_DispHostResponseCode(TRANSACTION_OBJECT *pobTran)
{
        int		inChoice = _DisTouch_No_Event_;
	int		inRetVal = VS_SUCCESS;
	char		szResponseCode[10 + 1] = {0};
	char		szMsg[42 + 1] = {0};
        unsigned char   uszKey = 0x00;
	
	memset(szMsg, 0x00, sizeof(szMsg));
	
	sprintf(szMsg, "%s", "拒絕交易");					/* 拒絕交易 */

		
	memset(szResponseCode, 0x00, sizeof(szResponseCode));
	sprintf(szResponseCode, "%s", pobTran->srBRec.szRespCode);		/* 錯誤代碼 */
	
	/* 如果當SDK，不用顯示 */
	if (memcmp(gszTermVersionID, "MD731UAGAS001", strlen("MD731UAGAS001")) == 0)
	{
		
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

int inDINERS_SyncHostTerminalDateTime(TRANSACTION_OBJECT *pobTran)
{
        return (VS_SUCCESS);
}

int inDINERS_ProcessReferral(TRANSACTION_OBJECT *pobTran)
{
        /* 檢查是否有支援 REFERRAL */

        /* 輸入授權碼 */
        if (inCREDIT_Func_GetAuthCode(pobTran) != VS_SUCCESS)
                return (VS_ERROR);

        return (VS_SUCCESS);
}

int inDINERS_GetReversalCnt(TRANSACTION_OBJECT *pobTran)
{
        int	inCnt;
        char	szReversalCnt[6 + 1];

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inDINERS_GetReversalCnt() START!");

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

int inDINERS_SetReversalCnt(TRANSACTION_OBJECT *pobTran, long lnSetMode)
{
        long	lnCnt;
        char	szReversalCnt[6 + 1];

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

int inDINERS_GetMustSettleBit(TRANSACTION_OBJECT *pobTran, char *szMustSettleBit)
{
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inDINERS_GetMustSettleBit() START!");

	/* 刷完卡就LoadHDPT，不另在Load */
        memset(szMustSettleBit, 0x00, sizeof(szMustSettleBit));
        if (inGetMustSettleBit(szMustSettleBit) == VS_ERROR)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inDINERS_GetMustSettleBit ERROR!");
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

int inDINERS_SetMustSettleBit(TRANSACTION_OBJECT *pobTran, char *szMustSettleBit)
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
Function        :vdDINERS_ISO_FormatDebug_DISP
Date&Time       :2016/11/30 下午 4:19
Describe        :顯示ISO Debug 
*/
void vdDINERS_ISO_FormatDebug_DISP(TRANSACTION_OBJECT *pobTran, unsigned char *uszDebugBuf, int inSendLen)
{
	int			inOneLineLen = 34;
	int			i, inPrintLineCnt = 0, inCnt = 0, inField, inLen, inFieldLen;
	int			inBitMapCnt1, inBitMapCnt2;
	char			szPrtBuf[50 + 1], szBuf[1536 + 1], szBitMap[8 + 1];
	char			szPrintLineData[36 + 1];
	unsigned char		uszBitMap[9], uszBitMapDisp[66];
	ISO_TYPE_DINERS_TABLE	srISOTypeTable;

	memset((char *)&srISOTypeTable, 0x00, sizeof(srISOTypeTable));
	memcpy((char *)&srISOTypeTable, (char *)&srDINERS_ISOFunc[0], sizeof(srISOTypeTable));

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
		if (!inDINERS_BitMapCheck((unsigned char *)szBitMap, i))
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
		
		inField = inDINERS_GetFieldIndex(i, srISOTypeTable.srISOFieldType);
		inFieldLen = inDINERS_GetFieldLen(i, &uszDebugBuf[inCnt], srISOTypeTable.srISOFieldType);

		switch (srISOTypeTable.srISOFieldType[inField].inFieldType)
		{
			case _DINERS_ISO_ASC_  :
				memcpy(&szBuf[inLen], &uszDebugBuf[inCnt], inFieldLen);
				break;
                        case _DINERS_ISO_BYTE_3_  :
			case _DINERS_ISO_BYTE_3_H_  :
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
			case _DINERS_ISO_NIBBLE_2_  :
			case _DINERS_ISO_BYTE_2_  :
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
			case _DINERS_ISO_BCD_  :
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

		inCnt += inFieldLen;
	}

        inLogPrintf(AT, "==========================================");
}

/*
Function        :vdDINERS_ISO_FormatDebug_PRINT
Date&Time       :2016/11/30 下午 4:20
Describe        :列印ISO Debug
*/
void vdDINERS_ISO_FormatDebug_PRINT(TRANSACTION_OBJECT *pobTran, unsigned char *uszDebugBuf, int inSendLen)
{
	int			inOneLineLen = 34, inPrintLineCnt = 0;
	int			i, inCnt = 0, inField, inLen, inFieldLen;
	int			inBitMapCnt1, inBitMapCnt2;
	char			szPrtBuf[50 + 1], szBuf[1536 + 1], szBitMap[8 + 1];
	char			szPrintLineData[36 + 1];
	unsigned char		uszBitMap[9], uszBitMapDisp[66];
	unsigned char		uszBuffer[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	BufferHandle		srBhandle;
	FONT_ATTRIB		srFont_Attrib;
	ISO_TYPE_DINERS_TABLE	srISOTypeTable;

	/* 是否有列印功能 */
	if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
	{
		
	}
	else
	{
		/* 初始化 */
		inPRINT_Buffer_Initial(uszBuffer, _BUFFER_MAX_LINE_, &srFont_Attrib, &srBhandle);

		memset((char *)&srISOTypeTable, 0x00, sizeof(srISOTypeTable));
		memcpy((char *)&srISOTypeTable, (char *)&srDINERS_ISOFunc[0], sizeof(srISOTypeTable));

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
			if (!inDINERS_BitMapCheck((unsigned char *)szBitMap, i))
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

			inField = inDINERS_GetFieldIndex(i, srISOTypeTable.srISOFieldType);
			inFieldLen = inDINERS_GetFieldLen(i, &uszDebugBuf[inCnt], srISOTypeTable.srISOFieldType);

			switch (srISOTypeTable.srISOFieldType[inField].inFieldType)
			{
				case _DINERS_ISO_ASC_  :
					memcpy(&szBuf[inLen], &uszDebugBuf[inCnt], inFieldLen);
					break;
				case _DINERS_ISO_BYTE_3_  :
				case _DINERS_ISO_BYTE_3_H_  :
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
				case _DINERS_ISO_NIBBLE_2_  :
				case _DINERS_ISO_BYTE_2_  :
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
				case _DINERS_ISO_BCD_  :
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

			inCnt += inFieldLen;
		}

		inPRINT_Buffer_PutIn("==========================================", _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		inPRINT_Buffer_OutPut(uszBuffer, &srBhandle);
	}
}
