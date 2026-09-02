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
#include "USB.h"

#define max(x, y) (((x) > (y)) ? (x) : (y))
#define min(x, y) (((x) < (y)) ? (x) : (y))

extern int		ginDebug;
extern int		ginISODebug;
extern int		ginDisplayDebug;
extern int		ginEngineerDebug;
extern ECR_TABLE	gsrECROb;
extern BYTE		gbBarCodeECRBit;

ECR_TRANS_TABLE stUSB_ECRTable[] =
{
	/* 第零組標準【400】【8N1】規格 */
	{
		inUSB_ECR_8N1_Standard_Initial,
		inUSB_ECR_8N1_Standard_Receive_Packet,
		inUSB_ECR_8N1_Standard_Send_Packet,
                inUSB_ECR_8N1_Standard_Mirror_Packet,        
		inUSB_ECR_8N1_Standard_Send_Error,
                NULL,
                NULL,
		inUSB_ECR_8N1_Standard_Close,
	},
	
	/* 第一組標準【144】【7E1】規格 */
	{
		inUSB_ECR_7E1_Standard_Initial,
		inUSB_ECR_7E1_Standard_Receive_Packet,
		inUSB_ECR_7E1_Standard_Send_Packet,
                NULL,        
		inUSB_ECR_7E1_Standard_Send_Error,
                NULL,
                NULL,
		inUSB_ECR_7E1_Standard_Close,
	},
	
	/* 第二組(107)邦柏客製化需求【400】【8N1】規格 */
	{
		inUSB_ECR_8N1_Standard_Initial,
		inUSB_ECR_8N1_Customer_107_Bumper_Receive_Packet,
		inUSB_ECR_8N1_Customer_107_Bumper_Send_Packet,
                NULL,        
		inUSB_ECR_8N1_Customer_107_Bumper_Send_Error,
                NULL,
                NULL,
		inUSB_ECR_8N1_Standard_Close,
	},
	
	/* 第三組(111)KIOSK標準客製化需求【400】【8N1】規格*/
	{
		inUSB_ECR_8N1_Standard_Initial,
		inUSB_ECR_8N1_Customer_111_Kiosk_Standard_Receive_Packet,
		inUSB_ECR_8N1_Customer_107_Bumper_Send_Packet,
                NULL,        
		inUSB_ECR_8N1_Customer_107_Bumper_Send_Error,
                NULL,
                NULL,
		inUSB_ECR_8N1_Standard_Close,
	},
	
	/* 第四組(039)【400】【8N1】規格 */
	/* 卡號遮眼前8後4 */
	{
		inUSB_ECR_8N1_Standard_Initial,
		inUSB_ECR_8N1_Standard_Receive_Packet,
		inUSB_ECR_8N1_Customer_039_CARD_NO_HIDE_F8_B4_Send_Packet,
                NULL,        
		inUSB_ECR_8N1_Customer_039_CARD_NO_HIDE_F8_B4_Send_Error,
                NULL,
                NULL,
		inUSB_ECR_8N1_Standard_Close,
	},
	
        /* 第五組 */
	{
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
                NULL,
                NULL,
                NULL,        
	},
        
        /* 第六組 */
	{
		NULL,
		NULL,
		NULL,
                NULL,
                NULL,
		NULL,
		NULL,
                NULL,        
	},
        
        /* 第七組(098)麥當勞客製化需求【400】【8N1】規格，不支援*/
	{
		NULL,
		NULL,
		NULL,
		NULL,
                NULL,
                NULL,
		NULL,
                NULL,        
	},
        
        /* 第八組(034)燦坤3C電子發票客製化需求【144】【8N1】規格，不支援*/
	{
		NULL,
		NULL,
		NULL,
		NULL,
                NULL,
                NULL,
		NULL,
                NULL,        
	},
};

/*
Function        :inUSB_Open
Date&Time       :2017/6/26 下午 4:30
Describe        :
*/
int inUSB_Open(void)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal = VS_ERROR;
	
	/* 清空接收的buffer */
	usRetVal = CTOS_USBOpen();
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBOpen Error: 0x%04x", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBOpen Successs");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inUSB_Close
Date&Time       :2017/6/26 下午 4:31
Describe        :
*/
int inUSB_Close(void)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal = VS_ERROR;
	
	/* 清空接收的buffer */
	usRetVal = CTOS_USBClose();
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBClose Error: 0x%04x", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBClose Successs");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inUSB_FlushTxBuffer
Date&Time       :2017/6/26 下午 5:15
Describe        :清空Txbuffer中的資料
*/
int inUSB_FlushTxBuffer(void)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	/* 沒設定完成，不用檢查 */
	if (gsrECROb.srSetting.uszSettingOK != VS_TRUE)
	{
		return (VS_ERROR);
	}
	
	/* 清空接收的buffer */
	usRetVal = CTOS_USBTxFlush();
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBTxFlush Error: 0x%04x", usRetVal);
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
			sprintf(szDebugMsg, "CTOS_USBTxFlush Successs");
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_SUCCESS);
	}
	       
}

/*
Function        :inUSB_FlushRxBuffer
Date&Time       :2017/6/26 下午 5:24
Describe        :清空buffer中的資料
*/
int inUSB_FlushRxBuffer(void)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	/* 沒設定完成，不用檢查 */
	if (gsrECROb.srSetting.uszSettingOK != VS_TRUE)
	{
		return (VS_ERROR);
	}
	
	/* 清空接收的buffer */
	usRetVal = CTOS_USBRxFlush();
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBRxFlush Error: 0x%04x", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBRxFlush Successs");
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_SUCCESS);
	}
	       
}

/*
Function        :inUSB_Data_Send_Check
Date&Time       :2017/6/26 下午 4:33
Describe        :
*/
int inUSB_Data_Send_Check(void)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal = VS_ERROR;
	
	/* 清空接收的buffer */
	usRetVal = CTOS_USBTxReady();
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBTxReady Error: 0x%04x", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBTxReady Successs");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inUSB_Data_Send
Date&Time       :2017/6/26 下午 4:38
Describe        :
*/
int inUSB_Data_Send(unsigned char *uszSendBuff, unsigned short usSendSize)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal = VS_ERROR;
	
	/* 清空接收的buffer */
	/* Length of data. The maximum value is 2048. */
	usRetVal = CTOS_USBTxData(uszSendBuff, usSendSize);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBTxData Error: 0x%04x", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBTxData Successs");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inUSB_Data_Receive_Check
Date&Time       :2017/6/26 下午 5:01
Describe        :
*/
int inUSB_Data_Receive_Check(unsigned short *usReceiveLen)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal = VS_ERROR;
	
	/* 清空接收的buffer */
	usRetVal = CTOS_USBRxReady(usReceiveLen);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBRxReady Error: 0x%04x", usRetVal);
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
Function        :inUSB_Data_Receive
Date&Time       :2017/6/26 下午 5:05
Describe        :
*/
int inUSB_Data_Receive(unsigned char *uszReceBuff, unsigned short *usReceSize)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal = VS_ERROR;
	
	usRetVal = CTOS_USBRxData(uszReceBuff, usReceSize);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBRxData Error: 0x%04x", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBRxData Successs (%u)", *usReceSize);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_SUCCESS);
	}
}

/*
Function        :inUSB_ECR_Initial
Date&Time       :2017/11/13 下午 2:37
Describe        :
*/
int inUSB_ECR_Initial(void)
{
	int	inRetVal = VS_ERROR;
	
	inRetVal = stUSB_ECRTable[gsrECROb.srSetting.inVersion].inEcrInitial(&gsrECROb);
	
	return (inRetVal);
}

/*
Function        :inUSB_ECR_Receive_Transaction
Date&Time       :2016/6/21 上午 11:42
Describe        :從ECR接收金額及交易別資料
*/
int inUSB_ECR_Receive_Transaction(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;

	/* 清空上次交易的資料 */
	memset(&gsrECROb.srTransData, 0x00, sizeof(ECR_TRANSACTION_DATA));
	/* 收資料 */	
	inRetVal = stUSB_ECRTable[gsrECROb.srSetting.inVersion].inEcrRece(pobTran, &gsrECROb);
	
	if (inRetVal == VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inUSB_ECR_Receive_Transaction ok");
		}
		return (VS_SUCCESS);
	}
	else if (inRetVal == VS_TIMEOUT)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inUSB_ECR_Receive_Transaction Timeout");
		}
		
		return (VS_TIMEOUT);
	}
	else if (inRetVal == VS_USER_CANCEL)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inUSB_ECR_Receive_Transaction Cancel");
		}
		
		return (VS_USER_CANCEL);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inUSB_ECR_Receive_Transaction Error");
		}
		
		return (VS_ERROR);
	}
	
        
}

/*
Function        :inUSB_ECR_Send_Transaction_Result
Date&Time       :2016/7/11 下午 3:44
Describe        :印帳單前要送給ECR
*/
int inUSB_ECR_Send_Transaction_Result(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;

	/* 送資料 */	
	inRetVal = stUSB_ECRTable[gsrECROb.srSetting.inVersion].inEcrSend(pobTran, &gsrECROb);
	
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inUSB_ECR_Send_Transaction_Result Error");
		}
		
		
		return (inRetVal);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inUSB_ECR_Send_Transaction_Result ok");
		}
		
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inUSB_ECR_SendError
Date&Time       :2016/7/18 上午 10:34
Describe        :送錯誤訊息給ECR
*/
int inUSB_ECR_SendError(TRANSACTION_OBJECT * pobTran, int inErrorType)
{
	if (pobTran->uszECRBit != VS_TRUE)
		return (VS_SUCCESS);
	
	gsrECROb.srTransData.inErrorType = inErrorType;
	
	if (stUSB_ECRTable[gsrECROb.srSetting.inVersion].inEcrSendError(pobTran, &gsrECROb) != VS_SUCCESS)
		return (VS_ERROR);
	
	return (VS_SUCCESS);
}

/*
Function        :inUSB_ECR_SendMirror
Date&Time       :2016/7/18 上午 10:34
Describe        :送映射訊息給ECR
*/
int inUSB_ECR_SendMirror(TRANSACTION_OBJECT * pobTran)
{
	if (pobTran->uszECRBit != VS_TRUE)
		return (VS_SUCCESS);
        
        if (stUSB_ECRTable[gsrECROb.srSetting.inVersion].inEcrMirror == NULL)
		return (VS_SUCCESS);
	
	if (stUSB_ECRTable[gsrECROb.srSetting.inVersion].inEcrMirror(pobTran, &gsrECROb) != VS_SUCCESS)
		return (VS_ERROR);
	
	return (VS_SUCCESS);
}

/*
Function        :inUSB_ECR_ReceiveEI
Date&Time       :2016/6/21 上午 11:42
Describe        :從ECR接收金額及交易別資料
*/
int inUSB_ECR_ReceiveEI(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;

	/* 清空上次交易的資料 */
	memset(&gsrECROb.srTransData, 0x00, sizeof(ECR_TRANSACTION_DATA));
	/* 收資料 */	
	inRetVal = stUSB_ECRTable[gsrECROb.srSetting.inVersion].inEcrEIRece(pobTran, &gsrECROb);
	
	if (inRetVal == VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inUSB_ECR_ReceiveEI ok");
		}
		return (VS_SUCCESS);
	}
	else if (inRetVal == VS_TIMEOUT)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inUSB_ECR_ReceiveEI Timeout");
		}
		
		return (VS_TIMEOUT);
	}
	else if (inRetVal == VS_USER_CANCEL)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inUSB_ECR_ReceiveEI Cancel");
		}
		
		return (VS_USER_CANCEL);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inUSB_ECR_ReceiveEI Error");
		}
		
		return (VS_ERROR);
	}
}

/*
Function        :inUSB_ECR_SendEI
Date&Time       :2016/7/18 上午 10:34
Describe        :送EI訊息給ECR
*/
int inUSB_ECR_SendEI(TRANSACTION_OBJECT * pobTran, int inErrorType)
{
	if (pobTran->uszECRBit != VS_TRUE)
		return (VS_SUCCESS);
	
	gsrECROb.srTransData.inErrorType = inErrorType;
	
	if (stUSB_ECRTable[gsrECROb.srSetting.inVersion].inEcrEISend(pobTran, &gsrECROb) != VS_SUCCESS)
		return (VS_ERROR);
	
	return (VS_SUCCESS);
}

/*
Function        :inUSB_ECR_DeInitial
Date&Time       :2018/7/26 下午 2:12
Describe        :反初始化
*/
int inUSB_ECR_DeInitial(void)
{	
	if (stUSB_ECRTable[gsrECROb.srSetting.inVersion].inEcrEnd(&gsrECROb) != VS_SUCCESS)
		return (VS_ERROR);
	
	return (VS_SUCCESS);
}

/*
Function        :inUSB_ECR_NCCC_144_To_400
Date&Time       :2018/7/26 下午 2:15
Describe        :回傳VS_ERROR代表不用轉換
*/
int inUSB_ECR_NCCC_144_To_400(ECR_TABLE * srECROb, unsigned char *uszReceiveBuffer)
{
	char	szCOMPort[4 + 1];
	
	memset(szCOMPort, 0x00, sizeof(szCOMPort));
	inGetECRComPort(szCOMPort);
	/* 非USB，不用繼續判斷 */
	if (memcmp(szCOMPort, _COMPORT_USB1_, strlen(_COMPORT_USB1_)) != 0)
	{
		
		return (VS_ERROR);
	}
	
	if (srECROb->srSetting.inVersion == 1)
	{
		/* 【需求單 - 108046】電子發票BIN大於6碼需求 標準400收銀機欄位ECR Indicator新增”E”值 by Russell 2019/7/5 下午 5:55 */
		if (uszReceiveBuffer[1] == 'I'	||
		    uszReceiveBuffer[1] == 'E')
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "Receive 'I' or 'E'!");
			}
			
			inECR_DeInitial();
			
			inSetECRVersion("00");
			inSaveEDCRec(0);

			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_ChineseFont("144轉400", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_CENTER_);

			inECR_Initial();

			return (VS_SUCCESS);
		}
		/* 不需要轉換協定 */
		return (VS_ERROR);
	}
	else
	{
		/* 不需要轉換協定 */
		return (VS_ERROR);
	}
}

/*
Function        :inUSB_ECR_Receive
Date&Time       :2016/7/6 下午 4:05
Describe        :接收收銀機傳來的資料
*/
int inUSB_ECR_Receive(TRANSACTION_OBJECT *pobTran, ECR_TABLE* srECROb, char *szDataBuffer, int inDataSize)
{
	int		i;
	int		inRetVal;
	int		inRetry = 0;							/* 目前已重試次數 */
	int		inRecMaxRetry = _ECR_USB_RETRYTIMES_;				/* 最大重試次數 */
	char		szDebugMsg[100 + 1];						/* DebugMsg */
	unsigned short	usReceiveBufferSize;						/* uszReceiveBuffer的長度，*/
	unsigned short	usOneSize = 1;							/* 一次只讀一個byte */
	unsigned short	usReceiveLen = 0;
	unsigned char	uszLRC;								/* LRC的值 */
	unsigned char	uszTempBuffer[1024 + 1];					/* 從USB Buffer收到的片段 */
	unsigned char	uszReceiveBuffer[_ECR_USB_BUFF_SIZE_];			/* 包含STX 和 ETX的原始電文 */
	unsigned char	uszSTX = VS_FALSE;						/* 是否收到STX */
	unsigned char	uszETX = VS_FALSE;						/* 是否收到ETX */
	
	/* 設定Timeout */
	srECROb->srSetting.inTimeout = _ECR_USB_RECEIVE_REQUEST_TIMEOUT_;
	inRetVal = inDISP_Timer_Start(_ECR_USB_RECEIVE_TIMER_, srECROb->srSetting.inTimeout);
	
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
		
		/* 當USB中有東西(也可用有USB事件來判斷)就開始分析 */
		while (usReceiveLen == 0)
		{
			/* 如果timeout就跳出去 */
			if (inTimerGet(_ECR_USB_RECEIVE_TIMER_) == VS_SUCCESS)
			{
				return (VS_TIMEOUT);
			}
			
			inUSB_Data_Receive_Check(&usReceiveLen);
		}
		
		if (ginDebug == VS_TRUE)
		{
			char szDebugMsg[100 + 1];

			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Len : %d", usReceiveLen);
			inLogPrintf(AT, szDebugMsg);
		}


		/* 如果沒收到STX就會進這隻，直到分析出STX */
		while (uszSTX == VS_FALSE && usReceiveLen > 0)
		{
			/* 如果timeout就跳出去 */
			if (inTimerGet(_ECR_USB_RECEIVE_TIMER_) == VS_SUCCESS)
			{
				return (VS_TIMEOUT);
			}
			
			/* 這邊一次只收一個byte */
			memset(uszTempBuffer, 0x00, 1);
			usOneSize = 1;
			inRetVal = inUSB_Data_Receive(uszTempBuffer, &usOneSize);
			if (inRetVal == VS_SUCCESS)
			{
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
					}

					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
						inFunc_BCD_to_ASCII(szDebugMsg, uszTempBuffer, usOneSize);
						inLogPrintf(AT, szDebugMsg);
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
			if (inTimerGet(_ECR_USB_RECEIVE_TIMER_) == VS_SUCCESS)
			{
				return (VS_TIMEOUT);
			}
			
			usOneSize = 1;
			inRetVal = inUSB_Data_Receive(&uszReceiveBuffer[usReceiveBufferSize], &usOneSize);

			if (inRetVal == VS_SUCCESS)
			{
				/* 若為為7E1協定，且收到STX之後的第一個為"I" or "E"，則自動轉為8N1 (NCCC ECR特殊規定) */
				/* 【需求單 - 108046】電子發票BIN大於6碼需求 標準400收銀機欄位ECR Indicator新增”E”值 by Russell 2019/7/5 下午 5:55 */
				if ((srECROb->srSetting.inVersion == 1)	&&
				     usReceiveBufferSize == 1		&&
				    (uszReceiveBuffer[usReceiveBufferSize] == 'I'	|| uszReceiveBuffer[usReceiveBufferSize] == 'E')
				   )
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "Receive 'I' or 'E'!");
					}
					inSetECRVersion("00");
					inSaveEDCRec(0);
					
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_ChineseFont("7E1轉8N1", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_CENTER_);
					
					inUSB_ECR_Initial();
					
					return (VS_ERROR);
				}
				
				/* buffer讀出一個byte，長度減一 */
				usReceiveLen --;

				/* 當已收到401byte 確認下個byte是不是ETX */
				if (usReceiveBufferSize == (inDataSize + 3) - 2)
				{
					if (uszReceiveBuffer[usReceiveBufferSize] == _ETX_)
					{	
						/* 是ETX */
						uszETX = VS_TRUE;
						usReceiveBufferSize ++;
						if (ginDebug == VS_TRUE)
						{
							inLogPrintf(AT, "Receive ETX!");
						}

						/* 再收LRC */
						usOneSize = 1;
						inRetVal = inUSB_Data_Receive(&uszReceiveBuffer[usReceiveBufferSize], &usOneSize);
						if (inRetVal == VS_SUCCESS)
						{
							usReceiveLen --;
							usReceiveBufferSize ++;
						}
						else
						{
							/* 沒收到LRC 資料長度有誤*/
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
					}	
				}
				usReceiveBufferSize ++;
			}

		}


		/* 有收到STX、ETX而且收到長度也對，就開始算LRC */
		if (uszSTX == VS_TRUE && uszETX == VS_TRUE)
		{
			/* 資料長度錯誤，清Buffer，送NAK，重收 */
			if (usReceiveBufferSize != inDataSize + 3)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "DataLength error!:%d Retry: %d", usReceiveBufferSize, inRetry);
					inLogPrintf(AT, szDebugMsg);
				}
				
				/* 回傳NAK */
				inUSB_ECR_Send_ACKorNAK(srECROb, _NAK_);
				/* retry次數+ 1 */
				inRetry ++;
				
				/* 初始化資料 */
				memset(uszReceiveBuffer, 0x00, sizeof(uszReceiveBuffer));
				usReceiveBufferSize = 0;
				uszSTX = VS_FALSE;
				uszETX = VS_FALSE;
				
				continue;
			}
			
			/* 計算收到DATA的LRC （Stx Not Include）*/
			for (i = 1; i <= (usReceiveBufferSize - 2); i++)
			{
				uszLRC ^= uszReceiveBuffer[i];
			}
			
			if (uszReceiveBuffer[usReceiveBufferSize-1] == uszLRC)
			{
				/* 比對收到的LRC是否正確，若正確回傳ACK */
				inUSB_ECR_Send_ACKorNAK(srECROb, _ACK_);
				
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
					sprintf(&szDebugMsg[0], "%X", uszReceiveBuffer[usReceiveBufferSize-1]);
					inLogPrintf(AT, szDebugMsg);
				}
				
				/* 若錯誤回傳NAK */
				inUSB_ECR_Send_ACKorNAK(srECROb, _NAK_);
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
	
	/* 列印紙本電文和顯示電文訊息 */
	inECR_Print_Receive_ISODeBug(szDataBuffer, usReceiveBufferSize - 3, inDataSize);
	
	return (VS_SUCCESS);
}

/*
Function        :inUSB_ECR_Send
Date&Time       :2016/7/11 下午 2:25
Describe        :傳送要給收銀機的資料
*/
int inUSB_ECR_Send(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer, int inDataSize)
{
	int		i;
	int		inRetVal;
	int		inRetry = 0;
	int		inRecMaxRetry = _ECR_USB_RETRYTIMES_;					/* 最大重試次數 */
	int		inSendLen = 0;
	char		szDebugMsg[100 +1];
	unsigned char	uszSendBuf[_ECR_USB_BUFF_SIZE_];					/* 包含STX、ETX、LRC的電文 */
	unsigned char	uszLRC = 0;
	
	
	/* Send之前清Buffer，避免收到錯的回應 */
	inUSB_FlushRxBuffer();
	
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
			
			if (inUSB_Data_Send_Check() == VS_SUCCESS)
			{
				break;
			}
		};

		/* 經由port傳送資料 */
		inRetVal = inUSB_Data_Send(uszSendBuf, (unsigned short)inSendLen);
                
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
				inRetVal = inUSB_ECR_Receive_ACKandNAK(srECROb);

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

		}/* inUSB_Send */
	
	}/* while(1) */
	
}

/*
Function        :inUSB_ECR_Send_ACKorNAK
Date&Time       :2016/6/21 下午 5:36
Describe        :送ACK OR NAK
*/
int inUSB_ECR_Send_ACKorNAK (ECR_TABLE * srECROb, int inAckorNak)
{
	unsigned char	uszSendBuffer[2 + 1];
	unsigned short	usRetVal;
    	
	memset(uszSendBuffer, 0x00, sizeof(uszSendBuffer));
	
	/* 檢查port是否已經準備好要送資料 */
        while (inUSB_Data_Send_Check() != VS_SUCCESS)
	{
		/* 等TxReady*/
	};
	
	if (inAckorNak == _ACK_)
	{	    
		/* 成功，回傳ACK */
		uszSendBuffer[0] = _ACK_;
		uszSendBuffer[1] = _ACK_;
		
		usRetVal = inUSB_Data_Send(uszSendBuffer, 2);
		
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
		
		usRetVal = inUSB_Data_Send(uszSendBuffer, 2);
		
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

int inUSB_ECR_Receive_ACKandNAK(ECR_TABLE * srECROb)
{
	int		inRetVal;
	char		szDebugMsg[100 + 1];
	unsigned char	uszBuf[_ECR_USB_BUFF_SIZE_];
	unsigned short	usTwoSize = 2;
	unsigned short	usReceiveLen = 0;
		
	/* 設定Timeout */
	inRetVal = inDISP_Timer_Start_MicroSecond(_TIMER_NEXSYS_2_, _ECR_RECEIVE_ACK_TIMEOUT_LONG_);
	
	while (1)
	{
		memset(uszBuf, 0x00, sizeof(uszBuf));
		/* 當USB中有東西(也可用有USB事件來判斷)就開始分析 */
		while (usReceiveLen == 0)
		{
			inUSB_Data_Receive_Check(&usReceiveLen);
			
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
			inRetVal = inUSB_Data_Receive(uszBuf, &usTwoSize);
                        
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

			}/* inUSB_Receive */
			
		}/* while (usReceiveLen > 0) (有收到資料) */
			
	}/* while(1)...... */
		
}

/*
Function        :inUSB_ECR_SelectTransType
Date&Time       :2016/6/22 上午 9:47
Describe        :Indicator為'S'時的交易選單
*/
int inUSB_ECR_SelectTransType(ECR_TABLE * srECROb)
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
				if (inTimerGet(_ECR_USB_RECEIVE_TIMER_) == VS_SUCCESS)
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
				if (inTimerGet(_ECR_USB_RECEIVE_TIMER_) == VS_SUCCESS)
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
				if (inTimerGet(_ECR_USB_RECEIVE_TIMER_) == VS_SUCCESS)
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
Function        :inUSB_ECR_8N1_Standard_Initial
Date&Time       :2016/6/20 下午 2:55
Describe        :initial 8N1 COM PORT
*/
int inUSB_ECR_8N1_Standard_Initial(ECR_TABLE *srECROb)
{
	char		szDebugMsg[100 + 1] = {0};
	unsigned short	usRetVal = 0x00;
			
        /* 開port */
	/* Portable 機型若沒接上底座再開Ethernet會失敗 */
	usRetVal = inUSB_Open();

	if (usRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inUSB_Open Error: 0x%04x", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);         
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inUSB_Open OK");
		}
	}
        
	/* 清空接收的buffer */
        inUSB_FlushRxBuffer();
	
        return (VS_SUCCESS);
}

/*
Function        :inUSB_ECR_8N1_Standard_Receive_Packet
Date&Time       :2016/7/6 下午 4:04
Describe        :處理收銀機傳來的資料
*/
int inUSB_ECR_8N1_Standard_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal;
	char	szDataBuffer[_ECR_USB_BUFF_SIZE_];	/* 電文不包含STX和LRC */
	
	
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
Function        :inUSB_ECR_8N1_Standard_Send_Packet
Date&Time       :2016/7/11 下午 3:29
Describe        :處理要送給收銀機的資料
*/
int inUSB_ECR_8N1_Standard_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_ERROR;
	char	szDataBuf[_ECR_USB_BUFF_SIZE_] = {0};	/* 封包資料 */
	
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
	
	if (memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INQUIRY_LAST_TRANSACTION_, strlen(_ECR_8N1_INQUIRY_LAST_TRANSACTION_)) != 0)
	{
		inECR_Save_Response(szDataBuf, _ECR_8N1_Standard_Data_Size_);
	}
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, _ECR_8N1_Standard_Data_Size_);
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponse = VS_TRUE;
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inUSB_ECR_8N1_Standard_Send_Error
Date&Time       :2016/7/18 上午 10:21
Describe        :傳送錯誤訊息ECR
*/
int inUSB_ECR_8N1_Standard_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb)
{
	int	inRetVal;
	char	szDataBuf[_ECR_USB_BUFF_SIZE_];	/* 封包資料 */
	
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
	
	if (memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INQUIRY_LAST_TRANSACTION_, strlen(_ECR_8N1_INQUIRY_LAST_TRANSACTION_)) != 0)
	{
		inECR_Save_Response(szDataBuf, _ECR_8N1_Standard_Data_Size_);
	}
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, _ECR_8N1_Standard_Data_Size_);
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponse = VS_TRUE;
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inUSB_ECR_8N1_Standard_Close
Date&Time       :2016/7/11 下午 3:34
Describe        :關閉Comport
*/
int inUSB_ECR_8N1_Standard_Close(ECR_TABLE* srECRob)
{
        /*關閉port*/
        if (inUSB_Close() != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

/*
Function        :inUSB_ECR_7E1_Standard_Initial
Date&Time       :2016/6/20 下午 2:55
Describe        :initial 7E1 COM PORT
*/
int inUSB_ECR_7E1_Standard_Initial(ECR_TABLE *srECROb)
{
	char		szDebugMsg[100 + 1] = {0};
	unsigned short	usRetVal = 0x00;
	
        /* 開port */
        usRetVal = inUSB_Open();
        
        if (usRetVal != d_OK)
        {
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inUSB_Open Error: 0x%04x", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
                return (VS_ERROR);         
        }
        else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inUSB_Open OK");
		}
	}
		
	/* 清空接收的buffer */
        inUSB_FlushRxBuffer();
	
        return (VS_SUCCESS);
}		

/*
Function        :inUSB_ECR_7E1_Standard_Receive_Packet
Date&Time       :2016/7/6 下午 4:04
Describe        :處理收銀機傳來的資料
*/
int inUSB_ECR_7E1_Standard_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal;
	char	szDataBuffer[_ECR_USB_BUFF_SIZE_];	/* 電文不包含STX和LRC */
	
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
Function        :inUSB_ECR_7E1_Standard_Send_Packet
Date&Time       :2016/7/11 下午 3:29
Describe        :處理要送給收銀機的資料
*/
int inUSB_ECR_7E1_Standard_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_ERROR;
	char	szDataBuf[_ECR_USB_BUFF_SIZE_] = {0};	/* 封包資料 */
	
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
	inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, _ECR_7E1_Standard_Data_Size_);
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponse = VS_TRUE;
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inUSB_ECR_7E1_Standard_Send_Error
Date&Time       :2016/7/18 上午 10:21
Describe        :傳送錯誤訊息ECR
*/
int inUSB_ECR_7E1_Standard_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb)
{
	int	inRetVal;
	char	szDataBuf[_ECR_USB_BUFF_SIZE_];	/* 封包資料 */
	
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
	inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, _ECR_7E1_Standard_Data_Size_);
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponse = VS_TRUE;
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inUSB_ECR_7E1_Standard_Close
Date&Time       :2016/7/11 下午 3:34
Describe        :關閉Comport
*/
int inUSB_ECR_7E1_Standard_Close(ECR_TABLE* srECROb)
{
        /*關閉port*/
        if (inUSB_Close() != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

/*
Function        :inUSB_SetCDCMode
Date&Time       :2017/6/26 下午 5:47
Describe        :
*/
int inUSB_SetCDCMode()
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	/* 清空接收的buffer */
	usRetVal = CTOS_USBSetCDCMode();
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBSetCDCMode Error: 0x%04x", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBSetCDCMode Successs");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inUSB_SetSTDMode
Date&Time       :2017/6/26 下午 5:47
Describe        :
*/
int inUSB_SetSTDMode()
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	/* 清空接收的buffer */
	usRetVal = CTOS_USBSetSTDMode();
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inUSB_SetSTDMode Error: 0x%04x", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inUSB_SetSTDMode Successs");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inUSB_SetVidPid
Date&Time       :2017/6/26 下午 5:58
Describe        :
*/
int inUSB_SetVidPid(unsigned int uiVidPid)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	/* 清空接收的buffer */
	usRetVal = CTOS_USBSetVidPid(uiVidPid);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBSetVidPid Error: 0x%04x", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBSetVidPid Successs");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inUSB_GetVidPid
Date&Time       :2017/6/26 下午 5:59
Describe        :
*/
int inUSB_GetVidPid(unsigned int *uiVidPid)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	/* 清空接收的buffer */
	usRetVal = CTOS_USBGetVidPid(uiVidPid);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBGetVidPid Error: 0x%04x", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBGetVidPid Successs");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inUSB_HostOpen
Date&Time       :2017/6/26 下午 6:14
Describe        :
*/
int inUSB_HostOpen(unsigned short usVendorID, unsigned short usProductID)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	usRetVal = CTOS_USBHostOpen(usVendorID, usProductID);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBHostOpen Error: 0x%04x", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBHostOpen Successs");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inUSB_HostClose
Date&Time       :2017/6/26 下午 6:14
Describe        :
*/
int inUSB_HostClose()
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	usRetVal = CTOS_USBHostClose();
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBHostClose Error: 0x%04x", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBHostClose Successs");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :CTOS_USBHostTxData
Date&Time       :2017/6/26 下午 6:14
Describe        :
*/
int inUSB_HostSend(unsigned char *uszSendBuffer, unsigned long ulSendLen, unsigned long ulTransTimeout)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	usRetVal = CTOS_USBHostTxData(uszSendBuffer, ulSendLen, ulTransTimeout);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBHostTxData Error: 0x%04x", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBHostTxData Successs");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inUSB_HostReceive
Date&Time       :2017/6/26 下午 6:14
Describe        :
*/
int inUSB_HostReceive(unsigned char *uszReceiveBuffer, unsigned long *ulReceiveLen, unsigned long ulTransTimeout)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	usRetVal = CTOS_USBHostRxData(uszReceiveBuffer, ulReceiveLen, ulTransTimeout);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBHostRxData Error: 0x%04x", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBHostRxData Successs");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inUSB_SelectMode
Date&Time       :2019/2/18 下午 1:58
Describe        :設定為Host Mode或device Mode
*/
int inUSB_SelectMode(int inMode)
{
	char		szDebugMsg[100 + 1] = {0};
	unsigned short	usRetVal = VS_ERROR;

	usRetVal = CTOS_USBSelectMode(inMode);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBSelectMode Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBSelectMode() OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inUSB_GetMode
Date&Time       :2019/3/12 上午 10:19
Describe        :設定為Host Mode或device Mode
*/
int inUSB_GetStatus(unsigned int *uiStatus)
{
	char		szDebugMsg[100 + 1] = {0};
	unsigned short	usRetVal = VS_ERROR;

	usRetVal = CTOS_USBGetStatus(uiStatus);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBGetStatus Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_USBGetStatus() OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inUSB_Get_Host_Device_Mode
Date&Time       :2019/3/12 上午 11:02
Describe        :
*/
int inUSB_Get_Host_Device_Mode(int *inMode)
{
	int		inRetVal = VS_ERROR;
	unsigned int	uiStatus = 0;
	
	inRetVal = inUSB_GetStatus(&uiStatus);
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	else
	{
		/* 代表為Host Mode 反之，device Mode */
		if ((uiStatus & d_MK_USB_STATUS_HOSTMODE) == d_MK_USB_STATUS_HOSTMODE)
		{
			*inMode = d_USB_HOST_MODE;
		}
		else
		{
			*inMode = d_USB_DEVICE_MODE;
		}
		
		return (VS_SUCCESS);
	}
}

/*
Function        :inUSB_ECR_8N1_TSB_KIOSK_Initial
Date&Time       :2018/2/21 下午 4:38
Describe        :initial 8N1 COM PORT
*/
int inUSB_ECR_8N1_TSB_KIOSK_Initial(ECR_TABLE *srECROb)
{
	char		szDebugMsg[100 + 1] = {0};
	unsigned short	usRetVal = 0x00;
	
        /* 開port */
	/* Portable 機型若沒接上底座再開Ethernet會失敗 */
	usRetVal = inUSB_Open();

	if (usRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inUSB_Open Error: 0x%04x", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);         
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inUSB_Open OK");
		}
	}
        
	/* 清空接收的buffer */
        inUSB_FlushRxBuffer();
	
        return (VS_SUCCESS);
}

/*
Function        :inUSB_ECR_8N1_TSB_KIOSK_Receive_Packet
Date&Time       :2016/7/6 下午 4:04
Describe        :處理收銀機傳來的資料
*/
int inUSB_ECR_8N1_TSB_KIOSK_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal;
	char	szDataBuffer[_ECR_USB_BUFF_SIZE_];	/* 電文不包含STX和LRC */
	char	szDebugMsg[100 + 1] = {0};
	
	memset(&szDataBuffer, 0x00, sizeof(szDataBuffer));
	if (ginDisplayDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "開始接收資料");
		inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_1_, VS_FALSE);
	}
/* -----------------------開始接收資料------------------------------------------ */
	srECROb->srSetting.inTimeout = _ECR_RECEIVE_REQUEST_TIMEOUT_;
	inRetVal = inECR_Receive(pobTran, srECROb, szDataBuffer, _ECR_8N1_Standard_Data_Size_);
	
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDisplayDebug == VS_TRUE)
		{
			uszKBD_GetKey(30);
		}
		
		return (inRetVal);
	}
	
	if (ginDisplayDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "開始分析資料");
		inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_1_, VS_FALSE);
	}
/* -----------------------開始分析資料------------------------------------------ */
	inRetVal = inECR_8N1_TSB_KIOSK_Unpack(pobTran, srECROb, szDataBuffer);
	
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
		
		if (ginDisplayDebug == VS_TRUE)
		{
			uszKBD_GetKey(30);
		}
		
		return (inRetVal);
	}
	
	
        return (VS_SUCCESS);
}

/*
Function        :inUSB_ECR_8N1_TSB_KIOSK_Send_Packet
Date&Time       :2016/7/11 下午 3:29
Describe        :處理要送給收銀機的資料
*/
int inUSB_ECR_8N1_TSB_KIOSK_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal;
	char	szDataBuf[_ECR_USB_BUFF_SIZE_];	/* 封包資料 */
	
	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransData.uszIsResponse == VS_TRUE)
		return (VS_SUCCESS);
	
	/* 初始化 */
	memset(szDataBuf, 0x00, sizeof(szDataBuf));
/* ---------------------包裝電文--------------------------------------------- */
	inRetVal = inECR_8N1_TSB_KIOSK_Pack(pobTran, srECROb, szDataBuf);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, _ECR_8N1_Standard_Data_Size_);
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponse = VS_TRUE;
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inUSB_ECR_8N1_TSB_KIOSK_Send_Error
Date&Time       :2016/7/18 上午 10:21
Describe        :傳送錯誤訊息ECR
*/
int inUSB_ECR_8N1_TSB_KIOSK_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb)
{
	int	inRetVal;
	char	szDataBuf[_ECR_USB_BUFF_SIZE_];	/* 封包資料 */
	char	szDebugMsg[100 + 1] = {0};
	
	if (ginDisplayDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Send Errorr 開始");
		inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_1_, VS_FALSE);
	}
	
	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransData.uszIsResponse == VS_TRUE)
		return (VS_SUCCESS);
	
	/* 初始化 */
	memset(szDataBuf, 0x00, sizeof(szDataBuf));
	if (ginDisplayDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Send Error Pack");
		inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_2_, VS_FALSE);
	}
/* ---------------------包裝電文--------------------------------------------- */
	inRetVal = inECR_8N1_TSB_KIOSK_Pack(pobTran, srECROb, szDataBuf);
	
	if (inRetVal == VS_ERROR)
	{
		if (ginDisplayDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Send Error Pack Fail");
			inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_3_, VS_TRUE);
		}
		return (VS_ERROR);
	}
	
	if (ginDisplayDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Send Error PackRe");
		inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_2_, VS_FALSE);
	}
/* ---------------------塞進錯誤訊息--------------------------------------------- */
	inECR_8N1_TSB_KIOSK_Pack_ResponseCode(pobTran, srECROb, szDataBuf);
	
	if (inRetVal == VS_ERROR)
	{
		if (ginDisplayDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Send Error PackRe Fail");
			inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_3_, VS_TRUE);
		}
		
		return (VS_ERROR);
	}
	
	if (ginDisplayDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Send Error Send");
		inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_2_, VS_FALSE);
	}
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, _ECR_8N1_Standard_Data_Size_);
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponse = VS_TRUE;
	
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDisplayDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Send Error Send Fail");
			inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_3_, VS_TRUE);
		}
		
		return (inRetVal);
	}
	
	if (ginDisplayDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Send Errorr 結束");
		inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_1_, VS_FALSE);
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inUSB_ECR_8N1_TSB_KIOSK_Close
Date&Time       :2016/7/11 下午 3:34
Describe        :關閉Comport
*/
int inUSB_ECR_8N1_TSB_KIOSK_Close(ECR_TABLE* srECRob)
{
        /*關閉port*/
        if (inUSB_Close() != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

/*
Function        :inUSB_ECR_8N1_Customer_107_Bumper_Receive_Packet
Date&Time       :2016/7/6 下午 4:04
Describe        :處理收銀機傳來的資料
*/
int inUSB_ECR_8N1_Customer_107_Bumper_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal;
	char	szDataBuffer[_ECR_USB_BUFF_SIZE_];	/* 電文不包含STX和LRC */
	
	
	memset(&szDataBuffer, 0x00, sizeof(szDataBuffer));
/* -----------------------開始接收資料------------------------------------------ */
	srECROb->srSetting.inTimeout = _ECR_RECEIVE_REQUEST_TIMEOUT_;
	inRetVal = inECR_Receive(pobTran, srECROb, szDataBuffer, _ECR_8N1_Standard_Data_Size_);
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
/* -----------------------開始分析資料------------------------------------------ */
	inRetVal = inECR_8N1_Customer_107_Bumper_Unpack(pobTran, srECROb, szDataBuffer);
	
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
Function        :inUSB_ECR_8N1_Customer_107_Bumper_Send_Packet
Date&Time       :2016/7/11 下午 3:29
Describe        :處理要送給收銀機的資料
*/
int inUSB_ECR_8N1_Customer_107_Bumper_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_ERROR;
	int	inSendSize = 0;
	char	szDataBuf[_ECR_USB_BUFF_SIZE_] = {0};	/* 封包資料 */
	
	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransData.uszIsResponse == VS_TRUE)
		return (VS_SUCCESS);
	
	/* 防停車場ECR遺失問題，判斷是否票證遺失檔案 */
	if (srECROb->srSetting.inVersion == atoi(_ECR_RS232_VERSION_STANDARD_KIOSK_))
	{
		TRANSACTION_OBJECT	pobDataTran = {0};
		memset(&pobDataTran, 0x00, sizeof(pobDataTran));
		inNCCC_Func_Get_Temp_PobTran_For_ECR_Missing_Data(&pobDataTran);
		
		if (pobDataTran.srTRec.uszESVCTransBit == VS_TRUE)
		{
			memcpy(&pobTran->srTRec, &pobDataTran.srTRec, sizeof(TICKET_REC));
		}
	}
	
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
	if (memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SETTLEMENT_, strlen(_ECR_8N1_SETTLEMENT_)) == 0)
	{
		inRetVal = inECR_8N1_Customer_Self_Trans_Settle_Pack(pobTran, srECROb, szDataBuf);
	}
	else if (memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INQUIRY_LAST_TRANSACTION_, strlen(_ECR_8N1_INQUIRY_LAST_TRANSACTION_)) == 0)
	{
		inRetVal = inECR_8N1_Inquiry_Last_Transaction_Pack(pobTran, srECROb, szDataBuf);
	}
	else
	{
		inRetVal = inECR_8N1_Standard_Pack(pobTran, srECROb, szDataBuf);
	}
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
	/* (需求單 - 107227)邦柏科技自助作業客製化 存起ECR Out Data by Russell 2018/12/7 下午 2:36 */
	if (memcmp(srECROb->srTransData.szTransType, _ECR_8N1_START_CARD_NO_INQUIRY_, strlen(_ECR_8N1_START_CARD_NO_INQUIRY_)) != 0	&&
	    memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INQUIRY_LAST_TRANSACTION_, strlen(_ECR_8N1_INQUIRY_LAST_TRANSACTION_)) != 0)
	{
		inECR_Save_Response(szDataBuf, inSendSize);
	}
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, inSendSize);
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponse = VS_TRUE;
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inUSB_ECR_8N1_Customer_107_Bumper_Send_Error
Date&Time       :2016/7/18 上午 10:21
Describe        :傳送錯誤訊息ECR
*/
int inUSB_ECR_8N1_Customer_107_Bumper_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb)
{
	int	inRetVal = VS_ERROR;
	int	inSendSize = 0;
	char	szDataBuf[_ECR_USB_BUFF_SIZE_] = {0};	/* 封包資料 */
	char	szCustomerIndicator[3 + 1] = {0};
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
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
	if (memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SETTLEMENT_, strlen(_ECR_8N1_SETTLEMENT_)) == 0)
	{
		inRetVal = inECR_8N1_Customer_Self_Trans_Settle_Pack(pobTran, srECROb, szDataBuf);
	}
	else
	{
		inRetVal = inECR_8N1_Standard_Pack_Error(pobTran, srECROb, szDataBuf);
	}
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------塞進錯誤訊息--------------------------------------------- */
	inECR_8N1_Customer_107_Bumper_Pack_ResponseCode(pobTran, srECROb, szDataBuf);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
	/* (需求單 - 107227)邦柏科技自助作業客製化 存起ECR Out Data by Russell 2018/12/7 下午 2:36 */
	inECR_Save_Response(szDataBuf, inSendSize);
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, inSendSize);
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponse = VS_TRUE;
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inUSB_ECR_8N1_Customer_111_Kiosk_Standard_Receive_Packet
Date&Time       :2019/2/12 下午 2:06
Describe        :處理收銀機傳來的資料
*/
int inUSB_ECR_8N1_Customer_111_Kiosk_Standard_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal;
	char	szDataBuffer[_ECR_USB_BUFF_SIZE_];	/* 電文不包含STX和LRC */
	
	/* 防停車場ECR遺失問題，開始ECR前刪暫存檔 */
	if (srECROb->srSetting.inVersion == atoi(_ECR_RS232_VERSION_STANDARD_KIOSK_))
	{
		inNCCC_Func_Delete_Temp_PobTran_For_ECR_Missing_Data(pobTran);
	}
	
	memset(&szDataBuffer, 0x00, sizeof(szDataBuffer));
/* -----------------------開始接收資料------------------------------------------ */
	srECROb->srSetting.inTimeout = _ECR_RECEIVE_REQUEST_TIMEOUT_;
	inRetVal = inECR_Receive(pobTran, srECROb, szDataBuffer, _ECR_8N1_Standard_Data_Size_);
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
/* -----------------------開始分析資料------------------------------------------ */
	inRetVal = inECR_8N1_Customer_111_Kiosk_Standard_Unpack(pobTran, srECROb, szDataBuffer);
	
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
Function        :inUSB_ECR_8N1_Customer_039_CARD_NO_HIDE_F8_B4_Send_Packet
Date&Time       :2021/2/1 下午 4:26
Describe        :處理要送給收銀機的資料
*/
int inUSB_ECR_8N1_Customer_039_CARD_NO_HIDE_F8_B4_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_ERROR;
	char	szDataBuf[_ECR_USB_BUFF_SIZE_] = {0};	/* 封包資料 */
	
	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransData.uszIsResponse == VS_TRUE)
		return (VS_SUCCESS);
	
	/* 初始化 */
	memset(szDataBuf, 0x00, sizeof(szDataBuf));
/* ---------------------包裝電文--------------------------------------------- */
	inRetVal = inECR_8N1_Customer_039_SKYKAND_Pack(pobTran, srECROb, szDataBuf);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
	if (memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INQUIRY_LAST_TRANSACTION_, strlen(_ECR_8N1_INQUIRY_LAST_TRANSACTION_)) != 0)
	{
		inECR_Save_Response(szDataBuf, _ECR_8N1_Standard_Data_Size_);
	}
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, _ECR_8N1_Standard_Data_Size_);
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponse = VS_TRUE;
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inUSB_ECR_8N1_Customer_039_CARD_NO_HIDE_F8_B4_Send_Error
Date&Time       :2021/2/1 下午 4:26
Describe        :傳送錯誤訊息ECR
*/
int inUSB_ECR_8N1_Customer_039_CARD_NO_HIDE_F8_B4_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb)
{
	int	inRetVal;
	char	szDataBuf[_ECR_USB_BUFF_SIZE_];	/* 封包資料 */
	
	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransData.uszIsResponse == VS_TRUE)
		return (VS_SUCCESS);
	
	/* 初始化 */
	memset(szDataBuf, 0x00, sizeof(szDataBuf));	
/* ---------------------包裝電文--------------------------------------------- */
	inRetVal = inECR_8N1_Customer_039_SKYKAND_Pack_Error(pobTran, srECROb, szDataBuf);
	
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
	
	if (memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INQUIRY_LAST_TRANSACTION_, strlen(_ECR_8N1_INQUIRY_LAST_TRANSACTION_)) != 0)
	{
		inECR_Save_Response(szDataBuf, _ECR_8N1_Standard_Data_Size_);
	}
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, _ECR_8N1_Standard_Data_Size_);
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponse = VS_TRUE;
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inUSB_ECR_8N1_Standard_Mirror_Packet
Date&Time       :2025/10/17 下午 5:14
Describe        :送給收銀機目前端末機行為
*/
int inUSB_ECR_8N1_Standard_Mirror_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_ERROR;
	int	inSendSize = 0;
	char	szDataBuf[_ECR_USB_BUFF_SIZE_] = {0};	/* 封包資料 */
	
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