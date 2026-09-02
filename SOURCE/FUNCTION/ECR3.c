#include <string.h>
#include <stdio.h>
#include <ctosapi.h>
#include <stdlib.h>
#include <libxml/parser.h>
#include <sqlite3.h>
#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Define_2.h"
#include "../INCLUDES/TransType.h"
#include "../INCLUDES/Transaction.h"
#include "../INCLUDES/AllStruct.h"
#include "../EVENT/Menu.h"
#include "../PRINT/Print.h"
#include "../DISPLAY/Display.h"
#include "../DISPLAY/DisTouch.h"
#include "../EVENT/MenuMsg.h"
#include "../EVENT/Flow.h"
#include "Sqlite.h"
#include "Function.h"
#include "FuncTable.h"
#include "Batch.h"
#include "HDT.h"
#include "CFGT.h"
#include "EDC.h"
#include "SCDT.h"
#include "HDPT.h"
#include "CDT.h"
#include "ECR.h"
#include "RS232.h"
#include "USB.h"
#include "BaseUSB.h"
#include "KMS.h"
#include "Accum.h"
#include "File.h"
#include "Utility.h"
#include "../COMM/Ethernet.h"
#include "../COMM/WiFi.h"
#include "../../NCCC/NCCCsrc.h"
#include "../../NCCC/NCCCtmk.h"
#include "../../NCCC/NCCCTicketSrc.h"
#include "../../NCCC/NCCCTicketIso.h"
#include "../../NCCC/NCCCloyalty.h"
#include "../../NCCC/NCCCEWsrc.h"
#include "../../CTLS/CTLS.h"
#include "../../ECC/ICER/stdAfx.h"
#include "../../ECC/ECC.h"
#include "../../HG/HGiso.h"
#include "TDT.h"
#include "PIT.h"

#define max(x, y) (((x) > (y)) ? (x) : (y))
#define min(x, y) (((x) < (y)) ? (x) : (y))

extern	int			ginDebug;
extern	int			ginISODebug;
extern	int			ginDisplayDebug;
extern	int			ginECR_ResponseFd;
extern	int			ginMachineType;
extern	int			ginFindRunTime;
extern	unsigned char		guszCTLSInitiOK;
extern	NCCC_TMK_COMMAND	gsrTMKdata;
extern	ECR_TABLE		gsrECROb;
extern  EI_TABLE		gsrEIOb;
extern	BYTE			gbBarCodeECRBit;	/* 是否為掃碼交易規格 */
extern  BYTE                    gbEIECRBit;             /* 是否為電子發票交易規格 */
extern  BYTE			gbECR_UDP_TransBit;
extern	FPG_FTC_REC		gsrFPG_FTC_Rec;

/*
Function        :inECR_7E1_Customer_079_CINEMARK_Unpack
Date&Time       :2022/8/23 下午 5:26
Describe        :僅支援sale、紅利，過卡前必出選單
*/
int inECR_7E1_Customer_079_CINEMARK_Unpack(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_SUCCESS;
	int	inTransType = 0;		/* 電文中的交易別字串轉為數字儲存 */
	int	inCardInquiryTRTType = _CARD_INQUIRY_TYPE_NONE_;
	char	szTemplate[100 + 1] = {0};
	char	szDebugMsg[100 + 1] = {0};	
	char	szCTLSEnable[2 + 1] = {0};
	char	szFiscContactlessEnable[1 + 1] = {0};
	char	szCUPContactlessEnable[1 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
	
	/* (需求單 - 107276)自助交易標準做法 EDC於發動交易及完成時，需發出提示音 by Russell 2019/3/8 上午 10:46 */
	inDISP_BEEP(1, 0);
	
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
        inGetCustomIndicator(szCustomerIndicator);
	
	/* 存ECR原始資料 */
	memcpy(srECROb->srTransData.szOrgData, szDataBuffer, _ECR_7E1_Standard_Data_Size_);
	
	inECR_7E1_Standard_Parse_Data(srECROb, szDataBuffer);
	
	/* Trans Type (交易別) */
	switch (inTransType)
	{
		default:
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szTransType[0], 2);
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s: %s  ", "Trans Type", szTemplate);
				inLogPrintf(AT, szDebugMsg);
			}
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Type", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				
				return (VS_ERROR);
			}
			
			/* 交易類別轉成數字 */
			inTransType = atoi(srECROb->srTransData.szTransType);
			
			/* 要連動結帳 */
			if (inTransType == _ECR_7E1_SETTLEMENT_NO_)
			{
				pobTran->uszAutoSettleBit = VS_TRUE;
			}
			
			break;
	}
	
	/* 銀行別/付款別 HOST ID*/
	
	/* EDC簽單序號 Receipt No(InvoiceNumber) */
		
	/* 處理交易金額 (Trans Amount) */
	switch (inTransType)
	{
		case _ECR_7E1_SALE_NO_:
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &srECROb->srTransData.szField_10[0], 10);
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Amount", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				
				return (VS_ERROR);
			}
			else if ((!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_076_8_DIGITS_, _CUSTOMER_INDICATOR_SIZE_) && atol(szTemplate) > 99999999) || 
				 (memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_076_8_DIGITS_, _CUSTOMER_INDICATOR_SIZE_) && atol(szTemplate) > 9999999))
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Amount", "Out of bound");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_076_8_DIGITS_, _CUSTOMER_INDICATOR_SIZE_))
				{
					sprintf(pobTran->szErrorMsgBuff2, "金額超過8位數");
				}
				else
				{
					sprintf(pobTran->szErrorMsgBuff2, "金額超過7位數");
				}
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
				
				return (VS_ERROR);
			}
			else
			{
				pobTran->srBRec.lnTxnAmount = atol(szTemplate);
				pobTran->srBRec.lnOrgTxnAmount = atol(szTemplate);
				pobTran->srBRec.lnTotalTxnAmount = atol(szTemplate);
			}

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%ld", "交易金額", pobTran->srBRec.lnTxnAmount);
				inLogPrintf(AT, szDebugMsg);
			}
			break;
		default:
			break;
	}
	
	/* 交易時間 Trans Time */
	
	/* 授權碼(Auth Code) ECR電文中給9個字元(左靠右補空白) 但ATS電文只需要6個，所以只抓6個(交易補登及所有退貨須送此欄位) */
	
	/* szDataBuffer[66] 144規格要看紅利分期來看使用用途  */
	
	/* 分期期數 Installment Period */
	
	/* szDataBuffer[102] 144規格要看紅利分期來看使用用途 保留欄位/首期金額／紅利扣抵點數  */
	
	/* 處理櫃號(Store ID) */
	switch (inTransType)
	{
		default:
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szStoreId[0], 18);
			
			memcpy(pobTran->srBRec.szStoreID, szTemplate, strlen(szTemplate));
			/* 櫃號不滿18，補空白 */
			if (strlen(szTemplate) < 18)
			{
				memset(&pobTran->srBRec.szStoreID[strlen(szTemplate)], 0x20, 18 - strlen(szTemplate));
			}
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "Store ID", pobTran->srBRec.szStoreID);
				inLogPrintf(AT, szDebugMsg);
			}
			break;
	}
	
	/* szDataBuffer[132] 144規格要看紅利分期來看使用用途  卡號查詢/每期金額/保留欄位 */
	
	/* 若前一次為卡號查詢，標記為第二次ECR */
	if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
	{
		pobTran->uszCardInquiryFirstBit = VS_FALSE;
		pobTran->uszCardInquirysSecondBit = VS_TRUE;
		
		/* 重新決定TRT流程 */
                if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
                {
                        /* 金融卡查詢要二次過卡，不依循前次查詢界面 */
                        inCardInquiryTRTType = _CARD_INQUIRY_TYPE_NONE_;
                }
                else
                {
                        if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
                        {
                                inCardInquiryTRTType = _CARD_INQUIRY_TYPE_CHIP_;
                        }
                        else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
                        {
                                inCardInquiryTRTType = _CARD_INQUIRY_TYPE_CTLS_;
                        }
                        else
                        {
                                inCardInquiryTRTType = _CARD_INQUIRY_TYPE_MEG_;
                        }
                }
	}
	
	/* szDataBuffer[142] 144規格要看紅利分期來看使用用途  卡號查詢/每期金額/保留欄位 */
	
	if (inECR_Check_Exception(pobTran) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	memset(szFiscContactlessEnable, 0x00, sizeof(szFiscContactlessEnable));
	inGetSmartPayContactlessEnable(szFiscContactlessEnable);
	memset(szCTLSEnable, 0x00, sizeof(szCTLSEnable));
	inGetContactlessEnable(szCTLSEnable);
	memset(szCUPContactlessEnable, 0x00, sizeof(szCUPContactlessEnable));
	inGetCUPContactlessEnable(szCUPContactlessEnable);
	
	/* 跑OPT */
	switch (inTransType)
	{
		case _ECR_7E1_SALE_NO_:
			/* 金融卡和一般Sale跑不同OPT，所以要分 */
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{	
				inDISP_ClearAll();
				/* 第三層顯示 ＜消費扣款＞ */
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_FISC_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜消費扣款＞ */

				pobTran->inFunctionID = _FISC_SALE_;
				if (!memcmp(szCTLSEnable, "Y", 1)		&&
				    !memcmp(szFiscContactlessEnable, "Y", 1)	&&
				    guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_FISC_SALE_CTLS_;
					pobTran->inRunTRTID = _TRT_FISC_SALE_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_FISC_SALE_;
					pobTran->inRunTRTID = _TRT_FISC_SALE_ICC_;
				}
		
				pobTran->inTransactionCode = _FISC_SALE_;
				pobTran->srBRec.inCode = _FISC_SALE_;
				pobTran->srBRec.inOrgCode = _FISC_SALE_;
                                
                                /* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_CUP_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜銀聯一般交易＞ */

				if (!memcmp(szCTLSEnable, "Y", 1)		&&
				    !memcmp(szCUPContactlessEnable, "Y", 1)	&&
				    guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
					pobTran->inRunTRTID = _TRT_CUP_SALE_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
					pobTran->inRunTRTID = _TRT_CUP_SALE_;
				}

				pobTran->inFunctionID = _CUP_SALE_;
				pobTran->inTransactionCode = _CUP_SALE_;
				pobTran->srBRec.inCode = pobTran->inTransactionCode;
				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
                                
                                /* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_TICKET_DEDUCT_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜購貨交易＞ */

				pobTran->inFunctionID = _TICKET_DEDUCT_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_TICKET_DEDUCT_;
				}
				else
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "感應未初始化，不能使用此功能");
					}
					return (VS_ERROR);
				}

				pobTran->inTransactionCode = _TICKET_DEDUCT_;
				pobTran->srTRec.inCode = _TICKET_DEDUCT_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else
			{
				inDISP_ClearAll();
				/* 第三層顯示 ＜一般交易＞ */
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般交易＞ */

				pobTran->inFunctionID = _SALE_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
					pobTran->inRunTRTID = _TRT_SALE_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
					pobTran->inRunTRTID = _TRT_SALE_;
				}

				pobTran->inTransactionCode = _SALE_;
				pobTran->srBRec.inCode = _SALE_;
				pobTran->srBRec.inOrgCode = _SALE_;
                                
                                /* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			break;
		default:
			pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
			inRetVal = VS_ERROR;
			break;
	}
	
	return (inRetVal);
}

/*
Function        :inECR_7E1_Customer_079_CINEMARK_SelectTransType
Date&Time       :2022/8/23 下午 6:25
Describe        :Sale 交易選單
*/
int inECR_7E1_Customer_079_CINEMARK_SelectTransType(TRANSACTION_OBJECT *pobTran)
{
	int			inTransType = 0;
	int			inRetVal = VS_SUCCESS;
	int			inChoice = 0;
	int			inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_8_3X3_;
	char			szTemplate[10 + 1] = {0};
	char			szDebugMsg[100 + 1] = {0};
	char			szKey = 0x00;
	MENU_CHECK_TABLE	srMenuChekDisplay1[] =
	{
		{_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_	, _SALE_		, inNCCC_Func_Check_Transaction_Function	, _ICON_HIGHTLIGHT_ECR_OPTION_SALE_1_SALE_		},
		{_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_	, _REDEEM_SALE_		, inNCCC_Func_Check_Transaction_Function	, _ICON_HIGHTLIGHT_ECR_OPTION_SALE_2_REDEEM_SALE_	},
		{_Touch_NONE_					, _TRANS_TYPE_NULL_	, NULL						, ""							}
	};
	
	/* 若進選單，則不參考原ECR送之CUP Indicator，以選單之交易為主 */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        memcpy(szTemplate, &gsrECROb.srTransData.szTransType[0], 2);
        inTransType = atoi(szTemplate);
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "%d", inTransType);
		inLogPrintf(AT, szDebugMsg);
        }
	
	/* 燦坤流程先過卡後選擇，所以要清上面的畫面 */
	inDISP_Clear_Line(_LINE_8_1_, _LINE_8_4_);
        switch (inTransType)
        {
                case _ECR_8N1_SALE_NO_ :
                        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般交易＞ */
			inDISP_PutGraphic(_CHOOSE_HOST_2_, 0, _COORDINATE_Y_LINE_8_4_);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode("一般", _FONTSIZE_16X33_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_1_, _COORDINATE_Y_LINE_16_9_, VS_FALSE);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode("信用卡1", _FONTSIZE_16X33_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_1_, _COORDINATE_Y_LINE_16_10_, VS_FALSE);

			inDISP_ChineseFont_Point_Color_By_Graphic_Mode("紅利", _FONTSIZE_16X33_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_2_, _COORDINATE_Y_LINE_16_9_, VS_FALSE);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode("交易  2", _FONTSIZE_16X33_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_2_, _COORDINATE_Y_LINE_16_10_, VS_FALSE);
			/* 檢查功能開關，並顯示反白的圖 */
			inMENU_CHECK_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay1);
			
			inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_8_3X3_;
                        while (1)
                        {
                                inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
				szKey = uszKBD_Key();
                                
				/* 回傳Timeout */
				if (inTimerGet(_ECR_SELECT_TIMEOUT_) == VS_SUCCESS)
				{
					szKey = _KEY_TIMEOUT_;
				}
				
				/* 一般交易 */
				/* 觸控要功能有開才能算 */
                                if ((szKey == _KEY_1_ || inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_)	&& 
				   (inNCCC_Func_Check_Transaction_Function(_SALE_) == VS_SUCCESS))
				{
                                        memcpy(&gsrECROb.srTransData.szTransType[0], _ECR_8N1_SALE_, 2);
					inRetVal = VS_SUCCESS;
					break;
				}
				/* 紅利扣抵 */
                                else if ((szKey == _KEY_2_ || inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_)	&& 
					(inNCCC_Func_Check_Transaction_Function(_REDEEM_SALE_) == VS_SUCCESS))
				{
                                        memcpy(&gsrECROb.srTransData.szTransType[0], _ECR_8N1_REDEEM_, 2);
                                        
                                        pobTran->inFunctionID = _REDEEM_SALE_;
					if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
					{
						pobTran->inRunTRTID = _TRT_REDEEM_SALE_CTLS_;
					}
					else if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
					{
						pobTran->inRunTRTID = _TRT_REDEEM_SALE_ICC_;
					}
					else
					{
						pobTran->inRunTRTID = _TRT_REDEEM_SALE_;
					}
                                        pobTran->inTransactionCode = _REDEEM_SALE_;
                                        pobTran->srBRec.inCode = _REDEEM_SALE_;
                                        pobTran->srBRec.inOrgCode = _REDEEM_SALE_;
                                        pobTran->srBRec.uszRedeemBit = VS_TRUE;
					inRetVal = VS_SUCCESS;
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

                        break;
                default :
			inRetVal = VS_ERROR;
                        break;
        }
	
	return (inRetVal);
}

/*
Function        :inECR_7E1_Customer_079_CINEMARK_Pack
Date&Time       :2022/8/29 上午 11:02
Describe        :先把要送的資料組好
*/
int inECR_7E1_Customer_079_CINEMARK_Pack(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, ECR_TABLE * srECROb)
{
	int	i = 0;
	int	inRetVal = VS_SUCCESS;
	int	inPacketSizes = 0;
	int	inCardLen = 0;
	char 	szTemplate[100 + 1];
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inECR_7E1_Customer_079_CINEMARK_Pack() START !");
	}
	
	srECROb->srTransData.inSendPacketSize = 0;
	
	if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
	{
		strcpy(srECROb->srTransData.szField_05, "C");
	}
	
	/* Trans Type (2 Byte) */
	memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szTransType, 2);
	inPacketSizes += 2;

	/* HOST ID (2 Byte) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTRTFileName(szTemplate);

	if (!memcmp(szTemplate, _TRT_FILE_NAME_CREDIT_, strlen(_TRT_FILE_NAME_CREDIT_)))
		memcpy(&szDataBuffer[inPacketSizes], _ECR_7E1_NCCC_HOSTID_NCCC_, 2);
	
	inPacketSizes += 2;

	/* Receipt No (6 Byte) */
	if (pobTran->srBRec.lnOrgInvNum != 0L)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%06ld", pobTran->srBRec.lnOrgInvNum);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 6);
	}

	inPacketSizes += 6;

	/* Card No (16 Byte)(左靠右補空白) */
	/* 卡號是否遮掩 */
	inRetVal = inECR_CardNoTruncateDecision(pobTran);

	/* 要遮卡號 */
	if (inRetVal == VS_SUCCESS)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		strcpy(szTemplate, pobTran->srBRec.szPAN);

		/* HAPPG_GO 卡不掩飾 */
		if (!memcmp(&pobTran->srBRec.szPAN[0], "9552", 4))
		{

		}
		else
		{
			/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
			/* 卡號長度 */
			inCardLen = strlen(szTemplate);

			/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
			for (i = 6; i < (inCardLen - 4); i ++)
				szTemplate[i] = '*';
		}

		memcpy(&szDataBuffer[inPacketSizes], szTemplate, strlen(szTemplate));
	}
	else
	{
		memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szPAN, strlen(pobTran->srBRec.szPAN));
	}
	inPacketSizes += 16;

	/* Trans Date (6 Byte) & Trans Time (6 Byte) */
	/* Trans Date */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	strcat(szTemplate, pobTran->srBRec.szDate);
	memcpy(&szDataBuffer[inPacketSizes], &szTemplate[2], 6);
	inPacketSizes += 6;

	/* Trans Time */
	memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szTime, 6);
	inPacketSizes += 6;

	/* Approval No (6 Byte) */
	if (strlen(pobTran->srBRec.szAuthCode) > 0)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		strcpy(szTemplate, pobTran->srBRec.szAuthCode);
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 9, _PADDING_RIGHT_);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 9);
	}

	inPacketSizes += 6;
	
	/* ECR Response Code (4 Byte) */
	memcpy(&szDataBuffer[inPacketSizes], "0000", 4);
	inPacketSizes += 4;
	
	/* Terminal ID (8 Byte) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTerminalID(szTemplate);
	inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 8, _PADDING_RIGHT_);
	memcpy(&szDataBuffer[inPacketSizes], szTemplate, 8);
	inPacketSizes += 8;

	if (!memcmp(&pobTran->srBRec.szPAN[0], "54338096", 8 ) ||
	    !memcmp(&pobTran->srBRec.szPAN[0], "54338096", 8 ) ||
	    !memcmp(&pobTran->srBRec.szPAN[0], "552117901", 9 ))
	{
		szDataBuffer[inPacketSizes] = '2'; /* 京華城聯名卡 */
	}
	else
	{
		szDataBuffer[inPacketSizes] = '1'; /* 一般信用卡 */
	}

	inPacketSizes ++;
	
	srECROb->srTransData.inSendPacketSize = inPacketSizes;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inECR_7E1_Customer_079_CINEMARK_Pack(%d) END !", srECROb->srTransData.inSendPacketSize);
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inECR_7E1_Customer_079_CINEMARK_Pack_Error
Date&Time       :2022/8/29 下午 2:02
Describe        :
*/
int inECR_7E1_Customer_079_CINEMARK_Pack_Error(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, ECR_TABLE * srECROb)
{
	int	inPacketSizes = 0;
	char 	szTemplate[100 + 1];
	unsigned char	uszHostResponseBit = VS_FALSE;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inECR_7E1_Customer_079_CINEMARK_Pack_Error() START !");
	}
	
	srECROb->srTransData.inSendPacketSize = 0;
	
	/* 有Response Code 代表主機有回 */
	if (strlen(pobTran->srBRec.szRespCode) > 0	||
	    strlen(pobTran->srTRec.szRespCode) > 0)
	{
		uszHostResponseBit = VS_TRUE;
	}
	
	if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
	{
		strcpy(srECROb->srTransData.szField_05, "C");
	}
	
	/* Trans Type (2 Byte) */
	memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szTransType, 2);
	inPacketSizes += 2;

	/* HOST ID (2 Byte) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTRTFileName(szTemplate);

	if (!memcmp(szTemplate, _TRT_FILE_NAME_CREDIT_, strlen(_TRT_FILE_NAME_CREDIT_)))
		memcpy(&szDataBuffer[inPacketSizes], _ECR_7E1_NCCC_HOSTID_NCCC_, 2);
	
	inPacketSizes += 2;

	/* Receipt No (6 Byte) */
	if (pobTran->srBRec.lnOrgInvNum != 0L)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%06ld", pobTran->srBRec.lnOrgInvNum);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 6);
	}

	inPacketSizes += 6;

	/* Card No (16 Byte)(左靠右補空白) */
	memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szPAN, strlen(pobTran->srBRec.szPAN));

	inPacketSizes += 16;

	/* Trans Date (6 Byte) & Trans Time (6 Byte) */
	/* Trans Date */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	strcat(szTemplate, pobTran->srBRec.szDate);
	memcpy(&szDataBuffer[inPacketSizes], &szTemplate[2], 6);
	inPacketSizes += 6;

	/* Trans Time */
	memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szTime, 6);
	inPacketSizes += 6;

	/* Approval No (6 Byte) */
	if (uszHostResponseBit == VS_TRUE)
	{
		if (strlen(pobTran->srBRec.szAuthCode) > 0)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			strcpy(szTemplate, pobTran->srBRec.szAuthCode);
			inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 9, _PADDING_RIGHT_);
			memcpy(&szDataBuffer[inPacketSizes], szTemplate, 9);
		}
	}
	else
	{
		
	}

	inPacketSizes += 6;
	
	/* ECR Response Code (4 Byte) */
	memcpy(&szDataBuffer[inPacketSizes], "0000", 4);
	inPacketSizes += 4;
	
	/* Terminal ID (8 Byte) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTerminalID(szTemplate);
	inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 8, _PADDING_RIGHT_);
	memcpy(&szDataBuffer[inPacketSizes], szTemplate, 8);
	inPacketSizes += 8;

	if (!memcmp(&pobTran->srBRec.szPAN[0], "54338096", 8 ) ||
	    !memcmp(&pobTran->srBRec.szPAN[0], "54338096", 8 ) ||
	    !memcmp(&pobTran->srBRec.szPAN[0], "552117901", 9 ))
	{
		szDataBuffer[inPacketSizes] = '2'; /* 京華城聯名卡 */
	}
	else
	{
		szDataBuffer[inPacketSizes] = '1'; /* 一般信用卡 */
	}

	inPacketSizes ++;
	
	srECROb->srTransData.inSendPacketSize = inPacketSizes;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inECR_7E1_Customer_079_CINEMARK_Pack_Error(%d) END !", srECROb->srTransData.inSendPacketSize);
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inECR_7E1_Customer_079_CINEMARK_Pack_ResponseCode
Date&Time       :2022/8/29 下午 2:06
Describe        :
*/
int inECR_7E1_Customer_079_CINEMARK_Pack_ResponseCode(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuf)
{
	if (srECROb->srTransData.inErrorType == VS_CALLBANK	||
	    srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_CALLBANK_)
		memcpy(&szDataBuf[44], "0002", 4);
	else if (srECROb->srTransData.inErrorType == VS_TIMEOUT)
		memcpy(&szDataBuf[44], "0003", 4);
	else
	{
		memcpy(&szDataBuf[44], "0001", 4);
	}
	
	memcpy(&szDataBuf[56], "1", 1);
	szDataBuf[57] = 0x00;
	
	return (VS_SUCCESS);
}

/*
Function        :inECR_7E1_Customer_082_IKEA_Unpack
Date&Time       :2022/9/8 上午 11:57
Describe        :分析收銀機傳來的資料
 *		ikea選單
 *		交易別僅sale inst redeem refund inst-refund redeem-refund offline
 *		分期退貨和紅利退貨要檢核TID和RRN
*/
int inECR_7E1_Customer_082_IKEA_Unpack(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_SUCCESS;
	int	inTransType = 0;		/* 電文中的交易別字串轉為數字儲存 */
	char	szTemplate[100 + 1] = {0};
	char	szDebugMsg[100 + 1] = {0};	
	char	szCTLSEnable[2 + 1] = {0};
	char	szFiscContactlessEnable[1 + 1] = {0};
	char	szCUPContactlessEnable[1 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	/* (需求單 - 107276)自助交易標準做法 EDC於發動交易及完成時，需發出提示音 by Russell 2019/3/8 上午 10:46 */
	inDISP_BEEP(1, 0);
	
	/* 客製化123 需要在簽名後才回送ECR */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_))
        { 
                pobTran->uszDelaySendBit = VS_TRUE;
	}
	
	/* 存ECR原始資料 */
	memcpy(srECROb->srTransData.szOrgData, szDataBuffer, _ECR_7E1_Standard_Data_Size_);
	
	inECR_7E1_Standard_Parse_Data(srECROb, szDataBuffer);
	
	/* Trans Type (交易別) */
	switch (inTransType)
	{
		default:
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szTransType[0], 2);
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s: %s  ", "Trans Type", szTemplate);
				inLogPrintf(AT, szDebugMsg);
			}
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Type", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				sprintf(pobTran->szErrorMsgBuff1, "%02d", inTransType);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "Trans Type Not Exist Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
				return (VS_ERROR);
			}
			
			/* 交易類別轉成數字 */
			inTransType = atoi(srECROb->srTransData.szTransType);
			
			/* 要連動結帳 */
			if (inTransType == _ECR_7E1_SETTLEMENT_NO_)
			{
				pobTran->uszAutoSettleBit = VS_TRUE;
			}
			
			break;
	}
		
	/* 處理交易金額 (Trans Amount) */
	switch (inTransType)
	{
		case _ECR_7E1_SALE_NO_:
		case _ECR_7E1_OFFLINE_NO_:
		case _ECR_7E1_REFUND_NO_:
		case _ECR_7E1_INSTALLMENT_NO_:
		case _ECR_7E1_REDEEM_NO_:
		case _ECR_7E1_INSTALLMENT_REFUND_NO_:
		case _ECR_7E1_REDEEM_REFUND_NO_:
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &srECROb->srTransData.szField_10[0], 10);
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Amount", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "Trans Amount Not Exist Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
				return (VS_ERROR);
			}
			else if ((!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_076_8_DIGITS_, _CUSTOMER_INDICATOR_SIZE_) && atol(szTemplate) > 99999999) || 
				 (memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_076_8_DIGITS_, _CUSTOMER_INDICATOR_SIZE_) && atol(szTemplate) > 9999999))
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Amount", "Out of bound");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_076_8_DIGITS_, _CUSTOMER_INDICATOR_SIZE_))
				{
					sprintf(pobTran->szErrorMsgBuff2, "金額超過8位數");
				}
				else
				{
					sprintf(pobTran->szErrorMsgBuff2, "金額超過7位數");
				}
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
				
				return (VS_ERROR);
			}
			else
			{
				pobTran->srBRec.lnTxnAmount = atol(szTemplate);
				pobTran->srBRec.lnOrgTxnAmount = atol(szTemplate);
				pobTran->srBRec.lnTotalTxnAmount = atol(szTemplate);
			}

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%ld", "交易金額", pobTran->srBRec.lnTxnAmount);
				inLogPrintf(AT, szDebugMsg);
			}
			break;
		default:
			break;
	}
	
	/* 刪除收銀機連線SALE、REFUND選擇交易類別顯示畫面 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_))
	{
		
	}
	else
	{
		/* IKEA 交易選單 */
		inRetVal = inECR_SelectTransType_Customer_082_IKEA(srECROb);
	}
	/* 交易類別轉成數字 */
	inTransType = atoi(srECROb->srTransData.szTransType);

	/* Select失敗 */
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
	/* 如果是'C'代表是銀聯卡交易 */
	if (memcmp(&srECROb->srTransData.szField_05[0], "C", 1) == 0)
	{
		/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/7 上午 10:28 */
		if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
		{
			/* 註：因Üny 不支援銀聯卡，故針對 Üny 掃碼交易無論
			 *  CUP Indicator 帶什麼值請 EDC 都視為信用卡交易。*/
		}
		else
		{
			pobTran->srBRec.uszCUPTransBit = VS_TRUE;
		}
	}
	
	/* 授權碼(Auth Code) ECR電文中給9個字元(左靠右補空白) 但ATS電文只需要6個，所以只抓6個(交易補登及所有退貨須送此欄位) */
	switch (inTransType)
	{
		case _ECR_7E1_REFUND_NO_:			/* 退貨 */
		case _ECR_7E1_OFFLINE_NO_:			/* 交易補登 */
		case _ECR_7E1_INSTALLMENT_REFUND_NO_:		/* 分期退貨 */
		case _ECR_7E1_REDEEM_REFUND_NO_:		/* 紅利退貨 */
			/* 處理授權碼(optional) */
			memset(pobTran->srBRec.szAuthCode, 0x00, sizeof(pobTran->srBRec.szAuthCode));
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szApprovalNo[0], 6);
			memcpy(pobTran->srBRec.szAuthCode, szTemplate, 6);
			break;
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Auth Code", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "Auth Code Not Exist Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
				return (VS_ERROR);
			}
			else
			{
				memcpy(pobTran->srBRec.szAuthCode, szTemplate, 6);
			}
			
			break;
			
		default :
			break;
	}
	
	/* szDataBuffer[66] 144規格要看紅利分期來看使用用途  */
	switch (inTransType)
	{
		case _ECR_7E1_REDEEM_REFUND_NO_:		/* 紅利退貨 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szRDMPaidAmt[0], 10);	/* 12位數，這裡只取10位 */
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "RDM Paid Amt", szTemplate);
				inLogPrintf(AT, szDebugMsg);
			}
			/* 實際支付金額 */
			pobTran->srBRec.lnRedemptionPaidCreditAmount = atol(szTemplate);
			
			break;	
		case _ECR_7E1_INSTALLMENT_REFUND_NO_:		/* 分期退貨 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szField_28[0], 10);	/* 12位數，這裡只取10位 */

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "Formallity Fee", szTemplate);
				inLogPrintf(AT, szDebugMsg);
			}
			pobTran->srBRec.lnInstallmentFormalityFee = atol(szTemplate);

			if (pobTran->srBRec.lnInstallmentFormalityFee != 0)
			{
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_INST_FEE_NOT_0_;
				pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;		/* 操作錯誤 */
				return (VS_ERROR);
			}

			break;
		default :
			break;
	}
	
	/* 分期期數 Installment Period */
	switch (inTransType)
	{
		case _ECR_7E1_INSTALLMENT_NO_:			/* 分期 */
		case _ECR_7E1_INSTALLMENT_REFUND_NO_:		/* 分期退貨 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szInstallmentPeriod[0], 2);
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "Installment Period", szTemplate);
				inLogPrintf(AT, szDebugMsg);
			}
			/* 分期期數(右靠左補0) */
			pobTran->srBRec.lnInstallmentPeriod = atol(szTemplate);
			
			break;
			
		default :
			break;
	}
	
	/* szDataBuffer[102] 144規格要看紅利分期來看使用用途 保留欄位/首期金額／紅利扣抵點數  */
	switch (inTransType)
	{
		case _ECR_7E1_REDEEM_REFUND_NO_:		/* 紅利退貨 */
			/* 最多八位 所以從106開始 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szRDMPoint[4], 8);
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "RDM Point", szTemplate);
				inLogPrintf(AT, szDebugMsg);
			}
			
			/* 信用卡紅利扣抵點數 RDM Point */
			pobTran->srBRec.lnRedemptionPoints = atol(szTemplate);
			break;
		case _ECR_7E1_INSTALLMENT_REFUND_NO_:		/* 分期退貨 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szField_26[0], 10);	/* 12位數，這裡只取10位 */
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "Down Payment Amount", szTemplate);
				inLogPrintf(AT, szDebugMsg);
			}
			/* 首期金額 Down Payment Amount (含小數 2位) */
			pobTran->srBRec.lnInstallmentDownPayment = atol(szTemplate);
			
			break;
		default :
			break;
	}
	
	/* 處理櫃號(Store ID) */
	switch (inTransType)
	{
		default:
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szStoreId[0], 18);
			
			if (strlen(szTemplate) > 0)
			{
				memcpy(pobTran->srBRec.szStoreID, szTemplate, strlen(szTemplate));
				/* 櫃號不滿18，補空白 */
				if (strlen(szTemplate) < 18)
				{
					memset(&pobTran->srBRec.szStoreID[strlen(szTemplate)], 0x20, 18 - strlen(szTemplate));
				}

			}
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "Store ID", pobTran->srBRec.szStoreID);
				inLogPrintf(AT, szDebugMsg);
			}
			break;
	}
	
	/* szDataBuffer[132] 144規格要看紅利分期來看使用用途  卡號查詢/每期金額/保留欄位 */
	switch (inTransType)
	{
		case _ECR_7E1_INSTALLMENT_REFUND_NO_:		/* 分期退貨 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szField_27[0], 10);	/* 12位數，這裡只取10位 */
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "Payment Amount", szTemplate);
				inLogPrintf(AT, szDebugMsg);
			}
			/* 每期金額 Installment Payment Amount (含小數 2位) */
			pobTran->srBRec.lnInstallmentPayment = atol(szTemplate);
			
			break;
		default:
			break;
	}
	
	/* 分期退貨、紅利退貨未輸入TID要拒絕 & Ref No.未輸入要拒絕 */
	switch (inTransType)
	{
		case _ECR_7E1_INSTALLMENT_REFUND_NO_ :
		case _ECR_7E1_REDEEM_REFUND_NO_ :
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szTerminalID, 8);

			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ","TID", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				sprintf(pobTran->szErrorMsgBuff1, "%02d", inTransType);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "TID Not Exist Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_; 
				
				return (VS_ERROR);
			}

			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szField_35, 12);

			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ","RRN", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				sprintf(pobTran->szErrorMsgBuff1, "%02d", inTransType);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "RRN Not Exist Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_; 
				
				return (VS_ERROR);
			}

			break;
		default :
			break;
	}
	
	if (inECR_Check_Exception(pobTran) != VS_SUCCESS)
	{
		memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
		sprintf(pobTran->szErrorMsgBuff2, "ECR_Check_Exception error");
		return (VS_ERROR);
	}
	
	memset(szFiscContactlessEnable, 0x00, sizeof(szFiscContactlessEnable));
	inGetSmartPayContactlessEnable(szFiscContactlessEnable);
	memset(szCTLSEnable, 0x00, sizeof(szCTLSEnable));
	inGetContactlessEnable(szCTLSEnable);
	memset(szCUPContactlessEnable, 0x00, sizeof(szCUPContactlessEnable));
	inGetCUPContactlessEnable(szCUPContactlessEnable);
	
	/* 跑OPT */
	switch (inTransType)
	{
		case _ECR_7E1_SALE_NO_:
			/* 金融卡和一般Sale跑不同OPT，所以要分 */
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{	
				inDISP_ClearAll();
				/* 第三層顯示 ＜消費扣款＞ */
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_FISC_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜消費扣款＞ */

				pobTran->inFunctionID = _FISC_SALE_;
				if (!memcmp(szCTLSEnable, "Y", 1)		&&
				    !memcmp(szFiscContactlessEnable, "Y", 1)	&&
				    guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_FISC_SALE_CTLS_;
					pobTran->inRunTRTID = _TRT_FISC_SALE_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_FISC_SALE_;
					pobTran->inRunTRTID = _TRT_FISC_SALE_ICC_;
				}
		
				pobTran->inTransactionCode = _FISC_SALE_;
				pobTran->srBRec.inCode = _FISC_SALE_;
				pobTran->srBRec.inOrgCode = _FISC_SALE_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_CUP_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜銀聯一般交易＞ */

				if (!memcmp(szCTLSEnable, "Y", 1)		&&
				    !memcmp(szCUPContactlessEnable, "Y", 1)	&&
				    guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
					pobTran->inRunTRTID = _TRT_CUP_SALE_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
					pobTran->inRunTRTID = _TRT_CUP_SALE_;
				}

				pobTran->inFunctionID = _CUP_SALE_;
				pobTran->inTransactionCode = _CUP_SALE_;
				pobTran->srBRec.inCode = pobTran->inTransactionCode;
				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_TICKET_DEDUCT_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜購貨交易＞ */

				pobTran->inFunctionID = _TICKET_DEDUCT_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_TICKET_DEDUCT_;
				}
				else
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "感應未初始化，不能使用此功能");
					}
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "CTLS not Init");
					return (VS_ERROR);
				}

				pobTran->inTransactionCode = _TICKET_DEDUCT_;
				pobTran->srTRec.inCode = _TICKET_DEDUCT_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else
			{
				inDISP_ClearAll();
				/* 第三層顯示 ＜一般交易＞ */
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般交易＞ */

				pobTran->inFunctionID = _SALE_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
					pobTran->inRunTRTID = _TRT_SALE_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
					pobTran->inRunTRTID = _TRT_SALE_;
				}

				pobTran->inTransactionCode = _SALE_;
				pobTran->srBRec.inCode = _SALE_;
				pobTran->srBRec.inOrgCode = _SALE_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			
			break;
		case _ECR_7E1_REFUND_NO_:
			/* 金融卡和一般Sale跑不同OPT，所以要分 */
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_FISC_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);	/* 第二層顯示 ＜退費交易＞ */

				pobTran->inFunctionID = _FISC_REFUND_;                       
				if (!memcmp(szCTLSEnable, "Y", 1)		&&
				    !memcmp(szFiscContactlessEnable, "Y", 1)	&&
				    guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_FISC_REFUND_CTLS_;
					pobTran->inRunTRTID = _TRT_FISC_REFUND_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_FISC_REFUND_;
					pobTran->inRunTRTID = _TRT_FISC_REFUND_ICC_;
				}

				pobTran->inTransactionCode = _FISC_REFUND_;
				pobTran->srBRec.inCode = _FISC_REFUND_;
				pobTran->srBRec.inOrgCode = _FISC_REFUND_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			else if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般退貨＞ */

				if (!memcmp(szCTLSEnable, "Y", 1)		&&
				    !memcmp(szCUPContactlessEnable, "Y", 1)	&&
				    guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_GET_CARD_FIRST_;
					pobTran->inRunTRTID = _TRT_CUP_REFUND_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_AMOUNT_FIRST_CUP_;
					pobTran->inRunTRTID = _TRT_CUP_REFUND_;
				}

				pobTran->inFunctionID = _CUP_REFUND_;
				pobTran->inTransactionCode = _CUP_REFUND_;
				pobTran->srBRec.inCode = pobTran->inTransactionCode;
				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_TICKET_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜退貨交易＞ */

				pobTran->inFunctionID = _TICKET_REFUND_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_TICKET_REFUND_;
				}
				else
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "感應未初始化，不能使用此功能");
					}
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "CTLS not Init");
					return (VS_ERROR);
				}

				pobTran->inTransactionCode = _TICKET_REFUND_;
				pobTran->srTRec.inCode = _TICKET_REFUND_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般退貨＞ */

				pobTran->inFunctionID = _REFUND_;

				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_GET_CARD_FIRST_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_AMOUNT_FIRST_;
				}
				pobTran->inRunTRTID = _TRT_REFUND_;

				pobTran->inTransactionCode = _REFUND_;
				pobTran->srBRec.inCode = _REFUND_;
				pobTran->srBRec.inOrgCode = _REFUND_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;
			
		case _ECR_7E1_OFFLINE_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_SALE_OFFLINE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜交易補登＞ */
			
			pobTran->inFunctionID = _SALE_OFFLINE_;
			pobTran->inRunOperationID = _OPERATION_SALE_OFFLINE_;
			pobTran->inRunTRTID = _TRT_SALE_OFFLINE_;

			pobTran->inTransactionCode = _SALE_OFFLINE_;
			pobTran->srBRec.inCode = _SALE_OFFLINE_;
			pobTran->srBRec.inOrgCode = _SALE_OFFLINE_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			break;
			
		case _ECR_7E1_INSTALLMENT_NO_:
//			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
//			{
//				inDISP_ClearAll();
//				/* 保留給銀聯分期使用 */
//				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
//				inDISP_PutGraphic(_MENU_INST_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜分期付款＞ */
//
//				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
//				{
//					pobTran->inRunOperationID = _OPERATION_INST_SALE_CTLS_;
//				}
//				else
//				{
//					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
//				}
//				pobTran->inRunTRTID = _TRT_INST_SALE_;
//
//				pobTran->inFunctionID = _CUP_INST_SALE_;
//				pobTran->inTransactionCode = _CUP_INST_SALE_;
//				pobTran->srBRec.inCode = pobTran->inTransactionCode;
//				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
//
//				pobTran->srBRec.uszInstallmentBit = VS_TRUE;
//
//				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
//			}
//			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_INST_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜分期付款＞ */

				pobTran->inFunctionID = _INST_SALE_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_INST_SALE_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
				}
				pobTran->inRunTRTID = _TRT_INST_SALE_;

				pobTran->inTransactionCode = _INST_SALE_;
				pobTran->srBRec.inCode = _INST_SALE_;
				pobTran->srBRec.inOrgCode = _INST_SALE_;

				pobTran->srBRec.uszInstallmentBit = VS_TRUE;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;
			
		case _ECR_7E1_INSTALLMENT_REFUND_NO_:
//			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
//			{
//				inDISP_ClearAll();
//				/* 保留給銀聯分期退貨使用 */
//				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
//				inDISP_PutGraphic(_MENU_INST_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜分期退貨＞ */
//
//				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
//				{
//					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_GET_CARD_FIRST_;
//				}
//				else
//				{
//					pobTran->inRunOperationID = _OPERATION_REFUND_;
//				}
//				pobTran->inRunTRTID = _TRT_INST_REFUND_;
//
//				pobTran->inFunctionID = _CUP_INST_REFUND_;
//				pobTran->inTransactionCode = _CUP_INST_REFUND_;
//				pobTran->srBRec.inCode = pobTran->inTransactionCode;
//				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
//
//				pobTran->srBRec.uszInstallmentBit = VS_TRUE;
//
//				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
//			}
//			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_INST_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜分期退貨＞ */

				pobTran->inFunctionID = _INST_REFUND_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_GET_CARD_FIRST_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_;
				}
				pobTran->inRunTRTID = _TRT_INST_REFUND_;

				pobTran->inTransactionCode = _INST_REFUND_;
				pobTran->srBRec.inCode = _INST_REFUND_;
				pobTran->srBRec.inOrgCode = _INST_REFUND_;

				pobTran->srBRec.uszInstallmentBit = VS_TRUE;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;
		case _ECR_7E1_REDEEM_NO_:
//			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
//			{
//				inDISP_ClearAll();
//				/* 保留給銀聯紅利扣抵使用 */
//				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
//				inDISP_PutGraphic(_MENU_REDEEM_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利扣抵＞ */
//
//				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
//				{
//					pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
//				}
//				else
//				{
//					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
//				}
//				pobTran->inRunTRTID = _TRT_REDEEM_SALE_;
//
//				pobTran->inFunctionID = _CUP_REDEEM_SALE_;
//				pobTran->inTransactionCode = _CUP_REDEEM_SALE_;
//				pobTran->srBRec.inCode = pobTran->inTransactionCode;
//				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
//
//				pobTran->srBRec.uszRedeemBit = VS_TRUE;
//
//				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
//			}
//			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REDEEM_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利扣抵＞ */

				pobTran->inFunctionID = _REDEEM_SALE_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
				}
				pobTran->inRunTRTID = _TRT_REDEEM_SALE_;

				pobTran->inTransactionCode = _REDEEM_SALE_;
				pobTran->srBRec.inCode = _REDEEM_SALE_;
				pobTran->srBRec.inOrgCode = _REDEEM_SALE_;

				pobTran->srBRec.uszRedeemBit = VS_TRUE;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;
		
		case _ECR_7E1_REDEEM_REFUND_NO_:
//			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
//			{
//				inDISP_ClearAll();
//				/* 保留給銀聯紅利退貨使用 */
//				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
//				inDISP_PutGraphic(_MENU_REDEEM_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利退貨＞ */
//
//				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
//				{
//					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_GET_CARD_FIRST_;
//				}
//				else
//				{
//					pobTran->inRunOperationID = _OPERATION_REFUND_;
//				}
//				pobTran->inRunTRTID = _TRT_REDEEM_REFUND_;
//
//				pobTran->inFunctionID = _CUP_REDEEM_REFUND_;
//				pobTran->inTransactionCode = _CUP_REDEEM_REFUND_;
//				pobTran->srBRec.inCode = pobTran->inTransactionCode;
//				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
//
//				pobTran->srBRec.uszRedeemBit = VS_TRUE;
//
//				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
//			}
//			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REDEEM_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利退貨＞ */

				pobTran->inFunctionID = _REDEEM_REFUND_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_GET_CARD_FIRST_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_;
				}
				pobTran->inRunTRTID = _TRT_REDEEM_REFUND_;

				pobTran->inTransactionCode = _REDEEM_REFUND_;
				pobTran->srBRec.inCode = _REDEEM_REFUND_;
				pobTran->srBRec.inOrgCode = _REDEEM_REFUND_;

				pobTran->srBRec.uszRedeemBit = VS_TRUE;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;
		case _ECR_7E1_VOID_NO_:
			memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
			sprintf(pobTran->szErrorMsgBuff1, "Void");
			memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
			sprintf(pobTran->szErrorMsgBuff2, "Not Support");
			pobTran->inErrorMsg = _ERROR_CODE_V3_FUNC_NOT_SUPPORT_;
			pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
			inRetVal = VS_ERROR;
			break;
		case _ECR_7E1_SETTLEMENT_NO_:
			memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
			sprintf(pobTran->szErrorMsgBuff1, "Settle");
			memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
			sprintf(pobTran->szErrorMsgBuff2, "Not Support");
			pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
			pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
			inRetVal = VS_ERROR;
			break;
		default:
			pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
			inRetVal = VS_ERROR;
			break;
	}
	
	inECR_ReCheck_144_TRT_Flow(pobTran);
	
	return (inRetVal);
}

/*
Function        :inECR_7E1_Customer_082_IKEA_Pack
Date&Time       :2022/9/8 下午 6:01
Describe        :先把要送的資料組好
*/
int inECR_7E1_Customer_082_IKEA_Pack(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, ECR_TABLE * srECROb)
{
	int	inRetVal;
	int 	i, inCardLen;
	int	inPacketSizes = 0;
	char 	szTemplate[100 + 1];
	
	if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
	{
		strcpy(srECROb->srTransData.szField_05, "C");
	}
	
	/* Trans Type (2 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_SALE_ , 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_INSTALLMENT_ , 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REDEEM_ , 2))
	{
		memcpy(&szDataBuffer[inPacketSizes], "01", 2);
	}
	else if (!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REFUND_ , 2)		||
	    	 !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_INSTALLMENT_REFUND_ , 2)	||
	         !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REDEEM_REFUND_ , 2))
	{
		memcpy(&szDataBuffer[inPacketSizes], "02", 2);
	}
	else
	{
		memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szTransType, 2);
	}
	inPacketSizes += 2;

	/* HOST ID (2 Byte) */
	/*
		IKEA HOST ID
		01 Credit Host
		02 Redeem Host Redeem function will be excluded in this version.
		03 Instalment Host
		04 CUP Host
		11 AMEX
		12 DINERS
	*/
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTRTFileName(szTemplate);

	if (!memcmp(szTemplate, _TRT_FILE_NAME_HG_, _TRT_FILE_NAME_HG_LEN_))
		memcpy(&szDataBuffer[inPacketSizes], _ECR_7E1_NCCC_HOSTID_CUS_082_IKEA_HG_, 2);
	else if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)))
	{
		memcpy(&szDataBuffer[inPacketSizes], _ECR_7E1_NCCC_HOSTID_CUS_082_IKEA_AMEX_, 2);
	}
	else
	{
		if (!memcmp(srECROb->srTransData.szField_05, "C", 1))
			memcpy(&szDataBuffer[inPacketSizes], _ECR_7E1_NCCC_HOSTID_CUS_082_IKEA_CUP_, 2);
		else if (!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_INSTALLMENT_, 2) ||
			 !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_INSTALLMENT_, 2))
			memcpy(&szDataBuffer[inPacketSizes], _ECR_7E1_NCCC_HOSTID_CUS_082_IKEA_INST_, 2);
		else if (!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REDEEM_, 2) ||
			 !memcmp(srECROb->srTransData.szTransType,  _ECR_7E1_REDEEM_REFUND_, 2))
			memcpy(&szDataBuffer[inPacketSizes], _ECR_7E1_NCCC_HOSTID_CUS_082_IKEA_REDEEM_, 2);
		else
			memcpy(&szDataBuffer[inPacketSizes], _ECR_7E1_NCCC_HOSTID_CUS_082_IKEA_CREDIT_, 2);
	}

	inPacketSizes += 2;

	/* Receipt No (6 Byte) */
	if (pobTran->srBRec.lnOrgInvNum != 0L ||
	   !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_POINT_INQUIRY_, 2)) /* 點數查詢就算是沒有INV也要回傳 */
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%06ld", pobTran->srBRec.lnOrgInvNum);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 6);
	}

	inPacketSizes += 6;

	/* Card No (19 Byte)(左靠右補空白) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_POINT_INQUIRY_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_FULL_REDEEMPTION_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REDEEM_REFUND_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REWARD_REFUND_, 2)	||
	    pobTran->srBRec.inOrgCode == _HG_REWARD_REFUND_				||	/* 取消回饋或扣抵退貨不需回傳信用卡卡號 */
	    pobTran->srBRec.inOrgCode == _HG_REDEEM_REFUND_				||
	    pobTran->srBRec.lnHGPaymentType == _HG_PAY_CASH_				||	/* 若是純HG交易，信用卡卡號不需回傳 */
	    pobTran->srBRec.lnHGPaymentType == _HG_PAY_GIFT_PAPER_			||
	    pobTran->srBRec.inHGCode == _HG_FULL_REDEMPTION_)				/* 修改ECR交易取消點數兌換多印卡號。*/
	{
		inPacketSizes += 19;
	}
	else
	{
		/* 卡號是否遮掩 */
		inRetVal = inECR_CardNoTruncateDecision(pobTran);

		/* 要遮卡號 */
		if (inRetVal == VS_SUCCESS)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			strcpy(szTemplate, pobTran->srBRec.szPAN);

			/* HAPPG_GO 卡不掩飾 */
			if (!memcmp(&pobTran->srBRec.szPAN[0], "9552", 4))
			{

			}
			else
			{
				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
				/* 卡號長度 */
				inCardLen = strlen(szTemplate);

				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
				for (i = 6; i < (inCardLen - 4); i ++)
					szTemplate[i] = '*';
			}

			memcpy(&szDataBuffer[inPacketSizes], szTemplate, strlen(szTemplate));
		}
		else
		{
			memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szPAN, strlen(pobTran->srBRec.szPAN));
		}

		inPacketSizes += 19;
	}

	/* Card Expire Date (4 Byte) */
	/* 這裡回傳條件抄Verifone的Code */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_POINT_INQUIRY_, 2)	||	/* 點數查詢 */
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_FULL_REDEEMPTION_, 2)	||	/* 點數扣抵 */
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REDEEM_REFUND_, 2)	||	/* 扣抵退貨 */
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REWARD_REFUND_, 2)	||	/* 回饋退貨 */
	    pobTran->srBRec.inOrgCode == _HG_REWARD_REFUND_				||	/* 取消回饋或扣抵退貨不需回傳信用卡卡號 */
	    pobTran->srBRec.inOrgCode == _HG_REDEEM_REFUND_				||
	    pobTran->srBRec.lnHGPaymentType == _HG_PAY_CASH_				||	/* 若是純HG交易，信用卡卡號不需回傳 */
	    pobTran->srBRec.lnHGPaymentType == _HG_PAY_GIFT_PAPER_)
	{
		inPacketSizes += 4;
	}
	else
	{
		/* 有效期是否回傳 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetECRExpDateReturnEnable(szTemplate);
		
		/* 要回傳有效期 */
		if (!memcmp(&szTemplate, "Y", 1))
			memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szExpDate, strlen(pobTran->srBRec.szExpDate));

		inPacketSizes += 4;
	}

	/* Trans Amount (12 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_SALE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REFUND_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_OFFLINE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_PREAUTH_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_INSTALLMENT_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_INSTALLMENT_REFUND_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REDEEM_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REDEEM_REFUND_, 2))
	{
		if (pobTran->srBRec.lnTxnAmount != 0)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%010ld00", pobTran->srBRec.lnTxnAmount);
			memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
		}
	}
	inPacketSizes += 12;

	/* Trans Date (6 Byte) & Trans Time (6 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_SALE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REFUND_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_VOID_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_OFFLINE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_INSTALLMENT_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_INSTALLMENT_REFUND_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REDEEM_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REDEEM_REFUND_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_POINT_INQUIRY_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_ONLINE_REDEEMPTION_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_POINT_CERTAIN_, 2)||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_FULL_REDEEMPTION_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REDEEM_REFUND_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REWARD_REFUND_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REWARD_SALE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REWARD_INSTALLMENT_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REWARD_REDEMPTION_, 2))
	{
		/* Trans Date */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		strcat(szTemplate, pobTran->srBRec.szDate);
		memcpy(&szDataBuffer[inPacketSizes], &szTemplate[2], 6);
		inPacketSizes += 6;

		/* Trans Time */
		memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szTime, 6);
		inPacketSizes += 6;
	}
	else
	{
		inPacketSizes += 6;
		inPacketSizes += 6;
	}

	/* Approval No (9 Byte) */
	if (strlen(pobTran->srBRec.szAuthCode) > 0)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		strcpy(szTemplate, pobTran->srBRec.szAuthCode);
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 9, _PADDING_RIGHT_);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 9);
	}

	inPacketSizes += 9;
	
	/* Amount 1 (12 Byte) */
	if (pobTran->srBRec.uszRedeemBit == VS_TRUE && pobTran->srBRec.uszVOIDBit != VS_TRUE)
	{
		/* 扣抵紅利點數 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%012lu", pobTran->srBRec.lnRedemptionPoints);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
		
		inPacketSizes += 12;
	}
	else if (pobTran->srBRec.uszInstallmentBit == VS_TRUE && pobTran->srBRec.uszVOIDBit != VS_TRUE)
	{	
		/* 首期金額 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010lu00", pobTran->srBRec.lnInstallmentDownPayment);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
		
		inPacketSizes += 12;
	}
	else 
	{
		if (pobTran->srBRec.lnHGTransactionType == _HG_ONLINE_REDEEM_ && pobTran->srBRec.uszVOIDBit != VS_TRUE)
		{
			/* HG點數扣抵要回傳實際支付金額 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%012lu", pobTran->srBRec.lnHGTransactionPoint);
			memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
		}
		
		inPacketSizes += 12;
	}
	
	/* ECR Response Code (4 Byte) */
	memcpy(&szDataBuffer[inPacketSizes], "0000", 4);
	inPacketSizes += 4;
	
	/* Terminal ID (8 Byte) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTerminalID(szTemplate);
	inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 8, _PADDING_RIGHT_);
	memcpy(&szDataBuffer[inPacketSizes], szTemplate, 8);
	inPacketSizes += 8;

	/* Reference No */
	if (strlen(pobTran->srBRec.szRefNo) > 0)
	{
		memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szRefNo, 12);
	}
	inPacketSizes += 12;
	
	/* Amount 2 (12 Byte) */
	if (pobTran->srBRec.uszInstallmentBit == VS_TRUE && pobTran->srBRec.uszVOIDBit != VS_TRUE)
	{
		/* 每期金額 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010lu00", pobTran->srBRec.lnInstallmentPayment);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
	}
	inPacketSizes += 12;
	
	/* Store Id (16 Byte) */
	/* Store ID 欄位放分期期數及手續費 */
	/* 與中信規格不同聯合無其他分期交易種類 */
	/* PP000000000000CD */
	/* C = 1 General instalment D = 1 No delay payment */
	if (pobTran->srBRec.uszInstallmentBit == VS_TRUE && pobTran->srBRec.uszVOIDBit != VS_TRUE)
	{
		/* 分期期數 + 分期手續費 */
		/* Installment Period (2 Byte) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%02ld", pobTran->srBRec.lnInstallmentPeriod);
		memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 2);
		inPacketSizes += 2;
		/* Formlity Fee (12 Byte) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		if (pobTran->srBRec.lnInstallmentFormalityFee > 0)
			sprintf(szTemplate, "%010lu00", pobTran->srBRec.lnInstallmentFormalityFee);
		else
			strcpy(szTemplate, "000000000000");
		memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 12);
		inPacketSizes += 12;
		/* 分期類型固定11 */
		memcpy(&szDataBuffer[inPacketSizes], "11", 2);
		inPacketSizes += 2;
	}
	else
	{
		inPacketSizes += 16;
	}
	
	/* Amount 3 (12 Byte) */
	/* 無flexible payments */
	inPacketSizes += 12;

	/* Reserved (2 Byte) */
	inPacketSizes += 2;
	
	return (VS_SUCCESS);
}

/*
Function        :inECR_7E1_Customer_082_IKEA_Pack_Error
Date&Time       :2022/9/8 下午 6:46
Describe        :
*/
int inECR_7E1_Customer_082_IKEA_Pack_Error(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, ECR_TABLE * srECROb)
{
	int	inRetVal;
	int 	i, inCardLen;
	int	inPacketSizes = 0;
	char 	szTemplate[100 + 1];
	unsigned char	uszHostResponseBit = VS_FALSE;
	
	/* 有Response Code 代表主機有回 */
	if (strlen(pobTran->srBRec.szRespCode) > 0	||
	    strlen(pobTran->srTRec.szRespCode) > 0)
	{
		uszHostResponseBit = VS_TRUE;
	}

	if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
	{
		strcpy(srECROb->srTransData.szField_05, "C");
	}
	
	/* Trans Type (2 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_SALE_ , 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_INSTALLMENT_ , 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REDEEM_ , 2))
	{
		memcpy(&szDataBuffer[inPacketSizes], "01", 2);
	}
	else if (!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REFUND_ , 2)		||
	    	 !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_INSTALLMENT_REFUND_ , 2)	||
	         !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REDEEM_REFUND_ , 2))
	{
		memcpy(&szDataBuffer[inPacketSizes], "02", 2);
	}
	else
	{
		memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szTransType, 2);
	}
	inPacketSizes += 2;

	/* HOST ID (2 Byte) */
	/*
		IKEA HOST ID
		01 Credit Host
		02 Redeem Host Redeem function will be excluded in this version.
		03 Instalment Host
		04 CUP Host
		11 AMEX
		12 DINERS
	*/
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTRTFileName(szTemplate);

	if (!memcmp(szTemplate, _TRT_FILE_NAME_HG_, _TRT_FILE_NAME_HG_LEN_))
		memcpy(&szDataBuffer[inPacketSizes], _ECR_7E1_NCCC_HOSTID_CUS_082_IKEA_HG_, 2);
	else if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)))
	{
		memcpy(&szDataBuffer[inPacketSizes], _ECR_7E1_NCCC_HOSTID_CUS_082_IKEA_AMEX_, 2);
	}
	else
	{
		if (!memcmp(srECROb->srTransData.szField_05, "C", 1))
			memcpy(&szDataBuffer[inPacketSizes], _ECR_7E1_NCCC_HOSTID_CUS_082_IKEA_CUP_, 2);
		else if (!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_INSTALLMENT_, 2) ||
			 !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_INSTALLMENT_, 2))
			memcpy(&szDataBuffer[inPacketSizes], _ECR_7E1_NCCC_HOSTID_CUS_082_IKEA_INST_, 2);
		else if (!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REDEEM_, 2) ||
			 !memcmp(srECROb->srTransData.szTransType,  _ECR_7E1_REDEEM_REFUND_, 2))
			memcpy(&szDataBuffer[inPacketSizes], _ECR_7E1_NCCC_HOSTID_CUS_082_IKEA_REDEEM_, 2);
		else
			memcpy(&szDataBuffer[inPacketSizes], _ECR_7E1_NCCC_HOSTID_CUS_082_IKEA_CREDIT_, 2);
	}

	inPacketSizes += 2;

	/* Receipt No (6 Byte) */
	if (pobTran->srBRec.lnOrgInvNum != 0L ||
	   !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_POINT_INQUIRY_, 2)) /* 點數查詢就算是沒有INV也要回傳 */
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%06ld", pobTran->srBRec.lnOrgInvNum);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 6);
	}

	inPacketSizes += 6;

	/* Card No (19 Byte)(左靠右補空白) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_POINT_INQUIRY_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_FULL_REDEEMPTION_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REDEEM_REFUND_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REWARD_REFUND_, 2)	||
	    pobTran->srBRec.inOrgCode == _HG_REWARD_REFUND_				||	/* 取消回饋或扣抵退貨不需回傳信用卡卡號 */
	    pobTran->srBRec.inOrgCode == _HG_REDEEM_REFUND_				||
	    pobTran->srBRec.lnHGPaymentType == _HG_PAY_CASH_				||	/* 若是純HG交易，信用卡卡號不需回傳 */
	    pobTran->srBRec.lnHGPaymentType == _HG_PAY_GIFT_PAPER_			||
	    pobTran->srBRec.inHGCode == _HG_FULL_REDEMPTION_)				/* 修改ECR交易取消點數兌換多印卡號。*/
	{
		inPacketSizes += 19;
	}
	else
	{
		/* 卡號是否遮掩 */
		inRetVal = inECR_CardNoTruncateDecision(pobTran);

		/* 要遮卡號 */
		if (inRetVal == VS_SUCCESS)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			strcpy(szTemplate, pobTran->srBRec.szPAN);

			/* HAPPG_GO 卡不掩飾 */
			if (!memcmp(&pobTran->srBRec.szPAN[0], "9552", 4))
			{

			}
			else
			{
				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
				/* 卡號長度 */
				inCardLen = strlen(szTemplate);

				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
				for (i = 6; i < (inCardLen - 4); i ++)
					szTemplate[i] = '*';
			}

			memcpy(&szDataBuffer[inPacketSizes], szTemplate, strlen(szTemplate));
		}
		else
		{
			memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szPAN, strlen(pobTran->srBRec.szPAN));
		}

		inPacketSizes += 19;
	}

	/* Card Expire Date (4 Byte) */
	/* 這裡回傳條件抄Verifone的Code */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_POINT_INQUIRY_, 2)	||	/* 點數查詢 */
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_FULL_REDEEMPTION_, 2)	||	/* 點數扣抵 */
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REDEEM_REFUND_, 2)	||	/* 扣抵退貨 */
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REWARD_REFUND_, 2)	||	/* 回饋退貨 */
	    pobTran->srBRec.inOrgCode == _HG_REWARD_REFUND_				||	/* 取消回饋或扣抵退貨不需回傳信用卡卡號 */
	    pobTran->srBRec.inOrgCode == _HG_REDEEM_REFUND_				||
	    pobTran->srBRec.lnHGPaymentType == _HG_PAY_CASH_				||	/* 若是純HG交易，信用卡卡號不需回傳 */
	    pobTran->srBRec.lnHGPaymentType == _HG_PAY_GIFT_PAPER_)
	{
		inPacketSizes += 4;
	}
	else
	{
		/* 有效期是否回傳 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetECRExpDateReturnEnable(szTemplate);
		
		/* 要回傳有效期 */
		if (!memcmp(&szTemplate, "Y", 1))
			memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szExpDate, strlen(pobTran->srBRec.szExpDate));

		inPacketSizes += 4;
	}

	/* Trans Amount (12 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_SALE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REFUND_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_OFFLINE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_PREAUTH_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_INSTALLMENT_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_INSTALLMENT_REFUND_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REDEEM_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REDEEM_REFUND_, 2))
	{
		if (pobTran->srBRec.lnTxnAmount != 0)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%010ld00", pobTran->srBRec.lnTxnAmount);
			memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
		}
	}
	inPacketSizes += 12;

	/* Trans Date (6 Byte) & Trans Time (6 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_SALE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REFUND_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_VOID_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_OFFLINE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_INSTALLMENT_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_INSTALLMENT_REFUND_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REDEEM_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REDEEM_REFUND_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_POINT_INQUIRY_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_ONLINE_REDEEMPTION_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_POINT_CERTAIN_, 2)||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_FULL_REDEEMPTION_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REDEEM_REFUND_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REWARD_REFUND_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REWARD_SALE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REWARD_INSTALLMENT_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REWARD_REDEMPTION_, 2))
	{
		/* Trans Date */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		strcat(szTemplate, pobTran->srBRec.szDate);
		memcpy(&szDataBuffer[inPacketSizes], &szTemplate[2], 6);
		inPacketSizes += 6;

		/* Trans Time */
		memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szTime, 6);
		inPacketSizes += 6;
	}
	else
	{
		inPacketSizes += 6;
		inPacketSizes += 6;
	}

	if (uszHostResponseBit == VS_TRUE)
	{
		/* Approval No (9 Byte) */
		if (strlen(pobTran->srBRec.szAuthCode) > 0)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			strcpy(szTemplate, pobTran->srBRec.szAuthCode);
			inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 9, _PADDING_RIGHT_);
			memcpy(&szDataBuffer[inPacketSizes], szTemplate, 9);
		}
	}

	inPacketSizes += 9;
	
	/* Amount 1 (12 Byte) */
	if (pobTran->srBRec.uszRedeemBit == VS_TRUE && pobTran->srBRec.uszVOIDBit != VS_TRUE)
	{
		/* 扣抵紅利點數 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%012lu", pobTran->srBRec.lnRedemptionPoints);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
		
		inPacketSizes += 12;
	}
	else if (pobTran->srBRec.uszInstallmentBit == VS_TRUE && pobTran->srBRec.uszVOIDBit != VS_TRUE)
	{	
		/* 首期金額 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010lu00", pobTran->srBRec.lnInstallmentDownPayment);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
		
		inPacketSizes += 12;
	}
	else 
	{
		if (pobTran->srBRec.lnHGTransactionType == _HG_ONLINE_REDEEM_ && pobTran->srBRec.uszVOIDBit != VS_TRUE)
		{
			/* HG點數扣抵要回傳實際支付金額 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%012lu", pobTran->srBRec.lnHGTransactionPoint);
			memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
		}
		
		inPacketSizes += 12;
	}
	
	/* ECR Response Code (4 Byte) */
	memcpy(&szDataBuffer[inPacketSizes], "0000", 4);
	inPacketSizes += 4;
	
	/* Terminal ID (8 Byte) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTerminalID(szTemplate);
	inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 8, _PADDING_RIGHT_);
	memcpy(&szDataBuffer[inPacketSizes], szTemplate, 8);
	inPacketSizes += 8;

	/* Reference No */
	if (strlen(pobTran->srBRec.szRefNo) > 0)
	{
		memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szRefNo, 12);
	}
	inPacketSizes += 12;
	
	/* Amount 2 (12 Byte) */
	if (pobTran->srBRec.uszInstallmentBit == VS_TRUE && pobTran->srBRec.uszVOIDBit != VS_TRUE)
	{
		/* 每期金額 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010lu00", pobTran->srBRec.lnInstallmentPayment);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
	}
	inPacketSizes += 12;
	
	/* Store Id (16 Byte) */
	/* Store ID 欄位放分期期數及手續費 */
	/* 與中信規格不同聯合無其他分期交易種類 */
	/* PP000000000000CD */
	/* C = 1 General instalment D = 1 No delay payment */
	if (pobTran->srBRec.uszInstallmentBit == VS_TRUE && pobTran->srBRec.uszVOIDBit != VS_TRUE)
	{
		/* 分期期數 + 分期手續費 */
		/* Installment Period (2 Byte) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%02ld", pobTran->srBRec.lnInstallmentPeriod);
		memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 2);
		inPacketSizes += 2;
		/* Formlity Fee (12 Byte) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		if (pobTran->srBRec.lnInstallmentFormalityFee > 0)
			sprintf(szTemplate, "%010lu00", pobTran->srBRec.lnInstallmentFormalityFee);
		else
			strcpy(szTemplate, "000000000000");
		memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 12);
		inPacketSizes += 12;
		/* 分期類型固定11 */
		memcpy(&szDataBuffer[inPacketSizes], "11", 2);
		inPacketSizes += 2;
	}
	else
	{
		inPacketSizes += 16;
	}
	
	/* Amount 3 (12 Byte) */
	/* 無flexible payments */
	inPacketSizes += 12;

	/* Reserved (2 Byte) */
	inPacketSizes += 2;
	
	return (VS_SUCCESS);
}

/*
Function        :inECR_7E1_Customer_082_IKEA_Pack_ResponseCode
Date&Time       :2022/9/8 下午 6:48
Describe        :
*/
int inECR_7E1_Customer_082_IKEA_Pack_ResponseCode(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuf)
{
	if (srECROb->srTransData.inErrorType == VS_CALLBANK	||
	    srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_CALLBANK_)
		memcpy(&szDataBuf[78], "0002", 4);
	else if (srECROb->srTransData.inErrorType == VS_TIMEOUT)
		memcpy(&szDataBuf[78], "0003", 4);
	else
	{
		memcpy(&szDataBuf[78], "0001", 4);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inECR_SelectTransType_Customer_082_IKEA_
Date&Time       :2022/10/21 下午 5:57
Describe        :Indicator為'S'時的交易選單
*/
int inECR_SelectTransType_Customer_082_IKEA(ECR_TABLE * srECROb)
{
	int			inTransType = 0;
	int			inRetVal = VS_SUCCESS;
	int			inChoice = 0;
	int			inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_8_3X3_;
	char			szTemplate[10 + 1];
	char			szDebugMsg[100 + 1];
	char			szKey = 0x00;
	BYTE			bUnyBit = VS_FALSE;
	MENU_CHECK_2_TABLE_	srMenuChekDisplay1[] =
	{
                {_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_	, _SALE_	, _MENU_CHECK_FORCE_DISPLAY_NONE_	, inNCCC_Func_Check_Transaction_Function, 
		"一般",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_1_, _COORDINATE_Y_LINE_24_14_, VS_FALSE, 
		"交易  1",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_1_, _COORDINATE_Y_LINE_24_15_, VS_FALSE},
		{_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_	, _INST_SALE_	, _MENU_CHECK_FORCE_DISPLAY_NONE_	, inNCCC_Func_Check_Transaction_Function, 
		"分期",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_2_, _COORDINATE_Y_LINE_24_14_, VS_FALSE, 
		"付款  2",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_2_, _COORDINATE_Y_LINE_24_15_, VS_FALSE},
		{_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_6_	, _REDEEM_SALE_	, _MENU_CHECK_FORCE_DISPLAY_NONE_	, inNCCC_Func_Check_Transaction_Function, 
		"紅利",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_3_, _COORDINATE_Y_LINE_24_14_, VS_FALSE, 
		"扣抵  3",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_3_, _COORDINATE_Y_LINE_24_15_, VS_FALSE},
		{_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_7_	, _CUP_SALE_	, _MENU_CHECK_FORCE_DISPLAY_NONE_	, inNCCC_Func_Check_Transaction_Function, 
		"銀聯一",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_1_, _COORDINATE_Y_LINE_24_20_, VS_FALSE, 
		"般交易4",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_1_, _COORDINATE_Y_LINE_24_21_, VS_FALSE},
		{_Touch_NONE_}
	};
	MENU_CHECK_2_TABLE_	srMenuChekDisplay3[] =
	{
                {_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_	, _REFUND_		, _MENU_CHECK_FORCE_DISPLAY_NONE_	, inNCCC_Func_Check_Transaction_Function, 
		"一般",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_1_, _COORDINATE_Y_LINE_24_14_, VS_FALSE, 
		"退貨  1",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_1_, _COORDINATE_Y_LINE_24_15_, VS_FALSE},
		{_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_	, _INST_REFUND_		, _MENU_CHECK_FORCE_DISPLAY_NONE_	, inNCCC_Func_Check_Transaction_Function, 
		"分期",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_2_, _COORDINATE_Y_LINE_24_14_, VS_FALSE, 
		"退貨  2",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_2_, _COORDINATE_Y_LINE_24_15_, VS_FALSE},
		{_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_6_	, _REDEEM_REFUND_	, _MENU_CHECK_FORCE_DISPLAY_NONE_	, inNCCC_Func_Check_Transaction_Function, 
		"紅利",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_3_, _COORDINATE_Y_LINE_24_14_, VS_FALSE, 
		"退貨  3",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_3_, _COORDINATE_Y_LINE_24_15_, VS_FALSE},
		{_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_7_	, _CUP_REFUND_		, _MENU_CHECK_FORCE_DISPLAY_NONE_	, inNCCC_Func_Check_Transaction_Function, 
		"銀聯",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_1_, _COORDINATE_Y_LINE_24_20_, VS_FALSE, 
		"退貨  4",  _FONTSIZE_24X33_, _COORDINATE_X_MENU_1_, _COORDINATE_Y_LINE_24_21_, VS_FALSE},
		{_Touch_NONE_}
	};
	
	/* 若進選單，則不參考原ECR送之CUP Indicator，以選單之交易為主 */
	memset(&srECROb->srTransData.szField_05[0], 0x00, sizeof(srECROb->srTransData.szField_05[0]));	
        memset(szTemplate, 0x00, sizeof(szTemplate));
        memcpy(szTemplate, &srECROb->srTransData.szTransType[0], 2);
        inTransType = atoi(szTemplate);
	
	if (inNCCC_Func_Check_Uny_Format(srECROb->srTransData.szBarCodeData) == VS_TRUE)
	{
		bUnyBit = VS_TRUE;
	}
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "%d", inTransType);
		inLogPrintf(AT, szDebugMsg);
        }
	
        switch (inTransType)
        {
                case _ECR_8N1_SALE_NO_ :
                        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般交易＞ */
			/* 檢查功能開關，並顯示反白的圖 */
			if (bUnyBit == VS_TRUE)
			{
				srMenuChekDisplay1[3].inForceDisplay = _MENU_CHECK_FORCE_DISPLAY_OFF_;
			}
			inMENU_CHECK_2_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay1);
			
			
			inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_8_3X3_;
                        while (1)
                        {
                                inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
				szKey = uszKBD_Key();
                                
				/* 回傳Timeout */
				if (inTimerGet(_ECR_RS232_RECEIVE_TIMER_) == VS_SUCCESS)
				{
					szKey = _KEY_TIMEOUT_;
				}
				
				/* 一般交易 */
				/* 觸控要功能有開才能算 */
                                if ((szKey == _KEY_1_ || inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_)	&& 
				   (inNCCC_Func_Check_Transaction_Function(_SALE_) == VS_SUCCESS))
				{
                                        memcpy(&srECROb->srTransData.szTransType[0], _ECR_7E1_SALE_, 2);
					memcpy(&srECROb->srTransData.szField_05[0], "N", strlen("N"));
					inRetVal = VS_SUCCESS;
					break;
				}
				/* 分期付款 */
                                else if ((szKey == _KEY_2_ || inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_)	&& 
					(inNCCC_Func_Check_Transaction_Function(_INST_SALE_) == VS_SUCCESS))
				{
					memcpy(&srECROb->srTransData.szTransType[0], _ECR_7E1_INSTALLMENT_, 2);
					memcpy(&srECROb->srTransData.szField_05[0], "N", strlen("N"));
					inRetVal = VS_SUCCESS;
					break;
				}
				/* 紅利扣抵 */
                                else if ((szKey == _KEY_3_ || inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_6_)	&& 
					(inNCCC_Func_Check_Transaction_Function(_REDEEM_SALE_) == VS_SUCCESS))
				{
                                        memcpy(&srECROb->srTransData.szTransType[0],_ECR_7E1_REDEEM_, 2);
					memcpy(&srECROb->srTransData.szField_05[0], "N", strlen("N"));
					inRetVal = VS_SUCCESS;
					break;
				}
				else if ((szKey == _KEY_4_ || inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_7_)	&& 
					(inNCCC_Func_Check_Transaction_Function(_CUP_SALE_) == VS_SUCCESS && bUnyBit != VS_TRUE))
				{
                                        memcpy(&srECROb->srTransData.szTransType[0], _ECR_7E1_SALE_, 2);
					memcpy(&srECROb->srTransData.szField_05[0], "C", strlen("C"));
					inRetVal = VS_SUCCESS;
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

                        break;
                case _ECR_8N1_OFFLINE_NO_ :
			memcpy(&srECROb->srTransData.szTransType[0], _ECR_8N1_OFFLINE_, 2);
			memcpy(&srECROb->srTransData.szField_05[0], "N", strlen("N"));
			inRetVal = VS_SUCCESS;

                        break;
                case _ECR_8N1_REFUND_NO_ :
                        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜一般退貨＞ */
			/* 檢查功能開關，並顯示反白的圖 */
			if (bUnyBit == VS_TRUE)
			{
				srMenuChekDisplay3[3].inForceDisplay = _MENU_CHECK_FORCE_DISPLAY_OFF_;
			}
			inMENU_CHECK_2_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay3);

			inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_8_3X3_;
                        while (1)
                        {
                                inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
				szKey = uszKBD_Key();
                                
				/* 回傳Timeout */
				if (inTimerGet(_ECR_RS232_RECEIVE_TIMER_) == VS_SUCCESS)
				{
					szKey = _KEY_TIMEOUT_;
				}
				
				/* 一般退貨 */
                                if (szKey == _KEY_1_			||
				   (inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_	&& (inNCCC_Func_Check_Transaction_Function(_REFUND_) == VS_SUCCESS)))
				{
                                        memcpy(&srECROb->srTransData.szTransType[0], _ECR_7E1_REFUND_, 2);
					memcpy(&srECROb->srTransData.szField_05[0], "N", strlen("N"));
					inRetVal = VS_SUCCESS;
					break;
                                }
				/* 分期退貨 */
                                else if (szKey == _KEY_2_			||
					(inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_	&& (inNCCC_Func_Check_Transaction_Function(_INST_REFUND_) == VS_SUCCESS)))
				{
					memcpy(&srECROb->srTransData.szTransType[0], _ECR_7E1_INSTALLMENT_REFUND_, 2);
					memcpy(&srECROb->srTransData.szField_05[0], "N", strlen("N"));
					inRetVal = VS_SUCCESS;
					break;
                                }
				/* 紅利退貨 */
                                else if (szKey == _KEY_3_			||
					((inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_6_)	&& (inNCCC_Func_Check_Transaction_Function(_REDEEM_REFUND_) == VS_SUCCESS)))
				{
					memcpy(&srECROb->srTransData.szTransType[0], _ECR_7E1_REDEEM_REFUND_, 2);
					memcpy(&srECROb->srTransData.szField_05[0], "N", strlen("N"));
					inRetVal = VS_SUCCESS;
					break;
                                }
				else if ((szKey == _KEY_4_ || inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_7_)	&& 
					(inNCCC_Func_Check_Transaction_Function(_CUP_REFUND_) == VS_SUCCESS && bUnyBit != VS_TRUE))
				{
                                        memcpy(&srECROb->srTransData.szTransType[0], _ECR_8N1_REFUND_, 2);
					memcpy(&srECROb->srTransData.szField_05[0], "C", strlen("C"));
					inRetVal = VS_SUCCESS;
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
			
                        break;

                default :
			inRetVal = VS_SUCCESS;
                        break;
        }
	
	return (inRetVal);
}

/*
Function        :inECR_8N1_Customer_038_Bellavita_Unpack
Date&Time       :2022/9/13 下午 6:27
Describe        :基本上和標準400相同，但不接受兩段式
*/
int inECR_8N1_Customer_038_Bellavita_Unpack(TRANSACTION_OBJECT *pobTran, ECR_TABLE* srECROb, char *szDataBuffer)
{
	int	i = 0;
	int	inRetVal = VS_SUCCESS;
	int	inTransType = 0;		/* 電文中的交易別字串轉為數字儲存 */
	int	inCardInquiryTRTType = _CARD_INQUIRY_TYPE_NONE_;
	int	inBarCodeLen = 0;		/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/7 上午 10:28 */
	int	inSize = 0;
	int	inTempTransType = 0;		/* 用來處理啟動卡號查詢流程前面就需要判斷的狀況 */
	char	szTemplate[100 + 1] = {0};
	char	szDebugMsg[100 + 1] = {0};	
	char	szHostLabel[8 + 1] = {0};
	char	szHostEnable[2 + 1] = {0};
	char	szBatchNum[6 + 1] = {0};
	char	szCTLSEnable[2 + 1] = {0};
	char	szPayItemFuncEnable[2 + 1] = {0};
	char	szPayItemCodeTemp[5 + 1] = {0};
	char	szTempSendData[_ECR_BUFF_SIZE_ + 1] = {0};
        char	szCustomerIndicator[3 + 1] = {0};
	char	szFiscContactlessEnable[1 + 1] = {0};
	char	szCUPContactlessEnable[1 + 1] = {0};
	
        /* (需求單 - 107276)自助交易標準做法 EDC於發動交易及完成時，需發出提示音 by Russell 2019/3/8 上午 10:46 */
        inDISP_BEEP(1, 0);
                
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
        inGetCustomIndicator(szCustomerIndicator);
	
	/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/10/12 下午 5:55 */
	if (gbBarCodeECRBit == VS_TRUE)
	{
		inSize = _ECR_8N1_1000_CUPQRCODE_Size_;
	}
	else
	{
		inSize = _ECR_8N1_Standard_Data_Size_;
	}
	
	/* 存ECR原始資料 */
	memcpy(srECROb->srTransData.szOrgData, szDataBuffer, inSize);
	
	inECR_8N1_Standard_Parse_Data(srECROb, szDataBuffer);
	
	/* ECR Indicator :新 ECR連線 Indicator"I""E"(規格新增欄位 )。 */
	/* 【需求單 - 108046】電子發票BIN大於6碼需求 標準400收銀機欄位ECR Indicator新增”E”值 by Russell 2019/7/5 下午 5:55 */
	switch (inTransType)
	{
		default:
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s: %s  ","ECR Indicator", srECROb->srTransData.szECRIndicator);
				inLogPrintf(AT, szDebugMsg);
			}
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(srECROb->srTransData.szECRIndicator) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ","ECR Indicator", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "ECR Indicator Not Exist Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
				return (VS_ERROR);
			}
			
			/* ECR Indicator必為'I' or 'E'，否則error */
			/* 【需求單 - 108046】電子發票BIN大於6碼需求 標準400收銀機欄位ECR Indicator新增”E”值 by Russell 2019/7/5 下午 5:55 */
			/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/8/13 下午 4:10 */
			if (memcmp(&srECROb->srTransData.szECRIndicator[0], "I", 1) != 0	&&
			    memcmp(&srECROb->srTransData.szECRIndicator[0], "E", 1) != 0	&&
			    memcmp(&srECROb->srTransData.szECRIndicator[0], "Q", 1) != 0)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ","ECR Indicator", "Not \"I\",\"E\",\"Q\" Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "ECR Indicator Not \"I\",\"E\",\"Q\" Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
				return (VS_ERROR);
			}
			break;
	}
	
	/* Trans Type Indicator :支援交易別 :一般交易、補登退貨 ，其餘 交易別，端末機不參考此欄位 。*/
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "%s: %s  ", "Trans Type Indicator", srECROb->srTransData.szTransTypeIndicator);
		inLogPrintf(AT, szDebugMsg);
	}
				
	/* Trans Type (交易別) */
	if (gbBarCodeECRBit == VS_TRUE)
	{
		switch (inTransType)
		{
			default:
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, srECROb->srTransData.szTransType, 2);

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Type", szTemplate);
					inLogPrintf(AT, szDebugMsg);
				}

				/* 欄位為M 卻不存在，回傳錯誤 */
				if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s: %s  ", "Trans Type", "Not Exist Error");
						inLogPrintf(AT, szDebugMsg);
					}
					memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
					inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "Trans Type Not Exist Error");
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

					return (VS_ERROR);
				}

				/* 交易類別轉成數字 */
				inTransType = atoi(szTemplate);

				if (inTransType == _ECR_8N1_SETTLEMENT_NO_)
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s%s  ", "掃碼格式", "不支援結帳");
						inLogPrintf(AT, szDebugMsg);
					}
					memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
					sprintf(pobTran->szErrorMsgBuff1, "掃碼格式");
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "不支援結帳");
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
					
					return (VS_ERROR);
				}
				break;
		}
	}
	else
	{
		switch (inTransType)
		{
			default:			
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, srECROb->srTransData.szTransType, 2);

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Type", szTemplate);
					inLogPrintf(AT, szDebugMsg);
				}

				/* 欄位為M 卻不存在，回傳錯誤 */
				if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s: %s  ", "Trans Type", "Not Exist Error");
						inLogPrintf(AT, szDebugMsg);
					}
					memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
					inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "Trans Type Not Exist Error");
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

					return (VS_ERROR);
				}

				/* 交易類別轉成數字 */
				inTransType = atoi(szTemplate);

				/* 要連動結帳 */
				if (inTransType == _ECR_8N1_SETTLEMENT_NO_)
				{
					pobTran->uszAutoSettleBit = VS_TRUE;
				}

				break;
		}
	}
	
	/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/7 上午 10:28 */
	/* 掃碼交易規格 */
	if (gbBarCodeECRBit == VS_TRUE)
	{
		/* inTempTransType目前只用在前面判斷 */
		if (inTransType == _ECR_8N1_START_CARD_NO_INQUIRY_NO_ ||
		    inTransType == _ECR_8N1_INQUIRY_LAST_TRANSACTION_NO_)
		{
			inTempTransType = atoi(srECROb->srTransData.szStartTransType);
		}
		else
		{
			inTempTransType = inTransType;
		}
		/* 確認是哪一種交易 */
		switch (inTempTransType)
		{
			case _ECR_8N1_END_CARD_NO_INQUIRY_NO_ :
			case _ECR_8N1_SALE_NO_:
			case _ECR_8N1_INSTALLMENT_NO_:
			case _ECR_8N1_REDEEM_NO_:
				if (inNCCC_Func_Check_Uny_Format(srECROb->srTransData.szBarCodeData) == VS_TRUE)
				{
					pobTran->srBRec.uszUnyTransBit = VS_TRUE;
					/* Uny交易一定免簽 */
					pobTran->srBRec.uszNoSignatureBit = VS_TRUE;
				}
				else
				{
					/* 接收資料錯誤 */
					memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
					inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "BarCode Data Error");
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

					return (VS_ERROR);
				}
				break;
			case _ECR_8N1_REFUND_NO_:
			case _ECR_8N1_INSTALLMENT_REFUND_NO_:
			case _ECR_8N1_REDEEM_REFUND_NO_:
				if (strlen(srECROb->srTransData.szUnyTransCode) > 0)
				{
					pobTran->srBRec.uszUnyTransBit = VS_TRUE;
					/* Uny交易一定免簽 */
					pobTran->srBRec.uszNoSignatureBit = VS_TRUE;
				}
				else
				{
					/* 接收資料錯誤 */
					memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
					inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "No Üny TransCode");
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

					return (VS_ERROR);
				}
				break;
			case _ECR_8N1_VOID_NO_:
				/* 直接撈原交易出來看是否是Uny交易 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, srECROb->srTransData.szHostID, 2);
				
				memset(szHostLabel, 0x00, sizeof(szHostLabel));
				if (memcmp(szTemplate, _ECR_8N1_NCCC_HOSTID_NCCC_, 2) == 0)
				{
					memcpy(szHostLabel, _HOST_NAME_CREDIT_NCCC_, strlen(_HOST_NAME_CREDIT_NCCC_));
				}
				else if (memcmp(szTemplate, _ECR_8N1_NCCC_HOSTID_DCC_, 2) == 0)
				{
					memcpy(szHostLabel, _HOST_NAME_DCC_, strlen(_HOST_NAME_DCC_));
				}
				/* 2018/5/8 下午 2:10 DFS需求不再使用大來主機 */
				else if (memcmp(szTemplate, _ECR_8N1_NCCC_HOSTID_LOYALTY_REDEEM_, 2) == 0)
				{
					memcpy(szHostLabel, _HOST_NAME_CREDIT_NCCC_, strlen(_HOST_NAME_CREDIT_NCCC_));
				}
				else if (memcmp(szTemplate, _ECR_8N1_NCCC_HOSTID_HG_, 2) == 0)
				{
					memcpy(szHostLabel, _HOST_NAME_HG_, strlen(_HOST_NAME_HG_));
				}
				else
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "無對應Host ID");
					}
					memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
					inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "%s : HOST ID Not Pair", szTemplate);
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

					return (VS_ERROR);
				}

				/* 初始化 */
				i = 0;
				do
				{
					/* 按順序load每一個HOST */
					if (inLoadHDTRec(i) < 0)
					{
						/* 如果return VS_ERROR 代表table沒有該HOST */
						memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
						inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
						memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
						sprintf(pobTran->szErrorMsgBuff2, "%s : HOST ID Not Found", szHostLabel);
						pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

						return (VS_ERROR);
					}

					memset(szTemplate, 0x00, sizeof(szTemplate));
					inGetHostLabel(szTemplate);
					if (!memcmp(szTemplate, szHostLabel, strlen(szHostLabel)))
					{
						/* 比對成功後，判斷HOST是否開啟  */
						memset(szHostEnable, 0x00, sizeof(szHostEnable));
						inGetHostEnable(szHostEnable);
						if (szHostEnable[0] != 'Y')
						{
							/* Host沒開 */
							if (ginDebug == VS_TRUE) 
							{
								memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
								sprintf(szDebugMsg, "%s Not Open Error", szHostLabel);
								inLogPrintf(AT, szDebugMsg);
							}

							memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
							inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
							memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
							sprintf(pobTran->szErrorMsgBuff2, "%s : HOST ID Not Open", szHostLabel);
							pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

							/* 如果return VS_ERROR 代表Host沒開 */
							return (VS_ERROR);
						}
						else
						{
							/* Host有開 */
							pobTran->srBRec.inHDTIndex = i;

							/* Load HDPT */
							inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
							memset(szBatchNum, 0x00, sizeof(szBatchNum));
							inGetBatchNum(szBatchNum);
							pobTran->srBRec.lnBatchNum = atol(szBatchNum);

							if (ginDebug == VS_TRUE)
							{
								memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
								sprintf(szDebugMsg, "%s Open", szHostLabel);
								inLogPrintf(AT, szDebugMsg);
							}

						}/* Host Enable比對End */

						break;
					}/* Host Label比對 End */
					i++;

				} while (1);
				
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], srECROb->srTransData.szReceiptNo, 6);
				
				pobTran->srBRec.lnOrgInvNum = atol(szTemplate);
				inBATCH_GetTransRecord_By_Sqlite(pobTran);
				
				break;
			default:
				break;
		}
	}
	
	/* (需求單 - 107227)邦柏科技自助作業客製化 查詢上一筆機制 by Russell 2018/12/8 下午 2:18 */
	if (inTransType == _ECR_8N1_INQUIRY_LAST_TRANSACTION_NO_)
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_REVIEW_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第一層顯示 <交易查詢> */
		
		/* 複製一份Send Data進去比對 */
		memset(szTempSendData, 0x00, sizeof(szTempSendData));
		memcpy(szTempSendData, szDataBuffer, inSize);
		
		inRetVal = inECR_Inquiry_Last_Transction(pobTran, srECROb, szTempSendData, inSize);

		/* 如果比對失敗，回操作錯誤 */
		if (inRetVal == VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "查詢上一筆流程");
			}
			pobTran->uszLastTranscationBit = VS_TRUE;
		}
		else if (inRetVal == VS_TAP_AGAIN)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "TAP AGAIN流程");
			}
			/* 0018 重新感應用成之前的ECR電文 */
			/* 主要是前面的交易類別：查詢上一筆要轉換回原交易別，並用pobTran->uszLastTranscationBit來識別悠遊卡感應要用舊資料，
			    OPT跑特殊流程，TRT則跑正常流程 */
			sprintf(szDataBuffer, szTempSendData);
			memset(srECROb->srTransData.szTransType, 0x00, sizeof(srECROb->srTransData.szTransType));
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &szDataBuffer[8], 2);
			memcpy(srECROb->srTransData.szTransType, szTemplate, 2);
			/* 交易類別轉成數字 */
			inTransType = atoi(srECROb->srTransData.szTransType);
			
			pobTran->uszLastTranscationBit = VS_TRUE;
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "比對失敗");
			}
			/* 比對失敗，要回操作錯誤 */
			memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
			inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
			memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
			sprintf(pobTran->szErrorMsgBuff2, "查詢上一筆比對失敗");
			pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
			return (VS_ERROR);
		}
	}
	else
	{
		/* 要另外存ECR Request電文 提供查詢上一筆時檢核用 */
		inECR_Save_Request(szDataBuffer, inSize);

		/* 不是查詢上一筆的話 移除Retry備份REQ */
		if (!memcmp(&srECROb->srTransData.szField_05[0], "E", 1))
		{
			inFile_Unlink_File(_ECC_API_REQ_RETRY_FILE_, _ECC_FOLDER_PATH_);
		}
	}
	
	/* CUP Indicator */
	/* 若前一次為卡號查詢，標記為第二次ECR */
	if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
	{
		pobTran->uszCardInquiryFirstBit = VS_FALSE;
		pobTran->uszCardInquirysSecondBit = VS_TRUE;
		/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/28 下午 2:42 */
		/* 避免兩段式帶上卡號查詢的交易碼 */
		if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
		{
			memset(pobTran->srBRec.szUnyTransCode, 0x00, sizeof(pobTran->srBRec.szUnyTransCode));
			memset(pobTran->srBRec.szUnyMaskedCardNo, 0x00, sizeof(pobTran->srBRec.szUnyMaskedCardNo));
			memset(pobTran->srBRec.szCheckNO, 0x00, sizeof(pobTran->srBRec.szCheckNO));
			memset(pobTran->srBRec.szEInvoiceHASH, 0x00, sizeof(pobTran->srBRec.szEInvoiceHASH));
			memset(pobTran->srBRec.szUnyCardLabelLen, 0x00, sizeof(pobTran->srBRec.szUnyCardLabelLen));
			memset(pobTran->srBRec.szUnyCardLabel, 0x00, sizeof(pobTran->srBRec.szUnyCardLabel));
		}
		
		/* 重新決定TRT流程 */
                if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
                {
                        /* 金融卡查詢要二次過卡，不依循前次查詢界面 */
                        inCardInquiryTRTType = _CARD_INQUIRY_TYPE_NONE_;
                }
                else
                {
                        if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
                        {
                                inCardInquiryTRTType = _CARD_INQUIRY_TYPE_CHIP_;
                        }
                        else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
                        {
                                inCardInquiryTRTType = _CARD_INQUIRY_TYPE_CTLS_;
                        }
                        else
                        {
                                inCardInquiryTRTType = _CARD_INQUIRY_TYPE_MEG_;
                        }
                }
	}
	

	/* Trans Type Indicator :支援交易別 :一般交易、補登退貨 ，其餘 交易別，端末機不參考此欄位 。*/
	switch (inTransType)
	{
		case _ECR_8N1_SALE_NO_:				/* 一般交易 */
		case _ECR_8N1_REFUND_NO_:			/* 退貨交易 */
		case _ECR_8N1_OFFLINE_NO_:			/* 交易補登 */
			/* Trans Type Indicator是'S'，要進選擇畫面 */
			if (!memcmp(&srECROb->srTransData.szTransTypeIndicator[0], "S", 1))
			{
				inRetVal = inECR_SelectTransType(srECROb);
				/* 交易類別轉成數字 */
				inTransType = atoi(srECROb->srTransData.szTransType);

				/* Select失敗 */
				if (inRetVal != VS_SUCCESS)
				{
					return (inRetVal);
				}
			}
			break;
		default:
			break;
	}


	/* CUP/Smart pay Indicator:銀聯卡 / Smart Pay交易使用
	 * Indicator = 'C'，表示為CUP交易
	 * Indicator = 'N'，表示為一般信用卡交易
	 * Indicator = 'S'，表示為SmartPay交易，(SmartPay的Sale Reversal和Refund，收銀機一定要送'S'。SmartPay的Sale送'N'，因為Sale不需要按Hotkey)
	 */
	switch (inTransType)
	{
		case _ECR_8N1_SALE_NO_:				/* 一般交易 */
		case _ECR_8N1_REFUND_NO_:			/* 退貨交易 */
		case _ECR_8N1_VOID_NO_:				/* 取消 */
		case _ECR_8N1_PREAUTH_NO_:			/* 預先授權 */
		case _ECR_8N1_PREAUTH_COMPLETE_NO_:		/* 預先授權完成 */
		case _ECR_8N1_ESVC_TOP_UP_NO_:			/* 電子票證加值 */
		case _ECR_8N1_ESVC_BALANCE_INQUIRY_NO_:		/* 電子票證餘額查詢 */
		case _ECR_8N1_ESVC_VOID_TOP_UP_NO_:		/* 電子票證加值取消 */
		case _ECR_8N1_HG_REWARD_SALE_NO_:		/* HG紅利積點 */
		case _ECR_8N1_HG_ONLINE_REDEEMPTION_NO_:	/* HG點數抵扣 */
		case _ECR_8N1_HG_POINT_CERTAIN_NO_:		/* HG加價購 */
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s: %s  ", "CUPIndicator", srECROb->srTransData.szField_05);
				inLogPrintf(AT, szDebugMsg);
			}

			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(srECROb->srTransData.szField_05) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "CUPIndicator", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "CUPIndicator Not Exist Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

				return (VS_ERROR);
			}

			break;
		case _ECR_8N1_PREAUTH_CANCEL_NO_:		/* 預先授權取消 */
		case _ECR_8N1_PREAUTH_COMPLETE_CANCEL_NO_:	/* 預先授權完成取消 */
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "  CUPIndicator :%s  ", srECROb->srTransData.szField_05);
				inLogPrintf(AT, szDebugMsg);
			}

			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(srECROb->srTransData.szField_05) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "CUPIndicator", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "CUPIndicator Not Exist Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

				return (VS_ERROR);
			}

			/* 預先授權取消及完成僅支援銀聯卡交易 ，故此二Request之 CUP Indicator欄位僅可放 ”C”*/
			if (memcmp(&srECROb->srTransData.szField_05[0], "C", 1) != 0)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "CUPIndicator", "CUP Not\"C\" Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "非銀聯交易別帶CUP Indicator");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				return (VS_ERROR);
			}
			break;
		default:
			break;
	}

	switch (inTransType)
	{
		case _ECR_8N1_SALE_NO_:				/* 一般交易 */
		case _ECR_8N1_REFUND_NO_:			/* 退貨交易 */
		case _ECR_8N1_VOID_NO_:				/* 取消 */
		case _ECR_8N1_PREAUTH_NO_:			/* 預先授權 */
		case _ECR_8N1_PREAUTH_COMPLETE_NO_:		/* 預先授權完成 */
		case _ECR_8N1_ESVC_TOP_UP_NO_:			/* 電子票證加值 */
		case _ECR_8N1_ESVC_BALANCE_INQUIRY_NO_:		/* 電子票證餘額查詢 */
		case _ECR_8N1_ESVC_VOID_TOP_UP_NO_:		/* 電子票證加值取消 */
		case _ECR_8N1_HG_REWARD_SALE_NO_:		/* HG紅利積點 */
		case _ECR_8N1_HG_ONLINE_REDEEMPTION_NO_:	/* HG點數抵扣 */
		case _ECR_8N1_HG_POINT_CERTAIN_NO_:		/* HG加價購 */
		case _ECR_8N1_PREAUTH_CANCEL_NO_:		/* 預先授權取消 */
		case _ECR_8N1_PREAUTH_COMPLETE_CANCEL_NO_:	/* 預先授權完成取消 */
			/* 如果是'C'代表是銀聯卡交易 */
			if (memcmp(&srECROb->srTransData.szField_05[0], "C", 1) == 0)
			{
				/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/7 上午 10:28 */
				if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
				{
					/* 註：因Üny 不支援銀聯卡，故針對 Üny 掃碼交易無論
					 *  CUP Indicator 帶什麼值請 EDC 都視為信用卡交易。
					 */
				}
				else
				{
					pobTran->srBRec.uszCUPTransBit = VS_TRUE;
					/* 銀聯目前沒這些交易別，要回接收資料錯誤 */
					switch (inTransType)
					{
						case _ECR_8N1_INSTALLMENT_NO_:
						case _ECR_8N1_REDEEM_NO_:
						case _ECR_8N1_INSTALLMENT_REFUND_NO_:
						case _ECR_8N1_REDEEM_REFUND_NO_:
						case _ECR_8N1_INSTALLMENT_ADJUST_NO_:
						case _ECR_8N1_REDEEM_ADJUST_NO_:
						case _ECR_8N1_OFFLINE_NO_:
							memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
							inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
							memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
							sprintf(pobTran->szErrorMsgBuff2, "銀聯無此交易別");
							pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
							return (VS_ERROR);
							break;
						default:
							break;
					}
				}
			}

			/* 如果是'S'代表是SmartPay交易 */
			if (memcmp(&srECROb->srTransData.szField_05[0], "S", 1) == 0)
			{
				pobTran->srBRec.uszFiscTransBit = VS_TRUE;
				/* SmartPay不用簽名 */
				pobTran->srBRec.uszNoSignatureBit = VS_TRUE;
			}

			/* 如果是'E'代表是電票交易 */
			if (memcmp(&srECROb->srTransData.szField_05[0], "E", 1) == 0)
			{
				pobTran->srTRec.uszESVCTransBit = VS_TRUE;
				/* 若有票證未結帳，要提示訊息，比照520 2019/8/29 下午 1:46 */
				inNCCC_Ticket_Func_Must_SETTLE_Show_In_Menu();
			}
			break;
		default:
			break;
	}
	
	/* 銀行別/付款別 HOST ID*/
	switch (inTransType)
	{
		case _ECR_8N1_VOID_NO_:				/* 取消交易 */
		case _ECR_8N1_TIP_NO_:				/* 小費交易 */
		case _ECR_8N1_PREAUTH_CANCEL_NO_:		/* 預先授權取消 */
		case _ECR_8N1_PREAUTH_COMPLETE_CANCEL_NO_:	/* 預先授權完成取消 */
		case _ECR_8N1_REPRINT_RECEIPT_NO_:		/* 重印簽單 */
		case _ECR_8N1_MENU_REVIEW_DETAIL_NO_:		/* 明細查詢 */
		case _ECR_8N1_MENU_REVIEW_TOTAL_NO_:		/* 總額查詢 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, srECROb->srTransData.szHostID, 2);
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "HOST ID", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "HOST ID Not Exist Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
				return (VS_ERROR);
			}
			
			memset(szHostLabel, 0x00, sizeof(szHostLabel));

			if (memcmp(szTemplate, _ECR_8N1_NCCC_HOSTID_NCCC_, 2) == 0)
			{
				memcpy(szHostLabel, _HOST_NAME_CREDIT_NCCC_, strlen(_HOST_NAME_CREDIT_NCCC_));
			}
			else if (memcmp(szTemplate, _ECR_8N1_NCCC_HOSTID_DCC_, 2) == 0)
			{
				memcpy(szHostLabel, _HOST_NAME_DCC_, strlen(_HOST_NAME_DCC_));
			}
			/* 2018/5/8 下午 2:10 DFS需求不再使用大來主機 */
			else if (memcmp(szTemplate, _ECR_8N1_NCCC_HOSTID_LOYALTY_REDEEM_, 2) == 0)
			{
				memcpy(szHostLabel, _HOST_NAME_CREDIT_NCCC_, strlen(_HOST_NAME_CREDIT_NCCC_));
			}
			else if (memcmp(szTemplate, _ECR_8N1_NCCC_HOSTID_ESVC_, 2) == 0)
			{
				memcpy(szHostLabel, _HOST_NAME_ESVC_, strlen(_HOST_NAME_ESVC_));
				pobTran->srTRec.uszESVCTransBit = VS_TRUE;
			}
			else if (memcmp(szTemplate, _ECR_8N1_NCCC_HOSTID_HG_, 2) == 0)
			{
				memcpy(szHostLabel, _HOST_NAME_HG_, strlen(_HOST_NAME_HG_));
			}
			else
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "無對應Host ID");
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "%s : HOST ID Not Pair", szTemplate);
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
					
				return (VS_ERROR);
			}
			
			/* 初始化 */
			i = 0;
			do
			{
				/* 按順序load每一個HOST */
				if (inLoadHDTRec(i) < 0)
				{
					/* 如果return VS_ERROR 代表table沒有該HOST */
					memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
					inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "%s : HOST ID Not Found", szHostLabel);
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
					
					return (VS_ERROR);
				}

				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetHostLabel(szTemplate);
				if (!memcmp(szTemplate, szHostLabel, strlen(szHostLabel)))
				{
					/* 比對成功後，判斷HOST是否開啟  */
					memset(szHostEnable, 0x00, sizeof(szHostEnable));
					inGetHostEnable(szHostEnable);
					if (szHostEnable[0] != 'Y')
					{
						/* Host沒開 */
						if (ginDebug == VS_TRUE) 
						{
							memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
							sprintf(szDebugMsg, "%s Not Open Error", szHostLabel);
							inLogPrintf(AT, szDebugMsg);
						}
						
						memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
						inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
						memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
						sprintf(pobTran->szErrorMsgBuff2, "%s : HOST ID Not Open", szHostLabel);
						pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
					
						/* 如果return VS_ERROR 代表Host沒開 */
						return (VS_ERROR);
					}
					else
					{
						/* Host有開 */
						pobTran->srBRec.inHDTIndex = i;
						
						/* Load HDPT */
						inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
						memset(szBatchNum, 0x00, sizeof(szBatchNum));
						inGetBatchNum(szBatchNum);
						pobTran->srBRec.lnBatchNum = atol(szBatchNum);
						
						if (ginDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
							sprintf(szDebugMsg, "%s Open", szHostLabel);
							inLogPrintf(AT, szDebugMsg);
						}
						
					}/* Host Enable比對End */

					break;
				}/* Host Label比對 End */
				i++;

			} while (1);
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%d", "HDTindex", pobTran->srBRec.inHDTIndex);
				inLogPrintf(AT, szDebugMsg);
			}
			
			break;
			
		default:
			break;
	}
	
	/* EDC簽單序號 Receipt No(InvoiceNumber) */
	switch (inTransType)
	{
		case _ECR_8N1_VOID_NO_:				/* 取消交易 */
		case _ECR_8N1_TIP_NO_:				/* 小費交易 */
		case _ECR_8N1_PREAUTH_CANCEL_NO_:		/* 預先授權取消 */
		case _ECR_8N1_PREAUTH_COMPLETE_CANCEL_NO_:	/* 預先授權完成取消 */
		case _ECR_8N1_REPRINT_RECEIPT_NO_:		/* 重印簽單 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], srECROb->srTransData.szReceiptNo, 6);
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Receipt No", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "Receipt No Not Exist Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
						
				return (VS_ERROR);
			}
			else
			{
				pobTran->srBRec.lnOrgInvNum = atol(szTemplate);
			}
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%ld", "lnOrgInvNum", pobTran->srBRec.lnOrgInvNum);
				inLogPrintf(AT, szDebugMsg);
			}
			
			break;
		default:
			break;
	}
	
	/* 處理交易金額 (Trans Amount) */
	switch (inTransType)
	{
		case _ECR_8N1_SALE_NO_:				/* 一般交易 */
		case _ECR_8N1_REFUND_NO_:			/* 退貨交易 */
		case _ECR_8N1_OFFLINE_NO_:			/* 交易補登 */
		case _ECR_8N1_PREAUTH_NO_:			/* 預先授權 */
		case _ECR_8N1_PREAUTH_COMPLETE_NO_:		/* 預先授權完成 */
		case _ECR_8N1_INSTALLMENT_NO_:			/* 分期 */
		case _ECR_8N1_INSTALLMENT_REFUND_NO_:		/* 分期退貨 */
		case _ECR_8N1_INSTALLMENT_ADJUST_NO_:		/* 分期調帳 */
		case _ECR_8N1_REDEEM_NO_:			/* 紅利 */		
		case _ECR_8N1_REDEEM_REFUND_NO_:		/* 紅利退貨 */
		case _ECR_8N1_REDEEM_ADJUST_NO_:		/* 紅利調帳 */
		case _ECR_8N1_ESVC_TOP_UP_NO_:			/* 電子票證加值 */
		case _ECR_8N1_ESVC_VOID_TOP_UP_NO_:		/* 電子票證加值取消 */
		case _ECR_8N1_HG_REWARD_SALE_NO_:		/* HG紅利積點 */
		case _ECR_8N1_HG_REWARD_INSTALLMENT_NO_ :	/* 快樂購紅利積點 + 信用卡分期付款 */
		case _ECR_8N1_HG_REWARD_REDEMPTION_NO_ :	/* 快樂購紅利積點 + 信用卡紅利扣抵 */
		case _ECR_8N1_HG_ONLINE_REDEEMPTION_NO_:	/* HG點數抵扣 */
		case _ECR_8N1_HG_POINT_CERTAIN_NO_:		/* HG加價購 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &srECROb->srTransData.szField_10[0], 10);
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Amount", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "Trans Amount Not Exist Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
				return (VS_ERROR);
			}
			else if ((!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_076_8_DIGITS_, _CUSTOMER_INDICATOR_SIZE_) && atol(szTemplate) > 99999999) || 
				 (memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_076_8_DIGITS_, _CUSTOMER_INDICATOR_SIZE_) && atol(szTemplate) > 9999999))
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Amount", "Out of bound");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_076_8_DIGITS_, _CUSTOMER_INDICATOR_SIZE_))
				{
					sprintf(pobTran->szErrorMsgBuff2, "金額超過8位數");
				}
				else
				{
					sprintf(pobTran->szErrorMsgBuff2, "金額超過7位數");
				}
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
				
				return (VS_ERROR);
			}
			else
			{
				pobTran->srBRec.lnTxnAmount = atol(szTemplate);
				pobTran->srBRec.lnOrgTxnAmount = atol(szTemplate);
				pobTran->srBRec.lnTotalTxnAmount = atol(szTemplate);
			}

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%ld", "交易金額", pobTran->srBRec.lnTxnAmount);
				inLogPrintf(AT, szDebugMsg);
			}
			
			break;
		default:
			break;
	}
	
	/* 交易日期 Trans Date */
	switch (inTransType)
	{
		case _ECR_8N1_END_CARD_NO_INQUIRY_NO_:		/* 結束卡號查詢*/
//		case _ECR_8N1_SALE_NO_:				/* 一般交易 */		/* 不以ECR日期為主，會發生與機器日期不符的狀況 */
		case _ECR_8N1_REFUND_NO_:			/* 退貨交易 */
		case _ECR_8N1_VOID_NO_:				/* 取消交易 */
//		case _ECR_8N1_OFFLINE_NO_:			/* 交易補登 */		/* 不以ECR日期為主，會發生與機器日期不符的狀況 */
		case _ECR_8N1_TIP_NO_:				/* 小費交易 */
//		case _ECR_8N1_PREAUTH_NO_:			/* 預先授權 */		/* 不以ECR日期為主，會發生與機器日期不符的狀況 */
		case _ECR_8N1_PREAUTH_CANCEL_NO_:		/* 預先授權取消 */
		case _ECR_8N1_PREAUTH_COMPLETE_NO_:		/* 預先授權完成 */
		case _ECR_8N1_PREAUTH_COMPLETE_CANCEL_NO_:	/* 預先授權完成取消 */
//		case _ECR_8N1_INSTALLMENT_NO_:			/* 分期 */		/* 不以ECR日期為主，會發生與機器日期不符的狀況 */
		case _ECR_8N1_INSTALLMENT_REFUND_NO_:		/* 分期退貨 */
		case _ECR_8N1_INSTALLMENT_ADJUST_NO_:		/* 分期調帳 */
//		case _ECR_8N1_REDEEM_NO_:			/* 紅利 */		/* 不以ECR日期為主，會發生與機器日期不符的狀況 */	
		case _ECR_8N1_REDEEM_REFUND_NO_:		/* 紅利退貨 */
		case _ECR_8N1_REDEEM_ADJUST_NO_:		/* 紅利調帳 */
		case _ECR_8N1_SETTLEMENT_NO_:			/* 結帳交易 */
		case _ECR_8N1_REPRINT_RECEIPT_NO_:		/* 重印簽單 */
		case _ECR_8N1_MENU_REVIEW_DETAIL_NO_:		/* 明細查詢 */
		case _ECR_8N1_MENU_REVIEW_TOTAL_NO_:		/* 總額查詢 */
		case _ECR_8N1_MENU_REPORT_DETAIL_NO_:		/* 明細列印 */
		case _ECR_8N1_MENU_REPORT_TOTAL_NO_:		/* 總額列印 */
		case _ECR_8N1_ESVC_TOP_UP_NO_:			/* 電子票證加值 */
		case _ECR_8N1_ESVC_BALANCE_INQUIRY_NO_:		/* 電子票證餘額查詢 */
		case _ECR_8N1_ESVC_VOID_TOP_UP_NO_:		/* 電子票證加值取消 */
		case _ECR_8N1_ECHO_NO_:				/* 連線測試 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &srECROb->srTransData.szTransDate[0], 6);
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Date", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
//				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
//				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
//				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
//				sprintf(pobTran->szErrorMsgBuff2, "Trans Date Not Exist Error");
//				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
//				
//				return (VS_ERROR);
			}
			else
			{
				/* 根據安全理由，不採信ECR日期時間 */
			}
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "交易日期", pobTran->srBRec.szDate);
				inLogPrintf(AT, szDebugMsg);
			}
			break;
		default:
			break;
	}
	
	/* 交易時間 Trans Time */
	switch (inTransType)
	{
		case _ECR_8N1_END_CARD_NO_INQUIRY_NO_:		/* 結束卡號查詢*/
		case _ECR_8N1_SALE_NO_:				/* 一般交易 */
		case _ECR_8N1_REFUND_NO_:			/* 退貨交易 */
		case _ECR_8N1_VOID_NO_:				/* 取消交易 */
		case _ECR_8N1_OFFLINE_NO_:			/* 交易補登 */
		case _ECR_8N1_TIP_NO_:				/* 小費交易 */
		case _ECR_8N1_PREAUTH_NO_:			/* 預先授權 */
		case _ECR_8N1_PREAUTH_CANCEL_NO_:		/* 預先授權取消 */
		case _ECR_8N1_PREAUTH_COMPLETE_NO_:		/* 預先授權完成 */
		case _ECR_8N1_PREAUTH_COMPLETE_CANCEL_NO_:	/* 預先授權完成取消 */
		case _ECR_8N1_INSTALLMENT_NO_:			/* 分期 */
		case _ECR_8N1_INSTALLMENT_REFUND_NO_:		/* 分期退貨 */
		case _ECR_8N1_INSTALLMENT_ADJUST_NO_:		/* 分期調帳 */
		case _ECR_8N1_REDEEM_NO_:			/* 紅利 */		
		case _ECR_8N1_REDEEM_REFUND_NO_:		/* 紅利退貨 */
		case _ECR_8N1_REDEEM_ADJUST_NO_:		/* 紅利調帳 */
		case _ECR_8N1_SETTLEMENT_NO_:			/* 結帳交易 */
		case _ECR_8N1_REPRINT_RECEIPT_NO_:		/* 重印簽單 */
		case _ECR_8N1_MENU_REVIEW_DETAIL_NO_:		/* 明細查詢 */
		case _ECR_8N1_MENU_REVIEW_TOTAL_NO_:		/* 總額查詢 */
		case _ECR_8N1_MENU_REPORT_DETAIL_NO_:		/* 明細列印 */
		case _ECR_8N1_MENU_REPORT_TOTAL_NO_:		/* 總額列印 */
		case _ECR_8N1_ESVC_TOP_UP_NO_:			/* 電子票證加值 */
		case _ECR_8N1_ESVC_BALANCE_INQUIRY_NO_:		/* 電子票證餘額查詢 */
		case _ECR_8N1_ESVC_VOID_TOP_UP_NO_:		/* 電子票證加值取消 */
		case _ECR_8N1_ECHO_NO_:				/* 連線測試 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szTransTime[0], 6);
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Time", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
//				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
//				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
//				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
//				sprintf(pobTran->szErrorMsgBuff2, "Trans Time Not Exist Error");
//				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
//				
//				return (VS_ERROR);
			}
			else
			{
				/* 根據安全理由，不採信ECR日期時間 */
			}
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "交易時間", pobTran->srBRec.szTime);
				inLogPrintf(AT, szDebugMsg);
			}
			
			break;
			
		default:
			break;
	}
	
	/* 授權碼(Auth Code) ECR電文中給9個字元 但ATS電文只需要6個，所以只抓6個 */
	switch (inTransType)
	{
		case _ECR_8N1_REFUND_NO_:			/* 退貨 */
		case _ECR_8N1_OFFLINE_NO_:			/* 交易補登 */
		case _ECR_8N1_INSTALLMENT_REFUND_NO_:		/* 分期退貨 */
		case _ECR_8N1_INSTALLMENT_ADJUST_NO_:		/* 分期調帳 */
		case _ECR_8N1_REDEEM_REFUND_NO_:		/* 紅利退貨 */
		case _ECR_8N1_REDEEM_ADJUST_NO_:		/* 紅利調帳 */
			/* 處理授權碼(optional) */
			memset(pobTran->srBRec.szAuthCode, 0x00, sizeof(pobTran->srBRec.szAuthCode));
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szApprovalNo[0], 6);
			memcpy(pobTran->srBRec.szAuthCode, szTemplate, 6);
			break;
		case _ECR_8N1_PREAUTH_COMPLETE_NO_:		/* 預先授權完成 */
			/* 處理授權碼(must have) */
			memset(pobTran->srBRec.szAuthCode, 0x00, sizeof(pobTran->srBRec.szAuthCode));
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szApprovalNo[0], 6);
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Auth Code", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "Auth Code Not Exist Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
				return (VS_ERROR);
			}
			else
			{
				memcpy(pobTran->srBRec.szAuthCode, szTemplate, 6);
			}
			
			break;
			
		default :
			break;
	}
	
	/* 其他金額(Exp Amount) */
	switch (inTransType)
	{
		case _ECR_8N1_TIP_NO_:				/* 小費交易 */
			/* 小費金額*/
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetTRTFileName(szTemplate);
			if (!memcmp(szTemplate, _TRT_FILE_NAME_DCC_, 6) && !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TIP_, 2))
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &srECROb->srTransData.szExpAmount[0], 12);
			}
			else
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &srECROb->srTransData.szExpAmount[0], 10);
			}
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Exp Amount", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "Exp Amount Not Exist Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
				return (VS_ERROR);
			}
			
			pobTran->lnOldTaxAmount = atol(szTemplate);
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Tip: %ld", pobTran->lnOldTaxAmount);
				inLogPrintf(AT, szDebugMsg);
			}

			break;
			
		case _ECR_8N1_PREAUTH_COMPLETE_NO_:			/* 預先授權完成 預先授權完成交易之原預先授權金額 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szAuthAmount[0], 10);

			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Exp Amount", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "Exp Amount Not Exist Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
				return (VS_ERROR);
			}

			pobTran->srBRec.lnOrgTxnAmount = atol(szTemplate);
			break;
			
		default :
			break;
	}
	
	/* 處理櫃號(Store ID) */
	switch (inTransType)
	{
		case _ECR_8N1_SALE_NO_:				/* 一般交易 */
		case _ECR_8N1_REFUND_NO_:			/* 退貨交易 */
		case _ECR_8N1_OFFLINE_NO_:			/* 交易補登 */
		case _ECR_8N1_PREAUTH_NO_:			/* 預先授權 */
		case _ECR_8N1_PREAUTH_CANCEL_NO_:		/* 預先授權取消 */
		case _ECR_8N1_PREAUTH_COMPLETE_NO_:		/* 預先授權完成 */
		case _ECR_8N1_PREAUTH_COMPLETE_CANCEL_NO_:	/* 預先授權完成取消 */
		case _ECR_8N1_INSTALLMENT_NO_:			/* 分期 */
		case _ECR_8N1_INSTALLMENT_REFUND_NO_:		/* 分期退貨 */
		case _ECR_8N1_INSTALLMENT_ADJUST_NO_:		/* 分期調帳 */
		case _ECR_8N1_REDEEM_NO_:			/* 紅利 */
		case _ECR_8N1_REDEEM_REFUND_NO_:		/* 紅利退貨 */
		case _ECR_8N1_REDEEM_ADJUST_NO_:		/* 紅利調帳 */
		case _ECR_8N1_HG_REWARD_SALE_NO_:		/* HG紅利積點 */
		case _ECR_8N1_HG_REWARD_INSTALLMENT_NO_ :	/* 快樂購紅利積點 + 信用卡分期付款 */
		case _ECR_8N1_HG_REWARD_REDEMPTION_NO_ :	/* 快樂購紅利積點 + 信用卡紅利扣抵 */
		case _ECR_8N1_HG_ONLINE_REDEEMPTION_NO_ :	/* 快樂購點數扣抵 */
		case _ECR_8N1_HG_POINT_CERTAIN_NO_ :		/* 快樂購加價購 */
		case _ECR_8N1_HG_FULL_REDEEMPTION_NO_ :		/* 快樂購點數兌換 */
		case _ECR_8N1_HG_REDEEM_REFUND_NO_ :		/* 快樂購扣抵退貨 */
		case _ECR_8N1_HG_REWARD_REFUND_NO_ :		/* 快樂購回饋退貨 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szStoreId[0], 18);
			
			if (strlen(szTemplate) > 0)
			{
				memcpy(pobTran->srBRec.szStoreID, szTemplate, strlen(szTemplate));
				/* 櫃號不滿18，補空白 */
				if (strlen(szTemplate) < 18)
				{
					memset(&pobTran->srBRec.szStoreID[strlen(szTemplate)], 0x20, 18 - strlen(szTemplate));
				}

			}
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "Store ID", pobTran->srBRec.szStoreID);
				inLogPrintf(AT, szDebugMsg);
			}
			
			break;
		case _ECR_8N1_ESVC_TOP_UP_NO_:			/* 電子票證加值 */
		case _ECR_8N1_ESVC_VOID_TOP_UP_NO_:		/* 電子票證加值取消 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szStoreId[0], 18);
			
			if (strlen(szTemplate) > 0)
			{
				memcpy(pobTran->srBRec.szStoreID, szTemplate, strlen(szTemplate));
				/* 櫃號不滿18，補空白 */
				if (strlen(szTemplate) < 18)
				{
					memset(&pobTran->srBRec.szStoreID[strlen(szTemplate)], 0x20, 18 - strlen(szTemplate));
				}

			}
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "Store ID", pobTran->srBRec.szStoreID);
				inLogPrintf(AT, szDebugMsg);
			}
			
			break;
		default:
			break;
	}
	
	if (inTransType == _ECR_8N1_AWARD_REDEEM_NO_)
	{
		/* 兌換模式 */
		switch (inTransType)
		{
			case _ECR_8N1_AWARD_REDEEM_NO_:		/* 優惠兌換 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &srECROb->srTransData.szField_20[0], 1);	/* (1=條碼兌換，2=卡號兌換) */

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s :%s", "Award Redeem Mode", szTemplate);
					inLogPrintf(AT, szDebugMsg);
				}
				
				/* 條碼兌換 */
				if (memcmp(szTemplate, "1", 1) == 0)
				{
					memcpy(pobTran->szL3_AwardWay, "1", 1);
				}
				else if (memcmp(szTemplate, "2", 1) == 0)
				{
					/* 至EDC過卡流程決定 */
					memcpy(pobTran->szL3_AwardWay, "0", 1);
				}
				else
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "ECR未輸入兌換方式");
					}
					memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
					inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "ECR未輸入兌換方式");
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

					return (VS_ERROR);
				}
				
				/* 兌換方式為條碼才抓條碼*/
				if (memcmp(pobTran->szL3_AwardWay, "1", strlen("1")) == 0)
				{
					if (srECROb->srTransData.szBarCode1[0] == ' ' && srECROb->srTransData.szBarCode2[0] == ' ')
					{
						if (ginDebug == VS_TRUE)
						{
							inLogPrintf(AT, "ECR未輸入條碼");
						}
						memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
						inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
						memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
						sprintf(pobTran->szErrorMsgBuff2, "ECR未輸入條碼");
						pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
						
						return (VS_ERROR);
					}
					
					/* 條碼兌換 防呆&阻擋&存Barcode */
					/* 11 表示只有一個一維條碼，且這是第1 個。 */
					if (srECROb->srTransData.szBarCode1[0] == '1' && srECROb->srTransData.szBarCode1[1] == '1')
					{
						memcpy(&pobTran->szL3_Barcode1[0], &srECROb->srTransData.szBarCode1[0], 20);
						sprintf(pobTran->szL3_Barcode1Len, "%02d", strlen(pobTran->szL3_Barcode1));
					}
					else if (srECROb->srTransData.szBarCode2[0] == '1' && srECROb->srTransData.szBarCode2[0] == '1')
					{
						memcpy(&pobTran->szL3_Barcode1[0], &srECROb->srTransData.szBarCode2[0], 20);
						sprintf(pobTran->szL3_Barcode1Len, "%02d", strlen(pobTran->szL3_Barcode1));
					}
					else if (srECROb->srTransData.szBarCode1[0] == '2' && srECROb->srTransData.szBarCode1[1] == '1')
					{
						if (srECROb->srTransData.szBarCode2[0] == '2' && srECROb->srTransData.szBarCode2[1] == '2')
						{
							memcpy(&pobTran->szL3_Barcode1[0], &srECROb->srTransData.szBarCode1[0], 20);
							sprintf(pobTran->szL3_Barcode1Len, "%02d", strlen(pobTran->szL3_Barcode1));
							memcpy(&pobTran->szL3_Barcode2[0], &srECROb->srTransData.szBarCode2[0], 20);
							sprintf(pobTran->szL3_Barcode2Len, "%02d", strlen(pobTran->szL3_Barcode2));
						}
						else
						{
							if (ginDebug == VS_TRUE)
							{
								inLogPrintf(AT, "ECR條碼邏輯錯誤");
							}
							memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
							inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
							memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
							sprintf(pobTran->szErrorMsgBuff2, "ECR條碼邏輯錯誤");
							pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
						
							return (VS_ERROR);
						}
					}
					else if (srECROb->srTransData.szBarCode1[0] == '2' && srECROb->srTransData.szBarCode1[1] == '2')
					{
						if (srECROb->srTransData.szBarCode2[0] == '2' && srECROb->srTransData.szBarCode2[1] == '1')
						{
							memcpy(&pobTran->szL3_Barcode2[0], &srECROb->srTransData.szBarCode1[0], 20);
							sprintf(pobTran->szL3_Barcode2Len, "%02d", strlen(pobTran->szL3_Barcode2));
							memcpy(&pobTran->szL3_Barcode1[0], &srECROb->srTransData.szBarCode2[0], 20);
							sprintf(pobTran->szL3_Barcode1Len, "%02d", strlen(pobTran->szL3_Barcode1));
						}
						else
						{
							if (ginDebug == VS_TRUE)
							{
								inLogPrintf(AT, "ECR條碼邏輯錯誤");
							}
							memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
							inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
							memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
							sprintf(pobTran->szErrorMsgBuff2, "ECR條碼邏輯錯誤");
							pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
							
							return (VS_ERROR);
						}
					}
					else
					{
						if (ginDebug == VS_TRUE)
						{
							inLogPrintf(AT, "ECR條碼邏輯錯誤");
						}
						memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
						inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
						memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
						sprintf(pobTran->szErrorMsgBuff2, "ECR條碼邏輯錯誤");
						pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
							
						return (VS_ERROR);
					}
				}
				break;
			default:
				break;
		}
	}
	else if (inTransType == _ECR_8N1_VOID_AWARD_REDEEM_NO_)
	{
		/* 兌換模式 */
		switch (inTransType)
		{
			case _ECR_8N1_VOID_AWARD_REDEEM_NO_:		/* 取消優惠兌換 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &srECROb->srTransData.szField_20[0], 1);	/* (1=條碼兌換，2=卡號兌換) */

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s :%s", "Award Redeem Mode", szTemplate);
					inLogPrintf(AT, szDebugMsg);
				}
				
				/* 條碼兌換 */
				if (memcmp(szTemplate, "1", 1) == 0)
				{
					memcpy(pobTran->szL3_AwardWay, "1", 1);
				}
				else
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "ECR取消兌換不接受卡號輸入");
					}
					memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
					inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "ECR取消兌換不接受卡號輸入");
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
							
					return (VS_ERROR);
				}
				
				/* 兌換方式為條碼才抓條碼*/
				if (memcmp(pobTran->szL3_AwardWay, "1", strlen("1")) == 0)
				{
					if (srECROb->srTransData.szBarCode1[0] == ' ' && srECROb->srTransData.szBarCode2[0] == ' ')
					{
						if (ginDebug == VS_TRUE)
						{
							inLogPrintf(AT, "ECR未輸入條碼");
						}
						memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
						inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
						memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
						sprintf(pobTran->szErrorMsgBuff2, "ECR未輸入條碼");
						pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
					
						return (VS_ERROR);
					}
					
					/* 條碼兌換 防呆&阻擋&存Barcode */
					/* 11 表示只有一個一維條碼，且這是第1 個。 */
					if (srECROb->srTransData.szBarCode1[0] == '1' && srECROb->srTransData.szBarCode1[1] == '1')
					{
						memcpy(&pobTran->szL3_Barcode1[0], &srECROb->srTransData.szBarCode1[0], 20);
						sprintf(pobTran->szL3_Barcode1Len, "%02d", strlen(pobTran->szL3_Barcode1));
					}
					else if (srECROb->srTransData.szBarCode2[0] == '1' && srECROb->srTransData.szBarCode2[1] == '1')
					{
						memcpy(&pobTran->szL3_Barcode1[0], &srECROb->srTransData.szBarCode2[0], 20);
						sprintf(pobTran->szL3_Barcode1Len, "%02d", strlen(pobTran->szL3_Barcode1));
					}
					else if (srECROb->srTransData.szBarCode1[0] == '2' && srECROb->srTransData.szBarCode1[1] == '1')
					{
						if (srECROb->srTransData.szBarCode2[0] == '2' && srECROb->srTransData.szBarCode2[1] == '2')
						{
							memcpy(&pobTran->szL3_Barcode1[0], &srECROb->srTransData.szBarCode1[0], 20);
							sprintf(pobTran->szL3_Barcode1Len, "%02d", strlen(pobTran->szL3_Barcode1));
							memcpy(&pobTran->szL3_Barcode2[0], &srECROb->srTransData.szBarCode2[0], 20);
							sprintf(pobTran->szL3_Barcode2Len, "%02d", strlen(pobTran->szL3_Barcode2));
						}
						else
						{
							if (ginDebug == VS_TRUE)
							{
								inLogPrintf(AT, "ECR條碼邏輯錯誤");
							}
							memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
							inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
							memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
							sprintf(pobTran->szErrorMsgBuff2, "ECR條碼邏輯錯誤");
							pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
						
							return (VS_ERROR);
						}
					}
					else if (srECROb->srTransData.szBarCode1[0] == '2' && srECROb->srTransData.szBarCode1[1] == '2')
					{
						if (srECROb->srTransData.szBarCode2[0] == '2' && srECROb->srTransData.szBarCode2[1] == '1')
						{
							memcpy(&pobTran->szL3_Barcode2[0], &srECROb->srTransData.szBarCode1[0], 20);
							sprintf(pobTran->szL3_Barcode2Len, "%02d", strlen(pobTran->szL3_Barcode2));
							memcpy(&pobTran->szL3_Barcode1[0], &srECROb->srTransData.szBarCode2[0], 20);
							sprintf(pobTran->szL3_Barcode1Len, "%02d", strlen(pobTran->szL3_Barcode1));
						}
						else
						{
							if (ginDebug == VS_TRUE)
							{
								inLogPrintf(AT, "ECR條碼邏輯錯誤");
							}
							memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
							inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
							memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
							sprintf(pobTran->szErrorMsgBuff2, "ECR條碼邏輯錯誤");
							pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
							
							return (VS_ERROR);
						}
					}
					else
					{
						if (ginDebug == VS_TRUE)
						{
							inLogPrintf(AT, "ECR條碼邏輯錯誤");
						}
						memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
						inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
						memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
						sprintf(pobTran->szErrorMsgBuff2, "ECR條碼邏輯錯誤");
						pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
							
						return (VS_ERROR);
					}
				}
				break;
			default:
				break;
		}
	}
	else
	{
		/* 實際支付金額RDM Paid Amt (含小數 2位) */
		switch (inTransType)
		{
			case _ECR_8N1_REDEEM_REFUND_NO_:		/* 紅利退貨 */
			case _ECR_8N1_REDEEM_ADJUST_NO_:		/* 紅利調帳 */
			case _ECR_8N1_HG_REWARD_REDEMPTION_NO_ :	/* 快樂購紅利積點 + 信用卡紅利扣抵 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &srECROb->srTransData.szRDMPaidAmt[0], 10);	/* 12位數，這裡只取10位 */
                                
                                /* 支付金額比交易金額大 */
                                if (atol(szTemplate) > pobTran->srBRec.lnTxnAmount)
                                {
                                        memset(srECROb->srTransData.szRDMPaidAmt, 0x00, sizeof(srECROb->srTransData.szRDMPaidAmt));
                                        memcpy(srECROb->srTransData.szRDMPaidAmt, "000000000000", 12);	/* 重輸 */
                                        
                                        memset(szTemplate, 0x00, sizeof(szTemplate));
                                        memcpy(szTemplate, &srECROb->srTransData.szRDMPaidAmt[0], 10);	/* 12位數，這裡只取10位 */
                                }

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s :%s", "RDM Paid Amt", szTemplate);
					inLogPrintf(AT, szDebugMsg);
				}

				pobTran->srBRec.lnRedemptionPaidCreditAmount = atol(szTemplate);

				break;
			default:
				break;
		}

		/* 信用卡紅利扣抵點數 RDM Point */
		switch (inTransType)
		{
			case _ECR_8N1_REDEEM_REFUND_NO_:		/* 紅利退貨 */
			case _ECR_8N1_REDEEM_ADJUST_NO_:		/* 紅利調帳 */
			case _ECR_8N1_HG_REWARD_REDEMPTION_NO_ :	/* 快樂購紅利積點 + 信用卡紅利扣抵 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &srECROb->srTransData.szRDMPoint[0], 8);

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s :%s", "RDM Point", szTemplate);
					inLogPrintf(AT, szDebugMsg);
				}

				pobTran->srBRec.lnRedemptionPoints = atol(szTemplate);

				break;
			default:
				break;
		}

		/* 分期期數 Installment Period */
		switch (inTransType)
		{
			case _ECR_8N1_INSTALLMENT_NO_:			/* 分期 */
			case _ECR_8N1_INSTALLMENT_REFUND_NO_:		/* 分期退貨 */
			case _ECR_8N1_INSTALLMENT_ADJUST_NO_:		/* 分期調帳 */
			case _ECR_8N1_HG_REWARD_INSTALLMENT_NO_ :	/* 快樂購紅利積點 + 信用卡分期付款 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &srECROb->srTransData.szInstallmentPeriod[0], 2);

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s :%s", "Installment Period", szTemplate);
					inLogPrintf(AT, szDebugMsg);
				}

				pobTran->srBRec.lnInstallmentPeriod = atol(szTemplate);

				break;
			default:
				break;
		}

		/* 首期金額 Down Payment Amount (含小數 2位) */
		switch (inTransType)
		{
			case _ECR_8N1_INSTALLMENT_REFUND_NO_:		/* 分期退貨 */
			case _ECR_8N1_INSTALLMENT_ADJUST_NO_:		/* 分期調帳 */
			case _ECR_8N1_HG_REWARD_INSTALLMENT_NO_ :	/* 快樂購紅利積點 + 信用卡分期付款 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &srECROb->srTransData.szField_26[0], 10);	/* 12位數，這裡只取10位 */

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s :%s", "Down Payment Amount", szTemplate);
					inLogPrintf(AT, szDebugMsg);
				}

				pobTran->srBRec.lnInstallmentDownPayment = atol(szTemplate);

				break;
			default:
				break;
		}

		/* 每期金額 Installment Payment Amount (含小數 2位) */
		switch (inTransType)
		{
			case _ECR_8N1_INSTALLMENT_REFUND_NO_:		/* 分期退貨 */
			case _ECR_8N1_INSTALLMENT_ADJUST_NO_:		/* 分期調帳 */
			case _ECR_8N1_HG_REWARD_INSTALLMENT_NO_ :	/* 快樂購紅利積點 + 信用卡分期付款 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &srECROb->srTransData.szField_27[0], 10);	/* 12位數，這裡只取10位 */

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s :%s", "Installment Payment Amount", szTemplate);
					inLogPrintf(AT, szDebugMsg);
				}

				pobTran->srBRec.lnInstallmentPayment = atol(szTemplate);

				break;
			default:
				break;
		}

		/* 分期手續費 Formallity Fee (含小數 2位) */
		switch (inTransType)
		{
			case _ECR_8N1_INSTALLMENT_REFUND_NO_:		/* 分期退貨 */
			case _ECR_8N1_INSTALLMENT_ADJUST_NO_:		/* 分期調帳 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &srECROb->srTransData.szField_28[0], 10);	/* 12位數，這裡只取10位 */

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s :%s", "Formallity Fee", szTemplate);
					inLogPrintf(AT, szDebugMsg);
				}
				pobTran->srBRec.lnInstallmentFormalityFee = atol(szTemplate);
				
				if (pobTran->srBRec.lnInstallmentFormalityFee != 0)
				{
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_INST_FEE_NOT_0_;
					pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;		/* 操作錯誤 */
					return (VS_ERROR);
				}
				
				break;
			case _ECR_8N1_HG_REWARD_INSTALLMENT_NO_ :	/* 快樂購紅利積點 + 信用卡分期付款 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &srECROb->srTransData.szField_28[0], 10);	/* 12位數，這裡只取10位 */

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s :%s", "Formallity Fee", szTemplate);
					inLogPrintf(AT, szDebugMsg);
				}

				pobTran->srBRec.lnInstallmentFormalityFee = atol(szTemplate);

				break;
			default:
				break;
		}

		/* 只有金融卡才要收的欄位 */
		if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
		{
			/* 金融卡原交易日期 SP Origin Date */
			switch (inTransType)
			{
				case _ECR_8N1_REFUND_NO_:				/* 退費 */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memcpy(szTemplate, &srECROb->srTransData.szField_34[0], 8);

					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s :%s", "SP Origin Date", szTemplate);
						inLogPrintf(AT, szDebugMsg);
					}
					
					if (atoi(szTemplate) > 0	&&
					    inFunc_CheckValidDate_Include_Year(szTemplate) == VS_SUCCESS)
					{
						memcpy(pobTran->srBRec.szFiscRefundDate, szTemplate, 8);
					}
					
					break;
				default:
					break;
			}

			/* 金融卡調單編號 SP RRN */
			switch (inTransType)
			{
				case _ECR_8N1_REFUND_NO_:				/* 退費 */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memcpy(szTemplate, &srECROb->srTransData.szField_35[0], 12);

					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s :%s", "SP RRN", szTemplate);
						inLogPrintf(AT, szDebugMsg);
					}

					if (atoi(szTemplate) > 0)
					{
						memcpy(pobTran->srBRec.szFiscRRN, szTemplate, 12);
					}

					break;
				default:
					break;
			}

		}
		/* 只有電票要收的欄位 */
		else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
		{
			/* ESVC Origin Date */
			switch (inTransType)
			{
				case _ECR_8N1_REFUND_NO_:				/* 電票退貨 */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memcpy(szTemplate, &srECROb->srTransData.szField_34[0], 8);

					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s :%s", "ESVC Origin Date", szTemplate);
						inLogPrintf(AT, szDebugMsg);
					}

					if (atoi(szTemplate) > 0	&&
					    inFunc_CheckValidDate_Include_Year(szTemplate) == VS_SUCCESS)
					{
						memcpy(pobTran->srTRec.szTicketRefundDate, &szTemplate[4], 4);
					}
					else
					{
/* [20251219_BUG_MDF][ECR][DATA] 修改ECR欄位34資料,因電票資料如果為空白會拒絕，跳過檢查步驟,進行後續人工輸入 */
#ifdef _ECR_CHECK_FILE_34_	
						/* 接收資料錯誤 */
						memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
						inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
						memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
						sprintf(pobTran->szErrorMsgBuff2, "ESVC Origin Date Not Exist");
						pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
						
						return (VS_ERROR);
#endif
					}

					break;
				default:
					break;
			}
			
			/* ATS電票交易序號 RF NUMBER 左靠右補空白 */
			switch (inTransType)
			{
				case _ECR_8N1_REFUND_NO_:				/* 電票退貨 */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memcpy(szTemplate, &srECROb->srTransData.szField_35[0], 12);

					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s :%s", "RF NUMBER", szTemplate);
						inLogPrintf(AT, szDebugMsg);
					}
					
					memcpy(pobTran->srTRec.szTicketRefundCode, szTemplate, 12);
					

					break;
				default:
					break;
			}
		}
		/* 只有銀聯要收的欄位 */
		else if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
		{
			/* CUP Origin Date */
			switch (inTransType)
			{
				case _ECR_8N1_REFUND_NO_:				/* 電票退貨 */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memcpy(szTemplate, &srECROb->srTransData.szField_34[0], 8);

					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s :%s", "CUP Origin Date", szTemplate);
						inLogPrintf(AT, szDebugMsg);
					}

					if (atoi(szTemplate) > 0	&&
					    inFunc_CheckValidDate_Include_Year(szTemplate) == VS_SUCCESS)
					{
						memcpy(pobTran->srBRec.szCUP_TD, &szTemplate[4], 4);
					}
					else
					{
						/* 接收資料錯誤 */
						memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
						inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
						memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
						sprintf(pobTran->szErrorMsgBuff2, "CUP Origin Date Not Exist");
						pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
						
						return (VS_ERROR);
					}

					break;
				default:
					break;
			}
		}
		
	}
	
	/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/5/30 下午 1:30 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_075_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_) || 
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_103_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_))
	{
		switch (inTransType)
		{
			case _ECR_8N1_SALE_NO_ :
			case _ECR_8N1_REDEEM_NO_ :
			case _ECR_8N1_INSTALLMENT_NO_ :
				/* 'Y' = 昇恆昌聯名卡判斷 */
				if (srECROb->srTransData.szMPFlag[0] == 'Y')
					pobTran->uszCheck_CoBrand_CardBit = VS_TRUE;
				else
					pobTran->uszCheck_CoBrand_CardBit = VS_FALSE;
				break;
			default :
				pobTran->uszCheck_CoBrand_CardBit = VS_FALSE;
				break;
		}
	}
	
	/* 【需求單 - 104014】發卡機構on-us繳費平台需求 +【需求單 - 104152】發卡機構on-us繳費平台 Payitem Code by Russell 2020/3/16 下午 5:38 */
	memset(szPayItemFuncEnable, 0x00, sizeof(szPayItemFuncEnable));
	inGetPayItemEnable(szPayItemFuncEnable);
	if (memcmp(szPayItemFuncEnable, "Y", 1) == 0							&&
	    memcmp(&srECROb->srTransData.szTransType[0], _ECR_8N1_START_CARD_NO_INQUIRY_, 2) != 0)
	{
		/* PayItemFuncEnable有開才進入檢核收銀機送的繳費項目 */
		/* 兩段式收銀機連線 第一段不檢核Payitem Code */
		switch (inTransType)
		{
			case _ECR_8N1_SALE_NO_ :
			case _ECR_8N1_REFUND_NO_ :
			case _ECR_8N1_OFFLINE_NO_ :
			case _ECR_8N1_PREAUTH_NO_ :
			case _ECR_8N1_PREAUTH_COMPLETE_NO_ :
			case _ECR_8N1_INSTALLMENT_NO_ :
			case _ECR_8N1_INSTALLMENT_REFUND_NO_ :
			case _ECR_8N1_REDEEM_NO_ :
			case _ECR_8N1_REDEEM_REFUND_NO_ :
				srECROb->srTransData.uszECRResponsePayitem = VS_TRUE;
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, srECROb->srTransData.szPayItem, 5);

				/* 先判斷是不是送5個空白 */
				if (!memcmp(szTemplate, "     ", 5))
				{
					/* 空白不必回傳payitem */
					if (ginDebug == VS_TRUE)
					{
						srECROb->srTransData.uszECRResponsePayitem = VS_FALSE;
						inLogPrintf(AT, "ECR not input PayItem");
					}
					break;
				}

				/* 比對端末機的PIT Table PayItemCode */
				/* 吳升文(Angus Wu) 2015-09-15 (週二) 下午 04:43 回覆 帶5個0視為有值，所以要比對。 */
				for (i = 0 ;; i++)
				{
					/* 吳升文(Angus Wu) 2015-09-15 (週二) 下午 04:24 回覆
						因為繳費項目的代碼會從授權一直帶到清算再回到發卡行，然後發卡行根據繳費項目代碼跟持卡人收費。
						所以不合法的繳費代碼端末機不可以後送，免得後端勾稽不到而收不到錢。
						因此比對不到繳費代碼則端末機提示”不支援該繳費項目”後，回傳Response Code=0004。
					*/
					/* 比對不到不進入輸入keymap畫面 在ECR連線阻檔 */
					if (inLoadPITRec(i) < 0)
					{
						/* 不支援該繳費項目 */
						pobTran->inErrorMsg = _ERROR_CODE_V3_NOT_SUPPORT_THIS_PAY_ITEM_;
						pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
						
						return (VS_ERROR);
					}

					/* 有比對到端末機的繳費項目 */
					memset(szPayItemCodeTemp, 0x00, sizeof(szPayItemCodeTemp));
					inGetPayItemCode(szPayItemCodeTemp);
					if (memcmp(szTemplate, szPayItemCodeTemp, 5) == 0)
					{
						memset(pobTran->srBRec.szPayItemCode, 0x00, sizeof(pobTran->srBRec.szPayItemCode));
						memcpy(&pobTran->srBRec.szPayItemCode[0], &szTemplate[0], 5);
						break;
					}
				}

				break;
			default :
				break;
		}
	}
	
	/* 處理HG */
	/* 支付工具 */
	switch (inTransType)
	{
		case _ECR_8N1_HG_REWARD_SALE_NO_ :
		case _ECR_8N1_HG_ONLINE_REDEEMPTION_NO_ :
		case _ECR_8N1_HG_POINT_CERTAIN_NO_ :
		case _ECR_8N1_HG_REWARD_INSTALLMENT_NO_ :
		case _ECR_8N1_HG_REWARD_REDEMPTION_NO_ :
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "HG Payment Tool", srECROb->srTransData.szHGPaymentTool);
				inLogPrintf(AT, szDebugMsg);
			}
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(srECROb->srTransData.szHGPaymentTool) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "HG Payment Tool", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "HG Payment Tool Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

				return (VS_ERROR);
			}
			
			if (memcmp(srECROb->srTransData.szHGPaymentTool, _ECR_8N1_HG_PAYMENT_TOOL_TYPE_EDC_CHOOSE_, 2) == 0)
			{
				pobTran->srBRec.lnHGPaymentType = 0;
			}
			else if (memcmp(srECROb->srTransData.szHGPaymentTool, _ECR_8N1_HG_PAYMENT_TOOL_TYPE_CREDIT_CARD_, 2) == 0)
			{
				pobTran->srBRec.lnHGPaymentType = _HG_PAY_CREDIT_;
			}
			else if (memcmp(srECROb->srTransData.szHGPaymentTool, _ECR_8N1_HG_PAYMENT_TOOL_TYPE_CASH_, 2) == 0)
			{
				pobTran->srBRec.lnHGPaymentType = _HG_PAY_CASH_;
			}
			else if (memcmp(srECROb->srTransData.szHGPaymentTool, _ECR_8N1_HG_PAYMENT_TOOL_TYPE_GIFT_PAPER_, 2) == 0)
			{
				pobTran->srBRec.lnHGPaymentType = _HG_PAY_GIFT_PAPER_;
			}
			else if (memcmp(srECROb->srTransData.szHGPaymentTool, _ECR_8N1_HG_PAYMENT_TOOL_TYPE_HGI_, 2) == 0)
			{
				pobTran->srBRec.lnHGPaymentType = _HG_PAY_CREDIT_INSIDE_;
			}
			else if (memcmp(srECROb->srTransData.szHGPaymentTool, _ECR_8N1_HG_PAYMENT_TOOL_TYPE_CUP_CARD_, 2) == 0)
			{
				pobTran->srBRec.lnHGPaymentType = _HG_PAY_CUP_;
			}
			else
			{
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "HG Payment Tool Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
			}
			break;
		default :
			break;
	}
	
	/* Happy Go 扣抵點數 */
	switch (inTransType)
	{
		case _ECR_8N1_HG_POINT_CERTAIN_NO_ :
		case _ECR_8N1_HG_FULL_REDEEMPTION_NO_ :
		case _ECR_8N1_HG_REDEEM_REFUND_NO_ :
		case _ECR_8N1_HG_REWARD_REFUND_NO_ :
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "HG Redeem Point", srECROb->srTransData.szHGRedeemPoint);
				inLogPrintf(AT, szDebugMsg);
			}
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(srECROb->srTransData.szHGRedeemPoint) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "HG Redeem Point", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "HG Redeem Point Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

				return (VS_ERROR);
			}
			
			pobTran->srBRec.lnHGTransactionPoint = atol(srECROb->srTransData.szHGRedeemPoint);
			break;
		default :
			break;
	}
	
	if (gbBarCodeECRBit == VS_TRUE)
	{
		/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/7 上午 10:28 */
		/* 一維或二維條碼資料 */
		switch (inTransType)
		{
			case _ECR_8N1_END_CARD_NO_INQUIRY_NO_ :
			case _ECR_8N1_SALE_NO_:
			case _ECR_8N1_INSTALLMENT_NO_:
			case _ECR_8N1_REDEEM_NO_:
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s :%s", "BarCode Data Len", srECROb->srTransData.szBarCodeLen);
					inLogPrintf(AT, szDebugMsg);
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s :%s", "BarCode Data", srECROb->srTransData.szBarCodeData);
					inLogPrintf(AT, szDebugMsg);
				}

				/* 欄位為M 卻不存在，回傳錯誤 */
				if (inFunc_CheckFullSpace(srECROb->srTransData.szBarCodeLen) == VS_TRUE)
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s: %s  ", "BarCode Data Len", "Not Exist Error");
						inLogPrintf(AT, szDebugMsg);
					}
					memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
					inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "BarCode Data Len Error");
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

					return (VS_ERROR);
				}

				/* 欄位為M 卻不存在，回傳錯誤 */
				if (inNCCC_Func_Check_Uny_Format(srECROb->srTransData.szBarCodeData) == VS_FALSE)
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s: %s  ", "BarCode Data", "Not Uny Format");
						inLogPrintf(AT, szDebugMsg);
					}
					memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
					inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "Not Uny Format");
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

					return (VS_ERROR);
				}

				inBarCodeLen = atoi(srECROb->srTransData.szBarCodeLen);
				if (inBarCodeLen > 0)
				{
					if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
					{
						memcpy(pobTran->srBRec.szBarCodeData, srECROb->srTransData.szBarCodeData, inBarCodeLen);
					}
				}

				break;
			case _ECR_8N1_REFUND_NO_:
				if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s :%s", "BarCode Data Len", srECROb->srTransData.szBarCodeLen);
						inLogPrintf(AT, szDebugMsg);
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s :%s", "BarCode Data", srECROb->srTransData.szBarCodeData);
						inLogPrintf(AT, szDebugMsg);
					}

					/* 欄位為M 卻不存在，回傳錯誤 */
					if (inFunc_CheckFullSpace(srECROb->srTransData.szBarCodeLen) == VS_TRUE)
					{
						if (ginDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
							sprintf(szDebugMsg, "%s: %s  ", "BarCode Data Len", "Not Exist Error");
							inLogPrintf(AT, szDebugMsg);
						}
						memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
						inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
						memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
						sprintf(pobTran->szErrorMsgBuff2, "BarCode Data Len Error");
						pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

						return (VS_ERROR);
					}

					/* 欄位為M 卻不存在，回傳錯誤 */
					if (inNCCC_Func_Check_Uny_Format(srECROb->srTransData.szBarCodeData) == VS_FALSE)
					{
						if (ginDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
							sprintf(szDebugMsg, "%s: %s  ", "BarCode Data", "Not Uny Format");
							inLogPrintf(AT, szDebugMsg);
						}
						memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
						inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
						memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
						sprintf(pobTran->szErrorMsgBuff2, "Not Uny Format");
						pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

						return (VS_ERROR);
					}

					inBarCodeLen = atoi(srECROb->srTransData.szBarCodeLen);
					if (inBarCodeLen > 0)
					{
						if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
						{
							memcpy(pobTran->srBRec.szBarCodeData, srECROb->srTransData.szBarCodeData, inBarCodeLen);
						}
					}
				}
				else
				{
					/* 非銀聯退貨不用檢核 */
				}

				break;
			default :
				break;
		}
	
                if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
		{
                        /* Uny交易碼 */
                        switch (inTransType)
                        {
                                case _ECR_8N1_REFUND_NO_ :
                                case _ECR_8N1_INSTALLMENT_REFUND_NO_ :
                                case _ECR_8N1_REDEEM_REFUND_NO_ :
                                        if (ginDebug == VS_TRUE)
                                        {
                                                memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                                                sprintf(szDebugMsg, "%s :%s", "Uny Trans Code", srECROb->srTransData.szUnyTransCode);
                                                inLogPrintf(AT, szDebugMsg);
                                        }

                                        /* 欄位為M 卻不存在，回傳錯誤 */
                                        if (inFunc_CheckFullSpace(srECROb->srTransData.szUnyTransCode) == VS_TRUE)
                                        {
                                                if (ginDebug == VS_TRUE)
                                                {
                                                        memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                                                        sprintf(szDebugMsg, "%s: %s  ", "Uny Trans Code", "Not Exist Error");
                                                        inLogPrintf(AT, szDebugMsg);
                                                }
                                                memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
                                                inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
                                                memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
                                                sprintf(pobTran->szErrorMsgBuff2, "Uny Trans Code Error");
                                                pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

                                                return (VS_ERROR);
                                        }

                                        memcpy(pobTran->srBRec.szUnyTransCode, srECROb->srTransData.szUnyTransCode, 20);
                                        break;
                                default :
                                        break;
                        }
                }
	}
	
	if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
	{
		/* 有開CUP且MACEnable有開但安全認證沒過，不能執行CUP交易 */
		if (inKMS_CheckKey(_TWK_KEYSET_NCCC_, _TWK_KEYINDEX_NCCC_MAC_) != VS_SUCCESS)
		{
			if (inNCCC_Func_CUP_PowerOn_LogOn(pobTran) != VS_SUCCESS)
			{
				/* 安全認證失敗 */
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "安全認證失敗");
				}
				pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
				
				return (VS_ERROR);
			}
		}
	}
	
	if (inECR_Check_Exception(pobTran) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	memset(szFiscContactlessEnable, 0x00, sizeof(szFiscContactlessEnable));
	inGetSmartPayContactlessEnable(szFiscContactlessEnable);
	memset(szCTLSEnable, 0x00, sizeof(szCTLSEnable));
	inGetContactlessEnable(szCTLSEnable);
	memset(szCUPContactlessEnable, 0x00, sizeof(szCUPContactlessEnable));
	inGetCUPContactlessEnable(szCUPContactlessEnable);
	
	/* 跑OPT */
	switch (inTransType)
	{
		case _ECR_8N1_SALE_NO_:
			/* 金融卡和一般Sale跑不同OPT，所以要分 */
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				/* 第三層顯示 ＜消費扣款＞ */
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_FISC_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜消費扣款＞ */

				pobTran->inFunctionID = _FISC_SALE_;
				if (!memcmp(szCTLSEnable, "Y", 1)		&&
				    !memcmp(szFiscContactlessEnable, "Y", 1)	&&
				    guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_FISC_SALE_CTLS_;
					pobTran->inRunTRTID = _TRT_FISC_SALE_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_FISC_SALE_;
					pobTran->inRunTRTID = _TRT_FISC_SALE_ICC_;
				}
		
				pobTran->inTransactionCode = _FISC_SALE_;
				pobTran->srBRec.inCode = _FISC_SALE_;
				pobTran->srBRec.inOrgCode = _FISC_SALE_;
				
				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_CUP_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜銀聯一般交易＞ */

				if (!memcmp(szCTLSEnable, "Y", 1)		&&
				    !memcmp(szCUPContactlessEnable, "Y", 1)	&&
				    guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
					pobTran->inRunTRTID = _TRT_CUP_SALE_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
					pobTran->inRunTRTID = _TRT_CUP_SALE_;
				}

				pobTran->inFunctionID = _CUP_SALE_;
				pobTran->inTransactionCode = _CUP_SALE_;
				pobTran->srBRec.inCode = pobTran->inFunctionID;
				pobTran->srBRec.inOrgCode = pobTran->inFunctionID;
				
				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_TICKET_DEDUCT_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜購貨交易＞ */

				pobTran->inFunctionID = _TICKET_DEDUCT_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_TICKET_DEDUCT_;
				}
				else
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "感應未初始化，不能使用此功能");
					}
					return (VS_ERROR);
				}

				pobTran->inTransactionCode = _TICKET_DEDUCT_;
				pobTran->srTRec.inCode = _TICKET_DEDUCT_;
				
				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else
			{
				inDISP_ClearAll();
				/* 第三層顯示 ＜一般交易＞ */
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般交易＞ */

				pobTran->inFunctionID = _SALE_;
				
				/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/10 上午 10:32 */
				if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_SALE_BARCODE_;
					pobTran->inRunTRTID = _TRT_SALE_;
				}
				else if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
					pobTran->inRunTRTID = _TRT_SALE_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
					pobTran->inRunTRTID = _TRT_SALE_;
				}

				pobTran->inTransactionCode = _SALE_;
				pobTran->srBRec.inCode = _SALE_;
				pobTran->srBRec.inOrgCode = _SALE_;
				
				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			
			break;
		case _ECR_8N1_VOID_NO_:
			/* 金融卡和一般Sale跑不同OPT，所以要分 */
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_FISC_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜沖正交易＞ */

				pobTran->inFunctionID = _FISC_VOID_;
				pobTran->inRunOperationID = _OPERATION_VOID_;
				pobTran->inRunTRTID = _TRT_FISC_VOID_;

				pobTran->inTransactionCode = _FISC_VOID_;
				pobTran->srBRec.inCode = _FISC_VOID_;
				pobTran->srBRec.inOrgCode = _FISC_VOID_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜取消交易＞ */

				pobTran->inRunOperationID = _OPERATION_VOID_;
				pobTran->inRunTRTID = _TRT_CUP_VOID_;

				pobTran->inFunctionID = _CUP_VOID_;
				pobTran->inTransactionCode = _CUP_VOID_;
				pobTran->srBRec.inCode = pobTran->inTransactionCode;
				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜取消交易＞ */

				pobTran->inFunctionID = _VOID_;
				pobTran->inRunOperationID = _OPERATION_VOID_;
				pobTran->inRunTRTID = _TRT_VOID_;

				pobTran->inTransactionCode = _VOID_;
				pobTran->srBRec.inCode = _VOID_;
				pobTran->srBRec.inOrgCode = _VOID_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			break;
		case _ECR_8N1_REFUND_NO_:
			/* 金融卡和一般Sale跑不同OPT，所以要分 */
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_FISC_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);	/* 第二層顯示 ＜退費交易＞ */

				pobTran->inFunctionID = _FISC_REFUND_;
				if (!memcmp(szCTLSEnable, "Y", 1)		&&
				    !memcmp(szFiscContactlessEnable, "Y", 1)	&&
				    guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_FISC_REFUND_CTLS_;
					pobTran->inRunTRTID = _TRT_FISC_REFUND_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_FISC_REFUND_;
					pobTran->inRunTRTID = _TRT_FISC_REFUND_ICC_;
				}

				pobTran->inTransactionCode = _FISC_REFUND_;
				pobTran->srBRec.inCode = _FISC_REFUND_;
				pobTran->srBRec.inOrgCode = _FISC_REFUND_;
				
				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			else if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般退貨＞ */

				if (!memcmp(szCTLSEnable, "Y", 1)		&&
				    !memcmp(szCUPContactlessEnable, "Y", 1)	&&
				    guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_GET_CARD_FIRST_;
					pobTran->inRunTRTID = _TRT_CUP_REFUND_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_AMOUNT_FIRST_CUP_;
					pobTran->inRunTRTID = _TRT_CUP_REFUND_;
				}

				pobTran->inFunctionID = _CUP_REFUND_;
				pobTran->inTransactionCode = _CUP_REFUND_;
				pobTran->srBRec.inCode = pobTran->inTransactionCode;
				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
				
				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_TICKET_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜退貨交易＞ */

				pobTran->inFunctionID = _TICKET_REFUND_;
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_TICKET_REFUND_;
				}
				else
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "感應未初始化，不能使用此功能");
					}
					return (VS_ERROR);
				}

				pobTran->inTransactionCode = _TICKET_REFUND_;
				pobTran->srTRec.inCode = _TICKET_REFUND_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般退貨＞ */

				pobTran->inFunctionID = _REFUND_;
				/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/10 上午 10:32 */
				if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_BARCODE_;
				}
				else if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_GET_CARD_FIRST_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_AMOUNT_FIRST_;
				}
				pobTran->inRunTRTID = _TRT_REFUND_;

				pobTran->inTransactionCode = _REFUND_;
				pobTran->srBRec.inCode = _REFUND_;
				pobTran->srBRec.inOrgCode = _REFUND_;
				
				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;
			
		case _ECR_8N1_OFFLINE_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_SALE_OFFLINE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜交易補登＞ */
			
			pobTran->inFunctionID = _SALE_OFFLINE_;
			pobTran->inRunOperationID = _OPERATION_SALE_OFFLINE_;
			pobTran->inRunTRTID = _TRT_SALE_OFFLINE_;

			pobTran->inTransactionCode = _SALE_OFFLINE_;
			pobTran->srBRec.inCode = _SALE_OFFLINE_;
			pobTran->srBRec.inOrgCode = _SALE_OFFLINE_;
			
			/* 卡號查詢不過第二次卡，所以要直接決定TRT */
			if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
			{
				inNCCC_Func_Decide_CTLS_TRT(pobTran);
			}
			else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
			{
				inNCCC_Func_Decide_ICC_TRT(pobTran);
			}
			else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
			{
				inNCCC_Func_Decide_MEG_TRT(pobTran);
			}
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			break;
			
		case _ECR_8N1_PREAUTH_NO_:
			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_PRE_AUTH_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜預先授權＞ */

				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_PRE_AUTH_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_PRE_AUTH_;
				}
				pobTran->inRunTRTID = _TRT_CUP_PRE_AUTH_;

				pobTran->inFunctionID = _CUP_PRE_AUTH_;
				pobTran->inTransactionCode = _CUP_PRE_AUTH_;
				pobTran->srBRec.inCode = pobTran->inTransactionCode;
				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
				
				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_PRE_AUTH_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜預先授權＞ */

				pobTran->inFunctionID = _PRE_AUTH_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_PRE_AUTH_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_PRE_AUTH_;
				}
				pobTran->inRunTRTID = _TRT_PRE_AUTH_;

				pobTran->inTransactionCode = _PRE_AUTH_;
				pobTran->srBRec.inCode = _PRE_AUTH_;
				pobTran->srBRec.inOrgCode = _PRE_AUTH_;
				
				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			
			break;
			
		case _ECR_8N1_PREAUTH_CANCEL_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_PRE_AUTH_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜預先授權取消＞ */
			
			pobTran->inFunctionID = _CUP_PRE_AUTH_VOID_;
			pobTran->inRunOperationID = _OPERATION_VOID_;
			pobTran->inRunTRTID = _TRT_CUP_PRE_AUTH_VOID_;

			pobTran->inTransactionCode = _CUP_PRE_AUTH_VOID_;
			pobTran->srBRec.inCode = _CUP_PRE_AUTH_VOID_;
			pobTran->srBRec.inOrgCode = _CUP_PRE_AUTH_VOID_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			break;
			
		case _ECR_8N1_PREAUTH_COMPLETE_NO_:
			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_PRE_COMP_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜預先授權完成＞ */

				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_PRE_COMP_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_PRE_COMP_;
				}
				pobTran->inRunTRTID = _TRT_CUP_PRE_COMP_;

				pobTran->inFunctionID = _CUP_PRE_COMP_;
				pobTran->inTransactionCode = _CUP_PRE_COMP_;
				pobTran->srBRec.inCode = pobTran->inTransactionCode;
				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
				
				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_PRE_COMP_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜預先授權完成＞ */

				pobTran->inFunctionID = _PRE_COMP_;
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_PRE_COMP_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_PRE_COMP_;
				}
				pobTran->inRunTRTID = _TRT_PRE_COMP_;

				pobTran->inTransactionCode = _PRE_COMP_;
				pobTran->srBRec.inCode = _PRE_COMP_;
				pobTran->srBRec.inOrgCode = _PRE_COMP_;
				
				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;
			
		case _ECR_8N1_PREAUTH_COMPLETE_CANCEL_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);					/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_PRE_COMP_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜預先授權完成取消＞ */
			
			pobTran->inFunctionID = _CUP_PRE_COMP_VOID_;
			pobTran->inRunOperationID = _OPERATION_VOID_;
			pobTran->inRunTRTID = _TRT_CUP_PRE_COMP_VOID_;

			pobTran->inTransactionCode = _CUP_PRE_COMP_VOID_;
			pobTran->srBRec.inCode = _CUP_PRE_COMP_VOID_;
			pobTran->srBRec.inOrgCode = _CUP_PRE_COMP_VOID_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			break;
			
		case _ECR_8N1_INSTALLMENT_NO_:
//			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
//			{
//				inDISP_ClearAll();
//				/* 保留給銀聯分期使用 */
//				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
//				inDISP_PutGraphic(_MENU_INST_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜分期付款＞ */
//
//				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
//				{
//					pobTran->inRunOperationID = _OPERATION_INST_SALE_CTLS_;
//				}
//				else
//				{
//					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
//				}
//				pobTran->inRunTRTID = _TRT_CUP_INST_SALE_;
//
//				pobTran->inFunctionID = _CUP_INST_SALE_;
//				pobTran->inTransactionCode = _CUP_INST_SALE_;
//				pobTran->srBRec.inCode = pobTran->inTransactionCode;
//				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
//
//				pobTran->srBRec.uszInstallmentBit = VS_TRUE;
//				
//				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
//				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
//				{
//					inNCCC_Func_Decide_CTLS_TRT(pobTran);
//				}
//				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
//				{
//					inNCCC_Func_Decide_ICC_TRT(pobTran);
//				}
//				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
//				{
//					inNCCC_Func_Decide_MEG_TRT(pobTran);
//				}
//
//				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
//			}
//			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_INST_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜分期付款＞ */

				pobTran->inFunctionID = _INST_SALE_;
				
				/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/10 上午 10:32 */
				if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_INST_SALE_BARCODE_;
				}
				else if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_INST_SALE_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
				}
				pobTran->inRunTRTID = _TRT_INST_SALE_;

				pobTran->inTransactionCode = _INST_SALE_;
				pobTran->srBRec.inCode = _INST_SALE_;
				pobTran->srBRec.inOrgCode = _INST_SALE_;

				pobTran->srBRec.uszInstallmentBit = VS_TRUE;
				
				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;
			
		case _ECR_8N1_INSTALLMENT_REFUND_NO_:
//			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
//			{
//				inDISP_ClearAll();
//				/* 保留給銀聯分期退貨使用 */
//				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
//				inDISP_PutGraphic(_MENU_INST_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜分期退貨＞ */
//
//				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
//				{
//					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_GET_CARD_FIRST_;
//				}
//				else
//				{
//					pobTran->inRunOperationID = _OPERATION_REFUND_;
//				}
//				pobTran->inRunTRTID = _TRT_CUP_INST_REFUND_;
//
//				pobTran->inFunctionID = _CUP_INST_REFUND_;
//				pobTran->inTransactionCode = _CUP_INST_REFUND_;
//				pobTran->srBRec.inCode = pobTran->inTransactionCode;
//				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
//
//				pobTran->srBRec.uszInstallmentBit = VS_TRUE;
//				
//				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
//				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
//				{
//					inNCCC_Func_Decide_CTLS_TRT(pobTran);
//				}
//				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
//				{
//					inNCCC_Func_Decide_ICC_TRT(pobTran);
//				}
//				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
//				{
//					inNCCC_Func_Decide_MEG_TRT(pobTran);
//				}
//
//				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
//			}
//			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_INST_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜分期退貨＞ */

				pobTran->inFunctionID = _INST_REFUND_;
				
				/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/10 上午 10:32 */
				if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_INST_REFUND_BARCODE_;
				}
				else if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_GET_CARD_FIRST_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_;
				}
				pobTran->inRunTRTID = _TRT_INST_REFUND_;

				pobTran->inTransactionCode = _INST_REFUND_;
				pobTran->srBRec.inCode = _INST_REFUND_;
				pobTran->srBRec.inOrgCode = _INST_REFUND_;

				pobTran->srBRec.uszInstallmentBit = VS_TRUE;
				
				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;
			
		case _ECR_8N1_INSTALLMENT_ADJUST_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_INST_ADJUST_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜分期調帳＞ */
			
			pobTran->inFunctionID = _INST_ADJUST_;
			pobTran->inRunOperationID = _OPERATION_INST_ADJUST_;
			pobTran->inRunTRTID = _TRT_INST_ADJUST_;

			pobTran->inTransactionCode = _INST_ADJUST_;
			pobTran->srBRec.inCode = _INST_ADJUST_;
			pobTran->srBRec.inOrgCode = _INST_ADJUST_;
			
			pobTran->srBRec.uszInstallmentBit = VS_TRUE;
			
			/* 卡號查詢不過第二次卡，所以要直接決定TRT */
			if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
			{
				inNCCC_Func_Decide_CTLS_TRT(pobTran);
			}
			else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
			{
				inNCCC_Func_Decide_ICC_TRT(pobTran);
			}
			else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
			{
				inNCCC_Func_Decide_MEG_TRT(pobTran);
			}
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			break;
			
		case _ECR_8N1_REDEEM_NO_:
//			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
//			{
//				inDISP_ClearAll();
//				/* 保留給銀聯紅利扣抵使用 */
//				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
//				inDISP_PutGraphic(_MENU_REDEEM_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利扣抵＞ */
//
//				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
//				{
//					pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
//				}
//				else
//				{
//					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
//				}
//				pobTran->inRunTRTID = _TRT_REDEEM_SALE_;
//
//				pobTran->inFunctionID = _CUP_REDEEM_SALE_;
//				pobTran->inTransactionCode = _CUP_REDEEM_SALE_;
//				pobTran->srBRec.inCode = pobTran->inTransactionCode;
//				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
//
//				pobTran->srBRec.uszRedeemBit = VS_TRUE;
//				
//				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
//				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
//				{
//					inNCCC_Func_Decide_CTLS_TRT(pobTran);
//				}
//				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
//				{
//					inNCCC_Func_Decide_ICC_TRT(pobTran);
//				}
//				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
//				{
//					inNCCC_Func_Decide_MEG_TRT(pobTran);
//				}
//
//				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
//			}
//			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REDEEM_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利扣抵＞ */

				pobTran->inFunctionID = _REDEEM_SALE_;
				
				/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/10 上午 10:32 */
				if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REDEEM_SALE_BARCODE_;
				}
				else if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REDEEM_SALE_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
				}
				pobTran->inRunTRTID = _TRT_REDEEM_SALE_;

				pobTran->inTransactionCode = _REDEEM_SALE_;
				pobTran->srBRec.inCode = _REDEEM_SALE_;
				pobTran->srBRec.inOrgCode = _REDEEM_SALE_;

				pobTran->srBRec.uszRedeemBit = VS_TRUE;
				
				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;
		
		case _ECR_8N1_REDEEM_REFUND_NO_:
//			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
//			{
//				inDISP_ClearAll();
//				/* 保留給銀聯紅利退貨使用 */
//				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
//				inDISP_PutGraphic(_MENU_REDEEM_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利退貨＞ */
//
//				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
//				{
//					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_GET_CARD_FIRST_;
//				}
//				else
//				{
//					pobTran->inRunOperationID = _OPERATION_REFUND_;
//				}
//				pobTran->inRunTRTID = _TRT_CUP_REDEEM_REFUND_;
//
//				pobTran->inFunctionID = _CUP_REDEEM_REFUND_;
//				pobTran->inTransactionCode = _CUP_REDEEM_REFUND_;
//				pobTran->srBRec.inCode = pobTran->inTransactionCode;
//				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
//
//				pobTran->srBRec.uszRedeemBit = VS_TRUE;
//				
//				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
//				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
//				{
//					inNCCC_Func_Decide_CTLS_TRT(pobTran);
//				}
//				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
//				{
//					inNCCC_Func_Decide_ICC_TRT(pobTran);
//				}
//				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
//				{
//					inNCCC_Func_Decide_MEG_TRT(pobTran);
//				}
//
//				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
//			}
//			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REDEEM_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利退貨＞ */

				pobTran->inFunctionID = _REDEEM_REFUND_;
				
				/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/10 上午 10:32 */
				if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REDEEM_REFUND_BARCODE_;
				}
				else if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_GET_CARD_FIRST_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_;
				}
				pobTran->inRunTRTID = _TRT_REDEEM_REFUND_;

				pobTran->inTransactionCode = _REDEEM_REFUND_;
				pobTran->srBRec.inCode = _REDEEM_REFUND_;
				pobTran->srBRec.inOrgCode = _REDEEM_REFUND_;

				pobTran->srBRec.uszRedeemBit = VS_TRUE;
				
				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;
			
		case _ECR_8N1_REDEEM_ADJUST_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_REDEEM_ADJUST_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利調帳＞ */
			
			pobTran->inFunctionID = _REDEEM_ADJUST_;
			pobTran->inRunOperationID = _OPERATION_REDEEM_ADJUST_;
			pobTran->inRunTRTID = _TRT_REDEEM_ADJUST_;

			pobTran->inTransactionCode = _REDEEM_ADJUST_;
			pobTran->srBRec.inCode = _REDEEM_ADJUST_;
			pobTran->srBRec.inOrgCode = _REDEEM_ADJUST_;
			
			pobTran->srBRec.uszRedeemBit = VS_TRUE;
			
			/* 卡號查詢不過第二次卡，所以要直接決定TRT */
			if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
			{
				inNCCC_Func_Decide_CTLS_TRT(pobTran);
			}
			else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
			{
				inNCCC_Func_Decide_ICC_TRT(pobTran);
			}
			else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
			{
				inNCCC_Func_Decide_MEG_TRT(pobTran);
			}
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
			
		case _ECR_8N1_TIP_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_TIP_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜小費交易＞ */
			
			pobTran->inFunctionID = _TIP_;
			pobTran->inRunOperationID = _OPERATION_TIP_;
			pobTran->inRunTRTID = _TRT_TIP_;

			pobTran->inTransactionCode = _TIP_;
			pobTran->srBRec.inCode = _TIP_;
			pobTran->srBRec.inOrgCode = _TIP_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
			
		case _ECR_8N1_AWARD_REDEEM_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_LOYALTY_REDEEM_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜優惠兌換＞ */
			
			pobTran->inFunctionID = _LOYALTY_REDEEM_;
			/* 收銀機以條碼兌換*/
			if (pobTran->szL3_AwardWay[0] == '1')
			{
				pobTran->inRunOperationID = _OPERATION_BARCODE_;
			}
			else
			{
				pobTran->inRunOperationID = _OPERATION_LOYALTY_REDEEM_CTLS_;
			}
			pobTran->inRunTRTID = _TRT_LOYALTY_REDEEM_;

			pobTran->inTransactionCode = _LOYALTY_REDEEM_;
			pobTran->srBRec.inCode = _LOYALTY_REDEEM_;
			pobTran->srBRec.inOrgCode = _LOYALTY_REDEEM_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
			
		case _ECR_8N1_VOID_AWARD_REDEEM_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_VOID_LOYALTY_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜兌換取消＞ */
			
			pobTran->inFunctionID = _VOID_LOYALTY_REDEEM_;
			pobTran->inRunOperationID = _OPERATION_VOID_LOYALTY_REDEEM_;
			pobTran->inRunTRTID = _TRT_LOYALTY_REDEEM_;

			pobTran->inTransactionCode = _VOID_LOYALTY_REDEEM_;
			pobTran->srBRec.inCode = _VOID_LOYALTY_REDEEM_;
			pobTran->srBRec.inOrgCode = _VOID_LOYALTY_REDEEM_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;


		/* 快樂購紅利積點 */
		case _ECR_8N1_HG_REWARD_SALE_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡紅利積點＞ */
			
			pobTran->inFunctionID = _SALE_;
			pobTran->inRunOperationID = _OPERATION_HG_;
			pobTran->inRunTRTID = _TRT_HG_REWARD_CREDIT_CTLS_;

			/* 這邊先設成sale，若之後支付方式選銀聯卡，會在那邊改成CUP_SALE */
			pobTran->inTransactionCode = _SALE_;
			pobTran->srBRec.inCode = _SALE_;
			pobTran->srBRec.inOrgCode = _SALE_;
			pobTran->srBRec.lnHGTransactionType = _HG_REWARD_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		
		/* 快樂購紅利積點─分期付款 */
		case _ECR_8N1_HG_REWARD_INSTALLMENT_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_INS_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡分期付款＞ */
			
			pobTran->inFunctionID = _INST_SALE_;
			pobTran->inRunOperationID = _OPERATION_HG_I_R_;
			pobTran->inRunTRTID = _TRT_HG_REWARD_INSTALLMENT_CTLS_;

			pobTran->inTransactionCode = _INST_SALE_;
			pobTran->srBRec.inCode = _INST_SALE_;
			pobTran->srBRec.inOrgCode = _INST_SALE_;
			pobTran->srBRec.lnHGTransactionType = _HG_REWARD_;
			pobTran->srBRec.uszInstallmentBit = VS_TRUE;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		
		/* 快樂購紅利積點─紅利扣抵 */
		case _ECR_8N1_HG_REWARD_REDEMPTION_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_REDEEM_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡紅利扣抵＞ */
			
			pobTran->inFunctionID = _REDEEM_SALE_;
			pobTran->inRunOperationID = _OPERATION_HG_I_R_;
			pobTran->inRunTRTID = _TRT_HG_REWARD_REDEMPTION_CTLS_;

			pobTran->inTransactionCode = _REDEEM_SALE_;
			pobTran->srBRec.inCode = _REDEEM_SALE_;
			pobTran->srBRec.inOrgCode = _REDEEM_SALE_;
			pobTran->srBRec.lnHGTransactionType = _HG_REWARD_;
			pobTran->srBRec.uszRedeemBit = VS_TRUE;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
			
		/* 快樂購點數扣抵 */
		case _ECR_8N1_HG_ONLINE_REDEEMPTION_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_ONLINE_REDEEM_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡點數扣抵＞ */
			
			pobTran->inFunctionID = _SALE_;
			pobTran->inRunOperationID = _OPERATION_HG_;
			pobTran->inRunTRTID = _TRT_HG_ONLINE_REDEEM_CREDIT_CTLS_;

			pobTran->inTransactionCode = _SALE_;
			pobTran->srBRec.inCode = _SALE_;
			pobTran->srBRec.inOrgCode = _SALE_;
			pobTran->srBRec.lnHGTransactionType = _HG_ONLINE_REDEEM_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		
		/* 快樂購加價購 */
		case _ECR_8N1_HG_POINT_CERTAIN_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_POINT_CERTAIN_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡加價購＞ */
			
			pobTran->inFunctionID = _SALE_;
			pobTran->inRunOperationID = _OPERATION_HG_;
			pobTran->inRunTRTID = _TRT_HG_POINT_CERTAIN_CREDIT_CTLS_;

			pobTran->inTransactionCode = _SALE_;
			pobTran->srBRec.inCode = _SALE_;
			pobTran->srBRec.inOrgCode = _SALE_;
			pobTran->srBRec.lnHGTransactionType = _HG_POINT_CERTAIN_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		
		/* 快樂購點數兌換 */
		case _ECR_8N1_HG_FULL_REDEEMPTION_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_REDEMPTION_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡點數兌換＞ */
			
			pobTran->inFunctionID = _SALE_;
			pobTran->inRunOperationID = _OPERATION_HG_;
			pobTran->inRunTRTID = _TRT_HG_FULL_REDEMPTION_;

			pobTran->inTransactionCode = _SALE_;
			pobTran->srBRec.inCode = _SALE_;
			pobTran->srBRec.inOrgCode = _SALE_;
			pobTran->srBRec.lnHGTransactionType = _HG_FULL_REDEMPTION_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		
		/* 快樂購扣抵退貨 */
		case _ECR_8N1_HG_REDEEM_REFUND_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_REDEEM_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡扣抵退貨＞ */
			
			pobTran->inFunctionID = _HG_REDEEM_REFUND_;
			pobTran->inRunOperationID = _OPERATION_HG_REFUND_;
			pobTran->inRunTRTID = _TRT_HG_REDEEM_REFUND_;

			pobTran->inTransactionCode = _HG_REDEEM_REFUND_;
			pobTran->srBRec.inCode = _HG_REDEEM_REFUND_;
			pobTran->srBRec.inOrgCode = _HG_REDEEM_REFUND_;
			pobTran->srBRec.lnHGTransactionType = _HG_REDEEM_REFUND_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		
		/* 快樂購回饋退貨 */
		case _ECR_8N1_HG_REWARD_REFUND_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡回饋退貨＞ */
			
			pobTran->inFunctionID = _HG_REWARD_REFUND_;
			pobTran->inRunOperationID = _OPERATION_HG_REFUND_;
			pobTran->inRunTRTID = _TRT_HG_REWARD_REFUND_;

			pobTran->inTransactionCode = _HG_REWARD_REFUND_;
			pobTran->srBRec.inCode = _HG_REWARD_REFUND_;
			pobTran->srBRec.inOrgCode = _HG_REWARD_REFUND_;
			pobTran->srBRec.lnHGTransactionType = _HG_REWARD_REFUND_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		
		/* 快樂購點數查詢 */
		case _ECR_8N1_HG_POINT_INQUIRY_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_INQUIRY_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡點數查詢＞ */
			
			pobTran->inFunctionID = _HG_INQUIRY_;
			pobTran->inRunOperationID = _OPERATION_HG_;
			pobTran->inRunTRTID = _TRT_HG_INQUIRY_;

			pobTran->inTransactionCode = _HG_INQUIRY_;
			pobTran->srBRec.inCode = _HG_INQUIRY_;
			pobTran->srBRec.inOrgCode = _HG_INQUIRY_;
			pobTran->srBRec.lnHGTransactionType = _HG_INQUIRY_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
			
		case _ECR_8N1_SETTLEMENT_NO_:
			inDISP_ClearAll();
			/* 第三層顯示 ＜結帳交易＞ */
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_SETTLE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜結帳交易＞ */

			pobTran->inFunctionID = _SETTLE_;
			pobTran->inRunOperationID = _OPERATION_SETTLE_;
			pobTran->inRunTRTID = _TRT_SETTLE_;
			
			pobTran->inTransactionCode = _SETTLE_;
			pobTran->srBRec.inCode = _SETTLE_;
			pobTran->srBRec.inOrgCode = _SETTLE_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		
		case _ECR_8N1_REPRINT_RECEIPT_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_REPRINT_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜重印帳單＞ */

			pobTran->inFunctionID = FALSE;
			pobTran->inRunOperationID = _OPERATION_REPRINT_;
			pobTran->inRunTRTID = FALSE;
			
			pobTran->inTransactionCode = FALSE;
			pobTran->srBRec.inCode = FALSE;
			pobTran->srBRec.inOrgCode = FALSE;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
				
		case _ECR_8N1_MENU_REVIEW_DETAIL_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_REVIEW_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 <交易查詢> */
			
			pobTran->inFunctionID = FALSE;
			pobTran->inRunOperationID = _OPERATION_REVIEW_DETAIL_;
			pobTran->inRunTRTID = FALSE;
			
			pobTran->inTransactionCode = FALSE;
			pobTran->srBRec.inCode = FALSE;
			pobTran->srBRec.inOrgCode = FALSE;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		
		case _ECR_8N1_MENU_REVIEW_TOTAL_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_REVIEW_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜總額查詢＞ */
			
			pobTran->inFunctionID = FALSE;
			pobTran->inRunOperationID = _OPERATION_REVIEW_TOTAL_;
			pobTran->inRunTRTID = FALSE;
			
			pobTran->inTransactionCode = FALSE;
			pobTran->srBRec.inCode = FALSE;
			pobTran->srBRec.inOrgCode = FALSE;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
			
		case _ECR_8N1_MENU_REPORT_DETAIL_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_DETAIL_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 明細列印 */
			
			pobTran->inFunctionID = FALSE;
			pobTran->inRunOperationID = _OPERATION_DETAIL_REPORT_;
			pobTran->inRunTRTID = FALSE;
			
			pobTran->inTransactionCode = FALSE;
			pobTran->srBRec.inCode = FALSE;
			pobTran->srBRec.inOrgCode = FALSE;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
			
		case _ECR_8N1_MENU_REPORT_TOTAL_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_TOTAIL_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 總額列印 */
			
			pobTran->inFunctionID = FALSE;
			pobTran->inRunOperationID = _OPERATION_TOTAL_REPORT_;
			pobTran->inRunTRTID = FALSE;
			
			pobTran->inTransactionCode = FALSE;
			pobTran->srBRec.inCode = FALSE;
			pobTran->srBRec.inOrgCode = FALSE;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		case _ECR_8N1_EDC_REBOOT_NO_:
			inDISP_ClearAll();
			/* 第三層顯示 ＜重新開機＞ */
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_REBOOT_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜重新開機＞ */
			
			pobTran->inFunctionID = FALSE;
			pobTran->inRunOperationID = _OPERATION_EDC_REBOOT_;
			pobTran->inRunTRTID = FALSE;
			
			pobTran->inTransactionCode = FALSE;
			pobTran->srBRec.inCode = FALSE;
			pobTran->srBRec.inOrgCode = FALSE;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			
			break;
		case _ECR_8N1_ESVC_TOP_UP_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_TICKET_TOP_UP_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜加值交易＞ */

			pobTran->inFunctionID = _TICKET_TOP_UP_;

			if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
			{
				pobTran->inRunOperationID = _OPERATION_TICKET_TOP_UP_;
			}
			else
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "感應未初始化，不能使用此功能");
				}
				return (VS_ERROR);
			}

			pobTran->inTransactionCode = _TICKET_TOP_UP_;
			pobTran->srTRec.inCode = _TICKET_TOP_UP_;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		case _ECR_8N1_ESVC_BALANCE_INQUIRY_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0, _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_TICKET_INQUIRY_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜餘額查詢＞ */

			pobTran->inFunctionID = _TICKET_INQUIRY_;

			if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
			{
				pobTran->inRunOperationID = _OPERATION_TICKET_INQUIRY_;
			}
			else
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "感應未初始化，不能使用此功能");
				}
				return (VS_ERROR);
			}

			pobTran->inTransactionCode = _TICKET_INQUIRY_;
			pobTran->srTRec.inCode = _TICKET_INQUIRY_;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		case _ECR_8N1_ESVC_VOID_TOP_UP_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);					/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_TICKET_VOID_TOP_UP_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜加值取消＞ */

			pobTran->inFunctionID = _TICKET_VOID_TOP_UP_;

			if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
			{
				pobTran->inRunOperationID = _OPERATION_TICKET_VOID_TOP_UP_;
			}
			else
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "感應未初始化，不能使用此功能");
				}
				return (VS_ERROR);
			}

			pobTran->inTransactionCode = _TICKET_VOID_TOP_UP_;
			pobTran->srTRec.inCode = _TICKET_VOID_TOP_UP_;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		/* 查詢上一筆 */
		case _ECR_8N1_INQUIRY_LAST_TRANSACTION_NO_:
			inRetVal = VS_SUCCESS;
			break;
		case _ECR_8N1_ECHO_NO_:
			inNCCC_Func_Disp_EchoTest();
			inRetVal = VS_SUCCESS;
			break;
		default:
			pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
			inRetVal = VS_ERROR;
			break;
	}
	
	return (inRetVal);
}

/*
Function        :inECR_8N1_Customer_005_FPG_Unpack
Date&Time       :2022/9/22 下午 4:29
Describe        :台塑生醫 長度480
*/
int inECR_8N1_Customer_005_FPG_Unpack(TRANSACTION_OBJECT *pobTran, ECR_TABLE* srECROb, char *szDataBuffer)
{
	int	i = 0;
	int	inRetVal = VS_SUCCESS;
	int	inTransType = 0;		/* 電文中的交易別字串轉為數字儲存 */
	int	inCardInquiryTRTType = _CARD_INQUIRY_TYPE_NONE_;
	int	inSize = 0;
	char	szTemplate[100 + 1] = {0};
	char	szDebugMsg[100 + 1] = {0};	
	char	szHostLabel[8 + 1] = {0};
	char	szHostEnable[2 + 1] = {0};
	char	szBatchNum[6 + 1] = {0};
	char	szCTLSEnable[2 + 1] = {0};
	char	szFiscContactlessEnable[1 + 1] = {0};
	char	szCUPContactlessEnable[1 + 1] = {0};
	char	szGetFPG_FTC_FEID[8 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
	
        /* (需求單 - 107276)自助交易標準做法 EDC於發動交易及完成時，需發出提示音 by Russell 2019/3/8 上午 10:46 */
        inDISP_BEEP(1, 0);
	
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
        inGetCustomIndicator(szCustomerIndicator);
	
	/* 存ECR原始資料 */
	memcpy(srECROb->srTransData.szOrgData, szDataBuffer, inSize);
	
	inECR_8N1_Customer_005_FPG_Parse_Data(srECROb, szDataBuffer);

	/* Trans Type (交易別) */
	switch (inTransType)
	{
		default:			
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, srECROb->srTransData.szTransType, 2);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s: %s  ", "Trans Type", szTemplate);
				inLogPrintf(AT, szDebugMsg);
			}

			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Type", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "Trans Type Not Exist Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

				return (VS_ERROR);
			}

			/* 交易類別轉成數字 */
			inTransType = atoi(szTemplate);

			/* 要連動結帳 */
			if (inTransType == _ECR_8N1_SETTLEMENT_NO_)
			{
				pobTran->uszAutoSettleBit = VS_TRUE;
			}

			break;
	}
	
	/* 若前一次為卡號查詢，標記為第二次ECR */
	if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
	{
		pobTran->uszCardInquiryFirstBit = VS_FALSE;
		pobTran->uszCardInquirysSecondBit = VS_TRUE;
		/* 重新決定TRT流程 */
                if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
                {
                        /* 金融卡查詢要二次過卡，不依循前次查詢界面 */
                        inCardInquiryTRTType = _CARD_INQUIRY_TYPE_NONE_;
                }
                else
                {
                        if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
                        {
                                inCardInquiryTRTType = _CARD_INQUIRY_TYPE_CHIP_;
                        }
                        else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
                        {
                                inCardInquiryTRTType = _CARD_INQUIRY_TYPE_CTLS_;
                        }
                        else
                        {
                                inCardInquiryTRTType = _CARD_INQUIRY_TYPE_MEG_;
                        }
                }
	}
	
	/* 最後一段只吃正向交易 */
	if (inTransType == _ECR_8N1_START_CARD_NO_INQUIRY_NO_)
	{
		if (pobTran->uszCardInquirysSecondBit == VS_TRUE)
		{

		}
		else
		{
			pobTran->uszCardInquiryFirstBit = VS_TRUE;
		}
	}
	
	/* 銀行別/付款別 HOST ID*/
	switch (inTransType)
	{
		case _ECR_8N1_FPG_FTC_REPRINT_COUPON_NO_:
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, srECROb->srTransData.szHostID, 2);
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "HOST ID", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "HOST ID Not Exist Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
				return (VS_ERROR);
			}
			
			memset(szHostLabel, 0x00, sizeof(szHostLabel));

			if (memcmp(szTemplate, _ECR_8N1_NCCC_HOSTID_CUS_005_FTC_HG_, 2) == 0)
			{
				memcpy(szHostLabel, _HOST_NAME_HG_, strlen(_HOST_NAME_HG_));
			}
			else
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "HOST要HG");
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "%s : HOST要HG", szTemplate);
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
					
				return (VS_ERROR);
			}
			
			/* 初始化 */
			i = 0;
			do
			{
				/* 按順序load每一個HOST */
				if (inLoadHDTRec(i) < 0)
				{
					/* 如果return VS_ERROR 代表table沒有該HOST */
					memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
					inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "%s : HOST ID Not Found", szHostLabel);
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
					
					return (VS_ERROR);
				}

				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetHostLabel(szTemplate);
				if (!memcmp(szTemplate, szHostLabel, strlen(szHostLabel)))
				{
					/* 比對成功後，判斷HOST是否開啟  */
					memset(szHostEnable, 0x00, sizeof(szHostEnable));
					inGetHostEnable(szHostEnable);
					if (szHostEnable[0] != 'Y')
					{
						/* Host沒開 */
						if (ginDebug == VS_TRUE) 
						{
							memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
							sprintf(szDebugMsg, "%s Not Open Error", szHostLabel);
							inLogPrintf(AT, szDebugMsg);
						}
						
						memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
						inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
						memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
						sprintf(pobTran->szErrorMsgBuff2, "%s : HOST ID Not Open", szHostLabel);
						pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
					
						/* 如果return VS_ERROR 代表Host沒開 */
						return (VS_ERROR);
					}
					else
					{
						/* Host有開 */
						pobTran->srBRec.inHDTIndex = i;
						
						/* Load HDPT */
						inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
						memset(szBatchNum, 0x00, sizeof(szBatchNum));
						inGetBatchNum(szBatchNum);
						pobTran->srBRec.lnBatchNum = atol(szBatchNum);
						
						if (ginDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
							sprintf(szDebugMsg, "%s Open", szHostLabel);
							inLogPrintf(AT, szDebugMsg);
						}
						
					}/* Host Enable比對End */

					break;
				}/* Host Label比對 End */
				i++;

			} while (1);
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%d", "HDTindex", pobTran->srBRec.inHDTIndex);
				inLogPrintf(AT, szDebugMsg);
			}
			
			break;
		default:
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, srECROb->srTransData.szHostID, 2);
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "HOST ID", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "HOST ID Not Exist Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
				return (VS_ERROR);
			}
			
			memset(szHostLabel, 0x00, sizeof(szHostLabel));

			if (memcmp(szTemplate, _ECR_8N1_NCCC_HOSTID_NCCC_, 2) == 0)
			{
				memcpy(szHostLabel, _HOST_NAME_CREDIT_NCCC_, strlen(_HOST_NAME_CREDIT_NCCC_));
			}
			else if (memcmp(szTemplate, _ECR_8N1_NCCC_HOSTID_DCC_, 2) == 0)
			{
				memcpy(szHostLabel, _HOST_NAME_DCC_, strlen(_HOST_NAME_DCC_));
			}
			else if (memcmp(szTemplate, _ECR_8N1_NCCC_HOSTID_CUS_005_FTC_HG_, 2) == 0)
			{
				memcpy(szHostLabel, _HOST_NAME_HG_, strlen(_HOST_NAME_HG_));
			}
			else
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "無對應Host ID");
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "%s : HOST ID Not Pair", szTemplate);
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
					
				return (VS_ERROR);
			}
			
			/* 初始化 */
			i = 0;
			do
			{
				/* 按順序load每一個HOST */
				if (inLoadHDTRec(i) < 0)
				{
					/* 如果return VS_ERROR 代表table沒有該HOST */
					memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
					inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "%s : HOST ID Not Found", szHostLabel);
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
					
					return (VS_ERROR);
				}

				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetHostLabel(szTemplate);
				if (!memcmp(szTemplate, szHostLabel, strlen(szHostLabel)))
				{
					/* 比對成功後，判斷HOST是否開啟  */
					memset(szHostEnable, 0x00, sizeof(szHostEnable));
					inGetHostEnable(szHostEnable);
					if (szHostEnable[0] != 'Y')
					{
						/* Host沒開 */
						if (ginDebug == VS_TRUE) 
						{
							memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
							sprintf(szDebugMsg, "%s Not Open Error", szHostLabel);
							inLogPrintf(AT, szDebugMsg);
						}
						
						memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
						inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
						memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
						sprintf(pobTran->szErrorMsgBuff2, "%s : HOST ID Not Open", szHostLabel);
						pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
					
						/* 如果return VS_ERROR 代表Host沒開 */
						return (VS_ERROR);
					}
					else
					{
						/* Host有開 */
						pobTran->srBRec.inHDTIndex = i;
						
						/* Load HDPT */
						inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
						memset(szBatchNum, 0x00, sizeof(szBatchNum));
						inGetBatchNum(szBatchNum);
						pobTran->srBRec.lnBatchNum = atol(szBatchNum);
						
						if (ginDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
							sprintf(szDebugMsg, "%s Open", szHostLabel);
							inLogPrintf(AT, szDebugMsg);
						}
						
					}/* Host Enable比對End */

					break;
				}/* Host Label比對 End */
				i++;

			} while (1);
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%d", "HDTindex", pobTran->srBRec.inHDTIndex);
				inLogPrintf(AT, szDebugMsg);
			}
			
			break;
	}
	
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_START_CARD_NO_INQUIRY_, 2))
	{
		/* NCCC = 03 + DINERS = 00 + HG = 09 */
		if ((!memcmp(srECROb->srTransData.szHostID, _ECR_8N1_NCCC_HOSTID_NCCC_, 2))	||
		    (!memcmp(srECROb->srTransData.szHostID, _ECR_8N1_NCCC_HOSTID_DINERS_, 2))	||
		    (!memcmp(srECROb->srTransData.szHostID, _ECR_8N1_NCCC_HOSTID_CUS_005_FTC_HG_, 2)))
		{
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
                        /* [113110-信託需求][UI] 修改抬頭黑底長度不夠問題,加入 inDISP_Display_Black_Back()  2025/11/20 */
                        inDISP_Display_Black_Back(0, _COORDINATE_Y_LINE_8_3_, _LCD_XSIZE_, _LCD_YSIZE_/ 8);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned("                ", _FONTSIZE_8X16_, _COLOR_BLACK_, _COLOR_BLACK_, _COORDINATE_Y_LINE_8_3_, VS_FALSE, _DISP_CENTER_);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned("查詢卡片資料", _FONTSIZE_8X16_, _COLOR_WHITE_, _COLOR_BLACK_, _COORDINATE_Y_LINE_8_3_, VS_FALSE, _DISP_CENTER_);

			if (!memcmp(srECROb->srTransData.szCardType, _ECR_8N1_NCCC_CARDTYPE_CUS_005_FTC_COMBO_, 1))
			{
				/* 讀聯名卡資料使用 */
				memset(szGetFPG_FTC_FEID, 0x00, sizeof(szGetFPG_FTC_FEID));
				memcpy(&szGetFPG_FTC_FEID[0], srECROb->srTransData.srCus005_Data.szFPG_EFID, 4);
				memcpy(&szGetFPG_FTC_FEID[4], srECROb->srTransData.srCus005_Data.szSVC_EFID, 4);
				
				if (!memcmp(srECROb->srTransData.srCus005_Data.szFPG_EFID, "    " , 4))
				{
					memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
					sprintf(pobTran->szErrorMsgBuff1, "INQUIRY_RECE");
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "N0_EFID_ERR");
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
					return (VS_ERROR);
				}

				/* 聯名卡 */
				if (inNCCC_Func_ICC_FPG_FTC_InsertCard(pobTran) != VS_SUCCESS)
				{
					memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
					sprintf(pobTran->szErrorMsgBuff1, "INQUIRY_RECE");
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "CARD_TYPE_1_ERR");
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
					return (VS_ERROR);
				}
				else
				{
					if (inNCCC_Func_ICC_FPG_FTC_GetChipCardData(pobTran) != VS_SUCCESS)
					{
						pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
						return (VS_ERROR);
					}

					if (inNCCC_Func_ICC_FPG_FTC_GetChipCardBalanceData(pobTran, &szGetFPG_FTC_FEID[0]) != VS_SUCCESS)
					{
						pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
						return (VS_ERROR);
					}
				}

				srECROb->srTransData.srCus005_Data.uszFPG_FTC_CardBit = VS_TRUE;
			}
			else if (!memcmp(srECROb->srTransData.szCardType, _ECR_8N1_NCCC_CARDTYPE_CUS_005_FTC_FETC_, 1))
			{
				/* FEG & FETC 卡 */
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				sprintf(pobTran->szErrorMsgBuff1, "INQUIRY_RECE");
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "CARD_TYPE_3_ERR");
				pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_CARD_TYPR_NOT_MATCHED;
				
				return (VS_ERROR);
			}
			else if (!memcmp(srECROb->srTransData.szCardType, _ECR_8N1_NCCC_CARDTYPE_CUS_005_FTC_HG_, 1))
			{
				srECROb->srTransData.srCus005_Data.uszHappyGoCardBit = VS_TRUE;
			}
			else if (!memcmp(srECROb->srTransData.szCardType, _ECR_8N1_NCCC_CARDTYPE_CUS_005_FTC_CUP_, 1))
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
                                /* [113110-信託需求][UI] 修改抬頭黑底長度不夠問題,加入 inDISP_Display_Black_Back()  2025/11/20 */
                                inDISP_Display_Black_Back(0, _COORDINATE_Y_LINE_8_3_, _LCD_XSIZE_, _LCD_YSIZE_/ 8);
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned("                ", _FONTSIZE_8X16_, _COLOR_BLACK_, _COLOR_BLACK_, _COORDINATE_Y_LINE_8_3_, VS_FALSE, _DISP_CENTER_);
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned("查詢卡片資料", _FONTSIZE_8X16_, _COLOR_WHITE_, _COLOR_BLACK_, _COORDINATE_Y_LINE_8_3_, VS_FALSE, _DISP_CENTER_);
				pobTran->srBRec.uszCUPTransBit = VS_TRUE;
				pobTran->inTransactionCode = _CUP_SALE_;
				pobTran->srBRec.inCode = pobTran->inFunctionID;
				pobTran->srBRec.inOrgCode = pobTran->inFunctionID;
				
				/* 銀聯目前沒這些交易別，要回接收資料錯誤 */
				if (inTransType == _ECR_8N1_INSTALLMENT_NO_		||
				    inTransType == _ECR_8N1_REDEEM_NO_			||
				    inTransType == _ECR_8N1_FPG_FTC_INSTALLMENT_REFUND_NO_	||
				    inTransType == _ECR_8N1_FPG_FTC_REDEMPTION_REFUND_NO_)
				{
					memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
					inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "銀聯無此交易別");
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
					return (VS_ERROR);

				}
			}
			else
			{
				pobTran->inTransactionCode = _SALE_;
				pobTran->srBRec.inCode = _SALE_;
				pobTran->srBRec.inOrgCode = _SALE_;
			}
		}
		else
		{
			/* 找不到相對應的 Host ID */
			memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
			sprintf(pobTran->szErrorMsgBuff1, "INQUIRY_RECE");
			memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
			sprintf(pobTran->szErrorMsgBuff2, "CARD_TYPE_3_ERR");
			pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
			return (VS_ERROR);
		}
	}
	
	/* EDC簽單序號 Receipt No(InvoiceNumber) */
	switch (inTransType)
	{
		case _ECR_8N1_VOID_NO_:				/* 取消交易 */
		case _ECR_8N1_FPG_FTC_REPRINT_RECEIPT_NO_:		/* 重印簽單 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], srECROb->srTransData.szReceiptNo, 6);
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Receipt No", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "Receipt No Not Exist Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
						
				return (VS_ERROR);
			}
			else
			{
				pobTran->srBRec.lnOrgInvNum = atol(szTemplate);
			}
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%ld", "lnOrgInvNum", pobTran->srBRec.lnOrgInvNum);
				inLogPrintf(AT, szDebugMsg);
			}
			
			break;
		default:
			break;
	}
	
	/* 處理交易金額 (Trans Amount) */
	switch (inTransType)
	{
		case _ECR_8N1_START_CARD_NO_INQUIRY_NO_:	/* 啟動卡號查詢 */
		case _ECR_8N1_SALE_NO_:				/* 一般交易 */
		case _ECR_8N1_REFUND_NO_:			/* 退貨交易 */
		case _ECR_8N1_INSTALLMENT_NO_:			/* 分期 */
		case _ECR_8N1_REDEEM_NO_:			/* 紅利 */		
		case _ECR_8N1_HG_REWARD_SALE_NO_:		/* HG紅利積點 */
		case _ECR_8N1_FPG_FTC_INSTALLMENT_REFUND_NO_:
		case _ECR_8N1_FPG_FTC_REDEMPTION_REFUND_NO_:
                case _ECR_8N1_FPG_FTC_HG_REDEEM_NO_:		/* HG 點數扣抵【信用卡、現金】(07) */
                case _ECR_8N1_FPG_FTC_HGI_ONLINE_REDEEM_NO_:	/* HGI 點數扣抵【HGI 信用卡】(08) */
                case _ECR_8N1_FPG_FTC_HG_POINT_CERTAIN_NO_:		/* HG 加價購【信用卡、現金】(09) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &srECROb->srTransData.szField_10[0], 10);
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Amount", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "Trans Amount Not Exist Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
				return (VS_ERROR);
			}
			else if ((!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_076_8_DIGITS_, _CUSTOMER_INDICATOR_SIZE_) && atol(szTemplate) > 99999999) || 
				 (memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_076_8_DIGITS_, _CUSTOMER_INDICATOR_SIZE_) && atol(szTemplate) > 9999999))
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Amount", "Out of bound");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_076_8_DIGITS_, _CUSTOMER_INDICATOR_SIZE_))
				{
					sprintf(pobTran->szErrorMsgBuff2, "金額超過8位數");
				}
				else
				{
					sprintf(pobTran->szErrorMsgBuff2, "金額超過7位數");
				}
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
				
				return (VS_ERROR);
			}
			else
			{
				pobTran->srBRec.lnTxnAmount = atol(szTemplate);
				pobTran->srBRec.lnOrgTxnAmount = atol(szTemplate);
				pobTran->srBRec.lnTotalTxnAmount = atol(szTemplate);
			}

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%ld", "交易金額", pobTran->srBRec.lnTxnAmount);
				inLogPrintf(AT, szDebugMsg);
			}
			
			break;
		default:
			break;
	}
	
	/* 授權碼(Auth Code) ECR電文中給9個字元 但ATS電文只需要6個，所以只抓6個 */
	switch (inTransType)
	{
		case _ECR_8N1_REFUND_NO_:			/* 退貨 */
		case _ECR_8N1_FPG_FTC_INSTALLMENT_REFUND_NO_:
		case _ECR_8N1_FPG_FTC_REDEMPTION_REFUND_NO_:
			/* 處理授權碼(optional) */
			memset(pobTran->srBRec.szAuthCode, 0x00, sizeof(pobTran->srBRec.szAuthCode));
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szApprovalNo[0], 6);
			memcpy(pobTran->srBRec.szAuthCode, szTemplate, 6);
			break;
		default :
			break;
	}
	
	/* 處理櫃號(Store ID) */
	switch (inTransType)
	{
		case _ECR_8N1_START_CARD_NO_INQUIRY_NO_:	/* 啟動卡號查詢 */
		case _ECR_8N1_SALE_NO_:				/* 一般交易 */
		case _ECR_8N1_REFUND_NO_:			/* 退貨交易 */
		case _ECR_8N1_INSTALLMENT_NO_:			/* 分期 */
		case _ECR_8N1_REDEEM_NO_:			/* 紅利 */
		case _ECR_8N1_FPG_FTC_INSTALLMENT_REFUND_NO_ :
		case _ECR_8N1_FPG_FTC_REDEMPTION_REFUND_NO_ :
                case _ECR_8N1_FPG_FTC_HG_REDEEM_NO_ :		/* HG點數扣抵(07) */
                case _ECR_8N1_FPG_FTC_HGI_ONLINE_REDEEM_NO_ :	/* HGI 點數扣抵【HGI 信用卡】(08) */ /* 2013-04-11 AM 08:55:28 add by kakab 舊TMS轉置新TMS台亞版收銀機連線開發 */
                case _ECR_8N1_FPG_FTC_HG_POINT_CERTAIN_NO_ :	/* HG 加價購【信用卡、現金】(09) */ /* 2013-04-11 AM 08:55:28 add by kakab 舊TMS轉置新TMS台亞版收銀機連線開發 */
                case _ECR_8N1_FPG_FTC_HG_POINT_REDEEM_NO_ :		/* HAPPY GO 點數兌換【HG 商品券兌換】(29) */
                case _ECR_8N1_FPG_FTC_REPRINT_COUPON_NO_ :		/* 重印HG點數兌換 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szStoreId[0], 18);
			
			if (strlen(szTemplate) > 0)
			{
				memcpy(pobTran->srBRec.szStoreID, szTemplate, strlen(szTemplate));
				/* 櫃號不滿18，補空白 */
				if (strlen(szTemplate) < 18)
				{
					memset(&pobTran->srBRec.szStoreID[strlen(szTemplate)], 0x20, 18 - strlen(szTemplate));
				}

			}
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "Store ID", pobTran->srBRec.szStoreID);
				inLogPrintf(AT, szDebugMsg);
			}
			break;
		default:
			break;
	}
	
	/* 存發票號碼 (10 BYTE) */
	if (inTransType != _ECR_8N1_VOID_NO_ && inTransType != _ECR_8N1_FPG_FTC_REPRINT_RECEIPT_NO_)
	{
		memset(pobTran->srBRec.szFPG_FTC_Invoice, 0x00, sizeof(pobTran->srBRec.szFPG_FTC_Invoice));
		memcpy(&pobTran->srBRec.szFPG_FTC_Invoice[0], srECROb->srTransData.srCus005_Data.szFPG_FTC_Invoice, 10);
	}
	/* 存發票號碼 (10 BYTE) */
	
	
	if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
	{
		/* 有開CUP且MACEnable有開但安全認證沒過，不能執行CUP交易 */
		if (inKMS_CheckKey(_TWK_KEYSET_NCCC_, _TWK_KEYINDEX_NCCC_MAC_) != VS_SUCCESS)
		{
			if (inNCCC_Func_CUP_PowerOn_LogOn(pobTran) != VS_SUCCESS)
			{
				/* 安全認證失敗 */
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "安全認證失敗");
				}
				pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
				
				return (VS_ERROR);
			}
		}
	}
	
	if (inECR_Check_Exception(pobTran) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	/* 某些Host不支援退貨 */
	if (inTransType == _ECR_8N1_REFUND_NO_)
	{
		/* 10不知道是那一個host，根據520 Code保留 */
		if (!memcmp(srECROb->srTransData.szHostID, _ECR_8N1_NCCC_HOSTID_CUS_005_FTC_HG_, 2) || 
		    !memcmp(srECROb->srTransData.szHostID, "10", 2))
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, srECROb->srTransData.szHostID, 2);
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "inECR_8N1_Customer_005_FPG_Unpack[HOST_ID : %s]_REFUND_ERROR", szTemplate);
			}
			memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
			inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
			memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
			sprintf(pobTran->szErrorMsgBuff2, "此Host不支援退貨");
			pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
			
			return (VS_ERROR);
		}
		
		if (memcmp(srECROb->srTransData.szCardType, _ECR_8N1_NCCC_CARDTYPE_CUS_005_FTC_CUP_, 1))
		{
			memset(pobTran->srBRec.szPAN, 0x00, sizeof(pobTran->srBRec.szPAN));
			memcpy(&pobTran->srBRec.szPAN[0], &srECROb->srTransData.szCardNo, 19);
			
			for (i = 0; i < 19; i ++)
			{
				if (pobTran->srBRec.szPAN[i] == 0x20)
					pobTran->srBRec.szPAN[i] = 0x00;
			}
			
			for (i = 0; i < strlen(pobTran->srBRec.szPAN); i ++)
			{
				if ((pobTran->srBRec.szPAN[i] >= '0') && (pobTran->srBRec.szPAN[i] <= '9'))
					continue;
				else
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "inECR_8N1_FPG_RecePacket()[PAN : %s]_REFUND_ERROR", pobTran->srBRec.szPAN);
					}
					memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
					inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "卡號非數字");
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
					return (VS_ERROR);
				}
			}
			
			if (strlen(pobTran->srBRec.szPAN) == 11 && memcmp(srECROb->srTransData.szCardType, _ECR_8N1_NCCC_CARDTYPE_CUS_005_FTC_CUP_, 1))
			{
				memset(&szTemplate[0], 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "4000%s", pobTran->srBRec.szPAN);
				memset(pobTran->srBRec.szPAN, 0x00, sizeof(pobTran->srBRec.szPAN));
				strcpy(pobTran->srBRec.szPAN, szTemplate);
			}

			/* 有效期 */
			memset(pobTran->srBRec.szExpDate, 0x00, sizeof(pobTran->srBRec.szExpDate));
			memcpy(&pobTran->srBRec.szExpDate[0], srECROb->srTransData.szField_09, 4);

			for (i = 0; i < 4; i ++)
			{
				if (pobTran->srBRec.szExpDate[i] == 0x20)
					pobTran->srBRec.szExpDate[i] = 0x30;
			}

			for (i = 0; i < 4; i ++)
			{
				if ((pobTran->srBRec.szExpDate[i] >= '0') && (pobTran->srBRec.szExpDate[i] <= '9'))
					continue;
				else
				{

					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "inECR_8N1_FPG_RecePacket()[ExpDate : %s]_REFUND_ERROR", pobTran->srBRec.szExpDate);
					}
					memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
					inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "有效期非數字");
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
					return (VS_SUCCESS);
				}
			}

			if ((strlen(pobTran->srBRec.szPAN) == 0)	||
			    (strlen(pobTran->srBRec.szExpDate) == 0)	||
			    (!memcmp(&pobTran->srBRec.szExpDate[0], "0000", 4)))
			{
				pobTran->srBRec.uszManualBit = VS_FALSE;
				memset(pobTran->srBRec.szPAN, 0x00, sizeof(pobTran->srBRec.szPAN));
				memset(pobTran->srBRec.szExpDate, 0x00, sizeof(pobTran->srBRec.szExpDate));
			}
			else
				pobTran->srBRec.uszManualBit = VS_TRUE; /* 當做是 Manual Keyin */
		}
	}
	else if (inTransType == _ECR_8N1_INSTALLMENT_NO_)
	{
		/* Get Installment Period 分期期數 */
		memset(&szTemplate[0], 0x00, sizeof(szTemplate));
		memcpy(&szTemplate[0], srECROb->srTransData.szInstallmentPeriod, 2);

		for (i = 0; i < strlen(szTemplate); i ++)
		{
			if (((szTemplate[i] >= '0') && (szTemplate[i] <= '9')) || (szTemplate[i] == 0x20))
				continue;
			else
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "inECR_8N1_FPG_RecePacket()[PERIOD : %s]_INSTALLMENT_PERIOD_ERR", szTemplate);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "分期資料錯誤");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
				return (VS_ERROR);
			}
		}
		pobTran->srBRec.lnInstallmentPeriod = atoi(szTemplate);
	}
	else if (inTransType == _ECR_8N1_FPG_FTC_INSTALLMENT_REFUND_NO_)
	{
		/* 10不知道是那一個host，根據520 Code保留 */
		if (!memcmp(srECROb->srTransData.szHostID, _ECR_8N1_NCCC_HOSTID_CUS_005_FTC_HG_, 2) || 
		    !memcmp(srECROb->srTransData.szHostID, "10", 2))
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, srECROb->srTransData.szHostID, 2);
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "inECR_8N1_Customer_005_FPG_Unpack[HOST_ID : %s]_REFUND_ERROR", szTemplate);
			}
			memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
			inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
			memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
			sprintf(pobTran->szErrorMsgBuff2, "此Host不支援退貨");
			pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
			
			return (VS_ERROR);
		}
		
		memset(pobTran->srBRec.szPAN, 0x00, sizeof(pobTran->srBRec.szPAN));
		memcpy(&pobTran->srBRec.szPAN[0], &srECROb->srTransData.szCardNo, 19);

		for (i = 0; i < 19; i ++)
		{
			if (pobTran->srBRec.szPAN[i] == 0x20)
				pobTran->srBRec.szPAN[i] = 0x00;
		}

		for (i = 0; i < strlen(pobTran->srBRec.szPAN); i ++)
		{
			if ((pobTran->srBRec.szPAN[i] >= '0') && (pobTran->srBRec.szPAN[i] <= '9'))
				continue;
			else
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "inECR_8N1_FPG_RecePacket()[PAN : %s]_REFUND_ERROR", pobTran->srBRec.szPAN);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "卡號非數字");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				return (VS_ERROR);
			}
		}

		if (strlen(pobTran->srBRec.szPAN) == 11 && memcmp(srECROb->srTransData.szCardType, _ECR_8N1_NCCC_CARDTYPE_CUS_005_FTC_CUP_, 1))
		{
			memset(&szTemplate[0], 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "4000%s", pobTran->srBRec.szPAN);
			memset(pobTran->srBRec.szPAN, 0x00, sizeof(pobTran->srBRec.szPAN));
			strcpy(pobTran->srBRec.szPAN, szTemplate);
		}

		/* 有效期 */
		memset(pobTran->srBRec.szExpDate, 0x00, sizeof(pobTran->srBRec.szExpDate));
		memcpy(&pobTran->srBRec.szExpDate[0], srECROb->srTransData.szField_09, 4);

		for (i = 0; i < 4; i ++)
		{
			if (pobTran->srBRec.szExpDate[i] == 0x20)
				pobTran->srBRec.szExpDate[i] = 0x30;
		}

		for (i = 0; i < 4; i ++)
		{
			if ((pobTran->srBRec.szExpDate[i] >= '0') && (pobTran->srBRec.szExpDate[i] <= '9'))
				continue;
			else
			{

				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "inECR_8N1_FPG_RecePacket()[ExpDate : %s]_REFUND_ERROR", pobTran->srBRec.szExpDate);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "有效期非數字");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				return (VS_SUCCESS);
			}
		}

		if ((strlen(pobTran->srBRec.szPAN) == 0)	||
		    (strlen(pobTran->srBRec.szExpDate) == 0)	||
		    (!memcmp(&pobTran->srBRec.szExpDate[0], "0000", 4)))
		{
			pobTran->srBRec.uszManualBit = VS_FALSE;
			memset(pobTran->srBRec.szPAN, 0x00, sizeof(pobTran->srBRec.szPAN));
			memset(pobTran->srBRec.szExpDate, 0x00, sizeof(pobTran->srBRec.szExpDate));
		}
		else
			pobTran->srBRec.uszManualBit = VS_TRUE; /* 當做是 Manual Keyin */

		/* Get Installment Period 分期期數 */
		memset(&szTemplate[0], 0x00, sizeof(szTemplate));
		memcpy(&szTemplate[0], srECROb->srTransData.szInstallmentPeriod, 2);

		for (i = 0; i < strlen(szTemplate); i ++)
		{
			if (((szTemplate[i] >= '0') && (szTemplate[i] <= '9')) || (szTemplate[i] == 0x20))
				continue;
			else
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "inECR_8N1_FPG_RecePacket()[PERIOD : %s]_INSTALLMENT_PERIOD_ERR", szTemplate);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "分期資料錯誤");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
				return (VS_ERROR);
			}
		}
		pobTran->srBRec.lnInstallmentPeriod = atoi(szTemplate);
		
		/* Get Installment Formality Fee 手續費 */
		memset(&szTemplate[0], 0x00, sizeof(szTemplate));
		memcpy(&szTemplate[0], srECROb->srTransData.szField_28, 8);

		for (i = 0; i < strlen(szTemplate); i ++)
		{
			if (((szTemplate[i] >= '0') && (szTemplate[i] <= '9')) || (szTemplate[i] == 0x20))
				continue;
			else
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "inECR_8N1_FPG_RecePacket()[FORMALITY : %s]_INSTALLMENT_REFUND_FORMALITY_ERR", szTemplate);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "分期資料錯誤");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
				return (VS_ERROR);
			}
		}
		pobTran->srBRec.lnInstallmentFormalityFee = atol(szTemplate);

		/* 【需求單 - 107092】多個LOGO及分期退貨不輸手續費需求 */
		if (pobTran->srBRec.lnInstallmentFormalityFee > 0L)
		{
			memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
			inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
			memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
			sprintf(pobTran->szErrorMsgBuff2, "不應輸入手續費");
			pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
			pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
			return (VS_ERROR);
		}

		/* Get Installment Down Payment Amount 首期金額 */
		memset(&szTemplate[0], 0x00, sizeof(szTemplate));
		memcpy(&szTemplate[0], srECROb->srTransData.szField_26, 10);

		for (i = 0; i < strlen(szTemplate); i ++)
		{
			if (((szTemplate[i] >= '0') && (szTemplate[i] <= '9')) || (szTemplate[i] == 0x20))
				continue;
			else
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "inECR_8N1_FPG_RecePacket()[DOWN : %s]_INSTALLMENT_REFUND_DOWN_PAYMENT_ERR", szTemplate);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "分期資料錯誤");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
				return (VS_ERROR);
			}
		}

		pobTran->srBRec.lnInstallmentDownPayment = atol(szTemplate);
		/* Get Installment Payment Amount 每期金額 */
		memset(&szTemplate[0], 0x00, sizeof(szTemplate));
		memcpy(&szTemplate[0], srECROb->srTransData.szField_27, 10);

		for (i = 0; i < strlen(szTemplate); i ++)
		{
			if (((szTemplate[i] >= '0') && (szTemplate[i] <= '9')) || (szTemplate[i] == 0x20))
				continue;
			else
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "inECR_8N1_FPG_RecePacket()[PAYMENT : %s]_INSTALLMENT_REFUND_PAYMENT_AMT_ERR", szTemplate);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "分期資料錯誤");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
				return (VS_ERROR);
			}
		}
		pobTran->srBRec.lnInstallmentPayment = atol(szTemplate);
	}
	else if (inTransType == _ECR_8N1_FPG_FTC_REDEMPTION_REFUND_NO_)
	{
		/* 10不知道是那一個host，根據520 Code保留 */
		if (!memcmp(srECROb->srTransData.szHostID, _ECR_8N1_NCCC_HOSTID_CUS_005_FTC_HG_, 2) || 
		    !memcmp(srECROb->srTransData.szHostID, "10", 2))
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, srECROb->srTransData.szHostID, 2);
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "inECR_8N1_Customer_005_FPG_Unpack[HOST_ID : %s]_REFUND_ERROR", szTemplate);
			}
			memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
			inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
			memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
			sprintf(pobTran->szErrorMsgBuff2, "此Host不支援退貨");
			pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
			
			return (VS_ERROR);
		}
		
		memset(pobTran->srBRec.szPAN, 0x00, sizeof(pobTran->srBRec.szPAN));
		memcpy(&pobTran->srBRec.szPAN[0], &srECROb->srTransData.szCardNo, 19);

		for (i = 0; i < 19; i ++)
		{
			if (pobTran->srBRec.szPAN[i] == 0x20)
				pobTran->srBRec.szPAN[i] = 0x00;
		}

		for (i = 0; i < strlen(pobTran->srBRec.szPAN); i ++)
		{
			if ((pobTran->srBRec.szPAN[i] >= '0') && (pobTran->srBRec.szPAN[i] <= '9'))
				continue;
			else
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "inECR_8N1_FPG_RecePacket()[PAN : %s]_REFUND_ERROR", pobTran->srBRec.szPAN);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "卡號非數字");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				return (VS_ERROR);
			}
		}

		if (strlen(pobTran->srBRec.szPAN) == 11 && memcmp(srECROb->srTransData.szCardType, _ECR_8N1_NCCC_CARDTYPE_CUS_005_FTC_CUP_, 1))
		{
			memset(&szTemplate[0], 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "4000%s", pobTran->srBRec.szPAN);
			memset(pobTran->srBRec.szPAN, 0x00, sizeof(pobTran->srBRec.szPAN));
			strcpy(pobTran->srBRec.szPAN, szTemplate);
		}

		/* 有效期 */
		memset(pobTran->srBRec.szExpDate, 0x00, sizeof(pobTran->srBRec.szExpDate));
		memcpy(&pobTran->srBRec.szExpDate[0], srECROb->srTransData.szField_09, 4);

		for (i = 0; i < 4; i ++)
		{
			if (pobTran->srBRec.szExpDate[i] == 0x20)
				pobTran->srBRec.szExpDate[i] = 0x30;
		}

		for (i = 0; i < 4; i ++)
		{
			if ((pobTran->srBRec.szExpDate[i] >= '0') && (pobTran->srBRec.szExpDate[i] <= '9'))
				continue;
			else
			{

				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "inECR_8N1_FPG_RecePacket()[ExpDate : %s]_REFUND_ERROR", pobTran->srBRec.szExpDate);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "有效期非數字");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				return (VS_SUCCESS);
			}
		}

		if ((strlen(pobTran->srBRec.szPAN) == 0)	||
		    (strlen(pobTran->srBRec.szExpDate) == 0)	||
		    (!memcmp(&pobTran->srBRec.szExpDate[0], "0000", 4)))
		{
			pobTran->srBRec.uszManualBit = VS_FALSE;
			memset(pobTran->srBRec.szPAN, 0x00, sizeof(pobTran->srBRec.szPAN));
			memset(pobTran->srBRec.szExpDate, 0x00, sizeof(pobTran->srBRec.szExpDate));
		}
		else
			pobTran->srBRec.uszManualBit = VS_TRUE; /* 當做是 Manual Keyin */
		
		/* Get Pay Credit Amount 實際支付金額 */
		memset(&szTemplate[0], 0x00, sizeof(szTemplate));
		memcpy(&szTemplate[0], srECROb->srTransData.szRedeemAmt, 10);

		for (i = 0; i < strlen(szTemplate); i ++)
		{
			if (((szTemplate[i] >= '0') && (szTemplate[i] <= '9')) || (szTemplate[i] == 0x20))
				continue;
			else
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "inECR_8N1_FPG_RecePacket()[PAY_CREDIT : %s]_REDEEM_REFUND_PAY_CREDIT_ERR", szTemplate);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				sprintf(pobTran->szErrorMsgBuff1, "FPG_OPT_REDE_REF");
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "PAY_CREDIT_ERR");
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "紅利扣抵資料錯誤");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				return (VS_ERROR);
			}
		}
				
		/* Redeem Amount 扣抵金額，但是要計算支付金額 */
		pobTran->srBRec.lnRedemptionPaidCreditAmount = (pobTran->srBRec.lnTxnAmount - (atol(szTemplate)));
		/* Get Points of Redeem 扣抵紅利點數 */
		memset(&szTemplate[0], 0x00, sizeof(szTemplate));
		memcpy(&szTemplate[0], srECROb->srTransData.szRDMPoint, 10);

		for (i = 0; i < strlen(szTemplate); i ++)
		{
			if (((szTemplate[i] >= '0') && (szTemplate[i] <= '9')) || (szTemplate[i] == 0x20))
				continue;
			else
			{
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "紅利扣抵資料錯誤");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				return (VS_ERROR);
			}
		}
		pobTran->srBRec.lnRedemptionPoints = atol(szTemplate);
	}
	
	memset(szFiscContactlessEnable, 0x00, sizeof(szFiscContactlessEnable));
	inGetSmartPayContactlessEnable(szFiscContactlessEnable);
	memset(szCTLSEnable, 0x00, sizeof(szCTLSEnable));
	inGetContactlessEnable(szCTLSEnable);
	memset(szCUPContactlessEnable, 0x00, sizeof(szCUPContactlessEnable));
	inGetCUPContactlessEnable(szCUPContactlessEnable);
	
	/* 跑OPT */
	switch (inTransType)
	{
		case _ECR_8N1_SALE_NO_:
			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_CUP_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜銀聯一般交易＞ */

				if (!memcmp(szCTLSEnable, "Y", 1)		&&
				    !memcmp(szCUPContactlessEnable, "Y", 1)	&&
				    guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
					pobTran->inRunTRTID = _TRT_CUP_SALE_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
					pobTran->inRunTRTID = _TRT_CUP_SALE_;
				}

				pobTran->inFunctionID = _CUP_SALE_;
				pobTran->inTransactionCode = _CUP_SALE_;
				pobTran->srBRec.inCode = pobTran->inFunctionID;
				pobTran->srBRec.inOrgCode = pobTran->inFunctionID;
				
				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else
			{
				inDISP_ClearAll();
				/* 第三層顯示 ＜一般交易＞ */
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般交易＞ */

				pobTran->inFunctionID = _SALE_;
				
				/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/10 上午 10:32 */
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
					pobTran->inRunTRTID = _TRT_SALE_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
					pobTran->inRunTRTID = _TRT_SALE_;
				}

				pobTran->inTransactionCode = _SALE_;
				pobTran->srBRec.inCode = _SALE_;
				pobTran->srBRec.inOrgCode = _SALE_;
				
				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			
			break;
		case _ECR_8N1_VOID_NO_:
			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜取消交易＞ */

				pobTran->inRunOperationID = _OPERATION_VOID_;
				pobTran->inRunTRTID = _TRT_CUP_VOID_;

				pobTran->inFunctionID = _CUP_VOID_;
				pobTran->inTransactionCode = _CUP_VOID_;
				pobTran->srBRec.inCode = pobTran->inTransactionCode;
				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				/* HG卡取消交易Title 先不做 */
				inDISP_PutGraphic(_MENU_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜取消交易＞ */

				pobTran->inFunctionID = _VOID_;
				pobTran->inRunOperationID = _OPERATION_VOID_;
				pobTran->inRunTRTID = _TRT_VOID_;

				pobTran->inTransactionCode = _VOID_;
				pobTran->srBRec.inCode = _VOID_;
				pobTran->srBRec.inOrgCode = _VOID_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			break;
		case _ECR_8N1_REFUND_NO_:
			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般退貨＞ */

				if (!memcmp(szCTLSEnable, "Y", 1)		&&
				    !memcmp(szCUPContactlessEnable, "Y", 1)	&&
				    guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_GET_CARD_FIRST_;
					pobTran->inRunTRTID = _TRT_CUP_REFUND_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_AMOUNT_FIRST_CUP_;
					pobTran->inRunTRTID = _TRT_CUP_REFUND_;
				}

				pobTran->inFunctionID = _CUP_REFUND_;
				pobTran->inTransactionCode = _CUP_REFUND_;
				pobTran->srBRec.inCode = pobTran->inTransactionCode;
				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
				
				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般退貨＞ */

				pobTran->inFunctionID = _REFUND_;
				/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/10 上午 10:32 */
				if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_BARCODE_;
				}
				else if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_GET_CARD_FIRST_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_AMOUNT_FIRST_;
				}
				pobTran->inRunTRTID = _TRT_REFUND_;

				pobTran->inTransactionCode = _REFUND_;
				pobTran->srBRec.inCode = _REFUND_;
				pobTran->srBRec.inOrgCode = _REFUND_;
				
				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;
		case _ECR_8N1_INSTALLMENT_NO_:
//			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
//			{
//				inDISP_ClearAll();
//				/* 保留給銀聯分期使用 */
//				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
//				inDISP_PutGraphic(_MENU_INST_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜分期付款＞ */
//
//				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
//				{
//					pobTran->inRunOperationID = _OPERATION_INST_SALE_CTLS_;
//				}
//				else
//				{
//					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
//				}
//				pobTran->inRunTRTID = _TRT_CUP_INST_SALE_;
//
//				pobTran->inFunctionID = _CUP_INST_SALE_;
//				pobTran->inTransactionCode = _CUP_INST_SALE_;
//				pobTran->srBRec.inCode = pobTran->inTransactionCode;
//				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
//
//				pobTran->srBRec.uszInstallmentBit = VS_TRUE;
//				
//				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
//				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
//				{
//					inNCCC_Func_Decide_CTLS_TRT(pobTran);
//				}
//				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
//				{
//					inNCCC_Func_Decide_ICC_TRT(pobTran);
//				}
//				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
//				{
//					inNCCC_Func_Decide_MEG_TRT(pobTran);
//				}
//
//				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
//			}
//			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_INST_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜分期付款＞ */

				pobTran->inFunctionID = _INST_SALE_;
				
				/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/10 上午 10:32 */
				if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_INST_SALE_BARCODE_;
				}
				else if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_INST_SALE_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
				}
				pobTran->inRunTRTID = _TRT_INST_SALE_;

				pobTran->inTransactionCode = _INST_SALE_;
				pobTran->srBRec.inCode = _INST_SALE_;
				pobTran->srBRec.inOrgCode = _INST_SALE_;

				pobTran->srBRec.uszInstallmentBit = VS_TRUE;
				
				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;
			
		case _ECR_8N1_FPG_FTC_INSTALLMENT_REFUND_NO_:
//			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
//			{
//				inDISP_ClearAll();
//				/* 保留給銀聯分期退貨使用 */
//				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
//				inDISP_PutGraphic(_MENU_INST_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜分期退貨＞ */
//
//				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
//				{
//					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_GET_CARD_FIRST_;
//				}
//				else
//				{
//					pobTran->inRunOperationID = _OPERATION_REFUND_;
//				}
//				pobTran->inRunTRTID = _TRT_CUP_INST_REFUND_;
//
//				pobTran->inFunctionID = _CUP_INST_REFUND_;
//				pobTran->inTransactionCode = _CUP_INST_REFUND_;
//				pobTran->srBRec.inCode = pobTran->inTransactionCode;
//				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
//
//				pobTran->srBRec.uszInstallmentBit = VS_TRUE;
//				
//				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
//				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
//				{
//					inNCCC_Func_Decide_CTLS_TRT(pobTran);
//				}
//				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
//				{
//					inNCCC_Func_Decide_ICC_TRT(pobTran);
//				}
//				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
//				{
//					inNCCC_Func_Decide_MEG_TRT(pobTran);
//				}
//
//				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
//			}
//			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_INST_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜分期退貨＞ */

				pobTran->inFunctionID = _INST_REFUND_;
				
				/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/10 上午 10:32 */
				if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_INST_REFUND_BARCODE_;
				}
				else if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_GET_CARD_FIRST_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_;
				}
				pobTran->inRunTRTID = _TRT_INST_REFUND_;

				pobTran->inTransactionCode = _INST_REFUND_;
				pobTran->srBRec.inCode = _INST_REFUND_;
				pobTran->srBRec.inOrgCode = _INST_REFUND_;

				pobTran->srBRec.uszInstallmentBit = VS_TRUE;
				
				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;
		case _ECR_8N1_REDEEM_NO_:
//			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
//			{
//				inDISP_ClearAll();
//				/* 保留給銀聯紅利扣抵使用 */
//				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
//				inDISP_PutGraphic(_MENU_REDEEM_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利扣抵＞ */
//
//				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
//				{
//					pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
//				}
//				else
//				{
//					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
//				}
//				pobTran->inRunTRTID = _TRT_REDEEM_SALE_;
//
//				pobTran->inFunctionID = _CUP_REDEEM_SALE_;
//				pobTran->inTransactionCode = _CUP_REDEEM_SALE_;
//				pobTran->srBRec.inCode = pobTran->inTransactionCode;
//				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
//
//				pobTran->srBRec.uszRedeemBit = VS_TRUE;
//				
//				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
//				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
//				{
//					inNCCC_Func_Decide_CTLS_TRT(pobTran);
//				}
//				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
//				{
//					inNCCC_Func_Decide_ICC_TRT(pobTran);
//				}
//				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
//				{
//					inNCCC_Func_Decide_MEG_TRT(pobTran);
//				}
//
//				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
//			}
//			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REDEEM_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利扣抵＞ */

				pobTran->inFunctionID = _REDEEM_SALE_;
				
				/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/10 上午 10:32 */
				if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REDEEM_SALE_BARCODE_;
				}
				else if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REDEEM_SALE_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
				}
				pobTran->inRunTRTID = _TRT_REDEEM_SALE_;

				pobTran->inTransactionCode = _REDEEM_SALE_;
				pobTran->srBRec.inCode = _REDEEM_SALE_;
				pobTran->srBRec.inOrgCode = _REDEEM_SALE_;

				pobTran->srBRec.uszRedeemBit = VS_TRUE;
				
				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;
		
		case _ECR_8N1_FPG_FTC_REDEMPTION_REFUND_NO_:
//			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
//			{
//				inDISP_ClearAll();
//				/* 保留給銀聯紅利退貨使用 */
//				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
//				inDISP_PutGraphic(_MENU_REDEEM_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利退貨＞ */
//
//				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
//				{
//					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_GET_CARD_FIRST_;
//				}
//				else
//				{
//					pobTran->inRunOperationID = _OPERATION_REFUND_;
//				}
//				pobTran->inRunTRTID = _TRT_CUP_REDEEM_REFUND_;
//
//				pobTran->inFunctionID = _CUP_REDEEM_REFUND_;
//				pobTran->inTransactionCode = _CUP_REDEEM_REFUND_;
//				pobTran->srBRec.inCode = pobTran->inTransactionCode;
//				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
//
//				pobTran->srBRec.uszRedeemBit = VS_TRUE;
//				
//				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
//				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
//				{
//					inNCCC_Func_Decide_CTLS_TRT(pobTran);
//				}
//				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
//				{
//					inNCCC_Func_Decide_ICC_TRT(pobTran);
//				}
//				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
//				{
//					inNCCC_Func_Decide_MEG_TRT(pobTran);
//				}
//
//				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
//			}
//			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REDEEM_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利退貨＞ */

				pobTran->inFunctionID = _REDEEM_REFUND_;
				
				/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/10 上午 10:32 */
				if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REDEEM_REFUND_BARCODE_;
				}
				else if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_GET_CARD_FIRST_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_;
				}
				pobTran->inRunTRTID = _TRT_REDEEM_REFUND_;

				pobTran->inTransactionCode = _REDEEM_REFUND_;
				pobTran->srBRec.inCode = _REDEEM_REFUND_;
				pobTran->srBRec.inOrgCode = _REDEEM_REFUND_;

				pobTran->srBRec.uszRedeemBit = VS_TRUE;
				
				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;
		/* 快樂購紅利積點 */
		case _ECR_8N1_FPG_FTC_HG_REDEEM_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡紅利積點＞ */
			
			pobTran->inFunctionID = _SALE_;
			pobTran->inRunOperationID = _OPERATION_HG_;
			pobTran->inRunTRTID = _TRT_HG_REWARD_CREDIT_CTLS_;

			/* 這邊先設成sale，若之後支付方式選銀聯卡，會在那邊改成CUP_SALE */
			pobTran->inTransactionCode = _SALE_;
			pobTran->srBRec.inCode = _SALE_;
			pobTran->srBRec.inOrgCode = _SALE_;
			pobTran->srBRec.lnHGTransactionType = _HG_REWARD_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;

		/* 快樂購點數扣抵 */
		case _ECR_8N1_FPG_FTC_HGI_ONLINE_REDEEM_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_ONLINE_REDEEM_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡點數扣抵＞ */
			
			pobTran->inFunctionID = _SALE_;
			pobTran->inRunOperationID = _OPERATION_HG_;
			pobTran->inRunTRTID = _TRT_HG_ONLINE_REDEEM_CREDIT_CTLS_;

			pobTran->inTransactionCode = _SALE_;
			pobTran->srBRec.inCode = _SALE_;
			pobTran->srBRec.inOrgCode = _SALE_;
			pobTran->srBRec.lnHGTransactionType = _HG_ONLINE_REDEEM_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		
		/* 快樂購加價購 */
		case _ECR_8N1_FPG_FTC_HG_POINT_CERTAIN_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_POINT_CERTAIN_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡加價購＞ */
			
			pobTran->inFunctionID = _SALE_;
			pobTran->inRunOperationID = _OPERATION_HG_;
			pobTran->inRunTRTID = _TRT_HG_POINT_CERTAIN_CREDIT_CTLS_;

			pobTran->inTransactionCode = _SALE_;
			pobTran->srBRec.inCode = _SALE_;
			pobTran->srBRec.inOrgCode = _SALE_;
			pobTran->srBRec.lnHGTransactionType = _HG_POINT_CERTAIN_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		
		/* 快樂購點數兌換 */
		case _ECR_8N1_FPG_FTC_HG_POINT_REDEEM_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_REDEMPTION_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡點數兌換＞ */
			
			pobTran->inFunctionID = _SALE_;
			pobTran->inRunOperationID = _OPERATION_HG_;
			pobTran->inRunTRTID = _TRT_HG_FULL_REDEMPTION_;

			pobTran->inTransactionCode = _SALE_;
			pobTran->srBRec.inCode = _SALE_;
			pobTran->srBRec.inOrgCode = _SALE_;
			pobTran->srBRec.lnHGTransactionType = _HG_FULL_REDEMPTION_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		
		/* 快樂購扣抵退貨 */
		case _ECR_8N1_FPG_FTC_HG_REDEEM_REFUND_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_REDEEM_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡扣抵退貨＞ */
			
			pobTran->inFunctionID = _HG_REDEEM_REFUND_;
			pobTran->inRunOperationID = _OPERATION_HG_REFUND_;
			pobTran->inRunTRTID = _TRT_HG_REDEEM_REFUND_;

			pobTran->inTransactionCode = _HG_REDEEM_REFUND_;
			pobTran->srBRec.inCode = _HG_REDEEM_REFUND_;
			pobTran->srBRec.inOrgCode = _HG_REDEEM_REFUND_;
			pobTran->srBRec.lnHGTransactionType = _HG_REDEEM_REFUND_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		case _ECR_8N1_SETTLEMENT_NO_:
			inDISP_ClearAll();
			/* 第三層顯示 ＜結帳交易＞ */
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_SETTLE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜結帳交易＞ */

			pobTran->inFunctionID = _SETTLE_;
			pobTran->inRunOperationID = _OPERATION_SETTLE_;
			pobTran->inRunTRTID = _TRT_SETTLE_;
			
			pobTran->inTransactionCode = _SETTLE_;
			pobTran->srBRec.inCode = _SETTLE_;
			pobTran->srBRec.inOrgCode = _SETTLE_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		
		case _ECR_8N1_FPG_FTC_REPRINT_RECEIPT_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_REPRINT_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜重印帳單＞ */

			pobTran->inFunctionID = FALSE;
			pobTran->inRunOperationID = _OPERATION_REPRINT_;
			pobTran->inRunTRTID = FALSE;
			
			pobTran->inTransactionCode = FALSE;
			pobTran->srBRec.inCode = FALSE;
			pobTran->srBRec.inOrgCode = FALSE;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		case _ECR_8N1_FPG_FTC_TRANS_TERMINATE_NO_:
			pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_END_INQUIRY_SUCCESS_;
			inRetVal = VS_ERROR;
			break;
		case _ECR_8N1_START_CARD_NO_INQUIRY_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
                        /* [113110-信託需求][UI] 修改抬頭黑底長度不夠問題,加入 inDISP_Display_Black_Back()  2025/11/20 */
                        inDISP_Display_Black_Back(0, _COORDINATE_Y_LINE_8_3_, _LCD_XSIZE_, _LCD_YSIZE_/ 8);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned("                ", _FONTSIZE_8X16_, _COLOR_BLACK_, _COLOR_BLACK_, _COORDINATE_Y_LINE_8_3_, VS_FALSE, _DISP_CENTER_);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned("查詢卡片資料", _FONTSIZE_8X16_, _COLOR_WHITE_, _COLOR_BLACK_, _COORDINATE_Y_LINE_8_3_, VS_FALSE, _DISP_CENTER_);
			inRetVal = VS_SUCCESS;
			/* 先給sale避免卡住，第二段再看他給什麼交易別 */
			pobTran->inTransactionCode = _SALE_;
			pobTran->srBRec.inCode = _SALE_;
			pobTran->srBRec.inOrgCode = _SALE_;
			break;
	default:
		pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
		inRetVal = VS_ERROR;
		break;
}

return (inRetVal);
}

/*
Function        :inECR_8N1_Customer_005_FPG_Parse_Data
Date&Time       :2022/9/22 下午 5:23
Describe        :
*/
int inECR_8N1_Customer_005_FPG_Parse_Data(ECR_TABLE* srECROb, char *szDataBuffer)
{
	int	inRetVal = VS_ERROR;
        char	szDate[6 + 1];
        char	szTime[6 + 1];
        RTC_NEXSYS	srRTC; 		/* Date & Time */
	
	/* Trans Type */
	memcpy(srECROb->srTransData.szTransType, &szDataBuffer[0], 2);
	
	/* Host ID */
	memcpy(&srECROb->srTransData.szHostID[0], &szDataBuffer[2], 2);
	
	/* Receipt No */
	memcpy(&srECROb->srTransData.szReceiptNo[0], &szDataBuffer[4], 6);
	
	/* Card No */
	memcpy(&srECROb->srTransData.szCardNo[0], &szDataBuffer[10], 19);
	
	/* Card Expire Date(保留欄位)4Byte */
	memcpy(&srECROb->srTransData.szField_09[0], &szDataBuffer[29], 4);
	
	/* Trans Amount */
	memcpy(&srECROb->srTransData.szField_10[0], &szDataBuffer[33], 12);
	
	memset(&srRTC, 0x00, sizeof(RTC_NEXSYS));
	inFunc_GetSystemDateAndTime(&srRTC);
        memset(szDate, 0x00, sizeof(szDate));
        sprintf(szDate, "%02d%02d%02d",  srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
        memset(szTime, 0x00, sizeof(szTime));
        sprintf(szTime, "%02d%02d%02d",  srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
        
	/* Trans Date */
	memcpy(&srECROb->srTransData.szTransDate[0], &szDate[0], 6);
	
	/* Trans Time */
	memcpy(&srECROb->srTransData.szTransTime[0], &szTime[0], 6);
	
	/* Approval No */
	memcpy(&srECROb->srTransData.szApprovalNo[0], &szDataBuffer[57], 6);
	
	/* Card Type */
	memcpy(&srECROb->srTransData.szCardType[0], &szDataBuffer[65], 1);
	
	/* RDM Paid Amt */
	memcpy(&srECROb->srTransData.szRDMPaidAmt[0], &szDataBuffer[66], 12);
	
	/* ECR RESPONSE CODE(4Byte) */
	memcpy(&srECROb->srTransData.szECRResponseCode[0], &szDataBuffer[78], 4);
	
	/* Terminal ID */
	memcpy(&srECROb->srTransData.szTerminalID[0], &szDataBuffer[82], 8);
	
	/* 信用卡OriginRRN */
	memcpy(&srECROb->srTransData.szField_35[0], &szDataBuffer[90], 12);	
	
	/* Redeem Amt */
	memcpy(&srECROb->srTransData.szRedeemAmt[0], &szDataBuffer[102], 12);
	
	/* Store Id */
	memcpy(&srECROb->srTransData.szStoreId[0], &szDataBuffer[114], 18);
	/* 油品代號 (6 Byte)(用不到) */
        
	/* 聯名卡 FE01 資料 (63 Byte) */
	
	/* 聯名卡 FE02 資料 (54 Byte) */
	
	/* 交易類型 (1 Byte)*/
	
	/* 聯名卡消費日期時間 (14 Byte) */
	
	/* 消費通路代號 (8 Byte) */
	
	/* 聯名卡消費金額(折扣後) (10 Byte) */
	
	/* 加油數量 (10 Byte) */
	
	/* 聯名卡積點 (10 Byte) */
	
	/* 聯名卡扣點 (10 Byte) */
	
	/* 儲值錢包扣款金額 (8 Byte) */
	
	/* 聯名卡基本資料 Update Indicator (1 Byte) */
	
	/* EFID1 (4 Byte) */
	memcpy(&srECROb->srTransData.srCus005_Data.szFPG_EFID, &szDataBuffer[335], 4);
	/* EFID2 (4 Byte) */
	memcpy(&srECROb->srTransData.srCus005_Data.szSVC_EFID, &szDataBuffer[339], 4);
	/* HG Card Number (19 Byte) */
	
	/* Happy Go Mode (1 Byte) */
	
	/* Redeem Amount (12 Byte)（HG及銀行） */
	memcpy(&srECROb->srTransData.szRedeemAmt[0], &szDataBuffer[363], 12);
	/* Redeem Point (12 Byte) */
	memcpy(&srECROb->srTransData.szRDMPoint[0], &szDataBuffer[375], 8);
	memcpy(&srECROb->srTransData.szHGRedeemPoint, &szDataBuffer[375], 8);
	/* Balance Point (12 Byte) */
	memcpy(&srECROb->srTransData.szPointsOfBalance[0], &szDataBuffer[387], 8);
	/* Installment Period (2 Byte) */
	memcpy(&srECROb->srTransData.szInstallmentPeriod[0], &szDataBuffer[399], 2);
	/* Formality Fee (10 Byte) */
	memcpy(&srECROb->srTransData.szField_28[0], &szDataBuffer[401], 12);
	/* Down Payment (12 Byte) */
	memcpy(&srECROb->srTransData.szField_26[0], &szDataBuffer[411], 12);
	/* Installment Payment (12 Byte) */
	memcpy(&srECROb->srTransData.szField_27[0], &szDataBuffer[423], 12);
	/* 發票號碼 (10 Byte) */
	memcpy(&srECROb->srTransData.srCus005_Data.szFPG_FTC_Invoice[0], &szDataBuffer[435], 10);
	/* Reserve (35 Byte) */
	
	return (inRetVal);
}

/*
Function        :inECR_8N1_Customer_005_FPG_Pack
Date&Time       :2022/10/3 上午 10:56
Describe        :
*/
int inECR_8N1_Customer_005_FPG_Pack(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer)
{
	int	inRetVal = VS_ERROR;
	int	i = 0, j = 0, inCardLen = 0;
	int	inPacketSizes = 0;
	char	szTemplate[100 + 1] = {0};
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inECR_8N1_Customer_005_FPG_Pack() START !");
	}

	/* Trans Type (2 Byte) */
	if (srECROb->srTransData.srCus005_Data.uszFPG_FTC_CardBit == VS_TRUE)
		memcpy(&szDataBuffer[inPacketSizes], "01", 2);
	else
		memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szTransType, 2);
	inPacketSizes += 2;

	/* HOST ID (2 Byte) */
	memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szHostID, 2);
	inPacketSizes += 2;

	/* Receipt No (6 Byte) */
	if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
	{
		
	}
	else
	{
		if (pobTran->srBRec.lnOrgInvNum != 0L)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%06ld", pobTran->srBRec.lnOrgInvNum);
			memcpy(&szDataBuffer[inPacketSizes], szTemplate, 6);
		}
	}
	inPacketSizes += 6;

	/* Card No (19 Byte) */
	if (memcmp(srECROb->srTransData.szTransType, _ECR_8N1_FPG_FTC_REPRINT_RECEIPT_, 2))
	{
		/* 卡號是否遮掩 */
		inRetVal = inECR_CardNoTruncateDecision(pobTran);
		/* 要遮卡號 */
		if (inRetVal == VS_SUCCESS)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			strcpy(szTemplate, pobTran->srBRec.szPAN);

			/* HAPPG_GO 卡不掩飾 */
			if (!memcmp(&pobTran->srBRec.szPAN[0], "9552", 4))
			{

			}
			else
			{
				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
				/* 卡號長度 */
				inCardLen = strlen(szTemplate);

				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
				for (i = 6; i < (inCardLen - 4); i ++)
					szTemplate[i] = '*';
			}
			memcpy(&szDataBuffer[inPacketSizes], szTemplate, strlen(szTemplate));
		}
		else
		{
			memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szPAN, strlen(pobTran->srBRec.szPAN));
		}
	}
	inPacketSizes += 19;

	/* Card Expire Date (4 Byte) */
	/* 有效期是否回傳 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetECRExpDateReturnEnable(szTemplate);
	if (memcmp(srECROb->srTransData.szTransType, _ECR_8N1_FPG_FTC_REPRINT_RECEIPT_, 2))
	{
		/* 要回傳有效期 */
		if (!memcmp(&szTemplate, "Y", 1))
			memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szExpDate, strlen(pobTran->srBRec.szExpDate));
	}
	inPacketSizes += 4;

	/* Trans Amount (12 Byte) */
	if (memcmp(srECROb->srTransData.szTransType, _ECR_8N1_FPG_FTC_REPRINT_RECEIPT_, 2))
	{
		if (pobTran->srBRec.lnTxnAmount > 0)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%010lu00", pobTran->srBRec.lnTxnAmount);
			memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
		}
	}
	inPacketSizes += 12;

	/* Trans Date (6 Byte) & Trans Time (6 Byte) */
	if (memcmp(srECROb->srTransData.szTransType, _ECR_8N1_FPG_FTC_REPRINT_RECEIPT_, 2))
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		strcat(szTemplate, pobTran->srBRec.szDate);
		memcpy(&szDataBuffer[inPacketSizes], &szTemplate[2], 6);
		inPacketSizes += 6;
		memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szTime[0], 6);
		inPacketSizes += 6;
	}
	else
	{
		inPacketSizes += 12;
	}

	/* Approval No (9 Byte) */
	if (memcmp(srECROb->srTransData.szTransType, _ECR_8N1_FPG_FTC_REPRINT_RECEIPT_, 2))
	{
		if (strlen(pobTran->srBRec.szAuthCode) > 0)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			strcpy(szTemplate, pobTran->srBRec.szAuthCode);
			inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 9, _PADDING_RIGHT_);
			memcpy(&szDataBuffer[inPacketSizes], szTemplate, 9);
		}
	}
	inPacketSizes += 8;

	/* Card Type (1 Byte) */
	if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
		memcpy(&szDataBuffer[inPacketSizes], "4", 1);
	inPacketSizes += 1;
	
	/* Paid Credit Amount (12 Byte) */
	inPacketSizes += 12;

	/* ECR Response Code (4 Byte) */
	memcpy(&szDataBuffer[inPacketSizes], "0000", 4);
	inPacketSizes += 4;

	/* Terminal ID (8 Byte) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTerminalID(szTemplate);
	inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 8, _PADDING_RIGHT_);
	memcpy(&szDataBuffer[inPacketSizes], szTemplate, 8);
	inPacketSizes += 8;

	/* Return Reference Number */
	if (memcmp(srECROb->srTransData.szTransType, _ECR_8N1_FPG_FTC_REPRINT_RECEIPT_, 2))
	{
		if (strlen(pobTran->srBRec.szRefNo) > 0)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szRefNo[0], 12);
		}
	}
	inPacketSizes += 12;
	
	/* Amount of Redeem (12 Byte) */
	inPacketSizes += 12;
	/* Store ID (18 Byte) */
	inPacketSizes += 18;
	/* 油品代號 (6 Byte) */
	inPacketSizes += 6;
	/* 聯名卡 FE01 資料 (63 Byte) */
	if (srECROb->srTransData.srCus005_Data.uszFPG_FTC_CardBit == VS_TRUE)
	{
		/* FE01_會員編號 (10 Byte) */
		memcpy(&szDataBuffer[inPacketSizes], &gsrFPG_FTC_Rec.usMemberNO[0], FPG_FTC_MEMBER_ID_SIZE);
		inPacketSizes += FPG_FTC_MEMBER_ID_SIZE;
		/* FE01_卡別 (2 Byte) */
		memcpy(&szDataBuffer[inPacketSizes], &gsrFPG_FTC_Rec.usCardType[0], FPG_FTC_CARD_TYPE_SIZE);
		inPacketSizes += FPG_FTC_CARD_TYPE_SIZE;
		/* FE01_身分代號 (2 Byte) */
		memcpy(&szDataBuffer[inPacketSizes], &gsrFPG_FTC_Rec.usMemberCode[0], FPG_FTC_MEMBER_CODE_SIZE);
		inPacketSizes += FPG_FTC_MEMBER_CODE_SIZE;
		/* FE01_等級 (2 Byte) */
		memcpy(&szDataBuffer[inPacketSizes], &gsrFPG_FTC_Rec.usRange[0], FPG_FTC_RANGE_SIZE);
		inPacketSizes += FPG_FTC_RANGE_SIZE;
		/* FE01_發票列印統編 (10 Byte) */
		memcpy(&szDataBuffer[inPacketSizes], &gsrFPG_FTC_Rec.usInvoiceNO[0], FPG_FTC_INVOICE_NO_SIZE);
		inPacketSizes += FPG_FTC_INVOICE_NO_SIZE;
		/* FE01_車號 (10 Byte) */
		memcpy(&szDataBuffer[inPacketSizes], &gsrFPG_FTC_Rec.usCardNum[0], FPG_FTC_CARD_NUM_SIZE);
		inPacketSizes += FPG_FTC_CARD_NUM_SIZE;
		/* FE01_油品代號 (2 Byte) */
		memcpy(&szDataBuffer[inPacketSizes], &gsrFPG_FTC_Rec.usOilCode[0], FPG_FTC_OIL_CODE_SIZE);
		inPacketSizes += FPG_FTC_OIL_CODE_SIZE;
		/* FE01_Basic_Data_1 (2 Byte) */
		memcpy(&szDataBuffer[inPacketSizes], &gsrFPG_FTC_Rec.usBasicData1[0], FPG_FTC_BASIC_DATA_SIZE);
		inPacketSizes += FPG_FTC_BASIC_DATA_SIZE;
	}
	else
		inPacketSizes += 63;

	/* 聯名卡 FE02 資料 (54 Byte) */
	if (srECROb->srTransData.srCus005_Data.uszFPG_FTC_CardBit == VS_TRUE)
	{
		/* FE02_姓名 (10 Byet) */
		memcpy(&szDataBuffer[inPacketSizes], &gsrFPG_FTC_Rec.usCustName[0], FPG_FTC_FE02_CUSTOMER_NAME_SIZE);
		inPacketSizes += FPG_FTC_FE02_CUSTOMER_NAME_SIZE;
		/* FE02_生日 (8 Byte) */
		memcpy(&szDataBuffer[inPacketSizes], &gsrFPG_FTC_Rec.usBirthday[0], FPG_FTC_BIRTHDAY_SIZE);
		inPacketSizes += FPG_FTC_BIRTHDAY_SIZE;
		/* FE02_會員卡號 (10 Byte) */
		memcpy(&szDataBuffer[inPacketSizes], &gsrFPG_FTC_Rec.usMemberID[0], FPG_FTC_MEMBER_ID_SIZE);
		inPacketSizes += FPG_FTC_MEMBER_ID_SIZE;
		/* 預留空白 (36 Byte) */
		inPacketSizes += 26;
	}
	else
		inPacketSizes += 54;

	/* 交易類型 (1 Byte) */
	inPacketSizes += 1;
	/* 聯名卡消費日期時間 (14 Byte) */
	inPacketSizes += 14;
	/* 消費通路代號 (8 Byte) */
	inPacketSizes += 8;
	/* 聯名卡消費金額(折扣後) (10 Byte) */
	inPacketSizes += 10;
	/* 加油數量 (10 Byte) */
	inPacketSizes += 10;
	/* 聯名卡積點 (10 Byte) */
	if (srECROb->srTransData.srCus005_Data.uszFPG_FTC_CardBit == VS_TRUE)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(&szTemplate[0], &gsrFPG_FTC_Rec.usEFID1[0], 8);
		inFunc_PAD_ASCII(szTemplate, szTemplate, '0', 10, _PADDING_RIGHT_);
		memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 10);
	}
	inPacketSizes += 10;
	
	/* 聯名卡扣點 (10 Byte) */
	inPacketSizes += 10;
	/* 聯名卡儲值金額 (8 Byte) */
	if (srECROb->srTransData.srCus005_Data.uszFPG_FTC_CardBit == VS_TRUE)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(&szTemplate[0], &gsrFPG_FTC_Rec.usEFID2[0], 8);
		inFunc_PAD_ASCII(szTemplate, szTemplate, '0', 10, _PADDING_RIGHT_);
		memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 8);
	}

	inPacketSizes += 8;
	/* 儲值錢包扣款金額 (8 Byte) */
	inPacketSizes += 8;
	/* 聯名卡基本資料 Update Indicator (1 Byte) */
	inPacketSizes += 1;
	/* EFID1 (4 Byte) */
	inPacketSizes += 4;
	/* EFID2 (4 Byte) */
	inPacketSizes += 4;
	/* HG Card Number (19 Byte) */
	if (srECROb->srTransData.srCus005_Data.uszHappyGoCardBit == VS_TRUE)
	{
		/* 卡號是否遮掩 */
		inRetVal = inECR_CardNoTruncateDecision(pobTran);
		/* 要遮卡號 */
		if (inRetVal == VS_SUCCESS)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
        		strcpy(szTemplate, pobTran->srBRec.szHGPAN);

			if (memcmp(&pobTran->srBRec.szHGPAN[0], "9552", 4))
			{
				/* 【需求單 - 106349】自有品牌判斷需求 add by LingHsiung 2018-02-26 下午 04:28:55 */
				/* 移除原U Card 判斷 */
				inCardLen = strlen(szTemplate);
		                j = 6;

				for (i = j; i < (inCardLen - 4); i ++)
					szTemplate[i] = 0x2A;

				memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], strlen(szTemplate));
			}
			else
			{
				memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szHGPAN[0], strlen(pobTran->srBRec.szHGPAN));
			}
		}
		else
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			strcpy(szTemplate, pobTran->srBRec.szHGPAN);
			/* 【需求單 - 106349】自有品牌判斷需求 add by LingHsiung 2018-02-26 下午 04:28:55 */
			/* 移除原U Card 判斷 */
			memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szHGPAN[0], strlen(pobTran->srBRec.szHGPAN));
		}
	}

	inPacketSizes += 19;
	/* Happy Go Mode (1 Byte) */
	inPacketSizes += 1;
	
	/* Redeem Amount (12 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_, 2) ||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_FPG_FTC_REDEMPTION_REFUND_, 2))
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%012lu", (pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnRedemptionPaidCreditAmount));
		memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 12);
	}

	inPacketSizes += 12;
	/* Redeem Point (12 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_, 2) ||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_FPG_FTC_REDEMPTION_REFUND_, 2))
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%012lu", pobTran->srBRec.lnRedemptionPoints);
		memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 12);
	}

	inPacketSizes += 12;
	/* Balance Point (12 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_, 2) ||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_FPG_FTC_REDEMPTION_REFUND_, 2))
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%012lu", pobTran->srBRec.lnRedemptionPointsBalance);
		memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 12);
	}

	inPacketSizes += 12;
	/* Installment Period (2 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_, 2) ||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_FPG_FTC_INSTALLMENT_REFUND_, 2))
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%02ld", pobTran->srBRec.lnInstallmentPeriod);
		memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 2);
	}

	inPacketSizes += 2;
	/* Formality Fee (10 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_, 2) ||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_FPG_FTC_INSTALLMENT_REFUND_, 2))
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%08lu00", pobTran->srBRec.lnInstallmentFormalityFee);
		memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 10);
	}

	inPacketSizes += 10;
	/* Down Payment (12 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_, 2) ||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_FPG_FTC_INSTALLMENT_REFUND_, 2))
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010lu00", pobTran->srBRec.lnInstallmentDownPayment);
		memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 12);
	}

	inPacketSizes += 12;
	/* Installment Payment (12 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_, 2) ||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_FPG_FTC_INSTALLMENT_REFUND_, 2))
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010lu00", pobTran->srBRec.lnInstallmentPayment);
		memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 12);
	}

	inPacketSizes += 12;
	/* 發票號碼 (10 Byte) */
	inPacketSizes += 10;
	/* Reserve (35 Byte) */
	inPacketSizes += 35;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inECR_8N1_Customer_005_FPG_Pack() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inECR_8N1_Customer_005_FPG_Pack_ResponseCode
Date&Time       :2022/10/4 下午 5:05
Describe        :先把要送的資料組好
*/
int inECR_8N1_Customer_005_FPG_Pack_ResponseCode(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuf)
{
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inECR_8N1_Customer_005_FPG_Pack_ResponseCode() START !");
	}
	
	if (srECROb->srTransData.inErrorType == VS_CALLBANK	||
	    srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_CALLBANK_)
	{
		memcpy(&szDataBuf[78], "0002", 4);
	}
	else if (srECROb->srTransData.inErrorType == VS_TIMEOUT		||
		 srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_TIMEOUT_)
	{
		memcpy(&szDataBuf[78], "0003", 4);
	}
	else if (srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_CARD_TYPR_NOT_MATCHED) 
	{
		memcpy(&szDataBuf[78], "0004", 4);
	}
	else if (srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_EMV_ERROR)
	{
		memcpy(&szDataBuf[78], "0005", 4);
	}
	else if (srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_MAGENTIC_STRIPE_ERROR)
	{
		memcpy(&szDataBuf[78], "0006", 4);
	}
	else if (srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_BALANCE_NOT_ENOUGH)
	{
		memcpy(&szDataBuf[78], "0007", 4); 
	}
	else if (srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_CARD_NUMBER_NOT_MATCH)
	{
		memcpy(&szDataBuf[78], "0008", 4);
	}
	else if (srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_UPDATE_CARD_FAIL)
	{
		memcpy(&szDataBuf[78], "0009", 4);
	}
	else if (srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_RECORD_NOT_FOUND)
	{
		memcpy(&szDataBuf[78], "0010", 4);
	}
	else if (srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_READ_CARD_ERROR)
	{
		memcpy(&szDataBuf[78], "0011", 4);
	}
	else if (srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_CARD_NUMBER_ERROR)
	{
		memcpy(&szDataBuf[78], "0012", 4);
	}
	else if (srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_HG_REDEEM_POINT_ERROR)
	{
		memcpy(&szDataBuf[78], "0013", 4);
	}
	else
	{
		memcpy(&szDataBuf[78], "0001", 4);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inECR_7E1_Customer_026_TAKA_Unpack
Date&Time       :2022/10/11 下午 8:28
Describe        :分析收銀機傳來的資料
*/
int inECR_7E1_Customer_026_TAKA_Unpack(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, ECR_TABLE * srECROb)
{
	int	i = 0;
	int	inRetVal = VS_SUCCESS;
	int	inTransType = 0;		/* 電文中的交易別字串轉為數字儲存 */
	int	inCardInquiryTRTType = _CARD_INQUIRY_TYPE_NONE_;
	char	szTemplate[100 + 1] = {0};
	char	szDebugMsg[100 + 1] = {0};	
	char	szHostLabel[8 + 1] = {0};
	char	szHostEnable[2 + 1] = {0};
	char	szBatchNum[6 + 1] = {0};
	char	szCTLSEnable[2 + 1] = {0};
	char	szFiscContactlessEnable[1 + 1] = {0};
	char	szCUPContactlessEnable[1 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
	
	/* (需求單 - 107276)自助交易標準做法 EDC於發動交易及完成時，需發出提示音 by Russell 2019/3/8 上午 10:46 */
	inDISP_BEEP(1, 0);
	
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
        inGetCustomIndicator(szCustomerIndicator);
	
	/* 存ECR原始資料 */
	memcpy(srECROb->srTransData.szOrgData, szDataBuffer, _ECR_7E1_Standard_Data_Size_);
	
	inECR_7E1_Standard_Parse_Data(srECROb, szDataBuffer);
	
	/* Trans Type (交易別) */
	switch (inTransType)
	{
		default:
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szTransType[0], 2);
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s: %s  ", "Trans Type", szTemplate);
				inLogPrintf(AT, szDebugMsg);
			}
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Type", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				sprintf(pobTran->szErrorMsgBuff1, "%02d", inTransType);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "Trans Type Not Exist Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
				return (VS_ERROR);
			}
			
			/* 交易類別轉成數字 */
			inTransType = atoi(srECROb->srTransData.szTransType);
			
			/* 要連動結帳 */
			if (inTransType == _ECR_7E1_SETTLEMENT_NO_)
			{
				pobTran->uszAutoSettleBit = VS_TRUE;
			}
			
			break;
	}
	
	/* 銀行別/付款別 HOST ID*/
	switch (inTransType)
	{
		case _ECR_7E1_VOID_NO_:				/* 取消交易 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szHostID[0], 2);
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "HOST ID", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "%s: %s  ", "HOST ID", "Not Exist Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
					
				return (VS_ERROR);
			}
			
			memset(szHostLabel, 0x00, sizeof(szHostLabel));

			if (memcmp(szTemplate, _ECR_7E1_NCCC_HOSTID_NCCC_, 2) == 0)
			{
				memcpy(szHostLabel, _HOST_NAME_CREDIT_NCCC_, strlen(_HOST_NAME_CREDIT_NCCC_));
			}
			else if (memcmp(szTemplate, _ECR_7E1_NCCC_HOSTID_CUS_026_DCC_, 2) == 0)
			{
				memcpy(szHostLabel, _HOST_NAME_DCC_, strlen(_HOST_NAME_DCC_));
			}
			else if (memcmp(szTemplate, _ECR_7E1_NCCC_HOSTID_CUS_026_TAKA_, 2) == 0)
			{
				memcpy(szHostLabel, _HOST_NAME_TAKA_, strlen(_HOST_NAME_TAKA_));
			}
			/* 2018/5/8 下午 2:10 DFS需求不再使用大來主機 */
			else
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "無對應Host ID");
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				sprintf(pobTran->szErrorMsgBuff1, "%02d", inTransType);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "%s : HOST ID Not Pair", szTemplate);
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
					
				return (VS_ERROR);
			}
			
			/* 初始化 */
			i = 0;
			do
			{
				/* 按順序load每一個HOST */
				if (inLoadHDTRec(i) < 0)
				{
					memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
					sprintf(pobTran->szErrorMsgBuff1, "%02d", inTransType);
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "%s : HOST ID Not Found", szHostLabel);
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
					/* 如果return VS_ERROR 代表table沒有該HOST */
					return (VS_ERROR);
				}

				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetHostLabel(szTemplate);
				if (!memcmp(szTemplate, szHostLabel, strlen(szHostLabel)))
				{
					/* 比對成功後，判斷HOST是否開啟  */
					memset(szHostEnable, 0x00, sizeof(szHostEnable));
					inGetHostEnable(szHostEnable);
					if (szHostEnable[0] != 'Y')
					{
						/* Host沒開 */
						if (ginDebug == VS_TRUE) 
						{
							memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
							sprintf(szDebugMsg, "%s Not Open Error", szHostLabel);
							inLogPrintf(AT, szDebugMsg);
						}
						memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
						sprintf(pobTran->szErrorMsgBuff1, "%02d", inTransType);
						memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
						sprintf(pobTran->szErrorMsgBuff2, "%s : HOST ID Not Open", szHostLabel);
						pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

						/* 如果return VS_ERROR 代表Host沒開 */
						return (VS_ERROR);
					}
					else
					{
						/* Host有開 */
						pobTran->srBRec.inHDTIndex = i;
						
						/* Load HDPT */
						inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
						memset(szBatchNum, 0x00, sizeof(szBatchNum));
						inGetBatchNum(szBatchNum);
						pobTran->srBRec.lnBatchNum = atol(szBatchNum);
						
						if (ginDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
							sprintf(szDebugMsg, "%s Open", szHostLabel);
							inLogPrintf(AT, szDebugMsg);
						}
						
					}/* Host Enable比對End */

					break;
				}/* Host Label比對 End */
				i++;

			} while (1);
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%d", "HDTindex", pobTran->srBRec.inHDTIndex);
				inLogPrintf(AT, szDebugMsg);
			}
			
			break;
			
		default:
			break;
	}
	
	/* EDC簽單序號 Receipt No(InvoiceNumber) */
	switch (inTransType)
	{
		case _ECR_7E1_VOID_NO_:				/* 取消交易 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &srECROb->srTransData.szReceiptNo[0], 6);
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Receipt No", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				sprintf(pobTran->szErrorMsgBuff1, "%02d", inTransType);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "Receipt No Not Exist Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
				return (VS_ERROR);
			}
			else
			{
				pobTran->srBRec.lnOrgInvNum = atol(szTemplate);
			}
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%ld", "lnOrgInvNum", pobTran->srBRec.lnOrgInvNum);
				inLogPrintf(AT, szDebugMsg);
			}
			
			break;
		default:
			break;
	}
		
	/* 處理交易金額 (Trans Amount) */
	switch (inTransType)
	{
		case _ECR_7E1_SALE_NO_:
		case _ECR_7E1_OFFLINE_NO_:
		case _ECR_7E1_REFUND_NO_:
		case _ECR_7E1_INSTALLMENT_NO_:
		case _ECR_7E1_REDEEM_NO_:
		case _ECR_7E1_INSTALLMENT_REFUND_NO_:
		case _ECR_7E1_REDEEM_REFUND_NO_:
                case _ECR_7E1_START_CARD_NO_INQUIRY_NO_:
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &srECROb->srTransData.szField_10[0], 10);
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Amount", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "Trans Amount Not Exist Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
				return (VS_ERROR);
			}
			else if ((!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_076_8_DIGITS_, _CUSTOMER_INDICATOR_SIZE_) && atol(szTemplate) > 99999999) || 
				 (memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_076_8_DIGITS_, _CUSTOMER_INDICATOR_SIZE_) && atol(szTemplate) > 9999999))
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Amount", "Out of bound");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_076_8_DIGITS_, _CUSTOMER_INDICATOR_SIZE_))
				{
					sprintf(pobTran->szErrorMsgBuff2, "金額超過8位數");
				}
				else
				{
					sprintf(pobTran->szErrorMsgBuff2, "金額超過7位數");
				}
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
				
				return (VS_ERROR);
			}
			else
			{
				pobTran->srBRec.lnTxnAmount = atol(szTemplate);
				pobTran->srBRec.lnOrgTxnAmount = atol(szTemplate);
				pobTran->srBRec.lnTotalTxnAmount = atol(szTemplate);
			}

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%ld", "交易金額", pobTran->srBRec.lnTxnAmount);
				inLogPrintf(AT, szDebugMsg);
			}
			break;
		default:
			break;
	}
	
	/* 授權碼(Auth Code) ECR電文中給9個字元(左靠右補空白) 但ATS電文只需要6個，所以只抓6個(交易補登及所有退貨須送此欄位) */
	switch (inTransType)
	{
		case _ECR_7E1_REFUND_NO_:			/* 退貨 */
		case _ECR_7E1_OFFLINE_NO_:			/* 交易補登 */
		case _ECR_7E1_INSTALLMENT_REFUND_NO_:		/* 分期退貨 */
		case _ECR_7E1_REDEEM_REFUND_NO_:		/* 紅利退貨 */
			/* 處理授權碼(optional) */
			memset(pobTran->srBRec.szAuthCode, 0x00, sizeof(pobTran->srBRec.szAuthCode));
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szApprovalNo[0], 6);
			memcpy(pobTran->srBRec.szAuthCode, szTemplate, 6);
			
			break;
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Auth Code", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "Auth Code Not Exist Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
				return (VS_ERROR);
			}
			else
			{
				memcpy(pobTran->srBRec.szAuthCode, szTemplate, 6);
			}
			
			break;
			
		default :
			break;
	}
	
	/* szDataBuffer[66] 144規格要看紅利分期來看使用用途  */
	switch (inTransType)
	{
		case _ECR_7E1_REDEEM_REFUND_NO_:		/* 紅利退貨 */
		case _ECR_7E1_REDEEM_ADJUST_NO_:		/* 紅利調帳 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szRDMPaidAmt[0], 10);	/* 12位數，這裡只取10位 */
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "RDM Paid Amt", szTemplate);
				inLogPrintf(AT, szDebugMsg);
			}
			/* 實際支付金額 */
			pobTran->srBRec.lnRedemptionPaidCreditAmount = atol(szTemplate);
			
			break;	
		case _ECR_7E1_INSTALLMENT_REFUND_NO_:		/* 分期退貨 */
		case _ECR_7E1_INSTALLMENT_ADJUST_NO_:		/* 分期調帳 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szField_28[0], 10);	/* 12位數，這裡只取10位 */

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "Formallity Fee", szTemplate);
				inLogPrintf(AT, szDebugMsg);
			}
			pobTran->srBRec.lnInstallmentFormalityFee = atol(szTemplate);

			if (pobTran->srBRec.lnInstallmentFormalityFee != 0)
			{
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_INST_FEE_NOT_0_;
				pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;		/* 操作錯誤 */
				return (VS_ERROR);
			}

			break;
		default :
			break;
	}
	
	/* 分期期數 Installment Period */
	switch (inTransType)
	{
		case _ECR_7E1_INSTALLMENT_NO_:			/* 分期 */
		case _ECR_7E1_INSTALLMENT_REFUND_NO_:		/* 分期退貨 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szInstallmentPeriod[0], 2);
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "Installment Period", szTemplate);
				inLogPrintf(AT, szDebugMsg);
			}
			/* 分期期數(右靠左補0) */
			pobTran->srBRec.lnInstallmentPeriod = atol(szTemplate);
			
			break;
			
		default :
			break;
	}
	
	/* szDataBuffer[102] 144規格要看紅利分期來看使用用途 保留欄位/首期金額／紅利扣抵點數  */
	switch (inTransType)
	{
		case _ECR_7E1_REDEEM_REFUND_NO_:		/* 紅利退貨 */
			/* 最多八位 所以從106開始 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szRDMPoint[4], 8);
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "RDM Point", szTemplate);
				inLogPrintf(AT, szDebugMsg);
			}
			
			/* 信用卡紅利扣抵點數 RDM Point */
			pobTran->srBRec.lnRedemptionPoints = atol(szTemplate);
			break;
		case _ECR_7E1_INSTALLMENT_REFUND_NO_:		/* 分期退貨 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szField_26[0], 10);	/* 12位數，這裡只取10位 */
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "Down Payment Amount", szTemplate);
				inLogPrintf(AT, szDebugMsg);
			}
			/* 首期金額 Down Payment Amount (含小數 2位) */
			pobTran->srBRec.lnInstallmentDownPayment = atol(szTemplate);
			
			break;
		default :
			break;
	}
	
	
	
	/* 處理櫃號(Store ID) */
	switch (inTransType)
	{
		default:
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szStoreId[0], 18);
			
			if (strlen(szTemplate) > 0)
			{
				memcpy(pobTran->srBRec.szStoreID, szTemplate, strlen(szTemplate));
				/* 櫃號不滿18，補空白 */
				if (strlen(szTemplate) < 18)
				{
					memset(&pobTran->srBRec.szStoreID[strlen(szTemplate)], 0x20, 18 - strlen(szTemplate));
				}

			}
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "Store ID", pobTran->srBRec.szStoreID);
				inLogPrintf(AT, szDebugMsg);
			}
			break;
	}
	
	/* szDataBuffer[132] 144規格要看紅利分期來看使用用途  卡號查詢/每期金額/保留欄位 */
	switch (inTransType)
	{
		case _ECR_7E1_INSTALLMENT_REFUND_NO_:		/* 分期退貨 */
		case _ECR_7E1_INSTALLMENT_ADJUST_NO_:		/* 分期調帳 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szField_27[0], 10);	/* 12位數，這裡只取10位 */
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "Payment Amount", szTemplate);
				inLogPrintf(AT, szDebugMsg);
			}
			/* 每期金額 Installment Payment Amount (含小數 2位) */
			pobTran->srBRec.lnInstallmentPayment = atol(szTemplate);
			
			break;
		default:
			break;
	}
	
	/* 若前一次為卡號查詢，標記為第二次ECR */
	if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
	{
		pobTran->uszCardInquiryFirstBit = VS_FALSE;
		pobTran->uszCardInquirysSecondBit = VS_TRUE;
		
		/* 重新決定TRT流程 */
                if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
                {
                        /* 金融卡查詢要二次過卡，不依循前次查詢界面 */
                        inCardInquiryTRTType = _CARD_INQUIRY_TYPE_NONE_;
                }
                else
                {
                        if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
                        {
                                inCardInquiryTRTType = _CARD_INQUIRY_TYPE_CHIP_;
                        }
                        else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
                        {
                                inCardInquiryTRTType = _CARD_INQUIRY_TYPE_CTLS_;
                        }
                        else
                        {
                                inCardInquiryTRTType = _CARD_INQUIRY_TYPE_MEG_;
                        }
                }
	}
	
	/* szDataBuffer[142] 144規格要看紅利分期來看使用用途  卡號查詢/每期金額/保留欄位 */
	switch (inTransType)
	{
		/* 啟動卡號查詢 Start Trans Type */
		case _ECR_7E1_START_CARD_NO_INQUIRY_NO_:		/* 卡號查詢 */
			/* 卡號查詢 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &srECROb->srTransData.szStartTransType[0], 2);
			inTransType = atoi(szTemplate);
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "Start Trans Type", szTemplate);
				inLogPrintf(AT, szDebugMsg);
			}
			
			switch (inTransType)
			{
				case _ECR_7E1_SALE_NO_ :
				case _ECR_7E1_REFUND_NO_ :
				case _ECR_7E1_PREAUTH_NO_ :
					if (pobTran->uszCardInquirysSecondBit == VS_TRUE)
					{

					}
					else
					{
						pobTran->uszCardInquiryFirstBit = VS_TRUE;
					}
					/* 有值影響到後面判斷  確保都是空格 */
					memcpy(&srECROb->srTransData.szField_05[0], " ", 1);
					memcpy(&srECROb->srTransData.szTransTypeIndicator[0], " ", 1);
					break;
				case _ECR_7E1_OFFLINE_NO_ :
				case _ECR_7E1_INSTALLMENT_NO_ :
				case _ECR_7E1_REDEEM_NO_ :
				case _ECR_7E1_INSTALLMENT_REFUND_NO_ :
				case _ECR_7E1_REDEEM_REFUND_NO_ :
				case _ECR_7E1_INSTALLMENT_ADJUST_NO_ :
				case _ECR_7E1_REDEEM_ADJUST_NO_ :
					if (pobTran->uszCardInquirysSecondBit == VS_TRUE)
					{

					}
					else
					{
						pobTran->uszCardInquiryFirstBit = VS_TRUE;
					}
					memcpy(&srECROb->srTransData.szField_05[0], " ", 1);
					memcpy(&srECROb->srTransData.szTransTypeIndicator[0], " ", 1);
					break;
				default :
					pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
					return (VS_ERROR);
			}
			break;
		default:
			break;
	}
	
	if (inECR_Check_Exception(pobTran) != VS_SUCCESS)
	{
		memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
		sprintf(pobTran->szErrorMsgBuff2, "ECR_Check_Exception error");
		return (VS_ERROR);
	}
	
	memset(szFiscContactlessEnable, 0x00, sizeof(szFiscContactlessEnable));
	inGetSmartPayContactlessEnable(szFiscContactlessEnable);
	memset(szCTLSEnable, 0x00, sizeof(szCTLSEnable));
	inGetContactlessEnable(szCTLSEnable);
	memset(szCUPContactlessEnable, 0x00, sizeof(szCUPContactlessEnable));
	inGetCUPContactlessEnable(szCUPContactlessEnable);
	
	/* 跑OPT */
	switch (inTransType)
	{
		case _ECR_7E1_SALE_NO_:
			/* 金融卡和一般Sale跑不同OPT，所以要分 */
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{	
				inDISP_ClearAll();
				/* 第三層顯示 ＜消費扣款＞ */
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_FISC_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜消費扣款＞ */

				pobTran->inFunctionID = _FISC_SALE_;
				if (!memcmp(szCTLSEnable, "Y", 1)		&&
				    !memcmp(szFiscContactlessEnable, "Y", 1)	&&
				    guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_FISC_SALE_CTLS_;
					pobTran->inRunTRTID = _TRT_FISC_SALE_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_FISC_SALE_;
					pobTran->inRunTRTID = _TRT_FISC_SALE_ICC_;
				}
		
				pobTran->inTransactionCode = _FISC_SALE_;
				pobTran->srBRec.inCode = _FISC_SALE_;
				pobTran->srBRec.inOrgCode = _FISC_SALE_;
                                
                                /* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_CUP_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜銀聯一般交易＞ */

				if (!memcmp(szCTLSEnable, "Y", 1)		&&
				    !memcmp(szCUPContactlessEnable, "Y", 1)	&&
				    guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
					pobTran->inRunTRTID = _TRT_CUP_SALE_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
					pobTran->inRunTRTID = _TRT_CUP_SALE_;
				}

				pobTran->inFunctionID = _CUP_SALE_;
				pobTran->inTransactionCode = _CUP_SALE_;
				pobTran->srBRec.inCode = pobTran->inTransactionCode;
				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
                                
                                /* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_TICKET_DEDUCT_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜購貨交易＞ */

				pobTran->inFunctionID = _TICKET_DEDUCT_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_TICKET_DEDUCT_;
				}
				else
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "感應未初始化，不能使用此功能");
					}
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "CTLS not Init");
					return (VS_ERROR);
				}

				pobTran->inTransactionCode = _TICKET_DEDUCT_;
				pobTran->srTRec.inCode = _TICKET_DEDUCT_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else
			{
				inDISP_ClearAll();
				/* 第三層顯示 ＜一般交易＞ */
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般交易＞ */

				pobTran->inFunctionID = _SALE_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
					pobTran->inRunTRTID = _TRT_SALE_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
					pobTran->inRunTRTID = _TRT_SALE_;
				}

				pobTran->inTransactionCode = _SALE_;
				pobTran->srBRec.inCode = _SALE_;
				pobTran->srBRec.inOrgCode = _SALE_;
                                
                                /* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			
			break;
		case _ECR_7E1_VOID_NO_:
			/* 金融卡和一般Sale跑不同OPT，所以要分 */
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_FISC_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜沖正交易＞ */

				pobTran->inFunctionID = _FISC_VOID_;
				pobTran->inRunOperationID = _OPERATION_VOID_;
				pobTran->inRunTRTID = _TRT_FISC_VOID_;

				pobTran->inTransactionCode = _FISC_VOID_;
				pobTran->srBRec.inCode = _FISC_VOID_;
				pobTran->srBRec.inOrgCode = _FISC_VOID_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜取消交易＞ */

				pobTran->inRunOperationID = _OPERATION_VOID_;
				pobTran->inRunTRTID = _TRT_CUP_VOID_;

				pobTran->inFunctionID = _CUP_VOID_;
				pobTran->inTransactionCode = _CUP_VOID_;
				pobTran->srBRec.inCode = pobTran->inTransactionCode;
				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜取消交易＞ */

				pobTran->inFunctionID = _VOID_;
				pobTran->inRunOperationID = _OPERATION_VOID_;
				pobTran->inRunTRTID = _TRT_VOID_;

				pobTran->inTransactionCode = _VOID_;
				pobTran->srBRec.inCode = _VOID_;
				pobTran->srBRec.inOrgCode = _VOID_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			break;
		case _ECR_7E1_REFUND_NO_:
			/* 金融卡和一般Sale跑不同OPT，所以要分 */
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_FISC_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);	/* 第二層顯示 ＜退費交易＞ */

				pobTran->inFunctionID = _FISC_REFUND_;                       
				if (!memcmp(szCTLSEnable, "Y", 1)		&&
				    !memcmp(szFiscContactlessEnable, "Y", 1)	&&
				    guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_FISC_REFUND_CTLS_;
					pobTran->inRunTRTID = _TRT_FISC_REFUND_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_FISC_REFUND_;
					pobTran->inRunTRTID = _TRT_FISC_REFUND_ICC_;
				}

				pobTran->inTransactionCode = _FISC_REFUND_;
				pobTran->srBRec.inCode = _FISC_REFUND_;
				pobTran->srBRec.inOrgCode = _FISC_REFUND_;
                                
                                /* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			else if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般退貨＞ */

				if (!memcmp(szCTLSEnable, "Y", 1)		&&
				    !memcmp(szCUPContactlessEnable, "Y", 1)	&&
				    guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_GET_CARD_FIRST_;
					pobTran->inRunTRTID = _TRT_CUP_REFUND_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_AMOUNT_FIRST_CUP_;
					pobTran->inRunTRTID = _TRT_CUP_REFUND_;
				}

				pobTran->inFunctionID = _CUP_REFUND_;
				pobTran->inTransactionCode = _CUP_REFUND_;
				pobTran->srBRec.inCode = pobTran->inTransactionCode;
				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
                                
                                /* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_TICKET_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜退貨交易＞ */

				pobTran->inFunctionID = _TICKET_REFUND_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_TICKET_REFUND_;
				}
				else
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "感應未初始化，不能使用此功能");
					}
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "CTLS not Init");
					return (VS_ERROR);
				}

				pobTran->inTransactionCode = _TICKET_REFUND_;
				pobTran->srTRec.inCode = _TICKET_REFUND_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般退貨＞ */

				pobTran->inFunctionID = _REFUND_;

				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_GET_CARD_FIRST_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_AMOUNT_FIRST_;
				}
				pobTran->inRunTRTID = _TRT_REFUND_;

				pobTran->inTransactionCode = _REFUND_;
				pobTran->srBRec.inCode = _REFUND_;
				pobTran->srBRec.inOrgCode = _REFUND_;
                                
                                /* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;
			
		case _ECR_7E1_OFFLINE_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_SALE_OFFLINE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜交易補登＞ */
			
			pobTran->inFunctionID = _SALE_OFFLINE_;
			pobTran->inRunOperationID = _OPERATION_SALE_OFFLINE_;
			pobTran->inRunTRTID = _TRT_SALE_OFFLINE_;

			pobTran->inTransactionCode = _SALE_OFFLINE_;
			pobTran->srBRec.inCode = _SALE_OFFLINE_;
			pobTran->srBRec.inOrgCode = _SALE_OFFLINE_;
                        
                        /* 卡號查詢不過第二次卡，所以要直接決定TRT */
                        if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
                        {
                                inNCCC_Func_Decide_CTLS_TRT(pobTran);
                        }
                        else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
                        {
                                inNCCC_Func_Decide_ICC_TRT(pobTran);
                        }
                        else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
                        {
                                inNCCC_Func_Decide_MEG_TRT(pobTran);
                        }
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			break;
		case _ECR_7E1_INSTALLMENT_NO_:
//			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
//			{
//				inDISP_ClearAll();
//				/* 保留給銀聯分期使用 */
//				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
//				inDISP_PutGraphic(_MENU_INST_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜分期付款＞ */
//
//				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
//				{
//					pobTran->inRunOperationID = _OPERATION_INST_SALE_CTLS_;
//				}
//				else
//				{
//					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
//				}
//				pobTran->inRunTRTID = _TRT_INST_SALE_;
//
//				pobTran->inFunctionID = _CUP_INST_SALE_;
//				pobTran->inTransactionCode = _CUP_INST_SALE_;
//				pobTran->srBRec.inCode = pobTran->inTransactionCode;
//				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
//
//				pobTran->srBRec.uszInstallmentBit = VS_TRUE;
//                              /* 卡號查詢不過第二次卡，所以要直接決定TRT */
//				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
//				{
//					inNCCC_Func_Decide_CTLS_TRT(pobTran);
//				}
//				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
//				{
//					inNCCC_Func_Decide_ICC_TRT(pobTran);
//				}
//				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
//				{
//					inNCCC_Func_Decide_MEG_TRT(pobTran);
//				}
//
//				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
//			}
//			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_INST_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜分期付款＞ */

				pobTran->inFunctionID = _INST_SALE_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_INST_SALE_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
				}
				pobTran->inRunTRTID = _TRT_INST_SALE_;

				pobTran->inTransactionCode = _INST_SALE_;
				pobTran->srBRec.inCode = _INST_SALE_;
				pobTran->srBRec.inOrgCode = _INST_SALE_;

				pobTran->srBRec.uszInstallmentBit = VS_TRUE;
                                /* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;
			
		case _ECR_7E1_INSTALLMENT_REFUND_NO_:
//			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
//			{
//				inDISP_ClearAll();
//				/* 保留給銀聯分期退貨使用 */
//				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
//				inDISP_PutGraphic(_MENU_INST_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜分期退貨＞ */
//
//				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
//				{
//					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_GET_CARD_FIRST_;
//				}
//				else
//				{
//					pobTran->inRunOperationID = _OPERATION_REFUND_;
//				}
//				pobTran->inRunTRTID = _TRT_INST_REFUND_;
//
//				pobTran->inFunctionID = _CUP_INST_REFUND_;
//				pobTran->inTransactionCode = _CUP_INST_REFUND_;
//				pobTran->srBRec.inCode = pobTran->inTransactionCode;
//				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
//
//				pobTran->srBRec.uszInstallmentBit = VS_TRUE;
//                                /* 卡號查詢不過第二次卡，所以要直接決定TRT */
//				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
//				{
//					inNCCC_Func_Decide_CTLS_TRT(pobTran);
//				}
//				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
//				{
//					inNCCC_Func_Decide_ICC_TRT(pobTran);
//				}
//				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
//				{
//					inNCCC_Func_Decide_MEG_TRT(pobTran);
//				}
//
//				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
//			}
//			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_INST_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜分期退貨＞ */

				pobTran->inFunctionID = _INST_REFUND_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_GET_CARD_FIRST_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_;
				}
				pobTran->inRunTRTID = _TRT_INST_REFUND_;

				pobTran->inTransactionCode = _INST_REFUND_;
				pobTran->srBRec.inCode = _INST_REFUND_;
				pobTran->srBRec.inOrgCode = _INST_REFUND_;

				pobTran->srBRec.uszInstallmentBit = VS_TRUE;
                                /* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;
		case _ECR_7E1_REDEEM_NO_:
//			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
//			{
//				inDISP_ClearAll();
//				/* 保留給銀聯紅利扣抵使用 */
//				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
//				inDISP_PutGraphic(_MENU_REDEEM_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利扣抵＞ */
//
//				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
//				{
//					pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
//				}
//				else
//				{
//					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
//				}
//				pobTran->inRunTRTID = _TRT_REDEEM_SALE_;
//
//				pobTran->inFunctionID = _CUP_REDEEM_SALE_;
//				pobTran->inTransactionCode = _CUP_REDEEM_SALE_;
//				pobTran->srBRec.inCode = pobTran->inTransactionCode;
//				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
//
//				pobTran->srBRec.uszRedeemBit = VS_TRUE;
//                              /* 卡號查詢不過第二次卡，所以要直接決定TRT */
//				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
//				{
//					inNCCC_Func_Decide_CTLS_TRT(pobTran);
//				}
//				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
//				{
//					inNCCC_Func_Decide_ICC_TRT(pobTran);
//				}
//				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
//				{
//					inNCCC_Func_Decide_MEG_TRT(pobTran);
//				}
//
//				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
//			}
//			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REDEEM_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利扣抵＞ */

				pobTran->inFunctionID = _REDEEM_SALE_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
				}
				pobTran->inRunTRTID = _TRT_REDEEM_SALE_;

				pobTran->inTransactionCode = _REDEEM_SALE_;
				pobTran->srBRec.inCode = _REDEEM_SALE_;
				pobTran->srBRec.inOrgCode = _REDEEM_SALE_;

				pobTran->srBRec.uszRedeemBit = VS_TRUE;
                                /* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;
		
		case _ECR_7E1_REDEEM_REFUND_NO_:
//			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
//			{
//				inDISP_ClearAll();
//				/* 保留給銀聯紅利退貨使用 */
//				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
//				inDISP_PutGraphic(_MENU_REDEEM_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利退貨＞ */
//
//				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
//				{
//					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_GET_CARD_FIRST_;
//				}
//				else
//				{
//					pobTran->inRunOperationID = _OPERATION_REFUND_;
//				}
//				pobTran->inRunTRTID = _TRT_REDEEM_REFUND_;
//
//				pobTran->inFunctionID = _CUP_REDEEM_REFUND_;
//				pobTran->inTransactionCode = _CUP_REDEEM_REFUND_;
//				pobTran->srBRec.inCode = pobTran->inTransactionCode;
//				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
//
//				pobTran->srBRec.uszRedeemBit = VS_TRUE;
//                              /* 卡號查詢不過第二次卡，所以要直接決定TRT */
//				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
//				{
//					inNCCC_Func_Decide_CTLS_TRT(pobTran);
//				}
//				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
//				{
//					inNCCC_Func_Decide_ICC_TRT(pobTran);
//				}
//				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
//				{
//					inNCCC_Func_Decide_MEG_TRT(pobTran);
//				}
//
//				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
//			}
//			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REDEEM_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利退貨＞ */

				pobTran->inFunctionID = _REDEEM_REFUND_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_GET_CARD_FIRST_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_;
				}
				pobTran->inRunTRTID = _TRT_REDEEM_REFUND_;

				pobTran->inTransactionCode = _REDEEM_REFUND_;
				pobTran->srBRec.inCode = _REDEEM_REFUND_;
				pobTran->srBRec.inOrgCode = _REDEEM_REFUND_;

				pobTran->srBRec.uszRedeemBit = VS_TRUE;
                                /* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;
			
		case _ECR_7E1_SETTLEMENT_NO_:
			pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
			pobTran->inErrorMsg = _ERROR_CODE_V3_FUNC_NOT_SUPPORT_;
			inRetVal = VS_ERROR;
			break;
		case _ECR_7E1_END_CARD_NO_INQUIRY_NO_:
			pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_END_INQUIRY_SUCCESS_;
			inRetVal = VS_ERROR;
			break;
		default:
			pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
			inRetVal = VS_ERROR;
			break;
	}
	
	inECR_ReCheck_144_TRT_Flow(pobTran);
	
	return (inRetVal);
}

/*
Function        :inECR_7E1_Customer_026_TAKA_Pack
Date&Time       :2022/10/12 上午 10:03
Describe        :先把要送的資料組好
*/
int inECR_7E1_Customer_026_TAKA_Pack(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_SUCCESS;
	int 	i = 0, inCardLen = 0;
	int	inPacketSizes = 0;
	char 	szTemplate[100 + 1] = {0};
	
	if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
	{
		strcpy(srECROb->srTransData.szField_05, "C");
	}
	
	/* Trans Type (2 Byte) */
	memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szTransType, 2);
	inPacketSizes += 2;

	/* HOST ID (2 Byte) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTRTFileName(szTemplate);

	if (!memcmp(szTemplate, _TRT_FILE_NAME_CREDIT_, strlen(_TRT_FILE_NAME_CREDIT_)))
		memcpy(&szDataBuffer[inPacketSizes], _ECR_7E1_NCCC_HOSTID_NCCC_, 2);
	/* 2018/5/8 下午 2:10 DFS需求不再使用大來主機 */
	else if (!memcmp(szTemplate, _TRT_FILE_NAME_DCC_, strlen(_TRT_FILE_NAME_DCC_)))
		memcpy(&szDataBuffer[inPacketSizes], _ECR_7E1_NCCC_HOSTID_CUS_026_DCC_, 2);
	else if (!memcmp(szTemplate, _TRT_FILE_NAME_TAKA_, strlen(_TRT_FILE_NAME_TAKA_)))
		memcpy(&szDataBuffer[inPacketSizes], _ECR_7E1_NCCC_HOSTID_CUS_026_TAKA_, 2);

	inPacketSizes += 2;

	/* Receipt No (6 Byte) */
	if (pobTran->srBRec.lnOrgInvNum != 0L)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%06ld", pobTran->srBRec.lnOrgInvNum);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 6);
	}
	inPacketSizes += 6;

	/* Card No (19 Byte)(左靠右補空白) */
	/* 卡號是否遮掩 */
	inRetVal = inECR_CardNoTruncateDecision(pobTran);

	/* 要遮卡號 */
	if (inRetVal == VS_SUCCESS)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		strcpy(szTemplate, pobTran->srBRec.szPAN);

		/* HAPPG_GO 卡不掩飾 */
		/* 大高社員卡卡號不遮掩 */
		if (!memcmp(&pobTran->srBRec.szPAN[0], "9552", 4) ||
		    pobTran->srBRec.uszTakaTransBit == VS_TRUE)
		{

		}
		else
		{
			/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
			/* 卡號長度 */
			inCardLen = strlen(szTemplate);

			/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
			for (i = 6; i < (inCardLen - 4); i ++)
				szTemplate[i] = '*';
		}

		memcpy(&szDataBuffer[inPacketSizes], szTemplate, strlen(szTemplate));
	}
	else
	{
		memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szPAN, strlen(pobTran->srBRec.szPAN));
	}

	inPacketSizes += 19;

	/* Card Expire Date (4 Byte) */
	/* 有效期是否回傳 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetECRExpDateReturnEnable(szTemplate);

	/* 要回傳有效期 */
	if (!memcmp(&szTemplate, "Y", 1))
		memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szExpDate, strlen(pobTran->srBRec.szExpDate));

	inPacketSizes += 4;

	/* Trans Amount (12 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_SALE_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REFUND_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_OFFLINE_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_INSTALLMENT_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_INSTALLMENT_REFUND_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REDEEM_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REDEEM_REFUND_, 2))
	{
		if (pobTran->srBRec.lnTxnAmount != 0)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%010ld00", pobTran->srBRec.lnTxnAmount);
			memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
		}
	}
	else
	{
		/* 要補0 */
		memcpy(&szDataBuffer[inPacketSizes], "000000000000", 12);
	}
	inPacketSizes += 12;

	/* Trans Date (6 Byte) & Trans Time (6 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_SALE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REFUND_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_VOID_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_OFFLINE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_INSTALLMENT_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_INSTALLMENT_REFUND_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REDEEM_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REDEEM_REFUND_, 2))
	{
		/* Trans Date */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		strcat(szTemplate, pobTran->srBRec.szDate);
		memcpy(&szDataBuffer[inPacketSizes], &szTemplate[2], 6);
		inPacketSizes += 6;

		/* Trans Time */
		memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szTime, 6);
		inPacketSizes += 6;
	}
	else
	{
		inPacketSizes += 6;
		inPacketSizes += 6;
	}

	/* Approval No (9 Byte) */
	if (strlen(pobTran->srBRec.szAuthCode) > 0)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		strcpy(szTemplate, pobTran->srBRec.szAuthCode);
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 9, _PADDING_RIGHT_);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 9);
	}
	inPacketSizes += 9;
	
	/* 保留欄位/實際支付金額(紅利交易)/分期期數 + 分期手續費(分期交易) (12 Byte) */
	if (pobTran->srBRec.uszRedeemBit == VS_TRUE && pobTran->srBRec.uszVOIDBit != VS_TRUE)
	{
		/* 實際支付金額 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010ld00", (pobTran->srBRec.lnRedemptionPaidCreditAmount + pobTran->srBRec.lnTipTxnAmount));
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
		
		inPacketSizes += 12;
	}
	else if (pobTran->srBRec.uszInstallmentBit == VS_TRUE && pobTran->srBRec.uszVOIDBit != VS_TRUE)
	{
		/* 分期期數 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%02ld", pobTran->srBRec.lnInstallmentPeriod);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 2);
		
		inPacketSizes += 2;
		
		/* 分期手續費 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010ld", pobTran->srBRec.lnInstallmentFormalityFee);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 10);
		
		inPacketSizes += 10;
	}
	else
	{
		/* Wave Card Indicator */
		memcpy(&szDataBuffer[inPacketSizes], "0", 1);
		inPacketSizes ++;
		
		/* CUP Indicator */
		if (!memcmp(&srECROb->srTransData.szField_05[0], "C", 1))
			memcpy(&szDataBuffer[inPacketSizes], &srECROb->srTransData.szField_05[0], 1);
		else
			memcpy(&szDataBuffer[inPacketSizes], "0", 1);

		inPacketSizes ++; /* CUP Indicator */
		/* Reserve (10 Byte) */
		memcpy(&szDataBuffer[inPacketSizes], "0000000000", 10);
		inPacketSizes += 10;
	}
	
	/* ECR Response Code (4 Byte) */
	memcpy(&szDataBuffer[inPacketSizes], "0000", 4);
	inPacketSizes += 4;
	
	/* Terminal ID (8 Byte) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTerminalID(szTemplate);
	inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 8, _PADDING_RIGHT_);
	memcpy(&szDataBuffer[inPacketSizes], szTemplate, 8);
	inPacketSizes += 8;

	/* Reference No */
	if (strlen(pobTran->srBRec.szRefNo) > 0)
	{
		memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szRefNo, 12);
	}
	inPacketSizes += 12;
	
	/* 保留欄位/扣抵紅利點數(紅利交易)/首期金額(分期交易) (12 Byte) */
	if (pobTran->srBRec.uszRedeemBit == VS_TRUE)
	{
		/* 扣抵紅利點數 (Points of Redemption 12 ByteS) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%012ld", pobTran->srBRec.lnRedemptionPoints);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
	}
	else if (pobTran->srBRec.uszInstallmentBit == VS_TRUE)
	{
		/* 首期金額 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010ld00", pobTran->srBRec.lnInstallmentDownPayment);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
	}
	else
	{
		memcpy(&szDataBuffer[inPacketSizes], "000000000000", 12);
	}
	inPacketSizes += 12;
	
	/* HGPAN(Request:Store Id Response:HappyGo Card Number) (18 Byte) */
	/* Store Id 要印 Merchant ID(18 Byte) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetMerchantID(szTemplate);
	inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 18, _PADDING_RIGHT_);
	memcpy(&szDataBuffer[inPacketSizes], szTemplate, 18);
	inPacketSizes += 18;
	
	/* 保留欄位(10 Byte)+ START Trans Type (2 Byte) /剩餘紅利點數(紅利交易)/每期金額(分期交易) (12 Byte) */
	if (pobTran->srBRec.uszRedeemBit == VS_TRUE)
	{
		/* 剩餘紅利點數(紅利交易) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%012ld", pobTran->srBRec.lnRedemptionPointsBalance);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
	}
	else if (pobTran->srBRec.uszInstallmentBit == VS_TRUE)
	{
		/* 每期金額(分期交易) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010ld00", pobTran->srBRec.lnInstallmentPayment);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
	}
	else
	{
		memcpy(&szDataBuffer[inPacketSizes], "000000000000", 12);
	}
	inPacketSizes += 12;
	
	return (VS_SUCCESS);
}

/*
Function        :inECR_7E1_Customer_026_TAKA_Pack_Error
Date&Time       :2022/10/12 下午 12:01
Describe        :
*/
int inECR_7E1_Customer_026_TAKA_Pack_Error(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, ECR_TABLE * srECROb)
{
	int		inRetVal;
	int		i, inCardLen;
	int		inPacketSizes = 0;
	char		szTemplate[100 + 1];
	unsigned char	uszHostResponseBit = VS_FALSE;
	
	/* 有Response Code 代表主機有回 */
	if (strlen(pobTran->srBRec.szRespCode) > 0	||
	    strlen(pobTran->srTRec.szRespCode) > 0)
	{
		uszHostResponseBit = VS_TRUE;
	}

	if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
	{
		strcpy(srECROb->srTransData.szField_05, "C");
	}
	
	/* Trans Type (2 Byte) */
	memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szTransType, 2);
	inPacketSizes += 2;

	/* HOST ID (2 Byte) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTRTFileName(szTemplate);

	if (!memcmp(szTemplate, _TRT_FILE_NAME_CREDIT_, strlen(_TRT_FILE_NAME_CREDIT_)))
		memcpy(&szDataBuffer[inPacketSizes], _ECR_7E1_NCCC_HOSTID_NCCC_, 2);
	/* 2018/5/8 下午 2:10 DFS需求不再使用大來主機 */
	else if (!memcmp(szTemplate, _TRT_FILE_NAME_DCC_, strlen(_TRT_FILE_NAME_DCC_)))
		memcpy(&szDataBuffer[inPacketSizes], _ECR_7E1_NCCC_HOSTID_CUS_026_DCC_, 2);
	else if (!memcmp(szTemplate, _TRT_FILE_NAME_DCC_, strlen(_TRT_FILE_NAME_DCC_)))
		memcpy(&szDataBuffer[inPacketSizes], _ECR_7E1_NCCC_HOSTID_CUS_026_TAKA_, 2);

	inPacketSizes += 2;

	/* Receipt No (6 Byte) */
	if (pobTran->srBRec.lnOrgInvNum != 0L)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%06ld", pobTran->srBRec.lnOrgInvNum);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 6);
	}
	inPacketSizes += 6;

	/* Card No (19 Byte)(左靠右補空白) */
	/* 卡號是否遮掩 */
	inRetVal = inECR_CardNoTruncateDecision(pobTran);

	/* 要遮卡號 */
	if (inRetVal == VS_SUCCESS)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		strcpy(szTemplate, pobTran->srBRec.szPAN);

		/* HAPPG_GO 卡不掩飾 */
		/* 大高社員卡卡號不遮掩 */
		if (!memcmp(&pobTran->srBRec.szPAN[0], "9552", 4) ||
		    pobTran->srBRec.uszTakaTransBit == VS_TRUE)
		{

		}
		else
		{
			/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
			/* 卡號長度 */
			inCardLen = strlen(szTemplate);

			/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
			for (i = 6; i < (inCardLen - 4); i ++)
				szTemplate[i] = '*';
		}

		memcpy(&szDataBuffer[inPacketSizes], szTemplate, strlen(szTemplate));
	}
	else
	{
		memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szPAN, strlen(pobTran->srBRec.szPAN));
	}

	inPacketSizes += 19;

	/* Card Expire Date (4 Byte) */
	/* 有效期是否回傳 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetECRExpDateReturnEnable(szTemplate);

	/* 要回傳有效期 */
	if (!memcmp(&szTemplate, "Y", 1))
		memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szExpDate, strlen(pobTran->srBRec.szExpDate));

	inPacketSizes += 4;

	/* Trans Amount (12 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_SALE_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REFUND_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_OFFLINE_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_INSTALLMENT_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_INSTALLMENT_REFUND_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REDEEM_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REDEEM_REFUND_, 2))
	{
		if (pobTran->srBRec.lnTxnAmount != 0)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%010ld00", pobTran->srBRec.lnTxnAmount);
			memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
		}
	}
	else
	{
		/* 要補0 */
		memcpy(&szDataBuffer[inPacketSizes], "000000000000", 12);
	}
	inPacketSizes += 12;

	/* Trans Date (6 Byte) & Trans Time (6 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_SALE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REFUND_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_VOID_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_OFFLINE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_INSTALLMENT_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_INSTALLMENT_REFUND_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REDEEM_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REDEEM_REFUND_, 2))
	{
		/* Trans Date */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		strcat(szTemplate, pobTran->srBRec.szDate);
		memcpy(&szDataBuffer[inPacketSizes], &szTemplate[2], 6);
		inPacketSizes += 6;

		/* Trans Time */
		memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szTime, 6);
		inPacketSizes += 6;
	}
	else
	{
		inPacketSizes += 6;
		inPacketSizes += 6;
	}

	/* Approval No (9 Byte) */
	if (uszHostResponseBit == VS_TRUE)
	{
		if (strlen(pobTran->srBRec.szAuthCode) > 0)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			strcpy(szTemplate, pobTran->srBRec.szAuthCode);
			inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 9, _PADDING_RIGHT_);
			memcpy(&szDataBuffer[inPacketSizes], szTemplate, 9);
		}
	}
	else
	{
		
	}

	inPacketSizes += 9;
	/* 保留欄位/實際支付金額(紅利交易)/分期期數 + 分期手續費(分期交易) (12 Byte) */
	if (pobTran->srBRec.uszRedeemBit == VS_TRUE && pobTran->srBRec.uszVOIDBit != VS_TRUE)
	{
		/* 實際支付金額 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010ld00", (pobTran->srBRec.lnRedemptionPaidCreditAmount + pobTran->srBRec.lnTipTxnAmount));
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
		
		inPacketSizes += 12;
	}
	else if (pobTran->srBRec.uszInstallmentBit == VS_TRUE && pobTran->srBRec.uszVOIDBit != VS_TRUE)
	{
		/* 分期期數 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%02ld", pobTran->srBRec.lnInstallmentPeriod);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 2);
		
		inPacketSizes += 2;
		
		/* 分期手續費 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010ld", pobTran->srBRec.lnInstallmentFormalityFee);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 10);
		
		inPacketSizes += 10;
	}
	else
	{
		/* Wave Card Indicator */
		memcpy(&szDataBuffer[inPacketSizes], "0", 1);
		inPacketSizes ++;
		
		/* CUP Indicator */
		if (!memcmp(&srECROb->srTransData.szField_05[0], "C", 1))
			memcpy(&szDataBuffer[inPacketSizes], &srECROb->srTransData.szField_05[0], 1);
		else
			memcpy(&szDataBuffer[inPacketSizes], "0", 1);

		inPacketSizes ++; /* CUP Indicator */
		/* Reserve (10 Byte) */
		inPacketSizes += 10;
	}
	
	/* ECR Response Code (4 Byte) */
	memcpy(&szDataBuffer[inPacketSizes], "0000", 4);
	inPacketSizes += 4;
	
	/* Terminal ID (8 Byte) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTerminalID(szTemplate);
	inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 8, _PADDING_RIGHT_);
	memcpy(&szDataBuffer[inPacketSizes], szTemplate, 8);
	inPacketSizes += 8;

	/* Reference No */
	if (strlen(pobTran->srBRec.szRefNo) > 0)
	{
		memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szRefNo, 12);
	}
	inPacketSizes += 12;
	
	/* 保留欄位/扣抵紅利點數(紅利交易)/首期金額(分期交易) (12 Byte) */
	if (pobTran->srBRec.uszRedeemBit == VS_TRUE)
	{
		/* 扣抵紅利點數 (Points of Redemption 12 ByteS) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%012ld", pobTran->srBRec.lnRedemptionPoints);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
	}
	else if (pobTran->srBRec.uszInstallmentBit == VS_TRUE)
	{
		/* 首期金額 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010ld00", pobTran->srBRec.lnInstallmentDownPayment);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
	}
	else
	{
		memcpy(&szDataBuffer[inPacketSizes], "000000000000", 12);
	}
	inPacketSizes += 12;
	
	/* HGPAN(Request:Store Id Response:HappyGo Card Number) (18 Byte) */
	/* Store Id 要印 Merchant ID(18 Byte) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetMerchantID(szTemplate);
	inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 18, _PADDING_RIGHT_);
	memcpy(&szDataBuffer[inPacketSizes], szTemplate, 18);
	inPacketSizes += 18;
	
	/* 保留欄位(10 Byte)+ START Trans Type (2 Byte) /剩餘紅利點數(紅利交易)/每期金額(分期交易) (12 Byte) */
	if (pobTran->srBRec.uszRedeemBit == VS_TRUE)
	{
		/* 剩餘紅利點數(紅利交易) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%012ld", pobTran->srBRec.lnRedemptionPointsBalance);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
	}
	else if (pobTran->srBRec.uszInstallmentBit == VS_TRUE)
	{
		/* 每期金額(分期交易) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010ld00", pobTran->srBRec.lnInstallmentPayment);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
	}
	else
	{
		memcpy(&szDataBuffer[inPacketSizes], "000000000000", 12);
	}
	inPacketSizes += 12;
	
	return (VS_SUCCESS);
}

/*
Function        :inECR_7E1_Customer_026_TAKA_Pack_ResponseCode
Date&Time       :2022/10/12 下午 2:08
Describe        :
*/
int inECR_7E1_Customer_026_TAKA_Pack_ResponseCode(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuf)
{
	char	szTmeplate[12 + 1];
	
	memset(szTmeplate, 0x00, sizeof(szTmeplate));
	inGetTRTFileName(szTmeplate);
	
	if (srECROb->srTransData.inErrorType == VS_CALLBANK	||
	    srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_CALLBANK_)
		memcpy(&szDataBuf[78], "0002", 4);
	else if (srECROb->srTransData.inErrorType == VS_TIMEOUT)
		memcpy(&szDataBuf[78], "0003", 4);
	else
	{
		memcpy(&szDataBuf[78], "0001", 4);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inECR_7E1_Customer_021_TAKAWEL_Unpack
Date&Time       :2022/10/18 下午 5:46
Describe        :
*/
int inECR_7E1_Customer_021_TAKAWEL_Unpack(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_SUCCESS;
	int	inTransType = 0;		/* 電文中的交易別字串轉為數字儲存 */
	int	inCardInquiryTRTType = _CARD_INQUIRY_TYPE_NONE_;
	char	szTemplate[100 + 1] = {0};
	char	szDebugMsg[100 + 1] = {0};	
	char	szCTLSEnable[2 + 1] = {0};
	char	szFiscContactlessEnable[1 + 1] = {0};
	char	szCUPContactlessEnable[1 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
	
	/* (需求單 - 107276)自助交易標準做法 EDC於發動交易及完成時，需發出提示音 by Russell 2019/3/8 上午 10:46 */
	inDISP_BEEP(1, 0);
	
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
        inGetCustomIndicator(szCustomerIndicator);
	
	/* 存ECR原始資料 */
	memcpy(srECROb->srTransData.szOrgData, szDataBuffer, _ECR_7E1_Standard_Data_Size_);
	
	inECR_7E1_Standard_Parse_Data(srECROb, szDataBuffer);
	
	/* 交易類別轉成數字 */
	inTransType = atoi(srECROb->srTransData.szTransType);
	
	/* Trans Type (交易別) */
	switch (inTransType)
	{
		case _ECR_7E1_SALE_NO_:
		case _ECR_7E1_REFUND_NO_:
		case _ECR_7E1_REDEEM_NO_:
		case _ECR_7E1_REDEEM_REFUND_NO_:
		case _ECR_7E1_SETTLEMENT_NO_:
		case _ECR_7E1_START_CARD_NO_INQUIRY_NO_:
		case _ECR_7E1_END_CARD_NO_INQUIRY_NO_:
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szTransType[0], 2);
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s: %s  ", "Trans Type", szTemplate);
				inLogPrintf(AT, szDebugMsg);
			}
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Type", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "CTLS not Init");
				return (VS_ERROR);
			}
			else
			{
				memcpy(srECROb->srTransData.szTransType, szTemplate, 2);
			}
			
			/* 交易類別轉成數字 */
			inTransType = atoi(srECROb->srTransData.szTransType);
			
			/* 要連動結帳 */
			if (inTransType == _ECR_7E1_SETTLEMENT_NO_)
			{
				pobTran->uszAutoSettleBit = VS_TRUE;
			}
			break;
		default:
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "不支援交易別%s", srECROb->srTransData.szTransType);
				inLogPrintf(AT, szDebugMsg);
			}
			memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
			sprintf(pobTran->szErrorMsgBuff1, "不支援交易別%s", srECROb->srTransData.szTransType);
			memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
			pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

			return (VS_ERROR);
			break;
	}
		
	/* 處理交易金額 (Trans Amount) */
	switch (inTransType)
	{
		case _ECR_7E1_SALE_NO_:				/* 一般交易 */
		case _ECR_7E1_REFUND_NO_:			/* 退貨交易 */
		case _ECR_7E1_REDEEM_NO_:			/* 紅利扣抵 */
		case _ECR_7E1_REDEEM_REFUND_NO_:		/* 紅利退貨 */
		case _ECR_7E1_START_CARD_NO_INQUIRY_NO_:	/* 啟動卡號查詢 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &srECROb->srTransData.szField_10[0], 10);
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Amount", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "Trans Amount Not Exist Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
				return (VS_ERROR);
			}
			else if ((!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_076_8_DIGITS_, _CUSTOMER_INDICATOR_SIZE_) && atol(szTemplate) > 99999999) || 
				 (memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_076_8_DIGITS_, _CUSTOMER_INDICATOR_SIZE_) && atol(szTemplate) > 9999999))
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Amount", "Out of bound");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_076_8_DIGITS_, _CUSTOMER_INDICATOR_SIZE_))
				{
					sprintf(pobTran->szErrorMsgBuff2, "金額超過8位數");
				}
				else
				{
					sprintf(pobTran->szErrorMsgBuff2, "金額超過7位數");
				}
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
				
				return (VS_ERROR);
			}
			else
			{
				pobTran->srBRec.lnTxnAmount = atol(szTemplate);
				pobTran->srBRec.lnOrgTxnAmount = atol(szTemplate);
				pobTran->srBRec.lnTotalTxnAmount = atol(szTemplate);
			}

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%ld", "交易金額", pobTran->srBRec.lnTxnAmount);
				inLogPrintf(AT, szDebugMsg);
			}
			break;
		default:
			break;
	}
	
	/* 授權碼(Auth Code) ECR電文中給9個字元(左靠右補空白) 但ATS電文只需要6個，所以只抓6個(交易補登及所有退貨須送此欄位) */
	switch (inTransType)
	{
		case _ECR_7E1_REFUND_NO_:			/* 退貨 */
		case _ECR_7E1_REDEEM_REFUND_NO_:		/* 紅利退貨 */
			/* 處理授權碼(optional) */
			memset(pobTran->srBRec.szAuthCode, 0x00, sizeof(pobTran->srBRec.szAuthCode));
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szApprovalNo[0], 6);
			memcpy(pobTran->srBRec.szAuthCode, szTemplate, 6);
			
			break;
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Auth Code", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "Auth Code Not Exist Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
				return (VS_ERROR);
			}
			else
			{
				memcpy(pobTran->srBRec.szAuthCode, szTemplate, 6);
			}
			
			break;
			
		default :
			break;
	}
	
	/* szDataBuffer[66] 144規格要看紅利分期來看使用用途  */
	switch (inTransType)
	{
		case _ECR_7E1_REDEEM_REFUND_NO_:		/* 紅利退貨 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szRDMPaidAmt[0], 10);
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "RDM Paid Amt", szTemplate);
				inLogPrintf(AT, szDebugMsg);
			}
			/* 實際支付金額 */
			pobTran->srBRec.lnRedemptionPaidCreditAmount = atol(szTemplate);
			
			break;
		default :
			break;
	}
	
	/* szDataBuffer[102] 144規格要看紅利分期來看使用用途 保留欄位/首期金額／紅利扣抵點數  */
	switch (inTransType)
	{
		case _ECR_7E1_REDEEM_REFUND_NO_:		/* 紅利退貨 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szRDMPoint[0], 12);
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "RDM Point", szTemplate);
				inLogPrintf(AT, szDebugMsg);
			}
			
			/* 信用卡紅利扣抵點數 RDM Point */
			pobTran->srBRec.lnRedemptionPoints = atol(szTemplate);
			break;
		default :
			break;
	}
	
	/* 處理櫃號(Store ID) */
	switch (inTransType)
	{
		default:
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szStoreId[0], 18);
			
			if (strlen(szTemplate) > 0)
			{
				memcpy(pobTran->srBRec.szStoreID, szTemplate, strlen(szTemplate));
				/* 櫃號不滿18，補空白 */
				if (strlen(szTemplate) < 18)
				{
					memset(&pobTran->srBRec.szStoreID[strlen(szTemplate)], 0x20, 18 - strlen(szTemplate));
				}

			}
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "Store ID", pobTran->srBRec.szStoreID);
				inLogPrintf(AT, szDebugMsg);
			}
			break;
	}
	
	/* 若前一次為卡號查詢，標記為第二次ECR */
	if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
	{
		pobTran->uszCardInquiryFirstBit = VS_FALSE;
		pobTran->uszCardInquirysSecondBit = VS_TRUE;
		
		/* 重新決定TRT流程 */
                if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
                {
                        /* 金融卡查詢要二次過卡，不依循前次查詢界面 */
                        inCardInquiryTRTType = _CARD_INQUIRY_TYPE_NONE_;
                }
                else
                {
                        if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
                        {
                                inCardInquiryTRTType = _CARD_INQUIRY_TYPE_CHIP_;
                        }
                        else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
                        {
                                inCardInquiryTRTType = _CARD_INQUIRY_TYPE_CTLS_;
                        }
                        else
                        {
                                inCardInquiryTRTType = _CARD_INQUIRY_TYPE_MEG_;
                        }
                }
	}
	
	/* szDataBuffer[142] 144規格要看紅利分期來看使用用途  卡號查詢/每期金額/保留欄位 */
	switch (inTransType)
	{
		/* 啟動卡號查詢 Start Trans Type */
		case _ECR_7E1_START_CARD_NO_INQUIRY_NO_:		/* 卡號查詢 */
			/* 卡號查詢 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &srECROb->srTransData.szStartTransType[0], 2);
			inTransType = atoi(szTemplate);
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "Start Trans Type", szTemplate);
				inLogPrintf(AT, szDebugMsg);
			}
			
			switch (inTransType)
			{
				case _ECR_7E1_SALE_NO_ :
				case _ECR_7E1_REFUND_NO_ :
					if (pobTran->uszCardInquirysSecondBit == VS_TRUE)
					{

					}
					else
					{
						pobTran->uszCardInquiryFirstBit = VS_TRUE;
					}
					/* 有值影響到後面判斷  確保都是空格 */
					memcpy(&srECROb->srTransData.szField_05[0], " ", 1);
					memcpy(&srECROb->srTransData.szTransTypeIndicator[0], " ", 1);
					break;
				case _ECR_7E1_REDEEM_NO_ :
				case _ECR_7E1_REDEEM_REFUND_NO_ :
					if (pobTran->uszCardInquirysSecondBit == VS_TRUE)
					{

					}
					else
					{
						pobTran->uszCardInquiryFirstBit = VS_TRUE;
					}
					memcpy(&srECROb->srTransData.szField_05[0], " ", 1);
					memcpy(&srECROb->srTransData.szTransTypeIndicator[0], " ", 1);
					break;
				default :
					pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
					return (VS_ERROR);
			}
			break;
		default:
			break;
	}
	
	if (inECR_Check_Exception(pobTran) != VS_SUCCESS)
	{
		memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
		sprintf(pobTran->szErrorMsgBuff2, "ECR_Check_Exception error");
		return (VS_ERROR);
	}
	
	memset(szFiscContactlessEnable, 0x00, sizeof(szFiscContactlessEnable));
	inGetSmartPayContactlessEnable(szFiscContactlessEnable);
	memset(szCTLSEnable, 0x00, sizeof(szCTLSEnable));
	inGetContactlessEnable(szCTLSEnable);
	memset(szCUPContactlessEnable, 0x00, sizeof(szCUPContactlessEnable));
	inGetCUPContactlessEnable(szCUPContactlessEnable);
	
	/* 跑OPT */
	switch (inTransType)
	{
		case _ECR_7E1_SALE_NO_:
			/* 金融卡和一般Sale跑不同OPT，所以要分 */
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{	
				inDISP_ClearAll();
				/* 第三層顯示 ＜消費扣款＞ */
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_FISC_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜消費扣款＞ */

				pobTran->inFunctionID = _FISC_SALE_;
				if (!memcmp(szCTLSEnable, "Y", 1)		&&
				    !memcmp(szFiscContactlessEnable, "Y", 1)	&&
				    guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_FISC_SALE_CTLS_;
					pobTran->inRunTRTID = _TRT_FISC_SALE_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_FISC_SALE_;
					pobTran->inRunTRTID = _TRT_FISC_SALE_ICC_;
				}
		
				pobTran->inTransactionCode = _FISC_SALE_;
				pobTran->srBRec.inCode = _FISC_SALE_;
				pobTran->srBRec.inOrgCode = _FISC_SALE_;
                                
                                /* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_CUP_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜銀聯一般交易＞ */

				if (!memcmp(szCTLSEnable, "Y", 1)		&&
				    !memcmp(szCUPContactlessEnable, "Y", 1)	&&
				    guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
					pobTran->inRunTRTID = _TRT_CUP_SALE_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
					pobTran->inRunTRTID = _TRT_CUP_SALE_;
				}

				pobTran->inFunctionID = _CUP_SALE_;
				pobTran->inTransactionCode = _CUP_SALE_;
				pobTran->srBRec.inCode = pobTran->inTransactionCode;
				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
                                
                                /* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_TICKET_DEDUCT_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜購貨交易＞ */

				pobTran->inFunctionID = _TICKET_DEDUCT_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_TICKET_DEDUCT_;
				}
				else
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "感應未初始化，不能使用此功能");
					}
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "CTLS not Init");
					return (VS_ERROR);
				}

				pobTran->inTransactionCode = _TICKET_DEDUCT_;
				pobTran->srTRec.inCode = _TICKET_DEDUCT_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else
			{
				inDISP_ClearAll();
				/* 第三層顯示 ＜一般交易＞ */
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般交易＞ */

				pobTran->inFunctionID = _SALE_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
					pobTran->inRunTRTID = _TRT_SALE_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
					pobTran->inRunTRTID = _TRT_SALE_;
				}

				pobTran->inTransactionCode = _SALE_;
				pobTran->srBRec.inCode = _SALE_;
				pobTran->srBRec.inOrgCode = _SALE_;
                                
                                /* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			
			break;
		case _ECR_7E1_REFUND_NO_:
			/* 金融卡和一般Sale跑不同OPT，所以要分 */
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_FISC_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);	/* 第二層顯示 ＜退費交易＞ */

				pobTran->inFunctionID = _FISC_REFUND_;                       
				if (!memcmp(szCTLSEnable, "Y", 1)		&&
				    !memcmp(szFiscContactlessEnable, "Y", 1)	&&
				    guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_FISC_REFUND_CTLS_;
					pobTran->inRunTRTID = _TRT_FISC_REFUND_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_FISC_REFUND_;
					pobTran->inRunTRTID = _TRT_FISC_REFUND_ICC_;
				}

				pobTran->inTransactionCode = _FISC_REFUND_;
				pobTran->srBRec.inCode = _FISC_REFUND_;
				pobTran->srBRec.inOrgCode = _FISC_REFUND_;
                                
                                /* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			else if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般退貨＞ */

				if (!memcmp(szCTLSEnable, "Y", 1)		&&
				    !memcmp(szCUPContactlessEnable, "Y", 1)	&&
				    guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_GET_CARD_FIRST_;
					pobTran->inRunTRTID = _TRT_CUP_REFUND_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_AMOUNT_FIRST_CUP_;
					pobTran->inRunTRTID = _TRT_CUP_REFUND_;
				}

				pobTran->inFunctionID = _CUP_REFUND_;
				pobTran->inTransactionCode = _CUP_REFUND_;
				pobTran->srBRec.inCode = pobTran->inTransactionCode;
				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
                                
                                /* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_TICKET_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜退貨交易＞ */

				pobTran->inFunctionID = _TICKET_REFUND_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_TICKET_REFUND_;
				}
				else
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "感應未初始化，不能使用此功能");
					}
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "CTLS not Init");
					return (VS_ERROR);
				}

				pobTran->inTransactionCode = _TICKET_REFUND_;
				pobTran->srTRec.inCode = _TICKET_REFUND_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般退貨＞ */

				pobTran->inFunctionID = _REFUND_;

				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_GET_CARD_FIRST_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_AMOUNT_FIRST_;
				}
				pobTran->inRunTRTID = _TRT_REFUND_;

				pobTran->inTransactionCode = _REFUND_;
				pobTran->srBRec.inCode = _REFUND_;
				pobTran->srBRec.inOrgCode = _REFUND_;
                                
                                /* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;
		case _ECR_7E1_REDEEM_NO_:
//			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
//			{
//				inDISP_ClearAll();
//				/* 保留給銀聯紅利扣抵使用 */
//				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
//				inDISP_PutGraphic(_MENU_REDEEM_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利扣抵＞ */
//
//				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
//				{
//					pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
//				}
//				else
//				{
//					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
//				}
//				pobTran->inRunTRTID = _TRT_REDEEM_SALE_;
//
//				pobTran->inFunctionID = _CUP_REDEEM_SALE_;
//				pobTran->inTransactionCode = _CUP_REDEEM_SALE_;
//				pobTran->srBRec.inCode = pobTran->inTransactionCode;
//				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
//
//				pobTran->srBRec.uszRedeemBit = VS_TRUE;
//                              /* 卡號查詢不過第二次卡，所以要直接決定TRT */
//				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
//				{
//					inNCCC_Func_Decide_CTLS_TRT(pobTran);
//				}
//				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
//				{
//					inNCCC_Func_Decide_ICC_TRT(pobTran);
//				}
//				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
//				{
//					inNCCC_Func_Decide_MEG_TRT(pobTran);
//				}
//
//				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
//			}
//			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REDEEM_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利扣抵＞ */

				pobTran->inFunctionID = _REDEEM_SALE_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
				}
				pobTran->inRunTRTID = _TRT_REDEEM_SALE_;

				pobTran->inTransactionCode = _REDEEM_SALE_;
				pobTran->srBRec.inCode = _REDEEM_SALE_;
				pobTran->srBRec.inOrgCode = _REDEEM_SALE_;

				pobTran->srBRec.uszRedeemBit = VS_TRUE;
                                /* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;
		case _ECR_7E1_REDEEM_REFUND_NO_:
//			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
//			{
//				inDISP_ClearAll();
//				/* 保留給銀聯紅利退貨使用 */
//				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
//				inDISP_PutGraphic(_MENU_REDEEM_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利退貨＞ */
//
//				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
//				{
//					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_GET_CARD_FIRST_;
//				}
//				else
//				{
//					pobTran->inRunOperationID = _OPERATION_REFUND_;
//				}
//				pobTran->inRunTRTID = _TRT_REDEEM_REFUND_;
//
//				pobTran->inFunctionID = _CUP_REDEEM_REFUND_;
//				pobTran->inTransactionCode = _CUP_REDEEM_REFUND_;
//				pobTran->srBRec.inCode = pobTran->inTransactionCode;
//				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
//
//				pobTran->srBRec.uszRedeemBit = VS_TRUE;
//                              /* 卡號查詢不過第二次卡，所以要直接決定TRT */
//				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
//				{
//					inNCCC_Func_Decide_CTLS_TRT(pobTran);
//				}
//				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
//				{
//					inNCCC_Func_Decide_ICC_TRT(pobTran);
//				}
//				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
//				{
//					inNCCC_Func_Decide_MEG_TRT(pobTran);
//				}
//
//				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
//			}
//			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REDEEM_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利退貨＞ */

				pobTran->inFunctionID = _REDEEM_REFUND_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_GET_CARD_FIRST_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_;
				}
				pobTran->inRunTRTID = _TRT_REDEEM_REFUND_;

				pobTran->inTransactionCode = _REDEEM_REFUND_;
				pobTran->srBRec.inCode = _REDEEM_REFUND_;
				pobTran->srBRec.inOrgCode = _REDEEM_REFUND_;

				pobTran->srBRec.uszRedeemBit = VS_TRUE;
                                /* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;
		case _ECR_7E1_SETTLEMENT_NO_:
			inDISP_ClearAll();
			/* 第三層顯示 ＜結帳交易＞ */
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_SETTLE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜結帳交易＞ */

			pobTran->inFunctionID = _SETTLE_;
			pobTran->inRunOperationID = _OPERATION_SETTLE_;
			pobTran->inRunTRTID = _TRT_SETTLE_;
			
			pobTran->inTransactionCode = _SETTLE_;
			pobTran->srBRec.inCode = _SETTLE_;
			pobTran->srBRec.inOrgCode = _SETTLE_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		case _ECR_7E1_END_CARD_NO_INQUIRY_NO_:
			pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_END_INQUIRY_SUCCESS_;
			inRetVal = VS_ERROR;
			break;
		default:
			pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
			inRetVal = VS_ERROR;
			break;
	}
	
	inECR_ReCheck_144_TRT_Flow(pobTran);
	
	return (inRetVal);
}

/*
Function        :inECR_7E1_Customer_021_TAKAWEL_Pack
Date&Time       :2022/10/18 下午 6:40
Describe        :先把要送的資料組好
*/
int inECR_7E1_Customer_021_TAKAWEL_Pack(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, ECR_TABLE * srECROb)
{
	int	inRetVal;
	int 	i, inCardLen;
	int	inPacketSizes = 0;
	char 	szTemplate[100 + 1];
	
	/* Trans Type (2 Byte) */
	memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szTransType, 2);
	inPacketSizes += 2;

	/* HOST ID (2 Byte) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTRTFileName(szTemplate);

	if (!memcmp(szTemplate, _TRT_FILE_NAME_CREDIT_, strlen(_TRT_FILE_NAME_CREDIT_)))
		memcpy(&szDataBuffer[inPacketSizes], _ECR_7E1_NCCC_HOSTID_NCCC_, 2);
	/* 2018/5/8 下午 2:10 DFS需求不再使用大來主機 */
	else if (!memcmp(szTemplate, _TRT_FILE_NAME_DCC_, strlen(_TRT_FILE_NAME_DCC_)))
		memcpy(&szDataBuffer[inPacketSizes], _ECR_7E1_NCCC_HOSTID_CUS_026_DCC_, 2);
	else if (!memcmp(szTemplate, _TRT_FILE_NAME_TAKA_, strlen(_TRT_FILE_NAME_TAKA_)))
		memcpy(&szDataBuffer[inPacketSizes], _ECR_7E1_NCCC_HOSTID_CUS_026_TAKA_, 2);

	inPacketSizes += 2;

	/* Receipt No (6 Byte) */
	if (pobTran->srBRec.lnOrgInvNum != 0L)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%06ld", pobTran->srBRec.lnOrgInvNum);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 6);
	}
	inPacketSizes += 6;

	/* Card No (19 Byte)(左靠右補空白) */
	/* 卡號是否遮掩 */
	inRetVal = inECR_CardNoTruncateDecision(pobTran);

	/* 要遮卡號 */
	if (inRetVal == VS_SUCCESS)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		strcpy(szTemplate, pobTran->srBRec.szPAN);

		/* HAPPG_GO 卡不掩飾 */
		/* 大高社員卡卡號不遮掩 */
		if (!memcmp(&pobTran->srBRec.szPAN[0], "9552", 4) ||
		    pobTran->srBRec.uszTakaTransBit == VS_TRUE)
		{

		}
		else
		{
			/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
			/* 卡號長度 */
			inCardLen = strlen(szTemplate);

			/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
			for (i = 6; i < (inCardLen - 4); i ++)
				szTemplate[i] = '*';
		}

		memcpy(&szDataBuffer[inPacketSizes], szTemplate, strlen(szTemplate));
	}
	else
	{
		memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szPAN, strlen(pobTran->srBRec.szPAN));
	}

	inPacketSizes += 19;

	/* Card Expire Date (4 Byte) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetECRExpDateReturnEnable(szTemplate);

	/* 要回傳有效期 */
	if (!memcmp(&szTemplate, "Y", 1))
		memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szExpDate, strlen(pobTran->srBRec.szExpDate));

	inPacketSizes += 4;

	/* Trans Amount (12 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_SALE_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REFUND_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_VOID_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REDEEM_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REDEEM_REFUND_, 2))
	{
		if (pobTran->srBRec.lnTxnAmount != 0)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%010ld00", pobTran->srBRec.lnTxnAmount);
			memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
		}
	}
	else
	{
		/* 要補0 */
		memcpy(&szDataBuffer[inPacketSizes], "000000000000", 12);
	}
	inPacketSizes += 12;

	/* Trans Date (6 Byte) & Trans Time (6 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_SALE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REFUND_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_VOID_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REDEEM_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REDEEM_REFUND_, 2))
	{
		/* Trans Date */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		strcat(szTemplate, pobTran->srBRec.szDate);
		memcpy(&szDataBuffer[inPacketSizes], &szTemplate[2], 6);
		inPacketSizes += 6;

		/* Trans Time */
		memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szTime, 6);
		inPacketSizes += 6;
	}
	else
	{
		inPacketSizes += 6;
		inPacketSizes += 6;
	}

	/* Approval No (9 Byte) */
	if (strlen(pobTran->srBRec.szAuthCode) > 0)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		strcpy(szTemplate, pobTran->srBRec.szAuthCode);
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 9, _PADDING_RIGHT_);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 9);
	}
	inPacketSizes += 9;
	
	/* 保留欄位/實際支付金額(紅利交易)/分期期數 + 分期手續費(分期交易) (12 Byte) */
	if (pobTran->srBRec.uszRedeemBit == VS_TRUE && pobTran->srBRec.uszVOIDBit != VS_TRUE)
	{
		/* 實際支付金額 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010ld00", (pobTran->srBRec.lnRedemptionPaidCreditAmount + pobTran->srBRec.lnTipTxnAmount));
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
		
		inPacketSizes += 12;
	}
	else
	{
		inPacketSizes += 12;
	}
	
	/* ECR Response Code (4 Byte) */
	memcpy(&szDataBuffer[inPacketSizes], "0000", 4);
	inPacketSizes += 4;
	
	/* Terminal ID (8 Byte) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTerminalID(szTemplate);
	inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 8, _PADDING_RIGHT_);
	memcpy(&szDataBuffer[inPacketSizes], szTemplate, 8);
	inPacketSizes += 8;

	/* Reference No */
	if (strlen(pobTran->srBRec.szRefNo) > 0)
	{
		memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szRefNo, 12);
	}
	inPacketSizes += 12;
	
	/* 保留欄位/扣抵紅利點數(紅利交易)/首期金額(分期交易) (12 Byte) */
	if (pobTran->srBRec.uszRedeemBit == VS_TRUE)
	{
		/* 扣抵紅利點數 (Points of Redemption 12 ByteS) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%012ld", pobTran->srBRec.lnRedemptionPoints);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
	}
	else
	{
		memcpy(&szDataBuffer[inPacketSizes], "000000000000", 12);
	}
	inPacketSizes += 12;
	
	/* HGPAN(Request:Store Id Response:HappyGo Card Number) (18 Byte) */
	/* Store Id 要印 Merchant ID(18 Byte) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetMerchantID(szTemplate);
	inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 18, _PADDING_RIGHT_);
	memcpy(&szDataBuffer[inPacketSizes], szTemplate, 18);
	inPacketSizes += 18;
	
	/* 保留欄位(10 Byte)+ START Trans Type (2 Byte) /剩餘紅利點數(紅利交易)/每期金額(分期交易) (12 Byte) */
	if (pobTran->srBRec.uszRedeemBit == VS_TRUE)
	{
		/* 剩餘紅利點數(紅利交易) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%012ld", pobTran->srBRec.lnRedemptionPointsBalance);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
	}
	else
	{
		memcpy(&szDataBuffer[inPacketSizes], "000000000000", 12);
	}
	inPacketSizes += 12;
	
	return (VS_SUCCESS);
}

/*
Function        :inECR_7E1_Customer_021_TAKAWEL_Pack_Error
Date&Time       :2022/10/18 下午 6:40
Describe        :先把要送的資料組好
*/
int inECR_7E1_Customer_021_TAKAWEL_Pack_Error(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_SUCCESS;
	int 	i = 0, inCardLen = 0;
	int	inPacketSizes = 0;
	char 	szTemplate[100 + 1] = {0};
	unsigned char	uszHostResponseBit = VS_FALSE;
	
	/* 有Response Code 代表主機有回 */
	if (strlen(pobTran->srBRec.szRespCode) > 0	||
	    strlen(pobTran->srTRec.szRespCode) > 0)
	{
		uszHostResponseBit = VS_TRUE;
	}
	
	/* Trans Type (2 Byte) */
	memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szTransType, 2);
	inPacketSizes += 2;

	/* HOST ID (2 Byte) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTRTFileName(szTemplate);

	if (!memcmp(szTemplate, _TRT_FILE_NAME_CREDIT_, strlen(_TRT_FILE_NAME_CREDIT_)))
		memcpy(&szDataBuffer[inPacketSizes], _ECR_7E1_NCCC_HOSTID_NCCC_, 2);
	/* 2018/5/8 下午 2:10 DFS需求不再使用大來主機 */
	else if (!memcmp(szTemplate, _TRT_FILE_NAME_DCC_, strlen(_TRT_FILE_NAME_DCC_)))
		memcpy(&szDataBuffer[inPacketSizes], _ECR_7E1_NCCC_HOSTID_CUS_026_DCC_, 2);
	else if (!memcmp(szTemplate, _TRT_FILE_NAME_TAKA_, strlen(_TRT_FILE_NAME_TAKA_)))
		memcpy(&szDataBuffer[inPacketSizes], _ECR_7E1_NCCC_HOSTID_CUS_026_TAKA_, 2);

	inPacketSizes += 2;

	/* Receipt No (6 Byte) */
	if (pobTran->srBRec.lnOrgInvNum != 0L)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%06ld", pobTran->srBRec.lnOrgInvNum);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 6);
	}
	inPacketSizes += 6;

	/* Card No (19 Byte)(左靠右補空白) */
	/* 卡號是否遮掩 */
	inRetVal = inECR_CardNoTruncateDecision(pobTran);

	/* 要遮卡號 */
	if (inRetVal == VS_SUCCESS)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		strcpy(szTemplate, pobTran->srBRec.szPAN);

		/* HAPPG_GO 卡不掩飾 */
		/* 大高社員卡卡號不遮掩 */
		if (!memcmp(&pobTran->srBRec.szPAN[0], "9552", 4) ||
		    pobTran->srBRec.uszTakaTransBit == VS_TRUE)
		{

		}
		else
		{
			/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
			/* 卡號長度 */
			inCardLen = strlen(szTemplate);

			/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
			for (i = 6; i < (inCardLen - 4); i ++)
				szTemplate[i] = '*';
		}

		memcpy(&szDataBuffer[inPacketSizes], szTemplate, strlen(szTemplate));
	}
	else
	{
		memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szPAN, strlen(pobTran->srBRec.szPAN));
	}

	inPacketSizes += 19;

	/* Card Expire Date (4 Byte) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetECRExpDateReturnEnable(szTemplate);

	/* 要回傳有效期 */
	if (!memcmp(&szTemplate, "Y", 1))
		memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szExpDate, strlen(pobTran->srBRec.szExpDate));

	inPacketSizes += 4;

	/* Trans Amount (12 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_SALE_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REFUND_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_VOID_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REDEEM_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REDEEM_REFUND_, 2))
	{
		if (pobTran->srBRec.lnTxnAmount != 0)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%010ld00", pobTran->srBRec.lnTxnAmount);
			memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
		}
	}
	else
	{
		/* 要補0 */
		memcpy(&szDataBuffer[inPacketSizes], "000000000000", 12);
	}
	inPacketSizes += 12;

	/* Trans Date (6 Byte) & Trans Time (6 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_SALE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REFUND_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_VOID_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REDEEM_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REDEEM_REFUND_, 2))
	{
		/* Trans Date */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		strcat(szTemplate, pobTran->srBRec.szDate);
		memcpy(&szDataBuffer[inPacketSizes], &szTemplate[2], 6);
		inPacketSizes += 6;

		/* Trans Time */
		memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szTime, 6);
		inPacketSizes += 6;
	}
	else
	{
		inPacketSizes += 6;
		inPacketSizes += 6;
	}

	/* Approval No (9 Byte) */
	if (uszHostResponseBit == VS_TRUE)
	{
		if (strlen(pobTran->srBRec.szAuthCode) > 0)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			strcpy(szTemplate, pobTran->srBRec.szAuthCode);
			inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 9, _PADDING_RIGHT_);
			memcpy(&szDataBuffer[inPacketSizes], szTemplate, 9);
		}
	}
	else
	{
		
	}
	inPacketSizes += 9;
	
	/* 保留欄位/實際支付金額(紅利交易)/分期期數 + 分期手續費(分期交易) (12 Byte) */
	if (pobTran->srBRec.uszRedeemBit == VS_TRUE && pobTran->srBRec.uszVOIDBit != VS_TRUE)
	{
		/* 實際支付金額 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010ld00", (pobTran->srBRec.lnRedemptionPaidCreditAmount + pobTran->srBRec.lnTipTxnAmount));
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
		
		inPacketSizes += 12;
	}
	else
	{
		inPacketSizes += 12;
	}
	
	/* ECR Response Code (4 Byte) */
	memcpy(&szDataBuffer[inPacketSizes], "0000", 4);
	inPacketSizes += 4;
	
	/* Terminal ID (8 Byte) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTerminalID(szTemplate);
	inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 8, _PADDING_RIGHT_);
	memcpy(&szDataBuffer[inPacketSizes], szTemplate, 8);
	inPacketSizes += 8;

	/* Reference No */
	if (strlen(pobTran->srBRec.szRefNo) > 0)
	{
		memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szRefNo, 12);
	}
	inPacketSizes += 12;
	
	/* 保留欄位/扣抵紅利點數(紅利交易)/首期金額(分期交易) (12 Byte) */
	if (pobTran->srBRec.uszRedeemBit == VS_TRUE)
	{
		/* 扣抵紅利點數 (Points of Redemption 12 ByteS) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%012ld", pobTran->srBRec.lnRedemptionPoints);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
	}
	else
	{
		memcpy(&szDataBuffer[inPacketSizes], "000000000000", 12);
	}
	inPacketSizes += 12;
	
	/* HGPAN(Request:Store Id Response:HappyGo Card Number) (18 Byte) */
	/* Store Id 要印 Merchant ID(18 Byte) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetMerchantID(szTemplate);
	inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 18, _PADDING_RIGHT_);
	memcpy(&szDataBuffer[inPacketSizes], szTemplate, 18);
	inPacketSizes += 18;
	
	/* 保留欄位(10 Byte)+ START Trans Type (2 Byte) /剩餘紅利點數(紅利交易)/每期金額(分期交易) (12 Byte) */
	if (pobTran->srBRec.uszRedeemBit == VS_TRUE)
	{
		/* 剩餘紅利點數(紅利交易) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%012ld", pobTran->srBRec.lnRedemptionPointsBalance);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
	}
	else
	{
		memcpy(&szDataBuffer[inPacketSizes], "000000000000", 12);
	}
	inPacketSizes += 12;
	
	return (VS_SUCCESS);
}

/*
Function        :inECR_7E1_Customer_021_TAKAWEL_Pack_ResponseCode
Date&Time       :2022/10/18 下午 7:12
Describe        :
*/
int inECR_7E1_Customer_021_TAKAWEL_Pack_ResponseCode(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuf)
{
	if (srECROb->srTransData.inErrorType == VS_CALLBANK	||
	    srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_CALLBANK_)
	{
		memcpy(&szDataBuf[78], "0002", 4);
	}
	else if (srECROb->srTransData.inErrorType == VS_TIMEOUT)
	{
		memcpy(&szDataBuf[78], "0003", 4);
	}
	else if (srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_END_INQUIRY_SUCCESS_)
	{
		memcpy(&szDataBuf[78], "0001", 4);
	}
	else
	{
		memcpy(&szDataBuf[78], "0001", 4);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_8N1_Customer_090_TAIPEI_101_Pack
Date&Time       :2022/12/6 下午 6:42
Describe        :先把要送的資料組好
*/
int inECR_8N1_Customer_090_TAIPEI_101_Pack(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer)
{
	int	inRetVal = VS_ERROR;
	int	i = 0, inCardLen = 0;
	int	inPacketSizes = 0;
	int	inLen = 0;
	int	inBarCodeLen = 0;
	char	szTemplate[100 + 1];
	char	szHash[44 + 1];
	char	szFESMode[2 + 1];
	char	szTemp[8 + 1] = {0};
	char	szTemp2[8 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};

	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);

	/* ECR Indicator (1 Byte) */
	memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szECRIndicator, 1);
	inPacketSizes ++;
	/* ECR Version Date (6 Byte) */
	memcpy(&szDataBuffer[inPacketSizes], "190702", 6);
	inPacketSizes += 6;
	/* Trans Type Indicator (1 Byte) */
	inPacketSizes ++;
	/* Trans Type (2 Byte) */
	memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szTransType, 2);
	inPacketSizes += 2;
	/* CUP Indicator (1 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_START_CARD_NO_INQUIRY_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REFUND_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_CANCEL_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_CANCEL_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_TOP_UP_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_BALANCE_INQUIRY_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_VOID_TOP_UP_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_SALE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_CERTAIN_, 2))
	{
		if (!memcmp(srECROb->srTransData.szField_05, "C", 1) || pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			memcpy(&szDataBuffer[inPacketSizes], "C", 1);
		else if (!memcmp(srECROb->srTransData.szField_05, "S", 1) || pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			memcpy(&szDataBuffer[inPacketSizes], "S", 1);
		else if (!memcmp(srECROb->srTransData.szField_05, "E", 1) || pobTran->srTRec.uszESVCTransBit == VS_TRUE)
			memcpy(&szDataBuffer[inPacketSizes], "E", 1);
		else
			memcpy(&szDataBuffer[inPacketSizes], "N", 1);
	}

	inPacketSizes ++;

	/* HOST ID (2 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REFUND_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_OFFLINE_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TIP_, 2)				||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_CANCEL_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_CANCEL_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_REFUND_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_ADJUST_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_REFUND_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_ADJUST_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_TOP_UP_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_VOID_TOP_UP_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_INQUIRY_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_SALE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_INSTALLMENT_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REDEMPTION_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_CERTAIN_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_FULL_REDEEMPTION_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REDEEM_REFUND_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REFUND_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_AWARD_REDEEM_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_AWARD_REDEEM_, 2))
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTRTFileName(szTemplate);
		
		if (!memcmp(szTemplate, _TRT_FILE_NAME_CREDIT_, strlen(_TRT_FILE_NAME_CREDIT_)))
			memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_HOSTID_NCCC_, 2);
		/* 2018/5/8 下午 2:10 DFS需求不再使用大來主機 */
		else if (!memcmp(szTemplate, _TRT_FILE_NAME_HG_, strlen(_TRT_FILE_NAME_HG_)))
			memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_HOSTID_HG_, 2);
		else if (!memcmp(szTemplate, _TRT_FILE_NAME_DCC_, strlen(_TRT_FILE_NAME_DCC_)))
			memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_HOSTID_DCC_, 2);
		else if (!memcmp(szTemplate, _TRT_FILE_NAME_ESVC_, strlen(_TRT_FILE_NAME_ESVC_)))
			memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_HOSTID_ESVC_, 2);
	}

	inPacketSizes += 2;

	/* Receipt No (6 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REFUND_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_OFFLINE_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TIP_, 2)				||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_CANCEL_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_CANCEL_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_REFUND_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_ADJUST_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_REFUND_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_ADJUST_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_TOP_UP_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_VOID_TOP_UP_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_INQUIRY_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_SALE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_INSTALLMENT_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REDEMPTION_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_CERTAIN_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_FULL_REDEEMPTION_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REDEEM_REFUND_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REFUND_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_AWARD_REDEEM_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_AWARD_REDEEM_, 2))
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
		{
			sprintf(szTemplate, "%06ld", pobTran->srTRec.lnInvNum);
		}
		else
		{
			sprintf(szTemplate, "%06ld", pobTran->srBRec.lnOrgInvNum);
		}
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 6);
	}

	inPacketSizes += 6;

	/* Card No (19 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_END_CARD_NO_INQUIRY_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SETTLEMENT_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REPRINT_RECEIPT_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REVIEW_DETAIL_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REVIEW_TOTAL_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REPORT_DETAIL_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REPORT_TOTAL_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_EDC_REBOOT_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_INQUIRY_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_FULL_REDEEMPTION_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REDEEM_REFUND_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REFUND_, 2)		||
	    (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_AWARD_REDEEM_, 2) && !(!memcmp(pobTran->szL3_AwardWay, "4", 1) || !memcmp(pobTran->szL3_AwardWay, "5", 1)))	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_AWARD_REDEEM_, 2)		||
	    pobTran->srBRec.uszHappyGoSingle == VS_TRUE)
	{
		/* 優惠兌換先設定不回傳卡號 */
		inPacketSizes += 19;
	}
	else
	{
		if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
		{
			 memset(szTemplate, 0x00, sizeof(szTemplate));
                        
			/* 兩段式ECR電票，第一段取不到真正的卡號，填空白 */
			if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
			{
				if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
				{
					memset(szTemplate, 0x00, sizeof(szTemplate));
					inLen = strlen(pobTran->srTRec.srECCRec.szCardID);
					memcpy(szTemplate, pobTran->srTRec.srECCRec.szCardID, inLen);
				}
				else
				{
					memset(szTemplate, 0x00, sizeof(szTemplate));
					inLen = strlen(pobTran->srTRec.szUID);
					memcpy(szTemplate, pobTran->srTRec.szUID, inLen);
				}
			}
			else
			{
				/* 一卡通：遮掩後2 碼，遮掩字元為”*” */
				if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
				{
					memset(szTemplate, 0x00, sizeof(szTemplate));
					inLen = strlen(pobTran->srTRec.szUID);
					memcpy(szTemplate, pobTran->srTRec.szUID, inLen);
					szTemplate[inLen - 1] = 0x2A;
					szTemplate[inLen - 2] = 0x2A;
				}
				/*	1.簽單卡號列印邏輯
					<T4800>票卡版號(Purse Version Number)=0，<T0200>列印晶片卡號(Card Id)
					<T0200>20 bytes，14th,15th,16th隱碼，17th,18th,19th,20th顯示
					<T4800>票卡版號(Purse Version Number)<>0，<T0211>列印外觀卡號(Purse Id)
					<T0211>16 bytes，6th,7th,8th,9th,10th,11th隱碼，12th,13th,14th,15th,16th顯示
					，遮掩字元為”*”
					以<T0200>悠遊卡卡號遮掩從後往前算第五碼，第六碼，第七碼遮掩
					範例:以8碼卡號為例，卡號11651733，1,6,5要遮掩，實際列印為1***1733
					1th 2th 3th 4th 5th 6th 7th 8th 9th 10th 11th 12th 13th 14th 15th 16th 17th
					0 0 0 0 0 0 0 0 0 1 1 6 5 1 7 3 3
					0 0 0 0 0 0 0 0 0 1 * * * 1 7 3 3
					範例:以10碼卡號為例，卡號6611651733，1,6,5要遮掩，實際列印為661***1733
					1th 2th 3th 4th 5th 6th 7th 8th 9th 10th 11th 12th 13th 14th 15th 16th 17th
					0 0 0 0 0 0 0 6 6 1 1 6 5 1 7 3 3
					0 0 0 0 0 0 0 6 6 1 * * * 1 7 3 3

					2. 電子發票載具卡號為T0200 (不遮掩)
				*/
				else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
				{
					memset(szTemplate, 0x00, sizeof(szTemplate));
					inLen = strlen(pobTran->srTRec.szUID);
					memcpy(szTemplate, pobTran->srTRec.szUID, inLen);

					if (atoi(pobTran->srTRec.srECCRec.szPurseVersionNumber) == 0)
					{
						szTemplate[inLen - 5] = 0x2A;
						szTemplate[inLen - 6] = 0x2A;
						szTemplate[inLen - 7] = 0x2A;
					}
					else
					{
						/* 全部16 第6-11隱碼 */
						szTemplate[inLen - 6] = 0x2A;
						szTemplate[inLen - 7] = 0x2A;
						szTemplate[inLen - 8] = 0x2A;
						szTemplate[inLen - 9] = 0x2A;
						szTemplate[inLen - 10] = 0x2A;
						szTemplate[inLen - 11] = 0x2A;
					}
				}
				else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
				{
					memset(szTemplate, 0x00, sizeof(szTemplate));
					inLen = strlen(pobTran->srTRec.szUID);
					memcpy(szTemplate, pobTran->srTRec.szUID, inLen);
					/* 愛金卡：遮掩9-12 碼，遮掩字元為"*" */
					szTemplate[8] = 0x2A;
					szTemplate[9] = 0x2A;
					szTemplate[10] = 0x2A;
					szTemplate[11] = 0x2A;
				}
			}
			
			memcpy(&szDataBuffer[inPacketSizes], szTemplate, strlen(szTemplate));
		}
		/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/3 下午 6:18 */
		/* 不參考TMS遮掩開關 */
		else if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
		{
			memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szPAN, strlen(pobTran->srBRec.szPAN));
		}
		else
		{
			/* 卡號是否遮掩 */
			inRetVal = inECR_CardNoTruncateDecision(pobTran);
			/* 要遮卡號 */
			if (inRetVal == VS_SUCCESS)
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));
				strcpy(szTemplate, pobTran->srBRec.szPAN);

				/* HAPPG_GO 卡不掩飾 */
				if (!memcmp(&pobTran->srBRec.szPAN[0], "9552", 4))
				{

				}
				else
				{
					/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
					/* 卡號長度 */
					inCardLen = strlen(szTemplate);

					if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_088_TINTIN_, _CUSTOMER_INDICATOR_SIZE_))
					{
						/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
						for (i = 8; i < (inCardLen - 4); i ++)
							szTemplate[i] = '*';
					}
					else
					{    
						/* [2604013][115028-客製化參數090前8後4明碼專案需求-台北101] 修改遮掩規則由 前6後4改為前8後4 */
						for (i = 8; i < (inCardLen - 4); i ++)
							szTemplate[i] = '*';
					}
				}
				memcpy(&szDataBuffer[inPacketSizes], szTemplate, strlen(szTemplate));
			}
			else
			{
				memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szPAN, strlen(pobTran->srBRec.szPAN));
			}
		}
		inPacketSizes += 19;
	}

	/* Card Expire Date (4 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_END_CARD_NO_INQUIRY_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SETTLEMENT_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REPRINT_RECEIPT_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REVIEW_DETAIL_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REVIEW_TOTAL_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REPORT_DETAIL_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REPORT_TOTAL_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_EDC_REBOOT_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_INQUIRY_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REDEEM_REFUND_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REFUND_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_FULL_REDEEMPTION_, 2))
	{
		inPacketSizes += 4;
	}
	else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
	{
		inPacketSizes += 4;
	}
	/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/3 下午 6:18 */
	/* Uny交易不回傳有效期 */
	else if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
	{
		inPacketSizes += 4;
	}
	else
	{
		/* 有效期是否回傳 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetECRExpDateReturnEnable(szTemplate);
		
		/* 要回傳有效期 */
		if (!memcmp(&szTemplate, "Y", 1))
			memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szExpDate, strlen(pobTran->srBRec.szExpDate));

		inPacketSizes += 4;
	}

	/* Trans Amount (12 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TIP_, 2)				||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_CANCEL_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_CANCEL_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_START_CARD_NO_INQUIRY_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_END_CARD_NO_INQUIRY_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SETTLEMENT_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REPRINT_RECEIPT_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REVIEW_DETAIL_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REVIEW_TOTAL_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REPORT_DETAIL_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REPORT_TOTAL_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_EDC_REBOOT_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_INQUIRY_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_FULL_REDEEMPTION_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REFUND_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REDEEM_REFUND_, 2))
	{
	        inPacketSizes += 12;
	}
        else if (pobTran->srBRec.uszUPlanTransBit == VS_TRUE)
        {
                if (pobTran->srBRec.uszVOIDBit == VS_TRUE)
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "%010lu00", pobTran->srBRec.lnTxnAmount);
                        memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
                }
                else
                {
                    if (pobTran->srBRec.lnCUPUPlanDiscountedAmount > 0)
                    {
                            memset(szTemplate, 0x00, sizeof(szTemplate));
                            sprintf(szTemplate, "%010lu00", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnCUPUPlanPreferentialAmount));
                            memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
                    }
                    else
                    {
                            memset(szTemplate, 0x00, sizeof(szTemplate));
                            sprintf(szTemplate, "%010lu00", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnCUPUPlanPreferentialAmount));
                            memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
                    }
                }
                inPacketSizes += 12;
        }
        else
        {
                 if (pobTran->srTRec.lnTxnAmount != 0)
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "%010lu00", pobTran->srTRec.lnTxnAmount);
                        memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);

                        inPacketSizes += 12;
                }
                else if (pobTran->srBRec.lnTxnAmount != 0)
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "%010lu00", pobTran->srBRec.lnTxnAmount);
                        memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);

                        inPacketSizes += 12;
                }
                else
                {
                        inPacketSizes += 12;
                }
        }

	/* Trans Date (6 Byte) & Trans Time (6 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REFUND_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_OFFLINE_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TIP_, 2)				||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_CANCEL_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_CANCEL_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_REFUND_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_ADJUST_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_REFUND_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_ADJUST_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_TOP_UP_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_VOID_TOP_UP_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SETTLEMENT_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_INQUIRY_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_SALE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_INSTALLMENT_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REDEMPTION_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_CERTAIN_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_FULL_REDEEMPTION_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REDEEM_REFUND_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REFUND_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_AWARD_REDEEM_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_AWARD_REDEEM_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ECHO_, 2))
	{
		if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
		{
			/* Trans Date */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			strcat(szTemplate, pobTran->srTRec.szDate);
			memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 6);
			inPacketSizes += 6;

			/* Trans Time */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			strcat(szTemplate, pobTran->srTRec.szTime);
			memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 6);
			inPacketSizes += 6;
		}
		else
		{
			/* Trans Date */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			strcat(szTemplate, pobTran->srBRec.szDate);
			memcpy(&szDataBuffer[inPacketSizes], &szTemplate[2], 6);
			inPacketSizes += 6;

			/* Trans Time */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			strcat(szTemplate, pobTran->srBRec.szTime);
			memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 6);
			inPacketSizes += 6;
		}
	}
	else
	{
		inPacketSizes += 6;
		inPacketSizes += 6;
	}

	/* Approval No (9 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REFUND_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_OFFLINE_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TIP_, 2)				||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_CANCEL_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_CANCEL_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_REFUND_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_ADJUST_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_REFUND_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_ADJUST_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_TOP_UP_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_BALANCE_INQUIRY_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_VOID_TOP_UP_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_SALE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_INSTALLMENT_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REDEMPTION_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_CERTAIN_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_FULL_REDEEMPTION_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REDEEM_REFUND_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REFUND_, 2))
	{
		if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			strcpy(szTemplate, pobTran->srTRec.szAuthCode);
			inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 9, _PADDING_RIGHT_);
			memcpy(&szDataBuffer[inPacketSizes], szTemplate, 9);
		}
		else
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			strcpy(szTemplate, pobTran->srBRec.szAuthCode);
			inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 9, _PADDING_RIGHT_);
			memcpy(&szDataBuffer[inPacketSizes], szTemplate, 9);
		}
	}

	inPacketSizes += 9;

	/* Wave Card Indicator (1 Byte) */
	if (!memcmp(&srECROb->srTransData.szECRIndicator[0], "Q", 1))
	{
		/* 電子錢包交易不回傳Wave Card Indicator */
		/* (需求單-111155)-電子錢包業者收付訊息整合平台需求 by Russell 2023/7/24 下午 6:17 */
		/* Indicator為Q目前不送此欄位 */
	}
	else
	{
		if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
		{
			if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2)			||
			    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_START_CARD_NO_INQUIRY_, 2)	||
			    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_TOP_UP_, 2)			||
			    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_BALANCE_INQUIRY_, 2)	||
			    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_VOID_TOP_UP_, 2))
			{
				if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
				{
					memcpy(&szDataBuffer[inPacketSizes], "P", 1);
				}
				else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
				{
					memcpy(&szDataBuffer[inPacketSizes], "Z", 1);
				}
				else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
				{
					memcpy(&szDataBuffer[inPacketSizes], "G", 1);
				}
				else
				{
					memcpy(&szDataBuffer[inPacketSizes], "O", 1);
				}
			}
		}
		else
		{
			if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2)			||
			    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_START_CARD_NO_INQUIRY_, 2)	||
			    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_TOP_UP_, 2)			||
			    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_BALANCE_INQUIRY_, 2)	||
			    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_VOID_TOP_UP_, 2))
			{
				if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
				{
					/* 規格未寫M 跟單機同步 */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					strcpy(szTemplate, pobTran->srBRec.szCardLabel);

					if (!memcmp(szTemplate, _CARD_TYPE_VISA_, strlen(_CARD_TYPE_VISA_)))
						memcpy(&szDataBuffer[inPacketSizes], "V", 1);
					else if (!memcmp(szTemplate, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)))
						memcpy(&szDataBuffer[inPacketSizes], "M", 1);
					else if (!memcmp(szTemplate, _CARD_TYPE_JCB_, strlen(_CARD_TYPE_JCB_)))
						memcpy(&szDataBuffer[inPacketSizes], "J", 1);
					else if (!memcmp(szTemplate, _CARD_TYPE_CUP_, strlen(_CARD_TYPE_CUP_)))
						memcpy(&szDataBuffer[inPacketSizes], "C", 1);
					else if (!memcmp(szTemplate, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)))
						memcpy(&szDataBuffer[inPacketSizes], "A", 1);
					/* (需求單 - 106349) 2018/10/31 下午 3:41 by Russell */
					else if (!memcmp(szTemplate, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)) ||
						 pobTran->srBRec.uszUCARDTransBit == VS_TRUE)
					{
						if (pobTran->srBRec.uszUCARDTransBit == VS_TRUE)
						{
							memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szMCP_BANKID[4], 1);
						}
						else
						{
							memcpy(&szDataBuffer[inPacketSizes], "D", 1);
						}
					}
					else
						memcpy(&szDataBuffer[inPacketSizes], "O", 1);
				}
			}
		}
	}
	inPacketSizes ++;

	/* ECR Response Code (4 Byte) */
	memcpy(&szDataBuffer[inPacketSizes], "0000", 4);
	
	inPacketSizes += 4;

	/* Merchant ID (15 Byte) & Terminal ID (8 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REPORT_DETAIL_, 2) ||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REPORT_TOTAL_, 2) ||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SETTLEMENT_, 2))
	{
		inPacketSizes += 15;
		inPacketSizes += 8;
	}
	else
	{
		/* Merchant ID (15 Byte) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetMerchantID(szTemplate);
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 15, _PADDING_RIGHT_);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 15);
		inPacketSizes += 15;
		/* Terminal ID (8 Byte) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTerminalID(szTemplate);
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 8, _PADDING_RIGHT_);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 8);
		inPacketSizes += 8;
	}

	/* Exp Amount (12 Byte) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTRTFileName(szTemplate);
        if (!memcmp(szTemplate, _TRT_FILE_NAME_DCC_, _TRT_FILE_NAME_DCC_LEN_) && !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TIP_, 2))
        {
                if (!memcmp(&pobTran->srBRec.szDCC_FCMU[0], "0", 1))
                {
        	        memset(szTemplate, 0x00, sizeof(szTemplate));
        		sprintf(szTemplate, "%010ld00", atol(pobTran->srBRec.szDCC_TIPFCA));
        		memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 12);
        	}
                else if (!memcmp(&pobTran->srBRec.szDCC_FCMU[0], "1", 1))
                {
        	        memset(szTemplate, 0x00, sizeof(szTemplate));
        		sprintf(szTemplate, "%011ld0", atol(pobTran->srBRec.szDCC_TIPFCA));
        		memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 12);
        	}
        	else if (!memcmp(&pobTran->srBRec.szDCC_FCMU[0], "2", 1))
                {
        	        memset(szTemplate, 0x00, sizeof(szTemplate));
        		sprintf(szTemplate, "%012ld", atol(pobTran->srBRec.szDCC_TIPFCA));
        		memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 12);
        	}
        }
        else if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TIP_, 2))
        {
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010lu00", pobTran->srBRec.lnTipTxnAmount);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
	}

	inPacketSizes += 12;
	
	/* Store Id (18 Byte) */
	inPacketSizes += 18;

	/* 處理紅利、分期 */
	/* Installment / Redeem Indictor (1 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_REFUND_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_ADJUST_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_REFUND_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_ADJUST_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REDEMPTION_, 2) ||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_INSTALLMENT_, 2))
	{
		if (pobTran->srBRec.uszInstallmentBit == VS_TRUE)
			memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szInstallmentIndicator[0], 1);
		else if (pobTran->srBRec.uszRedeemBit == VS_TRUE)
			memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szRedeemIndicator[0], 1);
	}

	/* 優惠兌換方式 */
	/* 1.條碼兌換 2.卡號兌換 */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_AWARD_REDEEM_, 2))
	{
		if (!memcmp(pobTran->szL3_AwardWay, "1", 1)	||
		    !memcmp(pobTran->szL3_AwardWay, "2", 1)	||
		    !memcmp(pobTran->szL3_AwardWay, "3", 1))
		{
			memcpy(&szDataBuffer[inPacketSizes], "1", 1);
		}
		else if (!memcmp(pobTran->szL3_AwardWay, "4", 1)	||
			 !memcmp(pobTran->szL3_AwardWay, "5", 1))
		{
			memcpy(&szDataBuffer[inPacketSizes], "2", 1);
		}
	}
	else if(!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_AWARD_REDEEM_, 2))
	{
		/* (1= 條碼兌換， 2= 卡號兌換 )*/
		/* 取消優惠兌換只接受 條碼兌換 。 */
		memcpy(&szDataBuffer[inPacketSizes], "1", 1);
	}
	inPacketSizes ++;
	
	/* 處理紅利扣抵 */
	if ((!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_, 2)		||
	     !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_REFUND_, 2)	||
	     !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_ADJUST_, 2)	||
	     !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REDEMPTION_, 2)) && pobTran->srBRec.uszRedeemBit == VS_TRUE)
	{
		/* 支付金額 RDM Paid Amt (12 Byte) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010lu00", (pobTran->srBRec.lnRedemptionPaidCreditAmount + pobTran->srBRec.lnTipTxnAmount));
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
		inPacketSizes += 12;
		/* 紅利扣抵點數 RDM Point (8 Byte) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%08lu", pobTran->srBRec.lnRedemptionPoints);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 8);
		inPacketSizes += 8;
		/* 紅利剩餘點數 Points Of Balance (8 Byte) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%08lu", pobTran->srBRec.lnRedemptionPointsBalance);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 8);
		inPacketSizes += 8;
		/* 紅利扣抵金額 Redeem Amt (12 Byte) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010lu00", (pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnRedemptionPaidCreditAmount));
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
		inPacketSizes += 12;
	}
	else if (pobTran->srBRec.uszUPlanTransBit == VS_TRUE)
	{
		if (pobTran->srBRec.uszVOIDBit == VS_TRUE)
		{
			inPacketSizes += 40;
		}
		else
		{
			/* 銀聯優計劃：優惠後金額(含小數2 位) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%010lu00", pobTran->srBRec.lnCUPUPlanDiscountedAmount);
			memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
			inPacketSizes += 12;

			/* reserved */
			inPacketSizes += 8;

			/* reserved */
			inPacketSizes += 8;

			/* 銀聯優計劃：優惠金額(含小數2 位) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%010lu00", pobTran->srBRec.lnCUPUPlanPreferentialAmount);
			memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
			inPacketSizes += 12;
		}
	}
	else
		inPacketSizes += 40;

	/* 處理分期交易 */
	if ((!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_, 2)	||
	     !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_REFUND_, 2)	||
	     !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_ADJUST_, 2)	||
	     !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_INSTALLMENT_, 2)) && pobTran->srBRec.uszInstallmentBit == VS_TRUE)
	{
		/* 分期期數 Installment Period (2 Byte) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%02ld", pobTran->srBRec.lnInstallmentPeriod);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 2);
		inPacketSizes += 2;
		/* 首期金額 Down Payment (12 Byte) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010lu00", pobTran->srBRec.lnInstallmentDownPayment);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
		inPacketSizes += 12;
		/* 每期金額 Installment Payment Amt (12 Byte) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010lu00", pobTran->srBRec.lnInstallmentPayment);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
		inPacketSizes += 12;
		/* 分期手續費 Formlity Fee (12 Byte) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010lu00", pobTran->srBRec.lnInstallmentFormalityFee);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
		inPacketSizes += 12;
	}
	else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
	{
		inPacketSizes += 2;
		
		/* 交易前餘額 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_BALANCE_INQUIRY_, 2)	||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_START_CARD_NO_INQUIRY_, 2)	||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_END_CARD_NO_INQUIRY_, 2)		||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REPRINT_RECEIPT_, 2)		||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REVIEW_TOTAL_, 2)		||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REVIEW_DETAIL_, 2))
		{
			/* 查餘額不帶此欄位 */
			/* 啟動卡號查詢不帶此欄位 */
			sprintf(szTemplate, "            ");
		}
		else
		{
			if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
			{
				if (pobTran->srTRec.lnFinalBeforeAmt > 100000)
				{
					sprintf(szTemplate, "-%09lu00", (pobTran->srTRec.lnFinalBeforeAmt - 100000));
				}
				else
				{
					/* 交易前餘額要帶未加值的金額 */
					if ((pobTran->srTRec.lnCardRemainAmount - pobTran->srTRec.lnTotalTopUpAmount) < 0)
						sprintf(szTemplate, "-%09lu00", (unsigned long)(0 - (pobTran->srTRec.lnCardRemainAmount - pobTran->srTRec.lnTotalTopUpAmount)));
					else
						sprintf(szTemplate, "+%09lu00", (unsigned long)pobTran->srTRec.lnCardRemainAmount - pobTran->srTRec.lnTotalTopUpAmount);
				}
			}
			else
			{
				if (pobTran->srTRec.lnFinalBeforeAmt < 0)
				{
					sprintf(szTemplate, "-%09lu00", (0 - pobTran->srTRec.lnFinalBeforeAmt));
				}
				else
				{
					sprintf(szTemplate, "+%09lu00", pobTran->srTRec.lnFinalBeforeAmt);
				}
			}
			
		}

		memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 12);
		inPacketSizes += 12;

		/* 交易後餘額 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_START_CARD_NO_INQUIRY_, 2)	||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_END_CARD_NO_INQUIRY_, 2)		||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REPRINT_RECEIPT_, 2)		||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REVIEW_TOTAL_, 2)		||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REVIEW_DETAIL_, 2))
		{
			/* 啟動卡號查詢不帶此欄位 */
			sprintf(szTemplate, "            ");
		}
		else if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_BALANCE_INQUIRY_, 2))
		{
			if (pobTran->srTRec.lnCardRemainAmount < 0)
			{
				sprintf(szTemplate, "-%09lu00", (unsigned long)(0 - pobTran->srTRec.lnCardRemainAmount));
			}
			else
			{
				sprintf(szTemplate, "+%09lu00", (unsigned long)pobTran->srTRec.lnCardRemainAmount);
			}
		}
		else
		{
			if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
			{
				if (pobTran->srTRec.lnFinalAfterAmt > 100000)
					sprintf(szTemplate, "-%09lu00", (unsigned long)(pobTran->srTRec.lnFinalAfterAmt - 100000));
				else
					sprintf(szTemplate, "+%09lu00", (unsigned long)(pobTran->srTRec.lnFinalAfterAmt));
			}
			else
			{
				if (pobTran->srTRec.lnFinalAfterAmt < 0)
					sprintf(szTemplate, "-%09lu00", (unsigned long)(0 - pobTran->srTRec.lnFinalAfterAmt));
				else
					sprintf(szTemplate, "+%09lu00", (unsigned long)(pobTran->srTRec.lnFinalAfterAmt));
			}
		}

		memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 12);
		inPacketSizes += 12;

		/* 自動加值金額 */
		if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2))
	        {
        		memset(szTemplate, 0x00, sizeof(szTemplate));
        		if (pobTran->srTRec.lnTotalTopUpAmount > 0L)
                                sprintf(szTemplate, "+%09lu00", (unsigned long)(pobTran->srTRec.lnTotalTopUpAmount));
                        else
                                sprintf(szTemplate, "+%09lu00", (unsigned long)(0));

                        memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 12);
                }
		inPacketSizes += 12;
	}
	else
		inPacketSizes += 38;

	/* Card Type (2 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_START_CARD_NO_INQUIRY_, 2)					||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2)							||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REFUND_, 2)							||
	    (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_, 2) && pobTran->srBRec.uszHappyGoSingle != VS_TRUE)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_OFFLINE_, 2)							||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TIP_, 2)								||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_, 2)							||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_CANCEL_, 2)						||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_, 2)						||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_CANCEL_, 2)					||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_, 2)							||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_REFUND_, 2)						||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_ADJUST_, 2)						||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_, 2)							||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_REFUND_, 2)						||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_ADJUST_, 2)						||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_TOP_UP_, 2)							||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_BALANCE_INQUIRY_, 2)					||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_VOID_TOP_UP_, 2)						||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_INSTALLMENT_, 2)					||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REDEMPTION_, 2)					||
	    ((!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_SALE_, 2)						||
	      !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_CERTAIN_, 2)						||
	      !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2)) && pobTran->srBRec.uszHappyGoMulti == VS_TRUE))
	{
		if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
		{
			if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
			{
				memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_IPASS_, 2);
			}
			else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
			{
				memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_ECC_, 2);
			}
			else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
			{
				memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_ICASH_, 2);
			}
			else
				memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_UNKNOWN_, _ECR_8N1_NCCC_CARDTYPE_UNKNOWN_LEN_);
		}
		else
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			strcpy(szTemplate, pobTran->srBRec.szCardLabel);

			/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
			if (!memcmp(szTemplate, _CARD_TYPE_VISA_, strlen(_CARD_TYPE_VISA_)))
				memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_VISA_, 2);
			else if (!memcmp(szTemplate, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)))
				memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_MASTERCARD_, 2);
			else if (!memcmp(szTemplate, _CARD_TYPE_JCB_, strlen(_CARD_TYPE_JCB_)))
				memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_JCB_, 2);
			else if (!memcmp(szTemplate, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)))
				memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_AMEX_, 2);
			else if (!memcmp(szTemplate, _CARD_TYPE_CUP_, strlen(_CARD_TYPE_CUP_)))
				memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_CUP_, 2);
			else if (!memcmp(szTemplate, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)) ||
				 pobTran->srBRec.uszUCARDTransBit == VS_TRUE)
			{
				if (pobTran->srBRec.uszUCARDTransBit == VS_TRUE)
				{
					memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_UCARD_, 2);
				}
				else
				{
					memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_DINERS_, 2);
				}
			}
			else if (!memcmp(szTemplate, _CARD_TYPE_SMARTPAY_, strlen(_CARD_TYPE_SMARTPAY_)))
				memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_SMARTPAY_, 2);
			else
				memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_UNKNOWN_, _ECR_8N1_NCCC_CARDTYPE_UNKNOWN_LEN_);
		}
	}

	inPacketSizes += 2;

	/* Batch No (6 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REFUND_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_OFFLINE_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TIP_, 2)				||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_CANCEL_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_CANCEL_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_REFUND_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_ADJUST_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_REFUND_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_ADJUST_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_TOP_UP_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_VOID_TOP_UP_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_SALE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_INSTALLMENT_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REDEMPTION_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_CERTAIN_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_FULL_REDEEMPTION_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REDEEM_REFUND_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REFUND_, 2))
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetBatchNum(szTemplate);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 6);
	}

	inPacketSizes += 6;

	/* Start Trans Type (2 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_START_CARD_NO_INQUIRY_, 2))
		memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szStartTransType, 2);

	inPacketSizes += 2;

	/* MP Flag(是否小額交易，MPAS小額交易用) (1 Byte) */
	/* 合併MPAS ECR到標準400 */
	memset(szFESMode, 0x00, sizeof(szFESMode));
	inGetNCCCFESMode(szFESMode);
	if (memcmp(szFESMode, _NCCC_04_MPAS_MODE_, strlen(_NCCC_04_MPAS_MODE_)) == 0)
	{
		if (pobTran->srBRec.uszMPASTransBit == VS_TRUE)
			memcpy(&szDataBuffer[inPacketSizes], "M", 1);
		else
			memcpy(&szDataBuffer[inPacketSizes], " ", 1);
	}
	inPacketSizes += 1;

	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_END_CARD_NO_INQUIRY_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SETTLEMENT_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REPRINT_RECEIPT_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REVIEW_DETAIL_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REVIEW_TOTAL_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REPORT_DETAIL_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REPORT_TOTAL_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_EDC_REBOOT_, 2))
	{
		/* 共99Bytes */
		inPacketSizes += 8;
		inPacketSizes += 8;
		inPacketSizes += 12;
		inPacketSizes += 5;
		inPacketSizes += 50;
		inPacketSizes += 6;
		inPacketSizes += 1;
		inPacketSizes += 1;
		inPacketSizes += 3;
		inPacketSizes += 5;
	}
	else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
	{
		inPacketSizes += 8;
		
		/* ESVC Origin Date */
		/* 電票退貨要帶回原交易日期 */
		if (memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REFUND_, 2) == 0)
		{
			memset(szTemp, 0x00, sizeof(szTemp));
			memset(szTemp2, 0x00, sizeof(szTemp2));
			/* [20251219_BUG_MDF][ECC] 修改電票退貨的判斷日期,由 2位MM改為 4位 MMDD */
			memcpy(szTemp2, pobTran->srTRec.szTicketRefundDate, 4);

			inNCCC_TICKET_Decide_Year(szTemp, szTemp2);
			memcpy(&szDataBuffer[inPacketSizes], szTemp, 4); 
			inPacketSizes += 4;
			
			memcpy(&szDataBuffer[inPacketSizes], &pobTran->srTRec.szTicketRefundDate[0], 4);
			inPacketSizes += 4;
		}
		else
		{
			inPacketSizes += 8;
		}

		/* 餘額查詢不用帶RF序號 */
		if (memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_BALANCE_INQUIRY_, 2) == 0)
		{
			inPacketSizes += 12;
		}
		else
		{
			/* RF序號 */
			memcpy(&szDataBuffer[inPacketSizes], &pobTran->srTRec.szTicketRefundCode[0], 6);
			inPacketSizes += 12;
		}

		/* Pay Item */
		inPacketSizes += 5;
		
		/* Card No. Hash Value */
                if (!memcmp(&srECROb->srTransData.szECRIndicator[0], "Q", 1))
                {
                        if (pobTran->uszCardInquiryFirstBit == VS_TRUE	||
                            !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_END_CARD_NO_INQUIRY_, 2)	||
                            !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_AWARD_REDEEM_, 2))
                        {
                                inPacketSizes += 6;
                                inPacketSizes += 44;
                        }
                        else
                        {
                                memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szEIVI_BANKID[0], 6);
                                inPacketSizes += 6;
                                memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szEInvoiceHASH[0], 44);
                                inPacketSizes += 44;
                        }
                }
		else if (!memcmp(&srECROb->srTransData.szECRIndicator[0], "E", 1))
		{
			/* 兩段式ECR電票，第一段取不到真正的卡號，填空白 */
			if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));
			}
			else if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_TOP_UP_, 2)		||
				 !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_VOID_TOP_UP_, 2)	||
				 !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_BALANCE_INQUIRY_, 2))
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));
			}
			else
			{
				if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
				{
					memset(szTemplate, 0x00, sizeof(szTemplate));
					strcpy(szTemplate, pobTran->srTRec.szUID);
				}
				else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
				{
					/* 【需求單-108240】EDC修改電票需求 悠遊卡指定要T0200 by Russell 2020/6/4 下午 2:02 */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					strcpy(szTemplate, pobTran->srTRec.srECCRec.szCardID);
				}
				else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
				{
					memset(szTemplate, 0x00, sizeof(szTemplate));
					strcpy(szTemplate, pobTran->srTRec.szUID);	
				}
			}
		}
		else
		{
			if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_TOP_UP_, 2)		||
			    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_VOID_TOP_UP_, 2)	||
			    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_BALANCE_INQUIRY_, 2))
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));
			}
			else
			{
				if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
				{
					memset(szTemplate, 0x00, sizeof(szTemplate));
					strcpy(szTemplate, pobTran->srTRec.szUID);
				}
				else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
				{
					/* 【需求單-108240】EDC修改電票需求 悠遊卡指定要T0200 by Russell 2020/6/4 下午 2:02 */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					strcpy(szTemplate, pobTran->srTRec.srECCRec.szCardID);
				}
				else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
				{
					memset(szTemplate, 0x00, sizeof(szTemplate));
					strcpy(szTemplate, pobTran->srTRec.szUID);	
				}
			}
		}
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, strlen(szTemplate));
		inPacketSizes += 50;
		
		/* MP Response Code */
		inPacketSizes += 6;
		
		/* ASM award flag */
		/* 【需求單-108215】電票交易支援優惠平台需求 by Russell 2020/5/11 下午 3:53 */
		if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2))
		{
			if ((pobTran->srTRec.uszRewardL1Bit == VS_TRUE	||
			     pobTran->srTRec.uszRewardL2Bit == VS_TRUE	||
			     pobTran->srTRec.uszRewardL5Bit == VS_TRUE)	&&
			     pobTran->srTRec.uszRewardSuspendBit != VS_TRUE)
			{
				memcpy(&szDataBuffer[inPacketSizes], "A", strlen("A"));
			}
			else
			{
				memcpy(&szDataBuffer[inPacketSizes], " ", strlen(" "));
			}
		}
		else
		{
			memcpy(&szDataBuffer[inPacketSizes], " ", strlen(" "));
		}
		inPacketSizes += 1;
		
		/* MCP Indicator & etc.*/
		inPacketSizes += 9;
	}
	else
	{
		/* SmartPay要回傳這三項資訊 (99 Bytes) */
		if ((!memcmp(&srECROb->srTransData.szField_05[0], "S", 1) || pobTran->srBRec.uszFiscTransBit == VS_TRUE) &&
		     pobTran->uszCardInquiryFirstBit != VS_TRUE)
		{
			/* SP ISSUER ID (8 Byte) */
			memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szFiscIssuerID[0], 8);
			inPacketSizes += 8;
			/* SP Origin Date (8 Byte) */
			if (strlen(pobTran->srBRec.szFiscRefundDate) > 0)
                        {
                                memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szFiscRefundDate[0], 8);
                        }
			else
                        {
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                strcat(szTemplate, pobTran->srBRec.szDate);
                                memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 8);
                        }
			inPacketSizes += 8;
			/* SP RRN (12 Byte) */
			memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szFiscRRN[0], 12);
			inPacketSizes += 12;
		}
		else
		{
			inPacketSizes += 8;
			inPacketSizes += 8;
			inPacketSizes += 12;
		}

		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetPayItemEnable(szTemplate);
		if (memcmp(szTemplate, "Y", strlen("Y")) == 0	&&
		   (strlen(pobTran->srBRec.szPayItemCode) > 0))
		{
			/* Pay Item (5 Bytes) */
			/* ECR一段式收銀機連線 送空白payitem 不必回傳 */
			if (srECROb->srTransData.uszECRResponsePayitem == VS_FALSE)
			{
				inPacketSizes += 5;
			}
			else
			{
				memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szPayItemCode[0], 5);
				inPacketSizes += 5;
			}
		}
		else
		{
			inPacketSizes += 5;
		}
		/* 【需求單 - 105039】信用卡為電子發票載具，端末機將卡號加密後回傳收銀機 add by LingHsiung 2016-04-20 上午 09:56:24 */
		/* 【需求單 - 108046】電子發票BIN大於6碼需求 by Russell 2019/7/8 上午 11:44 */
		/* 收銀機欄位ECR Indicator = “E” : 
		 * (1)銀行代碼為 ”B00999” : Card No. Hash Value = 空白50碼
		 * (2)其他 :Card No. Hash Value = 銀行代碼6碼 + Hash Value 44碼。
		*/
		/* Card No. Hash Value (50 Bytes) */
		/* Smartpay 和 DCC 因為不回傳"NI"，所以不回傳hash value */
		/* 只要沒回傳Table "NI" 一律回50個空白 */
		/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/3 下午 6:18 */
		/* Üny 交易：
		   電子發票加密卡號 (B00xxx ’’+44 碼 HASH 值 xxx為金融機構代碼) */
		if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
		{
			inPacketSizes += 50;
		}
		else
		{
			if (!memcmp(&srECROb->srTransData.szECRIndicator[0], "Q", 1))
			{
				if (pobTran->uszCardInquiryFirstBit == VS_TRUE	||
				    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_END_CARD_NO_INQUIRY_, 2) ||
                                    pobTran->srBRec.uszUPlanECRBit == VS_TRUE)
				{
					inPacketSizes += 6;
					inPacketSizes += 44;
				}
				else
				{
					memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szEIVI_BANKID[0], 6);
					inPacketSizes += 6;
					memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szEInvoiceHASH[0], 44);
					inPacketSizes += 44;
				}
			}
			else if (!memcmp(&srECROb->srTransData.szECRIndicator[0], "E", 1))
			{
				/* 啟動卡號查詢只有在Indicator = "I" 才送Hash Value("E"一律送50個空白) */
				/* 因為卡號查詢會轉交易別，所以用Flag判斷 */
				if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
				{
					inPacketSizes += 50;
				}
				else
				{
					/* [260610] 註解 101客制 Trans Type為 98, 不需要回傳 HASH值 */
					if(!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ECHO_, 2) )
					{
						inPacketSizes += 50;
					}else if (strlen(pobTran->srBRec.szEIVI_BANKID) > 0)
					{
						/* 【111191】客製化參數090在ECR E開頭的全信用卡交易都提供Hash值，沒有回傳BANKID要幫補B00999 */
						memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szEIVI_BANKID[0], 6);
						inPacketSizes += 6;

						memset(szTemplate, 0x00, sizeof(szTemplate));
						if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_AWARD_REDEEM_, 2))
						{
							
						}
						else
						{
							memcpy(&szTemplate[0], &pobTran->srBRec.szPAN[0], strlen(pobTran->srBRec.szPAN));
						}
						
						if (strlen(szTemplate) > 0)
						{
							memset(szHash, 0x00, sizeof(szHash));
							inNCCC_Func_CardNumber_Hash(szTemplate, szHash);
							memcpy(&szDataBuffer[inPacketSizes], &szHash[0], 44);
						}
						inPacketSizes += 44;
					}
					else
					{
						/* 【111191】客製化參數090在ECR E開頭的全信用卡交易都提供Hash值，沒有回傳BANKID要幫補B00999 */
						memcpy(&szDataBuffer[inPacketSizes], "B00999", 6);
						inPacketSizes += 6;

						memset(szTemplate, 0x00, sizeof(szTemplate));
						if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_AWARD_REDEEM_, 2))
						{
							
						}
						else
						{
							memcpy(&szTemplate[0], &pobTran->srBRec.szPAN[0], strlen(pobTran->srBRec.szPAN));
						}
						
						if (strlen(szTemplate) > 0)
						{
							memset(szHash, 0x00, sizeof(szHash));
							inNCCC_Func_CardNumber_Hash(szTemplate, szHash);
							memcpy(&szDataBuffer[inPacketSizes], &szHash[0], 44);
						}
						inPacketSizes += 44;
					}
				}
			}
			/* 收銀機欄位ECR Indicator = “I” : Card No. Hash Value = Card number前6碼 + Hash Value 44碼(原規格) */
			else
			{
				/* [260610] 註解 101客制 Trans Type為 98, 不需要回傳 HASH值 */
				if(!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ECHO_, 2) )
				{
					inPacketSizes += 50;
				}else
				{
					memset(szTemplate, 0x00, sizeof(szTemplate));
					if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_AWARD_REDEEM_, 2))
					{

					}
					else
					{
						/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
						memcpy(&szTemplate[0], &pobTran->srBRec.szPAN[0], strlen(pobTran->srBRec.szPAN));
					}

					if (strlen(szTemplate) > 0)
					{
						memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 6);
						inPacketSizes += 6;

						memset(szHash, 0x00, sizeof(szHash));
						inNCCC_Func_CardNumber_Hash(szTemplate, szHash);
						memcpy(&szDataBuffer[inPacketSizes], &szHash[0], 44);
						inPacketSizes += 44;
					}
					else
					{
						inPacketSizes += 50;
					}
				}
			}
		}

		/* MP Response Code (6 Bytes) */
		memset(szFESMode, 0x00, sizeof(szFESMode));
		inGetNCCCFESMode(szFESMode);
		if (memcmp(szFESMode, _NCCC_04_MPAS_MODE_, strlen(_NCCC_04_MPAS_MODE_)) == 0)
		{
			if (memcmp(pobTran->srBRec.szRespCode, "00", 2) != 0		&& 
			    memcmp(pobTran->srBRec.szRespCode, "11", 2) != 0		&&
			    pobTran->srBRec.uszMPASTransBit == VS_TRUE			&&
			    (memcmp(&pobTran->srBRec.szMPASAuthCode[0], "CAF", 3) == 0	||
			     memcmp(&pobTran->srBRec.szMPASAuthCode[0], "NEG", 3) == 0	||
			     memcmp(&pobTran->srBRec.szMPASAuthCode[0], "WEB", 3) == 0	|| 
			     memcmp(&pobTran->srBRec.szMPASAuthCode[0], "REJ", 3) == 0))
			{
				memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szMPASAuthCode[0], 6);
			}
		}
		inPacketSizes += 6;

		/* ASM award flag */
		if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2)	||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_OFFLINE_, 2)	||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_, 2)	||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_, 2))
		{	
			if ((pobTran->srBRec.uszRewardL1Bit == VS_TRUE	||
			     pobTran->srBRec.uszRewardL2Bit == VS_TRUE	||
			     pobTran->srBRec.uszRewardL5Bit == VS_TRUE)	&&
			     pobTran->srBRec.uszRewardSuspendBit != VS_TRUE)
			{
				memcpy(&szDataBuffer[inPacketSizes], "A", strlen("A"));
			}
			else
			{
				memcpy(&szDataBuffer[inPacketSizes], " ", strlen(" "));
			}
		}
		else
		{
			memcpy(&szDataBuffer[inPacketSizes], " ", strlen(" "));
		}
		inPacketSizes += 1;

		/* 【需求單 - 106128】行動支付標記及金融機構代碼 add by LingHsiung 2017-08-23 上午 11:48:57 */
		if (strlen(pobTran->srBRec.szMCP_BANKID) > 0)
		{
			/* MCP Indicator (1 Bytes) */
			memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szMCP_BANKID[0], 1);
			inPacketSizes += 1;

			/* 金融機構代碼 (3 Bytes) */
			memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szMCP_BANKID[1], 3);
			inPacketSizes += 3;
		}
		else
		{
			inPacketSizes += 4;
		}

		/* Reserve (5 Bytes) */
		inPacketSizes += 5;
	}

	/* HG Data (78 Byte) */
	if (pobTran->srBRec.lnHGTransactionType != 0	&&
	    (pobTran->srBRec.uszHappyGoMulti == VS_TRUE || pobTran->srBRec.uszHappyGoSingle == VS_TRUE))
	{
		/* Payment Tools (2 Byte) */
		if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2) ||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_CERTAIN_, 2))
		{
                	if (pobTran->srBRec.lnHGPaymentType == _HG_PAY_CREDIT_)
                		memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_HG_PAYMENT_TOOL_TYPE_CREDIT_CARD_, 2);
                	else if (pobTran->srBRec.lnHGPaymentType == _HG_PAY_CASH_)
                		memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_HG_PAYMENT_TOOL_TYPE_CASH_, 2);
                	else if (pobTran->srBRec.lnHGPaymentType == _HG_PAY_GIFT_PAPER_)
                		memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_HG_PAYMENT_TOOL_TYPE_GIFT_PAPER_, 2);
                	else if (pobTran->srBRec.lnHGPaymentType == _HG_PAY_CREDIT_INSIDE_)
                		memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_HG_PAYMENT_TOOL_TYPE_HGI_, 2);
                	else if (pobTran->srBRec.lnHGPaymentType == _HG_PAY_CUP_)
                		memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_HG_PAYMENT_TOOL_TYPE_CUP_CARD_, 2);
                }

		inPacketSizes += 2;

		/* HG Card Number (18 Byte) */
		/* 重印HG混信用卡的交易簽單不回傳HG卡號 */
		if (memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REPRINT_RECEIPT_, 2) != 0)
		{
		        inRetVal = inECR_CardNoTruncateDecision_HG(pobTran);
			/* 要遮卡號 */
			if (inRetVal == VS_SUCCESS)
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));
				strcpy(szTemplate, pobTran->srBRec.szHGPAN);

				/* HAPPG_GO 卡不掩飾 */
				if (!memcmp(&pobTran->srBRec.szHGPAN[0], "9552", 4))
				{

				}
				else
				{
					/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
					/* 卡號長度 */
					inCardLen = strlen(szTemplate);

                                        if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_088_TINTIN_, _CUSTOMER_INDICATOR_SIZE_))
                                        {
                                                /* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
                                                for (i = 8; i < (inCardLen - 4); i ++)
                                                        szTemplate[i] = '*';
                                        }
                                        else
                                        {    
                                                /* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
                                                for (i = 6; i < (inCardLen - 4); i ++)
                                                        szTemplate[i] = '*';
                                        }
				}

				memcpy(&szDataBuffer[inPacketSizes], szTemplate, strlen(szTemplate));
			}
			else
			{
				memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szHGPAN, strlen(pobTran->srBRec.szHGPAN));
			}
		
        	}

		inPacketSizes += 18;

		/* HG Pay Amount (12 Byte) 實際支付金額 */
		if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2))
		{
        		memset(szTemplate, 0x00, sizeof(szTemplate));
        		sprintf(szTemplate, "%010lu00", pobTran->srBRec.lnHGAmount);
        		memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 12);
        	}

        	inPacketSizes += 12;
		/* HG Redeem Amount (12 Byte) 扣抵金額 */
		if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2))
		{
        		memset(szTemplate, 0x00, sizeof(szTemplate));
        		sprintf(szTemplate, "%010lu00", pobTran->srBRec.lnHGRedeemAmount);
        		memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 12);
        	}

        	inPacketSizes += 12;
		/* HG Redeem Point (8 Byte) 扣抵點數 */
		if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2)	||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_CERTAIN_, 2)		||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_FULL_REDEEMPTION_, 2)		||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REDEEM_REFUND_, 2)		||
	            !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REFUND_, 2))
		{
        		memset(szTemplate, 0x00, sizeof(szTemplate));
        		sprintf(szTemplate, "%08lu", pobTran->srBRec.lnHGTransactionPoint);
        		memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 8);
        	}

        	inPacketSizes += 8;
		/* HG Lack Point (8 Byte) 不足點數*/
		if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REFUND_, 2))
		{
        		memset(szTemplate, 0x00, sizeof(szTemplate));
        		sprintf(szTemplate, "%08lu", pobTran->srBRec.lnHGRefundLackPoint);
        		memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 8);
        	}

		inPacketSizes += 8;
		/* HG Balance Point (8 Byte) 剩餘點數 */
		if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2) ||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_CERTAIN_, 2) ||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_FULL_REDEEMPTION_, 2) ||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REDEEM_REFUND_, 2) ||
	            !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REFUND_, 2) ||
	            !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_INQUIRY_, 2) ||
	            !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_, 2))
		{
        		memset(szTemplate, 0x00, sizeof(szTemplate));
        		sprintf(szTemplate, "%08lu", pobTran->srBRec.lnHGBalancePoint);
        		memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 8);
        	}

		inPacketSizes += 8;
		/* HG Reserve (10 Byte) */
		inPacketSizes += 10;
	}
	else
		inPacketSizes += 78;
	
	if (gbBarCodeECRBit == VS_TRUE)
	{
		/* 一維或二維條碼資料長度 */
		inBarCodeLen = atoi(pobTran->srBRec.szBarCodeData);
		inPacketSizes += 3;
		/* 一維或二維條碼資料內容 */
		inPacketSizes += inBarCodeLen;
		
		/* 補滿至980 */
		inPacketSizes = 980;
		
		/* Üny 交易碼 */
		if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
		{
			/* 除結束卡號查詢不用，其他Uny交易都要 */
			if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_END_CARD_NO_INQUIRY_, 2))
			{
				
			}
			else
			{
				memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szUnyTransCode, 20);
			}
		}
		
		inPacketSizes += 20;
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inECR_7E1_Customer_035_MIRAMAR_CINEMAS_Unpack
Date&Time       :2022/7/14 下午 3:16
Describe        :客製化035 結帳交易別為_ECR_7E1_MIRAMAR_CINEMAS_SETTLEMENT_NO_ 55
*/
int inECR_7E1_Customer_035_MIRAMAR_CINEMAS_Unpack(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, ECR_TABLE * srECROb)
{
	int	i = 0;
	int	inRetVal = VS_SUCCESS;
	int	inTransType = 0;		/* 電文中的交易別字串轉為數字儲存 */
        int	inCardInquiryTRTType = _CARD_INQUIRY_TYPE_NONE_;
	char	szTemplate[100 + 1] = {0};
	char	szDebugMsg[100 + 1] = {0};	
	char	szHostLabel[8 + 1] = {0};
	char	szHostEnable[2 + 1] = {0};
	char	szBatchNum[6 + 1] = {0};
	char	szCTLSEnable[2 + 1] = {0};
	char	szFiscContactlessEnable[1 + 1] = {0};
	char	szCUPContactlessEnable[1 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
	
	/* (需求單 - 107276)自助交易標準做法 EDC於發動交易及完成時，需發出提示音 by Russell 2019/3/8 上午 10:46 */
	inDISP_BEEP(1, 0);
	
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
        inGetCustomIndicator(szCustomerIndicator);
	
	/* 存ECR原始資料 */
	memcpy(srECROb->srTransData.szOrgData, szDataBuffer, _ECR_7E1_Standard_Data_Size_);
	
	inECR_7E1_Standard_Parse_Data(srECROb, szDataBuffer);
	
	/* Trans Type (交易別) */
	switch (inTransType)
	{
		default:
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szTransType[0], 2);
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s: %s  ", "Trans Type", szTemplate);
				inLogPrintf(AT, szDebugMsg);
			}
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Type", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				sprintf(pobTran->szErrorMsgBuff1, "%02d", inTransType);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "Trans Type Not Exist Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
				return (VS_ERROR);
			}
			
			/* 交易類別轉成數字 */
			inTransType = atoi(srECROb->srTransData.szTransType);
			
			/* 035結帳為單結 */
			/* 要自動設定為NCCC */
			if (inTransType == _ECR_7E1_CUSTOMER_035_MIRAMAR_CINEMAS_SETTLEMENT_NO_)
			{
				inLoadHDTRec(0);
				pobTran->srBRec.inHDTIndex = 0;
				inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
				memset(szBatchNum, 0x00, sizeof(szBatchNum));
				inGetBatchNum(szBatchNum);
				pobTran->srBRec.lnBatchNum = atol(szBatchNum);
			}
			break;
	}
	
	/* 銀行別/付款別 HOST ID*/
	switch (inTransType)
	{
		case _ECR_7E1_VOID_NO_:				/* 取消交易 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szHostID[0], 2);
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "HOST ID", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "%s: %s  ", "HOST ID", "Not Exist Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
				return (VS_ERROR);
			}
			
			memset(szHostLabel, 0x00, sizeof(szHostLabel));

			if (memcmp(szTemplate, _ECR_7E1_NCCC_HOSTID_NCCC_, 2) == 0)
			{
				memcpy(szHostLabel, _HOST_NAME_CREDIT_NCCC_, strlen(_HOST_NAME_CREDIT_NCCC_));
			}
			else if (memcmp(szTemplate, _ECR_7E1_NCCC_HOSTID_DCC_, 2) == 0)
			{
				memcpy(szHostLabel, _HOST_NAME_DCC_, strlen(_HOST_NAME_DCC_));
			}
			/* 2018/5/8 下午 2:10 DFS需求不再使用大來主機 */
			else
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "無對應Host ID");
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				sprintf(pobTran->szErrorMsgBuff1, "%02d", inTransType);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "%s : HOST ID Not Pair", szTemplate);
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				return (VS_ERROR);
			}
			
			/* 初始化 */
			i = 0;
			do
			{
				/* 按順序load每一個HOST */
				if (inLoadHDTRec(i) < 0)
				{
					memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
					sprintf(pobTran->szErrorMsgBuff1, "%02d", inTransType);
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "%s : HOST ID Not Found", szHostLabel);
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
					/* 如果return VS_ERROR 代表table沒有該HOST */
					return (VS_ERROR);
				}

				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetHostLabel(szTemplate);
				if (!memcmp(szTemplate, szHostLabel, strlen(szHostLabel)))
				{
					/* 比對成功後，判斷HOST是否開啟  */
					memset(szHostEnable, 0x00, sizeof(szHostEnable));
					inGetHostEnable(szHostEnable);
					if (szHostEnable[0] != 'Y')
					{
						/* Host沒開 */
						if (ginDebug == VS_TRUE) 
						{
							memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
							sprintf(szDebugMsg, "%s Not Open Error", szHostLabel);
							inLogPrintf(AT, szDebugMsg);
						}
						memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
						sprintf(pobTran->szErrorMsgBuff1, "%02d", inTransType);
						memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
						sprintf(pobTran->szErrorMsgBuff2, "%s : HOST ID Not Open", szHostLabel);
						pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

						/* 如果return VS_ERROR 代表Host沒開 */
						return (VS_ERROR);
					}
					else
					{
						/* Host有開 */
						pobTran->srBRec.inHDTIndex = i;
						
						/* Load HDPT */
						inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
						memset(szBatchNum, 0x00, sizeof(szBatchNum));
						inGetBatchNum(szBatchNum);
						pobTran->srBRec.lnBatchNum = atol(szBatchNum);
						
						if (ginDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
							sprintf(szDebugMsg, "%s Open", szHostLabel);
							inLogPrintf(AT, szDebugMsg);
						}
						
					}/* Host Enable比對End */

					break;
				}/* Host Label比對 End */
				i++;

			} while (1);
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%d", "HDTindex", pobTran->srBRec.inHDTIndex);
				inLogPrintf(AT, szDebugMsg);
			}
			
			break;
			
		default:
			break;
	}
	
	/* EDC簽單序號 Receipt No(InvoiceNumber) */
	switch (inTransType)
	{
		case _ECR_7E1_VOID_NO_:				/* 取消交易 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &srECROb->srTransData.szReceiptNo[0], 6);
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Receipt No", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				sprintf(pobTran->szErrorMsgBuff1, "%02d", inTransType);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "Receipt No Not Exist Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
				return (VS_ERROR);
			}
			else
			{
				pobTran->srBRec.lnOrgInvNum = atol(szTemplate);
			}
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%ld", "lnOrgInvNum", pobTran->srBRec.lnOrgInvNum);
				inLogPrintf(AT, szDebugMsg);
			}
			
			break;
		default:
			break;
	}
		
	/* 處理交易金額 (Trans Amount) */
	switch (inTransType)
	{
		case _ECR_7E1_SALE_NO_:
		case _ECR_7E1_OFFLINE_NO_:
		case _ECR_7E1_REFUND_NO_:
		case _ECR_7E1_REDEEM_NO_:
		case _ECR_7E1_REDEEM_REFUND_NO_:
		case _ECR_7E1_PREAUTH_NO_:
                case _ECR_7E1_START_CARD_NO_INQUIRY_NO_:
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &srECROb->srTransData.szField_10[0], 10);
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Amount", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "Trans Amount Not Exist Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
				return (VS_ERROR);
			}
			else if ((!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_076_8_DIGITS_, _CUSTOMER_INDICATOR_SIZE_) && atol(szTemplate) > 99999999) || 
				 (memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_076_8_DIGITS_, _CUSTOMER_INDICATOR_SIZE_) && atol(szTemplate) > 9999999))
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Amount", "Out of bound");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_076_8_DIGITS_, _CUSTOMER_INDICATOR_SIZE_))
				{
					sprintf(pobTran->szErrorMsgBuff2, "金額超過8位數");
				}
				else
				{
					sprintf(pobTran->szErrorMsgBuff2, "金額超過7位數");
				}
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
				
				return (VS_ERROR);
			}
			else
			{
				pobTran->srBRec.lnTxnAmount = atol(szTemplate);
				pobTran->srBRec.lnOrgTxnAmount = atol(szTemplate);
				pobTran->srBRec.lnTotalTxnAmount = atol(szTemplate);
			}

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%ld", "交易金額", pobTran->srBRec.lnTxnAmount);
				inLogPrintf(AT, szDebugMsg);
			}
			break;
		default:
			break;
	}
	
	/* 交易時間 Trans Time */
	switch (inTransType)
	{
		default:
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szTransTime[0], 6);
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Time", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				
//				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
//				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
//				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
//				sprintf(pobTran->szErrorMsgBuff2, "Trans Time Not Exist Error");
//				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
//				
//				/* 有時候會不送時間 */
//				return (VS_ERROR);
			}
			else
			{
				sprintf(pobTran->srBRec.szTime, "%s", szTemplate);
				sprintf(pobTran->srBRec.szOrgTime, "%s", szTemplate);
			}
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "交易時間", pobTran->srBRec.szTime);
				inLogPrintf(AT, szDebugMsg);
			}
			
			break;
			
		
			break;
	}
	
	/* 授權碼(Auth Code) ECR電文中給9個字元(左靠右補空白) 但ATS電文只需要6個，所以只抓6個(交易補登及所有退貨須送此欄位) */
	switch (inTransType)
	{
		case _ECR_7E1_REFUND_NO_:			/* 退貨 */
		case _ECR_7E1_OFFLINE_NO_:			/* 交易補登 */
		case _ECR_7E1_REDEEM_REFUND_NO_:		/* 紅利退貨 */
		case _ECR_7E1_REDEEM_ADJUST_NO_:		/* 紅利調帳 */
			/* 處理授權碼(optional) */
			memset(pobTran->srBRec.szAuthCode, 0x00, sizeof(pobTran->srBRec.szAuthCode));
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szApprovalNo[0], 6);
			memcpy(pobTran->srBRec.szAuthCode, szTemplate, 6);
			
			break;
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Auth Code", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "Auth Code Not Exist Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
				return (VS_ERROR);
			}
			else
			{
				memcpy(pobTran->srBRec.szAuthCode, szTemplate, 6);
			}
			
			break;
			
		default :
			break;
	}
	
	/* szDataBuffer[66] 144規格要看紅利分期來看使用用途  */
	switch (inTransType)
	{
		case _ECR_7E1_REDEEM_REFUND_NO_:		/* 紅利退貨 */
		case _ECR_7E1_REDEEM_ADJUST_NO_:		/* 紅利調帳 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szRDMPaidAmt[0], 10);	/* 12位數，這裡只取10位 */
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "RDM Paid Amt", szTemplate);
				inLogPrintf(AT, szDebugMsg);
			}
			/* 實際支付金額 */
			pobTran->srBRec.lnRedemptionPaidCreditAmount = atol(szTemplate);
			
			break;	
		default :
			break;
	}
	
	/* szDataBuffer[102] 144規格要看紅利分期來看使用用途 保留欄位/首期金額／紅利扣抵點數  */
	switch (inTransType)
	{
		case _ECR_7E1_REDEEM_REFUND_NO_:		/* 紅利退貨 */
		case _ECR_7E1_REDEEM_ADJUST_NO_:		/* 紅利調帳 */
			/* 最多八位 所以從106開始 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szRDMPoint[4], 8);
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "RDM Point", szTemplate);
				inLogPrintf(AT, szDebugMsg);
			}
			
			/* 信用卡紅利扣抵點數 RDM Point */
			pobTran->srBRec.lnRedemptionPoints = atol(szTemplate);
			break;
		default :
			break;
	}
	
	
	
	/* 處理櫃號(Store ID) */
	switch (inTransType)
	{
		default:
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szStoreId[0], 18);
			
			if (strlen(szTemplate) > 0)
			{
				memcpy(pobTran->srBRec.szStoreID, szTemplate, strlen(szTemplate));
				/* 櫃號不滿18，補空白 */
				if (strlen(szTemplate) < 18)
				{
					memset(&pobTran->srBRec.szStoreID[strlen(szTemplate)], 0x20, 18 - strlen(szTemplate));
				}

			}
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "Store ID", pobTran->srBRec.szStoreID);
				inLogPrintf(AT, szDebugMsg);
			}
			break;
	}
	
	/* szDataBuffer[132] 144規格要看紅利分期來看使用用途  卡號查詢/每期金額/保留欄位 */
	
	/* 若前一次為卡號查詢，標記為第二次ECR */
	if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
	{
		pobTran->uszCardInquiryFirstBit = VS_FALSE;
		pobTran->uszCardInquirysSecondBit = VS_TRUE;
                
                /* 重新決定TRT流程 */
                if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
                {
                        /* 金融卡查詢要二次過卡，不依循前次查詢界面 */
                        inCardInquiryTRTType = _CARD_INQUIRY_TYPE_NONE_;
                }
                else
                {
                        if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
                        {
                                inCardInquiryTRTType = _CARD_INQUIRY_TYPE_CHIP_;
                        }
                        else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
                        {
                                inCardInquiryTRTType = _CARD_INQUIRY_TYPE_CTLS_;
                        }
                        else
                        {
                                inCardInquiryTRTType = _CARD_INQUIRY_TYPE_MEG_;
                        }
                }
	}
	
	/* szDataBuffer[142] 144規格要看紅利分期來看使用用途  卡號查詢/每期金額/保留欄位 */
	switch (inTransType)
	{
		/* 啟動卡號查詢 Start Trans Type */
		case _ECR_7E1_START_CARD_NO_INQUIRY_NO_:		/* 卡號查詢 */
			/* 卡號查詢 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &srECROb->srTransData.szStartTransType[0], 2);
			inTransType = atoi(szTemplate);
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "Start Trans Type", szTemplate);
				inLogPrintf(AT, szDebugMsg);
			}
			
			switch (inTransType)
			{
				case _ECR_7E1_SALE_NO_ :
				case _ECR_7E1_REFUND_NO_ :
				case _ECR_7E1_PREAUTH_NO_ :
					if (pobTran->uszCardInquirysSecondBit == VS_TRUE)
					{

					}
					else
					{
						pobTran->uszCardInquiryFirstBit = VS_TRUE;
					}
					/* 有值影響到後面判斷  確保都是空格 */
					memcpy(&srECROb->srTransData.szField_05[0], " ", 1);
					memcpy(&srECROb->srTransData.szTransTypeIndicator[0], " ", 1);
					break;
				case _ECR_7E1_OFFLINE_NO_ :
				case _ECR_7E1_REDEEM_NO_ :
				case _ECR_7E1_REDEEM_REFUND_NO_ :
				case _ECR_7E1_REDEEM_ADJUST_NO_ :
					if (pobTran->uszCardInquirysSecondBit == VS_TRUE)
					{

					}
					else
					{
						pobTran->uszCardInquiryFirstBit = VS_TRUE;
					}
					memcpy(&srECROb->srTransData.szField_05[0], " ", 1);
					memcpy(&srECROb->srTransData.szTransTypeIndicator[0], " ", 1);
					break;
				default :
					pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
					return (VS_ERROR);
			}
			break;
		default:
			break;
	}
	
	if (inECR_Check_Exception(pobTran) != VS_SUCCESS)
	{
		memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
		sprintf(pobTran->szErrorMsgBuff2, "ECR_Check_Exception error");
		return (VS_ERROR);
	}
	
	memset(szFiscContactlessEnable, 0x00, sizeof(szFiscContactlessEnable));
	inGetSmartPayContactlessEnable(szFiscContactlessEnable);
	memset(szCTLSEnable, 0x00, sizeof(szCTLSEnable));
	inGetContactlessEnable(szCTLSEnable);
	memset(szCUPContactlessEnable, 0x00, sizeof(szCUPContactlessEnable));
	inGetCUPContactlessEnable(szCUPContactlessEnable);
	
	/* 跑OPT */
	switch (inTransType)
	{
		case _ECR_7E1_SALE_NO_:
			/* 金融卡和一般Sale跑不同OPT，所以要分 */
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{	
				inDISP_ClearAll();
				/* 第三層顯示 ＜消費扣款＞ */
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_FISC_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜消費扣款＞ */

				pobTran->inFunctionID = _FISC_SALE_;
				if (!memcmp(szCTLSEnable, "Y", 1)		&&
				    !memcmp(szFiscContactlessEnable, "Y", 1)	&&
				    guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_FISC_SALE_CTLS_;
					pobTran->inRunTRTID = _TRT_FISC_SALE_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_FISC_SALE_;
					pobTran->inRunTRTID = _TRT_FISC_SALE_ICC_;
				}
		
				pobTran->inTransactionCode = _FISC_SALE_;
				pobTran->srBRec.inCode = _FISC_SALE_;
				pobTran->srBRec.inOrgCode = _FISC_SALE_;
                                
                                /* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_CUP_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜銀聯一般交易＞ */

				if (!memcmp(szCTLSEnable, "Y", 1)		&&
				    !memcmp(szCUPContactlessEnable, "Y", 1)	&&
				    guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
					pobTran->inRunTRTID = _TRT_CUP_SALE_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
					pobTran->inRunTRTID = _TRT_CUP_SALE_;
				}

				pobTran->inFunctionID = _CUP_SALE_;
				pobTran->inTransactionCode = _CUP_SALE_;
				pobTran->srBRec.inCode = pobTran->inTransactionCode;
				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
                                
                                /* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_TICKET_DEDUCT_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜購貨交易＞ */

				pobTran->inFunctionID = _TICKET_DEDUCT_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_TICKET_DEDUCT_;
				}
				else
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "感應未初始化，不能使用此功能");
					}
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "CTLS not Init");
					return (VS_ERROR);
				}

				pobTran->inTransactionCode = _TICKET_DEDUCT_;
				pobTran->srTRec.inCode = _TICKET_DEDUCT_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else
			{
				inDISP_ClearAll();
				/* 第三層顯示 ＜一般交易＞ */
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般交易＞ */

				pobTran->inFunctionID = _SALE_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
					pobTran->inRunTRTID = _TRT_SALE_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
					pobTran->inRunTRTID = _TRT_SALE_;
				}

				pobTran->inTransactionCode = _SALE_;
				pobTran->srBRec.inCode = _SALE_;
				pobTran->srBRec.inOrgCode = _SALE_;
                                
                                /* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			
			break;
		case _ECR_7E1_VOID_NO_:
			/* 金融卡和一般Sale跑不同OPT，所以要分 */
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_FISC_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜沖正交易＞ */

				pobTran->inFunctionID = _FISC_VOID_;
				pobTran->inRunOperationID = _OPERATION_VOID_;
				pobTran->inRunTRTID = _TRT_FISC_VOID_;

				pobTran->inTransactionCode = _FISC_VOID_;
				pobTran->srBRec.inCode = _FISC_VOID_;
				pobTran->srBRec.inOrgCode = _FISC_VOID_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜取消交易＞ */

				pobTran->inRunOperationID = _OPERATION_VOID_;
				pobTran->inRunTRTID = _TRT_CUP_VOID_;

				pobTran->inFunctionID = _CUP_VOID_;
				pobTran->inTransactionCode = _CUP_VOID_;
				pobTran->srBRec.inCode = pobTran->inTransactionCode;
				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜取消交易＞ */

				pobTran->inFunctionID = _VOID_;
				pobTran->inRunOperationID = _OPERATION_VOID_;
				pobTran->inRunTRTID = _TRT_VOID_;

				pobTran->inTransactionCode = _VOID_;
				pobTran->srBRec.inCode = _VOID_;
				pobTran->srBRec.inOrgCode = _VOID_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			break;
		case _ECR_7E1_REFUND_NO_:
			/* 金融卡和一般Sale跑不同OPT，所以要分 */
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_FISC_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);	/* 第二層顯示 ＜退費交易＞ */

				pobTran->inFunctionID = _FISC_REFUND_;                       
				if (!memcmp(szCTLSEnable, "Y", 1)		&&
				    !memcmp(szFiscContactlessEnable, "Y", 1)	&&
				    guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_FISC_REFUND_CTLS_;
					pobTran->inRunTRTID = _TRT_FISC_REFUND_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_FISC_REFUND_;
					pobTran->inRunTRTID = _TRT_FISC_REFUND_ICC_;
				}

				pobTran->inTransactionCode = _FISC_REFUND_;
				pobTran->srBRec.inCode = _FISC_REFUND_;
				pobTran->srBRec.inOrgCode = _FISC_REFUND_;
                                
                                /* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			else if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般退貨＞ */

				if (!memcmp(szCTLSEnable, "Y", 1)		&&
				    !memcmp(szCUPContactlessEnable, "Y", 1)	&&
				    guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_GET_CARD_FIRST_;
					pobTran->inRunTRTID = _TRT_CUP_REFUND_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_AMOUNT_FIRST_CUP_;
					pobTran->inRunTRTID = _TRT_CUP_REFUND_;
				}

				pobTran->inFunctionID = _CUP_REFUND_;
				pobTran->inTransactionCode = _CUP_REFUND_;
				pobTran->srBRec.inCode = pobTran->inTransactionCode;
				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
                                
                                /* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_TICKET_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜退貨交易＞ */

				pobTran->inFunctionID = _TICKET_REFUND_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_TICKET_REFUND_;
				}
				else
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "感應未初始化，不能使用此功能");
					}
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "CTLS not Init");
					return (VS_ERROR);
				}

				pobTran->inTransactionCode = _TICKET_REFUND_;
				pobTran->srTRec.inCode = _TICKET_REFUND_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般退貨＞ */

				pobTran->inFunctionID = _REFUND_;

				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_GET_CARD_FIRST_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_AMOUNT_FIRST_;
				}
				pobTran->inRunTRTID = _TRT_REFUND_;

				pobTran->inTransactionCode = _REFUND_;
				pobTran->srBRec.inCode = _REFUND_;
				pobTran->srBRec.inOrgCode = _REFUND_;
                                
                                /* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;
			
		case _ECR_7E1_OFFLINE_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_SALE_OFFLINE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜交易補登＞ */
			
			pobTran->inFunctionID = _SALE_OFFLINE_;
			pobTran->inRunOperationID = _OPERATION_SALE_OFFLINE_;
			pobTran->inRunTRTID = _TRT_SALE_OFFLINE_;

			pobTran->inTransactionCode = _SALE_OFFLINE_;
			pobTran->srBRec.inCode = _SALE_OFFLINE_;
			pobTran->srBRec.inOrgCode = _SALE_OFFLINE_;
                        
                        /* 卡號查詢不過第二次卡，所以要直接決定TRT */
                        if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
                        {
                                inNCCC_Func_Decide_CTLS_TRT(pobTran);
                        }
                        else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
                        {
                                inNCCC_Func_Decide_ICC_TRT(pobTran);
                        }
                        else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
                        {
                                inNCCC_Func_Decide_MEG_TRT(pobTran);
                        }
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			break;
			
		case _ECR_7E1_PREAUTH_NO_:
			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_PRE_AUTH_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜預先授權＞ */

				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_PRE_AUTH_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_PRE_AUTH_;
				}
				pobTran->inRunTRTID = _TRT_PRE_AUTH_;

				pobTran->inFunctionID = _CUP_PRE_AUTH_;
				pobTran->inTransactionCode = _CUP_PRE_AUTH_;
				pobTran->srBRec.inCode = pobTran->inTransactionCode;
				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
                                
                                /* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_PRE_AUTH_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜預先授權＞ */

				pobTran->inFunctionID = _PRE_AUTH_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_PRE_AUTH_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_PRE_AUTH_;
				}
				pobTran->inRunTRTID = _TRT_PRE_AUTH_;

				pobTran->inTransactionCode = _PRE_AUTH_;
				pobTran->srBRec.inCode = _PRE_AUTH_;
				pobTran->srBRec.inOrgCode = _PRE_AUTH_;
                                
                                /* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			
			break;
						
		case _ECR_7E1_REDEEM_NO_:
//			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
//			{
//				inDISP_ClearAll();
//				/* 保留給銀聯紅利扣抵使用 */
//				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
//				inDISP_PutGraphic(_MENU_REDEEM_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利扣抵＞ */
//
//				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
//				{
//					pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
//				}
//				else
//				{
//					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
//				}
//				pobTran->inRunTRTID = _TRT_REDEEM_SALE_;
//
//				pobTran->inFunctionID = _CUP_REDEEM_SALE_;
//				pobTran->inTransactionCode = _CUP_REDEEM_SALE_;
//				pobTran->srBRec.inCode = pobTran->inTransactionCode;
//				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
//
//				pobTran->srBRec.uszRedeemBit = VS_TRUE;
//                                /* 卡號查詢不過第二次卡，所以要直接決定TRT */
//				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
//				{
//					inNCCC_Func_Decide_CTLS_TRT(pobTran);
//				}
//				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
//				{
//					inNCCC_Func_Decide_ICC_TRT(pobTran);
//				}
//				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
//				{
//					inNCCC_Func_Decide_MEG_TRT(pobTran);
//				}
//
//				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
//			}
//			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REDEEM_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利扣抵＞ */

				pobTran->inFunctionID = _REDEEM_SALE_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
				}
				pobTran->inRunTRTID = _TRT_REDEEM_SALE_;

				pobTran->inTransactionCode = _REDEEM_SALE_;
				pobTran->srBRec.inCode = _REDEEM_SALE_;
				pobTran->srBRec.inOrgCode = _REDEEM_SALE_;

				pobTran->srBRec.uszRedeemBit = VS_TRUE;
                                /* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;
		
		case _ECR_7E1_REDEEM_REFUND_NO_:
//			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
//			{
//				inDISP_ClearAll();
//				/* 保留給銀聯紅利退貨使用 */
//				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
//				inDISP_PutGraphic(_MENU_REDEEM_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利退貨＞ */
//
//				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
//				{
//					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_GET_CARD_FIRST_;
//				}
//				else
//				{
//					pobTran->inRunOperationID = _OPERATION_REFUND_;
//				}
//				pobTran->inRunTRTID = _TRT_REDEEM_REFUND_;
//
//				pobTran->inFunctionID = _CUP_REDEEM_REFUND_;
//				pobTran->inTransactionCode = _CUP_REDEEM_REFUND_;
//				pobTran->srBRec.inCode = pobTran->inTransactionCode;
//				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
//
//				pobTran->srBRec.uszRedeemBit = VS_TRUE;
//                                /* 卡號查詢不過第二次卡，所以要直接決定TRT */
//				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
//				{
//					inNCCC_Func_Decide_CTLS_TRT(pobTran);
//				}
//				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
//				{
//					inNCCC_Func_Decide_ICC_TRT(pobTran);
//				}
//				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
//				{
//					inNCCC_Func_Decide_MEG_TRT(pobTran);
//				}
//
//				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
//			}
//			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REDEEM_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利退貨＞ */

				pobTran->inFunctionID = _REDEEM_REFUND_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_GET_CARD_FIRST_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_;
				}
				pobTran->inRunTRTID = _TRT_REDEEM_REFUND_;

				pobTran->inTransactionCode = _REDEEM_REFUND_;
				pobTran->srBRec.inCode = _REDEEM_REFUND_;
				pobTran->srBRec.inOrgCode = _REDEEM_REFUND_;

				pobTran->srBRec.uszRedeemBit = VS_TRUE;
                                /* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;
			
		case _ECR_7E1_REDEEM_ADJUST_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_REDEEM_ADJUST_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利調帳＞ */
			
			pobTran->inFunctionID = _REDEEM_ADJUST_;
			pobTran->inRunOperationID = _OPERATION_REDEEM_ADJUST_;
			pobTran->inRunTRTID = _TRT_REDEEM_ADJUST_;

			pobTran->inTransactionCode = _REDEEM_ADJUST_;
			pobTran->srBRec.inCode = _REDEEM_ADJUST_;
			pobTran->srBRec.inOrgCode = _REDEEM_ADJUST_;
			
			pobTran->srBRec.uszRedeemBit = VS_TRUE;
                        /* 卡號查詢不過第二次卡，所以要直接決定TRT */
                        if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
                        {
                                inNCCC_Func_Decide_CTLS_TRT(pobTran);
                        }
                        else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
                        {
                                inNCCC_Func_Decide_ICC_TRT(pobTran);
                        }
                        else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
                        {
                                inNCCC_Func_Decide_MEG_TRT(pobTran);
                        }
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
			
		case _ECR_7E1_CUSTOMER_035_MIRAMAR_CINEMAS_SETTLEMENT_NO_:
			inDISP_ClearAll();
			/* 第三層顯示 ＜結帳交易＞ */
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_SETTLE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜結帳交易＞ */

			pobTran->inFunctionID = _SETTLE_;
			pobTran->inRunOperationID = _OPERATION_SETTLE_;
			pobTran->inRunTRTID = _TRT_SETTLE_;
			
			pobTran->inTransactionCode = _SETTLE_;
			pobTran->srBRec.inCode = _SETTLE_;
			pobTran->srBRec.inOrgCode = _SETTLE_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		case _ECR_7E1_END_CARD_NO_INQUIRY_NO_:
			pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_END_INQUIRY_SUCCESS_;
			inRetVal = VS_ERROR;
			break;
		default:
			pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
			inRetVal = VS_ERROR;
			break;
	}
	
	inECR_ReCheck_144_TRT_Flow(pobTran);
	
	return (inRetVal);
}

/*
Function        :inECR_7E1_Customer_035_MIRAMAR_CINEMAS_Pack
Date&Time       :2022/7/14 下午 3:31
Describe        :移除分期和HappyGo相關條件
*/
int inECR_7E1_Customer_035_MIRAMAR_CINEMAS_Pack(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_SUCCESS;
	int 	i = 0, inCardLen = 0;
	int	inPacketSizes = 0;
	char 	szTemplate[100 + 1] = {0};
	ACCUM_TOTAL_REC srAccumRec;
	
	if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
	{
		strcpy(srECROb->srTransData.szField_05, "C");
	}
	
	if (!memcmp(&srECROb->srTransData.szTransType[0], _ECR_7E1_CUSTOMER_035_MIRAMAR_CINEMAS_SETTLEMENT_, 2))
	{
		/* Trans Type (2 Byte) */
		memcpy(&szDataBuffer[inPacketSizes], &srECROb->srTransData.szTransType[0], 2);
		inPacketSizes += 2;
		/* 開啟交易總的檔案 */
		memset(&srAccumRec, 0x00, sizeof(srAccumRec));
		inRetVal = inACCUM_GetRecord(pobTran, &srAccumRec);
		if (pobTran->lnNCCC_SettleTotalCount == 0)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			strcpy(szTemplate, "0000000000000");
			/* 銷售筆數 + 銷售金額 */
			memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 13);
			inPacketSizes += 13;
			/* 退貨筆數 + 退貨金額 */
			memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 13);
			inPacketSizes += 13;
			/* 總筆數 + 總金額 */
			memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 13);
			inPacketSizes += 13;
		}
		else
		{
			/* 銷售筆數 + 銷售金額 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%03lu", pobTran->lnNCCC_SettleSaleCount);
			memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 3);
			inPacketSizes += 3;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%010lld", pobTran->llNCCC_SettleSaleAmount);
			memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 10);
			inPacketSizes += 10;
			/* 退貨筆數 + 退貨金額 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%03lu", pobTran->lnNCCC_SettleRefundCount);
			memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 3);
			inPacketSizes += 3;
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%010lld", (0 - pobTran->llNCCC_SettleRefundAmount));
			memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 10);
			inPacketSizes += 10;
			/* 總筆數 + 總金額 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%03lu", pobTran->lnNCCC_SettleTotalCount);
			memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 3);
			inPacketSizes += 3;

			memset(szTemplate, 0x00, sizeof(szTemplate));

			if (srAccumRec.llTotalAmount >= 0L)
				sprintf(szTemplate, "%010lld", pobTran->llNCCC_SettleTotalAmount);
			else
				sprintf(szTemplate, "%010lld", (0 - pobTran->llNCCC_SettleTotalAmount));

			memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 10);
			inPacketSizes += 10;
		}

		inPacketSizes += 103;
	}
	else
	{
		
		/* Trans Type (2 Byte) */
		memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szTransType, 2);
		inPacketSizes += 2;

		/* HOST ID (2 Byte) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTRTFileName(szTemplate);

		if (!memcmp(szTemplate, _TRT_FILE_NAME_CREDIT_, strlen(_TRT_FILE_NAME_CREDIT_)))
			memcpy(&szDataBuffer[inPacketSizes], _ECR_7E1_NCCC_HOSTID_NCCC_, 2);
		/* 2018/5/8 下午 2:10 DFS需求不再使用大來主機 */
		else if (!memcmp(szTemplate, _TRT_FILE_NAME_DCC_, strlen(_TRT_FILE_NAME_DCC_)))
			memcpy(&szDataBuffer[inPacketSizes], _ECR_7E1_NCCC_HOSTID_DCC_, 2);

		inPacketSizes += 2;

		/* Receipt No (6 Byte) */
		if (pobTran->srBRec.lnOrgInvNum != 0L)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%06ld", pobTran->srBRec.lnOrgInvNum);
			memcpy(&szDataBuffer[inPacketSizes], szTemplate, 6);
		}

		inPacketSizes += 6;

		/* Card No (19 Byte)(左靠右補空白) */
		/* 卡號是否遮掩 */
		inRetVal = inECR_CardNoTruncateDecision(pobTran);

		/* 要遮卡號 */
		if (inRetVal == VS_SUCCESS)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			strcpy(szTemplate, pobTran->srBRec.szPAN);

			/* HAPPG_GO 卡不掩飾 */
			if (!memcmp(&pobTran->srBRec.szPAN[0], "9552", 4))
			{

			}
			else
			{
				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
				/* 卡號長度 */
				inCardLen = strlen(szTemplate);

				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
				for (i = 6; i < (inCardLen - 4); i ++)
					szTemplate[i] = '*';
			}

			memcpy(&szDataBuffer[inPacketSizes], szTemplate, strlen(szTemplate));
		}
		else
		{
			memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szPAN, strlen(pobTran->srBRec.szPAN));
		}

		inPacketSizes += 19;

		/* Card Expire Date (4 Byte) */
		/* 有效期是否回傳 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetECRExpDateReturnEnable(szTemplate);

		/* 要回傳有效期 */
		if (!memcmp(&szTemplate, "Y", 1))
			memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szExpDate, strlen(pobTran->srBRec.szExpDate));

		inPacketSizes += 4;

		/* Trans Amount (12 Byte) */
		inPacketSizes += 12;

		/* Trans Date (6 Byte) & Trans Time (6 Byte) */
		if (!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_SALE_, 2)		||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REFUND_, 2)		||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_VOID_, 2)		||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_OFFLINE_, 2)		||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REDEEM_, 2)		||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REDEEM_REFUND_, 2))
		{
			/* Trans Date */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			strcat(szTemplate, pobTran->srBRec.szDate);
			memcpy(&szDataBuffer[inPacketSizes], &szTemplate[2], 6);
			inPacketSizes += 6;

			/* Trans Time */
			memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szTime, 6);
			inPacketSizes += 6;
		}
		else
		{
			inPacketSizes += 6;
			inPacketSizes += 6;
		}

		/* Approval No (9 Byte) */
		if (strlen(pobTran->srBRec.szAuthCode) > 0)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			strcpy(szTemplate, pobTran->srBRec.szAuthCode);
			inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 9, _PADDING_RIGHT_);
			memcpy(&szDataBuffer[inPacketSizes], szTemplate, 9);
		}

		inPacketSizes += 9;

		/* 保留欄位/實際支付金額(紅利交易)/分期期數 + 分期手續費(分期交易) (12 Byte) */
		if (pobTran->srBRec.uszRedeemBit == VS_TRUE && pobTran->srBRec.uszVOIDBit != VS_TRUE)
		{
			/* 實際支付金額 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%010ld00", (pobTran->srBRec.lnRedemptionPaidCreditAmount + pobTran->srBRec.lnTipTxnAmount));
			memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);

			inPacketSizes += 12;
		}
		else
		{
			/* 收銀機交易送退貨，如過卡是感應方式，不要回傳W */
			if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
				memcpy(&szDataBuffer[inPacketSizes], "W", 1);

			inPacketSizes ++;

			if (!memcmp(&srECROb->srTransData.szField_05[0], "C", 1))
				memcpy(&szDataBuffer[inPacketSizes], &srECROb->srTransData.szField_05[0], 1);
			else
				memcpy(&szDataBuffer[inPacketSizes], "0", 1);

			inPacketSizes ++; /* CUP Indicator */
			inPacketSizes += 10;
		}

		/* ECR Response Code (4 Byte) */
		memcpy(&szDataBuffer[inPacketSizes], "0000", 4);
		inPacketSizes += 4;

		/* Terminal ID (8 Byte) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTerminalID(szTemplate);
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 8, _PADDING_RIGHT_);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 8);
		inPacketSizes += 8;

		/* Reference No */
		if (strlen(pobTran->srBRec.szRefNo) > 0)
		{
			memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szRefNo, 12);
		}
		inPacketSizes += 12;

		/* 保留欄位/扣抵紅利點數(紅利交易)/首期金額(分期交易) (12 Byte) */
		if (pobTran->srBRec.uszRedeemBit == VS_TRUE && pobTran->inTransactionCode != _VOID_)
		{
			/* 實際支付金額 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%012ld", pobTran->srBRec.lnRedemptionPoints);
			memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
		}
		inPacketSizes += 12;

		/* HGPAN(Request:Store Id Response:HappyGo Card Number) (18 Byte) */
		inPacketSizes += 18;

		/* 保留欄位(10 Byte)+ START Trans Type (2 Byte) /剩餘紅利點數(紅利交易)/每期金額(分期交易) (12 Byte) */
		/* 取消交易不回傳分期紅利資訊 */
		if (pobTran->srBRec.uszRedeemBit == VS_TRUE && pobTran->inTransactionCode != _VOID_)
		{
			/* 剩餘紅利點數(紅利交易) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%012ld", pobTran->srBRec.lnRedemptionPointsBalance);
			memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);

			inPacketSizes += 12;
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inECR_7E1_Customer_035_MIRAMAR_CINEMAS_Pack_ResponseCode
Date&Time       :2022/7/14 下午 4:26
Describe        :
*/
int inECR_7E1_Customer_035_MIRAMAR_CINEMAS_Pack_ResponseCode(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuf)
{
	/* 2011-04-19 PM 01:09:02 新增規格 端末機與收銀機連線規格書 Version 2.0  2.2.11.
		Status		Code		說明
		------------------------------------------------------
		Approved	0000		Success Transaction
		Error		0001		Failure Transaction
		Call Bank;
		Referral	0002		Failure Transaction
		Time Out	0003		Failure Transaction
	*/
	if (srECROb->srTransData.inErrorType == VS_CALLBANK	||
	    srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_CALLBANK_)
		memcpy(&szDataBuf[78], "0002", 4);
	else if (srECROb->srTransData.inErrorType == VS_TIMEOUT)
		memcpy(&szDataBuf[78], "0003", 4);
	else
	{
		memcpy(&szDataBuf[78], "0001", 4);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inECR_7E1_Customer_035_MIRAMAR_CINEMAS_Pack_Error
Date&Time       :2019/8/28 上午 10:45
Describe        :
*/
int inECR_7E1_Customer_035_MIRAMAR_CINEMAS_Pack_Error(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, ECR_TABLE * srECROb)
{
	int		inRetVal;
	int		i, inCardLen;
	int		inPacketSizes = 0;
	char		szTemplate[100 + 1];
	unsigned char	uszHostResponseBit = VS_FALSE;
	
	/* 有Response Code 代表主機有回 */
	if (strlen(pobTran->srBRec.szRespCode) > 0	||
	    strlen(pobTran->srTRec.szRespCode) > 0)
	{
		uszHostResponseBit = VS_TRUE;
	}

	/* Trans Type (2 Byte) */
	memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szTransType, 2);
	inPacketSizes += 2;

	/* HOST ID (2 Byte) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTRTFileName(szTemplate);

	if (!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_CUSTOMER_035_MIRAMAR_CINEMAS_SETTLEMENT_, 2))
	{
		
	}
	else
	{
		if (!memcmp(szTemplate, _TRT_FILE_NAME_CREDIT_, strlen(_TRT_FILE_NAME_CREDIT_)))
			memcpy(&szDataBuffer[inPacketSizes], _ECR_7E1_NCCC_HOSTID_NCCC_, 2);
		/* 2018/5/8 下午 2:10 DFS需求不再使用大來主機 */
		else if (!memcmp(szTemplate, _TRT_FILE_NAME_DCC_, strlen(_TRT_FILE_NAME_DCC_)))
			memcpy(&szDataBuffer[inPacketSizes], _ECR_7E1_NCCC_HOSTID_DCC_, 2);
	}

	inPacketSizes += 2;

	/* Receipt No (6 Byte) */
	if (pobTran->srBRec.lnOrgInvNum != 0L)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%06ld", pobTran->srBRec.lnOrgInvNum);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 6);
	}

	inPacketSizes += 6;

	/* Card No (19 Byte)(左靠右補空白) */
	/* 卡號是否遮掩 */
	inRetVal = inECR_CardNoTruncateDecision(pobTran);

	/* 要遮卡號 */
	if (inRetVal == VS_SUCCESS)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		strcpy(szTemplate, pobTran->srBRec.szPAN);

		/* HAPPG_GO 卡不掩飾 */
		if (!memcmp(&pobTran->srBRec.szPAN[0], "9552", 4))
		{

		}
		else
		{
			/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
			/* 卡號長度 */
			inCardLen = strlen(szTemplate);

			/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
			for (i = 6; i < (inCardLen - 4); i ++)
				szTemplate[i] = '*';
		}

		memcpy(&szDataBuffer[inPacketSizes], szTemplate, strlen(szTemplate));
	}
	else
	{
		memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szPAN, strlen(pobTran->srBRec.szPAN));
	}

	inPacketSizes += 19;

	/* Card Expire Date (4 Byte) */
	/* 有效期是否回傳 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetECRExpDateReturnEnable(szTemplate);

	/* 要回傳有效期 */
	if (!memcmp(&szTemplate, "Y", 1))
		memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szExpDate, strlen(pobTran->srBRec.szExpDate));

	inPacketSizes += 4;

	/* Trans Amount (12 Byte) */
	inPacketSizes += 12;

	/* Trans Date (6 Byte) & Trans Time (6 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_SALE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REFUND_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_VOID_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_OFFLINE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REDEEM_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REDEEM_REFUND_, 2))
	{
		/* Trans Date */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		strcat(szTemplate, pobTran->srBRec.szDate);
		memcpy(&szDataBuffer[inPacketSizes], &szTemplate[2], 6);
		inPacketSizes += 6;

		/* Trans Time */
		memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szTime, 6);
		inPacketSizes += 6;
	}
	else
	{
		inPacketSizes += 6;
		inPacketSizes += 6;
	}

	/* Approval No (9 Byte) */
	if (uszHostResponseBit == VS_TRUE)
	{
		if (strlen(pobTran->srBRec.szAuthCode) > 0)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			strcpy(szTemplate, pobTran->srBRec.szAuthCode);
			inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 9, _PADDING_RIGHT_);
			memcpy(&szDataBuffer[inPacketSizes], szTemplate, 9);
		}
	}
	else
	{
		
	}

	inPacketSizes += 9;
	
	/* 保留欄位/實際支付金額(紅利交易)/分期期數 + 分期手續費(分期交易) (12 Byte) */
	/* 取消交易不回傳分期紅利資訊 */
	if (pobTran->srBRec.uszRedeemBit == VS_TRUE && pobTran->inTransactionCode != _VOID_)
	{
		/* 實際支付金額 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010ld00", (pobTran->srBRec.lnRedemptionPaidCreditAmount + pobTran->srBRec.lnTipTxnAmount));
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
		
		inPacketSizes += 12;
	}
	else if (!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_CUSTOMER_035_MIRAMAR_CINEMAS_SETTLEMENT_, 2))
	{
		inPacketSizes += 12;
	}
	else
	{
		/* 收銀機交易送退貨，如過卡是感應方式，不要回傳W */
		if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
			memcpy(&szDataBuffer[inPacketSizes], "W", 1);
		
		inPacketSizes ++;
		
		if (!memcmp(&srECROb->srTransData.szField_05[0], "C", 1))
			memcpy(&szDataBuffer[inPacketSizes], &srECROb->srTransData.szField_05[0], 1);
		else
			memcpy(&szDataBuffer[inPacketSizes], "0", 1);

		inPacketSizes ++; /* CUP Indicator */
		inPacketSizes += 10;
	}
	
	/* ECR Response Code (4 Byte) */
	memcpy(&szDataBuffer[inPacketSizes], "0000", 4);
	inPacketSizes += 4;
	
	/* Terminal ID (8 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_CUSTOMER_035_MIRAMAR_CINEMAS_SETTLEMENT_, 2))
	{
		
	}
	else
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTerminalID(szTemplate);
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 8, _PADDING_RIGHT_);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 8);
	}
	inPacketSizes += 8;

	/* Reference No */
	if (strlen(pobTran->srBRec.szRefNo) > 0)
	{
		memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szRefNo, 12);
	}
	inPacketSizes += 12;
	
	/* 保留欄位/扣抵紅利點數(紅利交易)/首期金額(分期交易) (12 Byte) */
	/* 取消交易不回傳分期紅利資訊 */
	if (pobTran->srBRec.uszRedeemBit == VS_TRUE && pobTran->inTransactionCode != _VOID_)
	{
		/* 實際支付金額 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010ld00", pobTran->srBRec.lnRedemptionPoints);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
	}
	inPacketSizes += 12;
	
	/* HGPAN(Request:Store Id Response:HappyGo Card Number) (18 Byte) */
	inPacketSizes += 18;
	
	/* 保留欄位(10 Byte)+ START Trans Type (2 Byte) /剩餘紅利點數(紅利交易)/每期金額(分期交易) (12 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_START_CARD_NO_INQUIRY_, 1))
	{
		inPacketSizes += 10;
		
		/* START Trans Type (2 Byte) */
		memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szStartTransType, 2);
				
		inPacketSizes += 2;
	}
	/* 取消交易不回傳分期紅利資訊 */
	else if (pobTran->srBRec.uszRedeemBit == VS_TRUE && pobTran->inTransactionCode != _VOID_)
	{
		/* 剩餘紅利點數(紅利交易) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010ld00", pobTran->srBRec.lnRedemptionPointsBalance);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
		
		inPacketSizes += 12;
	}
	
	return (VS_SUCCESS);
}


/*
Function        :inECR_Receive_Cus_096
Date&Time       :2022/8/8 上午 10:05
Describe        :客製化096要同時兼容一卡通直連
*/
int inECR_Receive_Cus_096(TRANSACTION_OBJECT *pobTran, ECR_TABLE* srECROb, char *szDataBuffer, int inDataSize)
{
	int		i;
	int		inRetVal;
	int		inRetry = 0;							/* 目前已重試次數 */
	int		inRecMaxRetry = _ECR_RETRYTIMES_;				/* 最大重試次數 */
	char		szDebugMsg[1000 + 1];						/* DebugMsg */
	unsigned short	usReceiveBufferSize;						/* uszReceiveBuffer的長度，*/
	unsigned short	usOneSize = 1;							/* 一次只讀一個byte */
	unsigned short	usReceiveLen = 0;
	unsigned char	uszLRC;								/* LRC的值 */
	unsigned char	uszTempBuffer[1024 + 1];					/* 從 Buffer收到的片段 */
	unsigned char	uszReceiveBuffer[_ECR_BUFF_SIZE_];				/* 包含STX 和 ETX的原始電文 */
	unsigned char	uszSTX = VS_FALSE;						/* 是否收到STX */
	unsigned char	uszETX = VS_FALSE;						/* 是否收到ETX */
	unsigned char	uszCheckSpecialLenthBit = VS_FALSE;                             /* 是否已檢查為特殊長度 */
        unsigned char	uszCheckUnyBit = VS_FALSE;					/* 是否已檢查Indicator是否為Q */
        unsigned char	uszCheckEIBit = VS_FALSE;					/* 是否為電子發票 */
        char	szTemp[10 + 1];
	
	/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/28 下午 2:42 */
	gbBarCodeECRBit = VS_FALSE;
        gbEIECRBit = VS_FALSE;

	/* 設定Timeout */
	inRetVal = inDISP_Timer_Start(_ECR_RECEIVE_TIMER_, srECROb->srSetting.inTimeout);
	
	/* 初始化放收到資料的陣列 */
	memset(uszReceiveBuffer, 0x00, sizeof(uszReceiveBuffer));
	usReceiveBufferSize = 0;
	
	while(1)
	{
		memset(uszTempBuffer, 0x00, sizeof(uszTempBuffer));
		uszLRC = 0;
		
		/* 超過重試次數 */
		if (inRetry > inRecMaxRetry)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "Retry too many times!");
			}
			
			return (VS_ERROR);
		}
		
		/* 當Comport中有東西就開始分析 */
		while (usReceiveLen == 0)
		{
			/* 如果timeout就跳出去 */
			if (inTimerGet(_ECR_RECEIVE_TIMER_) == VS_SUCCESS)
			{
				return (VS_TIMEOUT);
			}
			
			inECR_Receive_Check(&usReceiveLen);
		}
		
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Len : %d", usReceiveLen);
			inLogPrintf(AT, szDebugMsg);
		}

		/* 如果沒收到STX就會進這隻，直到分析出STX */
		while (uszSTX == VS_FALSE && usReceiveLen > 0)
		{
			/* 如果timeout就跳出去 */
			if (inTimerGet(_ECR_RECEIVE_TIMER_) == VS_SUCCESS)
			{
				return (VS_TIMEOUT);
			}
			
			/* 這邊一次只收一個byte */
			usOneSize = 1;
			memset(uszTempBuffer, 0x00, usOneSize);
			inRetVal = inECR_Data_Receive(uszTempBuffer, &usOneSize);
			if (inRetVal == VS_SUCCESS)
			{	
				/* 若為為7E1協定，且收到STX之後的第一個為"I"，則自動轉為8N1 (NCCC ECR特殊規定) */
				if (inECR_NCCC_144_To_400(srECROb, uszTempBuffer) == VS_SUCCESS)
				{
					return (VS_ERROR);
				}
				
				/* buffer讀出一個byte，長度減一 */
				usReceiveLen --;

				if (uszTempBuffer[0] == _STX_)
				{
					/* 收到STX */
					uszSTX = VS_TRUE;
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "Receive STX!");
					}
					if (ginDisplayDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "Receive STX");
						inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_2_, VS_FALSE);
					}

					memcpy(&uszReceiveBuffer[usReceiveBufferSize], &uszTempBuffer[0], 1);
					usReceiveBufferSize++;
					
					break;
				}
				else
				{
					/* 沒收到STX */
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "Not Receive STX!");
						memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
						inFunc_BCD_to_ASCII(szDebugMsg, uszTempBuffer, usOneSize);
						inLogPrintf(AT, szDebugMsg);
					}
					if (ginDisplayDebug == VS_TRUE)
					{
						inDISP_LOGDisplay("Not Receive STX", _FONTSIZE_16X22_, _LINE_16_3_, VS_FALSE);
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						inFunc_BCD_to_ASCII(szDebugMsg, uszTempBuffer, usOneSize);
						inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_4_, VS_FALSE);
					}

					/* 繼續收 */
					continue;
				}

			}

		}

		/* 如果有收到STX，沒收到ETX就會進這隻，直到分析出ETX */			
		while (uszSTX == VS_TRUE && uszETX == VS_FALSE && usReceiveLen > 0)
		{
			/* 如果timeout就跳出去 */
			if (inTimerGet(_ECR_RECEIVE_TIMER_) == VS_SUCCESS)
			{
				return (VS_TIMEOUT);
			}
			
			usOneSize = usReceiveLen;
			memset(uszTempBuffer, 0x00, usOneSize);
			inRetVal = inECR_Data_Receive(uszTempBuffer, &usOneSize);

			if (inRetVal == VS_SUCCESS)
			{
				/* buffer讀出一個byte，長度減一 */
				memcpy(&uszReceiveBuffer[usReceiveBufferSize], uszTempBuffer, usOneSize);
				usReceiveLen -= usOneSize;
				usReceiveBufferSize += usOneSize;
				                                
                                if (usReceiveBufferSize >= 2)
				{ 
                                        /* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/8/13 下午 4:10 */
                                        if (uszCheckUnyBit != VS_TRUE && uszCheckSpecialLenthBit != VS_TRUE)
                                        {
                                                uszCheckUnyBit = VS_TRUE;
                                                
                                                if (uszReceiveBuffer[1] == 'Q')
                                                {
                                                        uszCheckSpecialLenthBit = VS_TRUE;
                                                        gbBarCodeECRBit = VS_TRUE;
                                                        inDataSize = _ECR_8N1_1000_CUPQRCODE_Size_;
                                                }
                                        }
                                }

                                if (usReceiveBufferSize >= 11)
				{
                                        if (uszCheckEIBit != VS_TRUE && uszCheckSpecialLenthBit != VS_TRUE)
                                        {
                                                uszCheckEIBit = VS_TRUE;

                                                /* 電子發票交易別 "63" */
                                                memset(szTemp, 0x00, sizeof(szTemp));
                                                memcpy(&szTemp[0], &uszReceiveBuffer[9], 2);
                                                if (!memcmp(szTemp, _ECR_8N1_EI_TRANSACTION_, 2))
                                                {
                                                        uszCheckSpecialLenthBit = VS_TRUE;
                                                        gbEIECRBit = VS_TRUE;
                                                        inDataSize = _ECR_8N1_2044_EI_Receive_Size_;
                                                }
                                        }
				}
				
				/* 當已收到401byte 確認下個byte是不是ETX */
				if (usReceiveBufferSize >= (inDataSize + 3) - 1)
				{
					if (uszReceiveBuffer[(inDataSize + 3) - 2] == _ETX_)
					{	
						/* 是ETX */
						uszETX = VS_TRUE;
						if (ginDebug == VS_TRUE)
						{
							inLogPrintf(AT, "Receive ETX!");
						}
						if (ginDisplayDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
							sprintf(szDebugMsg, "Receive ETX");
							inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_2_, VS_FALSE);
						}
						
						if (usReceiveBufferSize < (inDataSize + 3))
						{
							/* 再收LRC */
							usOneSize = 1;
							memset(uszTempBuffer, 0x00, usOneSize);
							inRetVal = inECR_Data_Receive(&uszTempBuffer[0], &usOneSize);
							if (inRetVal == VS_SUCCESS)
							{
								memcpy(&uszReceiveBuffer[usReceiveBufferSize], &uszTempBuffer[0], usOneSize);
								usReceiveLen -= usOneSize;
								usReceiveBufferSize += usOneSize;
							}
							else
							{
								/* 沒收到LRC 資料長度有誤*/
							}
						}
						
						break;
					}
					else
					{
						/* 不是是ETX */
						if (ginDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
							inFunc_BCD_to_ASCII(szDebugMsg, uszTempBuffer, usOneSize);
							inLogPrintf(AT, szDebugMsg);
							inLogPrintf(AT, "Not Receive ETX!");
						}
						if (ginDisplayDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
							sprintf(szDebugMsg, "Not Receive ETX");
							inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_3_, VS_TRUE);
						}
						
						/* 義大一卡通直連，長度為512 */
						/* 400 沒看到ETX，就改為找512有沒有ETX */
						if (inDataSize == _ECR_8N1_Standard_Data_Size_)
						{
							inDataSize = _ECR_IPASS_Receive_Size_;
						}
						else if (inDataSize == _ECR_7E1_Standard_Data_Size_)
						{
							inDataSize = _ECR_IPASS_Receive_Size_;
						}
					}	
				}
			}

		}

		/* 有收到STX、ETX而且收到長度也對，就開始算LRC */
		if (uszSTX == VS_TRUE && uszETX == VS_TRUE)
		{
			/* 資料長度錯誤，清Buffer，送NAK，重收 */
			if (usReceiveBufferSize < inDataSize + 3)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "DataLength error!:%d Retry: %d", usReceiveBufferSize, inRetry);
					inLogPrintf(AT, szDebugMsg);
				}
				if (ginDisplayDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "DataLength error!:%d Retry: %d", usReceiveBufferSize, inRetry);
					inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_3_, VS_TRUE);
				}
				
				/* 回傳NAK */
				inECR_Send_ACKorNAK(srECROb, _NAK_);
				/* retry次數+ 1 */
				inRetry ++;
				
				/* 初始化資料 */
				memset(uszReceiveBuffer, 0x00, sizeof(uszReceiveBuffer));
				usReceiveBufferSize = 0;
				uszSTX = VS_FALSE;
				uszETX = VS_FALSE;
				
				continue;
			}
			else if (usReceiveBufferSize > inDataSize + 3)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "DataLength :%d", usReceiveBufferSize);
					inLogPrintf(AT, szDebugMsg);
				}
				if (ginDisplayDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "DataLength :%d", usReceiveBufferSize);
					inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_3_, VS_TRUE);
				}
			}
			
			/* 計算收到DATA的LRC （Stx Not Include）*/
			for (i = 1; i < ((inDataSize + 3) - 1); i++)
			{
				uszLRC ^= uszReceiveBuffer[i];
			}
			
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "Calculate LRC: %02x", uszLRC);
			}
			
			if (uszReceiveBuffer[((inDataSize + 3) - 1)] == uszLRC)
			{
				if (srECROb->srOptionalSetting.uszNotSendAck == VS_TRUE)
				{
					
				}
				else
				{
					/* 比對收到的LRC是否正確，若正確回傳ACK */
					inECR_Send_ACKorNAK(srECROb, _ACK_);
				}
				
				break;
			}
			else
			{
				/* 比對失敗 */
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "LRC error! Retry: %d", inRetry);
					inLogPrintf(AT, szDebugMsg);
					
					inLogPrintf(AT, "CountLRC");
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%X", uszLRC);
					inLogPrintf(AT, szDebugMsg);
					
					inLogPrintf(AT, "DataSendLRC");
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(&szDebugMsg[0], "%X", uszReceiveBuffer[((inDataSize + 3) - 1)]);
					inLogPrintf(AT, szDebugMsg);
				}
				if (ginDisplayDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "LRC error! Retry: %d", inRetry);
					inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_3_, VS_TRUE);
				}
				
				/* 若錯誤回傳NAK */
				inECR_Send_ACKorNAK(srECROb, _NAK_);
				/* retry次數+ 1 */
				inRetry ++;
				
				/* 初始化資料 */
				memset(uszReceiveBuffer, 0x00, sizeof(uszReceiveBuffer));
				usReceiveBufferSize = 0;
				uszSTX = VS_FALSE;
				uszETX = VS_FALSE;
				
				continue;
			}
			
		}/* 算LRC END */
		
	}/* Receive END */
	
	/* 去除STX、ETX、LRC，把資料放到szrRealReceBuffer */
	memcpy(szDataBuffer, &uszReceiveBuffer[1], inDataSize);
	
	if (inDataSize == _ECR_IPASS_Receive_Size_)
	{
		pobTran->uszCus096_iPASS_Direct_Bit = VS_TRUE;
	}
	
	/* 列印紙本電文和顯示電文訊息 */
	inECR_Print_Receive_ISODeBug(szDataBuffer, usReceiveBufferSize - 3, inDataSize);
	
	return (VS_SUCCESS);
}

/*
Function        :inECR_Cus_096_Ipass_Parse_Data
Date&Time       :2022/8/8 上午 11:23
Describe        :
*/
int inECR_Cus_096_Ipass_Parse_Data(TRANSACTION_OBJECT *pobTran, ECR_TABLE* srECROb, char *szDataBuffer)
{
	int	inRetVal = VS_SUCCESS;
	
	/* Trans_Type */
	memcpy(srECROb->srTransData.srCus096_Data.szTransType, &szDataBuffer[0], 2);
	
	/* szResponseCode */
	memcpy(srECROb->srTransData.srCus096_Data.szResponseCode, &szDataBuffer[2], 4);
	
	/* szCommandDataLength */
	memcpy(srECROb->srTransData.srCus096_Data.szCommandDataLength, &szDataBuffer[6], 4);
	
	/* szAPIResponseCode */
	memcpy(srECROb->srTransData.srCus096_Data.szAPIResponseCode, &szDataBuffer[10], 2);
	
	/* szCommandData */
	memcpy(srECROb->srTransData.srCus096_Data.szCommandData, &szDataBuffer[12], 500);
	
	return (inRetVal);
}

/*
Function        :inECR_Cus_096_Ipass_Transaction_Flow
Date&Time       :2022/8/12 下午 1:17
Describe        :
*/
int inECR_Cus_096_Ipass_Transaction_Flow(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	char	szComPort[4 + 1];
        
        vdUtility_SYSFIN_LogMessage(AT, "inECR_Cus_096_Ipass_Transaction_Flow START!");
	
	/* 不是ECR連線就跳走 */
	if (pobTran->uszECRBit != VS_TRUE)
	{
		return (VS_SUCCESS);
	}
	
	/* UDP和RS232並存，要分開寫 */
	if (gbECR_UDP_TransBit == VS_TRUE)
	{
		inRetVal = VS_ERROR;
	}
	else
	{
		memset(szComPort, 0x00, sizeof(szComPort));
		inGetECRComPort(szComPort);
		if (memcmp(szComPort, _COMPORT_COM1_, strlen(_COMPORT_COM1_)) == 0	||
		    memcmp(szComPort, _COMPORT_COM2_, strlen(_COMPORT_COM2_)) == 0	||
		    memcmp(szComPort, _COMPORT_COM3_, strlen(_COMPORT_COM3_)) == 0	||
		    memcmp(szComPort, _COMPORT_COM4_, strlen(_COMPORT_COM4_)) == 0)
		{
			inRetVal = inRS232_ECR_Cus_096_Ipass_Trasaction_Transaction(pobTran);
		}
		else if (memcmp(szComPort, _COMPORT_USB1_, strlen(_COMPORT_USB1_)) == 0)
		{
			inRetVal = VS_ERROR;
		}
		else if (memcmp(szComPort, _COMPORT_USB2_, strlen(_COMPORT_USB2_)) == 0)
		{
			inRetVal = VS_ERROR;
		}
		else if (memcmp(szComPort, _COMPORT_WIFI_, strlen(_COMPORT_WIFI_)) == 0)
		{
			inRetVal = VS_ERROR;
		}
		else
		{
			inRetVal = VS_ERROR;
		}
	}
	
	return (inRetVal);
}

/*
Function        :inECR_ReCheck_144_TRT_Flow
Date&Time       :2022/8/25 下午 5:53
Describe        :ECR144長度無銀聯選項，須讀出交易紀錄後再重新設定TRT
*/
int inECR_ReCheck_144_TRT_Flow(TRANSACTION_OBJECT *pobTran)
{
	if (pobTran->inTransactionCode == _VOID_)
	{
		if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
		{
			pobTran->inRunTRTID = _TRT_CUP_VOID_;
			pobTran->inTransactionCode = _CUP_VOID_;
		}
		else if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
		{
			pobTran->inRunTRTID = _TRT_FISC_VOID_;
			pobTran->inTransactionCode = _FISC_VOID_;
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inECR_8N1_Standard_Mirror_Pack
Date&Time       :2025/10/17 下午 4:59
Describe        :先把要送的資料組好
*/
int inECR_8N1_Standard_Mirror_Pack(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer)
{
	int	inPacketSizes = 0;
        char    szBig5[200 + 1] = {0};
        char    szUTF8[300 + 1] = {0};

	/* ECR Indicator (1 Byte) */
	memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szECRIndicator, 1);
	inPacketSizes ++;
	/* ECR Version Date (6 Byte) */
	/* [114197] 中心收單作業ESG永續發展優化 
	 * [260612] 修改版本期 251120 */
	memcpy(&szDataBuffer[inPacketSizes], "251120", 6);
	inPacketSizes += 6;
	/* Reserve (1 Byte) */
	inPacketSizes ++;
	/* Trans Type (2 Byte) */
	memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_MIRROR_, 2);
	inPacketSizes += 2;
	
	/* [20251219_BUG_MDF][ECR] 信託鏡像回傳訊息時,需加入"T"的辦識字元 */
	szDataBuffer[inPacketSizes] = 'T';
	inPacketSizes ++;
	
        /* Mirror Message (200 Byte) */
	switch (pobTran->inMirrorMsgType)
	{
		case _MIRROR_MSG_8N1_Standard_OPER_ERR_ :
			/* 操作錯誤(0004) */
                        memset(szUTF8, 0x00, sizeof(szUTF8));
                        sprintf(szUTF8, "操作錯誤(0004)");
                        memset(szBig5, 0x00, sizeof(szBig5));
                        inFunc_UTF8toBig5(szBig5, szUTF8);
			memcpy(&szDataBuffer[inPacketSizes], szBig5, strlen(szBig5));
			break;
                case _MIRROR_MSG_8N1_Standard_PLS_ENTER_PATIENT_ID_ :
			/* 操作錯誤(0004) */
                        memset(szUTF8, 0x00, sizeof(szUTF8));
                        sprintf(szUTF8, "請於EDC輸入病患或入住者ID");
                        memset(szBig5, 0x00, sizeof(szBig5));
                        inFunc_UTF8toBig5(szBig5, szUTF8);
			memcpy(&szDataBuffer[inPacketSizes], szBig5, strlen(szBig5));
			break;
                case _MIRROR_MSG_8N1_Standard_NOT_ENTER_PATIENT_ID_ :
			/* 操作錯誤(0004) */
                        memset(szUTF8, 0x00, sizeof(szUTF8));
                        sprintf(szUTF8, "未帶病患或入住者ID");
                        memset(szBig5, 0x00, sizeof(szBig5));
                        inFunc_UTF8toBig5(szBig5, szUTF8);
			memcpy(&szDataBuffer[inPacketSizes], szBig5, strlen(szBig5));
			break;
		default :
			return (VS_ERROR);
			break;
	}
	
	inPacketSizes += 200;

	/* Reserve (190 Byte) */
	inPacketSizes += 190;
        
	return (VS_SUCCESS);
}

/*
Function        :inECR_8N1_Standard_Pack_Error
Date&Time       :2019/8/21 下午 6:17
Describe        :
*/
int inECR_8N1_Standard_Pack_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer)
{
	int		inRetVal = VS_ERROR;
	int		i = 0, inCardLen = 0;
	int		inPacketSizes = 0;
	int		inLen = 0;
	int		inBarCodeLen = 0;
	char		szTemplate[100 + 1];
	char		szTemplate2[100 + 1];
	char		szTemp[8 + 1] = {0};
	char		szTemp2[8 + 1] = {0};
	char		szHash[44 + 1];
	char		szFESMode[2 + 1];
	char		szCustomerIndicator[3 + 1] = {0};
	unsigned char	uszHostResponseBit = VS_FALSE;		/* 標示有沒有回 */
	unsigned char	uszHostResponseSuccessBit = VS_FALSE;	/* 標示是否成功 */
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
        inGetCustomIndicator(szCustomerIndicator);
	
	/* 沒有Response Code 代表主機沒回 */
	if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
	{
		if (pobTran->uszAutoTopUpSuccessBit == VS_TRUE &&
		    pobTran->uszDeductSuccessBit != VS_TRUE)
		{
			/* 自動加值成功但購貨失敗有可能沒回應碼(回應碼只紀錄購貨的) */
			uszHostResponseBit = VS_TRUE;
			uszHostResponseSuccessBit = VS_TRUE;
		}
		else
		{
			/* 有Response Code 代表主機有回 */
			if (strlen(pobTran->srTRec.szRespCode) > 0)
			{
				uszHostResponseBit = VS_TRUE;
				if (memcmp(pobTran->srTRec.szRespCode, _RESPONSE_CODE_APPROVAL_, _RESPONSE_CODE_APPROVAL_LEN_) == 0)
				{
					uszHostResponseSuccessBit = VS_TRUE;
				}
				else
				{
					uszHostResponseSuccessBit = VS_FALSE;
				}
			}
			else
			{
				uszHostResponseBit = VS_FALSE;
			}
		}
	}
	else
	{
		if (strlen(pobTran->srBRec.szRespCode) > 0)
		{
			uszHostResponseBit = VS_TRUE;
			if (memcmp(pobTran->srBRec.szRespCode, _RESPONSE_CODE_APPROVAL_, _RESPONSE_CODE_APPROVAL_LEN_) == 0	||
			    memcmp(pobTran->srBRec.szRespCode, _RESPONSE_CODE_CALL_BANK_, _RESPONSE_CODE_CALL_BANK_LEN_) == 0	||
			    memcmp(pobTran->srBRec.szRespCode, _RESPONSE_CODE_CALL_BANK_, _RESPONSE_CODE_CALL_BANK_LEN_) == 0)
			{
				uszHostResponseSuccessBit = VS_TRUE;
			}
			else
			{
				uszHostResponseSuccessBit = VS_FALSE;
			}
		}
		else
		{
			uszHostResponseBit = VS_FALSE;
		}
	}
	
	/* START */
	/* Auth Code */
	if (uszHostResponseBit == VS_TRUE)
	{
		if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			strcpy(szTemplate, pobTran->srTRec.szAuthCode);
			inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 6, _PADDING_RIGHT_);
			memset(srECROb->srTransData.szApprovalNo, 0x00, sizeof(srECROb->srTransData.szApprovalNo));
			memcpy(srECROb->srTransData.szApprovalNo, szTemplate, 6);
		}
		else
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			strcpy(szTemplate, pobTran->srBRec.szAuthCode);
			inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 6, _PADDING_RIGHT_);
			memset(srECROb->srTransData.szApprovalNo, 0x00, sizeof(srECROb->srTransData.szApprovalNo));
			memcpy(srECROb->srTransData.szApprovalNo, szTemplate, 6);
		}
	}
	else
	{
		/* 沒回塞9個空白 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 6, _PADDING_RIGHT_);
		memset(srECROb->srTransData.szApprovalNo, 0x00, sizeof(srECROb->srTransData.szApprovalNo));
		memcpy(srECROb->srTransData.szApprovalNo, szTemplate, 6);
	}
	
	/* Installment / Redeem Indictor (1 Byte) */
	memset(srECROb->srTransData.szField_20, 0x00, sizeof(srECROb->srTransData.szField_20));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 1, _PADDING_RIGHT_);
	memcpy(srECROb->srTransData.szField_20, szTemplate, 1);
	
	/* 處理紅利扣抵 */
	/* 如果主機未回傳，就帶空白 */
	/* RDM Paid Amt */
	memset(srECROb->srTransData.szRDMPaidAmt, 0x00, sizeof(srECROb->srTransData.szRDMPaidAmt));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 12, _PADDING_RIGHT_);
	memcpy(srECROb->srTransData.szRDMPaidAmt, szTemplate, 12);

	/* RDM Point */
	memset(srECROb->srTransData.szRDMPoint, 0x00, sizeof(srECROb->srTransData.szRDMPoint));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 8, _PADDING_RIGHT_);
	memcpy(srECROb->srTransData.szRDMPoint, szTemplate, 8);

	/* Points of Balance */
	memset(srECROb->srTransData.szPointsOfBalance, 0x00, sizeof(srECROb->srTransData.szPointsOfBalance));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 8, _PADDING_RIGHT_);
	memcpy(srECROb->srTransData.szPointsOfBalance, szTemplate, 8);

	/* Redeem Amt */
	memset(srECROb->srTransData.szRedeemAmt, 0x00, sizeof(srECROb->srTransData.szRedeemAmt));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 12, _PADDING_RIGHT_);
	memcpy(srECROb->srTransData.szRedeemAmt, szTemplate, 12);
	
	/* 票證金額 */
	if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
	{
		/* 交易前餘額 */
		memset(srECROb->srTransData.szField_26, 0x00, sizeof(srECROb->srTransData.szField_26));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_BALANCE_INQUIRY_, 2)	||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_START_CARD_NO_INQUIRY_, 2)	||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_END_CARD_NO_INQUIRY_, 2)		||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REPRINT_RECEIPT_, 2)		||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REVIEW_TOTAL_, 2)		||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REVIEW_DETAIL_, 2))
		{
			/* 查餘額不帶此欄位 */
			sprintf(szTemplate, "            ");
		}
		else
		{
			if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
			{
				if (pobTran->srTRec.lnFinalBeforeAmt > 100000)
				{
					sprintf(szTemplate, "-%09lu00", (pobTran->srTRec.lnFinalBeforeAmt - 100000));
				}
				else
				{
					/* 交易前餘額要帶未加值的金額 */
					if ((pobTran->srTRec.lnCardRemainAmount - pobTran->srTRec.lnTotalTopUpAmount) < 0)
						sprintf(szTemplate, "-%09lu00", (unsigned long)(0 - (pobTran->srTRec.lnCardRemainAmount - pobTran->srTRec.lnTotalTopUpAmount)));
					else
						sprintf(szTemplate, "+%09lu00", (unsigned long)pobTran->srTRec.lnCardRemainAmount - pobTran->srTRec.lnTotalTopUpAmount);
				}
			}
			else
			{
				if (pobTran->srTRec.lnFinalBeforeAmt < 0)
				{
					sprintf(szTemplate, "-%09lu00", (0 - pobTran->srTRec.lnFinalBeforeAmt));
				}
				else
				{
					sprintf(szTemplate, "+%09lu00", pobTran->srTRec.lnFinalBeforeAmt);
				}
			}

		}
		memcpy(srECROb->srTransData.szField_26, szTemplate, 12);

		/* 交易後餘額 */
		if (uszHostResponseBit == VS_TRUE	&&
		    uszHostResponseSuccessBit == VS_TRUE)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_START_CARD_NO_INQUIRY_, 2)	||
			    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_END_CARD_NO_INQUIRY_, 2)		||
			    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REPRINT_RECEIPT_, 2)		||
			    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REVIEW_TOTAL_, 2)		||
			    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REVIEW_DETAIL_, 2))
			{
				/* 啟動卡號查詢不帶此欄位、結束卡號查詢不帶此欄位 */
				sprintf(szTemplate, "            ");
			}
			else if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_BALANCE_INQUIRY_, 2))
			{
				if (pobTran->srTRec.lnCardRemainAmount < 0)
				{
					sprintf(szTemplate, "-%09lu00", (unsigned long)(0 - pobTran->srTRec.lnCardRemainAmount));
				}
				else
				{
					sprintf(szTemplate, "+%09lu00", (unsigned long)pobTran->srTRec.lnCardRemainAmount);
				}
			}
			else
			{
				if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
				{
					if (pobTran->srTRec.lnFinalAfterAmt > 100000)
						sprintf(szTemplate, "-%09lu00", (unsigned long)(pobTran->srTRec.lnFinalAfterAmt - 100000));
					else
						sprintf(szTemplate, "+%09lu00", (unsigned long)(pobTran->srTRec.lnFinalAfterAmt));
				}
				else
				{
					sprintf(szTemplate, "+%09lu00", (unsigned long)(pobTran->srTRec.lnFinalAfterAmt));
				}
			}
		}
		else
		{
			memset(srECROb->srTransData.szField_27, 0x00, sizeof(srECROb->srTransData.szField_27));
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 12, _PADDING_RIGHT_);
		}
		memcpy(srECROb->srTransData.szField_27, szTemplate, 12);

		/* 自動加值金額 */
		memset(srECROb->srTransData.szField_28, 0x00, sizeof(srECROb->srTransData.szField_28));
		if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2))
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			if (pobTran->srTRec.lnTotalTopUpAmount > 0L)
				sprintf(szTemplate, "+%09lu00", (unsigned long)(pobTran->srTRec.lnTotalTopUpAmount));
			else
				sprintf(szTemplate, "+%09lu00", (unsigned long)(0));
		}
		else
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 12, _PADDING_RIGHT_);
		}
		memcpy(srECROb->srTransData.szField_28, szTemplate, 12);
	}
	
	/* Batch No (6 Byte)和主機掛勾，所以要看主機有沒有回 */
	memset(srECROb->srTransData.szBatchNo, 0x00, sizeof(srECROb->srTransData.szBatchNo));
	if (uszHostResponseBit == VS_TRUE	&&
	    uszHostResponseSuccessBit == VS_TRUE)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%06ld", pobTran->srBRec.lnBatchNum);
	}
	else
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 6, _PADDING_RIGHT_);
	}
	memcpy(srECROb->srTransData.szBatchNo, szTemplate, 6);
	
	/* MP Flag(是否小額交易，MPAS小額交易用) (1 Byte) */
	/* 合併MPAS ECR到標準400 */
	memset(srECROb->srTransData.szMPFlag, 0x00, sizeof(srECROb->srTransData.szMPFlag));
	memset(szFESMode, 0x00, sizeof(szFESMode));
	inGetNCCCFESMode(szFESMode);
	if (uszHostResponseBit == VS_TRUE	&&
	    uszHostResponseSuccessBit == VS_TRUE)
	{
		if (memcmp(szFESMode, _NCCC_04_MPAS_MODE_, strlen(_NCCC_04_MPAS_MODE_)) == 0)
		{
			if (pobTran->srBRec.uszMPASTransBit == VS_TRUE)
				memcpy(szTemplate, "M", 1);
			else
				memcpy(szTemplate, " ", 1);
		}
		else
		{
			memcpy(szTemplate, " ", 1);
		}
	}
	else
	{
		memcpy(szTemplate, " ", 1);
	}
	memcpy(srECROb->srTransData.szMPFlag, szTemplate, 1);
	
	/* SP ISSUER ID */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(szTemplate, pobTran->srBRec.szFiscIssuerID, 8);
	memcpy(&srECROb->srTransData.szSPIssuerID[0], szTemplate, 8);
	
	/* SP / 信用卡/ESVC Origin Date */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	if (uszHostResponseBit == VS_TRUE	&&
	    uszHostResponseSuccessBit == VS_TRUE)
	{
		if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
		{
			/* ESVC Origin Date */
			/* 電票退貨要帶回原交易日期 */
			if (memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REFUND_, 2) == 0)
			{
				memset(szTemp, 0x00, sizeof(szTemp));
				memset(szTemp2, 0x00, sizeof(szTemp2));
				/* [20251219_BUG_MDF][ECC] 修改電票退貨的判斷日期,由 2位MM改為 4位 MMDD */
				memcpy(szTemp2, pobTran->srTRec.szTicketRefundDate, 4);

				inNCCC_TICKET_Decide_Year(szTemp, szTemp2);
				
				memcpy(szTemplate, szTemp, 4); 
				strcat(szTemplate, pobTran->srTRec.szTicketRefundDate);
				inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 8, _PADDING_RIGHT_);
			}
			else
			{
				inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 8, _PADDING_RIGHT_);
			}
		}
		else if (!memcmp(&srECROb->srTransData.szField_05[0], "S", 1) || pobTran->srBRec.uszFiscTransBit == VS_TRUE)
		{
			if (strlen(pobTran->srBRec.szFiscRefundDate) > 0)
			{
				memcpy(szTemplate, &pobTran->srBRec.szFiscRefundDate[0], 8);
			}
			else
			{
				memcpy(szTemplate, &pobTran->srBRec.szDate[0], 8);
			}
		}
		else
		{
			inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 8, _PADDING_RIGHT_);
		}
	}
	else
	{
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 8, _PADDING_RIGHT_);
	}
	memcpy(&srECROb->srTransData.szField_34[0], szTemplate, 8);
	
	/* SP Origin RRN / 信用卡OriginRRN / ATS電票交易序號 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	if (uszHostResponseBit == VS_TRUE	&&
	    uszHostResponseSuccessBit == VS_TRUE)
	{
		if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
		{
			/* 左靠右補空白 */
			memcpy(szTemplate, pobTran->srTRec.szTicketRefundCode, 6);
			inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 12, _PADDING_RIGHT_);
		}
		else if (!memcmp(&srECROb->srTransData.szField_05[0], "S", 1) || pobTran->srBRec.uszFiscTransBit == VS_TRUE)
		{
			inFunc_PAD_ASCII(szTemplate, pobTran->srBRec.szFiscRRN, ' ', 12, _PADDING_RIGHT_);
		}
		else
		{
			inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 12, _PADDING_RIGHT_);
		}
	}
	else
	{
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 12, _PADDING_RIGHT_);
	}
	memcpy(&srECROb->srTransData.szField_35[0], szTemplate, 12);
	
	/* Pay Item */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetPayItemEnable(szTemplate);
	if (memcmp(szTemplate, "Y", strlen("Y")) == 0	&&
	   (strlen(pobTran->srBRec.szPayItemCode) > 0))
	{
		/* Pay Item (5 Bytes) */
		/* ECR一段式收銀機連線 送空白payitem 不必回傳 */
		if (srECROb->srTransData.uszECRResponsePayitem == VS_FALSE)
		{
			inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 5, _PADDING_RIGHT_);
		}
		else
		{
			memcpy(szTemplate, &pobTran->srBRec.szPayItemCode[0], 5);
		}
	}
	else
	{
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 5, _PADDING_RIGHT_);
	}
	memcpy(&srECROb->srTransData.szPayItem, szTemplate, 5);
	
	/* Card No. Hash Value */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	if (uszHostResponseBit == VS_TRUE	&&
	    uszHostResponseSuccessBit == VS_TRUE)
	{
		if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
		{
			if (!memcmp(&srECROb->srTransData.szECRIndicator[0], "E", 1))
			{
				/* 兩段式ECR電票，第一段取不到真正的卡號，填空白 */
				if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
				{
					memset(szTemplate, 0x00, sizeof(szTemplate));
				}
				else if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_TOP_UP_, 2)		||
					 !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_VOID_TOP_UP_, 2)	||
					 !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_BALANCE_INQUIRY_, 2))
				{
					memset(szTemplate, 0x00, sizeof(szTemplate));
				}
				else
				{
					/* Card No. Hash Value */
					if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
					{
						strcpy(szTemplate, pobTran->srTRec.szUID);
					}
					else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
					{
						/* 【需求單-108240】EDC修改電票需求 悠遊卡指定要T0200 by Russell 2020/6/4 下午 2:02 */
						strcpy(szTemplate, pobTran->srTRec.srECCRec.szCardID);
					}
					else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
					{
						strcpy(szTemplate, pobTran->srTRec.szUID);	
					}
				}
			}
			else
			{
				if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_TOP_UP_, 2)		||
				    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_VOID_TOP_UP_, 2)	||
				    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_BALANCE_INQUIRY_, 2))
				{
					memset(szTemplate, 0x00, sizeof(szTemplate));
				}
				else
				{
					/* Card No. Hash Value */
					if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
					{
						strcpy(szTemplate, pobTran->srTRec.szUID);
					}
					else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
					{
						/* 【需求單-108240】EDC修改電票需求 悠遊卡指定要T0200 by Russell 2020/6/4 下午 2:02 */
						strcpy(szTemplate, pobTran->srTRec.srECCRec.szCardID);
					}
					else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
					{
						strcpy(szTemplate, pobTran->srTRec.szUID);	
					}
				}
			}
		}
		else
		{
                        if (!memcmp(&srECROb->srTransData.szECRIndicator[0], "Q", 1))
                        {
                                if (pobTran->uszCardInquiryFirstBit == VS_TRUE	||
                                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_END_CARD_NO_INQUIRY_, 2)	||
                                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_AWARD_REDEEM_, 2) ||
                                    pobTran->srBRec.uszUPlanECRBit == VS_TRUE)
                                {
                                        inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 50, _PADDING_RIGHT_);
                                }
                                else if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
                                {
                                        memcpy(&szTemplate[0], &pobTran->srBRec.szEIVI_BANKID[0], 6);
                                        memcpy(&szTemplate[6], &pobTran->srBRec.szEInvoiceHASH[0], 44);
                                }
                                else
                                {
                                        /* For example:原交易為400格式，用1000格式取消的狀況 */
                                }
                        }
			else if (!memcmp(&srECROb->srTransData.szECRIndicator[0], "E", 1))
			{
				/* 啟動卡號查詢只有在Indicator = "I" 才送Hash Value("E"一律送50個空白) */
				/* 因為卡號查詢會轉交易別，所以用Flag判斷 */
				if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
				{
					inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 50, _PADDING_RIGHT_);
				}
				else
				{
					if (strlen(pobTran->srBRec.szEIVI_BANKID) > 0)
					{
						if (!memcmp(&pobTran->srBRec.szEIVI_BANKID[0], "B00999", 6))
						{
							inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 50, _PADDING_RIGHT_);
						}
						else
						{
							memcpy(&szTemplate[0], &pobTran->srBRec.szEIVI_BANKID[0], 6);

							memset(szTemplate2, 0x00, sizeof(szTemplate2));
							if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_AWARD_REDEEM_, 2))
							{
								
							}
							else
							{
								memcpy(&szTemplate2[0], &pobTran->srBRec.szPAN[0], strlen(pobTran->srBRec.szPAN));
							}
							
							if (strlen(szTemplate2) > 0)
							{
								memset(szHash, 0x00, sizeof(szHash));
								inNCCC_Func_CardNumber_Hash(szTemplate2, szHash);
								memcpy(&szTemplate[6], &szHash[0], 44);
							}
						}
					}
					else
					{
						inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 50, _PADDING_RIGHT_);
					}
				}
			}
			/* 收銀機欄位ECR Indicator = “I” : Card No. Hash Value = Card number前6碼 + Hash Value 44碼(原規格) */
			else
			{
				memset(szTemplate2, 0x00, sizeof(szTemplate2));
				if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_AWARD_REDEEM_, 2))
				{
					
				}
				else
				{
					/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
					memcpy(&szTemplate2[0], &pobTran->srBRec.szPAN[0], strlen(pobTran->srBRec.szPAN));
				}

				if (strlen(szTemplate2) > 0)
				{
					memcpy(szTemplate, &szTemplate2[0], 6);

					memset(szHash, 0x00, sizeof(szHash));
					inNCCC_Func_CardNumber_Hash(szTemplate2, szHash);
					memcpy(&szTemplate[6], &szHash[0], 44);
				}
				else
				{
					inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 50, _PADDING_RIGHT_);
				}
			}
		}
	}
	else
	{
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 50, _PADDING_RIGHT_);
	}
	
	memcpy(&srECROb->srTransData.szCardNoHashValue, szTemplate, 50);
	
	/* MP Response Code */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	if (uszHostResponseBit == VS_TRUE	&&
	    uszHostResponseSuccessBit == VS_TRUE)
	{
		memset(szFESMode, 0x00, sizeof(szFESMode));
		inGetNCCCFESMode(szFESMode);
		if (memcmp(szFESMode, _NCCC_04_MPAS_MODE_, strlen(_NCCC_04_MPAS_MODE_)) == 0)
		{
			if (memcmp(pobTran->srBRec.szRespCode, "00", 2) != 0		&& 
			    memcmp(pobTran->srBRec.szRespCode, "11", 2) != 0		&&
			    pobTran->srBRec.uszMPASTransBit == VS_TRUE			&&
			    (memcmp(&pobTran->srBRec.szMPASAuthCode[0], "CAF", 3) == 0	||
			     memcmp(&pobTran->srBRec.szMPASAuthCode[0], "NEG", 3) == 0	||
			     memcmp(&pobTran->srBRec.szMPASAuthCode[0], "WEB", 3) == 0	|| 
			     memcmp(&pobTran->srBRec.szMPASAuthCode[0], "REJ", 3) == 0))
			{
				memcpy(szTemplate, &pobTran->srBRec.szMPASAuthCode[0], 6);
			}
			else
			{
				inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 6, _PADDING_RIGHT_);
			}
		}
		else
		{
			inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 6, _PADDING_RIGHT_);
		}
	}
	else
	{
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 6, _PADDING_RIGHT_);
	}
	memcpy(&srECROb->srTransData.szMPResponseCode, szTemplate, 6);
	
	/* ASM Award flag */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	if (uszHostResponseBit == VS_TRUE	&&
	    uszHostResponseSuccessBit == VS_TRUE)
	{
		if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
		{
			/* 【需求單-108215】電票交易支援優惠平台需求 by Russell 2020/5/11 下午 3:53 */
			if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2))
			{
				if ((pobTran->srTRec.uszRewardL1Bit == VS_TRUE	||
				     pobTran->srTRec.uszRewardL2Bit == VS_TRUE	||
				     pobTran->srTRec.uszRewardL5Bit == VS_TRUE)	&&
				     pobTran->srTRec.uszRewardSuspendBit != VS_TRUE)
				{
					memcpy(szTemplate, "A", 1);
				}
				else
				{
					memcpy(szTemplate, " ", 1);
				}
			}
			else
			{
				memcpy(szTemplate, " ", 1);
			}
		}
		else
		{
			if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2)	||
			    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_OFFLINE_, 2)	||
			    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_, 2)	||
			    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_, 2))
			{
				if ((pobTran->srBRec.uszRewardL1Bit == VS_TRUE	||
				     pobTran->srBRec.uszRewardL2Bit == VS_TRUE	||
				     pobTran->srBRec.uszRewardL5Bit == VS_TRUE)	&&
				     pobTran->srBRec.uszRewardSuspendBit != VS_TRUE)
				{
					memcpy(szTemplate, "A", 1);
				}
				else
				{
					memcpy(szTemplate, " ", 1);
				}
			}
			else
			{
				memcpy(szTemplate, " ", 1);
			}
		}
	}
	else
	{
		memcpy(szTemplate, " ", 1);
	}
	memcpy(&srECROb->srTransData.szASMAwardFlag, szTemplate, 1);
	
	/* MCP Indicator */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	if (uszHostResponseBit == VS_TRUE	&&
	    uszHostResponseSuccessBit == VS_TRUE)
	{
		if (strlen(pobTran->srBRec.szMCP_BANKID) > 0)
		{
			/* MCP Indicator (1 Bytes) */
			memcpy(szTemplate, &pobTran->srBRec.szMCP_BANKID[0], 1);
		}
		else
		{
			inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 1, _PADDING_RIGHT_);
		}
	}
	else
	{
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 1, _PADDING_RIGHT_);
	}
	memcpy(&srECROb->srTransData.szMCPIndicator, szTemplate, 1);
	
	/* 金融機構代碼 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	if (uszHostResponseBit == VS_TRUE	&&
	    uszHostResponseSuccessBit == VS_TRUE)
	{
		if (strlen(pobTran->srBRec.szMCP_BANKID) > 0)
		{
			/* 金融機構代碼 (3 Bytes) */
			memcpy(szTemplate, &pobTran->srBRec.szMCP_BANKID[1], 3);
		}
		else
		{
			inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 3, _PADDING_RIGHT_);
		}
	}
	else
	{
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 3, _PADDING_RIGHT_);
	}
	memcpy(&srECROb->srTransData.szIssuerBankID, szTemplate, 3);
	
	/* 遮掩的信用卡卡號 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	if (uszHostResponseBit == VS_TRUE	&&
	    uszHostResponseSuccessBit == VS_TRUE)
	{
		if (strlen(pobTran->srEWRec.szEW_EWIssuerID) > 0)
		{
			memcpy(szTemplate, pobTran->srEWRec.szEW_MaskCreditCardNo, 19);
		}
		else
		{
			inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 19, _PADDING_RIGHT_);
		}
	}
	else
	{
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 19, _PADDING_RIGHT_);
	}
	memcpy(&srECROb->srTransData.srEW_Data.szEW_MaskCreditCardNo, szTemplate, 19);
	
	/* 電子錢包業者 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	if (uszHostResponseBit == VS_TRUE	&&
	    uszHostResponseSuccessBit == VS_TRUE)
	{
		if (strlen(pobTran->srEWRec.szEW_EWIssuerID) > 0)
		{
			memcpy(szTemplate, pobTran->srEWRec.szEW_EWIssuerID, _EW_ISSUERID_LEN_);
		}
		else
		{
			inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', _EW_ISSUERID_LEN_, _PADDING_RIGHT_);
		}
	}
	else
	{
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', _EW_ISSUERID_LEN_, _PADDING_RIGHT_);
	}
	memcpy(&srECROb->srTransData.srEW_Data.szEW_EWIssuerID, szTemplate, _EW_ISSUERID_LEN_);
	
	/* 特店訂單編號 */
	memcpy(&srECROb->srTransData.srEW_Data.szEW_OrderId, pobTran->srEWRec.szEW_OrderId, 30);
		
	/* 電子發票載具號碼 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	if (uszHostResponseBit == VS_TRUE	&&
	    uszHostResponseSuccessBit == VS_TRUE)
	{
		if (strlen(pobTran->srEWRec.szEW_EinvoiceHASH) > 0)
		{
			/* 金融機構代碼 (3 Bytes) */
			memcpy(szTemplate, &pobTran->srEWRec.szEW_EinvoiceHASH, 50);
		}
		else
		{
			inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 50, _PADDING_RIGHT_);
		}
	}
	else
	{
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 50, _PADDING_RIGHT_);
	}
	memcpy(&srECROb->srTransData.srEW_Data.szEW_EinvoiceHASH, szTemplate, 50);
	
	/* 退貨訂單編號*/
	memcpy(&srECROb->srTransData.srEW_Data.szEW_RefundTradeNo, pobTran->srEWRec.szEW_RefundTradeNo, 30);
	
	/* 電子錢包業者交易序號 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	if (uszHostResponseBit == VS_TRUE	&&
	    uszHostResponseSuccessBit == VS_TRUE)
	{
		if (strlen(pobTran->srEWRec.szEW_TransactionId) > 0)
		{
			/* 金融機構代碼 (3 Bytes) */
			memcpy(szTemplate, &pobTran->srEWRec.szEW_TransactionId, 64);
		}
		else
		{
			inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 64, _PADDING_RIGHT_);
		}
	}
	else
	{
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 64, _PADDING_RIGHT_);
	}
	memcpy(&srECROb->srTransData.srEW_Data.szEW_TransactionId, szTemplate, 64);
	
	/*  END */
	
	/* ECR Indicator (1 Byte) */
	memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szECRIndicator, 1);
	inPacketSizes ++;
	
	/* ECR Version Date (6 Byte) */
	/* [114197] 中心收單作業ESG永續發展優化 
	 * [260612] 修改版本期 251120 */
	memcpy(&szDataBuffer[inPacketSizes], "251120", 6);
	inPacketSizes += 6;
	
	/* Trans Type Indicator (1 Byte) */
	inPacketSizes ++;
	
	/* Trans Type (2 Byte) */
	memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szTransType, 2);
	inPacketSizes += 2;
	
	/* CUP Indicator (1 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_START_CARD_NO_INQUIRY_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REFUND_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_CANCEL_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_CANCEL_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_TOP_UP_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_BALANCE_INQUIRY_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_VOID_TOP_UP_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_SALE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_CERTAIN_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_EW_INQUIRY_TRANSACTION_, 2))
	{
		if (!memcmp(srECROb->srTransData.szField_05, "C", 1) || pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			memcpy(&szDataBuffer[inPacketSizes], "C", 1);
		else if (!memcmp(srECROb->srTransData.szField_05, "S", 1) || pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			memcpy(&szDataBuffer[inPacketSizes], "S", 1);
		else if (!memcmp(srECROb->srTransData.szField_05, "E", 1) || pobTran->srTRec.uszESVCTransBit == VS_TRUE)
			memcpy(&szDataBuffer[inPacketSizes], "E", 1);
		else if (!memcmp(srECROb->srTransData.szField_05, _ECR_EW_INDICATOR_, 1) || pobTran->srEWRec.uszEWTransBit == VS_TRUE)
			memcpy(&szDataBuffer[inPacketSizes], _ECR_EW_INDICATOR_, 1);
		else if (!memcmp(srECROb->srTransData.szField_05, _ECR_TRUST_INDICATOR_, 1) || pobTran->srTrustRec.uszTrustTransBit == VS_TRUE)
			memcpy(&szDataBuffer[inPacketSizes], _ECR_TRUST_INDICATOR_, 1);
		else
		        memcpy(&szDataBuffer[inPacketSizes], "N", 1);
	}
	inPacketSizes ++;

	/* HOST ID (2 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REFUND_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_OFFLINE_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TIP_, 2)				||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_CANCEL_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_CANCEL_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_REFUND_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_ADJUST_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_REFUND_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_ADJUST_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_TOP_UP_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_VOID_TOP_UP_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_INQUIRY_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_SALE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_INSTALLMENT_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REDEMPTION_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_CERTAIN_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_FULL_REDEEMPTION_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REDEEM_REFUND_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REFUND_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_AWARD_REDEEM_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_AWARD_REDEEM_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_EW_INQUIRY_TRANSACTION_, 2))
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTRTFileName(szTemplate);
		
		if (!memcmp(szTemplate, _TRT_FILE_NAME_CREDIT_, strlen(_TRT_FILE_NAME_CREDIT_)))
			memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_HOSTID_NCCC_, 2);
		/* 2018/5/8 下午 2:10 DFS需求不再使用大來主機 */
		else if (!memcmp(szTemplate, _TRT_FILE_NAME_HG_, strlen(_TRT_FILE_NAME_HG_)))
			memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_HOSTID_HG_, 2);
		else if (!memcmp(szTemplate, _TRT_FILE_NAME_DCC_, strlen(_TRT_FILE_NAME_DCC_)))
			memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_HOSTID_DCC_, 2);
		else if (!memcmp(szTemplate, _TRT_FILE_NAME_ESVC_, strlen(_TRT_FILE_NAME_ESVC_)))
			memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_HOSTID_ESVC_, 2);
		else if (!memcmp(szTemplate, _TRT_FILE_NAME_EW_, strlen(_TRT_FILE_NAME_EW_)))
			memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_HOSTID_EW_, 2);
                else if (!memcmp(szTemplate, _TRT_FILE_NAME_TRUST_, strlen(_TRT_FILE_NAME_TRUST_)))
			memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_HOSTID_TRUST_, 2);
	}
	inPacketSizes += 2;

        if (pobTran->srTrustRec.uszTrustTransBit == VS_TRUE)
	{
		if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2)			||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_, 2))
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%s", pobTran->srTrustRec.szTrustRRN);
			memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
		}
                inPacketSizes += 12;
		
		/* Reserved */
		inPacketSizes += 17;
	}
	else
	{
                /* Receipt No (6 Byte) */
                if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2)			||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REFUND_, 2)			||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_, 2)			||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_OFFLINE_, 2)			||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TIP_, 2)				||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_, 2)			||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_CANCEL_, 2)		||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_, 2)		||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_CANCEL_, 2)	||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_, 2)			||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_REFUND_, 2)		||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_ADJUST_, 2)		||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_, 2)			||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_REFUND_, 2)		||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_ADJUST_, 2)		||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_TOP_UP_, 2)			||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_VOID_TOP_UP_, 2)		||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_INQUIRY_, 2)		||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_SALE_, 2)		||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_INSTALLMENT_, 2)	||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REDEMPTION_, 2)	||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2)	||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_CERTAIN_, 2)		||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_FULL_REDEEMPTION_, 2)		||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REDEEM_REFUND_, 2)		||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REFUND_, 2)		||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_AWARD_REDEEM_, 2)		||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_AWARD_REDEEM_, 2)		||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_EW_INQUIRY_TRANSACTION_, 2))
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
                        {
                                sprintf(szTemplate, "%06ld", pobTran->srTRec.lnInvNum);
                        }
                        else
                        {
                                sprintf(szTemplate, "%06ld", pobTran->srBRec.lnOrgInvNum);
                        }
                        memcpy(&szDataBuffer[inPacketSizes], szTemplate, 6);
                }

                inPacketSizes += 6;

                /* Card No (19 Byte) */
                if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_END_CARD_NO_INQUIRY_, 2)		||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SETTLEMENT_, 2)			||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REPRINT_RECEIPT_, 2)		||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REVIEW_DETAIL_, 2)		||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REVIEW_TOTAL_, 2)		||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REPORT_DETAIL_, 2)		||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REPORT_TOTAL_, 2)		||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_EDC_REBOOT_, 2)			||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_INQUIRY_, 2)		||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_FULL_REDEEMPTION_, 2)		||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REDEEM_REFUND_, 2)		||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REFUND_, 2)		||
                    (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_AWARD_REDEEM_, 2) && !(!memcmp(pobTran->szL3_AwardWay, "4", 1) || !memcmp(pobTran->szL3_AwardWay, "5", 1)))	||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_AWARD_REDEEM_, 2)		||
                    pobTran->srBRec.uszHappyGoSingle == VS_TRUE)
                {
                        /* 優惠兌換先設定不回傳卡號 */
                        inPacketSizes += 19;
                }
                else
                {
                        if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
                        {
                                memset(szTemplate, 0x00, sizeof(szTemplate));

                                /* 兩段式ECR電票，第一段取不到真正的卡號，填空白 */
                                if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
                                {
                                        if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
                                        {
                                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                                inLen = strlen(pobTran->srTRec.srECCRec.szCardID);
                                                memcpy(szTemplate, pobTran->srTRec.srECCRec.szCardID, inLen);
                                        }
                                        else
                                        {
                                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                                inLen = strlen(pobTran->srTRec.szUID);
                                                memcpy(szTemplate, pobTran->srTRec.szUID, inLen);
                                        }
                                }
                                else
                                {
                                        /* 一卡通：遮掩後2 碼，遮掩字元為”*” */
                                        if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
                                        {
                                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                                inLen = strlen(pobTran->srTRec.szUID);
                                                memcpy(szTemplate, pobTran->srTRec.szUID, inLen);
                                                szTemplate[inLen - 1] = 0x2A;
                                                szTemplate[inLen - 2] = 0x2A;
                                        }
                                        /*	1.簽單卡號列印邏輯
                                                <T4800>票卡版號(Purse Version Number)=0，<T0200>列印晶片卡號(Card Id)
                                                <T0200>20 bytes，14th,15th,16th隱碼，17th,18th,19th,20th顯示
                                                <T4800>票卡版號(Purse Version Number)<>0，<T0211>列印外觀卡號(Purse Id)
                                                <T0211>16 bytes，6th,7th,8th,9th,10th,11th隱碼，12th,13th,14th,15th,16th顯示
                                                ，遮掩字元為”*”
                                                以<T0200>悠遊卡卡號遮掩從後往前算第五碼，第六碼，第七碼遮掩
                                                範例:以8碼卡號為例，卡號11651733，1,6,5要遮掩，實際列印為1***1733
                                                1th 2th 3th 4th 5th 6th 7th 8th 9th 10th 11th 12th 13th 14th 15th 16th 17th
                                                0 0 0 0 0 0 0 0 0 1 1 6 5 1 7 3 3
                                                0 0 0 0 0 0 0 0 0 1 * * * 1 7 3 3
                                                範例:以10碼卡號為例，卡號6611651733，1,6,5要遮掩，實際列印為661***1733
                                                1th 2th 3th 4th 5th 6th 7th 8th 9th 10th 11th 12th 13th 14th 15th 16th 17th
                                                0 0 0 0 0 0 0 6 6 1 1 6 5 1 7 3 3
                                                0 0 0 0 0 0 0 6 6 1 * * * 1 7 3 3

                                                2. 電子發票載具卡號為T0200 (不遮掩)
                                        */
                                        else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
                                        {
                                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                                inLen = strlen(pobTran->srTRec.szUID);
                                                memcpy(szTemplate, pobTran->srTRec.szUID, inLen);

                                                if (atoi(pobTran->srTRec.srECCRec.szPurseVersionNumber) == 0)
                                                {
                                                        szTemplate[inLen - 5] = 0x2A;
                                                        szTemplate[inLen - 6] = 0x2A;
                                                        szTemplate[inLen - 7] = 0x2A;
                                                }
                                                else
                                                {
                                                        /* 全部16 第6-11隱碼 */
                                                        szTemplate[inLen - 6] = 0x2A;
                                                        szTemplate[inLen - 7] = 0x2A;
                                                        szTemplate[inLen - 8] = 0x2A;
                                                        szTemplate[inLen - 9] = 0x2A;
                                                        szTemplate[inLen - 10] = 0x2A;
                                                        szTemplate[inLen - 11] = 0x2A;
                                                }
                                        }
                                        else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
                                        {
                                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                                inLen = strlen(pobTran->srTRec.szUID);
                                                memcpy(szTemplate, pobTran->srTRec.szUID, inLen);
                                                /* 愛金卡：遮掩9-12 碼，遮掩字元為"*" */
                                                szTemplate[8] = 0x2A;
                                                szTemplate[9] = 0x2A;
                                                szTemplate[10] = 0x2A;
                                                szTemplate[11] = 0x2A;
                                        }
                                }

                                memcpy(&szDataBuffer[inPacketSizes], szTemplate, strlen(szTemplate));
                        }
                        /* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/3 下午 6:18 */
                        /* 不參考TMS遮掩開關 */
                        else if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
                        {
                                memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szPAN, strlen(pobTran->srBRec.szPAN));
                        }
                        else if (pobTran->srEWRec.uszEWTransBit == VS_TRUE)
                        {
                                if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2))
                                {
                                        /* 悠遊付：當消費者支付工具為”1：信用卡”時，遮掩的信用卡卡號 */
                                        /* 全支付：遮掩的信用卡卡號或帳戶 */
                                        memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.srEW_Data.szEW_MaskCreditCardNo, strlen(srECROb->srTransData.srEW_Data.szEW_MaskCreditCardNo));
                                }
                                else if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_EW_INQUIRY_TRANSACTION_, 2))
                                {
                                        /* 悠遊付：當消費者支付工具為”1：信用卡”時，遮掩的信用卡卡號 */
                                        /* icash Pay：卡號前六、後四碼，中間*號遮蔽 */
                                        /* 全支付：遮掩的信用卡卡號或帳戶 */
                                        memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.srEW_Data.szEW_MaskCreditCardNo, strlen(srECROb->srTransData.srEW_Data.szEW_MaskCreditCardNo));
                                }
                        }
                        else
                        {
                                /* 卡號是否遮掩 */
                                inRetVal = inECR_CardNoTruncateDecision(pobTran);
                                /* 要遮卡號 */
                                if (inRetVal == VS_SUCCESS)
                                {
                                        memset(szTemplate, 0x00, sizeof(szTemplate));
                                        strcpy(szTemplate, pobTran->srBRec.szPAN);

                                        /* HAPPG_GO 卡不掩飾 */
                                        if (!memcmp(&pobTran->srBRec.szPAN[0], "9552", 4))
                                        {

                                        }
                                        else
                                        {
                                                /* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
                                                /* 卡號長度 */
                                                inCardLen = strlen(szTemplate);
												/* [2600610][115028-客製化參數090前8後4明碼專案需求-台北101] 修改遮掩規則由 前6後4改為前8後4 */
												if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_090_TAIPEI_101_, _CUSTOMER_INDICATOR_SIZE_))
												{													
													for (i = 8; i < (inCardLen - 4); i ++)
														szTemplate[i] = '*';
													
												}else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_088_TINTIN_, _CUSTOMER_INDICATOR_SIZE_))
                                                {
                                                        /* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
                                                        for (i = 8; i < (inCardLen - 4); i ++)
                                                                szTemplate[i] = '*';
                                                }
                                                else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
                                                {
                                                        /* 第一段查詢交易，回覆信用卡前8後2卡號 */
                                                        if (pobTran->uszCardInquiryFirstBit == VS_TRUE ||
                                                            pobTran->uszCardInquirysSecondBit == VS_TRUE)
                                                        {
                                                                for (i = 8; i < (inCardLen - 2); i ++)
                                                                        szTemplate[i] = '*';
                                                        }
                                                        else
                                                        {
                                                                for (i = 6; i < (inCardLen - 4); i ++)
                                                                        szTemplate[i] = '*';
                                                        }
                                                }
                                                else
                                                {
                                                        /* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
                                                        for (i = 6; i < (inCardLen - 4); i ++)
                                                                szTemplate[i] = '*';
                                                }
                                        }
                                        memcpy(&szDataBuffer[inPacketSizes], szTemplate, strlen(szTemplate));
                                }
                                else
                                {
                                        memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szPAN, strlen(pobTran->srBRec.szPAN));
                                }
                        }
                        inPacketSizes += 19;
                }

                /* Card Expire Date (4 Byte) */
                if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_END_CARD_NO_INQUIRY_, 2)	||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SETTLEMENT_, 2)		||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REPRINT_RECEIPT_, 2)	||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REVIEW_DETAIL_, 2)	||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REVIEW_TOTAL_, 2)	||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REPORT_DETAIL_, 2)	||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REPORT_TOTAL_, 2)	||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_EDC_REBOOT_, 2)		||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_INQUIRY_, 2)	||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REDEEM_REFUND_, 2)	||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REFUND_, 2)	||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_FULL_REDEEMPTION_, 2))
                {
                        inPacketSizes += 4;
                }
                else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
                {
                        inPacketSizes += 4;
                }
                /* 電子錢包交易不回傳有效期 */
                /* (需求單-111155)-電子錢包業者收付訊息整合平台需求 by Russell 2023/7/24 下午 6:17 */
                else if (pobTran->srEWRec.uszEWTransBit == VS_TRUE)
                {
                        inPacketSizes += 4;
                }
                else
                {
                        /* 有效期是否回傳 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        inGetECRExpDateReturnEnable(szTemplate);

                        /* 要回傳有效期 */
                        if (!memcmp(&szTemplate, "Y", 1))
                                memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szExpDate, strlen(pobTran->srBRec.szExpDate));

                        inPacketSizes += 4;
                }
        }

	/* Trans Amount (12 Byte) */
	if (pobTran->srTrustRec.uszTrustTransBit == VS_TRUE)
	{
                /* 因為信託交易必定要回交易金額，所以必須擺在最上面 */
                if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2)			||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_, 2))
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "%010lu00", pobTran->srTrustRec.lnTxnAmount);
                        memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
		}

		inPacketSizes += 12;
	}
	else if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TIP_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_CANCEL_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_CANCEL_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_START_CARD_NO_INQUIRY_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_END_CARD_NO_INQUIRY_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SETTLEMENT_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REPRINT_RECEIPT_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REVIEW_DETAIL_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REVIEW_TOTAL_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REPORT_DETAIL_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REPORT_TOTAL_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_EDC_REBOOT_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_INQUIRY_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_FULL_REDEEMPTION_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REFUND_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REDEEM_REFUND_, 2))
	{
	        inPacketSizes += 12;
	}
        else if (pobTran->srBRec.uszUPlanTransBit == VS_TRUE)
        {
                if (pobTran->srBRec.uszVOIDBit == VS_TRUE)
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "%010lu00", pobTran->srBRec.lnTxnAmount);
                        memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
                }
                else
                {
                    if (pobTran->srBRec.lnCUPUPlanDiscountedAmount > 0)
                    {
                            memset(szTemplate, 0x00, sizeof(szTemplate));
                            sprintf(szTemplate, "%010lu00", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnCUPUPlanPreferentialAmount));
                            memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
                    }
                    else
                    {
                            memset(szTemplate, 0x00, sizeof(szTemplate));
                            sprintf(szTemplate, "%010lu00", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnCUPUPlanPreferentialAmount));
                            memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
                    }
                }
                inPacketSizes += 12;
        }
        else
        {
                if (pobTran->srTRec.lnTxnAmount != 0)
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "%010lu00", pobTran->srTRec.lnTxnAmount);
                        memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);

                        inPacketSizes += 12;
                }
                else if (pobTran->srBRec.lnTxnAmount != 0)
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "%010lu00", pobTran->srBRec.lnTxnAmount);
                        memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);

                        inPacketSizes += 12;
                }
                else if (pobTran->srEWRec.uszEWTransBit == VS_TRUE)
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "%010lu", pobTran->srEWRec.lnTxnAmount);
                        memcpy(&szDataBuffer[inPacketSizes], szTemplate, 10);
                        inPacketSizes += 10;

                        memcpy(&szDataBuffer[inPacketSizes], pobTran->srEWRec.szEW_Decimal, 2);
                        inPacketSizes += 2;
                }
                else
                {
                        inPacketSizes += 12;
                }
        }

	/* Trans Date (6 Byte) & Trans Time (6 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REFUND_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_OFFLINE_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TIP_, 2)				||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_CANCEL_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_CANCEL_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_REFUND_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_ADJUST_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_REFUND_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_ADJUST_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_TOP_UP_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_VOID_TOP_UP_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SETTLEMENT_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_INQUIRY_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_SALE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_INSTALLMENT_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REDEMPTION_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_CERTAIN_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_FULL_REDEEMPTION_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REDEEM_REFUND_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REFUND_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_AWARD_REDEEM_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_AWARD_REDEEM_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_EW_INQUIRY_TRANSACTION_, 2))
	{
		if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
		{
			/* Trans Date */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			if (strlen(pobTran->srTRec.szDate) > 0)
			{
				strcat(szTemplate, pobTran->srTRec.szDate);
			}
			else
			{
				memcpy(szTemplate, &pobTran->srBRec.szDate[2], 6);
			}
			memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 6);
			inPacketSizes += 6;

			/* Trans Time */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			if (strlen(pobTran->srTRec.szTime) > 0)
			{
				strcat(szTemplate, pobTran->srTRec.szTime);
			}
			else
			{
				strcat(szTemplate, pobTran->srBRec.szTime);
			}
			memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 6);
			inPacketSizes += 6;
		}
		else
		{
			/* Trans Date */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			strcat(szTemplate, pobTran->srBRec.szDate);
			memcpy(&szDataBuffer[inPacketSizes], &szTemplate[2], 6);
			inPacketSizes += 6;

			/* Trans Time */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			strcat(szTemplate, pobTran->srBRec.szTime);
			memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 6);
			inPacketSizes += 6;
		}
	}
	else
	{
		inPacketSizes += 6;
		inPacketSizes += 6;
	}

	/* Approval No (9 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REFUND_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_OFFLINE_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TIP_, 2)				||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_CANCEL_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_CANCEL_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_REFUND_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_ADJUST_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_REFUND_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_ADJUST_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_TOP_UP_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_BALANCE_INQUIRY_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_VOID_TOP_UP_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_SALE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_INSTALLMENT_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REDEMPTION_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_CERTAIN_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_FULL_REDEEMPTION_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REDEEM_REFUND_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REFUND_, 2))
	{
		/* 電子錢包交易不回傳授權碼 */
		/* (需求單-111155)-電子錢包業者收付訊息整合平台需求 by Russell 2023/7/24 下午 6:17 */
		if (pobTran->srEWRec.uszEWTransBit == VS_TRUE)
		{
			
		}
		else
		{
			memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szApprovalNo, 6);
		}
	}
	inPacketSizes += 9;

	/* Wave Card Indicator (1 Byte) */
	if (!memcmp(&srECROb->srTransData.szECRIndicator[0], "Q", 1))
	{
		/* 電子錢包交易不回傳Wave Card Indicator */
		/* (需求單-111155)-電子錢包業者收付訊息整合平台需求 by Russell 2023/7/24 下午 6:17 */
		/* Indicator為Q目前不送此欄位 */
	}
	else
	{
		if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
		{
			if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2)			||
			    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_START_CARD_NO_INQUIRY_, 2)	||
			    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_TOP_UP_, 2)			||
			    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_BALANCE_INQUIRY_, 2)	||
			    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_VOID_TOP_UP_, 2))
			{
				if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
				{
					memcpy(&szDataBuffer[inPacketSizes], "P", 1);
				}
				else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
				{
					memcpy(&szDataBuffer[inPacketSizes], "Z", 1);
				}
				else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
				{
					memcpy(&szDataBuffer[inPacketSizes], "G", 1);
				}
				else
				{
					/* 國隆說，有過卡且辨認不出才回O */
					if (strlen(pobTran->srTRec.szUID) > 0)
					{
						memcpy(&szDataBuffer[inPacketSizes], "O", 1);
					}
					else
					{
						memcpy(&szDataBuffer[inPacketSizes], " ", 1);
					}
				}
			}
		}
		else
		{
			if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2)			||
			    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_START_CARD_NO_INQUIRY_, 2)	||
			    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_TOP_UP_, 2)			||
			    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_BALANCE_INQUIRY_, 2)	||
			    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_VOID_TOP_UP_, 2))
			{
				if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
				{
					/* 規格未寫M 跟單機同步 */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					strcpy(szTemplate, pobTran->srBRec.szCardLabel);

					if (!memcmp(szTemplate, _CARD_TYPE_VISA_, strlen(_CARD_TYPE_VISA_)))
						memcpy(&szDataBuffer[inPacketSizes], "V", 1);
					else if (!memcmp(szTemplate, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)))
						memcpy(&szDataBuffer[inPacketSizes], "M", 1);
					else if (!memcmp(szTemplate, _CARD_TYPE_JCB_, strlen(_CARD_TYPE_JCB_)))
						memcpy(&szDataBuffer[inPacketSizes], "J", 1);
					else if (!memcmp(szTemplate, _CARD_TYPE_CUP_, strlen(_CARD_TYPE_CUP_)))
						memcpy(&szDataBuffer[inPacketSizes], "C", 1);
					else if (!memcmp(szTemplate, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)))
						memcpy(&szDataBuffer[inPacketSizes], "A", 1);
					/* (需求單 - 106349) 2018/10/31 下午 3:41 by Russell */
					else if (!memcmp(szTemplate, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)) ||
						 pobTran->srBRec.uszUCARDTransBit == VS_TRUE)
					{
						if (pobTran->srBRec.uszUCARDTransBit == VS_TRUE)
						{
							memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szMCP_BANKID[4], 1);
						}
						else
						{
							memcpy(&szDataBuffer[inPacketSizes], "D", 1);
						}
					}
					else
						memcpy(&szDataBuffer[inPacketSizes], "O", 1);
				}
			}
		}
	}
	inPacketSizes ++;

	/* ECR Response Code (4 Byte) */
	/* 其他地方會塞 */
	inPacketSizes += 4;

	/* Merchant ID (15 Byte) & Terminal ID (8 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REPORT_DETAIL_, 2) ||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REPORT_TOTAL_, 2) ||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SETTLEMENT_, 2))
	{
		inPacketSizes += 15;
		inPacketSizes += 8;
	}
	else
	{
		/* Merchant ID (15 Byte) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetMerchantID(szTemplate);
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 15, _PADDING_RIGHT_);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 15);
		inPacketSizes += 15;
		/* Terminal ID (8 Byte) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTerminalID(szTemplate);
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 8, _PADDING_RIGHT_);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 8);
		inPacketSizes += 8;
	}

	/* Exp Amount (12 Byte) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTRTFileName(szTemplate);
        if (!memcmp(szTemplate, _TRT_FILE_NAME_DCC_, _TRT_FILE_NAME_DCC_LEN_) && !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TIP_, 2))
        {
                if (!memcmp(&pobTran->srBRec.szDCC_FCMU[0], "0", 1))
                {
        	        memset(szTemplate, 0x00, sizeof(szTemplate));
        		sprintf(szTemplate, "%010ld00", atol(pobTran->srBRec.szDCC_TIPFCA));
        		memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 12);
        	}
                else if (!memcmp(&pobTran->srBRec.szDCC_FCMU[0], "1", 1))
                {
        	        memset(szTemplate, 0x00, sizeof(szTemplate));
        		sprintf(szTemplate, "%011ld0", atol(pobTran->srBRec.szDCC_TIPFCA));
        		memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 12);
        	}
        	else if (!memcmp(&pobTran->srBRec.szDCC_FCMU[0], "2", 1))
                {
        	        memset(szTemplate, 0x00, sizeof(szTemplate));
        		sprintf(szTemplate, "%012ld", atol(pobTran->srBRec.szDCC_TIPFCA));
        		memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 12);
        	}
        }
        else if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TIP_, 2))
        {
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010lu00", pobTran->srBRec.lnTipTxnAmount);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
	}

	inPacketSizes += 12;
	
	/* Store Id (18 Byte) */
	inPacketSizes += 18;

        if (pobTran->srTrustRec.uszTrustTransBit == VS_TRUE)
        {
                /* 信託 */
                /* 銷帳編號 */
                if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2)	||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_, 2))
                {
                        if (strlen(pobTran->srTrustRec.szReconciliationNo) > 0)
                        {
                                memcpy(&szDataBuffer[inPacketSizes], &pobTran->srTrustRec.szReconciliationNo[0], 20);
                        }
                }
                inPacketSizes += 20;
                /* 信託機構代碼 */
                if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2)	||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_, 2))
                {
                        memcpy(&szDataBuffer[inPacketSizes], &pobTran->srTrustRec.szTrustInstitutionCode[0], 7);
                }
                inPacketSizes += 7;
                /* 交換平台序號 */
                if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2)	||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_, 2))
                {
                        memcpy(&szDataBuffer[inPacketSizes], &pobTran->srTrustRec.szExchangeTxSerialNumber[0], 32);
                }
                inPacketSizes += 32;
                /* 主機回復中文說明 */
                if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2)	||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_, 2))
                {
                        memcpy(&szDataBuffer[inPacketSizes], &pobTran->szHostResponseMessageBIG5[0], 20);
                }
                inPacketSizes += 20;
                /* 信託原交易日期 */
                inPacketSizes += 4;
                /* 保留 */
                inPacketSizes += 4;
        }
        else
        {
                /* 處理紅利、分期 */
                /* Installment / Redeem Indictor (1 Byte) */
                if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_, 2)		||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_REFUND_, 2)	||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_ADJUST_, 2)	||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_, 2)		||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_REFUND_, 2)	||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_ADJUST_, 2)	||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REDEMPTION_, 2) ||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_INSTALLMENT_, 2))
                {
                        memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szField_20, 1);
                }
                /* 優惠兌換方式 */
                /* 1.條碼兌換 2.卡號兌換 */
                else if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_AWARD_REDEEM_, 2))
                {
                        memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szField_20, 1);
                }
                else if(!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_AWARD_REDEEM_, 2))
                {
                        /* (1= 條碼兌換， 2= 卡號兌換 )*/
                        /* 取消優惠兌換只接受 條碼兌換 。 */
                        memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szField_20, 1);
                }
                inPacketSizes ++;

                /* 處理紅利扣抵 */
                inPacketSizes += 40;

                /* 處理分期交易 */
                if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
                {
                        inPacketSizes += 2;

                        /* 交易前餘額 */
                        memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szField_26, 12);
                        inPacketSizes += 12;

                        /* 交易後餘額 */
                        memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szField_27, 12);
                        inPacketSizes += 12;

                        /* 自動加值金額 */
                        memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szField_28, 12);
                        inPacketSizes += 12;
                }
                else
                        inPacketSizes += 38;

                /* Card Type (2 Byte) */
                if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_START_CARD_NO_INQUIRY_, 2)					||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2)							||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REFUND_, 2)							||
                    (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_, 2) && pobTran->srBRec.uszHappyGoSingle != VS_TRUE)	||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_OFFLINE_, 2)							||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TIP_, 2)								||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_, 2)							||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_CANCEL_, 2)						||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_, 2)						||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_CANCEL_, 2)					||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_, 2)							||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_REFUND_, 2)						||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_ADJUST_, 2)						||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_, 2)							||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_REFUND_, 2)						||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_ADJUST_, 2)						||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_TOP_UP_, 2)							||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_BALANCE_INQUIRY_, 2)					||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_VOID_TOP_UP_, 2)						||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_INSTALLMENT_, 2)					||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REDEMPTION_, 2)					||
                    ((!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_SALE_, 2)						||
                      !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_CERTAIN_, 2)						||
                      !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2)) && pobTran->srBRec.uszHappyGoMulti == VS_TRUE)		||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_EW_INQUIRY_TRANSACTION_, 2))
                {
                        if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
                        {
                                if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
                                {
                                        memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_IPASS_, 2);
                                }
                                else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
                                {
                                        memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_ECC_, 2);
                                }
                                else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
                                {
                                        memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_ICASH_, 2);
                                }
                                else
                                        memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_UNKNOWN_, _ECR_8N1_NCCC_CARDTYPE_UNKNOWN_LEN_);
                        }
                        else if (pobTran->srEWRec.uszEWTransBit == VS_TRUE)
                        {
                                memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.srEW_Data.szEW_EWIssuerID, _EW_ISSUERID_LEN_);
                        }
                        else
                        {
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                strcpy(szTemplate, pobTran->srBRec.szCardLabel);

                                /* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
                                if (!memcmp(szTemplate, _CARD_TYPE_VISA_, strlen(_CARD_TYPE_VISA_)))
                                        memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_VISA_, 2);
                                else if (!memcmp(szTemplate, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)))
                                        memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_MASTERCARD_, 2);
                                else if (!memcmp(szTemplate, _CARD_TYPE_JCB_, strlen(_CARD_TYPE_JCB_)))
                                        memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_JCB_, 2);
                                else if (!memcmp(szTemplate, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)))
                                        memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_AMEX_, 2);
                                else if (!memcmp(szTemplate, _CARD_TYPE_CUP_, strlen(_CARD_TYPE_CUP_)))
                                        memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_CUP_, 2);
                                else if (!memcmp(szTemplate, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)) ||
                                         pobTran->srBRec.uszUCARDTransBit == VS_TRUE)
                                {
                                        if (pobTran->srBRec.uszUCARDTransBit == VS_TRUE)
                                        {
                                                memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_UCARD_, 2);
                                        }
                                        else
                                        {
                                                memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_DINERS_, 2);
                                        }
                                }
                                else if (!memcmp(szTemplate, _CARD_TYPE_SMARTPAY_, strlen(_CARD_TYPE_SMARTPAY_)))
                                        memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_SMARTPAY_, 2);
                                else
                                        memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_UNKNOWN_, _ECR_8N1_NCCC_CARDTYPE_UNKNOWN_LEN_);
                        }
                }

                inPacketSizes += 2;

                /* Batch No (6 Byte) */
                if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2)			||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REFUND_, 2)			||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_, 2)			||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_OFFLINE_, 2)			||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TIP_, 2)				||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_, 2)			||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_CANCEL_, 2)		||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_, 2)		||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_CANCEL_, 2)	||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_, 2)			||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_REFUND_, 2)		||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_ADJUST_, 2)		||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_, 2)			||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_REFUND_, 2)		||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_ADJUST_, 2)		||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_TOP_UP_, 2)			||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_VOID_TOP_UP_, 2)		||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_SALE_, 2)		||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_INSTALLMENT_, 2)	||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REDEMPTION_, 2)	||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2)	||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_CERTAIN_, 2)		||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_FULL_REDEEMPTION_, 2)		||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REDEEM_REFUND_, 2)		||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REFUND_, 2)		||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_EW_INQUIRY_TRANSACTION_, 2))
                {
                        memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szBatchNo, 6);
                }

                inPacketSizes += 6;
        }/* 和信託做區別 */

	/* (需求單-111155)-電子錢包業者收付訊息整合平台需求 by Russell 2023/7/24 下午 6:17 */
	if (pobTran->srEWRec.uszEWTransBit == VS_TRUE)
	{
		/* 特店訂單編號 */
		memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.srEW_Data.szEW_OrderId, 30);
		inPacketSizes += 30;
		
		/* 保留 */
		inPacketSizes += 6;
		
		/* 電子發票載具號碼 */
		if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2)			||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_EW_INQUIRY_TRANSACTION_, 2))
		{
			if (strlen(pobTran->srEWRec.szEW_EinvoiceHASH) > 0)
			{
				memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.srEW_Data.szEW_EinvoiceHASH, 50);
			}
		}
		inPacketSizes += 50;
		
		/* 退貨訂單編號*/
		if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REFUND_, 2))
		{
			memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.srEW_Data.szEW_RefundTradeNo, 30);
		}
		inPacketSizes += 30;
		
		/* 電子錢包業者交易序號 */
		memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.srEW_Data.szEW_TransactionId, 64);
		inPacketSizes += 64;
	}
	else
	{
		/* Start Trans Type (2 Byte) */
		if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_START_CARD_NO_INQUIRY_, 2))
			memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szStartTransType, 2);

		inPacketSizes += 2;

		/* MP Flag(是否小額交易，MPAS小額交易用) (1 Byte) */
		/* 合併MPAS ECR到標準400 */
		memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szMPFlag, 1);

		inPacketSizes += 1;

		if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_END_CARD_NO_INQUIRY_, 2)	||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SETTLEMENT_, 2)		||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REPRINT_RECEIPT_, 2)	||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REVIEW_DETAIL_, 2)	||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REVIEW_TOTAL_, 2)	||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REPORT_DETAIL_, 2)	||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REPORT_TOTAL_, 2)	||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_EDC_REBOOT_, 2))
		{
			/* 共99Bytes */
			inPacketSizes += 8;
			inPacketSizes += 8;
			inPacketSizes += 12;
			inPacketSizes += 5;
			inPacketSizes += 50;
			inPacketSizes += 6;
			inPacketSizes += 1;
			inPacketSizes += 1;
			inPacketSizes += 3;
			inPacketSizes += 5;
		}
		else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
		{
			inPacketSizes += 8;

			/* ESVC Origin Date */
			/* 電票退貨要帶回原交易日期 */
			memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szField_34, 8);
			inPacketSizes += 8;

			/* RF序號 */
			memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szField_35, 6);
			inPacketSizes += 12;

			/* Pay Item (5 Bytes) */
			memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szPayItemCode[0], 5);
			inPacketSizes += 5;

			/* Card No. Hash Value (50 Bytes) */
			memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szCardNoHashValue, 50);
			inPacketSizes += 50;

			/* MP Response Code (6 Bytes) */
			memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szMPResponseCode, 6);
			inPacketSizes += 6;

			/* ASM award flag */
			memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szASMAwardFlag, 1);
			inPacketSizes += 1;

			/* MCP Indicator (1 Bytes) */
			memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szMCPIndicator, 1);
			inPacketSizes += 1;

			/* 金融機構代碼 (3 Bytes) */
			memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szIssuerBankID, 3);
			inPacketSizes += 3;

			/* Reserve (5 Bytes) */
			inPacketSizes += 5;
		}
		else
		{
			/* SmartPay要回傳這三項資訊 (99 Bytes) */
			if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
			{
				inPacketSizes += 28;
			}
			else
			{
				/* SP ISSUER ID (8 Byte) */
				memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szSPIssuerID, 8);
				inPacketSizes += 8;
				/* SP Origin Date (8 Byte) */
				memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szField_34, 8);
				inPacketSizes += 8;
				/* SP RRN (12 Byte) */
				memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szField_35, 12);
				inPacketSizes += 12;
			}
			
			/* Pay Item (5 Bytes) */
			memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szPayItemCode[0], 5);
			inPacketSizes += 5;

			/* 【需求單 - 105039】信用卡為電子發票載具，端末機將卡號加密後回傳收銀機 add by LingHsiung 2016-04-20 上午 09:56:24 */
			/* 【需求單 - 108046】電子發票BIN大於6碼需求 by Russell 2019/7/8 上午 11:44 */
			/* 收銀機欄位ECR Indicator = “E” : 
			 * (1)銀行代碼為 ”B00999” : Card No. Hash Value = 空白50碼
			 * (2)其他 :Card No. Hash Value = 銀行代碼6碼 + Hash Value 44碼。
			*/
			/* Card No. Hash Value (50 Bytes) */
			/* Smartpay 和 DCC 因為不回傳"NI"，所以不回傳hash value */
			/* 只要沒回傳Table "NI" 一律回50個空白 */
			memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szCardNoHashValue, 50);
			inPacketSizes += 50;

			/* MP Response Code (6 Bytes) */
			memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szMPResponseCode, 6);
			inPacketSizes += 6;

			/* ASM award flag */
			memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szASMAwardFlag, 1);
			inPacketSizes += 1;

			/* 【需求單 - 106128】行動支付標記及金融機構代碼 add by LingHsiung 2017-08-23 上午 11:48:57 */
			/* MCP Indicator (1 Bytes) */
			memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szMCPIndicator, 1);
			inPacketSizes += 1;

			/* 金融機構代碼 (3 Bytes) */
			memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szIssuerBankID, 3);
			inPacketSizes += 3;

			/* Reserve (5 Bytes) */
			inPacketSizes += 5;
		}

		/* HG Data (78 Byte) */
		if (uszHostResponseBit == VS_TRUE	&&
		    uszHostResponseSuccessBit == VS_TRUE)
		{
			if (pobTran->srBRec.lnHGTransactionType != 0	&&
			    (pobTran->srBRec.uszHappyGoMulti == VS_TRUE || pobTran->srBRec.uszHappyGoSingle == VS_TRUE))
			{
				/* Payment Tools (2 Byte) */
				if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2) ||
				    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_CERTAIN_, 2))
				{
					if (pobTran->srBRec.lnHGPaymentType == _HG_PAY_CREDIT_)
						memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_HG_PAYMENT_TOOL_TYPE_CREDIT_CARD_, 2);
					else if (pobTran->srBRec.lnHGPaymentType == _HG_PAY_CASH_)
						memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_HG_PAYMENT_TOOL_TYPE_CASH_, 2);
					else if (pobTran->srBRec.lnHGPaymentType == _HG_PAY_GIFT_PAPER_)
						memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_HG_PAYMENT_TOOL_TYPE_GIFT_PAPER_, 2);
					else if (pobTran->srBRec.lnHGPaymentType == _HG_PAY_CREDIT_INSIDE_)
						memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_HG_PAYMENT_TOOL_TYPE_HGI_, 2);
					else if (pobTran->srBRec.lnHGPaymentType == _HG_PAY_CUP_)
						memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_HG_PAYMENT_TOOL_TYPE_CUP_CARD_, 2);
				}

				inPacketSizes += 2;

				/* HG Card Number (18 Byte) */
				/* 重印HG混信用卡的交易簽單不回傳HG卡號 */
				if (memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REPRINT_RECEIPT_, 2) != 0)
				{
					inRetVal = inECR_CardNoTruncateDecision_HG(pobTran);
					/* 要遮卡號 */
					if (inRetVal == VS_SUCCESS)
					{
						memset(szTemplate, 0x00, sizeof(szTemplate));
						strcpy(szTemplate, pobTran->srBRec.szHGPAN);

						/* HAPPG_GO 卡不掩飾 */
						if (!memcmp(&pobTran->srBRec.szHGPAN[0], "9552", 4))
						{

						}
						else
						{
							/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
							/* 卡號長度 */
							inCardLen = strlen(szTemplate);

							/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
							for (i = 6; i < (inCardLen - 4); i ++)
								szTemplate[i] = '*';
						}

						memcpy(&szDataBuffer[inPacketSizes], szTemplate, strlen(szTemplate));
					}
					else
					{
						memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szHGPAN, strlen(pobTran->srBRec.szHGPAN));
					}

				}

				inPacketSizes += 18;

				/* HG Pay Amount (12 Byte) 實際支付金額 */
				if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2))
				{
					memset(szTemplate, 0x00, sizeof(szTemplate));
					sprintf(szTemplate, "%010lu00", pobTran->srBRec.lnHGAmount);
					memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 12);
				}

				inPacketSizes += 12;
				/* HG Redeem Amount (12 Byte) 扣抵金額 */
				if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2))
				{
					memset(szTemplate, 0x00, sizeof(szTemplate));
					sprintf(szTemplate, "%010lu00", pobTran->srBRec.lnHGRedeemAmount);
					memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 12);
				}

				inPacketSizes += 12;
				/* HG Redeem Point (8 Byte) 扣抵點數 */
				if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2)	||
				    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_CERTAIN_, 2)		||
				    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_FULL_REDEEMPTION_, 2)		||
				    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REDEEM_REFUND_, 2)		||
				    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REFUND_, 2))
				{
					memset(szTemplate, 0x00, sizeof(szTemplate));
					sprintf(szTemplate, "%08lu", pobTran->srBRec.lnHGTransactionPoint);
					memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 8);
				}

				inPacketSizes += 8;
				/* HG Lack Point (8 Byte) 不足點數*/
				if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REFUND_, 2))
				{
					memset(szTemplate, 0x00, sizeof(szTemplate));
					sprintf(szTemplate, "%08lu", pobTran->srBRec.lnHGRefundLackPoint);
					memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 8);
				}

				inPacketSizes += 8;
				/* HG Balance Point (8 Byte) 剩餘點數 */
				if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2) ||
				    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_CERTAIN_, 2) ||
				    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_FULL_REDEEMPTION_, 2) ||
				    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REDEEM_REFUND_, 2) ||
				    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REFUND_, 2) ||
				    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_INQUIRY_, 2) ||
				    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_, 2))
				{
					memset(szTemplate, 0x00, sizeof(szTemplate));
					sprintf(szTemplate, "%08lu", pobTran->srBRec.lnHGBalancePoint);
					memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 8);
				}

				inPacketSizes += 8;
				/* HG Reserve (10 Byte) */
				inPacketSizes += 10;
			}
			else
			{
				inPacketSizes += 78;
			}
		}
		else
		{
			inPacketSizes += 78;
		}
	}
	
	if (gbBarCodeECRBit == VS_TRUE)
	{
		/* Üny 交易碼 */
		if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
		{
			if (uszHostResponseBit == VS_TRUE	&&
			    uszHostResponseSuccessBit == VS_TRUE)
			{
				/* 一維或二維條碼資料長度 */
				inBarCodeLen = strlen(pobTran->srBRec.szBarCodeData);
				inPacketSizes += 3;
				/* 一維或二維條碼資料內容 */
				inPacketSizes += inBarCodeLen;

				/* 補滿至980 */
				inPacketSizes = 980;

				/* Üny 交易碼 */
				if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
				{
					/* 除結束卡號查詢不用，其他Uny交易都要 */
					if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_END_CARD_NO_INQUIRY_, 2))
					{

					}
					else
					{
						memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szUnyTransCode, 20);
					}
				}

				inPacketSizes += 20;
			}
		}
		else if (pobTran->srEWRec.uszEWTransBit == VS_TRUE)
		{
			if (uszHostResponseBit == VS_TRUE	&&
			    uszHostResponseSuccessBit == VS_TRUE)
			{
				/* 電子錢包資料長度 */
				inBarCodeLen = strlen(pobTran->srBRec.szBarCodeData);
				sprintf(&szDataBuffer[inPacketSizes], "%03d", 184);
				inPacketSizes += 3;
				/* 電子錢包交易條碼 */
				if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2))
				{
					memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szBarCodeData, inBarCodeLen);
				}
				inPacketSizes += 25;

				/* 電子錢包業者特店代號 */
				memcpy(&szDataBuffer[inPacketSizes], pobTran->srEWRec.szEW_ChannelId, 50);
				inPacketSizes += 50;
				/* 電子錢包交易日期時間 */
				memcpy(&szDataBuffer[inPacketSizes], pobTran->srEWRec.szEW_PaymentDate, 24);
				inPacketSizes += 24;
				/* 消費者支付工具 */
				if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2)	||
				    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_EW_INQUIRY_TRANSACTION_, 2))
				{
					memcpy(&szDataBuffer[inPacketSizes], pobTran->srEWRec.szEW_PayMethod, 1);
				}
				inPacketSizes += 1;
				/* 訂單狀態 */
				if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_EW_INQUIRY_TRANSACTION_, 2))
				{
					memcpy(&szDataBuffer[inPacketSizes], pobTran->srEWRec.szEW_TransactionType, 2);
				}
				inPacketSizes += 2;
				/* 付款交易編號 */
				if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2))
				{
					memcpy(&szDataBuffer[inPacketSizes], pobTran->srEWRec.szEW_PaymentNo, 30);
				}
				inPacketSizes += 30;
				/* 是否為重複付款請求 */
				if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2))
				{
					memcpy(&szDataBuffer[inPacketSizes], pobTran->srEWRec.szEW_IsDuplicatedPaymentRequest, 1);
				}
				inPacketSizes += 1;
				/* 聯名卡代碼 */
				if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2) ||
				    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_EW_INQUIRY_TRANSACTION_, 2))
				{
					memcpy(&szDataBuffer[inPacketSizes], pobTran->srEWRec.szEW_CobrandedCode, 11);
				}
				inPacketSizes += 11;
				/* 載具類型 */
				if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_EW_INQUIRY_TRANSACTION_, 2))
				{
					memcpy(&szDataBuffer[inPacketSizes], pobTran->srEWRec.szEW_EinvoiceCarrierType, 6);
				}
				inPacketSizes += 6;
				/* 折抵金額/退還折抵金額 */
				if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2) ||
				    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_EW_INQUIRY_TRANSACTION_, 2))
				{
					memcpy(&szDataBuffer[inPacketSizes], pobTran->srEWRec.szEW_DiscountAmt, 10);
				}
				else if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REFUND_, 2))
				{
					memcpy(&szDataBuffer[inPacketSizes], pobTran->srEWRec.szEW_DiscountAmt, 10);
				}
				inPacketSizes += 10;
				/* 門市代碼 */
				memcpy(&szDataBuffer[inPacketSizes], pobTran->srEWRec.szEW_Store_no, 24);
				inPacketSizes += 24;
				/* 保留 */
				inPacketSizes += 413;
			}
		}
		else if (pobTran->srBRec.uszUPlanECRBit == VS_TRUE)
		{
			if (uszHostResponseBit == VS_TRUE	&&
			    uszHostResponseSuccessBit == VS_TRUE)
			{
				/* 一維或二維條碼資料長度 */
				inBarCodeLen = strlen(pobTran->srBRec.szBarCodeData);
				inPacketSizes += 3;
				/* 一維或二維條碼資料內容 */
				inPacketSizes += inBarCodeLen;

				/* 補滿至980 */
				inPacketSizes = 980;

				/* Üny 交易碼 */
				inPacketSizes += 20;
			}
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inECR_7E1_Standard_Pack_Error
Date&Time       :2019/8/28 上午 10:45
Describe        :
*/
int inECR_7E1_Standard_Pack_Error(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, ECR_TABLE * srECROb)
{
	int		inRetVal;
	int		i, inCardLen;
	int		inPacketSizes = 0;
	char		szTemplate[100 + 1];
	unsigned char	uszHostResponseBit = VS_FALSE;
	
	/* 有Response Code 代表主機有回 */
	if (strlen(pobTran->srBRec.szRespCode) > 0	||
	    strlen(pobTran->srTRec.szRespCode) > 0)
	{
		uszHostResponseBit = VS_TRUE;
	}

	/* Trans Type (2 Byte) */
	memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szTransType, 2);
	inPacketSizes += 2;

	/* HOST ID (2 Byte) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTRTFileName(szTemplate);

	if (!memcmp(szTemplate, _TRT_FILE_NAME_CREDIT_, strlen(_TRT_FILE_NAME_CREDIT_)))
		memcpy(&szDataBuffer[inPacketSizes], _ECR_7E1_NCCC_HOSTID_NCCC_, 2);
	/* 2018/5/8 下午 2:10 DFS需求不再使用大來主機 */
	else if (!memcmp(szTemplate, _TRT_FILE_NAME_DCC_, strlen(_TRT_FILE_NAME_DCC_)))
		memcpy(&szDataBuffer[inPacketSizes], _ECR_7E1_NCCC_HOSTID_DCC_, 2);

	inPacketSizes += 2;

	/* Receipt No (6 Byte) */
	if (pobTran->srBRec.lnOrgInvNum != 0L ||
	   !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_POINT_INQUIRY_, 2)) /* 點數查詢就算是沒有INV也要回傳 */
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%06ld", pobTran->srBRec.lnOrgInvNum);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 6);
	}

	inPacketSizes += 6;

	/* Card No (19 Byte)(左靠右補空白) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_POINT_INQUIRY_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_FULL_REDEEMPTION_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REDEEM_REFUND_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REWARD_REFUND_, 2)	||
	    pobTran->srBRec.inOrgCode == _HG_REWARD_REFUND_				||	/* 取消回饋或扣抵退貨不需回傳信用卡卡號 */
	    pobTran->srBRec.inOrgCode == _HG_REDEEM_REFUND_				||
	    pobTran->srBRec.lnHGPaymentType == _HG_PAY_CASH_				||	/* 若是純HG交易，信用卡卡號不需回傳 */
	    pobTran->srBRec.lnHGPaymentType == _HG_PAY_GIFT_PAPER_			||
	    pobTran->srBRec.inHGCode == _HG_FULL_REDEMPTION_)				/* 修改ECR交易取消點數兌換多印卡號。*/
	{
		inPacketSizes += 19;
	}
	else
	{
		/* 卡號是否遮掩 */
		inRetVal = inECR_CardNoTruncateDecision(pobTran);

		/* 要遮卡號 */
		if (inRetVal == VS_SUCCESS)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			strcpy(szTemplate, pobTran->srBRec.szPAN);

			/* HAPPG_GO 卡不掩飾 */
			if (!memcmp(&pobTran->srBRec.szPAN[0], "9552", 4))
			{

			}
			else
			{
				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
				/* 卡號長度 */
				inCardLen = strlen(szTemplate);

				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
				for (i = 6; i < (inCardLen - 4); i ++)
					szTemplate[i] = '*';
			}

			memcpy(&szDataBuffer[inPacketSizes], szTemplate, strlen(szTemplate));
		}
		else
		{
			memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szPAN, strlen(pobTran->srBRec.szPAN));
		}

		inPacketSizes += 19;
	}

	/* Card Expire Date (4 Byte) */
	/* 這裡回傳條件抄Verifone的Code */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_POINT_INQUIRY_, 2)	||	/* 點數查詢 */
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_FULL_REDEEMPTION_, 2)	||	/* 點數扣抵 */
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REDEEM_REFUND_, 2)	||	/* 扣抵退貨 */
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REWARD_REFUND_, 2)	||	/* 回饋退貨 */
	    pobTran->srBRec.inOrgCode == _HG_REWARD_REFUND_				||	/* 取消回饋或扣抵退貨不需回傳信用卡卡號 */
	    pobTran->srBRec.inOrgCode == _HG_REDEEM_REFUND_				||
	    pobTran->srBRec.lnHGPaymentType == _HG_PAY_CASH_				||	/* 若是純HG交易，信用卡卡號不需回傳 */
	    pobTran->srBRec.lnHGPaymentType == _HG_PAY_GIFT_PAPER_)
	{
		inPacketSizes += 4;
	}
	else
	{
		/* 有效期是否回傳 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetECRExpDateReturnEnable(szTemplate);
		
		/* 要回傳有效期 */
		if (!memcmp(&szTemplate, "Y", 1))
			memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szExpDate, strlen(pobTran->srBRec.szExpDate));

		inPacketSizes += 4;
	}

	/* Trans Amount (12 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_SALE_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REFUND_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_OFFLINE_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_PREAUTH_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_INSTALLMENT_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_INSTALLMENT_REFUND_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_INSTALLMENT_ADJUST_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REDEEM_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REDEEM_REFUND_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REDEEM_ADJUST_, 2))
	{
		if (pobTran->srBRec.lnTxnAmount != 0)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%010ld00", pobTran->srBRec.lnTxnAmount);
			memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
		}
	}
	inPacketSizes += 12;

	/* Trans Date (6 Byte) & Trans Time (6 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_SALE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REFUND_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_VOID_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_OFFLINE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_INSTALLMENT_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_INSTALLMENT_REFUND_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REDEEM_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REDEEM_REFUND_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_POINT_INQUIRY_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_ONLINE_REDEEMPTION_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_POINT_CERTAIN_, 2)||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_FULL_REDEEMPTION_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REDEEM_REFUND_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REWARD_REFUND_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REWARD_SALE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REWARD_INSTALLMENT_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REWARD_REDEMPTION_, 2))
	{
		/* Trans Date */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		strcat(szTemplate, pobTran->srBRec.szDate);
		memcpy(&szDataBuffer[inPacketSizes], &szTemplate[2], 6);
		inPacketSizes += 6;

		/* Trans Time */
		memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szTime, 6);
		inPacketSizes += 6;
	}
	else
	{
		inPacketSizes += 6;
		inPacketSizes += 6;
	}

	/* Approval No (9 Byte) */
	if (uszHostResponseBit == VS_TRUE)
	{
		if (strlen(pobTran->srBRec.szAuthCode) > 0)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			strcpy(szTemplate, pobTran->srBRec.szAuthCode);
			inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 9, _PADDING_RIGHT_);
			memcpy(&szDataBuffer[inPacketSizes], szTemplate, 9);
		}
	}
	else
	{
		
	}

	inPacketSizes += 9;
	
	/* 保留欄位/實際支付金額(紅利交易)/分期期數 + 分期手續費(分期交易) (12 Byte) */
	if (pobTran->srBRec.uszRedeemBit == VS_TRUE && pobTran->srBRec.uszVOIDBit != VS_TRUE)
	{
		/* 實際支付金額 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010ld00", (pobTran->srBRec.lnRedemptionPaidCreditAmount + pobTran->srBRec.lnTipTxnAmount));
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
		
		inPacketSizes += 12;
	}
	else if (pobTran->srBRec.uszInstallmentBit == VS_TRUE && pobTran->srBRec.uszVOIDBit != VS_TRUE)
	{
		/* 分期期數 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%02ld", pobTran->srBRec.lnInstallmentPeriod);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 2);
		
		inPacketSizes += 2;
		
		/* 分期手續費 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010ld", pobTran->srBRec.lnInstallmentFormalityFee);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 10);
		
		inPacketSizes += 10;
	}
	else if (pobTran->srBRec.lnHGTransactionType == _HG_ONLINE_REDEEM_ && pobTran->srBRec.uszVOIDBit != VS_TRUE)
	{
		/* HG點數扣抵要回傳實際支付金額 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010ld00", pobTran->srBRec.lnHGAmount);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
		
		inPacketSizes += 12;
	}
	else
	{
		if (pobTran->srBRec.uszContactlessBit == VS_TRUE || pobTran->srBRec.uszRefundCTLSBit == VS_TRUE)
			memcpy(&szDataBuffer[inPacketSizes], "W", 1);
		
		inPacketSizes ++;
		
		if (!memcmp(&srECROb->srTransData.szField_05[0], "C", 1))
			memcpy(&szDataBuffer[inPacketSizes], &srECROb->srTransData.szField_05[0], 1);
		else
			memcpy(&szDataBuffer[inPacketSizes], "0", 1);

		inPacketSizes ++; /* CUP Indicator */
		inPacketSizes += 10;
	}
	
	/* ECR Response Code (4 Byte) */
	memcpy(&szDataBuffer[inPacketSizes], "0000", 4);
	inPacketSizes += 4;
	
	/* Terminal ID (8 Byte) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTerminalID(szTemplate);
	inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 8, _PADDING_RIGHT_);
	memcpy(&szDataBuffer[inPacketSizes], szTemplate, 8);
	inPacketSizes += 8;

	/* Reference No */
	if (strlen(pobTran->srBRec.szRefNo) > 0)
	{
		memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szRefNo, 12);
	}
	inPacketSizes += 12;
	
	/* 保留欄位/扣抵紅利點數(紅利交易)/首期金額(分期交易) (12 Byte) */
	if (pobTran->srBRec.uszRedeemBit == VS_TRUE)
	{
		/* 實際支付金額 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010ld00", pobTran->srBRec.lnRedemptionPoints);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
	}
	else if (pobTran->srBRec.uszInstallmentBit == VS_TRUE)
	{
		/* 首期金額 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010ld00", pobTran->srBRec.lnInstallmentDownPayment);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
	}
	else
	{
	        /* HG點數扣抵 回傳扣抵點數 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		if (pobTran->srBRec.lnHGTransactionType == _HG_ONLINE_REDEEM_ && pobTran->srBRec.uszVOIDBit != VS_TRUE)
			sprintf(szTemplate, "%012lu", pobTran->srBRec.lnHGTransactionPoint);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
	}
	inPacketSizes += 12;
	
	/* HGPAN(Request:Store Id Response:HappyGo Card Number) (18 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_POINT_INQUIRY_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_FULL_REDEEMPTION_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REDEEM_REFUND_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REWARD_REFUND_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_POINT_CERTAIN_, 2)||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_ONLINE_REDEEMPTION_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REWARD_SALE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REWARD_INSTALLMENT_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REWARD_REDEMPTION_, 2))
	{
		inRetVal = inECR_CardNoTruncateDecision_HG(pobTran);
		/* 要遮卡號 */
		if (inRetVal == VS_SUCCESS)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			strcpy(szTemplate, pobTran->srBRec.szHGPAN);

			/* HAPPG_GO 卡不掩飾 */
			if (!memcmp(&pobTran->srBRec.szHGPAN[0], "9552", 4))
			{

			}
			else
			{
				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
				/* 卡號長度 */
				inCardLen = strlen(szTemplate);

				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
				for (i = 6; i < (inCardLen - 4); i ++)
					szTemplate[i] = '*';

			}

			memcpy(&szDataBuffer[inPacketSizes], szTemplate, strlen(szTemplate));
		}
		else
		{
			memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szHGPAN, strlen(pobTran->srBRec.szHGPAN));
		}
	}
	inPacketSizes += 18;
	
	/* 保留欄位(10 Byte)+ START Trans Type (2 Byte) /剩餘紅利點數(紅利交易)/每期金額(分期交易) (12 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_START_CARD_NO_INQUIRY_, 1))
	{
		inPacketSizes += 10;
		
		/* START Trans Type (2 Byte) */
		memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szStartTransType, 2);
				
		inPacketSizes += 2;
	}
	else if (pobTran->srBRec.uszRedeemBit == VS_TRUE)
	{
		/* 剩餘紅利點數(紅利交易) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010ld00", pobTran->srBRec.lnRedemptionPointsBalance);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
		
		inPacketSizes += 12;
	}
	else if (pobTran->srBRec.uszInstallmentBit == VS_TRUE)
	{
		/* 每期金額(分期交易) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010ld", pobTran->srBRec.lnInstallmentPayment);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
		
		inPacketSizes += 12;
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inECR_8N1_Standard_Parse_Data
Date&Time       :2019/9/3 上午 10:31
Describe        :
*/
int inECR_8N1_Standard_Parse_Data(ECR_TABLE* srECROb, char *szDataBuffer)
{
	int	i = 0;
	int	inRetVal = VS_ERROR;
	int	inBarCodeLen = 0;
        char	szDate[6 + 1];
        char	szTime[6 + 1];
        char    szTemplate[20 + 1] = {0};
	char	szCustomIndicator[3 + 1] = {0};
        RTC_NEXSYS	srRTC; 		/* Date & Time */
	
	memset(szCustomIndicator, 0x00, sizeof(szCustomIndicator));
	inGetCustomIndicator(szCustomIndicator);
	
	/* ECR Indicator */
	memcpy(srECROb->srTransData.szECRIndicator, &szDataBuffer[0], 1);
	
	/* ECR Version Date */
	
	/* Trans Type Indicator */
	memcpy(srECROb->srTransData.szTransTypeIndicator , &szDataBuffer[7], 1);
	
	/* Trans Type */
	memcpy(srECROb->srTransData.szTransType, &szDataBuffer[8], 2);
	
	/* CUP / Smart pay / ESVC Indicator */
	memcpy(&srECROb->srTransData.szField_05[0], &szDataBuffer[10], 1);
	/* 要先判斷是否為電子錢包 */
	if (!memcmp(&srECROb->srTransData.szField_05[0], _ECR_EW_INDICATOR_, 1))
	{
		srECROb->srTransData.srEW_Data.uszEWTransBit = VS_TRUE;
	}
	else if (!memcmp(&srECROb->srTransData.szField_05[0], _ECR_TRUST_INDICATOR_, 1))
	{
		srECROb->srTransData.srTRUST_Data.uszTRUSTTransBit = VS_TRUE;
	}
	
	/* Host ID */
	memcpy(&srECROb->srTransData.szHostID[0], &szDataBuffer[11], 2);
	
	if (srECROb->srTransData.srTRUST_Data.uszTRUSTTransBit == VS_TRUE)
	{
		/* 共用收單序號 */
		memcpy(&srECROb->srTransData.srTRUST_Data.szTrustRRN[0], &szDataBuffer[13], 12);
	}
	else
	{
		/* Receipt No */
		memcpy(&srECROb->srTransData.szReceiptNo[0], &szDataBuffer[13], 6);
	
		/* Card No */
		memcpy(&srECROb->srTransData.szCardNo[0], &szDataBuffer[19], 19);
	
		/* Card Expire Date/ 結帳總筆數 */
		memcpy(&srECROb->srTransData.szField_09[0], &szDataBuffer[38], 4);
	}
	
	/* Trans Amount/結帳總金額 */
	memcpy(&srECROb->srTransData.szField_10[0], &szDataBuffer[42], 12);
	
        memset(&srRTC, 0x00, sizeof(RTC_NEXSYS));
	inFunc_GetSystemDateAndTime(&srRTC);
        memset(szDate, 0x00, sizeof(szDate));
        sprintf(szDate, "%02d%02d%02d",  srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
        memset(szTime, 0x00, sizeof(szTime));
        sprintf(szTime, "%02d%02d%02d",  srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
        
	/* Trans Date */
	memcpy(&srECROb->srTransData.szTransDate[0], &szDate[0], 6);
	
	/* Trans Time */
	memcpy(&srECROb->srTransData.szTransTime[0], &szTime[0], 6);
	
	/* Approval No */
	memcpy(&srECROb->srTransData.szApprovalNo[0], &szDataBuffer[66], 6);
	
	/* Wave Card (Contactless) Indicator */
	memcpy(&srECROb->srTransData.szWaveCardIndicator[0], &szDataBuffer[75], 1);
	
	/* ECR Response Code */
	memcpy(&srECROb->srTransData.szECRResponseCode[0], &szDataBuffer[76], 4);
	
	/* Merchant ID */
	memcpy(&srECROb->srTransData.szMerchantID[0], &szDataBuffer[80], 15);
	
	/* Terminal ID */
	memcpy(&srECROb->srTransData.szTerminalID[0], &szDataBuffer[95], 8);
	
	/* Exp Amount */
	if (memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_, 2) == 0)
	{
		memcpy(&srECROb->srTransData.szAuthAmount[0], &szDataBuffer[103], 12);
	}
	else
	{
		memcpy(&srECROb->srTransData.szExpAmount[0], &szDataBuffer[103], 12);
	}
	
	/* Store Id */
	memcpy(&srECROb->srTransData.szStoreId[0], &szDataBuffer[115], 18);
	
	if (srECROb->srTransData.srTRUST_Data.uszTRUSTTransBit == VS_TRUE)
	{
		/* 銷帳編號*/
		memcpy(&srECROb->srTransData.srTRUST_Data.szReconciliationNo[0], &szDataBuffer[133], 20);
		/* 信託機構代碼 */
		memcpy(&srECROb->srTransData.srTRUST_Data.szTrustInstitutionCode[0], &szDataBuffer[153], 7);
		/* 信託原交易日期 */
		memcpy(&srECROb->srTransData.srTRUST_Data.szTrustOrgDate[0], &szDataBuffer[212], 4);
	}
	else
	{
		/* Installment/Redeem Indicator/Award Redeem Mode */
		memcpy(&srECROb->srTransData.szField_20[0], &szDataBuffer[133], 1);

		if (memcmp(srECROb->srTransData.szTransType, _ECR_8N1_AWARD_REDEEM_, _ECR_8N1_AWARD_REDEEM_LEN_) == 0	||
		    memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_AWARD_REDEEM_, _ECR_8N1_VOID_AWARD_REDEEM_LEN_) == 0)
		{
			/* BarCode1 */
			memcpy(&srECROb->srTransData.szBarCode1[0], &szDataBuffer[134], 20);

			/* BarCode2 */
			memcpy(&srECROb->srTransData.szBarCode2[0], &szDataBuffer[154], 20);
		}
		else if (srECROb->srTransData.srEW_Data.uszEWTransBit == VS_TRUE)
		{
			/* 一般金額/一般退貨金額 */
			memcpy(&srECROb->srTransData.srEW_Data.szEW_NormalAmt[0], &szDataBuffer[134], 10);

			/* 代收金額/代收退貨金額 */
			memcpy(&srECROb->srTransData.srEW_Data.szEW_CollectionAmt[0], &szDataBuffer[144], 10);

			/* 代售金額/代售退貨金額 */
			memcpy(&srECROb->srTransData.srEW_Data.szEW_ConsignmentAmt[0], &szDataBuffer[154], 10);

			/* 不可折抵金額 */
			memcpy(&srECROb->srTransData.srEW_Data.szEW_PromotionRestriction[0], &szDataBuffer[164], 10);
		}
		else
		{
			/* RDM Paid Amt */
			memcpy(&srECROb->srTransData.szRDMPaidAmt[0], &szDataBuffer[134], 12);

			/* RDM Point */
			memcpy(&srECROb->srTransData.szRDMPoint[0], &szDataBuffer[146], 8);

			/* Points of Balance */
			memcpy(&srECROb->srTransData.szPointsOfBalance[0], &szDataBuffer[154], 8);

			/* Redeem Amt */
			memcpy(&srECROb->srTransData.szRedeemAmt[0], &szDataBuffer[162], 12);
		}

		/* Installment Period */
		memcpy(&srECROb->srTransData.szInstallmentPeriod[0], &szDataBuffer[174], 2);

		/* Down Payment Amount /ESVC Balance before Tx. */
		memcpy(&srECROb->srTransData.szField_26[0], &szDataBuffer[176], 12);

		/* Installment Payment Amount/ESVC Balance after Tx. */
		memcpy(&srECROb->srTransData.szField_27[0], &szDataBuffer[188], 12);

		/* Formality Fee/ESVC Autoload Amount */
		memcpy(&srECROb->srTransData.szField_28[0], &szDataBuffer[200], 12);

		/* Card Type */
		memcpy(&srECROb->srTransData.szCardType[0], &szDataBuffer[212], 2);

		/* Batch No */
		memcpy(&srECROb->srTransData.szBatchNo[0], &szDataBuffer[214], 6);
	}
	
	if (srECROb->srTransData.srEW_Data.uszEWTransBit == VS_TRUE)
	{
		/* 特店訂單編號 */
		memcpy(&srECROb->srTransData.srEW_Data.szEW_OrderId[0], &szDataBuffer[220], 30);
		
		/* reserved (6bytes) */
	}
	else
	{
		/* Start Trans Type */
		memcpy(&srECROb->srTransData.szStartTransType[0], &szDataBuffer[220], 2);

		/* MP Flag */
		memcpy(&srECROb->srTransData.szMPFlag[0], &szDataBuffer[222], 1);

		/* SP ISSUER ID */
		memcpy(&srECROb->srTransData.szSPIssuerID[0], &szDataBuffer[223], 8);

		/* SP / 信用卡/ESVC Origin Date */
		memcpy(&srECROb->srTransData.szField_34[0], &szDataBuffer[231], 8);

		/* SP Origin RRN / 信用卡OriginRRN / ATS電票交易序號 */
		memcpy(&srECROb->srTransData.szField_35[0], &szDataBuffer[239], 12);

		/* Pay Item */
		memcpy(&srECROb->srTransData.szPayItem, &szDataBuffer[251], 5);
	}
	
	/* Card No. Hash Value */
	memcpy(&srECROb->srTransData.szCardNoHashValue, &szDataBuffer[256], 50);
	
	if (srECROb->srTransData.srEW_Data.uszEWTransBit == VS_TRUE)
	{
		/* 退貨訂單編號 */
		memcpy(&srECROb->srTransData.srEW_Data.szEW_RefundTradeNo[0], &szDataBuffer[306], 30);
		
		/* 電子錢包交易序號 */
		memcpy(&srECROb->srTransData.srEW_Data.szEW_TransactionId[0], &szDataBuffer[336], 64);
	}
	else
	{
		if (!memcmp(szCustomIndicator, _CUSTOMER_INDICATOR_126_MASTERCARD_FLIGHT_TICKET_, _CUSTOMER_INDICATOR_SIZE_))
		{
			/* MP Response Code */
			memcpy(&srECROb->srTransData.szMPResponseCode, &szDataBuffer[306], 6);

			/* ASM Award flag */
			memcpy(&srECROb->srTransData.szASMAwardFlag, &szDataBuffer[312], 1);

			/* MCP Indicator */
			memcpy(&srECROb->srTransData.szMCPIndicator, &szDataBuffer[313], 1);

			/* 金融機構代碼 */
			memcpy(&srECROb->srTransData.szIssuerBankID, &szDataBuffer[314], 3);

			/* Reserved */

			/* 機票交易 */
			/* 判斷是否是機票交易 */
			memcpy(&srECROb->srTransData.szFlightTicketTransBit, &szDataBuffer[322], 1);

			/* 出發地機場代碼 */
			memcpy(&srECROb->srTransData.szFlightTicketPDS0523, &szDataBuffer[323], 5);
			
			/* 目的地機場代碼 */
			memcpy(&srECROb->srTransData.szFlightTicketPDS0524, &szDataBuffer[328], 5);
			
			/* 航班號碼 */
			memcpy(&srECROb->srTransData.szFlightTicketPDS0530, &szDataBuffer[333], 5);
		}
		else
		{
			/* MP Response Code */
			memcpy(&srECROb->srTransData.szMPResponseCode, &szDataBuffer[306], 6);

			/* ASM Award flag */
			memcpy(&srECROb->srTransData.szASMAwardFlag, &szDataBuffer[312], 1);

			/* MCP Indicator */
			memcpy(&srECROb->srTransData.szMCPIndicator, &szDataBuffer[313], 1);

			/* 金融機構代碼 */
			memcpy(&srECROb->srTransData.szIssuerBankID, &szDataBuffer[314], 3);

			/* Reserved */

			/* HG DATA */
			/* 支付工具 */
			memcpy(&srECROb->srTransData.szHGPaymentTool, &szDataBuffer[322], 2);

			/* 原扣抵或回饋點數 */
			memcpy(&srECROb->srTransData.szHGRedeemPoint, &szDataBuffer[366], 8);
		}
	}
	
	/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/10/12 下午 5:55 */
	if (gbBarCodeECRBit == VS_TRUE)
	{	
		if (srECROb->srTransData.srEW_Data.uszEWTransBit == VS_TRUE)
		{
			/* 電子錢包資料長度 */
			memcpy(&srECROb->srTransData.szBarCodeLen, &szDataBuffer[400], 3);
			/* 電子錢包交易條碼 */
			for (i = 0; i < 25; i++)
			{
				if (szDataBuffer[403 + i] == ' ')
				{
					break;
				}
				
				memcpy(&srECROb->srTransData.szBarCodeData[i], &szDataBuffer[403 + i], 1);
			}
			/* 電子錢包業者特店代號*/
			memcpy(&srECROb->srTransData.srEW_Data.szEW_ChannelId[0], &szDataBuffer[428], 50);
			/* 消費者支付工具 */
			memcpy(&srECROb->srTransData.srEW_Data.szEW_PayMethod[0], &szDataBuffer[478], 1);
			/* 訂單狀態 */
			memcpy(&srECROb->srTransData.srEW_Data.szEW_TransactionType[0], &szDataBuffer[479], 2);
			/* 付款交易編號 */
			memcpy(&srECROb->srTransData.srEW_Data.szEW_PaymentNo[0], &szDataBuffer[481], 30);
			/* 是否為重複付款請求 */
			memcpy(&srECROb->srTransData.srEW_Data.szEW_IsDuplicatedPaymentRequest[0], &szDataBuffer[511], 1);
			/* 聯名卡代碼 */
			memcpy(&srECROb->srTransData.srEW_Data.szEW_CobrandedCode[0], &szDataBuffer[512], 11);
			/* 載具類型 */
			memcpy(&srECROb->srTransData.srEW_Data.szEW_EinvoiceCarrierType[0], &szDataBuffer[523], 6);
			/* 折抵金額/退還折抵金額 */
			memcpy(&srECROb->srTransData.srEW_Data.szEW_DiscountAmt[0], &szDataBuffer[529], 10);
			/* 門市代碼 */
			memcpy(&srECROb->srTransData.srEW_Data.szEW_Store_no[0], &szDataBuffer[539], 24);
			/* reserved(417Bytes)*/
		}
		else
		{
			/* 一維或二維條碼資料長度 */
			memcpy(&srECROb->srTransData.szBarCodeLen, &szDataBuffer[400], 3);
			inBarCodeLen = atoi(srECROb->srTransData.szBarCodeLen);

			/* 一維或二維條碼資料內容 */
			memcpy(&srECROb->srTransData.szBarCodeData, &szDataBuffer[403], inBarCodeLen);
                        
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memcpy(szTemplate, &szDataBuffer[980], 20);
                        if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
                        {
                                
                        }
                        else
                        {
                                /* Üny 交易碼 */
                                memcpy(&srECROb->srTransData.szUnyTransCode, szTemplate, 20);
                        }
		}
	}
	
	return (inRetVal);
}

/*
Function        :inRS232_ECR_SelectTransType
Date&Time       :2016/6/22 上午 9:47
Describe        :Indicator為'S'時的交易選單
*/
int inECR_SelectTransType(ECR_TABLE * srECROb)
{
	int			inTransType = 0;
	int			inRetVal = VS_SUCCESS;
	int			inChoice = 0;
	int			inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_8_3X3_;
	char			szTemplate[10 + 1];
	char			szDebugMsg[100 + 1];
	char			szKey = 0x00;
	BYTE			bUnyBit = VS_FALSE;
	MENU_CHECK_TABLE	srMenuChekDisplay1[] =
	{
		{_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_	, _SALE_		, inNCCC_Func_Check_Transaction_Function	, _ICON_HIGHTLIGHT_ECR_OPTION_SALE_1_SALE_		},
		{_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_	, _REDEEM_SALE_		, inNCCC_Func_Check_Transaction_Function	, _ICON_HIGHTLIGHT_ECR_OPTION_SALE_2_REDEEM_SALE_	},
		{_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_6_	, _INST_SALE_		, inNCCC_Func_Check_Transaction_Function	, _ICON_HIGHTLIGHT_ECR_OPTION_SALE_3_INST_SALE_		},
		{_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_7_	, _CUP_SALE_		, inNCCC_Func_Check_Transaction_Function	, _ICON_HIGHTLIGHT_ECR_OPTION_SALE_4_CUP_SALE_		},
		{_Touch_NONE_					, _TRANS_TYPE_NULL_	, NULL						, ""							}
	};
	MENU_CHECK_TABLE	srMenuChekDisplay2[] =
	{
		{_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_	, _SALE_OFFLINE_	, inNCCC_Func_Check_Transaction_Function	, _ICON_HIGHTLIGHT_ECR_OPTION_ADJUST_1_SALE_OFFLINE_	},
		{_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_	, _REDEEM_ADJUST_	, inNCCC_Func_Check_Transaction_Function	, _ICON_HIGHTLIGHT_ECR_OPTION_ADJUST_2_REDEEM_ADJUST_	},
		{_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_6_	, _INST_ADJUST_		, inNCCC_Func_Check_Transaction_Function	, _ICON_HIGHTLIGHT_ECR_OPTION_ADJUST_3_INST_ADJUST_	},
		{_Touch_NONE_					, _TRANS_TYPE_NULL_	, NULL						, ""							}
	};
	MENU_CHECK_TABLE	srMenuChekDisplay3[] =
	{
		{_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_	, _REFUND_		, inNCCC_Func_Check_Transaction_Function	, _ICON_HIGHTLIGHT_ECR_OPTION_REFUND_1_REFUND_		},
		{_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_	, _REDEEM_REFUND_	, inNCCC_Func_Check_Transaction_Function	, _ICON_HIGHTLIGHT_ECR_OPTION_REFUND_2_REDEEM_REFUND_	},
		{_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_6_	, _INST_REFUND_		, inNCCC_Func_Check_Transaction_Function	, _ICON_HIGHTLIGHT_ECR_OPTION_REFUND_3_INST_REFUND_	},
		{_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_7_	, _CUP_REFUND_		, inNCCC_Func_Check_Transaction_Function	, _ICON_HIGHTLIGHT_ECR_OPTION_REFUND_4_CUP_REFUND_	},
		{_Touch_NONE_					, _TRANS_TYPE_NULL_	, NULL						, ""							}
	};
	
	/* 若進選單，則不參考原ECR送之CUP Indicator，以選單之交易為主 */
	memset(&srECROb->srTransData.szField_05[0], 0x00, sizeof(srECROb->srTransData.szField_05[0]));	
        memset(szTemplate, 0x00, sizeof(szTemplate));
        memcpy(szTemplate, &srECROb->srTransData.szTransType[0], 2);
        inTransType = atoi(szTemplate);
	
	if (inNCCC_Func_Check_Uny_Format(srECROb->srTransData.szBarCodeData) == VS_TRUE)
	{
		bUnyBit = VS_TRUE;
	}
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "%d", inTransType);
		inLogPrintf(AT, szDebugMsg);
        }
	
        switch (inTransType)
        {
                case _ECR_8N1_SALE_NO_ :
                        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般交易＞ */
			inDISP_PutGraphic(_MENU_SALE_OPTION_, 0, _COORDINATE_Y_LINE_8_4_);
			/* 檢查功能開關，並顯示反白的圖 */
			inMENU_CHECK_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay1);
			if (bUnyBit == VS_TRUE)
			{
				inMENU_Display_ICON(srMenuChekDisplay1[3].szFileName, srMenuChekDisplay1[3].inButtonPositionID);
			}
			
			inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_8_3X3_;
                        while (1)
                        {
                                inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
				szKey = uszKBD_Key();
                                
				/* 回傳Timeout */
				if (inTimerGet(_ECR_RS232_RECEIVE_TIMER_) == VS_SUCCESS)
				{
					szKey = _KEY_TIMEOUT_;
				}
				
				/* 一般交易 */
				/* 觸控要功能有開才能算 */
                                if ((szKey == _KEY_1_ || inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_)	&& 
				   (inNCCC_Func_Check_Transaction_Function(_SALE_) == VS_SUCCESS))
				{
                                        memcpy(&srECROb->srTransData.szTransType[0], _ECR_8N1_SALE_, 2);
					memcpy(&srECROb->srTransData.szField_05[0], "N", strlen("N"));
					inRetVal = VS_SUCCESS;
					break;
				}
				/* 紅利扣抵 */
                                else if ((szKey == _KEY_2_ || inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_)	&& 
					(inNCCC_Func_Check_Transaction_Function(_REDEEM_SALE_) == VS_SUCCESS))
				{
                                        memcpy(&srECROb->srTransData.szTransType[0], _ECR_8N1_REDEEM_, 2);
					memcpy(&srECROb->srTransData.szField_05[0], "N", strlen("N"));
					inRetVal = VS_SUCCESS;
					break;
				}
				/* 分期付款 */
                                else if ((szKey == _KEY_3_ || inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_6_)	&& 
					(inNCCC_Func_Check_Transaction_Function(_INST_SALE_) == VS_SUCCESS))
				{
                                        memcpy(&srECROb->srTransData.szTransType[0], _ECR_8N1_INSTALLMENT_, 2);
					memcpy(&srECROb->srTransData.szField_05[0], "N", strlen("N"));
					inRetVal = VS_SUCCESS;
					break;
				}
				else if ((szKey == _KEY_4_ || inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_7_)	&& 
					(inNCCC_Func_Check_Transaction_Function(_CUP_SALE_) == VS_SUCCESS && bUnyBit != VS_TRUE))
				{
                                        memcpy(&srECROb->srTransData.szTransType[0], _ECR_8N1_SALE_, 2);
					memcpy(&srECROb->srTransData.szField_05[0], "C", strlen("C"));
					inRetVal = VS_SUCCESS;
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

                        break;
                case _ECR_8N1_OFFLINE_NO_ :
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_SALE_OFFLINE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜交易補登＞ */
			inDISP_PutGraphic(_MENU_SALE_OFFLINE_OPTION_, 0, _COORDINATE_Y_LINE_8_4_);
			/* 檢查功能開關，並顯示反白的圖 */
			inMENU_CHECK_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay2);

			inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_8_3X3_;
                        while (1)
                        {
                                inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
				szKey = uszKBD_Key();
                                
				/* 回傳Timeout */
				if (inTimerGet(_ECR_RS232_RECEIVE_TIMER_) == VS_SUCCESS)
				{
					return (VS_TIMEOUT);
				}
				
				/* 一般補登 */
                                if (szKey == _KEY_1_			||
				   (inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_	&& (inNCCC_Func_Check_Transaction_Function(_SALE_OFFLINE_) == VS_SUCCESS)))
				{
                                        memcpy(&srECROb->srTransData.szTransType[0], _ECR_8N1_OFFLINE_, 2);
					memcpy(&srECROb->srTransData.szField_05[0], "N", strlen("N"));
					inRetVal = VS_SUCCESS;
					break;
                                }
				/* 紅利調帳 */
                                else if (szKey == _KEY_2_			||
					((inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_)	&& (inNCCC_Func_Check_Transaction_Function(_REDEEM_ADJUST_) == VS_SUCCESS)))
				{
                                        memcpy(&srECROb->srTransData.szTransType[0], _ECR_8N1_REDEEM_ADJUST_, 2);
					memcpy(&srECROb->srTransData.szField_05[0], "N", strlen("N"));
					inRetVal = VS_SUCCESS;
					break;
                                }
				/* 分期調帳 */
                                else if (szKey == _KEY_3_			||
					((inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_6_)	&& (inNCCC_Func_Check_Transaction_Function(_INST_ADJUST_) == VS_SUCCESS)))
				{
                                        memcpy(&srECROb->srTransData.szTransType[0], _ECR_8N1_INSTALLMENT_ADJUST_, 2);
					memcpy(&srECROb->srTransData.szField_05[0], "N", strlen("N"));
					inRetVal = VS_SUCCESS;
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

                        break;
                case _ECR_8N1_REFUND_NO_ :
                        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜一般退貨＞ */
			inDISP_PutGraphic(_MENU_REFUND_OPTION_5_, 0, _COORDINATE_Y_LINE_8_4_);
			/* 檢查功能開關，並顯示反白的圖 */
			inMENU_CHECK_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay3);
			if (bUnyBit == VS_TRUE)
			{
				inMENU_Display_ICON(srMenuChekDisplay1[3].szFileName, srMenuChekDisplay1[3].inButtonPositionID);
			}

			inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_8_3X3_;
                        while (1)
                        {
                                inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
				szKey = uszKBD_Key();
                                
				/* 回傳Timeout */
				if (inTimerGet(_ECR_RS232_RECEIVE_TIMER_) == VS_SUCCESS)
				{
					szKey = _KEY_TIMEOUT_;
				}
				
				/* 一般退貨 */
                                if (szKey == _KEY_1_			||
				   (inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_	&& (inNCCC_Func_Check_Transaction_Function(_REFUND_) == VS_SUCCESS)))
				{
                                        memcpy(&srECROb->srTransData.szTransType[0], _ECR_8N1_REFUND_, 2);
					memcpy(&srECROb->srTransData.szField_05[0], "N", strlen("N"));
					inRetVal = VS_SUCCESS;
					break;
                                }
				/* 紅利退貨 */
                                else if (szKey == _KEY_2_			||
					(inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_	&& (inNCCC_Func_Check_Transaction_Function(_REDEEM_REFUND_) == VS_SUCCESS)))
				{
                                        memcpy(&srECROb->srTransData.szTransType[0], _ECR_8N1_REDEEM_REFUND_, 2);
					memcpy(&srECROb->srTransData.szField_05[0], "N", strlen("N"));
					inRetVal = VS_SUCCESS;
					break;
                                }
				/* 分期退貨 */
                                else if (szKey == _KEY_3_			||
					((inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_6_)	&& (inNCCC_Func_Check_Transaction_Function(_INST_REFUND_) == VS_SUCCESS)))
				{
                                        memcpy(&srECROb->srTransData.szTransType[0], _ECR_8N1_INSTALLMENT_REFUND_, 2);
					memcpy(&srECROb->srTransData.szField_05[0], "N", strlen("N"));
					inRetVal = VS_SUCCESS;
					break;
                                }
				else if ((szKey == _KEY_4_ || inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_7_)	&& 
					(inNCCC_Func_Check_Transaction_Function(_CUP_REFUND_) == VS_SUCCESS && bUnyBit != VS_TRUE))
				{
                                        memcpy(&srECROb->srTransData.szTransType[0], _ECR_8N1_REFUND_, 2);
					memcpy(&srECROb->srTransData.szField_05[0], "C", strlen("C"));
					inRetVal = VS_SUCCESS;
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
			
                        break;

                default :
			inRetVal = VS_ERROR;
                        break;
        }
	
	return (inRetVal);
}

/*
Function        :inRS232_ECR_SelectOtherCardType
Date&Time       :2018/2/8 下午 5:04
Describe        :Indicator為'0'時的交易選單
*/
int inECR_SelectOtherCardType(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int			inTransType = 0;
	int			inRetVal = VS_SUCCESS;
	int			inChoice = 0;
	int			inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_8_3X3_;
	char			szTemplate[10 + 1];
	char			szKey = 0x00;
	char			szDebugMsg[100 + 1] = {0};
	MENU_CHECK_TABLE	srMenuChekDisplay1[] =
	{
		{_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_	, _SALE_		, inNCCC_Func_Check_Transaction_Function	, _ICON_HIGHTLIGHT_ECR_OPTION_OTHERCARD_1_CREDITCARD_	},
		{_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_	, _TRANS_TYPE_NULL_	, inMENU_Check_CUP_Enable			, _ICON_HIGHTLIGHT_ECR_OPTION_OTHERCARD_2_CUP_CARD_	},
		{_Touch_NONE_					, _TRANS_TYPE_NULL_	, NULL						, ""							}
	};
	
	memset(szTemplate, 0x00, sizeof(szTemplate));
        memcpy(szTemplate, &srECROb->srTransData.szTransType[0], 2);
        inTransType = atoi(szTemplate);
	
	switch (inTransType)
        {
                case _ECR_8N1_SALE_NO_ :
                        inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般交易＞ */
			inDISP_PutGraphic(_MENU_ECR_TRANSTYPE_OPTION_, 0, _COORDINATE_Y_LINE_8_4_);
                        break;
		case _ECR_8N1_VOID_NO_ :
                        inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜取消交易＞ */
			inDISP_PutGraphic(_MENU_ECR_TRANSTYPE_OPTION_, 0, _COORDINATE_Y_LINE_8_4_);
                        break;
		case _ECR_8N1_REFUND_NO_ :
                        inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般退貨＞ */
			inDISP_PutGraphic(_MENU_ECR_TRANSTYPE_OPTION_, 0, _COORDINATE_Y_LINE_8_4_);
                        break;
                case _ECR_8N1_OFFLINE_NO_ :
                        inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_SALE_OFFLINE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜交易補登＞ */
			inDISP_PutGraphic(_MENU_ECR_TRANSTYPE_OPTION_, 0, _COORDINATE_Y_LINE_8_4_);
                        break;
		case _ECR_8N1_PREAUTH_NO_ :
                        inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_PRE_AUTH_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜預先授權＞ */
			inDISP_PutGraphic(_MENU_ECR_TRANSTYPE_OPTION_, 0, _COORDINATE_Y_LINE_8_4_);
                        break;
		case _ECR_8N1_PREAUTH_CANCEL_NO_ :
                        inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_PRE_AUTH_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜預先授權取消＞ */
			inDISP_PutGraphic(_MENU_ECR_TRANSTYPE_OPTION_, 0, _COORDINATE_Y_LINE_8_4_);
                        break;
		case _ECR_8N1_PREAUTH_COMPLETE_NO_ :
                        inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_PRE_COMP_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜預先授權完成＞ */
			inDISP_PutGraphic(_MENU_ECR_TRANSTYPE_OPTION_, 0, _COORDINATE_Y_LINE_8_4_);
                        break;
		case _ECR_8N1_PREAUTH_COMPLETE_CANCEL_NO_ :
                        inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);					/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_PRE_COMP_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜預先授權完成取消交易＞ */
			inDISP_PutGraphic(_MENU_ECR_TRANSTYPE_OPTION_, 0, _COORDINATE_Y_LINE_8_4_);
                        break;
                case _ECR_8N1_HG_REWARD_SALE_NO_:		/* HG紅利積點 */
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);					/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜HG紅利積點＞ */
			inDISP_PutGraphic(_MENU_ECR_TRANSTYPE_OPTION_, 0, _COORDINATE_Y_LINE_8_4_);
                        break;
		case _ECR_8N1_HG_ONLINE_REDEEMPTION_NO_:	/* HG點數抵扣 */
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);					/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_ONLINE_REDEEM_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG點數抵扣＞ */
			inDISP_PutGraphic(_MENU_ECR_TRANSTYPE_OPTION_, 0, _COORDINATE_Y_LINE_8_4_);
                        break;
		case _ECR_8N1_HG_POINT_CERTAIN_NO_:		/* HG加價購 */
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);					/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_POINT_CERTAIN_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG加價購＞ */
			inDISP_PutGraphic(_MENU_ECR_TRANSTYPE_OPTION_, 0, _COORDINATE_Y_LINE_8_4_);
                        break;
                default :
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "CUPIndicator 不支援此交易別");
				inLogPrintf(AT, szDebugMsg);
			}
			memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
			inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
			memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
			sprintf(pobTran->szErrorMsgBuff2, "CUP Indicator 不支援此交易別");
			pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
			return (VS_ERROR);
			
                        break;
        }
	/* 檢查功能開關，並顯示反白的圖 */
	inMENU_CHECK_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay1);

	inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_8_3X3_;
	while (1)
	{
		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
		szKey = uszKBD_Key();

		/* 回傳Timeout */
		if (inTimerGet(_ECR_RS232_RECEIVE_TIMER_) == VS_SUCCESS)
		{
			szKey = _KEY_TIMEOUT_;
		}

		/* 一般交易 */
		/* 觸控要功能有開才能算 */
		if (szKey == _KEY_1_			||
		   ((inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_)	&& (inNCCC_Func_Check_Transaction_Function(_SALE_) == VS_SUCCESS)))
		{
			
			inRetVal = VS_SUCCESS;
			break;
		}
		/* 銀聯交易 */
		else if (szKey == _KEY_2_			||
			((inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_)	&& (inMENU_Check_CUP_Enable(_TRANS_TYPE_NULL_) == VS_SUCCESS)))
		{
			pobTran->srBRec.uszCUPTransBit = VS_TRUE;
			inRetVal = VS_SUCCESS;
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
Function        :inECR_SelectAwardRedeemMode
Date&Time       :2019/12/17 下午 2:36
Describe        :輸入空值時的交易選單
*/
int inECR_SelectAwardRedeemMode(ECR_TABLE * srECROb, char *szAwardType)
{
	int			inPageLoop = _PAGE_LOOP_1_;
        int			inRetVal = VS_SUCCESS;
        int			inChioce1 = _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_;
        int			inCount= 1;
        int			inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_7_3X3_;
	char			szKey = 0x00;
	MENU_CHECK_TABLE	srMenuChekDisplay[] =
	{
		{_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_	, _LOYALTY_REDEEM_		, inNCCC_Loyalty_CreditCardFlag		, _ICON_HIGHTLIGHT_5_1_AWARD_SWIPE_	},
		{_NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_	, _LOYALTY_REDEEM_		, inNCCC_Loyalty_BarCodeFlag		, _ICON_HIGHTLIGHT_5_2_AWARD_BARCODE_	},
		{_Touch_NONE_					, _TRANS_TYPE_NULL_		, NULL					, ""					}
	};
	
	
	if (inMENU_Check_AWARD_Enable(0) != VS_SUCCESS)
	{
		inRetVal = VS_FUNC_CLOSE_ERR;
	}
	else
	{
		/* 初始化 */
		inDISP_ClearAll();
		inFunc_Display_LOGO(0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_LOYALTY_REDEEM_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	
//		inDISP_PutGraphic(_MENU_SALE_OPTION_, 0,  _COORDINATE_Y_LINE_8_4_);
		
		/* 檢查功能開關，並顯示反白的圖 */
		inMENU_CHECK_FUNCTION_ENABLE_DISPLAY(srMenuChekDisplay);
	
		/* 設定Timeout */
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);

		while (inPageLoop != _PAGE_LOOP_0_)
		{
			while (inPageLoop == _PAGE_LOOP_1_)
			{
				szKey = uszKBD_Key();
				inChioce1 = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc); /* 回傳MENU畫面的按鈕選擇 */
				if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
				{
					szKey = _KEY_TIMEOUT_;
				}

				switch (inChioce1)
				{
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_:
						inCount = 1;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_2_:
						inCount = 2;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_3_:
						inCount = 3;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_4_:
						inCount = 4;
						inPageLoop = _PAGE_LOOP_2_;
						break;
					case _NEWUI_RETURN_IDLE_BUTTON_:
						inRetVal = VS_ERROR;
						inPageLoop = _PAGE_LOOP_0_;
						break;
					case _NEWUI_LAST_PAGE_BUTTON_:
						inRetVal = VS_USER_CANCEL;
						inPageLoop = _PAGE_LOOP_0_;
						break;
					case _DisTouch_Slide_Down_To_Up_:
						inRetVal = VS_USER_CANCEL;
						inPageLoop = _PAGE_LOOP_0_;
						break;
					default:
						break;
				}

				if (szKey == _KEY_1_)
				{
					inCount = 1;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_2_)
				{
					inCount = 2;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_3_)
				{
					inCount = 3;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_4_)
				{
					inCount = 4;
					inPageLoop = _PAGE_LOOP_2_;
					break;
				}
				else if (szKey == _KEY_CANCEL_)
				{
					inRetVal = VS_USER_CANCEL;
					inPageLoop = _PAGE_LOOP_0_;
					break;
				}
				else if (szKey == _KEY_TIMEOUT_)
				{
					inRetVal = VS_TIMEOUT;
					inPageLoop = _PAGE_LOOP_0_;
					break;
				}

			}

			/* 判斷MENU點進去按鈕之後要做的事情 */
			if (inPageLoop == _PAGE_LOOP_2_)
			{
				switch (inCount)
				{
					case 1:
						memcpy(szAwardType, "2", 1);
						break;
					case 2:
						memcpy(szAwardType, "1", 1);
						break;
					default:
						inPageLoop = _PAGE_LOOP_0_;
						break;
				}
			}
		}/* _PAGE_LOOP_0_ */
	}
	
	return (inRetVal);
}

/*
Function        :inECR_7E1_Standard_Parse_Data
Date&Time       :2021/3/4 下午 4:23
Describe        :
*/
int inECR_7E1_Standard_Parse_Data(ECR_TABLE* srECROb, char *szDataBuffer)
{
	int	inRetVal = VS_ERROR;
	
	/* Trans Type */
	memcpy(srECROb->srTransData.szTransType, &szDataBuffer[0], 2);
	
	/* Host ID */
	memcpy(&srECROb->srTransData.szHostID[0], &szDataBuffer[2], 2);
	
	/* Receipt No */
	memcpy(&srECROb->srTransData.szReceiptNo[0], &szDataBuffer[4], 6);
	
	/* Card No */
	memcpy(&srECROb->srTransData.szCardNo[0], &szDataBuffer[10], 19);
	
	/* Card Expire Date/ 結帳總筆數 */
	memcpy(&srECROb->srTransData.szField_09[0], &szDataBuffer[29], 4);
	
	/* Trans Amount/結帳總金額 */
	memcpy(&srECROb->srTransData.szField_10[0], &szDataBuffer[33], 12);
	
	/* Trans Date */
	memcpy(&srECROb->srTransData.szTransDate[0], &szDataBuffer[45], 6);
	
	/* Trans Time */
	memcpy(&srECROb->srTransData.szTransTime[0], &szDataBuffer[51], 6);
	
	/* Approval No */
	memcpy(&srECROb->srTransData.szApprovalNo[0], &szDataBuffer[57], 6);
	
	/* pre auth amt/reserve */
	if (memcmp(srECROb->srTransData.szTransType, _ECR_7E1_PREAUTH_, 2) == 0)
	{
		memcpy(&srECROb->srTransData.szAuthAmount[0], &szDataBuffer[66], 12);
	}
	else if (memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REDEEM_, 2) == 0		||
		 memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REDEEM_REFUND_, 2) == 0	||
		 memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REDEEM_ADJUST_, 2) == 0)
	{
		memcpy(&srECROb->srTransData.szRDMPaidAmt[0], &szDataBuffer[66], 12);
	}
	else if (memcmp(srECROb->srTransData.szTransType, _ECR_7E1_INSTALLMENT_, 2) == 0	||
		 memcmp(srECROb->srTransData.szTransType, _ECR_7E1_INSTALLMENT_REFUND_, 2) == 0	||
		 memcmp(srECROb->srTransData.szTransType, _ECR_7E1_INSTALLMENT_ADJUST_, 2) == 0)
	{
		memcpy(&srECROb->srTransData.szInstallmentPeriod[0], &szDataBuffer[66], 2);
		memcpy(&srECROb->srTransData.szField_28[0], &szDataBuffer[68], 10);
	}
	
	/* ECR Response Code */
	memcpy(&srECROb->srTransData.szECRResponseCode[0], &szDataBuffer[78], 4);
	
	/* Terminal ID */
	memcpy(&srECROb->srTransData.szTerminalID[0], &szDataBuffer[82], 8);
	
	/* SP Origin RRN / 信用卡OriginRRN / ATS電票交易序號 */
	memcpy(&srECROb->srTransData.szField_35[0], &szDataBuffer[90], 12);
	
	/* Exp Amount/pre auth amt/reserve */
	if (memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REDEEM_, 2) == 0		||
	    memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REDEEM_REFUND_, 2) == 0	||
	    memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REDEEM_ADJUST_, 2) == 0)
	{
		memcpy(&srECROb->srTransData.szRDMPoint[0], &szDataBuffer[102], 12);
	}
	else if (memcmp(srECROb->srTransData.szTransType, _ECR_7E1_INSTALLMENT_, 2) == 0	||
		 memcmp(srECROb->srTransData.szTransType, _ECR_7E1_INSTALLMENT_REFUND_, 2) == 0	||
		 memcmp(srECROb->srTransData.szTransType, _ECR_7E1_INSTALLMENT_ADJUST_, 2) == 0)
	{
		memcpy(&srECROb->srTransData.szField_26[0], &szDataBuffer[102], 12);
	}
	else if (memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_POINT_CERTAIN_, 2) == 0	||
		 memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_FULL_REDEEMPTION_, 2) == 0	||
		 memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REDEEM_REFUND_, 2) == 0	||
		 memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REWARD_REFUND_, 2) == 0)
	{
		memcpy(&srECROb->srTransData.szHGRedeemPoint[0], &szDataBuffer[106], 8);
	}
	else
	{
		memcpy(&srECROb->srTransData.szExpAmount[0], &szDataBuffer[102], 12);
	}
	
	/* Store Id */
	memcpy(&srECROb->srTransData.szStoreId[0], &szDataBuffer[114], 18);
	
	/* reserve(10Byte) */
	
	/* 每期金額 Installment Payment Amount (含小數 2位) */
	if (memcmp(srECROb->srTransData.szTransType, _ECR_7E1_INSTALLMENT_, 2) == 0	||
		 memcmp(srECROb->srTransData.szTransType, _ECR_7E1_INSTALLMENT_REFUND_, 2) == 0	||
		 memcmp(srECROb->srTransData.szTransType, _ECR_7E1_INSTALLMENT_ADJUST_, 2) == 0)
	{
		memcpy(&srECROb->srTransData.szField_27[0], &szDataBuffer[132], 10);
	}
	
	/* Start Trans Type */
	if (memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REWARD_SALE_, 2) == 0		||
	    memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REWARD_REDEMPTION_, 2) == 0		||
	    memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REWARD_INSTALLMENT_, 2) == 0	||
	    memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_POINT_CERTAIN_, 2) == 0		||
	    memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_ONLINE_REDEEMPTION_, 2) == 0)
	{
		memcpy(&srECROb->srTransData.szHGPaymentTool[0], &szDataBuffer[142], 2);
	}
	else if (memcmp(srECROb->srTransData.szTransType, _ECR_7E1_START_CARD_NO_INQUIRY_, 2) == 0)
	{
		memcpy(&srECROb->srTransData.szStartTransType[0], &szDataBuffer[142], 2);
	}
	
	return (inRetVal);
}

int inECR_PackEsgData(TRANSACTION_OBJECT *pobTran, int *inPackLen, char *szPackData)
{
	int inLen = *inPackLen;
	int inFixPackLen = 78;
	char szDemoMode[3] = {0};
	
	
	/* 使用前需先判斷是否已開啟 szEnableESG = Y */
	if (pobTran->szEsgSendCR[0] == 'Y' && (pobTran->szAgreeEsgBill[0] == 'Y' || pobTran->szAgreeEsgBill[0] == 'N'))
	{
		/* 是否為機票交易 (1) */
		inLen += 1;
		/* 出發地機場代碼 (5) */
		inLen += 5;
		/* 目的地機場代碼 (5) */
		inLen += 5;
		/* 航班號碼 (5) */
		inLen += 5;

		/* 持卡人同意接收數位帳單 (1) */
		szPackData[inLen] = pobTran->szAgreeEsgBill[0];
		inLen += 1;
		/* [20260616] 新增 ESG Demo 的訊息,因需選擇才會使用,所以不分交易類型都進行複制 */
		inGetDemoMode(szDemoMode);
		if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
		{
			memcpy(&szPackData[inLen], "Edu_training_Test", 17);
		}else
		{
			/* 持卡人數位簽帳單網址GUID (36) */
			if (pobTran->szAgreeEsgBill[0] == 'Y')
			{
				// 安全檢查：限制最多只能複製 36 個字元，防止覆蓋到後面的 Reserved 空間
				int guidLen = strlen(pobTran->szCHESGGuid);
				if (guidLen > 0)
				{
					if (guidLen > 36)
					{
						guidLen = 36;
					}
					memcpy(&szPackData[inLen], pobTran->szCHESGGuid, guidLen);
				}
			}
		}
		inLen += 36;

		/* Reserved (25) */
		inLen += 25;
	} else
	{
		inLen += inFixPackLen;
	}

	*inPackLen = inLen;

	return VS_SUCCESS;
}
