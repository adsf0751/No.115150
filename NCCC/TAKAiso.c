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
#include "../SOURCE/FUNCTION/EDC.h"
#include "../SOURCE/FUNCTION/HDT.h"
#include "../SOURCE/FUNCTION/HDPT.h"
#include "../SOURCE/FUNCTION/CPT.h"
#include "../SOURCE/FUNCTION/CFGT.h"
#include "../SOURCE/FUNCTION/MVT.h"
#include "../SOURCE/FUNCTION/KMS.h"
#include "../SOURCE/FUNCTION/Signpad.h"
#include "../SOURCE/FUNCTION/NexsysSDK.h"
#include "../SOURCE/FUNCTION/Utility.h"
#include "../SOURCE/FUNCTION/VWT.h"
#include "../SOURCE/FUNCTION/CDTX.h"
#include "../SOURCE/FUNCTION/ECR.h"
#include "../SOURCE/COMM/Comm.h"
#include "../CREDIT/Creditfunc.h"
#include "../CREDIT/CreditprtByBuffer.h"
#include "../CTLS/CTLS.h"
#include "../FISC/NCCCfisc.h"
#include "../EMVSRC/EMVsrc.h"
#include "NCCCats.h"
#include "NCCCsrc.h"
#include "NCCCtSAM.h"
#include "NCCCtmk.h"
#include "NCCCesc.h"
#include "NCCCdcc.h"
#include "NCCCtmsCPT.h"
#include "NCCCloyalty.h"
#include "TAKAsrc.h"
#include "TAKAiso.h"

int			ginTAKAHostIndex = -1;
int			ginTAKACommIndex = -1;
unsigned char		guszTAKA_ISO_Field03[_TAKA_PCODE_SIZE_ + 1];
unsigned char		guszTAKA_MTI[4 + 1];
extern	int		ginDebug; /* Debug使用 extern */
extern	int		ginISODebug;
extern	int		ginDisplayDebug;
extern	int		ginMachineType;
extern	int		ginPOSPrinterDebug;
extern	int		ginAPVersionType;
extern	char		gszTermVersionID[16 + 1];
extern	unsigned char	guszEnormousNoNeedResetBit;	/* 資料庫是否須從頭找 */

/* 傳回主機結果用 */
extern EMV_CONFIG	EMVGlobConfig;

ISO_FIELD_TAKA_TABLE srTAKA_ISOFieldPack[] =
{
	{2,             inTAKA_Pack02},
        {3,             inTAKA_Pack03},
        {4,             inTAKA_Pack04},
        {11,            inTAKA_Pack11},
        {12,            inTAKA_Pack12},
        {13,            inTAKA_Pack13},
	{14,		inTAKA_Pack14},
        {22,            inTAKA_Pack22},
        {24,            inTAKA_Pack24},
        {25,            inTAKA_Pack25},
        {35,            inTAKA_Pack35},
        {37,            inTAKA_Pack37},
        {38,            inTAKA_Pack38},
	{38,            inTAKA_Pack39},
        {41,            inTAKA_Pack41},
        {42,            inTAKA_Pack42},
        {48,            inTAKA_Pack48},
        {54,            inTAKA_Pack54},
        {60,            inTAKA_Pack60},
        {62,            inTAKA_Pack62},
        {63,            inTAKA_Pack63},
        {0,             NULL},  /* 最後一組一定要放 0 */
};

ISO_FIELD_TAKA_TABLE srTAKA_ISOFieldUnPack[] =
{
        {12,            inTAKA_UnPack12},
        {13,            inTAKA_UnPack13},
        {37,            inTAKA_UnPack37},
        {38,            inTAKA_UnPack38},
        {39,            inTAKA_UnPack39},
        {60,            inTAKA_UnPack60},
        {0,             NULL}, /* 最後一組一定要放【0】!! */
};

ISO_CHECK_TAKA_TABLE srTAKA_ISOFieldCheck[] =
{
        {3,             inTAKA_Check03},
        {4,             inTAKA_Check04},
        {41,            inTAKA_Check41},
        {0,		NULL}, /* 最後一組一定要放【0】!! */
};

ISO_FIELD_TYPE_TAKA_TABLE srTAKA_ISOFieldType[] =
{
	{2,             _TAKA_ISO_NIBBLE_2_,	VS_FALSE,       0},
        {3,             _TAKA_ISO_BCD_,         VS_FALSE,       6},
        {4,             _TAKA_ISO_BCD_,         VS_FALSE,       12},
        {11,            _TAKA_ISO_BCD_,         VS_FALSE,       6},
        {12,            _TAKA_ISO_BCD_,         VS_FALSE,       6},
        {13,            _TAKA_ISO_BCD_,         VS_FALSE,       4},
	{14,            _TAKA_ISO_BCD_,         VS_FALSE,       4},
        {22,            _TAKA_ISO_BCD_,         VS_FALSE,       4},
        {24,            _TAKA_ISO_BCD_,         VS_FALSE,       4},
        {25,            _TAKA_ISO_BCD_,         VS_FALSE,       2},
	{27,            _TAKA_ISO_BCD_,         VS_FALSE,       2},
        {35,            _TAKA_ISO_NIBBLE_2_,    VS_FALSE,       0},
        {37,            _TAKA_ISO_ASC_,         VS_FALSE,       12},
        {38,            _TAKA_ISO_ASC_,         VS_FALSE,       6},
        {39,            _TAKA_ISO_ASC_,         VS_FALSE,       2},
        {41,            _TAKA_ISO_ASC_,         VS_FALSE,       8},
        {42,            _TAKA_ISO_ASC_,         VS_FALSE,       15},
        {48,            _TAKA_ISO_BYTE_3_,      VS_TRUE,        0},
        {54,            _TAKA_ISO_BYTE_3_,      VS_TRUE,        0},
        {55,            _TAKA_ISO_BYTE_3_,      VS_FALSE,       0},
	{56,            _TAKA_ISO_BYTE_3_,      VS_FALSE,       0},
        {57,            _TAKA_ISO_BYTE_3_,      VS_FALSE,       0},
        {58,            _TAKA_ISO_BYTE_3_,      VS_TRUE,        0},
        {59,            _TAKA_ISO_BYTE_3_,      VS_TRUE,        0},
        {60,            _TAKA_ISO_BYTE_3_,      VS_TRUE,        0},
	{61,            _TAKA_ISO_BYTE_3_,      VS_TRUE,        0},
        {62,            _TAKA_ISO_BYTE_3_,      VS_TRUE,        0},
        {63,            _TAKA_ISO_BYTE_3_,      VS_TRUE,        0},
        {0,             _TAKA_ISO_BCD_,         VS_FALSE,       0},  /* 最後一組一定要放 0 */
};

int inTAKA_SALE[] = {3, 4, 11, 22, 24, 25, 35, 41, 42, 48, 62, 0}; /* 最後一組一定要放 0!! */
int inTAKA_REFUND[] = {3, 4, 11, 22, 24, 25, 35, 41, 42, 48, 62, 0}; /* 最後一組一定要放 0!! */
int inTAKA_VOID[] = {2, 3, 4, 11, 12, 13, 14, 22, 24, 25, 38, 41, 42, 62, 0}; /* 最後一組一定要放 0!! */
int inTAKA_REVERSAL[] = {2, 3, 4, 11, 14, 22, 24, 25, 41, 42, 62, 0}; /* 最後一組一定要放 0!! */
int inTAKA_SETTLE[] = {3, 11, 24, 41, 42, 48, 60, 63, 0}; /* 最後一組一定要放 0!! */
int inTAKA_CLS_BATCH[] = {3, 11, 24, 41, 42, 60, 63, 0}; /* 最後一組一定要放 0!! */
int inTAKA_BATCH_UPLOAD[] = {2, 3, 4, 11, 12, 13, 14, 22, 24, 25, 38, 39, 41, 42, 60, 62, 0}; /* 最後一組一定要放 0!! */

BIT_MAP_TAKA_TABLE srTAKA_ISOBitMap[] =
{
	{_SALE_,		inTAKA_SALE,		"0200",		"000000"}, /* 銷售 */
	{_REFUND_,		inTAKA_REFUND,		"0200",		"200000"}, /* 退貨 */
	{_VOID_,		inTAKA_VOID,		"0200",		"020000"}, /* 取消 */
	{_REVERSAL_,		inTAKA_REVERSAL,	"0400",		"000000"}, /* 沖銷 */
	{_SETTLE_,		inTAKA_SETTLE,		"0500",		"920000"}, /* 結帳_1 */
	{_CLS_BATCH_,		inTAKA_CLS_BATCH,	"0500",		"960000"}, /* 結帳_2 */
	{_BATCH_UPLOAD_,	inTAKA_BATCH_UPLOAD,	"0320",		"000000"}, /* 批次上傳 */
	{_TAKA_NULL_TX_,		NULL,			"0000",		"000000"}, /* 最後一組一定要放 TMS_NULL_TX!! */
};

ISO_TYPE_TAKA_TABLE srTAKA_ISOFunc[] =
{
        {
           srTAKA_ISOFieldPack,
           srTAKA_ISOFieldUnPack,
           srTAKA_ISOFieldCheck,
           srTAKA_ISOFieldType,
           srTAKA_ISOBitMap,
           inTAKA_ISOPackMessageType,
           inTAKA_ISOModifyBitMap,
           inTAKA_ISOModifyPackData,
           inTAKA_ISOCheckHeader,
           inTAKA_ISOOnlineAnalyse,
           inTAKA_ISOAdviceAnalyse
        },
};

/*
Function        :inTAKA_Func_SetTxnOnlineOffline
Date&Time       :2022/10/14 下午 3:27
Describe        :根據交易別決定是否Online
*/
int inTAKA_Func_SetTxnOnlineOffline(TRANSACTION_OBJECT *pobTran)
{
        vdUtility_SYSFIN_LogMessage(AT, "inTAKA_Func_SetTxnOnlineOffline START!");
        
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inTAKA_Func_SetTxnOnlineOffline START!");
        }
                  
	/* 開始預設值 */
	pobTran->srBRec.uszOfflineBit = VS_FALSE; /* 當筆交易要 Online */
	pobTran->srBRec.uszUpload1Bit = VS_FALSE; /* 不會產生任何 Advice */
	pobTran->srBRec.uszUpload2Bit = VS_FALSE; /* 不會產生任何 Advice */
	
	/* offline交易及settle不需要做reversal */
	pobTran->uszReversalBit = VS_TRUE;

	if (pobTran->inTransactionCode == _SETTLE_	||
	    pobTran->inTransactionCode == _CLS_SETTLE_)
	{
		pobTran->uszReversalBit = VS_FALSE;
	}
	
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inTAKA_Func_SetTxnOnlineOffline END!");
        }    
        
        return (VS_SUCCESS);
}

/*
Function        :inTAKA_Func_BuildAndSendPacket
Date&Time       :2022/10/14 下午 5:07
Describe        :處理交易，發送並分析
*/
int inTAKA_Func_BuildAndSendPacket(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	int	inBatchCnt = 0;
	char	szDemoMode[2 + 1] = {0};
	
        vdUtility_SYSFIN_LogMessage(AT, "inTAKA_Func_BuildAndSendPacket START!");
        
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inTAKA_Func_BuildAndSendPacket() START!");
	
	/* 教育訓練模式 */
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		inRetVal = inNCCC_Func_BuildAndSendPacket_Demo_Flow(pobTran);
		/* 步驟 1 Get System Trans Number */
		inTAKA_GetSTAN(pobTran);
		/* Set STAN */
		inTAKA_SetSTAN(pobTran);
		
		/* 交易失敗 */
		if (inRetVal != VS_SUCCESS)
		{
			return (inRetVal);
		}

		return (VS_SUCCESS);
	}
	else
	{
		/* 如果是結帳交易，要把請先結帳的Bit On起來，直到接收到host的回覆才Off*/
		if (pobTran->inTransactionCode == _SETTLE_)
		{
			/* 如果是結帳交易，要把請先結帳的Bit On起來，直到接收到host的回覆才Off*/
			inBatchCnt = inBATCH_GetTotalCountFromBakFile_By_Sqlite(pobTran);
			if (inBatchCnt >= 0)
			{
				inTAKA_SetMustSettleBit(pobTran, "Y");
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
		if (inTAKA_GetSTAN(pobTran) == VS_ERROR)
		{
			pobTran->inErrorMsg = _ERROR_CODE_V3_GET_HDPT_TAG_FAIL_;
			return (VS_ERROR);
		}

		/* 步驟 1.1 處理online交易 */
		if (pobTran->srBRec.uszOfflineBit == VS_FALSE)
		{
			/* Reset title(只有Online的交易才需要Reset) */
			inFunc_ResetTitle(pobTran);

			if (ginFindRunTime == VS_TRUE)
			{
				inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
			}
			
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
                        
                        /* Mirror Message */
                        if (pobTran->uszECRBit == VS_TRUE)
                        {
                                inECR_SendMirror(pobTran, _MIRROR_MSG_CONNECT_HOST_);
                        }

			if (ginFindRunTime == VS_TRUE)
			{
				inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
			}
			
			/* 步驟 1.3 檢查是否為 Online 交易，先送上筆交易失敗的 Reversal 及產生當筆交易 Reversal */
			if ((inRetVal = inTAKA_ProcessReversal(pobTran)) != VS_SUCCESS)
			{
				inUtility_StoreTraceLog_OneStep("inTAKA_ProcessReversal Error");
				return (inRetVal);
			}
			
			/* 步驟 1.4 */
			if (pobTran->inTransactionCode == _SETTLE_)
			{
				if ((inRetVal = inTAKA_ProcessAdvice(pobTran)) != VS_SUCCESS)
				{
					inUtility_StoreTraceLog_OneStep("inTAKA_ProcessAdvice Error");
					
					/* 只有結帳時前送advice錯誤時要顯示錯誤訊息，在inTAKA_ISOAdviceAnalyse裡顯示錯誤訊息 */
					pobTran->inErrorMsg = _ERROR_CODE_V3_SETTLE_NOT_SUCCESS_;
					return (inRetVal);
				}

				pobTran->inTransactionCode = _SETTLE_;
				pobTran->srBRec.inCode = _SETTLE_;

			}

		}
		
		/* 處理【ONLINE】交易 */
		if (pobTran->srBRec.uszOfflineBit == VS_FALSE)
		{
			if ((inRetVal = inTAKA_ProcessOnline(pobTran)) != VS_SUCCESS)
			{
				inTAKA_SetSTAN(pobTran); /* 成功或失敗 System_Trace_Number 都要加一 */

				return (inRetVal); /* Return 【VS_ERROR】【VS_SUCCESS】 */
			}
		}
		else
		{
			/* 處理【OFFLINE】交易 */
			inRetVal = inTAKA_ProcessOffline(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
		}

		inTAKA_SetSTAN(pobTran); /* 成功或失敗 System_Trace_Number 都要加一 */

		/* 1. 表示有收到完整的資料後的分析
		   2. 處理 Online & Offline 交易
		 */
		inRetVal = inTAKA_AnalysePacket(pobTran);
		if (inRetVal != VS_SUCCESS)
		{
			inUtility_StoreTraceLog_OneStep("inTAKA_AnalysePacket Error");
			
			return (inRetVal);
		}

		/* Reset Ttile */
		inFunc_ResetTitle(pobTran);

		/* 避免撥接太久，收送完就斷線 */
		/* 這裡判斷是否要送TC Upload再決定是否斷線 */
		inCOMM_End(pobTran);

		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "inTAKA_Func_BuildAndSendPacket() END!");

		return (VS_SUCCESS);
	}
}

/*
Function        :inTAKA_Pack02
Date&Time       :2022/10/14 下午 7:27
Describe        :
*/
int inTAKA_Pack02(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int 	inCnt = 0;
	int 	inPANLen = 0;
	char 	szPAN[_PAN_SIZE_ + 1] = {0};

	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inTAKA_Pack02(PAN = %s) START!", pobTran->srBRec.szPAN);
	
	memset(szPAN, 0x00, sizeof(szPAN));
	inPANLen = strlen(pobTran->srBRec.szPAN);
	uszPackBuf[inCnt ++] = (inPANLen / 10 * 16) + (inPANLen % 10);
	memcpy(szPAN, pobTran->srBRec.szPAN, inPANLen);
	if (inPANLen % 2)
		szPAN[inPANLen ++] = '0';

	inFunc_ASCII_to_BCD(&uszPackBuf[inCnt], szPAN, inPANLen / 2);
	inCnt += inPANLen / 2;
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inTAKA_Pack02(%ld) END!!", inCnt);
	
	return (inCnt);
}

/*
Function        :inTAKA_Pack03
Date&Time       :2022/10/14 下午 7:32
Describe        :Field_3:       Processing Code
*/
int inTAKA_Pack03(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 	inCnt = 0;
	char	szDebugMsg[100 + 1];
	char	szAscii[6 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inTAKA_Pack03(inCode = %d) START!!", pobTran->srBRec.inCode);
	
	
	memcpy(&uszPackBuf[inCnt], &guszTAKA_ISO_Field03[0], _TAKA_PCODE_SIZE_);
	
        inCnt += _TAKA_PCODE_SIZE_;
	
	if (ginDebug == VS_TRUE)
	{
		memset(szAscii, 0x00, sizeof(szAscii));
		inFunc_BCD_to_ASCII(szAscii, guszTAKA_ISO_Field03, _TAKA_PCODE_SIZE_);
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "F_03 [P_CODE %s]", szAscii);
		inLogPrintf(AT, szDebugMsg);
	}
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inTMS_ISO_Pack03(%ld) END!!", inCnt);
	
        return (inCnt);
}

/*
Function        :inTAKA_Pack04
Date&Time       :2022/10/14 下午 7:32
Describe        :Field_4:       Amount, Transaction
*/
int inTAKA_Pack04(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 		inCnt = 0;
        char 		szTemplate[100 + 1], szDebugMsg[100 + 1];
	char		szAscii[12 + 1];
        unsigned char 	uszBCD[6 + 1];

	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inTAKA_Pack04() START!");
	
	
        memset(szTemplate, 0x00, sizeof(szTemplate));
        sprintf(szTemplate, "%010ld00", pobTran->srBRec.lnTxnAmount);

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
                inLogPrintf(AT, "inTAKA_Pack04(%ld) END!!", inCnt);
	
        return (inCnt);
}

/*
Function        :inTAKA_Pack11
Date&Time       :2022/10/14 下午 7:35
Describe        :Field_11:      System Trace Audit Number
*/
int inTAKA_Pack11(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 		inCnt = 0;
        char 		szSTAN[6 + 1];
	char		szDebugMsg[100 + 1];
	char		szAscii[6 + 1];
        unsigned char 	uszBCD[3 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inTAKA_Pack11(STAN = %d) START!!", pobTran->srBRec.lnSTANNum);
	

        memset(szSTAN, 0x00, sizeof(szSTAN));
        sprintf(szSTAN, "%06ld", pobTran->srBRec.lnSTANNum);

        memset(uszBCD, 0x00, sizeof(uszBCD));
        inFunc_ASCII_to_BCD(&uszBCD[0], &szSTAN[0], 3);
        memcpy((char *) &uszPackBuf[inCnt], (char *) &uszBCD[0], 3);
        inCnt += 3;
	
	if (ginDebug == VS_TRUE)
	{
		memset(szAscii, 0x00, sizeof(szAscii));
		inFunc_BCD_to_ASCII(szAscii, uszBCD, 3);
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "F_011 [STAN %s]", szAscii);
		inLogPrintf(AT, szDebugMsg);
	}
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inTAKA_Pack11(%ld) END!!", inCnt);
	
        return (inCnt);
}

/*
Function        :inTAKA_Pack12
Date&Time       :2022/10/14 下午 7:36
Describe        :Field_12:      Time, Local Transaction
*/
int inTAKA_Pack12(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 		inCnt = 0;
	char		szDebugMsg[100 + 1];
	char		szAscii[6 + 1];
        unsigned char 	uszBCD[3 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inTAKA_Pack12() START!");
	

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
                inLogPrintf(AT, "inTAKA_Pack12(%ld) END!", inCnt);
	
        return (inCnt);
}

/*
Function        :inTAKA_Pack13
Date&Time       :
Describe        :Field_13:      Date, Local Transaction
*/
int inTAKA_Pack13(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 		inCnt = 0;
	char		szDebugMsg[100 + 1];
	char		szAscii[4 + 1];
        unsigned char 	uszBCD[2 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inTAKA_Pack13() START!");

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
                inLogPrintf(AT, "inTAKA_Pack13(%ld) END!", inCnt);

        return (inCnt);
}

/*
Function        :inTAKA_Pack14
Date&Time       :2022/10/14 下午 7:37
Describe        :
*/
int inTAKA_Pack14(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int	inCnt = 0;

	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inTAKA_Pack14(Exp = %s) START!!", pobTran->srBRec.szExpDate);
	inCnt = 0;
	inFunc_ASCII_to_BCD(&uszPackBuf[inCnt], pobTran->srBRec.szExpDate, _EXP_DATE_SIZE_ / 2);
	inCnt += _EXP_DATE_SIZE_ / 2;
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inTAKA_Pack14(%ld) END!", inCnt);
	return (inCnt);
}

/*
Function        :inTAKA_Pack22
Date&Time       :2022/10/14 下午 7:40
Describe        :Field_22:   Point of Service Entry Mode
*/
int inTAKA_Pack22(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int             inCnt = 0;
	char		szDebugMsg[100 + 1];			
	char		szAscii[4 + 1];				/* 放Debug中轉出來的Ascii用 */
        char            szPOSmode[4 + 1];			/* Field 22的Ascii值 */
        unsigned char   uszBCD[2 + 1];				/* Field 22的Hex值 */
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inTAKA_Pack22() START!");
	
        memset(uszBCD, 0x00, sizeof(uszBCD));
        memset(szPOSmode, 0x00, sizeof(szPOSmode));

	if (pobTran->srBRec.uszManualBit == VS_TRUE)
		sprintf(szPOSmode, "0012");
	else
		sprintf(szPOSmode, "0022");

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
                inLogPrintf(AT, "inTAKA_Pack22(%ld) END!", inCnt);
	
        return (inCnt);
}

/*
Function        :inTAKA_Pack24
Date&Time       :2022/10/14 下午 7:44
Describe        :Field_24:      Network International Identifier (NII)
*/
int inTAKA_Pack24(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 		inCnt = 0;
        char 		szTemplate[100 + 1];
	char		szDebugMsg[100 + 1];
	char		szAscii[4 + 1];
        unsigned char 	uszBCD[2 + 1];
	
	 memset(szTemplate, 0x00, sizeof(szTemplate));
        if (inGetNII(&szTemplate[1]) == VS_ERROR)
                return (VS_ERROR);

        /* NII 只有三碼所以前面補0 */
        szTemplate[0] = '0' ;
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inTAKA_Pack24(%s) START!", szTemplate);
	
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
                inLogPrintf(AT, "inTAKA_Pack24(%ld) END!", inCnt);

        return (inCnt);
}

/*
Function        :inTAKA_Pack25
Date&Time       :2022/10/14 下午 7:46
Describe        :Field_25:   Point of Service Condition Code
*/
int inTAKA_Pack25(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 		inCnt = 0;
	char		szTemplate[100 + 1];
	char		szDebugMsg[100 + 1];
	char		szAscii[2 + 1];
        unsigned char 	uszBCD[1 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inTAKA_Pack25() START!");

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
                inLogPrintf(AT, "inTAKA_Pack25(%ld) END!", inCnt);

        return (inCnt);
}

/*
Function        :inTAKA_Pack35
Date&Time       :2022/10/14 下午 7:56
Describe        :Field_35:   TRACK 2 Data
*/
int inTAKA_Pack35(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int     inCnt = 0, inPacketCnt = 0, i;
        char    szT2Data[50 + 1];		/* 目前track2最長到32 */
	char	szFinalPAN[20 + 1];		/* 為了U CARD */
	char	szDebugMsg[100 + 1];
	short   shLen = 0;
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inTAKA_Pack35() START!");

	memset(szT2Data, 0x00, sizeof(szT2Data));
	memset(szFinalPAN, 0x00, sizeof(szFinalPAN));
	/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
	strcpy(szFinalPAN, pobTran->srBRec.szPAN);
	switch (pobTran->inISOTxnCode)
	{
		case _REVERSAL_:
			strcpy(szT2Data, szFinalPAN);
			inPacketCnt += strlen(szFinalPAN);

			strcat(szT2Data, "D");
			inPacketCnt ++;
			memcpy(&szT2Data[inPacketCnt], &pobTran->srBRec.szExpDate[0], 4);
			inPacketCnt += 4;
			memcpy(&szT2Data[inPacketCnt], &pobTran->srBRec.szServiceCode[0], 3);
			inPacketCnt += 3;
			break;
		default:
			if (pobTran->srBRec.uszManualBit == VS_TRUE)
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
			else
			{
				memcpy(szT2Data, pobTran->szTrack2, strlen(pobTran->szTrack2));

				for (i = 0; i < strlen(szT2Data); i++)
				{
					if (szT2Data[i] == '=')
						szT2Data[i] = 'D';
				}
			}
			break;
	} /* End switch () ... */

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
                inLogPrintf(AT, "inTAKA_Pack35(%ld) END!", inCnt);

        return (inCnt);
}

/*
Function        :inTAKA_Pack37
Date&Time       :2022/10/14 下午 7:57
Describe        :Field_37:	Retrieval Reference Number (RRN)
*/
int inTAKA_Pack37(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 	inCnt = 0;
	char	szDebugMsg[100 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inTAKA_Pack37() START!");
	
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
                inLogPrintf(AT, "inTAKA_Pack37(%ld) END!", inCnt);
	
        return (inCnt);
}

/*
Function        :inTAKA_Pack38
Date&Time       :2022/10/14 下午 7:57
Describe        :Field_38:	Authorization Identification Response（SmartPay交易無授權碼）
*/
int inTAKA_Pack38(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 	inCnt = 0;
        char    szAuthCode[_AUTH_CODE_SIZE_ + 1];
	char	szDebugMsg[100 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inTAKA_Pack38() START!");


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
		if (!memcmp(pobTran->srBRec.szAuthCode, "000000", 6))
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Auth code全零");
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
                inLogPrintf(AT, "inTAKA_Pack38(%ld) END!", inCnt);
	
        return (inCnt);
}

/*
Function        :inTAKA_Pack39
Date&Time       :2022/10/14 下午 7:59
Describe        :
*/
int inTAKA_Pack39(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int 	inCnt = 0;

	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inTAKA_Pack39() START!");
	
	memcpy((char *)&uszPackBuf[inCnt], &pobTran->srBRec.szRespCode[0], 2);
	inCnt += 2;
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inTAKA_Pack38(%ld) END!", inCnt);
	
	return (inCnt);
}

/*
Function        :inTAKA_Pack41
Date&Time       :2022/10/14 下午 8:02
Describe        :Field_41:	Card Acceptor Terminal Identification（TID）補充說明：端末機代號需可支援英、數字。
*/
int inTAKA_Pack41(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 	inCnt = 0;
        char 	szTemplate[8 + 1] = {0};
	char	szDebugMsg[100 + 1] = {0};
	
	memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetTerminalID(szTemplate);
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inTAKA_Pack41(TID = %s) START!!", szTemplate);
	

        memcpy((char *) &uszPackBuf[inCnt], szTemplate, strlen(szTemplate));
        inCnt += strlen(szTemplate);
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "F_41 [TID %s]", szTemplate);
		inLogPrintf(AT, szDebugMsg);
	}
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inTAKA_Pack41(%ld) END!", inCnt);

        return (inCnt);
}

/*
Function        :inTAKA_Pack42
Date&Time       :2022/10/14 下午 8:02
Describe        :Field_42:	Card Acceptor Identification Code(MID)
 		補充說明：
		1. 商店代號需左靠右補空白。
		2. 語音開卡交易須使用固定商店代號 0122900410 。
*/
int inTAKA_Pack42(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 	inCnt = 0;
        char	szTemplate[16 + 1] = {0};
	char 	szDebugMsg[100 + 1] = {0};
	
	memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetMerchantID(szTemplate);
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inTAKA_Pack42(MID = %s) START!!", szTemplate);
	
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
                inLogPrintf(AT, "inTAKA_Pack42(%ld) END!", inCnt);

        return (inCnt);
}

/*
Function        :inTAKA_Pack48
Date&Time       :2022/10/14 下午 8:04
Describe        :Field_48:	Additional Data - Private Use
*/
int inTAKA_Pack48(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int	i = 0;
        int	inCnt = 0;
	char	szDebugMsg[150 + 1] = {0};
	char	szTemplate[40 + 1] = {0};
	char	szAscii[120 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
	char	szPOS_ID[6 + 1] = {0};
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inTAKA_Pack48() START!");

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
                inLogPrintf(AT, "inTAKA_Pack48(%ld) END!", inCnt);

	return (inCnt);
}

/*
Function        :inTAKA_Pack54
Date&Time       :2022/10/14 下午 8:06
Describe        :Field_54:	Additional Amounts(此規格放Tip)
*/
int inTAKA_Pack54(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 	inCnt = 0;
	char	szAscii[16 + 1];
	char	szDebugMsg[100 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inTAKA_Pack54() START!");

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
                inLogPrintf(AT, "inTAKA_Pack54(%ld) END!", inCnt);
	
        return (inCnt);
}



/*
Function        :inTAKA_Pack60
Date&Time       :
Describe        :Field_60:	Reserved-Private Data
		Record of Count must be Unique in the batch.(Batch Number)
*/
int inTAKA_Pack60(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 	inCnt = 0;
        long 	lnBatchNum = 0;
        char 	szTemplate[110 + 1] = {0};
	char    szMsgTypeId[4 + 1] = {0};
	char    szScratch[23] = {0};
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inTAKA_Pack60() START!");

	/*
	 For all reconciliation messages, this field will contain the 6 digit batch number.
	 This number may not be zero and will be represented in six bytes.
	 */
	
	/* Modify by 志鴻,for 大來不平帳上傳用 Original MTI+Original STAN(F_11)+Original RRN(F_37) (START) */
	if( pobTran->inISOTxnCode == _BATCH_UPLOAD_ )
	{
		memset(szMsgTypeId, 0x00, sizeof(szMsgTypeId));
		memset(szScratch, 0x00, sizeof(szScratch));

		switch (pobTran->srBRec.inCode)
		{
			case _SALE_ :
			case _REFUND_ :
				memcpy(szMsgTypeId, "\x02\x00", 2);
				break;
			default:
				break;
		}

    		sprintf(szScratch, "%02x%02x%06ld%12s", szMsgTypeId[0],
            						 szMsgTypeId[1],
            						 pobTran->srBRec.lnSTANNum,
            						 pobTran->srBRec.szRefNo);
            	inFunc_PAD_ASCII(szScratch, szScratch, ' ', 22, _PADDING_RIGHT_);

		uszPackBuf[inCnt ++] = 0x00;
		uszPackBuf[inCnt ++] = 0x22;
		memcpy((char *)&uszPackBuf[inCnt], szScratch, 22);
		inCnt += 22;
		/* Modify by 志鴻,for 大來不平帳上傳用 Original MTI+Original STAN(F_11)+Original RRN(F_37) (END) */
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
                inLogPrintf(AT, "inTAKA_Pack60(%ld) END!", inCnt);

        return (inCnt);
}

/*
Function        :inTAKA_Pack62
Date&Time       :2022/10/14 下午 8:16
Describe        :Field_62:	Reserved-Private Data(Invoice Number)
*/
int inTAKA_Pack62(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 	inCnt = 0;
	char	szTemplate[100 + 1];

	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inTAKA_Pack62() START!");
	
	
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
                inLogPrintf(AT, "inTAKA_Pack62(%ld) END!", inCnt);

        return (inCnt);
}

/*
Function        :inTAKA_Pack63
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
int inTAKA_Pack63(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int		inCnt = 0;
        int		inAccumCnt, inBatchCnt, inPacketCnt = 0;
	char		szASCII[4 + 1];
        char		szTemplate[100 + 1], szPacket[92 + 1];
	unsigned char	uszBCD[2 + 1];
        TAKA_ACCUM_TOTAL_REC srAccumRec;

	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inTAKA_Pack63() START!");
	
	
	memset(szPacket, 0x00, sizeof(szPacket));
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
		inAccumCnt = inACCUM_GetRecord_General(pobTran, &srAccumRec, sizeof(TAKA_ACCUM_TOTAL_REC));
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
			sprintf(szTemplate, "%010lld00", srAccumRec.llTotalSaleAmount);
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
                inLogPrintf(AT, "inTAKA_Pack63(%ld) END!", inCnt);

        return (inCnt);
}

/*
Function        :inTAKA_Check03
Date&Time       :
Describe        :送和收的processing Code要一致
*/
int inTAKA_Check03(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, unsigned char *uszRecePacket)
{
        if (memcmp((char *)uszSendPacket, (char *)uszRecePacket, 3))
        {
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

/*
Function        :inTAKA_Check04
Date&Time       :
Describe        :送和收的Amount要一致
*/
int inTAKA_Check04(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, unsigned char *uszRecePacket)
{
        if (memcmp((char *)uszSendPacket, (char *)uszRecePacket, 6))
        {
                //vdSGErrorMessage(NCCC_CHECK_ISO_FILED04_ERROR); /* 140 = 電文錯誤請重試 */
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

/*
Function        :inTAKA_Check41
Date&Time       :
Describe        :送和收的TID要一致
*/
int inTAKA_Check41(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, unsigned char *uszRecePacket)
{
        if (memcmp((char *)uszSendPacket, (char *)uszRecePacket, 8))
        {
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

/*
Function        :inTAKA_UnPack12
Date&Time       :
Describe        :
*/
int inTAKA_UnPack12(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
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
Function        :inTAKA_UnPack13
Date&Time       :
Describe        :
*/
int inTAKA_UnPack13(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
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
Function        :inTAKA_UnPack37
Date&Time       :
Describe        :
*/
int inTAKA_UnPack37(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
        if (pobTran->inISOTxnCode == _REVERSAL_)
                return (VS_SUCCESS);

        memset(pobTran->srBRec.szRefNo, 0x00, sizeof(pobTran->srBRec.szRefNo));
        memcpy(&pobTran->srBRec.szRefNo[0], (char *) &uszUnPackBuf[0], 12);

        return (VS_SUCCESS);
}

/*
Function        :inTAKA_UnPack38
Date&Time       :
Describe        :
*/
int inTAKA_UnPack38(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
	char	szFESMode[2 + 1] = {0};
	
        if (pobTran->inISOTxnCode == _REVERSAL_ || pobTran->inISOTxnCode == _CUP_LOGON_)
	{
                return (VS_SUCCESS);
	}
	else
	{
		memset(pobTran->srBRec.szAuthCode, 0x00, sizeof(pobTran->srBRec.szAuthCode));
		memcpy(&pobTran->srBRec.szAuthCode[0], (char *) &uszUnPackBuf[0], _AUTH_CODE_SIZE_);
	}
	
	memset(szFESMode, 0x00, sizeof(szFESMode));
	inGetNCCCFESMode(szFESMode);
	if (memcmp(szFESMode, _NCCC_04_MPAS_MODE_, strlen(_NCCC_04_MPAS_MODE_)) == 0)
	{
		memset(pobTran->srBRec.szMPASAuthCode, 0x00, sizeof(pobTran->srBRec.szMPASAuthCode));
		memcpy(&pobTran->srBRec.szMPASAuthCode[0], (char *) &uszUnPackBuf[0], _AUTH_CODE_SIZE_);
	}

        return (VS_SUCCESS);
}

/*
Function        :inTAKA_UnPack39
Date&Time       :
Describe        :Response code
*/
int inTAKA_UnPack39(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{	
	char	szTemplate[6 + 1];

	memset(&szTemplate, 0x00, sizeof(szTemplate));
	inFunc_BCD_to_ASCII(&szTemplate[0], &guszTAKA_ISO_Field03[0], 3);

	memset(pobTran->srBRec.szRespCode, 0x00, sizeof(pobTran->srBRec.szRespCode));
	memcpy(&pobTran->srBRec.szRespCode[0], (char *)&uszUnPackBuf[0], 2);

        return (VS_SUCCESS);
}

/*
Function        :inTAKA_UnPack60
Date&Time       :
Describe        :
*/
int inTAKA_UnPack60(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
	int	inRetVal = VS_SUCCESS;

	return (inRetVal);
}

/*
Function        :inTAKA_ISOPackMessageType
Date&Time       :2022/10/14 下午 5:59
Describe        :把MTI放到PackData中，並放到guszTAKA_MTI中以便在pack其他欄位時判斷
*/
int inTAKA_ISOPackMessageType(TRANSACTION_OBJECT *pobTran, int inISOTxnCode, unsigned char *uszPackData, char *szMTI)
{
        int 		inCnt = 0;
	char		szFinalMTI[4 + 1] = {0};
        unsigned char 	uszBCD[10 + 1];
	
	memset(szFinalMTI, 0x00, sizeof(szFinalMTI));
	strcpy(szFinalMTI, szMTI);

	/* 放到global中方便判斷 */
	memcpy((char*)guszTAKA_MTI, szFinalMTI, 4);
	
	/* 放到把MTI放到PackData中 */
        memset(uszBCD, 0x00, sizeof(uszBCD));
        inFunc_ASCII_to_BCD(&uszBCD[0], &szFinalMTI[0], _TAKA_MTI_SIZE_);
        memcpy((char *) &uszPackData[inCnt], (char *) &uszBCD[0], _TAKA_MTI_SIZE_);
        inCnt += _TAKA_MTI_SIZE_;

        return (inCnt);
}

/*
Function        :inTAKA_ISOModifyBitMap
Date&Time       :2022/10/14 下午 6:01
Describe        :
*/
int inTAKA_ISOModifyBitMap(TRANSACTION_OBJECT *pobTran, int inISOTxnCode, int *inBitMap)
{
	if (pobTran->srBRec.uszManualBit == VS_TRUE)
	{
		inTAKA_BitMapSet(inBitMap, 2);
		inTAKA_BitMapSet(inBitMap, 14);
		inTAKA_BitMapReset(inBitMap, 35);
	}

        return (VS_SUCCESS);
}

int inTAKA_ISOModifyPackData(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackData, int *inPackLen)
{	
        return (VS_SUCCESS);
}

/*
Function        :inTAKA_ISOCheckHeader
Date&Time       :2016/9/14 上午 10:17
Describe        :
*/
int inTAKA_ISOCheckHeader(TRANSACTION_OBJECT *pobTran, char *szSendISOHeader, char *szReceISOHeader)
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
Function        :inTAKA_ISOOnlineAnalyse
Date&Time       :2022/10/14 下午 6:05
Describe        :
*/
int inTAKA_ISOOnlineAnalyse(TRANSACTION_OBJECT *pobTran)
{
        int	inRetVal = VS_ERROR;
	char	szFESMode[2 + 1] = {0};
	inRetVal = inTAKA_OnlineAnalyseMagneticManual(pobTran);
	
	/* 重印簽單的判斷 可能有斷電的例外狀況 收完電文預設可重印 單機操作可重印 */
	memset(szFESMode, 0x00, sizeof(szFESMode));
	inGetNCCCFESMode(szFESMode);
	if (memcmp(szFESMode, _NCCC_04_MPAS_MODE_, strlen(_NCCC_04_MPAS_MODE_)) == 0)
		pobTran->srBRec.uszMPASReprintBit = VS_TRUE;
	
	if (inRetVal != VS_SUCCESS)
	{
		inUtility_StoreTraceLog_OneStep("Analyse Failed");
		inUtility_StoreTraceLog_OneStep("Response Code:%s", pobTran->srBRec.szRespCode);
		inUtility_StoreTraceLog_OneStep("Auth Code:%s", pobTran->srBRec.szAuthCode);
	}

        return (inRetVal);
}

/*
Function        :inTAKA_ISOAdviceAnalyse
Date&Time       :2016/9/14 上午 10:17
Describe        :
*/
int inTAKA_ISOAdviceAnalyse(TRANSACTION_OBJECT *pobTran, unsigned char *uszTcUpload)
{
        if (!memcmp(&pobTran->srBRec.szRespCode[0], "00", 2))
        {
                if (pobTran->srBRec.inCode == _VOID_)
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
		/* 只有結帳時前送advice錯誤時要顯示錯誤訊息，在inTAKA_ISOAdviceAnalyse裡顯示錯誤訊息 */
		/* 結帳交易流程中，若於前帳前補送電文，補送電文有拒絕或其他回覆碼的狀況，畫面皆顯示結帳失敗即可，不須顯示補送電文之回覆碼訊息。 */
        }

        return (VS_ERROR);
}

int inTAKA_BitMapSet(int *inBitMap, int inFeild)
{
        int 	i, inBMapCnt, inBitMapCnt;
        int 	inBMap[_TAKA_MAX_BIT_MAP_CNT_];

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

int inTAKA_BitMapReset(int *inBitMap, int inFeild)
{
        int 	i, inBMapCnt;
        int 	inBMap[_TAKA_MAX_BIT_MAP_CNT_];

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

int inTAKA_BitMapCheck(unsigned char *inBitMap, int inFeild)
{
        int 	inByteIndex, inBitIndex;

        inFeild--;
        inByteIndex = inFeild / 8;
        inBitIndex = 7 - (inFeild - inByteIndex * 8);

        if (_TAKA_CHECK_BIT_MAP_(inBitMap[inByteIndex], inBitIndex))
        {
                return (VS_TRUE);
        }

        return (VS_FALSE);
}

/*
Function        :inTAKA_CopyBitMap
Date&Time       :
Describe        :
*/
int inTAKA_CopyBitMap(int *inBitMap, int *inSearchBitMap)
{
        int 	i;

        for (i = 0; i < _TAKA_MAX_BIT_MAP_CNT_; i++)
        {
                if (inSearchBitMap[i] == 0)
                        break;
                else
                        inBitMap[i] = inSearchBitMap[i];
        }

        return (VS_SUCCESS);
}

/*
Function        :inTAKA_GetBitMapTableIndex
Date&Time       :
Describe        :
*/
int inTAKA_GetBitMapTableIndex(ISO_TYPE_TAKA_TABLE *srISOFunc, int inBitMapTxnCode)
{
        int 	inBitMapIndex;

        for (inBitMapIndex = 0;; inBitMapIndex++)
        {
                if (srISOFunc->srBitMap[inBitMapIndex].inTxnID == _TAKA_NULL_TX_)
                {
                        return (VS_ERROR); /* 找不到相對應交易別的Bit Map */
                }

                if (srISOFunc->srBitMap[inBitMapIndex].inTxnID == inBitMapTxnCode)
                        break; /* 找到一樣的交易類別 */
        }

        return (inBitMapIndex);
}

/*
Function        :inTAKA_GetBitMapMessagegTypeField03
Date&Time       :2016/9/14 下午 1:34
Describe        :
*/
int inTAKA_GetBitMapMessagegTypeField03(TRANSACTION_OBJECT *pobTran, ISO_TYPE_TAKA_TABLE *srISOFunc, int inIsoType, int *inTxnBitMap, unsigned char *uszSendBuf)
{
        int 		i, j, k, inBitMapIndex, *inBitMap, inCnt, inBitMapTxnCode = -1;
        char		szTemplate[64 + 1];
	unsigned char 	uszBuf;
       

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inTAKA_GetBitMapMessagegTypeField03() START!");
        
        if (ginDebug == VS_TRUE)
        {
                sprintf(szTemplate, "inIsoType = %d", inIsoType);
                inLogPrintf(AT, szTemplate);
        }      
        
        /* 設定交易別 */
        inBitMapTxnCode = inIsoType;
      
        /* 要搜尋 BIT_MAP_TAKA_TABLE srTAKA_ISOBitMap 相對應的 inTxnID */
        if ((inBitMapIndex = inTAKA_GetBitMapTableIndex(srISOFunc, inBitMapTxnCode)) == VS_ERROR)
        {
		inUtility_StoreTraceLog_OneStep("inTAKA_GetBitMapTableIndex Error");
		
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inTAKA_GetBitMapTableIndex == VS_ERROR");
                
                return (VS_ERROR);
        }

        /* Pack Message Type */
        inCnt = 0;
        inCnt += srISOFunc->inPackMTI(pobTran, inIsoType, &uszSendBuf[inCnt], srISOFunc->srBitMap[inBitMapIndex].szMTI);

        /* 要搜尋 BIT_MAP_TAKA_TABLE srTAKA_ISOBitMap 相對應的 inBitMap */
        inTAKA_CopyBitMap(inTxnBitMap, srISOFunc->srBitMap[inBitMapIndex].inBitMap);

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

        inCnt += _TAKA_BIT_MAP_SIZE_;

        /* Process Code */	
        memset(guszTAKA_ISO_Field03, 0x00, sizeof(guszTAKA_ISO_Field03));
        inFunc_ASCII_to_BCD(&guszTAKA_ISO_Field03[0], srISOFunc->srBitMap[inBitMapIndex].szPCode, 3);

        if (inBitMapTxnCode == _REVERSAL_)
        {
                if (pobTran->inTransactionCode == _VOID_)
                {
                        /*
                         Processing Code		Activities
                         --------------------------------------------------------
                         020000	|	Reversal Void Sale
			 120000	|	Reversal Void Cash Advance
                         220000	|	Reversal Void Refund		 
                         320000	|	Reversal Void Pre-Auth
			 420000	|	Reversal Void Pre-Auth Comp
			 720000	|	Reversal Void Loyalty Redeem
			 254200	|	Reversal Void Fisc Sale
                         */
                        switch (pobTran->srBRec.inOrgCode)
                        {
                                case _SALE_:
                                        guszTAKA_ISO_Field03[0] = 0x02;
                                        guszTAKA_ISO_Field03[1] = 0x00;
                                        break;
                                case _REFUND_:
                                        guszTAKA_ISO_Field03[0] = 0x22;
                                        guszTAKA_ISO_Field03[1] = 0x00;
                                        break;
                                default:
                                        guszTAKA_ISO_Field03[0] = 0x00;
                                        break;
                        }
                }
                else
                {
                        /*
                         Processing Code		Activities
                         --------------------------------------------------------
                         000000	|	Reversal Sale
			 100000	|	Reversal Cash Advance
                         200000	|	Reversal Refund		 
                         300000	|	Reversal Pre-Auth
			 400000	|	Reversal Pre-Auth Comp
			 700000	|	Reversal Loyalty Redeem
			 254100	|	Reversal Fisc Sale
			 254300	|	Reversal Fisc Refund
                         */
                        switch (pobTran->inTransactionCode)
                        {
                                case _SALE_:
                                        guszTAKA_ISO_Field03[0] = 0x00;
                                        guszTAKA_ISO_Field03[1] = 0x00;
                                        break;
                                case _REFUND_:
                                        guszTAKA_ISO_Field03[0] = 0x20;
                                        guszTAKA_ISO_Field03[1] = 0x00;
                                        break;
                                default:
                                        guszTAKA_ISO_Field03[0] = 0x00;
                                        break;
                        }
                }
        }
        else if (inBitMapTxnCode == _BATCH_UPLOAD_)
        {
                /*
		 * 優惠兌換不會 Batch Upload
                 Processing Code		Activities
                 --------------------------------------------------------
                 000000	|	Sale/Mail Order
		 100000	|	Cash Advance/FORCE_CASH_ADVANCE
		 200000	|	Refund(Refund Mail Order)		 
		 300000	|	Pre-Auth
		 400000	|	Pre-Auth Comp
		 254100	|	Fisc Sale
		 254200	|	Fisc Void
		 254300	|	Fisc Refund
                 */
                switch (pobTran->srBRec.inOrgCode)
                {
                        case _SALE_:
				guszTAKA_ISO_Field03[0] = 0x00;
				guszTAKA_ISO_Field03[1] = 0x00;
				break;
			case _REFUND_:
				guszTAKA_ISO_Field03[0] = 0x20;
				guszTAKA_ISO_Field03[1] = 0x00;
				break;
                        default:
                                break;
                }

                if (pobTran->uszLastBatchUploadBit == VS_TRUE)
                        guszTAKA_ISO_Field03[2] = 0x00;
                else
                        guszTAKA_ISO_Field03[2] = 0x01;

        }
        else if (inBitMapTxnCode == _VOID_)
        {
                /*
                 Processing Code		Activities
                 --------------------------------------------------------
		 020000	|	Void Sale
		 120000	|	Void Cash Advance
		 220000	|	Void Refund		 
		 320000	|	Void Pre-Auth(Only for UnionPay)
		 420000	|	Void Pre-Auth Comp(Only for UnionPay)
		 720000	|	Void Loyalty Redeem
                 */
                switch (pobTran->srBRec.inOrgCode)
                {
                        case _SALE_:
                                guszTAKA_ISO_Field03[0] = 0x02;
				guszTAKA_ISO_Field03[1] = 0x00;
				break;
			case _REFUND_:
				guszTAKA_ISO_Field03[0] = 0x22;
				guszTAKA_ISO_Field03[1] = 0x00;
				break;
			default:
                                break;
                }
        }

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inTAKA_GetBitMapMessagegTypeField03() END!");
        
        return (inCnt);
}

int inTAKA_PackISO(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendBuf, int inTxnCode)
{
        int				i, inSendCnt, inField, inCnt;
        int				inBitMap[_TAKA_MAX_BIT_MAP_CNT_ + 1];
        int				inRetVal, inISOFuncIndex = -1;
	char				szTemplate[42 + 1];
        char				szLogMessage[40 + 1];
        unsigned char			uszBCD[20 + 1];
        ISO_TYPE_TAKA_TABLE 	srISOFunc;		
	
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inTAKA_PackISO() START!");
	
        inSendCnt = 0;
        inField = 0;

	inISOFuncIndex = 0; /* 不加密 */
	
        /* 決定要執行第幾個 Function Index */
        memset((char *) &srISOFunc, 0x00, sizeof(srISOFunc));
        memcpy((char *) &srISOFunc, (char *) &srTAKA_ISOFunc[inISOFuncIndex], sizeof(srISOFunc));
        memset((char *) inBitMap, 0x00, sizeof(inBitMap));

        /* 開始組 ISO 電文 */
        /* Pack TPDU */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        if (inGetTPDU(szTemplate) != VS_SUCCESS)
        {
		inUtility_StoreTraceLog_OneStep("inGetTPDU Error");
		
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inGetTPDU() ERROR!");

                return (VS_ERROR);
        }

        memset(uszBCD, 0x00, sizeof(uszBCD));
        inFunc_ASCII_to_BCD(uszBCD, szTemplate, 5);
        memcpy((char *) &uszSendBuf[inSendCnt], (char *) uszBCD, _TAKA_TPDU_SIZE_);
        inSendCnt += _TAKA_TPDU_SIZE_;
        /* Get Bit Map / Mesage Type / Processing Code */
        inRetVal = inTAKA_GetBitMapMessagegTypeField03(pobTran, &srISOFunc, inTxnCode, inBitMap, &uszSendBuf[inSendCnt]);
        if (inRetVal == VS_ERROR)
        {
		inUtility_StoreTraceLog_OneStep("inTAKA_GetBitMapMessagegTypeField03 Error");
		
                if (ginDebug == VS_TRUE)
		{
                        inLogPrintf(AT, "inTAKA_GetBitMapMessagegTypeField03() ERROR!");
		}
		
		if (ginDisplayDebug == VS_TRUE)
		{
			memset(szLogMessage, 0x00, sizeof(szLogMessage));
			sprintf(szLogMessage, "PCODE錯誤");
			inDISP_LOGDisplay(szLogMessage, _FONTSIZE_16X22_, _LINE_16_2_, VS_FALSE);
		}

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
				inUtility_StoreTraceLog_OneStep("inField = %d Error", srISOFunc.srPackISO[inField].inFieldNum);
				
                                if (ginDebug == VS_TRUE)
                                {
                                        memset(szLogMessage, 0x00, sizeof(szLogMessage));
                                        sprintf(szLogMessage, "inField = %d Error", srISOFunc.srPackISO[inField].inFieldNum);
                                        inLogPrintf(AT, szLogMessage);
                                }
				if (ginDisplayDebug == VS_TRUE)
				{
					memset(szLogMessage, 0x00, sizeof(szLogMessage));
					sprintf(szLogMessage, "inField = %d Error", srISOFunc.srPackISO[inField].inFieldNum);
					inDISP_LOGDisplay(szLogMessage, _FONTSIZE_16X22_, _LINE_16_2_, VS_FALSE);
				}

                                return (VS_ERROR);
                        }
                        else
                                inSendCnt += inCnt;
                }
		
        }

        if (srISOFunc.inModifyPackData != _TAKA_NULL_TX_)
                srISOFunc.inModifyPackData(pobTran, uszSendBuf, &inSendCnt);
		
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inTAKA_PackISO() END!");

        return (inSendCnt);
}

int inTAKA_CheckUnPackField(int inField, ISO_FIELD_TAKA_TABLE *srCheckUnPackField)
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

int inTAKA_GetCheckField(int inField, ISO_CHECK_TAKA_TABLE *ISOFieldCheck)
{
        int 	i;

        for (i = 0; i < 64; i++)
        {
                if (ISOFieldCheck[i].inFieldNum == 0)
                        break;
                else if (ISOFieldCheck[i].inFieldNum == inField)
                {
                        return (i); /* i 是 ISO_CHECK_TAKA_TABLE 裡的第幾個 */
                }
        }

        return (VS_ERROR);
}

int inTAKA_GetFieldLen(int inField, unsigned char *uszSendData, ISO_FIELD_TYPE_TAKA_TABLE *srFieldType)
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
                        case _TAKA_ISO_ASC_:
                                inCnt += srFieldType[i].inFieldLen;
                                break;
                        case _TAKA_ISO_BCD_:
                                inCnt += srFieldType[i].inFieldLen / 2;
                                break;
                        case _TAKA_ISO_NIBBLE_2_:
                                inLen = (uszSendData[0] / 16 * 10) + (uszSendData[0] % 16);
				/* Smart Pay卡號上傳格式(Field_35)不同一般信用卡。一般信用卡的卡號需要Pack成 BCD Code，Smart pay的卡號/帳號直接上傳ASCII Code不進行Pack。 */
				/* 因為reversal不讀batch，沒辦法知道fiscBit */
				/* smartpay用00000956判斷 */
				/* Uny改用"Un"判斷 */
				if (inLen == 54 && srFieldType[i].inFieldNum == 35)
				{
					if (uszSendData[1] >= '0' && uszSendData[1] <= '9' &&
					    uszSendData[2] >= '0' && uszSendData[2] <= '9' &&
					    uszSendData[3] >= '0' && uszSendData[3] <= '9' &&
					    uszSendData[4] >= '0' && uszSendData[4] <= '9' &&
					    uszSendData[5] >= '0' && uszSendData[5] <= '9' &&
					    uszSendData[6] >= '0' && uszSendData[6] <= '9' &&
					    uszSendData[7] >= '0' && uszSendData[7] <= '9' &&
					    uszSendData[8] >= '0' && uszSendData[8] <= '9')
					{
					    inCnt += inLen + 1;
					}
					else
					{
					     inCnt += ((inLen + 1) / 2) + 1;   
					}
				}
				else if (inLen == 18 && srFieldType[i].inFieldNum == 35)
				{
				    if (!memcmp(&uszSendData[1], "UN", 2))
				    {
					inCnt += inLen + 1;
				    }
				    else if (!memcmp(&uszSendData[1], "Un", 2))
				    {
					inCnt += inLen + 1;
				    }
				    else
				    {
					 inCnt += ((inLen + 1) / 2) + 1;   
				    }
				}
				else
				{
					inCnt += ((inLen + 1) / 2) + 1;
				}
                                break;
                        case _TAKA_ISO_NIBBLE_3_:
                                inLen = (uszSendData[0] / 16 * 1000) + (uszSendData[0] % 16 * 100) + (uszSendData[1] / 16 * 10) + (uszSendData[1] % 16);
                                inCnt += ((inLen + 1) / 2) + 2;
                                break;
                        case _TAKA_ISO_BYTE_2_:
                                inLen = (uszSendData[0] / 16 * 10) + (uszSendData[0] % 16);
                                inCnt += inLen + 1;
                                break;
                        case _TAKA_ISO_BYTE_3_:
                                inLen = (uszSendData[0] / 16 * 1000) + (uszSendData[0] % 16 * 100) + (uszSendData[1] / 16 * 10) + (uszSendData[1] % 16);
                                inCnt += inLen + 2;
                                break;
                        case _TAKA_ISO_BYTE_2_H_:
                                inLen = (int) uszSendData[0];
                                inCnt += inLen + 1;
                                break;
                        case _TAKA_ISO_BYTE_3_H_:
                                inLen = ((int) uszSendData[0] * 0xFF) + (int) uszSendData[1];
                                inCnt += inLen + 1;
                                break;
			case _TAKA_ISO_BYTE_1_:
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

int inTAKA_GetFieldIndex(int inField, ISO_FIELD_TYPE_TAKA_TABLE *srFieldType)
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

int inTAKA_UnPackISO(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendBuf, unsigned char *uszRecvBuf)
{
	int				inRetVal;
        int				i, inRecvCnt, inCnt, inSendCnt, inSendField, inRecvField, inISOFuncIndex = -1;
        char				szBuf[_TAKA_TPDU_SIZE_ + _TAKA_MTI_SIZE_ + _TAKA_BIT_MAP_SIZE_ + 1];
        char				szErrorMessage[40 + 1];
        unsigned char			uszSendMap[_TAKA_BIT_MAP_SIZE_ + 1], uszReceMap[_TAKA_BIT_MAP_SIZE_ + 1];
        ISO_TYPE_TAKA_TABLE 	srISOFunc;


        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inTAKA_UnPackISO() START!");

        inSendField = inRecvField = 0;
        inSendCnt = inRecvCnt = 0;

        memset((char *) uszSendMap, 0x00, sizeof(uszSendMap));
        memset((char *) uszReceMap, 0x00, sizeof(uszReceMap));
        memset((char *) szBuf, 0x00, sizeof(szBuf));
	
	inISOFuncIndex = 0; /* 不加密 */
	
        memset((char *) &srISOFunc, 0x00, sizeof(srISOFunc));
        memcpy((char *) &srISOFunc, (char *) &srTAKA_ISOFunc[inISOFuncIndex], sizeof(srISOFunc));

        if (srISOFunc.inCheckISOHeader != NULL)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "srISOFunc.inCheckISOHeader != NULL");

                if (srISOFunc.inCheckISOHeader(pobTran, (char *) &uszSendBuf[inRecvCnt], (char *) &uszRecvBuf[inRecvCnt]) != VS_SUCCESS)
                {
			inUtility_StoreTraceLog_OneStep("srISOFunc.inCheckISOHeader ERROR", inISOFuncIndex);
                        if (ginDebug == VS_TRUE)
                                inLogPrintf(AT, "srISOFunc.inCheckISOHeader ERROR");

                        return (_TRAN_RESULT_UNPACK_ERR_);
                }
        }

        inSendCnt += _TAKA_TPDU_SIZE_;
        inRecvCnt += _TAKA_TPDU_SIZE_;
        inSendCnt += _TAKA_MTI_SIZE_;
        inRecvCnt += _TAKA_MTI_SIZE_;

        memcpy((char *) uszSendMap, (char *) &uszSendBuf[inSendCnt], _TAKA_BIT_MAP_SIZE_);
        memcpy((char *) uszReceMap, (char *) &uszRecvBuf[inRecvCnt], _TAKA_BIT_MAP_SIZE_);

        inSendCnt += _TAKA_BIT_MAP_SIZE_;
        inRecvCnt += _TAKA_BIT_MAP_SIZE_;

        /* 先檢查 ISO Field_39 */
        if (inTAKA_BitMapCheck(uszReceMap, 39) == VS_FALSE)
        {
		inUtility_StoreTraceLog_OneStep("inTAKA_BitMapCheck(39) ERROR");
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inTAKA_BitMapCheck(39) ERROR");

                return (VS_ERROR);
        }

        if (inTAKA_BitMapCheck(uszReceMap, 41) == VS_FALSE)
        {
		inUtility_StoreTraceLog_OneStep("inTAKA_BitMapCheck(41) ERROR");
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inTAKA_BitMapCheck(41) ERROR");

                return (VS_ERROR);
        }

        for (i = 1; i <= 64; i++)
        {
                /* 有送出去的 Field 但沒有收回來的 Field */
                if (inTAKA_BitMapCheck(uszSendMap, i) && !inTAKA_BitMapCheck(uszReceMap, i))
                {
                        inSendCnt += inTAKA_GetFieldLen(i, &uszSendBuf[inSendCnt], srISOFunc.srISOFieldType);
                }
                else if (inTAKA_BitMapCheck(uszReceMap, i)) /* 收的 BitMap */
                {
                        if (inTAKA_BitMapCheck(uszSendMap, i)) /* 送的 BitMap 都有 */
                        {
                                /* 是否要進行檢查封包資料包含【送】【收】 */
                                if ((inSendField = inTAKA_GetCheckField(i, srISOFunc.srCheckISO)) != VS_ERROR)
                                {
                                        if (srISOFunc.srCheckISO[inSendField].inISOCheck(pobTran, &uszSendBuf[inSendCnt], &uszRecvBuf[inRecvCnt]) != VS_SUCCESS)
                                        {
						inUtility_StoreTraceLog_OneStep("inSendField = %d Error!", srISOFunc.srCheckISO[inSendField].inFieldNum);
                                                if (ginDebug == VS_TRUE)
                                                {
                                                        memset(szErrorMessage, 0x00, sizeof(szErrorMessage));
                                                        sprintf(szErrorMessage, "inSendField = %d Error!", srISOFunc.srCheckISO[inSendField].inFieldNum);
                                                        inLogPrintf(AT, szErrorMessage);
                                                }

                                                return (_TRAN_RESULT_UNPACK_ERR_);
                                        }
                                }

                                inSendCnt += inTAKA_GetFieldLen(i, &uszSendBuf[inSendCnt], srISOFunc.srISOFieldType);
                        }

                        while (i > srISOFunc.srUnPackISO[inRecvField].inFieldNum) /* 只有收 */
                        {
                                inRecvField++;
                        }

                        if (i == srISOFunc.srUnPackISO[inRecvField].inFieldNum)
                        {
                                /* 要先檢查 UnpackISO 是否存在 */
                                if (inTAKA_CheckUnPackField(i, srISOFunc.srUnPackISO) == VS_SUCCESS)
				{
                                        inRetVal =srISOFunc.srUnPackISO[inRecvField].inISOLoad(pobTran, &uszRecvBuf[inRecvCnt]);
					
					if (inRetVal != VS_SUCCESS)
					{
						inUtility_StoreTraceLog_OneStep("inRecvField = %d Error!", srISOFunc.srUnPackISO[inRecvField].inFieldNum);
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
			
                        inCnt = inTAKA_GetFieldLen(i, &uszRecvBuf[inRecvCnt], srISOFunc.srISOFieldType);
                        if (inCnt == VS_ERROR)
                        {
				inUtility_StoreTraceLog_OneStep("inRecvField = %d Error!", srISOFunc.srUnPackISO[inRecvField].inFieldNum);
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
                        case _SALE_:
                        case _REFUND_:
                        case _VOID_:
                                if (inTAKA_BitMapCheck(uszReceMap, 38) == VS_FALSE)
                                {
					inUtility_StoreTraceLog_OneStep("inTAKA_BitMapCheck 38 Error!");
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "inTAKA_BitMapCheck 38 Error!");
                                        }

                                        return (VS_ERROR);
                                }
                                break;
                        default:
                                break;
                }
        }

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inTAKA_UnPackISO() END!");

        return (VS_SUCCESS);
}

int inTAKA_GetSTAN(TRANSACTION_OBJECT *pobTran)
{
        char 	szSTANNum[12 + 1];

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inTAKA_GetSTAN() START!");

        memset(szSTANNum, 0x00, sizeof(szSTANNum));
        if (inGetSTANNum(szSTANNum) == VS_ERROR)
                return (VS_ERROR);

        pobTran->srBRec.lnSTANNum = atol(szSTANNum);

        return (VS_SUCCESS);
}

/*
Function        :inTAKA_SetSTAN
Date&Time       :2022/10/14 下午 6:31
Describe        :STAN++
*/
int inTAKA_SetSTAN(TRANSACTION_OBJECT *pobTran)
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
Function        :inTAKA_ProcessReversal
Date&Time       :2022/10/14 下午 6:31
Describe        :
*/
int inTAKA_ProcessReversal(TRANSACTION_OBJECT *pobTran)
{
        int 	inRetVal;
        char 	szSendReversalBit[2 + 1];
	
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inTAKA_ProcessReversal() START!");
	
        memset(szSendReversalBit, 0x00, sizeof(szSendReversalBit));
        if (inGetSendReversalBit(szSendReversalBit) == VS_ERROR)
	{
		inUtility_StoreTraceLog_OneStep("inGetSendReversalBit Error");
		
		pobTran->inErrorMsg = _ERROR_CODE_V3_GET_HDPT_TAG_FAIL_;
                return (VS_ERROR);
	}

        if (!memcmp(szSendReversalBit, "Y", 1))
        {
                if ((inRetVal = inTAKA_ReversalSendRecvPacket(pobTran)) != VS_SUCCESS)
                {
			inUtility_StoreTraceLog_OneStep("inTAKA_ReversalSendRecvPacket Error");
			
                        return (VS_ERROR);
                }

                if (inTAKA_SetReversalCnt(pobTran, _ADD_) == VS_ERROR)
                {
			pobTran->inErrorMsg = _ERROR_CODE_V3_SET_HDPT_TAG_FAIL_;
                        return (VS_ERROR);
                }
        }
	
        /* 檢查是否要產生 Reversal */
        if (pobTran->uszReversalBit == VS_TRUE)
        {
		inRetVal = inTAKA_ReversalSave_Flow(pobTran);
                if (inRetVal != VS_SUCCESS)
                {
			inUtility_StoreTraceLog_OneStep("inTAKA_ReversalSave_Flow Error");
                        return (VS_ERROR);
                }
        }
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inTAKA_ProcessReversal() END!");
	
        return (VS_SUCCESS);
}

/*
Function        :inTAKA_AdviceSendRecvPacket
Date&Time       :2022/10/14 下午 6:31
Describe        :
*/
int inTAKA_AdviceSendRecvPacket(TRANSACTION_OBJECT *pobTran, int inAdvCnt)
{
        int			inCnt, inRetVal = VS_ERROR, inISOFuncIndex = -1;
        unsigned char		uszTCUpload = 0;
        TRANSACTION_OBJECT	ADVpobTran;
        ISO_TYPE_TAKA_TABLE 	srISOFunc;

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inTAKA_AdviceSendRecvPacket() START!");
        
        /* 加密模式，預設值 = 0。【0 = 不加密，1 = tSAM 加密，2 = 軟體加密】 */
        inISOFuncIndex = 0; /* 不加密 */
        memset((char *) &srISOFunc, 0x00, sizeof(srISOFunc));
        memcpy((char *) &srISOFunc, (char *) &srTAKA_ISOFunc[inISOFuncIndex], sizeof(srISOFunc));

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
		
		ADVpobTran.inISOTxnCode = _ADVICE_;

		if (inRetVal == VS_SUCCESS)
			inRetVal = inTAKA_SendPackRecvUnPack(&ADVpobTran);

		if (inRetVal == VS_SUCCESS)
			if (srISOFunc.inAdviceAnalyse != NULL)
				inRetVal = srISOFunc.inAdviceAnalyse(&ADVpobTran, &uszTCUpload);

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
                inLogPrintf(AT, "inTAKA_AdviceSendRecvPacket() END!");
        
        return (inRetVal);
}

/*
Function        :inTAKA_ProcessAdvice
Date&Time       :2022/10/14 下午 6:31
Describe        :
*/
int inTAKA_ProcessAdvice(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;
        int 	inSendAdviceCnt = 0;

        if ((inSendAdviceCnt = inADVICE_GetTotalCount(pobTran)) == VS_NO_RECORD)
        {
                return (VS_SUCCESS);
        }
        else
        {
		inRetVal = inTAKA_AdviceSendRecvPacket(pobTran, inSendAdviceCnt);
                if (inRetVal != VS_SUCCESS)
                {
                        return (inRetVal);
                }
        }

        return (VS_SUCCESS);
}

/*
Function        :inTAKA_ProcessOnline
Date&Time       :2022/10/14 下午 6:31
Describe        :
*/
int inTAKA_ProcessOnline(TRANSACTION_OBJECT *pobTran)
{
        int 	inRetVal;
        char    szTemplate[512 + 1];
	char	szCustomIndicator[3 + 1] = {0};

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inTAKA_ProcessOnline() START!");
	
	memset(szCustomIndicator, 0x00, sizeof(szCustomIndicator));
	inGetCustomIndicator(szCustomIndicator);

        /* 開始組交易封包，送、收、組、解 */
        pobTran->inISOTxnCode = pobTran->inTransactionCode; /* 以 srEventMenuItem.inCode Index */
        inRetVal = inTAKA_SendPackRecvUnPack(pobTran);

        if (inRetVal == VS_COMM_ERROR)
        {
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
                /* 決定後續交易的流程先看【Field_39】再看【Field_38】 */
                pobTran->inTransactionResult = inTAKA_CheckRespCode(pobTran); /* 【Field_39】 */
                
                if (ginDebug == VS_TRUE)
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "pobTran->inTransactionResult = %d", pobTran->inTransactionResult);
                        inLogPrintf(AT, szTemplate);
                }
                
                if (pobTran->inTransactionResult == _TRAN_RESULT_AUTHORIZED_)
                {
                        if (inTAKA_CheckAuthCode(pobTran) != VS_SUCCESS)
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
                inLogPrintf(AT, "inTAKA_ProcessOnline() END!");

        return (VS_SUCCESS);
}

/*
Function        :inTAKA_ProcessOffline
Date&Time       :2022/10/14 下午 6:31
Describe        :
*/
int inTAKA_ProcessOffline(TRANSACTION_OBJECT *pobTran)
{
        int 	inRetVal = VS_SUCCESS;

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
	
	/* Online在inTAKA_ISOOnlineAnalyse On起來，用來判斷能不能重印簽單的flag 預設開 */
	pobTran->srBRec.uszMPASReprintBit = VS_TRUE;

        return (inRetVal);
}

/*
Function        :inTAKA_AnalysePacket
Date&Time       :2016/9/14 上午 10:03
Describe        :
*/
int inTAKA_AnalysePacket(TRANSACTION_OBJECT *pobTran)
{
        int				inRetVal = VS_SUCCESS, inISOFuncIndex = -1;
        ISO_TYPE_TAKA_TABLE 	srISOFunc;

        if (pobTran->srBRec.uszOfflineBit == VS_FALSE)
        {
		/* 加密模式，預設值 = 0。【0 = 不加密，1 = tSAM 加密，2 = 軟體加密】 */
		inISOFuncIndex = 0; /* 不加密 */
		memset((char *) &srISOFunc, 0x00, sizeof(srISOFunc));
		memcpy((char *) &srISOFunc, (char *) &srTAKA_ISOFunc[inISOFuncIndex], sizeof(srISOFunc));

		if (srISOFunc.inOnAnalyse != NULL)
			inRetVal = srISOFunc.inOnAnalyse(pobTran);
		else
			inRetVal = VS_ERROR;
        }

        return (inRetVal);
}

/*
Function        :inTAKA_ReversalSendRecvPacket
Date&Time       :2016/9/13 下午 4:52
Describe        :
*/
int inTAKA_ReversalSendRecvPacket(TRANSACTION_OBJECT *pobTran)
{
        int 		inRetVal, inSendCnt;
        long 		lnREVCnt;
	char		szDialBackupEnable[2 + 1];
	char		szCommMode[2 + 1];
        unsigned char 	uszFileName[20 + 1];
        unsigned char 	uszSendPacket[_TAKA_ISO_SEND_ + 1], uszRecvPacket[_TAKA_ISO_RECV_ + 1];
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
                inLogPrintf(AT, "inTAKA_CommSendRecvToHost() Before");

        if ((inRetVal = inTAKA_CommSendRecvToHost(pobTran, uszSendPacket, inSendCnt, uszRecvPacket)) != VS_SUCCESS)
        {
		pobTran->inTransactionResult = _TRAN_RESULT_COMM_ERROR_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_COMM_;
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inTAKA_CommSendRecvToHost() Error");

                return (VS_COMM_ERROR);
        }

        /* 解 ISO 電文 */
	pobTran->inISOTxnCode = _REVERSAL_;
        if ((inRetVal = inTAKA_UnPackISO(pobTran, uszSendPacket, uszRecvPacket)) != VS_SUCCESS)
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
Function        :inTAKA_ReversalSave_Flow
Date&Time       :2017/3/30 上午 11:06
Describe        :
*/
int inTAKA_ReversalSave_Flow(TRANSACTION_OBJECT *pobTran)
{
	int 	inRetVal;
	char	szDialBackupEnable[2 + 1];
	
	inRetVal = inTAKA_ReversalSave(pobTran);
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	memset(szDialBackupEnable, 0x00, sizeof(szDialBackupEnable));
	inGetDialBackupEnable(szDialBackupEnable);

	if (memcmp(szDialBackupEnable, "Y", strlen("Y")) == 0)
	{
		inRetVal = inTAKA_ReversalSave_For_DialBeckUp(pobTran);
		if (inRetVal != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inTAKA_ReversalSave
Date&Time       :2016/9/13 下午 5:06
Describe        :
*/
int inTAKA_ReversalSave(TRANSACTION_OBJECT *pobTran)
{
        int 		inRetVal, inPacketCnt;
        unsigned char 	uszReversalPacket[_TAKA_ISO_SEND_ + 1];
        unsigned char 	uszFileName[20 + 1];
        unsigned long 	srFHandle;


        memset(uszReversalPacket, 0x00, sizeof(uszReversalPacket));
        pobTran->inISOTxnCode = _REVERSAL_; /* 沖銷 */
	
        /* 組_REVERSAL_ 封包 */
        inPacketCnt = inTAKA_PackISO(pobTran, &uszReversalPacket[0], pobTran->inISOTxnCode);
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

	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
	}
	
        inFILE_Close(&srFHandle);
        inSetSendReversalBit("Y");
        if (inSaveHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
        {
                inFILE_Close(&srFHandle);
		inFILE_Delete(uszFileName);
		
                return (VS_ERROR);
        }
	
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
	}

        return (VS_SUCCESS);
}

/*
Function        :inTAKA_ReversalSave_For_DialBeckUp
Date&Time       :2017/3/30 上午 10:24
Describe        :
*/
int inTAKA_ReversalSave_For_DialBeckUp(TRANSACTION_OBJECT *pobTran)
{
        int 		inRetVal, inPacketCnt;
        unsigned char 	uszReversalPacket[_TAKA_ISO_SEND_ + 1];
        unsigned char 	uszFileName[20 + 1];
        unsigned long 	srFHandle;


        memset(uszReversalPacket, 0x00, sizeof(uszReversalPacket));
        pobTran->inISOTxnCode = _REVERSAL_; /* 沖銷 */

        /* 組_REVERSAL_ 封包 */
        inPacketCnt = inTAKA_PackISO(pobTran, &uszReversalPacket[0], pobTran->inISOTxnCode);
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

int inTAKA_CommSendRecvToHost(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, int inSendLen, unsigned char *uszRecvPacket)
{
        int		inRetVal;
        int		inSendTimeout = 10;
        int		inReceiveTimeout = 10;
	int		inReceiveSize = _COMM_RECEIVE_MAX_LENGTH_;		/* 配合Ingenico */
	char		szTimeOut[2 + 1] = {0};
        char		szHostResponseTimeOut[2 + 1] = {0};
	unsigned char	uszDispBit = VS_TRUE;

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inTAKA_CommSendRecvToHost() START!");
        
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
		vdTAKA_ISO_FormatDebug_DISP(pobTran, &uszSendPacket[0], inSendLen);
	}
	/* 列印ISO Debug */
        if (ginISODebug == VS_TRUE)
	{
                vdTAKA_ISO_FormatDebug_PRINT(pobTran, &uszSendPacket[0], inSendLen);
	}
	
	/* POS PRINTER */
	if (ginPOSPrinterDebug == VS_TRUE)
	{
		vdTAKA_ISO_FormatDebug_POS_PRINT(pobTran, &uszSendPacket[0], inSendLen);
	}
	
	/* 顯示訊息 */
	uszDispBit = VS_TRUE;
	
        if ((inRetVal = inCOMM_Send(uszSendPacket, inSendLen, inSendTimeout, uszDispBit)) != VS_SUCCESS)
        {
		inUtility_StoreTraceLog_OneStep("inCOMM_Send Error");
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

	
	/* 顯示訊息 */
	uszDispBit = VS_TRUE;
	
        if ((inRetVal = inCOMM_Receive(uszRecvPacket, inReceiveSize, inReceiveTimeout, uszDispBit)) != VS_SUCCESS)
        {
		inUtility_StoreTraceLog_OneStep("inCOMM_Receive Error");
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inCOMM_Receive() ERROR");

                return (VS_ERROR);
        }

	/* 顯示ISO Debug */
	if (ginDebug == VS_TRUE)
	{
		vdTAKA_ISO_FormatDebug_DISP(pobTran, &uszRecvPacket[0], inRetVal);
	}
        /* 列印ISO Debug */
        if (ginISODebug == VS_TRUE)
	{
                vdTAKA_ISO_FormatDebug_PRINT(pobTran, &uszRecvPacket[0], inRetVal);
	}
	/* POS PRINTER */
	if (ginPOSPrinterDebug == VS_TRUE)
	{
		vdTAKA_ISO_FormatDebug_POS_PRINT(pobTran, &uszRecvPacket[0], inRetVal);
	}
	
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inTAKA_CommSendRecvToHost() END!");
        
        return (VS_SUCCESS);
}

int inTAKA_SendPackRecvUnPack(TRANSACTION_OBJECT *pobTran)
{
        int 		inSendCnt = 0, inRecvCnt = 0, inRetVal;
        unsigned char 	uszSendPacket[_TAKA_ISO_SEND_ + 1], uszRecvPacket[_TAKA_ISO_RECV_ + 1];

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inTAKA_SendPackRecvUnPack() START!");

        memset(uszSendPacket, 0x00, sizeof(uszSendPacket));
        memset(uszRecvPacket, 0x00, sizeof(uszRecvPacket));

	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("inTAKA_SendPackRecvUnPack() START!");
	}
	
        /* 組 ISO 電文 */
        if ((inSendCnt = inTAKA_PackISO(pobTran, uszSendPacket, pobTran->inISOTxnCode)) <= 0)
        {
		inUtility_StoreTraceLog_OneStep("inTAKA_PackISO Error");
		
		pobTran->inTransactionResult = _TRAN_RESULT_PACK_ERR_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_ISO_PACK_;
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inTAKA_PackISO() Error!");

                return (VS_ISO_PACK_ERR); /* 組交易電文錯誤，不用組_REVERSAL_ */
        }
	
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("Pack END");
	}
	
        /* 傳送及接收 ISO 電文 */
        if ((inRecvCnt = inTAKA_CommSendRecvToHost(pobTran, uszSendPacket, inSendCnt, uszRecvPacket)) != VS_SUCCESS)
        {
		inUtility_StoreTraceLog_OneStep("inTAKA_CommSendRecvToHost Error");
		
		pobTran->inTransactionResult = _TRAN_RESULT_COMM_ERROR_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_COMM_;
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inTAKA_CommSendRecvToHost() Error");

                if (pobTran->inISOTxnCode != _ADVICE_)
                        memset(pobTran->srBRec.szRespCode, 0x00, sizeof(pobTran->srBRec.szRespCode));

                return (VS_COMM_ERROR);
        }
	
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("Receive END");
	}

        /* 解 ISO 電文 */
        inRetVal = inTAKA_UnPackISO(pobTran, uszSendPacket, uszRecvPacket);
        
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("Unpack END");
	}
	
        if (inRetVal != VS_SUCCESS)
        {	
		inUtility_StoreTraceLog_OneStep("inTAKA_UnPackISO Error");
		
		pobTran->inTransactionResult = _TRAN_RESULT_UNPACK_ERR_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_ISO_UNPACK_;
		
                inRetVal = VS_ISO_UNPACK_ERROR;
        }
	
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
	}
	
        return (inRetVal);
}

/*
Function        :inTAKA_CheckRespCode
Date&Time       :2016/9/14 上午 9:48
Describe        :
*/
int inTAKA_CheckRespCode(TRANSACTION_OBJECT *pobTran)
{
        int 	inRetVal = _TRAN_RESULT_COMM_ERROR_;

	if (!memcmp(pobTran->srBRec.szRespCode, "00", 2))
	{
		inRetVal = _TRAN_RESULT_AUTHORIZED_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_NONE_;
	}
	else if (!memcmp(pobTran->srBRec.szRespCode, "01", 2) || !memcmp(pobTran->srBRec.szRespCode, "02", 2))
	{
		/* 維持原回應碼 */
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
	else if (!memcmp(&pobTran->srBRec.szRespCode[0], "0X", 2) ||
		 !memcmp(&pobTran->srBRec.szRespCode[0], "A2", 2) ||
		 !memcmp(&pobTran->srBRec.szRespCode[0], "A3", 2) ||
		 !memcmp(&pobTran->srBRec.szRespCode[0], "A4", 2) ||
		 !memcmp(&pobTran->srBRec.szRespCode[0], "A5", 2) ||
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
	         !memcmp(&pobTran->srBRec.szRespCode[0], "ID", 2) ||
		 !memcmp(&pobTran->srBRec.szRespCode[0], "BE", 2) ||
		 !memcmp(&pobTran->srBRec.szRespCode[0], "XU", 2))
	{
		/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/10/1 下午 4:17 */
		if (!memcmp(&pobTran->srBRec.szRespCode[0], "BE", 2))
		{
			pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_UNY_BARCODE_DATA_ERROR_;
		}
		inRetVal = _TRAN_RESULT_CANCELLED_;
	}
        else
        {
                if ((pobTran->srBRec.szRespCode[0] >= '0' && pobTran->srBRec.szRespCode[0] <= '9') && (pobTran->srBRec.szRespCode[1] >= '0' && pobTran->srBRec.szRespCode[1] <= '9'))
                {
                        /* 要增加結帳的判斷 */
                        if ((pobTran->inISOTxnCode == _SETTLE_		||
			     pobTran->inISOTxnCode == _CLS_SETTLE_)	&& 
			    !memcmp(&pobTran->srBRec.szRespCode[0], "95", 2))
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
Function        :inTAKA_CheckAuthCode
Date&Time       :2016/9/14 上午 9:48
Describe        :
*/
int inTAKA_CheckAuthCode(TRANSACTION_OBJECT *pobTran)
{
        int 	inRetVal = VS_SUCCESS;
	char	szDebugMsg[100 + 1];
	
        switch (pobTran->inISOTxnCode)
        {
		case _SETTLE_ :
		case _BATCH_UPLOAD_ :
		case _CLS_BATCH_ :
                        break; /* 不檢核 */
                default:
                        /* 修改主機授權碼帶空白TC Upload失敗，卡Advice的問題 */
                        if ((!memcmp(&pobTran->srBRec.szAuthCode[0], "000000", 6) || !memcmp(&pobTran->srBRec.szAuthCode[0], "      ", 6)) && pobTran->srBRec.uszCUPTransBit != VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "inTAKA_CheckAuthCode Error, %s", pobTran->srBRec.szAuthCode);
					inLogPrintf(AT, szDebugMsg);
				}
				
                                inRetVal = VS_ERROR;
			}

                        break;
        }

        return (inRetVal);
}

/*
Function        :inTAKA_OnlineAnalyseMagneticManual
Date&Time       :2016/9/14 上午 9:49
Describe        :
*/
int inTAKA_OnlineAnalyseMagneticManual(TRANSACTION_OBJECT *pobTran)
{
        int		inRetVal = VS_SUCCESS;

        if (pobTran->inISOTxnCode == _SETTLE_		||
	    pobTran->inTransactionCode == _CLS_SETTLE_)
        {
		/* Settle時非95、非00的話 */
                if (memcmp(pobTran->srBRec.szRespCode, "95", 2) && memcmp(pobTran->srBRec.szRespCode, "00", 2))
                        inRetVal = VS_ERROR;
                else
                {
			/* 需要執行BatchUpload時 */
                        if (pobTran->inTransactionResult == _TRAN_RESULT_SETTLE_UPLOAD_BATCH_ && !memcmp(pobTran->srBRec.szRespCode, "95", 2))
                                inRetVal = inTAKA_ProcessSettleBatchUpload(pobTran);
			
                }

		/* 結帳成功 */
                if (inRetVal == VS_SUCCESS)
                {
                        if (inTAKA_SetReversalCnt(pobTran, _RESET_) == VS_ERROR)
                        {
                                return (VS_ERROR);
                        }
			
			/* 結帳成功 把請先結帳的bit關掉 */
			inTAKA_SetMustSettleBit(pobTran, "N");
                }
		else
		{
			/* BatchUpload失敗 */
			/* 在inTAKA_ProcessSettleBatchUpload裡面顯示主機回的錯誤訊息 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_SETTLE_NOT_SUCCESS_;
			inRetVal = VS_ERROR;
		}

        }
        else
        {
                if (pobTran->inTransactionResult == _TRAN_RESULT_AUTHORIZED_)
                {
			/* 要更新端末機的日期及時間 */
			if (inTAKA_SyncHostTerminalDateTime(pobTran) != VS_SUCCESS)
				return (VS_ERROR);

			if (pobTran->srBRec.uszOfflineBit == VS_FALSE)
			{
				inSetSendReversalBit("N");
				if (inSaveHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
				{
					return (VS_ERROR);
				}

				/* 因為是【Online】交易在這裡送【Advice】 */
				inRetVal = inTAKA_AdviceSendRecvPacket(pobTran, 1);

				/* 不管advice最後如何，只要原交易授權就回傳成功 */
				inRetVal = VS_SUCCESS;
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

			inTAKA_DispHostResponseCode(pobTran);		/* 顯示主機回的錯誤訊息 */
			
			inRetVal = VS_ERROR;
                }
                else
                {
			inTAKA_DispHostResponseCode(pobTran);		/* 顯示主機回的錯誤訊息 */
			
                        inRetVal = VS_ERROR;
                }
        }

        return (inRetVal);
}

/*
Function        :inTAKA_ProcessSettleBatchUpload
Date&Time       :2016/9/14 上午 10:08
Describe        :
*/
int inTAKA_ProcessSettleBatchUpload(TRANSACTION_OBJECT *pobTran)
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
                        if (pobTran->srBRec.uszVOIDBit == VS_TRUE)
                                continue;
			
			/* 表示要上傳的合法筆數 */
			inBatchValidCnt ++;

                        if (inBatchValidCnt == inBatchValidTotalCnt)
                                pobTran->uszLastBatchUploadBit = VS_TRUE; /* 最後一筆交易 */
			
			/* 抓最新的STAN */
			inTAKA_GetSTAN(pobTran);

			inRetVal = inTAKA_SendPackRecvUnPack(pobTran);
			/* 成功或失敗 System Trace Number 都要加一 */
			inTAKA_SetSTAN(pobTran);
			
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
					inTAKA_DispHostResponseCode(pobTran);		/* 顯示主機回的錯誤訊息 */
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
	inTAKA_GetSTAN(pobTran);

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

		inRetVal = inTAKA_SendPackRecvUnPack(pobTran);
		/* 成功或失敗 System Trace Number 都要加一 */
		inTAKA_SetSTAN(pobTran);
		
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
Function        :inTAKA_DispHostResponseCode
Date&Time       :2016/11/15 下午 5:44
Describe        :顯示錯誤代碼
*/
int inTAKA_DispHostResponseCode(TRANSACTION_OBJECT *pobTran)
{
	int		inChoice = _DisTouch_No_Event_;
	int		inRetVal = VS_SUCCESS;
	int		inDisplayFormatMode = 1;
	char		szResponseCode[10 + 1] = {0};
	char		szMsg[42 + 1] = {0};
	char		szMsg2[42 + 1] = {0};
	char		szCustomerIndicator[3 + 1] = {0};
        unsigned char   uszKey = 0x00;
	
	memset(szMsg, 0x00, sizeof(szMsg));
	memset(szMsg2, 0x00, sizeof(szMsg2));
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
			if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)) == 0)
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
		else if (!memcmp(&pobTran->srBRec.szRespCode[0], "A2", 2))
			sprintf(szMsg, "%s", "請依正確卡別操作");		/* 請依正確卡別操作 */
		else if (!memcmp(&pobTran->srBRec.szRespCode[0], "A3", 2))
			sprintf(szMsg, "%s", "卡號錯誤");			/* 卡號錯誤 */
		else if (!memcmp(&pobTran->srBRec.szRespCode[0], "A4", 2))
			sprintf(szMsg, "%s", "卡片過期");			/* 卡片過期 */
		else if (!memcmp(&pobTran->srBRec.szRespCode[0], "A5", 2))
			sprintf(szMsg, "%s", "卡號長度錯誤");			/* 卡號長度錯誤 */
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
		else if (!memcmp(&pobTran->srBRec.szRespCode[0], "BE", 2))	/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/8 下午 1:55 */
		{
			inDisplayFormatMode = 2;
			sprintf(szMsg, "%s", "支付條碼錯誤");
			sprintf(szMsg2, "%s", "請重新取得條碼");
		}
		else if (!memcmp(&pobTran->srBRec.szRespCode[0], "XU", 2))	/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/8 下午 1:55 */
		{
			inDisplayFormatMode = 2;
			sprintf(szMsg, "%s", "Üny退貨");
			sprintf(szMsg2, "%s", "比對不到原交易");
		}
		else
		{
			strcpy(pobTran->srBRec.szRespCode, "05");
			sprintf(szMsg, "%s", "拒絕交易");			/* Display 【拒絕交易】 */
		}
	}
	
	/* MPAS的規則 */
	if (!memcmp(&pobTran->srBRec.szAuthCode[0], "REJB01", 6) ||
	    !memcmp(&pobTran->srBRec.szAuthCode[0], "REJB02", 6) ||
	    !memcmp(&pobTran->srBRec.szAuthCode[0], "REJB03", 6))
	{
		sprintf(szMsg, "%s", "非參加機構卡片");				/* 非參加機構卡片 */
	}
	
	/* (需求單-110202)-修改客製化參數111卡人自助EDC UI訊息需求 by Russell 2021/12/23 下午 1:57 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_))
	{
		if (memcmp(szMsg, "請聯絡發卡銀行", strlen("請聯絡發卡銀行")) == 0)
		{
			sprintf(szMsg, "%s", "拒絕交易");	/* 拒絕交易 */
		}
	}
		
	memset(szResponseCode, 0x00, sizeof(szResponseCode));	
	sprintf(szResponseCode, "%s", pobTran->srBRec.szRespCode);		/* 錯誤代碼 */
	
	/* 如果當SDK，不用顯示 */
	if (memcmp(gszTermVersionID, "MD731UAGAS001", strlen("MD731UAGAS001")) == 0)
	{
		
	}
	/* 在客製化107.111的時候將請按確認鍵跟請按清除鍵這兩個字樣移除 */
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)         ||
                 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_))
	{
		if (inDisplayFormatMode == 2)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
			strcpy(srDispMsgObj.szErrMsg1, szResponseCode);
			srDispMsgObj.inErrMsg1Line = _LINE_8_4_;
			strcpy(srDispMsgObj.szErrMsg2, szMsg);
			srDispMsgObj.inErrMsg2Line = _LINE_8_5_;
			strcpy(srDispMsgObj.szErrMsg3, szMsg2);
			srDispMsgObj.inErrMsg3Line = _LINE_8_6_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
			strcpy(srDispMsgObj.szErrMsg1, szResponseCode);
			srDispMsgObj.inErrMsg1Line = _LINE_8_5_;
			strcpy(srDispMsgObj.szErrMsg2, szMsg);
			srDispMsgObj.inErrMsg2Line = _LINE_8_6_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
		}

		/* 為了強調Timeout時間 */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	}
        else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)     ||
                 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)) 
	{
                /* Mirror Message */
                if (pobTran->uszECRBit == VS_TRUE)
                {
                        inECR_SendMirror(pobTran, _MIRROR_MSG_DECLINED_);
                }
                
		if (inDisplayFormatMode == 2)
		{
			/* 清下排 */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);

			inDISP_ChineseFont(szResponseCode, _FONTSIZE_8X16_, _LINE_8_4_, _DISP_CENTER_);
			inDISP_ChineseFont(szMsg, _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
			inDISP_ChineseFont(szMsg2, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);

			inDISP_PutGraphic(_ERR_CLEAR_, 0, _COORDINATE_Y_LINE_8_7_);
			inDISP_BEEP(1, 0);
		}
		else
		{
			/* 清下排 */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);

			inDISP_ChineseFont(szResponseCode, _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
			inDISP_ChineseFont(szMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);

			inDISP_PutGraphic(_ERR_CLEAR_, 0, _COORDINATE_Y_LINE_8_7_);
			inDISP_BEEP(1, 0);
		}

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
                /* Mirror Message */
                if (pobTran->uszECRBit == VS_TRUE)
                {
                        inECR_SendMirror(pobTran, _MIRROR_MSG_DECLINED_);
                }

		if (inDisplayFormatMode == 2)
		{
			/* 清下排 */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);

			inDISP_ChineseFont(szResponseCode, _FONTSIZE_8X16_, _LINE_8_4_, _DISP_CENTER_);
			inDISP_ChineseFont(szMsg, _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
			inDISP_ChineseFont(szMsg2, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);

			inDISP_PutGraphic(_ERR_CLEAR_, 0, _COORDINATE_Y_LINE_8_7_);
			inDISP_BEEP(1, 0);
		}
		else
		{
			/* 清下排 */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);

			inDISP_ChineseFont(szResponseCode, _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
			inDISP_ChineseFont(szMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);

			inDISP_PutGraphic(_ERR_CLEAR_, 0, _COORDINATE_Y_LINE_8_7_);
			inDISP_BEEP(1, 0);
		}

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
Function        :inTAKA_SyncHostTerminalDateTime
Date&Time       :2016/9/14 上午 10:14
Describe        :
*/
int inTAKA_SyncHostTerminalDateTime(TRANSACTION_OBJECT *pobTran)
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
Function        :inTAKA_GetReversalCnt
Date&Time       :2016/9/13 下午 5:02
Describe        :
*/
int inTAKA_GetReversalCnt(TRANSACTION_OBJECT *pobTran)
{
        int 	inCnt;
        char 	szReversalCnt[6 + 1];

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inTAKA_GetReversalCnt() START!");

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
Function        :inTAKA_SetReversalCnt
Date&Time       :2016/9/13 下午 5:02
Describe        :
*/
int inTAKA_SetReversalCnt(TRANSACTION_OBJECT *pobTran, long lnSetMode)
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
Function        :inTAKA_GetMustSettleBit
Date&Time       :2016/9/14 上午 10:12
Describe        :
*/
int inTAKA_GetMustSettleBit(TRANSACTION_OBJECT *pobTran, char *szMustSettleBit)
{
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inTAKA_GetMustSettleBit() START!");

	/* 刷完卡就LoadHDPT，不另在Load */
        memset(szMustSettleBit, 0x00, sizeof(szMustSettleBit));
        if (inGetMustSettleBit(szMustSettleBit) == VS_ERROR)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inTAKA_GetMustSettleBit ERROR!");
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

/*
Function        :inTAKA_SetMustSettleBit
Date&Time       :2016/9/14 上午 10:12
Describe        :
*/
int inTAKA_SetMustSettleBit(TRANSACTION_OBJECT *pobTran, char *szMustSettleBit)
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
Function        :vdTAKA_ISO_FormatDebug_DISP
Date&Time       :2016/11/30 下午 4:19
Describe        :顯示ISO Debug 
*/
void vdTAKA_ISO_FormatDebug_DISP(TRANSACTION_OBJECT *pobTran, unsigned char *uszDebugBuf, int inSendLen)
{
	int			inOneLineLen = 34;
	int			i, inPrintLineCnt = 0, inCnt = 0, inField, inLen, inFieldLen;
	int			inBitMapCnt1, inBitMapCnt2;
	char			szPrtBuf[50 + 1], szBuf[1536 + 1], szBitMap[8 + 1];
	char			szPrintLineData[36 + 1];
	unsigned char		uszBitMap[9], uszBitMapDisp[66];
	ISO_TYPE_TAKA_TABLE srISOTypeTable;

	memset((char *)&srISOTypeTable, 0x00, sizeof(srISOTypeTable));
	memcpy((char *)&srISOTypeTable, (char *)&srTAKA_ISOFunc[0], sizeof(srISOTypeTable));

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
		if (!inTAKA_BitMapCheck((unsigned char *)szBitMap, i))
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
			case 60 :
				break;
			case 64 :
				strcat(szBuf, "MAC ");
				inLen += 4;
				break;
			default :
				break;
		}       
		
		inField = inTAKA_GetFieldIndex(i, srISOTypeTable.srISOFieldType);
		inFieldLen = inTAKA_GetFieldLen(i, &uszDebugBuf[inCnt], srISOTypeTable.srISOFieldType);

		switch (srISOTypeTable.srISOFieldType[inField].inFieldType)
		{
			case _TAKA_ISO_ASC_  :
				memcpy(&szBuf[inLen], &uszDebugBuf[inCnt], inFieldLen);
				break;
                        case _TAKA_ISO_BYTE_3_  :
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
                        case _TAKA_ISO_BYTE_1_ :
                                inFunc_BCD_to_ASCII(&szBuf[inLen], &uszDebugBuf[inCnt ++], 1);
				inLen ++;
				strcat(szBuf, " ");
				inLen ++;

				inFieldLen --;
                                inFunc_BCD_to_ASCII(&szBuf[inLen], &uszDebugBuf[inCnt], inFieldLen);
				break;
			case _TAKA_ISO_NIBBLE_2_  :
			case _TAKA_ISO_BYTE_2_  :
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
			case _TAKA_ISO_BCD_  :
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
Function        :vdTAKA_ISO_FormatDebug_PRINT
Date&Time       :2016/11/30 下午 4:20
Describe        :列印ISO Debug
*/
void vdTAKA_ISO_FormatDebug_PRINT(TRANSACTION_OBJECT *pobTran, unsigned char *uszDebugBuf, int inSendLen)
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
	ISO_TYPE_TAKA_TABLE srISOTypeTable;

	/* 是否有列印功能 */
	if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
	{
		
	}
	else
	{
		/* 初始化 */
		inPRINT_Buffer_Initial(uszBuffer, _BUFFER_MAX_LINE_, &srFont_Attrib, &srBhandle);

		memset((char *)&srISOTypeTable, 0x00, sizeof(srISOTypeTable));
		memcpy((char *)&srISOTypeTable, (char *)&srTAKA_ISOFunc[0], sizeof(srISOTypeTable));

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
			if (!inTAKA_BitMapCheck((unsigned char *)szBitMap, i))
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

			inField = inTAKA_GetFieldIndex(i, srISOTypeTable.srISOFieldType);
			inFieldLen = inTAKA_GetFieldLen(i, &uszDebugBuf[inCnt], srISOTypeTable.srISOFieldType);

			switch (srISOTypeTable.srISOFieldType[inField].inFieldType)
			{
				case _TAKA_ISO_ASC_  :
					memcpy(&szBuf[inLen], &uszDebugBuf[inCnt], inFieldLen);
					break;
				case _TAKA_ISO_BYTE_3_  :
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
				case _TAKA_ISO_BYTE_1_ :
					inFunc_BCD_to_ASCII(&szBuf[inLen], &uszDebugBuf[inCnt ++], 1);
					inLen ++;
					strcat(szBuf, " ");
					inLen ++;

					inFieldLen --;
					inFunc_BCD_to_ASCII(&szBuf[inLen], &uszDebugBuf[inCnt], inFieldLen);
					break;
				case _TAKA_ISO_NIBBLE_2_  :
				case _TAKA_ISO_BYTE_2_  :
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
				case _TAKA_ISO_BCD_  :
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

/*
Function        :vdTAKA_ISO_FormatDebug_POS_PRINT
Date&Time       :2018/10/24 上午 11:47
Describe        :列印ISO Debug
*/
void vdTAKA_ISO_FormatDebug_POS_PRINT(TRANSACTION_OBJECT *pobTran, unsigned char *uszDebugBuf, int inSendLen)
{
	int			inOneLineLen = 34, inPrintLineCnt = 0;
	int			i, inCnt = 0, inField, inLen, inFieldLen;
	int			inBitMapCnt1, inBitMapCnt2;
	char			szPrtBuf[50 + 1], szBuf[1536 + 1], szBitMap[8 + 1];
	char			szPrintLineData[36 + 1];
	unsigned char		uszBitMap[9], uszBitMapDisp[66];
	ISO_TYPE_TAKA_TABLE srISOTypeTable;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "vdTAKA_ISO_FormatDebug_POS_PRINT() START !");
	}

	/* 初始化 */
	inPRINT_POS_Printer_Initial();

	memset((char *)&srISOTypeTable, 0x00, sizeof(srISOTypeTable));
	memcpy((char *)&srISOTypeTable, (char *)&srTAKA_ISOFunc[0], sizeof(srISOTypeTable));

	inPRINT_POS_Printer_Send_Data_Line("==========================================", strlen("=========================================="));

	if (uszDebugBuf[6] == 0x00 || uszDebugBuf[6] == 0x20 || uszDebugBuf[6] == 0x40)
	{
		inPRINT_POS_Printer_Send_Data_Line("ISO8583 Format  <<Send Data>>", strlen("ISO8583 Format  <<Send Data>>"));
	}
	else
	{
		inPRINT_POS_Printer_Send_Data_Line("ISO8583 Format  <<Recvive Data>>", strlen("ISO8583 Format  <<Recvive Data>>"));
	}

	memset(szBuf, 0x00, sizeof(szBuf));
	sprintf(szBuf, "TPDU = [");
	inFunc_BCD_to_ASCII(&szBuf[8], &uszDebugBuf[inCnt], 5);
	strcat(szBuf,"]");
	inPRINT_POS_Printer_Send_Data_Line(szBuf, strlen(szBuf));
	inCnt += 5;

	memset(szBuf, 0x00, sizeof(szBuf));
	sprintf(szBuf,"MTI  = [");
	inFunc_BCD_to_ASCII(&szBuf[8], &uszDebugBuf[inCnt], 2);
	strcat(szBuf,"]");
	inPRINT_POS_Printer_Send_Data_Line(szBuf, strlen(szBuf));
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
	inPRINT_POS_Printer_Send_Data_Line(szBuf, strlen(szBuf));
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

	inPRINT_POS_Printer_Send_Data_Line("  1234567890   1234567890   1234567890", strlen("  1234567890   1234567890   1234567890"));
	memset(szBuf, 0x00, sizeof(szBuf));
	sprintf(szBuf, "0>%10.10s 1>%10.10s 2>%10.10s", &uszBitMapDisp[1], &uszBitMapDisp[11], &uszBitMapDisp[21]);
	inPRINT_POS_Printer_Send_Data_Line(szBuf, strlen(szBuf));
	sprintf(szBuf, "3>%10.10s 4>%10.10s 5>%10.10s", &uszBitMapDisp[31], &uszBitMapDisp[41], &uszBitMapDisp[51]);
	inPRINT_POS_Printer_Send_Data_Line(szBuf, strlen(szBuf));
	sprintf(szBuf, "6>%4.4s", &uszBitMapDisp[61]);
	inPRINT_POS_Printer_Send_Data_Line(szBuf, strlen(szBuf));

	inCnt += 8;

	for (i = 1; i <= 64; i ++)
	{
		if (!inTAKA_BitMapCheck((unsigned char *)szBitMap, i))
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

		inField = inTAKA_GetFieldIndex(i, srISOTypeTable.srISOFieldType);
		inFieldLen = inTAKA_GetFieldLen(i, &uszDebugBuf[inCnt], srISOTypeTable.srISOFieldType);

		switch (srISOTypeTable.srISOFieldType[inField].inFieldType)
		{
			case _TAKA_ISO_ASC_  :
				memcpy(&szBuf[inLen], &uszDebugBuf[inCnt], inFieldLen);
				break;
			case _TAKA_ISO_BYTE_3_  :
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
			case _TAKA_ISO_BYTE_1_ :
				inFunc_BCD_to_ASCII(&szBuf[inLen], &uszDebugBuf[inCnt ++], 1);
				inLen ++;
				strcat(szBuf, " ");
				inLen ++;

				inFieldLen --;
				inFunc_BCD_to_ASCII(&szBuf[inLen], &uszDebugBuf[inCnt], inFieldLen);
				break;
			case _TAKA_ISO_NIBBLE_2_  :
			case _TAKA_ISO_BYTE_2_  :
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
			case _TAKA_ISO_BCD_  :
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

			inPRINT_POS_Printer_Send_Data_Line(szPrtBuf, strlen(szPrtBuf));
			inPrintLineCnt ++;
		};

		inCnt += inFieldLen;
	}

	inPRINT_POS_Printer_Send_Data_Line("==========================================", strlen("=========================================="));
	
	inPRINT_POS_Printer_Cut_Paper();
}
