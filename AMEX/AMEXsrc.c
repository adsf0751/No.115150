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
#include "../SOURCE/FUNCTION/Sqlite.h"
#include "../SOURCE/FUNCTION/Function.h"
#include "../SOURCE/FUNCTION/FuncTable.h"
#include "../SOURCE/FUNCTION/CDT.h"
#include "../SOURCE/FUNCTION/CFGT.h"
#include "../SOURCE/FUNCTION/EDC.h"
#include "../SOURCE/FUNCTION/HDPT.h"
#include "../SOURCE/FUNCTION/ECR.h"
#include "../SOURCE/EVENT/MenuMsg.h"
#include "../SOURCE/EVENT/Flow.h"
#include "../SOURCE/COMM/Comm.h"
#include "../CREDIT/Creditfunc.h"
#include "../FISC/NCCCfisc.h"
#include "../EMVSRC/EMVsrc.h"
#include "../NCCC/NCCCsrc.h"
#include "AMEXsrc.h"

extern  int     ginDebug;  /* Debug使用 extern */
extern	char	gszTermVersionID[16 + 1];

int SALE_TRT_AMEX_TABLE[] =
{
        _AMEX_FUNCTION_MUST_SETTLE_CHECK_,
	_FUNCTION_DUPLICATE_CHECK_,
        _AMEX_FUNCTION_GET_4DBC_,
        _CREDIT_FUNCTION_GET_AMOUNT_,
        _CREDIT_FUNCTION_GET_STOREID_,
        _AMEX_FUNCTION_SET_TXN_ONLINE_OFFLINE_,
        _AMEX_FUNCTION_BUILD_AND_SEND_PACKET_,
        _UPDATE_ACCUM_,
        _UPDATE_BATCH_,
	_FUNCTION_DUPLICATE_SAVE_,
        _FUNCTION_UPDATE_INV_,
	_FUNCTION_ECR_SEND_TRANSACTION_RESULT_,
	_FUNCTION_GET_SIGNPAD_,
        _FUNCTION_PRINT_RECEIPT_BY_BUFFER_FLOW_,
        _FLOW_NULL_ ,
};

int VOID_TRT_AMEX_TABLE[] =
{
        _AMEX_FUNCTION_MUST_SETTLE_CHECK_,
        _AMEX_FUNCTION_VOID_CHECK_,
        _AMEX_FUNCTION_VOID_CONFIRM_,
        _AMEX_FUNCTION_SET_TXN_ONLINE_OFFLINE_,
	_AMEX_FUNCTION_BUILD_AND_SEND_PACKET_,
        _UPDATE_ACCUM_,
        _UPDATE_BATCH_,
	_FUNCTION_ECR_SEND_TRANSACTION_RESULT_,
	_FUNCTION_GET_SIGNPAD_,
        _FUNCTION_PRINT_RECEIPT_BY_BUFFER_FLOW_,
        _FLOW_NULL_ ,
};

int REFUND_TRT_AMEX_TABLE[] =
{
        _AMEX_FUNCTION_MUST_SETTLE_CHECK_,
        _CREDIT_FUNCTION_GET_REFUND_AMOUNT_,
        _CREDIT_FUNCTION_GET_STOREID_,
        _AMEX_FUNCTION_SET_TXN_ONLINE_OFFLINE_,
	_AMEX_FUNCTION_BUILD_AND_SEND_PACKET_,
        _UPDATE_ACCUM_,
        _UPDATE_BATCH_,
        _FUNCTION_UPDATE_INV_,
	_FUNCTION_ECR_SEND_TRANSACTION_RESULT_,
	_FUNCTION_GET_SIGNPAD_,
        _FUNCTION_PRINT_RECEIPT_BY_BUFFER_FLOW_,
        _FLOW_NULL_ ,
};

int SETTLE_TRT_AMEX_TABLE[] =
{
        _AMEX_FUNCTION_SET_TXN_ONLINE_OFFLINE_,
	_AMEX_FUNCTION_BUILD_AND_SEND_PACKET_,
        _FUNCTION_DELETE_BATCH_FLOW_,
        _FUNCTION_PRINT_TOTAL_REPORT_BY_BUFFER_,
        _FUNCTION_DELETE_ACCUM_FLOW_,
        _FUNCTION_REST_BATCH_INV_,
	_FUNCTION_UPDATE_BATCH_NUM_,
        _FLOW_NULL_ ,
};

int TIP_TRT_AMEX_TABLE[] =
{
        _AMEX_FUNCTION_MUST_SETTLE_CHECK_,
        _AMEX_FUNCTION_TIP_CHECK_,
        _CREDIT_FUNCTION_GET_TIP_AMOUNT_,
        _AMEX_FUNCTION_SET_TXN_ONLINE_OFFLINE_,
	_AMEX_FUNCTION_BUILD_AND_SEND_PACKET_,
        _UPDATE_ACCUM_,
        _UPDATE_BATCH_,
	_FUNCTION_ECR_SEND_TRANSACTION_RESULT_,
	_FUNCTION_GET_SIGNPAD_,
        _FUNCTION_PRINT_RECEIPT_BY_BUFFER_FLOW_,
        _FLOW_NULL_ ,
};

int SALE_OFFLINE_TRT_AMEX_TABLE[] =
{
        _AMEX_FUNCTION_MUST_SETTLE_CHECK_,
	_FUNCTION_DUPLICATE_CHECK_,
        _CREDIT_FUNCTION_GET_AMOUNT_,
        _CREDIT_FUNCTION_GET_AUTHCODE_,
        _CREDIT_FUNCTION_GET_STOREID_,
        _AMEX_FUNCTION_SET_TXN_ONLINE_OFFLINE_,
	_AMEX_FUNCTION_BUILD_AND_SEND_PACKET_,
        _UPDATE_ACCUM_,
        _UPDATE_BATCH_,
	_FUNCTION_DUPLICATE_SAVE_,
        _FUNCTION_UPDATE_INV_,
	_FUNCTION_ECR_SEND_TRANSACTION_RESULT_,
	_FUNCTION_GET_SIGNPAD_,
        _FUNCTION_PRINT_RECEIPT_BY_BUFFER_FLOW_,
        _FLOW_NULL_ ,
};

int ADJUST_TRT_AMEX_TABLE[] =
{
        _AMEX_FUNCTION_MUST_SETTLE_CHECK_,
        _AMEX_FUNCTION_ADJUST_CHECK_,
        _CREDIT_FUNCTION_GET_ADJUST_AMOUNT_,
        _AMEX_FUNCTION_SET_TXN_ONLINE_OFFLINE_,
	_AMEX_FUNCTION_BUILD_AND_SEND_PACKET_,
        _UPDATE_ACCUM_,
        _UPDATE_BATCH_,
	_FUNCTION_ECR_SEND_TRANSACTION_RESULT_,
	_FUNCTION_GET_SIGNPAD_,
        _FUNCTION_PRINT_RECEIPT_BY_BUFFER_FLOW_,
        _FLOW_NULL_ ,
};

int PRE_AUTH_TRT_AMEX_TABLE[] =
{
        _AMEX_FUNCTION_MUST_SETTLE_CHECK_,
	_FUNCTION_DUPLICATE_CHECK_,
        _AMEX_FUNCTION_GET_4DBC_,
        _CREDIT_FUNCTION_GET_AMOUNT_,
        _CREDIT_FUNCTION_GET_STOREID_,
        _AMEX_FUNCTION_SET_TXN_ONLINE_OFFLINE_,
	_AMEX_FUNCTION_BUILD_AND_SEND_PACKET_,
	_FUNCTION_DUPLICATE_SAVE_,
        _FUNCTION_UPDATE_INV_,
	_FUNCTION_ECR_SEND_TRANSACTION_RESULT_,
	_FUNCTION_GET_SIGNPAD_,
        _FUNCTION_PRINT_RECEIPT_BY_BUFFER_FLOW_,
        _FLOW_NULL_ ,
};

TRT_TABLE TRANSACTION_AMEX_TABLE[] =
{
        {_TRT_SALE_,            SALE_TRT_AMEX_TABLE},
        {_TRT_VOID_,            VOID_TRT_AMEX_TABLE},
        {_TRT_REFUND_,          REFUND_TRT_AMEX_TABLE},
        {_TRT_SETTLE_,          SETTLE_TRT_AMEX_TABLE},
        {_TRT_TIP_,             TIP_TRT_AMEX_TABLE},
        {_TRT_SALE_OFFLINE_,    SALE_OFFLINE_TRT_AMEX_TABLE},
        {_TRT_ADJUST_,          ADJUST_TRT_AMEX_TABLE},
        {_TRT_PRE_AUTH_,        PRE_AUTH_TRT_AMEX_TABLE},
        {_FLOW_NULL_ ,          NULL}
};

int inAMEX_RunTRT(TRANSACTION_OBJECT *pobTran, int inTRTCode)
{
        int     *inTRTID = NULL;
        int     i, inRetVal = VS_ERROR ;

        for (i = 0 ;; i ++)
        {
                if (TRANSACTION_AMEX_TABLE[i].inTRTCode == inTRTCode)
                {
                        inTRTID = TRANSACTION_AMEX_TABLE[i].inTRTID;
                        break;
                }
                else if (TRANSACTION_AMEX_TABLE[i].inTRTCode == -1)
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
Function        :inAMEX_Func_Get4DBC
Date&Time       :2015/9/11 上午 10:51
Describe        :輸入AE卡的4DBC
*/
int inAMEX_Func_Get4DBC(TRANSACTION_OBJECT *pobTran)
{
        int		inRetVal;
        char		szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];
        char		sz4DBCEnable [1 + 1];
        DISPLAY_OBJECT  srDispObj;

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inAMEX_Func_Get4DBC()_START");
        }

        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));

        /* 不是AE卡 return */
        if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_AMEX_, 4))
        {
                if (ginDebug == VS_TRUE)
                {
                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        sprintf(szErrorMsg, "CardLabel = %s", pobTran->srBRec.szCardLabel);
                        inLogPrintf(AT, szErrorMsg);
                }

                return (VS_SUCCESS);
        }

        /* CDT 4DBC開關 */
        memset(sz4DBCEnable, 0x00, sizeof(sz4DBCEnable));
        inGet4DBCEnable(sz4DBCEnable);

        if (sz4DBCEnable[0] == 'N')
        {
                if (ginDebug == VS_TRUE)
                {
                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                        sprintf(szErrorMsg, "4DBCEnable == OFF ");
                        inLogPrintf(AT, szErrorMsg);
                }

                return (VS_SUCCESS);
        }
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_PutGraphic(_GET_AMEX_4DBC_, 0, _COORDINATE_Y_LINE_8_4_);
                
        while (1)
        {
                srDispObj.inY = _LINE_8_4_;
                srDispObj.inR_L = _DISP_RIGHT_;
                srDispObj.inMaxLen = 4;
		srDispObj.inColor = _COLOR_RED_;

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;
	
                /* 鍵盤輸入4碼 4DBC */
                inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);

                if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
                                return (VS_ERROR);

                if (srDispObj.inOutputLen == 4)
                {
                        memset(pobTran->srBRec.szAMEX4DBC, 0x00, sizeof(pobTran->srBRec.szAMEX4DBC));
                        memcpy(&pobTran->srBRec.szAMEX4DBC[0], &srDispObj.szOutput[0], srDispObj.inOutputLen);
                        break;
                }
                else
                {
                        inDISP_BEEP(1, 0);
                        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                        continue;
                }
        }

        if (ginDebug == VS_TRUE)
        {
                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                sprintf(szErrorMsg, "inAMEX_Func_Get4DBC()_[%s]_END", pobTran->srBRec.szAMEX4DBC);
                inLogPrintf(AT, szErrorMsg);
        }

        return (VS_SUCCESS);
}

/*
Function        :inAMEX_Func_VOID_Confirm
Date&Time       :2015/10/22 下午 04:00
Describe        :按0確認是否要進行取消交易
*/
int inAMEX_Func_VOID_Confirm(TRANSACTION_OBJECT *pobTran)
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

        /* pobTran->uszUpdateBatchBit 表示 uszUpdateBatchBit / TRANS_BATCH_KEY】是要更新記錄 */
	/* 暫時放這裡 */
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
Function        :inAMEX_Func_ADJUST_Confirm
Date&Time       :2015/10/22 下午 04:00
Describe        :確認是否能進行取消交易
*/
int inAMEX_Func_VOID_Check(TRANSACTION_OBJECT *pobTran)
{
        if (pobTran->srBRec.uszVOIDBit == VS_TRUE)
        {
                /* 顯示此交易已取消 */
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
Function        :inAMEX_Func_TIP_Confirm
Date&Time       :2015/10/22 下午 04:00
Describe        :確認是否能進行小費交易
*/
int inAMEX_Func_TIP_Check(TRANSACTION_OBJECT *pobTran)
{
        if (pobTran->srBRec.inCode == _TIP_    ||
            pobTran->srBRec.inCode == _VOID_   ||
            pobTran->srBRec.inCode == _REFUND_ ||
            pobTran->srBRec.inCode == _ADJUST_ )
        {
                /* 顯示不能做小費 */
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
Function        :inAMEX_Func_ADJUST_Confirm
Date&Time       :2015/10/22 下午 04:00
Describe        :確認是否能進行調帳交易
*/
int inAMEX_Func_ADJUST_Check(TRANSACTION_OBJECT *pobTran)
{
        if (pobTran->srBRec.inCode == _TIP_    ||
            pobTran->srBRec.inCode == _VOID_   ||
            pobTran->srBRec.inCode == _REFUND_ ||
            pobTran->srBRec.inCode == _ADJUST_ )
	{
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
Function        :inAMEX_Func_Must_SETTLE
Date&Time       :2015/10/22 下午 04:00
Describe        :確認是否要先結帳
*/
int inAMEX_Func_Must_SETTLE(TRANSACTION_OBJECT *pobTran)
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
		strcpy(srDispMsgObj.szErrMsg1, "AMEX");
		srDispMsgObj.inErrMsg1Line = _LINE_8_5_;
		srDispMsgObj.inBeepTimes = 1;
		srDispMsgObj.inBeepInterval = 0;
			
                inDISP_Msg_BMP(&srDispMsgObj);
		
		return (VS_ERROR);
        }
        else
                return (VS_SUCCESS);
}
