#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <linux/stddef.h>
#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Define_2.h"
#include "../INCLUDES/Transaction.h"
#include "../INCLUDES/TransType.h"
#include "../INCLUDES/AllStruct.h"
#include "../DISPLAY/Display.h"
#include "../DISPLAY/DisTouch.h"
#include "../DISPLAY/DispMsg.h"
#include "../PRINT/Print.h"
#include "../FUNCTION/Function.h"
#include "../EVENT/MenuMsg.h"
#include "../FUNCTION/CFGT.h"
#include "../FUNCTION/HDT.h"
#include "../FUNCTION/CPT.h"
#include "../FUNCTION/EDC.h"
#include "../FUNCTION/Utility.h"
#include "../../NCCC/NCCCtmsCPT.h"
#include "Comm.h"
#include "Ethernet.h"
#include "Modem.h"

extern  int     ginDebug;  /* Debug使用 extern */
extern	int	ginFindRunTime;
/* 聯合測試授權電話:22675604 */
/*
Function        :inModem_Open
Date&Time       :2017/7/19 下午 2:37
Describe        :
*/
int inModem_Open()
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inModem_Open() START !");
	}
	
	usRetVal = CTOS_ModemOpen(d_M_MODE_SDLC_FAST, d_M_HANDSHAKE_V22_ONLY, d_M_COUNTRY_TAIWAN);
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "CTOS_ModemOpen OK!");
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_ModemOpen Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		vdUtility_SYSFIN_LogMessage(AT, "CTOS_ModemOpen Err :0x%04X", usRetVal);
		return (VS_ERROR);
	}
	
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inModem_Open() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inModem_Close
Date&Time       :2017/7/19 下午 2:37
Describe        :
*/
int inModem_Close()
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inModem_Close() START !");
	}
	
	usRetVal = CTOS_ModemClose();
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "CTOS_ModemClose OK!");
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_ModemClose Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inModem_Close() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inModem_Dial
Date&Time       :2017/7/19 下午 2:37
Describe        :
*/
int inModem_Dial(unsigned char *uszNumber, unsigned short usLen)
{
	int		i = 0;
	char		szDebugMsg[100 + 1] = {0};
	unsigned char	*uszModfiedNumber = NULL;
	unsigned short	usRetVal;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inModem_Dial() START !");
	}
	
	uszModfiedNumber = malloc(usLen + 1);
	/* 聯合TMS會將,儲存為p，這裡要將p轉回來 */
	for (i = 0; i < usLen; i++)
	{
		if (*(uszNumber + i) == 'p'	||
		    *(uszNumber + i) == 'P')
		{
			*(uszModfiedNumber + i) = ',';
			
			/* 虹堡API一個comma延遲兩秒，所以兩個p轉換成1個 */
			if (*(uszNumber + i + 1) == 'p'	||
			    *(uszNumber + i + 1) == 'P')
			{
				i++;
			}
		}
		else
		{
			*(uszModfiedNumber + i) = *(uszNumber + i);
		}
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "Modfied Number:%s", uszModfiedNumber);
	}
	
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("Dial start");
	}
	
	usRetVal = CTOS_ModemDialup(uszModfiedNumber, usLen);
	
	/* 釋放記憶體 */
	free(uszModfiedNumber);
	
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "ModemDialup OK!");
		}
	}
	else if (usRetVal == d_MODEM_ALREADY_ONLINE)
	{
		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "MODEM_ALREADY_ONLINE");
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_ModemDialup Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
                vdUtility_SYSFIN_LogMessage(AT, "撥號失敗，CTOS_ModemDialup Err :0x%04X", usRetVal);
		return (VS_ERROR);
	}
	
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("Dial End");
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inModem_Dial() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inModem_GetStatus
Date&Time       :2017/7/20 上午 11:14
Describe        :
*/
int inModem_GetStatus(unsigned int *uiStatus)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	usRetVal = CTOS_ModemStatus(uiStatus);
	if (usRetVal == d_OK)
	{
//		if (ginDebug == VS_TRUE)
//		{
//			inLogPrintf(AT, "Get Status OK!");
//			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
//			sprintf(szDebugMsg, "Status :0x%08X", *uiStatus);
//			inLogPrintf(AT, szDebugMsg);
//		}
	}
	else if (usRetVal == d_MODEM_NOT_OPEN)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Modem not open, Can't get Status");
			inLogPrintf(AT, szDebugMsg);
		}
                vdUtility_SYSFIN_LogMessage(AT, "CTOS_ModemStatus failed Not opend Err :0x%04X", usRetVal);
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_ModemStatus Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
                vdUtility_SYSFIN_LogMessage(AT, "CTOS_ModemStatus failed Err :0x%04X", usRetVal);
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inModem_WatchStatus
Date&Time       :2017/7/20 下午 3:08
Describe        :
*/
int inModem_WatchStatus(void)
{
	//Declare Local Variable //
	unsigned int	uiStatus;
 
	//Get the status of the modem //
	inModem_GetStatus(&uiStatus);
	
	if (uiStatus & d_M_STATUS_MODEM_ONLINE)
	{
		inLogPrintf(AT, "Modem Online");
	}
  
	if (uiStatus & d_M_STATUS_SDLC_MODE)
	{
		inLogPrintf(AT, "SDLC Mode");
	}

	if (uiStatus & d_M_STATUS_SDLC_ONLINE)
	{
		inLogPrintf(AT, "SDLC Online");
	}

	if (uiStatus & d_M_STATUS_DIALING)
	{
		inLogPrintf(AT, "Dialing");
	}

	if (uiStatus & d_M_STATUS_NO_DIAL_TONE)
	{
		inLogPrintf(AT, "No Dial Tone");
	}

	if (uiStatus & d_M_STATUS_LINE_BUSY)
	{
		inLogPrintf(AT, "Line Busy");
	}

	if (uiStatus & d_M_STATUS_RING_BACK)
	{
		inLogPrintf(AT, "Ring Back");
	}

	if (uiStatus & d_M_STATUS_TX_BUSY)
	{
		inLogPrintf(AT, "Tx Busy");
	}

	if (uiStatus & d_M_STATUS_REMOTE_NOT_ANSWER)
	{
		inLogPrintf(AT, "Remote Not Answer");
	}

	if (uiStatus & d_M_STATUS_NO_CARRIER)
	{
		inLogPrintf(AT, "NO Carrier");
	}

	if (uiStatus & d_M_STATUS_ALL_DATA_SENT)
	{
		inLogPrintf(AT, "All Data Sent");
	}

	if (uiStatus & d_M_STATUS_RX_DATA_VALID)
	{
	      inLogPrintf(AT, "Rx Data Vaild");
	}

	if (uiStatus & d_M_STATUS_LISTENING)
	{
		inLogPrintf(AT, "Listening");
	}

	if (uiStatus & d_M_STATUS_OTHER_ERROR)
	{
		inLogPrintf(AT, "Other Error");
	}

	if (uiStatus & d_M_STATUS_DATA_SENT_ERROR)
	{
		inLogPrintf(AT, "Data Sent Error");
	}
  
	return (VS_SUCCESS);
}

/*
Function        :inModem_IsAlreadyOpen
Date&Time       :2017/7/20 上午 11:22
Describe        :
*/
int inModem_IsAlreadyOpen(unsigned int uiStatus)
{
	if ((uiStatus & d_M_STATUS_MODEM_OPEN) == d_M_STATUS_MODEM_OPEN)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "d_M_STATUS_MODEM_OPEN");
		}
		return (VS_SUCCESS);
	}
	else
		return (VS_ERROR);
}

/*
Function        :inModem_IsAlready_MODEM_Online
Date&Time       :2017/7/20 上午 11:43
Describe        :
*/
int inModem_IsAlready_MODEM_Online(unsigned int uiStatus)
{
	if ((uiStatus & d_M_STATUS_MODEM_ONLINE) == d_M_STATUS_MODEM_ONLINE)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "d_M_STATUS_MODEM_ONLINE");
		}
		return (VS_SUCCESS);
	}
	else
		return (VS_ERROR);
}

/*
Function        :inModem_IsAlready_SDLC_MODE
Date&Time       :2017/7/20 上午 11:44
Describe        :
*/
int inModem_IsAlready_SDLC_MODE(unsigned int uiStatus)
{
	if ((uiStatus & d_M_STATUS_SDLC_MODE) == d_M_STATUS_SDLC_MODE)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "d_M_STATUS_SDLC_MODE");
		}
		return (VS_SUCCESS);
	}
	else
		return (VS_ERROR);
}

/*
Function        :inModem_IsAlready_SDLC_Online
Date&Time       :2017/7/19 下午 3:16
Describe        :
*/
int inModem_IsAlready_SDLC_Online(unsigned int uiStatus)
{
	if ((uiStatus & d_M_STATUS_SDLC_ONLINE) == d_M_STATUS_SDLC_ONLINE)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "d_M_STATUS_SDLC_ONLINE");
		}
		return (VS_SUCCESS);
	}
	else
		return (VS_ERROR);
}

/*
Function        :inModem_IsAlready_Dialing
Date&Time       :2017/7/19 下午 3:16
Describe        :
*/
int inModem_IsAlready_Dialing(unsigned int uiStatus)
{
	if ((uiStatus & d_M_STATUS_DIALING) == d_M_STATUS_DIALING)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "d_M_STATUS_DIALING");
		}
		return (VS_SUCCESS);
	}
	else
		return (VS_ERROR);
}

/*
Function        :inModem_IsAlready_LineBusy
Date&Time       :2022/1/24 上午 10:50
Describe        :
*/
int inModem_IsAlready_LineBusy(unsigned int uiStatus)
{
	if ((uiStatus & d_M_STATUS_LINE_BUSY) == d_M_STATUS_LINE_BUSY)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "d_M_STATUS_LINE_BUSY");
		}
		return (VS_SUCCESS);
	}
	else
		return (VS_ERROR);
}

/*
Function        :inModem_Send_Ready
Date&Time       :2017/7/20 下午 5:21
Describe        :
*/
int inModem_Send_Ready()
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inModem_Send_Ready() START !");
	}
	
	usRetVal = CTOS_ModemTxReady();
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "CTOS_ModemTxReady OK!");
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_ModemTxReady Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inModem_Send_Ready() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inModem_Send_Data
Date&Time       :2017/7/20 下午 5:32
Describe        :傳送資料
*/
int inModem_Send_Data(unsigned char* uszData, unsigned short usLen)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	usRetVal = CTOS_ModemTxData(uszData, usLen);
	if (usRetVal == d_OK)
	{
		
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_ModemTxData Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inModem_Receive_Ready
Date&Time       :2017/7/20 下午 5:34
Describe        :表示現在Modem可以接收資料
*/
int inModem_Receive_Ready(unsigned short *usLen)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	usRetVal = CTOS_ModemRxReady(usLen);
	if (usRetVal == d_OK)
	{
		
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inModem_Receive_Ready Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inModem_Receive_Data
Date&Time       :2017/7/20 下午 5:34
Describe        :接收資料
*/
int inModem_Receive_Data(unsigned char* uszData, unsigned short *usLen)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	usRetVal = CTOS_ModemRxData(uszData, usLen);
	if (usRetVal == d_OK)
	{
		
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_ModemRxData Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inModem_Hook_On
Date&Time       :2017/7/20 下午 5:47
Describe        :掛上電話
*/
int inModem_Hook_On()
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inModem_Hook_On() START !");
	}
	
	usRetVal = CTOS_ModemHookOn();
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "CTOS_ModemHookOn OK!");
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_ModemHookOn Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inModem_Hook_On() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inModem_Flush_Rx
Date&Time       :2017/7/20 下午 5:53
Describe        :清空Receive Buffer
*/
int inModem_Flush_Rx()
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inModem_Flush_Rx() START !");
	}
	
	usRetVal = CTOS_ModemFlushRxData();
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "CTOS_ModemFlushRxData OK!");
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_ModemFlushRxData Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inModem_Flush_Rx() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inModem_Predial
Date&Time       :2017/7/19 下午 2:34
Describe        :
*/
int inModem_Predial(TRANSACTION_OBJECT *pobTran)
{
	char	szCommMode[1 + 1];
	char	szNumber[18 + 1];
	char	szPABXCode[2 + 1];
	char	szDebugMsg[100 + 1];
        
        vdUtility_SYSFIN_LogMessage(AT, "inModem_Predial START!");
	
        /* inFuncModemCreditPredial() START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inModem_Predial() START！");
        }
	
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
	}
	
	memset(szCommMode, 0x00, sizeof(szCommMode));
	inGetCommMode(szCommMode);
		
	if (memcmp(szCommMode, _COMM_MODEM_MODE_, 1) == 0)
	{
		inModem_Initial();
		
		if (inModem_Begin(pobTran) != VS_SUCCESS)
		{
			vdUtility_SYSFIN_LogMessage(AT, "inModem_Predial inModem_Begin failed");
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "inModem_Predial_Begin_ERROR!!");
			}
			
			return(VS_ERROR);
		}
		
		/* 撥出的號碼*/
		memset(szNumber, 0x00, sizeof(szNumber));
		memset(szPABXCode, 0x00, sizeof(szPABXCode));

		/* PABX code */
		inGetPABXCode(szPABXCode);
		memcpy(szNumber, szPABXCode, strlen(szPABXCode));	/* PABX Code */

		/* 第一授權電話 */
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		inGetHostTelPrimary(szDebugMsg);
		strcat(szNumber, szDebugMsg);				/* Phone Number */

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Phone Number = %s", szNumber);
			inLogPrintf(AT, szDebugMsg);
		}

		/* 撥了就跑，不看回傳值 */
		inModem_Dial((unsigned char*)szNumber, strlen(szNumber));
				
	}

        /* inModem_Predial()_END */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inModem_Predial()_END");
		inLogPrintf(AT, "----------------------------------------");
        }
	vdUtility_SYSFIN_LogMessage(AT, "inModem_Predial END!");
        
	return(VS_SUCCESS);
}

/*
Function        :inModem_Initial
Date&Time       :
Describe        :
*/
int inModem_Initial(void)
{
	int		inRetVal;
	unsigned int	uiStatus = 0;
	
	/* inModem_Initial() START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inModem_Initial() START！");
        }
	
	inModem_GetStatus(&uiStatus);
	/* 如果已經Open，不用重複Open */
	if (inModem_IsAlreadyOpen(uiStatus) == VS_SUCCESS)
	{
		/* 已經open就不用再open */
		if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "MODEM_ALREADY_OPEN_SKIP");
		
		vdUtility_SYSFIN_LogMessage(AT, "MODEM_ALREADY_OPEN_SKIP");
		return (VS_SUCCESS);
	}
	
	/* Portable 機型若沒接上底座再開Modem會失敗 */
	inRetVal = inModem_Open();
	if (inRetVal == VS_SUCCESS)
	{

	}
	else
	{
		vdUtility_SYSFIN_LogMessage(AT, "Modem open failed");
		return (VS_ERROR);
	}
	
	/* 解決modem環境較為不好時，要第二次才能連上的問題，設固定值為200 (單位100ms) */
	inModem_SetConfig(d_M_CONFIG_CARRIER_PRESENT_TIME, 255);

        /* inModem_Initial()_END */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inModem_Initial()_END");
		inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inModem_Begin
Date&Time       :2017/7/20 上午 11:27
Describe        :
*/
int inModem_Begin(TRANSACTION_OBJECT *pobTran)
{
	char    szCommIndex[2 + 1];

        /* inModem_Begin() START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inModem_Begin() START！");
        }

        memset(szCommIndex, 0x00, sizeof(szCommIndex));
        /* 從HDT內get CPT的index */
        if (inGetCommunicationIndex(szCommIndex) == VS_ERROR)
        {
                /* get失敗 */
                return (VS_ERROR);
        }
        else
        {
                /* CPT index從0開始，但是CPTRec index從1開始，所以要減一  */
                pobTran->srBRec.inCPTIndex = atoi(szCommIndex) - 1;
        }

        if (inLoadCPTRec(pobTran->srBRec.inCPTIndex) == VS_ERROR)
        {
                /* Load CPT失敗 */
                return (VS_ERROR);
        }

        /* inModem_Begin()_END */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inModem_Begin()_END");
		inLogPrintf(AT, "----------------------------------------");
        }

        return (VS_SUCCESS);
}

/*
Function        :inModem_Connect
Date&Time       :2017/7/19 下午 2:02
Describe        :
*/
int inModem_Connect(void)
{
	int		inRetval;
	int		inTimeout = 0;
	char		szNumber[18 + 1];
	char		szFinalNumber[20 + 1];
	char		szDebugMsg[100 + 1];
	char		szPABXCode[2 + 1];
	char		szCarrierTimeOut[2 + 1];
	unsigned int	uiStatus = 0;
	unsigned char	uszFirst_Tel_FailBit = VS_FALSE;	/* 第一組授權電話是否失敗 */
	unsigned char	uszSecond_Tel_FailBit = VS_FALSE;	/* 第二組授權電話是否失敗 */
	unsigned char	uszKey = 0;
	
        if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inModem_Connect() START!");
	}
		
	inRetval = inModem_GetStatus(&uiStatus);
	if (inRetval != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	if (inModem_IsAlready_SDLC_Online(uiStatus) == VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Already SDLC Online");
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_SUCCESS);
	}
		
	/* 組PABX*/
	memset(szPABXCode, 0x00, sizeof(szPABXCode));
	/* PABX code */
	inGetPABXCode(szPABXCode);
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "PABX = %s", szPABXCode);
		inLogPrintf(AT, szDebugMsg);
	}
	
/*****************第一授權電話*******************************/
	/* 組第一授權電話 */
	memset(szNumber, 0x00, sizeof(szNumber));
	inGetHostTelPrimary(szNumber);
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Phone Number = %s", szNumber);
		inLogPrintf(AT, szDebugMsg);
	}
	
	/* 最後播出號碼(不能分開撥，我試過了) */
	memset(szFinalNumber, 0x00, sizeof(szFinalNumber));
	sprintf(szFinalNumber, "%s%s", szPABXCode, szNumber);
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Final Number = %s", szFinalNumber);
		inLogPrintf(AT, szDebugMsg);
	}

	/* 若已經預撥就不用再撥 */
	if (inModem_IsAlready_Dialing(uiStatus) == VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Already Dialing");
			inLogPrintf(AT, szDebugMsg);
		}
		
	}
	else
	{
		/* 撥第一授權電話*/
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont("撥號中一..", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
		inRetval = inModem_Dial((unsigned char*)szFinalNumber, strlen(szFinalNumber));
		if (inRetval == VS_SUCCESS)
		{
			/* 撥成功 */
		}
		else
		{
			/* 第一授權失敗 */
			uszFirst_Tel_FailBit = VS_TRUE;
		}
	
	}
	
	if (ginDebug == VS_TRUE)
	{
		inModem_WatchStatus();
	}
	
	/* 第一次授權沒失敗就繼續往下做 */
	if (uszFirst_Tel_FailBit != VS_TRUE)
	{
		/* 連線timeout時間 */
		memset(szCarrierTimeOut, 0x00, sizeof(szCarrierTimeOut));
		inGetCarrierTimeOut(szCarrierTimeOut);
		inTimeout = atoi(szCarrierTimeOut);

		if (inTimeout != 0)
			inDISP_Timer_Start(_TIMER_NEXSYS_1_, inTimeout);
		/* 確定是否撥號中，非撥號狀態判斷MODEM 是否Online，不Online就跳出 */
		while (1)
		{
			/* 按取消可以切斷連線 */
			uszKey = uszKBD_Key();
			if (uszKey == _KEY_CANCEL_)
			{
				/* 掛電話 */
				inModem_END();
				/* 第一授權失敗 */
				uszFirst_Tel_FailBit = VS_TRUE;
				return (VS_ERROR);
			}
			
			/* 抓狀態 */
			inRetval = inModem_GetStatus(&uiStatus);
			if (inRetval != VS_SUCCESS)
			{
				return (VS_ERROR);
			}
			
			/* 確認Timeout */
			if (inTimeout != 0)
			{
				if (inTimerGet(_TIMER_NEXSYS_1_) ==VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "Modem Not Online_TIMEOUT");
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "Modem Not Online Status: 0x%08x", uiStatus);
						inLogPrintf(AT, szDebugMsg);

					}
					/* 第一授權失敗 */
					uszFirst_Tel_FailBit = VS_TRUE;
					break;
				}
			}

			/* 確認是否連上線 */
			if (inModem_IsAlready_MODEM_Online(uiStatus) == VS_SUCCESS)
			{
				/* 連上線，跳出 */
				break;
			}
		}
	}
	
	if (ginDebug == VS_TRUE)
	{
		inModem_WatchStatus();
	}
	
	/* 第一次授權沒失敗就繼續往下做 */
	if (uszFirst_Tel_FailBit != VS_TRUE)
	{
		/* 等SDLC Online */
		while (1)
		{
			/* 按取消可以切斷連線 */
			uszKey = uszKBD_Key();
			if (uszKey == _KEY_CANCEL_)
			{
				/* 掛電話 */
				inModem_END();
				/* 第一授權失敗 */
				uszFirst_Tel_FailBit = VS_TRUE;
				return (VS_ERROR);
			}
			
			/* 抓狀態 */
			inRetval = inModem_GetStatus(&uiStatus);
			if (inRetval != VS_SUCCESS)
			{
				return (VS_ERROR);
			}
			
			if (inTimeout != 0)
			{
				if (inTimerGet(_TIMER_NEXSYS_1_) ==VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "SDLC Not Online_TIMEOUT");
						if (ginDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
							sprintf(szDebugMsg, "Modem Not Online Status: 0x%08x", uiStatus);
							inLogPrintf(AT, szDebugMsg);
						}

					}
					/* 第一授權失敗 */
					uszFirst_Tel_FailBit = VS_TRUE;
					break;
				}
			}

			/* 確認是否連上線 */
			if (inModem_IsAlready_SDLC_Online(uiStatus) == VS_SUCCESS)
			{
				/* 連上線，跳出 */
				break;
			}

		}
	}
	
	if (ginDebug == VS_TRUE)
	{
		inModem_WatchStatus();
	}
	
/*****************第二授權電話*******************************/
	/* 第一授權電話失敗，掛掉改播第二授權電話 */
	if (uszFirst_Tel_FailBit == VS_TRUE)
	{
		/* 先把第一授權電話掛掉 */
		inModem_END();
		
		/* 組第二授權電話 */
		memset(szNumber, 0x00, sizeof(szNumber));
		inGetHostTelSecond(szNumber);
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Phone Number = %s", szNumber);
			inLogPrintf(AT, szDebugMsg);
		}
		
		/* 最後播出號碼(不能分開撥，我試過了) */
		memset(szFinalNumber, 0x00, sizeof(szFinalNumber));
		sprintf(szFinalNumber, "%s%s", szPABXCode, szNumber);
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Final Number = %s", szFinalNumber);
			inLogPrintf(AT, szDebugMsg);
		}
		
		/* 撥第二授權電話*/
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont("撥號中二..", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
		inRetval = inModem_Dial((unsigned char*)szFinalNumber, strlen(szFinalNumber));
		if (inRetval == VS_SUCCESS)
		{
			/* 撥成功 */
		}
		else
		{
			/* 第二授權失敗 */
			uszSecond_Tel_FailBit = VS_TRUE;
		}
		
		if (ginDebug == VS_TRUE)
		{
			inModem_WatchStatus();
		}

		/* 第二授權沒失敗就繼續往下做 */
		if (uszSecond_Tel_FailBit != VS_TRUE)
		{
			/* 連線timeout時間 */
			memset(szCarrierTimeOut, 0x00, sizeof(szCarrierTimeOut));
			inGetCarrierTimeOut(szCarrierTimeOut);
			inTimeout = atoi(szCarrierTimeOut);

			if (inTimeout != 0)
				inDISP_Timer_Start(_TIMER_NEXSYS_1_, inTimeout);
			/* 確定是否撥號中，非撥號狀態判斷MODEM 是否Online，不Online就跳出 */
			while (1)
			{
				/* 按取消可以切斷連線 */
				uszKey = uszKBD_Key();
				if (uszKey == _KEY_CANCEL_)
				{
					/* 掛電話 */
					inModem_END();
					/* 第二授權失敗 */
					uszSecond_Tel_FailBit = VS_TRUE;
					return (VS_ERROR);
				}
			
				/* 抓狀態 */
				inRetval = inModem_GetStatus(&uiStatus);
				if (inRetval != VS_SUCCESS)
				{
					return (VS_ERROR);
				}
				
				/* 確認Timeout */
				if (inTimeout != 0)
				{
					if (inTimerGet(_TIMER_NEXSYS_1_) ==VS_SUCCESS)
					{
						if (ginDebug == VS_TRUE)
						{
							inLogPrintf(AT, "Modem Not Online_TIMEOUT");
							memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
							sprintf(szDebugMsg, "Modem Not Online Status: 0x%08x", uiStatus);
							inLogPrintf(AT, szDebugMsg);

						}
						/* 第二授權失敗 */
						uszSecond_Tel_FailBit = VS_TRUE;
						break;
					}
				}

				/* 確認是否連上線 */
				if (inModem_IsAlready_MODEM_Online(uiStatus) == VS_SUCCESS)
				{
					/* 連上線，跳出 */
					break;
				}
			}
		}
		
		if (ginDebug == VS_TRUE)
		{
			inModem_WatchStatus();
		}

		/* 第二授權沒失敗就繼續往下做 */
		if (uszSecond_Tel_FailBit != VS_TRUE)
		{
			/* 等SDLC Online */
			while (1)
			{
				/* 按取消可以切斷連線 */
				uszKey = uszKBD_Key();
				if (uszKey == _KEY_CANCEL_)
				{
					/* 掛電話 */
					inModem_END();
					/* 第一授權失敗 */
					uszFirst_Tel_FailBit = VS_TRUE;
					return (VS_ERROR);
				}
			
				/* 抓狀態 */
				inRetval = inModem_GetStatus(&uiStatus);
				if (inRetval != VS_SUCCESS)
				{
					return (VS_ERROR);
				}
				
				if (inTimeout != 0)
				{
					if (inTimerGet(_TIMER_NEXSYS_1_) ==VS_SUCCESS)
					{
						if (ginDebug == VS_TRUE)
						{
							inLogPrintf(AT, "SDLC Not Online_TIMEOUT");
							if (ginDebug == VS_TRUE)
							{
								memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
								sprintf(szDebugMsg, "Modem Not Online Status: 0x%08x", uiStatus);
								inLogPrintf(AT, szDebugMsg);
							}

						}
						/* 第二授權失敗 */
						uszSecond_Tel_FailBit = VS_TRUE;
						break;
					}
				}

				/* 確認是否連上線 */
				if (inModem_IsAlready_SDLC_Online(uiStatus) == VS_SUCCESS)
				{
					/* 連上線，跳出 */
					break;
				}

			}
		}
	}

/*****************最後連線結果*******************************/
	if (ginDebug == VS_TRUE)
	{
		inModem_WatchStatus();
	}
	
	if (uszFirst_Tel_FailBit != VS_TRUE)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "第一授權電話連線完成");
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "狀態: 0x%08x", uiStatus);
			inLogPrintf(AT, szDebugMsg);
		}
	}
	else if (uszSecond_Tel_FailBit != VS_TRUE)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "第二授權電話連線完成");
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "狀態: 0x%08x", uiStatus);
			inLogPrintf(AT, szDebugMsg);
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "連線失敗");
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "狀態: 0x%08x", uiStatus);
			inLogPrintf(AT, szDebugMsg);
		}
		inModem_END();
		
		return (VS_ERROR);
	}
	
        if (ginDebug == VS_TRUE)
	{
                inLogPrintf(AT, "inModem_Dial() END!");
		inLogPrintf(AT, "----------------------------------------");
	}
        
        return (VS_SUCCESS);
}

/*
Function        :inModem_Send
Date&Time       :2017/7/20 上午 9:16
Describe        :
*/
int inModem_Send(unsigned char* uszSendBuff, int inSendSize, int inTimeout)
{
	int	inRetVal = VS_SUCCESS;
	char	szSendData[inSendSize + 32];

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inModem_Send() START!");
	}
	
	memset(szSendData, 0x00, sizeof(szSendData));
	memcpy(&szSendData[0], uszSendBuff, inSendSize);
	
	if (ginDebug == VS_TRUE)
	{
		inModem_WatchStatus();
	}
	
	if (inTimeout != 0)
                inDISP_Timer_Start(_TIMER_NEXSYS_1_, inTimeout);

	while (1)
	{
		if (inTimeout != 0)
                {
                        if (inTimerGet(_TIMER_NEXSYS_1_) ==VS_SUCCESS)
                        {
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "inModem_Send()_TIMEOUT");
				}
				
                                return(VS_TIMEOUT);
                        }
                }
		
		/* 等ready */
		if (inModem_Send_Ready() == VS_SUCCESS)
		{
			inRetVal = inModem_Send_Data((unsigned char*)szSendData, inSendSize);
			if (inRetVal != VS_SUCCESS)
			{
				inLogPrintf(AT, "inModem_Send_Data() ERROR!!");
				continue;
			}
			else
			{
				break;
			}
		}
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inModem_Send() END!");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inModem_Receive
Date&Time       :2017/7/20 上午 10:20
Describe        :
*/
int inModem_Receive(unsigned char* uszReceiveBuff, int inReceiveSize, int inReceiveTimeout)
{
	int		inRetVal;
	int		inReceivelen = 0;	/* Comport當前收到的長度 */
	int     	inDataLength = 0;	/* 收到的資料長度 */
	unsigned char	uszRawBuffer[inReceiveSize + 1];
	unsigned short	usOutputLen = 0;

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inModem_Receive() START!");
	}
	
	memset(uszRawBuffer, 0x00, sizeof(uszRawBuffer));

	if (inReceiveTimeout != 0)
                inDISP_Timer_Start(_TIMER_NEXSYS_1_, inReceiveTimeout);

	/* Modem不用收Head 所以只要一段 */
	/* 準備收下一次，清空暫存Buffer */
	inReceivelen = 0;

	while (1)
	{
		/* Timeout的話 */
		if (inReceiveTimeout != 0)
		{
			if (inTimerGet(_TIMER_NEXSYS_1_) ==VS_SUCCESS)
			{
				if (ginDebug == VS_TRUE)
				{
					if (inReceiveTimeout != 0)
					{
						inLogPrintf(AT, "inModem_Receive TimeOut");
						inModem_WatchStatus();
					}
				}
				return (VS_TIMEOUT);
			}
		}

		/* 看Comport是否還有沒收的 */
		inRetVal = inModem_Receive_Ready(&usOutputLen);
		if (inRetVal == VS_SUCCESS)
		{
			inModem_Receive_Data(&uszRawBuffer[inReceivelen], &usOutputLen);
			inReceivelen = inReceivelen + (int)usOutputLen;

			/* 沒有可以收的就跳出去 */
			if (inReceivelen > 0 && usOutputLen == 0)
			{
				break;
			}
		}

	}

	memcpy(&uszReceiveBuff[inDataLength], &uszRawBuffer[0], inReceivelen);
	inDataLength += inReceivelen;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inModem_Receive() END!");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inDataLength);
}

/*
Function        :inModem_END
Date&Time       :2017/7/20 下午 5:45
Describe        :
*/
int inModem_END(void)
{
	int		inRetVal;
	char		szDebugMsg[100 + 1];
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inModem_END() START!");
	}
	
	/* 這邊不使用close，HookOn(掛上電話)類似disconnect，但不會Close Modem，所以不用重新Modem Open */
	inRetVal = inModem_Hook_On();
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inModem_Hook_On Error");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	if (ginDebug == VS_TRUE)
	{
		inModem_WatchStatus();
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inModem_END() END!");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	
	return(VS_SUCCESS);
}

/*
Function        :inModem_Flush
Date&Time       :2017/7/20 下午 5:51
Describe        :
*/
int inModem_Flush(void)
{
	int	inRetVal;
	char	szDebugMsg[100 + 1];
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inModem_Flush() START!");
	}
	
	inRetVal = inModem_Flush_Rx();
	if (inRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_ModemFlushRxData Error: 0x%04x", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inModem_Flush() END!");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return(VS_SUCCESS);
}

/*
Function        :inModem_DeInitial
Date&Time       :2017/7/20 下午 5:56
Describe        :
*/
int inModem_DeInitial(void)
{
        unsigned short  inRetval;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inModem_DeInitial() START!");
	}
	
	inRetval = inModem_Close();
	if (inRetval != VS_SUCCESS)
	{
		return (inRetval);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inModem_DeInitial() END!");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return(VS_SUCCESS);
}

/*
Function        :inModemSetCommParam
Date&Time       :2016/6/2 下午 2:51
Describe        :
 * [ IN ] ulBaudRate
 * 	Baud rate in bps (bits per second).
 * 	= 115200 : 115200 bps
 * 	= 57600 : 57600 bps
 * 	= 38400 : 38400 bps
 * 	= 19200 : 19200 bps
 * 	= 9600 : 9600 bps
 * [ IN ] bParity
 *	Parity mode.
 * 	= ‘E’ : Even parity
 * 	= ‘O’ : Odd parity
 * 	= ‘N’ : None parity
 * [ IN ] bDataBits
 *	Number of data bits for each data byte.
 *	= 7 : 7 data bits
 *	= 8 : 8 data bits
 * [ IN ] bStopBits
 *	Number of stop bits for each data byte.
 *	= 1 : 1 stop bit
 *	= 2 : 2 stop bits
*/
int inModem_SetCommParam(unsigned long ulBaudRate, unsigned char uszParity, unsigned char uszDataBits, unsigned char uszStopBits)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inModemSetCommParam() START!");
	}
	
	usRetVal = CTOS_ModemSetCommParam(ulBaudRate, uszParity, uszDataBits, uszStopBits);
	
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_ModemFlushRxData Error: 0x%04x", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
	}
		
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inModemSetCommParam() END!");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inModem_SetConfig
Date&Time       :2016/6/2 下午 2:51
Describe        :
*/
int inModem_SetConfig(unsigned char uszTag, unsigned short usValue)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inModemSetCommParam() START!");
	}
	usRetVal = CTOS_ModemSetConfig(uszTag, usValue);
	
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_ModemSetConfig Error: 0x%04x", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		vdUtility_SYSFIN_LogMessage(AT, "CTOS_ModemSetConfig Error: 0x%04x", usRetVal);
	}
		
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inModemSetCommParam() END!");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inModem_SetCarrier_Present_Time
Date&Time       :2016/6/2 下午 2:51
Describe        :
*/
int inModem_SetCarrier_Present_Time(unsigned short usValue)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inModem_SetCarrier_Present_Time() START!");
	}
	usRetVal = CTOS_ModemSetConfig(d_M_CONFIG_CARRIER_PRESENT_TIME, usValue);
	
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_ModemSetConfig Error: 0x%04x", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
	}
		
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inModem_SetCarrier_Present_Time() END!");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inModem_SetMin_OnHook_Duration
Date&Time       :2016/6/2 下午 3:23
Describe        :
*/
int inModem_SetMin_OnHook_Duration(unsigned short usValue)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inModem_SetMin_OnHook_Duration() START!");
	}
	usRetVal = CTOS_ModemSetConfig(d_M_CONFIG_MIN_ONHOOK_DURATION, usValue);
	
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_ModemSetConfig Error: 0x%04x", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
	}
		
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inModem_SetMin_OnHook_Duration() END!");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inModem_SetPredial_Delay_Time
Date&Time       :2016/6/2 下午 4:24
Describe        :
*/
int inModem_SetPredial_Delay_Time(unsigned short usValue)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inModem_SetPredial_Delay_Time() START!");
	}
	usRetVal = CTOS_ModemSetConfig(d_M_CONFIG_PREDIAL_DELAY_TIME, usValue);
	
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_ModemSetConfig Error: 0x%04x", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
	}
		
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inModem_SetPredial_Delay_Time() END!");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inModem_Connect_TMS
Date&Time       :2017/7/21 上午 9:27
Describe        :
*/
int inModem_Connect_TMS(void)
{     
	int		inRetval;
	int		inTimeout = 0;
	char		szNumber[18 + 1];
	char		szFinalNumber[20 + 1];
	char		szDebugMsg[100 + 1];
	char		szPABXCode[2 + 1];
	char		szCarrierTimeOut[2 + 1];
	unsigned int	uiStatus = 0;
	unsigned char	uszFirst_Tel_FailBit = VS_FALSE;	/* 第一組授權電話是否失敗 */
	unsigned char	uszKey = 0;
	
        if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inModem_Connect_TMS() START!");
	}
		
	inRetval = inModem_GetStatus(&uiStatus);
	if (inRetval != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
        
        vdUtility_SYSFIN_LogMessage(AT, "Modem Status %08X", uiStatus);
	
	if (inModem_IsAlready_SDLC_Online(uiStatus) == VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Already SDLC Online");
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_SUCCESS);
	}
		
	/* 組PABX*/
	memset(szPABXCode, 0x00, sizeof(szPABXCode));
	/* PABX code */
	inGetPABXCode(szPABXCode);
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "PABX = %s", szPABXCode);
		inLogPrintf(AT, szDebugMsg);
	}
	
/*****************第一授權電話*******************************/
	/* 組第一授權電話 */
	memset(szNumber, 0x00, sizeof(szNumber));
	inGetTMSPhoneNumber(szNumber);
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Phone Number = %s", szNumber);
		inLogPrintf(AT, szDebugMsg);
	}
	
	/* 最後播出號碼(不能分開撥，我試過了) */
	memset(szFinalNumber, 0x00, sizeof(szFinalNumber));
	sprintf(szFinalNumber, "%s%s", szPABXCode, szNumber);
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Final Number = %s", szFinalNumber);
		inLogPrintf(AT, szDebugMsg);
	}


	/* 撥第一授權電話*/
        vdUtility_SYSFIN_LogMessage(AT, "第一次撥號");
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_ChineseFont("撥號中..", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
	inRetval = inModem_Dial((unsigned char*)szFinalNumber, strlen(szFinalNumber));
	if (inRetval == VS_SUCCESS)
	{
		/* 撥成功 */
	}
	else
	{
		/* 第一授權失敗 */
		uszFirst_Tel_FailBit = VS_TRUE;
	}
	
	if (ginDebug == VS_TRUE)
	{
		inModem_WatchStatus();
	}
	
	/* 第一次授權沒失敗就繼續往下做 */
	if (uszFirst_Tel_FailBit != VS_TRUE)
	{
		/* 連線timeout時間 */
		memset(szCarrierTimeOut, 0x00, sizeof(szCarrierTimeOut));
		inGetCarrierTimeOut(szCarrierTimeOut);
		inTimeout = atoi(szCarrierTimeOut);

		if (inTimeout != 0)
			inDISP_Timer_Start(_TIMER_NEXSYS_1_, inTimeout);
		/* 確定是否撥號中，非撥號狀態判斷MODEM 是否Online，不Online就跳出 */
		while (1)
		{
			/* 按取消可以切斷連線 */
			uszKey = uszKBD_Key();
			if (uszKey == _KEY_CANCEL_)
			{
				/* 掛電話 */
				inModem_END();
				/* 第一授權失敗 */
				uszFirst_Tel_FailBit = VS_TRUE;
                                vdUtility_SYSFIN_LogMessage(AT, "手動取消繼續撥號");
                                
				return (VS_ERROR);
			}
			
			/* 抓狀態 */
			inRetval = inModem_GetStatus(&uiStatus);
			if (inRetval != VS_SUCCESS)
			{
				return (VS_ERROR);
			}
			
			/* 確認Timeout */
			if (inTimeout != 0)
			{
				if (inTimerGet(_TIMER_NEXSYS_1_) ==VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "Modem Not Online_TIMEOUT");
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "Modem Not Online Status: 0x%08x", uiStatus);
						inLogPrintf(AT, szDebugMsg);

					}
					/* 第一授權失敗 */
					uszFirst_Tel_FailBit = VS_TRUE;
                                        vdUtility_SYSFIN_LogMessage(AT, "第一次撥號Modem online timeout,Status: 0x%08x", uiStatus);
                                        
					break;
				}
			}

			/* 確認是否連上線 */
			if (inModem_IsAlready_MODEM_Online(uiStatus) == VS_SUCCESS)
			{
				/* 連上線，跳出 */
				break;
			}
		}
	}
	
	if (ginDebug == VS_TRUE)
	{
		inModem_WatchStatus();
	}
	
	/* 第一次授權沒失敗就繼續往下做 */
	if (uszFirst_Tel_FailBit != VS_TRUE)
	{
		/* 等SDLC Online */
		while (1)
		{
			/* 按取消可以切斷連線 */
			uszKey = uszKBD_Key();
			if (uszKey == _KEY_CANCEL_)
			{
				/* 掛電話 */
				inModem_END();
				/* 第一授權失敗 */
				uszFirst_Tel_FailBit = VS_TRUE;
                                vdUtility_SYSFIN_LogMessage(AT, "手動取消繼續撥號");
				return (VS_ERROR);
			}
			
			/* 抓狀態 */
			inRetval = inModem_GetStatus(&uiStatus);
			if (inRetval != VS_SUCCESS)
			{
				return (VS_ERROR);
			}
			
			if (inTimeout != 0)
			{
				if (inTimerGet(_TIMER_NEXSYS_1_) ==VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "SDLC Not Online_TIMEOUT");
						if (ginDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
							sprintf(szDebugMsg, "Modem Not Online Status: 0x%08x", uiStatus);
							inLogPrintf(AT, szDebugMsg);
						}

					}
					/* 第一授權失敗 */
					uszFirst_Tel_FailBit = VS_TRUE;
                                         
					break;
				}
			}

			/* 確認是否連上線 */
			if (inModem_IsAlready_SDLC_Online(uiStatus) == VS_SUCCESS)
			{
				/* 連上線，跳出 */
				break;
			}

		}
	}
	
	if (ginDebug == VS_TRUE)
	{
		inModem_WatchStatus();
	}

/*****************最後連線結果*******************************/
	if (ginDebug == VS_TRUE)
	{
		inModem_WatchStatus();
	}
	
	if (uszFirst_Tel_FailBit != VS_TRUE)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "第一授權電話連線完成");
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "狀態: 0x%08x", uiStatus);
			inLogPrintf(AT, szDebugMsg);
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "連線失敗");
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "狀態: 0x%08x", uiStatus);
			inLogPrintf(AT, szDebugMsg);
		}
                vdUtility_SYSFIN_LogMessage(AT, "第一次撥號連線失敗 狀態: 0x%08x", uiStatus);
		inModem_END();
		
		return (VS_ERROR);
	}
	
        if (ginDebug == VS_TRUE)
	{
                inLogPrintf(AT, "inModem_Connect_TMS() END!");
		inLogPrintf(AT, "----------------------------------------");
	}
        
        return (VS_SUCCESS);
}

/*
Function        :inModem_Dial_Speed_Test
Date&Time       :
Describe        :
*/
int inModem_Dial_Speed_Test()
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inModem_Dial_Speed_Test() START!");
	}
	
	usRetVal = CTOS_ModemOpen(d_M_MODE_SDLC_FAST, d_M_HANDSHAKE_V22_ONLY, d_M_COUNTRY_TAIWAN);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_ModemSetConfig Error: 0x%04x", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
//	usRetVal = CTOS_ModemSetConfig(d_M_CONFIG_DIAL_TONE_DETECT_DURATION, 7000);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_ModemSetConfig Error: 0x%04x", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	usRetVal = CTOS_ModemDialup((unsigned char*)"022675604", strlen("022675604"));
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_ModemSetConfig Error: 0x%04x", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
	}
//	usRetVal = CTOS_ModemOpen(d_M_MODE_SDLC_FAST, d_M_HANDSHAKE_V22_ONLY, d_M_COUNTRY_TAIWAN);
//	usRetVal = CTOS_ModemSetConfig(d_M_CONFIG_DIAL_TONE_DETECT_DURATION, 3000);
//	usRetVal = CTOS_ModemDialup((unsigned char*)"022675604", strlen("022675604"));
	
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_ModemSetConfig Error: 0x%04x", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inModem_Dial_Speed_Test() END!");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_REFERRAL_DialVoiceLine
Date&Time       :
Describe        :
*/
int inFunc_REFERRAL_DialVoiceLine(char *szPhoneNum)
{
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inModem_DeInitial() START!");
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inModem_DeInitial() END!");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}
