#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <ctosapi.h>
#include "../SOURCE/INCLUDES/Define_1.h"
#include "../SOURCE/INCLUDES/Define_2.h"
#include "../SOURCE/INCLUDES/Transaction.h"
#include "../SOURCE/INCLUDES/TransType.h"
#include "../SOURCE/INCLUDES/AllStruct.h"
#include "../SOURCE/DISPLAY/Display.h"
#include "../SOURCE/DISPLAY/DispMsg.h"
#include "../SOURCE/DISPLAY/DisTouch.h"
#include "../SOURCE/PRINT/Print.h"
#include "../SOURCE/FUNCTION/Accum.h"
#include "../SOURCE/FUNCTION/Function.h"
#include "../SOURCE/FUNCTION/FuncTable.h"
#include "../SOURCE/FUNCTION/File.h"
#include "../SOURCE/FUNCTION/Card.h"
#include "../SOURCE/FUNCTION/CDT.h"
#include "../SOURCE/FUNCTION/CFGT.h"
#include "../SOURCE/FUNCTION/HDT.h"
#include "../SOURCE/FUNCTION/EDC.h"
#include "../SOURCE/FUNCTION/PCD.h"
#include "../SOURCE/FUNCTION/ECR.h"
#include "../SOURCE/FUNCTION/Utility.h"
#include "../SOURCE/EVENT/Flow.h"
#include "../SOURCE/EVENT/MenuMsg.h"
#include "../NCCC/NCCCsrc.h"
#include "../NCCC/NCCCTrust.h"
#include "../NCCC/TAKAsrc.h"
#include "CreditFunc.h"

extern	int		ginDebug;	/* Debug使用 extern */
extern	int		ginEventCode;
extern	int		ginMenuKeyIn;
extern	int		ginFindRunTime;
extern	char		gszTermVersionID[16 + 1];
extern	ECR_TABLE	gsrECROb;

/*
Function	:inCREDIT_Func_GetAmount
Date&Time	:2015/8/11 下午 4:27
Describe	:輸入金額
*/
int inCREDIT_Func_GetAmount(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
        char		szTemplate[_DISP_MSG_SIZE_ + 1];
        DISPLAY_OBJECT  srDispObj;
        
        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_GetAmount START!");
	
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
	}

	/* 若ECR或idle有輸入金額就跳走，OPT已經輸入過 */
        if (pobTran->uszECRBit == VS_TRUE)
	{
		if (pobTran->srBRec.lnTxnAmount > 0)
		{
                        vdUtility_SYSFIN_LogMessage(AT, "ECR inCREDIT_Func_GetAmount Amt %ld END!", pobTran->srBRec.lnTxnAmount);
                        
			return (VS_SUCCESS);
		}
		else
		{
			
		}
	}
	else if (pobTran->srBRec.lnTxnAmount > 0)
	{
                vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_GetAmount Amt %ld END!", pobTran->srBRec.lnTxnAmount);
                
		return (VS_SUCCESS);
	}
        
        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
        memset(szTemplate, 0x00, sizeof(szTemplate));

        srDispObj.inY = _LINE_8_7_;
        srDispObj.inR_L = _DISP_RIGHT_;
        srDispObj.inMaxLen = inFunc_Check_Digit();      /* 不可超過9，long變數最多放9位 */
	srDispObj.inCanNotBypass = VS_TRUE;
	srDispObj.inCanNotZero = VS_TRUE;
	srDispObj.inColor = _COLOR_RED_;
	srDispObj.inTouchSensorFunc = _Touch_OX_LINE8_8_;
        strcpy(srDispObj.szPromptMsg, "NT$ ");

        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        /* 輸入金額 */
        inDISP_PutGraphic(_GET_AMOUNT_, 0, _COORDINATE_Y_LINE_8_4_);
	inDISP_PutGraphic(_MSG_ENTER_OR_CANCEL_, 0, _COORDINATE_Y_LINE_8_8_);

        /* 進入畫面時先顯示金額為0 */
        strcpy(szTemplate, "NT$ 0");
        inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, srDispObj.inColor, _DISP_RIGHT_);

	memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
	srDispObj.inOutputLen = 0;

        inRetVal = inDISP_Enter8x16_GetAmount(&srDispObj);

        if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
	{
		vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_GetAmount Timeout_Or_UserCancel(%d) END!", inRetVal);
		return (inRetVal);
	}

        pobTran->srBRec.lnTxnAmount = atol(srDispObj.szOutput);
        pobTran->srBRec.lnOrgTxnAmount = atol(srDispObj.szOutput);
        pobTran->srBRec.lnTotalTxnAmount = atol(srDispObj.szOutput);
        
        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_GetAmount END!");
        
	return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_Get_OPT_Amount
Date&Time	:2016/12/29 上午 11:20
Describe	:輸入金額
*/
int inCREDIT_Func_Get_OPT_Amount(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
        char		szTemplate[_DISP_MSG_SIZE_ + 1];
        DISPLAY_OBJECT  srDispObj;
        
        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_Amount START");

	/* idle進入 */
	if (ginEventCode >= '1' && ginEventCode <= '9')
        {
		/* 若ECR或idle有輸入金額就跳走，OPT已經輸入過 */
		if (pobTran->uszECRBit == VS_TRUE)
		{
			if (pobTran->srBRec.lnTxnAmount > 0)
			{
				return (VS_SUCCESS);
			}
                        
			/* 兩段式收銀機連線 第一段不輸入 這邊只擋會出現在OPT的 */
			if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
			{
				return (VS_SUCCESS);
			}
		}

		memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		srDispObj.inY = _LINE_8_7_;
		srDispObj.inR_L = _DISP_RIGHT_;
		srDispObj.inMaxLen = inFunc_Check_Digit();      /* 不可超過9，long變數最多放9位 */
		srDispObj.inMenuKeyIn = ginEventCode;
		srDispObj.inCanNotBypass = VS_TRUE;
		srDispObj.inCanNotZero = VS_TRUE;
		srDispObj.inColor = _COLOR_RED_;
		srDispObj.inTouchSensorFunc = _Touch_OX_LINE8_8_;
		strcpy(srDispObj.szPromptMsg, "NT$ ");

		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		/* 輸入金額 */
		inDISP_PutGraphic(_GET_AMOUNT_, 0, _COORDINATE_Y_LINE_8_4_);
		inDISP_PutGraphic(_MSG_ENTER_OR_CANCEL_, 0, _COORDINATE_Y_LINE_8_8_);

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;
	
		inRetVal = inDISP_Enter8x16_GetAmount(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
		{
			vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_Amount Timeout_Or_UserCancel(%d) END!", inRetVal);
			return (inRetVal);
		}

		pobTran->srBRec.lnTxnAmount = atol(srDispObj.szOutput);
		pobTran->srBRec.lnOrgTxnAmount = atol(srDispObj.szOutput);
		pobTran->srBRec.lnTotalTxnAmount = atol(srDispObj.szOutput);
	}
        
        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_Amount END");
         
	return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_GetRefundAmount
Date&Time	:2017/1/3 上午 11:05
Describe	:輸入退貨金額
*/
int inCREDIT_Func_GetRefundAmount(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
        char		szTemplate[_DISP_MSG_SIZE_ + 1];
        DISPLAY_OBJECT  srDispObj;

        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_GetRefundAmount START!");
        
	//* 若ECR或idle有輸入金額就跳走，OPT已經輸入過 */
        if (pobTran->uszECRBit == VS_TRUE)
	{
		if (pobTran->srBRec.lnTxnAmount > 0)
		{
			return (VS_SUCCESS);
		}
		else
		{
			
		}
	}
	else if (pobTran->srBRec.lnTxnAmount > 0)
	{
		return (VS_SUCCESS);
	}
        
        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
        memset(szTemplate, 0x00, sizeof(szTemplate));

        srDispObj.inY = _LINE_8_7_;
        srDispObj.inR_L = _DISP_RIGHT_;
        srDispObj.inMaxLen = inFunc_Check_Digit();		/* 不可超過9，long變數最多放9位 */
	srDispObj.inCanNotBypass = VS_TRUE;
	srDispObj.inCanNotZero = VS_TRUE;
	srDispObj.inColor = _COLOR_RED_;
        strcpy(srDispObj.szPromptMsg, "NT$ ");

        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        /* 輸入金額 */
        inDISP_PutGraphic(_GET_REFUND_AMOUNT_, 0, _COORDINATE_Y_LINE_8_4_);

        /* 進入畫面時先顯示金額為0 */
        strcpy(szTemplate, "NT$ 0");
        inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, srDispObj.inColor, _DISP_RIGHT_);

	memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
	srDispObj.inOutputLen = 0;
	
        inRetVal = inDISP_Enter8x16_GetAmount(&srDispObj);

        if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
		return (inRetVal);

        pobTran->srBRec.lnTxnAmount = atol(srDispObj.szOutput);
        pobTran->srBRec.lnOrgTxnAmount = atol(srDispObj.szOutput);
        pobTran->srBRec.lnTotalTxnAmount = atol(srDispObj.szOutput);
         
	return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_Get_OPT_RefundAmount
Date&Time	:2017/6/28 上午 10:24
Describe	:輸入退貨金額
*/
int inCREDIT_Func_Get_OPT_RefundAmount(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
        char		szTemplate[_DISP_MSG_SIZE_ + 1];
        DISPLAY_OBJECT  srDispObj;
        
        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_RefundAmount START!");

	/* 若ECR或idle有輸入金額就跳走，OPT已經輸入過 */
        if (pobTran->uszECRBit == VS_TRUE)
	{
		if (pobTran->srBRec.lnTxnAmount > 0)
		{
                        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_RefundAmount ECR_lnTxnAmount>0(%ld) END!", pobTran->srBRec.lnTxnAmount);
                        
			return (VS_SUCCESS);
		}
                
		/* 兩段式收銀機連線 第一段不輸入 這邊只擋會出現在OPT的 */
		if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
		{
                        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_RefundAmount END!");
			return (VS_SUCCESS);
		}
	}
        
        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
        memset(szTemplate, 0x00, sizeof(szTemplate));

        srDispObj.inY = _LINE_8_7_;
        srDispObj.inR_L = _DISP_RIGHT_;
        srDispObj.inMaxLen = inFunc_Check_Digit();		/* 不可超過9，long變數最多放9位 */
	srDispObj.inCanNotBypass = VS_TRUE;
	srDispObj.inCanNotZero = VS_TRUE;
	srDispObj.inColor = _COLOR_RED_;
        strcpy(srDispObj.szPromptMsg, "NT$ ");

        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        /* 輸入金額 */
        inDISP_PutGraphic(_GET_REFUND_AMOUNT_, 0, _COORDINATE_Y_LINE_8_4_);

        /* 進入畫面時先顯示金額為0 */
        strcpy(szTemplate, "NT$ 0");
        inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, srDispObj.inColor, _DISP_RIGHT_);

	memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
	srDispObj.inOutputLen = 0;
	
        inRetVal = inDISP_Enter8x16_GetAmount(&srDispObj);

        if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
		return (inRetVal);

        pobTran->srBRec.lnTxnAmount = atol(srDispObj.szOutput);
        pobTran->srBRec.lnOrgTxnAmount = atol(srDispObj.szOutput);
        pobTran->srBRec.lnTotalTxnAmount = atol(srDispObj.szOutput);
         
        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_RefundAmount END!");
        
	return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_GetStoreID
Date&Time	:2015/8/11 下午 4:27
Describe	:輸入櫃號
*/
int inCREDIT_Func_GetStoreID(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_SUCCESS;
	int		inStoreIDLen = 18;	/* 預設值為18 */
	int		inMinStoreIDLen = 0;
	int		inMaxStoreIDLen = 0;
//        int             inX1 = 0, inX2 = 0;
	char		szMinStoreIDLen[2 + 1] = {0};
	char		szMaxStoreIDLen[2 + 1] = {0};
	char		szStoreIDEnable[2 + 1] = {0};
	char		szCustomerIndicator[3 + 1] = {0};
	char		szPOS_ID[6 + 1] = {0};
        DISPLAY_OBJECT  srDispObj = {};
        
        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_GetStoreID START!");
        
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
	}
	
	memset(szStoreIDEnable, 0x00, sizeof(szStoreIDEnable));
	inGetStoreIDEnable(szStoreIDEnable);
	if (memcmp(szStoreIDEnable, "Y", strlen("Y")) != 0)
	{
		vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_GetStoreID not enable END!");
		return (VS_SUCCESS);
	}

	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	/* 若ECR發動直接跳走或前面已輸入過櫃號(For idle 進入流程) */
  	if (pobTran->uszECRBit == VS_TRUE)
	{
		if (strlen(pobTran->srBRec.szStoreID) > 0)
		{
                        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_GetStoreID store_id_len > 0 END!");
			return (VS_SUCCESS);
		}
			
		/* 喜滿客無法輸入櫃號，亦不出現櫃號輸入流程 */
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_079_CINEMARK_, _CUSTOMER_INDICATOR_SIZE_))
		{
                        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_GetStoreID cus_079cinemark END!");
                        return (VS_SUCCESS);
                }
	}
	else if (memcmp(gszTermVersionID, "MD731UAGAS001", strlen("MD731UAGAS001")) == 0)
	{
		if (strlen(pobTran->srBRec.szStoreID) > 0)
		{
                        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_GetStoreID store_id_len > 0 END!");
			return (VS_SUCCESS);
		}
	}
	else 
	{
		if (strlen(pobTran->srBRec.szStoreID) > 0)
		{
                        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_GetStoreID store_id_len > 0 END!");
			return (VS_SUCCESS);
		}
	}
	
        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
	memset(szMinStoreIDLen, 0x00, sizeof(szMinStoreIDLen));
	memset(szMaxStoreIDLen, 0x00, sizeof(szMinStoreIDLen));
	
	inGetMinStoreIDLen(szMinStoreIDLen);
	inGetMaxStoreIDLen(szMaxStoreIDLen);
	inMinStoreIDLen = atoi(szMinStoreIDLen);
	inMaxStoreIDLen = atoi(szMaxStoreIDLen);
	
	/* (需求單 - 109327)V3機型開發支援Vx520的客製化參數需求 (002-耐斯廣場/王子大飯店) by Russell 2020/10/6 下午 4:46 */
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_002_NICE_PLAZA_, _CUSTOMER_INDICATOR_SIZE_))
	{
                /* TMS不會給對的 */
		inStoreIDLen = 20;
                inMaxStoreIDLen = 20;
	}
	else
	{
		inStoreIDLen = 18;
	}
	
        srDispObj.inY = _LINE_8_5_;
        srDispObj.inR_L = _DISP_LEFT_;
        srDispObj.inMaxLen = inMaxStoreIDLen;
	srDispObj.inColor = _COLOR_RED_;
//        srDispObj.uszUseCustomKeyAlphaBit = VS_TRUE;
//        srDispObj.uszCustomKeyAlpha = _KEY_FUNCTION_;

	while (1)
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		/* 輸入櫃號 */
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_026_TAKA_, _CUSTOMER_INDICATOR_SIZE_) ||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inDISP_ChineseFont("請輸入品群碼?", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_LEFT_);
		}
		else
		{
			inDISP_ChineseFont("請輸入櫃號", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_LEFT_);
		}
		inDISP_ChineseFont("英/數字切換請按#鍵", _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);
                
//                inX1 = 0;
//                inX2 = 85;
//                inDISP_ChineseFont_Point_Color_By_Graphic_Mode("       ", _FONTSIZE_8X16_, _COLOR_RED_, _COLOR_RED_, inX1, _Distouch_OPT8X16_LINE_8_Y1_, VS_FALSE);
//                inDISP_ChineseFont_Point_Color_By_Graphic_Mode("X|取消", _FONTSIZE_8X22_, _COLOR_WHITE_, _COLOR_RED_, inX1, _Distouch_OPT8X16_LINE_8_Y1_, VS_FALSE);
//                inDISP_ChineseFont_Point_Color_By_Graphic_Mode("CANCEL", _FONTSIZE_16X44_, _COLOR_WHITE_, _COLOR_RED_, inX2, _Distouch_OPT16X22_LINE_16_Y1_, VS_FALSE);
//                inX1 = 160;
//                inX2 = 245;
//                inDISP_ChineseFont_Point_Color_By_Graphic_Mode("       ", _FONTSIZE_8X16_, _COLOR_BUTTON_GREEN_, _COLOR_BUTTON_GREEN_, inX1, _Distouch_OPT8X16_LINE_8_Y1_, VS_FALSE);
//                inDISP_ChineseFont_Point_Color_By_Graphic_Mode("O|確認", _FONTSIZE_8X22_, _COLOR_WHITE_, _COLOR_BUTTON_GREEN_, inX1, _Distouch_OPT8X16_LINE_8_Y1_, VS_FALSE);
//                inDISP_ChineseFont_Point_Color_By_Graphic_Mode("ENTER", _FONTSIZE_16X44_, _COLOR_WHITE_, _COLOR_BUTTON_GREEN_, inX2, _Distouch_OPT16X22_LINE_16_Y1_, VS_FALSE);
	
		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;

		inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
		{
			vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_GetStoreID Timeout_Or_UserCancel(%d) END!", inRetVal);
			return (inRetVal);
		}

		if (srDispObj.inOutputLen >= 0)
		{
			/* 長度不符合，清空 */
			if (srDispObj.inOutputLen < inMinStoreIDLen	||
			    srDispObj.inOutputLen > inMaxStoreIDLen)
			{
				continue;
			}

			memcpy(&pobTran->srBRec.szStoreID[0], &srDispObj.szOutput[0], srDispObj.inOutputLen);
			/* 櫃號不滿長度，補空白 */
			if (srDispObj.inOutputLen < inStoreIDLen)
			{
				memset(&pobTran->srBRec.szStoreID[srDispObj.inOutputLen], 0x20, inStoreIDLen - srDispObj.inOutputLen);
			}
			break;
		}
		
	}
	
	/* 客製化026 櫃號後6碼要改成POS ID，放後面，避免被手動的覆蓋了 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_026_TAKA_, _CUSTOMER_INDICATOR_SIZE_)	||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
	{
		memset(szPOS_ID, 0x00, sizeof(szPOS_ID));
		inGetPOS_ID(szPOS_ID);
		memcpy(&pobTran->srBRec.szStoreID[12], szPOS_ID, 6);
	}
	
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        
	vdUtility_SYSFIN_LogMessage(AT, " inCREDIT_Func_GetStoreID END!");

	return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_Get_OPT_StoreID
Date&Time	:2016/12/29 上午 11:11
Describe	:for idle進入流程用，輸入櫃號
*/
int inCREDIT_Func_Get_OPT_StoreID(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_SUCCESS;
	int		inStoreIDLen = 18;	/* 預設值為18 */
	int		inMinStoreIDLen = 0;
	int		inMaxStoreIDLen = 0;
//        int             inX1 = 0, inX2 = 0;
	char		szMinStoreIDLen[2 + 1] = {0};
	char		szMaxStoreIDLen[2 + 1] = {0};
	char		szStoreIDEnable[2 + 1] = {0};
	char		szCustomerIndicator[3 + 1] = {0};
	char		szPOS_ID[6 + 1] = {0};
        DISPLAY_OBJECT  srDispObj = {};
        
        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_StoreID START");
	
	memset(szStoreIDEnable, 0x00, sizeof(szStoreIDEnable));
	inGetStoreIDEnable(szStoreIDEnable);
	if (memcmp(szStoreIDEnable, "Y", strlen("Y")) != 0)
	{
                vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_StoreID NotEnable_Pass END!");
		return (VS_SUCCESS);
	}
        
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	/* 若ECR發動直接跳走或前面已輸入過櫃號(For idle 進入流程) */
        if (pobTran->uszECRBit == VS_TRUE)
	{
		if (strlen(pobTran->srBRec.szStoreID) > 0)
		{
                        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_StoreID zStoreID)>0 END!");
			return (VS_SUCCESS);
		}
                
		/* 兩段式收銀機連線 第一段不輸入 這邊只擋會出現在OPT的 */
		if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
		{
                        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_StoreID InquiryFirst=True END!");
			return (VS_SUCCESS);
		}
		
		/* 收銀機資訊不完整時要先過卡再輸入，也就是到TRT再執行檢核 */
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_002_NICE_PLAZA_, _CUSTOMER_INDICATOR_SIZE_))
		{
                        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_StoreID Cus_002_Nice_Plaza END!");
			return (VS_SUCCESS);
		}
		/* 喜滿客無法輸入櫃號，亦不出現櫃號輸入流程 */
		else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_079_CINEMARK_, _CUSTOMER_INDICATOR_SIZE_))
		{
                        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_StoreID Cus_079_Cinemark_ END!");
                        return (VS_SUCCESS);
                }
	}
	else
	{
		if (strlen(pobTran->srBRec.szStoreID) > 0)
		{
                        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_StoreID szStoreID)>0 END!");
			return (VS_SUCCESS);
		}
	}

	memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
	memset(szMinStoreIDLen, 0x00, sizeof(szMinStoreIDLen));
	memset(szMaxStoreIDLen, 0x00, sizeof(szMinStoreIDLen));

	inGetMinStoreIDLen(szMinStoreIDLen);
	inGetMaxStoreIDLen(szMaxStoreIDLen);
	inMinStoreIDLen = atoi(szMinStoreIDLen);
	inMaxStoreIDLen = atoi(szMaxStoreIDLen);

	/* (需求單 - 109327)V3機型開發支援Vx520的客製化參數需求 (002-耐斯廣場/王子大飯店) by Russell 2020/10/6 下午 4:46 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_002_NICE_PLAZA_, _CUSTOMER_INDICATOR_SIZE_))
	{
		/* TMS不會給對的 */
		inStoreIDLen = 20;
                inMaxStoreIDLen = 20;
	}
	else
	{
		inStoreIDLen = 18;
	}

	srDispObj.inY = _LINE_8_5_;
	srDispObj.inR_L = _DISP_LEFT_;
	srDispObj.inMaxLen = inMaxStoreIDLen;
	srDispObj.inColor = _COLOR_RED_;
//        srDispObj.uszUseCustomKeyAlphaBit = VS_TRUE;
//        srDispObj.uszCustomKeyAlpha = _KEY_FUNCTION_;
	
	while (1)
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_026_TAKA_, _CUSTOMER_INDICATOR_SIZE_) ||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inDISP_ChineseFont("請輸入品群碼?", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_LEFT_);
		}
		else
		{
                        inDISP_ChineseFont("請輸入櫃號", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_LEFT_);
		}
                inDISP_ChineseFont("英/數字切換請按#鍵", _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);
                
//                inX1 = 0;
//                inX2 = 85;
//                inDISP_ChineseFont_Point_Color_By_Graphic_Mode("       ", _FONTSIZE_8X16_, _COLOR_RED_, _COLOR_RED_, inX1, _Distouch_OPT8X16_LINE_8_Y1_, VS_FALSE);
//                inDISP_ChineseFont_Point_Color_By_Graphic_Mode("X|取消", _FONTSIZE_8X22_, _COLOR_WHITE_, _COLOR_RED_, inX1, _Distouch_OPT8X16_LINE_8_Y1_, VS_FALSE);
//                inDISP_ChineseFont_Point_Color_By_Graphic_Mode("CANCEL", _FONTSIZE_16X44_, _COLOR_WHITE_, _COLOR_RED_, inX2, _Distouch_OPT16X22_LINE_16_Y1_, VS_FALSE);
//                inX1 = 160;
//                inX2 = 245;
//                inDISP_ChineseFont_Point_Color_By_Graphic_Mode("       ", _FONTSIZE_8X16_, _COLOR_BUTTON_GREEN_, _COLOR_BUTTON_GREEN_, inX1, _Distouch_OPT8X16_LINE_8_Y1_, VS_FALSE);
//                inDISP_ChineseFont_Point_Color_By_Graphic_Mode("O|確認", _FONTSIZE_8X22_, _COLOR_WHITE_, _COLOR_BUTTON_GREEN_, inX1, _Distouch_OPT8X16_LINE_8_Y1_, VS_FALSE);
//                inDISP_ChineseFont_Point_Color_By_Graphic_Mode("ENTER", _FONTSIZE_16X44_, _COLOR_WHITE_, _COLOR_BUTTON_GREEN_, inX2, _Distouch_OPT16X22_LINE_16_Y1_, VS_FALSE);
	
		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;

		inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
		{
			vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_StoreID Timeout_Or_UserCancel(%d) END!", inRetVal);
			return (inRetVal);
		}

		if (srDispObj.inOutputLen >= 0)
		{
			/* 長度不符合，清空 */
			if (srDispObj.inOutputLen < inMinStoreIDLen	||
			    srDispObj.inOutputLen > inMaxStoreIDLen)
			{
				memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
				continue;
			}

			memcpy(&pobTran->srBRec.szStoreID[0], &srDispObj.szOutput[0], srDispObj.inOutputLen);
			/* 櫃號不滿長度，補空白 */
			if (srDispObj.inOutputLen < inStoreIDLen)
			{
				memset(&pobTran->srBRec.szStoreID[srDispObj.inOutputLen], 0x20, inStoreIDLen - srDispObj.inOutputLen);
			}

			break;
		}

	}
	
	/* 客製化026 櫃號後6碼要改成POS ID，放後面，避免被手動的覆蓋了 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_026_TAKA_, _CUSTOMER_INDICATOR_SIZE_)	||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
	{
		memset(szPOS_ID, 0x00, sizeof(szPOS_ID));
		inGetPOS_ID(szPOS_ID);
		memcpy(&pobTran->srBRec.szStoreID[12], szPOS_ID, 6);
	}
        
        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_StoreID END");

	return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_GetAuthCode
Date&Time	:2015/8/11 下午 4:27
Describe	:輸入授權碼
		(二十七) 針對 Field 38 Approve Code 欄位，需區分一般信用卡及銀聯交易檢核如下:
			(1)Sale、Preauth 交易 : EDC 不查核未帶 Approve Code 之 CUP 交易，但原五卡交易仍需查核。
			(2)Refund 交易 : 退貨輸入授權碼之 CUP 交易可 Bypass 輸入，但五卡退貨交易需查核至少輸入 2 碼授權碼。
			(3)Preauth Complete 交易:輸入授權碼之 CUP 交易可 Bypass 輸入，但五卡交易需查核至少輸入 2 碼授權碼。

*/
int inCREDIT_Func_GetAuthCode(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
        char		szTemplate[_DISP_MSG_SIZE_ + 1];
	char            szCustomerIndicator[3 + 1] = {0};
	unsigned char	uszNotValidRetry = VS_FALSE;
        DISPLAY_OBJECT  srDispObj;
        
        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_GetAuthCode START!");
        
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);

	/* ECR交易檢核 */
	if (pobTran->uszECRBit == VS_TRUE)
	{
		if (inFunc_Check_AuthCode_Validate(pobTran->srBRec.szAuthCode) != VS_SUCCESS)
		{
			uszNotValidRetry = VS_TRUE;
			
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_AUTHCODE_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 0;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 3;
			srDispMsgObj.inBeepInterval = 1000;
			
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		
		if (uszNotValidRetry != VS_TRUE)
		{
			/* 分銀聯和非銀聯 */
			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				/* 客製化002 ECR發動退貨，由選單點選進入銀聯退貨，無法直接發動銀聯退貨，故授權碼需開放重新輸入，否則會使用空值 */
				/* 不分客製化，銀聯輸入空白授權碼，仍要重新輸入 by Russell 2022/9/1 下午 6:11 */
				if (strlen(pobTran->srBRec.szAuthCode) < 2			||
				    !memcmp(pobTran->srBRec.szAuthCode, "      ", 6))
				{
					memset(pobTran->srBRec.szAuthCode, 0x00, sizeof(pobTran->srBRec.szAuthCode));
				}
				else
					return (VS_SUCCESS);
			}
			else
			{
				/* 小於2碼或全0或全空白不給過 */
				if (strlen(pobTran->srBRec.szAuthCode) < 2			|| 
				    memcmp(pobTran->srBRec.szAuthCode, "000000", 6) == 0	|| 
				    memcmp(pobTran->srBRec.szAuthCode, "      ", 6) == 0)
				{

				}
				else
				{
					return (VS_SUCCESS);
				}
			}
		}
		else
		{
			/* 含不合法字元，重新輸入 */
		}
	}
	else if (memcmp(gszTermVersionID, "MD731UAGAS001", strlen("MD731UAGAS001")) == 0)
	{
		if (strlen(pobTran->srBRec.szAuthCode) > 0)
		{
			return (VS_SUCCESS);
		}
	}
	else
	{
		if (strlen(pobTran->srBRec.szAuthCode) > 0)
		{
			return (VS_SUCCESS);
		}
	}
	
        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
        memset(szTemplate, 0x00, sizeof(szTemplate));

        srDispObj.inY = _LINE_8_7_;
        srDispObj.inR_L = _DISP_RIGHT_;
        srDispObj.inMaxLen = 6;
	srDispObj.inColor = _COLOR_RED_;
	srDispObj.inTouchSensorFunc = _Touch_OX_LINE8_8_;
	/* 授權碼補登，顯示字眼為「請輸入銀行授權碼」，Time Out時間為5分鐘 */
	if (pobTran->srBRec.uszReferralBit == VS_TRUE)
	{
		srDispObj.inTimeout = 300;
	}
        strcpy(srDispObj.szPromptMsg, "APP.CODE= ");

	while (1)
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		/* 授權碼補登交易、交易補登及紅利分期調帳 */
		if (pobTran->inTransactionResult == _TRAN_RESULT_REFERRAL_	||
		    pobTran->inTransactionCode == _SALE_OFFLINE_		||
		    pobTran->inTransactionCode == _REDEEM_ADJUST_		||
		    pobTran->inTransactionCode == _INST_ADJUST_)
		{
			/* 輸入銀行授權碼 */
			inDISP_PutGraphic(_GET_BANK_AUTHCODE_, 0, _COORDINATE_Y_LINE_8_4_);
		}
		else
		{
			/* 輸入授權碼 */
			inDISP_PutGraphic(_GET_AUTHCODE_, 0, _COORDINATE_Y_LINE_8_4_);
		}
		
		strcpy(szTemplate, srDispObj.szPromptMsg);
		inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, _COLOR_RED_, _DISP_RIGHT_);
		inDISP_PutGraphic(_MSG_ENTER_OR_CANCEL_, 0, _COORDINATE_Y_LINE_8_8_);
		
		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;
	
		inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (inRetVal);
		
		/* 確認是否授權碼是合法字元 */
		/* 輸入授權碼畫面，若輸入符號及小寫英文字母，應於端末機檢核擋下 */
		uszNotValidRetry = VS_FALSE;
		if (inFunc_Check_AuthCode_Validate(srDispObj.szOutput) != VS_SUCCESS)
		{
			uszNotValidRetry = VS_TRUE;
			
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_AUTHCODE_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 0;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 3;
			srDispMsgObj.inBeepInterval = 1000;
			
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		
		/* 輸入不合法，重新輸入 */
		if (uszNotValidRetry == VS_TRUE)
		{
			continue;
		}
		
		/* 【需求單-112268】1.避免特店因退貨交易未輸入授權碼導致剔退 僅退貨不檢核 by Russell 2024/3/21 下午 5:10 */
		if (pobTran->srBRec.inCode == _REFUND_)
		{
			break;
		}
		else
		{
			/* 非CUP交易別 */
			if (pobTran->srBRec.inCode != _CUP_REFUND_	&&
			    pobTran->srBRec.inCode != _CUP_PRE_COMP_)
			{
				/* 不是CUP而且授權碼輸入少於2碼或6個0，不給過 */
				if (srDispObj.inOutputLen < 2 || memcmp(srDispObj.szOutput, "000000", 6) == 0)
				{
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_ChineseFont("授權碼檢查錯誤", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);
					inDISP_BEEP(2, 500);

					continue;
				}
				else
				{
					break;
				}
			}
			/* CUP卡不檢核 */
			else
			{
				break;
			}
		}
	
	}

	/* 【需求單-112268】1.避免特店因退貨交易未輸入授權碼導致剔退 僅退貨不檢核 by Russell 2024/3/21 下午 5:10 */
	if (pobTran->srBRec.inCode == _REFUND_)
	{
		if (srDispObj.inOutputLen == 0)
		{
			memset(pobTran->srBRec.szAuthCode, 0x00, sizeof(pobTran->srBRec.szAuthCode));
			/* 補空白 */
			strcpy(pobTran->srBRec.szAuthCode, "      ");
		}
		else
		{
			memcpy(&pobTran->srBRec.szAuthCode[0], &srDispObj.szOutput[0], srDispObj.inOutputLen);
		}
	}
	else
	{
		/* CUP沒輸入授權碼補空白 */
		if (pobTran->srBRec.uszCUPTransBit == VS_TRUE && srDispObj.inOutputLen == 0)
		{
			memset(pobTran->srBRec.szAuthCode, 0x00, sizeof(pobTran->srBRec.szAuthCode));
			/* 補空白 */
			strcpy(pobTran->srBRec.szAuthCode, "      ");
		}
		else
		{
			memcpy(&pobTran->srBRec.szAuthCode[0], &srDispObj.szOutput[0], srDispObj.inOutputLen);
		}
	}

	return (VS_SUCCESS);
}


/*
Function	:inCREDIT_Func_Get_OPT_AuthCode
Date&Time	:2017/6/27 下午 4:10
Describe	:輸入授權碼
		(二十七) 針對 Field 38 Approve Code 欄位，需區分一般信用卡及銀聯交易檢核如下:
			(1)Sale、Preauth 交易 : EDC 不查核未帶 Approve Code 之 CUP 交易，但原五卡交易仍需查核。
			(2)Refund 交易 : 退貨輸入授權碼之 CUP 交易可 Bypass 輸入，但五卡退貨交易需查核至少輸入 2 碼授權碼。
			(3)Preauth Complete 交易:輸入授權碼之 CUP 交易可 Bypass 輸入，但五卡交易需查核至少輸入 2 碼授權碼。

*/
int inCREDIT_Func_Get_OPT_AuthCode(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
        char		szTemplate[_DISP_MSG_SIZE_ + 1];
	char		szCustomerIndicator[3 + 1] = {0};
	unsigned char	uszNotValidRetry = VS_FALSE;
        DISPLAY_OBJECT  srDispObj;
        
        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_AuthCode START!");

	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	/* ECR交易檢核 */
	if (pobTran->uszECRBit == VS_TRUE)
	{
		/* 兩段式收銀機連線 第一段不輸入 這邊只擋會出現在OPT的 */
		if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
		{
                        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_AuthCode InquiryFirst=True END!");
			return (VS_SUCCESS);
		}
                
		if (inFunc_Check_AuthCode_Validate(pobTran->srBRec.szAuthCode) != VS_SUCCESS)
		{
			uszNotValidRetry = VS_TRUE;
			
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_AUTHCODE_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 0;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 3;
			srDispMsgObj.inBeepInterval = 1000;
			
			inDISP_Msg_BMP(&srDispMsgObj);
		}

		if (uszNotValidRetry != VS_TRUE)
		{
			/* 分銀聯和非銀聯 */
			/* 客製化002銀聯退貨即使沒輸入，也要跳出讓他輸入 */
			/* 不分客製化，銀聯輸入空白授權碼，仍要重新輸入 by Russell 2022/9/1 下午 6:11 */
			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
                                vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_AuthCode CUP_Pass END!");
				return (VS_SUCCESS);
			}
			else
			{
				/* 小於2碼或全0或全空白不給過 */
				if (strlen(pobTran->srBRec.szAuthCode) < 2			|| 
				    memcmp(pobTran->srBRec.szAuthCode, "000000", 6) == 0	|| 
				    memcmp(pobTran->srBRec.szAuthCode, "      ", 6) == 0)
				{

				}
				else
				{
                                        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_AuthCode ECR_AuthCode_Valid END!");
					return (VS_SUCCESS);
				}
			}
		}
		else
		{
			/* 含不合法字元，重新輸入 */
		}
	}
	
        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
        memset(szTemplate, 0x00, sizeof(szTemplate));

        srDispObj.inY = _LINE_8_7_;
        srDispObj.inR_L = _DISP_RIGHT_;
        srDispObj.inMaxLen = 6;
	srDispObj.inColor = _COLOR_RED_;
	srDispObj.inTouchSensorFunc = _Touch_OX_LINE8_8_;
	/* 授權碼補登，顯示字眼為「請輸入銀行授權碼」，Time Out時間為5分鐘 */
	if (pobTran->srBRec.uszReferralBit == VS_TRUE)
	{
		srDispObj.inTimeout = 300;
	}
        strcpy(srDispObj.szPromptMsg, "APP.CODE= ");

	while (1)
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		
		/* 授權碼補登交易、交易補登及紅利分期調帳 */
		if (pobTran->inTransactionResult == _TRAN_RESULT_REFERRAL_	||
		    pobTran->inTransactionCode == _SALE_OFFLINE_		||
		    pobTran->inTransactionCode == _REDEEM_ADJUST_		||
		    pobTran->inTransactionCode == _INST_ADJUST_)
		{
			/* 輸入銀行授權碼 */
			inDISP_PutGraphic(_GET_BANK_AUTHCODE_, 0, _COORDINATE_Y_LINE_8_4_);
		}
		else
		{
			/* 輸入授權碼 */
			inDISP_PutGraphic(_GET_AUTHCODE_, 0, _COORDINATE_Y_LINE_8_4_);
		}

		strcpy(szTemplate, srDispObj.szPromptMsg);
		inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, _COLOR_RED_, _DISP_RIGHT_);
		inDISP_PutGraphic(_MSG_ENTER_OR_CANCEL_, 0, _COORDINATE_Y_LINE_8_8_);
		
		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;

		inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
                {
                        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_AuthCode Timeout_Or_UserCancel(%d) END!", inRetVal);
			return (inRetVal);
                }
		
		/* 確認是否授權碼是合法字元 */
		/* 輸入授權碼畫面，若輸入符號及小寫英文字母，應於端末機檢核擋下 */
		uszNotValidRetry = VS_FALSE;
		if (inFunc_Check_AuthCode_Validate(srDispObj.szOutput) != VS_SUCCESS)
		{
			uszNotValidRetry = VS_TRUE;
			
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_AUTHCODE_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 0;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 3;
			srDispMsgObj.inBeepInterval = 1000;
			
			inDISP_Msg_BMP(&srDispMsgObj);
		}

		/* 輸入不合法，重新輸入 */
		if (uszNotValidRetry == VS_TRUE)
		{
			continue;
		}
		
		/* 【需求單-112268】1.避免特店因退貨交易未輸入授權碼導致剔退 僅退貨不檢核 by Russell 2024/3/21 下午 5:10 */
		if (pobTran->srBRec.inCode == _REFUND_)
		{
			break;
		}
		else
		{
			/* 非CUP交易別 */
			if (pobTran->srBRec.inCode != _CUP_REFUND_	&&
			    pobTran->srBRec.inCode != _CUP_PRE_COMP_)
			{
				/* 不是CUP而且授權碼輸入少於2碼或6個0，不給過 */
				if (srDispObj.inOutputLen < 2 || memcmp(srDispObj.szOutput, "000000", 6) == 0)
				{
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_ChineseFont("授權碼檢查錯誤", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);
					inDISP_BEEP(2, 500);
					continue;
				}
				else
				{
					break;
				}
			}
			/* CUP卡不檢核 */
			else
			{
				break;
			}
		}
	}

	/* 【需求單-112268】1.避免特店因退貨交易未輸入授權碼導致剔退 僅退貨不檢核 by Russell 2024/3/21 下午 5:10 */
	if (pobTran->srBRec.inCode == _REFUND_)
	{
		if (srDispObj.inOutputLen == 0)
		{
			memset(pobTran->srBRec.szAuthCode, 0x00, sizeof(pobTran->srBRec.szAuthCode));
			/* 補空白 */
			strcpy(pobTran->srBRec.szAuthCode, "      ");
		}
		else
		{
			memcpy(&pobTran->srBRec.szAuthCode[0], &srDispObj.szOutput[0], srDispObj.inOutputLen);
		}
	}
	else
	{
		/* CUP沒輸入授權碼補空白 */
		if (pobTran->srBRec.uszCUPTransBit == VS_TRUE && srDispObj.inOutputLen == 0)
		{
			memset(pobTran->srBRec.szAuthCode, 0x00, sizeof(pobTran->srBRec.szAuthCode));
			/* 補空白 */
			strcpy(pobTran->srBRec.szAuthCode, "      ");
		}
		else
		{
			memcpy(&pobTran->srBRec.szAuthCode[0], &srDispObj.szOutput[0], srDispObj.inOutputLen);
		}
	}
        
        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_AuthCode END!");
	
	return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_GetTipAmount
Date&Time	:2015/9/16 上午 11:19
Describe	:螢幕顯示原金額，輸入小費金額
*/
int inCREDIT_Func_GetTipAmount(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal, i;
        long		lnTipAmount, lnTipPercent, lnTiplimit;
        char		szTemplate[_DISP_MSG_SIZE_ + 1];
        char		szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];
        char		szKey;
        DISPLAY_OBJECT  srDispObj;

        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_GetTipAmount START!");
        
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inCREDIT_Func_GetTipAmount START!!");
        }
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
	}

        /* 取得HDT 小費百分比Tip percent 等於零的話提示錯誤畫面 */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetTipPercent(szTemplate);
        lnTipPercent = atol(szTemplate);

        if (lnTipPercent == 0L)
        {
		/* 小費百分比錯誤 */
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, _ERR_TIP_RATE_);
		srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
		srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
		srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
		strcpy(srDispMsgObj.szErrMsg1, "");
		srDispMsgObj.inErrMsg1Line = 0;
		srDispMsgObj.inBeepTimes = 1;
		srDispMsgObj.inBeepInterval = 0;
                inDISP_Msg_BMP(&srDispMsgObj);
		
		pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
                return (VS_ERROR);
        }

        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "Tip Percent ： %ld", lnTipPercent);
                inLogPrintf(AT, szErrorMsg);
        }
        
	/* ----------------------ECR小費是否超過判斷START-----------------------------------------------------*/
	if (pobTran->uszECRBit == VS_TRUE)
	{
		/* 還原被蓋掉的ECR小費 */
		pobTran->srBRec.lnTipTxnAmount = pobTran->lnOldTaxAmount;
		
		/* 如果原金額過大（9個9），計算limit會溢位，所以加判斷 */
                lnTiplimit = (long)(((long long)pobTran->srBRec.lnOrgTxnAmount * lnTipPercent) / 100L);

		if (ginDebug == VS_TRUE)
		{
			memset(szErrorMsg, 0x00, sizeof (szErrorMsg));
			sprintf(szErrorMsg, "Tip: %ld, TipLimit: %ld", pobTran->srBRec.lnTipTxnAmount, lnTiplimit);
			inLogPrintf(AT, szErrorMsg);
		}
	
		/* 若ECR發動直接跳走，且小費不為0 */
		if ((pobTran->srBRec.lnTipTxnAmount != 0) && (pobTran->srBRec.lnTipTxnAmount < lnTiplimit))
		{
			
		}
		else if (pobTran->srBRec.lnTipTxnAmount >= lnTiplimit)
		{
			/* 顯示小費過多 */
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_BIG_TIP_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
			pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
			
			return (VS_ERROR);
		}
		
	}
	else
	{
		if (pobTran->srBRec.lnTipTxnAmount > 0)
		{
			
		}
	}
	/* ----------------------ECR小費是否超過判斷END-----------------------------------------------------*/
	
	/* 如果沒輸入小費要輸入，ECR有給金額還是要出確認畫面 */
	if (pobTran->srBRec.lnTipTxnAmount == 0)
	{
		for (i = 0;; i++)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			/* 輸入小費 */
			inDISP_PutGraphic(_GET_TIP_, 0, _COORDINATE_Y_LINE_8_4_);

			/* 第二行原始金額 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%ld",  pobTran->srBRec.lnOrgTxnAmount);
			inFunc_Amount_Comma(szTemplate, "NT$ " , ' ', _SIGNED_NONE_, 16, VS_TRUE);
			inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_LEFT_);

			/* 第四行鍵盤輸入小費金額 */
			memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
			memset(szTemplate, 0x00, sizeof(szTemplate));

			srDispObj.inY = _LINE_8_7_;
			srDispObj.inR_L = _DISP_RIGHT_;
			srDispObj.inMaxLen = inFunc_Check_Digit();
			srDispObj.inCanNotBypass = VS_TRUE;
			srDispObj.inCanNotZero = VS_TRUE;
			srDispObj.inColor = _COLOR_RED_;
			strcpy(srDispObj.szPromptMsg, "NT$ ");

			/* 進入畫面時先顯示金額為0 */
			strcpy(szTemplate, "NT$ 0");
			inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, srDispObj.inColor, _DISP_RIGHT_);

			memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
			srDispObj.inOutputLen = 0;

			inRetVal = inDISP_Enter8x16_GetAmount(&srDispObj);

			if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
				return (inRetVal);

			lnTipAmount = atol(srDispObj.szOutput);

			/* 如果原金額過大（9個9），計算limit會溢位，所以加判斷 */
                        lnTiplimit = (long)(((long long)pobTran->srBRec.lnOrgTxnAmount * lnTipPercent) / 100L);

			if (lnTipAmount > lnTiplimit)
			{
				/* 顯示小費過多 */
				DISPLAY_OBJECT	srDispMsgObj;
				memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
				strcpy(srDispMsgObj.szDispPic1Name, _ERR_BIG_TIP_);
				srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
				srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
				srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
				strcpy(srDispMsgObj.szErrMsg1, "");
				srDispMsgObj.inErrMsg1Line = 0;
				srDispMsgObj.inBeepTimes = 1;
				srDispMsgObj.inBeepInterval = 0;
				inDISP_Msg_BMP(&srDispMsgObj);

				continue;
			}
			else
			{
				/* 儲存小費金額 */
				pobTran->srBRec.lnTipTxnAmount = lnTipAmount;
				break;
			}
		}
	}

        /* 總金額 */
        pobTran->srBRec.lnTotalTxnAmount = pobTran->srBRec.lnOrgTxnAmount + pobTran->srBRec.lnTipTxnAmount;
        
        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        /* 確認小費新金額畫面 */
        inDISP_PutGraphic(_CHECK_TIP_, 0, _COORDINATE_Y_LINE_8_4_);
        /* 第三行總金額 */
        memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTotalTxnAmount);
	inFunc_Amount_Comma(szTemplate, "NT$ " , ' ', _SIGNED_NONE_, 16, VS_TRUE);
        inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_LEFT_);
        inDISP_BEEP(1, 0);

        while (1)
        {
                szKey = uszKBD_GetKey(180);
                if (szKey == _KEY_0_)
                        break;
                else if (szKey == _KEY_CANCEL_)
			return (VS_USER_CANCEL);
		else if (szKey == _KEY_TIMEOUT_) 
                        return (VS_TIMEOUT);
                else
                        continue;
        }

        pobTran->srBRec.inCode = _TIP_;
	pobTran->inTransactionCode = _TIP_;
	pobTran->uszUpdateBatchBit = VS_TRUE;
	
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inCREDIT_Func_GetTipAmount END!!");
        }

        return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_GetAdjustAmount
Date&Time	:2015/9/16 上午 11:19
Describe	:螢幕顯示原金額，輸入調帳金額
 *		調帳和小費邏輯不同，
 *		小費lnTotalTxnAmount = lnOrgTxnAmount + lnTipTxnAmount = lnTxnAmount + lnTipTxnAmount
 *		調帳lnTotalTxnAmount = lnAdjustTxnAmount調帳後金額基本上和原金額無關
*/
int inCREDIT_Func_GetAdjustAmount(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal, i;
        long		lnAdjustAmount;
        char		szTemplate[_DISP_MSG_SIZE_ + 1];
        char		szCustomerIndicator[3 + 1] = {0};
        unsigned char   uszKey;
        DISPLAY_OBJECT  srDispObj;

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inCREDIT_Func_GetAdjustAmount START!!");
        }
        
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	/* 若ECR發動直接跳走 */
        if (pobTran->uszECRBit == VS_TRUE)
	{
                if (pobTran->srBRec.lnAdjustTxnAmount > 0)
		{
			return (VS_SUCCESS);
		}
		else
		{
			
		}
	}
	else
	{
		if (pobTran->srBRec.lnAdjustTxnAmount > 0)
		{
			return (VS_SUCCESS);
		}
	}
	

        for (i = 0 ;; i++)
        {
                inDISP_ClearAll();
                /* 第一行中文提示 */
                inDISP_ChineseFont("原交易金額：", _FONTSIZE_8X16_, _LINE_8_1_, _DISP_LEFT_);
                /* 第二行原始金額 */
                memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                srDispObj.inY = _LINE_8_2_;
                srDispObj.inR_L = _DISP_RIGHT_;
                srDispObj.inMaxLen = inFunc_Check_Digit();
                sprintf(szTemplate, "NT$ %ld",  pobTran->srBRec.lnOrgTxnAmount);
                inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 16, _PADDING_LEFT_);
                inDISP_EnglishFont(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, _DISP_LEFT_);
                /* 第三行中文提示 */
                inDISP_ChineseFont("請輸入調帳金額：", _FONTSIZE_8X16_, _LINE_8_3_, _DISP_LEFT_);
                /* 第四行鍵盤輸入金額 */
                memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
                memset(szTemplate, 0x00, sizeof(szTemplate));

                srDispObj.inY = _LINE_8_4_;
                srDispObj.inR_L = _DISP_RIGHT_;
                srDispObj.inMaxLen = inFunc_Check_Digit();
		srDispObj.inCanNotZero = VS_SUCCESS;
		srDispObj.inColor = _COLOR_RED_;
                strcpy(srDispObj.szPromptMsg, "NT$ ");

                /* 進入畫面時先顯示金額為0 */
                strcpy(szTemplate, "NT$ 0");
                inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 16, _PADDING_LEFT_);
                inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, srDispObj.inColor, _DISP_LEFT_);

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;
	
                inRetVal = inDISP_Enter8x16_GetAmount(&srDispObj);

                if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
                        return (inRetVal);

                lnAdjustAmount = atol(srDispObj.szOutput);

                if (lnAdjustAmount >= pobTran->srBRec.lnOrgTxnAmount)
                {
                        inDISP_ClearAll();
                        /* 調帳金額須小於原金額提示畫面 */
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);
                        inDISP_PutGraphic(_MENU_ADJUST_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);
                               
			/* 調帳金額超過上限 */
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_ADJUST_OVER_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
                        
                        if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_) ||
                            !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_))
                        {
                                srDispMsgObj.inTimeout = _CUSTOMER_105_MCDONALDS_DISPLAY_TIMEOUT_;
                        }
                        else
                        {
                                srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
                        }
                        
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
                        inDISP_Msg_BMP(&srDispMsgObj);
                        continue;
                }
                else
                {
                        /* 儲存調帳金額 */
                        pobTran->srBRec.lnAdjustTxnAmount = lnAdjustAmount;
                        break;
                }
        }

        /* 總金額 */
        pobTran->srBRec.lnTotalTxnAmount = pobTran->srBRec.lnAdjustTxnAmount;
        /* 確認小費新金額畫面 */
        inDISP_ClearAll();
        inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);
	inDISP_PutGraphic(_MENU_ADJUST_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);
        /* 第二行中文提示 */
        inDISP_ChineseFont("調帳後金額", _FONTSIZE_8X16_, _LINE_8_2_, _DISP_LEFT_);
        /* 第三行總金額 */
        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
        memset(szTemplate, 0x00, sizeof(szTemplate));
        srDispObj.inY = _LINE_8_3_;
        srDispObj.inR_L = _DISP_LEFT_;
        srDispObj.inMaxLen = inFunc_Check_Digit();
        sprintf(szTemplate, "NT$ %ld",  pobTran->srBRec.lnTotalTxnAmount);
        inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 16, _PADDING_RIGHT_);
        inDISP_EnglishFont(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, _DISP_LEFT_);
        /* 第四行請按確認鍵 */
        inDISP_ChineseFont("請按確認鍵", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_LEFT_);
        inDISP_BEEP(1, 0);

        while (1)
        {
                uszKey = uszKBD_Key();

                if (uszKey == _KEY_ENTER_)
                        break;
                else if (uszKey == _KEY_CANCEL_)
                        return (VS_USER_CANCEL);
                else
                        continue;
        }

        pobTran->srBRec.inCode = _ADJUST_;
	pobTran->inTransactionCode = _ADJUST_;
        pobTran->uszUpdateBatchBit = VS_TRUE ;

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inCREDIT_Func_GetAdjustAmount END!!");
        }

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_Func_GetPeriod
Date&Time       :2015/12/3 上午 09:46
Describe        :用來輸入分期期數 by bai
*/
int inCREDIT_Func_GetPeriod(TRANSACTION_OBJECT *pobTran)
{
        int             inRetVal;
        char            szTemplate[_DISP_MSG_SIZE_ + 1];
	char		szCustomerIndicator[3 + 1] = {0};
        DISPLAY_OBJECT  srDispObj;
        
        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_GetPeriod START!");
	
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);

	/* 分期期數ECR沒給的話，要再從EDC輸入期數 */
        if (pobTran->uszECRBit == VS_TRUE)
	{
		do
		{
			/* (【需求單-113181】收銀機連線分期退貨操作需求，分期期數、首期金額、每期金額調整為必須於端末設備上輸入 by Russell 2024/10/7 下午 2:50 */
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_027_CHUNGHWA_TELECOM_, _CUSTOMER_INDICATOR_SIZE_))
			{
				if (pobTran->srBRec.inCode == _INST_REFUND_)
				{
					break;
				}
			}
			
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_002_NICE_PLAZA_, _CUSTOMER_INDICATOR_SIZE_))
			{
				return (VS_SUCCESS);
			}
			
			
			if (pobTran->srBRec.lnInstallmentPeriod > 0)
			{
				return (VS_SUCCESS);
			}
			
			break;
		}while(1);
	}
	else
	{
		if (pobTran->srBRec.lnInstallmentPeriod > 0)
		{
			return (VS_SUCCESS);
		}
	}
	
        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
        memset(szTemplate, 0x00, sizeof(szTemplate));

	/* 不可輸入0，不可ByPass */
        srDispObj.inY = _LINE_8_7_;
        srDispObj.inR_L = _DISP_RIGHT_;
        srDispObj.inMaxLen = 2;
	srDispObj.inCanNotBypass = VS_TRUE;
	srDispObj.inCanNotZero = VS_TRUE;
	srDispObj.inColor = _COLOR_RED_;

        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_PutGraphic(_GET_INST_PERIOD_, 0, _COORDINATE_Y_LINE_8_4_);

	memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
	srDispObj.inOutputLen = 0;
	
	inRetVal = inDISP_Enter8x16(&srDispObj);
		
	if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
		return (inRetVal);

        pobTran->srBRec.lnInstallmentPeriod = atol(srDispObj.szOutput);

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_Func_Get_OPT_Period
Date&Time       :2017/9/7 下午 3:09
Describe        :用來輸入分期期數
*/
int inCREDIT_Func_Get_OPT_Period(TRANSACTION_OBJECT *pobTran)
{
        int             inRetVal;
        char            szTemplate[_DISP_MSG_SIZE_ + 1];
        char		szCustomerIndicator[3 + 1] = {0};
        DISPLAY_OBJECT  srDispObj;
        
        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_Period START!");

        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
        
	/* 分期期數ECR沒給的話，要再從EDC輸入期數 */
        if (pobTran->uszECRBit == VS_TRUE)
	{
		do
		{
			/* (【需求單-113181】收銀機連線分期退貨操作需求，分期期數、首期金額、每期金額調整為必須於端末設備上輸入 by Russell 2024/10/7 下午 2:50 */
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_027_CHUNGHWA_TELECOM_, _CUSTOMER_INDICATOR_SIZE_))
			{
				if (pobTran->srBRec.inCode == _INST_REFUND_)
				{
					break;
				}
			}
			
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_002_NICE_PLAZA_, _CUSTOMER_INDICATOR_SIZE_))
			{
                                vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_Period cus-002_Nice_plaza END!");
				return (VS_SUCCESS);
			}
			
			
			if (pobTran->srBRec.lnInstallmentPeriod > 0)
			{
                                vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_Period lnInstallmentPeriod(%02ld) END!", pobTran->srBRec.lnInstallmentPeriod);
				return (VS_SUCCESS);
			}

			/* 兩段式收銀機連線 第一段不輸入 這邊只擋會出現在OPT的 */
			if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
			{
                                vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_Period uszCardInquiryFirstBit=VS_True END!");
				return (VS_SUCCESS);
			}
			
			break;
		}while(1);
	}
	
        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
        memset(szTemplate, 0x00, sizeof(szTemplate));

	/* 不可輸入0，不可ByPass */
        srDispObj.inY = _LINE_8_7_;
        srDispObj.inR_L = _DISP_RIGHT_;
        srDispObj.inMaxLen = 2;
	srDispObj.inCanNotBypass = VS_TRUE;
	srDispObj.inCanNotZero = VS_TRUE;
	srDispObj.inColor = _COLOR_RED_;

        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);   
	inDISP_PutGraphic(_GET_INST_PERIOD_, 0, _COORDINATE_Y_LINE_8_4_);

	memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
	srDispObj.inOutputLen = 0;
	
	inRetVal = inDISP_Enter8x16(&srDispObj);
		
	if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
        {
                vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_Period Timeout_Or_UserCancel(%d) END!", inRetVal);
		return (inRetVal);
        }

        pobTran->srBRec.lnInstallmentPeriod = atol(srDispObj.szOutput);
        
        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_Period END!");

        return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_GetDownPayment
Date&Time	:2016/6/13 下午 3:33
Describe	:請輸入首期金額
*/
int inCREDIT_Func_GetDownPayment(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
        char		szTemplate[_DISP_MSG_SIZE_ + 1];
	char		szCustomerIndicator[3 + 1] = {0};
        DISPLAY_OBJECT  srDispObj;
        
        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_GetDownPayment START!");

	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	/* 若ECR發動且有值就直接跳走 */
        if (pobTran->uszECRBit == VS_TRUE)
	{
		do
		{
			/* (【需求單-113181】收銀機連線分期退貨操作需求，分期期數、首期金額、每期金額調整為必須於端末設備上輸入 by Russell 2024/10/7 下午 2:50 */
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_027_CHUNGHWA_TELECOM_, _CUSTOMER_INDICATOR_SIZE_))
			{
				if (pobTran->srBRec.inCode == _INST_REFUND_)
				{
					break;
				}
			}
		
			/* 收銀機資訊不完整時要先過卡再輸入，也就是到TRT再執行檢核 */
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_002_NICE_PLAZA_, _CUSTOMER_INDICATOR_SIZE_))
			{
				return (VS_SUCCESS);
			}
		
			/* 若ECR有拋值才跳走 */
			/* 因為可以輸入0，所以要用空白判斷是否要輸入 */
			if (memcmp(gsrECROb.srTransData.szField_26, " ", 1) == 0	||
			    memcmp(gsrECROb.srTransData.szField_26, "\x00", 1) == 0)
			{
				if (pobTran->uszEnterDownPaymentBit == VS_TRUE)
				{
					return (VS_SUCCESS);
				}
			}
			else
			{
				return (VS_SUCCESS);
			}
			
			break;
		}while(1);
	}
	else
	{
		/* OPT已經先輸入過了 */
		if (pobTran->inRunOperationID == _OPERATION_INST_REFUND_CTLS_	||
		    pobTran->inRunOperationID == _OPERATION_INST_ADJUST_CTLS_)
		{
			return (VS_SUCCESS);
		}
		else
		{
			
		}
	}
	
        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
        memset(szTemplate, 0x00, sizeof(szTemplate));

	/* 可輸入0，可ByPass */
        srDispObj.inY = _LINE_8_7_;
        srDispObj.inR_L = _DISP_RIGHT_;
        srDispObj.inMaxLen = inFunc_Check_Digit();
	srDispObj.inColor = _COLOR_RED_;
        strcpy(srDispObj.szPromptMsg, "NT$ ");

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* 輸入首期金額 */
	inDISP_PutGraphic(_GET_INST_DOWN_PAYMENT_, 0, _COORDINATE_Y_LINE_8_4_);

        /* 進入畫面時先顯示金額為0 */
        strcpy(szTemplate, "NT$ 0");
        inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, srDispObj.inColor, _DISP_RIGHT_);

	memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
	srDispObj.inOutputLen = 0;
	
        inRetVal = inDISP_Enter8x16_GetAmount(&srDispObj);

        if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
		return (inRetVal);

        pobTran->srBRec.lnInstallmentDownPayment = atol(srDispObj.szOutput);
         
	return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_Get_OPT_DownPayment
Date&Time	:2017/9/7 下午 3:58
Describe	:請輸入首期金額
*/
int inCREDIT_Func_Get_OPT_DownPayment(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
        char		szTemplate[_DISP_MSG_SIZE_ + 1];
        char		szCustomerIndicator[3 + 1] = {0};
        DISPLAY_OBJECT  srDispObj;
        
        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_DownPayment START!");

        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
        
	/* 若ECR發動且有值就直接跳走 */
        if (pobTran->uszECRBit == VS_TRUE)
	{
		do
		{
			/* (【需求單-113181】收銀機連線分期退貨操作需求，分期期數、首期金額、每期金額調整為必須於端末設備上輸入 by Russell 2024/10/7 下午 2:50 */
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_027_CHUNGHWA_TELECOM_, _CUSTOMER_INDICATOR_SIZE_))
			{
				if (pobTran->srBRec.inCode == _INST_REFUND_)
				{
					break;
				}
			}
		
			/* 收銀機資訊不完整時要先過卡再輸入，也就是到TRT再執行檢核 */
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_002_NICE_PLAZA_, _CUSTOMER_INDICATOR_SIZE_))
			{
                                vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_DownPayment Cus_002_Nice_Plaza END!");
				return (VS_SUCCESS);
			}
		
			/* 若ECR有拋值才跳走 */
			/* 因為可以輸入0，所以要用空白判斷是否要輸入 */
			if (memcmp(gsrECROb.srTransData.szField_26, "            ", 12) == 0)
			{
				/* 兩段式收銀機連線 第一段不輸入 這邊只擋會出現在OPT的 */
				if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
				{
                                        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_DownPayment InquiryFirst END!");
					return (VS_SUCCESS);
				}
			}
			else
			{
                                vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_DownPayment szField_26=space END!");
				return (VS_SUCCESS);
			}
			
			break;
		}while(1);
		
	}
	
        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
        memset(szTemplate, 0x00, sizeof(szTemplate));

	/* 可輸入0，可ByPass */
        srDispObj.inY = _LINE_8_7_;
        srDispObj.inR_L = _DISP_RIGHT_;
        srDispObj.inMaxLen = inFunc_Check_Digit();
	srDispObj.inColor = _COLOR_RED_;
        strcpy(srDispObj.szPromptMsg, "NT$ ");

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* 輸入首期金額 */
	inDISP_PutGraphic(_GET_INST_DOWN_PAYMENT_, 0, _COORDINATE_Y_LINE_8_4_);

        /* 進入畫面時先顯示金額為0 */
        strcpy(szTemplate, "NT$ 0");
        inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, srDispObj.inColor, _DISP_RIGHT_);

	memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
	srDispObj.inOutputLen = 0;
	
        inRetVal = inDISP_Enter8x16_GetAmount(&srDispObj);

        if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
        {
                vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_DownPayment Timeout_Or_UserCancel(%d) END!", inRetVal);
		return (inRetVal);
        }

        pobTran->srBRec.lnInstallmentDownPayment = atol(srDispObj.szOutput);
	pobTran->uszEnterDownPaymentBit = VS_TRUE;
	
        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_DownPayment END!");
        
	return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_GetInstPayment
Date&Time	:2016/6/13 下午 3:48
Describe	:請輸入每期金額
*/
int inCREDIT_Func_GetInstPayment(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
        char		szTemplate[_DISP_MSG_SIZE_ + 1];
	char		szCustomerIndicator[3 + 1] = {0};
        DISPLAY_OBJECT  srDispObj;
        
        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_GetInstPayment START!");

	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	/* 若ECR發動且有值就直接跳走 */
        if (pobTran->uszECRBit == VS_TRUE)
	{
		do
		{
			/* (【需求單-113181】收銀機連線分期退貨操作需求，分期期數、首期金額、每期金額調整為必須於端末設備上輸入 by Russell 2024/10/7 下午 2:50 */
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_027_CHUNGHWA_TELECOM_, _CUSTOMER_INDICATOR_SIZE_))
			{
				if (pobTran->srBRec.inCode == _INST_REFUND_)
				{
					break;
				}
			}
			
			/* 收銀機資訊不完整時要先過卡再輸入，也就是到TRT再執行檢核 */
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_002_NICE_PLAZA_, _CUSTOMER_INDICATOR_SIZE_))
			{
				return (VS_SUCCESS);
			}
			
			if (pobTran->srBRec.lnInstallmentPayment > 0)
			{
				return (VS_SUCCESS);
			}
			
			break;
		}while(1);
	}
	else
	{
		if (pobTran->srBRec.lnInstallmentPayment > 0)
		{
			return (VS_SUCCESS);
		}
	}
	
        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
        memset(szTemplate, 0x00, sizeof(szTemplate));

	/* 不可輸入0，不可ByPass */
        srDispObj.inY = _LINE_8_7_;
        srDispObj.inR_L = _DISP_RIGHT_;
        srDispObj.inMaxLen = inFunc_Check_Digit();
	srDispObj.inCanNotBypass = VS_TRUE;
	srDispObj.inCanNotZero = VS_TRUE;
	srDispObj.inColor = _COLOR_RED_;
	
        strcpy(srDispObj.szPromptMsg, "NT$ ");

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* 請輸入每期金額 */
	inDISP_PutGraphic(_GET_INST_PAYMENT_, 0, _COORDINATE_Y_LINE_8_4_);

        /* 進入畫面時先顯示金額為0 */
        strcpy(szTemplate, "NT$ 0");
        inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, srDispObj.inColor, _DISP_RIGHT_);

	memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
	srDispObj.inOutputLen = 0;
	
        inRetVal = inDISP_Enter8x16_GetAmount(&srDispObj);

        if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
		return (inRetVal);

        pobTran->srBRec.lnInstallmentPayment = atol(srDispObj.szOutput);
         
	return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_Get_OPT_InstPayment
Date&Time	:2017/9/7 下午 4:04
Describe	:請輸入每期金額
*/
int inCREDIT_Func_Get_OPT_InstPayment(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
        char		szTemplate[_DISP_MSG_SIZE_ + 1];
        char		szCustomerIndicator[3 + 1] = {0};
        DISPLAY_OBJECT  srDispObj;
        
        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_InstPayment START!");
        
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);

	/* 若ECR發動且有值就直接跳走 */
        if (pobTran->uszECRBit == VS_TRUE)
	{
		do
		{
			/* (【需求單-113181】收銀機連線分期退貨操作需求，分期期數、首期金額、每期金額調整為必須於端末設備上輸入 by Russell 2024/10/7 下午 2:50 */
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_027_CHUNGHWA_TELECOM_, _CUSTOMER_INDICATOR_SIZE_))
			{
				if (pobTran->srBRec.inCode == _INST_REFUND_)
				{
					break;
				}
			}
			
			/* 收銀機資訊不完整時要先過卡再輸入，也就是到TRT再執行檢核 */
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_002_NICE_PLAZA_, _CUSTOMER_INDICATOR_SIZE_))
			{
                                vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_InstPayment cus_002_Nice_Plaza END!");
				return (VS_SUCCESS);
			}
			
			if (pobTran->srBRec.lnInstallmentPayment > 0)
			{
                                vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_InstPayment lnInstallmentPayment>0 END!");
				return (VS_SUCCESS);
			}

			/* 兩段式收銀機連線 第一段不輸入 這邊只擋會出現在OPT的 */
			if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
			{
                                vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_InstPayment InquiryFirst=True END!");
				return (VS_SUCCESS);
			}
			
			break;
		}while(1);
	}
	
        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
        memset(szTemplate, 0x00, sizeof(szTemplate));

	/* 不可輸入0，不可ByPass */
        srDispObj.inY = _LINE_8_7_;
        srDispObj.inR_L = _DISP_RIGHT_;
        srDispObj.inMaxLen = inFunc_Check_Digit();
	srDispObj.inCanNotBypass = VS_TRUE;
	srDispObj.inCanNotZero = VS_TRUE;
	srDispObj.inColor = _COLOR_RED_;
	
        strcpy(srDispObj.szPromptMsg, "NT$ ");

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* 請輸入每期金額 */
	inDISP_PutGraphic(_GET_INST_PAYMENT_, 0, _COORDINATE_Y_LINE_8_4_);

        /* 進入畫面時先顯示金額為0 */
        strcpy(szTemplate, "NT$ 0");
        inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, srDispObj.inColor, _DISP_RIGHT_);

	memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
	srDispObj.inOutputLen = 0;
	
        inRetVal = inDISP_Enter8x16_GetAmount(&srDispObj);

        if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
        {
                vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_InstPayment Timout_Or_UserCancel(%d) END!", inRetVal);
		return (inRetVal);
        }

        pobTran->srBRec.lnInstallmentPayment = atol(srDispObj.szOutput);
        
        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_InstPayment END!");
         
	return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_GetInstFee
Date&Time	:2016/6/13 下午 3:48
Describe	:請輸入手續費
*/
int inCREDIT_Func_GetInstFee(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
        char		szTemplate[_DISP_MSG_SIZE_ + 1];
        DISPLAY_OBJECT  srDispObj;

	/* 若ECR發動直接跳走 */
        if (pobTran->uszECRBit == VS_TRUE)
        {
		/* 若ECR有拋值才跳走 */
		/* 因為可以輸入0，所以要用空白判斷是否要輸入 */
                if (memcmp(gsrECROb.srTransData.szField_28, "            ", 12) == 0)
		{
			if (pobTran->uszEnterInstFeeBit == VS_TRUE)
			{
				return (VS_SUCCESS);
			}
			else
			{
				
			}
		}
		else
		{
			return (VS_SUCCESS);
		}
	}
	else
	{
		if (pobTran->inRunOperationID == _OPERATION_INST_REFUND_CTLS_	||
		    pobTran->inRunOperationID == _OPERATION_INST_ADJUST_CTLS_)
		{
			return (VS_SUCCESS);
		}
		else
		{
			
		}
	}
	
        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
        memset(szTemplate, 0x00, sizeof(szTemplate));

	/* 可輸入0，也可以ByPass */
        srDispObj.inY = _LINE_8_7_;
        srDispObj.inR_L = _DISP_RIGHT_;
        srDispObj.inMaxLen = inFunc_Check_Digit();
	srDispObj.inColor = _COLOR_RED_;
        strcpy(srDispObj.szPromptMsg, "NT$ ");

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* 請輸入手續費 */
	inDISP_PutGraphic(_GET_INST_FEE_, 0, _COORDINATE_Y_LINE_8_4_);

        /* 進入畫面時先顯示金額為0 */
        strcpy(szTemplate, "NT$ 0");
        inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, srDispObj.inColor, _DISP_RIGHT_);

	memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
	srDispObj.inOutputLen = 0;
	
        inRetVal = inDISP_Enter8x16_GetAmount(&srDispObj);

        if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
		return (inRetVal);

        pobTran->srBRec.lnInstallmentFormalityFee = atol(srDispObj.szOutput);
         
	return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_Get_OPT_InstFee
Date&Time	:2017/9/7 下午 4:04
Describe	:請輸入手續費
*/
int inCREDIT_Func_Get_OPT_InstFee(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
        char		szTemplate[_DISP_MSG_SIZE_ + 1];
        char		szCustomerIndicator[3 + 1] = {0};
        DISPLAY_OBJECT  srDispObj;

        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
        
	/* 若ECR發動直接跳走 */
        if (pobTran->uszECRBit == VS_TRUE)
	{
		/* 若ECR有拋值才跳走 */
		/* 因為可以輸入0，所以要用空白判斷是否要輸入 */
                if (memcmp(gsrECROb.srTransData.szField_28, "            ", 12) == 0)
		{
			/* 兩段式收銀機連線 第一段不輸入 這邊只擋會出現在OPT的 */
			if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
			{
				return (VS_SUCCESS);
			}
			
                        /* 收銀機資訊不完整時要先過卡再輸入，也就是到TRT再執行檢核 */
                        if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_002_NICE_PLAZA_, _CUSTOMER_INDICATOR_SIZE_))
                        {
                                return (VS_SUCCESS);
                        }  
		}
		else
		{
			return (VS_SUCCESS);
		}
	}
	
        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
        memset(szTemplate, 0x00, sizeof(szTemplate));

	/* 可輸入0，也可以ByPass */
        srDispObj.inY = _LINE_8_7_;
        srDispObj.inR_L = _DISP_RIGHT_;
        srDispObj.inMaxLen = inFunc_Check_Digit();
	srDispObj.inColor = _COLOR_RED_;
        strcpy(srDispObj.szPromptMsg, "NT$ ");

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* 請輸入手續費 */
	inDISP_PutGraphic(_GET_INST_FEE_, 0, _COORDINATE_Y_LINE_8_4_);

        /* 進入畫面時先顯示金額為0 */
        strcpy(szTemplate, "NT$ 0");
        inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, srDispObj.inColor, _DISP_RIGHT_);

	memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
	srDispObj.inOutputLen = 0;
	
        inRetVal = inDISP_Enter8x16_GetAmount(&srDispObj);

        if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
		return (inRetVal);

        pobTran->srBRec.lnInstallmentFormalityFee = atol(srDispObj.szOutput);
	pobTran->uszEnterInstFeeBit = VS_TRUE;
	
	return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_GetPayAmount
Date&Time	:2016/6/14 上午 9:40
Describe	:請輸入支付金額
*/
int inCREDIT_Func_GetPayAmount(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
        char		szTemplate[_DISP_MSG_SIZE_ + 1];
        DISPLAY_OBJECT  srDispObj;

        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_GetPayAmount START!");
        
	/* 若ECR發動直接跳走 */
        if (pobTran->uszECRBit == VS_TRUE)
	{
                /* 若ECR有拋值才跳走 */
		/* 空白、0x00、全0判斷是否要輸入 */
                if (memcmp(gsrECROb.srTransData.szRDMPaidAmt, " ", 1) == 0  ||
                    memcmp(gsrECROb.srTransData.szRDMPaidAmt, "\x00", 1) == 0  ||    
                    memcmp(gsrECROb.srTransData.szRDMPaidAmt, "0000000000", 10) == 0)
		{
			if (pobTran->uszEnterRDMPaidAmtBit == VS_TRUE)
			{
				return (VS_SUCCESS);
			}
			else
			{
				
			}
		}
		else
		{
			return (VS_SUCCESS);
		}
	}
	else
	{
		if (pobTran->inRunOperationID == _OPERATION_REDEEM_REFUND_CTLS_	||
		    pobTran->inRunOperationID == _OPERATION_REDEEM_ADJUST_CTLS_)
		{
			return (VS_SUCCESS);
		}
		else
		{
			
		}
	}
	
        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
        memset(szTemplate, 0x00, sizeof(szTemplate));

	/* 可輸入0，也可以ByPass */
        srDispObj.inY = _LINE_8_7_;
        srDispObj.inR_L = _DISP_RIGHT_;
        srDispObj.inMaxLen = inFunc_Check_Digit();
	srDispObj.inColor = _COLOR_RED_;
        strcpy(srDispObj.szPromptMsg, "NT$ ");

	while (1)
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		/* 請輸入支付金額 */
		inDISP_PutGraphic(_GET_REDEEM_PAY_AMOUNT_, 0, _COORDINATE_Y_LINE_8_4_);

		/* 進入畫面時先顯示金額為0 */
		strcpy(szTemplate, "NT$ 0");
		inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, srDispObj.inColor, _DISP_RIGHT_);

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;
	
		inRetVal = inDISP_Enter8x16_GetAmount(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (inRetVal);

		/* 不能等於或大於 */
		if (atol(srDispObj.szOutput) < pobTran->srBRec.lnTxnAmount)
		{
			break;
		}
		
	}

        pobTran->srBRec.lnRedemptionPaidCreditAmount = atol(srDispObj.szOutput);
         
	return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_Get_OPT_PayAmount
Date&Time	:2017/9/7 下午 6:27
Describe	:請輸入支付金額
*/
int inCREDIT_Func_Get_OPT_PayAmount(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
        char		szTemplate[_DISP_MSG_SIZE_ + 1];
        char		szCustomerIndicator[3 + 1] = {0};
        DISPLAY_OBJECT  srDispObj;
        
        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_PayAmount START!");
        
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);

	/* 若ECR發動直接跳走 */
        if (pobTran->uszECRBit == VS_TRUE)
	{
		/* 若ECR有拋值才跳走 */
		/* 空白、0x00、全0判斷是否要輸入 */
                if (memcmp(gsrECROb.srTransData.szRDMPaidAmt, " ", 1) == 0  ||
                    memcmp(gsrECROb.srTransData.szRDMPaidAmt, "\x00", 1) == 0  ||    
                    memcmp(gsrECROb.srTransData.szRDMPaidAmt, "0000000000", 10) == 0)
		{
			/* 兩段式收銀機連線 第一段不輸入 這邊只擋會出現在OPT的 */
			if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
			{
				return (VS_SUCCESS);
			}
			
                        /* 收銀機資訊不完整時要先過卡再輸入，也就是到TRT再執行檢核 */
                        if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_002_NICE_PLAZA_, _CUSTOMER_INDICATOR_SIZE_))
                        {
                                return (VS_SUCCESS);
                        }  
		}
		else
		{
			return (VS_SUCCESS);
		}
	}
	
        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
        memset(szTemplate, 0x00, sizeof(szTemplate));

	/* 可輸入0，也可以ByPass */
        srDispObj.inY = _LINE_8_7_;
        srDispObj.inR_L = _DISP_RIGHT_;
        srDispObj.inMaxLen = inFunc_Check_Digit();
	srDispObj.inColor = _COLOR_RED_;
        strcpy(srDispObj.szPromptMsg, "NT$ ");

	while (1)
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		/* 請輸入支付金額 */
		inDISP_PutGraphic(_GET_REDEEM_PAY_AMOUNT_, 0, _COORDINATE_Y_LINE_8_4_);

		/* 進入畫面時先顯示金額為0 */
		strcpy(szTemplate, "NT$ 0");
		inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, srDispObj.inColor, _DISP_RIGHT_);

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;
	
		inRetVal = inDISP_Enter8x16_GetAmount(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (inRetVal);

		/* 不能等於或大於 */
		if (atol(srDispObj.szOutput) < pobTran->srBRec.lnTxnAmount)
		{
			break;
		}
		
	}

        pobTran->srBRec.lnRedemptionPaidCreditAmount = atol(srDispObj.szOutput);
	pobTran->uszEnterRDMPaidAmtBit = VS_TRUE;
         
	return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_GetRedeemPoint
Date&Time	:2016/6/14 上午 9:41
Describe	:請輸入扣抵紅利點數
*/
int inCREDIT_Func_GetRedeemPoint(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
        char		szTemplate[_DISP_MSG_SIZE_ + 1];
        DISPLAY_OBJECT  srDispObj;

        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_GetRedeemPoint START!");
        
	/* 若ECR發動而且有值就直接跳走 */
        if (pobTran->uszECRBit == VS_TRUE)
	{
                if (pobTran->srBRec.lnRedemptionPoints > 0)
		{
			return (VS_SUCCESS);
		}
		else
		{
			
		}
	}
	else
	{
		if (pobTran->srBRec.lnRedemptionPoints > 0)
		{
			return (VS_SUCCESS);
		}
	}
	
        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
        memset(szTemplate, 0x00, sizeof(szTemplate));

	/* 不可輸入0，不可ByPass */
        srDispObj.inY = _LINE_8_7_;
        srDispObj.inR_L = _DISP_RIGHT_;
        srDispObj.inMaxLen = 8;
	srDispObj.inCanNotBypass = VS_TRUE;
	srDispObj.inCanNotZero = VS_TRUE;
	srDispObj.inColor = _COLOR_RED_;

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* 請輸入扣抵紅利點數 */
	inDISP_PutGraphic(_GET_REDEEM_POINT_, 0, _COORDINATE_Y_LINE_8_4_);

	memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
	srDispObj.inOutputLen = 0;
	
        inRetVal = inDISP_Enter8x16(&srDispObj);

        if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
		return (VS_ERROR);

        pobTran->srBRec.lnRedemptionPoints = atol(srDispObj.szOutput);
         
	return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_Get_OPT_RedeemPoint
Date&Time	:2017/9/7 下午 6:27
Describe	:請輸入扣抵紅利點數
*/
int inCREDIT_Func_Get_OPT_RedeemPoint(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
        char		szTemplate[_DISP_MSG_SIZE_ + 1];
        char		szCustomerIndicator[3 + 1] = {0};
        DISPLAY_OBJECT  srDispObj;
        
        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_RedeemPoint START!");

        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
        
	/* 若ECR發動而且有值就直接跳走 */
        if (pobTran->uszECRBit == VS_TRUE)
	{
		if (pobTran->srBRec.lnRedemptionPoints > 0)
		{
			return (VS_SUCCESS);
		}
                
		/* 兩段式收銀機連線 第一段不輸入 這邊只擋會出現在OPT的 */
		if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
		{
			return (VS_SUCCESS);
		}
                
                /* 收銀機資訊不完整時要先過卡再輸入，也就是到TRT再執行檢核 */
                if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_002_NICE_PLAZA_, _CUSTOMER_INDICATOR_SIZE_))
                {
                        return (VS_SUCCESS);
                }  
	}
	
        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
        memset(szTemplate, 0x00, sizeof(szTemplate));

	/* 不可輸入0，不可ByPass */
        srDispObj.inY = _LINE_8_7_;
        srDispObj.inR_L = _DISP_RIGHT_;
        srDispObj.inMaxLen = 8;
	srDispObj.inCanNotBypass = VS_TRUE;
	srDispObj.inCanNotZero = VS_TRUE;
	srDispObj.inColor = _COLOR_RED_;

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* 請輸入扣抵紅利點數 */
	inDISP_PutGraphic(_GET_REDEEM_POINT_, 0, _COORDINATE_Y_LINE_8_4_);

	memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
	srDispObj.inOutputLen = 0;
	
        inRetVal = inDISP_Enter8x16(&srDispObj);

        if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
		return (VS_ERROR);

        pobTran->srBRec.lnRedemptionPoints = atol(srDispObj.szOutput);
         
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_Func_CheckResult
Date&Time       :2016/4/1 上午 9:53
Describe        :交易確認畫面，for 電子簽名用，避免直接把電子簽名bypass掉
*/
int inCREDIT_Func_CheckResult(TRANSACTION_OBJECT *pobTran)
{
	int inTimeout = 30;
	char szDispBuf[_DISP_MSG_SIZE_ + 1] = {0};
	char szSignPadMode[2 + 1] = {0};
	char szTimeout[3 + 1] = {0};
	unsigned char uszKey = 0x00;
	
	vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_CheckResult START!");

	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
	}
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inCREDIT_Func_CheckResult() START !");
	}
	
	/* 因銀聯優計畫參考優惠後金額作為免簽條件，故免簽條件檢查提前 */
	inFLOW_RunFunction(pobTran, _NCCC_FUNCTION_CHECK_NOSIGNATURE_FINAL_);
        
	memset(szTimeout, 0x00, sizeof(szTimeout));
	inGetEnterTimeout(szTimeout);
	if (atoi(szTimeout) > 0)
	{
		inTimeout = atoi(szTimeout);
	}
	else
	{
		inTimeout = 30;
	}
        
	
	/* 現在只有銀聯優計畫使用 */
	if ((pobTran->uszECRBit == VS_TRUE && gsrECROb.srTransData.szECRIndicator[0] == 'Q') &&
		pobTran->srBRec.uszUPlanECRBit == VS_TRUE)
	{
		/* CFGT的Signpad開關判斷 */
		memset(szSignPadMode , 0x00, sizeof(szSignPadMode));
		inGetSignPadMode(szSignPadMode);

		do
		{
			/* 不開signpad */
			if (!memcmp(szSignPadMode, _SIGNPAD_MODE_0_NO_, 1)	||
				!memcmp(szSignPadMode, _SIGNPAD_MODE_2_EXTERNAL_, 1))
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "signpad not internal, not display");
				}
				break;
			}

			/* 免簽不顯示 */
			if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "NoSignature, not display");
				}
				break;
			}

			inFunc_ResetTitle(pobTran);

			if (pobTran->srBRec.inCode == _CUP_VOID_)
			{
				/* 銀聯取消不顯示 */
			}
			else
			{
				/* 交易完成 */
				inDISP_ChineseFont_Color("交易完成", _FONTSIZE_8X16_, _LINE_8_4_, _COLOR_BLACK_, _DISP_CENTER_);

				/* 授權碼 */
				inDISP_ChineseFont_Color("授權碼=", _FONTSIZE_8X33_, _LINE_8_5_, _COLOR_BLACK_, _DISP_LEFT_);
				memset(szDispBuf, 0x00, sizeof(szDispBuf));
				sprintf(szDispBuf, "%s", pobTran->srBRec.szAuthCode);
				inDISP_ChineseFont_Color(szDispBuf, _FONTSIZE_8X33_, _LINE_8_5_, _COLOR_BLACK_, _DISP_CENTER_);

				/* 請在簽名版簽名 */
				inDISP_ChineseFont_Color("請在簽名版簽名", _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_BLACK_, _DISP_LEFT_);

				/* 確認本人指定以銀聯卡支付 */
				inDISP_ChineseFont_Color("本人指定以銀聯卡支付", _FONTSIZE_8X33_, _LINE_8_7_, _COLOR_BLACK_, _DISP_CENTER_);

				/* I chose CUP card to  pay */
				inDISP_ChineseFont_Color("I chose CUP card to  pay", _FONTSIZE_8X33_, _LINE_8_8_, _COLOR_BLACK_, _DISP_CENTER_);

				while (1)
				{
					uszKey = uszKBD_GetKey(inTimeout);

					if (uszKey == _KEY_ENTER_)
					{
						break;
					}
					/* 不接受清除鍵，timeout 預設為確認 */
					else if (uszKey == _KEY_TIMEOUT_)
					{
						break;
					}
				}
			}
			break;
		}while (1);
	}
	else if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
	{
		do
		{
			inFunc_ResetTitle(pobTran);
			/* 交易完成 */
			inDISP_ChineseFont_Color("交易完成", _FONTSIZE_8X16_, _LINE_8_4_, _COLOR_BLACK_, _DISP_CENTER_);

			/* 授權碼 */
			inDISP_ChineseFont_Color("授權碼=", _FONTSIZE_8X33_, _LINE_8_5_, _COLOR_BLACK_, _DISP_LEFT_);
			memset(szDispBuf, 0x00, sizeof(szDispBuf));
			sprintf(szDispBuf, "%s", pobTran->srBRec.szAuthCode);
			inDISP_ChineseFont_Color(szDispBuf, _FONTSIZE_8X33_, _LINE_8_5_, _COLOR_BLACK_, _DISP_RIGHT_);
			/* NT */
			inDISP_ChineseFont_Color("NT", _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_BLACK_, _DISP_LEFT_);
			memset(szDispBuf, 0x00, sizeof(szDispBuf));
			sprintf(szDispBuf, "%ld", pobTran->srBRec.lnTxnAmount);
			inFunc_Amount_Comma(szDispBuf, "$", '\x00', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
			inDISP_ChineseFont_Color(szDispBuf, _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_BLACK_, _DISP_RIGHT_);

			/* 確認本人指定以銀聯卡支付 */
			inDISP_ChineseFont_Color("本人指定以銀聯卡支付", _FONTSIZE_8X33_, _LINE_8_7_, _COLOR_BLACK_, _DISP_CENTER_);

			/* I chose CUP card to  pay */
			inDISP_ChineseFont_Color("I chose CUP card to  pay", _FONTSIZE_8X33_, _LINE_8_8_, _COLOR_BLACK_, _DISP_CENTER_);

			while (1)
			{
				uszKey = uszKBD_GetKey(inTimeout);

				if (uszKey == _KEY_ENTER_)
				{
					break;
				}
				/* 不接受清除鍵，timeout 預設為確認 */
				else if (uszKey == _KEY_TIMEOUT_)
				{
					break;
				}
			}

			break;
		}while (1);
	}else if(pobTran->srBRec.uszInstallmentBit == VS_TRUE)
	{
		/* [20251215_BUG_MDF][UI] 新增分期交易用警示畫面 */
		inFunc_DisplayInstallmentWarning(pobTran);
	}

	if (ginDebug == VS_TRUE)
	{
			inLogPrintf(AT, "inCREDIT_Func_CheckResult() END !");
			inLogPrintf(AT, "----------------------------------------");
	}

	vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_CheckResult END!");

	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_Func_GetOriTransDate
Date&Time       :2016/9/21 下午 5:40
Describe        :請輸入原交易日期
*/
int inCREDIT_Func_GetOriTransDate(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
	char		szTemplate[_DISP_MSG_SIZE_ + 1];
	char		szDebugMsg[100 + 1];
	DISPLAY_OBJECT  srDispObj;
	
        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_GetOriTransDate START!");
        
	/* 若ECR發動而且有值就直接跳走 */
        if (pobTran->uszECRBit == VS_TRUE)
	{
		if (strlen(pobTran->srBRec.szCUP_TD) > 0)
		{
			return (VS_SUCCESS);
		}
		else
		{
			
		}
	}
	else if (memcmp(gszTermVersionID, "MD731UAGAS001", strlen("MD731UAGAS001")) == 0)
	{
		if (strlen(pobTran->srBRec.szCUP_TD) > 0)
		{
			return (VS_SUCCESS);
		}
	}
	else
	{
		if (pobTran->inRunOperationID == _OPERATION_REFUND_AMOUNT_FIRST_CUP_	||
		    pobTran->inRunOperationID == _OPERATION_REFUND_CTLS_CUP_		||
		    pobTran->inRunOperationID == _OPERATION_PRE_COMP_CTLS_)
		{
			return (VS_SUCCESS);
		}
	}
        
	
	while (1)
	{
		memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		srDispObj.inY = _LINE_8_7_;
		srDispObj.inR_L = _DISP_RIGHT_;
		srDispObj.inMaxLen = 4;
		srDispObj.inCanNotBypass = VS_TRUE;
		srDispObj.inColor = _COLOR_RED_;
		strcpy(srDispObj.szPromptMsg, "MMDD = ");

		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		/* 請輸入原交易日期 */
		inDISP_PutGraphic(_GET_ORI_DATE_, 0, _COORDINATE_Y_LINE_8_4_);

		strcpy(szTemplate, srDispObj.szPromptMsg);
		inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, _COLOR_RED_, _DISP_RIGHT_);
	
		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;
	
		inRetVal = inDISP_Enter8x16(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (inRetVal);

		memset(pobTran->srBRec.szCUP_TD, 0x00, sizeof(pobTran->srBRec.szCUP_TD));
		memcpy(pobTran->srBRec.szCUP_TD, srDispObj.szOutput, srDispObj.inOutputLen);
		
		if (inFunc_CheckValidOriDate(pobTran->srBRec.szCUP_TD) == VS_SUCCESS)
		{
			break;
		}	
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Date Not Valid: %s", pobTran->srBRec.szCUP_TD);
				inLogPrintf(AT, szDebugMsg);
			}
		}
		
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_Func_Get_OPT_OriTransDate
Date&Time       :2017/6/28 上午 10:34
Describe        :
*/
int inCREDIT_Func_Get_OPT_OriTransDate(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
	char		szTemplate[_DISP_MSG_SIZE_ + 1];
	char		szDebugMsg[100 + 1];
        char		szCustomerIndicator[3 + 1] = {0};
	DISPLAY_OBJECT  srDispObj;
	
	vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_OriTransDate START!");
        
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	if (pobTran->uszECRBit == VS_TRUE)
	{
		if (strlen(pobTran->srBRec.szCUP_TD) > 0)
		{
			return (VS_SUCCESS);
		}
                
		/* 兩段式收銀機連線 第一段不輸入 這邊只擋會出現在OPT的 */
		if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
		{
			return (VS_SUCCESS);
		}
		
                /* 收銀機資訊不完整時要先過卡再輸入，也就是到TRT再執行檢核 */
                if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_002_NICE_PLAZA_, _CUSTOMER_INDICATOR_SIZE_))
                {
                        return (VS_SUCCESS);
                }  
	}
	
	while (1)
	{
		memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		srDispObj.inY = _LINE_8_7_;
		srDispObj.inR_L = _DISP_RIGHT_;
		srDispObj.inMaxLen = 4;
		srDispObj.inCanNotBypass = VS_TRUE;
		srDispObj.inColor = _COLOR_RED_;
		strcpy(srDispObj.szPromptMsg, "MMDD = ");

		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		/* 請輸入原交易日期 */
		inDISP_PutGraphic(_GET_ORI_DATE_, 0, _COORDINATE_Y_LINE_8_4_);

		strcpy(szTemplate, srDispObj.szPromptMsg);
		inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, _COLOR_RED_, _DISP_RIGHT_);
	
		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;
	
		inRetVal = inDISP_Enter8x16(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (inRetVal);

		memset(pobTran->srBRec.szCUP_TD, 0x00, sizeof(pobTran->srBRec.szCUP_TD));
		memcpy(pobTran->srBRec.szCUP_TD, srDispObj.szOutput, srDispObj.inOutputLen);
		
		if (inFunc_CheckValidOriDate(pobTran->srBRec.szCUP_TD) == VS_SUCCESS)
		{
			break;
		}	
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Date Not Valid: %s", pobTran->srBRec.szCUP_TD);
				inLogPrintf(AT, szDebugMsg);
			}
		}
		
	}
	
	vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_OriTransDate END!");
	
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_Func_Get_OPT_OriTransDate_ESVC
Date&Time       :2018/5/29 下午 5:39
Describe        :For票證使用
*/
int inCREDIT_Func_Get_OPT_OriTransDate_ESVC(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
	char		szTemplate[_DISP_MSG_SIZE_ + 1];
	char		szDebugMsg[100 + 1];
        char		szCustomerIndicator[3 + 1] = {0};
	DISPLAY_OBJECT  srDispObj;
	
        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_OriTransDate_ESVC START!");
        
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
        
	if (pobTran->uszECRBit == VS_TRUE)
	{
		if (strlen(pobTran->srTRec.szTicketRefundDate) > 0)
		{
			return (VS_SUCCESS);
		}
                
		/* 兩段式收銀機連線 第一段不輸入 這邊只擋會出現在OPT的 */
		if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
		{
			return (VS_SUCCESS);
		}
		
                /* 收銀機資訊不完整時要先過卡再輸入，也就是到TRT再執行檢核 */
                if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_002_NICE_PLAZA_, _CUSTOMER_INDICATOR_SIZE_))
                {
                        return (VS_SUCCESS);
                }  
	}
	
	while (1)
	{
		memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		srDispObj.inY = _LINE_8_7_;
		srDispObj.inR_L = _DISP_RIGHT_;
		srDispObj.inMaxLen = 4;
		srDispObj.inCanNotBypass = VS_TRUE;
		srDispObj.inColor = _COLOR_RED_;
		strcpy(srDispObj.szPromptMsg, "MMDD = ");

		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		/* 請輸入原交易日期 */
		inDISP_PutGraphic(_GET_ORI_DATE_, 0, _COORDINATE_Y_LINE_8_4_);

		strcpy(szTemplate, srDispObj.szPromptMsg);
		inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, _COLOR_RED_, _DISP_RIGHT_);
	
		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;
	
		inRetVal = inDISP_Enter8x16(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (inRetVal);

		memset(pobTran->srTRec.szTicketRefundDate, 0x00, sizeof(pobTran->srTRec.szTicketRefundDate));
		memcpy(pobTran->srTRec.szTicketRefundDate, srDispObj.szOutput, srDispObj.inOutputLen);
		
		if (inFunc_CheckValidOriDate(pobTran->srTRec.szTicketRefundDate) == VS_SUCCESS)
		{
			break;
		}	
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Date Not Valid: %s", pobTran->srTRec.szTicketRefundDate);
				inLogPrintf(AT, szDebugMsg);
			}
		}
		
	}
		
	return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_GetOriAmount
Date&Time	:2016/9/21 下午 5:42
Describe	:請輸入原交易金額
*/
int inCREDIT_Func_GetOriAmount(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
        char		szTemplate[_DISP_MSG_SIZE_ + 1];
        DISPLAY_OBJECT  srDispObj;
	
	/* 若ECR發動而且有值就直接跳走 */
	if (pobTran->uszECRBit == VS_TRUE)
	{
		/* 基本上一定大於0 */
		if (pobTran->srBRec.lnOrgTxnAmount > 0)
		{
			return (VS_SUCCESS);
		}
	}
	else
	{
		/* 基本上一定大於0 */
		if (pobTran->srBRec.lnOrgTxnAmount > 0)
		{
			return (VS_SUCCESS);
		}
	}
        
        
        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
        memset(szTemplate, 0x00, sizeof(szTemplate));

        srDispObj.inY = _LINE_8_7_;
        srDispObj.inR_L = _DISP_RIGHT_;
        srDispObj.inMaxLen = inFunc_Check_Digit();
	srDispObj.inCanNotBypass = VS_TRUE;
	srDispObj.inCanNotZero = VS_TRUE;
	srDispObj.inColor = _COLOR_RED_;
        strcpy(srDispObj.szPromptMsg, "NT$ ");

        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        /* 請輸入原交易金額 */
        inDISP_PutGraphic(_GET_ORI_AMOUNT_, 0, _COORDINATE_Y_LINE_8_4_);

        /* 進入畫面時先顯示金額為0 */
        strcpy(szTemplate, "NT$ 0");
        inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, srDispObj.inColor, _DISP_RIGHT_);

	memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
	srDispObj.inOutputLen = 0;
	
        inRetVal = inDISP_Enter8x16_GetAmount(&srDispObj);

        if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
		return (inRetVal);

        pobTran->srBRec.lnOrgTxnAmount = atol(srDispObj.szOutput);
         
	return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_GetOriPreAuthAmount
Date&Time	:2016/9/22 下午 3:51
Describe	:請輸入原預授權金額
*/
int inCREDIT_Func_GetOriPreAuthAmount(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_ERROR;
        char		szTemplate[_DISP_MSG_SIZE_ + 1] = {0};
        DISPLAY_OBJECT  srDispObj;
        
        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_GetOriPreAuthAmount START!");

	/* 若ECR發動而且有值就直接跳走 */
        if (pobTran->uszECRBit == VS_TRUE)
	{
		/* 基本上一定大於0 */
		if (pobTran->srBRec.lnOrgTxnAmount > 0)
		{
			return (VS_SUCCESS);
		}
	}
	else if (memcmp(gszTermVersionID, "MD731UAGAS001", strlen("MD731UAGAS001")) == 0)
	{
		if (pobTran->srBRec.lnOrgTxnAmount > 0)
		{
			return (VS_SUCCESS);
		}
	}
	else
	{
		/* 基本上一定大於0 */
		if (pobTran->srBRec.lnOrgTxnAmount > 0)
		{
			return (VS_SUCCESS);
		}
	}
	
        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
        memset(szTemplate, 0x00, sizeof(szTemplate));

        srDispObj.inY = _LINE_8_7_;
        srDispObj.inR_L = _DISP_RIGHT_;
        srDispObj.inMaxLen = inFunc_Check_Digit();
	srDispObj.inCanNotBypass = VS_TRUE;
	srDispObj.inCanNotZero = VS_TRUE;
	srDispObj.inColor = _COLOR_RED_;
        strcpy(srDispObj.szPromptMsg, "NT$ ");

        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        /* 請輸入原預授權金額 */
        inDISP_PutGraphic(_GET_ORI_PRE_AUTH_AMOUNT_, 0, _COORDINATE_Y_LINE_8_4_);

        /* 進入畫面時先顯示金額為0 */
        strcpy(szTemplate, "NT$ 0");
        inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, srDispObj.inColor, _DISP_RIGHT_);

	memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
	srDispObj.inOutputLen = 0;
	
        inRetVal = inDISP_Enter8x16_GetAmount(&srDispObj);

        if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
		return (inRetVal);
	
	pobTran->srBRec.lnOrgTxnAmount = atol(srDispObj.szOutput);
	
	return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_Get_OPT_OriPreAuthAmount
Date&Time	:2017/9/8 上午 11:12
Describe	:請輸入原預授權金額
*/
int inCREDIT_Func_Get_OPT_OriPreAuthAmount(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_ERROR;
        char		szTemplate[_DISP_MSG_SIZE_ + 1] = {0};
        char		szCustomerIndicator[3 + 1] = {0};
        DISPLAY_OBJECT  srDispObj;
        
	vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_OriPreAuthAmount START!");
	
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	/* 若ECR發動而且有值就直接跳走 */
        if (pobTran->uszECRBit == VS_TRUE)
	{
		/* 基本上一定大於0 */
		if (pobTran->srBRec.lnOrgTxnAmount > 0)
		{
			return (VS_SUCCESS);
		}
                
		/* 兩段式收銀機連線 第一段不輸入 這邊只擋會出現在OPT的 */
		if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
		{
			return (VS_SUCCESS);
		}
                
                /* 收銀機資訊不完整時要先過卡再輸入，也就是到TRT再執行檢核 */
                if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_002_NICE_PLAZA_, _CUSTOMER_INDICATOR_SIZE_))
                {
                        return (VS_SUCCESS);
                }  
	}

        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
        memset(szTemplate, 0x00, sizeof(szTemplate));

        srDispObj.inY = _LINE_8_7_;
        srDispObj.inR_L = _DISP_RIGHT_;
        srDispObj.inMaxLen = inFunc_Check_Digit();
	srDispObj.inCanNotBypass = VS_TRUE;
	srDispObj.inCanNotZero = VS_TRUE;
	srDispObj.inColor = _COLOR_RED_;
        strcpy(srDispObj.szPromptMsg, "NT$ ");

        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        /* 請輸入原預授權金額 */
        inDISP_PutGraphic(_GET_ORI_PRE_AUTH_AMOUNT_, 0, _COORDINATE_Y_LINE_8_4_);

        /* 進入畫面時先顯示金額為0 */
        strcpy(szTemplate, "NT$ 0");
        inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, srDispObj.inColor, _DISP_RIGHT_);

	memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
	srDispObj.inOutputLen = 0;
	
        inRetVal = inDISP_Enter8x16_GetAmount(&srDispObj);

        if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
		return (inRetVal);
	
	pobTran->srBRec.lnOrgTxnAmount = atol(srDispObj.szOutput);
	
	return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_GetPreCompAmount
Date&Time	:2016/9/22 下午 3:51
Describe	:請輸入預先授權完成金額
*/
int inCREDIT_Func_GetPreCompAmount(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_ERROR;
	long		lnPreCompMaxAmt = 0;
        char		szTemplate[_DISP_MSG_SIZE_ + 1] = {0};
        DISPLAY_OBJECT  srDispObj;
	
        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_GetPreCompAmount START!");
        
	/* 若ECR發動而且有值就直接跳走 */
	if (pobTran->uszECRBit == VS_TRUE)
	{
		/* 基本上一定大於0 */
		if (pobTran->srBRec.lnTxnAmount > 0)
		{
			return (VS_SUCCESS);
		}
	}
	else if (memcmp(gszTermVersionID, "MD731UAGAS001", strlen("MD731UAGAS001")) == 0)
	{
		if (pobTran->srBRec.lnTxnAmount > 0)
		{
			return (VS_SUCCESS);
		}
	}
	else
	{
		/* 基本上一定大於0 */
		if (pobTran->srBRec.lnTxnAmount > 0)
		{
			return (VS_SUCCESS);
		}
	}
	
	/* 計算原預先授權115%的金額 */
        lnPreCompMaxAmt = (long)(((long long)pobTran->srBRec.lnOrgTxnAmount * 115) / 100); 

	while (1)
	{
		memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		srDispObj.inY = _LINE_8_7_;
		srDispObj.inR_L = _DISP_RIGHT_;
		srDispObj.inMaxLen = inFunc_Check_Digit();
		srDispObj.inCanNotBypass = VS_TRUE;
		srDispObj.inCanNotZero = VS_TRUE;
		srDispObj.inColor = _COLOR_RED_;
		strcpy(srDispObj.szPromptMsg, "NT$ ");

		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		/* 請輸入預先授權完成金額 */
		inDISP_PutGraphic(_GET_PRE_COMP_AMOUNT_, 0, _COORDINATE_Y_LINE_8_4_);

		/* 進入畫面時先顯示金額為0 */
		strcpy(szTemplate, "NT$ 0");
		inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, srDispObj.inColor, _DISP_RIGHT_);

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;

		inRetVal = inDISP_Enter8x16_GetAmount(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (inRetVal);

		/* 一般信用卡預先授權完成及銀聯預先授權完成皆有原預先授權金額之15%限制 */
		if (atol(srDispObj.szOutput) <= lnPreCompMaxAmt)
		{
			pobTran->srBRec.lnTxnAmount = atol(srDispObj.szOutput);
			pobTran->srBRec.lnTotalTxnAmount = atol(srDispObj.szOutput);
			break;
		}
		else
		{
			/* 輸入金額超過上限 請按清除鍵 */
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
			strcpy(srDispMsgObj.szErrMsg1, "輸入金額超過上限");
			srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
			continue;
		}
	}
         
	return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_Get_OPT_PreCompAmount
Date&Time	:2017/9/8 上午 11:14
Describe	:請輸入預先授權完成金額
*/
int inCREDIT_Func_Get_OPT_PreCompAmount(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_ERROR;
	long		lnPreCompMaxAmt = 0;
        char		szTemplate[_DISP_MSG_SIZE_ + 1] = {0};
        char		szCustomerIndicator[3 + 1] = {0};
        DISPLAY_OBJECT  srDispObj;
        
	vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_PreCompAmount START!");
	
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	/* 若ECR發動而且有值就直接跳走 */
        if (pobTran->uszECRBit == VS_TRUE)
	{
		if (pobTran->srBRec.lnTxnAmount > 0)
		{
			return (VS_SUCCESS);
		}
                
		/* 兩段式收銀機連線 第一段不輸入 這邊只擋會出現在OPT的 */
		if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
		{
			return (VS_SUCCESS);
		}
                
                /* 收銀機資訊不完整時要先過卡再輸入，也就是到TRT再執行檢核 */
                if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_002_NICE_PLAZA_, _CUSTOMER_INDICATOR_SIZE_))
                {
                        return (VS_SUCCESS);
                }  
	}
	
	/* 計算原預先授權115%的金額 */
	lnPreCompMaxAmt = (long)(((long long)pobTran->srBRec.lnOrgTxnAmount * 115) / 100); 
        
	while (1)
	{
		memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		srDispObj.inY = _LINE_8_7_;
		srDispObj.inR_L = _DISP_RIGHT_;
		srDispObj.inMaxLen = inFunc_Check_Digit();
		srDispObj.inCanNotBypass = VS_TRUE;
		srDispObj.inCanNotZero = VS_TRUE;
		srDispObj.inColor = _COLOR_RED_;
		strcpy(srDispObj.szPromptMsg, "NT$ ");

		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		/* 請輸入預先授權完成金額 */
		inDISP_PutGraphic(_GET_PRE_COMP_AMOUNT_, 0, _COORDINATE_Y_LINE_8_4_);

		/* 進入畫面時先顯示金額為0 */
		strcpy(szTemplate, "NT$ 0");
		inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, srDispObj.inColor, _DISP_RIGHT_);

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;

		inRetVal = inDISP_Enter8x16_GetAmount(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (inRetVal);

		/* 一般信用卡預先授權完成及銀聯預先授權完成皆有原預先授權金額之15%限制 */
		if (atol(srDispObj.szOutput) <= lnPreCompMaxAmt)
		{
			pobTran->srBRec.lnTxnAmount = atol(srDispObj.szOutput);
			pobTran->srBRec.lnTotalTxnAmount = atol(srDispObj.szOutput);
			break;
		}
		else
		{
			/* 輸入金額超過上限 請按清除鍵 */
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
			strcpy(srDispMsgObj.szErrMsg1, "輸入金額超過上限");
			srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
			continue;
		}
	}
         
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_Func_Get_Card_Number_Txno_Flow
Date&Time       :2017/6/13 下午 6:33
Describe        :請選擇？ 1.輸入交易編號 2.輸入卡號
*/
int inCREDIT_Func_Get_Card_Number_Txno_Flow(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	int	inChoice = 0;
	int	inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_8_3X3_;
	char	szKey = 0x00;
	
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* 請選擇？ 1.輸入交易編號 2.輸入卡號 */
	inDISP_PutGraphic(_MENU_GET_CARD_NO_OPTION_, 0, _COORDINATE_Y_LINE_8_4_);
	
	inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);

	while (1)
	{
		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
                szKey = uszKBD_Key();
		
		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			szKey = _KEY_TIMEOUT_;
		}
		
		if (szKey == _KEY_1_			||
		    inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_)
		{
			inRetVal = inCREDIT_Func_Get_TransactionNO(pobTran);
			break;
		}
		else if (szKey == _KEY_2_			||
		         inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_)
		{
			inRetVal = inCREDIT_Func_Get_Card_Number(pobTran);
			break;
		}
		else if (szKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else if (szKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}
		
	}
	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();
	
	return (inRetVal);
}

/*
Function        :inCREDIT_Func_Get_Card_Number
Date&Time       :2017/1/20 下午 4:10
Describe        :
*/
int inCREDIT_Func_Get_Card_Number(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
	char		szCustomerIndicator[3 + 1] = {0};
	DISPLAY_OBJECT  srDispObj;
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* 第三行顯示 請輸入卡號? */
	inDISP_PutGraphic(_GET_CARD_NO_, 0, _COORDINATE_Y_LINE_8_4_);

	memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
	srDispObj.inY = _LINE_8_7_;
	srDispObj.inR_L = _DISP_RIGHT_;
	srDispObj.inMaxLen = 19;
	srDispObj.inMenuKeyIn = pobTran->inMenuKeyin;

	memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
	srDispObj.inOutputLen = 0;
	
	inRetVal = inDISP_Enter8x16_MenuKeyIn(&srDispObj);

	if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
	{
		return (inRetVal);
	}

	if (srDispObj.inOutputLen > 0)
	{
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_026_TAKA_, _CUSTOMER_INDICATOR_SIZE_) ||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
		{
			/* For 大高卡 */
			if (srDispObj.inOutputLen == 10)
			{
				memcpy(&pobTran->srBRec.szPAN[0], "000000", 6);
				memcpy(&pobTran->srBRec.szPAN[6], &srDispObj.szOutput[0], srDispObj.inOutputLen);
			}
			else
			{
				memcpy(&pobTran->srBRec.szPAN[0], &srDispObj.szOutput[0], srDispObj.inOutputLen);
			}
		}
		else
		{
			memcpy(&pobTran->srBRec.szPAN[0], &srDispObj.szOutput[0], srDispObj.inOutputLen);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_Func_Get_TransactionNO
Date&Time       :2017/6/15 下午 3:57
Describe        :請輸入交易編號
*/
int inCREDIT_Func_Get_TransactionNO(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
	char		szTemplate[_DISP_MSG_SIZE_ + 1];
	DISPLAY_OBJECT  srDispObj;
	
	memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
	memset(szTemplate, 0x00, sizeof(szTemplate));

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* 顯示請輸入交易編號? */
	inDISP_PutGraphic(_GET_TXNO_, 0, _COORDINATE_Y_LINE_8_4_);

	srDispObj.inY = _LINE_8_7_;
	srDispObj.inR_L = _DISP_LEFT_;
	srDispObj.inMaxLen = 23;
	srDispObj.inColor = _COLOR_RED_;
	srDispObj.inCanNotBypass = VS_TRUE;
	
	while (1)
	{
		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;
	
		inRetVal = inDISP_Enter8x16(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (inRetVal);

		if (srDispObj.inOutputLen > 0)
		{
			/* 判斷交易編號小於15碼都是長度錯誤 */
			if (srDispObj.inOutputLen < 15)
			{
				/* 交易編號長度錯誤 請按清除鍵 */
				DISPLAY_OBJECT	srDispMsgObj;
				memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
				strcpy(srDispMsgObj.szDispPic1Name, _ERR_TXNO_LEN_);
				srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
				srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
				srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
				strcpy(srDispMsgObj.szErrMsg1, "");
				srDispMsgObj.inErrMsg1Line = 0;
				srDispMsgObj.inBeepTimes = 1;
				srDispMsgObj.inBeepInterval = 0;
				inDISP_Msg_BMP(&srDispMsgObj);

				return (VS_USER_CANCEL);
			}
			else
			{
				memset(pobTran->srBRec.szTxnNo, 0x00, sizeof(pobTran->srBRec.szTxnNo));
				memcpy(pobTran->srBRec.szTxnNo, srDispObj.szOutput, srDispObj.inOutputLen);
				if (inNCCC_Func_Get_PAN_From_Transaction_No(pobTran) != VS_SUCCESS)
				{
                                        /* Mirror Message */
                                        if (pobTran->uszECRBit == VS_TRUE)
                                        {
                                                inECR_SendMirror(pobTran, _MIRROR_MSG_CHECK_CARD_ERROR_);
                                        }
                        
					/* 交易編號錯誤 請按清除鍵 */
					DISPLAY_OBJECT	srDispMsgObj;
					memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
					strcpy(srDispMsgObj.szDispPic1Name, _ERR_TXNO_);
					srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
					srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
					srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
					strcpy(srDispMsgObj.szErrMsg1, "");
					srDispMsgObj.inErrMsg1Line = 0;
					srDispMsgObj.inBeepTimes = 1;
					srDispMsgObj.inBeepInterval = 0;
					inDISP_Msg_BMP(&srDispMsgObj);

					return (VS_USER_CANCEL);
				}
				else
				{
					/* 標示由交易編號獲得卡號 */
					pobTran->uszInputTxnoBit = VS_TRUE;
					break;
				}

			}
		}
		
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_Func_Get_HG_Card_Number
Date&Time       :2017/5/17 下午 5:49
Describe        :
*/
int inCREDIT_Func_Get_HG_Card_Number(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
	DISPLAY_OBJECT  srDispObj;
	
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* 第三行顯示 請輸入卡號? */
	inDISP_PutGraphic(_GET_CARD_NO_, 0, _COORDINATE_Y_LINE_8_4_);

	memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
	srDispObj.inY = _LINE_8_7_;
	srDispObj.inR_L = _DISP_RIGHT_;
	srDispObj.inMaxLen = 19;
	srDispObj.inMenuKeyIn = pobTran->inMenuKeyin;

	memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
	srDispObj.inOutputLen = 0;
	
	inRetVal = inDISP_Enter8x16_MenuKeyIn(&srDispObj);

	if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
	{
		return (inRetVal);
	}

	if (srDispObj.inOutputLen > 0)
		memcpy(&pobTran->srBRec.szHGPAN[0], &srDispObj.szOutput[0], srDispObj.inOutputLen);
	
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_Func_Get_Exp_Date
Date&Time       :2017/1/20 下午 4:10
Describe        :請輸入有效期
*/
int inCREDIT_Func_Get_Exp_Date(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
	char		szTemplate[_DISP_MSG_SIZE_ + 1];
	DISPLAY_OBJECT  srDispObj;
	
	memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
	memset(szTemplate, 0x00, sizeof(szTemplate));

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* 顯示請輸入有效期 */
	inDISP_PutGraphic(_GET_EXPIRE_DATE_, 0, _COORDINATE_Y_LINE_8_4_);

	srDispObj.inY = _LINE_8_7_;
	srDispObj.inR_L = _DISP_RIGHT_;
	srDispObj.inMaxLen = 4;
	srDispObj.inColor = _COLOR_RED_;
	srDispObj.inCanNotBypass = VS_TRUE;
	strcpy(srDispObj.szPromptMsg, "MMYY= ");

	strcpy(szTemplate, srDispObj.szPromptMsg);
	
	while (1)
	{
		inDISP_Clear_Line(srDispObj.inY, _LINE_8_8_);
		inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, _COLOR_RED_, _DISP_RIGHT_);
		
		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;
		
		inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
				return (inRetVal);

		if (srDispObj.inOutputLen == 4)
		{
			/* 因為MenuKeyIn的有效期為MMYY，但檢核有效期格式為YYMM */
			memcpy(&pobTran->srBRec.szExpDate[0], &srDispObj.szOutput[2], 2);
			memcpy(&pobTran->srBRec.szExpDate[2], &srDispObj.szOutput[0], 2);
			break;
		}
		else
		{
			continue;
		}
	
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_Func_Get_CheckNO
Date&Time       :2017/1/23 下午 3:07
Describe        :請輸入檢查碼
*/
int inCREDIT_Func_Get_CheckNO(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
	char		szTemplate[_DISP_MSG_SIZE_ + 1];
	char		szEXPDate[4 + 1];
	char		szCheckPANKey;
	DISPLAY_OBJECT  srDispObj;
	
	memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
	memset(szTemplate, 0x00, sizeof(szTemplate));

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* 顯示請輸入檢查碼? */
	inDISP_PutGraphic(_GET_CHECK_NO_, 0, _COORDINATE_Y_LINE_8_4_);

	srDispObj.inY = _LINE_8_7_;
	srDispObj.inR_L = _DISP_RIGHT_;
	srDispObj.inMaxLen = 5;
	srDispObj.inColor = _COLOR_RED_;
	
	/* 某些狀況要可以ByPass */
	/* 2013-06-03 AM 11:27:51 add by kakab 配合修改規格書CUP需求說明書20130423.doc 預先授權完成人工輸入檢查碼需可by pass */
	if (pobTran->inTransactionCode == _CUP_PRE_COMP_)
	{
		srDispObj.inCanNotBypass = VS_FALSE;
	}
	/* 2013-12-03 AM 11:27:51 add by kakab 配合修改規格書CUP需求說明書20131128.doc 銀聯退貨人工輸入檢查碼需可by pass */
	else if (pobTran->inTransactionCode == _CUP_REFUND_)
	{
		srDispObj.inCanNotBypass = VS_FALSE;
	}
	else
	{
		srDispObj.inCanNotBypass = VS_TRUE;
	}
		
	strcpy(srDispObj.szPromptMsg, "Check No.= ");

	strcpy(szTemplate, srDispObj.szPromptMsg);
	
	while (1)
	{
		inDISP_Clear_Line(srDispObj.inY, _LINE_8_8_);
		inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, _COLOR_RED_, _DISP_RIGHT_);
		
		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;
	
		inRetVal = inDISP_Enter8x16(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (inRetVal);

		if (srDispObj.inCanNotBypass == VS_FALSE && srDispObj.inOutputLen == 0)
		{
			break;
		}
		else if (srDispObj.inOutputLen < 5)
		{
                        /* Mirror Message */
                        if (pobTran->uszECRBit == VS_TRUE)
                        {
                                inECR_SendMirror(pobTran, _MIRROR_MSG_CHECK_CARD_ERROR_);
                        }
                        
			/* 檢查碼錯誤 */
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_CHECK_NO_);
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
			memset(szEXPDate, 0x00, sizeof(szEXPDate));
			inRetVal = inCARD_ExpDateEncryptAndDecrypt(pobTran, srDispObj.szOutput, szEXPDate, _EXP_DECRYPT_);
			if (inRetVal != VS_SUCCESS)
			{
                                /* Mirror Message */
                                if (pobTran->uszECRBit == VS_TRUE)
                                {
                                        inECR_SendMirror(pobTran, _MIRROR_MSG_CHECK_CARD_ERROR_);
                                }
                        
				/* 檢查碼錯誤 */
				DISPLAY_OBJECT	srDispMsgObj;
				memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
				strcpy(srDispMsgObj.szDispPic1Name, _ERR_CHECK_NO_);
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
			
			/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
			szCheckPANKey = pobTran->srBRec.szPAN[9];
			
			/* 核對第十碼卡號 */
			if (szEXPDate[4] != szCheckPANKey)
			{
                                /* Mirror Message */
                                if (pobTran->uszECRBit == VS_TRUE)
                                {
                                        inECR_SendMirror(pobTran, _MIRROR_MSG_CARD_NO_ERROR_);
                                }
                        
				/* 檢查碼錯誤 */
				DISPLAY_OBJECT	srDispMsgObj;
				memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
				strcpy(srDispMsgObj.szDispPic1Name, _ERR_CHECK_NO_);
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
				/* 因為MenuKeyIn的有效期為MMYY(所以解開時也是)，但檢核有效期格式為YYMM */
				memcpy(&pobTran->srBRec.szExpDate[0], &szEXPDate[2], 2);
				memcpy(&pobTran->srBRec.szExpDate[2], &szEXPDate[0], 2);
				pobTran->uszInputCheckNoBit = VS_TRUE;
				break;
			}
			
		}
		
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_Func_Get_CheckNO_ExpDate_Flow
Date&Time       :2017/1/23 下午 4:27
Describe        :請輸入檢查碼 和 輸入有效期的流程
*/
int inCREDIT_Func_Get_CheckNO_ExpDate_Flow(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	int	inChoice = 0;
	int	inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_8_3X3_;
	int	inDefaultChoice = 0;
	char	szKey = 0x00;
	
	/* 如果輸入交易編號，直接選擇檢查碼輸入*/
	/* TMS開啟自存聯卡號遮掩功能，退貨交易選擇輸入交易編號流程，僅支援檢查碼輸入，不支援有效期輸入。(參照規格：簽帳單之特店自存聯內卡號隱藏需求內容-20160125) */
	if (pobTran->uszInputTxnoBit == VS_TRUE)
	{
		inDefaultChoice = 1;
	}
	else if (pobTran->srBRec.inCode == _CASH_ADVANCE_)
	{
		inDefaultChoice = 2;
	}
	else
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_MENU_CHECKNO_EXPDATE_OPTION_, 0, _COORDINATE_Y_LINE_8_4_);

		inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
	}
	
	while (1)
	{
		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
                szKey = uszKBD_Key();
		
		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			szKey = _KEY_TIMEOUT_;
		}
		
		if (szKey == _KEY_1_						||
		    inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_	||
		    inDefaultChoice == 1)
		{
			inRetVal = inCREDIT_Func_Get_CheckNO(pobTran);
			break;
		}
		else if (szKey == _KEY_2_					||
		         inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_	||
			 inDefaultChoice == 2)
		{
			inRetVal = inCREDIT_Func_Get_Exp_Date(pobTran);
			break;
		}
		else if (szKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else if (szKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}
	
	}
	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();
	
	return (inRetVal);
}

/*
Function        :inCREDIT_Func_Get_Barcode1
Date&Time       :2017/2/18 下午 3:19
Describe        :請掃描或輸入第一段條碼？
*/
int inCREDIT_Func_Get_Barcode1(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
	char		szTemplate[_DISP_MSG_SIZE_ + 1];
	DISPLAY_OBJECT  srDispObj;
	
	memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
	memset(szTemplate, 0x00, sizeof(szTemplate));

	srDispObj.inY = _LINE_8_8_;
	srDispObj.inR_L = _DISP_RIGHT_;
	srDispObj.inMaxLen = 20;
	srDispObj.inColor = _COLOR_RED_;
	srDispObj.inCanNotBypass = VS_TRUE;
	srDispObj.inMenuKeyIn = pobTran->inMenuKeyin;
	
	while (1)
	{
		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;
		
		inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
				return (inRetVal);

		if (srDispObj.inOutputLen > 0 && 
		   ((memcmp(srDispObj.szOutput, "11", strlen("11")) == 0)	||
		    (memcmp(srDispObj.szOutput, "21", strlen("21")) == 0)))		/* 若有輸入兌換條碼，且前兩個值合法(11表示只有一個條碼，21表示有兩個條碼中的第一個條碼) */
		{
			memcpy(&pobTran->szL3_Barcode1[0], &srDispObj.szOutput[0], srDispObj.inOutputLen);
			sprintf(pobTran->szL3_Barcode1Len, "%02d", strlen(pobTran->szL3_Barcode1));
			break;
		}
		else
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			/* 請掃描或輸入第一段條碼？ */
			inDISP_PutGraphic(_GET_BARCODE_1_, 0, _COORDINATE_Y_LINE_8_4_);
			break;
		}
	
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_Func_Get_Barcode2
Date&Time       :2017/2/18 下午 3:19
Describe        :請掃描或輸入第二段條碼？
*/
int inCREDIT_Func_Get_Barcode2(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
	char		szTemplate[_DISP_MSG_SIZE_ + 1];
	DISPLAY_OBJECT  srDispObj;
	
	memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
	memset(szTemplate, 0x00, sizeof(szTemplate));

	srDispObj.inY = _LINE_8_8_;
	srDispObj.inR_L = _DISP_RIGHT_;
	srDispObj.inMaxLen = 20;
	srDispObj.inColor = _COLOR_RED_;
	srDispObj.inCanNotBypass = VS_TRUE;
	srDispObj.inMenuKeyIn = pobTran->inMenuKeyin;
	
	while (1)
	{
		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;
		
		inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
				return (inRetVal);

		if (srDispObj.inOutputLen > 0 && 
		   (memcmp(srDispObj.szOutput, "22", strlen("22")) == 0))		/* 若有輸入兌換條碼，且前兩個值合法(22表示有兩個條碼中的第二個條碼) */
		{
			memcpy(&pobTran->szL3_Barcode2[0], &srDispObj.szOutput[0], srDispObj.inOutputLen);
			sprintf(pobTran->szL3_Barcode2Len, "%02d", strlen(pobTran->szL3_Barcode2));
			break;
		}
		else
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			/* 請掃描或輸入第二段條碼？ */
			inDISP_PutGraphic(_GET_BARCODE_2_, 0, _COORDINATE_Y_LINE_8_4_);
			break;
		}
	
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_Func_GetDCCOriTransDate
Date&Time       :2016/11/25 下午 3:40
Describe        :請輸入DCC原交易日期
*/
int inCREDIT_Func_GetDCCOriTransDate(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
        char		szTemplate[_DISP_MSG_SIZE_ + 1];
        DISPLAY_OBJECT  srDispObj;
        
        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_GetDCCOriTransDate START!");
	
	/* 非DCC交易跳走避免和一般交易混淆 */
	if (pobTran->srBRec.uszDCCTransBit != VS_TRUE)
		return (VS_SUCCESS);
	
	/* 若ECR發動而且有值就直接跳走 */
        if (pobTran->uszECRBit == VS_TRUE)
	{
		if (strlen(pobTran->srBRec.szDCC_OTD) > 0)
		{
			return (VS_SUCCESS);
		}
		else if (strlen(pobTran->srBRec.szCUP_TD) > 0)
		{
			strcpy(pobTran->srBRec.szDCC_OTD, pobTran->srBRec.szCUP_TD);
			return (VS_SUCCESS);
		}
		else
		{
			
		}
	}
	else
	{
		if (strlen(pobTran->srBRec.szCUP_TD) > 0)
		{
			strcpy(pobTran->srBRec.szDCC_OTD, pobTran->srBRec.szCUP_TD);
			return (VS_SUCCESS);
		}
		else
		{
			
		}
	}
	
        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
        memset(szTemplate, 0x00, sizeof(szTemplate));

        srDispObj.inY = _LINE_8_7_;
        srDispObj.inR_L = _DISP_RIGHT_;
        srDispObj.inMaxLen = 4;
	srDispObj.inCanNotBypass = VS_TRUE;
	srDispObj.inColor = _COLOR_RED_;
        strcpy(srDispObj.szPromptMsg, "MMDD = ");

        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        /* 請輸入原交易日期 */
        inDISP_PutGraphic(_GET_ORI_DATE_, 0, _COORDINATE_Y_LINE_8_4_);

        strcpy(szTemplate, srDispObj.szPromptMsg);
        
	while (1)
	{
		inDISP_Clear_Line(srDispObj.inY, _LINE_8_8_);
		inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, _COLOR_RED_, _DISP_RIGHT_);
		
		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;
	
		inRetVal = inDISP_Enter8x16(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (inRetVal);

		memset(pobTran->srBRec.szDCC_OTD, 0x00, sizeof(pobTran->srBRec.szDCC_OTD));
		memcpy(pobTran->srBRec.szDCC_OTD, srDispObj.szOutput, srDispObj.inOutputLen);
		
		if (inFunc_CheckValidOriDate(pobTran->srBRec.szDCC_OTD) == VS_SUCCESS)
		{
			break;
		}	
		else
		{
			memset(pobTran->srBRec.szDCC_OTD, 0x00, sizeof(pobTran->srBRec.szDCC_OTD));
			srDispObj.inOutputLen = 0;
		}

	}
		
	return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_GetDCCOriAmount
Date&Time	:2016/11/25 下午 3:40
Describe	:請輸入DCC原交易金額
*/
int inCREDIT_Func_GetDCCOriAmount(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
        char		szTemplate[_DISP_MSG_SIZE_ + 1];
        DISPLAY_OBJECT  srDispObj;
        
        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_GetDCCOriAmount START!");
        
	/* 非DCC交易跳走避免和一般交易混淆 */
	if (pobTran->srBRec.uszDCCTransBit != VS_TRUE)
		return (VS_SUCCESS);
	
	/* 若ECR發動而且有值就直接跳走 */
        if (pobTran->uszECRBit == VS_TRUE)
	{
		if (pobTran->srBRec.lnOrgTxnAmount > 0)
		{
			return (VS_SUCCESS);
		}
		else
		{
			
		}
	}
	else
	{
		
	}
	
        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
        memset(szTemplate, 0x00, sizeof(szTemplate));

        srDispObj.inY = _LINE_8_7_;
        srDispObj.inR_L = _DISP_RIGHT_;
        srDispObj.inMaxLen = inFunc_Check_Digit();
	srDispObj.inCanNotBypass = VS_TRUE;
	srDispObj.inCanNotZero = VS_TRUE;
	srDispObj.inColor = _COLOR_RED_;
        strcpy(srDispObj.szPromptMsg, "NT$ ");

        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        /* 請輸入原交易金額 */
        inDISP_PutGraphic(_GET_ORI_AMOUNT_, 0, _COORDINATE_Y_LINE_8_4_);

        /* 進入畫面時先顯示金額為0 */
        strcpy(szTemplate, "NT$ 0");
        inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, srDispObj.inColor, _DISP_RIGHT_);

	memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
	srDispObj.inOutputLen = 0;
	
        inRetVal = inDISP_Enter8x16_GetAmount(&srDispObj);

        if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
		return (inRetVal);

        pobTran->srBRec.lnOrgTxnAmount = atol(srDispObj.szOutput);
         
	return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_GetDCCTipAmount
Date&Time	:2015/9/16 上午 11:19
Describe	:螢幕顯示原金額，輸入外幣小費金額
*/
int inCREDIT_Func_GetDCCTipAmount(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal, i;
	int		inTWDMUPower;				/* 金額的指數*/
        long		lnTipPercent, lnTiplimit, lnOrgDCCAmt;
	long		lnTipAmount;
	long		lnTWD;
        char		szTemplate[_DISP_MSG_SIZE_ + 1];
        char		szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];
        char		szKey;
	char		szTWDMU[12 + 1];
	double		dbTWDTipAmount;
        DISPLAY_OBJECT  srDispObj;
        
        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_GetDCCTipAmount START!");

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inCREDIT_Func_GetDCCTipAmount START!!");
        }
	
	/* 非DCC交易跳走避免和一般交易混淆 */
	if (pobTran->srBRec.uszDCCTransBit != VS_TRUE)
		return (VS_SUCCESS);
	
        /* 取得HDT 小費百分比Tip percent 等於零的話提示錯誤畫面 */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetTipPercent(szTemplate);
        lnTipPercent = atol(szTemplate);

        if (lnTipPercent == 0L)
        {
		/* 小費百分比錯誤 */
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, _ERR_TIP_RATE_);
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

        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "Tip Percent ： %ld", lnTipPercent);
                inLogPrintf(AT, szErrorMsg);
        }
	
	/* DCC外幣金額(判斷及顯示用) */
	lnOrgDCCAmt = atol(pobTran->srBRec.szDCC_FCA);
	
	/* ----------------------ECR小費是否超過判斷START-----------------------------------------------------*/
	if (pobTran->uszECRBit == VS_TRUE)
	{
		/* 還原被蓋掉的ECR小費 */
		pobTran->srBRec.lnTipTxnAmount = pobTran->lnOldTaxAmount;
		
		/* 如果原金額過大（9個9），計算limit會溢位，所以加判斷 */
		lnTiplimit = (long)(((long long)lnOrgDCCAmt * lnTipPercent) / 100L);

		if (ginDebug == VS_TRUE)
		{
			memset(szErrorMsg, 0x00, sizeof (szErrorMsg));
			sprintf(szErrorMsg, "Tip: %ld, TipLimit: %ld", pobTran->srBRec.lnTipTxnAmount, lnTiplimit);
			inLogPrintf(AT, szErrorMsg);
		}
	
		if (pobTran->srBRec.lnTipTxnAmount >= lnTiplimit)
		{
			/* 顯示小費過多 */
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_BIG_TIP_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
			
			pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
			return (VS_ERROR);
		}
		
		sprintf(pobTran->srBRec.szDCC_TIPFCA, "%012ld", pobTran->srBRec.lnTipTxnAmount);
	}
	/* ----------------------ECR小費是否超過判斷END-----------------------------------------------------*/
	
	/* 如果沒輸入小費要輸入，ECR有給廢金額還是要出確認畫面 */
	if (pobTran->srBRec.lnTipTxnAmount == 0)
	{
		for (i = 0;; i++)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			/* 輸入小費 */
			inDISP_PutGraphic(_GET_DCC_TIP_, 0, _COORDINATE_Y_LINE_8_4_);

			/* 第二行原始金額 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inFunc_Amount_Comma_DCC(pobTran->srBRec.szDCC_FCA, "", 0x00, _SIGNED_NONE_, 16, _PADDING_RIGHT_, pobTran->srBRec.szDCC_FCMU, pobTran->srBRec.szDCC_FCAC, szTemplate);
			inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);

			/* 第四行鍵盤輸入小費金額 */
			memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
			memset(szTemplate, 0x00, sizeof(szTemplate));

			srDispObj.inY = _LINE_8_7_;
			srDispObj.inR_L = _DISP_RIGHT_;
			/* J.當台幣輸入金額為最大值NT $9,999,999時，若外幣幣別選擇KRW時，
			 * 則交易金額換算過來大約為九位數 KRW 365,468,963，若商家欲輸入小費金額時，
			 * 若小費限額是30%，則端末機最大外幣小費金額允許輸入八位數以上。
			 * Vx520 Code裡面直接開到9位 */
			srDispObj.inMaxLen = 9;
			srDispObj.inCanNotBypass = VS_TRUE;
			srDispObj.inCanNotZero = VS_TRUE;
			srDispObj.inColor = _COLOR_RED_;
			strcpy(srDispObj.szPromptMsg, "");
			strcpy(srDispObj.szMinorUnit, pobTran->srBRec.szDCC_FCMU);
			strcpy(srDispObj.szCurrencyCode, pobTran->srBRec.szDCC_FCAC);

			/* 進入畫面時先顯示金額為0 */
			inFunc_Amount_Comma_DCC("0", "", 0x00, _SIGNED_NONE_, 16, _PADDING_RIGHT_, pobTran->srBRec.szDCC_FCMU, pobTran->srBRec.szDCC_FCAC, szTemplate);
			inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, srDispObj.inColor, _DISP_RIGHT_);

			memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
			srDispObj.inOutputLen = 0;

			inRetVal = inDISP_Enter8x16_GetDCCAmount(&srDispObj);

			if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
				return (inRetVal);

			lnTipAmount = atol(srDispObj.szOutput);

			/* 如果原金額過大（9個9），計算limit會溢位，所以加判斷 */
			lnTiplimit = (long)(((long long)lnOrgDCCAmt * lnTipPercent) / 100L);

			if (lnTipAmount > lnTiplimit)
			{
				/* 顯示小費過多 */
				DISPLAY_OBJECT	srDispMsgObj;
				memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
				strcpy(srDispMsgObj.szDispPic1Name, _ERR_BIG_TIP_);
				srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
				srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
				srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
				strcpy(srDispMsgObj.szErrMsg1, "");
				srDispMsgObj.inErrMsg1Line = 0;
				srDispMsgObj.inBeepTimes = 1;
				srDispMsgObj.inBeepInterval = 0;
				inDISP_Msg_BMP(&srDispMsgObj);

				continue;
			}
			else
			{
				/* 儲存小費金額 */
				strcpy(pobTran->srBRec.szDCC_TIPFCA, srDispObj.szOutput);

				break;
			}

		}
	}

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* DCC小費金額確認 */
	inDISP_PutGraphic(_CHECK_DCC_TIP_, 0, _COORDINATE_Y_LINE_8_4_);
	/* 第二行金額 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inFunc_Amount_Comma_DCC(pobTran->srBRec.szDCC_TIPFCA, "", 0x00, _SIGNED_NONE_, 16, _PADDING_RIGHT_, pobTran->srBRec.szDCC_FCMU, pobTran->srBRec.szDCC_FCAC, szTemplate);
	inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);

	/* 第三行金額 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	/* 初始化 */
	inTWDMUPower = 0;

	inTWDMUPower += atoi(pobTran->srBRec.szDCC_FCMU); 
	/* 10USD = 300 NTD → 1USD = 30 NTD */
	inTWDMUPower += atoi(pobTran->srBRec.szDCC_IRDU);

	inTWDMUPower += atoi(pobTran->srBRec.szDCC_IRMU);

	/* 看指數有幾位直接補0(因為是10的指數所以可以直接這樣做)，用乘的會比較耗效能 */
	memset(szTWDMU, 0x00, sizeof(szTWDMU));
	strcpy(szTWDMU, "1");
	/* inTWDMUPower + 1 是因為要10的inTWDMUPower次方，但是Pad會連最前面的1都算進去 */
	inFunc_PAD_ASCII(szTWDMU, szTWDMU, '0', inTWDMUPower + 1, _PADDING_RIGHT_);
	lnTWD = atol(szTWDMU);

	/* 先相乘，再除位數*/
	dbTWDTipAmount = (atol(pobTran->srBRec.szDCC_TIPFCA) * atol(pobTran->srBRec.szDCC_IRV) / lnTWD);

	/* 小費外幣轉台必須四捨五入 */
	if (dbTWDTipAmount - (long)dbTWDTipAmount >= 0.5)
		pobTran->srBRec.lnTipTxnAmount = ((long)dbTWDTipAmount + 1);
	else
		pobTran->srBRec.lnTipTxnAmount = ((long)dbTWDTipAmount);

	sprintf(szTemplate, "%ld", pobTran->srBRec.lnTipTxnAmount);
	inFunc_Amount_Comma_DCC(szTemplate, "", 0x00, _SIGNED_NONE_, 16, _PADDING_RIGHT_, "0", "NTD", szTemplate);
	inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_RIGHT_);

	/* 請按0確認 */
	while (1)
	{
		szKey = uszKBD_GetKey(180);
		if (szKey == _KEY_0_)
			break;
		else if (szKey == _KEY_CANCEL_)
			return (VS_USER_CANCEL);
		else if (szKey == _KEY_TIMEOUT_) 
			return (VS_TIMEOUT);
		else
			continue;
	}


	/* 總金額 */
	pobTran->srBRec.lnTotalTxnAmount = pobTran->srBRec.lnOrgTxnAmount + pobTran->srBRec.lnTipTxnAmount;

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* 確認小費新金額畫面 */
	inDISP_PutGraphic(_CHECK_TIP_, 0, _COORDINATE_Y_LINE_8_4_);
	/* 第二行外幣新金額 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%ld", atol(pobTran->srBRec.szDCC_FCA) + atol(pobTran->srBRec.szDCC_TIPFCA));
	inFunc_Amount_Comma_DCC(szTemplate, "", 0x00, _SIGNED_NONE_, 16, _PADDING_RIGHT_, pobTran->srBRec.szDCC_FCMU, pobTran->srBRec.szDCC_FCAC, szTemplate);
	inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);
	/* 第三行總金額 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTotalTxnAmount);
	inFunc_Amount_Comma(szTemplate, "NT$ " , 0x00, _SIGNED_NONE_, 16, _PADDING_RIGHT_);
	inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_RIGHT_);
	inDISP_BEEP(1, 0);

	while (1)
	{
		szKey = uszKBD_GetKey(180);
		if (szKey == _KEY_0_)
			break;
		else if (szKey == _KEY_CANCEL_)
			return (VS_USER_CANCEL);
		else if (szKey == _KEY_TIMEOUT_) 
			return (VS_TIMEOUT);
		else
			continue;
	}

        pobTran->srBRec.inCode = _TIP_;
	pobTran->inTransactionCode = _TIP_;
	pobTran->uszUpdateBatchBit = VS_TRUE;

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inCREDIT_Func_GetDCCTipAmount END!!");
        }

        return (VS_SUCCESS);
}


/*
Function        :inCREDIT_Func_GetFiscRRN
Date&Time       :2016/11/25 下午 2:20
Describe        :請輸入調單編號
*/
int inCREDIT_Func_GetFiscRRN(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
        char		szTemplate[_DISP_MSG_SIZE_ + 1];
        DISPLAY_OBJECT  srDispObj;
        
        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_GetFiscRRN START!");
	
	/* 若ECR發動而且有值就直接跳走 */
        if (pobTran->uszECRBit == VS_TRUE)
	{
		if (strlen(pobTran->srBRec.szFiscRRN) > 0)
		{
			return (VS_SUCCESS);
		}
		else
		{
			
		}
	}
	else
	{
		if (strlen(pobTran->srBRec.szFiscRRN) > 0)
		{
			return (VS_SUCCESS);
		}
	}
	
        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
        memset(szTemplate, 0x00, sizeof(szTemplate));

        srDispObj.inY = _LINE_8_7_;
        srDispObj.inR_L = _DISP_RIGHT_;
        srDispObj.inMaxLen = 12;
	srDispObj.inCanNotBypass = VS_TRUE;
	srDispObj.inColor = _COLOR_RED_;

        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        /* 請輸入調單編號 */
        inDISP_PutGraphic(_GET_FISC_RRN_, 0, _COORDINATE_Y_LINE_8_4_);

	while (1)
	{
		inDISP_Clear_Line(srDispObj.inY, _LINE_8_8_);
		
		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;
	
		inRetVal = inDISP_Enter8x16(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (inRetVal);

		memset(pobTran->srBRec.szFiscRRN, 0x00, sizeof(pobTran->srBRec.szFiscRRN));
		memcpy(pobTran->srBRec.szFiscRRN, srDispObj.szOutput, srDispObj.inOutputLen);
		
		/* 調單編號，只允許輸入12碼，不多不少 */
		if (srDispObj.inOutputLen == 12)
		{
			break;
		}
		else
		{
			memset(pobTran->srBRec.szFiscRRN, 0x00, sizeof(pobTran->srBRec.szFiscRRN));
			srDispObj.inOutputLen = 0;
		}
		
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_Func_Get_OPT_FiscRRN
Date&Time       :2017/6/28 下午 4:18
Describe        :請輸入調單編號
*/
int inCREDIT_Func_Get_OPT_FiscRRN(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
        char		szTemplate[_DISP_MSG_SIZE_ + 1];
        char		szCustomerIndicator[3 + 1] = {0};
        DISPLAY_OBJECT  srDispObj;
	
	vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_FiscRRN START!");
        
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	/* 若ECR發動而且有值就直接跳走 */
        if (pobTran->uszECRBit == VS_TRUE)
	{
		if (strlen(pobTran->srBRec.szFiscRRN) > 0)
		{
			return (VS_SUCCESS);
		}
                
		/* 兩段式收銀機連線 第一段不輸入 這邊只擋會出現在OPT的 */
		if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
		{
			return (VS_SUCCESS);
		}
                
                /* 收銀機資訊不完整時要先過卡再輸入，也就是到TRT再執行檢核 */
                if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_002_NICE_PLAZA_, _CUSTOMER_INDICATOR_SIZE_))
                {
                        return (VS_SUCCESS);
                }  
	}
	
        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
        memset(szTemplate, 0x00, sizeof(szTemplate));

        srDispObj.inY = _LINE_8_7_;
        srDispObj.inR_L = _DISP_RIGHT_;
        srDispObj.inMaxLen = 12;
	srDispObj.inCanNotBypass = VS_TRUE;
	srDispObj.inColor = _COLOR_RED_;

        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        /* 請輸入調單編號 */
        inDISP_PutGraphic(_GET_FISC_RRN_, 0, _COORDINATE_Y_LINE_8_4_);

	while (1)
	{
		inDISP_Clear_Line(srDispObj.inY, _LINE_8_8_);
		
		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;
	
		inRetVal = inDISP_Enter8x16(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (inRetVal);

		memset(pobTran->srBRec.szFiscRRN, 0x00, sizeof(pobTran->srBRec.szFiscRRN));
		memcpy(pobTran->srBRec.szFiscRRN, srDispObj.szOutput, srDispObj.inOutputLen);
		
		/* 調單編號，只允許輸入12碼，不多不少 */
		if (srDispObj.inOutputLen == 12)
		{
			break;
		}
		else
		{
			memset(pobTran->srBRec.szFiscRRN, 0x00, sizeof(pobTran->srBRec.szFiscRRN));
			srDispObj.inOutputLen = 0;
		}
		
	}
		
	return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_GetFiscRefund
Date&Time	:2016/11/25 下午 3:18
Describe	:輸入退費金額
*/
int inCREDIT_Func_GetFiscRefund(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
        char		szTemplate[_DISP_MSG_SIZE_ + 1];
        DISPLAY_OBJECT  srDispObj;
        
        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_GetFiscRefund START!");

	/* 若ECR發動而且有值就直接跳走 */
        if (pobTran->uszECRBit == VS_TRUE)
	{
		if (pobTran->srBRec.lnTxnAmount > 0)
		{
			return (VS_SUCCESS);
		}
		else
		{
			
		}
	}
	else
	{
		if (pobTran->srBRec.lnTxnAmount > 0)
		{
			return (VS_SUCCESS);
		}
	}
        
        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
        memset(szTemplate, 0x00, sizeof(szTemplate));

        srDispObj.inY = _LINE_8_7_;
        srDispObj.inR_L = _DISP_RIGHT_;
        srDispObj.inMaxLen = inFunc_Check_Digit();
	srDispObj.inCanNotBypass = VS_TRUE;
	srDispObj.inCanNotZero = VS_TRUE;
	srDispObj.inColor = _COLOR_RED_;
        strcpy(srDispObj.szPromptMsg, "NT$ ");

        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        /* 請輸入退費金額 */
        inDISP_PutGraphic(_GET_FISC_REFUND_, 0, _COORDINATE_Y_LINE_8_4_);

        /* 進入畫面時先顯示金額為0 */
        strcpy(szTemplate, "NT$ 0");
        inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, srDispObj.inColor, _DISP_RIGHT_);

	memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
	srDispObj.inOutputLen = 0;
	
        inRetVal = inDISP_Enter8x16_GetAmount(&srDispObj);

        if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
		return (inRetVal);

        pobTran->srBRec.lnTxnAmount = atol(srDispObj.szOutput);
        pobTran->srBRec.lnOrgTxnAmount = atol(srDispObj.szOutput);
        pobTran->srBRec.lnTotalTxnAmount = atol(srDispObj.szOutput);
         
	return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_Get_OPT_FiscRefund
Date&Time	:2017/6/28 下午 4:20
Describe	:輸入退費金額
*/
int inCREDIT_Func_Get_OPT_FiscRefund(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
        char		szTemplate[_DISP_MSG_SIZE_ + 1];
        char		szCustomerIndicator[3 + 1] = {0};
        DISPLAY_OBJECT  srDispObj;
        
	vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_FiscRefund START!");
	
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);

	/* 若ECR發動而且有值就直接跳走 */
        if (pobTran->uszECRBit == VS_TRUE)
	{
		if (pobTran->srBRec.lnTxnAmount > 0)
		{
			return (VS_SUCCESS);
		}
                
		/* 兩段式收銀機連線 第一段不輸入 這邊只擋會出現在OPT的 */
		if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
		{
			return (VS_SUCCESS);
		}
                
                /* 收銀機資訊不完整時要先過卡再輸入，也就是到TRT再執行檢核 */
                if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_002_NICE_PLAZA_, _CUSTOMER_INDICATOR_SIZE_))
                {
                        return (VS_SUCCESS);
                }  
	}
        
        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
        memset(szTemplate, 0x00, sizeof(szTemplate));

        srDispObj.inY = _LINE_8_7_;
        srDispObj.inR_L = _DISP_RIGHT_;
        srDispObj.inMaxLen = inFunc_Check_Digit();
	srDispObj.inCanNotBypass = VS_TRUE;
	srDispObj.inCanNotZero = VS_TRUE;
	srDispObj.inColor = _COLOR_RED_;
        strcpy(srDispObj.szPromptMsg, "NT$ ");

        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        /* 請輸入退費金額 */
        inDISP_PutGraphic(_GET_FISC_REFUND_, 0, _COORDINATE_Y_LINE_8_4_);

        /* 進入畫面時先顯示金額為0 */
        strcpy(szTemplate, "NT$ 0");
        inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, srDispObj.inColor, _DISP_RIGHT_);

	memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
	srDispObj.inOutputLen = 0;
	
        inRetVal = inDISP_Enter8x16_GetAmount(&srDispObj);

        if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
		return (inRetVal);

        pobTran->srBRec.lnTxnAmount = atol(srDispObj.szOutput);
        pobTran->srBRec.lnOrgTxnAmount = atol(srDispObj.szOutput);
        pobTran->srBRec.lnTotalTxnAmount = atol(srDispObj.szOutput);
         
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_Func_GetFiscOriTransDate
Date&Time       :2016/11/25 下午 6:02
Describe        :請輸入原交易日期，Fisc年由端末機產生
*/
int inCREDIT_Func_GetFiscOriTransDate(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
	int		inYear = 0;
	char		szYear[4 + 1];
        char		szTemplate[_DISP_MSG_SIZE_ + 1];
	char		szDebugMsg[100 + 1];
        DISPLAY_OBJECT  srDispObj;
        
        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_GetFiscOriTransDate START!");
	
	/* 若ECR發動而且有值就直接跳走 */
        if (pobTran->uszECRBit == VS_TRUE)
	{
		if (strlen(pobTran->srBRec.szFiscRefundDate) > 0)
		{
			return (VS_SUCCESS);
		}
		else
		{
			
		}
	}
	else
	{
		if (strlen(pobTran->srBRec.szFiscRefundDate) > 0)
		{
			return (VS_SUCCESS);
		}
	}
	
	memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
	memset(szTemplate, 0x00, sizeof(szTemplate));

	srDispObj.inY = _LINE_8_7_;
	srDispObj.inR_L = _DISP_RIGHT_;
	srDispObj.inMaxLen = 4;
	srDispObj.inCanNotBypass = VS_TRUE;
	srDispObj.inColor = _COLOR_RED_;
	strcpy(srDispObj.szPromptMsg, "MMDD = ");

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* 請輸入原交易日期 */
	inDISP_PutGraphic(_GET_ORI_DATE_, 0, _COORDINATE_Y_LINE_8_4_);

	strcpy(szTemplate, srDispObj.szPromptMsg);
		
	while (1)
	{
		inDISP_Clear_Line(srDispObj.inY, _LINE_8_8_);
		inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, _COLOR_RED_, _DISP_RIGHT_);
	
		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;
	
		inRetVal = inDISP_Enter8x16(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (inRetVal);

		memset(&pobTran->srBRec.szFiscRefundDate[4], 0x00, 4);
		memcpy(&pobTran->srBRec.szFiscRefundDate[4], srDispObj.szOutput, srDispObj.inOutputLen);
		
		memcpy(szYear, pobTran->srBRec.szDate, 4);
		inYear = atoi(szYear);
		
		memcpy(pobTran->srBRec.szFiscRefundDate, pobTran->srBRec.szDate, 4);
		
		/* 日期大於今天日期就填入去年 */
		if (inFunc_SunDay_Sum(pobTran->srBRec.szFiscRefundDate) > inFunc_SunDay_Sum(pobTran->srBRec.szDate))
		{
			sprintf(pobTran->srBRec.szFiscRefundDate, "%04d%s", inYear - 1, &pobTran->srBRec.szFiscRefundDate[4]);
		}
		else
		{
			/* 已在上面填入今年 */
		}
			
		if (inFunc_CheckValidDate_Include_Year(pobTran->srBRec.szFiscRefundDate) == VS_SUCCESS)
		{
			break;
		}	
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Date Not Valid: %s", pobTran->srBRec.szFiscRefundDate);
				inLogPrintf(AT, szDebugMsg);
			}
		}
		
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_Func_Get_OPT_FiscOriTransDate
Date&Time       :2017/6/28 下午 4:21
Describe        :請輸入原交易日期，Fisc年由端末機產生
*/
int inCREDIT_Func_Get_OPT_FiscOriTransDate(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
	int		inYear = 0;
	char		szYear[4 + 1];
        char		szTemplate[_DISP_MSG_SIZE_ + 1];
	char		szDebugMsg[100 + 1];
        char		szCustomerIndicator[3 + 1] = {0};
        DISPLAY_OBJECT  srDispObj;
	
	vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_FiscOriTransDate START!");
        
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	/* 若ECR發動而且有值就直接跳走 */
        if (pobTran->uszECRBit == VS_TRUE)
	{
		if (strlen(pobTran->srBRec.szFiscRefundDate) > 0)
		{
			return (VS_SUCCESS);
		}
                
		/* 兩段式收銀機連線 第一段不輸入 這邊只擋會出現在OPT的 */
		if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
		{
			return (VS_SUCCESS);
		}
                
                /* 收銀機資訊不完整時要先過卡再輸入，也就是到TRT再執行檢核 */
                if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_002_NICE_PLAZA_, _CUSTOMER_INDICATOR_SIZE_))
                {
                        return (VS_SUCCESS);
                }  
	}
	
	memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
	memset(szTemplate, 0x00, sizeof(szTemplate));

	srDispObj.inY = _LINE_8_7_;
	srDispObj.inR_L = _DISP_RIGHT_;
	srDispObj.inMaxLen = 4;
	srDispObj.inCanNotBypass = VS_TRUE;
	srDispObj.inColor = _COLOR_RED_;
	strcpy(srDispObj.szPromptMsg, "MMDD = ");

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* 請輸入原交易日期 */
	inDISP_PutGraphic(_GET_ORI_DATE_, 0, _COORDINATE_Y_LINE_8_4_);

	strcpy(szTemplate, srDispObj.szPromptMsg);
		
	while (1)
	{
		inDISP_Clear_Line(srDispObj.inY, _LINE_8_8_);
		inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, _COLOR_RED_, _DISP_RIGHT_);
	
		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;
	
		inRetVal = inDISP_Enter8x16(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (inRetVal);

		memset(&pobTran->srBRec.szFiscRefundDate[4], 0x00, 4);
		memcpy(&pobTran->srBRec.szFiscRefundDate[4], srDispObj.szOutput, srDispObj.inOutputLen);
		
		memcpy(szYear, pobTran->srBRec.szDate, 4);
		inYear = atoi(szYear);
		
		memcpy(pobTran->srBRec.szFiscRefundDate, pobTran->srBRec.szDate, 4);
		
		/* 日期大於今天日期就填入去年 */
		if (inFunc_SunDay_Sum(pobTran->srBRec.szFiscRefundDate) > inFunc_SunDay_Sum(pobTran->srBRec.szDate))
		{
			sprintf(pobTran->srBRec.szFiscRefundDate, "%04d%s", inYear - 1, &pobTran->srBRec.szFiscRefundDate[4]);
		}
		else
		{
			/* 已在上面填入今年 */
		}
			
		if (inFunc_CheckValidDate_Include_Year(pobTran->srBRec.szFiscRefundDate) == VS_SUCCESS)
		{
			break;
		}	
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Date Not Valid: %s", pobTran->srBRec.szFiscRefundDate);
				inLogPrintf(AT, szDebugMsg);
			}
		}
		
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_Func_GetProductCode
Date&Time       :2017/4/6 上午 11:07
Describe        :
*/
int inCREDIT_Func_GetProductCode(TRANSACTION_OBJECT *pobTran)
{
	int	i;
	char	szEnable[2 + 1];
	char	szKey = 0x00;
	char	szKeymap[1 + 1];	/* PCD檔用 */
	char 	szDispMsg[_DISP_MSG_SIZE_ + 1];
	char	uszFind = VS_FALSE;

        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_GetProductCode START!");
        
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
	}
	
	if (inGetProductCodeEnable(szEnable) != VS_SUCCESS)
	{
		vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_GetProductCode inGetProductCodeEnable failed");
		return (VS_ERROR);
	}
	
	if (memcmp(szEnable, "Y", strlen("Y")) != 0)
	{
                vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_GetProductCode Not_enable END!");
		return (VS_SUCCESS);
	}
	else
	{
		if (inLoadPCDRec(0) < 0)
                {
                        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_GetProductCode Load_PCDRec_failed END!");
			return (VS_SUCCESS); /* 表示沒有下產品代碼檔 */
                }
	}

	/* 若ECR發動而且有值就直接跳走 */
        if (pobTran->uszECRBit == VS_TRUE)
	{
		if (strlen(pobTran->srBRec.szProductCode) > 0)
		{
                        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_GetProductCode ProductCode_Len > 0 END!");
			return (VS_SUCCESS);
		}
		else
		{
			
		}
	}
	else
	{
		if (strlen(pobTran->srBRec.szProductCode) > 0)
		{
                        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_GetProductCode ProductCode_Len > 0 END!");
			return (VS_SUCCESS);
		}
	}
	
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* 請輸入產品代碼 */
	inDISP_PutGraphic(_GET_PRODUCT_CODE_, 0, _COORDINATE_Y_LINE_8_4_);
		
	while (1)
	{
		
		while (1)
		{
			szKey = uszKBD_GetKey(_EDC_TIMEOUT_);
			
			if (szKey >= _KEY_0_ && szKey <= _KEY_9_)
			{
				break;
			}
			else if (szKey == _KEY_ENTER_)
			{
				/* 補空白 */
				if (strlen(pobTran->srBRec.szProductCode) == 0)
				{
					memset(pobTran->srBRec.szProductCode, 0x20, sizeof(pobTran->srBRec.szProductCode) - 1);
				}
				
                                vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_GetProductCode No_User_Input_Pass END!");
				return (VS_SUCCESS);
			}
			else if (szKey == _KEY_CANCEL_)
			{
				vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_GetProductCode VS_USER_CANCEL");
				return (VS_USER_CANCEL);
			}
			else if (szKey == _KEY_TIMEOUT_)
			{
				vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_GetProductCode VS_TIMEOUT");
				return (VS_TIMEOUT);
			}
		
		}
		
		uszFind = VS_FALSE;
		/* 尋找剛剛輸入的產品代碼是否在PCD檔內 */
		for (i = 0; ; i++)
		{
			if (inLoadPCDRec(i) < 0)	/* 產品代碼檔 */
			{
				break;
			}
			
			memset(szKeymap, 0x00, sizeof(szKeymap));
			inGetKeyMap(szKeymap);
			
			/* _KEY_0_ 到 _KEY_9_ 的代碼為 '0'~'9' */
			if (szKey == szKeymap[0])
			{
				uszFind = VS_TRUE;
				break;
			}
			
		}
		
		/* 根據查詢到的結果顯示 */
		if (uszFind != VS_TRUE)
		{
			/* 顯示請重新選擇 */
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_RESELECT_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 1;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
			inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
			memset(pobTran->srBRec.szProductCode, 0x00, sizeof(pobTran->srBRec.szProductCode));
			strcpy(pobTran->srBRec.szProductCode, " ");
			
			/* 重置畫面 */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			/* 請輸入產品代碼 */
			inDISP_PutGraphic(_GET_PRODUCT_CODE_, 0, _COORDINATE_Y_LINE_8_4_);
			
			continue;
		}
		else
		{
			inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
			memset(pobTran->srBRec.szProductCode, 0x00, sizeof(pobTran->srBRec.szProductCode));
			inGetProductScript(pobTran->srBRec.szProductCode);

			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			memcpy(szDispMsg, pobTran->srBRec.szProductCode, 16);
			inDISP_ChineseFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_7_, _COLOR_BLACK_, _DISP_LEFT_);
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			memcpy(szDispMsg, &pobTran->srBRec.szProductCode[16], 16);
			inDISP_ChineseFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_8_, _COLOR_BLACK_, _DISP_LEFT_);
		}
			
	}
        
        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_GetProductCode ProductCode(%s) END!", pobTran->srBRec.szProductCode);

	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_Func_Get_OPT_ProductCode
Date&Time       :2017/4/19 下午 3:00
Describe        :
*/
int inCREDIT_Func_Get_OPT_ProductCode(TRANSACTION_OBJECT *pobTran)
{
	int	i;
	char	szEnable[2 + 1];
	char	szKey = 0x00;
	char	szKeymap[1 + 1];	/* PCD檔用 */
	char 	szDispMsg[_DISP_MSG_SIZE_ + 1];
        char	szCustomerIndicator[3 + 1] = {0};
	char	uszFind = VS_FALSE;
        
        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_ProductCode START!");
        
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);

	if (inGetProductCodeEnable(szEnable) != VS_SUCCESS)
		return (VS_ERROR);
	
	if (memcmp(szEnable, "Y", strlen("Y")) != 0)
	{
		return (VS_SUCCESS);
	}
	else
	{
		if (inLoadPCDRec(0) < 0)
			return (VS_SUCCESS); /* 表示沒有下產品代碼檔 */
	}
	

	/* 若ECR發動而且有值就直接跳走 */
        if (pobTran->uszECRBit == VS_TRUE)
	{
		if (strlen(pobTran->srBRec.szProductCode) > 0)
		{
			return (VS_SUCCESS);
		}
                
		/* 兩段式收銀機連線 第一段不輸入 這邊只擋會出現在OPT的 */
		if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
		{
			return (VS_SUCCESS);
		}
                
                /* 收銀機資訊不完整時要先過卡再輸入，也就是到TRT再執行檢核 */
                if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_002_NICE_PLAZA_, _CUSTOMER_INDICATOR_SIZE_))
                {
                        return (VS_SUCCESS);
                }  
	}

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* 請輸入產品代碼 */
	inDISP_PutGraphic(_GET_PRODUCT_CODE_, 0, _COORDINATE_Y_LINE_8_4_);

	while (1)
	{

		while (1)
		{
			szKey = uszKBD_GetKey(_EDC_TIMEOUT_);

			if (szKey >= _KEY_0_ && szKey <= _KEY_9_)
			{
				break;
			}
			else if (szKey == _KEY_ENTER_)
			{
				/* 補空白 */
				if (strlen(pobTran->srBRec.szProductCode) == 0)
				{
					memset(pobTran->srBRec.szProductCode, 0x20, sizeof(pobTran->srBRec.szProductCode) - 1);
				}

				return (VS_SUCCESS);
			}
			else if (szKey == _KEY_CANCEL_)
			{
				return (VS_USER_CANCEL);
			}
			else if (szKey == _KEY_TIMEOUT_)
			{
				return (VS_TIMEOUT);
			}

		}

		uszFind = VS_FALSE;
		/* 尋找剛剛輸入的產品代碼是否在PCD檔內 */
		for (i = 0; ; i++)
		{
			if (inLoadPCDRec(i) < 0)	/* 產品代碼檔 */
			{
				break;
			}

			memset(szKeymap, 0x00, sizeof(szKeymap));
			inGetKeyMap(szKeymap);

			/* _KEY_0_ 到 _KEY_9_ 的代碼為 '0'~'9' */
			if (szKey == szKeymap[0])
			{
				uszFind = VS_TRUE;
				break;
			}

		}

		/* 根據查詢到的結果顯示 */
		if (uszFind != VS_TRUE)
		{
			/* 顯示請重新選擇 */
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_RESELECT_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 1;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
			inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
			memset(pobTran->srBRec.szProductCode, 0x00, sizeof(pobTran->srBRec.szProductCode));
			strcpy(pobTran->srBRec.szProductCode, " ");

			/* 重置畫面 */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			/* 請輸入產品代碼 */
			inDISP_PutGraphic(_GET_PRODUCT_CODE_, 0, _COORDINATE_Y_LINE_8_4_);

			continue;
		}
		else
		{
			inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
			memset(pobTran->srBRec.szProductCode, 0x00, sizeof(pobTran->srBRec.szProductCode));
			inGetProductScript(pobTran->srBRec.szProductCode);

			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			memcpy(szDispMsg, pobTran->srBRec.szProductCode, 16);
			inDISP_ChineseFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_7_, _COLOR_BLACK_, _DISP_LEFT_);
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			memcpy(szDispMsg, &pobTran->srBRec.szProductCode[16], 16);
			inDISP_ChineseFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_8_, _COLOR_BLACK_, _DISP_LEFT_);
		}

	}
        
        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_ProductCode END!");

	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_Func_GetCVV2
Date&Time       :2017/6/8 下午 3:16
Describe        :
*/
int inCREDIT_Func_GetCVV2(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
	char		szCardLable[20 + 1];
	char		szFuncEnable[2 + 1];
	DISPLAY_OBJECT  srDispObj;
	
	vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_GetCVV2 START!");
	
	memset(szCardLable, 0x00, sizeof(szCardLable));
	inGetCardLabel(szCardLable);
	
	/* AE由GET 4DBC那一隻來處理 */
	/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
	if (memcmp(szCardLable, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)) == 0)
	{
		return (VS_SUCCESS);
	}
	else
	{
		/* 剩下的要輸入CVV2 or CID */
	}
	
	/* 若ECR發動而且有值就直接跳走 */
        if (pobTran->uszECRBit == VS_TRUE)
	{
		if (strlen(pobTran->szCVV2Value) > 0)
		{
			return (VS_SUCCESS);
		}
		else
		{
			
		}
	}
	else
	{
		if (strlen(pobTran->szCVV2Value) > 0)
		{
			return (VS_SUCCESS);
		}
	}
	
        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
	
        srDispObj.inY = _LINE_8_7_;
        srDispObj.inR_L = _DISP_RIGHT_;
        srDispObj.inMaxLen = 3;
	srDispObj.inColor = _COLOR_RED_;
	
	/* 若FORCECVV2未開可ByPass */
	/* Discover看ForceCID，其他看ForceCVV2 */
	if (memcmp(szCardLable, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)) == 0)
	{
		memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
		inGetFORCE_CID(szFuncEnable);
		if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
		{
			srDispObj.inCanNotBypass = VS_TRUE;
		}
		else
		{
			srDispObj.inCanNotBypass = VS_FALSE;
		}
	}
	else
	{
		memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
		inGetFORCECVV2(szFuncEnable);
		if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
		{
			srDispObj.inCanNotBypass = VS_TRUE;
		}
		else
		{
			srDispObj.inCanNotBypass = VS_FALSE;
		}
	}

	while (1)
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		/* 請輸入背面末三碼 */
		inDISP_PutGraphic(_GET_CVV2_, 0, _COORDINATE_Y_LINE_8_4_);

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;
	
		inRetVal = inDISP_Enter8x16(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (inRetVal);

		if (srDispObj.inOutputLen == 3)
		{
			memcpy(&pobTran->szCVV2Value[0], &srDispObj.szOutput[0], srDispObj.inOutputLen);
			break;
		}
		else
		{
			break;
		}

	}

	if (strlen(pobTran->szCVV2Value) > 0)
		pobTran->srBRec.uszCVV2Bit = VS_TRUE;
	
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_Func_Get_OPT_RFNumber
Date&Time       :2018/1/18 下午 3:10
Describe        :請輸入RF序號
*/
int inCREDIT_Func_Get_OPT_RFNumber(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
        char		szTemplate[_DISP_MSG_SIZE_ + 1];
	char		szDebugMsg[100 + 1];
        char		szCustomerIndicator[3 + 1] = {0};
        DISPLAY_OBJECT  srDispObj;
        
        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_RFNumber START!");
        
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	/* 若ECR發動而且有值就直接跳走 */
        if (pobTran->uszECRBit == VS_TRUE)
	{
		/* 全空白不給過 */
		if (memcmp(pobTran->srTRec.szTicketRefundCode, "      ", 6) == 0)
		{

		}
		else
		{
			return (VS_SUCCESS);
		}
                
		/* 兩段式收銀機連線 第一段不輸入 這邊只擋會出現在OPT的 */
		if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
		{
			return (VS_SUCCESS);
		}
                
                /* 收銀機資訊不完整時要先過卡再輸入，也就是到TRT再執行檢核 */
                if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_002_NICE_PLAZA_, _CUSTOMER_INDICATOR_SIZE_))
                {
                        return (VS_SUCCESS);
                }  
	}
	else
	{
		if (strlen(pobTran->srTRec.szTicketRefundCode) > 0)
		{
			return (VS_SUCCESS);
		}
	}
	
        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
        memset(szTemplate, 0x00, sizeof(szTemplate));

        srDispObj.inY = _LINE_8_7_;
        srDispObj.inR_L = _DISP_RIGHT_;
        srDispObj.inMaxLen = 6;
	srDispObj.inCanNotBypass = VS_TRUE;
	srDispObj.inColor = _COLOR_RED_;

        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        /* 請輸入RF序號 */
        inDISP_PutGraphic(_GET_RF_NUMBER_, 0, _COORDINATE_Y_LINE_8_4_);

	while (1)
	{
		inDISP_Clear_Line(srDispObj.inY, _LINE_8_8_);
		
		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;
	
		/* 修改為可輸入英文 by Russell 2020/3/16 上午 10:47 */
		inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (inRetVal);

		memset(pobTran->srTRec.szTicketRefundCode, 0x00, sizeof(pobTran->srTRec.szTicketRefundCode));
		memcpy(pobTran->srTRec.szTicketRefundCode, srDispObj.szOutput, srDispObj.inOutputLen);
		
		/* RF序號，只允許輸入12碼，不多不少 */
		if (srDispObj.inOutputLen == 6)
		{
			break;
		}
		else
		{
			memset(pobTran->srTRec.szTicketRefundCode, 0x00, sizeof(pobTran->srTRec.szTicketRefundCode));
			srDispObj.inOutputLen = 0;
		}
		
	}
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "inCREDIT_Func_Get_OPT_RFNumber (RF = %s)END", pobTran->srTRec.szTicketRefundCode);
		inLogPrintf(AT, szDebugMsg);
	}
		
	return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_Get_OPT_Amount_Trust
Date&Time	:2025/9/11 下午 2:44
Describe	:輸入金額
*/
int inCREDIT_Func_Get_OPT_Amount_Trust(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
        char		szTemplate[_DISP_MSG_SIZE_ + 1];
        DISPLAY_OBJECT  srDispObj;
        
        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_Amount START");

        /* 若ECR或idle有輸入金額就跳走，OPT已經輸入過 */
        if (pobTran->uszECRBit == VS_TRUE)
        {
                if (pobTran->srTrustRec.lnTxnAmount > 0)
                {
                        return (VS_SUCCESS);
                }

                /* 兩段式收銀機連線 第一段不輸入 這邊只擋會出現在OPT的 */
                if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
                {
                        return (VS_SUCCESS);
                }
        }

        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
        memset(szTemplate, 0x00, sizeof(szTemplate));

        srDispObj.inY = _LINE_8_7_;
        srDispObj.inR_L = _DISP_RIGHT_;
        srDispObj.inMaxLen = inFunc_Check_Digit();      /* 不可超過9，long變數最多放9位 */
        srDispObj.inCanNotBypass = VS_TRUE;
        srDispObj.inCanNotZero = VS_TRUE;
        srDispObj.inColor = _COLOR_RED_;
        srDispObj.inTouchSensorFunc = _Touch_OX_LINE8_8_;
        strcpy(srDispObj.szPromptMsg, "NT$ ");

        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	if (pobTran->srTrustRec.inCode == _TRUST_SALE_)
	{
		/* 輸入應付金額 */
		inDISP_ChineseFont("輸入應付金額", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_CENTER_);
	}
	else
	{
		/* 輸入金額 */
		inDISP_ChineseFont("輸入金額", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_CENTER_);
	}
        inDISP_PutGraphic(_MSG_ENTER_OR_CANCEL_, 0, _COORDINATE_Y_LINE_8_8_);

        memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
        srDispObj.inOutputLen = 0;

        inRetVal = inDISP_Enter8x16_GetAmount(&srDispObj);

        if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
        {
                vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_Amount_Trust_Sale Timeout_Or_UserCancel(%d) END!", inRetVal);
                return (inRetVal);
        }

        pobTran->srTrustRec.lnTxnAmount = atol(srDispObj.szOutput);
        
        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_Amount_Trust_Sale END");
         
	return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_Get_OPT_TrustBeneficiaryId
Date&Time	:2025/9/11 下午 6:23
Describe	:for idle進入流程用，輸入受益人ID
*/
int inCREDIT_Func_Get_OPT_TrustBeneficiaryId(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_SUCCESS;
	int		inTrustBeneficiaryIdLen = 10;	/* 預設值為10 */
        DISPLAY_OBJECT  srDispObj = {};
        
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inCREDIT_Func_Get_OPT_TrustBeneficiaryId() START !");
        }
        
        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_TrustBeneficiaryId START");
	
	/* ECR沒有輸入輸入受益人ID流程 */
        if (pobTran->uszECRBit == VS_TRUE)
	{
		return (VS_SUCCESS);
	}
	else
	{
		if (strlen(pobTran->szIDNumber) > 0)
		{
                        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_TrustBeneficiaryId Id_len>0(%s) END!", pobTran->szIDNumber);
			return (VS_SUCCESS);
		}
	}

	memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
	srDispObj.inY = _LINE_8_5_;
	srDispObj.inR_L = _DISP_LEFT_;
	srDispObj.inMaxLen = inTrustBeneficiaryIdLen;
	srDispObj.inColor = _COLOR_RED_;
        srDispObj.inTimeout = 60;
	
	while (1)
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		/* 輸入受益人ID */
                inDISP_ChineseFont("輸入受益人ID", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_CENTER_);
                inDISP_PutGraphic(_MSG_ENTER_OR_CANCEL_, 0, _COORDINATE_Y_LINE_8_8_);
	
		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;

		inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
		{
			vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_TrustBeneficiaryId Timeout_Or_UserCancel(%d) END!", inRetVal);
			return (inRetVal);
		}

		if (srDispObj.inOutputLen >= 0)
		{
			/* 長度不符合，清空 */
			if (srDispObj.inOutputLen != inTrustBeneficiaryIdLen)
			{
                                vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_TrustBeneficiaryId invalid() END!");
                                DISPLAY_OBJECT	srDispMsgObj;
				memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
				strcpy(srDispMsgObj.szDispPic1Name, "");
				srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
				srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
				srDispMsgObj.inTimeout = 60;
				strcpy(srDispMsgObj.szErrMsg1, "ID身份證字號錯誤");
				srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
				srDispMsgObj.inErrMsg1FontSize = _FONTSIZE_8X22_;
				srDispMsgObj.inBeepTimes = 1;
				srDispMsgObj.inBeepInterval = 0;
				inDISP_Msg_BMP(&srDispMsgObj);
                                continue;
			}
                        
                        /* 小寫轉大寫，若非小寫英文不會處理，會返回原字元 */
                        srDispObj.szOutput[0] = (char)toupper(srDispObj.szOutput[0]);
                        srDispObj.szOutput[1] = (char)toupper(srDispObj.szOutput[1]);

                        /* 不符合ID格式，清空 */
                        if (inFunc_Check_ID_Valid(srDispObj.szOutput) == VS_FALSE)
                        {
                                vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_TrustBeneficiaryId invalid() END!");
                                DISPLAY_OBJECT	srDispMsgObj;
				memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
				strcpy(srDispMsgObj.szDispPic1Name, "");
				srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
				srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
				srDispMsgObj.inTimeout = 60;
				strcpy(srDispMsgObj.szErrMsg1, "ID身份證字號錯誤");
				srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
				srDispMsgObj.inErrMsg1FontSize = _FONTSIZE_8X22_;
				srDispMsgObj.inBeepTimes = 1;
				srDispMsgObj.inBeepInterval = 0;
				inDISP_Msg_BMP(&srDispMsgObj);
                                continue;
                        }
                        
			memcpy(&pobTran->szIDNumber[0], &srDispObj.szOutput[0], srDispObj.inOutputLen);
			break;
		}
	}
        
        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_TrustBeneficiaryId END");
        
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inCREDIT_Func_Get_OPT_TrustBeneficiaryId() END !");
                inLogPrintf(AT, "----------------------------------------");
        }

	return (VS_SUCCESS);
}

/*
Function	:inCREDIT_Func_Get_OPT_PatientId
Date&Time	:2025/9/11 下午 6:23
Describe	:for idle進入流程用，輸入病患ID
*/
int inCREDIT_Func_Get_OPT_PatientId(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_SUCCESS;
	int		inPatientIdLen = 10;	/* 預設值為10 */
        char            szCheck_IDEnable[1 + 1] = {0};
        DISPLAY_OBJECT  srDispObj = {};
        
        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_PatientId START");
        
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inCREDIT_Func_Get_OPT_PatientId() START !");
        }
        
        memset(szCheck_IDEnable, 0x00, sizeof(szCheck_IDEnable));
	inGetCHECK_ID(szCheck_IDEnable);
	if (memcmp(szCheck_IDEnable, "Y", strlen("Y")) != 0)
	{
                vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_PatientId NotEnable_Pass END!");
		return (VS_SUCCESS);
	}
	
	/* 若ECR發動直接跳走或前面已輸入過櫃號(For idle 進入流程) */
        if (pobTran->uszECRBit == VS_TRUE)
	{
		if (strlen(pobTran->szPatientId) > 0)
		{
                        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_PatientId Id_len>0(%s) END!", pobTran->szPatientId);
			return (VS_SUCCESS);
		}
                
		/* 兩段式收銀機連線 第一段不輸入 這邊只擋會出現在OPT的 */
		if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
		{
                        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_PatientId InquiryFirst=True END!");
			return (VS_SUCCESS);
		}
	}
	else
	{
		if (strlen(pobTran->szPatientId) > 0)
		{
                        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_PatientId Id_len>0(%s) END!", pobTran->szPatientId);
			return (VS_SUCCESS);
		}
	}

	memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
	srDispObj.inY = _LINE_8_6_;
	srDispObj.inR_L = _DISP_LEFT_;
	srDispObj.inMaxLen = inPatientIdLen;
	srDispObj.inColor = _COLOR_RED_;
	if (pobTran->uszECRBit == VS_TRUE)
	{
		srDispObj.inTimeout = 25;
	}
	else
	{
		srDispObj.inTimeout = 60;
	}
        
        inECR_SendMirror(pobTran, _MIRROR_MSG_8N1_Standard_PLS_ENTER_PATIENT_ID_);
	
	while (1)
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		/* 輸入受益人ID */
                inDISP_ChineseFont("輸入病患或", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_CENTER_);
		inDISP_ChineseFont("入住者ID", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
                inDISP_PutGraphic(_MSG_ENTER_OR_CANCEL_, 0, _COORDINATE_Y_LINE_8_8_);
	
		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;

		inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
		{
                        inECR_SendMirror(pobTran, _MIRROR_MSG_8N1_Standard_NOT_ENTER_PATIENT_ID_);
                        pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_NOT_ENTER_PATIENTID_;
			vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_PatientId Timeout_Or_UserCancel(%d) END!", inRetVal);
			return (inRetVal);
		}

		if (srDispObj.inOutputLen >= 0)
		{
			/* 長度不符合，清空 */
			if (srDispObj.inOutputLen != inPatientIdLen)
			{
                                vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_PatientId invalid() END!");
                                DISPLAY_OBJECT	srDispMsgObj;
				memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
				strcpy(srDispMsgObj.szDispPic1Name, "");
				srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
				srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
				srDispMsgObj.inTimeout = 60;
				strcpy(srDispMsgObj.szErrMsg1, "ID身份證字號錯誤");
				srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
				srDispMsgObj.inErrMsg1FontSize = _FONTSIZE_8X22_;
				srDispMsgObj.inBeepTimes = 1;
				srDispMsgObj.inBeepInterval = 0;
				inDISP_Msg_BMP(&srDispMsgObj);
                                continue;
			}
                        
                        /* 小寫轉大寫，若非小寫英文不會處理，會返回原字元 */
                        srDispObj.szOutput[0] = (char)toupper(srDispObj.szOutput[0]);
                        srDispObj.szOutput[1] = (char)toupper(srDispObj.szOutput[1]);

                        /* 不符合ID格式，清空 */
                        if (inFunc_Check_ID_Valid(srDispObj.szOutput) == VS_FALSE)
                        {
                                vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_PatientId invalid() END!");
                                DISPLAY_OBJECT	srDispMsgObj;
				memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
				strcpy(srDispMsgObj.szDispPic1Name, "");
				srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
				srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
				srDispMsgObj.inTimeout = 60;
				strcpy(srDispMsgObj.szErrMsg1, "ID身份證字號錯誤");
				srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
				srDispMsgObj.inErrMsg1FontSize = _FONTSIZE_8X22_;
				srDispMsgObj.inBeepTimes = 1;
				srDispMsgObj.inBeepInterval = 0;
				inDISP_Msg_BMP(&srDispMsgObj);
                                continue;
                        }
                        
			memcpy(&pobTran->szPatientId[0], &srDispObj.szOutput[0], srDispObj.inOutputLen);
			break;
		}
	}
        
        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_PatientId END");
        
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inCREDIT_Func_Get_OPT_PatientId() END !");
                inLogPrintf(AT, "----------------------------------------");
        }

	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_Func_Get_OPT_TrustInstitutionCode
Date&Time       :2025/9/15 下午 1:54
Describe        :輸入信託銀行代碼
*/
int inCREDIT_Func_Get_OPT_TrustInstitutionCode(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;
	int	inMaxLen = 0;
	char	szTemplate[_DISP_MSG_SIZE_ + 1];
	char	szDebugMsg[100 + 1];
	DISPLAY_OBJECT  srDispObj;

	vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_TrustInstitutionCode START!");

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inCREDIT_Func_Get_OPT_TrustInstitutionCode(%s) START !", pobTran->srTrustRec.szTrustInstitutionCode);
	}
	
	/* ECR跳過此流程 */
	if (pobTran->uszECRBit == VS_TRUE)
	{
		/* 全空白不給過 */
		if (memcmp(pobTran->srTrustRec.szTrustInstitutionCode, "       ", 7) == 0)
		{

		}
		else
		{
			return (VS_SUCCESS);
		}
	}
	else
	{
		if (strlen(pobTran->srTrustRec.szTrustInstitutionCode) > 0)
		{
			return (VS_SUCCESS);
		}
	}
        
	inMaxLen = 7;

	memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
	memset(szTemplate, 0x00, sizeof(szTemplate));

	srDispObj.inY = _LINE_8_7_;
	srDispObj.inR_L = _DISP_RIGHT_;
	srDispObj.inMaxLen = inMaxLen;
	srDispObj.inCanNotBypass = VS_TRUE;
	srDispObj.inColor = _COLOR_RED_;

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* 輸入信託銀行代碼 */
	inDISP_ChineseFont("輸入信託銀行代碼", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_CENTER_);
	inDISP_PutGraphic(_MSG_ENTER_OR_CANCEL_, 0, _COORDINATE_Y_LINE_8_8_);

	while (1)
	{
		inDISP_Clear_Line(srDispObj.inY, _LINE_8_7_);
		
		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;
	
		/* 修改為可輸入英文 by Russell 2020/3/16 上午 10:47 */
		inRetVal = inDISP_Enter8x16(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (inRetVal);

		/* [114261]信託模式一~三新增機構自行產QR Code 
		[260316] 新增查長度3-7碼(輸入若為822，左靠右補空白)判斷條件 */
		if(srDispObj.inOutputLen == 3 && !memcmp(srDispObj.szOutput,"822",3))
		{
			memset(&pobTran->srTrustRec.szTrustInstitutionCode[0], 0x00, sizeof(pobTran->srTrustRec.szTrustInstitutionCode));
			memset(pobTran->srTrustRec.szTrustInstitutionCode, ' ', inMaxLen);
			memcpy(pobTran->srTrustRec.szTrustInstitutionCode, srDispObj.szOutput, srDispObj.inOutputLen);
			break;
		}else if (srDispObj.inOutputLen >= 3 &&
					srDispObj.inOutputLen <= inMaxLen)
		{
			memset(&pobTran->srTrustRec.szTrustInstitutionCode[0], 0x00, sizeof(pobTran->srTrustRec.szTrustInstitutionCode));
			memset(pobTran->srTrustRec.szTrustInstitutionCode, ' ', inMaxLen);
			memcpy(pobTran->srTrustRec.szTrustInstitutionCode, srDispObj.szOutput, srDispObj.inOutputLen);
			break;
		}
	}
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "inCREDIT_Func_Get_OPT_TrustInstitutionCode (%s)END", pobTran->srTrustRec.szTrustInstitutionCode);
		inLogPrintf(AT, szDebugMsg);
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_Func_Get_OPT_ReconciliationNo
Date&Time       :2025/9/15 下午 1:54
Describe        :輸入銷帳編號
*/
int inCREDIT_Func_Get_OPT_ReconciliationNo(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_SUCCESS;
        int             inMaxLen = 20;
        char		szTemplate[_DISP_MSG_SIZE_ + 1];
	char		szDebugMsg[100 + 1];
        DISPLAY_OBJECT  srDispObj;
        
        vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_ReconciliationNo START!");
        
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inCREDIT_Func_Get_OPT_ReconciliationNo() START !");
        }
	
	/* ECR跳過此流程 */
        if (pobTran->uszECRBit == VS_TRUE)
	{
		/* 全空白不給過 */
		if (memcmp(pobTran->srTrustRec.szReconciliationNo, "                    ", 20) == 0)
		{

		}
		else
		{
                        return (VS_SUCCESS);
                }
	}
	else
	{
		if (strlen(pobTran->srTrustRec.szReconciliationNo) > 0)
		{
			return (VS_SUCCESS);
		}
	}
	
        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
        memset(szTemplate, 0x00, sizeof(szTemplate));

        srDispObj.inY = _LINE_8_7_;
        srDispObj.inR_L = _DISP_RIGHT_;
        srDispObj.inMaxLen = inMaxLen;
	srDispObj.inCanNotBypass = VS_FALSE;
	srDispObj.inColor = _COLOR_RED_;

        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        /* 輸入銷帳編號 */
        inDISP_ChineseFont("輸入銷帳編號", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_CENTER_);
        inDISP_PutGraphic(_MSG_ENTER_OR_CANCEL_, 0, _COORDINATE_Y_LINE_8_8_);

	while (1)
	{
		inDISP_Clear_Line(srDispObj.inY, _LINE_8_7_);
		
		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;
	
		inRetVal = inDISP_Enter8x16(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (inRetVal);

		if (srDispObj.inOutputLen == 0)
		{
			memset(pobTran->srTrustRec.szReconciliationNo, 0x00, sizeof(pobTran->srTrustRec.szReconciliationNo));
			break;
		}else if(srDispObj.inOutputLen <= inMaxLen)
                {   /* [113110-信託需求][輸入項] 修改成只要有輸入資料確認,就進行下個步驟 2025/11/20  */
                    memset(pobTran->srTrustRec.szReconciliationNo, ' ', sizeof(pobTran->srTrustRec.szReconciliationNo) - 1);
                    memset(&pobTran->srTrustRec.szReconciliationNo[sizeof(pobTran->srTrustRec.szReconciliationNo) - 1], 0x00, 1);
                    memcpy(pobTran->srTrustRec.szReconciliationNo, srDispObj.szOutput, srDispObj.inOutputLen);
                    break;
                }
	}
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "inCREDIT_Func_Get_OPT_ReconciliationNo (%s)END", pobTran->srTrustRec.szReconciliationNo);
		inLogPrintf(AT, szDebugMsg);
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_Func_Get_OPT_TrustRRN
Date&Time       :2025/10/8 下午 5:13
Describe        :請輸入共用收單序號
*/
int inCREDIT_Func_Get_OPT_TrustRRN(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_SUCCESS;
        char		szTemplate[_DISP_MSG_SIZE_ + 1] = {0};
        DISPLAY_OBJECT  srDispObj = {};
	
	vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_TrustRRN START!");
	
	/* 若ECR發動而且有值就直接跳走 */
        if (pobTran->uszECRBit == VS_TRUE)
	{
		if (strlen(pobTran->srTrustRec.szTrustRRN) > 0)
		{
			return (VS_SUCCESS);
		}
	}
	
        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
        memset(szTemplate, 0x00, sizeof(szTemplate));

        srDispObj.inY = _LINE_8_7_;
        srDispObj.inR_L = _DISP_RIGHT_;
        srDispObj.inMaxLen = 11;
	srDispObj.inCanNotBypass = VS_TRUE;
	srDispObj.inColor = _COLOR_RED_;

        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        /* 輸入共用收單序號 */
        inDISP_ChineseFont("輸入共用收單序號", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_LEFT_);
        inDISP_PutGraphic(_MSG_ENTER_OR_CANCEL_, 0, _COORDINATE_Y_LINE_8_8_);

	while (1)
	{
		inDISP_Clear_Line(srDispObj.inY, _LINE_8_7_);
		
		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;
	
		inRetVal = inDISP_Enter8x16(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (inRetVal);
		
		/* 共用收單序號，只允許輸入11碼，不多不少 */
		if (srDispObj.inOutputLen == 11)
		{
                        memset(pobTran->srTrustRec.szTrustRRN, 0x00, sizeof(pobTran->srTrustRec.szTrustRRN));
                        memcpy(pobTran->srTrustRec.szTrustRRN, srDispObj.szOutput, srDispObj.inOutputLen);
                        memcpy(&pobTran->srTrustRec.szTrustRRN[11], " ", 1);
			break;
		}
		else
		{
			memset(pobTran->srTrustRec.szTrustRRN, 0x00, sizeof(pobTran->srTrustRec.szTrustRRN));
			srDispObj.inOutputLen = 0;
		}
		
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_Func_Get_OPT_TrustVerificationCode
Date&Time       :2025/10/20 上午 10:25
Describe        :請輸入客戶約定驗證碼
*/
int inCREDIT_Func_Get_OPT_TrustVerificationCode(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	char	szTemplate[_DISP_MSG_SIZE_ + 1] = {0};
	DISPLAY_OBJECT  srDispObj = {};
	
	vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_TrustVerificationCode START!");

	/* 不是靜態QR Code就不用輸入 */
	if (memcmp(pobTran->szTrustQRCodeType, _TRUST_QRCODE_TYPE_STATIC_, _TRUST_QRCODE_TYPE_LEN_))
	{
		vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_TrustVerificationCode No Need Enter(%s)", pobTran->szTrustQRCodeType);
		return (VS_SUCCESS);
	}
	
	memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
	memset(szTemplate, 0x00, sizeof(szTemplate));

	srDispObj.inY = _LINE_8_7_;
	srDispObj.inR_L = _DISP_RIGHT_;
	srDispObj.inMaxLen = 3;
	srDispObj.inCanNotBypass = VS_TRUE;
	srDispObj.inColor = _COLOR_RED_;

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* 輸入共用收單序號 */
	inDISP_ChineseFont("輸入客戶約定", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_CENTER_);
	inDISP_ChineseFont("驗證碼", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);

	while (1)
	{
		inDISP_Clear_Line(srDispObj.inY, _LINE_8_7_);

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;

		inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (inRetVal);

		/* 客戶約定驗證碼，只允許輸入3碼，不多不少 */
		if (srDispObj.inOutputLen == 3)
		{
			memset(pobTran->szTrustVerificationCode, 0x00, sizeof(pobTran->szTrustVerificationCode));
			memcpy(pobTran->szTrustVerificationCode, srDispObj.szOutput, srDispObj.inOutputLen);
			break;
		}
		else
		{
			memset(pobTran->szTrustVerificationCode, 0x00, sizeof(pobTran->szTrustVerificationCode));
			srDispObj.inOutputLen = 0;
		}

	}

	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_Func_Get_OPT_OriTransDate_Trust
Date&Time       :2025/10/20 下午 6:11
Describe        :
*/
int inCREDIT_Func_Get_OPT_OriTransDate_Trust(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;
	char		szTemplate[_DISP_MSG_SIZE_ + 1];
	char		szDebugMsg[100 + 1];
        char		szCustomerIndicator[3 + 1] = {0};
	DISPLAY_OBJECT  srDispObj;
	
	vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_OriTransDate_Trust START!");
        
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	if (pobTran->uszECRBit == VS_TRUE)
	{
		if (inFunc_CheckFullSpace(pobTran->srBRec.szCUP_TD) == VS_TRUE)
		{
			
		}
                else
                {
                        return (VS_SUCCESS);
                }
	}
	
	while (1)
	{
		memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		srDispObj.inY = _LINE_8_7_;
		srDispObj.inR_L = _DISP_RIGHT_;
		srDispObj.inMaxLen = 4;
		srDispObj.inCanNotBypass = VS_TRUE;
		srDispObj.inColor = _COLOR_RED_;
		strcpy(srDispObj.szPromptMsg, "MMDD = ");

		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		/* 輸入原交易日期 */
		inDISP_ChineseFont("輸入原交易日期", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);

		strcpy(szTemplate, srDispObj.szPromptMsg);
		inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, srDispObj.inY, _COLOR_RED_, _DISP_RIGHT_);
		
		/* 請按確認或清除 */
                inDISP_PutGraphic(_MSG_ENTER_OR_CANCEL_, 0, _COORDINATE_Y_LINE_8_8_);
	
		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;
	
		inRetVal = inDISP_Enter8x16(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (inRetVal);

		memset(pobTran->srBRec.szCUP_TD, 0x00, sizeof(pobTran->srBRec.szCUP_TD));
		memcpy(pobTran->srBRec.szCUP_TD, srDispObj.szOutput, srDispObj.inOutputLen);
		
		if (inFunc_CheckValidOriDate(pobTran->srBRec.szCUP_TD) == VS_SUCCESS)
		{
			break;
		}	
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Date Not Valid: %s", pobTran->srBRec.szCUP_TD);
				inLogPrintf(AT, szDebugMsg);
			}
		}
		
	}
	
	vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_Func_Get_OPT_OriTransDate_Trust END!");
	
	return (VS_SUCCESS);
}