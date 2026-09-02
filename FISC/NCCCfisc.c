#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <ctos_clapi.h>
#include <emv_cl.h>
#include <sqlite3.h>
//#include <smartpay.h>
#include "../SOURCE/INCLUDES/Define_1.h"
#include "../SOURCE/INCLUDES/Transaction.h"
#include "../SOURCE/INCLUDES/TransType.h"
#include "../SOURCE/INCLUDES/AllStruct.h"
#include "../SOURCE/DISPLAY/Display.h"
#include "../SOURCE/DISPLAY/DispMsg.h"
#include "../SOURCE/DISPLAY/DisTouch.h"
#include "../SOURCE/FUNCTION/Function.h"
#include "../SOURCE/FUNCTION/File.h"
#include "../SOURCE/PRINT/Print.h"
#include "../EMVSRC/EMVsrc.h"
#include "../SOURCE/FUNCTION/Card.h"
#include "../SOURCE/COMM/Comm.h"
#include "../SOURCE/EVENT/MenuMsg.h"
#include "../NCCC/NCCCats.h"
#include "../NCCC/NCCCsrc.h"
#include "../NCCC/NCCCtmk.h"
#include "../CREDIT/Creditfunc.h"
#include "../SOURCE/EVENT/Flow.h"
#include "../SOURCE/FUNCTION/Sqlite.h"
#include "../SOURCE/FUNCTION/APDU.h"
#include "../SOURCE/FUNCTION/CDT.h"
#include "../SOURCE/FUNCTION/EDC.h"
#include "../SOURCE/FUNCTION/HDT.h"
#include "../SOURCE/FUNCTION/HDPT.h"
#include "../SOURCE/FUNCTION/ECR.h"
#include "../SOURCE/FUNCTION/KMS.h"
#include "../SOURCE/FUNCTION/CFGT.h"
#include "../SOURCE/FUNCTION/MVT.h"
#include "../SOURCE/FUNCTION/MultiFunc.h"
#include "../SOURCE/FUNCTION/SPAY.h"
#include "../SOURCE/FUNCTION/Utility.h"
#include "../CTLS/CTLS.h"
#include "NCCCfisc.h"

extern  int     ginDebug;		/* Debug使用 extern */
extern	int	ginDisplayDebug;
extern	int	ginMachineType;
extern	char	gszTermVersionID[16 + 1];

/*
Function        :inFISC_SelectAID
Date&Time       :
Describe        :
*/
int inFISC_SelectAID(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_ERROR;
	char		szSPAID[16 + 1] = {0};
	char		szSPAIDHex[8] = {0};
        APDU_COMMAND	srAPDUData;

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inFISC_SelectAID()_START");
        }
	
	if (inLoadSPAYRec(0) < 0)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inLoadSPAYRec ERROR!!");
                        
                return (VS_ERROR);
        }
        
        memset((char *)&srAPDUData, 0x00, sizeof(APDU_COMMAND));
	memset(szSPAID, 0x00, sizeof(szSPAID));
	inGetSPAID(szSPAID);
	memset(szSPAIDHex, 0x00, sizeof(szSPAIDHex));
	inFunc_ASCII_to_BCD((unsigned char*)szSPAIDHex, szSPAID, 8);
	
	srAPDUData.uszCommandCLAData[0] = _FISC_SELECT_AID_CLA_COMMAND_;	/* CLA */
	srAPDUData.uszCommandINSData[0] = _FISC_SELECT_AID_INS_COMMAND_;	/* INS */
	srAPDUData.uszCommandP1Data[0] = _FISC_SELECT_AID_P1_COMMAND_;		/* P1 */
	srAPDUData.uszCommandP2Data[0] = _FISC_SELECT_AID_P2_COMMAND_;		/* P2 */
	memcpy(&srAPDUData.uszCommandData[0], szSPAIDHex, sizeof(szSPAIDHex));
	srAPDUData.inCommandDataLen = 8;
		
        if (inAPDU_BuildAPDU(&srAPDUData) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

        /* 收送Command */
	inRetVal = inAPDU_Send_APDU_User_Slot_Process(&srAPDUData);
        if (inRetVal == VS_SUCCESS)
        {
                if (!(srAPDUData.uszRecevData[srAPDUData.inRecevLen -2] == 0x90 && srAPDUData.uszRecevData[srAPDUData.inRecevLen -1] == 0x00))
                {
                        if (ginDebug == VS_TRUE)
                                inLogPrintf(AT, "FISC Select AID != 9000");
                        
                        return (VS_ERROR);
                }
                else
                {
                        if (ginDebug == VS_TRUE)
                                inLogPrintf(AT, "FISC = TRUE");
                        
                        /* 確認為金融卡 */
                        pobTran->uszFISCBit = VS_TRUE;
                }
        }
        else
        {
		/* SEND APDU FAIL */
		pobTran->inErrorMsg = _ERROR_CODE_V3_FISC_12_SEND_APDU_FAIL_;
                /* 下Command失敗 */
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "FISC APDU Command Error");
                        
                return (inRetVal);
        }

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inFISC_SelectAID()_END");
        }

        return (VS_SUCCESS);
}

/*
Function        :inFISC_PowerON
Date&Time       :2016/12/1 下午 2:57
Describe        :
*/
int inFISC_PowerON(TRANSACTION_OBJECT *pobTran)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
        unsigned char	szATR[128 + 1], szATRLen, szCardType;
	

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inFISC_PowerON START!!");
        }
        
        szATRLen = sizeof(szATR); 
        //Power on the ICC and retrun the ATR content metting the ISO-7816 specification //
	usRetVal = CTOS_SCResetISO(d_SC_USER, d_SC_5V, szATR, &szATRLen, &szCardType);
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
        
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inFISC_PowerON END!!");
        }
        
        return (VS_SUCCESS);
}

/*
Function        :inFISC_PowerOFF
Date&Time       :
Describe        :
*/
int inFISC_PowerOFF(TRANSACTION_OBJECT *pobTran)
{
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inFISC_PowerOFF START!!");
        }
        
        if (CTOS_SCPowerOff(d_SC_USER) != d_OK)
        {
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "CTOS_SCPowerOff ERROR!!");
                }
                
                return (VS_ERROR);
        }

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inFISC_PowerOFF END!!");
        }
        
        return (VS_SUCCESS);
}

/*
App Name	: inNCCC_FISC_Select_Menu
App Date&Time	: 2015/1/19 下午 05:21:13
App Function	: Select FISC or EMV card menu
Input Param	: *pobTran --> 交易結構
Output Param	: Run CARD_OPERATION  : KEY_e
		  Run FISC_CARD_OPEARTION : KEY_f
		  Time Out : KEY_TIMEOUT
		  Cencel : KEY_CANCEL
*/
int inFISC_Select_Menu(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	int	inChoice = 0;
	int	inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_8_3X3_;
	int	inECRReVal = VS_ERROR;
	char	szKey = 0x00;
	char	szCustomerIndicator[3 + 1] = {0};

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inNCCC_FISC_Select_Menu()_START");
        }
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);

        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        /* 1.信用卡 
           2.Smart Pay 
           請選擇 */
        inDISP_PutGraphic(_SELECT_CARD_, 0, _COORDINATE_Y_LINE_8_4_);
        
	/* 客製化107顯示訊息TimeOut 3秒 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_))
	{
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_107_BUMPER_DISPLAY_TIMEOUT_);
	}
	/* (需求單 - 107276)自助交易標準400做法 顯示訊息TimeOut 3秒 by Russell 2018/12/27 上午 11:20 */
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_)	||
		 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)			||
		 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
	{
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_111_KIOSK_STANDARD_DISPLAY_TIMEOUT_);
	}
	else
	{
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, 120);
	}
	
	while (1)
	{
		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
		szKey = uszKBD_Key();
		if (ginMachineType == _CASTLE_TYPE_V3UL_)
		{
			/* 只有V3 UL要吃取消 */
			if (inMultiFunc_First_Receive_Check() == VS_SUCCESS)
			{
				inECRReVal = inMultiFunc_First_Receive_Cancel();
				if (inECRReVal == VS_SUCCESS)
				{

				}
				else
				{
					/* 判斷為雜值要清空，再等待下一次 */
					inMultiFunc_RS232_FlushRxBuffer();
				}
			}
		}
		
		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			szKey = _KEY_TIMEOUT_;
		}

		if (szKey == _KEY_1_			||
		    inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_)
		{
			if (pobTran->uszCreditBit == VS_TRUE)
			{
				pobTran->uszCreditBit = VS_TRUE;
			}
			if (pobTran->uszUICCBit == VS_TRUE)
			{
				pobTran->uszUICCBit = VS_TRUE;
			}
			pobTran->uszFISCBit = VS_FALSE;
			inRetVal = VS_SUCCESS;
			break;
		}
		else if (szKey == _KEY_2_			||
		         inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_)
		{
			pobTran->uszFISCBit = VS_TRUE;
			pobTran->uszCreditBit = VS_FALSE;
			pobTran->uszUICCBit = VS_FALSE;
			inRetVal = VS_SUCCESS;
			break;
		}
		else if (szKey == _KEY_CANCEL_	||
			 inECRReVal == VS_SUCCESS)
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

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inNCCC_FISC_Select_Menu()_END");
        }

	return (inRetVal);
}

/*
 .App Name      : inNCCC_FISC_CardProcess
 .App Date&Time : 2016/2/19 下午 4:32
 .App Function  : 讀取卡片檔案,產生驗證TAC資料
 .Input Param   : TRANSACTION_OBJECT *pobTran
 .Output Param  : 1. 成功. -> VS_SUCCESS
	          2. 失敗. -> VS_ERROR
*/
int inFISC_CardProcess(TRANSACTION_OBJECT *pobTran)
{
	int     inRetVal = VS_ERROR;
        
        vdUtility_SYSFIN_LogMessage(AT, "inFISC_CardProcess START!");

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inFISC_ProcessFISCCard()_START");
        }

	if ((pobTran->srBRec.inCode != _FISC_SALE_ && pobTran->srBRec.inCode != _FISC_REFUND_) || pobTran->srBRec.uszFiscTransBit != VS_TRUE)
	{
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "inFISC_ProcessFISCCard()_inCode_ERROR");
                }
                
		return (VS_SUCCESS);
	}

	/* 退費交易可使用人工輸入, 讀寫晶片卡流程需略過 */
	if (pobTran->srBRec.uszManualBit == VS_TRUE && pobTran->srBRec.inCode == _FISC_REFUND_)
	{
		/* 金融卡發卡單位代號長度 */
		pobTran->srBRec.inFiscIssuerIDLength = _FISC_ISSUER_ID_SIZE_;
		/* 金融卡發卡單位代號 */
		sprintf(pobTran->srBRec.szFiscIssuerID, "00000000");
		/* 金融卡帳號長度 */
		pobTran->srBRec.inFiscAccountLength = _FISC_ACCOUNT_SIZE_;
		/* 金融卡帳號由人工輸入 */
		/* 金融卡備註欄長度 */
		pobTran->srBRec.inFiscCardCommentLength = _FISC_CARD_COMMET_SIZE_;
		/* 金融卡備註欄 */
		memset(pobTran->srBRec.szFiscCardComment, 0xFF, sizeof(pobTran->srBRec.szFiscCardComment));
		pobTran->srBRec.szFiscCardComment[pobTran->srBRec.inFiscCardCommentLength] = 0x00;

                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "inNCCC_FISC_CardProcess()_END");
                }
                
		return (VS_SUCCESS);
	}
        
        /* 晶片卡EFID 1001 */
        pobTran->inEFID = 1001;
	inRetVal = inFISC_CARD_SelectEF(pobTran);
        
	if (inRetVal != VS_SUCCESS)
	{
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "FISC_SELECT_EF_ERROR");
                }
                
		return (inRetVal);
	}

	inRetVal = inFISC_ReadRecords(pobTran);
        
	if (inRetVal != VS_SUCCESS)
	{
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "FISC_READ_RECORDS_ERROR");
                }

		return (inRetVal);
	}

	inRetVal = inFISC_WriteRecords(pobTran);
        
	if (inRetVal != VS_SUCCESS)
	{
		if (memcmp(gszTermVersionID, "MD731UAGAS001", strlen("MD731UAGAS001")) == 0)
		{
			
		}
		else
		{
			inNCCC_Func_Check_Card_Still_Exist_Flow(pobTran, _REMOVE_CARD_ERROR_); /* 請退回晶片卡 */
		}

                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "FISC_WRITE_RECORDS_ERROR");
                }
                
		return (inRetVal);
	}
        
        /* 讀卡號範圍 Load CDT HDT HDPT */
        if (inCARD_GetBin(pobTran) != VS_SUCCESS)
                return (VS_ERROR);

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inNCCC_FISC_ProcessFISCCard()_END");
        }
	
	return (VS_SUCCESS);
}

/*
 .App Name      : inFISC_CARD_SelectEF
 .App Date&Time : 2016/2/22 上午 10:54
 .App Function  : 讀取金融卡晶片資料
 .Input Param   : TRANSACTION_OBJECT *pobTran
 .Output Param  : 1. 成功. -> VS_SUCCESS
	          2. 失敗. -> VS_ERROR
*/
int inFISC_CARD_SelectEF(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal= VS_ERROR;
	char		szBuffer[_SELECT_EF_SIZE_ + 1] = {0};
	char		szDebugMsg[100 + 1] = {0};
	char		szAscii[100 + 1] = {0};
	APDU_COMMAND	srAPDUData;

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inNCCC_FISC_CARD_SelectEF()_START");
        }

	/* Select EF */
	memset(&szBuffer[0], 0x00, sizeof(szBuffer));

	switch(pobTran->inEFID)
	{
	        /* EF(Elementary File) */
		case 1001 :
			memcpy(&szBuffer[0],"\x10\x01", _SELECT_EF_SIZE_);
			break;
		case 1003 :
			memcpy(&szBuffer[0],"\x10\x03", _SELECT_EF_SIZE_);
			break;
		case 1004 :
			memcpy(&szBuffer[0],"\x10\x04", _SELECT_EF_SIZE_);
			break;
		default :
			memcpy(&szBuffer[0],"\x10\x03", _SELECT_EF_SIZE_);
			break;
	}

	/* 開始塞資料 */
	srAPDUData.uszCommandCLAData[0] = _FISC_SELECT_EF_CLA_COMMAND_;		/* CLA */
	srAPDUData.uszCommandINSData[0] = _FISC_SELECT_EF_INS_COMMAND_;		/* INS */
	srAPDUData.uszCommandP1Data[0] = _FISC_SELECT_EF_P1_COMMAND_;		/* P1 */
	srAPDUData.uszCommandP2Data[0] = _FISC_SELECT_EF_P2_COMMAND_;		/* P2 */
	memcpy(&srAPDUData.uszCommandData[0], &szBuffer[0], sizeof(szBuffer)); /* Data */
	srAPDUData.inCommandDataLen = _SELECT_EF_SIZE_; /* Data length */

	if (inAPDU_BuildAPDU(&srAPDUData) == VS_SUCCESS)
	{
		inRetVal = inAPDU_APDUTransmit_Flow(pobTran, &srAPDUData);		
		if (inRetVal == VS_SUCCESS)
		{
			/* 成功的狀況 */
			if (!memcmp(&srAPDUData.uszRecevData[srAPDUData.inRecevLen - 2],"\x90\x00",2))
			{
				inRetVal = VS_SUCCESS;
			}
			else
			{	
				if (!memcmp(&srAPDUData.uszRecevData[srAPDUData.inRecevLen - 2],"\x69\x82",2))
				{
					pobTran->inErrorMsg = _ERROR_CODE_V3_FISC_02_6982_;
				}
				else
				{
					pobTran->inErrorMsg = _ERROR_CODE_V3_FISC_01_READ_FAIL_;
				}

				if (ginDisplayDebug == VS_TRUE)
				{
					memset(szAscii, 0x00, sizeof(szAscii));
					inFunc_BCD_to_ASCII(szAscii, &srAPDUData.uszRecevData[srAPDUData.inRecevLen - 2], 2);
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "Fisc Read ERR: %s", szAscii);
					inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_TRUE);
				}

				inRetVal = VS_ERROR;
			}
		}
		else
		{
			/* SEND APDU FAIL */
			pobTran->inErrorMsg = _ERROR_CODE_V3_FISC_12_SEND_APDU_FAIL_;
			inRetVal = VS_ERROR;
		}
			
	}

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inNCCC_FISC_CARD_SelectEF()_END");
        }
        
	return (inRetVal);
}

/*
 .App Name      : inFISC_ReadRecords
 .App Date&Time : 2016/2/22 下午 3:35
 .App Function  : 讀取金融卡晶片資料
 .Input Param   : TRANSACTION_OBJECT *pobTran
 .Output Param  : 1. 成功. -> VS_SUCCESS
	          2. 失敗. -> VS_ERROR
*/
int inFISC_ReadRecords(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_SUCCESS;
        int		inCnt = 0;
	char		szBuffer[_SELECT_EF_SIZE_ + 1] = {0};
	char		szDebugMsg[100 + 1] = {0};
	char		szAscii[100 + 1] = {0};
	APDU_COMMAND	srAPDUData;

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inFISC_ReadRecords()_START");
        }
	if (ginDisplayDebug == VS_TRUE)
	{
		inDISP_LOGDisplay("inFISC_ReadRecords()_START", _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
	}

	memset(&szBuffer[0], 0x00, sizeof(szBuffer));

	/* 開始塞資料 */
	srAPDUData.uszCommandCLAData[0] = _FISC_READ_RECORDS_CLA_COMMAND_; /* CLA */
	srAPDUData.uszCommandINSData[0] = _FISC_READ_RECORDS_INS_COMMAND_; /* INS */
	srAPDUData.uszCommandP1Data[0] = _FISC_READ_RECORDS_P1_COMMAND_; /* P1 */
	srAPDUData.uszCommandP2Data[0] = _FISC_READ_RECORDS_P2_COMMAND_; /* P2 */
	memcpy(&srAPDUData.uszCommandData[0], &szBuffer[0], sizeof(szBuffer)); /* Data */
	srAPDUData.inCommandDataLen = 0; /* Data length */

	/* 使用APDU Command */
	if (inAPDU_BuildAPDU(&srAPDUData) == VS_SUCCESS)
	{
		inRetVal = inAPDU_APDUTransmit_Flow(pobTran, &srAPDUData);
		if (inRetVal == VS_SUCCESS)
		{
			/* 成功的狀況 */
			if (!memcmp(&srAPDUData.uszRecevData[srAPDUData.inRecevLen - 2],"\x90\x00",2))
			{
				inRetVal = VS_SUCCESS;
			}
			else
			{	
				if (!memcmp(&srAPDUData.uszRecevData[srAPDUData.inRecevLen - 2],"\x69\x82",2))
				{
					pobTran->inErrorMsg = _ERROR_CODE_V3_FISC_02_6982_;
				}
				else
				{
					pobTran->inErrorMsg = _ERROR_CODE_V3_FISC_01_READ_FAIL_;
				}

				if (ginDisplayDebug == VS_TRUE)
				{
					memset(szAscii, 0x00, sizeof(szAscii));
					inFunc_BCD_to_ASCII(szAscii, &srAPDUData.uszRecevData[srAPDUData.inRecevLen - 2], 2);
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "Fisc Read ERR: %s", szAscii);
					inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_TRUE);
				}

				inRetVal = VS_ERROR;
			}
		}
		else
		{
			/* SEND APDU FAIL */
			pobTran->inErrorMsg = _ERROR_CODE_V3_FISC_12_SEND_APDU_FAIL_;
			inRetVal = VS_ERROR;
		}
	}

	/* Read Records失敗不需塞資料 */
	if (inRetVal != VS_SUCCESS)
	{
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "inFISC_ReadRecords()_ERR");
                }
                
		return (inRetVal);
	}


	/* 寫回取得的資料 */
	inCnt = 0;
	/* --I = Record ID */
	/* Record ID 不需儲存 */
	inCnt ++;

	/* --L = Record Length */
	/* 金融卡發卡單位代號長度 */
	pobTran->srBRec.inFiscIssuerIDLength = srAPDUData.uszRecevData[inCnt];
	inCnt ++;

	/* --DATA */
	/* 金融卡發卡單位代號 */
	memcpy(&pobTran->srBRec.szFiscIssuerID[0], &srAPDUData.uszRecevData[inCnt], pobTran->srBRec.inFiscIssuerIDLength);
	inCnt += _FISC_ISSUER_ID_SIZE_;

	/* 間格 */
	inCnt ++;

	/* 金融卡備註欄長度 */
	pobTran->srBRec.inFiscCardCommentLength = srAPDUData.uszRecevData[inCnt];
	inCnt ++;

	/* 金融卡備註欄 */
	memcpy(&pobTran->srBRec.szFiscCardComment[0], &srAPDUData.uszRecevData[inCnt], pobTran->srBRec.inFiscCardCommentLength);
	inCnt += _FISC_CARD_COMMET_SIZE_;

	if (1)
	{
		/* 間格 */
		inCnt ++;

		/* 金融卡帳號長度 */
		pobTran->srBRec.inFiscAccountLength = srAPDUData.uszRecevData[inCnt];
		inCnt ++;

		/* 金融卡帳號 */
		memcpy(&pobTran->srBRec.szFiscAccount[0], &srAPDUData.uszRecevData[inCnt], _FISC_ACCOUNT_SIZE_);
		inCnt += _FISC_ACCOUNT_SIZE_;

		/* 將金融卡帳號塞入卡號 */
		memcpy(pobTran->srBRec.szPAN, pobTran->srBRec.szFiscAccount, _FISC_ACCOUNT_SIZE_);
	}
	else
	{
		/* Select 多帳號 */
		inRetVal = inFISC_SelectAccountMenu(pobTran, &srAPDUData.uszRecevData[inCnt]);
	}
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg,"inFiscIssuerIDLength : [%d]", pobTran->srBRec.inFiscIssuerIDLength);
		inLogPrintf(AT, szDebugMsg);
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg,"szFiscIssuerID : [%s]", pobTran->srBRec.szFiscIssuerID);
		inLogPrintf(AT, szDebugMsg);
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg,"inFiscCardCommentLength : [%d]", pobTran->srBRec.inFiscCardCommentLength);
		inLogPrintf(AT, szDebugMsg);
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg,"szFiscCardComment : [%s]", pobTran->srBRec.szFiscCardComment);
		inLogPrintf(AT, szDebugMsg);
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg,"inFiscAccountLength : [%d]", pobTran->srBRec.inFiscAccountLength);
		inLogPrintf(AT, szDebugMsg);
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg,"szFiscAccount : [%s]", pobTran->srBRec.szFiscAccount);
		inLogPrintf(AT, szDebugMsg);
	}
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inFISC_ReadRecords()_END");
        }
	if (ginDisplayDebug == VS_TRUE)
	{
		inDISP_LOGDisplay("inFISC_ReadRecords()_END", _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
	}
        
	return (inRetVal);
}

/*
Function        :inFISC_ReadRecords_1004
Date&Time       :2017/9/19 下午 1:24
Describe        :用來分大額還是小額
*/
int inFISC_ReadRecords_1004(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_SUCCESS;
        int		inCnt = 0;
	int		inRecordCnt = 0;		/* 單一Record的長度 */
	char		szBuffer[_SELECT_EF_SIZE_ + 1] = {0};
	char		szDebugMsg[100 + 1] = {0};
	char		szAscii[100 + 1] = {0};
	char		szFiscPhoneCardType[2 + 1] = {0};
	char		szFiscMobileDevice[2 + 1] = {0};
	APDU_COMMAND	srAPDUData;

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inFISC_ReadRecords_1004()_START");
        }

	memset(&szBuffer[0], 0x00, sizeof(szBuffer));

	/* 開始塞資料 */
	srAPDUData.uszCommandCLAData[0] = _FISC_READ_RECORDS_CLA_COMMAND_; /* CLA */
	srAPDUData.uszCommandINSData[0] = _FISC_READ_RECORDS_INS_COMMAND_; /* INS */
	srAPDUData.uszCommandP1Data[0] = _FISC_READ_RECORDS_P1_COMMAND_; /* P1 */
	srAPDUData.uszCommandP2Data[0] = _FISC_READ_RECORDS_P2_COMMAND_; /* P2 */
	memcpy(&srAPDUData.uszCommandData[0], &szBuffer[0], sizeof(szBuffer)); /* Data */
	srAPDUData.inCommandDataLen = 0; /* Data length */

	/* 使用APDU Command */
	if (inAPDU_BuildAPDU(&srAPDUData) == VS_SUCCESS)
	{
		inRetVal = inAPDU_APDUTransmit_Flow(pobTran, &srAPDUData);
		if (inRetVal == VS_SUCCESS)
		{
			/* 成功的狀況 */
			if (!memcmp(&srAPDUData.uszRecevData[srAPDUData.inRecevLen - 2],"\x90\x00",2))
			{
				inRetVal = VS_SUCCESS;
			}
			else
			{	
				if (!memcmp(&srAPDUData.uszRecevData[srAPDUData.inRecevLen - 2],"\x69\x82",2))
				{
					pobTran->inErrorMsg = _ERROR_CODE_V3_FISC_02_6982_;
				}
				else
				{
					pobTran->inErrorMsg = _ERROR_CODE_V3_FISC_01_READ_FAIL_;
				}

				if (ginDisplayDebug == VS_TRUE)
				{
					memset(szAscii, 0x00, sizeof(szAscii));
					inFunc_BCD_to_ASCII(szAscii, &srAPDUData.uszRecevData[srAPDUData.inRecevLen - 2], 2);
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "Fisc Read ERR: %s", szAscii);
					inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_TRUE);
				}

				inRetVal = VS_ERROR;
			}
		}
		else
		{
			/* SEND APDU FAIL */
			pobTran->inErrorMsg = _ERROR_CODE_V3_FISC_12_SEND_APDU_FAIL_;
			inRetVal = VS_ERROR;
		}
	}

	/* Read Records失敗不需塞資料 */
	if (inRetVal != VS_SUCCESS)
	{
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "inFISC_ReadRecords()_ERR");
                }
                
		return (inRetVal);
	}

	/* FOR當外接設備使用時 */
	pobTran->inFisc1004Len = srAPDUData.inRecevLen - 2;
	memset(pobTran->szFisc1004Data, 0x00, sizeof(pobTran->szFisc1004Data));
	memcpy(pobTran->szFisc1004Data, srAPDUData.uszRecevData, pobTran->inFisc1004Len);

	/* 寫回取得的資料 */
	inCnt = 0;
	
	/* 第一筆資料為交易裝置判別，值之定義如下：0x05：雲端卡片(Cloud-Based) */
	/* --I = Record ID */
	/* Record ID 不需儲存 */
	inCnt ++;

	/* --L = Record Length */
	inRecordCnt = srAPDUData.uszRecevData[inCnt];
	inCnt ++;

	/* --DATA */
	/* 如果為0x05代表為雲端行動金融卡 */
	memset(szFiscMobileDevice, 0x00, sizeof(szFiscMobileDevice));
	memcpy(szFiscMobileDevice, &srAPDUData.uszRecevData[inCnt], 1);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "szFiscMobileDevice: %X", szFiscMobileDevice[0]);
	}
	
	if (memcmp(szFiscMobileDevice, "\x05", 1) == 0)
	{
		memcpy(pobTran->srBRec.szFiscMobileDevice, _FISC_MOBILE_DEVICE_CLOUD_TXN_, 1);
	}
	else
	{
		memcpy(pobTran->srBRec.szFiscMobileDevice, szFiscMobileDevice, inRecordCnt);
	}
	inCnt += inRecordCnt;

	/* 還有剩餘資料，才可以繼續抓 */
	if ((srAPDUData.inRecevLen - 2 - inCnt) > 0)
	{
		/* --I = 第二欄位ID */
		inCnt ++;

		/* --L = Record Length */
		inRecordCnt = srAPDUData.uszRecevData[inCnt];
		inCnt ++;

		/* --DATA */
		memset(szFiscPhoneCardType, 0x00, sizeof(szFiscPhoneCardType));
		memcpy(szFiscPhoneCardType, &srAPDUData.uszRecevData[inCnt], 1);
		inCnt ++;

		/* "1004:01" */
		if(!memcmp(szFiscPhoneCardType, "\x01", 1))
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "1004:01");
			}

			memset(pobTran->srBRec.szFiscMobileNFType, 0x00, sizeof(pobTran->srBRec.szFiscMobileNFType));
			memcpy(pobTran->srBRec.szFiscMobileNFType, _FISC_MOBILE_NFTYPE_CONDITION_ENTER_PIN_, strlen(_FISC_MOBILE_NFTYPE_CONDITION_ENTER_PIN_));
		}
		else if (!memcmp(szFiscPhoneCardType, "\x02",1))
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "1004:02");
			}

			memset(pobTran->srBRec.szFiscMobileNFType, 0x00, sizeof(pobTran->srBRec.szFiscMobileNFType));
			memcpy(pobTran->srBRec.szFiscMobileNFType, _FISC_MOBILE_NFTYPE_MANDATORY_ENTER_PIN_, strlen(_FISC_MOBILE_NFTYPE_MANDATORY_ENTER_PIN_));
		}
		else if (!memcmp(szFiscPhoneCardType, "\x00",1))
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "1004:00");
			}

			memset(pobTran->srBRec.szFiscMobileNFType, 0x00, sizeof(pobTran->srBRec.szFiscMobileNFType));
			memcpy(pobTran->srBRec.szFiscMobileNFType, _FISC_MOBILE_NFTYPE_WITHOUT_ENTER_PIN_, strlen(_FISC_MOBILE_NFTYPE_WITHOUT_ENTER_PIN_));
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "1004:other, %X", szFiscPhoneCardType[0]);
			}
			/* 例外狀況，轉回實體卡 */
			memset(pobTran->srBRec.szFiscPayDevice, 0x00, sizeof(pobTran->srBRec.szFiscPayDevice));
			memcpy(pobTran->srBRec.szFiscPayDevice, _FISC_PAY_DEVICE_SMARTPAY_CARD_, strlen(_FISC_PAY_DEVICE_SMARTPAY_CARD_));
		}
	}
	
	/* 還有剩餘資料，才可以繼續抓 */
	if ((srAPDUData.inRecevLen - 2 - inCnt) > 0)
	{
		/* --I = 第三欄位ID */
		inCnt ++;

		/* --L = Record Length */
		inRecordCnt = srAPDUData.uszRecevData[inCnt];
		inCnt ++;

		/* --DATA */
		memset(pobTran->srBRec.szFiscMobileNFSetting, 0x00, sizeof(pobTran->srBRec.szFiscMobileNFSetting));
		memcpy(pobTran->srBRec.szFiscMobileNFSetting, &srAPDUData.uszRecevData[inCnt], inRecordCnt);
	}

	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inFISC_ReadRecords_1004()_END");
        }
	
	return (VS_SUCCESS);
}

/*
Function        :inFISC_ReadTxnAuthReqres
Date&Time       :2017/9/20 上午 9:36
Describe        :讀取安全元件的交易授權請求相關資料或授權回應相關資料
*/
int inFISC_ReadTxnAuthReqres(TRANSACTION_OBJECT *pobTran)
{
	int		inCnt = 0;
	int		inTacLen = 0;
	int		inRetVal = VS_SUCCESS;
	char		szBuffer[_SELECT_EF_SIZE_ + 1] = {0};
	char		szDebugMsg[100 + 1] = {0};
	char		szAscii[100 + 1] = {0};
	char		szTemplate[64 + 1] = {0};
	APDU_COMMAND	srAPDUData;

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inFISC_ReadTxnAuthReqres()_START");
        }

	memset(&szBuffer[0], 0x00, sizeof(szBuffer));

	/* 開始塞資料 */
	srAPDUData.uszCommandCLAData[0] = _FISC_READ_AUTH_RECORD_CLA_COMMAND_;		/* CLA */
	srAPDUData.uszCommandINSData[0] = _FISC_READ_AUTH_RECORD_INS_COMMAND_;		/* INS */
	srAPDUData.uszCommandP1Data[0] = _FISC_READ_AUTH_RECORD_P1_COMMAND_;		/* P1 */
	srAPDUData.uszCommandP2Data[0] = _FISC_READ_AUTH_RECORD_P2_COMMAND_;		/* P2 */
	memcpy(&srAPDUData.uszCommandData[0], &szBuffer[0], sizeof(szBuffer));		/* Data */
	srAPDUData.inCommandDataLen = 0; /* Data length */

	/* 使用APDU Command */
	if (inAPDU_BuildAPDU(&srAPDUData) == VS_SUCCESS)
	{
		inRetVal = inAPDU_APDUTransmit_Flow(pobTran, &srAPDUData);
		/* 成功的狀況 */
		if (inRetVal == VS_SUCCESS)
		{
			if (!memcmp(&srAPDUData.uszRecevData[srAPDUData.inRecevLen - 2],"\x90\x00",2))
			{
				inRetVal = VS_SUCCESS;
			}
			else
			{	
				if (!memcmp(&srAPDUData.uszRecevData[srAPDUData.inRecevLen - 2],"\x69\x82",2))
				{
					pobTran->inErrorMsg = _ERROR_CODE_V3_FISC_02_6982_;
				}
				else
				{
					pobTran->inErrorMsg = _ERROR_CODE_V3_FISC_01_READ_FAIL_;
				}

				if (ginDisplayDebug == VS_TRUE)
				{
					memset(szAscii, 0x00, sizeof(szAscii));
					inFunc_BCD_to_ASCII(szAscii, &srAPDUData.uszRecevData[srAPDUData.inRecevLen - 2], 2);
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "Fisc Read ERR: %s", szAscii);
					inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_TRUE);
				}

				inRetVal = VS_ERROR;
			}
		}
		else
		{
			/* SEND APDU FAIL */
			pobTran->inErrorMsg = _ERROR_CODE_V3_FISC_12_SEND_APDU_FAIL_;
			inRetVal = VS_ERROR;
		}
	}

	/* Read Records失敗不需塞資料 */
	if (inRetVal != VS_SUCCESS)
	{
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "inFISC_ReadRecords()_ERR");
                }
                
		return (inRetVal);
	}
	
	inCnt = 0;
	/* 金融卡交易序號長度 */
	pobTran->srBRec.inFiscSTANLength = srAPDUData.uszRecevData[inCnt];
	inCnt ++;
	/* 金融卡交易序號 */
	memcpy(pobTran->srBRec.szFiscSTAN, &srAPDUData.uszRecevData[inCnt], pobTran->srBRec.inFiscSTANLength);
	inCnt += 8;
	/* 金融卡交易授權驗證碼長度 */
	memcpy(pobTran->srBRec.szFiscTacLength,(char*)&srAPDUData.uszRecevData[inCnt], 2);
	memset(szTemplate, 0x00, sizeof(szTemplate));
        inFunc_BCD_to_ASCII(szTemplate, (unsigned char *)pobTran->srBRec.szFiscTacLength, 2);
	inTacLen = atoi(szTemplate);
	inCnt += 2;
	/* 金融卡交易授權驗證碼 */
	memcpy(pobTran->srBRec.szFiscTac, &srAPDUData.uszRecevData[inCnt], inTacLen);
	inCnt += 8;

	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inFISC_ReadRecords()_END");
        }
        
	return (inRetVal);
}

/*
 .App Name      : inFISC_SelectAccountMenu
 .App Date&Time : 2016/2/22 下午 3:57
 .App Function  : 選擇Smart Pay的帳號
 .Input Param   : TRANSACTION_OBJECT *pobTran, BYTE *uszReceBuf
 .Output Param  : 1. 成功. -> VS_SUCCESS
	          2. 失敗. -> VS_ERROR
*/
int inFISC_SelectAccountMenu(TRANSACTION_OBJECT *pobTran, BYTE *uszReceBuf)
{
	int	inRetVal, inCnt, inAccountIndex;
	int	inTotalAccount = 0, inSelectAccountLen = 0;
	int	inPage, inKEY, i;
	char	szDispMsg[26 + 1], szTemplate[_FISC_ACCOUNT_SIZE_ + 1];
        char    szDebugMSG[100 + 1];
	VS_BOOL	fSelectPage = VS_TRUE;
	SELECT_ACCOUNT_OBJECT srAccountData[8 + 1];

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inFISC_SelectAccountMenu()_START");
        }

	inCnt = 0;

	for (inAccountIndex = 0; inAccountIndex < 8; inAccountIndex ++)
	{
		/* 間格 */
		inCnt ++;

		/* 金融卡帳號長度 */
		inSelectAccountLen = uszReceBuf[inCnt];
		inCnt ++;

		/* 金融卡帳號 */
		memcpy(&szTemplate[0], &uszReceBuf[inCnt], inSelectAccountLen);
		inCnt += inSelectAccountLen;

		/* 檢查帳號值是否正確 */
		inRetVal = inFISC_CheckAccountData(szTemplate, inSelectAccountLen);

		if (inRetVal == VS_SUCCESS)
		{
			srAccountData[inTotalAccount].inSelectAccountLen = inSelectAccountLen;
			memcpy(&srAccountData[inTotalAccount].szSelectAccount[0], &szTemplate[0], _FISC_ACCOUNT_SIZE_);
			inTotalAccount ++;
		}
		else if (inRetVal != VS_ESCAPE)
		{
			/* 此帳號未啟用 */
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "inFISC_CARD_CheckAccountData()_ERROR");
                        }
		}

	} /* end of for */

	if (inTotalAccount == 1)
	{
		/* 金融卡帳號長度 */
		pobTran->srBRec.inFiscAccountLength = srAccountData[0].inSelectAccountLen;

		/* 金融卡帳號 */
		memcpy(&pobTran->srBRec.szFiscAccount[0], &srAccountData[0].szSelectAccount[0], pobTran->srBRec.inFiscAccountLength);

		/* 將金融卡帳號塞入卡號 */
		memcpy(pobTran->srBRec.szPAN, pobTran->srBRec.szFiscAccount, _FISC_ACCOUNT_SIZE_);
	}
	else if (inTotalAccount > 1)
	{
                inDISP_ChineseFont("請選擇帳號:", _FONTSIZE_8X16_, _LINE_8_2_, _DISP_LEFT_);

		inPage = 1;

		do
		{
			switch (inPage)
			{
				case 1 :
                                        inDISP_Clear_Line(_LINE_8_3_, _LINE_8_8_);
                                        
					for (i = 0; i < 4; i ++)
					{
						if (srAccountData[i].inSelectAccountLen > 0)
						{
							memset(szDispMsg, 0x00, sizeof(szDispMsg));
							sprintf(szDispMsg,"%d.%s", i + 1, &srAccountData[i].szSelectAccount[0]);
							
                                                        if (ginDebug == VS_TRUE)
                                                        {
                                                                sprintf(szDebugMSG,"%s", szDispMsg);
                                                                inLogPrintf(AT, szDebugMSG);
                                                        }
                                                        
                                                        inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, i + 4, _DISP_LEFT_);
						}
					}

					inKEY = uszKBD_GetKey(_FISC_PIN_TIMEOUT_);
					while (1)
					{
						if (inKEY == _KEY_1_)
						{
							if (srAccountData[0].inSelectAccountLen > 0)
							{
								/* 金融卡帳號長度 */
								pobTran->srBRec.inFiscAccountLength = srAccountData[0].inSelectAccountLen;

								/* 金融卡帳號 */
								memcpy(&pobTran->srBRec.szFiscAccount[0], &srAccountData[0].szSelectAccount[0], pobTran->srBRec.inFiscAccountLength);
							}

							fSelectPage = VS_FALSE;
						}
						else if (inKEY == _KEY_2_)
						{
							if (inTotalAccount > 1 && srAccountData[1].inSelectAccountLen > 0)
							{
								/* 金融卡帳號長度 */
								pobTran->srBRec.inFiscAccountLength = srAccountData[1].inSelectAccountLen;

								/* 金融卡帳號 */
								memcpy(&pobTran->srBRec.szFiscAccount[0], &srAccountData[1].szSelectAccount[0], pobTran->srBRec.inFiscAccountLength);
							}

							fSelectPage = VS_FALSE;
						}
						else if (inKEY == _KEY_3_)
						{
							if (inTotalAccount > 2 && srAccountData[2].inSelectAccountLen > 0)
							{
								/* 金融卡帳號長度 */
								pobTran->srBRec.inFiscAccountLength = srAccountData[2].inSelectAccountLen;

								/* 金融卡帳號 */
								memcpy(&pobTran->srBRec.szFiscAccount[0], &srAccountData[2].szSelectAccount[0], pobTran->srBRec.inFiscAccountLength);
							}

							fSelectPage = VS_FALSE;
						}
						else if (inKEY == _KEY_4_)
						{
							if (inTotalAccount > 3 && srAccountData[3].inSelectAccountLen > 0)
							{
								/* 金融卡帳號長度 */
								pobTran->srBRec.inFiscAccountLength = srAccountData[3].inSelectAccountLen;

								/* 金融卡帳號 */
								memcpy(&pobTran->srBRec.szFiscAccount[0], &srAccountData[3].szSelectAccount[0], pobTran->srBRec.inFiscAccountLength);
							}

							fSelectPage = VS_FALSE;
						}
						else if (inKEY == _KEY_ENTER_)
						{
							if (inTotalAccount > 4)
								inPage ++;
						}
						else if (inKEY == _KEY_CANCEL_ || inKEY == _KEY_TIMEOUT_)
							return (VS_ERROR);
						else
							continue;

						break;
					}

					break;
				case 2 :
                                        inDISP_Clear_Line(_LINE_8_3_, _LINE_8_8_);
                                        
					for (i = 4; i < 8; i ++)
					{
						if (srAccountData[i].inSelectAccountLen > 0)
						{
							memset(szDispMsg, 0x00, sizeof(szDispMsg));
							sprintf(szDispMsg,"%d.%s", i + 1, &srAccountData[i].szSelectAccount[0]);
                                                        
                                                        if (ginDebug == VS_TRUE)
                                                        {
                                                                sprintf(szDebugMSG,"%s", szDispMsg);
                                                                inLogPrintf(AT, szDebugMSG);
                                                        }
                                                        
                                                        inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, i + 4, _DISP_LEFT_);
						}
					}

					inKEY = uszKBD_GetKey(_FISC_PIN_TIMEOUT_);
					while (1)
					{
						if (inKEY == _KEY_5_)
						{
							if (srAccountData[4].inSelectAccountLen > 0)
							{
								/* 金融卡帳號長度 */
								pobTran->srBRec.inFiscAccountLength = srAccountData[4].inSelectAccountLen;

								/* 金融卡帳號 */
								memcpy(&pobTran->srBRec.szFiscAccount[0], &srAccountData[4].szSelectAccount[0], pobTran->srBRec.inFiscAccountLength);
							}

							fSelectPage = VS_FALSE;
						}
						else if (inKEY == _KEY_6_)
						{
							if (inTotalAccount > 5 && srAccountData[5].inSelectAccountLen > 0)
							{
								/* 金融卡帳號長度 */
								pobTran->srBRec.inFiscAccountLength = srAccountData[5].inSelectAccountLen;

								/* 金融卡帳號 */
								memcpy(&pobTran->srBRec.szFiscAccount[0], &srAccountData[5].szSelectAccount[0], pobTran->srBRec.inFiscAccountLength);
							}

							fSelectPage = VS_FALSE;
						}
						else if (inKEY == _KEY_7_)
						{
							if (inTotalAccount > 6 && srAccountData[6].inSelectAccountLen > 0)
							{
								/* 金融卡帳號長度 */
								pobTran->srBRec.inFiscAccountLength = srAccountData[6].inSelectAccountLen;

								/* 金融卡帳號 */
								memcpy(&pobTran->srBRec.szFiscAccount[0], &srAccountData[6].szSelectAccount[0], pobTran->srBRec.inFiscAccountLength);
							}

							fSelectPage = VS_FALSE;
						}
						else if (inKEY == _KEY_8_)
						{
							if (inTotalAccount > 7 && srAccountData[7].inSelectAccountLen > 0)
							{
								/* 金融卡帳號長度 */
								pobTran->srBRec.inFiscAccountLength = srAccountData[7].inSelectAccountLen;

								/* 金融卡帳號 */
								memcpy(&pobTran->srBRec.szFiscAccount[0], &srAccountData[7].szSelectAccount[0], pobTran->srBRec.inFiscAccountLength);
							}

							fSelectPage = VS_FALSE;
						}
						else if (inKEY == _KEY_ENTER_)
						{
							inPage --;
						}
						else if (inKEY == _KEY_CANCEL_ || inKEY == _KEY_TIMEOUT_)
							return (VS_ERROR);
						else
							continue;

						break;
					}
					break;
				default :
					
                                        if (ginDebug == VS_TRUE)
                                        {
                                                sprintf(szDebugMSG,"inNCCC_FISC_SelectAccountMenu()_Page_ERROR");
                                                inLogPrintf(AT, szDebugMSG);
                                        }
					return (VS_ERROR);

			} /* end of switch */

		} while (fSelectPage == VS_TRUE);

		/* 將選擇的金融卡帳號塞入卡號 */
		memcpy(pobTran->srBRec.szPAN, pobTran->srBRec.szFiscAccount, _FISC_ACCOUNT_SIZE_);

	}

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inNCCC_FISC_SelectAccountMenu()_END");
        }

	return (VS_SUCCESS);
}


/*
 .App Name      : inFISC_GetUserPin
 .App Date&Time : 2016/11/4 上午 9:48
 .App Function  : 取得金融卡Pin 內建/外接
 .Input Param   : TRANSACTION_OBJECT *pobTran
 .Output Param  : 1. 成功. -> VS_SUCCESS
	          2. 失敗. -> VS_ERROR
*/
int inFISC_GetUserPin(TRANSACTION_OBJECT *pobTran)
{
//        int     i = 0;
	int	inRetVal;
	char 	szPinData[_FISC_PIN_SIZE_ + 1];
        char    szPinpadMode[1 + 1]; 
        char    szDebugMsg[100 + 1];
        char    szDispMsg[100 + 1];

        vdUtility_SYSFIN_LogMessage(AT, "inFISC_GetUserPin START!");
        
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inFISC_GetUserPin()_START");
        }
        

	/* 僅消費扣款, 退費交易需要輸入Pin */
	if ((pobTran->srBRec.inCode != _FISC_SALE_ && pobTran->srBRec.inCode != _FISC_REFUND_) || pobTran->srBRec.uszFiscTransBit != VS_TRUE)
	{
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "NO need enter PIN");
                }
                
		return (VS_SUCCESS);
	}

	/* 人工輸入不需要輸入Pin */
	if (pobTran->srBRec.uszManualBit == VS_TRUE && pobTran->srBRec.inCode == _FISC_REFUND_)
	{
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "Manual Key in NO need enter PIN");
                }
                
		return (VS_SUCCESS);
	}

	/* Smart Pay感應不需要輸入Pin (從TRT中拿掉了) */

	if (inLoadCFGTRec(0) < 0)
	{
		return (VS_ERROR);
	}

        memset(szPinpadMode, 0x00, sizeof(szPinpadMode));
        inGetPinpadMode(szPinpadMode);
        
	if (memcmp(szPinpadMode, _PINPAD_MODE_0_NO_, 1) == 0)
	{
                /* 密碼機未連線 */
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, _ERR_PINPAD_CONNECT_);
		srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
		srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
		srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
		strcpy(srDispMsgObj.szErrMsg1, "");
		srDispMsgObj.inErrMsg1Line = 0;
		srDispMsgObj.inBeepTimes = 1;
		srDispMsgObj.inBeepInterval = 0;
                inDISP_Msg_BMP(&srDispMsgObj);
		return(VS_ERROR);
	}
	else if (memcmp(szPinpadMode, _PINPAD_MODE_1_INTERNAL_, 1) == 0)
	{
		if (pobTran->srBRec.lnTxnAmount > 0 )
		{
                        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                        inDISP_PutGraphic(_FISC_GET_PASSWORD_, 0, _COORDINATE_Y_LINE_8_4_);
                        memset(szDispMsg, 0x00, sizeof(szDispMsg));
                        sprintf(szDispMsg, "%ld", pobTran->srBRec.lnTxnAmount);
                        inFunc_Amount_Comma(szDispMsg, "NT$ " , ' ', _SIGNED_NONE_, 16, VS_TRUE);
                        inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_LEFT_);
                        
                        /* Mirror Message */
                        if (pobTran->uszECRBit == VS_TRUE)
                        {
                                inECR_SendMirror(pobTran, _MIRROR_MSG_ENTER_PW_);
                        }

			/* 輸入密碼 */
			memset(szPinData, 0x00, sizeof(szPinData));
			inRetVal = inFISC_EnterPin(szPinData);

                        /* 重新顯示的交易名稱 */
			inFunc_ResetTitle(pobTran);

                        if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL || inRetVal == VS_ERROR)
                        {
                                if (ginDebug == VS_TRUE)
                                {
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                                        sprintf(szDebugMsg, "inFISC_EnterPin TimeOut or Cancel: %d", inRetVal);
                                        inLogPrintf(AT, szDebugMsg);
                                }
				
                                return(inRetVal);
                        }

                        /* Pin 不足16碼, 補'F' */
                        memset(pobTran->szFiscPin, 'F', sizeof(pobTran->szFiscPin));
                        memcpy(pobTran->szFiscPin, szPinData, strlen(szPinData));
                        pobTran->szFiscPin[_FISC_PIN_SIZE_] = 0x00;
                        
                        if (ginDebug == VS_TRUE)
                        {
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                                sprintf(szDebugMsg,"FISC_PIN(s) : [%s]", pobTran->szFiscPin);
                                inLogPrintf(AT, szDebugMsg);
                        }
		}
	}
        else if (memcmp(szPinpadMode, _PINPAD_MODE_2_EXTERNAL_, 1) == 0)
        {
                /* 還沒開發外接Pinpad，TMS設錯 提示密碼機未連線 */
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, _ERR_PINPAD_CONNECT_);
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
                sprintf(szDebugMsg,"inFISC_GetUserPin()_END");
                inLogPrintf(AT, szDebugMsg);
        }

	return(VS_SUCCESS);
}

/*
 .App Name      : inFISC_VerifyPin
 .App Date&Time : 2016/11/4 上午 9:58
 .App Function  : 驗證金融卡Pin
 .Input Param   : TRANSACTION_OBJECT *pobTran
 .Output Param  : 1. 成功. -> VS_SUCCESS
	          2. 失敗. -> VS_ERROR
*/
int inFISC_VerifyPin(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_ERROR;
	char		szBuffer[_FISC_PIN_SIZE_ + 1];
	char		szDebugMSG[100 + 1];
	APDU_COMMAND	srAPDUData;

        vdUtility_SYSFIN_LogMessage(AT, "inFISC_VerifyPin START!");
        
        if (ginDebug == VS_TRUE)
        {
                sprintf(szDebugMSG,"inFISC_VerifyPin()_START");
                inLogPrintf(AT, szDebugMSG);
        }

	/* 消費扣款, 退費交易需要輸入Pin */
	if ((pobTran->srBRec.inCode != _FISC_SALE_ && pobTran->srBRec.inCode != _FISC_REFUND_) || pobTran->srBRec.uszFiscTransBit != VS_TRUE)
	{
                if (ginDebug == VS_TRUE)
                {
                        sprintf(szDebugMSG,"inFISC_VerifyPin()_END");
                        inLogPrintf(AT, szDebugMSG);
                }
		return (VS_SUCCESS);
	}

	/* 人工輸入不需要輸入Pin */
	if (pobTran->srBRec.uszManualBit == VS_TRUE && pobTran->srBRec.inCode == _FISC_REFUND_)
	{
                if (ginDebug == VS_TRUE)
                {
                        sprintf(szDebugMSG,"inFISC_GetUserPin()_END");
                        inLogPrintf(AT, szDebugMSG);
                }
		return (VS_SUCCESS);
	}

	/* Smart Pay感應不需要輸入Pin(從TRT拿掉) */

	/* 不足補'F' */
	memset(szBuffer, 0x00, sizeof(szBuffer));

	memcpy(szBuffer, pobTran->szFiscPin, _FISC_PIN_SIZE_);
	
        if (ginDebug == VS_TRUE)
        {
                sprintf(szDebugMSG, "Pin Hex(%s)", szBuffer);
                inLogPrintf(AT, szDebugMSG);
        }
        
	/* 開始塞資料 */
	srAPDUData.uszCommandCLAData[0] = _FISC_VERIFY_PIN_CLA_COMMAND_;		/* CLA */
	srAPDUData.uszCommandINSData[0] = _FISC_VERIFY_PIN_INS_COMMAND_;		/* INS */
	srAPDUData.uszCommandP1Data[0] = _FISC_VERIFY_PIN_P1_COMMAND_;			/* P1 */
	srAPDUData.uszCommandP2Data[0] = _FISC_VERIFY_PIN_P2_COMMAND_;			/* P2 */
        inFunc_ASCII_to_BCD(&srAPDUData.uszCommandData[0], &szBuffer[0], _FISC_PIN_BCD_SIZE_);
	srAPDUData.inCommandDataLen = _FISC_PIN_BCD_SIZE_; /* Data length */

	if (inAPDU_BuildAPDU(&srAPDUData) == VS_SUCCESS)
	{
		inRetVal = inAPDU_APDUTransmit_Flow(pobTran, &srAPDUData);
		if (inRetVal == VS_SUCCESS)
		{
			/* 成功的狀況 */
			if (!memcmp(&srAPDUData.uszRecevData[srAPDUData.inRecevLen - 2],"\x90\x00",2))
			{
				inRetVal = VS_SUCCESS;
			}
			else
			{	
				if(!memcmp(&srAPDUData.uszRecevData[0], "\x69\x82", 2))
				{
					DISPLAY_OBJECT	srDispMsgObj;
					memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
					strcpy(srDispMsgObj.szDispPic1Name, _ERR_CARD_FAIL_);
					srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
					srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
					srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
					strcpy(srDispMsgObj.szErrMsg1, "");
					srDispMsgObj.inErrMsg1Line = 0;
					srDispMsgObj.inBeepTimes = 1;
					srDispMsgObj.inBeepInterval = 0;
					inDISP_Msg_BMP(&srDispMsgObj);      /* 卡片失效 */
				}
				/* 密碼錯誤*/
				else if(!memcmp(&srAPDUData.uszRecevData[0], "\x66\x08", 2))
				{
					DISPLAY_OBJECT	srDispMsgObj;
					memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
					strcpy(srDispMsgObj.szDispPic1Name, "");
					srDispMsgObj.inDispPic1YPosition = 0;
					srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
					srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
					strcpy(srDispMsgObj.szErrMsg1, "密碼錯誤");
					srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
					srDispMsgObj.inBeepTimes = 1;
					srDispMsgObj.inBeepInterval = 0;
					inDISP_Msg_BMP(&srDispMsgObj);      /* 卡片失效 */
				}
				else if(!memcmp(&srAPDUData.uszRecevData[0], "\x66\x20", 2))      /* 用鎖卡卡片抓出來的值 */
				{
					DISPLAY_OBJECT	srDispMsgObj;
					memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
					strcpy(srDispMsgObj.szDispPic1Name, _ERR_PASSWORD_OVER_LIMIT_);
					srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
					srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
					srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
					strcpy(srDispMsgObj.szErrMsg1, "");
					srDispMsgObj.inErrMsg1Line = 0;
					srDispMsgObj.inBeepTimes = 1;
					srDispMsgObj.inBeepInterval = 0;
					inDISP_Msg_BMP(&srDispMsgObj); /* 密碼錯誤超過上限 */
				}
				else
				{
					DISPLAY_OBJECT	srDispMsgObj;
					memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
					strcpy(srDispMsgObj.szDispPic1Name, _ERR_READ_CARD_);
					srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
					srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
					srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
					strcpy(srDispMsgObj.szErrMsg1, "");
					srDispMsgObj.inErrMsg1Line = 0;
					srDispMsgObj.inBeepTimes = 1;
					srDispMsgObj.inBeepInterval = 0;
					inDISP_Msg_BMP(&srDispMsgObj);       /* 讀卡失敗 */
				}

				inRetVal = VS_ERROR;
			}
		}
		else
		{
			/* SEND APDU FAIL */
			pobTran->inErrorMsg = _ERROR_CODE_V3_FISC_12_SEND_APDU_FAIL_;
			inRetVal = VS_ERROR;
		}
	}

        if (ginDebug == VS_TRUE)
        {
                sprintf(szDebugMSG,"inFISC_VerifyPin()_END");
                inLogPrintf(AT, szDebugMSG);
        }
        
	return (inRetVal);
}

/*
 .App Name      : inFISC_WriteRecords
 .App Date&Time : 2016/11/4 上午 10:06
 .App Function  : 取得晶片卡交易授權驗證碼, 把時間設定為讀卡的時間
 .Input Param   : TRANSACTION_OBJECT *pobTran
 .Output Param  : 1. 成功. -> VS_SUCCESS
	        	  2. 失敗. -> VS_ERROR
*/
int inFISC_WriteRecords(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_SUCCESS;
        int		inCnt = 0, inTacLen = 0;
	char		szSendData[_WRITE_RECORD_SIZE_ + 1] = {0};
	char		szMID[10 + 1] = {0};
	char		szTemplate[64 + 1] = {0};
        char		szDebugMSG[100 + 1] = {0};
        char		szMCCCode[4 + 1] = {0};
        char		szMerchantID[15 + 1] = {0};
        char		szTerminalID[8 + 1] = {0};
	char		szIssuerID[5 + 1] = {0};
	APDU_COMMAND	srAPDUData;
        RTC_NEXSYS	srRTC;
	
        if (ginDebug == VS_TRUE)
        {
                sprintf(szDebugMSG,"inFISC_WriteRecords()_START_inCode(%d)", pobTran->srBRec.inCode);
                inLogPrintf(AT, szDebugMSG);
        }

	if (inLoadMVTRec(0) < 0)
	{
                if (ginDebug == VS_TRUE)
                {
                        sprintf(szDebugMSG,"inLoadMVTRec(0) ERROR");
                        inLogPrintf(AT, szDebugMSG);
                }
                
		return (VS_ERROR);
	}

	if (inLoadHDTRec(0) < 0)
	{
                if (ginDebug == VS_TRUE)
                {
                        sprintf(szDebugMSG,"inLoadHDTRec(0) ERROR");
                        inLogPrintf(AT, szDebugMSG);
                }
                
		return (VS_ERROR);
	}
        
	inCnt = 0;
	inTacLen = 0;
	memset(szSendData, 0x00, sizeof(szSendData));
	memset(szTemplate, 0x00, sizeof(szTemplate));
        
	memset(szMID, 0x00, sizeof(szMID));
        memset(szMerchantID, 0x00, sizeof(szMerchantID));
        inGetMerchantID(szMerchantID);
        memcpy(szMID, szMerchantID, 10);
	
        memset(szMCCCode, 0x00, sizeof(szMCCCode));
        inGetMCCCode(szMCCCode);
	
	memset(szIssuerID, 0x00, sizeof(szIssuerID));
	inGetIssuerID(szIssuerID);

	/* MCC 由 EMVDef.txt第一組的參數的MCC(下TMS時同步到EDC.dat上) */
	memcpy(&pobTran->srBRec.szFiscMCC[0], &szMCCCode[0], 4);

	/* 設卡片交易時間 */
        memset(&srRTC, 0x00, sizeof(CTOS_RTC));
        /* 取得EDC時間日期 */
        if (inFunc_GetSystemDateAndTime(&srRTC) != VS_SUCCESS)
        {
                if (ginDebug == VS_TRUE)
                {
                        sprintf(szDebugMSG,"inFunc_GetDateAndTime ERROR");
                        inLogPrintf(AT, szDebugMSG);
                }
		
                /* 感應燈號及聲響 */
		pobTran->inErrorMsg = _ERROR_CODE_V3_FISC_03_TIME_ERROR_;
                
                return (VS_ERROR);
        }

        memset(&pobTran->srBRec.szDate, 0x00, sizeof(pobTran->srBRec.szDate));
        memset(&pobTran->srBRec.szTime, 0x00, sizeof(pobTran->srBRec.szTime));
        sprintf(pobTran->srBRec.szDate, "20%02d%02d%02d",  srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
        sprintf(pobTran->srBRec.szTime, "%02d%02d%02d",  srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
	sprintf(pobTran->srBRec.szFiscDateTime, "%s%s", pobTran->srBRec.szDate, pobTran->srBRec.szTime);

	/* 產生端末設備查核碼 */
        if (ginDebug == VS_TRUE)
        {
                sprintf(szDebugMSG,"TCC Start");
                inLogPrintf(AT, szDebugMSG);
        }
        
	if (inFISC_GenMACAndTCC(pobTran) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
                {
                        sprintf(szDebugMSG, "inFISC_GenMAC Error");
                        inLogPrintf(AT, szDebugMSG);
                }

                /* 感應燈號及聲響 */
		pobTran->inErrorMsg = _ERROR_CODE_V3_FISC_04_MAC_TAC_;
                
		return (VS_ERROR);
	}

	/* 針對不同交易別, 組Send Data */
	switch (pobTran->srBRec.inCode)
	{
		case _FISC_SALE_ :
			memcpy(&szSendData[inCnt], "2541", 4);
			inCnt += 4;
			/* 金額 */
			sprintf(&szSendData[inCnt], "%012ld00", pobTran->srBRec.lnTxnAmount);
			inCnt += 14;
			/* TID */
                        memset(szTerminalID, 0x00, sizeof(szTerminalID));
                        inGetTerminalID(szTerminalID);
			memcpy(&szSendData[inCnt], szTerminalID, 8);
			inCnt += 8;
			/* 端末機檢核碼 */
			memcpy(&szSendData[inCnt], &pobTran->srBRec.szFiscTCC[0], 8);
			inCnt += 8;
			/* 交易時間 YYYYMMDDHHMMSS */
			sprintf(&szSendData[inCnt], "%s", pobTran->srBRec.szFiscDateTime);
			inCnt += 14;
			/* MID */
			/* 聯合的最前面要放金融機構末三碼 */
			sprintf(&szSendData[inCnt], "%s%s  ", &szIssuerID[1], szMID);
			inCnt += 15;
			/* MCC */
			sprintf(&szSendData[inCnt], "%s", szMCCCode);
			inCnt += 4;
			/* 金融機構代碼 */
			/* 合庫金融機構代碼為0060 聯合為0956 */
			sprintf(&szSendData[inCnt], szIssuerID);
			inCnt += 4;
			memcpy(&szSendData[inCnt], "       ", 7);
			inCnt += 7;
			/* 金融卡帳號 */
			memcpy(&szSendData[inCnt], pobTran->srBRec.szFiscAccount, _FISC_ACCOUNT_SIZE_);
			inCnt += _FISC_ACCOUNT_SIZE_;
			break;
		case _FISC_REFUND_ :
			/* 2015/4/22 下午 05:54:15 add by luko 退費交易不用寫入卡片, 不用組Send Data */
			/* 卡片交易序號 : 退費交易預設值為 "00000000" */
			memcpy(&pobTran->srBRec.szFiscSTAN[0], "00000000", 8);

			/* Smart Pay 交易認證碼 : 退費交易預設值為 "00000000" */
			memcpy(&pobTran->srBRec.szFiscTac[0], "00000000", 8);

                        if (ginDebug == VS_TRUE)
                        {
                                sprintf(szDebugMSG,"inFISC_WriteRecords()_END");
                                inLogPrintf(AT, szDebugMSG);
                        }
			return (VS_SUCCESS);
		default :
			
                        if (ginDebug == VS_TRUE)
                        {
                                sprintf(szDebugMSG,"inFISC_WriteRecords()_inCode_ERR");
                                inLogPrintf(AT, szDebugMSG);
                        }
                        
                        /* 感應燈號及聲響 */
                        pobTran->inErrorMsg = _ERROR_CODE_V3_FISC_05_NO_INCODE_;
                        
			return(VS_ERROR);
	}

	/* 開始塞資料 */
	memset(&srAPDUData, 0x00, sizeof(srAPDUData));
	srAPDUData.uszCommandCLAData[0] = _FISC_WRITE_RECORD_CLA_COMMAND_;		/* CLA */
	srAPDUData.uszCommandINSData[0] = _FISC_WRITE_RECORD_INS_COMMAND_;		/* INS */
	srAPDUData.uszCommandP1Data[0] = _FISC_WRITE_RECORD_P1_COMMAND_;		/* P1 */
	
	/* 這個欄位塞錯，會導致TAC算錯 */
	/* 感應要改塞0x90 */
        if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
                srAPDUData.uszCommandP2Data[0] = _FISC_WRITE_RECORD_P2_COMMAND_CTLS_;
        else
                srAPDUData.uszCommandP2Data[0] = _FISC_WRITE_RECORD_P2_COMMAND_;	/* P2 */
	
	memcpy(&srAPDUData.uszCommandData[0], &szSendData[0], sizeof(szSendData));	/* Data */
	srAPDUData.inCommandDataLen = inCnt; /* Data length */

        if (ginDebug == VS_TRUE)
        {
                sprintf(szDebugMSG,"[srAPDUData]");
                inLogPrintf(AT, szDebugMSG);
		sprintf(szDebugMSG,"srAPDUData.uszCommandCLAData = [%02x]", srAPDUData.uszCommandCLAData[0]);
                inLogPrintf(AT, szDebugMSG);
                sprintf(szDebugMSG,"srAPDUData.uszCommandINSData = [%02x]", srAPDUData.uszCommandINSData[0]);
                inLogPrintf(AT, szDebugMSG);
                sprintf(szDebugMSG,"srAPDUData.uszCommandP1Data = [%02x]", srAPDUData.uszCommandP1Data[0]);
                inLogPrintf(AT, szDebugMSG);
                sprintf(szDebugMSG,"srAPDUData.uszCommandP2Data = [%02x]", srAPDUData.uszCommandP2Data[0]);
                inLogPrintf(AT, szDebugMSG);
                sprintf(szDebugMSG,"srAPDUData.inCommandDataLen = [%d]", srAPDUData.inCommandDataLen);
                inLogPrintf(AT, szDebugMSG);
        }

	if (inAPDU_BuildAPDU(&srAPDUData) == VS_SUCCESS)
	{
		inRetVal = inAPDU_APDUTransmit_Flow(pobTran, &srAPDUData);
		if (inRetVal == VS_SUCCESS)
		{
			/* 成功的狀況 */
			if (!memcmp(&srAPDUData.uszRecevData[srAPDUData.inRecevLen - 2],"\x90\x00",2))
			{
				inRetVal = VS_SUCCESS;
			}
			else
			{	
				if (!memcmp(&srAPDUData.uszRecevData[srAPDUData.inRecevLen - 2],"\x69\x82",2))
				{
					pobTran->inErrorMsg = _ERROR_CODE_V3_FISC_02_6982_;
				}
				else
				{
					pobTran->inErrorMsg = _ERROR_CODE_V3_FISC_01_READ_FAIL_;
				}

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMSG, 0x00, sizeof(szDebugMSG));
					sprintf(szDebugMSG,"inFISC_APDUTransmit()_ERROR");
					inLogPrintf(AT, szDebugMSG);
				}
				inRetVal = VS_ERROR;
			}
		}
		else
		{
			/* SEND APDU FAIL */
			pobTran->inErrorMsg = _ERROR_CODE_V3_FISC_12_SEND_APDU_FAIL_;
			inRetVal = VS_ERROR;
		}
	}

	/* Write Records失敗不需塞資料 */
	if (inRetVal != VS_SUCCESS)
	{
                if (ginDebug == VS_TRUE)
                {
                        sprintf(szDebugMSG,"inNCCC_FISC_ReadRecords()_END");
                        inLogPrintf(AT, szDebugMSG);
                }
                
		return (inRetVal);
	}

	inCnt = 0;
	/* 金融卡交易序號長度 */
	pobTran->srBRec.inFiscSTANLength = srAPDUData.uszRecevData[inCnt];
	inCnt ++;
	/* 金融卡交易序號 */
	memcpy(pobTran->srBRec.szFiscSTAN, &srAPDUData.uszRecevData[inCnt], pobTran->srBRec.inFiscSTANLength);
	inCnt += 8;
	/* 金融卡交易授權驗證碼長度 */
	memcpy(pobTran->srBRec.szFiscTacLength,(char*)&srAPDUData.uszRecevData[inCnt], 2);
	memset(szTemplate, 0x00, sizeof(szTemplate));
        inFunc_BCD_to_ASCII(szTemplate, (unsigned char *)pobTran->srBRec.szFiscTacLength, 2);
	inTacLen = atoi(szTemplate);
	inCnt += 2;
	/* 金融卡交易授權驗證碼 */
	memcpy(pobTran->srBRec.szFiscTac, &srAPDUData.uszRecevData[inCnt], inTacLen);
	inCnt += 8;

	/* Debug */
        if (ginDebug == VS_TRUE)
        {
                inFunc_BCD_to_ASCII(&szDebugMSG[0], &srAPDUData.uszRecevData[0], srAPDUData.inRecevLen);
                inLogPrintf(AT, szDebugMSG);
                sprintf(szDebugMSG,"[Write Records Debug]");
                inLogPrintf(AT, szDebugMSG);
                sprintf(szDebugMSG,"pobTran->srBRec.szFiscSTAN : [%s]", pobTran->srBRec.szFiscSTAN);
                inLogPrintf(AT, szDebugMSG);
                sprintf(szDebugMSG,"pobTran->srBRec.szFiscTac : [%s]", pobTran->srBRec.szFiscTac);
                inLogPrintf(AT, szDebugMSG);
                inLogPrintf(AT, "inFISC_WriteRecords()_END");
        }

	return (inRetVal);
}

/*
Function        :inFISC_WriteTxnAuthReqres
Date&Time       :2017/9/21 上午 10:59
Describe	:在行動裝置上的安全元件寫入交易授權請求相關資料或授權回應相關資料
*/
int inFISC_WriteTxnAuthReqres(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_SUCCESS;
        int		inCnt, inTacLen;
	char		szSendData[_WRITE_RECORD_SIZE_ + 1];
	char		szMID[10 + 1];
	char		szTemplate[64 + 1];
        char		szDebugMSG[100 + 1];
        char		szMCCCode[4 + 1]; 
        char		szMerchantID[15 + 1];
        char		szTerminalID[8 + 1];
	char		szIssuerID[5 + 1];
	char		szLRC = 0x00;
	APDU_COMMAND	srAPDUData;
        RTC_NEXSYS	srRTC;
	
        if (ginDebug == VS_TRUE)
        {
                sprintf(szDebugMSG,"inFISC_WriteTxnAuthReqres()_START_inCode(%d)", pobTran->srBRec.inCode);
                inLogPrintf(AT, szDebugMSG);
        }

	if (inLoadMVTRec(0) < 0)
	{
                if (ginDebug == VS_TRUE)
                {
                        sprintf(szDebugMSG,"inLoadMVTRec(0) ERROR");
                        inLogPrintf(AT, szDebugMSG);
                }
                
		return (VS_ERROR);
	}

	if (inLoadHDTRec(0) < 0)
	{
                if (ginDebug == VS_TRUE)
                {
                        sprintf(szDebugMSG,"inLoadHDTRec(0) ERROR");
                        inLogPrintf(AT, szDebugMSG);
                }
                
		return (VS_ERROR);
	}
        
	inCnt = 0;
	inTacLen = 0;
	memset(szSendData, 0x00, sizeof(szSendData));
	memset(szTemplate, 0x00, sizeof(szTemplate));
        
	memset(szMID, 0x00, sizeof(szMID));
        memset(szMerchantID, 0x00, sizeof(szMerchantID));
        inGetMerchantID(szMerchantID);
        memcpy(szMID, szMerchantID, 10);
	
        memset(szMCCCode, 0x00, sizeof(szMCCCode));
        inGetMCCCode(szMCCCode);
	
	memset(szIssuerID, 0x00, sizeof(szIssuerID));
	inGetIssuerID(szIssuerID);

	/* MCC 由 EMVDef.txt第一組的參數的MCC(下TMS時同步到EDC.dat上) */
	memcpy(&pobTran->srBRec.szFiscMCC[0], &szMCCCode[0], 4);

	/* 設卡片交易時間 */
        memset(&srRTC, 0x00, sizeof(CTOS_RTC));
        /* 取得EDC時間日期 */
        if (inFunc_GetSystemDateAndTime(&srRTC) != VS_SUCCESS)
        {
                if (ginDebug == VS_TRUE)
                {
                        sprintf(szDebugMSG,"inFunc_GetDateAndTime ERROR");
                        inLogPrintf(AT, szDebugMSG);
                }
                /* 取得時間失敗 */
		pobTran->inErrorMsg = _ERROR_CODE_V3_FISC_03_TIME_ERROR_;
                
                return (VS_ERROR);
        }

        memset(&pobTran->srBRec.szDate, 0x00, sizeof(pobTran->srBRec.szDate));
        memset(&pobTran->srBRec.szTime, 0x00, sizeof(pobTran->srBRec.szTime));
        sprintf(pobTran->srBRec.szDate, "20%02d%02d%02d",  srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
        sprintf(pobTran->srBRec.szTime, "%02d%02d%02d",  srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
	sprintf(pobTran->srBRec.szFiscDateTime, "%s%s", pobTran->srBRec.szDate, pobTran->srBRec.szTime);

	/* 產生端末設備查核碼 */
        if (ginDebug == VS_TRUE)
        {
                sprintf(szDebugMSG,"TCC Start");
                inLogPrintf(AT, szDebugMSG);
        }
        
	if (inFISC_GenMACAndTCC(pobTran) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
                {
                        sprintf(szDebugMSG, "inFISC_GenMAC Error");
                        inLogPrintf(AT, szDebugMSG);
                }

                /* 產生MAC及TAC失敗 */
		pobTran->inErrorMsg = _ERROR_CODE_V3_FISC_04_MAC_TAC_;
                
		return (VS_ERROR);
	}

	/* 針對不同交易別, 組Send Data */
	switch (pobTran->srBRec.inCode)
	{
		case _FISC_SALE_ :
			memcpy(&szSendData[inCnt], "2541", 4);
			inCnt += 4;
			/* 金額 */
			sprintf(&szSendData[inCnt], "%012ld00", pobTran->srBRec.lnTxnAmount);
			inCnt += 14;
			/* TID */
                        memset(szTerminalID, 0x00, sizeof(szTerminalID));
                        inGetTerminalID(szTerminalID);
			memcpy(&szSendData[inCnt], szTerminalID, 8);
			inCnt += 8;
			/* 端末機檢核碼 */
			memcpy(&szSendData[inCnt], &pobTran->srBRec.szFiscTCC[0], 8);
			inCnt += 8;
			/* 交易時間 YYYYMMDDHHMMSS */
			sprintf(&szSendData[inCnt], "%s", pobTran->srBRec.szFiscDateTime);
			inCnt += 14;
			/* MID */
			/* 聯合的最前面要放金融機構末三碼 */
			sprintf(&szSendData[inCnt], "%s     ", szMID);
			inCnt += 15;
			/* MCC */
			sprintf(&szSendData[inCnt], "%s", szMCCCode);
			inCnt += 4;
			/* 金融機構代碼 */
			/* 合庫金融機構代碼為0060 聯合為0956 */
			sprintf(&szSendData[inCnt], szIssuerID);
			inCnt += 4;
			memcpy(&szSendData[inCnt], "       ", 7);
			inCnt += 7;
			/* 金融卡帳號 */
			memcpy(&szSendData[inCnt], pobTran->srBRec.szFiscAccount, _FISC_ACCOUNT_SIZE_);
			inCnt += _FISC_ACCOUNT_SIZE_;
			
			/* 財金提示需先在此先組lrc並存起來,之後要比對 */
                        szLRC = 0x00;
			inFunc_Check_LRC(szSendData, inCnt, &szLRC);
			
			if (ginDisplayDebug == VS_TRUE)
			{
				memset(szDebugMSG, 0x00, sizeof(szDebugMSG));
				sprintf(szDebugMSG, "LRC For NP: %X", szLRC);
				inDISP_LOGDisplay(szDebugMSG, _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
			}
			break;
		default :
			
                        if (ginDebug == VS_TRUE)
                        {
                                sprintf(szDebugMSG,"inFISC_WriteRecords()_inCode_ERR");
                                inLogPrintf(AT, szDebugMSG);
                        }
                        
                        /* 無此交易別 */
                        pobTran->inErrorMsg = _ERROR_CODE_V3_FISC_05_NO_INCODE_;
                        
			return(VS_ERROR);
	}

	/* 開始塞資料 */
	memset(&srAPDUData, 0x00, sizeof(srAPDUData));
	srAPDUData.uszCommandCLAData[0] = _FISC_WRITE_AUTH_RECORD_CLA_COMMAND_;		/* CLA */
	srAPDUData.uszCommandINSData[0] = _FISC_WRITE_AUTH_RECORD_INS_COMMAND_;		/* INS */
	srAPDUData.uszCommandP1Data[0] = _FISC_WRITE_AUTH_RECORD_P1_COMMAND_;		/* P1 */
	srAPDUData.uszCommandP2Data[0] = _FISC_WRITE_AUTH_RECORD_P2_COMMAND_;		/* P2 */
	
	memcpy(&srAPDUData.uszCommandData[0], &szSendData[0], sizeof(szSendData)); /* Data */
	srAPDUData.inCommandDataLen = inCnt; /* Data length */

        if (ginDebug == VS_TRUE)
        {
                sprintf(szDebugMSG,"[srAPDUData]");
                inLogPrintf(AT, szDebugMSG);
		sprintf(szDebugMSG,"srAPDUData.uszCLAData = [%02x]", srAPDUData.uszCommandCLAData[0]);
                inLogPrintf(AT, szDebugMSG);
                sprintf(szDebugMSG,"srAPDUData.uszINSData = [%02x]", srAPDUData.uszCommandINSData[0]);
                inLogPrintf(AT, szDebugMSG);
                sprintf(szDebugMSG,"srAPDUData.uszP1Data = [%02x]", srAPDUData.uszCommandP1Data[0]);
                inLogPrintf(AT, szDebugMSG);
                sprintf(szDebugMSG,"srAPDUData.uszP2Data = [%02x]", srAPDUData.uszCommandP2Data[0]);
                inLogPrintf(AT, szDebugMSG);
                sprintf(szDebugMSG,"srAPDUData.inDataLen = [%d]", srAPDUData.inCommandDataLen);
                inLogPrintf(AT, szDebugMSG);
        }

	if (inAPDU_BuildAPDU(&srAPDUData) == VS_SUCCESS)
	{
		inRetVal = inAPDU_APDUTransmit_Flow(pobTran, &srAPDUData);
		if (inRetVal == VS_SUCCESS)
		{
			/* 成功的狀況 */
			if (!memcmp(&srAPDUData.uszRecevData[srAPDUData.inRecevLen - 2],"\x90\x00",2))
			{
				inRetVal = VS_SUCCESS;
			}
			else
			{	
				if (!memcmp(&srAPDUData.uszRecevData[srAPDUData.inRecevLen - 2],"\x69\x82",2))
				{
					pobTran->inErrorMsg = _ERROR_CODE_V3_FISC_02_6982_;
				}
				else
				{
					pobTran->inErrorMsg = _ERROR_CODE_V3_FISC_01_READ_FAIL_;
				}

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMSG, 0x00, sizeof(szDebugMSG));
					sprintf(szDebugMSG,"inFISC_APDUTransmit()_ERROR");
					inLogPrintf(AT, szDebugMSG);
				}
				inRetVal = VS_ERROR;
			}
		}
		else
		{
			/* SEND APDU FAIL */
			pobTran->inErrorMsg = _ERROR_CODE_V3_FISC_12_SEND_APDU_FAIL_;
			inRetVal = VS_ERROR;
		}
	}

	/* Write Records失敗不需塞資料 */
	if (inRetVal != VS_SUCCESS)
	{
                if (ginDebug == VS_TRUE)
                {
                        sprintf(szDebugMSG,"inNCCC_FISC_ReadRecords()_END");
                        inLogPrintf(AT, szDebugMSG);
                }
                
		return (inRetVal);
	}

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFISC_WriteTxnAuthReqres() END !");
		inLogPrintf(AT, "----------------------------------------");
	}

	return (inRetVal);
}

/*
 .App Name      : inFISC_CheckAccountData
 .App Date&Time : 2016/11/4 上午 10:13
 .App Function  : 檢查傳進來的值是否符合帳號規則
 .Input Param   : char *szAccountData, int inCheckLen
 .Output Param  : 1. 成功. -> VS_SUCCESS
	        	  2. 失敗. -> VS_ERROR
*/
int inFISC_CheckAccountData(char *szAccountData, int inCheckLen)
{
	int	inRetVal, inInitialValue, inCheckDataLen;

	inInitialValue = 0;
	inCheckDataLen = 0;

	while (1)
	{
		if (szAccountData[inCheckDataLen] < 0x30 && szAccountData[inCheckDataLen] > 0x39)
		{
			if (szAccountData[inCheckDataLen] == 0x20 || szAccountData[inCheckDataLen] == 0x00)
				inInitialValue ++;
			else
			{
				inRetVal = VS_ERROR;
				break;
			}
		}
		else
		{
			if(szAccountData[inCheckDataLen] == 0x30)
				inInitialValue ++;
		}

		/*
		    因為卡帳號如果沒有那初始值會是空白或0, 所以只要傳入資料是空白或0,
		    就表示帳號未啟用 (inCheckLen-1)是因為傳入值不計算第0個位置
		*/
		if (inInitialValue == (inCheckLen - 1))
		{
			inRetVal = VS_ESCAPE;
			break;
		}
		if (inCheckDataLen == (inCheckLen - 1))
		{
			inRetVal = VS_SUCCESS;
			break;
		}
		else
			inCheckDataLen ++;

	} /* end of while */

	return (inRetVal);
}

/*
 .App Name      : inFISC_EnterPin
 .App Date&Time : 2016/11/4 上午 11:07
 .App Function  : 輸入金融卡Pin
 .Input Param   : szPinData
 .Output Param  : 1. 成功. -> VS_SUCCESS
	          2. 失敗. -> VS_ERROR
*/
int inFISC_EnterPin(char *szPinData)
{
        int		inRetVal;
        DISPLAY_OBJECT  srDispObj;
        
	while (1)
	{
                inDISP_BEEP(1, 0);
		inDISP_Clear_Line(_LINE_8_8_, _LINE_8_8_);
		memset(&srDispObj, 0x00, DISPLAY_OBJECT_SIZE);

                /* 設定顯示變數 */
                srDispObj.inMaxLen = _FISC_PIN_SIZE_;
                srDispObj.inY = _LINE_8_8_;
                srDispObj.inR_L = _DISP_RIGHT_;
                srDispObj.inMask = VS_TRUE;
		srDispObj.inCanNotBypass = VS_TRUE;
		srDispObj.inColor = _COLOR_RED_;
		
		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;
	
                inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);
		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (inRetVal);

		if (srDispObj.inOutputLen > 0)
		{
                        memset(szPinData, 0x00, sizeof(szPinData));
                        memcpy(&szPinData[0], &srDispObj.szOutput[0], srDispObj.inOutputLen);
                        break;
		}
                else
                        continue;
	}

	return (VS_SUCCESS);
}

/*
Function        :inFISC_GenMACAndTCC
Date&Time       :2016/11/23 上午 9:42
Describe        :消費扣款和退費要多做算TCC的動作
*/
int inFISC_GenMACAndTCC(TRANSACTION_OBJECT *pobTran)
{
	int 	inRetVal = 0, i = 0, inCheckLen = 0;
	char	szTemplate[128 + 1] = {0};
	char	szTerminalCheckCode[8 + 1] = {0};
	char	szDebugMSG[100 + 1] = {0};
	char	szDemoMode[2 + 1] = {0};
	
        if (ginDebug == VS_TRUE)
        {
                sprintf(szDebugMSG,"inFISC_GenMACAndTCC START!!");
                inLogPrintf(AT, szDebugMSG);
        }
        
        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        inDISP_PutGraphic(_PROCESS_, 0, _COORDINATE_Y_LINE_8_7_); /* 處理中‧‧‧‧‧ */

        /* 教育訓練模式 */
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
                memset(pobTran->srBRec.szFiscTCC, 0x00, sizeof(pobTran->srBRec.szFiscTCC));
                memcpy(pobTran->srBRec.szFiscTCC, "12345678", 8);
        }
	else
	{
                inRetVal = inNCCC_Func_FISC_GenMAC(pobTran);
		
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
        
        	/*
        	   以下產生Smart Pay Field_58 Tag S4 端末設備查核碼
        	   數字部分超過8 位數，則取前8 位數當作端末設備檢核碼。
        	   數字部分不足8 位數，則右補’0’至8 位數當作端末設備檢核碼。
        	*/
        	inCheckLen = 0;
        	memset(szTerminalCheckCode, 0x00, sizeof(szTerminalCheckCode));
        	memset(szTemplate, 0x00, sizeof(szTemplate));
                inFunc_BCD_to_ASCII(szTemplate, (unsigned char *)pobTran->szMAC_HEX, 8);
        
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, szTemplate);
        
        	for (i = 0; i < 16; i ++)
        	{
        		if (inCheckLen > 7)
        			break;
        
        		if (szTemplate[i] >= '0' && szTemplate[i] <= '9')
        		{
        			szTerminalCheckCode[inCheckLen] = szTemplate[i];
        			inCheckLen ++;
        		}
        
        	}
        
                inFunc_PAD_ASCII(szTerminalCheckCode, szTerminalCheckCode, '0', 8, _DISP_LEFT_);

                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, szTerminalCheckCode);
                
        	memcpy(pobTran->srBRec.szFiscTCC, szTerminalCheckCode, sizeof(szTerminalCheckCode));
        }

        if (ginDebug == VS_TRUE)
        {
                sprintf(szDebugMSG,"inFISC_GenMACAndTCC END!!");
                inLogPrintf(AT, szDebugMSG);
        }
                
        return (VS_SUCCESS);
}

/* 用來確認SmartPay功能是否可使用 */
int inFISC_CheckSmartPayEnable()
{
        char    szSPAID[16 + 1];
	char    szSmartPayContactlessEnable[2 + 1];
	char	szFiscFunctionEnable[2 + 1];
	char	szDebugMsg[100 + 1];

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inFISC_CheckSmartPayEnable START!!");
	
	/* 1.若EDC的FiscFunctionEnable未開，顯示此功能以關閉 */
	memset(szFiscFunctionEnable, 0x00, sizeof(szFiscFunctionEnable));
	inGetFiscFuncEnable(szFiscFunctionEnable);

	/* 沒開Fisc */
	if ((memcmp(szFiscFunctionEnable, "Y", 1) != 0))
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "EDC.dat Fisc開關沒開");
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	
	/* 2.檢查CFGT是否支援SmartPay感應 */
        memset(szSmartPayContactlessEnable, 0x00, sizeof(szSmartPayContactlessEnable));
        inGetSmartPayContactlessEnable(szSmartPayContactlessEnable);
	
	if (memcmp(szSmartPayContactlessEnable, "Y", 1) != 0)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CFGT.dat Fisc開關沒開");
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
        
	/* 3.檢查SMARTPAY AID*/
        if (inLoadSPAYRec(0) < 0)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inLoadSPAYRec ERROR!!");
                        
                return (VS_ERROR);
        }
        
        memset(szSPAID, 0x00, sizeof(szSPAID));
        
        if (inGetSPAID(szSPAID) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }
            
        if (!memcmp(szSPAID, "                ", 16))
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "szSPAID ERROR!!");
        
                return (VS_ERROR);
        }
        
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inFISC_CheckSmartPayEnable END!!");
        
        return (VS_SUCCESS);
}

/* 感應使用 SmartPay Select AID Call Back Function 用 */
int inFISC_SelectAID_CTLS(TRANSACTION_OBJECT * pobTran)
{
	char		szSPAID[16 + 1];
	char		szSPAIDHex[8 + 1];
	char		szDebugMsg[100 + 1];
        APDU_COMMAND	srAPDUData;
	
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inFISC_SelectAID_CTLS()_START");
        }
	
	if (inLoadSPAYRec(0) < 0)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inLoadSPAYRec ERROR!!");
                        
                return (VS_ERROR);
        }
	
	memset((char *)&srAPDUData, 0x00, sizeof(APDU_COMMAND));
	memset(szSPAID, 0x00, sizeof(szSPAID));
	inGetSPAID(szSPAID);
	memset(szSPAIDHex, 0x00, sizeof(szSPAIDHex));
	inFunc_ASCII_to_BCD((unsigned char*)szSPAIDHex, szSPAID, 8);
	if (ginDisplayDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "SPAID:%s", szSPAID);
		inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
	}
	
	srAPDUData.uszCommandCLAData[0] = _FISC_SELECT_AID_CLA_COMMAND_;	/* CLA */
	srAPDUData.uszCommandINSData[0] = _FISC_SELECT_AID_INS_COMMAND_;	/* INS */
	srAPDUData.uszCommandP1Data[0] = _FISC_SELECT_AID_P1_COMMAND_;		/* P1 */
	srAPDUData.uszCommandP2Data[0] = _FISC_SELECT_AID_P2_COMMAND_;		/* P2 */
	memcpy((char*)&srAPDUData.uszCommandData[0], szSPAIDHex, sizeof(szSPAIDHex));
	srAPDUData.inCommandDataLen = 8;
	
	if (inAPDU_BuildAPDU(&srAPDUData) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
        
	
        /* 收送Command */
	/* 這邊直接用inFISC_APDUTransmit_CTLS的原因是，Contactless的Bit還沒On，
	   要SelectAID成功之後，才能確定是Contactless */
        if (inAPDU_Send_APDU_CTLS_Process(&srAPDUData) == VS_SUCCESS)
        {
                if (!(srAPDUData.uszRecevData[srAPDUData.inRecevLen -2] == 0x90 && srAPDUData.uszRecevData[srAPDUData.inRecevLen -1] == 0x00))
                {
                        if (ginDebug == VS_TRUE)
                                inLogPrintf(AT, "FISC Select AID != 9000");
                        
                        return (VS_ERROR);
                }
                else
                {
                        if (ginDebug == VS_TRUE)
                                inLogPrintf(AT, "FISC = TRUE");
                        
                        /* 確認為金融卡 */
                        pobTran->uszFISCBit = VS_TRUE;
                }
        }
        else
        {
                /* 下Command失敗 */
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "FISC APDU Command Error");
                        
                return (VS_ERROR);
        }

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inFISC_SelectAID_CTLS()_END");
        }

        return (VS_SUCCESS);
}

/* 變換燈號及聲響用 */
int inFISC_CTLS_LED_TONE(int inResult)
{
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inFISC_CTLS_LED_TONE()");
            
        switch (inResult)
        {
                case VS_SUCCESS :
                        inCTLS_Set_LED(_CTLS_LIGHT_GREEN_);
			CTOS_Sound(2700, 20);
                        inDISP_Wait(1000);      /* 停個一秒 */
                    break;
                case VS_ERROR :
                        inCTLS_Set_LED(_CTLS_LIGHT_RED_);
			CTOS_Sound(750, 20);
			CTOS_Delay(200);
			CTOS_Sound(750, 20);
                    break;
                default :
                    return (VS_ERROR);
        }
        
        return (VS_SUCCESS);
}

/*
Function        :inFISC_CTLSProcess
Date&Time       :2017/7/6 下午 4:30
Describe        :SmartPay感應流程跑這隻，從call back function vdCTLS_EVENT_EMVCL_NON_EMV_CARD 呼叫
*/
int inFISC_CTLSProcess(TRANSACTION_OBJECT *pobTran)
{
	int     inRetVal = VS_ERROR;
        long    lnAmountTXNLimit;
	char    szDispMsg[128 + 1];
        char    szTemplate[100 + 1];
	char	szDebugMsg[100 + 1];
        char    szSPCVMRequireLimit[12 + 1]; /* SmartPay感應限額 */

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inFISC_CTLSProcess()_START");
        }
	
	/* 各種SmartPay檢核 不支援SmartPay感應直接擋下 */
	if (inFISC_CheckSmartPayEnable() != VS_SUCCESS)
	{
		 if (ginDebug == VS_TRUE)
			 inLogPrintf(AT, "SmartPay CTLS No Surport!");

		 /* 不接受此感應卡 請改插卡 */
		 pobTran->inErrorMsg = _ERROR_CODE_V3_FISC_06_TMS_NOT_SUPPORT_;
		 pobTran->srBRec.inTxnResult = VS_ERROR;

		 return (VS_ERROR);
	}
	
	/* SELECT AID FAILED */
	/* 這邊還要select一次是因為有可能按Smartpay，但是用非金融卡感應，但是不能在外層判斷會變成直接走信用卡*/
	if (inFISC_SelectAID_CTLS(pobTran) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			 inLogPrintf(AT, "SmartPay CTLS SELECT AID FAILED!");
		}
		
		/* 請改插金融卡 */
		pobTran->inErrorMsg = _ERROR_CODE_V3_FISC_11_FISC_FALLBACK_;
		pobTran->srBRec.inTxnResult = VS_ERROR;
		 
		return (VS_ERROR);
	}
	
	/* IDLE進入，一開始預設是_SALE_ */
	if (pobTran->inTransactionCode == _SALE_)
	{
		pobTran->inTransactionCode = _FISC_SALE_;
		pobTran->srBRec.inCode = pobTran->inTransactionCode;
		pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
	}

	/* NCCC SMARTPAY同一個HOST */
	pobTran->srBRec.inHDTIndex = 0;
	inLoadHDTRec(pobTran->srBRec.inHDTIndex);
	inLoadHDPTRec(pobTran->srBRec.inHDTIndex);

	pobTran->srBRec.uszContactlessBit = VS_TRUE;
	pobTran->srBRec.uszFiscTransBit = VS_TRUE;
	pobTran->uszFISCBit = VS_TRUE;

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
			pobTran->inErrorMsg = _ERROR_CODE_V3_FISC_10_LOGON_FAIL_;
			pobTran->srBRec.inTxnResult = VS_ERROR;
			
			return (VS_ERROR);
		}
		
		/* 確認是那一種狀況 */
		if (inFISC_SelectAID_CTLS(pobTran) == VS_SUCCESS)
		{
			/* 狀況一，poweron時間很短，卡片還沒斷電，直接做 */
		}
		else
		{
			/* 狀況二，重新把卡片上電到成功才繼續做 */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_ERR_PLS_PUT_BACK_CTLS_, 0, _COORDINATE_Y_LINE_8_6_);
			while (1)
			{
				/* 沿用感應的Timeout */
				if (inDISP_TimeoutCheck(_FONTSIZE_8X16_, _LINE_8_6_, _DISP_RIGHT_) == VS_TIMEOUT)
				{	
					pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TIMEOUT_;
					pobTran->inErrorMsg = _ERROR_CODE_V3_FISC_03_TIME_ERROR_;
					pobTran->srBRec.inTxnResult = VS_ERROR;
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					return (VS_ERROR);
				}
				
				if (inCTLS_ReActive_TypeACard()== VS_SUCCESS)
				{
					break;
				}
			}
		}
	}
	
	if (inNCCC_Func_Decide_CTLS_TRT(pobTran) != VS_SUCCESS)
	{
                /* Mirror Message */
                if (pobTran->uszECRBit == VS_TRUE)
                {
                        inECR_SendMirror(pobTran, _MIRROR_MSG_WRONG_CARD_ERROR_);
                }
                                                
		/* 請依正確卡別操作 */
		pobTran->inErrorMsg = _ERROR_CODE_V3_FISC_09_NOT_RIGHT_INCODE_;
		pobTran->srBRec.inTxnResult = VS_ERROR;
		
		return (VS_ERROR);
	}
        
        /* 檢核SmartPay感應限額 */
        memset(szSPCVMRequireLimit, 0x00, sizeof(szSPCVMRequireLimit));
        memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetSPCVMRequireLimit(szTemplate);
        memcpy(&szSPCVMRequireLimit[0], &szTemplate[0], 10);
        
        lnAmountTXNLimit = atol(szSPCVMRequireLimit);
        
        if (pobTran->srBRec.lnTxnAmount >= lnAmountTXNLimit)
        {
                /* 超過感應限額 請改插卡 */
		pobTran->inErrorMsg = _ERROR_CODE_V3_FISC_07_AMT_OVERLIMIT_;
                pobTran->srBRec.inTxnResult = VS_ERROR;
		
                return (VS_ERROR);
        }
	
        if (pobTran->uszCardInquirysSecondBit == VS_TRUE)
        {
                
        }
        else
        {
                if (pobTran->inRunTRTID == _TRT_FISC_REFUND_CTLS_)
                {
                        /* 顯示請插金融卡或感應 */
                        inDISP_PutGraphic(_MENU_FISC_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_2_);	/* 第二層顯示 ＜退費交易＞ */
                        inDISP_PutGraphic(_CTLS_FISCICC_RF_, 0, _COORDINATE_Y_LINE_8_3_);		/* 顯示請插金融卡或感應 */
                }
                else
                {
                        /* 顯示 消費扣款標題及處理中 */
                        inDISP_PutGraphic(_MENU_FISC_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_2_);		/* 第二層顯示 ＜消費扣款＞ */
                        inDISP_PutGraphic(_CTLS_PROCESSING_, 0, _COORDINATE_Y_LINE_8_3_);		/* 顯示處理中 */
                }
	
                /* 回復顯示金額 */
                memset(szDispMsg, 0x00, sizeof(szDispMsg));
                sprintf(szDispMsg, "%ld",  pobTran->srBRec.lnTxnAmount);
                inFunc_Amount_Comma(szDispMsg, "NT$ " , ' ', _SIGNED_NONE_, 16, VS_TRUE);
                inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_3_, _COLOR_RED_, _DISP_LEFT_);
        }
        
	/* 若1004Select成功，則為手機金融卡 */
	inRetVal = inFISC_Check_Mobile(pobTran);
	
	if (inRetVal == VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "手機金融卡 1004");
			inLogPrintf(AT, szDebugMsg);
		}
		
		memset(pobTran->srBRec.szFiscPayDevice, 0x00, sizeof(pobTran->srBRec.szFiscPayDevice));
		memcpy(pobTran->srBRec.szFiscPayDevice, _FISC_PAY_DEVICE_MOBILE_, strlen(_FISC_PAY_DEVICE_MOBILE_));
		
		/* 判別是否要輸入密碼 */
		inRetVal = inFISC_ReadRecords_1004(pobTran);
		if (inRetVal != VS_SUCCESS)
		{
			/* 顯示讀卡失敗 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_FISC_01_READ_FAIL_;			
			pobTran->srBRec.inTxnResult = VS_ERROR;
			return (inRetVal);
		}
		
		/* 0x02 大額流程，需要輸入密碼 - 沒測過 */
		if (memcmp(pobTran->srBRec.szFiscMobileNFType, _FISC_MOBILE_NFTYPE_MANDATORY_ENTER_PIN_, strlen(_FISC_MOBILE_NFTYPE_MANDATORY_ENTER_PIN_)) == 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "手機金融卡 1004 MustPIN:02");
				inLogPrintf(AT, szDebugMsg);
			}
			
			inRetVal = inFISC_Flow_MobileHighValuePayment(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				pobTran->srBRec.inTxnResult = VS_ERROR;
				return (inRetVal);
			}
		}
		/* 0x01 依情況選擇，目前都跑小額流程 */
		else if (memcmp(pobTran->srBRec.szFiscMobileNFType, _FISC_MOBILE_NFTYPE_CONDITION_ENTER_PIN_, strlen(_FISC_MOBILE_NFTYPE_CONDITION_ENTER_PIN_)) == 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "手機金融卡 1004 Condition:01");
				inLogPrintf(AT, szDebugMsg);
			}
			
			inRetVal = inFISC_Flow_MobileMicroPayment(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				pobTran->srBRec.inTxnResult = VS_ERROR;
				return (inRetVal);
			}
		}
		/* 目前只加不需密碼的流程 */
		else if (memcmp(pobTran->srBRec.szFiscMobileNFType, _FISC_MOBILE_NFTYPE_WITHOUT_ENTER_PIN_, strlen(_FISC_MOBILE_NFTYPE_WITHOUT_ENTER_PIN_)) == 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "手機金融卡 1004 NoPIN:00");
				inLogPrintf(AT, szDebugMsg);
			}
			
			inRetVal = inFISC_Flow_MobileMicroPayment(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				pobTran->srBRec.inTxnResult = VS_ERROR;
				return (inRetVal);
			}
		}
		else
		{
			/* 例外狀況 */
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "FISC MOBILE EXCEPTION");
			}
			inRetVal = VS_ERROR;
			
			return (inRetVal);
		}
		
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "非手機金融卡");
			inLogPrintf(AT, szDebugMsg);
		}
		
		memset(pobTran->srBRec.szFiscPayDevice, 0x00, sizeof(pobTran->srBRec.szFiscPayDevice));
		memcpy(pobTran->srBRec.szFiscPayDevice, _FISC_PAY_DEVICE_SMARTPAY_CARD_, strlen(_FISC_PAY_DEVICE_SMARTPAY_CARD_));
		/* 金融卡實體卡流程 */
		inRetVal = inFISC_Flow_SmartPayPayment(pobTran);
		if (inRetVal != VS_SUCCESS)
		{
			/* 在裡面流程會設定 */
			pobTran->srBRec.inTxnResult = VS_ERROR;
			
			return (VS_ERROR);
		}

	}
        
	/* 讀卡號範圍 Load CDT HDT HDPT */
	if (inCARD_GetBin(pobTran) != VS_SUCCESS)
	{
		/* 找不到CardBin */
		pobTran->inErrorMsg = _ERROR_CODE_V3_FISC_08_NO_CARD_BIN_;
		pobTran->srBRec.inTxnResult = VS_ERROR;
		
		return (VS_ERROR);
	}

        if (pobTran->uszCardInquirysSecondBit == VS_TRUE)
        {
                
        }
        else
        {
            /* 亮成功的綠燈 */
            inFISC_CTLS_LED_TONE(VS_SUCCESS);
        }
        
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inNCCC_FISC_ProcessFISCCard()_END");
        }
        
	return (VS_SUCCESS);
}

/*
 .App Name      : inFISC_Check_Mobile
 .App Date&Time : 2016/12/1 下午 1:39
 .App Function  : 確認是否是手機金融卡
 .Input Param   : TRANSACTION_OBJECT *pobTran
 .Output Param  : 1. 成功. -> VS_SUCCESS
	          2. 失敗. -> VS_ERROR
*/
int inFISC_Check_Mobile(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_ERROR;
	char		szBuffer[_SELECT_EF_SIZE_ + 1];
	APDU_COMMAND	srAPDUData;

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inFISC_Check_Mobile()_START");
        }

	inRetVal = VS_ERROR;

	/* Select EF */
	memset(&szBuffer[0], 0x00, sizeof(szBuffer));

	/* 若1004Select成功，則為手機金融卡 */
	pobTran->inEFID = 1004;
	
	switch(pobTran->inEFID)
	{
	        /* EF(Elementary File) */
		case 1001 :
			memcpy(&szBuffer[0],"\x10\x01", _SELECT_EF_SIZE_);
			break;
		case 1003 :
			memcpy(&szBuffer[0],"\x10\x03", _SELECT_EF_SIZE_);
			break;
		case 1004 :
			memcpy(&szBuffer[0],"\x10\x04", _SELECT_EF_SIZE_);
			break;
		default :
			memcpy(&szBuffer[0],"\x10\x03", _SELECT_EF_SIZE_);
			break;
	}

	/* 開始塞資料 */
	srAPDUData.uszCommandCLAData[0] = _FISC_SELECT_EF_CLA_COMMAND_;		/* CLA */
	srAPDUData.uszCommandINSData[0] = _FISC_SELECT_EF_INS_COMMAND_;		/* INS */
	srAPDUData.uszCommandP1Data[0] = _FISC_SELECT_EF_P1_COMMAND_;		/* P1 */
	srAPDUData.uszCommandP2Data[0] = _FISC_SELECT_EF_P2_COMMAND_;		/* P2 */
	memcpy(&srAPDUData.uszCommandData[0], &szBuffer[0], sizeof(szBuffer));	/* Data */
	srAPDUData.inCommandDataLen = _SELECT_EF_SIZE_;				/* Data length */

	if (inAPDU_BuildAPDU(&srAPDUData) == VS_SUCCESS)
	{
		inRetVal = inAPDU_APDUTransmit_Flow(pobTran, &srAPDUData);
		if (inRetVal == VS_SUCCESS)
		{
			/* 成功的狀況 */
			if (!memcmp(&srAPDUData.uszRecevData[srAPDUData.inRecevLen - 2],"\x90\x00",2))
			{
				inRetVal = VS_SUCCESS;
			}
			else
			{	
				inRetVal = VS_ERROR;
			}
		}
		else
		{
			/* SEND APDU FAIL */
			pobTran->inErrorMsg = _ERROR_CODE_V3_FISC_12_SEND_APDU_FAIL_;
			inRetVal = VS_ERROR;
		}
	}

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inFISC_Check_Mobile()_END");
        }
        
	return (inRetVal);
}

/*
Function        :inFISC_Flow_SmartPayPayment
Date&Time       :2017/8/28 上午 10:21
Describe        :實體卡金融卡流程
*/
int inFISC_Flow_SmartPayPayment(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;
	
	/* 感應卡EFID 先用1003 */
	pobTran->inEFID = 1003;
	inRetVal = inFISC_CARD_SelectEF(pobTran);

	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "FISC_SELECT_EF_ERROR");
		}

		pobTran->srBRec.inTxnResult = VS_ERROR;
		return (inRetVal);
	}

	inRetVal = inFISC_ReadRecords(pobTran);

	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "FISC_READ_RECORDS_ERROR");
		}

		pobTran->srBRec.inTxnResult = VS_ERROR;
		return (inRetVal);
	}

	inRetVal = inFISC_WriteRecords(pobTran);

	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "FISC_WRITE_RECORDS_ERROR");
		}

		pobTran->srBRec.inTxnResult = VS_ERROR;
		return (inRetVal);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFISC_Flow_MobileMicroPayment
Date&Time       :2017/8/28 上午 10:37
Describe        :行動近端小額交易流程(不須密碼)
*/
int inFISC_Flow_MobileMicroPayment(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;
	/* 金融卡流程6-3,辨別是否支援輸入密碼,方式就是讀1004的第二軌,00就是代表不支援輸入密碼,但仍需辨別金額是否大於3k,超過就拒絕 */
	/* 確定支援輸入密碼之後,仍需辨別金額是否大於3k,大於就拒絕,<3000走元感應金融卡流程 */
	
	if (pobTran->srBRec.lnTxnAmount > 3000)
	{
		/* 顯示交易金額過大 */
		
		inRetVal = VS_ERROR;
	}
	else
	{
		/* 感應卡EFID 先用1003 */
		pobTran->inEFID = 1003;
		
		inRetVal = inFISC_CARD_SelectEF(pobTran);
		if (inRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "FISC_SELECT_EF_ERROR");
			}

			pobTran->srBRec.inTxnResult = VS_ERROR;
			return (inRetVal);
		}

		inRetVal = inFISC_ReadRecords(pobTran);
		if (inRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "FISC_READ_RECORDS_ERROR");
			}

			pobTran->srBRec.inTxnResult = VS_ERROR;
			return (inRetVal);
		}

		inRetVal = inFISC_WriteRecords(pobTran);
		if (inRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "FISC_WRITE_RECORDS_ERROR");
			}

			pobTran->srBRec.inTxnResult = VS_ERROR;
			return (inRetVal);
		}
	}
	
	return (inRetVal);
}

/*
Function        :inFISC_Flow_MobileConditionPayment
Date&Time       :2017/9/20 上午 11:25
Describe        :視情況是否需輸入密碼
*/
int inFISC_Flow_MobileConditionPayment(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;
	/*金融卡流程6-1,辨別是否支援輸入密碼,方式就是讀1004的第二軌,01就是可支援大額輸入密碼,在此先以回傳99分辨 */
	/*確定支援輸入密碼之後, 仍需辨別金額是否大於3k, <3000走元感應金融卡流程*/
	
	/* 大於就輸入密碼授權 */
	if (pobTran->srBRec.lnTxnAmount > 3000)
	{
		inRetVal = inFISC_WriteTxnAuthReqres(pobTran);
		if (inRetVal != VS_SUCCESS)
		{
			/* 顯示金融卡寫檔失敗 */

			return (VS_ERROR);
		}
		
		/* TAP TWO(這個動作只是確認是否重新靠卡，用select AID檢查) */
		inRetVal = inFISC_TwoTap_Wait_Second_Approach(pobTran);
		if (inRetVal != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		
		/* SELECT EF */
		pobTran->inEFID = 1003;
		inRetVal = inFISC_CARD_SelectEF(pobTran);
		if (inRetVal != VS_SUCCESS)
		{
			/* 顯示金融卡選軌失敗 */

			return (VS_ERROR);
		}

		inRetVal = inFISC_ReadRecords(pobTran);
		if (inRetVal != VS_SUCCESS)
		{
			/* 金融卡讀軌失敗 */

			pobTran->srBRec.inTxnResult = VS_ERROR;
			return (inRetVal);
		}

		/* READ_TXN_AUTH_REQRES */
		inRetVal = inFISC_ReadTxnAuthReqres(pobTran);
		if (inRetVal != VS_SUCCESS)
		{
			/* 金融卡讀檔失敗 */

			return (VS_ERROR);
		}
	}
	/* 小於3000直接寫TAC取授權 */
	else
	{
		/* 感應卡EFID 先用1003 */
		pobTran->inEFID = 1003;
		inRetVal = inFISC_CARD_SelectEF(pobTran);

		if (inRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "FISC_SELECT_EF_ERROR");
			}

			pobTran->srBRec.inTxnResult = VS_ERROR;
			return (inRetVal);
		}

		inRetVal = inFISC_ReadRecords(pobTran);

		if (inRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "FISC_READ_RECORDS_ERROR");
			}

			pobTran->srBRec.inTxnResult = VS_ERROR;
			return (inRetVal);
		}

		inRetVal = inFISC_WriteRecords(pobTran);

		if (inRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "FISC_WRITE_RECORDS_ERROR");
			}

			pobTran->srBRec.inTxnResult = VS_ERROR;
			return (inRetVal);
		}
	}
	
	return (inRetVal);
}

/*
Function	:inFISC_Flow_MobileHighValuePayment
Date&Time	:2017/9/19 下午 2:58
Describe	:行動近端大額交易流程
*/
int inFISC_Flow_MobileHighValuePayment(TRANSACTION_OBJECT *pobTran)
{
	char	szDebugMsg[100 + 1];
	int 	inRetVal = VS_ERROR;

	/*-------------------------------------------------
	iii 行動近端大額交易運作概要
	---------------------------------------------------
	1) 讀取卡片交易所需相關資料
	2) 寫入交易授權所需相關資料(WRITE_TXN_AUTH_REQRES)
	3) 讀出授權所需相關資料(READ_TXN_AUTH_REQRES)
	4) 持卡人密碼
	5) 寫入交易進入並產生交易授權碼
	6) 寫入交易授權回應資料(WRITE_TXN_AUTH_REQRES)
	7) 讀出交易授權明細紀錄(READ_TXN_AUTH_REQRES)

	ii.行動近端小額交易(須密碼)之運作同行動近端大額交易運作方式
	---------------------------------------------------*/
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "inFISC_Flow_MobileHighValuePayment()_START");
		inLogPrintf(AT, szDebugMsg);
	}

	inRetVal = inFISC_WriteTxnAuthReqres(pobTran);
	if (inRetVal != VS_SUCCESS)
	{
		/* 顯示金融卡寫檔失敗 */
		
		return (VS_ERROR);
	}

	/* TAP TWO(這個動作只是確認是否重新靠卡，用select AID檢查) */
	inRetVal = inFISC_TwoTap_Wait_Second_Approach(pobTran);
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	/* SELECT EF */
	pobTran->inEFID = 1003;
	inRetVal = inFISC_CARD_SelectEF(pobTran);
	if (inRetVal != VS_SUCCESS)
	{
		/* 顯示金融卡選軌失敗 */
		
		return (VS_ERROR);
	}

	inRetVal = inFISC_ReadRecords(pobTran);
	if (inRetVal != VS_SUCCESS)
	{
		/* 金融卡讀軌失敗 */

		pobTran->srBRec.inTxnResult = VS_ERROR;
		return (inRetVal);
	}
	
	/* READ_TXN_AUTH_REQRES */
	inRetVal = inFISC_ReadTxnAuthReqres(pobTran);
	if (inRetVal != VS_SUCCESS)
	{
		/* 金融卡讀檔失敗 */

		return (VS_ERROR);
	}

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFISC_Flow_MobileHighValuePayment() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

int inFISC_TwoTap_Wait_Second_Approach(TRANSACTION_OBJECT *pobTran)
{
	int	inMultiFunc_RetVal = VS_ERROR;
	char	szKey = 0x00;
	
	/* 請輸密碼或指紋 並再感應一次 */
	inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
	inDISP_PutGraphic(_CTLS_TWO_TAP_, 0, _COORDINATE_Y_LINE_8_7_);
			
	while (1)
	{
		if (inFISC_SelectAID_CTLS(pobTran) == VS_SUCCESS)
		{
			break;
		}
		
		/* 感應倒數時間 && Display Countdown */
		if (inDISP_TimeoutCheck(_FONTSIZE_8X16_, _LINE_8_6_, _DISP_RIGHT_) == VS_TIMEOUT)
		{
			/* 感應時間到Timeout */
			szKey = _KEY_TIMEOUT_;
		}
		
		/* ------------MasterTerminal---------------*/
		/* 偵測多接設備收到資料 */
		inMultiFunc_RetVal = inMultiFunc_First_Receive_Check();
		if (inMultiFunc_RetVal == VS_SUCCESS)
		{
			szKey = _KEY_TIMEOUT_;
		}
		
		if (szKey == _KEY_TIMEOUT_)
		{
			/* Timeout or Cancel */
			return (VS_TIMEOUT);
		}
	}
	
	return (VS_SUCCESS);
}

/*
 Function        :inFISC_Read_Card_Data_Flow
 Date&Time       :2025/2/6 下午 2:09
 Describe        :僅用來讀取金融卡資料的流程，不含交易
 */
int inFISC_Read_Card_Data_Flow(TRANSACTION_OBJECT *pobTran)
{
	int     inRetVal = VS_ERROR;
        
        vdUtility_SYSFIN_LogMessage(AT, "inFISC_Read_Card_Data_Flow START!");

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inFISC_Read_Card_Data_Flow()_START");
        }
        
        /* 晶片卡EFID 1001 */
        pobTran->inEFID = 1001;
	inRetVal = inFISC_CARD_SelectEF(pobTran);
        
	if (inRetVal != VS_SUCCESS)
	{
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "FISC_SELECT_EF_ERROR");
                }
                
		return (inRetVal);
	}

	inRetVal = inFISC_ReadRecords(pobTran);
        
	if (inRetVal != VS_SUCCESS)
	{
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "FISC_READ_RECORDS_ERROR");
                }

		return (inRetVal);
	}
        
        /* 讀卡號範圍 Load CDT HDT HDPT */
        if (inCARD_GetBin(pobTran) != VS_SUCCESS)
                return (VS_ERROR);

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inFISC_Read_Card_Data_Flow()_END");
        }
	
	return (VS_SUCCESS);
}

/*
 Function        :inFISC_Power_On_And_Select_AID
 Date&Time       :2025/2/10 下午 2:04
 Describe        :
 */
int inFISC_Power_On_And_Select_AID(TRANSACTION_OBJECT *pobTran)
{
	int     inRetVal = VS_SUCCESS;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFISC_Power_On_And_Select_AID() START !");
	}
	
	vdUtility_SYSFIN_LogMessage(AT, "inFISC_Power_On_And_Select_AID START!");
	
	inRetVal = inFISC_PowerON(pobTran);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "inFISC_Func_PowerON_ERR");

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

	inRetVal = inFISC_SelectAID(pobTran);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "inFISC_Func_SelectAID_ERR");

		if (inRetVal == VS_EMV_CARD_OUT)
		{
			/* 晶片卡被取出 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_EMV_CARD_OUT_;
			return (VS_ERROR);
		}
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFISC_Power_On_And_Select_AID() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	vdUtility_SYSFIN_LogMessage(AT, "inFISC_Power_On_And_Select_AID END!");
	
	return (inRetVal);
}