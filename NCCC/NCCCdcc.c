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
#include "../SOURCE/PRINT/PrtMsg.h"
#include "../HG/HGsrc.h"
#include "../SOURCE/FUNCTION/Sqlite.h"
#include "../SOURCE/FUNCTION/Accum.h"
#include "../SOURCE/FUNCTION/Batch.h"
#include "../SOURCE/FUNCTION/Function.h"
#include "../SOURCE/FUNCTION/FuncTable.h"
#include "../SOURCE/FUNCTION/HDT.h"
#include "../SOURCE/FUNCTION/CCI.h"
#include "../SOURCE/FUNCTION/CDT.h"
#include "../SOURCE/FUNCTION/CPT.h"
#include "../SOURCE/FUNCTION/CFGT.h"
#include "../SOURCE/FUNCTION/EDC.h"
#include "../SOURCE/FUNCTION/HDPT.h"
#include "../SOURCE/FUNCTION/PWD.h"
#include "../SOURCE/FUNCTION/MVT.h"
#include "../SOURCE/FUNCTION/VWT.h"
#include "../SOURCE/FUNCTION/EST.h"
#include "../SOURCE/FUNCTION/SKM.h"
#include "../SOURCE/FUNCTION/ASMC.h"
#include "../SOURCE/FUNCTION/QAT.h"
#include "../SOURCE/FUNCTION/File.h"
#include "../SOURCE/FUNCTION/Signpad.h"
#include "../SOURCE/FUNCTION/Utility.h"
#include "../SOURCE/FUNCTION/ECR.h"
#include "../SOURCE/EVENT/MenuMsg.h"
#include "../SOURCE/EVENT/Flow.h"
#include "../SOURCE/COMM/Comm.h"
#include "../SOURCE/COMM/Ethernet.h"
#include "../SOURCE/COMM/Modem.h"
#include "../CREDIT/Creditfunc.h"
#include "../FISC/NCCCfisc.h"
#include "../EMVSRC/EMVsrc.h"
#include "../EMVSRC/EMVxml.h"
#include "../CREDIT/CreditprtByBuffer.h"
#include "../CTLS/CTLS.h"
#include "NCCCdcc.h"
#include "NCCCtSAM.h"
#include "NCCCesc.h"
#include "NCCCats.h"
#include "NCCCsrc.h"
#include "TAKAsrc.h"

unsigned char	guszNCCC_DCC_ISO_Field03[_NCCC_DCC_PCODE_SIZE_ + 1];
unsigned char	guszDCC_MTI[4 + 1];
extern  int     ginDebug;
extern	int	ginISODebug;
extern	int	ginMachineType;
extern	int	ginHalfLCD;
extern	int	ginAPVersionType;
extern	char	gszTermVersionID[16 + 1];
extern	unsigned char	guszEnormousNoNeedResetBit;	/* 資料庫是否須從頭找 */
extern	char	gszReprintDBPath[100 + 1];

unsigned char	guszField_DCC35 = VS_FALSE;
unsigned char	guszField_DCC55 = VS_FALSE;
unsigned char 	gusztSAMKeyIndex_DCC = 0x00;
unsigned char	gusztSAMCheckSum_DCC35[4 + 1];
unsigned char	gusztSAMCheckSum_DCC55[4 + 1];
int		ginDCCHostIndex = -1;

DCC_DATA        gsrDCC_Download;

/* 傳回主機反應用 */
extern EMV_CONFIG	EMVGlobConfig;

ISO_FIELD_NCCC_DCC_TABLE srNCCC_DCC_ISOFieldPack[] =
{
	{3,		inNCCC_DCC_Pack03},
	{4,		inNCCC_DCC_Pack04},
	{11,		inNCCC_DCC_Pack11},
	{12,		inNCCC_DCC_Pack12},
	{13,		inNCCC_DCC_Pack13},
	{22,		inNCCC_DCC_Pack22},
	{24,		inNCCC_DCC_Pack24},
	{25,		inNCCC_DCC_Pack25},
	{35,		inNCCC_DCC_Pack35},
	{37,		inNCCC_DCC_Pack37},
	{38,		inNCCC_DCC_Pack38},
	{41,		inNCCC_DCC_Pack41},
	{42,		inNCCC_DCC_Pack42},
	{48,		inNCCC_DCC_Pack48},
	{54,		inNCCC_DCC_Pack54},
	{55,		inNCCC_DCC_Pack55},
	{56,		inNCCC_DCC_Pack56},
	{59,		inNCCC_DCC_Pack59},
	{60,		inNCCC_DCC_Pack60},
	{62,		inNCCC_DCC_Pack62},
	{63,		inNCCC_DCC_Pack63},
	{0,		NULL}, /* 最後一組一定要放【0】!! */
};

ISO_FIELD_NCCC_DCC_TABLE srNCCC_DCC_ISOFieldPack_tSAM[] =
{
	{3,		inNCCC_DCC_Pack03},
	{4,		inNCCC_DCC_Pack04},
	{11,		inNCCC_DCC_Pack11},
	{12,		inNCCC_DCC_Pack12},
	{13,		inNCCC_DCC_Pack13},
	{22,		inNCCC_DCC_Pack22},
	{24,		inNCCC_DCC_Pack24},
	{25,		inNCCC_DCC_Pack25},
	{35,		inNCCC_DCC_Pack35_tSAM},
	{37,		inNCCC_DCC_Pack37},
	{38,		inNCCC_DCC_Pack38},
	{41,		inNCCC_DCC_Pack41},
	{42,		inNCCC_DCC_Pack42},
	{48,		inNCCC_DCC_Pack48},
	{54,		inNCCC_DCC_Pack54},
	{55,		inNCCC_DCC_Pack55_tSAM},
	{56,		inNCCC_DCC_Pack56},
	{57,		inNCCC_DCC_Pack57_tSAM},
	{59,		inNCCC_DCC_Pack59},
	{60,		inNCCC_DCC_Pack60},
	{62,		inNCCC_DCC_Pack62},
	{63,		inNCCC_DCC_Pack63},
	{0,		NULL}, /* 最後一組一定要放【0】!! */
};

ISO_FIELD_NCCC_DCC_TABLE srNCCC_DCC_ISOFieldPack_Software[] =
{
	{3,		inNCCC_DCC_Pack03},
	{4,		inNCCC_DCC_Pack04},
	{11,		inNCCC_DCC_Pack11},
	{12,		inNCCC_DCC_Pack12},
	{13,		inNCCC_DCC_Pack13},
	{22,		inNCCC_DCC_Pack22},
	{24,		inNCCC_DCC_Pack24},
	{25,		inNCCC_DCC_Pack25_Software},
	{35,		inNCCC_DCC_Pack35_Software},
	{37,		inNCCC_DCC_Pack37},
	{38,		inNCCC_DCC_Pack38},
	{41,		inNCCC_DCC_Pack41},
	{42,		inNCCC_DCC_Pack42},
	{48,		inNCCC_DCC_Pack48},
	{54,		inNCCC_DCC_Pack54},
	{55,		inNCCC_DCC_Pack55_Software},
	{56,		inNCCC_DCC_Pack56},
	{59,		inNCCC_DCC_Pack59},
	{60,		inNCCC_DCC_Pack60},
	{62,		inNCCC_DCC_Pack62},
	{63,		inNCCC_DCC_Pack63},
	{0,		NULL}, /* 最後一組一定要放【0】!! */
};

ISO_FIELD_NCCC_DCC_TABLE srNCCC_DCC_ISOFieldUnPack[] =
{
	{12,		inNCCC_DCC_UnPack12},
	{13,		inNCCC_DCC_UnPack13},
	{38,		inNCCC_DCC_UnPack38},
	{39,		inNCCC_DCC_UnPack39},
	{55,		inNCCC_DCC_UnPack55},
	{59,		inNCCC_DCC_UnPack59},
	{60,		inNCCC_DCC_UnPack60},
	{0,		NULL}, /* 最後一組一定要放【0】!! */
};

ISO_CHECK_NCCC_DCC_TABLE srNCCC_DCC_ISOFieldCheck[] =
{
	{3,		inNCCC_DCC_Check03},
	{41,		inNCCC_DCC_Check41},
	{0,		NULL}, /* 最後一組一定要放【0】!! */
};

ISO_FIELD_TYPE_NCCC_DCC_TABLE srNCCC_DCC_ISOFieldType[] =
{
	{2,	_NCCC_DCC_ISO_NIBBLE_2_,	VS_FALSE,	0},
	{3,	_NCCC_DCC_ISO_BCD_,		VS_FALSE,	6},
	{4,	_NCCC_DCC_ISO_BCD_,		VS_FALSE,	12},
	{11,    _NCCC_DCC_ISO_BCD_,		VS_FALSE,	6},
	{12,    _NCCC_DCC_ISO_BCD_,		VS_FALSE,	6},
	{13,    _NCCC_DCC_ISO_BCD_,		VS_FALSE,	4},
	{14,    _NCCC_DCC_ISO_BCD_,		VS_FALSE,	4},
	{22,    _NCCC_DCC_ISO_BCD_,		VS_FALSE,	4},
	{24,    _NCCC_DCC_ISO_BCD_,		VS_FALSE,	4},
	{25,    _NCCC_DCC_ISO_BCD_,		VS_FALSE,	2},
	{27,    _NCCC_DCC_ISO_BCD_,		VS_FALSE,	2},
	{35,    _NCCC_DCC_ISO_NIBBLE_2_,	VS_FALSE,	0},
	{37,    _NCCC_DCC_ISO_ASC_,		VS_FALSE,	12},
	{38,    _NCCC_DCC_ISO_ASC_,		VS_FALSE,	6},
	{39,    _NCCC_DCC_ISO_ASC_,		VS_FALSE,	2},
	{41,    _NCCC_DCC_ISO_ASC_,		VS_FALSE,	8},
	{42,    _NCCC_DCC_ISO_ASC_,		VS_FALSE,	15},
	{48,    _NCCC_DCC_ISO_BYTE_3_,		VS_TRUE,	0},
	{54,    _NCCC_DCC_ISO_BYTE_3_,		VS_TRUE,	0},
	{55,    _NCCC_DCC_ISO_BYTE_3_,		VS_FALSE,	0},
	{56,    _NCCC_DCC_ISO_BYTE_3_,		VS_FALSE,	0},
	{57,	_NCCC_DCC_ISO_BYTE_3_,		VS_FALSE,	0},
	{58,    _NCCC_DCC_ISO_BYTE_3_,		VS_TRUE,	0},
	{59,    _NCCC_DCC_ISO_BYTE_3_,		VS_TRUE,	0},
	{60,    _NCCC_DCC_ISO_BYTE_3_,		VS_TRUE,	0},
	{61,    _NCCC_DCC_ISO_BYTE_3_,		VS_TRUE,	0},
	{62,    _NCCC_DCC_ISO_BYTE_3_,		VS_TRUE,	0},
	{63,    _NCCC_DCC_ISO_BYTE_3_,		VS_TRUE,	0},
	{0,	_NCCC_DCC_ISO_BCD_,		VS_FALSE,	0}, /* 最後一組一定要放【0】 !! */
};

int inNCCC_DCC_RATE[] = {3, 4, 11, 24, 35, 41, 42, 59, 60, 0}; /* 最後一組一定要放 0!! */
int inNCCC_DCC_SALE[] = {3, 4, 11, 22, 24, 25, 35, 37, 41, 42, 59, 60, 62, 0}; /* 最後一組一定要放 0!! */
int inNCCC_DCC_REFUND[] = {3, 4, 11, 22, 24, 25, 35, 37, 38, 41, 42, 59, 60, 62, 0}; /* 最後一組一定要放 0!! */
int inNCCC_DCC_PRE_COMP[] = {3, 4, 11, 12, 13, 22, 24, 25, 35, 37, 38, 41, 42, 59, 60, 62, 0}; /* 最後一組一定要放 0!! */
int inNCCC_DCC_TIP[] = {3, 4, 11, 12, 13, 22, 24, 25, 35, 37, 38, 41, 42, 54, 59, 60, 62, 0}; /* 最後一組一定要放 0!! */
int inNCCC_DCC_REVERSAL[] = {3, 4, 11, 22, 24, 25, 35, 37, 41, 42, 59, 60, 62, 0}; /* 最後一組一定要放 0!! */
int inNCCC_DCC_ICCTcUpload[] = {3, 4, 11, 12, 13, 22, 24, 25, 35, 37, 38, 41, 42, 55, 56, 59, 60, 62, 0}; /* 最後一組一定要放 0!! */
//inNCCC_DCC_SEND_ADVICE[] = {3, 4, 11, 12, 13, 22, 24, 25, 35, 37, 38, 41, 42, 59, 60, 62, 0}; /* 最後一組一定要放 0!! */
int inNCCC_DCC_VOID[] = {3, 4, 11, 22, 24, 25, 35, 37, 38, 41, 42, 59, 60, 62, 0}; /* 最後一組一定要放 0!! */ /* 【需求單 - 104024】上傳電子簽單至ESC系統，取消要加帶F_59 by Tusin - 2015/12/23 下午 02:44:03 */ 
int inNCCC_DCC_SETTLE[] = {3, 11, 24, 41, 42, 60, 63, 0}; /* 最後一組一定要放 0!! */
int inNCCC_DCC_CLS_BATCH[] = {3, 11, 24, 41, 42, 60, 63, 0}; /* 最後一組一定要放 0!! */
int inNCCC_DCC_BATCH_UPLOAD[] = {3, 4, 11, 12, 13, 22, 24, 25, 35, 37, 38, 41, 42, 59, 60, 62, 0}; /* 最後一組一定要放 0!! */
int inNCCC_DCC_UPDATE_INF[] = {3, 11, 24, 41, 42, 59, 0};
int inNCCC_DCC_BIN[] = {3, 11, 24, 41, 42, 59, 0};
int inNCCC_DCC_UPDATE_CLOSE[] = {3, 11, 24, 41, 42, 59, 0};
int inNCCC_DCC_EX_RATE[] = {3, 11, 24, 41, 42, 0};
int inNCCC_DCC_SEND_ESC_ADVICE[] = {3, 4, 11, 12, 13, 22, 24, 25, 35, 37, 38, 41, 42, 59, 60, 62, 0}; /* 最後一組一定要放 0!! */

BIT_MAP_NCCC_DCC_TABLE srNCCC_DCC_ISOBitMap[] =
{
	{_DCC_RATE_,		inNCCC_DCC_RATE,		"0340",		"000000"}, /* 詢價 */
	{_SALE_,		inNCCC_DCC_SALE,		"0200",		"000000"}, /* 銷售 */
	{_TIP_,			inNCCC_DCC_TIP,			"0220",		"020000"}, /* 小費 */
	{_REFUND_,		inNCCC_DCC_REFUND,		"0200",		"200000"}, /* 退貨 */
	{_PRE_COMP_,		inNCCC_DCC_PRE_COMP,		"0220",		"000000"}, /* 預先授權完成 */
	{_REVERSAL_,		inNCCC_DCC_REVERSAL,		"0400",		"000000"}, /* 沖銷 */
	{_TC_UPLOAD_,		inNCCC_DCC_ICCTcUpload,		"0220",		"250000"}, /* 晶片卡 TC UPLOAD */
//	{_SEND_ADVICE_,		inNCCC_DCC_SEND_ADVICE,		"0220",		"000000"}, /* Advice 交易 */
	{_VOID_,		inNCCC_DCC_VOID,		"0200",		"020000"}, /* 取消 */
	{_SETTLE_,		inNCCC_DCC_SETTLE,		"0500",		"920000"}, /* 結帳_1 */
	{_CLS_BATCH_,		inNCCC_DCC_CLS_BATCH,		"0500",		"960000"}, /* 結帳_2 */
	{_BATCH_UPLOAD_,	inNCCC_DCC_BATCH_UPLOAD,	"0320",		"000001"}, /* 批次上傳，【xxxxx1】表示要繼續上傳交易 */
        {_DCC_UPDATE_INF_,	inNCCC_DCC_UPDATE_INF,		"0340",         "000000"}, /* 參數下載資訊上傳 */
        {_DCC_BIN_,		inNCCC_DCC_BIN,			"0340",         "000000"}, /* 參數下載內容 */
        {_DCC_UPDATE_CLOSE_,	inNCCC_DCC_UPDATE_CLOSE,	"0340",         "000000"}, /* 參數下載結尾 */
        {_DCC_EX_RATE_,		inNCCC_DCC_EX_RATE,		"0340",		"610000"}, /* 匯率下載 */
        {_SEND_ESC_ADVICE_,	inNCCC_DCC_SEND_ESC_ADVICE,	"0220",		"690000"}, /* ESC Advice */
	{_NCCC_DCC_NULL_TX_,	NULL,				"0000",		"000000"}, /* 最後一組一定要放 TMS_NULL_TX!! */
};

ISO_TYPE_NCCC_DCC_TABLE srNCCC_DCC_ISOFunc[] =
{
        {
           srNCCC_DCC_ISOFieldPack,
           srNCCC_DCC_ISOFieldUnPack,
           srNCCC_DCC_ISOFieldCheck,
           srNCCC_DCC_ISOFieldType,
           srNCCC_DCC_ISOBitMap,
           inNCCC_DCC_ISOPackMessageType,
           inNCCC_DCC_ISOModifyBitMap,
           inNCCC_DCC_ISOModifyPackData,
           inNCCC_DCC_ISOCheckHeader,
           inNCCC_DCC_ISOOnlineAnalyse,
           inNCCC_DCC_ISOAdviceAnalyse
        },
	
	/* tSAM用 */
        {
           srNCCC_DCC_ISOFieldPack_tSAM,
           srNCCC_DCC_ISOFieldUnPack,
           srNCCC_DCC_ISOFieldCheck,
           srNCCC_DCC_ISOFieldType,
           srNCCC_DCC_ISOBitMap,
           inNCCC_DCC_ISOPackMessageType,
           inNCCC_DCC_ISOModifyBitMap,
           inNCCC_DCC_ISOModifyPackData,
           inNCCC_DCC_ISOCheckHeader,
           inNCCC_DCC_ISOOnlineAnalyse,
           inNCCC_DCC_ISOAdviceAnalyse
        },
	
	/* 軟加用 */
        {
           srNCCC_DCC_ISOFieldPack_Software,
           srNCCC_DCC_ISOFieldUnPack,
           srNCCC_DCC_ISOFieldCheck,
           srNCCC_DCC_ISOFieldType,
           srNCCC_DCC_ISOBitMap,
           inNCCC_DCC_ISOPackMessageType,
           inNCCC_DCC_ISOModifyBitMap,
           inNCCC_DCC_ISOModifyPackData,
           inNCCC_DCC_ISOCheckHeader,
           inNCCC_DCC_ISOOnlineAnalyse,
           inNCCC_DCC_ISOAdviceAnalyse
        }
};

/*
Function        :inNCCC_DCC_Func_SetTxnOnlineOffline
Date&Time       :2016/9/14 上午 11:52
Describe        :根據交易別決定是否Online
Log				:Done
*/
int inNCCC_DCC_Func_SetTxnOnlineOffline(TRANSACTION_OBJECT *pobTran)
{
        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_Func_SetTxnOnlineOffline START!");
        
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inNCCC_DCC_Func_SetTxnOnlineOffline START!");
        }
		
	/* DCC不能離線授權 */
	if (!memcmp(pobTran->srBRec.szAuthCode, "Y1", 2))
	{
		vdUtility_SYSFIN_LogMessage(AT, "DCC AuthCode:(%s) is not allow to be offline",pobTran->srBRec.szAuthCode);
		return (VS_ERROR);
	}

	/* 【DCC】交易目前全部都是Online交易 */
	pobTran->srBRec.uszForceOnlineBit = VS_TRUE;
	pobTran->srBRec.uszOfflineBit = VS_FALSE;
	pobTran->srBRec.uszUpload1Bit = VS_FALSE; /* 不會產生任何 Advice */
	pobTran->srBRec.uszUpload2Bit = VS_FALSE; /* 不會產生任何 Advice */
	pobTran->srBRec.uszUpload3Bit = VS_FALSE; /* 不會產生任何 Advice */
	
	/* Settle不需要做reversal */
	pobTran->uszReversalBit = VS_TRUE;
	
	if (pobTran->inTransactionCode == _SETTLE_)
	{
		pobTran->uszReversalBit = VS_FALSE;
	}
	
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inNCCC_DCC_Func_SetTxnOnlineOffline END!");
        }    
        
        return (VS_SUCCESS);
}

/*
Function        :inNCCC_DCC_Func_BuildAndSendPacket
Date&Time       :2016/9/6 下午 2:41
Describe        :
*/
int inNCCC_DCC_Func_BuildAndSendPacket(TRANSACTION_OBJECT *pobTran)
{
	int	inHDTIndex = -1;
	int	inRetVal = VS_ERROR;
	int	inBatchCnt = 0;
	char	szDemoMode[2 + 1] = {0};
	char	szDebugMsg[100 + 1] = {0};
	char	szCustomIndicator[3 + 1] = {0};
	unsigned char	uszFileName[20 + 1] = {0};
	DUTYFREE_REPRINT_TITLE	srReprintTitle;
	SQLITE_ALL_TABLE	srALL;
	
        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_Func_BuildAndSendPacket START!");
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_DCC_Func_BuildAndSendPacket() START !");
	}
	
	memset(szCustomIndicator, 0x00, sizeof(szCustomIndicator));
	inGetCustomIndicator(szCustomIndicator);
	/* 昇恆昌客製化075，進入收送電文流程就檢查是否有前批的紀錄要清掉 */
	if  (!memcmp(szCustomIndicator, _CUSTOMER_INDICATOR_075_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
	     !memcmp(szCustomIndicator, _CUSTOMER_INDICATOR_103_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_))
	{
		memset(&srReprintTitle, 0x00, sizeof(DUTYFREE_REPRINT_TITLE));
		memset(&srALL, 0x00, sizeof(SQLITE_ALL_TABLE));
		inBatch_Table_Link_Reprint_Title(&srReprintTitle, &srALL, _LS_READ_);
		inSqlite_Get_Table_ByRecordID_All(gszReprintDBPath, _TABLE_NAME_REPRINT_TITLE_, 0, &srALL);
		if (memcmp(srReprintTitle.szSettleReprintDCCEnable, "Y", 1) == 0)
		{
			/* 兩個都是Y代表都沒有交易了 */
			if (memcmp(srReprintTitle.szSettleReprintNCCCEnable, "Y", 1) == 0	&&
			    memcmp(srReprintTitle.szSettleReprintDCCEnable, "Y", 1) == 0	&&
			    pobTran->inRunOperationID != _OPERATION_SETTLE_)
			{
				memset(srReprintTitle.szTMSUpdateSuccessNum, 0x00, sizeof(srReprintTitle.szTMSUpdateSuccessNum));
				sprintf(srReprintTitle.szTMSUpdateSuccessNum, "000");
				memset(srReprintTitle.szTMSUpdateFailNum, 0x00, sizeof(srReprintTitle.szTMSUpdateFailNum));
				sprintf(srReprintTitle.szTMSUpdateFailNum, "000");
				/* 刪除重印明細 */
				inSqlite_Drop_Table(gszReprintDBPath, _TABLE_NAME_REPRINT_NCCC_);
				inSqlite_Drop_Table(gszReprintDBPath, _TABLE_NAME_REPRINT_DCC_);
				/* 刪除TMS通知 */
				memset(uszFileName, 0x00, sizeof(uszFileName));
				sprintf((char*)uszFileName, "%s%s", _DUTY_FREE_REPRINT_TMS_NOTIFY_FILE_NAME_, _DUTY_FREE_REPRINT_TOTAL_REPORT_FILE_EXTENSION_);
				inFILE_Delete(uszFileName);
			}
			
			memset(srReprintTitle.szSettleReprintDCCEnable, 0x00, sizeof(srReprintTitle.szSettleReprintDCCEnable));
			sprintf(srReprintTitle.szSettleReprintDCCEnable, "N");
			memset(&srALL, 0x00, sizeof(SQLITE_ALL_TABLE));
			inBatch_Table_Link_Reprint_Title(&srReprintTitle, &srALL, _LS_UPDATE_);
			inSqlite_Insert_Or_Replace_ByRecordID_All(gszReprintDBPath, _TABLE_NAME_REPRINT_TITLE_, 0, &srALL);
		}
	}
	
	/* 教育訓練模式 */
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		inRetVal = inNCCC_Func_BuildAndSendPacket_Demo_Flow(pobTran);
		
		/* 步驟 1 Get System Trans Number */
		inNCCC_DCC_GetSTAN(pobTran);
		/* Set STAN */
		inNCCC_DCC_SetSTAN(pobTran);
		
		/* 交易失敗 */
		if (inRetVal != VS_SUCCESS)
		{
			return (inRetVal);
		}

		return (VS_SUCCESS);
	}
	else
	{
		/* 防呆，若跑這隻function，卻沒on起來，代表不能做DCC交易（Ex:做DCC取消，結果舊參數因某種原因不見） */
		if (pobTran->srBRec.uszDCCTransBit != VS_TRUE)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "不能做DCC交易, uszDCCTransBit沒On");
				inLogPrintf(AT, szDebugMsg);
			}
			return (VS_ERROR);
		}

		/* 切換到DCC Host */
		inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_DCC_, &inHDTIndex);
		if (inHDTIndex == -1)
		{
			return (VS_ERROR);
		}
		else
		{
			pobTran->srBRec.inHDTIndex = inHDTIndex;
		}

		inLoadHDTRec(pobTran->srBRec.inHDTIndex);
		inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
		
		/* 如果是結帳交易，要把請先結帳的Bit On起來，直到接收到host的回覆才Off*/
		if (pobTran->inTransactionCode == _SETTLE_)
		{
			/* 如果是結帳交易，要把請先結帳的Bit On起來，直到接收到host的回覆才Off*/
			inBatchCnt = inBATCH_GetTotalCountFromBakFile_By_Sqlite(pobTran);
			if (inBatchCnt >= 0)
			{
				inNCCC_DCC_SetMustSettleBit(pobTran, "Y");
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
		if (inNCCC_DCC_GetSTAN(pobTran) == VS_ERROR)
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
                                        /* Mirror Message */
                                        if (pobTran->uszECRBit == VS_TRUE)
                                        {
                                                inECR_SendMirror(pobTran, _MIRROR_MSG_COMM_ERROR_);
                                        }
                                        
					/* 通訊失敗‧‧‧‧ */
					pobTran->inTransactionResult = _TRAN_RESULT_COMM_ERROR_;
					pobTran->inErrorMsg = _ERROR_CODE_V3_COMM_;
					inFunc_Display_Error(pobTran);			/* 通訊失敗 */

					return (VS_COMM_ERROR);
				}
			}

			/* 步驟 1.3 檢查是否為 Online 交易，先送上筆交易失敗的 Reversal 及產生當筆交易 Reversal */
			if ((inRetVal = inNCCC_DCC_ProcessReversal(pobTran)) != VS_SUCCESS)
			{
				return (inRetVal);
			}

			/* 步驟 1.4 */
			if (pobTran->inTransactionCode == _SETTLE_)
			{
				if ((inRetVal = inNCCC_DCC_ProcessAdvice(pobTran)) != VS_SUCCESS)
				{
					/* 只有結帳時前送advice錯誤時要顯示錯誤訊息，在inNCCC_DCC_ISOAdviceAnalyse裡顯示錯誤訊息 */
					pobTran->inErrorMsg = _ERROR_CODE_V3_SETTLE_NOT_SUCCESS_;
					return (inRetVal);
				}

				if ((inRetVal = inNCCC_DCC_ProcessAdvice_ESC(pobTran)) != VS_SUCCESS)
				{
					/* 只有結帳時前送advice錯誤時要顯示錯誤訊息，在inNCCC_DCC_ISOAdviceAnalyse裡顯示錯誤訊息 */
					pobTran->inErrorMsg = _ERROR_CODE_V3_SETTLE_NOT_SUCCESS_;
					return (inRetVal);
				}

			}

		}

		/* 處理【ONLINE】交易 */
		if (pobTran->srBRec.uszOfflineBit == VS_FALSE)
		{
			if ((inRetVal = inNCCC_DCC_ProcessOnline(pobTran)) != VS_SUCCESS)
			{
				inNCCC_DCC_SetSTAN(pobTran); /* 成功或失敗 System_Trace_Number 都要加一 */

				return (inRetVal); /* Return 【VS_ERROR】【VS_SUCCESS】 */
			}
		}
		else
		{
			/* 處理【OFFLINE】交易 */
			inRetVal = inNCCC_DCC_ProcessOffline(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
		}

		inNCCC_DCC_SetSTAN(pobTran); /* 成功或失敗 System_Trace_Number 都要加一 */

		/* 1. 表示有收到完整的資料後的分析
		   2. 處理 Online & Offline 交易
		 */
		inRetVal = inNCCC_DCC_AnalysePacket(pobTran);
		if (inRetVal != VS_SUCCESS)
		{
			return (inRetVal);
		}


		/* 避免撥接太久，收送完就斷線 */
		/* 這裡判斷是否要送TC Upload再決定是否斷線 */
		if (pobTran->uszEMVProcessDisconectBit != VS_TRUE)
		{
			inCOMM_End(pobTran);
		}

		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inNCCC_DCC_Func_BuildAndSendPacket() END !");
			inLogPrintf(AT, "----------------------------------------");

		}

		return (VS_SUCCESS);
	}
}

int inNCCC_DCC_Pack03 (TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int inCnt = 0;
	memcpy (&uszPackBuf[inCnt], &guszNCCC_DCC_ISO_Field03[0], _NCCC_DCC_PCODE_SIZE_);
	inCnt += _NCCC_DCC_PCODE_SIZE_;
	
	return (inCnt);
}

/*
Function        :inNCCC_DCC_Pack04
Date&Time       :
Describe        :Field_4:       Amount, Transaction
*/
int inNCCC_DCC_Pack04(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 		inCnt = 0;
        char 		szTemplate[100 + 1], szDebugMsg[100 + 1];
	char		szAscii[12 + 1];
        unsigned char 	uszBCD[6 + 1];

	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_DCC_Pack04() START!");
	
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
	
	if (ginDebug == VS_TRUE)
	{
		memset(szAscii, 0x00, sizeof(szAscii));
		inFunc_BCD_to_ASCII(szAscii, uszBCD, 6);
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "F_04 [AMT %s]", szAscii);
		inLogPrintf(AT, szDebugMsg);
	}
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_DCC_Pack04() END!");
	
        return (inCnt);
}

/*
Function        :inNCCC_DCC_Pack11
Date&Time       :2016/9/22 下午 5:29
Describe        :
*/
int inNCCC_DCC_Pack11 (TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int		inCnt = 0;
	char		szSTAN[6 + 1];
	unsigned char	uszBCD[3 + 1];
	
	if (pobTran->inISOTxnCode == _SEND_ESC_ADVICE_)
	{
		memset(szSTAN, 0x00, sizeof(szSTAN));
		/* For ESC電子簽單，Advice 使用當下STAN */
		inGetSTANNum(szSTAN);
	}
	else
	{
		memset(szSTAN, 0x00, sizeof(szSTAN));
		sprintf(szSTAN, "%06ld", pobTran->srBRec.lnSTANNum);
	}
	
	memset(uszBCD, 0x00, sizeof(uszBCD));
	inFunc_ASCII_to_BCD(uszBCD, szSTAN, 3);
	memcpy((char*) &uszPackBuf[inCnt], (char *)&uszBCD[0], 3);
	
	inCnt += 3;
	
	return (inCnt);
}

/*
Function        :inNCCC_DCC_Pack12
Date&Time       :
Describe        :Field_12:      Time, Local Transaction
*/
int inNCCC_DCC_Pack12(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 		inCnt = 0;
	char		szDebugMsg[100 + 1];
	char		szAscii[6 + 1];
        unsigned char 	uszBCD[3 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_DCC_Pack12() START!");
	

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
                inLogPrintf(AT, "inNCCC_DCC_Pack12() END!");
	
        return (inCnt);
}

/*
Function        :inNCCC_DCC_Pack13
Date&Time       :
Describe        :Field_13:      Date, Local Transaction
*/
int inNCCC_DCC_Pack13(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 		inCnt = 0;
	char		szDebugMsg[100 + 1];
	char		szAscii[4 + 1];
        unsigned char 	uszBCD[2 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_DCC_Pack13() START!");

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
                inLogPrintf(AT, "inNCCC_DCC_Pack13() END!");

        return (inCnt);
}

/*
Function        :inNCCC_DCC_Pack22
Date&Time       :
Describe        :Field_22:   Point of Service Entry Mode
*/
int inNCCC_DCC_Pack22(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
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
                inLogPrintf(AT, "inNCCC_DCC_Pack22() START!");
	
	/* 1.比對TerminalCapbility */
	memset(szTerminalCapbility, 0x00, sizeof(szTerminalCapbility));
	if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_VISA_, strlen(_CARD_TYPE_VISA_)) == 0)
	{
		strcpy(szExpectApplicationIndex, _MVT_VISA_NCCC_INDEX_);	/* VISA */
	}
	else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)) == 0)
	{
		strcpy(szExpectApplicationIndex, _MVT_MCHIP_NCCC_INDEX_);	/* MASTERCARD */
	}
	else
	{
		return (VS_ERROR);		/* 防呆 */
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
	/* 和 ATS邏輯同步 (2024/6/19 下午 4:36) */
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
	/* 不是CUP，則pin bit一律為True 和 ATS邏輯同步 (2024/6/19 下午 4:36)*/
	else
	{
		uszPinBit = VS_TRUE;
	}
	
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
	/* 信用卡 */
	/* 晶片卡Fallback轉磁條 */
	if (pobTran->srBRec.uszEMVFallBackBit == VS_TRUE)
	{
		if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)) == 0)
		{
			strcat(szPOSmode, "80");
		}
		else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_VISA_, strlen(_CARD_TYPE_VISA_)) == 0)
		{
			strcat(szPOSmode, "90");
		}
		/* DCC 目前不支援JCB */
//		else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_JCB_, strlen(_CARD_TYPE_JCB_)) == 0)
//		{
//			strcat(szPOSmode, "97");
//		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "規格沒有此卡");
				inLogPrintf(AT, szDebugMsg);
			}
			return (VS_ERROR);
		}
		
	}
	/* 晶片卡一律05 */
	else if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
	{
		strcat(szPOSmode, "05");
	}
	/* 【需求單 - 106252】	DCC交易新增感應及BIN11碼需求 by Russell 2020/1/14 下午 3:37 */
	else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
	{
		/* MASTERCARD 和 NewJPeedy 感應磁條卡送91 */
		if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE	||
		    pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_64_NEWJSPEEDY_MSD	||
		    pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE)
		{
			strcat(szPOSmode, "91");
		}
		/* 除了特例外，只要contactless就是 07 */
		else
			strcat(szPOSmode, "07");
	}
	/* 02是磁卡 */
	else if (pobTran->srBRec.uszManualBit == VS_TRUE)
	{
		strcat(szPOSmode, "01");
	}
	/* 01是manual keyin */
	else
	{
		strcat(szPOSmode, "02");
	}
	
	/* Position 4 */
	if (uszPinBit == VS_TRUE)
		strcat(szPOSmode, "1");
	else
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
                inLogPrintf(AT, "inNCCC_DCC_Pack22() END!");
	
        return (inCnt);
}

/*
Function        :inNCCC_DCC_Pack24
Date&Time       :2016/9/30 下午 6:21
Describe        :NII
*/
int inNCCC_DCC_Pack24(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int		inCnt = 0;
	char		szTemplate[10 + 1];
	unsigned char	uszBCD[2 + 1];
	
	memset(szTemplate, 0x00, sizeof(szTemplate));
	
	/* NPS Indicator ( Response from Host) */
	if (pobTran->srBRec.uszField24NPSBit == VS_TRUE)
	{
		szTemplate[0] = '9';
	}
	/* Unspecified */
	else
	{
		szTemplate[0] = '0';
	}
	
	/* DCC FES:345 */
	if (inGetNII(&szTemplate[1]) == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
	memset(uszBCD, 0x00, sizeof(uszBCD));
	inFunc_ASCII_to_BCD(uszBCD, szTemplate, 2);
	memcpy((char*) &uszPackBuf[inCnt], (char *)&uszBCD[0], 2);
	
	inCnt += 2;
	
	return (inCnt);
}

/*
Function        :inNCCC_DCC_Pack25
Date&Time       :2016/9/30 下午 6:27
Describe        :Field_25:   Point of Service Condition Code
*/
int inNCCC_DCC_Pack25(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 		inCnt = 0;
	char		szTemplate[2 + 1];
	char		szDebugMsg[100 + 1];
	char		szAscii[2 + 1];
        unsigned char 	uszBCD[1 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_DCC_Pack25() START!");

        /*
                    Code        |       Meaning
                ---------------------------------------------------
                    00          |       Normal presentment
         */
	/* 初值設為00 */
	inFunc_ASCII_to_BCD(&uszPackBuf[inCnt], "00", 1);
	
        memset(uszBCD, 0x00, sizeof(uszBCD));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(szTemplate, "00", 2);
	
	/* Pre compelete是06，Batch Upload要跟原交易一樣 */
	if (pobTran->srBRec.inOrgCode == _PRE_COMP_)
		memcpy(szTemplate, "06", 2);
	else
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
                inLogPrintf(AT, "inNCCC_DCC_Pack25() END!");

        return (inCnt);
}

/*
Function        :inNCCC_DCC_Pack25_Software
Date&Time       :2016/10/4 上午 10:21
Describe        :Field_25:   Point of Service Condition Code
*/
int inNCCC_DCC_Pack25_Software(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 		inCnt = 0;
	char		szTemplate[2 + 1];
	char		szDebugMsg[100 + 1];
	char		szAscii[2 + 1];
        unsigned char 	uszBCD[1 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_DCC_Pack25_Software() START!");

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
	
	if (pobTran->uszDialBackup == VS_TRUE)
	{
		/* Pre compelete是06，Batch Upload要跟原交易一樣 */
		if (pobTran->srBRec.inOrgCode == _PRE_COMP_)
			memcpy(szTemplate, "06", 2);
		else
			memcpy(szTemplate, "00", 2);
	}
	else
	{
		memcpy(szTemplate, "98", 2);
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
                inLogPrintf(AT, "inNCCC_DCC_Pack25_Software() END!");

        return (inCnt);
}

/*
Function        :inNCCC_DCC_Pack35
Date&Time       :2016/10/4 上午 9:23
Describe        :Field_35:   TRACK 2 Data
*/
int inNCCC_DCC_Pack35(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int     inCnt = 0, inPacketCnt = 0, i;
        char    szT2Data[50 + 1];		/* 目前track2最長到32 */
	char	szDebugMsg[100 + 1];
	short   shLen = 0;
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_DCC_Pack35() START!");

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

        
	/* Manual keyin -> PAN + 'D' + Expire Date */
	memset(szT2Data, 0x00, sizeof(szT2Data));
	if (pobTran->srBRec.uszManualBit == VS_TRUE)
	{
		strcpy(szT2Data, pobTran->srBRec.szPAN);
		inPacketCnt += strlen(pobTran->srBRec.szPAN);

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
			case _VOID_ :
			case _PRE_COMP_ :
			case _BATCH_UPLOAD_ :
			case _REVERSAL_:
			case _SEND_ESC_ADVICE_:
				strcpy(szT2Data, pobTran->srBRec.szPAN);
				inPacketCnt += strlen(pobTran->srBRec.szPAN);

				strcat(szT2Data, "D");
				inPacketCnt ++;
				memcpy(&szT2Data[inPacketCnt], &pobTran->srBRec.szExpDate[0], 4);
				inPacketCnt += 4;
				memcpy(&szT2Data[inPacketCnt], &pobTran->srBRec.szServiceCode[0], 3);
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
                inLogPrintf(AT, "inNCCC_DCC_Pack35() END!");

        return (inCnt);
}

/*
Function        :inNCCC_DCC_Pack35_tSAM
Date&Time       :2016/10/4 上午 9:23
Describe        :Field_35:   TRACK 2 Data
*/
int inNCCC_DCC_Pack35_tSAM(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        /* Field_35:	TRACK 2 Data */
	int		inCnt = 0, inPacketCnt = 0, i, inRetVal = -1;
	short		shLen;
	char		szT2Data[50 + 1];				/* 目前track2最長到32 */
	char		szTemplate[100 + 1];
	char		szDebugMsg[100 + 1];
	char		szEncrypt[50 + 1];
	char		szField37[8 + 1], szField41[4 + 1];
	char		szAscii[84 + 1];
	unsigned char	uszSlot = 0;
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_DCC_Pack35_tSAM() START!");
	
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
	if (pobTran->srBRec.uszManualBit == VS_TRUE)
	{
		strcpy(szT2Data, pobTran->srBRec.szPAN);
		inPacketCnt += strlen(pobTran->srBRec.szPAN);

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
			case _TIP_ :
			case _TC_UPLOAD_ :
			case _VOID_ :
			case _PRE_COMP_ :
			case _BATCH_UPLOAD_ :
			case _REVERSAL_ :
			case _SEND_ESC_ADVICE_:
				strcpy(szT2Data, pobTran->srBRec.szPAN);
				inPacketCnt += strlen(pobTran->srBRec.szPAN);


				strcat(szT2Data, "D");
				inPacketCnt ++;
				memcpy(&szT2Data[inPacketCnt], &pobTran->srBRec.szExpDate[0], 4);
				inPacketCnt += 4;
				memcpy(&szT2Data[inPacketCnt], &pobTran->srBRec.szServiceCode[0], 3);
				inPacketCnt += 3;

				break;
			default :
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					inLogPrintf(AT, "F_35 [Track2: ]");
					sprintf(szDebugMsg, "%s", pobTran->szTrack2);
					inLogPrintf(AT, szDebugMsg);

				}
				memcpy(szT2Data, pobTran->szTrack2, strlen(pobTran->szTrack2));

				for (i = 0; i < strlen(szT2Data); i ++)
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

	if (shLen % 2)
	{
		shLen ++;
		strcat(szT2Data, "0");
	}

	if (ginDebug == VS_TRUE)
		inLogPrintf(AT, "inNCCC_DCC_Pack35_tSAM() 開始準備加密");

	/* 開始加密 */
	/* 組 Field_37 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	if (pobTran->inISOTxnCode == _DCC_RATE_)
	{
		
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
	memset(gusztSAMCheckSum_DCC35, 0x00, sizeof(gusztSAMCheckSum_DCC35));
	memset(szEncrypt, 0x00, sizeof(szEncrypt));

	/* 空的部份全補F */
	if (shLen < 16)
	{
		for (i = shLen; i < 16; i ++)
		{
			szT2Data[i] = 0x46;
		}
		shLen = 16;

		inFunc_ASCII_to_BCD((unsigned char*)szEncrypt, szT2Data, shLen / 2);
	}
	else
		inFunc_ASCII_to_BCD((unsigned char*)szEncrypt, szT2Data, shLen / 2);

	if (ginDebug == VS_TRUE)
	{
		memset(szAscii, 0x00, sizeof(szAscii));
		inFunc_BCD_to_ASCII(szAscii, (unsigned char*)szEncrypt, shLen / 2);
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		inLogPrintf(AT, "F_35 [szEncrypt ]");
		sprintf(szDebugMsg, "%s L:%d", szAscii, shLen);
		inLogPrintf(AT, szDebugMsg);
	}

	memcpy((char *)&uszPackBuf[inCnt], szEncrypt, (shLen / 2));

	if (ginDebug == VS_TRUE)
		inLogPrintf(AT, "inNCCC_DCC_Pack35_tSAM() 開始加密");

	inRetVal = inNCCC_tSAM_Encrypt(uszSlot, 
					shLen,
					szEncrypt,
					(unsigned char*)&szField37,
					(unsigned char*)&szField41,
					&gusztSAMKeyIndex_DCC,
					gusztSAMCheckSum_DCC35);

	if (inRetVal == VS_ERROR)
	{
		pobTran->inErrorMsg = _ERROR_CODE_V3_TSAM_ENCRYPT_FAIL_;
		return (VS_ERROR);
	}
	else
	{
		guszField_DCC35 = VS_TRUE;
		memcpy((char *)&uszPackBuf[inCnt], szEncrypt, inRetVal);
		inCnt += (shLen / 2);
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
                inLogPrintf(AT, "inNCCC_DCC_Pack35_tSAM() END!");
	
	return (inCnt);
}

/*
Function        :inNCCC_DCC_Pack35_Software
Date&Time       :2016/10/4 上午 9:23
Describe        :Field_35:   TRACK 2 Data
*/
int inNCCC_DCC_Pack35_Software(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        /* Field_35:	TRACK 2 Data */
	int		inSoftwareKey = 0;
	int		inCnt = 0, inPacketCnt = 0, i;
	short		shLen;
	char		szT2Data[50 + 1];				/* 目前track2最長到32 */
	char		szDebugMsg[100 + 1];
	char		szEncrypt[50 + 1];
	char		szAscii[84 + 1];
	char		szField11[3 + 1];
	char		szSTAN[6 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_DCC_Pack35_Software() START!");
	
	
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
	if (pobTran->srBRec.uszManualBit == VS_TRUE)
	{
		strcpy(szT2Data, pobTran->srBRec.szPAN);
		inPacketCnt += strlen(pobTran->srBRec.szPAN);

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
			case _TIP_ :
			case _TC_UPLOAD_ :
			case _VOID_ :
			case _PRE_COMP_ :
			case _BATCH_UPLOAD_ :
			case _REVERSAL_ :
			case _SEND_ESC_ADVICE_:
				strcpy(szT2Data, pobTran->srBRec.szPAN);
				inPacketCnt += strlen(pobTran->srBRec.szPAN);


				strcat(szT2Data, "D");
				inPacketCnt ++;
				memcpy(&szT2Data[inPacketCnt], &pobTran->srBRec.szExpDate[0], 4);
				inPacketCnt += 4;
				memcpy(&szT2Data[inPacketCnt], &pobTran->srBRec.szServiceCode[0], 3);
				inPacketCnt += 3;

				break;
			default :
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					inLogPrintf(AT, "F_35 [Track2: ]");
					sprintf(szDebugMsg, "%s", pobTran->szTrack2);
					inLogPrintf(AT, szDebugMsg);

				}
				memcpy(szT2Data, pobTran->szTrack2, strlen(pobTran->szTrack2));

				for (i = 0; i < strlen(szT2Data); i ++)
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

	if (shLen % 2)
	{
		shLen ++;
		strcat(szT2Data, "0");
	}

	if (ginDebug == VS_TRUE)
		inLogPrintf(AT, "inNCCC_DCC_Pack35_Software() 開始準備加密");

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
		inLogPrintf(AT, "inNCCC_DCC_Pack35_Software() 開始加密");
	
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
                inLogPrintf(AT, "inNCCC_DCC_Pack35_Software() END!");
	
	return (inCnt);
}

/*
Function        :inNCCC_DCC_Pack37
Date&Time       :2016/9/30 下午 6:42
Describe        :Field_37:	Retrieval Reference Number (RRN)
*/
int inNCCC_DCC_Pack37(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 	inCnt = 0;
	char	szDebugMsg[100 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_DCC_Pack37() START!");
	
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
                inLogPrintf(AT, "inNCCC_DCC_Pack37() END!");
	
        return (inCnt);
}

/*
Function        :inNCCC_DCC_Pack38
Date&Time       :2016/9/19 下午 1:09
Describe        :Field_38:	Authorization Identification Response（SmartPay交易無授權碼）
*/
int inNCCC_DCC_Pack38(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 	inCnt = 0;
        char    szAuthCode[_AUTH_CODE_SIZE_ + 1];
	char	szDebugMsg[100 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_DCC_Pack38() START!");


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
		if ((!memcmp(pobTran->srBRec.szAuthCode, "000000", 6) || !memcmp(pobTran->srBRec.szAuthCode, "      ", 6)))
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
                inLogPrintf(AT, "inNCCC_DCC_Pack38() END!");
	
        return (inCnt);
}

/*
Function        :inNCCC_DCC_Pack41
Date&Time       :2016/9/30 下午 6:44
Describe        :TID
*/
int inNCCC_DCC_Pack41(TRANSACTION_OBJECT *pobTran , unsigned char *uszPackBuf)
{
        int	inCnt = 0;
	char	szTemplate[8 + 1];

	memset(szTemplate, 0x00, sizeof(szTemplate));

	if (inGetTerminalID(szTemplate) == VS_ERROR)
		return (VS_ERROR);

	memcpy((char *)&uszPackBuf[inCnt], szTemplate, strlen(szTemplate));
	inCnt += strlen(szTemplate);

	return (inCnt);
}

/*
Function        :inNCCC_DCC_Pack42
Date&Time       :2016/9/30 下午 6:44
Describe        :MID
*/
int inNCCC_DCC_Pack42(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int	inCnt = 0;
	char	szTemplate[15 + 1];

	memset(szTemplate, 0x00, sizeof(szTemplate));

        if (inGetMerchantID(szTemplate) == VS_ERROR)
		return (VS_ERROR);

	inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 15, _PADDING_RIGHT_);
	memcpy((char *)&uszPackBuf[inCnt], szTemplate, strlen(szTemplate));
	inCnt += strlen(szTemplate);

	return (inCnt);
}

/*
Function        :inNCCC_DCC_Pack48
Date&Time       :
Describe        :Field_48:	Additional Data - Private Use
*/
int inNCCC_DCC_Pack48(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int	i;
        int	inCnt = 0;
	char	szDebugMsg[100 + 1]= {0};
	char	szTemplate[30 + 1]= {0};
	char	szAscii[100 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
	char	szPOS_ID[6 + 1] = {0};
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_DCC_Pack48() START!");

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
                inLogPrintf(AT, "inNCCC_DCC_Pack48() END!");

	return (inCnt);
}

/*
Function        :inNCCC_DCC_Pack54
Date&Time       :2016/9/30 下午 6:49
Describe        :Field_54:	Additional Amounts(此規格放Tip)
*/
int inNCCC_DCC_Pack54(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 	inCnt = 0;
	char	szAscii[16 + 1];
	char	szDebugMsg[100 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_DCC_Pack54() START!");

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
                inLogPrintf(AT, "inNCCC_DCC_Pack54() END!");
	
        return (inCnt);
}

/*
Function        :inNCCC_DCC_Pack55
Date&Time       :
Describe        :Field_55:	EMV Chip Data
*/
int inNCCC_DCC_Pack55(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int		inBitMapIndex = 0;
	int		inCnt = 0, inPacketCnt = 0;
	int		inContactlessType = 0;
	char		szASCII[4 + 1];
	char		szPacket[512 + 1];
        unsigned char	uszBCD[2 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_DCC_Pack55() START!");

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
	/* 【需求單 - 106252】	DCC交易新增感應及BIN11碼需求 by Russell 2020/2/20 上午 11:56 */
	/* 這邊直接放和ATS一樣的邏輯 by Russell 2020/2/21 下午 5:04 */
	/* 因為把F55的檢查寫的較嚴格，所以沒考慮到Reversal的狀況，所以若Reversal跑到F55時，先換回原MTI (START) */
	if (pobTran->inISOTxnCode == _REVERSAL_)
	{
		for (inBitMapIndex = 0;; inBitMapIndex++)
		{
			if (srNCCC_DCC_ISOBitMap[inBitMapIndex].inTxnID == _NCCC_DCC_NULL_TX_)
			{
				return (VS_ERROR); /* 找不到相對應交易別的Bit Map */
			}

			if (srNCCC_DCC_ISOBitMap[inBitMapIndex].inTxnID == pobTran->inTransactionCode)
				break; /* 找到一樣的交易類別 */
		}
		
		memcpy(guszDCC_MTI, srNCCC_DCC_ISOBitMap[inBitMapIndex].szMTI, 4);
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
		
		if (pobTran->srEMVRec.in5F2A_TransCurrCodeLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x5F;
			szPacket[inPacketCnt ++] = 0x2A;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in5F2A_TransCurrCodeLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz5F2A_TransCurrCode[0], pobTran->srEMVRec.in5F2A_TransCurrCodeLen);
			inPacketCnt += pobTran->srEMVRec.in5F2A_TransCurrCodeLen;
                }
	
		if (pobTran->srEMVRec.in5F34_ApplPanSeqnumLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x5F;
			szPacket[inPacketCnt ++] = 0x34;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in5F34_ApplPanSeqnumLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz5F34_ApplPanSeqnum[0], pobTran->srEMVRec.in5F34_ApplPanSeqnumLen);
			inPacketCnt += pobTran->srEMVRec.in5F34_ApplPanSeqnumLen;
		}
	
		if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY		 ||
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY || 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1) &&
		    !memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
		{
			/* 不送 */
		}
		else
		{
			if (pobTran->srEMVRec.in82_AIPLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x82;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in82_AIPLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz82_AIP[0], pobTran->srEMVRec.in82_AIPLen);
				inPacketCnt += pobTran->srEMVRec.in82_AIPLen;
			}
		}
	
		/* Add By Tim 2015-04-16 PM 03:19:09 更新規格收單交易系統端末機電文規格V4_4_20150727.pdf */
		if (pobTran->srEMVRec.in84_DFNameLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x84;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in84_DFNameLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz84_DF_NAME[0], pobTran->srEMVRec.in84_DFNameLen);
			inPacketCnt += pobTran->srEMVRec.in84_DFNameLen;
		}
	
		/* Add By Tim 2015-04-16 PM 03:19:09 更新規格收單交易系統端末機電文規格V4_4_20150727.pdf */
		if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1 && (!memcmp(&guszDCC_MTI[0], "0200", 4) || (!memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)))		|| 
		    (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY && (!memcmp(&guszDCC_MTI[0], "0200", 4) || (!memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)))		|| 
		    (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY && (!memcmp(&guszDCC_MTI[0], "0200", 4) || (!memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)))	||
		    (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE && (pobTran->inISOTxnCode == _SALE_ || pobTran->inISOTxnCode ==_REVERSAL_))							||
		    (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_21_PAYPASS_MCHIP && (pobTran->inISOTxnCode == _SALE_ || pobTran->inISOTxnCode ==_REVERSAL_))								||
		    (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_17_WAVE3 && !memcmp(&guszDCC_MTI[0], "0200", 4))													|| 
		    (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_63_NEWJSPEEDY_EMV && !memcmp(&guszDCC_MTI[0], "0200", 4))												|| 
		    (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_91_QUICKPASS && !memcmp(&guszDCC_MTI[0], "0200", 4))													||
		    (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_50_EXPRESSSPAY && !memcmp(&guszDCC_MTI[0], "0200", 4))												||
		    (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE && !memcmp(&guszDCC_MTI[0], "0200", 4))											||
		    (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_42_DPAS && !memcmp(&guszDCC_MTI[0], "0200", 4))													||
		    (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_43_DISCOVER_DPAS_MAG_STRIPE && !memcmp(&guszDCC_MTI[0], "0200", 4)))
		{
			/* 不送 */
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

		if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY		||
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY	|| 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1) &&
		    !memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
		{
			/* 不送 */
		}
		else
		{
			if (pobTran->srEMVRec.in95_TVRLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x95;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in95_TVRLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz95_TVR[0], pobTran->srEMVRec.in95_TVRLen);
				inPacketCnt += pobTran->srEMVRec.in95_TVRLen;
			}
		}

		if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY		||
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY	|| 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1) &&
		    !memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
		{
			/* 不送 */
		}
		else
		{
			if (pobTran->srEMVRec.in9A_TranDateLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9A;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9A_TranDateLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9A_TranDate[0], pobTran->srEMVRec.in9A_TranDateLen);
				inPacketCnt += pobTran->srEMVRec.in9A_TranDateLen;
			}
		}

		/* Add By Tim 2015-04-16 PM 03:19:09 更新規格收單交易系統端末機電文規格V4_4_20150727.pdf */
		if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY		||
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY	|| 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1) &&
		    !memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
		{
			/* 不送 */
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

		if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY		||
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY	|| 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1) &&
		    !memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
		{
			/* 不送 */
		}
		else
		{
			if (pobTran->srEMVRec.in9C_TranTypeLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9C;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9C_TranTypeLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9C_TranType[0], pobTran->srEMVRec.in9C_TranTypeLen);
				inPacketCnt += pobTran->srEMVRec.in9C_TranTypeLen;
			}
		}

		if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY		||
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY	|| 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1) &&
		    !memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
		{
			/* 不送 */
		}
		else
		{
			if (pobTran->srEMVRec.in9F02_AmtAuthNumLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x02;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F02_AmtAuthNumLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F02_AmtAuthNum[0], pobTran->srEMVRec.in9F02_AmtAuthNumLen);
				inPacketCnt += pobTran->srEMVRec.in9F02_AmtAuthNumLen;
			}
		}

		if (pobTran->srEMVRec.in9F03_AmtOtherNumLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x03;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F03_AmtOtherNumLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F03_AmtOtherNum[0], pobTran->srEMVRec.in9F03_AmtOtherNumLen);
			inPacketCnt += pobTran->srEMVRec.in9F03_AmtOtherNumLen;
		}

		if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY		||
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY	|| 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1) &&
		    !memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
		{
			/* 不送 */
		}
		else
		{
			if (pobTran->srEMVRec.in9F10_IssuerAppDataLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x10;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F10_IssuerAppDataLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F10_IssuerAppData[0], pobTran->srEMVRec.in9F10_IssuerAppDataLen);
				inPacketCnt += pobTran->srEMVRec.in9F10_IssuerAppDataLen;
			}
		}

		if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY		||
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY	|| 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1) &&
		    !memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
		{
			/* 不送 */
		}
		else
		{
			if (pobTran->srEMVRec.in9F1A_TermCountryCodeLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x1A;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F1A_TermCountryCodeLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F1A_TermCountryCode[0], pobTran->srEMVRec.in9F1A_TermCountryCodeLen);
				inPacketCnt += pobTran->srEMVRec.in9F1A_TermCountryCodeLen;
			}
		}

		/* Add By Tim 2015-04-16 PM 03:19:09 更新規格收單交易系統端末機電文規格V4_4_20150727.pdf */
		if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY		 || 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY || 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1) &&
		    !memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
		{
			/* 不送 */
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

		if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY		||
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY	|| 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1) &&
		    !memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
		{
			/* 不送 */
		}
		else
		{
			if (pobTran->srEMVRec.in9F26_ApplCryptogramLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x26;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F26_ApplCryptogramLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F26_ApplCryptogram[0], pobTran->srEMVRec.in9F26_ApplCryptogramLen);
				inPacketCnt += pobTran->srEMVRec.in9F26_ApplCryptogramLen;
			}
		}

		if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY		||
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY	|| 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1) &&
		    !memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
		{
			/* 不送 */
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

		if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY		||
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY	|| 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1) &&
		    !memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
		{
			/* 不送 */
		}
		else
		{
			if (pobTran->srEMVRec.in9F33_TermCapabilitiesLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x33;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F33_TermCapabilitiesLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F33_TermCapabilities[0], pobTran->srEMVRec.in9F33_TermCapabilitiesLen);
				inPacketCnt += pobTran->srEMVRec.in9F33_TermCapabilitiesLen;
			}
		}

		/* Add By Tim 2015-04-16 PM 03:19:09 更新規格收單交易系統端末機電文規格V4_4_20150727.pdf */
		if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY			|| 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY	|| 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1) &&
		    !memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
		{
			/* 不送 */
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

		/* Add By Tim 2015-04-16 PM 03:19:09 更新規格收單交易系統端末機電文規格V4_4_20150727.pdf */
		if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY		 || 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY || 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1) &&
		    !memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
		{
			/* 不送 */
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

		if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY		||
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY	|| 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1) &&
		    !memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
		{
			/* 不送 */
		}
		else
		{
			if (pobTran->srEMVRec.in9F36_ATCLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x36;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F36_ATCLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F36_ATC[0], pobTran->srEMVRec.in9F36_ATCLen);
				inPacketCnt += pobTran->srEMVRec.in9F36_ATCLen;
			}
		}

		if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY		||
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY	|| 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1) &&
		    !memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
		{
			/* 不送 */
		}
		else
		{
			if (pobTran->srEMVRec.in9F37_UnpredictNumLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x37;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F37_UnpredictNumLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F37_UnpredictNum[0], pobTran->srEMVRec.in9F37_UnpredictNumLen);
				inPacketCnt += pobTran->srEMVRec.in9F37_UnpredictNumLen;
			}
		}

		/* Add By Tim 2015-04-16 PM 03:19:09 更新規格收單交易系統端末機電文規格V4_4_20150727.pdf */
		if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY		|| 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY	|| 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1) &&
		    !memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
		{
			/* 不送 */
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

		if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY		||
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY	|| 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1))
		{
			/* 不送 */
		}
		else
		{
			/* Add By Tim 2015-09-10 PM 04:55:00 【需求單103399】VCPS2.1.3規範，請協助評估端末機 DE55新增上傳 FFI (Tag 9F6E) */
			if (pobTran->srEMVRec.in9F6E_From_Factor_IndicatorLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x6E;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F6E_From_Factor_IndicatorLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F6E_From_Factor_Indicator[0], pobTran->srEMVRec.in9F6E_From_Factor_IndicatorLen);
				inPacketCnt += pobTran->srEMVRec.in9F6E_From_Factor_IndicatorLen;
			}
		}
	}
	/* 晶片卡 */
	else
	{
		/* 組 0200 要用完整的 Track2 */
		if (!memcmp(guszDCC_MTI, "0200", 4) && pobTran->inISOTxnCode == _SALE_)
		{
			if (pobTran->in57_Track2Len > 0)
			{
				szPacket[inPacketCnt ++] = 0x57;
				szPacket[inPacketCnt ++] = pobTran->in57_Track2Len;
				memcpy(&szPacket[inPacketCnt], pobTran->usz57_Track2, pobTran->in57_Track2Len);
				inPacketCnt += pobTran->in57_Track2Len;
			}
		}
		/* 其他用【卡號】【有效期】 */
		else
		{
			if (pobTran->srEMVRec.in5A_ApplPanLen <= 0 || pobTran->srEMVRec.in5F24_ExpireDateLen <= 0)
				return (VS_ERROR);

			szPacket[inPacketCnt ++] = 0x5A;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in5A_ApplPanLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz5A_ApplPan[0], pobTran->srEMVRec.in5A_ApplPanLen);
			inPacketCnt += pobTran->srEMVRec.in5A_ApplPanLen;

			szPacket[inPacketCnt ++] = 0x5F;
			szPacket[inPacketCnt ++] = 0x24;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in5F24_ExpireDateLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz5F24_ExpireDate[0], pobTran->srEMVRec.in5F24_ExpireDateLen);
			inPacketCnt += pobTran->srEMVRec.in5F24_ExpireDateLen;
		}

		if (pobTran->srEMVRec.in5F2A_TransCurrCodeLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x5F;
			szPacket[inPacketCnt ++] = 0x2A;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in5F2A_TransCurrCodeLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz5F2A_TransCurrCode[0], pobTran->srEMVRec.in5F2A_TransCurrCodeLen);
			inPacketCnt += pobTran->srEMVRec.in5F2A_TransCurrCodeLen;
		}

		if (pobTran->srEMVRec.in5F34_ApplPanSeqnumLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x5F;
			szPacket[inPacketCnt ++] = 0x34;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in5F34_ApplPanSeqnumLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz5F34_ApplPanSeqnum[0], pobTran->srEMVRec.in5F34_ApplPanSeqnumLen);
			inPacketCnt += pobTran->srEMVRec.in5F34_ApplPanSeqnumLen;
		}

		if (pobTran->srEMVRec.in82_AIPLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x82;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in82_AIPLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz82_AIP[0], pobTran->srEMVRec.in82_AIPLen);
			inPacketCnt += pobTran->srEMVRec.in82_AIPLen;
		}

		if (pobTran->srEMVRec.in84_DFNameLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x84;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in84_DFNameLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz84_DF_NAME[0], pobTran->srEMVRec.in84_DFNameLen);
			inPacketCnt += pobTran->srEMVRec.in84_DFNameLen;
		}

		/* MIT = 〈0220〉 〈0320〉用送 */
		if (!memcmp(&guszDCC_MTI[0], "0220", 4) || !memcmp(&guszDCC_MTI[0], "0320", 4))
		{
			if (pobTran->srEMVRec.in8A_AuthRespCodeLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x8A;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in8A_AuthRespCodeLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz8A_AuthRespCode[0], pobTran->srEMVRec.in8A_AuthRespCodeLen);
				inPacketCnt += pobTran->srEMVRec.in8A_AuthRespCodeLen;
			}
		}

		if (pobTran->srEMVRec.in95_TVRLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x95;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in95_TVRLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz95_TVR[0], pobTran->srEMVRec.in95_TVRLen);
			inPacketCnt += pobTran->srEMVRec.in95_TVRLen;
		}

		if (pobTran->srEMVRec.in9A_TranDateLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9A;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9A_TranDateLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9A_TranDate[0], pobTran->srEMVRec.in9A_TranDateLen);
			inPacketCnt += pobTran->srEMVRec.in9A_TranDateLen;
		}

		if (pobTran->srEMVRec.in9B_TSILen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9B;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9B_TSILen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9B_TSI[0], pobTran->srEMVRec.in9B_TSILen);
			inPacketCnt += pobTran->srEMVRec.in9B_TSILen;
		}

		if (pobTran->srEMVRec.in9C_TranTypeLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9C;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9C_TranTypeLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9C_TranType[0], pobTran->srEMVRec.in9C_TranTypeLen);
			inPacketCnt += pobTran->srEMVRec.in9C_TranTypeLen;
		}

		if (pobTran->srEMVRec.in9F02_AmtAuthNumLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x02;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F02_AmtAuthNumLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F02_AmtAuthNum[0], pobTran->srEMVRec.in9F02_AmtAuthNumLen);
			inPacketCnt += pobTran->srEMVRec.in9F02_AmtAuthNumLen;
		}

		if (pobTran->srEMVRec.in9F03_AmtOtherNumLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x03;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F03_AmtOtherNumLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F03_AmtOtherNum[0], pobTran->srEMVRec.in9F03_AmtOtherNumLen);
			inPacketCnt += pobTran->srEMVRec.in9F03_AmtOtherNumLen;
		}

		if (pobTran->srEMVRec.in9F09_TermVerNumLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x09;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F09_TermVerNumLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F09_TermVerNum[0], pobTran->srEMVRec.in9F09_TermVerNumLen);
			inPacketCnt += pobTran->srEMVRec.in9F09_TermVerNumLen;
		}

		if (pobTran->srEMVRec.in9F10_IssuerAppDataLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x10;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F10_IssuerAppDataLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F10_IssuerAppData[0], pobTran->srEMVRec.in9F10_IssuerAppDataLen);
			inPacketCnt += pobTran->srEMVRec.in9F10_IssuerAppDataLen;
		}

		if (pobTran->srEMVRec.in9F1A_TermCountryCodeLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x1A;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F1A_TermCountryCodeLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F1A_TermCountryCode[0], pobTran->srEMVRec.in9F1A_TermCountryCodeLen);
			inPacketCnt += pobTran->srEMVRec.in9F1A_TermCountryCodeLen;
		}

		if (pobTran->srEMVRec.in9F1E_IFDNumLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x1E;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F1E_IFDNumLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F1E_IFDNum[0], pobTran->srEMVRec.in9F1E_IFDNumLen);
			inPacketCnt += pobTran->srEMVRec.in9F1E_IFDNumLen;
		}

		if (pobTran->srEMVRec.in9F26_ApplCryptogramLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x26;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F26_ApplCryptogramLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F26_ApplCryptogram[0], pobTran->srEMVRec.in9F26_ApplCryptogramLen);
			inPacketCnt += pobTran->srEMVRec.in9F26_ApplCryptogramLen;
		}

		if (pobTran->srEMVRec.in9F27_CIDLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x27;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F27_CIDLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F27_CID[0], pobTran->srEMVRec.in9F27_CIDLen);
			inPacketCnt += pobTran->srEMVRec.in9F27_CIDLen;
		}

		if (pobTran->srEMVRec.in9F33_TermCapabilitiesLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x33;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F33_TermCapabilitiesLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F33_TermCapabilities[0], pobTran->srEMVRec.in9F33_TermCapabilitiesLen);
			inPacketCnt += pobTran->srEMVRec.in9F33_TermCapabilitiesLen;
		}

		if (pobTran->srEMVRec.in9F34_CVMLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x34;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F34_CVMLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F34_CVM[0], pobTran->srEMVRec.in9F34_CVMLen);
			inPacketCnt += pobTran->srEMVRec.in9F34_CVMLen;
		}

		if (pobTran->srEMVRec.in9F35_TermTypeLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x35;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F35_TermTypeLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F35_TermType[0], pobTran->srEMVRec.in9F35_TermTypeLen);
			inPacketCnt += pobTran->srEMVRec.in9F35_TermTypeLen;
		}

		if (pobTran->srEMVRec.in9F36_ATCLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x36;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F36_ATCLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F36_ATC[0], pobTran->srEMVRec.in9F36_ATCLen);
			inPacketCnt += pobTran->srEMVRec.in9F36_ATCLen;
		}

		if (pobTran->srEMVRec.in9F37_UnpredictNumLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x37;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F37_UnpredictNumLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F37_UnpredictNum[0], pobTran->srEMVRec.in9F37_UnpredictNumLen);
			inPacketCnt += pobTran->srEMVRec.in9F37_UnpredictNumLen;
		}

		if (pobTran->srEMVRec.in9F41_TransSeqCounterLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x41;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F41_TransSeqCounterLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F41_TransSeqCounter[0], pobTran->srEMVRec.in9F41_TransSeqCounterLen);
			inPacketCnt += pobTran->srEMVRec.in9F41_TransSeqCounterLen;
		}

		if (pobTran->srEMVRec.in9F5B_ISRLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x5B;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F5B_ISRLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F5B_ISR[0], pobTran->srEMVRec.in9F5B_ISRLen);
			inPacketCnt += pobTran->srEMVRec.in9F5B_ISRLen;
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
                inLogPrintf(AT, "inNCCC_DCC_Pack55() END!");
	
	return (inCnt);
}

/*
Function        :inNCCC_DCC_Pack55_tSAM
Date&Time       :2016/10/3 上午 10:42
Describe        :Field_55:	EMV Chip Data
*/
int inNCCC_DCC_Pack55_tSAM(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int		i = 0;
	int		inRetVal = VS_SUCCESS;
	int		inBitMapIndex = 0;
	int		inCnt = 0, inPacketCnt = 0;
	int		inContactlessType = 0;
	char		szPacket[512 + 1] = {0};
	char		szTemplate[50 + 1] = {0};
	char		szField37[8 + 1] = {0}, szField41[4 + 1] = {0};
	char		szASCII[84 + 1] = {0};
	char		szDebugMsg[100 + 1] = {0};
        unsigned char	uszBCD[2 + 1] = {0};
	unsigned char	szEncrypt[200 + 1] = {0};
	unsigned char	uszSlot = 0;
	unsigned short	usLen = 0;

	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_DCC_Pack55_tSAM() START!");
	
	
	/* 【需求單 - 106252】	DCC交易新增感應及BIN11碼需求 by Russell 2020/2/20 上午 11:56 */
	/* 這邊直接放和ATS一樣的邏輯 by Russell 2020/2/21 下午 5:04 */
	/* 因為把F55的檢查寫的較嚴格，所以沒考慮到Reversal的狀況，所以若Reversal跑到F55時，先換回原MTI (START) */
	if (pobTran->inISOTxnCode == _REVERSAL_)
	{
		for (inBitMapIndex = 0;; inBitMapIndex++)
		{
			if (srNCCC_DCC_ISOBitMap[inBitMapIndex].inTxnID == _NCCC_DCC_NULL_TX_)
			{
				return (VS_ERROR); /* 找不到相對應交易別的Bit Map */
			}

			if (srNCCC_DCC_ISOBitMap[inBitMapIndex].inTxnID == pobTran->inTransactionCode)
				break; /* 找到一樣的交易類別 */
		}
		
		memcpy(guszDCC_MTI, srNCCC_DCC_ISOBitMap[inBitMapIndex].szMTI, 4);
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
		
		if (pobTran->srEMVRec.in5F2A_TransCurrCodeLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x5F;
			szPacket[inPacketCnt ++] = 0x2A;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in5F2A_TransCurrCodeLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz5F2A_TransCurrCode[0], pobTran->srEMVRec.in5F2A_TransCurrCodeLen);
			inPacketCnt += pobTran->srEMVRec.in5F2A_TransCurrCodeLen;
                }
	
		if (pobTran->srEMVRec.in5F34_ApplPanSeqnumLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x5F;
			szPacket[inPacketCnt ++] = 0x34;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in5F34_ApplPanSeqnumLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz5F34_ApplPanSeqnum[0], pobTran->srEMVRec.in5F34_ApplPanSeqnumLen);
			inPacketCnt += pobTran->srEMVRec.in5F34_ApplPanSeqnumLen;
		}
	
		if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY		 ||
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY || 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1) &&
		    !memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
		{
			/* 不送 */
		}
		else
		{
			if (pobTran->srEMVRec.in82_AIPLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x82;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in82_AIPLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz82_AIP[0], pobTran->srEMVRec.in82_AIPLen);
				inPacketCnt += pobTran->srEMVRec.in82_AIPLen;
			}
		}
	
		/* Add By Tim 2015-04-16 PM 03:19:09 更新規格收單交易系統端末機電文規格V4_4_20150727.pdf */
		if (pobTran->srEMVRec.in84_DFNameLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x84;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in84_DFNameLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz84_DF_NAME[0], pobTran->srEMVRec.in84_DFNameLen);
			inPacketCnt += pobTran->srEMVRec.in84_DFNameLen;
		}
	
		/* Add By Tim 2015-04-16 PM 03:19:09 更新規格收單交易系統端末機電文規格V4_4_20150727.pdf */
		if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1 && (!memcmp(&guszDCC_MTI[0], "0200", 4) || (!memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)))		|| 
		    (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY && (!memcmp(&guszDCC_MTI[0], "0200", 4) || (!memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)))		|| 
		    (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY && (!memcmp(&guszDCC_MTI[0], "0200", 4) || (!memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)))	||
		    (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE && (pobTran->inISOTxnCode == _SALE_ || pobTran->inISOTxnCode ==_REVERSAL_))							||
		    (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_21_PAYPASS_MCHIP && (pobTran->inISOTxnCode == _SALE_ || pobTran->inISOTxnCode ==_REVERSAL_))								||
		    (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_17_WAVE3 && !memcmp(&guszDCC_MTI[0], "0200", 4))													|| 
		    (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_63_NEWJSPEEDY_EMV && !memcmp(&guszDCC_MTI[0], "0200", 4))												|| 
		    (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_91_QUICKPASS && !memcmp(&guszDCC_MTI[0], "0200", 4))													||
		    (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_50_EXPRESSSPAY && !memcmp(&guszDCC_MTI[0], "0200", 4))												||
		    (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE && !memcmp(&guszDCC_MTI[0], "0200", 4))											||
		    (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_42_DPAS && !memcmp(&guszDCC_MTI[0], "0200", 4))													||
		    (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_43_DISCOVER_DPAS_MAG_STRIPE && !memcmp(&guszDCC_MTI[0], "0200", 4)))
		{
			/* 不送 */
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

		if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY		||
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY	|| 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1) &&
		    !memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
		{
			/* 不送 */
		}
		else
		{
			if (pobTran->srEMVRec.in95_TVRLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x95;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in95_TVRLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz95_TVR[0], pobTran->srEMVRec.in95_TVRLen);
				inPacketCnt += pobTran->srEMVRec.in95_TVRLen;
			}
		}

		if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY		||
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY	|| 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1) &&
		    !memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
		{
			/* 不送 */
		}
		else
		{
			if (pobTran->srEMVRec.in9A_TranDateLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9A;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9A_TranDateLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9A_TranDate[0], pobTran->srEMVRec.in9A_TranDateLen);
				inPacketCnt += pobTran->srEMVRec.in9A_TranDateLen;
			}
		}

		/* Add By Tim 2015-04-16 PM 03:19:09 更新規格收單交易系統端末機電文規格V4_4_20150727.pdf */
		if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY		||
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY	|| 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1) &&
		    !memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
		{
			/* 不送 */
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

		if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY		||
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY	|| 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1) &&
		    !memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
		{
			/* 不送 */
		}
		else
		{
			if (pobTran->srEMVRec.in9C_TranTypeLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9C;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9C_TranTypeLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9C_TranType[0], pobTran->srEMVRec.in9C_TranTypeLen);
				inPacketCnt += pobTran->srEMVRec.in9C_TranTypeLen;
			}
		}

		if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY		||
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY	|| 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1) &&
		    !memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
		{
			/* 不送 */
		}
		else
		{
			if (pobTran->srEMVRec.in9F02_AmtAuthNumLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x02;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F02_AmtAuthNumLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F02_AmtAuthNum[0], pobTran->srEMVRec.in9F02_AmtAuthNumLen);
				inPacketCnt += pobTran->srEMVRec.in9F02_AmtAuthNumLen;
			}
		}

		if (pobTran->srEMVRec.in9F03_AmtOtherNumLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x03;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F03_AmtOtherNumLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F03_AmtOtherNum[0], pobTran->srEMVRec.in9F03_AmtOtherNumLen);
			inPacketCnt += pobTran->srEMVRec.in9F03_AmtOtherNumLen;
		}

		if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY		||
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY	|| 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1) &&
		    !memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
		{
			/* 不送 */
		}
		else
		{
			if (pobTran->srEMVRec.in9F10_IssuerAppDataLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x10;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F10_IssuerAppDataLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F10_IssuerAppData[0], pobTran->srEMVRec.in9F10_IssuerAppDataLen);
				inPacketCnt += pobTran->srEMVRec.in9F10_IssuerAppDataLen;
			}
		}

		if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY		||
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY	|| 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1) &&
		    !memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
		{
			/* 不送 */
		}
		else
		{
			if (pobTran->srEMVRec.in9F1A_TermCountryCodeLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x1A;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F1A_TermCountryCodeLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F1A_TermCountryCode[0], pobTran->srEMVRec.in9F1A_TermCountryCodeLen);
				inPacketCnt += pobTran->srEMVRec.in9F1A_TermCountryCodeLen;
			}
		}

		/* Add By Tim 2015-04-16 PM 03:19:09 更新規格收單交易系統端末機電文規格V4_4_20150727.pdf */
		if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY		 || 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY || 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1) &&
		    !memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
		{
			/* 不送 */
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

		if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY		||
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY	|| 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1) &&
		    !memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
		{
			/* 不送 */
		}
		else
		{
			if (pobTran->srEMVRec.in9F26_ApplCryptogramLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x26;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F26_ApplCryptogramLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F26_ApplCryptogram[0], pobTran->srEMVRec.in9F26_ApplCryptogramLen);
				inPacketCnt += pobTran->srEMVRec.in9F26_ApplCryptogramLen;
			}
		}

		if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY		||
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY	|| 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1) &&
		    !memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
		{
			/* 不送 */
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

		if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY		||
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY	|| 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1) &&
		    !memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
		{
			/* 不送 */
		}
		else
		{
			if (pobTran->srEMVRec.in9F33_TermCapabilitiesLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x33;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F33_TermCapabilitiesLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F33_TermCapabilities[0], pobTran->srEMVRec.in9F33_TermCapabilitiesLen);
				inPacketCnt += pobTran->srEMVRec.in9F33_TermCapabilitiesLen;
			}
		}

		/* Add By Tim 2015-04-16 PM 03:19:09 更新規格收單交易系統端末機電文規格V4_4_20150727.pdf */
		if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY			|| 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY	|| 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1) &&
		    !memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
		{
			/* 不送 */
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

		/* Add By Tim 2015-04-16 PM 03:19:09 更新規格收單交易系統端末機電文規格V4_4_20150727.pdf */
		if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY		 || 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY || 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1) &&
		    !memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
		{
			/* 不送 */
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

		if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY		||
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY	|| 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1) &&
		    !memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
		{
			/* 不送 */
		}
		else
		{
			if (pobTran->srEMVRec.in9F36_ATCLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x36;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F36_ATCLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F36_ATC[0], pobTran->srEMVRec.in9F36_ATCLen);
				inPacketCnt += pobTran->srEMVRec.in9F36_ATCLen;
			}
		}

		if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY		||
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY	|| 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1) &&
		    !memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
		{
			/* 不送 */
		}
		else
		{
			if (pobTran->srEMVRec.in9F37_UnpredictNumLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x37;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F37_UnpredictNumLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F37_UnpredictNum[0], pobTran->srEMVRec.in9F37_UnpredictNumLen);
				inPacketCnt += pobTran->srEMVRec.in9F37_UnpredictNumLen;
			}
		}

		/* Add By Tim 2015-04-16 PM 03:19:09 更新規格收單交易系統端末機電文規格V4_4_20150727.pdf */
		if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY		|| 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY	|| 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1) &&
		    !memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
		{
			/* 不送 */
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

		if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY		||
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY	|| 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1))
		{
			/* 不送 */
		}
		else
		{
			/* Add By Tim 2015-09-10 PM 04:55:00 【需求單103399】VCPS2.1.3規範，請協助評估端末機 DE55新增上傳 FFI (Tag 9F6E) */
			if (pobTran->srEMVRec.in9F6E_From_Factor_IndicatorLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x6E;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F6E_From_Factor_IndicatorLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F6E_From_Factor_Indicator[0], pobTran->srEMVRec.in9F6E_From_Factor_IndicatorLen);
				inPacketCnt += pobTran->srEMVRec.in9F6E_From_Factor_IndicatorLen;
			}
		}
	}
	/* 晶片卡 */
	else
	{
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

		if (!memcmp(guszDCC_MTI, "0200", 4) && pobTran->inISOTxnCode == _SALE_)
		{
			if (pobTran->in57_Track2Len > 0)
			{
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
							       &gusztSAMKeyIndex_DCC,
							       gusztSAMCheckSum_DCC55);
				if (inRetVal == VS_ERROR)
				{
					pobTran->inErrorMsg = _ERROR_CODE_V3_TSAM_ENCRYPT_FAIL_;
					return (VS_ERROR);
				}
				else
				{
					guszField_DCC55 = VS_TRUE;
					memcpy(&szPacket[inPacketCnt], szEncrypt, inRetVal);
					inPacketCnt += usLen;
				}
			}
		}
		/* 其他用【卡號】【有效期】 */
		else
		{
			if (pobTran->srEMVRec.in5A_ApplPanLen <= 0 || pobTran->srEMVRec.in5F24_ExpireDateLen <= 0)
				return (VS_ERROR);

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
							&gusztSAMKeyIndex_DCC,
							gusztSAMCheckSum_DCC55);
			if (inRetVal == VS_ERROR)
			{
				pobTran->inErrorMsg = _ERROR_CODE_V3_TSAM_ENCRYPT_FAIL_;
				return (VS_ERROR);
			}
			else
			{
				guszField_DCC55 = VS_TRUE;
				memcpy(&szPacket[inPacketCnt], szEncrypt, inRetVal);
				inPacketCnt += usLen;
			}

			szPacket[inPacketCnt ++] = 0x5F;
			szPacket[inPacketCnt ++] = 0x24;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in5F24_ExpireDateLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz5F24_ExpireDate[0], pobTran->srEMVRec.in5F24_ExpireDateLen);
			inPacketCnt += pobTran->srEMVRec.in5F24_ExpireDateLen;
		}

		if (pobTran->srEMVRec.in5F2A_TransCurrCodeLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x5F;
			szPacket[inPacketCnt ++] = 0x2A;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in5F2A_TransCurrCodeLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz5F2A_TransCurrCode[0], pobTran->srEMVRec.in5F2A_TransCurrCodeLen);
			inPacketCnt += pobTran->srEMVRec.in5F2A_TransCurrCodeLen;
		}

		if (pobTran->srEMVRec.in5F34_ApplPanSeqnumLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x5F;
			szPacket[inPacketCnt ++] = 0x34;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in5F34_ApplPanSeqnumLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz5F34_ApplPanSeqnum[0], pobTran->srEMVRec.in5F34_ApplPanSeqnumLen);
			inPacketCnt += pobTran->srEMVRec.in5F34_ApplPanSeqnumLen;
		}

		if (pobTran->srEMVRec.in82_AIPLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x82;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in82_AIPLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz82_AIP[0], pobTran->srEMVRec.in82_AIPLen);
			inPacketCnt += pobTran->srEMVRec.in82_AIPLen;
		}

		if (pobTran->srEMVRec.in84_DFNameLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x84;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in84_DFNameLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz84_DF_NAME[0], pobTran->srEMVRec.in84_DFNameLen);
			inPacketCnt += pobTran->srEMVRec.in84_DFNameLen;
		}

		/* MIT = 〈0220〉 〈0320〉用送 */
		if (!memcmp(&guszDCC_MTI[0], "0220", 4) || !memcmp(&guszDCC_MTI[0], "0320", 4))
		{
			if (pobTran->srEMVRec.in8A_AuthRespCodeLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x8A;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in8A_AuthRespCodeLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz8A_AuthRespCode[0], pobTran->srEMVRec.in8A_AuthRespCodeLen);
				inPacketCnt += pobTran->srEMVRec.in8A_AuthRespCodeLen;
			}
		}

		if (pobTran->srEMVRec.in95_TVRLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x95;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in95_TVRLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz95_TVR[0], pobTran->srEMVRec.in95_TVRLen);
			inPacketCnt += pobTran->srEMVRec.in95_TVRLen;
		}

		if (pobTran->srEMVRec.in9A_TranDateLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9A;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9A_TranDateLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9A_TranDate[0], pobTran->srEMVRec.in9A_TranDateLen);
			inPacketCnt += pobTran->srEMVRec.in9A_TranDateLen;
		}

		if (pobTran->srEMVRec.in9B_TSILen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9B;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9B_TSILen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9B_TSI[0], pobTran->srEMVRec.in9B_TSILen);
			inPacketCnt += pobTran->srEMVRec.in9B_TSILen;
		}

		if (pobTran->srEMVRec.in9C_TranTypeLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9C;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9C_TranTypeLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9C_TranType[0], pobTran->srEMVRec.in9C_TranTypeLen);
			inPacketCnt += pobTran->srEMVRec.in9C_TranTypeLen;
		}

		if (pobTran->srEMVRec.in9F02_AmtAuthNumLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x02;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F02_AmtAuthNumLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F02_AmtAuthNum[0], pobTran->srEMVRec.in9F02_AmtAuthNumLen);
			inPacketCnt += pobTran->srEMVRec.in9F02_AmtAuthNumLen;
		}

		if (pobTran->srEMVRec.in9F03_AmtOtherNumLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x03;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F03_AmtOtherNumLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F03_AmtOtherNum[0], pobTran->srEMVRec.in9F03_AmtOtherNumLen);
			inPacketCnt += pobTran->srEMVRec.in9F03_AmtOtherNumLen;
		}

		if (pobTran->srEMVRec.in9F09_TermVerNumLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x09;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F09_TermVerNumLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F09_TermVerNum[0], pobTran->srEMVRec.in9F09_TermVerNumLen);
			inPacketCnt += pobTran->srEMVRec.in9F09_TermVerNumLen;
		}

		if (pobTran->srEMVRec.in9F10_IssuerAppDataLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x10;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F10_IssuerAppDataLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F10_IssuerAppData[0], pobTran->srEMVRec.in9F10_IssuerAppDataLen);
			inPacketCnt += pobTran->srEMVRec.in9F10_IssuerAppDataLen;
		}

		if (pobTran->srEMVRec.in9F1A_TermCountryCodeLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x1A;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F1A_TermCountryCodeLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F1A_TermCountryCode[0], pobTran->srEMVRec.in9F1A_TermCountryCodeLen);
			inPacketCnt += pobTran->srEMVRec.in9F1A_TermCountryCodeLen;
		}

		if (pobTran->srEMVRec.in9F1E_IFDNumLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x1E;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F1E_IFDNumLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F1E_IFDNum[0], pobTran->srEMVRec.in9F1E_IFDNumLen);
			inPacketCnt += pobTran->srEMVRec.in9F1E_IFDNumLen;
		}

		if (pobTran->srEMVRec.in9F26_ApplCryptogramLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x26;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F26_ApplCryptogramLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F26_ApplCryptogram[0], pobTran->srEMVRec.in9F26_ApplCryptogramLen);
			inPacketCnt += pobTran->srEMVRec.in9F26_ApplCryptogramLen;
		}

		if (pobTran->srEMVRec.in9F27_CIDLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x27;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F27_CIDLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F27_CID[0], pobTran->srEMVRec.in9F27_CIDLen);
			inPacketCnt += pobTran->srEMVRec.in9F27_CIDLen;
		}

		if (pobTran->srEMVRec.in9F33_TermCapabilitiesLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x33;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F33_TermCapabilitiesLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F33_TermCapabilities[0], pobTran->srEMVRec.in9F33_TermCapabilitiesLen);
			inPacketCnt += pobTran->srEMVRec.in9F33_TermCapabilitiesLen;
		}

		if (pobTran->srEMVRec.in9F34_CVMLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x34;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F34_CVMLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F34_CVM[0], pobTran->srEMVRec.in9F34_CVMLen);
			inPacketCnt += pobTran->srEMVRec.in9F34_CVMLen;
		}

		if (pobTran->srEMVRec.in9F35_TermTypeLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x35;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F35_TermTypeLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F35_TermType[0], pobTran->srEMVRec.in9F35_TermTypeLen);
			inPacketCnt += pobTran->srEMVRec.in9F35_TermTypeLen;
		}

		if (pobTran->srEMVRec.in9F36_ATCLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x36;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F36_ATCLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F36_ATC[0], pobTran->srEMVRec.in9F36_ATCLen);
			inPacketCnt += pobTran->srEMVRec.in9F36_ATCLen;
		}

		if (pobTran->srEMVRec.in9F37_UnpredictNumLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x37;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F37_UnpredictNumLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F37_UnpredictNum[0], pobTran->srEMVRec.in9F37_UnpredictNumLen);
			inPacketCnt += pobTran->srEMVRec.in9F37_UnpredictNumLen;
		}

		if (pobTran->srEMVRec.in9F41_TransSeqCounterLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x41;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F41_TransSeqCounterLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F41_TransSeqCounter[0], pobTran->srEMVRec.in9F41_TransSeqCounterLen);
			inPacketCnt += pobTran->srEMVRec.in9F41_TransSeqCounterLen;
		}

		if (pobTran->srEMVRec.in9F5B_ISRLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x5B;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F5B_ISRLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F5B_ISR[0], pobTran->srEMVRec.in9F5B_ISRLen);
			inPacketCnt += pobTran->srEMVRec.in9F5B_ISRLen;
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
                inLogPrintf(AT, "inNCCC_DCC_Pack55_tSAM() END!");
	
	return (inCnt);
}

/*
Function        :inNCCC_DCC_Pack55_Software
Date&Time       :2016/10/4 上午 9:54
Describe        :Field_55:	EMV Chip Data
*/
int inNCCC_DCC_Pack55_Software(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int		inSoftwareKey = 0;
	int		i = 0;
	int		inBitMapIndex = 0;
	int		inCnt = 0, inPacketCnt = 0;
	int		inContactlessType = 0;
	char		szPacket[512 + 1] = {0};
	char		szASCII[84 + 1] = {0};
	char		szSTAN[6 + 1] = {0}, szField11[3 + 1] = {0};
        unsigned char	uszBCD[2 + 1] = {0};
	unsigned char	szEncrypt[200 + 1] = {0};
	unsigned short	usLen = 0;
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_DCC_Pack55_Software() START!");
	
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
	
	/* 【需求單 - 106252】	DCC交易新增感應及BIN11碼需求 by Russell 2020/2/20 上午 11:56 */
	/* 這邊直接放和ATS一樣的邏輯 by Russell 2020/2/21 下午 5:04 */
	/* 因為把F55的檢查寫的較嚴格，所以沒考慮到Reversal的狀況，所以若Reversal跑到F55時，先換回原MTI (START) */
	if (pobTran->inISOTxnCode == _REVERSAL_)
	{
		for (inBitMapIndex = 0;; inBitMapIndex++)
		{
			if (srNCCC_DCC_ISOBitMap[inBitMapIndex].inTxnID == _NCCC_DCC_NULL_TX_)
			{
				return (VS_ERROR); /* 找不到相對應交易別的Bit Map */
			}

			if (srNCCC_DCC_ISOBitMap[inBitMapIndex].inTxnID == pobTran->inTransactionCode)
				break; /* 找到一樣的交易類別 */
		}
		
		memcpy(guszDCC_MTI, srNCCC_DCC_ISOBitMap[inBitMapIndex].szMTI, 4);
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
		
		if (pobTran->srEMVRec.in5F2A_TransCurrCodeLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x5F;
			szPacket[inPacketCnt ++] = 0x2A;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in5F2A_TransCurrCodeLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz5F2A_TransCurrCode[0], pobTran->srEMVRec.in5F2A_TransCurrCodeLen);
			inPacketCnt += pobTran->srEMVRec.in5F2A_TransCurrCodeLen;
                }
	
		if (pobTran->srEMVRec.in5F34_ApplPanSeqnumLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x5F;
			szPacket[inPacketCnt ++] = 0x34;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in5F34_ApplPanSeqnumLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz5F34_ApplPanSeqnum[0], pobTran->srEMVRec.in5F34_ApplPanSeqnumLen);
			inPacketCnt += pobTran->srEMVRec.in5F34_ApplPanSeqnumLen;
		}
	
		if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY		 ||
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY || 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1) &&
		    !memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
		{
			/* 不送 */
		}
		else
		{
			if (pobTran->srEMVRec.in82_AIPLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x82;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in82_AIPLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz82_AIP[0], pobTran->srEMVRec.in82_AIPLen);
				inPacketCnt += pobTran->srEMVRec.in82_AIPLen;
			}
		}
	
		/* Add By Tim 2015-04-16 PM 03:19:09 更新規格收單交易系統端末機電文規格V4_4_20150727.pdf */
		if (pobTran->srEMVRec.in84_DFNameLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x84;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in84_DFNameLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz84_DF_NAME[0], pobTran->srEMVRec.in84_DFNameLen);
			inPacketCnt += pobTran->srEMVRec.in84_DFNameLen;
		}
	
		/* Add By Tim 2015-04-16 PM 03:19:09 更新規格收單交易系統端末機電文規格V4_4_20150727.pdf */
		if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1 && (!memcmp(&guszDCC_MTI[0], "0200", 4) || (!memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)))		|| 
		    (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY && (!memcmp(&guszDCC_MTI[0], "0200", 4) || (!memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)))		|| 
		    (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY && (!memcmp(&guszDCC_MTI[0], "0200", 4) || (!memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)))	||
		    (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE && (pobTran->inISOTxnCode == _SALE_ || pobTran->inISOTxnCode ==_REVERSAL_))							||
		    (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_21_PAYPASS_MCHIP && (pobTran->inISOTxnCode == _SALE_ || pobTran->inISOTxnCode ==_REVERSAL_))								||
		    (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_17_WAVE3 && !memcmp(&guszDCC_MTI[0], "0200", 4))													|| 
		    (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_63_NEWJSPEEDY_EMV && !memcmp(&guszDCC_MTI[0], "0200", 4))												|| 
		    (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_91_QUICKPASS && !memcmp(&guszDCC_MTI[0], "0200", 4))													||
		    (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_50_EXPRESSSPAY && !memcmp(&guszDCC_MTI[0], "0200", 4))												||
		    (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE && !memcmp(&guszDCC_MTI[0], "0200", 4))											||
		    (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_42_DPAS && !memcmp(&guszDCC_MTI[0], "0200", 4))													||
		    (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_43_DISCOVER_DPAS_MAG_STRIPE && !memcmp(&guszDCC_MTI[0], "0200", 4)))
		{
			/* 不送 */
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

		if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY		||
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY	|| 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1) &&
		    !memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
		{
			/* 不送 */
		}
		else
		{
			if (pobTran->srEMVRec.in95_TVRLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x95;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in95_TVRLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz95_TVR[0], pobTran->srEMVRec.in95_TVRLen);
				inPacketCnt += pobTran->srEMVRec.in95_TVRLen;
			}
		}

		if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY		||
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY	|| 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1) &&
		    !memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
		{
			/* 不送 */
		}
		else
		{
			if (pobTran->srEMVRec.in9A_TranDateLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9A;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9A_TranDateLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9A_TranDate[0], pobTran->srEMVRec.in9A_TranDateLen);
				inPacketCnt += pobTran->srEMVRec.in9A_TranDateLen;
			}
		}

		/* Add By Tim 2015-04-16 PM 03:19:09 更新規格收單交易系統端末機電文規格V4_4_20150727.pdf */
		if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY		||
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY	|| 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1) &&
		    !memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
		{
			/* 不送 */
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

		if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY		||
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY	|| 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1) &&
		    !memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
		{
			/* 不送 */
		}
		else
		{
			if (pobTran->srEMVRec.in9C_TranTypeLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9C;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9C_TranTypeLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9C_TranType[0], pobTran->srEMVRec.in9C_TranTypeLen);
				inPacketCnt += pobTran->srEMVRec.in9C_TranTypeLen;
			}
		}

		if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY		||
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY	|| 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1) &&
		    !memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
		{
			/* 不送 */
		}
		else
		{
			if (pobTran->srEMVRec.in9F02_AmtAuthNumLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x02;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F02_AmtAuthNumLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F02_AmtAuthNum[0], pobTran->srEMVRec.in9F02_AmtAuthNumLen);
				inPacketCnt += pobTran->srEMVRec.in9F02_AmtAuthNumLen;
			}
		}

		if (pobTran->srEMVRec.in9F03_AmtOtherNumLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x03;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F03_AmtOtherNumLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F03_AmtOtherNum[0], pobTran->srEMVRec.in9F03_AmtOtherNumLen);
			inPacketCnt += pobTran->srEMVRec.in9F03_AmtOtherNumLen;
		}

		if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY		||
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY	|| 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1) &&
		    !memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
		{
			/* 不送 */
		}
		else
		{
			if (pobTran->srEMVRec.in9F10_IssuerAppDataLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x10;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F10_IssuerAppDataLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F10_IssuerAppData[0], pobTran->srEMVRec.in9F10_IssuerAppDataLen);
				inPacketCnt += pobTran->srEMVRec.in9F10_IssuerAppDataLen;
			}
		}

		if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY		||
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY	|| 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1) &&
		    !memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
		{
			/* 不送 */
		}
		else
		{
			if (pobTran->srEMVRec.in9F1A_TermCountryCodeLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x1A;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F1A_TermCountryCodeLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F1A_TermCountryCode[0], pobTran->srEMVRec.in9F1A_TermCountryCodeLen);
				inPacketCnt += pobTran->srEMVRec.in9F1A_TermCountryCodeLen;
			}
		}

		/* Add By Tim 2015-04-16 PM 03:19:09 更新規格收單交易系統端末機電文規格V4_4_20150727.pdf */
		if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY		 || 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY || 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1) &&
		    !memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
		{
			/* 不送 */
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

		if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY		||
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY	|| 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1) &&
		    !memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
		{
			/* 不送 */
		}
		else
		{
			if (pobTran->srEMVRec.in9F26_ApplCryptogramLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x26;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F26_ApplCryptogramLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F26_ApplCryptogram[0], pobTran->srEMVRec.in9F26_ApplCryptogramLen);
				inPacketCnt += pobTran->srEMVRec.in9F26_ApplCryptogramLen;
			}
		}

		if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY		||
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY	|| 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1) &&
		    !memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
		{
			/* 不送 */
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

		if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY		||
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY	|| 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1) &&
		    !memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
		{
			/* 不送 */
		}
		else
		{
			if (pobTran->srEMVRec.in9F33_TermCapabilitiesLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x33;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F33_TermCapabilitiesLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F33_TermCapabilities[0], pobTran->srEMVRec.in9F33_TermCapabilitiesLen);
				inPacketCnt += pobTran->srEMVRec.in9F33_TermCapabilitiesLen;
			}
		}

		/* Add By Tim 2015-04-16 PM 03:19:09 更新規格收單交易系統端末機電文規格V4_4_20150727.pdf */
		if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY			|| 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY	|| 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1) &&
		    !memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
		{
			/* 不送 */
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

		/* Add By Tim 2015-04-16 PM 03:19:09 更新規格收單交易系統端末機電文規格V4_4_20150727.pdf */
		if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY		 || 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY || 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1) &&
		    !memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
		{
			/* 不送 */
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

		if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY		||
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY	|| 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1) &&
		    !memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
		{
			/* 不送 */
		}
		else
		{
			if (pobTran->srEMVRec.in9F36_ATCLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x36;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F36_ATCLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F36_ATC[0], pobTran->srEMVRec.in9F36_ATCLen);
				inPacketCnt += pobTran->srEMVRec.in9F36_ATCLen;
			}
		}

		if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY		||
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY	|| 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1) &&
		    !memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
		{
			/* 不送 */
		}
		else
		{
			if (pobTran->srEMVRec.in9F37_UnpredictNumLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x37;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F37_UnpredictNumLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F37_UnpredictNum[0], pobTran->srEMVRec.in9F37_UnpredictNumLen);
				inPacketCnt += pobTran->srEMVRec.in9F37_UnpredictNumLen;
			}
		}

		/* Add By Tim 2015-04-16 PM 03:19:09 更新規格收單交易系統端末機電文規格V4_4_20150727.pdf */
		if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY		|| 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY	|| 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1) &&
		    !memcmp(&guszDCC_MTI[0], "0320", 4) && pobTran->srBRec.uszOfflineBit == VS_TRUE)
		{
			/* 不送 */
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

		if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_61_JSPEEDY		||
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_65_NEWJSPEEDY_LEGACY	|| 
		     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_16_WAVE1))
		{
			/* 不送 */
		}
		else
		{
			/* Add By Tim 2015-09-10 PM 04:55:00 【需求單103399】VCPS2.1.3規範，請協助評估端末機 DE55新增上傳 FFI (Tag 9F6E) */
			if (pobTran->srEMVRec.in9F6E_From_Factor_IndicatorLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x9F;
				szPacket[inPacketCnt ++] = 0x6E;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F6E_From_Factor_IndicatorLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F6E_From_Factor_Indicator[0], pobTran->srEMVRec.in9F6E_From_Factor_IndicatorLen);
				inPacketCnt += pobTran->srEMVRec.in9F6E_From_Factor_IndicatorLen;
			}
		}
	}
	/* 晶片卡 */
	else
	{
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

		if (!memcmp(guszDCC_MTI, "0200", 4) && pobTran->inISOTxnCode == _SALE_)
		{
			if (pobTran->in57_Track2Len > 0)
			{
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
				if (pobTran->srBRec.uszContactlessBit != VS_TRUE)
					return (VS_ERROR); /* 【WAVE】交易有可能不會回 */
			}
		}
		/* 其他用【卡號】【有效期】 */
		else
		{
			if (pobTran->srEMVRec.in5A_ApplPanLen <= 0 || pobTran->srEMVRec.in5F24_ExpireDateLen <= 0)
				return (VS_ERROR);

			/* 5A */
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

		if (pobTran->srEMVRec.in5F2A_TransCurrCodeLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x5F;
			szPacket[inPacketCnt ++] = 0x2A;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in5F2A_TransCurrCodeLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz5F2A_TransCurrCode[0], pobTran->srEMVRec.in5F2A_TransCurrCodeLen);
			inPacketCnt += pobTran->srEMVRec.in5F2A_TransCurrCodeLen;
		}

		if (pobTran->srEMVRec.in5F34_ApplPanSeqnumLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x5F;
			szPacket[inPacketCnt ++] = 0x34;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in5F34_ApplPanSeqnumLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz5F34_ApplPanSeqnum[0], pobTran->srEMVRec.in5F34_ApplPanSeqnumLen);
			inPacketCnt += pobTran->srEMVRec.in5F34_ApplPanSeqnumLen;
		}

		if (pobTran->srEMVRec.in82_AIPLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x82;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in82_AIPLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz82_AIP[0], pobTran->srEMVRec.in82_AIPLen);
			inPacketCnt += pobTran->srEMVRec.in82_AIPLen;
		}

		if (pobTran->srEMVRec.in84_DFNameLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x84;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in84_DFNameLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz84_DF_NAME[0], pobTran->srEMVRec.in84_DFNameLen);
			inPacketCnt += pobTran->srEMVRec.in84_DFNameLen;
		}

		/* MIT = 〈0220〉 〈0320〉用送 */
		if (!memcmp(&guszDCC_MTI[0], "0220", 4) || !memcmp(&guszDCC_MTI[0], "0320", 4))
		{
			if (pobTran->srEMVRec.in8A_AuthRespCodeLen > 0)
			{
				szPacket[inPacketCnt ++] = 0x8A;
				szPacket[inPacketCnt ++] = pobTran->srEMVRec.in8A_AuthRespCodeLen;
				memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz8A_AuthRespCode[0], pobTran->srEMVRec.in8A_AuthRespCodeLen);
				inPacketCnt += pobTran->srEMVRec.in8A_AuthRespCodeLen;
			}
		}

		if (pobTran->srEMVRec.in95_TVRLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x95;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in95_TVRLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz95_TVR[0], pobTran->srEMVRec.in95_TVRLen);
			inPacketCnt += pobTran->srEMVRec.in95_TVRLen;
		}

		if (pobTran->srEMVRec.in9A_TranDateLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9A;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9A_TranDateLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9A_TranDate[0], pobTran->srEMVRec.in9A_TranDateLen);
			inPacketCnt += pobTran->srEMVRec.in9A_TranDateLen;
		}

		if (pobTran->srEMVRec.in9B_TSILen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9B;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9B_TSILen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9B_TSI[0], pobTran->srEMVRec.in9B_TSILen);
			inPacketCnt += pobTran->srEMVRec.in9B_TSILen;
		}

		if (pobTran->srEMVRec.in9C_TranTypeLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9C;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9C_TranTypeLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9C_TranType[0], pobTran->srEMVRec.in9C_TranTypeLen);
			inPacketCnt += pobTran->srEMVRec.in9C_TranTypeLen;
		}

		if (pobTran->srEMVRec.in9F02_AmtAuthNumLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x02;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F02_AmtAuthNumLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F02_AmtAuthNum[0], pobTran->srEMVRec.in9F02_AmtAuthNumLen);
			inPacketCnt += pobTran->srEMVRec.in9F02_AmtAuthNumLen;
		}

		if (pobTran->srEMVRec.in9F03_AmtOtherNumLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x03;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F03_AmtOtherNumLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F03_AmtOtherNum[0], pobTran->srEMVRec.in9F03_AmtOtherNumLen);
			inPacketCnt += pobTran->srEMVRec.in9F03_AmtOtherNumLen;
		}

		if (pobTran->srEMVRec.in9F09_TermVerNumLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x09;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F09_TermVerNumLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F09_TermVerNum[0], pobTran->srEMVRec.in9F09_TermVerNumLen);
			inPacketCnt += pobTran->srEMVRec.in9F09_TermVerNumLen;
		}

		if (pobTran->srEMVRec.in9F10_IssuerAppDataLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x10;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F10_IssuerAppDataLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F10_IssuerAppData[0], pobTran->srEMVRec.in9F10_IssuerAppDataLen);
			inPacketCnt += pobTran->srEMVRec.in9F10_IssuerAppDataLen;
		}

		if (pobTran->srEMVRec.in9F1A_TermCountryCodeLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x1A;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F1A_TermCountryCodeLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F1A_TermCountryCode[0], pobTran->srEMVRec.in9F1A_TermCountryCodeLen);
			inPacketCnt += pobTran->srEMVRec.in9F1A_TermCountryCodeLen;
		}

		if (pobTran->srEMVRec.in9F1E_IFDNumLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x1E;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F1E_IFDNumLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F1E_IFDNum[0], pobTran->srEMVRec.in9F1E_IFDNumLen);
			inPacketCnt += pobTran->srEMVRec.in9F1E_IFDNumLen;
		}

		if (pobTran->srEMVRec.in9F26_ApplCryptogramLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x26;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F26_ApplCryptogramLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F26_ApplCryptogram[0], pobTran->srEMVRec.in9F26_ApplCryptogramLen);
			inPacketCnt += pobTran->srEMVRec.in9F26_ApplCryptogramLen;
		}

		if (pobTran->srEMVRec.in9F27_CIDLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x27;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F27_CIDLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F27_CID[0], pobTran->srEMVRec.in9F27_CIDLen);
			inPacketCnt += pobTran->srEMVRec.in9F27_CIDLen;
		}

		if (pobTran->srEMVRec.in9F33_TermCapabilitiesLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x33;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F33_TermCapabilitiesLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F33_TermCapabilities[0], pobTran->srEMVRec.in9F33_TermCapabilitiesLen);
			inPacketCnt += pobTran->srEMVRec.in9F33_TermCapabilitiesLen;
		}

		if (pobTran->srEMVRec.in9F34_CVMLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x34;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F34_CVMLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F34_CVM[0], pobTran->srEMVRec.in9F34_CVMLen);
			inPacketCnt += pobTran->srEMVRec.in9F34_CVMLen;
		}

		if (pobTran->srEMVRec.in9F35_TermTypeLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x35;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F35_TermTypeLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F35_TermType[0], pobTran->srEMVRec.in9F35_TermTypeLen);
			inPacketCnt += pobTran->srEMVRec.in9F35_TermTypeLen;
		}

		if (pobTran->srEMVRec.in9F36_ATCLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x36;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F36_ATCLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F36_ATC[0], pobTran->srEMVRec.in9F36_ATCLen);
			inPacketCnt += pobTran->srEMVRec.in9F36_ATCLen;
		}

		if (pobTran->srEMVRec.in9F37_UnpredictNumLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x37;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F37_UnpredictNumLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F37_UnpredictNum[0], pobTran->srEMVRec.in9F37_UnpredictNumLen);
			inPacketCnt += pobTran->srEMVRec.in9F37_UnpredictNumLen;
		}

		if (pobTran->srEMVRec.in9F41_TransSeqCounterLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x41;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F41_TransSeqCounterLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F41_TransSeqCounter[0], pobTran->srEMVRec.in9F41_TransSeqCounterLen);
			inPacketCnt += pobTran->srEMVRec.in9F41_TransSeqCounterLen;
		}

		if (pobTran->srEMVRec.in9F5B_ISRLen > 0)
		{
			szPacket[inPacketCnt ++] = 0x9F;
			szPacket[inPacketCnt ++] = 0x5B;
			szPacket[inPacketCnt ++] = pobTran->srEMVRec.in9F5B_ISRLen;
			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.usz9F5B_ISR[0], pobTran->srEMVRec.in9F5B_ISRLen);
			inPacketCnt += pobTran->srEMVRec.in9F5B_ISRLen;
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
                inLogPrintf(AT, "inNCCC_DCC_Pack55_Software() END!");
	
	return (inCnt);
}

/*
Function        :inNCCC_DCC_Pack56
Date&Time       :2016/9/30 下午 7:07
Describe        :
*/
int inNCCC_DCC_Pack56(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int	inCnt = 0, inPacketCnt = 0;
	char	szASCII[128 + 1] = {0}, szBCD[2 + 1] = {0};
	char	szPacket[64 + 1] = {0};
	char	szTemplate[20 + 1] = {0};

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_DCC_Pack56() START !");
	}
	if (pobTran->srBRec.uszEMVFallBackBit == VS_TRUE)
	{
		/* DFEC */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		if (inFunc_TWNAddDataToEMVPacket(pobTran, TAG_DFEC_FALLBACK_INDICATOR, (unsigned char *)&szTemplate[0]) > 0)
		{
			memset(pobTran->srEMVRec.uszDFEC_FallBackIndicator, 0x00, sizeof(pobTran->srEMVRec.uszDFEC_FallBackIndicator));
			pobTran->srEMVRec.inDFEC_FallBackIndicatorLen = szTemplate[2];
			memcpy((char *)&pobTran->srEMVRec.uszDFEC_FallBackIndicator[0], &szTemplate[3], pobTran->srEMVRec.inDFEC_FallBackIndicatorLen);
		}
		/* ISO Display Debug */
		if (ginDebug == VS_TRUE)
		{
			memset(szASCII, 0x00, sizeof(szASCII));
			inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.uszDFEC_FallBackIndicator[0], pobTran->srEMVRec.inDFEC_FallBackIndicatorLen);

			inCTLS_ISOFormatDebug_DISP("DFEC", pobTran->srEMVRec.inDFEC_FallBackIndicatorLen, szASCII);
		}
		/* ISO Print Debug */
		if (ginISODebug == VS_TRUE)
		{
			memset(szASCII, 0x00, sizeof(szASCII));
			inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.uszDFEC_FallBackIndicator[0], pobTran->srEMVRec.inDFEC_FallBackIndicatorLen);

			inCTLS_ISOFormatDebug_PRINT("DFEC", pobTran->srEMVRec.inDFEC_FallBackIndicatorLen, szASCII);
		}

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
			
//		if (pobTran->srEMVRec.inDFEC_FallBackIndicatorLen > 0)
//		{
//			szPacket[inPacketCnt ++] = 0xDF;
//			szPacket[inPacketCnt ++] = 0xEC;
//			szPacket[inPacketCnt ++] = pobTran->srEMVRec.inDFEC_FallBackIndicatorLen;
//			memcpy(&szPacket[inPacketCnt], (char *)&pobTran->srEMVRec.uszDFEC_FallBackIndicator[0], pobTran->srEMVRec.inDFEC_FallBackIndicatorLen);
//			inPacketCnt += pobTran->srEMVRec.inDFEC_FallBackIndicatorLen;
//		}

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
		/* DFEC */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		if (inFunc_TWNAddDataToEMVPacket(pobTran, TAG_DFEC_FALLBACK_INDICATOR, (unsigned char *)&szTemplate[0]) > 0)
		{
			memset(pobTran->srEMVRec.uszDFEC_FallBackIndicator, 0x00, sizeof(pobTran->srEMVRec.uszDFEC_FallBackIndicator));
			pobTran->srEMVRec.inDFEC_FallBackIndicatorLen = szTemplate[2];
			memcpy((char *)&pobTran->srEMVRec.uszDFEC_FallBackIndicator[0], &szTemplate[3], pobTran->srEMVRec.inDFEC_FallBackIndicatorLen);
		}
		/* ISO Display Debug */
		if (ginDebug == VS_TRUE)
		{
			memset(szASCII, 0x00, sizeof(szASCII));
			inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.uszDFEC_FallBackIndicator[0], pobTran->srEMVRec.inDFEC_FallBackIndicatorLen);

			inCTLS_ISOFormatDebug_DISP("DFEC", pobTran->srEMVRec.inDFEC_FallBackIndicatorLen, szASCII);
		}
		/* ISO Print Debug */
		if (ginISODebug == VS_TRUE)
		{
			memset(szASCII, 0x00, sizeof(szASCII));
			inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.uszDFEC_FallBackIndicator[0], pobTran->srEMVRec.inDFEC_FallBackIndicatorLen);

			inCTLS_ISOFormatDebug_PRINT("DFEC", pobTran->srEMVRec.inDFEC_FallBackIndicatorLen, szASCII);
		}

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

	/* Packet Data Length */
	memset(szASCII, 0x00, sizeof(szASCII));
	sprintf(szASCII, "%04d", inPacketCnt);
	memset(szBCD, 0x00, sizeof(szBCD));
	inFunc_ASCII_to_BCD((unsigned char*)szBCD, szASCII, 2);
	memcpy((char *)&uszPackBuf[inCnt], &szBCD[0], 2);
	inCnt += 2;
	/* Packet Data */
	memcpy((char *)&uszPackBuf[inCnt], &szPacket[0], inPacketCnt);
	inCnt += inPacketCnt;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_DCC_Pack56() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inCnt);
}

/*
Function        :inNCCC_DCC_Pack57_tSAM
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
int inNCCC_DCC_Pack57_tSAM(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int		inCnt = 0, inPacketCnt = 0;
	char		szASCII[4 + 1];
	char		szPacket[100 + 1];
	char		szPacket_Ascii[100 + 1];
	char		szTemplate[100 + 1];
	unsigned char	uszBCD[2 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_DCC_Pack57_tSAM() START!");
	
	
	/* BitMap 8 Byte */
	memset(szPacket, 0x00, sizeof(szPacket));
	szPacket[inPacketCnt ++] = 0x00;
	szPacket[inPacketCnt ++] = 0x00;
	szPacket[inPacketCnt ++] = 0x00;
	szPacket[inPacketCnt ++] = 0x00;

	/* 第5個byte 00100000 */
	if (guszField_DCC35 == VS_TRUE)
		szPacket[inPacketCnt ++] = 0x20;
	else
		szPacket[inPacketCnt ++] = 0x00;

	szPacket[inPacketCnt ++] = 0x00;

	/* 第7個byte 00000010 */
	if (guszField_DCC55 == VS_TRUE)
		szPacket[inPacketCnt ++] = 0x02;
	else
		szPacket[inPacketCnt ++] = 0x00;

	szPacket[inPacketCnt ++] = 0x00;
	
	if (guszField_DCC35 == VS_TRUE)
	{
		memcpy((char *)&szPacket[inPacketCnt], (char *)&gusztSAMCheckSum_DCC35[0], 4);
		inPacketCnt += 4;
	}

	if (guszField_DCC55 == VS_TRUE)
	{
		memcpy((char *)&szPacket[inPacketCnt], (char *)&gusztSAMCheckSum_DCC55[0], 4);
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
                inLogPrintf(AT, "inNCCC_DCC_Pack57_tSAM() END!");
	
        return (inCnt);
}

/*
Function        :inNCCC_DCC_Pack59
Date&Time       :2016/9/2 下午 5:27
Describe        :
*/
int inNCCC_DCC_Pack59(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int		inNCLen = 0;
	int		inCnt = 0, inPacketCnt = 0;
	char		szASCII[4 + 1] = {0}, szBCD[2 + 1] = {0};
	char		szTemplate[1000] = {0}, szPacket[1000 + 1] = {0};
	char		szESCMode[2 + 1] = {0};
	char		szFES_ID[3 + 1] = {0};
	char		szCFESMode[1 + 1] = {0};
	char		szFESMode[2 + 1] = {0};
	unsigned char	uszBCD[2 + 1] = {0};
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_DCC_Pack59 START !");
	}
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memset(szPacket, 0x00, sizeof(szPacket));

	if (pobTran->inISOTxnCode == _DCC_RATE_)
	{
		/* DCC Rate Request Information */
		szPacket[inPacketCnt] = 'R'; /* Table ID */
		inPacketCnt ++;
		szPacket[inPacketCnt] = 86; /* Sub-Data Total Length */
		inPacketCnt ++;
		
		memcpy(&pobTran->srBRec.szDCC_FCNFR[0], &pobTran->srBRec.szDCC_FCN[0], 3);
		if (strlen(&pobTran->srBRec.szDCC_FCN[0]) <= 3)
		{
			memcpy(&szPacket[inPacketCnt], "   ", 3);
		}
		else
		{
			memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szDCC_FCNFR[0], 3);
		}
		inPacketCnt += 3;
		/* 補空白 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		strcpy(szTemplate, "                    ");
		/* Action Code (4 Byte) */
		memcpy(&szPacket[inPacketCnt], &szTemplate[0], 4);
		inPacketCnt += 4;
		/* Foreign Currency Number (3 Byte) */
		memcpy(&szPacket[inPacketCnt], &szTemplate[0], 3);
		inPacketCnt += 3;
		/* Foreign Currency Amount (12 Byte) */
		memcpy(&szPacket[inPacketCnt], &szTemplate[0], 12);
		inPacketCnt += 12;
		/* Foreign Currency Minor Unit (1 Byte) */
		memcpy(&szPacket[inPacketCnt], &szTemplate[0], 1);
		inPacketCnt += 1;
		/* Foreign Currency Alphabetic Code (3 Byte) */
		memcpy(&szPacket[inPacketCnt], &szTemplate[0], 3);
		inPacketCnt += 3;
		/* Exchange Rate Minor Unit (1 Byte ) */
		memcpy(&szPacket[inPacketCnt], &szTemplate[0], 1);
		inPacketCnt += 1;
		/* Exchange Rate Value (9 Byte) */
		memcpy(&szPacket[inPacketCnt], &szTemplate[0], 9);
		inPacketCnt += 9;
		/* Inverted Rate Minor Unit (1 Byte) */
		memcpy(&szPacket[inPacketCnt], &szTemplate[0], 1);
		inPacketCnt += 1;
		/* Inverted Rate Value (9 Byte) */
		memcpy(&szPacket[inPacketCnt], &szTemplate[0], 9);
		inPacketCnt += 9;
		/* Inverted Rate Display Unit (1 Byte) */
		memcpy(&szPacket[inPacketCnt], &szTemplate[0], 1);
		inPacketCnt += 1;
		/* Markup Percentage Value (8 Byte) */
		memcpy(&szPacket[inPacketCnt], &szTemplate[0], 8);
		inPacketCnt += 8;
		/* Markup Percentage Decimal Point (1 Byte) */
		memcpy(&szPacket[inPacketCnt], &szTemplate[0], 1);
		inPacketCnt += 1;
		/* Commission Value Currency Number (3 Byte) */
		memcpy(&szPacket[inPacketCnt], &szTemplate[0], 3);
		inPacketCnt += 3;
		/* Commission Value Currency Amount (12 Byte) */
		memcpy(&szPacket[inPacketCnt], &szTemplate[0], 12);
		inPacketCnt += 12;
		/* Commission Value Minor Unit (1 Byte) */
		memcpy(&szPacket[inPacketCnt], &szTemplate[0], 1);
		inPacketCnt += 1;
		/* Update Call Date & Time (14 Byte) */
		memcpy(&szPacket[inPacketCnt], &szTemplate[0], 14);
		inPacketCnt += 14;
		
		/* (需求單-111060)-DCC流程修改需求 by Russell 2022/4/11 下午 3:26 */
		/* DCC Flow Information */
		szPacket[inPacketCnt] = 'U'; /* Table ID */
		inPacketCnt ++;
		szPacket[inPacketCnt] = 2; /* Sub-Data Total Length */
		inPacketCnt ++;
		memcpy(&szPacket[inPacketCnt], "01", 2);
		inPacketCnt += 2;
	}
	else if (pobTran->inISOTxnCode == _SEND_ESC_ADVICE_)
	{
                /* 【需求單 - 104024】上傳電子簽單至ESC系統 by Tusin - 2015/12/23 下午 02:44:03 */
		memcpy(&szPacket[inPacketCnt], "NE", 2);	/* Table ID */
		inPacketCnt += 2;
		inPacketCnt ++;
		szPacket[inPacketCnt] = 0x01; 	/* Sub-Data Total Length */
		inPacketCnt ++;

		memcpy(&szPacket[inPacketCnt], "N", 1); 
		inPacketCnt ++;
	}
	else if (pobTran->inISOTxnCode == _BATCH_UPLOAD_)
	{
		if (pobTran->srBRec.inCode == _SALE_)
		{
			/* DCC Transaction Information */
			szPacket[inPacketCnt] = 'D'; /* Table ID */
			inPacketCnt ++;
			szPacket[inPacketCnt] = 16; /* Sub-Data Total Length */
			inPacketCnt ++;
			/* Foreign Currency Number (3 Byte) */
			memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szDCC_FCN[0], 3);
			inPacketCnt += 3;
			/* Foreign Currency Amount (12 Byte) */
			memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szDCC_FCA[0], 12);
			inPacketCnt += 12;
			/* Foreign Currency Minor Unit (1 Byte) */
			memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szDCC_FCMU[0], 1);
			inPacketCnt ++;
			/* DCC Exchange Rate information */
			szPacket[inPacketCnt] = 'E'; /* Table ID */
			inPacketCnt ++;
			szPacket[inPacketCnt] = 10; /* Sub-Data Total Length */
			inPacketCnt ++;
			/* Exchange Rate Minor Unit (1 Byte) */
			memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szDCC_ERMU[0], 1);
			inPacketCnt ++;
			/* Exchange Rate Value (9 Byte) */
			memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szDCC_ERV[0], 9);
			inPacketCnt += 9;
			
			/* 【需求單 - 107096】優化行動支付交易功能，DCC交易支援行動支付indicator by Russell 2020/2/12 上午 11:39 */
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
		else if (pobTran->srBRec.inCode == _PRE_COMP_)
		{
			/* DCC Transaction Information */
			szPacket[inPacketCnt] = 'D'; /* Table ID */
			inPacketCnt ++;
			szPacket[inPacketCnt] = 16; /* Sub-Data Total Length */
			inPacketCnt ++;
			/* Foreign Currency Number (3 Byte) */
			memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szDCC_FCN[0], 3);
			inPacketCnt += 3;
			/* Foreign Currency Amount (12 Byte) */
			memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szDCC_FCA[0], 12);
			inPacketCnt += 12;
			/* Foreign Currency Minor Unit (1 Byte) */
			memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szDCC_FCMU[0], 1);
			inPacketCnt ++;
			/* DCC Exchange Rate information */
			szPacket[inPacketCnt] = 'E'; /* Table ID */
			inPacketCnt ++;
			szPacket[inPacketCnt] = 10; /* Sub-Data Total Length */
			inPacketCnt ++;
			/* Exchange Rate Minor Unit (1 Byte) */
			memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szDCC_ERMU[0], 1);
			inPacketCnt ++;
			/* Exchange Rate Value (9 Byte) */
			memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szDCC_ERV[0], 9);
			inPacketCnt += 9;
			/* Pre Authorization Complete Transaction */
			szPacket[inPacketCnt] = 'O'; /* Table ID */
			inPacketCnt ++;
			szPacket[inPacketCnt] = 16; /* Sub-Data Total Length */
			inPacketCnt ++;
			/* Original Transaction Amount(MMDD) (4 Byte) */
			memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szDCC_OTD[0], 4);
			inPacketCnt += 4;
			/* Original Transaction Amount (12 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%010ld00", pobTran->srBRec.lnOrgTxnAmount);
			memcpy(&szPacket[inPacketCnt], &szTemplate[0], 12);
			inPacketCnt += 12;
		}
		else if (pobTran->srBRec.inCode == _REFUND_)
		{
			/* DCC Exchange Rate information */
			szPacket[inPacketCnt] = 'E'; /* Table ID */
			inPacketCnt ++;
			szPacket[inPacketCnt] = 10; /* Sub-Data Total Length */
			inPacketCnt ++;
			/* Exchange Rate Minor Unit (1 Byte) */
			memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szDCC_ERMU[0], 1);
			inPacketCnt ++;
			/* Exchange Rate Value (9 Byte) */
			memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szDCC_ERV[0], 9);
			inPacketCnt += 9;
			/* DCC Information For Refund */
			szPacket[inPacketCnt] = 'F'; /* Table ID */
			inPacketCnt ++;
			szPacket[inPacketCnt] = 71; /* Sub-Data Total Length */
			inPacketCnt ++;
			/* Original Transaction Date & Time (MMDD) (4 Byte) */
        		memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szDCC_OTD[0], 4);
			inPacketCnt += 4;
			/* Original Transaction Amount (12 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%010ld00", pobTran->srBRec.lnOrgTxnAmount);
		
			memset(pobTran->srBRec.szDCC_OTA, 0x00, sizeof(pobTran->srBRec.szDCC_OTA));
			memcpy(&pobTran->srBRec.szDCC_OTA[0], &szTemplate[0], 12);
			
        		memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szDCC_OTA[0], 12);
			inPacketCnt += 12;
			/* Foreign Currency Number (3 Byte) */
			memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szDCC_FCN[0], 3);
			inPacketCnt += 3;
			/* Foreign Currency Amount (12 Byte) */
			memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szDCC_FCA[0], 12);
			inPacketCnt += 12;
			/* Foreign Currency Minor Unit (1 Byte) */
			memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szDCC_FCMU[0], 1);
			inPacketCnt += 1;
			/* Foreign Currency Alphabetic Code (3 Byte) */
			memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szDCC_FCAC[0], 3);
			inPacketCnt += 3;
			/* Inverted Rate Minor Unit (1 Byte) */
			memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szDCC_IRMU[0], 1);
			inPacketCnt += 1;
			/* Inverted Rate Value (9 Byte) */
			memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szDCC_IRV[0], 9);
			inPacketCnt += 9;
			/* Inverted Rate Display Unit (1 Byte) */
			memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szDCC_IRDU[0], 1);
			inPacketCnt += 1;
			/* Markup Percentage Value (8 Byte) */
			memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szDCC_MPV[0], 8);
			inPacketCnt += 8;
			/* Markup Percentage Decimal Point (1 Byte) */
			memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szDCC_MPDP[0], 1);
			inPacketCnt += 1;
			/* Commission Value Currency Number (3 Byte) */
			memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szDCC_CVCN[0], 3);
			inPacketCnt += 3;
			/* Commission Value Currency Amount (12 Byte) */
			memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szDCC_CVCA[0], 12);
			inPacketCnt += 12;
			/* Commission Value Minor Unit (1 Byte) */
			memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szDCC_CVCMU[0], 1);
			inPacketCnt += 1;
			
			/* 【需求單 - 107096】優化行動支付交易功能，DCC交易支援行動支付indicator by Russell 2020/2/12 上午 11:39 */
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
		else if (pobTran->srBRec.inCode == _TIP_)
		{
			/* DCC Transaction Information */
			szPacket[inPacketCnt] = 'D'; /* Table ID */
			inPacketCnt ++;
			szPacket[inPacketCnt] = 16; /* Sub-Data Total Length */
			inPacketCnt ++;
			/* Foreign Currency Number (3 Byte) */
			memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szDCC_FCN[0], 3);
			inPacketCnt += 3;
			/* Foreign Currency Amount (12 Byte) */
			/* 2011-06-07 PM 05:29:27 小費要為加總過後的金額 */
			sprintf(&szPacket[inPacketCnt], "%012lu", atol(pobTran->srBRec.szDCC_FCA) + atol(pobTran->srBRec.szDCC_TIPFCA));
			inPacketCnt += 12;
			/* Foreign Currency Minor Unit (1 Byte) */
			memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szDCC_FCMU[0], 1);
			inPacketCnt ++;
			/* DCC Exchange Rate information */
			szPacket[inPacketCnt] = 'E'; /* Table ID */
			inPacketCnt ++;
			szPacket[inPacketCnt] = 10; /* Sub-Data Total Length */
			inPacketCnt ++;
			/* Exchange Rate Minor Unit (1 Byte) */
			memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szDCC_ERMU[0], 1);
			inPacketCnt ++;
			/* Exchange Rate Value (9 Byte) */
			memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szDCC_ERV[0], 9);
			inPacketCnt += 9;
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
		
		/* TABLE "NE" */
		memset(szESCMode, 0x00, sizeof(szESCMode));
		inGetESCMode(szESCMode);
		if (memcmp(szESCMode, "Y", 1) == 0)
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
	else if (pobTran->inISOTxnCode == _TC_UPLOAD_)
        {
                /* DCC Transaction Information */
		szPacket[inPacketCnt] = 'D'; /* Table ID */
		inPacketCnt ++;
		szPacket[inPacketCnt] = 16; /* Sub-Data Total Length */
		inPacketCnt ++;
		/* Foreign Currency Number (3 Byte) */
		memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szDCC_FCN[0], 3);
		inPacketCnt += 3;
		/* Foreign Currency Amount (12 Byte) */
		memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szDCC_FCA[0], 12);
		inPacketCnt += 12;
		/* Foreign Currency Minor Unit (1 Byte) */
		memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szDCC_FCMU[0], 1);
		inPacketCnt ++;
		/* DCC Exchange Rate information */
		szPacket[inPacketCnt] = 'E'; /* Table ID */
		inPacketCnt ++;
		szPacket[inPacketCnt] = 10; /* Sub-Data Total Length */
		inPacketCnt ++;
		/* Exchange Rate Minor Unit (1 Byte) */
		memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szDCC_ERMU[0], 1);
		inPacketCnt ++;
		/* Exchange Rate Value (9 Byte) */
		memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szDCC_ERV[0], 9);
		inPacketCnt += 9;
		
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
		
                /* TABLE "NE" */
		memset(szESCMode, 0x00, sizeof(szESCMode));
		inGetESCMode(szESCMode);
		if (memcmp(szESCMode, "Y", 1) == 0)
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
	else if ((pobTran->inTransactionCode == _SALE_) ||
		 (pobTran->inTransactionCode == _SALE_ && pobTran->inISOTxnCode == _REVERSAL_))
	{
		/* DCC Transaction Information */
		szPacket[inPacketCnt] = 'D'; /* Table ID */
		inPacketCnt ++;
		szPacket[inPacketCnt] = 16; /* Sub-Data Total Length */
		inPacketCnt ++;
		/* Foreign Currency Number (3 Byte) */
		memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szDCC_FCN[0], 3);
		inPacketCnt += 3;
		/* Foreign Currency Amount (12 Byte) */
		memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szDCC_FCA[0], 12);
		inPacketCnt += 12;
		/* Foreign Currency Minor Unit (1 Byte) */
		memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szDCC_FCMU[0], 1);
		inPacketCnt ++;
		/* DCC Exchange Rate information */
		szPacket[inPacketCnt] = 'E'; /* Table ID */
		inPacketCnt ++;
		szPacket[inPacketCnt] = 10; /* Sub-Data Total Length */
		inPacketCnt ++;
		/* Exchange Rate Minor Unit (1 Byte) */
		memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szDCC_ERMU[0], 1);
		inPacketCnt ++;
		/* Exchange Rate Value (9 Byte) */
		memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szDCC_ERV[0], 9);
		inPacketCnt += 9;
		
		/* (需求單-111060)-DCC流程修改需求 by Russell 2022/4/11 下午 3:26 */
		/* DCC Flow Information */
		szPacket[inPacketCnt] = 'U'; /* Table ID */
		inPacketCnt ++;
		szPacket[inPacketCnt] = 2; /* Sub-Data Total Length */
		inPacketCnt ++;
		memcpy(&szPacket[inPacketCnt], "01", 2);
		inPacketCnt += 2;
		
		/* 【需求單 - 107096】優化行動支付交易功能，DCC交易支援行動支付indicator by Russell 2020/2/12 上午 11:39 */
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
		
		/* TABLE "NE" */
		memset(szESCMode, 0x00, sizeof(szESCMode));
		inGetESCMode(szESCMode);
		if (memcmp(szESCMode, "Y", 1) == 0)
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
	else if ((pobTran->inTransactionCode == _PRE_COMP_) ||
		 (pobTran->inTransactionCode == _PRE_COMP_ && pobTran->inISOTxnCode == _REVERSAL_))
	{
		/* DCC Transaction Information */
		szPacket[inPacketCnt] = 'D'; /* Table ID */
		inPacketCnt ++;
		szPacket[inPacketCnt] = 16; /* Sub-Data Total Length */
		inPacketCnt ++;
		/* Foreign Currency Number (3 Byte) */
		memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szDCC_FCN[0], 3);
		inPacketCnt += 3;
		/* Foreign Currency Amount (12 Byte) */
		memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szDCC_FCA[0], 12);
		inPacketCnt += 12;
		/* Foreign Currency Minor Unit (1 Byte) */
		memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szDCC_FCMU[0], 1);
		inPacketCnt ++;
		/* DCC Exchange Rate information */
		szPacket[inPacketCnt] = 'E'; /* Table ID */
		inPacketCnt ++;
		szPacket[inPacketCnt] = 10; /* Sub-Data Total Length */
		inPacketCnt ++;
		/* Exchange Rate Minor Unit (1 Byte) */
		memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szDCC_ERMU[0], 1);
		inPacketCnt ++;
		/* Exchange Rate Value (9 Byte) */
		memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szDCC_ERV[0], 9);
		inPacketCnt += 9;
		/* Pre Authorization Complete Transaction */
		szPacket[inPacketCnt] = 'O'; /* Table ID */
		inPacketCnt ++;
		szPacket[inPacketCnt] = 16; /* Sub-Data Total Length */
		inPacketCnt ++;
		/* Original Transaction Date(MMDD) (4 Byte) */
		memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szDCC_OTD[0], 4);
		inPacketCnt += 4;
		/* Original Transaction Amount (12 Byte) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010ld00", pobTran->srBRec.lnOrgTxnAmount);
		memcpy(&szPacket[inPacketCnt], &szTemplate[0], 12);
		inPacketCnt += 12;
		
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
		
                /* TABLE "NE" */
		memset(szESCMode, 0x00, sizeof(szESCMode));
		inGetESCMode(szESCMode);
		if (memcmp(szESCMode, "Y", 1) == 0)
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
	else if (pobTran->inISOTxnCode == _REFUND_ ||
		(pobTran->inTransactionCode == _REFUND_ && pobTran->inISOTxnCode == _REVERSAL_))
	{
		/* DCC Information For Refund */
		szPacket[inPacketCnt] = 'F'; /* Table ID */
		inPacketCnt ++;
		szPacket[inPacketCnt] = 71; /* Sub-Data Total Length */
		inPacketCnt ++;
		/* Original Transaction Date & Time (MMDD) (4 Byte) */
		memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szDCC_OTD[0], 4);
		inPacketCnt += 4;
		/* Original Transaction Amount (12 Byte) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010ld00", pobTran->srBRec.lnOrgTxnAmount);
		
		memset(pobTran->srBRec.szDCC_OTA, 0x00, sizeof(pobTran->srBRec.szDCC_OTA));
		memcpy(&pobTran->srBRec.szDCC_OTA[0], &szTemplate[0], 12);
		
		memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szDCC_OTA[0], 12);
		inPacketCnt += 12;
		/* 補空白 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		strcpy(szTemplate, "                    ");
		/* Foreign Currency Number (3 Byte) */
		memcpy(&szPacket[inPacketCnt], &szTemplate[0], 3);
		inPacketCnt += 3;
		/* Foreign Currency Amount (12 Byte) */
		memcpy(&szPacket[inPacketCnt], &szTemplate[0], 12);
		inPacketCnt += 12;
		/* Foreign Currency Minor Unit (1 Byte) */
		memcpy(&szPacket[inPacketCnt], &szTemplate[0], 1);
		inPacketCnt += 1;
		/* Foreign Currency Alphabetic Code (3 Byte) */
		memcpy(&szPacket[inPacketCnt], &szTemplate[0], 3);
		inPacketCnt += 3;
		/* Inverted Rate Minor Unit (1 Byte) */
		memcpy(&szPacket[inPacketCnt], &szTemplate[0], 1);
		inPacketCnt += 1;
		/* Inverted Rate Value (9 Byte) */
		memcpy(&szPacket[inPacketCnt], &szTemplate[0], 9);
		inPacketCnt += 9;
		/* Inverted Rate Display Unit (1 Byte) */
		memcpy(&szPacket[inPacketCnt], &szTemplate[0], 1);
		inPacketCnt += 1;
		/* Markup Percentage Value (8 Byte) */
		memcpy(&szPacket[inPacketCnt], &szTemplate[0], 8);
		inPacketCnt += 8;
		/* Markup Percentage Decimal Point (1 Byte) */
		memcpy(&szPacket[inPacketCnt], &szTemplate[0], 1);
		inPacketCnt += 1;
		/* Commission Value Currency Number (3 Byte) */
		memcpy(&szPacket[inPacketCnt], &szTemplate[0], 3);
		inPacketCnt += 3;
		/* Commission Value Currency Amount (12 Byte) */
		memcpy(&szPacket[inPacketCnt], &szTemplate[0], 12);
		inPacketCnt += 12;
		/* Commission Value Minor Unit (1 Byte) */
		memcpy(&szPacket[inPacketCnt], &szTemplate[0], 1);
		inPacketCnt += 1;
		
		/* 【需求單 - 107096】優化行動支付交易功能，DCC交易支援行動支付indicator by Russell 2020/2/12 上午 11:39 */
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
		
                /* TABLE "NE" */
		memset(szESCMode, 0x00, sizeof(szESCMode));
		inGetESCMode(szESCMode);
		if (memcmp(szESCMode, "Y", 1) == 0)
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
	else if (pobTran->inISOTxnCode == _TIP_	 ||
		(pobTran->inTransactionCode == _TIP_ && pobTran->inISOTxnCode == _REVERSAL_))
	{
		/* DCC Transaction Information */
		szPacket[inPacketCnt] = 'D'; /* Table ID */
		inPacketCnt ++;
		szPacket[inPacketCnt] = 16; /* Sub-Data Total Length */
		inPacketCnt ++;
		/* Foreign Currency Number (3 Byte) */
		memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szDCC_FCN[0], 3);
		inPacketCnt += 3;
		/* Foreign Currency Amount (12 Byte) */
        	/* 2011-06-07 PM 05:29:27 小費要為加總過後的金額 */
        	sprintf(&szPacket[inPacketCnt], "%012lu", atol(pobTran->srBRec.szDCC_FCA) + atol(pobTran->srBRec.szDCC_TIPFCA));
        	inPacketCnt += 12;
		/* Foreign Currency Minor Unit (1 Byte) */
		memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szDCC_FCMU[0], 1);
		inPacketCnt ++;
		/* DCC Exchange Rate information */
		szPacket[inPacketCnt] = 'E'; /* Table ID */
		inPacketCnt ++;
		szPacket[inPacketCnt] = 10; /* Sub-Data Total Length */
		inPacketCnt ++;
		/* Exchange Rate Minor Unit (1 Byte) */
		memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szDCC_ERMU[0], 1);
		inPacketCnt ++;
		/* Exchange Rate Value (9 Byte) */
		memcpy(&szPacket[inPacketCnt], &pobTran->srBRec.szDCC_ERV[0], 9);
		inPacketCnt += 9;
		
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
		
                /* TABLE "NE" */
		memset(szESCMode, 0x00, sizeof(szESCMode));
		inGetESCMode(szESCMode);
		if (memcmp(szESCMode, "Y", 1) == 0)
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
	else if (pobTran->inISOTxnCode == _DCC_UPDATE_INF_)
	{
		/* DCC_UPDATE_INF Request Information */
		szPacket[inPacketCnt] = 'G'; /* Table ID */
		inPacketCnt ++;
		szPacket[inPacketCnt] = 71; /* Sub-Data Total Length */
		inPacketCnt ++;
		/* Installation Indicator (1 Byte) */
		memcpy(&szPacket[inPacketCnt], &gsrDCC_Download.szDCC_Install[0], 1);
		inPacketCnt ++;
		/* BIN Table Version (4 Byte) */
		memcpy(&szPacket[inPacketCnt], &gsrDCC_Download.szDCC_BINVersion[0], 4);
		inPacketCnt += 4;
		/* 雖然規格沒寫，但是不補空白會收到0下載 */
		/* 補空白 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		strcpy(szTemplate, "                                   ");
		/* Batch/File transfer message count (32 Byte) */
		memcpy(&szPacket[inPacketCnt], &szTemplate[0], 8);
		inPacketCnt += 8;
                /* Batch/File transfer file identfication (32 Byte) */
		memcpy(&szPacket[inPacketCnt], &szTemplate[0], 32);
		inPacketCnt += 32;
                /* File Transfer file size (6 Byte) */
		memcpy(&szPacket[inPacketCnt], &szTemplate[0], 6);
		inPacketCnt += 6;
                /* File transfer elementary data record count (6 Byte) */
		memcpy(&szPacket[inPacketCnt], &szTemplate[0], 6);
		inPacketCnt += 6;
                /* File transfer remaining elementary data record count (6 Byte) */
		memcpy(&szPacket[inPacketCnt], &szTemplate[0], 6);
		inPacketCnt += 6;
		/* Available Parameters (8 Byte) */
		memcpy(&szPacket[inPacketCnt], &szTemplate[0], 8);
		inPacketCnt += 8;
		
		/* (需求單-111060)-DCC流程修改需求 by Russell 2022/4/11 下午 3:26 */
		/* DCC Flow Information */
		szPacket[inPacketCnt] = 'U'; /* Table ID */
		inPacketCnt ++;
		szPacket[inPacketCnt] = 2; /* Sub-Data Total Length */
		inPacketCnt ++;
		memcpy(&szPacket[inPacketCnt], "01", 2);
		inPacketCnt += 2;
	}
	else if (pobTran->inISOTxnCode == _DCC_BIN_)
	{
		/* DCC Rate Request Information */
		szPacket[inPacketCnt] = 'H'; /* Table ID */
		inPacketCnt ++;
		/* szPacket[inPacketCnt] = 61; *//* Sub-Data Total Length */
		szPacket[inPacketCnt] = 0;
		szPacket[inPacketCnt + 1] = 61;
		inPacketCnt += 2;
		/* Function Code (3 Byte) */
		memcpy(&szPacket[inPacketCnt], &gsrDCC_Download.szDCC_FC[0], 3);
		inPacketCnt += 3 ;
		/* Batch/File transfer message count (8 Byte) */
		memcpy(&szPacket[inPacketCnt], &gsrDCC_Download.szDCC_BTMC[0], 8);
		inPacketCnt += 8;
		/* Batch/File transfer file identification (32 Byte) */
		memcpy(&szPacket[inPacketCnt], &gsrDCC_Download.szDCC_BTFI[0], 32);
		inPacketCnt += 32;
		/* File Transfer file size (6 Byte) */
		memcpy(&szPacket[inPacketCnt], &gsrDCC_Download.szDCC_FTFS[0], 6);
		inPacketCnt += 6;
		/* File transfer elementary data record count (6 Byte) */
		memcpy(&szPacket[inPacketCnt], &gsrDCC_Download.szDCC_FTEDRC[0], 6);
		inPacketCnt += 6;
		/* File transfer remaining elementary data record count (6 Byte) */
		memcpy(&szPacket[inPacketCnt], &gsrDCC_Download.szDCC_FTREDRC[0], 6);
		inPacketCnt += 6;
	}
	else if (pobTran->inISOTxnCode == _DCC_UPDATE_CLOSE_)
	{
		/* DCC Rate Request Information */
		szPacket[inPacketCnt] = 'I'; /* Table ID */
		inPacketCnt ++;
		szPacket[inPacketCnt] = 6; /* Sub-Data Total Length */
		inPacketCnt ++;
		/* Action Code (4 Byte) */
		memcpy(&szPacket[inPacketCnt], "    ", 4);
                inPacketCnt += 4;
		
		if (atoi(gsrDCC_Download.szDCC_FileList) == atoi(gsrDCC_Download.szDCC_FileList))
		{
			/* Parameter Update Result Code (1 Byte) */
			memcpy(&szPacket[inPacketCnt], "1", 1);
			inPacketCnt ++;
			/* BIN Table Update Result Code (1 Byte) */
			memcpy(&szPacket[inPacketCnt], "1", 1);
			inPacketCnt ++;
		}
		/* 下載失敗 */
		else
		{
			/* Parameter Update Result Code (1 Byte) */
			memcpy(&szPacket[inPacketCnt], "0", 1);
			inPacketCnt ++;
			/* BIN Table Update Result Code (1 Byte) */
			memcpy(&szPacket[inPacketCnt], "0", 1);
			inPacketCnt ++;
		}
		
	}
	else
		return (VS_ERROR);

	/* Packet Data Length */
	memset(szASCII, 0x00, sizeof(szASCII));
	sprintf(szASCII, "%04d", inPacketCnt);
	memset(szBCD, 0x00, sizeof(szBCD));
	inFunc_ASCII_to_BCD((unsigned char*)szBCD, szASCII, 2);
	memcpy((char *)&uszPackBuf[inCnt], &szBCD[0], 2);
	inCnt += 2;
	/* Packet Data */
	memcpy((char *)&uszPackBuf[inCnt], &szPacket[0], inPacketCnt);
	inCnt += inPacketCnt;
	
	return (inCnt);
}

/*
Function        :inNCCC_DCC_Pack60
Date&Time       :
Describe        :Field_60:	Reserved-Private Data
		Record of Count must be Unique in the batch.(Batch Number)
*/
int inNCCC_DCC_Pack60(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 	inCnt = 0;
        long 	lnBatchNum;
        char 	szTemplate[110 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_DCC_Pack60() START!");

	/*
	 For all reconciliation messages, this field will contain the 6 digit batch number.
	 This number may not be zero and will be represented in six bytes.
	 */
	
	if ((pobTran->inISOTxnCode == _TIP_) ||
	    (pobTran->inISOTxnCode == _REVERSAL_ && pobTran->srBRec.inCode == _TIP_))
	{
		/* 小費原始交易 */
		uszPackBuf[inCnt++] = 0x00;
		uszPackBuf[inCnt++] = 0x12;
		sprintf((char *)&uszPackBuf[inCnt], "%010ld00", pobTran->srBRec.lnTxnAmount);
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
                inLogPrintf(AT, "inNCCC_DCC_Pack60() END!");

        return (inCnt);
}

/*
Function        :inNCCC_DCC_Pack62
Date&Time       :
Describe        :Field_62:	Reserved-Private Data(Invoice Number)
*/
int inNCCC_DCC_Pack62(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 	inCnt = 0;
	char	szTemplate[100 + 1];

	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_DCC_Pack62() START!");
	
	
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
                inLogPrintf(AT, "inNCCC_DCC_Pack62() END!");

        return (inCnt);
}

/*
Function        :inNCCC_DCC_Pack63
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
int inNCCC_DCC_Pack63(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int		inCnt = 0;
        int		inAccumCnt, inBatchCnt, inPacketCnt = 0;
	char		szASCII[4 + 1];
        char		szTemplate[100 + 1], szPacket[92 + 1];
	unsigned char	uszBCD[2 + 1];
        ACCUM_TOTAL_REC srAccumRec;

	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_DCC_Pack63() START!");
	
	
	memset(szPacket, 0x00, sizeof(szPacket));
	inBatchCnt = inBATCH_GetTotalCountFromBakFile_By_Sqlite(pobTran);
	if (inBatchCnt == 0)
	{
		/*  Sale Count +  Sale Amount */
		//strcpy(szPacket, "000000000000000");
//		/* Refund Count +  Refund Amount */
		//strcat(szPacket, "000000000000000");
		//inPacketCnt += 30;
//		/* Number of reversals */
//		strcat(szPacket, "000");
//		inPacketCnt += 3;
//		/* Number of adjustments */
//		strcat(szPacket, "000");
//		inPacketCnt += 3;
//		/* Pack Length */
//		uszPackBuf[inCnt++] = 0x00;
//		uszPackBuf[inCnt++] = 0x36;
		strcpy(szPacket, "000000000000000");
		strcat(szPacket, "000000000000000");
		inPacketCnt += 30;
	}
	else
	{
		memset(&srAccumRec, 0x00, sizeof(srAccumRec));
		inAccumCnt = inACCUM_GetRecord(pobTran, &srAccumRec);
		if (inAccumCnt == VS_ERROR)
			return (VS_ERROR);
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
                inLogPrintf(AT, "inNCCC_DCC_Pack63() END!");

        return (inCnt);
}

int inNCCC_DCC_UnPack12(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
	memset(pobTran->srBRec.szTime, 0x00, sizeof(pobTran->srBRec.szTime));
	inFunc_BCD_to_ASCII(&pobTran->srBRec.szTime[0], &uszUnPackBuf[0], 3);
	
	return (VS_SUCCESS);
}

int inNCCC_DCC_UnPack13(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
	memset(&pobTran->srBRec.szDate[4], 0x00, 4);
	inFunc_BCD_to_ASCII(&pobTran->srBRec.szDate[4], &uszUnPackBuf[0], 2);
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_DCC_UnPack38
Date&Time       :
Describe        :
*/
int inNCCC_DCC_UnPack38(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
        if (pobTran->inISOTxnCode == _REVERSAL_)
                return (VS_SUCCESS);
	else
	{
		memset(pobTran->srBRec.szAuthCode, 0x00, sizeof(pobTran->srBRec.szAuthCode));
		memcpy(&pobTran->srBRec.szAuthCode[0], (char *) &uszUnPackBuf[0], _AUTH_CODE_SIZE_);
	}

        return (VS_SUCCESS);
}

int inNCCC_DCC_UnPack39(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
	memset(&pobTran->srBRec.szRespCode, 0x00, sizeof(pobTran->srBRec.szRespCode));
	memcpy(&pobTran->srBRec.szRespCode[0], (char *)&uszUnPackBuf[0], 2);
	
	/* 補0(保險起見) */
	pobTran->srBRec.szRespCode[2] = 0x00;

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_DCC_UnPack55
Date&Time       :2016/9/2 下午 3:12
Describe        :
*/
int inNCCC_DCC_UnPack55(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
        int 	inLen;

	if (pobTran->inISOTxnCode == _REVERSAL_)
		return (VS_SUCCESS);

	/* Add by hanlin 2013/12/5 PM 02:23:11 修正長度解錯的問題 */
	inLen = ((uszUnPackBuf[0] / 16 * 10 + uszUnPackBuf[0] % 16) * 100);
	inLen += (uszUnPackBuf[1] / 16 * 10) + uszUnPackBuf[1] % 16;

	if (inLen > 0)
		inNCCC_EMVUnPackData55(pobTran, &uszUnPackBuf[2], inLen);
	else
	{
		return (VS_ERROR);
	}

        return (VS_SUCCESS);
}

int inNCCC_DCC_UnPack59(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
	
	int	inCnt = 0;
	int	inSubLen, inTotalLen;
	char	szTemplate[32];

	if (pobTran->inISOTxnCode == _DCC_RATE_ || pobTran->inISOTxnCode == _REFUND_ ||
	    pobTran->inISOTxnCode == _DCC_UPDATE_INF_ || pobTran->inISOTxnCode == _DCC_BIN_ ||
	    pobTran->inISOTxnCode == _DCC_EX_RATE_)
	{
		/* Sub Total Length (2 Byte) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%02x%02x", uszUnPackBuf[0], uszUnPackBuf[1]);
		inTotalLen = atoi(szTemplate);
		inCnt += 2;

		while (inTotalLen > inCnt)
		{
			switch (uszUnPackBuf[inCnt])
			{
				case 'N' :
					if (pobTran->inISOTxnCode == _REFUND_ && uszUnPackBuf[inCnt + 1] == 'C')
					{
	        				/* Table ID */
						inCnt += 2;
						/* Table Length */
						inSubLen = ((uszUnPackBuf[inCnt] % 16) * 100) +
							   ((uszUnPackBuf[inCnt + 1] / 16) * 10) + (uszUnPackBuf[inCnt + 1] % 16);
						inCnt += 2;
						/* MCP BANK ID (變動長度) */
						memcpy(&pobTran->srBRec.szMCP_BANKID[0], &uszUnPackBuf[inCnt], inSubLen);

						if (pobTran->srBRec.szMCP_BANKID[0] == 'T')
							pobTran->srBRec.uszMobilePayBit = VS_TRUE;
						else
							pobTran->srBRec.uszMobilePayBit = VS_FALSE;

						inCnt += inSubLen;
					}
					else
						inCnt ++;

					break;
				case 'R' :
					/* Sub-field_Tag (1 Byte) */
					inCnt ++;
					/* Sub-field_Length (1 Byte) */
					inSubLen = uszUnPackBuf[inCnt];
					if (inSubLen != 86)
						return (VS_ERROR);
					else
						inCnt ++;

					/* Foreign Currency No. For Rate (3 Byte) */
					memset(pobTran->srBRec.szDCC_FCNFR, 0x00, sizeof(pobTran->srBRec.szDCC_FCNFR));
					memcpy(&pobTran->srBRec.szDCC_FCNFR[0], (char *)&uszUnPackBuf[inCnt], 3);
					inCnt += 3;
					/* Action Code (4 Byte) */
					memset(pobTran->srBRec.szDCC_AC, 0x00, sizeof(pobTran->srBRec.szDCC_AC));
					memcpy(&pobTran->srBRec.szDCC_AC[0], (char *)&uszUnPackBuf[inCnt], 4);
					inCnt += 4;
					/* Foreign Currency Number (3 Byte) */
					memset(pobTran->srBRec.szDCC_FCN, 0x00, sizeof(pobTran->srBRec.szDCC_FCN));
					memcpy(&pobTran->srBRec.szDCC_FCN[0], (char *)&uszUnPackBuf[inCnt], 3);
					inCnt += 3;
					/* Foreign Currency Amount (12 Byte) */
					memset(pobTran->srBRec.szDCC_FCA, 0x00, sizeof(pobTran->srBRec.szDCC_FCA));
					memcpy(&pobTran->srBRec.szDCC_FCA[0], (char *)&uszUnPackBuf[inCnt], 12);
					inCnt += 12;
					/* Foreign Currency Minor Unit (1 Byte) */
					memset(pobTran->srBRec.szDCC_FCMU, 0x00, sizeof(pobTran->srBRec.szDCC_FCMU));
					memcpy(&pobTran->srBRec.szDCC_FCMU[0], (char *)&uszUnPackBuf[inCnt], 1);
					inCnt ++;
					/* Foreign currcncy Alphabetic Code (3 Byte) */
					memset(pobTran->srBRec.szDCC_FCAC, 0x00, sizeof(pobTran->srBRec.szDCC_FCAC));
					memcpy(&pobTran->srBRec.szDCC_FCAC[0], (char *)&uszUnPackBuf[inCnt], 3);
					inCnt += 3;
					/* Exchange Rate Minor Unit (1 Byte) */
					memset(pobTran->srBRec.szDCC_ERMU, 0x00, sizeof(pobTran->srBRec.szDCC_ERMU));
					memcpy(&pobTran->srBRec.szDCC_ERMU[0], (char *)&uszUnPackBuf[inCnt], 1);
					inCnt ++;
					/* Exchange Rate Value (9 Byte) */
					memset(pobTran->srBRec.szDCC_ERV, 0x00, sizeof(pobTran->srBRec.szDCC_ERV));
					memcpy(&pobTran->srBRec.szDCC_ERV[0], (char *)&uszUnPackBuf[inCnt], 9);
					inCnt += 9;
					/* Inverted Rate Minor Unit (1 Byte) */
					memset(pobTran->srBRec.szDCC_IRMU, 0x00, sizeof(pobTran->srBRec.szDCC_IRMU));
					memcpy(&pobTran->srBRec.szDCC_IRMU[0], (char *)&uszUnPackBuf[inCnt], 1);
					inCnt ++;
					/* Inverted Rate Value (9 Byte) */
					memset(pobTran->srBRec.szDCC_IRV, 0x00, sizeof(pobTran->srBRec.szDCC_IRV));
					memcpy(&pobTran->srBRec.szDCC_IRV[0], (char *)&uszUnPackBuf[inCnt], 9);
					inCnt += 9;
					/* Inverted Rate Display Unit (1 Byte) */
					memset(pobTran->srBRec.szDCC_IRDU, 0x00, sizeof(pobTran->srBRec.szDCC_IRDU));
					memcpy(&pobTran->srBRec.szDCC_IRDU[0], (char *)&uszUnPackBuf[inCnt], 1);
					inCnt ++;
					/* Markup Percentage Value (8 Byte) */
					memset(pobTran->srBRec.szDCC_MPV, 0x00, sizeof(pobTran->srBRec.szDCC_MPV));
					memcpy(&pobTran->srBRec.szDCC_MPV[0], (char *)&uszUnPackBuf[inCnt], 8);
					inCnt += 8;
					/* Markup Percentage Decimal Point (1 Byte) */
					memset(pobTran->srBRec.szDCC_MPDP, 0x00, sizeof(pobTran->srBRec.szDCC_MPDP));
					memcpy(&pobTran->srBRec.szDCC_MPDP[0], (char *)&uszUnPackBuf[inCnt], 1);
					inCnt ++;
					/* Commissino Value Currency Number (3 Byte) */
					memset(pobTran->srBRec.szDCC_CVCN, 0x00, sizeof(pobTran->srBRec.szDCC_CVCN));
					memcpy(&pobTran->srBRec.szDCC_CVCN[0], (char *)&uszUnPackBuf[inCnt], 3);
					inCnt += 3;
					/* Commission Value Currency Amount (12 Byte) */
					memset(pobTran->srBRec.szDCC_CVCA, 0x00, sizeof(pobTran->srBRec.szDCC_CVCA));
					memcpy(&pobTran->srBRec.szDCC_CVCA[0], (char *)&uszUnPackBuf[inCnt], 12);
					inCnt += 12;
					/* Commission Value Currency Minor Unit (1 Byte) */
					memset(pobTran->srBRec.szDCC_CVCMU, 0x00, sizeof(pobTran->srBRec.szDCC_CVCMU));
					memcpy(&pobTran->srBRec.szDCC_CVCMU[0], (char *)&uszUnPackBuf[inCnt], 1);
					inCnt ++;
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memcpy(szTemplate, (char *)&uszUnPackBuf[inCnt], 14);

//					if (memcmp(szTemplate, "              ", 14))
//					{
//					        inNCCC_DCC_SAVE_SCHEDULE(szTemplate, inCnt);
//					}

					inCnt += 14;
					break;
				case 'D' :
					/* Sub-field_Tag (1 Byte) */
					inCnt ++;
					/* Sub-field_Length (1 Byte) */
					inSubLen = uszUnPackBuf[inCnt];
					if (inSubLen != 16)
						return (VS_ERROR);
					else
						inCnt ++;

					/* Foreign Currency Number (3 Byte) */
					memset(pobTran->srBRec.szDCC_FCN, 0x00, sizeof(pobTran->srBRec.szDCC_FCN));
					memcpy(&pobTran->srBRec.szDCC_FCN[0], (char *)&uszUnPackBuf[inCnt], 3);
					inCnt += 3;
					/* Foreign Currency Amount (12 Byte) */
					memset(pobTran->srBRec.szDCC_FCA, 0x00, sizeof(pobTran->srBRec.szDCC_FCA));
					memcpy(&pobTran->srBRec.szDCC_FCA[0], (char *)&uszUnPackBuf[inCnt], 12);
					inCnt += 12;
					/* Foreign Currency Minor Unit (1 Byte) */
					memset(pobTran->srBRec.szDCC_FCMU, 0x00, sizeof(pobTran->srBRec.szDCC_FCMU));
					memcpy(&pobTran->srBRec.szDCC_FCMU[0], (char *)&uszUnPackBuf[inCnt], 1);
					inCnt ++;
					break;
				case 'E' :
					/* Sub-field_Length (1 Byte) */
					inCnt ++;
					inSubLen = uszUnPackBuf[inCnt];
					if (inSubLen != 10)
						return (VS_ERROR);
					else
						inCnt ++;

					/* Exchange Rate Minor Unit (1 Byte) */
					memset(pobTran->srBRec.szDCC_ERMU, 0x00, sizeof(pobTran->srBRec.szDCC_ERMU));
					memcpy(&pobTran->srBRec.szDCC_ERMU[0], (char *)&uszUnPackBuf[inCnt], 1);
					inCnt ++;
					/* Exchange Rate Value (9 Byte) */
					memset(pobTran->srBRec.szDCC_ERV, 0x00, sizeof(pobTran->srBRec.szDCC_ERV));
					memcpy(&pobTran->srBRec.szDCC_ERV[0], (char *)&uszUnPackBuf[inCnt], 9);
					inCnt += 9;
					break;
				case 'F' :
					/* Sub-field_Length (1 Byte) */
					inCnt ++;
					inSubLen = uszUnPackBuf[inCnt];
					if (inSubLen != 71)
						return (VS_ERROR);
					else
						inCnt ++;

					/* Original Transaction Date & Time (MMDD) (4 Byte) */
					inCnt += 4;
					/* Original Transaction Amount (12 Byte) */
					inCnt += 12;
					/* Foreign Currency Number (3 Byte) */
					memset(pobTran->srBRec.szDCC_FCN, 0x00, sizeof(pobTran->srBRec.szDCC_FCN));
					memcpy(&pobTran->srBRec.szDCC_FCN[0], (char *)&uszUnPackBuf[inCnt], 3);
					inCnt += 3;
					/* Foreign Currency Amount (12 Byte) */
					memset(pobTran->srBRec.szDCC_FCA, 0x00, sizeof(pobTran->srBRec.szDCC_FCA));
					memcpy(&pobTran->srBRec.szDCC_FCA[0], (char *)&uszUnPackBuf[inCnt], 12);
					inCnt += 12;
					/* Foreign Currency Minor Unit (1 Byte) */
					memset(pobTran->srBRec.szDCC_FCMU, 0x00, sizeof(pobTran->srBRec.szDCC_FCMU));
					memcpy(&pobTran->srBRec.szDCC_FCMU[0], (char *)&uszUnPackBuf[inCnt], 1);
					inCnt ++;
					/* Foreign currcncy Alphabetic Code (3 Byte) */
					memset(pobTran->srBRec.szDCC_FCAC, 0x00, sizeof(pobTran->srBRec.szDCC_FCAC));
					memcpy(&pobTran->srBRec.szDCC_FCAC[0], (char *)&uszUnPackBuf[inCnt], 3);
					inCnt += 3;
					/* Inverted Rate Minor Unit (1 Byte) */
					memset(pobTran->srBRec.szDCC_IRMU, 0x00, sizeof(pobTran->srBRec.szDCC_IRMU));
					memcpy(&pobTran->srBRec.szDCC_IRMU[0], (char *)&uszUnPackBuf[inCnt], 1);
					inCnt ++;
					/* Inverted Rate Value (9 Byte) */
					memset(pobTran->srBRec.szDCC_IRV, 0x00, sizeof(pobTran->srBRec.szDCC_IRV));
					memcpy(&pobTran->srBRec.szDCC_IRV[0], (char *)&uszUnPackBuf[inCnt], 9);
					inCnt += 9;
					/* Inverted Rate Display Unit (1 Byte) */
					memset(pobTran->srBRec.szDCC_IRDU, 0x00, sizeof(pobTran->srBRec.szDCC_IRDU));
					memcpy(&pobTran->srBRec.szDCC_IRDU[0], (char *)&uszUnPackBuf[inCnt], 1);
					inCnt ++;
					/* Markup Percentage Value (8 Byte) */
					memset(pobTran->srBRec.szDCC_MPV, 0x00, sizeof(pobTran->srBRec.szDCC_MPV));
					memcpy(&pobTran->srBRec.szDCC_MPV[0], (char *)&uszUnPackBuf[inCnt], 8);
					inCnt += 8;
					/* Markup Percentage Decimal Point (1 Byte) */
					memset(pobTran->srBRec.szDCC_MPDP, 0x00, sizeof(pobTran->srBRec.szDCC_MPDP));
					memcpy(&pobTran->srBRec.szDCC_MPDP[0], (char *)&uszUnPackBuf[inCnt], 1);
					inCnt ++;
					/* Commissino Value Currency Number (3 Byte) */
					memset(pobTran->srBRec.szDCC_CVCN, 0x00, sizeof(pobTran->srBRec.szDCC_CVCN));
					memcpy(&pobTran->srBRec.szDCC_CVCN[0], (char *)&uszUnPackBuf[inCnt], 3);
					inCnt += 3;
					/* Commission Value Currency Amount (12 Byte) */
					memset(pobTran->srBRec.szDCC_CVCA, 0x00, sizeof(pobTran->srBRec.szDCC_CVCA));
					memcpy(&pobTran->srBRec.szDCC_CVCA[0], (char *)&uszUnPackBuf[inCnt], 12);
					inCnt += 12;
					/* Commission Value Currency Minor Unit (1 Byte) */
					memset(pobTran->srBRec.szDCC_CVCMU, 0x00, sizeof(pobTran->srBRec.szDCC_CVCMU));
					memcpy(&pobTran->srBRec.szDCC_CVCMU[0], (char *)&uszUnPackBuf[inCnt], 1);
					inCnt ++;
					break;
				case 'G' :
					/* Sub-field_Tag (1 Byte) */
					inCnt ++;
					/* Sub-field_Length (1 Byte) */
					inSubLen = uszUnPackBuf[inCnt];
					inCnt ++;
                                        /* Installation Indicator (1 Byte) */
					memset(gsrDCC_Download.szDCC_Install, 0x00, sizeof(gsrDCC_Download.szDCC_Install));
					memcpy(&gsrDCC_Download.szDCC_Install[0], (char *)&uszUnPackBuf[inCnt], 1);
					inCnt += 1;
                                        /* BIN Table Version (4 Byte) */
					memset(gsrDCC_Download.szDCC_BINVersion, 0x00, sizeof(gsrDCC_Download.szDCC_BINVersion));
					memcpy(&gsrDCC_Download.szDCC_BINVersion[0], (char *)&uszUnPackBuf[inCnt], 4);
					inCnt += 4;
                                        /* Batch/File transfer message count (8 Byte) */
					memset(gsrDCC_Download.szDCC_BTMC, 0x00, sizeof(gsrDCC_Download.szDCC_BTMC));
					memcpy(&gsrDCC_Download.szDCC_BTMC[0], (char *)&uszUnPackBuf[inCnt], 8);
					inCnt += 8;
                                        /* Batch/File transfer file identfication (32 Byte) */
					memset(gsrDCC_Download.szDCC_BTFI, 0x00, sizeof(gsrDCC_Download.szDCC_BTFI));
					memcpy(&gsrDCC_Download.szDCC_BTFI[0], (char *)&uszUnPackBuf[inCnt], 32);
					inCnt += 32;
                                        /* File Transfer file size (6 Byte) */
					memset(gsrDCC_Download.szDCC_FTFS, 0x00, sizeof(gsrDCC_Download.szDCC_FTFS));
					memcpy(&gsrDCC_Download.szDCC_FTFS[0], (char *)&uszUnPackBuf[inCnt], 6);
					inCnt += 6;
                                        /* File transfer elementary data record count (6 Byte) */
					memset(gsrDCC_Download.szDCC_FTEDRC, 0x00, sizeof(gsrDCC_Download.szDCC_FTEDRC));
					memcpy(&gsrDCC_Download.szDCC_FTEDRC[0], (char *)&uszUnPackBuf[inCnt], 6);
					inCnt += 6;
                                        /* File transfer remaining elementary data record count (6 Byte) */
					memset(gsrDCC_Download.szDCC_FTREDRC, 0x00, sizeof(gsrDCC_Download.szDCC_FTREDRC));
					memcpy(&gsrDCC_Download.szDCC_FTREDRC[0], (char *)&uszUnPackBuf[inCnt], 6);
					inCnt += 6;
					/* Available Parameters (1 Byte) */
					memset(gsrDCC_Download.szDCC_AP, 0x00, sizeof(gsrDCC_Download.szDCC_AP));
					memcpy(&gsrDCC_Download.szDCC_AP[0], (char *)&uszUnPackBuf[inCnt], 1);  /* 2012-08-22 PM 03:01:12 add by kakab 只需要一個byte */
					inCnt += 1;
					break;
				case 'H' :
					/* Sub-field_Tag (1 Byte) */
					inCnt ++;
					/* Sub-field_Length (2 Byte) */
					inSubLen = ((uszUnPackBuf[inCnt] % 16) * 256) +
					             ((uszUnPackBuf[inCnt + 1] / 16) * 16) + (uszUnPackBuf[inCnt + 1] % 16);
					inCnt += 2;
                                        /* Function Code (3 Byte) */
					memset(gsrDCC_Download.szDCC_FC, 0x00, sizeof(gsrDCC_Download.szDCC_FC));
					memcpy(&gsrDCC_Download.szDCC_FC[0], (char *)&uszUnPackBuf[inCnt], 3);
					inCnt += 3;
                                        /* Batch/File transfer message count (8 Byte) */
					memset(gsrDCC_Download.szDCC_BTMC, 0x00, sizeof(gsrDCC_Download.szDCC_BTMC));
					memcpy(&gsrDCC_Download.szDCC_BTMC[0], (char *)&uszUnPackBuf[inCnt], 8);
					inCnt += 8;
                                        /* Batch/File transfer file identfication (32 Byte) */
					memset(gsrDCC_Download.szDCC_BTFI, 0x00, sizeof(gsrDCC_Download.szDCC_BTFI));
					memcpy(&gsrDCC_Download.szDCC_BTFI[0], (char *)&uszUnPackBuf[inCnt], 32);
                                        inCnt += 32;
                                        /* File Transfer file size (6 Byte) */
					memset(gsrDCC_Download.szDCC_FTFS, 0x00, sizeof(gsrDCC_Download.szDCC_FTFS));
					memcpy(&gsrDCC_Download.szDCC_FTFS[0], (char *)&uszUnPackBuf[inCnt], 6);
					inCnt += 6;
                                        /* File transfer elementary data record count (6 Byte) */
					memset(gsrDCC_Download.szDCC_FTEDRC, 0x00, sizeof(gsrDCC_Download.szDCC_FTEDRC));
					memcpy(&gsrDCC_Download.szDCC_FTEDRC[0], (char *)&uszUnPackBuf[inCnt], 6);
					inCnt += 6;
                                        /* File transfer remaining elementary data record count (6 Byte) */
					memset(gsrDCC_Download.szDCC_FTREDRC, 0x00, sizeof(gsrDCC_Download.szDCC_FTREDRC));
					memcpy(&gsrDCC_Download.szDCC_FTREDRC[0], (char *)&uszUnPackBuf[inCnt], 6);
					inCnt += 6;
					/* Action Code (4 Byte) */
					memset(gsrDCC_Download.szDCC_AC, 0x00, sizeof(gsrDCC_Download.szDCC_AC));
					memcpy(&gsrDCC_Download.szDCC_AC[0], (char *)&uszUnPackBuf[inCnt], 4);
					inCnt += 4;
					/* Data Record (max to 250 Byte) (data record 66-318 所以要減65) */
					memset(gsrDCC_Download.uszDCC_Record, 0x00, sizeof(gsrDCC_Download.uszDCC_Record));
					memcpy(&gsrDCC_Download.uszDCC_Record[0], (char *)&uszUnPackBuf[inCnt], (inSubLen - 65));
					inCnt += (inSubLen - 65);
					gsrDCC_Download.ulDCC_BINLen = (inSubLen - 65);
					inCnt ++;
					break;
				case 'I' :
				        /* Sub-field_Tag (1 Byte) */
					inCnt ++;
					/* Sub-field_Length (1 Byte) */
					inSubLen = uszUnPackBuf[inCnt];
					if (inSubLen != 6)
						return (VS_ERROR);
					else
						inCnt ++;


					inCnt += 6;
					break;
				case 'X' :
					/* Sub-field_Tag (1 Byte) */
					inCnt ++;
					/* Sub-field_Length (2 Byte) */
					inSubLen = uszUnPackBuf[inCnt];
					inCnt ++;
					/* Data Record (220 Byte) */
					memset(gsrDCC_Download.uszDCC_Record, 0x00, sizeof(gsrDCC_Download.uszDCC_Record));
					memcpy(&gsrDCC_Download.uszDCC_Record[0], (char *)&uszUnPackBuf[inCnt], inSubLen);
					inCnt += inSubLen;
					gsrDCC_Download.ulDCC_BINLen = inSubLen;
				default :
					inCnt ++;
					break;
			} /* End switch () .... */
		} /* End while () ... */
	}
	else if (pobTran->inISOTxnCode == _SALE_ || pobTran->inISOTxnCode == _PRE_COMP_)
	{
		/* Sub Total Length (2 Byte) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%02x%02x", uszUnPackBuf[0], uszUnPackBuf[1]);
		inTotalLen = atoi(szTemplate);
		inCnt += 2;

		while (inTotalLen > inCnt)
		{
			switch (uszUnPackBuf[inCnt])
			{
				case 'N' :
					if (uszUnPackBuf[inCnt + 1] == 'C')
					{
	        				/* Table ID */
						inCnt += 2;
						/* Table Length */
						inSubLen = ((uszUnPackBuf[inCnt] % 16) * 100) +
							   ((uszUnPackBuf[inCnt + 1] / 16) * 10) + (uszUnPackBuf[inCnt + 1] % 16);
						inCnt += 2;
						/* MCP BANK ID (變動長度) */
						memcpy(&pobTran->srBRec.szMCP_BANKID[0], &uszUnPackBuf[inCnt], inSubLen);

						if (pobTran->srBRec.szMCP_BANKID[0] == 'T')
							pobTran->srBRec.uszMobilePayBit = VS_TRUE;
						else
							pobTran->srBRec.uszMobilePayBit = VS_FALSE;

						inCnt += inSubLen;
					}
					else
						inCnt ++;

					break;
				default :
					inCnt ++;
					break;
			}
		}
	}
		
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_DCC_UnPack60
Date&Time       :
Describe        :
*/
int inNCCC_DCC_UnPack60(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
	int	inRetVal = VS_SUCCESS;

	return (inRetVal);
}

int inNCCC_DCC_Check03(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, unsigned char *uszRecePacket)
{
	if (memcmp((char *)uszSendPacket, (char *)uszRecePacket, 3))
	{
		return (VS_ERROR);
	}
	return (VS_SUCCESS);
}

int inNCCC_DCC_Check41(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, unsigned char *uszRecePacket)
{
        if (memcmp((char *)uszSendPacket, (char *)uszRecePacket, 8))
        {
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_DCC_ISOPackMessageType
Date&Time       :2017/2/7 上午 9:27
Describe        :MTI最後修改的地方
*/
int inNCCC_DCC_ISOPackMessageType(TRANSACTION_OBJECT *pobTran, int inTxnCode, unsigned char *uszPackData, char *szMTI)
{
	int		inCnt = 0;
	char		szDebugMsg[100 + 1];
	char		szFinalMTI[4 + 1] = {0};
	unsigned char	uszBCD[10 + 1];
	
	/* 直接使用外面傳進來的MTI，會動到global 變數 */
	memset(szFinalMTI, 0x00, sizeof(szFinalMTI));
	strcpy(szFinalMTI, szMTI);
	
	/* 取消預先授權完成要送0220 */
	if (inTxnCode == _VOID_ && pobTran->srBRec.inOrgCode == _PRE_COMP_)
	{
		memset(szFinalMTI, 0x00, sizeof(szFinalMTI));
		strcpy(szFinalMTI, "0220");
	}
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Final MTI : %s", szFinalMTI);
		inLogPrintf(AT, szDebugMsg);
	}
	
	/* 放到global中方便判斷 */
	memcpy((char*)guszDCC_MTI, szFinalMTI, 4);

	memset(uszBCD, 0x00, sizeof(uszBCD));
        inFunc_ASCII_to_BCD(&uszBCD[0], &szFinalMTI[0], _NCCC_DCC_MTI_SIZE_);
	memcpy((char *)&uszPackData[inCnt], (char *)&uszBCD[0], _NCCC_DCC_MTI_SIZE_);
	inCnt += _NCCC_DCC_MTI_SIZE_;

        return (inCnt);
}

int inNCCC_DCC_ISOModifyBitMap(TRANSACTION_OBJECT *pobTran, int inTxnType, int *inBitMap)
{
	char	szCommMode[2 + 1] = {0}, szEncryptMode[2 + 1] = {0};
	char	szFesMode[2 + 1] = {0};
	char	szCFESMode[2 + 1] = {0};
	
	/* Reversal要根據原交易別，決定要多送哪些欄位 */
	if (pobTran->inISOTxnCode == _REVERSAL_)
	{
		if (pobTran->inTransactionCode == _PRE_COMP_)
		{
			inNCCC_DCC_BitMapSet(inBitMap, 12);
			inNCCC_DCC_BitMapSet(inBitMap, 13);
		}
		else if (pobTran->inTransactionCode == _TIP_)
		{
			inNCCC_DCC_BitMapSet(inBitMap, 12);
			inNCCC_DCC_BitMapSet(inBitMap, 13);
		}
		
		 /* 2011-12-13 PM 03:22:03 add by kakab reversal 除取消交易外 其餘不送 授權碼 */
		if (pobTran->inTransactionCode == _VOID_)
		{
			/* 取消預先授權要帶12、13 */
			if (pobTran->srBRec.inOrgCode == _PRE_COMP_)
			{
				inNCCC_DCC_BitMapSet(inBitMap, 12);
				inNCCC_DCC_BitMapSet(inBitMap, 13);
			}
			
			if (strlen(pobTran->srBRec.szAuthCode) > 0)
				inNCCC_DCC_BitMapSet(inBitMap, 38);
			
			/* 根據DCC EDC SPEC V6.4 取消不帶F_59 */
			inNCCC_DCC_BitMapReset(inBitMap, 59);
		}
	}
	
	/* BatchUpload 的特殊處理 */
	/* BatchUpload TIP要送F_54：Tip的金額 */
        if (pobTran->inISOTxnCode == _BATCH_UPLOAD_)
	{
		if (pobTran->srBRec.inCode == _TIP_)
		{
			inNCCC_DCC_BitMapSet(inBitMap, 54);
		}
		else if (pobTran->inTransactionCode == _VOID_	&&
			 pobTran->srBRec.inOrgCode == _PRE_COMP_)
		{
			/* 取消預先授權要帶12、13 */
			inNCCC_DCC_BitMapSet(inBitMap, 12);
			inNCCC_DCC_BitMapSet(inBitMap, 13);
		}
		else
		{
			/* 一般batch Upload不做處理 */
		}
		
		/* 根據DCC EDC SPEC V6.4 取消不帶F_59 */
		if (pobTran->srBRec.inCode == _VOID_)
		{
			inNCCC_DCC_BitMapReset(inBitMap, 59);
		}
	}
	
	/*  不知為何DCC軟加檢查很嚴格，詢價時不能送F_48而且一定要送F_25 */
	if (pobTran->inISOTxnCode != _DCC_RATE_)
	{
		/* 轉台幣的DCC取消，要送 12、13 */
		if (pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE)
		{
			inNCCC_DCC_BitMapSet(inBitMap, 12);
			inNCCC_DCC_BitMapSet(inBitMap, 13);
		}
		/* 取消預先授權要帶12、13 */
		else if (pobTran->inTransactionCode == _VOID_	&&
			 pobTran->srBRec.inOrgCode == _PRE_COMP_)
		{
			
			inNCCC_DCC_BitMapSet(inBitMap, 12);
			inNCCC_DCC_BitMapSet(inBitMap, 13);
		}
		
		/* ESC轉紙本不須帶F_55 */
		if (pobTran->inISOTxnCode == _SEND_ESC_ADVICE_)
		{

		}
		else if (pobTran->srBRec.uszEMVFallBackBit == VS_TRUE)
		{
			inNCCC_DCC_BitMapSet(inBitMap, 56);
		}
		/* 【需求單 - 106252】	DCC交易新增感應及BIN11碼需求 by Russell 2020/2/20 上午 11:56 */
		else if (pobTran->srBRec.inChipStatus == _EMV_CARD_	|| pobTran->srBRec.uszContactlessBit == VS_TRUE)
		{
			if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE		||
			    pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_64_NEWJSPEEDY_MSD)
			{
				
			}
			else
			{
				inNCCC_DCC_BitMapSet(inBitMap, 55);
				inNCCC_DCC_BitMapSet(inBitMap, 56);
			}
		}
		
		/* F_48 */
		/* 有輸入櫃號才加送F_48， TIPS不送F_48 */
		if (strlen(pobTran->srBRec.szStoreID) > 0)
		{
			/* DCC小費 reversal也不用送櫃號，所以用inorg code by Russell 2020/8/10 上午 10:27 */
			if (pobTran->srBRec.inOrgCode != _TIP_)
				inNCCC_DCC_BitMapSet(inBitMap, 48);
		}
		
		/* 根據DCC EDC SPEC V6.4 取消不帶F_59 */
		if (pobTran->srBRec.inCode == _VOID_)
		{
			inNCCC_DCC_BitMapReset(inBitMap, 59);
		}
	}
	else
	{
		/* MFES 詢價要送F25 (2011-07-13 PM 03:56:30 add by kakab 由於詢價會送F_35 所以M-fes必須送F_25 給 sever判斷需不需要解密)*/
		memset(szFesMode, 0x00, sizeof(szFesMode));
		inGetNCCCFESMode(szFesMode);
		memset(szCFESMode, 0x00, sizeof(szCFESMode));
		inGetCloud_MFES(szCFESMode);
		
		if (memcmp(szFesMode, _NCCC_03_MFES_MODE_, strlen(_NCCC_03_MFES_MODE_)) == 0)
		{
			/* (需求單 - 107276)自助交易標準做法 CFES不用 by Russell 2019/3/6 上午 10:32 */
			if (memcmp(szCFESMode, "Y", strlen("Y")) == 0)
			{
				
			}
			else
			{
				inNCCC_DCC_BitMapSet(inBitMap, 25);
			}
		}
	}
	
	
	/* 撥被備援轉成撥接時，要送不加密的電文 */
	if (pobTran->uszDialBackup == VS_TRUE)
	{
		inNCCC_DCC_BitMapReset(inBitMap, 57);
	}
	else
	{
		/* 有Tsam 要加送 F_57，除了0500和CUP LOGON */
		memset(szCommMode, 0x00, sizeof(szCommMode));
		inGetCommMode(szCommMode);

		if (memcmp(szCommMode, _COMM_ETHERNET_MODE_, 1) == 0	|| 
		    memcmp(szCommMode, _COMM_GPRS_MODE_, 1) == 0	||
		    memcmp(szCommMode, _COMM_3G_MODE_, 1) == 0		||
		    memcmp(szCommMode, _COMM_4G_MODE_, 1) == 0)
		{
			if (inTxnType == _SETTLE_ || inTxnType == _CLS_BATCH_)
			{

			}
			else
			{
				memset(szEncryptMode, 0x00, sizeof(szEncryptMode));
				inGetEncryptMode(szEncryptMode);

				if (memcmp(szEncryptMode, _NCCC_ENCRYPTION_TSAM_, 1) == 0)
				{
					if (pobTran->inISOTxnCode == _DCC_UPDATE_INF_	||
					    pobTran->inISOTxnCode == _DCC_BIN_		||
					    pobTran->inISOTxnCode == _DCC_UPDATE_CLOSE_	||
					    pobTran->inISOTxnCode == _DCC_EX_RATE_)
					{
						/* 參數下載不送 */
					}
					else
					{
						inNCCC_DCC_BitMapSet(inBitMap, 57);
					}
				}
			}
		}
		
	}

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_DCC_ISOModifyPackData
Date&Time       :2016/9/14 上午 10:16
Describe        :
*/
int inNCCC_DCC_ISOModifyPackData(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackData, int *inPackLen)
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
				uszPackData[4] = gusztSAMKeyIndex_DCC; /* TPDU 最後一個 Byte */
		}
	
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_DCC_ISOCheckHeader
Date&Time       :2016/9/14 上午 10:16
Describe        :
*/
int inNCCC_DCC_ISOCheckHeader(TRANSACTION_OBJECT *pobTran, char *szSendISOHeader, char *szReceISOHeader)
{
	int	inCnt = 0;

	/* 檢查TPDU */
	inCnt += _NCCC_DCC_TPDU_SIZE_;
	/* 檢查MTI */
	szSendISOHeader[inCnt + 1] += 0x10;
	if (memcmp(&szSendISOHeader[inCnt], &szReceISOHeader[inCnt], _NCCC_DCC_MTI_SIZE_))
		return (VS_ERROR);

	return (VS_SUCCESS);
}

/*
Function        :inNCCC_DCC_ISOOnlineAnalyse
Date&Time       :2016/9/14 上午 10:17
Describe        :
*/
int inNCCC_DCC_ISOOnlineAnalyse(TRANSACTION_OBJECT *pobTran)
{
        int inRetVal = VS_ERROR;

        if (pobTran->srBRec.inChipStatus == _EMV_CARD_ && (pobTran->inTransactionCode == _SALE_	||
							   pobTran->inTransactionCode == _PRE_AUTH_))
	{
		inRetVal = inNCCC_DCC_OnlineAnalyseEMV(pobTran);
	}
	else
	{
		inRetVal = inNCCC_DCC_OnlineAnalyseMagneticManual(pobTran);
		
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
Function        :inNCCC_DCC_ISOAdviceAnalyse
Date&Time       :2016/9/14 上午 10:17
Describe        :
*/
int inNCCC_DCC_ISOAdviceAnalyse(TRANSACTION_OBJECT *pobTran, unsigned char *uszTcUpload)
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

		/* 若非當筆TC送完在這裡把bit On起來，避免TC重送 */
		if (pobTran->srBRec.inChipStatus == _EMV_CARD_ && pobTran->inISOTxnCode == _TC_UPLOAD_)
			pobTran->srBRec.uszTCUploadBit = VS_TRUE;
		
                /* pobTran->uszUpdateBatchBit 表示 uszUpdateBatchBit / TRANS_BATCH_KEY】是要更新記錄 */
                pobTran->uszUpdateBatchBit = VS_TRUE;
	
		/* 若不是當筆TCUpload，而是送一般advice，要做batch更新和更新advice檔 */
		if (pobTran->uszTCUploadBit != VS_TRUE)
		{
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
                /* 結帳交易流程中，若於前帳前補送電文，補送電文有拒絕或其他回覆碼的狀況，畫面皆顯示結帳失敗即可，不須顯示補送電文之回覆碼訊息。 */
        }

        return (VS_ERROR);
}

/*
Function        :inNCCC_DCC_BitMapSet
Date&Time       :2016/9/5 下午 6:43
Describe        :
*/
int inNCCC_DCC_BitMapSet(int *inBitMap, int inFeild)
{
        int 	i, inBMapCnt, inBitMapCnt;
        int 	inBMap[_NCCC_DCC_MAX_BIT_MAP_CNT_];

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

/*
Function        :inNCCC_DCC_BitMapReset
Date&Time       :2016/9/5 下午 6:43
Describe        :
*/
int inNCCC_DCC_BitMapReset(int *inBitMap, int inFeild)
{
        int 	i, inBMapCnt;
        int 	inBMap[_NCCC_DCC_MAX_BIT_MAP_CNT_];

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

/*
Function        :inNCCC_DCC_BitMapCheck
Date&Time       :2016/9/2 下午 3:24
Describe        :
*/
int inNCCC_DCC_BitMapCheck(unsigned char *inBitMap, int inFeild)
{
        int 	inByteIndex, inBitIndex;

        inFeild--;
        inByteIndex = inFeild / 8;
        inBitIndex = 7 - (inFeild - inByteIndex * 8);

        if (_NCCC_DCC_CHECK_BIT_MAP_(inBitMap[inByteIndex], inBitIndex))
        {
                return (VS_TRUE);
        }

        return (VS_FALSE);
}

/*
Function        :inNCCC_DCC_CopyBitMap
Date&Time       :
Describe        :
*/
int inNCCC_DCC_CopyBitMap(int *inBitMap, int *inSearchBitMap)
{
        int 	i;

        for (i = 0; i < _NCCC_DCC_MAX_BIT_MAP_CNT_; i++)
        {
                if (inSearchBitMap[i] == 0)
                        break;
                else
                        inBitMap[i] = inSearchBitMap[i];
        }

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_DCC_GetBitMapTableIndex
Date&Time       :2016/9/2 上午 11:05
Describe        :
*/
int inNCCC_DCC_GetBitMapTableIndex(ISO_TYPE_NCCC_DCC_TABLE *srISOFunc, int inBitMapTxnCode)
{
        int 	inBitMapIndex;

        for (inBitMapIndex = 0;; inBitMapIndex++)
        {
                if (srISOFunc->srBitMap[inBitMapIndex].inTxnID == _NCCC_DCC_NULL_TX_)
                {
                        return (VS_ERROR); /* 找不到相對應交易別的Bit Map */
                }

                if (srISOFunc->srBitMap[inBitMapIndex].inTxnID == inBitMapTxnCode)
                        break; /* 找到一樣的交易類別 */
        }

        return (inBitMapIndex);
}

/*
Function        :inNCCC_DCC_GetBitMapMessagegTypeField03
Date&Time       :2016/9/2 上午 11:05
Describe        :
*/
int inNCCC_DCC_GetBitMapMessagegTypeField03(TRANSACTION_OBJECT *pobTran, ISO_TYPE_NCCC_DCC_TABLE *srISOFunc, int inTxnType, int *inTxnBitMap,
                unsigned char *uszSendBuf)
{
        int 		i, j, k, inBitMapIndex, *inBitMap, inCnt, inBitMapTxnCode = -1;
        unsigned char 	uszBuf;
        char		szTemplate[64 + 1];

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_DCC_GetBitMapMessagegTypeField03() START!");
        
        if (ginDebug == VS_TRUE)
        {
                sprintf(szTemplate, "inTxnType = %d", inTxnType);
                inLogPrintf(AT, szTemplate);
        }      
        
        /* 設定交易別 */
        inBitMapTxnCode = inTxnType;

        /* 要搜尋 BIT_MAP_NCCC_DCC_TABLE srNCCC_DCC_ISOBitMap 相對應的 inTxnID */
        if ((inBitMapIndex = inNCCC_DCC_GetBitMapTableIndex(srISOFunc, inBitMapTxnCode)) == VS_ERROR)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inNCCC_DCC_GetBitMapTableIndex == VS_ERROR");
                
                return (VS_ERROR);
        }

        /* Pack Message Type */
        inCnt = 0;
        inCnt += srISOFunc->inPackMTI(pobTran, inTxnType, &uszSendBuf[inCnt], srISOFunc->srBitMap[inBitMapIndex].szMTI);

        /* 要搜尋 BIT_MAP_NCCC_DCC_TABLE srNCCC_DCC_ISOBitMap 相對應的 inBitMap */
        inNCCC_DCC_CopyBitMap(inTxnBitMap, srISOFunc->srBitMap[inBitMapIndex].inBitMap);

        /* 修改 Bit Map */
        srISOFunc->inModifyBitMap(pobTran, inTxnType, inTxnBitMap);
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

        inCnt += _NCCC_DCC_BIT_MAP_SIZE_;

        /* Process Code */
		
        memset(guszNCCC_DCC_ISO_Field03, 0x00, sizeof(guszNCCC_DCC_ISO_Field03));
        inFunc_ASCII_to_BCD(&guszNCCC_DCC_ISO_Field03[0], srISOFunc->srBitMap[inBitMapIndex].szPCode, 3);

        if (inBitMapTxnCode == _REVERSAL_)
        {
                if (pobTran->inTransactionCode == _VOID_)
                {
                        /*
                         Processing Code		Activities
                         --------------------------------------------------------
                         020000	|	Reversal Void Sale
                         220000	|	Reversal Void Refund		 
			 420000	|	Reversal Void Pre-Auth Comp
                         */
                        switch (pobTran->srBRec.inOrgCode)
                        {
                                case _SALE_:
                                        guszNCCC_DCC_ISO_Field03[0] = 0x02;
                                        guszNCCC_DCC_ISO_Field03[1] = 0x00;
                                        break;
                                case _REFUND_:
                                        guszNCCC_DCC_ISO_Field03[0] = 0x22;
                                        guszNCCC_DCC_ISO_Field03[1] = 0x00;
                                        break;
				case _PRE_COMP_:
					guszNCCC_DCC_ISO_Field03[0] = 0x42;
                                        guszNCCC_DCC_ISO_Field03[1] = 0x00;
					break;
                                default:
                                        guszNCCC_DCC_ISO_Field03[0] = 0x00;
                                        break;
                        }
                }
                else
                {
                        /*
                         Processing Code		Activities
                         --------------------------------------------------------
                         000000	|	Reversal Sale
			 020000	|	Reversal Cash Advance
                         200000	|	Reversal Refund		 
			 400000	|	Reversal Pre-Auth Comp
                         */
                        switch (pobTran->inTransactionCode)
                        {
                                case _SALE_:
                                        guszNCCC_DCC_ISO_Field03[0] = 0x00;
                                        guszNCCC_DCC_ISO_Field03[1] = 0x00;
					break;
				case _TIP_:
					guszNCCC_DCC_ISO_Field03[0] = 0x02;
                                        guszNCCC_DCC_ISO_Field03[1] = 0x00;
					break;
                                case _REFUND_:
                                        guszNCCC_DCC_ISO_Field03[0] = 0x20;
                                        guszNCCC_DCC_ISO_Field03[1] = 0x00;
					break;
				case _PRE_COMP_:
					guszNCCC_DCC_ISO_Field03[0] = 0x40;
                                        guszNCCC_DCC_ISO_Field03[1] = 0x00;
					break;
					break;
                                default:
                                        guszNCCC_DCC_ISO_Field03[0] = 0x00;
                                        break;
                        }
                }
        }
        else if (inBitMapTxnCode == _BATCH_UPLOAD_)
        {
                /*
                 Processing Code		Activities
                 --------------------------------------------------------
                 000000	|	Sale
		 200000	|	Refund
                 */
                switch (pobTran->srBRec.inOrgCode)
                {
                        case _SALE_:
				guszNCCC_DCC_ISO_Field03[0] = 0x00;
				guszNCCC_DCC_ISO_Field03[1] = 0x00;
				break;
			case _REFUND_:
				guszNCCC_DCC_ISO_Field03[0] = 0x20;
				guszNCCC_DCC_ISO_Field03[1] = 0x00;
				break;
                        default:
                                break;
                }

                if (pobTran->uszLastBatchUploadBit == VS_TRUE)
                        guszNCCC_DCC_ISO_Field03[2] = 0x00;
                else
                        guszNCCC_DCC_ISO_Field03[2] = 0x01;

        }
        else if (inBitMapTxnCode == _VOID_)
        {
                /*
                 Processing Code		Activities
                 --------------------------------------------------------
		 020000	|	Void Sale
		 220000	|	Void Refund
		 * 
		 * pre-comp的離線取消應該也是020000		 
                 */
                switch (pobTran->srBRec.inOrgCode)
                {
                        case _SALE_:
                        case _TIP_:
			case _PRE_COMP_:
                                guszNCCC_DCC_ISO_Field03[0] = 0x02;
				guszNCCC_DCC_ISO_Field03[1] = 0x00;
				break;
			case _REFUND_:
				guszNCCC_DCC_ISO_Field03[0] = 0x22;
				guszNCCC_DCC_ISO_Field03[1] = 0x00;
				break;
			default:
                                break;
                }
		
        }
	
	/* DCC還沒做 先註解*/
//	else if (inBitMapTxnCode == _ADVICE_ && pobTran->srBRec.fDCC_046_OfflineVoid == VS_TRUE &&
//		 pobTran->srBRec.inOrgCode == _PRE_COMP_)
//	{
//		/*  Advice(Offline Sale)新增DCC預先授權完成轉台幣支付的Processing Code=400000。 */
//		gszDCC_ISO_Field03[0] = 0x40;
//	}

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_DCC_GetBitMapMessagegTypeField03() END!");
        
        return (inCnt);
}

/*
Function        :inNCCC_DCC_PackISO
Date&Time       :2016/9/2 上午 10:42
Describe        :Pack電文
*/
int inNCCC_DCC_PackISO(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendBuf, int inTxnCode)
{
        int				i, inSendCnt, inField, inCnt;
        int				inBitMap[_NCCC_DCC_MAX_BIT_MAP_CNT_ + 1];
        int				inRetVal, inISOFuncIndex = -1;
	char				szTemplate[42 + 1];
        char				szLogMessage[40 + 1];
	char				szCommMode[1 + 1];
	char				szEncryptMode[1 +1];
        unsigned char			uszBCD[20 + 1];
        ISO_TYPE_NCCC_DCC_TABLE 	srISOFunc;		
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_DCC_PackISO() START!");
	}

        inSendCnt = 0;
        inField = 0;

        /* 加密模式，預設值 = 0。【0 = 不加密，1 = tSAM 加密】 */
	memset(szCommMode, 0x00, sizeof(szCommMode));
	inGetCommMode(szCommMode);

	if (memcmp(szCommMode, _COMM_MODEM_MODE_, 1) == 0)
		inISOFuncIndex = 0; /* 不加密 */
	else if (memcmp(szCommMode, _COMM_ETHERNET_MODE_, 1) == 0)
	{
		guszField_DCC35 = VS_FALSE;
		guszField_DCC55 = VS_FALSE;
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
		guszField_DCC35 = VS_FALSE;
		guszField_DCC55 = VS_FALSE;
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
		/* 防呆 */
		if (ginDebug == VS_TRUE)
		{
			memset(szLogMessage, 0x00, sizeof(szLogMessage));
			sprintf(szLogMessage,"No such CommMode! szCommMode = %d", atoi(szCommMode));
			inLogPrintf(AT, szLogMessage);
		}
		
		return (VS_ERROR);
	}
	
	/* DCC有軟加密 */
	if (inISOFuncIndex >= 3 || inISOFuncIndex < 0)
	{
		/* 防呆 */
		if (ginDebug == VS_TRUE)
		{
			memset(szLogMessage, 0x00, sizeof(szLogMessage));
			sprintf(szLogMessage,"szEncryptMode ERROR!! szEncryptMode = %d",inISOFuncIndex);
			inLogPrintf(AT, szLogMessage);
		}
		
		return (VS_ERROR);
	}
	
        /* 決定要執行第幾個 Function Index */
        memset((char *) &srISOFunc, 0x00, sizeof(srISOFunc));
        memcpy((char *) &srISOFunc, (char *) &srNCCC_DCC_ISOFunc[inISOFuncIndex], sizeof(srISOFunc));
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
        memcpy((char *) &uszSendBuf[inSendCnt], (char *) uszBCD, _NCCC_DCC_TPDU_SIZE_);
        inSendCnt += _NCCC_DCC_TPDU_SIZE_;
        /* Get Bit Map / Mesage Type / Processing Code */
        inRetVal = inNCCC_DCC_GetBitMapMessagegTypeField03(pobTran, &srISOFunc, inTxnCode, inBitMap, &uszSendBuf[inSendCnt]);
        if (inRetVal == VS_ERROR)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inNCCC_DCC_GetBitMapMessagegTypeField03() ERROR!");

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

        if (srISOFunc.inModifyPackData != _NCCC_DCC_NULL_TX_)
                srISOFunc.inModifyPackData(pobTran, uszSendBuf, &inSendCnt);
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_DCC_PackISO() END!");

        return (inSendCnt);
}

/*
Function        :inNCCC_DCC_CheckUnPackField
Date&Time       :2016/9/2 下午 3:51
Describe        :
*/
int inNCCC_DCC_CheckUnPackField(int inField, ISO_FIELD_NCCC_DCC_TABLE *srCheckUnPackField)
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

/*
Function        :inNCCC_DCC_GetCheckField
Date&Time       :2016/9/2 下午 3:51
Describe        :
*/
int inNCCC_DCC_GetCheckField(int inField, ISO_CHECK_NCCC_DCC_TABLE *ISOFieldCheck)
{
        int 	i;

        for (i = 0; i < 64; i++)
        {
                if (ISOFieldCheck[i].inFieldNum == 0)
                        break;
                else if (ISOFieldCheck[i].inFieldNum == inField)
                {
                        return (i); /* i 是 ISO_CHECK_NCCC_DCC_TABLE 裡的第幾個 */
                }
        }

        return (VS_ERROR);
}

/*
Function        :inNCCC_DCC_GetFieldLen
Date&Time       :2016/9/2 下午 3:27
Describe        :
*/
int inNCCC_DCC_GetFieldLen(int inField, unsigned char *uszSendData, ISO_FIELD_TYPE_NCCC_DCC_TABLE *srFieldType)
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
                        case _NCCC_DCC_ISO_ASC_:
                                inCnt += srFieldType[i].inFieldLen;
                                break;
                        case _NCCC_DCC_ISO_BCD_:
                                inCnt += srFieldType[i].inFieldLen / 2;
                                break;
                        case _NCCC_DCC_ISO_NIBBLE_2_:
                                inLen = (uszSendData[0] / 16 * 10) + (uszSendData[0] % 16);
                                inCnt += ((inLen + 1) / 2) + 1;
                                break;
                        case _NCCC_DCC_ISO_NIBBLE_3_:
                                inLen = (uszSendData[0] / 16 * 1000) + (uszSendData[0] % 16 * 100) + (uszSendData[1] / 16 * 10) + (uszSendData[1] % 16);
                                inCnt += ((inLen + 1) / 2) + 2;
                                break;
                        case _NCCC_DCC_ISO_BYTE_2_:
                                inLen = (uszSendData[0] / 16 * 10) + (uszSendData[0] % 16);
                                inCnt += inLen + 1;
                                break;
                        case _NCCC_DCC_ISO_BYTE_3_:
                                inLen = (uszSendData[0] / 16 * 1000) + (uszSendData[0] % 16 * 100) + (uszSendData[1] / 16 * 10) + (uszSendData[1] % 16);
                                inCnt += inLen + 2;
                                break;
                        case _NCCC_DCC_ISO_BYTE_2_H_:
                                inLen = (int) uszSendData[0];
                                inCnt += inLen + 1;
                                break;
                        case _NCCC_DCC_ISO_BYTE_3_H_:
                                inLen = ((int) uszSendData[0] * 0xFF) + (int) uszSendData[1];
                                inCnt += inLen + 1;
                                break;
			case _NCCC_DCC_ISO_BYTE_1_:
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

/*
Function        :inNCCC_DCC_GetFieldIndex
Date&Time       :2016/9/2 下午 3:27
Describe        :
*/
int inNCCC_DCC_GetFieldIndex(int inField, ISO_FIELD_TYPE_NCCC_DCC_TABLE *srFieldType)
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

/*
Function        :inNCCC_DCC_UnPackISO
Date&Time       :2016/9/2 下午 3:46
Describe        :
*/
int inNCCC_DCC_UnPackISO(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendBuf, unsigned char *uszRecvBuf)
{
	int				inRetVal;
        int				i, inRecvCnt, inCnt, inSendCnt, inSendField, inRecvField, inISOFuncIndex = -1;
        char				szBuf[_NCCC_DCC_TPDU_SIZE_ + _NCCC_DCC_MTI_SIZE_ + _NCCC_DCC_BIT_MAP_SIZE_ + 1];
        char				szErrorMessage[40 + 1];
	char				szCommMode[1 + 1];
	char				szEncryptMode[1 +1];
        unsigned char			uszSendMap[_NCCC_DCC_BIT_MAP_SIZE_ + 1], uszReceMap[_NCCC_DCC_BIT_MAP_SIZE_ + 1];
        ISO_TYPE_NCCC_DCC_TABLE 	srISOFunc;


        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_DCC_UnPackISO() START!");

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
		guszField_DCC35 = VS_FALSE;
		guszField_DCC55 = VS_FALSE;
		memset(szEncryptMode, 0x00, sizeof(szEncryptMode));
		inGetEncryptMode(szEncryptMode);
		inISOFuncIndex = atoi(szEncryptMode);
	}
	else if (memcmp(szCommMode, _COMM_GPRS_MODE_, 1) == 0	||
		 memcmp(szCommMode, _COMM_3G_MODE_, 1) == 0	||
		 memcmp(szCommMode, _COMM_4G_MODE_, 1) == 0)
	{
		guszField_DCC35 = VS_FALSE;
		guszField_DCC55 = VS_FALSE;
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
	
	/* DCC有軟加密 */
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
        memcpy((char *) &srISOFunc, (char *) &srNCCC_DCC_ISOFunc[inISOFuncIndex], sizeof(srISOFunc));

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

        inSendCnt += _NCCC_DCC_TPDU_SIZE_;
        inRecvCnt += _NCCC_DCC_TPDU_SIZE_;
        inSendCnt += _NCCC_DCC_MTI_SIZE_;
        inRecvCnt += _NCCC_DCC_MTI_SIZE_;

        memcpy((char *) uszSendMap, (char *) &uszSendBuf[inSendCnt], _NCCC_DCC_BIT_MAP_SIZE_);
        memcpy((char *) uszReceMap, (char *) &uszRecvBuf[inRecvCnt], _NCCC_DCC_BIT_MAP_SIZE_);

        inSendCnt += _NCCC_DCC_BIT_MAP_SIZE_;
        inRecvCnt += _NCCC_DCC_BIT_MAP_SIZE_;

        /* 先檢查 ISO Field_39 */
        if (inNCCC_DCC_BitMapCheck(uszReceMap, 39) == VS_FALSE)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inNCCC_DCC_BitMapCheck(39) ERROR");

                return (VS_ERROR);
        }

        if (inNCCC_DCC_BitMapCheck(uszReceMap, 41) == VS_FALSE)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inNCCC_DCC_BitMapCheck(41) ERROR");

                return (VS_ERROR);
        }

        for (i = 1; i <= 64; i++)
        {
                /* 有送出去的 Field 但沒有收回來的 Field */
                if (inNCCC_DCC_BitMapCheck(uszSendMap, i) && !inNCCC_DCC_BitMapCheck(uszReceMap, i))
                {
                        inSendCnt += inNCCC_DCC_GetFieldLen(i, &uszSendBuf[inSendCnt], srISOFunc.srISOFieldType);
                }
                else if (inNCCC_DCC_BitMapCheck(uszReceMap, i)) /* 收的 BitMap */
                {
                        if (inNCCC_DCC_BitMapCheck(uszSendMap, i)) /* 送的 BitMap 都有 */
                        {
                                /* 是否要進行檢查封包資料包含【送】【收】 */
                                if ((inSendField = inNCCC_DCC_GetCheckField(i, srISOFunc.srCheckISO)) != VS_ERROR)
                                {
                                        if (srISOFunc.srCheckISO[inSendField].inISOCheck(pobTran, &uszSendBuf[inSendCnt], &uszRecvBuf[inRecvCnt]) != VS_SUCCESS)
                                        {
                                                if (ginDebug == VS_TRUE)
                                                {
                                                        memset(szErrorMessage, 0x00, sizeof(szErrorMessage));
                                                        sprintf(szErrorMessage, "inSendField = %d Error!", srISOFunc.srUnPackISO[inRecvField].inFieldNum);
                                                        inLogPrintf(AT, szErrorMessage);
                                                }

                                                return (_TRAN_RESULT_UNPACK_ERR_);
                                        }
                                }

                                inSendCnt += inNCCC_DCC_GetFieldLen(i, &uszSendBuf[inSendCnt], srISOFunc.srISOFieldType);
                        }

                        while (i > srISOFunc.srUnPackISO[inRecvField].inFieldNum) /* 只有收 */
                        {
                                inRecvField++;
                        }

                        if (i == srISOFunc.srUnPackISO[inRecvField].inFieldNum)
                        {
                                /* 要先檢查 UnpackISO 是否存在 */
                                if (inNCCC_DCC_CheckUnPackField(i, srISOFunc.srUnPackISO) == VS_SUCCESS)
				{
                                        inRetVal = srISOFunc.srUnPackISO[inRecvField].inISOLoad(pobTran, &uszRecvBuf[inRecvCnt]);
					if (inRetVal != VS_SUCCESS)
					{
						if (ginDebug == VS_TRUE)
						{
							memset(szErrorMessage, 0x00, sizeof(szErrorMessage));
							sprintf(szErrorMessage, "inRecvField = %d Error!", srISOFunc.srUnPackISO[inRecvField].inFieldNum);
							inLogPrintf(AT, szErrorMessage);
						}

						return (_TRAN_RESULT_UNPACK_ERR_);
					}
				}
                        }
			
                        inCnt = inNCCC_DCC_GetFieldLen(i, &uszRecvBuf[inRecvCnt], srISOFunc.srISOFieldType);
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

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_DCC_UnPackISO() END!");

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_DCC_GetSTAN
Date&Time       :2016/9/2 上午 10:13
Describe        :Get STAN
*/
int inNCCC_DCC_GetSTAN(TRANSACTION_OBJECT *pobTran)
{
        char 	szSTANNum[12 + 1];

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_DCC_GetSTAN() START!");

        memset(szSTANNum, 0x00, sizeof(szSTANNum));
        if (inGetSTANNum(szSTANNum) == VS_ERROR)
                return (VS_ERROR);

        pobTran->srBRec.lnSTANNum = atol(szSTANNum);

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_DCC_SetSTAN
Date&Time       :2016/9/2 上午 10:13
Describe        :STAN++
*/
int inNCCC_DCC_SetSTAN(TRANSACTION_OBJECT *pobTran)
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
Function        :inNCCC_DCC_ProcessReversal
Date&Time       :2016/9/13 下午 4:49
Describe        :
*/
int inNCCC_DCC_ProcessReversal(TRANSACTION_OBJECT *pobTran)
{
        int 	inRetVal;
        char 	szSendReversalBit[2 + 1];

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_DCC_ProcessReversal() START!");

        memset(szSendReversalBit, 0x00, sizeof(szSendReversalBit));
        if (inGetSendReversalBit(szSendReversalBit) == VS_ERROR)
                return (VS_ERROR);

        if (!memcmp(szSendReversalBit, "Y", 1))
        {
                if ((inRetVal = inNCCC_DCC_ReversalSendRecvPacket(pobTran)) != VS_SUCCESS)
                {
                        return (VS_ERROR);
                }

                if (inNCCC_DCC_SetReversalCnt(pobTran, _ADD_) == VS_ERROR)
                {
                        return (VS_ERROR);
                }
        }
	
        /* 檢查是否要產生 Reversal */
        if (pobTran->uszReversalBit == VS_TRUE)
        {
                if ((inRetVal = inNCCC_DCC_ReversalSave_Flow(pobTran)) != VS_SUCCESS)
                {
                        return (VS_ERROR);
                }
        }
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_DCC_ProcessReversal() END!");
	
        return (VS_SUCCESS);
}

/*
Function        :inNCCC_DCC_AdviceSendRecvPacket
Date&Time       :2016/9/14 上午 9:43
Describe        :
*/
int inNCCC_DCC_AdviceSendRecvPacket(TRANSACTION_OBJECT *pobTran, int inAdvCnt)
{
        int				inCnt, inRetVal = VS_ERROR, inISOFuncIndex = -1;
	char				szDebugMsg[100 + 1];
        unsigned char			uszTCUpload = 0;
        TRANSACTION_OBJECT		ADVpobTran;
        ISO_TYPE_NCCC_DCC_TABLE 	srISOFunc;

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_DCC_AdviceSendRecvPacket() START!");
        
        /* 加密模式，預設值 = 0。【0 = 不加密，1 = tSAM 加密，2 = 軟體加密】 */
        inISOFuncIndex = 0; /* 不加密 */
        memset((char *) &srISOFunc, 0x00, sizeof(srISOFunc));
        memcpy((char *) &srISOFunc, (char *) &srNCCC_DCC_ISOFunc[inISOFuncIndex], sizeof(srISOFunc));

        memset((char *) &ADVpobTran, 0x00, sizeof(TRANSACTION_OBJECT));
        memcpy((char *) &ADVpobTran, (char *) pobTran, sizeof(TRANSACTION_OBJECT));

	/* 若批次第一筆TCupload會因為還沒存batch檔而失敗，所以TCUpload不做開啟動作 */
	if (ADVpobTran.uszTCUploadBit != VS_TRUE)
	{
		/* 開啟【.bat】【.bkey】【.adv】三個檔 */
		if (inBATCH_AdviceHandleReadOnly_By_Sqlite(pobTran) != VS_SUCCESS)
			return (VS_NO_RECORD);
	}

        /* 這裡的 for () 不可以在裡面直接call Return () ....  */
        for (inCnt = 0; inCnt < inAdvCnt; inCnt++)
        {
                if (ADVpobTran.uszTCUploadBit == VS_FALSE)
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
		
		/* 當筆TC */
		if (ADVpobTran.uszTCUploadBit == VS_TRUE	&&
		    ADVpobTran.srBRec.inChipStatus == _EMV_CARD_)
		{
			ADVpobTran.inISOTxnCode = _TC_UPLOAD_; 
		}
		/* 非當筆TC 或 取消*/
		else
		{
			/* EMV CARD 但未送TCUpload*/
			if (ADVpobTran.srBRec.inChipStatus == _EMV_CARD_ && ADVpobTran.srBRec.uszTCUploadBit != VS_TRUE)
			{
				/* TCUpload 未上傳，先送TCUpload */
				ADVpobTran.inISOTxnCode = _TC_UPLOAD_; /* 不是當筆 */
			}
			else if (ADVpobTran.srBRec.inCode == _VOID_)
			{
				ADVpobTran.inISOTxnCode = _VOID_; 
	
			}
			else
			{
				/* DCC advice 只有TC Upload 和 VOID */
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "ADVICE logic error");
					inLogPrintf(AT, szDebugMsg);
				}
			}
		
		}

                if (inRetVal == VS_SUCCESS)
                        inRetVal = inNCCC_DCC_SendPackRecvUnPack(&ADVpobTran);

                if (inRetVal == VS_SUCCESS)
                        if (srISOFunc.inAdviceAnalyse != NULL)
                                inRetVal = srISOFunc.inAdviceAnalyse(&ADVpobTran, &uszTCUpload);

                if (inRetVal != VS_SUCCESS)
                        break;
		
        }

        /* 關閉【.bat】【.bket】【.adv】三個檔 */
	/* 若批次第一筆TCupload會因為還沒存batch檔而失敗，所以TCUpload不做關閉動作 */
	if (ADVpobTran.uszTCUploadBit != VS_TRUE)
	{
		inBATCH_GlobalAdviceHandleClose_By_Sqlite();
	}

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_DCC_AdviceSendRecvPacket() END!");
        
        return (inRetVal);
}

/*
Function        :inNCCC_DCC_Advice_ESC_SendRecvPacket
Date&Time       :2017/3/29 上午 11:05
Describe        :
*/
int inNCCC_DCC_Advice_ESC_SendRecvPacket(TRANSACTION_OBJECT *pobTran, int inAdvCnt)
{
        int				inCnt, inRetVal = VS_ERROR, inISOFuncIndex = -1;
        TRANSACTION_OBJECT		ADVpobTran;
        ISO_TYPE_NCCC_DCC_TABLE 	srISOFunc;

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_DCC_Advice_ESC_SendRecvPacket() START!");
        
        /* 加密模式，預設值 = 0。【0 = 不加密，1 = tSAM 加密，2 = 軟體加密】 */
        inISOFuncIndex = 0; /* 不加密 */
        memset((char *) &srISOFunc, 0x00, sizeof(srISOFunc));
        memcpy((char *) &srISOFunc, (char *) &srNCCC_DCC_ISOFunc[inISOFuncIndex], sizeof(srISOFunc));

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
			inRetVal = inNCCC_DCC_SendPackRecvUnPack(&ADVpobTran);

		/* 不回寫到batch */
		if (inRetVal == VS_ERROR || inRetVal == VS_ISO_PACK_ERR || inRetVal == VS_ISO_UNPACK_ERROR)
			break;
		else
		{
			/* 【Field_39】 */
			if (inNCCC_DCC_CheckRespCode(&ADVpobTran) != _TRAN_RESULT_AUTHORIZED_)
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
                inLogPrintf(AT, "inNCCC_DCC_Advice_ESC_SendRecvPacket() END!");
        
        return (inRetVal);
}

/*
Function        :inNCCC_DCC_ProcessOnline
Date&Time       :2016/9/14 上午 9:36
Describe        :
*/
int inNCCC_DCC_ProcessOnline(TRANSACTION_OBJECT *pobTran)
{
        int 	inRetVal;
        char    szTemplate[512 + 1];
	char	szCustomIndicator[3 + 1] = {0};

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_DCC_ProcessOnline() START!");
	
	memset(szCustomIndicator, 0x00, sizeof(szCustomIndicator));
	inGetCustomIndicator(szCustomIndicator);

        /* 開始組交易封包，送、收、組、解 */
        pobTran->inISOTxnCode = pobTran->inTransactionCode; /* 以 srEventMenuItem.inCode Index */
        inRetVal = inNCCC_DCC_SendPackRecvUnPack(pobTran);

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
			inNCCC_DCC_ProcessReversal(pobTran);
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
                pobTran->inTransactionResult = inNCCC_DCC_CheckRespCode(pobTran); /* 【Field_39】 */
                
                if (ginDebug == VS_TRUE)
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "pobTran->inTransactionResult = %d", pobTran->inTransactionResult);
                        inLogPrintf(AT, szTemplate);
                }
                
                if (pobTran->inTransactionResult == _TRAN_RESULT_AUTHORIZED_)
                {
                        if (inNCCC_DCC_CheckAuthCode(pobTran) != VS_SUCCESS)
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
                inLogPrintf(AT, "inNCCC_DCC_ProcessOnline() END!");

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_DCC_ProcessOffline
Date&Time       :2016/9/14 上午 10:02
Describe        :
*/
int inNCCC_DCC_ProcessOffline(TRANSACTION_OBJECT *pobTran)
{
        int 	inRetVal = VS_SUCCESS;

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

/*
Function        :inNCCC_DCC_ProcessAdvice
Date&Time       :2016/9/13 下午 5:13
Describe        :
*/
int inNCCC_DCC_ProcessAdvice(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;
        int 	inSendAdviceCnt;

        if ((inSendAdviceCnt = inADVICE_GetTotalCount(pobTran)) == VS_NO_RECORD)
        {
                return (VS_SUCCESS);
        }
        else
        {
		inRetVal = inNCCC_DCC_AdviceSendRecvPacket(pobTran, inSendAdviceCnt);
                if (inRetVal != VS_SUCCESS)
                {
                        return (inRetVal);
                }
        }

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_DCC_ProcessAdvice_ESC
Date&Time       :2017/3/29 上午 10:59
Describe        :處理ESC advice
*/
int inNCCC_DCC_ProcessAdvice_ESC(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;
        int 	inSendAdviceCnt;

        if ((inSendAdviceCnt = inADVICE_ESC_GetTotalCount_Flow(pobTran)) == VS_NO_RECORD)
        {
                return (VS_SUCCESS);
        }
        else
        {
		inRetVal = inNCCC_DCC_Advice_ESC_SendRecvPacket(pobTran, inSendAdviceCnt);
                if (inRetVal != VS_SUCCESS)
                {
                        return (inRetVal);
                }
        }

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_DCC_ReversalSendRecvPacket
Date&Time       :2016/9/13 下午 4:56
Describe        :
*/
int inNCCC_DCC_ReversalSendRecvPacket(TRANSACTION_OBJECT *pobTran)
{
        int 		inRetVal, inSendCnt;
        long 		lnREVCnt;
	char		szDialBackupEnable[2 + 1];
	char		szCommMode[2 + 1];
        unsigned char 	uszFileName[20 + 1];
        unsigned char 	uszSendPacket[_NCCC_DCC_ISO_SEND_ + 1], uszRecvPacket[_NCCC_DCC_ISO_RECV_ + 1];
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
                inLogPrintf(AT, "inNCCC_DCC_CommSendRecvToHost() Before");

        if ((inRetVal = inNCCC_DCC_CommSendRecvToHost(pobTran, uszSendPacket, inSendCnt, uszRecvPacket)) != VS_SUCCESS)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inNCCC_DCC_CommSendRecvToHost() Error");

                return (VS_COMM_ERROR);
        }

        /* 解 ISO 電文 */
	pobTran->inISOTxnCode = _REVERSAL_;
        if ((inRetVal = inNCCC_DCC_UnPackISO(pobTran, uszSendPacket, uszRecvPacket)) != VS_SUCCESS)
        {
		pobTran->inTransactionResult = _TRAN_RESULT_UNPACK_ERR_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_ISO_UNPACK_;
		
                return (VS_ISO_UNPACK_ERROR);
        }

        /* memcmp兩字串相同回傳0 */
        if (memcmp(pobTran->srBRec.szRespCode, "00", 2) != 0)
        {	pobTran->inTransactionResult = _TRAN_RESULT_CANCELLED_;
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
Function        :inNCCC_DCC_AnalysePacket
Date&Time       :2016/9/14 上午 10:03
Describe        :
*/
int inNCCC_DCC_AnalysePacket(TRANSACTION_OBJECT *pobTran)
{
        int				inRetVal = VS_SUCCESS, inISOFuncIndex = -1;
        ISO_TYPE_NCCC_DCC_TABLE 	srISOFunc;

        if (pobTran->srBRec.uszOfflineBit == VS_FALSE)
        {
		/* 加密模式，預設值 = 0。【0 = 不加密，1 = tSAM 加密，2 = 軟體加密】 */
		inISOFuncIndex = 0; /* 不加密 */
		memset((char *) &srISOFunc, 0x00, sizeof(srISOFunc));
		memcpy((char *) &srISOFunc, (char *) &srNCCC_DCC_ISOFunc[inISOFuncIndex], sizeof(srISOFunc));

		if (srISOFunc.inOnAnalyse != NULL)
			inRetVal = srISOFunc.inOnAnalyse(pobTran);
		else
			inRetVal = VS_ERROR;
        }

        return (inRetVal);
}

/*
Function        :inNCCC_DCC_ReversalSave_Flow
Date&Time       :2017/3/30 上午 11:06
Describe        :
*/
int inNCCC_DCC_ReversalSave_Flow(TRANSACTION_OBJECT *pobTran)
{
	int 	inRetVal;
	char	szDialBackupEnable[2 + 1];
	
	inRetVal = inNCCC_DCC_ReversalSave(pobTran);
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	memset(szDialBackupEnable, 0x00, sizeof(szDialBackupEnable));
	inGetDialBackupEnable(szDialBackupEnable);

	if (memcmp(szDialBackupEnable, "Y", strlen("Y")) == 0)
	{

		inRetVal = inNCCC_DCC_ReversalSave_For_DialBeckUp(pobTran);
		if (inRetVal != VS_SUCCESS)
		{
			return (VS_ERROR);
		}

	}
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_DCC_ReversalSave
Date&Time       :2016/9/13 下午 5:06
Describe        :
*/
int inNCCC_DCC_ReversalSave(TRANSACTION_OBJECT *pobTran)
{
        int 		inRetVal, inPacketCnt;
        unsigned char 	uszReversalPacket[_NCCC_DCC_ISO_SEND_ + 1];
        unsigned char 	uszFileName[20 + 1];
        unsigned long 	srFHandle;


        memset(uszReversalPacket, 0x00, sizeof(uszReversalPacket));
        pobTran->inISOTxnCode = _REVERSAL_; /* 沖銷 */

        /* 組_REVERSAL_ 封包 */
        inPacketCnt = inNCCC_DCC_PackISO(pobTran, &uszReversalPacket[0], pobTran->inISOTxnCode);
        if (inPacketCnt <= 0)
	{
		pobTran->inTransactionResult = _TRAN_RESULT_PACK_ERR_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_ISO_PACK_;
		
                return (VS_ERROR);
	}

        memset(uszFileName, 0x00, sizeof(uszFileName));
	if (inFunc_ComposeFileName(pobTran, (char*)uszFileName, _REVERSAL_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

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
Function        :inNCCC_DCC_ReversalSave_For_DialBeckUp
Date&Time       :2017/3/30 上午 10:24
Describe        :
*/
int inNCCC_DCC_ReversalSave_For_DialBeckUp(TRANSACTION_OBJECT *pobTran)
{
        int 		inRetVal, inPacketCnt;
        unsigned char 	uszReversalPacket[_NCCC_DCC_ISO_SEND_ + 1];
        unsigned char 	uszFileName[20 + 1];
        unsigned long 	srFHandle;


        memset(uszReversalPacket, 0x00, sizeof(uszReversalPacket));
        pobTran->inISOTxnCode = _REVERSAL_; /* 沖銷 */

        /* 組_REVERSAL_ 封包 */
        inPacketCnt = inNCCC_DCC_PackISO(pobTran, &uszReversalPacket[0], pobTran->inISOTxnCode);
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

/*
Function        :inNCCC_DCC_CommSendRecvToHost
Date&Time       :2016/9/2 下午 2:49
Describe        :決定送收和Timeout
*/
int inNCCC_DCC_CommSendRecvToHost(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, int inSendLen, unsigned char *uszRecvPacket)
{
        int		inRetVal;
        int		inSendTimeout = 10;
        int		inReceiveTimeout = 10;
        int		inReceiveSize = _COMM_RECEIVE_MAX_LENGTH_;		/* 配合Ingenico */
	char		szTimeOut[2 + 1] = {0};
        char		szHostResponseTimeOut[2 + 1] = {0};
	unsigned char	uszDispBit = VS_TRUE;

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_DCC_CommSendRecvToHost() START!");
        
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
		vdNCCC_DCC_ISO_FormatDebug_DISP(pobTran, &uszSendPacket[0], inSendLen);
	}
	/* 列印ISO Debug */
        if (ginISODebug == VS_TRUE)
	{
                vdNCCC_DCC_ISO_FormatDebug_PRINT(pobTran, &uszSendPacket[0], inSendLen);
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
		vdNCCC_DCC_ISO_FormatDebug_DISP(pobTran, &uszRecvPacket[0], inRetVal);
	}
        /* 列印ISO Debug */
        if (ginISODebug == VS_TRUE)
                vdNCCC_DCC_ISO_FormatDebug_PRINT(pobTran, &uszRecvPacket[0], inRetVal);

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_DCC_CommSendRecvToHost() END!");
        
        return (VS_SUCCESS);
}

/*
Function        :inNCCC_DCC_GetReversalCnt
Date&Time       :2016/9/13 下午 5:02
Describe        :
*/
int inNCCC_DCC_GetReversalCnt(TRANSACTION_OBJECT *pobTran)
{
        int 	inCnt;
        char 	szReversalCnt[6 + 1];

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_DCC_GetReversalCnt() START!");

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
Function        :inNCCC_DCC_SetReversalCnt
Date&Time       :2016/9/13 下午 5:02
Describe        :
*/
int inNCCC_DCC_SetReversalCnt(TRANSACTION_OBJECT *pobTran, long lnSetMode)
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
Function        :inNCCC_DCC_GetMustSettleBit
Date&Time       :2016/9/14 上午 10:12
Describe        :
*/
int inNCCC_DCC_GetMustSettleBit(TRANSACTION_OBJECT *pobTran, char *szMustSettleBit)
{
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_DCC_GetMustSettleBit() START!");

	/* 刷完卡就LoadHDPT，不另在Load */
        memset(szMustSettleBit, 0x00, sizeof(szMustSettleBit));
        if (inGetMustSettleBit(szMustSettleBit) == VS_ERROR)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inNCCC_DCC_GetMustSettleBit ERROR!");
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_DCC_SetMustSettleBit
Date&Time       :2016/9/14 上午 10:12
Describe        :
*/
int inNCCC_DCC_SetMustSettleBit(TRANSACTION_OBJECT *pobTran, char *szMustSettleBit)
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
Function        :inNCCC_DCC_SendPackRecvUnPack
Date&Time       :2016/9/2 上午 10:30
Describe        :組收送解
 */
int inNCCC_DCC_SendPackRecvUnPack(TRANSACTION_OBJECT *pobTran)
{
        int 		inSendCnt = 0, inRecvCnt = 0, inRetVal;
        unsigned char 	uszSendPacket[_NCCC_DCC_ISO_SEND_ + 1], uszRecvPacket[_NCCC_DCC_ISO_SEND_ + 1];

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_DCC_SendPackRecvUnPack() START!");

        memset(uszSendPacket, 0x00, sizeof(uszSendPacket));
        memset(uszRecvPacket, 0x00, sizeof(uszRecvPacket));

        /* 組 ISO 電文 */
        if ((inSendCnt = inNCCC_DCC_PackISO(pobTran, uszSendPacket, pobTran->inISOTxnCode)) <= 0)
        {
		pobTran->inTransactionResult = _TRAN_RESULT_PACK_ERR_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_ISO_PACK_;
		
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inNCCC_DCC_PackISO() Error!");

                return (VS_ISO_PACK_ERR); /* 組交易電文錯誤，不用組_REVERSAL_ */
        }
        
        /* 傳送及接收 ISO 電文 */
        if ((inRecvCnt = inNCCC_DCC_CommSendRecvToHost(pobTran, uszSendPacket, inSendCnt, uszRecvPacket)) != VS_SUCCESS)
        {
		pobTran->inTransactionResult = _TRAN_RESULT_COMM_ERROR_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_COMM_;
		
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inNCCC_DCC_CommSendRecvToHost() Error");

                if (pobTran->inISOTxnCode != _ADVICE_)
                        memset(pobTran->srBRec.szRespCode, 0x00, sizeof(pobTran->srBRec.szRespCode));

                return (VS_COMM_ERROR);
        }

        /* 解 ISO 電文 */
        inRetVal = inNCCC_DCC_UnPackISO(pobTran, uszSendPacket, uszRecvPacket);
        
        if (inRetVal != VS_SUCCESS)
        {
		pobTran->inTransactionResult = _TRAN_RESULT_UNPACK_ERR_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_ISO_UNPACK_;

                inRetVal = VS_ISO_UNPACK_ERROR;
        }
	
        return (inRetVal);
}

/*
Function        :inNCCC_DCC_CheckRespCode
Date&Time       :2016/9/14 上午 9:48
Describe        :
*/
int inNCCC_DCC_CheckRespCode(TRANSACTION_OBJECT *pobTran)
{
	int 	inRetVal = _TRAN_RESULT_COMM_ERROR_;
	
	/* DCC端末規格書 14.Response Code = “00”或”11”皆視為授權*/
	if (!memcmp(pobTran->srBRec.szRespCode, "00", 2) || !memcmp(pobTran->srBRec.szRespCode, "00", 2))
	{
			inRetVal = _TRAN_RESULT_AUTHORIZED_;
			pobTran->inErrorMsg = _ERROR_CODE_V3_NONE_;
	}
	else if (!memcmp(pobTran->srBRec.szRespCode, "01", 2) ||
		 !memcmp(pobTran->srBRec.szRespCode, "02", 2))
	{
		memset(pobTran->srBRec.szRespCode, 0x00, sizeof(pobTran->srBRec.szRespCode));
		strcpy(pobTran->srBRec.szRespCode, "05");
		inRetVal = _TRAN_RESULT_CANCELLED_;
	}
	/* DCC交易若有收到主機回覆之電文，只顯示授權及”拒絕交易XX”兩種結果 */
        else if (!memcmp(pobTran->srBRec.szRespCode, "  ", 2))
	{
                inRetVal = _TRAN_RESULT_COMM_ERROR_;		/* 不是定義的 Response Code */
		pobTran->inErrorMsg = _ERROR_CODE_V3_COMM_;
	}
        else if (pobTran->srBRec.szRespCode[0] == 0x00)
	{
                inRetVal = _TRAN_RESULT_COMM_ERROR_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_COMM_;
	}
	else if (!memcmp(&pobTran->srBRec.szRespCode[0], "0X", 2) ||
		 !memcmp(&pobTran->srBRec.szRespCode[0], "A0", 2) ||
	         !memcmp(&pobTran->srBRec.szRespCode[0], "XA", 2) ||
	         !memcmp(&pobTran->srBRec.szRespCode[0], "XB", 2) ||
	         !memcmp(&pobTran->srBRec.szRespCode[0], "XC", 2) ||
	         !memcmp(&pobTran->srBRec.szRespCode[0], "XN", 2) ||
	         !memcmp(&pobTran->srBRec.szRespCode[0], "XX", 2) ||
	         !memcmp(&pobTran->srBRec.szRespCode[0], "XY", 2) ||
	         !memcmp(&pobTran->srBRec.szRespCode[0], "XZ", 2))
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
		/* DCC交易若有收到主機回覆之電文，只顯示授權及”拒絕交易XX”兩種結果 */
                else
		{
                        inRetVal = _TRAN_RESULT_UNPACK_ERR_;	/* 不是定義的 Response Code */
			pobTran->inErrorMsg = _ERROR_CODE_V3_ISO_UNPACK_;
		}
        }

        return (inRetVal);
}

/*
Function        :inNCCC_DCC_CheckAuthCode
Date&Time       :2016/9/14 上午 9:48
Describe        :
*/
int inNCCC_DCC_CheckAuthCode(TRANSACTION_OBJECT *pobTran)
{
        int 	inRetVal = VS_SUCCESS;

        switch (pobTran->inISOTxnCode)
        {
                case _SETTLE_:
                case _BATCH_UPLOAD_:
                case _CLS_BATCH_:
                        break; /* 不檢核 */
                default:
                        if (!memcmp(&pobTran->srBRec.szAuthCode[0], "000000", 6) || !memcmp(&pobTran->srBRec.szAuthCode[0], "      ", 6))
                                inRetVal = VS_ERROR;

                        break;
        }

        return (inRetVal);
}

/*
Function        :inNCCC_DCC_OnlineAnalyseMagneticManual
Date&Time       :2016/9/14 上午 9:49
Describe        :
*/
int inNCCC_DCC_OnlineAnalyseMagneticManual(TRANSACTION_OBJECT *pobTran)
{
        int 	inRetVal = VS_SUCCESS;

        if (pobTran->inISOTxnCode == _SETTLE_)
        {
		/* Settle時非95、非00的話 */
                if (memcmp(pobTran->srBRec.szRespCode, "95", 2) && memcmp(pobTran->srBRec.szRespCode, "00", 2))
                        inRetVal = VS_ERROR;
                else
                {
			/* 需要執行BatchUpload時 */
                        if (pobTran->inTransactionResult == _TRAN_RESULT_SETTLE_UPLOAD_BATCH_ && !memcmp(pobTran->srBRec.szRespCode, "95", 2))
                                inRetVal = inNCCC_DCC_ProcessSettleBatchUpload(pobTran);
			
                }

                if (inRetVal == VS_SUCCESS)
                {
                        if (inNCCC_DCC_SetReversalCnt(pobTran, _RESET_) == VS_ERROR)
                        {
                                return (VS_ERROR);
                        }
			
			/* 結帳成功 把請先結帳的bit關掉 */
			inNCCC_DCC_SetMustSettleBit(pobTran, "N");
                }
		else
		{
			/* BatchUpload失敗 */
			/* 在inNCCC_DCC_ProcessSettleBatchUpload裡面顯示主機回的錯誤訊息 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_SETTLE_NOT_SUCCESS_;
			inRetVal = VS_ERROR;
		}

        }
        else
        {
                if (pobTran->inTransactionResult == _TRAN_RESULT_AUTHORIZED_)
                {
                        /* 要更新端末機的日期及時間 */
                        if (inNCCC_DCC_SyncHostTerminalDateTime(pobTran) != VS_SUCCESS)
                                return (VS_ERROR);

                        if (pobTran->srBRec.uszOfflineBit == VS_FALSE)
                        {
                                inSetSendReversalBit("N");
                                if (inSaveHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
                                {
                                        return (VS_ERROR);
                                }

                                /* 因為是【Online】交易在這裡送【Advice】 */
                                inRetVal = inNCCC_DCC_AdviceSendRecvPacket(pobTran, 1);
				if (inRetVal == VS_NO_RECORD)
				{
					inNCCC_DCC_Advice_ESC_SendRecvPacket(pobTran, 1);
				}
				
				/* 不管advice最後如何，只要原交易授權就回傳成功 */
				inRetVal = VS_SUCCESS;
                        }
			
                }
                else if (pobTran->inTransactionResult == _TRAN_RESULT_REFERRAL_ && pobTran->inTransactionCode == _SALE_)
                {
			/* DCC沒有Call Bank */
			inNCCC_DCC_DispHostResponseCode(pobTran);	/* 顯示主機回的錯誤訊息 */
			inRetVal = VS_ERROR;
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

			inNCCC_DCC_DispHostResponseCode(pobTran);	/* 顯示主機回的錯誤訊息 */
			inRetVal = VS_ERROR;
                }
                else
                {
			inNCCC_DCC_DispHostResponseCode(pobTran);	/* 顯示主機回的錯誤訊息 */
                        inRetVal = VS_ERROR;
                }
        }

        return (inRetVal);
}

/*
Function        :inNCCC_DCC_OnlineAnalyseEMV
Date&Time       :2016/11/17 下午 4:57
Describe        :分析結果
*/
int inNCCC_DCC_OnlineAnalyseEMV(TRANSACTION_OBJECT *pobTran)
{
        char            szCustomerIndicator[3 + 1] = {0};
	int		inRetVal = VS_SUCCESS;
	
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
                        EMVGlobConfig.uszAction = d_ONLINE_ACTION_UNABLE;
                        inRetVal = VS_ERROR;
                }
	}
	else if (pobTran->inTransactionResult == _TRAN_RESULT_REFERRAL_)
	{
		/* 不應該有Call Bank 電文錯誤 */
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, _ERR_ISO_);
		srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
		srDispMsgObj.inMsgType = _NO_KEY_MSG_;
		srDispMsgObj.inTimeout = 2;
		strcpy(srDispMsgObj.szErrMsg1, "");
		srDispMsgObj.inErrMsg1Line = 0;
		inDISP_Msg_BMP(&srDispMsgObj);		/* 電文錯誤 */
		
		/* 將主機下來的回覆資料依照Kernel的要求塞入指定的參數中 */
		EMVGlobConfig.uszAction = d_ONLINE_ACTION_DECLINE;
		
		inRetVal = VS_ERROR;
	}
	/* 主機授權 */
	else if (pobTran->inTransactionResult == _TRAN_RESULT_AUTHORIZED_)
	{
		/* 要更新端末機的日期及時間
		20. 為避免端末機系統時間因 Base24 及 DCC FES 的時間差而互相影響，故
		    DCC 交易一律不更新端末機系統時間，DCC FES 電文所回覆的日期及時間
		    僅供紀錄交易 Log 及列印帳單資訊用。
		*/
		
		/* 主機回傳結果 */
		EMVGlobConfig.uszAction = d_ONLINE_ACTION_APPROVAL;
		
		inRetVal = VS_SUCCESS;
	}
	/* 主機拒絕 */
	else
	{
		/* 主機回傳結果 */
		EMVGlobConfig.uszAction = d_ONLINE_ACTION_DECLINE;
		
		if (pobTran->inTransactionResult == _TRAN_RESULT_CANCELLED_)
		{
			inNCCC_DCC_DispHostResponseCode(pobTran);	/* 顯示主機回的錯誤訊息 */

			inRetVal = VS_ERROR;
		}
		else
		{
			inNCCC_DCC_DispHostResponseCode(pobTran);	/* 顯示主機回的錯誤訊息 */

			inRetVal = VS_ERROR;
		}
	}
		
	return (inRetVal);
}

/*
Function        :inNCCC_DCC_OnlineEMV_Complete
Date&Time       :2016/11/17 下午 5:47
Describe        :處理Second Gen AC之後TC or Reversal
*/
int inNCCC_DCC_OnlineEMV_Complete(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
        char		szDebugMsg[100 + 1];
	char            szCustomerIndicator[3 + 1] = {0};
	unsigned short	usTagLen;
	unsigned char 	uszValue[128 + 1];
	
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
		
		inNCCC_DCC_DispHostResponseCode(pobTran);
		
		return (VS_ERROR);
	}
	else if (!memcmp(uszValue, "Y1", 2))
	{
		/* 防呆，DCC必定Online，不可有Y1 */
		if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "EMV First Gen AC產生Y1");
		
		/* Y1= approved offline (first Gen AC , CID=TC) */
		/* 拒絕交易 */
		inNCCC_DCC_DispHostResponseCode(pobTran);
		
		/* 紀錄已送到主機的調閱編號 */
		inNCCC_Func_Update_Memory_Invoice(pobTran);
		
		return (VS_SUCCESS);
	}
	else
	{
		/* 開始執行 Second Generate AC */
		inRetVal = inEMV_SecondGenerateAC(pobTran);

		if (ginDebug == VS_TRUE)
		{
			sprintf(szDebugMsg, "inEMVAPISecondGenerateAC = %d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}

		/* 將EMV參數更新存檔 */
		if (inNCCC_FuncEMVPrepareBatch(pobTran) != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
				inLogPrintf(AT, "inNCCC_FuncEMVPrepareBatch Error!");

			inFunc_EDCLock(AT);

			return (VS_ERROR);
		}

		/* Second Gen AC Approve */
		if (inRetVal == VS_SUCCESS)
		{
			/* DCC不會有Call Bank */
			if (pobTran->inTransactionResult == _TRAN_RESULT_AUTHORIZED_)
			{
				/* 【DCC】交易不會有【Y1】【Y3】 */
				/* 重組【REVERSAL】 */
				if (!memcmp(&pobTran->srBRec.szAuthCode[0], "Y3", 2))
				{
					if (pobTran->uszCus099_Already_Decide_Reversal_Bit == VS_TRUE)
					{
						/* 已送過reversal，不必再組 */
					}
					else
					{
						if (inNCCC_DCC_ReversalSave_Flow(pobTran) != VS_SUCCESS)
						{
							if (ginDebug == VS_TRUE)
								inLogPrintf(AT, "inNCCC_DCC_ReversalSave() Error!");

							inFunc_EDCLock(AT);

							return (VS_ERROR);
						}
					}

                                        /* 紅利、分期交易也支援「OL 拒絕交易 */
					memset(pobTran->srBRec.szRespCode, 0x00, sizeof(pobTran->srBRec.szRespCode));
					strcpy(pobTran->srBRec.szRespCode, "OL");
					inNCCC_DCC_DispHostResponseCode(pobTran);			/* 顯示主機回的錯誤訊息 */
					
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

					return (VS_ERROR);
				}
				else
				{
					inSetSendReversalBit("N");
					if (inSaveHDPTRec(pobTran->srBRec.inHDTIndex) != VS_SUCCESS)
						return (VS_ERROR);
				}

				/* 在這裡送【TC UPLOAD】 */
				pobTran->uszTCUploadBit = VS_TRUE;
				pobTran->srBRec.uszTCUploadBit = VS_FALSE;
				/* TC_UPLOAD 要加一 */
				inNCCC_DCC_GetSTAN(pobTran);
				
				inRetVal = inFLOW_RunFunction(pobTran, _COMM_START_);
				inNCCC_DCC_SetSTAN(pobTran);
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
					if ((inRetVal = inNCCC_DCC_AdviceSendRecvPacket(pobTran, 1)) != VS_SUCCESS)
					{
						if (ginDebug == VS_TRUE)
							inLogPrintf(AT, "inNCCC_DCC_AdviceSendRecvPacket() Error!");

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
							inLogPrintf(AT, "inNCCC_DCC_AdviceSendRecvPacket() SUCCESS!");

						/* 這裡是For當筆，因為inNCCC_DCC_ISOAdviceAnalyse的pobTran是ADVPobTran，所以TRT的update Batch不會存到在那裡的改動 */
						/* 在這裡把bit On起來，避免TC重送 */
						pobTran->srBRec.uszTCUploadBit = VS_TRUE;
						
						/* 紀錄已送到主機的調閱編號 */
						inNCCC_Func_Update_Memory_Invoice(pobTran);

						return (VS_SUCCESS);
					}
				}

			}
			/* 主機拒絕 */
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
					if (inNCCC_DCC_ReversalSave_Flow(pobTran) != VS_SUCCESS)
					{
						if (ginDebug == VS_TRUE)
							inLogPrintf(AT, "inNCCC_DCC_ReversalSave() Error!");

						inFunc_EDCLock(AT);

						return (VS_ERROR);
					}
				}

				/* 拒絕交易 */
				inNCCC_DCC_DispHostResponseCode(pobTran);			/* 顯示主機回的錯誤訊息 */
				
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
				inSaveHDPTRec(pobTran->srBRec.inHDTIndex);

//				/* 主機有回拒絕，不顯示晶片錯誤碼 */
//				inNCCC_DCC_DispHostResponseCode(pobTran);			/* 顯示主機回的錯誤訊息 */
			}

			return (VS_ERROR);
		}/* Second Gen AC END */
			
	}/* First Gen AC END */
	
}

/*
Function        :inNCCC_DCC_ProcessSettleBatchUpload
Date&Time       :2016/9/14 上午 10:08
Describe        :
*/
int inNCCC_DCC_ProcessSettleBatchUpload(TRANSACTION_OBJECT *pobTran)
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
			inNCCC_DCC_GetSTAN(pobTran);

			inRetVal = inNCCC_DCC_SendPackRecvUnPack(pobTran);
			/* 成功或失敗 System Trace Number 都要加一 */
			inNCCC_DCC_SetSTAN(pobTran);
			
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
					inNCCC_DCC_DispHostResponseCode(pobTran);	/* 顯示主機回的錯誤訊息 */
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
	inNCCC_DCC_GetSTAN(pobTran);

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

		inRetVal = inNCCC_DCC_SendPackRecvUnPack(pobTran);
		/* 成功或失敗 System Trace Number 都要加一 */
		inNCCC_DCC_SetSTAN(pobTran);
			
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
Function        :inNCCC_DCC_DispHostResponseCode
Date&Time       :2016/11/15 下午 5:57
Describe        :顯示錯誤代碼
*/
int inNCCC_DCC_DispHostResponseCode(TRANSACTION_OBJECT *pobTran)
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
	
	if (!memcmp(pobTran->srBRec.szRespCode, "19", 2))
	{
		sprintf(szMsg, "%s", "請重新交易");					/* 請重新交易 */
	}
	else if (!memcmp(pobTran->srBRec.szRespCode, "25", 2) && pobTran->inTransactionCode == _PRE_COMP_)
	{
		/* 聯合更新規格 2011-04-01
		   預授權完成 DCCFES 查無原交易時，將 Response code 由 05 改為 25 ，
		   故端末機修改 Response code = 25 的顯示訊息為” 無原交易記錄25”。 */
		sprintf(szMsg, "%s", "無原交易紀錄");					/* 無原交易紀錄 */
	}
	else
	{
		/* 轉換回應訊息 */
		inNCCC_Func_ResponseCode_Transform(atoi(pobTran->srBRec.szRespCode), szMsg);
	}
		
	memset(szResponseCode, 0x00, sizeof(szResponseCode));	
	sprintf(szResponseCode, "%s", pobTran->srBRec.szRespCode);			/* 錯誤代碼 */
	
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
Function        :inNCCC_DCC_SyncHostTerminalDateTime
Date&Time       :2016/9/14 上午 10:14
Describe        :
*/
int inNCCC_DCC_SyncHostTerminalDateTime(TRANSACTION_OBJECT *pobTran)
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
	
	/* DCC交易一律不更新端末機系統時間，所以註解掉 */
//	inFunc_SetEDCDateTime(pobTran->srBRec.szDate, pobTran->srBRec.szTime);

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_DCC_ProcessReferral
Date&Time       :2016/9/14 上午 10:14
Describe        :
*/
int inNCCC_DCC_ProcessReferral(TRANSACTION_OBJECT *pobTran)
{
        /* 輸入授權碼 */
        if (inFunc_REFERRAL_GetManualApproval(pobTran) != VS_SUCCESS)
                return (VS_ERROR);

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_DCC_CHECK
Date&Time       :2016/8/15 上午 11:31
Describe        :用來確認是否繼續跑DCC流程
*/
int inNCCC_DCC_CHECK(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;
	char	szDCCSuport[1 + 1] = {0};
	char	szHostEnable[1 + 1] = {0};
	char	szTemplate[20 + 1] = {0};
	char	szDemoMode[2 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
	
        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_CHECK START!");
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_DCC_CHECK() START !");
	}
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		memset(szHostEnable, 0x00, sizeof(szHostEnable));
		/* DCC Host 沒開 不檢查是否做DCC */
		if (inNCCC_DCC_GetDCC_Enable(pobTran->srBRec.inHDTIndex, szHostEnable)!= VS_SUCCESS)
		{

			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "----------------------------------------");
				inLogPrintf(AT, "DCC CHEKC:NO DCC HOST");
				inLogPrintf(AT, "----------------------------------------");
			}

                        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_CHECK DCC CHEKC:NO DCC HOST END!");
			/* 找不到DCC 直接跳過 */
			return (VS_SUCCESS);
		}

		if (memcmp(szHostEnable, "Y", 1) != 0)
		{
			/* 找到DCC但是沒開 跳過 */
			pobTran->srBRec.uszDCCTransBit = VS_FALSE;

			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "----------------------------------------");
				inLogPrintf(AT, "DCC CHEKC:HDT DCC NOT OPEN");
				inLogPrintf(AT, "----------------------------------------");
			}

                        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_CHECK DCC CHEKC:HDT DCC NOT OPEN END!");
			return (VS_SUCCESS);
		}

		/* (需求單-111060)-DCC流程修改需求 by Russell 2022/4/11 下午 3:26 */
		/* 不參考DCC FlowVersion */

		/* 結帳不用檢核這些 */
		if (pobTran->inTransactionCode != _SETTLE_)
		{
			/* 沒有DCC舊參數，擋下來(當TMS排程下載(參數或AP)成功，但DCC參數下載失敗時，所有交易直接以台幣進行支付，連動結帳時DCC以0元進行結帳) */
			/* DCC 參數沒下也要可以結帳 */
			if (inNCCC_DCC_Already_Have_Parameter(pobTran) != VS_TRUE)
			{
				pobTran->srBRec.uszDCCTransBit = VS_FALSE;

				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "----------------------------------------");
					inLogPrintf(AT, "DCC CHEKC:NO DCC PARAMETER");
					inLogPrintf(AT, "----------------------------------------");
				}

                                vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_CHECK DCC CHEKC:NO DCC PARAMETER END!");
				return (VS_SUCCESS);
			}
		
			/* MPAS不支援DCC */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetNCCCFESMode(szTemplate);
			if (memcmp(szTemplate, _NCCC_04_MPAS_MODE_, strlen(_NCCC_04_MPAS_MODE_)) == 0)
			{
				pobTran->srBRec.uszDCCTransBit = VS_FALSE;

				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "----------------------------------------");
					inLogPrintf(AT, "DCC CHEKC:DCC NOT SUPPORT MPAS");
					inLogPrintf(AT, "----------------------------------------");
				}

                                vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_CHECK DCC CHEKC:DCC NOT SUPPORT MPAS END!");
				return (VS_SUCCESS);
			}

			/* 【需求單 - 106252】	DCC交易新增感應及BIN11碼需求 by Russell 2020/1/14 下午 3:37 */
			/* 改為有支援感應 */

			/* 不支援DCC+CUP */
			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				pobTran->srBRec.uszDCCTransBit = VS_FALSE;

				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "----------------------------------------");
					inLogPrintf(AT, "DCC CHEKC:DCC NOT SUPPORT CUP");
					inLogPrintf(AT, "----------------------------------------");
				}

                                vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_CHECK DCC CHEKC:DCC NOT SUPPORT CUP END!");
				return (VS_SUCCESS);
			}

			/* 不支援DCC + SaleOffline */
			if (pobTran->srBRec.inCode == _SALE_OFFLINE_)
			{
				pobTran->srBRec.uszDCCTransBit = VS_FALSE;

				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "----------------------------------------");
					inLogPrintf(AT, "DCC CHEKC:DCC NOT SUPPORT SALE-OFFLINE");
					inLogPrintf(AT, "----------------------------------------");
				}

                                vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_CHECK DCC CHEKC:DCC NOT SUPPORT SALE-OFFLINE END!");
				return (VS_SUCCESS);
			}

			/* 不支援DCC + PreAuth */
			if (pobTran->srBRec.inCode == _PRE_AUTH_)
			{
				pobTran->srBRec.uszDCCTransBit = VS_FALSE;

				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "----------------------------------------");
					inLogPrintf(AT, "DCC CHEKC:DCC NOT SUPPORT PRE-AUTH");
					inLogPrintf(AT, "----------------------------------------");
				}

                                vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_CHECK DCC CHEKC:DCC NOT SUPPORT PRE-AUTH END!");
				return (VS_SUCCESS);
			}

			/* 不支援分期 */
			if (pobTran->srBRec.uszInstallmentBit == VS_TRUE)
			{
				pobTran->srBRec.uszDCCTransBit = VS_FALSE;

				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "----------------------------------------");
					inLogPrintf(AT, "DCC CHEKC:DCC NOT SUPPORT INST");
					inLogPrintf(AT, "----------------------------------------");
				}

                                vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_CHECK DCC CHEKC:DCC NOT SUPPORT INST END!");
				return (VS_SUCCESS);
			}

			/* 不支援紅利 */
			if (pobTran->srBRec.uszRedeemBit == VS_TRUE)
			{
				pobTran->srBRec.uszDCCTransBit = VS_FALSE;

				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "----------------------------------------");
					inLogPrintf(AT, "DCC CHEKC:DCC NOT SUPPORT REDEEM");
					inLogPrintf(AT, "----------------------------------------");
				}

                                vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_CHECK DCC CHEKC:DCC NOT SUPPORT REDEEM END!");
				return (VS_SUCCESS);
			}
			
			/* (客製化-111134)配合Mastercard DCC交易流程修改為「先詢價再授權」開發EDC教育訓練版本DCC流程修改需求 */
			/* 因為DCC退貨不跑詢價或選幣別流程，所以加在這裡 */
			if (pobTran->srBRec.lnTxnAmount == 10000)
			{
				sprintf(pobTran->srBRec.szDCC_FCN, "392");
			}
			else if (pobTran->srBRec.lnTxnAmount == 10100)
			{
				sprintf(pobTran->srBRec.szDCC_FCN, "840");
			}
			else if (pobTran->srBRec.lnTxnAmount == 10200)
			{
				sprintf(pobTran->srBRec.szDCC_FCN, "344");
			}
			else if (pobTran->srBRec.lnTxnAmount == 10300)
			{
				sprintf(pobTran->srBRec.szDCC_FCN, "978");
			}
			else if (pobTran->srBRec.lnTxnAmount == 10400)
			{
				sprintf(pobTran->srBRec.szDCC_FCN, "410");
			}
			else if (pobTran->srBRec.lnTxnAmount == 10500)
			{
				sprintf(pobTran->srBRec.szDCC_FCN, "458");
			}
			else if (pobTran->srBRec.lnTxnAmount == 10600)
			{
				sprintf(pobTran->srBRec.szDCC_FCN, "702");
			}
			else if (pobTran->srBRec.lnTxnAmount == 10700)
			{
				sprintf(pobTran->srBRec.szDCC_FCN, "764");
			}
			else if (pobTran->srBRec.lnTxnAmount == 10800)
			{
				sprintf(pobTran->srBRec.szDCC_FCN, "036");
			}
			else if (pobTran->srBRec.lnTxnAmount == 10900)
			{
				sprintf(pobTran->srBRec.szDCC_FCN, "124");
			}
			else
			{
				pobTran->srBRec.uszDCCTransBit = VS_FALSE;

				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "----------------------------------------");
					inLogPrintf(AT, "DCC CHEKC:DCC NOT DEMO AMOUNT");
					inLogPrintf(AT, "----------------------------------------");
				}
                                
                                vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_CHECK DCC CHEKC:DCC NOT DEMO AMOUNT END!");
				return (VS_SUCCESS);
			}

			/* 是否為人工輸入卡號 */
			if (pobTran->srBRec.uszManualBit == VS_TRUE &&
			    pobTran->srBRec.uszRefundCTLSBit != VS_TRUE)
			{
				if (pobTran->srBRec.inCode == _REFUND_)
				{
					/* 退貨手動輸入都不擋 */
				}
				else
				{ 
					pobTran->srBRec.uszDCCTransBit = VS_FALSE;

					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "----------------------------------------");
						inLogPrintf(AT, "DCC CHEKC:DCC NOT SUPPORT MENU-KEYIN");
						inLogPrintf(AT, "----------------------------------------");
					}

                                        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_CHECK DCC CHEKC:DCC NOT SUPPORT MENU-KEYIN END!");
					return (VS_SUCCESS);
				}
			}

			/* 是否為Local Bin(查Local Bin Table) */
			/* DEMO版不檢查Cardbin */
			/* (EX: 若EMS設定只開DCC VISA卡，則預設所有VISA卡，都走DCC流程，其它卡別走一般信用卡流程。 */
			
			/* 判斷DCC是否接受 VISA或Master Card */
			/* 先判斷是哪一種卡 */
			if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_VISA_, strlen(_CARD_TYPE_VISA_)) == 0)
			{
				/* 判斷CFGT中參數是否支援 */
				memset(szDCCSuport, 0x00, sizeof(szDCCSuport));
				inGetSupDccVisa(szDCCSuport);
				if (memcmp(szDCCSuport, "N", 1) == 0)
				{
					pobTran->srBRec.uszDCCTransBit = VS_FALSE;

					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "----------------------------------------");
						inLogPrintf(AT, "DCC CHEKC:CFGT VISA NOT SUPPORT");
						inLogPrintf(AT, "----------------------------------------");
					}

                                        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_CHECK DCC CHEKC:CFGT VISA NOT SUPPORT END!");
					return (VS_SUCCESS);
				}
			}
			else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)) == 0)
			{
				memset(szDCCSuport, 0x00, sizeof(szDCCSuport));
				inGetSupDccMasterCard(szDCCSuport);
				if (memcmp(szDCCSuport, "N", 1) == 0)
				{
					pobTran->srBRec.uszDCCTransBit = VS_FALSE;

					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "----------------------------------------");
						inLogPrintf(AT, "DCC CHEKC:CFGT MASTER NOT SUPPORT");
						inLogPrintf(AT, "----------------------------------------");
					}

                                        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_CHECK DCC CHEKC:CFGT MASTER NOT SUPPORT END!");
					return (VS_SUCCESS);
				}
			}
			/* 非VISA、MASTERCARD 不支援*/
			else
			{
				pobTran->srBRec.uszDCCTransBit = VS_FALSE;

				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "----------------------------------------");
					inLogPrintf(AT, "DCC CHEKC:CARD NOT SUPPORT");
					inLogPrintf(AT, "----------------------------------------");
				}

                                vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_CHECK DCC CHEKC:CARD NOT SUPPORT END!");
				return (VS_SUCCESS);
			}
			
			/* 不支援Uny交易 */
			if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
			{
				pobTran->srBRec.uszDCCTransBit = VS_FALSE;

				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "----------------------------------------");
					inLogPrintf(AT, "DCC CHEKC:DCC NOT SUPPORT Uny");
					inLogPrintf(AT, "----------------------------------------");
				}

                                vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_CHECK DCC CHEKC:DCC NOT SUPPORT Uny END!");
				return (VS_SUCCESS);
			}
			
			/* 客製化042不支援DCC交易 */
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_042_BDAU1_, _CUSTOMER_INDICATOR_SIZE_))
			{
				pobTran->srBRec.uszDCCTransBit = VS_FALSE;

				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "----------------------------------------");
					inLogPrintf(AT, "DCC CHEKC:DCC NOT SUPPORT Cus_042");
					inLogPrintf(AT, "----------------------------------------");
				}
                                
                                vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_CHECK DCC CHEKC:DCC NOT SUPPORT Cus_042 END!");
				return (VS_SUCCESS);
			}
			else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_043_BDAU9_, _CUSTOMER_INDICATOR_SIZE_))
			{
				pobTran->srBRec.uszDCCTransBit = VS_FALSE;

				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "----------------------------------------");
					inLogPrintf(AT, "DCC CHEKC:DCC NOT SUPPORT Cus_043");
					inLogPrintf(AT, "----------------------------------------");
				}
                                
                                vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_CHECK DCC CHEKC:DCC NOT SUPPORT Cus_043 END!");
				return (VS_SUCCESS);
			}
			else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_041_CASH_, _CUSTOMER_INDICATOR_SIZE_))
			{
				pobTran->srBRec.uszDCCTransBit = VS_FALSE;

				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "----------------------------------------");
					inLogPrintf(AT, "DCC CHEKC:DCC NOT SUPPORT Cus_041");
					inLogPrintf(AT, "----------------------------------------");
				}

                                vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_CHECK DCC CHEKC:DCC NOT SUPPORT Cus_041 END!");
				return (VS_SUCCESS);
			}
			
			/* DCC是否需要結帳(在DCC有開的狀況下，即使該交易不支援DCC，仍然需要DCC結帳成功) */
			if (inNCCC_DCC_Func_Must_SETTLE(pobTran) != VS_SUCCESS)
			{
				vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_CHECK inNCCC_DCC_Func_Must_SETTLE failed");
				return (VS_ERROR);
			}
		}

		/* 設定要進行DCC交易 */
		pobTran->srBRec.uszDCCTransBit = VS_TRUE;

		/* [114197] 中心收單作業ESG永續發展優化
		 * [20260515] DCC交易不支援ESG，改為N 
		 * [20260612] 新增 uszEsgOptrionBit 條件 */
		pobTran->szAgreeEsgBill[0] = 'N';
		pobTran->uszEsgOptrionBit = VS_TRUE;
		pobTran->szEsgSendCR[0] = 'N';
		
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "----------------------------------------");
			inLogPrintf(AT, "DCC CHEKC:DCC ACCEPT");
			inLogPrintf(AT, "----------------------------------------");
		}

		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inNCCC_DCC_CHECK() END !");
			inLogPrintf(AT, "----------------------------------------");
		}
	}
	else
	{
		memset(szHostEnable, 0x00, sizeof(szHostEnable));
		/* DCC Host 沒開 不檢查是否做DCC */
		if (inNCCC_DCC_GetDCC_Enable(pobTran->srBRec.inHDTIndex, szHostEnable)!= VS_SUCCESS)
		{

			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "----------------------------------------");
				inLogPrintf(AT, "DCC CHEKC:NO DCC HOST");
				inLogPrintf(AT, "----------------------------------------");
			}

			/* 找不到DCC 直接跳過 */
                        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_CHECK DCC CHEKC:NO DCC HOST END!");
			return (VS_SUCCESS);
		}

		if (memcmp(szHostEnable, "Y", 1) != 0)
		{
			/* 找到DCC但是沒開 跳過 */
			pobTran->srBRec.uszDCCTransBit = VS_FALSE;

			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "----------------------------------------");
				inLogPrintf(AT, "DCC CHEKC:HDT DCC NOT OPEN");
				inLogPrintf(AT, "----------------------------------------");
			}

                        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_CHECK DCC CHEKC:HDT DCC NOT OPEN END!");
			return (VS_SUCCESS);
		}

		/* (需求單-111060)-DCC流程修改需求 by Russell 2022/4/11 下午 3:26 */
		/* 不參考DCC FlowVersion */

		/* 結帳不用檢核這些 */
		if (pobTran->inTransactionCode != _SETTLE_)
		{
			/* 沒有DCC舊參數，擋下來(當TMS排程下載(參數或AP)成功，但DCC參數下載失敗時，所有交易直接以台幣進行支付，連動結帳時DCC以0元進行結帳) */
			if (inNCCC_DCC_Already_Have_Parameter(pobTran) != VS_TRUE)
			{
				pobTran->srBRec.uszDCCTransBit = VS_FALSE;

				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "----------------------------------------");
					inLogPrintf(AT, "DCC CHEKC:NO DCC PARAMETER");
					inLogPrintf(AT, "----------------------------------------");
				}

                                vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_CHECK DCC CHEKC:NO DCC PARAMETER END!");
				return (VS_SUCCESS);
			}
		
			/* MPAS不支援DCC */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetNCCCFESMode(szTemplate);
			if (memcmp(szTemplate, _NCCC_04_MPAS_MODE_, strlen(_NCCC_04_MPAS_MODE_)) == 0)
			{
				pobTran->srBRec.uszDCCTransBit = VS_FALSE;

				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "----------------------------------------");
					inLogPrintf(AT, "DCC CHEKC:DCC NOT SUPPORT MPAS");
					inLogPrintf(AT, "----------------------------------------");
				}

                                vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_CHECK DCC CHEKC:DCC NOT SUPPORT MPAS END!");
				return (VS_SUCCESS);
			}

			/* 【需求單 - 106252】	DCC交易新增感應及BIN11碼需求 by Russell 2020/1/14 下午 3:37 */
			/* 改為有支援感應 */

			/* 不支援DCC+CUP */
			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				pobTran->srBRec.uszDCCTransBit = VS_FALSE;

				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "----------------------------------------");
					inLogPrintf(AT, "DCC CHEKC:DCC NOT SUPPORT CUP");
					inLogPrintf(AT, "----------------------------------------");
				}

                                vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_CHECK DCC CHEKC:DCC NOT SUPPORT CUP END!");
				return (VS_SUCCESS);
			}

			/* 不支援DCC + SaleOffline */
			if (pobTran->srBRec.inCode == _SALE_OFFLINE_)
			{
				pobTran->srBRec.uszDCCTransBit = VS_FALSE;

				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "----------------------------------------");
					inLogPrintf(AT, "DCC CHEKC:DCC NOT SUPPORT SALE-OFFLINE");
					inLogPrintf(AT, "----------------------------------------");
				}

                                vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_CHECK DCC CHEKC:DCC NOT SUPPORT SALE-OFFLINE END!");
				return (VS_SUCCESS);
			}

			/* 不支援DCC + PreAuth */
			if (pobTran->srBRec.inCode == _PRE_AUTH_)
			{
				pobTran->srBRec.uszDCCTransBit = VS_FALSE;

				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "----------------------------------------");
					inLogPrintf(AT, "DCC CHEKC:DCC NOT SUPPORT PRE-AUTH");
					inLogPrintf(AT, "----------------------------------------");
				}

                                vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_CHECK DCC CHEKC:DCC NOT SUPPORT PRE-AUTH END!");
				return (VS_SUCCESS);
			}

			/* 不支援分期 */
			if (pobTran->srBRec.uszInstallmentBit == VS_TRUE)
			{
				pobTran->srBRec.uszDCCTransBit = VS_FALSE;

				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "----------------------------------------");
					inLogPrintf(AT, "DCC CHEKC:DCC NOT SUPPORT INST");
					inLogPrintf(AT, "----------------------------------------");
				}

                                vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_CHECK DCC CHEKC:DCC NOT SUPPORT INST END!");
				return (VS_SUCCESS);
			}

			/* 不支援紅利 */
			if (pobTran->srBRec.uszRedeemBit == VS_TRUE)
			{
				pobTran->srBRec.uszDCCTransBit = VS_FALSE;

				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "----------------------------------------");
					inLogPrintf(AT, "DCC CHEKC:DCC NOT SUPPORT REDEEM");
					inLogPrintf(AT, "----------------------------------------");
				}

                                vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_CHECK DCC CHEKC:DCC NOT SUPPORT REDEEM END!");
				return (VS_SUCCESS);
			}

			/* 是否為人工輸入卡號 */
			if (pobTran->srBRec.uszManualBit == VS_TRUE &&
			    pobTran->srBRec.uszRefundCTLSBit != VS_TRUE)
			{
				if (pobTran->srBRec.inCode == _REFUND_)
				{
					/* 退貨手動輸入不擋 */
				}
				else
				{
					pobTran->srBRec.uszDCCTransBit = VS_FALSE;

					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "----------------------------------------");
						inLogPrintf(AT, "DCC CHEKC:DCC NOT SUPPORT MENU-KEYIN");
						inLogPrintf(AT, "----------------------------------------");
					}

                                        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_CHECK DCC CHEKC:DCC NOT SUPPORT MENU-KEYIN END!");
					return (VS_SUCCESS);
				}
			}

			/* 調整判斷順序如果是JCB就不用先檢查local BIN 2025/1/6 下午 2:49 */
			/* 判斷DCC是否接受 VISA或Master Card */
			/* 先判斷是哪一種卡 */
			if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_VISA_, strlen(_CARD_TYPE_VISA_)) == 0)
			{
				/* 判斷CFGT中參數是否支援 */
				memset(szDCCSuport, 0x00, sizeof(szDCCSuport));
				inGetSupDccVisa(szDCCSuport);
				if (memcmp(szDCCSuport, "N", 1) == 0)
				{
					pobTran->srBRec.uszDCCTransBit = VS_FALSE;

					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "----------------------------------------");
						inLogPrintf(AT, "DCC CHEKC:CFGT VISA NOT SUPPORT");
						inLogPrintf(AT, "----------------------------------------");
					}

                                        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_CHECK DCC CHEKC:CFGT VISA NOT SUPPORT END!");
					return (VS_SUCCESS);
				}
			}
			else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)) == 0)
			{
				memset(szDCCSuport, 0x00, sizeof(szDCCSuport));
				inGetSupDccMasterCard(szDCCSuport);
				if (memcmp(szDCCSuport, "N", 1) == 0)
				{
					pobTran->srBRec.uszDCCTransBit = VS_FALSE;

					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "----------------------------------------");
						inLogPrintf(AT, "DCC CHEKC:CFGT MASTER NOT SUPPORT");
						inLogPrintf(AT, "----------------------------------------");
					}

                                        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_CHECK DCC CHEKC:CFGT MASTER NOT SUPPORT END!");
					return (VS_SUCCESS);
				}
			}
			/* 非VISA、MASTERCARD 不支援*/
			else
			{
				pobTran->srBRec.uszDCCTransBit = VS_FALSE;

				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "----------------------------------------");
					inLogPrintf(AT, "DCC CHEKC:CARD NOT SUPPORT");
					inLogPrintf(AT, "----------------------------------------");
				}

                                vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_CHECK DCC CHEKC:CARD NOT SUPPORT END!");
				return (VS_SUCCESS);
			}
			
			/* 是否為Local Bin(查Local Bin Table) */
			inRetVal = inNCCC_DCC_LocalBin_Check(pobTran->srBRec.szPAN);
			if (inRetVal == VS_TRUE)
			{
				pobTran->srBRec.uszDCCTransBit = VS_FALSE;

				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "----------------------------------------");
					inLogPrintf(AT, "DCC CHEKC:LOCAL BIN");
					inLogPrintf(AT, "----------------------------------------");
				}

                                vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_CHECK DCC CHEKC:LOCAL BIN END!");
				return (VS_SUCCESS);
			}
			/* 檢查時發生錯誤，直接當台幣 */
			else if (inRetVal == VS_ERROR)
			{
				pobTran->srBRec.uszDCCTransBit = VS_FALSE;

				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "----------------------------------------");
					inLogPrintf(AT, "DCC CHEKC:LOCAL BIN CHECK ERROR");
					inLogPrintf(AT, "----------------------------------------");
				}

                                vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_CHECK DCC CHEKC:LOCAL BIN CHECK ERROR END!");
				return (VS_SUCCESS);
			}
			
			/* 不支援Uny交易 */
			if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
			{
				pobTran->srBRec.uszDCCTransBit = VS_FALSE;

				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "----------------------------------------");
					inLogPrintf(AT, "DCC CHEKC:DCC NOT SUPPORT Uny");
					inLogPrintf(AT, "----------------------------------------");
				}

                                vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_CHECK DCC CHEKC:DCC NOT SUPPORT Uny END!");
				return (VS_SUCCESS);
			}
			
			/* 客製化042不支援DCC交易 */
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_042_BDAU1_, _CUSTOMER_INDICATOR_SIZE_))
			{
				pobTran->srBRec.uszDCCTransBit = VS_FALSE;

				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "----------------------------------------");
					inLogPrintf(AT, "DCC CHEKC:DCC NOT SUPPORT Cus_042");
					inLogPrintf(AT, "----------------------------------------");
				}

                                vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_CHECK DCC CHEKC:DCC NOT SUPPORT Cus_042 END!");
				return (VS_SUCCESS);
			}
			else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_043_BDAU9_, _CUSTOMER_INDICATOR_SIZE_))
			{
				pobTran->srBRec.uszDCCTransBit = VS_FALSE;

				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "----------------------------------------");
					inLogPrintf(AT, "DCC CHEKC:DCC NOT SUPPORT Cus_043");
					inLogPrintf(AT, "----------------------------------------");
				}

                                vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_CHECK DCC CHEKC:DCC NOT SUPPORT Cus_043 END!");
				return (VS_SUCCESS);
			}
			else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_041_CASH_, _CUSTOMER_INDICATOR_SIZE_))
			{
				pobTran->srBRec.uszDCCTransBit = VS_FALSE;

				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "----------------------------------------");
					inLogPrintf(AT, "DCC CHEKC:DCC NOT SUPPORT Cus_041");
					inLogPrintf(AT, "----------------------------------------");
				}

                                vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_CHECK DCC CHEKC:DCC NOT SUPPORT Cus_041 END!");
				return (VS_SUCCESS);
			}
			
			/* DCC是否需要結帳(在DCC有開的狀況下，即使該交易不支援DCC，仍然需要DCC結帳成功) */
			if (inNCCC_DCC_Func_Must_SETTLE(pobTran) != VS_SUCCESS)
			{
				vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_CHECK inNCCC_DCC_Func_Must_SETTLE failed");
				return (VS_ERROR);
			}
		}

		/* 設定要進行DCC交易 */
		pobTran->srBRec.uszDCCTransBit = VS_TRUE;
		/* [114197] 中心收單作業ESG永續發展優化
		 * [20260515] DCC交易不支援ESG，改為N
		 * [20260612] 新增 uszEsgOptrionBit 條件 */
		pobTran->szAgreeEsgBill[0] = 'N';
		pobTran->uszEsgOptrionBit = VS_TRUE;
		pobTran->szEsgSendCR[0] = 'N';
		
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "----------------------------------------");
			inLogPrintf(AT, "DCC CHEKC:DCC ACCEPT");
			inLogPrintf(AT, "pobTran->szAgreeEsgBill:%s",pobTran->szAgreeEsgBill);
			inLogPrintf(AT, "----------------------------------------");
		}

		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inNCCC_DCC_CHECK() END !");
			inLogPrintf(AT, "----------------------------------------");
		}
	}
        
        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_CHECK DCC CHEKC:DCC ACCEPT END!");
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_DCC_GetDCC_Enable
Date&Time       :2016/8/22 上午 10:03
Describe        :傳HostEnable進來，會得到'Y' OR 'N'
*/
int inNCCC_DCC_GetDCC_Enable(int inOrgHDTIndex, char *szHostEnable)
{
	/* 此function只用來查詢DCC是否開，不應該切換Host */
	/* 若之前已查詢到DCC的index為何，就可以直接使用 */
	if (ginDCCHostIndex != -1)
	{
		/* 直接使用 */
	}
	else
	{
		if (inFunc_Find_Specific_HDTindex(inOrgHDTIndex, _HOST_NAME_DCC_, &ginDCCHostIndex) != VS_SUCCESS)
		{
			/* 找不到直接return VS_ERROR */
			ginDCCHostIndex = -1;
			return (VS_ERROR);
		}
	}
	

	/* 如果Load失敗，回復原Host並回傳VS_ERROR(理論上不會有此情況) */
	if (inLoadHDTRec(ginDCCHostIndex) < 0)
	{
		if (inOrgHDTIndex >= 0)
		{
			/* 回覆原本的Host */
			inLoadHDTRec(inOrgHDTIndex);
		}
		else
		{
			/* load回信用卡主機 */
			inLoadHDTRec(0);
		}

		return (VS_ERROR);
	}
		
	/* 判斷HOST是否開啟  */
	memset(szHostEnable, 0x00, 1);
	inGetHostEnable(szHostEnable);
	if (szHostEnable[0] != 'Y')
	{
		ginDCCHostIndex = -1;
		
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "DCC Enable not open.");
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "DCC Enable open.");
		}
	}

	if (inOrgHDTIndex >= 0)
	{
		/* 回覆原本的Host */
		inLoadHDTRec(inOrgHDTIndex);
	}
	else
	{
		/* load回信用卡主機 */
		inLoadHDTRec(0);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_DCC_SwitchToDCC_Host
Date&Time       :2016/8/31 下午 4:11
Describe        :切換到HDT中DCC的REC上，之後要回覆就靠inOrgHDTIndex（在此function中回傳錯誤也會回覆原host）
*/
int inNCCC_DCC_SwitchToDCC_Host(int inOrgHDTIndex)
{
	/* 開機後只找一次DCC */
	if (ginDCCHostIndex != -1)
	{
		/* 直接使用 */
	}
	else
	{
		if (inFunc_Find_Specific_HDTindex(inOrgHDTIndex, _HOST_NAME_DCC_, &ginDCCHostIndex) != VS_SUCCESS)
		{
			/* 找不到直接return VS_ERROR */
			return (VS_ERROR);
		}

	}
	
	/* 如果Load失敗，回復原Host並回傳VS_ERROR(理論上不會有此情況) */
	if (inLoadHDTRec(ginDCCHostIndex) < 0)
	{
		if (inOrgHDTIndex >= 0)
		{
			/* 回覆原本的Host */
			inLoadHDTRec(inOrgHDTIndex);
		}
		else
		{
			/* load回信用卡主機 */
			inLoadHDTRec(0);
		}

		return (VS_ERROR);
	}
	else
	{
		return (VS_SUCCESS);
	}
}

/*
Function        :inNCCC_DCC_CurrencyOption
Date&Time       :2016/8/22 下午 2:36
Describe        :先看Flow version，若為1，依CardBin回覆對應外幣幣別及金額(F59 TableR為空白)，若為晶片卡；看晶片卡的幣別，否則選幣別
*/
int inNCCC_DCC_CurrencyOption(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal2 = VS_SUCCESS;			/* 回傳出去的值 */
	char		szDebugMsg[100 + 1] = {0};
	
        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_CurrencyOption START!");
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inNCCC_DCC_CurrencyOption START!!");
		inLogPrintf(AT, szDebugMsg);
	}
	
	do
	{
		/* inNCCC_DCC_CHECK會決定uszDCCTransBit要不要On,非DCC交易會直接跳出 */
		if (pobTran->srBRec.uszDCCTransBit != VS_TRUE)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "NOT DCC TRANSACTION");
			}
			inRetVal2 = VS_SUCCESS;
			break;
		}

		/* (需求單-111060)-DCC流程修改需求 by Russell 2022/4/11 下午 3:26 */
		/* 不參考DCC FlowVersion */
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "DCC By Cardbin");
		}

		/* 填入空白幣別碼 Verfifone NCCCfunc.c:15244 */
		inRetVal2 = VS_SUCCESS;
		
		/* (需求單-111060)-DCC流程修改需求 by Russell 2022/4/11 下午 3:26 */
		/* 送上主機用card bin選幣別，手動選擇流程拿掉 */

		/* 一定要放，不然會無線迴圈 */
		break;
	}while (1);
	
	/* DCC感應交易不參考CVM，必定要簽名 */
	/* 【需求單 - 106252】	DCC交易新增感應及BIN11碼需求 by Russell 2020/2/20 上午 11:56 */
	if (pobTran->srBRec.uszDCCTransBit == VS_TRUE)
	{
		pobTran->srBRec.uszNoSignatureBit = VS_FALSE;
	}
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "Choose Result: %d", inRetVal2);
		inLogPrintf(AT, szDebugMsg);
		
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "DCC Choose %s", pobTran->srBRec.szDCC_FCAC);
		inLogPrintf(AT, szDebugMsg);
		
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inNCCC_DCC_CurrencyOption END!!");
		inLogPrintf(AT, szDebugMsg);
		inLogPrintf(AT, "----------------------------------------");
	}

        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_CurrencyOption END!");
        
	return (inRetVal2);
}

/*
Function        :inNCCC_DCC_Display_Currency_Option
Date&Time       :2016/8/23 上午 11:43
Describe        :顯示可支援的幣別
*/
int inNCCC_DCC_Display_Currency_Option(int inPage)
{
	int	i = 0;
	int	inCurrencyPositions = 0;
	int	inCurrencyPositionsChiOffset = 0;
	int	inCallingCodesPositions = 0;
	int	inCallingCodesPositionsChiOffset = 0;
	int	inMaxCCI = 0;
	int	inPreOffset = 0;
	char	szCurrencyNameENG[3 + 1] = {0};
	char	szCurrencyNameCHI[6 + 1] = {0};
	char	szCurrencyOption[40 + 1] = {0};
	char	szCountryCallingCodes[40 + 1] = {0};
	char	szCallingCodesOption[40 + 1] = {0};
	char	szKeyDisp[10 + 1] = {0};
	
	/* 看有幾個外幣 */
	for(;; i++)
	{
		if (inLoadCCIRec(i) < 0)
		{
			inMaxCCI = i - 1;
			break;
		}
	}
	
	
	inDISP_ClearAll();
	if (inPage == 1)
	{
		inDISP_PutGraphic(_DCC_CURRENCY_OPT1_, 0,  _COORDINATE_Y_LINE_8_1_);
	}
	else
	{
		inDISP_PutGraphic(_DCC_CURRENCY_OPT1_, 0,  _COORDINATE_Y_LINE_8_1_);
	}

	for (i = 0 + (11 * (inPage - 1)); i < 11 + (11 * (inPage - 1)); i++)
	{
		/* 讀不到代表沒有資料了 */
		if (inLoadCCIRec(i) < 0)
		{
			break;
		}
		
		/* Get 英文名 */
		memset(szCurrencyNameENG, 0x00, sizeof(szCurrencyNameENG));
		inGetDCCCurrencyName(szCurrencyNameENG);

		/* 英文名轉中文 */
		memset(szCurrencyNameCHI, 0x00, sizeof(szCurrencyNameCHI));
		inNCCC_DCC_Get_Currency_Name_CHI_ByCurrencyNameENG(szCurrencyNameENG, szCurrencyNameCHI);
		
		/* Unicode是三個Bytes */
		inCurrencyPositionsChiOffset = strlen(szCurrencyNameCHI) / 3;

		/* 決定按鍵 */
		memset(szKeyDisp, 0x00, sizeof(szKeyDisp));
		if (i == inMaxCCI)
		{
			strcpy(szKeyDisp, "*");
		}
		else
		{
			switch ((i % 11) + 1)
			{
				case 1:
					strcpy(szKeyDisp, "1");
					break;
				case 2:
					strcpy(szKeyDisp, "2");
					break;
				case 3:
					strcpy(szKeyDisp, "3");
					break;
				case 4:
					strcpy(szKeyDisp, "4");
					break;
				case 5:
					strcpy(szKeyDisp, "5");
					break;
				case 6:
					strcpy(szKeyDisp, "6");
					break;
				case 7:
					strcpy(szKeyDisp, "7");
					break;
				case 8:
					strcpy(szKeyDisp, "8");
					break;
				case 9:
					strcpy(szKeyDisp, "9");
					break;
				case 10:
					strcpy(szKeyDisp, "0");
					break;
				default:
					strcpy(szKeyDisp, "*");
					break;
			}
		}
		
		/* 組字串 */
		memset(szCurrencyOption, 0x00, sizeof(szCurrencyOption));
		sprintf(szCurrencyOption, "%s.%s%s", szKeyDisp, szCurrencyNameCHI, szCurrencyNameENG);
		
		/* 國際區碼 */
		memset(szCountryCallingCodes, 0x00, sizeof(szCountryCallingCodes));
		inNCCC_DCC_Get_Country_Calling_Codes(szCurrencyNameENG, szCountryCallingCodes);
		
		memset(szCallingCodesOption, 0x00, sizeof(szCallingCodesOption));
		if (strlen(szCountryCallingCodes) > 0)
		{
			sprintf(szCallingCodesOption, "+%s", szCountryCallingCodes);
		}
		inFunc_PAD_ASCII(szCallingCodesOption, szCallingCodesOption, ' ', 9, _PADDING_RIGHT_);
		
		inPreOffset = 2;
		inCurrencyPositions = 13 - inCurrencyPositionsChiOffset;
		inCallingCodesPositions = 13 - inCallingCodesPositionsChiOffset;
		
		switch ((i % 11) + 1)
		{
			case 1:
				inDISP_ChineseFont_Point_Color(szCurrencyOption, _FONTSIZE_24X22_, _LINE_24_2_, _COLOR_BLACK_,_COLOR_LIGHTYELLOW_, inPreOffset);
				inDISP_ChineseFont_Point_Color(szCallingCodesOption, _FONTSIZE_24X22_, _LINE_24_3_, _COLOR_BLACK_,_COLOR_LIGHTYELLOW_, inPreOffset);
				break;
			case 2:
				inDISP_ChineseFont_Point_Color(szCurrencyOption, _FONTSIZE_24X22_, _LINE_24_2_, _COLOR_BLACK_,_COLOR_LIGHTYELLOW_, inCurrencyPositions);
				inDISP_ChineseFont_Point_Color(szCallingCodesOption, _FONTSIZE_24X22_, _LINE_24_3_, _COLOR_BLACK_,_COLOR_LIGHTYELLOW_, inCallingCodesPositions);
				break;
			case 3:
				inDISP_ChineseFont_Point_Color(szCurrencyOption, _FONTSIZE_24X22_, _LINE_24_6_, _COLOR_BLACK_,_COLOR_LIGHTBLUE_, inPreOffset);
				inDISP_ChineseFont_Point_Color(szCallingCodesOption, _FONTSIZE_24X22_, _LINE_24_7_, _COLOR_BLACK_,_COLOR_LIGHTBLUE_, inPreOffset);
				break;
			case 4:
				inDISP_ChineseFont_Point_Color(szCurrencyOption, _FONTSIZE_24X22_, _LINE_24_6_, _COLOR_BLACK_,_COLOR_LIGHTBLUE_, inCurrencyPositions);
				inDISP_ChineseFont_Point_Color(szCallingCodesOption, _FONTSIZE_24X22_, _LINE_24_7_, _COLOR_BLACK_,_COLOR_LIGHTBLUE_, inCallingCodesPositions);
				break;
			case 5:
				inDISP_ChineseFont_Point_Color(szCurrencyOption, _FONTSIZE_24X22_, _LINE_24_10_, _COLOR_BLACK_,_COLOR_LIGHTYELLOW_, inPreOffset);
				inDISP_ChineseFont_Point_Color(szCallingCodesOption, _FONTSIZE_24X22_, _LINE_24_11_, _COLOR_BLACK_,_COLOR_LIGHTYELLOW_, inPreOffset);
				break;
			case 6:
				inDISP_ChineseFont_Point_Color(szCurrencyOption, _FONTSIZE_24X22_, _LINE_24_10_, _COLOR_BLACK_,_COLOR_LIGHTYELLOW_, inCurrencyPositions);
				inDISP_ChineseFont_Point_Color(szCallingCodesOption, _FONTSIZE_24X22_, _LINE_24_11_, _COLOR_BLACK_,_COLOR_LIGHTYELLOW_, inCallingCodesPositions);
				break;
			case 7:
				inDISP_ChineseFont_Point_Color(szCurrencyOption, _FONTSIZE_24X22_, _LINE_24_14_, _COLOR_BLACK_,_COLOR_LIGHTBLUE_, inPreOffset);
				inDISP_ChineseFont_Point_Color(szCallingCodesOption, _FONTSIZE_24X22_, _LINE_24_15_, _COLOR_BLACK_,_COLOR_LIGHTBLUE_, inPreOffset);
				break;
			case 8:
				inDISP_ChineseFont_Point_Color(szCurrencyOption, _FONTSIZE_24X22_, _LINE_24_14_, _COLOR_BLACK_,_COLOR_LIGHTBLUE_, inCurrencyPositions);
				inDISP_ChineseFont_Point_Color(szCallingCodesOption, _FONTSIZE_24X22_, _LINE_24_15_, _COLOR_BLACK_,_COLOR_LIGHTBLUE_, inCallingCodesPositions);
				break;
			case 9:
				inDISP_ChineseFont_Point_Color(szCurrencyOption, _FONTSIZE_24X22_, _LINE_24_18_, _COLOR_BLACK_,_COLOR_LIGHTYELLOW_, inPreOffset);
				inDISP_ChineseFont_Point_Color(szCallingCodesOption, _FONTSIZE_24X22_, _LINE_24_19_, _COLOR_BLACK_,_COLOR_LIGHTYELLOW_, inPreOffset);
				break;
			case 10:
				inDISP_ChineseFont_Point_Color(szCurrencyOption, _FONTSIZE_24X22_, _LINE_24_18_, _COLOR_BLACK_,_COLOR_LIGHTYELLOW_, inCurrencyPositions);
				inDISP_ChineseFont_Point_Color(szCallingCodesOption, _FONTSIZE_24X22_, _LINE_24_19_, _COLOR_BLACK_,_COLOR_LIGHTYELLOW_, inCallingCodesPositions);
				break;
			case 11:
				inDISP_ChineseFont_Point_Color(szCurrencyOption, _FONTSIZE_24X22_, _LINE_24_22_, _COLOR_BLACK_,_COLOR_LIGHTBLUE_, inPreOffset);
				inDISP_ChineseFont_Point_Color(szCallingCodesOption, _FONTSIZE_24X22_, _LINE_24_23_, _COLOR_BLACK_,_COLOR_LIGHTBLUE_, inPreOffset);
				break;
			default:
				break;
		}
		
	}
	
	inDISP_ChineseFont_Point_Color("請選擇幣別?", _FONTSIZE_24X22_, _LINE_24_22_, _COLOR_BLACK_, _COLOR_WHITE_, inCurrencyPositions);
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_DCC_Get_Currency_Name_CHI_ByCurrencyNameENG
Date&Time       :2016/8/23 上午 11:07
Describe        :吃一個3byte的貨幣英文名，給一個6byte的貨幣中文名
*/
int inNCCC_DCC_Get_Currency_Name_CHI_ByCurrencyNameENG(char *szCurrencyNameENG, char *szCurrencyNameCHI)
{
	if (memcmp(szCurrencyNameENG, _CURRENCY_JPY_ENG_NAME_, _CURRENCY_JPY_ENG_NAME_LEN_) == 0)
	{
		memcpy(szCurrencyNameCHI, _CURRENCY_JPY_CHI_NAME_, _CURRENCY_JPY_CHI_NAME_LEN_);
	}
	else if (memcmp(szCurrencyNameENG, _CURRENCY_USD_ENG_NAME_, _CURRENCY_USD_ENG_NAME_LEN_) == 0)
	{
		memcpy(szCurrencyNameCHI, _CURRENCY_USD_CHI_NAME_, _CURRENCY_USD_CHI_NAME_LEN_);
	}
	else if (memcmp(szCurrencyNameENG, _CURRENCY_HKD_ENG_NAME_, _CURRENCY_HKD_ENG_NAME_LEN_) == 0)
	{
		memcpy(szCurrencyNameCHI, _CURRENCY_HKD_CHI_NAME_, _CURRENCY_HKD_CHI_NAME_LEN_);
	}
	else if (memcmp(szCurrencyNameENG, _CURRENCY_EUR_ENG_NAME_, _CURRENCY_EUR_ENG_NAME_LEN_) == 0)
	{
		memcpy(szCurrencyNameCHI, _CURRENCY_EUR_CHI_NAME_, _CURRENCY_EUR_CHI_NAME_LEN_);
	}
	else if (memcmp(szCurrencyNameENG, _CURRENCY_KRW_ENG_NAME_, _CURRENCY_KRW_ENG_NAME_LEN_) == 0)
	{
		memcpy(szCurrencyNameCHI, _CURRENCY_KRW_CHI_NAME_, _CURRENCY_KRW_CHI_NAME_LEN_);
	}
	else if (memcmp(szCurrencyNameENG, _CURRENCY_MYR_ENG_NAME_, _CURRENCY_MYR_ENG_NAME_LEN_) == 0)
	{
		memcpy(szCurrencyNameCHI, _CURRENCY_MYR_CHI_NAME_, _CURRENCY_MYR_CHI_NAME_LEN_);
	}
	else if (memcmp(szCurrencyNameENG, _CURRENCY_SGD_ENG_NAME_, _CURRENCY_SGD_ENG_NAME_LEN_) == 0)
	{
		memcpy(szCurrencyNameCHI, _CURRENCY_SGD_CHI_NAME_, _CURRENCY_SGD_CHI_NAME_LEN_);
	}
	else if (memcmp(szCurrencyNameENG, _CURRENCY_THB_ENG_NAME_, _CURRENCY_THB_ENG_NAME_LEN_) == 0)
	{
		memcpy(szCurrencyNameCHI, _CURRENCY_THB_CHI_NAME_, _CURRENCY_THB_CHI_NAME_LEN_);
	}
	else if (memcmp(szCurrencyNameENG, _CURRENCY_AUD_ENG_NAME_, _CURRENCY_AUD_ENG_NAME_LEN_) == 0)
	{
		memcpy(szCurrencyNameCHI, _CURRENCY_AUD_CHI_NAME_, _CURRENCY_AUD_CHI_NAME_LEN_);
	}
	else if (memcmp(szCurrencyNameENG, _CURRENCY_CAD_ENG_NAME_, _CURRENCY_CAD_ENG_NAME_LEN_) == 0)
	{
		memcpy(szCurrencyNameCHI, _CURRENCY_CAD_CHI_NAME_, _CURRENCY_CAD_CHI_NAME_LEN_);
	}
	else if (memcmp(szCurrencyNameENG, _CURRENCY_TWD_ENG_NAME_, _CURRENCY_TWD_ENG_NAME_LEN_) == 0)
	{
		memcpy(szCurrencyNameCHI, _CURRENCY_TWD_CHI_NAME_, _CURRENCY_TWD_CHI_NAME_LEN_);
	}
	else
	{
		memcpy(szCurrencyNameCHI, _CURRENCY_UNKNOWN_CHI_NAME_, _CURRENCY_UNKNOWN_CHI_NAME_LEN_);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_DCC_Get_Country_Calling_Codes
Date&Time       :2019/9/23 上午 11:58
Describe        :吃一個3byte的貨幣英文名，回傳國際電話區碼
*/
int inNCCC_DCC_Get_Country_Calling_Codes(char *szCurrencyNameENG, char *szCallingCodes)
{
	
	if (memcmp(szCurrencyNameENG, _CURRENCY_JPY_ENG_NAME_, _CURRENCY_JPY_ENG_NAME_LEN_) == 0)
	{
		memcpy(szCallingCodes, _CURRENCY_JPY_CALLING_CODES_, _CURRENCY_JPY_CALLING_CODES_LEN_);
	}
	else if (memcmp(szCurrencyNameENG, _CURRENCY_USD_ENG_NAME_, _CURRENCY_USD_ENG_NAME_LEN_) == 0)
	{
		memcpy(szCallingCodes, _CURRENCY_USD_CALLING_CODES_, _CURRENCY_USD_CALLING_CODES_LEN_);
	}
	else if (memcmp(szCurrencyNameENG, _CURRENCY_HKD_ENG_NAME_, _CURRENCY_HKD_ENG_NAME_LEN_) == 0)
	{
		memcpy(szCallingCodes, _CURRENCY_HKD_CALLING_CODES_, _CURRENCY_HKD_CALLING_CODES_LEN_);
	}
	else if (memcmp(szCurrencyNameENG, _CURRENCY_EUR_ENG_NAME_, _CURRENCY_EUR_ENG_NAME_LEN_) == 0)
	{
		memcpy(szCallingCodes, _CURRENCY_EUR_CALLING_CODES_, _CURRENCY_EUR_CALLING_CODES_LEN_);
	}
	else if (memcmp(szCurrencyNameENG, _CURRENCY_KRW_ENG_NAME_, _CURRENCY_KRW_ENG_NAME_LEN_) == 0)
	{
		memcpy(szCallingCodes, _CURRENCY_KRW_CALLING_CODES_, _CURRENCY_KRW_CALLING_CODES_LEN_);
	}
	else if (memcmp(szCurrencyNameENG, _CURRENCY_MYR_ENG_NAME_, _CURRENCY_MYR_ENG_NAME_LEN_) == 0)
	{
		memcpy(szCallingCodes, _CURRENCY_MYR_CALLING_CODES_, _CURRENCY_MYR_CALLING_CODES_LEN_);
	}
	else if (memcmp(szCurrencyNameENG, _CURRENCY_SGD_ENG_NAME_, _CURRENCY_SGD_ENG_NAME_LEN_) == 0)
	{
		memcpy(szCallingCodes, _CURRENCY_SGD_CALLING_CODES_, _CURRENCY_SGD_CALLING_CODES_LEN_);
	}
	else if (memcmp(szCurrencyNameENG, _CURRENCY_THB_ENG_NAME_, _CURRENCY_THB_ENG_NAME_LEN_) == 0)
	{
		memcpy(szCallingCodes, _CURRENCY_THB_CALLING_CODES_, _CURRENCY_THB_CALLING_CODES_LEN_);
	}
	else if (memcmp(szCurrencyNameENG, _CURRENCY_AUD_ENG_NAME_, _CURRENCY_AUD_ENG_NAME_LEN_) == 0)
	{
		memcpy(szCallingCodes, _CURRENCY_AUD_CALLING_CODES_, _CURRENCY_AUD_CALLING_CODES_LEN_);
	}
	else if (memcmp(szCurrencyNameENG, _CURRENCY_CAD_ENG_NAME_, _CURRENCY_CAD_ENG_NAME_LEN_) == 0)
	{
		memcpy(szCallingCodes, _CURRENCY_CAD_CALLING_CODES_, _CURRENCY_CAD_CALLING_CODES_LEN_);
	}
	else if (memcmp(szCurrencyNameENG, _CURRENCY_TWD_ENG_NAME_, _CURRENCY_TWD_ENG_NAME_LEN_) == 0)
	{
		
	}
	else
	{
		
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_DCC_Get_Currency_Name_ENG_ByCurrencyCode
Date&Time       :2016/9/8 下午 1:49
Describe        :吃一個3byte的幣別碼，給一個3byte的貨幣英文名
*/
int inNCCC_DCC_Get_Currency_Name_ENG_ByCurrencyCode(char *szCurrencyCode, char *szCurrencyNameENG)
{
	int	i;
	char	szCurrencyCodeTemp[3 + 1];
	
	for (i = 0;; i++)
	{
		if (inLoadCCIRec(i) < 0)
		{
			return (VS_ERROR);
		}

		memset(szCurrencyCodeTemp, 0x00, sizeof(szCurrencyCodeTemp));
		inGetDCCCurrencyCode(szCurrencyCodeTemp);
		
		if (memcmp(szCurrencyCode, szCurrencyCodeTemp, 3) == 0)
		{
			memset(szCurrencyNameENG, 0x00, 3);
			inGetDCCCurrencyName(szCurrencyNameENG);
			
			break;
		}
			
	}

	return (VS_SUCCESS);
}

/*
Function        :inNCCC_DCC_ChipCard_Check_CurrencyCode
Date&Time       :2016/8/31 下午 1:20
Describe        :晶片卡判斷幣別碼流程
 *		 【需求單 - 106252】	DCC交易新增感應及BIN11碼需求 by Russell（感應卡比照晶片流程，所以直接用同一隻）
*/
int inNCCC_DCC_ChipCard_Check_CurrencyCode(TRANSACTION_OBJECT *pobTran)
{
	int		i;
	char		szKey = 0;
	char		szDCCCurrencyCode[3 + 1] = {0};
	char		szDCCCurrencyName[3 + 1] = {0};
	char		szTagVal[6 + 1] = {0};		/* EMV TAG轉成Ascii的值 */
	char		szDebugMsg[100 + 1] = {0};
	char		szDemoMode[1 + 1] = {0};
	unsigned char	usz9F42[2 + 1] = {0};		/* EMV TAG的Hex值 */
	unsigned short	us9F42Len = 0;
	unsigned char	usz5F28[2 + 1] = {0};		/* EMV TAG的Hex值 */
	unsigned short	us5F28Len = 0;
	
	if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
	{
		/* 抓9F42 Application Currency Code(ICC) */
		us9F42Len = sizeof(usz9F42);
		memset(usz9F42, 0x00, sizeof(usz9F42));
		inEMV_Get_Tag_Value(0x9F42, &us9F42Len, usz9F42);
		/* 抓5F28 Issuer Country Code */
		us5F28Len = sizeof(usz5F28);
		memset(usz5F28, 0x00, sizeof(usz5F28));
		inEMV_Get_Tag_Value(0x5F28, &us5F28Len, usz5F28);
	}
	else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
	{
		/* 抓9F42 Application Currency Code(ICC) */
		us9F42Len = pobTran->srEMVRec.in9F42_ApplicationCurrencyCodeLen;
		memset(usz9F42, 0x00, sizeof(usz9F42));
		memcpy(usz9F42, pobTran->srEMVRec.usz9F42_ApplicationCurrencyCode, us9F42Len);
		/* 抓5F28 Issuer Country Code */
		us5F28Len = pobTran->srEMVRec.in5F28_IssuerCountryCodeLen;
		memset(usz5F28, 0x00, sizeof(usz5F28));
		memcpy(usz5F28, pobTran->srEMVRec.usz5F28_IssuerCountryCode, us5F28Len);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "非晶片也非感應，無法判斷，直接轉台幣");
		}
		
		return (_NCCC_DCC_FLOW_TWD_TRASACTION_);
	}
	
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		/* DEMO板一律手動選擇 */
		return (_NCCC_DCC_FLOW_MANUAL_SELECT_);
	}
	else
	{
		memset(szTagVal, 0x00, sizeof(szTagVal));
		inFunc_BCD_to_ASCII(szTagVal, usz9F42, us9F42Len);

		/* 是否有幣別碼 */
		/* 沒有幣別碼的狀況，若沒有幣別碼應該為0000 */
		if (atoi(szTagVal) != 0)
		{
			i = 0;

			while (1)
			{
				if (inLoadCCIRec(i) < 0)
				{
					/* 此卡無法以DCC進行交易，將改由台幣支付，請按[0]確認 */
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_PutGraphic(_DCC_TO_NTD_, 0, _LINE_8_4_);
					inDISP_PutGraphic(_ERR_0_, 0, _LINE_8_7_);
					pobTran->srBRec.uszDCCTransBit = VS_FALSE;

					while (1)
					{
						szKey = uszKBD_GetKey(30);

						if (szKey == _KEY_0_)
						{
							return (_NCCC_DCC_FLOW_TWD_TRASACTION_);
						}
						else if (szKey == _KEY_CANCEL_ || szKey == _KEY_TIMEOUT_)
						{
							return (_NCCC_DCC_FLOW_CANCEL_);
						}

					}

				}

				memset(szDCCCurrencyCode, 0x00, sizeof(szDCCCurrencyCode));
				inGetDCCCurrencyCode(szDCCCurrencyCode);
				/* 因為Tag前有補0，所以從位置1開始 */
				if (memcmp(szDCCCurrencyCode, &szTagVal[1], 3) == 0)
				{
					/* 找到支援幣別，不用手動選 */
					/* 特例狀況，幣別為台幣(901)直接進入台幣交易流程 */
					if (memcmp("901", &szTagVal[1], 3) == 0)
					{
						if (ginDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
							sprintf(szDebugMsg, "幣別碼901");
							inLogPrintf(AT, szDebugMsg);
						}

						return (_NCCC_DCC_FLOW_TWD_TRASACTION_);
					}

					memcpy(pobTran->srBRec.szDCC_FCN, &szTagVal[1], 3);
					memset(szDCCCurrencyName, 0x00, sizeof(szDCCCurrencyName));
					inGetDCCCurrencyName(szDCCCurrencyName);
					memcpy(pobTran->srBRec.szDCC_FCAC, szDCCCurrencyName, 3);

					return (_NCCC_DCC_FLOW_RATE_REQUEST_);
				}
				i++;

			}

		}
		else
		{
			/* 晶片卡是否有國別碼 */
			memset(szTagVal, 0x00, sizeof(szTagVal));
			inFunc_BCD_to_ASCII(szTagVal, usz5F28, us5F28Len);

			/* 沒有國別碼 */
			/* 沒有國別碼的狀況，若沒有國別碼應該為0000 */
			if (atoi(szTagVal) == 0)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "沒幣別碼也沒國別碼");
					inLogPrintf(AT, szDebugMsg);
				}

				return (_NCCC_DCC_FLOW_TWD_TRASACTION_);
			}
			/* 有國別碼的狀況 */
			else
			{
				/* 因為Tag前有補0，所以從位置1開始 */
				/* 國別碼 158(TW)*/
				if (memcmp("158", &szTagVal[1], 3) == 0)
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "國別碼158");
						inLogPrintf(AT, szDebugMsg);
					}

					return (_NCCC_DCC_FLOW_TWD_TRASACTION_);
				}
				else
				{
					return (_NCCC_DCC_FLOW_MANUAL_SELECT_);
				}

			}

		}
	}
}

/*
Function        :inNCCC_DCC_CTLS_Check_CurrencyCode
Date&Time       :2020/2/20 下午 3:49
Describe        :感應卡判斷幣別碼流程
 *               
*/
int inNCCC_DCC_CTLS_Check_CurrencyCode(TRANSACTION_OBJECT *pobTran)
{
	int		i;
	char		szKey = 0;
	char		szDCCCurrencyCode[3 + 1] = {0};
	char		szDCCCurrencyName[3 + 1] = {0};
	char		szTagVal[6 + 1] = {0};		/* EMV TAG轉成Ascii的值 */
	char		szDebugMsg[100 + 1] = {0};
	char		szDemoMode[1 + 1] = {0};
	
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		/* DEMO板一律手動選擇 */
		return (_NCCC_DCC_FLOW_MANUAL_SELECT_);
	}
	else
	{
		/* 抓9F42 Application Currency Code(ICC) */
		memset(szTagVal, 0x00, sizeof(szTagVal));
		inFunc_BCD_to_ASCII(szTagVal, pobTran->srEMVRec.usz9F42_ApplicationCurrencyCode, pobTran->srEMVRec.in9F42_ApplicationCurrencyCodeLen);

		/* 是否有幣別碼 */
		/* 沒有幣別碼的狀況，若沒有幣別碼應該為0000 */
		if (atoi(szTagVal) != 0)
		{
			i = 0;

			while (1)
			{
				if (inLoadCCIRec(i) < 0)
				{
					/* 此卡無法以DCC進行交易，將改由台幣支付，請按[0]確認 */
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_PutGraphic(_DCC_TO_NTD_, 0, _LINE_8_4_);
					inDISP_PutGraphic(_ERR_0_, 0, _LINE_8_7_);
					pobTran->srBRec.uszDCCTransBit = VS_FALSE;

					while (1)
					{
						szKey = uszKBD_GetKey(30);

						if (szKey == _KEY_0_)
						{
							return (_NCCC_DCC_FLOW_TWD_TRASACTION_);
						}
						else if (szKey == _KEY_CANCEL_ || szKey == _KEY_TIMEOUT_)
						{
							return (_NCCC_DCC_FLOW_CANCEL_);
						}

					}

				}

				memset(szDCCCurrencyCode, 0x00, sizeof(szDCCCurrencyCode));
				inGetDCCCurrencyCode(szDCCCurrencyCode);
				/* 因為Tag前有補0，所以從位置1開始 */
				if (memcmp(szDCCCurrencyCode, &szTagVal[1], 3) == 0)
				{
					/* 找到支援幣別，不用手動選 */
					/* 特例狀況，幣別為台幣(901)直接進入台幣交易流程 */
					if (memcmp("901", &szTagVal[1], 3) == 0)
					{
						if (ginDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
							sprintf(szDebugMsg, "幣別碼901");
							inLogPrintf(AT, szDebugMsg);
						}

						return (_NCCC_DCC_FLOW_TWD_TRASACTION_);
					}

					memcpy(pobTran->srBRec.szDCC_FCN, &szTagVal[1], 3);
					memset(szDCCCurrencyName, 0x00, sizeof(szDCCCurrencyName));
					inGetDCCCurrencyName(szDCCCurrencyName);
					memcpy(pobTran->srBRec.szDCC_FCAC, szDCCCurrencyName, 3);

					return (_NCCC_DCC_FLOW_RATE_REQUEST_);
				}
				i++;

			}

		}
		else
		{
			/* 晶片卡是否有國別碼 */
			/* 抓5F28 Issuer Country Code */
			memset(szTagVal, 0x00, sizeof(szTagVal));
			inFunc_BCD_to_ASCII(szTagVal, pobTran->srEMVRec.usz5F28_IssuerCountryCode, pobTran->srEMVRec.in5F28_IssuerCountryCodeLen);

			/* 沒有國別碼 */
			/* 沒有國別碼的狀況，若沒有國別碼應該為0000 */
			if (atoi(szTagVal) == 0)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "沒幣別碼也沒國別碼");
					inLogPrintf(AT, szDebugMsg);
				}

				return (_NCCC_DCC_FLOW_TWD_TRASACTION_);
			}
			/* 有國別碼的狀況 */
			else
			{
				/* 因為Tag前有補0，所以從位置1開始 */
				/* 國別碼 158(TW)*/
				if (memcmp("158", &szTagVal[1], 3) == 0)
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "國別碼158");
						inLogPrintf(AT, szDebugMsg);
					}

					return (_NCCC_DCC_FLOW_TWD_TRASACTION_);
				}
				else
				{
					return (_NCCC_DCC_FLOW_MANUAL_SELECT_);
				}

			}

		}
	}
}

/*
Function        :inNCCC_DCC_OnlineRate
Date&Time       :2016/9/1 下午 5:28
Describe        :DCC詢價
*/
int inNCCC_DCC_OnlineRate(TRANSACTION_OBJECT *pobTran)
{
	int     i = 0;
	int	inHDTIndex = -1;
	int	inOrgTxnCode = -1;
	int	inRetVal = VS_ERROR;
	char	szKey = 0x00;
	char	szTemplate[100 + 1] = {0};
	char	szDemoMode[2 + 1] = {0};
        
        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_OnlineRate START!");
	
	if (pobTran->srBRec.uszDCCTransBit != VS_TRUE)
		return (VS_SUCCESS);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_DCC_OnlineRate() START !");
	}
	
	/* 教育訓練模式 */
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		/* <DCC詢價> */
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_DCC_RATE_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜DCC詢價交易＞ */

		for (i = 0; i < 2; i++)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_SEND_, 0, _COORDINATE_Y_LINE_8_7_);/* 傳送中... */
		}

		/* 切換到DCC Host */
		inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_DCC_, &inHDTIndex);
		if (inHDTIndex == -1)
		{
			vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_OnlineRate inFunc_Find_Specific_HDTindex(%d)", inHDTIndex);
			return (VS_ERROR);
		}
		else
		{
			pobTran->srBRec.inHDTIndex = inHDTIndex;
		}
		inLoadHDTRec(pobTran->srBRec.inHDTIndex);
		inLoadHDPTRec(pobTran->srBRec.inHDTIndex);

		inOrgTxnCode = pobTran->srBRec.inCode;

		/* 步驟 1 Get System Trans Number */
		if (inNCCC_DCC_GetSTAN(pobTran) == VS_ERROR)
		{
			vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_OnlineRate inNCCC_DCC_GetSTAN failed");
			return (VS_ERROR);
		}

		/* GET F59 Data */
		inNCCC_Func_DEMO_Get_DCC_F59_DATA(pobTran);

		/* 主機回應碼 */
		strcpy(pobTran->srBRec.szRespCode, "00");

		/* Set STAN */
		inNCCC_DCC_SetSTAN(pobTran);

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
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "----------------------------------------");
			inLogPrintf(AT, "inNCCC_DCC_OnlineRate()_START");

		}

		/* <DCC詢價> */
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_DCC_RATE_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜DCC詢價交易＞ */

		/* 切換到DCC Host */
		inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_DCC_, &inHDTIndex);
		if (inHDTIndex == -1)
		{
			vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_OnlineRate inFunc_Find_Specific_HDTindex(%d)", inHDTIndex);
			return (VS_ERROR);
		}
		else
		{
			pobTran->srBRec.inHDTIndex = inHDTIndex;
		}
		inLoadHDTRec(pobTran->srBRec.inHDTIndex);
		inLoadHDPTRec(pobTran->srBRec.inHDTIndex);

		inRetVal = inFunc_Get_HDPT_General_Data(pobTran);
		if (inRetVal != VS_SUCCESS)
		{
			vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_OnlineRate inFunc_Get_HDPT_General_Data failed");
			return (VS_ERROR);
		}

		inOrgTxnCode = pobTran->srBRec.inCode;

		/* 步驟 1 Get System Trans Number */
		if (inNCCC_DCC_GetSTAN(pobTran) == VS_ERROR)
		{
			vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_OnlineRate inNCCC_DCC_GetSTAN failed");
			return (VS_ERROR);
		}

		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_PROCESS_, 0, _COORDINATE_Y_LINE_8_7_);	/* 處理中... */

		/* 步驟 1.2 開始連線 */
		if (pobTran->uszConnectionBit != VS_TRUE)
		{
			inRetVal = inFLOW_RunFunction(pobTran, _COMM_START_);
			if (inRetVal != VS_SUCCESS)
			{
				vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_OnlineRate _COMM_START_ failed");
				
                                /* Mirror Message */
                                if (pobTran->uszECRBit == VS_TRUE)
                                {
                                        inECR_SendMirror(pobTran, _MIRROR_MSG_COMM_ERROR_);
                                }
                                        
				/* 通訊失敗‧‧‧‧ */
				pobTran->inTransactionResult = _TRAN_RESULT_COMM_ERROR_;
				pobTran->inErrorMsg = _ERROR_CODE_V3_COMM_;
				inFunc_Display_Error(pobTran);			/* 通訊失敗 */

				return (VS_COMM_ERROR);
			}
		}

		pobTran->srBRec.inCode = _DCC_RATE_; /* 設定交易類別 */
		pobTran->inTransactionCode = _DCC_RATE_; /* 設定交易類別 */
		pobTran->inISOTxnCode = _DCC_RATE_; /* 組 ISO 電文 */

		/* 組_送_收_解 */
		inRetVal = inNCCC_DCC_SendPackRecvUnPack(pobTran);
		inNCCC_DCC_SetSTAN(pobTran);		/* 成功或失敗 System Trace Number 都要加一 */

		if (inRetVal != VS_SUCCESS)
		{
			vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_OnlineRate inNCCC_DCC_SendPackRecvUnPack failed");
			return (inRetVal);
		}
	}

	pobTran->srBRec.inCode = inOrgTxnCode;		/* 設定回原來的交易類別 */
	pobTran->inTransactionCode = inOrgTxnCode;	/* 設定回原來的交易類別 */
	
	
	/* 如果成功判斷 Action Code */
	/* action code 8800代表詢價成功 */
	if (!memcmp(&pobTran->srBRec.szDCC_AC[0], "8800", 4))
	{
		/* 例外處理
		 * 22. DCC 詢價時，若是 ISO Response Code = 00，且 Action Code 為8800 時，
		 * 未回覆 Field_59 Table ID “R” 或回覆 Table ID “R”的資料有異常，則端末機
		 * 顯示【拒絕交易XI】不提示改由台幣支付。
		 */
		
		if (inNCCC_DCC_OnlineRate_CheckF59_TableR(pobTran) != VS_SUCCESS)
		{
			vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_OnlineRate inNCCC_DCC_OnlineRate_CheckF59_TableR failed");
                        /* 交易在這裡結束，交易不論成功或失敗都在這裡斷線
		           交易以前做法是要全部斷網路線，新的做法只要關【SOCKET】，【GPRS】同樣的做法
		        */
		        inCOMM_End(pobTran);

			memset(pobTran->srBRec.szRespCode, 0x00, sizeof(pobTran->srBRec.szRespCode));
			strcpy(pobTran->srBRec.szRespCode, "XI");
			
			/* 拒絕交易 */
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_DECLINE_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = 60;
			strcpy(srDispMsgObj.szErrMsg1, pobTran->srBRec.szRespCode);
			srDispMsgObj.inErrMsg1Line = _LINE_8_5_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
			
			return (VS_ERROR);
		}
		
                vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_OnlineRate res8800 END!");
		return (VS_SUCCESS);
	}
	/* 9430代表該卡為鎖卡 */
	else if (!memcmp(&pobTran->srBRec.szDCC_AC[0], "9430", 4))
	{
		vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_OnlineRate szDCC_AC(%s)", pobTran->srBRec.szDCC_AC);
                /* 交易在這裡結束，交易不論成功或失敗都在這裡斷線
		   交易以前做法是要全部斷網路線，新的做法只要關【SOCKET】，【GPRS】同樣的做法
		*/
		inCOMM_End(pobTran);
		
		/* 表示非DCC交易(保險起見) */
		pobTran->srBRec.uszDCCTransBit = VS_FALSE;

		/* 拒絕交易 */
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, _ERR_DECLINE_);
		srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_5_;
		srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
		srDispMsgObj.inTimeout = 60;
		sprintf(srDispMsgObj.szErrMsg1, "代碼=%s", pobTran->srBRec.szDCC_AC);
		srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
		srDispMsgObj.inBeepTimes = 1;
		srDispMsgObj.inBeepInterval = 0;
		inDISP_Msg_BMP(&srDispMsgObj);

		return (VS_ERROR);
	}
	/*
	 * 9420	無此商店代號
	 * 9421	商店不支援DCC服務
	 * 9423	商店不支援該卡別服務
	 * 9425	無此端末機代號
	 * 9426	端末機不支援DCC服務
	 * 9428	端末機不支援該卡別服務
	 * 9440	無此端末機代號(Terminal Unknown)
	 * 9441	無此商店代號(Merchant Unknown)
	 * 9442	Acquire ID錯誤(Acquire Unknown)
	 * 9443	端末機不支援服務(Terminal Disabled)
	 * "尚未開啟DCC服務
	 * 代碼=xxxx
	 * 將改由台幣支付
	 * 請按[0]確認"
	 */
	else if (!memcmp(&pobTran->srBRec.szDCC_AC[0], "9420", 4)	||
		 !memcmp(&pobTran->srBRec.szDCC_AC[0], "9421", 4)	||
		 !memcmp(&pobTran->srBRec.szDCC_AC[0], "9423", 4)	||
		 !memcmp(&pobTran->srBRec.szDCC_AC[0], "9425", 4)	||
		 !memcmp(&pobTran->srBRec.szDCC_AC[0], "9426", 4)	||
		 !memcmp(&pobTran->srBRec.szDCC_AC[0], "9428", 4)	||
		 !memcmp(&pobTran->srBRec.szDCC_AC[0], "9440", 4)	||
		 !memcmp(&pobTran->srBRec.szDCC_AC[0], "9441", 4)	||
		 !memcmp(&pobTran->srBRec.szDCC_AC[0], "9442", 4)	||
		 !memcmp(&pobTran->srBRec.szDCC_AC[0], "9443", 4))
	{
		/* 交易在這裡結束，交易不論成功或失敗都在這裡斷線
		   交易以前做法是要全部斷網路線，新的做法只要關【SOCKET】，【GPRS】同樣的做法
		*/
		inCOMM_End(pobTran);
		
		/* Load回原Host */
		/* 切換回NCCC Host */
		inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_CREDIT_NCCC_, &inHDTIndex);
		if (inHDTIndex == -1)
		{
			vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_OnlineRate inFunc_Find_Specific_HDTindex(%d)", inHDTIndex);
			return (VS_ERROR);
		}
		else
		{
			pobTran->srBRec.inHDTIndex = inHDTIndex;
		}
		inLoadHDTRec(pobTran->srBRec.inHDTIndex);
		inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
		
		pobTran->srBRec.uszDCCTransBit = VS_FALSE;

		/* 尚未開啟DCC服務，代碼=xxxx，將改由台幣支付，請按[0]確認 */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		memset(szTemplate, 0x00, sizeof(szTemplate));	
		sprintf(szTemplate, "%s", "尚未開啟DCC服務");
		inDISP_ChineseFont_Point_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_4_, _COLOR_BLACK_, _COLOR_WHITE_, 1);
		/* 錯誤代碼 */
		memset(szTemplate, 0x00, sizeof(szTemplate));	
		sprintf(szTemplate, "代碼=%s", pobTran->srBRec.szDCC_AC);
		inDISP_ChineseFont_Point_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_BLACK_, _COLOR_WHITE_, 1);
		/* 將改由台幣支付 */
		memset(szTemplate, 0x00, sizeof(szTemplate));	
		sprintf(szTemplate, "%s", "將改由台幣支付");
		inDISP_ChineseFont_Point_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_BLACK_, _COLOR_WHITE_, 1);
		/* 請按[0]確認 */
		inDISP_PutGraphic(_ERR_0_, 0, _COORDINATE_Y_LINE_8_7_);
		pobTran->srBRec.uszDCCTransBit = VS_FALSE;

		while (1)
		{
			szKey = uszKBD_GetKey(60);

			if (szKey == _KEY_0_)
			{
                                vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_OnlineRate ToTWD_AC=(%s) END!", pobTran->srBRec.szDCC_AC);
				return (VS_SUCCESS);
			}
			else if (szKey == _KEY_CANCEL_ || szKey == _KEY_TIMEOUT_)
			{
				vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_OnlineRate cancel or timeout");
				return (VS_ERROR);
			}

		}
	}
	/*
	 * 9432	幣別不支援(此為本國卡)
	 * 9410	卡別不支援(卡別不支援)
	 * 9444	不支援的卡Bin(Unsupported Bin)
	 * 9445	不支援的幣別(Rate unavailable)
	 * "該卡不支援DCC服務
	 * 代碼=xxxx
	 * 將改由台幣支付
	 * 請按[0]確認"
	 */
	else if (!memcmp(&pobTran->srBRec.szDCC_AC[0], "9410", 4)	||
		 !memcmp(&pobTran->srBRec.szDCC_AC[0], "9432", 4)	||
		 !memcmp(&pobTran->srBRec.szDCC_AC[0], "9444", 4)	||
		 !memcmp(&pobTran->srBRec.szDCC_AC[0], "9445", 4))
	{
		/* 交易在這裡結束，交易不論成功或失敗都在這裡斷線
		   交易以前做法是要全部斷網路線，新的做法只要關【SOCKET】，【GPRS】同樣的做法
		*/
		inCOMM_End(pobTran);
		
		/* Load回原Host */
		/* 切換回NCCC Host */
		inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_CREDIT_NCCC_, &inHDTIndex);
		if (inHDTIndex == -1)
		{
			vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_OnlineRate inFunc_Find_Specific_HDTindex(%d)", inHDTIndex);
			return (VS_ERROR);
		}
		else
		{
			pobTran->srBRec.inHDTIndex = inHDTIndex;
		}
		inLoadHDTRec(pobTran->srBRec.inHDTIndex);
		inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
		
		pobTran->srBRec.uszDCCTransBit = VS_FALSE;

		/* 尚未開啟DCC服務，代碼=xxxx，將改由台幣支付，請按[0]確認 */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		memset(szTemplate, 0x00, sizeof(szTemplate));	
		sprintf(szTemplate, "%s", "該卡不支援DCC服務");
		inDISP_ChineseFont_Point_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_4_, _COLOR_BLACK_, _COLOR_WHITE_, 1);
		/* 錯誤代碼 */
		memset(szTemplate, 0x00, sizeof(szTemplate));	
		sprintf(szTemplate, "代碼=%s", pobTran->srBRec.szDCC_AC);
		inDISP_ChineseFont_Point_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_BLACK_, _COLOR_WHITE_, 1);
		/* 將改由台幣支付 */
		memset(szTemplate, 0x00, sizeof(szTemplate));	
		sprintf(szTemplate, "%s", "將改由台幣支付");
		inDISP_ChineseFont_Point_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_BLACK_, _COLOR_WHITE_, 1);
		/* 請按[0]確認 */
		inDISP_PutGraphic(_ERR_0_, 0, _COORDINATE_Y_LINE_8_7_);
		pobTran->srBRec.uszDCCTransBit = VS_FALSE;

		while (1)
		{
			szKey = uszKBD_GetKey(60);

			if (szKey == _KEY_0_)
			{
                                vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_OnlineRate ToTWD_AC=(%s) END!", pobTran->srBRec.szDCC_AC);
				return (VS_SUCCESS);
			}
			else if (szKey == _KEY_CANCEL_ || szKey == _KEY_TIMEOUT_)
			{
				vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_OnlineRate cancel or timeout");
				return (VS_ERROR);
			}

		}
	}
	else
	{
                /* 交易在這裡結束，交易不論成功或失敗都在這裡斷線
		   交易以前做法是要全部斷網路線，新的做法只要關【SOCKET】，【GPRS】同樣的做法
		*/
		inCOMM_End(pobTran);
		
		/* Load回原Host */
		/* 切換回NCCC Host */
		inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_CREDIT_NCCC_, &inHDTIndex);
		if (inHDTIndex == -1)
		{
			vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_OnlineRate inFunc_Find_Specific_HDTindex(%d)", inHDTIndex);
			return (VS_ERROR);
		}
		else
		{
			pobTran->srBRec.inHDTIndex = inHDTIndex;
		}
		inLoadHDTRec(pobTran->srBRec.inHDTIndex);
		inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
		
		pobTran->srBRec.uszDCCTransBit = VS_FALSE;
		/* 暫不支援此服務，(錯誤代碼)，將改由台幣支付，請按[0]確認 */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		memset(szTemplate, 0x00, sizeof(szTemplate));	
		sprintf(szTemplate, "%s", "暫不支援此服務");
		inDISP_ChineseFont_Point_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_4_, _COLOR_BLACK_, _COLOR_WHITE_, 1);
		/* 錯誤代碼 */
		memset(szTemplate, 0x00, sizeof(szTemplate));	
		sprintf(szTemplate, "代碼=%s", pobTran->srBRec.szDCC_AC);
		inDISP_ChineseFont_Point_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_BLACK_, _COLOR_WHITE_, 1);
		/* 將改由台幣支付 */
		memset(szTemplate, 0x00, sizeof(szTemplate));	
		sprintf(szTemplate, "%s", "將改由台幣支付");
		inDISP_ChineseFont_Point_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_BLACK_, _COLOR_WHITE_, 1);
		/* 請按[0]確認 */
		inDISP_PutGraphic(_ERR_0_, 0, _COORDINATE_Y_LINE_8_7_);
		pobTran->srBRec.uszDCCTransBit = VS_FALSE;

		while (1)
		{
			szKey = uszKBD_GetKey(60);

			if (szKey == _KEY_0_)
			{
                                vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_OnlineRate ToTWD_AC=(%s) END!", pobTran->srBRec.szDCC_AC);
				return (VS_SUCCESS);
			}
			else if (szKey == _KEY_CANCEL_ || szKey == _KEY_TIMEOUT_)
			{
				vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_OnlineRate cancel or timeout");
				return (VS_ERROR);
			}

		}

	}
	
}

/*
Function        :inNCCC_DCC_Final_DCC_Option
Date&Time       :2016/9/1 下午 5:28
Describe        :這裡決定要直接出帳單或改存台幣advice，列印詢價單並選擇是外幣還是台幣
*/
int inNCCC_DCC_Final_DCC_Option(TRANSACTION_OBJECT *pobTran)
{
	int	inTouchEvent = 0;			/* 感應事件 */
	int	inEnterTimeout = 0;			/* 按鍵TimeOut轉數字 */
	int	inIRDU = 0;
	int	inNCCCHostIndex = 0;
	char	szIRDU[12 + 1] = {0};			/* Inverted Rate Display Unit */
	char	szCurrencyAmt[26 + 1] = {0};		/* 放組出金額 */
	char	szCurrencyOption[50 + 1] = {0};		/* 外幣選項 */
	char	szTWDOption[50 + 1] = {0};		/* 台幣選項 */
	char	szConversionFeeDisp[50 + 1] = {0};	/* 轉換費率顯示 */
	char	szConversionFee[50 + 1] = {0};		/* 放組出轉換費率 */
	char	szExchangeAmt[50 + 1] = {0};		/* 轉換金額 */
	char	szExchangeRateDisp[100 + 1] = {0};	/* 轉換比率 */
	char	szKey = 0;				/* 按鍵 */
	
        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_Final_DCC_Option START!");
        
	if (pobTran->srBRec.uszDCCTransBit != VS_TRUE)
        {
                vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_Final_DCC_Option Not_DCC_Pass END!");
		return (VS_SUCCESS);
        }
	
	inFunc_ResetTitle(pobTran);
	
	while (1) 
	{ 
		/* 外幣選項 */
		memset(szCurrencyAmt, 0x00, sizeof(szCurrencyAmt));
		inFunc_Amount_Comma_DCC(pobTran->srBRec.szDCC_FCA, "", ' ', _SIGNED_NONE_, 26, _PADDING_RIGHT_, pobTran->srBRec.szDCC_FCMU, pobTran->srBRec.szDCC_FCAC, szCurrencyAmt);
		memset(szCurrencyOption, 0x00, sizeof(szCurrencyOption));
		sprintf(szCurrencyOption, "1.%s", szCurrencyAmt);
		inFunc_DiscardSpace(szCurrencyOption);
		
		/* 台幣選項 */
		memset(szCurrencyAmt, 0x00, sizeof(szCurrencyAmt));
		sprintf(szCurrencyAmt, "%ld", pobTran->srBRec.lnTxnAmount);
		inFunc_Amount_Comma_DCC(szCurrencyAmt, "", ' ', _SIGNED_NONE_, 26, _PADDING_RIGHT_, "0", "NTD", szCurrencyAmt);
		memset(szTWDOption, 0x00, sizeof(szTWDOption));
		sprintf(szTWDOption, "2.%s", szCurrencyAmt);
		inFunc_DiscardSpace(szTWDOption);
		
		/* 顯示轉換比率*/
		/* Inverted Rate Display Unit */
		inIRDU = atoi(pobTran->srBRec.szDCC_IRDU); 
		memset(szIRDU, 0x00, sizeof(szIRDU));
		strcpy(szIRDU, "1");
		inFunc_PAD_ASCII(szIRDU, szIRDU, '0', (inIRDU + 1), _PADDING_RIGHT_);
	
		memset(szExchangeAmt, 0x00, sizeof(szExchangeAmt));
		inFunc_Amount_Comma_DCC(pobTran->srBRec.szDCC_IRV, "", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, pobTran->srBRec.szDCC_IRMU, "", szExchangeAmt);
		memset(szExchangeRateDisp, 0x00, sizeof(szExchangeRateDisp));
		sprintf(szExchangeRateDisp, "%s %s = %s NTD", szIRDU, pobTran->srBRec.szDCC_FCAC, szExchangeAmt);
		
		/* 轉換費率 */
		memset(szConversionFee, 0x00, sizeof(szConversionFee));
		inFunc_Currency_Conversion_Fee(pobTran->srBRec.szDCC_MPV, pobTran->srBRec.szDCC_MPDP, szConversionFee);
		strcat(szConversionFee, "%");
		memset(szConversionFeeDisp, 0x00, sizeof(szConversionFeeDisp));
		sprintf(szConversionFeeDisp, "Currency Conversion Fee: ");
		
		/* 詢價成功，顯示成功訊息 */
		/* 匯率換算後交易金額 */
		inDISP_ClearAll();
		/* Please Select? */
		inDISP_ChineseFont_Point_Color("Please Select?", _FONTSIZE_16X33_, _LINE_16_1_, _COLOR_BLACK_ ,_COLOR_WHITE_, 1);
		/* 顯示外幣 */
		inDISP_Display_Black_Back(_DCC_FINAL_OPTION_FOREIGN_CURRENCY_OUTER_X1_, _DCC_FINAL_OPTION_FOREIGN_CURRENCY_OUTER_Y1_, _DCC_FINAL_OPTION_OUTER_WIDTH_, _DCC_FINAL_OPTION_OUTER_HEIGHT_);
		inDISP_Display_White_Block(_DCC_FINAL_OPTION_FOREIGN_CURRENCY_INNER_X1_, _DCC_FINAL_OPTION_FOREIGN_CURRENCY_INNER_Y1_, _DCC_FINAL_OPTION_INNER_WIDTH_, _DCC_FINAL_OPTION_INNER_HEIGHT_);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szCurrencyOption, _FONTSIZE_16X33_, _COLOR_BLACK_, _COLOR_WHITE_, 12, 75, VS_FALSE);
		/* Exchange Rate: */
		inDISP_ChineseFont_Point_Color("Exchange Rate:", _FONTSIZE_16X33_, _LINE_16_5_, _COLOR_BLACK_ ,_COLOR_WHITE_, 1);
		/* 顯示轉換比率 */
		inDISP_ChineseFont_Point_Color(szExchangeRateDisp, _FONTSIZE_16X33_, _LINE_16_6_, _COLOR_BLACK_ ,_COLOR_WHITE_, 1);
		/* 顯示轉換費率 */
		inDISP_ChineseFont_Point_Color(szConversionFeeDisp, _FONTSIZE_16X33_, _LINE_16_7_, _COLOR_BLACK_ ,_COLOR_WHITE_, 1);
		inDISP_ChineseFont_Point_Color(szConversionFee, _FONTSIZE_16X33_, _LINE_16_8_, _COLOR_BLACK_ ,_COLOR_WHITE_, 1);
		/* 轉換費率文字說明 */
		inDISP_ChineseFont_Point_Color("(Currency Conversion Fee on ", _FONTSIZE_16X33_, _LINE_16_9_, _COLOR_BLACK_ ,_COLOR_WHITE_, 1);
		inDISP_ChineseFont_Point_Color("the exchange rate over a ", _FONTSIZE_16X33_, _LINE_16_10_, _COLOR_BLACK_ ,_COLOR_WHITE_, 1);
		inDISP_ChineseFont_Point_Color("wholesale rate.)", _FONTSIZE_16X33_, _LINE_16_11_, _COLOR_BLACK_ ,_COLOR_WHITE_, 1);
		inDISP_ChineseFont_Point_Color("", _FONTSIZE_16X33_, _LINE_16_12_, _COLOR_BLACK_ ,_COLOR_WHITE_, 1);
		/* 顯示台幣 */
		inDISP_Display_Black_Back(_DCC_FINAL_OPTION_LOCAL_CURRENCY_OUTER_X1_, _DCC_FINAL_OPTION_LOCAL_CURRENCY_OUTER_Y1_, _DCC_FINAL_OPTION_OUTER_WIDTH_, _DCC_FINAL_OPTION_OUTER_HEIGHT_);
		inDISP_Display_White_Block(_DCC_FINAL_OPTION_LOCAL_CURRENCY_INNER_X1_, _DCC_FINAL_OPTION_LOCAL_CURRENCY_INNER_Y1_, _DCC_FINAL_OPTION_INNER_WIDTH_, _DCC_FINAL_OPTION_INNER_HEIGHT_);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szTWDOption, _FONTSIZE_16X33_, _COLOR_BLACK_, _COLOR_WHITE_, 12, 375, VS_FALSE);
		
		/* 事件初始化 */
		szKey = 0;
		/* 設定Timeout( 三分鐘) */
		inEnterTimeout = 180;
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, inEnterTimeout);
		
		while (1)
		{
			/* 感應 */
			inTouchEvent = inDisTouch_TouchSensor_Click_Slide(_Touch_DCC_CURENCY_CHECK_);
			
			/* 按鍵 */
			szKey = uszKBD_Key();
			
			/* 偵測TimeOut */
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}

			if (szKey == _KEY_1_ || inTouchEvent == _DccCurCheckTouch_KEY_1_)
			{
				vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_Final_DCC_Option Choose DCC END!");
				return (VS_SUCCESS);
			}
			else if (szKey == _KEY_2_ || inTouchEvent == _DccCurCheckTouch_KEY_2_ || szKey == _KEY_TIMEOUT_)
			{
				/* 轉回台幣流程 */
				/* 詢價後卻選台幣 */
				pobTran->srBRec.uszNCCCDCCRateBit = VS_TRUE;
				pobTran->srBRec.uszDCCTransBit = VS_FALSE;
				
				/* 轉換主機 */
				if (inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_CREDIT_NCCC_, &inNCCCHostIndex) != VS_SUCCESS)
				{
					vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_Final_DCC_Option inFunc_Find_Specific_HDTindex failed");
					return (VS_ERROR);
				}
				pobTran->srBRec.inHDTIndex = inNCCCHostIndex;

				if (inLoadHDTRec(pobTran->srBRec.inHDTIndex) < 0)
				{
					vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_Final_DCC_Option inLoadHDTRec failed");
					return (VS_ERROR);
				}

				if (inLoadHDPTRec(pobTran->srBRec.inHDTIndex) < 0)
				{
					vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_Final_DCC_Option inLoadHDPTRec failed");
					return (VS_ERROR);
				}

				inFunc_Get_HDPT_General_Data(pobTran);
				inCOMM_End(pobTran);
				
                                vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_Final_DCC_Option Choose_TWD END!");
                                
				return (VS_SUCCESS);
			}
			else if (szKey == _KEY_CANCEL_)
			{
				vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_Final_DCC_Option VS_USER_CANCEL");
				return (VS_USER_CANCEL);
			}

		}
		
	}
        
        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_Final_DCC_Option Impossible END!");
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_DCC_OnlineRate_CheckF59_TableR
Date&Time       :2016/9/5 上午 11:39
Describe        :檢查回覆Table "R"的資料是否有誤
		註：Table ID “R”的資料有異常定義為，下述欄位值為空白或為0時，
		(1) Foreign Currency No. For Rate
		(2) Foreign Currency Number
		(3) Foreign Currency Amount
		(4) Foreign Currency Alphabetic Code
		(5) Exchange Rate Value
		(6) Inverted Rate Value
 
		22. DCC 詢價時，若是ISO Response Code = 00，且Action Code 為8800 時，
		未回覆Field_59 Table ID “R”或回覆Table ID “R”的資料有異常，，則端末機
		顯示【拒絕交易XI】不提示改由台幣支付。
*/
int inNCCC_DCC_OnlineRate_CheckF59_TableR(TRANSACTION_OBJECT *pobTran)
{
	int	i;

	/* Foreign Currency No. For Rate (3 Byte) */
	for (i = 0; i < 3; i ++)
	{
		if (pobTran->srBRec.szDCC_FCNFR[i] == 0x00 || pobTran->srBRec.szDCC_FCNFR[i] == 0x20)
			return (VS_ERROR);
	}

	/* 修正DCC詢價，Field_59 Table ID "R"解析後內容為0之異常參數，需顯示"拒絕交易XI" by Russell 2018/12/6 下午 5:01 */
	if (!memcmp(pobTran->srBRec.szDCC_FCNFR, "000", 3))
	{
	        return (VS_ERROR);
	}
	
	/* Foreign Currency Number (3 Byte) */
	for (i = 0; i < 3; i ++)
	{
		if (pobTran->srBRec.szDCC_FCN[i] == 0x00 || pobTran->srBRec.szDCC_FCN[i] == 0x20)
			return (VS_ERROR);
	}

	/* 修正DCC詢價，Field_59 Table ID "R"解析後內容為0之異常參數，需顯示"拒絕交易XI" by Russell 2018/12/6 下午 5:01 */
	if (!memcmp(pobTran->srBRec.szDCC_FCN, "000", 3))
	{
		return (VS_ERROR);
	}
	
	/* Foreign Currency Amount (12 Byte) */
	for (i = 0; i < 12; i ++)
	{
		if (pobTran->srBRec.szDCC_FCA[i] == 0x00 || pobTran->srBRec.szDCC_FCA[i] == 0x20)
			return (VS_ERROR);
	}

	/* 修正DCC詢價，Field_59 Table ID "R"解析後內容為0之異常參數，需顯示"拒絕交易XI" by Russell 2018/12/6 下午 5:01 */
	if (!memcmp(pobTran->srBRec.szDCC_FCA, "000000000000", 12)) 
	        return (VS_ERROR);
	
	/* Foreign currcncy Alphabetic Code (3 Byte) */
	for (i = 0; i < 3; i ++)
	{
		if (pobTran->srBRec.szDCC_FCAC[i] == 0x00 || pobTran->srBRec.szDCC_FCAC[i] == 0x20)
			return (VS_ERROR);
	}

	/* 修正DCC詢價，Field_59 Table ID "R"解析後內容為0之異常參數，需顯示"拒絕交易XI" by Russell 2018/12/6 下午 5:01 */
	if (!memcmp(pobTran->srBRec.szDCC_FCAC, "000", 3)) 
	        return (VS_ERROR);
	
	/* Exchange Rate Value (9 Byte) */
	for (i = 0; i < 9; i ++)
	{
		if (pobTran->srBRec.szDCC_ERV[i] == 0x00 || pobTran->srBRec.szDCC_ERV[i] == 0x20)
			return (VS_ERROR);
	}
	
	/* 修正DCC詢價，Field_59 Table ID "R"解析後內容為0之異常參數，需顯示"拒絕交易XI" by Russell 2018/12/6 下午 5:01 */
	if (!memcmp(pobTran->srBRec.szDCC_ERV, "000000000", 9)) 
	        return (VS_ERROR);
	
	/* Inverted Rate Value (9 Byte) */
	for (i = 0; i < 9; i ++)
	{
		if (pobTran->srBRec.szDCC_IRV[i] == 0x00 || pobTran->srBRec.szDCC_IRV[i] == 0x20)
			return (VS_ERROR);
	}
	
	/* 修正DCC詢價，Field_59 Table ID "R"解析後內容為0之異常參數，需顯示"拒絕交易XI" by Russell 2018/12/6 下午 5:01 */
	if (!memcmp(pobTran->srBRec.szDCC_IRV, "000000000", 9))
		return (VS_ERROR);

	return (VS_SUCCESS);
}

/*
Function	:inNCCC_DCC_LocalBin_Check
Date&Time	:2016/8/16 上午 11:35
Describe        :確認是否LocalBin，是則回傳VS_TRUE,否則回傳VS_FALSE
*/
int inNCCC_DCC_LocalBin_Check(char* szPAN)
{
	int		i, inValidLength = 0;				/* 算有效長度使用 */
	int		inResult = VS_FALSE;
	int		inRecTotalCnt = 0;
	int		inRetVal = 0;
        int		inRetVal2 = VS_SUCCESS;
	long		lnSearchStart, lnSearchEnd, lnSearchIndex = 0;	/* lnSearchStart:範圍開頭在第幾筆 lnSearchEnd:範圍結束在第幾筆 */
        long		lnOffset, lnBinRecordSize = 0;
	char		szDCCBinRecord[20 + 1];
	char		szDebugMsg[100 + 1];
	char		szReadData;
	unsigned long	ulFileHandle;

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inNCCC_DCC_LocalBin_Check()_START");
        }

	/* 理論上會在Check_Terminal_Status擋掉沒舊參數的狀況，但要是有萬一，直接當本國卡 */
	if (inFILE_OpenReadOnly(&ulFileHandle, (unsigned char*)_NCCC_DCC_FILE_CARD_BIN_) != VS_SUCCESS)
	{
		return (VS_TRUE);
	}
        
        do
        {
		/* 因DCCBin.txt不確定record大小，所以先抓看單一record多大 */
		inFILE_Seek(ulFileHandle, 0, _SEEK_BEGIN_);
		while (1)
		{
			memset(&szReadData, 0x00, 1);
			inFILE_Read(&ulFileHandle, (unsigned char*)&szReadData, 1);
			lnBinRecordSize++;

			if (szReadData == 0x0A)
			{
				break;
			}

		}

		/* 算出DCCBin.txt有幾個Bin Record */
		inRecTotalCnt = lnFILE_GetSize(&ulFileHandle, (unsigned char*)_NCCC_DCC_FILE_CARD_BIN_) / lnBinRecordSize;

		lnSearchStart = 1;
		lnSearchEnd = inRecTotalCnt;

		if (inRecTotalCnt == 1)
			lnSearchIndex = 1;
		else
			lnSearchIndex = (inRecTotalCnt / 2);	/* 二分法 */


		while (1)
		{
			lnOffset = (lnSearchIndex - 1) * lnBinRecordSize;	/* offset從0開始所以要減一 */

			/* seek .bkey */
			if (lnOffset >= 0)
			{
				if (inFILE_Seek(ulFileHandle, lnOffset, _SEEK_BEGIN_) == VS_ERROR)
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "inNCCC_DCC_LocalBin_Check() Seek Record Error!");
					}
					vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_LocalBin_Check() Seek Record Error!");

					inRetVal2 = VS_ERROR;
					break;
				}
			}
			else
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "inNCCC_DCC_LocalBin_Check() Seek offset < 0");
				}
				vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_LocalBin_Check() Seek offset < 0");
				
				inRetVal2 = VS_ERROR;
				break;
			}

			/* 一次讀取BinRecordSize的大小 */
			memset(szDCCBinRecord, 0x00, sizeof(szDCCBinRecord));
			if (inFILE_Read(&ulFileHandle, (unsigned char *)szDCCBinRecord, lnBinRecordSize) == VS_ERROR)
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "inNCCC_DCC_LocalBin_Check() Read Record Error!");
				}

				inRetVal2 = VS_ERROR;
				break;
			}

			/* F之前都算有效長度 */
			for (i = 0; i < lnBinRecordSize; i++)
			{
				if (szDCCBinRecord[i] == 'F')
				{
					break;
				}

			}

			inValidLength = i;

			/* 判斷讀到的資料是否正確，只有前6位有效 */
			inRetVal = memcmp(szDCCBinRecord, szPAN, inValidLength);
			if (inRetVal == 0)
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "The PAN is Local BIN");
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "Index: %ld ,Bin Range: %s", lnSearchIndex, szDCCBinRecord);
					inLogPrintf(AT, szDebugMsg);
				}
				inResult = VS_TRUE;

				break;
			}
			else
			{

				/* 無法再二分下去(小數點自動捨去 ex: (7 + 8) / 2 = 7) */
				if (lnSearchStart >= lnSearchEnd)
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "The PAN is NOT Local BIN! No other index,start = %d end = %d", lnSearchStart, lnSearchEnd);
					}

					inResult = VS_FALSE;

					break;
				}

				if (inRetVal > 0)
				{
					/* 往上邊找 */
					lnSearchEnd = lnSearchIndex - 1;
				}
				else
				{
					/* 往下邊找 */
					lnSearchStart = lnSearchIndex + 1;
				}

				lnSearchIndex = (lnSearchStart + lnSearchEnd) / 2;
				
				if (lnSearchIndex < 1 || lnSearchIndex > inRecTotalCnt)
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "The PAN is NOT Local BIN! out of bound,index = %d", lnSearchIndex);
					}

					inResult = VS_FALSE;

					break;
				}
			}

		} /* end while loop */

		if (inRetVal2 == VS_ERROR)
		{
			break;
		}

		break;
        }while(1);

	inFILE_Close(&ulFileHandle);
	
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inNCCC_DCC_LocalBin_Check()_END");
                inLogPrintf(AT, "----------------------------------------");
        }

        if (inRetVal2 == VS_ERROR)
        {
                return (VS_ERROR);
        }
        else
        {
                return (inResult);
        }
}

/*
Function	:inNCCC_DCC_Sync_BatchNumber
Date&Time	:2016/9/1 上午 11:10
Describe        :同步NCCC和DCC的Batch Number
*/
int inNCCC_DCC_Sync_BatchNumber(TRANSACTION_OBJECT *pobTran)
{
	int	inOrgHostIndex = -1;	/* 因為inACCUM_Check_Transaction_Count會還原回pobTran->srBRec.inHDTIndex，所以多用inOrgHostIndex紀錄原Host*/
	int	inDCC_HostIndex = -1;
	int	inNCCC_HostIndex = -1;
	char	szBatchNumNCCC[6 + 1] = {0};
	char	szBatchNumDCC[6 + 1] = {0};
	char	szTrnas = 0;		/* Host 是否有帳 */
	char	szSwitch[2 + 1] = {0};
	BOOL	bNeedSettleBit = VS_FALSE;
	unsigned char	uszFileName[30 + 1] = {0};
	
	inOrgHostIndex = pobTran->srBRec.inHDTIndex;
	
	if (inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_DCC_, &inDCC_HostIndex) != VS_SUCCESS)
	{
		/* 如果沒有DCC HOST 直接跳出，不回傳ERROR */
		return (VS_SUCCESS);
	}
	
	if (inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_CREDIT_NCCC_, &inNCCC_HostIndex) != VS_SUCCESS)
	{
		/* 如果沒有NCCC HOST 直接跳出，不回傳ERROR */
		return (VS_SUCCESS);
	}
	
	/* 獲得NCCC Batch Number */
	if (inLoadHDPTRec(inNCCC_HostIndex) != VS_SUCCESS)
		return (VS_ERROR);
	memset(szBatchNumNCCC, 0x00, sizeof(szBatchNumNCCC));
	if (inGetBatchNum(szBatchNumNCCC) != VS_SUCCESS)
		return (VS_ERROR);

	/* 獲得DCC Batch Number */
	if (inLoadHDPTRec(inDCC_HostIndex) != VS_SUCCESS)
		return (VS_ERROR);
	memset(szBatchNumDCC, 0x00, sizeof(szBatchNumDCC));
	if (inGetBatchNum(szBatchNumDCC) != VS_SUCCESS)
		return (VS_ERROR);
	
	memset(szSwitch, 0x00, sizeof(szSwitch));
	inFunc_Get_ResetBatchNum_Switch(szSwitch);
	
	/* 這邊要分是誰結帳，如果NCCC結帳，若NCCC比較大，則DCC SettleBit On(若沒帳，自動調整到與NCCC一樣)起來，若DCC比較大，則NCCC Batch Num同步到和DCC一樣
				反之，    若DCC比較大，則NCCC SettleBit On(若沒帳，自動調整到與DCC一樣)起來，若NCCC比較大，則DCC Batch Num同步到和NCCC一樣
	 */
	if (atol(szBatchNumNCCC) == atol(szBatchNumDCC))
	{
		/* 等於的狀況就不用同步了 */
	}
	else
	{
		/* 結帳的是NCCC */
		if (inOrgHostIndex == inNCCC_HostIndex)
		{
			/* 有優先性的狀況下 */
			if (memcmp(szSwitch, _RESET_BATCH_NONE_, 1) != 0)
			{
				/* 以NCCC為主 */
				if (memcmp(szSwitch, _RESET_BATCH_NCCC_FIRST_, 1) == 0)
				{
					pobTran->srBRec.inHDTIndex = inDCC_HostIndex;	/* 這個動作是為了inACCUM_Check_Transaction_Count還原用pobTran->srBRec.inHDTIndex */

					if (inLoadHDPTRec(inDCC_HostIndex) != VS_SUCCESS)
						return (VS_ERROR);

					do
					{
						/* DCC有帳的狀況，強制結帳 */
						szTrnas = 0x00;
						if (inACCUM_Check_Transaction_Count(pobTran, _HOST_NAME_DCC_, &szTrnas) != VS_SUCCESS)
						{
							return (VS_ERROR);
						}

						if (memcmp(&szTrnas, "Y", 1) == 0)
						{
							bNeedSettleBit = VS_TRUE;
							break;
						}

						/* Check REVERSAL */
						memset(uszFileName, 0x00, sizeof(uszFileName));
						if (inFunc_ComposeFileName(pobTran, (char*)uszFileName, _REVERSAL_FILE_EXTENSION_, 6) != VS_SUCCESS)
						{
							if (ginDebug == VS_TRUE)
							 {
								 inLogPrintf(AT, "組filename失敗：%s", uszFileName);
							 }
							 return (VS_ERROR);
						}
						if (inFILE_Check_Exist(uszFileName) == VS_SUCCESS)
						{
							 /* 表示要結帳 */
							 bNeedSettleBit = VS_TRUE;
							 break;
						}
						break;
					}while(1);
		       
					if (bNeedSettleBit == VS_TRUE)
					{
						inSetMustSettleBit("Y");
					}
					else
					{
						/* 沒帳的狀況，直接調成一樣 */
						if (inSetBatchNum(szBatchNumNCCC) != VS_SUCCESS)
							return (VS_ERROR);

						inFunc_Set_ResetBatchNum_Switch(_RESET_BATCH_NONE_);
					}

					if (inSaveHDPTRec(inDCC_HostIndex) != VS_SUCCESS)
					{
						inFunc_EDCLock(AT);
					}
				}
				else if (memcmp(szSwitch, _RESET_BATCH_DCC_FIRST_, 1) == 0)
				{
					pobTran->srBRec.inHDTIndex = inNCCC_HostIndex;	/* 這個動作是為了inACCUM_Check_Transaction_Count還原用pobTran->srBRec.inHDTIndex */

					if (inLoadHDPTRec(inNCCC_HostIndex) != VS_SUCCESS)
						return (VS_ERROR);
					if (inSetBatchNum(szBatchNumDCC) != VS_SUCCESS)
						return (VS_ERROR);
					if (inSaveHDPTRec(inNCCC_HostIndex) != VS_SUCCESS)
					{
						inFunc_EDCLock(AT);
					}
					
					inFunc_Set_ResetBatchNum_Switch(_RESET_BATCH_NONE_);
				}
			}
			else if (atol(szBatchNumNCCC) > atol(szBatchNumDCC))
			{
				pobTran->srBRec.inHDTIndex = inDCC_HostIndex;	/* 這個動作是為了inACCUM_Check_Transaction_Count還原用pobTran->srBRec.inHDTIndex */

				if (inLoadHDPTRec(inDCC_HostIndex) != VS_SUCCESS)
					return (VS_ERROR);
				
				do
				{
					/* DCC有帳的狀況，強制結帳 */
					szTrnas = 0x00;
					if (inACCUM_Check_Transaction_Count(pobTran, _HOST_NAME_DCC_, &szTrnas) != VS_SUCCESS)
					{
						return (VS_ERROR);
					}

					if (memcmp(&szTrnas, "Y", 1) == 0)
					{
						bNeedSettleBit = VS_TRUE;
						break;
					}

					/* Check REVERSAL */
					memset(uszFileName, 0x00, sizeof(uszFileName));
					if (inFunc_ComposeFileName(pobTran, (char*)uszFileName, _REVERSAL_FILE_EXTENSION_, 6) != VS_SUCCESS)
					{
						if (ginDebug == VS_TRUE)
						 {
							 inLogPrintf(AT, "組filename失敗：%s", uszFileName);
						 }
						 return (VS_ERROR);
					}
					if (inFILE_Check_Exist(uszFileName) == VS_SUCCESS)
					{
						 /* 表示要結帳 */
						 bNeedSettleBit = VS_TRUE;
						 break;
					}
					break;
				}while(1);

				if (bNeedSettleBit == VS_TRUE)
				{
					inSetMustSettleBit("Y");
				}
				else
				{
					/* 沒帳的狀況，直接調成一樣 */
					if (inSetBatchNum(szBatchNumNCCC) != VS_SUCCESS)
						return (VS_ERROR);
				}

				if (inSaveHDPTRec(inDCC_HostIndex) != VS_SUCCESS)
				{
					inFunc_EDCLock(AT);
				}

			}
			else if (atol(szBatchNumNCCC) < atol(szBatchNumDCC))
			{
				pobTran->srBRec.inHDTIndex = inNCCC_HostIndex;	/* 這個動作是為了inACCUM_Check_Transaction_Count還原用pobTran->srBRec.inHDTIndex */

				if (inLoadHDPTRec(inNCCC_HostIndex) != VS_SUCCESS)
					return (VS_ERROR);
				if (inSetBatchNum(szBatchNumDCC) != VS_SUCCESS)
					return (VS_ERROR);
				if (inSaveHDPTRec(inNCCC_HostIndex) != VS_SUCCESS)
				{
					inFunc_EDCLock(AT);
				}
			}
			else
			{
				/* 鎖機 */
				inFunc_EDCLock(AT);
			}
		}
		else if (inOrgHostIndex == inDCC_HostIndex)
		{
			/* 有優先性的狀況下 */
			if (memcmp(szSwitch, _RESET_BATCH_NONE_, 1) != 0)
			{
				/* 以DCCC為主 */
				if (memcmp(szSwitch, _RESET_BATCH_DCC_FIRST_, 1) == 0)
				{
					pobTran->srBRec.inHDTIndex = inNCCC_HostIndex;	/* 這個動作是為了inACCUM_Check_Transaction_Count還原用pobTran->srBRec.inHDTIndex */

					if (inLoadHDPTRec(inNCCC_HostIndex) != VS_SUCCESS)
						return (VS_ERROR);

					do
					{
						/* NCCC有帳的狀況，強制結帳 */
						szTrnas = 0x00;
						if (inACCUM_Check_Transaction_Count(pobTran, _HOST_NAME_CREDIT_NCCC_, &szTrnas) != VS_SUCCESS)
						{
							return (VS_ERROR);
						}

						if (memcmp(&szTrnas, "Y", 1) == 0)
						{
							bNeedSettleBit = VS_TRUE;
							break;
						}

						/* Check REVERSAL */
						memset(uszFileName, 0x00, sizeof(uszFileName));
						if (inFunc_ComposeFileName(pobTran, (char*)uszFileName, _REVERSAL_FILE_EXTENSION_, 6) != VS_SUCCESS)
						{
							if (ginDebug == VS_TRUE)
							 {
								 inLogPrintf(AT, "組filename失敗：%s", uszFileName);
							 }
							 return (VS_ERROR);
						}
						if (inFILE_Check_Exist(uszFileName) == VS_SUCCESS)
						{
							 /* 表示要結帳 */
							 bNeedSettleBit = VS_TRUE;
							 break;
						}
						break;
					}while(1);
					
					if (bNeedSettleBit == VS_TRUE)
					{
						inSetMustSettleBit("Y");
					}
					else
					{
						/* 沒帳的狀況，直接調成一樣 */
						if (inSetBatchNum(szBatchNumDCC) != VS_SUCCESS)
							return (VS_ERROR);

						inFunc_Set_ResetBatchNum_Switch(_RESET_BATCH_NONE_);
					}

					if (inSaveHDPTRec(inNCCC_HostIndex) != VS_SUCCESS)
					{
						inFunc_EDCLock(AT);
					}
				}
				else if (memcmp(szSwitch, _RESET_BATCH_NCCC_FIRST_, 1) == 0)
				{
					pobTran->srBRec.inHDTIndex = inDCC_HostIndex;	/* 這個動作是為了inACCUM_Check_Transaction_Count還原用pobTran->srBRec.inHDTIndex */

					if (inLoadHDPTRec(inDCC_HostIndex) != VS_SUCCESS)
						return (VS_ERROR);
					if (inSetBatchNum(szBatchNumNCCC) != VS_SUCCESS)
						return (VS_ERROR);
					if (inSaveHDPTRec(inDCC_HostIndex) != VS_SUCCESS)
					{
						inFunc_EDCLock(AT);
					}
					
					inFunc_Set_ResetBatchNum_Switch(_RESET_BATCH_NONE_);
				}
			}
			else if (atol(szBatchNumNCCC) < atol(szBatchNumDCC))
			{
				pobTran->srBRec.inHDTIndex = inNCCC_HostIndex;	/* 這個動作是為了inACCUM_Check_Transaction_Count還原用pobTran->srBRec.inHDTIndex */

				if (inLoadHDPTRec(inNCCC_HostIndex) != VS_SUCCESS)
					return (VS_ERROR);

				do
				{
					/* NCCC有帳的狀況，強制結帳 */
					szTrnas = 0x00;
					if (inACCUM_Check_Transaction_Count(pobTran, _HOST_NAME_CREDIT_NCCC_, &szTrnas) != VS_SUCCESS)
					{
						return (VS_ERROR);
					}

					if (memcmp(&szTrnas, "Y", 1) == 0)
					{
						bNeedSettleBit = VS_TRUE;
						break;
					}

					/* Check REVERSAL */
					memset(uszFileName, 0x00, sizeof(uszFileName));
					if (inFunc_ComposeFileName(pobTran, (char*)uszFileName, _REVERSAL_FILE_EXTENSION_, 6) != VS_SUCCESS)
					{
						if (ginDebug == VS_TRUE)
						 {
							 inLogPrintf(AT, "組filename失敗：%s", uszFileName);
						 }
						 return (VS_ERROR);
					}
					if (inFILE_Check_Exist(uszFileName) == VS_SUCCESS)
					{
						 /* 表示要結帳 */
						 bNeedSettleBit = VS_TRUE;
						 break;
					}
					break;
				}while(1);

				if (bNeedSettleBit == VS_TRUE)
				{
					inSetMustSettleBit("Y");
				}
				else
				{
					/* 沒帳的狀況，直接調成一樣 */
					if (inSetBatchNum(szBatchNumDCC) != VS_SUCCESS)
						return (VS_ERROR);

				}

				if (inSaveHDPTRec(inNCCC_HostIndex) != VS_SUCCESS)
				{
					inFunc_EDCLock(AT);
				}

			}
			else if (atol(szBatchNumNCCC) > atol(szBatchNumDCC))
			{
				pobTran->srBRec.inHDTIndex = inDCC_HostIndex;	/* 這個動作是為了inACCUM_Check_Transaction_Count還原用pobTran->srBRec.inHDTIndex */

				if (inLoadHDPTRec(inDCC_HostIndex) != VS_SUCCESS)
					return (VS_ERROR);
				if (inSetBatchNum(szBatchNumNCCC) != VS_SUCCESS)
					return (VS_ERROR);
				if (inSaveHDPTRec(inDCC_HostIndex) != VS_SUCCESS)
				{
					inFunc_EDCLock(AT);
				}
			}
			else
			{
				/* 鎖機 */
				inFunc_EDCLock(AT);
			}
		}
		else
		{
			/* 鎖機 */
			inFunc_EDCLock(AT);
		}
	}
	
	/* 同步完了要load回原來的HDPT避免意外 */
	pobTran->srBRec.inHDTIndex = inOrgHostIndex;
	if (inLoadHDPTRec(inOrgHostIndex) != VS_SUCCESS)
		return (VS_ERROR);
		
	return (VS_SUCCESS);
}

/*
Function	:inNCCC_DCC_DCCBin_Transfer
Date&Time	:2016/8/15 下午 4:51
Describe        :將主機帶下來的DCCBin.txt轉換成CBIN.dat（目前寫法不使用，直接讀DCCBin.txt）
*/
int inNCCC_DCC_DCCBin_Transfer(void)
{
	int             i, j = 0;		/* i為目前從DCCBin.txt讀到的第幾個字元, j為該record的長度 */
	long            lnReadLength;		/* 記錄每次要從DCCBin.txt讀多長的資料 */
	long		lnCBINLength = 0;	/* CBIN.dat的長度 */
	long		lnDCCBINLength = 0;	
	char		szDebugMsg[100 + 1];	
	unsigned char   *uszReadData;		/* 放抓到的資料 */
        unsigned char   *uszTemp;		/* 暫存，放整筆DCCBin.txt檔案 */
	unsigned long   ulFile_Handle;		/* File Handle */
	
	/* inFunc_DCCBin_Transfer() END */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                sprintf(szDebugMsg, "inNCCC_DCC_DCCBin_Transfer START!!");
                inLogPrintf(AT, szDebugMsg);
        }
	
	if (inFILE_Open(&ulFile_Handle, (unsigned char *)_NCCC_DCC_FILE_CARD_BIN_) == VS_ERROR)
        {
                /* 開檔失敗 ，不用關檔 */
                /* 開檔失敗，所以回傳error */
                return (VS_ERROR);
        }
	
	/*
         * get data size 為了接下來動態宣告記憶體的大小需要知道多少 需先取得檔案Size
         * get size 失敗時，if條件成立，關檔並回傳VS_ERROR
         */
	lnDCCBINLength = lnFILE_GetSize(&ulFile_Handle, (unsigned char *)_NCCC_DCC_FILE_CARD_BIN_);
	/*
         * allocate 記憶體
         * allocate時多分配一個byte以防萬一（ex:換行符號）
         */
        uszReadData = malloc(lnDCCBINLength + 1);
        uszTemp = malloc(lnDCCBINLength + 1);
        /* 初始化 uszTemp uszReadData */
        memset(uszReadData, 0x00, lnDCCBINLength + 1);
        memset(uszTemp, 0x00, lnDCCBINLength + 1);
	
	/* seek 到檔案開頭 & 從檔案開頭開始read */
        if (inFILE_Seek(ulFile_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnDCCBINLength;

                for (i = 0;; ++i)
                {
                        /* 剩餘長度大於或等於1024 */
                        if (lnReadLength >= 1024)
                        {
                                if (inFILE_Read(&ulFile_Handle, &uszTemp[1024 * i], 1024) == VS_SUCCESS)
                                {
                                        /* 一次讀1024 */
                                        lnReadLength -= 1024;

                                        /* 當剩餘長度剛好為1024，會剛好讀完 */
                                        if (lnReadLength == 0)
                                                break;
                                }
                                /* 讀失敗時 */
                                else
                                {
                                        /* Close檔案 */
                                        inFILE_Close(&ulFile_Handle);

                                        /* Free pointer */
                                        free(uszReadData);
                                        free(uszTemp);

                                        return (VS_ERROR);
                                }
                        }
                        /* 剩餘長度小於1024 */
                        else if (lnReadLength < 1024)
                        {
                                /* 就只讀剩餘長度 */
                                if (inFILE_Read(&ulFile_Handle, &uszTemp[1024 * i], lnReadLength) == VS_SUCCESS)
                                {
                                        break;
                                }
                                /* 讀失敗時 */
                                else
                                {
                                        /* Close檔案 */
                                        inFILE_Close(&ulFile_Handle);

                                        /* Free pointer */
                                        free(uszReadData);
                                        free(uszTemp);

                                        return (VS_ERROR);
                                }
				
                        }
			
                }/* end for loop */
		
        }
        /* seek不成功時 */
        else
        {
                /* 關檔並回傳 */
                inFILE_Close(&ulFile_Handle);
                /* Free pointer */
                free(uszReadData);
                free(uszTemp);

                /* Seek失敗，所以回傳Error */
                return (VS_ERROR);
        }
	
	
	/*
         * 抓取所需要的那筆record
         * i為目前從DCCCBIN讀到的第幾個字元
         * j為該record的長度
	 * k為CBIN.dat應該寫到第幾個字元
         */
        j = 0;
	lnCBINLength = 0;
        for (i = 0; i < lnDCCBINLength; i++)
        {
                /* 讀完一筆record */
                if (uszTemp[i] == 0x0D && uszTemp[i+1] == 0x0A)
                {
                        /* 把record從temp指定的位置截取出來放到uszReadData */
                        memcpy(&uszReadData[lnCBINLength], &uszTemp[i-j], j);
			
                        /* 為了跳過 0x0D 0x0A */
                        i = i + 2;
                        /* 每讀完一筆record，j就歸0 */
			lnCBINLength += j;
                        j = 0;
                }

                j ++;
        }
	
	/* release */
        /* 關檔 */
        inFILE_Close(&ulFile_Handle);

        /* Free pointer */
        free(uszTemp);
	
	if (inFILE_Check_Exist((unsigned char *)_NCCC_DCC_FILE_CARD_BIN_) != VS_SUCCESS)
	{
		
	}
	else
	{
		inFILE_Delete((unsigned char *)_NCCC_DCC_FILE_CARD_BIN_);
	}
	
	if (inFILE_Create(&ulFile_Handle, (unsigned char *)_NCCC_DCC_FILE_CARD_BIN_) == VS_ERROR)
        {
                /* 開檔失敗 ，不用關檔 */
                /* 開檔失敗，所以回傳error */
		free(uszReadData);
		
                return (VS_ERROR);
        }
	
	/* seek 到檔案開頭 & 從檔案開頭開始read */
        if (inFILE_Seek(ulFile_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                if (inFILE_Write(&ulFile_Handle, uszReadData, lnCBINLength) == VS_SUCCESS)
		{
			
		}
		/* 讀失敗時 */
		else
		{
			/* Close檔案 */
			inFILE_Close(&ulFile_Handle);

			/* Free pointer */
			free(uszReadData);
			free(uszTemp);

			return (VS_ERROR);
		}
		
        }
        /* seek不成功時 */
        else
        {
                /* 關檔並回傳 */
                inFILE_Close(&ulFile_Handle);
                /* Free pointer */
                free(uszReadData);

                /* Seek失敗，所以回傳Error */
                return (VS_ERROR);
        }
	
	/* release */
        /* 關檔 */
        inFILE_Close(&ulFile_Handle);

        /* Free pointer */
	free(uszReadData);
	
	/* 轉完刪掉DCCCBIN.txt */
	inFILE_Delete((unsigned char *)_NCCC_DCC_FILE_CARD_BIN_);
	
	/* inFunc_DCCBin_Transfer() END */
        if (ginDebug == VS_TRUE)
        {
                memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                sprintf(szDebugMsg, "inNCCC_DCC_DCCBin_Transfer END!!");
                inLogPrintf(AT, szDebugMsg);
                inLogPrintf(AT, "----------------------------------------");
        }
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_DCC_ChangeToTWD_For_SALE
Date&Time       :2016/9/13 下午 3:25
Describe        :DCC一般交易 外幣轉台幣
*/
int inNCCC_DCC_ChangeToTWD_For_SALE(TRANSACTION_OBJECT *pobTran)
{
	int	inNCCCHostIndex = -1;
	char	szBatchNum [6 + 1], szInvoiceNum [6 + 1];
	
	/* 【步驟一】取消DCC SALE */
	pobTran->srBRec.uszOfflineBit = VS_TRUE;
	pobTran->srBRec.uszUpload1Bit = VS_FALSE;	/* 不會產生任何 Advice */
	pobTran->srBRec.uszUpload2Bit = VS_FALSE;	/* 不會產生任何 Advice */
	pobTran->srBRec.uszUpload3Bit = VS_FALSE;	/* 不會產生任何 Advice */
	pobTran->uszUpdateBatchBit = VS_TRUE;
	
	pobTran->inTransactionCode = _VOID_;
	pobTran->srBRec.inCode = _VOID_;
	pobTran->srBRec.inOrgCode = _SALE_;
	pobTran->srBRec.uszVOIDBit = VS_TRUE;
	pobTran->srBRec.uszNCCCDCCRateBit = VS_TRUE;	/* 有詢價過轉台幣 */
	
	/* 離線取消，沒有上傳簽單 */
	pobTran->srBRec.inESCUploadMode = _ESC_STATUS_NOT_SUPPORTED_;
	
	memset(szInvoiceNum, 0x00, sizeof(szInvoiceNum));
        inGetInvoiceNum(szInvoiceNum);
	/* 因為現在轉台幣流程放在update Invoice Number後面，所以要自己減一 */
        pobTran->srBRec.lnOrgInvNum = atol(szInvoiceNum) - 1;
	
	/* 2013-02-18 AM 11:41:48 add by kakab 修改一段式DCC晶片卡交易，當DCC晶片卡交易TC Upload失敗後，轉台幣交易程式在處理此種情形時邏輯上有疏失導致帳務問題
	 * 這裡修正若原先TCUpload失敗，會先存advice，然後在轉台幣流程會再存一筆取消的advice，但因為pobTran->uszUpdateBatchBit = VS_TRUE，所以會覆蓋到原先需補送TCUpload的電文狀態，
	 * 在此情況的解決方法:不再補送TCUpload，直接把補送TC的advice狀態轉成送取消
	 */
	if (pobTran->srBRec.inChipStatus == _EMV_CARD_ && pobTran->srBRec.uszTCUploadBit == VS_FALSE)
	{
		/* 直接當已送過TC，所以STAN往後跳 */
		pobTran->srBRec.uszTCUploadBit = VS_TRUE;
		memset(pobTran->srBRec.szRespCode, 0x00, sizeof(pobTran->srBRec.szRespCode));
		strcpy(pobTran->srBRec.szRespCode, "00");
		pobTran->srBRec.szRespCode[2] = 0x00;
	}
	else
	{
        	if (inFLOW_RunFunction(pobTran, _NCCC_DCC_FUNCTION_BUILD_AND_SEND_PACKET_) != VS_SUCCESS)
			return (VS_ERROR);
        }
	
	if (inFLOW_RunFunction(pobTran, _UPDATE_ACCUM_) != VS_SUCCESS)
		return (VS_ERROR);
	
	if (inFLOW_RunFunction(pobTran, _UPDATE_BATCH_) != VS_SUCCESS)
		return (VS_ERROR);
	
	/* 【步驟二】轉換主機 */
	if (inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_CREDIT_NCCC_, &inNCCCHostIndex) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	pobTran->srBRec.inHDTIndex = inNCCCHostIndex;
	
	if (inLoadHDTRec(pobTran->srBRec.inHDTIndex) < 0)
		return (VS_ERROR);
	
	if (inLoadHDPTRec(pobTran->srBRec.inHDTIndex) < 0)
		return (VS_ERROR);
	
	/* NCCC和DCC的Batch Number基本上相同，invoice 會在_FUNCTION_UPDATE_INV_更新, 但為了以防萬一還是更新BatchNum和invoiceNum */
	inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
        memset(szBatchNum, 0x00, sizeof(szBatchNum));
        inGetBatchNum(szBatchNum);
        pobTran->srBRec.lnBatchNum = atol(szBatchNum);

        memset(szInvoiceNum, 0x00, sizeof(szInvoiceNum));
        inGetInvoiceNum(szInvoiceNum);
	/* 因為現在轉台幣流程放在update Invoice Number後面，所以要自己減一 */
        pobTran->srBRec.lnOrgInvNum = atol(szInvoiceNum) - 1;
	
	/* 【步驟三】信用卡 Offline SALE */
	pobTran->srBRec.uszDCCTransBit = VS_FALSE;
	pobTran->srBRec.uszOfflineBit = VS_TRUE;
	pobTran->srBRec.uszUpload1Bit = VS_TRUE;	/* 不會產生任何 Advice */
	pobTran->srBRec.uszUpload2Bit = VS_FALSE;	/* 不會產生任何 Advice */
	pobTran->srBRec.uszUpload3Bit = VS_FALSE;	/* 不會產生任何 Advice */
	pobTran->uszUpdateBatchBit = VS_FALSE;
	pobTran->inTransactionCode = _SALE_OFFLINE_;
	pobTran->srBRec.inCode = _SALE_OFFLINE_;
	pobTran->srBRec.inOrgCode = _SALE_OFFLINE_;
	pobTran->srBRec.uszVOIDBit = VS_FALSE;
	pobTran->srBRec.uszNCCCDCCRateBit = VS_TRUE;	/* 有詢價過轉台幣 */
	
	if (inFLOW_RunFunction(pobTran, _NCCC_ESC_CHECK_) != VS_SUCCESS)
		return (VS_ERROR);
	
	if (inFLOW_RunFunction(pobTran, _NCCC_FUNCTION_BUILD_AND_SEND_PACKET_FLOW_) != VS_SUCCESS)
		return (VS_ERROR);
	
	if (inFLOW_RunFunction(pobTran, _UPDATE_ACCUM_) != VS_SUCCESS)
		return (VS_ERROR);
	
	if (inFLOW_RunFunction(pobTran, _UPDATE_BATCH_) != VS_SUCCESS)
		return (VS_ERROR);
	
	return (VS_SUCCESS);
	
}

/*
Function        :inNCCC_DCC_ChangeToTWD_For_PRE_COMP
Date&Time       :2016/10/4 上午 10:43
Describe        :預先授權完成 外幣轉台幣
*/
int inNCCC_DCC_ChangeToTWD_For_PRE_COMP(TRANSACTION_OBJECT *pobTran)
{
	int	inNCCCHostIndex = -1;
	char	szBatchNum [6 + 1], szInvoiceNum [6 + 1];
	
	/* 【步驟一】取消PRE COMP */
	pobTran->srBRec.uszOfflineBit = VS_TRUE;
	pobTran->srBRec.uszUpload1Bit = VS_FALSE;	/* 不會產生任何 Advice */
	pobTran->srBRec.uszUpload2Bit = VS_FALSE;	/* 不會產生任何 Advice */
	pobTran->srBRec.uszUpload3Bit = VS_FALSE;	/* 不會產生任何 Advice */
	pobTran->uszUpdateBatchBit = VS_TRUE;
	
	pobTran->inTransactionCode = _VOID_;
	pobTran->srBRec.inCode = _VOID_;
	pobTran->srBRec.inOrgCode = _PRE_COMP_;
	pobTran->srBRec.uszVOIDBit = VS_TRUE;
	pobTran->srBRec.uszNCCCDCCRateBit = VS_TRUE;	/* 有詢價過轉台幣 */
	
	/* 離線取消，沒有上傳簽單 */
	pobTran->srBRec.inESCUploadMode = _ESC_STATUS_NOT_SUPPORTED_;
	
	/* 因DCC預先授權完成只能刷卡，所以無需像DCC一般交易轉台幣一樣，做TCUpload的特殊處理 */
	if (inFLOW_RunFunction(pobTran, _NCCC_DCC_FUNCTION_BUILD_AND_SEND_PACKET_) != VS_SUCCESS)
		return (VS_ERROR);
	
	if (inFLOW_RunFunction(pobTran, _UPDATE_ACCUM_) != VS_SUCCESS)
		return (VS_ERROR);
	
	if (inFLOW_RunFunction(pobTran, _UPDATE_BATCH_) != VS_SUCCESS)
		return (VS_ERROR);
	
	/* 【步驟二】轉換主機 */
	if (inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_CREDIT_NCCC_, &inNCCCHostIndex) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	pobTran->srBRec.inHDTIndex = inNCCCHostIndex;
	
	if (inLoadHDTRec(pobTran->srBRec.inHDTIndex) < 0)
		return (VS_ERROR);
	
	if (inLoadHDPTRec(pobTran->srBRec.inHDTIndex) < 0)
		return (VS_ERROR);
	
	/* NCCC和DCC的Batch Number基本上相同，invoice 會在_FUNCTION_UPDATE_INV_更新, 但為了以防萬一還是更新BatchNum和invoiceNum */
	inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
        memset(szBatchNum, 0x00, sizeof(szBatchNum));
        inGetBatchNum(szBatchNum);
        pobTran->srBRec.lnBatchNum = atol(szBatchNum);

        memset(szInvoiceNum, 0x00, sizeof(szInvoiceNum));
        inGetInvoiceNum(szInvoiceNum);
	/* 因為現在轉台幣流程放在update Invoice Number後面，所以要自己減一 */
        pobTran->srBRec.lnOrgInvNum = atol(szInvoiceNum) - 1;
	
	/* 【步驟三】信用卡 Offline PRE_COMP */
	pobTran->srBRec.uszDCCTransBit = VS_FALSE;
	pobTran->srBRec.uszOfflineBit = VS_TRUE;
	pobTran->srBRec.uszUpload1Bit = VS_TRUE;	/* 不會產生任何 Advice */
	pobTran->srBRec.uszUpload2Bit = VS_FALSE;	/* 不會產生任何 Advice */
	pobTran->srBRec.uszUpload3Bit = VS_FALSE;	/* 不會產生任何 Advice */
	pobTran->uszUpdateBatchBit = VS_FALSE;
	pobTran->inTransactionCode = _PRE_COMP_;
	pobTran->srBRec.inCode = _PRE_COMP_;
	pobTran->srBRec.inOrgCode = _PRE_COMP_;
	pobTran->srBRec.uszVOIDBit = VS_FALSE;
	pobTran->srBRec.uszNCCCDCCRateBit = VS_TRUE;	/* 有詢價過轉台幣 */
	
	if (inFLOW_RunFunction(pobTran, _NCCC_ESC_CHECK_) != VS_SUCCESS)
		return (VS_ERROR);
	
	if (inFLOW_RunFunction(pobTran, _NCCC_FUNCTION_BUILD_AND_SEND_PACKET_FLOW_) != VS_SUCCESS)
		return (VS_ERROR);
	
	if (inFLOW_RunFunction(pobTran, _UPDATE_ACCUM_) != VS_SUCCESS)
		return (VS_ERROR);
	
	if (inFLOW_RunFunction(pobTran, _UPDATE_BATCH_) != VS_SUCCESS)
		return (VS_ERROR);
	
	return (VS_SUCCESS);
	
}

/*
Function        :inNCCC_DCC_Transacton_Origin_Check
Date&Time       :2016/9/21 下午 4:46
Describe        :原交易是否使用外幣支付，是按0，否按9
*/
int inNCCC_DCC_Transacton_Origin_Check(TRANSACTION_OBJECT *pobTran)
{
	char	szKey;
        
        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_Transacton_Origin_Check START!");
	
	/* 不是可做DCC的卡 */
	if (pobTran->srBRec.uszDCCTransBit != VS_TRUE)
	{
		return (VS_SUCCESS);
	}
	
	inDISP_PutGraphic(_DCC_ORIGIN_CHECK_, 0, _COORDINATE_Y_LINE_8_4_);
		
	while (1)
	{
		szKey = uszKBD_GetKey(30);

		if (szKey == _KEY_0_)
		{
			pobTran->srBRec.uszDCCTransBit = VS_TRUE;
			
			return (VS_SUCCESS);
		}
		else if (szKey == _KEY_9_)
		{
			pobTran->srBRec.uszDCCTransBit = VS_FALSE;
			
			return (VS_SUCCESS);
		}
		else if (szKey == _KEY_CANCEL_)
		{
			return (VS_ERROR);
		}
		else if (szKey == _KEY_TIMEOUT_)
		{
			return (VS_TIMEOUT);
		}

	}
		
}

/*
Function        :inNCCC_DCC_Transacton_Check
Date&Time       :2016/9/22 下午 3:43
Describe        :是否選擇外幣支付，是按0，否按9
*/
int inNCCC_DCC_Transacton_Check(TRANSACTION_OBJECT *pobTran)
{
	char	szKey;
	
	/* 不是可做DCC的卡 */
	if (pobTran->srBRec.uszDCCTransBit != VS_TRUE)
	{
		return (VS_SUCCESS);
	}
	
	inDISP_PutGraphic(_DCC_CHECK_, 0, _COORDINATE_Y_LINE_8_4_);
		
	while (1)
	{
		szKey = uszKBD_GetKey(30);

		if (szKey == _KEY_0_)
		{
			pobTran->srBRec.uszDCCTransBit = VS_TRUE;
			
			return (VS_SUCCESS);
		}
		else if (szKey == _KEY_9_)
		{
			pobTran->srBRec.uszDCCTransBit = VS_FALSE;
			
			return (VS_SUCCESS);
		}
		else if (szKey == _KEY_CANCEL_)
		{
			return (VS_ERROR);
		}
		else if (szKey == _KEY_TIMEOUT_)
		{
			return (VS_TIMEOUT);
		}

	}
		
}

/*
Function        :inNCCC_DCC_TMSDownload_CheckStatus
Date&Time       :2016/10/24 下午 1:40
Describe        :DCC參數下載 並設定參數狀態，若inNCCC_DCC_TMSDownload回傳VS_ERROR之後的設定參數在這邊
*/
int inNCCC_DCC_TMSDownload_CheckStatus(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;
	
	inRetVal = inNCCC_DCC_TMSDownload(pobTran);
	
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	else
	{
		/* 下載成功 */
		/* 成功就把狀態設定回來 */
		/* 下載成功就設為0 */
		inLoadEDCRec(0);
		inSetDCCInit("0");
		inSetDCCDownloadMode(_NCCC_DCC_DOWNLOAD_MODE_NOARMAL_);
		inSaveEDCRec(0);
	
		return (VS_SUCCESS);
	}
}

/*
Function        :inNCCC_DCC_TMSDownload
Date&Time       :2016/9/13 上午 10:22
Describe        :DCC參數下載
*/
int inNCCC_DCC_TMSDownload(TRANSACTION_OBJECT *pobTran)
{
	int		i;
        int		inRetVal;
	char		szTMSDownloadTimes[6 + 1] = {0};
	unsigned long	ulFile_Handle;
        
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_DCC_TMSDownload() START!");
        
        if (inLoadEDCRec(0) < 0)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inLoadEDCRec() ERROR!");

                return (VS_ERROR);
        }
        
	/* DCC參數手動下載必定塞一，全下 */
	if (pobTran->inRunOperationID == _OPERATION_FUN5_DCC_DOWNLOAD_ || pobTran->inRunOperationID == _OPERATION_FUN6_DCC_DOWNLOAD_)
	{
		memset(gsrDCC_Download.szDCC_Install, _DCC_INSTALL_INDICATOR_NEW_INSTALL_11BIN_, 1);
	}
	else
	{
		memset(szTMSDownloadTimes, 0x00, sizeof(szTMSDownloadTimes));
		inGetTMSDownloadTimes(szTMSDownloadTimes);
		/* 新裝機 */
		/* 【需求單 - 106252】	DCC交易新增感應及BIN11碼需求 by Russell 2020/1/14 下午 3:37 */
		if (atoi(szTMSDownloadTimes) == 1 /* newly installed */	||
		    inNCCC_DCC_Already_Have_Parameter(pobTran) != VS_TRUE /* No Parameter */)	
		{
			memset(gsrDCC_Download.szDCC_Install, _DCC_INSTALL_INDICATOR_NEW_INSTALL_11BIN_, 1);
		}
		else
		{
			memset(gsrDCC_Download.szDCC_Install, _DCC_INSTALL_INDICATOR_NOT_NEW_INSTALL_11BIN_, 1);
		}
	}
		
	/* BinVersion(_DCC_UPDATE_INF_要用) */
        if (inGetDCCBinVer(gsrDCC_Download.szDCC_BINVersion) != VS_SUCCESS)
		return (VS_ERROR);
        
	inNCCC_DCC_GetSTAN(pobTran);
	/* 處理 Table G _DCC_UPDATE_INF_ */
	pobTran->inISOTxnCode = _DCC_UPDATE_INF_;
	/* 組電文 */
	inRetVal = inNCCC_DCC_SendPackRecvUnPack(pobTran);
	inNCCC_DCC_SetSTAN(pobTran);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "inNCCC_DCC_SendPackRecvUnPack() ERROR!");

		return (VS_ERROR);
	}
	
	/* 分析並存檔案 */
	if (inNCCC_DCC_SaveParameter(pobTran, 0) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	
	/* 處理 Table H（分841（Bin Table）和842（Parameter）） */
	pobTran->inISOTxnCode = _DCC_BIN_;
	
	/* 表示要下載Bin Table(841) */
	if (gsrDCC_Download.szDCC_FileList[0] == '1')
	{
		/* 開新檔案用來儲存DCC參數 */
		/* 先刪檔，以確保創檔成功 */
		inFILE_Delete((unsigned char *)_NCCC_DCC_FILE_CARD_BIN_TEMP_);
		inRetVal = inFILE_Create(&ulFile_Handle, (unsigned char *)_NCCC_DCC_FILE_CARD_BIN_TEMP_);

		if (inRetVal == VS_ERROR)
		{
			if (ginDebug == VS_TRUE)
				inLogPrintf(AT, "inFILE_Create(DCCCBin_T.txt) ERROR!");

			/* 開失敗不用關 */
			return (VS_ERROR);
		}
		
		
		memcpy(gsrDCC_Download.szDCC_FC, "841", 3);
		/* 做到剩餘長度為0 */
		do
		{
			/* 組電文 */
			inNCCC_DCC_GetSTAN(pobTran);
			inRetVal = inNCCC_DCC_SendPackRecvUnPack(pobTran);
			inNCCC_DCC_SetSTAN(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				if (ginDebug == VS_TRUE)
					inLogPrintf(AT, "inNCCC_DCC_SendPackRecvUnPack() ERROR!");

				inFILE_Close(&ulFile_Handle);

				return (VS_ERROR);
			}

			/* 分析並存檔案 */
			if (inNCCC_DCC_SaveParameter(pobTran, &ulFile_Handle) != VS_SUCCESS)
			{
				return (VS_ERROR);
			}
			
		} while (memcmp(gsrDCC_Download.szDCC_FTREDRC, "000000", 6) != 0);
		
		inFILE_Close(&ulFile_Handle);
		
	}
	
	/* 看需不需要下載Parameter */
	for (i = 1; i < 8; i++) 
	{
		/* 代表參數部份需要下載 */
		if (gsrDCC_Download.szDCC_FileList[i] == '1')
		{
			/* 開新檔案用來儲存DCC參數 */
			/* 先刪檔，以確保創檔成功 */
			inFILE_Delete((unsigned char *)_NCCC_DCC_FILE_PARA_);
			inRetVal = inFILE_Create(&ulFile_Handle, (unsigned char *)_NCCC_DCC_FILE_PARA_);

			if (inRetVal == VS_ERROR)
			{
				if (ginDebug == VS_TRUE)
					inLogPrintf(AT, "inFILE_Create(DCCCBin_T.txt) ERROR!");

				/* 開失敗不用關 */
				return (VS_ERROR);
			}
		
			memcpy(gsrDCC_Download.szDCC_FC, "842", 3);
			/* 只要確認Parameter任一檔案需下載，就好了 */
			/* 做到剩餘長度為0 */
			do
			{
				/* 組電文 */
				inNCCC_DCC_GetSTAN(pobTran);
				inRetVal = inNCCC_DCC_SendPackRecvUnPack(pobTran);
				inNCCC_DCC_SetSTAN(pobTran);
				if (inRetVal != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
						inLogPrintf(AT, "inNCCC_DCC_SendPackRecvUnPack() ERROR!");

				inFILE_Close(&ulFile_Handle);

					return (VS_ERROR);
				}

				/* 分析並存檔案 */
				if (inNCCC_DCC_SaveParameter(pobTran, &ulFile_Handle) != VS_SUCCESS)
				{
					return (VS_ERROR);
				}

			} while (memcmp(gsrDCC_Download.szDCC_FTREDRC, "000000", 6) != 0);
			
			inFILE_Close(&ulFile_Handle);
		
			break;
		}

	}
	
	/* 分割檔案並整理下載下來的檔案 */
	inRetVal = inNCCC_DCC_CutAndFormat_TempFile();
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	/* 處理 Table I _DCC_UPDATE_CLOSE_ */
	pobTran->inISOTxnCode = _DCC_UPDATE_CLOSE_;
	/* 組電文 */
	inNCCC_DCC_GetSTAN(pobTran);
	inRetVal = inNCCC_DCC_SendPackRecvUnPack(pobTran);
	inNCCC_DCC_SetSTAN(pobTran);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "inNCCC_DCC_SendPackRecvUnPack() ERROR!");

		return (VS_ERROR);
	}
			
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_DCC_TMSDownload() END!");
        
        return (VS_SUCCESS);
}

/*
Function        :inNCCC_DCC_SaveParameter
Date&Time       :2016/10/5 下午 3:29
Describe        :
 *		“841”:BIN Table Data
 *		“842”:Parameter Data
*/
int inNCCC_DCC_SaveParameter(TRANSACTION_OBJECT *pobTran, unsigned long* ulFile_Handle)
{
        int     i = 0;
        
	if (pobTran->inISOTxnCode == _DCC_UPDATE_INF_)
        {
		/* 沒有需下載的檔案 */
                if (gsrDCC_Download.szDCC_AP[0] == 0x00)
                {
			/* 全設為0代表全部不下載 */
			memset(gsrDCC_Download.szDCC_FileList, '0', 7);
			
                        return (VS_SUCCESS);
                }
		else
		{
			for (i = 0; i < 8; i ++)
			{
				/* 256向右位移7位等於1，以此類推 */
				if ((gsrDCC_Download.szDCC_AP[0] >> ( 7 - i ) & 1) == 1)
				{
					memset(&gsrDCC_Download.szDCC_FileList[i], '1', 1);
				}
				/* 要設為0 */
				else
				{
					memset(&gsrDCC_Download.szDCC_FileList[i], '0', 1);
				}
				
			}
		}
		         
        }
        else if (pobTran->inISOTxnCode == _DCC_BIN_)
        {
		/* _DCC_BIN_用到時，必須保證_DCC_BIN_不為0 */
		if (ulFile_Handle == 0)
		{
			return (VS_ERROR);
		}
		
                if (inNCCC_DCC_CheckSumData(gsrDCC_Download.uszDCC_Record) != VS_SUCCESS)    /* 確認檢查碼 */
                        return (VS_ERROR);

		/* 前2byte是長度，最後1byte是CheckSum */
                inFILE_Write(ulFile_Handle,  &gsrDCC_Download.uszDCC_Record[2], (gsrDCC_Download.ulDCC_BINLen - 3));

		/* 在外面判斷是否已收完 */
        }
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_DCC_CheckSumData
Date&Time       :2016/10/6 下午 1:39
Describe        :DCC TMS F_59 Table H Data_Record的CheckSum
 *		以XOR對Data每一個Byte進行運算，運算最後所得之值即CheckSum。 (運算Data不含2bytes的Data Record Length)
*/
int inNCCC_DCC_CheckSumData(unsigned char *uszDataRecord)
{
        int     i = 0, inLen = 0;
	char	szChecksum = 0;
	char	szCalCheckSum = 0;

        inLen = (uszDataRecord[0] % 16) * 100 + (uszDataRecord[1] / 16) * 10 + uszDataRecord[1] % 16;
	szChecksum = uszDataRecord[1 + inLen];

        for (i = 2; i <= 1 + (inLen - 1); i ++)
        {
                szCalCheckSum = szCalCheckSum ^ uszDataRecord[i];
        }

        if (szCalCheckSum == szChecksum)
        {
                return (VS_SUCCESS);
        }
        else
        {
                return (VS_ERROR);
        }
}

/*
Function        :inNCCC_DCC_CutAndFormat_TempFile
Date&Time       :2016/10/6 下午 1:35
Describe        :把DCCCBin_T.dat和DCCPara_T.dat切割並轉換
*/
int inNCCC_DCC_CutAndFormat_TempFile()
{
	/* 是否有下BinTable */
	if (gsrDCC_Download.szDCC_FileList[0] ==  '1')
	{
		if (inNCCC_DCC_Format_DCCBin_TempFile() == VS_SUCCESS)
		{
			/* 轉換成功才更改BinVersion */
			inSetDCCBinVer(gsrDCC_Download.szDCC_BINVersion);
			inSaveEDCRec(0);
		}
		else
		{
			/* 轉換失敗繼續處理其他檔案（不跳出） */
		}
	}
	
	/* 是否有下參數 */
	/* 看需不需要處裡Parameter暫存檔 */
	/* 代表參數部份需要處裡Parameter暫存檔 */
	/* Ex:szDCC_FileList = "01000000" */
	if (atoi(&gsrDCC_Download.szDCC_FileList[1]) != 0)
	{
		inNCCC_DCC_Cut_ParameterFile();
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_DCC_Format_DCCBin_TempFile
Date&Time       :2016/10/6 下午 1:35
Describe        :把DCCCBin_T.dat轉換
*/
int inNCCC_DCC_Format_DCCBin_TempFile()
{
	int		inCnt = 0;
	long		lnTempSize = 0;			/* DCCBin_T.dat的長度 */
	long		lnFinalSize = 0;		/* DCCBin.dat的長度 */
	long		lnReadLength;			/* 剩餘DCCBin_T.dat的長度 */
	long		lnBinSize = 0;			/* Length for uncompressed field */	/* 【需求單 - 106252】	DCC交易新增感應及BIN11碼需求 by Russell 2020/1/14 下午 3:37 */
	long		lnBinHexSize = 0;	/* Length for compressed field */
	char		*szDCCBIN;			/* 暫存，放整筆DCCCBin.dat檔案 */
	char		szDebugMsg[100 + 1];
	unsigned char   uszTemp[lnBinHexSize + 1];	/* 暫存，放一筆DCCCBin */
	unsigned long	ulFile_Handle = 0;
	
	
	if (inFILE_OpenReadOnly(&ulFile_Handle, (unsigned char*)_NCCC_DCC_FILE_CARD_BIN_TEMP_) != VS_SUCCESS)
	{
		/* 開失敗，繼續處理其他檔案，但DownloadStatus就不設為成功 */
		return (VS_ERROR);
	}
	
	lnTempSize = lnFILE_GetSize(&ulFile_Handle, (unsigned char*)_NCCC_DCC_FILE_CARD_BIN_TEMP_);
	if (lnTempSize == VS_ERROR)
	{
		/* GetSize失敗 ，關檔 */
		inFILE_Close(&ulFile_Handle);
		
		return (VS_ERROR);
	}
	
	/* 根據install indicator決定是6碼還是5碼 */
	if (gsrDCC_Download.szDCC_Install[0] == _DCC_INSTALL_INDICATOR_NOT_NEW_INSTALL_11BIN_ ||
	    gsrDCC_Download.szDCC_Install[0] == _DCC_INSTALL_INDICATOR_NEW_INSTALL_11BIN_)
	{
		lnBinSize = 12;
	}
	else if (gsrDCC_Download.szDCC_Install[0] == _DCC_INSTALL_INDICATOR_NOT_NEW_INSTALL_ ||
		 gsrDCC_Download.szDCC_Install[0] == _DCC_INSTALL_INDICATOR_NEW_INSTALL_)
	{
		lnBinSize = 10;
	}
	else
	{
		lnBinSize = 12;
	}
	lnBinHexSize = (lnBinSize / 2);
	
	/* lnFinalSize是轉回ascii並加上斷行的檔案長度 */
	/*	       ASCII BinSize     Bin Cnt        * (Size of 0x0D 0x0A) */
	lnFinalSize = lnTempSize * 2 + (lnTempSize / lnBinHexSize) * 2;
	szDCCBIN = malloc(lnFinalSize + 1);
	
	 /* 初始化 uszTemp */
	memset(uszTemp, 0x00, lnBinHexSize + 1);
	memset(szDCCBIN, 0x00, lnFinalSize + 1);
	
	/* seek 到檔案開頭 & 從檔案開頭開始read */
        if (inFILE_Seek(ulFile_Handle, 0, _SEEK_BEGIN_) != VS_SUCCESS)
        {
		/* seek不成功時 */
                /* 關檔並回傳 */
                inFILE_Close(&ulFile_Handle);
                /* Free pointer */
                free(szDCCBIN);

                /* Seek失敗，所以回傳Error */
                return (VS_ERROR);
        }
	
	lnReadLength = lnTempSize;

	while(lnReadLength > 0)
	{
		memset(uszTemp, 0x00, lnBinHexSize + 1);
		if (inFILE_Read(&ulFile_Handle, uszTemp, lnBinHexSize) == VS_SUCCESS)
		{
			/* 轉Ascii */
			inFunc_BCD_to_ASCII((char*)&szDCCBIN[inCnt], uszTemp, lnBinHexSize);
			inCnt += lnBinSize;
			
			memset(&szDCCBIN[inCnt], 0x0D, 1);
			inCnt ++;
			memset(&szDCCBIN[inCnt], 0x0A, 1);
			inCnt ++;
			
			/* 減掉讀出長度 */
			lnReadLength -= lnBinHexSize;

			/* 當剩餘長度剛好為lnBinHexSize，會剛好讀完 */
			if (lnReadLength == 0)
				break;
		}
		/* 讀失敗時 */
		else
		{
			/* Close檔案 */
			inFILE_Close(&ulFile_Handle);

			/* Free pointer */
			free(szDCCBIN);

			return (VS_ERROR);
		}
	}
	
	/* 先關檔 */
	inFILE_Close(&ulFile_Handle);
	inFILE_Delete((unsigned char*)_NCCC_DCC_FILE_CARD_BIN_TEMP_);
	
	if (inFILE_Create(&ulFile_Handle, (unsigned char*)_NCCC_DCC_FILE_CARD_BIN_TEMP_2_) != VS_SUCCESS)
	{
		/* 開失敗，繼續處理其他檔案，但DownloadStatus就不設為成功 */
		/* Free pointer */
                free(szDCCBIN);
		
		return (VS_ERROR);
	}
	
	/* seek 到檔案開頭 & 從檔案開頭開始read */
        if (inFILE_Seek(ulFile_Handle, 0, _SEEK_BEGIN_) != VS_SUCCESS)
        {
		/* seek不成功時 */
                /* 關檔並回傳 */
                inFILE_Close(&ulFile_Handle);
                /* Free pointer */
                free(szDCCBIN);

                /* Seek失敗，所以回傳Error */
                return (VS_ERROR);
        }
	
	if (inFILE_Write(&ulFile_Handle, (unsigned char*)szDCCBIN, lnFinalSize) != VS_SUCCESS)
	{
		/* Free pointer */
		free(szDCCBIN);

		return (VS_ERROR);
	}
	
	/* Free pointer */
	free(szDCCBIN);
	/* 先關檔 */
	inFILE_Close(&ulFile_Handle);
	
	/* 先刪原檔 */
	inFILE_Delete((unsigned char*)_NCCC_DCC_FILE_CARD_BIN_);
	
	/* 改檔名 */
	if (inFILE_Rename((unsigned char*)_NCCC_DCC_FILE_CARD_BIN_TEMP_2_, (unsigned char*)_NCCC_DCC_FILE_CARD_BIN_) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inNCCC_DCC_Format_DCCBin_TempFile 改檔失敗");
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	/* 成功 */
	else
	{
		memset(&gsrDCC_Download.szDCC_FileDownloadStaus[0], '1', 1);
		
		return (VS_SUCCESS);
	}
}

/*
Function        :inNCCC_DCC_Cut_ParameterFile
Date&Time       :2016/10/6 下午 3:09
Describe        :
*/
int inNCCC_DCC_Cut_ParameterFile()
{
	int		i = 0;
	int		inCnt = 0;
	int		inRetVal = VS_SUCCESS;
	long		lnTempSize = 0;			/* DCCBin_T.dat的長度 */
	long		lnReadLength;			/* 剩餘DCCBin_T.dat的長度 */
	char		*szDCCParameter;		/* 暫存，放整筆DCCCBin.dat檔案 */
	char		szDCCFlowVersion[1 + 1] = {0};
	char		szDebugMsg[100 + 1] = {0};
	char		szDemoMode[1 + 1] = {0};
	unsigned long	ulFile_Handle = 0;
	
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	
	if (inFILE_OpenReadOnly(&ulFile_Handle, (unsigned char*)_NCCC_DCC_FILE_PARA_) != VS_SUCCESS)
	{
		/* 開失敗，繼續處理其他檔案，但DownloadStatus就不設為成功 */
		return (VS_ERROR);
	}
	
	lnTempSize = lnFILE_GetSize(&ulFile_Handle, (unsigned char*)_NCCC_DCC_FILE_PARA_);
	if (lnTempSize == VS_ERROR)
	{
		/* GetSize失敗 ，關檔 */
		inFILE_Close(&ulFile_Handle);
		
		return (VS_ERROR);
	}
	
	szDCCParameter = malloc(lnTempSize + 1);
	
	 /* 初始化 uszTemp */
	memset(szDCCParameter, 0x00, lnTempSize + 1);
	
	/* seek 到檔案開頭 & 從檔案開頭開始read */
        if (inFILE_Seek(ulFile_Handle, 0, _SEEK_BEGIN_) != VS_SUCCESS)
        {
		/* seek不成功時 */
                /* 關檔並回傳 */
                inFILE_Close(&ulFile_Handle);
                /* Free pointer */
                free(szDCCParameter);

                /* Seek失敗，所以回傳Error */
                return (VS_ERROR);
        }
	
	lnReadLength = lnTempSize;

	while(lnReadLength > 0)
	{
		/* 剩餘長度大於或等於1024 */
		if (lnReadLength >= 1024)
		{
			if (inFILE_Read(&ulFile_Handle, (unsigned char*)&szDCCParameter[inCnt], 1024) == VS_SUCCESS)
			{
				inCnt += 1024;
				/* 減掉讀出長度 */
				lnReadLength -= 1024;

				/* 當剩餘長度剛好為lnBinHexSize，會剛好讀完 */
				if (lnReadLength == 0)
					break;
			}
			/* 讀失敗時 */
			else
			{
				/* Close檔案 */
				inFILE_Close(&ulFile_Handle);

				/* Free pointer */
				free(szDCCParameter);

				return (VS_ERROR);
			}
		}
		/* 剩餘長度小於1024 */
		else if (lnReadLength < 1024)
		{
			/* 就只讀剩餘長度 */
			if (inFILE_Read(&ulFile_Handle, (unsigned char*)&szDCCParameter[inCnt], lnReadLength) == VS_SUCCESS)
			{
				inCnt += lnReadLength;
				/* 減掉讀出長度 */
				lnReadLength -= lnReadLength;
				
				break;
			}
			/* 讀失敗時 */
			else
			{
				/* Close檔案 */
				inFILE_Close(&ulFile_Handle);

				/* Free pointer */
				free(szDCCParameter);

				return (VS_ERROR);
			}
			
		}
		
	}
	
	/* 先關檔 */
	inFILE_Close(&ulFile_Handle);
	inFILE_Delete((unsigned char*)_NCCC_DCC_FILE_PARA_);
	
	/* 初始化*/
	inCnt = 0;
	
	/* 原本只用if，但是會一直忘記同時改szDCC_FileList 和 szDCC_FileDownloadStaus 的index 所以加上 for 和 switch case 來自動化 */
	while (inCnt < lnTempSize)
	{
		/* 看Parameter ID(BCD compressed)是什麼，也可以用for(i= 1; i < 7; i++)的方式來搜尋，但若電文中的Parameter ID不照順序可能會有問題 */
		i = szDCCParameter[inCnt] - 1;
		
		switch (i)
		{
			case 1:
				/* 有沒有Exchange Rate Source */
				if (gsrDCC_Download.szDCC_FileList[i] == '1')
				{
					if (inNCCC_DCC_Make_Parameter_File((unsigned char*)_NCCC_DCC_FILE_EXCHANGE_RATE_SOURCE_, (unsigned char*)&szDCCParameter[inCnt], &inCnt) == VS_SUCCESS)
					{
						memset(&gsrDCC_Download.szDCC_FileDownloadStaus[i], '1', 1);
					}

				}
				/* 理論上不會有此情況，若電文中有該檔案內容，但Filelist沒有紀錄要下載該檔案，則直接跳到下個seperator後面，*/
				else
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "----------------------------------------");
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "Skip Make File:%d", i);
						inLogPrintf(AT, szDebugMsg);
						inLogPrintf(AT, "----------------------------------------");
					}
					
					while (szDCCParameter[inCnt] != 0x1E)
					{
						inCnt ++;
					}
					/* 找到0x1E 的下一個 */
					inCnt ++;
				}
				break;

			case 2:
				/* VISA Disclaimer Text */
				if (gsrDCC_Download.szDCC_FileList[i] == '1')
				{
					if (inNCCC_DCC_Make_Parameter_File((unsigned char*)_NCCC_DCC_FILE_VISA_DISCLAIMER_, (unsigned char*)&szDCCParameter[inCnt], &inCnt) == VS_SUCCESS)
					{
						memset(&gsrDCC_Download.szDCC_FileDownloadStaus[i], '1', 1);
					}

				}
				/* 理論上不會有此情況，若電文中有該檔案內容，但Filelist沒有紀錄要下載該檔案，則直接跳到下個seperator後面，*/
				else
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "----------------------------------------");
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "Skip Make File:%d", i);
						inLogPrintf(AT, szDebugMsg);
						inLogPrintf(AT, "----------------------------------------");
					}
					
					while (szDCCParameter[inCnt] != 0x1E)
					{
						inCnt ++;
					}
					/* 找到0x1E 的下一個 */
					inCnt ++;
				}
				break;
				
			case 3:
				/* MasterCard Disclaimer */
				if (gsrDCC_Download.szDCC_FileList[i] == '1')
				{
					if (inNCCC_DCC_Make_Parameter_File((unsigned char*)_NCCC_DCC_FILE_MASTERCARD_DISCLAIMER_, (unsigned char*)&szDCCParameter[inCnt], &inCnt) == VS_SUCCESS)
					{
						memset(&gsrDCC_Download.szDCC_FileDownloadStaus[i], '1', 1);
					}

				}
				/* 理論上不會有此情況，若電文中有該檔案內容，但Filelist沒有紀錄要下載該檔案，則直接跳到下個seperator後面，*/
				else
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "----------------------------------------");
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "Skip Make File:%d", i);
						inLogPrintf(AT, szDebugMsg);
						inLogPrintf(AT, "----------------------------------------");
					}
					
					while (szDCCParameter[inCnt] != 0x1E)
					{
						inCnt ++;
					}
					/* 找到0x1E 的下一個 */
					inCnt ++;
				}
				break;
				
			case 4:
				/* VISA Supported Currency List 和 MasterCard Supported Currency List 因為要生成CCI_V.dat，所以另寫一隻function */
				/* VISA Supported Currency List */
				if (!memcmp(szDemoMode, "Y", 1))
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "----------------------------------------");
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "Demo, Skip Make File:%d", i);
						inLogPrintf(AT, szDebugMsg);
						inLogPrintf(AT, "----------------------------------------");
					}

					while (szDCCParameter[inCnt] != 0x1E)
					{
						inCnt ++;
					}
					/* 找到0x1E 的下一個 */
					inCnt ++;
				}
				else
				{
					if (gsrDCC_Download.szDCC_FileList[i] == '1')
					{
						if (inNCCC_DCC_Make_CCI((unsigned char*)_NCCC_DCC_FILE_VISA_SUPPORTED_CURRENCY_LIST_, (unsigned char*)&szDCCParameter[inCnt], &inCnt) == VS_SUCCESS)
						{
							memset(&gsrDCC_Download.szDCC_FileDownloadStaus[i], '1', 1);
						}

					}
					/* 理論上不會有此情況，若電文中有該檔案內容，但Filelist沒有紀錄要下載該檔案，則直接跳到下個seperator後面，*/
					else
					{
						if (ginDebug == VS_TRUE)
						{
							inLogPrintf(AT, "----------------------------------------");
							memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
							sprintf(szDebugMsg, "Skip Make File:%d", i);
							inLogPrintf(AT, szDebugMsg);
							inLogPrintf(AT, "----------------------------------------");
						}

						while (szDCCParameter[inCnt] != 0x1E)
						{
							inCnt ++;
						}
						/* 找到0x1E 的下一個 */
						inCnt ++;
					}
				}
				break;
				
			case 5:
				/* VISA Supported Currency List 和 MasterCard Supported Currency List 因為要生成CCI_M.dat，所以另寫一隻function */
				/* MasterCard Supported Currency List */
				if (!memcmp(szDemoMode, "Y", 1))
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "----------------------------------------");
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "Demo, Skip Make File:%d", i);
						inLogPrintf(AT, szDebugMsg);
						inLogPrintf(AT, "----------------------------------------");
					}

					while (szDCCParameter[inCnt] != 0x1E)
					{
						inCnt ++;
					}
					/* 找到0x1E 的下一個 */
					inCnt ++;
				}
				else
				{
					if (gsrDCC_Download.szDCC_FileList[i] == '1')
					{
						if (inNCCC_DCC_Make_CCI((unsigned char*)_NCCC_DCC_FILE_MASTERCARD_SUPPORTED_CURRENCY_LIST_, (unsigned char*)&szDCCParameter[inCnt], &inCnt) == VS_SUCCESS)
						{
							memset(&gsrDCC_Download.szDCC_FileDownloadStaus[i], '1', 1);
						}

					}
					/* 理論上不會有此情況，若電文中有該檔案內容，但Filelist沒有紀錄要下載該檔案，則直接跳到下個seperator後面，*/
					else
					{
						if (ginDebug == VS_TRUE)
						{
							inLogPrintf(AT, "----------------------------------------");
							memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
							sprintf(szDebugMsg, "Skip Make File:%d", i);
							inLogPrintf(AT, szDebugMsg);
							inLogPrintf(AT, "----------------------------------------");
						}

						while (szDCCParameter[inCnt] != 0x1E)
						{
							inCnt ++;
						}
						/* 找到0x1E 的下一個 */
						inCnt ++;
					}
				}
				break;
				
			case 6:
				/* DCC FlowVersion */
				if (!memcmp(szDemoMode, "Y", 1))
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "----------------------------------------");
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "Demo, Skip Make File:%d", i);
						inLogPrintf(AT, szDebugMsg);
						inLogPrintf(AT, "----------------------------------------");
					}

					while (szDCCParameter[inCnt] != 0x1E)
					{
						inCnt ++;
					}
					/* 找到0x1E 的下一個 */
					inCnt ++;
				}
				else
				{
					if (gsrDCC_Download.szDCC_FileList[i] == '1')
					{
						/* 跳過 Parameter ID(BCD compressed) */
						inCnt ++;

						memset(szDCCFlowVersion, 0x00, sizeof(szDCCFlowVersion));
						sprintf(szDCCFlowVersion, "%d", szDCCParameter[inCnt]);
						inRetVal = inSetDccFlowVersion(szDCCFlowVersion);
						if (inRetVal != VS_SUCCESS)
						{
							inFunc_EDCLock(AT);

							return (VS_ERROR);
						}
						inRetVal = inSaveCFGTRec(0);
						if (inRetVal != VS_SUCCESS)
						{
							inFunc_EDCLock(AT);

							return (VS_ERROR);
						}
						inCnt ++;

						/* 0x1E */
						inCnt ++;

						memset(&gsrDCC_Download.szDCC_FileDownloadStaus[i], '1', 1);
					}
					/* 理論上不會有此情況，若電文中有該檔案內容，但Filelist沒有紀錄要下載該檔案，則直接跳到下個seperator後面，*/
					else
					{
						if (ginDebug == VS_TRUE)
						{
							inLogPrintf(AT, "----------------------------------------");
							memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
							sprintf(szDebugMsg, "Skip Make File:%d", i);
							inLogPrintf(AT, szDebugMsg);
							inLogPrintf(AT, "----------------------------------------");
						}

						while (szDCCParameter[inCnt] != 0x1E)
						{
							inCnt ++;
						}
						/* 找到0x1E 的下一個 */
						inCnt ++;
					}
				}
				break;
				
			default:
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "----------------------------------------");
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "Skip Make File:%d", i);
					inLogPrintf(AT, szDebugMsg);
					inLogPrintf(AT, "----------------------------------------");
				}

				while (szDCCParameter[inCnt] != 0x1E)
				{
					inCnt ++;
				}
				/* 找到0x1E 的下一個 */
				inCnt ++;
				break;
			
		}
		
	}
	
	free(szDCCParameter);
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_DCC_Make_Parameter_File
Date&Time       :2016/10/6 下午 4:50
Describe        :將從_NCCC_DCC_FILE_PARA_TEMP_分割出的檔案建檔
*/
int inNCCC_DCC_Make_Parameter_File(unsigned char *uszFileName, unsigned char *uszData, int *inCnt)
{
	int		i;
	long		lnFileSize = 0;			/* 檔案內容 */
	char		szDebugMsg[100 + 1];
	char		szSeperator = 0x1E;		/* Record Separator(0x1E) */
	unsigned char*	uszDataStart = uszData + 1;	/*  uszData + 1 是因為要跳過Parameter ID(BCD compressed) */
	unsigned long	ulHandle;
	
	inFILE_Delete((unsigned char*)_NCCC_DCC_FILE_PARA_TEMP_);
	
	if (inFILE_Create(&ulHandle, (unsigned char*)_NCCC_DCC_FILE_PARA_TEMP_) != VS_SUCCESS)
	{
		/* 開失敗，繼續處理其他檔案，但DownloadStatus就不設為成功 */

		return (VS_ERROR);
	}

	/* seek 到檔案開頭 & 從檔案開頭開始read */
	if (inFILE_Seek(ulHandle, 0, _SEEK_BEGIN_) != VS_SUCCESS)
	{
		/* seek不成功時 */
		/* 關檔並回傳 */
		inFILE_Close(&ulHandle);

		/* Seek失敗，所以回傳Error */
		return (VS_ERROR);
	}
	
	/* Parameter ID(BCD compressed) */
	*inCnt += 1;
	/* FileData + 0x1E */
	for (i = 0;; i++)
	{
		if (memcmp(&uszDataStart[i], &szSeperator, 1) == 0)
		{
			/* 數了i + 1個byte，但最後一個byte是Seperator，所以FileSize為i */
			lnFileSize = i;
			*inCnt += lnFileSize + 1;
			break;
		}
		
	}
	
	if (inFILE_Write(&ulHandle, uszDataStart, lnFileSize) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	/* 先刪原檔 */
	inFILE_Delete(uszFileName);
	
	/* 改檔名 */
	if (inFILE_Rename((unsigned char*)_NCCC_DCC_FILE_PARA_TEMP_, uszFileName) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inNCCC_DCC_Make_Parameter_File 改檔失敗");
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	/* 成功 */
	else
	{
		return (VS_SUCCESS);
	}

}

/*
Function        :inNCCC_DCC_Make_CCI
Date&Time       :2016/10/7 下午 1:35
Describe        :直接生成CCI
*/
int inNCCC_DCC_Make_CCI(unsigned char *uszFileName, unsigned char *uszData, int *inCnt)
{
	int		i;
	int		inRecordCnt;			/* 有幾個Record */
	long		lnFileSize = 0;			/*  檔案內容的長度 */
	char		szDebugMsg[100 + 1];
	char		szSeperator = 0x1E;		/* Record Separator(0x1E) */
	char		szRecord[12 + 1];		/* CCI 一個Record的內容 */
	char		szDCCCurrencyName[3 + 1];	/* 貨幣名稱 */
	char		szDCCCurrencyCode[4 + 1];	/* 幣別碼 要多一位放0*/
	unsigned char*	uszDataStart = uszData + 1;	/*  uszData + 1 是因為要跳過Parameter ID(BCD compressed) */
	unsigned long	ulHandle;
	
	inFILE_Delete((unsigned char*)_NCCC_DCC_FILE_PARA_TEMP_);
	
	if (inFILE_Create(&ulHandle, (unsigned char*)_NCCC_DCC_FILE_PARA_TEMP_) != VS_SUCCESS)
	{
		/* 開失敗，繼續處理其他檔案，但DownloadStatus就不設為成功 */

		return (VS_ERROR);
	}

	/* seek 到檔案開頭 & 從檔案開頭開始read */
	if (inFILE_Seek(ulHandle, 0, _SEEK_BEGIN_) != VS_SUCCESS)
	{
		/* seek不成功時 */
		/* 關檔並回傳 */
		inFILE_Close(&ulHandle);

		/* Seek失敗，所以回傳Error */
		return (VS_ERROR);
	}
	
	/* Parameter ID(BCD compressed) */
	*inCnt += 1;
	/* FileData + 0x1E */
	for (i = 0;; i++)
	{
		if (memcmp(&uszDataStart[i], &szSeperator, 1) == 0)
		{
			/* 數了i + 1個byte，但最後一個byte是Seperator，所以FileSize為i */
			lnFileSize = i;
			*inCnt += lnFileSize + 1;
			break;
		}
		
	}
	
	/* 長度2n + 3a 共5 byte Ex: 0x03 0x92 "JPY" */
	inRecordCnt = lnFileSize / 5;
	
	/* i是Current Record index */
	for (i = 0; i < inRecordCnt; i++)
	{
		memset(szDCCCurrencyCode, 0x00, sizeof(szDCCCurrencyCode));
		inFunc_BCD_to_ASCII(szDCCCurrencyCode, (uszDataStart + (5 * i)), 2);
		
		memset(szDCCCurrencyName, 0x00, sizeof(szDCCCurrencyName));
		memcpy(szDCCCurrencyName, (uszDataStart + (5 * i)) + 2, 3);
		
		memset(szRecord, 0x00, sizeof(szRecord));
		/* 因為szDCCCurrencyCode轉ascii前面會多一個零所以從1開始 */
		sprintf(szRecord, "%02d,%s,%s%c%c", i + 1, szDCCCurrencyName, &szDCCCurrencyCode[1], 0x0D, 0x0A);
		
		if (inFILE_Write(&ulHandle, (unsigned char*)szRecord, strlen(szRecord)) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
	}
	
	
	/* 先刪原檔 */
	inFILE_Delete(uszFileName);
	
	/* 改檔名 */
	if (inFILE_Rename((unsigned char*)_NCCC_DCC_FILE_PARA_TEMP_, uszFileName) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inNCCC_DCC_Make_Parameter_File 改檔失敗");
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	/* 成功 */
	else
	{
		return (VS_SUCCESS);
	}

}

/*
Function        :inNCCC_DCC_Fun6_ExchangeRateDownload
Date&Time       :2016/10/19 上午 10:28
Describe        :功能6 DCC匯率下載
*/
int inNCCC_DCC_Fun6_ExchangeRateDownload(TRANSACTION_OBJECT *pobTran)
{
	int		inDCCindex = -1;
	int		i = 0, inRetVal = VS_ERROR;
	int		j = 0;
	char		szDCCEnable[2 + 1] = {0};
	char		szDCCFlowVersion[2 + 1] = {0};
	char		szDemoMode[2 + 1] = {0};
	char		szPrintBuf[42 + 1] = {0}, szTemplate[42] = {0};
	RTC_NEXSYS	srRTC;							/* Date & Time */
	unsigned char	uszBuffer[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	BufferHandle	srBhandle;
	FONT_ATTRIB	srFont_Attrib;

        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_Fun6_ExchangeRateDownload START!");
        
	/* 不支援 DCC 時，EDC 顯示此功能已關閉 */
	memset(szDCCEnable, 0x00, sizeof(szDCCEnable));
	if (inNCCC_DCC_GetDCC_Enable(pobTran->srBRec.inHDTIndex, szDCCEnable) != VS_SUCCESS)
	{
		/* 找不到DCC也會回傳 ERROR */
		/* 此功能已關閉 */
		pobTran->inErrorMsg = _ERROR_CODE_V3_FUNC_CLOSE_;
		
		return (VS_ERROR);
	}
	/* DCC 沒開 */
	if (memcmp(szDCCEnable, "Y", 1) != 0)
	{
		/* 此功能已關閉 */
		pobTran->inErrorMsg = _ERROR_CODE_V3_FUNC_CLOSE_;
		
		return (VS_ERROR);
	}
	
	/* DCC 沒開 */
	memset(szDCCFlowVersion, 0x00, sizeof(szDCCFlowVersion));
	inGetDccFlowVersion(szDCCFlowVersion);
	if (memcmp(szDCCFlowVersion, _NCCC_DCC_FLOW_VER_NOT_SUPORTED_, strlen(_NCCC_DCC_FLOW_VER_NOT_SUPORTED_)) == 0)
	{
		/* 此功能已關閉 */
		pobTran->inErrorMsg = _ERROR_CODE_V3_FUNC_CLOSE_;
		
		return (VS_ERROR);
	}

	/* 教育訓練模式 */
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		/* 是否有列印功能 */
		if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
		{
			return (VS_SUCCESS);
		}
		else
		{
			/* 初始化 */
			inPRINT_Buffer_Initial(uszBuffer, _BUFFER_MAX_LINE_, &srFont_Attrib, &srBhandle);

			/* 印商店名稱 */
			if (inCREDIT_PRINT_MerchantName(pobTran, uszBuffer, &srBhandle) != VS_SUCCESS)
			{
				return (VS_ERROR);
			}

			inRetVal = inPRINT_Buffer_PutIn("", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			if (inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_DCC_, &inDCCindex) != VS_SUCCESS)
				return (VS_ERROR);

			/* 主機參數檔 */
			if (inLoadHDTRec(inDCCindex) < 0) 
				return(VS_ERROR);

			inRetVal = inCREDIT_PRINT_Tidmid_ByBuffer(pobTran, uszBuffer, &srFont_Attrib, &srBhandle);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate, 0x00, sizeof(szTemplate));

			/* Date & Time */
			memset(&srRTC, 0x00, sizeof(RTC_NEXSYS));
			if (inFunc_GetSystemDateAndTime(&srRTC) != VS_SUCCESS)
			{
				return (VS_ERROR);
			}
			strcpy(szPrintBuf, "Valid at ");
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "20%02d/%02d/%02d   ", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
			strcat(szPrintBuf, szTemplate);
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%02d:%02d:%02d", srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
			strcat(szPrintBuf, szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			/* DCC Ecchange Rates */
			inPRINT_Buffer_PutIn("DCC Invert Rates ",_PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn(" ", _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("1 TWD =   ",_PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("0.0346 AUD",_PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
			inPRINT_Buffer_PutIn(" ", _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			inPRINT_Buffer_PutIn("1 TWD =   ",_PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("0.0354 CAD",_PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
			inPRINT_Buffer_PutIn(" ", _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			inPRINT_Buffer_PutIn("1 TWD =   ",_PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("0.2655 HKD",_PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
			inPRINT_Buffer_PutIn(" ", _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			inPRINT_Buffer_PutIn("1 TWD =   ",_PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("2.6227 JPY",_PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
			inPRINT_Buffer_PutIn(" ", _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			inPRINT_Buffer_PutIn("1 TWD =  ",_PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("39.0048 KRW",_PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
			inPRINT_Buffer_PutIn(" ", _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			inPRINT_Buffer_PutIn("1 TWD =   ",_PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("0.1087 MYR",_PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
			inPRINT_Buffer_PutIn(" ", _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			inPRINT_Buffer_PutIn("1 TWD =   ",_PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("0.0445 SGD",_PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
			inPRINT_Buffer_PutIn(" ", _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			inPRINT_Buffer_PutIn("1 TWD =   ",_PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("1.0651 THB",_PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
			inPRINT_Buffer_PutIn(" ", _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			inPRINT_Buffer_PutIn("1 TWD =   ",_PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("0.0341 USD",_PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
			inPRINT_Buffer_PutIn(" ", _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			inPRINT_Buffer_PutIn("1 TWD =   ",_PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("0.0253 EUR",_PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
			inPRINT_Buffer_PutIn(" ", _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);


			inPRINT_Buffer_PutIn(" ", _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("DCC Exchange Rates ",_PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn(" ", _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);


			inPRINT_Buffer_PutIn("1 AUD =    ",_PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("21.74 TWD",_PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
			inPRINT_Buffer_PutIn(" ", _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			inPRINT_Buffer_PutIn("1 CAD =    ",_PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("28.25 TWD",_PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
			inPRINT_Buffer_PutIn(" ", _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			inPRINT_Buffer_PutIn("1 HKD =   ",_PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("3.7664 TWD",_PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
			inPRINT_Buffer_PutIn(" ", _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			inPRINT_Buffer_PutIn("1 JPY =   ",_PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("0.3812 TWD",_PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
			inPRINT_Buffer_PutIn(" ", _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			inPRINT_Buffer_PutIn("1 KRW =   ",_PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("0.0256 TWD",_PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
			inPRINT_Buffer_PutIn(" ", _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			inPRINT_Buffer_PutIn("1 MYR =   ",_PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("9.1996 TWD",_PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
			inPRINT_Buffer_PutIn(" ", _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			inPRINT_Buffer_PutIn("1 SGD =   ",_PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("22.47 TWD",_PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
			inPRINT_Buffer_PutIn(" ", _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			inPRINT_Buffer_PutIn("1 THB =   ",_PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("0.9389 TWD",_PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
			inPRINT_Buffer_PutIn(" ", _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			inPRINT_Buffer_PutIn("1 USD =    ",_PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("29.32 TWD",_PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
			inPRINT_Buffer_PutIn(" ", _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			inPRINT_Buffer_PutIn("1 EUR =    ",_PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("39.52 TWD",_PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
			for (i = 0; i < 8; i++)
			{
				inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}

			if (inPRINT_Buffer_OutPut(uszBuffer, &srBhandle) != VS_SUCCESS)
				return (VS_ERROR);

			return (VS_SUCCESS);
		}
	}
	else
	{
		if (inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_DCC_, &inDCCindex) != VS_SUCCESS)
			return (VS_ERROR);

		pobTran->srBRec.inHDTIndex = inDCCindex;

		/* 主機參數檔 */
		if (inLoadHDTRec(pobTran->srBRec.inHDTIndex) < 0) 
			return(VS_ERROR);
		if (inLoadHDPTRec(pobTran->srBRec.inHDTIndex) < 0) 
			return(VS_ERROR);

		/* 先檢查連線狀況 */
		if (pobTran->uszConnectionBit != VS_TRUE)
		{
			inRetVal = inFLOW_RunFunction(pobTran, _COMM_START_);
			if (inRetVal != VS_SUCCESS)
			{
                                /* Mirror Message */
                                if (pobTran->uszECRBit == VS_TRUE)
                                {
                                        inECR_SendMirror(pobTran, _MIRROR_MSG_COMM_ERROR_);
                                }
                                        
				/* 通訊失敗‧‧‧‧ */
				pobTran->inTransactionResult = _TRAN_RESULT_COMM_ERROR_;
				pobTran->inErrorMsg = _ERROR_CODE_V3_COMM_;
				inFunc_Display_Error(pobTran);			/* 通訊失敗 */

				/* 交易在這裡結束，交易不論成功或失敗都在這裡斷線
				   交易以前做法是要全部斷網路線，新的做法只要關【SOCKET】，【GPRS】同樣的做法
				*/
				inCOMM_End(pobTran);

				return (VS_ERROR);
			}

		}

		/* 組送收電文 */
		pobTran->inISOTxnCode = _DCC_EX_RATE_;				/* 組 ISO 電文 */
		inRetVal = inNCCC_DCC_SendPackRecvUnPack(pobTran);
		inNCCC_DCC_SetSTAN(pobTran);

		if (inRetVal != VS_SUCCESS)
		{
			/* 交易在這裡結束，交易不論成功或失敗都在這裡斷線
			   交易以前做法是要全部斷網路線，新的做法只要關【SOCKET】，【GPRS】同樣的做法
			*/
			inCOMM_End(pobTran);

			return (VS_ERROR);
		}

		/* 檢核資料正確性 */
		/* 1~3 Byte 為字母
		   4~6 Byte 為數字
		   7~21 空白、數字、小數點，共 220 Bytes */
		for (i = 0 ; i < 220; i += 22)
		{
			for (j = 0; j < 3 ; j ++)
			{
				if (gsrDCC_Download.uszDCC_Record[i + j] < 'A' || gsrDCC_Download.uszDCC_Record[i + j] > 'Z')
				{
					return (VS_ERROR);
				}
			}

			for (j = 3; j < 6; j ++)
			{
				if (gsrDCC_Download.uszDCC_Record[i + j] < '0' || gsrDCC_Download.uszDCC_Record[i + j] > '9')
				{
					return (VS_ERROR);
				}
			}

			for (j = 6; j < 22; j ++)
			{
				if ((gsrDCC_Download.uszDCC_Record[i + j] < '0' || gsrDCC_Download.uszDCC_Record[i + j] > '9') &&
				     gsrDCC_Download.uszDCC_Record[i + j] != ' ' &&
				     gsrDCC_Download.uszDCC_Record[i + j] != '.')
				{
					return (VS_ERROR);
				}
			}
		}

		/* 列印匯率表 */
		inRetVal = inNCCC_DCC_PRINT_ExchangeRate(pobTran);

		if (inRetVal != VS_SUCCESS)
		{
			return (VS_ERROR);
		}

		inCOMM_End(pobTran);

		return (VS_SUCCESS);
	}
}

/*
Function        :inNCCC_DCC_PRINT_ExchangeRate
Date&Time       :2016/10/19 下午 4:40
Describe        :
*/
int inNCCC_DCC_PRINT_ExchangeRate(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
	int		i;
	int		inDCCindex = -1;
	char		szPrintBuf[42 + 1], szTemplate[42];
	RTC_NEXSYS	srRTC;							/* Date & Time */
	unsigned char	uszBuffer[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	BufferHandle	srBhandle;
	FONT_ATTRIB	srFont_Attrib;

	/* 是否有列印功能 */
	if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
	{
		return (VS_SUCCESS);
	}
	else
	{
		/* 初始化 */
		inPRINT_Buffer_Initial(uszBuffer, _BUFFER_MAX_LINE_, &srFont_Attrib, &srBhandle);

		/* 印商店名稱 */
		if (inCREDIT_PRINT_MerchantName(pobTran, uszBuffer, &srBhandle) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}

		inRetVal = inPRINT_Buffer_PutIn("", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		if (inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_DCC_, &inDCCindex) != VS_SUCCESS)
			return (VS_ERROR);

		/* 主機參數檔 */
		if (inLoadHDTRec(inDCCindex) < 0) 
			return(VS_ERROR);

		inRetVal = inCREDIT_PRINT_Tidmid_ByBuffer(pobTran, uszBuffer, &srFont_Attrib, &srBhandle);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		/* Date & Time */
		memset(&srRTC, 0x00, sizeof(RTC_NEXSYS));
		if (inFunc_GetSystemDateAndTime(&srRTC) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		strcpy(szPrintBuf, "Valid at ");
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "20%02d/%02d/%02d   ", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
		strcat(szPrintBuf, szTemplate);
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%02d:%02d:%02d", srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
		strcat(szPrintBuf, szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* DCC Ecchange Rates */
		inPRINT_Buffer_PutIn("DCC Invert Rates ", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		for (i = 0 ; i < 220; i += 22)
		{
			inPRINT_Buffer_PutIn(" ", _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "1 TWD = ");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &gsrDCC_Download.uszDCC_Record[i + 6], 8);
			strcat(szPrintBuf, szTemplate);
			strcat(szPrintBuf, " ");
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &gsrDCC_Download.uszDCC_Record[i], 3);
			strcat(szPrintBuf, szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
		}

		inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* DCC Invert Rates */
		inPRINT_Buffer_PutIn("DCC Exchange Rates ",_PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		for (i = 0 ; i < 220; i += 22)
		{
			inPRINT_Buffer_PutIn(" ", _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "1 ");
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &gsrDCC_Download.uszDCC_Record[i], 3);
			strcat(szPrintBuf, szTemplate);
			strcat(szPrintBuf, " = ");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &gsrDCC_Download.uszDCC_Record[i + 14], 8);
			strcat(szPrintBuf, szTemplate);
			strcat(szPrintBuf, " TWD");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
		}

		/* 清空 */
		memset(gsrDCC_Download.uszDCC_Record, 0x00, sizeof(gsrDCC_Download.uszDCC_Record));

		for (i = 0; i < 8; i++)
		{
			inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}

		if (inPRINT_Buffer_OutPut(uszBuffer, &srBhandle) != VS_SUCCESS)
			return (VS_ERROR);

		return (VS_SUCCESS);
	}
}

/*
Function        :inNCCC_DCC_Already_Have_Parameter
Date&Time       :2016/10/18 下午 4:45
Describe        :是否有舊參數（回傳Trur Or False）
*/
int inNCCC_DCC_Already_Have_Parameter(TRANSACTION_OBJECT *pobTran)
{
	char	szDemoMode[2 + 1] = {0};
	char	szFileName[50 + 1] = {0};
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_DCC_Already_Have_Parameter() START !");
	}

	/* 教育訓練模式 */
	/* DCC教育訓練版不下DCC參數 */
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "DEMO Mode Skip");
		}
		return (VS_TRUE);
	}
	
	/* DCCBin.dat */
	memset(szFileName, 0x00, sizeof(szFileName));
	strcpy(szFileName, _NCCC_DCC_FILE_CARD_BIN_);
	if (inFILE_Check_Exist((unsigned char*)szFileName) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "%s NOT OK", szFileName);
		}
                vdUtility_SYSFIN_LogMessage(AT, "%s NOT OK", szFileName);
		return (VS_FALSE);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "%s OK", szFileName);
		}
	}
	
	/* ERS.dat */
	memset(szFileName, 0x00, sizeof(szFileName));
	strcpy(szFileName, _NCCC_DCC_FILE_EXCHANGE_RATE_SOURCE_);
	if (inFILE_Check_Exist((unsigned char*)szFileName) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "%s NOT OK", szFileName);
		}
                vdUtility_SYSFIN_LogMessage(AT, "%s NOT OK", szFileName);
		return (VS_FALSE);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "%s OK", szFileName);
		}
	}
	
	/* VISADis.dat */
	memset(szFileName, 0x00, sizeof(szFileName));
	strcpy(szFileName, _NCCC_DCC_FILE_VISA_DISCLAIMER_);
	if (inFILE_Check_Exist((unsigned char*)szFileName) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "%s NOT OK", szFileName);
		}
                vdUtility_SYSFIN_LogMessage(AT, "%s NOT OK", szFileName);
		return (VS_FALSE);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "%s OK", szFileName);
		}
	}
	
	/* MASTERDis.dat */
	memset(szFileName, 0x00, sizeof(szFileName));
	strcpy(szFileName, _NCCC_DCC_FILE_MASTERCARD_DISCLAIMER_);
	if (inFILE_Check_Exist((unsigned char*)szFileName) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "%s NOT OK", szFileName);
		}
                vdUtility_SYSFIN_LogMessage(AT, "%s NOT OK", szFileName);
		return (VS_FALSE);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "%s OK", szFileName);
		}
	}
	
	/* (需求單-111060)-DCC流程修改需求 by Russell 2022/4/14 下午 5:48 */
	/* CCI_V.dat CCI_M.dat改為不檢核，非必要項目 */
//	/* CCI_V.dat */
//	memset(szFileName, 0x00, sizeof(szFileName));
//	strcpy(szFileName, _NCCC_DCC_FILE_VISA_SUPPORTED_CURRENCY_LIST_);
//	if (inFILE_Check_Exist((unsigned char*)szFileName) != VS_SUCCESS)
//	{
//		if (ginDebug == VS_TRUE)
//		{
//			inLogPrintf(AT, "%s NOT OK", szFileName);
//		}
//              vdUtility_SYSFIN_LogMessage(AT, "%s NOT OK", szFileName);
//		return (VS_FALSE);
//	}
//	else
//	{
//		if (ginDebug == VS_TRUE)
//		{
//			inLogPrintf(AT, "%s OK", szFileName);
//		}
//	}
//	
//	/* CCI_M.dat */
//	memset(szFileName, 0x00, sizeof(szFileName));
//	strcpy(szFileName, _NCCC_DCC_FILE_MASTERCARD_SUPPORTED_CURRENCY_LIST_);
//	if (inFILE_Check_Exist((unsigned char*)szFileName) != VS_SUCCESS)
//	{
//		if (ginDebug == VS_TRUE)
//		{
//			inLogPrintf(AT, "%s NOT OK", szFileName);
//		}
//              vdUtility_SYSFIN_LogMessage(AT, "%s NOT OK", szFileName);
//		return (VS_FALSE);
//	}
//	else
//	{
//		if (ginDebug == VS_TRUE)
//		{
//			inLogPrintf(AT, "%s OK", szFileName);
//		}
//	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_DCC_Already_Have_Parameter() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_TRUE);
}

/*
Function        :inNCCC_DCC_TMS_Schedule_Hour_Check
Date&Time       :2016/10/24 下午 3:11
Describe        :DCC參數下載整點提示
*/
int inNCCC_DCC_TMS_Schedule_Hour_Check()
{
	RTC_NEXSYS	srRTC;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_DCC_TMS_Schedule_Hour_Check() START");
	}
	
	/* szTerminalNowDateAndTime */
	memset(&srRTC, 0x00, sizeof(srRTC));
	if (inFunc_GetSystemDateAndTime(&srRTC) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	if (srRTC.uszMinute == 0)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inNCCC_DCC_TMS_Schedule_Hour_Check() SUCCESS END");
		}
		return (VS_SUCCESS);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inNCCC_DCC_TMS_Schedule_Hour_Check() ERROR END");
		}
		return (VS_ERROR);
	}
}

/*
Function        :inNCCC_DCC_TMS_Schedule_Check
Date&Time       :2016/10/24 下午 3:23
Describe        :TMS排程下載連動DCC參數下載檢查
*/
int inNCCC_DCC_TMS_Schedule_Check(TRANSACTION_OBJECT *pobTran)
{
	char	szTMSOK[2 + 1] = {0};
	char	szDCCFlowVersion[2 + 1] = {0};
	char	szHostEnable[2 + 1] = {0};
	char	szTMSDownloadTimes[6 + 1] = {0};
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_DCC_TMS_Schedule_Check() START !");
	}
	
	/* 1.檢查是否做過TMS下載(參數下載) */
	memset(szTMSOK, 0x00, sizeof(szTMSOK));
	if (inGetTMSOK(szTMSOK) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inNCCC_DCC_TMS_Schedule_Check：TMS GET FAIL");
		}
		return (VS_ERROR);
	}
	
	/* 沒下TMS，跳出 */
	if (szTMSOK[0] != 'Y')
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inNCCC_DCC_TMS_Schedule_Check：TMS NOT OK");
		}
		return (VS_ERROR);
	}
	
	/* 2.是否支援DCC */
	memset(szHostEnable, 0x00, sizeof(szHostEnable));
	inNCCC_DCC_GetDCC_Enable(pobTran->srBRec.inHDTIndex, szHostEnable);
		
	if (memcmp(szHostEnable, "Y", 1) != 0)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inNCCC_DCC_TMS_Schedule_Check：DCC HOST NOT OPEN");
		}
		return (VS_ERROR);
	}
	
	/* 3.檢查CFGT內的DCC開關
	 * DCC詢價版本, 預設值 = 0, 0 = 不支援DCC, 1 = 直接於詢價時由 DCC 依 Card Bin 回覆其外幣幣別及金額, 2 = 於EDC選擇交易幣別詢價 */
	memset(szDCCFlowVersion, 0x00, sizeof(szDCCFlowVersion));
	if (inGetDccFlowVersion(szDCCFlowVersion) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inNCCC_DCC_TMS_Schedule_Check：CFGT DCC SWITCH GET FAIL");
		}
		return (VS_ERROR);
	}
	
	/* 不支援DCC，跳出 */
	if (memcmp(szDCCFlowVersion, _NCCC_DCC_FLOW_VER_NOT_SUPORTED_, 1) == 0)
        {
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inNCCC_DCC_TMS_Schedule_Check：CFGT DCC SWITCH NOT OPEN：%s", szDCCFlowVersion);
		}
        	return (VS_ERROR);
        }
	
	/* 檢查是否有舊DCC參數(沒有才要下) */
	if (inNCCC_DCC_Already_Have_Parameter(pobTran) == VS_TRUE)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inNCCC_DCC_TMS_Schedule_Check：DCC PARAMETER ALREADY HAVE");
		}
		return (VS_ERROR);
	}
	
	/* 若為新裝機狀態，不能自動下載 */
	/* 只要更新完後，計數就加一，所以新裝機為一 */
	/* 如果預Load有可能次數為0，也加入判斷 */
	memset(szTMSDownloadTimes, 0x00, sizeof(szTMSDownloadTimes));
	inGetTMSDownloadTimes(szTMSDownloadTimes);
	if (atoi(szTMSDownloadTimes) == 1	||
	    atoi(szTMSDownloadTimes) == 0)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inNCCC_DCC_TMS_Schedule_Check：TERMINAL NEW INSTALLED:%s", szTMSDownloadTimes);
		}
		return (VS_ERROR);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_DCC_TMS_Schedule_Check() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_DCC_TMS_Schecule
Date&Time       :2016/10/14 下午 1:40
Describe        :TMS排程下載連動DCC參數下載
*/
int inNCCC_DCC_TMS_Schedule(TRANSACTION_OBJECT *pobTran)
{
/* 如果沒有連線能力，就不跑 */
#ifndef _COMMUNICATION_CAPBILITY_
	return (VS_SUCCESS);
#endif
	
	int	inRetVal = VS_SUCCESS;
	int	inRetryTimes = 0;			/* 重試次數 */
	int	inChoice = 0;
	int	inTouchSensorFunc = _Touch_CUP_LOGON_;	/* 位置和Logon一樣 */
	int	inOrgIndex = 0;
	char	szKey = 0x00;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_DCC_TMS_Schedule() START !");
	}
	
	if (inNCCC_DCC_TMS_Schedule_Check(pobTran) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
/* -------------------------------------上面是檢查------------------------------------- */
	
	/* 切換到DCC Host */
	inRetVal = inNCCC_DCC_SwitchToDCC_Host(pobTran->srBRec.inHDTIndex);
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	inOrgIndex = pobTran->srBRec.inHDTIndex;
	pobTran->srBRec.inHDTIndex = ginDCCHostIndex;
        inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
			
	/* DCC參數下載 執行下載請按確認，不執行請按清除 */
	inDISP_ClearAll();
	/* DCC參數下載 */
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_DCC_DOWNLOAD_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜DCC參數下載＞ */
	
	inRetryTimes = 0;
	do
	{
		/* 判斷結果 */
		/* 一次加重試三次 */
		if (inRetryTimes > 3)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_CHECK_DOWNLOAD_, 0, _COORDINATE_Y_LINE_8_4_);
			
			inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);

			/* 執行下載請按確認，不執行請按清除 */
			while (1)
			{
				inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
				szKey = uszKBD_Key();
				
				/* 比照520 排程下載失敗不要有timeout */
				/* 2012/9/27 下午 03:49:24 for 升文說 排程下載的失敗畫面不要有timeout */

				if (szKey == _KEY_ENTER_)
				{
					break;
				}
				else if (szKey == _KEY_CANCEL_)
				{
					inRetVal = VS_USER_CANCEL;
					break;
				}
			}
			/* 清空Touch資料 */
			inDisTouch_Flush_TouchFile();
		
			/* 取消則跳出 */
			if (inRetVal == VS_USER_CANCEL)
			{
			     break;
			}
			else
			{
				/* 繼續重試 */
			}
		}
		
		/* 每執行inNCCC_DCC_TMSDownload_CheckStatus一次就加一 */
		inRetryTimes ++;
		inNCCC_DCC_GetSTAN(pobTran);
		
		/* 開始連線 */
		if (pobTran->uszConnectionBit != VS_TRUE)
		{
			inRetVal = inFLOW_RunFunction(pobTran, _COMM_START_);
			if (inRetVal != VS_SUCCESS)
			{
				/* 例外處理: DCC下載失敗，DCC結帳完成後再進行一次DCC參數下載 */
				inSetDCCSettleDownload("1");
				inSaveEDCRec(0);

				/* 比照520提示下載失敗 */
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("下載失敗", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);  
				inDISP_BEEP(2, 0500);
				continue;
			}

		}
		inRetVal = inNCCC_DCC_TMSDownload_CheckStatus(pobTran);
		
		if (inRetVal == VS_SUCCESS)
		{
			/* 顯示下載成功停兩秒 */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_ERR_DOWNLOAD_SUCCESS_, 0, _COORDINATE_Y_LINE_8_7_);
			inDISP_BEEP(1, 0);
			inDISP_Wait(2000);
			break;
		}
		else
		{
			/* 顯示下載失敗停兩秒 */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_ERR_DOWNLOAD_FAILED_, 0, _COORDINATE_Y_LINE_8_7_);
			inDISP_BEEP(1, 0);
			inDISP_Wait(2000);
			continue;
		}
		
	}while (1);
	
	/* 結束連線 */
	inCOMM_End(pobTran);
	
	/* 下載失敗 */
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "下載失敗");
		}
		
		/* 只有在沒有舊參數，且下載失敗的情況下才需改成整點提示 */
		if (inNCCC_DCC_Already_Have_Parameter(pobTran) == VS_FALSE)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "無DCC參數且下載失敗，改為整點提示");
			}
			
			inLoadEDCRec(0);
			inSetDCCDownloadMode(_NCCC_DCC_DOWNLOAD_MODE_HOUR_);
			inSaveEDCRec(0);
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "下載成功");
		}
	}
	
	pobTran->srBRec.inHDTIndex = inOrgIndex;
        inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_DCC_TMS_Schedule() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :inNCCC_DCC_Hour_Notify
Date&Time       :2019/12/31 下午 2:52
Describe        :DCC整點下載提示
*/
int inNCCC_DCC_Hour_Notify(TRANSACTION_OBJECT *pobTran)
{
/* 如果沒有連線能力，就不跑 */
#ifndef _COMMUNICATION_CAPBILITY_
	return (VS_SUCCESS);
#endif
	
	int	inRetVal = VS_SUCCESS;
	int	inRetryTimes = 0;			/* 重試次數 */
	int	inChoice = 0;
	int	inTouchSensorFunc = _Touch_CUP_LOGON_;	/* 位置和Logon一樣 */
	int	inOrgIndex = 0;
	char	szKey = 0x00;
	
        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_Hour_Notify START!");
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_DCC_Hour_Notify() START !");
	}
	
	if (inNCCC_DCC_TMS_Schedule_Check(pobTran) != VS_SUCCESS)
		return (VS_ERROR);
/* -------------------------------------上面是檢查------------------------------------- */
	
	/* 切換到DCC Host */
	inRetVal = inNCCC_DCC_SwitchToDCC_Host(pobTran->srBRec.inHDTIndex);
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	inOrgIndex = pobTran->srBRec.inHDTIndex;
	pobTran->srBRec.inHDTIndex = ginDCCHostIndex;
        inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
			
	/* DCC參數下載 執行下載請按確認，不執行請按清除 */
	inDISP_ClearAll();
	/* DCC參數下載 */
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_DCC_DOWNLOAD_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜DCC參數下載＞ */
	
	inRetryTimes = 0;
	do
	{
		/* 判斷結果 */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_CHECK_DOWNLOAD_, 0, _COORDINATE_Y_LINE_8_4_);

		inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);

		/* 執行下載請按確認，不執行請按清除 */
		while (1)
		{
			inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
			szKey = uszKBD_Key();

			/* 比照520 排程下載失敗不要有timeout */
			/* 2012/9/27 下午 03:49:24 for 升文說 排程下載的失敗畫面不要有timeout */

			if (szKey == _KEY_ENTER_)
			{
				break;
			}
			else if (szKey == _KEY_CANCEL_)
			{
				inRetVal = VS_USER_CANCEL;
				break;
			}
		}
		/* 清空Touch資料 */
		inDisTouch_Flush_TouchFile();

		/* 取消則跳出 */
		if (inRetVal == VS_USER_CANCEL)
		{
		     break;
		}
		else
		{
			/* 繼續重試 */
		}
		
		inNCCC_DCC_GetSTAN(pobTran);
		/* 開始連線 */
		if (pobTran->uszConnectionBit != VS_TRUE)
		{
			inRetVal = inFLOW_RunFunction(pobTran, _COMM_START_);
			if (inRetVal != VS_SUCCESS)
			{
				/* 例外處理: DCC下載失敗，DCC結帳完成後再進行一次DCC參數下載 */
				inSetDCCSettleDownload("1");
				inSaveEDCRec(0);

				/* 比照520提示下載失敗 */
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("下載失敗", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);  
				inDISP_BEEP(2, 0500);
				continue;
			}

		}
		inRetVal = inNCCC_DCC_TMSDownload_CheckStatus(pobTran);
		
		if (inRetVal == VS_SUCCESS)
		{
			/* 顯示下載成功停兩秒 */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_ERR_DOWNLOAD_SUCCESS_, 0, _COORDINATE_Y_LINE_8_7_);
			inDISP_BEEP(1, 0);
			inDISP_Wait(2000);
			break;
		}
		else
		{
			/* 顯示下載失敗停兩秒 */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_ERR_DOWNLOAD_FAILED_, 0, _COORDINATE_Y_LINE_8_7_);
			inDISP_BEEP(1, 0);
			inDISP_Wait(2000);
			continue;
		}
		
	}while (1);
	
	/* 結束連線 */
	inCOMM_End(pobTran);
	
	/* 離開前清空 */
	pobTran->inErrorMsg = 0x00;
	
	pobTran->srBRec.inHDTIndex = inOrgIndex;
        inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_DCC_Hour_Notify() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :inNCCC_DCC_AutoDownload_Check
Date&Time       :2016/10/14 下午 1:40
Describe        :確認DCC自動排程下載模式狀態

*/
int inNCCC_DCC_AutoDownload_Check()
{
	int			inRetVal;
	char			szTMSOK[2 + 1];
	char			szDCCFlowVersion[2 + 1];
	char			szHostEnable[2 + 1];
	TRANSACTION_OBJECT	pobTran;			/* 無實際用途，僅inFunc_Find_Specific_HDTindex會用到 */
	
	/* 1.檢查是否做過TMS下載(參數下載) */
	memset(szTMSOK, 0x00, sizeof(szTMSOK));
	if (inGetTMSOK(szTMSOK) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	/* 沒下TMS，跳出 */
	if (szTMSOK[0] != 'Y')
	{
		return (VS_ERROR);
	}
	
	/* 2.檢查HDT是否有DCC這個HOST並Enable */
	memset(&pobTran, 0x00, sizeof(TRANSACTION_OBJECT));
	pobTran.srBRec.inHDTIndex = 0;
	
	memset(szHostEnable, 0x00, sizeof(szHostEnable));
	inNCCC_DCC_GetDCC_Enable(pobTran.srBRec.inHDTIndex, szHostEnable);
		
	if (memcmp(szHostEnable, "Y", 1) != 0)
	{
		return (VS_ERROR);
	}
	
	/* 3.檢查CFGT內的DCC開關
	 * DCC詢價版本, 預設值 = 0, 0 = 不支援DCC, 1 = 直接於詢價時由 DCC 依 Card Bin 回覆其外幣幣別及金額, 2 = 於EDC選擇交易幣別詢價 */
	memset(szDCCFlowVersion, 0x00, sizeof(szDCCFlowVersion));
	if (inGetDccFlowVersion(szDCCFlowVersion) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	/* 不支援DCC，跳出 */
	if (memcmp(szDCCFlowVersion, _NCCC_DCC_FLOW_VER_NOT_SUPORTED_, 1) == 0)
        {
        	return (VS_ERROR);
        }

	/* 檢查日期 */
	inRetVal = inNCCC_DCC_AutoDownload_Check_Date();
	if (inRetVal == VS_SUCCESS)
	{
		/* 檢查時間 */
		inRetVal = inNCCC_DCC_AutoDownload_Check_Time();
		if (inRetVal == VS_SUCCESS)
		{
			return (VS_SUCCESS);
		}
	}
	
	return (VS_ERROR);
}

/*
Function        :inNCCC_DCC_AutoDownload_Check_Date
Date&Time       :2016/10/21 下午 3:40
Describe        :檢查日期
*/
int inNCCC_DCC_AutoDownload_Check_Date()
{
	int		inOrgIndex = 0;
	int		inHostIndex = -1;	/* 找DCC Host indxe用 */
	int		inTidLen = 0;
	long	lnCurrentSolarDays = 0;
	long	lnLastUpdateDays = 0;
	char	szTID[8 + 1];	/* TID*/
	char	szDCCUpdateDay[2 + 1];	/* 循環觸發日 */
	/*  20251215_BUG_MDF][FUNC] 修改DCC參數下載排程在12號後的排程會一直進行下載
	 *  放大 szTerminalNowDate 欄位由6改成8 以符合YYYYMMDD格式共8Bytes
	 */
	char			szDCCLastUpdateDate[8 + 1]; /* YYYYMMDD */
	char			szTerminalNowDate[8 + 1];	/* YYYYMMDD */
	char			szTemplate[12 + 1];
	RTC_NEXSYS		srRTC;
	
	/* Get Terminal ID */
	/* 開機後只找一次DCC */
	if (ginDCCHostIndex != -1)
	{
		/* 直接使用 */
		inHostIndex = ginDCCHostIndex;
	}
	else
	{
		inFunc_Find_Specific_HDTindex(inOrgIndex, _HOST_NAME_DCC_, &inHostIndex);
	}
	
	if (inLoadHDTRec(inHostIndex) != VS_SUCCESS)
		return (VS_ERROR);
	
	memset(szTID, 0x00, sizeof(szTID));
	if (inGetTerminalID(szTID) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	/* 回覆原Host */
	if (inLoadHDTRec(inOrgIndex) != VS_SUCCESS)
		return (VS_ERROR);
	
	/* 觸發日期 = Terminal ID末碼 */
	/* [20260128_BUG_MDF][FUN] 修改DCC檢查日期拷貝方式 */
	inTidLen = strlen(szTID);
    memset(szDCCUpdateDay, 0x00, sizeof(szDCCUpdateDay));
    if (inTidLen > 0)
    {
        memcpy(szDCCUpdateDay, &szTID[inTidLen - 1], 1);
    }
    else
    {
        /* 若取不到 TID，預設為 0 或不觸發 */
        szDCCUpdateDay[0] = '0';
        if (ginDebug == VS_TRUE) inLogPrintf(AT, "Warning: TID Length is 0");
		/*[260414] 移除記錄,因此記錄在主畫面會多次進入 */
		//vdUtility_SYSFIN_LogMessage(AT, "Warning: TID Length is 0");
    }
	
	/* szDCCLastUpdateDate */
	/*  20251215_BUG_MDF][FUNC] 修改DCC參數下載排程在12號後的排程會一直進行下載
	 *  修改inGetDCCLastUpdateDate 抓取欄位,因抓取欄位只有六個Bytes YYMMDD
	 *  所以修改傳入欄位後再進行拷貝,以不動szDCCLastUpdateDate命名判斷
	 */
	memset(szDCCLastUpdateDate, 0x00, sizeof(szDCCLastUpdateDate));
	memset(szTemplate, 0x00, sizeof(szTemplate));

	if (inGetDCCLastUpdateDate(szTemplate) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	snprintf(szDCCLastUpdateDate, sizeof(szDCCLastUpdateDate), "20%s",szTemplate);
	
	/* szTerminalNowDateAndTime */
	memset(&srRTC, 0x00, sizeof(srRTC));
	if (inFunc_GetSystemDateAndTime(&srRTC) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "szDCCLastUpdateDate[%s] ", szDCCLastUpdateDate);
	
	/*  20251215_BUG_MDF][FUNC] 修改DCC參數下載排程在12號後的排程會一直進行下載
	 *  在szTerminalNowDate格式欄位補上'20'以符合YYYYMMDD格式共8Bytes
	 */
	memset(szTerminalNowDate, 0x00, sizeof(szTerminalNowDate));
	snprintf(szTerminalNowDate, sizeof(szTerminalNowDate), "20%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);	
	
	if (ginDebug == VS_TRUE) inLogPrintf(AT, "szTerminalNowDate[%s] ", szTerminalNowDate);
	
	
	/*  20251215_BUG_MDF][FUNC] 修改DCC參數下載排程在12號後的排程會一直進行下載
	 *  修改比對日期的欄位，因日期格式由原本的6改為8的 YYYYMMDD格式,所以需比對資料的第三個byte後的值
	 */
	/* 更新記錄的日期與現在EDC日期相同 表示已自動更新過 */
	if (memcmp(&szTerminalNowDate[2], &szDCCLastUpdateDate[2], 6) == 0)
	{
		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "CMP Same T_NowDate[%s] DCC_LastUpdate[%s] ",szTerminalNowDate, szDCCLastUpdateDate);
		return (VS_ERROR);
	}
	
	if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "----Start SunDay Sum----");
	/* 例外狀況：端末機超過10天未啟動DCC參數下載 */
	/* 1. 直接計算絕對天數 (已包含年份資訊) */
	lnCurrentSolarDays = inFunc_SunDay_Sum(szTerminalNowDate);
	lnLastUpdateDays = 0;
	
	if (ginDebug == VS_TRUE)
			inLogPrintf(AT, " SunDay lnCSDays[%ld] ",lnCurrentSolarDays);
	/* 2. 檢查是否有上次更新日期 */
	if (szDCCLastUpdateDate != NULL && atoi(szDCCLastUpdateDate) > 0)
	{
		/* DCC 最後更新日期 */
		lnLastUpdateDays = inFunc_SunDay_Sum(szDCCLastUpdateDate);
		
		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "SunDay lnLUDays[%ld] ",lnLastUpdateDays);

		/* 錯誤處理：如果日期格式錯誤，inFunc_SunDay_Sum 回傳 VS_ERROR (-1) */
		if (lnCurrentSolarDays != VS_ERROR && lnLastUpdateDays != VS_ERROR)
		{
			/* 3. 直接相減，跨年也能自動正確計算 */
			if ((lnCurrentSolarDays - lnLastUpdateDays) > 10)
			{
				if (ginDebug == VS_TRUE)
					inLogPrintf(AT, "Over 10 Days Diff: %ld", (lnCurrentSolarDays - lnLastUpdateDays));

				return (VS_SUCCESS);
			}
			
		}else{
			if (ginDebug == VS_TRUE)
				inLogPrintf(AT, "SolarDays lnCSDays[%ld] lnLUDays[%ld] Error", lnCurrentSolarDays, lnLastUpdateDays);
			/*[260414] 移除記錄,因此記錄在主畫面會多次進入 */
			//vdUtility_SYSFIN_LogMessage(AT, "SolarDays lnCSDays[%ld] lnLUDays[%ld] Error",lnCurrentSolarDays, lnLastUpdateDays);
		}
	}
		
	/* 端末機末碼循環日觸發 */
	/* 忽略31日 */
	if (srRTC.uszDay == 31)
	{
		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "SolarDays 31 CurrentSD[%ld] LastUD[%ld] Error",lnCurrentSolarDays, lnLastUpdateDays);
		/*[260414] 移除記錄,因此記錄在主畫面會多次進入 */
		//vdUtility_SYSFIN_LogMessage(AT, "SolarDays 31 CurrentSD[%ld] LastUD[%ld] Error",lnCurrentSolarDays, lnLastUpdateDays);
		return (VS_ERROR);
	}
	
	if (ginDebug == VS_TRUE)
		inLogPrintf(AT, "szDCCUpdateDay[%s]",szDCCUpdateDay);
	/* 日期末碼 */
	if ((srRTC.uszDay % 10) == atoi(szDCCUpdateDay))
	{
		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "Check Terminal Value DCC_PT T_D[%d] DCC_UD[%s] ",srRTC.uszDay, szDCCUpdateDay);
		return (VS_SUCCESS);
	}
	
	if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "inNCCC_DCC_AutoDownload_Check_Date ERROR END ");
	return (VS_ERROR);
}

/*
Function        :inNCCC_DCC_AutoDownload_Check_Time
Date&Time       :2016/10/21 下午 4:48
Describe        :檢查時間
 * 觸發時間：當符合觸發日期後，端末機於營業時間自動進行DCC參數下載。
 * 以端末機代號倒數第2碼當作下載時間的小時，倒數第2、3碼當作下載時間的分鐘。
 * 例：端末機代號13991234倒數第2碼為3，倒數第3、2碼為23。表觸發日期的下午3點23分自動進行DCC參數下載。
 * 
 * 註：若”小時”大於8，自動減8。”分鐘”大於59，自動減59。
 * 例：端末機代號為13991896，倒數第2碼為9  大於8，故需要自動減8，所以是下午1點。
 *     倒數第3、2碼為89大於59，故需要自動減59，所以是30分。因此端末機代號13991896 DCC BIN自動下載時間為下午1點30分。
 * 註：端末機代號倒數第2碼為0，視為中午12點。
 */
int inNCCC_DCC_AutoDownload_Check_Time()
{
	int			inOrgIndex = 0;
	int			inDCCUpdateHour = 0, inDCCUpdateMinute = 0, inDCCUpdateTotalMinute = 0;
	int			inHostIndex = -1;						/* 找DCC Host indxe用 */
	char			szTID[8 + 1];
	char			szDCCUpdateHour[2 + 1];						/* 循環觸發小時 */
	char			szDCCUpdateMinute[2 + 1];					/* 循環觸發分鐘 */
	RTC_NEXSYS		srRTC;

	if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "inNCCC_DCC_AutoDownload_Check_Time START ");
	/* 開機後只找一次DCC */
	if (ginDCCHostIndex != -1)
	{
		/* 直接使用 */
		inHostIndex = ginDCCHostIndex;
	}
	else
	{
		inFunc_Find_Specific_HDTindex(inOrgIndex, _HOST_NAME_DCC_, &inHostIndex);
	}
	
	if (inLoadHDTRec(inHostIndex) != VS_SUCCESS)
		return (VS_ERROR);
	
	memset(szTID, 0x00, sizeof(szTID));
	if (inGetTerminalID(szTID) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	/* 回覆原Host */
	if (inLoadHDTRec(inOrgIndex) != VS_SUCCESS)
		return (VS_ERROR);

	/* Terminal ID換算觸發小時 */
	memset(szDCCUpdateHour, 0x00, sizeof(szDCCUpdateHour));
	memcpy(szDCCUpdateHour, &szTID[strlen(szTID) - 2], 1);
	inDCCUpdateHour = atoi(szDCCUpdateHour);

	if (inDCCUpdateHour > 8)
		inDCCUpdateHour = inDCCUpdateHour - 8;

	inDCCUpdateHour = inDCCUpdateHour + 12;					/* 中午過後開始下載 */

	/* Terminal ID換算觸發分鐘 */
	memset(szDCCUpdateMinute, 0x00, sizeof(szDCCUpdateMinute));
	memcpy(szDCCUpdateMinute, &szTID[strlen(szTID) - 3], 2);
	inDCCUpdateMinute = atoi(szDCCUpdateMinute);
	
	if (inDCCUpdateMinute > 59)
		inDCCUpdateMinute = inDCCUpdateMinute - 59;

	inDCCUpdateTotalMinute = inDCCUpdateMinute + (inDCCUpdateHour * 60);	/* 總分鐘數 */
	
	/* szTerminalNowDateAndTime */
	memset(&srRTC, 0x00, sizeof(srRTC));
	if (inFunc_GetSystemDateAndTime(&srRTC) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	if (ginDebug == VS_TRUE)
			inLogPrintf(AT, " DCC Time[%d] RCT[%d] ", inDCCUpdateTotalMinute, (srRTC.uszHour * 60 + srRTC.uszMinute));
	
	if (inDCCUpdateTotalMinute == (srRTC.uszHour * 60 + srRTC.uszMinute))
		return (VS_SUCCESS);

	if (inDCCUpdateTotalMinute < (srRTC.uszHour * 60 + srRTC.uszMinute)) /* 意外狀況 超過觸發時間(交易中、未開機、未在IDLE畫面) */
	{
		/* 例外處理: 超過觸發時間，DCC結帳完成後再進行一次DCC參數下載 */
		inSetDCCSettleDownload("1");
		inSaveEDCRec(0);
		
		return (VS_ERROR);
	}
	
	if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "inNCCC_DCC_AutoDownload_Check_Time Return ERROR END ");
	
	return (VS_ERROR);
}

/*
Function        :inNCCC_DCC_AutoDownload
Date&Time       :2016/10/14 下午 1:40
Describe        :DCC 排程下載

*/
int inNCCC_DCC_AutoDownload(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
	int		inOrgIndex = 0;
	char		szDCCLastUpdateDate[6 + 1];	/* YYMMDD */
	RTC_NEXSYS	srRTC;

	vdUtility_SYSFIN_LogMessage(AT, "inNCCC_DCC_AutoDownload START!");
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_DCC_AutoDownload() START !");
	}
	
	/* 重置DCC結帳下載參數 */
	inSetDCCSettleDownload("0");
	inSaveEDCRec(0);
	
	/* <DCC參數下載> */
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_DCC_DOWNLOAD_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 <DCC參數下載> */
	/* 參數下載 請勿關機 */
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_PutGraphic(_TMS_DOWNLOADING_, 0, _COORDINATE_Y_LINE_8_4_);
	inDISP_BEEP(1, 0);
	inDISP_Wait(4000);
	
	/* 不論下載成功或失敗都更新szDCCLastUpdateDate參數 下載失敗結帳下載用szDCCSettleDownload參數 */
	/* 紀錄丄一次DCC排程下載日期 */
	memset(&srRTC, 0x00, sizeof(srRTC));
	if (inFunc_GetSystemDateAndTime(&srRTC) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	memset(szDCCLastUpdateDate, 0x00, sizeof(szDCCLastUpdateDate));
	sprintf(szDCCLastUpdateDate, "%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
	inSetDCCLastUpdateDate(szDCCLastUpdateDate);
	inSaveEDCRec(0);
	
	/* 切換到DCC Host */
	inRetVal = inNCCC_DCC_SwitchToDCC_Host(pobTran->srBRec.inHDTIndex);
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	inOrgIndex = pobTran->srBRec.inHDTIndex;
	pobTran->srBRec.inHDTIndex = ginDCCHostIndex;
	inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
	
	inNCCC_DCC_GetSTAN(pobTran);
	if (pobTran->uszConnectionBit != VS_TRUE)
	{
		inRetVal = inFLOW_RunFunction(pobTran, _COMM_START_);
		if (inRetVal != VS_SUCCESS)
		{
			/* 例外處理: DCC下載失敗，DCC結帳完成後再進行一次DCC參數下載 */
			inSetDCCSettleDownload("1");
			inSaveEDCRec(0);
		
			/* 通訊失敗‧‧‧‧ */
			pobTran->inTransactionResult = _TRAN_RESULT_COMM_ERROR_;
			pobTran->inErrorMsg = _ERROR_CODE_V3_COMM_;
			inFunc_Display_Error(pobTran);			/* 通訊失敗 */
			/* 結束連線 */
			inCOMM_End(pobTran);
			
			pobTran->srBRec.inHDTIndex = inOrgIndex;
			inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
			
			return (VS_COMM_ERROR);
		}
		
	}
	
	/* 開始下載 */
	inRetVal = inNCCC_DCC_TMSDownload_CheckStatus(pobTran);
	
	/* 斷線 */
	inCOMM_End(pobTran);
	
	pobTran->srBRec.inHDTIndex = inOrgIndex;
	inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_DCC_AutoDownload() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	/* 自動下載不列印參數 */
	if (inRetVal != VS_SUCCESS)
	{
		/* 例外處理: DCC下載失敗，DCC結帳完成後再進行一次DCC參數下載 */
		inSetDCCSettleDownload("1");
		inSaveEDCRec(0);
		
		/* 顯示下載失敗停兩秒 */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_ERR_DOWNLOAD_FAILED_, 0, _COORDINATE_Y_LINE_8_7_);
		inDISP_BEEP(1, 0);
		inDISP_Wait(2000);
		
		return (VS_ERROR);
	}
	else
	{
		/* 顯示下載成功停兩秒 */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_ERR_DOWNLOAD_SUCCESS_, 0, _COORDINATE_Y_LINE_8_7_);
		inDISP_BEEP(1, 0);
		inDISP_Wait(2000);
		
		return (VS_SUCCESS);
	}
	
}

/*
Function        :inNCCC_DCC_Disclaimer_Display
Date&Time       :2016/9/12 下午 2:56
Describe        :印Disclaimer
*/
int inNCCC_DCC_Disclaimer_Display(char *szFilename, int inFontSize, int inLine)
{
	int		i = 0;
	int		inOffset = 0;				/* 偏移 */
	int		inRetVal = 0;
	int		inRetVal2 = VS_SUCCESS;			/* 發生錯誤時，立即中止並釋放記憶體 */
	int		inDataSegament = 0;			/* 一次讀多少 */
	int		inPrintHandle = 0;			/* put in 到哪裡 */
	int		inLineLength = 0;			/* 一行印得下多少char */
	int		inCurrentLine = 0;
	long		lnDataLength = 0;			/* 資料長度 */
	long            lnReadLength = 0;			/* 剩餘讀取長度 */
	char		szPrintTemp[50 + 1] = {0};		/* 放印一行的陣列 */
	unsigned long	ulFile_Handle;
        unsigned char   *uszTemp;                               /* 暫存，放整筆CDT檔案 */
	unsigned short	usFontSize = 0x0000;

        if (inFILE_OpenReadOnly(&ulFile_Handle, (unsigned char *)szFilename) == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
        /*
         * get data size 為了接下來動態宣告記憶體的大小需要知道多少 需先取得檔案Size
         * get size 失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        lnDataLength = lnFILE_GetSize(&ulFile_Handle, (unsigned char *)szFilename);

        if (lnDataLength == VS_ERROR)
        {
                /* GetSize失敗 ，關檔 */
                inFILE_Close(&ulFile_Handle);

                return (VS_ERROR);
        }

        /*
         * allocate 記憶體
         * allocate時多分配一個byte以防萬一（ex:換行符號）
         */
        uszTemp = malloc(lnDataLength + 1);
        /* 初始化 uszTemp uszReadData */
        memset(uszTemp, 0x00, lnDataLength + 1);

	do
	{
		if (inDISP_Decide_FontSize(inFontSize, _DISP_CHINESE_, &usFontSize) != VS_SUCCESS)
		{
			inRetVal2 = VS_ERROR;
			break;
		}

		/* 算出一行可以放幾個char (_LCD_XSIZE_ = 384) */
		inLineLength = (_LCD_XSIZE_ / (usFontSize / 0X0100)) + 1;

		/*一次讀1024 */
		inDataSegament = 1024;

		/* seek 到檔案開頭 & 從檔案開頭開始read */
		if (inFILE_Seek(ulFile_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
		{
			lnReadLength = lnDataLength;

			for (i = 0;; ++i)
			{
				/* 剩餘長度大於或等於inLineLength */
				if (lnReadLength >= inDataSegament)
				{
					if (inFILE_Read(&ulFile_Handle, &uszTemp[inDataSegament * i], inDataSegament) == VS_SUCCESS)
					{
						/* 一次讀inDataSegament */
						lnReadLength -= inDataSegament;

						/* 當剩餘長度剛好為inLineLength，會剛好讀完 */
						if (lnReadLength == 0)
							break;
					}
					/* 讀失敗時 */
					else
					{
						/* Close檔案 */
						inFILE_Close(&ulFile_Handle);

						/* Free pointer */
						free(uszTemp);

						return (VS_ERROR);
					}
				}
				/* 剩餘長度小於inLineLength */
				else if (lnReadLength < inDataSegament)
				{
					/* 就只讀剩餘長度 */
					if (inFILE_Read(&ulFile_Handle, &uszTemp[inDataSegament * i], inDataSegament) == VS_SUCCESS)
					{
						break;
					}
					/* 讀失敗時 */
					else
					{
						inRetVal2 = VS_ERROR;
						break;
					}
				}
			}/* end for loop */
			
			if (inRetVal2 == VS_ERROR)
			{
				break;
			}
		}
		/* seek不成功時 */
		else
		{
			inRetVal2 = VS_ERROR;
			break;
		}

		/* 剩餘讀取長度 */
		lnReadLength = lnDataLength;
		inCurrentLine = inLine;

		while (inPrintHandle < (int)lnDataLength)
		{
			/* 為了去除句首的空白 */
			while (uszTemp[inPrintHandle] == 0x20)
			{
				inPrintHandle++;
				lnReadLength--;
			}

			/* 確認句末是否會斷到字 */
			if (lnReadLength >= inLineLength)
			{
				/* 不能斷行的狀況 */
				if (uszTemp[inPrintHandle + inLineLength] != 0x20 && uszTemp[inPrintHandle + inLineLength - 1] != 0x20)
				{
					memset(szPrintTemp, 0x00, sizeof(szPrintTemp));
					inOffset = 0;

					/* 若句末為非空白，則往前找直到可以段句不斷字 */
					while(uszTemp[inPrintHandle + inLineLength - inOffset] != 0x20)
					{
						inOffset++;
						/* 萬一整行都沒空白，直接印 */
						if ((inLineLength - inOffset) == 0)
						{
							inOffset = 0;
							break;
						}
					}
					memcpy(szPrintTemp, &uszTemp[inPrintHandle], (int)inLineLength - inOffset);
					inPrintHandle += inLineLength - inOffset;
					lnReadLength -= inLineLength - inOffset;
				}
				/* 可以直接斷行的狀況 */
				else
				{
					memset(szPrintTemp, 0x00, sizeof(szPrintTemp));
					memcpy(szPrintTemp, &uszTemp[inPrintHandle], (int)inLineLength);
					inPrintHandle += inLineLength;
					lnReadLength -= inLineLength;
				}
			}
			/* 只剩不到一行的狀況 */
			else
			{
				memset(szPrintTemp, 0x00, sizeof(szPrintTemp));
				memcpy(szPrintTemp, &uszTemp[inPrintHandle], (int)lnReadLength);
				inPrintHandle += lnReadLength;
				lnReadLength -= lnReadLength;
			}

			inRetVal = inDISP_ChineseFont(szPrintTemp, inFontSize, inCurrentLine, _DISP_LEFT_);
			if (inRetVal != VS_SUCCESS)
			{
				inRetVal2 = VS_ERROR;
				break;
			}
			else
			{
				inCurrentLine += 1;
			}

		}
		
		break;
	}while(1);
	
	/* release */
        /* 關檔 */
        inFILE_Close(&ulFile_Handle);

        /* Free pointer */
        free(uszTemp);
	
	if (inRetVal2 == VS_SUCCESS)
	{
		return (VS_SUCCESS);
	}
	else
	{
		return (VS_ERROR);
	}
}

/*
Function        :inNCCC_DCC_Clean_Parameter
Date&Time       :2019/12/25 下午 2:00
Describe        :清除舊參數
*/
int inNCCC_DCC_Clean_Parameter(void)
{
	/* DCCBin.dat */
	inFILE_Delete((unsigned char*)_NCCC_DCC_FILE_CARD_BIN_);

	/* ERS.dat */
	inFILE_Delete((unsigned char*)_NCCC_DCC_FILE_EXCHANGE_RATE_SOURCE_);
	
	/* VISADis.dat */
	inFILE_Delete((unsigned char*)_NCCC_DCC_FILE_VISA_DISCLAIMER_);
	
	/* MASTERDis.dat */
	inFILE_Delete((unsigned char*)_NCCC_DCC_FILE_MASTERCARD_DISCLAIMER_);
	
	/* CCI_V.dat */
	inFILE_Delete((unsigned char*)_NCCC_DCC_FILE_VISA_SUPPORTED_CURRENCY_LIST_);
	
	/* CCI_M.dat */
	inFILE_Delete((unsigned char*)_NCCC_DCC_FILE_MASTERCARD_SUPPORTED_CURRENCY_LIST_);
	
	return (VS_TRUE);
}

/*
Function        :vdNCCC_DCC_ISO_FormatDebug_DISP
Date&Time       :2016/11/30 下午 6:37
Describe        :顯示ISO Debug
*/
void vdNCCC_DCC_ISO_FormatDebug_DISP(TRANSACTION_OBJECT *pobTran, unsigned char *uszDebugBuf, int inSendLen)
{
	int			inOneLineLen = 34;
	int			i, inPrintLineCnt = 0, inCnt = 0, inField, inLen, inFieldLen;
	int			inBitMapCnt1, inBitMapCnt2;
	char			szPrtBuf[50 + 1], szBuf[1536 + 1], szBitMap[8 + 1];
	char			szPrintLineData[36 + 1];
	unsigned char		uszBitMap[9], uszBitMapDisp[66];
	ISO_TYPE_NCCC_DCC_TABLE srISOTypeTable;

	memset((char *)&srISOTypeTable, 0x00, sizeof(srISOTypeTable));
	memcpy((char *)&srISOTypeTable, (char *)&srNCCC_DCC_ISOFunc[0], sizeof(srISOTypeTable));

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
		if (!inNCCC_DCC_BitMapCheck((unsigned char *)szBitMap, i))
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

		inField = inNCCC_DCC_GetFieldIndex(i, srISOTypeTable.srISOFieldType);
		inFieldLen = inNCCC_DCC_GetFieldLen(i, &uszDebugBuf[inCnt], srISOTypeTable.srISOFieldType);

		switch (srISOTypeTable.srISOFieldType[inField].inFieldType)
		{
			case _NCCC_DCC_ISO_ASC_  :
				memcpy(&szBuf[inLen], &uszDebugBuf[inCnt], inFieldLen);
				break;
                        case _NCCC_DCC_ISO_BYTE_3_  :
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
                        case _NCCC_DCC_ISO_BYTE_1_ :
                                inFunc_BCD_to_ASCII(&szBuf[inLen], &uszDebugBuf[inCnt ++], 1);
				inLen ++;
				strcat(szBuf, " ");
				inLen ++;

				inFieldLen --;
                                inFunc_BCD_to_ASCII(&szBuf[inLen], &uszDebugBuf[inCnt], inFieldLen);
				break;
			case _NCCC_DCC_ISO_NIBBLE_2_  :
			case _NCCC_DCC_ISO_BYTE_2_  :
                                inFunc_BCD_to_ASCII(&szBuf[inLen], &uszDebugBuf[inCnt ++], 1);
				inLen += 2;
				strcat(szBuf, " ");
				inLen ++;

				inFieldLen --;
				if (srISOTypeTable.srISOFieldType[inField].inFieldType == _NCCC_DCC_ISO_BYTE_3_ ||
				    srISOTypeTable.srISOFieldType[inField].inFieldType == _NCCC_DCC_ISO_BYTE_3_H_)
				{
                                        inFunc_BCD_to_ASCII(&szBuf[inLen], &uszDebugBuf[inCnt ++], 1);
					inLen += 2;
					inFieldLen --;
				}

			case _NCCC_DCC_ISO_BCD_  :
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

		if (i == 55)
			vdNCCC_DCC_ISO_FormatDebug_DISP_EMV(&uszDebugBuf[inCnt], inFieldLen);
		else if (i == 56)
			vdNCCC_DCC_ISO_FormatDebug_DISP_EMV(&uszDebugBuf[inCnt], inFieldLen);
		else if (i == 58)
			vdNCCC_DCC_ISO_FormatDebug_DISP_58(&uszDebugBuf[inCnt], inFieldLen);
		else if (i == 59)
			vdNCCC_DCC_ISO_FormatDebug_DISP_59(&uszDebugBuf[inCnt], inFieldLen);
                
		inCnt += inFieldLen;
	}

        inLogPrintf(AT, "==========================================");
}

/*
Function        :vdNCCC_DCC_ISO_FormatDebug_DISP_EMV
Date&Time       :2016/11/30 下午 6:38
Describe        :
*/
void vdNCCC_DCC_ISO_FormatDebug_DISP_EMV(unsigned char *uszDebugBuf, int inFieldLen)
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
Function        :vdNCCC_DCC_ISO_FormatDebug_DISP_58
Date&Time       :2016/11/30 下午 6:38
Describe        :
*/
void vdNCCC_DCC_ISO_FormatDebug_DISP_58(unsigned char *uszDebugBuf, int inFieldLen)
{
	int	inCnt = 0, inTableLen = 0;
	int	inPrintLineCnt = 0, inOneLineLen = 34;
	char	szTagData[38 + 1];
	char	szPrintBuf[1024 + 1], szTemplate[1024 + 1];

	while (inFieldLen > inCnt)
	{
		if (!memcmp(&uszDebugBuf[inCnt], "S1", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "S2", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "S3", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "S4", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "S5", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "S6", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "S7", 2))
		{
			/* Table ID */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 2);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, " Table ID   [%s]", szTemplate);
                        inLogPrintf(AT, szPrintBuf);
			inCnt += 2;
			/* Table Length */
			inTableLen = (uszDebugBuf[inCnt] / 16 * 10) + uszDebugBuf[inCnt] % 16;
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, " Table Len  [%02X][%d]", uszDebugBuf[inCnt], inTableLen);
			inLogPrintf(AT, szPrintBuf);
			inCnt ++;
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
				while ((inPrintLineCnt * inOneLineLen) < strlen(szTemplate))
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					memset(szTagData, 0x00, sizeof(szTagData));
					if (((inPrintLineCnt + 1) * inOneLineLen) > strlen(szTemplate))
					{
						strcat(szTagData, &szTemplate[inPrintLineCnt * inOneLineLen]);
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
		else
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, " F_58 OTHER DATA [%02X]", uszDebugBuf[inCnt]);
                        inLogPrintf(AT, szPrintBuf);
			inCnt ++;
		}
		
	}
	
}

/*
Function        :vdNCCC_DCC_ISO_FormatDebug_DISP_59
Date&Time       :2016/11/30 下午 6:38
Describe        :
*/
void vdNCCC_DCC_ISO_FormatDebug_DISP_59(unsigned char *uszDebugBuf, int inFieldLen)
{
	int	inCnt = 0, inTableLen, i = 4, j, inFlag;
	char	szPrintBuf[1024 + 1], szTemplate[1024 + 1];

	while (inFieldLen > inCnt)
	{
		switch (uszDebugBuf[inCnt])
		{
			case 'R' :
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
				/* Foreign Currency No.For Rate (3 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 3);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Foreign Currency No.For Rate = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 3;
				/* Action Code (4 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 4);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Action Code = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 4;
				/* Foreign Currency Number (3 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 3);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Foreign Currency Number = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 3;
				/* Foreign Currency Amount (12 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 12);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Foreign Currency Amount = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 12;
				/* Foreign Currency Minor Unit (1 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 1);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Foreign Currency Minor Unit = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 1;
				/* Foreign Currency Alphabetic Code (3 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 3);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Foreign Currency Alphabetic Code = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 3;
				/* Exchange Rate Minor Unit (1 Byte ) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 1);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Exchange Rate Minor Unit = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 1;
				/* Exchange Rate Value (9 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 9);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Exchange Rate Value = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 9;
				/* Inverted Rate Minor Unit (1 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 1);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Inverted Rate Minor Unit = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 1;
				/* Inverted Rate Value (9 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 9);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Inverted Rate Value = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 9;
				/* Inverted Rate Display Unit (1 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 1);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Inverted Rate Display Unit = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 1;
				/* Markup Percentage Value (8 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 8);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Markup Percentage Value = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 8;
				/* Markup Percentage Decimal Point (1 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 1);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Markup Percentage Decimal Point = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 1;
				/* Commission Value Currency Number (3 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 3);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Commission Value Currency Number = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 3;
				/* Commission Value Currency Amount (12 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 12);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Commission Value Currency Amount = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 12;
				/* Commission Value Minor Unit (1 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 1);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Commission Value Minor Unit = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 1;
				/* Update Call Date & Time (14 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 14);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Update Call Date & Time = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 14;
				break;
			case 'F' :
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
				/* Original transaction date & time(MMDD) (4 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 4);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Original transaction date & time(MMDD) = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 4;
				/* Original transaction amount (12 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 12);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Original transaction amount = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 12;
				/* Foreign Currency Number (3 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 3);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Foreign Currency Number = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 3;
				/* Foreign Currency Amount (12 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 12);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Foreign Currency Amount = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 12;
				/* Foreign Currency Minor Unit (1 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 1);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Foreign Currency Minor Unit = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 1;
				/* Foreign Currency Alphabetic Code (3 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 3);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Foreign Currency Alphabetic Code = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 3;
				/* Inverted Rate Minor Unit (1 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 1);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Inverted Rate Minor Unit = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 1;
				/* Inverted Rate Value (9 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 9);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Inverted Rate Value = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 9;
				/* Inverted Rate Display Unit (1 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 1);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Inverted Rate Display Unit = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 1;
				/* Markup Percentage Value (8 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 8);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Markup Percentage Value = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 8;
				/* Markup Percentage Decimal Point (1 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 1);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Markup Percentage Decimal Point = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 1;
				/* Commission Value Currency Number (3 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 3);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Commission Value Currency Number = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 3;
				/* Commission Value Currency Amount (12 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 12);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Commission Value Currency Amount = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 12;
				/* Commission Value Minor Unit (1 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 1);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Commission Value Minor Unit = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 1;
				break;
			case 'D' :
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
				/* Foreign Currency Number (3 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 3);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Foreign Currency Number = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 3;
				/* Foreign Currency Amount (12 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 12);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Foreign Currency Amount = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 12;
				/* Foreign Currency Minor Unit (1 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 1);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Foreign Currency Minor Unit = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 1;
				break;
			case 'E' :
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
				/* Exchange Rate Minor Unit (1 Byte ) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 1);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Exchange Rate Minor Unit = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 1;
				/* Exchange Rate Value (9 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 9);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Exchange Rate Value = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 9;
				break;
			case 'O' :
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
				/* Original Transaction Date (MMDD) (4 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 4);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Original Transaction Date(MMDD) = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 4;
				/* Original Amount (12 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 12);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Original Amount = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 12;
				break;
			case 'G' :
				/* Table ID */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				szTemplate[0] = uszDebugBuf[inCnt];
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "  Table ID = %2x / %s", uszDebugBuf[inCnt], szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt ++;
				/* Table Length */
				inTableLen = uszDebugBuf[inCnt];
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "  Table Len = %2x / %d", uszDebugBuf[inCnt], inTableLen);
				inLogPrintf(AT, szPrintBuf);
				inCnt ++;
				/* Installation Indicator (1 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 1);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Installation Indicator = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 1;
				/* BIN Table Version (4 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 4);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   BIN Table Version = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 4;
				/* Batch/File transfer message count (8 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 8);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Batch/File transfer message count = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 8;
				/* Batch/File transfer file identfication (32 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 32);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Batch/File transfer file identfication = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 32;
				/* File Transfer file size (6 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 6);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   File Transfer file size = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 6;
				/* File transfer elementary data record count (6 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 6);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   File transfer elementary data record count = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 6;
				/* File transfer remaining elementary data record count (6 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 6);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   File transfer remaining elementary data record count = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 6;
				/* Available Parameters (1 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 1);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Available Parameters = 0x%02X", szTemplate[0]);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 8;
				break;
			case 'H' :
				/* Table ID */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				szTemplate[0] = uszDebugBuf[inCnt];
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "  Table ID = %2x / %s", uszDebugBuf[inCnt], szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt ++;
				/* Table Length */
				inTableLen = ((uszDebugBuf[inCnt] % 16) * 256) +
				             ((uszDebugBuf[inCnt + 1] / 16) * 16) + (uszDebugBuf[inCnt + 1] % 16);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "  Table Len = %2x%2x / %d", uszDebugBuf[inCnt],uszDebugBuf[inCnt + 1], inTableLen);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 2;
				/* Function Code (3 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 3);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

				if (!memcmp(&szTemplate[0], "841", 3))
				        inFlag = 1;
				else
				        inFlag = 2;

				sprintf(szPrintBuf, "   Function Code = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 3;
				/* Batch/File transfer message count (8 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 8);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Batch/File transfer message count = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 8;
				/* Batch/File transfer file identfication (32 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 32);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Batch/File transfer file identfication = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 32;
				/* File Transfer file size (6 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 6);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   File Transfer file size = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 6;
				/* File transfer elementary data record count (6 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 6);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   File transfer elementary data record count = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 6;
				/* File transfer remaining elementary data record count (6 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 6);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   File transfer remaining elementary data record count = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 6;
				/* Action Code (4 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 4);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Action Code = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 4;
				/* Data Record (max to 250 Byte + 2 Byte len) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], (inTableLen - 65));
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				inLogPrintf(AT, "   Data Record = ");
				sprintf(&szPrintBuf[0],"%2c%2c", szTemplate[0], szTemplate[1]);
				/* sprintf(&szPrintBuf[0],"%02x%02x", szTemplate[0], szTemplate[1]); */

				if (inFlag == 1)
				{
        				for (j = 2; j < (inTableLen - 65) ;j ++)
        				{
        				        inFunc_BCD_to_ASCII(&szTemplate[j], (unsigned char*)&szPrintBuf[i], 1);
        				        i += 2;
        				}
        			}
        			else
        			{
        			        memcpy(&szPrintBuf[4], szTemplate, (inTableLen - 65));
        			}

				inLogPrintf(AT, szPrintBuf);
				inCnt += (inTableLen - 65);
				break;
			case 'I' :
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
				/* Action Code (4 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 4);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Action Code = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 4;
				/* Parameter Update Result Code (1 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 1);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Parameter Update Result Code = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt ++;
				/* Bin Table Update Result Code (1 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 1);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Bin Table Update Result Code = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt ++;
				break;
			case 'X' :
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
				/* Data Record ( 220 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], inTableLen);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				inLogPrintf(AT, "   Data Record = ");
				sprintf(&szPrintBuf[0],"%s", &szTemplate[0]);
				inLogPrintf(AT, szPrintBuf);
				inCnt += inTableLen;
				break;
			case 'N' :
                                /* 【需求單 - 104024】上傳電子簽單至ESC系統 by Tusin - 2015/12/23 下午 02:44:03 */
			        if (uszDebugBuf[inCnt + 1] == 'E')
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
				else if (uszDebugBuf[inCnt + 1] == 'C')
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

					memset(szTemplate, 0x00, sizeof(szTemplate));
					memcpy(szTemplate, &uszDebugBuf[inCnt], inTableLen);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "  NC = %s", szTemplate);
					inLogPrintf(AT, szPrintBuf);
					inCnt ++;
				}
				else if (uszDebugBuf[inCnt + 1] == 'D')
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
					memcpy(szTemplate, &uszDebugBuf[inCnt], inTableLen);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "  ND = %s", szTemplate);
					inLogPrintf(AT, szPrintBuf);
					inCnt ++;
				}
				else
				{
					inCnt ++;
				}
				break;	
			case 'U' :
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
				/* 01 (2 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], inTableLen);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   DCC Flow Information = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += inTableLen;
				break;
			default :
				inCnt ++;
				break;
		}
		
	}
}

/*
Function        :vdNCCC_DCC_ISO_FormatDebug_PRINT
Date&Time       :2016/11/30 下午 6:37
Describe        :顯示ISO Debug
*/
void vdNCCC_DCC_ISO_FormatDebug_PRINT(TRANSACTION_OBJECT *pobTran, unsigned char *uszDebugBuf, int inSendLen)
{
	int			inOneLineLen = 34;
	int			i, inPrintLineCnt = 0, inCnt = 0, inField, inLen, inFieldLen;
	int			inBitMapCnt1, inBitMapCnt2;
	char			szPrintLineData[36 + 1];
	char			szPrtBuf[50], szBuf[1536 + 1], szBitMap[8 + 1];
	unsigned char		uszBitMap[9], uszBitMapDisp[66];
	unsigned char		uszBuffer[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	BufferHandle		srBhandle;
	FONT_ATTRIB		srFont_Attrib;
	ISO_TYPE_NCCC_DCC_TABLE srISOTypeTable;
	
	/* 是否有列印功能 */
	if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
	{
		
	}
	else
	{
		/* 初始化 */
		inPRINT_Buffer_Initial(uszBuffer, _BUFFER_MAX_LINE_, &srFont_Attrib, &srBhandle);

		memset((char *)&srISOTypeTable, 0x00, sizeof(srISOTypeTable));
		memcpy((char *)&srISOTypeTable, (char *)&srNCCC_DCC_ISOFunc[0], sizeof(srISOTypeTable));

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
			if (!inNCCC_DCC_BitMapCheck((unsigned char *)szBitMap, i))
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

			inField = inNCCC_DCC_GetFieldIndex(i, srISOTypeTable.srISOFieldType);
			inFieldLen = inNCCC_DCC_GetFieldLen(i, &uszDebugBuf[inCnt], srISOTypeTable.srISOFieldType);

			switch (srISOTypeTable.srISOFieldType[inField].inFieldType)
			{
				case _NCCC_DCC_ISO_ASC_  :
					memcpy(&szBuf[inLen], &uszDebugBuf[inCnt], inFieldLen);
					break;
				case _NCCC_DCC_ISO_BYTE_3_  :
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
				case _NCCC_DCC_ISO_BYTE_1_ :
					inFunc_BCD_to_ASCII(&szBuf[inLen], &uszDebugBuf[inCnt ++], 1);
					inLen ++;
					strcat(szBuf, " ");
					inLen ++;

					inFieldLen --;
					inFunc_BCD_to_ASCII(&szBuf[inLen], &uszDebugBuf[inCnt], inFieldLen);
					break;
				case _NCCC_DCC_ISO_NIBBLE_2_  :
				case _NCCC_DCC_ISO_BYTE_2_  :
					inFunc_BCD_to_ASCII(&szBuf[inLen], &uszDebugBuf[inCnt ++], 1);
					inLen += 2;
					strcat(szBuf, " ");
					inLen ++;

					inFieldLen --;
					if (srISOTypeTable.srISOFieldType[inField].inFieldType == _NCCC_DCC_ISO_BYTE_3_ ||
					    srISOTypeTable.srISOFieldType[inField].inFieldType == _NCCC_DCC_ISO_BYTE_3_H_)
					{
						inFunc_BCD_to_ASCII(&szBuf[inLen], &uszDebugBuf[inCnt ++], 1);
						inLen += 2;
						inFieldLen --;
					}

				case _NCCC_DCC_ISO_BCD_  :
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

			if (i == 55)
				vdNCCC_DCC_ISO_FormatDebug_PRINT_EMV(&uszDebugBuf[inCnt], inFieldLen, uszBuffer, &srFont_Attrib, &srBhandle);
			else if (i == 56)
				vdNCCC_DCC_ISO_FormatDebug_PRINT_EMV(&uszDebugBuf[inCnt], inFieldLen, uszBuffer, &srFont_Attrib, &srBhandle);
			else if (i == 58)
				vdNCCC_DCC_ISO_FormatDebug_PRINT_58(&uszDebugBuf[inCnt], inFieldLen, uszBuffer, &srFont_Attrib, &srBhandle);
			else if (i == 59)
				vdNCCC_DCC_ISO_FormatDebug_PRINT_59(&uszDebugBuf[inCnt], inFieldLen, uszBuffer, &srFont_Attrib, &srBhandle);

			inCnt += inFieldLen;
		}

		inPRINT_Buffer_PutIn("==========================================", _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		inPRINT_Buffer_OutPut(uszBuffer, &srBhandle);
	}
}

/*
Function        :vdNCCC_DCC_ISO_FormatDebug_PRINT_EMV
Date&Time       :2016/11/30 下午 6:38
Describe        :
*/
void vdNCCC_DCC_ISO_FormatDebug_PRINT_EMV(unsigned char *uszDebugBuf, int inFieldLen, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
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
Function        :vdNCCC_DCC_ISO_FormatDebug_PRINT_58
Date&Time       :2016/11/30 下午 6:38
Describe        :
*/
void vdNCCC_DCC_ISO_FormatDebug_PRINT_58(unsigned char *uszDebugBuf, int inFieldLen, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inCnt = 0, inTableLen = 0;
	int	inPrintLineCnt = 0, inOneLineLen = 34;
	char	szTagData[38 + 1];
	char	szPrintBuf[1024 + 1], szTemplate[1024 + 1];

	while (inFieldLen > inCnt)
	{
		if (!memcmp(&uszDebugBuf[inCnt], "S1", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "S2", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "S3", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "S4", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "S5", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "S6", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "S7", 2))
		{
			/* Table ID */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 2);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, " Table ID   [%s]", szTemplate);
                        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += 2;
			/* Table Length */
			inTableLen = (uszDebugBuf[inCnt] / 16 * 10) + uszDebugBuf[inCnt] % 16;
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, " Table Len  [%02X][%d]", uszDebugBuf[inCnt], inTableLen);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt ++;
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
				while ((inPrintLineCnt * inOneLineLen) < strlen(szTemplate))
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					memset(szTagData, 0x00, sizeof(szTagData));
					if (((inPrintLineCnt + 1) * inOneLineLen) > strlen(szTemplate))
					{
						strcat(szTagData, &szTemplate[inPrintLineCnt * inOneLineLen]);
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
		else
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, " F_58 OTHER DATA [%02X]", uszDebugBuf[inCnt]);
                        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt ++;
		}
		
	}
	
}

/*
Function        :vdNCCC_DCC_ISO_FormatDebug_PRINT_59
Date&Time       :2016/11/30 下午 6:38
Describe        :
*/
void vdNCCC_DCC_ISO_FormatDebug_PRINT_59(unsigned char *uszDebugBuf, int inFieldLen, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inCnt = 0, inTableLen, i = 4, j, inFlag;
	char	szPrintBuf[1024 + 1], szTemplate[1024 + 1];

	while (inFieldLen > inCnt)
	{
		switch (uszDebugBuf[inCnt])
		{
			case 'R' :
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
				/* Foreign Currency No.For Rate (3 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 3);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Foreign C No.For Rate = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 3;
				/* Action Code (4 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 4);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Action Code = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 4;
				/* Foreign Currency Number (3 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 3);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Foreign C Number = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 3;
				/* Foreign Currency Amount (12 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 12);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Foreign C Amount = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 12;
				/* Foreign Currency Minor Unit (1 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 1);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Foreign C M Unit = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 1;
				/* Foreign Currency Alphabetic Code (3 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 3);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Foreign C A Code = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 3;
				/* Exchange Rate Minor Unit (1 Byte ) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 1);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Exchange R M Unit = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 1;
				/* Exchange Rate Value (9 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 9);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Exchange R Value = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 9;
				/* Inverted Rate Minor Unit (1 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 1);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Inverted R M Unit = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 1;
				/* Inverted Rate Value (9 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 9);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Inverted R Value = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 9;
				/* Inverted Rate Display Unit (1 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 1);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Inverted R D Unit = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 1;
				/* Markup Percentage Value (8 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 8);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Markup P Value = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 8;
				/* Markup Percentage Decimal Point (1 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 1);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Markup P D Point = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 1;
				/* Comm Value Currency Number (3 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 3);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Comm V C Number = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 3;
				/* Comm Value Currency Amount (12 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 12);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Comm V C Amount = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 12;
				/* Comm Value Minor Unit (1 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 1);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Comm V M Unit = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 1;
				/* Update Call Date & Time (14 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 14);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   U C Date & Time = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 14;
				break;
			case 'F' :
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
				/* Original transaction date & time(MMDD) (4 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 4);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   ORG T date & time(MMDD) = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 4;
				/* Original transaction amount (12 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 12);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   ORG T amount = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 12;
				/* Foreign Currency Number (3 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 3);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Foreign C Number = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 3;
				/* Foreign Currency Amount (12 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 12);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Foreign C Amount = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 12;
				/* Foreign Currency Minor Unit (1 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 1);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Foreign C M Unit = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 1;
				/* Foreign Currency Alphabetic Code (3 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 3);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Foreign C A Code = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 3;
				/* Inverted Rate Minor Unit (1 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 1);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Inverted R M Unit = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 1;
				/* Inverted Rate Value (9 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 9);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Inverted R Value = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 9;
				/* Inverted Rate Display Unit (1 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 1);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Inverted R D Unit = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 1;
				/* Markup Percentage Value (8 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 8);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Markup P Value = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 8;
				/* Markup Percentage Decimal Point (1 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 1);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Markup P D Point = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 1;
				/* Comm Value Currency Number (3 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 3);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Comm V C Number = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 3;
				/* Comm Value Currency Amount (12 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 12);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Comm V C Amount = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 12;
				/* Comm Value Minor Unit (1 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 1);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Comm V M Unit = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 1;
				break;
			case 'D' :
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
				/* Foreign Currency Number (3 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 3);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Foreign C Number = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 3;
				/* Foreign Currency Amount (12 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 12);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Foreign C Amount = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 12;
				/* Foreign Currency Minor Unit (1 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 1);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Foreign C M Unit = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 1;
				break;
			case 'E' :
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
				/* Exchange Rate Minor Unit (1 Byte ) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 1);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Exchange R M Unit = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 1;
				/* Exchange Rate Value (9 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 9);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Exchange R Value = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 9;
				break;
			case 'O' :
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
				/* Original Transaction Date (MMDD) (4 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 4);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   ORG T Date(MMDD) = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 4;
				/* Original Amount (12 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 12);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   ORG Amount = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 12;
				break;
			case 'G' :
				/* Table ID */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				szTemplate[0] = uszDebugBuf[inCnt];
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "  Table ID = %2x / %s", uszDebugBuf[inCnt], szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt ++;
				/* Table Length */
				inTableLen = uszDebugBuf[inCnt];
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "  Table Len = %2x / %d", uszDebugBuf[inCnt], inTableLen);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt ++;
				/* Installation Indicator (1 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 1);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Installation Indicator = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 1;
				/* BIN Table Version (4 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 4);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   BIN Table Version = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 4;
				/* Batch/File transfer message count (8 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 8);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Batch/File T M count = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 8;
				/* Batch/File transfer file identfication (32 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 32);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Batch/File T F identfication = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 32;
				/* File Transfer file size (6 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 6);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   File T F size = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 6;
				/* File transfer elementary data record count (6 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 6);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   File T E D R count = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 6;
				/* File transfer remaining elementary data record count (6 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 6);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   File T R E D R count = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 6;
				/* Available Parameters (1 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 1);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Available Parameters = 0x%02X", szTemplate[0]);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 8;
				break;
			case 'H' :
				/* Table ID */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				szTemplate[0] = uszDebugBuf[inCnt];
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "  Table ID = %2x / %s", uszDebugBuf[inCnt], szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt ++;
				/* Table Length */
				inTableLen = ((uszDebugBuf[inCnt] % 16) * 256) +
				             ((uszDebugBuf[inCnt + 1] / 16) * 16) + (uszDebugBuf[inCnt + 1] % 16);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "  Table Len = %2x%2x / %d", uszDebugBuf[inCnt],uszDebugBuf[inCnt + 1], inTableLen);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 2;
				/* Function Code (3 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 3);
				
				if (!memcmp(&szTemplate[0], "841", 3))
				        inFlag = 1;
				else
				        inFlag = 2;
				
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Function Code = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 3;
				/* Batch/File transfer message count (8 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 8);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Batch/File T M count = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 8;
				/* Batch/File transfer file identfication (32 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 32);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Batch/File T F Identfication = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 32;
				/* File Transfer file size (6 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 6);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   File T F size = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 6;
				/* File transfer elementary data record count (6 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 6);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   File T E D R count = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 6;
				/* File transfer remaining elementary data record count (6 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 6);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   File T R E D R count = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 6;
				/* Action Code (4 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 4);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Action Code = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 4;
				/* Data Record (max to 250 Byte + 2 Byte len) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], (inTableLen - 65));
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				inPRINT_Buffer_PutIn("   Data Record = ", _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				sprintf(&szPrintBuf[0],"%2c%2c", szTemplate[0], szTemplate[1]);
				/* sprintf(&szPrintBuf[0],"%02x%02x", szTemplate[0], szTemplate[1]); */

				if (inFlag == 1)
				{
        				for (j = 2; j < (inTableLen - 65) ;j ++)
        				{
        				        inFunc_BCD_to_ASCII(&szTemplate[j], (unsigned char*)&szPrintBuf[i], 1);
        				        i += 2;
        				}
        			}
        			else
        			{
        			        memcpy(&szPrintBuf[4], szTemplate, (inTableLen - 65));
        			}

				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += (inTableLen - 65);
				break;
			case 'I' :
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
				/* Action Code (4 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 4);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Action Code = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 4;
				/* Parameter Update Result Code (1 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 1);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Parameter U R Code = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt ++;
				/* Bin Table Update Result Code (1 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], 1);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Bin Table U R Code = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt ++;
				break;
			case 'X' :
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
				/* Data Record ( 220 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], inTableLen);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				inPRINT_Buffer_PutIn("   Data Record = ", _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				sprintf(&szPrintBuf[0],"%s", &szTemplate[0]);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += inTableLen;
				break;
			case 'N' :
                                /* 【需求單 - 104024】上傳電子簽單至ESC系統 by Tusin - 2015/12/23 下午 02:44:03 */
			        if (uszDebugBuf[inCnt + 1] == 'E')
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
				else if (uszDebugBuf[inCnt + 1] == 'C')
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
					
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memcpy(szTemplate, &uszDebugBuf[inCnt], inTableLen);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "  NC = %s", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					inCnt ++;
				}
				else if (uszDebugBuf[inCnt + 1] == 'D')
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
					memcpy(szTemplate, &uszDebugBuf[inCnt], inTableLen);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "  ND = %s", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					inCnt ++;
				}
				else
				{
					inCnt ++;
				}
				break;	
			case 'U' :
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
				/* 01 (2 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszDebugBuf[inCnt], inTableLen);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   DCC Flow Information = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += inTableLen;
				break;
			default :
				inCnt ++;
				break;
		}
	}
}

