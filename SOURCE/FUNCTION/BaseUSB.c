#include <string.h>
#include <stdio.h>
#include <ctosapi.h>
#include <stdlib.h>
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
#include "../FUNCTION/Function.h"
#include "../FUNCTION/HDT.h"
#include "../FUNCTION/CFGT.h"
#include "../FUNCTION/EDC.h"
#include "../FUNCTION/SCDT.h"
#include "../../NCCC/NCCCsrc.h"
#include "Sqlite.h"
#include "HDPT.h"
#include "CDT.h"
#include "ECR.h"
#include "MultiFunc.h"
#include "BaseUSB.h"

#define max(x, y) (((x) > (y)) ? (x) : (y))
#define min(x, y) (((x) < (y)) ? (x) : (y))

extern int		ginDebug;
extern int		ginISODebug;
extern unsigned char	guszCTLSInitiOK;
extern ECR_TABLE	gsrECROb;
extern BYTE		gbBarCodeECRBit;

ECR_TRANS_TABLE stBaseUSB_ECRTable[] =
{
	/* 第零組標準【400】【8N1】規格 */
	{
		inBaseUSB_ECR_8N1_Standard_Initial,
		inBaseUSB_ECR_8N1_Standard_Receive_Packet,
		inBaseUSB_ECR_8N1_Standard_Send_Packet,
                NULL,        
		inBaseUSB_ECR_8N1_Standard_Send_Error,
                NULL,
                NULL,
		inBaseUSB_ECR_8N1_Standard_Close,
	},
	
	/* 第一組標準【144】【7E1】規格 */
	{
		inBaseUSB_ECR_7E1_Standard_Initial,
		inBaseUSB_ECR_7E1_Standard_Receive_Packet,
		inBaseUSB_ECR_7E1_Standard_Send_Packet,
                NULL,        
		inBaseUSB_ECR_7E1_Standard_Send_Error,
                NULL,
                NULL,
		inBaseUSB_ECR_7E1_Standard_Close,
	},
};

/*
Function        :inBaseUSB_Open
Date&Time       :2017/6/26 下午 4:30
Describe        :
*/
int inBaseUSB_Open()
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal = VS_ERROR;
	
	/* 清空接收的buffer */
	usRetVal = CTOS_BaseUSBOpen();
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_BaseUSBOpen Error: 0x%04x", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_BaseUSBOpen Successs");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inBaseUSB_Close
Date&Time       :2017/6/26 下午 4:31
Describe        :
*/
int inBaseUSB_Close()
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal = VS_ERROR;
	
	/* 清空接收的buffer */
	usRetVal = CTOS_BaseUSBClose();
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_BaseUSBClose Error: 0x%04x", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_BaseUSBClose Successs");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inBaseUSB_FlushTxBuffer
Date&Time       :2017/6/26 下午 5:15
Describe        :清空Txbuffer中的資料
*/
int inBaseUSB_FlushTxBuffer()
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	/* 沒設定完成，不用檢查 */
	if (gsrECROb.srSetting.uszSettingOK != VS_TRUE)
	{
		return (VS_ERROR);
	}
	
	/* 清空接收的buffer */
	usRetVal = CTOS_BaseUSBTxFlush();
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_BaseUSBTxFlush Error: 0x%04x", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		/* 收到的資料長度歸0 */
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_BaseUSBTxFlush Successs");
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_SUCCESS);
	}
	       
}

/*
Function        :inBaseUSB_FlushRxBuffer
Date&Time       :2017/6/26 下午 5:24
Describe        :清空buffer中的資料
*/
int inBaseUSB_FlushRxBuffer()
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	/* 沒設定完成，不用檢查 */
	if (gsrECROb.srSetting.uszSettingOK != VS_TRUE)
	{
		return (VS_ERROR);
	}
	
	/* 清空接收的buffer */
	usRetVal = CTOS_BaseUSBRxFlush();
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_BaseUSBRxFlush Error: 0x%04x", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_BaseUSBRxFlush Successs");
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_SUCCESS);
	}
	       
}

/*
Function        :inBaseUSB_Data_Send_Check
Date&Time       :2017/6/26 下午 4:33
Describe        :
*/
int inBaseUSB_Data_Send_Check()
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal = VS_ERROR;
	
	/* 清空接收的buffer */
	usRetVal = CTOS_BaseUSBTxReady();
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_BaseUSBTxReady Error: 0x%04x", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_BaseUSBTxReady Successs");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inBaseUSB_Data_Send
Date&Time       :2017/6/26 下午 4:38
Describe        :
*/
int inBaseUSB_Data_Send(unsigned char *uszSendBuff, unsigned short usSendSize)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal = VS_ERROR;
	
	/* 清空接收的buffer */
	/* Length of data. The maximum value is 2048. */
	usRetVal = CTOS_BaseUSBTxData(uszSendBuff, usSendSize);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_BaseUSBTxData Error: 0x%04x", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_BaseUSBTxData Successs");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inBaseUSB_Data_Receive_Check
Date&Time       :2017/6/26 下午 5:01
Describe        :
*/
int inBaseUSB_Data_Receive_Check(unsigned short *usReceiveLen)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal = VS_ERROR;
	
	/* 清空接收的buffer */
	usRetVal = CTOS_BaseUSBRxReady(usReceiveLen);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_BaseUSBRxReady Error: 0x%04x", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		/* 有收到資料就回傳成功 */
		if (*usReceiveLen > 0)
		{
			return (VS_SUCCESS);
		}
		else
		{
			return (VS_ERROR);
		}
	}
}

/*
Function        :inBaseUSB_Data_Receive
Date&Time       :2017/6/26 下午 5:05
Describe        :
*/
int inBaseUSB_Data_Receive(unsigned char *uszReceBuff, unsigned short *usReceSize)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal = VS_ERROR;
	
	usRetVal = CTOS_BaseUSBRxData(uszReceBuff, usReceSize);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_BaseUSBRxData Error: 0x%04x", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_BaseUSBRxData Successs");
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_SUCCESS);
	}
}

/*
Function        :inBaseUSB_ECR_Initial
Date&Time       :2017/11/13 下午 2:37
Describe        :
*/
int inBaseUSB_ECR_Initial(void)
{
	int	inRetVal = VS_ERROR;
	
	inRetVal = stBaseUSB_ECRTable[gsrECROb.srSetting.inVersion].inEcrInitial(&gsrECROb);
	
	return (inRetVal);
}

/*
Function        :inBaseUSB_ECR_Receive_Transaction
Date&Time       :2016/6/21 上午 11:42
Describe        :從ECR接收金額及交易別資料
*/
int inBaseUSB_ECR_Receive_Transaction(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;

	/* 清空上次交易的資料 */
	memset(&gsrECROb.srTransData, 0x00, sizeof(ECR_TRANSACTION_DATA));
	/* 收資料 */	
	inRetVal = stBaseUSB_ECRTable[gsrECROb.srSetting.inVersion].inEcrRece(pobTran, &gsrECROb);
	
	if (inRetVal == VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inBaseUSB_ECR_Receive_Transaction ok");
		}
		return (VS_SUCCESS);
	}
	else if (inRetVal == VS_TIMEOUT)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inBaseUSB_ECR_Receive_Transaction Timeout");
		}
		
		return (VS_TIMEOUT);
	}
	else if (inRetVal == VS_USER_CANCEL)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inBaseUSB_ECR_Receive_Transaction Cancel");
		}
		
		return (VS_USER_CANCEL);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inBaseUSB_ECR_Receive_Transaction Error");
		}
		
		return (VS_ERROR);
	}
	
        
}

/*
Function        :inBaseUSB_ECR_Send_Transaction_Result
Date&Time       :2016/7/11 下午 3:44
Describe        :印帳單前要送給ECR
*/
int inBaseUSB_ECR_Send_Transaction_Result(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;

	/* 送資料 */	
	inRetVal = stBaseUSB_ECRTable[gsrECROb.srSetting.inVersion].inEcrSend(pobTran, &gsrECROb);
	
	if (inRetVal == VS_ERROR)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inBaseUSB_ECR_Send_Transaction_Result Error");
		}
		
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inBaseUSB_ECR_Send_Transaction_Result ok");
		}
		
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inBaseUSB_ECR_SendError
Date&Time       :2016/7/18 上午 10:34
Describe        :送錯誤訊息給ECR
*/
int inBaseUSB_ECR_SendError(TRANSACTION_OBJECT * pobTran, int inErrorType)
{
	if (pobTran->uszECRBit != VS_TRUE)
		return (VS_SUCCESS);
	
	gsrECROb.srTransData.inErrorType = inErrorType;
	
	if (stBaseUSB_ECRTable[gsrECROb.srSetting.inVersion].inEcrSendError(pobTran, &gsrECROb) != VS_SUCCESS)
		return (VS_ERROR);
	
	return (VS_SUCCESS);
}

/*
Function        :inBaseUSB_ECR_SendMirror
Date&Time       :2016/7/18 上午 10:34
Describe        :送映射訊息給ECR
*/
int inBaseUSB_ECR_SendMirror(TRANSACTION_OBJECT * pobTran)
{
	if (pobTran->uszECRBit != VS_TRUE)
		return (VS_SUCCESS);
	
        if (stBaseUSB_ECRTable[gsrECROb.srSetting.inVersion].inEcrMirror == NULL)
		return (VS_SUCCESS);
        
	if (stBaseUSB_ECRTable[gsrECROb.srSetting.inVersion].inEcrMirror(pobTran, &gsrECROb) != VS_SUCCESS)
		return (VS_ERROR);
	
	return (VS_SUCCESS);
}

/*
Function        :inBaseUSB_ECR_ReceiveEI
Date&Time       :2016/6/21 上午 11:42
Describe        :從ECR接收金額及交易別資料
*/
int inBaseUSB_ECR_ReceiveEI(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;

	/* 清空上次交易的資料 */
	memset(&gsrECROb.srTransData, 0x00, sizeof(ECR_TRANSACTION_DATA));
	/* 收資料 */	
	inRetVal = stBaseUSB_ECRTable[gsrECROb.srSetting.inVersion].inEcrEIRece(pobTran, &gsrECROb);
	
	if (inRetVal == VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inBaseUSB_ECR_ReceiveEI ok");
		}
		return (VS_SUCCESS);
	}
	else if (inRetVal == VS_TIMEOUT)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inBaseUSB_ECR_ReceiveEI Timeout");
		}
		
		return (VS_TIMEOUT);
	}
	else if (inRetVal == VS_USER_CANCEL)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inBaseUSB_ECR_ReceiveEI Cancel");
		}
		
		return (VS_USER_CANCEL);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inBaseUSB_ECR_ReceiveEI Error");
		}
		
		return (VS_ERROR);
	}
}

/*
Function        :inBaseUSB_ECR_SendEI
Date&Time       :2016/7/18 上午 10:34
Describe        :送EI訊息給ECR
*/
int inBaseUSB_ECR_SendEI(TRANSACTION_OBJECT * pobTran, int inErrorType)
{
	if (pobTran->uszECRBit != VS_TRUE)
		return (VS_SUCCESS);
	
	gsrECROb.srTransData.inErrorType = inErrorType;
	
	if (stBaseUSB_ECRTable[gsrECROb.srSetting.inVersion].inEcrEISend(pobTran, &gsrECROb) != VS_SUCCESS)
		return (VS_ERROR);
	
	return (VS_SUCCESS);
}

/*
Function        :inBaseUSB_ECR_Send
Date&Time       :2016/7/11 下午 2:25
Describe        :傳送要給收銀機的資料
*/
int inBaseUSB_ECR_Send(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer, int inDataSize)
{
	int		i;
	int		inRetVal;
	int		inRetry = 0;
	int		inRecMaxRetry = _ECR_BaseUSB_RETRYTIMES_;					/* 最大重試次數 */
	int		inSendLen = 0;
	char		szDebugMsg[100 +1];
	unsigned char	uszSendBuf[_ECR_BaseUSB_BUFF_SIZE_];					/* 包含STX、ETX、LRC的電文 */
	unsigned char	uszLRC = 0;
	
	
	/* Send之前清Buffer，避免收到錯的回應 */
	inBaseUSB_FlushRxBuffer();
	
	/* 在要傳送Buffer裡放STX */
	uszSendBuf[inSendLen] = _STX_;
	inSendLen ++;
	
	/* 把Data Buffer(純資料)裡所有0x00值改成0x20 */
	for (i = 0; i < inDataSize; i++)
	{
		if (szDataBuffer[i] == 0x00)
		{
			szDataBuffer[i] = 0x20;
		}
	}
	
	/* 在要傳送Buffer裡放要傳送的資料 */
	memcpy(&uszSendBuf[inSendLen], szDataBuffer, inDataSize);
	inSendLen += inDataSize;
	
	/* 在要傳送Buffer裡放ETX */
	uszSendBuf[inSendLen] = _ETX_;
	inSendLen ++;
	
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
	
	inDISP_Timer_Start(_TIMER_NEXSYS_1_, 2);
	
	while (1)
	{
		/* 檢查port是否已經準備好要送資料 */
		while (1)
		{
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				return (VS_TIMEOUT);
			}
			
			if (inBaseUSB_Data_Send_Check() == VS_SUCCESS)
			{
				break;
			}
		};

		/* 經由port傳送資料 */
		inRetVal = inBaseUSB_Data_Send(uszSendBuf, (unsigned short)inSendLen);
                
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
			if (inRetry < inRecMaxRetry)
			{
				/* 接收ACK OR NAK */
				inRetVal = inBaseUSB_ECR_Receive_ACKandNAK(srECROb);

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

		}/* inBaseUSB_Send */
	
	}/* while(1) */
	
}

/*
Function        :inBaseUSB_ECR_Send_ACKorNAK
Date&Time       :2016/6/21 下午 5:36
Describe        :送ACK OR NAK
*/
int inBaseUSB_ECR_Send_ACKorNAK (ECR_TABLE * srECROb, int inAckorNak)
{
	unsigned char	uszSendBuffer[2 + 1];
	unsigned short	usRetVal;
    	
	memset(uszSendBuffer, 0x00, sizeof(uszSendBuffer));
	
	/* 檢查port是否已經準備好要送資料 */
        while (inBaseUSB_Data_Send_Check() != VS_SUCCESS)
	{
		/* 等TxReady*/
	};
	
	if (inAckorNak == _ACK_)
	{	    
		/* 成功，回傳ACK */
		uszSendBuffer[0] = _ACK_;
		uszSendBuffer[1] = _ACK_;
		
		usRetVal = inBaseUSB_Data_Send(uszSendBuffer, 2);
		
		if (usRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "Send ACK_ACK Not OK");
			}

			return (VS_ERROR);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "ACK_ACK");
			}
			
		}
        
	}	
	else if (inAckorNak == _NAK_)
	{
		/* 失敗，回傳NAK */
		uszSendBuffer[0] = _NAK_;
		uszSendBuffer[1] = _NAK_;
		
		usRetVal = inBaseUSB_Data_Send(uszSendBuffer, 2);
		
		if (usRetVal != VS_SUCCESS)
		{		
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "Send NAK NAK Not OK");
			}
			
			return (VS_ERROR);
		}
		else
		{	
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "NAK_NAK");
			}
			
		}
	
	}
	else
	{
		/* 傳入錯誤的參數 */
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "Can't Send Neither ACK or NAK");
		}
		
		return (VS_ERROR);
	}
        
        return (VS_SUCCESS);
}

int inBaseUSB_ECR_Receive_ACKandNAK(ECR_TABLE * srECROb)
{
	int		inRetVal;
	char		szDebugMsg[100 + 1];
	unsigned char	uszBuf[_ECR_BaseUSB_BUFF_SIZE_];
	unsigned short	usTwoSize = 2;
	unsigned short	usReceiveLen = 0;
		
	/* 設定Timeout */
	inRetVal = inDISP_Timer_Start_MicroSecond(_TIMER_NEXSYS_2_, _ECR_RECEIVE_ACK_TIMEOUT_LONG_);
	
	while (1)
	{
		memset(uszBuf, 0x00, sizeof(uszBuf));
		/* 當BaseUSB中有東西(也可用有BaseUSB事件來判斷)就開始分析 */
		while (usReceiveLen == 0)
		{
			inBaseUSB_Data_Receive_Check(&usReceiveLen);
			
			/* 如果timeout就跳出去 */
			if (inTimerGet(_TIMER_NEXSYS_2_) == VS_SUCCESS)
			{
				return (VS_TIMEOUT);
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
			/* 如果timeout就跳出去 */
			if (inTimerGet(1) == VS_SUCCESS)
			{
				return (VS_TIMEOUT);
			}
			
			/* 這邊一次只收兩個byte */
			usTwoSize = 2;
			inRetVal = inBaseUSB_Data_Receive(uszBuf, &usTwoSize);
                        
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

			}/* inBaseUSB_Receive */
			
		}/* while (usReceiveLen > 0) (有收到資料) */
			
	}/* while(1)...... */
		
}

/*
Function        :inBaseUSB_ECR_SelectTransType
Date&Time       :2016/6/22 上午 9:47
Describe        :Indicator為'S'時的交易選單
*/
int inBaseUSB_ECR_SelectTransType(ECR_TABLE * srECROb)
{
	int	inTransType = 0;
	int	inRetVal = VS_SUCCESS;
	int	inChoice = 0;
	int	inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_8_3X3_;
	char	szTemplate[10 + 1];
	char	szDebugMsg[100 + 1];
	char	szKey = 0x00;
		
        memset(szTemplate, 0x00, sizeof(szTemplate));
        memcpy(szTemplate, &srECROb->srTransData.szTransType[0], 2);
        inTransType = atoi(szTemplate);
	
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
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般交易＞ */
			inDISP_PutGraphic(_MENU_SALE_OPTION_, 0, _COORDINATE_Y_LINE_8_4_);
			
			inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_8_3X3_;
                        while (1)
                        {
                                inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
				szKey = uszKBD_Key();
                                
				/* 回傳Timeout */
				if (inTimerGet(_ECR_BaseUSB_RECEIVE_TIMER_) == VS_SUCCESS)
				{
					szKey = _KEY_TIMEOUT_;
				}
				
				/* 一般交易 */
                                if (szKey == _KEY_1_			||
				    inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_)
				{
                                        memcpy(&srECROb->srTransData.szTransType[0], _ECR_8N1_SALE_, 2);
					inRetVal = VS_SUCCESS;
					break;
				}
				/* 紅利扣抵 */
                                else if (szKey == _KEY_2_			||
					 inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_)
				{
                                        memcpy(&srECROb->srTransData.szTransType[0], _ECR_8N1_REDEEM_, 2);
					inRetVal = VS_SUCCESS;
					break;
				}
				/* 分期付款 */
                                else if (szKey == _KEY_3_			||
					 inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_6_)
				{
                                        memcpy(&srECROb->srTransData.szTransType[0], _ECR_8N1_INSTALLMENT_, 2);
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
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_SALE_OFFLINE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜交易補登＞ */
			inDISP_PutGraphic(_MENU_SALE_OFFLINE_OPTION_, 0, _COORDINATE_Y_LINE_8_4_);

			inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_8_3X3_;
                        while (1)
                        {
                                inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
				szKey = uszKBD_Key();
                                
				/* 回傳Timeout */
				if (inTimerGet(_ECR_BaseUSB_RECEIVE_TIMER_) == VS_SUCCESS)
				{
					return (VS_TIMEOUT);
				}
				
				/* 一般補登 */
                                if (szKey == _KEY_1_			||
				    inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_)
				{
                                        memcpy(&srECROb->srTransData.szTransType[0], _ECR_8N1_OFFLINE_, 2);
					inRetVal = VS_SUCCESS;
					break;
                                }
				/* 紅利調帳 */
                                else if (szKey == _KEY_2_			||
					 inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_)
				{
                                        memcpy(&srECROb->srTransData.szTransType[0], _ECR_8N1_REDEEM_ADJUST_, 2);
					inRetVal = VS_SUCCESS;
					break;
                                }
				/* 分期調帳 */
                                else if (szKey == _KEY_3_			||
					 inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_6_)
				{
                                        memcpy(&srECROb->srTransData.szTransType[0], _ECR_8N1_INSTALLMENT_ADJUST_, 2);
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
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般退貨＞ */
			inDISP_PutGraphic(_MENU_REFUND_OPTION_2_, 0, _COORDINATE_Y_LINE_8_4_);

			inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_8_3X3_;
                        while (1)
                        {
                                inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
				szKey = uszKBD_Key();
                                
				/* 回傳Timeout */
				if (inTimerGet(_ECR_BaseUSB_RECEIVE_TIMER_) == VS_SUCCESS)
				{
					szKey = _KEY_TIMEOUT_;
				}
				
				/* 一般退貨 */
                                if (szKey == _KEY_1_			||
				    inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_)
				{
                                        memcpy(&srECROb->srTransData.szTransType[0], _ECR_8N1_REFUND_, 2);
					inRetVal = VS_SUCCESS;
					break;
                                }
				/* 紅利退貨 */
                                else if (szKey == _KEY_2_			||
					 inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_)
				{
                                        memcpy(&srECROb->srTransData.szTransType[0], _ECR_8N1_REDEEM_REFUND_, 2);
					inRetVal = VS_SUCCESS;
					break;
                                }
				/* 分期退貨 */
                                else if (szKey == _KEY_3_			||
					 inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_6_)
				{
                                        memcpy(&srECROb->srTransData.szTransType[0], _ECR_8N1_INSTALLMENT_REFUND_, 2);
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
Function        :inBaseUSB_ECR_8N1_Standard_Initial
Date&Time       :2016/6/20 下午 2:55
Describe        :initial 8N1 COM PORT
*/
int inBaseUSB_ECR_8N1_Standard_Initial(ECR_TABLE *srECROb)
{
	int		inRetVal = VS_ERROR;
	char		szDebugMsg[100 + 1];
	unsigned char	uszParity;
	unsigned char	uszDataBits;
	unsigned char	uszStopBits;
	unsigned long	ulBaudRate;
	
	
	memset(&uszParity, 0x00, sizeof(uszParity));
	memset(&uszDataBits, 0x00, sizeof(uszDataBits));
	memset(&uszStopBits, 0x00, sizeof(uszStopBits));
	memset(&ulBaudRate, 0x00, sizeof(ulBaudRate));
		
	/* BaudRate = 9600 */
	ulBaudRate = 9600;
	
	/* Parity */
	uszParity = 'N';
	
	/* Data Bits */
	uszDataBits = 8;
	
	/* Stop Bits */
	uszStopBits = 1;
	
	
        /* 開port */
	/* Portable 機型若沒接上底座再開Ethernet會失敗 */
	inRetVal = inBaseUSB_Open();

	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inBaseUSB_Open Error");
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);         
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inBaseUSB_Open OK");
		}
	}
        
	/* 清空接收的buffer */
        inBaseUSB_FlushRxBuffer();
	
        return (VS_SUCCESS);
}

/*
Function        :inBaseUSB_ECR_8N1_Standard_Receive_Packet
Date&Time       :2016/7/6 下午 4:04
Describe        :處理收銀機傳來的資料
*/
int inBaseUSB_ECR_8N1_Standard_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal;
	char	szDataBuffer[_ECR_BaseUSB_BUFF_SIZE_];	/* 電文不包含STX和LRC */
	
	
	memset(&szDataBuffer, 0x00, sizeof(szDataBuffer));
/* -----------------------開始接收資料------------------------------------------ */
	srECROb->srSetting.inTimeout = _ECR_RECEIVE_REQUEST_TIMEOUT_;
	inRetVal = inECR_Receive(pobTran, srECROb, szDataBuffer, _ECR_8N1_Standard_Data_Size_);
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
/* -----------------------開始分析資料------------------------------------------ */
	inRetVal = inECR_8N1_Standard_Unpack(pobTran, srECROb, szDataBuffer);
	
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
		return (inRetVal);
	}
	
	
        return (VS_SUCCESS);
}

/*
Function        :inBaseUSB_ECR_8N1_Standard_Send_Packet
Date&Time       :2016/7/11 下午 3:29
Describe        :處理要送給收銀機的資料
*/
int inBaseUSB_ECR_8N1_Standard_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_ERROR;
	char	szDataBuf[_ECR_BaseUSB_BUFF_SIZE_] = {0};	/* 封包資料 */
	
	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransData.uszIsResponse == VS_TRUE)
		return (VS_SUCCESS);
	
	/* 初始化 */
	memset(szDataBuf, 0x00, sizeof(szDataBuf));
/* ---------------------包裝電文--------------------------------------------- */
	inRetVal = inECR_8N1_Standard_Pack(pobTran, srECROb, szDataBuf);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inBaseUSB_ECR_Send(pobTran, srECROb, szDataBuf, _ECR_8N1_Standard_Data_Size_);
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponse = VS_TRUE;
	
        return (VS_SUCCESS);
}

/*
Function        :inBaseUSB_ECR_8N1_Standard_Send_Error
Date&Time       :2016/7/18 上午 10:21
Describe        :傳送錯誤訊息ECR
*/
int inBaseUSB_ECR_8N1_Standard_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb)
{
	int	inRetVal;
	char	szDataBuf[_ECR_BaseUSB_BUFF_SIZE_];	/* 封包資料 */
	
	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransData.uszIsResponse == VS_TRUE)
		return (VS_SUCCESS);
	
	/* 初始化 */
	memset(szDataBuf, 0x00, sizeof(szDataBuf));
/* ---------------------包裝電文--------------------------------------------- */
	inRetVal = inECR_8N1_Standard_Pack_Error(pobTran, srECROb, szDataBuf);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------塞進錯誤訊息--------------------------------------------- */
	inECR_8N1_Standard_Pack_ResponseCode(pobTran, srECROb, szDataBuf);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inBaseUSB_ECR_Send(pobTran, srECROb, szDataBuf, _ECR_8N1_Standard_Data_Size_);
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponse = VS_TRUE;
	
        return (VS_SUCCESS);
}

/*
Function        :inBaseUSB_ECR_8N1_Standard_Close
Date&Time       :2016/7/11 下午 3:34
Describe        :關閉Comport
*/
int inBaseUSB_ECR_8N1_Standard_Close(ECR_TABLE* srECRob)
{
        /*關閉port*/
        if (inBaseUSB_Close() != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

/*
Function        :inBaseUSB_ECR_7E1_Standard_Initial
Date&Time       :2016/6/20 下午 2:55
Describe        :initial 7E1 COM PORT
*/
int inBaseUSB_ECR_7E1_Standard_Initial(ECR_TABLE *srECROb)
{
	int		inRetVal = VS_ERROR;
	char		szDebugMsg[100 + 1];
	unsigned char	uszParity;
	unsigned char	uszDataBits;
	unsigned char	uszStopBits;
	unsigned long	ulBaudRate;
	
	memset(&uszParity, 0x00, sizeof(uszParity));
	memset(&uszDataBits, 0x00, sizeof(uszDataBits));
	memset(&uszStopBits, 0x00, sizeof(uszStopBits));
	memset(&ulBaudRate, 0x00, sizeof(ulBaudRate));		
	
	/* BaudRate = 9600 */
	ulBaudRate = 9600;
	
	/* Parity */
	uszParity = 'N';
	
	/* Data Bits */
	uszDataBits = 7;
	
	/* Stop Bits */
	uszStopBits = 1;
	
	
        /* 開port */
        inRetVal = inBaseUSB_Open();
        
        if (inRetVal != VS_SUCCESS)
        {
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inBaseUSB_Open Error");
			inLogPrintf(AT, szDebugMsg);
		}
                return (VS_ERROR);         
        }
        else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inBaseUSB_Open OK");
		}
	}
		
	/* 清空接收的buffer */
        inBaseUSB_FlushRxBuffer();
	
        return (VS_SUCCESS);
}


/*
Function        :inBaseUSB_ECR_7E1_Standard_Receive_Packet
Date&Time       :2016/7/6 下午 4:04
Describe        :處理收銀機傳來的資料
*/
int inBaseUSB_ECR_7E1_Standard_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal;
	char	szDataBuffer[_ECR_BaseUSB_BUFF_SIZE_];	/* 電文不包含STX和LRC */
	
	memset(&szDataBuffer, 0x00, sizeof(szDataBuffer));
/* -----------------------開始接收資料------------------------------------------ */
	srECROb->srSetting.inTimeout = _ECR_RECEIVE_REQUEST_TIMEOUT_;
	inRetVal = inECR_Receive(pobTran, srECROb, szDataBuffer, _ECR_7E1_Standard_Data_Size_);
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
/* -----------------------開始分析資料------------------------------------------ */
	inRetVal = inECR_7E1_Standard_Unpack(pobTran, szDataBuffer, srECROb);
	
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
		return (inRetVal);
	}
	
	
        return (VS_SUCCESS);
}

/*
Function        :inBaseUSB_ECR_7E1_Standard_Send_Packet
Date&Time       :2016/7/11 下午 3:29
Describe        :處理要送給收銀機的資料
*/
int inBaseUSB_ECR_7E1_Standard_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal;
	char	szDataBuf[_ECR_BaseUSB_BUFF_SIZE_];	/* 封包資料 */
	
	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransData.uszIsResponse == VS_TRUE)
		return (VS_SUCCESS);
	
	/* 初始化 */
	memset(szDataBuf, 0x00, sizeof(szDataBuf));
/* ---------------------包裝電文--------------------------------------------- */
	inRetVal = inECR_7E1_Standard_Pack(pobTran, szDataBuf, srECROb);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inBaseUSB_ECR_Send(pobTran, srECROb, szDataBuf, _ECR_7E1_Standard_Data_Size_);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponse = VS_TRUE;
	
        return (VS_SUCCESS);
}

/*
Function        :inBaseUSB_ECR_7E1_Standard_Send_Error
Date&Time       :2016/7/18 上午 10:21
Describe        :傳送錯誤訊息ECR
*/
int inBaseUSB_ECR_7E1_Standard_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb)
{
	int	inRetVal;
	char	szDataBuf[_ECR_BaseUSB_BUFF_SIZE_];	/* 封包資料 */
	
	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransData.uszIsResponse == VS_TRUE)
		return (VS_SUCCESS);
	
	/* 初始化 */
	memset(szDataBuf, 0x00, sizeof(szDataBuf));
/* ---------------------包裝電文--------------------------------------------- */
	inRetVal = inECR_7E1_Standard_Pack_Error(pobTran, szDataBuf, srECROb);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------塞進錯誤訊息--------------------------------------------- */
	inECR_7E1_Standard_Pack_ResponseCode(pobTran, srECROb, szDataBuf);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inBaseUSB_ECR_Send(pobTran, srECROb, szDataBuf, _ECR_7E1_Standard_Data_Size_);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponse = VS_TRUE;
	
        return (VS_SUCCESS);
}

/*
Function        :inBaseUSB_ECR_7E1_Standard_Close
Date&Time       :2016/7/11 下午 3:34
Describe        :關閉Comport
*/
int inBaseUSB_ECR_7E1_Standard_Close(ECR_TABLE* srECROb)
{
        /*關閉port*/
        if (inBaseUSB_Close() != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

/*
Function        :inBaseUSB_SetCDCMode
Date&Time       :2017/6/26 下午 5:47
Describe        :
*/
int inBaseUSB_SetCDCMode()
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	/* 清空接收的buffer */
	usRetVal = CTOS_BaseUSBSetCDCMode();
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_BaseUSBSetCDCMode Error: 0x%04x", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_BaseUSBSetCDCMode Successs");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inBaseUSB_SetSTDMode
Date&Time       :2017/6/26 下午 5:47
Describe        :
*/
int inBaseUSB_SetSTDMode()
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	/* 清空接收的buffer */
	usRetVal = CTOS_BaseUSBSetSTDMode();
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inBaseUSB_SetSTDMode Error: 0x%04x", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inBaseUSB_SetSTDMode Successs");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inBaseUSB_SetVidPid
Date&Time       :2017/6/26 下午 5:58
Describe        :
*/
int inBaseUSB_SetVidPid(unsigned int uiVidPid)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	/* 清空接收的buffer */
	usRetVal = CTOS_BaseUSBSetVidPid(uiVidPid);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_BaseUSBSetVidPid Error: 0x%04x", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_BaseUSBSetVidPid Successs");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inBaseUSB_GetVidPid
Date&Time       :2017/6/26 下午 5:59
Describe        :
*/
int inBaseUSB_GetVidPid(unsigned int *uiVidPid)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	/* 清空接收的buffer */
	usRetVal = CTOS_BaseUSBGetVidPid(uiVidPid);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_BaseUSBGetVidPid Error: 0x%04x", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_BaseUSBGetVidPid Successs");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}


/*
Function        :inBaseUSB_ECR_8N1_Standard_Mirror_Packet
Date&Time       :2025/10/17 下午 5:14
Describe        :送給收銀機目前端末機行為
*/
int inBaseUSB_ECR_8N1_Standard_Mirror_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_ERROR;
	int	inSendSize = 0;
	char	szDataBuf[_ECR_BaseUSB_BUFF_SIZE_] = {0};	/* 封包資料 */
	
	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransData.uszIsResponse == VS_TRUE)
		return (VS_SUCCESS);
	
	/* 初始化 */
	memset(szDataBuf, 0x00, sizeof(szDataBuf));
	/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/10 上午 10:32 */
	if (gbBarCodeECRBit == VS_TRUE)
	{
		inSendSize = _ECR_8N1_1000_CUPQRCODE_Size_;
	}
	else
	{
		inSendSize = _ECR_8N1_Standard_Data_Size_;
	}
/* ---------------------包裝電文--------------------------------------------- */
	inRetVal = inECR_8N1_Standard_Mirror_Pack(pobTran, srECROb, szDataBuf);

	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inECR_Send_NotACK(pobTran, srECROb, szDataBuf, inSendSize);

        return (VS_SUCCESS);
}