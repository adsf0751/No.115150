#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <emvaplib.h>
#include <sqlite3.h>
#include "../SOURCE/INCLUDES/Define_1.h"
#include "../SOURCE/INCLUDES/Define_2.h"
#include "../SOURCE/INCLUDES/Transaction.h"
#include "../SOURCE/INCLUDES/TransType.h"
#include "../SOURCE/INCLUDES/AllStruct.h"
#include "../SOURCE/DISPLAY/Display.h"
#include "../SOURCE/DISPLAY/DispMsg.h"
#include "../SOURCE/DISPLAY/DisTouch.h"
#include "../SOURCE/FUNCTION/Sqlite.h"
#include "../SOURCE/FUNCTION/Function.h"
#include "../SOURCE/FUNCTION/FuncTable.h"
#include "../SOURCE/FUNCTION/MVT.h"
#include "../SOURCE/PRINT/Print.h"
#include "../SOURCE/EVENT/MenuMsg.h"
#include "../CREDIT/Creditfunc.h"
#include "../SOURCE/FUNCTION/APDU.h"
#include "../SOURCE/FUNCTION/Batch.h"
#include "../SOURCE/FUNCTION/CDT.h"
#include "../SOURCE/FUNCTION/CFGT.h"
#include "../SOURCE/FUNCTION/EDC.h"
#include "../SOURCE/FUNCTION/ECR.h"
#include "../SOURCE/FUNCTION/HDT.h"
#include "../SOURCE/FUNCTION/HDPT.h"
#include "../SOURCE/FUNCTION/Card.h"
#include "../SOURCE/FUNCTION/KMS.h"
#include "../SOURCE/FUNCTION/MultiFunc.h"
#include "../SOURCE/FUNCTION/Utility.h"
#include "../SOURCE/EVENT/Flow.h"
#include "../SOURCE/COMM/Comm.h"
#include "../NCCC/NCCCsrc.h"
#include "../NCCC/NCCCats.h"
#include "../NCCC/NCCCmfes.h"
#include "../NCCC/NCCCdcc.h"
#include "../NCCC/NCCCtmk.h"
#include "../CTLS/CTLS.h"
#include "../FISC/NCCCfisc.h"
#include "EMVsrc.h"
#include "EMVxml.h"


extern int		ginDebug;
extern int		ginISODebug;
extern	int		ginMachineType;
extern int		ginMenuKeyIn, ginIdleMSRStatus, ginIdleICCStatus;
extern int		ginFallback;
extern int		ginAPVersionType;
extern EMV_CONFIG	EMVGlobConfig;
extern unsigned char	guszStrangeJCBCard;	/* 為了防玉山統一時代JCB卡的特殊Bit */
int			ginGlobalRetVal;
int			ginEMVAppSelection = -1;
int			ginEMVPINType = 0;

TRANSACTION_OBJECT	pobEmvTran;

/* EMV 流程會用到的所有 Callback Function */
EMV_EVENT g_emv_event =
{
        1,				/* version*/
        NULL,				/* 執行EMV_TxnAppSelect開始時會CALL這支function是否要顯示訊息 */
        OnErrorMsg,			/* 執行EMV_TxnAppSelect若是有錯誤訊息要顯示最後面會CALL這支function */
        OnEMVConfigActive,		/* 執行EMV_Initialize時會先CALL這支function，選擇要跑的Config */
        NULL,				/* HashVerify Not using (This event function is used for application to verify if the CAPKs of EMV kernel are correct.)*/
        OnTxnDataGet,			/* 執行EMV_TxnAppSelect時選完後會要求輸入交易資料，如金額，這是必跑的CallBack */
        OnAppList,			/* 執行EMV_TxnAppSelect時若是有多個AID會進入此function，要顯示AID列表供選擇 */
        OnAppSelectedConfirm,		/* 執行EMV_TxnAppSelect時若是只有一個AID會進入此function */
        OnTerminalDataGet,		/* 執行EMV_TxnPerform時若是Kernel在XML找不到需要的TAG值會從這裡要求提供所需的TAG */
        OnCAPKGet,			/* 執行EMV_TxnPerform時若是Kernel在XML找不到需要的CAPK KEY值會從這裡要求提供所需的CAPK KEY */
        OnGetPINNotify,			/* 執行EMV_TxnPerform時若是要用Internal PINPAD輸入PIN會CALL這支，輸入完後交由KERNAL處理，若是輸入錯則流程繼續往下跑，除非重新交易才會再進來 */
        OnOnlinePINBlockGet,		/* 執行EMV_TxnPerform時若是需要用外接PINPAD輸入Online PIN BLOCK會CALL這支function，目前都會用內建，所以應該不會用這支 */
        OnOfflinePINBlockGet,		/* 執行EMV_TxnPerform時若是需要用外接PINPAD輸入Offline PIN BLOCK會CALL這支function，目前都會用內建，所以應該不會用這支 */
        OnOfflinePINVerifyResult,	/* 執行EMV_TxnPerform時若是輸入Offline PIN後會CALL這支function檢查PIN是否合法 */
        OnTxnOnline,			/* 執行EMV_TxnPerform時若是First GenAC要求Online則進入這支function做電文送收，這支function一定要跑 */
        OnTxnIssuerScriptResult,	/* 執行EMV_TxnPerform時若是要處理Issuer Script則要跑這支function */
        OnTxnResult,			/* 執行EMV_TxnPerform的最後要判斷晶片處理結果且顯示通知使用者要在這處理 */
        OnTotalAmountGet,		/* 執行EMV_TxnPerform時要在這輸入金額將金額回傳給Kernel */
        OnExceptionFileCheck,		/* 執行EMV_TxnPerform時在進行FirstGenAC前會進入這檢查檔案 */
        OnCAPKRevocationCheck,		/* 執行EMV_TxnPerform時取得CAPK後會檢查是否CAPK合法 */
};

int inCheckFallbackFlag(TRANSACTION_OBJECT *pobTran)
{
	if (ginFallback == VS_TRUE)
	{
		inEMV_SetICCReadFailure(VS_FALSE);		/* 進入FALL BACK流程，把global標示要走FallBack的Bit關掉，否則會一直走Fallback */
		
		/* 不支援晶片卡交易別，不走Fallback流程 */
		if (pobTran->inTransactionCode == _SALE_OFFLINE_		||
		    pobTran->inTransactionCode == _REFUND_			||
	            pobTran->inTransactionCode == _INST_REFUND_			||
	            pobTran->inTransactionCode == _REDEEM_REFUND_		||
	            pobTran->inTransactionCode == _CUP_REFUND_			||
		    pobTran->inTransactionCode == _CUP_MAIL_ORDER_REFUND_	||
	            pobTran->inTransactionCode == _INST_ADJUST_			||
	            pobTran->inTransactionCode == _REDEEM_ADJUST_		||
                    pobTran->inTransactionCode == _ADJUST_			||
                    pobTran->inTransactionCode == _PRE_COMP_			||
	            pobTran->inTransactionCode == _CUP_PRE_COMP_		||
		    pobTran->inTransactionCode == _HG_REWARD_REFUND_		||
		    pobTran->inTransactionCode == _HG_REDEEM_REFUND_		||
		    pobTran->inTransactionCode == _FORCE_CASH_ADVANCE_		||
		   (memcmp(&pobTran->srBRec.szServiceCode[0], "2", 1) != 0	&& 
		    memcmp(&pobTran->srBRec.szServiceCode[0], "6", 1) != 0))		/* 如果此次過卡Service code未含2和6，代表此卡與觸發Fallback流程之卡不同，不應進Fallback流程 */
		{

			return (VS_SUCCESS);
		}
		else
		{
			/* 移除fallback功能 */
		}
		
		/* 【需求單-108408】端末機關閉Fallback功能 add by LingHsiung 2020/3/16 上午 10:48 */
		/* 晶片卡不支援刷磁條 */
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, _ERR_FALLBACK_);
		srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_5_;
		srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
		srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
		strcpy(srDispMsgObj.szErrMsg1, "");
		srDispMsgObj.inErrMsg1Line = 0;
		srDispMsgObj.inBeepTimes = 1;
		srDispMsgObj.inBeepInterval = 0;
		inDISP_Msg_BMP(&srDispMsgObj);
		
		return (VS_ERROR);
	}
        else if (pobTran->inTransactionCode != _SALE_OFFLINE_		&&
                 pobTran->inTransactionCode != _REFUND_			&&
	         pobTran->inTransactionCode != _INST_REFUND_		&&
	         pobTran->inTransactionCode != _REDEEM_REFUND_		&&
	         pobTran->inTransactionCode != _CUP_REFUND_		&&
		 pobTran->inTransactionCode != _CUP_MAIL_ORDER_REFUND_	&&
	         pobTran->inTransactionCode != _INST_ADJUST_		&&
	         pobTran->inTransactionCode != _REDEEM_ADJUST_		&&
                 pobTran->inTransactionCode != _ADJUST_			&&
                 pobTran->inTransactionCode != _PRE_COMP_		&&
	         pobTran->inTransactionCode != _CUP_PRE_COMP_		&&
		 pobTran->inTransactionCode != _FORCE_CASH_ADVANCE_	&&
                (memcmp(&pobTran->srBRec.szServiceCode[0], "2", 1) == 0 || memcmp(&pobTran->srBRec.szServiceCode[0], "6", 1) == 0)
		)
        {
		if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
		{
			/* 請讀晶片，無晶片 請持卡人洽發卡行 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_EMV_PLS_READ_EMV_CUP_;
		}
		else
		{
			/* 請改讀晶片卡 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_EMV_PLS_READ_EMV_;
		}
                
                /* Mirror Message */
                if (pobTran->uszECRBit == VS_TRUE)
                {
                        inECR_SendMirror(pobTran, _MIRROR_MSG_EMV_USE_CHIP_ERROR_);
                }
		
		/* ECR交易晶片過卡磁條，Response code要回0004 */
		pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
                
		return (VS_ERROR);
        }
        else
	{
		return (VS_SUCCESS);
	}
}

/*
Function	:fICCEvent
Date&Time	:2015/11/23 上午 10:41
Describe	:偵測EMV插卡
*/
int inEMV_ICCEvent(void)
{
        int		inRetVal;
        unsigned char   uszStatus;  /* 偵測晶片用狀態 */

        uszStatus = 0x00;

        /* 偵測晶片插卡事件，需依照各種不同機型去調整，以下為V3作法 */
        inRetVal = CTOS_SCStatus(d_SC_USER, &uszStatus);

        if (inRetVal != d_OK)
        {
                return (VS_ERROR);
        }
        else
        {
		/* 要用BitMask */
		if ((uszStatus & d_MK_SC_PRESENT) == d_MK_SC_PRESENT)
			return (VS_SUCCESS);
		else
			return (VS_ERROR);
        }
}

/*
Function        :inEMV_CheckRemoveCard
Date&Time       :2016/12/8 上午 11:08
Describe        :目前加在兩個地方 in(host)_RunTRT的尾端和接inFLOW_RunOperation回傳值的地方
*/
int inEMV_CheckRemoveCard(TRANSACTION_OBJECT *pobTran, int inIsError)
{
        int	inRetVal;
        unsigned short		usLen = 0;
        char	szCustomerIndicator[3 + 1] = {0};
        
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
        
        inRetVal = inEMV_ICCEvent(); 
	if (inRetVal== VS_SUCCESS)
	{
		/* 仍然插著卡片 */
		/* 設定Timeout */
//		inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);

                if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)     ||
                    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_))  
                {
                        inDISP_BEEP(1, 0);
                        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                        inDISP_PutGraphic(_ERR_PLS_REMOVE_CARD_, 0, _COORDINATE_Y_LINE_8_6_);
                }
                else
                {
                        if (inIsError == _REMOVE_CARD_ERROR_)
                        {
                                /* 請退回晶片卡 */
                                DISPLAY_OBJECT	srDispMsgObj;
                                memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
                                strcpy(srDispMsgObj.szDispPic1Name, _ERR_PLS_REMOVE_CARD_);
                                srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
                                srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
                                srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
                                strcpy(srDispMsgObj.szErrMsg1, "");
                                srDispMsgObj.inErrMsg1Line = 0;
                                srDispMsgObj.inBeepTimes = 1;
                                srDispMsgObj.inBeepInterval = 0;
                                inDISP_Msg_BMP(&srDispMsgObj);
                        }
                        else
                        {
                                inDISP_BEEP(1, 0);
                                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                                inDISP_PutGraphic(_ERR_PLS_REMOVE_CARD_, 0, _COORDINATE_Y_LINE_8_6_);
                        }
                }
	}
	else
		return (VS_SUCCESS);
    
        /* 客製化098 : 退卡時，不抽卡可直接接收ECR執行新流程(取消、查詢最後一筆)
         * 原流程 : 請移除卡片 => 退卡 => 清ECR Buffer => 等待觸發ECR => 新交易流程 
         * 麥當勞 : 請移除卡片 => 1.等待觸發ECR => 判定交易別 => 儲存交易內容 => 清ECR Buffer => 直接觸發ECR => 使用儲存內容不接收，並執行新交易流程
         *                                                  => 依然退卡UI 與 等待觸發ECR
         *                       2.退卡 => 清ECR Buffer => 等待觸發ECR => 新交易流程 
         */
        if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)     ||
            !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_))  
        {
                /* 此畫面需要能接收ECR資料並啟動，這邊退出時不去清除ECR Buffer */
            
                if (ginMachineType == _CASTLE_TYPE_V3UL_)
                {
                        inMultiFunc_RS232_FlushRxBuffer();
                }
                else
                {
                        /* ECR清buffer */
                        inECR_FlushRxBuffer();
                }
                
                while (1)
                {
                        if (inECR_Receive_Check(&usLen) == VS_SUCCESS)
                        {
                                pobTran->uszRemoveChecECRkBit = VS_TRUE;
                                
                                inRetVal = inECR_Receive_Transaction(pobTran);
                                
                                if (inRetVal == VS_SUCCESS)
                                        return (VS_SUCCESS);
                                
//                                inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
                        }    
                        
//                        if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
//                        {
//                                return (VS_SUCCESS);
//                        }

                        inRetVal = inEMV_ICCEvent(); 
                        if (inRetVal== VS_SUCCESS)
                        {
                                /* 仍然插著卡片 */
                                inDISP_PutGraphic(_ERR_PLS_REMOVE_CARD_, 0, _COORDINATE_Y_LINE_8_6_);
                        }
                        else
                        {
                                return (VS_SUCCESS);
                        }
                }
        }
        else
        {
                inDISP_PutGraphic(_ERR_PLS_REMOVE_CARD_, 0, _COORDINATE_Y_LINE_8_6_);
            
                while (1)
                {
//                        if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
//                        {
//                                return (VS_SUCCESS);
//                        }

                        inRetVal = inEMV_ICCEvent(); 
                        if (inRetVal == VS_SUCCESS)
                        {
                                /* 仍然插著卡片 */
                        }
                        else
                        {
                                return (VS_SUCCESS);
                        }
                }
        }
        
        return (VS_SUCCESS);
}

/*
Function	:inEMV_GetEMVCardData
Date&Time	:2015/12/20 PM 06:30:50
Describe	:進入EMV交易流程
*/
int inEMV_GetEMVCardData(TRANSACTION_OBJECT *pobTran)
{
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inEMV_GetEMVCardData() Start!");
        }

	if (pobTran->inTransactionCode == _SALE_OFFLINE_		||
	    pobTran->inTransactionCode == _REFUND_			||
	    pobTran->inTransactionCode == _INST_REFUND_			||
	    pobTran->inTransactionCode == _REDEEM_REFUND_		||
	    pobTran->inTransactionCode == _CUP_REFUND_			||
	    pobTran->inTransactionCode == _CUP_MAIL_ORDER_REFUND_	||
	    pobTran->inTransactionCode == _ADJUST_			||
	    pobTran->inTransactionCode == _INST_ADJUST_			||
	    pobTran->inTransactionCode == _REDEEM_ADJUST_		||
	    pobTran->inTransactionCode == _PRE_COMP_			||
	    pobTran->inTransactionCode == _CUP_PRE_COMP_	)
	{
                /* Mirror Message */
                if (pobTran->uszECRBit == VS_TRUE)
                {
                        inECR_SendMirror(pobTran, _MIRROR_MSG_NOT_USE_IC_CARD_);
                }
                                
		/* 本交易不接受晶片 請重新交易 */
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, _ERR_EMV_FAIL_);
		srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_5_;
		srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
		srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
		strcpy(srDispMsgObj.szErrMsg1, "");
		srDispMsgObj.inErrMsg1Line = 0;
		srDispMsgObj.inBeepTimes = 1;
		srDispMsgObj.inBeepInterval = 0;
                inDISP_Msg_BMP(&srDispMsgObj);
                
		return (VS_ERROR);
	}
        
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        /* 晶片卡讀取中 */
        inDISP_PutGraphic(_READ_ICC_, 0, _COORDINATE_Y_LINE_8_4_);
	
        /* 跑EMV流程 */
        if (inEMV_SelectICCAID(pobTran) != VS_SUCCESS)
	{
		/* 金融卡沒磁條，不開Fallback */
		if (pobTran->srBRec.uszFiscTransBit != VS_TRUE)
		{
			inEMV_SetICCReadFailure(VS_TRUE);		/* 開啟FALL BACK */
		}
		
                return (VS_ERROR);
	}
        
        
	if (pobTran->uszFISCBit == VS_TRUE)
	{
		/* SMARTPAY不算晶片卡 */
	}
	/* 此為晶片卡 */
	else
	{
		pobTran->srBRec.inChipStatus = _EMV_CARD_;
	}
	      
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inEMV_GetEMVCardData() End!");
        }	
        
	return(VS_SUCCESS); 
}

/*
Function	:inEMV_GetEMVTag
Date&Time	:2016/1/9 下午 11:42
Describe	:取得晶片卡EMVTag，若抓不到的會進TermainalDataGet裡面找來塞
*/
int inEMV_GetEMVTag(TRANSACTION_OBJECT *pobTran)
{
        int		inRetVal;
        char		szASCII[128 + 1];
	char		szTemplate[20 + 1];
	unsigned short	ushTagLen;
	unsigned char	uszTagData[128];
        
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inEMV_GetEMVTag()_START");
        }   

	if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
	{
		/* 50 */
		ushTagLen = sizeof(uszTagData);
		memset(uszTagData, 0x00, sizeof(uszTagData));
		inRetVal = EMV_DataGet(0x50, &ushTagLen, uszTagData);

                if (inRetVal == d_EMVAPLIB_OK  && ushTagLen > 0)
                {
                        memset(pobTran->srEMVRec.usz50_APLabel, 0x00, sizeof(pobTran->srEMVRec.usz50_APLabel));
                        pobTran->srEMVRec.in50_APLabelLen = ushTagLen;
                        memcpy(&pobTran->srEMVRec.usz50_APLabel[0], &uszTagData[0], pobTran->srEMVRec.in50_APLabelLen);
                        
                        /* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                sprintf(szASCII, "%s", pobTran->srEMVRec.usz50_APLabel);
                                
				inCTLS_ISOFormatDebug_DISP("50", pobTran->srEMVRec.in50_APLabelLen, szASCII);
			}
			
			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
				/* AP LABEL本來就是Ascii 不用轉 */
				memset(szASCII, 0x00, sizeof(szASCII));
				sprintf(szASCII, "%s", pobTran->srEMVRec.usz50_APLabel);

				inCTLS_ISOFormatDebug_PRINT("50", pobTran->srEMVRec.in50_APLabelLen, szASCII);
			}
			
                }
		
		/* 57 */
		ushTagLen = sizeof(uszTagData);
		memset(uszTagData, 0x00, sizeof(uszTagData));
		inRetVal = EMV_DataGet(0x57, &ushTagLen, uszTagData);

                if (inRetVal == d_EMVAPLIB_OK  && ushTagLen > 0)
                {
                        memset(pobTran->usz57_Track2, 0x00, sizeof(pobTran->usz57_Track2));
                        pobTran->in57_Track2Len = ushTagLen;
                        memcpy(&pobTran->usz57_Track2[0], &uszTagData[0], pobTran->in57_Track2Len);
                        
                        /* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->usz57_Track2[0], pobTran->in57_Track2Len);
                                
				inCTLS_ISOFormatDebug_DISP("57", pobTran->in57_Track2Len, szASCII);
			}
			
			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->usz57_Track2[0], pobTran->in57_Track2Len);
                                
				inCTLS_ISOFormatDebug_PRINT("57", pobTran->in57_Track2Len, szASCII);
			}
                }
		
		/* 5A */
		ushTagLen = sizeof(uszTagData);
		memset(uszTagData, 0x00, sizeof(uszTagData));
		inRetVal = EMV_DataGet(0x5A, &ushTagLen, uszTagData);

                if (inRetVal == d_EMVAPLIB_OK  && ushTagLen > 0)
                {
                        memset(pobTran->srEMVRec.usz5A_ApplPan, 0x00, sizeof(pobTran->srEMVRec.usz5A_ApplPan));
                        pobTran->srEMVRec.in5A_ApplPanLen = ushTagLen;
                        memcpy(&pobTran->srEMVRec.usz5A_ApplPan[0], &uszTagData[0], pobTran->srEMVRec.in5A_ApplPanLen);
                        
                        /* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz5A_ApplPan[0], pobTran->srEMVRec.in5A_ApplPanLen);
                                
				inCTLS_ISOFormatDebug_DISP("5A", pobTran->srEMVRec.in5A_ApplPanLen, szASCII);
			}
			
			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz5A_ApplPan[0], pobTran->srEMVRec.in5A_ApplPanLen);
                                
				inCTLS_ISOFormatDebug_PRINT("5A", pobTran->srEMVRec.in5A_ApplPanLen, szASCII);
			}
			
                }

		/* 5F20 */
		ushTagLen = sizeof(uszTagData);
		memset(uszTagData, 0x00, sizeof(uszTagData));
		inRetVal = EMV_DataGet(0x5F20, &ushTagLen, uszTagData);

                if (inRetVal == d_EMVAPLIB_OK  && ushTagLen > 0)
                {
                        memset(pobTran->srEMVRec.usz5F20_CardholderName, 0x00, sizeof(pobTran->srEMVRec.usz5F20_CardholderName));
                        pobTran->srEMVRec.in5F20_CardholderNameLen = ushTagLen;
                        memcpy(&pobTran->srEMVRec.usz5F20_CardholderName[0], &uszTagData[0], pobTran->srEMVRec.in5F20_CardholderNameLen);
                        
                        /* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                memcpy(&szASCII[0], &pobTran->srEMVRec.usz5F20_CardholderName[0], pobTran->srEMVRec.in5F20_CardholderNameLen);
				inCTLS_ISOFormatDebug_DISP("5F20", pobTran->srEMVRec.in5F20_CardholderNameLen, szASCII);
			}
			
			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                memcpy(&szASCII[0], &pobTran->srEMVRec.usz5F20_CardholderName[0], pobTran->srEMVRec.in5F20_CardholderNameLen);
				inCTLS_ISOFormatDebug_PRINT("5F20", pobTran->srEMVRec.in5F20_CardholderNameLen, szASCII);
			}
			
                }
		
		/* 5F24 */
		ushTagLen = sizeof(uszTagData);
		memset(uszTagData, 0x00, sizeof(uszTagData));
		inRetVal = EMV_DataGet(0x5F24, &ushTagLen, uszTagData);

                if (inRetVal == d_EMVAPLIB_OK  && ushTagLen > 0)
                {
                        memset(pobTran->srEMVRec.usz5F24_ExpireDate, 0x00, sizeof(pobTran->srEMVRec.usz5F24_ExpireDate));
                        pobTran->srEMVRec.in5F24_ExpireDateLen = ushTagLen;
                        memcpy(&pobTran->srEMVRec.usz5F24_ExpireDate[0], &uszTagData[0], pobTran->srEMVRec.in5F24_ExpireDateLen);
                        
                        /* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
				inCTLS_ISOFormatDebug_DISP("5F24", pobTran->srEMVRec.in5F24_ExpireDateLen, "-----");
			}
			
			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
				inCTLS_ISOFormatDebug_PRINT("5F24", pobTran->srEMVRec.in5F24_ExpireDateLen, "-----");
			}
			
                }
                
		/* 5F2A */
		ushTagLen = sizeof(uszTagData);
		memset(uszTagData, 0x00, sizeof(uszTagData));
		inRetVal = EMV_DataGet(0x5F2A, &ushTagLen, uszTagData);

                if (inRetVal == d_EMVAPLIB_OK  && ushTagLen > 0)
                {
                        memset(pobTran->srEMVRec.usz5F2A_TransCurrCode, 0x00, sizeof(pobTran->srEMVRec.usz5F2A_TransCurrCode));
                        pobTran->srEMVRec.in5F2A_TransCurrCodeLen = ushTagLen;
                        memcpy(&pobTran->srEMVRec.usz5F2A_TransCurrCode[0], &uszTagData[0], pobTran->srEMVRec.in5F2A_TransCurrCodeLen);
                        
                        /* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz5F2A_TransCurrCode[0], pobTran->srEMVRec.in5F2A_TransCurrCodeLen);
                                
				inCTLS_ISOFormatDebug_DISP("5F2A", pobTran->srEMVRec.in5F2A_TransCurrCodeLen, szASCII);
			}
			
			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz5F2A_TransCurrCode[0], pobTran->srEMVRec.in5F2A_TransCurrCodeLen);
                                
				inCTLS_ISOFormatDebug_PRINT("5F2A", pobTran->srEMVRec.in5F2A_TransCurrCodeLen, szASCII);
			}
			
                }
                
		/* 5F34 */
		ushTagLen = sizeof(uszTagData);
		memset(uszTagData, 0x00, sizeof(uszTagData));
		inRetVal = EMV_DataGet(0x5F34, &ushTagLen, uszTagData);

                if (inRetVal == d_EMVAPLIB_OK  && ushTagLen > 0)
                {
                        memset(pobTran->srEMVRec.usz5F34_ApplPanSeqnum, 0x00, sizeof(pobTran->srEMVRec.usz5F34_ApplPanSeqnum));
                        pobTran->srEMVRec.in5F34_ApplPanSeqnumLen = ushTagLen;
                        memcpy(&pobTran->srEMVRec.usz5F34_ApplPanSeqnum[0], &uszTagData[0], pobTran->srEMVRec.in5F34_ApplPanSeqnumLen);
                        
                        /* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz5F34_ApplPanSeqnum[0], pobTran->srEMVRec.in5F34_ApplPanSeqnumLen);
                                
				inCTLS_ISOFormatDebug_DISP("5F34", pobTran->srEMVRec.in5F34_ApplPanSeqnumLen, szASCII);
			}
			
			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz5F34_ApplPanSeqnum[0], pobTran->srEMVRec.in5F34_ApplPanSeqnumLen);
                                
				inCTLS_ISOFormatDebug_PRINT("5F34", pobTran->srEMVRec.in5F34_ApplPanSeqnumLen, szASCII);
			}
			
                }
                
		/* 82 */
		ushTagLen = sizeof(uszTagData);
		memset(uszTagData, 0x00, sizeof(uszTagData));
		inRetVal = EMV_DataGet(0x82, &ushTagLen, uszTagData);

                if (inRetVal == d_EMVAPLIB_OK  && ushTagLen > 0)
                {
                        memset(pobTran->srEMVRec.usz82_AIP, 0x00, sizeof(pobTran->srEMVRec.usz82_AIP));
                        pobTran->srEMVRec.in82_AIPLen = ushTagLen;
                        memcpy(&pobTran->srEMVRec.usz82_AIP[0], &uszTagData[0], pobTran->srEMVRec.in82_AIPLen);
                        
                        /* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz82_AIP[0], pobTran->srEMVRec.in82_AIPLen);
                                
				inCTLS_ISOFormatDebug_DISP("82", pobTran->srEMVRec.in82_AIPLen, szASCII);
			}
			
			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz82_AIP[0], pobTran->srEMVRec.in82_AIPLen);
                                
				inCTLS_ISOFormatDebug_PRINT("82", pobTran->srEMVRec.in82_AIPLen, szASCII);
			}
			
                }
                
		/* 84 */
		ushTagLen = sizeof(uszTagData);
		memset(uszTagData, 0x00, sizeof(uszTagData));
		inRetVal = EMV_DataGet(0x84, &ushTagLen, uszTagData);

                if (inRetVal == d_EMVAPLIB_OK  && ushTagLen > 0)
                {
                        memset(pobTran->srEMVRec.usz84_DF_NAME, 0x00, sizeof(pobTran->srEMVRec.usz84_DF_NAME));
                        pobTran->srEMVRec.in84_DFNameLen = ushTagLen;
                        memcpy(&pobTran->srEMVRec.usz84_DF_NAME[0], &uszTagData[0], pobTran->srEMVRec.in84_DFNameLen);
                        
                        /* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz84_DF_NAME[0], pobTran->srEMVRec.in84_DFNameLen);
                                
				inCTLS_ISOFormatDebug_DISP("84", pobTran->srEMVRec.in84_DFNameLen, szASCII);
			}
			
			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz84_DF_NAME[0], pobTran->srEMVRec.in84_DFNameLen);
                                
				inCTLS_ISOFormatDebug_PRINT("84", pobTran->srEMVRec.in84_DFNameLen, szASCII);
			}
			
                }
                
                /* 8A */
		ushTagLen = sizeof(uszTagData);
		memset(uszTagData, 0x00, sizeof(uszTagData));
		inRetVal = EMV_DataGet(0x8A, &ushTagLen, uszTagData);

                if (inRetVal == d_EMVAPLIB_OK  && ushTagLen > 0)
                {
                        memset(pobTran->srEMVRec.usz8A_AuthRespCode, 0x00, sizeof(pobTran->srEMVRec.usz8A_AuthRespCode));
                        pobTran->srEMVRec.in8A_AuthRespCodeLen = ushTagLen;
                        memcpy(&pobTran->srEMVRec.usz8A_AuthRespCode[0], &uszTagData[0], pobTran->srEMVRec.in8A_AuthRespCodeLen);
                        
                        /* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz8A_AuthRespCode[0], pobTran->srEMVRec.in8A_AuthRespCodeLen);
                                
				inCTLS_ISOFormatDebug_DISP("8A", pobTran->srEMVRec.in8A_AuthRespCodeLen, szASCII);
			}
			
			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz8A_AuthRespCode[0], pobTran->srEMVRec.in8A_AuthRespCodeLen);
                                
				inCTLS_ISOFormatDebug_PRINT("8A", pobTran->srEMVRec.in8A_AuthRespCodeLen, szASCII);
			}
			
                }
                
		/* 95 */
		ushTagLen = sizeof(uszTagData);
		memset(uszTagData, 0x00, sizeof(uszTagData));
		inRetVal = EMV_DataGet(0x95, &ushTagLen, uszTagData);

                if (inRetVal == d_EMVAPLIB_OK  && ushTagLen > 0)
                {
                        memset(pobTran->srEMVRec.usz95_TVR, 0x00, sizeof(pobTran->srEMVRec.usz95_TVR));
                        pobTran->srEMVRec.in95_TVRLen = ushTagLen;
                        memcpy(&pobTran->srEMVRec.usz95_TVR[0], &uszTagData[0], pobTran->srEMVRec.in95_TVRLen);
                        
                        /* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz95_TVR[0], pobTran->srEMVRec.in95_TVRLen);
                                
				inCTLS_ISOFormatDebug_DISP("95", pobTran->srEMVRec.in95_TVRLen, szASCII);
			}
			
			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz95_TVR[0], pobTran->srEMVRec.in95_TVRLen);
                                
				inCTLS_ISOFormatDebug_PRINT("95", pobTran->srEMVRec.in95_TVRLen, szASCII);
			}
			
                }
                
		/* 9A */
		ushTagLen = sizeof(uszTagData);
		memset(uszTagData, 0x00, sizeof(uszTagData));
		inRetVal = EMV_DataGet(0x9A, &ushTagLen, uszTagData);

                if (inRetVal == d_EMVAPLIB_OK  && ushTagLen > 0)
                {
                        memset(pobTran->srEMVRec.usz9A_TranDate, 0x00, sizeof(pobTran->srEMVRec.usz9A_TranDate));
                        pobTran->srEMVRec.in9A_TranDateLen = ushTagLen;
                        memcpy(&pobTran->srEMVRec.usz9A_TranDate[0], &uszTagData[0], pobTran->srEMVRec.in9A_TranDateLen);
                        
                        /* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9A_TranDate[0], pobTran->srEMVRec.in9A_TranDateLen);
                                
				inCTLS_ISOFormatDebug_DISP("9A", pobTran->srEMVRec.in9A_TranDateLen, szASCII);
			}
			
			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9A_TranDate[0], pobTran->srEMVRec.in9A_TranDateLen);
                                
				inCTLS_ISOFormatDebug_PRINT("9A", pobTran->srEMVRec.in9A_TranDateLen, szASCII);
			}
                }
                
		/* 9B */
		ushTagLen = sizeof(uszTagData);
		memset(uszTagData, 0x00, sizeof(uszTagData));
		inRetVal = EMV_DataGet(0x9B, &ushTagLen, uszTagData);

                if (inRetVal == d_EMVAPLIB_OK  && ushTagLen > 0)
                {
                        memset(pobTran->srEMVRec.usz9B_TSI, 0x00, sizeof(pobTran->srEMVRec.usz9B_TSI));
                        pobTran->srEMVRec.in9B_TSILen = ushTagLen;
                        memcpy(&pobTran->srEMVRec.usz9B_TSI[0], &uszTagData[0], pobTran->srEMVRec.in9B_TSILen);
                        
                        /* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9B_TSI[0], pobTran->srEMVRec.in9B_TSILen);
                                
				inCTLS_ISOFormatDebug_DISP("9B", pobTran->srEMVRec.in9B_TSILen, szASCII);
			}
			
			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9B_TSI[0], pobTran->srEMVRec.in9B_TSILen);
                                
				inCTLS_ISOFormatDebug_PRINT("9B", pobTran->srEMVRec.in9B_TSILen, szASCII);
			}
			
                }
                
		/* 9C */
		ushTagLen = sizeof(uszTagData);
		memset(uszTagData, 0x00, sizeof(uszTagData));
		inRetVal = EMV_DataGet(0x9C, &ushTagLen, uszTagData);

                if (inRetVal == d_EMVAPLIB_OK  && ushTagLen > 0)
                {
                        memset(pobTran->srEMVRec.usz9C_TranType, 0x00, sizeof(pobTran->srEMVRec.usz9C_TranType));
                        pobTran->srEMVRec.in9C_TranTypeLen = ushTagLen;
                        memcpy(&pobTran->srEMVRec.usz9C_TranType[0], &uszTagData[0], pobTran->srEMVRec.in9C_TranTypeLen);
                        
                        /* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9C_TranType[0], pobTran->srEMVRec.in9C_TranTypeLen);
                                
				inCTLS_ISOFormatDebug_DISP("9C", pobTran->srEMVRec.in9C_TranTypeLen, szASCII);
			}
			
			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9C_TranType[0], pobTran->srEMVRec.in9C_TranTypeLen);
                                
				inCTLS_ISOFormatDebug_PRINT("9C", pobTran->srEMVRec.in9C_TranTypeLen, szASCII);
			}
                }
                
		/* 9F02 */
		ushTagLen = sizeof(uszTagData);
		memset(uszTagData, 0x00, sizeof(uszTagData));
		inRetVal = EMV_DataGet(0x9F02, &ushTagLen, uszTagData);

                if (inRetVal == d_EMVAPLIB_OK  && ushTagLen > 0)
                {
                        memset(pobTran->srEMVRec.usz9F02_AmtAuthNum, 0x00, sizeof(pobTran->srEMVRec.usz9F02_AmtAuthNum));
                        pobTran->srEMVRec.in9F02_AmtAuthNumLen = ushTagLen;
                        memcpy(&pobTran->srEMVRec.usz9F02_AmtAuthNum[0], &uszTagData[0], pobTran->srEMVRec.in9F02_AmtAuthNumLen);
                        
                        /* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F02_AmtAuthNum[0], pobTran->srEMVRec.in9F02_AmtAuthNumLen);
                                
				inCTLS_ISOFormatDebug_DISP("9F02", pobTran->srEMVRec.in9F02_AmtAuthNumLen, szASCII);
			}
			
			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F02_AmtAuthNum[0], pobTran->srEMVRec.in9F02_AmtAuthNumLen);
                                
				inCTLS_ISOFormatDebug_PRINT("9F02", pobTran->srEMVRec.in9F02_AmtAuthNumLen, szASCII);
			}
			
                }
                
		/* 9F03 */
		ushTagLen = sizeof(uszTagData);
		memset(uszTagData, 0x00, sizeof(uszTagData));
		inRetVal = EMV_DataGet(0x9F03, &ushTagLen, uszTagData);

                if (inRetVal == d_EMVAPLIB_OK  && ushTagLen > 0)
                {
                        memset(pobTran->srEMVRec.usz9F03_AmtOtherNum, 0x00, sizeof(pobTran->srEMVRec.usz9F03_AmtOtherNum));
                        pobTran->srEMVRec.in9F03_AmtOtherNumLen = ushTagLen;
                        memcpy(&pobTran->srEMVRec.usz9F03_AmtOtherNum[0], &uszTagData[0], pobTran->srEMVRec.in9F03_AmtOtherNumLen);
                        
                        /* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F03_AmtOtherNum[0], pobTran->srEMVRec.in9F03_AmtOtherNumLen);
                                
				inCTLS_ISOFormatDebug_DISP("9F03", pobTran->srEMVRec.in9F03_AmtOtherNumLen, szASCII);
			}
			
			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F03_AmtOtherNum[0], pobTran->srEMVRec.in9F03_AmtOtherNumLen);
                                
				inCTLS_ISOFormatDebug_PRINT("9F03", pobTran->srEMVRec.in9F03_AmtOtherNumLen, szASCII);
			}
			
                }
                
		/* 9F08 */
		ushTagLen = sizeof(uszTagData);
		memset(uszTagData, 0x00, sizeof(uszTagData));
		inRetVal = EMV_DataGet(0x9F08, &ushTagLen, uszTagData);

                if (inRetVal == d_EMVAPLIB_OK  && ushTagLen > 0)
                {
                        memset(pobTran->srEMVRec.usz9F08_AppVerNumICC, 0x00, sizeof(pobTran->srEMVRec.usz9F08_AppVerNumICC));
                        pobTran->srEMVRec.in9F08_AppVerNumICCLen = ushTagLen;
                        memcpy(&pobTran->srEMVRec.usz9F08_AppVerNumICC[0], &uszTagData[0], pobTran->srEMVRec.in9F08_AppVerNumICCLen);
                        
                        /* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &uszTagData[0], ushTagLen);
                                
				inCTLS_ISOFormatDebug_DISP("9F08", ushTagLen, szASCII);
			}
			
			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &uszTagData[0], ushTagLen);
                                
				inCTLS_ISOFormatDebug_PRINT("9F08", ushTagLen, szASCII);
			}
			
                }
                
		/* 9F09 */
		ushTagLen = sizeof(uszTagData);
		memset(uszTagData, 0x00, sizeof(uszTagData));
		inRetVal = EMV_DataGet(0x9F09, &ushTagLen, uszTagData);

                if (inRetVal == d_EMVAPLIB_OK  && ushTagLen > 0)
                {
                        memset(pobTran->srEMVRec.usz9F09_TermVerNum, 0x00, sizeof(pobTran->srEMVRec.usz9F09_TermVerNum));
                        pobTran->srEMVRec.in9F09_TermVerNumLen = ushTagLen;
                        memcpy(&pobTran->srEMVRec.usz9F09_TermVerNum[0], &uszTagData[0], pobTran->srEMVRec.in9F09_TermVerNumLen);
                        
                        /* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &uszTagData[0], ushTagLen);
                                
				inCTLS_ISOFormatDebug_DISP("9F09", ushTagLen, szASCII);
			}
			
			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &uszTagData[0], ushTagLen);
                                
				inCTLS_ISOFormatDebug_PRINT("9F09", ushTagLen, szASCII);
			}
			
                }
                
		/* 9F10 */
		ushTagLen = sizeof(uszTagData);
		memset(uszTagData, 0x00, sizeof(uszTagData));
		inRetVal = EMV_DataGet(0x9F10, &ushTagLen, uszTagData);

                if (inRetVal == d_EMVAPLIB_OK  && ushTagLen > 0)
                {
                        memset(pobTran->srEMVRec.usz9F10_IssuerAppData, 0x00, sizeof(pobTran->srEMVRec.usz9F10_IssuerAppData));
                        pobTran->srEMVRec.in9F10_IssuerAppDataLen = ushTagLen;
                        memcpy(&pobTran->srEMVRec.usz9F10_IssuerAppData[0], &uszTagData[0], pobTran->srEMVRec.in9F10_IssuerAppDataLen);
                        
                        /* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F10_IssuerAppData[0], pobTran->srEMVRec.in9F10_IssuerAppDataLen);
                                
				inCTLS_ISOFormatDebug_DISP("9F10", pobTran->srEMVRec.in9F10_IssuerAppDataLen, szASCII);
			}
			
			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F10_IssuerAppData[0], pobTran->srEMVRec.in9F10_IssuerAppDataLen);
                                
				inCTLS_ISOFormatDebug_PRINT("9F10", pobTran->srEMVRec.in9F10_IssuerAppDataLen, szASCII);
			}
			
                }
                
		/* 9F1A */
		ushTagLen = sizeof(uszTagData);
		memset(uszTagData, 0x00, sizeof(uszTagData));
		inRetVal = EMV_DataGet(0x9F1A, &ushTagLen, uszTagData);

                if (inRetVal == d_EMVAPLIB_OK  && ushTagLen > 0)
                {
                        memset(pobTran->srEMVRec.usz9F1A_TermCountryCode, 0x00, sizeof(pobTran->srEMVRec.usz9F1A_TermCountryCode));
                        pobTran->srEMVRec.in9F1A_TermCountryCodeLen = ushTagLen;
                        memcpy(&pobTran->srEMVRec.usz9F1A_TermCountryCode[0], &uszTagData[0], pobTran->srEMVRec.in9F1A_TermCountryCodeLen);
                        
                        /* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F1A_TermCountryCode[0], pobTran->srEMVRec.in9F1A_TermCountryCodeLen);
                                
				inCTLS_ISOFormatDebug_DISP("9F1A", pobTran->srEMVRec.in9F1A_TermCountryCodeLen, szASCII);
			}
			
			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F1A_TermCountryCode[0], pobTran->srEMVRec.in9F1A_TermCountryCodeLen);
                                
				inCTLS_ISOFormatDebug_PRINT("9F1A", pobTran->srEMVRec.in9F1A_TermCountryCodeLen, szASCII);
			}
			
                }
                
		/* 9F1E */
		ushTagLen = sizeof(uszTagData);
		memset(uszTagData, 0x00, sizeof(uszTagData));
		inRetVal = EMV_DataGet(0x9F1E, &ushTagLen, uszTagData);
		
                if (inRetVal == d_EMVAPLIB_OK  && ushTagLen > 0)
                {
                        memset(pobTran->srEMVRec.usz9F1E_IFDNum, 0x00, sizeof(pobTran->srEMVRec.usz9F1E_IFDNum));
                        pobTran->srEMVRec.in9F1E_IFDNumLen = ushTagLen;
                        memcpy(&pobTran->srEMVRec.usz9F1E_IFDNum[0], &uszTagData[0], pobTran->srEMVRec.in9F1E_IFDNumLen);
                        
                        /* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F1E_IFDNum[0], pobTran->srEMVRec.in9F1E_IFDNumLen);
                                
				inCTLS_ISOFormatDebug_DISP("9F1E", pobTran->srEMVRec.in9F1E_IFDNumLen, szASCII);
			}
			
			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F1E_IFDNum[0], pobTran->srEMVRec.in9F1E_IFDNumLen);
                                
				inCTLS_ISOFormatDebug_PRINT("9F1E", pobTran->srEMVRec.in9F1E_IFDNumLen, szASCII);
			}
			
                }
                
		/* 9F26 */
		ushTagLen = sizeof(uszTagData);
		memset(uszTagData, 0x00, sizeof(uszTagData));
		inRetVal = EMV_DataGet(0x9F26, &ushTagLen, uszTagData);

                if (inRetVal == d_EMVAPLIB_OK  && ushTagLen > 0)
                {
                        memset(pobTran->srEMVRec.usz9F26_ApplCryptogram, 0x00, sizeof(pobTran->srEMVRec.usz9F26_ApplCryptogram));
                        pobTran->srEMVRec.in9F26_ApplCryptogramLen = ushTagLen;
                        memcpy(&pobTran->srEMVRec.usz9F26_ApplCryptogram[0], &uszTagData[0], pobTran->srEMVRec.in9F26_ApplCryptogramLen);
                        
                        /* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F26_ApplCryptogram[0], pobTran->srEMVRec.in9F26_ApplCryptogramLen);
                                
				inCTLS_ISOFormatDebug_DISP("9F26", pobTran->srEMVRec.in9F26_ApplCryptogramLen, szASCII);
			}
			
			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F26_ApplCryptogram[0], pobTran->srEMVRec.in9F26_ApplCryptogramLen);
                                
				inCTLS_ISOFormatDebug_PRINT("9F26", pobTran->srEMVRec.in9F26_ApplCryptogramLen, szASCII);
			}
			
                }
                
		/* 9F27 */
		ushTagLen = sizeof(uszTagData);
		memset(uszTagData, 0x00, sizeof(uszTagData));
		inRetVal = EMV_DataGet(0x9F27, &ushTagLen, uszTagData);

                if (inRetVal == d_EMVAPLIB_OK  && ushTagLen > 0)
                {
                        memset(pobTran->srEMVRec.usz9F27_CID, 0x00, sizeof(pobTran->srEMVRec.usz9F27_CID));
                        pobTran->srEMVRec.in9F27_CIDLen = ushTagLen;
                        memcpy(&pobTran->srEMVRec.usz9F27_CID[0], &uszTagData[0], pobTran->srEMVRec.in9F27_CIDLen);
                        
                        /* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F27_CID[0], pobTran->srEMVRec.in9F27_CIDLen);
                                
				inCTLS_ISOFormatDebug_DISP("9F27", pobTran->srEMVRec.in9F27_CIDLen, szASCII);
			}
			
			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F27_CID[0], pobTran->srEMVRec.in9F27_CIDLen);
                                
				inCTLS_ISOFormatDebug_PRINT("9F27", pobTran->srEMVRec.in9F27_CIDLen, szASCII);
			}
			
                }
                
		/* 9F33 */
		ushTagLen = sizeof(uszTagData);
		memset(uszTagData, 0x00, sizeof(uszTagData));
		inRetVal = EMV_DataGet(0x9F33, &ushTagLen, uszTagData);

                if (inRetVal == d_EMVAPLIB_OK  && ushTagLen > 0)
                {
                        memset(pobTran->srEMVRec.usz9F33_TermCapabilities, 0x00, sizeof(pobTran->srEMVRec.usz9F33_TermCapabilities));
                        pobTran->srEMVRec.in9F33_TermCapabilitiesLen = ushTagLen;
                        memcpy(&pobTran->srEMVRec.usz9F33_TermCapabilities[0], &uszTagData[0], pobTran->srEMVRec.in9F33_TermCapabilitiesLen);
                        
                        /* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F33_TermCapabilities[0], pobTran->srEMVRec.in9F33_TermCapabilitiesLen);
                                
				inCTLS_ISOFormatDebug_DISP("9F33", pobTran->srEMVRec.in9F33_TermCapabilitiesLen, szASCII);
			}
			
			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F33_TermCapabilities[0], pobTran->srEMVRec.in9F33_TermCapabilitiesLen);
                                
				inCTLS_ISOFormatDebug_PRINT("9F33", pobTran->srEMVRec.in9F33_TermCapabilitiesLen, szASCII);
			}
			
                }
                
		/* 9F34 */
		ushTagLen = sizeof(uszTagData);
		memset(uszTagData, 0x00, sizeof(uszTagData));
		inRetVal = EMV_DataGet(0x9F34, &ushTagLen, uszTagData);

                if (inRetVal == d_EMVAPLIB_OK  && ushTagLen > 0)
                {
                        memset(pobTran->srEMVRec.usz9F34_CVM, 0x00, sizeof(pobTran->srEMVRec.usz9F34_CVM));
                        pobTran->srEMVRec.in9F34_CVMLen = ushTagLen;
                        memcpy(&pobTran->srEMVRec.usz9F34_CVM[0], &uszTagData[0], pobTran->srEMVRec.in9F34_CVMLen);
                        
                        /* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F34_CVM[0], pobTran->srEMVRec.in9F34_CVMLen);
                                
				inCTLS_ISOFormatDebug_DISP("9F34", pobTran->srEMVRec.in9F34_CVMLen, szASCII);
			}
			
			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F34_CVM[0], pobTran->srEMVRec.in9F34_CVMLen);
                                
				inCTLS_ISOFormatDebug_PRINT("9F34", pobTran->srEMVRec.in9F34_CVMLen, szASCII);
			}
			
                }
                
		/* 9F35 */
		ushTagLen = sizeof(uszTagData);
		memset(uszTagData, 0x00, sizeof(uszTagData));
		inRetVal = EMV_DataGet(0x9F35, &ushTagLen, uszTagData);

                if (inRetVal == d_EMVAPLIB_OK  && ushTagLen > 0)
                {
                        memset(pobTran->srEMVRec.usz9F35_TermType, 0x00, sizeof(pobTran->srEMVRec.usz9F35_TermType));
                        pobTran->srEMVRec.in9F35_TermTypeLen = ushTagLen;
                        memcpy(&pobTran->srEMVRec.usz9F35_TermType[0], &uszTagData[0], pobTran->srEMVRec.in9F35_TermTypeLen);
                        
                        /* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F35_TermType[0], pobTran->srEMVRec.in9F35_TermTypeLen);
                                
				inCTLS_ISOFormatDebug_DISP("9F35", pobTran->srEMVRec.in9F35_TermTypeLen, szASCII);
			}
			
			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F35_TermType[0], pobTran->srEMVRec.in9F35_TermTypeLen);
                                
				inCTLS_ISOFormatDebug_PRINT("9F35", pobTran->srEMVRec.in9F35_TermTypeLen, szASCII);
			}
			
                }
                
		/* 9F36 */
		ushTagLen = sizeof(uszTagData);
		memset(uszTagData, 0x00, sizeof(uszTagData));
		inRetVal = EMV_DataGet(0x9F36, &ushTagLen, uszTagData);

                if (inRetVal == d_EMVAPLIB_OK  && ushTagLen > 0)
                {
                        memset(pobTran->srEMVRec.usz9F36_ATC, 0x00, sizeof(pobTran->srEMVRec.usz9F36_ATC));
                        pobTran->srEMVRec.in9F36_ATCLen = ushTagLen;
                        memcpy(&pobTran->srEMVRec.usz9F36_ATC[0], &uszTagData[0], pobTran->srEMVRec.in9F36_ATCLen);
                        
                        /* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F36_ATC[0], pobTran->srEMVRec.in9F36_ATCLen);
                                
				inCTLS_ISOFormatDebug_DISP("9F36", pobTran->srEMVRec.in9F36_ATCLen, szASCII);
			}
			
			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F36_ATC[0], pobTran->srEMVRec.in9F36_ATCLen);
                                
				inCTLS_ISOFormatDebug_PRINT("9F36", pobTran->srEMVRec.in9F36_ATCLen, szASCII);
			}
			
                }
                
		/* 9F37 */
		ushTagLen = sizeof(uszTagData);
		memset(uszTagData, 0x00, sizeof(uszTagData));
		inRetVal = EMV_DataGet(0x9F37, &ushTagLen, uszTagData);
                if (inRetVal == d_EMVAPLIB_OK  && ushTagLen > 0)
                {
                        memset(pobTran->srEMVRec.usz9F37_UnpredictNum, 0x00, sizeof(pobTran->srEMVRec.usz9F37_UnpredictNum));
                        pobTran->srEMVRec.in9F37_UnpredictNumLen = ushTagLen;
                        memcpy(&pobTran->srEMVRec.usz9F37_UnpredictNum[0], &uszTagData[0], pobTran->srEMVRec.in9F37_UnpredictNumLen);
                        
                        /* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F37_UnpredictNum[0], pobTran->srEMVRec.in9F37_UnpredictNumLen);
                                
				inCTLS_ISOFormatDebug_DISP("9F37", pobTran->srEMVRec.in9F37_UnpredictNumLen, szASCII);
			}
			
			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F37_UnpredictNum[0], pobTran->srEMVRec.in9F37_UnpredictNumLen);
                                
				inCTLS_ISOFormatDebug_PRINT("9F37", pobTran->srEMVRec.in9F37_UnpredictNumLen, szASCII);
			}
			
                }
                
		/* 9F41 */
		ushTagLen = sizeof(uszTagData);
		memset(uszTagData, 0x00, sizeof(uszTagData));
		inRetVal = EMV_DataGet(0x9F41, &ushTagLen, uszTagData);

                if (inRetVal == d_EMVAPLIB_OK  && ushTagLen > 0)
                {
                        memset(pobTran->srEMVRec.usz9F41_TransSeqCounter, 0x00, sizeof(pobTran->srEMVRec.usz9F41_TransSeqCounter));
                        pobTran->srEMVRec.in9F41_TransSeqCounterLen = ushTagLen;
                        memcpy(&pobTran->srEMVRec.usz9F41_TransSeqCounter[0], &uszTagData[0], pobTran->srEMVRec.in9F41_TransSeqCounterLen);
                        
                        /* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F41_TransSeqCounter[0], pobTran->srEMVRec.in9F41_TransSeqCounterLen);
                                
				inCTLS_ISOFormatDebug_DISP("9F41", pobTran->srEMVRec.in9F41_TransSeqCounterLen, szASCII);
			}
			
			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F41_TransSeqCounter[0], pobTran->srEMVRec.in9F41_TransSeqCounterLen);
                                
				inCTLS_ISOFormatDebug_PRINT("9F41", pobTran->srEMVRec.in9F41_TransSeqCounterLen, szASCII);
			}
			
                }
                
		/* 9F5B不在這邊抓，在online完的OnTxnIssuerScriptResult抓 */
		
		if (pobTran->srBRec.uszEMVFallBackBit == VS_TRUE)
		{
			/* 現在不再送DFEC了 */
			/* DFEC */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			if (inFunc_TWNAddDataToEMVPacket(pobTran, TAG_DFEC_FALLBACK_INDICATOR, (unsigned char *)&szTemplate[0]) > 0)
			{
				memset(pobTran->srEMVRec.uszDFEC_FallBackIndicator, 0x00, sizeof(pobTran->srEMVRec.uszDFEC_FallBackIndicator));
				pobTran->srEMVRec.inDFEC_FallBackIndicatorLen = szTemplate[2];
				memcpy((char *)&pobTran->srEMVRec.uszDFEC_FallBackIndicator[0], &szTemplate[3], pobTran->srEMVRec.inDFEC_FallBackIndicatorLen);
			}
			/* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.uszDFEC_FallBackIndicator[0], pobTran->srEMVRec.inDFEC_FallBackIndicatorLen);
                                
				inCTLS_ISOFormatDebug_DISP("DFEC", pobTran->srEMVRec.inDFEC_FallBackIndicatorLen, szASCII);
			}
			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.uszDFEC_FallBackIndicator[0], pobTran->srEMVRec.inDFEC_FallBackIndicatorLen);
                                
				inCTLS_ISOFormatDebug_PRINT("DFEC", pobTran->srEMVRec.inDFEC_FallBackIndicatorLen, szASCII);
			}

			/* DFED */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			if (inFunc_TWNAddDataToEMVPacket(pobTran, TAG_DFED_CHIP_CONDITION_CODE, (unsigned char *)&szTemplate[0]) > 0)
			{
				memset(pobTran->srEMVRec.uszDFED_ChipConditionCode, 0x00, sizeof(pobTran->srEMVRec.uszDFED_ChipConditionCode));
				pobTran->srEMVRec.inDFED_ChipConditionCodeLen = szTemplate[2];
				memcpy((char *)&pobTran->srEMVRec.uszDFED_ChipConditionCode[0], &szTemplate[3], pobTran->srEMVRec.inDFED_ChipConditionCodeLen);
			}
			/* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.uszDFED_ChipConditionCode[0], pobTran->srEMVRec.inDFED_ChipConditionCodeLen);
                                
				inCTLS_ISOFormatDebug_DISP("DFED", pobTran->srEMVRec.inDFED_ChipConditionCodeLen, szASCII);
			}
			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.uszDFED_ChipConditionCode[0], pobTran->srEMVRec.inDFED_ChipConditionCodeLen);
                                
				inCTLS_ISOFormatDebug_PRINT("DFED", pobTran->srEMVRec.inDFED_ChipConditionCodeLen, szASCII);
			}
			
			/* DFEE */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			if (inFunc_TWNAddDataToEMVPacket(pobTran, TAG_DFEE_TERMINAL_ENTRY_CAPABILITY, (unsigned char *)&szTemplate[0]) > 0)
			{
				memset(pobTran->srEMVRec.uszDFEE_TerEntryCap, 0x00, sizeof(pobTran->srEMVRec.uszDFEE_TerEntryCap));
				pobTran->srEMVRec.inDFEE_TerEntryCapLen = szTemplate[2];
				memcpy((char *)&pobTran->srEMVRec.uszDFEE_TerEntryCap[0], &szTemplate[3], pobTran->srEMVRec.inDFEE_TerEntryCapLen);
			}
			/* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.uszDFEE_TerEntryCap[0], pobTran->srEMVRec.inDFED_ChipConditionCodeLen);
                                
				inCTLS_ISOFormatDebug_DISP("DFEE", pobTran->srEMVRec.inDFED_ChipConditionCodeLen, szASCII);
			}
			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.uszDFEE_TerEntryCap[0], pobTran->srEMVRec.inDFED_ChipConditionCodeLen);
                                
				inCTLS_ISOFormatDebug_PRINT("DFEE", pobTran->srEMVRec.inDFED_ChipConditionCodeLen, szASCII);
			}

			/* DFEF */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			if (inFunc_TWNAddDataToEMVPacket(pobTran, TAG_DFEF_REASON_ONLINE_CODE, (unsigned char *)&szTemplate[0]) > 0)
			{
				memset(pobTran->srEMVRec.uszDFEF_ReasonOnlineCode, 0x00, sizeof(pobTran->srEMVRec.uszDFEF_ReasonOnlineCode));
				pobTran->srEMVRec.inDFEF_ReasonOnlineCodeLen = szTemplate[2];
				memcpy((char *)&pobTran->srEMVRec.uszDFEF_ReasonOnlineCode[0], &szTemplate[3], pobTran->srEMVRec.inDFEF_ReasonOnlineCodeLen);
			}
			/* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.uszDFEF_ReasonOnlineCode[0], pobTran->srEMVRec.inDFEF_ReasonOnlineCodeLen);
                                
				inCTLS_ISOFormatDebug_DISP("DFEF", pobTran->srEMVRec.inDFEF_ReasonOnlineCodeLen, szASCII);
			}
			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.uszDFEF_ReasonOnlineCode[0], pobTran->srEMVRec.inDFEF_ReasonOnlineCodeLen);
                                
				inCTLS_ISOFormatDebug_PRINT("DFEF", pobTran->srEMVRec.inDFEF_ReasonOnlineCodeLen, szASCII);
			}
			
		}
		else
		{
			if (pobTran->inTransactionCode == _SALE_	||
			    pobTran->inTransactionCode == _REDEEM_SALE_ ||
			    pobTran->inTransactionCode == _INST_SALE_	||
			    pobTran->inTransactionCode == _PRE_AUTH_	||
			    pobTran->inTransactionCode == _TC_UPLOAD_	||
			    pobTran->inTransactionCode == _CASH_ADVANCE_||
			    pobTran->inTransactionCode == _CUP_SALE_	||
			    pobTran->inTransactionCode == _CUP_PRE_AUTH_)
			{
				/* DFEE */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				if (inFunc_TWNAddDataToEMVPacket(pobTran, TAG_DFEE_TERMINAL_ENTRY_CAPABILITY, (unsigned char *)szTemplate) > 0)
				{
					memset(pobTran->srEMVRec.uszDFEE_TerEntryCap, 0x00, sizeof(pobTran->srEMVRec.uszDFEE_TerEntryCap));
					pobTran->srEMVRec.inDFEE_TerEntryCapLen = szTemplate[2];
					memcpy((char *)&pobTran->srEMVRec.uszDFEE_TerEntryCap[0], &szTemplate[3], pobTran->srEMVRec.inDFEE_TerEntryCapLen);
				}
				
				/* DFEF */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				if (inFunc_TWNAddDataToEMVPacket(pobTran, TAG_DFEF_REASON_ONLINE_CODE, (unsigned char *)szTemplate) > 0)
				{
					memset(pobTran->srEMVRec.uszDFEF_ReasonOnlineCode, 0x00, sizeof(pobTran->srEMVRec.uszDFEF_ReasonOnlineCode));
					pobTran->srEMVRec.inDFEF_ReasonOnlineCodeLen = szTemplate[2];
					memcpy((char *)&pobTran->srEMVRec.uszDFEF_ReasonOnlineCode[0], &szTemplate[3], pobTran->srEMVRec.inDFEF_ReasonOnlineCodeLen);
				}
				
			}
		
		}
		
	}
	
	/* Tag 9F63 若UICC卡片包含此Tag 時， EDC則應支援該Tag上傳。 */
	if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
	{
		/* 9F63 */
		ushTagLen = sizeof(uszTagData);
		memset(uszTagData, 0x00, sizeof(uszTagData));
		inRetVal = EMV_DataGet(0x9F63, &ushTagLen, uszTagData);

                if (inRetVal == d_EMVAPLIB_OK  && ushTagLen > 0)
                {
                        memset(pobTran->srEMVRec.usz9F63_CardProductLabelInformation, 0x00, sizeof(pobTran->srEMVRec.usz9F63_CardProductLabelInformation));
                        pobTran->srEMVRec.in9F63_CardProductLabelInformationLen = ushTagLen;
                        memcpy(&pobTran->srEMVRec.usz9F63_CardProductLabelInformation[0], &uszTagData[0], pobTran->srEMVRec.in9F63_CardProductLabelInformationLen);
                        
                        /* ISO Display Debug */
			if (ginDebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F63_CardProductLabelInformation[0], pobTran->srEMVRec.in9F63_CardProductLabelInformationLen);
                                
				inCTLS_ISOFormatDebug_DISP("9F63", pobTran->srEMVRec.in9F63_CardProductLabelInformationLen, szASCII);
			}
			
			/* ISO Print Debug */
			if (ginISODebug == VS_TRUE)
			{
                                memset(szASCII, 0x00, sizeof(szASCII));
                                inFunc_BCD_to_ASCII(&szASCII[0], &pobTran->srEMVRec.usz9F63_CardProductLabelInformation[0], pobTran->srEMVRec.in9F63_CardProductLabelInformationLen);
                                
				inCTLS_ISOFormatDebug_PRINT("9F63", pobTran->srEMVRec.in9F63_CardProductLabelInformationLen, szASCII);
			}
			
                }
	}

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inEMV_GetEMVTag()_END");
        }   
        
	return (VS_SUCCESS);
}

/*
Function        :inEMV_CreditPowerON
Date&Time       :2017/7/14 上午 11:42
Describe        :
*/
int inEMV_CreditPowerON(TRANSACTION_OBJECT *pobTran)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
        unsigned char	szATR[128 + 1], szATRLen, szCardType;
        
        szATRLen = sizeof(szATR); 
        //Power on the ICC and retrun the ATR contents metting the EMV2000 specification //
	usRetVal = CTOS_SCResetEMV(d_SC_USER, d_SC_5V, szATR, &szATRLen, &szCardType);
        if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "CTOS_SCResetISO ERROR!!");
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "RetVal: 0x%04x", usRetVal);
			inLogPrintf(AT, szDebugMsg);
                }
		
                if (usRetVal == d_SC_NOT_PRESENT)
		{
			return (VS_EMV_CARD_OUT);
		}
                else
		{
			return (VS_ERROR);
		}
	}
        
        return (VS_SUCCESS);
}

/*
Function        :inCard_CreditPowerOFF
Date&Time       :
Describe        :
*/
int inEMV_CreditPowerOFF(TRANSACTION_OBJECT *pobTran)
{
        if (CTOS_SCPowerOff(d_SC_USER) != d_OK) 
                return (VS_ERROR);
        
        return (VS_SUCCESS);
}

/*
Function        :inCard_CreditSelectAID
Date&Time       :
Describe        :
*/
int inEMV_CreditSelectAID(TRANSACTION_OBJECT *pobTran, int inContactType)
{
	int			inRetVal = 0;
        int			i = 0;
	int			inAIDLen = 0;
	int			inCnt = 0;
	int			inRemainLen = 0;
	int			inTagLen = 0;
	int			inLevel = 0;
	int			inChangeErrMsg = _ERROR_CODE_V3_NONE_;
        char			szMVTAID[20 + 1] = {0}, szHexAID[13 + 1] = {0};
	char			szBinCheckAID[13 + 1] = {0};
	char			szBinCheck[2 + 1] = {0};
	char			szDebug[200 + 1] = {0} , szTemplate[64 + 1] = {0};
	char			szASCII[2048 + 1] = {0};
	char			szTag[2 + 1] = {0};
	unsigned char		uszPPSE_APDUBit = VS_FALSE;
	unsigned char		uszFind4FBit = VS_FALSE;
	unsigned char		uszUseAPDUBit = VS_FALSE;
	unsigned char		uszUseEMVKernelBit = VS_FALSE;
	unsigned char		uszLabel[32 + 1] = {}, uszLabelLen = 0;
	unsigned char		uszSelectedAID[20 + 1] = {}, uszSelectedAIDLen = 0;
	unsigned short		ushRetVal = 0;
	unsigned short		ushTagLen = 0;
	unsigned char		uszTagData[128 + 1] = {0};
	unsigned char		uszTag2ByteBit = VS_FALSE;
        APDU_COMMAND		srAPDU_COMMAND;
	
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inEMV_CreditSelectAID()_START");
        }
	
        /* 先select PPSE */
	if (inContactType == _CONTACT_TYPE_01_CONTACTLESS_)
	{
		/* 2PAY.SYS.DDF01 */
		memset(szMVTAID, 0x00, sizeof(szMVTAID));
		memset(szHexAID, 0x00, sizeof(szHexAID));
		sprintf(szMVTAID, "2PAY.SYS.DDF01");
		inAIDLen = strlen(szMVTAID);
		
		/* APDU Command */
                memset(&srAPDU_COMMAND, 0x00, sizeof(APDU_COMMAND));

		srAPDU_COMMAND.uszCommandINSData[0] = _FISC_SELECT_AID_CLA_COMMAND_;		/* CLA */
                srAPDU_COMMAND.uszCommandINSData[0] = _FISC_SELECT_AID_INS_COMMAND_;		/* INS */
		srAPDU_COMMAND.uszCommandP1Data[0] = _FISC_SELECT_AID_P1_COMMAND_;		/* P1 */
		srAPDU_COMMAND.uszCommandP2Data[0] = _FISC_SELECT_AID_P2_COMMAND_;		/* P2 */
		srAPDU_COMMAND.inCommandDataLen = inAIDLen;
		memcpy(srAPDU_COMMAND.uszCommandData, szMVTAID, inAIDLen);
		
                if (inAPDU_BuildAPDU(&srAPDU_COMMAND) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		
		inRetVal = inAPDU_Send_APDU_CTLS_Process(&srAPDU_COMMAND);
		
		if (inRetVal == VS_SUCCESS)
                {
			/* 6283 是鎖卡，需要到EMV執行步驟再回傳錯誤訊息 d_EMVAPLIB_ERR_ONLY_1_AP_NO_FALLBACK */
                        if ((srAPDU_COMMAND.uszRecevData[srAPDU_COMMAND.inRecevLen -2] == 0x90 && srAPDU_COMMAND.uszRecevData[srAPDU_COMMAND.inRecevLen -1] == 0x00))
                        {
				uszPPSE_APDUBit = VS_TRUE;
				uszUseAPDUBit = VS_TRUE;
			}
			else
			{
				uszPPSE_APDUBit = VS_FALSE;
			}
		}
		else
		{
			/* 不支援PPSE，繼續select */
			uszPPSE_APDUBit = VS_FALSE;
		}
	}
	else
	{
		/* pobTran的資料存入pobEmvTran */
		memset((char *)&pobEmvTran, 0x00, _TRANSACTION_OBJECT_SIZE_);
		memcpy((char *)&pobEmvTran, (char *)pobTran, _TRANSACTION_OBJECT_SIZE_);
		
		/* ================================================ */
		/* EMV AID Select */
		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "EMV_TxnAppSelect START!");
		
                ushRetVal = EMV_TxnAppSelect(uszSelectedAID, &uszSelectedAIDLen, uszLabel, &uszLabelLen);
		
		/* 將callback使用的pobEmvTran結構傳回流程在用的pobTran結構 */
		memcpy((char *)pobTran, (char *)&pobEmvTran, _TRANSACTION_OBJECT_SIZE_);
	
		if (ginDebug == VS_TRUE)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inFunc_BCD_to_ASCII(&szTemplate[0], &uszSelectedAID[0], uszSelectedAIDLen);
			memset(szDebug, 0x00, sizeof(szDebug));
			sprintf(szDebug, "SelectedAID:%s", szTemplate);
			inLogPrintf(AT, szDebug);

			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &uszLabel[0], uszLabelLen);
			memset(szDebug, 0x00, sizeof(szDebug));
			sprintf(szDebug, "Label:%s", szTemplate);
			inLogPrintf(AT, szDebug);
		}

		if (ushRetVal != d_EMVAPLIB_OK)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebug, 0x00, sizeof(szDebug));
				sprintf(szDebug, "EMV_TxnAppSelect:0X%04x", ushRetVal);
				inLogPrintf(AT, szDebug);
			}

			inChangeErrMsg = _ERROR_CODE_V3_NONE_;
			switch (ushRetVal)
			{
				case d_EMVAPLIB_ERR_FUNCTION_NOT_SUPPORTED:
				case d_EMVAPLIB_ERR_TERM_DATA_MISSING:
				case d_EMVAPLIB_ERR_CRITICAL_MISTAKES:
				case d_EMVAPLIB_ERR_DATA_BUFFER_EXCEEDED:
					inEMV_SetICCReadFailure(VS_TRUE);		/* 開啟FALL BACK */
					inChangeErrMsg = _ERROR_CODE_V3_EMV_FALLBACK_;
					break;
				case d_EMVAPLIB_ERR_EVENT_CONFIRMED:
				case d_EMVAPLIB_ERR_EVENT_SELECTED:
				case d_EMVAPLIB_ERR_EVENT_GET_TXNDATA:
				case d_EMVAPLIB_ERR_EVENT_VERSION:
					inEMV_SetICCReadFailure(VS_TRUE);		/* 開啟FALL BACK */
					inChangeErrMsg = _ERROR_CODE_V3_EMV_FALLBACK_;
					break;
				case d_EMVAPLIB_ERR_ONLY_1_AP_NO_FALLBACK:
					inChangeErrMsg = _ERROR_CODE_V3_EMV_FALLBACK_;
					break;
				/* 找不到該AID的AP，不開啟Fallback */
				case d_EMVAPLIB_ERR_NO_AP_FOUND:
					break;
				case d_SC_NOT_PRESENT:
				case d_EMVAPLIB_ERR_SEND_APDU_CMD_FAIL:
					/* 晶片卡被取出 */
					inChangeErrMsg = _ERROR_CODE_V3_EMV_CARD_OUT_;
					break;
				case d_EMVAPLIB_ERR_SELECTION_FAIL:
					break;
				default:
					inEMV_SetICCReadFailure(VS_TRUE);		/* 開啟FALL BACK */
					if (inEMV_ICCEvent() != VS_SUCCESS)
					{
						/* 晶片卡被取出 */
						inChangeErrMsg = _ERROR_CODE_V3_EMV_CARD_OUT_;
					}
					else
					{
						inChangeErrMsg = _ERROR_CODE_V3_EMV_FALLBACK_;
					}
					break;
			}
			
			if (pobTran->inErrorMsg == _ERROR_CODE_V3_NONE_ && inChangeErrMsg != _ERROR_CODE_V3_NONE_)
			{
				pobTran->inErrorMsg = inChangeErrMsg;
			}
			pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;

			return (VS_ERROR);
		}
		else
		{
			uszUseEMVKernelBit = VS_TRUE;
		}
	}
	
	/* 分析PPSE or PSE內容 */
	if (uszPPSE_APDUBit == VS_TRUE)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebug, 0x00, sizeof(szDebug));
			sprintf(szDebug, "PPSE ChipDataLen = %d", srAPDU_COMMAND.inRecevLen);
			inLogPrintf(AT, szDebug);

			memset(szASCII, 0x00, sizeof(szASCII));
			inFunc_BCD_to_ASCII(&szASCII[0], &srAPDU_COMMAND.uszRecevData[0], srAPDU_COMMAND.inRecevLen);

			/* 把整個chipData印出來 */
			inLogPrintf(AT, "0123456789012345678901234567890123456789");
			inLogPrintf(AT, "========================================");
			memset(szDebug, 0x00, sizeof(szDebug));

			for (inCnt = 0; inCnt < (srAPDU_COMMAND.inRecevLen * 2); inCnt++)
			{
				memcpy(&szDebug[i], &szASCII[inCnt], 1);

				i ++;

				if (i == 40)
				{
					inLogPrintf(AT, szDebug);
					memset(szDebug, 0x00, sizeof(szDebug));
					i = 0;
				}
			}

			inLogPrintf(AT, szDebug);
			inLogPrintf(AT, "----------------------------------------");

			inCnt = 0;
			i = 0;
		}
		
		inLevel = 0;
		inCnt = 0;
		inRemainLen = srAPDU_COMMAND.inRecevLen;
		
		if (inContactType == _CONTACT_TYPE_01_CONTACTLESS_)
		{
			while (inRemainLen > 0)
			{
				memset(szTag, 0x00, sizeof(szTag));
				inTagLen = 0;
				uszTag2ByteBit = VS_FALSE;

				if (srAPDU_COMMAND.uszRecevData[inCnt] == (unsigned char)'\x8F'	||
				    srAPDU_COMMAND.uszRecevData[inCnt] == (unsigned char)'\xBF')
				{
					memcpy(szTag, &srAPDU_COMMAND.uszRecevData[inCnt], 2);
					inTagLen = srAPDU_COMMAND.uszRecevData[inCnt + 2];
					uszTag2ByteBit = VS_TRUE;
				}
				else
				{
					szTag[0] = srAPDU_COMMAND.uszRecevData[inCnt];
					inTagLen = srAPDU_COMMAND.uszRecevData[inCnt + 1];
				}

				/* 先找6F */
				if (inLevel == 0)
				{
					if (szTag[0] == '\x6F')
					{
						if (uszTag2ByteBit == VS_TRUE)
						{
							inCnt += 3;
						}
						else
						{
							inCnt += 2;
						}
						inRemainLen = inTagLen;
						inLevel++;
						
						if (ginDebug == VS_TRUE)
						{
							inLogPrintf(AT, "Find 6F");
						}
					}
					else
					{
						if (uszTag2ByteBit == VS_TRUE)
						{
							inCnt += inTagLen + 3;
							inRemainLen -= inTagLen + 3;
						}
						else
						{
							inCnt += inTagLen + 2;
							inRemainLen -= inTagLen + 2;
						}

					}
				}
				/* 再找A5 */
				else if (inLevel == 1)
				{
					if (szTag[0] == '\xA5')
					{
						if (uszTag2ByteBit == VS_TRUE)
						{
							inCnt += 3;
						}
						else
						{
							inCnt += 2;
						}
						inRemainLen = inTagLen;
						inLevel++;
						
						if (ginDebug == VS_TRUE)
						{
							inLogPrintf(AT, "Find A5");
						}
					}
					else
					{
						if (uszTag2ByteBit == VS_TRUE)
						{
							inCnt += inTagLen + 3;
							inRemainLen -= inTagLen + 3;
						}
						else
						{
							inCnt += inTagLen + 2;
							inRemainLen -= inTagLen + 2;
						}

					}
				}
				/* 再找BF 0C */
				else if (inLevel == 2)
				{
					if (memcmp(szTag, "\xBF\x0C", 2) == 0)
					{
						if (uszTag2ByteBit == VS_TRUE)
						{
							inCnt += 3;
						}
						else
						{
							inCnt += 2;
						}
						inRemainLen = inTagLen;
						inLevel++;
						
						if (ginDebug == VS_TRUE)
						{
							inLogPrintf(AT, "Find BF0C");
						}
					}
					else
					{
						if (uszTag2ByteBit == VS_TRUE)
						{
							inCnt += inTagLen + 3;
							inRemainLen -= inTagLen + 3;
						}
						else
						{
							inCnt += inTagLen + 2;
							inRemainLen -= inTagLen + 2;
						}

					}
				}
				/* 再找61 */
				else if (inLevel == 3)
				{
					if (szTag[0] == '\x61')
					{
						if (uszTag2ByteBit == VS_TRUE)
						{
							inCnt += 3;
						}
						else
						{
							inCnt += 2;
						}
						inRemainLen = inTagLen;
						inLevel++;
						
						if (ginDebug == VS_TRUE)
						{
							inLogPrintf(AT, "Find 61");
						}
					}
					else
					{
						if (uszTag2ByteBit == VS_TRUE)
						{
							inCnt += inTagLen + 3;
							inRemainLen -= inTagLen + 3;
						}
						else
						{
							inCnt += inTagLen + 2;
							inRemainLen -= inTagLen + 2;
						}

					}
				}
				/* 再找4F */
				else if (inLevel == 4)
				{
					if (szTag[0] == '\x4F')
					{
						if (uszTag2ByteBit == VS_TRUE)
						{
							inCnt += 3;
						}
						else
						{
							inCnt += 2;
						}
						inRemainLen = inTagLen;
						inLevel++;
						uszFind4FBit = VS_TRUE;
						
						if (ginDebug == VS_TRUE)
						{
							inLogPrintf(AT, "Find 4F");
						}
						
						break;
					}
					else
					{
						if (uszTag2ByteBit == VS_TRUE)
						{
							inCnt += inTagLen + 3;
							inRemainLen -= inTagLen + 3;
						}
						else
						{
							inCnt += inTagLen + 2;
							inRemainLen -= inTagLen + 2;
						}

					}
				}
			}
		}
		else
		{
			while (inRemainLen > 0)
			{
				memset(szTag, 0x00, sizeof(szTag));
				inTagLen = 0;
				uszTag2ByteBit = VS_FALSE;

				if (srAPDU_COMMAND.uszRecevData[inCnt] == (unsigned char)'\x8F'	||
				    srAPDU_COMMAND.uszRecevData[inCnt] == (unsigned char)'\xBF')
				{
					memcpy(szTag, &srAPDU_COMMAND.uszRecevData[inCnt], 2);
					inTagLen = srAPDU_COMMAND.uszRecevData[inCnt + 2];
					uszTag2ByteBit = VS_TRUE;
				}
				else
				{
					szTag[0] = srAPDU_COMMAND.uszRecevData[inCnt];
					inTagLen = srAPDU_COMMAND.uszRecevData[inCnt + 1];
				}

				/* 先找70 */
				if (inLevel == 0)
				{
					if (szTag[0] == '\x70')
					{
						if (uszTag2ByteBit == VS_TRUE)
						{
							inCnt += 3;
						}
						else
						{
							inCnt += 2;
						}
						inRemainLen = inTagLen;
						inLevel++;
						
						if (ginDebug == VS_TRUE)
						{
							inLogPrintf(AT, "Find 70");
						}
					}
					else
					{
						if (uszTag2ByteBit == VS_TRUE)
						{
							inCnt += inTagLen + 3;
							inRemainLen -= inTagLen + 3;
						}
						else
						{
							inCnt += inTagLen + 2;
							inRemainLen -= inTagLen + 2;
						}

					}
				}
				/* 再找61 */
				else if (inLevel == 1)
				{
					if (szTag[0] == '\x61')
					{
						if (uszTag2ByteBit == VS_TRUE)
						{
							inCnt += 3;
						}
						else
						{
							inCnt += 2;
						}
						inRemainLen = inTagLen;
						inLevel++;
						
						if (ginDebug == VS_TRUE)
						{
							inLogPrintf(AT, "Find 61");
						}
					}
					else
					{
						if (uszTag2ByteBit == VS_TRUE)
						{
							inCnt += inTagLen + 3;
							inRemainLen -= inTagLen + 3;
						}
						else
						{
							inCnt += inTagLen + 2;
							inRemainLen -= inTagLen + 2;
						}

					}
				}
				/* 再找4F */
				else if (inLevel == 2)
				{
					if (szTag[0] == '\x4F')
					{
						if (uszTag2ByteBit == VS_TRUE)
						{
							inCnt += 3;
						}
						else
						{
							inCnt += 2;
						}
						inRemainLen = inTagLen;
						inLevel++;
						uszFind4FBit = VS_TRUE;
						
						if (ginDebug == VS_TRUE)
						{
							inLogPrintf(AT, "Find 4F");
						}
						
						break;
					}
					else
					{
						if (uszTag2ByteBit == VS_TRUE)
						{
							inCnt += inTagLen + 3;
							inRemainLen -= inTagLen + 3;
						}
						else
						{
							inCnt += inTagLen + 2;
							inRemainLen -= inTagLen + 2;
						}

					}
				}
			}
		}
	}
	else if (uszUseEMVKernelBit == VS_TRUE)
	{
		/* 4F (AID) */
		ushTagLen = sizeof(uszTagData);
		memset(uszTagData, 0x00, sizeof(uszTagData));
		inRetVal = EMV_DataGet(d_TAG_AID, &ushTagLen, uszTagData);

		if (inRetVal == d_EMVAPLIB_OK  && ushTagLen > 0)
		{
			/* 一律存AID */
			memset(pobTran->srBRec.szCUP_EMVAID, 0x00, sizeof(pobTran->srBRec.szCUP_EMVAID));
			inFunc_BCD_to_ASCII(pobTran->srBRec.szCUP_EMVAID, uszTagData, ushTagLen);
			uszFind4FBit = VS_TRUE;

		}
	}
		
	if (uszFind4FBit == VS_TRUE)
	{
		memset(szHexAID, 0x00, sizeof(szHexAID));
		if (uszUseAPDUBit == VS_TRUE)
		{
			memcpy(szHexAID, &srAPDU_COMMAND.uszRecevData[inCnt], inTagLen);
		}
		else if (uszUseEMVKernelBit == VS_TRUE)
		{
			inFunc_ASCII_to_BCD((unsigned char*)szHexAID, pobTran->srBRec.szCUP_EMVAID, strlen(pobTran->srBRec.szCUP_EMVAID));
		}

		if(!memcmp(&szHexAID[0], _EMV_AID_VISA_HEX_, _EMV_AID_VISA_HEX_LEN_)			|| 
		   !memcmp(&szHexAID[0], _EMV_AID_MASTERCARD_HEX_, _EMV_AID_MASTERCARD_HEX_LEN_)	|| 
		   !memcmp(&szHexAID[0], _EMV_AID_JCB_HEX_, _EMV_AID_JCB_HEX_LEN_)			||
		   !memcmp(&szHexAID[0], _EMV_AID_AMEX_HEX_, _EMV_AID_AMEX_HEX_LEN_)			||
		   !memcmp(&szHexAID[0], _EMV_AID_DISCOVER_HEX_, _EMV_AID_DISCOVER_HEX_LEN_)		||
		   !memcmp(&szHexAID[0], _EMV_AID_TWIN_HEX_, _EMV_AID_TWIN_HEX_LEN_))
		{
			/* 確認為信用卡 */
			pobTran->uszCreditBit = VS_TRUE;

			if (!memcmp(&szHexAID[0], _EMV_AID_TWIN_HEX_, _EMV_AID_TWIN_HEX_LEN_))
			{
				pobTran->uszUCardBit = VS_TRUE;
			}
		}

		if(!memcmp(&szHexAID[0], _EMV_AID_CUP_HEX_, _EMV_AID_CUP_HEX_LEN_))
		{
			/* 確認為銀聯卡 */
			pobTran->uszUICCBit = VS_TRUE;
		}

		memset(szBinCheckAID, 0x00, sizeof(szBinCheckAID));
		memcpy(&szBinCheckAID[0], &szHexAID[0], inAIDLen);

		if (inContactType == _CONTACT_TYPE_01_CONTACTLESS_	&&
		    !memcmp(&szHexAID[0], _EMV_AID_JCB_HEX_, _EMV_AID_JCB_HEX_LEN_))
		{
			guszStrangeJCBCard = VS_TRUE;
		}
	}
	/* 如果PPSEselect失敗或PPSE解失敗用老方法一個一個AID select */
	else
	{
		for (i = 0 ;; i ++)
		{
			if (inLoadMVTRec(i) < 0) /* 主機參數檔 */
				break;

			memset(szMVTAID, 0x00, sizeof(szMVTAID));
			memset(szHexAID, 0x00, sizeof(szHexAID));
			inGetMVTApplicationId(szMVTAID);
			inAIDLen = strlen(szMVTAID) / 2;
			inFunc_ASCII_to_BCD((unsigned char*)szHexAID, szMVTAID, inAIDLen);

			/* APDU Command */
			memset(&srAPDU_COMMAND, 0x00, sizeof(APDU_COMMAND));

			srAPDU_COMMAND.uszCommandINSData[0] = _FISC_SELECT_AID_CLA_COMMAND_;		/* CLA */
			srAPDU_COMMAND.uszCommandINSData[0] = _FISC_SELECT_AID_INS_COMMAND_;		/* INS */
			srAPDU_COMMAND.uszCommandP1Data[0] = _FISC_SELECT_AID_P1_COMMAND_;		/* P1 */
			srAPDU_COMMAND.uszCommandP2Data[0] = _FISC_SELECT_AID_P2_COMMAND_;		/* P2 */
			srAPDU_COMMAND.inCommandDataLen = inAIDLen;
			memcpy(srAPDU_COMMAND.uszCommandData, szHexAID, inAIDLen);

			if (inAPDU_BuildAPDU(&srAPDU_COMMAND) != VS_SUCCESS)
			{
				return (VS_ERROR);
			}

			/* 收送Command */
			if (inContactType == _CONTACT_TYPE_01_CONTACTLESS_)
			{
				inRetVal = inAPDU_Send_APDU_CTLS_Process(&srAPDU_COMMAND);
			}
			else
			{
				inRetVal = inAPDU_Send_APDU_User_Slot_Process(&srAPDU_COMMAND);
			}

			if (inRetVal == VS_SUCCESS)
			{
				/* 6283 是鎖卡，需要到EMV執行步驟再回傳錯誤訊息 d_EMVAPLIB_ERR_ONLY_1_AP_NO_FALLBACK */
				if ((srAPDU_COMMAND.uszRecevData[srAPDU_COMMAND.inRecevLen -2] == 0x90 && srAPDU_COMMAND.uszRecevData[srAPDU_COMMAND.inRecevLen -1] == 0x00)	||
				    (srAPDU_COMMAND.uszRecevData[srAPDU_COMMAND.inRecevLen -2] == 0x62 && srAPDU_COMMAND.uszRecevData[srAPDU_COMMAND.inRecevLen -1] == 0x83))
				{	
					if(!memcmp(&szHexAID[0], _EMV_AID_VISA_HEX_, _EMV_AID_VISA_HEX_LEN_)			|| 
					   !memcmp(&szHexAID[0], _EMV_AID_MASTERCARD_HEX_, _EMV_AID_MASTERCARD_HEX_LEN_)	|| 
					   !memcmp(&szHexAID[0], _EMV_AID_JCB_HEX_, _EMV_AID_JCB_HEX_LEN_)			||
					   !memcmp(&szHexAID[0], _EMV_AID_AMEX_HEX_, _EMV_AID_AMEX_HEX_LEN_)			||
					   !memcmp(&szHexAID[0], _EMV_AID_DISCOVER_HEX_, _EMV_AID_DISCOVER_HEX_LEN_)		||
					   !memcmp(&szHexAID[0], _EMV_AID_TWIN_HEX_, _EMV_AID_TWIN_HEX_LEN_))
					{
						/* 確認為信用卡 */
						pobTran->uszCreditBit = VS_TRUE;

						if (!memcmp(&szHexAID[0], _EMV_AID_TWIN_HEX_, _EMV_AID_TWIN_HEX_LEN_))
						{
							pobTran->uszUCardBit = VS_TRUE;
						}
					}
					if(!memcmp(&szHexAID[0], _EMV_AID_CUP_HEX_, _EMV_AID_CUP_HEX_LEN_))
					{
						/* 確認為銀聯卡 */
						pobTran->uszUICCBit = VS_TRUE;
					}

					memset(szBinCheckAID, 0x00, sizeof(szBinCheckAID));
					memcpy(&szBinCheckAID[0], &szHexAID[0], inAIDLen);
				}
				else
				{
					if (ginDebug == VS_TRUE)
						inLogPrintf(AT, "FISC Select AID != 9000");
				}

			}
			else
			{
				/* 下Command失敗 */
				return (inRetVal);
			}

		}
	}

	/* 【需求單 - 108097】Card BIN由ATS查核 add by LingHsiung 2020/2/3 下午 5:19 */
	memset(szBinCheck, 0x00, sizeof(szBinCheck));
	inGetBIN_CHECK(szBinCheck);
	
	/* 由ATS主機檢核的話晶片和感應卡先塞CardLabel 後面自訂義cardbin不存CardLabel */
	if (memcmp(szBinCheck, "A", strlen("A")) == 0)
	{
		if (!memcmp(&szBinCheckAID[0], "\xA0\x00\x00\x00\x03", 5))
		{
			memcpy(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_VISA_, strlen(_CARD_TYPE_VISA_));
		}
		else if (!memcmp(&szBinCheckAID[0], "\xA0\x00\x00\x00\x04", 5))
		{
			memcpy(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_));
		}
		else if (!memcmp(&szBinCheckAID[0], "\xA0\x00\x00\x00\x65", 5))
		{
			memcpy(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_JCB_, strlen(_CARD_TYPE_JCB_));
		}
		else if (!memcmp(&szBinCheckAID[0], "\xA0\x00\x00\x00\x25", 5))
		{
			memcpy(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_));
		}
		else if (!memcmp(&szBinCheckAID[0], "\xA0\x00\x00\x01\x52", 5))
		{
			memcpy(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_));
		}
		else if (!memcmp(&szBinCheckAID[0], "\xA0\x00\x00\x03\x33", 5))
		{
			memcpy(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_CUP_, strlen(_CARD_TYPE_CUP_));
		}
	}

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inEMV_CreditSelectAID()_END");
        }

        return (VS_SUCCESS);
}

int inEMV_SelectICCAID(TRANSACTION_OBJECT *pobTran)
{        
        int	inRetVal;
	char	szDebugMsg[100 + 1];
	char	szCUPFunctionEnable[2 + 1];
	char	szFiscFunctionEnable[2 + 1];
	char	szMACEnable[2 + 1];
	

	/* 初始化 */
	pobTran->uszCreditBit = VS_FALSE;
	pobTran->uszUICCBit = VS_FALSE;
	pobTran->uszFISCBit = VS_FALSE;
        
	/* ================================================ */
	/* 先檢查SmartPay功能是否可用 不能就直接跳過跑信用卡EMV */
        
	/* 有開Smartpay，才去Select Smartpay AID */
	if (inFISC_CheckSmartPayEnable() != VS_SUCCESS)
	{

	}
	else
	{
		inRetVal = inFLOW_RunFunction(pobTran, _FISC_CARD_POWER_ON_AND_SELECT_AID_);
		inFLOW_RunFunction(pobTran, _FISC_CARD_POWER_OFF_);
	}
        /* ================================================ */
	inRetVal = inEMV_CreditPowerON(pobTran);
        if (inRetVal != VS_SUCCESS)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inEMV_CreditPowerON_ERR");
		
		if (inRetVal == VS_EMV_CARD_OUT)
		{
			/* 晶片卡被取出 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_EMV_CARD_OUT_;
		}
		else
		{
			pobTran->inErrorMsg = _ERROR_CODE_V3_EMV_FALLBACK_;
		}
                
                return (VS_ERROR);
        }
        
	inRetVal = inEMV_CreditSelectAID(pobTran, _CONTACT_TYPE_00_CONTACT_);
	
        if (inRetVal != VS_SUCCESS)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inEMV_CreditSelectAID_ERR");
		
		if (inRetVal == VS_EMV_CARD_OUT)
		{
			/* 晶片卡被取出 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_EMV_CARD_OUT_;
			return (VS_ERROR);
		}
        }
        
        inEMV_CreditPowerOFF(pobTran);
        /* ================================================ */
        
	/* 擋按銀聯鍵但插非銀聯晶片卡 */
	if (pobTran->srBRec.uszCUPTransBit == VS_TRUE && pobTran->uszUICCBit != VS_TRUE)
	{
		/* 請勿按銀聯鍵”(第一行)請按清除鍵 */
		pobTran->inErrorMsg = _ERROR_CODE_V3_DO_NOT_CUP_KEY_;
		
		return (VS_ERROR);
	}
	
        /* 其中有Select到AID代表有抓到其中一組 */
        if (pobTran->uszCreditBit == VS_TRUE && pobTran->uszFISCBit == VS_TRUE && pobTran->uszUICCBit == VS_FALSE)
        {	
		/* 已經選了SmartPay相關交易不要進選單 */
		if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
		{
			inRetVal = VS_SUCCESS;
			pobTran->uszFISCBit = VS_TRUE;
			pobTran->uszCreditBit = VS_FALSE;
		}
		else
		{
			inRetVal = inFISC_Select_Menu(pobTran);
		}
                
		if (inRetVal == VS_SUCCESS)
		{
			if (pobTran->uszCreditBit == VS_TRUE)
			{
					if (pobTran->inTransactionCode == _SALE_)
					{
						pobTran->inRunTRTID = _TRT_SALE_ICC_;
					}
					else if (pobTran->inTransactionCode == _PRE_AUTH_)
					{
						pobTran->inRunTRTID = _TRT_PRE_AUTH_ICC_;
					}
					else if (pobTran->inTransactionCode == _INST_SALE_)
					{
						pobTran->inRunTRTID = _TRT_INST_SALE_ICC_;
					}
					else if (pobTran->inTransactionCode == _REDEEM_SALE_)
					{
						pobTran->inRunTRTID = _TRT_REDEEM_SALE_ICC_;
					}
					else
					{
                                                /* Mirror Message */
                                                if (pobTran->uszECRBit == VS_TRUE)
                                                {
                                                        inECR_SendMirror(pobTran, _MIRROR_MSG_WRONG_CARD_ERROR_);
                                                }
                                                
						/* 請依正確卡別操作 */
						DISPLAY_OBJECT	srDispMsgObj;
						memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
						strcpy(srDispMsgObj.szDispPic1Name, _ERR_PLEASE_FOLLOW_RIGHT_OPT_);
						srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_5_;
						srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
						srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
						strcpy(srDispMsgObj.szErrMsg1, "");
						srDispMsgObj.inErrMsg1Line = 0;
						srDispMsgObj.inBeepTimes = 1;
						srDispMsgObj.inBeepInterval = 0;
						inDISP_Msg_BMP(&srDispMsgObj);

						if (ginDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
							sprintf(szDebugMsg, "AID_Select But Cat not Select TRT, inCode: %d", pobTran->srBRec.inCode);
							inLogPrintf(AT, szDebugMsg);
						}

						return (VS_ERROR);
					}

					/* NCCC */
					pobTran->srBRec.inHDTIndex = 0;
					inLoadHDTRec(pobTran->srBRec.inHDTIndex);
					inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
					
					/* 在這裡重啟信用卡 */
					inRetVal = inEMV_CreditPowerON(pobTran);
					if (inRetVal != VS_SUCCESS)
					{
						if (ginDebug == VS_TRUE)
							inLogPrintf(AT, "inEMV_CreditPowerON_ERR");

						if (inRetVal == VS_EMV_CARD_OUT)
						{
							/* 晶片卡被取出 */
							pobTran->inErrorMsg = _ERROR_CODE_V3_EMV_CARD_OUT_;
						}
						else
						{
							pobTran->inErrorMsg = _ERROR_CODE_V3_EMV_FALLBACK_;
						}

						return (VS_ERROR);
					}

					inRetVal = inEMV_CreditSelectAID(pobTran, _CONTACT_TYPE_00_CONTACT_);

					if (inRetVal != VS_SUCCESS)
					{
						if (ginDebug == VS_TRUE)
							inLogPrintf(AT, "inEMV_CreditSelectAID_ERR");

						if (inRetVal == VS_EMV_CARD_OUT)
						{
							/* 晶片卡被取出 */
							pobTran->inErrorMsg = _ERROR_CODE_V3_EMV_CARD_OUT_;
							return (VS_ERROR);
						}
					}

					if (ginDebug == VS_TRUE)
						inLogPrintf(AT, "Select = CreditBit");

			}
			else if (pobTran->uszFISCBit == VS_TRUE)
			{
				/* IDLE進入，一開始預設是_SALE_ */
				if (pobTran->inTransactionCode == _SALE_)
				{
					pobTran->inTransactionCode = _FISC_SALE_;
					pobTran->srBRec.inCode = pobTran->inTransactionCode;
					pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
				}

				if (pobTran->inTransactionCode == _FISC_SALE_)
				{
					pobTran->inRunTRTID = _TRT_FISC_SALE_ICC_;
				}
				else if (pobTran->inTransactionCode == _FISC_REFUND_)
				{
					pobTran->inRunTRTID = _TRT_FISC_REFUND_ICC_;
				}
				else
				{
                                        /* Mirror Message */
                                        if (pobTran->uszECRBit == VS_TRUE)
                                        {
                                                inECR_SendMirror(pobTran, _MIRROR_MSG_WRONG_CARD_ERROR_);
                                        }
                                                
					/* 請依正確卡別操作 */
					DISPLAY_OBJECT	srDispMsgObj;
					memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
					strcpy(srDispMsgObj.szDispPic1Name, _ERR_PLEASE_FOLLOW_RIGHT_OPT_);
					srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_5_;
					srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
					srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
					strcpy(srDispMsgObj.szErrMsg1, "");
					srDispMsgObj.inErrMsg1Line = 0;
					srDispMsgObj.inBeepTimes = 1;
					srDispMsgObj.inBeepInterval = 0;
					inDISP_Msg_BMP(&srDispMsgObj);

					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "AID_Select But Cat not Select TRT, inCode: %d", pobTran->inTransactionCode);
						inLogPrintf(AT, szDebugMsg);
					}

					return (VS_ERROR);
				}

				/* NCCC SMARTPAY同一個HOST */
				pobTran->srBRec.inHDTIndex = 0;
				inLoadHDTRec(pobTran->srBRec.inHDTIndex);
				inLoadHDPTRec(pobTran->srBRec.inHDTIndex);

				pobTran->srBRec.uszFiscTransBit = VS_TRUE;
				/* SmartPay不用簽名 */
				if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
				{
					pobTran->srBRec.uszNoSignatureBit = VS_TRUE;
				}

				/* SMARTPAY要GEN MAC來算TCC，一定要安全認證 */
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

				 /* 要再重啟一次 */
				if (inFISC_PowerON(pobTran) != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
						inLogPrintf(AT, "inFISC_Func_PowerON_ERR");

					return (VS_ERROR);
				}

				if (inFISC_SelectAID(pobTran) != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
						inLogPrintf(AT, "inFISC_Func_SelectAID_ERR");

					return (VS_ERROR);
				}
				
				if (ginDebug == VS_TRUE)
					inLogPrintf(AT, "Select = FISCBit");
			}
			else
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "Selec Failed");
				}
				return (VS_ERROR);
			}
			
			return (VS_SUCCESS);
		}
		else
		{
			return (inRetVal);
		}
                
                
        }
        else if ((pobTran->uszCreditBit == VS_FALSE && pobTran->uszFISCBit == VS_FALSE && pobTran->uszUICCBit == VS_TRUE) ||
                 (pobTran->uszCreditBit == VS_TRUE && pobTran->uszFISCBit == VS_FALSE && pobTran->uszUICCBit == VS_TRUE))
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "Select = UICCBit");
		
		/* 若銀聯功能沒開，擋掉 */
		/* 若EDC的CUPFunctionEnable 和 MACEnable 未開，顯示此功能以關閉 */
		memset(szCUPFunctionEnable, 0x00, sizeof(szCUPFunctionEnable));
		inGetCUPFuncEnable(szCUPFunctionEnable);
		memset(szMACEnable, 0x00, sizeof(szMACEnable));
		inGetMACEnable(szMACEnable);

		/* 沒開CUP */
		if ((memcmp(&szCUPFunctionEnable[0], "Y", 1) != 0) || memcmp(szMACEnable, "Y", 1) != 0)
		{
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_CUP_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜銀聯一般交易＞ */
			/* 此功能已關閉 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_FUNC_CLOSE_;

			return (VS_ERROR);
		}
	
		/* IDLE進入，一開始預設是_SALE_ */
		if (pobTran->inTransactionCode == _SALE_)
		{
			pobTran->inTransactionCode = _CUP_SALE_;
			pobTran->srBRec.inCode = pobTran->inTransactionCode;
			pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
		}
		else if (pobTran->inTransactionCode == _PRE_AUTH_)
		{
			pobTran->inTransactionCode = _CUP_PRE_AUTH_;
			pobTran->srBRec.inCode = pobTran->inTransactionCode;
			pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
		}
		
                if (pobTran->inTransactionCode == _CUP_SALE_)
		{
			pobTran->inRunTRTID = _TRT_CUP_SALE_ICC_;
		}
		else if (pobTran->inTransactionCode == _CUP_PRE_AUTH_)
		{
			pobTran->inRunTRTID = _TRT_CUP_PRE_AUTH_ICC_;
		}
		else
		{
                        /* Mirror Message */
                        if (pobTran->uszECRBit == VS_TRUE)
                        {
                                inECR_SendMirror(pobTran, _MIRROR_MSG_WRONG_CARD_ERROR_);
                        }
                                                
			/* 請依正確卡別操作 */
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_PLEASE_FOLLOW_RIGHT_OPT_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_5_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "AID_Select But Cat not Select TRT, inCode: %d", pobTran->inTransactionCode);
				inLogPrintf(AT, szDebugMsg);
			}
			
			return (VS_ERROR);
		}
		
		/* NCCC CUP同一個HOST */
		pobTran->srBRec.inHDTIndex = 0;
		inLoadHDTRec(pobTran->srBRec.inHDTIndex);
		inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
			
                pobTran->srBRec.uszCUPTransBit = VS_TRUE;
		
		/* 確認是銀聯卡，檢查是否已做安全認證 */
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
		
		/* 在這裡重啟信用卡 */
		inRetVal = inEMV_CreditPowerON(pobTran);
		if (inRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
				inLogPrintf(AT, "inEMV_CreditPowerON_ERR");

			if (inRetVal == VS_EMV_CARD_OUT)
			{
				/* 晶片卡被取出 */
				pobTran->inErrorMsg = _ERROR_CODE_V3_EMV_CARD_OUT_;
			}
			else
			{
				pobTran->inErrorMsg = _ERROR_CODE_V3_EMV_FALLBACK_;
			}

			return (VS_ERROR);
		}

		inRetVal = inEMV_CreditSelectAID(pobTran, _CONTACT_TYPE_00_CONTACT_);

		if (inRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
				inLogPrintf(AT, "inEMV_CreditSelectAID_ERR");

			if (inRetVal == VS_EMV_CARD_OUT)
			{
				/* 晶片卡被取出 */
				pobTran->inErrorMsg = _ERROR_CODE_V3_EMV_CARD_OUT_;
				return (VS_ERROR);
			}
		}
		
                return (VS_SUCCESS);
        }
        else if (pobTran->uszCreditBit == VS_FALSE && pobTran->uszFISCBit == VS_TRUE && pobTran->uszUICCBit == VS_FALSE)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "Select = FISCBit");
		
		/* 若EDC的FiscFunctionEnable未開，顯示此功能以關閉 */
		memset(szFiscFunctionEnable, 0x00, sizeof(szFiscFunctionEnable));
		inGetFiscFuncEnable(szFiscFunctionEnable);

		/* 沒開Fisc */
		if ((memcmp(szFiscFunctionEnable, "Y", 1) != 0))
		{
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_SMARTPAY_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);	/* 第一層顯示 Smartpay */
			/* 此功能已關閉 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_FUNC_CLOSE_;

			return (VS_ERROR);
		}
		
		/* IDLE進入，一開始預設是_SALE_ */
		if (pobTran->inTransactionCode == _SALE_)
		{
			pobTran->inTransactionCode = _FISC_SALE_;
			pobTran->srBRec.inCode = pobTran->inTransactionCode;
			pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
		}
		
		if (pobTran->inTransactionCode == _FISC_SALE_)
		{
			pobTran->inRunTRTID = _TRT_FISC_SALE_ICC_;
		}
		else if (pobTran->inTransactionCode == _FISC_REFUND_)
		{
			pobTran->inRunTRTID = _TRT_FISC_REFUND_ICC_;
		}
		else
		{
                        /* Mirror Message */
                        if (pobTran->uszECRBit == VS_TRUE)
                        {
                                inECR_SendMirror(pobTran, _MIRROR_MSG_WRONG_CARD_ERROR_);
                        }
                                                
			/* 請依正確卡別操作 */
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_PLEASE_FOLLOW_RIGHT_OPT_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_5_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
		
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "AID_Select But Cat not Select TRT, inCode: %d", pobTran->inTransactionCode);
				inLogPrintf(AT, szDebugMsg);
			}
			
			return (VS_ERROR);
		}
		
		/* NCCC SMARTPAY同一個HOST */
		pobTran->srBRec.inHDTIndex = 0;
		inLoadHDTRec(pobTran->srBRec.inHDTIndex);
		inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
		
                pobTran->srBRec.uszFiscTransBit = VS_TRUE;
		/* SmartPay不用簽名 */
		if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
		{
			pobTran->srBRec.uszNoSignatureBit = VS_TRUE;
		}
		
		/* SMARTPAY要GEN MAC來算TCC，一定要安全認證 */
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
                
                /* 要再重啟一次 */
                if (inFISC_PowerON(pobTran) != VS_SUCCESS)
                {
                        if (ginDebug == VS_TRUE)
                                inLogPrintf(AT, "inFISC_Func_PowerON_ERR");

                        return (VS_ERROR);
                }

                if (inFISC_SelectAID(pobTran) != VS_SUCCESS)
                {
                        if (ginDebug == VS_TRUE)
                                inLogPrintf(AT, "inFISC_Func_SelectAID_ERR");
                        
                        return (VS_ERROR);
                }
                
                return (VS_SUCCESS);
        }       
        else if (pobTran->uszCreditBit == VS_TRUE && pobTran->uszFISCBit == VS_FALSE && pobTran->uszUICCBit == VS_FALSE)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "Select = CreditBit");
                
		if (pobTran->inTransactionCode == _SALE_)
		{
			pobTran->inRunTRTID = _TRT_SALE_ICC_;
		}
		else if (pobTran->inTransactionCode == _PRE_AUTH_)
		{
			pobTran->inRunTRTID = _TRT_PRE_AUTH_ICC_;
		}
		else if (pobTran->inTransactionCode == _INST_SALE_)
		{
			pobTran->inRunTRTID = _TRT_INST_SALE_ICC_;
		}
		else if (pobTran->inTransactionCode == _REDEEM_SALE_)
		{
			pobTran->inRunTRTID = _TRT_REDEEM_SALE_ICC_;
		}
		/* 有設定HG交易別 */
		else if (pobTran->srBRec.lnHGTransactionType != 0)
		{
			/* HG交易的TRT由inHG_Func_SelectPaymentType決定 */
		}
		else if (pobTran->inTransactionCode == _CASH_ADVANCE_)
		{
			pobTran->inRunTRTID = _TRT_CASH_ADVANCE_ICC_;
		}
		else
		{
                        /* Mirror Message */
                        if (pobTran->uszECRBit == VS_TRUE)
                        {
                                inECR_SendMirror(pobTran, _MIRROR_MSG_WRONG_CARD_ERROR_);
                        }
                                                
			/* 請依正確卡別操作 */
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_PLEASE_FOLLOW_RIGHT_OPT_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_5_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "AID_Select But Cat not Select TRT, inCode: %d", pobTran->inTransactionCode);
				inLogPrintf(AT, szDebugMsg);
			}
			
			return (VS_ERROR);
		}
		
		/* NCCC同一個HOST */
		pobTran->srBRec.inHDTIndex = 0;
		inLoadHDTRec(pobTran->srBRec.inHDTIndex);
		inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
		
		/* 在這裡重啟信用卡 */
		inRetVal = inEMV_CreditPowerON(pobTran);
		if (inRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
				inLogPrintf(AT, "inEMV_CreditPowerON_ERR");

			if (inRetVal == VS_EMV_CARD_OUT)
			{
				/* 晶片卡被取出 */
				pobTran->inErrorMsg = _ERROR_CODE_V3_EMV_CARD_OUT_;
			}
			else
			{
				pobTran->inErrorMsg = _ERROR_CODE_V3_EMV_FALLBACK_;
			}

			return (VS_ERROR);
		}

		inRetVal = inEMV_CreditSelectAID(pobTran, _CONTACT_TYPE_00_CONTACT_);

		if (inRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
				inLogPrintf(AT, "inEMV_CreditSelectAID_ERR");

			if (inRetVal == VS_EMV_CARD_OUT)
			{
				/* 晶片卡被取出 */
				pobTran->inErrorMsg = _ERROR_CODE_V3_EMV_CARD_OUT_;
				return (VS_ERROR);
			}
		}
		
                return(VS_SUCCESS);
        }
        else if (pobTran->uszCreditBit == VS_FALSE && pobTran->uszUICCBit == VS_FALSE && pobTran->uszFISCBit == VS_FALSE)
        {
		/* 乙、	插銀聯晶片卡(select不到AID), 顯示”請退出晶片卡” */
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "AID_Select_Not_Found");
                }
                
                return (VS_ERROR);
        }
        
        return (VS_SUCCESS);
}

/* EMV CallBack Function */
/* 執行EMV_TxnAppSelect開始時會CALL這支function是否要顯示訊息 */
void OnDisplayShow(IN char *pStrMsg)
{
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "OnDisplayShow_START!!");
        }
	
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "OnDisplayShow_END!!");
        }
}
/* 執行EMV_TxnAppSelect若是有錯誤訊息要顯示最後面會CALL這支function */
void OnErrorMsg(IN char *pStrMsg)
{
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "OnErrorMsg_START!!");
        }

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "OnErrorMsg_END!!");
        }
}

/* 執行EMV_Initialize時會先CALL這支function，選擇要跑的Config */
void OnEMVConfigActive(INOUT BYTE* pActiveIndex)
{
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "OnEMVConfigActive_START!!");
        }

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "OnEMVConfigActive_END!!");
        }
}

/*
Function        :OnTxnDataGet
Date&Time       :2016/9/30 下午 2:26
Describe        :執行EMV_TxnAppSelect時選完後會要求輸入交易資料，如金額，這是必跑的CallBack
*/
USHORT OnTxnDataGet(OUT EMV_TXNDATA *pTxnData)
{
	char	szEMVForceOnline[2 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
	
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "OnTxnDataGet_START!!");
        }
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	pTxnData->Version = 0x01;
	pTxnData->ulAmount = pobEmvTran.srBRec.lnTxnAmount * 100;	/* 沒有小數點 要補2個0 */
	/* Default為0x00 不設好像不會怎麼樣 */
//	pTxnData->bPOSEntryMode = 0x00;
	/* YYMMDD */
	memcpy(pTxnData->TxnDate, &pobEmvTran.srBRec.szDate[2], 6);
	/* HHMMSS */
	memcpy(pTxnData->TxnTime, &pobEmvTran.srBRec.szTime[0], 6);

	memset(szEMVForceOnline, 0x00, sizeof(szEMVForceOnline));
	inGetEMVForceOnline(szEMVForceOnline);
	if (pobEmvTran.inTransactionCode == _PRE_AUTH_			||
	    pobEmvTran.inTransactionCode == _INST_SALE_			||
	    pobEmvTran.inTransactionCode == _REDEEM_SALE_		||
	    pobEmvTran.inTransactionCode == _CASH_ADVANCE_		||
	    pobEmvTran.srBRec.uszDCCTransBit == VS_TRUE			||
	    memcmp(szEMVForceOnline, "Y", strlen("Y")) == 0		||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_071_FORCE_ONLINE_, _CUSTOMER_INDICATOR_SIZE_)	||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_042_BDAU1_, _CUSTOMER_INDICATOR_SIZE_)		||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_043_BDAU9_, _CUSTOMER_INDICATOR_SIZE_))
	{
		/* DCC Sale和Pre-Auth因為流程關係，所以在OnEventTxnForcedOnline這個Special Event中On Force Online */
		/* 紅利分期、銀聯TRT、FORCEOnlineBit FORCE_ONLINE */
		pTxnData->isForceOnline = VS_TRUE;
	}
	else
		pTxnData->isForceOnline = VS_FALSE;
	
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "OnTxnDataGet_END!!");
        }

	return d_EMVAPLIB_OK;
}

/*
Function        :OnAppList
Date&Time       :2016/12/15 下午 3:52
Describe        :因為要看AID，所以OnAppList廢棄不用，改用OnEventAppListEx
 *		Range of pAppSelectedIndex value is 0 to (AppNum-1)
 *		執行EMV_TxnAppSelect時若是有多個AID會進入此function，要顯示AID列表供選擇
 */
USHORT OnAppList(IN BYTE AppNum, IN char AppLabel[][d_LABEL_STR_SIZE + 1], OUT BYTE *pAppSelectedIndex)
{
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "OnAppList_START!!");
        }
	
	/* 此Function廢棄不用，改用OnEventAppListEx，OnEventAppListEx 和 OnAppList若同時註冊，只會跑OnEventAppListEx */
       	       
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "OnAppList_END!!");
        }

	return d_EMVAPLIB_OK;
}

/* Return d_OK to indicate CONFIRMED */
/* 執行EMV_TxnAppSelect時若是只有一個AID會進入此function */
USHORT OnAppSelectedConfirm(IN BOOL IsRequiredbyCard, IN BYTE *pLabel, IN BYTE bLabelLen)
{
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "OnAppSelectedConfirm_START!!");
        }

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "OnAppSelectedConfirm_END!!");
        }

	return d_OK;
}

/* 執行EMV_TxnPerform時若是Kernel在XML找不到需要的TAG值會從這裡要求提供所需的TAG */
BOOL OnTerminalDataGet(IN USHORT usTag, INOUT USHORT *pLen, OUT BYTE *pValue)
{
	char	szDebugMsg[100 + 1];
	char	szSerialNumber[16 + 1];
	
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "OnTerminalDataGet_START!!");
        }
	
	/* d_TAG_IFD_SN 0x9F1E */
	if (usTag == d_TAG_IFD_SN)
	{
		memset(szSerialNumber, 0x00, sizeof(szSerialNumber));
		inFunc_GetSeriaNumber(szSerialNumber);
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "%s", szSerialNumber);
			inLogPrintf(AT, szDebugMsg);
		}
		/* 取後8碼，但最後一碼為CheckSum，所以取8~15 */
		*pLen = 8;
		memcpy((unsigned char*)pValue, &szSerialNumber[7], *pLen);
		
		return TRUE;
	}

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "OnTerminalDataGet_END!!");
        }

	return FALSE;
}

/* 執行EMV_TxnPerform時若是Kernel在XML找不到需要的CAPK KEY值會從這裡要求提供所需的CAPK KEY */
BOOL OnCAPKGet(IN BYTE *pRID, IN BYTE bKeyIndex, OUT BYTE *pModulus, OUT USHORT *pModulusLen, OUT BYTE *pExponent, OUT USHORT *pExponentLen)
{
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "OnCAPKGet_START!!");
        }

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "OnCAPKGet_END!!");
        }

	return FALSE;
}

/* 執行EMV_TxnPerform時若是要用Internal PINPAD輸入PIN會CALL這支，輸入完後交由KERNAL處理，若是輸入錯則流程繼續往下跑，除非重新交易才會再進來 */
/*
mail 2013/09/27 RE: V5S開發EMV流程相關問題:
3.我想確認執行Internal EMV Offline PIN是否都是在OnGetPINNotify裡面執行?
Ans: 在OnGetPINNotify Return回Kernel後,由Kernel執行
因為若是PIN輸入錯誤Retry次數會-1，因此在OnPINVerifyResult判斷PIN錯誤後是否會再回到OnGetPINNotify且傳入的bRemainingCounter會-1?
Ans: 當VerifyPIN錯誤後,Kernel並不會再呼叫OnGetPINNotify,仍舊必須繼續其後的流程,直至結束
若User要再輸入第二次,必須重新發起另一個交易
*/
void OnGetPINNotify(IN BYTE bPINType, IN USHORT bRemainingCounter, OUT BOOL* pIsUseDefaultGetPINFunc, OUT DEFAULT_GETPIN_FUNC_PARA *pPara)
{
	char		szTimeOut[2 + 1];
	char		szAmountMsg[_DISP_MSG_SIZE_ + 1];
	char		szPinpadMode[2 + 1];
	char		szDebugMsg[100 + 1];
	char		szEMVPINBypassEnable[2 + 1];
	unsigned long	ulTimeout = 0;

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "OnGetPINNotify_START!!");
        }
	
	/* 清空，並紀錄 */
	ginEMVPINType = -1;
	ginEMVPINType = bPINType;
        
	memset(szPinpadMode, 0x00, sizeof(szPinpadMode));
	inGetPinpadMode(szPinpadMode);
	memset(szEMVPINBypassEnable, 0x00, sizeof(szEMVPINBypassEnable));
	inGetEMVPINBypassEnable(szEMVPINBypassEnable);
	
	/* 不使用密碼機 */
	/* 銀聯卡不受EMVPinByPass影響 */
	if (memcmp(szPinpadMode, _PINPAD_MODE_0_NO_, strlen(_PINPAD_MODE_0_NO_)) == 0)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "不使用密碼機");
			inLogPrintf(AT, szDebugMsg);
		}
		
		if (memcmp(szPinpadMode, _PINPAD_MODE_0_NO_, strlen(_PINPAD_MODE_0_NO_)) == 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Pinpad Mode = 0");
				inLogPrintf(AT, szDebugMsg);
			}
		}
		else if (memcmp(szEMVPINBypassEnable, "Y", strlen("Y")) == 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "EMVPINBypassEnable :Y");
				inLogPrintf(AT, szDebugMsg);
			}
		}
		
		/* 不用密碼機 */
		*pIsUseDefaultGetPINFunc = FALSE;
		
	}
	/* 內建密碼機 */
	else if (memcmp(szPinpadMode, _PINPAD_MODE_1_INTERNAL_, strlen(_PINPAD_MODE_1_INTERNAL_)) == 0)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "內建密碼機");
			inLogPrintf(AT, szDebugMsg);
		}
		
		/* 用內建密碼機 */
		*pIsUseDefaultGetPINFunc = TRUE;
		
		memset(pPara, 0x00, sizeof(DEFAULT_GETPIN_FUNC_PARA));

		if (bPINType == d_NOTIFY_OFFLINE_PIN)
		{
			/* offline使用 */
			memset(szTimeOut, 0x00, sizeof(szTimeOut));
			inGetEMVPINEntryTimeout(szTimeOut);
			ulTimeout = atoi(szTimeOut);
			
			pPara->Version = 0x01;
			pPara->usLineLeft_X = 2;
			pPara->usLineRight_X = 20;
			pPara->usLinePosition_Y = 8;
			pPara->bPINDigitMaxLength = 8;
			pPara->bPINDigitMinLength = 4;
			pPara->ulTimeout = ulTimeout;

			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_ChineseFont("請輸入Offline PIN", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);
		}
		else
		{
			/* Mirror Message */
                        if (pobEmvTran.uszECRBit == VS_TRUE)
                        {
                                inECR_SendMirror(&pobEmvTran, _MIRROR_MSG_ENTER_PW_);
                        }

                        /* online使用 */
			memset(szTimeOut, 0x00, sizeof(szTimeOut));
			inGetCUPOnlinePINEntryTimeout(szTimeOut);
			ulTimeout = atoi(szTimeOut);
			
			pPara->Version = 0x01;
			pPara->usLineLeft_X = 2;
			pPara->usLineRight_X = 20;
			pPara->usLinePosition_Y = 8;
			pPara->bPINDigitMaxLength = 8;
			pPara->bPINDigitMinLength = 4;
			pPara->ulTimeout = ulTimeout;

			pPara->ONLINEPIN_PARA.CipherKeySet = _TWK_KEYSET_NCCC_;
			pPara->ONLINEPIN_PARA.CipherKeyIndex = _TWK_KEYINDEX_NCCC_PIN_ONLINE_;
			pPara->ONLINEPIN_PARA.bPANLen = strlen(pobEmvTran.srBRec.szPAN);
			memcpy(pPara->ONLINEPIN_PARA.baPAN, pobEmvTran.srBRec.szPAN, strlen(pobEmvTran.srBRec.szPAN));

			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_CUP_GET_PASSWORD_IN_, 0, _COORDINATE_Y_LINE_8_4_);

			/* 顯示金額 */
			memset(szAmountMsg, 0x00, sizeof(szAmountMsg));
			if ((pobEmvTran.srBRec.uszVOIDBit == VS_TRUE	&&
			    (pobEmvTran.srBRec.inOrgCode != _REFUND_ && pobEmvTran.srBRec.inOrgCode != _INST_REFUND_ && pobEmvTran.srBRec.inOrgCode != _REDEEM_REFUND_ && pobEmvTran.srBRec.inOrgCode != _CUP_REFUND_ && pobEmvTran.srBRec.inOrgCode != _CUP_MAIL_ORDER_REFUND_))	|| 
			     pobEmvTran.srBRec.inCode == _REFUND_		|| 
			     pobEmvTran.srBRec.inCode == _INST_REFUND_	|| 
			     pobEmvTran.srBRec.inCode == _REDEEM_REFUND_	||
			     pobEmvTran.srBRec.inCode == _CUP_REFUND_	|| 
			     pobEmvTran.srBRec.inCode == _CUP_MAIL_ORDER_REFUND_)
			{
				sprintf(szAmountMsg, "%ld", 0 - pobEmvTran.srBRec.lnTotalTxnAmount);
			}
			else
			{
				sprintf(szAmountMsg, "%ld", pobEmvTran.srBRec.lnTotalTxnAmount);
			}
			inFunc_Amount_Comma(szAmountMsg, "NT$", ' ', _SIGNED_NONE_,  15, _PADDING_RIGHT_);
			inDISP_EnglishFont_Point_Color(szAmountMsg, _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_RED_, _COLOR_WHITE_, 7);

			/* 嗶三聲 */
			inDISP_BEEP(3, 500);

		}
		
	}
	/* 外接密碼機 */
	else if (memcmp(szPinpadMode, _PINPAD_MODE_2_EXTERNAL_, strlen(_PINPAD_MODE_2_EXTERNAL_)) == 0)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "外接密碼機");
			inLogPrintf(AT, szDebugMsg);
		}
		
		/* 用外接密碼機 */
		*pIsUseDefaultGetPINFunc = FALSE;
		
		/* 繼續到OnOnlinePINBlockGet 或 OnOfflinePINBlockGet 才輸入 */
		inDISP_PutGraphic(_CUP_GET_PASSWORD_OUT_, 0, _COORDINATE_Y_LINE_8_4_);
	}
	
	
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "OnGetPINNotify_END!!");
        }
}

/* Return d_OK to indicate Online PIN block is ready for application */
/* 執行EMV_TxnPerform時若是需要用外接PINPAD輸入Online PIN BLOCK會CALL這支function，目前都會用內建，所以應該不會用這支 */
USHORT OnOnlinePINBlockGet(OUT ONLINE_PIN_DATA *pOnlinePINData)
{
	char	szPinpadMode[2 + 1];
	char	szEMVPINBypassEnable[2 + 1];
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "OnOnlinePINBlockGet_START!!");
        }
	
	memset(szPinpadMode, 0x00, sizeof(szPinpadMode));
	inGetPinpadMode(szPinpadMode);
	memset(szEMVPINBypassEnable, 0x00, sizeof(szEMVPINBypassEnable));
	inGetEMVPINBypassEnable(szEMVPINBypassEnable);
	
	/* 不使用密碼機 */
	/* 銀聯卡不受EMVPinByPass影響 */
	if (memcmp(szPinpadMode, _PINPAD_MODE_0_NO_, strlen(_PINPAD_MODE_0_NO_)) == 0)
	{
		pOnlinePINData->isOnlinePINRequired = VS_FALSE;
		
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "OnOnlinePINBlockGet_END!!");
		}
		
		return d_EMVAPLIB_OK;
	}
	/* 這裡放外接密碼機回傳的判斷或使用內建KMS加密後的判斷 */
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "OnOnlinePINBlockGet_END!!");
		}
		
		return d_EMVAPLIB_OK;
	}
        
//	pOnlinePINData->bPINLen = 8;
//	memcpy(pin_block, "\xB5\x1E\xC3\xBE\xB7\x24\x37\x13", 8);
//	pOnlinePINData->pPIN = pin_block;
}

/* Return d_OK to indicate Offline PIN block is ready for Kernel */
/* If this function uses KMS_GetEncOfflinePIN function to get offline pin, return d_EMV_ENTER_KMS_OFFLINEPIN to indicate enciphed offline PIN is ready for Kernel */
/* 執行EMV_TxnPerform時若是需要用外接PINPAD輸入Offline PIN BLOCK會CALL這支function，目前都會用內建，所以應該不會用這支 */
USHORT OnOfflinePINBlockGet(void)
{
	char	szPinpadMode[2 + 1];
	char	szEMVPINBypassEnable[2 + 1];
	
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "OnOfflinePINBlockGet_START!!");
        }

	memset(szPinpadMode, 0x00, sizeof(szPinpadMode));
	inGetPinpadMode(szPinpadMode);
	memset(szEMVPINBypassEnable, 0x00, sizeof(szEMVPINBypassEnable));
	inGetEMVPINBypassEnable(szEMVPINBypassEnable);
	
	/* 不使用密碼機 */
	/* 銀聯卡不受EMVPinByPass影響 */
	if (memcmp(szPinpadMode, _PINPAD_MODE_0_NO_, strlen(_PINPAD_MODE_0_NO_)) == 0)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "OnOfflinePINBlockGet_END!!");
		}
		
		return d_EMVAPLIB_OK;
	}
	/* 這裡放外接密碼機回傳的判斷或使用內建KMS加密後的判斷 */
	else
	{
		
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "OnOfflinePINBlockGet_END!!");
		}
		
		return d_EMVAPLIB_OK;
	}
	
	
//	USHORT ret;
//	BYTE key;
//	CTOS_stDefEncOffPINStruc para;
//	CTOS_KMS2DATAENCRYPT_PARA stDataEncPara;
//	BYTE PINBlock[32], PINBlockLen;
//	BYTE msg1[20], msg2[20], procmsg[20];
//
//	stDataEncPara.Version = 0x00;
//
//	stDataEncPara.Protection.CipherKeyIndex = d_OFFLINE_KEYINDEX;
//	stDataEncPara.Protection.CipherKeySet = d_ONLINE_KEYSET;
//	stDataEncPara.Protection.CipherMethod = KMS2_DATAENCRYPTCIPHERMETHOD_EXTPIN_ECB;
//	memcpy(PINBlock, "\xB5\x1E\xC3\xBE\xB7\x24\x37\x13", 8);
//	PINBlockLen = 8;
//	stDataEncPara.Input.pData = PINBlock;
//	stDataEncPara.Input.Length = PINBlockLen;
//
//	ret = CTOS_KMS2DataEncrypt(&stDataEncPara);
//
//	if (ret != d_OK)
//	{
//		printf("OnOfflinePINBlockGet_ERROR!! ");
//		return d_EMVAPLIB_ERR_CRITICAL_ERROR;
//	}
}

/* 執行EMV_TxnPerform時若是輸入Offline PIN後會CALL這支function檢查PIN是否合法 */
void OnOfflinePINVerifyResult(IN USHORT usResult)
{
//	BYTE data[32];

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "OnPINVerifyResult_START!!");
        }

        /* Sample */
//	memset(data, 0, sizeof(data));
//
//    	if (usResult == d_PIN_RESULT_OK)
//    	{
//		//"PIN Verify OK"
//		printf("PIN Verify OK ");
//        	sprintf((char *)data, "PIN Verify OK");
//
//	}
//	else if (usResult == d_PIN_RESULT_FAIL)
//	{
//		//"!PIN Wrong!"
//		printf("PIN Wrong ");
//        	sprintf((char *)data, "PIN Wrong");
//	}
//	else if (usResult == d_PIN_RESULT_BLOCKED || usResult == d_PIN_RESULT_FAILBLOCKED)
//	{
//		//"!PIN Blocked!"
//		printf("PIN Blocked ");
//        	sprintf((char *)data, "PIN Blocked");
//	}
//
//	CTOS_LCDTPrintXY(1, 6, data);

    	if (usResult == d_PIN_RESULT_OK)
    	{
		//"PIN Verify OK"
                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont("PIN Verify OK", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);
	}
	else if (usResult == d_PIN_RESULT_FAIL)
	{
		//"!PIN Wrong!"
                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont("PIN Wrong", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);
	}
	else if (usResult == d_PIN_RESULT_BLOCKED || usResult == d_PIN_RESULT_FAILBLOCKED)
	{
		//"!PIN Blocked!"
                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont("PIN Blocked", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);
	}
        
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "OnPINVerifyResult_END!!");
        }
}

/* 執行EMV_TxnPerform時若是First GenAC要求Online則進入這支function做電文送收，這支function一定要跑 */
void OnTxnOnline(IN ONLINE_PIN_DATA *pOnlinePINData, OUT EMV_ONLINE_RESPONSE_DATA* pOnlineResponseData)
{
	int	inIssuerScriptLen = 0;
	int	inRetVal = 0;
	char	szDemoMode[2 + 1] = {0};
	char	szDebugMsg[100 + 1] = {0};
	
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "OnTxnOnline_START!!");
        }
	
	pobEmvTran.inEMVDecision = _EMV_DECESION_ONLINE;
	
	/* 從API傳入OnlinePin資料 */
	if (pOnlinePINData->bPINLen > 0)
	{
		memset(pobEmvTran.szPIN, 0x00, 8 + 1);
		memcpy(pobEmvTran.szPIN, pOnlinePINData->pPIN, pOnlinePINData->bPINLen);
		pobEmvTran.srBRec.uszPinEnterBit = VS_TRUE;
	}
	else
	{
		pobEmvTran.srBRec.uszPinEnterBit = VS_FALSE;
	}
	
	/* 初始化 EMV Config */
	memset(&EMVGlobConfig, 0x00, sizeof(EMV_CONFIG));
        
        ginGlobalRetVal = VS_SUCCESS;

        if (ginGlobalRetVal == VS_SUCCESS)
	{
                if (inEMV_GetEMVTag(&pobEmvTran) != VS_SUCCESS)
		{
                        ginGlobalRetVal = VS_ERROR;
		}
	}
	
        if (ginGlobalRetVal == VS_SUCCESS)
	{
		if (0)
		{
			
		}
		else if (0)
		{
			
		}
		else
		{
			inRetVal = inFLOW_RunFunction(&pobEmvTran, _NCCC_FUNCTION_SET_TXN_ONLINE_OFFLINE_FLOW_);
		}
		
                if (inRetVal != VS_SUCCESS)
		{
                        ginGlobalRetVal = VS_ERROR;
		}
	}

        if (ginGlobalRetVal == VS_SUCCESS)
	{
		if (0)
		{
			
		}
		else if (0)
		{
			
		}
		else
		{
			inRetVal = inFLOW_RunFunction(&pobEmvTran, _NCCC_FUNCTION_BUILD_AND_SEND_PACKET_FLOW_);
		}
		
		if (inRetVal == VS_SUCCESS)
		{
			
		}
		else if (inRetVal == VS_COMM_ERROR)
		{
			EMVGlobConfig.uszAction = d_ONLINE_ACTION_UNABLE ;
			ginGlobalRetVal = VS_ERROR;
		}
                else if (inRetVal != VS_SUCCESS)
		{
			ginGlobalRetVal = VS_ERROR;
		}
	}
		
	/* [20251219_BUG_MDF][FUNC] 總帳及明細列印時不可出現電子錢包及信託的主機
	 * 只要有進連線流程，不論成功或失敗，INV都加一 */
	if(EMVGlobConfig.uszAction == d_ONLINE_ACTION_ISSUER_REFERRAL_DENY)
	{
	
		inFLOW_RunFunction(&pobEmvTran, _FUNCTION_UPDATE_INV_);
	}
	
	/* 教育訓練模式 */
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		pOnlineResponseData->bAction = d_ONLINE_ACTION_APPROVAL;
		memcpy(EMVGlobConfig.uszAuthorizationCode, "00", 2);
		pOnlineResponseData->pAuthorizationCode = EMVGlobConfig.uszAuthorizationCode;
		pOnlineResponseData->pIssuerAuthenticationData = EMVGlobConfig.uszIssuerAuthenticationData;
		pOnlineResponseData->pIssuerScript = EMVGlobConfig.uszIssuerScript;
	}
	else
	{
		/* Second Gen AC會在Online後執行，執行完Second Gen AC的流程在OnTxnResult中執行 */

		/* Host Action 用global 變數 guszAction*/

		/* 主機Auth Code */
		memset(EMVGlobConfig.uszAuthorizationCode, 0x00, sizeof(EMVGlobConfig.uszAuthorizationCode));
		memcpy(EMVGlobConfig.uszAuthorizationCode, &pobEmvTran.srBRec.szRespCode, 2);

		/* 填入IssuerAuthData */
		if (pobEmvTran.srEMVRec.in91_IssuerAuthDataLen > 0)
		{
			memset(EMVGlobConfig.uszIssuerAuthenticationData, 0x00, sizeof(EMVGlobConfig.uszIssuerAuthenticationData));
			memcpy(EMVGlobConfig.uszIssuerAuthenticationData, &pobEmvTran.srEMVRec.usz91_IssuerAuthData, pobEmvTran.srEMVRec.in91_IssuerAuthDataLen);
			EMVGlobConfig.usIssuerAuthenticationDataLen = pobEmvTran.srEMVRec.in91_IssuerAuthDataLen;

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "IssuerAuthenticationDataLen : %d", EMVGlobConfig.usIssuerAuthenticationDataLen);
				inLogPrintf(AT, szDebugMsg);
			}
		}

		inIssuerScriptLen = 0;
		memset(EMVGlobConfig.uszIssuerScript, 0x00, sizeof(EMVGlobConfig.uszIssuerScript));
		EMVGlobConfig.usIssuerScriptLen = 0;
		
		/* 填入IssuerScript1 */
		if (pobEmvTran.srEMVRec.in71_IssuerScript1Len > 0)
		{
			memcpy(&EMVGlobConfig.uszIssuerScript[inIssuerScriptLen], "\x71", 1);
			EMVGlobConfig.usIssuerScriptLen += 1;
			inIssuerScriptLen += 1;
			memcpy(&EMVGlobConfig.uszIssuerScript[inIssuerScriptLen], &pobEmvTran.srEMVRec.in71_IssuerScript1Len, 1);
			EMVGlobConfig.usIssuerScriptLen += 1;
			inIssuerScriptLen += 1;
			memcpy(&EMVGlobConfig.uszIssuerScript[inIssuerScriptLen], &pobEmvTran.srEMVRec.usz71_IssuerScript1, pobEmvTran.srEMVRec.in71_IssuerScript1Len);
			EMVGlobConfig.usIssuerScriptLen += pobEmvTran.srEMVRec.in71_IssuerScript1Len;
			inIssuerScriptLen += pobEmvTran.srEMVRec.in71_IssuerScript1Len;
		}

		/* 填入IssuerScript2 */
		if (pobEmvTran.srEMVRec.in72_IssuerScript2Len > 0)
		{
			memcpy(&EMVGlobConfig.uszIssuerScript[inIssuerScriptLen], "\x72", 1);
			EMVGlobConfig.usIssuerScriptLen += 1;
			inIssuerScriptLen += 1;
			memcpy(&EMVGlobConfig.uszIssuerScript[inIssuerScriptLen], &pobEmvTran.srEMVRec.in72_IssuerScript2Len, 1);
			EMVGlobConfig.usIssuerScriptLen += 1;
			inIssuerScriptLen += 1;
			memcpy(&EMVGlobConfig.uszIssuerScript[inIssuerScriptLen], &pobEmvTran.srEMVRec.usz72_IssuerScript2, pobEmvTran.srEMVRec.in72_IssuerScript2Len);
			EMVGlobConfig.usIssuerScriptLen += pobEmvTran.srEMVRec.in72_IssuerScript2Len;
			inIssuerScriptLen += pobEmvTran.srEMVRec.in72_IssuerScript2Len;
		}

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "IssuerScriptLen : %d", EMVGlobConfig.usIssuerScriptLen);
			inLogPrintf(AT, szDebugMsg);
		}

		if (ginDebug == VS_TRUE)
		{
			if (EMVGlobConfig.uszAction == d_ONLINE_ACTION_APPROVAL)
			{
				inLogPrintf(AT, "EMVAction: APPROVAL");
			}
			else if (EMVGlobConfig.uszAction == d_ONLINE_ACTION_DECLINE)
			{
				inLogPrintf(AT, "EMVAction: DECLINE");
			}
			else if (EMVGlobConfig.uszAction == d_ONLINE_ACTION_UNABLE)
			{
				inLogPrintf(AT, "EMVAction: UNABLE");
			}
			else
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "EMVAction: %d", EMVGlobConfig.uszAction);
				inLogPrintf(AT, szDebugMsg);
			}
		}

		/* 注意，傳入的pointer一定要是global的，否則會吃不到資料 */
		pOnlineResponseData->bAction = EMVGlobConfig.uszAction;
		pOnlineResponseData->pAuthorizationCode = EMVGlobConfig.uszAuthorizationCode;
		pOnlineResponseData->pIssuerAuthenticationData = EMVGlobConfig.uszIssuerAuthenticationData;
		pOnlineResponseData->IssuerAuthenticationDataLen = EMVGlobConfig.usIssuerAuthenticationDataLen;
		pOnlineResponseData->pIssuerScript = EMVGlobConfig.uszIssuerScript;
		pOnlineResponseData->IssuerScriptLen = EMVGlobConfig.usIssuerScriptLen;
        }

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "OnTxnOnline_END!!");
        }
}

/* 執行EMV_TxnPerform時若是要處理Issuer Script則要跑這支function */
void OnTxnIssuerScriptResult(IN BYTE* pScriptResult, IN USHORT pScriptResultLen)
{
	char	szAscii[100 + 1];
	char	szDebugMsg[100 + 1];
	
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "OnTxnIssuerScriptResult_START!!");
        }
	
	pobEmvTran.srEMVRec.in9F5B_ISRLen = pScriptResultLen;
	memcpy(pobEmvTran.srEMVRec.usz9F5B_ISR, pScriptResult, pobEmvTran.srEMVRec.in9F5B_ISRLen);
	
        if (ginDebug == VS_TRUE)
	{
		memset(szAscii, 0x00, sizeof (szAscii));
		inFunc_BCD_to_ASCII(szAscii, (unsigned char*)pScriptResult, pScriptResultLen);
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "%s", szAscii);
		inLogPrintf(AT, szDebugMsg);
	}

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "OnTxnIssuerScriptResult_END!!");
        }
}

/* 執行EMV_TxnPerform的最後要判斷晶片處理結果且顯示通知使用者要在這處理 */
void OnTxnResult(IN BYTE bResult, IN BOOL IsSignatureRequired)
{
	char		szDemoMode[2 + 1] = {0};
	unsigned char 	uszValue[128 + 1] = {0};
	unsigned short	usTagLen = 0;
	
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "OnTxnResult_START!!");
        }
	
	/* bResult 為交易結果，IsSignatureRequired是CVM結果，目前只存，但我們自己抓TAG判斷 */
	if (bResult == d_TXN_RESULT_APPROVAL)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "Txn Approval");
		}
		
		pobEmvTran.inEMVResult = _EMV_RESULT_APPROVE_;
		
		if (pobEmvTran.inEMVDecision == _EMV_DECESION_ONLINE)
		{
			
		}
		else
		{
			pobEmvTran.inEMVDecision = _EMV_DECESION_OFFLINE;
		}
	}
	else if (bResult == d_TXN_RESULT_DECLINE)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "Txn Decline");
		}
		
		pobEmvTran.inEMVResult = _EMV_RESULT_DECLINE_;
		
		if (pobEmvTran.inEMVDecision == _EMV_DECESION_ONLINE)
		{
			
		}
		else
		{
			pobEmvTran.inEMVDecision = _EMV_DECESION_OFFLINE;
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "Txn Not Recognize");
		}
		
		pobEmvTran.inEMVResult = _EMV_RESULT_UNKOWN_;
		
		if (pobEmvTran.inEMVDecision == _EMV_DECESION_ONLINE)
		{
			
		}
		else
		{
			pobEmvTran.inEMVDecision = _EMV_DECESION_OFFLINE;
		}
	}
	
	/* 判斷是否要簽名 */
	/* NCCC不判斷晶片卡結果(有事Vx520負責) 2018/6/22 下午 6:11 */
	if (ginAPVersionType == _APVERSION_TYPE_NCCC_)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "NCCC Don't Check CVM Result");
		}
		
		if (IsSignatureRequired == 1)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "EMV Kernel Decide Need Signature");
			}

			pobEmvTran.inCVMResult = _CVM_RESULT_NEED_SIGNATURE_;
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "EMV Kernel Decide Not Need Signature");
			}

			pobEmvTran.inCVMResult = _CVM_RESULT_NO_NEED_SIGNATURE_;
		}
		
		
	}
	else
	{
		if (IsSignatureRequired == 1)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "EMV Kernel Decide Need Signature");
			}

			pobEmvTran.inCVMResult = _CVM_RESULT_NEED_SIGNATURE_;
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "EMV Kernel Decide Not Need Signature");
			}
			pobEmvTran.inCVMResult = _CVM_RESULT_NO_NEED_SIGNATURE_;

			/* 分期付款交易，使用感應卡進行交易，皆要簽名，不允許分期付款交易"免簽名"。 */
			if (pobEmvTran.srBRec.uszInstallmentBit == VS_TRUE)
			{
				pobEmvTran.srBRec.uszNoSignatureBit = VS_FALSE; /* 免簽名條件 */
			}
			else
			{
				pobEmvTran.srBRec.uszNoSignatureBit = VS_TRUE;
			}
		}
	}
	
	/* 驗證時交易失敗(不online)也要看Tag */
	/* 取得8A的值(callbank) */
	inEMV_GetEMVTag(&pobEmvTran);
        
	/* 教育訓練模式 */
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		/* 不用送TC，但若失敗仍要回傳錯誤(For demo，正式版會在inXXX_OnlineEMV_Complete中檢核) */
		/* 8A */
		usTagLen = sizeof(uszValue);
		memset(uszValue, 0x00, sizeof(uszValue));
		inEMV_Get_Tag_Value(0x8A, &usTagLen, uszValue);

		/*  先檢核first gen AC的結果 若是Y1 或 Z1就不用在做Second Gen AC */
		if (!memcmp(uszValue, "Z1", 2))
		{
			/* Z1= declined offline (first Gen AC , CID=AAC) */
			/* 拒絕交易 */

			memset(pobEmvTran.srBRec.szRespCode, 0x00, sizeof(pobEmvTran.srBRec.szRespCode));
			strcpy(pobEmvTran.srBRec.szRespCode, "Z1");

			inEMV_Decide_DispHostRespCodeMsg();
			ginGlobalRetVal = VS_ERROR;
		}
		else if (!memcmp(uszValue, "Y1", 2))
		{
			/* EMV First Gen AC產生Y1 */
			if (ginDebug == VS_TRUE)
				inLogPrintf(AT, "EMV First Gen AC產生Y1");

			memset(pobEmvTran.srBRec.szRespCode, 0x00, sizeof(pobEmvTran.srBRec.szRespCode));
			memset(pobEmvTran.srBRec.szAuthCode, 0x00, sizeof(pobEmvTran.srBRec.szAuthCode));
			strcpy(pobEmvTran.srBRec.szRespCode, "Y1");
			strcpy(pobEmvTran.srBRec.szAuthCode, "Y1");
		}
		else
		{
			/* DEMO版本插晶片做2nd Gen AC有可能變Z3 直接跳過 */
			/* 開始執行 Second Generate AC */
//			inRetVal = inEMV_SecondGenerateAC(&pobEmvTran);
//			if (inRetVal == VS_SUCCESS)
//			{
//
//			}
//			else
//			{
//				/* 拒絕交易Z3 */
//				inEMV_Decide_DispHostRespCodeMsg();
//				ginGlobalRetVal = VS_ERROR;
//			}

		}
	}
	else
	{
		if (inEMV_Decide_OnlineEMV_Complete() != VS_SUCCESS)
		{
			ginGlobalRetVal = VS_ERROR;
		}
	}
	
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "OnTxnResult_END!!");
        }
}

/* 執行EMV_TxnPerform時要在這輸入金額將金額回傳給Kernel */
void OnTotalAmountGet(IN BYTE *pPAN, IN BYTE bPANLen, OUT ULONG *pAmount)
{
	char	szTemplate[50 + 1] = {0};
	
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "OnTotalAmountGet_START!!");
        }
	
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(szTemplate, pPAN, bPANLen);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "PAN:%s", szTemplate);
		inLogPrintf(AT, "PANLen:%u", bPANLen);
		inLogPrintf(AT, "Amt:%lu", pobEmvTran.srBRec.lnTxnAmount);
	}
		
	*pAmount = pobEmvTran.srBRec.lnTxnAmount;
        
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "OnTotalAmountGet_END!!");
        }
}

/* 執行EMV_TxnPerform時在進行FirstGenAC前會進入這檢查檔案 */
void OnExceptionFileCheck(IN BYTE *pPAN, IN BYTE bPANLen, OUT BOOL *isException)
{
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "OnExceptionFileCheck_START!!");
        }

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, (char *)isException);
        }
        

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "OnExceptionFileCheck_END!!");
        }
}

/* 執行EMV_TxnPerform時取得CAPK後會檢查是否CAPK合法 */
BOOL OnCAPKRevocationCheck(IN BYTE *pbRID, IN BYTE bCAPKIdx, BYTE *pbSerialNumuber)
{
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "OnCAPKRevocationCheck_START!!");
        }

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "OnCAPKRevocationCheck_END!!");
        }

	return FALSE;
}

/*
Function        :OnEventTxnForcedOnline
Date&Time       :2016/12/13 下午 5:14
Describe        :For DCC不能在Txn Data Get中設定Force Online用，在OnTxnOnline前，OnExceptionFileCheck後執行
*/
void OnEventTxnForcedOnline(BYTE *pbForcedONL)
{
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "ON_EVENT_TxnForcedOnline() START !");
	}
	
	//Inform kernel forced online or not
	/* forced online */
	if (pobEmvTran.srBRec.uszDCCTransBit == VS_TRUE)
	{
		*pbForcedONL = 0x01;
	}
	
	/* NO forced online */
	//*pbForcedONL = 0x00;
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "ON_EVENT_TxnForcedOnline() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
}

/*
Function        :OnEventAppListEx
Date&Time       :2016/12/15 下午 3:57
Describe        :
 * 端末機Select到多個UICC的AID時，端末機自動Select AID而不需要出現Select AP的List讓收銀員選擇。(本題僅針對UICC，V/M/J維持原EMV 流程)
 * 端末機自動Select AID的條件如下：
 * (1)	以優先權較高的AID，優先執行。
 * (2)	若所有AID都沒有標示優先權，則直接以TMS下載之AID順序進行Select。端末機自動以Select出來的第一個AID進行交易。

*/
USHORT OnEventAppListEx(BYTE bAppNum, EMV_APP_LIST_EX_DATA *pstAppListExData, BYTE *pbAppSelectedIndex)
{
	int		i = 0;
	int		inAppIndex = -1;
	int		inTimeout = _EMV_MANUAL_CARD_LIST_TIMEOUT_;
	int		inTouchSensorFunc = _Touch_8X16_OPT_;
	int		inChoice = _Touch_NONE_;
	char	szDebugMsg[100 + 1] = {0};
	char	szTemplate[50 + 1] = {0};
	char	szExamBit[10 + 1] = {0};
	char	szVISA_AID[16 + 1] = {0};
	char	szCUP_AID[16 + 1] = {0};
	char	szUCARD_AID[16 + 1] = {0};
	char	szAID_Ascii[16 + 1] = {0};
	char	szEnableESG[2] = {0};
    char szCustomerIndicator[3 + 1] = {0};
	unsigned char	uszVisaBit = VS_FALSE;
	unsigned char	uszCUPBit = VS_FALSE;
	unsigned char	uszUCardBit = VS_FALSE;
	unsigned char	uszVisaIndex = -1;
	unsigned char	uszCUPIndex = -1;
	unsigned char	uszUCARDIndex = -1;
	unsigned char	uszDispSelecDefaulttMenuBit = VS_FALSE;
	unsigned char	uszKey = 0x00;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "OnEventAppListEx() START !");
	}
	
	if (ginEMVAppSelection != -1)
	{
		*pbAppSelectedIndex = ginEMVAppSelection;
		
		return d_EMVAPLIB_OK;
	}
	
	
	
	/* ECR和單機操作Timeout不同 */
	if (pobEmvTran.uszECRBit == VS_TRUE)
	{
		inTimeout = _EMV_ECR_CARD_LIST_TIMEOUT_;
	}
	else
	{
		inTimeout = _EMV_MANUAL_CARD_LIST_TIMEOUT_;
	}
	
	/* 取得VISA AID */
	inLoadMVTRec(_MVT_VISA_INDEX_);
	memset(szVISA_AID, 0x00, sizeof(szVISA_AID));
	inGetMVTApplicationId(szVISA_AID);
	
	/* 取得CUP AID */
	inLoadMVTRec(_MVT_CUP_CREDIT_INDEX_);
	memset(szCUP_AID, 0x00, sizeof(szCUP_AID));
	inGetMVTApplicationId(szCUP_AID);
	
	/* 取得UCARD AID */
	inLoadMVTRec(_MVT_NCCC_INDEX_);
	memset(szUCARD_AID, 0x00, sizeof(szUCARD_AID));
	inGetMVTApplicationId(szUCARD_AID);
	
	for (i = 0; i < bAppNum; i++)
	{
		memset(szAID_Ascii, 0x00, sizeof(szAID_Ascii));
		inFunc_BCD_to_ASCII(szAID_Ascii, pstAppListExData[i].baAID, pstAppListExData[i].baAIDLen);
		
		if (ginDebug == VS_TRUE)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%d.%s", i + 1, pstAppListExData[i].cAppLabel);
			inLogPrintf(AT, szTemplate);
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%s", szAID_Ascii);
			inLogPrintf(AT, szTemplate);
		}
		
		if (ginISODebug == VS_TRUE)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%d.%s", i + 1, pstAppListExData[i].cAppLabel);
			inPRINT_ChineseFont(szTemplate, _PRT_ISO_);
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%s", szAID_Ascii);
			inPRINT_ChineseFont(szTemplate, _PRT_ISO_);
		}
		
		if (uszVisaBit != VS_TRUE)
		{
			if (!memcmp(szAID_Ascii, szVISA_AID, pstAppListExData[i].baAIDLen * 2))
			{
				uszVisaBit = VS_TRUE;
				uszVisaIndex = i;
				continue;
			}
		}
		
		if (uszCUPBit != VS_TRUE)
		{
			if (!memcmp(szAID_Ascii, szCUP_AID, pstAppListExData[i].baAIDLen * 2))
			{
				uszCUPBit = VS_TRUE;
				uszCUPIndex = i;
				continue;
			}
		}
		
		if (uszUCardBit != VS_TRUE)
		{
			if (!memcmp(szAID_Ascii, szUCARD_AID, pstAppListExData[i].baAIDLen * 2))
			{
				uszUCardBit = VS_TRUE;
				uszUCARDIndex = i;
				continue;
			}
		}
	}
	
	/* 驗測一律出選項 */
	memset(szExamBit, 0x00, sizeof(szExamBit));
	inGetExamBit(szExamBit);
	
	/* 驗測必定讓人選 */
	if (szExamBit[0] == '1')
	{
		uszDispSelecDefaulttMenuBit = VS_TRUE;
	}
	else if (pobEmvTran.srBRec.uszCUPTransBit == VS_TRUE)
	{
		if (uszCUPBit == VS_TRUE)
		{
			inAppIndex = uszCUPIndex;
		}
		else
		{
			inAppIndex = -1;
		}
	}
	/* 如果是自有品牌卡也要幫忙選 */
	else if (uszUCardBit == VS_TRUE)
	{
		inAppIndex = uszUCARDIndex;
	}
	/* 金融卡交易不出選單 */
	else if (pobEmvTran.srBRec.uszFiscTransBit == VS_TRUE)
	{
		/* 直接選第一項*/
		inAppIndex = 0;
	}
	/* 其他狀況，NCCC Production開auto selection，自動選第一組 */
	else
	{
		/* VISA、CUP雙AID卡 */
		if (uszVisaBit == VS_TRUE && uszCUPBit == VS_TRUE)
		{
			uszDispSelecDefaulttMenuBit = VS_TRUE;

			/* [114197] 中心收單作業ESG永續發展優化 
			 * [260602] 只有雙品牌卡且為ECR發動的交易要修改秒數 */
			if (pobEmvTran.uszECRBit == VS_TRUE)
			{
				/* [114197] 中心收單作業ESG永續發展優化 
				 * [260610] 雙品牌卡且為ECR發動的交易且符合ESG需求的客製化才設定為6秒 */
				memset(szCustomerIndicator, 0x00, sizeof (szCustomerIndicator));
				if(inGetCustomIndicator(szCustomerIndicator) == VS_SUCCESS)
				{
					if(!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_000_, _CUSTOMER_INDICATOR_SIZE_) ||
					!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_038_BELLAVITA_, _CUSTOMER_INDICATOR_SIZE_) ||
					!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_039_CARD_NO_HIDE_F8_B4_, _CUSTOMER_INDICATOR_SIZE_) ||
					!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_076_8_DIGITS_, _CUSTOMER_INDICATOR_SIZE_) ||
					!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_099_SINYA_, _CUSTOMER_INDICATOR_SIZE_) ||
					!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_113_H_AND_M_, _CUSTOMER_INDICATOR_SIZE_) ||
					!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_067_KOBAY, _CUSTOMER_INDICATOR_SIZE_))
					{
						/* 新增判斷是否有開啟ESG功能, 如果沒抓到資料就設定為N,不改變Timout秒數 */
						if(inGetCHESG_FLAG(szEnableESG) != VS_SUCCESS)
						{
							szEnableESG[0] = 'N';
						}

						/* 新增判斷是否有開啟ESG功能,如果有開啟時Timeout修改為 6 秒 */
						if(szEnableESG[0] == 'Y')
						{	
							inTimeout = _EMV_ECR_ESG_CARD_LIST_TIMEOUT_;
						}
					}	
				}
			}
		}
		else
		{
			/* 直接選第一項*/
			inAppIndex = 0;
		}
	}

	if (uszDispSelecDefaulttMenuBit == VS_TRUE)
	{
		inTouchSensorFunc = _Touch_8X16_OPT_;
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned("請持卡人選擇交易卡別", _FONTSIZE_8X22_, _COLOR_WHITE_, _COLOR_BLACK_, _COORDINATE_Y_LINE_8_3_, VS_FALSE, _DISP_CENTER_);
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		for (i = 0; i < bAppNum; i++)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%d.%s", i + 1, pstAppListExData[i].cAppLabel);
			inDISP_ChineseFont(szTemplate, _FONTSIZE_8X22_, _LINE_8_4_ + i, _DISP_LEFT_);
		}
		inDISP_TimeoutStart(inTimeout);
		/* [114197] 中心收單作業ESG永續發展優化 
		 * [260602] 新增立既顯示倒數秒數 */
		inDISP_LiveCountdown(inTimeout,_FONTSIZE_8X16_, _LINE_8_6_, _DISP_RIGHT_);
		
		while (1)
		{
			inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
			/* ------------偵測key in------------------ */
			uszKey = -1;
			uszKey = uszKBD_Key();
			
			/* 感應倒數時間 && Display Countdown */
			if (inDISP_TimeoutCheck(_FONTSIZE_8X16_, _LINE_8_6_, _DISP_RIGHT_) == VS_TIMEOUT)
			{
				/* 感應時間到Timeout */
				uszKey = _KEY_TIMEOUT_;
			}
		
			if (uszKey == _KEY_CANCEL_)
			{
				inAppIndex = -1;
				break;
			}
			else if (uszKey == _KEY_TIMEOUT_)
			{
				inAppIndex = -1;
				pobEmvTran.inErrorMsg = _ERROR_CODE_V3_SELECT_AID_TIMEOUT_;
				break;
			}
			else if (bAppNum >= 1 &&
				(uszKey == _KEY_1_ || inChoice == _OPTTouch8X16_LINE_4_))
			{
				inAppIndex = 0;
				inDISP_BEEP(1, 0);
				break;
			}
			else if (bAppNum >= 2 &&
				(uszKey == _KEY_2_ || inChoice == _OPTTouch8X16_LINE_5_))
			{
				inAppIndex = 1;
				inDISP_BEEP(1, 0);
				break;
			}
			else if (bAppNum >= 3 && 
				(uszKey == _KEY_3_ || inChoice == _OPTTouch8X16_LINE_6_))
			{
				inAppIndex = 2;
				inDISP_BEEP(1, 0);
				break;
			}
			else if (bAppNum >= 4 && 
				(uszKey == _KEY_4_ || inChoice == _OPTTouch8X16_LINE_7_))
			{
				inAppIndex = 3;
				break;
			}
			else if (bAppNum >= 5 && 
				(uszKey == _KEY_5_ || inChoice == _OPTTouch8X16_LINE_8_))
			{
				inAppIndex = 4;
				break;
			}
			else
			{
				continue;
			}
		}
		/* 清空Touch資料 */
		inDisTouch_Flush_TouchFile();
	}

	/* 將選擇的index塞回去 */
	if (inAppIndex != -1)
	{
		*pbAppSelectedIndex = (unsigned char)inAppIndex;
		
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Select %d.%s", inAppIndex + 1, pstAppListExData[inAppIndex].cAppLabel);
			inLogPrintf(AT, szDebugMsg);
			
			inLogPrintf(AT, "OnEventAppListEx() END !");
			inLogPrintf(AT, "----------------------------------------");
		}
		
		/* 記住選項，避免跳多次選單 */
		ginEMVAppSelection = *pbAppSelectedIndex;
		
		return d_EMVAPLIB_OK;
	}
	else
	{
		/* uszAppIndex = -1，沒設定index，return False */
		return d_EMVAPLIB_ERR_SELECTION_FAIL;
	}
	
}

/*
Function        :OnEventDisable_PinNull
Date&Time       :2019/11/13 上午 9:39
Describe        :設定為無法輸入null pin
*/
void OnEventDisable_PINNull(OUT BOOL *IsDisablePINNull)
{
	char	szEMVPINBypassEnable[2 + 1] = {0};
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "OnEventDisable_PINNull() START !");
	}
	
	/* 預設為可以輸入Null PIN */
	*IsDisablePINNull = FALSE;
	
	if (ginEMVPINType == d_NOTIFY_ONLINE_PIN)
	{
		*IsDisablePINNull = FALSE;
	}
	else
	{
		/* 如果PINBYPASS開關關閉代表 PIN NULL Disable */
		memset(szEMVPINBypassEnable, 0x00, sizeof(szEMVPINBypassEnable));
		inGetEMVPINBypassEnable(szEMVPINBypassEnable);
		if (memcmp(szEMVPINBypassEnable, "N", 1) == 0)
		{
			*IsDisablePINNull = TRUE;
		}
		else
		{
			*IsDisablePINNull = FALSE;
		}
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "OnEventDisable_PINNull(%d) END !", *IsDisablePINNull);
		inLogPrintf(AT, "----------------------------------------");
	}
}

/*
Function        :inEMV_Initial
Date&Time       :2016/12/13 下午 12:52
Describe        :
*/
int inEMV_Initial()
{
	char		szDebug[128] = {0};
	char		szTMSOK[1 + 1] = {0};
	char		szPath[100 + 1] = {0};
	unsigned short 	ushRetVal = 0x0000;
	/* EMV初始化 */
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "EMV_Initialize START!");
	       
	memset(szPath, 0x00, sizeof(szPath));
	sprintf(szPath, "%s%s", _EMV_EMVCL_DATA_PATH_, _EMV_CONFIG_FILENAME_);
	ushRetVal = EMV_Initialize(&g_emv_event, szPath);
        
        if (ginDebug == VS_TRUE)
        {
                memset(szDebug, 0x00, sizeof(szDebug));
                sprintf(szDebug, "EMV_Initialize:0X%04x", ushRetVal);
                inLogPrintf(AT, szDebug);
        }
        vdUtility_SYSFIN_LogMessage(AT, "EMV_Initialize Failed:0X%04x", ushRetVal);
        
        if (ushRetVal != d_EMVAPLIB_OK)
        {
                if (ginDebug == VS_TRUE)
		{
                        inLogPrintf(AT, "EMVxml初始化錯誤");
		}
		
		/* 若下完TMS，且EMV Initial失敗，提示錯誤訊息 */
		memset(szTMSOK, 0x00, sizeof(szTMSOK));
		inGetTMSOK(szTMSOK);
		if (!memcmp(szTMSOK, "Y", 1))
		{
			inDISP_ClearAll();
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_INIT_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
			strcpy(srDispMsgObj.szErrMsg1, "EMV XML");
			srDispMsgObj.inErrMsg1Line = _LINE_8_5_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
		}
                
                return (VS_ERROR);
        }
	
	/* 註冊Event，*/
	/* appList可以看到選項的AID */
	EMV_SpecialEventRegister(d_EVENTID_APP_LIST_EX, OnEventAppListEx);
	/* 可以讓Force Online流程往前 */
	EMV_SpecialEventRegister(d_EVENTID_TXN_FORCED_ONLINE, OnEventTxnForcedOnline);
	/* Null PIN無法繼續輸入 */
	EMV_SpecialEventRegister(d_EVENTID_DISABLE_PINNULL, OnEventDisable_PINNull);
	
	return (VS_SUCCESS);
}

/*
Function        :inEMV_Process
Date&Time       :2016/12/13 下午 12:53
Describe        :
*/
int inEMV_Process(TRANSACTION_OBJECT *pobTran)
{
	char		szDebug[200] = {0};
	char		szCustomIndicator[3 + 1] = {0};
	unsigned short 	ushRetVal = 0x00;
	unsigned char	uszHex[100 + 1] = {0};

        vdUtility_SYSFIN_LogMessage(AT, "inEMV_Process START!");
        
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inEMV_Process_START");
	
	memset(szCustomIndicator, 0x00, sizeof(szCustomIndicator));
	inGetCustomIndicator(szCustomIndicator);

        /* pobTran的資料存入pobEmvTran */
        memset((char *)&pobEmvTran, 0x00, _TRANSACTION_OBJECT_SIZE_);
	memcpy((char *)&pobEmvTran, (char *)pobTran, _TRANSACTION_OBJECT_SIZE_);

	/* 移除EMV_TxnAppSelect，改只在抓卡號時做 */

	/* 需要卡號才能跑得流程*/
	/* 因為晶片卡到這裡才讀卡號，所以從TRT拉到這邊 */
	/* 2018/10/9 下午 3:26 其實現在已將讀卡號放到前面，待適當時機將這些流程放回TRT中 */
	if (inNCCC_DCC_EMV_Set_Value(&pobEmvTran) != VS_SUCCESS)
        {
                vdUtility_SYSFIN_LogMessage(AT, "inEMV_Process inNCCC_DCC_EMV_Set_Value failed END!");
		return (VS_ERROR);
        }

	/* 客製化所有EMV晶片卡一律Force go online，不可有離線授權之交易 (Y1 及Y3 授權) (NCCC規格書)，這邊直接把Floor limit設為0 */
	if (!memcmp(szCustomIndicator, _CUSTOMER_INDICATOR_071_FORCE_ONLINE_, _CUSTOMER_INDICATOR_SIZE_))
	{
		memset(uszHex, 0x00, sizeof(uszHex));
		inFunc_ASCII_to_BCD(uszHex, "00", 2);
		inEMV_Set_TagValue_During_Txn(d_TAG_FLOOR_LIMIT, 4, uszHex);
	}
	
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "EMV_TxnPerform START!");
	
        /* 用來控制TC Upload前不斷線 */
	pobEmvTran.uszEMVProcessDisconectBit = VS_TRUE;
	
	/* 執行EMV驗證及交易流程，到結束Second Generate AC為止 */
    	ushRetVal = EMV_TxnPerform();
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "EMV_TxnPerform_END");
	
	/* 將callback使用的pobEmvTran結構傳回流程在用的pobTran結構 */
        memcpy((char *)pobTran, (char *)&pobEmvTran, _TRANSACTION_OBJECT_SIZE_);
	
	/* 斷線 */
	inCOMM_End(pobTran);

	/* 不論是Library function執行失敗或是中間callback跑的流程失敗都是不成功 */
    	if (ushRetVal != d_EMVAPLIB_OK || ginGlobalRetVal != VS_SUCCESS)
    	{
                if (ginDebug == VS_TRUE)
                {
                        memset(szDebug, 0x00, sizeof(szDebug));
                        sprintf(szDebug, "EMV_TxnPerform_ERR:0X%04x", ushRetVal);
                        inLogPrintf(AT, szDebug);
                        memset(szDebug, 0x00, sizeof(szDebug));
                        sprintf(szDebug, "ginGlobalRetVal_ERR:%d", ginGlobalRetVal);
                        inLogPrintf(AT, szDebug);
                }
		
		if (inEMV_ICCEvent() != VS_SUCCESS)
		{
			/* 晶片卡被取出 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_EMV_CARD_OUT_;
		}
                
                vdUtility_SYSFIN_LogMessage(AT, "EMV_TxnPerform_ERR:0X%04x", ushRetVal);
                vdUtility_SYSFIN_LogMessage(AT, "ginGlobalRetVal_ERR:%d", ginGlobalRetVal);
                vdUtility_SYSFIN_LogMessage(AT, "inEMV_Process EMV failed END!");
                
		return (VS_ERROR);
    	}
	
	/* (需求單-112101)-IKEA新增客製化參數需 by Russell 2023/6/20 下午 3:44 */
	/* 主機回覆授權加入提示音 */
	if (!memcmp(szCustomIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)	||
	    !memcmp(szCustomIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
	{
		inDISP_BEEP(1, 0);
	}
	
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inEMV_Process_END");
        
        vdUtility_SYSFIN_LogMessage(AT, "inEMV_Process END!");

	return(VS_SUCCESS);
}

/*
Function        :inEMV_SecondGenerateAC
Date&Time       :2016/10/3 下午 4:27
Describe        :
*/
int inEMV_SecondGenerateAC(TRANSACTION_OBJECT *pobTran)
{
	char		szTagVal[128 + 1];
        char		szTemplate[128 + 1];
	char		szCustomerIndicator[3 + 1] = {0};
	unsigned char	uszValue[128 + 1];
	unsigned short  usTagLen;

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inEMV_SecondGenerateAC Start!");
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
        
	usTagLen = sizeof(uszValue);
	memset(uszValue, 0x00, sizeof(uszValue));
	inEMV_Get_Tag_Value(0x9F27, &usTagLen, uszValue);
	memset(szTagVal, 0x00, sizeof(szTagVal));
        inFunc_BCD_to_ASCII(szTagVal, uszValue, usTagLen);

        if (ginDebug == VS_TRUE)
        {
                sprintf(szTemplate, "TAG_9F27 = %s", szTagVal);
                inLogPrintf(AT, szTemplate);
        }

	/* 根據verifone程式邏輯，只判斷9F27第一碼 */
	if (!memcmp(szTagVal, "4", 1))
	{
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inEMVAPISecondGenerateAC() TC");
		
		/* 客製化071不能判定為Y3 */
                if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_071_FORCE_ONLINE_, _CUSTOMER_INDICATOR_SIZE_))
		{
			if (pobTran->inTransactionResult == _TRAN_RESULT_COMM_ERROR_ && pobTran->inTransactionCode == _SALE_)
			{
				if (ginDebug == VS_TRUE)
					inLogPrintf(AT, "CUS 071 NOT ALLOWED Y3");
				
				vdUtility_SYSFIN_LogMessage(AT, "inEMV_SecondGenerateAC Response code:OL");
				memcpy(pobTran->srBRec.szRespCode, "OL", strlen("OL"));
				
				return (VS_ERROR);
			}
		}
		else
		{
			if (pobTran->inTransactionResult == _TRAN_RESULT_COMM_ERROR_ && pobTran->inTransactionCode == _SALE_)
			{
				/* 這裡應該是 Y3 */
				if (ginDebug == VS_TRUE)
					inLogPrintf(AT, "inEMV_SecondGenerateAC() Y3");

				if (ginDebug == VS_TRUE)
					inLogPrintf(AT, "NCCC NOT ALLOWED Y3");
				
				vdUtility_SYSFIN_LogMessage(AT, "inEMV_SecondGenerateAC Response code:OL");
				memcpy(pobTran->srBRec.szRespCode, "OL", strlen("OL"));
			
				return (VS_ERROR);
			}
		}
		
	}
	else if (!memcmp(szTagVal, "0", 1))
	{
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inEMV_SecondGenerateAC() AAC!");
                        
                memcpy(pobTran->srBRec.szRespCode, "Z3", 2);

		return (VS_ERROR);
	}
	/* Second Generate AC不會有ARQC，因此判斷為ERROR */
	else 
	{
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inEMV_SecondGenerateAC() ERROR!");

		/* 被卡片拒絕 */
		
		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

int inNCCC_FuncEMVPrepareBatch(TRANSACTION_OBJECT *pobTran)
{
	USHORT  usTagLen;
	BYTE    value[128 + 1];
	char	szAscii[200 + 1];
        char    szTemplate[1024 + 1];
	char	szDebugMsg[100 + 1];

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_FuncEMVPrepareBatch() START!");

	if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
	{
		/* 5A */
		usTagLen = sizeof(value);
		memset(value, 0x00, sizeof(value));
		EMV_DataGet(0x5A, &usTagLen, value);
		
                if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof(szAscii));
			inFunc_BCD_to_ASCII(szAscii, value, usTagLen);
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "TAG_5A = %s, length = %d", szAscii, usTagLen);
                        inLogPrintf(AT, szTemplate);
		}

		memset(pobTran->srEMVRec.usz5A_ApplPan, 0x00, sizeof(pobTran->srEMVRec.usz5A_ApplPan));
		pobTran->srEMVRec.in5A_ApplPanLen = usTagLen;
		memcpy(&pobTran->srEMVRec.usz5A_ApplPan[0], &value[0], pobTran->srEMVRec.in5A_ApplPanLen);

		/* 5F24 */
		usTagLen = sizeof(value);
		memset(value, 0x00, sizeof(value));
		EMV_DataGet(0x5F24, &usTagLen, value);
		
		if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof(szAscii));
			inFunc_BCD_to_ASCII(szAscii, value, usTagLen);
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "TAG_5F24 = %s, length = %d", szAscii, usTagLen);
                        inLogPrintf(AT, szTemplate);
		}

		memset(pobTran->srEMVRec.usz5F24_ExpireDate, 0x00, sizeof(pobTran->srEMVRec.usz5F24_ExpireDate));
		pobTran->srEMVRec.in5F24_ExpireDateLen = usTagLen;
		memcpy(&pobTran->srEMVRec.usz5F24_ExpireDate[0], &value[0], pobTran->srEMVRec.in5F24_ExpireDateLen);

		/* 5F2A */
		usTagLen = sizeof(value);
		memset(value, 0x00, sizeof(value));
		EMV_DataGet(0x5F2A, &usTagLen, value);
		
		if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof(szAscii));
			inFunc_BCD_to_ASCII(szAscii, value, usTagLen);
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "TAG_5F2A = %s, length = %d", szAscii, usTagLen);
                        inLogPrintf(AT, szTemplate);
		}

		memset(pobTran->srEMVRec.usz5F2A_TransCurrCode, 0x00, sizeof(pobTran->srEMVRec.usz5F2A_TransCurrCode));
		pobTran->srEMVRec.in5F2A_TransCurrCodeLen = usTagLen;
		memcpy(&pobTran->srEMVRec.usz5F2A_TransCurrCode[0], &value[0], pobTran->srEMVRec.in5F2A_TransCurrCodeLen);

		/* 5F34 */
		usTagLen = sizeof(value);
		memset(value, 0x00, sizeof(value));
		EMV_DataGet(0x5F34, &usTagLen, value);
		
		if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof(szAscii));
			inFunc_BCD_to_ASCII(szAscii, value, usTagLen);
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "TAG_5F34 = %s, length = %d", szAscii, usTagLen);
                        inLogPrintf(AT, szTemplate);
		}

		memset(pobTran->srEMVRec.usz5F34_ApplPanSeqnum, 0x00, sizeof(pobTran->srEMVRec.usz5F34_ApplPanSeqnum));
		pobTran->srEMVRec.in5F34_ApplPanSeqnumLen = usTagLen;
		memcpy(&pobTran->srEMVRec.usz5F34_ApplPanSeqnum[0], &value[0], pobTran->srEMVRec.in5F34_ApplPanSeqnumLen);

		/* 8A（Ascii） */
		usTagLen = sizeof(value);
		memset(value, 0x00, sizeof(value));
		EMV_DataGet(0x8A, &usTagLen, value);
		
		if (ginDebug == VS_TRUE)
		{
			sprintf(szTemplate, "TAG_8A = %s, length = %d", value, usTagLen);
                        inLogPrintf(AT, szTemplate);
		}

		memset(pobTran->srEMVRec.usz8A_AuthRespCode, 0x00, sizeof(pobTran->srEMVRec.usz8A_AuthRespCode));
		pobTran->srEMVRec.in8A_AuthRespCodeLen = usTagLen;
		memcpy(&pobTran->srEMVRec.usz8A_AuthRespCode[0], &value[0], pobTran->srEMVRec.in8A_AuthRespCodeLen);

		/* 82 */
		usTagLen = sizeof(value);
		memset(value, 0x00, sizeof(value));
		EMV_DataGet(0x82, &usTagLen, value);
		
		if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof(szAscii));
			inFunc_BCD_to_ASCII(szAscii, value, usTagLen);
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "TAG_82 = %s, length = %d", szAscii, usTagLen);
                        inLogPrintf(AT, szTemplate);
		}

		memset(pobTran->srEMVRec.usz82_AIP, 0x00, sizeof(pobTran->srEMVRec.usz82_AIP));
		pobTran->srEMVRec.in82_AIPLen = usTagLen;
		memcpy(&pobTran->srEMVRec.usz82_AIP[0], &value[0], pobTran->srEMVRec.in82_AIPLen);

		/* 84 */
		usTagLen = sizeof(value);
		memset(value, 0x00, sizeof(value));
		EMV_DataGet(0x84, &usTagLen, value);
		
		if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof(szAscii));
			inFunc_BCD_to_ASCII(szAscii, value, usTagLen);
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "TAG_84 = %s, length = %d", szAscii, usTagLen);
                        inLogPrintf(AT, szTemplate);
		}

		memset(pobTran->srEMVRec.usz84_DF_NAME, 0x00, sizeof(pobTran->srEMVRec.usz84_DF_NAME));
		pobTran->srEMVRec.in84_DFNameLen = usTagLen;
		memcpy(&pobTran->srEMVRec.usz84_DF_NAME[0], &value[0], pobTran->srEMVRec.in84_DFNameLen);

		/* 95 */
		usTagLen = sizeof(value);
		memset(value, 0x00, sizeof(value));
		EMV_DataGet(0x95, &usTagLen, value);
		
		if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof(szAscii));
			inFunc_BCD_to_ASCII(szAscii, value, usTagLen);
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "TAG_95 = %s, length = %d", szAscii, usTagLen);
                        inLogPrintf(AT, szTemplate);
		}

		memset(pobTran->srEMVRec.usz95_TVR, 0x00, sizeof(pobTran->srEMVRec.usz95_TVR));
		pobTran->srEMVRec.in95_TVRLen = usTagLen;
		memcpy(&pobTran->srEMVRec.usz95_TVR[0], &value[0], pobTran->srEMVRec.in95_TVRLen);

		/* 9A */
		usTagLen = sizeof(value);
		memset(value, 0x00, sizeof(value));
		EMV_DataGet(0x9A, &usTagLen, value);
		
		if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof(szAscii));
			inFunc_BCD_to_ASCII(szAscii, value, usTagLen);
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "TAG_9A = %s, length = %d", szAscii, usTagLen);
                        inLogPrintf(AT, szTemplate);
		}

		memset(pobTran->srEMVRec.usz9A_TranDate, 0x00, sizeof(pobTran->srEMVRec.usz9A_TranDate));
		pobTran->srEMVRec.in9A_TranDateLen = usTagLen;
		memcpy(&pobTran->srEMVRec.usz9A_TranDate[0], &value[0], pobTran->srEMVRec.in9A_TranDateLen);

		/* 9B */
		usTagLen = sizeof(value);
		memset(value, 0x00, sizeof(value));
		EMV_DataGet(0x9B, &usTagLen, value);
		
		if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof(szAscii));
			inFunc_BCD_to_ASCII(szAscii, value, usTagLen);
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "TAG_9B = %s, length = %d", szAscii, usTagLen);
                        inLogPrintf(AT, szTemplate);
		}

		memset(pobTran->srEMVRec.usz9B_TSI, 0x00, sizeof(pobTran->srEMVRec.usz9B_TSI));
		pobTran->srEMVRec.in9B_TSILen = usTagLen;
		memcpy(&pobTran->srEMVRec.usz9B_TSI[0], &value[0], pobTran->srEMVRec.in9B_TSILen);

		/* 9C */
		usTagLen = sizeof(value);
		memset(value, 0x00, sizeof(value));
		EMV_DataGet(0x9C, &usTagLen, value);
		
		if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof(szAscii));
			inFunc_BCD_to_ASCII(szAscii, value, usTagLen);
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "TAG_9C = %s, length = %d", szAscii, usTagLen);
                        inLogPrintf(AT, szTemplate);
		}

		memset(pobTran->srEMVRec.usz9C_TranType, 0x00, sizeof(pobTran->srEMVRec.usz9C_TranType));
		pobTran->srEMVRec.in9C_TranTypeLen = usTagLen;
		memcpy(&pobTran->srEMVRec.usz9C_TranType[0], &value[0], pobTran->srEMVRec.in9C_TranTypeLen);

		/* 9F02 */
		usTagLen = sizeof(value);
		memset(value, 0x00, sizeof(value));
		EMV_DataGet(0x9F02, &usTagLen, value);
		
		if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof(szAscii));
			inFunc_BCD_to_ASCII(szAscii, value, usTagLen);
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "TAG_9F02 = %s, length = %d", szAscii, usTagLen);
                        inLogPrintf(AT, szTemplate);
		}

		memset(pobTran->srEMVRec.usz9F02_AmtAuthNum, 0x00, sizeof(pobTran->srEMVRec.usz9F02_AmtAuthNum));
		pobTran->srEMVRec.in9F02_AmtAuthNumLen = usTagLen;
		memcpy(&pobTran->srEMVRec.usz9F02_AmtAuthNum[0], &value[0], pobTran->srEMVRec.in9F02_AmtAuthNumLen);

		/* 9F03 */
		usTagLen = sizeof(value);
		memset(value, 0x00, sizeof(value));
		EMV_DataGet(0x9F03, &usTagLen, value);
		
		if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof(szAscii));
			inFunc_BCD_to_ASCII(szAscii, value, usTagLen);
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "TAG_9F03 = %s, length = %d", szAscii, usTagLen);
                        inLogPrintf(AT, szTemplate);
		}

		memset(pobTran->srEMVRec.usz9F03_AmtOtherNum, 0x00, sizeof(pobTran->srEMVRec.usz9F03_AmtOtherNum));
		pobTran->srEMVRec.in9F03_AmtOtherNumLen = usTagLen;
		memcpy(&pobTran->srEMVRec.usz9F03_AmtOtherNum[0], &value[0], pobTran->srEMVRec.in9F03_AmtOtherNumLen);

		/* 9F09 */
		usTagLen = sizeof(value);
		memset(value, 0x00, sizeof(value));
		EMV_DataGet(0x9F09, &usTagLen, value);
		
		if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof(szAscii));
			inFunc_BCD_to_ASCII(szAscii, value, usTagLen);
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "TAG_9F09 = %s, length = %d", szAscii, usTagLen);
                        inLogPrintf(AT, szTemplate);
		}

		memset(pobTran->srEMVRec.usz9F09_TermVerNum, 0x00, sizeof(pobTran->srEMVRec.usz9F09_TermVerNum));
		pobTran->srEMVRec.in9F09_TermVerNumLen = usTagLen;
		memcpy(&pobTran->srEMVRec.usz9F09_TermVerNum[0], &value[0], pobTran->srEMVRec.in9F09_TermVerNumLen);

		/* 9F10 */
		usTagLen = sizeof(value);
		memset(value, 0x00, sizeof(value));
		EMV_DataGet(0x9F10, &usTagLen, value);
		
		if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof(szAscii));
			inFunc_BCD_to_ASCII(szAscii, value, usTagLen);
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "TAG_9F10 = %s, length = %d", szAscii, usTagLen);
                        inLogPrintf(AT, szTemplate);
		}

		memset(pobTran->srEMVRec.usz9F10_IssuerAppData, 0x00, sizeof(pobTran->srEMVRec.usz9F10_IssuerAppData));
		pobTran->srEMVRec.in9F10_IssuerAppDataLen = usTagLen;
		memcpy(&pobTran->srEMVRec.usz9F10_IssuerAppData[0], &value[0], pobTran->srEMVRec.in9F10_IssuerAppDataLen);

		/* 9F1A */
		usTagLen = sizeof(value);
		memset(value, 0x00, sizeof(value));
		EMV_DataGet(0x9F1A, &usTagLen, value);
		
		if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof(szAscii));
			inFunc_BCD_to_ASCII(szAscii, value, usTagLen);
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "TAG_9F1A = %s, length = %d", szAscii, usTagLen);
                        inLogPrintf(AT, szTemplate);
		}

		memset(pobTran->srEMVRec.usz9F1A_TermCountryCode, 0x00, sizeof(pobTran->srEMVRec.usz9F1A_TermCountryCode));
		pobTran->srEMVRec.in9F1A_TermCountryCodeLen = usTagLen;
		memcpy(&pobTran->srEMVRec.usz9F1A_TermCountryCode[0], &value[0], pobTran->srEMVRec.in9F1A_TermCountryCodeLen);

		/* 9F1E */
		usTagLen = sizeof(value);
		memset(value, 0x00, sizeof(value));
		EMV_DataGet(0x9F1E, &usTagLen, value);
		
		if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof(szAscii));
			inFunc_BCD_to_ASCII(szAscii, value, usTagLen);
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "TAG_9F1E = %s, length = %d", szAscii, usTagLen);
                        inLogPrintf(AT, szTemplate);
		}

		memset(pobTran->srEMVRec.usz9F1E_IFDNum, 0x00, sizeof(pobTran->srEMVRec.usz9F1E_IFDNum));
		pobTran->srEMVRec.in9F1E_IFDNumLen = usTagLen;
		memcpy(&pobTran->srEMVRec.usz9F1E_IFDNum[0], &value[0], pobTran->srEMVRec.in9F1E_IFDNumLen);

		/* 9F26 */
		usTagLen = sizeof(value);
		memset(value, 0x00, sizeof(value));
		EMV_DataGet(0x9F26, &usTagLen, value);
		
		if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof(szAscii));
			inFunc_BCD_to_ASCII(szAscii, value, usTagLen);
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "TAG_9F26 = %s, length = %d", szAscii, usTagLen);
                        inLogPrintf(AT, szTemplate);
		}

		memset(pobTran->srEMVRec.usz9F26_ApplCryptogram, 0x00, sizeof(pobTran->srEMVRec.usz9F26_ApplCryptogram));
		pobTran->srEMVRec.in9F26_ApplCryptogramLen = usTagLen;
		memcpy(&pobTran->srEMVRec.usz9F26_ApplCryptogram[0], &value[0], pobTran->srEMVRec.in9F26_ApplCryptogramLen);

		/* 9F27 */
		usTagLen = sizeof(value);
		memset(value, 0x00, sizeof(value));
		EMV_DataGet(0x9F27, &usTagLen, value);
		
		if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof(szAscii));
			inFunc_BCD_to_ASCII(szAscii, value, usTagLen);
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "TAG_9F27 = %s, length = %d", szAscii, usTagLen);
                        inLogPrintf(AT, szTemplate);
		}

		memset(pobTran->srEMVRec.usz9F27_CID, 0x00, sizeof(pobTran->srEMVRec.usz9F27_CID));
		pobTran->srEMVRec.in9F27_CIDLen = usTagLen;
		memcpy(&pobTran->srEMVRec.usz9F27_CID[0], &value[0], pobTran->srEMVRec.in9F27_CIDLen);

		/* 9F33 */
		usTagLen = sizeof(value);
		memset(value, 0x00, sizeof(value));
		EMV_DataGet(0x9F33, &usTagLen, value);
		
                if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof(szAscii));
			inFunc_BCD_to_ASCII(szAscii, value, usTagLen);
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "TAG_9F33 = %s, length = %d", szAscii, usTagLen);
                        inLogPrintf(AT, szTemplate);
		}

		memset(pobTran->srEMVRec.usz9F33_TermCapabilities, 0x00, sizeof(pobTran->srEMVRec.usz9F33_TermCapabilities));
		pobTran->srEMVRec.in9F33_TermCapabilitiesLen = usTagLen;
		memcpy(&pobTran->srEMVRec.usz9F33_TermCapabilities[0], &value[0], pobTran->srEMVRec.in9F33_TermCapabilitiesLen);

		/* 9F34 */
		usTagLen = sizeof(value);
		memset(value, 0x00, sizeof(value));
		EMV_DataGet(0x9F34, &usTagLen, value);
		
                if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof(szAscii));
			inFunc_BCD_to_ASCII(szAscii, value, usTagLen);
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "TAG_9F34 = %s, length = %d", szAscii, usTagLen);
                        inLogPrintf(AT, szTemplate);
		}

		memset(pobTran->srEMVRec.usz9F34_CVM, 0x00, sizeof(pobTran->srEMVRec.usz9F34_CVM));
		pobTran->srEMVRec.in9F34_CVMLen = usTagLen;
		memcpy(&pobTran->srEMVRec.usz9F34_CVM[0], &value[0], pobTran->srEMVRec.in9F34_CVMLen);

		/* 9F35 */
		usTagLen = sizeof(value);
		memset(value, 0x00, sizeof(value));
		EMV_DataGet(0x9F35, &usTagLen, value);
		
                if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof(szAscii));
			inFunc_BCD_to_ASCII(szAscii, value, usTagLen);
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "TAG_9F35 = %s, length = %d", szAscii, usTagLen);
                        inLogPrintf(AT, szTemplate);
		}

		memset(pobTran->srEMVRec.usz9F35_TermType, 0x00, sizeof(pobTran->srEMVRec.usz9F35_TermType));
		pobTran->srEMVRec.in9F35_TermTypeLen = usTagLen;
		memcpy(&pobTran->srEMVRec.usz9F35_TermType[0], &value[0], pobTran->srEMVRec.in9F35_TermTypeLen);

		/* 9F36 */
		usTagLen = sizeof(value);
		memset(value, 0x00, sizeof(value));
		EMV_DataGet(0x9F36, &usTagLen, value);
		
                if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof(szAscii));
			inFunc_BCD_to_ASCII(szAscii, value, usTagLen);
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "TAG_9F36 = %s, length = %d", szAscii, usTagLen);
			inLogPrintf(AT, szTemplate);
		}
                        

		memset(pobTran->srEMVRec.usz9F36_ATC, 0x00, sizeof(pobTran->srEMVRec.usz9F36_ATC));
		pobTran->srEMVRec.in9F36_ATCLen = usTagLen;
		memcpy(&pobTran->srEMVRec.usz9F36_ATC[0], &value[0], pobTran->srEMVRec.in9F36_ATCLen);

		/* 9F37 */
		usTagLen = sizeof(value);
		memset(value, 0x00, sizeof(value));
		EMV_DataGet(0x9F37, &usTagLen, value);
		
                if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof(szAscii));
			inFunc_BCD_to_ASCII(szAscii, value, usTagLen);
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "TAG_9F37 = %s, length = %d", szAscii, usTagLen);
			inLogPrintf(AT, szTemplate);
		}
                        
		memset(pobTran->srEMVRec.usz9F37_UnpredictNum, 0x00, sizeof(pobTran->srEMVRec.usz9F37_UnpredictNum));
		pobTran->srEMVRec.in9F37_UnpredictNumLen = usTagLen;
		memcpy(&pobTran->srEMVRec.usz9F37_UnpredictNum[0], &value[0], pobTran->srEMVRec.in9F37_UnpredictNumLen);

		/* 9F41 */
		usTagLen = sizeof(value);
		memset(value, 0x00, sizeof(value));
		EMV_DataGet(0x9F41, &usTagLen, value);
		
                if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof(szAscii));
			inFunc_BCD_to_ASCII(szAscii, value, usTagLen);
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "TAG_9F41 = %s, length = %d", szAscii, usTagLen);
                        inLogPrintf(AT, szTemplate);
		}

		memset(pobTran->srEMVRec.usz9F41_TransSeqCounter, 0x00, sizeof(pobTran->srEMVRec.usz9F41_TransSeqCounter));
		pobTran->srEMVRec.in9F41_TransSeqCounterLen = usTagLen;
		memcpy(&pobTran->srEMVRec.usz9F41_TransSeqCounter[0], &value[0], pobTran->srEMVRec.in9F41_TransSeqCounterLen);

		/* 9F5B */
		usTagLen = sizeof(value);
		memset(value, 0x00, sizeof(value));
		EMV_DataGet(0x9F5B, &usTagLen, value);
		
                if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof(szAscii));
			inFunc_BCD_to_ASCII(szAscii, value, usTagLen);
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "TAG_9F5B = %s, length = %d", szAscii, usTagLen);
			inLogPrintf(AT, szTemplate);
		}
                        

		memset(pobTran->srEMVRec.usz9F5B_ISR, 0x00, sizeof(pobTran->srEMVRec.usz9F5B_ISR));
		pobTran->srEMVRec.in9F5B_ISRLen = usTagLen;
		memcpy(&pobTran->srEMVRec.usz9F5B_ISR[0], &value[0], pobTran->srEMVRec.in9F5B_ISRLen);

		if (pobTran->srBRec.uszEMVFallBackBit == VS_TRUE)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			if (inFunc_TWNAddDataToEMVPacket(pobTran, TAG_DFEC_FALLBACK_INDICATOR, (unsigned char *)&szTemplate[0]) > 0)
			{
				memset(pobTran->srEMVRec.uszDFEC_FallBackIndicator, 0x00, sizeof(pobTran->srEMVRec.uszDFEC_FallBackIndicator));
				pobTran->srEMVRec.inDFEC_FallBackIndicatorLen = szTemplate[2];
				memcpy((char *)&pobTran->srEMVRec.uszDFEC_FallBackIndicator[0], &szTemplate[3], pobTran->srEMVRec.inDFEC_FallBackIndicatorLen);
			}

			memset(szTemplate, 0x00, sizeof(szTemplate));
			if (inFunc_TWNAddDataToEMVPacket(pobTran, TAG_DFED_CHIP_CONDITION_CODE, (unsigned char *)&szTemplate[0]) > 0)
			{
				memset(pobTran->srEMVRec.uszDFED_ChipConditionCode, 0x00, sizeof(pobTran->srEMVRec.uszDFED_ChipConditionCode));
				pobTran->srEMVRec.inDFED_ChipConditionCodeLen = szTemplate[2];
				memcpy((char *)&pobTran->srEMVRec.uszDFED_ChipConditionCode[0], &szTemplate[3], pobTran->srEMVRec.inDFED_ChipConditionCodeLen);
			}

			memset(szTemplate, 0x00, sizeof(szTemplate));
			if (inFunc_TWNAddDataToEMVPacket(pobTran, TAG_DFEE_TERMINAL_ENTRY_CAPABILITY, (unsigned char *)&szTemplate[0]) > 0)
			{
				memset(pobTran->srEMVRec.uszDFEE_TerEntryCap, 0x00, sizeof(pobTran->srEMVRec.uszDFEE_TerEntryCap));
				pobTran->srEMVRec.inDFEE_TerEntryCapLen = szTemplate[2];
				memcpy((char *)&pobTran->srEMVRec.uszDFEE_TerEntryCap[0], &szTemplate[3], pobTran->srEMVRec.inDFEE_TerEntryCapLen);
			}

			memset(szTemplate, 0x00, sizeof(szTemplate));
			if (inFunc_TWNAddDataToEMVPacket(pobTran, TAG_DFEF_REASON_ONLINE_CODE, (unsigned char *)&szTemplate[0]) > 0)
			{
				memset(pobTran->srEMVRec.uszDFEF_ReasonOnlineCode, 0x00, sizeof(pobTran->srEMVRec.uszDFEF_ReasonOnlineCode));
				pobTran->srEMVRec.inDFEF_ReasonOnlineCodeLen = szTemplate[2];
				memcpy((char *)&pobTran->srEMVRec.uszDFEF_ReasonOnlineCode[0], &szTemplate[3], pobTran->srEMVRec.inDFEF_ReasonOnlineCodeLen);
			}
		}
		else
		{
			if (pobTran->inTransactionCode == _SALE_ ||
			    pobTran->inTransactionCode == _REDEEM_SALE_ ||
			    pobTran->inTransactionCode == _INST_SALE_ ||
			    pobTran->inTransactionCode == _PRE_AUTH_ ||
			    pobTran->inTransactionCode == _TC_UPLOAD_)
			{
				/* 先hot code */
				/* DFEE */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				if (inFunc_TWNAddDataToEMVPacket(pobTran, TAG_DFEE_TERMINAL_ENTRY_CAPABILITY, (unsigned char *)&szTemplate[0]) > 0)
				{
					memset(pobTran->srEMVRec.uszDFEE_TerEntryCap, 0x00, sizeof(pobTran->srEMVRec.uszDFEE_TerEntryCap));
					pobTran->srEMVRec.inDFEE_TerEntryCapLen = szTemplate[2];
					memcpy((char *)&pobTran->srEMVRec.uszDFEE_TerEntryCap[0], &szTemplate[3], pobTran->srEMVRec.inDFEE_TerEntryCapLen);
				}
				
                                if (ginDebug == VS_TRUE)
				{
					memset(szAscii, 0x00, sizeof(szAscii));
					inFunc_BCD_to_ASCII(szAscii, (unsigned char*)&szTemplate[3], pobTran->srEMVRec.inDFEE_TerEntryCapLen);
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "TAG_DFEE = %s, length = %d", szAscii, pobTran->srEMVRec.inDFEE_TerEntryCapLen);
                                        inLogPrintf(AT, szDebugMsg);
				}

				/* DFEF */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				if (inFunc_TWNAddDataToEMVPacket(pobTran, TAG_DFEF_REASON_ONLINE_CODE, (unsigned char *)&szTemplate[0]) > 0)
				{
					memset(pobTran->srEMVRec.uszDFEF_ReasonOnlineCode, 0x00, sizeof(pobTran->srEMVRec.uszDFEF_ReasonOnlineCode));
					pobTran->srEMVRec.inDFEF_ReasonOnlineCodeLen = szTemplate[2];
					memcpy((char *)&pobTran->srEMVRec.uszDFEF_ReasonOnlineCode[0], &szTemplate[3], pobTran->srEMVRec.inDFEF_ReasonOnlineCodeLen);
				}
				
                                if (ginDebug == VS_TRUE)
				{
					memset(szAscii, 0x00, sizeof(szAscii));
					inFunc_BCD_to_ASCII(szAscii, (unsigned char*)&szTemplate[3], pobTran->srEMVRec.inDFEF_ReasonOnlineCodeLen);
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "TAG_DFEF = %s, length = %d", szAscii, pobTran->srEMVRec.inDFEF_ReasonOnlineCodeLen);
                                        inLogPrintf(AT, szDebugMsg);
				}
				
			}
			
		}
		
	}
	else if (pobTran->srBRec.uszEMVFallBackBit == VS_TRUE)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		if (inFunc_TWNAddDataToEMVPacket(pobTran, TAG_DFEC_FALLBACK_INDICATOR, (unsigned char *)&szTemplate[0]) > 0)
		{
			memset(pobTran->srEMVRec.uszDFEC_FallBackIndicator, 0x00, sizeof(pobTran->srEMVRec.uszDFEC_FallBackIndicator));
			pobTran->srEMVRec.inDFEC_FallBackIndicatorLen = szTemplate[2];
			memcpy((char *)&pobTran->srEMVRec.uszDFEC_FallBackIndicator[0], &szTemplate[3], pobTran->srEMVRec.inDFEC_FallBackIndicatorLen);
		}

		memset(szTemplate, 0x00, sizeof(szTemplate));
		if (inFunc_TWNAddDataToEMVPacket(pobTran, TAG_DFED_CHIP_CONDITION_CODE, (unsigned char *)&szTemplate[0]) > 0)
		{
			memset(pobTran->srEMVRec.uszDFED_ChipConditionCode, 0x00, sizeof(pobTran->srEMVRec.uszDFED_ChipConditionCode));
			pobTran->srEMVRec.inDFED_ChipConditionCodeLen = szTemplate[2];
			memcpy((char *)&pobTran->srEMVRec.uszDFED_ChipConditionCode[0], &szTemplate[3], pobTran->srEMVRec.inDFED_ChipConditionCodeLen);
		}

		memset(szTemplate, 0x00, sizeof(szTemplate));
		if (inFunc_TWNAddDataToEMVPacket(pobTran, TAG_DFEE_TERMINAL_ENTRY_CAPABILITY, (unsigned char *)&szTemplate[0]) > 0)
		{
			memset(pobTran->srEMVRec.uszDFEE_TerEntryCap, 0x00, sizeof(pobTran->srEMVRec.uszDFEE_TerEntryCap));
			pobTran->srEMVRec.inDFEE_TerEntryCapLen = szTemplate[2];
			memcpy((char *)&pobTran->srEMVRec.uszDFEE_TerEntryCap[0], &szTemplate[3], pobTran->srEMVRec.inDFEE_TerEntryCapLen);
		}

		memset(szTemplate, 0x00, sizeof(szTemplate));
		if (inFunc_TWNAddDataToEMVPacket(pobTran, TAG_DFEF_REASON_ONLINE_CODE, (unsigned char *)&szTemplate[0]) > 0)
		{
			memset(pobTran->srEMVRec.uszDFEF_ReasonOnlineCode, 0x00, sizeof(pobTran->srEMVRec.uszDFEF_ReasonOnlineCode));
			pobTran->srEMVRec.inDFEF_ReasonOnlineCodeLen = szTemplate[2];
			memcpy((char *)&pobTran->srEMVRec.uszDFEF_ReasonOnlineCode[0], &szTemplate[3], pobTran->srEMVRec.inDFEF_ReasonOnlineCodeLen);
		}
		
	}

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_FuncEMVPrepareBatch() END!");
        
	return (VS_SUCCESS);
}


int inNCCC_EMVUnPackData55(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf, int inLen)
{
	int		inTotalLen;
	unsigned short	usTag;

	memset(pobTran->srEMVRec.usz8A_AuthRespCode, 0x00, sizeof(pobTran->srEMVRec.usz8A_AuthRespCode));
	pobTran->srEMVRec.in8A_AuthRespCodeLen = strlen(pobTran->srBRec.szRespCode);
	memcpy(&pobTran->srEMVRec.usz8A_AuthRespCode[0], &pobTran->srBRec.szRespCode[0], pobTran->srEMVRec.in8A_AuthRespCodeLen);

	for (inTotalLen = 0; inTotalLen < inLen;)
	{
		usTag = (unsigned short)uszUnPackBuf[inTotalLen];

		if (uszUnPackBuf[inTotalLen ++] & 0x1F00)
			usTag += ((unsigned short)uszUnPackBuf[inTotalLen ++]);

		switch (usTag)
		{
			case _TAG_91_ISS_AUTH_DATA_ :
				memset(pobTran->srEMVRec.usz91_IssuerAuthData, 0x00, sizeof(pobTran->srEMVRec.usz91_IssuerAuthData));
				pobTran->srEMVRec.in91_IssuerAuthDataLen = (unsigned short)uszUnPackBuf[inTotalLen ++];
				memcpy(&pobTran->srEMVRec.usz91_IssuerAuthData[0], (char *)&uszUnPackBuf[inTotalLen], pobTran->srEMVRec.in91_IssuerAuthDataLen);
				inTotalLen += pobTran->srEMVRec.in91_IssuerAuthDataLen;
				break;
			case _TAG_71_ISUER_SCRPT_TEMPL_ :
				memset(pobTran->srEMVRec.usz71_IssuerScript1, 0x00, sizeof(pobTran->srEMVRec.usz71_IssuerScript1));
				pobTran->srEMVRec.in71_IssuerScript1Len = (unsigned short)uszUnPackBuf[inTotalLen ++];
				memcpy(&pobTran->srEMVRec.usz71_IssuerScript1[0], (char *)&uszUnPackBuf[inTotalLen], pobTran->srEMVRec.in71_IssuerScript1Len);
				inTotalLen += pobTran->srEMVRec.in71_IssuerScript1Len;
				break;
			case _TAG_72_ISUER_SCRPT_TEMPL_ :
				memset(pobTran->srEMVRec.usz72_IssuerScript2, 0x00, sizeof(pobTran->srEMVRec.usz72_IssuerScript2));
				pobTran->srEMVRec.in72_IssuerScript2Len = (unsigned short)uszUnPackBuf[inTotalLen ++];
				memcpy(&pobTran->srEMVRec.usz72_IssuerScript2[0], (char *)&uszUnPackBuf[inTotalLen], pobTran->srEMVRec.in72_IssuerScript2Len);
				inTotalLen += pobTran->srEMVRec.in72_IssuerScript2Len;
				break;
			default :
				return (VS_SUCCESS);
		}
	}

	return (VS_SUCCESS);
}

/*
Function        :inEMV_Set_TagValue_During_Txn
Date&Time       :2016/9/30 下午 1:28
Describe        :交易中直接改Tag值，基本上不會直接設定，這邊因為DCC會用到
*/
int inEMV_Set_TagValue_During_Txn(unsigned short usTag, unsigned short usLen, unsigned char *uszValue)
{
	char		szDebugMsg[300 + 1];
	char		szAscii[300 + 1];
	unsigned short	usRetVal;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inEMV_Set_TagValue_During_Txn() START !");
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Tag :%X Len :%d Value :%s", usTag, usLen, uszValue);
		inLogPrintf(AT, szDebugMsg);
		
		memset(szAscii, 0x00, sizeof(szAscii));
		inFunc_BCD_to_ASCII(szAscii, uszValue, usLen);
		sprintf(szDebugMsg, "Ascii : %s", szAscii);
		inLogPrintf(AT, szDebugMsg);
	}
	
	usRetVal = EMV_TxnDataSet(usTag, usLen, uszValue);

	/* 不論是Library function執行失敗或是中間callback跑的流程失敗都是不成功 */
    	if (usRetVal != d_EMVAPLIB_OK)
    	{
                if (ginDebug == VS_TRUE)
                {
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                        sprintf(szDebugMsg, "Tag :%X Len :%d Value :%s", usTag, usLen, uszValue);
                        inLogPrintf(AT, szDebugMsg);
			
                        memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                        sprintf(szDebugMsg, "EMV_TxnDataSet_ERR:0X%04x", usRetVal);
                        inLogPrintf(AT, szDebugMsg);
                }
                
		return (VS_ERROR);
    	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inEMV_Set_TagValue_During_Txn() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_DCC_EMV_Set_Value
Date&Time       :2016/9/30 下午 1:43
Describe        :若是DCC 交易要改5F2A和9F02 兩個Tag，而且晶片卡都force online
*/
int inNCCC_DCC_EMV_Set_Value(TRANSACTION_OBJECT * pobTran)
{
	int		inRetVal;
	char		szTemplate[100 + 1];
	unsigned char	uszHex[100 + 1];
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_DCC_EMV_Set_Value() START !");
	}
	
	if (pobTran->srBRec.uszDCCTransBit != VS_TRUE)
	{
		return (VS_SUCCESS);
	}
	
	/* 5F2A */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memset(uszHex, 0x00, sizeof(uszHex));
	sprintf(szTemplate, "0%s", pobEmvTran.srBRec.szDCC_FCN);
	inFunc_ASCII_to_BCD(uszHex, szTemplate, 2);
	inRetVal = inEMV_Set_TagValue_During_Txn(d_TAG_TERM_CURRENCY_CODE, 2, (unsigned char*)uszHex);
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	/* 9F02 */
	memset(uszHex, 0x00, sizeof(uszHex));
	inFunc_ASCII_to_BCD(uszHex, pobEmvTran.srBRec.szDCC_FCA, 6);
	inRetVal = inEMV_Set_TagValue_During_Txn(d_TAG_AMOUNT_AUTHORIZED, 6, (unsigned char*)uszHex);		/* 要注意ascii to bcd會出現0x00，strlen會出錯 */
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	/* DCC交易所有EMV晶片卡一律Force go online，不可有離線授權之交易 (Y1 及Y3 授權) (NCCC規格書)，這邊直接把Floor limit設為0 */
	memset(uszHex, 0x00, sizeof(uszHex));
	inFunc_ASCII_to_BCD(uszHex, "00", 2);
	inRetVal = inEMV_Set_TagValue_During_Txn(d_TAG_FLOOR_LIMIT, 4, uszHex);
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_DCC_EMV_Set_Value() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inEMV_Get_Tag_Value
Date&Time       :2016/10/3 上午 11:19
Describe        :usLen放uszValue陣列的大小
*/
int inEMV_Get_Tag_Value(unsigned short usTag, unsigned short *usLen, unsigned char *uszValue)
{
	int	inRetVal;
	char	szDebugMsg[300 + 1];
	char	szAscii[300 + 1];
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inEMV_Get_Tag_Value() START !");
	}
	
	memset(uszValue, 0x00, *usLen);
	inRetVal = EMV_DataGet(usTag, usLen, uszValue);

	if (inRetVal == d_EMVAPLIB_OK  && *usLen > 0)
	{
		/* ISO Display Debug */
		if (ginDebug == VS_TRUE)
		{
			memset(szAscii, 0x00, sizeof(szAscii));
			inFunc_BCD_to_ASCII(szAscii, uszValue, *usLen);	
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "TAG %X = %s, Length = %d", usTag, uszValue, *usLen);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Ascii : %s", szAscii);
			inLogPrintf(AT, szDebugMsg);
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
                {
			inLogPrintf(AT, "inEMV_Get_Tag_Value() ERROR !");
		
                        memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                        sprintf(szDebugMsg, "EMV_DataGet_ERR:0X%04x", inRetVal);
                        inLogPrintf(AT, szDebugMsg);
                }

	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inEMV_Get_Tag_Value() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inEMV_SetICCReadFailure
Date&Time       :2016/12/5 下午 3:44
Describe        :開啟Fallback
*/
int inEMV_SetICCReadFailure(int inFallbackSwitch)
{
	if (inFallbackSwitch == VS_TRUE)
	{
		ginFallback = VS_TRUE;
	}
	else if (inFallbackSwitch == VS_FALSE)
	{
		ginFallback = VS_FALSE;
	}
	else
	{
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inEMV_GetKernelVersion
Date&Time       :2016/12/28 上午 11:20
Describe        :
*/
int inEMV_GetKernelVersion()
{
	int	inRetVal;
	char	szKernelVersion[100 + 1];
	char	szDebugMsg[100 + 1];
	
	memset(szKernelVersion, 0x00, sizeof(szKernelVersion));
	inRetVal = EMV_TxnKernelVersionGet((unsigned char*)szKernelVersion);
	if (inRetVal != d_EMVAPLIB_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inEMV_GetKernelVersion() ERROR !");

			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "EMV_DataGet_ERR:0X%04x", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Kernel Version : %s", szKernelVersion);
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inEMV_GetCardNoFlow
Date&Time       :2017/3/6 上午 10:07
Describe        :抓卡號，目前只For HG 使用
*/
int inEMV_GetCardNoFlow(TRANSACTION_OBJECT *pobTran)
{
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inEMV_GetCardNoFlow() START !");
	}

	/* 取得Track2資料 */
	if (inCARD_unPackCard(pobTran) != VS_SUCCESS)
		return (VS_ERROR);
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inEMV_GetCardNoFlow() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inEMV_Decide_OnlineEMV_Complete
Date&Time       :2016/10/3 下午 5:25
Describe        :分Host流程
*/
int inEMV_Decide_OnlineEMV_Complete()
{
	int	inRetVal = {0};
	char	szDebugMsg[100 + 1] = {0};
	char	szTRTFileName[12 + 1] = {0};
	char	szFesMode[2 + 1] = {0};
	char	szCFESMode[2 + 1] = {0};
	
	memset(szTRTFileName, 0x00, sizeof(szTRTFileName));
	inGetTRTFileName(szTRTFileName);
	
	memset(szFesMode, 0x00, sizeof(szFesMode));
	inGetNCCCFESMode(szFesMode);
	memset(szCFESMode, 0x00, sizeof(szCFESMode));
	inGetCloud_MFES(szCFESMode);

	if (!memcmp(szTRTFileName, _TRT_FILE_NAME_CREDIT_, strlen(_TRT_FILE_NAME_CREDIT_)))
	{
		if (0)
		{

		}
		else
		{
			if (memcmp(szFesMode, _NCCC_03_MFES_MODE_, 2) == 0)
			{
				/* (需求單 - 107276)自助交易標準做法 CFES by Russell 2019/3/6 上午 10:32 */
				if (memcmp(szCFESMode, "Y", strlen("Y")) == 0)
				{
					inRetVal = inNCCC_ATS_OnlineEMV_Complete(&pobEmvTran);
				}
				else
				{
					inRetVal = inNCCC_MFES_OnlineEMV_Complete(&pobEmvTran);
				}
			}
			else if (memcmp(szFesMode, _NCCC_05_ATS_MODE_, 2) == 0	||
				 memcmp(szFesMode, _NCCC_04_MPAS_MODE_, 2) == 0)
			{
				inRetVal = inNCCC_ATS_OnlineEMV_Complete(&pobEmvTran);
			}
			else
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "inEMV_Decide_OnlineEMV_Complete :EMV無此FES 流程 :%s", szFesMode);
					inLogPrintf(AT, szDebugMsg);
				}
				inRetVal = VS_ERROR;
			}
		}
		
		return (inRetVal);
	}
	else if (!memcmp(szTRTFileName, _TRT_FILE_NAME_DCC_, strlen(_TRT_FILE_NAME_DCC_)))
	{
		inRetVal = inNCCC_DCC_OnlineEMV_Complete(&pobEmvTran);
		
		return (inRetVal);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inEMV_Decide_OnlineEMV_Complete :EMV無此TRT流程 :%s", szTRTFileName);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	
}

/*
Function        :inEMV_Decide_DispHostRespCodeMsg
Date&Time       :2016/11/15 下午 6:25
Describe        :分顯示錯誤代碼流程
*/
int inEMV_Decide_DispHostRespCodeMsg()
{
	int	inRetVal = VS_ERROR;
	
	inRetVal = inNCCC_Func_Decide_DispHostRespCodeMsg(&pobEmvTran);
	
	return (inRetVal);
}

/*
Function        :inEMV_SetDebug
Date&Time       :2021/10/20 下午 4:28
Describe        :
*/
int inEMV_SetDebug(void)
{
	int		inRetVal;
        char		szTemplate[64 + 1];
        DISPLAY_OBJECT  srDispObj;
        
        /* Debug */
	inDISP_Clear_Line(_LINE_8_1_, _LINE_8_8_);
        memset(szTemplate, 0x00, sizeof(szTemplate));
        
	strcpy(szTemplate, "EMV Debug 開關");
 
        inDISP_ChineseFont(szTemplate, _FONTSIZE_8X22_, _LINE_8_3_, _DISP_LEFT_);
        inDISP_ChineseFont("0 = OFF,1 = COM1", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
	inDISP_ChineseFont("2 = COM2,3 = COM3", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
	inDISP_ChineseFont("4 = USB", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);

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
				EMV_SetDebug(FALSE, 0xFF);
                                break;
                        }
                        else if (srDispObj.szOutput[0] == '1')
                        {
                                EMV_SetDebug(TRUE, d_COM1);
                                break;
                        }
			else if (srDispObj.szOutput[0] == '2')
                        {
                                EMV_SetDebug(TRUE, d_COM2);
                                break;
                        }
			else if (srDispObj.szOutput[0] == '3')
                        {
                                EMV_SetDebug(TRUE, d_COM3);
                                break;
                        }
			else if (srDispObj.szOutput[0] == '4')
                        {
                                EMV_SetDebug(TRUE, 0xFF);
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
