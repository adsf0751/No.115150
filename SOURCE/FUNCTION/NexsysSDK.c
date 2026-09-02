#include <string.h>
#include <stdio.h>
#include <stdlib.h>
/** These two files are necessary for calling CTOS API **/
#include <ctosapi.h>
#include <emv_cl.h>
#include <sqlite3.h>
#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Define_2.h"
#include "../INCLUDES/Transaction.h"
#include "../INCLUDES/TransType.h"
#include "../INCLUDES/AllStruct.h"
#include "../DISPLAY/Display.h"
#include "../EVENT/MenuMsg.h"
#include "../EVENT/Menu.h"
#include "../EVENT/Event.h"
#include "../EVENT/Flow.h"
#include "../COMM/Comm.h"
#include "../COMM/Ethernet.h"
#include "../../CREDIT/Creditfunc.h"
#include "../../CTLS/CTLS.h"
#include "../../EMVSRC/EMVsrc.h"
#include "../../NCCC/NCCCsrc.h"
#include "../../NCCC/NCCCtms.h"
#include "../../NCCC/NCCCtmsCPT.h"
#include "../../NCCC/NCCCtmsFLT.h"
#include "../../NCCC/NCCCtmsFTP.h"
#include "../../NCCC/NCCCtmsFTPFLT.h"
#include "../../NCCC/NCCCtmsiso.h"
#include "../../NCCC/NCCCtmk.h"
#include "Sqlite.h"
#include "Accum.h"
#include "Batch.h"
#include "Card.h"
#include "CDT.h"
#include "CFGT.h"
#include "ECR.h"
#include "File.h"
#include "Function.h"
#include "FuncTable.h"
#include "HDT.h"
#include "RS232.h"
#include "NexsysSDK.h"
#include "EDC.h"
#include "CPT.h"
#include "CPT_Backup.h"
#include "HDPT.h"

extern  int		ginIdleMSRStatus, ginMenuKeyIn, ginIdleICCStatus;
extern	int		ginEventCode;
extern	int		ginMachineType;
extern	int		ginAPVersionType;
extern	int		ginDebug;
extern	int		ginFindRunTime;
extern	int		ginExamBit;
extern	int		ginFallback;
extern	char		gszTermVersionDate[16 + 1];
extern	TMS_Field58_REC	gsrTMS_Field58;
int			ginPOSPrinterDebug = VS_FALSE;
char			guszOPT_InitialBit = VS_FALSE;
long			glnExamAmt = 0;

/*
Function        :inOpt_TerminalInitial
Date&Time       :2018/9/17 上午 11:28
Describe        :Initial
*/
int OPT_TerminalInitial(void)
{
	int	inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	char	szVersion[20 + 1] = {0};
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "OPT_TerminalInitial() START !");
	}
	
	if (ginDebug == VS_TRUE)
	{
		memset(szVersion, 0x00, sizeof(szVersion));
		OPT_GET_NEXSYS_SDK_VERSION(szVersion);
		inLogPrintf(AT, "Version:%s", szVersion);
	}
	
	inFunc_CalculateRunTimeGlobal_Start();
	
	inFunc_Log_All_Firmware_Version();
	inFunc_ls("-R", "./");
				
	/* 紀錄機器類型 */
	inFunc_Decide_Machine_Type(&ginMachineType);
	/* 確認銀行版本 */
	inFunc_Decide_APVersion_Type(&ginAPVersionType);
	
	/* 開機流程(開機流程不能中途跳出，所以都強制return success) */
	inEVENT_Responder(_BOOTING_EVENT_);
	
	inDISP_ClearAll();
	
	/* 代表有執行過一次OPT_TerminalInitial */
	guszOPT_InitialBit = VS_TRUE;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "OPT_TerminalInitial() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :OPT_ReadCard
Date&Time       :2018/9/17 上午 11:33
Describe        :
*/
int OPT_ReadCard(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	char		szTMSOK[2 + 1] = {0};
	RTC_NEXSYS	srRTC; 		/* Date & Time */
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "OPT_ReadCard() START !");
	}
	
	memset(szTMSOK, 0x00, sizeof(szTMSOK));
	inGetTMSOK(szTMSOK);
	if (memcmp(szTMSOK, "Y", strlen("Y")) != 0)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "TMS未下載");
		}
		
		inRetVal = _SDK_ERROR_CODE_TMS_NOT_OK_ERROR_;
		return (inRetVal);
	}
	
	/* 尚未執行OPT_TerminalInitial，無法執行SDK內容 */
	if (guszOPT_InitialBit != VS_TRUE)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "未進行SDK初始化，不允許執行");
		}
		
		inRetVal = _SDK_ERROR_CODE_OPT_INITIAL_NOT_YET_;
		return (inRetVal);
	}
	
	/* 取得EDC時間日期 */
        memset(&srRTC, 0x00, sizeof(RTC_NEXSYS));
	if (inFunc_GetSystemDateAndTime(&srRTC) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "時間取得失敗");
		}
		inRetVal = _SDK_ERROR_CODE_GET_TIME_FAIL_;
		
		return (inRetVal);
	}
	inFunc_Sync_BRec_Date_Time(pobTran, &srRTC);
	pobTran->inTransactionCode = _SALE_;
	pobTran->inRunOperationID = _OPERATION_SDK_READ_CARD_;
	
	inRetVal = inNexsysSDK_GetCardNO_CTLS(pobTran);
	if (inRetVal == VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "取得卡號流程成功");
		}
		inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	}
	else if (inRetVal == VS_TIMEOUT)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "取得卡號超時");
		}
		inRetVal = _SDK_ERROR_CODE_TIMEOUT_;
	}
	else if (inRetVal == VS_USER_CANCEL)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "PreAuthComp 使用者取消");
		}
		inRetVal = _SDK_ERROR_CODE_USER_CANCEL_;
	}
	else
	{
		inNexsysSDK_ErrorMsg_Transform(pobTran, &inRetVal);
		
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "取得卡號流程失敗: 0x%04x", inRetVal);
		}
	}
	
	/* 關閉燈號 */
	inCTLS_Clear_LED();
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "OPT_ReadCard() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :OPT_PreAuth
Date&Time       :2018/9/19 上午 11:27
Describe        :
*/
int OPT_PreAuth(TRANSACTION_OBJECT *pobTran)
{
	int		inPasswordLevel = _ACCESS_FREELY_;
	int		inCode = _TRANS_TYPE_NULL_;
	int		inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	int		inCardLabelLen = 0;
	char		szTMSOK[2 + 1] = {0};
	char		szTemplate2[20 + 1] = {0};
	RTC_NEXSYS	srRTC; 		/* Date & Time */
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "OPT_PreAuth() START !");
	}
	
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_ResetAll();
		inFunc_RecordTime_Append("%d %s, ", __LINE__, __FUNCTION__);
	}
	
	memset(szTMSOK, 0x00, sizeof(szTMSOK));
	inGetTMSOK(szTMSOK);
	if (memcmp(szTMSOK, "Y", strlen("Y")) != 0)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "TMS未下載");
		}
		inRetVal = _SDK_ERROR_CODE_TMS_NOT_OK_ERROR_;
		return (inRetVal);
	}
	
	/* 尚未執行OPT_TerminalInitial，無法執行SDK內容 */
	if (guszOPT_InitialBit != VS_TRUE)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "未進行SDK初始化，不允許執行");
		}
		
		inRetVal = _SDK_ERROR_CODE_OPT_INITIAL_NOT_YET_;
		return (inRetVal);
	}
	
	/* 輸入密碼的層級 */
	inPasswordLevel = _ACCESS_WITH_CUSTOM_;
	inCode = _PRE_AUTH_;
	/* 第一層輸入密碼 */
	if (inFunc_CheckCustomizePassword(inPasswordLevel, inCode) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "管理密碼錯誤");
		}
		inRetVal = _SDK_ERROR_CODE_PASSWORD_ERROR_;
		
		return (inRetVal);
	}
	
	/* 檢核資料 */
	if (pobTran->srBRec.lnTxnAmount == 0)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "金額為0，無法預先授權");
		}
		inRetVal = _SDK_ERROR_CODE_TXN_AMOUNT_0_;
		
		return (inRetVal);
	}
	
	/* 取得EDC時間日期 */
        memset(&srRTC, 0x00, sizeof(RTC_NEXSYS));
	if (inFunc_GetSystemDateAndTime(&srRTC) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "時間取得失敗");
		}
		inRetVal = _SDK_ERROR_CODE_GET_TIME_FAIL_;
		
		return (inRetVal);
	}
	
	inFunc_Sync_BRec_Date_Time(pobTran, &srRTC);
	pobTran->inTransactionCode = _PRE_AUTH_;
	pobTran->srBRec.inCode = _PRE_AUTH_;
	pobTran->srBRec.inOrgCode = _PRE_AUTH_;
	if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
	{
		pobTran->inRunTRTID = _TRT_PRE_AUTH_CTLS_;
	}
	else if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
	{
		pobTran->inRunTRTID = _TRT_PRE_AUTH_ICC_;
	}
	else
	{
		pobTran->inRunTRTID = _TRT_PRE_AUTH_;
	}
	
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("HDPT Before");
	}
	
	inLoadHDTRec(pobTran->srBRec.inHDTIndex);
	inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
	inRetVal = inFunc_Get_HDPT_General_Data(pobTran);
	if (inRetVal != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_GET_HDPT_TAG_ERROR_;
		
		return (inRetVal);
	}
	
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("HDPT After");
	}
	
	inRetVal = inNCCC_RunTRT(pobTran, pobTran->inRunTRTID);
	inFunc_Display_Error(pobTran);
	
	/* 【需求單 - 106349】自有品牌判斷需求 */
	/* 如果是自有品牌，要幫忙轉卡別名稱給POS*/
	if (pobTran->srBRec.uszUCARDTransBit == VS_TRUE)
	{
		memset(szTemplate2, 0x00, sizeof(szTemplate2));
		memcpy(szTemplate2, &pobTran->srBRec.szMCP_BANKID[5], 2);
		inCardLabelLen = atoi(szTemplate2);
		memset(szTemplate2, 0x00, sizeof(szTemplate2));
		memcpy(szTemplate2, &pobTran->srBRec.szMCP_BANKID[7], inCardLabelLen);
		
		/* 清空卡別名稱 */
		memset(pobTran->srBRec.szCardLabel, 0x00, sizeof(pobTran->srBRec.szCardLabel));
		strcpy(pobTran->srBRec.szCardLabel, szTemplate2);
		
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "Final CardLabel:%s", pobTran->srBRec.szCardLabel);
		}
	}
	
	if (inRetVal == VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "PreAuth 流程成功");
		}
		inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	}
	else if (inRetVal == VS_TIMEOUT)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "PreAuth 流程超時");
		}
		inRetVal = _SDK_ERROR_CODE_TIMEOUT_;
	}
	else if (inRetVal == VS_COMM_ERROR)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "PreAuth 通訊失敗");
		}
		inRetVal = _SDK_ERROR_CODE_TXN_COMM_ERROR_;
	}
	else if (inRetVal == VS_USER_CANCEL)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "PreAuth 使用者取消");
		}
		inRetVal = _SDK_ERROR_CODE_USER_CANCEL_;
	}
	else if (pobTran->inTransactionResult == _TRAN_RESULT_CANCELLED_)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "PreAuth 交易拒絕");
			inLogPrintf(AT, "主機回覆碼:%s", pobTran->srBRec.szRespCode);
		}
		inRetVal = _SDK_ERROR_CODE_TXN_DECLINE_;
	}
	else
	{
		inNexsysSDK_ErrorMsg_Transform(pobTran, &inRetVal);
		
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "PreAuth 流程失敗: 0x%04x", inRetVal);
		}
	}
	
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("OPT_PreAuth() END");
		inFunc_RecordTime_WatchAll();
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "OPT_PreAuth() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :OPT_PreAuthComp
Date&Time       :2018/9/19 上午 11:27
Describe        :
*/
int OPT_PreAuthComp(TRANSACTION_OBJECT *pobTran)
{
	int			inPasswordLevel = _ACCESS_FREELY_;
	int			inCode = _TRANS_TYPE_NULL_;
	int			inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	int			inFuncRetVal = VS_SUCCESS;
	int			inCardLabelLen = 0;
	char			szTMSOK[2 + 1] = {0};
	char			szTemplate2[20 + 1] = {0};
	RTC_NEXSYS		srRTC = {}; 		/* Date & Time */
	TRANSACTION_OBJECT	pobOrgTran = {};
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "OPT_PreAuthComp() START !");
	}
	
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_ResetAll();
		inFunc_RecordTime_Append("OPT_PreAuthComp START");
	}
	
	memset(szTMSOK, 0x00, sizeof(szTMSOK));
	inGetTMSOK(szTMSOK);
	if (memcmp(szTMSOK, "Y", strlen("Y")) != 0)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "TMS未下載");
		}
		inRetVal = _SDK_ERROR_CODE_TMS_NOT_OK_ERROR_;
		return (inRetVal);
	}
	
	/* 尚未執行OPT_TerminalInitial，無法執行SDK內容 */
	if (guszOPT_InitialBit != VS_TRUE)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "未進行SDK初始化，不允許執行");
		}
		
		inRetVal = _SDK_ERROR_CODE_OPT_INITIAL_NOT_YET_;
		return (inRetVal);
	}
	
	/* 輸入密碼的層級 */
	inPasswordLevel = _ACCESS_WITH_CUSTOM_;
	inCode = _SALE_OFFLINE_;
	/* 第一層輸入密碼 */
	if (inFunc_CheckCustomizePassword(inPasswordLevel, inCode) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "管理密碼錯誤");
		}
		inRetVal = _SDK_ERROR_CODE_PASSWORD_ERROR_;
		
		return (inRetVal);
	}
	
	/* 檢核資料 */
	/* 用調閱編號取得原交易日期 */
	if (pobTran->srBRec.lnOrgInvNum <= 0)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "無調閱編號，無法取出原交易日期");
		}
		
		inRetVal = _SDK_ERROR_CODE_TXN_INVOICE_NUMBER_ERROR_;
		
		return (inRetVal);
	}
	else
	{
		/* Load Batch Record */
		memset(&pobOrgTran, 0x00, sizeof(pobOrgTran));
		pobOrgTran.srBRec.inHDTIndex = pobTran->srBRec.inHDTIndex;
		pobOrgTran.srBRec.lnOrgInvNum = pobTran->srBRec.lnOrgInvNum;
		inFuncRetVal = inBATCH_GetTransRecord_By_Sqlite(&pobOrgTran);

		if (inFuncRetVal == VS_SUCCESS)
		{
			/* 更改過卡方式 */
			pobOrgTran.srBRec.inChipStatus = _NOT_USING_CHIP_;
			pobOrgTran.srBRec.uszContactlessBit = VS_FALSE;
			pobOrgTran.srBRec.uszManualBit = VS_TRUE;
			/* 清除EMV資料 */
			memset(&pobOrgTran.srEMVRec, 0x00, sizeof(pobOrgTran.srEMVRec));
			
			/* 取得原交易日期 */
			memset(pobOrgTran.srBRec.szCUP_TD, 0x00, sizeof(pobOrgTran.srBRec.szCUP_TD));
			memcpy(pobOrgTran.srBRec.szCUP_TD, &pobOrgTran.srBRec.szDate[4], 4);
			/* 原交易金額 */
			pobOrgTran.srBRec.lnOrgTxnAmount = pobOrgTran.srBRec.lnTxnAmount;
			/* 預先授權完成金額 */
			pobOrgTran.srBRec.lnTxnAmount = pobTran->srBRec.lnTxnAmount;
		}
		/* 找不到 */
		else if (inFuncRetVal == VS_NO_RECORD)
		{
			inRetVal = _SDK_ERROR_CODE_NO_TXN_RECORD_ERROR_;
		
			return (inRetVal);
		}
		else
		{
			inRetVal = _SDK_ERROR_CODE_READ_BATCH_ERROR_;
		
			return (inRetVal);
		}
	}
	
	if (strlen(pobOrgTran.srBRec.szCUP_TD) <= 0)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "無原交易日期，無法預先授權完成");
		}
		inRetVal = _SDK_ERROR_CODE_TXN_ORGDATE_ERROR_;
		
		return (inRetVal);
	}
	
	inFuncRetVal = inFunc_CheckValidOriDate(pobOrgTran.srBRec.szCUP_TD);
	if (inFuncRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "原交易日期有誤，無法預先授權完成");
			inLogPrintf(AT, "%s", pobOrgTran.srBRec.szCUP_TD);
		}
		inRetVal = _SDK_ERROR_CODE_TXN_ORGDATE_ERROR_;
		
		return (inRetVal);
	}
	
	/* 授權碼 */
	inFuncRetVal = inFunc_Check_AuthCode_Validate(pobOrgTran.srBRec.szAuthCode);
	if (inFuncRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "授權碼不合法，無法預先授權完成");
		}
		inRetVal = _SDK_ERROR_CODE_TXN_AUTHCODE_ERROR_;
		
		return (inRetVal);
	}
	
	/* 檢核授權碼分銀聯和非銀聯 */
	if (pobOrgTran.srBRec.uszCUPTransBit == VS_TRUE)
	{
		/* CUP沒輸入授權碼補空白 */
		if (strlen(pobOrgTran.srBRec.szAuthCode) == 0)
		{
			memset(pobOrgTran.srBRec.szAuthCode, 0x00, sizeof(pobOrgTran.srBRec.szAuthCode));
			/* 補空白 */
			strcpy(pobOrgTran.srBRec.szAuthCode, "      ");
		}
	}
	else
	{
		/* 小於2碼或全0或全空白不給過 */
		if (strlen(pobOrgTran.srBRec.szAuthCode) < 2			|| 
		    memcmp(pobOrgTran.srBRec.szAuthCode, "000000", 6) == 0	|| 
		    memcmp(pobOrgTran.srBRec.szAuthCode, "      ", 6) == 0)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "授權碼小於兩碼、全0或全空白，無法預先授權完成");
				inLogPrintf(AT, "AuthCode:%s", pobOrgTran.srBRec.szAuthCode);
			}
			inRetVal = _SDK_ERROR_CODE_TXN_AUTHCODE_ERROR_;

			return (inRetVal);
		}
	}
	
	/* 檢核原預先授權金額 */
	if (pobOrgTran.srBRec.lnOrgTxnAmount == 0)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "原預先授權金額為0，無法預先授權完成");
		}
		inRetVal = _SDK_ERROR_CODE_TXN_AMOUNT_0_;
		
		return (inRetVal);
	}
	
	/* 預先授權完成金額 */
	if (pobOrgTran.srBRec.lnTxnAmount == 0)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "預先授權完成金額為0，無法預先授權完成");
		}
		inRetVal = _SDK_ERROR_CODE_TXN_AMOUNT_0_;
		
		return (inRetVal);
	}
	
	/* 取得EDC時間日期 */
        memset(&srRTC, 0x00, sizeof(RTC_NEXSYS));
	if (inFunc_GetSystemDateAndTime(&srRTC) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "時間取得失敗");
		}
		inRetVal = _SDK_ERROR_CODE_GET_TIME_FAIL_;
		
		return (inRetVal);
	}
	
	inFunc_Sync_BRec_Date_Time(&pobOrgTran, &srRTC);
	pobOrgTran.inTransactionCode = inCode;
	pobOrgTran.srBRec.inCode = inCode;
	pobOrgTran.srBRec.inOrgCode = inCode;
	pobOrgTran.inRunTRTID = _TRT_SALE_OFFLINE_;
	inLoadHDTRec(pobOrgTran.srBRec.inHDTIndex);
	inLoadHDPTRec(pobOrgTran.srBRec.inHDTIndex);
	inRetVal = inFunc_Get_HDPT_General_Data(&pobOrgTran);
	if (inRetVal != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_GET_HDPT_TAG_ERROR_;
		
		return (inRetVal);
	}
	
	inRetVal = inNCCC_RunTRT(&pobOrgTran, pobOrgTran.inRunTRTID);
	inFunc_Display_Error(&pobOrgTran);
	
	/* 【需求單 - 106349】自有品牌判斷需求 */
	/* 如果是自有品牌，要幫忙轉卡別名稱給POS*/
	if (pobTran->srBRec.uszUCARDTransBit == VS_TRUE)
	{
		memset(szTemplate2, 0x00, sizeof(szTemplate2));
		memcpy(szTemplate2, &pobTran->srBRec.szMCP_BANKID[5], 2);
		inCardLabelLen = atoi(szTemplate2);
		memset(szTemplate2, 0x00, sizeof(szTemplate2));
		memcpy(szTemplate2, &pobTran->srBRec.szMCP_BANKID[7], inCardLabelLen);
		
		/* 清空卡別名稱 */
		memset(pobTran->srBRec.szCardLabel, 0x00, sizeof(pobTran->srBRec.szCardLabel));
		strcpy(pobTran->srBRec.szCardLabel, szTemplate2);
		
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "Final CardLabel:%s", pobTran->srBRec.szCardLabel);
		}
	}
	
	if (inRetVal == VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "PreAuthComp 流程成功");
		}
		inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	}
	else if (inRetVal == VS_TIMEOUT)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "PreAuthComp 流程超時");
		}
		inRetVal = _SDK_ERROR_CODE_TIMEOUT_;
	}
	else if (inRetVal == VS_COMM_ERROR)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "PreAuthComp 通訊失敗");
		}
		inRetVal = _SDK_ERROR_CODE_TXN_COMM_ERROR_;
	}
	else if (inRetVal == VS_USER_CANCEL)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "PreAuthComp 使用者取消");
		}
		inRetVal = _SDK_ERROR_CODE_USER_CANCEL_;
	}
	else if (pobOrgTran.inTransactionResult == _TRAN_RESULT_CANCELLED_)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "PreAuthComp 交易拒絕");
			inLogPrintf(AT, "主機回覆碼:%s", pobOrgTran.srBRec.szRespCode);
		}
		inRetVal = _SDK_ERROR_CODE_TXN_DECLINE_;
	}
	else
	{
		inNexsysSDK_ErrorMsg_Transform(&pobOrgTran, &inRetVal);
		
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "PreAuthComp 流程失敗: 0x%04x", inRetVal);
		}
	}
	
	memset(pobTran, 0x00, sizeof(TRANSACTION_OBJECT));
	memcpy(pobTran, &pobOrgTran, sizeof(pobOrgTran));
	
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("OPT_PreAuthComp() END");
		inFunc_RecordTime_WatchAll();
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "OPT_PreAuthComp() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :OPT_Refund
Date&Time       :2018/10/9 下午 7:20
Describe        :
*/
int OPT_Refund(TRANSACTION_OBJECT *pobTran)
{
	int		inPasswordLevel = _ACCESS_FREELY_;
	int		inCode = _TRANS_TYPE_NULL_;
	int		inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	int		inFuncVal = VS_SUCCESS;
	int		inCardLabelLen = 0;
	char		szTMSOK[2 + 1] = {0};
	char		szTemplate2[20 + 1] = {0};
	char		szFuncEnable[2 + 1] = {0};
	char		szEXPDate[5 + 1] = {0};
	RTC_NEXSYS	srRTC; 		/* Date & Time */
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "OPT_Refund() START !");
	}
	
	memset(szTMSOK, 0x00, sizeof(szTMSOK));
	inGetTMSOK(szTMSOK);
	if (memcmp(szTMSOK, "Y", strlen("Y")) != 0)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "TMS未下載");
		}
		inRetVal = _SDK_ERROR_CODE_TMS_NOT_OK_ERROR_;
		return (inRetVal);
	}
	
	/* 尚未執行OPT_TerminalInitial，無法執行SDK內容 */
	if (guszOPT_InitialBit != VS_TRUE)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "未進行SDK初始化，不允許執行");
		}
		
		inRetVal = _SDK_ERROR_CODE_OPT_INITIAL_NOT_YET_;
		return (inRetVal);
	}
	
	/* 輸入密碼的層級 */
	inPasswordLevel = _ACCESS_WITH_CUSTOM_;
	inCode = _REFUND_;
	/* 第一層輸入密碼 */
	if (inFunc_CheckCustomizePassword(inPasswordLevel, inCode) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "管理密碼錯誤");
		}
		inRetVal = _SDK_ERROR_CODE_PASSWORD_ERROR_;
		
		return (inRetVal);
	}
	
	/* 檢核資料 */
	if (pobTran->srBRec.lnTxnAmount == 0)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "金額為0，無法退貨");
		}
		inRetVal = _SDK_ERROR_CODE_TXN_AMOUNT_0_;
		
		return (inRetVal);
	}
	
	/* 檢核卡號 */
	/* U卡要補卡號 */
	if (inCARD_Generate_Special_Card(pobTran->srBRec.szPAN) != VS_SUCCESS)
	{
		/* U卡補卡號失敗 */
		inRetVal = _SDK_ERROR_CODE_ERROR_;
		
		return (inRetVal);
	}
	
	 /* 判斷card bin 讀HDT */
	if (inCARD_GetBin(pobTran) != VS_SUCCESS)
	{
		pobTran->inErrorMsg = _SDK_ERROR_CODE_CARD_GET_BIN_;
		inRetVal = pobTran->inErrorMsg;
		return(inRetVal);
	}

	/* 第四步驟檢核PAN module 10 */
	memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
	inGetModule10Check(szFuncEnable);
	if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
	{
		/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
		if (inCARD_ValidTrack2_PAN(pobTran) != VS_SUCCESS)
		{
			pobTran->inErrorMsg = _SDK_ERROR_CODE_CARD_MODULE_10_ERROR_;
			inRetVal = pobTran->inErrorMsg;
			return(inRetVal);
		}
	}
	
	if (strlen(pobTran->srBRec.szCheckNO) > 0)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "檢查碼轉換：%s", pobTran->srBRec.szCheckNO);
		}
		memset(szEXPDate, 0x00, sizeof(szEXPDate));
		
		inFuncVal = inCARD_ExpDateEncryptAndDecrypt(pobTran, pobTran->srBRec.szCheckNO, szEXPDate, _EXP_DECRYPT_);
		if (inFuncVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "有效期轉換失敗");
			}
			pobTran->inErrorMsg = _SDK_ERROR_CODE_EXPDATE_TRANSFORM_FAIL_;
			inRetVal = pobTran->inErrorMsg;
			return(inRetVal);
		}
		else
		{
			memcpy(&pobTran->srBRec.szExpDate[0], &szEXPDate[2], 2);
			memcpy(&pobTran->srBRec.szExpDate[2], &szEXPDate[0], 2);
			pobTran->uszInputCheckNoBit = VS_TRUE;
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "有效期：%s", pobTran->srBRec.szCheckNO);
		}
		if (strlen(pobTran->srBRec.szExpDate) > 0)
		{
			
		}
		else
		{
			pobTran->inErrorMsg = _SDK_ERROR_CODE_CARD_EXP_ERROR_;
			inRetVal = pobTran->inErrorMsg;
			return(inRetVal);
		}
	}
	
	/* 檢核ExpDate */
	memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
	inGetExpiredDateCheck(szFuncEnable);
	if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
	{
		if (inCARD_ValidTrack2_ExpDate(pobTran) != VS_SUCCESS)
		{
			pobTran->inErrorMsg = _SDK_ERROR_CODE_CARD_EXP_ERROR_;
			inRetVal = pobTran->inErrorMsg;
			return(inRetVal);
		}
	}
			
	/* 取得EDC時間日期 */
        memset(&srRTC, 0x00, sizeof(RTC_NEXSYS));
	if (inFunc_GetSystemDateAndTime(&srRTC) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "時間取得失敗");
		}
		inRetVal = _SDK_ERROR_CODE_GET_TIME_FAIL_;
		
		return (inRetVal);
	}
	
	inFunc_Sync_BRec_Date_Time(pobTran, &srRTC);
	pobTran->inTransactionCode = _REFUND_;
	pobTran->srBRec.inCode = _REFUND_;
	pobTran->srBRec.inOrgCode = _REFUND_;
	/* 表示是手動輸入 */
	pobTran->srBRec.uszManualBit = VS_TRUE;
	if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
	{
		pobTran->inRunTRTID = _TRT_REFUND_CTLS_;
	}
	else
	{
		pobTran->inRunTRTID = _TRT_REFUND_;
	}
	
	inLoadHDTRec(pobTran->srBRec.inHDTIndex);
	inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
	inRetVal = inFunc_Get_HDPT_General_Data(pobTran);
	if (inRetVal != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_GET_HDPT_TAG_ERROR_;
		
		return (inRetVal);
	}
	
	/* 人工輸入卡號開關 */
	/* 退貨交易及預先授權/預先授權完成交易部參考人工輸入功能開關 */
	memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
	inGetManualKeyin(szFuncEnable);
	if (memcmp(szFuncEnable, "Y", strlen("Y")) != 0		&& 
	   (pobTran->inTransactionCode != _REFUND_		&& 
	    pobTran->inTransactionCode != _INST_REFUND_		&&
	    pobTran->inTransactionCode != _REDEEM_REFUND_	&&
	    pobTran->inTransactionCode != _PRE_AUTH_		&&
	    pobTran->inTransactionCode != _PRE_COMP_		&&
	    pobTran->inTransactionCode != _REDEEM_REFUND_	&&
	    pobTran->inTransactionCode != _CUP_REFUND_		&&
	    pobTran->inTransactionCode != _CUP_INST_REFUND_	&&
	    pobTran->inTransactionCode != _CUP_REDEEM_REFUND_	&&
	    pobTran->inTransactionCode != _CUP_PRE_AUTH_	&& 
	    pobTran->inTransactionCode != _CUP_PRE_COMP_))
	{
		inRetVal =  _SDK_ERROR_CODE_FUNCTION_CLOSE_ERROR_;

		return (inRetVal);
	}
	
	
	inRetVal = inNCCC_RunTRT(pobTran, pobTran->inRunTRTID);
	inFunc_Display_Error(pobTran);
	
	/* 【需求單 - 106349】自有品牌判斷需求 */
	/* 如果是自有品牌，要幫忙轉卡別名稱給POS */
	if (pobTran->srBRec.uszUCARDTransBit == VS_TRUE)
	{
		memset(szTemplate2, 0x00, sizeof(szTemplate2));
		memcpy(szTemplate2, &pobTran->srBRec.szMCP_BANKID[5], 2);
		inCardLabelLen = atoi(szTemplate2);
		memset(szTemplate2, 0x00, sizeof(szTemplate2));
		memcpy(szTemplate2, &pobTran->srBRec.szMCP_BANKID[7], inCardLabelLen);
		
		/* 清空卡別名稱 */
		memset(pobTran->srBRec.szCardLabel, 0x00, sizeof(pobTran->srBRec.szCardLabel));
		strcpy(pobTran->srBRec.szCardLabel, szTemplate2);
		
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "Final CardLabel:%s", pobTran->srBRec.szCardLabel);
		}
	}
	
	if (inRetVal == VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "Refund 流程成功");
		}
		inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	}
	else if (inRetVal == VS_TIMEOUT)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "Refund 流程超時");
		}
		inRetVal = _SDK_ERROR_CODE_TIMEOUT_;
	}
	else if (inRetVal == VS_COMM_ERROR)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "Refund 通訊失敗");
		}
		inRetVal = _SDK_ERROR_CODE_TXN_COMM_ERROR_;
	}
	else if (inRetVal == VS_USER_CANCEL)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "Refund 使用者取消");
		}
		inRetVal = _SDK_ERROR_CODE_USER_CANCEL_;
	}
	else if (pobTran->inTransactionResult == _TRAN_RESULT_CANCELLED_)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "Refund 交易拒絕");
			inLogPrintf(AT, "主機回覆碼:%s", pobTran->srBRec.szRespCode);
		}
		inRetVal = _SDK_ERROR_CODE_TXN_DECLINE_;
	}
	else
	{
		inNexsysSDK_ErrorMsg_Transform(pobTran, &inRetVal);
		
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "Refund 流程失敗: 0x%04x", inRetVal);
		}
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "OPT_Refund() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :OPT_Settle
Date&Time       :2018/9/27 下午 5:46
Describe        :
*/
int OPT_Settle(TRANSACTION_OBJECT *pobTran)
{
	int		inPasswordLevel = _ACCESS_FREELY_;
	int		inCode = _TRANS_TYPE_NULL_;
	int		inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	char		szTMSOK[2 + 1] = {0};
	RTC_NEXSYS	srRTC; 		/* Date & Time */
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "OPT_Settle() START !");
	}
	
	/* 清空 */
	memset(pobTran, 0x00, sizeof(TRANSACTION_OBJECT));
	
	memset(szTMSOK, 0x00, sizeof(szTMSOK));
	inGetTMSOK(szTMSOK);
	if (memcmp(szTMSOK, "Y", strlen("Y")) != 0)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "TMS未下載");
		}
		inRetVal = _SDK_ERROR_CODE_TMS_NOT_OK_ERROR_;
		return (inRetVal);
	}
	
	/* 尚未執行OPT_TerminalInitial，無法執行SDK內容 */
	if (guszOPT_InitialBit != VS_TRUE)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "未進行SDK初始化，不允許執行");
		}
		
		inRetVal = _SDK_ERROR_CODE_OPT_INITIAL_NOT_YET_;
		return (inRetVal);
	}
	
	/* 輸入密碼的層級 */
	inPasswordLevel = _ACCESS_WITH_CUSTOM_;
	inCode = _SETTLE_;
	/* 第一層輸入密碼 */
	if (inFunc_CheckCustomizePassword(inPasswordLevel, inCode) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "管理密碼錯誤");
		}
		inRetVal = _SDK_ERROR_CODE_PASSWORD_ERROR_;
		
		return (inRetVal);
	}
	
	/* 取得EDC時間日期 */
        memset(&srRTC, 0x00, sizeof(RTC_NEXSYS));
	if (inFunc_GetSystemDateAndTime(&srRTC) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "時間取得失敗");
		}
		inRetVal = _SDK_ERROR_CODE_GET_TIME_FAIL_;
		
		return (inRetVal);
	}
	inFunc_Sync_BRec_Date_Time(pobTran, &srRTC);
	
	pobTran->srBRec.inHDTIndex = 0;
	pobTran->inTransactionCode = _SETTLE_;
	pobTran->srBRec.inCode = _SETTLE_;
	pobTran->inRunTRTID = _TRT_SETTLE_;
	inLoadHDTRec(pobTran->srBRec.inHDTIndex);
	inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
	
	inRetVal = inNCCC_RunTRT(pobTran, pobTran->inRunTRTID);
	inFunc_Display_Error(pobTran);
	
	if (inRetVal == VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "Settle 流程成功");
		}
		inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	}
	else if (inRetVal == VS_TIMEOUT)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "Settle 流程超時");
		}
		inRetVal = _SDK_ERROR_CODE_TIMEOUT_;
	}
	else if (inRetVal == VS_COMM_ERROR)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "Settle 通訊失敗");
		}
		inRetVal = _SDK_ERROR_CODE_TXN_COMM_ERROR_;
	}
	else if (inRetVal == VS_USER_CANCEL)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "Settle 使用者取消");
		}
		inRetVal = _SDK_ERROR_CODE_USER_CANCEL_;
	}
	else if (pobTran->inTransactionResult == _TRAN_RESULT_CANCELLED_)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "Settle 交易拒絕");
			inLogPrintf(AT, "主機回覆碼:%s", pobTran->srBRec.szRespCode);
		}
		inRetVal = _SDK_ERROR_CODE_TXN_DECLINE_;
	}
	else
	{
		inNexsysSDK_ErrorMsg_Transform(pobTran, &inRetVal);
		
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "Settle 流程失敗: 0x%04x", inRetVal);
		}
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "OPT_Settle() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :OPT_TMS_DOWNLOAD
Date&Time       :2018/10/1 下午 3:27
Describe        :
*/
int OPT_TMS_DOWNLOAD(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "OPT_TMS_DOWNLOAD() START !");
	}
	
	/* 尚未執行OPT_TerminalInitial，無法執行SDK內容 */
	if (guszOPT_InitialBit != VS_TRUE)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "未進行SDK初始化，不允許執行");
		}
		
		inRetVal = _SDK_ERROR_CODE_OPT_INITIAL_NOT_YET_;
		return (inRetVal);
	}
	
	/* ISO填'N' */
	pobTran->uszFTP_TMS_Download = _TMS_DOWNLOAD_SECURE_MODE_NONE_;
	
	/* */
	inRetVal = inNexsysSDK_TMS_Download(pobTran);
	if (inRetVal == VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "TMS下載 流程成功");
		}
		inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	}
	else if (inRetVal == VS_TIMEOUT)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "TMS下載 流程超時");
		}
		inRetVal = _SDK_ERROR_CODE_TIMEOUT_;
	}
	else if (inRetVal == VS_COMM_ERROR)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "TMS下載 通訊失敗");
		}
		inRetVal = _SDK_ERROR_CODE_TXN_COMM_ERROR_;
	}
	else if (inRetVal == VS_USER_CANCEL)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "PreAuthComp 使用者取消");
		}
		inRetVal = _SDK_ERROR_CODE_USER_CANCEL_;
	}
	else if (pobTran->inTransactionResult == _TRAN_RESULT_CANCELLED_)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "TMS下載 交易拒絕");
		}
		inRetVal = _SDK_ERROR_CODE_TXN_DECLINE_;
	}
	else
	{
		inNexsysSDK_ErrorMsg_Transform(pobTran, &inRetVal);
		
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "TMS下載 流程失敗: 0x%04x", inRetVal);
		}
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "OPT_TMS_DOWNLOAD() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :OPT_TMS_DOWNLOAD_FTPS
Date&Time       :2018/10/2 上午 10:01
Describe        :
*/
int OPT_TMS_DOWNLOAD_FTPS(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	char	szCFESMode[2 + 1] = {0};
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "OPT_TMS_DOWNLOAD_FTPS() START !");
	}
	
	/* 尚未執行OPT_TerminalInitial，無法執行SDK內容 */
	if (guszOPT_InitialBit != VS_TRUE)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "未進行SDK初始化，不允許執行");
		}
		
		inRetVal = _SDK_ERROR_CODE_OPT_INITIAL_NOT_YET_;
		return (inRetVal);
	}
	
	/* FTPS填'Y' */
	memset(szCFESMode, 0x00, sizeof(szCFESMode));
	inGetCloud_MFES(szCFESMode);
	if (memcmp(szCFESMode, "Y", strlen("Y")) == 0)
	{
		pobTran->uszFTP_TMS_Download = _TMS_DOWNLOAD_SECURE_MODE_SFTP_;
	}
	else
	{
		pobTran->uszFTP_TMS_Download = _TMS_DOWNLOAD_SECURE_MODE_FTPS_;
	}
	
	/* */
	inRetVal = inNexsysSDK_TMS_Download(pobTran);
	if (inRetVal == VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "TMS下載 流程成功");
		}
		inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	}
	else if (inRetVal == VS_TIMEOUT)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "TMS下載 流程超時");
		}
		inRetVal = _SDK_ERROR_CODE_TIMEOUT_;
	}
	else if (inRetVal == VS_COMM_ERROR)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "TMS下載 通訊失敗");
		}
		inRetVal = _SDK_ERROR_CODE_TXN_COMM_ERROR_;
	}
	else if (inRetVal == VS_USER_CANCEL)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "PreAuthComp 使用者取消");
		}
		inRetVal = _SDK_ERROR_CODE_USER_CANCEL_;
	}
	else if (pobTran->inTransactionResult == _TRAN_RESULT_CANCELLED_)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "TMS下載 交易拒絕");
		}
		inRetVal = _SDK_ERROR_CODE_TXN_DECLINE_;
	}
	else
	{
		inNexsysSDK_ErrorMsg_Transform(pobTran, &inRetVal);
		
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "TMS下載 流程失敗: 0x%04x", inRetVal);
		}
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "OPT_TMS_DOWNLOAD_FTPS() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :OPT_WRITE_NCCC_TEST_KEY
Date&Time       :2018/9/21 下午 4:10
Describe        :
*/
int OPT_WRITE_NCCC_TEST_KEY()
{
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "OPT_WRITE_NCCC_TEST_KEY() START !");
	}
	
	inNCCC_TMK_Write_Test_TMK_By_Terminal();
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "OPT_WRITE_NCCC_TEST_KEY() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :OPT_WRITE_NCCC_TEST_KEY_SELF_TRANS
Date&Time       :2018/11/30 下午 7:06
Describe        :
*/
int OPT_WRITE_NCCC_TEST_KEY_SELF_TRANS()
{
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "OPT_WRITE_NCCC_TEST_KEY_SELF_TRANS() START !");
	}
	
	inNCCC_TMK_Write_Test_TMK_By_Terminal_Self_Trans();
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "OPT_WRITE_NCCC_TEST_KEY_SELF_TRANS() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :OPT_WRITE_NCCC_TMK_BY_KEY_CARD
Date&Time       :2018/11/19 下午 5:25
Describe        :
*/
int OPT_WRITE_NCCC_TMK_BY_KEY_CARD()
{
	TRANSACTION_OBJECT	pobTran;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "OPT_WRITE_NCCC_TMK_BY_KEY_CARD() START !");
	}
	
	memset(&pobTran, 0x00, sizeof(pobTran));
	pobTran.inRunOperationID = _OPERATION_LOAD_KEY_FROM_KEY_CARD_;
	
	inFLOW_RunOperation(&pobTran, pobTran.inRunOperationID);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "OPT_WRITE_NCCC_TMK_BY_KEY_CARD() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :OPT_GET_NEXSYS_SDK_VERSION
Date&Time       :2018/9/20 下午 4:05
Describe        :
*/
int OPT_GET_NEXSYS_SDK_VERSION(char* szVersion)
{
	int	inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "OPT_GET_NEXSYS_SDK_VERSION() START !");
	}
	
	if (szVersion == NULL)
	{
		inRetVal = _SDK_ERROR_CODE_POINTER_NULL_ERROR_;
		
		return (inRetVal);
	}
	
	sprintf(szVersion, "%s_%02d", gszTermVersionDate, 0);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "OPT_GET_NEXSYS_SDK_VERSION() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :OPT_GET_TMS_IP
Date&Time       :2018/9/20 下午 3:08
Describe        :取得TMS IP
*/
int OPT_GET_TMS_IP(char* szTMS_IP)
{
	int	inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "OPT_GET_TMS_IP() START !");
	}
	
	if (szTMS_IP == NULL)
	{
		inRetVal = _SDK_ERROR_CODE_POINTER_NULL_ERROR_;
		
		return (inRetVal);
	}
	
	if (inLoadTMSCPTRec(0) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_LOAD_TMSCPT_ERROR_;
		
		return (inRetVal);
	}
	
	if (inGetTMSIPAddress(szTMS_IP) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_GET_TMSCPT_TAG_ERROR_;
		
		return (inRetVal);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "OPT_GET_TMS_IP() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :OPT_SET_TMS_IP
Date&Time       :2018/9/19 下午 5:53
Describe        :設定TMS IP
*/
int OPT_SET_TMS_IP(char* szTMS_IP)
{
	int	inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "OPT_SET_TMS_IP() START !");
	}
	
	if (strlen(szTMS_IP) <= 0 ||	strlen(szTMS_IP) > 15)
	{
		inRetVal = _SDK_ERROR_CODE_SET_LENGTH_ERROR_;
		
		return (inRetVal);
	}
	
	if (inLoadTMSCPTRec(0) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_LOAD_TMSCPT_ERROR_;
		
		return (inRetVal);
	}
	
	if (inSetTMSIPAddress(szTMS_IP) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_SET_TMSCPT_TAG_ERROR_;
		
		return (inRetVal);
	}
	
	if (inSaveTMSCPTRec(0) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_SAVE_TMSCPT_ERROR_;
		
		return (inRetVal);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "OPT_SET_TMS_IP() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :OPT_GET_TMS_PORT
Date&Time       :2018/9/20 下午 3:16
Describe        :取得TMS PORT
*/
int OPT_GET_TMS_PORT(char* szTMS_PORT)
{
	int	inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "OPT_GET_TMS_PORT() START !");
	}
	
	if (szTMS_PORT == NULL)
	{
		inRetVal = _SDK_ERROR_CODE_POINTER_NULL_ERROR_;
		
		return (inRetVal);
	}
	
	if (inLoadTMSCPTRec(0) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_LOAD_TMSCPT_ERROR_;
		
		return (inRetVal);
	}
	
	if (inGetTMSPortNum(szTMS_PORT) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_GET_TMSCPT_TAG_ERROR_;
		
		return (inRetVal);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "OPT_GET_TMS_PORT() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :OPT_SET_TMS_PORT
Date&Time       :2018/9/20 上午 10:43
Describe        :設定TMS PORT
*/
int OPT_SET_TMS_PORT(char* szTMS_PORT)
{
	int	inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "OPT_SET_TMS_PORT() START !");
	}
	
	if (strlen(szTMS_PORT) <= 0 ||	strlen(szTMS_PORT) > 5)
	{
		inRetVal = _SDK_ERROR_CODE_SET_LENGTH_ERROR_;
		
		return (inRetVal);
	}
	
	if (inLoadTMSCPTRec(0) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_LOAD_TMSCPT_ERROR_;
		
		return (inRetVal);
	}
	
	if (inSetTMSPortNum(szTMS_PORT) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_SET_TMSCPT_TAG_ERROR_;
		
		return (inRetVal);
	}
	
	if (inSaveTMSCPTRec(0) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_SAVE_TMSCPT_ERROR_;
		
		return (inRetVal);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "OPT_SET_TMS_PORT() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :OPT_GET_FTP_IP
Date&Time       :2018/10/2 上午 10:04
Describe        :取得FTP IP
*/
int OPT_GET_FTP_IP(char* szFTP_IP)
{
	int	inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "OPT_GET_FTP_IP() START !");
	}
	
	if (szFTP_IP == NULL)
	{
		inRetVal = _SDK_ERROR_CODE_POINTER_NULL_ERROR_;
		
		return (inRetVal);
	}
	
	if (inLoadTMSFTPRec(0) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_LOAD_TMSFTP_ERROR_;
		
		return (inRetVal);
	}
	
	if (inGetFTPIPAddress(szFTP_IP) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_GET_TMSFTP_TAG_ERROR_;
		
		return (inRetVal);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "OPT_GET_FTP_IP() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :OPT_SET_FTP_IP
Date&Time       :2018/10/2 上午 10:13
Describe        :設定FTP IP
*/
int OPT_SET_FTP_IP(char* szFTP_IP)
{
	int	inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "OPT_SET_FTP_IP() START !");
	}
	
	if (strlen(szFTP_IP) <= 0 ||	strlen(szFTP_IP) > 15)
	{
		inRetVal = _SDK_ERROR_CODE_SET_LENGTH_ERROR_;
		
		return (inRetVal);
	}
	
	if (inLoadTMSFTPRec(0) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_LOAD_TMSFTP_ERROR_;
		
		return (inRetVal);
	}
	
	if (inSetFTPIPAddress(szFTP_IP) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_SET_TMSFTP_TAG_ERROR_;
		
		return (inRetVal);
	}
	
	if (inSaveTMSFTPRec(0) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_SAVE_TMSFTP_ERROR_;
		
		return (inRetVal);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "OPT_SET_FTP_IP() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :OPT_GET_FTP_PORT
Date&Time       :2018/10/2 上午 10:17
Describe        :取得FTP PORT
*/
int OPT_GET_FTP_PORT(char* szFTP_PORT)
{
	int	inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "OPT_GET_FTP_PORT() START !");
	}
	
	if (szFTP_PORT == NULL)
	{
		inRetVal = _SDK_ERROR_CODE_POINTER_NULL_ERROR_;
		
		return (inRetVal);
	}
	
	if (inLoadTMSFTPRec(0) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_LOAD_TMSFTP_ERROR_;
		
		return (inRetVal);
	}
	
	if (inGetFTPPortNum(szFTP_PORT) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_GET_TMSFTP_TAG_ERROR_;
		
		return (inRetVal);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "OPT_GET_FTP_PORT() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :OPT_SET_FTP_PORT
Date&Time       :2018/10/2 上午 10:20
Describe        :設定FTP PORT
*/
int OPT_SET_FTP_PORT(char* szFTP_PORT)
{
	int	inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "OPT_SET_FTP_PORT() START !");
	}
	
	if (strlen(szFTP_PORT) <= 0 ||	strlen(szFTP_PORT) > 5)
	{
		inRetVal = _SDK_ERROR_CODE_SET_LENGTH_ERROR_;
		
		return (inRetVal);
	}
	
	if (inLoadTMSFTPRec(0) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_LOAD_TMSFTP_ERROR_;
		
		return (inRetVal);
	}
	
	if (inSetFTPPortNum(szFTP_PORT) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_SET_TMSFTP_TAG_ERROR_;
		
		return (inRetVal);
	}
	
	if (inSaveTMSFTPRec(0) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_SAVE_TMSFTP_ERROR_;
		
		return (inRetVal);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "OPT_SET_FTP_PORT() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :OPT_GET_HOST_IP
Date&Time       :2018/9/25 上午 9:58
Describe        :取得HOST IP
*/
int OPT_GET_HOST_IP(char* szHOST_IP)
{
	int	inHDTIndex = 0;
	int	inCPTIndex = 0;
	int	inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	char	szCPTIndex[2 + 1] = {0};
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "OPT_GET_HOST_IP() START !");
	}
	
	if (szHOST_IP == NULL)
	{
		inRetVal = _SDK_ERROR_CODE_POINTER_NULL_ERROR_;
		
		return (inRetVal);
	}
	
	inFunc_Find_Specific_HDTindex(0, _HOST_NAME_CREDIT_NCCC_, &inHDTIndex);
	if (inLoadHDTRec(inHDTIndex) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_LOAD_HDT_ERROR_;
		
		return (inRetVal);
	}
	
	memset(szCPTIndex, 0x00, sizeof(szCPTIndex));
	if (inGetCommunicationIndex(szCPTIndex) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_GET_HDT_TAG_ERROR_;
		
		return (inRetVal);
	}
	
	inCPTIndex = atoi(szCPTIndex) - 1;
	if (inLoadCPTRec(inCPTIndex) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_LOAD_CPT_ERROR_;
		
		return (inRetVal);
	}
	
	if (inGetHostIPPrimary(szHOST_IP) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_GET_CPT_TAG_ERROR_;
		
		return (inRetVal);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "OPT_GET_HOST_IP() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :OPT_SET_HOST_IP
Date&Time       :2018/9/25 上午 10:24
Describe        :設定HOST IP
*/
int OPT_SET_HOST_IP(char* szHOST_IP)
{
	int	inHDTIndex = 0;
	int	inCPTIndex = 0;
	int	inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	char	szCPTIndex[2 + 1] = {0};
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "OPT_SET_HOST_IP() START !");
	}
	
	if (strlen(szHOST_IP) <= 0 ||	strlen(szHOST_IP) > 15)
	{
		inRetVal = _SDK_ERROR_CODE_SET_LENGTH_ERROR_;
		
		return (inRetVal);
	}
	
	inFunc_Find_Specific_HDTindex(0, _HOST_NAME_CREDIT_NCCC_, &inHDTIndex);
	if (inLoadHDTRec(inHDTIndex) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_LOAD_HDT_ERROR_;
		
		return (inRetVal);
	}
	
	memset(szCPTIndex, 0x00, sizeof(szCPTIndex));
	if (inGetCommunicationIndex(szCPTIndex) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_GET_HDT_TAG_ERROR_;
		
		return (inRetVal);
	}
	
	inCPTIndex = atoi(szCPTIndex) - 1;
	if (inLoadCPTRec(inCPTIndex) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_LOAD_CPT_ERROR_;
		
		return (inRetVal);
	}
	
	if (inSetHostIPPrimary(szHOST_IP) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_SET_CPT_TAG_ERROR_;
		
		return (inRetVal);
	}
	
	if (inSaveCPTRec(inHDTIndex) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_SAVE_CPT_ERROR_;
		
		return (inRetVal);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "OPT_SET_HOST_IP() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :OPT_GET_HOST_PORT
Date&Time       :2018/9/25 上午 10:53
Describe        :取得HOST PORT
*/
int OPT_GET_HOST_PORT(char* szHOST_PORT)
{
	int	inHDTIndex = 0;
	int	inCPTIndex = 0;
	int	inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	char	szCPTIndex[2 + 1] = {0};
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "OPT_GET_HOST_PORT() START !");
	}
	
	if (szHOST_PORT == NULL)
	{
		inRetVal = _SDK_ERROR_CODE_POINTER_NULL_ERROR_;
		
		return (inRetVal);
	}
	
	inFunc_Find_Specific_HDTindex(0, _HOST_NAME_CREDIT_NCCC_, &inHDTIndex);
	if (inLoadHDTRec(inHDTIndex) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_LOAD_HDT_ERROR_;
		
		return (inRetVal);
	}
	
	memset(szCPTIndex, 0x00, sizeof(szCPTIndex));
	if (inGetCommunicationIndex(szCPTIndex) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_GET_HDT_TAG_ERROR_;
		
		return (inRetVal);
	}
	
	inCPTIndex = atoi(szCPTIndex) - 1;
	if (inLoadCPTRec(inCPTIndex) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_LOAD_CPT_ERROR_;
		
		return (inRetVal);
	}
	
	if (inGetHostPortNoPrimary(szHOST_PORT) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_SAVE_CPT_ERROR_;
		
		return (inRetVal);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "OPT_GET_HOST_PORT() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :OPT_SET_HOST_PORT
Date&Time       :2018/9/20 上午 10:43
Describe        :設定HOST PORT
*/
int OPT_SET_HOST_PORT(char* szHOST_PORT)
{
	int	inHDTIndex = 0;
	int	inCPTIndex = 0;
	int	inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	char	szCPTIndex[2 + 1] = {0};
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "OPT_SET_HOST_PORT() START !");
	}
	
	if (strlen(szHOST_PORT) <= 0 ||	strlen(szHOST_PORT) > 5)
	{
		inRetVal = _SDK_ERROR_CODE_SET_LENGTH_ERROR_;
		
		return (inRetVal);
	}
	
	inFunc_Find_Specific_HDTindex(0, _HOST_NAME_CREDIT_NCCC_, &inHDTIndex);
	if (inLoadHDTRec(inHDTIndex) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_LOAD_HDT_ERROR_;
		
		return (inRetVal);
	}
	
	memset(szCPTIndex, 0x00, sizeof(szCPTIndex));
	if (inGetCommunicationIndex(szCPTIndex) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_GET_HDT_TAG_ERROR_;
		
		return (inRetVal);
	}
	
	inCPTIndex = atoi(szCPTIndex) - 1;
	if (inLoadCPTRec(inCPTIndex) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_LOAD_CPT_ERROR_;
		
		return (inRetVal);
	}
	
	if (inSetHostPortNoPrimary(szHOST_PORT) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_SAVE_CPT_ERROR_;
		
		return (inRetVal);
	}
	
	if (inSaveCPTRec(inHDTIndex) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_SAVE_CPT_ERROR_;
		
		return (inRetVal);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "OPT_SET_HOST_PORT() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :OPT_GET_EDC_IP
Date&Time       :2018/9/20 下午 5:20
Describe        :取得EDC IP
*/
int OPT_GET_EDC_IP(char* szEDC_IP)
{
	int	inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "OPT_GET_EDC_IP() START !");
	}
	
	if (szEDC_IP == NULL)
	{
		inRetVal = _SDK_ERROR_CODE_POINTER_NULL_ERROR_;
		
		return (inRetVal);
	}
	
	if (inLoadEDCRec(0) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_LOAD_EDC_ERROR_;
		
		return (inRetVal);
	}
	
	if (inGetTermIPAddress(szEDC_IP) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_GET_EDC_TAG_ERROR_;
		
		return (inRetVal);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "OPT_GET_EDC_IP() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :OPT_SET_EDC_IP
Date&Time       :2018/9/19 下午 5:53
Describe        :設定EDC IP
*/
int OPT_SET_EDC_IP(char* szEDC_IP)
{
	int	inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "OPT_SET_EDC_IP() START !");
	}
	
	if (strlen(szEDC_IP) <= 0 ||	strlen(szEDC_IP) > 15)
	{
		inRetVal = _SDK_ERROR_CODE_SET_LENGTH_ERROR_;
		
		return (inRetVal);
	}
	
	if (inLoadEDCRec(0) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_LOAD_EDC_ERROR_;
		
		return (inRetVal);
	}
	
	if (inSetTermIPAddress(szEDC_IP) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_SET_EDC_TAG_ERROR_;
		
		return (inRetVal);
	}
	
	if (inSaveEDCRec(0) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_SAVE_EDC_ERROR_;
		
		return (inRetVal);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "OPT_SET_EDC_IP() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :OPT_GET_SUBMASK
Date&Time       :2018/9/20 下午 5:57
Describe        :取得SUBMASK
*/
int OPT_GET_SUBMASK(char* szSUBMASK)
{
	int	inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "OPT_GET_SUBMASK() START !");
	}
	
	if (szSUBMASK == NULL)
	{
		inRetVal = _SDK_ERROR_CODE_POINTER_NULL_ERROR_;
		
		return (inRetVal);
	}
	
	if (inLoadEDCRec(0) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_LOAD_EDC_ERROR_;
		
		return (inRetVal);
	}
	
	if (inGetTermMASKAddress(szSUBMASK) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_GET_EDC_TAG_ERROR_;
		
		return (inRetVal);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "OPT_GET_SUBMASK() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :OPT_SET_SUBMASK
Date&Time       :2018/9/20 下午 5:59
Describe        :設定SUBMASK
*/
int OPT_SET_SUBMASK(char* szSUBMASK)
{
	int	inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "OPT_SET_SUBMASK() START !");
	}
	
	if (strlen(szSUBMASK) <= 0 ||	strlen(szSUBMASK) > 15)
	{
		inRetVal = _SDK_ERROR_CODE_SET_LENGTH_ERROR_;
		
		return (inRetVal);
	}
	
	if (inLoadEDCRec(0) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_LOAD_EDC_ERROR_;
		
		return (inRetVal);
	}
	
	if (inSetTermMASKAddress(szSUBMASK) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_SET_EDC_TAG_ERROR_;
		
		return (inRetVal);
	}
	
	if (inSaveEDCRec(0) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_SAVE_EDC_ERROR_;
		
		return (inRetVal);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "OPT_SET_SUBMASK() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :OPT_GET_GATEWAY
Date&Time       :2018/9/20 下午 6:00
Describe        :取得GATEWAY
*/
int OPT_GET_GATEWAY(char* szGATEWAY)
{
	int	inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "OPT_GET_GATEWAY() START !");
	}
	
	if (szGATEWAY == NULL)
	{
		inRetVal = _SDK_ERROR_CODE_POINTER_NULL_ERROR_;
		
		return (inRetVal);
	}
	
	if (inLoadEDCRec(0) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_LOAD_EDC_ERROR_;
		
		return (inRetVal);
	}
	
	if (inGetTermGetewayAddress(szGATEWAY) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_GET_EDC_TAG_ERROR_;
		
		return (inRetVal);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "OPT_GET_GATEWAY() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :OPT_SET_GATEWAY
Date&Time       :2018/9/20 下午 5:59
Describe        :設定GATEWAY
*/
int OPT_SET_GATEWAY(char* szGATEWAY)
{
	int	inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "OPT_SET_GATEWAY() START !");
	}
	
	if (strlen(szGATEWAY) <= 0 ||	strlen(szGATEWAY) > 15)
	{
		inRetVal = _SDK_ERROR_CODE_SET_LENGTH_ERROR_;
		
		return (inRetVal);
	}
	
	if (inLoadEDCRec(0) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_LOAD_EDC_ERROR_;
		
		return (inRetVal);
	}
	
	if (inSetTermGetewayAddress(szGATEWAY) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_SET_EDC_TAG_ERROR_;
		
		return (inRetVal);
	}
	
	if (inSaveEDCRec(0) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_SAVE_EDC_ERROR_;
		
		return (inRetVal);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "OPT_SET_GATEWAY() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :OPT_GET_TID
Date&Time       :2018/9/20 下午 4:10
Describe        :取得TID，因為只有VMJ，所以先只用第一個
*/
int OPT_GET_TID(char* szTID)
{
	int	inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "OPT_GET_TID() START !");
	}
	
	if (szTID == NULL)
	{
		inRetVal = _SDK_ERROR_CODE_POINTER_NULL_ERROR_;
		
		return (inRetVal);
	}
	
	if (inLoadHDTRec(0) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_LOAD_HDT_ERROR_;
		
		return (inRetVal);
	}
	
	if (inGetTerminalID(szTID) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_GET_HDT_TAG_ERROR_;
		
		return (inRetVal);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "OPT_GET_TID() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :OPT_SET_TID
Date&Time       :2018/9/20 下午 4:13
Describe        :設定TID
*/
int OPT_SET_TID(char* szTID)
{
	int	inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "OPT_SET_TID() START !");
	}
	
	if (strlen(szTID) <= 0 ||	strlen(szTID) > 8)
	{
		inRetVal = _SDK_ERROR_CODE_SET_LENGTH_ERROR_;
		
		return (inRetVal);
	}
	
	if (inLoadHDTRec(0) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_LOAD_HDT_ERROR_;
		
		return (inRetVal);
	}
	
	if (inSetTerminalID(szTID) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_SET_HDT_TAG_ERROR_;
		
		return (inRetVal);
	}
	
	if (inSaveHDTRec(0) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_SAVE_HDT_ERROR_;
		
		return (inRetVal);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "OPT_SET_TID() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :OPT_GET_MID
Date&Time       :2018/9/20 下午 4:10
Describe        :取得MID
*/
int OPT_GET_MID(char* szMID)
{
	int	inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "OPT_GET_MID() START !");
	}
	
	if (szMID == NULL)
	{
		inRetVal = _SDK_ERROR_CODE_POINTER_NULL_ERROR_;
		
		return (inRetVal);
	}
	
	if (inLoadHDTRec(0) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_LOAD_HDT_ERROR_;
		
		return (inRetVal);
	}
	
	if (inGetMerchantID(szMID) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_GET_HDT_TAG_ERROR_;
		
		return (inRetVal);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "OPT_GET_MID() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :OPT_SET_MID
Date&Time       :2018/9/20 下午 4:13
Describe        :設定MID
*/
int OPT_SET_MID(char* szMID)
{
	int	inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "OPT_SET_MID() START !");
	}
	
	if (strlen(szMID) <= 0 ||	strlen(szMID) > 15)
	{
		inRetVal = _SDK_ERROR_CODE_SET_LENGTH_ERROR_;
		
		return (inRetVal);
	}
	
	if (inLoadHDTRec(0) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_LOAD_HDT_ERROR_;
		
		return (inRetVal);
	}
	
	if (inSetMerchantID(szMID) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_SET_HDT_TAG_ERROR_;
		
		return (inRetVal);
	}
	
	if (inSaveHDTRec(0) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_SAVE_HDT_ERROR_;
		
		return (inRetVal);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "OPT_SET_MID() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :OPT_GET_BATCH_NUMBER
Date&Time       :2018/9/27 下午 2:56
Describe        :
*/
int OPT_GET_BATCH_NUMBER(char *szBatchNum)
{
	int	inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	
	if (inLoadHDPTRec(0) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_LOAD_HDPT_ERROR_;
		
		return (inRetVal);
	}
	
	if (inGetBatchNum(szBatchNum) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_GET_HDPT_TAG_ERROR_;
		
		return (inRetVal);
	}
	
	
	return (inRetVal);
}

/*
Function        :OPT_GET_TMS_OK
Date&Time       :2018/10/2 下午 2:01
Describe        :
*/
int OPT_GET_TMS_OK(char *szTMSOK)
{
	int	inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	
	if (inLoadEDCRec(0) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_LOAD_EDC_ERROR_;
		
		return (inRetVal);
	}
	
	if (inGetTMSOK(szTMSOK) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "TMS未下載");
		}
		inRetVal = _SDK_ERROR_CODE_GET_EDC_TAG_ERROR_;
		
		return (inRetVal);
	}
	
	
	return (inRetVal);
}

/*
Function        :OPT_SET_COMM_MODE_ETHERNET
Date&Time       :2018/10/2 上午 9:35
Describe        :
*/
int OPT_SET_COMM_MODE_ETHERNET(void)
{
	int	inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "OPT_SET_COMM_MODE_ETHERNET() START !");
	}
	
	inLoadCFGTRec(0);
	inSetCommMode(_COMM_ETHERNET_MODE_);
	inSetI_FES_Mode("N");
	inSaveCFGTRec(0);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "OPT_SET_COMM_MODE_ETHERNET() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :OPT_SET_COMM_MODE_ETHERNET_IFES
Date&Time       :2018/10/2 上午 9:35
Describe        :
*/
int OPT_SET_COMM_MODE_ETHERNET_IFES(void)
{
	int	inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "OPT_SET_COMM_MODE_ETHERNET_IFES() START !");
	}
	
	inLoadCFGTRec(0);
	inSetCommMode(_COMM_ETHERNET_MODE_);
	inSetI_FES_Mode("Y");
	inSaveCFGTRec(0);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "OPT_SET_COMM_MODE_ETHERNET_IFES() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :OPT_DEBUG_ON
Date&Time       :2018/9/25 上午 11:10
Describe        :開啟Debug Log模式
*/
int OPT_DEBUG_ON(void)
{
	int	inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	char	szISODebug[10 + 1] = {0};
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "OPT_DEBUG_ON() START !");
	}
	
	if (inLoadEDCRec(0) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_LOAD_EDC_ERROR_;
		
		return (inRetVal);
	}
	
	if (inGetISODebug(szISODebug) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_GET_EDC_TAG_ERROR_;
		
		return (inRetVal);
	}
	
	szISODebug[0] = '1';
	ginDebug = VS_TRUE;
	
	if (inSetISODebug(szISODebug) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_SET_EDC_TAG_ERROR_;
		
		return (inRetVal);
	}
	
	if (inSaveEDCRec(0) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_SAVE_EDC_ERROR_;
		
		return (inRetVal);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "OPT_DEBUG_ON() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :OPT_DEBUG_OFF
Date&Time       :2018/9/25 上午 11:10
Describe        :關閉Debug Log
*/
int OPT_DEBUG_OFF(void)
{
	int	inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	char	szISODebug[10 + 1] = {0};
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "OPT_DEBUG_OFF() START !");
	}
	
	if (inLoadEDCRec(0) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_LOAD_EDC_ERROR_;
		
		return (inRetVal);
	}
	
	if (inGetISODebug(szISODebug) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_GET_EDC_TAG_ERROR_;
		
		return (inRetVal);
	}
	
	szISODebug[0] = '0';
	ginDebug = VS_FALSE;
	
	if (inSetISODebug(szISODebug) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_SET_EDC_TAG_ERROR_;
		
		return (inRetVal);
	}
	
	if (inSaveEDCRec(0) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_SAVE_EDC_ERROR_;
		
		return (inRetVal);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "OPT_DEBUG_OFF() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :OPT_DEMO_MODE_ON
Date&Time       :2018/9/25 上午 11:10
Describe        :開啟DEMO MODE模式
*/
int OPT_DEMO_MODE_ON(void)
{
	int	inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	char	szDEMOMode[2 + 1] = {0};
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "OPT_DEMO_MODE_ON() START !");
	}
	
	if (inLoadEDCRec(0) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_LOAD_EDC_ERROR_;
		
		return (inRetVal);
	}
	
	if (inGetDemoMode(szDEMOMode) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_GET_EDC_TAG_ERROR_;
		
		return (inRetVal);
	}
	
	szDEMOMode[0] = 'Y';
	
	if (inSetDemoMode(szDEMOMode) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_SET_EDC_TAG_ERROR_;
		
		return (inRetVal);
	}
	
	if (inSaveEDCRec(0) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_SAVE_EDC_ERROR_;
		
		return (inRetVal);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "OPT_DEMO_MODE_ON() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :OPT_DEMO_MODE_OFF
Date&Time       :2018/9/25 上午 11:10
Describe        :關閉DEMO模式
*/
int OPT_DEMO_MODE_OFF(void)
{
	int	inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	char	szDEMOMode[2 + 1] = {0};
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "OPT_DEMO_MODE_OFF() START !");
	}
	
	if (inLoadEDCRec(0) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_LOAD_EDC_ERROR_;
		
		return (inRetVal);
	}
	
	if (inGetDemoMode(szDEMOMode) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_GET_EDC_TAG_ERROR_;
		
		return (inRetVal);
	}
	
	szDEMOMode[0] = 'N';
	
	if (inSetDemoMode(szDEMOMode) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_SET_EDC_TAG_ERROR_;
		
		return (inRetVal);
	}
	
	if (inSaveEDCRec(0) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_SAVE_EDC_ERROR_;
		
		return (inRetVal);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "OPT_DEMO_MODE_OFF() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :OPT_READ_DETAIL_RECORD_START
Date&Time       :2018/9/26 下午 1:51
Describe        :
*/
int OPT_READ_DETAIL_RECORD_START(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	int	inFuncRetVal = VS_SUCCESS;
	int	inFileRecCnt = 0;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "OPT_READ_DETAIL_RECORD_START() START !");
	}
	
	/* 尚未執行OPT_TerminalInitial，無法執行SDK內容 */
	if (guszOPT_InitialBit != VS_TRUE)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "未進行SDK初始化，不允許執行");
		}
		
		inRetVal = _SDK_ERROR_CODE_OPT_INITIAL_NOT_YET_;
		return (inRetVal);
	}
	
	pobTran->srBRec.inHDTIndex = 0;
	inFileRecCnt = inBATCH_GetTotalCountFromBakFile_By_Sqlite(pobTran);
	if (inFileRecCnt == 0)
	{
		inRetVal = _SDK_ERROR_CODE_NO_TXN_RECORD_ERROR_;
		return (inRetVal);
	}
	
	/* 先抓出總資料 */
	inFuncRetVal = inBATCH_GetDetailRecords_By_Sqlite_Enormous_START(pobTran);
	if (inFuncRetVal != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_ERROR_;
		return (inRetVal);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "OPT_READ_DETAIL_RECORD_START() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :OPT_READ_DETAIL_RECORD_READ
Date&Time       :2018/9/26 下午 2:07
Describe        :
*/
int OPT_READ_DETAIL_RECORD_READ(TRANSACTION_OBJECT *pobTran, int inCnt)
{
	int	inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	int	inFuncRetVal = VS_SUCCESS;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "OPT_READ_DETAIL_RECORD_READ() START !");
	}
	
	/* 尚未執行OPT_TerminalInitial，無法執行SDK內容 */
	if (guszOPT_InitialBit != VS_TRUE)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "未進行SDK初始化，不允許執行");
		}
		
		inRetVal = _SDK_ERROR_CODE_OPT_INITIAL_NOT_YET_;
		return (inRetVal);
	}
	
	inFuncRetVal = inBATCH_GetDetailRecords_By_Sqlite_Enormous_Read(pobTran, inCnt);
	if (inFuncRetVal != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_NO_TXN_RECORD_ERROR_;
		return (inRetVal);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "OPT_READ_DETAIL_RECORD_READ() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :OPT_READ_DETAIL_RECORD_END
Date&Time       :2018/9/26 下午 2:16
Describe        :
*/
int OPT_READ_DETAIL_RECORD_END(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	int	inFuncRetVal = VS_SUCCESS;
	
	/* 尚未執行OPT_TerminalInitial，無法執行SDK內容 */
	if (guszOPT_InitialBit != VS_TRUE)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "未進行SDK初始化，不允許執行");
		}
		
		inRetVal = _SDK_ERROR_CODE_OPT_INITIAL_NOT_YET_;
		return (inRetVal);
	}
	
	inFuncRetVal = inBATCH_GetDetailRecords_By_Sqlite_Enormous_END(pobTran);
	if (inFuncRetVal != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_ERROR_;
		return (inRetVal);
	}
	
	return (inRetVal);
}

/*
Function        :OPT_GET_BATCH_RECORD_COUNT
Date&Time       :2018/10/3 下午 4:14
Describe        :
*/
int OPT_GET_BATCH_RECORD_COUNT(TRANSACTION_OBJECT *pobTran, int *inCnt)
{
	int	inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	
	pobTran->srBRec.inHDTIndex = 0;
	*inCnt = inBATCH_GetTotalCountFromBakFile_By_Sqlite(pobTran);
	
	return (inRetVal);
}

/*
Function        :OPT_GET_ACCUM_RECORD
Date&Time       :2018/9/27 下午 5:14
Describe        :
*/
int OPT_GET_ACCUM_RECORD(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec)
{
	int	inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	int	inFuncRetVal = VS_SUCCESS;
	
	/* 尚未執行OPT_TerminalInitial，無法執行SDK內容 */
	if (guszOPT_InitialBit != VS_TRUE)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "未進行SDK初始化，不允許執行");
		}
		
		inRetVal = _SDK_ERROR_CODE_OPT_INITIAL_NOT_YET_;
		return (inRetVal);
	}
	
	pobTran->srBRec.inHDTIndex = 0;
	inFuncRetVal = inACCUM_GetRecord(pobTran, srAccumRec);
	if (inFuncRetVal == VS_SUCCESS)
	{
		
	}
	else if (inFuncRetVal == VS_NO_RECORD)
	{
		inRetVal = _SDK_ERROR_CODE_NO_TXN_RECORD_ERROR_;
		return (inRetVal);
	}
	else
	{
		inRetVal = _SDK_ERROR_CODE_ERROR_;
		return (inRetVal);
	}
	
	return (inRetVal);
}

/*
Function        :OPT_PAN_TRANSFORM_TO_CHECK_NO
Date&Time       :2018/10/3 下午 4:50
Describe        :
*/
int OPT_EXPIRE_DATE_TRANSFORM_TO_CHECK_NO(TRANSACTION_OBJECT *pobTran, char *szOutput)
{
	int	inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	int	inFuncRetVal = VS_SUCCESS;
	
	/* 卡號長度不足，第十碼要當key */
	if (strlen(pobTran->srBRec.szPAN) < 10)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "卡號長度不足,Len: %d", strlen(pobTran->srBRec.szPAN));
		}
		
		inRetVal = _SDK_ERROR_CODE_CARD_PAN_LEN_ERROR_;
		return (inRetVal);
	}
	
	/* 有效其一定要填4位 */
	if (strlen(pobTran->srBRec.szExpDate) != 4)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "有效期長度錯誤,Len: %d", strlen(pobTran->srBRec.szExpDate));
		}
		
		inRetVal = _SDK_ERROR_CODE_CARD_EXP_ERROR_;
		return (inRetVal);
	}
	
	
	inFuncRetVal = inCARD_ExpDateEncryptAndDecrypt(pobTran, "", szOutput, _EXP_ENCRYPT_);
	if (inFuncRetVal == VS_SUCCESS)
	{
		
	}
	else
	{
		inRetVal = _SDK_ERROR_CODE_ERROR_;
		return (inRetVal);
	}
	
	return (inRetVal);
}

/*
Function        :OPT_PAN_TRANSFORM_TO_HASH_NO
Date&Time       :2018/10/8 下午 1:42
Describe        :
*/
int OPT_PAN_TRANSFORM_TO_HASH_NO(TRANSACTION_OBJECT *pobTran, char *szOutput)
{
	int	inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	int	inFuncRetVal = VS_SUCCESS;
	int	inOffset = 0;
	char	szTemplate[50 + 1] = {0};
	char	szHash[50 + 1] = {0};
	
	/* 卡號長度不足 */
	if (strlen(pobTran->srBRec.szPAN) <= 0)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "卡號長度不足,Len: %d", strlen(pobTran->srBRec.szPAN));
		}
		
		inRetVal = _SDK_ERROR_CODE_CARD_PAN_LEN_ERROR_;
		return (inRetVal);
	}
	
	/* Card No. Hash Value (50 Bytes) */
	/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(&szTemplate[0], &pobTran->srBRec.szPAN[0], strlen(pobTran->srBRec.szPAN));

	memset(szHash, 0x00, sizeof(szHash));
	inFuncRetVal = inNCCC_Func_CardNumber_Hash(szTemplate, szHash);
	if (inFuncRetVal == VS_SUCCESS)
	{
		memcpy(&szOutput[inOffset], &szTemplate[0], 6);
		inOffset += 6;
		memcpy(&szOutput[inOffset], &szHash[0], 44);
		inOffset += 44;
	}
	else
	{
		inRetVal = _SDK_ERROR_CODE_ERROR_;
		return (inRetVal);
	}
	
	return (inRetVal);
}

/*
Function        :OPT_CARD_EXIST
Date&Time       :2018/10/5 下午 12:01
Describe        :
*/
int OPT_CARD_EXIST(char* szExist)
{
	int	inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	int	inFuncRetVal = VS_SUCCESS;
	
	inFuncRetVal = inEMV_ICCEvent();
	if (inFuncRetVal == VS_SUCCESS)
	{
		sprintf(szExist, "Y");
	}
	else
	{
		sprintf(szExist, "N");
	}
	
	return (inRetVal);
}

/*
Function        :OPT_SET_ENCRYPTMODE_NONE_FOR_TEST
Date&Time       :2018/10/5 下午 1:18
Describe        :關閉加密模式
*/
int OPT_SET_ENCRYPTMODE_NONE_FOR_TEST(void)
{
	int	inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	
	if (inLoadCFGTRec(0) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_LOAD_CFGT_ERROR_;
		
		return (inRetVal);
	}
	
	if (inSetEncryptMode(_NCCC_ENCRYPTION_NONE_) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_SET_CFGT_TAG_ERROR_;
		
		return (inRetVal);
	}
	
	if (inSaveCFGTRec(0) != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_SAVE_CFGT_ERROR_;
		
		return (inRetVal);
	}
	
	return (inRetVal);
}

/*
Function        :OPT_DEL_ALL_TRANS
Date&Time       :2018/10/5 下午 2:15
Describe        :
*/
int OPT_DEL_ALL_TRANS(void)
{
	int			inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	int			inFuncRetVal = VS_SUCCESS;
	TRANSACTION_OBJECT	pobTran;
	
	memset(&pobTran, 0x00, sizeof(pobTran));
	pobTran.srBRec.inHDTIndex = 0;
	inLoadHDTRec(pobTran.srBRec.inHDTIndex);
	inFuncRetVal = inFLOW_RunFunction(&pobTran, _FUNCTION_DELETE_BATCH_FLOW_);
	if (inRetVal != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_ERROR_;
		return (inRetVal);
	}
	
	inFuncRetVal = inFLOW_RunFunction(&pobTran, _FUNCTION_DELETE_ACCUM_FLOW_);
	if (inRetVal != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_ERROR_;
		return (inRetVal);
	}
	
	inFuncRetVal = inFLOW_RunFunction(&pobTran, _FUNCTION_REST_BATCH_INV_);
	if (inRetVal != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_ERROR_;
		return (inRetVal);
	}
	
	inFuncRetVal = inFLOW_RunFunction(&pobTran, _FUNCTION_UPDATE_BATCH_NUM_);
	if (inRetVal != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_ERROR_;
		return (inRetVal);
	}
	
	return (inRetVal);
}

/*
Function        :
Date&Time       :
Describe        :
*/
int OPT_CHECK_TMS_FILE(int inCnt, char *szFileName, char *szResult)
{
	int	j = 0;
	int	inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	int	inFileNameLen = 0;
	int	inSlash = 0;
	char	szTemplate[60 + 1] = {0};
	char	szTemp[60 + 1] = {0};
	
	if (inLoadTMSFLTRec(inCnt) < 0)
	{
		inRetVal = _SDK_ERROR_CODE_ERROR_;
		return (inRetVal);
	}
	
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTMSFilePathName(szTemplate);

	/* 取得檔案路徑後 抓取檔名儲存到FileName */
	for (j = 0; j < 60 ; j++)
	{
		if (szTemplate[j] == '/')
		    inSlash = j + 1;

		if (szTemplate[j] == 0x00)
		    break;
	}

	memset(szTemp, 0x00, sizeof(szTemp));
	inFileNameLen = j - inSlash;
	memcpy(szTemp, &szTemplate[inSlash], inFileNameLen);
	sprintf(szFileName, szTemp);
	
	/* 結果 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTMSFileIndex(szTemplate);
	sprintf(szResult, szTemplate);
	
	
	return (inRetVal);
}

/*
Function        :OPT_GET_MIFARE_CARD_NO
Date&Time       :2018/10/22 下午 2:38
Describe        :
*/
int OPT_GET_MIFARE_CARD_NO(char *szCardNO)
{
	int	inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	int	inFuncVal = VS_SUCCESS;
	char	szTemplate[32 + 1] = {0};
	char	szTemplate2[32 + 1] = {0};
	
	if (szCardNO == NULL)
	{
		inRetVal = _SDK_ERROR_CODE_POINTER_NULL_ERROR_;
		return (inRetVal);
	}
	
//	memset(szTemplate, 0x00, sizeof(szTemplate));
//	inCTLS_Mifare_Read_Block(0, (unsigned char*)szTemplate);
//	memset(szAscii, 0x00, sizeof(szAscii));
//	inFunc_BCD_to_ASCII(szAscii, (unsigned char*)szTemplate, 16);
//	if (ginDebug == VS_TRUE)
//	{
//		inLogPrintf(AT, "mifare: %s", szAscii);
//	}
//	memset(szCardNO, 0x00, 32);
//	memcpy(szCardNO, szAscii, 32);
	
	inDISP_Timer_Start(_TIMER_NEXSYS_4_, 5);
	while (1)
	{
		memset(szTemplate2, 0x00, sizeof(szTemplate2));
		inFuncVal = inCTLS_Get_TypeACardSN(szTemplate2);
		if (inFuncVal == VS_SUCCESS)
		{
			break;
		}
		else
		{
			if (inTimerGet(_TIMER_NEXSYS_4_) == VS_SUCCESS)
			{
				inRetVal = _SDK_ERROR_CODE_TIMEOUT_;
				return (inRetVal);
			}
			else
			{
				inRetVal = _SDK_ERROR_CODE_ERROR_;
				return (inRetVal);
			}
		}
	}
	
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(&szTemplate[0], &szTemplate2[6], 2);
	memcpy(&szTemplate[2], &szTemplate2[4], 2);
	memcpy(&szTemplate[4], &szTemplate2[2], 2);
	memcpy(&szTemplate[6], &szTemplate2[0], 2);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "TypeA SN:%s", szTemplate);
	}
	
	inFuncVal = inFunc_String_Hex_to_Dec_Big2Little(szTemplate, szCardNO, 14);
	if (inFuncVal != VS_SUCCESS)
	{
		inRetVal = _SDK_ERROR_CODE_ERROR_;
		return (inRetVal);
	}
	
	return (inRetVal);
}

/*
Function        :OPT_POS_PRINTER_DEBUG_ON
Date&Time       :2018/10/24 下午 3:12
Describe        :
*/
int OPT_POS_PRINTER_DEBUG_ON(void)
{
	int	inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "OPT_POS_PRINTER_DEBUG_ON() START !");
	}
	
	ginPOSPrinterDebug = VS_TRUE;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "OPT_POS_PRINTER_DEBUG_ON() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :OPT_POS_PRINTER_DEBUG_OFF
Date&Time       :2018/10/24 下午 3:12
Describe        :
*/
int OPT_POS_PRINTER_DEBUG_OFF(void)
{
	int	inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "OPT_POS_PRINTER_DEBUG_OFF() START !");
	}
	ginPOSPrinterDebug = VS_FALSE;
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "OPT_POS_PRINTER_DEBUG_OFF() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :OPT_LOAD_FILE
Date&Time       :2018/12/12 下午 6:10
Describe        :
*/
int OPT_LOAD_FILE(void)
{
	int		inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	EventMenuItem	srEventMenuItem;
	
	memset(&srEventMenuItem, 0x00, sizeof(srEventMenuItem));
	
	inMENU_CHECK_FILE(&srEventMenuItem);
	
	return (inRetVal);
}

/*
Function        :OPT_SET_CASTLE_EMVCL_DEBUG
Date&Time       :2018/12/19 下午 5:10
Describe        :
*/
int OPT_SET_CASTLE_EMVCL_DEBUG(unsigned char uszSwitch, unsigned char uszPort)
{
	int		inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	unsigned char	uszDebugSwitch = FALSE;
	unsigned char	uszDebugPort = _SDK_COM_PORT_NONE_;
	
	if (uszSwitch == TRUE)
	{
		uszDebugSwitch = TRUE;
	}
	else
	{
		uszDebugSwitch = FALSE;
	}
	
	if (uszPort == _SDK_COM_PORT_USB_)
	{
		uszDebugPort = d_EMVCL_DEBUG_PORT_USB;
	}
	else if (uszPort == _SDK_COM_PORT_COM1_)
	{
		uszDebugPort = d_EMVCL_DEBUG_PORT_COM1;
	}
	else if (uszPort == _SDK_COM_PORT_COM2_)
	{
		uszDebugPort = d_EMVCL_DEBUG_PORT_COM2;
	}
	else if (uszPort == _SDK_COM_PORT_COM3_)
	{
		uszDebugPort = d_EMVCL_DEBUG_PORT_COM3;
	}
	else
	{
		uszDebugPort = -1;
		return (_SDK_ERROR_CODE_ERROR_);
	}
	
	EMVCL_SetDebug(uszDebugSwitch, uszDebugPort);
	
	return (inRetVal);
}

/*
Function        :OPT_EXAM_FLOW_ON
Date&Time       :2019/1/17 下午 3:28
Describe        :
*/
int OPT_EXAM_FLOW_ON(void)
{
	int	inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	
	ginExamBit = VS_TRUE;
	
	return (inRetVal);
}

/*
Function        :OPT_EXAM_FLOW_OFF
Date&Time       :2019/1/17 下午 3:28
Describe        :
*/
int OPT_EXAM_FLOW_OFF(void)
{
	int	inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	
	ginExamBit = VS_FALSE;
	
	return (inRetVal);
}

/*
Function        :inFunc_GetCardFields_CTLS
Date&Time       :2018/9/17 下午 2:10
Describe        :
*/
int inNexsysSDK_GetCardNO_CTLS(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_ERROR;
	int		inMSR_RetVal = -1;	/* 磁條事件的反應，怕和其他用到inRetVal的搞混所以獨立出來 */
	int		inEMV_RetVal = -1;	/* 晶片卡事件的反應，怕和其他用到inRetVal的搞混所以獨立出來*/
	int		inCTLS_RetVal = -1;	/* 感應卡事件的反應，怕和其他用到inRetVal的搞混所以獨立出來*/
        char		szKey = -1;
	char		szFuncEnable[2 + 1] = {0};
	char		szDebugMsg[100 + 1] = {0};
	char		szCustomerIndicator[3 + 1] = {0};
	long		lnTimeout = 0;
        unsigned long   ulCTLS_RetVal = -1;

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNexsySDK_GetCardNO_CTLS() START !");
	}
	
	/* 如果有卡號就直接跳走 */
	if (strlen(pobTran->srBRec.szPAN) > 0)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "已有卡號，不須執行流程");
		}
		return (VS_SUCCESS);
	}
	
        /* 過卡方式參數初始化  */
        pobTran->srBRec.uszManualBit = VS_FALSE;
	/* 初始化 ginEventCode */
        ginEventCode = -1;
		
	/* Send CTLS Readly for Sale Command */
	if (inCTLS_SendReadyForSale_Flow(pobTran) != VS_SUCCESS)
	{
		pobTran->inErrorMsg = _SDK_ERROR_CODE_CTLS_SEND_FAIL_;
		return (VS_ERROR);
	}
	
	/* 顯示對應交易別的感應畫面 */
	inDISP_PutGraphic(_SDK_CTLS_ICC_RF_, 0, _COORDINATE_Y_LINE_8_3_);			/* 請刷卡、插卡或感應銀聯卡 */
	
	/* 顯示虛擬LED(不閃爍)，但inCTLS_ReceiveReadyForSales(V3機型)也會顯示LED（閃爍），所以沒有實質作用 */
        EMVCL_StartIdleLEDBehavior(NULL);
      
        /* 不顯示金額 */
	
        /* 設定Timeout */
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)         ||
            !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_) ||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_))
	{
		lnTimeout = _CUSTOMER_107_BUMPER_GET_CARD_TIMEOUT_;
	}
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_005_FPG_, _CUSTOMER_INDICATOR_SIZE_))
	{
		lnTimeout = _CUSTOMER_005_FPG_GET_CARD_TIMEOUT_;
	}
	else
	{
		if (pobTran->uszECRBit == VS_TRUE)
		{
			lnTimeout = _ECR_RS232_GET_CARD_TIMEOUT_;
		}
		else
		{
			lnTimeout = 30;
		}
	}
	
	/* inSecond剩餘倒數時間 */
	inDISP_Timer_Start(_TIMER_NEXSYS_4_, lnTimeout);
	
	while (1)
        {
                /* （idle畫面刷卡此function不會發生） or 刷卡事件發生 */
	        if (ginIdleMSRStatus == VS_TRUE || ginEventCode == _SWIPE_EVENT_)
	        {
                        /* 取消感應交易 */
                        inCTLS_CancelTransacton_Flow();
                        inFunc_ResetTitle(pobTran);
			
	                /* 第一步驟GetTrack123 */
	                inCARD_GetTrack123(pobTran);

	                /* 第二步驟unPackTrack資料 */
	                if (inCARD_unPackCard(pobTran) != VS_SUCCESS)
	                {
				pobTran->inErrorMsg = _SDK_ERROR_CODE_CARD_UNPACK_FAIL_;
				/* Unpack失敗，再開感應太麻煩 */
	                        return(VS_ERROR);
	                }

                        /* 第三步驟 判斷card bin 讀HDT */
                        if (inCARD_GetBin(pobTran) != VS_SUCCESS)
			{
				pobTran->inErrorMsg = _SDK_ERROR_CODE_CARD_GET_BIN_;
                                return(VS_ERROR);
			}

                        /* 第四步驟檢核PAN module 10 */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetModule10Check(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
			{
				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
				if (inCARD_ValidTrack2_PAN(pobTran) != VS_SUCCESS)
				{
					pobTran->inErrorMsg = _SDK_ERROR_CODE_CARD_MODULE_10_ERROR_;
					return (VS_ERROR);
				}
			}
			
	                /* 第六步驟檢核SeviceCode */
                        if (inCheckFallbackFlag(pobTran) != VS_SUCCESS)
			{
				pobTran->inErrorMsg = _SDK_ERROR_CODE_SWIPE_EMV_CARD_;
	                        return (VS_ERROR);
			}

	                /* 第七步驟檢核ExpDate */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetExpiredDateCheck(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
			{
				if (inCARD_ValidTrack2_ExpDate(pobTran) != VS_SUCCESS)
				{
					pobTran->inErrorMsg = _SDK_ERROR_CODE_CARD_EXP_ERROR_;
					return (VS_ERROR);
				}
			}

                        return (VS_SUCCESS);
	        }
                /* （idle 插晶片卡此function不會發生）or 晶片卡事件 */
                else if  (ginIdleICCStatus == VS_TRUE || ginEventCode == _EMV_DO_EVENT_)
                {
			/* 若啟動FALLBACK，而且下一次不是刷卡事件，關FALLBACK */
			if (ginFallback == VS_TRUE)
			{
				inEMV_SetICCReadFailure(VS_FALSE);		/* 關閉FALL BACK */
			}
			
			/* 取消感應交易 */
                        inCTLS_CancelTransacton_Flow();
                        inFunc_ResetTitle(pobTran);
                        
                        /* 取得晶片卡資料 */
                        if (inEMV_GetEMVCardData(pobTran) != VS_SUCCESS)
			{
				pobTran->inErrorMsg = _SDK_ERROR_CODE_EMV_SELECT_AID_FAIL_;
                                return (VS_ERROR);
			}
                        else
                        {
				if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
				{
					if (inEMV_GetCardNoFlow(pobTran) != VS_SUCCESS)
					{
						pobTran->inErrorMsg = _SDK_ERROR_CODE_EMV_SELECT_AID_FAIL_;
						return (VS_ERROR);
					}

					/* 第三步驟 判斷card bin 讀HDT */
					if (inCARD_GetBin(pobTran) != VS_SUCCESS)
					{
						pobTran->inErrorMsg = _SDK_ERROR_CODE_CARD_GET_BIN_;
						return(VS_ERROR);
					}

					/* 第四步驟檢核PAN module 10 */
					memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
					inGetModule10Check(szFuncEnable);
					if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
					{
						/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
						if (inCARD_ValidTrack2_PAN(pobTran) != VS_SUCCESS)
						{
							pobTran->inErrorMsg = _SDK_ERROR_CODE_CARD_MODULE_10_ERROR_;
							return (VS_ERROR);
						}
					}

					/* 第五步驟檢核ExpDate */
					memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
					inGetExpiredDateCheck(szFuncEnable);
					if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
					{
						if (inCARD_ValidTrack2_ExpDate(pobTran) != VS_SUCCESS)
						{
							pobTran->inErrorMsg = _SDK_ERROR_CODE_CARD_EXP_ERROR_;
							return (VS_ERROR);
						}
					}
					
				}
				/* Host、TRT在inEMV_GetEMVCardData裡面selectAID時設定 */
				/* 如果之後有不同的收單行，可能要用AID來分 (EX:AE晶片卡)*/
                        }

                        return (VS_SUCCESS);
                } 
		/* 感應事件 */
		else if (ginEventCode == _SENSOR_EVENT_)
		{
			/* 若啟動FALLBACK，而且下一次不是刷卡事件，關FALLBACK */
			if (ginFallback == VS_TRUE)
			{
				inEMV_SetICCReadFailure(VS_FALSE);		/* 關閉FALL BACK */
			}
			
			/* 收到的retuen不是等待資料的狀態就去解析 */
			ulCTLS_RetVal = ulCTLS_CheckResponseCode_SALE(pobTran);
			
			if (ulCTLS_RetVal == d_EMVCL_RC_NO_CARD)
                        {
				pobTran->inErrorMsg = _SDK_ERROR_CODE_TIMEOUT_;
				/* Timeout沒卡 */
				return (VS_TIMEOUT);
			}
			else if (ulCTLS_RetVal == d_EMVCL_RC_MORE_CARDS)
			{
				pobTran->inErrorMsg = _SDK_ERROR_CODE_CTLS_MORE_CARDS_;
				
				return (VS_WAVE_ERROR);
			}
                        /* 這邊要切SmartPay */
                        else if (ulCTLS_RetVal == d_EMVCL_NON_EMV_CARD)
                        {
				if (pobTran->srBRec.inTxnResult != VS_SUCCESS)
				{
					pobTran->inErrorMsg = _SDK_ERROR_CODE_CTLS_FISC_FAIL_;
					return (VS_WAVE_ERROR);
				}
                               
				/* 轉 FISC_SALE */
				/* FISC incode 在 inFISC_CTLSProcess內設定 */
                        }
                        /* 走信用卡流程 */
                        else if (ulCTLS_RetVal == d_EMVCL_RC_DATA)
                        {
				/* 判斷card bin 讀HDT */
				if (inCARD_GetBin(pobTran) != VS_SUCCESS)
				{
					pobTran->inErrorMsg = _SDK_ERROR_CODE_CARD_GET_BIN_;
					return (VS_NO_CARD_BIN);
				}

				/* 檢核PAN module 10 */
				memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
				inGetModule10Check(szFuncEnable);
				if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
				{
					/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
					if (inCARD_ValidTrack2_PAN(pobTran) != VS_SUCCESS)
					{
						pobTran->inErrorMsg = _SDK_ERROR_CODE_CARD_MODULE_10_ERROR_;
						return (VS_CARD_PAN_ERROR);
					}
				}

				/* 檢核ExpDate */
				memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
				inGetExpiredDateCheck(szFuncEnable);
				if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
				{
					if (inCARD_ValidTrack2_ExpDate(pobTran) != VS_SUCCESS)
					{
						pobTran->inErrorMsg = _SDK_ERROR_CODE_CARD_EXP_ERROR_;
						return (VS_CARD_EXP_ERR);
					}
				}
                        }
			/*  Two Tap 流程 */
			else if (ulCTLS_RetVal == d_EMVCL_RC_SEE_PHONE)
			{
				/* 重Send命令，等第二次感應 */
				/* Send CTLS Readly for Sale Command */
				if (inCTLS_SendReadyForSale_Flow(pobTran) != VS_SUCCESS)
				{
					pobTran->inErrorMsg = _SDK_ERROR_CODE_CTLS_SEND_FAIL_;
					return (VS_WAVE_ERROR);
				}
				
				ginEventCode = -1;
				continue;
			}
			/* 感應錯誤 */
			else
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "感應失敗,0x%08lX", ulCTLS_RetVal);
					inLogPrintf(AT, szDebugMsg);
				}
				
				pobTran->inErrorMsg = _SDK_ERROR_CODE_CTLS_ERROR_;
				return (VS_WAVE_ERROR);
			}
			
			inFunc_ResetTitle(pobTran);
			return (VS_SUCCESS);
		}
		/* 走票證流程 */
		else if (ginEventCode == _TICKET_EVENT_)
		{
			if (inMENU_Check_ETICKET_Enable(0) == VS_SUCCESS)
			{
				pobTran->inTransactionCode = _TICKET_DEDUCT_;
				pobTran->srTRec.inCode = pobTran->inTransactionCode;
				pobTran->srTRec.uszESVCTransBit = VS_TRUE;
				inFunc_ResetTitle(pobTran);
				inRetVal = inFLOW_RunOperation(pobTran, _OPERATION_TICKET_DEDUCT_);

				return (inRetVal);
			}
			else
			{
				/* 沒開票證，不做反應 */
			}
		}
                
		/* 進迴圈前先清MSR BUFFER */
		inCARD_Clean_MSR_Buffer();
		while (1)
		{
			ginEventCode = -1;
			/* ------------偵測刷卡------------------*/
			inMSR_RetVal = inCARD_MSREvent();
			if (inMSR_RetVal == VS_SUCCESS)
			{
				/* 刷卡事件 */
				ginEventCode = _SWIPE_EVENT_;
			}
			/* 回復錯誤訊息蓋掉的圖 */
			else if (inMSR_RetVal == VS_SWIPE_ERROR)
			{
				pobTran->inErrorMsg = _SDK_ERROR_CODE_TRT_ERROR_;
				return (VS_SWIPE_ERROR);
			}
			
			/* ------------偵測晶片卡---------------- */
			inEMV_RetVal = inEMV_ICCEvent();
			if (inEMV_RetVal == VS_SUCCESS)
			{
				/* 晶片卡事件 */
				ginEventCode = _EMV_DO_EVENT_;
			}
			
			/* ------------偵測感應卡------------------ */
			inCTLS_RetVal = inCTLS_ReceiveReadyForSales_Flow(pobTran);
			if (inCTLS_RetVal == VS_SUCCESS)
			{
				/* 感應卡事件 */
				ginEventCode = _SENSOR_EVENT_;
			}

			/* ------------偵測key in------------------ */
			szKey = -1;
			szKey = uszKBD_Key();
			
			/* 感應倒數時間 && Display Countdown */
			if (inTimerGet(_TIMER_NEXSYS_4_) == VS_SUCCESS)
			{
				/* 感應時間到Timeout */
				szKey = _KEY_TIMEOUT_;
			}

			if (szKey == _KEY_TIMEOUT_)
			{
				/* 因CTLS Timeout參數在XML內，所以將CTLS API Timeout時間拉長至60秒，程式內直接用取消交易來模擬TimeOut效果 */
				inCTLS_CancelTransacton_Flow();
				/* Timeout */
				pobTran->inErrorMsg = _SDK_ERROR_CODE_TIMEOUT_;
				
				return (VS_TIMEOUT);
			}
			else if (szKey == _KEY_CANCEL_)
			{
				/* 因CTLS Timeout參數在XML內，所以將CTLS API Timeout時間拉長至60秒，程式內直接用取消交易來模擬TimeOut效果 */
				inCTLS_CancelTransacton_Flow();
				/* Cancel */
				return (VS_USER_CANCEL);
			}
			
			/* 有事件發生，跳出迴圈做對應反應 */
			if (ginEventCode != -1)
			{
				break;
			}
			
		}/* while (1) 偵測事件迴圈...*/
		
		
	} /* while (1) 對事件做回應迴圈...*/

        return (VS_SUCCESS);
}

/*
Function        :inNexsysSDK_InitCommDevice
Date&Time       :2018/9/20 下午 5:07
Describe        :For SDK使用，只設定Function Index，但不做Initial動作
*/
int inNexsysSDK_InitCommDevice(TRANSACTION_OBJECT *pobTran)
{
        char    szCommmode[1 + 1] = {0};
	char	szDialBackupEnable[2 + 1] = {0};

        /* inCOMM_InitCommDevice() START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inCOMM_InitCommDevice() START！");
        }

	memset(szDialBackupEnable, 0x00, sizeof(szDialBackupEnable));
	inGetDialBackupEnable(szDialBackupEnable);
	if (memcmp(szDialBackupEnable, "Y", strlen("Y")) == 0)
	{
		inSetCommMode(_COMM_ETHERNET_MODE_);
		inSaveCFGTRec(0);
	}
	
        memset(szCommmode,0x00,sizeof(szCommmode));
        /* 取得通訊模式 */
        if (inGetCommMode(szCommmode) == VS_ERROR)
        {
                return (VS_ERROR);
        }
        
        /* Modem Mode */
        if (memcmp(szCommmode, _COMM_MODEM_MODE_, 1) == 0)
        {
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "MODEM");
		}
		
		inCOMM_MODEM_SetFuncIndex();
        }
        /* 乙太網路 Mode */
        else if (memcmp(szCommmode, _COMM_ETHERNET_MODE_, 1) == 0)
        {
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "ETHERNET");
		}
		
		inCOMM_ETHERNET_SetFuncIndex();
        }
	else if (memcmp(szCommmode, _COMM_GPRS_MODE_, 1) == 0	||
		 memcmp(szCommmode, _COMM_3G_MODE_, 1) == 0	||
		 memcmp(szCommmode, _COMM_4G_MODE_, 1) == 0)
        {
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "GPRS");
		}
		
		inCOMM_GPRS_SetFuncIndex();
        }
	else if (memcmp(szCommmode, _COMM_WIFI_MODE_, 1) == 0)
        {
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "WiFi");
		}
		
		inCOMM_WiFi_SetFuncIndex();
        }

        /* inCOMM_InitCommDevice()_END */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inCOMM_InitCommDevice()_END");
        }
	
        return (VS_SUCCESS);
}

/*
Function        :inNexsysSDK_ErrorMsg_Transform
Date&Time       :2018/9/26 下午 1:18
Describe        :
*/
int inNexsysSDK_ErrorMsg_Transform(TRANSACTION_OBJECT *pobTran, int *inSDKErrorMsg)
{
	if (pobTran->inErrorMsg == _ERROR_CODE_V3_GET_HDPT_TAG_FAIL_)
	{
		*inSDKErrorMsg = _SDK_ERROR_CODE_GET_HDPT_TAG_ERROR_;
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_FUNC_CLOSE_)
	{
		*inSDKErrorMsg = _SDK_ERROR_CODE_FUNCTION_CLOSE_ERROR_;
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_COMM_	||
		 pobTran->inTransactionResult == _TRAN_RESULT_COMM_ERROR_)
	{
		*inSDKErrorMsg = _SDK_ERROR_CODE_TXN_COMM_ERROR_;
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_EMV_PLS_READ_EMV_	||
		 pobTran->inErrorMsg == _ERROR_CODE_V3_EMV_PLS_READ_EMV_CUP_)
	{
		*inSDKErrorMsg = _SDK_ERROR_CODE_SWIPE_EMV_CARD_;
	}
	else if (pobTran->inErrorMsg != 0)
	{
		*inSDKErrorMsg = pobTran->inErrorMsg;
	}
	else
	{
		*inSDKErrorMsg = _SDK_ERROR_CODE_ERROR_;
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inNexsySDK_SelfTest
Date&Time       :2018/9/18 上午 9:29
Describe        :
*/
int inNexsysSDK_SelfTest(void)
{
	int			inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	long			lnOrgInvNum = 0;
	long			lnPreAuthCompAmt = 0;
	char			szTMSOK[2 + 1] = {0};
	char			szAuthCode[6 + 1] = {0};
	char			szExpDate[4 + 1] = {0};
	char			szCheckNo[5 + 1] = {0};
	char			szPAN[20 + 1] = {0};
	TRANSACTION_OBJECT	pobTran;
	
	CTOS_EthernetOpen();
	CTOS_EthernetConfigSet(d_ETHERNET_CONFIG_IP, (unsigned char*)"10.105.7.88", strlen("10.105.7.88"));
	CTOS_EthernetConfigSet(d_ETHERNET_CONFIG_MASK, (unsigned char*)"255.255.254.0", strlen("255.255.254.0"));
	CTOS_EthernetConfigSet(d_ETHERNET_CONFIG_GATEWAY, (unsigned char*)"10.105.7.254", strlen("10.105.7.254"));
	
	/* 開啟Debug Log */
	OPT_DEBUG_ON();
	
	memset(&pobTran, 0x00, sizeof(pobTran));
	OPT_TerminalInitial();
	
	/* For 測試使用，電文改送不加密 */
	OPT_SET_ENCRYPTMODE_NONE_FOR_TEST();
	
	memset(szTMSOK, 0x00, sizeof(szTMSOK));
	OPT_GET_TMS_OK(szTMSOK);
	if (memcmp(szTMSOK, "Y", strlen("Y")) != 0)
	{
		OPT_SET_TID("13992001");
		OPT_SET_MID("0108003521");
		OPT_SET_TMS_IP("61.220.34.44");
		OPT_SET_TMS_PORT("9736");
					
		/* 如果IP設為Internet Gateway的話 要設成IFES */
		if (1)
		{
			OPT_SET_COMM_MODE_ETHERNET_IFES();
		}
		else
		{
			OPT_SET_COMM_MODE_ETHERNET();
		}
		
		/* ISO8583下載 */
		if (1)
		{
			memset(pobTran.szManagementCode, 0x00, sizeof(pobTran.szManagementCode));
			strcpy(pobTran.szManagementCode, "111111");
			inRetVal = OPT_TMS_DOWNLOAD(&pobTran);
		}
		/* FTP下載 */
		else
		{
			OPT_SET_FTP_IP("61.220.34.44");
			inRetVal = OPT_TMS_DOWNLOAD_FTPS(&pobTran);
		}
	}
	else
	{
		/* 讀卡 */
		inRetVal = OPT_ReadCard(&pobTran);
		
		/* 預授權 */
		if (inRetVal == _SDK_ERROR_CODE_SUCCESS_)
		{
			/* 預授權金額 */
			pobTran.srBRec.lnTxnAmount = 500;
			sprintf(pobTran.srBRec.szStoreID, "%s", "                  ");
			inRetVal = OPT_PreAuth(&pobTran);
			lnOrgInvNum = pobTran.srBRec.lnOrgInvNum;
		}
		
		/* 預授權完成 */
		if (inRetVal == _SDK_ERROR_CODE_SUCCESS_)
		{
			memset(&pobTran, 0x00, sizeof(pobTran));
			/* 調閱編號 */
			pobTran.srBRec.lnOrgInvNum = lnOrgInvNum;
			/* 預先授權完成金額 */
			lnPreAuthCompAmt = 200;
			pobTran.srBRec.lnTxnAmount = lnPreAuthCompAmt;
			/* 櫃號 */
			sprintf(pobTran.srBRec.szStoreID, "%s", "                  ");

			inRetVal = OPT_PreAuthComp(&pobTran);
			
			memset(szPAN, 0x00, sizeof(szPAN));
			strcpy(szPAN, pobTran.srBRec.szPAN);
			
			memset(szAuthCode, 0x00, sizeof(szAuthCode));
			strcpy(szAuthCode, pobTran.srBRec.szAuthCode);
			
			memset(szExpDate, 0x00, sizeof(szExpDate));
			strcpy(szExpDate, pobTran.srBRec.szExpDate);
			
			memset(szCheckNo, 0x00, sizeof(szCheckNo));
			OPT_EXPIRE_DATE_TRANSFORM_TO_CHECK_NO(&pobTran, szCheckNo);
		}
		
		memset(&pobTran, 0x00, sizeof(pobTran));
		/* 填入金額 */
		pobTran.srBRec.lnTxnAmount = 100;
		/* 填入授權碼 */
		strcpy(pobTran.srBRec.szAuthCode, szAuthCode);
		/* 填入卡號 */
		strcpy(pobTran.srBRec.szPAN, szPAN);
		/* 檢查碼或有效期 */
		if (1)
		{
			/* 檢查碼 */
			strcpy(pobTran.srBRec.szCheckNO, szCheckNo);
		}
		else
		{
			/* 有效期 */
			strcpy(pobTran.srBRec.szExpDate, szExpDate);
		}
		sprintf(pobTran.srBRec.szStoreID, "%s", "                  ");
		OPT_Refund(&pobTran);
		
//		OPT_Settle(&pobTran);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inNexsySDK_CardDisplayTest
Date&Time       :2018/9/21 下午 1:41
Describe        :
*/
int inNexsysSDK_CardDisplayTest(TRANSACTION_OBJECT *pobTran)
{
	int		inCardLabelLen = 0;
	char		szTemplate[50 + 1] = {0};
	char		szTemplate2[50 + 1] = {0};
	unsigned char	szkey = 0;
	
	CTOS_LCDTClearDisplay();
	CTOS_LCDTTFSelect((unsigned char*)_CHINESE_FONE_1_ , 0);
	CTOS_LCDTSelectFontSize(0x0E1E);
	
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "Test_Expire: %s", pobTran->srBRec.szExpDate);
	CTOS_LCDTPrintAligned(6, (unsigned char*)szTemplate, d_LCD_ALIGNLEFT);
	if (pobTran->srBRec.uszUCARDTransBit == VS_TRUE)
	{
		memset(szTemplate2, 0x00, sizeof(szTemplate2));
		memcpy(szTemplate2, &pobTran->srBRec.szMCP_BANKID[5], 2);
		inCardLabelLen = atoi(szTemplate2);
		memset(szTemplate2, 0x00, sizeof(szTemplate2));
		memcpy(szTemplate2, &pobTran->srBRec.szMCP_BANKID[7], inCardLabelLen);
		
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "Test_Type: %s", szTemplate2);
	}
	else
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "Test_Type: %s", pobTran->srBRec.szCardLabel);
	}
	CTOS_LCDTPrintAligned(7, (unsigned char*)szTemplate, d_LCD_ALIGNLEFT);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "Test_CardNo: %s", pobTran->srBRec.szPAN);
	CTOS_LCDTPrintAligned(8, (unsigned char*)szTemplate, d_LCD_ALIGNLEFT);
	
	CTOS_KBDGet(&szkey);
	
	return (VS_SUCCESS);
}

/*
Function        :inNexsySDK_IP_Before_Test
Date&Time       :2018/9/28 下午 12:05
Describe        :
*/
int inNexsysSDK_IP_Before_Test(void)
{
	int		inTagLen = 0;
	char		szTag[50 + 1] = {0};
	char		szTemplate[100 + 1] = {0};
	unsigned char	uszKey = 0;
	
	CTOS_LCDTClearDisplay();
	CTOS_LCDTTFSelect((unsigned char*)_CHINESE_FONE_1_ , 0);
	CTOS_LCDTSelectFontSize(0x0E1E);
	
	memset(szTag, 0x00, sizeof(szTag));
	inTagLen = sizeof(szTag);
	CTOS_EthernetConfigGet(d_ETHERNET_CONFIG_IP, (unsigned char*)szTag, (unsigned char*)&inTagLen);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "Bef IP:%s", szTag);
	CTOS_LCDTPrintXY(1, 1, (unsigned char*)szTemplate);
	
	memset(szTag, 0x00, sizeof(szTag));
	inTagLen = sizeof(szTag);
	CTOS_EthernetConfigGet(d_ETHERNET_CONFIG_MASK, (unsigned char*)szTag, (unsigned char*)&inTagLen);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "Bef MASK:%s", szTag);
	CTOS_LCDTPrintXY(1, 2, (unsigned char*)szTemplate);
	
	memset(szTag, 0x00, sizeof(szTag));
	inTagLen = sizeof(szTag);
	CTOS_EthernetConfigGet(d_ETHERNET_CONFIG_GATEWAY, (unsigned char*)szTag, (unsigned char*)&inTagLen);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "Bef GATEWAY:%s", szTag);
	CTOS_LCDTPrintXY(1, 3, (unsigned char*)szTemplate);
	
	CTOS_LCDTPrintXY(1, 4, (unsigned char*)"pls test ping");
	
	CTOS_KBDGet(&uszKey);
	
	return (VS_SUCCESS);
}

/*
Function        :inNexsySDK_IP_After_Test
Date&Time       :2018/9/28 下午 12:05
Describe        :
*/
int inNexsysSDK_IP_After_Test(void)
{
	int		inTagLen = 0;
	char		szTag[50 + 1] = {0};
	char		szTemplate[100 + 1] = {0};
	unsigned char	uszKey = 0;
	
	CTOS_LCDTClearDisplay();
	CTOS_LCDTTFSelect((unsigned char*)_CHINESE_FONE_1_ , 0);
	CTOS_LCDTSelectFontSize(0x0E1E);
	
	memset(szTag, 0x00, sizeof(szTag));
	inTagLen = sizeof(szTag);
	CTOS_EthernetConfigGet(d_ETHERNET_CONFIG_IP, (unsigned char*)szTag, (unsigned char*)&inTagLen);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "Aft IP:%s", szTag);
	CTOS_LCDTPrintXY(1, 5, (unsigned char*)szTemplate);
	
	memset(szTag, 0x00, sizeof(szTag));
	inTagLen = sizeof(szTag);
	CTOS_EthernetConfigGet(d_ETHERNET_CONFIG_MASK, (unsigned char*)szTag, (unsigned char*)&inTagLen);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "Aft MASK:%s", szTag);
	CTOS_LCDTPrintXY(1, 6, (unsigned char*)szTemplate);
	
	memset(szTag, 0x00, sizeof(szTag));
	inTagLen = sizeof(szTag);
	CTOS_EthernetConfigGet(d_ETHERNET_CONFIG_GATEWAY, (unsigned char*)szTag, (unsigned char*)&inTagLen);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "Aft GATEWAY:%s", szTag);
	CTOS_LCDTPrintXY(1, 7, (unsigned char*)szTemplate);
	
	CTOS_LCDTPrintXY(1, 4, (unsigned char*)"pls test ping");
	
	CTOS_KBDGet(&uszKey);
	
	return (VS_SUCCESS);
}

/*
Function        :inNexsySDK_TMS_Download
Date&Time       :2018/10/2 上午 10:00
Describe        :
*/
int inNexsysSDK_TMS_Download(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = _SDK_ERROR_CODE_SUCCESS_;
	int		inFuncRetVal = VS_SUCCESS;
	int		inTagLen = 0;
	char		szTag[20 + 1] = {0};
	RTC_NEXSYS	srRTC; 		/* Date & Time */
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNEXSYSSDK_TMS_DOWNLOAD() START !");
	}
	
	if (pobTran->uszFTP_TMS_Download == _TMS_DOWNLOAD_SECURE_MODE_FTPS_	||
	    pobTran->uszFTP_TMS_Download == _TMS_DOWNLOAD_SECURE_MODE_SFTP_)
	{
		inFILE_Delete((unsigned char *)_FTPFLT_FILE_NAME_);
		/* 刪除TermInfo2 */
		inFILE_Delete((unsigned char *)_FTP_INFO_2_);
	}
	else
	{
		inFILE_Delete((unsigned char *)_TMSFLT_FILE_NAME_);
	}
	
	inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_TMS_DOWNLOAD_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜TMS參數下載＞ */
	
	/* 將IP、MASK、GATEWAY設定回去，回報需要這些資料 */
	memset(szTag, 0x00, sizeof(szTag));
	inTagLen = sizeof(szTag);
	inETHERNET_Cofig_Get(d_ETHERNET_CONFIG_IP, (unsigned char*)szTag, (unsigned char*)&inTagLen);
	OPT_SET_EDC_IP(szTag);
	
	memset(szTag, 0x00, sizeof(szTag));
	inTagLen = sizeof(szTag);
	inETHERNET_Cofig_Get(d_ETHERNET_CONFIG_MASK, (unsigned char*)szTag, (unsigned char*)&inTagLen);
	OPT_SET_SUBMASK(szTag);
	
	memset(szTag, 0x00, sizeof(szTag));
	inTagLen = sizeof(szTag);
	inETHERNET_Cofig_Get(d_ETHERNET_CONFIG_GATEWAY, (unsigned char*)szTag, (unsigned char*)&inTagLen);
	OPT_SET_GATEWAY(szTag);
	
	/* 取得EDC時間日期 */
        memset(&srRTC, 0x00, sizeof(RTC_NEXSYS));
	if (inFunc_GetSystemDateAndTime(&srRTC) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "時間取得失敗");
		}
		
		pobTran->inErrorMsg = _SDK_ERROR_CODE_GET_TIME_FAIL_;
		inRetVal = _SDK_ERROR_CODE_ERROR_;
		
		return (inRetVal);
	}
	inFunc_Sync_BRec_Date_Time(pobTran, &srRTC);
	
	inFuncRetVal = inFLOW_RunFunction(pobTran, _FUNCTION_All_HOST_MUST_SETTLE_);
	if (inFuncRetVal != VS_SUCCESS)
	{
		pobTran->inErrorMsg = _SDK_ERROR_CODE_TXN_PLS_SETTLE_FIRST_;
		inRetVal = _SDK_ERROR_CODE_ERROR_;
		
		return (inRetVal);
	}
	
	/* 先LOAD NCCC HOST */
	pobTran->srBRec.inHDTIndex = 0;
        inLoadHDTRec(pobTran->srBRec.inHDTIndex);
        inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
        /* TMS的CPT參數 */
        inLoadTMSCPTRec(0);
        /* EDC參數 */
        inLoadEDCRec(0);
        /* CPT參數 */
        inLoadCPTRec(0);
        /* FTP參數 */
        inLoadTMSFTPRec(0);
        
        pobTran->inTransactionCode = _NCCCTMS_LOGON_; /* 先Hard code */
        pobTran->inTMSDwdMode = _TMS_MANUAL_DOWNLOAD_; /* 手動下載 */
	
	inNCCCTMS_Field58_Initial(pobTran);
	
	/* 下載管理號碼 */
	strcpy(gsrTMS_Field58.szManagementCode, pobTran->szManagementCode);
	
	inFuncRetVal = inFLOW_RunFunction(pobTran, _FUNCTION_TMS_CONNECT_);
	if (inFuncRetVal != VS_SUCCESS)
	{
		pobTran->inErrorMsg = _SDK_ERROR_CODE_TXN_COMM_ERROR_;
		inRetVal = _SDK_ERROR_CODE_ERROR_;
		
		return (inRetVal);
	}
	
	inFuncRetVal = inFLOW_RunFunction(pobTran, _FUNCTION_TMS_SEND_RECEIVE_FLOW_);
	if (inFuncRetVal != VS_SUCCESS)
	{
		pobTran->inErrorMsg = _SDK_ERROR_CODE_TXN_COMM_ERROR_;
		inRetVal = _SDK_ERROR_CODE_ERROR_;
		
		return (inRetVal);
	}
	
	inFuncRetVal = inFLOW_RunFunction(pobTran, _FUNCTION_TMS_RESULT_HANDLE_);
	if (inFuncRetVal != VS_SUCCESS	||
	    memcmp(pobTran->srBRec.szRespCode , "00", strlen("00") != 0))
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "TMS 下載失敗，回應碼：%s", pobTran->srBRec.szRespCode);
		}
		
		inRetVal = _SDK_ERROR_CODE_ERROR_;
		
		return (inRetVal);
	}
	
	inFuncRetVal = inFLOW_RunFunction(pobTran, _FUNCTION_TMS_DISCONNECT_);
	if (inFuncRetVal != VS_SUCCESS)
	{
		pobTran->inErrorMsg = _SDK_ERROR_CODE_TXN_COMM_ERROR_;
		inRetVal = _SDK_ERROR_CODE_ERROR_;
		
		return (inRetVal);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNEXSYSSDK_TMS_DOWNLOAD() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :inNexsysSDK_Set_TMSOK_Flow
Date&Time       :2018/11/20 上午 11:07
Describe        :
*/
int inNexsysSDK_Set_TMSOK_Flow(void)
{
	int	inRetVal = VS_SUCCESS;
	
	inRetVal = inFunc_Set_TMSOK_Flow();
	
	return (VS_SUCCESS);
}
