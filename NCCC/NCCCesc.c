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
#include "../SOURCE/PRINT/Print.h"
#include "../SOURCE/PRINT/PrtMsg.h"
#include "../SOURCE/FUNCTION/Function.h"
#include "../SOURCE/FUNCTION/FuncTable.h"
#include "../SOURCE/FUNCTION/File.h"
#include "../SOURCE/FUNCTION/Sqlite.h"
#include "../SOURCE/FUNCTION/Batch.h"
#include "../SOURCE/EVENT/MenuMsg.h"
#include "../SOURCE/EVENT/Flow.h"
#include "../HG/HGsrc.h"
#include "../SOURCE/FUNCTION/Accum.h"
#include "../SOURCE/FUNCTION/CDT.h"
#include "../SOURCE/FUNCTION/CFGT.h"
#include "../SOURCE/FUNCTION/CPT.h"
#include "../SOURCE/FUNCTION/EDC.h"
#include "../SOURCE/FUNCTION/HDT.h"
#include "../SOURCE/FUNCTION/HDPT.h"
#include "../SOURCE/FUNCTION/MVT.h"
#include "../SOURCE/FUNCTION/Signpad.h"
#include "../SOURCE/FUNCTION/KMS.h"
#include "../SOURCE/FUNCTION/Utility.h"
#include "../SOURCE/COMM/Comm.h"
#include "../CREDIT/Creditfunc.h"
#include "../CREDIT/CreditprtByBuffer.h"
#include "../CTLS/CTLS.h"
#include "../FISC/NCCCfisc.h"
#include "NCCCats.h"
#include "NCCCsrc.h"
#include "NCCCtSAM.h"
#include "NCCCtmk.h"
#include "NCCCescReceipt.h"
#include "NCCCesc.h"
#include "TAKAsrc.h"


int		ginESCHostIndex = -1;			/* ESC HOST INDEX */
long		glnESC_PokaYoke_Amt = 0;		/* 避免再度產生送錯電子簽單的機制 */
long		glnESC_PokaYoke_TestAmt = 0;		/* 測試ESC防呆機制使用金額 */
unsigned char	gusztSAMKeyIndex_ESC = 0x00;
unsigned char	guszNCCC_ESC_ISO_Field03[_NCCC_ESC_PCODE_SIZE_ + 1];
unsigned char	gusztSAMCheckSum_ESC35[4 + 1];
unsigned char	gusztSAMCheckSum_ESC57_E1[4 + 1];
unsigned char	gusztSAMCheckSum_ESC57_E2[4 + 1];
unsigned char	guszFiscBit = VS_FALSE;			/* Fisc要用ascii壓 */
unsigned char	guszTestRedundantESCPacketBit = VS_FALSE;
extern	int	ginDebug;	/* Debug使用 extern */
extern	int	ginISODebug;	/* Debug使用 extern */
extern	int	ginEngineerDebug;
extern	int	ginFindRunTime;
extern	int	ginMachineType;
extern	int	ginAPVersionType;
extern	char	gszTermVersionID[16 + 1];
extern	char	gszTranDBPath[100 + 1];
int		ginESCDebug = VS_FALSE;


/* ESC上傳使用 START */
EscData		ESC_UPLOAD_DATA[_ESC_LIMIT_];			/* 總上傳資料 */
int		ginEscDataIndex;				/* 總上傳次數 */
int		ginEscUploadIndex;				/* 上傳中使用 */
int		ginESC_Idle_flag = _ESC_IDLE_NO_NEED_UPLOAD_;	/* idle畫面是否需要上傳電子簽單 */
int		ginESC_Connect_flag = 0;			/* idle畫面是否與ESC主機連線 一直重連會耗時*/
int		ginESC_Send_Cnt;				/* 電子簽單上傳計數 超過三次不上傳 */

char		gszESC_Date[8 + 1] = "0000000";			/* yyyymmdd */
char		gszESC_Time[6 + 1] = "0000000";			/* hhmmss */
/* ESC上傳使用 END */

ISO_FIELD_NCCC_ESC_TABLE srNCCC_ESC_ISOFieldPack[] =
{
        {3,             inNCCC_ESC_Pack03},
        {4,             inNCCC_ESC_Pack04},
        {11,            inNCCC_ESC_Pack11},
        {12,            inNCCC_ESC_Pack12},
        {13,            inNCCC_ESC_Pack13},
        {22,            inNCCC_ESC_Pack22},
        {24,            inNCCC_ESC_Pack24},
        {25,            inNCCC_ESC_Pack25},
        {35,            inNCCC_ESC_Pack35},
        {37,            inNCCC_ESC_Pack37},
        {38,            inNCCC_ESC_Pack38},
        {41,            inNCCC_ESC_Pack41},
        {42,            inNCCC_ESC_Pack42},
        {48,            inNCCC_ESC_Pack48},
        {59,            inNCCC_ESC_Pack59},
        {60,            inNCCC_ESC_Pack60},
        {62,            inNCCC_ESC_Pack62},
        {0,             NULL},  /* 最後一組一定要放 0 */
};

ISO_FIELD_NCCC_ESC_TABLE srNCCC_ESC_ISOFieldPack_TSAM[] =
{
        {3,             inNCCC_ESC_Pack03},
        {4,             inNCCC_ESC_Pack04},
        {11,            inNCCC_ESC_Pack11},
        {12,            inNCCC_ESC_Pack12},
        {13,            inNCCC_ESC_Pack13},
        {22,            inNCCC_ESC_Pack22},
        {24,            inNCCC_ESC_Pack24},
        {25,            inNCCC_ESC_Pack25},
        {35,            inNCCC_ESC_Pack35_tSAM},
        {37,            inNCCC_ESC_Pack37},
        {38,            inNCCC_ESC_Pack38},
        {41,            inNCCC_ESC_Pack41},
        {42,            inNCCC_ESC_Pack42},
        {48,            inNCCC_ESC_Pack48},
	{57,		inNCCC_ESC_Pack57_tSAM},
        {59,            inNCCC_ESC_Pack59},
        {60,            inNCCC_ESC_Pack60},
        {62,            inNCCC_ESC_Pack62},
        {0,             NULL},  /* 最後一組一定要放 0 */
};

ISO_FIELD_NCCC_ESC_TABLE srNCCC_ESC_ISOFieldUnPack[] =
{
	{4,		inNCCC_ESC_UnPack04},
        {24,            inNCCC_ESC_UnPack24},
        {38,            inNCCC_ESC_UnPack38},
        {39,            inNCCC_ESC_UnPack39},
        {0,             NULL}, /* 最後一組一定要放【0】!! */
};

ISO_CHECK_NCCC_ESC_TABLE srNCCC_ESC_ISOFieldCheck[] =
{
        {3,             inNCCC_ESC_Check03},
        {4,             inNCCC_ESC_Check04},
        {41,            inNCCC_ESC_Check41},
        {0,		NULL}, /* 最後一組一定要放【0】!! */
};

ISO_FIELD_TYPE_NCCC_ESC_TABLE srNCCC_ESC_ISOFieldType[] =
{
        {3,             _NCCC_ESC_ISO_BCD_,         VS_FALSE,       6},
        {4,             _NCCC_ESC_ISO_BCD_,         VS_FALSE,       12},
        {11,            _NCCC_ESC_ISO_BCD_,         VS_FALSE,       6},
        {12,            _NCCC_ESC_ISO_BCD_,         VS_FALSE,       6},
        {13,            _NCCC_ESC_ISO_BCD_,         VS_FALSE,       4},
        {22,            _NCCC_ESC_ISO_BCD_,         VS_FALSE,       4},
        {24,            _NCCC_ESC_ISO_BCD_,         VS_FALSE,       4},
        {25,            _NCCC_ESC_ISO_BCD_,         VS_FALSE,       2},
        {35,            _NCCC_ESC_ISO_NIBBLE_2_,    VS_FALSE,       0},
        {37,            _NCCC_ESC_ISO_ASC_,         VS_FALSE,       12},
        {38,            _NCCC_ESC_ISO_ASC_,         VS_FALSE,       6},
        {39,            _NCCC_ESC_ISO_ASC_,         VS_FALSE,       2},
        {41,            _NCCC_ESC_ISO_ASC_,         VS_FALSE,       8},
        {42,            _NCCC_ESC_ISO_ASC_,         VS_FALSE,       15},
        {48,            _NCCC_ESC_ISO_BYTE_3_,      VS_TRUE,        0},
        {52,            _NCCC_ESC_ISO_BCD_,         VS_FALSE,       16},
        {54,            _NCCC_ESC_ISO_BYTE_3_,      VS_TRUE,        0},
        {55,            _NCCC_ESC_ISO_BYTE_3_,      VS_FALSE,       0},
        {57,            _NCCC_ESC_ISO_BYTE_3_,      VS_FALSE,       0},
        {58,            _NCCC_ESC_ISO_BYTE_3_,      VS_TRUE,        0},
        {59,            _NCCC_ESC_ISO_BYTE_3_,      VS_TRUE,        0},
        {60,            _NCCC_ESC_ISO_BYTE_3_,      VS_TRUE,        0},
        {62,            _NCCC_ESC_ISO_BYTE_3_,      VS_TRUE,        0},
        {63,            _NCCC_ESC_ISO_BYTE_3_,      VS_TRUE,        0},
        {64,            _NCCC_ESC_ISO_BCD_,         VS_FALSE,       16},
        {0,             _NCCC_ESC_ISO_BCD_,         VS_FALSE,       0},  /* 最後一組一定要放 0 */
};

/* ESC-UPLOAD */
int inNCCC_ESC_UPLOAD[] = {3, 4, 11, 12, 13, 22, 24, 25, 35, 37, 38, 41, 42, 59, 60, 62, 0}; /* 最後一組一定要放 0!! */

BIT_MAP_NCCC_ESC_TABLE srNCCC_ESC_ISOBitMap[] =
{
	{_ESC_UPLOAD_,		inNCCC_ESC_UPLOAD,		"0720",		"000000"}, /* 電簽上傳 */
	{_NCCC_ESC_NULL_TX_,	NULL,				"0000",		"000000"}, /* 最後一組一定要放 TMS_NULL_TX!! */
};

ISO_TYPE_NCCC_ESC_TABLE srNCCC_ESC_ISOFunc[] =
{
        {
           srNCCC_ESC_ISOFieldPack,
           srNCCC_ESC_ISOFieldUnPack,
           srNCCC_ESC_ISOFieldCheck,
           srNCCC_ESC_ISOFieldType,
           srNCCC_ESC_ISOBitMap,
           inNCCC_ESC_ISOGetBitMapCode,
           inNCCC_ESC_ISOPackMessageType,
           inNCCC_ESC_ISOModifyBitMap,
           inNCCC_ESC_ISOModifyPackData,
           inNCCC_ESC_ISOCheckHeader,
           inNCCC_ESC_ISOOnlineAnalyse,
           inNCCC_ESC_ISOAdviceAnalyse
        },
	
	{
           srNCCC_ESC_ISOFieldPack_TSAM,
           srNCCC_ESC_ISOFieldUnPack,
           srNCCC_ESC_ISOFieldCheck,
           srNCCC_ESC_ISOFieldType,
           srNCCC_ESC_ISOBitMap,
           inNCCC_ESC_ISOGetBitMapCode,
           inNCCC_ESC_ISOPackMessageType,
           inNCCC_ESC_ISOModifyBitMap,
           inNCCC_ESC_ISOModifyPackData,
           inNCCC_ESC_ISOCheckHeader,
           inNCCC_ESC_ISOOnlineAnalyse,
           inNCCC_ESC_ISOAdviceAnalyse
        },

};

/*
Function        :inNCCC_ESC_Check
Date&Time       :2016/4/29 下午 3:26
Describe        :check是否上傳ESC
*/
int inNCCC_ESC_Check(TRANSACTION_OBJECT *pobTran)
{
	int	inUpload = VS_TRUE;		/* 預設為上傳 */
	int	inAgain = 0, inFail = 0;
	char	szHostEnable[2 + 1];
	char	szTRTFileName[16 + 1];
	char	szESCReciptUploadUpLimit[4 + 1];
	char	szCOMMMode[2 + 1];
	char	szDebugMsg[100 + 1];
	char	szESCMode[2 + 1];
	
        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_Check START!");
        
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
	}
	
	/* 若找不到ESC或ESC沒開或水位為0，不送欄位NE */
	memset(szESCMode, 0x00, sizeof(szESCMode));
	inGetESCMode(szESCMode);
	if (memcmp(szESCMode, "Y", strlen("Y")) != 0)
	{
		pobTran->srBRec.inESCUploadMode = _ESC_STATUS_NOT_SUPPORTED_;
		inUpload = VS_FALSE;
	}
	
	
	/* ESC沒開，不上傳(這個條件已在ESCMode中判斷過，但暫時不拿掉) */
	if (inUpload == VS_TRUE)
	{
		memset(szHostEnable, 0x00, sizeof(szHostEnable));
		if (inNCCC_ESC_GetESC_Enable(pobTran->srBRec.inHDTIndex, szHostEnable) != VS_SUCCESS)
		{
			pobTran->srBRec.inESCUploadMode = _ESC_STATUS_NOT_SUPPORTED_;
			inUpload = VS_FALSE;
		}
		else
		{
			if (szHostEnable[0] != 'Y')
			{
				pobTran->srBRec.inESCUploadMode = _ESC_STATUS_NOT_SUPPORTED_;
				inUpload = VS_FALSE;
			}

		}
		
	}
	
	
	/* 3.電子簽單是否上傳(其他條件寫這，如不支援大來主機) */
	if (inUpload == VS_TRUE)
	{
		memset(szTRTFileName, 0x00, sizeof(szTRTFileName));
		if (inGetTRTFileName(szTRTFileName) != VS_SUCCESS)
		{
			vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_Check inGetTRTFileName failed");
			return (VS_ERROR);
		}
		
		/* 不是NCCC 或 DCC 不上傳 */
		/* HG的點數扣抵若全額扣抵，會切成HG，所以會在那邊(inHG_Func_Online_Redeem_Credit_Flow)設定為ESC不上傳 */
		if (memcmp(szTRTFileName, _TRT_FILE_NAME_CREDIT_, strlen(_TRT_FILE_NAME_CREDIT_)) != 0	&&
		    memcmp(szTRTFileName, _TRT_FILE_NAME_DCC_, strlen(_TRT_FILE_NAME_DCC_)) != 0)
		{
			pobTran->srBRec.inESCUploadMode = _ESC_STATUS_NOT_SUPPORTED_;
			inUpload = VS_FALSE;
		}
		/* 預先授權不上傳，因為不入帳 */
		else if (pobTran->srBRec.inCode == _PRE_AUTH_	||
			(pobTran->srBRec.inCode == _VOID_ && pobTran->srBRec.inOrgCode == _PRE_AUTH_)	||
			 pobTran->srBRec.inCode == _CUP_PRE_AUTH_	||
			 pobTran->srBRec.inCode == _CUP_PRE_AUTH_VOID_)
		{
			pobTran->srBRec.inESCUploadMode = _ESC_STATUS_NOT_SUPPORTED_;
			inUpload = VS_FALSE;
		}
		/* 優惠兌換不上傳 */
		else if (pobTran->srBRec.inCode == _LOYALTY_REDEEM_ || pobTran->srBRec.inCode == _VOID_LOYALTY_REDEEM_)
		{
			pobTran->srBRec.inESCUploadMode = _ESC_STATUS_NOT_SUPPORTED_;
			inUpload = VS_FALSE;
		}
	}
	
	/* 只支援TCP/IP */
	if (inUpload == VS_TRUE)
	{
		memset(szCOMMMode, 0x00, sizeof(szCOMMMode));
		inGetCommMode(szCOMMMode);
		if (memcmp(szCOMMMode, _COMM_ETHERNET_MODE_, 1) != 0	&&
		    memcmp(szCOMMMode, _COMM_GPRS_MODE_, 1) != 0	&&
		    memcmp(szCOMMMode, _COMM_3G_MODE_, 1) != 0		&&
		    memcmp(szCOMMMode, _COMM_4G_MODE_, 1) != 0)
		{
			pobTran->srBRec.inESCUploadMode = _ESC_STATUS_NOT_SUPPORTED_;
			inUpload = VS_FALSE;
		}
	}
	
	/* 水位檢核 */
	if (inUpload == VS_TRUE)
	{
		memset(szESCReciptUploadUpLimit, 0x00, sizeof(szESCReciptUploadUpLimit));
		inGetESCReciptUploadUpLimit(szESCReciptUploadUpLimit);
		
		inAgain = inNCCC_ESC_Again_Table_Count(pobTran);
		inFail = inNCCC_ESC_Fail_Table_Count(pobTran);
		
		/* 若水位為0，也當成不開ESC(這個條件已在ESCMode中判斷過，但暫時不拿掉) */
		if (atoi(szESCReciptUploadUpLimit) == 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "水位為0，不送ESC");
				inLogPrintf(AT, szDebugMsg);
			}
			pobTran->srBRec.inESCUploadMode = _ESC_STATUS_NOT_SUPPORTED_;
			inUpload = VS_FALSE;
		}
		/* 若超過水位 */
		else if ((inAgain + inFail) >= atoi(szESCReciptUploadUpLimit))
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "%d > %d 超過水位", inAgain + inFail, atoi(szESCReciptUploadUpLimit));
				inLogPrintf(AT, szDebugMsg);
			}
			pobTran->srBRec.inESCUploadMode = _ESC_STATUS_OVERLIMIT_;
			inUpload = VS_FALSE;
		}
	}
	
	/* 要上傳都先預設沒簽，等簽之後再改狀態(避免斷電沒簽) */
	if (inUpload == VS_TRUE)
	{
		pobTran->srBRec.inESCUploadMode = _ESC_STATUS_SUPPORTED_;
	}
	
	/* 6.其他 */
        pobTran->srBRec.inESCTransactionCode = pobTran->inTransactionCode;      /* 重新上傳使用 */
	
	if (ginDebug == VS_TRUE)
	{
	        if (pobTran->srBRec.inESCUploadMode == _ESC_STATUS_SUPPORTED_)
	        {
	                inLogPrintf(AT, "此筆上傳電子簽單");
	        }
	        else
	        {
	                inLogPrintf(AT, "此筆不上傳電子簽單");

	                if (pobTran->srBRec.inESCUploadMode == _ESC_STATUS_NOT_SUPPORTED_)
	                        inLogPrintf(AT, "  不支援");
	                else if (pobTran->srBRec.inESCUploadMode == _ESC_STATUS_OVERLIMIT_)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "  %d > %d 超過水位", inAgain + inFail, atoi(szESCReciptUploadUpLimit));
				inLogPrintf(AT, szDebugMsg);
	                        inLogPrintf(AT, "  水位已滿");
			}
			else
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Reason : %d", pobTran->srBRec.inESCUploadMode);
				inLogPrintf(AT, szDebugMsg);
			}
			
	        }
		
	}
	
	if (ginISODebug == VS_TRUE)
	{
	        if (pobTran->srBRec.inESCUploadMode == _ESC_STATUS_SUPPORTED_)
	        {
	                inPRINT_ChineseFont("此筆上傳電子簽單", _PRT_ISO_);
	        }
	        else
	        {
	                inPRINT_ChineseFont("此筆不上傳電子簽單", _PRT_ISO_);

	                if (pobTran->srBRec.inESCUploadMode == _ESC_STATUS_NOT_SUPPORTED_)
	                        inPRINT_ChineseFont("  不支援", _PRT_ISO_);
	                else if (pobTran->srBRec.inESCUploadMode == _ESC_STATUS_OVERLIMIT_)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "  %d > %d 超過水位", inAgain + inFail, atoi(szESCReciptUploadUpLimit));
				inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
	                        inPRINT_ChineseFont("  水位已滿", _PRT_ISO_);
			}
			else
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Reason : %d", pobTran->srBRec.inESCUploadMode);
				inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
			}
			
	        }
		
	}
	
        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_Check END!");
        
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_ESC_MerchantCopy_Check
Date&Time       :2016/4/29 下午 3:27
Describe        :check是否加印商店聯
*/
int inNCCC_ESC_MerchantCopy_Check(TRANSACTION_OBJECT *pobTran)
{
	char		szESCMerchantCopyFlag[2 + 1];
	char		szStartDate[8 + 1];
	char		szEndDate[8 + 1];
	char		szNowDate[8 + 1];
	char		szDebugMsg[100 + 1];
	RTC_NEXSYS	srRTC;

        memset(szESCMerchantCopyFlag , 0x00, sizeof(szESCMerchantCopyFlag));
	inGetESCPrintMerchantCopy(szESCMerchantCopyFlag);
	if (szESCMerchantCopyFlag[0] == 'Y')
	{
		/* 起始日 */
		memset(szStartDate , 0x00, sizeof(szStartDate));
		inGetESCPrintMerchantCopyStartDate(szStartDate);
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "MerchantCopyStartDate = %s", szStartDate);
			inLogPrintf(AT, szDebugMsg);
		}
		
		/* 結束日 */
		memset(szEndDate , 0x00, sizeof(szEndDate));
		inGetESCPrintMerchantCopyEndDate(szEndDate);
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "MerchantCopyEndDate = %s", szEndDate);
			inLogPrintf(AT, szDebugMsg);
		}
		
		/* 現在日期 */
		memset(&srRTC, 0x00, sizeof(RTC_NEXSYS));
		inFunc_GetSystemDateAndTime(&srRTC);
		memset(szNowDate, 0x00, sizeof(szNowDate));
		sprintf(szNowDate, "20%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "NowDate = %s", szNowDate);
			inLogPrintf(AT, szDebugMsg);
		}

		/* 設定值Y，且在起迄日內 */
		/* 用太陽日比較 */
		if (inFunc_SunDay_Sum_Check_In_Range(szNowDate, szStartDate, szEndDate) == VS_SUCCESS)
		{
			return (VS_SUCCESS);
		}
		
	}
	
	return (VS_ERROR);
}

/*
Function        :inNCCC_ESC_Func_Upload
Date&Time       :2016/5/3 下午 2:06
Describe        :當筆上傳ESC
*/
int inNCCC_ESC_Func_Upload(TRANSACTION_OBJECT *pobTran)
{
	int	i = 0;
        int	inRetVal = VS_ERROR;
	char	szESCMode[2 + 1] = {0};
	char	szFileName[16 + 1] = {0};
	char	szDemoMode[2 + 1] = {0};
	unsigned char	uszTipBit = VS_FALSE;
	
        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_Func_Upload START!");
        
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_Func_Upload() START!");
	
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
	}
	
	/* 先確認ESC功能有沒有開 */
	memset(szESCMode, 0x00, sizeof(szESCMode));
	inGetESCMode(szESCMode);
	if (memcmp(&szESCMode[0], "Y", 1) != 0)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "ESC mode is not open");
			inLogPrintf(AT, "inNCCC_ESC_Func_Upload() END");
		}
		vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_Func_Upload END!");
		/* 沒開，就不跑ESC流程 */
		return (VS_SUCCESS);
	}
	if (ginDebug == VS_TRUE)
	{	
		inLogPrintf(AT, "ESC Upload Mode :%d",pobTran->srBRec.inESCUploadMode);
	}
	/* 教育訓練模式 */
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
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
		
		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "inNCCC_ESC_Func_Upload() END!");
		
		/* 當筆符合ESC上傳條件為
		 * 支援上傳且有簽名 _ESC_STATUS_SUPPORTED_
		 * 或者是支援上傳且免簽名 */
		if (pobTran->srBRec.inESCUploadMode == _ESC_STATUS_SUPPORTED_	&&
		   (pobTran->srBRec.inSignStatus == _SIGN_SIGNED_		||
		    pobTran->srBRec.inSignStatus == _SIGN_NO_NEED_SIGN_))
		{
			/* 不分Online/offline 一律上傳 */
			/* 簽單可以先上傳再上傳advice交易 */

			/* 通訊也要切到ESC主機(通訊內會直接loadHDT內的CommIndex會自動切過去) */

			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_ESC_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示<電子簽單上傳> */
			/* 電子簽單連線中 */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_ESC_CONNECTING_, 0, _COORDINATE_Y_LINE_8_4_);

			/* ESC UPLOAD交易	
			* 
			* 1. 水位：參數預設值3筆。
			* 2. ESC上傳失敗：金額末碼為9，則固定ESC上傳失敗。
			*/
			/* 2019/9/27 上午 10:05 
			 * 為符合特店實務需求，原教育訓練版本程式預設開啟電子簽名和電簽上傳功能、水位為3筆之做法，
			 * 調整為依EMS/TMS參數設定EDC是否支援電子簽名、電簽上傳、水位筆數之功能
			 */
			if (pobTran->srBRec.lnTxnAmount % 10 == 9)
			{
			       inRetVal = VS_ERROR;
			}
			else
			{
			       inRetVal = VS_SUCCESS;
			}

			if (inRetVal == VS_ERROR)
			{
				/* 送電文失敗，存AGAIN區 */
				inNCCC_ESC_Insert_Again_Record(pobTran);
			}
			else if (inRetVal == VS_SUCCESS)
			{
				/* 結帳帳條統計 */
				/* 要放在取批次前面，避免被取消覆蓋原交易 */
				inLoadHDTRec(pobTran->srBRec.inHDTIndex);
				inACCUM_UpdateFlow_ESC(pobTran, _ESC_ACCUM_STATUS_UPLOADED_);
					
				/* 標示為已上傳 */
				if (pobTran->srBRec.inCode == _TIP_)
				{
					uszTipBit = VS_TRUE;
				}
				else
				{
					uszTipBit = VS_FALSE;
				}
				
				inBATCH_GetTransRecord_By_Sqlite(pobTran);
				pobTran->srBRec.inESCUploadStatus = _ESC_UPLOAD_STATUS_UPLOADED_;
				if (uszTipBit == VS_TRUE)
				{
					pobTran->srBRec.uszESCTipUploadBit = VS_TRUE;
				}
				else
				{
					pobTran->srBRec.uszESCOrgUploadBit = VS_TRUE;
				}
				inBATCH_Update_ESC_Uploaded_By_Sqlite(pobTran);

				/* 上傳成功 刪除簽名圖檔 */
				/* 藉由TRT_FileName比對來組出bmp的檔名 */
				inLoadHDPTRec(pobTran->srBRec.inHDTIndex);

				memset(szFileName, 0x00, sizeof(szFileName));
				/* 因為用invoice所以不用inFunc_ComposeFileName */
				/* 刪除檔案，檔名:主機+調閱編號+.bmp */
				inFunc_ComposeFileName_InvoiceNumber(pobTran, szFileName, _PICTURE_FILE_EXTENSION_, 6);
				inFILE_Delete((unsigned char*)szFileName);
			}
		
			/* 回到Idle立刻上傳ESC */
			ginESC_Idle_flag = _ESC_IDLE_UPLOAD_;

		}
		/* 在這裡計算 _ESC_STATUS_OVERLIMIT_ 和 _ESC_STATUS_BYPASS_ 筆數 （有出紙本的）*/
		else
		{
			/* 結帳帳條統計Bypass */
			/* 支援但是Bypass */
			/* 支援但沒簽名斷電 */
			if (pobTran->srBRec.inESCUploadMode == _ESC_STATUS_SUPPORTED_	&&
			   (pobTran->srBRec.inSignStatus == _SIGN_BYPASS_	||
			    pobTran->srBRec.inSignStatus == _SIGN_NONE_))
			{
				inACCUM_UpdateFlow_ESC(pobTran, _ESC_ACCUM_STATUS_BYPASS_);
			}
			/* _ESC_STATUS_NOT_SUPPORTED_	不支援也會在這裡*/
			/* _ESC_STATUS_OVERLIMIT_	超過水位算上傳電簽失敗，因為馬上出紙本所以直接加入accum */
			else
			{
				inACCUM_UpdateFlow_ESC(pobTran, _ESC_ACCUM_STATUS_FAIL_);
			}
			
			/* 結帳帳條統計Bypass */
			/* 支援但是Bypass */
			/* 支援但沒簽名斷電 */
			if (pobTran->srBRec.inESCUploadMode == _ESC_STATUS_SUPPORTED_	&&
			   (pobTran->srBRec.inSignStatus == _SIGN_BYPASS_	||
			    pobTran->srBRec.inSignStatus == _SIGN_NONE_))
			{
				/* 原本"NE"送Y的才要送advie */
				/* 組成對應主機ADVICE */
				if (pobTran->srBRec.uszOfflineBit == VS_TRUE)
				{
					/* 因為電文後送所以就不Append，改在原本advice送 */
				}
				else
				{
					/* 當筆出紙本要送advice告訴fes把當筆紙本的flag on 起來 */
					pobTran->srBRec.inESCUploadStatus = _ESC_UPLOAD_STATUS_PAPER_;
					inBATCH_ESC_Save_Advice_Flow(pobTran);
				}
			}
			
			/* 標示為已出紙本 */
			inBATCH_GetTransRecord_By_Sqlite(pobTran);
			pobTran->srBRec.inESCUploadStatus = _ESC_UPLOAD_STATUS_PAPER_;
			inBATCH_Update_ESC_Uploaded_By_Sqlite(pobTran);
		}
		
	}
	else
	{
		/* 當筆符合ESC上傳條件為
		 * 支援上傳且有簽名 _ESC_STATUS_SUPPORTED_
		 * 或者是支援上傳且免簽名 */
		if (pobTran->srBRec.inESCUploadMode == _ESC_STATUS_SUPPORTED_	&&
		   (pobTran->srBRec.inSignStatus == _SIGN_SIGNED_		||
		    pobTran->srBRec.inSignStatus == _SIGN_NO_NEED_SIGN_))
		{

			/* 不分Online/offline 一律上傳 */
			/* 簽單可以先上傳再上傳advice交易 */

			/* 把之前交易的連線先斷掉 */
			inCOMM_End(pobTran);

			/* 切換到ESC HOST */                  
			inNCCC_ESC_SwitchToESC_Host(pobTran->srBRec.inHDTIndex);
			/* 通訊也要切到ESC主機(通訊內會直接loadHDT內的CommIndex會自動切過去) */

			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_ESC_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示<電子簽單上傳> */
			/* 電子簽單連線中 */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_ESC_CONNECTING_, 0, _COORDINATE_Y_LINE_8_4_);
                        /* _COMM_START_ 會執行 .inBegin / .inCheck，
                           .inBegin: load CPT record 
                           .inCheck: set ip/port config and connect socket 
                        */
			inRetVal = inFLOW_RunFunction(pobTran, _COMM_START_);
			
			/* 開始連線 */
			if (inRetVal != VS_SUCCESS)
			{
				/* 通訊失敗‧‧‧‧ */
				
				inLoadHDTRec(pobTran->srBRec.inHDTIndex);
				/* 無法連線，直接就存AGAIN區 */
				inNCCC_ESC_Insert_Again_Record(pobTran);
			}
			else
			{
				/* 連線成功 */
				inRetVal = inNCCC_ESC_ProcessOnline(pobTran, _ESC_UPLOAD_TRADE_);
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "inNCCC_ESC_ProcessOnline() RetVal is %d",inRetVal);
				}
				if (inRetVal == VS_ESCAPE)		        
				{
					/* 回應碼不為"00"，要存Fail區 */
					inNCCC_ESC_Insert_Fail_Record(pobTran);
				}
				else if (inRetVal == VS_ERROR)
				{
					/* 送電文失敗，存AGAIN區 */
					inNCCC_ESC_Insert_Again_Record(pobTran);
				}
				else if (inRetVal == _NCCC_ESC_PACK_ERR_ ||		/* 組電文失敗 */
					inRetVal == _NCCC_ESC_UNPACK_ERR_)		/* 解電文失敗 */
				{
					/* 組解電文失敗，存Fail區 */
					inNCCC_ESC_Insert_Fail_Record(pobTran);
				}
				else if (inRetVal == VS_SUCCESS)
				{
					/* 結帳帳條統計 */
					/* 要放在取批次前面，避免被取消覆蓋原交易 */
					inLoadHDTRec(pobTran->srBRec.inHDTIndex);
					inACCUM_UpdateFlow_ESC(pobTran, _ESC_ACCUM_STATUS_UPLOADED_);
					
					/* 標示為已上傳 */
					if (pobTran->srBRec.inCode == _TIP_)
					{
						uszTipBit = VS_TRUE;
					}
					else
					{
						uszTipBit = VS_FALSE;
					}
					/* 取得該筆Inovice number的 srBRec和srEMVRec 資料 */
					inBATCH_GetTransRecord_By_Sqlite(pobTran);
					pobTran->srBRec.inESCUploadStatus = _ESC_UPLOAD_STATUS_UPLOADED_;
					if (uszTipBit == VS_TRUE)
					{
						pobTran->srBRec.uszESCTipUploadBit = VS_TRUE;
					}
					else
					{
						pobTran->srBRec.uszESCOrgUploadBit = VS_TRUE;
					}
					inBATCH_Update_ESC_Uploaded_By_Sqlite(pobTran);

					/* 上傳成功 刪除簽名圖檔 */
					/* 藉由TRT_FileName比對來組出bmp的檔名 */
					inLoadHDPTRec(pobTran->srBRec.inHDTIndex);

					memset(szFileName, 0x00, sizeof(szFileName));
					/* 因為用invoice所以不用inFunc_ComposeFileName */
					inFunc_ComposeFileName_InvoiceNumber(pobTran, szFileName, _PICTURE_FILE_EXTENSION_, 6);
					inFILE_Delete((unsigned char*)szFileName);
				}

			}
			/* 回到Idle立刻上傳ESC */
			ginESC_Idle_flag = _ESC_IDLE_UPLOAD_;

		}
		/* 在這裡計算 _ESC_STATUS_OVERLIMIT_ 和 _ESC_STATUS_BYPASS_ 筆數 （有出紙本的）*/
		else
		{
			/* 結帳帳條統計Bypass */
			/* 支援但是Bypass */
			/* 支援但沒簽名斷電 */
			if (pobTran->srBRec.inESCUploadMode == _ESC_STATUS_SUPPORTED_	&&
			   (pobTran->srBRec.inSignStatus == _SIGN_BYPASS_	||
			    pobTran->srBRec.inSignStatus == _SIGN_NONE_))
			{
				inACCUM_UpdateFlow_ESC(pobTran, _ESC_ACCUM_STATUS_BYPASS_);
			}
			/* _ESC_STATUS_NOT_SUPPORTED_	不支援也會在這裡*/
			/* _ESC_STATUS_OVERLIMIT_	超過水位算上傳電簽失敗，因為馬上出紙本所以直接加入accum */
			else
			{
				inACCUM_UpdateFlow_ESC(pobTran, _ESC_ACCUM_STATUS_FAIL_);
			}
			
			/* 結帳帳條統計Bypass */
			/* 支援但是Bypass */
			/* 支援但沒簽名斷電 */
			if (pobTran->srBRec.inESCUploadMode == _ESC_STATUS_SUPPORTED_	&&
			   (pobTran->srBRec.inSignStatus == _SIGN_BYPASS_	||
			    pobTran->srBRec.inSignStatus == _SIGN_NONE_))
			{
				/* 原本"NE"送Y的才要送advie */
				/* 組成對應主機ADVICE */
				if (pobTran->srBRec.uszOfflineBit == VS_TRUE)
				{
					/* 因為電文後送所以就不Append，改在原本advice送 */
				}
				else
				{
					/* 當筆出紙本要送advice告訴fes把當筆紙本的flag on 起來 */
					pobTran->srBRec.inESCUploadStatus = _ESC_UPLOAD_STATUS_PAPER_;
					inBATCH_ESC_Save_Advice_Flow(pobTran);
				}
			}
			
			/* 標示為已出紙本 */
			inBATCH_GetTransRecord_By_Sqlite(pobTran);
			pobTran->srBRec.inESCUploadStatus = _ESC_UPLOAD_STATUS_PAPER_;
			inBATCH_Update_ESC_Uploaded_By_Sqlite(pobTran);
		}
	}
	/* 
	 * inNCCC_Func_ESC_FuncProcess()會先記錄ESC暫存檔資料，
	 * 遇到斷電，重開機 會列印簽單?
	*/
	/* 刪除暫存紀錄，代表已完成統計流程 */
	/* ESC暫存檔 */
	pobTran->uszFileNameNoNeedHostBit = VS_TRUE;
	pobTran->uszFileNameNoNeedNumBit = VS_TRUE;
	inRetVal = inSqlite_ESC_Delete_Record_Flow(pobTran, _TN_BATCH_TABLE_ESC_TEMP_);
	pobTran->uszFileNameNoNeedHostBit = VS_FALSE;
	pobTran->uszFileNameNoNeedNumBit = VS_FALSE;
	if (inRetVal != VS_SUCCESS)
	{
		inUtility_StoreTraceLog_OneStep("Delete_ESC_Temp_Fail");
	}
	
	
	if (ginDebug == VS_TRUE)
		inLogPrintf(AT, "inNCCC_ESC_Func_Upload() END!");
        
        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_Func_Upload END!");

	return (VS_SUCCESS);
}

/*
Function        :inNCCC_ESC_Func_Upload_Idle
Date&Time       :2016/5/3 下午 2:06
Describe        :idle上傳ESC
*/
int inNCCC_ESC_Func_Upload_Idle(TRANSACTION_OBJECT *pobTran)
{
	/*[260414] 調整記錄,如果是待機畫面進行,則不在此記錄 */
	if(pobTran->inRunOperationID != _OPERATION_ESC_IDLE_UPLOAD_)
        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_Func_Upload_Idle START!");
        
	char	szDemoMode[2 + 1] = {0};
	/* 教育訓練模式 */
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		ginESC_Idle_flag = _ESC_IDLE_NO_NEED_UPLOAD_;
		return (VS_SUCCESS);
	}
	else
	{
		int		inRetVal;
		int		inHostIndex = -1;
		int		inAgain = 0;						/* Again table中的筆數 */
		int		inTotalAgain = 0;
		char		szTemp[4 + 1];
		char		szHostName[8 + 1];
		char		szESCMode[2 + 1];
		char		szTMSOK[2 + 1];
		unsigned char	uszTipBit = VS_FALSE;
		RTC_NEXSYS	srRTC;

		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "inNCCC_ESC_Func_Upload_Idle() START!");

		if (ginESC_Connect_flag == _ESC_CONNECT_ALREADY_)
		{
			pobTran->uszConnectionBit = VS_TRUE;
		}
		else
		{
			pobTran->uszConnectionBit = VS_FALSE;
		}
		/* 基本檢核 */
		memset(szTMSOK, 0x00, sizeof(szTMSOK));
		inGetTMSOK(szTMSOK);

		if (szTMSOK[0] != 'Y')
			return (VS_SUCCESS);

		/* 先確認ESC功能有沒有開 */
		memset(szESCMode, 0x00, sizeof(szESCMode));
		inGetESCMode(szESCMode);
		if (memcmp(&szESCMode[0], "Y", 1) != 0)
		{
			/* 沒開，就不跑ESC流程 */
			return (VS_SUCCESS);
		}

		/* 更新時間成為新的判斷點 */
		memset(&srRTC, 0x00, sizeof(CTOS_RTC));
		/* 取得EDC時間日期 */
		if (inFunc_GetSystemDateAndTime(&srRTC) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}

		memset(&gszESC_Date, 0x00, sizeof(gszESC_Date));
		memset(&gszESC_Time, 0x00, sizeof(gszESC_Time));
		sprintf(gszESC_Date, "20%02d%02d%02d",  srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
		sprintf(gszESC_Time, "%02d%02d%02d",  srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);

		/* 查各Host中是否有again筆數 */
		inTotalAgain = 0;

		inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_CREDIT_NCCC_, &inHostIndex);
		pobTran->srBRec.inHDTIndex = inHostIndex;
		inAgain = inNCCC_ESC_Again_Table_Count(pobTran);

		if (inAgain >= 0)
		{
			inTotalAgain += inAgain;
		}

		inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_DCC_, &inHostIndex);
		pobTran->srBRec.inHDTIndex = inHostIndex;
		inAgain = inNCCC_ESC_Again_Table_Count(pobTran);

		if (inAgain >= 0)
		{
			inTotalAgain += inAgain;
		}

		/* 沒筆數或是重試超過三次 */
		if (inTotalAgain <= 0 || ginESC_Send_Cnt > _ESC_SEND_RETRY_LIMIT_)
		{
			if (ginESC_Connect_flag == _ESC_CONNECT_ALREADY_)
			{
				/* 沒有上傳資料要斷線 */
				inCOMM_End(pobTran);
				ginESC_Connect_flag = _ESC_CONNECT_NOT_CONNECT_;
			}

			ginESC_Send_Cnt = 0;
			ginESC_Idle_flag = _ESC_IDLE_NO_NEED_UPLOAD_;

			return (VS_SUCCESS);
		}
		/*[260414] 調整記錄,如果是待機畫面進行,有要進行上傳時才記錄LOG */
		if(pobTran->inRunOperationID == _OPERATION_ESC_IDLE_UPLOAD_)
			vdUtility_SYSFIN_LogMessage(AT, "OPT_inNCCC_ESC_Func_Upload_Idle %d START!", inTotalAgain);
		
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_ESC_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示<電子簽單上傳> */

		if (ginESC_Connect_flag == _ESC_CONNECT_NOT_CONNECT_)
		{
			inNCCC_ESC_SwitchToESC_Host(pobTran->srBRec.inHDTIndex);
			/* 通訊也要切到ESC主機(通訊內會直接loadHDT內的CommIndex會自動切過去) */

			/* 電子簽單連線中 */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_ESC_CONNECTING_, 0, _COORDINATE_Y_LINE_8_4_);

			/* 連線 */
			if ((inRetVal = inFLOW_RunFunction(pobTran, _COMM_START_)) != VS_SUCCESS)
			{
				ginESC_Connect_flag = _ESC_CONNECT_NOT_CONNECT_;
				ginESC_Idle_flag = _ESC_IDLE_NO_NEED_UPLOAD_;		/* 失敗就等五分鐘時間 */

				/* 通訊失敗‧‧‧‧ */

			}
			else
			{
				ginESC_Connect_flag = _ESC_CONNECT_ALREADY_;
				ginESC_Idle_flag = _ESC_IDLE_UPLOAD_;			/* 成功表示連線上，下次Idle要直接跑上傳 */
			}
		}
		else
		{
			/* 有連線 */
			inRetVal = VS_SUCCESS;
		}

		if (inRetVal == VS_SUCCESS)
		{
			/* 上傳A區失敗的電子簽名 ======================================================== */
			/*       失敗 = (1)有送出去，若主機拒絕就移掉Again區紀錄，存Fail區，其他則無任何動作
					(2)沒送出去，無任何動作，下次重試
				 成功 = 移掉Again區紀錄
			*/
			/* 這裡上傳NCCC的 */
			inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_CREDIT_NCCC_, &inHostIndex);
			pobTran->srBRec.inHDTIndex = inHostIndex;
			inLoadHDTRec(pobTran->srBRec.inHDTIndex);
			inLoadHDPTRec(pobTran->srBRec.inHDTIndex);

			while(1)
			{
				/* 初始化 */
				inAgain = 0;
				/* Again table的count */
				inAgain = inNCCC_ESC_Again_Table_Count(pobTran);

				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_PutGraphic(_ESC_NOT_UPLOADED_YET_, 0, _COORDINATE_Y_LINE_8_4_);

				/* 若有次數則上傳 */
				if (inAgain > 0)
				{

				}
				/* 沒有上傳筆數 跳出*/
				else
				{
					break;
				}

				/* 顯示主機別 */
				memset(szHostName, 0x00, sizeof(szHostName));
				if (inGetHostLabel(szHostName) != VS_SUCCESS)
				{
					return (VS_ERROR);
				}
				inDISP_EnglishFont(szHostName, _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);

				memset(szTemp, 0x00, sizeof(szTemp));
				sprintf(szTemp, "%d", inAgain);
				inDISP_ChineseFont(szTemp, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_RIGHT_);
				inDISP_Wait(1000);								/* 等一秒，不然會看不到上傳狀態 */

				/* 這裡要跑上傳流程 : 連線沿用 收送 */
				inRetVal = inNCCC_ESC_ProcessOnline(pobTran, _ESC_UPLOAD_IDLE_);

				if (inRetVal == VS_ESCAPE)		        /* 回應碼不為"00"，要存Fail區 */
				{
					/* 抓出資料 */
					inNCCC_ESC_Get_Again_Record_Most_TOP(pobTran);
					/* 塞到Fail區 */
					inNCCC_ESC_Insert_Fail_Record(pobTran);
					/* 刪掉again區的 */
					inNCCC_ESC_Delete_Again_Record_Most_TOP(pobTran);
					break;
				}
				else if (inRetVal == VS_ERROR)	                /* 送電文失敗 */
				{
					ginESC_Send_Cnt ++;
					break;
				}
				else if (inRetVal == _NCCC_ESC_PACK_ERR_ ||	/* 組電文失敗 */
					 inRetVal == _NCCC_ESC_UNPACK_ERR_)	/* 解電文失敗 */
				{
					/* 抓出資料 */
					inNCCC_ESC_Get_Again_Record_Most_TOP(pobTran);
					/* 塞到Fail區 */
					inNCCC_ESC_Insert_Fail_Record(pobTran);
					/* 刪掉again區的 */
					inNCCC_ESC_Delete_Again_Record_Most_TOP(pobTran);
					break;
				}
				else if (inRetVal >= 0)
				{
					/* 結帳帳條統計 */
					/* 要放在取批次前面，避免被取消覆蓋原交易 */
					inLoadHDTRec(pobTran->srBRec.inHDTIndex);
					inACCUM_UpdateFlow_ESC(pobTran, _ESC_ACCUM_STATUS_UPLOADED_);
					
					/* 標示為已上傳 */
					if (pobTran->srBRec.inCode == _TIP_)
					{
						uszTipBit = VS_TRUE;
					}
					else
					{
						uszTipBit = VS_FALSE;
					}
					
					inBATCH_GetTransRecord_By_Sqlite(pobTran);
					pobTran->srBRec.inESCUploadStatus = _ESC_UPLOAD_STATUS_UPLOADED_;
					if (uszTipBit == VS_TRUE)
					{
						pobTran->srBRec.uszESCTipUploadBit = VS_TRUE;
					}
					else
					{
						pobTran->srBRec.uszESCOrgUploadBit = VS_TRUE;
					}
					inBATCH_Update_ESC_Uploaded_By_Sqlite(pobTran);
					inNCCC_ESC_Delete_Again_Record_Most_TOP(pobTran);
				}

			}

			/* 這裡上傳DCC的 */
			inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_DCC_, &inHostIndex);
			pobTran->srBRec.inHDTIndex = inHostIndex;
			inLoadHDTRec(pobTran->srBRec.inHDTIndex);
			inLoadHDPTRec(pobTran->srBRec.inHDTIndex);

			while(1)
			{
				/* 初始化 */
				inAgain = 0;
				/* Again table的count */
				inAgain = inNCCC_ESC_Again_Table_Count(pobTran);

				/* 若有次數則上傳 */
				if (inAgain > 0)
				{

				}
				/* 沒有上傳筆數 跳出*/
				else
				{
					break;
				}

				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_PutGraphic(_ESC_NOT_UPLOADED_YET_, 0, _COORDINATE_Y_LINE_8_4_);

				/* 顯示主機別 */
				memset(szHostName, 0x00, sizeof(szHostName));
				if (inGetHostLabel(szHostName) != VS_SUCCESS)
				{
					return (VS_ERROR);
				}
				inDISP_EnglishFont(szHostName, _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);

				memset(szTemp, 0x00, sizeof(szTemp));
				sprintf(szTemp, "%d", inAgain);
				inDISP_ChineseFont(szTemp, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_RIGHT_);
				inDISP_Wait(1000);

				/* 這裡要跑上傳流程 : 連線沿用 收送 */
				inRetVal = inNCCC_ESC_ProcessOnline(pobTran, _ESC_UPLOAD_IDLE_);

				if (inRetVal == VS_ESCAPE)		        /* 回應碼不為"00"，要存Fail區 */
				{
					/* 抓出資料 */
					inNCCC_ESC_Get_Again_Record_Most_TOP(pobTran);
					/* 塞到Fail區 */
					inNCCC_ESC_Insert_Fail_Record(pobTran);
					/* 刪掉again區的 */
					inNCCC_ESC_Delete_Again_Record_Most_TOP(pobTran);
					break;
				}
				else if (inRetVal == VS_ERROR)	                /* 送電文失敗 */
				{
					ginESC_Send_Cnt ++;
					break;
				}
				else if (inRetVal == _NCCC_ESC_PACK_ERR_ ||	/* 組電文失敗 */
					 inRetVal == _NCCC_ESC_UNPACK_ERR_)	/* 解電文失敗 */
				{
					/* 抓出資料 */
					inNCCC_ESC_Get_Again_Record_Most_TOP(pobTran);
					/* 塞到Fail區 */
					inNCCC_ESC_Insert_Fail_Record(pobTran);
					/* 刪掉again區的 */
					inNCCC_ESC_Delete_Again_Record_Most_TOP(pobTran);
					break;
				}
				else if (inRetVal >= 0)
				{
					/* 結帳帳條統計 */
					/* 要放在取批次前面，避免被取消覆蓋原交易 */
					inLoadHDTRec(pobTran->srBRec.inHDTIndex);
					inACCUM_UpdateFlow_ESC(pobTran, _ESC_ACCUM_STATUS_UPLOADED_);
					
					/* 標示為已上傳 */
					if (pobTran->srBRec.inCode == _TIP_)
					{
						uszTipBit = VS_TRUE;
					}
					else
					{
						uszTipBit = VS_FALSE;
					}
					
					inBATCH_GetTransRecord_By_Sqlite(pobTran);
					pobTran->srBRec.inESCUploadStatus = _ESC_UPLOAD_STATUS_UPLOADED_;
					if (uszTipBit == VS_TRUE)
					{
						pobTran->srBRec.uszESCTipUploadBit = VS_TRUE;
					}
					else
					{
						pobTran->srBRec.uszESCOrgUploadBit = VS_TRUE;
					}
					inBATCH_Update_ESC_Uploaded_By_Sqlite(pobTran);
					inNCCC_ESC_Delete_Again_Record_Most_TOP(pobTran);
				}

			}

			inCOMM_End(pobTran);
		}

		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "inNCCC_ESC_Func_Upload_Idle() END!");

		return (VS_SUCCESS);
	}
}


/*
Function        :inNCCC_ESC_Func_Upload_Settle
Date&Time       :2016/5/4 下午 4:23
Describe        :Settle上傳ESC
*/
int inNCCC_ESC_Func_Upload_Settle(TRANSACTION_OBJECT *pobTran)
{
	int			inRetVal = 0;
	int			inAgain = -1, inFail = -1;	/* Again table中的筆數 */
	char			szTemp[4 + 1] = {0};
	char			szHostName[8 + 1] = {0};
	char			szESCMode[2 + 1] = {0};
	char			szDemoMode[2 + 1] = {0};
	char			szCustomerIndicator[3 + 1] = {0};
	unsigned char		uszTipBit = VS_FALSE;
	TRANSACTION_OBJECT	pobESCTran;
	
        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_Func_Upload_Settle START!");
        
	/* 教育訓練模式 */
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		/* 先確認ESC功能有沒有開 */
		memset(szESCMode, 0x00, sizeof(szESCMode));
		inGetESCMode(szESCMode);
		if (memcmp(&szESCMode[0], "Y", 1) != 0)
		{
			/* 沒開，就不跑ESC流程 */
			return (VS_SUCCESS);
		}

		/* 修改讀資料會影響到結帳交易別的問題 */
		memset(&pobESCTran, 0x00, sizeof(pobESCTran));
		memcpy(&pobESCTran, pobTran, sizeof(TRANSACTION_OBJECT));
		
		/* 印帳單load回原host */
		inLoadHDTRec(pobESCTran.srBRec.inHDTIndex);

		inAgain = -1;
		/* Again table的count */
		inAgain = inNCCC_ESC_Again_Table_Count(&pobESCTran);

		inFail = -1;
		/* Fail table的count */
		inFail = inNCCC_ESC_Fail_Table_Count(&pobESCTran);

		if ((inAgain + inFail) > 0)
		{
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_ESC_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示<電子簽單上傳> */
			
			/* 列印Again區紙本 */
			while(1)
			{
				inAgain = -1;
				/* Again table的count */
				inAgain = inNCCC_ESC_Again_Table_Count(&pobESCTran);

				if (inAgain <= 0)
				{
					break;
				}

				if (inNCCC_ESC_Get_Again_Record_Most_TOP(&pobESCTran) != VS_SUCCESS)
				{
					break;
				}

				pobESCTran.srBRec.inPrintOption = _PRT_MERCH_;

				while (1)
				{
					/* (需求單 - 107227)邦柏科技自助作業客製化 邦柏客製化即使上傳失敗也不印 by Russell 2018/12/7 下午 2:36 */
					memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
					inGetCustomIndicator(szCustomerIndicator);
					if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)         ||
                                            !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_) ||
					    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)		||
					    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
					{
						inRetVal = VS_SUCCESS;
					}
					else
					{
						/* 列印帳單中 */
						inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);

						inRetVal = inCREDIT_PRINT_Receipt_ByBuffer(&pobESCTran);
					}

					/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
					if (inRetVal != VS_SUCCESS)
					{
						continue;
					}
					else
					{
						/* 成功就砍簽名圖檔 */
						inFunc_Delete_Signature(&pobESCTran);

						break;
					}

				}

				/* 直接從again區移掉，不再放入fail */
				inNCCC_ESC_Delete_Again_Record_Most_TOP(&pobESCTran);

				/* 結帳帳條統計(此時已要結帳，直接計入Fail) */
				inACCUM_UpdateFlow_ESC(&pobESCTran, _ESC_ACCUM_STATUS_FAIL_);

				/* 當筆出紙本要送advice告訴fes把當筆紙本的flag on 起來(DCC才會用到NE) */
				pobESCTran.srBRec.inESCUploadStatus = _ESC_UPLOAD_STATUS_PAPER_;
				inBATCH_ESC_Save_Advice_Flow(&pobESCTran);
				
				/* 標示為已出紙本 */
				inBATCH_GetTransRecord_By_Sqlite(&pobESCTran);
				pobESCTran.srBRec.inESCUploadStatus = _ESC_UPLOAD_STATUS_PAPER_;
				inBATCH_Update_ESC_Uploaded_By_Sqlite(&pobESCTran);
			}

			/* 列印Fail區紙本 */
			while(1)
			{
				inFail = -1;
				/* Fail table的count */
				inFail = inNCCC_ESC_Fail_Table_Count(&pobESCTran);

				if (inFail <= 0)
				{
					break;
				}

				if (inNCCC_ESC_Get_Fail_Record_Most_TOP(&pobESCTran) != VS_SUCCESS)
				{
					break;
				}


				pobESCTran.srBRec.inPrintOption = _PRT_MERCH_;

				/* 印帳單load回原host */
				inLoadHDTRec(pobESCTran.srBRec.inHDTIndex);

				while (1)
				{
					/* (需求單 - 107227)邦柏科技自助作業客製化 邦柏客製化即使上傳失敗也不印 by Russell 2018/12/7 下午 2:36 */
					memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
					inGetCustomIndicator(szCustomerIndicator);
					if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)         ||
                                            !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_) ||
					    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)		||
					    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
					{
						inRetVal = VS_SUCCESS;
					}
					else
					{
						/* 列印帳單中 */
						inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);

						inRetVal = inCREDIT_PRINT_Receipt_ByBuffer(&pobESCTran);
					}

					/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
					if (inRetVal != VS_SUCCESS)
					{
						continue;
					}
					else
					{
						/* 成功就砍簽名圖檔 */
						inFunc_Delete_Signature(&pobESCTran);

						break;
					}

				}

				/* 直接從Fail區移掉 */
				inNCCC_ESC_Delete_Fail_Record_Most_TOP(&pobESCTran);

				/* 結帳帳條統計 */
				inACCUM_UpdateFlow_ESC(&pobESCTran, _ESC_ACCUM_STATUS_FAIL_);

				/* 當筆出紙本要送advice告訴fes把當筆紙本的flag on 起來(DCC才會用到NE) */
				pobESCTran.srBRec.inESCUploadStatus = _ESC_UPLOAD_STATUS_PAPER_;
				inBATCH_ESC_Save_Advice_Flow(&pobESCTran);
				
				/* 標示為已出紙本 */
				inBATCH_GetTransRecord_By_Sqlite(&pobESCTran);
				pobESCTran.srBRec.inESCUploadStatus = _ESC_UPLOAD_STATUS_PAPER_;
				inBATCH_Update_ESC_Uploaded_By_Sqlite(&pobESCTran);
			}

		}
		
		/* 先初始化，避免送到上個Host的值 */
		pobTran->inESC_Sale_UploadCnt = 0;
		pobTran->inESC_Refund_UploadCnt = 0;
		pobTran->inESC_Sale_PaperCnt = 0;
		pobTran->inESC_Refund_PaperCnt = 0;
		pobTran->lnESC_Sale_UploadAmt = 0;
		pobTran->lnESC_Refund_UploadAmt = 0;
		pobTran->lnESC_Sale_PaperAmt = 0;
		pobTran->lnESC_Refund_PaperAmt = 0;
		
		/* 結帳前最後紀錄 */
		/* Sale已上傳 */
		if (inBATCH_Get_ESC_Upload_Count_Flow(pobTran, _TN_BATCH_TABLE_, _ESC_REINFORCE_TXNCODE_SALE_, VS_FALSE, &pobTran->inESC_Sale_UploadCnt, &pobTran->lnESC_Sale_UploadAmt) != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "ESC COUNT SQLite read fail.");
			}

		}

		/* Refund已上傳 */
		if (inBATCH_Get_ESC_Upload_Count_Flow(pobTran, _TN_BATCH_TABLE_, _ESC_REINFORCE_TXNCODE_REFUND_, VS_FALSE, &pobTran->inESC_Refund_UploadCnt, &pobTran->lnESC_Refund_UploadAmt) != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "ESC COUNT SQLite read fail.");
			}

		}

		/* Sale出紙本 */
		if (inBATCH_Get_ESC_Upload_Count_Flow(pobTran, _TN_BATCH_TABLE_, _ESC_REINFORCE_TXNCODE_SALE_, VS_TRUE, &pobTran->inESC_Sale_PaperCnt, &pobTran->lnESC_Sale_PaperAmt) != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "ESC COUNT SQLite read fail.");
			}

		}

		/* Refund出紙本 */
		if (inBATCH_Get_ESC_Upload_Count_Flow(pobTran, _TN_BATCH_TABLE_, _ESC_REINFORCE_TXNCODE_REFUND_, VS_TRUE, &pobTran->inESC_Refund_PaperCnt, &pobTran->lnESC_Refund_PaperAmt) != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "ESC COUNT SQLite read fail.");
			}

		}
		
		return (VS_SUCCESS);
	}
	else
	{
		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "inNCCC_ESC_Func_Upload_Settle() START!");

		/* 先確認ESC功能有沒有開 */
		memset(szESCMode, 0x00, sizeof(szESCMode));
		inGetESCMode(szESCMode);
		if (memcmp(&szESCMode[0], "Y", 1) != 0)
		{
			/* 沒開，就不跑ESC流程 */
			return (VS_SUCCESS);
		}

		/* 修改讀資料會影響到結帳交易別的問題 */
		memset(&pobESCTran, 0x00, sizeof(pobESCTran));
		memcpy(&pobESCTran, pobTran, sizeof(TRANSACTION_OBJECT));

		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_ESC_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示<電子簽單上傳> */


		/* Again table的count */
		inAgain = inNCCC_ESC_Again_Table_Count(&pobESCTran);

		if (inAgain > 0)
		{
			inNCCC_ESC_SwitchToESC_Host(pobESCTran.srBRec.inHDTIndex);
			/* 通訊也要切到ESC主機(通訊內會直接loadHDT內的CommIndex會自動切過去) */

			/* 第一層顯示<電子簽單上傳> */
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_ESC_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示<電子簽單上傳> */
			/* 電子簽單連線中 */
			inDISP_PutGraphic(_ESC_CONNECTING_, 0, _COORDINATE_Y_LINE_8_4_);

			/* 連線 */
			if ((inRetVal = inFLOW_RunFunction(&pobESCTran, _COMM_START_)) != VS_SUCCESS)
			{
				/* 通訊失敗‧‧‧‧ */
			}
			else
			{

			}

			if (inRetVal == VS_SUCCESS)
			{
				/* 上傳A區失敗的電子簽名 ======================================================== */
				/*       失敗 = (1)有送出去，若主機拒絕就移掉Again區紀錄，存Fail區，其他則無任何動作
						(2)沒送出去，失敗就存存Fail區
					 成功 = 移掉Again區紀錄
				*/
				inLoadHDTRec(pobESCTran.srBRec.inHDTIndex);
				inLoadHDPTRec(pobESCTran.srBRec.inHDTIndex);

				while(1)
				{
					/* Again table的count */
					inAgain = -1;
					/* Again table的count */
					inAgain = inNCCC_ESC_Again_Table_Count(&pobESCTran);

					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_PutGraphic(_ESC_NOT_UPLOADED_YET_, 0, _COORDINATE_Y_LINE_8_4_);

					/* 顯示主機別 */
					memset(szHostName, 0x00, sizeof(szHostName));
					if (inGetHostLabel(szHostName) != VS_SUCCESS)
					{
						return (VS_ERROR);
					}
					inDISP_EnglishFont(szHostName, _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);

					memset(szTemp, 0x00, sizeof(szTemp));
					sprintf(szTemp, "%d", inAgain);
					inDISP_ChineseFont(szTemp, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_RIGHT_);
					inDISP_Wait(1000);

					/* 若有次數則上傳 */
					if (inAgain > 0)
					{

					}
					/* 沒有上傳筆數 跳出*/
					else
					{
						break;
					}

					/* 這裡要跑上傳流程 : 連線沿用 收送 */
					inRetVal = inNCCC_ESC_ProcessOnline(&pobESCTran, _ESC_UPLOAD_IDLE_);

					if (inRetVal == VS_ESCAPE)		        /* 回應碼不為"00"，要存Fail區 */
					{
						/* 抓出資料 */
						inNCCC_ESC_Get_Again_Record_Most_TOP(&pobESCTran);
						/* 塞到Fail區 */
						inNCCC_ESC_Insert_Fail_Record(&pobESCTran);
						/* 刪掉again區的 */
						inNCCC_ESC_Delete_Again_Record_Most_TOP(&pobESCTran);
						break;
					}
					else if (inRetVal == VS_ERROR)	                /* 送電文失敗 */
					{
						ginESC_Send_Cnt ++;
						break;
					}
					else if (inRetVal == _NCCC_ESC_PACK_ERR_ ||	/* 組電文失敗 */
						 inRetVal == _NCCC_ESC_UNPACK_ERR_)	/* 解電文失敗 */
					{
						/* 抓出資料 */
						inNCCC_ESC_Get_Again_Record_Most_TOP(&pobESCTran);
						/* 塞到Fail區 */
						inNCCC_ESC_Insert_Fail_Record(&pobESCTran);
						/* 刪掉again區的 */
						inNCCC_ESC_Delete_Again_Record_Most_TOP(&pobESCTran);
						break;
					}
					else if (inRetVal >= 0)
					{
						/* 結帳帳條統計 */
						/* 要放在取批次前面，避免被取消覆蓋原交易 */
						inLoadHDTRec(pobESCTran.srBRec.inHDTIndex);
						inACCUM_UpdateFlow_ESC(&pobESCTran, _ESC_ACCUM_STATUS_UPLOADED_);
					
						/* 標示為已上傳 */
						if (pobESCTran.srBRec.inCode == _TIP_)
						{
							uszTipBit = VS_TRUE;
						}
						else
						{
							uszTipBit = VS_FALSE;
						}
						
						inBATCH_GetTransRecord_By_Sqlite(&pobESCTran);
						pobESCTran.srBRec.inESCUploadStatus = _ESC_UPLOAD_STATUS_UPLOADED_;
						if (uszTipBit == VS_TRUE)
						{
							pobESCTran.srBRec.uszESCTipUploadBit = VS_TRUE;
						}
						else
						{
							pobESCTran.srBRec.uszESCOrgUploadBit = VS_TRUE;
						}
						inBATCH_Update_ESC_Uploaded_By_Sqlite(&pobESCTran);
						inNCCC_ESC_Delete_Again_Record_Most_TOP(&pobESCTran);
					}

				}

				inCOMM_End(&pobESCTran);
			}

		}

		/* 印帳單load回原host */
		inLoadHDTRec(pobESCTran.srBRec.inHDTIndex);

		inAgain = -1;
		/* Again table的count */
		inAgain = inNCCC_ESC_Again_Table_Count(&pobESCTran);

		inFail = -1;
		/* Fail table的count */
		inFail = inNCCC_ESC_Fail_Table_Count(&pobESCTran);

		if ((inAgain + inFail) > 0)
		{
			/* 列印Again區紙本 */
			while(1)
			{
				inAgain = -1;
				/* Again table的count */
				inAgain = inNCCC_ESC_Again_Table_Count(&pobESCTran);

				if (inAgain <= 0)
				{
					break;
				}

				if (inNCCC_ESC_Get_Again_Record_Most_TOP(&pobESCTran) != VS_SUCCESS)
				{
					break;
				}

				pobESCTran.srBRec.inPrintOption = _PRT_MERCH_;

				while (1)
				{
					/* (需求單 - 107227)邦柏科技自助作業客製化 邦柏客製化即使上傳失敗也不印 by Russell 2018/12/7 下午 2:36 */
					memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
					inGetCustomIndicator(szCustomerIndicator);
					if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)         ||
                                            !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_) ||
					    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)		||
					    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
					{
						inRetVal = VS_SUCCESS;
					}
					else
					{
						/* 列印帳單中 */
						inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);

						inRetVal = inCREDIT_PRINT_Receipt_ByBuffer(&pobESCTran);
					}

					/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
					if (inRetVal != VS_SUCCESS)
					{
						continue;
					}
					else
					{
						/* 成功就砍簽名圖檔 */
						inFunc_Delete_Signature(&pobESCTran);

						break;
					}

				}

				/* 直接從again區移掉，不再放入fail */
				inNCCC_ESC_Delete_Again_Record_Most_TOP(&pobESCTran);

				/* 結帳帳條統計(此時已要結帳，直接計入Fail) */
				inACCUM_UpdateFlow_ESC(&pobESCTran, _ESC_ACCUM_STATUS_FAIL_);

				/* 當筆出紙本要送advice告訴fes把當筆紙本的flag on 起來(DCC才會用到NE) */
				pobESCTran.srBRec.inESCUploadStatus = _ESC_UPLOAD_STATUS_PAPER_;
				inBATCH_ESC_Save_Advice_Flow(&pobESCTran);
				
				/* 標示為已出紙本 */
				inBATCH_GetTransRecord_By_Sqlite(&pobESCTran);
				pobESCTran.srBRec.inESCUploadStatus = _ESC_UPLOAD_STATUS_PAPER_;
				inBATCH_Update_ESC_Uploaded_By_Sqlite(&pobESCTran);
			}

			/* 列印Fail區紙本 */
			while(1)
			{
				inFail = -1;
				/* Fail table的count */
				inFail = inNCCC_ESC_Fail_Table_Count(&pobESCTran);

				if (inFail <= 0)
				{
					break;
				}

				if (inNCCC_ESC_Get_Fail_Record_Most_TOP(&pobESCTran) != VS_SUCCESS)
				{
					break;
				}


				pobESCTran.srBRec.inPrintOption = _PRT_MERCH_;

				/* 印帳單load回原host */
				inLoadHDTRec(pobESCTran.srBRec.inHDTIndex);

				while (1)
				{
					/* (需求單 - 107227)邦柏科技自助作業客製化 邦柏客製化即使上傳失敗也不印 by Russell 2018/12/7 下午 2:36 */
					memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
					inGetCustomIndicator(szCustomerIndicator);
					if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)         ||
                                            !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_) ||
					    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)		||
					    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
					{
						inRetVal = VS_SUCCESS;
					}
					else
					{
						/* 列印帳單中 */
						inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);

						inRetVal = inCREDIT_PRINT_Receipt_ByBuffer(&pobESCTran);
					}

					/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
					if (inRetVal != VS_SUCCESS)
					{
						continue;
					}
					else
					{
						/* 成功就砍簽名圖檔 */
						inFunc_Delete_Signature(&pobESCTran);

						break;
					}

				}

				/* 直接從Fail區移掉 */
				inNCCC_ESC_Delete_Fail_Record_Most_TOP(&pobESCTran);

				/* 結帳帳條統計 */
				inACCUM_UpdateFlow_ESC(&pobESCTran, _ESC_ACCUM_STATUS_FAIL_);

				/* 當筆出紙本要送advice告訴fes把當筆紙本的flag on 起來(DCC才會用到NE) */
				pobESCTran.srBRec.inESCUploadStatus = _ESC_UPLOAD_STATUS_PAPER_;
				inBATCH_ESC_Save_Advice_Flow(&pobESCTran);
				
				/* 標示為已出紙本 */
				inBATCH_GetTransRecord_By_Sqlite(&pobESCTran);
				pobESCTran.srBRec.inESCUploadStatus = _ESC_UPLOAD_STATUS_PAPER_;
				inBATCH_Update_ESC_Uploaded_By_Sqlite(&pobESCTran);
			}

		}

		/* 先初始化，避免送到上個Host的值 */
		pobTran->inESC_Sale_UploadCnt = 0;
		pobTran->inESC_Refund_UploadCnt = 0;
		pobTran->inESC_Sale_PaperCnt = 0;
		pobTran->inESC_Refund_PaperCnt = 0;
		pobTran->lnESC_Sale_UploadAmt = 0;
		pobTran->lnESC_Refund_UploadAmt = 0;
		pobTran->lnESC_Sale_PaperAmt = 0;
		pobTran->lnESC_Refund_PaperAmt = 0;
		
		/* 結帳前最後紀錄 */
		/* Sale已上傳 */
		if (inBATCH_Get_ESC_Upload_Count_Flow(pobTran, _TN_BATCH_TABLE_, _ESC_REINFORCE_TXNCODE_SALE_, VS_FALSE, &pobTran->inESC_Sale_UploadCnt, &pobTran->lnESC_Sale_UploadAmt) != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "ESC COUNT SQLite read fail.");
			}

		}

		/* Refund已上傳 */
		if (inBATCH_Get_ESC_Upload_Count_Flow(pobTran, _TN_BATCH_TABLE_, _ESC_REINFORCE_TXNCODE_REFUND_, VS_FALSE, &pobTran->inESC_Refund_UploadCnt, &pobTran->lnESC_Refund_UploadAmt) != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "ESC COUNT SQLite read fail.");
			}

		}

		/* Sale出紙本 */
		if (inBATCH_Get_ESC_Upload_Count_Flow(pobTran, _TN_BATCH_TABLE_, _ESC_REINFORCE_TXNCODE_SALE_, VS_TRUE, &pobTran->inESC_Sale_PaperCnt, &pobTran->lnESC_Sale_PaperAmt) != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "ESC COUNT SQLite read fail.");
			}

		}

		/* Refund出紙本 */
		if (inBATCH_Get_ESC_Upload_Count_Flow(pobTran, _TN_BATCH_TABLE_, _ESC_REINFORCE_TXNCODE_REFUND_, VS_TRUE, &pobTran->inESC_Refund_PaperCnt, &pobTran->lnESC_Refund_PaperAmt) != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "ESC COUNT SQLite read fail.");
			}

		}

		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "inNCCC_ESC_Func_Upload_Settle() END!");

		return (VS_SUCCESS);
	}
}

/*
Function        :inNCCC_ESC_Pack03
Date&Time       :
Describe        :Field_3:       Processing Code
*/
int inNCCC_ESC_Pack03(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 	inCnt = 0;
	char	szTemplate[42 + 1];
	char	szDebugMsg[100 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_Pack03() START!");
	
	memcpy(&uszPackBuf[inCnt], &guszNCCC_ESC_ISO_Field03[0], _NCCC_ESC_PCODE_SIZE_);
	
        inCnt += _NCCC_ESC_PCODE_SIZE_;
	
	if (ginDebug == VS_TRUE)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inFunc_BCD_to_ASCII(szTemplate, guszNCCC_ESC_ISO_Field03, _NCCC_ESC_PCODE_SIZE_);
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "F_03 [P_CODE %s]", szTemplate);
		inLogPrintf(AT, szDebugMsg);
	}
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_Pack03() END!");
	
        return (inCnt);
}

/*
Function        :inNCCC_ESC_Pack04
Date&Time       :
Describe        :Field_4:       Amount, Transaction
*/
int inNCCC_ESC_Pack04(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 		inCnt = 0;
        char 		szTemplate[200 + 1];
	char		szDebugMsg[100 + 1];
        unsigned char 	uszBCD[6 + 1];

	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_Pack04() START!");
	
	/* 防呆送錯電子簽單，比較簽單金額 */
	if (pobTran->srBRec.lnTxnAmount != glnESC_PokaYoke_Amt)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "%ld != %ld", pobTran->srBRec.lnTxnAmount, glnESC_PokaYoke_Amt);
			inLogPrintf(AT, "簽單金額比較錯誤，送入失敗區");
		}
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%ld != %ld, 簽單金額比較錯誤，送入失敗區", pobTran->srBRec.lnTxnAmount, glnESC_PokaYoke_Amt);
		inUtility_StoreTraceLog_OneStep(szTemplate);
		
		return (VS_ERROR);
	}
	
        memset(szTemplate, 0x00, sizeof(szTemplate));

        switch (pobTran->inISOTxnCode)
        {
                case _ESC_UPLOAD_:
                        /* 銀聯優電文上傳都要帶原金額 */
                        if (pobTran->srBRec.uszUPlanTransBit == VS_TRUE)
                        {
                                if  (pobTran->srBRec.lnCUPUPlanDiscountedAmount > 0 )
                                {
                                        sprintf(szTemplate, "%010ld00", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnCUPUPlanPreferentialAmount));
                                }
                                else
                                {
                                        sprintf(szTemplate, "%010ld00", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
                                }
                        }
                        else
                        {
                                sprintf(szTemplate, "%010ld00", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
                        }
                        break;
                default:
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "inESC_Pack04()_TxnCode = %d ERROR", pobTran->inISOTxnCode);
				inLogPrintf(AT, szDebugMsg);
			}
                        return (VS_ERROR);
			
                        break;
        }

        memset(uszBCD, 0x00, sizeof(uszBCD));
        inFunc_ASCII_to_BCD(&uszBCD[0], &szTemplate[0], 6);
        memcpy((char *) &uszPackBuf[inCnt], (char *) &uszBCD[0], 6);
        inCnt += 6;
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "F_04 [AMT %s]", szTemplate);
		inLogPrintf(AT, szDebugMsg);
	}
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_Pack04() END!");
	
        return (inCnt);
}

/*
Function        :inNCCC_ESC_Pack11
Date&Time       :
Describe        :Field_11:      System Trace Audit Number
*/
int inNCCC_ESC_Pack11(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int		inESC_Hostindex;
        int 		inCnt = 0;
        char 		szSTAN[6 + 1];
	char		szDebugMsg[100 + 1];;
        unsigned char 	uszBCD[3 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_Pack11() START!");
	/* STAN使用ESC主機的 */
	inNCCC_ESC_SwitchToESC_Host(pobTran->srBRec.inHDTIndex);
	if (inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_ESC_, &inESC_Hostindex) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	/* 透過inFunc_Find_Specific_HDTindex 取得inESC_Hostindex ，
	 * inLoadHDTRec(inESC_Hostindex) 這樣可以少檢查一次?
	 *  */
	inLoadHDPTRec(inESC_Hostindex);
        memset(szSTAN, 0x00, sizeof(szSTAN));
	inGetSTANNum(szSTAN);
	/* 記得切回來 */
	inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
	inLoadHDTRec(pobTran->srBRec.inHDTIndex);
	
        memset(uszBCD, 0x00, sizeof(uszBCD));
        inFunc_ASCII_to_BCD(&uszBCD[0], &szSTAN[0], 3);
        memcpy((char *) &uszPackBuf[inCnt], (char *) &uszBCD[0], 3);
        inCnt += 3;
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "F_011 [STAN %s]", szSTAN);
		inLogPrintf(AT, szDebugMsg);
	}
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_Pack11() END!");
	
        return (inCnt);
}

/*
Function        :inNCCC_ESC_Pack12
Date&Time       :
Describe        :Field_12:      Time, Local Transaction
*/
int inNCCC_ESC_Pack12(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 		inCnt = 0;
	char		szDebugMsg[100 + 1];
        unsigned char 	uszBCD[3 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_Pack12() START!");
	

        memset(uszBCD, 0x00, sizeof(uszBCD));
        inFunc_ASCII_to_BCD(&uszBCD[0], &pobTran->srBRec.szTime[0], 3);
        memcpy((char *) &uszPackBuf[inCnt], (char *) &uszBCD[0], 3);
        inCnt += 3;
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "F_012 [TIME %s]", pobTran->srBRec.szTime);
		inLogPrintf(AT, szDebugMsg);
	}

	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_Pack12() END!");
	
        return (inCnt);
}

/*
Function        :inNCCC_ESC_Pack13
Date&Time       :
Describe        :Field_13:      Date, Local Transaction
*/
int inNCCC_ESC_Pack13(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 		inCnt = 0;
	char		szTemplate[42 + 1];
	char		szDebugMsg[100 + 1];
        unsigned char 	uszBCD[2 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_Pack13() START!");

        memset(uszBCD, 0x00, sizeof(uszBCD));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(szTemplate, &pobTran->srBRec.szDate[4], 4);
        inFunc_ASCII_to_BCD(&uszBCD[0], szTemplate, 2);
        memcpy((char *) &uszPackBuf[inCnt], (char *) &uszBCD[0], 2);
        inCnt += 2;
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "F_013 [DATE %s]", szTemplate);
		inLogPrintf(AT, szDebugMsg);
	}
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_Pack13() END!");

        return (inCnt);
}

/*
Function        :inNCCC_ESC_Pack22
Date&Time       :
Describe        :Field_22:   Point of Service Entry Mode
*/
int inNCCC_ESC_Pack22(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int		inRetVal = VS_ERROR;
        int             inCnt = 0;
	char		szDebugMsg[100 + 1];			
        char            szPOSmode[4 + 1];			/* Field 22的Ascii值 */
	char		szTerminalCapbilityAscii[6 + 1];	/* TerminalCapbility的Ascii值 */
	char		szTerminalCapbility[3 + 1];		/* TerminalCapbility的Hex值 */
	char		szExpectApplicationIndex[2 + 1] = {0};
	unsigned char	uszPinBit = VS_FALSE;
        unsigned char   uszBCD[2 + 1];				/* Field 22的Hex值 */
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_Pack22() START!");
	
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
	/* 不是CUP，則pin bit一律為True 和 ATS邏輯同步 (2024/6/19 下午 4:36)*/
	else
	{
		uszPinBit = VS_TRUE;
	}
	
        /*
	Position 1	PAN Entry Capability
	5		EMV compliance terminal

	Position 2 & 3	PAN Entry Mode
	01		PAN entered manually
	02			PAN auto-entry via magnetic stripe
				UICC Fallback
	05		PAN auto-entry via chip
	07		PAN auto-entry via contactless;
			MasterCard PayPass Chip
			VISA Paywave 1
			VISA Paywave 3
			JCB J/Speedy
	80		M/Chip Fallback
	90		VSDC Fallback
	91		PAN auto-entry via contactless
			PayPass Magnetic Stripe only
	97		J/Smart Fallback

	Position 4	PAN Entry Capability
	1		With PIN entry capability
	2		Without PIN entry capability
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
		if (pobTran->srBRec.uszCUPEMVQRCodeBit == VS_TRUE)
		{
			strcpy(szPOSmode, "5032");
		}
		else
		{
			strcpy(szPOSmode, "5012");
		}
	}
	else
	{
		/* Position 1 */
		strcpy(szPOSmode, "5");

		/* Position 2 & 3 */
		if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
		{
			/*  金融卡且Contactless */
			if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
			{
				/* 端末機及相關系統新增支援手機金融卡「行動裝置」判斷邏輯處理流程 */
				/* 1 = 卡片 2 = 手機 */
				if (!memcmp(&pobTran->srBRec.szFiscPayDevice[0], _FISC_PAY_DEVICE_MOBILE_, 1))
					strcat(szPOSmode, "67");
				else
					strcat(szPOSmode, "07");
			}
			else
			{
				strcat(szPOSmode, "05");
			}
		}
		else
		{
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
			else if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
			{
				strcat(szPOSmode, "03");
			}
			else if (pobTran->srBRec.uszCUPEMVQRCodeBit == VS_TRUE)
			{
				strcat(szPOSmode, "03");
			}
			/* 02是磁卡 */
			else
			{
				strcat(szPOSmode, "02");
			}
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
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "F_022 [POSMode %s]", szPOSmode);
		inLogPrintf(AT, szDebugMsg);
	}

	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_Pack22() END!");
	
        return (inCnt);
}

/*
Function        :inNCCC_ESC_Pack24
Date&Time       :
Describe        :Field_24:      Network International Identifier (NII)
*/
int inNCCC_ESC_Pack24(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 		inCnt = 0;
        char 		szTemplate[42 + 1], szDebugMsg[100 + 1];
        unsigned char 	uszBCD[2 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_Pack24() START!");
	
	/* For ESC電子簽單，上傳流程要使用ESC Host */
	inNCCC_ESC_SwitchToESC_Host(pobTran->srBRec.inHDTIndex);
        memset(szTemplate, 0x00, sizeof(szTemplate));
        if (inGetNII(&szTemplate[1]) == VS_ERROR)
                return (VS_ERROR);
	
	/* 記得load回來 */
	inLoadHDTRec(pobTran->srBRec.inHDTIndex);

        /* NII 只有三碼所以前面補0 */
        szTemplate[0] = '0' ;

        memset(uszBCD, 0x00, sizeof(uszBCD));
        inFunc_ASCII_to_BCD(&uszBCD[0], &szTemplate[0], 2);

        memcpy((char *) &uszPackBuf[inCnt], (char *) &uszBCD[0], 2);
        inCnt += 2;
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "F_024 [NII %s]", szTemplate);
		inLogPrintf(AT, szDebugMsg);
	}
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_Pack24() END!");

        return (inCnt);
}

/*
Function        :inNCCC_ESC_Pack25
Date&Time       :
Describe        :Field_25:   Point of Service Condition Code
*/
int inNCCC_ESC_Pack25(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 		inCnt = 0;
	char		szTemplate[42 + 1], szDebugMsg[100 + 1];
	char		szSignStatus[2 + 1];
	char		szEncryptMode[2 + 1];
        unsigned char 	uszBCD[1 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_Pack25() START!");

        /* For ESC電子簽單，上傳流程 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	/*
		第一個Nibble表示加密方式，
		0  tSAM加密
		9  3DES軟體加密

		第二個Nibble表示本交易有無電子簽名檔。
		0  有上傳電子簽名檔
		1  無上傳電子簽名檔(免簽名)
		(刪除線(是否改為一律帶1))2  無上傳電子簽名檔(持卡人使用紙本簽名) 
	*/
	
	memset(szEncryptMode, 0x00, sizeof(szEncryptMode));
	inGetEncryptMode(szEncryptMode);
	if (memcmp(szEncryptMode, _NCCC_ENCRYPTION_TSAM_, strlen(_NCCC_ENCRYPTION_TSAM_)) == 0)
	{
		strcpy(szTemplate, "0");
	}
	else
	{
		strcpy(szTemplate, "9");
	}
	
	memset(szSignStatus, 0x00, sizeof(szSignStatus));
	if (pobTran->srBRec.inSignStatus == _SIGN_SIGNED_)
	{
		sprintf(szSignStatus, "%d", 0);
	}
	else
	{
		sprintf(szSignStatus, "%d", 1);
	}
	memcpy(&szTemplate[1], szSignStatus, 1);
	
	inFunc_ASCII_to_BCD(uszBCD, szTemplate, 1);
        memcpy((char *) &uszPackBuf[inCnt], (char *)uszBCD, 1);
        inCnt++;
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "F_025 [POSCondition code %s]", szTemplate);
		inLogPrintf(AT, szDebugMsg);
	}
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_Pack25() END!");

        return (inCnt);
}

/*
Function        :inNCCC_ESC_Pack35
Date&Time       :2017/5/4 下午 6:02
Describe        :Field_35:   TRACK 2 Data
*/
int inNCCC_ESC_Pack35(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int     inCnt = 0, inPacketCnt = 0;
        char    szT2Data[100 + 1] = {0};
	char	szFinalPAN[20 + 1] = {0};		/* 為了U CARD */
	char	szDebugMsg[100 + 1] = {0};
	char	szEncrypt[16 + 1] = {0};
	char	szResult[16 + 1] = {0};
	char	szAscii[16 + 1] = {0};
	char	szFinalAscii[200 + 1] = {0};
	short   shLen = 0;
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_Pack35() START!");

        /*
        1.	本系統所有卡號一律透過ISO8583 Field_35上傳，不使用Field_02及Field_14。
	2.	所有交易只上傳【卡號】=【有效期】
	3.	U Card卡號上傳規則【11碼卡號】=【有效期】。
	4.	本欄位為敏感性資料須符合PCI規範，若以TCP/IP 協定上傳之交易電文，本欄位須依照本中心規範進行卡號加密。

        */
        
	/* PAN + 'D' + Expire Date */
	memset(szT2Data, 0x00, sizeof(szT2Data));

	/* Field 35 raw data */
	/* 備註：U CARD */
	memset(szFinalPAN, 0x00, sizeof(szFinalPAN));
	/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
	strcpy(szFinalPAN, pobTran->srBRec.szPAN);
	
	/* 初始化 */
	guszFiscBit = VS_FALSE;
	
	/* 金融卡（SmartPay） */
        if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
        {
		guszFiscBit = VS_TRUE;
                uszPackBuf[inCnt ++] = 0x16;
		
		shLen = _FISC_ACCOUNT_SIZE_;
		
                /* 帳號（PAN） */
		/* 開始加密 */
		memset(szEncrypt, 0x00, sizeof(szEncrypt));
		memset(szResult, 0x00, sizeof(szResult));
		/* 轉hex */
		inFunc_ASCII_to_BCD((unsigned char*)szEncrypt, pobTran->srBRec.szFiscAccount, (shLen / 2));
		inNCCC_TMK_ESC_3DES_Encrypt(szEncrypt, (shLen / 2), szResult);

		/* 轉回 ascii 塞回去 */
		memset(szAscii, 0x00, sizeof(szAscii));
		inFunc_BCD_to_ASCII(szAscii, (unsigned char*)szResult, (shLen / 2));

		/* 直接塞回去 */
		memset(szFinalAscii, 0x00, sizeof(szFinalAscii));
		memcpy(&szFinalAscii[0], szAscii, shLen);
		
                memcpy(&uszPackBuf[inCnt], szFinalAscii, shLen);
                inCnt += shLen;
		
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "F_35 [%X %s]", uszPackBuf[inCnt - shLen - 1], szFinalAscii);
			inLogPrintf(AT, szDebugMsg);
		}
        }
        /* 信用卡 */
        else
        {
		strcpy(szT2Data, szFinalPAN);
		inPacketCnt += strlen(szFinalPAN);

		strcat(szT2Data, "D");
		inPacketCnt ++;

		memcpy(&szT2Data[inPacketCnt], &pobTran->srBRec.szExpDate[0], 4);
		inPacketCnt += 4;

		/* 卡號 + 有效期不滿16碼要補F */
		if (strlen(szT2Data) < 16)
		{
			inFunc_PAD_ASCII(szT2Data, szT2Data, 'F', 16, _PADDING_RIGHT_);
			inPacketCnt = 16;
		}
		/* F35最前面放長度*/
		shLen = strlen(szT2Data);

		uszPackBuf[inCnt] = (shLen / 10 * 16) + (shLen % 10);
		inCnt ++;

		/* 如果長度不為偶數，需補0*/
		if (shLen % 2)
		{
			shLen ++;
			strcat(szT2Data, "0");
		}

		/* 開始加密 */
		memset(szEncrypt, 0x00, sizeof(szEncrypt));
		memset(szResult, 0x00, sizeof(szResult));
		/* 轉hex */
		inFunc_ASCII_to_BCD((unsigned char*)szEncrypt, szT2Data, 8);
		inNCCC_TMK_ESC_3DES_Encrypt(szEncrypt, 8, szResult);

		/* 轉回 ascii 塞回去 */
		memset(szAscii, 0x00, sizeof(szAscii));
		inFunc_BCD_to_ASCII(szAscii, (unsigned char*)szResult, 8);

		memcpy(szT2Data, szAscii, 16);

		/* 一起轉回HEX並塞到packet中*/
		inFunc_ASCII_to_BCD(&uszPackBuf[inCnt], &szT2Data[0], (shLen / 2));
		inCnt += (shLen / 2);
		
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "F_35 [T2DATA  %s]", szT2Data);
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_Pack35() END!");

        return (inCnt);
}

/*
Function        :inNCCC_ESC_Pack35_tSAM
Date&Time       :2017/5/4 下午 6:02
Describe        :Field_35:   TRACK 2 Data
*/
int inNCCC_ESC_Pack35_tSAM(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int		inCnt = 0, inPacketCnt = 0;
	int		inLen = 0;
	int		i, inRetVal = -1;
	char		szT2Data[100 + 1] = {0};
	char		szFinalPAN[20 + 1] = {0};		/* 為了U CARD */
	char		szDebugMsg[100 + 1] = {0};
	char		szEncrypt[16 + 1] = {0};
	char		szAscii[84 + 1] = {0};
	char		szTemplate[100 + 1] = {0};
	char		szField37[8 + 1] = {0}, szField41[4 + 1] = {0};
	char		szFinalAscii[200 + 1] = {0};
	unsigned char	uszSlot = 0;
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_Pack35_tSAM() START!");
	
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
        1.	本系統所有卡號一律透過ISO8583 Field_35上傳，不使用Field_02及Field_14。
	2.	所有交易只上傳【卡號】=【有效期】
	3.	U Card卡號上傳規則【11碼卡號】=【有效期】。
	4.	本欄位為敏感性資料須符合PCI規範，若以TCP/IP 協定上傳之交易電文，本欄位須依照本中心規範進行卡號加密。

        */
        
	/* PAN + 'D' + Expire Date */
	memset(szT2Data, 0x00, sizeof(szT2Data));

	/* Field 35 raw data */
	/* 備註：U CARD */
	memset(szFinalPAN, 0x00, sizeof(szFinalPAN));
	/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
	strcpy(szFinalPAN, pobTran->srBRec.szPAN);
	
	/* 初始化 */
	guszFiscBit = VS_FALSE;
	
	/* 金融卡（SmartPay） */
        if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
        {
		guszFiscBit = VS_TRUE;
		
                uszPackBuf[inCnt ++] = 0x16;
		
		/* 原長度16，最後電文也放16 Bytes*/
		inLen = 16;
		
		/* 金融卡帳號開始加密 */
		/* 組 Field_37 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(szTemplate, pobTran->srBRec.szRefNo, 11);
		inFunc_PAD_ASCII(szTemplate, szTemplate, 'F', 16, _PADDING_RIGHT_); /* 補【F】 */
		memset(szField37, 0x00, sizeof(szField37));
		inFunc_ASCII_to_BCD((unsigned char*)szField37, szTemplate, 8);
		/* 組 Field_41 */
		/* Load回原Host */
		inLoadHDTRec(pobTran->srBRec.inHDTIndex);
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTerminalID(szTemplate);
		memset(szField41, 0x00, sizeof(szField41));
		inFunc_ASCII_to_BCD((unsigned char*)szField41, szTemplate, 4);
		/* Field_35 */
		memset(gusztSAMCheckSum_ESC35, 0x00, sizeof(gusztSAMCheckSum_ESC35));
		memset(szEncrypt, 0x00, sizeof(szEncrypt));
		
		memcpy(szEncrypt, pobTran->srBRec.szFiscAccount, inLen);
		
		inRetVal = inNCCC_tSAM_Encrypt(uszSlot, 
					       (inLen * 2),
					       szEncrypt,
					       (unsigned char*)&szField37,
					       (unsigned char*)&szField41,
					       &gusztSAMKeyIndex_ESC,
					       gusztSAMCheckSum_ESC35);

		if (inRetVal == VS_ERROR)
		{
			pobTran->inErrorMsg = _ERROR_CODE_V3_TSAM_ENCRYPT_FAIL_;
			return (VS_ERROR);
		}

                /* 帳號（PAN） */
		memset(szFinalAscii, 0x00, sizeof(szFinalAscii));
		memcpy(&szFinalAscii[0], szEncrypt, inRetVal);
		
                memcpy(&uszPackBuf[inCnt], szFinalAscii, inRetVal);
                inCnt += inRetVal;
		
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "F_35 [%X %s]", uszPackBuf[inCnt - inRetVal - 1], szFinalAscii);
			inLogPrintf(AT, szDebugMsg);
		}
        }
        /* 信用卡 */
        else
        {
		/* Data = 後面Data總長+長度+卡號+D+有效期 */
                /* 修改ESC F_35加密規則，移除長度兩碼 by Wei Hsiu - 2014/6/23 上午 10:35:37 */
		
		strcpy(szT2Data, szFinalPAN);
		inPacketCnt += strlen(szFinalPAN);

		strcat(szT2Data, "D");
		inPacketCnt ++;

		memcpy(&szT2Data[inPacketCnt], &pobTran->srBRec.szExpDate[0], 4);
		inPacketCnt += 4;

		/* 卡號 + 有效期不滿16碼要補F */
		if (strlen(szT2Data) < 16)
		{
			inFunc_PAD_ASCII(szT2Data, szT2Data, 'F', 16, _PADDING_RIGHT_);
			inPacketCnt = 16;
		}
		/* F35最前面放長度*/
		inLen = strlen(szT2Data);

		uszPackBuf[inCnt] = (inLen / 10 * 16) + (inLen % 10);
		inCnt ++;

		/* 如果長度不為偶數，需補0*/
		if (inLen % 2)
		{
			inLen ++;
			strcat(szT2Data, "0");
		}
		
		/* Field_35 */
		memset(gusztSAMCheckSum_ESC35, 0x00, sizeof(gusztSAMCheckSum_ESC35));
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
		
		memcpy((char *)&uszPackBuf[inCnt], szEncrypt, (inLen / 2));

                if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "inNCCC_ESC_Pack35_tSAM() 開始加密");
		
		inRetVal = inNCCC_tSAM_Encrypt(uszSlot, 
					       inLen,
					       szEncrypt,
					       (unsigned char*)&szField37,
					       (unsigned char*)&szField41,
					       &gusztSAMKeyIndex_ESC,
					       gusztSAMCheckSum_ESC35);

		if (inRetVal == VS_ERROR)
		{
			pobTran->inErrorMsg = _ERROR_CODE_V3_TSAM_ENCRYPT_FAIL_;
			return (VS_ERROR);
		}
		else
		{
			/* 塞到packet中*/
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
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "F_35 [T2DATA  %s]", szT2Data);
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_Pack35_tSAM() END!");

        return (inCnt);
}


/*
Function        :inNCCC_ESC_Pack37
Date&Time       :
Describe        :Field_37:	Retrieval Reference Number (RRN)
*/
int inNCCC_ESC_Pack37(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 	inCnt = 0;
	char	szDebugMsg[100 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_Pack37() START!");
	
        memcpy((char *) &uszPackBuf[inCnt], pobTran->srBRec.szRefNo, 12);
        inCnt += 12;

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "F_37 [RRN %s]", pobTran->srBRec.szRefNo);
		inLogPrintf(AT, szDebugMsg);
	}
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_Pack37() END!");
	
        return (inCnt);
}

/*
Function        :inNCCC_ESC_Pack38
Date&Time       :
Describe        :Field_38:	Authorization Identification Response（SmartPay交易無授權碼）
*/
int inNCCC_ESC_Pack38(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 	inCnt = 0;
        char    szAuthCode[_AUTH_CODE_SIZE_ + 1];
	char	szDebugMsg[100 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_Pack38() START!");


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

	/* 授權碼不能全0或全空白，除了CUP */
        if ((!memcmp(pobTran->srBRec.szAuthCode, "000000", 6) || !memcmp(pobTran->srBRec.szAuthCode, "      ", 6)) && 
             pobTran->srBRec.uszCUPTransBit != VS_TRUE &&
             pobTran->srBRec.uszFiscTransBit != VS_TRUE)
        {
                return (VS_ERROR);
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
		sprintf(szDebugMsg, "F_38 [AUTH CODE %s]", szAuthCode);
		inLogPrintf(AT, szDebugMsg);
	}
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_Pack38() END!");
	
        return (inCnt);
}

/*
Function        :inNCCC_ESC_Pack41
Date&Time       :
Describe        :Field_41:	Card Acceptor Terminal Identification（TID）補充說明：端末機代號需可支援英、數字。
*/
int inNCCC_ESC_Pack41(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 	inCnt = 0;
        char 	szTemplate[8 + 1]; /* szTemplate for TID */
	char	szDebugMsg[100 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_Pack41() START!");
	
	/* For ESC電子簽單，上傳流程要使用ESC Host */
	inNCCC_ESC_SwitchToESC_Host(pobTran->srBRec.inHDTIndex);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	if (inGetTerminalID(szTemplate) == VS_ERROR)
                return (VS_ERROR);

	/* 回覆原host */
	inLoadHDTRec(pobTran->srBRec.inHDTIndex);
		
        memcpy((char *) &uszPackBuf[inCnt], szTemplate, strlen(szTemplate));
        inCnt += strlen(szTemplate);
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "F_41 [TID %s]", szTemplate);
		inLogPrintf(AT, szDebugMsg);
	}
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_Pack41() END!");

        return (inCnt);
}

/*
Function        :inNCCC_ESC_Pack42
Date&Time       :
Describe        :Field_42:	Card Acceptor Identification Code(MID)
 		補充說明：
		1. 商店代號需左靠右補空白。
		2. 語音開卡交易須使用固定商店代號 0122900410 。
*/
int inNCCC_ESC_Pack42(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 	inCnt = 0;
        char 	szTemplate[16 + 1]; /* szTemplate for MID */
	char	szDebugMsg[100 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_Pack42() START!");
	
	/* For ESC電子簽單，上傳流程要使用ESC Host */
	inNCCC_ESC_SwitchToESC_Host(pobTran->srBRec.inHDTIndex);
        memset(szTemplate, 0x00, sizeof(szTemplate));
        if (inGetMerchantID(szTemplate) == VS_ERROR)
                return (VS_ERROR);

        inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 15, _PADDING_RIGHT_);
	
	/* 回覆原host */
	inLoadHDTRec(pobTran->srBRec.inHDTIndex);
	
        memcpy((char *) &uszPackBuf[inCnt], szTemplate, strlen(szTemplate));
        inCnt += strlen(szTemplate);
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "F_42 [MID %s]", szTemplate);
		inLogPrintf(AT, szDebugMsg);
	}
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_Pack42() END!");

        return (inCnt);
}

/*
Function        :inNCCC_ESC_Pack48
Date&Time       :
Describe        :Field_48:	Additional Data - Private Use
*/
int inNCCC_ESC_Pack48(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int	i;
        int	inCnt = 0;
	char	szDebugMsg[100 + 1];
	char	szTemplate[30 + 1];
	char	szCustomerIndicator[3 + 1] = {0};
	char	szPOS_ID[6 + 1] = {0};
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_Pack48() START!");

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
		sprintf(szDebugMsg, "F_48 [StoreID 001912%s]", pobTran->srBRec.szStoreID);
		inLogPrintf(AT, szDebugMsg);
	}
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_Pack48() END!");

	return (inCnt);
}

/*
Function        :inNCCC_ESC_Pack57_tSAM
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
int inNCCC_ESC_Pack57_tSAM(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int		inCnt = 0, inPacketCnt = 0;
	char		szASCII[4 + 1];
	char		szPacket[100 + 1];
	char		szPacket_Ascii[100 + 1];
	char		szTemplate[100 + 1];
	unsigned char	uszBCD[2 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_Pack57_tSAM() START!");
	
	
	/* BitMap 8 Byte */
	memset(szPacket, 0x00, sizeof(szPacket));
	szPacket[inPacketCnt ++] = 0x00;
	szPacket[inPacketCnt ++] = 0x00;
	szPacket[inPacketCnt ++] = 0x00;
	szPacket[inPacketCnt ++] = 0x00;
	szPacket[inPacketCnt ++] = 0x20;
	szPacket[inPacketCnt ++] = 0x00;
	szPacket[inPacketCnt ++] = 0x00;
	szPacket[inPacketCnt ++] = 0x20;
	
	memcpy((char *)&szPacket[inPacketCnt], (char *)&gusztSAMCheckSum_ESC35[0], 4);
	inPacketCnt += 4;

	if (ESC_UPLOAD_DATA[ginEscUploadIndex].inTableIndex == _TABLE_E1_)
	        memcpy(&szPacket[inPacketCnt], gusztSAMCheckSum_ESC57_E1, 4);
	else
	        memcpy(&szPacket[inPacketCnt], gusztSAMCheckSum_ESC57_E2, 4);

	inPacketCnt += 4;

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
                inLogPrintf(AT, "inNCCC_ESC_Pack57_tSAM() END!");
	
        return (inCnt);
}

/*
Function        :inNCCC_ESC_Pack59
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
int inNCCC_ESC_Pack59(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int		i;
	int		inCnt = 0, inPacketCnt = 0;
	int		inSwitchTxnCode;
	int		inSubLen = 0, inCardLen, inLRCDataLen = 0;
	char		szLRC;
	char		szASCII[4 + 1];
	char		szTemplate[1024 + 1], szTemplate1[42 + 1], szPacket[1024 + 1]; /* szTemplate for temprary*/
	char		szDebugMsg[100 + 1], szDebugMsg1[10 + 1], szDebugMsg2[10 + 1];
	unsigned char	uszBCD[2 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_Pack59() START!");

        memset(szTemplate, 0x00, sizeof(szTemplate));
	memset(szPacket, 0x00, sizeof(szPacket));

	/* For ESC電子簽單，上傳流程 */

	/* CUP交易 */
	if (pobTran->srBRec.uszCUPTransBit == VS_TRUE )
	{
		/* 	Table ID “N1”: UnionPay (原CUP)交易 */
		/* UnionPay(原CUP) data information */
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, " Table ID   [N1]");
		}
		memcpy(&szPacket[inPacketCnt], "N1", 2); /* Table ID */
		inPacketCnt += 2;
		/* Sub Total Length */
		szPacket[inPacketCnt] = 0x00;
		inPacketCnt ++;
		szPacket[inPacketCnt] = 0x02;
		inPacketCnt ++;

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			memset(szDebugMsg1, 0x00, sizeof(szDebugMsg1));
			memset(szDebugMsg2, 0x00, sizeof(szDebugMsg2));
			inFunc_BCD_to_ASCII(szDebugMsg1, (unsigned char*)&szPacket[inPacketCnt - 2], 1);
			inFunc_BCD_to_ASCII(szDebugMsg2, (unsigned char*)&szPacket[inPacketCnt - 1], 1);
			
			sprintf(szDebugMsg, " Table Len  [%s]", &szPacket[inPacketCnt - 2]);
			inLogPrintf(AT, szDebugMsg);
		}

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
			1 = UnionPay (原CUP) card.
		*/
		szPacket[inPacketCnt] = '1';
		inPacketCnt ++;

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			memset(szDebugMsg1, 0x00, sizeof(szDebugMsg1));
			memset(szDebugMsg2, 0x00, sizeof(szDebugMsg2));
			memcpy(szDebugMsg1, (unsigned char*)&szPacket[inPacketCnt - 2], 1);
			memcpy(szDebugMsg2, (unsigned char*)&szPacket[inPacketCnt - 1], 1);
			
			sprintf(szDebugMsg, "  Table Data [%s%s]", szDebugMsg1, szDebugMsg2);
			inLogPrintf(AT, szDebugMsg);
		}
		
	}
	/* 非CUP交易 */
	else
	{
		/* 	Table ID “N1”: UnionPay (原CUP)交易 */
		/* UnionPay(原CUP) data information */
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, " Table ID   [N1]");
		}
		memcpy(&szPacket[inPacketCnt], "N1", 2); /* Table ID */
		inPacketCnt += 2;
		/* Sub Total Length */
		szPacket[inPacketCnt] = 0x00;
		inPacketCnt ++;
		szPacket[inPacketCnt] = 0x02;
		inPacketCnt ++;

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			memset(szDebugMsg1, 0x00, sizeof(szDebugMsg1));
			memset(szDebugMsg2, 0x00, sizeof(szDebugMsg2));
			inFunc_BCD_to_ASCII(szDebugMsg1, (unsigned char*)&szPacket[inPacketCnt - 2], 1);
			inFunc_BCD_to_ASCII(szDebugMsg2, (unsigned char*)&szPacket[inPacketCnt - 1], 1);
			
			sprintf(szDebugMsg, " Table Len  [%s %s]", szDebugMsg1, szDebugMsg2);
			inLogPrintf(AT, szDebugMsg);
		}

		/*
			Function number.
			The terminal support UnionPay (原CUP) function.
			0 = Normal.
			1 = Support.
		*/
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetCUPFuncEnable(szTemplate);
		if (memcmp(szTemplate, "Y", 1) != 0)
			szPacket[inPacketCnt] = '0';
		else
			szPacket[inPacketCnt] = '1';
		
		inPacketCnt ++;
		/*
			Acquire Indicator.
			Terminal hot key control.
			0 = Normal.
			1 = UnionPay (原CUP) card.
		*/
		szPacket[inPacketCnt] = '0';
		inPacketCnt ++;

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			memset(szDebugMsg1, 0x00, sizeof(szDebugMsg1));
			memset(szDebugMsg2, 0x00, sizeof(szDebugMsg2));
			inFunc_BCD_to_ASCII(szDebugMsg1, (unsigned char*)&szPacket[inPacketCnt - 2], 1);
			inFunc_BCD_to_ASCII(szDebugMsg2, (unsigned char*)&szPacket[inPacketCnt - 1], 1);
			
			sprintf(szDebugMsg, "  Table Data [%s%s]", szDebugMsg1, szDebugMsg2);
			inLogPrintf(AT, szDebugMsg);
		}
		
		if (pobTran->inTransactionCode == _VOID_)
			inSwitchTxnCode = pobTran->srBRec.inOrgCode;
		else
			inSwitchTxnCode = pobTran->inTransactionCode;
		
		switch (inSwitchTxnCode)
		{
			case _INST_SALE_ :
			case _INST_ADJUST_ :
			case _INST_REFUND_ :
				/* 分期資料 */
				/*	Table ID “N4”: Installment Relation Data(分期付款資料) */
				/* Table ID */
				memcpy(&szPacket[inPacketCnt], "N4", 2); 
				inPacketCnt += 2;
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, " Table ID   [N4]");
				}
				
				/* Sub-Data Total Length */
				szPacket[inPacketCnt] = 0x00;
				inPacketCnt ++;
				szPacket[inPacketCnt] = 0x03; 	
				inPacketCnt ++;
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					memset(szDebugMsg1, 0x00, sizeof(szDebugMsg1));
					memset(szDebugMsg2, 0x00, sizeof(szDebugMsg2));
					inFunc_BCD_to_ASCII(szDebugMsg1, (unsigned char*)&szPacket[inPacketCnt - 2], 1);
					inFunc_BCD_to_ASCII(szDebugMsg2, (unsigned char*)&szPacket[inPacketCnt - 1], 1);

					sprintf(szDebugMsg, " Table Len  [%s %s]", szDebugMsg1, szDebugMsg2);
					inLogPrintf(AT, szDebugMsg);
				}
				
				memcpy(&szPacket[inPacketCnt], pobTran->srBRec.szInstallmentIndicator, 1);
				inPacketCnt ++;
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					memset(szDebugMsg1, 0x00, sizeof(szDebugMsg1));
					memcpy(szDebugMsg1, &szPacket[inPacketCnt - 1], 1);

					sprintf(szDebugMsg, "   InstallmentIndicator = %s", szDebugMsg1);
					inLogPrintf(AT, szDebugMsg);
				}

				/* Installment Period */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%02ld", pobTran->srBRec.lnInstallmentPeriod);
				memcpy(&szPacket[inPacketCnt], &szTemplate[0], 2);
				inPacketCnt += 2;
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					memset(szDebugMsg1, 0x00, sizeof(szDebugMsg1));
					inFunc_BCD_to_ASCII(szDebugMsg1, (unsigned char*)&szPacket[inPacketCnt - 1], 1);

					sprintf(szDebugMsg, "   InstallmentPeriod = %s", szDebugMsg1);
					inLogPrintf(AT, szDebugMsg);
				}

				break;
				
			case _REDEEM_SALE_ :
			case _REDEEM_ADJUST_ :
			case _REDEEM_REFUND_ :
				/* 紅利資料 */
				/* 	Table ID “N5”: Redeem Relation Data(紅利扣抵資料) */
				/* Table ID */
				memcpy(&szPacket[inPacketCnt], "N5", 2);
				inPacketCnt += 2;
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, " Table ID   [N5]");
				}
				/* Sub-Data Total Length */
				szPacket[inPacketCnt] = 0x00;
				inPacketCnt ++;
				szPacket[inPacketCnt] = 0x01; 	
				inPacketCnt ++;
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					memset(szDebugMsg1, 0x00, sizeof(szDebugMsg1));
					memset(szDebugMsg2, 0x00, sizeof(szDebugMsg2));
					inFunc_BCD_to_ASCII(szDebugMsg1, (unsigned char*)&szPacket[inPacketCnt - 2], 1);
					inFunc_BCD_to_ASCII(szDebugMsg2, (unsigned char*)&szPacket[inPacketCnt - 1], 1);

					sprintf(szDebugMsg, " Table Len  [%s %s]", szDebugMsg1, szDebugMsg2);
					inLogPrintf(AT, szDebugMsg);
				}
				memcpy(&szPacket[inPacketCnt], pobTran->srBRec.szRedeemIndicator, 1);
				inPacketCnt ++;
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					memset(szDebugMsg1, 0x00, sizeof(szDebugMsg1));
					inFunc_BCD_to_ASCII(szDebugMsg1, (unsigned char*)&szPacket[inPacketCnt - 1], 1);

					sprintf(szDebugMsg, "   szRedeemIndicator = %s", szDebugMsg1);
					inLogPrintf(AT, szDebugMsg);
				}

				break;
			case _TIP_ :
				if (pobTran->srBRec.uszInstallmentBit == VS_TRUE)
				{
					/* 分期資料 */
					memcpy(&szPacket[inPacketCnt], "N4", 2);	/* Table ID */
					inPacketCnt += 2;
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, " Table ID   [N4]");
					}
					
					/* Sub-Data Total Length */
					szPacket[inPacketCnt] = 0x00;
					inPacketCnt ++;
					szPacket[inPacketCnt] = 0x03; 	
					inPacketCnt ++;
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						memset(szDebugMsg1, 0x00, sizeof(szDebugMsg1));
						memset(szDebugMsg2, 0x00, sizeof(szDebugMsg2));
						inFunc_BCD_to_ASCII(szDebugMsg1, (unsigned char*)&szPacket[inPacketCnt - 2], 1);
						inFunc_BCD_to_ASCII(szDebugMsg2, (unsigned char*)&szPacket[inPacketCnt - 1], 1);

						sprintf(szDebugMsg, " Table Len  [%s %s]", szDebugMsg1, szDebugMsg2);
						inLogPrintf(AT, szDebugMsg);
					}
					
					memcpy(&szPacket[inPacketCnt], pobTran->srBRec.szInstallmentIndicator, 1);
					inPacketCnt ++;
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						memset(szDebugMsg1, 0x00, sizeof(szDebugMsg1));
						memcpy(szDebugMsg1, &szPacket[inPacketCnt - 1], 1);

						sprintf(szDebugMsg, "   InstallmentIndicator = %s", szDebugMsg1);
						inLogPrintf(AT, szDebugMsg);
					}

					/* Installment Period */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					sprintf(szTemplate, "%02ld", pobTran->srBRec.lnInstallmentPeriod);
					memcpy(&szPacket[inPacketCnt], &szTemplate[0], 2);
					inPacketCnt += 2;
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						memset(szDebugMsg1, 0x00, sizeof(szDebugMsg1));
						inFunc_BCD_to_ASCII(szDebugMsg1, (unsigned char*)&szPacket[inPacketCnt - 1], 1);

						sprintf(szDebugMsg, "   InstallmentPeriod = %s", szDebugMsg1);
						inLogPrintf(AT, szDebugMsg);
					}
				}
				else if (pobTran->srBRec.uszRedeemBit == VS_TRUE)
				{
					/* 紅利資料 */
					memcpy(&szPacket[inPacketCnt], "N5", 2);	/* Table ID */
					inPacketCnt += 2;
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, " Table ID   [N5]");
					}
					/* Sub-Data Total Length */
					szPacket[inPacketCnt] = 0x00;
					inPacketCnt ++;
					szPacket[inPacketCnt] = 0x01;
					inPacketCnt ++;
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						memset(szDebugMsg1, 0x00, sizeof(szDebugMsg1));
						memset(szDebugMsg2, 0x00, sizeof(szDebugMsg2));
						inFunc_BCD_to_ASCII(szDebugMsg1, (unsigned char*)&szPacket[inPacketCnt - 2], 1);
						inFunc_BCD_to_ASCII(szDebugMsg2, (unsigned char*)&szPacket[inPacketCnt - 1], 1);

						sprintf(szDebugMsg, " Table Len  [%s %s]", szDebugMsg1, szDebugMsg2);
						inLogPrintf(AT, szDebugMsg);
					}
					memcpy(&szPacket[inPacketCnt], pobTran->srBRec.szRedeemIndicator, 1);
					inPacketCnt ++;
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						memset(szDebugMsg1, 0x00, sizeof(szDebugMsg1));
						inFunc_BCD_to_ASCII(szDebugMsg1, (unsigned char*)&szPacket[inPacketCnt - 1], 1);

						sprintf(szDebugMsg, "   szRedeemIndicator = %s", szDebugMsg1);
						inLogPrintf(AT, szDebugMsg);
					}
				}

				break;
			default :
				break;
		}
		
		/*
		 * ● Table ID "UN" UNY 實體掃碼交易
		*/
		/* 要排除銀聯卡交易 */
		if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
		{
			memcpy(&szPacket[inPacketCnt], "UN", 2); 
			inPacketCnt += 2;	
			/* Sub Total Length */
			szPacket[inPacketCnt] = 0x00;
			inPacketCnt ++;
			szPacket[inPacketCnt] = 0x20;
			inPacketCnt ++;

			/* UNY 實體掃碼交易碼（由 ATS 編製唯一值回覆）。（註1) */
			memcpy(&szPacket[inPacketCnt], pobTran->srBRec.szUnyTransCode, 20);
			inPacketCnt += 20;
		}
	}
	
	/* E1 and E2 */
	if (ESC_UPLOAD_DATA[ginEscUploadIndex].inTableIndex == _TABLE_E1_ ||
	    ESC_UPLOAD_DATA[ginEscUploadIndex].inTableIndex == _TABLE_E2_)
	{
		szPacket[inPacketCnt] = 'E'; /* Table ID */
		inPacketCnt ++;

		if (ESC_UPLOAD_DATA[ginEscUploadIndex].inTableIndex == _TABLE_E1_)
			szPacket[inPacketCnt] = '1'; /* Table ID */
		else if (ESC_UPLOAD_DATA[ginEscUploadIndex].inTableIndex == _TABLE_E2_)
			szPacket[inPacketCnt] = '2'; /* Table ID */
		inPacketCnt ++;
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, " Table ID   [E%s]", &szPacket[inPacketCnt - 1]);
			inLogPrintf(AT, szDebugMsg);
		}

		/* E1 Len = 28 + 10 + 2 + 2 + 5 + 3 + Data Len + 1 */
		inSubLen = 51 + ESC_UPLOAD_DATA[ginEscUploadIndex].inPacketSize;

		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%04d", inSubLen);
		inFunc_ASCII_to_BCD((unsigned char*)&szPacket[inPacketCnt], szTemplate, 2);
		inPacketCnt += 2;
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, " Table Len  [%s]", szTemplate);
			inLogPrintf(AT, szDebugMsg);
		}

		/* 1.唯一序號 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		inGetTerminalID(szTemplate1);
		/* 取末3碼所以mod 1000 */
		sprintf(szTemplate, "%s%03ld%03ld", szTemplate1, (pobTran->srBRec.lnBatchNum % 1000), (pobTran->srBRec.lnOrgInvNum % 1000));

		memcpy(&szPacket[inPacketCnt], szTemplate, strlen(szTemplate));
		inPacketCnt += strlen(szTemplate);
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "   ESC S/N = %s", szTemplate);
			inLogPrintf(AT, szDebugMsg);
		}

		memset(szTemplate, 0x00, sizeof(szTemplate));
		strcat(szTemplate, pobTran->srBRec.szDate);
		strcat(szTemplate, pobTran->srBRec.szTime);

		memcpy(&szPacket[inPacketCnt], szTemplate, strlen(szTemplate));
		inPacketCnt += strlen(szTemplate);
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "           = %s", szTemplate);
			inLogPrintf(AT, szDebugMsg);
		}
		
		/* 2.Card Information */
		/* Type */
		/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
		if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_VISA_, 4))
			memcpy(&szPacket[inPacketCnt], "VS", 2);
		else if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, 10))
			memcpy(&szPacket[inPacketCnt], "MC", 2);
		else if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_JCB_, 3))
			memcpy(&szPacket[inPacketCnt], "JB", 2);
		else if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_AMEX_, 4))
			memcpy(&szPacket[inPacketCnt], "AE", 2);
		else if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_CUP_, 3))
			memcpy(&szPacket[inPacketCnt], "UP", 2);
		else if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_DINERS_, 6) ||
			 pobTran->srBRec.uszUCARDTransBit == VS_TRUE)
		{
			if (pobTran->srBRec.uszUCARDTransBit == VS_TRUE)
			{
				memcpy(&szPacket[inPacketCnt], "UC", 2);
			}
			else
			{
				memcpy(&szPacket[inPacketCnt], "DN", 2);
			}
			
		}
		else if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_SMARTPAY_, 3))
			memcpy(&szPacket[inPacketCnt], "SP", 2);
		else
		{
			/* NCCC電子簽名電文規格_V1_12_20200702.pdf 未知卡別改空白 */
			memcpy(&szPacket[inPacketCnt], "  ", 2);
		}

		inPacketCnt += 2;

		/* 前四碼後四碼 */
		memset(szTemplate, 0x00, sizeof(szTemplate));

                /* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
		strcpy(szTemplate, pobTran->srBRec.szPAN);
	        inCardLen = strlen(szTemplate);

		/* 前四碼 */
		memcpy(&szPacket[inPacketCnt], szTemplate, 4);
		inPacketCnt += 4;

		/* 後四碼 */
		memcpy(&szPacket[inPacketCnt], &szTemplate[inCardLen - 4], 4);
		inPacketCnt += 4;

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "    Card Type = %s", &szPacket[inPacketCnt - 10]);
			inLogPrintf(AT, szDebugMsg);
		}
		
		/* 3.封包內容 */
		/* 封包總個數 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%02d", ESC_UPLOAD_DATA[ginEscUploadIndex].inTotalPacketCnt);

		memcpy(&szPacket[inPacketCnt], &szTemplate[0], 2);
		inPacketCnt += 2;

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "    Total Cnt = %s", &szPacket[inPacketCnt - 2]);
			inLogPrintf(AT, szDebugMsg);
		}
		/* 目前封包編號 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%02d", ESC_UPLOAD_DATA[ginEscUploadIndex].inPacketCnt);

		memcpy(&szPacket[inPacketCnt], &szTemplate[0], 2);
		inPacketCnt += 2;
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "    Packet Cnt = %s", &szPacket[inPacketCnt - 2]);
			inLogPrintf(AT, szDebugMsg);
		}
		/* 封包總大小 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%05ld", ESC_UPLOAD_DATA[ginEscUploadIndex].lnTotalPacketSize);

		memcpy(&szPacket[inPacketCnt], &szTemplate[0], 5);
		inPacketCnt += 5;
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "    Total Size = %s", &szPacket[inPacketCnt - 5]);
			inLogPrintf(AT, szDebugMsg);
		}
		/* 目前封包大小 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%03d", ESC_UPLOAD_DATA[ginEscUploadIndex].inPacketSize);

		memcpy(&szPacket[inPacketCnt], &szTemplate[0], 3);
		inPacketCnt += 3;
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "    Packet Size = %s", &szPacket[inPacketCnt - 3]);
			inLogPrintf(AT, szDebugMsg);
		}
		/* 封包內容 */
		/* 這裡的szTemplate是用來算LRC用的 */
		memcpy(&szTemplate[3], ESC_UPLOAD_DATA[ginEscUploadIndex].szPackData, ESC_UPLOAD_DATA[ginEscUploadIndex].inPacketSize);

		memcpy(&szPacket[inPacketCnt], &ESC_UPLOAD_DATA[ginEscUploadIndex].szPackData[0], ESC_UPLOAD_DATA[ginEscUploadIndex].inPacketSize);
		inPacketCnt += ESC_UPLOAD_DATA[ginEscUploadIndex].inPacketSize;

		/* LRC = 目前封包大小 + 封包內容 XOR */
		inLRCDataLen = 3 + ESC_UPLOAD_DATA[ginEscUploadIndex].inPacketSize;
		memset(&szLRC, 0x00, 1);
		for (i = 0; i < inLRCDataLen; i++)
		{
			szLRC = szLRC ^ szTemplate[i];
		}
		
		szPacket[inPacketCnt] = szLRC;
		inPacketCnt ++;
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			memset(szASCII, 0x00, sizeof(szASCII));
			inFunc_BCD_to_ASCII(szASCII, (unsigned char*)&szPacket[inPacketCnt - 1], 1);
			sprintf(szDebugMsg, "    LRC = %s", szASCII);
			inLogPrintf(AT, szDebugMsg);
		}
	}

	/* Packet Data Length */
	memset(szASCII, 0x00, sizeof(szASCII));
	sprintf(szASCII, "%04d", inPacketCnt);
	memset(uszBCD, 0x00, sizeof(uszBCD));
	inFunc_ASCII_to_BCD(uszBCD, szASCII, 2);
	memcpy((char *)&uszPackBuf[inCnt], &uszBCD[0], 2);
	inCnt += 2;
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "    Packet Data Length = %s", szASCII);
		inLogPrintf(AT, szDebugMsg);
	}
	/* Packet Data */
	memcpy((char *)&uszPackBuf[inCnt], &szPacket[0], inPacketCnt);
	inCnt += inPacketCnt;

	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_Pack59() END!");
	
        return (inCnt);
}
/*
Function        :inNCCC_ESC_Pack60
Date&Time       :
Describe        :Field_60:	Reserved-Private Data
		Record of Count must be Unique in the batch.(Batch Number)
*/
int inNCCC_ESC_Pack60(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 	inCnt = 0;
        long 	lnBatchNum;
        char 	szTemplate[110 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_Pack60() START!");

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
                inLogPrintf(AT, "inNCCC_ESC_Pack60() END!");

        return (inCnt);
}

/*
Function        :inNCCC_ESC_Pack62
Date&Time       :
Describe        :Field_62:	Reserved-Private Data(Invoice Number)
*/
int inNCCC_ESC_Pack62(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 	inCnt = 0;
	char	szTemplate[42 + 1];

	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_Pack62() START!");
	
	
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
                inLogPrintf(AT, "inNCCC_ESC_Pack62() END!");

        return (inCnt);
}

/*
Function        :inNCCC_ESC_Check03
Date&Time       :
Describe        :
*/
int inNCCC_ESC_Check03(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, unsigned char *uszRecePacket)
{
        if (memcmp((char *)uszSendPacket, (char *)uszRecePacket, 3))
        {
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_ESC_Check04
Date&Time       :
Describe        :
*/
int inNCCC_ESC_Check04(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, unsigned char *uszRecePacket)
{
        if (memcmp((char *)uszSendPacket, (char *)uszRecePacket, 6))
        {
                //vdSGErrorMessage(NCCC_CHECK_ISO_FILED04_ERROR); /* 140 = 電文錯誤請重試 */
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_ESC_Check41
Date&Time       :
Describe        :
*/
int inNCCC_ESC_Check41(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, unsigned char *uszRecePacket)
{
        if (memcmp((char *)uszSendPacket, (char *)uszRecePacket, 8))
        {
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

int inNCCC_ESC_UnPack04(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
	long	lnAmount = 0;
	char	szDebugMsg[100 + 1];
        char	szASCII[12 + 1];


	memset(szASCII, 0x00, sizeof(szASCII));
	inFunc_BCD_to_ASCII(&szASCII[0], uszUnPackBuf, 6);
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "ESC UnPack04 Amount : %s", szASCII);
		inLogPrintf(AT, szDebugMsg);
	}
	
	lnAmount = atol(szASCII) / 100;
	
	if (lnAmount != (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount))
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "ESC Host return Amount Error : %s , EDC Upload Amount : %010ld00", szASCII, (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
		inUtility_StoreTraceLog_OneStep(szDebugMsg);
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inNCCC_ESC_UnPack24
Date&Time       :
Describe        : 1. NPS 需求要判斷 Field_24 第一個 Byte 是否為【9】
		2. 此 Flag 是由主機做控管，端末機不管例外處理
*/
int inNCCC_ESC_UnPack24(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
        char	szNPS[8];

	memset(szNPS, 0x00, sizeof(szNPS));
	inFunc_BCD_to_ASCII(&szNPS[0], uszUnPackBuf, 2);
	if (szNPS[0] == 0x39)
		pobTran->srBRec.uszField24NPSBit = VS_TRUE;
	
        return (VS_SUCCESS);
}

int inNCCC_ESC_UnPack32(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
	char	szACQ_ID[8+1];

	memset(szACQ_ID, 0x00, sizeof(szACQ_ID));
	inFunc_BCD_to_ASCII(szACQ_ID, &uszUnPackBuf[1], 4);

        if (memcmp(_ACQ_ID_, szACQ_ID, 8))
	{
//		if (pobTran->inISOTxnCode != TC_UPLOAD)
//			lnTOUCH_SGErrorMessage(TOUCH_SKM_FORMAT_ERROR_MSG);     /* 554 = 電文格式錯誤 */

		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

/*
Function        :inNCCC_ESC_UnPack38
Date&Time       :
Describe        :
*/
int inNCCC_ESC_UnPack38(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
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
Function        :inNCCC_ESC_UnPack39
Date&Time       :
Describe        :Response code
*/
int inNCCC_ESC_UnPack39(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{	
	char	szTemplate[6 + 1];

	memset(&szTemplate, 0x00, sizeof(szTemplate));
	inFunc_BCD_to_ASCII(&szTemplate[0], &guszNCCC_ESC_ISO_Field03[0], 3);

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
        	inSaveHDTRec(0);

	}

	pobTran->srBRec.szRespCode[2] = 0x00;
	
        return (VS_SUCCESS);
}

int inNCCC_ESC_ISOGetBitMapCode(TRANSACTION_OBJECT *pobTran, int inTxnType)
{
        int 	inBitMapTxnCode = -1;

        switch (inTxnType)
        {
                case _ESC_UPLOAD_ :
                        inBitMapTxnCode = inTxnType;
                        break;
                default:
                        break;
        }

        return (inBitMapTxnCode);
}

/*
Function        :inNCCC_ESC_ISOPackMessageType
Date&Time       :
Describe        :
*/
int inNCCC_ESC_ISOPackMessageType(TRANSACTION_OBJECT *pobTran, int inTxnCode, unsigned char *uszPackData, char *szMTI)
{
        int 		inCnt = 0;
        unsigned char 	uszBCD[10 + 1];
	
	/* Force Sale(強制授權交易)，Field_25=”06”。Force交易定義為Online的補登交易。
	   分期調帳及紅利調帳處理邏輯比照【Force Sale(強制授權交易)】 */
	if ((pobTran->srBRec.uszForceOnlineBit == VS_TRUE) &&
	    (inTxnCode == _SALE_OFFLINE_ || inTxnCode == _FORCE_CASH_ADVANCE_ ||
	     inTxnCode == _REDEEM_ADJUST_ || inTxnCode == _INST_ADJUST_))
	{
		memset(szMTI, 0x00, sizeof(szMTI));
		strcpy(szMTI, "0200"); /* Default */
	}

        memset(uszBCD, 0x00, sizeof(uszBCD));
        inFunc_ASCII_to_BCD(&uszBCD[0], &szMTI[0], _NCCC_ESC_MTI_SIZE_);
        memcpy((char *) &uszPackData[inCnt], (char *) &uszBCD[0], _NCCC_ESC_MTI_SIZE_);
        inCnt += _NCCC_ESC_MTI_SIZE_;

        return (inCnt);
}

/*
Function        :inNCCC_ESC_ISOModifyBitMap
Date&Time       :
Describe        :inTxnType from pobTran->inISOTxnCode
*/
int inNCCC_ESC_ISOModifyBitMap(TRANSACTION_OBJECT *pobTran, int inTxnType, int *inBitMap)
{
	char	szEncryptMode[2 + 1];
	
	if (strlen(pobTran->srBRec.szStoreID) > 0	&&
	    inTxnType != _TIP_)
		inNCCC_ESC_BitMapSet(inBitMap, 48);
	
	/* Uny交易不送35 */
	if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
	{
		inNCCC_ESC_BitMapReset(inBitMap, 35);
	}
	
	memset(szEncryptMode, 0x00, sizeof(szEncryptMode));
	inGetEncryptMode(szEncryptMode);
	if (memcmp(szEncryptMode, _NCCC_ENCRYPTION_TSAM_, strlen(_NCCC_ENCRYPTION_TSAM_)) == 0)
	{
		inNCCC_ESC_BitMapSet(inBitMap, 57);
	}

	/* ESC_UPLOAD 不用送 MAC */

        return (VS_SUCCESS);
}

int inNCCC_ESC_ISOModifyPackData(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackData, int *inPackLen)
{
	char	szCommMode[2 + 1];
	char	szEncryptionMode[2 + 1];
	
	/* 用TSAM加密，要放TSam key index到最後一個Bit */
	memset(szCommMode, 0x00, sizeof(szCommMode));
	inGetCommMode(szCommMode);
	if (memcmp(szCommMode, _COMM_ETHERNET_MODE_, 1) == 0 || 
	    memcmp(szCommMode, _COMM_GPRS_MODE_, 1) == 0	||
	    memcmp(szCommMode, _COMM_3G_MODE_, 1) == 0	||
	    memcmp(szCommMode, _COMM_4G_MODE_, 1) == 0)
	{
		memset(szEncryptionMode, 0x00, sizeof(szEncryptionMode));
		inGetEncryptMode(szEncryptionMode);

		if (memcmp(szEncryptionMode, _NCCC_ENCRYPTION_TSAM_, 1) == 0)
			uszPackData[4] = gusztSAMKeyIndex_ESC; /* TPDU 最後一個 Byte */
	}
	
        return (VS_SUCCESS);
}

int inNCCC_ESC_ISOCheckHeader(TRANSACTION_OBJECT *pobTran, char *szSendISOHeader, char *szReceISOHeader)
{
	int	inCnt = 0;

	if (ginDebug == VS_TRUE)
	{
		char szDebugMsg[100 + 1];

		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "OK");
		inLogPrintf(AT, szDebugMsg);
	}
	/* 檢查TPDU */
	inCnt += _NCCC_ESC_TPDU_SIZE_;

	/* 檢查MTI */
	szSendISOHeader[inCnt + 1] += 0x10;
	if (memcmp(&szSendISOHeader[inCnt], &szReceISOHeader[inCnt], _NCCC_ESC_MTI_SIZE_))
		return (VS_ERROR);

        return (VS_SUCCESS);
}

int inNCCC_ESC_ISOOnlineAnalyse(TRANSACTION_OBJECT *pobTran)
{
        return (VS_SUCCESS);
}

int inNCCC_ESC_ISOAdviceAnalyse(TRANSACTION_OBJECT *pobTran, unsigned char *uszTcUpload)
{
	return (VS_SUCCESS);
}

int inNCCC_ESC_BitMapSet(int *inBitMap, int inFeild)
{
        int 	i, inBMapCnt, inBitMapCnt;
        int 	inBMap[_NCCC_ESC_MAX_BIT_MAP_CNT_];

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

int inNCCC_ESC_BitMapReset(int *inBitMap, int inFeild)
{
        int 	i, inBMapCnt;
        int 	inBMap[_NCCC_ESC_MAX_BIT_MAP_CNT_];

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

int inNCCC_ESC_BitMapCheck(unsigned char *inBitMap, int inFeild)
{
        int 	inByteIndex, inBitIndex;

        inFeild--;
        inByteIndex = inFeild / 8;
        inBitIndex = 7 - (inFeild - inByteIndex * 8);

        if (_NCCC_ESC_CHECK_BIT_MAP_(inBitMap[inByteIndex], inBitIndex))
        {
                return (VS_TRUE);
        }

        return (VS_FALSE);
}

/*
Function        :inNCCC_ESC_CopyBitMap
Date&Time       :
Describe        :
*/
int inNCCC_ESC_CopyBitMap(int *inBitMap, int *inSearchBitMap)
{
        int 	i;

        for (i = 0; i < _NCCC_ESC_MAX_BIT_MAP_CNT_; i++)
        {
                if (inSearchBitMap[i] == 0)
                        break;
                else
                        inBitMap[i] = inSearchBitMap[i];
        }

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_ESC_GetBitMapTableIndex
Date&Time       :
Describe        :
*/
int inNCCC_ESC_GetBitMapTableIndex(ISO_TYPE_NCCC_ESC_TABLE *srISOFunc, int inBitMapTxnCode)
{
        int 	inBitMapIndex;

        for (inBitMapIndex = 0;; inBitMapIndex++)
        {
                if (srISOFunc->srBitMap[inBitMapIndex].inTxnID == _NCCC_ESC_NULL_TX_)
                {
                        return (VS_ERROR); /* 找不到相對應交易別的Bit Map */
                }

                if (srISOFunc->srBitMap[inBitMapIndex].inTxnID == inBitMapTxnCode)
                        break; /* 找到一樣的交易類別 */
        }

        return (inBitMapIndex);
}

int inNCCC_ESC_GetBitMapMessagegTypeField03(TRANSACTION_OBJECT *pobTran, ISO_TYPE_NCCC_ESC_TABLE *srISOFunc, int inTxnType, int *inTxnBitMap,
                unsigned char *uszSendBuf)
{
        int 		i, j, k, inBitMapIndex, *inBitMap, inCnt, inBitMapTxnCode = -1;
        char 		szErrorMessage[256 + 1];
        unsigned char 	uszBuf;

        /* 設定交易別 */
        inBitMapTxnCode = srISOFunc->inGetBitMapCode(pobTran, inTxnType);
        if (inBitMapTxnCode == -1)
                return (VS_ERROR);

        /* 要搜尋 BIT_MAP_NCCC_ESC_TABLE srNCCC_ESC_ISOBitMap 相對應的 inTxnID */
        if ((inBitMapIndex = inNCCC_ESC_GetBitMapTableIndex(srISOFunc, inBitMapTxnCode)) == VS_ERROR)
                return (VS_ERROR);

        /* Pack Message Type */
        inCnt = 0;
        inCnt += srISOFunc->inPackMTI(pobTran, inTxnType, &uszSendBuf[inCnt], srISOFunc->srBitMap[inBitMapIndex].szMTI);

        if (ginDebug == VS_TRUE)
        {
                for (i = 0; srISOFunc->srBitMap[inBitMapIndex].inBitMap[i] != 0; i++)
                {
                        memset(szErrorMessage, 0x00, sizeof(szErrorMessage));
                        sprintf(szErrorMessage, "1.inBitMap = %d", srISOFunc->srBitMap[inBitMapIndex].inBitMap[i]);
                        inLogPrintf(AT, szErrorMessage);
                }
        }

        /* 要搜尋 BIT_MAP_NCCC_ESC_TABLE srNCCC_ESC_ISOBitMap 相對應的 inBitMap */
        inNCCC_ESC_CopyBitMap(inTxnBitMap, srISOFunc->srBitMap[inBitMapIndex].inBitMap);

        if (ginDebug == VS_TRUE)
        {
                for (i = 0; srISOFunc->srBitMap[inBitMapIndex].inBitMap[i] != 0; i++)
                {
                        memset(szErrorMessage, 0x00, sizeof(szErrorMessage));
                        sprintf(szErrorMessage, "2.inBitMap = %d", srISOFunc->srBitMap[inBitMapIndex].inBitMap[i]);
                        inLogPrintf(AT, szErrorMessage);
                }
        }

        /* 修改 Bit Map */
        srISOFunc->inModifyBitMap(pobTran, inTxnType, inTxnBitMap);
	
	/* Process Code */	
        memset(guszNCCC_ESC_ISO_Field03, 0x00, sizeof(guszNCCC_ESC_ISO_Field03));
        inFunc_ASCII_to_BCD(&guszNCCC_ESC_ISO_Field03[0], srISOFunc->srBitMap[inBitMapIndex].szPCode, 3);

	/* 修改ESC電文規格Processing Code欄位值 by Wei Hsiu - 2014/8/8 下午 12:06:54
	        000000	00一般交易                      110000	11取消交易
                000100	00一般交易(分)                  110100	11取消交易(分)
                000200	00一般交易(紅)                  110200	11取消交易(紅)
                001000	00交易補登                      111000	11取消交易補登
                001100	00後台調帳(分)                  111100	11取消後台調帳(分)
                001200	00後台調帳(紅)                  111200	11取消後台調帳(紅)
                010000	01預先授權                      112000	11取消預先授權
                020000	02預先授權完成                  113000	11取消預先授權完成
                030000	03郵購交易                      114000	11取消郵購
                040000	04退貨交易                      120000	12取消退貨
                040100	04退貨交易(分)                  120100	12取消退貨(分)
                040200	04退貨交易(紅)                  120200	12取消退貨(紅)
                050000	05預借現金                      130000	13取消預借現金
                051000	05預借現金補登

                115000	11小費交易
                115100	11小費交易(分)
                115200	11小費交易(紅)
        */
	
        /* 不會有reversal 和 batch upload */
	/* 正向交易 */
        if (pobTran->srBRec.uszVOIDBit == VS_FALSE)
        {
		switch (pobTran->srBRec.inCode)
		{
			case _SALE_ :
			case _CUP_SALE_ :
			        inFunc_ASCII_to_BCD(guszNCCC_ESC_ISO_Field03, "000000", _NCCC_ESC_PCODE_SIZE_);
			        break;
			case _INST_SALE_ :
				inFunc_ASCII_to_BCD(guszNCCC_ESC_ISO_Field03, "000100", _NCCC_ESC_PCODE_SIZE_);
			        break;
			case _REDEEM_SALE_ :
				inFunc_ASCII_to_BCD(guszNCCC_ESC_ISO_Field03, "000200", _NCCC_ESC_PCODE_SIZE_);
			        break;
			case _SALE_OFFLINE_ :
				inFunc_ASCII_to_BCD(guszNCCC_ESC_ISO_Field03, "001000", _NCCC_ESC_PCODE_SIZE_);
			        break;
			case _INST_ADJUST_ :
				inFunc_ASCII_to_BCD(guszNCCC_ESC_ISO_Field03, "001100", _NCCC_ESC_PCODE_SIZE_);
			        break;
			case _REDEEM_ADJUST_ :
				inFunc_ASCII_to_BCD(guszNCCC_ESC_ISO_Field03, "001200", _NCCC_ESC_PCODE_SIZE_);
			        break;
			case _PRE_AUTH_ :
			case _CUP_PRE_AUTH_ :
				inFunc_ASCII_to_BCD(guszNCCC_ESC_ISO_Field03, "010000", _NCCC_ESC_PCODE_SIZE_);
			        break;
			case _PRE_COMP_ :
			case _CUP_PRE_COMP_ :
				inFunc_ASCII_to_BCD(guszNCCC_ESC_ISO_Field03, "020000", _NCCC_ESC_PCODE_SIZE_);
			        break;
			case _MAIL_ORDER_ :
			case _CUP_MAIL_ORDER_ :
				inFunc_ASCII_to_BCD(guszNCCC_ESC_ISO_Field03, "030000", _NCCC_ESC_PCODE_SIZE_);
			        break;
			case _REFUND_ :
			case _CUP_REFUND_ :
				inFunc_ASCII_to_BCD(guszNCCC_ESC_ISO_Field03, "040000", _NCCC_ESC_PCODE_SIZE_);
			        break;
			case _INST_REFUND_ :
				inFunc_ASCII_to_BCD(guszNCCC_ESC_ISO_Field03, "040100", _NCCC_ESC_PCODE_SIZE_);
			        break;
			case _REDEEM_REFUND_ :
				inFunc_ASCII_to_BCD(guszNCCC_ESC_ISO_Field03, "040200", _NCCC_ESC_PCODE_SIZE_);
			        break;
			case _CASH_ADVANCE_ :
				inFunc_ASCII_to_BCD(guszNCCC_ESC_ISO_Field03, "050000", _NCCC_ESC_PCODE_SIZE_);
			        break;
			case _FORCE_CASH_ADVANCE_ :
				inFunc_ASCII_to_BCD(guszNCCC_ESC_ISO_Field03, "051000", _NCCC_ESC_PCODE_SIZE_);
			        break;
			case _TIP_ :
				if (pobTran->srBRec.uszInstallmentBit == VS_TRUE)
					inFunc_ASCII_to_BCD(guszNCCC_ESC_ISO_Field03, "115100", _NCCC_ESC_PCODE_SIZE_);
				else if (pobTran->srBRec.uszRedeemBit == VS_TRUE)
					inFunc_ASCII_to_BCD(guszNCCC_ESC_ISO_Field03, "115200", _NCCC_ESC_PCODE_SIZE_);
				else
					inFunc_ASCII_to_BCD(guszNCCC_ESC_ISO_Field03, "115000", _NCCC_ESC_PCODE_SIZE_);

				break;
			case _CUP_MAIL_ORDER_REFUND_ :
				inFunc_ASCII_to_BCD(guszNCCC_ESC_ISO_Field03, "051000", _NCCC_ESC_PCODE_SIZE_);
			        break;
			case _FISC_SALE_ :
				inFunc_ASCII_to_BCD(guszNCCC_ESC_ISO_Field03, "002000", _NCCC_ESC_PCODE_SIZE_);
			        break;
			case _FISC_REFUND_ :
				inFunc_ASCII_to_BCD(guszNCCC_ESC_ISO_Field03, "041000", _NCCC_ESC_PCODE_SIZE_);
			        break;
			default :
			        memset(szErrorMessage, 0x00, sizeof(szErrorMessage));
			        sprintf(szErrorMessage, "ESC1 無此交易別 %d", pobTran->srBRec.inCode);
			        inLogPrintf(AT, szErrorMessage);
				return (VS_ERROR);
	        }
		
        }
	else
	{
		switch (pobTran->srBRec.inOrgCode)
		{
			case _SALE_ :
			case _CUP_SALE_ :
			        inFunc_ASCII_to_BCD(guszNCCC_ESC_ISO_Field03, "110000", _NCCC_ESC_PCODE_SIZE_);
			        break;
			case _INST_SALE_ :
				inFunc_ASCII_to_BCD(guszNCCC_ESC_ISO_Field03, "110100", _NCCC_ESC_PCODE_SIZE_);
			        break;
			case _REDEEM_SALE_ :
				inFunc_ASCII_to_BCD(guszNCCC_ESC_ISO_Field03, "110200", _NCCC_ESC_PCODE_SIZE_);
			        break;
			case _SALE_OFFLINE_ :
				inFunc_ASCII_to_BCD(guszNCCC_ESC_ISO_Field03, "111000", _NCCC_ESC_PCODE_SIZE_);
			        break;
			case _INST_ADJUST_ :
				inFunc_ASCII_to_BCD(guszNCCC_ESC_ISO_Field03, "111100", _NCCC_ESC_PCODE_SIZE_);
			        break;
			case _REDEEM_ADJUST_ :
				inFunc_ASCII_to_BCD(guszNCCC_ESC_ISO_Field03, "111200", _NCCC_ESC_PCODE_SIZE_);
			        break;
			case _PRE_AUTH_ :
			case _CUP_PRE_AUTH_ :
				inFunc_ASCII_to_BCD(guszNCCC_ESC_ISO_Field03, "112000", _NCCC_ESC_PCODE_SIZE_);
			        break;
			case _PRE_COMP_ :
			case _CUP_PRE_COMP_ :
				inFunc_ASCII_to_BCD(guszNCCC_ESC_ISO_Field03, "113000", _NCCC_ESC_PCODE_SIZE_);
			        break;
			case _MAIL_ORDER_ :
			case _CUP_MAIL_ORDER_ :
				inFunc_ASCII_to_BCD(guszNCCC_ESC_ISO_Field03, "114000", _NCCC_ESC_PCODE_SIZE_);
			        break;
			case _REFUND_ :
			case _CUP_REFUND_ :
				inFunc_ASCII_to_BCD(guszNCCC_ESC_ISO_Field03, "120000", _NCCC_ESC_PCODE_SIZE_);
			        break;
			case _INST_REFUND_ :
				inFunc_ASCII_to_BCD(guszNCCC_ESC_ISO_Field03, "120100", _NCCC_ESC_PCODE_SIZE_);
			        break;
			case _REDEEM_REFUND_ :
				inFunc_ASCII_to_BCD(guszNCCC_ESC_ISO_Field03, "120200", _NCCC_ESC_PCODE_SIZE_);
			        break;
			case _CASH_ADVANCE_ :
				inFunc_ASCII_to_BCD(guszNCCC_ESC_ISO_Field03, "130000", _NCCC_ESC_PCODE_SIZE_);
			        break;
			case _FISC_SALE_ :
				inFunc_ASCII_to_BCD(guszNCCC_ESC_ISO_Field03, "116000", _NCCC_ESC_PCODE_SIZE_);
			        break;
			case _FISC_REFUND_ :
				inFunc_ASCII_to_BCD(guszNCCC_ESC_ISO_Field03, "116000", _NCCC_ESC_PCODE_SIZE_);
			        break;
			default :
			        memset(szErrorMessage, 0x00, sizeof(szErrorMessage));
			        sprintf(szErrorMessage, "ESC2 無此交易別 %d", pobTran->srBRec.inCode);
			        inLogPrintf(AT, szErrorMessage);
				return (VS_ERROR);
	        }
		
	}
	
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

        inCnt += _NCCC_ESC_BIT_MAP_SIZE_;

        

        return (inCnt);
}

int inNCCC_ESC_PackISO(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendBuf, int inTxnCode)
{
        int				i, inSendCnt, inField, inCnt;
        int				inBitMap[_NCCC_ESC_MAX_BIT_MAP_CNT_ + 1];
        int				inRetVal, inISOFuncIndex = -1;
        char				szTemplate[40 + 1];
        char				szLogMessage[40 + 1];
	char				szCommMode[1 + 1];
	char				szEncryptMode[2 + 1];
        unsigned char			uszBCD[20 + 1];
        ISO_TYPE_NCCC_ESC_TABLE 	srISOFunc;		
	
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_PackISO() START!");

        inSendCnt = 0;
        inField = 0;

        /* 加密模式，預設值 = 0。 ESC一定為軟加密 3DES */
	memset(szCommMode, 0x00, sizeof(szCommMode));
	inGetCommMode(szCommMode);
	if (memcmp(szCommMode, _COMM_MODEM_MODE_, 1) == 0)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "ESC 不支援走撥接");
		}
		return (VS_ERROR);
	}
	else if (memcmp(szCommMode, _COMM_ETHERNET_MODE_, 1) == 0)
	{
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
		if (ginDebug == VS_TRUE)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate,"No such CommMode! szCommMode = %d", atoi(szCommMode));
			inLogPrintf(AT, szTemplate);
		}
		
		return (VS_ERROR); /* 防呆 */
	}
	
        /* 決定要執行第幾個 Function Index */
        memset((char *) &srISOFunc, 0x00, sizeof(srISOFunc));
        memcpy((char *) &srISOFunc, (char *) &srNCCC_ESC_ISOFunc[inISOFuncIndex], sizeof(srISOFunc));
        memset((char *) inBitMap, 0x00, sizeof(inBitMap));

        /* 開始組 ISO 電文 */
	/* 轉換到ESC HOST */
	inNCCC_ESC_SwitchToESC_Host(pobTran->srBRec.inHDTIndex);
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
        memcpy((char *) &uszSendBuf[inSendCnt], (char *) &uszBCD[0], _NCCC_ESC_TPDU_SIZE_);
        inSendCnt += _NCCC_ESC_TPDU_SIZE_;
	
        /* Get Bit Map / Mesage Type / Processing Code */
        inRetVal = inNCCC_ESC_GetBitMapMessagegTypeField03(pobTran, &srISOFunc, inTxnCode, inBitMap, &uszSendBuf[inSendCnt]);
        if (inRetVal == VS_ERROR)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inNCCC_ESC_GetBitMapMessagegTypeField03() ERROR!");

                return (VS_ERROR);
        }
        else
                inSendCnt += inRetVal; /* Bit Map 長度 */

	/* ESC組封包時，要使用原本Host的參數，例如batch Number */
	inLoadHDTRec(pobTran->srBRec.inHDTIndex);
	
        for (i = 0;; i++)
        {
                if (inBitMap[i] == 0)
                        break;

                while (inBitMap[i] > srISOFunc.srPackISO[inField].inFieldNum)
                {
                        inField++;
                }

                if (inBitMap[i] == srISOFunc.srPackISO[inField].inFieldNum)
                {
                        inCnt = srISOFunc.srPackISO[inField].inISOLoad(pobTran, &uszSendBuf[inSendCnt]);
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

        if (srISOFunc.inModifyPackData != _NCCC_ESC_NULL_TX_)
                srISOFunc.inModifyPackData(pobTran, uszSendBuf, &inSendCnt);
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_PackISO() END!");

        return (inSendCnt);
}

int inNCCC_ESC_CheckUnPackField(int inField, ISO_FIELD_NCCC_ESC_TABLE *srCheckUnPackField)
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

int inNCCC_ESC_GetCheckField(int inField, ISO_CHECK_NCCC_ESC_TABLE *ISOFieldCheck)
{
        int 	i;

        for (i = 0; i < 64; i++)
        {
                if (ISOFieldCheck[i].inFieldNum == 0)
                        break;
                else if (ISOFieldCheck[i].inFieldNum == inField)
                {
                        return (i); /* i 是 ISO_CHECK_NCCC_ESC_TABLE 裡的第幾個 */
                }
        }

        return (VS_ERROR);
}

int inNCCC_ESC_GetFieldLen(int inField, unsigned char *uszSendData, ISO_FIELD_TYPE_NCCC_ESC_TABLE *srFieldType)
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
                        case _NCCC_ESC_ISO_ASC_:
                                inCnt += srFieldType[i].inFieldLen;
                                break;
                        case _NCCC_ESC_ISO_BCD_:
                                inCnt += srFieldType[i].inFieldLen / 2;
                                break;
                        case _NCCC_ESC_ISO_NIBBLE_2_:
                                inLen = (uszSendData[0] / 16 * 10) + (uszSendData[0] % 16);
				/* Smart Pay卡號上傳格式(Field_35)不同一般信用卡。一般信用卡的卡號需要Pack成 BCD Code，Smart pay的卡號/帳號直接上傳ASCII Code不進行Pack。 */
				/* 因為reversal不讀batch，沒辦法知道fiscBit，所以還是只能強制用長度判斷 */
				if (srFieldType[i].inFieldNum == 35 && guszFiscBit == VS_TRUE)
					inCnt += inLen + 1;
				else
					inCnt += ((inLen + 1) / 2) + 1;
                                break;
                        case _NCCC_ESC_ISO_NIBBLE_3_:
                                inLen = (uszSendData[0] / 16 * 1000) + (uszSendData[0] % 16 * 100) + (uszSendData[1] / 16 * 10) + (uszSendData[1] % 16);
                                inCnt += ((inLen + 1) / 2) + 2;
                                break;
                        case _NCCC_ESC_ISO_BYTE_2_:
                                inLen = (uszSendData[0] / 16 * 10) + (uszSendData[0] % 16);
                                inCnt += inLen + 1;
                                break;
                        case _NCCC_ESC_ISO_BYTE_3_:
                                inLen = (uszSendData[0] / 16 * 1000) + (uszSendData[0] % 16 * 100) + (uszSendData[1] / 16 * 10) + (uszSendData[1] % 16);
                                inCnt += inLen + 2;
                                break;
                        case _NCCC_ESC_ISO_BYTE_2_H_:
                                inLen = (int) uszSendData[0];
                                inCnt += inLen + 1;
                                break;
                        case _NCCC_ESC_ISO_BYTE_3_H_:
                                inLen = ((int) uszSendData[0] * 0xFF) + (int) uszSendData[1];
                                inCnt += inLen + 1;
                                break;
			case _NCCC_ESC_ISO_BYTE_1_:
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

int inNCCC_ESC_GetFieldIndex(int inField, ISO_FIELD_TYPE_NCCC_ESC_TABLE *srFieldType)
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

int inNCCC_ESC_UnPackISO(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendBuf, unsigned char *uszRecvBuf)
{
	int				inRetVal;
        int				i, inRecvCnt, inCnt, inSendCnt, inSendField, inRecvField, inISOFuncIndex = -1;
        char				szBuf[_NCCC_ESC_TPDU_SIZE_ + _NCCC_ESC_MTI_SIZE_ + _NCCC_ESC_BIT_MAP_SIZE_ + 1];
        char				szErrorMessage[40 + 1];
	char				szCommMode[1 + 1];
        unsigned char			uszSendMap[_NCCC_ESC_BIT_MAP_SIZE_ + 1], uszReceMap[_NCCC_ESC_BIT_MAP_SIZE_ + 1];
        ISO_TYPE_NCCC_ESC_TABLE 	srISOFunc;


        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_UnPackISO() START!");

        inSendField = inRecvField = 0;
        inSendCnt = inRecvCnt = 0;

        memset((char *) uszSendMap, 0x00, sizeof(uszSendMap));
        memset((char *) uszReceMap, 0x00, sizeof(uszReceMap));
        memset((char *) szBuf, 0x00, sizeof(szBuf));
	
	/* 加密模式，預設值 = 0。【0 = 3DES加密】 */
	memset(szCommMode, 0x00, sizeof(szCommMode));
	inGetCommMode(szCommMode);
	if (memcmp(szCommMode, _COMM_MODEM_MODE_, 1) == 0)
		inISOFuncIndex = 0; 
	else if (memcmp(szCommMode, _COMM_ETHERNET_MODE_, 1) == 0)
	{
		inISOFuncIndex = 0;
	}
	else if (memcmp(szCommMode, _COMM_GPRS_MODE_, 1) == 0	||
		 memcmp(szCommMode, _COMM_3G_MODE_, 1) == 0	||
		 memcmp(szCommMode, _COMM_4G_MODE_, 1) == 0)
	{
		inISOFuncIndex = 0;
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
	
        memset((char *) &srISOFunc, 0x00, sizeof(srISOFunc));
        memcpy((char *) &srISOFunc, (char *) &srNCCC_ESC_ISOFunc[inISOFuncIndex], sizeof(srISOFunc));

        if (srISOFunc.inCheckISOHeader != NULL)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "srISOFunc.inCheckISOHeader != NULL");

                if (srISOFunc.inCheckISOHeader(pobTran, (char *) &uszSendBuf[inRecvCnt], (char *) &uszRecvBuf[inRecvCnt]) != VS_SUCCESS)
                {
                        if (ginDebug == VS_TRUE)
                                inLogPrintf(AT, "srISOFunc.inCheckISOHeader ERROR");

                        return (_NCCC_ESC_UNPACK_ERR_);
                }
        }

        inSendCnt += _NCCC_ESC_TPDU_SIZE_;
        inRecvCnt += _NCCC_ESC_TPDU_SIZE_;
        inSendCnt += _NCCC_ESC_MTI_SIZE_;
        inRecvCnt += _NCCC_ESC_MTI_SIZE_;

        memcpy((char *) uszSendMap, (char *) &uszSendBuf[inSendCnt], _NCCC_ESC_BIT_MAP_SIZE_);
        memcpy((char *) uszReceMap, (char *) &uszRecvBuf[inRecvCnt], _NCCC_ESC_BIT_MAP_SIZE_);

        inSendCnt += _NCCC_ESC_BIT_MAP_SIZE_;
        inRecvCnt += _NCCC_ESC_BIT_MAP_SIZE_;
	
        /* 先檢查 ISO Field_39 */
        if (inNCCC_ESC_BitMapCheck(uszReceMap, 39) == VS_FALSE)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inNCCC_ESC_BitMapCheck(39) ERROR");

                return (VS_ERROR);
        }

        if (inNCCC_ESC_BitMapCheck(uszReceMap, 41) == VS_FALSE)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inNCCC_ESC_BitMapCheck(41) ERROR");

                return (VS_ERROR);
        }

        for (i = 1; i <= 64; i++)
        {
                /* 有送出去的 Field 但沒有收回來的 Field */
                if (inNCCC_ESC_BitMapCheck(uszSendMap, i) && !inNCCC_ESC_BitMapCheck(uszReceMap, i))
                {
                        inSendCnt += inNCCC_ESC_GetFieldLen(i, &uszSendBuf[inSendCnt], srISOFunc.srISOFieldType);
                }
                else if (inNCCC_ESC_BitMapCheck(uszReceMap, i)) /* 收的 BitMap */
                {
                        if (inNCCC_ESC_BitMapCheck(uszSendMap, i)) /* 送的 BitMap 都有 */
                        {
                                /* 是否要進行檢查封包資料包含【送】【收】 */
                                if ((inSendField = inNCCC_ESC_GetCheckField(i, srISOFunc.srCheckISO)) != VS_ERROR)
                                {
                                        if (srISOFunc.srCheckISO[inSendField].inISOCheck(pobTran, &uszSendBuf[inSendCnt], &uszRecvBuf[inRecvCnt]) != VS_SUCCESS)
                                        {
                                                if (ginDebug == VS_TRUE)
                                                {
                                                        memset(szErrorMessage, 0x00, sizeof(szErrorMessage));
                                                        sprintf(szErrorMessage, "inSendField = %d Error!", srISOFunc.srPackISO[inSendField].inFieldNum);
                                                        inLogPrintf(AT, szErrorMessage);
                                                }

                                                return (_NCCC_ESC_UNPACK_ERR_);
                                        }
                                }

                                inSendCnt += inNCCC_ESC_GetFieldLen(i, &uszSendBuf[inSendCnt], srISOFunc.srISOFieldType);
                        }

                        while (i > srISOFunc.srUnPackISO[inRecvField].inFieldNum) /* 只有收 */
                        {
                                inRecvField++;
                        }

                        if (i == srISOFunc.srUnPackISO[inRecvField].inFieldNum)
                        {
                                /* 要先檢查 UnpackISO 是否存在 */
                                if (inNCCC_ESC_CheckUnPackField(i, srISOFunc.srUnPackISO) == VS_SUCCESS)
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

                        inCnt = inNCCC_ESC_GetFieldLen(i, &uszRecvBuf[inRecvCnt], srISOFunc.srISOFieldType);
                        if (inCnt == VS_ERROR)
                        {
                                if (ginDebug == VS_TRUE)
                                {
                                        memset(szErrorMessage, 0x00, sizeof(szErrorMessage));
                                        sprintf(szErrorMessage, "inRecvField = %d Error!", srISOFunc.srPackISO[inRecvField].inFieldNum);
                                        inLogPrintf(AT, szErrorMessage);
                                }

                                return (_NCCC_ESC_UNPACK_ERR_);
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
				if (inNCCC_ESC_BitMapCheck(uszReceMap, 59) == VS_FALSE)
                                {
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "inNCCC_ESC_BitMapCheck 59 Error!");
                                        }

                                        return (VS_ERROR);
                                }
				
				if (inNCCC_ESC_BitMapCheck(uszReceMap, 60) == VS_FALSE)
                                {
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "inNCCC_ESC_BitMapCheck 60 Error!");
                                        }

                                        return (VS_ERROR);
                                }

                                break;
				
			case _CUP_SALE_ :
			case _CUP_PRE_AUTH_ :
				if (inNCCC_ESC_BitMapCheck(uszReceMap, 04) == VS_FALSE)
                                {
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "inNCCC_ESC_BitMapCheck 04 Error!");
                                        }

                                        return (VS_ERROR);
                                }
				
				if (inNCCC_ESC_BitMapCheck(uszReceMap, 59) == VS_FALSE)
                                {
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "inNCCC_ESC_BitMapCheck 59 Error!");
                                        }

                                        return (VS_ERROR);
                                }

                                break;
				
			case _CUP_REFUND_:
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
				if (inNCCC_ESC_BitMapCheck(uszReceMap, 04) == VS_FALSE)
                                {
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "inNCCC_ESC_BitMapCheck 04 Error!");
                                        }

                                        return (VS_ERROR);
                                }
				
				if (inNCCC_ESC_BitMapCheck(uszReceMap, 38) == VS_FALSE)
                                {
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "inNCCC_ESC_BitMapCheck 38 Error!");
                                        }

                                        return (VS_ERROR);
                                }
				
				if (inNCCC_ESC_BitMapCheck(uszReceMap, 59) == VS_FALSE)
                                {
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "inNCCC_ESC_BitMapCheck 59 Error!");
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
				if (inNCCC_ESC_BitMapCheck(uszReceMap, 04) == VS_FALSE)
                                {
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "inNCCC_ESC_BitMapCheck 04 Error!");
                                        }

                                        return (VS_ERROR);
                                }
				
                                if (inNCCC_ESC_BitMapCheck(uszReceMap, 38) == VS_FALSE)
                                {
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "inNCCC_ESC_BitMapCheck 38 Error!");
                                        }

                                        return (VS_ERROR);
                                }

                                break;
				
			case _FISC_SALE_:
			case _FISC_VOID_:
			case _FISC_REFUND_:
				if (inNCCC_ESC_BitMapCheck(uszReceMap, 04) == VS_FALSE)
                                {
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "inNCCC_ESC_BitMapCheck 04 Error!");
                                        }

                                        return (VS_ERROR);
                                }
				
                                if (inNCCC_ESC_BitMapCheck(uszReceMap, 58) == VS_FALSE)
                                {
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "inNCCC_ESC_BitMapCheck 58 Error!");
                                        }

                                        return (VS_ERROR);
                                }
				if (inNCCC_ESC_BitMapCheck(uszReceMap, 59) == VS_FALSE)
                                {
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "inNCCC_ESC_BitMapCheck 59 Error!");
                                        }

                                        return (VS_ERROR);
                                }

                                break;
                        default:
                                break;
                }
        }

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_UnPackISO() END!");

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_ESC_GetSTAN
Date&Time       :2016/5/10 上午 9:52
Describe        :這裡get是ESC主機的STAN
*/
int inNCCC_ESC_GetSTAN(TRANSACTION_OBJECT *pobTran)
{
	int	inESC_Hostindex;
        char 	szSTANNum[12 + 1];

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_GetSTAN() START!");

	/* 這裡get是ESC主機的STAN */
	inNCCC_ESC_SwitchToESC_Host(pobTran->srBRec.inHDTIndex);
	if (inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_ESC_, &inESC_Hostindex) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	inLoadHDPTRec(inESC_Hostindex);
	
        memset(szSTANNum, 0x00, sizeof(szSTANNum));
        if (inGetSTANNum(szSTANNum) == VS_ERROR)
                return (VS_ERROR);

        pobTran->srBRec.lnSTANNum = atol(szSTANNum);
	
	/* 記得切回來 */
	inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
	inLoadHDTRec(pobTran->srBRec.inHDTIndex);

	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_GetSTAN() END!");
	
        return (VS_SUCCESS);
}

/*
Function        :inNCCC_ESC_SetSTAN
Date&Time       :2015/12/24 早上 10:25
Describe        :STAN++ 這裡加的是ESC主機的STAN
*/
int inNCCC_ESC_SetSTAN(TRANSACTION_OBJECT *pobTran)
{
	int	inESC_Hostindex;
        long 	lnSTAN;
        char 	szSTANNum[12 + 1];

	/* 這裡加的是ESC主機的STAN */
	inNCCC_ESC_SwitchToESC_Host(pobTran->srBRec.inHDTIndex);
	if (inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_ESC_, &inESC_Hostindex) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	inLoadHDPTRec(inESC_Hostindex);
		
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

	
	
        if (inSaveHDPTRec(inESC_Hostindex) < 0)
        {
                return (VS_ERROR);
        }
	
	/* 記得切回來 */
	inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
	inLoadHDTRec(pobTran->srBRec.inHDTIndex);

        return (VS_SUCCESS);
}

int inNCCC_ESC_ProcessOnline(TRANSACTION_OBJECT *pobTran, int inType)
{
	int	i;
        int 	inRetVal;
	int	inTotalPacketExamCnt = 0;
	int	inE2Index = 0;

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_ProcessOnline() START!");

	/* 如果是Idel或Settle 就把之前again table資料讀出來 */
	if (inType == _ESC_UPLOAD_IDLE_ || inType == _ESC_UPLOAD_SETTLE_)
        {
		inRetVal = inNCCC_ESC_Get_Again_Record_Most_TOP(pobTran);
		if (inRetVal != VS_SUCCESS)
		{
			vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_ProcessOnline inNCCC_ESC_Get_Again_Record_Most_TOP failed");
		}
	}
	
	/* 組簽單要原本的HDT */
        inLoadHDTRec(pobTran->srBRec.inHDTIndex);
	/* 組上傳簽單 */
	if (inNCCC_ESC_Make_E1Data(pobTran) != VS_SUCCESS)
	{
		vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_ProcessOnline inNCCC_ESC_Make_E1Data failed");
		return (VS_ERROR);
	}
	/* 【STEP 1】取得上傳資料(壓縮加密) ===========================================================
		A.簽單   : ESC_RECEIPT.txt (ESC_UPLOAD_RECEIPT)
		B.圖檔   : ESC_BMP.bmp     (ESC_UPLOAD_BMP)
		單次上傳Data limit : ESC_ISO_MAX_LEN
	*/
	
	/* 加密置換 - 裡面會使用不一樣的Host data */
	ginEscDataIndex = 0;
	if (inNCCC_ESC_Data_Compress_Encryption(pobTran) != VS_SUCCESS)
	{
		vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_ProcessOnline inNCCC_ESC_Data_Compress_Encryption");
		return (VS_ERROR);
	}
	/* 【STEP 2】置入上傳結構 ============================================================== */
	for (i = 0; i < _ESC_LIMIT_; i++)
	{
		memset(&ESC_UPLOAD_DATA[i], 0x00, sizeof(ESC_UPLOAD_DATA[i]));
	
	}
	/*
		針對整個陣列初始化 雖然只寫一行code但可讀性比起上述還差。
		memset(&ESC_UPLOAD_DATA[0],0x00,sizeof(ESC_UPLOAD_DATA));
	*/
	
	
	/* 以下會分解ESC_UPLOAD_RECEIPT and ESC_UPLOAD_BMP 到 ESC_UPLOAD_DATA並取得總上傳次數inEscDataIndex */
	/* 簽單 Append E1 */
	if (inNCCC_ESC_Data_Packet(_ESC_FILE_RECEIPT_GZ_ENCRYPTED_, _TABLE_E1_) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	/* 圖檔 Append E2 */
	/* 如果圖檔不存在就不塞 */
	if (inFILE_Check_Exist((unsigned char*)_ESC_FILE_BMP_GZ_ENCRYPTED_) == VS_SUCCESS)
	{
		if (inNCCC_ESC_Data_Packet(_ESC_FILE_BMP_GZ_ENCRYPTED_, _TABLE_E2_) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
	}
	/* 【STEP 3】開始上傳 ====================================================================== */
	/* 上傳到第幾個封包 */
	ginEscUploadIndex = 0;
	/* 防呆Start，ginEscUploadIndex和E1+E2的封包數再比一次，因出現過線上機器多送ESC封包的狀況而加 */
	inTotalPacketExamCnt = 0;
	inE2Index = 0;
	/* E1封包 */
	inTotalPacketExamCnt += ESC_UPLOAD_DATA[0].inTotalPacketCnt;
	inE2Index += inTotalPacketExamCnt;
	/* E2封包 */
	inTotalPacketExamCnt += ESC_UPLOAD_DATA[inE2Index].inTotalPacketCnt;
	/* 防呆End */
	
	if (guszTestRedundantESCPacketBit == VS_TRUE)
	{
		memcpy(&ESC_UPLOAD_DATA[ginEscDataIndex], &ESC_UPLOAD_DATA[0], sizeof(&ESC_UPLOAD_DATA[0]));
		ginEscDataIndex += 1;
	}
		
	while(1)
	{
		/* 以防萬一，下兩個條件 1.inTableIndex不是E1 or E2 2.正在上傳的封包index大於全部需要上傳的封包數 */
		if ((ESC_UPLOAD_DATA[ginEscUploadIndex].inTableIndex != _TABLE_E1_ &&
		     ESC_UPLOAD_DATA[ginEscUploadIndex].inTableIndex != _TABLE_E2_) ||
		     (ginEscUploadIndex > ginEscDataIndex))
		    	break;
		
		/* 防呆Start，ginEscUploadIndex和E1+E2的封包數再比一次，因出現過線上機器多送ESC封包的狀況而加 */
		if (ginEscUploadIndex >= inTotalPacketExamCnt)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "ESC Packet檢核，超過應送數目");
			}
			/* [BUG] 移除檔案同步API SYNC()指令
			 * [20260506] 記錄防呆筆數 */
			vdUtility_SYSFIN_LogMessage(AT, "ESC Pack Count Error UpLoadCnt[%d] TotalPack[%d]", ginEscUploadIndex, inTotalPacketExamCnt);
			break;
		}
		/* 防呆End */
		
		/* 開始組交易封包，送、收、組、解 */
		pobTran->inISOTxnCode = _ESC_UPLOAD_;					/* 封包別一律為_ESC_UPLOAD_ */
		pobTran->inTransactionCode = pobTran->srBRec.inESCTransactionCode;
		inRetVal = inNCCC_ESC_SendPackRecvUnPack(pobTran);
		
		/* 成功或失敗 System_Trace_Number 都要加一 這裡加的是ESC主機的STAN */
		inNCCC_ESC_SetSTAN(pobTran);
		
		if (inRetVal != VS_SUCCESS)
		{
			vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_ProcessOnline inNCCC_ESC_SendPackRecvUnPack_failed");
			ginESC_Connect_flag = _ESC_CONNECT_NOT_CONNECT_;
			/* 不論上傳成功與否 都刪上傳用檔 */
			inFILE_Delete((unsigned char*)_ESC_FILE_RECEIPT_GZ_ENCRYPTED_);
			inFILE_Delete((unsigned char*)_ESC_FILE_BMP_GZ_ENCRYPTED_);
			
			return (inRetVal);
		}
		ginEscUploadIndex ++;
		
	}
	
	/* 不論上傳成功與否 都刪上傳用檔 */
	inFILE_Delete((unsigned char*)_ESC_FILE_RECEIPT_GZ_ENCRYPTED_);
	inFILE_Delete((unsigned char*)_ESC_FILE_BMP_GZ_ENCRYPTED_);
	
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_ProcessOnline() END!");

        return (VS_SUCCESS);
}

int inNCCC_ESC_AnalysePacket(TRANSACTION_OBJECT *pobTran)
{
        int				inRetVal = VS_SUCCESS, inISOFuncIndex = -1;
        ISO_TYPE_NCCC_ESC_TABLE 	srISOFunc;

        if (pobTran->srBRec.uszOfflineBit == VS_FALSE)
        {
                if (pobTran->inTransactionResult == _NCCC_ESC_AUTHORIZED_ || pobTran->inTransactionResult == _NCCC_ESC_REFERRAL_ || pobTran->inTransactionResult == _NCCC_ESC_SETTLE_UPLOAD_BATCH_)
                {
                        /* 加密模式，預設值 = 0。【0 = 不加密，1 = tSAM 加密，2 = 軟體加密】 */
                        inISOFuncIndex = 0; /* 不加密 */
                        memset((char *) &srISOFunc, 0x00, sizeof(srISOFunc));
                        memcpy((char *) &srISOFunc, (char *) &srNCCC_ESC_ISOFunc[inISOFuncIndex], sizeof(srISOFunc));

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

int inNCCC_ESC_CommSendRecvToHost(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, int inSendLen, unsigned char *uszRecvPacket)
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
	
	if (ginDebug == VS_TRUE)
	{
		vdNCCC_ESC_ISO_FormatDebug_DISP(uszSendPacket, inSendLen);
	}
	
	if (ginISODebug == VS_TRUE)
	{
		vdNCCC_ESC_ISO_FormatDebug_PRINT(uszSendPacket, inSendLen);
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
	
	if (ginDebug == VS_TRUE)
	{
		vdNCCC_ESC_ISO_FormatDebug_DISP(uszRecvPacket, inReceiveSize);
	}
	
	if (ginISODebug == VS_TRUE)
	{
		vdNCCC_ESC_ISO_FormatDebug_PRINT(uszRecvPacket, inReceiveSize);
	}
	
        return (VS_SUCCESS);
}

int inNCCC_ESC_SendPackRecvUnPack(TRANSACTION_OBJECT *pobTran)
{
        int 		inSendCnt = 0, inRecvCnt = 0, inRetVal;
        unsigned char 	uszSendPacket[_NCCC_ESC_ISO_SEND_ + 1], uszRecvPacket[_NCCC_ESC_ISO_RECV_ + 1];

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_SendPackRecvUnPack() START!");

        memset(uszSendPacket, 0x00, sizeof(uszSendPacket));
        memset(uszRecvPacket, 0x00, sizeof(uszRecvPacket));

        /* 組 ISO 電文 */
        if ((inSendCnt = inNCCC_ESC_PackISO(pobTran, uszSendPacket, pobTran->inISOTxnCode)) <= 0)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inNCCC_ESC_PackISO() Error!");

                return (_NCCC_ESC_PACK_ERR_); /* 組交易電文錯誤，不用組_REVERSAL_ */
        }
	
        /* 傳送及接收 ISO 電文 */
        if ((inRecvCnt = inNCCC_ESC_CommSendRecvToHost(pobTran, uszSendPacket, inSendCnt, uszRecvPacket)) != VS_SUCCESS)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inNCCC_ESC_CommSendRecvToHost() Error");

                if (pobTran->srBRec.uszOfflineBit == VS_TRUE)
                        memset(pobTran->srBRec.szRespCode, 0x00, sizeof(pobTran->srBRec.szRespCode));

                return (VS_ERROR);
        }
	
	
        /* 解 ISO 電文 */
        inRetVal = inNCCC_ESC_UnPackISO(pobTran, uszSendPacket, uszRecvPacket);
        if (inRetVal != VS_SUCCESS)
        {
                inRetVal = _NCCC_ESC_UNPACK_ERR_;
        }
	/* 上傳途中任一封包不接受 */
	else
	{
		if (memcmp(pobTran->srBRec.szRespCode, "00", 2))
		{
			/* 回覆碼不為"00"，表示有問題，先回拒絕 */
			inRetVal = VS_ESCAPE;
		}
		
	}
	
        return (inRetVal);
}

int inNCCC_ESC_DispHostResponseCode(TRANSACTION_OBJECT *pobTran)
{
	char	szCustomerIndicator[3 + 1] = {0};
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	/* 如果當SDK，不用顯示 */
	if (memcmp(gszTermVersionID, "MD731UAGAS001", strlen("MD731UAGAS001")) == 0)
	{
		
	}
	/* 在客製化107.111的時候將請按確認鍵跟請按清除鍵這兩個字樣移除 */
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)         ||
                 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_) ||
		 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)		||
		 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
	{
	
	}
	else
	{
		
	}
	
        return (VS_SUCCESS);
}

int inNCCC_ESC_SyncHostTerminalDateTime(TRANSACTION_OBJECT *pobTran)
{	
	char		szDebugMsg[100 + 1];
	unsigned short	usRet;
	CTOS_RTC	srSetRTC; /* Date & Time */
	
	/* date */
	memcpy(&srSetRTC.bYear, &pobTran->srBRec.szDate[2], 2);
	memcpy(&srSetRTC.bMonth, &pobTran->srBRec.szDate[4], 2);
	memcpy(&srSetRTC.bDay, &pobTran->srBRec.szDate[6], 2);
	
	/* time */
	memcpy(&srSetRTC.bHour, &pobTran->srBRec.szTime[0], 2);
	memcpy(&srSetRTC.bMinute, &pobTran->srBRec.szTime[2], 2);
	memcpy(&srSetRTC.bSecond, &pobTran->srBRec.szTime[4], 2);

	usRet = CTOS_RTCSet(&srSetRTC);
	
	if (usRet != d_OK)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "時間同步失敗 錯誤代碼：%x", usRet);
		inLogPrintf(AT, szDebugMsg);
		
		return (VS_ERROR);
	}

        return (VS_SUCCESS);
}

/*
Function        :inGetESC_Enable
Date&Time       :2016/4/15 下午 3:30
Describe        :傳HostEnable進來，會得到'Y' OR 'N'
*/
int inNCCC_ESC_GetESC_Enable(int inOrgHDTIndex, char *szHostEnable)
{
	/* 此function只用來查詢ESC是否開，不應該切換Host */
	/* 若之前已查詢到ESC的index為何，就可以直接使用 */
	if (ginESCHostIndex != -1)
	{
		/* 直接使用 */
	}
	else
	{
		if (inFunc_Find_Specific_HDTindex(inOrgHDTIndex, _HOST_NAME_ESC_, &ginESCHostIndex) != VS_SUCCESS)
		{
			/* 找不到直接return VS_ERROR */
			return (VS_ERROR);
		}
	}
	

	/* 如果Load失敗，回復原Host並回傳VS_ERROR(理論上不會有此情況) */
	if (inLoadHDTRec(ginESCHostIndex) < 0)
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
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "ESC Enable not open.");
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "ESC Enable open.");
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
Function        :inNCCC_ESC_SwitchToESC_Host
Date&Time       :2016/4/15 下午 3:30
Describe        :切換到HDT中ESC的REC上，之後要回覆就靠inOrgHDTIndex（在此function中回傳錯誤也會回覆原host）
*/
int inNCCC_ESC_SwitchToESC_Host(int inOrgHDTIndex)
{	
	/* 開機後只找一次ESC */
	if (ginESCHostIndex != -1)
	{
		/* 直接使用 */
	}
	else
	{
		if (inFunc_Find_Specific_HDTindex(inOrgHDTIndex, _HOST_NAME_ESC_, &ginESCHostIndex) != VS_SUCCESS)
		{
			/* 找不到直接return VS_ERROR */
			return (VS_ERROR);
		}
		
	}
	
	/* 如果Load失敗，回復原Host並回傳VS_ERROR(理論上不會有此情況) */
	if (inLoadHDTRec(ginESCHostIndex) < 0)
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
Function        :inNCCC_ESC_Data_Compress_Encryption
Date&Time       :2016/4/20 下午 4:40
Describe        :先壓縮再加密 
 *		將ESC_E1.txt		轉成ESC_E1.txt.gz並加密
 *		將簽名圖檔多複製一份	轉成ESC_E2.bmp.gz並加密
*/
int inNCCC_ESC_Data_Compress_Encryption(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_SUCCESS;
	int		inRetVal2 = VS_SUCCESS;
	int		inLen = 0;
	char		szFileName[16 + 1] = {0};
	char		szFileNameGzBmp[16 + 1] = {0};
	char		szTemplate[40 + 1] = {0};
	char		szRawData32[32 + 1] = {0};
	char		szRawData17_32[16 + 1] = {0};
	char		szEncryData[16 + 1] = {0};
	char		szEncryptionMode[2 + 1] = {0};
	char		szField37[8 + 1], szField41[4 + 1] = {0};
	char		szAscii[84 + 1] = {0};
	char		szDebugMsg[100 + 1] = {0};
	unsigned long	ulHandle = 0;
	unsigned char	uszSlot = 0;
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_Data_Compress_Encryption() START!");
	
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
	
	/* 預防漏砍，導致多送E2，這裡再預先砍一次 */
	inFILE_Delete((unsigned char*)_ESC_FILE_RECEIPT_GZ_ENCRYPTED_);
	inFILE_Delete((unsigned char*)_ESC_FILE_BMP_GZ_ENCRYPTED_);
	
/* 確認加密用 */
if (ginESCDebug == VS_TRUE)
{
	inFunc_Data_Copy(_ESC_FILE_RECEIPT_, _FS_DATA_PATH_, "", _AP_ROOT_PATH_);
}
	
	/* 壓縮加密檔案(GZIP) */
	
		/* 簽單 */
		if (inFunc_Data_GZip("", _ESC_FILE_RECEIPT_, _FS_DATA_PATH_) != VS_SUCCESS)
		{
			vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_Data_Compress_Encryption inFunc_Data_GZip_failed(%s)", _ESC_FILE_RECEIPT_);
			return (VS_ERROR);
		}
		
/* 確認加密用 */
if (ginESCDebug == VS_TRUE)
{
	inFunc_Data_Copy(_ESC_FILE_RECEIPT_GZ_, _FS_DATA_PATH_, "", _AP_ROOT_PATH_);
}
		
	/* 加密並置換資料內容(軟加密) */
		/* 寫入當筆要用的key */
		if (inNCCC_TMK_Write_ESCKey(pobTran) != VS_SUCCESS)
		{
			vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_Data_Compress_Encryption inNCCC_TMK_Write_ESCKey_failed");
			return (VS_ERROR);
		}
		
		/* 簽單 */
		if (inFILE_Open(&ulHandle, (unsigned char*)_ESC_FILE_RECEIPT_GZ_) != VS_SUCCESS)
		{
			vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_Data_Compress_Encryption inFILE_Open failed(%s)", _ESC_FILE_RECEIPT_GZ_);
			return (VS_ERROR);
		}
		
		inRetVal2 = VS_SUCCESS;
		do
		{
			if (inFILE_Seek(ulHandle, 0L, _SEEK_BEGIN_) != VS_SUCCESS)
			{
				vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_Data_Compress_Encryption inFILE_Seek failed");
				/* 跳出迴圈，關閉檔案 */
				inRetVal2 = VS_ERROR;
				break;
			}

			/* 取得前17到32 Bytes (szData為前32Byte) */
			memset(szRawData32, 0x00, sizeof(szRawData32));
			if (inFILE_Read(&ulHandle, (unsigned char*)szRawData32, 32) != VS_SUCCESS)
			{
				vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_Data_Compress_Encryption inFILE_Read failed");
				/* 跳出迴圈，關閉檔案 */
				inRetVal2 = VS_ERROR;
				break;
			}
			memset(szRawData17_32, 0x00, sizeof(szRawData17_32));
			memcpy(szRawData17_32, &szRawData32[16], 16);
			inLen = 32;	/* 32 nibble , 16Bytes */

			if (ginDebug == VS_TRUE)
			{
				memset(szAscii, 0x00, sizeof(szAscii));
				inFunc_BCD_to_ASCII(szAscii, (unsigned char*)szRawData17_32, inLen);
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				inLogPrintf(AT, "F_35 [szEncrypt ]");
				sprintf(szDebugMsg, "%s L:%d", szAscii, inLen);
				inLogPrintf(AT, szDebugMsg);
			}

			/* 加密 */
			/* tSAM加密用SAM加密，其餘用3DES */
			memset(szEncryptionMode, 0x00, sizeof(szEncryptionMode));
			inGetEncryptMode(szEncryptionMode);
			if (memcmp(szEncryptionMode, _NCCC_ENCRYPTION_TSAM_, strlen(_NCCC_ENCRYPTION_TSAM_)) == 0)
			{
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
					sprintf(szDebugMsg, "[field37: %s] %d", szAscii, 8);
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
					sprintf(szDebugMsg, "[field41: %s] %d", szAscii, 4);
					inLogPrintf(AT, szDebugMsg);
				}

				inRetVal = inNCCC_tSAM_Encrypt(uszSlot, 
							       inLen,
							       szRawData17_32,
							       (unsigned char*)&szField37,
							       (unsigned char*)&szField41,
							       &gusztSAMKeyIndex_ESC,
							       gusztSAMCheckSum_ESC57_E1);

				if (inRetVal == VS_ERROR)
				{
					vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_Data_Compress_Encryption inNCCC_tSAM_Encrypt failed");
					pobTran->inErrorMsg = _ERROR_CODE_V3_TSAM_ENCRYPT_FAIL_;
					/* 跳出迴圈，關閉檔案 */
					inRetVal2 = VS_ERROR;
					break;
				}
				else
				{
					memset(szEncryData, 0x00, sizeof(szEncryData));
					memcpy(szEncryData, szRawData17_32, inRetVal);
				}

			}
			else
			{
				memset(szEncryData, 0x00, sizeof(szEncryData));
				if (inNCCC_TMK_ESC_3DES_Encrypt(szRawData17_32, 16, szEncryData) != VS_SUCCESS)
				{
					vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_Data_Compress_Encryption inNCCC_TMK_ESC_3DES_Encrypt failed");
					/* 跳出迴圈，關閉檔案 */
					inRetVal2 = VS_ERROR;
					break;
				}
			}
			/* 取得加密後16 Bytes並塞回去 */
			memcpy(&szRawData32[16], szEncryData, 16);

			if (inFILE_Seek(ulHandle, 0L, _SEEK_BEGIN_) != VS_SUCCESS)
			{
				vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_Data_Compress_Encryption inFILE_Seek failed");
				/* 跳出迴圈，關閉檔案 */
				inRetVal2 = VS_ERROR;
				break;
			}
			if (inFILE_Write(&ulHandle, (unsigned char*)szRawData32, 32) != VS_SUCCESS)
			{
				/* 跳出迴圈，關閉檔案 */
				inRetVal2 = VS_ERROR;
				break;
			}
			break;
		}while(1);
		
		inFILE_Close(&ulHandle);
		
		/* 加密過程中有誤，先關再離開 */
		if (inRetVal2 != VS_SUCCESS)
		{
			return (inRetVal2);
		}
		
		if (inFILE_Rename((unsigned char*)_ESC_FILE_RECEIPT_GZ_, (unsigned char*)_ESC_FILE_RECEIPT_GZ_ENCRYPTED_) != VS_SUCCESS)
		{
			vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_Data_Compress_Encryption inFILE_Rename failed");
			return (VS_ERROR);
		}
		
	/* 壓縮加密檔案(GZIP) */
		/* 圖檔 */
		/* 藉由TRT_FileName比對來組出bmp的檔名 */
		if (inLoadHDPTRec(pobTran->srBRec.inHDTIndex) != VS_SUCCESS)
		{
			vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_Data_Compress_Encryption inLoadHDPTRec(%d)", pobTran->srBRec.inHDTIndex);
			return (VS_ERROR);
		}
		
		memset(szFileName, 0x00, sizeof(szFileName));
		/* 因為用invoice所以不用inFunc_ComposeFileName */
		inFunc_ComposeFileName_InvoiceNumber(pobTran, szFileName, _PICTURE_FILE_EXTENSION_, 6);
		
		/* 如果檔案不存在，就不壓 */
		if (inFILE_Check_Exist((unsigned char*)szFileName) == VS_SUCCESS)
		{
			/* 備份起來，因為Gzip會不留原檔 */
			inFunc_Data_Copy(szFileName, _FS_DATA_PATH_, _ESC_FILE_BMP_BACKUP_, _FS_DATA_PATH_);
			
			if (inFunc_Data_GZip("", szFileName, _FS_DATA_PATH_) != VS_SUCCESS)
			{
				vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_Data_Compress_Encryption inFunc_Data_GZip(%s)", szFileName);
				return (VS_ERROR);
			}
			
			/* 還原 */
			inFunc_Data_Rename(_ESC_FILE_BMP_BACKUP_, _FS_DATA_PATH_, szFileName, _FS_DATA_PATH_);
			
			memset(szFileNameGzBmp, 0x00, sizeof(szFileNameGzBmp));
			sprintf(szFileNameGzBmp, "%s%s", szFileName, _GZIP_FILE_EXTENSION_);
			
			/* 當筆的一律用_ESC_UPLOAD_BMP_GZ_ */
			if (inFILE_Rename((unsigned char*)szFileNameGzBmp, (unsigned char*)_ESC_FILE_BMP_GZ_) != VS_SUCCESS)
			{
				vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_Data_Compress_Encryption inFILE_Rename_failed(%s)(%s)", szFileNameGzBmp, _ESC_FILE_BMP_GZ_);
				return (VS_ERROR);
			}
			
/* 確認加密用 */
if (ginESCDebug == VS_TRUE)
{
	inFunc_Data_Copy(_ESC_FILE_BMP_GZ_, _FS_DATA_PATH_, "", _AP_ROOT_PATH_);

}
			
		/* 加密並置換資料內容(軟加密) */
			if (inFILE_Open(&ulHandle, (unsigned char*)_ESC_FILE_BMP_GZ_) != VS_SUCCESS)
			{
				vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_Data_Compress_Encryption inFILE_Open_failed(%s)", _ESC_FILE_BMP_GZ_);
				return (VS_ERROR);
			}

			inRetVal2 = VS_SUCCESS;
			do
			{
				if (inFILE_Seek(ulHandle, 0L, _SEEK_BEGIN_) != VS_SUCCESS)
				{
					vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_Data_Compress_Encryption inFILE_Seek_failed");
					/* 跳出迴圈，關閉檔案 */
					inRetVal2 = VS_ERROR;
					break;
				}

				/* 取得前17到32 Bytes (szData為前32Byte) */
				memset(szRawData32, 0x00, sizeof(szRawData32));
				if (inFILE_Read(&ulHandle, (unsigned char*)szRawData32, 32) != VS_SUCCESS)
				{
					vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_Data_Compress_Encryption inFILE_Read_failed");
					/* 跳出迴圈，關閉檔案 */
					inRetVal2 = VS_ERROR;
					break;
				}
				memset(szRawData17_32, 0x00, sizeof(szRawData17_32));
				memcpy(szRawData17_32, &szRawData32[16], 16);
				inLen = 32;		/* 32 nibble , 16Bytes */

				if (ginDebug == VS_TRUE)
				{
					memset(szAscii, 0x00, sizeof(szAscii));
					inFunc_BCD_to_ASCII(szAscii, (unsigned char*)szRawData17_32, inLen);
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					inLogPrintf(AT, "F_35 [szEncrypt ]");
					sprintf(szDebugMsg, "%s L:%d", szAscii, inLen);
					inLogPrintf(AT, szDebugMsg);
				}

				/* 加密 */
				/* tSAM加密用SAM加密，其餘用3DES */
				memset(szEncryptionMode, 0x00, sizeof(szEncryptionMode));
				inGetEncryptMode(szEncryptionMode);
				if (memcmp(szEncryptionMode, _NCCC_ENCRYPTION_TSAM_, strlen(_NCCC_ENCRYPTION_TSAM_)) == 0)
				{
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
						sprintf(szDebugMsg, "[field37: %s] %d", szAscii, 8);
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
						sprintf(szDebugMsg, "[field41: %s] %d", szAscii, 4);
						inLogPrintf(AT, szDebugMsg);
					}

					inRetVal = inNCCC_tSAM_Encrypt(uszSlot, 
								       inLen,
								       szRawData17_32,
								       (unsigned char*)&szField37,
								       (unsigned char*)&szField41,
								       &gusztSAMKeyIndex_ESC,
								       gusztSAMCheckSum_ESC57_E2);

					if (inRetVal == VS_ERROR)
					{
						vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_Data_Compress_Encryption inNCCC_tSAM_Encrypt_failed");
						pobTran->inErrorMsg = _ERROR_CODE_V3_TSAM_ENCRYPT_FAIL_;
						/* 跳出迴圈，關閉檔案 */
						inRetVal2 = VS_ERROR;
						break;
					}
					else
					{
						memset(szEncryData, 0x00, sizeof(szEncryData));
						memcpy(szEncryData, szRawData17_32, inRetVal);
					}

				}
				else
				{
					memset(szEncryData, 0x00, sizeof(szEncryData));
					if (inNCCC_TMK_ESC_3DES_Encrypt(szRawData17_32, 16, szEncryData) != VS_SUCCESS)
					{
						vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_Data_Compress_Encryption inNCCC_TMK_ESC_3DES_Encrypt_failed");
						/* 跳出迴圈，關閉檔案 */
						inRetVal2 = VS_ERROR;
						break;
					}
				}

				/* 取得加密後16 Bytes並塞回去 */
				memcpy(&szRawData32[16], szEncryData, 16);
				if (inFILE_Seek(ulHandle, 0L, _SEEK_BEGIN_) != VS_SUCCESS)
				{
					vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_Data_Compress_Encryption inFILE_Seek_failed");
					/* 跳出迴圈，關閉檔案 */
					inRetVal2 = VS_ERROR;
					break;
				}
				if (inFILE_Write(&ulHandle, (unsigned char*)szRawData32, 32) != VS_SUCCESS)
				{
					vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_Data_Compress_Encryption inFILE_Write_failed");
					/* 跳出迴圈，關閉檔案 */
					inRetVal2 = VS_ERROR;
					break;
				}
				break;
			}while(1);
		
			inFILE_Close(&ulHandle);

			/* 加密過程中有誤，先關再離開 */
			if (inRetVal2 != VS_SUCCESS)
			{
				return (inRetVal2);
			}
			
			if (inFILE_Rename((unsigned char*)_ESC_FILE_BMP_GZ_, (unsigned char*)_ESC_FILE_BMP_GZ_ENCRYPTED_) != VS_SUCCESS)
			{
				vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_Data_Compress_Encryption inFILE_Rename_failed(%s)(%s)", _ESC_FILE_BMP_GZ_, _ESC_FILE_BMP_GZ_ENCRYPTED_);
				return (VS_ERROR);
			}
			
		}/* 圖檔壓縮END*/
		/* 圖檔 */
		
if (ginESCDebug == VS_TRUE)
{
	inFunc_Data_Copy(_ESC_FILE_RECEIPT_GZ_ENCRYPTED_, _FS_DATA_PATH_, "", _AP_ROOT_PATH_);
	inFunc_Data_Copy(_ESC_FILE_BMP_GZ_ENCRYPTED_, _FS_DATA_PATH_, "", _AP_ROOT_PATH_);
}
				
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_Data_Compress_Encryption() END!");
		
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_ESC_Data_Packet
Date&Time       :2016/4/21 下午 4:41
Describe        :將壓縮加密完的data切塊並塞進ESC_DATA中
*/
int inNCCC_ESC_Data_Packet(char *szFileName, int inTableIndex)
{
	int		i = 0;
	int		inRetVal = VS_SUCCESS;
	int		inTableStart = 0;				/* 當切完E1後，E2要從裡開始接 */
	int		inTotalCnt = 0;				/* 總封包數 */
	long		lnFileSize = 0, lnRemainSize = 0;	/* 檔案長度、 剩餘長度 */
	char		szDebugMsg[100 + 1] = {0};
	unsigned long	ulHandle = 0;
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_Data_Packet() START!");

	/* E2從哪裡繼續append */
	inTableStart = ginEscDataIndex;
	inRetVal = inFILE_OpenReadOnly(&ulHandle, (unsigned char*)szFileName);

	if (inRetVal == VS_SUCCESS)
	{
		lnFileSize = lnFILE_GetSize(&ulHandle, (unsigned char*)szFileName);
		/* 設定剩餘長度 */
		lnRemainSize = lnFileSize;
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "%s size: %ld", szFileName, lnFileSize);
			inLogPrintf(AT, szDebugMsg);
		}

		/* 移到檔案最前面 */
		inFILE_Seek(ulHandle, 0L, _SEEK_BEGIN_);

		/* E1 or E2的總封包數 */
		inTotalCnt = (lnFileSize / _ESC_ISO_MAX_LEN_);
		/* 如果不滿一個封包，要再加一 */
		if (lnFileSize > ((lnFileSize / _ESC_ISO_MAX_LEN_) * _ESC_ISO_MAX_LEN_))
			inTotalCnt += 1;
				
		/* i表示第幾個封包 */
		for (i = 1 ;; i++)
		{
			
			ESC_UPLOAD_DATA[ginEscDataIndex].inTableIndex = inTableIndex;
			ESC_UPLOAD_DATA[ginEscDataIndex].inTotalPacketCnt = inTotalCnt;
			ESC_UPLOAD_DATA[ginEscDataIndex].inPacketCnt = i;
			ESC_UPLOAD_DATA[ginEscDataIndex].lnTotalPacketSize = lnFileSize;
			
			/* 清空 */
			memset(ESC_UPLOAD_DATA[ginEscDataIndex].szPackData, 0x00, sizeof(ESC_UPLOAD_DATA[ginEscDataIndex].szPackData));

			/* 如果剩餘長度小於ESC_ISO_MAX_LEN，就只讀剩下的 */
			if ((lnRemainSize - _ESC_ISO_MAX_LEN_) <= 0)
			{
				/* 讀出長度並設定structure中的封包長度 */
				if (inFILE_Read(&ulHandle, (unsigned char*)ESC_UPLOAD_DATA[ginEscDataIndex].szPackData, lnRemainSize) != VS_SUCCESS)
				{
					vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_Data_Packet(%s)(%d) inFILE_Read_failed", szFileName, inTableIndex);
					/* [BUG] 移除檔案同步API SYNC()指令     
					 * [20260506] ESC包檔失敗要關檔案*/
					inFILE_Close(&ulHandle);
					return (VS_ERROR);
				}
				ESC_UPLOAD_DATA[ginEscDataIndex].inPacketSize = lnRemainSize;
				
				if (ginDebug == VS_TRUE)
				{
					char szDebugMsg[100 + 1];

					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "%d %d %d %ld %d", ESC_UPLOAD_DATA[ginEscDataIndex].inTableIndex, ESC_UPLOAD_DATA[ginEscDataIndex].inTotalPacketCnt, ESC_UPLOAD_DATA[ginEscDataIndex].inPacketCnt, ESC_UPLOAD_DATA[ginEscDataIndex].lnTotalPacketSize, ESC_UPLOAD_DATA[ginEscDataIndex].inPacketSize);
					inLogPrintf(AT, szDebugMsg);
				}
				/* 總封包數(E1 + E2)加一 */
				ginEscDataIndex ++;
				
				/* 減去讀出長度 */
				lnRemainSize -= lnRemainSize;
				
				
				
				break;
			}
			else
			{
				/* 讀出長度並設定structure中的封包長度 */
				if (inFILE_Read(&ulHandle, (unsigned char*)ESC_UPLOAD_DATA[ginEscDataIndex].szPackData, _ESC_ISO_MAX_LEN_) != VS_SUCCESS)
				{
					vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_Data_Packet(%s)(%d) inFILE_Read_failed", szFileName, inTableIndex);
					/* [BUG] 移除檔案同步API SYNC()指令     
					 * [20260506] ESC包檔失敗要關檔案*/
					inFILE_Close(&ulHandle);
					return (VS_ERROR);
				}
				ESC_UPLOAD_DATA[ginEscDataIndex].inPacketSize = _ESC_ISO_MAX_LEN_;
				
				if (ginDebug == VS_TRUE)
				{
					char szDebugMsg[100 + 1];

					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "%d %d %d %ld %d", ESC_UPLOAD_DATA[ginEscDataIndex].inTableIndex, ESC_UPLOAD_DATA[ginEscDataIndex].inTotalPacketCnt, ESC_UPLOAD_DATA[ginEscDataIndex].inPacketCnt, ESC_UPLOAD_DATA[ginEscDataIndex].lnTotalPacketSize, ESC_UPLOAD_DATA[ginEscDataIndex].inPacketSize);
					inLogPrintf(AT, szDebugMsg);
				}
				/* 總封包數(E1 + E2)加一 */
				ginEscDataIndex ++;
				
				/* 減去讀出長度 */
				lnRemainSize -= _ESC_ISO_MAX_LEN_;
			}
			
			
			
		}
		
		/* 關檔 */
		inFILE_Close(&ulHandle);
	}
	else
	{
		vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_Data_Packet(%s)(%d) inFILE_OpenReadOnly_failed", szFileName, inTableIndex);
		/* 一樣，如果檔案開啟已經失敗，是否需要多做一次關檔 */
		/* 關檔 */
		inFILE_Close(&ulHandle);
		
		return (VS_ERROR);
	}

	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_Data_Packet() END!");

	return (VS_SUCCESS);
}

/*
Function        :inNCCC_ESC_Insert_Again_Record
Date&Time       :2017/3/23 上午 10:12
Describe        :建立各自的TABLE NAME ，例如主機名稱+6位數的批次號_AGAIN 和 主機名稱+6位數的批次號_AGAINEMV,
 *				 insert 當筆srBRec 並取得該筆的TableID，如果有EMV資料，同樣insert 該筆srEMVRec
*/
int inNCCC_ESC_Insert_Again_Record(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	char	szTableID[12 + 1] = {0};
	char	szDebugMsg[100 + 1] = {0};
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_ESC_Insert_Again_Record() START !");
	}
	
	inRetVal = inBATCH_Create_BatchTable_Flow(pobTran, _TN_BATCH_TABLE_ESC_AGAIN_);
	if (inRetVal != VS_SUCCESS)
	{
		vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_Insert_Again_Record inNCCC_ESC_Insert_Again_Record_create_table");
		inFunc_EDCLock(AT);
		return (inRetVal);
	}
	
	inRetVal = inBATCH_Create_BatchTable_Flow(pobTran, _TN_BATCH_TABLE_ESC_AGAIN_EMV_);
	if (inRetVal != VS_SUCCESS)
	{
		vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_Insert_Again_Record _TN_BATCH_TABLE_ESC_AGAIN_EMV_create_table");
		inFunc_EDCLock(AT);
	}
	
	inRetVal = inBATCH_Insert_All_Flow(pobTran, _TN_BATCH_TABLE_ESC_AGAIN_);
	if (inRetVal != VS_SUCCESS)
	{
		vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_Insert_Again_Record _TN_BATCH_TABLE_ESC_AGAIN_insert_failed");
		inFunc_EDCLock(AT);
		return (inRetVal);
	}
	
	memset(szTableID, 0x00, sizeof(szTableID));
	/* 抓出TableID */
	inRetVal = inSqlite_Get_Max_TableID_Flow(pobTran, _TN_BATCH_TABLE_ESC_AGAIN_, szTableID);
	if (inRetVal != VS_SUCCESS)
	{
		vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_Insert_Again_Record _TN_BATCH_TABLE_ESC_AGAIN_get_max_table_ID");
		inFunc_EDCLock(AT);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inTableID : %d", atoi(szTableID));
			inLogPrintf(AT, szDebugMsg);
		}
		pobTran->inTableID = atoi(szTableID);
	}
	
	/* 算出是否有【EMV】和【Contactless】交易 */
	if (pobTran->srBRec.inChipStatus != 0			|| 
	    pobTran->srBRec.uszContactlessBit == VS_TRUE	|| 
	    pobTran->srBRec.uszEMVFallBackBit == VS_TRUE)
	{
		inRetVal = inBATCH_Insert_All_Flow(pobTran, _TN_BATCH_TABLE_ESC_AGAIN_EMV_);
		if (inRetVal != VS_SUCCESS)
		{
			vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_Insert_Again_Record _TN_BATCH_TABLE_ESC_AGAIN_EMV_insert_failed");
			inFunc_EDCLock(AT);
		}
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_ESC_Insert_Again_Record() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :inNCCC_ESC_Insert_Fail_Record
Date&Time       :2017/3/23 上午 10:12
Describe        :
*/
int inNCCC_ESC_Insert_Fail_Record(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	char	szTableID[12 + 1] = {0};
	char	szDebugMsg[100 + 1] = {0};
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_ESC_Insert_Fail_Record() START !");
	}
	
	inRetVal = inBATCH_Create_BatchTable_Flow(pobTran, _TN_BATCH_TABLE_ESC_FAIL_);
	if (inRetVal != VS_SUCCESS)
	{
		vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_Insert_Fail_Record _TN_BATCH_TABLE_ESC_FAIL_create_table");
		inFunc_EDCLock(AT);
		return (inRetVal);
	}
	
	inRetVal = inBATCH_Create_BatchTable_Flow(pobTran, _TN_BATCH_TABLE_ESC_FAIL_EMV_);
	if (inRetVal != VS_SUCCESS)
	{
		vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_Insert_Fail_Record _TN_BATCH_TABLE_ESC_FAIL_EMV_create_table");
		inFunc_EDCLock(AT);
	}
	
	inRetVal = inBATCH_Insert_All_Flow(pobTran, _TN_BATCH_TABLE_ESC_FAIL_);
	if (inRetVal != VS_SUCCESS)
	{
		vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_Insert_Fail_Record _TN_BATCH_TABLE_ESC_FAIL_insert_failed");
		inFunc_EDCLock(AT);
		return (inRetVal);
	}
	
	memset(szTableID, 0x00, sizeof(szTableID));
	/* 抓出TableID */
	inRetVal = inSqlite_Get_Max_TableID_Flow(pobTran, _TN_BATCH_TABLE_ESC_FAIL_, szTableID);
	if (inRetVal != VS_SUCCESS)
	{
		vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_Insert_Fail_Record _TN_BATCH_TABLE_ESC_FAIL_get_max_tableID_failed");
		inFunc_EDCLock(AT);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inTableID : %d", atoi(szTableID));
			inLogPrintf(AT, szDebugMsg);
		}
		/* 如果要存EMV Record會用到這個inTableID */
		pobTran->inTableID = atoi(szTableID);
	}
	
	/* 算出是否有【EMV】和【Contactless】交易 */
	if (pobTran->srBRec.inChipStatus != 0			|| 
	    pobTran->srBRec.uszContactlessBit == VS_TRUE	|| 
	    pobTran->srBRec.uszEMVFallBackBit == VS_TRUE)
	{
		inRetVal = inBATCH_Insert_All_Flow(pobTran, _TN_BATCH_TABLE_ESC_FAIL_EMV_);
		if (inRetVal != VS_SUCCESS)
		{
			vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_Insert_Fail_Record _TN_BATCH_TABLE_ESC_FAIL_EMV_insert_failed");
			inFunc_EDCLock(AT);
		}
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_ESC_Insert_Fail_Record() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :inNCCC_ESC_Get_Again_Record_Most_TOP
Date&Time       :2017/3/23 上午 10:45
Describe        :取ESC資料庫中最上面一筆
*/
int inNCCC_ESC_Get_Again_Record_Most_TOP(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_ESC_Get_Again_Record_Most_TOP() START !");
	}
	
	inRetVal = inSqlite_ESC_Get_BRec_Top_Flow(pobTran, _TN_BATCH_TABLE_ESC_AGAIN_);
	if (inRetVal != VS_SUCCESS)
	{
		vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_Get_Again_Record_Most_TOP _TN_BATCH_TABLE_ESC_AGAIN_Get_BRec_TOP failed");
		return (inRetVal);
	}
	
	if (pobTran->srBRec.inChipStatus != 0			|| 
	    pobTran->srBRec.uszContactlessBit == VS_TRUE	|| 
	    pobTran->srBRec.uszEMVFallBackBit == VS_TRUE)
	{
		inRetVal = inSqlite_ESC_Get_BRec_Top_Flow(pobTran, _TN_BATCH_TABLE_ESC_AGAIN_EMV_);
		if (inRetVal != VS_SUCCESS)
		{
			vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_Get_Again_Record_Most_TOP _TN_BATCH_TABLE_ESC_AGAIN_EMV_Get_BRec_TOP failed");
			return (inRetVal);
		}
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_ESC_Get_Again_Record_Most_TOP() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :inNCCC_ESC_Get_Fail_Record_Most_TOP
Date&Time       :2017/3/23 下午 5:24
Describe        :取ESC資料庫中最上面一筆
*/
int inNCCC_ESC_Get_Fail_Record_Most_TOP(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_ESC_Get_Fail_Record_Most_TOP() START !");
	}
	
	inRetVal = inSqlite_ESC_Get_BRec_Top_Flow(pobTran, _TN_BATCH_TABLE_ESC_FAIL_);
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
	if (pobTran->srBRec.inChipStatus != 0			|| 
	    pobTran->srBRec.uszContactlessBit == VS_TRUE	|| 
	    pobTran->srBRec.uszEMVFallBackBit == VS_TRUE)
	{
		inRetVal = inSqlite_ESC_Get_BRec_Top_Flow(pobTran, _TN_BATCH_TABLE_ESC_FAIL_EMV_);
		if (inRetVal != VS_SUCCESS)
		{
			return (inRetVal);
		}
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_ESC_Get_Fail_Record_Most_TOP() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :inNCCC_ESC_Delete_Again_Record_Most_TOP
Date&Time       :2017/3/23 上午 10:45
Describe        :刪除ESC資料庫中最上面一筆
*/
int inNCCC_ESC_Delete_Again_Record_Most_TOP(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_ESC_Delete_Again_Record_Most_TOP() START !");
	}
	
	inRetVal = inSqlite_ESC_Delete_Record_Flow(pobTran, _TN_BATCH_TABLE_ESC_AGAIN_);
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_ESC_Delete_Again_Record_Most_TOP() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :inNCCC_ESC_Delete_Fail_Record_Most_TOP
Date&Time       :2017/3/23 上午 10:45
Describe        :刪除ESC資料庫中最上面一筆
*/
int inNCCC_ESC_Delete_Fail_Record_Most_TOP(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_ESC_Delete_Fail_Record_Most_TOP() START !");
	}
	
	inRetVal = inSqlite_ESC_Delete_Record_Flow(pobTran, _TN_BATCH_TABLE_ESC_FAIL_);
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_ESC_Delete_Fail_Record_Most_TOP() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :inNCCC_ESC_Again_Table_Count
Date&Time       :2017/3/23 下午 5:28
Describe        :回傳Table資料筆數
*/
int inNCCC_ESC_Again_Table_Count(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;
	int	inAgain = -1;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_ESC_Again_Table_Count() START !");
	}
	
	inRetVal = inSqlite_Get_Table_Count_Flow(pobTran, _TN_BATCH_TABLE_ESC_AGAIN_, &inAgain);
	if (inRetVal == VS_NO_RECORD)
	{
		inAgain = 0;
	}
	else if (inRetVal == VS_ERROR)
	{
		inAgain = 0;
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_ESC_Again_Table_Count() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inAgain);
}

/*
Function        :inNCCC_ESC_Fail_Table_Count
Date&Time       :2017/3/23 下午 5:28
Describe        :回傳Table資料筆數
*/
int inNCCC_ESC_Fail_Table_Count(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;
	int	inFail = -1;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_ESC_Fail_Table_Count() START !");
	}
	
	inRetVal = inSqlite_Get_Table_Count_Flow(pobTran, _TN_BATCH_TABLE_ESC_FAIL_, &inFail);
	if (inRetVal == VS_NO_RECORD)
	{
		inFail = 0;
	}
	else if (inRetVal == VS_ERROR)
	{
		inFail = 0;
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_ESC_Fail_Table_Count() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inFail);
}

/*
Function        :inNCCC_ESC_Process_PowerOff_When_Signing
Date&Time       :2018/3/23 下午 3:59
Describe        :1.有開簽名版才需重印
 *		 2.有開電簽上傳才需補統計bypass
*/
int inNCCC_ESC_Process_PowerOff_When_Signing(TRANSACTION_OBJECT *pobTran)
{
	int			inCount = -1;
	int			inRetVal = VS_SUCCESS;
	char			szESCMode[2 + 1] = {0};
	char			szSignpadMode[2 + 1] = {0};
	TRANSACTION_OBJECT	pobESCTran;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_ESC_Process_PowerOff_When_Signing() START !");
	}
		
	memset(szESCMode, 0x00, sizeof(szESCMode));
	inGetESCMode(szESCMode);
	
	memset(szSignpadMode, 0x00, sizeof(szSignpadMode));
	inGetSignPadMode(szSignpadMode);
	
	memset(&pobESCTran, 0x00, sizeof(TRANSACTION_OBJECT));
	
	/* 先確認是否有暫存紀錄 */
	pobESCTran.uszFileNameNoNeedHostBit = VS_TRUE;
	pobESCTran.uszFileNameNoNeedNumBit = VS_TRUE;
	inRetVal = inSqlite_Get_Table_Count_Flow(&pobESCTran, _TN_BATCH_TABLE_ESC_TEMP_, &inCount);
	pobESCTran.uszFileNameNoNeedHostBit = VS_FALSE;
	pobESCTran.uszFileNameNoNeedNumBit = VS_FALSE;
	if (inRetVal == VS_NO_RECORD)
	{
		inCount = 0;
	}
	else if (inRetVal == VS_ERROR)
	{
		inCount = 0;
	}
	
	if (inCount > 0)
	{
		pobESCTran.uszFileNameNoNeedHostBit = VS_TRUE;
		pobESCTran.uszFileNameNoNeedNumBit = VS_TRUE;
		inRetVal = inSqlite_ESC_Get_BRec_Top_Flow(&pobESCTran, _TN_BATCH_TABLE_ESC_TEMP_);
		if (inRetVal != VS_SUCCESS)
		{
			inUtility_StoreTraceLog_OneStep("inSqlite_ESC_Get_BRec_Top_Flow ESC_TEMP Failed");
			vdUtility_SYSFIN_LogMessage(AT, "inSqlite_ESC_Get_BRec_Top_Flow ESC_TEMP Failed");
		}
		pobESCTran.uszFileNameNoNeedHostBit = VS_FALSE;
		pobESCTran.uszFileNameNoNeedNumBit = VS_FALSE;
		if (inRetVal == VS_SUCCESS)
		{
			if (pobTran->srBRec.inChipStatus != 0			|| 
			    pobTran->srBRec.uszContactlessBit == VS_TRUE	|| 
			    pobTran->srBRec.uszEMVFallBackBit == VS_TRUE)
			{
				pobESCTran.uszFileNameNoNeedHostBit = VS_TRUE;
				pobESCTran.uszFileNameNoNeedNumBit = VS_TRUE;
				inRetVal = inSqlite_ESC_Get_BRec_Top_Flow(&pobESCTran, _TN_BATCH_TABLE_ESC_TEMP_EMV_);
				if (inRetVal != VS_SUCCESS)
				{
					inUtility_StoreTraceLog_OneStep("inSqlite_ESC_Get_BRec_Top_Flow ESC_TEMP_EMV Failed");
					vdUtility_SYSFIN_LogMessage(AT, "inSqlite_ESC_Get_BRec_Top_Flow ESC_TEMP_EMV Failed");
				}
				pobESCTran.uszFileNameNoNeedHostBit = VS_FALSE;
				pobESCTran.uszFileNameNoNeedNumBit = VS_FALSE;
			}
			
			inLoadHDTRec(pobESCTran.srBRec.inHDTIndex);
			inLoadHDPTRec(pobESCTran.srBRec.inHDTIndex);
			/* 如果是沒簽的狀態就重印簽單 */
			pobESCTran.srBRec.inSignStatus = _SIGN_BYPASS_;
			/* 要改為註記為紙本 */
			pobESCTran.srBRec.inESCUploadStatus = _ESC_UPLOAD_STATUS_PAPER_;

			/* 內建簽名板，才要重印 */
			if (memcmp(szSignpadMode, _SIGNPAD_MODE_1_INTERNAL_, strlen(_SIGNPAD_MODE_1_INTERNAL_)) == 0)
			{
				inRetVal = inFunc_RePrintReceipt_ByBuffer_Flow(&pobESCTran);
				if (inRetVal != VS_SUCCESS)
				{
					inUtility_StoreTraceLog_OneStep("inBATCH_Update_Sign_Status_By_Sqlite Failed");
					vdUtility_SYSFIN_LogMessage(AT, "inBATCH_Update_Sign_Status_By_Sqlite Failed");
				}
			}

			inRetVal = inBATCH_Update_Sign_Status_By_Sqlite(&pobESCTran);
			if (inRetVal != VS_SUCCESS)
			{
				inUtility_StoreTraceLog_OneStep("inBATCH_Update_Sign_Status_By_Sqlite Failed");
				vdUtility_SYSFIN_LogMessage(AT, "inBATCH_Update_Sign_Status_By_Sqlite Failed");
			}
			inRetVal = inBATCH_Update_ESC_Upload_Status_By_Sqlite(&pobESCTran);
			if (inRetVal != VS_SUCCESS)
			{
				inUtility_StoreTraceLog_OneStep("inBATCH_Update_ESC_Upload_Status_By_Sqlite Failed");
			}
			
			if (memcmp(szESCMode, "Y", strlen("Y")) == 0)
			{
				/* 補統計筆數 */
				inRetVal = inACCUM_UpdateFlow_ESC(&pobESCTran, _ESC_ACCUM_STATUS_BYPASS_);
				if (inRetVal != VS_SUCCESS)
				{
					inUtility_StoreTraceLog_OneStep("inACCUM_UpdateFlow_ESC Failed");
				}
			}


			/* 不支援就不用送了 */
			if (pobESCTran.srBRec.inESCUploadMode == _ESC_STATUS_NOT_SUPPORTED_)
			{

			}
			else
			{
				inRetVal = inBATCH_ESC_Save_Advice_Flow(&pobESCTran);
				if (inRetVal != VS_SUCCESS)
				{
					inUtility_StoreTraceLog_OneStep("inBATCH_ESC_Save_Advice_Flow Failed");
				}
			}
		}

		/* 刪除暫存紀錄，代表已完成統計流程 */
		/* ESC暫存檔 */
		pobESCTran.uszFileNameNoNeedHostBit = VS_TRUE;
		pobESCTran.uszFileNameNoNeedNumBit = VS_TRUE;
		inRetVal = inSqlite_ESC_Delete_Record_Flow(&pobESCTran, _TN_BATCH_TABLE_ESC_TEMP_);
		pobESCTran.uszFileNameNoNeedHostBit = VS_FALSE;
		pobESCTran.uszFileNameNoNeedNumBit = VS_FALSE;
		if (inRetVal != VS_SUCCESS)
		{
			inUtility_StoreTraceLog_OneStep("Delete_ESC_Temp_Fail");
		}
		
		if (pobTran->srBRec.inHDTIndex >= 0)
		{
			inLoadHDTRec(pobTran->srBRec.inHDTIndex);
			inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
		}
		else
		{
			inLoadHDTRec(0);
			inLoadHDPTRec(0);
		}
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_ESC_Process_PowerOff_When_Signing() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_ESC_Func_Upload_Idle_Check_Time
Date&Time       :2018/5/23 上午 9:33
Describe        :
*/
int inNCCC_ESC_Func_Upload_Idle_Check_Time(void)
{
	int		inEDCTime = 0, inCheckTime = 0, inTimeout = 300;	/* timeout 失敗等五分鐘 (300秒) */
	char		szTimeTemp[2 + 1];
	unsigned char	uszTimeup = VS_FALSE;
	RTC_NEXSYS	srRTC;
	
	/* Idle等待 */
	/* 全為0 還沒開始計時 設定完起始時間 直接跳出 */
	if (atoi(gszESC_Date) == 0 && atoi(gszESC_Time) == 0)
	{
		/* 取得EDC時間日期 */
		memset(&srRTC, 0x00, sizeof(RTC_NEXSYS));
		if (inFunc_GetSystemDateAndTime(&srRTC) != VS_SUCCESS)
		{

		}
		else
		{
			memset(&gszESC_Date, 0x00, sizeof(gszESC_Date));
			memset(&gszESC_Time, 0x00, sizeof(gszESC_Time));
			sprintf(gszESC_Date, "20%02d%02d%02d",  srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
			sprintf(gszESC_Time, "%02d%02d%02d",  srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
		}

		uszTimeup = VS_FALSE;
	}
	else
	{
		memset(&srRTC, 0x00, sizeof(RTC_NEXSYS));
		/* 取得EDC時間日期 */
		if (inFunc_GetSystemDateAndTime(&srRTC) != VS_SUCCESS)
		{
			uszTimeup = VS_FALSE;
		}
		else
		{
			memset(szTimeTemp, 0x00, sizeof(szTimeTemp));
			memcpy(&szTimeTemp, &gszESC_Date[2], 2);
			/* 跨年就更新時間 */
			if (srRTC.uszYear != atoi(szTimeTemp))
			{
				uszTimeup = VS_FALSE;
			}
			else
			{
				memset(szTimeTemp, 0x00, sizeof(szTimeTemp));
				memcpy(&szTimeTemp, &gszESC_Date[4], 2);
				/* 跨月就更新時間 */
				if (srRTC.uszMonth != atoi(szTimeTemp))
				{
					uszTimeup = VS_FALSE;
				}
				else
				{
					memset(szTimeTemp, 0x00, sizeof(szTimeTemp));
					memcpy(&szTimeTemp, &gszESC_Date[6], 2);
					/* 跨日就更新時間 */
					if (srRTC.uszDay != atoi(szTimeTemp))
					{
						uszTimeup = VS_FALSE;
					}
					/* 同日 */
					else
					{
						/* 時 */
						memset(szTimeTemp, 0x00, sizeof(szTimeTemp));
						memcpy(&szTimeTemp, gszESC_Time, 2);
						inCheckTime += atoi(szTimeTemp) * 60 * 60;
						inEDCTime += srRTC.uszHour * 60 * 60;

						/* 分 */
						memset(szTimeTemp, 0x00, sizeof(szTimeTemp));
						memcpy(&szTimeTemp, &gszESC_Time[2], 2);
						inCheckTime += atoi(szTimeTemp) * 60;
						inEDCTime += srRTC.uszMinute * 60;

						/* 秒 */
						memset(szTimeTemp, 0x00, sizeof(szTimeTemp));
						memcpy(&szTimeTemp, &gszESC_Time[4], 2);
						inCheckTime += atoi(szTimeTemp);
						inEDCTime += srRTC.uszSecond;


						if (inEDCTime >= (inCheckTime + inTimeout))
							uszTimeup = VS_TRUE;
						else
							uszTimeup = VS_FALSE;

					}/* 跨日 */

				}/* 垮月 */

			}/* 跨年 */
		}

	}/* 確認是否預設值 */
	
	if (uszTimeup == VS_TRUE)
	{
		return (VS_SUCCESS);
	}
	else
	{
		return (VS_ERROR);
	}
}

/*
Function        :vdNCCC_ESC_ISOFormatDebug_DISP
Date&Time       :2016/12/1 上午 9:41
Describe        :
*/
void vdNCCC_ESC_ISO_FormatDebug_DISP(unsigned char *uszSendBuf, int inSendLen)
{
	char			szBuf[512 + 1], szBitMap[_NCCC_ESC_BIT_MAP_SIZE_ + 1];
	int			i, inCnt = 0, inField, inLen, inFieldLen;
	int			inPrintLineCnt = 0;
	int			inOneLineLen = 34;
	int			inBitMapCnt1, inBitMapCnt2;
	char			szPrtBuf[50 + 1];
	char			szPrintLineData[36 + 1];
	unsigned char		uszBitMap[9], uszBitMapDisp[66];
	ISO_TYPE_NCCC_ESC_TABLE srISOTypeTablePrt;

	memset((char *)&srISOTypeTablePrt, 0x00, sizeof(srISOTypeTablePrt));
	memcpy((char *)&srISOTypeTablePrt, (char *)srNCCC_ESC_ISOFunc, sizeof(srISOTypeTablePrt));

	if (uszSendBuf[6] == 0x00 || uszSendBuf[6] == 0x20 || uszSendBuf[6] == 0x40)
	{
		inLogPrintf(AT, " ");
		inLogPrintf(AT, "==========================================");
		inLogPrintf(AT, "ISO8583 Format  <<ESC Send>>");
	}
	else
		inLogPrintf(AT, "ISO8583 Format  <<ESC Recvive>>");

	memset(szBuf, 0x00, sizeof(szBuf));
	sprintf(szBuf,"TPDU = [");
	inFunc_BCD_to_ASCII(&szBuf[8], (unsigned char*)&uszSendBuf[inCnt], _NCCC_ESC_TPDU_SIZE_);
	strcat(szBuf,"]");
	inLogPrintf(AT, szBuf);
	inCnt += _NCCC_ESC_TPDU_SIZE_;

	memset(szBuf, 0x00, sizeof(szBuf));
	sprintf(szBuf,"MTI  = [");
	inFunc_BCD_to_ASCII(&szBuf[8], (unsigned char*)&uszSendBuf[inCnt], _NCCC_ESC_MTI_SIZE_);
	strcat(szBuf,"]");
	inLogPrintf(AT, szBuf);
	inCnt += _NCCC_ESC_MTI_SIZE_;

	memset(szBuf, 0x00, sizeof(szBuf));
	memset(szBitMap, 0x00, sizeof(szBitMap));
	memcpy(szBitMap, (char *)&uszSendBuf[inCnt], _NCCC_ESC_BIT_MAP_SIZE_);
	sprintf(szBuf, "BMap = [%02X %02X %02X %02X %02X %02X %02X %02X]",
						uszSendBuf[inCnt + 0],
						uszSendBuf[inCnt + 1],
						uszSendBuf[inCnt + 2],
						uszSendBuf[inCnt + 3],
						uszSendBuf[inCnt + 4],
						uszSendBuf[inCnt + 5],
						uszSendBuf[inCnt + 6],
						uszSendBuf[inCnt + 7]);

	inLogPrintf(AT, szBuf);
	memset(uszBitMapDisp, 0x00, sizeof(uszBitMapDisp));
	memset(uszBitMap, 0x00, sizeof(uszBitMap));
	memcpy(uszBitMap, (char *)&uszSendBuf[inCnt], _NCCC_ESC_BIT_MAP_SIZE_);

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

	inLogPrintf(AT, "   1234567890    1234567890    1234567890");
	memset(szBuf, 0x00, sizeof(szBuf));
	sprintf(szBuf, " 0>%10.10s  1>%10.10s  2>%10.10s", &uszBitMapDisp[1], &uszBitMapDisp[11], &uszBitMapDisp[21]);
        inLogPrintf(AT, szBuf);
        sprintf(szBuf, " 3>%10.10s  4>%10.10s  5>%10.10s", &uszBitMapDisp[31], &uszBitMapDisp[41], &uszBitMapDisp[51]);
      	inLogPrintf(AT, szBuf);
        sprintf(szBuf, " 6>%4.4s", &uszBitMapDisp[61]);
        inLogPrintf(AT, szBuf);

	inCnt += _NCCC_ESC_BIT_MAP_SIZE_;

	for (i = 1; i <= 64; i ++)
	{
		if (!inNCCC_ESC_BitMapCheck((unsigned char *)szBitMap, i))
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

		inField = inNCCC_ESC_GetFieldIndex(i, srISOTypeTablePrt.srISOFieldType);
		inFieldLen = inNCCC_ESC_GetFieldLen(i, &uszSendBuf[inCnt], srISOTypeTablePrt.srISOFieldType);
		switch (srISOTypeTablePrt.srISOFieldType[inField].inFieldType)
		{
			case _NCCC_ESC_ISO_ASC_ :
				memcpy(&szBuf[inLen], &uszSendBuf[inCnt], inFieldLen);
				break;
			case _NCCC_ESC_ISO_BYTE_3_ :
				inFunc_BCD_to_ASCII(&szBuf[inLen], (unsigned char*)&uszSendBuf[inCnt], 2);
				inCnt += 2;
				inLen += 4;
				strcat(szBuf, " ");
				inLen ++;
				inFieldLen --;
				inFieldLen --;

				if (srISOTypeTablePrt.srISOFieldType[inField].uszDispAscii == VS_TRUE)
				{
					if (ginDebug == VS_TRUE && i != 59)
						memcpy(&szBuf[inLen], (char *)&uszSendBuf[inCnt], inFieldLen);
				}

				break;
			case _NCCC_ESC_ISO_NIBBLE_2_ :
			case _NCCC_ESC_ISO_BYTE_2_ :
			case _NCCC_ESC_ISO_BYTE_2_H_ :
			case _NCCC_ESC_ISO_BYTE_3_H_ :
//				strcat(&szBuf[inLen ++], "[");
				inFunc_BCD_to_ASCII(&szBuf[inLen], (unsigned char*)&uszSendBuf[inCnt ++], 1);
				inLen += 2;
				strcat(szBuf, " ");
				inLen ++;

				inFieldLen --;
				if (srISOTypeTablePrt.srISOFieldType[inField].inFieldType == _NCCC_ESC_ISO_BYTE_3_ ||
				    srISOTypeTablePrt.srISOFieldType[inField].inFieldType == _NCCC_ESC_ISO_BYTE_3_H_)
				{
					inFunc_BCD_to_ASCII(&szBuf[inLen], (unsigned char*)&uszSendBuf[inCnt ++], 1);
					inLen += 2;
					inFieldLen --;
				}

			case _NCCC_ESC_ISO_BCD_ :
				if (srISOTypeTablePrt.srISOFieldType[inField].uszDispAscii == VS_TRUE)
					memcpy(&szBuf[inLen], (char *)&uszSendBuf[inCnt], inFieldLen);
				else
					inFunc_BCD_to_ASCII(&szBuf[inLen], (unsigned char*)&uszSendBuf[inCnt], inFieldLen);

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

		if (ginDebug == VS_TRUE && i == 59)
			vdNCCC_ESC_ISO_FormatDebug_DISP_59(&uszSendBuf[inCnt], inFieldLen);

		inCnt += inFieldLen;
	}

	inLogPrintf(AT, "==========================================");
	inLogPrintf(AT, " ");
}

/*
Function        :vdNCCC_ESC_ISO_FormatDebug_DISP_59
Date&Time       :2016/12/1 上午 9:42
Describe        :
*/
void vdNCCC_ESC_ISO_FormatDebug_DISP_59(unsigned char *uszSendBuf, int inFieldLen)
{
	int	inCnt = 0, inTableLen;
	int	inPrintLineCnt = 0, inOneLineLen = 34;
	char	szTagData[38 + 1] = {0};
	long	lnTableLen = 0;
	char	szPrintBuf[1024 + 1] = {0}, szTemplate[42 + 1];
	char 	szTempTable[1 + 1];

	while (inFieldLen > inCnt)
	{
		switch (uszSendBuf[inCnt])
		{
			case 'C' :
				/* Table ID */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				szTemplate[0] = uszSendBuf[inCnt];
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "  Table ID = %x / %s", uszSendBuf[inCnt], szTemplate);
                                inLogPrintf(AT, szPrintBuf);
				inCnt ++;

				/* Table Length */
				inTableLen = uszSendBuf[inCnt];
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "  Table Len = %x / %d", uszSendBuf[inCnt], inTableLen);
                                inLogPrintf(AT, szPrintBuf);
				inCnt ++;

				/* Function Number / The Terminal Support CUP Function (1 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				szTemplate[0] = uszSendBuf[inCnt];
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "    Function Number = %s", szTemplate);
                                inLogPrintf(AT, szPrintBuf);
				inCnt ++;

				/* Acquire Indicator / Terminal Hot Key Control (1 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				szTemplate[0] = uszSendBuf[inCnt];
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "    Acquire Indicator = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt ++;

				if (szTemplate[0] == '1')
				{
					/* CUP Trace Number (6 Byte) */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memcpy(&szTemplate[0], &uszSendBuf[inCnt], 6);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "    CUP Trace Number = %s", szTemplate);
					inLogPrintf(AT, szPrintBuf);
				        inCnt += 6;

					/* CUP Transaction Date(MMDD) (4 Byte) */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memcpy(&szTemplate[0], &uszSendBuf[inCnt], 4);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "    CUP Transaction Date(MMDD) = %s", szTemplate);
					inLogPrintf(AT, szPrintBuf);
					inCnt += 4;

					/* CUP Transaction Time(hhmmss) (6 Byte) */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memcpy(&szTemplate[0], &uszSendBuf[inCnt], 6);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "    CUP Transaction Time(hhmmss) = %s", szTemplate);
					inLogPrintf(AT, szPrintBuf);
					inCnt += 6;

					/* CUP Retrieve Reference Number(CRRN) (12 Byte) */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memcpy(&szTemplate[0], &uszSendBuf[inCnt], 12);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "    CUP Retrieve Reference Number(CRRN) = %s", szTemplate);
					inLogPrintf(AT, szPrintBuf);
					inCnt += 12;

					/* Settlement Date(MMDD) (4 Byte) */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memcpy(&szTemplate[0], &uszSendBuf[inCnt], 4);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "    Settlement Date(MMDD) = %s", szTemplate);
					inLogPrintf(AT, szPrintBuf);
					inCnt += 4;
				}

				break;
			case 'D' :
				/* Table ID */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				szTemplate[0] = uszSendBuf[inCnt];
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "  Table ID = %x / %s", uszSendBuf[inCnt], szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt ++;

				/* Table Length */
				inTableLen = uszSendBuf[inCnt];
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "  Table Len = %x / %d", uszSendBuf[inCnt], inTableLen);
				inLogPrintf(AT, szPrintBuf);
				inCnt ++;

				/* Online Rate DCC (1 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				szTemplate[0] = uszSendBuf[inCnt];
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Online Rate DCC = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt ++;

				break;
			case 'O' :
				/* Table ID */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				szTemplate[0] = uszSendBuf[inCnt];
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "  Table ID = %x / %s", uszSendBuf[inCnt], szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt ++;

				/* Table Length */
				inTableLen = uszSendBuf[inCnt];
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "  Table Len = %x / %d", uszSendBuf[inCnt], inTableLen);
				inLogPrintf(AT, szPrintBuf);
				inCnt ++;

				/* Original Transaction Date(MMDD) (4 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszSendBuf[inCnt], 4);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "    Original Transaction Date(MMDD) = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 4;

				/* Original Amount (12 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszSendBuf[inCnt], 12);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "    Original Amount = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 12;

				break;
			case 'Y' :
				/* Table ID */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				szTemplate[0] = uszSendBuf[inCnt];
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "  Table ID = %x / %s", uszSendBuf[inCnt], szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt ++;

				/* Table Length */
				inTableLen = uszSendBuf[inCnt];
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "  Table Len = %x / %d", uszSendBuf[inCnt], inTableLen);
				inLogPrintf(AT, szPrintBuf);
				inCnt ++;

				/* 授權主機的西元年 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszSendBuf[inCnt], 4);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "    Host YYYY = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 4;

				break;
                        /* For ESC電子簽單，ISODebug */
			case 'N' :
				/* Table ID */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				szTemplate[0] = uszSendBuf[inCnt];
				inCnt ++;

				memset(szTempTable, 0x00, sizeof(szTempTable));
				szTempTable[0] = uszSendBuf[inCnt];

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "  Table ID = %x%x / %s%s", uszSendBuf[inCnt - 1], uszSendBuf[inCnt], szTemplate, szTempTable);
				inLogPrintf(AT, szPrintBuf);
				inCnt ++;

				/* Table Length */
				inTableLen = 0;
				inTableLen += ((uszSendBuf[inCnt] / 16 * 10 + uszSendBuf[inCnt] % 16) * 100);
				inTableLen += (uszSendBuf[inCnt + 1] / 16 * 10) + uszSendBuf[inCnt + 1] % 16;
				
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "  Table Len = %x%x / %d", uszSendBuf[inCnt], uszSendBuf[inCnt + 1], inTableLen);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 2;

				if (szTempTable[0] == 0x31)		/* 1 */
				{
					/* Function Number / The Terminal Support CUP Function (1 Byte) */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					szTemplate[0] = uszSendBuf[inCnt];
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "    Function Number = %s", szTemplate);
					inLogPrintf(AT, szPrintBuf);
					inCnt ++;

					/* Acquire Indicator / Terminal Hot Key Control (1 Byte) */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					szTemplate[0] = uszSendBuf[inCnt];
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "    Acquire Indicator = %s", szTemplate);
					inLogPrintf(AT, szPrintBuf);
					inCnt ++;
				}
				else if (szTempTable[0] == 0x34)	/* 4 */
				{
					/* Installment Indicator (1 Byte) */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					szTemplate[0] = uszSendBuf[inCnt];
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "    Inst Indicator = %s", szTemplate);
					inLogPrintf(AT, szPrintBuf);
					inCnt ++;

					/* Installment Period (2 Byte) */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					szTemplate[0] = uszSendBuf[inCnt];
					inCnt ++;

					memset(szTempTable, 0x00, sizeof(szTempTable));
					szTempTable[0] = uszSendBuf[inCnt];

					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "    Inst Period = %s%s", szTemplate, szTempTable);
					inLogPrintf(AT, szPrintBuf);
					inCnt ++;
				}
				else if (szTempTable[0] == 0x35)	/* 5 */
				{
					/* Installment Indicator (1 Byte) */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					szTemplate[0] = uszSendBuf[inCnt];
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "    Redeem Indicator = %s", szTemplate);
					inLogPrintf(AT, szPrintBuf);
					inCnt ++;
				}
				else if (szTempTable[0] == 0x45)	/* E */
				{
					/* ESC Indicator (1 Byte) */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					szTemplate[0] = uszSendBuf[inCnt];
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "    ESC = %s", szTemplate);
					inLogPrintf(AT, szPrintBuf);
					inCnt ++;
				}
				else if (szTempTable[0] == 0x49)	/* I */
				{
					/* VEPS Indicator (1 Byte) */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					szTemplate[0] = uszSendBuf[inCnt];
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "    Stand In = %s", szTemplate);
					inLogPrintf(AT, szPrintBuf);
					inCnt ++;
				}
                                else if (szTempTable[0] == 0x51)	/* Q */
                                {
                                        /*【需求單 - 104058】免簽名需求《 依照MFES授權主機回覆交易電文的Indicator判斷帳單是否列印免簽名》by Tusin - 2015/12/2 上午 11:22:04 */
                                        /* VEPS Indicator (1 Byte) */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					szTemplate[0] = uszSendBuf[inCnt];
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "    Quick Pay = %s", szTemplate);
					inLogPrintf(AT, szPrintBuf);
					inCnt ++;
                                }

				break;
			case 'E' :
				/* Table ID */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				szTemplate[0] = uszSendBuf[inCnt];
				inCnt ++;

				memset(szTempTable, 0x00, sizeof(szTempTable));
				szTempTable[0] = uszSendBuf[inCnt];

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "  Table ID = %x%x / %s%s", uszSendBuf[inCnt - 1], uszSendBuf[inCnt], szTemplate, szTempTable);
                                inLogPrintf(AT, szPrintBuf);
				inCnt ++;

				/* Table Length */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszSendBuf[inCnt], 2);

				memset(szTempTable, 0x00, sizeof(szTempTable));
				inFunc_BCD_to_ASCII(szTempTable, (unsigned char*)szTemplate, 2);

				inTableLen = atoi(szTempTable);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "  Table Len = %02x%02x / %04d", szTemplate[0], szTemplate[1], inTableLen);
                                inLogPrintf(AT, szPrintBuf);
				inCnt += 2;

				/* 唯一序號 (28 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszSendBuf[inCnt], 14);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "    ESC S/N = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 14;

				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszSendBuf[inCnt], 14);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "              %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 14;

				/* Card Information (10 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszSendBuf[inCnt], 10);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "    Card Type = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 10;

				/* 封包總個數 (2 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszSendBuf[inCnt], 2);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "    Total Cnt = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 2;

				/* 目前封包編號 (2 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszSendBuf[inCnt], 2);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "    Packet Cnt = %s", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 2;

				/* 封包總大小 (5 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszSendBuf[inCnt], 5);

				lnTableLen = atol(szTemplate);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "    Total Size = %ld", lnTableLen);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 5;

				/* 目前封包大小 (3 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszSendBuf[inCnt], 3);

				inTableLen = atoi(szTemplate);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "    Packet Size = %d", inTableLen);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 3;

				/* 封包內容 */
				inCnt += inTableLen;

				/* LRC (1 Byte) */
//				memset(szTemplate, 0x00, sizeof(szTemplate));
//				memcpy(szTemplate, (char*)&uszSendBuf[inCnt], 1);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "    LRC = %02x", uszSendBuf[inCnt]);
				inLogPrintf(AT, szPrintBuf);
				inCnt ++;

				break;
			case 'U' :
				/* Table ID */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				szTemplate[0] = uszSendBuf[inCnt];
				inCnt ++;

				memset(szTempTable, 0x00, sizeof(szTempTable));
				szTempTable[0] = uszSendBuf[inCnt];

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "  Table ID = %x%x / %s%s", uszSendBuf[inCnt - 1], uszSendBuf[inCnt], szTemplate, szTempTable);
				inLogPrintf(AT, szPrintBuf);
				inCnt ++;

				/* Table Length */
				inTableLen = 0;
				inTableLen += ((uszSendBuf[inCnt] / 16 * 10 + uszSendBuf[inCnt] % 16) * 100);
				inTableLen += (uszSendBuf[inCnt + 1] / 16 * 10) + uszSendBuf[inCnt + 1] % 16;
				
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "  Table Len = %x%x / %d", uszSendBuf[inCnt], uszSendBuf[inCnt + 1], inTableLen);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 2;
				
				if (inTableLen <= 8)
				{
					/* Table Data */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memcpy(&szTemplate[0], &uszSendBuf[inCnt], inTableLen);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, " Table Data [%s]", szTemplate);
					inLogPrintf(AT, szPrintBuf);
				}
				else
				{
					/* Table Data */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memcpy(&szTemplate[0], &uszSendBuf[inCnt], inTableLen);
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
				break;
			default :
				memset(szTemplate, 0x00, sizeof(szTemplate));
				szTemplate[0] = uszSendBuf[inCnt];
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "      OTHER = (%s)", szTemplate);
				inLogPrintf(AT, szPrintBuf);
				inCnt ++;

				break;
		}
	}
}

/*
Function        :vdNCCC_ESC_ISO_FormatDebug_PRINT
Date&Time       :2016/12/1 上午 9:41
Describe        :
*/
void vdNCCC_ESC_ISO_FormatDebug_PRINT(unsigned char *uszSendBuf, int inSendLen)
{
	char			szBuf[512 + 1], szBitMap[_NCCC_ESC_BIT_MAP_SIZE_ + 1];
	int			i, inCnt = 0, inField, inLen, inFieldLen;
	int			inPrintLineCnt = 0;
	int			inOneLineLen = 34;
	int			inBitMapCnt1, inBitMapCnt2;
	char			szPrtBuf[50 + 1];
	char			szPrintLineData[36 + 1];
	unsigned char		uszBitMap[9], uszBitMapDisp[66];
	unsigned char		uszBuffer[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	BufferHandle		srBhandle;
	FONT_ATTRIB		srFont_Attrib;
	ISO_TYPE_NCCC_ESC_TABLE srISOTypeTablePrt;
	
	/* 是否有列印功能 */
	if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
	{
		
	}
	else
	{
		/* 初始化 */
		inPRINT_Buffer_Initial(uszBuffer, _BUFFER_MAX_LINE_, &srFont_Attrib, &srBhandle);

		memset((char *)&srISOTypeTablePrt, 0x00, sizeof(srISOTypeTablePrt));
		memcpy((char *)&srISOTypeTablePrt, (char *)srNCCC_ESC_ISOFunc, sizeof(srISOTypeTablePrt));

		if (uszSendBuf[6] == 0x00 || uszSendBuf[6] == 0x20 || uszSendBuf[6] == 0x40)
		{
			inPRINT_Buffer_PutIn(" ", _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("==========================================", _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("ISO8583 Format  <<ESC Send>>", _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		else
			inPRINT_Buffer_PutIn("ISO8583 Format  <<ESC Recvive>>", _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szBuf, 0x00, sizeof(szBuf));
		sprintf(szBuf,"TPDU = [");
		inFunc_BCD_to_ASCII(&szBuf[8], (unsigned char*)&uszSendBuf[inCnt], _NCCC_ESC_TPDU_SIZE_);
		strcat(szBuf,"]");
		inPRINT_Buffer_PutIn(szBuf, _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inCnt += _NCCC_ESC_TPDU_SIZE_;

		memset(szBuf, 0x00, sizeof(szBuf));
		sprintf(szBuf,"MTI  = [");
		inFunc_BCD_to_ASCII(&szBuf[8], (unsigned char*)&uszSendBuf[inCnt], _NCCC_ESC_MTI_SIZE_);
		strcat(szBuf,"]");
		inPRINT_Buffer_PutIn(szBuf, _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inCnt += _NCCC_ESC_MTI_SIZE_;

		memset(szBuf, 0x00, sizeof(szBuf));
		memset(szBitMap, 0x00, sizeof(szBitMap));
		memcpy(szBitMap, (char *)&uszSendBuf[inCnt], _NCCC_ESC_BIT_MAP_SIZE_);
		sprintf(szBuf, "BMap = [%02X %02X %02X %02X %02X %02X %02X %02X]",
							uszSendBuf[inCnt + 0],
							uszSendBuf[inCnt + 1],
							uszSendBuf[inCnt + 2],
							uszSendBuf[inCnt + 3],
							uszSendBuf[inCnt + 4],
							uszSendBuf[inCnt + 5],
							uszSendBuf[inCnt + 6],
							uszSendBuf[inCnt + 7]);

		inPRINT_Buffer_PutIn(szBuf, _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		memset(uszBitMapDisp, 0x00, sizeof(uszBitMapDisp));
		memset(uszBitMap, 0x00, sizeof(uszBitMap));
		memcpy(uszBitMap, (char *)&uszSendBuf[inCnt], _NCCC_ESC_BIT_MAP_SIZE_);

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

		inPRINT_Buffer_PutIn("   1234567890    1234567890    1234567890", _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		memset(szBuf, 0x00, sizeof(szBuf));
		sprintf(szBuf, " 0>%10.10s  1>%10.10s  2>%10.10s", &uszBitMapDisp[1], &uszBitMapDisp[11], &uszBitMapDisp[21]);
		inPRINT_Buffer_PutIn(szBuf, _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		sprintf(szBuf, " 3>%10.10s  4>%10.10s  5>%10.10s", &uszBitMapDisp[31], &uszBitMapDisp[41], &uszBitMapDisp[51]);
		inPRINT_Buffer_PutIn(szBuf, _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		sprintf(szBuf, " 6>%4.4s", &uszBitMapDisp[61]);
		inPRINT_Buffer_PutIn(szBuf, _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		inCnt += _NCCC_ESC_BIT_MAP_SIZE_;

		for (i = 1; i <= 64; i ++)
		{
			if (!inNCCC_ESC_BitMapCheck((unsigned char *)szBitMap, i))
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

			inField = inNCCC_ESC_GetFieldIndex(i, srISOTypeTablePrt.srISOFieldType);
			inFieldLen = inNCCC_ESC_GetFieldLen(i, &uszSendBuf[inCnt], srISOTypeTablePrt.srISOFieldType);
			switch (srISOTypeTablePrt.srISOFieldType[inField].inFieldType)
			{
				case _NCCC_ESC_ISO_ASC_ :
					memcpy(&szBuf[inLen], &uszSendBuf[inCnt], inFieldLen);
					break;
				case _NCCC_ESC_ISO_BYTE_3_ :
					inFunc_BCD_to_ASCII(&szBuf[inLen], (unsigned char*)&uszSendBuf[inCnt], 2);
					inCnt += 2;
					inLen += 4;
					strcat(szBuf, " ");
					inLen ++;
					inFieldLen --;
					inFieldLen --;

					if (srISOTypeTablePrt.srISOFieldType[inField].uszDispAscii == VS_TRUE)
					{
						if (ginISODebug == VS_TRUE && i != 59)
							memcpy(&szBuf[inLen], (char *)&uszSendBuf[inCnt], inFieldLen);
					}

					break;
				case _NCCC_ESC_ISO_NIBBLE_2_ :
				case _NCCC_ESC_ISO_BYTE_2_ :
				case _NCCC_ESC_ISO_BYTE_2_H_ :
				case _NCCC_ESC_ISO_BYTE_3_H_ :
	//				strcat(&szBuf[inLen ++], "[");
					inFunc_BCD_to_ASCII(&szBuf[inLen], (unsigned char*)&uszSendBuf[inCnt ++], 1);
					inLen += 2;
					strcat(szBuf, " ");
					inLen ++;

					inFieldLen --;
					if (srISOTypeTablePrt.srISOFieldType[inField].inFieldType == _NCCC_ESC_ISO_BYTE_3_ ||
					    srISOTypeTablePrt.srISOFieldType[inField].inFieldType == _NCCC_ESC_ISO_BYTE_3_H_)
					{
						inFunc_BCD_to_ASCII(&szBuf[inLen], (unsigned char*)&uszSendBuf[inCnt ++], 1);
						inLen += 2;
						inFieldLen --;
					}

				case _NCCC_ESC_ISO_BCD_ :
					if (srISOTypeTablePrt.srISOFieldType[inField].uszDispAscii == VS_TRUE)
						memcpy(&szBuf[inLen], (char *)&uszSendBuf[inCnt], inFieldLen);
					else
						inFunc_BCD_to_ASCII(&szBuf[inLen], (unsigned char*)&uszSendBuf[inCnt], inFieldLen);

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

			if (ginISODebug == VS_TRUE && i == 59)
				vdNCCC_ESC_ISO_FormatDebug_PRINT_59(&uszSendBuf[inCnt], inFieldLen, uszBuffer, &srFont_Attrib, &srBhandle);

			inCnt += inFieldLen;
		}

		inPRINT_Buffer_PutIn("==========================================", _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		inPRINT_Buffer_OutPut(uszBuffer, &srBhandle);
	}
}

/*
Function        :vdNCCC_ESC_ISO_FormatDebug_PRINT_59
Date&Time       :2016/12/1 上午 9:42
Describe        :
*/
void vdNCCC_ESC_ISO_FormatDebug_PRINT_59(unsigned char *uszSendBuf, int inFieldLen, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inCnt = 0, inTableLen;
	int	inPrintLineCnt = 0, inOneLineLen = 34;
	char	szTagData[38 + 1] = {0};
	long	lnTableLen;
	char	szPrintBuf[128], szTemplate[42 + 1];
	char 	szTempTable[1 + 1];

	while (inFieldLen > inCnt)
	{
		switch (uszSendBuf[inCnt])
		{
			case 'C' :
				/* Table ID */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				szTemplate[0] = uszSendBuf[inCnt];
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "  Table ID = %x / %s", uszSendBuf[inCnt], szTemplate);
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt ++;

				/* Table Length */
				inTableLen = uszSendBuf[inCnt];
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "  Table Len = %x / %d", uszSendBuf[inCnt], inTableLen);
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt ++;

				/* Function Number / The Terminal Support CUP Function (1 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				szTemplate[0] = uszSendBuf[inCnt];
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "    Function Number = %s", szTemplate);
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt ++;

				/* Acquire Indicator / Terminal Hot Key Control (1 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				szTemplate[0] = uszSendBuf[inCnt];
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "    Acquire Indicator = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt ++;

				if (szTemplate[0] == '1')
				{
					/* CUP Trace Number (6 Byte) */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memcpy(&szTemplate[0], &uszSendBuf[inCnt], 6);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "    CUP Trace Number = %s", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				        inCnt += 6;

					/* CUP Transaction Date(MMDD) (4 Byte) */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memcpy(&szTemplate[0], &uszSendBuf[inCnt], 4);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "    CUP Transaction Date(MMDD) = %s", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					inCnt += 4;

					/* CUP Transaction Time(hhmmss) (6 Byte) */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memcpy(&szTemplate[0], &uszSendBuf[inCnt], 6);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "    CUP Transaction Time(hhmmss) = %s", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					inCnt += 6;

					/* CUP Retrieve Reference Number(CRRN) (12 Byte) */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memcpy(&szTemplate[0], &uszSendBuf[inCnt], 12);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "    CUP Retrieve Reference Number(CRRN) = %s", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					inCnt += 12;

					/* Settlement Date(MMDD) (4 Byte) */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memcpy(&szTemplate[0], &uszSendBuf[inCnt], 4);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "    Settlement Date(MMDD) = %s", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					inCnt += 4;
				}

				break;
			case 'D' :
				/* Table ID */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				szTemplate[0] = uszSendBuf[inCnt];
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "  Table ID = %x / %s", uszSendBuf[inCnt], szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt ++;

				/* Table Length */
				inTableLen = uszSendBuf[inCnt];
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "  Table Len = %x / %d", uszSendBuf[inCnt], inTableLen);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt ++;

				/* Online Rate DCC (1 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				szTemplate[0] = uszSendBuf[inCnt];
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "   Online Rate DCC = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt ++;

				break;
			case 'O' :
				/* Table ID */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				szTemplate[0] = uszSendBuf[inCnt];
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "  Table ID = %x / %s", uszSendBuf[inCnt], szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt ++;

				/* Table Length */
				inTableLen = uszSendBuf[inCnt];
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "  Table Len = %x / %d", uszSendBuf[inCnt], inTableLen);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt ++;

				/* Original Transaction Date(MMDD) (4 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszSendBuf[inCnt], 4);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "    Original Transaction Date(MMDD) = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 4;

				/* Original Amount (12 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszSendBuf[inCnt], 12);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "    Original Amount = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 12;

				break;
			case 'Y' :
				/* Table ID */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				szTemplate[0] = uszSendBuf[inCnt];
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "  Table ID = %x / %s", uszSendBuf[inCnt], szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt ++;

				/* Table Length */
				inTableLen = uszSendBuf[inCnt];
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "  Table Len = %x / %d", uszSendBuf[inCnt], inTableLen);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt ++;

				/* 授權主機的西元年 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszSendBuf[inCnt], 4);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "    Host YYYY = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 4;

				break;
                        /* For ESC電子簽單，ISODebug */
			case 'N' :
				/* Table ID */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				szTemplate[0] = uszSendBuf[inCnt];
				inCnt ++;

				memset(szTempTable, 0x00, sizeof(szTempTable));
				szTempTable[0] = uszSendBuf[inCnt];

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "  Table ID = %x%x / %s%s", uszSendBuf[inCnt - 1], uszSendBuf[inCnt], szTemplate, szTempTable);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt ++;

				/* Table Length */
				inTableLen = 0;
				inTableLen += ((uszSendBuf[inCnt] / 16 * 10 + uszSendBuf[inCnt] % 16) * 100);
				inTableLen += (uszSendBuf[inCnt + 1] / 16 * 10) + uszSendBuf[inCnt + 1] % 16;
				
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "  Table Len = %x%x / %d", uszSendBuf[inCnt], uszSendBuf[inCnt + 1], inTableLen);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 2;

				if (szTempTable[0] == 0x31)		/* 1 */
				{
					/* Function Number / The Terminal Support CUP Function (1 Byte) */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					szTemplate[0] = uszSendBuf[inCnt];
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "    Function Number = %s", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					inCnt ++;

					/* Acquire Indicator / Terminal Hot Key Control (1 Byte) */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					szTemplate[0] = uszSendBuf[inCnt];
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "    Acquire Indicator = %s", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					inCnt ++;
				}
				else if (szTempTable[0] == 0x34)	/* 4 */
				{
					/* Installment Indicator (1 Byte) */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					szTemplate[0] = uszSendBuf[inCnt];
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "    Inst Indicator = %s", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					inCnt ++;

					/* Installment Period (2 Byte) */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					szTemplate[0] = uszSendBuf[inCnt];
					inCnt ++;

					memset(szTempTable, 0x00, sizeof(szTempTable));
					szTempTable[0] = uszSendBuf[inCnt];

					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "    Inst Period = %s%s", szTemplate, szTempTable);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					inCnt ++;
				}
				else if (szTempTable[0] == 0x35)	/* 5 */
				{
					/* Installment Indicator (1 Byte) */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					szTemplate[0] = uszSendBuf[inCnt];
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "    Redeem Indicator = %s", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					inCnt ++;
				}
				else if (szTempTable[0] == 0x45)	/* E */
				{
					/* ESC Indicator (1 Byte) */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					szTemplate[0] = uszSendBuf[inCnt];
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "    ESC = %s", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					inCnt ++;
				}
				else if (szTempTable[0] == 0x49)	/* I */
				{
					/* VEPS Indicator (1 Byte) */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					szTemplate[0] = uszSendBuf[inCnt];
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "    Stand In = %s", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					inCnt ++;
				}
                                else if (szTempTable[0] == 0x51)	/* Q */
                                {
                                        /*【需求單 - 104058】免簽名需求《 依照MFES授權主機回覆交易電文的Indicator判斷帳單是否列印免簽名》by Tusin - 2015/12/2 上午 11:22:04 */
                                        /* VEPS Indicator (1 Byte) */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					szTemplate[0] = uszSendBuf[inCnt];
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "    Quick Pay = %s", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					inCnt ++;
                                }

				break;
			case 'E' :
				/* Table ID */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				szTemplate[0] = uszSendBuf[inCnt];
				inCnt ++;

				memset(szTempTable, 0x00, sizeof(szTempTable));
				szTempTable[0] = uszSendBuf[inCnt];

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "  Table ID = %x%x / %s%s", uszSendBuf[inCnt - 1], uszSendBuf[inCnt], szTemplate, szTempTable);
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt ++;

				/* Table Length */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszSendBuf[inCnt], 2);

				memset(szTempTable, 0x00, sizeof(szTempTable));
				inFunc_BCD_to_ASCII(szTempTable, (unsigned char*)szTemplate, 2);

				inTableLen = atoi(szTempTable);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "  Table Len = %02x%02x / %04d", szTemplate[0], szTemplate[1], inTableLen);
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 2;

				/* 唯一序號 (28 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszSendBuf[inCnt], 14);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "    ESC S/N = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 14;

				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszSendBuf[inCnt], 14);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "              %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 14;

				/* Card Information (10 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszSendBuf[inCnt], 10);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "    Card Type = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 10;

				/* 封包總個數 (2 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszSendBuf[inCnt], 2);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "    Total Cnt = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 2;

				/* 目前封包編號 (2 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszSendBuf[inCnt], 2);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "    Packet Cnt = %s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 2;

				/* 封包總大小 (5 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszSendBuf[inCnt], 5);

				lnTableLen = atol(szTemplate);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "    Total Size = %ld", lnTableLen);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 5;

				/* 目前封包大小 (3 Byte) */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], &uszSendBuf[inCnt], 3);

				inTableLen = atoi(szTemplate);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "    Packet Size = %d", inTableLen);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt += 3;

				/* 封包內容 */
				inCnt += inTableLen;

				/* LRC (1 Byte) */
//				memset(szTemplate, 0x00, sizeof(szTemplate));
//				memcpy(szTemplate, (char*)&uszSendBuf[inCnt], 1);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "    LRC = %02x", uszSendBuf[inCnt]);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt ++;

				break;
			case 'U' :
				/* Table ID */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				szTemplate[0] = uszSendBuf[inCnt];
				inCnt ++;

				memset(szTempTable, 0x00, sizeof(szTempTable));
				szTempTable[0] = uszSendBuf[inCnt];

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "  Table ID = %x%x / %s%s", uszSendBuf[inCnt - 1], uszSendBuf[inCnt], szTemplate, szTempTable);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt ++;

				/* Table Length */
				inTableLen = 0;
				inTableLen += ((uszSendBuf[inCnt] / 16 * 10 + uszSendBuf[inCnt] % 16) * 100);
				inTableLen += (uszSendBuf[inCnt + 1] / 16 * 10) + uszSendBuf[inCnt + 1] % 16;
				
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "  Table Len = %x%x / %d", uszSendBuf[inCnt], uszSendBuf[inCnt + 1], inTableLen);
				inLogPrintf(AT, szPrintBuf);
				inCnt += 2;
				
				if (inTableLen <= 8)
				{
					/* Table Data */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memcpy(&szTemplate[0], &uszSendBuf[inCnt], inTableLen);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, " Table Data [%s]", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				}
				else
				{
					/* Table Data */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memcpy(&szTemplate[0], &uszSendBuf[inCnt], inTableLen);
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
				break;
			default :
				memset(szTemplate, 0x00, sizeof(szTemplate));
				szTemplate[0] = uszSendBuf[inCnt];
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "      OTHER = (%s)", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_ISO_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inCnt ++;

				break;
		}
	}
}

/*
Function        :inNCCC_ESC_Data_Compress_Encryption_Test
Date&Time       :2016/4/20 下午 4:40
Describe        :先壓縮再加密 
 *		將ESC_E1.txt		轉成ESC_E1.txt.gz並加密
 *		將簽名圖檔多複製一份	轉成ESC_E2.bmp.gz並加密
*/
int inNCCC_ESC_Data_Compress_Encryption_Test()
{
//	char		szFileName[16 + 1];
//	char		szFileNameGzBmp[16 + 1];
	char		szData[32 + 1];
	char		szTemplate[16 + 1];
	char		szEncryData[16 + 1];
	unsigned long	ulHandle = 0;
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_Data_Compress_Encryption_Test() START!");
	
	/* 壓縮加密檔案(GZIP) */
	
		/* 簽單 */
		if (inFunc_Data_GZip("", "BMP_1_A.TXT", _FS_DATA_PATH_) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		/* 圖檔 */
		if (inFunc_Data_GZip("", "BMP_1_B.TXT", _FS_DATA_PATH_) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		
		inFile_Move_File("BMP_1_A.TXT.gz", _AP_ROOT_PATH_, "", _SD_PATH_);
		inFile_Move_File("BMP_1_B.BMP.gz", _AP_ROOT_PATH_, "", _SD_PATH_);
	/* 加密並置換資料內容(軟加密) */
	
		char			szKeyFull[24 + 1] = {"956630010216111595663001"};	/* 放已組完全部長度的Key */
		CTOS_KMS2KEYWRITE_PARA	srKeyWritePara;
		
		memset(&srKeyWritePara, 0x00, sizeof(srKeyWritePara));
		/* 將所需參數放入結構中 */
		srKeyWritePara.Version = 0x01;						
		srKeyWritePara.Info.KeySet = _TWK_KEYSET_NCCC_;
		srKeyWritePara.Info.KeyIndex = _TWK_KEYINDEX_NCCC_ESC_;
		srKeyWritePara.Info.KeyType = KMS2_KEYTYPE_3DES;
		srKeyWritePara.Info.KeyVersion = 0x01;					
		srKeyWritePara.Info.KeyAttribute = KMS2_KEYATTRIBUTE_ENCRYPT;		
		srKeyWritePara.Protection.Mode = KMS2_KEYPROTECTIONMODE_PLAINTEXT;	
		srKeyWritePara.Value.pKeyData = (unsigned char*)szKeyFull;
		srKeyWritePara.Value.KeyLength = strlen(szKeyFull);
		
		inKMS_Write(&srKeyWritePara);
		
		/* 簽單 */
		if (inFILE_Open(&ulHandle, (unsigned char*)"BMP_1_A.TXT.gz") != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		
		if (inFILE_Seek(ulHandle, 0L, _SEEK_BEGIN_) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}

		/* 取得前17到32 Bytes (szData為前32Byte) */
		memset(szData, 0x00, sizeof(szData));
		if (inFILE_Read(&ulHandle, (unsigned char*)szData, 32) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(szTemplate, &szData[16], 16);

		/* 加密 */
		memset(szEncryData, 0x00, sizeof(szEncryData));
		if (inNCCC_TMK_ESC_3DES_Encrypt(szTemplate, 16, szEncryData) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		/* 取得加密後16 Bytes並塞回去 */
		memcpy(&szData[16], szEncryData, 16);

		if (inFILE_Seek(ulHandle, 0L, _SEEK_BEGIN_) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		if (inFILE_Write(&ulHandle, (unsigned char*)szData, 32) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		
		inFILE_Rename((unsigned char*)"BMP_1_A.TXT.gz", (unsigned char*)"BMP_3_A.TXT.gz");
		inFunc_Data_Copy("BMP_3_A.TXT.gz", _AP_ROOT_PATH_, "", _SD_PATH_);
		
		/* 圖檔 */
		if (inFILE_Open(&ulHandle, (unsigned char*)"BMP_1_B.BMP.gz") != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		
		if (inFILE_Seek(ulHandle, 0L, _SEEK_BEGIN_) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}

		/* 取得前17到32 Bytes (szData為前32Byte) */
		memset(szData, 0x00, sizeof(szData));
		if (inFILE_Read(&ulHandle, (unsigned char*)szData, 32) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(szTemplate, &szData[16], 16);

		/* 加密 */
		memset(szEncryData, 0x00, sizeof(szEncryData));
		if (inNCCC_TMK_ESC_3DES_Encrypt(szTemplate, 16, szEncryData) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		
		/* 取得加密後16 Bytes並塞回去 */
		memcpy(&szData[16], szEncryData, 16);

		if (inFILE_Seek(ulHandle, 0L, _SEEK_BEGIN_) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		if (inFILE_Write(&ulHandle, (unsigned char*)szData, 32) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		
		inFILE_Rename((unsigned char*)"BMP_1_B.BMP.gz", (unsigned char*)"BMP_3_B.BMP.gz");
		inFunc_Data_Copy("BMP_3_B.TXT.gz", _AP_ROOT_PATH_, "", _SD_PATH_);
		
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_Data_Compress_Encryption_Test() END!");
		
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_ESC_Test_Redundant_Switch
Date&Time       :2023/2/23 下午 4:25
Describe        :
*/
int inNCCC_ESC_Test_Redundant_ESCPacket_Switch(void)
{
	int		inRetVal;
        char		szTemplate[64 + 1];
        DISPLAY_OBJECT  srDispObj;
        
        /* Debug */
	inDISP_Clear_Line(_LINE_8_1_, _LINE_8_8_);
        memset(szTemplate, 0x00, sizeof(szTemplate));
        
	strcpy(szTemplate, "ESC redundant switch:");
	
        inDISP_ChineseFont(szTemplate, _FONTSIZE_8X22_, _LINE_8_3_, _DISP_LEFT_);
	if (guszTestRedundantESCPacketBit == VS_TRUE)
	{
		inDISP_ChineseFont("Now: On", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
	}
	else
	{
		inDISP_ChineseFont("Now: Off", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
	}
        inDISP_ChineseFont("0 = OFF,1 = On", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);

        while (1)
        {
                memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
                srDispObj.inMaxLen = 1;
                srDispObj.inY = _LINE_8_7_;
                srDispObj.inR_L = _DISP_RIGHT_;
		srDispObj.inColor = _COLOR_RED_;

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;
	
                inRetVal = inDISP_Enter8x16(&srDispObj);

                if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
                        return (VS_ERROR);

                if (strlen(srDispObj.szOutput) > 0)
                {
                        if (srDispObj.szOutput[0] == '0')
                        {
				guszTestRedundantESCPacketBit = VS_FALSE;
                                break;
                        }
                        else if (srDispObj.szOutput[0] == '1')
                        {
				guszTestRedundantESCPacketBit = VS_TRUE;
                                break;
                        }
                        else
                        {
                                inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
                                continue;
                        }
                }
                else
                {
                        break;
                }
        }
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_ESC_Check_Again_Record_Inv_Exist
Date&Time       :2024/2/5 上午 10:50
Describe        :確認ESC資料庫是否有特定inv Rec的存在
*/
int inNCCC_ESC_Check_Again_Record_Inv_Exist(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	char	szTableName[50 + 1];		/* 若傳進的TableName1為空字串，則用szTableName組TableName */
	char			szQuerySql[200 + 1] = {0};
	TRANSACTION_OBJECT	pobDataTran;
	SQLITE_ALL_TABLE	srAll;
	
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_ESC_Check_Again_Record_Inv_Exist() START !");
	}
	
	memset(&pobDataTran, 0x00, sizeof(pobDataTran));
	pobDataTran.srBRec.inHDTIndex = pobTran->srBRec.inHDTIndex;
	pobDataTran.srBRec.lnOrgInvNum = pobTran->srBRec.lnOrgInvNum;
	pobDataTran.uszFileNameNoNeedNumBit = pobTran->uszFileNameNoNeedNumBit;
	memset(&srAll, 0x00, sizeof(srAll));
	inRetVal = inSqlite_Table_Link_BRec(&pobDataTran, &srAll, _LS_READ_);
	
	/* 由function決定TableName */
	memset(szTableName, 0x00, sizeof(szTableName));
	if (pobDataTran.uszFileNameNoNeedNumBit == VS_TRUE)
	{
		inFunc_ComposeFileName(&pobDataTran, szTableName, "", 0);
	}
	else
	{
		inFunc_ComposeFileName(&pobDataTran, szTableName, "", 6);
	}
	
	/* 會長得CR000001的形式 */
	strcat(szTableName, _BATCH_TABLE_ESC_AGAIN_SUFFIX_);
	
	memset(szQuerySql, 0x00, sizeof(szQuerySql));
	sprintf(szQuerySql, "SELECT EXISTS(SELECT 1 FROM %s WHERE lnOrgInvNum = %ld LIMIT 1)", szTableName, pobDataTran.srBRec.lnOrgInvNum);
	
	inRetVal = inSqlite_Get_Data_By_External_SQL(gszTranDBPath, &srAll, szQuerySql);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_ESC_Check_Again_Record_Inv_Exist() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :inNCCC_ESC_Check_Fail_Record_Inv_Exist
Date&Time       :2024/2/5 上午 10:50
Describe        :確認ESC資料庫是否有特定inv Rec的存在
*/
int inNCCC_ESC_Check_Fail_Record_Inv_Exist(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	char	szTableName[50 + 1];		/* 若傳進的TableName1為空字串，則用szTableName組TableName */
	char			szQuerySql[200 + 1] = {0};
	TRANSACTION_OBJECT	pobDataTran;
	SQLITE_ALL_TABLE	srAll;
	
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_ESC_Check_Fail_Record_Inv_Exist() START !");
	}
	
	memset(&pobDataTran, 0x00, sizeof(pobDataTran));
	pobDataTran.srBRec.inHDTIndex = pobTran->srBRec.inHDTIndex;
	pobDataTran.srBRec.lnOrgInvNum = pobTran->srBRec.lnOrgInvNum;
	pobDataTran.uszFileNameNoNeedNumBit = pobTran->uszFileNameNoNeedNumBit;
	memset(&srAll, 0x00, sizeof(srAll));
	inRetVal = inSqlite_Table_Link_BRec(&pobDataTran, &srAll, _LS_READ_);
	
	/* 由function決定TableName */
	memset(szTableName, 0x00, sizeof(szTableName));
	if (pobDataTran.uszFileNameNoNeedNumBit == VS_TRUE)
	{
		inFunc_ComposeFileName(&pobDataTran, szTableName, "", 0);
	}
	else
	{
		inFunc_ComposeFileName(&pobDataTran, szTableName, "", 6);
	}
	
	/* 會長得CR000001的形式 */
	strcat(szTableName, _BATCH_TABLE_ESC_FAIL_SUFFIX_);
	
	memset(szQuerySql, 0x00, sizeof(szQuerySql));
	sprintf(szQuerySql, "SELECT EXISTS(SELECT 1 FROM %s WHERE lnOrgInvNum = %ld LIMIT 1)", szTableName, pobDataTran.srBRec.lnOrgInvNum);
	
	inRetVal = inSqlite_Get_Data_By_External_SQL(gszTranDBPath, &srAll, szQuerySql);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_ESC_Check_Fail_Record_Inv_Exist() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}