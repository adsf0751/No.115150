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
#include "../SOURCE/FUNCTION/CFGT.h"
#include "../SOURCE/FUNCTION/EDC.h"
#include "../SOURCE/FUNCTION/HDT.h"
#include "../SOURCE/FUNCTION/HDPT.h"
#include "../SOURCE/FUNCTION/IPASSDT.h"
#include "../SOURCE/FUNCTION/TDT.h"
#include "../SOURCE/FUNCTION/File.h"
#include "../SOURCE/FUNCTION/ECR.h"
#include "../SOURCE/EVENT/Flow.h"
#include "../SOURCE/EVENT/MenuMsg.h"
#include "../CTLS/CTLS.h"
#include "../NCCC/NCCCTicketSrc.h"
#include "../NCCC/NCCCTicketIso.h"
#include "../NCCC/NCCCsrc.h"
#include "IPassMicroPayment.h"
#include "IPassMicroPayment_ErrCode.h"
#include "IPASSFunc.h"

extern  int		ginDebug;	/* Debug使用 extern */
extern	int		ginISODebug;
extern	int		ginFindRunTime;
extern	unsigned long	gulDemoTicketPoint;

/* 註:若一卡通API回傳0103，則必定是TID錯誤！！！！ 因為很重要所以註解要打三遍 */
/* 註:若一卡通API回傳0103，則必定是TID錯誤！！！！ 因為很重要所以註解要打三遍 */
/* 註:若一卡通API回傳0103，則必定是TID錯誤！！！！ 因為很重要所以註解要打三遍 */

/*
Function        :inIPASS_First_Tap
Date&Time       :2017/12/22 上午 10:12
Describe        :
*/
int inIPASS_First_Tap(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = 0;	/* IPASS API使用 */
	int		inRetVal2 = 0;
	int		inPollTime = 0;
        int		inInput_Len = 0, inOutput_Len = 0;
	int		inCount = 0;
	int		inUIDLen = 0;
        long		lnAmount = 0;
	char		szDebugMsg[1000 + 1] = {0};
	char		szFuncEnable[2 + 1] = {0};
	char		szDemoMode[2 + 1] = {0};
	char		szTemplate[20 + 1] = {0};
	char		szAscii[20 + 1] = {0};
	char		szUnixTime[10 + 1] = {0};
	unsigned char	uszInput[200] = {0}, uszOutput[500] = {0};
	RTC_NEXSYS	srRTC_Out;
	
	/* 教育訓練模式 */
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		if (pobTran->srTRec.inCode == _TICKET_IPASS_AUTO_TOP_UP_)
		{
			inNCCC_Ticket_Set_Msg(pobTran->srTRec.inTicketType, &pobTran->inTicketMsgStatus, _TICKET_MSG_IPASS_AUTO_TOP_UP_);
		}
		else
		{
			inNCCC_Ticket_Set_Msg(pobTran->srTRec.inTicketType, &pobTran->inTicketMsgStatus, _TICKET_MSG_IPASS_PUT_CARD_IN_AREA_);
		}
		
		inNCCC_Ticket_Set_CTLS_Light(&pobTran->inCTLSLightStatus, _CTLS_LIGHT_YELLOW_);
		inNCCC_Ticket_Set_Msg(pobTran->srTRec.inTicketType, &pobTran->inTicketMsgStatus, _TICKET_MSG_IPASS_DO_NOT_MOVE_CARD_);

		pobTran->srTRec.lnCardRemainAmount = gulDemoTicketPoint;
		pobTran->srTRec.lnTopUpAmount = 500;     /* 自動加值金額(元/次) */
		
		/* 教育訓練模式金額大於500是聯名卡 */
		if (pobTran->srTRec.lnTxnAmount >= 500)
			pobTran->srTRec.srIPASSRec.uszCB_CardBit = VS_TRUE;

		return (VS_SUCCESS);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "----------------------------------------");
			inLogPrintf(AT, "inIPASS_First_Tap() START !");
		}

		if (ginFindRunTime == VS_TRUE)
		{
			inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
		}
			
		/* UI */
		memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
		inGetIntegrate_Device(szFuncEnable);
		if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
		{

		}
		else
		{
			if (pobTran->srTRec.inCode == _TICKET_IPASS_AUTO_TOP_UP_)
			{
				inNCCC_Ticket_Set_Msg(pobTran->srTRec.inTicketType, &pobTran->inTicketMsgStatus, _TICKET_MSG_IPASS_AUTO_TOP_UP_);
			}
			else
			{
				
			}
		}

		memset(uszInput, 0x00, sizeof(uszInput));
		memset(uszOutput, 0x00, sizeof(uszOutput));

		/* 組Request Command */
		inInput_Len = inIPASS_Command_Packet(pobTran, uszInput);

		if (ginDebug == VS_TRUE)
		{
			switch (pobTran->srTRec.inCode)
			{
				case _TICKET_IPASS_QUERY_ :
				case _TICKET_IPASS_INQUIRY_ :
					inLogPrintf(AT, "[詢卡]");
					break;
				case _TICKET_IPASS_DEDUCT_ :
					inLogPrintf(AT, "[購貨]");
					break;
				case _TICKET_IPASS_REFUND_ :
					inLogPrintf(AT, "[退貨加值]");
					break;
				case _TICKET_IPASS_TOP_UP_ :
					inLogPrintf(AT, "[手動加值]");
					break;
				case _TICKET_IPASS_AUTO_TOP_UP_ :
					inLogPrintf(AT, "[自動加值]");
					break;
				case _TICKET_IPASS_VOID_TOP_UP_ :
					inLogPrintf(AT, "[取消加值]");
					break;
				default :
					break;
			}

			inLogPrintf(AT, "API_1 Input :");
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			inFunc_BCD_to_ASCII(szDebugMsg, uszInput, inInput_Len);
			inLogPrintf(AT, szDebugMsg);
		}
		if (ginISODebug == VS_TRUE)
		{
			inPRINT_ChineseFont(" ", _PRT_ISO_);
			switch (pobTran->srTRec.inCode)
			{
				case _TICKET_IPASS_QUERY_ :
				case _TICKET_IPASS_INQUIRY_ :
					inPRINT_ChineseFont("[詢卡]", _PRT_ISO_);
					break;
				case _TICKET_IPASS_DEDUCT_ :
					inPRINT_ChineseFont("[購貨]", _PRT_ISO_);
					break;
				case _TICKET_IPASS_REFUND_ :
					inPRINT_ChineseFont("[退貨加值]", _PRT_ISO_);
					break;
				case _TICKET_IPASS_TOP_UP_ :
					inPRINT_ChineseFont("[手動加值]", _PRT_ISO_);
					break;
				case _TICKET_IPASS_AUTO_TOP_UP_ :
					inPRINT_ChineseFont("[自動加值]", _PRT_ISO_);
					break;
				case _TICKET_IPASS_VOID_TOP_UP_ :
					inPRINT_ChineseFont("[取消加值]", _PRT_ISO_);
					break;
				default :
					break;
			}

			inPRINT_ChineseFont("API_1 Input :", _PRT_ISO_);
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			inFunc_BCD_to_ASCII(szDebugMsg, uszInput, inInput_Len);
			inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
		}

		inPollTime = 0;
#ifndef _LOAD_KEY_AP_
		/* 參考IPassMicroPayment.h API define */
		
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "IPASS API START");
		}
		
		switch (pobTran->srTRec.inCode)
		{
			case _TICKET_IPASS_QUERY_ :
			case _TICKET_IPASS_INQUIRY_ :
				inRetVal = IPassMP_RequestQueryTicket(inPollTime, uszInput, inInput_Len, uszOutput, (unsigned int*)&inOutput_Len);
				break;
			case _TICKET_IPASS_DEDUCT_ :
				inRetVal = IPassMP_RequestDeductValue(inPollTime, uszInput, inInput_Len, uszOutput, (unsigned int*)&inOutput_Len);
				break;
			case _TICKET_IPASS_REFUND_ :
			case _TICKET_IPASS_TOP_UP_ :
			case _TICKET_IPASS_AUTO_TOP_UP_ :
				inRetVal = IPassMP_RequestAddValue(inPollTime, uszInput, inInput_Len, uszOutput, (unsigned int*)&inOutput_Len);
				break;
			case _TICKET_IPASS_VOID_TOP_UP_ :
				inRetVal = IPassMP_RequestCancelAddValue(inPollTime, uszInput, inInput_Len, uszOutput, (unsigned int*)&inOutput_Len);
				break;
			default :
				return (VS_ERROR);
		}
		
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "IPASS API END");
			inLogPrintf(AT, "inRetVal = 0x%04x", inRetVal);
		}
#endif
		
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "API Response Len1 [%d]", inOutput_Len);
			inLogPrintf(AT, szDebugMsg);
		}
		if (ginISODebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "API Response Len1 [%d]", inOutput_Len);
			inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
		}

		inNCCC_Ticket_Set_CTLS_Light(&pobTran->inCTLSLightStatus, _CTLS_LIGHT_YELLOW_);
		inNCCC_Ticket_Set_Msg(pobTran->srTRec.inTicketType, &pobTran->inTicketMsgStatus, _TICKET_MSG_IPASS_DO_NOT_MOVE_CARD_);
		
		if (inRetVal == IPASS_EXECUTE_OK			|| 
		    inRetVal == IPASS_ERROR_RECOVERY			|| 
		    inRetVal == IPASS_ERROR_AUTOLOAD_OFF		|| 
		    inRetVal == IPASS_ERROR_AUTOLOAD_ON			|| 
		    inRetVal == IPASS_ERROR_INVALID_AUTOADDVALUE	|| 
		    inRetVal == IPASS_ERROR_BLACK_LIST)
		/* 0x0000 執行正確
		 * 0x0002 票值回覆
		 * 0x0003 關閉自動加值
		 * 0x0004 開啟自動加值
		 * 0x0517 黑名單鎖卡
		 * 0x0705 黑名單鎖卡 */
		{
			/* 黑名單 */
			if (inRetVal == IPASS_ERROR_INVALID_AUTOADDVALUE || 
			    inRetVal == IPASS_ERROR_BLACK_LIST)
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "判斷為黑名單");
				}
				if (ginISODebug == VS_TRUE)
				{
					char	szDebugMsg[100 + 1] = {0};

					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "判斷為黑名單");
					inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
				}
				
				pobTran->srTRec.uszBlackListBit = VS_TRUE;
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%04X", inRetVal);
                                inIPASS_Display_Error(pobTran, szTemplate);
			}
			else if (inRetVal == IPASS_ERROR_AUTOLOAD_OFF)
			{
				pobTran->srTRec.uszCloseAutoTopUpBit = VS_TRUE;
			}

			pobTran->srTRec.srIPASSRec.inStepNum = 1;

			/* 感應成功 */
			switch (pobTran->srTRec.inCode)
			{
				case _TICKET_IPASS_DEDUCT_ :
				case _TICKET_IPASS_REFUND_ :
				case _TICKET_IPASS_TOP_UP_ :
				case _TICKET_IPASS_AUTO_TOP_UP_ :
				case _TICKET_IPASS_VOID_TOP_UP_ :
					memset(pobTran->srTRec.srIPASSRec.szDAVTITxn, 0x00, sizeof(pobTran->srTRec.srIPASSRec.szDAVTITxn));
					pobTran->srTRec.srIPASSRec.lnSign_Len = 0;

					/* 票卡扣值交易請求命令回覆 */
					/* 票卡加值交易請求命令回覆 */
					/* 票卡取消加值交易請求命令回覆 */
					if (inOutput_Len == 203)
					{
						/* 請求扣款回覆 */
						/* 卡片內容結構 (91) */
						memset(pobTran->srTRec.srIPASSRec.szCardInfo, 0x00, sizeof(pobTran->srTRec.srIPASSRec.szCardInfo));
						memcpy(&pobTran->srTRec.srIPASSRec.szCardInfo[0], &uszOutput[inCount], _IPASS_CARD_DATA_SIZE_);
						inCount += _IPASS_CARD_DATA_SIZE_;

						/* 簽章 (112) */
						pobTran->srTRec.srIPASSRec.lnSign_Len = _IPASS_SIGN_SIZE_112_BYTES_;
						memset(pobTran->srTRec.srIPASSRec.szSign_Data, 0x00, sizeof(pobTran->srTRec.srIPASSRec.szSign_Data));
						memcpy(&pobTran->srTRec.srIPASSRec.szSign_Data[0], &uszOutput[inCount], pobTran->srTRec.srIPASSRec.lnSign_Len);
						inCount += _IPASS_SIGN_SIZE_112_BYTES_;
					}
					/* 若回應碼為0x0002，會再加上DAVTITxn */
					else if (inOutput_Len == 478)
					{
						/* DAVTITxn (275) - 一般流程放在3(前面有詢卡可能會吐DAVTITxn) */
						memset(pobTran->srTRec.srIPASSRec.szDAVTITxn, 0x00, sizeof(pobTran->srTRec.srIPASSRec.szDAVTITxn));
						memcpy(&pobTran->srTRec.srIPASSRec.szDAVTITxn[0], &uszOutput[inCount], _IPASS_DAVTI_SIZE_);
						inCount += _IPASS_DAVTI_SIZE_;

						/* 卡片內容結構 (91) */
						memset(pobTran->srTRec.srIPASSRec.szCardInfo, 0x00, sizeof(pobTran->srTRec.srIPASSRec.szCardInfo));
						memcpy(&pobTran->srTRec.srIPASSRec.szCardInfo[0], &uszOutput[inCount], _IPASS_CARD_DATA_SIZE_);
						inCount += _IPASS_CARD_DATA_SIZE_;

						/* 簽章 (112) */
						pobTran->srTRec.srIPASSRec.lnSign_Len = _IPASS_SIGN_SIZE_112_BYTES_;
						memset(pobTran->srTRec.srIPASSRec.szSign_Data, 0x00, sizeof(pobTran->srTRec.srIPASSRec.szSign_Data));
						memcpy(&pobTran->srTRec.srIPASSRec.szSign_Data[0], &uszOutput[inCount], pobTran->srTRec.srIPASSRec.lnSign_Len);
						inCount += _IPASS_SIGN_SIZE_112_BYTES_;
					}
					else
					{
						return (VS_ERROR);
					}

					break;
				case _TICKET_IPASS_QUERY_ :
				case _TICKET_IPASS_INQUIRY_ :
					memset(pobTran->srTRec.srIPASSRec.szDAVTITxn, 0x00, sizeof(pobTran->srTRec.srIPASSRec.szDAVTITxn));
					pobTran->srTRec.srIPASSRec.lnSign_Len = 0;

					if (inOutput_Len == 91)
					{
						/* 91 */
						/* 卡片內容結構 (91) - 詢卡才做存的動作，因為要判斷是否聯名卡 */
						memset(pobTran->srTRec.srIPASSRec.szCardInfo, 0x00, sizeof(pobTran->srTRec.srIPASSRec.szCardInfo));
						memcpy(&pobTran->srTRec.srIPASSRec.szCardInfo[0], &uszOutput[inCount], _IPASS_CARD_DATA_SIZE_);
						inCount += _IPASS_CARD_DATA_SIZE_;

						/* 有簽章表示聯名卡 */
						pobTran->srTRec.srIPASSRec.uszCB_CardBit = VS_FALSE;
					}
					else if (inOutput_Len == 147)
					{
						/* 91 + 56 */
						/* 卡片內容結構 (91) */
						memset(pobTran->srTRec.srIPASSRec.szCardInfo, 0x00, sizeof(pobTran->srTRec.srIPASSRec.szCardInfo));
						memcpy(&pobTran->srTRec.srIPASSRec.szCardInfo[0], &uszOutput[inCount], _IPASS_CARD_DATA_SIZE_);
						inCount += _IPASS_CARD_DATA_SIZE_;

						/* 簽章 (56) */
						pobTran->srTRec.srIPASSRec.lnSign_Len = _IPASS_SIGN_SIZE_56_BYTES_;
						memset(pobTran->srTRec.srIPASSRec.szSign_Data, 0x00, sizeof(pobTran->srTRec.srIPASSRec.szSign_Data));
						memcpy(&pobTran->srTRec.srIPASSRec.szSign_Data[0], &uszOutput[inCount], pobTran->srTRec.srIPASSRec.lnSign_Len);
						inCount += _IPASS_SIGN_SIZE_56_BYTES_;

						/* 有簽章表示聯名卡 */
						pobTran->srTRec.srIPASSRec.uszCB_CardBit = VS_TRUE;
					}
					else if (inOutput_Len == 366)
					{
						/* 275+ 91 (票值回覆會再加上 275，所以長度變成366) */
						/* DAVTITxn (275) */
						memset(pobTran->srTRec.srIPASSRec.szDAVTITxn, 0x00, sizeof(pobTran->srTRec.srIPASSRec.szDAVTITxn));
						memcpy(&pobTran->srTRec.srIPASSRec.szDAVTITxn[0], &uszOutput[inCount], _IPASS_DAVTI_SIZE_);
						inCount += _IPASS_DAVTI_SIZE_;

						/* 卡片內容結構 (91) */
						memset(pobTran->srTRec.srIPASSRec.szCardInfo, 0x00, sizeof(pobTran->srTRec.srIPASSRec.szCardInfo));
						memcpy(&pobTran->srTRec.srIPASSRec.szCardInfo[0], &uszOutput[inCount], _IPASS_CARD_DATA_SIZE_);
						inCount += _IPASS_CARD_DATA_SIZE_;

						/* 有簽章表示聯名卡 */
						pobTran->srTRec.srIPASSRec.uszCB_CardBit = VS_FALSE;
					}
					else if (inOutput_Len == 422)
					{
						/* 275+ 91 + 56 (票值回覆會再加上 275，所以長度變成366) */
						/* DAVTITxn (275) */
						memset(pobTran->srTRec.srIPASSRec.szDAVTITxn, 0x00, sizeof(pobTran->srTRec.srIPASSRec.szDAVTITxn));
						memcpy(&pobTran->srTRec.srIPASSRec.szDAVTITxn[0], &uszOutput[inCount], _IPASS_DAVTI_SIZE_);
						inCount += _IPASS_DAVTI_SIZE_;

						/* 卡片內容結構 (91) */
						memset(pobTran->srTRec.srIPASSRec.szCardInfo, 0x00, sizeof(pobTran->srTRec.srIPASSRec.szCardInfo));
						memcpy(&pobTran->srTRec.srIPASSRec.szCardInfo[0], &uszOutput[inCount], _IPASS_CARD_DATA_SIZE_);
						inCount += _IPASS_CARD_DATA_SIZE_;

						/* 簽章 (56) */
						pobTran->srTRec.srIPASSRec.lnSign_Len = _IPASS_SIGN_SIZE_56_BYTES_;
						memset(pobTran->srTRec.srIPASSRec.szSign_Data, 0x00, sizeof(pobTran->srTRec.srIPASSRec.szSign_Data));
						memcpy(&pobTran->srTRec.srIPASSRec.szSign_Data[0], &uszOutput[inCount], pobTran->srTRec.srIPASSRec.lnSign_Len);
						inCount += _IPASS_SIGN_SIZE_56_BYTES_;

						/* 有簽章表示聯名卡 */
						pobTran->srTRec.srIPASSRec.uszCB_CardBit = VS_TRUE;
					}
					else
					{
						return (VS_ERROR);
					}

					break;
				default :
					return (VS_ERROR);
			}

			/* 長度不為0,有掃到資料 */
			if (pobTran->srTRec.srIPASSRec.szCardInfo[0] != 0x00)
			{
				/* UID或卡號 */
				inUIDLen = pobTran->srTRec.srIPASSRec.szCardInfo[0];

				memset(pobTran->srTRec.szUID, 0x00, sizeof(pobTran->srTRec.szUID));
				inFunc_BCD_to_ASCII(pobTran->srTRec.szUID, (unsigned char*)&pobTran->srTRec.srIPASSRec.szCardInfo[1], inUIDLen);

				/* 卡片餘額 */
				/* 最後一位為-1時，金額為負數 */
				if (pobTran->srTRec.srIPASSRec.szCardInfo[33] == -1)
				{
					memcpy((char*)&lnAmount, &pobTran->srTRec.srIPASSRec.szCardInfo[30], 4);
				}
				else
				{
					memcpy((char*)&lnAmount, &pobTran->srTRec.srIPASSRec.szCardInfo[30], 4);
				}

				pobTran->srTRec.lnCardRemainAmount = lnAmount;

				/* 允許自動加值 */
				if (pobTran->srTRec.srIPASSRec.szCardInfo[43] == 0x01)
				{
					/* 卡片每次自動加值金額(底數) */
					/* 和蔚修確認 */
					lnAmount = 0;
					memcpy((char*)&lnAmount, (unsigned char*)&pobTran->srTRec.srIPASSRec.szCardInfo[44], 1);
					memcpy((char*)&lnAmount + 1, (unsigned char*)&pobTran->srTRec.srIPASSRec.szCardInfo[45], 1);
					pobTran->srTRec.lnTopUpAmount = lnAmount;
				}
				else
				{
					if (pobTran->srTRec.inCode == _TICKET_IPASS_AUTO_TOP_UP_)
					{
						memset(szTemplate, 0x00, sizeof(szTemplate));
                                                sprintf(szTemplate, "%s", "0518");
                                                inIPASS_Display_Error(pobTran, szTemplate);
						return (VS_ERROR);
					}
				}
				
				/* 最後消費日期 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &pobTran->srTRec.srIPASSRec.szCardInfo[51], 4);
				memset(szAscii, 0x00, sizeof(szAscii));
				inFunc_BCD_to_ASCII(szAscii, (unsigned char*)szTemplate, 4);
				/* 有可能會全0，導致轉換出奇怪的日期 */
				if (memcmp(szAscii, "00000000", strlen("00000000")) == 0)
				{
					sprintf(pobTran->srTRec.srIPASSRec.szLastDeductDate, "00000000");
				}
				else
				{
					memset(szUnixTime, 0x00, sizeof(szUnixTime));
					inFunc_String_Hex_to_Dec_Little2Little(szAscii, szUnixTime, 8);
					memset(&srRTC_Out, 0x00, sizeof(srRTC_Out));
					inFuncGetUnixTimeToLocalTime(&srRTC_Out, szUnixTime, 0, 8);
					
					/* 防呆，前面有發生(最後消費日期帶全0)導致轉換出超過100年，進而導致溢位影響其他資料 */
					if (srRTC_Out.uszYear < 100)
					{
						sprintf(pobTran->srTRec.srIPASSRec.szLastDeductDate, "20%02d%02d%02d",  srRTC_Out.uszYear, srRTC_Out.uszMonth, srRTC_Out.uszDay);
					}
				}
				
				/* 最後消費金額(55~56) */
				memcpy((char*)&pobTran->srTRec.srIPASSRec.lnLastDeductOneDayValue, &pobTran->srTRec.srIPASSRec.szCardInfo[55], 2);

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "UID [%s]", pobTran->srTRec.szUID);
					inLogPrintf(AT, szDebugMsg);

					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "票卡種類 [%02X], 身分別 [%02X], 自動加值 [%02X]",
							  pobTran->srTRec.srIPASSRec.szCardInfo[27],
							  pobTran->srTRec.srIPASSRec.szCardInfo[34],
							  pobTran->srTRec.srIPASSRec.szCardInfo[43]);
					inLogPrintf(AT, szDebugMsg);

					if (pobTran->srTRec.lnCardRemainAmount < 0)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "卡片餘額 [-%ld]", (0 - pobTran->srTRec.lnCardRemainAmount));
						inLogPrintf(AT, szDebugMsg);
					}
					else
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "卡片餘額 [%ld]", pobTran->srTRec.lnCardRemainAmount);
						inLogPrintf(AT, szDebugMsg);
					}

					/* 自動加值旗標 */
					if (pobTran->srTRec.srIPASSRec.szCardInfo[43] == 0x01)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "自動加值金額 [%ld]", pobTran->srTRec.lnTopUpAmount);
						inLogPrintf(AT, szDebugMsg);
					}
					
					/* 最後消費日期 */
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "最後消費日期 [%s]", pobTran->srTRec.srIPASSRec.szLastDeductDate);
					inLogPrintf(AT, szDebugMsg);
					
					/* 最後消費日累積金額 */
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "最後消費日累積金額 [%ld]", pobTran->srTRec.srIPASSRec.lnLastDeductOneDayValue);
					inLogPrintf(AT, szDebugMsg);
					
					if (pobTran->srTRec.srIPASSRec.uszCB_CardBit == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "是聯名卡");
						inLogPrintf(AT, szDebugMsg, _PRT_ISO_);
					}
				}
				if (ginISODebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "UID [%s]", pobTran->srTRec.szUID);
					inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);

					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "票卡種類 [%02X], 身分別 [%02X], 自動加值 [%02X]",
							  pobTran->srTRec.srIPASSRec.szCardInfo[27],
							  pobTran->srTRec.srIPASSRec.szCardInfo[34],
							  pobTran->srTRec.srIPASSRec.szCardInfo[43]);
					inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);

					if (pobTran->srTRec.lnCardRemainAmount < 0)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "卡片餘額 [-%ld]", (0 - pobTran->srTRec.lnCardRemainAmount));
						inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
					}
					else
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "卡片餘額 [%ld]", pobTran->srTRec.lnCardRemainAmount);
						inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
					}

					/* 自動加值旗標 */
					if (pobTran->srTRec.srIPASSRec.szCardInfo[43] == 0x01)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "自動加值金額 [%ld]", pobTran->srTRec.lnTopUpAmount);
						inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
					}
					
					/* 最後消費日期 */
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "最後消費日期 [%s]", pobTran->srTRec.srIPASSRec.szLastDeductDate);
					inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
					
					/* 最後消費日累積金額 */
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "最後消費日累積金額 [%ld]", pobTran->srTRec.srIPASSRec.lnLastDeductOneDayValue);
					inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
					
					if (pobTran->srTRec.srIPASSRec.uszCB_CardBit == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "是聯名卡");
						inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
					}
				}
			}

			/* DAVTI有資料 */
			if (pobTran->srTRec.srIPASSRec.szDAVTITxn[0] == 'D')
			{
				pobTran->srTRec.lnCountInvNum++;
				pobTran->srTRec.uszResponseBit = VS_TRUE;
				pobTran->uszTicketADVOnBit = VS_TRUE;
				inRetVal2 = inNCCC_Ticket_Insert_Advice_Ticket_Record(pobTran);
				if (inRetVal2 != VS_SUCCESS)
				{
					inFunc_EDCLock(AT);
				}
				else
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "InsertAdvice");
					}
					if (ginISODebug == VS_TRUE)
					{
						inPRINT_ChineseFont("InsertAdvice", _PRT_ISO_);
					}
				}
			}

		}
		else
		{
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetIntegrate_Device(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
			{

			}
			else
			{
				inNCCC_Ticket_Set_CTLS_Light(&pobTran->inCTLSLightStatus, _CTLS_LIGHT_RED_);
			}

			/* Timeout or Cancel = 0x0600 */
			if (inRetVal != IPASS_ERROR_NO_CARD)
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));
                                sprintf(szTemplate, "%04X", inRetVal);
				inIPASS_Display_Error(pobTran, szTemplate);
			}
			else
			{
				pobTran->srTRec.uszStopPollBit = VS_TRUE;
			}

			return (VS_ERROR);
		}
		
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inIPASS_First_Tap() END !");
			inLogPrintf(AT, "----------------------------------------");
		}
		
		return (VS_SUCCESS);
	}
}

/*
Function        :inIPASS_Second_Tap
Date&Time       :2018/1/11 上午 11:53
Describe        :
*/
int inIPASS_Second_Tap(TRANSACTION_OBJECT *pobTran)
{
#ifdef	_LOAD_KEY_AP_
	return (VS_SUCCESS);
#else
	int		inCount = 0;
	int		inRetVal = 0;	/* IPASS API 使用*/
	int		inRetVal2 = 0;
	char		szFuncEnable[2 + 1] = {0};
	char		szTemplate[20 + 1] = {0};
	char		szDebugMsg[1000 + 1] = {0};
	char		szDemoMode[2 + 1] = {0};
        unsigned char	uszInput[200] = {0}, uszOutput[500] = {0};
        unsigned int	inInput_Len = 0, inOutput_Len = 0;
	
	/* 教育訓練模式 */
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		inNCCC_Ticket_Set_CTLS_Light(&pobTran->inCTLSLightStatus, _CTLS_LIGHT_YELLOW_);

		inNCCC_Ticket_Trans_Success_Beep(pobTran);
		
		/* 購貨成功 */
		if (pobTran->srTRec.inCode == _TICKET_IPASS_DEDUCT_)
		{
			pobTran->uszDeductSuccessBit = VS_TRUE;
		}
		
		return (VS_SUCCESS);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "----------------------------------------");
			inLogPrintf(AT, "inIPASS_Second_Tap() START !");
		}
		
		if (ginFindRunTime == VS_TRUE)
		{
			inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
		}
			
		/* 感應中 */
		memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
		inGetIntegrate_Device(szFuncEnable);
		if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
		{

		}
		else
		{
			inNCCC_Ticket_Set_CTLS_Light(&pobTran->inCTLSLightStatus, _CTLS_LIGHT_YELLOW_);
		}

		memset(uszInput, 0x00, sizeof(uszInput));
		memset(uszOutput, 0x00, sizeof(uszOutput));

		inInput_Len = pobTran->srTRec.srIPASSRec.lnSign_Len;
		memcpy(&uszInput[0], &pobTran->srTRec.srIPASSRec.szSign_Data[0], inInput_Len);
		memset(pobTran->srTRec.srIPASSRec.szCardInfo, 0x00, sizeof(pobTran->srTRec.srIPASSRec.szCardInfo));

		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, " ");
			inLogPrintf(AT, "API_2 Input : ...");
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "InLen : %d", inInput_Len);
			inLogPrintf(AT, szDebugMsg);

			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			inFunc_BCD_to_ASCII(szDebugMsg, uszInput, inInput_Len);
			inLogPrintf(AT, szDebugMsg);
		}
		if (ginISODebug == VS_TRUE)
		{
			inPRINT_ChineseFont(" ", _PRT_ISO_);
			inPRINT_ChineseFont("API_2 Input : ...", _PRT_ISO_);
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "InLen : %d", inInput_Len);
			inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
		}


		/* 參考IPassMicroPayment.h API define */
		memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
		inGetIntegrate_Device(szFuncEnable);
		if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "V3不處理週邊設備");
				inLogPrintf(AT, szDebugMsg);
			}
		}
		else
		{
			switch (pobTran->srTRec.inCode)
			{
				case _TICKET_IPASS_QUERY_ :
				case _TICKET_IPASS_INQUIRY_ :
					inRetVal = IPassMP_ExecuteQueryTicket(uszInput, inInput_Len, uszOutput, &inOutput_Len);
					break;
				case _TICKET_IPASS_DEDUCT_ :
					inRetVal = IPassMP_ExecuteDeductValue(uszInput, inInput_Len, uszOutput, &inOutput_Len);
					break;
				case _TICKET_IPASS_REFUND_ :
				case _TICKET_IPASS_TOP_UP_ :
				case _TICKET_IPASS_AUTO_TOP_UP_ :
					inRetVal = IPassMP_ExecuteAddValue(uszInput, inInput_Len, uszOutput, &inOutput_Len);
					break;
				case _TICKET_IPASS_VOID_TOP_UP_ :
					inRetVal = IPassMP_ExecuteCancelAddValue(uszInput, inInput_Len, uszOutput, &inOutput_Len);
					break;
				default :
					inSetTicket_ReversalBit("Y");
					inSaveTDTRec(pobTran->srTRec.inTDTIndex);
					inNCCC_Ticket_Func_UpdatetTicketInvNum(pobTran);
					return (VS_ERROR);
			}
			
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "API2 End");
			}
			
			inNCCC_Ticket_Set_Msg(pobTran->srTRec.inTicketType, &pobTran->inTicketMsgStatus, _TICKET_MSG_IPASS_DO_NOT_MOVE_CARD_);

			if (ginISODebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "API Response Len2 [%d]", inOutput_Len);
				inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
			}

			if (inRetVal == IPASS_EXECUTE_OK		|| 
			    inRetVal == IPASS_ERROR_RECOVERY		|| 
			    inRetVal == IPASS_ERROR_AUTOLOAD_OFF	|| 
			    inRetVal == IPASS_ERROR_AUTOLOAD_ON		|| 
			    inRetVal == IPASS_ERROR_INVALID_AUTOADDVALUE|| 
			    inRetVal == IPASS_ERROR_BLACK_LIST)
			/* 0x0000 執行正確
			 * 0x0002 票值回覆
			 * 0x0003 關閉自動加值
			 * 0x0004 開啟自動加值
			 * 0x0517 黑名單鎖卡
			 * 0x0705 黑名單鎖卡 */
			{
				if (inRetVal == IPASS_ERROR_INVALID_AUTOADDVALUE || 
				    inRetVal == IPASS_ERROR_BLACK_LIST)
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "判斷為黑名單");
					}
					if (ginISODebug == VS_TRUE)
					{
						char	szDebugMsg[100 + 1] = {0};

						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "判斷為黑名單");
						inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
					}
					
					pobTran->srTRec.uszBlackListBit = VS_TRUE;
					memset(szTemplate, 0x00, sizeof(szTemplate));
					sprintf(szTemplate, "%04X", inRetVal);
					inIPASS_Display_Error(pobTran, szTemplate);
				}
				else if (inRetVal == IPASS_ERROR_AUTOLOAD_OFF)
				{
					pobTran->srTRec.uszCloseAutoTopUpBit = VS_TRUE;
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "關閉自動加值");
					}
					if (ginISODebug == VS_TRUE)
					{
						char	szDebugMsg[100 + 1] = {0};

						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "關閉自動加值");
						inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
					}
				}

				pobTran->srTRec.srIPASSRec.inStepNum = 2;

				/* 感應成功 */
				switch (pobTran->srTRec.inCode)
				{
					case _TICKET_IPASS_DEDUCT_ :
					case _TICKET_IPASS_REFUND_ :
					case _TICKET_IPASS_TOP_UP_ :
					case _TICKET_IPASS_AUTO_TOP_UP_ :
					case _TICKET_IPASS_VOID_TOP_UP_ :
						memset(pobTran->srTRec.srIPASSRec.szDAVTITxn, 0x00, sizeof(pobTran->srTRec.srIPASSRec.szDAVTITxn));
						pobTran->srTRec.srIPASSRec.lnSign_Len = 0;

						/* 275 + 136 */
						if (inOutput_Len == 411)
						{
							/* DAVTITxn (275) - 一般流程放在4(前面有詢卡可能會吐DAVTITxn) */
							memset(pobTran->srTRec.srIPASSRec.szDAVTITxn, 0x00, sizeof(pobTran->srTRec.srIPASSRec.szDAVTITxn));
							memcpy(&pobTran->srTRec.srIPASSRec.szDAVTITxn[0], &uszOutput[inCount], _IPASS_DAVTI_SIZE_);
							inCount += _IPASS_DAVTI_SIZE_;

							/* 簽章 (136) */
							pobTran->srTRec.srIPASSRec.lnSign_Len = _IPASS_SIGN_SIZE_136_BYTES_;
							memset(pobTran->srTRec.srIPASSRec.szSign_Data, 0x00, sizeof(pobTran->srTRec.srIPASSRec.szSign_Data));
							memcpy(&pobTran->srTRec.srIPASSRec.szSign_Data[0], &uszOutput[inCount], pobTran->srTRec.srIPASSRec.lnSign_Len);
							inCount += _IPASS_SIGN_SIZE_136_BYTES_;
						}
						else if (inOutput_Len == 339)
						{
							/* DAVTITxn (275) */
							memset(pobTran->srTRec.srIPASSRec.szDAVTITxn, 0x00, sizeof(pobTran->srTRec.srIPASSRec.szDAVTITxn));
							memcpy(&pobTran->srTRec.srIPASSRec.szDAVTITxn[0], &uszOutput[inCount], _IPASS_DAVTI_SIZE_);
							inCount += _IPASS_DAVTI_SIZE_;

							/* 簽章 (64) - 黑名單鎖卡回的Size，要回給主機 */
							pobTran->srTRec.srIPASSRec.lnSign_Len = _IPASS_SIGN_SIZE_64_BYTES_;
							memset(pobTran->srTRec.srIPASSRec.szSign_Data, 0x00, sizeof(pobTran->srTRec.srIPASSRec.szSign_Data));
							memcpy(&pobTran->srTRec.srIPASSRec.szSign_Data[0], &uszOutput[inCount], pobTran->srTRec.srIPASSRec.lnSign_Len);
							inCount += _IPASS_SIGN_SIZE_64_BYTES_;
						}
						else
						{
							inSetTicket_ReversalBit("Y");
							inSaveTDTRec(pobTran->srTRec.inTDTIndex);
							inNCCC_Ticket_Func_UpdatetTicketInvNum(pobTran);
							return (VS_ERROR);
						}

						/* iPassTSN 卡片交易序號  */
						memset(szTemplate, 0x00, sizeof(szTemplate));
						memcpy(szTemplate, &pobTran->srTRec.srIPASSRec.szDAVTITxn[45], 6);
						pobTran->srTRec.srIPASSRec.lnCardInvNum = atol(szTemplate);

						break;
					case _TICKET_IPASS_QUERY_ :
					case _TICKET_IPASS_INQUIRY_ :
						memset(pobTran->srTRec.srIPASSRec.szDAVTITxn, 0x00, sizeof(pobTran->srTRec.srIPASSRec.szDAVTITxn));
						pobTran->srTRec.srIPASSRec.lnSign_Len = 0;

						if (inOutput_Len == 91)
						{
							/* 卡片內容結構 (91) */
							memset(pobTran->srTRec.srIPASSRec.szCardInfo, 0x00, sizeof(pobTran->srTRec.srIPASSRec.szCardInfo));
							memcpy(&pobTran->srTRec.srIPASSRec.szCardInfo[0], &uszOutput[inCount], _IPASS_CARD_DATA_SIZE_);
							inCount += _IPASS_CARD_DATA_SIZE_;
						}
						else if (inOutput_Len == 275)
						{
							/* DAVTITxn (275) - 黑名單 */
							memset(pobTran->srTRec.srIPASSRec.szDAVTITxn, 0x00, sizeof(pobTran->srTRec.srIPASSRec.szDAVTITxn));
							memcpy(&pobTran->srTRec.srIPASSRec.szDAVTITxn[0], &uszOutput[inCount], _IPASS_DAVTI_SIZE_);
							inCount += _IPASS_DAVTI_SIZE_;
							
							if (ginDebug == VS_TRUE)
							{
								inLogPrintf(AT, "判斷為黑名單");
							}
							if (ginISODebug == VS_TRUE)
							{
								char	szDebugMsg[100 + 1] = {0};

								memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
								sprintf(szDebugMsg, "判斷為黑名單");
								inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
							}
							
							pobTran->srTRec.uszBlackListBit = VS_TRUE;
						}
						/* 275 + 91 */
						else if (inOutput_Len == 366)
						{
							/* DAVTITxn (275) */
							memset(pobTran->srTRec.srIPASSRec.szDAVTITxn, 0x00, sizeof(pobTran->srTRec.srIPASSRec.szDAVTITxn));
							memcpy(&pobTran->srTRec.srIPASSRec.szDAVTITxn[0], &uszOutput[inCount], _IPASS_DAVTI_SIZE_);
							inCount += _IPASS_DAVTI_SIZE_;

							/* 卡片內容結構 (91) */
							memset(pobTran->srTRec.srIPASSRec.szCardInfo, 0x00, sizeof(pobTran->srTRec.srIPASSRec.szCardInfo));
							memcpy(&pobTran->srTRec.srIPASSRec.szCardInfo[0], &uszOutput[inCount], _IPASS_CARD_DATA_SIZE_);
							inCount += _IPASS_CARD_DATA_SIZE_;
						}
						else
						{
							inSetTicket_ReversalBit("Y");
							inSaveTDTRec(pobTran->srTRec.inTDTIndex);
							inNCCC_Ticket_Func_UpdatetTicketInvNum(pobTran);
							return (VS_ERROR);
						}

						break;
					default :
						inSetTicket_ReversalBit("Y");
						inSaveTDTRec(pobTran->srTRec.inTDTIndex);
						inNCCC_Ticket_Func_UpdatetTicketInvNum(pobTran);
						return (VS_ERROR);
				}

				if (pobTran->srTRec.srIPASSRec.szDAVTITxn[0] == 'D')
				{
					if (pobTran->srTRec.inCode == _TICKET_IPASS_QUERY_	|| 
					    pobTran->srTRec.inCode == _TICKET_IPASS_INQUIRY_	||
					    pobTran->srTRec.uszBlackListBit == VS_TRUE)
					{
						pobTran->srTRec.lnCountInvNum++;
					}
					else
					{
						pobTran->srTRec.uszConfirmBit = VS_TRUE;
					}

					pobTran->srTRec.uszResponseBit = VS_FALSE;
					pobTran->uszTicketADVOnBit = VS_TRUE;
					inRetVal2 = inNCCC_Ticket_Insert_Advice_Ticket_Record(pobTran);
					if (inRetVal2 != VS_SUCCESS)
					{
						inFunc_EDCLock(AT);
					}
					else
					{
						if (ginDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
							sprintf(szDebugMsg, "InsertAdvice Inv:%ld", pobTran->srTRec.lnCountInvNum);
							inLogPrintf(AT, szDebugMsg);
						}
						if (ginISODebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
							sprintf(szDebugMsg, "InsertAdvice Inv:%ld", pobTran->srTRec.lnCountInvNum);
							inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
						}
					}

					if (pobTran->srTRec.uszBlackListBit == VS_TRUE)
					{
						inNCCC_Ticket_Func_UpdatetTicketInvNum(pobTran);
					}
				}

				if (pobTran->srTRec.uszBlackListBit == VS_TRUE)
				{
					
				}
				else
				{
					inNCCC_Ticket_Trans_Success_Beep(pobTran);
					/* 購貨成功 */
					if (pobTran->srTRec.inCode == _TICKET_IPASS_DEDUCT_)
					{
						pobTran->uszDeductSuccessBit = VS_TRUE;
					}
				}
			}
			else
			{
				inSetTicket_ReversalBit("Y");
				inSaveTDTRec(pobTran->srTRec.inTDTIndex);
				inNCCC_Ticket_Func_UpdatetTicketInvNum(pobTran);
				
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);

				memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
				inGetIntegrate_Device(szFuncEnable);
				if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
				{

				}
				else
				{
					inNCCC_Ticket_Set_CTLS_Light(&pobTran->inCTLSLightStatus, _CTLS_LIGHT_RED_);
				}

				/* Timeout or Cancel = 0x0600 */
				if (inRetVal != IPASS_ERROR_NO_CARD)
				{
					memset(szTemplate, 0x00, sizeof(szTemplate));
					sprintf(szTemplate, "%04X", inRetVal);
					inIPASS_Display_Error(pobTran, szTemplate);
				}
				else
				{
					memset(szTemplate, 0x00, sizeof(szTemplate));
					sprintf(szTemplate, "%04X", inRetVal);
					inIPASS_Display_Error(pobTran, szTemplate);
					pobTran->srTRec.uszStopPollBit = VS_TRUE;
				}

				return (VS_ERROR);
			}
		}
		
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inIPASS_Second_Tap() END !");
			inLogPrintf(AT, "----------------------------------------");
		}
		
		return (VS_SUCCESS);
	}
	
#endif
}

/*
Function        :inIPASS_Init_Command_Packet
Date&Time       :2018/1/2 下午 3:48
Describe        :理論長度33
*/
int inIPASS_Init_Command_Packet(unsigned char *uszSendBuffer)
{
	int		inAIIIndex = 0, inTermAmount = 0, inTermIndex = 0;
	int		inESVCIndex =  -1;
        char		szTemplate[10 + 1] = {0}, szIP[16 + 1] = {0};
	char		szDecimalUnixTime[10 + 1];
	unsigned int	inLen = 0;
	RTC_NEXSYS	srRTC;

        /* Load HDT */
        if (inLoadTDTRec(_TDT_INDEX_00_IPASS_) != VS_SUCCESS)
		return (VS_ERROR);
	
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTicket_HostIndex(szTemplate);
	inESVCIndex = atoi(szTemplate);

	if (inLoadHDTRec(inESVCIndex) != VS_SUCCESS)
		return (VS_ERROR);

	if (inLoadIPASSDTRec(0) != VS_SUCCESS)
		return (VS_ERROR);

        /* 1.EDC IP */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        memset(szIP, 0x00, sizeof(szIP));
	inGetTermIPAddress(szIP);
	
	/* inTermAmount 為有幾個區段 inTermIndex為區段中第幾個字元 */
	inAIIIndex = 0;
        inTermAmount = 0;
        inTermIndex = 0;

	do
	{
		if (szIP[inAIIIndex] == '.' || szIP[inAIIIndex] == 0x00)
		{
			 uszSendBuffer[inLen] = atoi(szTemplate);
			 inLen++;
			 
			 /* 下一區段IP */
			 inTermAmount++;
			 /* 區段index歸0 */
			 inTermIndex = 0;
			 /* 陣列清空 */
			 memset(szTemplate, 0x00, sizeof(szTemplate));
			 
			 if (szIP[inAIIIndex] == 0x00)
			 {
				 break;
			 }
		}
		else
		{
			memcpy(&szTemplate[inTermIndex], &szIP[inAIIIndex], 1);
			inTermIndex++;
		}
		
		/* 換下一個index */
		inAIIIndex++;
		
	} while(inAIIIndex <= 16);
	/* 跳出條件 遇到結尾0x00 或 超過IP最大長度 */

        /* 2.ADD Host IP */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        memset(szIP, 0x00, sizeof(szIP));
	inGetIPASS_AMS_TMS_IP_Address(szIP);
	
	inAIIIndex = 0;
        inTermAmount = 0;
        inTermIndex = 0;

        do
	{
		if (szIP[inAIIIndex] == '.' || szIP[inAIIIndex] == 0x00)
		{
			 uszSendBuffer[inLen] = atoi(szTemplate);
			 inLen++;
			 
			 /* 下一區段IP */
			 inTermAmount++;
			 /* 區段index歸0 */
			 inTermIndex = 0;
			 /* 陣列清空 */
			 memset(szTemplate, 0x00, sizeof(szTemplate));
			 
			 if (szIP[inAIIIndex] == 0x00)
			 {
				 break;
			 }
		}
		else
		{
			memcpy(&szTemplate[inTermIndex], &szIP[inAIIIndex], 1);
			inTermIndex++;
		}
		
		/* 換下一個index */
		inAIIIndex++;
		
	} while(inAIIIndex <= 16);
	/* 跳出條件 遇到結尾0x00 或 超過IP最大長度 */

        /* 3.Bank Host IP */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        memset(szIP, 0x00, sizeof(szIP));
        inGetIPASS_BMS_IP_Address(szIP);
	
	inAIIIndex = 0;
        inTermAmount = 0;
        inTermIndex = 0;

        do
	{
		if (szIP[inAIIIndex] == '.' || szIP[inAIIIndex] == 0x00)
		{
			 uszSendBuffer[inLen] = atoi(szTemplate);
			 inLen++;
			 
			 /* 下一區段IP */
			 inTermAmount++;
			 /* 區段index歸0 */
			 inTermIndex = 0;
			 /* 陣列清空 */
			 memset(szTemplate, 0x00, sizeof(szTemplate));
			 
			 if (szIP[inAIIIndex] == 0x00)
			 {
				 break;
			 }
		}
		else
		{
			memcpy(&szTemplate[inTermIndex], &szIP[inAIIIndex], 1);
			inTermIndex++;
		}
		
		/* 換下一個index */
		inAIIIndex++;
		
	} while(inAIIIndex <= 16);
	/* 跳出條件 遇到結尾0x00 或 超過IP最大長度 */

	/* 以下e共長度19 */
	
        /* 4.系統代碼 */
        memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetIPASS_System_ID(szTemplate);
        inFunc_ASCII_to_BCD(&uszSendBuffer[inLen], szTemplate, 1);
        inLen++;

        /* 5.業者代碼 */
        memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetIPASS_SP_ID(szTemplate);
        inFunc_ASCII_to_BCD(&uszSendBuffer[inLen], szTemplate, 1);
        inLen++;

        /* 6.店舖編號 */
        memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetIPASS_Shop_ID(szTemplate);
        inFunc_ASCII_to_BCD(&uszSendBuffer[inLen], szTemplate, 3);
        inLen += 3;

        /* 7.POS編號 */
        memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetIPASS_POS_ID(szTemplate);
        inFunc_ASCII_to_BCD(&uszSendBuffer[inLen], szTemplate, 1);
        inLen++;

        /* 8.POS交易編號 */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        sprintf(szTemplate, "%s", "0000000000");
        inFunc_ASCII_to_BCD(&uszSendBuffer[inLen], szTemplate, 5);
        inLen += 5;

        /* 9.GMT (Unix Time, Little Endian) */
	inFunc_GetSystemDateAndTime(&srRTC);
	memset(szDecimalUnixTime, 0x00, sizeof(szDecimalUnixTime));
	inFuncGetTimeToUnix(&srRTC, szDecimalUnixTime, 8, 0);
	
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inFunc_String_Dec_to_Hex_Little2Little(szDecimalUnixTime, szTemplate);
        inFunc_ASCII_to_BCD(&uszSendBuffer[inLen], szTemplate, 4);
        inLen += 4;

        /* 10.加密模式 0x00 : DES for AMS
                       0x01 : 3DES for TMS */
	/* hard code */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        sprintf(szTemplate, "%s", "01");
        inFunc_ASCII_to_BCD(&uszSendBuffer[inLen], szTemplate, 1);
        inLen++;

        /* 11.次業者代碼 */
        memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetIPASS_Sub_Company_ID(szTemplate);
        inFunc_ASCII_to_BCD(&uszSendBuffer[inLen], szTemplate, 2);
        inLen += 2;

        /* 12.交易設備類別 */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        sprintf(szTemplate, "%s", "23");
        inFunc_ASCII_to_BCD(&uszSendBuffer[inLen], szTemplate, 1);
        inLen++;
	
	/* 13.SAM Slot */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTicket_SAM_Slot(szTemplate);
        inFunc_ASCII_to_BCD(&uszSendBuffer[inLen], szTemplate, 1);
        inLen++;
	
	/* 14.執行模式設定 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
        sprintf(szTemplate, "%s", "01");
        inFunc_ASCII_to_BCD(&uszSendBuffer[inLen], szTemplate, 1);
        inLen++;
	
	
        return (inLen);
}

/*
Function        :inIPASS_Init_Register_Packet
Date&Time       :2018/1/2 下午 3:48
Describe        :理論長度70
*/
int inIPASS_Init_Register_Packet(unsigned char *uszSendBuffer)
{
	int		inAIIIndex = 0, inTermAmount = 0, inTermIndex = 0;
	int		inESVCIndex =  -1;
        char		szTemplate[10 + 1] = {0}, szIP[16 + 1] = {0};
	char		szDecimalUnixTime[10 + 1];
	unsigned int	inLen = 0;
	RTC_NEXSYS	srRTC;

        /* Load HDT */
        if (inLoadTDTRec(_TDT_INDEX_00_IPASS_) != VS_SUCCESS)
		return (VS_ERROR);
	
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTicket_HostIndex(szTemplate);
	inESVCIndex = atoi(szTemplate);

	if (inLoadHDTRec(inESVCIndex) != VS_SUCCESS)
		return (VS_ERROR);

	if (inLoadIPASSDTRec(0) != VS_SUCCESS)
		return (VS_ERROR);

        /* 1.EDC IP */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        memset(szIP, 0x00, sizeof(szIP));
	inGetTermIPAddress(szIP);
	
	/* inTermAmount 為有幾個區段 inTermIndex為區段中第幾個字元 */
	inAIIIndex = 0;
        inTermAmount = 0;
        inTermIndex = 0;

	do
	{
		if (szIP[inAIIIndex] == '.' || szIP[inAIIIndex] == 0x00)
		{
			 uszSendBuffer[inLen] = atoi(szTemplate);
			 inLen++;
			 
			 /* 下一區段IP */
			 inTermAmount++;
			 /* 區段index歸0 */
			 inTermIndex = 0;
			 /* 陣列清空 */
			 memset(szTemplate, 0x00, sizeof(szTemplate));
			 
			 if (szIP[inAIIIndex] == 0x00)
			 {
				 break;
			 }
		}
		else
		{
			memcpy(&szTemplate[inTermIndex], &szIP[inAIIIndex], 1);
			inTermIndex++;
		}
		
		/* 換下一個index */
		inAIIIndex++;
		
	} while(inAIIIndex <= 16);
	/* 跳出條件 遇到結尾0x00 或 超過IP最大長度 */

        /* 2.ADD Host IP */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        memset(szIP, 0x00, sizeof(szIP));
	inGetIPASS_AMS_TMS_IP_Address(szIP);
	
	inAIIIndex = 0;
        inTermAmount = 0;
        inTermIndex = 0;

        do
	{
		if (szIP[inAIIIndex] == '.' || szIP[inAIIIndex] == 0x00)
		{
			 uszSendBuffer[inLen] = atoi(szTemplate);
			 inLen++;
			 
			 /* 下一區段IP */
			 inTermAmount++;
			 /* 區段index歸0 */
			 inTermIndex = 0;
			 /* 陣列清空 */
			 memset(szTemplate, 0x00, sizeof(szTemplate));
			 
			 if (szIP[inAIIIndex] == 0x00)
			 {
				 break;
			 }
		}
		else
		{
			memcpy(&szTemplate[inTermIndex], &szIP[inAIIIndex], 1);
			inTermIndex++;
		}
		
		/* 換下一個index */
		inAIIIndex++;
		
	} while(inAIIIndex <= 16);
	/* 跳出條件 遇到結尾0x00 或 超過IP最大長度 */

        /* 3.Bank Host IP */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        memset(szIP, 0x00, sizeof(szIP));
        inGetIPASS_BMS_IP_Address(szIP);
	
	inAIIIndex = 0;
        inTermAmount = 0;
        inTermIndex = 0;

        do
	{
		if (szIP[inAIIIndex] == '.' || szIP[inAIIIndex] == 0x00)
		{
			 uszSendBuffer[inLen] = atoi(szTemplate);
			 inLen++;
			 
			 /* 下一區段IP */
			 inTermAmount++;
			 /* 區段index歸0 */
			 inTermIndex = 0;
			 /* 陣列清空 */
			 memset(szTemplate, 0x00, sizeof(szTemplate));
			 
			 if (szIP[inAIIIndex] == 0x00)
			 {
				 break;
			 }
		}
		else
		{
			memcpy(&szTemplate[inTermIndex], &szIP[inAIIIndex], 1);
			inTermIndex++;
		}
		
		/* 換下一個index */
		inAIIIndex++;
		
	} while(inAIIIndex <= 16);
	/* 跳出條件 遇到結尾0x00 或 超過IP最大長度 */

	/* 以下共長度58 */
	
        /* 4.系統代碼 */
        memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetIPASS_System_ID(szTemplate);
        inFunc_ASCII_to_BCD(&uszSendBuffer[inLen], szTemplate, 1);
        inLen++;

        /* 5.業者代碼 */
        memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetIPASS_SP_ID(szTemplate);
        inFunc_ASCII_to_BCD(&uszSendBuffer[inLen], szTemplate, 1);
        inLen++;

        /* 6.店舖編號 */
        memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetIPASS_Shop_ID(szTemplate);
        inFunc_ASCII_to_BCD(&uszSendBuffer[inLen], szTemplate, 3);
        inLen += 3;

        /* 7.POS編號 */
        memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetIPASS_POS_ID(szTemplate);
        inFunc_ASCII_to_BCD(&uszSendBuffer[inLen], szTemplate, 1);
        inLen++;

	/* 8.加密模式 0x00 : DES for AMS
                       0x01 : 3DES for TMS */
	/* hard code */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        sprintf(szTemplate, "%s", "01");
        inFunc_ASCII_to_BCD(&uszSendBuffer[inLen], szTemplate, 1);
        inLen++;

        /* 9.次業者代碼 */
        memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetIPASS_Sub_Company_ID(szTemplate);
        inFunc_ASCII_to_BCD(&uszSendBuffer[inLen], szTemplate, 2);
        inLen += 2;
	
        /* 10.POS交易編號 */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        sprintf(szTemplate, "%s", "0000000000");
        inFunc_ASCII_to_BCD(&uszSendBuffer[inLen], szTemplate, 5);
        inLen += 5;

        /* 11.GMT (Unix Time, Little Endian) */
	inFunc_GetSystemDateAndTime(&srRTC);
	memset(szDecimalUnixTime, 0x00, sizeof(szDecimalUnixTime));
	inFuncGetTimeToUnix(&srRTC, szDecimalUnixTime, 8, 0);
	
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inFunc_String_Dec_to_Hex_Little2Little(szDecimalUnixTime, szTemplate);
        inFunc_ASCII_to_BCD(&uszSendBuffer[inLen], szTemplate, 4);
        inLen += 4;
	
	/* 12.other */
	memset(szTemplate, 0x00, sizeof(szTemplate));
        sprintf(szTemplate, "%s", "0123456789");
        memcpy(&uszSendBuffer[inLen], szTemplate, 10);
        inLen += 10;
	
	memset(szTemplate, 0x00, sizeof(szTemplate));
        sprintf(szTemplate, "%s", "0123456789");
        memcpy(&uszSendBuffer[inLen], szTemplate, 10);
        inLen += 10;
	
	memset(szTemplate, 0x00, sizeof(szTemplate));
        sprintf(szTemplate, "%s", "0123456789");
        memcpy(&uszSendBuffer[inLen], szTemplate, 10);
        inLen += 10;
	
	memset(szTemplate, 0x00, sizeof(szTemplate));
        sprintf(szTemplate, "%s", "0123456789");
        memcpy(&uszSendBuffer[inLen], szTemplate, 10);
        inLen += 10;
	
        return (inLen);
}

/*
Function        :inIPASS_Command_Packet
Date&Time       :2017/12/22 上午 10:43
Describe        :回傳組出命令長度，長度代表錯誤
*/
int inIPASS_Command_Packet(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendBuffer)
{
	int	inLen = 0;
	char    szTemplate[20 + 1];
	char    szTempBuffer[20 + 1];
	char	szDecimalUnixTime[10 + 1];
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inIPASS_Command_Packet() START !");
	}

        if (pobTran->srTRec.inCode == _TICKET_IPASS_DEDUCT_)
        {
                /* 1.交易序號 */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szTemplate, "%010ld", pobTran->srTRec.lnInvNum);
                inFunc_ASCII_to_BCD((unsigned char *)&uszSendBuffer[inLen], szTemplate, 5);
                inLen += 5;

                /* 2.GMT (Unix Time, Little Endian) */
		/* 一卡通用little endian， 虹堡也用little endian，所以不用特別轉 */
		memset(szDecimalUnixTime, 0x00, sizeof(szDecimalUnixTime));
		sprintf(szDecimalUnixTime, "%lu", pobTran->srTRec.srIPASSRec.lnUnixTime);
		
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inFunc_String_Dec_to_Hex_Little2Little(szDecimalUnixTime, szTemplate);
		
                inFunc_ASCII_to_BCD(&uszSendBuffer[inLen], szTemplate, 4);
                inLen += 4;

                /* 3.AMT (Little Endian) */
                memset(szTempBuffer, 0x00, sizeof(szTempBuffer));
                sprintf(szTempBuffer, "%ld", pobTran->srTRec.lnTxnAmount);
		
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inFunc_String_Dec_to_Hex_Little2Little(szTempBuffer, szTemplate);
		
                inFunc_ASCII_to_BCD(&uszSendBuffer[inLen], szTemplate, 2);
                inLen += 2;
        }
        else if (pobTran->srTRec.inCode == _TICKET_IPASS_REFUND_	||
                 pobTran->srTRec.inCode == _TICKET_IPASS_TOP_UP_	||
                 pobTran->srTRec.inCode == _TICKET_IPASS_AUTO_TOP_UP_)
        {
                /* 1.Transaction Type(inADDType)
                     0x42 : 自動加值
                     0x89 : 退貨加值
                     0x30 : 手動加值 */
                memset(szTemplate, 0x00, sizeof(szTemplate));

                if (pobTran->srTRec.inCode == _TICKET_IPASS_AUTO_TOP_UP_)
                        sprintf(szTemplate, "%s", "42");
                else if (pobTran->srTRec.inCode == _TICKET_IPASS_REFUND_)
                        sprintf(szTemplate, "%s", "89");
                else
                        sprintf(szTemplate, "%s", "30");

                inFunc_ASCII_to_BCD(&uszSendBuffer[inLen], szTemplate, 1);
                inLen ++;

                /* 2.交易序號 */
                memset(szTemplate, 0x00, sizeof(szTemplate));

                if (pobTran->srTRec.inCode == _TICKET_IPASS_REFUND_)
                        sprintf(szTemplate, "%10ld", pobTran->srTRec.srIPASSRec.lnCardInvNum);
                else
                        sprintf(szTemplate, "%10ld", pobTran->srTRec.lnInvNum);

                inFunc_ASCII_to_BCD(&uszSendBuffer[inLen], &szTemplate[0], 5);
                inLen += 5;

                /* 3.GMT (Unix Time, Little Endian) */
                memset(szDecimalUnixTime, 0x00, sizeof(szDecimalUnixTime));
		sprintf(szDecimalUnixTime, "%lu", pobTran->srTRec.srIPASSRec.lnUnixTime);
		
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inFunc_String_Dec_to_Hex_Little2Little(szDecimalUnixTime, szTemplate);

		/* 一卡通用little endian， 虹堡也用little endian，所以不用特別轉 */
                inFunc_ASCII_to_BCD(&uszSendBuffer[inLen], szTemplate, 4);
                inLen += 4;

                /* 4.Amount */
                memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(szTempBuffer, 0x00, sizeof(szTempBuffer));
		
                if (pobTran->srTRec.inCode == _TICKET_IPASS_AUTO_TOP_UP_)
                        sprintf(szTempBuffer, "%ld", pobTran->srTRec.lnTotalTopUpAmount);
                else
                        sprintf(szTempBuffer, "%ld", pobTran->srTRec.lnTxnAmount);
		
		inFunc_String_Dec_to_Hex_Little2Little(szTempBuffer, szTemplate);

                inFunc_ASCII_to_BCD(&uszSendBuffer[inLen], szTemplate, 2);
                inLen += 2;
        }
        else if (pobTran->srTRec.inCode == _TICKET_IPASS_QUERY_		||
                 pobTran->srTRec.inCode == _TICKET_IPASS_INQUIRY_)
        {
                /* 1.Transaction Type(inADDType)
                     0x00 : 讀取卡號
                     0x01 : 讀取票卡基本資料
                     0x02 : 讀取票卡最近一筆加值與六筆扣值  */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szTemplate, "%s", "01");
                inFunc_ASCII_to_BCD((unsigned char *)&uszSendBuffer[inLen], szTemplate, 1);
                inLen ++;

                /* 2.GMT (Unix Time, Little Endian) */
                memset(szDecimalUnixTime, 0x00, sizeof(szDecimalUnixTime));
		sprintf(szDecimalUnixTime, "%lu", pobTran->srTRec.srIPASSRec.lnUnixTime);
		
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inFunc_String_Dec_to_Hex_Little2Little(szDecimalUnixTime, szTemplate);

                inFunc_ASCII_to_BCD((unsigned char *)&uszSendBuffer[inLen], szTemplate, 4);
                inLen += 4;
        }
        else if (pobTran->srTRec.inCode == _TICKET_IPASS_VOID_TOP_UP_)
        {
                /* 1.交易序號 */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szTemplate, "%10ld", pobTran->srTRec.srIPASSRec.lnCardInvNum);
                inFunc_ASCII_to_BCD((unsigned char *)&uszSendBuffer[inLen], szTemplate, 5);
                inLen += 5;

                /* 2.GMT (Unix Time, Little Endian) */
                memset(szDecimalUnixTime, 0x00, sizeof(szDecimalUnixTime));
		sprintf(szDecimalUnixTime, "%lu", pobTran->srTRec.srIPASSRec.lnUnixTime);
		
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inFunc_String_Dec_to_Hex_Little2Little(szDecimalUnixTime, szTemplate);

		inFunc_ASCII_to_BCD((unsigned char *)&uszSendBuffer[inLen], szTemplate, 4);
                inLen += 4;
        }
        else
        {
                inLen = 0;
        }
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inIPASS_Command_Packet() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inLen);
}

/*
Function        :inIPASS_Display_Error
Date&Time       :2018/9/6 下午 3:59
Describe        :
*/
int inIPASS_Display_Error(TRANSACTION_OBJECT *pobTran, char *szError)
{
        int     inCustomer = 0;
        char    szLine1[33] = {0}, szLine2[33] = {0}, szLine3[33] = {0};
	char	szFuncEnable[2 + 1] = {0};
        char	szCustomerIndicator[3 + 1] = {0};
	
	memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
	inGetIntegrate_Device(szFuncEnable);
	if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
	{

	}
	else
	{
		inNCCC_Ticket_Set_CTLS_Light(&pobTran->inCTLSLightStatus, _CTLS_LIGHT_RED_);
	}
        
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
        inCustomer = atoi(szCustomerIndicator);
	
        memset(szLine2, 0x00, sizeof(szLine2));
        memset(szLine3, 0x00, sizeof(szLine3));
        memset(pobTran->szTicket_ErrorCode, 0x00, sizeof(pobTran->szTicket_ErrorCode));

        if (!memcmp(szError, "01", 2))
        {
                sprintf(szLine3, "%s", "初始化失敗請報修");
                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
        }
        else if (!memcmp(szError, "02", 2) || !memcmp(szError, "03", 2))
        {
                /* 客製化098，0010以上的回應碼都與標準版少1 */
                switch(inCustomer)
                {
                        case 98:
                        case 105:
                                if (!memcmp(szError, "0200", 4) || !memcmp(szError, "0300", 4))
                                {
                                        sprintf(szLine2, "%s", "讀取寫入票卡異常");
                                        sprintf(szLine3, "%s", "請勿移動票卡");
                                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0004");
                                }
                                else
                                {
                                        sprintf(szLine3, "%s", "交易失敗");
                                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                                }
                                break;
                        default:
                                sprintf(szLine2, "%s", "讀取寫入票卡異常");
                                sprintf(szLine3, "%s", "請勿移動票卡");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0004");
                                break;
                }
        }
        else if (!memcmp(szError, "04", 2))
        {
                switch(inCustomer)
                {
                        case 98:
                        case 105:
                                if (!memcmp(szError, "0400", 4))
                                {
                                        sprintf(szLine2, "%s", "讀取寫入票卡異常");
                                        sprintf(szLine3, "%s", "請勿移動票卡");
                                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0004");
                                }
                                else if (!memcmp(szError, "0410", 4) || !memcmp(szError, "0411", 4))
                                {
                                        sprintf(szLine3, "%s", "票卡自動加值失敗");
                                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                                }
                                else
                                {
                                        sprintf(szLine3, "%s", "交易失敗");
                                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                                }
                                break;
                        default:
                                if (!memcmp(szError, "040", 3))
                                {
                                        sprintf(szLine2, "%s", "讀取寫入票卡異常");
                                        sprintf(szLine3, "%s", "請勿移動票卡");
                                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0004");
                                }
                                else if (!memcmp(szError, "0410", 4) || !memcmp(szError, "0411", 4))
                                {
                                        sprintf(szLine3, "%s", "票卡自動加值失敗");
                                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                                }
                                else
                                {
                                        sprintf(szLine3, "%s", "交易失敗");
                                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                                }
                                break;
                }
        }
        else if (!memcmp(szError, "05", 2))
        {
                if (!memcmp(szError, "0501", 4) || !memcmp(szError, "0502", 4) ||
                    !memcmp(szError, "0504", 4) || !memcmp(szError, "0505", 4) || !memcmp(szError, "0506", 4) || !memcmp(szError, "0507", 4))
                {
                        sprintf(szLine3, "%s", "票卡異常洽一卡通");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                }
                else if (!memcmp(szError, "0503", 4))
                {
                        switch(inCustomer)
                        {
                                case 98:
                                case 105:
                                        sprintf(szLine3, "%s", "票卡已鎖洽一卡通");
                                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0012");
                                        break;
                                default:
                                        sprintf(szLine3, "%s", "票卡異常洽一卡通");
                                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                                        break;
                        }
                }    
                else if (!memcmp(szError, "0508", 4))
                {
                        sprintf(szLine2, "%s", "非儲值型票卡");
                        sprintf(szLine3, "%s", "無法進行交易");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                }
                else if (!memcmp(szError, "0509", 4))
                {
                        switch(inCustomer)
                        {
                                case 98:
                                case 105:
                                        sprintf(szLine2, "%s", "單筆交易金額");
                                        sprintf(szLine3, "%s", "超過上限");
                                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0011");
                                        break;
                                default:
                                        sprintf(szLine2, "%s", "單筆交易金額");
                                        sprintf(szLine3, "%s", "超過上限");
                                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0012");
                                        break;
                        }
                }
                else if (!memcmp(szError, "0510", 4))
                {
                        switch(inCustomer)
                        {
                                case 98:
                                case 105:
                                        sprintf(szLine3, "%s", "票卡無法自動加值");
                                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0014");
                                        break;
                                default:
                                        sprintf(szLine3, "%s", "票卡無法自動加值");
                                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0015");
                                        break;
                        }
                }
                else if (!memcmp(szError, "0511", 4))
                {
                        switch(inCustomer)
                        {
                                case 98:
                                case 105:
                                        sprintf(szLine2, "%s", "當日累計交易金額");
                                        sprintf(szLine3, "%s", "超過上限");
                                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0011");
                                        break;
                                default:
                                        sprintf(szLine2, "%s", "當日累計交易金額");
                                        sprintf(szLine3, "%s", "超過上限");
                                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0012");
                                        break;
                        }
                }
                else if (!memcmp(szError, "0512", 4))
                {
                        switch(inCustomer)
                        {
                                case 98:
                                case 105:
                                        sprintf(szLine2, "%s", "使用非原加值票卡");
                                        sprintf(szLine3, "%s", "進行取消加值交易");
                                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0013");
                                        break;
                                default:
                                        sprintf(szLine2, "%s", "使用非原加值票卡");
                                        sprintf(szLine3, "%s", "進行取消加值交易");
                                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0014");
                                        break;
                        }
                }
                else if (!memcmp(szError, "0513", 4) || !memcmp(szError, "0514", 4))
                {
                        sprintf(szLine3, "%s", "交易失敗，請重試");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                }
                else if (!memcmp(szError, "0515", 4) || !memcmp(szError, "0516", 4))
                {
                        sprintf(szLine2, "%s", "交易過程");
                        sprintf(szLine3, "%s", "勿抽換不同票卡");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0004");
                }
                else if (!memcmp(szError, "0517", 4))
                {
                        switch(inCustomer)
                        {
                                case 98:
                                case 105:
                                        sprintf(szLine3, "%s", "票卡已鎖洽一卡通");
                                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0012");
                                        break;
                                default:
                                        sprintf(szLine3, "%s", "票卡已鎖洽一卡通");
                                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0013");
                                        break;
                        }
                }
                else if (!memcmp(szError, "0518", 4))
                {
                        switch(inCustomer)
                        {
                                case 98:
                                case 105:
                                        sprintf(szLine3, "%s", "票卡餘額不足");
                                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0015");
                                        break;
                                default:
                                        sprintf(szLine3, "%s", "票卡餘額不足");
                                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0016");
                                        break;
                        }
                }
                else if (!memcmp(szError, "0523", 4))
                {
                        switch(inCustomer)
                        {
                                case 98:
                                case 105:
                                        sprintf(szLine3, "%s", "電票金額超過上限");
                                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0011");
                                        break;
                                default:
                                        sprintf(szLine3, "%s", "電票金額超過上限");
                                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0012");
                                        break;
                        }
                }
                else
                {
                        sprintf(szLine3, "%s", "交易失敗");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                }
        }
        else if (!memcmp(szError, "06", 2))
        {
                if (!memcmp(szError, "0600", 4))
                {
                        sprintf(szLine2, "%s", "讀取寫入票卡異常");
                        sprintf(szLine3, "%s", "請勿移動票卡");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0004");
                }
                else if (!memcmp(szError, "0601", 4))
                {
                        switch(inCustomer)
                        {
                                case 98:
                                case 105:
                                        sprintf(szLine3, "%s", "多張票卡讀取失敗");
                                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0016");
                                        break;
                                default:
                                        sprintf(szLine3, "%s", "多張票卡讀取失敗");
                                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0017");
                                        break;
                        }
                }
                else
                {
                        sprintf(szLine3, "%s", "交易失敗");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                }
        }
        else if (!memcmp(szError, "07", 2))
        {
                if (!memcmp(szError, "0705", 4))
                {
                        switch(inCustomer)
                        {
                                case 98:
                                case 105:
                                        sprintf(szLine3, "%s", "票卡已鎖洽一卡通");
                                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0012");
                                        break;
                                default:
                                        sprintf(szLine3, "%s", "票卡已鎖");
                                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0013");
                                        break;
                        }
                }
                else if (!memcmp(szError, "0708", 4) || !memcmp(szError, "0710", 4))
                {
                        sprintf(szLine3, "%s", "交易逾時");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0003");
                }
                else
                {
                        sprintf(szLine3, "%s", "交易失敗");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                }
        }
        else if (!memcmp(szError, "0801", 4))
        {
                sprintf(szLine3, "%s", "交易逾時");
                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0003");
        }
        else
        {
                sprintf(szLine3, "%s", "交易失敗");
                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
        }

        memset(szLine1, 0x00, sizeof(szLine1));
        sprintf(szLine1, "%s", szError);
	
	DISPLAY_OBJECT	srDispMsgObj;
	memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
	strcpy(srDispMsgObj.szDispPic1Name, "");
	srDispMsgObj.inDispPic1YPosition = 0;
	srDispMsgObj.inMsgType = _ENTER_KEY_MSG_;
	
        if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)     ||
            !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_))  
        {
                srDispMsgObj.inTimeout = _CUSTOMER_105_MCDONALDS_DISPLAY_TIMEOUT_;
        }
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)		||
		 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))  
        {
                srDispMsgObj.inTimeout = _CUSTOMER_111_KIOSK_STANDARD_DISPLAY_TIMEOUT_;
        }
        else
        {
                srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
        }
        
	strcpy(srDispMsgObj.szErrMsg1, szLine1);
	srDispMsgObj.inErrMsg1Line = _LINE_8_4_;
	strcpy(srDispMsgObj.szErrMsg2, szLine2);
	srDispMsgObj.inErrMsg2Line = _LINE_8_5_;
	strcpy(srDispMsgObj.szErrMsg3, szLine3);
	srDispMsgObj.inErrMsg3Line = _LINE_8_6_;
	srDispMsgObj.inBeepTimes = 3;
	srDispMsgObj.inBeepInterval = 1000;
	
	inDISP_Msg_BMP(&srDispMsgObj);

        return (VS_SUCCESS);
}

/*
Function        :inIPASS_Init_Flow
Date&Time       :2018/1/2 上午 11:55
Describe        :參考:iniPASS_Init_flow()
*/
int inIPASS_Init_Flow(TRANSACTION_OBJECT *pobTran, unsigned char uszReInitBit)
{
#ifdef _LOAD_KEY_AP_
	return (VS_SUCCESS);
#else
	int		inRetVal = 0;
	int		inESVCIndex = 0;
	char		szTemplate[10] = {0};
	char		szFuncEnable[2 + 1] = {0};
	char		szDebugMsg[100 + 1] = {0};
	char		szAscii[200 + 1] = {0};
	unsigned int	uiInputLen = 0;
	unsigned int	uiOutputLen = 0;
	unsigned char	uszSlot = 0;
	unsigned char	uszDeviceID[5] = {0};
	unsigned char	uszInput[200 + 1] = {0};
	unsigned char	uszOutput[200 + 1] = {0};
	DebugPrintFunc	*vdIPassDebug;
	
	
   	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        inDISP_ChineseFont("一卡通初始化", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);

        /* 1.初始化SAM slot等等 */
        /* Load IPASS HDT */
	pobTran->srTRec.inTicketType = _TICKET_TYPE_IPASS_;
	pobTran->srTRec.inTDTIndex = _TDT_INDEX_00_IPASS_;
        if (inLoadTDTRec(pobTran->srTRec.inTDTIndex) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	/* 存API Version到TDT */
	inSetTicket_API_Version(IPASSMP_VERSION);
	inSaveTDTRec(_TDT_INDEX_00_IPASS_);

	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTicket_HostIndex(szTemplate);
	inESVCIndex = atoi(szTemplate);
	pobTran->srBRec.inHDTIndex = inESVCIndex;
	
	if (inLoadHDTRec(pobTran->srBRec.inHDTIndex) != VS_SUCCESS)
		return (VS_ERROR);
	if (inLoadHDPTRec(pobTran->srBRec.inHDTIndex) != VS_SUCCESS)
		return (VS_ERROR);
	
	/* 外接周邊 */
	memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
	inGetIntegrate_Device(szFuncEnable);
        if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "V3不處理週邊設備");
			inLogPrintf(AT, szDebugMsg);
		}
		
                return (VS_ERROR);
	}
	else
        {
                if (uszReInitBit == VS_FALSE)
                {
                        vdIPassDebug = vdIPASS_Callback_FuncDebug;
			
			memset(szTemplate, 0x00, sizeof(szTemplate));
                        inGetTicket_SAM_Slot(szTemplate);
			if (memcmp(szTemplate, "01", strlen("01")) == 0)
			{
				uszSlot = _SAM_SLOT_1_;
			}
			else if (memcmp(szTemplate, "02", strlen("02")) == 0)
			{
				uszSlot = _SAM_SLOT_2_;
			}
			else if (memcmp(szTemplate, "03", strlen("03")) == 0)
			{
				uszSlot = _SAM_SLOT_3_;
			}
			else if (memcmp(szTemplate, "04", strlen("04")) == 0)
			{
				uszSlot = _SAM_SLOT_4_;
			}
			
			/* DeviceID是TID八碼以Hex壓成4Bytes */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        inGetTerminalID(szTemplate);
			
                        memset(uszDeviceID, 0x00, sizeof(uszDeviceID));
                        inFunc_ASCII_to_BCD(uszDeviceID, &szTemplate[strlen(szTemplate) - 8], 4);
			
			if (ginDebug == VS_TRUE)
			{
				memset(szAscii, 0x00, sizeof(szAscii));
				inFunc_BCD_to_ASCII(szAscii, uszDeviceID, 4);
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Slot: %d, DeviceID: %s", uszSlot, szAscii);
				inLogPrintf(AT, szDebugMsg);
			}
			
			/* SAM Slot 0x01~0x08 */
                        inRetVal = IPassMP_initLib(vdIPASS_Callback_FuncDebug, uszSlot, uszDeviceID);
        
                        if (inRetVal != IPASS_EXECUTE_OK)
                        {
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "IPassMP_initLib error: %04X", inRetVal);
					inIPASS_System_Log_Printf();
				}
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%04X", inRetVal);
                                inIPASS_Display_Error(pobTran, szTemplate);
                                return (VS_ERROR);
                        }

                        memset(uszOutput, 0x00, sizeof(uszOutput));
			uiOutputLen = sizeof(uszOutput);
                        inRetVal = IPassMP_GetSystemInfo(uszOutput, &uiOutputLen);
        
			if (ginDebug == VS_TRUE)
			{
				if (inRetVal != IPASS_EXECUTE_OK)
				{
					inLogPrintf(AT, "IPassMP_GetSystemInfo error: %04X", inRetVal);
					inIPASS_System_Log_Printf();
				}
				
				inLogPrintf(AT, "TID(8) + SAMID(16)");
				inLogPrintf(AT, "Len :%u", uiOutputLen);
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				inFunc_BCD_to_ASCII(szDebugMsg, uszOutput, uiOutputLen);
				inLogPrintf(AT, szDebugMsg);
			}
			
                        if (ginISODebug == VS_TRUE)
                        {
				if (inRetVal != IPASS_EXECUTE_OK)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "IPassMP_GetSystemInfo error: %04X", inRetVal);
					inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
				}
				
                                inPRINT_ChineseFont("TID(8) + SAMID(16)", _PRT_ISO_);
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Len :%u", uiOutputLen);
				inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
                                memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				inFunc_BCD_to_ASCII(szDebugMsg, uszOutput, uiOutputLen);
                                inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
                        }
                }
        }
	
        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        /* 2.連線 */
        inDISP_PutGraphic(_CONNECTING_, 0, _COORDINATE_Y_LINE_8_7_);
        inRetVal = inFLOW_RunFunction(pobTran, _TICKET_CONNECT_);

        if (inRetVal != VS_SUCCESS)
        {
                inFLOW_RunFunction(pobTran, _TICKET_DISCONNECT_);
                return (VS_ERROR);
        }

        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);

        /* 3.iPass Reader Auth (in = 31 Byte, out = 72 Byte) */
        memset(uszInput, 0x00, sizeof(uszInput));
        memset(uszOutput, 0x00, sizeof(uszOutput));
        uiInputLen = inIPASS_Init_Command_Packet(uszInput);

	if (ginDebug == VS_TRUE)
	{
                inLogPrintf(AT, "API Input :");
		inLogPrintf(AT, "Len :%u", uiInputLen);
                memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                inFunc_BCD_to_ASCII(szDebugMsg, uszInput, uiInputLen);
                inDISP_LogPrintf_Format(szDebugMsg, "", 34);
                inLogPrintf(AT, " ");
	}
        if (ginISODebug == VS_TRUE)
        {
                inPRINT_ChineseFont("API Input :", _PRT_ISO_);
		inPRINT_ChineseFont("Len :%u", uiInputLen);
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		inFunc_BCD_to_ASCII(szDebugMsg, uszInput, uiInputLen);
                inPRINT_ChineseFont_Format(szDebugMsg, "", 34, _PRT_ISO_);
                inPRINT_ChineseFont(" ", _PRT_ISO_);
        }

	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetIntegrate_Device(szTemplate);
        if (memcmp(szTemplate, "Y", strlen("Y")) != 0)
        {
                inRetVal = IPassMP_RequestCheckRW(uszInput, uiInputLen, uszOutput, &uiOutputLen, uszDeviceID);
		
                if (inRetVal != IPASS_EXECUTE_OK)
                {
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "IPassMP_RequestCheckRW error: %04X", inRetVal);
				inLogPrintf(AT, "API Output :");
				inLogPrintf(AT, "Len :%u", uiOutputLen);
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				inFunc_BCD_to_ASCII(szDebugMsg, uszOutput, uiOutputLen);
				inDISP_LogPrintf_Format(szDebugMsg, "", 34);
				inLogPrintf(AT, " ");
				memset(szAscii, 0x00, sizeof(szAscii));
				inFunc_BCD_to_ASCII(szAscii, uszDeviceID, 4);
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Device ID:%s", szAscii);
				inLogPrintf(AT, szDebugMsg);
				
				inIPASS_System_Log_Printf();
			}
			if (ginISODebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "IPassMP_RequestCheckRW error: %04X", inRetVal);
				inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
				inPRINT_ChineseFont("API Output :", _PRT_ISO_);
				inPRINT_ChineseFont("Len :%u", uiOutputLen);
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				inFunc_BCD_to_ASCII(szDebugMsg, uszOutput, uiOutputLen);
				inPRINT_ChineseFont_Format(szDebugMsg, "", 34, _PRT_ISO_);
				inPRINT_ChineseFont(" ", _PRT_ISO_);
				memset(szAscii, 0x00, sizeof(szAscii));
				inFunc_BCD_to_ASCII(szAscii, uszDeviceID, 4);
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Device ID:%s", szAscii);
				inPRINT_ChineseFont(" ", _PRT_ISO_);
			}
                        /* API失敗就不Try */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%04X", inRetVal);
			inIPASS_Display_Error(pobTran, szTemplate);
                        inFLOW_RunFunction(pobTran, _TICKET_DISCONNECT_);
			
                        return (VS_ERROR);
                }
        }
        else
        {
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "V3不處理週邊設備");
			inLogPrintf(AT, szDebugMsg);
		}
		
                return (VS_ERROR);
        }
	
        /* 4.iPass TMS Host Auth */
        memset(pobTran->srTRec.srIPASSRec.szSign_Data, 0x00, sizeof(pobTran->srTRec.srIPASSRec.szSign_Data));
        pobTran->srTRec.srIPASSRec.lnSign_Len = uiOutputLen;
        memcpy(pobTran->srTRec.srIPASSRec.szSign_Data, uszOutput, pobTran->srTRec.srIPASSRec.lnSign_Len);

        pobTran->srTRec.srIPASSRec.inStepNum = 1;
        inRetVal = inIPASS_Logon_FuncBuildAndSendPacket(pobTran, _TICKET_ISO_MODE_IPASS_LOGON_);

        if (inRetVal != VS_SUCCESS)
	{
	        /* 收送失敗 */
                inFLOW_RunFunction(pobTran, _TICKET_DISCONNECT_);
                return (VS_ERROR);
        }

        /* 5.iPass Reader Auth */
        memset(uszInput, 0x00, sizeof(uszInput));
        memset(uszOutput, 0x00, sizeof(uszOutput));
        memcpy(uszInput, pobTran->srTRec.srIPASSRec.szSign_Data, pobTran->srTRec.srIPASSRec.lnSign_Len);
        uiOutputLen = pobTran->srTRec.srIPASSRec.lnSign_Len;

	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetIntegrate_Device(szTemplate);
        if (memcmp(szTemplate, "Y", strlen("Y")) != 0)
        {
                inRetVal = IPassMP_ExcuteCheckRW(uszInput, uiOutputLen, uszOutput, &uiOutputLen);
        }
        else
        {
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "V3不處理週邊設備");
			inLogPrintf(AT, szDebugMsg);
		}
		
                return (VS_ERROR);
        }

        if (inRetVal == IPASS_ERROR_ILLEGAL_DEVICE || inRetVal == IPASS_ERROR_ILLEGAL_SERVICE)
        {
                /* 註冊 */
                memset(uszInput, 0x00, sizeof(uszInput));
                memset(uszOutput, 0x00, sizeof(uszOutput));
                uiOutputLen = inIPASS_Init_Register_Packet(uszInput);

		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "API Input :");
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			inFunc_BCD_to_ASCII(szDebugMsg, uszInput, uiInputLen);
			inDISP_LogPrintf_Format(szDebugMsg, "", 34);
			inLogPrintf(AT, " ");
		}
                if (ginISODebug == VS_TRUE)
                {
                        inPRINT_ChineseFont("API Input :", _PRT_ISO_);
                        memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			inFunc_BCD_to_ASCII(szDebugMsg, uszInput, uiInputLen);
			inPRINT_ChineseFont_Format(szDebugMsg, "", 34, _PRT_ISO_);
			inPRINT_ChineseFont(" ", _PRT_ISO_);
                }

                memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetIntegrate_Device(szTemplate);
		if (memcmp(szTemplate, "Y", strlen("Y")) != 0)
		{
                        inRetVal = IPassMP_RequestRWRegister(uszInput, uiOutputLen, uszOutput, &uiOutputLen);

                        if (inRetVal != VS_SUCCESS)
                        {
                                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%04X", inRetVal);
                                inIPASS_Display_Error(pobTran, szTemplate);
				inFLOW_RunFunction(pobTran, _TICKET_DISCONNECT_);
				
                                return (VS_ERROR);
                        }
                }
                else
                {
                        if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "V3不處理週邊設備");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
                }

                /* iPass TMS Host Auth */
                memset(pobTran->srTRec.srIPASSRec.szSign_Data, 0x00, sizeof(pobTran->srTRec.srIPASSRec.szSign_Data));
                pobTran->srTRec.srIPASSRec.lnSign_Len = uiOutputLen;
                memcpy(&pobTran->srTRec.srIPASSRec.szSign_Data[0], &uszOutput[0], pobTran->srTRec.srIPASSRec.lnSign_Len);

                pobTran->srTRec.srIPASSRec.inStepNum = 1;
                inRetVal = inIPASS_Logon_FuncBuildAndSendPacket(pobTran, _TICKET_ISO_MODE_IPASS_REGISTER_);

                /* 收送失敗都再跑一次，成功就會過，不成功就再註冊一次 */
	        inFLOW_RunFunction(pobTran, _TICKET_DISCONNECT_);
		
	        return (VS_ERROR);
	}
	else
        {
		inFLOW_RunFunction(pobTran, _TICKET_DISCONNECT_);
		
		if (inRetVal != VS_SUCCESS)
                {
                        return (VS_ERROR);
                }

		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetIntegrate_Device(szTemplate);
		if (memcmp(szTemplate, "Y", strlen("Y")) != 0)
		{
                        /* 6.檢驗是否真的授權 */
                        memset(uszInput, 0x00, sizeof(uszInput));
                        memset(uszOutput, 0x00, sizeof(uszOutput));
                        inRetVal = IPassMP_IsRWCheck(uszInput, 0, uszOutput, &uiOutputLen);

			/*
			 * [0]
			 * (1) 讀卡機未認證： 0x00
			 * (2) 讀卡機已認證： 0x01
			 * (3) 讀卡機認證失敗： 0xFF
			 * (4) SAM 已認證： 0x02
			 * (5) SAM 認證失敗： 0xFE
			 * 
			 * [1]~[4]交易讀寫器編號
			 */
                        if (uszOutput[0] == 0x01 && inRetVal == VS_SUCCESS)
                        {
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                memcpy(szTemplate, (char *)&uszOutput[1], 4);
                                inSetTicket_ReaderID(szTemplate);
                                inSaveTDTRec(_TDT_INDEX_00_IPASS_);
                        }
                        else
                        {
                                return (VS_ERROR);
                        }
                }
                else
                {
                        if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "V3不處理週邊設備");
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
                }
        }

        return (VS_SUCCESS);
#endif
}

/*
Function        :vdIPASS_Callback_FuncDebug
Date&Time       :2018/1/2 下午 1:42
Describe        :讓ipass Lib印debug用
*/
void vdIPASS_Callback_FuncDebug(unsigned int data_len, unsigned short cmd, unsigned char sn, unsigned short sc, unsigned char *pdata)
{
	int		inOneTimeLen = 1000;
	int		inCnt = 0;
	char		szTemp[1000 + 1] = {0};
        char		szDebugMsg[1500] = {0};
	unsigned int	uiRemainLen = 0;
	
	inCnt = 0;
	uiRemainLen = data_len;

	do
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		memset(szTemp, 0x00, sizeof(szTemp));
		
		if (uiRemainLen < inOneTimeLen)
		{
			memcpy(szTemp, (char *)&pdata[inCnt], uiRemainLen);
			sprintf(szDebugMsg, "  %s", szTemp);
			inLogPrintf(AT, szDebugMsg);
		
			inCnt += uiRemainLen;
			uiRemainLen -= uiRemainLen;
		}
		else
		{
			memcpy(szTemp, (char *)&pdata[inCnt], inOneTimeLen);
			sprintf(szDebugMsg, "  %s", szTemp);
			inLogPrintf(AT, szDebugMsg);
		
			inCnt += inOneTimeLen;
			uiRemainLen -= inOneTimeLen;
		}
		
	}while(uiRemainLen > 0);
}

/*
Function        :inIPASS_Logon_FuncBuildAndSendPacket
Date&Time       :2018/1/3 上午 10:27
Describe        :Mode:1_LOGON  Mode:2_Register
*/
int inIPASS_Logon_FuncBuildAndSendPacket(TRANSACTION_OBJECT *pobTran, int inMode)
{
	int     inRetVal = 0;
        int     inOriTransactionCode = 0, inOriCode = 0, inOriISOTxnCode = 0;

	inDISP_ClearAll();
	inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_);
	inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_4_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);

        inOriTransactionCode = pobTran->inTransactionCode;
        inOriCode = pobTran->srTRec.inCode;
        inOriISOTxnCode = pobTran->inISOTxnCode;

        if (inMode == _TICKET_ISO_MODE_IPASS_REGISTER_)
        {
                pobTran->inTransactionCode = _TICKET_IPASS_REGISTER_;
                pobTran->srTRec.inCode = _TICKET_IPASS_REGISTER_;
        	pobTran->inISOTxnCode = _TICKET_IPASS_REGISTER_;
	}
	else
	{
                pobTran->inTransactionCode = _TICKET_IPASS_LOGON_;
                pobTran->srTRec.inCode = _TICKET_IPASS_LOGON_;
        	pobTran->inISOTxnCode = _TICKET_IPASS_LOGON_;
	}

	/* RRN */
	inNCCC_Ticket_Func_MakeRefNo(pobTran);

	inRetVal = inNCCC_TICKET_Func_BuildAndSendPacket(pobTran);

        pobTran->inTransactionCode = inOriTransactionCode;
        pobTran->srTRec.inCode = inOriCode;
        pobTran->inISOTxnCode = inOriISOTxnCode;

	return (inRetVal);
}

/*
Function        :inIPASS_Fast_Tap
Date&Time       :2018/1/9 下午 4:24
Describe        :
*/
int inIPASS_Fast_Tap(char* szUID, int inUIDLen)
{
#ifdef	_LOAD_KEY_AP_
	return (VS_SUCCESS);
#else
	int		i = 0;
        int		inRetVal = VS_ERROR;
	int		inUIDLen_BCD = 0;
	char		szTemplate[100 + 1] = {0};
	char		szDemoMode[2 + 1] = {0};
	unsigned char	uszUID_BCD[10 + 1] = {0};
	unsigned char	uszKey[6 + 1] = {0};
	unsigned char	uszAuthenKey[12 + 1] = {0};
        unsigned int	uiOutput_Len = 0, uiPollTime = 0;
	unsigned char	uszOutput[500] = {0};
	unsigned char	uszKeyBuffer = 0x00;
	
	
	/* 教育訓練模式 */
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inIPASS_Fast_Tap() END !");
			inLogPrintf(AT, "----------------------------------------");
		}
		
		return (VS_SUCCESS);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "----------------------------------------");
			inLogPrintf(AT, "inIPASS_Fast_Tap() START !");
		}

		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "----------------------------------------");
			inLogPrintf(AT, "IPassMP_FastQueryTicket() START !");
		}
		memset(uszOutput, 0x00, sizeof(uszOutput));
		inRetVal = IPassMP_FastQueryTicket(uiPollTime, uszOutput, &uiOutput_Len);
		if (inRetVal != IPASS_EXECUTE_OK)
		{
			if (ginDebug == VS_TRUE)
			{
				inIPASS_System_Log_Printf();
			}
			
			return (VS_ERROR);
		}
		
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "IPassMP_FastQueryTicket() END !");
			inLogPrintf(AT, "----------------------------------------");
		}

		memset(szTemplate, 0x00, sizeof(szTemplate)); 
		inFunc_BCD_to_ASCII(szTemplate, uszOutput, uiOutput_Len);

		memset(uszKey, 0x00, sizeof(uszKey)); 
		inFunc_ASCII_to_BCD(uszKey, &szTemplate[2], 6);
		memset(uszAuthenKey, 0x00, sizeof(uszAuthenKey));

		/* 轉換方式抄虹堡Sample code */
		for (i = 0; i < 6; i++)
		{
			uszKeyBuffer = uszKey[i];
			uszAuthenKey[i * 2]=(uszKeyBuffer ^ 0xF0) & 0xF0;
			uszAuthenKey[i * 2]|= (uszKeyBuffer >> 4);
			uszAuthenKey[i * 2 + 1]=((uszKeyBuffer ^ 0x0F) << 4);
			uszAuthenKey[i * 2 + 1]|= (uszKeyBuffer & 0x0F);
		}

		if (inCTLS_Mifare_LoadKey(uszAuthenKey) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}

		/* IPASS 授權固定只取UID前四Byte */
		inUIDLen = 8;
		*(szUID + 8) = 0x00;
		
		inUIDLen_BCD = inUIDLen / 2;
		memset(uszUID_BCD, 0x00, sizeof(uszUID_BCD));
		inFunc_ASCII_to_BCD(uszUID_BCD, szUID, inUIDLen_BCD);
		/* 第0個Block */
		if (uszOutput[0] == 0x60)
		{
			inRetVal = inCTLS_Mifare_Auth(0x60, 0, uszUID_BCD, inUIDLen_BCD);
		}
		else if (uszOutput[0] == 0x61)
		{
			inRetVal = inCTLS_Mifare_Auth(0x61, 0, uszUID_BCD, inUIDLen_BCD);
		}
		else
		{
			return (VS_ERROR);
		}

		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inIPASS_Fast_Tap() END !");
			inLogPrintf(AT, "----------------------------------------");
		}

		return (inRetVal);
	}
#endif
}

/*
Function        :inIPASS_Query_Flow
Date&Time       :2017/12/21 下午 1:52
Describe        :
*/
int inIPASS_Query_Flow(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = 0;
	char	szTxnType[20 + 1] = {0};
	char	szDebugMsg[100 + 1] = {0};
	char	szDemoMode[2 + 1] = {0};
	char	szTemplate[100 + 1] = {0};
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_Ticket_IPASS_Query_Flow() START !");
	}
	
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("%d %s start", __LINE__, __FUNCTION__);
	}
	
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		/* 1.詢卡 - 只有扣款要額外詢卡 */
		pobTran->inTransactionCode = _TICKET_IPASS_QUERY_;
		pobTran->srTRec.inCode = _TICKET_IPASS_QUERY_;
		pobTran->srTRec.srIPASSRec.uszQueryBit = VS_TRUE;

		inRetVal = inFLOW_RunFunction(pobTran, _TICKET_GET_PARM_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		inRetVal = inFLOW_RunFunction(pobTran, _TICKET_IPASS_TAPCARD_FIRST_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		/* 2.聯名卡要回送簽章 */
		if (pobTran->srTRec.srIPASSRec.uszCB_CardBit == VS_TRUE)
		{
			if (inFLOW_RunFunction(pobTran, _TICKET_CONNECT_) != VS_SUCCESS)
				return (VS_ERROR);

			pobTran->srTRec.srIPASSRec.uszQueryBit = VS_TRUE;  

			/* 回送簽章 IP01 */
			if (inFLOW_RunFunction(pobTran, _TICKET_FUNCTION_SET_TXN_ONLINE_OFFLINE_) != VS_SUCCESS)
				return (VS_ERROR);

			if (inFLOW_RunFunction(pobTran, _TICKET_FUNCTION_BUILD_AND_SEND_PACKET_) != VS_SUCCESS)
				return (VS_ERROR); 

			/* 拍完不響嗶聲 */
			pobTran->uszESVCTxnNotEndBit = VS_TRUE;

			if (inFLOW_RunFunction(pobTran, _TICKET_IPASS_TAPCARD_SECOND_) != VS_SUCCESS)
				return (VS_ERROR);

			/* 回覆原來設定 */
			pobTran->uszESVCTxnNotEndBit = VS_FALSE;

			pobTran->srTRec.srIPASSRec.uszQueryBit = VS_FALSE;               
		}
		else
		{
			pobTran->srTRec.srIPASSRec.uszQueryBit = VS_FALSE;  
		}

		/* 累加電文 inv */
		inNCCC_Ticket_Func_UpdatetTicketInvNum(pobTran);

		/* 3.檢查是否餘額不足 */
		if ((pobTran->srTRec.lnCardRemainAmount < 0) || 
		    ((pobTran->srTRec.lnCardRemainAmount - pobTran->srTRec.lnTxnAmount) < 0))
		{
			if (pobTran->srTRec.srIPASSRec.uszCB_CardBit == VS_TRUE)    
			{
				/* 交易開關 */
				memset(szTxnType, 0x00, sizeof(szTxnType));
				inGetTicket_HostTransFunc(szTxnType);

				if (szTxnType[3] == 'Y')
				{
					/* 要測自動加值後達10000元以上 */
					if (inIPASS_Top_Up_Amount_Check(pobTran) != VS_SUCCESS)  
					{
						memset(szTemplate, 0x00, sizeof(szTemplate));
						sprintf(szTemplate, "%04X", 0x0518);
						inIPASS_Display_Error(pobTran, szTemplate);
						return (VS_ERROR);
					}
					else
					{
						pobTran->srTRec.uszAutoTopUpBit = VS_TRUE;
					}
				}
				else
				{
					/* 負值不可交易 */
					/* 要測0元扣101元且沒開自動加值功能，會回傳0518 */			
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "inNCCC_Ticket_IPASS_Query_Flow()_負值不可交易");
						inLogPrintf(AT, szDebugMsg);
					}
					
					memset(szTemplate, 0x00, sizeof(szTemplate));
					sprintf(szTemplate, "%04X", 0x0518);
					inIPASS_Display_Error(pobTran, szTemplate);
					return (VS_ERROR);
				}
			}
			else
			{        
				/* 負值不可交易 */
				/* 要測0元扣101元，會回傳0518(一般卡) */
				/* DEMO板沒辦法用API辨別，所以要擋起來 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%04X", 0x0518);
                                inIPASS_Display_Error(pobTran, szTemplate);
				return (VS_ERROR);
			}
		}
		else
		{
			pobTran->srTRec.uszAutoTopUpBit = VS_FALSE;     
			pobTran->srTRec.lnTotalTopUpAmount = 0;        
		}

		if (pobTran->srTRec.uszBlackListBit == VS_TRUE)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "黑名單中");
				inLogPrintf(AT, szDebugMsg);
			}
			return (VS_ERROR);
		}

		/* 4.自動加值 */
		if (pobTran->srTRec.uszAutoTopUpBit == VS_TRUE)
		{
			inNCCC_Ticket_Set_Msg(pobTran->srTRec.inTicketType, &pobTran->inTicketMsgStatus, _TICKET_MSG_IPASS_AUTO_TOP_UP_);

			pobTran->inTransactionCode = _TICKET_IPASS_AUTO_TOP_UP_;
			pobTran->srTRec.inCode = _TICKET_IPASS_AUTO_TOP_UP_;
			/* 自動加值金額DEMO點數累加 */
			gulDemoTicketPoint += pobTran->srTRec.lnTotalTopUpAmount;

			/* 詢卡 */  
			if (inFLOW_RunFunction(pobTran, _TICKET_GET_PARM_) != VS_SUCCESS)
				return (VS_ERROR); 

			if (inFLOW_RunFunction(pobTran, _TICKET_IPASS_TAPCARD_FIRST_) != VS_SUCCESS)
				return (VS_ERROR); 

			if (inFLOW_RunFunction(pobTran, _TICKET_CONNECT_) != VS_SUCCESS)
				return (VS_ERROR);        

			if (inFLOW_RunFunction(pobTran, _TICKET_FUNCTION_SET_TXN_ONLINE_OFFLINE_) != VS_SUCCESS)
				return (VS_ERROR);

			if (inFLOW_RunFunction(pobTran, _TICKET_FUNCTION_BUILD_AND_SEND_PACKET_) != VS_SUCCESS)
				return (VS_ERROR);     

			/* 拍完不響嗶聲 */
			pobTran->uszESVCTxnNotEndBit = VS_TRUE;

			if (inFLOW_RunFunction(pobTran, _TICKET_IPASS_TAPCARD_SECOND_) != VS_SUCCESS)
				return (VS_ERROR);

			/* 回覆原來設定 */
			pobTran->uszESVCTxnNotEndBit = VS_FALSE;

			if (inFLOW_RunFunction(pobTran, _TICKET_FUNCTION_SET_TXN_ONLINE_OFFLINE_) != VS_SUCCESS)
				return (VS_ERROR);

			if (inFLOW_RunFunction(pobTran, _TICKET_FUNCTION_BUILD_AND_SEND_PACKET_) != VS_SUCCESS)
				return (VS_ERROR);             

			if (inFLOW_RunFunction(pobTran, _TICKET_UPDATE_ACCUM_) != VS_SUCCESS)
				return (VS_ERROR);  

			if (inFLOW_RunFunction(pobTran, _TICKET_UPDATE_BATCH_) != VS_SUCCESS)
				return (VS_ERROR);  

			if (inFLOW_RunFunction(pobTran, _TICKET_UPDATE_INV_) != VS_SUCCESS)
				return (VS_ERROR);
			
			/* 取得金額，避免加值成功購貨失敗回傳金額錯誤 by Russell 2020/8/4 下午 4:32 */
			inFLOW_RunFunction(pobTran, _TICKET_GET_DAVTI_PARM_);
			
			/* 自動加值成功 */
			pobTran->uszAutoTopUpSuccessBit = VS_TRUE;
		} 

		/* 5.自動加值結束要再詢卡一次 */ 
		if (pobTran->srTRec.uszAutoTopUpBit == VS_TRUE)
		{
			pobTran->inTransactionCode = _TICKET_IPASS_QUERY_;
			pobTran->srTRec.inCode = _TICKET_IPASS_QUERY_;
			pobTran->srTRec.srIPASSRec.uszQueryBit = VS_TRUE;

			/* 詢卡 */  
			if (inFLOW_RunFunction(pobTran, _TICKET_GET_PARM_) != VS_SUCCESS)
				return (VS_ERROR); 

			if (inFLOW_RunFunction(pobTran, _TICKET_IPASS_TAPCARD_FIRST_) != VS_SUCCESS)
				return (VS_ERROR); 

			/* 2.聯名卡要回送簽章 */
			if (pobTran->srTRec.srIPASSRec.uszCB_CardBit == VS_TRUE)
			{
				if (inFLOW_RunFunction(pobTran, _TICKET_CONNECT_) != VS_SUCCESS)
					return (VS_ERROR);

				pobTran->srTRec.srIPASSRec.uszQueryBit = VS_TRUE;  

				/* 回送簽章 IP01 */
				if (inFLOW_RunFunction(pobTran, _TICKET_FUNCTION_SET_TXN_ONLINE_OFFLINE_) != VS_SUCCESS)
					return (VS_ERROR);

				if (inFLOW_RunFunction(pobTran, _TICKET_FUNCTION_BUILD_AND_SEND_PACKET_) != VS_SUCCESS)
					return (VS_ERROR); 

				/* 拍完不響嗶聲 */
				pobTran->uszESVCTxnNotEndBit = VS_TRUE;

				if (inFLOW_RunFunction(pobTran, _TICKET_IPASS_TAPCARD_SECOND_) != VS_SUCCESS)
					return (VS_ERROR);
				/* 回覆原來設定 */
				pobTran->uszESVCTxnNotEndBit = VS_FALSE;

			}  

			inNCCC_Ticket_Func_UpdatetTicketInvNum(pobTran);

			if (pobTran->srTRec.uszBlackListBit == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "黑名單中");
					inLogPrintf(AT, szDebugMsg);
				}
				return (VS_ERROR);
			}
		}

		/* 6.詢卡跟自動加值結束後要跑原來的購貨流程 */
		pobTran->inTransactionCode = _TICKET_IPASS_DEDUCT_;
		pobTran->srTRec.inCode = _TICKET_IPASS_DEDUCT_;
		pobTran->srTRec.srIPASSRec.uszQueryBit = VS_FALSE;
	}
	else
	{
		/* 1.詢卡 - 只有扣款要額外詢卡 */
		pobTran->inTransactionCode = _TICKET_IPASS_QUERY_;
		pobTran->srTRec.inCode = _TICKET_IPASS_QUERY_;
		pobTran->srTRec.srIPASSRec.uszQueryBit = VS_TRUE;

		inRetVal = inFLOW_RunFunction(pobTran, _TICKET_GET_PARM_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		inRetVal = inFLOW_RunFunction(pobTran, _TICKET_IPASS_TAPCARD_FIRST_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		/* 2.聯名卡要回送簽章 */
		if (pobTran->srTRec.srIPASSRec.uszCB_CardBit == VS_TRUE)
		{
			if (inFLOW_RunFunction(pobTran, _TICKET_CONNECT_) != VS_SUCCESS)
				return (VS_ERROR);

			pobTran->srTRec.srIPASSRec.uszQueryBit = VS_TRUE;  

			/* 回送簽章 IP01 */
			if (inFLOW_RunFunction(pobTran, _TICKET_FUNCTION_SET_TXN_ONLINE_OFFLINE_) != VS_SUCCESS)
				return (VS_ERROR);

			if (inFLOW_RunFunction(pobTran, _TICKET_FUNCTION_BUILD_AND_SEND_PACKET_) != VS_SUCCESS)
				return (VS_ERROR); 

			/* 拍完不響嗶聲 */
			pobTran->uszESVCTxnNotEndBit = VS_TRUE;

			if (inFLOW_RunFunction(pobTran, _TICKET_IPASS_TAPCARD_SECOND_) != VS_SUCCESS)
				return (VS_ERROR);

			/* 回覆原來設定 */
			pobTran->uszESVCTxnNotEndBit = VS_FALSE;

			pobTran->srTRec.srIPASSRec.uszQueryBit = VS_FALSE;               
		}
		else
		{
			pobTran->srTRec.srIPASSRec.uszQueryBit = VS_FALSE;  
		}

		/* 累加電文 inv */
		inNCCC_Ticket_Func_UpdatetTicketInvNum(pobTran);

		/* 3.檢查是否餘額不足 */
		if ((pobTran->srTRec.lnCardRemainAmount < 0) || 
		    ((pobTran->srTRec.lnCardRemainAmount - pobTran->srTRec.lnTxnAmount) < 0))
		{
			if (pobTran->srTRec.srIPASSRec.szCardInfo[43] == 0x01 && pobTran->srTRec.srIPASSRec.uszCB_CardBit == VS_TRUE)    
			{
				/* 交易開關 */
				memset(szTxnType, 0x00, sizeof(szTxnType));
				inGetTicket_HostTransFunc(szTxnType);

				if (szTxnType[3] == 'Y')
				{
					/* 要測自動加值後達10000元以上 */
					if (inIPASS_Top_Up_Amount_Check(pobTran) != VS_SUCCESS)  
					{

					}
					else
					{
						pobTran->srTRec.uszAutoTopUpBit = VS_TRUE;
					}
				}
				/* 仍有押金可扣狀況 */
				else if ((pobTran->srTRec.lnCardRemainAmount - pobTran->srTRec.lnTxnAmount) > -100)
				{
					pobTran->srTRec.uszAutoTopUpBit = VS_FALSE;     
					pobTran->srTRec.lnTotalTopUpAmount = 0;    
				}
				else
				{
					/* 負值不可交易 */
					/* 要測0元扣101元且沒開自動加值功能，會回傳0518 */			
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "inNCCC_Ticket_IPASS_Query_Flow()_負值不可交易");
						inLogPrintf(AT, szDebugMsg);
					}
				}
			}
			else if ((pobTran->srTRec.lnCardRemainAmount - pobTran->srTRec.lnTxnAmount) > -100)
			{
				pobTran->srTRec.uszAutoTopUpBit = VS_FALSE;     
				pobTran->srTRec.lnTotalTopUpAmount = 0;    
			}
			else
			{        
				/* 負值不可交易 */
				/* 要測0元扣101元，會回傳0518(一般卡) */	
			}
		}
		else
		{
			pobTran->srTRec.uszAutoTopUpBit = VS_FALSE;     
			pobTran->srTRec.lnTotalTopUpAmount = 0;        
		}

		if (pobTran->srTRec.uszBlackListBit == VS_TRUE)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "黑名單中");
				inLogPrintf(AT, szDebugMsg);
			}
			return (VS_ERROR);
		}

		/* 4.自動加值 */
		if (pobTran->srTRec.uszAutoTopUpBit == VS_TRUE)
		{
			inNCCC_Ticket_Set_Msg(pobTran->srTRec.inTicketType, &pobTran->inTicketMsgStatus, _TICKET_MSG_IPASS_AUTO_TOP_UP_);

			pobTran->inTransactionCode = _TICKET_IPASS_AUTO_TOP_UP_;
			pobTran->srTRec.inCode = _TICKET_IPASS_AUTO_TOP_UP_;

			/* 詢卡 */  
			if (inFLOW_RunFunction(pobTran, _TICKET_GET_PARM_) != VS_SUCCESS)
				return (VS_ERROR); 

			if (inFLOW_RunFunction(pobTran, _TICKET_IPASS_TAPCARD_FIRST_) != VS_SUCCESS)
				return (VS_ERROR); 

			if (inFLOW_RunFunction(pobTran, _TICKET_CONNECT_) != VS_SUCCESS)
				return (VS_ERROR);        

			if (inFLOW_RunFunction(pobTran, _TICKET_FUNCTION_SET_TXN_ONLINE_OFFLINE_) != VS_SUCCESS)
				return (VS_ERROR);

			if (inFLOW_RunFunction(pobTran, _TICKET_FUNCTION_BUILD_AND_SEND_PACKET_) != VS_SUCCESS)
				return (VS_ERROR);     

			/* 拍完不響嗶聲 */
			pobTran->uszESVCTxnNotEndBit = VS_TRUE;

			if (inFLOW_RunFunction(pobTran, _TICKET_IPASS_TAPCARD_SECOND_) != VS_SUCCESS)
				return (VS_ERROR);

			/* 回覆原來設定 */
			pobTran->uszESVCTxnNotEndBit = VS_FALSE;

			if (inFLOW_RunFunction(pobTran, _TICKET_FUNCTION_SET_TXN_ONLINE_OFFLINE_) != VS_SUCCESS)
				return (VS_ERROR);

			if (inFLOW_RunFunction(pobTran, _TICKET_FUNCTION_BUILD_AND_SEND_PACKET_) != VS_SUCCESS)
				return (VS_ERROR);             

			if (inFLOW_RunFunction(pobTran, _TICKET_UPDATE_ACCUM_) != VS_SUCCESS)
				return (VS_ERROR);  

			if (inFLOW_RunFunction(pobTran, _TICKET_UPDATE_BATCH_) != VS_SUCCESS)
				return (VS_ERROR);  

			if (inFLOW_RunFunction(pobTran, _TICKET_UPDATE_INV_) != VS_SUCCESS)
				return (VS_ERROR);
			
			/* 取得金額，避免加值成功購貨失敗回傳金額錯誤 by Russell 2020/8/4 下午 4:32 */
			inFLOW_RunFunction(pobTran, _TICKET_GET_DAVTI_PARM_);
			
			/* 自動加值成功 */
			pobTran->uszAutoTopUpSuccessBit = VS_TRUE;
		} 

		/* 5.自動加值結束要再詢卡一次 */ 
		if (pobTran->srTRec.uszAutoTopUpBit == VS_TRUE)
		{
			pobTran->inTransactionCode = _TICKET_IPASS_QUERY_;
			pobTran->srTRec.inCode = _TICKET_IPASS_QUERY_;
			pobTran->srTRec.srIPASSRec.uszQueryBit = VS_TRUE;

			/* 詢卡 */  
			if (inFLOW_RunFunction(pobTran, _TICKET_GET_PARM_) != VS_SUCCESS)
				return (VS_ERROR); 

			if (inFLOW_RunFunction(pobTran, _TICKET_IPASS_TAPCARD_FIRST_) != VS_SUCCESS)
				return (VS_ERROR); 

			/* 2.聯名卡要回送簽章 */
			if (pobTran->srTRec.srIPASSRec.uszCB_CardBit == VS_TRUE)
			{
				if (inFLOW_RunFunction(pobTran, _TICKET_CONNECT_) != VS_SUCCESS)
					return (VS_ERROR);

				pobTran->srTRec.srIPASSRec.uszQueryBit = VS_TRUE;  

				/* 回送簽章 IP01 */
				if (inFLOW_RunFunction(pobTran, _TICKET_FUNCTION_SET_TXN_ONLINE_OFFLINE_) != VS_SUCCESS)
					return (VS_ERROR);

				if (inFLOW_RunFunction(pobTran, _TICKET_FUNCTION_BUILD_AND_SEND_PACKET_) != VS_SUCCESS)
					return (VS_ERROR); 

				/* 拍完不響嗶聲 */
				pobTran->uszESVCTxnNotEndBit = VS_TRUE;

				if (inFLOW_RunFunction(pobTran, _TICKET_IPASS_TAPCARD_SECOND_) != VS_SUCCESS)
					return (VS_ERROR);
				/* 回覆原來設定 */
				pobTran->uszESVCTxnNotEndBit = VS_FALSE;

			}  

			inNCCC_Ticket_Func_UpdatetTicketInvNum(pobTran);

			if (pobTran->srTRec.uszBlackListBit == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "黑名單中");
					inLogPrintf(AT, szDebugMsg);
				}
				return (VS_ERROR);
			}
		}

		/* 6.詢卡跟自動加值結束後要跑原來的購貨流程 */
		pobTran->inTransactionCode = _TICKET_IPASS_DEDUCT_;
		pobTran->srTRec.inCode = _TICKET_IPASS_DEDUCT_;
		pobTran->srTRec.srIPASSRec.uszQueryBit = VS_FALSE;
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inIPASS_Check_System_Error
Date&Time       :2019/2/21 下午 4:41
Describe        :
*/
int inIPASS_Check_System_Error(int *inErrorCode, char *szErrorMsg, int *inErrorLen)
{
	int	inRetVal = VS_SUCCESS;
	int	inRetVal2 = VS_ERROR;
	int	inIPassErrorLen = 0;
	int	inUTF8Len = 0;
	char	*szIPassError = NULL;
	char	szUTF8[150 + 1] = {0};
	
	inRetVal2 = IPassMP_getSystemError();
	inIPassErrorLen = IPassMP_translateReturnCodeString(inRetVal2, &szIPassError);
	memset(szUTF8, 0x00, sizeof(szUTF8));
	inFunc_Big5toUTF8(szUTF8, szIPassError);
	inUTF8Len = ((inIPassErrorLen * 3) / 2);
	
	*inErrorCode = inRetVal2;
	strcpy(szErrorMsg, szUTF8);
	*inErrorLen = inUTF8Len;
	
	return (inRetVal);
}

/*
Function        :inIPASS_System_Log_Printf
Date&Time       :2019/2/21 下午 4:59
Describe        :
*/
int inIPASS_System_Log_Printf(void)
{
	int	inIPassErrorCode = 0;
	int	inUTF8Len = 0;
	char	szUTF8[100 + 1] = {0};
	char	szDebugMsg[200 + 1] = {0};
	
	memset(szUTF8, 0x00, sizeof(szUTF8));
	inIPASS_Check_System_Error(&inIPassErrorCode, szUTF8, &inUTF8Len);
	memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
	sprintf(szDebugMsg, "%04X:%s", inIPassErrorCode, szUTF8);
	inLogPrintf(AT, szDebugMsg);
				
	return (VS_SUCCESS);
}

/*
Function        :inIPASS_Top_Up_Amount_Check
Date&Time       :2019/4/11 上午 11:27
Describe        :增加檢核超過1000元就不加值
 *		 增加檢核會超過當日上限(3000)就不加值
*/
int inIPASS_Top_Up_Amount_Check(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;
	long	lnDeductAmount = 0;
	char	szDebugMsg[100 + 1] = {0};
	
	/* 超過1000元就不加值，雖然前面有擋，這邊預防有利外狀況，再擋一次 */
	lnDeductAmount = pobTran->srTRec.lnTxnAmount;		/* 扣款金額 */
	if (lnDeductAmount > 1000)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "超過購貨金額上限1000元: %ld", lnDeductAmount);
			inLogPrintf(AT, szDebugMsg);
		}
		inRetVal = VS_ERROR;
		return (inRetVal);
	}
	
	/* 預期當日會超過3000元，也不加值 */
	if (memcmp(pobTran->srTRec.szDate, &pobTran->srTRec.srIPASSRec.szLastDeductDate[2], 6) == 0)
	{
		if (pobTran->srTRec.srIPASSRec.lnLastDeductOneDayValue + lnDeductAmount > 3000)
		{
			/* 自動加值金額過大 */
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "超過當日購貨金額上限3000元: %ld", (pobTran->srTRec.srIPASSRec.lnLastDeductOneDayValue + lnDeductAmount));
				inLogPrintf(AT, szDebugMsg);
			}
			inRetVal = VS_ERROR;
			return (inRetVal);
		}
	}
	
	inRetVal = inNCCC_Ticket_Top_Up_Amount_Check(pobTran);
	if (inRetVal != VS_SUCCESS)
	{
		/* 自動加值金額過大 */
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inNCCC_Ticket_IPASS_Query_Flow()_自動加值金額過大");
			inLogPrintf(AT, szDebugMsg);
		}
		inRetVal = VS_ERROR;
		return (inRetVal);
	}
	
	return (inRetVal);
}

/*
Function        :inIPASS_Cus_096_Do_Cmd
Date&Time       :2022/8/18 下午 6:31
Describe        :
*/
int inIPASS_Cus_096_Do_Cmd(TRANSACTION_OBJECT *pobTran, ECR_TABLE* srECROb)
{
	int		inIPASS_API_RetVal = VS_SUCCESS;
	int		inRetVal = VS_SUCCESS;
	int		inPollTime = 0;
	uint32_t	uiInput_Len = 0;
	uint32_t	uiOutput_Len = 0;
	uint8_t		uszInput[500 + 1] = {0};
	uint8_t		uszOutput[500 + 1] = {0};
	char		szTemplate[10 + 1] = {0}, szAPI_ErrCode[2 + 1] = {0};

	srECROb->srTransData.srCus096_Data.uszIsError = VS_FALSE;
	
	/* Input Len */
	uiInput_Len = atoi(srECROb->srTransData.srCus096_Data.szCommandDataLength);

	/* Input Data */
	memset(uszInput, 0x00, sizeof(uszInput));
	memcpy(&uszInput[0], srECROb->srTransData.srCus096_Data.szCommandData, uiInput_Len);

        memset(uszOutput, 0x00, sizeof(uszOutput));
	memset(szAPI_ErrCode, 0x00, sizeof(szAPI_ErrCode));
	uiOutput_Len = 0;
	do
	{
		/* 如果用外接做直連的話(先預留位置) */
		if (0)
		{
			/* 沒寫，預設為失敗 */
			inRetVal = VS_ERROR;
		}
		else
		{
			/* 不用拍卡 */
			if (!memcmp(srECROb->srTransData.srCus096_Data.szTransType, _F_INIT_, 2)	||
			    !memcmp(srECROb->srTransData.srCus096_Data.szTransType, _F_REGI_, 2)	||
			    !memcmp(srECROb->srTransData.srCus096_Data.szTransType, _S_END_, 2)		||
			    !memcmp(srECROb->srTransData.srCus096_Data.szTransType, _S_INIT_, 2))
			{
				inRetVal = VS_SUCCESS;
			}
			else
			{
				inRetVal = inNCCC_Ticket_Fast_Tap_Wait(pobTran, pobTran->srTRec.szUID);
			}
			
			if (inRetVal != VS_SUCCESS)
			{
				/* 區分是User Cancel or Timeout */
				if (inRetVal == VS_TIMEOUT)
					srECROb->srTransData.srCus096_Data.inErrorType = _ECR_IPASS_CUS096_ERR_TIMEOUT_;
				else if (inRetVal == VS_USER_CANCEL)
					srECROb->srTransData.srCus096_Data.inErrorType = _ECR_IPASS_CUS096_ERR_CANCEL_;
				
				srECROb->srTransData.srCus096_Data.uszIsError = VS_TRUE;
			}
			else
			{
				/* 不用拍卡 */
				if (!memcmp(srECROb->srTransData.srCus096_Data.szTransType, _F_INIT_, 2)	||
				    !memcmp(srECROb->srTransData.srCus096_Data.szTransType, _F_REGI_, 2)	||
				    !memcmp(srECROb->srTransData.srCus096_Data.szTransType, _S_END_, 2)		||
				    !memcmp(srECROb->srTransData.srCus096_Data.szTransType, _S_INIT_, 2))
				{
					inRetVal = TRUE;
				}
				else
				{
					memset(pobTran->srTRec.szAPUID, 0x00, sizeof(pobTran->srTRec.szAPUID));
					inFunc_ASCII_to_BCD((unsigned char*)pobTran->srTRec.szAPUID, pobTran->srTRec.szUID, (strlen(pobTran->srTRec.szUID) / 2));

					inRetVal = IPASS_PollCard();
				}
				
				if (inRetVal == TRUE)
				{
					pobTran->srTRec.inTicketType = _TICKET_TYPE_IPASS_;
					pobTran->srTRec.inTDTIndex = _TDT_INDEX_00_IPASS_;
					pobTran->srTRec.uszESVCTransBit = VS_TRUE;

					if (!memcmp(srECROb->srTransData.srCus096_Data.szTransType, _F_DEDUCT_, 2))
						inIPASS_API_RetVal = IPassMP_RequestDeductValue(inPollTime, uszInput, uiInput_Len, uszOutput, &uiOutput_Len);
					else if (!memcmp(srECROb->srTransData.srCus096_Data.szTransType, _F_ADD_, 2))
						inIPASS_API_RetVal = IPassMP_RequestAddValue(inPollTime, uszInput, uiInput_Len, uszOutput, &uiOutput_Len);
					else if (!memcmp(srECROb->srTransData.srCus096_Data.szTransType, _F_VOID_ADD_, 2))
						inIPASS_API_RetVal = IPassMP_RequestCancelAddValue(inPollTime, uszInput, uiInput_Len, uszOutput, &uiOutput_Len);
					else if (!memcmp(srECROb->srTransData.srCus096_Data.szTransType, _F_QUERY_, 2))
						inIPASS_API_RetVal = IPassMP_RequestQueryTicket(inPollTime, uszInput, uiInput_Len, uszOutput, &uiOutput_Len);
					else if (!memcmp(srECROb->srTransData.srCus096_Data.szTransType, _F_REGI_, 2))
						inIPASS_API_RetVal = IPassMP_RequestRWRegister(uszInput, uiInput_Len, uszOutput, &uiOutput_Len);
					else if (!memcmp(srECROb->srTransData.srCus096_Data.szTransType, _F_INIT_, 2))
						inIPASS_API_RetVal = iniPASS_Init_Flow_Cus096_Internal(uszInput, uiInput_Len, uszOutput, &uiOutput_Len, _IPASS_COMAND_FIRST_);
					else if (!memcmp(srECROb->srTransData.srCus096_Data.szTransType, _S_DEDUCT_, 2))
						inIPASS_API_RetVal = IPassMP_ExecuteDeductValue(uszInput, uiInput_Len, uszOutput, &uiOutput_Len);
					else if (!memcmp(srECROb->srTransData.srCus096_Data.szTransType, _S_ADD_, 2))
						inIPASS_API_RetVal = IPassMP_ExecuteAddValue(uszInput, uiInput_Len, uszOutput, &uiOutput_Len);
					else if (!memcmp(srECROb->srTransData.srCus096_Data.szTransType, _S_VOID_ADD_, 2))
						inIPASS_API_RetVal = IPassMP_ExecuteCancelAddValue(uszInput, uiInput_Len, uszOutput, &uiOutput_Len);
					else if (!memcmp(srECROb->srTransData.srCus096_Data.szTransType, _S_QUERY_, 2))
						inIPASS_API_RetVal = IPassMP_ExecuteQueryTicket(uszInput, uiInput_Len, uszOutput, &uiOutput_Len);
					else if (!memcmp(srECROb->srTransData.srCus096_Data.szTransType, _S_END_, 2))
					{
						uiOutput_Len = 2;
						memcpy((char *)&uszOutput[0], "OK", 2);
						inIPASS_API_RetVal = VS_SUCCESS;
					}
					else if (!memcmp(srECROb->srTransData.srCus096_Data.szTransType, _S_LOCK_, 2))
						inIPASS_API_RetVal = IPassMP_ExecuteBlockTicket(uszInput, uiInput_Len, uszOutput, &uiOutput_Len);
					else if (!memcmp(srECROb->srTransData.srCus096_Data.szTransType, _S_INIT_, 2))
						inIPASS_API_RetVal = iniPASS_Init_Flow_Cus096_Internal(uszInput, uiInput_Len, uszOutput, &uiOutput_Len, _IPASS_COMAND_SECOND_);
					else
					{
						srECROb->srTransData.srCus096_Data.uszIsError = VS_TRUE;
					}

					memset(szTemplate, 0x00, sizeof(szTemplate));
					sprintf(szTemplate, "%04X", inIPASS_API_RetVal);
					memset(szAPI_ErrCode, 0x00, sizeof(szAPI_ErrCode));
					inFunc_ASCII_to_BCD((unsigned char*)szAPI_ErrCode, szTemplate, 2);

					if (inIPASS_API_RetVal == IPASS_EXECUTE_OK		|| 
					    inIPASS_API_RetVal == IPASS_ERROR_RECOVERY		|| 
					    inIPASS_API_RetVal == IPASS_ERROR_AUTOLOAD_OFF	|| 
					    inIPASS_API_RetVal == IPASS_ERROR_AUTOLOAD_ON	|| 
					    inIPASS_API_RetVal == IPASS_ERROR_INVALID_AUTOADDVALUE || 
					    inIPASS_API_RetVal == IPASS_ERROR_BLACK_LIST)
					{
						if (inIPASS_API_RetVal == IPASS_ERROR_INVALID_AUTOADDVALUE || 
						    inIPASS_API_RetVal == IPASS_ERROR_BLACK_LIST)
						{
							if (!memcmp(srECROb->srTransData.srCus096_Data.szTransType, _F_INIT_, 2)		||
							    !memcmp(srECROb->srTransData.srCus096_Data.szTransType, _S_INIT_, 2))
							{
								/* Init自己處理顯示*/
							}
							else
							{
								inIPASS_Display_Error(pobTran, szTemplate);
							}
						}
					}
					else
					{
						if (!memcmp(srECROb->srTransData.srCus096_Data.szTransType, _F_INIT_, 2)		||
						    !memcmp(srECROb->srTransData.srCus096_Data.szTransType, _S_INIT_, 2))
						{
							/* Init自己處理顯示*/
						}
						else
						{
							inIPASS_Display_Error(pobTran, szTemplate);
						}
						
						srECROb->srTransData.srCus096_Data.uszIsError = VS_TRUE;

						if (inIPASS_API_RetVal == IPASS_ERROR_NO_CARD)
						{
							/* 需要測試 */
							inCTLS_Power_Off();
							inCTLS_ReActive_TypeACard();
						}
					}
				}
				else
				{
					/* 非IPASS卡 */
					srECROb->srTransData.srCus096_Data.uszIsError = VS_TRUE;
				}
			}
			
			memset(srECROb->srTransData.srCus096_Data.szAPIResponseCode, 0x00, sizeof(srECROb->srTransData.srCus096_Data.szAPIResponseCode));
			memcpy(srECROb->srTransData.srCus096_Data.szAPIResponseCode, szAPI_ErrCode, 2);
			memset(srECROb->srTransData.srCus096_Data.szCommandDataLength, 0x00, sizeof(srECROb->srTransData.srCus096_Data.szCommandDataLength));
			sprintf(srECROb->srTransData.srCus096_Data.szCommandDataLength, "%04u", uiOutput_Len);
			memset(srECROb->srTransData.srCus096_Data.szCommandData, 0x00, sizeof(srECROb->srTransData.srCus096_Data.szCommandData));
			memcpy(srECROb->srTransData.srCus096_Data.szCommandData, uszOutput, atoi(srECROb->srTransData.srCus096_Data.szCommandDataLength));		
		}
		break;
	}while(1);

	return (inRetVal);
}

/*
Function        :IPASS_PollCard
Date&Time       :2022/8/17 下午 2:51
Describe        :和inIPASS_Fast_Tap幾乎相同，但較為簡潔
*/
USHORT IPASS_PollCard(void)
{
	int		inRetVal = VS_SUCCESS;
	char		szTemplate[100 + 1] = {0};
	char		szIPASS_SN[10 + 1] = {0};
	char		szIPASS_SN_Len[1 + 1] = {0};
	unsigned char	uszOutData_IPASS[500 + 1] = {0};
	unsigned int	uiOutDataLen_IPASS = 0;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "IPASS_PollCard() START !");
	}
	
	// TODO: Add your program here //
	memset(szTemplate, 0x00, sizeof(szTemplate));
	szTemplate[0] = 0;
	inRetVal = IPassMP_RequestQueryTicket(0, (unsigned char*)szTemplate, 5,  uszOutData_IPASS, &uiOutDataLen_IPASS);
	if (inRetVal == IPASS_EXECUTE_OK)
	{
		memset(szIPASS_SN_Len, 0x00, sizeof(szIPASS_SN_Len));
		memset(szIPASS_SN, 0x00, sizeof(szIPASS_SN));
		szIPASS_SN_Len[0] = uszOutData_IPASS[0];
		memcpy(szIPASS_SN, &uszOutData_IPASS[1], szIPASS_SN_Len[0]);

		/* 1B卡號長度，10B卡號 */
		inRetVal = IPassMP_GetFirstKey(&uszOutData_IPASS[1], (unsigned char*)szIPASS_SN_Len);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "IPASS_PollCard() END !");
		inLogPrintf(AT, "----------------------------------------");
	}

	if (inRetVal == IPASS_EXECUTE_OK)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/*
Function        :iniPASS_PackResult_Cus096_Internal
Date&Time       :2022/8/19 上午 11:14
Describe        :
*/
int iniPASS_PackResult_Cus096_Internal(char *szData, ECR_TABLE* srECROb)
{
        int     inLen = 0;
	int	inDataLen = 0;
	int	inRemainLen = 0;
        char    szTemp[4 + 1] = {0}, szBuff[576] = {0};
	char	szHex[2 + 1] = {0};

        memset(szBuff, 0x00, sizeof(szBuff));
	/* TransCode */
        memcpy(&szBuff[inLen], srECROb->srTransData.srCus096_Data.szTransType, 2);
        inLen += 2;

        /* Response Code */
        if (srECROb->srTransData.srCus096_Data.uszIsError == VS_TRUE)
        {
		/* Response Code */
		memset(szTemp, 0x00, sizeof(szTemp));
		sprintf(szTemp, "%04X", IPASS_ERROR_NO_CARD);
		memset(szHex, 0x00, sizeof(szHex));
		inFunc_ASCII_to_BCD((unsigned char*)szHex, szTemp, 2);
		
                if (!memcmp(srECROb->srTransData.srCus096_Data.szAPIResponseCode, szHex, 2))
                {
                        if (srECROb->srTransData.srCus096_Data.inErrorType == _ECR_IPASS_CUS096_ERR_TIMEOUT_)
                                memcpy(&szBuff[inLen], "0002", 4);          /* Timeout */
                        else if (srECROb->srTransData.srCus096_Data.inErrorType == _ECR_IPASS_CUS096_ERR_CANCEL_)
                                memcpy(&szBuff[inLen], "0003", 4);          /* User Cancel */
                        else
                                memcpy(&szBuff[inLen], "0001", 4);
                }
                else
                {
                        memcpy(&szBuff[inLen], "0001", 4);
                }
                inLen += 4;

                /* Command Data Length */
                memcpy(&szBuff[inLen], "0000", 4);
                inLen += 4;
		
		/* API Response Code */
		memcpy(&szBuff[inLen], "    ", 4);
                inLen += 4;
		
		/* Command Data */
		memset(&szBuff[inLen], 0x20, 500);
                inLen += 500;
        }
        else
        {
		/* Response Code */
                memcpy(&szBuff[inLen], "0000", 4);
                inLen += 4;

                /* Command Data Length */
                memcpy(&szBuff[inLen], srECROb->srTransData.srCus096_Data.szCommandDataLength, 4);
                inLen += 4;

                /* API Response Code */
                memcpy(&szBuff[inLen], srECROb->srTransData.srCus096_Data.szAPIResponseCode, 2);
                inLen += 2;

                /* szCommandData */
		inDataLen = atoi(srECROb->srTransData.srCus096_Data.szCommandDataLength);
		inRemainLen = 500 - inDataLen;
                memcpy(&szBuff[inLen], srECROb->srTransData.srCus096_Data.szCommandData, inDataLen);
                inLen += inDataLen;
		
		memset(&szBuff[inLen], 0x20, inRemainLen);
		inLen += inRemainLen;
        }
        memcpy(szData, szBuff, _ECR_IPASS_Receive_Size_);

        return (VS_SUCCESS);
}

/*
Function        :iniPASS_Init_Flow_Cus096_Internal
Date&Time       :2022/8/22 上午 9:41
Describe        :
*/
int iniPASS_Init_Flow_Cus096_Internal(unsigned char *uszInData, int inInLen, unsigned char *uszOutData,unsigned int* uiOutLen, int inType)
{
        int     inRetVal = 0;
        uint32_t inInput_Len = 0, inOutput_Len = 0;
        uint8_t szInput[100] = {0}, szOutput[500] = {0};
        char    szTemplate[10] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
	char	szAscii[200 + 1] = {0};
	char	szDebugMsg[100 + 1] = {0};
	unsigned char	uszSlot = 0;
	unsigned char	uszDeviceID[5] = {0};
	DebugPrintFunc	*vdIPassDebug;
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);

   	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        inDISP_ChineseFont("一卡通初始化", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);

   	/* Input Len */
        inInput_Len = inInLen;

        /* Input Data */
   	memset(szInput, 0x00, sizeof(szInput));
        memcpy(&szInput[0], &uszInData[0], inInput_Len);
	
	memset(szOutput, 0x00, sizeof(szOutput));
	inOutput_Len = 0;

	do
	{
		if (inType == _IPASS_COMAND_FIRST_)
		{
			/* 1.初始化SAM slot等等 */
			vdIPassDebug = vdIPASS_Callback_FuncDebug;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetTicket_SAM_Slot(szTemplate);
			if (memcmp(szTemplate, "01", strlen("01")) == 0)
			{
				uszSlot = _SAM_SLOT_1_;
			}
			else if (memcmp(szTemplate, "02", strlen("02")) == 0)
			{
				uszSlot = _SAM_SLOT_2_;
			}
			else if (memcmp(szTemplate, "03", strlen("03")) == 0)
			{
				uszSlot = _SAM_SLOT_3_;
			}
			else if (memcmp(szTemplate, "04", strlen("04")) == 0)
			{
				uszSlot = _SAM_SLOT_4_;
			}

			/* DeviceID是TID八碼以Hex壓成4Bytes */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetTerminalID(szTemplate);

			memset(uszDeviceID, 0x00, sizeof(uszDeviceID));
			inFunc_ASCII_to_BCD(uszDeviceID, &szTemplate[strlen(szTemplate) - 8], 4);

			if (ginDebug == VS_TRUE)
			{
				memset(szAscii, 0x00, sizeof(szAscii));
				inFunc_BCD_to_ASCII(szAscii, uszDeviceID, 4);
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Slot: %d, DeviceID: %s", uszSlot, szAscii);
				inLogPrintf(AT, szDebugMsg);
			}
			/* SAM Slot 0x01~0x08 */
			inRetVal = IPassMP_initLib(vdIPASS_Callback_FuncDebug, uszSlot, uszDeviceID);

			if (inRetVal != VS_SUCCESS)
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("一卡通初始化失敗", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
				inDISP_ChineseFont("Tsam Fail", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
				inDISP_BEEP(3, 1000);

				break;
			}

			/* 2.iPass Reader Auth (in = 31 Byte, out = 72 Byte) */
			memset(szOutput, 0x00, sizeof(szOutput));
			inRetVal = IPassMP_RequestCheckRW(szInput, inInput_Len, szOutput, &inOutput_Len, uszDeviceID);

			if (inRetVal != VS_SUCCESS)
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("一卡通初始化失敗", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
				inDISP_ChineseFont("Check Fail", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
				inDISP_BEEP(3, 1000);

				break;
			}

		}
		else if (inType == _IPASS_COMAND_SECOND_)
		{
			/* 3.iPass Reader Auth */
			memset(szOutput, 0x00, sizeof(szOutput));
			inRetVal = IPassMP_ExcuteCheckRW(szInput, inInput_Len, szOutput, &inOutput_Len);

			if (inRetVal != VS_SUCCESS)
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("一卡通初始化失敗", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
				inDISP_ChineseFont("Excute Fail", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
				inDISP_BEEP(3, 1000);

				break;
			}

			/* 4.Library Check */
			memset(szInput, 0x00, sizeof(szInput));
			memset(szOutput, 0x00, sizeof(szOutput));
			inRetVal = IPassMP_IsRWCheck(szInput, 0, szOutput, &inOutput_Len);

			if (szOutput[0] == 0x01)
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("讀卡機認證完成", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
				inDISP_BEEP(1, 1000);
				
				break;
			}
			else
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("讀卡機認證失敗", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
				inDISP_BEEP(3, 1000);

				break;
			}

		}
		
		break;
	}while(1);
			
	*uiOutLen = inOutput_Len;
	memcpy(uszOutData, szOutput, *uiOutLen);
		
        return (inRetVal);
}