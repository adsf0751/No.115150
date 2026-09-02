#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <sqlite3.h>
#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Define_2.h"
#include "../INCLUDES/Transaction.h"
#include "../INCLUDES/TransType.h"
#include "../INCLUDES/AllStruct.h"
#include "../PRINT/Print.h"
#include "../DISPLAY/Display.h"
#include "../DISPLAY/DisTouch.h"
#include "../../HG/HGsrc.h"
#include "../FUNCTION/Sqlite.h"
#include "../FUNCTION/Accum.h"
#include "../FUNCTION/CFGT.h"
#include "../FUNCTION/EDC.h"
#include "../FUNCTION/HDT.h"
#include "../FUNCTION/HDPT.h"
#include "../FUNCTION/Function.h"
#include "../FUNCTION/FuncTable.h"
#include "../FUNCTION/ECR.h"
#include "../FUNCTION/RS232.h"
#include "../FUNCTION/MultiFunc.h"
#include "../FUNCTION/PowerManagement.h"
#include "../FUNCTION/Utility.h"
#include "../../AMEX/AMEXsrc.h"
#include "../../DINERS/DINERSsrc.h"
#include "../../FISC/NCCCfisc.h"
#include "../../EMVSRC/EMVsrc.h"
#include "../../NCCC/NCCCsrc.h"
#include "../../NCCC/NCCCTicketSrc.h"
#include "../../NCCC/NCCCtms.h"
#include "../../NCCC/TAKAsrc.h"
#include "../../NCCC/NCCCEWsrc.h"
#include "../../NCCC/NCCCTrust.h"
#include "../../CTLS/CTLS.h"
#include "../COMM/Comm.h"
#include "../COMM/WiFi.h"
#include "Menu.h"
#include "MenuMsg.h"
#include "Event.h"
#include "Flow.h"
#include "CustomerMenu.h"

int			ginEventCode;   /* 用於儲存IdleMenuKeyIn第一個鍵 */
extern  int		ginIdleDispFlag;
extern	int		ginFindRunTime;
extern	int		ginTouch_Handle;
extern	int		ginDebug;
extern	int		ginMachineType;
extern	int		ginAPVersionType;
extern	int		ginEMVAppSelection;
extern	char		gszTermVersionID[16 + 1];
extern	BYTE		gbECR_UDP_TransBit;
extern	ECR_TABLE	gsrECROb;
extern	unsigned char	guszAlreadySelectMultiAIDBit;

int inEVENT_Test(void)
{
        inLogPrintf(AT, "inEVENT_Test()");
        return (VS_SUCCESS);
}

int inEVENT_Responder(int inKey)
{
        int			inRetVal = VS_ERROR;
        char			szTRTFileName[12 + 1];
	char			szCustomerIndicator[3 + 1] = {0};
	char			szTemplate[3 + 1] = {0};
	char szEnableESG[2+1] = {0};
	RTC_NEXSYS		srRTC;			/* Date & Time */
        EventMenuItem		srEventMenuItem;
        TRANSACTION_OBJECT	pobTran;

#ifdef _TOUCH_CAPBILITY_
	/* 觸發事件關閉觸控檔案 */
	inDisTouch_Flush_TouchFile();
#endif
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
		
        memset((char *)&pobTran, 0x00, sizeof(TRANSACTION_OBJECT));
        memset((char *)&srEventMenuItem, 0x00, sizeof(EventMenuItem));
        memset(szTRTFileName, 0x00, sizeof(szTRTFileName));

        /* 取得EDC時間日期 */
        memset(&srRTC, 0x00, sizeof(RTC_NEXSYS));
	if (inFunc_GetSystemDateAndTime(&srRTC) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	inFunc_Sync_BRec_Date_Time(&pobTran, &srRTC);

        pobTran.srBRec.inHDTIndex = -1 ;	/* reset 為-1 ,因為LoadRec 是從0開始,避免load錯 */
	inLoadHDTRec(0);			/* 這邊先使用NCCC HDT的設定 */
	inLoadHDPTRec(0);			/* 初始化，避免ECR過卡timeout回錯的Host ID */
	ginEMVAppSelection = -1;		/* 初始化，解決EMV流程會自動選的問題 */
	guszAlreadySelectMultiAIDBit = VS_FALSE;/* 初始化，用於紀錄已選擇感應多AID卡 */
	
        srEventMenuItem.inEventCode = inKey;
        
        /* 用於儲存IdleMenuKeyIn第一個鍵 */
        ginEventCode = inKey;
        
#if defined	_LOAD_KEY_AP_
	inRetVal = inMENU_000_MenuFlow_LoadKeyUI(&srEventMenuItem);
#else
	if (ginMachineType == _CASTLE_TYPE_V3C_)
	{
                if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)     ||
                    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)) 
                {
			inRetVal = inMENU_098_MenuFlow_NEWUI(&srEventMenuItem);
                }
		else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_005_FPG_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inRetVal = inMENU_005_FPG_MenuFlow_NEWUI(&srEventMenuItem);
		}
		else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_041_CASH_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inRetVal = inMENU_041_CASH_MenuFlow_NEWUI(&srEventMenuItem);
		}
		else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_) ||
			 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inRetVal = inMENU_123_MenuFlow_NEWUI(&srEventMenuItem);
		}
                else
                {
			inRetVal = inMENU_000_MenuFlow_NEWUI(&srEventMenuItem);
                }
	}
	else if (ginMachineType == _CASTLE_TYPE_V3M_)
	{
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)     ||
                    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)) 
                {
			inRetVal = inMENU_098_MenuFlow_NEWUI(&srEventMenuItem);
                }
		else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_005_FPG_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inRetVal = inMENU_005_FPG_MenuFlow_NEWUI(&srEventMenuItem);
		}
		else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_041_CASH_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inRetVal = inMENU_041_CASH_MenuFlow_NEWUI(&srEventMenuItem);
		}
		else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_) ||
			 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inRetVal = inMENU_123_MenuFlow_NEWUI(&srEventMenuItem);
		}
                else
                {
			inRetVal = inMENU_000_MenuFlow_NEWUI(&srEventMenuItem);
                }
	}
	else if (ginMachineType == _CASTLE_TYPE_V3P_)
	{
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)     ||
                    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)) 
                {
			inRetVal = inMENU_098_MenuFlow_NEWUI(&srEventMenuItem);
                }
		else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_005_FPG_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inRetVal = inMENU_005_FPG_MenuFlow_NEWUI(&srEventMenuItem);
		}
		else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_041_CASH_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inRetVal = inMENU_041_CASH_MenuFlow_NEWUI(&srEventMenuItem);
		}
		/* 客製化124 UI僅修改秒數，其他沿用123，輸入密碼15秒，等待選擇功能30秒 */
		else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_) ||
			 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inRetVal = inMENU_123_MenuFlow_NEWUI(&srEventMenuItem);
		}
                else
                {
			inRetVal = inMENU_000_MenuFlow_NEWUI(&srEventMenuItem);
                }
	}
	else if (ginMachineType == _CASTLE_TYPE_V3UL_)
	{
		inRetVal = inMENU_000_MenuFlow_V3UL(&srEventMenuItem);
	}
	else if (ginMachineType == _CASTLE_TYPE_MP200_)
	{
		inRetVal = inMENU_000_MenuFlow_MP200(&srEventMenuItem);
	}
	else if (ginMachineType == _CASTLE_TYPE_UPT1000_)
	{
		inRetVal = inMENU_000_MenuFlow_NEWUI(&srEventMenuItem);
	}
	else
	{
		inRetVal = inMENU_000_MenuFlow_NEWUI(&srEventMenuItem);
	}

	if (inRetVal != VS_SUCCESS)
	{
		pobTran.inErrorMsg = srEventMenuItem.inErrorMsg;
		inFunc_Display_Error(&pobTran);
	}
#endif
	
        if (inRetVal == VS_SUCCESS)
        {
        	pobTran.inFunctionID = srEventMenuItem.inCode;
        	pobTran.inRunOperationID = srEventMenuItem.inRunOperationID;
        	pobTran.inRunTRTID = srEventMenuItem.inRunTRTID;

                pobTran.inTransactionCode = srEventMenuItem.inCode;
		if (srEventMenuItem.inCode == _TICKET_DEDUCT_	||
		    srEventMenuItem.inCode == _TICKET_REFUND_	||
		    srEventMenuItem.inCode == _TICKET_INQUIRY_	||
		    srEventMenuItem.inCode == _TICKET_TOP_UP_	||
		    srEventMenuItem.inCode == _TICKET_VOID_TOP_UP_)
		{
			pobTran.srTRec.inCode = srEventMenuItem.inCode;
		}
		else if (srEventMenuItem.inCode == _TRUST_SALE_ ||
			 srEventMenuItem.uszTrustTransBit == VS_TRUE)
		{
			pobTran.srTrustRec.inCode = srEventMenuItem.inCode;
		}
		else
		{
			pobTran.srBRec.inCode = srEventMenuItem.inCode;
		}
        	pobTran.srBRec.inOrgCode = srEventMenuItem.inCode;
		
                pobTran.srBRec.uszCUPTransBit = srEventMenuItem.uszCUPTransBit;
		pobTran.srBRec.uszInstallmentBit = srEventMenuItem.uszInstallmentBit;
		pobTran.srBRec.uszRedeemBit = srEventMenuItem.uszRedeemBit;
		pobTran.uszECRBit = srEventMenuItem.uszECRBit;
		pobTran.uszAutoSettleBit = srEventMenuItem.uszAutoSettleBit;
                pobTran.srBRec.uszFiscTransBit = srEventMenuItem.uszFISCTransBit;	/* 確認是金融卡才on */
		pobTran.srBRec.uszMail_OrderBit = srEventMenuItem.uszMailOrderBit;
		pobTran.uszMultiFuncSlaveBit = srEventMenuItem.uszMultiFuncSlaveBit;
		pobTran.srTRec.uszESVCTransBit = srEventMenuItem.uszESVCTransBit;
		pobTran.uszESVCLogOnDispBit = srEventMenuItem.uszESVCLogOnDispBit;
		pobTran.srTrustRec.uszTrustTransBit = srEventMenuItem.uszTrustTransBit;
                
                pobTran.srBRec.lnHGTransactionType = srEventMenuItem.lnHGTransactionType;
		
		/* SmartPay不用簽名 */
		if (pobTran.srBRec.uszFiscTransBit == VS_TRUE)
		{
			pobTran.srBRec.uszNoSignatureBit = VS_TRUE;
		}
		/* [114197] 中心收單作業ESG永續發展優化
		 * [260616] 不管ESG是否有開,初始化參數為要送 "CR",在上傳時會再參考是否有按選擇項
		 * [20260624] 把固定設成"Y"的條件刪除,改為依照功能是否有開設定對應的初始參數 */
		if (ginMachineType == _CASTLE_TYPE_V3C_)
		{
			/* 新增判斷是否有開啟ESG功能, 如果沒抓到資料就設定為N,不改變Timout秒數 */
			if (inGetCHESG_FLAG(szEnableESG) != VS_SUCCESS)
			{
				pobTran.szEsgSendCR[0] = 'N';
			}

			/* 新增判斷是否有開啟ESG功能,如果有開啟時Timeout修改為 6 秒 */
			if (szEnableESG[0] == 'Y')
			{
				pobTran.szEsgSendCR[0] = 'Y';
			} else
			{
				pobTran.szEsgSendCR[0] = 'N';
			}


		} else
		{
			pobTran.szEsgSendCR[0] = 'N';
		}

		inRetVal = inFLOW_RunOperation(&pobTran, srEventMenuItem.inRunOperationID);
		
		if (inRetVal != VS_SUCCESS)
		{
                        vdUtility_SYSFIN_LogMessage(AT, "Trans Failed HostID=(%d) Inv=(%d) Amt=(%d)", pobTran.srBRec.inHDTIndex, pobTran.srBRec.lnOrgInvNum, pobTran.srBRec.lnTxnAmount);
                        
			/* 如果有插SmartPay錯誤要Power Off */
			if (pobTran.uszFISCBit == VS_TRUE)
			{
				inFLOW_RunFunction(&pobTran, _FISC_CARD_POWER_OFF_);
			}
			
			if (ginMachineType == _CASTLE_TYPE_V3UL_)
			{
				/* 被外接設備出錯時回傳 */
				inMultiFunc_SendError(&pobTran, inRetVal);
			}
			else
			{
				if (pobTran.inECRErrorMsg != _ECR_RESPONSE_CODE_NOT_SET_ERROR_)
				{
					inECR_SendError(&pobTran, pobTran.inECRErrorMsg);
				}
				else
				{
					inECR_SendError(&pobTran, inRetVal);
				}
			}
			
			/* 斷線 */
			inCOMM_End(&pobTran);
	
			/* 要先回傳再顯示錯誤訊息 */
			inFunc_Display_Error(&pobTran);
			
			/* 退回晶片卡 */
			if (memcmp(gszTermVersionID, "MD731UAGAS001", strlen("MD731UAGAS001")) == 0)
			{
				
			}
			else
			{
				inNCCC_Func_Check_Card_Still_Exist_Flow(&pobTran, _REMOVE_CARD_ERROR_);
			}
		}
        }
		
        if (inRetVal == VS_SUCCESS)
        {
		/* 沒有TRT的狀況就不跑 */
		if (pobTran.inRunTRTID == _TRT_NULL_)
		{
			inNCCC_Func_Check_Card_Still_Exist_Flow(&pobTran, _REMOVE_CARD_NOT_ERROR_);
		}
		else
		{
			/* 邦柏客製化結帳要走特殊流程 */
			/* (需求單 - 107276)自助交易標準400做法 結帳要走特殊流程 by Russell 2018/12/27 上午 11:20 */
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)         ||
                            !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_) ||
			    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)	     ||
			    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
			{
				if (pobTran.inTransactionCode == _SETTLE_)
				{
					inNCCC_Func_Bumper_Settlement_Flow(&pobTran);
				}
				else
				{
					inEVENT_Run_Normal_TRT(&pobTran);
				}
			}
                        else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)     ||
                                 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_))  
                        {
                                if (pobTran.inTransactionCode == _SETTLE_)
				{
					inNCCC_Func_Mcdonalds_Settlement_Flow(&pobTran);
				}
				else
				{
					inEVENT_Run_Normal_TRT(&pobTran);
				}
                        }
			else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_046_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
				 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_047_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
				 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_075_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
				 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_103_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_))  
                        {
                                if (pobTran.inTransactionCode == _SETTLE_)
				{
					inNCCC_Func_Dutyfree_Settlement_Flow(&pobTran);
				}
				else
				{
					inEVENT_Run_Normal_TRT(&pobTran);
				}
                        }
			else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_035_MIRAMAR_CINEMAS_, _CUSTOMER_INDICATOR_SIZE_))  
                        {
				if (pobTran.inTransactionCode == _SETTLE_)
				{
					inNCCC_Func_CUS_035_MIRAMAR_CINEMAS_Settlement_Flow(&pobTran);
				}
				else
				{
					inEVENT_Run_Normal_TRT(&pobTran);
				}
			}
			else
			{
				/* 如果當筆要連動結帳 */
				if (pobTran.uszAutoSettleBit == VS_TRUE)
				{
					/* 連動結帳流程 */
					inNCCC_Func_AutoSettle_Flow(&pobTran);
				}
				else
				{
					inEVENT_Run_Normal_TRT(&pobTran);
				}
			}
		}
        }/* 走TRT */
	
	/* 客製化123，idle時設為較暗 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)	||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetScreenBrightness(szTemplate);
		if (strlen(szTemplate) > 0)
		{
			inDISP_BackLightSetBrightness(d_BKLIT_LCD_DEVICE_0, atoi(szTemplate));
		}
	}
	
#ifdef _COMMUNICATION_CAPBILITY_
	/* 如果有撥接備援，在這裡回復Ethernet */
	inCOMM_MODEM_DialBackUpOff(&pobTran);
#endif
        /* 回IDLE前，清LED燈號 */
	inCTLS_Clear_LED();
	/* 要回Idle，顯示idle圖片ON */
        ginIdleDispFlag = VS_TRUE;
	
	/* 計時器初始化 */
	inDISP_Timer_Start(_TIMER_NEXSYS_1_, 0);
	inDISP_Timer_Start(_TIMER_NEXSYS_2_, 0);
	inDISP_Timer_Start(_TIMER_NEXSYS_3_, 0);
	inDISP_Timer_Start(_TIMER_NEXSYS_4_, 0);
	
	/* 清鍵盤buffer */
	inFlushKBDBuffer();
	
	if (ginMachineType == _CASTLE_TYPE_V3UL_)
	{
		inMultiFunc_RS232_FlushRxBuffer();
	}
	else
	{
		/* ECR清buffer */
		inECR_FlushRxBuffer();
		gbECR_UDP_TransBit = VS_FALSE;
	}
	
	/* 關閉感應天線，若沒關閉感應天線會和觸控板衝突(有悠遊卡時) */
	inCTLS_Power_Off();
#ifdef _TOUCH_CAPBILITY_
	/* 觸發事件關閉觸控檔案 */
	inDisTouch_Flush_TouchFile();
#endif
	
	if (ginMachineType == _CASTLE_TYPE_V3M_)
	{
		/* 電量管理用 */
		inPWM_StandBy_Mode_Timer_Start();
	}
	
	/* 調節diff使用 */
	
        return (inRetVal);
}

/*
Function        :inEVENT_Run_Normal_TRT
Date&Time       :2018/12/9 下午 5:59
Describe        :
*/
int inEVENT_Run_Normal_TRT(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;
	char	szTRTFileName[12 + 1] = {0};
	char	szTemplate[10 + 1] = {0};
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inEVENT_Run_Normal_TRT() START !");
	}
	
	if (pobTran->inRunTRTID == 0)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "TRT ERROR");
		}
		
		inRetVal = VS_ERROR;
	}
	else
	{
		/* 預防萬一再LoadHDPT一次 */
		if (inLoadHDPTRec(pobTran->srBRec.inHDTIndex) == VS_SUCCESS)
		{
			/* 在這裡決定跑那一個host的TRT */
			inGetTRTFileName(szTRTFileName);

			/* ResetTitle */
			inFunc_ResetTitle(pobTran);

			if (0)
			{
				
			}
			else if (0)
			{
				
			}
			else
			{
				/* 2018/5/8 下午 2:10 DFS需求不再使用大來主機 */
				if (!memcmp(_TRT_FILE_NAME_HG_, szTRTFileName, strlen(_TRT_FILE_NAME_HG_)))
				{
					inRetVal = inHG_RunTRT(pobTran, pobTran->inRunTRTID);
				}
				else if (!memcmp(_TRT_FILE_NAME_CREDIT_, szTRTFileName, strlen(_TRT_FILE_NAME_CREDIT_)))
				{
					inRetVal = inNCCC_RunTRT(pobTran, pobTran->inRunTRTID);
				}
				else if (!memcmp(_TRT_FILE_NAME_DCC_, szTRTFileName, strlen(_TRT_FILE_NAME_DCC_)))
				{
					inRetVal = inNCCC_DCC_RunTRT(pobTran, pobTran->inRunTRTID);
				}
				else if (!memcmp(_TRT_FILE_NAME_ESVC_, szTRTFileName, strlen(_TRT_FILE_NAME_ESVC_)))
				{
					inRetVal = inNCCC_Ticket_RunTRT(pobTran, pobTran->inRunTRTID);
				}
				else if (!memcmp(_TRT_FILE_NAME_TAKA_, szTRTFileName, strlen(_TRT_FILE_NAME_TAKA_)))
				{
					inRetVal = inTAKA_Func_RunTRT(pobTran, pobTran->inRunTRTID);
				}
				else if (!memcmp(_TRT_FILE_NAME_EW_, szTRTFileName, strlen(_TRT_FILE_NAME_EW_)))
				{
					inRetVal = inNCCC_EW_RunTRT(pobTran, pobTran->inRunTRTID);
				}
                                else if (!memcmp(_TRT_FILE_NAME_TRUST_, szTRTFileName, strlen(_TRT_FILE_NAME_TRUST_)))
				{
					inRetVal = inNCCC_Trust_RunTRT(pobTran, pobTran->inRunTRTID);
				}
				else
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "No TRT對應,TRT:%s", szTRTFileName);
					}
                                        snprintf(pobTran->szUnpredictErrorLogMsg, sizeof(pobTran->szUnpredictErrorLogMsg), "No TRT對應,TRT:%s", szTRTFileName);
                                        
				}
			}
		}
	}
	
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

		/* 要先回傳再顯示錯誤訊息 */
		inFunc_Display_Error(pobTran);
	}
	
	if (pobTran->inRunTRTID == _TRT_SETTLE_ ||
	    pobTran->inRunTRTID == _TRT_TICKET_ECC_SETTLE_)
	{
                /* ECR單一host結帳回傳 */
                if (pobTran->uszECRBit == VS_TRUE)
                {
                        if (pobTran->uszDelaySendBit)
                        {
                                inFLOW_RunFunction(pobTran, _NCCC_FUNCTION_SEND_ECR_AFTER_PRINT_RECEIPT_);
                        }
                        else
                        {
                                inFLOW_RunFunction(pobTran, _FUNCTION_ECR_SEND_TRANSACTION_RESULT_);
                        }
                }
                
		/* 如果結帳失敗，就不跑下載 */
		/* 分ISO和FTP，若是ISO，則有結帳失敗就不下，
		   FTP則是不論結帳是否成功，一律執行 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTMSDownloadMode(szTemplate);
		if (memcmp(szTemplate, _TMS_DLMODE_FTPS_, strlen(_TMS_DLMODE_FTPS_)) == 0)
		{
			if (inFunc_All_Host_Settle_Check(pobTran, VS_FALSE) == VS_SUCCESS)
			{
				inFLOW_RunFunction(pobTran, _FUNCTION_TMS_SCHEDULE_INQUIRE_);
				inFLOW_RunFunction(pobTran, _FUNCTION_TMS_DOWNLOAD_SETTLE_);
				/* 提示人工安全認證畫面 */
				inFLOW_RunFunction(pobTran, _NCCC_FUNCTION_DISPLAY_PLEASE_LOGON_FIRST_);
				inFLOW_RunFunction(pobTran, _FUNCTION_TMS_REBOOT_);
			}
		}
		else
		{
			/* ISO維持原流程 */
			if (!memcmp(_TRT_FILE_NAME_CREDIT_, szTRTFileName, strlen(_TRT_FILE_NAME_CREDIT_)))
			{
				/* 結帳失敗，不下載 */
				if (inRetVal != VS_SUCCESS)
				{

				}
				else
				{
					/* ISO不用詢問 */
					inNCCCTMS_PRINT_ScheduleMessage_Flow(pobTran);
					inFLOW_RunFunction(pobTran, _FUNCTION_TMS_DOWNLOAD_SETTLE_);
					inFLOW_RunFunction(pobTran, _FUNCTION_TMS_REBOOT_);
				}
			}
		}
	}

	/* 票證交易不用去偵測感應卡片 */
	if (!memcmp(szTRTFileName, _TRT_FILE_NAME_ESVC_, strlen(_TRT_FILE_NAME_ESVC_)))
	{

	}
	else
	{
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
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inEVENT_Run_Normal_TRT() END !");
		inLogPrintf(AT, "----------------------------------------");
	}

	return (VS_SUCCESS);
}
