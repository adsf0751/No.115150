//#include <string.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <stdarg.h>
//#include <unistd.h>
//#include <errno.h>
//#include <ctosapi.h>
//
//#include "../SOURCE/INCLUDES/Define_1.h"
//#include "../SOURCE/INCLUDES/Define_2.h"
//#include "../SOURCE/INCLUDES/TransType.h"
//#include "../SOURCE/INCLUDES/Transaction.h"
//#include "../SOURCE/PRINT/Print.h"
//#include "../SOURCE/COMM/Ethernet.h"
//#include "../SOURCE/DISPLAY/Display.h"
//#include "../SOURCE/DISPLAY/DispMsg.h"
//#include "../SOURCE/DISPLAY/DisTouch.h"
//#include "../SOURCE/EVENT/MenuMsg.h"
//#include "../SOURCE/EVENT/Menu.h"
//#include "../SOURCE/EVENT/Flow.h"
//#include "../SOURCE/FUNCTION/CPT.h"
//#include "../SOURCE/FUNCTION/CFGT.h"
//#include "../SOURCE/FUNCTION/Function.h"
//#include "../SOURCE/FUNCTION/EDC.h"
//#include "../SOURCE/COMM/Comm.h"
//#include "eticket.h"
//#include "Ticket.h"
//
//extern int	ginDebug;
//
///*
//Function        :inTicket_ETICKET_Init
//Date&Time       :2016/11/1 下午 4:42
//Describe        :票證初始化流程
//*/
//int inTicket_ETICKET_Init(void)
//{
//#ifndef	_LOAD_KEY_AP_
//	char	szCommMode[2 + 1];
//	
//	memset(szCommMode, 0x00, sizeof(szCommMode));
//	inGetCommMode(szCommMode);
//	/* 目前此library只支援Ethernet */
//	if (memcmp(szCommMode, _COMM_ETHERNET_MODE_, strlen(_COMM_ETHERNET_MODE_)) != 0)
//	{
//		return (VS_ERROR);
//	}
//	
//	
//	inDISP_ClearAll();
//	
//	/* 票證初始化*/
//	ETICKET_Init();
//	
//	#ifdef _DEMO_EDC_
//
//	#else
//	/* 先Call這隻同步票證網路設定 */
//	inTicket_Set_Ethernet_Config();
//	
//	/* IPASS LogOn */
//	inTicket_LogOn(_ETICKET_CARD_IPASS_);
//	#endif
//
//#endif
//	return (VS_SUCCESS);
//}
//
///*
//Function        :inTicket_RunPollingCard
//Date&Time       :2016/11/1 下午 4:42
//Describe        :因為是票證DEMO測試用所以暫放在這
//*/
//int inTicket_RunPollingCard(TRANSACTION_OBJECT *pobTran)
//{
//
//#ifndef	_LOAD_KEY_AP_
//
//	int		inRetVal = 0;
//	char		szDebugMsg[100 + 1];
//	cardInfo	xParam;
//
//	memset(&xParam, 0x00, sizeof(xParam));
//	xParam.CheckListTable |= 0x0000000F;
//	xParam.ulTimeout = 10000;
//	
//	inRetVal = ETICKET_GetCardInfo(&xParam);
//	inFunc_ResetTitle(pobTran);
//	
//	if (inRetVal != d_OK)
//	{
//		memcpy(pobTran->srBRec.szCardLabel, "NO Card", strlen("NO Card"));
//	}
//	else
//	{
//		switch(xParam.CardIndex)
//		{
//		    case d_ETICKET_ECC_CARD:
//			    memcpy(pobTran->srBRec.szCardLabel, "ECC", strlen("ECC"));
//			    break;
//		    case d_ETICKET_IPASS_CARD:
//			    memcpy(pobTran->srBRec.szCardLabel, "IPASS", strlen("IPASS"));
//			    break;
//		    case d_ETICKET_YHDP_CARD:
//			    memcpy(pobTran->srBRec.szCardLabel, "YHDP", strlen("YHDP"));
//			    break;
//		    case d_ETICKET_ICASH_CARD:
//			    memcpy(pobTran->srBRec.szCardLabel, "ICASH", strlen("ICASH"));
//			    break;
//		    default:
//			    memcpy(pobTran->srBRec.szCardLabel, "Unknown Card", strlen("Unknown Card"));
//			    break;
//		}
//	}
//	
//	if (ginDebug == VS_TRUE)
//	{
//		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
//		sprintf(szDebugMsg, "Card Type : %s", pobTran->srBRec.szCardLabel);
//		inLogPrintf(AT, szDebugMsg);
//	}
//    
//#endif
//
//	return (VS_SUCCESS);
//}
//
///*
//Function        :inTicket_RunDeduct
//Date&Time       :2016/11/1 下午 4:42
//Describe        :因為是票證DEMO測試用所以暫放在這
//*/
//int inTicket_RunDeduct(TRANSACTION_OBJECT *pobTran)
//{
//
//#ifndef	_LOAD_KEY_AP_
//
//	int			inRetVal = VS_SUCCESS;
//	int			inChoice = 0;
//	int			inTouchSensorFunc = _Touch_TICKET_CHECK_RESULT_;
//	char			szKey = 0x00;
//	char			szDebugMsg[100 + 1];
//	char			szTemplate[_DISP_MSG_SIZE_ + 1];
//	eTicket_TxnParam	stDeductParam;
//
//	/* 初始化 */
//	memset(&stDeductParam, 0x00, sizeof(stDeductParam));
//	
//	/* 清下排， 顯示請感應卡片 */
//	inDISP_ClearAll();
//	inDISP_PutGraphic(_MENU_TICKET_DEDUCT_TITLE_, 0, _COORDINATE_Y_LINE_8_2_);	/* 第二層顯示 <消費扣款> */
//	inDISP_PutGraphic(_CTLS_RF_, 0, _COORDINATE_Y_LINE_8_3_);			/* 第三層顯示 請感應卡片 */
//	
//	/* 進入畫面時先顯示金額 */
//	stDeductParam.uiAmount = (unsigned int)pobTran->srBRec.lnTxnAmount;
//	
//        memset(szTemplate, 0x00, sizeof(szTemplate));
//        sprintf(szTemplate, "%u", stDeductParam.uiAmount);
//        inFunc_Amount_Comma(szTemplate, "NT$ " , ' ', _SIGNED_NONE_, 16, _PAD_RIGHT_FILL_LEFT_);
//        inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_3_, _COLOR_RED_, _DISP_LEFT_);
//	
//	inRetVal = ETICKET_Deduct(&stDeductParam);
//	
//	inFunc_ResetTitle(pobTran);
//	
//	if (inRetVal != d_OK)
//	{
//		if (ginDebug == VS_TRUE)
//		{
//			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
//			sprintf(szDebugMsg, "ErrCode = %04X", inRetVal);
//			inLogPrintf(AT, szDebugMsg);
//		}
//		
//		inDISP_PutGraphic(_TICKET_RESULT_FAIL_, 0, _COORDINATE_Y_LINE_8_4_);
//		
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		sprintf(szTemplate, "%01ld", pobTran->srBRec.lnTxnAmount);
//		inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);
//
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		sprintf(szTemplate, "%01ld", atol((char*)stDeductParam.bBalance));
//		inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_RIGHT_);
//		
//		inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
//		while (1)
//		{
//			inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
//			szKey = uszKBD_Key();
//
//			
//			/* Timeout */
//			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
//			{
//				szKey = _KEY_TIMEOUT_;
//			}
//			
//			if (szKey == _KEY_CANCEL_				||
//			    inChoice == _TICKET_RESULT_CHECK_Touch_ENTER_)
//			{
//				inRetVal = VS_SUCCESS;
//				break;
//			}
//			else if (szKey == _KEY_TIMEOUT_)
//			{
//				inRetVal = VS_TIMEOUT;
//				break;
//			}
//		}
//		
//	}
//	else
//	{
//		inDISP_PutGraphic(_TICKET_RESULT_SUCCESS_, 0, _COORDINATE_Y_LINE_8_4_);
//		
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		sprintf(szTemplate, "%01ld", pobTran->srBRec.lnTxnAmount);
//		inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);
//
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		sprintf(szTemplate, "%01ld", atol((char*)stDeductParam.bBalance));
//		inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_RIGHT_);
//		
//		inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
//		while (1)
//		{
//			inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
//			szKey = uszKBD_Key();
//
//			/* Timeout */
//			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
//			{
//				szKey = _KEY_TIMEOUT_;
//			}
//
//			if (szKey == _KEY_ENTER_				||
//			    inChoice == _TICKET_RESULT_CHECK_Touch_ENTER_)
//			{
//				inRetVal = VS_SUCCESS;
//				break;
//			}
//			else if (szKey == _KEY_TIMEOUT_)
//			{
//				inRetVal = VS_TIMEOUT;
//				break;
//			}
//		}
//	}
//	/* 清空Touch資料 */
//	inDisTouch_Flush_TouchFile();
//
//#endif
//	return (inRetVal);
//}
//
///*
//Function        :inTicket_RunCheckBalance
//Date&Time       :2016/11/1 下午 4:42
//Describe        :因為是票證DEMO測試用所以暫放在這
//*/
//int inTicket_RunCheckBalance(TRANSACTION_OBJECT *pobTran)
//{
//
//#ifndef	_LOAD_KEY_AP_
//
//	int		inRetVal = 0;
//	char		szKey;
//	char		szDebugMsg[100 + 1];
//	char		szBalance[8 + 1];
//	char		szTemplate[_DISP_MSG_SIZE_ + 1];
////	char		szDispCardType[20 + 1];
//	unsigned char	uszCardType = 0;
//	
//	memset(szBalance, 0x00, sizeof(szBalance));
//	
//	/* 清下排， 顯示請感應卡片 */
//	inDISP_ClearAll();
//	inDISP_PutGraphic(_MENU_TICKET_INQUIRY_TITLE_, 0, _COORDINATE_Y_LINE_8_2_);	/* 第二層顯示 <餘額查詢> */
//	inDISP_PutGraphic(_CTLS_RF_, 0, _COORDINATE_Y_LINE_8_3_);			/* 第三層顯示 請感應卡片 */
//	
////	inDISP_ChineseFont("請感應以偵測卡別", _FONESIZE_8X16_, _LINE_8X23_6_, _DISP_LEFT_);
////	inTicket_RunPollingCard(pobTran);
////	
////	memset(szDispCardType, 0x00, sizeof(szDispCardType));
////	if (memcmp(pobTran->srBRec.szCardLabel, "ECC", strlen("ECC")) == 0)
////	{
////		uszCardType = d_ETICKET_ECC_CARD;
////		memcpy(szDispCardType, "悠遊卡", strlen("悠遊卡"));
////	}
////	else if (memcmp(pobTran->srBRec.szCardLabel, "IPASS", strlen("IPASS")) == 0)
////	{
////		uszCardType = d_ETICKET_IPASS_CARD;
////		memcpy(szDispCardType, "一卡通", strlen("一卡通"));
////	}
////	else if (memcmp(pobTran->srBRec.szCardLabel, "YHDP", strlen("YHDP")) == 0)
////	{
////		uszCardType = d_ETICKET_YHDP_CARD;
////		memcpy(szDispCardType, "有錢卡", strlen("有錢卡"));
////	}
////	else if (memcmp(pobTran->srBRec.szCardLabel, "ICASH", strlen("ICASH")) == 0)
////	{
////		uszCardType = d_ETICKET_ICASH_CARD;
////		memcpy(szDispCardType, "愛金卡", strlen("愛金卡"));
////	}
////	else
////	{
////		/* 找不到卡別 */
////		uszCardType = 0;
////		memcpy(szDispCardType, "未知卡", strlen("未知卡"));
////	}
////	
////	memset(szTemplate, 0x00, sizeof(szTemplate));
////	sprintf(szTemplate, "偵測卡別：%s", szDispCardType);
////	inDISP_ChineseFont(szTemplate, _FONESIZE_8X16_, _LINE_8X23_6_, _DISP_LEFT_);
////	inDISP_ChineseFont("請感應以偵測餘額", _FONESIZE_8X16_, _LINE_8X23_7_, _DISP_LEFT_);
//	
//	inRetVal = ETICKET_CheckBalance((unsigned char*)szBalance, sizeof(szBalance), &uszCardType);
//	
//	inFunc_ResetTitle(pobTran);
//	
//	if (inRetVal != d_OK)
//	{
//		if (ginDebug == VS_TRUE)
//		{
//			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
//			sprintf(szDebugMsg, "ErrCode = %04X", inRetVal);
//			inLogPrintf(AT, szDebugMsg);
//		}
//		
//		inDISP_PutGraphic(_TICKET_RESULT_FAIL_, 0, _COORDINATE_Y_LINE_8_4_);
//		
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		sprintf(szTemplate, "%01ld", pobTran->srBRec.lnTxnAmount);
//		inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);
//
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		sprintf(szTemplate, "%01ld", atol(szBalance));
//		inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_RIGHT_);
//		
//		while (1)
//		{
//			szKey = uszKBD_GetKey(_EDC_TIMEOUT_);
//
//
//			if (szKey == _KEY_CANCEL_)
//			{
//				break;
//			}
//			else if (szKey == _KEY_TIMEOUT_)
//			{
//				return (VS_ERROR);
//			}
//		}
//		
//	}
//	else
//	{
//		inDISP_PutGraphic(_TICKET_RESULT_SUCCESS_, 0, _COORDINATE_Y_LINE_8_4_);
//		
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		sprintf(szTemplate, "%01ld", pobTran->srBRec.lnTxnAmount);
//		inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);
//
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		sprintf(szTemplate, "%01ld", atol(szBalance));
//		inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_RIGHT_);
//	
//		while (1)
//		{
//			szKey = uszKBD_GetKey(_EDC_TIMEOUT_);
//
//			if (szKey == _KEY_ENTER_)
//			{
//				break;
//			}
//			else if (szKey == _KEY_TIMEOUT_)
//			{
//				return (VS_ERROR);
//			}
//		}
//		
//	}
//
//#endif
//
//	return (VS_SUCCESS);
//}
//
///*
//Function        :inTicket_RunTopUp
//Date&Time       :2016/11/1 下午 4:42
//Describe        :因為是票證DEMO測試用所以暫放在這
//*/
//int inTicket_RunTopUp(TRANSACTION_OBJECT *pobTran)
//{
//
//#ifndef	_LOAD_KEY_AP_
//
//	int			inRetVal = 0;
//	char			szKey;
//	char			szDebugMsg[100 + 1];
//	char			szTemplate[_DISP_MSG_SIZE_ + 1];
//	eTicket_TxnParam	stTopUpParam;
//
//	/* 初始化 */
//	memset(&stTopUpParam, 0x00, sizeof(stTopUpParam));
//	
//	/* 清下排， 顯示請感應卡片 */
//	inDISP_ClearAll();
//	inDISP_PutGraphic(_MENU_TICKET_TOP_UP_TITLE_, 0, _COORDINATE_Y_LINE_8_2_);	/* 第二層顯示 <餘額加值> */
//	inDISP_PutGraphic(_CTLS_RF_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 請感應卡片 */
//	
//	/* 進入畫面時先顯示金額 */
//	stTopUpParam.uiAmount = (unsigned int)pobTran->srBRec.lnTxnAmount;
//	
//        memset(szTemplate, 0x00, sizeof(szTemplate));
//        sprintf(szTemplate, "%u",  stTopUpParam.uiAmount);
//        inFunc_Amount_Comma(szTemplate, "NT$ " , ' ', _SIGNED_NONE_, 16, _PAD_RIGHT_FILL_LEFT_);
//        inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_3_, _COLOR_RED_, _DISP_LEFT_);
//	
//	
//	inRetVal = ETICKET_TopUp(&stTopUpParam);
//	if (ginDebug == VS_TRUE)
//	{
//		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
//		sprintf(szDebugMsg, "ETICKET_TopUp Done!");
//		inLogPrintf(AT, szDebugMsg);
//	}
//	
//	inFunc_ResetTitle(pobTran);
//	
//	if (inRetVal != d_OK)
//	{
//		if (ginDebug == VS_TRUE)
//		{
//			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
//			sprintf(szDebugMsg, "ErrCode = %04X", inRetVal);
//			inLogPrintf(AT, szDebugMsg);
//		}
//		
//		inDISP_PutGraphic(_TICKET_RESULT_FAIL_, 0, _COORDINATE_Y_LINE_8_4_);
//		
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		sprintf(szTemplate, "%01ld", pobTran->srBRec.lnTxnAmount);
//		inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);
//
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		sprintf(szTemplate, "%01ld", atol((char*)stTopUpParam.bBalance));
//		inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_RIGHT_);
//		
//		while (1)
//		{
//			szKey = uszKBD_GetKey(_EDC_TIMEOUT_);
//
//			if (szKey == _KEY_CANCEL_)
//			{
//				break;
//			}
//			else if (szKey == _KEY_TIMEOUT_)
//			{
//				return (VS_ERROR);
//			}
//		}
//		
//	}
//	else
//	{
//		inDISP_PutGraphic(_TICKET_RESULT_SUCCESS_, 0, _COORDINATE_Y_LINE_8_4_);
//		
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		sprintf(szTemplate, "%01ld", pobTran->srBRec.lnTxnAmount);
//		inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);
//
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		sprintf(szTemplate, "%01ld", atol((char*)stTopUpParam.bBalance));
//		inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_RIGHT_);
//		
//		while (1)
//		{
//			szKey = uszKBD_GetKey(_EDC_TIMEOUT_);
//
//			if (szKey == _KEY_ENTER_)
//			{
//				break;
//			}
//			else if (szKey == _KEY_TIMEOUT_)
//			{
//				return (VS_ERROR);
//			}
//			
//		}
//		
//	}
//
//#endif
//
//	return (VS_SUCCESS);
//}
//
///*
//Function        :inTicket_Check_Amount
//Date&Time       :2016/11/1 下午 4:42
//Describe        :因為是票證DEMO測試用所以暫放在這
//*/
//int inTicket_Check_Amount(TRANSACTION_OBJECT *pobTran)
//{
//	int	inRetVal = VS_ERROR;
//	int	inChoice = 0;
//	int	inTouchSensorFunc = _Touch_TICKET_CHECK_AMOUNT_;
//	char	szKey = 0x00;
//	char    szAmountMsg[_DISP_MSG_SIZE_ + 1];
//	
//	/* 清下排， 顯示請感應卡片 */
//	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
//	inDISP_PutGraphic(_TICKET_CTLS_CHECK_AMOUNT_, 0, _COORDINATE_Y_LINE_8_4_);
//	
//	memset(szAmountMsg, 0x00, sizeof(szAmountMsg));
//	sprintf(szAmountMsg, "%ld", pobTran->srBRec.lnTxnAmount);
//	
//	inFunc_Amount_Comma(szAmountMsg, " NT$ " , ' ', _SIGNED_NONE_, 18, _PAD_LEFT_FILL_RIGHT_);
//	inDISP_EnglishFont_Point_Color(szAmountMsg, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_,_COLOR_WHITE_, 6);
//	
//	inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
//	/* 執行下載請按0確認，不執行請按清除 */
//	while (1)
//	{
//		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
//                szKey = uszKBD_Key();
//		
//		/* Timeout */
//		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
//		{
//			szKey = _KEY_TIMEOUT_;
//		}
//		
//		if (szKey == _KEY_0_)
//		{
//			inRetVal = VS_SUCCESS;
//			break;
//		}
//		else if (szKey == _KEY_CANCEL_				|| 
//			 inChoice == _TICKET_AMT_CHECK_Touch_KEY_2_)
//		{
//			inRetVal = VS_USER_CANCEL;
//			break;
//		}
//		else if (szKey == _KEY_TIMEOUT_)
//		{
//			inRetVal = VS_TIMEOUT;
//			break;
//		}
//
//	}
//	/* 清空Touch資料 */
//	inDisTouch_Flush_TouchFile();
//	
//	return (inRetVal);
//}
//
///*
//Function        :inTicket_Register
//Date&Time       :2016/11/21 下午 5:21
//Describe        :註冊ipass SAM卡
//*/
//int inTicket_IPASS_Register(TRANSACTION_OBJECT *pobTran)
//{
//	
//#ifndef	_LOAD_KEY_AP_
//
//	int			inRetVal = 0;
//	unsigned char		uszKey;
//	eTicket_IPASSRegister	regIpassParam;
//	
//	memset(&regIpassParam, 0x00, sizeof(regIpassParam));
//	
//	memcpy(regIpassParam.bClientIP, "\x11\x22\x33\x44", 4);
//	regIpassParam.systemID = 0xAC;
//	regIpassParam.serviceProvider = 0xFF;
//	memcpy(regIpassParam.shopNumber, "\x00\x00\x01", 3);
//	regIpassParam.posID = 0x01;
//	memcpy(regIpassParam.subCompanyID, "\xFF\xFF", 2);
//	memcpy(regIpassParam.ipassUserID, "0987654321", 10);
//	memcpy(regIpassParam.ipassUserPWD, "1111122222", 10);
//	memcpy(regIpassParam.spUserID, "1234567890", 10);
//	memcpy(regIpassParam.spUserPWD, "2222211111", 10);		
//	inRetVal = ETICKET_Register(d_ETICKET_IPASS_CARD, (void *)&regIpassParam);
//	
//	if (inRetVal != d_OK)
//	{
//		printf("Register IPASS SAM fail");
//		
//		return (VS_ERROR);
//	}
//
//	/* 註冊SAM卡完成請按0確認 */
//	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
//	inDISP_PutGraphic(_TSAM_REGISTER_OK_, 0, _COORDINATE_Y_LINE_8_4_);
//	inDISP_BEEP(1, 0);
//
//	while (1)
//	{
//		uszKey = uszKBD_GetKey(30);
//
//		if (uszKey == _KEY_0_ || _KEY_TIMEOUT_)
//		{            
//			break;
//		}
//		else
//		{
//			continue;
//		}
//
//	}
//
//#endif
//
//	return (VS_SUCCESS);
//}
//
///*
//Function        :inTicket_Set_Ethernet_Config
//Date&Time       :2016/11/21 下午 5:21
//Describe        :改完NEXSYS程式的網路設定要call這隻function使得ETICKET_Init的網路設定和NEXSYS程式同步
//*/
//int inTicket_Set_Ethernet_Config()
//{
//
//#ifndef	_LOAD_KEY_AP_
//
//	char		szEDCIPAddress[16 + 1], szSubMask[16 + 1], szGateWay[16 + 1];
//	char		szHostIPPrimary[16 + 1];
//	char		szHostPortNoPrimary[6 + 1];
//	char		szConfig[16 + 1];
//	char		szDebugMsg[100 + 1];
//	char		szDHCPMode[2 + 1];
//	char		szTemplate[10 + 1];
//	unsigned short	usRetval;
//
//        if (ginDebug == VS_TRUE)
//                inLogPrintf(AT, "inTicket_Set_Ethernet_Config() START!");
//
//        memset(szEDCIPAddress, 0x00, sizeof(szEDCIPAddress));
//        memset(szSubMask, 0x00, sizeof(szSubMask));
//        memset(szGateWay, 0x00, sizeof(szGateWay));
//
//        if (inGetTermIPAddress(szEDCIPAddress) == VS_ERROR)
//        {
//                /* inGetEDCIP ERROR */
//                /* debug */
//                if (ginDebug == VS_TRUE)
//                        inLogPrintf(AT, "inGetEDCIP() ERROR!!");
//                return (VS_ERROR);
//        }
//        
//        if (inGetTermGetewayAddress(szGateWay) == VS_ERROR)
//        {
//                /* inGetGateWay ERROR */
//                /* debug */
//                if (ginDebug == VS_TRUE)
//                        inLogPrintf(AT, "inGetGateWay() ERROR!!");
//                return (VS_ERROR);
//        }
//        
//        if (inGetTermMASKAddress(szSubMask) == VS_ERROR)
//        {
//                /* inGetSubMask ERROR */
//                /* debug */
//                if (ginDebug == VS_TRUE)
//                        inLogPrintf(AT, "inGetSubMask() ERROR!!");
//                return (VS_ERROR);
//        }
//
//	/* Set EDC IP Address */
//	if (ETICKET_SetParameter(d_ETICKET_ETHERNET_IP, (BYTE *)szEDCIPAddress, strlen(szEDCIPAddress)) != d_OK)
//	{
//                if (ginDebug == VS_TRUE)
//                        inLogPrintf(AT, "ETICKET─d_ETHERNET_CONFIG_IP Error");
//                
//		return (VS_ERROR);
//	}
//
//	/* Set EDC SUB Mask */
//	if (ETICKET_SetParameter(d_ETICKET_ETHERNET_MASK, (BYTE *)szSubMask, strlen(szSubMask)) != d_OK)
//	{
//                if (ginDebug == VS_TRUE)
//                        inLogPrintf(AT, "ETICKET─d_ETHERNET_CONFIG_MASK Error");
//                
//		return (VS_ERROR);
//	}
//
//	/* Set Geteway IP */
//	if (ETICKET_SetParameter(d_ETICKET_ETHERNET_GATEWAY, (BYTE *)szGateWay, strlen(szGateWay)) != d_OK)
//	{
//                if (ginDebug == VS_TRUE)
//                        inLogPrintf(AT, "ETICKET─d_ETHERNET_CONFIG_GATEWAY Error");
//	
//                return (VS_ERROR);
//	}
//        
//	/* Set Host IP */
//        memset(szHostIPPrimary,0x00,sizeof(szHostIPPrimary));
//        
//        if (inGetHostIPPrimary(szHostIPPrimary) == VS_ERROR)
//        {
//                /* inGetHostIPPrimary ERROR */
//                /* debug */
//                if (ginDebug == VS_TRUE)
//                        inLogPrintf(AT, "inGetHostIPPrimary() ERROR!!");
//
////                return (VS_ERROR);
//        }
//        
//	usRetval = ETICKET_SetParameter(d_ETICKET_ETHERNET_HOSTIP, (BYTE *)szHostIPPrimary, strlen(szHostIPPrimary));
//	if (usRetval != d_OK)
//	{
//		if (ginDebug == VS_TRUE)
//                {
//			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
//                        sprintf(szDebugMsg, "ETICKET─d_ETHERNET_CONFIG_HOSTIP Error : 0x%04x", usRetval);
//                        inLogPrintf(AT, szDebugMsg);
//                    
//                }
//	
//                return (VS_ERROR);
//	}
//
//	/* Set Host Port */
//        memset(szHostPortNoPrimary,0x00,sizeof(szHostPortNoPrimary));
//        
//        if (inGetHostPortNoPrimary(szHostPortNoPrimary) == VS_ERROR)
//        {
//                /* Get HostPortNumber Primary ERROR */
//                /* debug */
//                if (ginDebug == VS_TRUE)
//                        inLogPrintf(AT, "inGetHostPortNoPrimary() ERROR!!");
//                
////                return (VS_ERROR);
//        }
//        
//	usRetval = ETICKET_SetParameter((BYTE)d_ETICKET_ETHERNET_HOSTPORT, (BYTE *)szHostPortNoPrimary, strlen(szHostPortNoPrimary));
//	if (usRetval != d_OK)
//	{
//                if (ginDebug == VS_TRUE)
//                        inLogPrintf(AT, "ETICKET─d_ETHERNET_CONFIG_HOSTPORT Error");
//	
//                return (VS_ERROR);
//	}
//        
//        /*
//                d_ETHERNET_CONFIG_AUTOCON
//                Set Connection Mode.
//                = 0 : Auto-connect. When Auto-connect is set, the Ethernet module will automatically try to connect to the host every 5 seconds.
//                = 1 : Not support
//                = 2 : Manual. The connection must be established manually by calling CTOS_EthernetConnectxxx() function.
//        */
//        /* 預設成2 */
//	inGetAutoConnect(szTemplate);
//	memset(szConfig, 0x00, sizeof(szConfig));
//	if (szTemplate[0] == '1')
//	{
//		szConfig[0] = 0x30;
//	}
//        else
//	{
//		szConfig[0] = 0x32;
//	}
//	
//	usRetval = ETICKET_SetParameter(d_ETICKET_ETHERNET_AUTOCON, (BYTE *)szConfig, 1);
//	if (usRetval != d_OK)
//	{
//                if (ginDebug == VS_TRUE)        
//                        inLogPrintf(AT, "ETICKET─d_ETHERNET_CONFIG_AUTOCON Error");
//		
//                return (VS_ERROR);
//	}
//
//        /*
//                d_ETHERNET_CONFIG_DHCP
//                Set IP configuration.
//                = 0 : Static. Use the static IP set in the Ethernet configuration.
//                = 1 : DHCP. Retrieve the dynamic IP from the DHCP server.
//        */
//        /* DHCP預設0 */
//	memset(szConfig, 0x00, sizeof(szConfig));
//	memset(szDHCPMode, 0x00, sizeof(szDHCPMode));
//	inGetDHCP_Mode(szDHCPMode);
//	if (memcmp(szDHCPMode, "Y", strlen("Y")) == 0)
//	{
//		szConfig[0] = 0x31;
//	}
//	else
//	{
//		szConfig[0] = 0x30;
//	}
//	
//	if (ETICKET_SetParameter(d_ETICKET_ETHERNET_DHCP, (BYTE *)szConfig, 1) != d_OK)
//	{
//                if (ginDebug == VS_TRUE)
//                        inLogPrintf(AT, "ETICKET─d_ETHERNET_CONFIG_DHCP Error");
//	
//                return (VS_ERROR);
//	}
//	
//        if (ginDebug == VS_TRUE)
//                inLogPrintf(AT, "inTicket_Set_Ethernet_Config() END!");
//#endif
//        
//        return (VS_SUCCESS);
//}
//
///*
//Function        :inTicket_LogOn
//Date&Time       :2016/11/28 下午 3:56
//Describe        :LogOn流程
//*/
//int inTicket_LogOn(unsigned char uszCardIndex)
//{
//
//#ifndef	_LOAD_KEY_AP_
//
//	int	inRetVal;
//	char	szDebugMsg[100 + 1];
//	
//	inRetVal = ETICKET_LogOn(uszCardIndex);
//	
//	if (inRetVal != VS_SUCCESS)
//	{
//		if (ginDebug == VS_TRUE)
//		{
//			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
//			sprintf(szDebugMsg, "ErrCode = %04X", inRetVal);
//			inLogPrintf(AT, szDebugMsg);
//		}
//		
//	}
//
//#endif
//
//	return (VS_SUCCESS);
//}
