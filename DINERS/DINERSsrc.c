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
#include "../SOURCE/FUNCTION/Accum.h"
#include "../SOURCE/FUNCTION/Function.h"
#include "../SOURCE/FUNCTION/FuncTable.h"
#include "../SOURCE/FUNCTION/CFGT.h"
#include "../SOURCE/FUNCTION/EDC.h"
#include "../SOURCE/FUNCTION/HDT.h"
#include "../SOURCE/FUNCTION/HDPT.h"
#include "../SOURCE/FUNCTION/ECR.h"
#include "../SOURCE/FUNCTION/RS232.h"
#include "../SOURCE/EVENT/MenuMsg.h"
#include "../SOURCE/EVENT/Flow.h"
#include "../SOURCE/COMM/Comm.h"
#include "../CREDIT/Creditfunc.h"
#include "../FISC/NCCCfisc.h"
#include "../EMVSRC/EMVsrc.h"
#include "../NCCC/NCCCsrc.h"
#include "DINERSsrc.h"

extern  int     ginDebug;
extern	char	gszTermVersionID[16 + 1];

int SALE_TRT_DINERS_TABLE[] =
{
	_DINERS_FUNCTION_CHECK_TRANS_FUNCTION_,
        _DINERS_FUNCTION_MUST_SETTLE_CHECK_,
	_FUNCTION_DUPLICATE_CHECK_,
	_COMM_MODEM_PREDIAL_,
        _CREDIT_FUNCTION_GET_AMOUNT_,
        _CREDIT_FUNCTION_GET_STOREID_,
	_CREDIT_FUNCTION_GET_PRODUCT_CODE_,
	_NCCC_FUNCTION_CHECK_SPECIALCARD_FLOW_,
        _DINERS_FUNCTION_SET_TXN_ONLINE_OFFLINE_,
        _DINERS_FUNCTION_BUILD_AND_SEND_PACKET_,
        _UPDATE_ACCUM_,
        _UPDATE_BATCH_,
	_FUNCTION_DUPLICATE_SAVE_,
        _FUNCTION_UPDATE_INV_,
	_FUNCTION_ECR_SEND_TRANSACTION_RESULT_,
	_FUNCTION_GET_SIGNPAD_,
        _FUNCTION_PRINT_RECEIPT_BY_BUFFER_FLOW_,
        _FLOW_NULL_,
};

int VOID_TRT_DINERS_TABLE[] =
{
	_DINERS_FUNCTION_CHECK_TRANS_FUNCTION_,
        _DINERS_FUNCTION_MUST_SETTLE_CHECK_,
	_COMM_MODEM_PREDIAL_,
        _DINERS_FUNCTION_VOID_CHECK_,
        _DINERS_FUNCTION_VOID_CONFIRM_,
	_NCCC_FUNCTION_CHECK_SPECIALCARD_FLOW_,
        _DINERS_FUNCTION_SET_TXN_ONLINE_OFFLINE_,
        _DINERS_FUNCTION_BUILD_AND_SEND_PACKET_,
        _UPDATE_ACCUM_,
        _UPDATE_BATCH_,
	_FUNCTION_ECR_SEND_TRANSACTION_RESULT_,
	_FUNCTION_GET_SIGNPAD_,
        _FUNCTION_PRINT_RECEIPT_BY_BUFFER_FLOW_,
        _FLOW_NULL_,
};

int REFUND_TRT_DINERS_TABLE[] =
{
	_DINERS_FUNCTION_CHECK_TRANS_FUNCTION_,
        _DINERS_FUNCTION_MUST_SETTLE_CHECK_,
	_COMM_MODEM_PREDIAL_,
        _CREDIT_FUNCTION_GET_REFUND_AMOUNT_,
        _CREDIT_FUNCTION_GET_STOREID_,
	_CREDIT_FUNCTION_GET_PRODUCT_CODE_,
	_NCCC_FUNCTION_CHECK_SPECIALCARD_FLOW_,
        _DINERS_FUNCTION_SET_TXN_ONLINE_OFFLINE_,
        _DINERS_FUNCTION_BUILD_AND_SEND_PACKET_,
        _UPDATE_ACCUM_,
        _UPDATE_BATCH_,
        _FUNCTION_UPDATE_INV_,
	_FUNCTION_ECR_SEND_TRANSACTION_RESULT_,
	_FUNCTION_GET_SIGNPAD_,
        _FUNCTION_PRINT_RECEIPT_BY_BUFFER_FLOW_,
        _FLOW_NULL_,
};

int SETTLE_TRT_DINERS_TABLE[] =
{
	_DINERS_FUNCTION_CHECK_TRANS_FUNCTION_,
        _DINERS_FUNCTION_SET_TXN_ONLINE_OFFLINE_,
        _DINERS_FUNCTION_BUILD_AND_SEND_PACKET_,
        _FUNCTION_DELETE_BATCH_FLOW_,
        _FUNCTION_PRINT_TOTAL_REPORT_BY_BUFFER_,
        _FUNCTION_DELETE_ACCUM_FLOW_,
        _FUNCTION_REST_BATCH_INV_,
	_FUNCTION_UPDATE_BATCH_NUM_,
        _FLOW_NULL_,
};

int TIP_TRT_DINERS_TABLE[] =
{
	_DINERS_FUNCTION_CHECK_TRANS_FUNCTION_,
        _DINERS_FUNCTION_MUST_SETTLE_CHECK_,
	_COMM_MODEM_PREDIAL_,
        _DINERS_FUNCTION_TIP_CHECK_,
        _CREDIT_FUNCTION_GET_TIP_AMOUNT_,
	_NCCC_FUNCTION_CHECK_SPECIALCARD_FLOW_,
        _DINERS_FUNCTION_SET_TXN_ONLINE_OFFLINE_,
        _DINERS_FUNCTION_BUILD_AND_SEND_PACKET_,
        _UPDATE_ACCUM_,
        _UPDATE_BATCH_,
	_FUNCTION_ECR_SEND_TRANSACTION_RESULT_,
	_FUNCTION_GET_SIGNPAD_,
        _FUNCTION_PRINT_RECEIPT_BY_BUFFER_FLOW_,
        _FLOW_NULL_,
};

int SALE_OFFLINE_TRT_DINERS_TABLE[] =
{
	_DINERS_FUNCTION_CHECK_TRANS_FUNCTION_,
        _DINERS_FUNCTION_MUST_SETTLE_CHECK_,
	_FUNCTION_DUPLICATE_CHECK_,
	_COMM_MODEM_PREDIAL_,
        _CREDIT_FUNCTION_GET_AMOUNT_,
        _CREDIT_FUNCTION_GET_AUTHCODE_,
        _CREDIT_FUNCTION_GET_STOREID_,
	_CREDIT_FUNCTION_GET_PRODUCT_CODE_,
	_NCCC_FUNCTION_CHECK_SPECIALCARD_FLOW_,
        _DINERS_FUNCTION_SET_TXN_ONLINE_OFFLINE_,
        _DINERS_FUNCTION_BUILD_AND_SEND_PACKET_,
        _UPDATE_ACCUM_,
        _UPDATE_BATCH_,
	_FUNCTION_DUPLICATE_SAVE_,
        _FUNCTION_UPDATE_INV_,
	_FUNCTION_ECR_SEND_TRANSACTION_RESULT_,
	_FUNCTION_GET_SIGNPAD_,
        _FUNCTION_PRINT_RECEIPT_BY_BUFFER_FLOW_,
        _FLOW_NULL_,
};

int ADJUST_TRT_DINERS_TABLE[] =
{
	_DINERS_FUNCTION_CHECK_TRANS_FUNCTION_,
        _DINERS_FUNCTION_MUST_SETTLE_CHECK_,
	_COMM_MODEM_PREDIAL_,
        _DINERS_FUNCTION_ADJUST_CHECK_,
        _CREDIT_FUNCTION_GET_ADJUST_AMOUNT_,
	_NCCC_FUNCTION_CHECK_SPECIALCARD_FLOW_,
        _DINERS_FUNCTION_SET_TXN_ONLINE_OFFLINE_,
        _DINERS_FUNCTION_BUILD_AND_SEND_PACKET_,
        _UPDATE_ACCUM_,
        _UPDATE_BATCH_,
	_FUNCTION_ECR_SEND_TRANSACTION_RESULT_,
	_FUNCTION_GET_SIGNPAD_,
        _FUNCTION_PRINT_RECEIPT_BY_BUFFER_FLOW_,
        _FLOW_NULL_,
};

int PRE_AUTH_TRT_DINERS_TABLE[] =
{
	_DINERS_FUNCTION_CHECK_TRANS_FUNCTION_,
        _DINERS_FUNCTION_MUST_SETTLE_CHECK_,
	_FUNCTION_DUPLICATE_CHECK_,
	_COMM_MODEM_PREDIAL_,
        _CREDIT_FUNCTION_GET_AMOUNT_,
        _CREDIT_FUNCTION_GET_STOREID_,
	_CREDIT_FUNCTION_GET_PRODUCT_CODE_,
	_NCCC_FUNCTION_CHECK_SPECIALCARD_FLOW_,
        _DINERS_FUNCTION_SET_TXN_ONLINE_OFFLINE_,
        _DINERS_FUNCTION_BUILD_AND_SEND_PACKET_,
	_FUNCTION_DUPLICATE_SAVE_,
        _FUNCTION_UPDATE_INV_,
	_FUNCTION_ECR_SEND_TRANSACTION_RESULT_,
	_FUNCTION_GET_SIGNPAD_,
        _FUNCTION_PRINT_RECEIPT_BY_BUFFER_FLOW_,
        _FLOW_NULL_,
};

TRT_TABLE  TRANSACTION_DINERS_TABLE[] =
{
        {_TRT_SALE_,        SALE_TRT_DINERS_TABLE},
        {_TRT_VOID_,        VOID_TRT_DINERS_TABLE},
        {_TRT_REFUND_,      REFUND_TRT_DINERS_TABLE},
        {_TRT_SETTLE_,      SETTLE_TRT_DINERS_TABLE},
        {_TRT_TIP_,         TIP_TRT_DINERS_TABLE},
        {_TRT_SALE_OFFLINE_,SALE_OFFLINE_TRT_DINERS_TABLE},
        {_TRT_ADJUST_,      ADJUST_TRT_DINERS_TABLE},
        {_TRT_PRE_AUTH_,    PRE_AUTH_TRT_DINERS_TABLE},
        {_FLOW_NULL_,                 NULL}        
};

int inDINERS_RunTRT(TRANSACTION_OBJECT *pobTran, int inTRTCode)
{
        int     *inTRTID = NULL;
        int     i, inRetVal = VS_ERROR ;

        for (i = 0 ;; i ++)
        {
                if (TRANSACTION_DINERS_TABLE[i].inTRTCode == inTRTCode)
                {
                        inTRTID = TRANSACTION_DINERS_TABLE[i].inTRTID;
                        break;
                }
                else if (TRANSACTION_DINERS_TABLE[i].inTRTCode == -1)
                        break;
        }

        if (inTRTID == NULL)
        {
                return (VS_ERROR);
        }

        for (i = 0 ; ; i ++)
        {
                if (inTRTID[i] == VS_ERROR)
                        break;

                inRetVal = inFLOW_RunFunction(pobTran, inTRTID[i]);

                if (inRetVal != VS_SUCCESS)
                {
			/* 只有出錯才需SendError */
			if (pobTran->inECRErrorMsg != _ECR_RESPONSE_CODE_NOT_SET_ERROR_)
			{
				inECR_SendError(pobTran, pobTran->inECRErrorMsg);
			}
			else
			{
				inECR_SendError(pobTran, inRetVal);
			}
                        break;
                }
        }
	
	/* 斷線 */
	inCOMM_End(pobTran);
	
	/* 要先回傳再顯示錯誤訊息 */
	inFunc_Display_Error(pobTran);
			
	/* 退回晶片卡 */
	if (inRetVal != VS_SUCCESS)
	{
		if (memcmp(gszTermVersionID, "MD731UAGAS001", strlen("MD731UAGAS001")) == 0)
		{
			
		}
		else
		{
			inNCCC_Func_Check_Card_Still_Exist_Flow(pobTran, _REMOVE_CARD_ERROR_);
		}
	}
	else
	{
		if (memcmp(gszTermVersionID, "MD731UAGAS001", strlen("MD731UAGAS001")) == 0)
		{
			
		}
		else
		{
			inNCCC_Func_Check_Card_Still_Exist_Flow(pobTran, _REMOVE_CARD_NOT_ERROR_);
		}
	}
	
        return (inRetVal);
}

/*
Function        :inDINERS_Func_VOID_Confirm
Date&Time       :2015/10/22 下午 04:00
Describe        :按0確認是否要進行取消交易
*/
int inDINERS_Func_VOID_Confirm(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;
	char    szAuthCodeMsg[_DISP_MSG_SIZE_ + 1];
        char    szAmountMsg[_DISP_MSG_SIZE_ + 1];
        char    szKey;

        /* 確認金額 */
        if (pobTran->srBRec.lnTotalTxnAmount <= 0)
                return (VS_ERROR);

        memset(szAuthCodeMsg, 0x00, sizeof(szAuthCodeMsg));
        memset(szAmountMsg, 0x00, sizeof(szAmountMsg));

        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        /* 提示檢核簽名和授權碼 */
        inDISP_PutGraphic(_CHECK_AUTH_AMT_, 0, _COORDINATE_Y_LINE_8_4_);
        /* 顯示授權碼 */
        sprintf(szAuthCodeMsg, "%s", pobTran->srBRec.szAuthCode);
        inDISP_EnglishFont_Point_Color(szAuthCodeMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _COLOR_WHITE_, 11);

        /* 顯示金額 */
        if (pobTran->srBRec.inOrgCode == _REFUND_)
	{
        sprintf(szAmountMsg, "%ld", 0 - pobTran->srBRec.lnTotalTxnAmount);
	inFunc_Amount_Comma(szAmountMsg, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	}
        else
	{
                sprintf(szAmountMsg, "%ld", pobTran->srBRec.lnTotalTxnAmount);
		inFunc_Amount_Comma(szAmountMsg, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	}

        inDISP_EnglishFont_Point_Color(szAmountMsg, _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_RED_,_COLOR_WHITE_, 8);

        while (1)
	{
		szKey = uszKBD_GetKey(_EDC_TIMEOUT_);

		if (szKey == _KEY_0_)
		{
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
	
	if (inRetVal == VS_SUCCESS)
	{
		pobTran->uszUpdateBatchBit = VS_TRUE;
		pobTran->srBRec.uszVOIDBit = VS_TRUE;
		pobTran->srBRec.inOrgCode = pobTran->srBRec.inCode;
		pobTran->srBRec.inCode = pobTran->inTransactionCode;
	}
	else
	{
		
	}

        return (inRetVal);
}

/*
Function        :inDINERS_Func_ADJUST_Confirm
Date&Time       :2015/10/22 下午 04:00
Describe        :確認是否能進行取消交易
*/
int inDINERS_Func_VOID_Check(TRANSACTION_OBJECT *pobTran)
{
        if (pobTran->srBRec.uszVOIDBit == VS_TRUE)
        {
		/* 此交易已取消 */
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, _ERR_CANCEL_ALREADY_);
		srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
		srDispMsgObj.inMsgType = _ENTER_KEY_MSG_;
		srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
		strcpy(srDispMsgObj.szErrMsg1, "");
		srDispMsgObj.inErrMsg1Line = 0;
		srDispMsgObj.inBeepTimes = 1;
		srDispMsgObj.inBeepInterval = 0;
			
                inDISP_Msg_BMP(&srDispMsgObj);
		
		return (VS_ERROR);
        }
        else
                return (VS_SUCCESS);
}

/*
Function        :inDINERS_Func_TIP_Confirm
Date&Time       :2015/10/22 下午 04:00
Describe        :確認是否能進行小費交易
*/
int inDINERS_Func_TIP_Check(TRANSACTION_OBJECT *pobTran)
{
        if (pobTran->srBRec.inCode == _TIP_    ||
            pobTran->srBRec.inCode == _VOID_   ||
            pobTran->srBRec.inCode == _REFUND_ ||
            pobTran->srBRec.inCode == _ADJUST_ )
        {
		/* 不能做小費 */
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, _ERR_TIP_);
		srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
		srDispMsgObj.inMsgType = _ENTER_KEY_MSG_;
		srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
		strcpy(srDispMsgObj.szErrMsg1, "");
		srDispMsgObj.inErrMsg1Line = 0;
		srDispMsgObj.inBeepTimes = 1;
		srDispMsgObj.inBeepInterval = 0;
			
                inDISP_Msg_BMP(&srDispMsgObj);
		
		return (VS_ERROR);
        }
        else
                return (VS_SUCCESS);
        
}

/*
Function        :inDINERS_Func_ADJUST_Confirm
Date&Time       :2015/10/22 下午 04:00
Describe        :確認是否能進行調帳交易
*/
int inDINERS_Func_ADJUST_Check(TRANSACTION_OBJECT *pobTran)
{
        if (pobTran->srBRec.inCode == _TIP_    ||
            pobTran->srBRec.inCode == _VOID_   ||
            pobTran->srBRec.inCode == _REFUND_ ||
            pobTran->srBRec.inCode == _ADJUST_ )
        {
		/* 不能做調帳 */
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, _ERR_ADJUST_);
		srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
		srDispMsgObj.inMsgType = _ENTER_KEY_MSG_;
		srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
		strcpy(srDispMsgObj.szErrMsg1, "");
		srDispMsgObj.inErrMsg1Line = 0;
		srDispMsgObj.inBeepTimes = 1;
		srDispMsgObj.inBeepInterval = 0;
			
                inDISP_Msg_BMP(&srDispMsgObj);
		
		return (VS_ERROR);
        }
        else
                return (VS_SUCCESS);
}

/*
Function        :inDINERS_Func_Must_SETTLE
Date&Time       :2015/10/22 下午 04:00
Describe        :確認是否要先結帳
*/
int inDINERS_Func_Must_SETTLE(TRANSACTION_OBJECT *pobTran)
{
        char	szMustSettleBit[2 + 1] = {0};

        memset(szMustSettleBit, 0x00, sizeof(szMustSettleBit));

        if (inGetMustSettleBit(szMustSettleBit) == VS_ERROR)
        {
                return (VS_ERROR);
        }

        if (!memcmp(szMustSettleBit, "Y", 1))
        {
                /* 表示要結帳 */
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, _ERR_MUST_SETTLE_);
		srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
		srDispMsgObj.inMsgType = _ENTER_KEY_MSG_;
		srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
		strcpy(srDispMsgObj.szErrMsg1, _HOST_NAME_DINERS_);
		srDispMsgObj.inErrMsg1Line = _LINE_8_5_;
		srDispMsgObj.inBeepTimes = 1;
		srDispMsgObj.inBeepInterval = 0;
			
		inDISP_Msg_BMP(&srDispMsgObj);
		
		return (VS_ERROR);
        }
        else
                return (VS_SUCCESS);
}

/*
Function        :inDINERS_Func_CheckTransactionFunction
Date&Time       :2016/10/26 下午 3:40
Describe        :確認交易功能是否打開，因為根據Host，所以每個Host一個Function
*/
int inDINERS_Func_CheckTransactionFunction(TRANSACTION_OBJECT *pobTran)
{
	char		szTransFunc[20 + 1];
	unsigned char	uszTxnEnable = VS_TRUE;
	
	memset(szTransFunc, 0x00, sizeof(szTransFunc));
	if (inGetTransFunc(szTransFunc) != VS_SUCCESS)
		return (VS_ERROR);
	
	if (pobTran->inTransactionCode == _SALE_)
	{
		if (memcmp(&szTransFunc[0], "Y", 1) != 0)
		{
			uszTxnEnable = VS_FALSE;
		}
	}
	else if (pobTran->inTransactionCode == _VOID_)
	{
		if (memcmp(&szTransFunc[1], "Y", 1) != 0)
		{
			uszTxnEnable = VS_FALSE;
		}
	}
	else if (pobTran->inTransactionCode == _SETTLE_)
	{
		if (memcmp(&szTransFunc[2], "Y", 1) != 0)
		{
			uszTxnEnable = VS_FALSE;
		}
	}
	else if (pobTran->inTransactionCode == _REFUND_)
	{
		if (memcmp(&szTransFunc[3], "Y", 1) != 0)
		{
			uszTxnEnable = VS_FALSE;
		}
	}
	/* 分期退貨要看兩個開關 */
	else if (pobTran->inTransactionCode == _INST_REFUND_)
	{
		if ((memcmp(&szTransFunc[3], "Y", 1) != 0) || (memcmp(&szTransFunc[7], "Y", 1) != 0))
		{
			uszTxnEnable = VS_FALSE;
		}
	}
	else if (pobTran->inTransactionCode == _PRE_AUTH_	||
		 pobTran->inTransactionCode == _PRE_COMP_)
	{
		if (memcmp(&szTransFunc[5], "Y", 1) != 0)
		{
			uszTxnEnable = VS_FALSE;
		}
	}
	else if (pobTran->inTransactionCode == _TIP_)
	{
		if (memcmp(&szTransFunc[6], "Y", 1) != 0)
		{
			uszTxnEnable = VS_FALSE;
		}
	}
	
	/* 此功能已關閉 */
	if (uszTxnEnable == VS_FALSE)
	{
		pobTran->inErrorMsg = _ERROR_CODE_V3_FUNC_CLOSE_;
		
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inDINERS_Func_Display_Review_Settle
Date&Time       :2018/1/19 下午 4:03
Describe        :
*/
int inDINERS_Func_Display_Review_Settle(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_SUCCESS;
	int		inChoice = 0;
	int		inTouchSensorFunc = _Touch_BATCH_END_;
	char		szTemplate1[44 + 1] = {0};
	char		szTemplate2[44 + 1] = {0};
	char		szDispBuffer1[44 + 1] = {0};
	char		szDispBuffer2[44 + 1] = {0};
	long long	llSum = 0;
	unsigned char	uszKey = 0x00;
	ACCUM_TOTAL_REC	srAccumRec = {0};
	
	/* 讀交易資料，並放交易查詢Title */
	memset(&srAccumRec, 0x00, sizeof(ACCUM_TOTAL_REC));
	inACCUM_GetRecord(pobTran, &srAccumRec);
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_PutGraphic(_REVIEW_SETTLE_, 0, _COORDINATE_Y_LINE_8_4_);
	
	/* 銷售筆數 */
	memset(szDispBuffer1, 0x00, sizeof(szDispBuffer1));
	memset(szTemplate1, 0x00, sizeof(szTemplate1));

	sprintf(szTemplate1, "%03ld", srAccumRec.lnDinersTotalSaleCount);
	sprintf(szDispBuffer1, "%s", szTemplate1);
	inDISP_EnglishFont_Point_Color(szDispBuffer1, _FONTSIZE_8X22_, _LINE_8_4_, _COLOR_BLACK_,_COLOR_WHITE_, 7);
	
	/* 銷售金額 */
	memset(szDispBuffer2, 0x00, sizeof(szDispBuffer2));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));

	llSum = srAccumRec.llDinersTotalSaleAmount;
	if (abs(llSum) < 100000000)
	{
		sprintf(szTemplate2, "%01lld", llSum);
		inFunc_Amount_Comma(szTemplate2, "$", ' ', _SIGNED_NONE_, 11, _PADDING_LEFT_);
		sprintf(szDispBuffer2, "%s", szTemplate2);
		inDISP_EnglishFont_Point_Color(szDispBuffer2, _FONTSIZE_8X22_, _LINE_8_4_, _COLOR_BLACK_,_COLOR_WHITE_, 10);
	}
	else
	{
		sprintf(szTemplate2, "%01lld", llSum);
		inFunc_Amount_Comma(szTemplate2, "$", ' ', _SIGNED_NONE_, 14, _PADDING_LEFT_);
		sprintf(szDispBuffer2, "%s", szTemplate2);
		inDISP_EnglishFont_Point_Color(szDispBuffer2, _FONTSIZE_8X33_, _LINE_8_4_, _COLOR_BLACK_,_COLOR_WHITE_, 13);
	}

	/* 退貨筆數 */
	memset(szDispBuffer1, 0x00, sizeof(szDispBuffer1));
	memset(szTemplate1, 0x00, sizeof(szTemplate1));

	sprintf(szTemplate1, "%03ld", srAccumRec.lnDinersTotalRefundCount);
	sprintf(szDispBuffer1, "%s", szTemplate1);
	inDISP_EnglishFont_Point_Color(szDispBuffer1, _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_,_COLOR_WHITE_, 7);
	
	/* 退貨金額 */
	memset(szDispBuffer2, 0x00, sizeof(szDispBuffer2));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));

	llSum = (0 - srAccumRec.llDinersTotalRefundAmount);
	if (abs(llSum) < 100000000)
	{
		sprintf(szTemplate2, "%01lld", (0 - llSum));
		inFunc_Amount_Comma(szTemplate2, "$", ' ', _SIGNED_NONE_, 11, _PADDING_LEFT_);
		sprintf(szDispBuffer2, "%s", szTemplate2);
		inDISP_EnglishFont_Point_Color(szDispBuffer2, _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_,_COLOR_WHITE_, 10);
	}
	else
	{
		sprintf(szTemplate2, "%01lld", (0 - llSum));
		inFunc_Amount_Comma(szTemplate2, "$", ' ', _SIGNED_NONE_, 14, _PADDING_LEFT_);
		sprintf(szDispBuffer2, "%s", szTemplate2);
		inDISP_EnglishFont_Point_Color(szDispBuffer2, _FONTSIZE_8X33_, _LINE_8_5_, _COLOR_BLACK_,_COLOR_WHITE_, 13);
	}

	/* 淨額筆數 */
	memset(szDispBuffer1, 0x00, sizeof(szDispBuffer1));
	memset(szTemplate1, 0x00, sizeof(szTemplate1));

	sprintf(szTemplate1, "%03ld", srAccumRec.lnDinersTotalCount);
	sprintf(szDispBuffer1, "%s", szTemplate1);
	inDISP_EnglishFont_Point_Color(szDispBuffer1, _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_BLACK_,_COLOR_WHITE_, 7);
	/* 淨額金額 */
	memset(szDispBuffer2, 0x00, sizeof(szDispBuffer2));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));

	llSum = srAccumRec.llDinersTotalAmount;
	if (abs(llSum) < 100000000)
	{
		sprintf(szTemplate2, "%01lld", llSum);
		inFunc_Amount_Comma(szTemplate2, "$", ' ', _SIGNED_NONE_, 11, _PADDING_LEFT_);
		sprintf(szDispBuffer2, "%s", szTemplate2);
		inDISP_EnglishFont_Point_Color(szDispBuffer2, _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_BLACK_,_COLOR_WHITE_, 10);
	}
	else
	{
		sprintf(szTemplate2, "%01lld", llSum);
		inFunc_Amount_Comma(szTemplate2, "$", ' ', _SIGNED_NONE_, 14, _PADDING_LEFT_);
		sprintf(szDispBuffer2, "%s", szTemplate2);
		inDISP_EnglishFont_Point_Color(szDispBuffer2, _FONTSIZE_8X33_, _LINE_8_6_, _COLOR_BLACK_,_COLOR_WHITE_, 13);
	}
	
	inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
	
	uszKey = 0x00;
	while (1)
	{
		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
		uszKey = uszKBD_Key();

		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			uszKey = _KEY_TIMEOUT_;
		}

		if (uszKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else if (uszKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}
		else if (inChoice == _BATCH_END_Touch_ENTER_BUTTON_	||
			 uszKey == _KEY_0_)
		{
			inRetVal = VS_SUCCESS;
			break;
		}
	}
	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();
	
	return (inRetVal);
}

/*
Function        :inDINERS_Func_Display_Review
Date&Time       :2018/1/26 下午 5:29
Describe        :
*/
int inDINERS_Func_Display_Review(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_SUCCESS;
	int		inChoice = 0;
	int		inTouchSensorFunc = _Touch_NEWUI_REVIEW_TOTAL_;
	char		szTemplate1[44 + 1] = {0};
	char		szTemplate2[44 + 1] = {0};
	char		szDispBuffer1[44 + 1] = {0};
	char		szDispBuffer2[44 + 1] = {0};
	long long	llSum = 0;
	unsigned char	uszKey = 0x00;
	ACCUM_TOTAL_REC	srAccumRec = {0};
	
	/* 讀交易資料，並放交易查詢Title */
	memset(&srAccumRec, 0x00, sizeof(ACCUM_TOTAL_REC));
	inACCUM_GetRecord(pobTran, &srAccumRec);
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_PutGraphic(_REVIEW_TOTAL_2_, 0, _COORDINATE_Y_LINE_8_4_);
	
	/* 銷售筆數 */
	memset(szDispBuffer1, 0x00, sizeof(szDispBuffer1));
	memset(szTemplate1, 0x00, sizeof(szTemplate1));

	sprintf(szTemplate1, "%03ld", srAccumRec.lnDinersTotalSaleCount);
	sprintf(szDispBuffer1, "%s", szTemplate1);
	inDISP_EnglishFont_Point_Color(szDispBuffer1, _FONTSIZE_8X22_, _LINE_8_4_, _COLOR_BLACK_,_COLOR_WHITE_, 7);
	
	/* 銷售金額 */
	memset(szDispBuffer2, 0x00, sizeof(szDispBuffer2));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));

	llSum = srAccumRec.llDinersTotalSaleAmount;
	if (abs(llSum) < 100000000)
	{
		sprintf(szTemplate2, "%01lld", llSum);
		inFunc_Amount_Comma(szTemplate2, "$", ' ', _SIGNED_NONE_, 11, _PADDING_LEFT_);
		sprintf(szDispBuffer2, "%s", szTemplate2);
		inDISP_EnglishFont_Point_Color(szDispBuffer2, _FONTSIZE_8X22_, _LINE_8_4_, _COLOR_BLACK_,_COLOR_WHITE_, 10);
	}
	else
	{
		sprintf(szTemplate2, "%01lld", llSum);
		inFunc_Amount_Comma(szTemplate2, "$", ' ', _SIGNED_NONE_, 14, _PADDING_LEFT_);
		sprintf(szDispBuffer2, "%s", szTemplate2);
		inDISP_EnglishFont_Point_Color(szDispBuffer2, _FONTSIZE_8X33_, _LINE_8_4_, _COLOR_BLACK_,_COLOR_WHITE_, 13);
	}
	/* 退貨筆數 */
	memset(szDispBuffer1, 0x00, sizeof(szDispBuffer1));
	memset(szTemplate1, 0x00, sizeof(szTemplate1));

	sprintf(szTemplate1, "%03ld", srAccumRec.lnDinersTotalRefundCount);
	sprintf(szDispBuffer1, "%s", szTemplate1);
	inDISP_EnglishFont_Point_Color(szDispBuffer1, _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_,_COLOR_WHITE_, 7);
	
	/* 退貨金額 */
	memset(szDispBuffer2, 0x00, sizeof(szDispBuffer2));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));

	llSum = (0 - srAccumRec.llDinersTotalRefundAmount);
	if (abs(llSum) < 100000000)
	{
		sprintf(szTemplate2, "%01lld", (0 - llSum));
		inFunc_Amount_Comma(szTemplate2, "$", ' ', _SIGNED_NONE_, 11, _PADDING_LEFT_);
		sprintf(szDispBuffer2, "%s", szTemplate2);
		inDISP_EnglishFont_Point_Color(szDispBuffer2, _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_,_COLOR_WHITE_, 10);
	}
	else
	{
		sprintf(szTemplate2, "%01lld", (0 - llSum));
		inFunc_Amount_Comma(szTemplate2, "$", ' ', _SIGNED_NONE_, 14, _PADDING_LEFT_);
		sprintf(szDispBuffer2, "%s", szTemplate2);
		inDISP_EnglishFont_Point_Color(szDispBuffer2, _FONTSIZE_8X33_, _LINE_8_5_, _COLOR_BLACK_,_COLOR_WHITE_, 13);
	}

	/* 淨額筆數 */
	memset(szDispBuffer1, 0x00, sizeof(szDispBuffer1));
	memset(szTemplate1, 0x00, sizeof(szTemplate1));

	sprintf(szTemplate1, "%03ld", srAccumRec.lnDinersTotalCount);
	sprintf(szDispBuffer1, "%s", szTemplate1);
	inDISP_EnglishFont_Point_Color(szDispBuffer1, _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_BLACK_,_COLOR_WHITE_, 7);

	/* 淨額金額 */
	memset(szDispBuffer2, 0x00, sizeof(szDispBuffer2));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));

	llSum = srAccumRec.llDinersTotalAmount;
	if (abs(llSum) < 100000000)
	{
		sprintf(szTemplate2, "%01lld", llSum);
		inFunc_Amount_Comma(szTemplate2, "$", ' ', _SIGNED_NONE_, 11, _PADDING_LEFT_);
		sprintf(szDispBuffer2, "%s", szTemplate2);
		inDISP_EnglishFont_Point_Color(szDispBuffer2, _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_BLACK_,_COLOR_WHITE_, 10);
	}
	else
	{
		sprintf(szTemplate2, "%01lld", llSum);
		inFunc_Amount_Comma(szTemplate2, "$", ' ', _SIGNED_NONE_, 14, _PADDING_LEFT_);
		sprintf(szDispBuffer2, "%s", szTemplate2);
		inDISP_EnglishFont_Point_Color(szDispBuffer2, _FONTSIZE_8X33_, _LINE_8_6_, _COLOR_BLACK_,_COLOR_WHITE_, 13);
	}
	
	uszKey = 0x00;
	inTouchSensorFunc = _Touch_NEWUI_REVIEW_TOTAL_;
	inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
	
	while (1)
	{
		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
		uszKey = uszKBD_Key();
		
		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			uszKey = _KEY_TIMEOUT_;
		}

		if (inChoice == _NEWUI_REVIEW_TOTAL_Touch_RETURN_IDLE_BUTTON_	||
		    uszKey == _KEY_ENTER_					|| 
		    uszKey == _KEY_CANCEL_					||
		    uszKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_SUCCESS;
			break;
		}
		else if (inChoice == _NEWUI_REVIEW_TOTAL_Touch_LAST_PAGE_BUTTON_	||
			 uszKey == _KEY_CLEAR_)
		{
			inRetVal = VS_LAST_PAGE;
			pobTran->srBRec.inHDTIndex = -1;
			break;
		}
	}
	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();
	
	return (inRetVal);
}
