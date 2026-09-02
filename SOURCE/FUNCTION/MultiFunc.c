#include <string.h>
#include <stdio.h>
#include <ctosapi.h>
#include <stdlib.h>
#include <emv_cl.h>
#include <sqlite3.h>
#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Define_2.h"
#include "../INCLUDES/TransType.h"
#include "../INCLUDES/Transaction.h"
#include "../INCLUDES/AllStruct.h"
#include "../EVENT/Menu.h"
#include "../PRINT/Print.h"
#include "../DISPLAY/Display.h"
#include "../EVENT/MenuMsg.h"
#include "../EVENT/Flow.h"
#include "../EVENT/Event.h"
#include "Sqlite.h"
#include "APDU.h"
#include "File.h"
#include "Function.h"
#include "FuncTable.h"
#include "CDT.h"
#include "Card.h"
#include "CFGT.h"
#include "EDC.h"
#include "HDT.h"
#include "HDPT.h"
#include "KMS.h"
#include "SCDT.h"
#include "SPAY.h"
#include "VWT.h"
#include "ECR.h"
#include "../COMM/WiFi.h"
#include "RS232.h"
#include "MVT.h"
#include "Utility.h"
#include "../../NCCC/NCCCsrc.h"
#include "../../NCCC/NCCCtmk.h"
#include "../../FISC/NCCCfisc.h"
#include "../../CTLS/CTLS.h"
#include "MultiFunc.h"

extern	int			ginDebug;
extern	int			ginISODebug;
extern	int			ginDisplayDebug;
extern	int			ginEngineerDebug;
extern	int			ginFindRunTime;
extern	int			ginEventCode;
extern	CTLS_OBJECT		srCtlsObj;
extern	EMVCL_RC_DATA_EX	szRCDataEx;

int			ginMultiCTLSLen;
char			gszMultiCTLSData[1024 + 1];
MULTI_TABLE		gstMultiOb;

MULTI_TRANS_TABLE stMultiSlave[] ={
    /* 第零組標準 */
    {
        inMultiFunc_SlaveInitial,
        inMultiFunc_SlaveRecePacket,
        inMultiFunc_SlaveSendPacket,
        inMultiFunc_SlaveSendError,
        inMultiFunc_SlaveEnd
    },

};

/*
Function        :inMultiFunc_Initial
Date&Time       :22017/7/3 上午 10:22
Describe        :開機initial 第一個多接設備
*/
int inMultiFunc_First_Initial(void)
{
	int	inRetVal = VS_ERROR;
	char	szTMSOK[2 + 1];
	char	szMulti1Version[2 + 1];
	char	szComPort[4 + 1];
	
	/* 檢查是否做過【參數下載】 */
	memset(szTMSOK, 0x00, sizeof(szTMSOK));
	inGetTMSOK(szTMSOK);
	if (szTMSOK[0] != 'Y')
	{
		return (VS_SUCCESS);
	}
	
	memset(&gstMultiOb, 0x00, sizeof(MULTI_TABLE));
	/* 設定 ECR 版本 */
	memset(szMulti1Version, 0x00, sizeof(szMulti1Version));
	inGetMultiComPort1Version(szMulti1Version);
	gstMultiOb.srSetting.inVersion = atoi(szMulti1Version);
	if (ginDebug == VS_TRUE)
	{
		char szDebugMsg[100 + 1];

		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "MultiVerson :%d", gstMultiOb.srSetting.inVersion);
		inLogPrintf(AT, szDebugMsg);
	}
	
	memset(szComPort, 0x00, sizeof(szComPort));
	inGetMultiComPort1(szComPort);
	if (memcmp(szComPort, _COMPORT_COM1_, strlen(_COMPORT_COM1_)) == 0	||
	    memcmp(szComPort, _COMPORT_COM2_, strlen(_COMPORT_COM2_)) == 0	||
	    memcmp(szComPort, _COMPORT_COM3_, strlen(_COMPORT_COM3_)) == 0	||
	    memcmp(szComPort, _COMPORT_COM4_, strlen(_COMPORT_COM4_)) == 0)
	{
		inRetVal = inMultiFunc_RS232_Initial();
	}
	else if (memcmp(szComPort, _COMPORT_USB1_, strlen(_COMPORT_USB1_)) == 0)
	{
		
	}
	else if (memcmp(szComPort, _COMPORT_WIFI_, strlen(_COMPORT_WIFI_)) == 0)
	{
	}
	else
	{
		inRetVal = VS_ERROR;
	}
	
	/* 根據ECRVersion來決定，COMPORT的設定 */
	if (inRetVal != VS_SUCCESS)
	{
		inDISP_ClearAll();
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, _ERR_INIT_);
		srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
		srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
		srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
		strcpy(srDispMsgObj.szErrMsg1, "RS232");
		srDispMsgObj.inErrMsg1Line = _LINE_8_5_;
		srDispMsgObj.inBeepTimes = 1;
		srDispMsgObj.inBeepInterval = 0;
		inDISP_Msg_BMP(&srDispMsgObj);
	}
	else
	{
		/* 代表設定完成 */
		gstMultiOb.srSetting.uszSettingOK = VS_TRUE;
		if (ginDisplayDebug == VS_TRUE)
		{
			inDISP_LOGDisplay("MultiInital OK", _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
		}
		
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inMultiFunc_First_Receive_Check
Date&Time       :2017/6/30 下午 2:56
Describe        :偵測第一個多接設備
*/
int inMultiFunc_First_Receive_Check()
{
	int		inRetVal = VS_ERROR;
	char		szComPort[4 + 1];
	char		szDebugMsg[100 + 1];
	unsigned short	usReceiveLen = 0;
	
	memset(szComPort, 0x00, sizeof(szComPort));
	inGetMultiComPort1(szComPort);
	if (memcmp(szComPort, _COMPORT_COM1_, strlen(_COMPORT_COM1_)) == 0	||
	    memcmp(szComPort, _COMPORT_COM2_, strlen(_COMPORT_COM2_)) == 0	||
	    memcmp(szComPort, _COMPORT_COM3_, strlen(_COMPORT_COM3_)) == 0	||
	    memcmp(szComPort, _COMPORT_COM4_, strlen(_COMPORT_COM4_)) == 0)
	{
		/* 沒設定完成，不用檢查 */
		if (gstMultiOb.srSetting.uszSettingOK != VS_TRUE)
		{
			inRetVal = VS_ERROR;
		}
		else
		{
			inRetVal = inRS232_Data_Receive_Check(gstMultiOb.srSetting.uszComPort, &usReceiveLen);
			if (inRetVal == VS_SUCCESS)
			{
				if (ginDisplayDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "Receive Check: %d", usReceiveLen);
					inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
				}
			}
		}
	}
	else if (memcmp(szComPort, _COMPORT_USB1_, strlen(_COMPORT_USB1_)) == 0)
	{
		inRetVal = VS_ERROR;
	}
	else if (memcmp(szComPort, _COMPORT_WIFI_, strlen(_COMPORT_WIFI_)) == 0)
	{
		inRetVal = inWiFi_IsAccept();
	}
	else
	{
		inRetVal = VS_ERROR;
	}
	
	return (inRetVal);
}

/*
Function        :inMultiFunc_First_Receive_Cancel
Date&Time       :2017/7/10 下午 4:38
Describe        :偵測第一個多接設備接收到交易取消
*/
int inMultiFunc_First_Receive_Cancel()
{
	int		inRetVal = VS_ERROR;
	char		szComPort[4 + 1];
	
	memset(szComPort, 0x00, sizeof(szComPort));
	inGetMultiComPort1(szComPort);
	if (memcmp(szComPort, _COMPORT_COM1_, strlen(_COMPORT_COM1_)) == 0	||
	    memcmp(szComPort, _COMPORT_COM2_, strlen(_COMPORT_COM2_)) == 0	||
	    memcmp(szComPort, _COMPORT_COM3_, strlen(_COMPORT_COM3_)) == 0	||
	    memcmp(szComPort, _COMPORT_COM4_, strlen(_COMPORT_COM4_)) == 0)
	{
		/* 沒設定完成，不用檢查 */
		if (gstMultiOb.srSetting.uszSettingOK != VS_TRUE)
		{
			inRetVal = VS_ERROR;
		}
		else
		{
			inRetVal = inMultiFunc_RS232_Slave_GetCancel(gstMultiOb.srSetting.uszComPort, &gstMultiOb);
		}
	}
	else if (memcmp(szComPort, _COMPORT_USB1_, strlen(_COMPORT_USB1_)) == 0)
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
	
	return (inRetVal);
}

/*
Function        :inMultiFunc_RS232_Initial
Date&Time       :2017/7/3 上午 11:11
Describe        :
*/
int inMultiFunc_RS232_Initial(void)
{
	int	inRetVal = VS_ERROR;
	
	
	inRetVal = stMultiSlave[gstMultiOb.srSetting.inVersion].inMultiInitial(&gstMultiOb);
	
	return (inRetVal);
}

/*
Function        :inMultiFunc_RS232_FlushRxBuffer
Date&Time       :2017/7/3 上午 11:15
Describe        :
*/
int inMultiFunc_RS232_FlushRxBuffer(void)
{
	int	inRetVal = VS_ERROR;
	
	inRetVal = inRS232_FlushRxBuffer(gstMultiOb.srSetting.uszComPort);
	
	return (inRetVal);
}

/*
Function        :inMultiFunc_RS232_Send
Date&Time       :2017/7/4 上午 10:15
Describe        :傳送要給收銀機的資料
*/
int inMultiFunc_RS232_Send(TRANSACTION_OBJECT *pobTran, MULTI_TABLE * stMultiOb, char *szDataBuffer, int inDataSize)
{
	int		i;
	int		inRetVal;
	int		inRetry = 0;
	int		inSendLen = 0;
	char		szDebugMsg[100 +1];
	unsigned char	uszSendBuf[_ECR_RS232_BUFF_SIZE_];					/* 包含STX、ETX、LRC的電文 */
	unsigned char	uszLRC = 0;
	
	if (ginDisplayDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Send %d", inDataSize);
		inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
	}
	
	/* Send之前清Buffer，避免收到錯的回應 */
	inRS232_FlushRxBuffer(stMultiOb->srSetting.uszComPort);

	/* 將Buffer初始化 */
	memset(uszSendBuf, 0x00, sizeof(uszSendBuf));
	
	if (stMultiOb->stMulti_Optional_Setting.uszPadStxEtx == VS_TRUE)
	{
		/* 在要傳送Buffer裡放STX */
		uszSendBuf[inSendLen] = _STX_;
		inSendLen ++;
		/* 在要傳送Buffer裡放要傳送的資料 */
		memcpy(&uszSendBuf[inSendLen], szDataBuffer, inDataSize);
		inSendLen += inDataSize;
		/* 在要傳送Buffer裡放ETX */
		uszSendBuf[inSendLen] = _ETX_;
		inSendLen ++;
	}
	else
	{
		/* 在要傳送Buffer裡放要傳送的資料 */
		memcpy(&uszSendBuf[inSendLen], szDataBuffer, inDataSize);
		inSendLen += inDataSize;
	}
		
	/* 運算LRC(STX Not include) */
	for (i = 1; i < (inSendLen); i++)
	{
		uszLRC ^= uszSendBuf[i];
	}

	/* 在要傳送Buffer裡放LRC */
	uszSendBuf[inSendLen] = uszLRC;
	inSendLen ++;
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "LRC : 0x%02X", uszLRC);
		inLogPrintf(AT, szDebugMsg);
	}
	
	while (1)
	{
		/* 檢查port是否已經準備好要送資料 */
		while (inRS232_Data_Send_Check(stMultiOb->srSetting.uszComPort) != VS_SUCCESS);

		/* 經由port傳送資料 */
		inRetVal = inRS232_Data_Send(stMultiOb->srSetting.uszComPort, uszSendBuf, (unsigned short)inSendLen);
                
		if (inRetVal != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		else
		{
			
		/*---------------------------------- 印Debug 開始---------------------------------------------------------------*/
			/* 列印紙本電文和顯示電文訊息 */
			inECR_Print_Send_ISODeBug(szDataBuffer, inSendLen, inDataSize);
		/*---------------------------------- 印Debug 結束---------------------------------------------------------------*/
			/* 傳送Retry */
			if (inRetry < stMultiOb->stMulti_Optional_Setting.inMaxRetries)
			{
				/* 接收ACK OR NAK */
				inRetVal = inMultiFunc_RS232_Receive_ACKandNAK(stMultiOb);

				/* 超過一秒沒收到回應 */
				if (inRetVal == VS_TIMEOUT)
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "Not Receive Response, Retry");
					}
					inRetry++;
					continue;
				}
				/* 收到NAK */
				else if (inRetVal == _NAK_)
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "Receive NAK, Retry");
					}
					inRetry++;
					continue;
				}
				/* 收到ACK */	
				else
				{	
					/* 成功 */
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "ECR ACK OK!");
					}
					return (VS_SUCCESS);
				}
			}
			/* 超過最大重試次數，仍要完成交易，收銀機提示補登畫面 */
			else
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "Exceed max retry times!");
				}
				return (VS_SUCCESS);
			}

		}/* inRS232_Data_Send */
	
	}/* while(1) */
	
}

/*
Function        :inMultiFunc_Send_ACKorNAK
Date&Time       :2017/6/30 下午 2:11
Describe        :送ACK OR NAK
*/
int inMultiFunc_RS232_Send_ACKorNAK (MULTI_TABLE *stMultiOb, int inResponse)
{
	int		inRetVal;
	unsigned char	uszSendBuffer[2 + 1];
	
	memset(uszSendBuffer, 0x00, sizeof(uszSendBuffer));
	
	/* 檢查port是否已經準備好要送資料 */
        while (inRS232_Data_Send_Check(stMultiOb->srSetting.uszComPort) != VS_SUCCESS)
	{
		/* 等TxReady*/
	};
	
	if (inResponse == _ACK_)
	{	    
		/* 成功，回傳ACK */
		uszSendBuffer[0] = _ACK_;
		uszSendBuffer[1] = _ACK_;
		
		inRetVal = inRS232_Data_Send(stMultiOb->srSetting.uszComPort, uszSendBuffer, 2);
		
		if (inRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "Send ACK ACK Not OK");
			}
			if (ginDisplayDebug == VS_TRUE)
			{
				inDISP_LOGDisplay("Send ACK ACK Not OK", _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
			}

			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "ACK ACK");
			}
			if (ginDisplayDebug == VS_TRUE)
			{
				inDISP_LOGDisplay("Send ACK ACK", _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
			}
			
		}
        
	}
	else if (inResponse == _NAK_)
	{
		/* 失敗，回傳NAK */
		uszSendBuffer[0] = _NAK_;
		uszSendBuffer[1] = _NAK_;
		
		inRetVal = inRS232_Data_Send(stMultiOb->srSetting.uszComPort, uszSendBuffer, 2);
		
		if (inRetVal != VS_SUCCESS)
		{		
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "Send NAK NAK Not OK");
			}
			if (ginDisplayDebug == VS_TRUE)
			{
				inDISP_LOGDisplay("Send NAK NAK Not OK", _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
			}
			
			return (VS_ERROR);
		}
		else
		{	
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "NAK_NAK");
			}
			if (ginDisplayDebug == VS_TRUE)
			{
				inDISP_LOGDisplay("Send NAK NAK", _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
			}
			
		}
	
	}
	else if (inResponse == _FS_)
	{
		/* 失敗，回傳_FS_ */
		uszSendBuffer[0] = _FS_;
		uszSendBuffer[1] = _FS_;
		
		inRetVal = inRS232_Data_Send(stMultiOb->srSetting.uszComPort, uszSendBuffer, 2);
		
		if (inRetVal != VS_SUCCESS)
		{		
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "Send FS FS Not OK");
			}
			if (ginDisplayDebug == VS_TRUE)
			{
				inDISP_LOGDisplay("Send FS FS Not OK", _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
			}
			
			return (VS_ERROR);
		}
		else
		{	
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "FS FS");
			}
			if (ginDisplayDebug == VS_TRUE)
			{
				inDISP_LOGDisplay("Send FS FS", _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
			}
			
		}
	
	}
	else
	{
		/* 傳入錯誤的參數 */
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "Can't Send Neither Response");
		}
		if (ginDisplayDebug == VS_TRUE)
		{
			inDISP_LOGDisplay("Can't Send Response", _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
		}
		
		return (VS_ERROR);
	}
        
        return (VS_SUCCESS);
}

/*
Function        :inMultiFunc_Receive_ACKandNAK
Date&Time       :2017/7/4 上午 10:23
Describe        :
*/
int inMultiFunc_RS232_Receive_ACKandNAK(MULTI_TABLE *stMultiOb)
{
	int		inRetVal;
	char		szDebugMsg[100 + 1];
	unsigned char	uszBuf[_ECR_RS232_BUFF_SIZE_];
	unsigned short	usTwoSize = 2;
	unsigned short	usReceiveLen = 0;
		
	/* 設定Timeout */
	if (stMultiOb->stMulti_Optional_Setting.uszWaitForAck == VS_TRUE)
	{
		inRetVal = inDISP_Timer_Start(_TIMER_NEXSYS_2_, stMultiOb->stMulti_Optional_Setting.inACKTimeOut);
	}
	
	while (1)
	{
		memset(uszBuf, 0x00, sizeof(uszBuf));
		/* 當RS232中有東西(也可用有RS232事件來判斷)就開始分析 */
		while (usReceiveLen == 0)
		{
			inRS232_Data_Receive_Check(stMultiOb->srSetting.uszComPort, &usReceiveLen);
			
			if (stMultiOb->stMulti_Optional_Setting.uszWaitForAck == VS_TRUE)
			{
				/* 如果timeout就跳出去 */
				if (inTimerGet(_TIMER_NEXSYS_2_) == VS_SUCCESS)
				{
					return (VS_TIMEOUT);
				}
			}
		}
		
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Len : %d", usReceiveLen);
			inLogPrintf(AT, szDebugMsg);
		}
                
		while (usReceiveLen > 0)
		{
			if (stMultiOb->stMulti_Optional_Setting.uszWaitForAck == VS_TRUE)
			{
				/* 如果timeout就跳出去 */
				if (inTimerGet(_TIMER_NEXSYS_2_) == VS_SUCCESS)
				{
					return (VS_TIMEOUT);
				}
			}
			
			/* 這邊一次只收兩個byte */
			usTwoSize = 2;
			inRetVal = inRS232_Data_Receive(stMultiOb->srSetting.uszComPort, uszBuf, &usTwoSize);
                        
			if (inRetVal == VS_SUCCESS)
			{
				/* buffer讀出兩個byte，長度減二 */
				usReceiveLen -= 2;
				
				/* 判斷收到資料是否為ACK */	
				if (uszBuf[0] == _ACK_ && uszBuf[1] == _ACK_)
				{ 
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "Receive ACK!");
					}
					
					return (_ACK_);				
				}
				/* 判斷收到資料是否為NAK */
				else if (uszBuf[0] == _NAK_ && uszBuf[1] == _NAK_)
				{	
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "Receive NAK!");
					}
					
					return (_NAK_);
				}
				else 
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "Receive Not Ack Neither NAK!");
						memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
						inFunc_BCD_to_ASCII(szDebugMsg, uszBuf, usTwoSize);
						inLogPrintf(AT, szDebugMsg);
					}

					continue;
				}

			}/* inRS232_Data_Receive */
			
		}/* while (usReceiveLen > 0) (有收到資料) */
			
	}/* while(1)...... */
		
}

/*
Function        :inMultiFunc_RS232_Slave_GetCancel
Date&Time       :2017/7/10 下午 4:52
Describe        :用來看Host是否發送停止交易
*/
int inMultiFunc_RS232_Slave_GetCancel(unsigned char inHandle, MULTI_TABLE* stMultiFuncOb)
{
   	int		i = 0;
	int		inSize = 0;
	int		inExpectedSize = 0;
	int		inRespTimeOut = 5;
	char		szTemplate[42 + 1];
	char		szGetMultiData[_MULTI_MAX_SIZES_ + 4];
	char		szDebugMsg[100 + 1];
	unsigned short	usOneSize = 1;
	unsigned char	uszSTX[2 + 1];
	unsigned char	uszLRCData = 0x00;
	unsigned char	uszLRC = VS_FALSE;
	unsigned char	uszRecBuf[_MULTI_MAX_SIZES_ + 4];
	
	
	if (ginDisplayDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "inMultiFunc_RS232_Slave_GetCancel(%d)_START", inHandle);
		inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
	}

	/* 收STX + 前16個Byte */
	while (1)
	{
		usOneSize = 1;
    		if (inRS232_Data_Receive(inHandle, &uszSTX[0], &usOneSize) == VS_SUCCESS)
    		{
    			if (uszSTX[0] == _STX_)
    			{
				if (ginDisplayDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "Receive STX");
					inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
				}
    				uszRecBuf[inSize ++] = _STX_;
				break;
    			}
    		}
    		else
    		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "inMultiFunc_SlaveData_Recv()_STX_ERROR");
			}
    			
    			return (VS_ESCAPE);
    		}
	}
	
	/* 預計長度 */
	inExpectedSize = _MULTI_SUB_SIZE_SMALL_ + 30 + 3;
	
	/* 設定計時器 */
	if (inRespTimeOut > 0)
	{
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, inRespTimeOut);
	}
	
	/* 收Sub Data */
	while (1)
	{
		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == 0)
		{
			return (VS_TIMEOUT);
		}

		usOneSize = 1;
		if (inRS232_Data_Receive(inHandle, &uszRecBuf[inSize], &usOneSize) ==  VS_SUCCESS)
		{
			inSize ++;
		}

		/* 讀完該功能別的Subdata */
		if (inSize == inExpectedSize)
		{
			if (uszRecBuf[inSize - 2] == _ETX_)
			{
				break;
			}
		}
	} /* End while () .... */
	

	/* 算LRC */
	for ( i = 1; i < (inSize-1); i++)
	{
		uszLRCData ^= uszRecBuf[i];
	}
	
	/* 驗證LRC */
	if (uszLRCData == uszRecBuf[inSize - 1])
	{
		uszLRC = VS_TRUE;
	}
	else
	{
		
	}
	
	if (ginDisplayDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		
		if (uszLRC == VS_TRUE)
		{
			sprintf(szDebugMsg, "LRC OK");
		}
		else
		{
			sprintf(szDebugMsg, "LRC NOT OK");
		}
		
		inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
	}
	
	/* LRC驗證錯誤 */
	if (uszLRC != VS_TRUE)
	{
		return (VS_ERROR);
	}
	
        memcpy(szGetMultiData, &uszRecBuf[1], inSize - 3);
	szGetMultiData[inSize - 3] = 0x00;

        /* 檢核功能別 */
        memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(szTemplate, &szGetMultiData[14], 2);

	if (!memcmp(szTemplate, _MULTI_CTLS_, 2))
	{
	        /* 檢核回應碼 */
                memset(szTemplate, 0x00, sizeof(szTemplate));
        	memcpy(&szTemplate[0], &szGetMultiData[10], 4);

        	if (!memcmp(szTemplate, "0004", 4))
	        {
	                return (VS_SUCCESS);
	        }
		else
		{
			return (VS_ERROR);
		}
	}
	else
	{
		return (VS_ERROR);
	}

}

/*
Function        :inMultiFunc_Receive_Command
Date&Time       :2017/6/30 下午 6:30
Describe        :收命令
*/
int inMultiFunc_Receive_Command(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;

        vdUtility_SYSFIN_LogMessage(AT, "inMultiFunc_Receive_Command START!");
        
	/* 清空上次交易的資料 */
	memset(&gstMultiOb.stMulti_TransData, 0x00, sizeof(MULTIFUC_SLAVE_TRANSACTION_DATA));
	/* 收資料 */	
	inRetVal = stMultiSlave[gstMultiOb.srSetting.inVersion].inMultiRece(pobTran, &gstMultiOb);
	
	if (inRetVal == VS_ERROR)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inMultifuc_Receive_Command Error");
		}
		
		return (VS_ERROR);
	}
	else if (inRetVal == VS_TIMEOUT)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inMultifuc_Receive_Command Timeout");
		}
		
		return (VS_TIMEOUT);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inMultifuc_Receive_Command ok");
		}
		return (VS_SUCCESS);
	}
	
}

/*
Function        :inMultiFunc_SendResult
Date&Time       :2017/7/3 下午 4:59
Describe        :回傳結果
*/
int inMultiFunc_SendResult(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;
        
        vdUtility_SYSFIN_LogMessage(AT, "inMultiFunc_SendResult START!");

	/* 送資料 */	
	inRetVal = stMultiSlave[gstMultiOb.srSetting.inVersion].inMultiSend(pobTran, &gstMultiOb);
	
	if (inRetVal == VS_ERROR)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inMultiFunc_SendResult Error");
		}
		
		return (VS_ERROR);
	}
	else if (inRetVal == VS_TIMEOUT)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inMultiFunc_SendResult Timeout");
		}
		
		return (VS_TIMEOUT);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inMultiFunc_SendResult ok");
		}
		return (VS_SUCCESS);
	}
	
}

/*
Function        :inMultiFunc_SendError
Date&Time       :2017/7/3 下午 5:03
Describe        :送錯誤訊息給Master
*/
int inMultiFunc_SendError(TRANSACTION_OBJECT * pobTran, int inErrorType)
{
	if (pobTran->uszMultiFuncSlaveBit != VS_TRUE)
		return (VS_SUCCESS);
	if (gstMultiOb.stMulti_TransData.inErrorType == 0)
	{
		gstMultiOb.stMulti_TransData.inErrorType = inErrorType;
	}
	
	if (stMultiSlave[gstMultiOb.srSetting.inVersion].inMultiSendError(pobTran, &gstMultiOb) != VS_SUCCESS)
		return (VS_ERROR);
	
	return (VS_SUCCESS);
}

/*
Function        :inMultiFunc_SlaveInitial
Date&Time       :2016/6/20 下午 2:55
Describe        :initial COM PORT
 */
int inMultiFunc_SlaveInitial(MULTI_TABLE *stMultiOb)
{
    char		szDebugMsg[100 + 1];
    char		szMultiComPort1[4 + 1];
    unsigned char	uszParity;
    unsigned char	uszDataBits;
    unsigned char	uszStopBits;
    unsigned long	ulBaudRate;
    unsigned short	usRetVal;

    memset(&uszParity, 0x00, sizeof (uszParity));
    memset(&uszDataBits, 0x00, sizeof (uszDataBits));
    memset(&uszStopBits, 0x00, sizeof (uszStopBits));
    memset(&ulBaudRate, 0x00, sizeof (ulBaudRate));

    /* 從EDC.Dat抓出哪一個Comport，這裡先HardCode */
    /* inGetMultiComPort1 */
    memset(szMultiComPort1, 0x00, sizeof (szMultiComPort1));

    inGetMultiComPort1(szMultiComPort1);
    /* Verifone用handle紀錄，Castle用Port紀錄 */
    if (!memcmp(szMultiComPort1, _COMPORT_COM1_, 4))
        stMultiOb->srSetting.uszComPort = d_COM1;
    else if (!memcmp(szMultiComPort1, _COMPORT_COM2_, 4))
        stMultiOb->srSetting.uszComPort = d_COM2;
    else if (!memcmp(szMultiComPort1, _COMPORT_COM3_, 4))
        stMultiOb->srSetting.uszComPort = d_COM3;
    else if (!memcmp(szMultiComPort1, _COMPORT_COM4_, 4))
        stMultiOb->srSetting.uszComPort = d_COM4;


    /* BaudRate = 115200 */
    ulBaudRate = 115200;

    /* Parity */
    uszParity = 'N';

    /* Data Bits */
    uszDataBits = 8;

    /* Stop Bits */
    uszStopBits = 1;


    /* 開port */
    usRetVal = inRS232_Open(stMultiOb->srSetting.uszComPort, ulBaudRate, uszParity, uszDataBits, uszStopBits);

    if (usRetVal != VS_SUCCESS)
    {
	    if (ginDebug == VS_TRUE)
	    {
		    memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		    sprintf(szDebugMsg, "inRS232_Open Error: 0x%04x", usRetVal);
		    inLogPrintf(AT, szDebugMsg);
		    memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		    sprintf(szDebugMsg, "COM%d BaudRate:%lu %d%c%d", stMultiOb->srSetting.uszComPort + 1, ulBaudRate, uszDataBits, uszParity, uszStopBits);
		    inLogPrintf(AT, szDebugMsg);
	    }
	    return (VS_ERROR);
    }
    else
    {
	    if (ginDebug == VS_TRUE)
	    {
		inPRINT_ChineseFont("inRS232_Open OK", _PRT_NORMAL_);
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "COM%d BaudRate:%lu %d%c%d", stMultiOb->srSetting.uszComPort, ulBaudRate, uszDataBits, uszParity, uszStopBits);
		inPRINT_ChineseFont(szDebugMsg, _PRT_NORMAL_);
	    }
    }

    /* 清空接收的buffer */
    inMultiFunc_RS232_FlushRxBuffer();

    return (VS_SUCCESS);
}

/*
Function        :inMultiFunc_SlaveRecePacket
Date&Time       :2016/7/6 下午 4:04
Describe        :處理收銀機傳來的資料
 */
int inMultiFunc_SlaveRecePacket(TRANSACTION_OBJECT *pobTran, MULTI_TABLE *stMultiOb)
{
	int	i = 0;
	int	inRetVal;
	char	szTemplate[50];
	char	szGetDate[8 + 1], szGetTime[6 + 1];
	char	szDebugMsg[100 + 1];
	char	szPinTemp[16 + 1];

	memset(stMultiOb->stMulti_TransData.szReceData, 0x00, sizeof (stMultiOb->stMulti_TransData.szReceData));

	/* 測試Flag*/
	if (ginEngineerDebug == VS_TRUE)
	{
		memcpy(stMultiOb->stMulti_TransData.szReceData, "NCCC70231800002001010000000000", 30);
		inRetVal = VS_SUCCESS;
	}
	else
	{
		/* -----------------------開始接收資料------------------------------------------ */
		inRetVal = inMultiFunc_SlaveData_Recv(stMultiOb->srSetting.uszComPort,
						  2,
						  stMultiOb->stMulti_TransData.szReceData,
						  VS_FALSE,
						  stMultiOb);

		{
			inDISP_BEEP(1, 0);
		}
	}
		
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDisplayDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "第一階段分析不OK");
			inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
		}
		
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "第一階段分析不OK");
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (inRetVal);
	}
	else
	{
		if (ginDisplayDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "第一階段分析OK");
			inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
		}
		
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "第一階段分析OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}

	
	/* -----------------------開始分析資料------------------------------------------ */
	/* 解封包流程 */
	inRetVal = inMultiFunc_SlaveData_Unpack_Header(pobTran, stMultiOb->stMulti_TransData.szReceData, stMultiOb);
	
	/* 如果是Polling不回idle直接做 */
	if (!memcmp(stMultiOb->stMulti_TransData.szTransType, _MULTI_POLL_, 2))
	{
		memset(&stMultiOb->stMulti_TransData, 0x00, sizeof (stMultiOb->stMulti_TransData));
		/* -----------------------開始接收資料------------------------------------------ */
		
		{
			inDISP_BEEP(1, 0);
		}
		
		/* 測試Flag */
		if (ginEngineerDebug == VS_TRUE)
		{
			memcpy(stMultiOb->stMulti_TransData.szReceData, "NCCC702318000002010100020001370000000001001202017091813360967124141  006167441111001  2AF75E094C3D65B697F4D2123F759F67000000300100801167124141001000600000N000000000249                                                               ", 230);
			inRetVal = VS_SUCCESS;
		}
		else
		{
			inRetVal = inMultiFunc_SlaveData_Recv(stMultiOb->srSetting.uszComPort,
							  2,
							  stMultiOb->stMulti_TransData.szReceData,
							  VS_FALSE,
							  stMultiOb);
					
			if (inRetVal != VS_SUCCESS)
			{
				if (ginDisplayDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "第二階段分析不OK");
					inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
				}

				return (inRetVal);
			}
			else
			{
				if (ginDisplayDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "第二階段分析OK");
					inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
				}
			}
		}
		
		/* 解封包 */		
		inRetVal = inMultiFunc_SlaveData_Unpack_Header(pobTran, stMultiOb->stMulti_TransData.szReceData, stMultiOb);
	}

	/* 根據功能別，來解SubData */
	if (!memcmp(stMultiOb->stMulti_TransData.szTransType, _MULTI_PIN_, 2))
	{
		/* Amount (12 Bytes) */
		memset(szTemplate, 0x00, sizeof (szTemplate));
		memcpy(&szTemplate[0], &stMultiOb->stMulti_TransData.szReceData[46], 10);
		pobTran->srBRec.lnTxnAmount = atol(szTemplate);

		/* 卡號長度 (2 Bytes) */
		memset(szTemplate, 0x00, sizeof (szTemplate));
		memcpy(&szTemplate[0], &stMultiOb->stMulti_TransData.szReceData[58], 2);

		/* 卡號 (20 Bytes) */
		memset(pobTran->srBRec.szPAN, 0x00, sizeof (pobTran->srBRec.szPAN));
		memcpy(&pobTran->srBRec.szPAN[0], &stMultiOb->stMulti_TransData.szReceData[60], atoi(szTemplate));
		
		/* 這邊輸入PIN */
		memset(szPinTemp, 0x00, sizeof(szPinTemp));
		inRetVal = inMultiFunc_EnterPin(pobTran, szPinTemp);
		if (strlen(szPinTemp) > 0)
		{
			pobTran->srBRec.uszPinEnterBit = VS_TRUE;
		}
		
		if (inRetVal == VS_SUCCESS)
		{
			if (ginDisplayDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "輸入PIN OK");
				inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
			}
			
			/* 成功且沒輸入密碼，代表Bypass */
			if (pobTran->srBRec.uszPinEnterBit != VS_TRUE)
				stMultiOb->stMulti_TransData.inErrorType = _ECR_RESPONSE_CODE_PIN_BYPASS_;
			else
			{
				/* 0x00的地方改0x20，補滿16碼 */
				for (i = 0; i < 16; i++)
				{
					if (szPinTemp[i] == 0x00)
					{
						szPinTemp[i] = 0x20;
					}
				}
				memset(pobTran->szPIN, 0x00, sizeof(pobTran->szPIN));
				inRetVal = inMultiFunc_3DES_Encrypt(szPinTemp, strlen(szPinTemp), pobTran->szPIN);
				if (inRetVal != VS_SUCCESS)
				{
					if (ginDisplayDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "MultiFun 3DES Enc Fail");
						inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_TRUE);
					}
				}
				stMultiOb->stMulti_TransData.inErrorType = _ECR_RESPONSE_CODE_SUCCESS_;
			}
		}
		else if (inRetVal == VS_USER_CANCEL)
		{
			stMultiOb->stMulti_TransData.inErrorType = _ECR_RESPONSE_CODE_USER_TERMINATE_ERROR_;
		}
		else if (inRetVal == VS_TIMEOUT)
		{
			stMultiOb->stMulti_TransData.inErrorType = _ECR_RESPONSE_CODE_TIMEOUT_;
		}
		else
		{
			stMultiOb->stMulti_TransData.inErrorType = _ECR_RESPONSE_CODE_ERROR_;
		}
	}
	else if (!memcmp(stMultiOb->stMulti_TransData.szTransType, _MULTI_CTLS_, 2))
	{
		/* Amount */
		memset(szTemplate, 0x00, sizeof (szTemplate));
		memcpy(&szTemplate[0], &stMultiOb->stMulti_TransData.szReceData[30], 10);
		pobTran->srBRec.lnOrgTxnAmount = atol(szTemplate);
		pobTran->srBRec.lnTxnAmount = pobTran->srBRec.lnOrgTxnAmount;
		pobTran->srBRec.lnTotalTxnAmount = pobTran->srBRec.lnOrgTxnAmount;
		
		/* Timeout */
		memset(szTemplate, 0x00, sizeof (szTemplate));
		memcpy(&szTemplate[0], &stMultiOb->stMulti_TransData.szReceData[42], 3);
		stMultiOb->stMulti_TransData.inCTLS_Timeout = atoi(szTemplate);

		/* EDC TIME */
		memset(szGetDate, 0x00, sizeof (szGetDate));
		memcpy(&szGetDate[0], &stMultiOb->stMulti_TransData.szReceData[45], 8);
		memset(pobTran->srBRec.szDate, 0x00, sizeof(pobTran->srBRec.szDate));
		memcpy(pobTran->srBRec.szDate, szGetDate, 8);
		
		memset(szGetTime, 0x00, sizeof (szGetTime));
		memcpy(&szGetTime[0], &stMultiOb->stMulti_TransData.szReceData[53], 6);
		memset(pobTran->srBRec.szTime, 0x00, sizeof(pobTran->srBRec.szTime));
		memcpy(pobTran->srBRec.szTime, szGetTime, 6);

		/* FISC TIME */
		/* 直接當EDC時間 */
		memset(pobTran->srBRec.szDate, 0x00, sizeof(pobTran->srBRec.szDate));
		memcpy(pobTran->srBRec.szDate, &stMultiOb->stMulti_TransData.szReceData[45], 8);
		memset(pobTran->srBRec.szTime, 0x00, sizeof(pobTran->srBRec.szTime));
		memcpy(pobTran->srBRec.szTime, &stMultiOb->stMulti_TransData.szReceData[53], 6);

		/* TID (10 Bytes) */
		memset(szTemplate, 0x00, sizeof (szTemplate));
		memcpy(&szTemplate[0], &stMultiOb->stMulti_TransData.szReceData[59], 8);
		memset(pobTran->szMultiFuncTID, 0x00, sizeof (pobTran->szMultiFuncTID));
		memcpy(pobTran->szMultiFuncTID, szTemplate, 8);

		/* MID (15 Bytes) */
		memset(szTemplate, 0x00, sizeof (szTemplate));
		memcpy(&szTemplate[0], &stMultiOb->stMulti_TransData.szReceData[69], 15);
		memset(pobTran->szMultiFuncMID, 0x00, sizeof (pobTran->szMultiFuncMID));
		memcpy(pobTran->szMultiFuncMID, szTemplate, 15);

		/* CUP MAC KEY (32 Bytes) */
		/* MAC key目前用不到 */
		memset(szTemplate, 0x00, sizeof (szTemplate));
		memcpy(&szTemplate[0], &stMultiOb->stMulti_TransData.szReceData[86], 32);
		
		/* MCC CODE */
		memset(pobTran->srBRec.szFiscMCC, 0x00, sizeof (pobTran->srBRec.szFiscMCC));
		memcpy(pobTran->srBRec.szFiscMCC, &stMultiOb->stMulti_TransData.szReceData[130], 4);
		
		/* 端末機查核碼(8 Bytes)*/
		memset(pobTran->srBRec.szFiscTCC, 0x00, sizeof (pobTran->srBRec.szFiscTCC));
		memcpy(pobTran->srBRec.szFiscTCC, &stMultiOb->stMulti_TransData.szReceData[134], 8);
		
		/* inTransactionCode (3 Bytes) */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], &stMultiOb->stMulti_TransData.szReceData[142], 3);
                pobTran->inMultiTransactionCode = atoi(szTemplate);
		if (pobTran->inMultiTransactionCode == _520_MULTIFUNC_REFUND_)
		{
			pobTran->srBRec.inCode = _REFUND_;
		}
		else
		{
			pobTran->srBRec.inCode = _SALE_;
		}

		/* Nationalpay(全國繳) */
                memset(szTemplate, 0x00, sizeof(szTemplate));
	        memcpy(&szTemplate[0],&stMultiOb->stMulti_TransData.szReceData[145],1);
		memset(pobTran->szMultiNaitionalPay, 0x00, sizeof(pobTran->szMultiNaitionalPay));
	        if (!memcmp(szTemplate, "1", 1))
		{
			memcpy(pobTran->szMultiNaitionalPay, "10", 2);		/* 設定有全國性繳費*/
		}
	        else
		{
			memcpy(pobTran->szMultiNaitionalPay, "00", 2);		/* 設定無全國性繳費*/
		}
	        
		/* 端末機IssuerId，TAC產生時要比對 */
		memset(pobTran->szMultiEdcFiscIssuerId, 0x00, sizeof(pobTran->szMultiEdcFiscIssuerId));
	        memcpy(pobTran->szMultiEdcFiscIssuerId, &stMultiOb->stMulti_TransData.szReceData[146], 8);

		/* FdtFeeFlag "Y" or "N"*/
		memset(pobTran->szMultiEdcFdtFeeFlag, 0x00, sizeof(pobTran->szMultiEdcFdtFeeFlag));
	        memcpy(pobTran->szMultiEdcFdtFeeFlag, &stMultiOb->stMulti_TransData.szReceData[154], 1);

		/* lnFdtFeeAmt */
                memset(szTemplate, 0x00, sizeof(szTemplate));
	        memcpy(&szTemplate[0], &stMultiOb->stMulti_TransData.szReceData[155],4);
	        pobTran->lnMultiFdtFeeAmt = atol(szTemplate);
	        
		/* TCC Code for NP */
		memset(pobTran->szMultiEdcTccCode, 0x00, sizeof(pobTran->szMultiEdcTccCode));
	        memcpy(pobTran->szMultiEdcTccCode, &stMultiOb->stMulti_TransData.szReceData[159], 8);	        
		
		/* uszECRPreferCreditBit(信用卡優先) */
		memset(&pobTran->uszECRPreferCreditBit, 0x00, sizeof(pobTran->uszECRPreferCreditBit));
		if (stMultiOb->stMulti_TransData.szReceData[167] == '1')
		{
			pobTran->uszECRPreferCreditBit = VS_TRUE;
		}
		else
		{
			pobTran->uszECRPreferCreditBit = VS_FALSE;
		}
		
		/* uszECRPreferFiscBit(金融卡優先) */
		memset(&pobTran->uszECRPreferFiscBit, 0x00, sizeof(pobTran->uszECRPreferFiscBit));
	        if (stMultiOb->stMulti_TransData.szReceData[168] == '1')
		{
			pobTran->uszECRPreferFiscBit = VS_TRUE;
		}
		else
		{
			pobTran->uszECRPreferFiscBit = VS_FALSE;
		}
		
		/* 檢查時間 */
		inRetVal = inMultiFunc_TimeCheck(szGetDate, szGetTime);
		
		if (inRetVal == VS_SUCCESS)
		{
			if (pobTran->inMultiTransactionCode == _520_MULTIFUNC_REFUND_)
			{
				inRetVal = inMultiFunc_GetCardFields_Refund_CTLS(pobTran, stMultiOb);
			}
			else
			{
				inRetVal = inMultiFunc_GetCardFields_CTLS(pobTran, stMultiOb);
			}
			
			if (inRetVal == VS_SUCCESS)
			{
				if (ginDisplayDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "Get Card Success");
					inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
				}
				
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "Get Gard Success");
					inLogPrintf(AT, szDebugMsg);
				}
			}
			else
			{
				if (ginDisplayDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "Get Gard Fail");
					inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
				}
				
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "Get Gard Fail");
					inLogPrintf(AT, szDebugMsg);
				}
			}
		}
		else
		{
			/* 不顯示錯誤訊息 */
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "inMultiFunc_TimeCheck()_ERROR");
				inLogPrintf(AT, szDebugMsg);
			}
		}
		
		if (ginDisplayDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTLS inRetval: %d", inRetVal);
			inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
		}
		
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTLS inRetval: %d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		
		if (inRetVal == VS_SUCCESS)
		{
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE || 
			    memcmp(pobTran->srBRec.szFiscPayDevice, _FISC_PAY_DEVICE_MOBILE_, strlen(_FISC_PAY_DEVICE_MOBILE_)) == 0)
			{
				stMultiOb->stMulti_TransData.inErrorType = _ECR_RESPONSE_CODE_SMARTPAY_ERROR_;
			}
			else
			{
				stMultiOb->stMulti_TransData.inErrorType = _ECR_RESPONSE_CODE_SUCCESS_;
			}
		}
		else if (inRetVal == VS_USER_CANCEL)
		{
			stMultiOb->stMulti_TransData.inErrorType = _ECR_RESPONSE_CODE_USER_TERMINATE_ERROR_;
			pobTran->inErrorMsg = _ERROR_CODE_V3_USER_CANCEL_;
		}
		else if (inRetVal == VS_TIMEOUT)
		{
			stMultiOb->stMulti_TransData.inErrorType = _ECR_RESPONSE_CODE_TIMEOUT_;
		}
		else if (inRetVal == VS_ESCAPE)
		{
			stMultiOb->stMulti_TransData.inErrorType = _ECR_RESPONSE_CODE_CTLS_HOST_CANCEL_;
		}
		else if (inRetVal == VS_WAVE_AMOUNT_ERR)
		{
			stMultiOb->stMulti_TransData.inErrorType = _ECR_RESPONSE_CODE_AMOUNT_ERROR_;

			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE || 
			    memcmp(pobTran->srBRec.szFiscPayDevice, _FISC_PAY_DEVICE_MOBILE_, strlen(_FISC_PAY_DEVICE_MOBILE_)) == 0)
			{
//				inDISP_ChineseFont("    請移除卡片", _FONESIZE_16X22_, _LINE_16_4_, _DISP_LEFT_);
			}
		}
		else
		{
			stMultiOb->stMulti_TransData.inErrorType = _ECR_RESPONSE_CODE_CTLS_ERROR_;
		}

	}
	else if (!memcmp(stMultiOb->stMulti_TransData.szTransType, _MULTI_TMS_CAPK_, 2)	||
		 !memcmp(stMultiOb->stMulti_TransData.szTransType, _MULTI_TMS_MASTER_, 2)	||
		 !memcmp(stMultiOb->stMulti_TransData.szTransType, _MULTI_TMS_VISA_, 2)	||
		 !memcmp(stMultiOb->stMulti_TransData.szTransType, _MULTI_TMS_JCB_, 2)	||
		 !memcmp(stMultiOb->stMulti_TransData.szTransType, _MULTI_TMS_CUP_, 2))
	{
		inRetVal = VS_SUCCESS;
	}
	else if (!memcmp(stMultiOb->stMulti_TransData.szTransType, _MULTI_POLL_, 2))
	{
		inRetVal = VS_SUCCESS;
	}
	else if (!memcmp(stMultiOb->stMulti_TransData.szTransType, _MULTI_EXCHANGE_, 2))
	{
		stMultiOb->stMulti_TransData.inErrorType = _ECR_RESPONSE_CODE_END_INQUIRY_SUCCESS_;
		inRetVal = VS_SUCCESS;
	}
	else
	{
		return (VS_ERROR);
	}
	
	/* 回送 Packet */
	if (inRetVal == VS_SUCCESS) 
	{       
		inRetVal = inMultiFunc_Response_Host(pobTran, stMultiOb);
	}
	/* 主機交易取消 */
	else if (inRetVal == VS_ESCAPE)
	{
		/* 回idle */
	}
	else
	{
		/* 讓他回idle前（inFlow_RunFunction）在跑Send Error，顯示錯誤訊息也在那邊 */
	}

	if (inRetVal == VS_SUCCESS)
	{
		return (VS_SUCCESS);
	}
	else if (inRetVal == VS_ESCAPE)
	{
		return (VS_SUCCESS);
	}
	else
		return (VS_ERROR);
}


/*
Function        :inMultiFunc_SlaveSendPacket
Date&Time       :2016/7/11 下午 3:29
Describe        :處理要送給收銀機的資料
 */
int inMultiFunc_SlaveSendPacket(TRANSACTION_OBJECT *pobTran, MULTI_TABLE * stMultiSlave)
{
	int	inRetVal = VS_SUCCESS;
	int	inSendPacketSizes;
	char	szDebugMsg[100 + 1];
	
	if (ginDisplayDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "inMultiFunc_SendPacket");
		inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
	}
	
	if (memcmp(stMultiSlave->stMulti_TransData.szTransType, _MULTI_PIN_, sizeof(_MULTI_PIN_)) == 0				||
	    memcmp(stMultiSlave->stMulti_TransData.szTransType, _MULTI_CTLS_, sizeof(_MULTI_CTLS_)) == 0			||
	    memcmp(stMultiSlave->stMulti_TransData.szTransType, _MULTI_EXCHANGE_, sizeof(_MULTI_EXCHANGE_NO_)) == 0)
	{
		/* 交易取消不回傳 */
		if (stMultiSlave->stMulti_TransData.inErrorType == _ECR_RESPONSE_CODE_CTLS_HOST_CANCEL_)
		{
			return (VS_SUCCESS);
		}
	}
	else
	{
		return (VS_SUCCESS);
	}
	
	/* 清空資料 */
        memset(stMultiSlave->stMulti_TransData.szSendData, 0x20, sizeof(stMultiSlave->stMulti_TransData.szSendData));

	/* 包裝封包 */
	inSendPacketSizes = inMultiFunc_SlavePackResult(pobTran, stMultiSlave->stMulti_TransData.szSendData, stMultiSlave);
	if (inSendPacketSizes > 0)
	{
		if (ginDisplayDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Pack OK");
			inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_TRUE);
		}
	}
	else
	{
		if (ginDisplayDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Pack NOT OK");
			inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_TRUE);
		}
		
		return (VS_ERROR);
	}

	/* 傳送封包 */
	/* 用完要清掉設定 */
	memset(&stMultiSlave->stMulti_Optional_Setting, 0x00, sizeof(stMultiSlave->stMulti_Optional_Setting));
	stMultiSlave->stMulti_Optional_Setting.inACKTimeOut = 2;
	stMultiSlave->stMulti_Optional_Setting.inMaxRetries = 0;
	stMultiSlave->stMulti_Optional_Setting.uszPadStxEtx = VS_TRUE;
	stMultiSlave->stMulti_Optional_Setting.uszWaitForAck = VS_FALSE;
	inRetVal = inMultiFunc_RS232_Send(pobTran, stMultiSlave, stMultiSlave->stMulti_TransData.szSendData, inSendPacketSizes);
	/* 用完要清掉設定 */
	memset(&stMultiSlave->stMulti_Optional_Setting, 0x00, sizeof(stMultiSlave->stMulti_Optional_Setting));
	if (inRetVal == VS_SUCCESS)
	{
		if (ginDisplayDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Send OK");
			inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_TRUE);
		}
	}
	else
	{
		if (ginDisplayDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Send NOT OK");
			inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_TRUE);
		}
		
		return (VS_ERROR);
	}

	return (inRetVal); 
}

/*
Function        :inMultiFunc_SlaveSendError
Date&Time       :2016/7/18 上午 10:21
Describe        :傳送錯誤訊息ECR
 */
int inMultiFunc_SlaveSendError(TRANSACTION_OBJECT *pobTran, MULTI_TABLE *stMultiOb) 
{
	int	inRetVal = VS_SUCCESS;
	int	inTotalSize = 0;
	int	inSubSize = 0;
	int	inLenlocation = 0;
	char	szTemplate[128 + 1];
	char	szPackData[_MULTI_MAX_SIZES_ + 1];
	char	szDebugMsg[100 + 1];
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "inMultiFunc_SendError");
		inLogPrintf(AT, szDebugMsg);
	}
	if (ginDisplayDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "inMultiFunc_SendError");
		inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_TRUE);
	}

	memset(szPackData, 0x20, sizeof(szPackData));
         
	/* Bank (4 Bytes) */
	memcpy(&szPackData[inTotalSize], &stMultiOb->stMulti_TransData.szTranHost[0], 4);
	inTotalSize += 4;

	/* EDC SN (6 Bytes) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(szTemplate, stMultiOb->stMulti_TransData.szTermSN, 6);
	memcpy(&szPackData[inTotalSize], szTemplate, 6);
	inTotalSize += 6;
	
	if (ginDisplayDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "inErrorType :%d", stMultiOb->stMulti_TransData.inErrorType);
		inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
	}

	/* Response Code (4 Bytes) */
	if (stMultiOb->stMulti_TransData.inErrorType == _ECR_RESPONSE_CODE_SUCCESS_)
		memcpy(&szPackData[inTotalSize], "0000", 4);
	else if (stMultiOb->stMulti_TransData.inErrorType == _ECR_RESPONSE_CODE_USER_TERMINATE_ERROR_)
		memcpy(&szPackData[inTotalSize], "0002", 4);
	else if (stMultiOb->stMulti_TransData.inErrorType == _ECR_RESPONSE_CODE_TIMEOUT_)
		memcpy(&szPackData[inTotalSize], "0003", 4);
	else if (stMultiOb->stMulti_TransData.inErrorType == _ECR_RESPONSE_CODE_CTLS_HOST_CANCEL_)
		memcpy(&szPackData[inTotalSize], "0004", 4);
	else if (stMultiOb->stMulti_TransData.inErrorType == _ECR_RESPONSE_CODE_AMOUNT_ERROR_)
		memcpy(&szPackData[inTotalSize], "0006", 4);        
	else if (stMultiOb->stMulti_TransData.inErrorType == _ECR_RESPONSE_CODE_END_INQUIRY_SUCCESS_)
		memcpy(&szPackData[inTotalSize], "0000", 4);
	else if (stMultiOb->stMulti_TransData.inErrorType == _ECR_RESPONSE_CODE_CTLS_ERROR_)
		memcpy(&szPackData[inTotalSize], "0000", 4);
	else
		memcpy(&szPackData[inTotalSize], "0001", 4);

	inTotalSize += 4;

	/* Trans Type (2 Bytes) */
	memcpy(&szPackData[inTotalSize], &stMultiOb->stMulti_TransData.szTransType[0], 2);
	inTotalSize += 2;

	/* Total Packet (2 Bytes) */
	memcpy(&szPackData[inTotalSize], "01", 2);
	inTotalSize += 2;

	/* Sub Packet Index (2 Bytes) */
	memcpy(&szPackData[inTotalSize], "01", 2);
	inTotalSize += 2;

	/* Total Packet Size (6 Bytes) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	if (!memcmp(stMultiOb->stMulti_TransData.szTransType, _MULTI_CTLS_, 2))
	{
		sprintf(szTemplate, "%06d", _MULTI_SUB_SIZE_MAX_);
	}
	else
	{
		sprintf(szTemplate, "%06d", _MULTI_SUB_SIZE_SMALL_);
	}
	memcpy(&szPackData[inTotalSize], &szTemplate[0], 6);
	inTotalSize += 6;

	/* Sub Packet Size (4 Bytes) - 最後再算，動態 */
	inLenlocation = inTotalSize;
	inTotalSize += 4;

	if (pobTran->inErrorMsg == _ERROR_CODE_V3_MULTI_FUNC_CTLS_)
	{
		if (ginDisplayDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTLS ERROR len:%d", ginMultiCTLSLen);
			inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_TRUE);
		}
		memcpy(&szPackData[inTotalSize], gszMultiCTLSData, ginMultiCTLSLen);
		inSubSize += ginMultiCTLSLen;
	}
	else
	{
		inSubSize = 0;
	}
	
	/* SubData Size */
	if (!memcmp(stMultiOb->stMulti_TransData.szTransType, _MULTI_CTLS_, 2))
	{
		inTotalSize += _MULTI_SUB_SIZE_MAX_;
	}
	else
	{
		inTotalSize += _MULTI_SUB_SIZE_SMALL_;
	}
	
	/* Sub Packet Size (4 Bytes) */
	memset(szTemplate, 0x00, sizeof (szTemplate));
	sprintf(szTemplate, "%04d", inSubSize);
	memcpy(&szPackData[inLenlocation], &szTemplate[0], 4);
  	
	/* 送封包 */
	/* 用完要清掉設定 */
	memset(&stMultiOb->stMulti_Optional_Setting, 0x00, sizeof(stMultiOb->stMulti_Optional_Setting));
	stMultiOb->stMulti_Optional_Setting.inACKTimeOut = 2;
	stMultiOb->stMulti_Optional_Setting.inMaxRetries = 0;
	stMultiOb->stMulti_Optional_Setting.uszPadStxEtx = VS_TRUE;
	stMultiOb->stMulti_Optional_Setting.uszWaitForAck = VS_FALSE;
	inRetVal = inMultiFunc_RS232_Send(pobTran, stMultiOb, szPackData, inTotalSize);
	
	/* 用完要清掉設定 */
	memset(&stMultiOb->stMulti_Optional_Setting, 0x00, sizeof(stMultiOb->stMulti_Optional_Setting));
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDisplayDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Send Fail");
			inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_TRUE);
		}
		
		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

/*
Function        :inMultiFunc_SlaveEnd
Date&Time       :2016/7/11 下午 3:34
Describe        :關閉Comport
 */
int inMultiFunc_SlaveEnd(MULTI_TABLE* stMultiOb)
{
	/*清空接收的buffer*/
	inRS232_FlushRxBuffer(stMultiOb->srSetting.uszComPort);

	/*關閉port*/
	if (inRS232_Close(stMultiOb->srSetting.uszComPort) != VS_SUCCESS)
	{
	    return (VS_ERROR);
	}

	return (VS_SUCCESS);
}
/*
Function        :inMultiFunc_SlaveData_Recv
Date&Time       :2016/7/6 下午 4:05
Describe        :接收收銀機傳來的資料
*/
int inMultiFunc_SlaveData_Recv(unsigned char inHandle, int inRespTimeOut, char *szGetMultiData, unsigned char uszSendAck, MULTI_TABLE* stMultiFuncOb)
{
	int		inSize, i = 0;
	int		inExpectedSize = 0;
	char		szDebugMsg[100 + 1];
	char		szTemplate[30 + 1];
	unsigned char	uszSTX[2 + 1];
	unsigned char	uszRecBuf[_MULTI_MAX_SIZES_ + 4];
	unsigned char	uszLRCData = 0x00;
	unsigned short	usOneSize = 1;
	unsigned char	uszLRC = VS_FALSE, uszSN = VS_FALSE, uszReStart = VS_FALSE;

	if (ginDisplayDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "inMultiSlave_Recv(%d)_START", inHandle);
		inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
	}
	
	inSize = 0;
	memset(uszRecBuf, 0x00, sizeof(uszRecBuf));
	memset(uszSTX, 0x00, sizeof(uszSTX));
	
	/* 設定計時器 */
	if (30 > 0)
	{
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, inRespTimeOut);
	}

	/* 收STX + 前16個Byte */
	while (1)
	{
		/* Comport有東西 */
		while (inRS232_Data_Receive_Check(inHandle, &usOneSize) != VS_SUCCESS)
		{
			/* Timeout */
			if (inTimerGet(_TIMER_NEXSYS_1_) == 0)
			{
				return (VS_TIMEOUT);
			}
		}
		
		/* 這是Buffer的大小，若沒收到則該值會轉為0，所以需重置 */
		usOneSize = 1;
    		if (inRS232_Data_Receive(inHandle, &uszSTX[0], &usOneSize) == VS_SUCCESS)
    		{
			/* Timeout */
			if (inTimerGet(_TIMER_NEXSYS_1_) == 0)
			{
				return (VS_TIMEOUT);
			}
			
    			if (uszSTX[0] == _STX_)
    			{
				if (ginDisplayDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "Receive STX");
					inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
				}
    				uszRecBuf[inSize ++] = _STX_;

			        i = 16;

			        while (i > 0)
			        {
					/* 銀行別 + 回應碼 + 功能別 */
					usOneSize = 100;
					if (inRS232_Data_Receive(inHandle, &uszRecBuf[inSize], &usOneSize) == VS_SUCCESS)
					{
						inSize += usOneSize;
						i -= usOneSize;
					}
  
				}

				break;
    			}
			else
			{
				if (ginDisplayDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "Val = %02X", uszSTX[0]);
					inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
				}
			}
    		}
    		else
    		{		
			if (ginDisplayDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "inMultiFunc_SlaveData_Recv()_STX_ERROR");
				inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
			}
    			
    			return (VS_ESCAPE);
    		}
	}
	
	
	/* 前16Byte OK*/
	if (ginDisplayDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "前16 OK");
		inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
	}

	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(&szTemplate[0], &uszRecBuf[15], 2);

	/* 下TMS參數 */
	if (!memcmp(szTemplate, _MULTI_TMS_CAPK_, 2)	||
	    !memcmp(szTemplate, _MULTI_TMS_MASTER_, 2)	||
	    !memcmp(szTemplate, _MULTI_TMS_VISA_, 2)	||
	    !memcmp(szTemplate, _MULTI_TMS_JCB_, 2)	||
	    !memcmp(szTemplate, _MULTI_TMS_CUP_, 2))/*20151014浩瑋新增*/
	{
                memset(stMultiFuncOb->stMulti_TransData.szTransType, 0x00, sizeof(stMultiFuncOb->stMulti_TransData.szTransType));
                memcpy(&stMultiFuncOb->stMulti_TransData.szTransType[0], &szTemplate[0], 2);
       
                memset(stMultiFuncOb->stMulti_TransData.szTranHost, 0x00, sizeof(stMultiFuncOb->stMulti_TransData.szTranHost));
                memcpy(&stMultiFuncOb->stMulti_TransData.szTranHost[0], &uszRecBuf[1], 4);

		return (VS_SUCCESS);
	}
	/* 重新開機 */
	else if (!memcmp(szTemplate, _MULTI_SLAVE_REBOOT_, 2))
	{
		uszReStart = VS_TRUE;
	}
	else if (!memcmp(szTemplate, _MULTI_POLL_, 2))
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(&szTemplate[0], &uszRecBuf[5], 6);
		
		/* 不核對SN */

		uszSN = VS_TRUE;
		uszSendAck = VS_TRUE;
	}
	
	/* 看看是否要收SubData*/
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(&szTemplate[0], &uszRecBuf[15], 2);

	/* +30是header +3表示STX ETX LRC */
	/* 根據功能別，決定要收的SubData長度 */
	if (!memcmp(szTemplate, _MULTI_POLL_, 2)		||
	    !memcmp(szTemplate, _MULTI_SLAVE_REBOOT_, 2)	||
	    !memcmp(szTemplate, _MULTI_EXCHANGE_, 2))
	{
		inExpectedSize = _MULTI_SUB_SIZE_NONE_ + 30 + 3;
	}
	else if (!memcmp(szTemplate, _MULTI_PIN_, 2)		||
		 !memcmp(szTemplate, _MULTI_SIGNPAD_, 2)	||
		 !memcmp(szTemplate, _MULTI_SIGN_CONFIRM_, 2)	||
		 !memcmp(szTemplate, _MULTI_NOSIGN_, 2))
	{
		inExpectedSize = _MULTI_SUB_SIZE_SMALL_ + 30 + 3;
	}
	else if (!memcmp(szTemplate, _MULTI_CTLS_, 2))
	{
		inExpectedSize = _MULTI_SUB_SIZE_MIDDLE_ + 30 + 3;
	}
	else
	{
		inExpectedSize = _MULTI_SUB_SIZE_MAX_ + 30 + 3;
	}

	/* 設定計時器 */
	if (inRespTimeOut > 0)
	{
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, inRespTimeOut);
	}

	/* 收Sub Data */
	while (1)
	{
		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == 0)
		{
			return (VS_TIMEOUT);
		}

		usOneSize = 100;
		if (inRS232_Data_Receive(inHandle, &uszRecBuf[inSize], &usOneSize) ==  VS_SUCCESS)
		{
			inSize += usOneSize;
		}

		/* 讀完該功能別的Subdata */
		if (inSize == inExpectedSize)
		{
			if (uszRecBuf[inSize - 2] == _ETX_)
			{
				break;
			}
		}
	} /* End while () .... */
		
	/* 算LRC */
	for (i = 1; i < (inSize-1); i++)
	{
		uszLRCData ^= uszRecBuf[i];
	}
		
	/* 驗證LRC */
	if (uszLRCData == uszRecBuf[inSize - 1])
	{
		uszLRC = VS_TRUE;
	}
	else
	{
		uszLRC = VS_FALSE;
	}
	
	if (ginDisplayDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		
		if (uszLRC == VS_TRUE)
		{
			sprintf(szDebugMsg, "LRC OK");
		}
		else
		{
			sprintf(szDebugMsg, "LRC NOT OK");
		}
		
		inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
	}

	/* 是否Send ACK */
	if (uszSendAck)
	{
		
		/* SN是否核對正確 */
		if (uszSN)
		{
			if (uszLRC == VS_TRUE)
			{
				inMultiFunc_RS232_Send_ACKorNAK(&gstMultiOb, _ACK_);
			}
			else
			{
				inMultiFunc_RS232_Send_ACKorNAK(&gstMultiOb, _NAK_);

				return (VS_ERROR);
			}
		}
		else
		{
			inMultiFunc_RS232_Send_ACKorNAK(&gstMultiOb, _FS_);

			return (VS_ERROR);
		}
	}
	else
	{

	}
	
	/* 執行重新開機命令 */
	if (uszReStart == VS_TRUE)
	{
		inFunc_Reboot();
	}

	memcpy(&szGetMultiData[0], &uszRecBuf[1], inSize - 3);
	szGetMultiData[inSize - 3] = 0x00;
     
	return (VS_SUCCESS);
}
/*
Function        :inMultiFunc_SlaveData_Unpack_Header
Date&Time       :2017/7/13 下午 3:53
Describe        :分析前30Bytes
 */
int inMultiFunc_SlaveData_Unpack_Header(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, MULTI_TABLE *stMultiOb)
{
	int	inTotalSize = 0;
	char	szTemplate[6 + 1];
	char	szDebugMsg[100 + 1];

	/* Bank (4 Bytes) */
	memset(stMultiOb->stMulti_TransData.szTranHost, 0x00, sizeof (stMultiOb->stMulti_TransData.szTranHost));
	memcpy(stMultiOb->stMulti_TransData.szTranHost, &szDataBuffer[inTotalSize], 4);
	inTotalSize += 4;

	/* EDC SN (6 Bytes) */
	memset(stMultiOb->stMulti_TransData.szTermSN, 0x00, sizeof (stMultiOb->stMulti_TransData.szTermSN));
	memcpy(stMultiOb->stMulti_TransData.szTermSN, &szDataBuffer[inTotalSize], 6);
	inTotalSize += 6;

	/* Response Code (4 Bytes) */
	memset(stMultiOb->stMulti_TransData.szErrorCode, 0x00, sizeof (stMultiOb->stMulti_TransData.szErrorCode));
	memcpy(stMultiOb->stMulti_TransData.szErrorCode, &szDataBuffer[inTotalSize], 4);
	inTotalSize += 4;

	/* Trans Type(功能別)(2 Bytes) */
	memset(stMultiOb->stMulti_TransData.szTransType, 0x00, sizeof (stMultiOb->stMulti_TransData.szTransType));
	memcpy(stMultiOb->stMulti_TransData.szTransType, &szDataBuffer[inTotalSize], 2);
	inTotalSize += 2;
	
	if (ginDisplayDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "功能別: %s", stMultiOb->stMulti_TransData.szTransType);
		inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
	}

	/* Total Packet (2 Bytes) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(szTemplate, &szDataBuffer[inTotalSize], 2);
	
	stMultiOb->stMulti_TransData.inTotalPacketNum = 0;
	stMultiOb->stMulti_TransData.inTotalPacketNum = atoi(szTemplate);
	inTotalSize += 2;

	/* Sub Packet Index (2 Bytes) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(szTemplate, &szDataBuffer[inTotalSize], 2);
	
	stMultiOb->stMulti_TransData.inSubPacketNum = 0;
	stMultiOb->stMulti_TransData.inSubPacketNum = atoi(szTemplate);
	inTotalSize += 2;

	/* Total Packet Size (6 Bytes) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(szTemplate, &szDataBuffer[inTotalSize], 6);
	
	stMultiOb->stMulti_TransData.lnTotalPacketSize = 0;
	stMultiOb->stMulti_TransData.lnTotalPacketSize = atoi(szTemplate);
	inTotalSize += 6;

	/* Sub Packet Size (4 Bytes) - 最後再算，動態 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(szTemplate, &szDataBuffer[inTotalSize], 4);
	
	stMultiOb->stMulti_TransData.inSubPacketSize = 0;
	stMultiOb->stMulti_TransData.inSubPacketSize = atoi(szTemplate);
	inTotalSize += 4;

	return (VS_SUCCESS);
}

/*
Function        :inMultiFunc_SlavePackResult
Date&Time       :2017/7/3 下午 5:20
Describe        :
*/
int inMultiFunc_SlavePackResult(TRANSACTION_OBJECT *pobTran, char *szPackData, MULTI_TABLE * stMultiFuncOb)
{
	int	inTotalSize = 0, inSubIndex = 0, inSubSize = 0, inLenlocation = 26;
	char	szTemplate[50];

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inMultiFunc_SlavePackResult() START !");
	}
	
	/* Bank (4 Bytes) */
	memcpy(&szPackData[inTotalSize], &stMultiFuncOb->stMulti_TransData.szTranHost[0], 4);
	inTotalSize += 4;

	/* EDC SN (6 Bytes) */
	memset(szTemplate, 0x00, sizeof (szTemplate));
	memcpy(szTemplate, stMultiFuncOb->stMulti_TransData.szTermSN, 6);
	memcpy(&szPackData[inTotalSize], szTemplate, 6);
	inTotalSize += 6;

	/* Response Code (4 Bytes) */
	if (stMultiFuncOb->stMulti_TransData.inErrorType == _ECR_RESPONSE_CODE_SUCCESS_)
	{
		if (pobTran->uszTwoTapBit == VS_TRUE)
			memcpy(&szPackData[inTotalSize], "1111", 4);
		else
			memcpy(&szPackData[inTotalSize], "0000", 4);
	}
	else if (stMultiFuncOb->stMulti_TransData.inErrorType == _ECR_RESPONSE_CODE_USER_TERMINATE_ERROR_)
	{
		memcpy(&szPackData[inTotalSize], "0002", 4);
	}
	else if (stMultiFuncOb->stMulti_TransData.inErrorType == _ECR_RESPONSE_CODE_TIMEOUT_)
	{
		memcpy(&szPackData[inTotalSize], "0003", 4);
	}
	else if (stMultiFuncOb->stMulti_TransData.inErrorType == _ECR_RESPONSE_CODE_CTLS_HOST_CANCEL_)
	{
		memcpy(&szPackData[inTotalSize], "0004", 4);
	}
	else if (stMultiFuncOb->stMulti_TransData.inErrorType == _ECR_RESPONSE_CODE_SMARTPAY_ERROR_)
	{
		memcpy(&szPackData[inTotalSize], "0005", 4);
	}
	else if (stMultiFuncOb->stMulti_TransData.inErrorType == _ECR_RESPONSE_CODE_PIN_BYPASS_)
	{
		memcpy(&szPackData[inTotalSize], "0006", 4);
	}
	else if (stMultiFuncOb->stMulti_TransData.inErrorType == _ECR_RESPONSE_CODE_END_INQUIRY_SUCCESS_)
	{
		memcpy(&szPackData[inTotalSize], "0000", 4);
	}
	else
	{
		memcpy(&szPackData[inTotalSize], "0001", 4);
	}

	inTotalSize += 4;

	/* Trans Type (2 Bytes) */
	memcpy(&szPackData[inTotalSize], &stMultiFuncOb->stMulti_TransData.szTransType[0], 2);
	inTotalSize += 2;

	/* Total Packet (2 Bytes) */
	memset(szTemplate, 0x00, sizeof (szTemplate));
	sprintf(szTemplate, "%02d", stMultiFuncOb->stMulti_TransData.inTotalPacketNum);
	memcpy(&szPackData[inTotalSize], &szTemplate[0], 2);
	inTotalSize += 2;

	/* Sub Packet Index (2 Bytes) */
	memset(szTemplate, 0x00, sizeof (szTemplate));
	sprintf(szTemplate, "%02d", stMultiFuncOb->stMulti_TransData.inSubPacketNum);
	memcpy(&szPackData[inTotalSize], &szTemplate[0], 2);
	inTotalSize += 2;

	/* Total Packet Size (6 Bytes) */
	memset(szTemplate, 0x00, sizeof (szTemplate));
	sprintf(szTemplate, "%06ld", stMultiFuncOb->stMulti_TransData.lnTotalPacketSize);
	memcpy(&szPackData[inTotalSize], &szTemplate[0], 6);
	inTotalSize += 6;

	/* Sub Packet Size (4 Bytes) - 最後再算，動態 */
	inLenlocation = inTotalSize;
	inTotalSize += 4;

	/* SubData by TranCode (0, 100 or 990) */
	inSubIndex = inTotalSize;
	if (!memcmp(stMultiFuncOb->stMulti_TransData.szTransType, _MULTI_PIN_, 2))
	{
		/* TMK Key Index (2 Bytes) */
		inSubIndex += 2;
		inSubSize += 2;

		/* PIN Key Value (32 Bytes) */
		inSubIndex += 32;
		inSubSize += 32;

		/* PIN Block (16 Bytes) */
		memcpy(&szPackData[inSubIndex], &pobTran->szPIN[0], 16);
		inSubIndex += 16;
		inSubSize += 16;

		/* Amount (12 Bytes) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010ld00", pobTran->srBRec.lnTxnAmount);
		memcpy(&szPackData[inSubIndex], szTemplate, 16);
		inSubIndex += 12;
		inSubSize += 12;

		inTotalSize += _MULTI_SUB_SIZE_SMALL_;
	}
	else if (!memcmp(stMultiFuncOb->stMulti_TransData.szTransType, _MULTI_CTLS_, 2))
	{
		/* CTLS Data(Max 990 Bytes) */
		memcpy(&szPackData[inSubIndex], &gszMultiCTLSData[0], ginMultiCTLSLen);
		inSubIndex += ginMultiCTLSLen;
		inSubSize += ginMultiCTLSLen;

		inTotalSize += _MULTI_SUB_SIZE_MAX_;
	}
	else if (!memcmp(stMultiFuncOb->stMulti_TransData.szTransType, _MULTI_SIGNPAD_, 2))
	{
		inSubSize += _MULTI_SUB_SIZE_MAX_;
		inTotalSize += _MULTI_SUB_SIZE_MAX_;
	}
	else if (!memcmp(stMultiFuncOb->stMulti_TransData.szTransType, _MULTI_SIGN_CONFIRM_, 2))
	{
		inTotalSize += _MULTI_SUB_SIZE_NONE_;
	}

	/* Sub Packet Size (4 Bytes) */
	memset(szTemplate, 0x00, sizeof (szTemplate));
	sprintf(szTemplate, "%04d", inSubSize);
	memcpy(&szPackData[inLenlocation], &szTemplate[0], 4);

	return (inTotalSize);
}

/*
Function        :inMultiFunc_TimeCheck
Date&Time       :2017/7/10 下午 2:35
Describe        :
*/
int inMultiFunc_TimeCheck(char* szDate, char* szTime)
{
        char	szVaildDateTime[14 + 1];
        char	szDebugMsg[100 + 1];
	
        if (inMultiFunc_ValidDate(szDate) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }
	
        if (inMultiFunc_ValidTime(szTime) != VS_SUCCESS)
        {
               return (VS_ERROR);
        }
	
	if (ginDisplayDebug == VS_TRUE)
	{
		memset(szVaildDateTime, 0x00, sizeof(szVaildDateTime));
		sprintf(szVaildDateTime, "%s%s", szDate, szTime);
		inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
	}
        
	
        return (VS_SUCCESS);        
}

/*
Function        :inEDC_ValidDate
Date&Time       :2017/7/10 下午 2:37
Describe        :不檢查閏年、和大小月
*/
int inMultiFunc_ValidDate(char *szDate)
{
	int		inMaxDay[12] = {/* 一月 */31, /* 二月 */28, /* 三月 */31, /* 四月 */30, /* 五月 */31, /* 六月 */30, /* 七月 */31, /* 八月 */31, /* 九月 */30, /* 十月 */31, /* 十一月 */30, /* 十二月 */31};
	int		inFinalMaxDay = 0;
	int		inDay = 0, inMon = 0, inYear = 0;
	char		szTemplate[14 + 1];
	unsigned char	uszLeapYear = VS_FALSE;
	
	/* 判斷閏年 */
	if (inYear % 4 == 0)		
		uszLeapYear = VS_TRUE;
	else
		uszLeapYear = VS_FALSE;
	
	/* Get 4-Digit Year */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(&szTemplate[0], &szDate[0], 4);
	inYear = atoi(szTemplate);
	/*  Get 2-Digit Month */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(&szTemplate[0], &szDate[4], 2);
	inMon = atoi(szTemplate);
	/*  Get 2-Digit Day */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(&szTemplate[0], &szDate[6], 2);
	inDay = atoi(szTemplate);

	/* 超出該刷卡機使用年限 */
	if ((inYear < 1998) || (inYear > 2088))
	{
		return (VS_ERROR);
	}

	/* 月份不合法 */
	if ((inMon < 1) || (inMon > 12))
	{
		return (VS_ERROR);
	}
	
	/* 如果是閏年，要加一天 */
	if (uszLeapYear == VS_TRUE)
	{
		inFinalMaxDay = inMaxDay[inMon - 1] + 1;
	}
	else
	{
		inFinalMaxDay = inMaxDay[inMon - 1];
	}
	
	/* 日期不合法 */
	if ((inDay < 1) || (inDay > inFinalMaxDay))
	{
		return (VS_ERROR);
	}

	return(VS_SUCCESS);
}

/*
Function        :inMultiFunc_ValidTime
Date&Time       :2017/7/10 下午 4:14
Describe        :
*/
int inMultiFunc_ValidTime(char *szTime)
{
	char	szTemplate[4 + 1];
	int 	inMins = 0, inHours = 0, inSecs = 0;
	
	/*  Get 2-Digit Hours */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(&szTemplate[0], &szTime[0], 2);
	inHours = atoi(szTemplate);
    	/*  Get 2-Digit Minutes */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(&szTemplate[0], &szTime[2], 2);
	inMins = atoi(szTemplate);
	/*  Get 2-Digit Seconds */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(&szTemplate[0], &szTime[4], 2);
	inSecs = atoi(szTemplate);

	/* 小時不合法 */
	if ((inHours < 0) || (inHours > 23))
	{
		return (VS_ERROR);
	}

	/* 分鐘不合法 */
	if ((inMins < 0) || (inMins > 59))
	{
		return (VS_ERROR);
	}

	/* 秒數不合法 */
	if ((inSecs < 0) || (inSecs > 59))
	{
		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

/*
Function        :inMultiFunc_Response_Host
Date&Time       :2017/7/10 下午 4:18
Describe        :在這裡決定Timeout和封包長度
*/
int inMultiFunc_Response_Host(TRANSACTION_OBJECT *pobTran, MULTI_TABLE* stMultiTable)
{
        int	inTimeout = 0;
	int	inTranCode = 0;
	
	inTranCode = atoi(stMultiTable->stMulti_TransData.szTransType);
        
	stMultiTable->stMulti_TransData.inTotalPacketNum = 1;
	stMultiTable->stMulti_TransData.inSubPacketNum = 1;
         
        switch (inTranCode)
        {
                case _MULTI_PIN_NO_ :       
			memcpy(&stMultiTable->stMulti_TransData.szTransType[0], _MULTI_PIN_, 2);
                        stMultiTable->stMulti_TransData.lnTotalPacketSize = _MULTI_SUB_SIZE_SMALL_;  /* Sub Size 100 */
                        inTimeout = 200;
                        break; 
                case _MULTI_CTLS_NO_ :     
                        memcpy(&stMultiTable->stMulti_TransData.szTransType[0], _MULTI_CTLS_, 2);
                        stMultiTable->stMulti_TransData.lnTotalPacketSize = _MULTI_SUB_SIZE_MAX_;    /* Sub Size 990 */
                        inTimeout = 100;
                        break;   
                case _MULTI_EXCHANGE_NO_ :   
                        memcpy(&stMultiTable->stMulti_TransData.szTransType[0], _MULTI_EXCHANGE_, 2);
                        stMultiTable->stMulti_TransData.lnTotalPacketSize = _MULTI_SUB_SIZE_NONE_;   /* Sub Size 0 */
                        inTimeout = 100;
                        break;
                case _MULTI_SIGNPAD_NO_ : 
                        memcpy(&stMultiTable->stMulti_TransData.szTransType[0], _MULTI_SIGNPAD_, 2);
                        inTimeout = 1000;
                        break;
                case _MULTI_SIGN_CONFIRM_NO_ :
			memcpy(&stMultiTable->stMulti_TransData.szTransType[0], _MULTI_SIGN_CONFIRM_, 2);
                        stMultiTable->stMulti_TransData.lnTotalPacketSize = _MULTI_SUB_SIZE_NONE_;   /* Sub Size 0 */
                        inTimeout = 200;
                        break;   
                case _MULTI_SLAVE_REBOOT_NO_ :
                case _MULTI_NOSIGN_NO_ :    
                case _MULTI_TMS_CAPK_NO_ :   
                case _MULTI_TMS_MASTER_NO_ : 
                case _MULTI_TMS_VISA_NO_ :   
                case _MULTI_TMS_JCB_NO_ :    
                case _MULTI_TMS_CUP_NO_ :    /*20151014浩瑋新增*/    /* 【需求單 - 104094】銀聯閃付需求 by Wei Hsiu - 2015/8/13 下午 10:12:06 */	
                case _MULTI_POLL_NO_ :
			break;
                default :
                        return (VS_ERROR);
        }
                
        return (VS_SUCCESS);
}

/*
Function        :inMultiFunc_GetCardFields_CTLS
Date&Time       :2017/7/4 下午 2:26
Describe        :For 外接設備的感應流程
*/
int inMultiFunc_GetCardFields_CTLS(TRANSACTION_OBJECT *pobTran, MULTI_TABLE *stMultiOb)
{
	int		inRetVal;
	int		inCTLS_RetVal = -1;	/* 感應卡事件的反應，怕和其他用到inRetVal的搞混所以獨立出來*/
	int		inMultiFunc_RetVal = -1;
	char		szKey = -1;
	char		szTemplate[_DISP_MSG_SIZE_ + 1];
	char		szDebugMsg[100 + 1];
	long		lnTimeout;
	unsigned long   ulCTLS_RetVal;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inMultiFunc_GetCardFields_CTLS() START !");
	}

	/* 過卡方式參數初始化  */
	pobTran->srBRec.uszManualBit = VS_FALSE;
	/* 初始化 ginEventCode */
	ginEventCode = -1;
	
	/* Send CTLS Readly for Sale Command */
	if (inCTLS_SendReadyForSale_Flow(pobTran) != VS_SUCCESS)
		return (VS_ERROR);
	
	/* 顯示 請感應卡片 */
	inDISP_PutGraphic(_CTLS_LOGO_, 0, _COORDINATE_Y_LINE_8_1_);
        
        /* 進入畫面時先顯示金額 */
	if (pobTran->srBRec.lnTxnAmount > 0)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
		inFunc_Amount_Comma(szTemplate, "NT$ " , ' ', _SIGNED_NONE_, 16, VS_TRUE);
		inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_3_, _COLOR_RED_, _DISP_LEFT_);
	}
		
        /* 設定Timeout */
	if (pobTran->uszECRBit == VS_TRUE)
	{
		lnTimeout = _ECR_RS232_GET_CARD_TIMEOUT_;
	}
	else if (pobTran->uszMultiFuncSlaveBit == VS_TRUE)
	{
		lnTimeout = stMultiOb->stMulti_TransData.inCTLS_Timeout;
	}
	else
	{
		lnTimeout = 30;
	}
	/* inSecond剩餘倒數時間 */
	inDISP_TimeoutStart(lnTimeout);
	/* 立既顯示倒數秒數 */
	inDISP_LiveCountdown(lnTimeout,_FONTSIZE_8X16_, _LINE_8_6_, _DISP_RIGHT_);
	
	while (1)
        {
                
		/* 感應事件 */
		if (ginEventCode == _SENSOR_EVENT_)
		{		
			/* 收到的retuen不是等待資料的狀態就去解析 */
			ulCTLS_RetVal = ulMultiFunc_CheckResponseCode_SALE(pobTran);
			
			if (ulCTLS_RetVal == d_EMVCL_RC_NO_CARD)
                        {
				/* Timeout沒卡 */
				return (VS_ERROR);
			}
			else if (ulCTLS_RetVal == d_EMVCL_RC_MORE_CARDS)
			{
				pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_NOT_ONE_CARD_;
				
				return (VS_WAVE_ERROR);
			}
                        /* 這邊要切SmartPay */
                        else if (ulCTLS_RetVal == d_EMVCL_NON_EMV_CARD)
                        {
				if (pobTran->srBRec.inTxnResult != VS_SUCCESS)
					 return (VS_ERROR);
                               
				/* 轉 FISC_SALE */
				/* FISC incode 在 inFISC_CTLSProcess內設定 */
                        }
                        /* 走信用卡流程 */
                        else if (ulCTLS_RetVal == d_EMVCL_RC_DATA)
                        {
				
                        }
			/*  Two Tap 流程 */
			else if (ulCTLS_RetVal == d_EMVCL_RC_SEE_PHONE)
			{
				/* 重Send命令，等第二次感應 */
				/* Send CTLS Readly for Sale Command */
				if (inCTLS_SendReadyForSale_Flow(pobTran) != VS_SUCCESS)
					return (VS_ERROR);
				
				ginEventCode = -1;
				continue;
			}
			/* 感應錯誤 */
			else
			{
				pobTran->inErrorMsg = _ERROR_CODE_V3_MULTI_FUNC_CTLS_;
				if (ginDisplayDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "感應失敗");
					inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_TRUE);
				}
				return (VS_ERROR);
			}
			return (VS_SUCCESS);
		}
		
		if (ginEventCode == _MULTIFUNC_SLAVE_EVENT_)
		{
			if (ginDisplayDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "_MULTIFUNC_SLAVE_EVENT_");
				inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_TRUE);
			}
			
			inRetVal = inMultiFunc_First_Receive_Cancel();
			if (inRetVal == VS_SUCCESS)
			{
				return (VS_ESCAPE);
			}
			else
			{
				/* 判斷為雜值要清空，再等待下一次 */
				inMultiFunc_RS232_FlushRxBuffer();
			}
		}
                
		while (1)
		{
			ginEventCode = -1;
			
			/* ------------偵測感應卡------------------ */
			inCTLS_RetVal = inCTLS_ReceiveReadyForSales_Flow(pobTran);
			if (pobTran->uszMenuSelectCancelBit == VS_TRUE)
			{
				/* 被按取消了 */
				return (VS_ESCAPE);
			}
			else
			{
				if (inCTLS_RetVal == VS_SUCCESS)
				{
					/* 感應卡事件 */
					ginEventCode = _SENSOR_EVENT_;
				}
			}
			
			/* ------------MasterTerminal---------------*/
			/* 偵測多接設備收到資料 */
			inMultiFunc_RetVal = inMultiFunc_First_Receive_Check();
			if (inMultiFunc_RetVal == VS_SUCCESS)
			{
				ginEventCode = _MULTIFUNC_SLAVE_EVENT_;
			}
			
			/* ------------偵測key in------------------ */
			szKey = -1;
			szKey = uszKBD_Key();
			
			/* 感應倒數時間 && Display Countdown */
			if (inDISP_TimeoutCheck(_FONTSIZE_8X16_, _LINE_8_6_, _DISP_RIGHT_) == VS_TIMEOUT)
			{
				/* 感應時間到Timeout */
				szKey = _KEY_TIMEOUT_;
			}

			if (szKey == _KEY_TIMEOUT_)
			{
				/* 因CTLS Timeout參數在XML內，所以將CTLS API Timeout時間拉長至60秒，程式內直接用取消交易來模擬TimeOut效果 */
				inCTLS_CancelTransacton_Flow();
				/* Timeout or Cancel */
				return (VS_TIMEOUT);
			}
			/* 外接設備不允許按取消鍵 */
			else if ((szKey >= '0' && szKey <= '9') || (szKey == _KEY_ENTER_))
			{
				/* key in事件 */
				ginEventCode = _MENUKEYIN_EVENT_;
			}
			
			/* 有事件發生，跳出迴圈做對應反應 */
			if (ginEventCode != -1)
			{
				break;
			}
			
		}/* while (1) 偵測事件迴圈...*/
		
		
	} /* while (1) 對事件做回應迴圈...*/

        return (VS_SUCCESS);
}

/*
Function        :inMultiFunc_GetCardFields_Refund_CTLS
Date&Time       :2017/9/19 上午 10:17
Describe        :For外接設備的感應退貨流程
*/
int inMultiFunc_GetCardFields_Refund_CTLS(TRANSACTION_OBJECT *pobTran, MULTI_TABLE *stMultiOb)
{
	int		inRetVal;
	int		inCTLS_RetVal = -1;	/* 感應卡事件的反應，怕和其他用到inRetVal的搞混所以獨立出來*/
	int		inMultiFunc_RetVal = -1;
        char		szKey = -1;
        char		szTemplate[_DISP_MSG_SIZE_ + 1];
	char		szDebugMsg[100 + 1];
	long		lnTimeout;
        unsigned long   ulCTLS_RetVal;

        /* 過卡方式參數初始化  */
        pobTran->srBRec.uszManualBit = VS_FALSE;
	/* 初始化 ginEventCode */
        ginEventCode = -1;
	
	/* Send CTLS Readly for Refund Command */
	if (inCTLS_SendReadyForRefund_Flow(pobTran) != VS_SUCCESS)
		return (VS_ERROR);

        /* 顯示 請感應卡片 */
	inDISP_PutGraphic(_CTLS_LOGO_2_, 0, _COORDINATE_Y_LINE_8_1_);
        
        /* 進入畫面時先顯示金額 */
	if (pobTran->srBRec.lnTxnAmount > 0)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
		inFunc_Amount_Comma(szTemplate, "NT$ " , ' ', _SIGNED_NONE_, 16, VS_TRUE);
		inDISP_EnglishFont_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_3_, _COLOR_RED_, _DISP_LEFT_);
	}
        
        /* 設定Timeout */
	if (pobTran->uszECRBit == VS_TRUE)
	{
		lnTimeout = _ECR_RS232_GET_CARD_TIMEOUT_;
	}
	else if (pobTran->uszMultiFuncSlaveBit == VS_TRUE)
	{
		lnTimeout = stMultiOb->stMulti_TransData.inCTLS_Timeout;
	}
	else
	{
		lnTimeout = 30;
	}
	/* inSecond剩餘倒數時間 */
	inDISP_TimeoutStart(lnTimeout);
	/* 立既顯示倒數秒數 */
	inDISP_LiveCountdown(lnTimeout,_FONTSIZE_8X16_, _LINE_8_6_, _DISP_RIGHT_);
	
	while (1)
        {
		/* 感應事件 */
		if (ginEventCode == _SENSOR_EVENT_)
		{		
			/* 收到的retuen不是等待資料的狀態就去解析 */
			ulCTLS_RetVal = ulMultiFunc_CheckResponseCode_REFUND(pobTran);
			
			if (ulCTLS_RetVal == d_EMVCL_RC_NO_CARD)
                        {
				/* Timeout沒卡 */
				return (VS_ERROR);
			}
			else if (ulCTLS_RetVal == d_EMVCL_RC_MORE_CARDS)
			{
				pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_NOT_ONE_CARD_;
				
				return (VS_WAVE_ERROR);
			}
                        /* 這邊要切SmartPay */
                        else if (ulCTLS_RetVal == d_EMVCL_NON_EMV_CARD)
                        {
				if (pobTran->srBRec.inTxnResult != VS_SUCCESS)
					 return (VS_ERROR);
                               
				/* 轉 FISC_REFUND */
				/* FISC incode 在 inFISC_CTLSProcess內設定 */
                        }
                        /* 走信用卡流程 */
                        else if (ulCTLS_RetVal == d_EMVCL_RC_DATA)
                        {
				
                        }
			/*  Two Tap 流程 */
			else if (ulCTLS_RetVal == d_EMVCL_RC_SEE_PHONE)
			{
				/* 重Send命令，等第二次感應 */
				/* Send CTLS Readly for Refund Command */
				if (inCTLS_SendReadyForRefund_Flow(pobTran) != VS_SUCCESS)
					return (VS_ERROR);
				
				ginEventCode = -1;
				continue;
			}
			/* 感應錯誤 */
			else
			{
				pobTran->inErrorMsg = _ERROR_CODE_V3_MULTI_FUNC_CTLS_;
				if (ginDisplayDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "感應失敗");
					inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_TRUE);
				}
				return (VS_ERROR);
			}
			return (VS_SUCCESS);
		}
		
		if (ginEventCode == _MULTIFUNC_SLAVE_EVENT_)
		{
			if (ginDisplayDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "_MULTIFUNC_SLAVE_EVENT_");
				inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_TRUE);
			}
			
			inRetVal = inMultiFunc_First_Receive_Cancel();
			if (inRetVal == VS_SUCCESS)
			{
				return (VS_ESCAPE);
			}
			else
			{
				/* 判斷為雜值要清空，再等待下一次 */
				inMultiFunc_RS232_FlushRxBuffer();
			}
		}
                
		while (1)
		{
			ginEventCode = -1;
			/* ------------偵測感應卡------------------ */
			inCTLS_RetVal = inCTLS_ReceiveReadyForRefund_Flow(pobTran);
			if (inCTLS_RetVal == VS_SUCCESS)
			{
				/* 感應卡事件 */
				ginEventCode = _SENSOR_EVENT_;
			}
			
			/* ------------MasterTerminal---------------*/
			/* 偵測多接設備收到資料 */
			inMultiFunc_RetVal = inMultiFunc_First_Receive_Check();
			if (inMultiFunc_RetVal == VS_SUCCESS)
			{
				ginEventCode = _MULTIFUNC_SLAVE_EVENT_;
			}

			
			/* ------------偵測key in------------------ */
			szKey = -1;
			szKey = uszKBD_Key();
			
			/* 感應倒數時間 && Display Countdown */
			if (inDISP_TimeoutCheck(_FONTSIZE_8X16_, _LINE_8_6_, _DISP_RIGHT_) == VS_TIMEOUT)
			{
				/* 感應時間到Timeout */
				szKey = _KEY_TIMEOUT_;
			}

			if (szKey == _KEY_TIMEOUT_)
			{
				/* 因CTLS Timeout參數在XML內，所以將CTLS API Timeout時間拉長至60秒，程式內直接用取消交易來模擬TimeOut效果 */
				inCTLS_CancelTransacton_Flow();
				/* Timeout or Cancel */
				return (VS_TIMEOUT);
			}
			/* 外接設備不允許按取消鍵 */
			else if ((szKey >= '0' && szKey <= '9') || (szKey == _KEY_ENTER_))
			{
				/* key in事件 */
				ginEventCode = _MENUKEYIN_EVENT_;
			}
			
			/* 有事件發生，跳出迴圈做對應反應 */
			if (ginEventCode != -1)
			{
				break;
			}
			
		}/* while (1) 偵測事件迴圈...*/
		
		
	} /* while (1) 對事件做回應迴圈...*/

        return (VS_SUCCESS);
}

unsigned long ulMultiFunc_CheckResponseCode_SALE(TRANSACTION_OBJECT *pobTran)
{
	char		szDebugMsg[100 + 1];
        unsigned long	ulRetVal;
	
        switch (srCtlsObj.lnSaleRespCode)
	{
		/* d_EMVCL_RC_DATA 和 d_EMVCL_NON_EMV_CARD 為有收到資料的狀況 */
                case d_EMVCL_RC_DATA :
                        pobTran->srBRec.uszContactlessBit = VS_TRUE;
			ulRetVal = srCtlsObj.lnSaleRespCode;
                        break;
                case d_EMVCL_NON_EMV_CARD :
                        /* SmartPay會跑這個case */
                        pobTran->srBRec.uszContactlessBit = VS_TRUE;
                        ulRetVal = srCtlsObj.lnSaleRespCode;
                        break;
		/* 感應無效 */
                case d_EMVCL_RC_FAILURE :
                        if (ginDebug == VS_TRUE)
			{
                                inLogPrintf(AT, "CTLS Resp Code = d_EMVCL_RC_FAILURE");
			}
			if (ginDisplayDebug == VS_TRUE)
			{
				inDISP_LOGDisplay("CTLS d_EMVCL_RC_FAILURE", _FONTSIZE_16X22_, _LINE_16_16_, VS_TRUE);
			}
			
			pobTran->inErrorMsg = _ERROR_CODE_V3_MULTI_FUNC_CTLS_;
                        ulRetVal = srCtlsObj.lnSaleRespCode;
                        break;
		/* 請問d_EMVCL_RC_FAILURE和d_EMVCL_RC_FALLBACK的差別在哪？
		   [Ans] : 這兩個 Return code 都是表示交易的過程中發生了問題而交易終止了。差別在於 Kernel 是否要求要換另一個介面的交易 (例如 : CL 交易失敗，同一張卡改成 CT 或是 MSR 的介面交易)，當然這個轉換介面的要求是根據各個 payment 的規格而定的。
			   d_EMVCL_RC_FAILURE : 交易中止
			   d_EMVCL_RC_FALLBACK : 交易中止，但嘗試別的介面交易。
		 */
		/* 感應中止，改插卡或刷卡 */
                case d_EMVCL_RC_FALLBACK :
                        if (ginDebug == VS_TRUE)
			{
                                inLogPrintf(AT, "CTLS Resp Code = d_EMVCL_RC_FALLBACK");
			}
			if (ginDisplayDebug == VS_TRUE)
			{
				inDISP_LOGDisplay("CTLS d_EMVCL_RC_FALLBACK", _FONTSIZE_16X22_, _LINE_16_16_, VS_TRUE);
			}
			
                        ulRetVal = srCtlsObj.lnSaleRespCode;
			pobTran->inErrorMsg = _ERROR_CODE_V3_MULTI_FUNC_CTLS_;
                        break;
		/* 請重試 */
		case d_EMVCL_TRY_AGAIN :
                        if (ginDebug == VS_TRUE)
			{
                                inLogPrintf(AT, "CTLS Resp Code = d_EMVCL_RC_MORE_CARDS");
			}
			if (ginDisplayDebug == VS_TRUE)
			{
				inDISP_LOGDisplay("CTLS d_EMVCL_RC_MORE_CARDS", _FONTSIZE_16X22_, _LINE_16_16_, VS_TRUE);
			}
			
			/* 依目前聯合線上機器，只要一失敗就跳感應失敗，請改刷卡或插卡 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_MULTI_FUNC_CTLS_;
                        ulRetVal = srCtlsObj.lnSaleRespCode;
                        break;
		/* 取消的話，基本上會直接跳出迴圈，所以也不會進這裡 */
                case d_EMVCL_TX_CANCEL :
                        if (ginDebug == VS_TRUE)
			{
                                inLogPrintf(AT, "CTLS Resp Code = d_EMVCL_TX_CANCEL");
			}
			if (ginDisplayDebug == VS_TRUE)
			{
				inDISP_LOGDisplay("CTLS d_EMVCL_TX_CANCEL", _FONTSIZE_16X22_, _LINE_16_16_, VS_TRUE);
			}
			
			pobTran->inErrorMsg = _ERROR_CODE_V3_USER_CANCEL_;
                        ulRetVal = srCtlsObj.lnSaleRespCode;
                        break;
		/* 多卡重疊 */
                case d_EMVCL_RC_MORE_CARDS :
                        if (ginDebug == VS_TRUE)
			{
                                inLogPrintf(AT, "CTLS Resp Code = d_EMVCL_RC_MORE_CARDS");
			}
			if (ginDisplayDebug == VS_TRUE)
			{
				inDISP_LOGDisplay("CTLS d_EMVCL_RC_MORE_CARDS", _FONTSIZE_16X22_, _LINE_16_16_, VS_TRUE);
			}
			
			/* 依目前聯合線上機器，只要一失敗就跳感應失敗，請改刷卡或插卡 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_MULTI_FUNC_CTLS_;
                        ulRetVal = srCtlsObj.lnSaleRespCode;
                        break;
		/* Timeout 沒感應到卡，這個Timeout指的是xml檔內設定，目前設定到最大，理論上不會出現此回應 */
                case d_EMVCL_RC_NO_CARD :
                        if (ginDebug == VS_TRUE)
			{
                                inLogPrintf(AT, "CTLS Resp Code = d_EMVCL_RC_NO_CARD");
			}
			if (ginDisplayDebug == VS_TRUE)
			{
				inDISP_LOGDisplay("CTLS d_EMVCL_RC_NO_CARD", _FONTSIZE_16X22_, _LINE_16_16_, VS_TRUE);
			}
			
                        ulRetVal = srCtlsObj.lnSaleRespCode;
                        break;
		/* 基本上pending不會進這裡，而會繼續感應 */
                case d_EMVCL_PENDING :
                        if (ginDebug == VS_TRUE)
			{
                                inLogPrintf(AT, "CTLS Resp Code = d_EMVCL_PENDING");
			}
			if (ginDisplayDebug == VS_TRUE)
			{
				inDISP_LOGDisplay("CTLS d_EMVCL_PENDING", _FONTSIZE_16X22_, _LINE_16_16_, VS_TRUE);
			}
			
                        ulRetVal = srCtlsObj.lnSaleRespCode;
                        break;
		/* 請問d_EMVCL_RC_DEK_SIGNAL這個回應碼是什麼意思？
		   [Ans] : 收到 d_EMVCL_RC_DEK_SIGNAL 表示交易中間 kernel 有訊息要帶出外面給 Application，交易尚未結束，仍須等待 EMVCL_PerformTransactionEx 給出真正的交易結果。
		 */
		case d_EMVCL_RC_DEK_SIGNAL :
                        if (ginDebug == VS_TRUE)
			{
                                inLogPrintf(AT, "CTLS Resp Code = d_EMVCL_RC_DEK_SIGNAL");
			}
			if (ginDisplayDebug == VS_TRUE)
			{
				inDISP_LOGDisplay("CTLS d_EMVCL_RC_DEK_SIGNAL", _FONTSIZE_16X22_, _LINE_16_16_, VS_TRUE);
			}
			
			/* 依目前聯合線上機器，只要一失敗就跳感應失敗，請改刷卡或插卡 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_MULTI_FUNC_CTLS_;
                        ulRetVal = srCtlsObj.lnSaleRespCode;
                        break;
		/* Two Tap流程 */
		case d_EMVCL_RC_SEE_PHONE :
                        if (ginDebug == VS_TRUE)
                                inLogPrintf(AT, "CTLS Resp Code = d_EMVCL_RC_SEE_PHONE");
			/* 請輸密碼或指紋 並再感應一次 */
			inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
			inDISP_PutGraphic(_CTLS_TWO_TAP_, 0, _COORDINATE_Y_LINE_8_7_);
			pobTran->uszTwoTapBit = VS_TRUE;
			
			ulRetVal = srCtlsObj.lnSaleRespCode;
                        break;
                default :
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "CTLS Res: %08lX", srCtlsObj.lnSaleRespCode);
				inLogPrintf(AT, szDebugMsg);
			}
			if (ginDisplayDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "CTLS Res: %08lX", srCtlsObj.lnSaleRespCode);
				inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
			}
			ulRetVal = d_EMVCL_RC_FAILURE;
			break;
        }
	
	if (srCtlsObj.lnSaleRespCode == d_EMVCL_NON_EMV_CARD)
	{
		if (inMultiFunc_Process_SmartPay_Data(pobTran) != VS_SUCCESS)
		{
			/* 組失敗也當感應失敗 */
			ulRetVal = d_EMVCL_RC_FAILURE;
		}
	}
	else if (srCtlsObj.lnSaleRespCode == d_EMVCL_RC_DATA)
	{
		/* 準備處理要送給Master的感應資料 */
		if (inMultiFunc_Process_CTLS_DATA() != VS_SUCCESS)
		{
			/* 組失敗也當感應失敗 */
			ulRetVal = d_EMVCL_RC_FAILURE;
		}
	}
	else
	{
		inMultiFunc_Process_CTLS_ERROR_DATA();
	}
        
        return (ulRetVal);
}

unsigned long ulMultiFunc_CheckResponseCode_REFUND(TRANSACTION_OBJECT *pobTran)
{
	char		szDebugMsg[100 + 1];
        unsigned long	ulRetVal;
	
        switch (srCtlsObj.lnSaleRespCode)
	{
		/* d_EMVCL_RC_DATA 和 d_EMVCL_NON_EMV_CARD 為有收到資料的狀況 */
                case d_EMVCL_RC_DATA :
                        pobTran->srBRec.uszContactlessBit = VS_TRUE;
			ulRetVal = srCtlsObj.lnSaleRespCode;
                        break;
                case d_EMVCL_NON_EMV_CARD :
                        /* SmartPay會跑這個case */
                        pobTran->srBRec.uszContactlessBit = VS_TRUE;
                        ulRetVal = srCtlsObj.lnSaleRespCode;
                        break;
		/* 感應無效 */
                case d_EMVCL_RC_FAILURE :
                        if (ginDebug == VS_TRUE)
			{
                                inLogPrintf(AT, "CTLS Resp Code = d_EMVCL_RC_FAILURE");
			}
			if (ginDisplayDebug == VS_TRUE)
			{
				inDISP_LOGDisplay("CTLS d_EMVCL_RC_FAILURE", _FONTSIZE_16X22_, _LINE_16_16_, VS_TRUE);
			}
			
			pobTran->inErrorMsg = _ERROR_CODE_V3_MULTI_FUNC_CTLS_;
                        ulRetVal = srCtlsObj.lnSaleRespCode;
                        break;
		/* 請問d_EMVCL_RC_FAILURE和d_EMVCL_RC_FALLBACK的差別在哪？
		   [Ans] : 這兩個 Return code 都是表示交易的過程中發生了問題而交易終止了。差別在於 Kernel 是否要求要換另一個介面的交易 (例如 : CL 交易失敗，同一張卡改成 CT 或是 MSR 的介面交易)，當然這個轉換介面的要求是根據各個 payment 的規格而定的。
			   d_EMVCL_RC_FAILURE : 交易中止
			   d_EMVCL_RC_FALLBACK : 交易中止，但嘗試別的介面交易。
		 */
		/* 感應中止，改插卡或刷卡 */
                case d_EMVCL_RC_FALLBACK :
                        if (ginDebug == VS_TRUE)
			{
                                inLogPrintf(AT, "CTLS Resp Code = d_EMVCL_RC_FALLBACK");
			}
			if (ginDisplayDebug == VS_TRUE)
			{
				inDISP_LOGDisplay("CTLS d_EMVCL_RC_FALLBACK", _FONTSIZE_16X22_, _LINE_16_16_, VS_TRUE);
			}
			
                        ulRetVal = srCtlsObj.lnSaleRespCode;
			pobTran->inErrorMsg = _ERROR_CODE_V3_MULTI_FUNC_CTLS_;
                        break;
		/* 請重試 */
		case d_EMVCL_TRY_AGAIN :
                        if (ginDebug == VS_TRUE)
			{
                                inLogPrintf(AT, "CTLS Resp Code = d_EMVCL_RC_MORE_CARDS");
			}
			if (ginDisplayDebug == VS_TRUE)
			{
				inDISP_LOGDisplay("CTLS d_EMVCL_RC_MORE_CARDS", _FONTSIZE_16X22_, _LINE_16_16_, VS_TRUE);
			}
			
			/* 依目前聯合線上機器，只要一失敗就跳感應失敗，請改刷卡或插卡 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_MULTI_FUNC_CTLS_;
                        ulRetVal = srCtlsObj.lnSaleRespCode;
                        break;
		/* 取消的話，基本上會直接跳出迴圈，所以也不會進這裡 */
                case d_EMVCL_TX_CANCEL :
                        if (ginDebug == VS_TRUE)
			{
                                inLogPrintf(AT, "CTLS Resp Code = d_EMVCL_TX_CANCEL");
			}
			if (ginDisplayDebug == VS_TRUE)
			{
				inDISP_LOGDisplay("CTLS d_EMVCL_TX_CANCEL", _FONTSIZE_16X22_, _LINE_16_16_, VS_TRUE);
			}
			
			pobTran->inErrorMsg = _ERROR_CODE_V3_USER_CANCEL_;
                        ulRetVal = srCtlsObj.lnSaleRespCode;
                        break;
		/* 多卡重疊 */
                case d_EMVCL_RC_MORE_CARDS :
                        if (ginDebug == VS_TRUE)
			{
                                inLogPrintf(AT, "CTLS Resp Code = d_EMVCL_RC_MORE_CARDS");
			}
			if (ginDisplayDebug == VS_TRUE)
			{
				inDISP_LOGDisplay("CTLS d_EMVCL_RC_MORE_CARDS", _FONTSIZE_16X22_, _LINE_16_16_, VS_TRUE);
			}
			
			/* 依目前聯合線上機器，只要一失敗就跳感應失敗，請改刷卡或插卡 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_MULTI_FUNC_CTLS_;
                        ulRetVal = srCtlsObj.lnSaleRespCode;
                        break;
		/* Timeout 沒感應到卡，這個Timeout指的是xml檔內設定，目前設定到最大，理論上不會出現此回應 */
                case d_EMVCL_RC_NO_CARD :
                        if (ginDebug == VS_TRUE)
			{
                                inLogPrintf(AT, "CTLS Resp Code = d_EMVCL_RC_NO_CARD");
			}
			if (ginDisplayDebug == VS_TRUE)
			{
				inDISP_LOGDisplay("CTLS d_EMVCL_RC_NO_CARD", _FONTSIZE_16X22_, _LINE_16_16_, VS_TRUE);
			}
			
                        ulRetVal = srCtlsObj.lnSaleRespCode;
                        break;
		/* 基本上pending不會進這裡，而會繼續感應 */
                case d_EMVCL_PENDING :
                        if (ginDebug == VS_TRUE)
			{
                                inLogPrintf(AT, "CTLS Resp Code = d_EMVCL_PENDING");
			}
			if (ginDisplayDebug == VS_TRUE)
			{
				inDISP_LOGDisplay("CTLS d_EMVCL_PENDING", _FONTSIZE_16X22_, _LINE_16_16_, VS_TRUE);
			}
			
                        ulRetVal = srCtlsObj.lnSaleRespCode;
                        break;
		/* 請問d_EMVCL_RC_DEK_SIGNAL這個回應碼是什麼意思？
		   [Ans] : 收到 d_EMVCL_RC_DEK_SIGNAL 表示交易中間 kernel 有訊息要帶出外面給 Application，交易尚未結束，仍須等待 EMVCL_PerformTransactionEx 給出真正的交易結果。
		 */
		case d_EMVCL_RC_DEK_SIGNAL :
                        if (ginDebug == VS_TRUE)
			{
                                inLogPrintf(AT, "CTLS Resp Code = d_EMVCL_RC_DEK_SIGNAL");
			}
			if (ginDisplayDebug == VS_TRUE)
			{
				inDISP_LOGDisplay("CTLS d_EMVCL_RC_DEK_SIGNAL", _FONTSIZE_16X22_, _LINE_16_16_, VS_TRUE);
			}
			
			/* 依目前聯合線上機器，只要一失敗就跳感應失敗，請改刷卡或插卡 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_MULTI_FUNC_CTLS_;
                        ulRetVal = srCtlsObj.lnSaleRespCode;
                        break;
		/* Two Tap流程 */
		case d_EMVCL_RC_SEE_PHONE :
                        if (ginDebug == VS_TRUE)
                                inLogPrintf(AT, "CTLS Resp Code = d_EMVCL_RC_SEE_PHONE");
			/* 請輸密碼或指紋 並再感應一次 */
			inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
			inDISP_PutGraphic(_CTLS_TWO_TAP_, 0, _COORDINATE_Y_LINE_8_7_);
			pobTran->uszTwoTapBit = VS_TRUE;
			
			ulRetVal = srCtlsObj.lnSaleRespCode;
                        break;
                default :
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "CTLS Res: %08lX", srCtlsObj.lnSaleRespCode);
				inLogPrintf(AT, szDebugMsg);
			}
			if (ginDisplayDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "CTLS Res: %08lX", srCtlsObj.lnSaleRespCode);
				inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
			}
			ulRetVal = d_EMVCL_RC_FAILURE;
			break;
        }
	
	if (srCtlsObj.lnSaleRespCode == d_EMVCL_NON_EMV_CARD)
	{
		if (inMultiFunc_Process_SmartPay_Data(pobTran) != VS_SUCCESS)
		{
			/* 組失敗也當感應失敗 */
			ulRetVal = d_EMVCL_RC_FAILURE;
		}
	}
	else if (srCtlsObj.lnSaleRespCode == d_EMVCL_RC_DATA)
	{
		/* 準備處理要送給Master的感應資料 */
		if (inMultiFunc_Process_CTLS_DATA() != VS_SUCCESS)
		{
			/* 組失敗也當感應失敗 */
			ulRetVal = d_EMVCL_RC_FAILURE;
		}
	}
	else
	{
		inMultiFunc_Process_CTLS_ERROR_DATA();
	}
        
        return (ulRetVal);
}

/*
Function        :inMultiFunc_Process_VIVO_Header
Date&Time       :2017/7/5 上午 11:45
Describe        :塞入前綴並回傳塞入的長度
*/
int inMultiFunc_Process_VIVO_Header(char* szData)
{
	int	inCnt = 0;
	int	inReadLen = 0;
	
	memcpy(&szData[inCnt], _VIVO_HEADER_, _VIVO_HEADER_LEN_);
	inCnt += _VIVO_HEADER_LEN_;
	
	switch (srCtlsObj.lnSaleRespCode)
	{
		case d_EMVCL_RC_DATA :
			memcpy(&szData[inCnt], _VIVO_STATUS_CODES_ONLINE_, _VIVO_STATUS_CODES_LEN_);
			inCnt += _VIVO_STATUS_CODES_LEN_;
			
			/* Chip data 加上 additional data 的長度 */
			inReadLen = szRCDataEx.usChipDataLen + szRCDataEx.usAdditionalDataLen;
			szData[inCnt] = inReadLen / 256;
			inCnt ++;
			szData[inCnt] = inReadLen % 256;
			inCnt ++;
			break;
		case d_EMVCL_NON_EMV_CARD :
			memcpy(&szData[inCnt], _VIVO_STATUS_CODES_SMARTPAY_, _VIVO_STATUS_CODES_LEN_);
			inCnt += _VIVO_STATUS_CODES_LEN_;
			
			/* Chip data 加上 additional data 的長度 */
			inReadLen = szRCDataEx.usChipDataLen + szRCDataEx.usAdditionalDataLen;
			szData[inCnt] = inReadLen / 256;
			inCnt ++;
			szData[inCnt] = inReadLen % 256;
			inCnt ++;
			break;
                case d_EMVCL_RC_FAILURE :
		case d_EMVCL_RC_FALLBACK :
                case d_EMVCL_TRY_AGAIN :
		case d_EMVCL_TX_CANCEL :
		case d_EMVCL_RC_MORE_CARDS :
		case d_EMVCL_RC_NO_CARD :
		case d_EMVCL_PENDING :
                case d_EMVCL_RC_DEK_SIGNAL :
		case d_EMVCL_RC_SEE_PHONE :
		default :
			memcpy(&szData[inCnt], _VIVO_STATUS_CODES_FAIL_, _VIVO_STATUS_CODES_LEN_);
			inCnt += _VIVO_STATUS_CODES_LEN_;
			
			/* Chip data 加上 additional data 的長度 */
			inReadLen = szRCDataEx.usChipDataLen + szRCDataEx.usAdditionalDataLen;
			szData[inCnt] = inReadLen / 256;
			inCnt ++;
			szData[inCnt] = inReadLen % 256;
			inCnt ++;
						
			memcpy(&szData[inCnt], _VIVO_ERROR_CODES_CTLS_INTERFACE_, _VIVO_ERROR_CODES_LEN_);
			inCnt += _VIVO_ERROR_CODES_LEN_;
			
			memcpy(&szData[inCnt], _VIVO_SW1SW2_, _VIVO_SW1SW2_LEN_);
			inCnt += _VIVO_SW1SW2_LEN_;
			
			memcpy(&szData[inCnt], _VIVO_RF_STATE_CODES_NONE_, _VIVO_RF_STATE_CODES_LEN_);
			inCnt += _VIVO_RF_STATE_CODES_LEN_;
			
			break;
	}
	
	return (inCnt);
}

/*
Function        :inMultiFunc_Process_CTLS_DATA
Date&Time       :2017/7/5 下午 1:22
Describe        :用來處理要送到Master Terminal(VX520)的CTLS DATA
*/
int inMultiFunc_Process_CTLS_DATA()
{
	int	i;
	int	inTempCnt = 0;
	int	inCnt = 0;
	int	inReadLen = 0;
	int	inAsciiLen = 0;
	char	szAscii[100 + 1];
	char	szDebugMsg[100 + 1];
	
	memset(gszMultiCTLSData, 0x00, sizeof(gszMultiCTLSData));
	ginMultiCTLSLen = 0;

	/* 加入Verifone命令前綴 */
	inTempCnt = inMultiFunc_Process_VIVO_Header(&gszMultiCTLSData[inCnt]);
	inCnt += inTempCnt;
	
	/* Track 1 */
	memset(szAscii, 0x00, sizeof(szAscii));
	/* 代表原來就是ascii，不用轉，目前在VISA手機用卡發現此現象 */
	if (szRCDataEx.baTrack1Data[0] == '%')
	{
		for (i = 0; (1 + i) < szRCDataEx.bTrack1Len; i++)
		{
			
			if (szRCDataEx.baTrack1Data[1 + i] == '?')
				break;

			/* 因為Track1第0個byte為"%"，因此略過 */
			memcpy(&szAscii[0 + i], &szRCDataEx.baTrack1Data[1 + i], 1);
		}
		inAsciiLen = strlen(szAscii);
	}
	else
	{
		inFunc_BCD_to_ASCII(szAscii, szRCDataEx.baTrack1Data, szRCDataEx.bTrack1Len);
		inAsciiLen = szRCDataEx.bTrack1Len * 2;
		/* 最後一碼為F時去除 */
		if (szAscii[inAsciiLen - 1] == 'F')
		{
			inAsciiLen -= 1;
		}
	}
	gszMultiCTLSData[inCnt] = inAsciiLen;
	inCnt ++;
	memcpy(&gszMultiCTLSData[inCnt], szAscii, inAsciiLen);
	inCnt += inAsciiLen;
	
	/* Track 2 */
	memset(szAscii, 0x00, sizeof(szAscii));
	/* 代表原來就是ascii，不用轉，目前在VISA手機用卡發現此現象 */
	if (szRCDataEx.baTrack2Data[0] == ';')
	{
		for (i = 0; (1 + i) < szRCDataEx.bTrack2Len; i++)
		{
			
			if (szRCDataEx.baTrack2Data[1 + i] == '?')
				break;

			/* 因為Track1第0個byte為"%"，因此略過 */
			memcpy(&szAscii[0 + i], &szRCDataEx.baTrack2Data[1 + i], 1);
		}
		inAsciiLen = strlen(szAscii);
	}
	else
	{
		inFunc_BCD_to_ASCII(szAscii, szRCDataEx.baTrack2Data, szRCDataEx.bTrack2Len);
		inAsciiLen = szRCDataEx.bTrack2Len * 2;
		/* 最後一碼為F時去除 */
		if (szAscii[inAsciiLen - 1] == 'F')
		{
			inAsciiLen -= 1;
		}
	}
	/* 要把D換成'=' for接520 */
	for (i = 0; i < inAsciiLen; i++)
	{
		if (szAscii[i] == 'D')
			szAscii[i] = '=';
	}
	
	gszMultiCTLSData[inCnt] = inAsciiLen;
	inCnt ++;
	memcpy(&gszMultiCTLSData[inCnt], szAscii, inAsciiLen);
	inCnt += inAsciiLen;
	
	/* DE 055 data (if available) as a TLV data object
  	   encoded with Tag ‘E1’. The DE 055 data is the
  	   same data as is included in the Clearing Record.
  	   Details given in next Table.
  	   Tag: E1 Format: b1..126 variable. */
	/* 前面要塞 0x01 0xE1 不知原因 */
	memcpy(&gszMultiCTLSData[inCnt], "\x01\xE1", 2);
	inCnt += 2;
	
	if (szRCDataEx.bSID == d_EMVCL_SID_CUP_QPBOC)
	{
		gszMultiCTLSData[inCnt] = 0x67;
		inCnt ++;
	}
	else
	{
		gszMultiCTLSData[inCnt] = 0x82;
		inCnt ++;
		
		/* Chip data 加上 additional data 的長度 */
		inReadLen = szRCDataEx.usChipDataLen + szRCDataEx.usAdditionalDataLen;
		
		gszMultiCTLSData[inCnt] = inReadLen / 256;
		inCnt ++;
		gszMultiCTLSData[inCnt] = inReadLen % 256;
		inCnt ++;
	}
	
	/* chip data */
	memcpy(&gszMultiCTLSData[inCnt], szRCDataEx.baChipData, szRCDataEx.usChipDataLen);
	inCnt += szRCDataEx.usChipDataLen;
	
	if (ginDisplayDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "ChipLen : %d", szRCDataEx.usChipDataLen);
		inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_TRUE);
	}

	/* additional data */
	memcpy(&gszMultiCTLSData[inCnt], szRCDataEx.baAdditionalData, szRCDataEx.usAdditionalDataLen);
	inCnt += szRCDataEx.usAdditionalDataLen;
	
	if (ginDisplayDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "AdditionalLen : %d", szRCDataEx.usAdditionalDataLen);
		inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_TRUE);
	}


	ginMultiCTLSLen += inCnt;
	
	return (VS_SUCCESS);
}

/*
Function        :inMultiFunc_Process_CTLS_ERROR_DATA
Date&Time       :2017/7/25 下午 5:45
Describe        :用來處理要送到Master Terminal(VX520)的CTLS ERROR DATA
*/
int inMultiFunc_Process_CTLS_ERROR_DATA()
{
	int	inTempCnt = 0;
	int	inCnt = 0;
	char	szDebugMsg[100 + 1];
	
	if (ginDisplayDebug == VS_TRUE)
	{
		char	szDebugMsg[100 + 1];

		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Process ctls error data");
		inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_TRUE);
	}
	
	memset(gszMultiCTLSData, 0x00, sizeof(gszMultiCTLSData));
	ginMultiCTLSLen = 0;

	/* 加入Verifone命令前綴 */
	inTempCnt = inMultiFunc_Process_VIVO_Header(&gszMultiCTLSData[inCnt]);
	inCnt += inTempCnt;
	
	/* chip data */
	memcpy(&gszMultiCTLSData[inCnt], szRCDataEx.baChipData, szRCDataEx.usChipDataLen);
	inCnt += szRCDataEx.usChipDataLen;
	if (ginDisplayDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "ChipLen : %d", szRCDataEx.usChipDataLen);
		inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_TRUE);
	}

	/* additional data */
	memcpy(&gszMultiCTLSData[inCnt], szRCDataEx.baAdditionalData, szRCDataEx.usAdditionalDataLen);
	inCnt += szRCDataEx.usAdditionalDataLen;
	if (ginDisplayDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "AdditionalLen : %d", szRCDataEx.usAdditionalDataLen);
		inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_TRUE);
	}


	ginMultiCTLSLen += inCnt;
	
	return (VS_SUCCESS);
}

/*
Function        :inMultiFunc_Process_Smartpay_data
Date&Time       :2017/7/6 下午 4:04
Describe        :
*/
int inMultiFunc_Process_SmartPay_Data(TRANSACTION_OBJECT *pobTran)
{
	int	inCnt = 0;
	int	inAsciiLen = 0;
	char	szTemplate[100 + 1] = {0};
	char	szAscii[512 + 1] = {0};
	char	szDebugMsg[512 + 1] = {0};
	
        memset(gszMultiCTLSData, 0x00, sizeof(gszMultiCTLSData));
	ginMultiCTLSLen = 0;
	
        /* 金融卡發卡單位代號 (8 Bytes)*/
        memcpy(&gszMultiCTLSData[inCnt], pobTran->srBRec.szFiscIssuerID, 8);
        inCnt += 8;
        /* 金融卡備註欄 (30 Bytes) */ 
        memcpy(&gszMultiCTLSData[inCnt], &pobTran->srBRec.szFiscCardComment[0], 30);
        inCnt += 30;
        /* 金融卡帳號 (16 Bytes)*/ 
        memcpy(&gszMultiCTLSData[inCnt], &pobTran->srBRec.szFiscAccount[0], 16);
        inCnt += 16;
        /* 金融卡交易序號 (8 Bytes)*/ 
        memcpy(&gszMultiCTLSData[inCnt], &pobTran->srBRec.szFiscSTAN[0], 8);
        inCnt += 8;    
        /* 金融卡交易授權驗證碼 (8 Bytes)*/ 
        memcpy(&gszMultiCTLSData[inCnt], &pobTran->srBRec.szFiscTac[0], 8);
        inCnt += 8; 
        /* 計算TAC(S2)的交易日期時間 (14 Bytes)*/ 
        memcpy(&gszMultiCTLSData[inCnt], &pobTran->srBRec.szDate[0], 8);
        inCnt += 8;
	memcpy(&gszMultiCTLSData[inCnt], &pobTran->srBRec.szTime[0], 6);
        inCnt += 6;
	
        /* 金融卡MCC (4 Bytes)*/ 
	memcpy(&gszMultiCTLSData[inCnt], &pobTran->srBRec.szFiscMCC[0], 15);
        inCnt += 15; 
        
	/* 卡片載具 */
	if (memcmp(pobTran->srBRec.szFiscPayDevice, _FISC_PAY_DEVICE_SMARTPAY_CARD_, strlen(_FISC_PAY_DEVICE_SMARTPAY_CARD_)) == 0)
	{
                memcpy(&gszMultiCTLSData[inCnt], "1", 1);
	}
	else if (memcmp(pobTran->srBRec.szFiscPayDevice, _FISC_PAY_DEVICE_MOBILE_, strlen(_FISC_PAY_DEVICE_MOBILE_)) == 0)
	{
                memcpy(&gszMultiCTLSData[inCnt], "2", 1);
	}
        else
	{
                memcpy(&gszMultiCTLSData[inCnt], "0", 1);
	}
        inCnt += 1; 
	
	 /* lnOrgBaseTransactionAmount (12 Bytes) */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        sprintf(szTemplate, "%010ld00", pobTran->srBRec.lnTxnAmount);
        memcpy(&gszMultiCTLSData[inCnt], szTemplate, 12);
        inCnt += 12;
        
        /* lnBaseTransactionAmount (12 Bytes) */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        sprintf(szTemplate, "%010ld00", pobTran->srBRec.lnOrgTxnAmount);
        memcpy(&gszMultiCTLSData[inCnt], szTemplate, 12);
        inCnt += 12;        
        
        /* lnOldTotalTransactionAmount (12 Bytes) */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        sprintf(szTemplate, "%010ld00", pobTran->srBRec.lnTotalTxnAmount);
        memcpy(&gszMultiCTLSData[inCnt], szTemplate, 12);
        inCnt += 12;        

        /* inNormalPayRule (2 Bytes) */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        sprintf(szTemplate, "%02d", pobTran->inMultiNormalPayRule);
        memcpy(&gszMultiCTLSData[inCnt], szTemplate, 2);
        inCnt += 2;
	
	/* 金融卡 卡別(1 Bytes) */
        memset(szTemplate, 0x00, sizeof(szTemplate));
	/* 根據語燦判別，雲端行動金融卡 = 1,行動金融卡 = 2，卡片為0 */
	if (memcmp(pobTran->srBRec.szFiscPayDevice, _FISC_PAY_DEVICE_SMARTPAY_CARD_, strlen(_FISC_PAY_DEVICE_SMARTPAY_CARD_)) == 0)
	{
		sprintf(szTemplate, "%d", 0);
	}
	else if (memcmp(pobTran->srBRec.szFiscPayDevice, _FISC_PAY_DEVICE_MOBILE_, strlen(_FISC_PAY_DEVICE_MOBILE_)) == 0)
	{
		/* 是雲端行動金融卡*/
		if (memcmp(pobTran->srBRec.szFiscMobileDevice, _FISC_MOBILE_DEVICE_CLOUD_TXN_, strlen(_FISC_MOBILE_DEVICE_CLOUD_TXN_)) == 0)
		{
			sprintf(szTemplate, "%d", 1);
		}
		else
		{
			sprintf(szTemplate, "%d", 2);
		}
	}
	/* 未定義，先當實體卡*/
	else
	{
		sprintf(szTemplate, "%d", 0);
	}
        memcpy(&gszMultiCTLSData[inCnt], szTemplate, 1);
        inCnt += 1;
	if (ginDisplayDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "FiscType:%s", szTemplate);
		inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X44_, _LINE_16_1_, VS_FALSE);
	}
        
	/* 是行動金融卡的話，附上1004的資料 */
        if (memcmp(pobTran->srBRec.szFiscPayDevice, _FISC_PAY_DEVICE_MOBILE_, strlen(_FISC_PAY_DEVICE_MOBILE_)) == 0)
	{
		inAsciiLen = pobTran->inFisc1004Len * 2;
		
	        /* 行動金融卡 EF1004資料 變動長度(3 Bytes) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%03d", inAsciiLen);						/* ASCII Len */
	        memcpy(&gszMultiCTLSData[inCnt], szTemplate, 3);
	        inCnt += 3;
		if (ginDisplayDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "1004Len:%s", szTemplate);
			inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X44_, _LINE_16_1_, VS_FALSE);
		}
	
		/* 行動金融卡 EF1004的資料 (...256 Bytes) */
		if (inAsciiLen > 0)
	        {
			memset(szAscii, 0x00, sizeof(szAscii));
			inFunc_BCD_to_ASCII(szAscii, (unsigned char*)pobTran->szFisc1004Data, (inAsciiLen / 2));
			
			memcpy(&gszMultiCTLSData[inCnt], szAscii, inAsciiLen);		/* ASCII Len */
			inCnt += inAsciiLen;
			if (ginDisplayDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "1004Data:%s", szAscii);
				inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X44_, _LINE_16_1_, VS_FALSE);
			}
		}
	}

	ginMultiCTLSLen += inCnt;
	
        return (VS_SUCCESS);
}

/*
Function        :inMultiFunc_FISC_CTLSProcess
Date&Time       :2017/7/6 下午 4:30
Describe        :當外接設備時的流程
*/
int inMultiFunc_FISC_CTLSProcess(TRANSACTION_OBJECT *pobTran)
{
	int     inRetVal;
        long    lnAmountTXNLimit;
	char    szDispMsg[128 + 1];
        char    szTemplate[100 + 1];
	char	szDebugMsg[100 + 1];
        char    szSPCVMRequireLimit[12 + 1]; /* SmartPay感應限額 */

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inMultiFunc_FISC_CTLSProcess()_START");
        }
//	if (ginDisplayDebug == VS_TRUE)
//	{
//		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
//		sprintf(szDebugMsg, "FISC_CTLSProcess()_START");
//		inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
//	}
	
	/* 各種SmartPay檢核 不支援SmartPay感應直接擋下 */
	if (inFISC_CheckSmartPayEnable() != VS_SUCCESS)
	{
		 if (ginDebug == VS_TRUE)
			 inLogPrintf(AT, "SmartPay CTLS No Surport!");

		 pobTran->inErrorMsg = _ERROR_CODE_V3_MULTI_FUNC_SMARTPAY_CTLS_REAL_;
		 pobTran->srBRec.inTxnResult = VS_ERROR;

		 return (VS_ERROR);
	}
	
//	if (ginDisplayDebug == VS_TRUE)
//	{
//		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
//		sprintf(szDebugMsg, "SmartPayEnable OK");
//		inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
//	}
	
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
	pobTran->inTransactionCode = _SALE_;
	if (pobTran->inTransactionCode == _SALE_)
	{
		pobTran->inTransactionCode = _FISC_SALE_;
		pobTran->srBRec.inCode = pobTran->inTransactionCode;
		pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
	}
	/* 如果之後有感應退貨加入 */
	else if (pobTran->inTransactionCode == _FISC_REFUND_)
	{
		
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "AID_Select But Cat not Select TRT, inCode: %d", pobTran->srBRec.inCode);
			inLogPrintf(AT, szDebugMsg);
		}
		
		/* 請依正確卡別操作 */
		pobTran->inErrorMsg = _ERROR_CODE_V3_MULTI_FUNC_SMARTPAY_CTLS_REAL_;
		pobTran->srBRec.inTxnResult = VS_ERROR;
		
		return (VS_ERROR);
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

        /* 檢核SmartPay感應限額 */
        memset(szSPCVMRequireLimit, 0x00, sizeof(szSPCVMRequireLimit));
        memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetSPCVMRequireLimit(szTemplate);
        memcpy(&szSPCVMRequireLimit[0], &szTemplate[0], 10);
        
        lnAmountTXNLimit = atol(szSPCVMRequireLimit);
        
        if (pobTran->srBRec.lnTxnAmount >= lnAmountTXNLimit)
        {
		pobTran->inErrorMsg = _ERROR_CODE_V3_MULTI_FUNC_SMARTPAY_OVER_AMOUNT_;
                pobTran->srBRec.inTxnResult = VS_ERROR;
		
                return (VS_ERROR);
        }
        
        /* 顯示 請感應卡片 */
	inDISP_PutGraphic(_CTLS_LOGO_3_, 0, _COORDINATE_Y_LINE_8_1_);
        /* 回復顯示金額 */
        memset(szDispMsg, 0x00, sizeof(szDispMsg));
        sprintf(szDispMsg, "%ld",  pobTran->srBRec.lnTxnAmount);
        inFunc_Amount_Comma(szDispMsg, "NT$ " , ' ', _SIGNED_NONE_, 16, VS_TRUE);
        inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_3_, _COLOR_RED_, _DISP_LEFT_);
        
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
		if (ginDisplayDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "手機金融卡 1004");
			inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
		}
		
		memset(pobTran->srBRec.szFiscPayDevice, 0x00, sizeof(pobTran->srBRec.szFiscPayDevice));
		memcpy(pobTran->srBRec.szFiscPayDevice, _FISC_PAY_DEVICE_MOBILE_, strlen(_FISC_PAY_DEVICE_MOBILE_));
		
		/* 判別是否要輸入密碼 */
		inRetVal = inFISC_ReadRecords_1004(pobTran);
		if (inRetVal != VS_SUCCESS)
		{
			pobTran->srBRec.inTxnResult = VS_ERROR;
			return (inRetVal);
		}
		
		/* 0x02 大額流程，需要輸入密碼 - 沒測過 */
		if (memcmp(pobTran->srBRec.szFiscMobileNFType, _FISC_MOBILE_NFTYPE_MANDATORY_ENTER_PIN_, strlen(_FISC_MOBILE_NFTYPE_MANDATORY_ENTER_PIN_)) == 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "手機金融卡 1004:02");
				inLogPrintf(AT, szDebugMsg);
			}
			if (ginDisplayDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "手機金融卡 1004:02");
				inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
			}
			
			inRetVal = inMultiFunc_FISC_Flow_MobileHighValuePayment(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				pobTran->srBRec.inTxnResult = VS_ERROR;
				return(VS_ERROR);
			}
		}
		/* 0x01 依情況選擇，目前都跑小額流程 */
		else if (memcmp(pobTran->srBRec.szFiscMobileNFType, _FISC_MOBILE_NFTYPE_CONDITION_ENTER_PIN_, strlen(_FISC_MOBILE_NFTYPE_CONDITION_ENTER_PIN_)) == 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "手機金融卡 1004:01");
				inLogPrintf(AT, szDebugMsg);
			}
			if (ginDisplayDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "手機金融卡 1004:01");
				inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
			}
			
			inRetVal = inMultiFunc_FISC_Flow_MobileConditionPayment(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				pobTran->srBRec.inTxnResult = VS_ERROR;
				return(VS_ERROR);
			}
		}
		/* 目前只加不需密碼的流程 */
		else if (memcmp(pobTran->srBRec.szFiscMobileNFType, _FISC_MOBILE_NFTYPE_WITHOUT_ENTER_PIN_, strlen(_FISC_MOBILE_NFTYPE_WITHOUT_ENTER_PIN_)) == 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "手機金融卡 1004:00");
				inLogPrintf(AT, szDebugMsg);
			}
			if (ginDisplayDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "手機金融卡 1004:00");
				inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
			}
			
			inRetVal = inMultiFunc_FISC_Flow_MobileMicroPayment(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				pobTran->srBRec.inTxnResult = VS_ERROR;
				return(VS_ERROR);
			}
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "手機金融卡 EXCEPTION: %X", pobTran->srBRec.szFiscMobileNFType[0]);
				inLogPrintf(AT, szDebugMsg);
			}
			if (ginDisplayDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "手機金融卡 EXCEPTION");
				inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
			}
			
			/* 例外狀況 */
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "FISC MOBILE EXCEPTION");
			}
			
			pobTran->inErrorMsg = _ERROR_CODE_V3_MULTI_FUNC_SMARTPAY_CTLS_REAL_;
			pobTran->srBRec.inTxnResult = VS_ERROR;
			return(VS_ERROR);
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "實體金融卡");
			inLogPrintf(AT, szDebugMsg);
		}
		if (ginDisplayDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "實體金融卡");
			inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
		}
		
		memset(pobTran->srBRec.szFiscPayDevice, 0x00, sizeof(pobTran->srBRec.szFiscPayDevice));
		memcpy(pobTran->srBRec.szFiscPayDevice, _FISC_PAY_DEVICE_SMARTPAY_CARD_, strlen(_FISC_PAY_DEVICE_SMARTPAY_CARD_));
		
		inRetVal = inMultiFunc_FISC_Flow_SmartPayPayment(pobTran);
		if (inRetVal != VS_SUCCESS)
		{
			pobTran->srBRec.inTxnResult = VS_ERROR;
			return(VS_ERROR);
		}
	}
        
	/* 讀卡號範圍 Load CDT HDT HDPT */
	if (inCARD_GetBin(pobTran) != VS_SUCCESS)
	{
		pobTran->inErrorMsg = _ERROR_CODE_V3_MULTI_FUNC_TERMINAL_;
		pobTran->srBRec.inTxnResult = VS_ERROR;
		return(VS_ERROR);
	}
	if (ginDisplayDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Get Bin OK");
		inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
	}

	/* 亮成功的綠燈 */
	inFISC_CTLS_LED_TONE(VS_SUCCESS);
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "金融卡感應完成");
		inLogPrintf(AT, szDebugMsg);
	}
	if (ginDisplayDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "金融卡感應完成");
		inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
	}
        
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inMultiFunc_FISC_CTLSProcess()_END");
        }
        
	return (VS_SUCCESS);
}

/*
Function        :inMultiFunc_FISC_Flow_SmartPayPayment
Date&Time       :2017/8/28 上午 10:21
Describe        :實體卡金融卡流程
*/
int inMultiFunc_FISC_Flow_SmartPayPayment(TRANSACTION_OBJECT *pobTran)
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

	inRetVal = inMultiFunc_FISC_WriteRecords(pobTran);

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
Function        :inMultiFunc_FISC_Flow_MobileMicroPayment
Date&Time       :2017/8/28 上午 10:21
Describe        :行動近端小額交易流程(不須密碼)
*/
int inMultiFunc_FISC_Flow_MobileMicroPayment(TRANSACTION_OBJECT *pobTran)
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

	inRetVal = inMultiFunc_FISC_WriteRecords(pobTran);

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
Function        :inMultiFunc_FISC_Flow_MobileConditionPayment
Date&Time       :2017/9/21 上午 11:18
Describe        :視情況是否需輸入密碼
*/
int inMultiFunc_FISC_Flow_MobileConditionPayment(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;
	/*金融卡流程6-1,辨別是否支援輸入密碼,方式就是讀1004的第二軌,01就是可支援大額輸入密碼,在此先以回傳99分辨 */
	/*確定支援輸入密碼之後, 仍需辨別金額是否大於3k, <3000走元感應金融卡流程*/
	
	/* 大於就輸入密碼授權 */
	if (pobTran->srBRec.lnTxnAmount > 3000)
	{
		inRetVal = inMultiFunc_FISC_WriteTxnAuthReqres(pobTran);
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

		inRetVal = inMultiFunc_FISC_WriteRecords(pobTran);

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
Function	:inMultiFunc_FISC_Flow_MobileHighValuePayment
Date&Time	:2017/9/21 上午 11:18
Describe	:行動近端大額交易流程
*/
int inMultiFunc_FISC_Flow_MobileHighValuePayment(TRANSACTION_OBJECT *pobTran)
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
		sprintf(szDebugMsg, "inMultiFunc_FISC_Flow_MobileHighValuePayment()_START");
		inLogPrintf(AT, szDebugMsg);
	}

	inRetVal = inMultiFunc_FISC_WriteTxnAuthReqres(pobTran);
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
		inLogPrintf(AT, "inMultiFunc_FISC_Flow_MobileHighValuePayment() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :inMultiFunc_FISC_WriteRecords
Date&Time       :2017/7/12 下午 3:22
Describe        :因為不生成TCC，所以獨立出來
*/
int inMultiFunc_FISC_WriteRecords(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_ERROR;
	int		inCnt = 0, inTacLen = 0;
	int		inDebugMsgLen = 0;
	int		inDebugMsgCnt = 0;
	char		szSendData[_WRITE_RECORD_SIZE_ + 1] = {0};
	char		szMID[15 + 1] = {0};
	char		szTemplate[64 + 1] = {0};
	char		szDebugMsg[500 + 1] = {0};
	char		szMCCCode[4 + 1] = {0}; 
	char		szMerchantID[15 + 1] = {0};
	char		szTerminalID[8 + 1] = {0};
	char		szIssuerID[5 + 1] = {0};
	char		szAmount[14 + 1] = {0};
	char		szTCC[8 + 1] = {0};
	char		szFiscDateTime[14 + 1] = {0};
	char		szFiscAccount[16 + 1] = {0};
	char		szLRC = 0x00;
	char		szAscii[1000 + 1] = {0};
	APDU_COMMAND	srAPDUData;
	
        if (ginDebug == VS_TRUE)
        {
                sprintf(szDebugMsg,"inMultiFunc_FISC_WriteRecords()_START_inCode(%d)", pobTran->srBRec.inCode);
                inLogPrintf(AT, szDebugMsg);
        }
	if (ginDisplayDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "FISC_Write Start()");
		inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
	}

	if (inLoadMVTRec(0) < 0)
	{
                if (ginDebug == VS_TRUE)
                {
                        sprintf(szDebugMsg,"inLoadMVTRec(0) ERROR");
                        inLogPrintf(AT, szDebugMsg);
                }
                
		return (VS_ERROR);
	}

	if (inLoadHDTRec(0) < 0)
	{
                if (ginDebug == VS_TRUE)
                {
                        sprintf(szDebugMsg,"inLoadHDTRec(0) ERROR");
                        inLogPrintf(AT, szDebugMsg);
                }
                
		return (VS_ERROR);
	}
        
	inCnt = 0;
	inTacLen = 0;
	memset(szSendData, 0x00, sizeof(szSendData));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	
	memset(szAmount, 0x00, sizeof(szAmount));
	sprintf(szAmount, "%012ld00", pobTran->srBRec.lnTxnAmount);
	
	/* TID */
	memset(szTerminalID, 0x00, sizeof(szTerminalID));
	memcpy(szTerminalID, pobTran->szMultiFuncTID, 8);
	
	/* TCC */
	memset(szTCC, 0x00, sizeof(szTCC));
	memcpy(szTCC, pobTran->srBRec.szFiscTCC, 8);
	
	memset(pobTran->srBRec.szFiscDateTime, 0x00, sizeof(pobTran->srBRec.szFiscDateTime));
	sprintf(pobTran->srBRec.szFiscDateTime, "%s%s", pobTran->srBRec.szDate, pobTran->srBRec.szTime);
	memset(szFiscDateTime, 0x00, sizeof(szFiscDateTime));
	memcpy(szFiscDateTime, pobTran->srBRec.szFiscDateTime, 14);
        
	/* MID */
	/* 聯合和財金塞的MID不同，聯合前面塞金融機構末三碼+MID10碼+兩個空白 */
	/* 財金則15碼都是MID */
	memset(szMerchantID, 0x00, sizeof(szMerchantID));
	memcpy(szMerchantID, pobTran->szMultiFuncMID, 15);
	memset(szMID, 0x00, sizeof(szMID));
	memcpy(szMID, szMerchantID, 15);
	
	memset(szMCCCode, 0x00, sizeof(szMCCCode));
	memcpy(szMCCCode, pobTran->srBRec.szFiscMCC, 4);
	
	memset(szIssuerID, 0x00, sizeof(szIssuerID));
	inGetIssuerID(szIssuerID);

	/* Account */
	memset(szFiscAccount, 0x00, sizeof(szFiscAccount));
	memcpy(szFiscAccount, pobTran->srBRec.szFiscAccount, _FISC_ACCOUNT_SIZE_);
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "2541");
		inLogPrintf(AT, szDebugMsg);
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "%s", szAmount);
		inLogPrintf(AT, szDebugMsg);
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "%s", szTerminalID);
		inLogPrintf(AT, szDebugMsg);
			
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "%s", szTCC);
		inLogPrintf(AT, szDebugMsg);
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "%s", szFiscDateTime);
		inLogPrintf(AT, szDebugMsg);
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "%s", szMID);
		inLogPrintf(AT, szDebugMsg);
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "%s", szMCCCode);
		inLogPrintf(AT, szDebugMsg);
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "%s", szIssuerID);
		inLogPrintf(AT, szDebugMsg);
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "%s", "       ");
		inLogPrintf(AT, szDebugMsg);
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "%s", szFiscAccount);
		inLogPrintf(AT, szDebugMsg);
	}
	
	/* 針對不同交易別, 組Send Data */
	/* 當外接設備時使用，所以要看*/
	switch (pobTran->inMultiTransactionCode)
	{
		case _520_MULTIFUNC_SALE_ :
		case _520_MULTIFUNC_FISC2_PURCHASE_ :
			memcpy(&szSendData[inCnt], "2541", 4);
			inCnt += 4;
			/* 金額 */
			sprintf(&szSendData[inCnt], "%s", szAmount);
			inCnt += 14;
			/* TID */
			memcpy(&szSendData[inCnt], szTerminalID, 8);
			inCnt += 8;
			/* 端末機檢核碼 */
			memcpy(&szSendData[inCnt], szTCC, 8);
			inCnt += 8;
			/* 交易時間 YYYYMMDDHHMMSS */
			sprintf(&szSendData[inCnt], "%s", szFiscDateTime);
			inCnt += 14;
			/* MID */
			sprintf(&szSendData[inCnt], "%s", szMID);
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
			memcpy(&szSendData[inCnt], szFiscAccount, _FISC_ACCOUNT_SIZE_);
			inCnt += _FISC_ACCOUNT_SIZE_;
			
			/* 財金提示需先在此先組lrc並存起來,之後要比對 */
                        szLRC = 0x00;
			inFunc_Check_LRC(szSendData, inCnt, &szLRC);
			
			if (ginDisplayDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "LRC: %X", szLRC);
				inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
			}
			break;
		case _520_MULTIFUNC_NP_NATIONAL_PAY_:
		case _520_MULTIFUNC_NP_NORMAL_PAY_:
			/* 交易代號(4 Bytes) */
			if (!memcmp(pobTran->srBRec.szFiscIssuerID, pobTran->szMultiEdcFiscIssuerId, 3))
			{
				/* 同行卡，自行轉帳交易 */
				memcpy(&szSendData[inCnt], "2563", 4);
			}
			else
			{
				/* 非同行卡，轉出交易 */
				memcpy(&szSendData[inCnt], "2562", 4);

				/*. 如果為他行卡,手續費的flag開啟時,而交易類別為 FISC2_NORMAL_PAY .*/
				/*. 需重新計算金額 .*/
				if (pobTran->inMultiTransactionCode == _520_MULTIFUNC_NP_NORMAL_PAY_)
				{
					if (!memcmp(pobTran->szMultiEdcFdtFeeFlag, "Y", 1))
					{
						if (pobTran->lnMultiFdtFeeAmt > pobTran->srBRec.lnTxnAmount)
						{
                                            		pobTran->inErrorMsg = _ERROR_CODE_V3_MULTI_FUNC_SMARTPAY_TIP_TOO_BIG_;
							
							return (VS_ESCAPE);
						}

						/* 因為送上去的金額要先減參數的固定手續費後再上傳,*/
						/* 主機會回傳銀行的手續費 */
						pobTran->srBRec.lnTxnAmount = pobTran->srBRec.lnTxnAmount - pobTran->lnMultiFdtFeeAmt;
						pobTran->srBRec.lnTotalTxnAmount = pobTran->srBRec.lnTxnAmount;
						pobTran->inMultiNormalPayRule = 77;
					}
				}
                        }
                        inCnt += 4;

			/* 金額 */
			sprintf(&szSendData[inCnt], "%012ld00", pobTran->srBRec.lnTxnAmount);
			inCnt += 14;
			/* TID */
                        memset(szTerminalID, 0x00, sizeof(szTerminalID));
                        memcpy(szTerminalID, pobTran->szMultiFuncTID, 8);
			memcpy(&szSendData[inCnt], szTerminalID, 8);
			inCnt += 8;
			/* 端末機檢核碼 */
			memcpy(&szSendData[inCnt], pobTran->szMultiEdcTccCode, 8);
			inCnt += 8;
			/* 交易時間 YYYYMMDDHHMMSS */
			sprintf(&szSendData[inCnt], "%s", szFiscDateTime);
			inCnt += 14;
			/* 金融卡帳號 */
			memcpy(&szSendData[inCnt], szFiscAccount, _FISC_ACCOUNT_SIZE_);
			inCnt += _FISC_ACCOUNT_SIZE_;
			
                        /* 財金提示需先在此先組lrc並存起來,之後要比對 */
                        szLRC = 0x00;
			inFunc_Check_LRC(szSendData, inCnt, &szLRC);
			
			if (ginDisplayDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "LRC For NP: %X", szLRC);
				inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
			}
			break;
		/* 2017/12/27 下午 1:27 For合庫版 */
		case _520_MULTIFUNC_REFUND_:
		case _520_MULTIFUNC_FISC2_REFUND_NOTIFY_:
		case _520_MULTIFUNC_FISC2_REFUND_TRANSFER_:
			memcpy(&szSendData[inCnt], "2543", 4);
			inCnt += 4;
			/* 金額 */
			sprintf(&szSendData[inCnt], "%s", szAmount);
			inCnt += 14;
			/* TID */
			memcpy(&szSendData[inCnt], szTerminalID, 8);
			inCnt += 8;
			/* 端末機檢核碼 */
			memcpy(&szSendData[inCnt], szTCC, 8);
			inCnt += 8;
			/* 交易時間 YYYYMMDDHHMMSS */
			sprintf(&szSendData[inCnt], "%s", szFiscDateTime);
			inCnt += 14;
			/* MID */
			sprintf(&szSendData[inCnt], "%s", szMID);
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
			memcpy(&szSendData[inCnt], szFiscAccount, _FISC_ACCOUNT_SIZE_);
			inCnt += _FISC_ACCOUNT_SIZE_;
			
			/* 財金提示需先在此先組lrc並存起來,之後要比對 */
                        szLRC = 0x00;
			inFunc_Check_LRC(szSendData, inCnt, &szLRC);
			
			if (ginDisplayDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "LRC: %X", szLRC);
				inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
			}
			break;
		default :
			
                        if (ginDebug == VS_TRUE)
                        {
                                sprintf(szDebugMsg,"inFISC_WriteRecords()_inCode_ERR");
                                inLogPrintf(AT, szDebugMsg);
                        }
			
			pobTran->inErrorMsg = _ERROR_CODE_V3_FISC_05_NO_INCODE_;
                        
			return(VS_ERROR);
	}

	/* 開始塞資料 */
	memset(&srAPDUData, 0x00, sizeof(srAPDUData));
	srAPDUData.uszCommandCLAData[0] = _FISC_WRITE_RECORD_CLA_COMMAND_; /* CLA */
	srAPDUData.uszCommandINSData[0] = _FISC_WRITE_RECORD_INS_COMMAND_; /* INS */
	srAPDUData.uszCommandP1Data[0] = _FISC_WRITE_RECORD_P1_COMMAND_; /* P1 */
	
	/* 這個欄位塞錯，會導致TAC算錯 */
        if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
                srAPDUData.uszCommandP2Data[0] = _FISC_WRITE_RECORD_P2_COMMAND_CTLS_;
        else
                srAPDUData.uszCommandP2Data[0] = _FISC_WRITE_RECORD_P2_COMMAND_; /* P2 */
	
	memcpy(&srAPDUData.uszCommandData[0], &szSendData[0], sizeof(szSendData)); /* Data */
	srAPDUData.inCommandDataLen = inCnt; /* Data length */

        if (ginDebug == VS_TRUE)
        {
                sprintf(szDebugMsg,"[srAPDUData]");
                inLogPrintf(AT, szDebugMsg);
		sprintf(szDebugMsg,"srAPDUData.uszCommandCLAData = [%02x]", srAPDUData.uszCommandCLAData[0]);
                inLogPrintf(AT, szDebugMsg);
                sprintf(szDebugMsg,"srAPDUData.uszCommandINSData = [%02x]", srAPDUData.uszCommandINSData[0]);
                inLogPrintf(AT, szDebugMsg);
                sprintf(szDebugMsg,"srAPDUData.uszCommandP1Data = [%02x]", srAPDUData.uszCommandP1Data[0]);
                inLogPrintf(AT, szDebugMsg);
                sprintf(szDebugMsg,"srAPDUData.uszCommandP2Data = [%02x]", srAPDUData.uszCommandP2Data[0]);
                inLogPrintf(AT, szDebugMsg);
                sprintf(szDebugMsg,"srAPDUData.inCommandDataLen = [%d]", srAPDUData.inCommandDataLen);
                inLogPrintf(AT, szDebugMsg);
        }

	inRetVal = inAPDU_BuildAPDU(&srAPDUData);
	if (inRetVal == VS_SUCCESS)
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
				/* 感應燈號及聲響 */
				if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
				{
					pobTran->inErrorMsg = _ERROR_CODE_V3_MULTI_FUNC_SMARTPAY_CTLS_REAL_;
				}
				else
				{
					pobTran->inErrorMsg = _ERROR_CODE_V3_MULTI_FUNC_SMARTPAY_REAL_;
				}

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg,"inFISC_APDUTransmit()_ERROR");
					inLogPrintf(AT, szDebugMsg);
				}
				if (ginDisplayDebug == VS_TRUE)
				{
					memset(szAscii, 0x00, sizeof(szAscii));
					inFunc_BCD_to_ASCII(szAscii, &srAPDUData.uszRecevData[srAPDUData.inRecevLen - 2], 2);
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "Fisc Write ERR: %s", szAscii);
					inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_TRUE);
				}
				inRetVal = VS_ERROR;
			}
		}
		else
		{
                        pobTran->inErrorMsg = _ERROR_CODE_V3_FISC_12_SEND_APDU_FAIL_;
			inRetVal = VS_ERROR;
		}
	}
	else
	{
		inRetVal = VS_ERROR;
	}

	/* Write Records失敗不需塞資料 */
	if (inRetVal != VS_SUCCESS)
	{
                if (ginDebug == VS_TRUE)
                {
                        sprintf(szDebugMsg,"inNCCC_FISC_ReadRecords()_END");
                        inLogPrintf(AT, szDebugMsg);
                }
                
		return (inRetVal);
	}

	/*
		----------------------------------------------------------
		Response訊息格式
		----------------------------------------------------------
		SLen(1)	| 交易序號 | TLen(2) | 簽章資訊或交易驗證碼 | SW1 SW2
		----------------------------------------------------------
		交易資料明細		  | 交易授權驗證碼
		----------------------------------------------------------
		交易序號(8)|Input Data(64)| 簽章資料 或交易驗證碼 (8~128)
		----------------------------------------------------------
	 */
	
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
	memset(pobTran->srBRec.szFiscTac, 0x00, sizeof(pobTran->srBRec.szFiscTac));
	memcpy(pobTran->srBRec.szFiscTac, &srAPDUData.uszRecevData[inCnt], inTacLen);
	inCnt += 8;

	/* Debug */
        if (ginDebug == VS_TRUE)
        {
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                inFunc_BCD_to_ASCII(szDebugMsg, srAPDUData.uszRecevData, srAPDUData.inRecevLen);
                inLogPrintf(AT, szDebugMsg);
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                sprintf(szDebugMsg,"[Write Records Debug]");
                inLogPrintf(AT, szDebugMsg);
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                sprintf(szDebugMsg,"pobTran->srBRec.szFiscSTAN : [%s]", pobTran->srBRec.szFiscSTAN);
                inLogPrintf(AT, szDebugMsg);
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                sprintf(szDebugMsg,"pobTran->srBRec.szFiscTac : [%s]", pobTran->srBRec.szFiscTac);
                inLogPrintf(AT, szDebugMsg);
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		inFunc_BCD_to_ASCII(szDebugMsg, (unsigned char*)pobTran->srBRec.szFiscTac, 8);
                inLogPrintf(AT, szDebugMsg);
		
                inLogPrintf(AT, "inMultiFunc_FISC_WriteRecords()_END");
        }
		
	if (ginDisplayDebug == VS_TRUE)
	{
		memset(szAscii, 0x00, sizeof(szAscii));
		inFunc_BCD_to_ASCII(szAscii, srAPDUData.uszSendData, srAPDUData.inSendLen);
		inDebugMsgLen = 2 * srAPDUData.inSendLen;
		
		inDISP_LOGDisplay("Send APDU:", _FONTSIZE_16X44_, _LINE_16_9_, VS_FALSE);
		
		if (inDebugMsgLen >= 44)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			memcpy(szDebugMsg, szAscii, 44);
			inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X44_, _LINE_16_10_, VS_TRUE);
			inDebugMsgCnt += 44;
		}
		else
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			memcpy(szDebugMsg, szAscii, strlen(szAscii));
			inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X44_, _LINE_16_10_, VS_TRUE);
			inDebugMsgCnt += inDebugMsgLen;
		}
		inDebugMsgLen -= inDebugMsgCnt;
		
		if (inDebugMsgLen >= 44)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			memcpy(szDebugMsg, &szAscii[inDebugMsgCnt - 1], 44);
			inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X44_, _LINE_16_11_, VS_TRUE);
			inDebugMsgCnt += 44;
		}
		else if (inDebugMsgLen < 44 && inDebugMsgLen >= 0)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			memcpy(szDebugMsg, &szAscii[inDebugMsgCnt - 1], inDebugMsgLen);
			inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X44_, _LINE_16_11_, VS_TRUE);
			inDebugMsgCnt += inDebugMsgLen;
		}
		inDebugMsgLen -= inDebugMsgCnt;
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "inMultiTransactionCode : %d", pobTran->inMultiTransactionCode);
		inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X44_, _LINE_16_12_, VS_TRUE);
			
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "FISC_Write End OK()");
		inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_TRUE);
	}

	return (inRetVal);
}

/*
Function        :inMultiFunc_FISC_WriteTxnAuthReqres
Date&Time       :2017/9/21 上午 10:59
Describe	:在行動裝置上的安全元件寫入交易授權請求相關資料或授權回應相關資料

*/
int inMultiFunc_FISC_WriteTxnAuthReqres(TRANSACTION_OBJECT *pobTran)
{
	int			inRetVal = VS_SUCCESS;
        int			inCnt, inTacLen;
	char			szSendData[_WRITE_RECORD_SIZE_ + 1];
	char			szMID[10 + 1];
	char			szTemplate[64 + 1];
        char			szDebugMsg[100 + 1];
        char			szMCCCode[4 + 1]; 
        char			szMerchantID[15 + 1];
        char			szTerminalID[8 + 1];
	char			szIssuerID[5 + 1];
	char			szFiscDateTime[14 + 1];
	char			szLRC = 0x00;
	APDU_COMMAND		srAPDUData;
	
        if (ginDebug == VS_TRUE)
        {
                sprintf(szDebugMsg,"inMultiFunc_FISC_WriteTxnAuthReqres()_START_inCode(%d)", pobTran->srBRec.inCode);
                inLogPrintf(AT, szDebugMsg);
        }

	if (inLoadMVTRec(0) < 0)
	{
                if (ginDebug == VS_TRUE)
                {
                        sprintf(szDebugMsg,"inLoadMVTRec(0) ERROR");
                        inLogPrintf(AT, szDebugMsg);
                }
                
		return (VS_ERROR);
	}

	if (inLoadHDTRec(0) < 0)
	{
                if (ginDebug == VS_TRUE)
                {
                        sprintf(szDebugMsg,"inLoadHDTRec(0) ERROR");
                        inLogPrintf(AT, szDebugMsg);
                }
                
		return (VS_ERROR);
	}
        
	inCnt = 0;
	inTacLen = 0;
	memset(szSendData, 0x00, sizeof(szSendData));
	memset(szTemplate, 0x00, sizeof(szTemplate));
        
	memset(szMerchantID, 0x00, sizeof(szMerchantID));
	memcpy(szMerchantID, pobTran->szMultiFuncMID, 15);
	memset(szMID, 0x00, sizeof(szMID));
	memcpy(szMID, szMerchantID, 10);
	
        memset(szMCCCode, 0x00, sizeof(szMCCCode));
	memcpy(szMCCCode, pobTran->srBRec.szFiscMCC, 4);
	
	memset(szIssuerID, 0x00, sizeof(szIssuerID));
	inGetIssuerID(szIssuerID);

	/* MCC 由 EMVDef.txt第一組的參數的MCC(下TMS時同步到EDC.dat上) */
	memcpy(&pobTran->srBRec.szFiscMCC[0], &szMCCCode[0], 4);

	/* 設卡片交易時間 */
	memset(pobTran->srBRec.szFiscDateTime, 0x00, sizeof(pobTran->srBRec.szFiscDateTime));
	sprintf(pobTran->srBRec.szFiscDateTime, "%s%s", pobTran->srBRec.szDate, pobTran->srBRec.szTime);
	memset(szFiscDateTime, 0x00, sizeof(szFiscDateTime));
	memcpy(szFiscDateTime, pobTran->srBRec.szFiscDateTime, 14);

	/* 產生端末設備查核碼 */
        if (ginDebug == VS_TRUE)
        {
                sprintf(szDebugMsg,"TCC Start");
                inLogPrintf(AT, szDebugMsg);
        }
        
	if (inFISC_GenMACAndTCC(pobTran) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
                {
                        sprintf(szDebugMsg, "inFISC_GenMAC Error");
                        inLogPrintf(AT, szDebugMsg);
                }

                /* 產生MAC及TAC失敗 */
		pobTran->inErrorMsg = _ERROR_CODE_V3_FISC_04_MAC_TAC_;
                
		return (VS_ERROR);
	}

	/* 針對不同交易別, 組Send Data */
	/* 當外接設備時使用，所以要看*/
	switch (pobTran->inMultiTransactionCode)
	{
		case _520_MULTIFUNC_SALE_ :
		case _520_MULTIFUNC_FISC2_PURCHASE_ :			
			memcpy(&szSendData[inCnt], "2541", 4);
			inCnt += 4;
			/* 金額 */
			sprintf(&szSendData[inCnt], "%012ld00", pobTran->srBRec.lnTxnAmount);
			inCnt += 14;
			/* TID */
                        memset(szTerminalID, 0x00, sizeof(szTerminalID));
                        memcpy(szTerminalID, pobTran->szMultiFuncTID, 8);
			memcpy(&szSendData[inCnt], szTerminalID, 8);
			inCnt += 8;
			/* 端末機檢核碼 */
			memcpy(&szSendData[inCnt], &pobTran->srBRec.szFiscTCC[0], 8);
			inCnt += 8;
			/* 交易時間 YYYYMMDDHHMMSS */
			sprintf(&szSendData[inCnt], "%s", pobTran->srBRec.szFiscDateTime);
			inCnt += 14;
			/* MID */
			sprintf(&szSendData[inCnt], "%s", szMID);
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
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "LRC: %X", szLRC);
				inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
			}
			break;
		case _520_MULTIFUNC_NP_NATIONAL_PAY_:
		case _520_MULTIFUNC_NP_NORMAL_PAY_:
			/* 交易代號(4 Bytes) */
			if (!memcmp(pobTran->srBRec.szFiscIssuerID, pobTran->szMultiEdcFiscIssuerId, 3))
			{
				/* 同行卡，自行轉帳交易 */
				memcpy(&szSendData[inCnt], "2563", 4);
			}
			else
			{
				/* 非同行卡，轉出交易 */
				memcpy(&szSendData[inCnt], "2562", 4);

				/*. 如果為他行卡,手續費的flag開啟時,而交易類別為 FISC2_NORMAL_PAY .*/
				/*. 需重新計算金額 .*/
				if (pobTran->inMultiTransactionCode == _520_MULTIFUNC_NP_NORMAL_PAY_)
				{
					if (!memcmp(pobTran->szMultiEdcFdtFeeFlag, "Y", 1))
					{
						if (pobTran->lnMultiFdtFeeAmt > pobTran->srBRec.lnTxnAmount)
						{
                                            		pobTran->inErrorMsg = _ERROR_CODE_V3_MULTI_FUNC_SMARTPAY_TIP_TOO_BIG_;
							
							return (VS_ESCAPE);
						}

						/* 因為送上去的金額要先減參數的固定手續費後再上傳,*/
						/* 主機會回傳銀行的手續費 */
						pobTran->srBRec.lnTxnAmount = pobTran->srBRec.lnTxnAmount - pobTran->lnMultiFdtFeeAmt;
						pobTran->srBRec.lnTotalTxnAmount = pobTran->srBRec.lnTxnAmount;
						pobTran->inMultiNormalPayRule = 77;
					}
				}
                        }
                        inCnt += 4;

			/* 金額 */
			sprintf(&szSendData[inCnt], "%012ld00", pobTran->srBRec.lnTxnAmount);
			inCnt += 14;
			/* TID */
                        memset(szTerminalID, 0x00, sizeof(szTerminalID));
                        memcpy(szTerminalID, pobTran->szMultiFuncTID, 8);
			memcpy(&szSendData[inCnt], szTerminalID, 8);
			inCnt += 8;
			/* 端末機檢核碼 */
			memcpy(&szSendData[inCnt], pobTran->szMultiEdcTccCode, 8);
			inCnt += 8;
			/* 交易時間 YYYYMMDDHHMMSS */
			sprintf(&szSendData[inCnt], "%s", pobTran->srBRec.szFiscDateTime);
			inCnt += 14;
			/* 金融卡帳號 */
			memcpy(&szSendData[inCnt], pobTran->srBRec.szFiscAccount, _FISC_ACCOUNT_SIZE_);
			inCnt += _FISC_ACCOUNT_SIZE_;
			
                        /* 財金提示需先在此先組lrc並存起來,之後要比對 */
                        szLRC = 0x00;
			inFunc_Check_LRC(szSendData, inCnt, &szLRC);
			
			if (ginDisplayDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "LRC For NP: %X", szLRC);
				inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
			}
			break;
		/* 先加好，避免之後有感應交易 2017/9/22 下午 4:16 */
		case _520_MULTIFUNC_REFUND_:
		case _520_MULTIFUNC_FISC2_REFUND_NOTIFY_:
		case _520_MULTIFUNC_FISC2_REFUND_TRANSFER_:
			/* 卡片交易序號 : 退費交易預設值為 "00000000" */
			memcpy(&pobTran->srBRec.szFiscSTAN[0], "00000000", 8);

			/* Smart Pay 交易認證碼 : 退費交易預設值為 "00000000" */
			memcpy(&pobTran->srBRec.szFiscTac[0], "00000000", 8);

                        if (ginDebug == VS_TRUE)
                        {
                                sprintf(szDebugMsg,"inFISC_WriteRecords()_END");
                                inLogPrintf(AT, szDebugMsg);
                        }
			return (VS_SUCCESS);
		default :
			
                        if (ginDebug == VS_TRUE)
                        {
                                sprintf(szDebugMsg,"inFISC_WriteRecords()_inCode_ERR");
                                inLogPrintf(AT, szDebugMsg);
                        }
                        
			pobTran->inErrorMsg = _ERROR_CODE_V3_FISC_05_NO_INCODE_;
			
			return(VS_ERROR);
	}

	/* 開始塞資料 */
	memset(&srAPDUData, 0x00, sizeof(srAPDUData));
	srAPDUData.uszCommandCLAData[0] = _FISC_WRITE_AUTH_RECORD_CLA_COMMAND_; /* CLA */
	srAPDUData.uszCommandINSData[0] = _FISC_WRITE_AUTH_RECORD_INS_COMMAND_; /* INS */
	srAPDUData.uszCommandP1Data[0] = _FISC_WRITE_AUTH_RECORD_P1_COMMAND_; /* P1 */
	srAPDUData.uszCommandP2Data[0] = _FISC_WRITE_AUTH_RECORD_P2_COMMAND_; /* P2 */
	
	memcpy(&srAPDUData.uszCommandData[0], &szSendData[0], sizeof(szSendData)); /* Data */
	srAPDUData.inCommandDataLen = inCnt; /* Data length */

        if (ginDebug == VS_TRUE)
        {
                sprintf(szDebugMsg,"[srAPDUData]");
                inLogPrintf(AT, szDebugMsg);
		sprintf(szDebugMsg,"srAPDUData.uszCommandCLAData = [%02x]", srAPDUData.uszCommandCLAData[0]);
                inLogPrintf(AT, szDebugMsg);
                sprintf(szDebugMsg,"srAPDUData.uszCommandINSData = [%02x]", srAPDUData.uszCommandINSData[0]);
                inLogPrintf(AT, szDebugMsg);
                sprintf(szDebugMsg,"srAPDUData.uszCommandP1Data = [%02x]", srAPDUData.uszCommandP1Data[0]);
                inLogPrintf(AT, szDebugMsg);
                sprintf(szDebugMsg,"srAPDUData.uszCommandP2Data = [%02x]", srAPDUData.uszCommandP2Data[0]);
                inLogPrintf(AT, szDebugMsg);
                sprintf(szDebugMsg,"srAPDUData.inCommandDataLen = [%d]", srAPDUData.inCommandDataLen);
                inLogPrintf(AT, szDebugMsg);
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
				if (!memcmp(&srAPDUData.uszRecevData[0],"\x69\x82",2))
				{
					/* 卡片失效 */
					pobTran->inErrorMsg = _ERROR_CODE_V3_FISC_02_6982_;
				}
				else
				{
					/* 讀卡失敗 */
					pobTran->inErrorMsg = _ERROR_CODE_V3_FISC_01_READ_FAIL_;
				}

				inRetVal = VS_ERROR;
			}
		}
		else
		{
                        pobTran->inErrorMsg = _ERROR_CODE_V3_FISC_12_SEND_APDU_FAIL_;
			inRetVal = VS_ERROR;
		}
	}

	/* Write Records失敗不需塞資料 */
	if (inRetVal != VS_SUCCESS)
	{
                if (ginDebug == VS_TRUE)
                {
                        sprintf(szDebugMsg,"inMultiFunc_FISC_WriteTxnAuthReqres()_END");
                        inLogPrintf(AT, szDebugMsg);
                }
                
		return (inRetVal);
	}

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inMultiFunc_FISC_WriteTxnAuthReqres() END !");
		inLogPrintf(AT, "----------------------------------------");
	}

	return (inRetVal);
}

/*
Function        :inMultiFunc_EnterPin
Date&Time       :2017/7/10 下午 2:28
Describe        :輸入PIN
*/
int inMultiFunc_EnterPin(TRANSACTION_OBJECT *pobTran, char *szPinData)
{
        int		inRetVal;
	char		szDispMsg[100 + 1];
        DISPLAY_OBJECT  srDispObj;
		
	while (1)
	{
                /* Mirror Message */
                if (pobTran->uszECRBit == VS_TRUE)
                {
                        inECR_SendMirror(pobTran, _MIRROR_MSG_ENTER_PW_);
                }
            
                inDISP_BEEP(1, 0);
		
		/* 目前先借用金融卡的圖片，若之後想改直接改路徑 */
		inDISP_ClearAll();
		inDISP_PutGraphic(_MULTI_GET_PASSWORD_, 0, _COORDINATE_Y_LINE_8_4_);
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		sprintf(szDispMsg, "%ld", pobTran->srBRec.lnTxnAmount);
		inFunc_Amount_Comma(szDispMsg, "NT$ " , ' ', _SIGNED_NONE_, 16, VS_TRUE);
		inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_LEFT_);
		
		memset(&srDispObj, 0x00, DISPLAY_OBJECT_SIZE);

                /* 設定顯示變數 */
                srDispObj.inMaxLen = 16;
                srDispObj.inY = _LINE_8_8_;
                srDispObj.inR_L = _DISP_RIGHT_;
                srDispObj.inMask = VS_TRUE;
		srDispObj.inCanNotBypass = VS_FALSE;
		srDispObj.inColor = _COLOR_RED_;
		
		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;
	
                inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);
		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (inRetVal);

		if (srDispObj.inOutputLen >= 0)
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
Function        :inMultiFunc_Write_PINKey
Date&Time       :2017/7/6 下午 5:23
Describe        :
 *para.Version:				Structure Format Version，可填入0x00或0x01
 *para.Info.KeySet:			key set(一個key set只能有一種key type)
 *para.Info.KeyIndex:			key index
 *para.Info.KeyType:			key之後會用在哪一種加密方法
 *para.Info.KeyVersion:			使用者自訂，用來管理key的版本，基本上無用
 *para.Info.KeyAttribute:		若有多個屬性，用"or"把bit on起來
 *Para.Protection.CipherKeySet		KetProtectionKey的KeySet
 *Para.Protection.CipherKeyIndex	KetProtectionKey的KeyIndex
 *para.Protection.Mode:			key寫進去時的加密狀態（KMS2_KEYPROTECTIONMODE_KPK_ECB 表示ECB寫入）
 *usTMKindex:				用第幾把Master Key
 *usPINKeyLen:				PINKey HEX長度	
 *szPINKeyAscii				PINKey Ascii形式
 *szKeyCheckValueAscii			KeyCheckValue Ascii形式
*/
int inMultiFunc_Write_PINKey()
{
	char			szDebugMsg[100 + 1];
	char			szAscii[64 + 1];
	char			szPINKeyAscii[32 + 1] = _MULTIFUNC_PINKEY_VALUE_;
	unsigned char		uszPINKeyHex[24 + 1];		/* 3DES最長24BYTE */
	unsigned short		usReturnValue;
	unsigned short		usPINKeyLen = _MULTIFUNC_PINKEY_VALUE_LEN_ / 2;
	CTOS_KMS2KEYWRITE_PARA	srKeyWritePara;
	
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "------------------------------------------------------------------");
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inMultiFunc_Write_PINKey START()！");
		inLogPrintf(AT, szDebugMsg);
	}
	
	memset(&srKeyWritePara, 0x00, sizeof(CTOS_KMS2KEYWRITE_PARA));
	memset(&uszPINKeyHex, 0x00, sizeof(uszPINKeyHex));
	
	/* 將key轉成HEX格式 */
	inFunc_ASCII_to_BCD(uszPINKeyHex, szPINKeyAscii, usPINKeyLen);
	
	/* 將PIN KEY所需參數放入結構中 */
	srKeyWritePara.Version = 0x01;
	srKeyWritePara.Info.KeySet = _TWK_KEYSET_NCCC_;
	srKeyWritePara.Info.KeyIndex = _TWK_KEYINDEX_NCCC_PIN_MULTIFUNC_;
	srKeyWritePara.Info.KeyType = KMS2_KEYTYPE_3DES;
	srKeyWritePara.Info.KeyVersion = 0x01;
	srKeyWritePara.Info.KeyAttribute =  KMS2_KEYATTRIBUTE_ENCRYPT;
	srKeyWritePara.Protection.Mode = KMS2_KEYPROTECTIONMODE_PLAINTEXT;
	srKeyWritePara.Value.KeyLength = usPINKeyLen;
	srKeyWritePara.Value.pKeyData = uszPINKeyHex;
		
	/* Write PIN Key*/
	usReturnValue = inKMS_Write(&srKeyWritePara);
	
	if (usReturnValue != VS_SUCCESS)
	{	
		/* 失敗 */
		if (ginDebug == VS_TRUE)
		{
			/* 被加密的working key */
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			memset(szAscii, 0x00, sizeof(szAscii));
			inFunc_BCD_to_ASCII(szAscii, srKeyWritePara.Value.pKeyData, srKeyWritePara.Value.KeyLength);
			inLogPrintf(AT, "Failed Encrypted Key :");
			sprintf(szDebugMsg, "%s", szAscii);
			inLogPrintf(AT, szDebugMsg);
		
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "inMultiFunc_Write_PINKey END()！");
			inLogPrintf(AT, szDebugMsg);
			inLogPrintf(AT, "------------------------------------------------------------------");
		}
		
		return (VS_ERROR);
	}
	else
	{
		/* 成功 */
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "inMultiFunc_Write_PINKey END()！");
			inLogPrintf(AT, szDebugMsg);
			inLogPrintf(AT, "------------------------------------------------------------------");
		}
		
		return (VS_SUCCESS);
	}
}

/*
Function        :inMultiFunc_3DES_Encrypt
Date&Time       :2017/7/7 下午 6:05
Describe        :以3DES ECB模式加密
 *para.Version:				Structure Format Version，可填入0x00或0x01
 *para.Protection.CipherKeySet		用來加密的KeySet
 *para.Protection.CipherKeyIndex	用來加密的KeyIndex
 *para.Protection.CipherMethod		加密的method是CBC
 *para.Protection.SK_Length		SK_Length = 0 表示不使用session key(需要用DUKPT)
 *para.ICV.Length			Initial Vector Length
 *para.ICV.pData			Initial Vector
 *para.Input.pData			要加密的資料
 *para.Output.pData			產生的MAC
 *inLength				剩下未處理的資料長度
 *inIndex				已處理的資料長度
 *szInitialVector			當次加密用的InitialVector
 *szPlaindata				被切成8bytes用來加密的資料塊
*/
int inMultiFunc_3DES_Encrypt(char* szInPlaindata, int inInPlaindataLen, char *szResult)
{
	int				inRetVal;
	char				szAscii[64 + 1];
	char				szDebugMsg[100 + 1];
	CTOS_KMS2DATAENCRYPT_PARA	srDataEncryptPara;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "------------------------------------------------------------------");
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "inMultiFunc_3DES_Encrypt START()！");
		inLogPrintf(AT, szDebugMsg);
	}
	
	/* 把要加密的Key寫入 */
	if (inMultiFunc_Write_PINKey() != VS_SUCCESS)
		return (VS_ERROR);
	
	if (ginDebug == VS_TRUE)
	{
		memset(szAscii, 0x00, sizeof(szAscii));
		inFunc_BCD_to_ASCII(szAscii, (unsigned char *)szInPlaindata, inInPlaindataLen);
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "%s", szAscii);
		inLogPrintf(AT, szDebugMsg);
	}
	
	if (ginDisplayDebug == VS_TRUE)
	{
		memset(szAscii, 0x00, sizeof(szAscii));
		inFunc_BCD_to_ASCII(szAscii, (unsigned char *)szInPlaindata, 8);
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "%s", szAscii);
		inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_TRUE);
		
		memset(szAscii, 0x00, sizeof(szAscii));
		inFunc_BCD_to_ASCII(szAscii, (unsigned char *)&szInPlaindata[8], 8);
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "%s", szAscii);
		inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_TRUE);
		
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "Len: %d", inInPlaindataLen);
		inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_TRUE);
	}
	
	memset(&srDataEncryptPara, 0x00, sizeof(CTOS_KMS2DATAENCRYPT_PARA));
	srDataEncryptPara.Version = 0x01;
	srDataEncryptPara.Protection.CipherKeySet = _TWK_KEYSET_NCCC_;
	srDataEncryptPara.Protection.CipherKeyIndex = _TWK_KEYINDEX_NCCC_PIN_MULTIFUNC_;
	srDataEncryptPara.Protection.CipherMethod = KMS2_DATAENCRYPTCIPHERMETHOD_ECB;
	srDataEncryptPara.Protection.SK_Length = 0;
	
	srDataEncryptPara.Input.Length = inInPlaindataLen;
	srDataEncryptPara.Input.pData = (unsigned char*)szInPlaindata;
	srDataEncryptPara.Output.pData = (unsigned char*)szResult;
	inRetVal = inKMS_DataEncrypt(&srDataEncryptPara);
	
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "MultiFunc_3DES_Encrypt Failed");
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "MultiFunc_3DES_Encrypt Success");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	
	if (ginDebug == VS_TRUE)
	{	
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "inMultiFunc_3DES_Encrypt END()！");
		inLogPrintf(AT, szDebugMsg);
		inLogPrintf(AT, "------------------------------------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inMultiFunc_GetCardFields_CTLS_Test
Date&Time       :2017/8/22 上午 9:46
Describe        :
*/
int inMultiFunc_GetCardFields_CTLS_Test()
{
	TRANSACTION_OBJECT	pobTran;
	MULTI_TABLE		stMultiOb;
	
	memset(&pobTran, 0x00, sizeof(pobTran));
	memset(&stMultiOb, 0x00, sizeof(stMultiOb));
	
	inMultiFunc_GetCardFields_CTLS(&pobTran, &stMultiOb);
	
	return (VS_SUCCESS);
}
