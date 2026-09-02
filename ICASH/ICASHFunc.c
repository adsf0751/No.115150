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
#include "../SOURCE/FUNCTION/ICASHDT.h"
#include "../SOURCE/FUNCTION/TDT.h"
#include "../SOURCE/FUNCTION/File.h"
#include "../SOURCE/FUNCTION/Utility.h"
#include "../SOURCE/EVENT/Flow.h"
#include "../SOURCE/EVENT/MenuMsg.h"
#include "../CTLS/CTLS.h"
#include "../NCCC/NCCCTicketSrc.h"
#include "../NCCC/NCCCTicketIso.h"
#include "../NCCC/NCCCsrc.h"
#include "icash2api.h"
#include "ICASHFunc.h"

extern  int		ginDebug;	/* Debug使用 extern */
extern	int		ginISODebug;
extern	int		ginEngineerDebug;
extern	int		ginAPVersionType;
extern	unsigned long	gulDemoTicketPoint;		/* DEMO用 */

/*
Function        :inICASH_DEMOflow
Date&Time       :2019/10/2 下午 3:41
Describe        :教育訓練版需要 先比照悠遊卡來跑
*/
int inICASH_DEMOflow(TRANSACTION_OBJECT *pobTran)
{
        int     inRetVal = VS_ERROR;
	char	szDebugMsg[100 + 1] = {0};
	char	szDemoMode[2 + 1] = {0};
	char	szTxnType[20 + 1] = {0};

	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) != 0)
		return (VS_SUCCESS);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inICASH_DEMOflow() START !");
	}
	
	inDISP_ClearAll();
	inCTLS_Set_LED(_CTLS_LIGHT_YELLOW_);
	
	inDISP_ChineseFont("交易進行中", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
	inDISP_ChineseFont("請勿移動票卡", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
	
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
        {
                memset(pobTran->srTRec.szUID, 0x00, sizeof(pobTran->srTRec.szUID));
                memcpy(&pobTran->srTRec.szUID[0], "7411222233334444", 16);

                pobTran->srTRec.lnCardRemainAmount = gulDemoTicketPoint;
                pobTran->srTRec.lnFinalBeforeAmt = gulDemoTicketPoint;
                pobTran->srTRec.lnTopUpAmount = 500;			/* 自動加值金額(元/次) */

                if (pobTran->srTRec.inCode == _TICKET_ICASH_DEDUCT_)
                {
                        if ((pobTran->srTRec.lnFinalBeforeAmt - pobTran->srTRec.lnTxnAmount) >= 0)
                        {
                                pobTran->srTRec.lnFinalAfterAmt = pobTran->srTRec.lnFinalBeforeAmt - pobTran->srTRec.lnTxnAmount;
                                gulDemoTicketPoint = pobTran->srTRec.lnFinalAfterAmt;
				inRetVal = VS_SUCCESS;
                        }
                        else
                        {
                                /* 交易開關 */
				memset(szTxnType, 0x00, sizeof(szTxnType));
				inGetTicket_HostTransFunc(szTxnType);

				if (szTxnType[3] == 'Y' && pobTran->srTRec.lnTxnAmount >= 500)
				{
					inRetVal = VS_SUCCESS;
                                        /* 先取得自動加值金額 */
                                        pobTran->srTRec.lnTotalTopUpAmount = 0;

                                        if (inNCCC_Ticket_Top_Up_Amount_Check(pobTran) != VS_SUCCESS)
                                        {
                                                /* 自動加值金額過大 */
						inNCCC_Ticket_Set_CTLS_Light(&pobTran->inCTLSLightStatus, _CTLS_LIGHT_RED_);
						inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
						inDISP_ChineseFont("餘額不足", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);  
						inDISP_BEEP(3, 1000);

						if (ginDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
							sprintf(szDebugMsg, "inNCCC_Ticket_ICASH_Query_Flow()_自動加值金額過大");
							inLogPrintf(AT, szDebugMsg);
						}
						inRetVal = VS_ERROR;
                                	}

					if (inRetVal == VS_SUCCESS)
					{
						/* 餘額不足，要自動加值 */
						inNCCC_Ticket_Set_CTLS_Light(&pobTran->inCTLSLightStatus, _CTLS_LIGHT_RED_);
						inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
						inDISP_ChineseFont("餘額不足", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);  
						inDISP_ChineseFont("進行自動加值", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_); 

						pobTran->inTransactionCode = _TICKET_ICASH_AUTO_TOP_UP_;
						pobTran->srTRec.inCode = _TICKET_ICASH_AUTO_TOP_UP_;

						gulDemoTicketPoint += pobTran->srTRec.lnTotalTopUpAmount;
						/* 自動加值成功 */
						pobTran->uszAutoTopUpSuccessBit = VS_TRUE;
					}
					

					if (inRetVal == VS_SUCCESS)
					{
						if (inFLOW_RunFunction(pobTran, _TICKET_UPDATE_ACCUM_) != VS_SUCCESS)
						{
							inRetVal = VS_ERROR;
						}
					}

					if (inRetVal == VS_SUCCESS)
					{
						if (inFLOW_RunFunction(pobTran, _TICKET_UPDATE_BATCH_) != VS_SUCCESS)
						{
							inRetVal = VS_ERROR;
						}
					}

					if (inRetVal == VS_SUCCESS)
					{
						if (inFLOW_RunFunction(pobTran, _TICKET_UPDATE_INV_) != VS_SUCCESS)
						{
							inRetVal = VS_ERROR;
						}
					}

					if (inRetVal == VS_SUCCESS)
					{
						pobTran->inTransactionCode = _TICKET_ICASH_DEDUCT_;
						pobTran->srTRec.inCode = _TICKET_ICASH_DEDUCT_;

						pobTran->srTRec.lnFinalAfterAmt = gulDemoTicketPoint - pobTran->srTRec.lnTxnAmount;
						gulDemoTicketPoint = pobTran->srTRec.lnFinalAfterAmt;
						/* 購貨成功 */
						pobTran->uszDeductSuccessBit = VS_TRUE;
					}

					if (inRetVal == VS_SUCCESS)
					{
						if (inFLOW_RunFunction(pobTran, _TICKET_GET_PARM_) != VS_SUCCESS)
						{
							inRetVal = VS_ERROR;
						}
					}
                                }
                                else
                                {
					inNCCC_Ticket_Set_CTLS_Light(&pobTran->inCTLSLightStatus, _CTLS_LIGHT_RED_);
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_ChineseFont("餘額不足", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);  
					inDISP_BEEP(3, 1000);
                        	        inRetVal = VS_ERROR;
                        	}
                        }

                        memset(pobTran->srTRec.szTicketRefundCode, 0x00, sizeof(pobTran->srTRec.szTicketRefundCode));
                        memcpy(&pobTran->srTRec.szTicketRefundCode[0], "222222", 6);
                }
                else if (pobTran->srTRec.inCode == _TICKET_ICASH_REFUND_ || pobTran->srTRec.inCode == _TICKET_ICASH_TOP_UP_)
                {
                        pobTran->srTRec.lnFinalAfterAmt = pobTran->srTRec.lnFinalBeforeAmt + pobTran->srTRec.lnTxnAmount;
                        gulDemoTicketPoint = pobTran->srTRec.lnFinalAfterAmt;
			inRetVal = VS_SUCCESS;
                }
                else if (pobTran->srTRec.inCode == _TICKET_ICASH_VOID_TOP_UP_)
                {
                        if (pobTran->srTRec.lnFinalBeforeAmt - pobTran->srTRec.lnTxnAmount < 0)
                        {
				inNCCC_Ticket_Set_CTLS_Light(&pobTran->inCTLSLightStatus, _CTLS_LIGHT_RED_);
                                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("餘額不足", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);  
				inDISP_BEEP(3, 1000);
                	        inRetVal = VS_ERROR;
                        }
                        else
                        {
                                pobTran->srTRec.lnFinalAfterAmt = pobTran->srTRec.lnFinalBeforeAmt - pobTran->srTRec.lnTxnAmount;
                                gulDemoTicketPoint = pobTran->srTRec.lnFinalAfterAmt;
				inRetVal = VS_SUCCESS;
                        }
                }
		else if (pobTran->srTRec.inCode == _TICKET_ICASH_INQUIRY_)
		{
			inRetVal = VS_SUCCESS;
		}
        }
        else
	{
		/* DEMO不會跑到else */
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "無對應ICASH incode: %d", pobTran->srTRec.inCode);
			inLogPrintf(AT, szDebugMsg);
		}

		inRetVal = VS_ERROR;
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inICASH_DEMOflow() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	if (inRetVal == VS_SUCCESS)
	{
		/* 成功 */
		inNCCC_Ticket_Trans_Success_Beep(pobTran);
		return (VS_SUCCESS);
	}
	else
	{
		return (VS_ERROR);
	}
}

/*
Function        :inICASH_Init_Flow
Date&Time       :2019/11/12 上午 11:14
Describe        :
*/
int inICASH_Init_Flow(TRANSACTION_OBJECT *pobTran)
{
        int     inRetVal = VS_ERROR;
	int	inHostIndex = 0;
	char	szHostIndex[2 + 1] = {0};
	unsigned char	uszAPI_Version[20 + 1];
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inICASH_Init_flow() START !");
	}

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);					/* 先清除螢幕顯示 */
	inDISP_ChineseFont("愛金卡初始化", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
	
        /* Load ICASH HDT */
	pobTran->srTRec.inTicketType = _TICKET_TYPE_ICASH_;
	pobTran->srTRec.inTDTIndex = _TDT_INDEX_02_ICASH_;
        if (inLoadTDTRec(pobTran->srTRec.inTDTIndex) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	icasH2_GetApVersion(uszAPI_Version);
	/* 存API Version到TDT */
	inSetTicket_API_Version((char *)uszAPI_Version);
	inSaveTDTRec(_TDT_INDEX_02_ICASH_);
	
	memset(szHostIndex, 0x00, sizeof(szHostIndex));
	inGetTicket_HostIndex(szHostIndex);
	inHostIndex = atoi(szHostIndex);
	pobTran->srBRec.inHDTIndex = inHostIndex;
	
	
	if (inLoadHDTRec(pobTran->srBRec.inHDTIndex) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	if (inLoadHDPTRec(pobTran->srBRec.inHDTIndex) != VS_SUCCESS)
		return (VS_ERROR);
	
	if (inLoadICASHDTRec(0) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	/* 連線 */
        inRetVal = inFLOW_RunFunction(pobTran, _TICKET_CONNECT_);

        if (inRetVal != VS_SUCCESS)
        {
                inFLOW_RunFunction(pobTran, _TICKET_DISCONNECT_);
		
                return (VS_ERROR);
        }

        inRetVal = inICASH_SetupReader(pobTran);

        inFLOW_RunFunction(pobTran, _TICKET_DISCONNECT_);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inICASH_Init_flow() END !");
		inLogPrintf(AT, "----------------------------------------");
	}

        return (inRetVal);
}

/*
Function        :inICASH_SetupReader
Date&Time       :2019/11/12 下午 4:27
Describe        :
*/
int inICASH_SetupReader(TRANSACTION_OBJECT *pobTran)
{
        int     inRetVal = VS_ERROR;
	char	szICASH_PIN[4 + 1] = {0};

        /*
                1.InitializeAuth : init, SAM Unlock
                2.SetupReader    : Get PIN Code
                3.TermSysAuth    : Use PIN Code Auth
        */

        /* SAM Unlock - 每次開機必跑 */
        if (pobTran->uszESVC_IC_SAM_Unlock_Bit == VS_FALSE)
        {
                inRetVal = inICASH_SAM_Unlock(pobTran);

                if (inRetVal == VS_SUCCESS)
		{
                        pobTran->uszESVC_IC_SAM_Unlock_Bit = VS_TRUE;
		}
                else
		{
                        return (VS_ERROR);
		}
        }

        /* Get PIN - 取得key，LOGON失敗或重新TMS需要重取 */
	memset(szICASH_PIN, 0x00, sizeof(szICASH_PIN));
	inGetTicket_PIN(szICASH_PIN);
        if (memcmp(szICASH_PIN, "0000", 4) != 0)
	{
		pobTran->uszESVC_Get_PIN_Bit = VS_TRUE;
	}

        if (pobTran->uszESVC_Get_PIN_Bit == VS_FALSE)
        {
                inRetVal = inICASH_Get_PIN(pobTran);

                if (inRetVal == VS_SUCCESS)
		{
                        pobTran->uszESVC_Get_PIN_Bit = VS_TRUE;
		}
                else
		{
                        return (VS_ERROR);
		}
        }

        /* Logon - 每次開機必跑 */
        inRetVal = inICASH_Logon(pobTran);

        if (inRetVal != VS_SUCCESS)
        {
                pobTran->uszESVC_Get_PIN_Bit = VS_FALSE;

		inLoadTDTRec(_TDT_INDEX_02_ICASH_);
                /* 失敗要重取PIN */
                inSetTicket_PIN("0000");

                if (inSaveTDTRec(_TDT_INDEX_02_ICASH_) != VS_SUCCESS)
		{
                        return (VS_ERROR);
		}
        }
        else
        {
		/* Logon成功在外面設定 */
        }

        return (inRetVal);
}

/*
Function        :inICASH_SAM_Unlock
Date&Time       :2019/11/12 下午 4:32
Describe        :
*/
int inICASH_SAM_Unlock(TRANSACTION_OBJECT *pobTran)
{
        int		inRetVal = VS_ERROR, inInput_Len = 0;
	char		szSAMSlot[2 + 1] = {0};
	char		szIntegrate_DeviceEnable[2 + 1] = {0};
        char		szDateTime[15 + 1], szTemp[10] = {0};
        unsigned char   uszInput[150] = {0}, uszOut[150] = {0};
	
	
	memset(szSAMSlot, 0x00, sizeof(szSAMSlot));
	inGetTicket_SAM_Slot(szSAMSlot);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inICASH_SAM_Unlock(%s) START !", szSAMSlot);
	}
	
        /* icasH_InitializeAuth() */
        memset(uszInput, 0x00, sizeof(uszInput));
        memset(uszOut, 0x00, sizeof(uszOut));

        if (memcmp(szSAMSlot, "01", 2) == 0)
        {
                /* Slot = 1 */
                uszInput[0] = 0x01;
        }
        else if (memcmp(szSAMSlot, "02", 2) == 0)
        {
                /* Slot = 2 */
                uszInput[0] = 0x02;
        }

        inInput_Len = 1;

	memset(szIntegrate_DeviceEnable, 0x00, sizeof(szIntegrate_DeviceEnable));
	inGetIntegrate_Device(szIntegrate_DeviceEnable);
	if (memcmp(szIntegrate_DeviceEnable, "N", 1) == 0)
	{
                /* Output = 8 + 42 */
		inRetVal = icasH_InitializeAuth(uszInput, uszOut);

                if (inRetVal != 0x0000)
                {
                        memset(szTemp, 0x00, sizeof(szTemp));
                        sprintf(szTemp, "%08X", inRetVal);
                        inICASH_Display_Error(pobTran, szTemp);
			
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "inICASH_SAM_Unlock()_icasH_InitializeAuth_Error[%08X]", inRetVal);
			}
			
                        return (VS_ERROR);
                }
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "V3暫無外接設備");
		}
		inUtility_StoreTraceLog_OneStep("V3暫無外接設備，無法初始化ICASH");
		
		return (VS_ERROR);
	}

        /* 組IC01 */
        inICASH_Init_Packet(pobTran, _INIT_PACKET_, (char *)uszOut, szDateTime);

	/* 連線中 */
        inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
	inDISP_PutGraphic(_CONNECTING_, 0, _COORDINATE_Y_LINE_8_7_);

        /* 收送 */
        inRetVal = inICASH_Logon_FuncBuildAndSendPacket(pobTran, _ICASH_PACKET_MODE_UNLOCK_);

        if (inRetVal != VS_SUCCESS)
	{
	        /* 收送失敗 */
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inICASH_SAM_Unlock()_Comm_Fail");
		}
		inUtility_StoreTraceLog_OneStep("inICASH_SAM_Unlock()_Comm_Fail");
		
                return (VS_ERROR);
        }

        /* API - 112 bytes */
        memset(uszInput, 0x00, sizeof(uszInput));
        memcpy((char *)&uszInput[0], (char *)&pobTran->srTRec.srICASHRec.szSign_Data[0], pobTran->srTRec.srICASHRec.lnSign_Len);
        inInput_Len = pobTran->srTRec.srICASHRec.lnSign_Len;

	memset(szIntegrate_DeviceEnable, 0x00, sizeof(szIntegrate_DeviceEnable));
	inGetIntegrate_Device(szIntegrate_DeviceEnable);
	if (memcmp(szIntegrate_DeviceEnable, "N", 1) == 0)
	{
                inRetVal = icasH_InitializeEnv(uszInput);

                if (inRetVal != 0x0000)
                {
			memset(szTemp, 0x00, sizeof(szTemp));
                        sprintf(szTemp, "%08X", inRetVal);
                        inICASH_Display_Error(pobTran, szTemp);
			
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "inICASH_SAM_Unlock()_icasH_InitializeEnv_Error[%08X]", inRetVal);
			}
			
                        return (VS_ERROR);
                }
        }
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "V3暫無外接設備");
		}
		inUtility_StoreTraceLog_OneStep("V3暫無外接設備，無法初始化ICASH");
		
		return (VS_ERROR);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inICASH_SAM_Unlock() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inICASH_Display_Error
Date&Time       :2019/11/12 下午 5:02
Describe        :
*/
int inICASH_Display_Error(TRANSACTION_OBJECT *pobTran, char *szError)
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

	if (!memcmp(szError, "0106", 4))
        {
                /* 客製化098，0010以上的回應碼都與標準版少1 */
                switch(inCustomer)
                {
                        case 98:
                        case 105:
                                sprintf(szLine2, "%s", "電票餘額不足");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0015");
                                break;
                        default:
                                sprintf(szLine2, "%s", "電票餘額不足");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0016");
                                break;
                }     
        }
        else if (!memcmp(szError, "0110", 4) ||
                 !memcmp(szError, "010F", 4))
        {
                switch(inCustomer)
                {
                        case 98:
                        case 105:
                                sprintf(szLine2, "%s", "電票金額超過上限");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0011");
                                break;
                        default:
                                sprintf(szLine2, "%s", "電票金額超過上限");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0012");
                                break;
                }   
        }
        else if (!memcmp(szError, "010B", 4))
        {
                switch(inCustomer)
                {
                        case 98:
                        case 105:
                                sprintf(szLine2, "%s", "無法比對原交易");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0013");
                                break;
                        default:
                                sprintf(szLine2, "%s", "無法比對原交易");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0014");
                                break;
                }  
        }
        else if (!memcmp(szError, "0346", 4))
        {
                sprintf(szLine2, "%s", "操作錯誤");
                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0004");
        }
        else
        {
                sprintf(szLine2, "%s", "交易失敗");
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
Function        :inICASH_Display_TCPError
Date&Time       :2019/11/19 下午 6:06
Describe        :
*/
int inICASH_Display_TCPError(TRANSACTION_OBJECT *pobTran, char *szError)
{
        int     inCustomer = 0;
        char    szLine1[33] = {0}, szLine2[33] = {0}, szLine3[33] = {0};
	char	szFuncEnable[2 + 1] = {0};
	char    szErrorTemp[5] = {0};
        char	szCustomerIndicator[3 + 1] = {0};
        
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
        inCustomer = atoi(szCustomerIndicator);
        
	memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
	inGetIntegrate_Device(szFuncEnable);
	if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
	{

	}
	else
	{
		inNCCC_Ticket_Set_CTLS_Light(&pobTran->inCTLSLightStatus, _CTLS_LIGHT_RED_);
	}
	
        memset(szLine2, 0x00, sizeof(szLine2));
        memset(szLine3, 0x00, sizeof(szLine3));
        memset(pobTran->szTicket_ErrorCode, 0x00, sizeof(pobTran->szTicket_ErrorCode));
	
	memset(szErrorTemp, 0x00, sizeof(szErrorTemp));
        memcpy(&szErrorTemp[0], &szError[2], 4);

	if (!memcmp(szError, "99", 2))
        {
                if (!memcmp(szErrorTemp, "0001", 4))
                {
                        sprintf(szLine2, "%s", "交易失敗，請重試");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                }
                else if (!memcmp(szErrorTemp, "0002", 4))
                {
                        sprintf(szLine2, "%s", "加值餘額不足");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                }
                else if (!memcmp(szErrorTemp, "0003", 4))
                {
                        sprintf(szLine2, "%s", "票卡已鎖洽愛金卡");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                }
                else if (!memcmp(szErrorTemp, "0010", 4))
                {
                        sprintf(szLine2, "%s", "該特店不開放加值");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                }
                else if (!memcmp(szErrorTemp, "0011", 4))
                {
                        sprintf(szLine2, "%s", "聯名卡無法授權");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                }
                else if (!memcmp(szErrorTemp, "0012", 4))
                {
                        sprintf(szLine2, "%s", "非有效卡");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                }
                else if (!memcmp(szErrorTemp, "0013", 4))
                {
                        sprintf(szLine2, "%s", "交易金額錯誤");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                }
                else if (!memcmp(szErrorTemp, "0014", 4))
                {
                        switch(inCustomer)
                        {
                                case 98:
                                case 105:
                                        sprintf(szLine2, "%s", "自動加值未開啟");
                                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0014");
                                        break;
                                default:
                                        sprintf(szLine2, "%s", "自動加值未開啟");
                                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0015");
                                        break;
                        }     
                }
                else
                {
                        sprintf(szLine2, "%s", "交易異常，請報修");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                }
        }
        else if (!memcmp(szError, "77", 2))
        {
                if (!memcmp(szErrorTemp, "0001", 4))
                {
                        sprintf(szLine2, "%s", "交易失敗，請重試");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                }
                else
                {
                        sprintf(szLine2, "%s", "交易異常，請報修");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                }
        }
        else
        {
                sprintf(szLine2, "%s", "交易異常，請報修");
                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
        }
	
        memset(szLine1, 0x00, sizeof(szLine1));
        sprintf(szLine1, "%s", szError);
	
	DISPLAY_OBJECT	srDispMsgObj;
	memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
	strcpy(srDispMsgObj.szDispPic1Name, "");
	srDispMsgObj.inDispPic1YPosition = 0;
	srDispMsgObj.inMsgType = _ENTER_KEY_MSG_;
	srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
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
Function        :inICASH_Init_Packet
Date&Time       :2019/11/12 下午 5:54
Describe        :
*/
int inICASH_Init_Packet(TRANSACTION_OBJECT *pobTran, int inType, char *szData, char *szDateTime)
{
        int     inLen = 0;
        char    szSend[150] = {0};
	char	szTemplate[50 + 1] = {0};

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inICASH_Init_Packet(Type = %d) START !", inType);
	}
	
        memset(szSend, 0x00, sizeof(szSend));

        /* Comm Type */
        switch(inType)
        {
                case _INIT_PACKET_ :
                        memcpy(&szSend[inLen], "0811", 4);
                        break;
                case _READER_PACKET_ :
                        memcpy(&szSend[inLen], "0931", 4);
                        break;
                case _TERM_PACKET_ :
                        memcpy(&szSend[inLen], "0831", 4);
                        break;
                default :
                        return (VS_ERROR);
        }
        inLen += 4;

        /* Channel Type(特店簡碼 NCCC收單的特約機構代碼) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetICASH_NCCC_Agency_Code(szTemplate);
        memcpy(&szSend[inLen], szTemplate, 3);
        inLen += 3;

        /* Store No(門市代碼 門市對照碼 供愛金卡系統對應使用) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetICASH_Shop_ID(szTemplate);
        memcpy(&szSend[inLen], szTemplate, 8);
        inLen += 8;

        /* POS RegNo(愛金卡端末機編號) */
        memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetICASH_RegID(szTemplate);
        memcpy(&szSend[inLen], szTemplate, 3);
        inLen += 3;

        switch(inType)
        {
                case _INIT_PACKET_ :
                        /* ReturnCode */
                        memcpy(&szSend[inLen], "000000", 6);
                        inLen += 6;

                        /* API Data */
                        memcpy(&szSend[inLen], &szData[8], 42);
                        inLen += 42;
                        break;
                case _READER_PACKET_ :
                        /* POS SeqNo */
                        memcpy(&szSend[inLen], "00000000", 8);
                        inLen += 8;

                        /* CashierNo(收單行代碼) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetICASH_Bank_ID(szTemplate);
                        memcpy(&szSend[inLen], szTemplate, 4);
                        inLen += 4;

                        /* Time YYYYMMDDHHMMSS */
                        memcpy(&szSend[inLen], &szDateTime[0], 14);
                        inLen += 14;

                        /* ReturnCode */
                        memcpy(&szSend[inLen], "000000", 6);
                        inLen += 6;

                        /* API Data */
                        memcpy(&szSend[inLen], &szData[8], 60);
                        inLen += 60;
                        break;
                case _TERM_PACKET_ :
                        /* POS SeqNo */
                        memcpy(&szSend[inLen], "00000000", 8);
                        inLen += 8;

                        /* CashierNo(收單行代碼) */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetICASH_Bank_ID(szTemplate);
                        memcpy(&szSend[inLen], szTemplate, 4);
                        inLen += 4;

                        /* Time YYYYMMDDHHMMSS */
                        memcpy(&szSend[inLen], &szDateTime[0], 14);
                        inLen += 14;

                        /* ReturnCode */
                        memcpy(&szSend[inLen], "000000", 6);
                        inLen += 6;

                        /* API Data */
                        memcpy(&szSend[inLen], &szData[8], 52);
                        inLen += 52;
                        break;
                default :
                        return (VS_ERROR);
        }

        pobTran->srTRec.srICASHRec.lnSign_Len = inLen;
        memset(pobTran->srTRec.srICASHRec.szSign_Data, 0x00, sizeof(pobTran->srTRec.srICASHRec.szSign_Data));
        memcpy(&pobTran->srTRec.srICASHRec.szSign_Data[0], &szSend[0], pobTran->srTRec.srICASHRec.lnSign_Len);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inICASH_Init_Packet(Len = %d) END !", pobTran->srTRec.srICASHRec.lnSign_Len);
		inLogPrintf(AT, "----------------------------------------");
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inICASH_Logon_FuncBuildAndSendPacket
Date&Time       :2019/11/13 下午 2:18
Describe        :
*/
int inICASH_Logon_FuncBuildAndSendPacket(TRANSACTION_OBJECT *pobTran, int inMode)
{
        int     inRetVal = VS_ERROR, inTemp = 0;
        int     inOriTransactionCode = 0, inOriCode = 0, inOriISOTxnCode = 0;

        inOriTransactionCode = pobTran->inTransactionCode;
        inOriCode = pobTran->srTRec.inCode;
        inOriISOTxnCode = pobTran->inISOTxnCode;

        if (inMode == _ICASH_PACKET_MODE_UNLOCK_)
                inTemp = _TICKET_ICASH_UNLOCK_;
	else if (inMode == _ICASH_PACKET_MODE_GET_PIN_)
                inTemp = _TICKET_ICASH_GETPIN_;
	else
                inTemp = _TICKET_ICASH_LOGON_;

	pobTran->inTransactionCode = inTemp;
        pobTran->srTRec.inCode = inTemp;
        pobTran->inISOTxnCode = inTemp;
        pobTran->srTRec.srICASHRec.inStepNum = 1;

	/* RRN */
	inNCCC_Ticket_Func_MakeRefNo(pobTran);

	inRetVal = inNCCC_TICKET_Func_BuildAndSendPacket(pobTran);

        pobTran->inTransactionCode = inOriTransactionCode;
        pobTran->srTRec.inCode = inOriCode;
        pobTran->inISOTxnCode = inOriISOTxnCode;

	return (inRetVal);
}

/*
Function        :inICASH_Get_PIN
Date&Time       :2019/11/14 下午 1:47
Describe        :
*/
int inICASH_Get_PIN(TRANSACTION_OBJECT *pobTran)
{
        int		inRetVal = VS_ERROR, inInput_Len = 0;
        char		szDateTime[15 + 1] = {0}, szTemp[20] = {0};
	char		szIntegrate_DeviceEnable[2 + 1] = {0};
	unsigned char   uszInput[150] = {0}, uszOut[150] = {0};
	RTC_NEXSYS	srRTC = {};			/* Date & Time */

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inICASH_Get_PIN() START !");
	}

        /* icasH_InitializeAuth() */
        memset(szDateTime, 0x00, sizeof(szDateTime));
	memset(&srRTC, 0x00, sizeof(RTC_NEXSYS));
	inFunc_GetSystemDateAndTime(&srRTC);
	sprintf(szDateTime, "20%02d%02d%02d%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay, srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);

        memset(uszInput, 0x00, sizeof(uszInput));
        memcpy((char *)&uszInput[0], (char *)&szDateTime[0], 14);

	/* TID讀卡機設備編號(ASCII 靠左右補0x20) */
        memset(szTemp, 0x00, sizeof(szTemp));
	inGetTerminalID(szTemp);
        inFunc_PAD_ASCII(szTemp, szTemp, ' ', 16, _PADDING_RIGHT_);
        memcpy((char *)&uszInput[14], &szTemp[0], 16);
        
        inInput_Len = 30;
        
        memset(uszOut, 0x00, sizeof(uszOut));

	memset(szIntegrate_DeviceEnable, 0x00, sizeof(szIntegrate_DeviceEnable));
	inGetIntegrate_Device(szIntegrate_DeviceEnable);
	if (memcmp(szIntegrate_DeviceEnable, "N", 1) == 0)
	{
		/* Output = 8 + 60 */
                inRetVal = icasH_SetupReader_1(uszInput, uszOut);
        
                if (inRetVal != 0x0000)
                {
			memset(szTemp, 0x00, sizeof(szTemp));
                        sprintf(szTemp, "%08X", inRetVal);
                        inICASH_Display_Error(pobTran, szTemp);
			
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "inICASH_Get_PIN()_icasH_SetupReader_1_Error[%08X]", inRetVal);
			}
			
                        return (VS_ERROR);
                }
        }
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "V3暫無外接設備");
		}
		inUtility_StoreTraceLog_OneStep("V3暫無外接設備，無法初始化ICASH");
		
		return (VS_ERROR);
	}      

        /* 組IC01 */
        inICASH_Init_Packet(pobTran, _READER_PACKET_, (char *)uszOut, szDateTime);

	/* 連線中 */
        inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
	inDISP_PutGraphic(_CONNECTING_, 0, _COORDINATE_Y_LINE_8_7_);

        /* 收送 */
        inRetVal = inICASH_Logon_FuncBuildAndSendPacket(pobTran, _ICASH_PACKET_MODE_GET_PIN_);

        if (inRetVal != VS_SUCCESS)
	{
		/* 收送失敗 */
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inICASH_Get_PIN()_Comm_Fail");
		}
		inUtility_StoreTraceLog_OneStep("inICASH_Get_PIN()_Comm_Fail");
		
                return (VS_ERROR);
        }

        /* API - 110 bytes */
        memset(uszInput, 0x00, sizeof(uszInput));
        memcpy((char *)&uszInput[0], (char *)&pobTran->srTRec.srICASHRec.szSign_Data[0], pobTran->srTRec.srICASHRec.lnSign_Len);
        inInput_Len = pobTran->srTRec.srICASHRec.lnSign_Len;
        memset(uszOut, 0x00, sizeof(uszOut));

	memset(szIntegrate_DeviceEnable, 0x00, sizeof(szIntegrate_DeviceEnable));
	inGetIntegrate_Device(szIntegrate_DeviceEnable);
	if (memcmp(szIntegrate_DeviceEnable, "N", 1) == 0)
	{
                inRetVal = icasH_SetupReader_2(uszInput, uszOut);
        
                if (inRetVal != 0x0000)
                {
			memset(szTemp, 0x00, sizeof(szTemp));
                        sprintf(szTemp, "%08X", inRetVal);
                        inICASH_Display_Error(pobTran, szTemp);
			
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "inICASH_Get_PIN()_icasH_SetupReader_2_Error[%08X]", inRetVal);
			}
			
                        return (VS_ERROR);
                }
                else
                {
                        memset(szTemp, 0x00, sizeof(szTemp));
                        memcpy(&szTemp[0], (char *)&uszOut[8], 4);
			
			inLoadTDTRec(_TDT_INDEX_02_ICASH_);
                        inSetTicket_PIN(szTemp);
                        if (inSaveTDTRec(_TDT_INDEX_02_ICASH_) < 0)
			{
                                return (VS_ERROR);
			}
                }
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "V3暫無外接設備");
		}
		inUtility_StoreTraceLog_OneStep("V3暫無外接設備，無法初始化ICASH");
		
		return (VS_ERROR);
	}
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inICASH_Get_PIN(%s) END !", szTemp);
		inLogPrintf(AT, "----------------------------------------");
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inICASH_Logon
Date&Time       :2019/11/14 下午 4:06
Describe        :
*/
int inICASH_Logon(TRANSACTION_OBJECT *pobTran)
{
        int		inRetVal = VS_ERROR, inInput_Len = 0;
        char		szDateTime[15 + 1] = {0}, szTemp[20] = {0};
	char		szIntegrate_DeviceEnable[2 + 1] = {0};
	unsigned char   uszInput[150] = {0}, uszOut[150] = {0};
	RTC_NEXSYS	srRTC = {};			/* Date & Time */

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inICASH_Logon() START !");
	}

	/* icasH_InitializeAuth() */
        memset(szDateTime, 0x00, sizeof(szDateTime));
	memset(&srRTC, 0x00, sizeof(RTC_NEXSYS));
	inFunc_GetSystemDateAndTime(&srRTC);
	sprintf(szDateTime, "20%02d%02d%02d%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay, srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);

        memset(uszInput, 0x00, sizeof(uszInput));
        memcpy((char *)&uszInput[0], (char *)&szDateTime[0], 14);
	memset(szTemp, 0x00, sizeof(szTemp));
	inGetTicket_PIN(szTemp);
        memcpy((char *)&uszInput[14], szTemp, 4);

	/* TID讀卡機設備編號(ASCII 靠左右補0x20) */
        memset(szTemp, 0x00, sizeof(szTemp));
	inGetTerminalID(szTemp);
        inFunc_PAD_ASCII(szTemp, szTemp, ' ', 16, _PADDING_RIGHT_);
        memcpy((char *)&uszInput[18], &szTemp[0], 16);
        
        inInput_Len = 34;
	  
        memset(uszOut, 0x00, sizeof(uszOut));

	memset(szIntegrate_DeviceEnable, 0x00, sizeof(szIntegrate_DeviceEnable));
	inGetIntegrate_Device(szIntegrate_DeviceEnable);
	if (memcmp(szIntegrate_DeviceEnable, "N", 1) == 0)
	{
		/* Output = 8 + 52 */
                inRetVal = icasH_TermSysAuth_1(uszInput, uszOut);
        
                if (inRetVal != 0x0000)
                {
			memset(szTemp, 0x00, sizeof(szTemp));
                        sprintf(szTemp, "%08X", inRetVal);
                        inICASH_Display_Error(pobTran, szTemp);
			
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "inICASH_Logon()icasH_TermSysAuth_1[%08X]", inRetVal);
			}
			
                        return (VS_ERROR);
                }
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "V3暫無外接設備");
		}
		inUtility_StoreTraceLog_OneStep("V3暫無外接設備，無法初始化ICASH");

		return (VS_ERROR);
	}
        
        /* 組IC01 */
        inICASH_Init_Packet(pobTran, _TERM_PACKET_, (char *)uszOut, szDateTime);

	/* 連線中 */
        inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
	inDISP_PutGraphic(_CONNECTING_, 0, _COORDINATE_Y_LINE_8_7_);
	
	/* 收送 */
        inRetVal = inICASH_Logon_FuncBuildAndSendPacket(pobTran, _ICASH_PACKET_MODE_OTHER_);

	if (inRetVal != VS_SUCCESS)
	{
		/* 收送失敗 */
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inICASH_Logon()_Comm_Fail");
		}
		inUtility_StoreTraceLog_OneStep("inICASH_Logon()_Comm_Fail");
		
                return (VS_ERROR);
        }

        /* API - 74 bytes */
        memset(uszInput, 0x00, sizeof(uszInput));
        memcpy((char *)&uszInput[0], (char *)&pobTran->srTRec.srICASHRec.szSign_Data[0], pobTran->srTRec.srICASHRec.lnSign_Len);
        inInput_Len = pobTran->srTRec.srICASHRec.lnSign_Len;
        memset(uszOut, 0x00, sizeof(uszOut));

	memset(szIntegrate_DeviceEnable, 0x00, sizeof(szIntegrate_DeviceEnable));
	inGetIntegrate_Device(szIntegrate_DeviceEnable);
	if (memcmp(szIntegrate_DeviceEnable, "N", 1) == 0)
	{
                inRetVal = icasH_TermSysAuth_2(uszInput, uszOut);
        
                if (inRetVal != 0x0000)
                {
			memset(szTemp, 0x00, sizeof(szTemp));
                        sprintf(szTemp, "%08X", inRetVal);
                        inICASH_Display_Error(pobTran, szTemp);
			
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "inICASH_Logon()icasH_TermSysAuth_2[%08X]", inRetVal);
			}
			
                        return (VS_ERROR);
                }
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "V3暫無外接設備");
		}
		inUtility_StoreTraceLog_OneStep("V3暫無外接設備，無法初始化ICASH");
		
		return (VS_ERROR);
	}
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inICASH_Logon() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inICASH_Fast_Tap
Date&Time       :2019/11/18 下午 5:23
Describe        :
*/
int inICASH_Fast_Tap(TRANSACTION_OBJECT *pobTran)
{
        int     inRetVal = VS_ERROR;
        char    szTemp[20] = {0};
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inICASH_Fast_Tap_Slave() START !");
		inLogPrintf(AT, "UID: %08X", pobTran->srTRec.szAPUID);
	}

	/* 被IPASS的快詢影響咬住不能下APDU Command 把感應器天線OFF再ON */
	inCTLS_Power_Off();
	inCTLS_Power_On();
	
        /* ECC開開關關，所以這裡重新POLL，不然icasH2_SeCheckService()會失敗 */
	inCTLS_ReActive_TypeACard();
        /* 填入7個byte */
        inRetVal = icasH2_SeCheckService((unsigned char *)pobTran->srTRec.szAPUID);

        if (inRetVal != 0x0000)
        {
                /* 修正愛金卡快速詢卡失敗時，畫面會提示問題 by Tusin - 2018/8/28 上午 10:10:17 */
                memset(szTemp, 0x00, sizeof(szTemp));
                sprintf(szTemp, "%08X", inRetVal);
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inICASH_Fast_Tap_Slave(%s)_Error", szTemp);
		}
		
		inRetVal = VS_ERROR;
        }

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inICASH_Fast_Tap_Slave() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
        return (inRetVal);
}

/*
Function        :inICASH_Get_CardAmt
Date&Time       :2019/11/19 上午 11:24
Describe        :
*/
int inICASH_Get_CardAmt(TRANSACTION_OBJECT *pobTran)
{	
	int		inRetVal = VS_ERROR;
	char		szDemoMode[2 + 1] = {0};
	char		szIntegrateDevice[2 + 1] = {0};
	char		szTemp[50 + 1] = {0};
	char		szDebugMsg[100 + 1] = {0};
	unsigned char   uszInput[300] = {0}, uszOut[300] = {0};

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inICASH_Get_CardAmt() START !");
	}

	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		return (VS_SUCCESS);
	}
	
	/* UI */
	memset(szIntegrateDevice, 0x00, sizeof(szIntegrateDevice));
	inGetIntegrate_Device(szIntegrateDevice);
	if (memcmp(szIntegrateDevice, "N", 1) == 0)
	{
		/* 燈號會蓋到銀行logo 乾脆清掉 */
		inDISP_Clear_Line(_LINE_8_1_, _LINE_8_2_);
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);

		if (pobTran->srTRec.inCode == _TICKET_ICASH_AUTO_TOP_UP_)
		{
			inNCCC_Ticket_Set_Msg(pobTran->srTRec.inTicketType, &pobTran->inTicketMsgStatus, _TICKET_MSG_ICASH_AUTO_TOP_UP_);
		}
		else
		{
			inNCCC_Ticket_Set_Msg(pobTran->srTRec.inTicketType, &pobTran->inTicketMsgStatus, _TICKET_MSG_ICASH_PUT_CARD_IN_AREA_);
		}

		inNCCC_Ticket_Set_CTLS_Light(&pobTran->inCTLSLightStatus, _CTLS_LIGHT_YELLOW_);
		inNCCC_Ticket_Set_Msg(pobTran->srTRec.inTicketType, &pobTran->inTicketMsgStatus, _TICKET_MSG_ICASH_DO_NOT_MOVE_CARD_);
	}
	else
	{
		/* V3 尚未有週邊裝置 */
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "V3沒有週邊裝置");
		}
		inUtility_StoreTraceLog_OneStep("V3沒有週邊裝置");

		return (VS_ERROR);
	}
        
        /* 先取卡片資料 */
        memset(uszInput, 0x00, sizeof(uszInput));
        memset(uszOut, 0x00, sizeof(uszOut));

        memcpy((char *)&uszInput[0], &pobTran->srTRec.srICASHRec.szDate[0], 8);
        memcpy((char *)&uszInput[8], &pobTran->srTRec.srICASHRec.szTime[0], 6);

	memset(szIntegrateDevice, 0x00, sizeof(szIntegrateDevice));
	inGetIntegrate_Device(szIntegrateDevice);
	if (memcmp(szIntegrateDevice, "N", 1) == 0)
	{
                inRetVal = icasH2_SeCheckService((unsigned char*)pobTran->srTRec.szAPUID);
                inRetVal = icasH2_TradeReferCard(uszInput, uszOut);

                if (inRetVal == 0x0000)
                {
                        memset(pobTran->srTRec.srICASHRec.szCardInfo, 0x00, sizeof(pobTran->srTRec.srICASHRec.szCardInfo));
                        memcpy(&pobTran->srTRec.srICASHRec.szCardInfo[0], (char *)&uszOut[0], 67);

                        /* UID */
                        memset(pobTran->srTRec.szUID, 0x00, sizeof(pobTran->srTRec.szUID));
                        memcpy(&pobTran->srTRec.szUID[0], (char *)&uszOut[9], 16);

                        memset(szTemp, 0x00, sizeof(szTemp));
                        memcpy(&szTemp[0], &pobTran->srTRec.szUID[2], 2);

                        if (!memcmp(szTemp, "17", 2))
                                pobTran->srTRec.srICASHRec.uszCB_CardBit = VS_TRUE;
                        else
                                pobTran->srTRec.srICASHRec.uszCB_CardBit = VS_FALSE;

                        /* 餘額 */
                        memset(szTemp, 0x00, sizeof(szTemp));
                        memcpy(&szTemp[0], (char *)&uszOut[25], 1);

                        if (szTemp[0] == '-')
                        {
                                memset(szTemp, 0x00, sizeof(szTemp));
                                memcpy(&szTemp[0], (char *)&uszOut[26], 7);
                                pobTran->srTRec.lnCardRemainAmount = 0 - atol(szTemp);
                        }
                        else
                        {
                                memset(szTemp, 0x00, sizeof(szTemp));
                                memcpy(&szTemp[0], (char *)&uszOut[25], 8);
                                pobTran->srTRec.lnCardRemainAmount = atol(szTemp);
                        }

                        /* 狀態 */
                        memset(szTemp, 0x00, sizeof(szTemp));
                        memcpy(&szTemp[0], (char *)&uszOut[42], 1);

                        if (szTemp[0] != 0x01)
                        {
                                memset(szTemp, 0x00, sizeof(szTemp));
                                sprintf(szTemp, "%s", "990012");
                                inICASH_Display_TCPError(pobTran, szTemp);
				
                	        return (VS_ERROR);
                        }
			
			if (ginDebug == VS_TRUE)
			{
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
			}
			
			if (ginISODebug == VS_TRUE)
			{
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
			}
                }
                else
                {
                        memset(szTemp, 0x00, sizeof(szTemp));
                        sprintf(szTemp, "%08X", inRetVal);
                        inICASH_Display_Error(pobTran, szTemp);

        	        return (VS_ERROR);
                }
        }
        else
        {
                /* V3 尚未有週邊裝置 */
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "V3沒有週邊裝置");
		}
		inUtility_StoreTraceLog_OneStep("V3沒有週邊裝置");

		return (VS_ERROR);
        }
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inICASH_Get_CardAmt() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
		
        return (VS_SUCCESS);
}

/*
Function        :inICASH_AutoAdd_Flow
Date&Time       :2019/11/25 下午 2:23
Describe        :
*/
int inICASH_AutoAdd_Flow(TRANSACTION_OBJECT *pobTran)
{
        char	szTxnType[20 + 1] = {0};
	char	szDemoMode[2 + 1] = {0};
        char	szCustomerIndicator[3 + 1] = {0};
	
	/* 教育訓練模式 */
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		return (VS_SUCCESS);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inICASH_AutoAdd_Flow() START !");
	}
        
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	/* 1.檢查是否餘額不足 */
	if ((pobTran->srTRec.lnCardRemainAmount < 0) ||
	    ((pobTran->srTRec.lnCardRemainAmount - pobTran->srTRec.lnTxnAmount) < 0))
	{
		if (pobTran->srTRec.srICASHRec.uszCB_CardBit == VS_TRUE)
		{
			/* 交易開關 */
			memset(szTxnType, 0x00, sizeof(szTxnType));
			inGetTicket_HostTransFunc(szTxnType);

			if (szTxnType[3] == 0x59)
			{
				/* 先取得自動加值金額 */
				pobTran->srTRec.lnTotalTopUpAmount = 0;

				if (inICASH_Top_Up_Amount_Check(pobTran) != VS_SUCCESS)
				{
					/* 自動加值金額過大 */
					inNCCC_Ticket_Set_CTLS_Light(&pobTran->inCTLSLightStatus, _CTLS_LIGHT_RED_);
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_ChineseFont("餘額不足", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);  
					inDISP_BEEP(3, 1000);

                                        if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)     ||
                                            !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_))  
                                        {
                                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0015");
                                        }
                                        else
                                        {
                                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0016");
                                        }

					return (VS_ERROR);
				}
			}
			else
			{
				/* 負值不可交易 */
				inNCCC_Ticket_Set_CTLS_Light(&pobTran->inCTLSLightStatus, _CTLS_LIGHT_RED_);
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("餘額不足", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);  
				inDISP_BEEP(3, 1000);

                                if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)     ||
                                    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_))  
                                {
                                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0015");
                                }
                                else
                                {
                                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0016");
                                }

				return (VS_ERROR);
			}
		}
		else
		{
			/* 負值不可交易 */
			inNCCC_Ticket_Set_CTLS_Light(&pobTran->inCTLSLightStatus, _CTLS_LIGHT_RED_);
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_ChineseFont("餘額不足", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);  
			inDISP_BEEP(3, 1000);

			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)     ||
                            !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_))  
                        {
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0015");
                        }
                        else
                        {
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0016");
                        }

			return (VS_ERROR);
		}
	}
	else
	{
		pobTran->srTRec.lnTotalTopUpAmount = 0;
		return (VS_SUCCESS);
	}

	inNCCC_Ticket_Set_Msg(pobTran->srTRec.inTicketType, &pobTran->inTicketMsgStatus, _TICKET_MSG_ICASH_AUTO_TOP_UP_);

	pobTran->inTransactionCode = _TICKET_ICASH_AUTO_TOP_UP_;
	pobTran->srTRec.inCode = _TICKET_ICASH_AUTO_TOP_UP_;

	if (inFLOW_RunFunction(pobTran, _TICKET_ICASH_TAPCARD_FIRST_) != VS_SUCCESS)
		return (VS_ERROR);

	if (inFLOW_RunFunction(pobTran, _TICKET_CONNECT_) != VS_SUCCESS)
		return (VS_ERROR);

	if (inFLOW_RunFunction(pobTran, _TICKET_FUNCTION_BUILD_AND_SEND_PACKET_) != VS_SUCCESS)
		return (VS_ERROR);

	if (inFLOW_RunFunction(pobTran, _TICKET_ICASH_TAPCARD_SECOND_) != VS_SUCCESS)
		return (VS_ERROR);

	inFLOW_RunFunction(pobTran, _TICKET_ICASH_BUILD_SEND_RECV_PACKET_COMFIRM_);

	if (inFLOW_RunFunction(pobTran, _TICKET_FUNCTION_BUILD_AND_SEND_PACKET_) != VS_SUCCESS)
		return (VS_ERROR);

	/* 失敗要中斷 */
	if (pobTran->srTRec.srICASHRec.uszAPI_FailBit == VS_TRUE)    
		return (VS_ERROR);

	if (inFLOW_RunFunction(pobTran, _TICKET_UPDATE_ACCUM_) != VS_SUCCESS)
		return (VS_ERROR);

	if (inFLOW_RunFunction(pobTran, _TICKET_UPDATE_BATCH_) != VS_SUCCESS)
		return (VS_ERROR);

	if (inFLOW_RunFunction(pobTran, _TICKET_UPDATE_INV_) != VS_SUCCESS)
		return (VS_ERROR);

	if (inFLOW_RunFunction(pobTran, _TICKET_GET_PARM_) != VS_SUCCESS)
		return (VS_ERROR);

	if (inFLOW_RunFunction(pobTran, _TICKET_ICASH_GET_CARD_AMT_) != VS_SUCCESS)
		return (VS_ERROR);
	
	/* 取得金額，避免加值成功購貨失敗回傳金額錯誤 by Russell 2020/8/4 下午 4:32 */
	inFLOW_RunFunction(pobTran, _TICKET_GET_DAVTI_PARM_);
	
	/* 自動加值成功 */
	pobTran->uszAutoTopUpSuccessBit = VS_TRUE;

	pobTran->inTransactionCode = _TICKET_ICASH_DEDUCT_;
        pobTran->srTRec.inCode = _TICKET_ICASH_DEDUCT_;

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inICASH_AutoAdd_Flow() END !");
		inLogPrintf(AT, "----------------------------------------");
	}

        return (VS_SUCCESS);
}

/*
Function        :inICASH_Top_Up_Amount_Check
Date&Time       :2019/11/25 下午 6:48
Describe        :檢核是否可以自動加值
*/
int inICASH_Top_Up_Amount_Check(TRANSACTION_OBJECT *pobTran)
{
        long    lnAddAmt = 0;
        long    lnAmount = 0, lnSubAmount = 0, lnDeductAmount = 0;

	/* icash固定500 */
	pobTran->srTRec.lnTopUpAmount = 500;
	
        lnAmount = pobTran->srTRec.lnCardRemainAmount;		/* 餘額 */
        lnSubAmount = pobTran->srTRec.lnTopUpAmount;		/* 自動加值金額 */
        lnDeductAmount = pobTran->srTRec.lnTxnAmount;		/* 扣款金額 */

        while(1)
        {
                lnAmount += lnSubAmount;
                lnAddAmt += lnSubAmount;
		
		/* 防呆 */
		/* 卡內金額不得高於10000 */
                if (lnAmount > 10000)
		{
                        lnAmount = 10000;
			lnAddAmt = 10000 - pobTran->srTRec.lnCardRemainAmount;

			if (lnAmount < lnDeductAmount)
			{
				/* 交易超過一萬 */
				return (VS_ERROR);
			}
		}

                if (lnAmount >= lnDeductAmount)
                {
                        if (lnAddAmt > 3000)
                        {
                                /* 自動加值最多3000 */
                                return (VS_ERROR);
                        }
                        else
                        {
                                pobTran->srTRec.lnTotalTopUpAmount = lnAddAmt;
                                return (VS_SUCCESS);
                        }
                }
        }
}

/*
Function        :inICASH_First_Tap
Date&Time       :2019/11/27 下午 2:06
Describe        :
*/
int inICASH_First_Tap(TRANSACTION_OBJECT *pobTran)
{
        int		inRetVal, inInput_Len;
        char		szTemp[50];
	char		szDemoMode[2 + 1] = {0};
	char		szIntegrate_Device_Enable[2 + 1] = {0};
        unsigned char   uszInput[300], uszOut[300];
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inICASH_First_Tap() START !");
	}
	
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	
	/* 教育訓練模式 */
	if (memcmp(szDemoMode, "Y", 1) == 0)
	{
                return (VS_SUCCESS);
	}
	/* 查餘額好像本來就不會跑First Tap，不過還是先加 */
	else if (pobTran->srTRec.inCode == _TICKET_ICASH_INQUIRY_)
	{
                return (VS_SUCCESS);
	}

        /* 正式交易 */
        memset(uszInput, 0x00, sizeof(uszInput));
        memset(uszOut, 0x00, sizeof(uszOut));

        /* 組Request Command */
        inInput_Len = inICASH_Command_Packet(pobTran, uszInput);

	if (ginISODebug == VS_TRUE)	
	{
		inPRINT_ChineseFont(" ", _PRT_HEIGHT_);	
		
		switch (pobTran->srTRec.inCode)
		{
			case _TICKET_ICASH_INQUIRY_ :
				inPRINT_ChineseFont("[詢卡]", _PRT_HEIGHT_);
				break;
			case _TICKET_ICASH_DEDUCT_ :
				inPRINT_ChineseFont("[購貨]", _PRT_HEIGHT_);
				break;
			case _TICKET_ICASH_REFUND_ :
				inPRINT_ChineseFont("[退貨]", _PRT_HEIGHT_);
				break;
			case _TICKET_ICASH_TOP_UP_ :
				inPRINT_ChineseFont("[手動加值]", _PRT_HEIGHT_);
				break;
			case _TICKET_ICASH_AUTO_TOP_UP_ :
				inPRINT_ChineseFont("[自動加值]", _PRT_HEIGHT_);
				break;
			case _TICKET_ICASH_VOID_TOP_UP_ :
				inPRINT_ChineseFont("[取消加值]", _PRT_HEIGHT_);
				break;
			default :
				break;
		}
		
		inPRINT_ChineseFont("API_1 Input : ", _PRT_HEIGHT_);
		inPRINT_ChineseFont((char *)uszInput, _PRT_HEIGHT_);
	}

	/* 外接周邊 */
	memset(szIntegrate_Device_Enable, 0x00, sizeof(szIntegrate_Device_Enable));
	inGetIntegrate_Device(szIntegrate_Device_Enable);
	
	if (memcmp(szIntegrate_Device_Enable, "Y", strlen("Y")) == 0)
	{
		/* 目前沒有外接周邊 */
		return (VS_ERROR);	
	}
	else
	{
		switch (pobTran->srTRec.inCode)
		{
			case _TICKET_ICASH_DEDUCT_ :
                                /* Output = 116 */
                                inRetVal = icasH2_TradeSaleAuth(uszInput, uszOut);
                                inInput_Len = 116;
                                break;
			case _TICKET_ICASH_REFUND_ :
                                /* Output = 99 */
                                inRetVal = icasH2_TradeRefundAuth(uszInput, uszOut);
                                inInput_Len = 99;
                                break;
			case _TICKET_ICASH_TOP_UP_ :
                                /* Output = 79 */
                                inRetVal = icasH2_TradeChargeAuth(uszInput, uszOut);
                                inInput_Len = 79;
                                break;
			case _TICKET_ICASH_AUTO_TOP_UP_ :
                                /* Output = 109 */
                                inRetVal = icasH2_TradeAutoLoadAuth(uszInput, uszOut);
                                inInput_Len = 109;
                                break;
			case _TICKET_ICASH_VOID_TOP_UP_ :
                                /* Output = 79 */
                                inRetVal = icasH2_TradeChargeCancelAuth(uszInput, uszOut);
                                inInput_Len = 79;
                                break;
                        default :
                                return (VS_ERROR);
		}
		
		inNCCC_Ticket_Set_CTLS_Light(&pobTran->inCTLSLightStatus, _CTLS_LIGHT_YELLOW_);

		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont("交易進行中", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
		inDISP_ChineseFont("請勿移動票卡", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
		
		if (inRetVal == 0x0000)
		{
			pobTran->srTRec.srICASHRec.inStepNum = 1;
			
			/* 反轉 uszInput <=> uszOut */
			memset(uszInput, 0x00, sizeof(uszInput));
			memcpy((char *)&uszInput[0], (char *)&uszOut[0], inInput_Len);
			
			memset(uszOut, 0x00, sizeof(uszOut));
			pobTran->srTRec.srICASHRec.lnSign_Len = inICASH_Command_Packet2(pobTran, inInput_Len, uszInput, uszOut);
		
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "Command_Packet2:[%s]", uszOut);
			}
			
			/* 簽章 */
			memset(pobTran->srTRec.srICASHRec.szSign_Data, 0x00, sizeof(pobTran->srTRec.srICASHRec.szSign_Data));
			memcpy(&pobTran->srTRec.srICASHRec.szSign_Data[0], (char *)&uszOut[0], pobTran->srTRec.srICASHRec.lnSign_Len);
		}
		else
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			memset(szTemp, 0x00, sizeof(szTemp));
                        sprintf(szTemp, "%08X", inRetVal);
                        inICASH_Display_Error(pobTran, szTemp);
			
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "inICASH_SAM_Unlock()_icasH_InitializeEnv_Error[%08X]", inRetVal);
			}
			
			return (VS_ERROR);
		}
	}
        
        return (VS_SUCCESS);
}

int inICASH_Second_Tap(TRANSACTION_OBJECT *pobTran)
{
        int		inRetVal, inInput_Len, inTempLen;
        char		szTemp[50], szTCP[10];
	char		szDemoMode[2 + 1] = {0};
	char		szIntegrate_Device_Enable[2 + 1] = {0};
        unsigned char   uszInput[500], uszOut[800];
	unsigned char	uszAutoFlag = VS_FALSE, uszDisplay = VS_FALSE;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inICASH_Second_Tap()_START");
	}
	
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "DEMO版不跑inICASH_Second_Tap");
		}
		return (VS_SUCCESS);
	}

	/* 外接周邊 */
	memset(szIntegrate_Device_Enable, 0x00, sizeof(szIntegrate_Device_Enable));
	inGetIntegrate_Device(szIntegrate_Device_Enable);
	
	if (memcmp(szIntegrate_Device_Enable, "Y", strlen("Y")) != 0)
		inNCCC_Ticket_Set_CTLS_Light(&pobTran->inCTLSLightStatus, _CTLS_LIGHT_YELLOW_);
	
	memset(uszInput, 0x00, sizeof(uszInput));
	memset(uszOut, 0x00, sizeof(uszOut));
	
	inTempLen = pobTran->srTRec.srICASHRec.lnSign_Len;
	memcpy(&uszInput[0], &pobTran->srTRec.srICASHRec.szSign_Data[0], inTempLen);
	
	inInput_Len = inICASH_Command_Packet3(pobTran, inTempLen, uszInput, uszOut);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "Command_Packet3:[%s]", uszOut);
	}
	
	/* 主機錯誤碼 */
	memset(szTCP, 0x00, sizeof(szTCP));
	memcpy(&szTCP[0], &pobTran->srTRec.srICASHRec.szSign_Data[44], 6);
	
	if (memcmp(szTCP, "000000", 6))
	{
		/* API要跑完 先不秀 */
		uszDisplay = VS_TRUE;
	}
	
	if (memcmp(szIntegrate_Device_Enable, "Y", strlen("Y")) == 0)
	{
		/* 目前沒外接 */
		return (VS_ERROR);
	}
	else
	{	
		switch (pobTran->srTRec.inCode)
		{
			case _TICKET_ICASH_DEDUCT_ :
                                if (inInput_Len != 206)
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "ICASH_DEDUCT input不足206碼");
					}
					inUtility_StoreTraceLog_OneStep("ICASH_DEDUCT input不足206碼");
                                        return (VS_ERROR);
				}
                                else
                                {
                                        memset(uszInput, 0x00, sizeof(uszInput));
                                        memcpy((char *)&uszInput[0], (char *)&uszOut[0], inInput_Len);

                                        /* Output = 388 */
                                        memset(uszOut, 0x00, sizeof(uszOut));
                                        inRetVal = icasH2_TradeSale(uszInput, uszOut);
                                }
				
				break;
			case _TICKET_ICASH_REFUND_ :
                                if (inInput_Len != 206)
                                {
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "ICASH_REFUND input不足206碼");
					}
					inUtility_StoreTraceLog_OneStep("ICASH_REFUND input不足206碼");
                                        return (VS_ERROR);
				}
                                else
                                {
                                        memset(uszInput, 0x00, sizeof(uszInput));
                                        memcpy((char *)&uszInput[0], (char *)&uszOut[0], inInput_Len);

                                        /* Output = 388 */
                                        memset(uszOut, 0x00, sizeof(uszOut));
                                        inRetVal = icasH2_TradeRefund(uszInput, uszOut);
                                }

				break;
			case _TICKET_ICASH_TOP_UP_ :
                                if (inInput_Len != 166)
                                {
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "ICASH_TOP_UP input不足166碼");
					}
					inUtility_StoreTraceLog_OneStep("ICASH_TOP_UP input不足166碼");
                                        return (VS_ERROR);
				}
                                else
                                {
                                        memset(uszInput, 0x00, sizeof(uszInput));
                                        memcpy((char *)&uszInput[0], (char *)&uszOut[0], inInput_Len);

                                        /* Output = 388 */
                                        memset(uszOut, 0x00, sizeof(uszOut));
                                        inRetVal = icasH2_TradeCharge(uszInput, uszOut);
                                }

                                break;
			case _TICKET_ICASH_AUTO_TOP_UP_ :
                                if (inInput_Len != 196)
                                {
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "AUTO_TOP_UP input不足196碼");
					}
					inUtility_StoreTraceLog_OneStep("AUTO_TOP_UP input不足196碼");
                                        return (VS_ERROR);
				}
                                else
                                {
                                        memset(uszInput, 0x00, sizeof(uszInput));
                                        memcpy((char *)&uszInput[0], (char *)&uszOut[0], inInput_Len);

                                        /* Output = 388 or 720 */
                                        memset(uszOut, 0x00, sizeof(uszOut));
                                        inRetVal = icasH2_TradeAutoLoad(uszInput, uszOut);
                                }

                                break;
			case _TICKET_ICASH_VOID_TOP_UP_ :
                                if (inInput_Len != 166)
                                {
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "VOID_TOP_UP input不足166碼");
					}
					inUtility_StoreTraceLog_OneStep("VOID_TOP_UP input不足166碼");
                                        return (VS_ERROR);
				}
                                else
                                {
                                        memset(uszInput, 0x00, sizeof(uszInput));
                                        memcpy((char *)&uszInput[0], (char *)&uszOut[0], inInput_Len);

                                        /* Output = 388 */
                                        memset(uszOut, 0x00, sizeof(uszOut));
                                        inRetVal = icasH2_TradeChargeCancel(uszInput, uszOut);
                                }

                                break;
			default :
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "無此icash交易別,%d", pobTran->srTRec.inCode);
				}
				inUtility_StoreTraceLog_OneStep("無此icash交易別,%d", pobTran->srTRec.inCode);
				return (VS_ERROR);	
		}
		
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont("交易進行中", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
		inDISP_ChineseFont("請勿移動票卡", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
		pobTran->srTRec.srICASHRec.inStepNum = 3;
		
		if (inRetVal == 0x0000)
		{
			memset(pobTran->srTRec.srICASHRec.szTxLog, 0x00, sizeof(pobTran->srTRec.srICASHRec.szTxLog));
			memset(pobTran->srTRec.srICASHRec.szTxLog_Other, 0x00, sizeof(pobTran->srTRec.srICASHRec.szTxLog_Other));
			
			/* 感應成功 */
			switch (pobTran->srTRec.inCode)
			{
				case _TICKET_ICASH_DEDUCT_ :
				case _TICKET_ICASH_REFUND_ :
				case _TICKET_ICASH_TOP_UP_ :
				case _TICKET_ICASH_AUTO_TOP_UP_ :
					memcpy(&pobTran->srTRec.srICASHRec.szTxLog[0], (char *)&uszOut[41], 347);
					break;
				case _TICKET_ICASH_VOID_TOP_UP_ :
					memcpy(&pobTran->srTRec.srICASHRec.szTxLog[0], (char *)&uszOut[41], 347);
					
					if (uszOut[388] == 'Y')
					{
						memcpy(&pobTran->srTRec.srICASHRec.szTxLog_Other[0], (char *)&uszOut[41], 16);
						memcpy(&pobTran->srTRec.srICASHRec.szTxLog_Other[16], (char *)&uszOut[389], 331);
						
						uszAutoFlag = VS_TRUE;
					}
					
					break;
				default :
					return (VS_ERROR);	
			}
			
			if (pobTran->srTRec.inCode != _TICKET_ICASH_AUTO_TOP_UP_)
				inNCCC_Ticket_Trans_Success_Beep(pobTran);
			
			/* HOST Batch */
			pobTran->srTRec.srICASHRec.uszAPI_FailBit = VS_FALSE;
			inNCCC_Ticket_Insert_Advice_Ticket_Record(pobTran);
			
			if (uszAutoFlag == VS_TRUE)
			{
				pobTran->srTRec.srICASHRec.uszTxLogBit = VS_TRUE;
				pobTran->srTRec.lnCountInvNum ++;
				inNCCC_Ticket_Insert_Advice_Ticket_Record(pobTran);
				pobTran->srTRec.srICASHRec.uszTxLogBit = VS_FALSE;
			}
			
			inRetVal = VS_SUCCESS;
			
			/* 購貨成功 */
			if (pobTran->srTRec.inCode == _TICKET_ICASH_DEDUCT_)
			{
				pobTran->uszDeductSuccessBit = VS_TRUE;
			}
		}
		else
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inNCCC_Ticket_Set_CTLS_Light(&pobTran->inCTLSLightStatus, _CTLS_LIGHT_RED_);
			memset(pobTran->srTRec.srICASHRec.szTxLog, 0x00, sizeof(pobTran->srTRec.srICASHRec.szTxLog));
			
			/* 跟0100電文區分開 */
			if (pobTran->srTRec.uszBlackListBit == VS_TRUE)
			{
				/* output txlog要參考鎖卡的，前面自己補 */
				pobTran->srTRec.srICASHRec.inStepNum = 3;
				memcpy(&pobTran->srTRec.srICASHRec.szTxLog[0], "----------------", 16);
				memcpy(&pobTran->srTRec.srICASHRec.szTxLog[16], (char *)&uszOut[57], 331);
			}
			else
			{
				memcpy(&pobTran->srTRec.srICASHRec.szTxLog[0], (char *)&uszOut[41], 347);
			}
			
			/* Host Batch */
			pobTran->srTRec.srICASHRec.uszAPI_FailBit = VS_TRUE;
			inNCCC_Ticket_Insert_Advice_Ticket_Record(pobTran);
			
			if (uszDisplay == VS_TRUE)
			{
				inICASH_Display_TCPError(pobTran, szTCP);
			}
			else
			{
				memset(szTemp, 0x00, sizeof(szTemp));
				
				if (memcmp(szIntegrate_Device_Enable, "Y", strlen("Y")) == 0)
				{
					/* 目前沒周邊 */
				}
				else 
				{
					sprintf(szTemp, "%08X", inRetVal);
					inICASH_Display_Error(pobTran, szTemp);
				}
			}
			
			inRetVal = VS_SUCCESS;
		}
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inICASH_Second_Tap()_END");
	}
	
	return (inRetVal);
}

/*
Function        :inICASH_Command_Packet
Date&Time       :2019/11/27 下午 2:59
Describe        :
*/
int inICASH_Command_Packet(TRANSACTION_OBJECT *pobTran, unsigned char *szSendBuffer)
{
        int     inLen = 0;
	char	szDateTime[15 + 1], szTemp[20];
	
	if (pobTran->srTRec.inCode == _TICKET_ICASH_DEDUCT_)
	{
		/* 1.Time */
		memset(szDateTime, 0x00, sizeof(szDateTime));
		memcpy(&szDateTime[0], &pobTran->srTRec.srICASHRec.szDate[0], 8);
		memcpy(&szDateTime[8], &pobTran->srTRec.srICASHRec.szTime[0], 6);
		memcpy((char *)&szSendBuffer[inLen], &szDateTime[0], 14);
		inLen += 14;
		
                /* 2.Amount */
                memset(szTemp, 0x00, sizeof(szTemp));
                sprintf(szTemp, "%08ld", pobTran->srTRec.lnTxnAmount);
                memcpy((char *)&szSendBuffer[inLen], &szTemp[0], 8);
                inLen += 8;

                /* 3.同上 */
                memcpy((char *)&szSendBuffer[inLen], &szTemp[3], 5);
                inLen += 5;

                /* 4.其他金額 */
                memcpy((char *)&szSendBuffer[inLen], "000000000000000", 15);
                inLen += 15;
	}
	else if (pobTran->srTRec.inCode == _TICKET_ICASH_REFUND_)
	{
		/* 1.Time */
		memset(szDateTime, 0x00, sizeof(szDateTime));
		memcpy(&szDateTime[0], &pobTran->srTRec.srICASHRec.szDate[0], 8);
		memcpy(&szDateTime[8], &pobTran->srTRec.srICASHRec.szTime[0], 6);
		memcpy((char *)&szSendBuffer[inLen], &szDateTime[0], 14);
		inLen += 14;

                /* 2.Amount */
                memset(szTemp, 0x00, sizeof(szTemp));
                sprintf(szTemp, "%08ld", pobTran->srTRec.lnTxnAmount);
                memcpy((char *)&szSendBuffer[inLen], &szTemp[0], 8);
                inLen += 8;

                /* 3.同上 */
                memcpy((char *)&szSendBuffer[inLen], &szTemp[3], 5);
                inLen += 5;

                /* 4.其他金額 */
                memcpy((char *)&szSendBuffer[inLen], "000000000000000", 15);
                inLen += 15;		
		
	}
	else if (pobTran->srTRec.inCode == _TICKET_ICASH_TOP_UP_)
	{
		/* 1.Time */
		memset(szDateTime, 0x00, sizeof(szDateTime));
		memcpy(&szDateTime[0], &pobTran->srTRec.srICASHRec.szDate[0], 8);
		memcpy(&szDateTime[8], &pobTran->srTRec.srICASHRec.szTime[0], 6);
		memcpy((char *)&szSendBuffer[inLen], &szDateTime[0], 14);
		inLen += 14;

                /* 2.Amount */
                memset(szTemp, 0x00, sizeof(szTemp));
                sprintf(szTemp, "%08ld", pobTran->srTRec.lnTxnAmount);
                memcpy((char *)&szSendBuffer[inLen], &szTemp[0], 8);
                inLen += 8;	
	}
	else if (pobTran->srTRec.inCode == _TICKET_ICASH_VOID_TOP_UP_)
	{
		/* 1.Time */
		memset(szDateTime, 0x00, sizeof(szDateTime));
		memcpy(&szDateTime[0], &pobTran->srTRec.srICASHRec.szDate[0], 8);
		memcpy(&szDateTime[8], &pobTran->srTRec.srICASHRec.szTime[0], 6);
		memcpy((char *)&szSendBuffer[inLen], &szDateTime[0], 14);
		inLen += 14;

                /* 2.Amount */
                memset(szTemp, 0x00, sizeof(szTemp));
                sprintf(szTemp, "%08ld", pobTran->srTRec.lnTxnAmount);
                memcpy((char *)&szSendBuffer[inLen], &szTemp[0], 8);
                inLen += 8;	
	}
	else if (pobTran->srTRec.inCode == _TICKET_ICASH_AUTO_TOP_UP_)
	{
		/* 1.Time */
		memset(szDateTime, 0x00, sizeof(szDateTime));
		memcpy(&szDateTime[0], &pobTran->srTRec.srICASHRec.szDate[0], 8);
		memcpy(&szDateTime[8], &pobTran->srTRec.srICASHRec.szTime[0], 6);
		memcpy((char *)&szSendBuffer[inLen], &szDateTime[0], 14);
		inLen += 14;

                /* 2.Amount */
                memset(szTemp, 0x00, sizeof(szTemp));
                sprintf(szTemp, "%08ld", pobTran->srTRec.lnTxnAmount);
                memcpy((char *)&szSendBuffer[inLen], &szTemp[0], 8);
                inLen += 8;
	}
	
        return (inLen);
}

int inICASH_Command_Packet2(TRANSACTION_OBJECT *pobTran, int inLen, unsigned char *szInputBuffer, unsigned char *szSendBuffer)
{
        int     inCnt = 0, inSubLen = 0;
        char    szDateTime[15 + 1] = {0}, szTemp[20] = {0};
	char	szICASH_NCCC_Agency_Code[3 + 1] = {0};
	char	szICASH_Shop_ID[8 + 1] = {0};
	char	szICASH_RegID[3 + 1] = {0};
	char	szICASH_Bank_ID[4 + 1] = {0};
	
	if (pobTran->srTRec.inCode == _TICKET_ICASH_DEDUCT_)
		memcpy((char *)&szSendBuffer[inCnt], "0731", 4);
	else if (pobTran->srTRec.inCode == _TICKET_ICASH_REFUND_)
		memcpy((char *)&szSendBuffer[inCnt], "0531", 4);
	else if (pobTran->srTRec.inCode == _TICKET_ICASH_TOP_UP_)
		memcpy((char *)&szSendBuffer[inCnt], "0331", 4);
	else if (pobTran->srTRec.inCode == _TICKET_ICASH_VOID_TOP_UP_)
		memcpy((char *)&szSendBuffer[inCnt], "0431", 4);
	else if (pobTran->srTRec.inCode == _TICKET_ICASH_AUTO_TOP_UP_)
		memcpy((char *)&szSendBuffer[inCnt], "0631", 4);
	
	inCnt += 4;

	/* 2.Channel Type */
	memset(szICASH_NCCC_Agency_Code, 0x00, sizeof(szICASH_NCCC_Agency_Code));
	inGetICASH_NCCC_Agency_Code(szICASH_NCCC_Agency_Code);
	memcpy((char *)&szSendBuffer[inCnt], &szICASH_NCCC_Agency_Code[0], 3);
	inCnt += 3;
	
	/* 3.Store No */
	memset(szICASH_Shop_ID, 0x00, sizeof(szICASH_Shop_ID));
	inGetICASH_Shop_ID(szICASH_Shop_ID);
	memcpy((char *)&szSendBuffer[inCnt], &szICASH_Shop_ID[0], 8);
	inCnt += 8;
	
	/* 4.POS RegNo */
	memset(szICASH_RegID, 0x00, sizeof(szICASH_RegID));
	inGetICASH_RegID(szICASH_RegID);
	memcpy((char *)&szSendBuffer[inCnt], &szICASH_RegID[0], 3);
	inCnt += 3;

	/* 5.POS SeqNo */
	/* 調整愛金卡交易序號內容，與調閱編號相同 */
	memset(szTemp, 0x00, sizeof(szTemp));
	sprintf(szTemp, "%08ld", pobTran->srTRec.lnInvNum);
	memcpy((char *)&szSendBuffer[inCnt], &szTemp[0], 8);
	inCnt += 8;
	
	/* 6.CashierNo */
	memset(szICASH_Bank_ID, 0x00, sizeof(szICASH_Bank_ID));
	inGetICASH_Bank_ID(szICASH_Bank_ID);
	memcpy((char *)&szSendBuffer[inCnt], &szICASH_Bank_ID[0], 4);
	inCnt += 4;
	
	/* 7.Time YYYYMMDDHHMMSS */
	memset(szDateTime, 0x00, sizeof(szDateTime));
	memcpy(&szDateTime[0], &pobTran->srTRec.srICASHRec.szDate[0], 8);
	memcpy(&szDateTime[8], &pobTran->srTRec.srICASHRec.szTime[0], 6);
	memcpy((char *)&szSendBuffer[inCnt], &szDateTime[0], 14);
	inCnt += 14;
	
	/* 8.ReturnCode */
	memcpy((char *)&szSendBuffer[inCnt], "000000", 6);
	inCnt += 6;
	
	/* 9.只取API後 inSubLen bytes */
	if (pobTran->srTRec.inCode == _TICKET_ICASH_DEDUCT_)
		inSubLen = 90;
	else if (pobTran->srTRec.inCode == _TICKET_ICASH_REFUND_)
		inSubLen = 90;
	else if (pobTran->srTRec.inCode == _TICKET_ICASH_TOP_UP_)
		inSubLen = 70;
	else if (pobTran->srTRec.inCode == _TICKET_ICASH_VOID_TOP_UP_)
		inSubLen = 70;
	else if (pobTran->srTRec.inCode == _TICKET_ICASH_AUTO_TOP_UP_)
		inSubLen = 100;
	
	memcpy((char *)&szSendBuffer[inCnt], (char *)&szInputBuffer[inLen - inSubLen], inSubLen);
	inCnt += inSubLen;
	
	return (inCnt);
}

int inICASH_Command_Packet3(TRANSACTION_OBJECT *pobTran, int inLen, unsigned char *szInputBuffer, unsigned char *szSendBuffer)
{
        int     inCnt = 0;
        char    szDateTime[15 + 1], szTemp[20];	
	char	szICASH_NCCC_Agency_Code[3 + 1];
	char	szICASH_Shop_ID[8 + 1];
	char	szICASH_RegID[3 + 1];
	char	szICASH_Bank_ID[4 + 1];
	
	/* 電文收到後再加工 */
	/* 1.Time */
	memset(szDateTime, 0x00, sizeof(szDateTime));
	memcpy(&szDateTime[0], &pobTran->srTRec.srICASHRec.szDate[0], 8);
	memcpy(&szDateTime[8], &pobTran->srTRec.srICASHRec.szTime[0], 6);
	memcpy((char *)&szSendBuffer[inCnt], &szDateTime[0], 14);
	inCnt += 14;
	
	/* 2.Channel Type */
	memset(szICASH_NCCC_Agency_Code, 0x00, sizeof(szICASH_NCCC_Agency_Code));
	inGetICASH_NCCC_Agency_Code(szICASH_NCCC_Agency_Code);
	memcpy((char *)&szSendBuffer[inCnt], &szICASH_NCCC_Agency_Code[0], 3);
	inCnt += 3;
	
	/* 3.Store No */
	memset(szICASH_Shop_ID, 0x00, sizeof(szICASH_Shop_ID));
	inGetICASH_Shop_ID(szICASH_Shop_ID);
	memcpy((char *)&szSendBuffer[inCnt], &szICASH_Shop_ID[0], 8);
	inCnt += 8;
	
	/* 4.POS RegNo */
	memset(szICASH_RegID, 0x00, sizeof(szICASH_RegID));
	inGetICASH_RegID(szICASH_RegID);
	memcpy((char *)&szSendBuffer[inCnt], &szICASH_RegID[0], 3);
	inCnt += 3;
	
	/* 5.POS SeqNo */
	/* 調整愛金卡交易序號內容，與調閱編號相同 */
	memset(szTemp, 0x00, sizeof(szTemp));
	sprintf(szTemp, "%06ld", pobTran->srTRec.lnInvNum);
	memcpy((char *)&szSendBuffer[inCnt], &szTemp[0], 6);
	inCnt += 6;
	
	/* 6.CashierNo */
	memset(szICASH_Bank_ID, 0x00, sizeof(szICASH_Bank_ID));
	inGetICASH_Bank_ID(szICASH_Bank_ID);
	memcpy((char *)&szSendBuffer[inCnt], &szICASH_Bank_ID[0], 4);
	inCnt += 4;
	
	/* 7.Amount */
	memset(szTemp, 0x00, sizeof(szTemp));
	
	if (pobTran->srTRec.inCode == _TICKET_ICASH_AUTO_TOP_UP_)
		sprintf(szTemp, "%08ld", pobTran->srTRec.lnTotalTopUpAmount);
	else
		sprintf(szTemp, "%08ld", pobTran->srTRec.lnTxnAmount);
	memcpy((char *)&szSendBuffer[inCnt], &szTemp[0], 8);
	inCnt += 8;
	
	if (pobTran->srTRec.inCode == _TICKET_ICASH_DEDUCT_ ||
	    pobTran->srTRec.inCode == _TICKET_ICASH_REFUND_)
	{
		/* 8.同上 */
		memcpy((char *)&szSendBuffer[inCnt], &szTemp[3], 5);
		inCnt += 5;
		
		/* 9.其他金額 */
		memcpy((char *)&szSendBuffer[inCnt], "000000000000000", 15);
		inCnt += 15;
	}
	
	memcpy((char *)&szSendBuffer[inCnt], (char *)&szInputBuffer[0], inLen);
	inCnt += inLen;
	
	return (inCnt);
}

int inICASH_Func_BuildAndSendPacket_Comfirm(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	char	szDemoMode[2 + 1];
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inICASH_Func_BuildAndSendPacket_Comfirm()_START");
	}

	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		return (VS_SUCCESS);
	}
	
	if (pobTran->srTRec.inCode == _TICKET_ICASH_REFUND_ ||
	    pobTran->srTRec.inCode == _TICKET_ICASH_AUTO_TOP_UP_ ||
	    pobTran->srTRec.inCode == _TICKET_ICASH_TOP_UP_)
	{
		if (pobTran->srTRec.srICASHRec.uszAPI_FailBit == VS_FALSE)
		{
			/* Comfirm使用交易結果 */
			inICASH_Command_Packet_Comfirm(pobTran);
			pobTran->srTRec.srICASHRec.inStepNum = 2;
			
			/* Online Comfirm 不論成功失敗 */
			inFLOW_RunFunction(pobTran, _TICKET_FUNCTION_BUILD_AND_SEND_PACKET_);
		}
	}
	    
	pobTran->srTRec.srICASHRec.inStepNum = 3;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inICASH_Func_BuildAndSendPacket_Comfirm()_END");
	}
	
	return (inRetVal);
}

int inICASH_Command_Packet_Comfirm(TRANSACTION_OBJECT *pobTran)
{
	int	inCnt = 0;
	char	szDateTime[15 + 1], szTemp[400];
	char	szICASH_NCCC_Agency_Code[3 + 1];
	char	szICASH_Shop_ID[8 + 1];
	char	szICASH_RegID[3 + 1];
	char	szICASH_Bank_ID[4 + 1];
	
	memset(szTemp, 0x00, sizeof(szTemp));
	
	/* 1.Comm Type */
	if (pobTran->srTRec.inCode == _TICKET_ICASH_DEDUCT_)
		memcpy(&szTemp[inCnt], "0741", 4);
	else if (pobTran->srTRec.inCode == _TICKET_ICASH_REFUND_)
		memcpy(&szTemp[inCnt], "0541", 4);
	else if (pobTran->srTRec.inCode == _TICKET_ICASH_TOP_UP_)
		memcpy(&szTemp[inCnt], "0341", 4);
	else if (pobTran->srTRec.inCode == _TICKET_ICASH_VOID_TOP_UP_)
		memcpy(&szTemp[inCnt], "0441", 4);
	else if (pobTran->srTRec.inCode == _TICKET_ICASH_AUTO_TOP_UP_)
		memcpy(&szTemp[inCnt], "0641", 4);
	
	inCnt += 4;
	
	/* 2.Channel Type */
	memset(szICASH_NCCC_Agency_Code, 0x00, sizeof(szICASH_NCCC_Agency_Code));
	inGetICASH_NCCC_Agency_Code(szICASH_NCCC_Agency_Code);
	memcpy((char *)&szTemp[inCnt], &szICASH_NCCC_Agency_Code[0], 3);
	inCnt += 3;
	
	/* 3.Store No */
	memset(szICASH_Shop_ID, 0x00, sizeof(szICASH_Shop_ID));
	inGetICASH_Shop_ID(szICASH_Shop_ID);
	memcpy((char *)&szTemp[inCnt], &szICASH_Shop_ID[0], 8);
	inCnt += 8;
	
	/* 4.POS RegNo */
	memset(szICASH_RegID, 0x00, sizeof(szICASH_RegID));
	inGetICASH_RegID(szICASH_RegID);
	memcpy((char *)&szTemp[inCnt], &szICASH_RegID[0], 3);
	inCnt += 3;
	
	/* 5.POS SeqNo */
	memcpy((char *)&szTemp[inCnt], "00000000", 8);
	inCnt += 8;
	
	/* 6.CashierNo */
	memset(szICASH_Bank_ID, 0x00, sizeof(szICASH_Bank_ID));
	inGetICASH_Bank_ID(szICASH_Bank_ID);
	memcpy((char *)&szTemp[inCnt], &szICASH_Bank_ID[0], 4);
	inCnt += 4;
	
	/* 7.Time YYYYMMDDHHMMSS */
	memset(szDateTime, 0x00, sizeof(szDateTime));
	memcpy(&szDateTime[0], &pobTran->srTRec.srICASHRec.szDate[0], 8);
	memcpy(&szDateTime[8], &pobTran->srTRec.srICASHRec.szTime[0], 6);
	memcpy(&szTemp[inCnt], &szDateTime[0], 14);
	inCnt += 14;
	
	/* 8.ReturnCode */
	memcpy(&szTemp[inCnt], "000000", 6);
	inCnt += 6;
	
	/* 9.Txlog - 16 + 331 */
	memcpy(&szTemp[inCnt], &pobTran->srTRec.srICASHRec.szTxLog[0], 347);
	inCnt += 347;
	
	pobTran->srTRec.srICASHRec.lnSign_Len = inCnt;
	memset(pobTran->srTRec.srICASHRec.szSign_Data, 0x00, sizeof(pobTran->srTRec.srICASHRec.szSign_Data));
	memcpy(&pobTran->srTRec.srICASHRec.szSign_Data[0], &szTemp[0], pobTran->srTRec.srICASHRec.lnSign_Len);
	
	return (VS_SUCCESS);
}
