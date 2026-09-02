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
#include "../SOURCE/FUNCTION/CDT.h"
#include "../SOURCE/FUNCTION/CPT.h"
#include "../SOURCE/FUNCTION/CFGT.h"
#include "../SOURCE/FUNCTION/EDC.h"
#include "../SOURCE/FUNCTION/Utility.h"
#include "../SOURCE/COMM/Comm.h"
#include "../CREDIT/Creditfunc.h"
#include "AMEXiso.h"
#include "AMEXsrc.h"

unsigned char		guszAMEX_ISO_Field03[_AMEX_PCODE_SIZE_ + 1];
extern  int		ginDebug;  /* Debug使用 extern */
extern	char		gszTermVersionID[16 + 1];
extern	unsigned char	guszEnormousNoNeedResetBit;	/* 資料庫是否須從頭找 */

ISO_FIELD_AMEX_TABLE srAMEX_ISOFieldPack[] =
{
        {2,        inAMEX_Pack02},
        {3,        inAMEX_Pack03},
        {4,        inAMEX_Pack04},
        {11,       inAMEX_Pack11},
        {12,       inAMEX_Pack12},
        {13,       inAMEX_Pack13},
        {14,       inAMEX_Pack14},
	{15,       inAMEX_Pack15},
        {22,       inAMEX_Pack22},
        {24,       inAMEX_Pack24},
        {25,       inAMEX_Pack25},
        {35,       inAMEX_Pack35},
        {37,       inAMEX_Pack37},
        {38,       inAMEX_Pack38},
        {39,       inAMEX_Pack39},
        {41,       inAMEX_Pack41},
        {42,       inAMEX_Pack42},
	{44,       inAMEX_Pack44},
        {48,       inAMEX_Pack48},
        {54,       inAMEX_Pack54},
	{55,       inAMEX_Pack55},
        {60,       inAMEX_Pack60},
        {62,       inAMEX_Pack62},
        {63,       inAMEX_Pack63},
        {0,        NULL},  /* 最後一組一定要放 0 */
};

ISO_FIELD_AMEX_TABLE srAMEX_ISOFieldPack_tSAM[] =
{
        {2,        inAMEX_Pack02},
        {3,        inAMEX_Pack03},
        {4,        inAMEX_Pack04},
        {11,       inAMEX_Pack11},
        {12,       inAMEX_Pack12},
        {13,       inAMEX_Pack13},
        {14,       inAMEX_Pack14},
	{15,       inAMEX_Pack15},
        {22,       inAMEX_Pack22},
        {24,       inAMEX_Pack24},
        {25,       inAMEX_Pack25},
        {35,       inAMEX_Pack35},
        {37,       inAMEX_Pack37},
        {38,       inAMEX_Pack38},
        {39,       inAMEX_Pack39},
        {41,       inAMEX_Pack41},
        {42,       inAMEX_Pack42},
	{44,       inAMEX_Pack44},
        {48,       inAMEX_Pack48},
        {54,       inAMEX_Pack54},
	{55,       inAMEX_Pack55},
        {60,       inAMEX_Pack60},
        {62,       inAMEX_Pack62},
        {63,       inAMEX_Pack63},
        {0,        NULL},  /* 最後一組一定要放 0 */
};

ISO_FIELD_AMEX_TABLE srAMEX_ISOFieldPack_SoftWare[] =
{
        {2,        inAMEX_Pack02},
        {3,        inAMEX_Pack03},
        {4,        inAMEX_Pack04},
        {11,       inAMEX_Pack11},
        {12,       inAMEX_Pack12},
        {13,       inAMEX_Pack13},
        {14,       inAMEX_Pack14},
	{15,       inAMEX_Pack15},
        {22,       inAMEX_Pack22},
        {24,       inAMEX_Pack24},
        {25,       inAMEX_Pack25},
        {35,       inAMEX_Pack35},
        {37,       inAMEX_Pack37},
        {38,       inAMEX_Pack38},
        {39,       inAMEX_Pack39},
        {41,       inAMEX_Pack41},
        {42,       inAMEX_Pack42},
	{44,       inAMEX_Pack44},
        {48,       inAMEX_Pack48},
        {54,       inAMEX_Pack54},
	{55,       inAMEX_Pack55},
        {60,       inAMEX_Pack60},
        {62,       inAMEX_Pack62},
        {63,       inAMEX_Pack63},
        {0,        NULL},  /* 最後一組一定要放 0 */
};

ISO_FIELD_AMEX_TABLE srAMEX_ISOFieldUnPack[] =
{
        {12,       inAMEX_UnPack12},
        {13,       inAMEX_UnPack13},
        {37,       inAMEX_UnPack37},
        {38,       inAMEX_UnPack38},
        {39,       inAMEX_UnPack39},
        {0,        NULL},  /* 最後一組一定要放 0 */
};

ISO_CHECK_AMEX_TABLE srAMEX_ISOFieldCheck[] =
{
        {0,        NULL},  /* 最後一組一定要放 0 */
};

ISO_FIELD_TYPE_AMEX_TABLE srAMEX_ISOFieldType[] =
{
        {2,     _AMEX_ISO_NIBBLE_2_,    VS_FALSE,       0},
        {3,     _AMEX_ISO_BCD_,         VS_FALSE,       6},
        {4,     _AMEX_ISO_BCD_,         VS_FALSE,       12},
        {11,    _AMEX_ISO_BCD_,         VS_FALSE,       6},
        {12,    _AMEX_ISO_BCD_,         VS_FALSE,       6},
        {13,    _AMEX_ISO_BCD_,         VS_FALSE,       4},
        {14,    _AMEX_ISO_BCD_,         VS_FALSE,       4},
        {15,    _AMEX_ISO_BCD_,		VS_FALSE,	4},
        {22,    _AMEX_ISO_BCD_,         VS_FALSE,       4},
        {24,    _AMEX_ISO_BCD_,         VS_FALSE,       4},
        {25,    _AMEX_ISO_BCD_,         VS_FALSE,       2},
        {27,    _AMEX_ISO_BCD_,         VS_FALSE,       2},
        {35,    _AMEX_ISO_NIBBLE_2_,    VS_FALSE,       0},
        {37,    _AMEX_ISO_ASC_,         VS_FALSE,       12},
        {38,    _AMEX_ISO_ASC_,         VS_FALSE,       6},
        {39,    _AMEX_ISO_ASC_,         VS_FALSE,       2},
        {41,    _AMEX_ISO_ASC_,         VS_FALSE,       8},
        {42,    _AMEX_ISO_ASC_,         VS_FALSE,       15},
        {44,    _AMEX_ISO_BYTE_3_,	VS_TRUE,	0},
        {48,    _AMEX_ISO_BYTE_3_,      VS_TRUE,        0},
        {52,    _AMEX_ISO_BCD_,         VS_FALSE,       16},
        {54,    _AMEX_ISO_BYTE_3_,      VS_TRUE,        0},
        {55,    _AMEX_ISO_BYTE_3_,      VS_FALSE,       0},
        {56,    _AMEX_ISO_BYTE_3_,      VS_FALSE,       0},
        {57,    _AMEX_ISO_BYTE_3_,      VS_FALSE,       0},
        {58,    _AMEX_ISO_BYTE_3_,      VS_TRUE,        0},
        {59,    _AMEX_ISO_BYTE_3_,      VS_TRUE,        0},
        {60,    _AMEX_ISO_BYTE_3_,      VS_TRUE,        0},
        {61,    _AMEX_ISO_BYTE_3_,      VS_TRUE,        0},
        {62,    _AMEX_ISO_BYTE_3_,      VS_TRUE,        0},
        {63,    _AMEX_ISO_BYTE_3_,      VS_TRUE,        0},
        {64,    _AMEX_ISO_BCD_,         VS_FALSE,       8},
        {0,     _AMEX_ISO_BCD_,         VS_FALSE,       0},  /* 最後一組一定要放 0 */
};

int inAMEX_SALE[] = {3, 4, 11, 22, 24, 25, 35, 41, 42, 55, 62, 0}; /* 最後一組一定要放 0!! */
int inAMEX_TIP[] = {2, 3, 4, 11, 12, 13, 14, 22, 24, 25, 38, 41, 42, 54, 62, 0}; /* 最後一組一定要放 0!! */
int inAMEX_ADJUST[] = {2, 3, 4, 11, 12, 13, 14, 22, 24, 25, 38, 41, 42, 62, 0}; /* 最後一組一定要放 0!! */
int inAMEX_REFUND[] = {3, 4, 11, 22, 24, 25, 35, 41, 42, 62, 0}; /* 最後一組一定要放 0!! */
int inAMEX_VOID[] = {2, 3, 4, 11, 12, 13, 14, 22, 24, 25, 41, 42, 62, 0}; /* 最後一組一定要放 0!! */
int inAMEX_PRE_AUTH[] = {3, 4, 11, 22, 24, 25, 35, 41, 42, 55, 62, 0}; /* 最後一組一定要放 0!! */
int inAMEX_SALE_OFFLINE[] = {2, 3, 4, 11, 12, 13, 14, 22, 24, 25, 38, 41, 42, 62, 0}; /* 最後一組一定要放 0!! */
int inAMEX_ADVICE[] = {2, 3, 4, 11, 12, 13, 14, 22, 24, 25, 38, 41, 42, 62, 0}; /* 最後一組一定要放 0!! */
int inAMEX_REVERSAL[] = {2, 3, 4, 11, 14, 22, 24, 25, 41, 42, 62, 0}; /* 最後一組一定要放 0!! */
int inAMEX_SETTLE[] = {3, 11, 15, 24, 41, 42, 60, 63, 0}; /* 最後一組一定要放 0!! */
int inAMEX_CLS_BATCH[] = {3, 11, 15, 24, 41, 42, 60, 63, 0}; /* 最後一組一定要放 0!! */
int inAMEX_BATCH_UPLOAD[] = {2, 3, 4, 11, 12, 13, 14, 22, 24, 25, 37, 38, 41, 42, 60, 62, 0}; /* 最後一組一定要放 0!! */

BIT_MAP_AMEX_TABLE srAMEX_ISOBitMap[] =
{
        {_SALE_,                inAMEX_SALE,            "0200",         "004000"}, /* 銷售 */
        {_TIP_,                 inAMEX_TIP,             "0220",         "024000"}, /* 小費 */
        {_ADJUST_,              inAMEX_ADJUST,          "0220",         "024000"}, /* 調整 */
        {_REFUND_,              inAMEX_REFUND,          "0200",         "204000"}, /* 退貨 */
        {_VOID_,                inAMEX_VOID,            "0200",         "024000"}, /* 取消 */
        {_PRE_AUTH_,            inAMEX_PRE_AUTH,        "0100",         "304000"}, /* 預先授權 */
        {_SALE_OFFLINE_,        inAMEX_SALE_OFFLINE,    "0220",         "004000"}, /* 交易補登 */
        {_ADVICE_,              inAMEX_ADVICE,          "0220",         "004000"}, /* Advice 交易 */
        {_REVERSAL_,            inAMEX_REVERSAL,        "0400",         "000000"}, /* 沖銷 */
        {_SETTLE_,              inAMEX_SETTLE,          "0500",         "920000"}, /* 結帳_1 */
        {_CLS_BATCH_,           inAMEX_CLS_BATCH,       "0500",         "960000"}, /* 結帳_2 */
        {_BATCH_UPLOAD_,        inAMEX_BATCH_UPLOAD,    "0320",         "204000"}, /* 批次上傳 */
        {_AMEX_NULL_TX_,        NULL,                   "0000",         "000000"}, /* 最後一組一定要放 _AMEX_NULL_TX_!! */
};

ISO_TYPE_AMEX_TABLE srAMEX_ISOFunc[] =
{
        {
           srAMEX_ISOFieldPack,
           srAMEX_ISOFieldUnPack,
           srAMEX_ISOFieldCheck,
           srAMEX_ISOFieldType,
           srAMEX_ISOBitMap,
           inAMEX_ISOGetBitMapCode,
           inAMEX_ISOPackMessageType,
           inAMEX_ISOModifyBitMap,
           inAMEX_ISOModifyPackData,
           inAMEX_ISOCheckHeader,
           inAMEX_ISOOnlineAnalyse,
           inAMEX_ISOAdviceAnalyse
        },

        {
           srAMEX_ISOFieldPack_tSAM,
           srAMEX_ISOFieldUnPack,
           srAMEX_ISOFieldCheck,
           srAMEX_ISOFieldType,
           srAMEX_ISOBitMap,
           inAMEX_ISOGetBitMapCode,
           inAMEX_ISOPackMessageType,
           inAMEX_ISOModifyBitMap,
           inAMEX_ISOModifyPackData,
           inAMEX_ISOCheckHeader,
           inAMEX_ISOOnlineAnalyse,
           inAMEX_ISOAdviceAnalyse
        },

        {
           srAMEX_ISOFieldPack_SoftWare,
           srAMEX_ISOFieldUnPack,
           srAMEX_ISOFieldCheck,
           srAMEX_ISOFieldType,
           srAMEX_ISOBitMap,
           inAMEX_ISOGetBitMapCode,
           inAMEX_ISOPackMessageType,
           inAMEX_ISOModifyBitMap,
           inAMEX_ISOModifyPackData,
           inAMEX_ISOCheckHeader,
           inAMEX_ISOOnlineAnalyse,
           inAMEX_ISOAdviceAnalyse
        }
};

int inAMEX_Func_SetTxnOnlineOffline(TRANSACTION_OBJECT *pobTran)
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

int inAMEX_Func_BuildAndSendPacket(TRANSACTION_OBJECT *pobTran)
{
        int     inRetVal;
	int	inBatchCnt = 0;

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inAMEX_Func_BuildAndSendPacket() START!");
	
	if (pobTran->inTransactionCode == _SETTLE_)
	{
		/* 如果是結帳交易，要把請先結帳的Bit On起來，直到接收到host的回覆才Off*/
		inBatchCnt = inBATCH_GetTotalCountFromBakFile_By_Sqlite(pobTran);
		if (inBatchCnt >= 0)
		{
			inAMEX_SetMustSettleBit(pobTran, "Y");
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
        if (inAMEX_GetSTAN(pobTran) == VS_ERROR)
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
                if ((inRetVal = inAMEX_ProcessReversal(pobTran)) != VS_SUCCESS)
		{
                        return (VS_ERROR);
		}
                /* 步驟 1.4 */
                if (pobTran->inTransactionCode == _SETTLE_)
                {
                        if (inAMEX_ProcessAdvice(pobTran) != VS_SUCCESS)
			{
                                return (VS_ERROR);
			}
                }
        }

        /* 處理【ONLINE】交易 */
        if (pobTran->srBRec.uszOfflineBit == VS_FALSE)
        {
                if ((inRetVal = inAMEX_ProcessOnline(pobTran)) != VS_SUCCESS)
                {
                        inAMEX_SetSTAN(pobTran); /* 成功或失敗 System_Trace_Number 都要加一 */

                        return (VS_ERROR); /* Return 【VS_ERROR】【VS_SUCCESS】 */
                }
        }
        else
        {
                /* 處理【OFFLINE】交易 */
		inRetVal = inAMEX_ProcessOffline(pobTran);
                if (inRetVal != VS_SUCCESS)
		{
                        return (inRetVal);
		}
        }
	
	inAMEX_SetSTAN(pobTran); /* 成功或失敗 System_Trace_Number 都要加一 */

        /* 1. 表示有收到完整的資料後的分析
           2. 處理 Online & Offline 交易 */
	inRetVal = inAMEX_AnalysePacket(pobTran);
        if (inRetVal != VS_SUCCESS)
        {
                return (inRetVal);
        }

	
	/* 避免撥接太久，收送完就斷線 */
	inCOMM_End(pobTran);

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inAMEX_Func_BuildAndSendPacket() END!");

        return (VS_SUCCESS);
}

int inAMEX_Pack02(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int		inCnt = 0, inPANLen;
	char		szPAN[40 + 1];
	unsigned char	uszBCD[20 + 1];

	memset(szPAN, 0x00, sizeof(szPAN));
	inPANLen = strlen(pobTran->srBRec.szPAN);
	uszPackBuf[inCnt ++] = (inPANLen / 10 * 16) + (inPANLen % 10);
	memcpy(szPAN, pobTran->srBRec.szPAN, inPANLen);
	if (inPANLen % 2)
		szPAN[inPANLen ++] = '0';

	memset(uszBCD, 0x00, sizeof(uszBCD));
        inFunc_ASCII_to_BCD(&uszBCD[0], &szPAN[0], inPANLen/2);
	memcpy((char *)&uszPackBuf[inCnt], (char *)&uszBCD[0], (inPANLen / 2));

	inCnt += (inPANLen / 2);

	return (inCnt);
}

int inAMEX_Pack03(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int	inCnt = 0;

	memcpy(&uszPackBuf[inCnt], &guszAMEX_ISO_Field03[0], _AMEX_PCODE_SIZE_);
	inCnt += _AMEX_PCODE_SIZE_;

        return (inCnt);
}

int inAMEX_Pack04(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int		inCnt = 0;
	char		szTemplate[12 + 1];
	unsigned char	uszBCD[20 + 1];

	memset(szTemplate, 0x00, sizeof(szTemplate));

	switch (pobTran->inISOTxnCode)
	{
		case _BATCH_UPLOAD_ :
                        if (pobTran->srBRec.inCode == _TIP_)
                                sprintf(szTemplate, "%010ld00", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
                        else if (pobTran->srBRec.inCode == _ADJUST_)
                                sprintf(szTemplate, "%010ld00", pobTran->srBRec.lnAdjustTxnAmount);
                        else if (pobTran->srBRec.inCode == _VOID_)
                                sprintf(szTemplate, "000000000000");
                        else
                                sprintf(szTemplate, "%010ld00", pobTran->srBRec.lnTxnAmount);
                        break;
                case _TIP_:
                        sprintf(szTemplate, "%010ld00", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
			break;
		case _ADJUST_ :
			sprintf(szTemplate, "%010ld00", pobTran->srBRec.lnAdjustTxnAmount);
			break;
		case _VOID_ :
			sprintf(szTemplate, "000000000000");
			break;
		default :
                        sprintf(szTemplate, "%010ld00", pobTran->srBRec.lnTxnAmount);
			break;
	}

	memset(uszBCD, 0x00, sizeof(uszBCD));
        inFunc_ASCII_to_BCD(&uszBCD[0], &szTemplate[0], 6);
	memcpy((char *)&uszPackBuf[inCnt], (char *)&uszBCD[0], 6);
	inCnt += 6;

	return (inCnt);
}

int inAMEX_Pack11(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int		inCnt = 0;
	char		szSTAN[6 + 1];
	unsigned char	uszBCD[20 + 1];

	memset(szSTAN, 0x00, sizeof(szSTAN));
	sprintf(szSTAN, "%06ld", pobTran->srBRec.lnSTANNum);

	memset(uszBCD, 0x00, sizeof(uszBCD));
        inFunc_ASCII_to_BCD(&uszBCD[0], &szSTAN[0], 3);
	memcpy((char *)&uszPackBuf[inCnt], (char *)&uszBCD[0], 3);
	inCnt += 3;

	return (inCnt);
}

int inAMEX_Pack12(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int		inCnt = 0;
        unsigned char	uszBCD[20 + 1];

	memset(uszBCD, 0x00, sizeof(uszBCD));
        inFunc_ASCII_to_BCD(&uszBCD[0], &pobTran->srBRec.szTime[0], 3);
	memcpy((char *)&uszPackBuf[inCnt], (char *)&uszBCD[0], 3);
	inCnt += 3;

	return (inCnt);
}

int inAMEX_Pack13(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int		inCnt = 0;
        unsigned char	uszBCD[20 + 1];

	memset(uszBCD, 0x00, sizeof(uszBCD));
        inFunc_ASCII_to_BCD(&uszBCD[0], &pobTran->srBRec.szDate[4], 2);
	memcpy((char *)&uszPackBuf[inCnt], (char *)&uszBCD[0], 2);
	inCnt += 2;

	return (inCnt);
}

int inAMEX_Pack14(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int		inCnt = 0;
        unsigned char	uszBCD[20 + 1];

	memset(uszBCD, 0x00, sizeof(uszBCD));
        inFunc_ASCII_to_BCD(&uszBCD[0], &pobTran->srBRec.szExpDate[0], 2);
	memcpy((char *)&uszPackBuf[inCnt], (char *)&uszBCD[0], 2);
	inCnt += 2;

	return (inCnt);
}

int inAMEX_Pack15(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int		inCnt = 0;
        unsigned char	uszBCD[20 + 1];

	memset(uszBCD, 0x00, sizeof(uszBCD));
        inFunc_ASCII_to_BCD(&uszBCD[0], &pobTran->srBRec.szDate[4], 2);
	memcpy((char *)&uszPackBuf[inCnt], (char *)&uszBCD[0], 2);
	inCnt += 2;

	return (inCnt);
}

int inAMEX_Pack22(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int		inCnt = 0;
	char		sz4DBCEnable[2 + 1];
	unsigned char	uszBCD[20 + 1];

	/*
		---------------------------------------------
		Dight0	| 0
		---------------------------------------------
			| 0 Unspecified
		Dight1	| 5 Chipcard Capable
			| 6 Chipcard Capable - Fallback Mode
		---------------------------------------------
			| 1 Manual
			| 2 Magnetic Stripe
		Dight2	| 5 Chip
			| 6 Mag Stripe with 4DBC Entry
			| 7 Manual with 4DBC Entry
		---------------------------------------------
			| 0 Unspecified
			| 1 PIN Entry capability
		Dight3	| 2 No PIN Entry capability
			| 3 PIN Entry capability Off-Line
	*/
	memset(uszBCD, 0x00, sizeof(uszBCD));
	memset(sz4DBCEnable, 0x00, sizeof(sz4DBCEnable));
        inGet4DBCEnable(sz4DBCEnable);
        
	if (sz4DBCEnable[0] == 'Y')
	{
		if (pobTran->srBRec.uszManualBit == VS_TRUE)
                        inFunc_ASCII_to_BCD(&uszBCD[0], "0072", 2);
                else if (pobTran->srBRec.inCode == _SALE_ || pobTran->srBRec.inOrgCode == _SALE_ || pobTran->srBRec.inCode == _PRE_AUTH_)
		{
                        inFunc_ASCII_to_BCD(&uszBCD[0], "0062", 2);
		}
		else
		{
                        inFunc_ASCII_to_BCD(&uszBCD[0], "0022", 2);
		}
	}
	else
	{
		if (pobTran->srBRec.uszManualBit == VS_TRUE)
                        inFunc_ASCII_to_BCD(&uszBCD[0], "0012", 2);
		else
                        inFunc_ASCII_to_BCD(&uszBCD[0], "0022", 2);
	}

	memcpy((char *)&uszPackBuf[inCnt], (char *)&uszBCD[0], 2);
	inCnt += 2;

	return (inCnt);
}

int inAMEX_Pack24(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int		inCnt = 0;
	char		szTemplate[10 + 1];
	unsigned char	uszBCD[10 + 1];

	memset(szTemplate, 0x00, sizeof(szTemplate));
        if (inGetNII(&szTemplate[1]) == VS_ERROR)
        	return (VS_ERROR);
        szTemplate[0] = '0' ;

	memset(uszBCD, 0x00, sizeof(uszBCD));
        inFunc_ASCII_to_BCD(&uszBCD[0], &szTemplate[0], 2);

	memcpy((char *)&uszPackBuf[inCnt], (char *)&uszBCD[0], 2);
	inCnt += 2;

	return (inCnt);
}

int inAMEX_Pack25(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int		inCnt = 0;
	unsigned char	uszBCD[10 + 1];

	/*
		00 - Voids, refunds, offline, adjustments (adding tips)
		06 - Sale completion, below floor limit terminal approved, pre-authorisation and authorisation
	*/
	memset(uszBCD, 0x00, sizeof(uszBCD));

	if (pobTran->srBRec.inCode == _PRE_AUTH_)
                inFunc_ASCII_to_BCD(&uszBCD[0], "06", 1);
	else
                inFunc_ASCII_to_BCD(&uszBCD[0], "00", 1);

	memcpy((char *)&uszPackBuf[inCnt], (char *)&uszBCD[0], 1);
	inCnt ++;

	return (inCnt);
}

int inAMEX_Pack35(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int		inCnt = 0, i;
	char		szT2Data[128 + 1];
        short           inT2Len;
	unsigned char	uszBCD[64 + 1];

	memset(szT2Data, 0x00, sizeof(szT2Data));

	switch (pobTran->inISOTxnCode)
	{
		case _TIP_ :
		case _ADJUST_ :
		case _SALE_OFFLINE_ :
		case _REVERSAL_ :
			strcpy(szT2Data, pobTran->srBRec.szPAN);
			strcat(szT2Data, "D");
			strcat(szT2Data, pobTran->srBRec.szExpDate);
			strcat(szT2Data, pobTran->srBRec.szServiceCode);
			break;
		default :
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

				for (i = 0; i < strlen(szT2Data); i ++)
				{
					if (szT2Data[i] == '=')
						szT2Data[i] = 'D';
				}
			}

			break;
	}

	inT2Len = strlen(szT2Data);
	uszPackBuf[inCnt ++] = (inT2Len / 10 * 16) + (inT2Len % 10);

	if (inT2Len % 2)
	{
		inT2Len ++;
		strcat(szT2Data, "0");
	}

	memset(uszBCD, 0x00, sizeof(uszBCD));
        inFunc_ASCII_to_BCD(&uszBCD[0], &szT2Data[0], (inT2Len / 2));
	memcpy((char *)&uszPackBuf[inCnt], (char *)&uszBCD[0], (inT2Len / 2));
	inCnt += (inT2Len / 2);

	return (inCnt);
}

int inAMEX_Pack37(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 	inCnt = 0;

	memcpy((char *)&uszPackBuf[inCnt], &pobTran->srBRec.szRefNo[0], 12);
	inCnt += 12;

	return (inCnt);
}

int inAMEX_Pack38(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 	inCnt = 0;
	char	szAuthCode[_AUTH_CODE_SIZE_ + 1];


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

        if (pobTran->srBRec.inCode == _REFUND_ || (pobTran->srBRec.inCode == _VOID_ && pobTran->srBRec.inOrgCode == _REFUND_))
        {
                inFunc_PAD_ASCII(szAuthCode, szAuthCode, ' ', 6, _PADDING_LEFT_);
        }
        else if (pobTran->srBRec.inCode == _SALE_OFFLINE_)
        {
                sprintf(szAuthCode, pobTran->srBRec.szAuthCode);//到時候看是前補空白還是後補空白
        }
        else
        {
                strcpy(szAuthCode, pobTran->srBRec.szAuthCode);
        }

	memcpy((char *)&uszPackBuf[inCnt], &szAuthCode[0], _AUTH_CODE_SIZE_);
	inCnt += _AUTH_CODE_SIZE_;

	return (inCnt);
}

int inAMEX_Pack39(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 	inCnt = 0;

	memcpy((char *)&uszPackBuf[inCnt], &pobTran->srBRec.szRespCode[0], 2);
	inCnt += 2;

	return (inCnt);
}

int inAMEX_Pack41(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int	inCnt = 0;
	char	szTemplate[8 + 1];

	memset(szTemplate, 0x00, sizeof(szTemplate));

	if (inGetTerminalID(szTemplate) == VS_ERROR)
		return (VS_ERROR);
	inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 15, _PADDING_RIGHT_);
	memcpy((char *)&uszPackBuf[inCnt], szTemplate, strlen(szTemplate));
	inCnt += strlen(szTemplate);

	return (inCnt);
}

int inAMEX_Pack42(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int	inCnt = 0;
	char	szTemplate[15 + 1];

	memset(szTemplate, 0x00, sizeof(szTemplate));

        if (inGetMerchantID(szTemplate) == VS_ERROR)
		return (VS_ERROR);

	inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 15, _PADDING_LEFT_);
	memcpy((char *)&uszPackBuf[inCnt], szTemplate, strlen(szTemplate));
	inCnt += strlen(szTemplate);

	return (inCnt);
}

int inAMEX_Pack44(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 	inCnt = 0;

	/*
		Additional Response Data
		00 - Unspecified e.g. off-line refund,
		     sale completion and off-line sale
		     (not including below floor limit);
		01 - Response from Capture Host;
		02 - Response from stand-in processor;
		03 - Below floor limit, terminal authorised.
	*/
	uszPackBuf[inCnt ++] = 0x00;
	uszPackBuf[inCnt ++] = 0x02;
	sprintf((char *)&uszPackBuf[inCnt], "00");
	inCnt += 2;

	return (inCnt);
}

int inAMEX_Pack45(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 	inCnt = 0, inT1Len = 0;
	char 	szT1Data[128 + 1];

	/* Track 1 Data */
	memset(&szT1Data[0], 0x00, sizeof(szT1Data));
	strcpy(szT1Data, pobTran->szTrack1);
	inT1Len = strlen(pobTran->szTrack1);

	if (inT1Len % 2)
	{
		inT1Len ++;
		strcat(szT1Data, "0");

	}

	uszPackBuf[inCnt ++] = (inT1Len / 10 * 16) + (inT1Len % 10);
	memcpy((char *)&uszPackBuf[inCnt], &szT1Data[0], inT1Len);
	inCnt += inT1Len;

	return (inCnt);
}

int inAMEX_Pack48(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        return (VS_SUCCESS);
}

int inAMEX_Pack54(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int	inCnt = 0;

	/* Tip Amount */
	uszPackBuf[inCnt ++] = 0x00;
	uszPackBuf[inCnt ++] = 0x12;
	sprintf((char *)&uszPackBuf[inCnt], "%010ld00", pobTran->srBRec.lnTipTxnAmount);
	inCnt += 12;

	return (inCnt);
}

int inAMEX_Pack55(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int	inCnt = 0;

	/* 4DBC */
	uszPackBuf[inCnt ++] = 0x00;
	uszPackBuf[inCnt ++] = 0x04;
	memcpy(&uszPackBuf[inCnt], &pobTran->srBRec.szAMEX4DBC[0], 4);
	inCnt += 4;

	return	(inCnt);
}

int inAMEX_Pack60(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int	inCnt = 0;
        long	lnBatchNum;
	char    szMsgTypeId[4 + 1];
	char    szTemplate[30 + 1];

	/*
		AMEX不平帳上傳:
		Original MTI(4 bytes) + Original STAN(6 bytes) + Reserved for future use(12 bytes) Space fill
	*/
	if (pobTran->inISOTxnCode == _BATCH_UPLOAD_)
	{
		memset(szMsgTypeId, 0x00, sizeof(szMsgTypeId));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		switch (pobTran->srBRec.inCode)
		{
			case _SALE_ :
			case _REFUND_ :
				memcpy(szMsgTypeId, "\x02\x00", 2);
				break;
			case _TIP_ :
			case _ADJUST_ :
			case _SALE_OFFLINE_ :
				memcpy(szMsgTypeId, "\x02\x20", 2);
				break;
			default:
				break;
		}

    		sprintf(szTemplate, "%02x%02x%06ld            ", szMsgTypeId[0], szMsgTypeId[1], pobTran->srBRec.lnSTANNum);
            	inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 22, _PADDING_RIGHT_);

		uszPackBuf[inCnt ++] = 0x00;
		uszPackBuf[inCnt ++] = 0x22;
		memcpy((char *)&uszPackBuf[inCnt], &szTemplate[0], 22);
		inCnt += 22;
	}
	else if (pobTran->inTransactionCode == _SETTLE_)
	{
		/*
			For all reconciliation messages, this field will contain the 6 digit batch number.
			This number may not be zero and will be represented in six bytes.
		*/
		memset(szTemplate, 0x00, sizeof(szTemplate));
		if (inGetBatchNum(szTemplate) == VS_ERROR)
			return (VS_ERROR);

		lnBatchNum = atol(szTemplate);
		uszPackBuf[inCnt ++] = 0x00;
		uszPackBuf[inCnt ++] = 0x06;
		sprintf((char *)&uszPackBuf[inCnt], "%06ld", lnBatchNum);
		inCnt += 6;
	}

	return (inCnt);
}

int inAMEX_Pack62(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int	inCnt = 0;

	uszPackBuf[inCnt ++] = 0x00;
	uszPackBuf[inCnt ++] = 0x06;
	sprintf((char *)&uszPackBuf[inCnt], "%06ld", pobTran->srBRec.lnOrgInvNum);
	inCnt += 6;

	return (inCnt);
}

int inAMEX_Pack63(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int		inCnt = 0;
	int		inAccumCnt, inBatchCnt, inPacketCnt = 0;
	char		szTemplate[100], szPacket[100 + 1];
	ACCUM_TOTAL_REC	srAccumRec;

	if (pobTran->inTransactionCode == _SETTLE_ || pobTran->inTransactionCode == _CLS_BATCH_)
	{
		memset(szPacket, 0x00, sizeof(szPacket));
		inBatchCnt = inBATCH_GetTotalCountFromBakFile_By_Sqlite(pobTran);
		if (inBatchCnt == 0)
		{
                        /*  Sale Count +  Sale Amount */
			strcpy(szPacket, "000000000000000");
                        /* Refund Count +  Refund Amount */
			strcat(szPacket, "000000000000000");
			inPacketCnt += 30;
			/* Number of reversals */
			strcat(szPacket, "000");
			inPacketCnt += 3;
			/* Number of adjustments */
			strcat(szPacket, "000");
			inPacketCnt += 3;
			/* Pack Length */
			uszPackBuf[inCnt ++] = 0x00;
			uszPackBuf[inCnt ++] = 0x36;
		}
		else
		{
			memset(&srAccumRec, 0x00, sizeof(srAccumRec));
			inAccumCnt = inACCUM_GetRecord(pobTran, &srAccumRec);
			if (inAccumCnt == VS_ERROR)
				return (VS_ERROR);
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

                                /* Number of reversals */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                sprintf(szTemplate, "%03d", inAMEX_GetReversalCnt(pobTran));
                                strcat(szPacket, szTemplate);
                                inPacketCnt += 3;
                                /* Number of adjustments */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                sprintf(szTemplate, "%03lu", (srAccumRec.lnTotalTipsCount + srAccumRec.lnTotalAdjustCount));
                                strcat(szPacket, szTemplate);
                                inPacketCnt += 3;
                                /* Pack Length */
                                uszPackBuf[inCnt++] = 0x00;
                                uszPackBuf[inCnt++] = 0x36;

			}
		}
	}
	else
	{
		return (VS_ERROR);
	}

	/* Packet Data */
	memcpy((char *)&uszPackBuf[inCnt], &szPacket[0], inPacketCnt);
	inCnt += inPacketCnt;

	return (inCnt);
}

int inAMEX_UnPack12(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
	memset(pobTran->srBRec.szTime, 0x00, sizeof(pobTran->srBRec.szTime));
        inFunc_BCD_to_ASCII(&pobTran->srBRec.szTime[0], &uszUnPackBuf[0], 3);

        return (VS_SUCCESS);
}

int inAMEX_UnPack13(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
        /* 只改變月份和日期 */
        memset(&pobTran->srBRec.szDate[4], 0x00, 4);
        inFunc_BCD_to_ASCII(&pobTran->srBRec.szDate[4], &uszUnPackBuf[0], 2);

        return (VS_SUCCESS);
}

int inAMEX_UnPack37(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
        if (pobTran->inISOTxnCode == _REVERSAL_)
		return (VS_SUCCESS);

	memset(pobTran->srBRec.szRefNo, 0x00, sizeof(pobTran->srBRec.szRefNo));
	memcpy(&pobTran->srBRec.szRefNo[0], (char *)&uszUnPackBuf[0], 12);

        return (VS_SUCCESS);
}

int inAMEX_UnPack38(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
        if (pobTran->inISOTxnCode == _REVERSAL_)
		return (VS_SUCCESS);

	memset(pobTran->srBRec.szAuthCode, 0x00, sizeof(pobTran->srBRec.szAuthCode));
	memcpy(&pobTran->srBRec.szAuthCode[0], (char *)&uszUnPackBuf[0], _AUTH_CODE_SIZE_);

        return (VS_SUCCESS);
}

int inAMEX_UnPack39(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
	memcpy(&pobTran->srBRec.szRespCode[0], (char *)&uszUnPackBuf[0], 2);
	pobTran->srBRec.szRespCode[2] = 0x00;

        return (VS_SUCCESS);
}

int inAMEX_ISOGetBitMapCode(TRANSACTION_OBJECT *pobTran, int inTxnType)
{
	int	inBitMapTxnCode = -1;

	switch (inTxnType)
	{
		case _REVERSAL_ :
		case _SALE_ :
		case _SALE_OFFLINE_ :
		case _TIP_ :
		case _ADJUST_ :
		case _ADVICE_ :
		case _REFUND_ :
		case _PRE_AUTH_ :
		case _SETTLE_ :
		case _CLS_BATCH_ :
		case _BATCH_UPLOAD_ :
		case _VOID_ :
			inBitMapTxnCode = inTxnType;
			break;
		default :
			break;
	}

        return (inBitMapTxnCode);
}

int inAMEX_ISOPackMessageType(TRANSACTION_OBJECT *pobTran, int inTxnCode, unsigned char *uszPackData, char *szMTI)
{
	int		inCnt = 0;
	unsigned char	uszBCD[10 + 1];

	memset(uszBCD, 0x00, sizeof(uszBCD));
        inFunc_ASCII_to_BCD(&uszBCD[0], &szMTI[0], _AMEX_MTI_SIZE_);
	memcpy((char *)&uszPackData[inCnt], (char *)&uszBCD[0], _AMEX_MTI_SIZE_);
	inCnt += _AMEX_MTI_SIZE_;

        return (inCnt);
}

int inAMEX_ISOModifyBitMap(TRANSACTION_OBJECT *pobTran, int inTxnType, int *inBitMap)
{
	if (pobTran->inISOTxnCode == _SETTLE_ || pobTran->inISOTxnCode == _CLS_BATCH_)
		return (VS_SUCCESS); /* 因為測試到【Batch UpLoad】會亂送 */

	if (pobTran->srBRec.uszManualBit == VS_TRUE)
	{
		inAMEX_BitMapSet(inBitMap, 2);
		inAMEX_BitMapSet(inBitMap, 14);
		inAMEX_BitMapReset(inBitMap, 35);
	}

	if (strlen(pobTran->srBRec.szRefNo) > 0)
		inAMEX_BitMapSet(inBitMap, 37);

	if (pobTran->srBRec.uszUpload1Bit == VS_TRUE || pobTran->srBRec.uszUpload2Bit == VS_TRUE || pobTran->srBRec.uszUpload3Bit == VS_TRUE)
	{
		inAMEX_BitMapSet(inBitMap, 38);
	}

	inAMEX_BitMapReset(inBitMap, 45);

	/* 取消 - 小費 */
	if (pobTran->srBRec.inCode == _VOID_ && pobTran->srBRec.inOrgCode == _TIP_)
	{
		inAMEX_BitMapSet(inBitMap, 54);
	}

	/* 沒有輸入4DBC(預設有輸入) */
	if (strlen(pobTran->srBRec.szAMEX4DBC) == 0)
	{
		if (pobTran->inISOTxnCode == _SALE_  || pobTran->inISOTxnCode == _PRE_AUTH_)
			inAMEX_BitMapReset(inBitMap, 55);
	}

        return (VS_SUCCESS);
}

int inAMEX_ISOModifyPackData(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackData, int *inPackLen)
{
        return (VS_SUCCESS);
}

int inAMEX_ISOCheckHeader(TRANSACTION_OBJECT *pobTran, char *szSendISOHeader, char *szReceISOHeader)
{
        return (VS_SUCCESS);
}

int inAMEX_ISOOnlineAnalyse(TRANSACTION_OBJECT *pobTran)
{
        int	inRetVal;

	inRetVal = inAMEX_OnlineAnalyseMagneticManual(pobTran);

	return (inRetVal);
}

int inAMEX_ISOAdviceAnalyse(TRANSACTION_OBJECT *pobTran, unsigned char *uszTcUpload)
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

		/* 如果【ADVICE】刪除失敗會鎖機，使用Global的Handle */
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

int inAMEX_BitMapSet(int *inBitMap, int inFeild)
{
	int 	i, inBMapCnt, inBitMapCnt;
	int 	inBMap[_AMEX_MAX_BIT_MAP_CNT_];

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

int inAMEX_BitMapReset(int *inBitMap, int inFeild)
{
	int 	i, inBMapCnt;
	int 	inBMap[_AMEX_MAX_BIT_MAP_CNT_];

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

int inAMEX_BitMapCheck(unsigned char *inBitMap, int inFeild)
{
	int	inByteIndex, inBitIndex;

	inFeild --;
	inByteIndex = inFeild / 8;
	inBitIndex  = 7 - (inFeild - inByteIndex * 8);

	if (_AMEX_CHECK_BIT_MAP_(inBitMap[inByteIndex], inBitIndex))
	{
		return (VS_TRUE);
	}

	return (VS_FALSE);
}

int inAMEX_CopyBitMap(int *inBitMap, int *inSearchBitMap)
{
	int 	i;

	for (i = 0; i < _AMEX_MAX_BIT_MAP_CNT_; i ++)
	{
		if (inSearchBitMap[i] == 0)
			break;
		else
			inBitMap[i] = inSearchBitMap[i];
	}

	return (VS_SUCCESS);
}

int inAMEX_GetBitMapTableIndex(ISO_TYPE_AMEX_TABLE *srISOFunc, int inBitMapTxnCode)
{
	int	inBitMapIndex;

	for (inBitMapIndex = 0 ;; inBitMapIndex ++)
	{
		if (srISOFunc->srBitMap[inBitMapIndex].inTxnID == _AMEX_NULL_TX_)
		{
			return (VS_ERROR); /* 找不到相對應交易別的Bit Map */
		}

		if (srISOFunc->srBitMap[inBitMapIndex].inTxnID == inBitMapTxnCode)
			break; /* 找到一樣的交易類別 */
	}

	return (inBitMapIndex);
}

int inAMEX_GetBitMapMessagegTypeField03(TRANSACTION_OBJECT *pobTran, ISO_TYPE_AMEX_TABLE *srISOFunc, int inTxnType, int *inTxnBitMap,
                unsigned char *uszSendBuf)
{
	int		i, j, k, inBitMapIndex, *inBitMap, inCnt, inBitMapTxnCode = -1;
        char		szErrorMessage[256 + 1];
        unsigned char 	uszBuf;

	/* 設定交易別 */
	inBitMapTxnCode = srISOFunc->inGetBitMapCode(pobTran, inTxnType);
	if (inBitMapTxnCode == -1)
		return (VS_ERROR);

	/* 要搜尋 BIT_MAP_AMEX_TABLE srAMEX_ISOBitMap 相對應的 inTxnID */
	if ((inBitMapIndex = inAMEX_GetBitMapTableIndex(srISOFunc, inBitMapTxnCode)) == VS_ERROR)
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

	/* 要搜尋 BIT_MAP_AMEX_TABLE srAMEX_ISOBitMap 相對應的 inBitMap */
	inAMEX_CopyBitMap(inTxnBitMap, srISOFunc->srBitMap[inBitMapIndex].inBitMap);

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

	inCnt += _AMEX_BIT_MAP_SIZE_;

	/* Process Code */
	memset(guszAMEX_ISO_Field03, 0x00, sizeof(guszAMEX_ISO_Field03));
        inFunc_ASCII_to_BCD(&guszAMEX_ISO_Field03[0], srISOFunc->srBitMap[inBitMapIndex].szPCode, 3);

	if (inBitMapTxnCode == _REVERSAL_)
	{
		if (pobTran->inTransactionCode == _VOID_)
		{
			/*
				Processing Code		Activities
				--------------------------------------------------------
				    024000	|	Reversal Void Sale
				    320000	|	Reversal Void Pre-Auth
				    224000	|	Reversal Void Refund
			*/
			memset(guszAMEX_ISO_Field03, 0x00, sizeof(guszAMEX_ISO_Field03));
			switch (pobTran->srBRec.inOrgCode)
			{
				case _SALE_ :
				case _SALE_OFFLINE_ :
					guszAMEX_ISO_Field03[0] = 0x02;
					guszAMEX_ISO_Field03[1] = 0x40;
					break;
				case _REFUND_ :
					guszAMEX_ISO_Field03[0] = 0x22;
					guszAMEX_ISO_Field03[1] = 0x40;
					break;
				default :
					guszAMEX_ISO_Field03[0] = 0x00;
					break;
			}
		}
		else
		{
			/*
				Processing Code		Activities
				--------------------------------------------------------
				    004000	|	Reversal Sale
				    204000	|	Reversal Refund
				    304000	|	Reversal Pre-Auth
			*/
			memset(guszAMEX_ISO_Field03, 0x00, sizeof(guszAMEX_ISO_Field03));
			switch (pobTran->inTransactionCode)
			{
				case _SALE_ :
				case _SALE_OFFLINE_ :
					guszAMEX_ISO_Field03[0] = 0x00;
					guszAMEX_ISO_Field03[1] = 0x40;
					break;
				case _REFUND_ :
					guszAMEX_ISO_Field03[0] = 0x20;
					guszAMEX_ISO_Field03[1] = 0x40;
					break;
				case _PRE_AUTH_ :
					guszAMEX_ISO_Field03[0] = 0x30;
					guszAMEX_ISO_Field03[1] = 0x40;
					break;
				default :
					guszAMEX_ISO_Field03[0] = 0x00;
					break;
			}
		}
	}
	else if (inBitMapTxnCode == _BATCH_UPLOAD_)
	{
		/*
			Processing Code		Activities
			--------------------------------------------------------
			    004000	|	Reversal Sale
			    204000	|	Reversal Refund
		*/
		memset(guszAMEX_ISO_Field03, 0x00, sizeof(guszAMEX_ISO_Field03));
		switch (pobTran->srBRec.inOrgCode)
		{
			case _SALE_ :
			case _SALE_OFFLINE_ :
				guszAMEX_ISO_Field03[0] = 0x00;
				guszAMEX_ISO_Field03[1] = 0x40;
				break;
			case _REFUND_ :
				guszAMEX_ISO_Field03[0] = 0x20;
				guszAMEX_ISO_Field03[1] = 0x40;
				break;
			default :
				break;
		}

		if (pobTran->uszLastBatchUploadBit == VS_TRUE)
			guszAMEX_ISO_Field03[2] = 0x00;
		else
			guszAMEX_ISO_Field03[2] = 0x01;

	}
	else if (inBitMapTxnCode == _VOID_)
	{
		/*
			Processing Code		Activities
			--------------------------------------------------------
			    024000	|	Void Sale
			    224000	|	Void Refund
		*/
		memset(guszAMEX_ISO_Field03, 0x00, sizeof(guszAMEX_ISO_Field03));
		switch (pobTran->srBRec.inOrgCode)
		{
			case _SALE_ :
			case _SALE_OFFLINE_ :
			case _TIP_ :
			case _ADJUST_ :
				guszAMEX_ISO_Field03[0] = 0x02;
				guszAMEX_ISO_Field03[1] = 0x40;
				break;
			case _REFUND_ :
				guszAMEX_ISO_Field03[0] = 0x22;
				guszAMEX_ISO_Field03[1] = 0x40;
				break;
			default :
				guszAMEX_ISO_Field03[0] = 0x00;
				break;
		}
	}
	else if (pobTran->inTransactionCode == _TIP_ || pobTran->inTransactionCode == _ADJUST_)
	{
		/*
			Processing Code		Activities
			--------------------------------------------------------------
			    004000	|	When original sale not seen by host
			    024000	|	When original sale seen by host
		*/
		memset(guszAMEX_ISO_Field03, 0x00, sizeof(guszAMEX_ISO_Field03));
		if (pobTran->srBRec.inOrgCode == _SALE_OFFLINE_)
		{
			guszAMEX_ISO_Field03[0] = 0x00;
			guszAMEX_ISO_Field03[1] = 0x40;
		}
	}

	return (inCnt);
}

int inAMEX_PackISO(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendBuf, int inTxnCode)
{
	int			i, inSendCnt, inField, inCnt;
	int			inBitMap[_AMEX_MAX_BIT_MAP_CNT_ + 1];
	int			inRetVal, inISOFuncIndex = -1;
	char			szTemplate[40 + 1];
        char			szLogMessage[40 + 1];
	unsigned char		uszBCD[20 + 1];
	ISO_TYPE_AMEX_TABLE	srISOFunc;

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inAMEX_PackISO() START!");

	inSendCnt = 0;
	inField = 0;

	/* 加密模式，預設值 = 0。【0 = 不加密，1 = tSAM 加密，2 = 軟體加密】 */
	inISOFuncIndex = 0; /* 不加密 */
	/* 決定要執行第幾個 Function Index */
	memset((char *)&srISOFunc, 0x00, sizeof(srISOFunc));
	memcpy((char *)&srISOFunc, (char *)&srAMEX_ISOFunc[inISOFuncIndex], sizeof(srISOFunc));
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
        inFunc_ASCII_to_BCD(&uszBCD[0], &szTemplate[0], 5);
	memcpy((char *)&uszSendBuf[inSendCnt], (char *)&uszBCD[0], _AMEX_TPDU_SIZE_);
	inSendCnt += _AMEX_TPDU_SIZE_;
	/* Get Bit Map / Mesage Type / Processing Code */
	inRetVal = inAMEX_GetBitMapMessagegTypeField03(pobTran, &srISOFunc, inTxnCode, inBitMap, &uszSendBuf[inSendCnt]);
	if (inRetVal == VS_ERROR)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inAMEX_GetBitMapMessagegTypeField03() ERROR!");

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

	if (srISOFunc.inModifyPackData != _AMEX_NULL_TX_)
		srISOFunc.inModifyPackData(pobTran, uszSendBuf, &inSendCnt);

	return (inSendCnt);
}

int inAMEX_CheckUnPackField(int inField, ISO_FIELD_AMEX_TABLE *srCheckUnPackField)
{
	int 	i;

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

int inAMEX_GetCheckField(int inField, ISO_CHECK_AMEX_TABLE *ISOFieldCheck)
{
	int 	i;

	for (i = 0; i < 64; i ++)
	{
		if (ISOFieldCheck[i].inFieldNum == 0)
			break;
		else if (ISOFieldCheck[i].inFieldNum == inField)
		{
			return (i); /* i 是 ISO_CHECK_AMEX_TABLE 裡的第幾個 */
		}
	}

	return (VS_ERROR);
}

int inAMEX_GetFieldLen(int inField, unsigned char *uszSendData, ISO_FIELD_TYPE_AMEX_TABLE *srFieldType)
{
	int 	inCnt = 0, i, inLen;

	for (i = 0 ;; i ++)
	{
		if (srFieldType[i].inFieldNum == 0)
			break;

		if (srFieldType[i].inFieldNum != inField)
			continue;

		switch (srFieldType[i].inFieldType)
		{
			case _AMEX_ISO_ASC_ :
				inCnt += srFieldType[i].inFieldLen;
				break;
			case _AMEX_ISO_BCD_ :
				inCnt += srFieldType[i].inFieldLen / 2;
				break;
			case _AMEX_ISO_NIBBLE_2_ :
				inLen = (uszSendData[0] / 16 * 10) + (uszSendData[0] % 16);
				inCnt += ((inLen + 1) / 2) + 1;
				break;
			case _AMEX_ISO_NIBBLE_3_ :
				inLen = (uszSendData[0] / 16 * 1000) + (uszSendData[0] % 16 * 100) + (uszSendData[1] / 16 * 10) + (uszSendData[1] % 16);
				inCnt += ((inLen + 1) / 2) + 2;
				break;
			case _AMEX_ISO_BYTE_2_ :
				inLen = (uszSendData[0] / 16 * 10) + (uszSendData[0] % 16);
				inCnt += inLen + 1;
				break;
			case _AMEX_ISO_BYTE_3_ :
				inLen = (uszSendData[0] / 16 * 1000) + (uszSendData[0] % 16 * 100) + (uszSendData[1] / 16 * 10) + (uszSendData[1] % 16);
				inCnt += inLen + 2;
				break;
			case _AMEX_ISO_BYTE_2_H_ :
				inLen = (int)uszSendData[0];
				inCnt += inLen + 1;
				break;
			case _AMEX_ISO_BYTE_3_H_ :
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

int inAMEX_UnPackISO(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendBuf, unsigned char *uszRecvBuf)
{
	int			i, inRecvCnt, inCnt, inSendCnt, inSendField, inRecvField, inISOFuncIndex = -1;
	char			szBuf[_AMEX_TPDU_SIZE_ + _AMEX_MTI_SIZE_ + _AMEX_BIT_MAP_SIZE_ + 1];
        char 			szErrorMessage[40 + 1];
	unsigned char		uszSendMap[_AMEX_BIT_MAP_SIZE_ + 1], uszReceMap[_AMEX_BIT_MAP_SIZE_ + 1];
	ISO_TYPE_AMEX_TABLE	srISOFunc;


        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inAMEX_UnPackISO() START!");

	inSendField = inRecvField = 0;
	inSendCnt = inRecvCnt = 0;

	memset((char *)uszSendMap, 0x00, sizeof(uszSendMap));
	memset((char *)uszReceMap, 0x00, sizeof(uszReceMap));
	memset((char *)szBuf, 0x00, sizeof(szBuf));
	/* 加密模式，預設值 = 0。【0 = 不加密，1 = tSAM 加密，2 = 軟體加密】 */
	inISOFuncIndex = 0; /* 不加密 */
	memset((char *)&srISOFunc, 0x00, sizeof(srISOFunc));
	memcpy((char *)&srISOFunc, (char *)&srAMEX_ISOFunc[inISOFuncIndex], sizeof(srISOFunc));

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

	inSendCnt += _AMEX_TPDU_SIZE_;
	inRecvCnt += _AMEX_TPDU_SIZE_;
	inSendCnt += _AMEX_MTI_SIZE_;
	inRecvCnt += _AMEX_MTI_SIZE_;

	memcpy((char *)uszSendMap, (char *)&uszSendBuf[inSendCnt], _AMEX_BIT_MAP_SIZE_);
	memcpy((char *)uszReceMap, (char *)&uszRecvBuf[inRecvCnt], _AMEX_BIT_MAP_SIZE_);

	inSendCnt += _AMEX_BIT_MAP_SIZE_;
	inRecvCnt += _AMEX_BIT_MAP_SIZE_;

	/* 先檢查 ISO Field_39 */
	if (inAMEX_BitMapCheck(uszReceMap, 39) == VS_FALSE)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inAMEX_BitMapCheck(39) ERROR");

		return (VS_ERROR);
        }

	if (inAMEX_BitMapCheck(uszReceMap, 41) == VS_FALSE)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inAMEX_BitMapCheck(41) ERROR");

		return (VS_ERROR);
        }

	for (i = 1; i <= 64; i ++)
	{
		/* 有送出去的 Field 但沒有收回來的 Field */
		if (inAMEX_BitMapCheck(uszSendMap, i) && !inAMEX_BitMapCheck(uszReceMap, i))
		{
			inSendCnt += inAMEX_GetFieldLen(i, &uszSendBuf[inSendCnt], srISOFunc.srISOFieldType);
		}
		else if (inAMEX_BitMapCheck(uszReceMap, i)) /* 收的 BitMap */
		{
			if (inAMEX_BitMapCheck(uszSendMap, i)) /* 送的 BitMap 都有 */
			{
				/* 是否要進行檢查封包資料包含【送】【收】 */
				if ((inSendField = inAMEX_GetCheckField(i, srISOFunc.srCheckISO)) != VS_ERROR)
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

				inSendCnt += inAMEX_GetFieldLen(i, &uszSendBuf[inSendCnt], srISOFunc.srISOFieldType);
			}

			while (i > srISOFunc.srUnPackISO[inRecvField].inFieldNum) /* 只有收 */
			{
				inRecvField ++;
			}

			if (i == srISOFunc.srUnPackISO[inRecvField].inFieldNum)
			{
				/* 要先檢查 UnpackISO 是否存在 */
				if (inAMEX_CheckUnPackField(i, srISOFunc.srUnPackISO) == VS_SUCCESS)
					srISOFunc.srUnPackISO[inRecvField].inISOLoad(pobTran, &uszRecvBuf[inRecvCnt]);
			}

			inCnt = inAMEX_GetFieldLen(i, &uszRecvBuf[inRecvCnt], srISOFunc.srISOFieldType);
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
				if (inAMEX_BitMapCheck(uszReceMap, 38) == VS_FALSE)
                                {
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "inAMEX_BitMapCheck 38 Error!");
                                        }

					return (VS_ERROR);
                                }

				break;
			default :
				break;
		}
	}

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inAMEX_UnPackISO() END!");

	return (VS_SUCCESS);
}

int inAMEX_GetSTAN(TRANSACTION_OBJECT *pobTran)
{
        char szSTANNum[12 + 1];

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inAMEX_GetSTAN() START!");

        memset(szSTANNum, 0x00, sizeof(szSTANNum));
        if (inGetSTANNum(szSTANNum) == VS_ERROR)
                return (VS_ERROR);

        pobTran->srBRec.lnSTANNum = atol(szSTANNum);

        return (VS_SUCCESS);
}

int inAMEX_SetSTAN(TRANSACTION_OBJECT *pobTran)
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

int inAMEX_ProcessReversal(TRANSACTION_OBJECT *pobTran)
{
	int     inRetVal;
        char    szSendReversalBit[2 + 1];

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inAMEX_ProcessReversal() START!");

        memset(szSendReversalBit, 0x00, sizeof(szSendReversalBit));
        if (inGetSendReversalBit(szSendReversalBit) == VS_ERROR)
                return (VS_ERROR);

        if (!memcmp(szSendReversalBit, "Y", 1))
        {
                if ((inRetVal = inAMEX_ReversalSendRecvPacket(pobTran)) != VS_SUCCESS)
                {
                        return (VS_ERROR);
                }

                if (inAMEX_SetReversalCnt(pobTran, _ADD_) == VS_ERROR)
                {
                        return (VS_ERROR);
                }
        }

        /* 檢查是否要產生 Reversal */
        if (pobTran->uszReversalBit == VS_TRUE)
        {
                if ((inRetVal = inAMEX_ReversalSave_Flow(pobTran)) != VS_SUCCESS)
                {
                        return (VS_ERROR);
                }
        }

        return (VS_SUCCESS);
}

int inAMEX_AdviceSendRecvPacket(TRANSACTION_OBJECT *pobTran, int inAdvCnt)
{
	int			inCnt, inRetVal = VS_ERROR, inISOFuncIndex = -1;
        unsigned char		uszTCUpload = 0;
	TRANSACTION_OBJECT	ADVpobTran;
	ISO_TYPE_AMEX_TABLE	srISOFunc;

	/* 加密模式，預設值 = 0。【0 = 不加密，1 = tSAM 加密，2 = 軟體加密】 */
	inISOFuncIndex = 0; /* 不加密 */
	memset((char *)&srISOFunc, 0x00, sizeof(srISOFunc));
	memcpy((char *)&srISOFunc, (char *)&srAMEX_ISOFunc[inISOFuncIndex], sizeof(srISOFunc));

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
			inRetVal = inAMEX_SendPackRecvUnPack(&ADVpobTran);

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

int inAMEX_ProcessAdvice(TRANSACTION_OBJECT *pobTran)
{
	int	inSendAdviceCnt;

	if ((inSendAdviceCnt = inADVICE_GetTotalCount(pobTran)) == VS_NO_RECORD)
	{
		return (VS_SUCCESS);
	}
	else
	{
		if (inAMEX_AdviceSendRecvPacket(pobTran, inSendAdviceCnt) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
	}

        return (VS_SUCCESS);
}

int inAMEX_ProcessOnline(TRANSACTION_OBJECT *pobTran)
{
        int	inRetVal;
	char    szTemplate[512 + 1];
	
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inAMEX_ProcessOnline() START!");

	/* 開始組交易封包，送、收、組、解 */
	pobTran->inISOTxnCode = pobTran->inTransactionCode; /* 以 srEventMenuItem.inCode Index */
	inRetVal = inAMEX_SendPackRecvUnPack(pobTran);
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
		pobTran->inTransactionResult = inAMEX_CheckRespCode(pobTran); /* 【Field_39】 */
		
		if (ginDebug == VS_TRUE)
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "pobTran->inTransactionResult = %d", pobTran->inTransactionResult);
                        inLogPrintf(AT, szTemplate);
                }
		
		if (pobTran->inTransactionResult == _TRAN_RESULT_AUTHORIZED_)
		{
			if (inAMEX_CheckAuthCode(pobTran) != VS_SUCCESS)
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
                inLogPrintf(AT, "inAMEX_ProcessOnline() END!");

        return (VS_SUCCESS);
}

int inAMEX_ProcessOffline(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;

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

int inAMEX_AnalysePacket(TRANSACTION_OBJECT *pobTran)
{
	int			inRetVal = VS_SUCCESS, inISOFuncIndex = -1;
	ISO_TYPE_AMEX_TABLE	srISOFunc;

	if (pobTran->srBRec.uszOfflineBit == VS_FALSE)
	{
                if (pobTran->inTransactionResult == _TRAN_RESULT_AUTHORIZED_ || pobTran->inTransactionResult == _TRAN_RESULT_REFERRAL_ || pobTran->inTransactionResult == _TRAN_RESULT_SETTLE_UPLOAD_BATCH_)
		{
			/* 加密模式，預設值 = 0。【0 = 不加密，1 = tSAM 加密，2 = 軟體加密】 */
			inISOFuncIndex = 0; /* 不加密 */
			memset((char *)&srISOFunc, 0x00, sizeof(srISOFunc));
			memcpy((char *)&srISOFunc, (char *)&srAMEX_ISOFunc[inISOFuncIndex], sizeof(srISOFunc));

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

int inAMEX_ReversalSendRecvPacket(TRANSACTION_OBJECT *pobTran)
{
	int             inRetVal, inSendCnt;
	long            lnREVCnt;
	char		szDialBackupEnable[2 + 1];
	char		szCommMode[2 + 1];
        unsigned char   uszFileName[20 + 1];
	unsigned char   uszSendPacket[_AMEX_ISO_SEND_ + 1], uszRecvPacket[_AMEX_ISO_RECV_ + 1];
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
                inLogPrintf(AT, "inAMEX_CommSendRecvToHost() Before");

	if ((inRetVal = inAMEX_CommSendRecvToHost(pobTran, uszSendPacket, inSendCnt, uszRecvPacket)) != VS_SUCCESS)
	{
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inAMEX_CommSendRecvToHost() Error");

		return (VS_ERROR);
	}

	/* 解 ISO 電文 */
	if ((inRetVal = inAMEX_UnPackISO(pobTran, uszSendPacket, uszRecvPacket)) != VS_SUCCESS)
	{
		pobTran->inTransactionResult = _TRAN_RESULT_UNPACK_ERR_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_ISO_UNPACK_;
		
		return (VS_ERROR);
	}

        /* memcmp兩字串相同回傳0 */
	if (memcmp(pobTran->srBRec.szRespCode, "00", 2))
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
Function        :inAMEX_ReversalSave_Flow
Date&Time       :2017/3/30 上午 11:06
Describe        :
*/
int inAMEX_ReversalSave_Flow(TRANSACTION_OBJECT *pobTran)
{
	int 	inRetVal;
	char	szDialBackupEnable[2 + 1];
	
	inRetVal = inAMEX_ReversalSave(pobTran);
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	memset(szDialBackupEnable, 0x00, sizeof(szDialBackupEnable));
	inGetDialBackupEnable(szDialBackupEnable);

	if (memcmp(szDialBackupEnable, "Y", strlen("Y")) == 0)
	{

		inRetVal = inAMEX_ReversalSave_For_DialBeckUp(pobTran);
		if (inRetVal != VS_SUCCESS)
		{
			return (VS_ERROR);
		}

	}
	
	return (VS_SUCCESS);
}

int inAMEX_ReversalSave(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal, inPacketCnt;
	long		lnBatchNum;
	char		szTemplate[20 + 1];
	unsigned char	uszReversalPacket[_AMEX_ISO_SEND_ + 1];
        unsigned char	uszFileName[20 + 1];
	unsigned long	ulFHandle;

	memset(uszReversalPacket, 0x00, sizeof(uszReversalPacket));
	pobTran->inISOTxnCode = _REVERSAL_; /* 沖銷 */

	/* 組 REVERSAL 封包 */
	inPacketCnt = inAMEX_PackISO(pobTran, &uszReversalPacket[0], pobTran->inISOTxnCode);
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
        sprintf((char *)uszFileName, "%s%06lu%s", _FILE_NAME_AMEX_, lnBatchNum, _REVERSAL_FILE_EXTENSION_);

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
Function        :inAMEX_ReversalSave_For_DialBeckUp
Date&Time       :2017/3/30 上午 10:24
Describe        :
*/
int inAMEX_ReversalSave_For_DialBeckUp(TRANSACTION_OBJECT *pobTran)
{
        int 		inRetVal, inPacketCnt;
        unsigned char 	uszReversalPacket[_AMEX_ISO_SEND_ + 1];
        unsigned char 	uszFileName[20 + 1];
        unsigned long 	srFHandle;


        memset(uszReversalPacket, 0x00, sizeof(uszReversalPacket));
        pobTran->inISOTxnCode = _REVERSAL_; /* 沖銷 */

        /* 組_REVERSAL_ 封包 */
        inPacketCnt = inAMEX_PackISO(pobTran, &uszReversalPacket[0], pobTran->inISOTxnCode);
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

int inAMEX_CommSendRecvToHost(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, int inSendLen, unsigned char *uszRecvPacket)
{
        int		inRetVal;
        int		inSendTimeout = 10;
        int		inReceiveTimeout = 10;
        int		inReceiveSize = _COMM_RECEIVE_MAX_LENGTH_;		/* 配合Ingenico */
	char		szTimeOut[2 + 1] = {0};
        char		szHostResponseTimeOut[2 + 1] = {0};
	unsigned char	uszDispBit = VS_TRUE;

	/* 如果沒設定TimeOut，就用EDC.dat內的TimeOut */
	if (inSendTimeout <= 0)
	{
		memset(szTimeOut, 0x00, sizeof(szTimeOut));
		inGetIPSendTimeout(szTimeOut);
		inSendTimeout = atoi(szTimeOut);
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

	return (VS_SUCCESS);
}

int inAMEX_SendPackRecvUnPack(TRANSACTION_OBJECT *pobTran)
{
	int		inSendCnt = 0, inRecvCnt = 0, inRetVal;
	unsigned char	uszSendPacket[_AMEX_ISO_SEND_ + 1], uszRecvPacket[_AMEX_ISO_RECV_ + 1];

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inAMEX_SendPackRecvUnPack() START!");

	memset(uszSendPacket, 0x00, sizeof(uszSendPacket));
	memset(uszRecvPacket, 0x00, sizeof(uszRecvPacket));

	/* 組 ISO 電文 */
	if ((inSendCnt = inAMEX_PackISO(pobTran, uszSendPacket, pobTran->inISOTxnCode)) <= 0)
	{
		pobTran->inTransactionResult = _TRAN_RESULT_PACK_ERR_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_ISO_PACK_;
		
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inAMEX_PackISO() Error!");

		return (VS_ISO_PACK_ERR); /* 組交易電文錯誤，不用組 REVERSAL */
	}

	/* 傳送及接收 ISO 電文 */
	if ((inRecvCnt = inAMEX_CommSendRecvToHost(pobTran, uszSendPacket, inSendCnt, uszRecvPacket)) != VS_SUCCESS)
	{
		pobTran->inTransactionResult = _TRAN_RESULT_COMM_ERROR_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_COMM_;

                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inAMEX_CommSendRecvToHost() Error");

		if (pobTran->inISOTxnCode != _ADVICE_)
			memset(pobTran->srBRec.szRespCode, 0x00, sizeof(pobTran->srBRec.szRespCode));

		return (VS_COMM_ERROR);
	}

	/* 解 ISO 電文 */
	inRetVal = inAMEX_UnPackISO(pobTran, uszSendPacket, uszRecvPacket);
	if (inRetVal != VS_SUCCESS)
	{
		pobTran->inTransactionResult = _TRAN_RESULT_UNPACK_ERR_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_ISO_UNPACK_;
				
		inRetVal = VS_ISO_UNPACK_ERROR;
	}

	return (inRetVal);
}

int inAMEX_CheckRespCode(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = _TRAN_RESULT_COMM_ERROR_;
	
	if (!memcmp(pobTran->srBRec.szRespCode, "00", 2))
	{
		inRetVal = _TRAN_RESULT_AUTHORIZED_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_NONE_;
	}
        else if (!memcmp(pobTran->srBRec.szRespCode, "01", 2) || !memcmp(pobTran->srBRec.szRespCode, "02", 2))
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
			inRetVal = _TRAN_RESULT_UNPACK_ERR_;			/* 不是定義的 Response Code */
			pobTran->inErrorMsg = _ERROR_CODE_V3_ISO_UNPACK_;
		}
	}

	return (inRetVal);
}

int inAMEX_CheckAuthCode(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;

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

int inAMEX_OnlineAnalyseMagneticManual(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;

	if (pobTran->inISOTxnCode == _SETTLE_)
	{
		if (memcmp(pobTran->srBRec.szRespCode, "95", 2) && memcmp(pobTran->srBRec.szRespCode, "00", 2))
			inRetVal = VS_ERROR;
		else
                {
                        if (pobTran->inTransactionResult == _TRAN_RESULT_SETTLE_UPLOAD_BATCH_ && !memcmp(pobTran->srBRec.szRespCode, "95", 2))
                                inRetVal = inAMEX_ProcessSettleBatchUpload(pobTran);
                }

		/* 結帳成功 */
                if (inRetVal == VS_SUCCESS)
                {
                        if (inAMEX_SetReversalCnt(pobTran, _RESET_) == VS_ERROR)
                        {
                                return (VS_ERROR);
                        }
			
			/* 結帳成功 把請先結帳的bit關掉 */
			inAMEX_SetMustSettleBit(pobTran, "N");
                }
		else
		{
			/* BatchUpload失敗 */
			/* 在inAMEX_ProcessSettleBatchUpload裡面顯示主機回的錯誤訊息 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_SETTLE_NOT_SUCCESS_;
			inRetVal = VS_ERROR;
		}
	}
	else
	{
		if (pobTran->inTransactionResult == _TRAN_RESULT_AUTHORIZED_)
		{
			/* 要更新端末機的日期及時間 */
			if (inAMEX_SyncHostTerminalDateTime(pobTran) != VS_SUCCESS)
				return (VS_ERROR);

			if (pobTran->srBRec.uszOfflineBit == VS_FALSE)
			{
				inSetSendReversalBit("N");
				if (inSaveHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
				{
			                return (VS_ERROR);
				}

				/* 因為是【Online】交易在這裡送【Advice】 */
				inAMEX_AdviceSendRecvPacket(pobTran, 1);
			}
		}
		else if (pobTran->inTransactionResult == _TRAN_RESULT_REFERRAL_ && pobTran->inTransactionCode == _SALE_)
		{
			/* 要更新端末機的日期及時間 */
			if (inAMEX_SyncHostTerminalDateTime(pobTran) != VS_SUCCESS)
				return (VS_ERROR);

			if (pobTran->srBRec.uszOfflineBit == VS_FALSE)
			{
				inSetSendReversalBit("N");
				if (inSaveHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
				{
			                return (VS_ERROR);
				}
			}

			if ((inRetVal = inAMEX_ProcessReferral(pobTran)) == VS_SUCCESS)
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

			inAMEX_DispHostResponseCode(pobTran);		/* 顯示主機回的錯誤訊息 */
			inRetVal = VS_ERROR;
		}
		else
		{
			inAMEX_DispHostResponseCode(pobTran);		/* 顯示主機回的錯誤訊息 */
			inRetVal = VS_ERROR;
		}
	}

	return (inRetVal);
}

int inAMEX_ProcessSettleBatchUpload(TRANSACTION_OBJECT *pobTran)
{
	int			inRetVal;
	int			inBatchTotalCnt, i;
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
			inAMEX_GetSTAN(pobTran);

                        /* AE 取消(void)會上傳 batch upload */
                        if (pobTran->srBRec.inCode == _PRE_AUTH_)
				continue;

			if ((i + 1) == inBatchTotalCnt)
				pobTran->uszLastBatchUploadBit = VS_TRUE; /* 最後一筆交易 */

			inRetVal = inAMEX_SendPackRecvUnPack(pobTran);
			/* 成功或失敗 System Trace Number 都要加一 */
			inAMEX_SetSTAN(pobTran);
			
			if (inRetVal != VS_SUCCESS)
			{
				inRunCLS_BATCH = VS_FALSE;
				break;
			}
			else
			{
				if (memcmp(pobTran->srBRec.szRespCode, "00", 2))
				{
					inAMEX_DispHostResponseCode(pobTran);		/* 顯示主機回的錯誤訊息 */
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
	inAMEX_GetSTAN(pobTran);

	if (inRunCLS_BATCH == VS_TRUE)
	{
		/* 暫時保留
		pobTran->srBRec.uszManualBit = VS_FALSE;
		pobTran->srBRec.uszVOIDBit = VS_FALSE;
		pobTran->srBRec.uszSignatureBit = VS_FALSE;
		pobTran->srBRec.uszOfflineBit = VS_FALSE;
		*/

		pobTran->inISOTxnCode = _CLS_BATCH_;

		if (inAMEX_SendPackRecvUnPack(pobTran) != VS_SUCCESS)
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
Function        :inAMEX_DispHostResponseCode
Date&Time       :2016/11/15 下午 6:12
Describe        :顯示錯誤代碼
*/
int inAMEX_DispHostResponseCode(TRANSACTION_OBJECT *pobTran)
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

int inAMEX_SyncHostTerminalDateTime(TRANSACTION_OBJECT *pobTran)
{
	return (VS_SUCCESS);
}

int inAMEX_ProcessReferral(TRANSACTION_OBJECT *pobTran)
{
	/* 檢查是否有支援 REFERRAL */

	/* 輸入授權碼 */
	if (inFunc_REFERRAL_GetManualApproval(pobTran) != VS_SUCCESS)
		return (VS_ERROR);

	return (VS_SUCCESS);
}

int inAMEX_GetReversalCnt(TRANSACTION_OBJECT *pobTran)
{
        int	inCnt;
        char	szReversalCnt[6 + 1];

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inAMEX_GetReversalCnt() START!");

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

int inAMEX_SetReversalCnt(TRANSACTION_OBJECT *pobTran, long lnSetMode)
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

int inAMEX_GetMustSettleBit(TRANSACTION_OBJECT *pobTran, char *szMustSettleBit)
{
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inAMEX_GetMustSettleBit() START!");

	/* 刷完卡就LoadHDPT，不另在Load */
        memset(szMustSettleBit, 0x00, sizeof(szMustSettleBit));
        if (inGetMustSettleBit(szMustSettleBit) == VS_ERROR)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inAMEX_GetMustSettleBit ERROR!");
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

int inAMEX_SetMustSettleBit(TRANSACTION_OBJECT *pobTran, char *szMustSettleBit)
{
        if (inSetMustSettleBit(&szMustSettleBit[0]) == VS_ERROR)
                return (VS_ERROR);

        if (inSaveHDPTRec(pobTran->srBRec.inHDTIndex) < 0)
        {
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}
