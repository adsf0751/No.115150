#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <ctosapi.h>
#include <curl/curl.h>
#include <sqlite3.h>
#include "../SOURCE/INCLUDES/Define_1.h"
#include "../SOURCE/INCLUDES/Define_2.h"
#include "../SOURCE/INCLUDES/TransType.h"
#include "../SOURCE/INCLUDES/Transaction.h"
#include "../SOURCE/INCLUDES/AllStruct.h"
#include "../SOURCE/DISPLAY/Display.h"
#include "../SOURCE/DISPLAY/DisTouch.h"
#include "../SOURCE/PRINT/Print.h"
#include "../SOURCE/FUNCTION/Accum.h"
#include "../SOURCE/FUNCTION/Sqlite.h"
#include "../SOURCE/FUNCTION/CDTX.h"
#include "../SOURCE/FUNCTION/CFGT.h"
#include "../SOURCE/FUNCTION/CPT.h"
#include "../SOURCE/FUNCTION/EDC.h"
#include "../SOURCE/FUNCTION/HDT.h"
#include "../SOURCE/FUNCTION/HDPT.h"
#include "../SOURCE/FUNCTION/ECR.h"
#include "../SOURCE/FUNCTION/KMS.h"
#include "../SOURCE/FUNCTION/Function.h"
#include "../SOURCE/FUNCTION/FuncTable.h"
#include "../SOURCE/FUNCTION/Utility.h"
#include "../SOURCE/FUNCTION/NexsysSDK.h"
#include "../SOURCE/COMM/Comm.h"
#include "../SOURCE/EVENT/Flow.h"
#include "../SOURCE/EVENT/MenuMsg.h"
#include "NCCCsrc.h"
#include "NCCCats.h"
#include "NCCCloyalty.h"
#include "NCCCtmsCPT.h"
#include "TAKAsrc.h"
#include "NCCCtSAM.h"
#include "NCCCEWiso.h"

unsigned char		guszNCCC_EW_ISO_Field03[_NCCC_EW_PCODE_SIZE_ + 1];
unsigned char		guszEW_MTI[4 + 1];
extern	char		gszMAC_F_03[2 + 1];	/* Field_3:	Processing Code */
extern	char		gszMAC_F_04[12 + 1];	/* Field_4:	Amount, Transaction */
extern	char		gszMAC_F_11[2 + 1];	/* Field_11:	System Trace Audit Number */
extern	char		gszMAC_F_35[40 + 1];	/* Field_35:	TRACK 2 Data */
extern	char		gszMAC_F_59[18 + 1];	/* Field_59:	F _59 之 Table ID “ 電票交易訊 )18 碼。 */
extern	char		gszMAC_F_63[45 + 1];	/* Field_63:	Reserved-Private Data */
extern	unsigned char	guszField_35;
extern	unsigned char	gusztSAMKeyIndex;
extern	unsigned char	gusztSAMCheckSum_35[4 + 1];
extern	unsigned char	gusztSAMCheckSum_59[4 + 1];
extern	int		ginMacError;
extern	int		ginDebug; /* Debug使用 extern */
extern	int		ginISODebug;
extern	int		ginDisplayDebug;
extern	int		ginMachineType;
extern	int		ginAPVersionType;
extern	int		ginFindRunTime;
extern	char		gszTermVersionID[16 + 1];


ISO_FIELD_NCCC_EW_TABLE srNCCC_EW_ISOFieldPack[] =
{
        {3,             inNCCC_EW_Pack03},
        {4,             inNCCC_EW_Pack04},
        {11,            inNCCC_EW_Pack11},
        {12,            inNCCC_EW_Pack12},
        {13,            inNCCC_EW_Pack13},
        {22,            inNCCC_EW_Pack22},
        {24,            inNCCC_EW_Pack24},
        {25,            inNCCC_EW_Pack25},
        {32,            inNCCC_EW_Pack32},
        {35,            inNCCC_EW_Pack35},
        {37,            inNCCC_EW_Pack37},
        {38,            inNCCC_EW_Pack38},
        {41,            inNCCC_EW_Pack41},
        {42,            inNCCC_EW_Pack42},
        {48,            inNCCC_EW_Pack48},
        {59,            inNCCC_EW_Pack59},
        {60,            inNCCC_EW_Pack60},
        {62,            inNCCC_EW_Pack62},
        {64,            inNCCC_EW_Pack64},
        {0,             NULL},  /* 最後一組一定要放 0 */
};

ISO_FIELD_NCCC_EW_TABLE srNCCC_EW_ISOFieldPack_tSAM[] =
{
        {3,             inNCCC_EW_Pack03},
        {4,             inNCCC_EW_Pack04},
        {11,            inNCCC_EW_Pack11},
        {12,            inNCCC_EW_Pack12},
        {13,            inNCCC_EW_Pack13},
        {22,            inNCCC_EW_Pack22},
        {24,            inNCCC_EW_Pack24},
        {25,            inNCCC_EW_Pack25},
        {32,            inNCCC_EW_Pack32},
        {35,            inNCCC_EW_Pack35_tSAM},
        {37,            inNCCC_EW_Pack37},
        {38,            inNCCC_EW_Pack38},
        {41,            inNCCC_EW_Pack41},
        {42,            inNCCC_EW_Pack42},
        {48,            inNCCC_EW_Pack48},
        {57,            inNCCC_EW_Pack57_tSAM},
        {59,            inNCCC_EW_Pack59},
        {60,            inNCCC_EW_Pack60},
        {62,            inNCCC_EW_Pack62},
        {64,            inNCCC_EW_Pack64},
        {0,             NULL},  /* 最後一組一定要放 0 */
};


ISO_FIELD_NCCC_EW_TABLE srNCCC_EW_ISOFieldUnPack[] =
{
	{4,		inNCCC_EW_UnPack04},
        {12,            inNCCC_EW_UnPack12},
        {13,            inNCCC_EW_UnPack13},
        {24,            inNCCC_EW_UnPack24},
        {38,            inNCCC_EW_UnPack38},
        {39,            inNCCC_EW_UnPack39},
        {59,            inNCCC_EW_UnPack59},
        {60,            inNCCC_EW_UnPack60},
	{62,		inNCCC_EW_UnPack62},
        {0,             NULL}, /* 最後一組一定要放【0】!! */
};

ISO_CHECK_NCCC_EW_TABLE srNCCC_EW_ISOFieldCheck[] =
{
        {3,             inNCCC_EW_Check03},
        {4,             inNCCC_EW_Check04},
        {41,            inNCCC_EW_Check41},
        {0,		NULL}, /* 最後一組一定要放【0】!! */
};

ISO_FIELD_TYPE_NCCC_EW_TABLE srNCCC_EW_ISOFieldType[] =
{
        {3,             _NCCC_EW_ISO_BCD_,         VS_FALSE,       6},
        {4,             _NCCC_EW_ISO_BCD_,         VS_FALSE,       12},
        {11,            _NCCC_EW_ISO_BCD_,         VS_FALSE,       6},
        {12,            _NCCC_EW_ISO_BCD_,         VS_FALSE,       6},
        {13,            _NCCC_EW_ISO_BCD_,         VS_FALSE,       4},
        {22,            _NCCC_EW_ISO_BCD_,         VS_FALSE,       4},
        {24,            _NCCC_EW_ISO_BCD_,         VS_FALSE,       4},
        {25,            _NCCC_EW_ISO_BCD_,         VS_FALSE,       2},
        {32,            _NCCC_EW_ISO_BYTE_1_,      VS_FALSE,	    0},
        {35,            _NCCC_EW_ISO_NIBBLE_2_,    VS_FALSE,       0},
        {37,            _NCCC_EW_ISO_ASC_,         VS_FALSE,       12},
        {38,            _NCCC_EW_ISO_ASC_,         VS_FALSE,       6},
        {39,            _NCCC_EW_ISO_ASC_,         VS_FALSE,       2},
        {41,            _NCCC_EW_ISO_ASC_,         VS_FALSE,       8},
        {42,            _NCCC_EW_ISO_ASC_,         VS_FALSE,       15},
        {48,            _NCCC_EW_ISO_BYTE_3_,      VS_TRUE,        0},
        {52,            _NCCC_EW_ISO_BCD_,         VS_FALSE,       16},
        {54,            _NCCC_EW_ISO_BYTE_3_,      VS_TRUE,        0},
        {55,            _NCCC_EW_ISO_BYTE_3_,      VS_FALSE,       0},
        {57,            _NCCC_EW_ISO_BYTE_3_,      VS_FALSE,       0},
        {58,            _NCCC_EW_ISO_BYTE_3_,      VS_TRUE,        0},
        {59,            _NCCC_EW_ISO_BYTE_3_,      VS_TRUE,        0},
        {60,            _NCCC_EW_ISO_BYTE_3_,      VS_TRUE,        0},
        {62,            _NCCC_EW_ISO_BYTE_3_,      VS_TRUE,        0},
        {63,            _NCCC_EW_ISO_BYTE_3_,      VS_TRUE,        0},
        {64,            _NCCC_EW_ISO_BCD_,         VS_FALSE,       16},
        {0,             _NCCC_EW_ISO_BCD_,         VS_FALSE,       0},  /* 最後一組一定要放 0 */
};

/* sale */
int inNCCC_EW_EW_SALE[] = {3, 4, 11, 22, 24, 25, 32, 35, 37, 41, 42, 59, 60, 62, 64, 0}; /* 最後一組一定要放 0!! */
/* refund */
int inNCCC_EW_EW_REFUND[] = {3, 4, 11, 22, 24, 25, 32, 35, 37, 41, 42, 59, 60, 62, 64, 0}; /* 最後一組一定要放 0!! */
/* 查詢 */
int inNCCC_EW_EW_INQUIRY[] = {3, 11, 24, 25, 32, 37, 41, 42, 59, 0}; /* 最後一組一定要放 0!! */

BIT_MAP_NCCC_EW_TABLE srNCCC_EW_ISOBitMap[] =
{
	{_EW_SALE_,			inNCCC_EW_EW_SALE,			"0200",		"000000"},
	{_EW_REFUND_,			inNCCC_EW_EW_REFUND,			"0200",		"200000"},
	{_EW_INQUIRY_,			inNCCC_EW_EW_INQUIRY,			"0800",		"0061A0"},
	
	{_NCCC_EW_NULL_TX_,		NULL,					"0000",		"000000"}, /* 最後一組一定要放 TMS_NULL_TX!! */
};

ISO_TYPE_NCCC_EW_TABLE srNCCC_EW_ISOFunc[] =
{
        {
           srNCCC_EW_ISOFieldPack,
           srNCCC_EW_ISOFieldUnPack,
           srNCCC_EW_ISOFieldCheck,
           srNCCC_EW_ISOFieldType,
           srNCCC_EW_ISOBitMap,
           inNCCC_EW_ISOPackMessageType,
           inNCCC_EW_ISOModifyBitMap,
           inNCCC_EW_ISOModifyPackData,
           inNCCC_EW_ISOCheckHeader,
           inNCCC_EW_ISOOnlineAnalyse,
	   NULL
        },

        /* tSAM用 */
        {
           srNCCC_EW_ISOFieldPack_tSAM,
           srNCCC_EW_ISOFieldUnPack,
           srNCCC_EW_ISOFieldCheck,
           srNCCC_EW_ISOFieldType,
           srNCCC_EW_ISOBitMap,
           inNCCC_EW_ISOPackMessageType,
           inNCCC_EW_ISOModifyBitMap,
           inNCCC_EW_ISOModifyPackData,
           inNCCC_EW_ISOCheckHeader,
           inNCCC_EW_ISOOnlineAnalyse,
           NULL
        }
};

/*
Function        :inNCCC_EW_Func_SetTxnOnlineOffline
Date&Time       :2023/2/21 下午 2:31
Describe        :根據交易別決定是否Online
Log				:Done
*/
int inNCCC_EW_Func_SetTxnOnlineOffline(TRANSACTION_OBJECT *pobTran)
{
        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_EW_Func_SetTxnOnlineOffline START!");
        
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inNCCC_EW_Func_SetTxnOnlineOffline START!");
        }
        
        /* 電子錢包無offline交易 */
	pobTran->srBRec.uszOfflineBit = VS_FALSE; /* 強制Online */
	pobTran->srBRec.uszUpload1Bit = VS_FALSE; /* 強制Online */
	pobTran->srBRec.uszUpload2Bit = VS_FALSE;
	pobTran->srBRec.uszUpload3Bit = VS_FALSE;
	
	pobTran->uszReversalBit = VS_FALSE;
	
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inNCCC_EW_Func_SetTxnOnlineOffline END!");
        }    
        
        return (VS_SUCCESS);
}

/*
Function        :inNCCC_EW_Func_BuildAndSendPacket
Date&Time       :2023/2/21 下午 4:16
Describe        :處理交易，發送並分析
*/
int inNCCC_EW_Func_BuildAndSendPacket(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	char	szDemoMode[2 + 1] = {0};
	
        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_EW_Func_BuildAndSendPacket START!");
        
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_EW_Func_BuildAndSendPacket() START!");
	
	/* 教育訓練模式 */
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		inRetVal = inNCCC_Func_BuildAndSendPacket_Demo_Flow(pobTran);
		/* 步驟 1 Get System Trans Number */
		inNCCC_EW_GetSTAN(pobTran);
		/* Set STAN */
		inNCCC_EW_SetSTAN(pobTran);
		
		/* 交易失敗 */
		if (inRetVal != VS_SUCCESS)
		{
			if (strlen(pobTran->srBRec.szRespCode) > 0)
			{
				inNCCC_EW_DispHostResponseCode(pobTran);
			}
			
			return (inRetVal);
		}

		return (VS_SUCCESS);
	}
	else
	{	
		/* 步驟 1 Get System Trans Number */
		if (inNCCC_EW_GetSTAN(pobTran) == VS_ERROR)
		{
			pobTran->inErrorMsg = _ERROR_CODE_V3_GET_HDPT_TAG_FAIL_;
			return (VS_ERROR);
		}

		inFunc_ResetTitle(pobTran);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned("交易處理中", _FONTSIZE_8X16_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_Y_LINE_8_5_, VS_FALSE, _DISP_CENTER_);		/* 第五層顯示 "交易處理中" */

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

		
		/* 處理【ONLINE】交易 */
		if ((inRetVal = inNCCC_EW_ProcessOnline(pobTran)) != VS_SUCCESS)
		{
			inNCCC_EW_SetSTAN(pobTran); /* 成功或失敗 System_Trace_Number 都要加一 */

			return (inRetVal); /* Return 【VS_ERROR】【VS_SUCCESS】 */
		}
	

		inNCCC_EW_SetSTAN(pobTran); /* 成功或失敗 System_Trace_Number 都要加一 */

		/* 1. 表示有收到完整的資料後的分析
		   2. 處理 Online & Offline 交易
		 */
		inRetVal = inNCCC_EW_AnalysePacket(pobTran);
		if (inRetVal != VS_SUCCESS)
		{
			inUtility_StoreTraceLog_OneStep("inNCCC_EW_AnalysePacket Error");
			
			return (inRetVal);
		}

		/* Reset Ttile */
		inFunc_ResetTitle(pobTran);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned("交易處理中", _FONTSIZE_8X16_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_Y_LINE_8_5_, VS_FALSE, _DISP_CENTER_);		/* 第五層顯示 "交易處理中" */

		/* 避免撥接太久，收送完就斷線 */
		inCOMM_End(pobTran);

		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "inNCCC_EW_Func_BuildAndSendPacket() END!");

		return (VS_SUCCESS);
	}
}

/*
Function        :inNCCC_EW_GetSTAN
Date&Time       :2023/2/21 下午 4:27
Describe        :STAN++
*/
int inNCCC_EW_GetSTAN(TRANSACTION_OBJECT *pobTran)
{
        char 	szSTANNum[12 + 1];

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_EW_GetSTAN() START!");

        memset(szSTANNum, 0x00, sizeof(szSTANNum));
        if (inGetSTANNum(szSTANNum) == VS_ERROR)
                return (VS_ERROR);

        pobTran->srBRec.lnSTANNum = atol(szSTANNum);

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_EW_SetSTAN
Date&Time       :2023/2/21 下午 4:27
Describe        :STAN++
*/
int inNCCC_EW_SetSTAN(TRANSACTION_OBJECT *pobTran)
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
Function        :inNCCC_EW_ProcessOnline
Date&Time       :2023/8/1 下午 5:46
Describe        :
*/
int inNCCC_EW_ProcessOnline(TRANSACTION_OBJECT *pobTran)
{
        int 	inRetVal;
        char    szTemplate[512 + 1];

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_EW_ProcessOnline() START!");

        /* 開始組交易封包，送、收、組、解 */
        pobTran->inISOTxnCode = pobTran->inTransactionCode; /* 以 srEventMenuItem.inCode Index */
        inRetVal = inNCCC_EW_SendPackRecvUnPack(pobTran);

        if (inRetVal == VS_COMM_ERROR)
        {
		pobTran->inTransactionResult = _TRAN_RESULT_COMM_ERROR_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_COMM_;
		pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TIMEOUT_;
                
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
                pobTran->inTransactionResult = inNCCC_EW_CheckRespCode(pobTran); /* 【Field_39】 */
                
                if (ginDebug == VS_TRUE)
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "pobTran->inTransactionResult = %d", pobTran->inTransactionResult);
                        inLogPrintf(AT, szTemplate);
                }
                
                if (pobTran->inTransactionResult == _TRAN_RESULT_AUTHORIZED_)
                {
                        if (inNCCC_EW_CheckAuthCode(pobTran) != VS_SUCCESS)
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
                inLogPrintf(AT, "inNCCC_EW_ProcessOnline() END!");

        return (VS_SUCCESS);
}

int inNCCC_EW_SendPackRecvUnPack(TRANSACTION_OBJECT *pobTran)
{
        int 		inSendCnt = 0, inRecvCnt = 0, inRetVal;
        unsigned char 	uszSendPacket[_NCCC_EW_ISO_SEND_ + 1], uszRecvPacket[_NCCC_EW_ISO_RECV_ + 1];

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_EW_SendPackRecvUnPack() START!");

        memset(uszSendPacket, 0x00, sizeof(uszSendPacket));
        memset(uszRecvPacket, 0x00, sizeof(uszRecvPacket));

	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("inNCCC_EW_SendPackRecvUnPack() START!");
	}
	
        /* 組 ISO 電文 */
        if ((inSendCnt = inNCCC_EW_PackISO(pobTran, uszSendPacket, pobTran->inISOTxnCode)) <= 0)
        {
			/* inNCCC_EW_PackISO()已有Log紀錄是Get Value錯誤還是Pack欄位錯誤，這邊不再紀錄Log */
//		inUtility_StoreTraceLog_OneStep("inNCCC_EW_PackISO Error");
		
		pobTran->inTransactionResult = _TRAN_RESULT_PACK_ERR_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_ISO_PACK_;
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inNCCC_EW_PackISO() Error!");

                return (VS_ISO_PACK_ERR); /* 組交易電文錯誤，不用組_REVERSAL_ */
        }
	
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("Pack END");
	}
	
        /* 傳送及接收 ISO 電文 */
        if ((inRecvCnt = inNCCC_EW_CommSendRecvToHost(pobTran, uszSendPacket, inSendCnt, uszRecvPacket)) != VS_SUCCESS)
        {
//		inUtility_StoreTraceLog_OneStep("inNCCC_EW_CommSendRecvToHost Error");
		
		pobTran->inTransactionResult = _TRAN_RESULT_COMM_ERROR_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_COMM_;
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inNCCC_EW_CommSendRecvToHost() Error");

                if (pobTran->inISOTxnCode != _ADVICE_)
                        memset(pobTran->srBRec.szRespCode, 0x00, sizeof(pobTran->srBRec.szRespCode));

                return (VS_COMM_ERROR);
        }
	
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("Receive END");
	}

        /* 解 ISO 電文 */
        inRetVal = inNCCC_EW_UnPackISO(pobTran, uszSendPacket, uszRecvPacket);
        
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("Unpack END");
	}
	
        if (inRetVal != VS_SUCCESS)
        {	
//		inUtility_StoreTraceLog_OneStep("inNCCC_EW_UnPackISO Error");
		
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
	
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
	}
	
        return (inRetVal);
}

int inNCCC_EW_PackISO(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendBuf, int inTxnCode)
{
        int				i, inSendCnt = 0, inField = 0, inCnt;
	int				inBitMap[_NCCC_EW_MAX_BIT_MAP_CNT_ + 1];
	int				inRetVal, inISOFuncIndex = -1;
	char				szTemplate[42 + 1];
	char				szLogMessage[40 + 1];
	char				szCommMode[1 + 1];
	char				szEncryptMode[1 +1];
	unsigned char			uszBCD[20 + 1];
	ISO_TYPE_NCCC_EW_TABLE		srISOFunc;		
	vdUtility_SYSFIN_LogMessage(AT,"inNCCC_EW_PackISO START");
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_EW_PackISO() START!");
	
        inSendCnt = 0;
        inField = 0;

        /* 加密模式，預設值 = 0。【0 = 不加密，1 = tSAM 加密】 */
	memset(szCommMode, 0x00, sizeof(szCommMode));
	inGetCommMode(szCommMode);

	if (memcmp(szCommMode, _COMM_MODEM_MODE_, strlen(_COMM_MODEM_MODE_)) == 0)
		inISOFuncIndex = 0; /* 不加密 */
	else if (memcmp(szCommMode, _COMM_ETHERNET_MODE_, strlen(_COMM_ETHERNET_MODE_)) == 0)
	{
		/* 初始化tSAM加密用欄位 */
		inNCCC_Func_Init_tSAM_Data();

		memset(szEncryptMode, 0x00, sizeof(szEncryptMode));
		inGetEncryptMode(szEncryptMode);
		if (memcmp(szEncryptMode, _NCCC_ENCRYPTION_TSAM_, strlen(_NCCC_ENCRYPTION_TSAM_)) == 0)
		{
			inISOFuncIndex = 1;
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
		
		memset(szEncryptMode, 0x00, sizeof(szEncryptMode));
		inGetEncryptMode(szEncryptMode);
		if (memcmp(szEncryptMode, _NCCC_ENCRYPTION_TSAM_, strlen(_NCCC_ENCRYPTION_TSAM_)) == 0)
		{
			inISOFuncIndex = 1;
		}
		else
		{
			inISOFuncIndex = 0;
		}
	}
	else if (memcmp(szCommMode, _COMM_WIFI_MODE_, strlen(_COMM_WIFI_MODE_)) == 0)
	{
		/* 初始化tSAM加密用欄位 */
		inNCCC_Func_Init_tSAM_Data();
		
		memset(szEncryptMode, 0x00, sizeof(szEncryptMode));
		inGetEncryptMode(szEncryptMode);
		if (memcmp(szEncryptMode, _NCCC_ENCRYPTION_TSAM_, strlen(_NCCC_ENCRYPTION_TSAM_)) == 0)
		{
			inISOFuncIndex = 1;
		}
		else
		{
			inISOFuncIndex = 0;
		}
	}
	else
	{
		/* 初始化tSAM加密用欄位 */
		inNCCC_Func_Init_tSAM_Data();
		vdUtility_SYSFIN_LogMessage(AT,"No such CommMode! szCommMode = %d", atoi(szCommMode));
		if (ginDebug == VS_TRUE)
		{
			memset(szLogMessage, 0x00, sizeof(szLogMessage));
			sprintf(szLogMessage,"No such CommMode! szCommMode = %d", atoi(szCommMode));
			inLogPrintf(AT, szLogMessage);
		}
		return (VS_ERROR); /* 防呆 */
	}
	/* szCommMode會設定inISOFuncIndex，讓inISOFuncIndex不是-1，正常來說，不會進入此防呆? */
	if (inISOFuncIndex >= 2 || inISOFuncIndex < 0)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szLogMessage, 0x00, sizeof(szLogMessage));
			sprintf(szLogMessage,"szEncryptMode ERROR!! szEncryptMode = %d",inISOFuncIndex);
			inLogPrintf(AT, szLogMessage);
		}
		if (ginDisplayDebug == VS_TRUE)
		{
			memset(szLogMessage, 0x00, sizeof(szLogMessage));
			sprintf(szLogMessage, "加密模式錯誤");
			inDISP_LOGDisplay(szLogMessage, _FONTSIZE_16X22_, _LINE_16_2_, VS_FALSE);
		}
		return (VS_ERROR); /* 防呆 */
	}
	
        /* 決定要執行第幾個 Function Index */
        memset((char *) &srISOFunc, 0x00, sizeof(srISOFunc));
        memcpy((char *) &srISOFunc, (char *) &srNCCC_EW_ISOFunc[inISOFuncIndex], sizeof(srISOFunc));
        memset((char *) inBitMap, 0x00, sizeof(inBitMap));

        /* 開始組 ISO 電文 */
        /* Pack TPDU */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        if (inGetTPDU(szTemplate) != VS_SUCCESS)
        {
		vdUtility_SYSFIN_LogMessage(AT,"inGetTPDU Error");
		
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inGetTPDU() ERROR!");

                return (VS_ERROR);
        }

        memset(uszBCD, 0x00, sizeof(uszBCD));
        inFunc_ASCII_to_BCD(uszBCD, szTemplate, 5);
        memcpy((char *) &uszSendBuf[inSendCnt], (char *) uszBCD, _NCCC_EW_TPDU_SIZE_);
        inSendCnt += _NCCC_EW_TPDU_SIZE_;
        /* Get Bit Map / Mesage Type / Processing Code */
        inRetVal = inNCCC_EW_GetBitMapMessagegTypeField03(pobTran, &srISOFunc, inTxnCode, inBitMap, &uszSendBuf[inSendCnt]);
        if (inRetVal == VS_ERROR)
        {
			vdUtility_SYSFIN_LogMessage(AT,"inNCCC_EW_GetBitMapMessagegTypeField03 Error");
                if (ginDebug == VS_TRUE)
		{
                        inLogPrintf(AT, "inNCCC_EW_GetBitMapMessagegTypeField03() ERROR!");
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
            /* 
			 * inCnt == 0 ，可能是要pack的資料為空，
			 * inCnt == -1，則為回傳值VS_ERROR 
			 */
			/* pack的字數小於等於0，一定出錯 */
                        if (inCnt <= 0)
                        {
							vdUtility_SYSFIN_LogMessage(AT,"inField = %d Error", srISOFunc.srPackISO[inField].inFieldNum);
				
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

        if (srISOFunc.inModifyPackData != _NCCC_EW_NULL_TX_)
                srISOFunc.inModifyPackData(pobTran, uszSendBuf, &inSendCnt);
		
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_EW_PackISO() END!");

        return (inSendCnt);
}

/*
Function        :inNCCC_EW_GetBitMapMessagegTypeField03
Date&Time       :2023/8/8 下午 6:24
Describe        :
*/
int inNCCC_EW_GetBitMapMessagegTypeField03(TRANSACTION_OBJECT *pobTran, ISO_TYPE_NCCC_EW_TABLE *srISOFunc, int inIsoType, int *inTxnBitMap, unsigned char *uszSendBuf)
{
        int 		i, j, k, inBitMapIndex, *inBitMap, inCnt, inBitMapTxnCode = -1;
        char		szTemplate[64 + 1];
	unsigned char 	uszBuf;

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_EW_GetBitMapMessagegTypeField03() START!");
        
        if (ginDebug == VS_TRUE)
        {
                sprintf(szTemplate, "inIsoType = %d", inIsoType);
                inLogPrintf(AT, szTemplate);
        }      
        
        /* 設定交易別 */
        inBitMapTxnCode = inIsoType;
      
        /* 要搜尋 BIT_MAP_NCCC_EW_TABLE srNCCC_EW_ISOBitMap 相對應的 inTxnID */
        if ((inBitMapIndex = inNCCC_EW_GetBitMapTableIndex(srISOFunc, inBitMapTxnCode)) == VS_ERROR)
        {		
			vdUtility_SYSFIN_LogMessage(AT,"inNCCC_EW_GetBitMapTableIndex Error");
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inNCCC_EW_GetBitMapTableIndex == VS_ERROR");
                
                return (VS_ERROR);
        }

        /* Pack Message Type */
        inCnt = 0;
        inCnt += srISOFunc->inPackMTI(pobTran, inIsoType, &uszSendBuf[inCnt], srISOFunc->srBitMap[inBitMapIndex].szMTI);

        /* 要搜尋 BIT_MAP_NCCC_EW_TABLE srNCCC_EW_ISOBitMap 相對應的 inBitMap */
        inNCCC_EW_CopyBitMap(inTxnBitMap, srISOFunc->srBitMap[inBitMapIndex].inBitMap);

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

        inCnt += _NCCC_EW_BIT_MAP_SIZE_;

        /* Process Code */
	/* 將MAC初始化 */
	memset(gszMAC_F_03, 0x00, sizeof(gszMAC_F_03));
	memset(gszMAC_F_04, 0x00, sizeof(gszMAC_F_04));
	memset(gszMAC_F_11, 0x00, sizeof(gszMAC_F_11));
	memset(gszMAC_F_35, 0x00, sizeof(gszMAC_F_35));
	memset(gszMAC_F_63, 0x00, sizeof(gszMAC_F_63));
		
        memset(guszNCCC_EW_ISO_Field03, 0x00, sizeof(guszNCCC_EW_ISO_Field03));
        inFunc_ASCII_to_BCD(&guszNCCC_EW_ISO_Field03[0], srISOFunc->srBitMap[inBitMapIndex].szPCode, 3);

	/* (需求單-111155)-電子錢包業者收付訊息整合平台需求 by Russell 2023/7/24 下午 6:17 */
	if (pobTran->srEWRec.uszEWTransBit == VS_TRUE)
	{
		guszNCCC_EW_ISO_Field03[2] = guszNCCC_EW_ISO_Field03[2] | 0xA0;
	}

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_EW_GetBitMapMessagegTypeField03() END!");
        
        return (inCnt);
}

/*
Function        :inNCCC_EW_GetBitMapTableIndex
Date&Time       :2023/8/8 下午 6:34
Describe        :
*/
int inNCCC_EW_GetBitMapTableIndex(ISO_TYPE_NCCC_EW_TABLE *srISOFunc, int inBitMapTxnCode)
{
        int 	inBitMapIndex;

        for (inBitMapIndex = 0;; inBitMapIndex++)
        {
                if (srISOFunc->srBitMap[inBitMapIndex].inTxnID == _NCCC_EW_NULL_TX_)
                {
                        return (VS_ERROR); /* 找不到相對應交易別的Bit Map */
                }

                if (srISOFunc->srBitMap[inBitMapIndex].inTxnID == inBitMapTxnCode)
                        break; /* 找到一樣的交易類別 */
        }

        return (inBitMapIndex);
}

/*
Function        :inNCCC_EW_CopyBitMap
Date&Time       :
Describe        :
*/
int inNCCC_EW_CopyBitMap(int *inBitMap, int *inSearchBitMap)
{
        int 	i;

        for (i = 0; i < _NCCC_EW_MAX_BIT_MAP_CNT_; i++)
        {
                if (inSearchBitMap[i] == 0)
                        break;
                else
                        inBitMap[i] = inSearchBitMap[i];
        }

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_EW_ISOPackMessageType
Date&Time       :2023/8/9 上午 11:33
Describe        :把MTI放到PackData中，並放到guszEW_MTI中以便在pack其他欄位時判斷
*/
int inNCCC_EW_ISOPackMessageType(TRANSACTION_OBJECT *pobTran, int inISOTxnCode, unsigned char *uszPackData, char *szMTI)
{
        int 		inCnt = 0;
	char		szFinalMTI[4 + 1] = {0};
        unsigned char 	uszBCD[10 + 1];
	
	memset(szFinalMTI, 0x00, sizeof(szFinalMTI));
	strcpy(szFinalMTI, szMTI);

	/* 放到global中方便判斷 */
	memcpy((char*)guszEW_MTI, szFinalMTI, 4);
	
	/* 放到把MTI放到PackData中 */
        memset(uszBCD, 0x00, sizeof(uszBCD));
        inFunc_ASCII_to_BCD(&uszBCD[0], &szFinalMTI[0], _NCCC_EW_MTI_SIZE_);
        memcpy((char *) &uszPackData[inCnt], (char *) &uszBCD[0], _NCCC_EW_MTI_SIZE_);
        inCnt += _NCCC_EW_MTI_SIZE_;

        return (inCnt);
}

/*
Function        :inNCCC_EW_ISOModifyBitMap
Date&Time       :2023/8/10 下午 5:59
Describe        :
*/
int inNCCC_EW_ISOModifyBitMap(TRANSACTION_OBJECT *pobTran, int inISOTxnCode, int *inBitMap)
{
	char	szCommMode[2 + 1], szEncryptMode[2 + 1];
	char	szMACEnable[2 + 1];
        	
	
	/* Uny交易退貨和batch upload不送F35(取消退貨要送) */
	if (pobTran->srEWRec.uszEWTransBit == VS_TRUE)
	{
		if (pobTran->srBRec.inCode == _EW_REFUND_)
		{
			inNCCC_EW_BitMapReset(inBitMap, 35);
		}
	}
	
	/* F_48 */
	/* 有輸入櫃號才加送F_48 */
	/* 電子錢包交易查詢不送櫃號 */
	if (strlen(pobTran->srBRec.szStoreID) > 0	&&
	    pobTran->inTransactionCode != _EW_INQUIRY_)
	{
		inNCCC_EW_BitMapSet(inBitMap, 48);
	}

	/* F_57 */
	/* 撥被備援轉成撥接時，要送不加密的電文 */
	if (pobTran->uszDialBackup == VS_TRUE)
	{
		
	}
	else
	{
		/* 有Tsam 要加送 F_57，除了0500和CUP LOGON */
		memset(szCommMode, 0x00, sizeof(szCommMode));
		inGetCommMode(szCommMode);

		if (memcmp(szCommMode, _COMM_ETHERNET_MODE_, 1) == 0	|| 
		    memcmp(szCommMode, _COMM_GPRS_MODE_, 1) == 0	||
		    memcmp(szCommMode, _COMM_3G_MODE_, 1) == 0		||
		    memcmp(szCommMode, _COMM_4G_MODE_, 1) == 0		||
		    memcmp(szCommMode, _COMM_WIFI_MODE_, 1) == 0)
		{
			memset(szEncryptMode, 0x00, sizeof(szEncryptMode));
			inGetEncryptMode(szEncryptMode);

			if (memcmp(szEncryptMode, _NCCC_ENCRYPTION_TSAM_, 1) == 0)
			{
				if (!memcmp(&guszEW_MTI[0], "0800", 4))
				{
					
				}
				else
				{
					if (pobTran->srBRec.inCode == _EW_REFUND_)
					{

					}
					else
					{
						inNCCC_EW_BitMapSet(inBitMap, 57);
					}
				}
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
	     !memcmp(&guszEW_MTI[0], "0800", 4))
	{
		inNCCC_EW_BitMapReset(inBitMap, 64);
	}

        return (VS_SUCCESS);
}

int inNCCC_EW_BitMapSet(int *inBitMap, int inFeild)
{
        int 	i, inBMapCnt, inBitMapCnt;
        int 	inBMap[_NCCC_EW_MAX_BIT_MAP_CNT_];

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

int inNCCC_EW_BitMapReset(int *inBitMap, int inFeild)
{
        int 	i, inBMapCnt;
        int 	inBMap[_NCCC_EW_MAX_BIT_MAP_CNT_];

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

int inNCCC_EW_ISOModifyPackData(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackData, int *inPackLen)
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
		    memcmp(szCommMode, _COMM_4G_MODE_, 1) == 0		||
		    memcmp(szCommMode, _COMM_WIFI_MODE_, 1) == 0)
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
 Log		:Done
 */
int inNCCC_EW_CommSendRecvToHost(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, int inSendLen, unsigned char *uszRecvPacket)
{
        int		inRetVal;
        int		inSendTimeout = 10;
        int		inReceiveTimeout = 10;
	int		inReceiveSize = _COMM_RECEIVE_MAX_LENGTH_;		/* 配合Ingenico */
	char		szTimeOut[2 + 1] = {0};
        char		szHostResponseTimeOut[2 + 1] = {0};
	unsigned char	uszDispBit = VS_TRUE;
	vdUtility_SYSFIN_LogMessage(AT,"inNCCC_EW_CommSendRecvToHost START");
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_EW_CommSendRecvToHost() START!");
        
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
		vdNCCC_EW_ISO_FormatDebug_DISP(pobTran, &uszSendPacket[0], inSendLen);
	}
	/* 列印ISO Debug */
        if (ginISODebug == VS_TRUE)
	{
                vdNCCC_EW_ISO_FormatDebug_PRINT(pobTran, &uszSendPacket[0], inSendLen);
	}
	
	/* 不顯示訊息 */
	uszDispBit = VS_FALSE;
	
        if ((inRetVal = inCOMM_Send(uszSendPacket, inSendLen, inSendTimeout, uszDispBit)) != VS_SUCCESS)
        {
            vdUtility_SYSFIN_LogMessage(AT,"inCOMM_Send Error");
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inCOMM_Send() ERROR");
                return (VS_ERROR);
        }
	
	/* 這裡是用CPT.DAT的東西，所以一定要用 */
	memset(szHostResponseTimeOut, 0x00, sizeof(szHostResponseTimeOut));
        inRetVal = inGetHostResponseTimeOut(szHostResponseTimeOut);
		
        if (inRetVal != VS_SUCCESS)
				vdUtility_SYSFIN_LogMessage(AT,"inGetHostResponseTimeOut() ERROR");
                return (VS_ERROR);

        inReceiveTimeout = atoi(szHostResponseTimeOut);

	/* 不顯示訊息 */
	uszDispBit = VS_FALSE;
	
        if ((inRetVal = inCOMM_Receive(uszRecvPacket, inReceiveSize, inReceiveTimeout, uszDispBit)) != VS_SUCCESS)
        {
            vdUtility_SYSFIN_LogMessage(AT,"inCOMM_Receive Error");
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inCOMM_Receive() ERROR");

                return (VS_ERROR);
        }

	/* 顯示ISO Debug */
	if (ginDebug == VS_TRUE)
	{
		vdNCCC_EW_ISO_FormatDebug_DISP(pobTran, &uszRecvPacket[0], inRetVal);
	}
        /* 列印ISO Debug */
        if (ginISODebug == VS_TRUE)
	{
                vdNCCC_EW_ISO_FormatDebug_PRINT(pobTran, &uszRecvPacket[0], inRetVal);
	}
	
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_EW_CommSendRecvToHost() END!");
        
        return (VS_SUCCESS);
}

/*
Function        :vdNCCC_EW_ISO_FormatDebug_DISP
Date&Time       :2016/11/30 下午 4:19
Describe        :顯示ISO Debug 
*/
void vdNCCC_EW_ISO_FormatDebug_DISP(TRANSACTION_OBJECT *pobTran, unsigned char *uszDebugBuf, int inSendLen)
{
	int			inOneLineLen = 34;
	int			i, inPrintLineCnt = 0, inCnt = 0, inField, inLen, inFieldLen;
	int			inBitMapCnt1, inBitMapCnt2;
	char			szPrtBuf[50 + 1], szBuf[1536 + 1], szBitMap[8 + 1];
	char			szPrintLineData[36 + 1];
	unsigned char		uszBitMap[9], uszBitMapDisp[66];
	ISO_TYPE_NCCC_EW_TABLE srISOTypeTable;

	memset((char *)&srISOTypeTable, 0x00, sizeof(srISOTypeTable));
	memcpy((char *)&srISOTypeTable, (char *)&srNCCC_EW_ISOFunc[0], sizeof(srISOTypeTable));

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
		if (!inNCCC_EW_BitMapCheck((unsigned char *)szBitMap, i))
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
		
		inField = inNCCC_EW_GetFieldIndex(i, srISOTypeTable.srISOFieldType);
		inFieldLen = inNCCC_EW_GetFieldLen(i, &uszDebugBuf[inCnt], srISOTypeTable.srISOFieldType);

		switch (srISOTypeTable.srISOFieldType[inField].inFieldType)
		{
			case _NCCC_EW_ISO_ASC_  :
				memcpy(&szBuf[inLen], &uszDebugBuf[inCnt], inFieldLen);
				break;
                        case _NCCC_EW_ISO_BYTE_3_  :
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
                        case _NCCC_EW_ISO_BYTE_1_ :
                                inFunc_BCD_to_ASCII(&szBuf[inLen], &uszDebugBuf[inCnt ++], 1);
				inLen ++;
				strcat(szBuf, " ");
				inLen ++;

				inFieldLen --;
                                inFunc_BCD_to_ASCII(&szBuf[inLen], &uszDebugBuf[inCnt], inFieldLen);
				break;
			case _NCCC_EW_ISO_NIBBLE_2_  :
			case _NCCC_EW_ISO_BYTE_2_  :
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
			case _NCCC_EW_ISO_BCD_  :
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

		if (i == 59)
			vdNCCC_EW_ISO_FormatDebug_DISP_59(&uszDebugBuf[inCnt], inFieldLen);
                
		inCnt += inFieldLen;
	}

        inLogPrintf(AT, "==========================================");
}

void vdNCCC_EW_ISO_FormatDebug_DISP_59(unsigned char *uszDebugBuf, int inFieldLen)
{
	int	inCnt = 0, inTableLen = 0;
	int	inPrintLineCnt = 0, inOneLineLen = 34;
	int	inTempLen = 0;
	char	szTagData[38 + 1] = {0};
	char	szPrintBuf[1024 + 1] = {0}, szTemplate[1024 + 1] = {0}, szBig5[1024 + 1] = {0};
	char	szEncryptMode[2 + 1] = {0};
	
	while (inFieldLen > inCnt)
	{
		if (!memcmp(&uszDebugBuf[inCnt], "N1", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "N2", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "N3", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "N4", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "N5", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "N6", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "N7", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "N8", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "N9", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "NA", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "NC", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "ND", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "NE", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "NI", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "NQ", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "NF", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "NP", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "NS", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "NT", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "NU", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "NY", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "NZ", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "MP", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "TD", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "TP", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "UN", 2))
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
		else if (!memcmp(&uszDebugBuf[inCnt], "NY", 2))
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
			
			memset(szEncryptMode, 0x00, sizeof(szEncryptMode));
			inGetEncryptMode(szEncryptMode);
			
			if (inTableLen <= 8)
			{
				/* Table Data */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				if (memcmp(szEncryptMode, _NCCC_ENCRYPTION_TSAM_, _NCCC_ENCRYPTION_LEN_) == 0)
				{
					inFunc_BCD_to_ASCII(&szTemplate[0], &uszDebugBuf[inCnt], inTableLen);
				}
				else
				{
					memcpy(&szTemplate[0], &uszDebugBuf[inCnt], inTableLen);
				}
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, " Table Data [%s]", szTemplate);
				inLogPrintf(AT, szPrintBuf);
			}
			else
			{
				/* Table Data */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				if (memcmp(szEncryptMode, _NCCC_ENCRYPTION_TSAM_, _NCCC_ENCRYPTION_LEN_) == 0)
				{
					inFunc_BCD_to_ASCII(&szTemplate[0], &uszDebugBuf[inCnt], inTableLen);
				}
				else
				{
					memcpy(&szTemplate[0], &uszDebugBuf[inCnt], inTableLen);
				}
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
			
			if (memcmp(szEncryptMode, _NCCC_ENCRYPTION_TSAM_, _NCCC_ENCRYPTION_LEN_) == 0)
			{
				inLogPrintf(AT, "            (Hex)");
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
			inLogPrintf(AT, " -----------------------------------------");
			
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
			inLogPrintf(AT, " -----------------------------------------");
			
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
			inLogPrintf(AT, " -----------------------------------------");
			
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
			inLogPrintf(AT, " -----------------------------------------");
			
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
			inLogPrintf(AT, " -----------------------------------------");
			
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
			inLogPrintf(AT, " -----------------------------------------");
			
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
			inLogPrintf(AT, " -----------------------------------------");
			
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
		else if (!memcmp(&uszDebugBuf[inCnt], "WS", 2))
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
			/* 特店訂單編號 (30 Byte) */
			inTempLen = 30;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  特店訂單編號  = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += inTempLen;
			
			/* 一般交易金額 （不含小數位）。右靠左補 0 */
			inTempLen = 10;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  一般交易金額  = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += inTempLen;

			/* 代收交易金額 （不含小數位）。右靠左補 0 */
			inTempLen = 10;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  代收交易金額  = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += inTempLen;

			/* 代售交易金額 （不含小數位）。右靠左補0 */
			inTempLen = 10;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  代售交易金額  = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += inTempLen;

			/* 不可折抵金額 （不含小數位）。右靠左補0 */
			inTempLen = 10;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  不可折抵金額  = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += inTempLen;

			if (inTableLen == 70)
				continue;
			
			/* 錢包業者交易序號 。（左靠右補空白）*/
			inTempLen = 64;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  錢包業者交易序號  = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += inTempLen;

			/* 電子錢包業者代碼 */
			inTempLen = 2;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  電子錢包業者代碼  = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += inTempLen;
			/* 錢包業者特店代號 (預設由 ATS抓主檔回覆 )。（左靠右補空白）*/
			inTempLen = 50;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  錢包業者特店代號  = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += inTempLen;
			/* 消費者支付工具 ATS判斷後回覆給 EDC*/
			inTempLen = 1;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  消費者支付工具  = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += inTempLen;
			/* 錢包業者交易日期時間 。（左靠右補空白）*/
			inTempLen = 24;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  錢包業者交易日期時間  = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += inTempLen;
			/* 電子發票HASH值 。（左靠右補空白）*/
			inTempLen = 50;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  電子發票HASH值  = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += inTempLen;
			/* 折抵金額 （不含小數位）。（右靠左補0、 不適用業者 全帶 0）*/
			inTempLen = 10;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  折抵金額  = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += inTempLen;
			/* 遮罩帳號/卡號 。（左靠右補空白、不適用業者 全 帶空白 space）*/
			inTempLen = 19;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  遮罩帳號/卡號  = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += inTempLen;
			/* 付款交易編號 。（左靠右補空白、不適用業者 全 帶空白 space）*/
			inTempLen = 30;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  付款交易編號  = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += inTempLen;
			/* 重複付款請求 。 Y/N 。（不適用業者 全帶空白 space）*/
			inTempLen = 1;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  重複付款請求  = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += inTempLen;
			/* 聯名卡代碼 。（左靠右補空白、不適用業者 全 帶空白 space）*/
			inTempLen = 11;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  聯名卡代碼  = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += inTempLen;
			/* 門市代碼 。（左靠右補空白、不適用業者 全 帶空白 space）*/
			inTempLen = 24;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  門市代碼  = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += inTempLen;
		}
		else if (!memcmp(&uszDebugBuf[inCnt], "WR", 2))
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
			/* 特店訂單編號 (30 Byte) */
			inTempLen = 30;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  特店訂單編號  = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += inTempLen;

			/* 錢包業者交易序號 。（左靠右補空白）*/
			inTempLen = 64;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  錢包業者交易序號  = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += inTempLen;

			/* 退貨訂單編號 。（左靠右補空白、 不適用業者 全 帶空白 space）*/
			inTempLen = 30;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  退貨訂單編號  = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += inTempLen;

			/* 一般退貨金額 （不含小數位）。（右靠左補0、 不適用業者全帶 0 */
			inTempLen = 10;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  一般退貨金額  = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += inTempLen;

			/* 代收退貨金額 （不含小數位）。（右靠左補0、 不適用業者全帶 0 */
			inTempLen = 10;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  代收退貨金額  = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += inTempLen;

			/* 代售退貨金額 （不含小數位）。（右靠左補0、 不適用業者全帶 0 */
			inTempLen = 10;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  代售退貨金額  = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += inTempLen;

			if (inTableLen == 154)
				continue;
			
			/* 電子錢包業者代碼 */
			inTempLen = 2;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  電子錢包業者代碼  = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += inTempLen;
			/* 錢包業者特店代號 (預設由 ATS抓主檔回覆 )。（左靠右補空白）*/
			inTempLen = 50;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  錢包業者特店代號  = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += inTempLen;
			/* 錢包業者交易日期時間 。（左靠右補空白）*/
			inTempLen = 24;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  錢包業者交易日期時間  = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += inTempLen;
			/* 折抵金額 （不含小數位）。（右靠左補0、 不適用業者 全帶 0）*/
			inTempLen = 10;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  折抵金額  = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += inTempLen;
			/* 門市代碼 。（左靠右補空白、不適用業者 全 帶空白 space）*/
			inTempLen = 24;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  門市代碼  = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += inTempLen;
		}
		else if (!memcmp(&uszDebugBuf[inCnt], "WQ", 2))
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
			/* 特店訂單編號 (30 Byte) */
			inTempLen = 30;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  特店訂單編號  = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += inTempLen;

			if (inTableLen == 30)
				continue;
			
			/* 錢包業者交易序號 。（左靠右補空白）*/
			inTempLen = 64;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  錢包業者交易序號  = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += inTempLen;

			/* 訂單狀態 */
			inTempLen = 2;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  訂單狀態  = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += inTempLen;
			
			/* 錢包業者特店代號 (預設由 ATS抓主檔回覆 )。（左靠右補空白）*/
			inTempLen = 50;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  錢包業者特店代號  = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += inTempLen;
			
			/* 錢包業者交易日期時間 。（左靠右補空白）*/
			inTempLen = 24;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  錢包業者交易日期時間  = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += inTempLen;
			
			/* 消費者支付工具 ATS判斷後回覆給 EDC*/
			inTempLen = 1;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  消費者支付工具  = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += inTempLen;
			
			/* 電子錢包業者代碼 */
			inTempLen = 2;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  電子錢包業者代碼  = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += inTempLen;
			
			/* 折抵金額 （不含小數位）。（右靠左補0、 不適用業者 全帶 0）*/
			inTempLen = 10;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  折抵金額  = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += inTempLen;
			
			/* 電子發票HASH值 。（左靠右補空白）*/
			inTempLen = 50;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  電子發票HASH值  = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += inTempLen;
			
			/* 遮罩帳號/卡號 。（左靠右補空白、不適用業者 全 帶空白 space）*/
			inTempLen = 19;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  遮罩帳號/卡號  = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += inTempLen;
			
			/* 載具類型。（左靠右補空白、不適用業者 全 帶空白 space）*/
			inTempLen = 6;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  載具類型  = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += inTempLen;
			
			/* 聯名卡代碼。（左靠右補空白、不適用業者 全 帶空白 space）*/
			inTempLen = 11;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  聯名卡代碼  = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += inTempLen;
			
			/* 門市代碼 。（左靠右補空白、不適用業者 全 帶空白 space）*/
			inTempLen = 24;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  門市代碼  = %s", szTemplate);
			inLogPrintf(AT, szPrintBuf);
			inCnt += inTempLen;
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
Function        :vdNCCC_EW_ISO_FormatDebug_PRINT
Date&Time       :2016/11/30 下午 4:20
Describe        :列印ISO Debug
*/
void vdNCCC_EW_ISO_FormatDebug_PRINT(TRANSACTION_OBJECT *pobTran, unsigned char *uszDebugBuf, int inSendLen)
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
	ISO_TYPE_NCCC_EW_TABLE srISOTypeTable;

	/* 是否有列印功能 */
	if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
	{
		
	}
	else
	{
		/* 初始化 */
		inPRINT_Buffer_Initial(uszBuffer, _BUFFER_MAX_LINE_, &srFont_Attrib, &srBhandle);

		memset((char *)&srISOTypeTable, 0x00, sizeof(srISOTypeTable));
		memcpy((char *)&srISOTypeTable, (char *)&srNCCC_EW_ISOFunc[0], sizeof(srISOTypeTable));

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
			if (!inNCCC_EW_BitMapCheck((unsigned char *)szBitMap, i))
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

			inField = inNCCC_EW_GetFieldIndex(i, srISOTypeTable.srISOFieldType);
			inFieldLen = inNCCC_EW_GetFieldLen(i, &uszDebugBuf[inCnt], srISOTypeTable.srISOFieldType);

			switch (srISOTypeTable.srISOFieldType[inField].inFieldType)
			{
				case _NCCC_EW_ISO_ASC_  :
					memcpy(&szBuf[inLen], &uszDebugBuf[inCnt], inFieldLen);
					break;
				case _NCCC_EW_ISO_BYTE_3_  :
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
				case _NCCC_EW_ISO_BYTE_1_ :
					inFunc_BCD_to_ASCII(&szBuf[inLen], &uszDebugBuf[inCnt ++], 1);
					inLen ++;
					strcat(szBuf, " ");
					inLen ++;

					inFieldLen --;
					inFunc_BCD_to_ASCII(&szBuf[inLen], &uszDebugBuf[inCnt], inFieldLen);
					break;
				case _NCCC_EW_ISO_NIBBLE_2_  :
				case _NCCC_EW_ISO_BYTE_2_  :
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
				case _NCCC_EW_ISO_BCD_  :
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

			if (i == 59)
				vdNCCC_EW_ISO_FormatDebug_PRINT_59(&uszDebugBuf[inCnt], inFieldLen, uszBuffer, &srFont_Attrib, &srBhandle);

			inCnt += inFieldLen;
		}

		inPRINT_Buffer_PutIn("==========================================", _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		inPRINT_Buffer_OutPut(uszBuffer, &srBhandle);
	}
}

void vdNCCC_EW_ISO_FormatDebug_PRINT_59(unsigned char *uszDebugBuf, int inFieldLen, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inCnt = 0, inTableLen = 0;
	int	inPrintLineCnt = 0, inOneLineLen = 34;
	int	inTempLen = 0;
	char	szTagData[38 + 1] = {0};
	char	szPrintBuf[1024 + 1] = {0}, szTemplate[1024 + 1] = {0}, szBig5[1024 + 1] = {0};
	char	szEncryptMode[2 + 1] = {0};

	while (inFieldLen > inCnt)
	{
		if (!memcmp(&uszDebugBuf[inCnt], "N1", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "N2", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "N3", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "N4", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "N5", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "N6", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "N7", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "N8", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "N9", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "NA", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "NC", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "ND", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "NE", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "NI", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "NQ", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "NF", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "NP", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "NS", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "NT", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "NU", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "NY", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "NZ", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "MP", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "TD", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "TP", 2) ||
		    !memcmp(&uszDebugBuf[inCnt], "UN", 2))
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
		else if (!memcmp(&uszDebugBuf[inCnt], "NY", 2))
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
			
			memset(szEncryptMode, 0x00, sizeof(szEncryptMode));
			inGetEncryptMode(szEncryptMode);
			
			if (inTableLen <= 8)
			{
				/* Table Data */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				if (memcmp(szEncryptMode, _NCCC_ENCRYPTION_TSAM_, _NCCC_ENCRYPTION_LEN_) == 0)
				{
					inFunc_BCD_to_ASCII(&szTemplate[0], &uszDebugBuf[inCnt], inTableLen);
				}
				else
				{
					memcpy(&szTemplate[0], &uszDebugBuf[inCnt], inTableLen);
				}
				
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, " Table Data [%s]", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			else
			{
				/* Table Data */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				if (memcmp(szEncryptMode, _NCCC_ENCRYPTION_TSAM_, _NCCC_ENCRYPTION_LEN_) == 0)
				{
					inFunc_BCD_to_ASCII(&szTemplate[0], &uszDebugBuf[inCnt], inTableLen);
				}
				else
				{
					memcpy(&szTemplate[0], &uszDebugBuf[inCnt], inTableLen);
				}
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
			
			if (memcmp(szEncryptMode, _NCCC_ENCRYPTION_TSAM_, _NCCC_ENCRYPTION_LEN_) == 0)
			{
				inPRINT_Buffer_PutIn("            (Hex)", _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
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
			inPRINT_Buffer_PutIn(" -----------------------------------------", _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			
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
			inPRINT_Buffer_PutIn(" -----------------------------------------", _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			
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
			inPRINT_Buffer_PutIn(" -----------------------------------------", _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			
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
			inPRINT_Buffer_PutIn(" -----------------------------------------", _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			
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
			inPRINT_Buffer_PutIn(" -----------------------------------------", _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			
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
			inPRINT_Buffer_PutIn(" -----------------------------------------", _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			
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
			inPRINT_Buffer_PutIn(" -----------------------------------------", _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			
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
		else if (!memcmp(&uszDebugBuf[inCnt], "WS", 2))
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
			/* 特店訂單編號 (30 Byte) */
			inTempLen = 30;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  特店訂單編號  = %s", szTemplate);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += inTempLen;
			
			/* 一般交易金額 （不含小數位）。右靠左補 0 */
			inTempLen = 10;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  一般交易金額  = %s", szTemplate);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += inTempLen;

			/* 代收交易金額 （不含小數位）。右靠左補 0 */
			inTempLen = 10;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  代收交易金額  = %s", szTemplate);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += inTempLen;

			/* 代售交易金額 （不含小數位）。右靠左補0 */
			inTempLen = 10;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  代售交易金額  = %s", szTemplate);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += inTempLen;

			/* 不可折抵金額 （不含小數位）。右靠左補0 */
			inTempLen = 10;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  不可折抵金額  = %s", szTemplate);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += inTempLen;

			if (inTableLen == 70)
				continue;
			
			/* 錢包業者交易序號 。（左靠右補空白）*/
			inTempLen = 64;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  錢包業者交易序號  = %s", szTemplate);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += inTempLen;

			/* 電子錢包業者代碼 */
			inTempLen = 2;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  電子錢包業者代碼  = %s", szTemplate);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += inTempLen;
			/* 錢包業者特店代號 (預設由 ATS抓主檔回覆 )。（左靠右補空白）*/
			inTempLen = 50;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  錢包業者特店代號  = %s", szTemplate);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += inTempLen;
			/* 消費者支付工具 ATS判斷後回覆給 EDC*/
			inTempLen = 1;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  消費者支付工具  = %s", szTemplate);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += inTempLen;
			/* 錢包業者交易日期時間 。（左靠右補空白）*/
			inTempLen = 24;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  錢包業者交易日期時間  = %s", szTemplate);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += inTempLen;
			/* 電子發票HASH值 。（左靠右補空白）*/
			inTempLen = 50;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  電子發票HASH值  = %s", szTemplate);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += inTempLen;
			/* 折抵金額 （不含小數位）。（右靠左補0、 不適用業者 全帶 0）*/
			inTempLen = 10;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  折抵金額  = %s", szTemplate);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += inTempLen;
			/* 遮罩帳號/卡號 。（左靠右補空白、不適用業者 全 帶空白 space）*/
			inTempLen = 19;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  遮罩帳號/卡號  = %s", szTemplate);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += inTempLen;
			/* 付款交易編號 。（左靠右補空白、不適用業者 全 帶空白 space）*/
			inTempLen = 30;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  付款交易編號  = %s", szTemplate);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += inTempLen;
			/* 重複付款請求 。 Y/N 。（不適用業者 全帶空白 space）*/
			inTempLen = 1;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  重複付款請求  = %s", szTemplate);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += inTempLen;
			/* 聯名卡代碼 。（左靠右補空白、不適用業者 全 帶空白 space）*/
			inTempLen = 11;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  聯名卡代碼  = %s", szTemplate);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += inTempLen;
			/* 門市代碼 。（左靠右補空白、不適用業者 全 帶空白 space）*/
			inTempLen = 24;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  門市代碼  = %s", szTemplate);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += inTempLen;
		}
		else if (!memcmp(&uszDebugBuf[inCnt], "WR", 2))
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
			/* 特店訂單編號 (30 Byte) */
			inTempLen = 30;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  特店訂單編號  = %s", szTemplate);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += inTempLen;

			/* 錢包業者交易序號 。（左靠右補空白）*/
			inTempLen = 64;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  錢包業者交易序號  = %s", szTemplate);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += inTempLen;

			/* 退貨訂單編號 。（左靠右補空白、 不適用業者 全 帶空白 space）*/
			inTempLen = 30;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  退貨訂單編號  = %s", szTemplate);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += inTempLen;

			/* 一般退貨金額 （不含小數位）。（右靠左補0、 不適用業者全帶 0 */
			inTempLen = 10;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  一般退貨金額  = %s", szTemplate);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += inTempLen;

			/* 代收退貨金額 （不含小數位）。（右靠左補0、 不適用業者全帶 0 */
			inTempLen = 10;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  代收退貨金額  = %s", szTemplate);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += inTempLen;

			/* 代售退貨金額 （不含小數位）。（右靠左補0、 不適用業者全帶 0 */
			inTempLen = 10;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  代售退貨金額  = %s", szTemplate);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += inTempLen;

			if (inTableLen == 154)
				continue;
			
			/* 電子錢包業者代碼 */
			inTempLen = 2;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  電子錢包業者代碼  = %s", szTemplate);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += inTempLen;
			/* 錢包業者特店代號 (預設由 ATS抓主檔回覆 )。（左靠右補空白）*/
			inTempLen = 50;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  錢包業者特店代號  = %s", szTemplate);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += inTempLen;
			/* 錢包業者交易日期時間 。（左靠右補空白）*/
			inTempLen = 24;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  錢包業者交易日期時間  = %s", szTemplate);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += inTempLen;
			/* 折抵金額 （不含小數位）。（右靠左補0、 不適用業者 全帶 0）*/
			inTempLen = 10;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  折抵金額  = %s", szTemplate);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += inTempLen;
			/* 門市代碼 。（左靠右補空白、不適用業者 全 帶空白 space）*/
			inTempLen = 24;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  門市代碼  = %s", szTemplate);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += inTempLen;
		}
		else if (!memcmp(&uszDebugBuf[inCnt], "WQ", 2))
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
			/* 特店訂單編號 (30 Byte) */
			inTempLen = 30;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  特店訂單編號  = %s", szTemplate);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += inTempLen;

			if (inTableLen == 30)
				continue;
			
			/* 錢包業者交易序號 。（左靠右補空白）*/
			inTempLen = 64;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  錢包業者交易序號  = %s", szTemplate);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += inTempLen;

			/* 訂單狀態 */
			inTempLen = 2;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  訂單狀態  = %s", szTemplate);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += inTempLen;
			
			/* 錢包業者特店代號 (預設由 ATS抓主檔回覆 )。（左靠右補空白）*/
			inTempLen = 50;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  錢包業者特店代號  = %s", szTemplate);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += inTempLen;
			
			/* 錢包業者交易日期時間 。（左靠右補空白）*/
			inTempLen = 24;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  錢包業者交易日期時間  = %s", szTemplate);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += inTempLen;
			
			/* 消費者支付工具 ATS判斷後回覆給 EDC*/
			inTempLen = 1;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  消費者支付工具  = %s", szTemplate);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += inTempLen;
			
			/* 電子錢包業者代碼 */
			inTempLen = 2;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  電子錢包業者代碼  = %s", szTemplate);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += inTempLen;
			
			/* 折抵金額 （不含小數位）。（右靠左補0、 不適用業者 全帶 0）*/
			inTempLen = 10;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  折抵金額  = %s", szTemplate);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += inTempLen;
			
			/* 電子發票HASH值 。（左靠右補空白）*/
			inTempLen = 50;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  電子發票HASH值  = %s", szTemplate);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += inTempLen;
			
			/* 遮罩帳號/卡號 。（左靠右補空白、不適用業者 全 帶空白 space）*/
			inTempLen = 19;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  遮罩帳號/卡號  = %s", szTemplate);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += inTempLen;
			
			/* 載具類型。（左靠右補空白、不適用業者 全 帶空白 space）*/
			inTempLen = 6;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  載具類型  = %s", szTemplate);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += inTempLen;
			
			/* 聯名卡代碼。（左靠右補空白、不適用業者 全 帶空白 space）*/
			inTempLen = 11;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  聯名卡代碼  = %s", szTemplate);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += inTempLen;
			
			/* 門市代碼 。（左靠右補空白、不適用業者 全 帶空白 space）*/
			inTempLen = 24;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszDebugBuf[inCnt], inTempLen);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "  門市代碼  = %s", szTemplate);
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inCnt += inTempLen;
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

int inNCCC_EW_BitMapCheck(unsigned char *inBitMap, int inFeild)
{
        int 	inByteIndex, inBitIndex;

        inFeild--;
        inByteIndex = inFeild / 8;
        inBitIndex = 7 - (inFeild - inByteIndex * 8);

        if (_NCCC_EW_CHECK_BIT_MAP_(inBitMap[inByteIndex], inBitIndex))
        {
                return (VS_TRUE);
        }
        return (VS_FALSE);
}

int inNCCC_EW_GetFieldLen(int inField, unsigned char *uszSendData, ISO_FIELD_TYPE_NCCC_EW_TABLE *srFieldType)
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
                        case _NCCC_EW_ISO_ASC_:
                                inCnt += srFieldType[i].inFieldLen;
                                break;
                        case _NCCC_EW_ISO_BCD_:
                                inCnt += srFieldType[i].inFieldLen / 2;
                                break;
                        case _NCCC_EW_ISO_NIBBLE_2_:
                                inLen = (uszSendData[0] / 16 * 10) + (uszSendData[0] % 16);
				inCnt += inLen + 1;
                                break;
                        case _NCCC_EW_ISO_NIBBLE_3_:
                                inLen = (uszSendData[0] / 16 * 1000) + (uszSendData[0] % 16 * 100) + (uszSendData[1] / 16 * 10) + (uszSendData[1] % 16);
                                inCnt += ((inLen + 1) / 2) + 2;
                                break;
                        case _NCCC_EW_ISO_BYTE_2_:
                                inLen = (uszSendData[0] / 16 * 10) + (uszSendData[0] % 16);
                                inCnt += inLen + 1;
                                break;
                        case _NCCC_EW_ISO_BYTE_3_:
                                inLen = (uszSendData[0] / 16 * 1000) + (uszSendData[0] % 16 * 100) + (uszSendData[1] / 16 * 10) + (uszSendData[1] % 16);
                                inCnt += inLen + 2;
                                break;
                        case _NCCC_EW_ISO_BYTE_2_H_:
                                inLen = (int) uszSendData[0];
                                inCnt += inLen + 1;
                                break;
                        case _NCCC_EW_ISO_BYTE_3_H_:
                                inLen = ((int) uszSendData[0] * 0xFF) + (int) uszSendData[1];
                                inCnt += inLen + 1;
                                break;
			case _NCCC_EW_ISO_BYTE_1_:
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

int inNCCC_EW_GetFieldIndex(int inField, ISO_FIELD_TYPE_NCCC_EW_TABLE *srFieldType)
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

int inNCCC_EW_UnPackISO(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendBuf, unsigned char *uszRecvBuf)
{
	int				inRetVal;
        int				i, inRecvCnt, inCnt, inSendCnt, inSendField, inRecvField, inISOFuncIndex = -1;
        char				szBuf[_NCCC_EW_TPDU_SIZE_ + _NCCC_EW_MTI_SIZE_ + _NCCC_EW_BIT_MAP_SIZE_ + 1];
        char				szErrorMessage[40 + 1];
	char				szCommMode[1 + 1];
	char				szEncryptMode[1 +1];
        unsigned char			uszSendMap[_NCCC_EW_BIT_MAP_SIZE_ + 1], uszReceMap[_NCCC_EW_BIT_MAP_SIZE_ + 1];
        ISO_TYPE_NCCC_EW_TABLE 	srISOFunc;
	vdUtility_SYSFIN_LogMessage(AT,"inNCCC_EW_UnPackISO START");

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_EW_UnPackISO() START!");

        inSendField = inRecvField = 0;
        inSendCnt = inRecvCnt = 0;

        memset((char *) uszSendMap, 0x00, sizeof(uszSendMap));
        memset((char *) uszReceMap, 0x00, sizeof(uszReceMap));
        memset((char *) szBuf, 0x00, sizeof(szBuf));
	
	/* 加密模式，預設值 = 0。【0 = 不加密，1 = tSAM 加密】 */
	memset(szCommMode, 0x00, sizeof(szCommMode));
	inGetCommMode(szCommMode);
	if (memcmp(szCommMode, _COMM_MODEM_MODE_, strlen(_COMM_MODEM_MODE_)) == 0)
		inISOFuncIndex = 0; /* 不加密 */
	else if (memcmp(szCommMode, _COMM_ETHERNET_MODE_, strlen(_COMM_ETHERNET_MODE_)) == 0)
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
	else if (memcmp(szCommMode, _COMM_WIFI_MODE_, strlen(_COMM_WIFI_MODE_)) == 0)
	{
		memset(szEncryptMode, 0x00, sizeof(szEncryptMode));
		inGetEncryptMode(szEncryptMode);
		inISOFuncIndex = atoi(szEncryptMode);
	}
	else
	{
		vdUtility_SYSFIN_LogMessage(AT,"No such CommMode! szCommMode = %d", atoi(szCommMode));
		if (ginDebug == VS_TRUE)
		{
			memset(szErrorMessage, 0x00, sizeof(szErrorMessage));
			sprintf(szErrorMessage,"No such CommMode! szCommMode = %d", atoi(szCommMode));
			inLogPrintf(AT, szErrorMessage);
		}
		return (VS_ERROR); /* 防呆 */
	}
	
	/* 新的〈EW〉不支援軟加密 */
	if (inISOFuncIndex >= 2 || inISOFuncIndex < 0)
	{
		vdUtility_SYSFIN_LogMessage(AT,"inISOFuncIndex : %d is out of range",inISOFuncIndex);
                if (ginDebug == VS_TRUE)
                {
                        memset(szErrorMessage, 0x00, sizeof(szErrorMessage));
                        sprintf(szErrorMessage, "inISOFuncIndex : %d", inISOFuncIndex);
                        inLogPrintf(AT, szErrorMessage);
                }
                
		return (VS_ERROR); /* 防呆 */
	}
	
        memset((char *) &srISOFunc, 0x00, sizeof(srISOFunc));
        memcpy((char *) &srISOFunc, (char *) &srNCCC_EW_ISOFunc[inISOFuncIndex], sizeof(srISOFunc));

        if (srISOFunc.inCheckISOHeader != NULL)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "srISOFunc.inCheckISOHeader != NULL");

                if (srISOFunc.inCheckISOHeader(pobTran, (char *) &uszSendBuf[inRecvCnt], (char *) &uszRecvBuf[inRecvCnt]) != VS_SUCCESS)
                {
					vdUtility_SYSFIN_LogMessage(AT,"inNCCC_EW_UnPackISO srISOFunc.inCheckISOHeader ERROR");
                        if (ginDebug == VS_TRUE)
                                inLogPrintf(AT, "srISOFunc.inCheckISOHeader ERROR");

                        return (_TRAN_RESULT_UNPACK_ERR_);
                }
        }

        inSendCnt += _NCCC_EW_TPDU_SIZE_;
        inRecvCnt += _NCCC_EW_TPDU_SIZE_;
        inSendCnt += _NCCC_EW_MTI_SIZE_;
        inRecvCnt += _NCCC_EW_MTI_SIZE_;

        memcpy((char *) uszSendMap, (char *) &uszSendBuf[inSendCnt], _NCCC_EW_BIT_MAP_SIZE_);
        memcpy((char *) uszReceMap, (char *) &uszRecvBuf[inRecvCnt], _NCCC_EW_BIT_MAP_SIZE_);

        inSendCnt += _NCCC_EW_BIT_MAP_SIZE_;
        inRecvCnt += _NCCC_EW_BIT_MAP_SIZE_;

        /* 先檢查 ISO Field_39 */
        if (inNCCC_EW_BitMapCheck(uszReceMap, 39) == VS_FALSE)
        {
            vdUtility_SYSFIN_LogMessage(AT,"inNCCC_ATS_BitMapCheck(39) ERROR");
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inNCCC_EW_BitMapCheck(39) ERROR");

                return (VS_ERROR);
        }

        if (inNCCC_EW_BitMapCheck(uszReceMap, 41) == VS_FALSE)
        {
            vdUtility_SYSFIN_LogMessage(AT,"inNCCC_ATS_BitMapCheck(41) ERROR");
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inNCCC_EW_BitMapCheck(41) ERROR");

                return (VS_ERROR);
        }

        for (i = 1; i <= 64; i++)
        {
                /* 有送出去的 Field 但沒有收回來的 Field */
                if (inNCCC_EW_BitMapCheck(uszSendMap, i) && !inNCCC_EW_BitMapCheck(uszReceMap, i))
                {
                        inSendCnt += inNCCC_EW_GetFieldLen(i, &uszSendBuf[inSendCnt], srISOFunc.srISOFieldType);
                }
                else if (inNCCC_EW_BitMapCheck(uszReceMap, i)) /* 收的 BitMap */
                {
                        if (inNCCC_EW_BitMapCheck(uszSendMap, i)) /* 送的 BitMap 都有 */
                        {
                                /* 是否要進行檢查封包資料包含【送】【收】 */
                                if ((inSendField = inNCCC_EW_GetCheckField(i, srISOFunc.srCheckISO)) != VS_ERROR)
                                {
                                        if (srISOFunc.srCheckISO[inSendField].inISOCheck(pobTran, &uszSendBuf[inSendCnt], &uszRecvBuf[inRecvCnt]) != VS_SUCCESS)
                                        {
											vdUtility_SYSFIN_LogMessage(AT,"Field_%d : Send/Recv data are not the same",srISOFunc.srCheckISO[inSendField].inFieldNum);
                                                if (ginDebug == VS_TRUE)
                                                {
                                                        memset(szErrorMessage, 0x00, sizeof(szErrorMessage));
                                                        sprintf(szErrorMessage, "inSendField = %d Error!", srISOFunc.srCheckISO[inSendField].inFieldNum);
                                                        inLogPrintf(AT, szErrorMessage);
                                                }

                                                return (_TRAN_RESULT_UNPACK_ERR_);
                                        }
                                }

                                inSendCnt += inNCCC_EW_GetFieldLen(i, &uszSendBuf[inSendCnt], srISOFunc.srISOFieldType);
                        }

                        while (i > srISOFunc.srUnPackISO[inRecvField].inFieldNum) /* 只有收 */
                        {
                                inRecvField++;
                        }

                        if (i == srISOFunc.srUnPackISO[inRecvField].inFieldNum)
                        {
                                /* 要先檢查 UnpackISO 是否存在 */
                                if (inNCCC_EW_CheckUnPackField(i, srISOFunc.srUnPackISO) == VS_SUCCESS)
				{
                                        inRetVal = srISOFunc.srUnPackISO[inRecvField].inISOLoad(pobTran, &uszRecvBuf[inRecvCnt]);
					
					if (inRetVal != VS_SUCCESS)
					{
						vdUtility_SYSFIN_LogMessage(AT,"inRecvField:%d UnPackISO Error!",srISOFunc.srUnPackISO[inRecvField].inFieldNum);
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
			
                        inCnt = inNCCC_EW_GetFieldLen(i, &uszRecvBuf[inRecvCnt], srISOFunc.srISOFieldType);
                        /* inNCCC_EW_GetFieldLen 返回值會>=0 ?*/
					/*
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
					*/
                        inRecvCnt += inCnt;
                }
        }

        /* 這裡表示已經解完電文要檢查是否有回 ISO Field_38 */
        if (!memcmp(&pobTran->srBRec.szRespCode[0], "00", 2))
        {
                switch (pobTran->inISOTxnCode)
                {
                        case _EW_SALE_:
                        case _EW_REFUND_:
				if (inNCCC_EW_BitMapCheck(uszReceMap, 04) == VS_FALSE)
                                {
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "inNCCC_EW_BitMapCheck 04 Error!");
                                        }

                                        return (VS_ERROR);
                                }
				
                                if (inNCCC_EW_BitMapCheck(uszReceMap, 38) == VS_FALSE)
                                {
                                    vdUtility_SYSFIN_LogMessage(AT,"inNCCC_ATS_BitMapCheck(38) ERROR");
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "inNCCC_EW_BitMapCheck 38 Error!");
                                        }

                                        return (VS_ERROR);
                                }

                                break;
                        default:
                                break;
                }
        }

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_EW_UnPackISO() END!");

        return (VS_SUCCESS);
}

int inNCCC_EW_GetCheckField(int inField, ISO_CHECK_NCCC_EW_TABLE *ISOFieldCheck)
{
        int 	i;

        for (i = 0; i < 64; i++)
        {
                if (ISOFieldCheck[i].inFieldNum == 0)
                        break;
                else if (ISOFieldCheck[i].inFieldNum == inField)
                {
                        return (i); /* i 是 ISO_CHECK_NCCC_EW_TABLE 裡的第幾個 */
                }
        }

        return (VS_ERROR);
}

int inNCCC_EW_CheckUnPackField(int inField, ISO_FIELD_NCCC_EW_TABLE *srCheckUnPackField)
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
Function        :inNCCC_EW_ISOCheckHeader
Date&Time       :2016/9/14 上午 10:17
Describe        :
*/
int inNCCC_EW_ISOCheckHeader(TRANSACTION_OBJECT *pobTran, char *szSendISOHeader, char *szReceISOHeader)
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
Function        :inNCCC_EW_CheckRespCode
Date&Time       :2016/9/14 上午 9:48
Describe        :
*/
int inNCCC_EW_CheckRespCode(TRANSACTION_OBJECT *pobTran)
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
		pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_CALLBANK_;
		inRetVal = _TRAN_RESULT_CANCELLED_;
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
		ginMacError ++;
		inRetVal = _TRAN_RESULT_CANCELLED_;
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
	else if (!memcmp(&pobTran->srBRec.szRespCode[0], "W1", 2))
	{
		pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_FORMAT_CHECK_ERROR_;
		inRetVal = _TRAN_RESULT_CANCELLED_;
	}
	else if (!memcmp(&pobTran->srBRec.szRespCode[0], "W2", 2))
	{
		pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_EW_INSUFFICIENT_BALANCE_;
		 inRetVal = _TRAN_RESULT_CANCELLED_;
	}
	else if (!memcmp(&pobTran->srBRec.szRespCode[0], "W3", 2))
	{
		pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_EW_HOST_TIMEOUT_;
		 inRetVal = _TRAN_RESULT_CANCELLED_;
	}
        else
        {
                if ((pobTran->srBRec.szRespCode[0] >= '0' && pobTran->srBRec.szRespCode[0] <= '9') && (pobTran->srBRec.szRespCode[1] >= '0' && pobTran->srBRec.szRespCode[1] <= '9'))
                {
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
Function        :inNCCC_EW_CheckAuthCode
Date&Time       :2016/9/14 上午 9:48
Describe        :
*/
int inNCCC_EW_CheckAuthCode(TRANSACTION_OBJECT *pobTran)
{
        int 	inRetVal = VS_SUCCESS;
	
        switch (pobTran->inISOTxnCode)
        {
                default:
                        break;
        }

        return (inRetVal);
}

/*
Function        :inNCCC_EW_AnalysePacket
Date&Time       :2016/9/14 上午 10:03
Describe        :
*/
int inNCCC_EW_AnalysePacket(TRANSACTION_OBJECT *pobTran)
{
        int				inRetVal = VS_SUCCESS, inISOFuncIndex = -1;
        ISO_TYPE_NCCC_EW_TABLE 	srISOFunc;

        if (pobTran->srBRec.uszOfflineBit == VS_FALSE)
        {
		/* 加密模式，預設值 = 0。【0 = 不加密，1 = tSAM 加密，2 = 軟體加密】 */
		inISOFuncIndex = 0; /* 不加密 */
		memset((char *) &srISOFunc, 0x00, sizeof(srISOFunc));
		memcpy((char *) &srISOFunc, (char *) &srNCCC_EW_ISOFunc[inISOFuncIndex], sizeof(srISOFunc));

		if (srISOFunc.inOnAnalyse != NULL)
			inRetVal = srISOFunc.inOnAnalyse(pobTran);
		else
			inRetVal = VS_ERROR;
        }

        return (inRetVal);
}

/*
Function        :inNCCC_EW_ISOOnlineAnalyse
Date&Time       :2016/9/14 上午 10:17
Describe        :
*/
int inNCCC_EW_ISOOnlineAnalyse(TRANSACTION_OBJECT *pobTran)
{
        int	inRetVal = VS_ERROR;

	inRetVal = inNCCC_EW_OnlineAnalyseNormal(pobTran);
	
	if (inRetVal != VS_SUCCESS)
	{
		inUtility_StoreTraceLog_OneStep("Analyse Failed");
		inUtility_StoreTraceLog_OneStep("Response Code:%s", pobTran->srBRec.szRespCode);
		inUtility_StoreTraceLog_OneStep("Auth Code:%s", pobTran->srBRec.szAuthCode);
	}

        return (inRetVal);
}

/*
Function        :inNCCC_EW_OnlineAnalyseNormal
Date&Time       :2023/8/11 下午 6:13
Describe        :
*/
int inNCCC_EW_OnlineAnalyseNormal(TRANSACTION_OBJECT *pobTran)
{
        int		inRetVal = VS_SUCCESS;
	char		szCustomerIndicator[3 + 1] = {0};
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);

	if (pobTran->inTransactionResult == _TRAN_RESULT_AUTHORIZED_)
	{
		if (pobTran->srBRec.uszOfflineBit == VS_FALSE)
		{
			inSetSendReversalBit("N");
			if (inSaveHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
			{
				return (VS_ERROR);
			}

			inRetVal = VS_SUCCESS;
		}
	}
	else if (pobTran->inTransactionResult == _TRAN_RESULT_CANCELLED_)
	{
		inNCCC_EW_DispHostResponseCode(pobTran);		/* 顯示主機回的錯誤訊息 */

		inRetVal = VS_ERROR;
	}
	else
	{
		inNCCC_EW_DispHostResponseCode(pobTran);		/* 顯示主機回的錯誤訊息 */

		inRetVal = VS_ERROR;
	}

        return (inRetVal);
}

/*
Function        :inNCCC_EW_SyncHostTerminalDateTime
Date&Time       :2023/8/11 下午 5:50
Describe        :
*/
int inNCCC_EW_SyncHostTerminalDateTime(TRANSACTION_OBJECT *pobTran)
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
Function        :inNCCC_EW_DispHostResponseCode
Date&Time       :2023/8/11 下午 6:08
Describe        :顯示錯誤代碼
*/
int inNCCC_EW_DispHostResponseCode(TRANSACTION_OBJECT *pobTran)
{
	int		inChoice = _DisTouch_No_Event_;
	int		inRetVal = VS_SUCCESS;
	int		inDisplayFormatMode = 1;
	int		inEWTimeout = 0;
	char		szResponseCode[10 + 1] = {0};
	char		szMsg[42 + 1] = {0};
	char		szMsg2[42 + 1] = {0};
	char		szCustomerIndicator[3 + 1] = {0};
        unsigned char   uszKey = 0x00;
	
	/* 電子錢包所有的交易被主機拒絕，都一律2秒後回到待機畫面 */
	inEWTimeout = 2;
	
	memset(szMsg, 0x00, sizeof(szMsg));
	memset(szMsg2, 0x00, sizeof(szMsg2));
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	memset(szResponseCode, 0x00, sizeof(szResponseCode));	
	sprintf(szResponseCode, "%s", pobTran->srBRec.szRespCode);		/* 錯誤代碼 */
	
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
			sprintf(szMsg, "%s", "請聯絡發卡銀行");
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
		else if (!memcmp(&pobTran->srBRec.szRespCode[0], "W1", 2))	/* (需求單-111155)-電子錢包業者收付訊息整合平台需求 by Russell 2023/7/24 下午 6:17 */
		{
			inDisplayFormatMode = 3;
			sprintf(szMsg, "%s", "W1錢包交易");
			sprintf(szMsg2, "%s", "電文格式錯誤");
		}
		else if (!memcmp(&pobTran->srBRec.szRespCode[0], "W2", 2))	/* (需求單-111155)-電子錢包業者收付訊息整合平台需求 by Russell 2023/7/24 下午 6:17 */
		{
			inDisplayFormatMode = 3;
			sprintf(szMsg, "%s", "W2錢包交易");
			sprintf(szMsg2, "%s", "餘額不足");
		}
		else if (!memcmp(&pobTran->srBRec.szRespCode[0], "W3", 2))	/* (需求單-111155)-電子錢包業者收付訊息整合平台需求 by Russell 2023/7/24 下午 6:17 */
		{
			inDisplayFormatMode = 3;
			sprintf(szMsg, "%s", "W3錢包交易逾時");
			sprintf(szMsg2, "%s", "請發動查詢");
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
	
	/* 【需求單-111190】行動支付交易顯示畫面調整V3排除以下客製化外須開發(下列客製化保持原規則) :
	001、006、010、011、013、019、020、024、028、031、032、036、040~044、048、060、065、068、073、077、078、081、084、087、089、092~094、100、104~107、109、111~119
	 * 因需求顯示回應碼
	*/
	if (memcmp(szMsg, "請聯絡發卡銀行", strlen("請聯絡發卡銀行")) == 0)
	{
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_001_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_006_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_010_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_011_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_013_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_019_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_020_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_024_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_028_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_031_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_032_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_036_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_040_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_041_CASH_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_042_BDAU1_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_043_BDAU9_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_044_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_048_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_060_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_065_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_068_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_073_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_077_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_078_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_081_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_084_ON_US_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_087_ON_US_NO_ID_, _CUSTOMER_INDICATOR_SIZE_)	||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_089_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_092_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_093_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_094_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_100_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_104_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)	||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_106_ON_US_BCA_, _CUSTOMER_INDICATOR_SIZE_)	||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)	||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_109_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_)||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_112_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_113_H_AND_M_, _CUSTOMER_INDICATOR_SIZE_)	||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_114_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_115_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_116_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_117_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_118_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_119_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_))
		{
			/* 【需求單 - 104014】發卡機構on-us繳費平台需求 +【需求單 - 104152】發卡機構on-us繳費平台 Payitem Code */
			/* 	on-us交易不支援Call bank交易，提示「XX 拒絕交易」。 */
			if ((!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_084_ON_US_, _CUSTOMER_INDICATOR_SIZE_)		||
				!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_087_ON_US_NO_ID_, _CUSTOMER_INDICATOR_SIZE_)	||
				!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_106_ON_US_BCA_, _CUSTOMER_INDICATOR_SIZE_))	&&
				pobTran->inTransactionCode == _EW_SALE_)
			{
				sprintf(szMsg, "%s", "拒絕交易");	/* 拒絕交易 */
			}
			/* (需求單-110202)-修改客製化參數111卡人自助EDC UI訊息需求 by Russell 2021/12/23 下午 1:57 */
			/* 這邊直接照520，01、02直接拒絕 */
			else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_)	||
				 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_))
			{
				sprintf(szMsg, "%s", "拒絕交易");	/* 拒絕交易 */
			}
			else
			{
				/* 回應碼01或02才需要修改顯示訊息和顯示回應碼 */
				if (!memcmp(&pobTran->srBRec.szRespCode[0], "01", 2) ||
				    !memcmp(&pobTran->srBRec.szRespCode[0], "02", 2))
				{
					/* 	DISCOVER交易不支援Call bank交易，提示「XX 拒絕交易」。 */
					/* 要排除TWIN卡的狀況 */
					if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)) == 0 &&
						 pobTran->srBRec.uszUCARDTransBit != VS_TRUE)
					{
						sprintf(szMsg, "%s", "拒絕交易");	/* 拒絕交易 */
						sprintf(szResponseCode, "%s", "05");
					}
				}
				/* 回應碼非01或02(ex:61 or 98)顯示原訊息和顯示原回應碼 */
				else
				{
					
				}
					
			}
		}
		else
		{
			/* 比照123拒絕交易 */
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
			{
				sprintf(szMsg, "%s", "拒絕交易");	/* 拒絕交易 */
			}
			else
			{
				/* 回應碼01或02才需要修改顯示訊息和顯示回應碼 */
				if (!memcmp(&pobTran->srBRec.szRespCode[0], "01", 2) ||
				    !memcmp(&pobTran->srBRec.szRespCode[0], "02", 2))
				{
					/* 	DISCOVER交易不支援Call bank交易，提示「XX 拒絕交易」。 */
					/* 要排除TWIN卡的狀況 */
					if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)) == 0 &&
						 pobTran->srBRec.uszUCARDTransBit != VS_TRUE)
					{
						sprintf(szMsg, "%s", "拒絕交易");	/* 拒絕交易 */
						sprintf(szResponseCode, "%s", "05");
					}
				}
				else
				{

				}
			}
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
			srDispMsgObj.inTimeout = inEWTimeout;
		}
		
		if (inDisplayFormatMode == 2)
		{
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
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
		else if (inDisplayFormatMode == 3)
		{
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			strcpy(srDispMsgObj.szErrMsg1, "");
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
		else if (inDisplayFormatMode == 3)
		{
			/* 清下排 */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);

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

		inDISP_Timer_Start(_TIMER_NEXSYS_1_, inEWTimeout);
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
		else if (inDisplayFormatMode == 3)
		{
			/* 清下排 */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);

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

		inDISP_Timer_Start(_TIMER_NEXSYS_1_, inEWTimeout);
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
Function        :inNCCC_EW_Pack03
Date&Time       :
Describe        :Field_3:       Processing Code
*/
int inNCCC_EW_Pack03(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 	inCnt = 0;
	char	szDebugMsg[100 + 1];
	char	szAscii[6 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_EW_Pack03() START!");
	
	
	memcpy(&uszPackBuf[inCnt], &guszNCCC_EW_ISO_Field03[0], _NCCC_EW_PCODE_SIZE_);
	
        inCnt += _NCCC_EW_PCODE_SIZE_;
	
	/* for Mac */
	inFunc_BCD_to_ASCII(&gszMAC_F_03[0], &guszNCCC_EW_ISO_Field03[0], 1);

	if (ginDebug == VS_TRUE)
	{
		memset(szAscii, 0x00, sizeof(szAscii));
		inFunc_BCD_to_ASCII(szAscii, guszNCCC_EW_ISO_Field03, _NCCC_EW_PCODE_SIZE_);
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "F_03 [P_CODE %s]", szAscii);
		inLogPrintf(AT, szDebugMsg);
	}
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_EW_Pack03() END!");
	
        return (inCnt);
}

/*
Function        :inNCCC_EW_Pack04
Date&Time       :
Describe        :Field_4:       Amount, Transaction
*/
int inNCCC_EW_Pack04(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 		inCnt = 0;
        char 		szTemplate[100 + 1], szDebugMsg[100 + 1];
	char		szAscii[12 + 1];
        unsigned char 	uszBCD[6 + 1];

	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_EW_Pack04() START!");
	
	
        memset(szTemplate, 0x00, sizeof(szTemplate));

        switch (pobTran->inISOTxnCode)
        {
                default:
                        sprintf(szTemplate, "%010ld%s", pobTran->srEWRec.lnTxnAmount, pobTran->srEWRec.szEW_Decimal);
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
                inLogPrintf(AT, "inNCCC_EW_Pack04() END!");
	
        return (inCnt);
}

/*
Function        :inNCCC_EW_Pack11
Date&Time       :
Describe        :Field_11:      System Trace Audit Number
*/
int inNCCC_EW_Pack11(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 		inCnt = 0;
        char 		szSTAN[6 + 1];
	char		szDebugMsg[100 + 1];
	char		szAscii[6 + 1];
        unsigned char 	uszBCD[3 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_EW_Pack11() START!");
	

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
                inLogPrintf(AT, "inNCCC_EW_Pack11() END!");
	
        return (inCnt);
}

/*
Function        :inNCCC_EW_Pack12
Date&Time       :
Describe        :Field_12:      Time, Local Transaction
*/
int inNCCC_EW_Pack12(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 		inCnt = 0;
	char		szDebugMsg[100 + 1];
	char		szAscii[6 + 1];
        unsigned char 	uszBCD[3 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_EW_Pack12() START!");
	

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
                inLogPrintf(AT, "inNCCC_EW_Pack12() END!");
	
        return (inCnt);
}

/*
Function        :inNCCC_EW_Pack13
Date&Time       :
Describe        :Field_13:      Date, Local Transaction
*/
int inNCCC_EW_Pack13(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 		inCnt = 0;
	char		szDebugMsg[100 + 1];
	char		szAscii[4 + 1];
        unsigned char 	uszBCD[2 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_EW_Pack13() START!");

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
                inLogPrintf(AT, "inNCCC_EW_Pack13() END!");

        return (inCnt);
}

/*
Function        :inNCCC_EW_Pack22
Date&Time       :
Describe        :Field_22:   Point of Service Entry Mode
*/
int inNCCC_EW_Pack22(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int             inCnt = 0;
	char		szDebugMsg[100 + 1];			
	char		szAscii[4 + 1];				/* 放Debug中轉出來的Ascii用 */
        char            szPOSmode[4 + 1];			/* Field 22的Ascii值 */
	unsigned char	uszPinBit = VS_FALSE;
        unsigned char   uszBCD[2 + 1];				/* Field 22的Hex值 */
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_EW_Pack22() START!");
	
	/* 1.比對TerminalCapbility */
	uszPinBit = VS_FALSE;
	/* 不是CUP，則pin bit一律為True (陳美鈴 2014-07-10 (週四) PM 07:48郵件通知：非銀聯卡的pos entry mode第4碼, 不管有沒有輸入PIN都要帶”1”)*/
	uszPinBit = VS_TRUE;
	
        /*
         Position 1       |     PAN Entry Mode
        ------------------------------------------------------------
             5            |     EMV compliance terminal
        -------------------------------------------------------------
         Position 2 & 3   |     PAN Entry Mode
        ------------------------------------------------------------
	     00		  |	僅限「 人工授權 系統」（ EWLOG ）使用
	------------------------------------------------------------
             01           |     PAN entered manually
			  |	感應退貨(感應退貨會轉成手動輸入的flag(uszManualBit))
			  |	感應預先授權完成
			  |	「人工授權系統」（ EWLOG DFS/TWIN 卡使用
        ------------------------------------------------------------
             02           |     PAN auto-entry via magnetic stripe
		          |     Fallback
                          |     (Smart Pay不支援磁條卡或Fallback)
        ------------------------------------------------------------
	     03		  |	PAN auto entry via scan QR C( 被掃模式
			  |	UNY 實體掃碼交易
			  |	電子錢包交易
	-------------------------------------------------------------
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
	     67		  |	Smart Pay Contactless(Consumer Device)
			  |	Consumer Device  例： Smart Phone
	--------------------------------------------------------------
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
	strcat(szPOSmode, "03");

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
                inLogPrintf(AT, "inNCCC_EW_Pack22() END!");
	
        return (inCnt);
}

/*
Function        :inNCCC_EW_Pack24
Date&Time       :
Describe        :Field_24:      Network International Identifier (NII)
*/
int inNCCC_EW_Pack24(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 		inCnt = 0;
        char 		szTemplate[100 + 1];
	char		szDebugMsg[100 + 1];
	char		szAscii[4 + 1];
        unsigned char 	uszBCD[2 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_EW_Pack24() START!");
	

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
                inLogPrintf(AT, "inNCCC_EW_Pack24() END!");

        return (inCnt);
}

/*
Function        :inNCCC_EW_Pack25
Date&Time       :
Describe        :Field_25:   Point of Service Condition Code
*/
int inNCCC_EW_Pack25(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 		inCnt = 0;
	char		szTemplate[100 + 1];
	char		szDebugMsg[100 + 1];
	char		szAscii[2 + 1];
        unsigned char 	uszBCD[1 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_EW_Pack25() START!");

        /*
                    Code        |       Meaning
                ---------------------------------------------------
                    00          |       Normal presentment
                                |       Smart Pay所有交易
				|	銀聯純 QRC 交易
				|	UNY 實體掃碼交易
				|	電子錢包交易
                ---------------------------------------------------
                    06          |       Pre-Auth
				|	Pre-Auth Compelete
                                |---------------------------------------
                                |       Force
                                |       (Online,系統需要強迫授權的交易)
                ---------------------------------------------------
                    08          |       MO/TO
		-----------------------------------------------------
		    63		|	銀聯Uplan Coupon V erification Transaction
				|	包含銀聯優計畫(核券+卡片消費)及二碼合一(核券+QRC 消費)
				|	註：不含銀聯純 QRC 消費。
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
                inLogPrintf(AT, "inNCCC_EW_Pack25() END!");

        return (inCnt);
}

/*
Function        :inNCCC_EW_Pack32
Date&Time       :
Describe        :Field_32:    Acquiring Institution Identification Code(金融代碼)
*/
int inNCCC_EW_Pack32(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int	inCnt = 0;
	char	szAscii[6 + 1];
	char	szDebugMsg[100 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_EW_Pack32() START!");

        /*
                                補充說明：
                1. 本欄位請依收單銀行之金融機構代碼上傳(3碼)。
                2. 範例：金融代碼=956
                        Length (BCD)+ Data(BCD)= 0x02 0x09 0x56
                3. 吳升文 E-Mail Reply : 2014-04-03 (週四) AM 09:44
                        Field_32是變動長度，所以定義最多11 Betes，但是ATS系統只用2 Bytes (3 Nibble)。所以端末機開發可以Hard Code填入範例的值即可。
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
                inLogPrintf(AT, "inNCCC_EW_Pack32() END!");
	
        return (inCnt);
}

/*
Function        :inNCCC_EW_Pack35
Date&Time       :
Describe        :Field_35:   TRACK 2 Data
*/
int inNCCC_EW_Pack35(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int     inCnt = 0;
        char    szT2Data[50 + 1];		/* 目前track2最長到32 */
	char	szDebugMsg[100 + 1];
	short   shLen = 0;
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_EW_Pack35() START!");

	memset(szT2Data, 0x00, sizeof(szT2Data));

	/* 電子錢包交易條碼 */
	shLen = strlen(pobTran->srBRec.szBarCodeData);
	uszPackBuf[inCnt ++] = ((shLen / 10) * 16) + (shLen %10);
	memset(szT2Data, 0x00, sizeof(szT2Data));
	memcpy(szT2Data, pobTran->srBRec.szBarCodeData, shLen);
	memcpy(&uszPackBuf[inCnt], szT2Data, shLen);
	inCnt += shLen;

	/* 組MAC使用 */
	memset(gszMAC_F_35, 0x00, sizeof(gszMAC_F_35));
	memcpy(gszMAC_F_35, pobTran->srBRec.szBarCodeData, shLen);

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "F_35 [%X %s]", uszPackBuf[inCnt - shLen - 1], szT2Data);
		inLogPrintf(AT, szDebugMsg);
	}
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_EW_Pack35() END!");

        return (inCnt);
}

/*
Function        :inNCCC_EW_Pack35_tSAM
Date&Time       :
Describe        :Field_35:   TRACK 2 Data
*/
int inNCCC_EW_Pack35_tSAM(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        /* Field_35:	TRACK 2 Data */
	int		inCnt = 0, inRetVal = -1;
	int		inLen = 0;
	int		inRemainLen = 0;
	char		szT2Data[50 + 1];				/* 目前track2最長到32 */
	char		szTemplate[100 + 1];
	char		szDebugMsg[100 + 1];
	char		szEncrypt[50 + 1];
	char		szField37[8 + 1], szField41[4 + 1];
	char		szFinalAscii[200 + 1] = {0};
	short		shLen = 0;
	unsigned char	uszSlot = 0;
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_EW_Pack35_tSAM() START!");
	
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

	memset(szT2Data, 0x00, sizeof(szT2Data));

	shLen = strlen(pobTran->srBRec.szBarCodeData);
	uszPackBuf[inCnt] = ((shLen / 10) * 16) + (shLen %10);
	inCnt ++;

	inRemainLen = (shLen - 2) % 8;
	inLen = (shLen - 2) - inRemainLen;

	/* 組 Field_37 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(szTemplate, pobTran->srBRec.szRefNo, 11);
	inFunc_PAD_ASCII(szTemplate, szTemplate, 'F', 16, _PADDING_RIGHT_); /* 補【F】 */
	memset(szField37, 0x00, sizeof(szField37));
	inFunc_ASCII_to_BCD((unsigned char*)szField37, szTemplate, 8);
	/* 組 Field_41 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTerminalID(szTemplate);
	memset(szField41, 0x00, sizeof(szField41));
	inFunc_ASCII_to_BCD((unsigned char*)szField41, szTemplate, 4);
	/* Field_35 */
	memset(gusztSAMCheckSum_35, 0x00, sizeof(gusztSAMCheckSum_35));
	memset(szEncrypt, 0x00, sizeof(szEncrypt));

	memcpy(szEncrypt, &pobTran->srBRec.szBarCodeData[2], inLen);

	/* 組MAC使用 */
	memset(gszMAC_F_35, 0x00, sizeof(gszMAC_F_35));
	memcpy(gszMAC_F_35, pobTran->srBRec.szBarCodeData, shLen);

	inRetVal = inNCCC_tSAM_Encrypt(uszSlot, 
				       (inLen * 2),
				       szEncrypt,
				       (unsigned char*)&szField37,
				       (unsigned char*)&szField41,
				       &gusztSAMKeyIndex,
				       gusztSAMCheckSum_35);

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

	memset(szFinalAscii, 0x00, sizeof(szFinalAscii));
	memcpy(&szFinalAscii[0], &pobTran->srBRec.szBarCodeData[0], 2);
	memcpy(&szFinalAscii[2], szEncrypt, inRetVal);
	memcpy(&szFinalAscii[2 + inRetVal], &pobTran->srBRec.szBarCodeData[2 + inRetVal], inRemainLen);

	memcpy(&uszPackBuf[inCnt], szFinalAscii, 2 + inRetVal + inRemainLen);
	inCnt += 2 + inRetVal + inRemainLen;

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "F_35 [%X %s]", uszPackBuf[inCnt - inRemainLen - inRetVal - 2 - 1], szFinalAscii);
		inLogPrintf(AT, szDebugMsg);
	}
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_EW_Pack35_tSAM() END!");
	
	return (inCnt);
}

/*
Function        :inNCCC_EW_Pack37
Date&Time       :
Describe        :Field_37:	Retrieval Reference Number (RRN)
*/
int inNCCC_EW_Pack37(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 	inCnt = 0;
	char	szDebugMsg[100 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_EW_Pack37() START!");
	
	/* 靠inNCCC_Func_MakeRefNo來組 */
		/* pobTran->srBRec.szRefNo 有可能長度<12嗎? 這行會發生什麼事? */
        memcpy((char *) &uszPackBuf[inCnt], pobTran->srBRec.szRefNo, 12);
        inCnt += 12;

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "F_37 [RRN %s]", pobTran->srBRec.szRefNo);
		inLogPrintf(AT, szDebugMsg);
	}
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_EW_Pack37() END!");
	
        return (inCnt);
}

/*
Function        :inNCCC_EW_Pack38
Date&Time       :
Describe        :Field_38:	Authorization Identification Response（SmartPay交易無授權碼）
*/
int inNCCC_EW_Pack38(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 	inCnt = 0;
        char    szAuthCode[_AUTH_CODE_SIZE_ + 1];
	char	szDebugMsg[100 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_EW_Pack38() START!");


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
                inLogPrintf(AT, "inNCCC_EW_Pack38() END!");
	
        return (inCnt);
}

/*
Function        :inNCCC_EW_Pack41
Date&Time       :
Describe        :Field_41:	Card Acceptor Terminal Identification（TID）補充說明：端末機代號需可支援英、數字。
*/
int inNCCC_EW_Pack41(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 	inCnt = 0;
        char 	szTemplate[8 + 1];
	char	szDebugMsg[100 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_EW_Pack41() START!");
	

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
                inLogPrintf(AT, "inNCCC_EW_Pack41() END!");

        return (inCnt);
}

/*
Function        :inNCCC_EW_Pack42
Date&Time       :
Describe        :Field_42:	Card Acceptor Identification Code(MID)
 		補充說明：
		1. 商店代號需左靠右補空白。
		2. 語音開卡交易須使用固定商店代號 0122900410 。
*/
int inNCCC_EW_Pack42(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 	inCnt = 0;
        char	szTemplate[16 + 1];
	char 	szDebugMsg[100 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_EW_Pack42() START!");
	
	
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
                inLogPrintf(AT, "inNCCC_EW_Pack42() END!");

        return (inCnt);
}

/*
Function        :inNCCC_EW_Pack48
Date&Time       :
Describe        :Field_48:	Additional Data - Private Use
*/
int inNCCC_EW_Pack48(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int	i = 0;
        int	inCnt = 0;
	char	szDebugMsg[150 + 1] = {0};
	char	szTemplate[40 + 1] = {0};
	char	szAscii[120 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
	char	szPOS_ID[6 + 1] = {0};
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_EW_Pack48() START!");

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
                inLogPrintf(AT, "inNCCC_EW_Pack48() END!");

	return (inCnt);
}

/*
Function        :inNCCC_EW_Pack57_tSAM
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
int inNCCC_EW_Pack57_tSAM(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int		inCnt = 0, inPacketCnt = 0;
	char		szASCII[4 + 1];
	char		szPacket[100 + 1];
	char		szPacket_Ascii[100 + 1];
	char		szTemplate[100 + 1];
	char		szCustomerIndiator[2 + 1] = {0};
	unsigned char	uszBCD[2 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_EW_Pack57_tSAM() START!");
	
	
	/* BitMap 8 Byte */
	memset(szPacket, 0x00, sizeof(szPacket));
	szPacket[inPacketCnt ++] = 0x00;
	szPacket[inPacketCnt ++] = 0x00;
	szPacket[inPacketCnt ++] = 0x00;
	szPacket[inPacketCnt ++] = 0x00;

	/* 第5個byte */
	if (guszField_35 == VS_TRUE)
		szPacket[inPacketCnt ++] = 0x20;
	else
		szPacket[inPacketCnt ++] = 0x00;

	szPacket[inPacketCnt ++] = 0x00;

	szPacket[inPacketCnt ++] = 0x00;

	/* 【需求單 - 104014】發卡機構on-us繳費平台需求 +【需求單 - 104152】發卡機構on-us繳費平台 Payitem Code */
	/* Field 59 Table NY 加密 */
	memset(szCustomerIndiator, 0x00, sizeof(szCustomerIndiator));
	inGetCustomIndicator(szCustomerIndiator);
	if (!memcmp(szCustomerIndiator, _CUSTOMER_INDICATOR_084_ON_US_, _CUSTOMER_INDICATOR_SIZE_) &&
	    strlen(pobTran->szIDNumber) > 0)
	{
		inNCCC_Func_IDNumber_Encrypt(pobTran);
		szPacket[inPacketCnt ++] = 0x20;
	}
	else
		szPacket[inPacketCnt ++] = 0x00;
	
	if (guszField_35 == VS_TRUE)
	{
		memcpy((char *)&szPacket[inPacketCnt], (char *)&gusztSAMCheckSum_35[0], 4);
		inPacketCnt += 4;
	}

	/* Field 55現在沒有需加密的欄位*/

	/* 【需求單 - 104014】發卡機構on-us繳費平台需求 +【需求單 - 104152】發卡機構on-us繳費平台 Payitem Code by Russell 2020/3/16 下午 5:38 */
	if (!memcmp(szCustomerIndiator, _CUSTOMER_INDICATOR_084_ON_US_, _CUSTOMER_INDICATOR_SIZE_) &&
	    strlen(pobTran->szIDNumber) > 0)
	{
		memcpy((char *)&szPacket[inPacketCnt], (char *)&gusztSAMCheckSum_59[0], 4);
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
                inLogPrintf(AT, "inNCCC_EW_Pack57_tSAM() END!");
	
        return (inCnt);
}

/*
Function        :inNCCC_EW_Pack59
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
int inNCCC_EW_Pack59(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int		inCnt = 0, inPacketCnt = 0;
	int		inSubLen = 0;
	char		szASCII[4 + 1] = {0};
	char		szTemplate[400 + 1] = {0}, szTemplate1[100 + 1] = {0}, szPacket[768 + 1] = {0}; /* sztemplae for debug message,szTemplate1 for temprary*/
	char		szFESMode[2 + 1] = {0};
	char		szFES_ID[3 + 1] = {0};
	char		szCFESMode[1 + 1] = {0};
	char		szCustomerIndicator[2 + 1] = {0};
	unsigned char	uszBCD[2 + 1] = {0};
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_EW_Pack59() START!");

        memset(szTemplate1, 0x00, sizeof(szTemplate1));
	memset(szPacket, 0x00, sizeof(szPacket));
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);

	if (!memcmp(&guszEW_MTI[0], "0800", 4))
	{
		/*
			Table ID “N3”: Host AD “YYYY” (系統西元年)
			1. 為解決ISO8583無法更新西元年的問題，故增加此Table ID N3，端末機可依此欄位更新端末機系統年份(西元)。
			2. 避免端末機因Clock故障或是跨年的秒差造成交易年份錯誤。
		*/
		memcpy(&szPacket[inPacketCnt], "N3", 2); /* Table ID */
		inPacketCnt += 2;
		/* Sub Total Length */
		inPacketCnt ++;
		szPacket[inPacketCnt] = 0x04;
		inPacketCnt ++;
		memcpy(&szPacket[inPacketCnt], "0000", 4); /* YYYY，上傳時，固定為'0000'，並由授權主機更新回傳 */
		inPacketCnt += 4;
		
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, " Table ID   [N3]");
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, " Table Len  [%s %s]","00","04");
			inLogPrintf(AT, szTemplate);
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "  Table Data [%s]", "0000");
			inLogPrintf(AT, szTemplate);
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
				/* EW判斷端末機上傳Table ID "NF" = CFES
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
			Table ID “NF”: FES Indicator
			依TMS Config.txt中NCCC_FES_Mode參數設定上傳值。
			當NCCC_FES_Mode=03時，表示此為MFES端末機，本欄位值為”MFxx”。
			(端末機預設xx=0x20 0x20。xx由MFES填入新值。
			當MFES已經進行基本查核後SAF的交易，EW不需再查，則MFES填入xx=”SF”。
			反之則維持0x20,0x20)
			當NCCC_FES_Mode=05時，表示此非MFES端末機，本欄位值為”EW”。(表0x20)
			EW以此判斷若為”MFES”端末機，則EW不產生該端末機的請款檔。
		*/
		memcpy(&szPacket[inPacketCnt], "NF", 2); /* Table ID */
		inPacketCnt += 2;
		/* Sub Total Length */
		inPacketCnt ++;
		szPacket[inPacketCnt] = 0x04;
		inPacketCnt ++;
		memset(szFESMode, 0x00, sizeof(szFESMode));
		inGetNCCCFESMode(szFESMode);
		memset(szCFESMode, 0x00, sizeof(szCFESMode));
		inGetCloud_MFES(szCFESMode);
		
		if (memcmp(szFESMode, _NCCC_04_MPAS_MODE_, strlen(_NCCC_04_MPAS_MODE_)) == 0)
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
		
		/* Table ID WQ */
		memcpy(&szPacket[inPacketCnt], "WQ", 2); 
		inPacketCnt += 2;

		inSubLen = 0;
		memset(szTemplate, 0x00, sizeof(szTemplate));

		/* 特店訂單編號 (ans 30) */
		inFunc_PAD_ASCII(&szTemplate[inSubLen], pobTran->srEWRec.szEW_OrderId, ' ', 30, _PADDING_RIGHT_);
		inSubLen += 30;

		/* TP Data長度轉BCD */
		memset(szASCII, 0x00, sizeof(szASCII));
		sprintf(szASCII, "%04d", inSubLen);
		memset(uszBCD, 0x00, sizeof(uszBCD));
		inFunc_ASCII_to_BCD(uszBCD, szASCII, 2);

		/* Sub-Data Total Length */
		memcpy(&szPacket[inPacketCnt], &uszBCD[0], 2);
		inPacketCnt += 2;
		memcpy(&szPacket[inPacketCnt], &szTemplate[0], inSubLen);
		inPacketCnt += inSubLen;
	}
	else
	{
		/*
			Table ID “N2”: Settlement Flag
			Settlement Flag
			依TMS下載的Settlement Flag填入此欄位
			1=”ST” (EW清算)
			2=”AU” (EW不清算)
		*/
		memcpy(&szPacket[inPacketCnt], "N2", 2); /* Table ID */
		inPacketCnt += 2;

		/* Sub Total Length */
		szPacket[inPacketCnt] = 0x00;
		inPacketCnt ++;
		szPacket[inPacketCnt] = 0x02;
		inPacketCnt ++;
		/*
			吳升文(Angus Wu) <angus.wu@nccc.com.tw> 2014-05-26 (週一) PM 03:43 通知
			EW針對Table ID “N2” Settlement Flag的值，請調整為當收到TMS客製化參數042(建設公司)及043(保險公司)時，填入”AU”。
			其餘填入”ST”。
		*/
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_042_BDAU1_, _CUSTOMER_INDICATOR_SIZE_)	||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_043_BDAU9_, _CUSTOMER_INDICATOR_SIZE_))
			memcpy(&szPacket[inPacketCnt], "AU", 2);
		else
			memcpy(&szPacket[inPacketCnt], "ST", 2);	
		inPacketCnt += 2;

		/*
			Table ID “N3”: Host AD “YYYY” (系統西元年)
			1. 為解決ISO8583無法更新西元年的問題，故增加此Table ID N3，端末機可依此欄位更新端末機系統年份(西元)。
			2. 避免端末機因Clock故障或是跨年的秒差造成交易年份錯誤。
		*/
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
				/* EW判斷端末機上傳Table ID "NF" = CFES
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
			Table ID “NF”: FES Indicator
			依TMS Config.txt中NCCC_FES_Mode參數設定上傳值。
			當NCCC_FES_Mode=03時，表示此為MFES端末機，本欄位值為”MFxx”。
			(端末機預設xx=0x20 0x20。xx由MFES填入新值。
			當MFES已經進行基本查核後SAF的交易，EW不需再查，則MFES填入xx=”SF”。
			反之則維持0x20,0x20)
			當NCCC_FES_Mode=05時，表示此非MFES端末機，本欄位值為”EW”。(表0x20)
			EW以此判斷若為”MFES”端末機，則EW不產生該端末機的請款檔。
		*/
		/* (需求單 - 107276)自助交易標準做法 by Russell 2019/3/5 下午 4:39 */
		memcpy(&szPacket[inPacketCnt], "NF", 2); /* Table ID */
		inPacketCnt += 2;
		
		/* Sub Total Length */
		szPacket[inPacketCnt] = 0x00;
		inPacketCnt ++;
		szPacket[inPacketCnt] = 0x04;
		inPacketCnt ++;
		
		memset(szFESMode, 0x00, sizeof(szFESMode));
		inGetNCCCFESMode(szFESMode);
		memset(szCFESMode, 0x00, sizeof(szCFESMode));
		inGetCloud_MFES(szCFESMode);
		
		if (memcmp(szFESMode, _NCCC_04_MPAS_MODE_, strlen(_NCCC_04_MPAS_MODE_)) == 0)
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
			Table ID "WS"：電子錢包交易 (e-Wallet Message - Sale
		*/
		if (pobTran->srBRec.inCode == _EW_SALE_)
		{
			memcpy(&szPacket[inPacketCnt], "WS", 2); /* Table ID */
			inPacketCnt += 2;

			inSubLen = 0;
			memset(szTemplate, 0x00, sizeof(szTemplate));

			/* 特店訂單編號 (ans 30)（左靠右補空白） */
			inFunc_PAD_ASCII(&szTemplate[inSubLen], pobTran->srEWRec.szEW_OrderId, ' ', 30, _PADDING_RIGHT_);
			inSubLen += 30;

			/* 一般交易金額（不含小數位）(ans 30)（右靠左補0）。 */
			inFunc_PAD_ASCII(&szTemplate[inSubLen], pobTran->srEWRec.szEW_NormalAmt, '0', 10, _PADDING_LEFT_);
			inSubLen += 10;

			/* 代收交易金額（不含小數位）(ans 10)（右靠左補0）。 */
			inFunc_PAD_ASCII(&szTemplate[inSubLen], pobTran->srEWRec.szEW_CollectionAmt, '0', 10, _PADDING_LEFT_);
			inSubLen += 10;

			/* 代售交易金額（不含小數位）(ans 10)（右靠左補0）。 */
			inFunc_PAD_ASCII(&szTemplate[inSubLen], pobTran->srEWRec.szEW_ConsignmentAmt, '0', 10, _PADDING_LEFT_);
			inSubLen += 10;
			
			/* 不可折抵金額（不含小數位）(ans 10)（右靠左補0）。 */
			inFunc_PAD_ASCII(&szTemplate[inSubLen], pobTran->srEWRec.szEW_PromotionRestriction, '0', 10, _PADDING_LEFT_);
			inSubLen += 10;

			/* Data長度轉BCD */
			memset(szASCII, 0x00, sizeof(szASCII));
			sprintf(szASCII, "%04d", inSubLen);
			memset(uszBCD, 0x00, sizeof(uszBCD));
			inFunc_ASCII_to_BCD(uszBCD, szASCII, 2);

			/* Sub-Data Total Length */
			memcpy(&szPacket[inPacketCnt], &uszBCD[0], 2);
			inPacketCnt += 2;
			memcpy(&szPacket[inPacketCnt], &szTemplate[0], inSubLen);
			inPacketCnt += inSubLen;
		}
		
		/*
			Table ID "WR"：電子錢包交易 (e-Wallet Message - Refund
		*/
		if (pobTran->srBRec.inCode == _EW_REFUND_)
		{
			memcpy(&szPacket[inPacketCnt], "WR", 2); /* Table ID */
			inPacketCnt += 2;

			inSubLen = 0;
			memset(szTemplate, 0x00, sizeof(szTemplate));

			/* 特店訂單編號 (ans 30) （左靠右補空白） */
			inFunc_PAD_ASCII(&szTemplate[inSubLen], pobTran->srEWRec.szEW_OrderId, ' ', 30, _PADDING_RIGHT_);
			inSubLen += 30;

			/* 錢包業者交易序號 (ans 64)。 （左靠右補空白） */
			inFunc_PAD_ASCII(&szTemplate[inSubLen], pobTran->srEWRec.szEW_TransactionId, ' ', 64, _PADDING_RIGHT_);
			inSubLen += 64;

			/* 退貨訂單編號(ans 30)。（左靠右補空白、不適用業者全帶空白space） */
			inFunc_PAD_ASCII(&szTemplate[inSubLen], pobTran->srEWRec.szEW_RefundTradeNo, ' ', 30, _PADDING_RIGHT_);
			inSubLen += 30;

			/* 一般退貨金額（不含小數位）(ans 10)（右靠左補0、不適用業者全帶0）。 */
			inFunc_PAD_ASCII(&szTemplate[inSubLen], pobTran->srEWRec.szEW_NormalAmt, '0', 10, _PADDING_LEFT_);
			inSubLen += 10;
			
			/* 代收退貨金額（不含小數位）(ans 10)（右靠左補0、不適用業者全帶0）。 */
			inFunc_PAD_ASCII(&szTemplate[inSubLen], pobTran->srEWRec.szEW_CollectionAmt, '0', 10, _PADDING_LEFT_);
			inSubLen += 10;
			
			/* 代售退貨金額（不含小數位）(ans 10)（右靠左補0、不適用業者全帶0）。 */
			inFunc_PAD_ASCII(&szTemplate[inSubLen], pobTran->srEWRec.szEW_ConsignmentAmt, '0', 10, _PADDING_LEFT_);
			inSubLen += 10;

			/* Data長度轉BCD */
			memset(szASCII, 0x00, sizeof(szASCII));
			sprintf(szASCII, "%04d", inSubLen);
			memset(uszBCD, 0x00, sizeof(uszBCD));
			inFunc_ASCII_to_BCD(uszBCD, szASCII, 2);

			/* Sub-Data Total Length */
			memcpy(&szPacket[inPacketCnt], &uszBCD[0], 2);
			inPacketCnt += 2;
			memcpy(&szPacket[inPacketCnt], &szTemplate[0], inSubLen);
			inPacketCnt += inSubLen;
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
                inLogPrintf(AT, "inNCCC_EW_Pack59() END!");
	
        return (inCnt);
}

/*
Function        :inNCCC_EW_Pack60
Date&Time       :
Describe        :Field_60:	Reserved-Private Data
		Record of Count must be Unique in the batch.(Batch Number)
*/
int inNCCC_EW_Pack60(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 	inCnt = 0;
        long 	lnBatchNum;
        char 	szTemplate[110 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_EW_Pack60() START!");

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
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_EW_Pack60() END!");

        return (inCnt);
}

/*
Function        :inNCCC_EW_Pack62
Date&Time       :
Describe        :Field_62:	Reserved-Private Data(Invoice Number)
*/
int inNCCC_EW_Pack62(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 	inCnt = 0;
	char	szTemplate[100 + 1];

	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_EW_Pack62() START!");
	
	
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
                inLogPrintf(AT, "inNCCC_EW_Pack62() END!");

        return (inCnt);
}

/*
Function        :inNCCC_EW_Pack64
Date&Time       :
Describe        :Field_64	Reserved-Private Data
		1. 除了0800及0400之外，其餘交易電文皆要上傳MAC。
		2. 若連續三次MAC Error則EDC先自動關閉MAC上傳(含無輸入PIN的CUP交易)。
*/
int inNCCC_EW_Pack64(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int     inCnt = 0;
	char	szTerminalID[8 + 1];
	char	szTemplate[100 + 1];
	char	szAscii[16 + 1];

	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_EW_Pack64() START!");
	
	memset(szTerminalID, 0x00, sizeof(szTerminalID));
	inGetTerminalID(szTerminalID);
	if (inNCCC_Func_Uny_GenMAC(pobTran, gszMAC_F_03, gszMAC_F_04, gszMAC_F_11, gszMAC_F_35, &szTerminalID[0], gszMAC_F_63) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "Uny 壓MAC失敗");
		}
		return (VS_ERROR);
	}

	memcpy((char *)&uszPackBuf[inCnt], &pobTran->szMAC_HEX[0], 8);
	inCnt += 8;
	
	if (ginDebug == VS_TRUE)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(szAscii, 0x00, sizeof(szAscii));
		inFunc_BCD_to_ASCII(szAscii, (unsigned char*)pobTran->szMAC_HEX, 8);
		sprintf(szTemplate, "F_64 [MAC %s]", szAscii);
		inLogPrintf(AT, szTemplate);
	}
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_EW_Pack64() END!");
	
        return (inCnt);
}

/*
Function        :inNCCC_EW_Check03
Date&Time       :
Describe        :送和收的processing Code要一致
*/
int inNCCC_EW_Check03(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, unsigned char *uszRecePacket)
{
        if (memcmp((char *)uszSendPacket, (char *)uszRecePacket, 3))
        {
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_EW_Check04
Date&Time       :
Describe        :送和收的Amount要一致
*/
int inNCCC_EW_Check04(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, unsigned char *uszRecePacket)
{
        if (memcmp((char *)uszSendPacket, (char *)uszRecePacket, 6))
        {
                //vdSGErrorMessage(NCCC_CHECK_ISO_FILED04_ERROR); /* 140 = 電文錯誤請重試 */
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_EW_Check41
Date&Time       :
Describe        :送和收的TID要一致
*/
int inNCCC_EW_Check41(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, unsigned char *uszRecePacket)
{
        if (memcmp((char *)uszSendPacket, (char *)uszRecePacket, 8))
        {
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_EW_UnPack12
Date&Time       :
Describe        :
*/
int inNCCC_EW_UnPack04(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
	char	szAscii[12 + 1] = {0};
	
	if (pobTran->srBRec.inCode == _EW_INQUIRY_)
	{
		memset(szAscii, 0x00, sizeof(szAscii));
		inFunc_BCD_to_ASCII(szAscii, &uszUnPackBuf[0], 5);

		pobTran->srEWRec.lnTxnAmount = atol(szAscii);
		
		memset(szAscii, 0x00, sizeof(szAscii));
		inFunc_BCD_to_ASCII(szAscii, &uszUnPackBuf[5], 2);
		memcpy(pobTran->srEWRec.szEW_Decimal, szAscii, 2);
	}

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_EW_UnPack12
Date&Time       :
Describe        :
*/
int inNCCC_EW_UnPack12(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
	/* 因無法確認各電子錢包交易主機是否有進行對時，為避免時間出現錯亂狀況，故EDC僅針對信用卡的交易才進行對時，電子錢包交易則不進行。
	 * 電子錢包交易取日期時間回傳，但不更新到機器中 */
	memset(pobTran->srBRec.szTime, 0x00, sizeof(pobTran->srBRec.szTime));
	inFunc_BCD_to_ASCII(&pobTran->srBRec.szTime[0], &uszUnPackBuf[0], 3);

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_EW_UnPack13
Date&Time       :
Describe        :
*/
int inNCCC_EW_UnPack13(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
	/* 因無法確認各電子錢包交易主機是否有進行對時，為避免時間出現錯亂狀況，故EDC僅針對信用卡的交易才進行對時，電子錢包交易則不進行。
	 * 電子錢包交易取日期時間回傳，但不更新到機器中 */
	memset(&pobTran->srBRec.szDate[4], 0x00, 4);
	inFunc_BCD_to_ASCII(&pobTran->srBRec.szDate[4], &uszUnPackBuf[0], 2);

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_EW_UnPack24
Date&Time       :
Describe        : 1. NPS 需求要判斷 Field_24 第一個 Byte 是否為【9】
		2. 此 Flag 是由主機做控管，端末機不管例外處理
*/
int inNCCC_EW_UnPack24(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
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
Function        :inNCCC_EW_UnPack37
Date&Time       :
Describe        :
*/
int inNCCC_EW_UnPack37(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
        if (pobTran->inISOTxnCode == _REVERSAL_)
                return (VS_SUCCESS);

        memset(pobTran->srBRec.szRefNo, 0x00, sizeof(pobTran->srBRec.szRefNo));
        memcpy(&pobTran->srBRec.szRefNo[0], (char *) &uszUnPackBuf[0], 12);

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_EW_UnPack38
Date&Time       :
Describe        :
*/
int inNCCC_EW_UnPack38(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
	char	szFESMode[2 + 1] = {0};
	
	memset(pobTran->srBRec.szAuthCode, 0x00, sizeof(pobTran->srBRec.szAuthCode));
	memcpy(&pobTran->srBRec.szAuthCode[0], (char *) &uszUnPackBuf[0], _AUTH_CODE_SIZE_);
	
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
Function        :inNCCC_EW_UnPack39
Date&Time       :
Describe        :Response code
*/
int inNCCC_EW_UnPack39(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{	
	char	szTemplate[6 + 1];

	memset(&szTemplate, 0x00, sizeof(szTemplate));
	inFunc_BCD_to_ASCII(&szTemplate[0], &guszNCCC_EW_ISO_Field03[0], 3);

	memset(pobTran->srBRec.szRespCode, 0x00, sizeof(pobTran->srBRec.szRespCode));
	memcpy(&pobTran->srBRec.szRespCode[0], (char *)&uszUnPackBuf[0], 2);

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_EW_UnPack59
Date&Time       :
Describe        :
*/
int inNCCC_EW_UnPack59(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
        int			inLen, inCnt = 0, inSubTotalLength, inTPCnt, inCardLabelLen = 0;
	int			inRetVal = VS_ERROR, i;
	int			inCDTXIndex = 0;
	int			inUnyCardLabelLen = 0;
	int			inTempLen = 0;
	char			szTemplate[1024 + 1];
	char			szTemplate2[8 + 1];
	char			szTemp[8 + 1];
	char			szDebugMsg1[42 + 1], szDebugMsg2[42 + 1];
        char                    szCustomerIndicator[3 + 1] = {0};
        
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);

        if (pobTran->inISOTxnCode == _REVERSAL_)
                return (VS_SUCCESS);

	inLen = ((uszUnPackBuf[0] / 16 * 10 + uszUnPackBuf[0] % 16) * 100);
	inLen += (uszUnPackBuf[1] / 16 * 10) + uszUnPackBuf[1] % 16;

	if (inLen > 0)
	{
		inCnt += 2;

		while (inLen > inCnt)
		{
			if (!memcmp(&uszUnPackBuf[inCnt], "N1", 2))
			{
				/* Table ID “N1”: UnionPay (原CUP)交易 */
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
				/* Table ID “N2”: Settlement Flag */
				inCnt += 2; /* Table ID */
				/* Sub Total Length */
				inSubTotalLength = ((uszUnPackBuf[inCnt] % 16) * 100) + ((uszUnPackBuf[inCnt + 1] / 16) * 10) + (uszUnPackBuf[inCnt + 1] % 16);
				inCnt += 2;
				inCnt += inSubTotalLength;
			}
			else if (!memcmp(&uszUnPackBuf[inCnt], "N3", 2))
			{
				/* Table ID “N3”: Host AD “YYYY” (系統西元年) */
				inCnt += 2; /* Table ID */
				if (ginDebug == VS_TRUE)
				{
					memset(szTemplate, 0x00, sizeof(szTemplate));
					
					strcat(szTemplate, " Table ID   [");
					memcpy(&szTemplate[strlen(szTemplate)], &uszUnPackBuf[inCnt - 2], 2);
					strcat(szTemplate, "]");
					inLogPrintf(AT, szTemplate);
				}
				
				/* Sub Total Length */
				inSubTotalLength = ((uszUnPackBuf[inCnt] % 16) * 100) + ((uszUnPackBuf[inCnt + 1] / 16) * 10) + (uszUnPackBuf[inCnt + 1] % 16);
				inCnt += 2;
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg1, 0x00, sizeof(szDebugMsg1));
					inFunc_BCD_to_ASCII(szDebugMsg1, &uszUnPackBuf[inCnt-2], 1);
					memset(szDebugMsg2, 0x00, sizeof(szDebugMsg2));
					inFunc_BCD_to_ASCII(szDebugMsg2, &uszUnPackBuf[inCnt-1], 1);
					
					memset(szTemplate, 0x00, sizeof(szTemplate));
					strcat(szTemplate, " Table Len  [");
					strcat(szTemplate, szDebugMsg1);
					strcat(szTemplate, " ");
					strcat(szTemplate, szDebugMsg2);
					strcat(szTemplate, "]");
					
					strcat(szTemplate, "[");
					memset(szTemp, 0x00, sizeof(szTemp));
					sprintf(szTemp, "%i", atoi(szDebugMsg2));
					strcat(szTemplate, szTemp);
					strcat(szTemplate, "]");
					inLogPrintf(AT, szTemplate);
				}
				
				memcpy(&pobTran->srBRec.szDate[0], &uszUnPackBuf[inCnt], 4);
				
				inCnt += 4;
			}
			else if (!memcmp(&uszUnPackBuf[inCnt], "N4", 2))
			{
				/* ●	Table ID “N4”: Installment Relation Data(分期付款資料) */
				inCnt += 2; /* Table ID */
				/* Sub Total Length */
				inSubTotalLength = ((uszUnPackBuf[inCnt] % 16) * 100) + ((uszUnPackBuf[inCnt + 1] / 16) * 10) + (uszUnPackBuf[inCnt + 1] % 16);
				inCnt += 2;
				/* Installmet Indicator (分期標記)
					‘I’=分期-手續費內含
					‘E’=分期-手續費外加
					請求電文預設值為’I’。
				*/
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
					memcpy(&pobTran->srBRec.szRespCode[0], "XI", 2);
					pobTran->srBRec.szRespCode[2] = 0x00; /* 要顯示【拒絕交易XI】 */
					return (inRetVal);
				}

				/* Installment Period (期數)，右靠左補0 “01” ~ “99” */
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
				        		memcpy(&pobTran->srBRec.szRespCode[0], "I0", 2);
							pobTran->srBRec.szRespCode[2] = 0x00; /* 要顯示【分期電文錯誤】 */
							return (inRetVal);
						}
					}

					pobTran->srBRec.lnInstallmentPeriod = atol(szTemplate);
					inCnt += 2;
				}
				else
				{
					memcpy(&pobTran->srBRec.szRespCode[0], "XI", 2);
					pobTran->srBRec.szRespCode[2] = 0x00; /* 要顯示【拒絕交易XI】 */
					return (inRetVal);
				}

				/* Down Payment(首期金額)，右靠左補0 */
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
				        		memcpy(&pobTran->srBRec.szRespCode[0], "I0", 2);
							pobTran->srBRec.szRespCode[2] = 0x00; /* 要顯示【分期電文錯誤】 */
							return (inRetVal);
						}
					}

					/* 2014/12/1 下午 03:26:52 修正小費重複加的問題 */
					if (pobTran->srBRec.lnTipTxnAmount == 0L)
						pobTran->srBRec.lnInstallmentDownPayment = atol(szTemplate);
					else
						pobTran->srBRec.lnInstallmentDownPayment = (atol(szTemplate) - pobTran->srBRec.lnTipTxnAmount);

					inCnt += 8;
				}
				else
				{
					memcpy(&pobTran->srBRec.szRespCode[0], "XI", 2);
					pobTran->srBRec.szRespCode[2] = 0x00; /* 要顯示【拒絕交易XI】 */
					return (inRetVal);
				}

				/* Installment Payment(每期金額)，右靠左補0 */
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
				        		memcpy(&pobTran->srBRec.szRespCode[0], "I0", 2);
							pobTran->srBRec.szRespCode[2] = 0x00; /* 要顯示【分期電文錯誤】 */
							return (inRetVal);
						}
					}

					pobTran->srBRec.lnInstallmentPayment = atol(szTemplate);
					inCnt += 8;
				}
				else
				{
					memcpy(&pobTran->srBRec.szRespCode[0], "XI", 2);
					pobTran->srBRec.szRespCode[2] = 0x00; /* 要顯示【拒絕交易XI】 */
					return (inRetVal);
				}

				/* Formality Fee(手續費)，右靠左補0 */
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
				        		memcpy(&pobTran->srBRec.szRespCode[0], "I0", 2);
							pobTran->srBRec.szRespCode[2] = 0x00; /* 要顯示【分期電文錯誤】 */
							return (inRetVal);
						}
					}

					pobTran->srBRec.lnInstallmentFormalityFee = atol(szTemplate);
					inCnt += 6;
				}
				else
				{
					memcpy(&pobTran->srBRec.szRespCode[0], "XI", 2);
					pobTran->srBRec.szRespCode[2] = 0x00; /* 要顯示【拒絕交易XI】 */
					return (inRetVal);
				}

				/* Response Code 同原分期交易主機回覆值。“00”=Approved */
				inCnt += 2;
				
				/* 代表有回傳Table ID "N4", for 修改分期及紅利交易防呆機制 */
				pobTran->uszN4TableBit = VS_TRUE;
			}
			else if (!memcmp(&uszUnPackBuf[inCnt], "N5", 2))
			{
				/* ●	Table ID “N5”: Redeem Relation Data(紅利扣抵資料) */
				inCnt += 2; /* Table ID */
				/* Sub Total Length */
				inSubTotalLength = ((uszUnPackBuf[inCnt] % 16) * 100) + ((uszUnPackBuf[inCnt + 1] / 16) * 10) + (uszUnPackBuf[inCnt + 1] % 16);
				inCnt += 2;
				/*
					Redeem Indicator (紅利標記)
					‘1’=全額扣抵
					‘2’=部分扣抵
					‘3’=CLM全額扣抵
					‘4’=CLM部分扣抵
					‘6’= CLM部分扣抵，但不清算
					請求電文預設值為’1’。
				*/
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], (char *)&uszUnPackBuf[inCnt], 1);
				if (!memcmp(&szTemplate[0], "1", 1) || !memcmp(&szTemplate[0], "2", 1))
				{
					memset(pobTran->srBRec.szRedeemIndicator, 0x00, sizeof(pobTran->srBRec.szRedeemIndicator));
					memcpy(&pobTran->srBRec.szRedeemIndicator, (char *)&uszUnPackBuf[inCnt], 1);
					inCnt ++;
				}
				else
				{
					memcpy(&pobTran->srBRec.szRespCode[0], "XI", 2);
					pobTran->srBRec.szRespCode[2] = 0x00; /* 要顯示【拒絕交易XI】 */
					return (inRetVal);
				}

				/* Point of Redemption (紅利扣抵點數)，右靠左補0。 */
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
				        		memcpy(&pobTran->srBRec.szRespCode[0], "R0", 2);
							pobTran->srBRec.szRespCode[2] = 0x00; /* 要顯示【紅利電文錯誤】 */
							return (inRetVal);
						}
					}
					pobTran->srBRec.lnRedemptionPoints = atol(szTemplate);

					inCnt += 8;
				}
				else
				{
					memcpy(&pobTran->srBRec.szRespCode[0], "XI", 2);
					pobTran->srBRec.szRespCode[2] = 0x00; /* 要顯示【拒絕交易XI】 */
					return (inRetVal);
				}

				/*
					Sign of Balance(剩餘點數之正負值)
					‘-‘，表負值 ‘ ’，表正值
				*/
				memset(pobTran->srBRec.szRedeemSignOfBalance, 0x00, sizeof(pobTran->srBRec.szRedeemSignOfBalance));
				memcpy(&pobTran->srBRec.szRedeemSignOfBalance[0], (char *)&uszUnPackBuf[inCnt], 1);
				inCnt ++;
				/* Point of Balance(紅利剩餘點數)，右靠左補0。 */
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
				        		memcpy(&pobTran->srBRec.szRespCode[0], "R0", 2);
							pobTran->srBRec.szRespCode[2] = 0x00; /* 要顯示【紅利電文錯誤】 */
							return (inRetVal);
						}
					}

					pobTran->srBRec.lnRedemptionPointsBalance = atol(szTemplate);
					inCnt += 8;
				}
				else
				{
					memcpy(&pobTran->srBRec.szRespCode[0], "XI", 2);
					pobTran->srBRec.szRespCode[2] = 0x00; /* 要顯示【拒絕交易XI】 */
					return (inRetVal);
				}

				/* Paid Credit Amount(紅利支付金額)，右靠左補0。 */
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
				        		memcpy(&pobTran->srBRec.szRespCode[0], "R0", 2);
							pobTran->srBRec.szRespCode[2] = 0x00; /* 要顯示【紅利電文錯誤】 */
							return (inRetVal);
						}
					}

					/* 2014/12/1 下午 03:26:52 修正小費重複加的問題 */
					if (pobTran->srBRec.lnTipTxnAmount == 0L)
						pobTran->srBRec.lnRedemptionPaidCreditAmount = atol(szTemplate);
					else
						pobTran->srBRec.lnRedemptionPaidCreditAmount = (atol(szTemplate) - pobTran->srBRec.lnTipTxnAmount);

					inCnt += 8;
				}
				else
				{
					memcpy(&pobTran->srBRec.szRespCode[0], "XI", 2);
					pobTran->srBRec.szRespCode[2] = 0x00; /* 要顯示【拒絕交易XI】 */
					return (inRetVal);
				}

				/* Response Code 同原紅利交易主機回覆值。“00”=Approved */
				inCnt += 2;
				
				/* 代表有回傳Table ID "N5", for 修改分期及紅利交易防呆機制 */
				pobTran->uszN5TableBit = VS_TRUE;
			}
			else if (!memcmp(&uszUnPackBuf[inCnt], "N6", 2))
			{
				/* ●	Table ID “N6”: DCC Change to Pay NTD(DCC轉台幣支付) */
				inCnt += 2; /* Table ID */
				/* Sub Total Length */
				inSubTotalLength = ((uszUnPackBuf[inCnt] % 16) * 100) + ((uszUnPackBuf[inCnt + 1] / 16) * 10) + (uszUnPackBuf[inCnt + 1] % 16);
				inCnt += 2;
				inCnt += inSubTotalLength;
			}
			else if (!memcmp(&uszUnPackBuf[inCnt], "N7", 2))
			{
				/* ●	Table ID “N7”: Preauth Complete Relation Data (預授權完成資料) */
				inCnt += 2; /* Table ID */
				/* Sub Total Length */
				inSubTotalLength = ((uszUnPackBuf[inCnt] % 16) * 100) + ((uszUnPackBuf[inCnt + 1] / 16) * 10) + (uszUnPackBuf[inCnt + 1] % 16);
				inCnt += 2;
				/* Original Transaction Date(MMDD) */
				memset(pobTran->srBRec.szCUP_TD, 0x00, sizeof(pobTran->srBRec.szCUP_TD));
				memcpy(&pobTran->srBRec.szCUP_TD[0], &uszUnPackBuf[inCnt], 4);
				inCnt += 4;
				/* Original Amount Right justified and zeros filled on the left with 2 decimals. No decimal point. */
				inCnt += 12;
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
			else if (!memcmp(&uszUnPackBuf[inCnt], "N9", 2))
			{
				/* ●	Table ID “N9”: Mail/Telephone Order Relation Data */
				inCnt += 2; /* Table ID */
				/* Sub Total Length */
				inSubTotalLength = ((uszUnPackBuf[inCnt] % 16) * 100) + ((uszUnPackBuf[inCnt + 1] / 16) * 10) + (uszUnPackBuf[inCnt + 1] % 16);
				inCnt += 2;
				inCnt += inSubTotalLength;
			}
			else if (!memcmp(&uszUnPackBuf[inCnt], "NA", 2))
			{
				/* ●	Table ID “NA”: Call Bank Referral Telephone no. */
				inCnt += 2; /* Table ID */
				/* Sub Total Length */
				inSubTotalLength = ((uszUnPackBuf[inCnt] % 16) * 100) + ((uszUnPackBuf[inCnt + 1] / 16) * 10) + (uszUnPackBuf[inCnt + 1] % 16);
				inCnt += 2;
				/*
					含區號的電話號碼。(左靠右補空白)
					3碼區域號+12碼電話號碼
					例：
					台北〈0227191919〉
					端末機依TMS下載的第三支電話中區域碼判斷是否需要 將Referral Telephone no.前的區碼過濾掉
					Table ID”NA”(Call Bank Referral Telephone no.)下載的電話號碼中間不會 有空白或’-’
				*/
				memset(pobTran->szReferralPhoneNum, 0x00, sizeof(pobTran->szReferralPhoneNum));
				memcpy(&pobTran->szReferralPhoneNum[0], &uszUnPackBuf[inCnt], 15);
				inCnt += 15;
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
					(3) EW不管是否為MCP(行動支付)皆要回傳發卡行金融機構代碼。
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
					if (pobTran->srBRec.uszMobilePayBit == VS_TRUE)
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
					if (pobTran->srBRec.uszMobilePayBit == VS_TRUE)
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
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memcpy(szTemplate, &pobTran->srBRec.szMCP_BANKID[5], 2);
					inCardLabelLen = atoi(szTemplate);
					inCnt += 2;
					
					/* 卡別名稱 只有國內自有品牌卡片需要回覆此欄位 */
					if ((pobTran->srBRec.uszUCARDTransBit == VS_TRUE) && (inCardLabelLen > 0))
					{
						memset(pobTran->srBRec.szCardLabel, 0x00, sizeof(pobTran->srBRec.szCardLabel));
						memcpy(&pobTran->srBRec.szCardLabel[0], &pobTran->srBRec.szMCP_BANKID[7], inCardLabelLen);
					}

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
			/* 免簽名需求《依照EW、MFES授權主機回覆交易電文的Indicator判斷帳單是否列印免簽名》 add by LingHsiung 2015-07-20 上午 11:36:30 */
			/*
				註：
					端末機支援Table ID “NQ”後，一併移除原端末機Table ID“NI”
					之功能及其端末機依TMS參數判斷免簽名之邏輯。
			*/
			else if (!memcmp(&uszUnPackBuf[inCnt], "NQ", 2))
			{
				/* Table ID “NQ”: EW Quick Pay (快速支付免簽名) Flag */
				inCnt += 2; /* Table ID */
				/* Sub Total Length */
				inSubTotalLength = ((uszUnPackBuf[inCnt] % 16) * 100) + ((uszUnPackBuf[inCnt + 1] / 16) * 10) + (uszUnPackBuf[inCnt + 1] % 16);
				inCnt += 2;
				/*
					EW 主機判斷免簽名邏輯
					‘Y’=符合Quick Pay本交易可以免簽名。
					‘N’=不符合Quick Pay本交易需要簽名。(要印簽名欄)
				*/
				/* add by LingHsiung 2015-07-20 上午 11:06:20 判斷主機回'Y' 簽單直接印免簽名 */
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
					pobTran->srBRec.uszNoSignatureBit = VS_TRUE;
				}

				inCnt ++;
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
			else if (!memcmp(&uszUnPackBuf[inCnt], "NP", 2))
			{
				/* ●	Table ID “NP”: Terminal Type (Only for miniPOS) */
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
			else if (!memcmp(&uszUnPackBuf[inCnt], "NT", 2))
			{
				/*
					●	Table ID 'NT': TMS 排程資訊
					補充說明：結帳成功時，EW才需回覆Table ID 'NT'
					(即Process Code=920000或960000平帳(Response code =00)時，
					EW才回覆Table ID 'NT',若不平帳(Response code =95)，
					則不需回覆Table ID 'NT')。
				*/
				inCnt += 2;			/* Table ID */
				/* Sub Total Length */
				inSubTotalLength = ((uszUnPackBuf[inCnt] % 16) * 100) + ((uszUnPackBuf[inCnt + 1] / 16) * 10) + (uszUnPackBuf[inCnt + 1] % 16);
				inCnt += 2;

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
					{
						if (ginDebug == VS_TRUE)
						{
							inLogPrintf(AT, "inNCCC_EW_UnPack59()_ERROR"); /* 排程下載 SCHEDULE，不要影響結帳交易進行 */
						}
						
					}
					
				}
				
			}
			else if (!memcmp(&uszUnPackBuf[inCnt], "NY", 2))
			{
				/* ●	Table ID 'NY': 身分證字號  */
				inCnt += 2; /* Table ID */
				/* Sub Total Length */
				inSubTotalLength = ((uszUnPackBuf[inCnt] % 16) * 100) + ((uszUnPackBuf[inCnt + 1] / 16) * 10) + (uszUnPackBuf[inCnt + 1] % 16);
				inCnt += 2;
				inCnt += inSubTotalLength;
			}
			else if (!memcmp(&uszUnPackBuf[inCnt], "NZ", 2))
			{
				/* ●	Table ID 'NZ':開卡密碼 */
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
			else if (!memcmp(&uszUnPackBuf[inCnt], "MP", 2))
			{
				/* ● Table ID “MP ”: MPAS Indicator(小額支付標記) */
				/* MPAS(小額支付)端末機注意事項：
					1. 只有MPAS(小額支付)端末機才需上傳Table ID "MP"。
					   (TMS參數 Config2.txt NCCC_FES_Mode=04(MPAS))
					2. EW(MPAS)回覆Table ID "MP" = 'Y'時，視為小額授權交易。
					   晶片卡交易端末機需執行完整EMV流程，但忽略EMV second Generate AC的
					   產生結果(TC\AAC\AAR)，僅判斷ISO8583 Field_39(ARC)為"00"，即授權該筆交易。
					   此筆小額授權交易"免簽名"且端末機不需TC Upload。
					3. EW(MPAS)回覆Table ID "MP" = 'N'時，視為一般信用卡授權交易，
					   端末機執行EMV交易流程不變且須上傳TC Upload。
				*/
				inCnt += 2; /* Table ID */
				/* Sub Total Length */
				inSubTotalLength = ((uszUnPackBuf[inCnt] % 16) * 100) + ((uszUnPackBuf[inCnt + 1] / 16) * 10) + (uszUnPackBuf[inCnt + 1] % 16);
				inCnt += 2;

				if (!memcmp(&uszUnPackBuf[inCnt], "Y", 1))
				{
					pobTran->srBRec.uszMPASTransBit = VS_TRUE;
					pobTran->srBRec.uszNoSignatureBit = VS_TRUE;
				}
				else
				{
					pobTran->srBRec.uszMPASTransBit = VS_FALSE;
				}
                                
                                /* 小於CVM不出簽單 */
                                if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)     ||
                                    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_))
                                {
                                        if (pobTran->lnCVM_CheckAmt > 0)
                                        {
                                                if (ginDebug == VS_TRUE)
                                                {
                                                        inLogPrintf(AT, "Unpack_CVM_CheckAmt");
                                                }
                                                
                                                if (pobTran->lnCVM_CheckAmt > pobTran->srEWRec.lnTxnAmount)  
                                                {
                                                        pobTran->srBRec.uszMPASTransBit = VS_TRUE;
                                                }    
                                        }
                                }
                                
				inCnt ++;
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
			else if (!memcmp(&uszUnPackBuf[inCnt], "UN", 2))
			{
				inCnt += 2; /* Table ID */
				/* Sub Total Length */
				inSubTotalLength = ((uszUnPackBuf[inCnt] % 16) * 100) + ((uszUnPackBuf[inCnt + 1] / 16) * 10) + (uszUnPackBuf[inCnt + 1] % 16);
				inCnt += 2;
				
				/* UNY 實體掃碼交易碼（由 EW 編製唯一值回覆）。（註1) */
				memcpy(pobTran->srBRec.szUnyTransCode, &uszUnPackBuf[inCnt], 20);
				inCnt += 20;
				
				/* 遮掩交易卡號資料，回覆前6 後 4 ，中間以 A 字進行遮掩，左靠右補空白。 */
				memcpy(pobTran->srBRec.szUnyMaskedCardNo, &uszUnPackBuf[inCnt], 20);
				for (i = 0; i < strlen(pobTran->srBRec.szUnyMaskedCardNo); i++)
				{
					if (pobTran->srBRec.szUnyMaskedCardNo[i] == 'A')
					{
						pobTran->srBRec.szPAN[i] = '*';
					}
					else
					{
						pobTran->srBRec.szPAN[i] = pobTran->srBRec.szUnyMaskedCardNo[i];
					}
				}
				inFunc_DiscardSpace(pobTran->srBRec.szPAN);
				inCnt += 20;
				
				/* 簽單檢查碼 */
				memcpy(pobTran->srBRec.szCheckNO, &uszUnPackBuf[inCnt], 5);
				inCnt += 5;
				
				/* 電子發票HASH 值 */
				memcpy(pobTran->srBRec.szEInvoiceHASH, &uszUnPackBuf[inCnt], 44);
				inCnt += 44;
				
				/* 卡別名稱長度。 */
				memcpy(pobTran->srBRec.szUnyCardLabelLen, &uszUnPackBuf[inCnt], 2);
				inUnyCardLabelLen = atoi(pobTran->srBRec.szUnyCardLabelLen);
				inCnt += 2;
				
				/* 卡別名稱。 */
				memcpy(pobTran->srBRec.szUnyCardLabel, &uszUnPackBuf[inCnt], inUnyCardLabelLen);
				memcpy(pobTran->srBRec.szCardLabel, pobTran->srBRec.szUnyCardLabel, inUnyCardLabelLen);
				inCnt += inUnyCardLabelLen;
				
				/* 比對卡別名稱，若找不到要存 */
				inCDTXIndex = -1;
				if (inNCCC_Func_Check_Card_Label_Is_Already_Have(pobTran->srBRec.szCardLabel, &inCDTXIndex) != VS_TRUE)
				{
					inLoadCDTXRec(inCDTXIndex);
					inSetUnknownCardLabel(pobTran->srBRec.szCardLabel);
					inSaveCDTXRec(inCDTXIndex);
				}
			}
			else if (!memcmp(&uszUnPackBuf[inCnt], "TP", 2))
			{
				/* ●	Table ID 'TP':Terminal Parameter Data */
				inCnt += 2; /* Table ID */
				/* Sub Total Length */
				inSubTotalLength = ((uszUnPackBuf[inCnt] % 16) * 100) + ((uszUnPackBuf[inCnt + 1] / 16) * 10) + (uszUnPackBuf[inCnt + 1] % 16);
				inCnt += 2;
				/* 轉存到template處理 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszUnPackBuf[inCnt], inSubTotalLength);
				inCnt += inSubTotalLength;
				
				inTPCnt = 0;
				/* 由EW查核的才處理 */
				if (szTemplate[inTPCnt] == 'A')
				{
					/* CARD BIN Check Indicator ans 1 */
					inTPCnt += 1;
					/* 商店自存聯卡號遮掩標記 ans 1 */
					if (szTemplate[inTPCnt] == 'Y')
						pobTran->srBRec.uszTxNoCheckBit = VS_TRUE;
					else
						pobTran->srBRec.uszTxNoCheckBit = VS_FALSE;
					
					inTPCnt += 1;
					/* 保留欄位 ans 20 */
					inTPCnt += 20;
					/* 卡別名稱長度 ans 2 */
					inCardLabelLen = 0;
					memset(szTemplate2, 0x00, sizeof(szTemplate2));
					memcpy(&szTemplate2[0], &szTemplate[inTPCnt], 2);
					inCardLabelLen = atoi(szTemplate2);
					inTPCnt += 2;
					/* 卡別名稱 Max. ans 20 */
					memset(pobTran->srBRec.szCardLabel, 0x00, sizeof(pobTran->srBRec.szCardLabel));
					memcpy(&pobTran->srBRec.szCardLabel[0], &szTemplate[inTPCnt], inCardLabelLen);
					inTPCnt += inCardLabelLen;
					
					/* 新增未知卡別小計 add by LingHsiung 2020/7/7 下午 2:26 */
					/* 先比對卡別名稱 */
					inCDTXIndex = -1;
					if (inNCCC_Func_Check_Card_Label_Is_Already_Have(pobTran->srBRec.szCardLabel, &inCDTXIndex) != VS_TRUE)
					{
						
						inLoadCDTXRec(inCDTXIndex);
						inSetUnknownCardLabel(pobTran->srBRec.szCardLabel);
						inSaveCDTXRec(inCDTXIndex);
					}
				}
			}
			else if (!memcmp(&uszUnPackBuf[inCnt], "WS", 2))
			{
				/*
					●	Table ID " 電子錢包交易 (e-Wallet Message - Sale)
				*/
				inCnt += 2; /* Table ID */
				/* Sub Total Length */
				inSubTotalLength = ((uszUnPackBuf[inCnt] % 16) * 100) + ((uszUnPackBuf[inCnt + 1] / 16) * 10) + (uszUnPackBuf[inCnt + 1] % 16);
				inCnt += 2;
				
				/* 特店訂單編號 */
				inTempLen = 30;
				inCnt += inTempLen;
				
				/* 一般交易金額 （不含小數位）。右靠左補 0 */
				inTempLen = 10;
				inCnt += inTempLen;
				
				/* 代收交易金額 （不含小數位）。右靠左補 0 */
				inTempLen = 10;
				inCnt += inTempLen;
				
				/* 代售交易金額 （不含小數位）。右靠左補0 */
				inTempLen = 10;
				inCnt += inTempLen;
				
				/* 不可折抵金額 （不含小數位）。右靠左補0 */
				inTempLen = 10;
				inCnt += inTempLen;
				
				/* 錢包業者交易序號 。（左靠右補空白）*/
				inTempLen = 64;
				memset(pobTran->srEWRec.szEW_TransactionId, 0x00, sizeof(pobTran->srEWRec.szEW_TransactionId));
				memcpy(pobTran->srEWRec.szEW_TransactionId, &uszUnPackBuf[inCnt], inTempLen);
				inCnt += inTempLen;
				
				/* 電子錢包業者代碼 */
				inTempLen = 2;
				memset(pobTran->srEWRec.szEW_EWIssuerID, 0x00, sizeof(pobTran->srEWRec.szEW_EWIssuerID));
				memcpy(pobTran->srEWRec.szEW_EWIssuerID, &uszUnPackBuf[inCnt], inTempLen);
				inCnt += inTempLen;
				/* 錢包業者特店代號 (預設由 ATS抓主檔回覆 )。（左靠右補空白）*/
				inTempLen = 50;
				memset(pobTran->srEWRec.szEW_ChannelId, 0x00, sizeof(pobTran->srEWRec.szEW_ChannelId));
				memcpy(pobTran->srEWRec.szEW_ChannelId, &uszUnPackBuf[inCnt], inTempLen);
				inCnt += inTempLen;
				/* 消費者支付工具 ATS判斷後回覆給 EDC*/
				inTempLen = 1;
				memset(pobTran->srEWRec.szEW_PayMethod, 0x00, sizeof(pobTran->srEWRec.szEW_PayMethod));
				memcpy(pobTran->srEWRec.szEW_PayMethod, &uszUnPackBuf[inCnt], inTempLen);
				inCnt += inTempLen;
				/* 錢包業者交易日期時間 。（左靠右補空白）*/
				inTempLen = 24;
				memset(pobTran->srEWRec.szEW_PaymentDate, 0x00, sizeof(pobTran->srEWRec.szEW_PaymentDate));
				memcpy(pobTran->srEWRec.szEW_PaymentDate, &uszUnPackBuf[inCnt], inTempLen);
				inCnt += inTempLen;
				/* 電子發票 HASH值 。（左靠右補空白）*/
				inTempLen = 50;
				memset(pobTran->srEWRec.szEW_EinvoiceHASH, 0x00, sizeof(pobTran->srEWRec.szEW_EinvoiceHASH));
				memcpy(pobTran->srEWRec.szEW_EinvoiceHASH, &uszUnPackBuf[inCnt], inTempLen);
				inCnt += inTempLen;
				/* 折抵金額 （不含小數位）。（右靠左補0、 不適用業者 全帶 0）*/
				inTempLen = 10;
				memset(pobTran->srEWRec.szEW_DiscountAmt, 0x00, sizeof(pobTran->srEWRec.szEW_DiscountAmt));
				memcpy(pobTran->srEWRec.szEW_DiscountAmt, &uszUnPackBuf[inCnt], inTempLen);
				inCnt += inTempLen;
				/* 遮罩帳號 /卡號 。（左靠右補空白、不適用業者 全 帶空白 space）*/
				inTempLen = 19;
				memset(pobTran->srEWRec.szEW_MaskCreditCardNo, 0x00, sizeof(pobTran->srEWRec.szEW_MaskCreditCardNo));
				memcpy(pobTran->srEWRec.szEW_MaskCreditCardNo, &uszUnPackBuf[inCnt], inTempLen);
				inCnt += inTempLen;
				/* 付款交易編號 。（左靠右補空白、不適用業者 全 帶空白 space）*/
				inTempLen = 30;
				memset(pobTran->srEWRec.szEW_PaymentNo, 0x00, sizeof(pobTran->srEWRec.szEW_PaymentNo));
				memcpy(pobTran->srEWRec.szEW_PaymentNo, &uszUnPackBuf[inCnt], inTempLen);
				inCnt += inTempLen;
				/* 重複付款請求 。 Y/N 。（不適用業者 全帶空白 space）*/
				inTempLen = 1;
				memset(pobTran->srEWRec.szEW_IsDuplicatedPaymentRequest, 0x00, sizeof(pobTran->srEWRec.szEW_IsDuplicatedPaymentRequest));
				memcpy(pobTran->srEWRec.szEW_IsDuplicatedPaymentRequest, &uszUnPackBuf[inCnt], inTempLen);
				inCnt += inTempLen;
				/* 聯名卡代碼 。（左靠右補空白、不適用業者 全 帶空白 space）*/
				inTempLen = 11;
				memset(pobTran->srEWRec.szEW_CobrandedCode, 0x00, sizeof(pobTran->srEWRec.szEW_CobrandedCode));
				memcpy(pobTran->srEWRec.szEW_CobrandedCode, &uszUnPackBuf[inCnt], inTempLen);
				inCnt += inTempLen;
				/* 門市代碼 。（左靠右補空白、不適用業者 全 帶空白 space）*/
				inTempLen = 24;
				memset(pobTran->srEWRec.szEW_Store_no, 0x00, sizeof(pobTran->srEWRec.szEW_Store_no));
				memcpy(pobTran->srEWRec.szEW_Store_no, &uszUnPackBuf[inCnt], inTempLen);
				inCnt += inTempLen;
			}
			else if (!memcmp(&uszUnPackBuf[inCnt], "WR", 2))
			{
				/*
					●	Table ID " 電子錢包交易 (e-Wallet Message - Refund
				*/
				inCnt += 2; /* Table ID */
				/* Sub Total Length */
				inSubTotalLength = ((uszUnPackBuf[inCnt] % 16) * 100) + ((uszUnPackBuf[inCnt + 1] / 16) * 10) + (uszUnPackBuf[inCnt + 1] % 16);
				inCnt += 2;
				
				/* 特店訂單編號 */
				inTempLen = 30;
				inCnt += inTempLen;
				
				/* 錢包業者交易序號 。（左靠右補空白）*/
				inTempLen = 64;
				inCnt += inTempLen;
				
				/* 退貨訂單編號 。（左靠右補空白、 不適用業者 全 帶空白 space）*/
				inTempLen = 30;
				memset(pobTran->srEWRec.szEW_RefundTradeNo, 0x00, sizeof(pobTran->srEWRec.szEW_RefundTradeNo));
				memcpy(pobTran->srEWRec.szEW_RefundTradeNo, &uszUnPackBuf[inCnt], inTempLen);
				inCnt += inTempLen;
				
				/* 一般退貨金額 （不含小數位）。（右靠左補0、 不適用業者全帶 0 */
				inTempLen = 10;
				inCnt += inTempLen;
				
				/* 代收退貨金額 （不含小數位）。（右靠左補0、 不適用業者全帶 0 */
				inTempLen = 10;
				inCnt += inTempLen;
				
				/* 代售退貨金額 （不含小數位）。（右靠左補0、 不適用業者全帶 0 */
				inTempLen = 10;
				inCnt += inTempLen;
				
				/* 電子錢包業者代碼 */
				inTempLen = 2;
				memset(pobTran->srEWRec.szEW_EWIssuerID, 0x00, sizeof(pobTran->srEWRec.szEW_EWIssuerID));
				memcpy(pobTran->srEWRec.szEW_EWIssuerID, &uszUnPackBuf[inCnt], inTempLen);
				inCnt += inTempLen;
				/* 錢包業者特店代號 (預設由 ATS抓主檔回覆 )。（左靠右補空白）*/
				inTempLen = 50;
				memset(pobTran->srEWRec.szEW_ChannelId, 0x00, sizeof(pobTran->srEWRec.szEW_ChannelId));
				memcpy(pobTran->srEWRec.szEW_ChannelId, &uszUnPackBuf[inCnt], inTempLen);
				inCnt += inTempLen;
				/* 錢包業者交易日期時間 。（左靠右補空白）*/
				inTempLen = 24;
				memset(pobTran->srEWRec.szEW_PaymentDate, 0x00, sizeof(pobTran->srEWRec.szEW_PaymentDate));
				memcpy(pobTran->srEWRec.szEW_PaymentDate, &uszUnPackBuf[inCnt], inTempLen);
				inCnt += inTempLen;
				/* 折抵金額 （不含小數位）。（右靠左補0、 不適用業者 全帶 0）*/
				inTempLen = 10;
				memset(pobTran->srEWRec.szEW_DiscountAmt, 0x00, sizeof(pobTran->srEWRec.szEW_DiscountAmt));
				memcpy(pobTran->srEWRec.szEW_DiscountAmt, &uszUnPackBuf[inCnt], inTempLen);
				inCnt += inTempLen;
				/* 門市代碼 。（左靠右補空白、不適用業者 全 帶空白 space）*/
				inTempLen = 24;
				memset(pobTran->srEWRec.szEW_Store_no, 0x00, sizeof(pobTran->srEWRec.szEW_Store_no));
				memcpy(pobTran->srEWRec.szEW_Store_no, &uszUnPackBuf[inCnt], inTempLen);
				inCnt += inTempLen;
			}
			else if (!memcmp(&uszUnPackBuf[inCnt], "WQ", 2))
			{
				/*
					●	Table ID " 電子錢包交易 (e-Wallet Message - Query
				*/
				inCnt += 2; /* Table ID */
				/* Sub Total Length */
				inSubTotalLength = ((uszUnPackBuf[inCnt] % 16) * 100) + ((uszUnPackBuf[inCnt + 1] / 16) * 10) + (uszUnPackBuf[inCnt + 1] % 16);
				inCnt += 2;
				
				/* 特店訂單編號 */
				inTempLen = 30;
				inCnt += inTempLen;
				
				/* 錢包業者交易序號 。（左靠右補空白）*/
				inTempLen = 64;
				memset(pobTran->srEWRec.szEW_TransactionId, 0x00, sizeof(pobTran->srEWRec.szEW_TransactionId));
				memcpy(pobTran->srEWRec.szEW_TransactionId, &uszUnPackBuf[inCnt], inTempLen);
				inCnt += inTempLen;
				
				/* 訂單狀態 。 */
				inTempLen = 2;
				memset(pobTran->srEWRec.szEW_TransactionType, 0x00, sizeof(pobTran->srEWRec.szEW_TransactionType));
				memcpy(pobTran->srEWRec.szEW_TransactionType, &uszUnPackBuf[inCnt], inTempLen);
				inCnt += inTempLen;
				
				/* 錢包業者特店代號 (預設由 ATS抓主檔回覆 )。（左靠右補空白）*/
				inTempLen = 50;
				memset(pobTran->srEWRec.szEW_ChannelId, 0x00, sizeof(pobTran->srEWRec.szEW_ChannelId));
				memcpy(pobTran->srEWRec.szEW_ChannelId, &uszUnPackBuf[inCnt], inTempLen);
				inCnt += inTempLen;
				
				/* 錢包業者交易日期時間 。（左靠右補空白）*/
				inTempLen = 24;
				memset(pobTran->srEWRec.szEW_PaymentDate, 0x00, sizeof(pobTran->srEWRec.szEW_PaymentDate));
				memcpy(pobTran->srEWRec.szEW_PaymentDate, &uszUnPackBuf[inCnt], inTempLen);
				inCnt += inTempLen;
				
				/* 消費者支付工具 ATS判斷後回覆給 EDC*/
				inTempLen = 1;
				memset(pobTran->srEWRec.szEW_PayMethod, 0x00, sizeof(pobTran->srEWRec.szEW_PayMethod));
				memcpy(pobTran->srEWRec.szEW_PayMethod, &uszUnPackBuf[inCnt], inTempLen);
				inCnt += inTempLen;
				
				/* 電子錢包業者代碼 */
				inTempLen = 2;
				memset(pobTran->srEWRec.szEW_EWIssuerID, 0x00, sizeof(pobTran->srEWRec.szEW_EWIssuerID));
				memcpy(pobTran->srEWRec.szEW_EWIssuerID, &uszUnPackBuf[inCnt], inTempLen);
				inCnt += inTempLen;
				
				/* 折抵金額 （不含小數位）。（右靠左補0、 不適用業者 全帶 0）*/
				inTempLen = 10;
				memset(pobTran->srEWRec.szEW_DiscountAmt, 0x00, sizeof(pobTran->srEWRec.szEW_DiscountAmt));
				memcpy(pobTran->srEWRec.szEW_DiscountAmt, &uszUnPackBuf[inCnt], inTempLen);
				inCnt += inTempLen;
				
				/* 電子發票 HASH值 。（左靠右補空白）*/
				inTempLen = 50;
				memset(pobTran->srEWRec.szEW_EinvoiceHASH, 0x00, sizeof(pobTran->srEWRec.szEW_EinvoiceHASH));
				memcpy(pobTran->srEWRec.szEW_EinvoiceHASH, &uszUnPackBuf[inCnt], inTempLen);
				inCnt += inTempLen;
				
				/* 遮罩帳號 /卡號 。（左靠右補空白、不適用業者 全 帶空白 space）*/
				inTempLen = 19;
				memset(pobTran->srEWRec.szEW_MaskCreditCardNo, 0x00, sizeof(pobTran->srEWRec.szEW_MaskCreditCardNo));
				memcpy(pobTran->srEWRec.szEW_MaskCreditCardNo, &uszUnPackBuf[inCnt], inTempLen);
				inCnt += inTempLen;
				
				/* 載具類型 。（左靠右補空白、不適用業者 全 帶空白 space）*/
				inTempLen = 6;
				memset(pobTran->srEWRec.szEW_EinvoiceCarrierType, 0x00, sizeof(pobTran->srEWRec.szEW_EinvoiceCarrierType));
				memcpy(pobTran->srEWRec.szEW_EinvoiceCarrierType, &uszUnPackBuf[inCnt], inTempLen);
				inCnt += inTempLen;
				
				/* 聯名卡代碼 。（左靠右補空白、不適用業者 全 帶空白 space）*/
				inTempLen = 11;
				memset(pobTran->srEWRec.szEW_CobrandedCode, 0x00, sizeof(pobTran->srEWRec.szEW_CobrandedCode));
				memcpy(pobTran->srEWRec.szEW_CobrandedCode, &uszUnPackBuf[inCnt], inTempLen);
				inCnt += inTempLen;
				
				/* 門市代碼 。（左靠右補空白、不適用業者 全 帶空白 space）*/
				inTempLen = 24;
				memset(pobTran->srEWRec.szEW_Store_no, 0x00, sizeof(pobTran->srEWRec.szEW_Store_no));
				memcpy(pobTran->srEWRec.szEW_Store_no, &uszUnPackBuf[inCnt], inTempLen);
				inCnt += inTempLen;
			}
			else
			{
				inCnt ++;
			}
		}
                
                /* 免簽不出簽單 */
                if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)     ||
                    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_))
                {
                        if (pobTran->srBRec.uszMPASTransBit == VS_FALSE && pobTran->srBRec.uszNoSignatureBit == VS_TRUE)
                        {
                                pobTran->srBRec.uszMPASTransBit = VS_TRUE;
                        }    
                }
	}
	else
	{
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_EW_UnPack60
Date&Time       :
Describe        :
*/
int inNCCC_EW_UnPack60(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
	int	inRetVal = VS_SUCCESS;
	int	inCnt = 0;
	char	szTemplate[6 + 1] = {0};

	if (!memcmp(pobTran->srBRec.szRespCode, "A0", 2))
	{
		inRetVal = inNCCC_ATS_TMKKeyExchange(pobTran, uszUnPackBuf);
	}
	else
	{
		if (pobTran->srBRec.inCode == _EW_INQUIRY_)
		{
			inCnt += 2;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &uszUnPackBuf[inCnt], 6);
			pobTran->srBRec.lnBatchNum = atol(szTemplate);
		}
	}

	return (inRetVal);
}

/*
Function        :inNCCC_EW_UnPack62
Date&Time       :
Describe        :
*/
int inNCCC_EW_UnPack62(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
	int	inRetVal = VS_SUCCESS;
	int	inCnt = 0;
	char	szTemplate[6 + 1] = {0};

	if (pobTran->srBRec.inCode == _EW_INQUIRY_)
	{
		inCnt += 2;
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(szTemplate, &uszUnPackBuf[inCnt], 6);
		pobTran->srBRec.lnOrgInvNum = atol(szTemplate);
	}

	return (inRetVal);
}