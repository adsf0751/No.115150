#include <string.h>
#include <stdio.h>
#include <ctosapi.h>
#include <stdlib.h>
#include <libxml/parser.h>
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
#include "Sqlite.h"
#include "Function.h"
#include "FuncTable.h"
#include "Batch.h"
#include "HDT.h"
#include "CFGT.h"
#include "EDC.h"
#include "SCDT.h"
#include "HDPT.h"
#include "CDT.h"
#include "ECR.h"
#include "RS232.h"
#include "USB.h"
#include "BaseUSB.h"
#include "KMS.h"
#include "Accum.h"
#include "File.h"
#include "Utility.h"
#include "../COMM/Ethernet.h"
#include "../COMM/WiFi.h"
#include "../COMM/UDP.h"
#include "../../NCCC/NCCCsrc.h"
#include "../../NCCC/NCCCtmk.h"
#include "../../NCCC/NCCCTicketSrc.h"
#include "../../NCCC/NCCCTicketIso.h"
#include "../../NCCC/NCCCloyalty.h"
#include "../../NCCC/NCCCEWsrc.h"
#include "../../NCCC/NCCCtSAM.h"
#include "../../CTLS/CTLS.h"
#include "../../ECC/ICER/stdAfx.h"
#include "../../ECC/ECC.h"
#include "TDT.h"
#include "PIT.h"

#define max(x, y) (((x) > (y)) ? (x) : (y))
#define min(x, y) (((x) < (y)) ? (x) : (y))

extern	int			ginDebug;
extern	int			ginISODebug;
extern	int			ginDisplayDebug;
extern	int			ginECR_ResponseFd;
extern	int			ginMachineType;
extern	int			ginFindRunTime;
extern	unsigned char		guszCTLSInitiOK;
extern	NCCC_TMK_COMMAND	gsrTMKdata;
BYTE				gbBarCodeECRBit = VS_FALSE;	/* 是否為掃碼交易規格 */
BYTE				gbEIECRBit = VS_FALSE;          /* 是否為電子發票交易規格 */
BYTE				gbECR_UDP_TransBit = VS_FALSE;
ECR_TABLE			gsrECROb = {};
EI_TABLE			gsrEIOb = {};
FPG_FTC_REC			gsrFPG_FTC_Rec = {};

/*
Function        :inECR_Initial
Date&Time       :2016/6/20 下午 2:55
Describe        :開機initial
*/
int inECR_Initial(void)
{
	int	inRetVal = VS_ERROR;
	char	szTMSOK[2 + 1] = {0};
	char	szECREnable[2 + 1] = {0};
	char	szECRVersion[2 + 1] = {0};
	char	szComPort[4 + 1] = {0};
	char	szDebugMsg[100 + 1] = {0};
	char	szTemplate[20 + 1] = {0};
	
	/* 檢查是否做過【參數下載】 */
	memset(szTMSOK, 0x00, sizeof(szTMSOK));
	inGetTMSOK(szTMSOK);
	if (szTMSOK[0] != 'Y')
	{
//		DISPLAY_OBJECT	srDispMsgObj;
//		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
//		strcpy(srDispMsgObj.szDispPic1Name, _ERR_TMS_DWL_);
//		srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
//		srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
//		srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
//		strcpy(srDispMsgObj.szErrMsg1, "");
//		srDispMsgObj.inErrMsgLine1 = 0;
//		inDISP_Msg_BMP(_ERR_TMS_DWL_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
		
		return (VS_SUCCESS);
	}
	
	/* 檢查是否有支援收銀機連線 */
	memset(szECREnable, 0x00, sizeof(szECREnable));
	inGetECREnable(szECREnable);
	if (szECREnable[0] != 'Y')
	{
		return (VS_SUCCESS);
	}
	
	memset(&gsrECROb, 0x00, sizeof(ECR_TABLE));
	/* 設定 ECR 版本 */
	memset(szECRVersion, 0x00, sizeof(szECRVersion));
	inGetECRVersion(szECRVersion);
	gsrECROb.srSetting.inVersion = atoi(szECRVersion);
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "ECRVerson :%d", gsrECROb.srSetting.inVersion);
		inLogPrintf(AT, szDebugMsg);
	}
        vdUtility_SYSFIN_LogMessage(AT, "ECR Init ECRVerson :%d", gsrECROb.srSetting.inVersion);
	
	/* UDP和RS232並存，要分開寫 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetSupECR_UDP(szTemplate);
	if (memcmp(szTemplate, "Y", 1) == 0)
	{
		inUDP_ECR_Initial();
	}
	
	memset(szComPort, 0x00, sizeof(szComPort));
	inGetECRComPort(szComPort);
	if (memcmp(szComPort, _COMPORT_COM1_, strlen(_COMPORT_COM1_)) == 0	||
	    memcmp(szComPort, _COMPORT_COM2_, strlen(_COMPORT_COM2_)) == 0	||
	    memcmp(szComPort, _COMPORT_COM3_, strlen(_COMPORT_COM3_)) == 0	||
	    memcmp(szComPort, _COMPORT_COM4_, strlen(_COMPORT_COM4_)) == 0)
	{
		inRetVal = inRS232_ECR_Initial();
	}
	else if (memcmp(szComPort, _COMPORT_USB1_, strlen(_COMPORT_USB1_)) == 0)
	{
		inRetVal = inUSB_ECR_Initial();
	}
	else if (memcmp(szComPort, _COMPORT_USB2_, strlen(_COMPORT_USB2_)) == 0)
	{
		inRetVal = inBaseUSB_ECR_Initial();
	}
	else if (memcmp(szComPort, _COMPORT_WIFI_, strlen(_COMPORT_WIFI_)) == 0)
	{
		inRetVal = inWiFi_ECR_Initial();
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
		gsrECROb.srSetting.uszSettingOK = VS_TRUE;
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inECR_FlushRxBuffer
Date&Time       :2017/6/1 下午 5:54
Describe        :清Buffer用
*/
int inECR_FlushRxBuffer()
{
	int	inRetVal = VS_ERROR;
	char	szComPort[4 + 1] = {0};
	
	/* UDP和RS232並存，要分開寫 */
	inUDP_FlushRxBuffer();
	
	memset(szComPort, 0x00, sizeof(szComPort));
	inGetECRComPort(szComPort);
	if (memcmp(szComPort, _COMPORT_COM1_, strlen(_COMPORT_COM1_)) == 0	||
	    memcmp(szComPort, _COMPORT_COM2_, strlen(_COMPORT_COM2_)) == 0	||
	    memcmp(szComPort, _COMPORT_COM3_, strlen(_COMPORT_COM3_)) == 0	||
	    memcmp(szComPort, _COMPORT_COM4_, strlen(_COMPORT_COM4_)) == 0)
	{
		inRetVal = inRS232_FlushRxBuffer(gsrECROb.srSetting.uszComPort);
	}
	else if (memcmp(szComPort, _COMPORT_USB1_, strlen(_COMPORT_USB1_)) == 0)
	{
		inRetVal = inUSB_FlushRxBuffer();
	}
	else if (memcmp(szComPort, _COMPORT_USB2_, strlen(_COMPORT_USB2_)) == 0)
	{
		inRetVal = inBaseUSB_FlushRxBuffer();
	}
	else if (memcmp(szComPort, _COMPORT_WIFI_, strlen(_COMPORT_WIFI_)) == 0)
	{
		inRetVal = VS_SUCCESS;
	}
	else
	{
		inRetVal = VS_SUCCESS;
	}
	
	return (inRetVal);
}

/*
Function        :inECR_FlushTxBuffer
Date&Time       :2018/12/8 下午 3:10
Describe        :清Buffer用
*/
int inECR_FlushTxBuffer()
{
	int	inRetVal = VS_ERROR;
	char	szComPort[4 + 1];
	
	/* UDP和RS232並存，要分開寫 */
	if (gbECR_UDP_TransBit == VS_TRUE)
	{
		inRetVal = inUDP_FlushTxBuffer();
	}
	else
	{
		memset(szComPort, 0x00, sizeof(szComPort));
		inGetECRComPort(szComPort);
		if (memcmp(szComPort, _COMPORT_COM1_, strlen(_COMPORT_COM1_)) == 0	||
		    memcmp(szComPort, _COMPORT_COM2_, strlen(_COMPORT_COM2_)) == 0	||
		    memcmp(szComPort, _COMPORT_COM3_, strlen(_COMPORT_COM3_)) == 0	||
		    memcmp(szComPort, _COMPORT_COM4_, strlen(_COMPORT_COM4_)) == 0)
		{
			inRetVal = inRS232_FlushTxBuffer(gsrECROb.srSetting.uszComPort);
		}
		else if (memcmp(szComPort, _COMPORT_USB1_, strlen(_COMPORT_USB1_)) == 0)
		{
			inRetVal = inUSB_FlushTxBuffer();
		}
		else if (memcmp(szComPort, _COMPORT_USB2_, strlen(_COMPORT_USB2_)) == 0)
		{
			inRetVal = inBaseUSB_FlushTxBuffer();
		}
		else if (memcmp(szComPort, _COMPORT_WIFI_, strlen(_COMPORT_WIFI_)) == 0)
		{
			inRetVal = VS_SUCCESS;
		}
		else
		{
			inRetVal = VS_SUCCESS;
		}
	}
	
	return (inRetVal);
}

/*
Function        :inECR_Send_Check
Date&Time       :2017/11/15 下午 5:35
Describe        :
*/
int inECR_Send_Check()
{
	int	inRetVal = VS_ERROR;
	char	szComPort[4 + 1];
	
	/* UDP和RS232並存，要分開寫 */
	if (gbECR_UDP_TransBit == VS_TRUE)
	{
		inRetVal = inUDP_Data_Send_Check();
	}
	else
	{
		memset(szComPort, 0x00, sizeof(szComPort));
		inGetECRComPort(szComPort);
		if (memcmp(szComPort, _COMPORT_COM1_, strlen(_COMPORT_COM1_)) == 0	||
		    memcmp(szComPort, _COMPORT_COM2_, strlen(_COMPORT_COM2_)) == 0	||
		    memcmp(szComPort, _COMPORT_COM3_, strlen(_COMPORT_COM3_)) == 0	||
		    memcmp(szComPort, _COMPORT_COM4_, strlen(_COMPORT_COM4_)) == 0)
		{
			/* 沒設定完成，不用檢查 */
			if (gsrECROb.srSetting.uszSettingOK != VS_TRUE)
			{
				inRetVal = VS_ERROR;
			}
			else
			{
				inRetVal = inRS232_Data_Send_Check(gsrECROb.srSetting.uszComPort);
			}
		}
		else if (memcmp(szComPort, _COMPORT_USB1_, strlen(_COMPORT_USB1_)) == 0)
		{
			/* 沒設定完成，不用檢查 */
			if (gsrECROb.srSetting.uszSettingOK != VS_TRUE)
			{
				inRetVal = VS_ERROR;
			}
			else
			{
				inRetVal = inUSB_Data_Send_Check();
			}
		}
		else if (memcmp(szComPort, _COMPORT_USB2_, strlen(_COMPORT_USB2_)) == 0)
		{
			/* 沒設定完成，不用檢查 */
			if (gsrECROb.srSetting.uszSettingOK != VS_TRUE)
			{
				inRetVal = VS_ERROR;
			}
			else
			{
				inRetVal = inBaseUSB_Data_Send_Check();
			}
		}
		else if (memcmp(szComPort, _COMPORT_WIFI_, strlen(_COMPORT_WIFI_)) == 0)
		{
			inRetVal = VS_SUCCESS;
		}
		else
		{
			inRetVal = VS_ERROR;
		}
	}
	
	return (inRetVal);
}

/*
Function        :inECR_Receive_Check
Date&Time       :2017/6/1 下午 6:00
Describe        :用來確認ECR是否發動交易
*/
int inECR_Receive_Check(unsigned short *usLen)
{
	int	inRetVal = VS_ERROR;
	char	szComPort[4 + 1] = {0};
	char	szSup_UDP[2 + 1] = {0};
	
	memset(szSup_UDP, 0x00, sizeof(szSup_UDP));
	inGetSupECR_UDP(szSup_UDP);
	if (memcmp(szSup_UDP, "Y", 1) == 0)
	{
		inRetVal = inUDP_Data_Receive_Check(usLen);
		if (inRetVal == VS_SUCCESS)
		{
			gbECR_UDP_TransBit = VS_TRUE;
			return (inRetVal);
		}
	}
	
	memset(szComPort, 0x00, sizeof(szComPort));
	inGetECRComPort(szComPort);
	if (memcmp(szComPort, _COMPORT_COM1_, strlen(_COMPORT_COM1_)) == 0	||
	    memcmp(szComPort, _COMPORT_COM2_, strlen(_COMPORT_COM2_)) == 0	||
	    memcmp(szComPort, _COMPORT_COM3_, strlen(_COMPORT_COM3_)) == 0	||
	    memcmp(szComPort, _COMPORT_COM4_, strlen(_COMPORT_COM4_)) == 0)
	{
		/* 沒設定完成，不用檢查 */
		if (gsrECROb.srSetting.uszSettingOK != VS_TRUE)
		{
			inRetVal = VS_ERROR;
		}
		else
		{
			inRetVal = inRS232_Data_Receive_Check(gsrECROb.srSetting.uszComPort, usLen);
		}
	}
	else if (memcmp(szComPort, _COMPORT_USB1_, strlen(_COMPORT_USB1_)) == 0)
	{
		/* 沒設定完成，不用檢查 */
		if (gsrECROb.srSetting.uszSettingOK != VS_TRUE)
		{
			inRetVal = VS_ERROR;
		}
		else
		{
			inRetVal = inUSB_Data_Receive_Check(usLen);
		}
	}
	else if (memcmp(szComPort, _COMPORT_USB2_, strlen(_COMPORT_USB2_)) == 0)
	{
		/* 沒設定完成，不用檢查 */
		if (gsrECROb.srSetting.uszSettingOK != VS_TRUE)
		{
			inRetVal = VS_ERROR;
		}
		else
		{
			inRetVal = inBaseUSB_Data_Receive_Check(usLen);
		}
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
Function        :inECR_Receive_Transaction
Date&Time       :2017/6/1 下午 6:13
Describe        :從ECR接收金額及交易別資料
*/
int inECR_Receive_Transaction(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;
	char	szComPort[4 + 1];
        
        vdUtility_SYSFIN_LogMessage(AT, "inECR_Receive_Transaction START!");
	
	/* 不是ECR連線就跳走 */
	if (pobTran->uszECRBit != VS_TRUE)
	{
		return (VS_SUCCESS);
	}
	
	/* UDP和RS232並存，要分開寫 */
	if (gbECR_UDP_TransBit == VS_TRUE)
	{
		inRetVal = inUDP_ECR_Receive_Transaction(pobTran);
	}
	else
	{
		memset(szComPort, 0x00, sizeof(szComPort));
		inGetECRComPort(szComPort);
		if (memcmp(szComPort, _COMPORT_COM1_, strlen(_COMPORT_COM1_)) == 0	||
		    memcmp(szComPort, _COMPORT_COM2_, strlen(_COMPORT_COM2_)) == 0	||
		    memcmp(szComPort, _COMPORT_COM3_, strlen(_COMPORT_COM3_)) == 0	||
		    memcmp(szComPort, _COMPORT_COM4_, strlen(_COMPORT_COM4_)) == 0)
		{
			inRetVal = inRS232_ECR_Receive_Transaction(pobTran);
		}
		else if (memcmp(szComPort, _COMPORT_USB1_, strlen(_COMPORT_USB1_)) == 0)
		{
			inRetVal = inUSB_ECR_Receive_Transaction(pobTran);
		}
		else if (memcmp(szComPort, _COMPORT_USB2_, strlen(_COMPORT_USB2_)) == 0)
		{
			inRetVal = inBaseUSB_ECR_Receive_Transaction(pobTran);
		}
		else if (memcmp(szComPort, _COMPORT_WIFI_, strlen(_COMPORT_WIFI_)) == 0)
		{
			inRetVal = inWiFi_ECR_Receive_Transaction(pobTran);
		}
		else
		{
			inRetVal = VS_SUCCESS;
		}
	}
	
	return (inRetVal);
}

/*
Function        :inECR_Receive_Transaction_Remove_card
Date&Time       :2017/6/1 下午 6:13
Describe        :從ECR接收金額及交易別資料
*/
int inECR_Receive_Transaction_Remove_card(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;
	char	szComPort[4 + 1];
	
	/* 不是ECR連線就跳走 */
	if (pobTran->uszECRBit != VS_TRUE)
	{
		return (VS_SUCCESS);
	}
	
	/* UDP和RS232並存，要分開寫 */
	if (gbECR_UDP_TransBit == VS_TRUE)
	{
		inRetVal = inUDP_ECR_Receive_Transaction(pobTran);
	}
	else
	{
		memset(szComPort, 0x00, sizeof(szComPort));
		inGetECRComPort(szComPort);
		if (memcmp(szComPort, _COMPORT_COM1_, strlen(_COMPORT_COM1_)) == 0	||
		    memcmp(szComPort, _COMPORT_COM2_, strlen(_COMPORT_COM2_)) == 0	||
		    memcmp(szComPort, _COMPORT_COM3_, strlen(_COMPORT_COM3_)) == 0	||
		    memcmp(szComPort, _COMPORT_COM4_, strlen(_COMPORT_COM4_)) == 0)
		{
			inRetVal = inRS232_ECR_Receive_Transaction(pobTran);
		}
		else if (memcmp(szComPort, _COMPORT_USB1_, strlen(_COMPORT_USB1_)) == 0)
		{
			inRetVal = inUSB_ECR_Receive_Transaction(pobTran);
		}
		else if (memcmp(szComPort, _COMPORT_USB2_, strlen(_COMPORT_USB2_)) == 0)
		{
			inRetVal = inBaseUSB_ECR_Receive_Transaction(pobTran);
		}
		else if (memcmp(szComPort, _COMPORT_WIFI_, strlen(_COMPORT_WIFI_)) == 0)
		{
			inRetVal = inWiFi_ECR_Receive_Transaction(pobTran);
		}
		else
		{
			inRetVal = VS_SUCCESS;
		}
	}
	
	return (inRetVal);
}

/*
Function        :inECR_Receive_Second_Transaction
Date&Time       :2018/5/28 下午 4:22
Describe        :For兩段式交易使用，第二次收
*/
int inECR_Receive_Second_Transaction(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;
	char	szComPort[4 + 1];
	
	/* 不是ECR連線就跳走 */
	if (pobTran->uszECRBit != VS_TRUE)
	{
		return (VS_SUCCESS);
	}
	/* 不是兩段式交易，不用收第二次 */
	if (pobTran->uszCardInquiryFirstBit != VS_TRUE)
	{
		return (VS_SUCCESS);
	}
	
	/* UDP和RS232並存，要分開寫 */
	if (gbECR_UDP_TransBit == VS_TRUE)
	{
		inRetVal = inUDP_ECR_Receive_Transaction(pobTran);
	}
	else
	{
		memset(szComPort, 0x00, sizeof(szComPort));
		inGetECRComPort(szComPort);
		if (memcmp(szComPort, _COMPORT_COM1_, strlen(_COMPORT_COM1_)) == 0	||
		    memcmp(szComPort, _COMPORT_COM2_, strlen(_COMPORT_COM2_)) == 0	||
		    memcmp(szComPort, _COMPORT_COM3_, strlen(_COMPORT_COM3_)) == 0	||
		    memcmp(szComPort, _COMPORT_COM4_, strlen(_COMPORT_COM4_)) == 0)
		{
			inRetVal = inRS232_ECR_Receive_Transaction(pobTran);
		}
		else if (memcmp(szComPort, _COMPORT_USB1_, strlen(_COMPORT_USB1_)) == 0)
		{
			inRetVal = inUSB_ECR_Receive_Transaction(pobTran);
		}
		else if (memcmp(szComPort, _COMPORT_USB2_, strlen(_COMPORT_USB2_)) == 0)
		{
			inRetVal = inBaseUSB_ECR_Receive_Transaction(pobTran);
		}
		else if (memcmp(szComPort, _COMPORT_WIFI_, strlen(_COMPORT_WIFI_)) == 0)
		{
			inRetVal = inWiFi_ECR_Receive_Transaction(pobTran);
		}
		else
		{
			inRetVal = VS_SUCCESS;
		}
	}
	
	return (inRetVal);
}

/*
Function        :inECR_Send_Transaction_Result
Date&Time       :2017/6/1 下午 6:13
Describe        :印帳單前要送給ECR
*/
int inECR_Send_Transaction_Result(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;
	char	szFESMode[2 + 1];
        char	szCustomerIndicator[3 + 1] = {0};
        
        vdUtility_SYSFIN_LogMessage(AT, "inECR_Send_Transaction_Result START!");
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inECR_Send_Transaction_Result() START !");
	}
	
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
	}
	
	/* 不是ECR連線就跳走 */
	if (pobTran->uszECRBit != VS_TRUE)
        {
                vdUtility_SYSFIN_LogMessage(AT, "inECR_Send_Transaction_Result Not_ECR_PASS END!");
		return (VS_SUCCESS);
        }

	/* 延遲列印，目前電子發票系列ECR時才會觸發 */
        if (!pobTran->uszDelaySendBit)
        {    
                inRetVal = inECR_Send_Transaction(pobTran);
		
		if (inRetVal != VS_SUCCESS)
		{
			vdUtility_SYSFIN_LogMessage(AT, "ECR 回傳失敗");
		}
 
                /* MPAS ECR回傳成功且MP為Y，不印簽單 */
                memset(szFESMode, 0x00, sizeof(szFESMode));
                inGetNCCCFESMode(szFESMode);

                memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
                inGetCustomIndicator(szCustomerIndicator);

                if (memcmp(szFESMode, _NCCC_04_MPAS_MODE_, strlen(_NCCC_04_MPAS_MODE_)) == 0)
                {
                        /* 客製化098，電票列印簽單規則不同 */
                        if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)    ||
                            !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_))
                        {
				if (pobTran->srEWRec.uszEWTransBit == VS_TRUE)
				{
					/* 電子錢包交易沒有簽單問題 */
				}
				else
				{
					if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
					{
						/* 只動電票，預設應該是
						 * pobTran->srTRec.uszMPASReprintBit = VS_TRUE; (當筆交易用)
						 * 電票重印有新增流程
						 * VS_TRUE = 出一聯，重印也一聯
						 * VS_FALSE= 不印，重印兩聯都印 (比照VX520)
						 */
						if (inRetVal != VS_SUCCESS  ||
						    pobTran->srTRec.inCode == _TICKET_EASYCARD_REFUND_		||
						    pobTran->srTRec.inCode == _TICKET_IPASS_REFUND_		||
						    pobTran->srTRec.inCode == _TICKET_ICASH_REFUND_		||
						    pobTran->srTRec.inCode == _TICKET_EASYCARD_VOID_TOP_UP_	||
						    pobTran->srTRec.inCode == _TICKET_IPASS_VOID_TOP_UP_	||
						    pobTran->srTRec.inCode == _TICKET_ICASH_VOID_TOP_UP_)
						{
							vdUtility_SYSFIN_LogMessage(AT, "MPAS電票出簽單");
							/* 取消交易、退貨交易：EDC一律印出簽帳單 */
							pobTran->srBRec.uszMPASReprintBit = VS_TRUE;
							pobTran->srTRec.uszMPASReprintBit = VS_TRUE;
						}
						/* 其他就不印 */
						else
						{ 
							/* 當下不出，重印出簽單 */
							pobTran->srBRec.uszMPASReprintBit = VS_TRUE;
							pobTran->srTRec.uszMPASReprintBit = VS_FALSE;
						}    
					}
					else
					{
						/* 這邊要反過來判斷MP Flag N及退貨交易和取消交易一律印出簽單 */
						/* 優惠兌換不印簽單 */
						if (inRetVal != VS_SUCCESS				||
						    (pobTran->srBRec.uszMPASTransBit != VS_TRUE && pobTran->srBRec.inCode != _LOYALTY_REDEEM_)  ||
						    pobTran->srBRec.uszVOIDBit == VS_TRUE		||
						    pobTran->srBRec.inCode == _REDEEM_REFUND_		||
						    pobTran->srBRec.inCode == _INST_REFUND_		||
						    pobTran->srBRec.inCode == _CUP_REFUND_		||
						    pobTran->srBRec.inCode == _REFUND_			||
						    pobTran->srBRec.inCode == _FISC_REFUND_		||
						    pobTran->srBRec.inCode == _CUP_MAIL_ORDER_REFUND_	||
						    pobTran->inTransactionCode == _SETTLE_)			/* 結帳一定出簽單 */
						{
							vdUtility_SYSFIN_LogMessage(AT, "MPAS信用卡出簽單");
							/* 取消交易、退貨交易：EDC一律印出簽帳單 */
							pobTran->srBRec.uszMPASReprintBit = VS_TRUE;
							pobTran->srTRec.uszMPASReprintBit = VS_TRUE;
						}
						/* 其他就不印 */
						else
						{
							/* 可以重印一定可以出簽單，反之，不能重印代表不出簽單 */
							pobTran->srBRec.uszMPASReprintBit = VS_FALSE;
							pobTran->srTRec.uszMPASReprintBit = VS_FALSE;
							/* 把不能重印的結果存回Batch */
							inBATCH_Update_MPAS_Reprint_By_Sqlite(pobTran);
						}
					}
				}
                        }
                        else
                        {
				if (pobTran->srEWRec.uszEWTransBit == VS_TRUE)
				{
					
				}
				else
				{
					/* 這邊要反過來判斷MP Flag N及退貨交易和取消交易一律印出簽單 */
					/* 優惠兌換不受Table ID:MP回多少來判斷， 但是收銀機未回傳仍要印簽單 */
					if (inRetVal != VS_SUCCESS				||
					    pobTran->srBRec.uszMPASTransBit != VS_TRUE		||
					    pobTran->srBRec.uszVOIDBit == VS_TRUE		||
					    pobTran->srBRec.inCode == _REDEEM_REFUND_		||
					    pobTran->srBRec.inCode == _INST_REFUND_		||
					    pobTran->srBRec.inCode == _CUP_REFUND_		||
					    pobTran->srBRec.inCode == _REFUND_			||
					    pobTran->srBRec.inCode == _FISC_REFUND_		||
					    pobTran->srBRec.inCode == _CUP_MAIL_ORDER_REFUND_	||
					    pobTran->inTransactionCode == _SETTLE_)			/* 結帳一定出簽單 */
					{
						vdUtility_SYSFIN_LogMessage(AT, "MPAS信用卡出簽單");
						/* 取消交易、退貨交易：EDC一律印出簽帳單 */
						pobTran->srBRec.uszMPASReprintBit = VS_TRUE;
						pobTran->srTRec.uszMPASReprintBit = VS_TRUE;
					}
					/* 其他就不印 */
					else
					{
						/* 可以重印一定可以出簽單，反之，不能重印代表不出簽單 */
						pobTran->srBRec.uszMPASReprintBit = VS_FALSE;
						pobTran->srTRec.uszMPASReprintBit = VS_FALSE;
						/* 把不能重印的結果存回Batch */
						inBATCH_Update_MPAS_Reprint_By_Sqlite(pobTran);
					}
				}
                        }
                }
        }
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inECR_Send_Transaction_Result() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
        
        vdUtility_SYSFIN_LogMessage(AT, "inECR_Send_Transaction_Result END!");
	
	/* 即使回傳失敗也要印簽單 */
	return (VS_SUCCESS);
}

/*
Function        :inECR_Send_Inquiry_Result
Date&Time       :2018/5/16 下午 5:16
Describe        :回傳查詢卡號結果
*/
int inECR_Send_Inquiry_Result(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;
	
	/* 不是ECR連線就跳走 */
	if (pobTran->uszECRBit != VS_TRUE)
	{
		return (VS_SUCCESS);
	}
	/* 不是兩段式交易，不用回傳查詢*/
	if (pobTran->uszCardInquiryFirstBit != VS_TRUE)
	{
		return (VS_SUCCESS);
	}
	
	inRetVal = inECR_Send_Transaction(pobTran);
	
	return (inRetVal);
}

int inECR_Send_Transaction(TRANSACTION_OBJECT *pobTran)
{
        int	inRetVal = VS_ERROR;
	char	szComPort[4 + 1] = {0};
        
        /* 不是ECR連線就跳走，防呆 */
	if (pobTran->uszECRBit != VS_TRUE)
		return (VS_SUCCESS);
        
        /* UDP和RS232並存，要分開寫 */
	if (gbECR_UDP_TransBit == VS_TRUE)
	{
		inRetVal = inUDP_ECR_Send_Transaction_Result(pobTran);
	}
	else
	{
		memset(szComPort, 0x00, sizeof(szComPort));
		inGetECRComPort(szComPort);
		if (memcmp(szComPort, _COMPORT_COM1_, strlen(_COMPORT_COM1_)) == 0	||
		    memcmp(szComPort, _COMPORT_COM2_, strlen(_COMPORT_COM2_)) == 0	||
		    memcmp(szComPort, _COMPORT_COM3_, strlen(_COMPORT_COM3_)) == 0	||
		    memcmp(szComPort, _COMPORT_COM4_, strlen(_COMPORT_COM4_)) == 0)
		{
			inRetVal = inRS232_ECR_Send_Transaction_Result(pobTran);
		}
		else if (memcmp(szComPort, _COMPORT_USB1_, strlen(_COMPORT_USB1_)) == 0)
		{
			inRetVal = inUSB_ECR_Send_Transaction_Result(pobTran);
		}
		else if (memcmp(szComPort, _COMPORT_USB2_, strlen(_COMPORT_USB2_)) == 0)
		{
			inRetVal = inBaseUSB_ECR_Send_Transaction_Result(pobTran);
		}
		else if (memcmp(szComPort, _COMPORT_WIFI_, strlen(_COMPORT_WIFI_)) == 0)
		{
			inRetVal = inWiFi_ECR_Send_Result(pobTran);
		}
		else
		{
			inRetVal = VS_SUCCESS;
		}
	}
        
        return (inRetVal);
}

/*
Function        :inECR_SendError
Date&Time       :2017/6/1 下午 6:17
Describe        :送錯誤訊息給ECR
*/
int inECR_SendError(TRANSACTION_OBJECT * pobTran, int inErrorType)
{
	int	inRetVal = VS_ERROR;
	char	szComPort[4 + 1];
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inECR_SendError() START !");
	}
	
	/* UDP和RS232並存，要分開寫 */
	if (gbECR_UDP_TransBit == VS_TRUE)
	{
		inRetVal = inUDP_ECR_SendError(pobTran, inErrorType);
	}
	else
	{
		memset(szComPort, 0x00, sizeof(szComPort));
		inGetECRComPort(szComPort);
		if (memcmp(szComPort, _COMPORT_COM1_, strlen(_COMPORT_COM1_)) == 0	||
		    memcmp(szComPort, _COMPORT_COM2_, strlen(_COMPORT_COM2_)) == 0	||
		    memcmp(szComPort, _COMPORT_COM3_, strlen(_COMPORT_COM3_)) == 0	||
		    memcmp(szComPort, _COMPORT_COM4_, strlen(_COMPORT_COM4_)) == 0)
		{
			inRetVal = inRS232_ECR_SendError(pobTran, inErrorType);
		}
		else if (memcmp(szComPort, _COMPORT_USB1_, strlen(_COMPORT_USB1_)) == 0)
		{
			inRetVal = inUSB_ECR_SendError(pobTran, inErrorType);
		}
		else if (memcmp(szComPort, _COMPORT_USB2_, strlen(_COMPORT_USB2_)) == 0)
		{
			inRetVal = inBaseUSB_ECR_SendError(pobTran, inErrorType);
		}
		else if (memcmp(szComPort, _COMPORT_WIFI_, strlen(_COMPORT_WIFI_)) == 0)
		{
			inRetVal = inWiFi_ECR_Send_Error(pobTran, inErrorType);
		}
		else
		{
			inRetVal = VS_SUCCESS;
		}
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inECR_SendError() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :inECR_SendMirror
Date&Time       :2017/6/1 下午 6:17
Describe        :送映射訊息給ECR
*/
int inECR_SendMirror(TRANSACTION_OBJECT * pobTran, int inMirrorType)
{
	int	inRetVal = VS_ERROR;
	char	szComPort[4 + 1];
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inECR_SendMirror() START !");
	}
	
	pobTran->inMirrorMsgType = inMirrorType;
	/* UDP和RS232並存，要分開寫 */
	if (gbECR_UDP_TransBit == VS_TRUE)
	{
		inRetVal = inUDP_ECR_SendMirror(pobTran);
	}
	else
	{
		memset(szComPort, 0x00, sizeof(szComPort));
		inGetECRComPort(szComPort);
		if (memcmp(szComPort, _COMPORT_COM1_, strlen(_COMPORT_COM1_)) == 0	||
		    memcmp(szComPort, _COMPORT_COM2_, strlen(_COMPORT_COM2_)) == 0	||
		    memcmp(szComPort, _COMPORT_COM3_, strlen(_COMPORT_COM3_)) == 0	||
		    memcmp(szComPort, _COMPORT_COM4_, strlen(_COMPORT_COM4_)) == 0)
		{
			inRetVal = inRS232_ECR_SendMirror(pobTran);
		}
		else if (memcmp(szComPort, _COMPORT_USB1_, strlen(_COMPORT_USB1_)) == 0)
		{
			inRetVal = inUSB_ECR_SendMirror(pobTran);
		}
		else if (memcmp(szComPort, _COMPORT_USB2_, strlen(_COMPORT_USB2_)) == 0)
		{
			inRetVal = inBaseUSB_ECR_SendMirror(pobTran);
		}
		else if (memcmp(szComPort, _COMPORT_WIFI_, strlen(_COMPORT_WIFI_)) == 0)
		{
			inRetVal = inWiFi_ECR_Send_Mirror(pobTran);
		}
		else
		{
			inRetVal = VS_SUCCESS;
		}
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inECR_SendMirror() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :inECR_ReceiveEI
Date&Time       :2017/6/1 下午 6:13
Describe        :從ECR接收金額及交易別資料
*/
int inECR_ReceiveEI(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;
	char	szComPort[4 + 1];
	
	/* 不是ECR連線就跳走 */
	if (pobTran->uszECRBit != VS_TRUE)
	{
		return (VS_SUCCESS);
	}
	
	/* UDP和RS232並存，要分開寫 */
	if (gbECR_UDP_TransBit == VS_TRUE)
	{
		inRetVal = inUDP_ECR_ReceiveEI(pobTran);
	}
	else
	{
		memset(szComPort, 0x00, sizeof(szComPort));
		inGetECRComPort(szComPort);
		if (memcmp(szComPort, _COMPORT_COM1_, strlen(_COMPORT_COM1_)) == 0	||
		    memcmp(szComPort, _COMPORT_COM2_, strlen(_COMPORT_COM2_)) == 0	||
		    memcmp(szComPort, _COMPORT_COM3_, strlen(_COMPORT_COM3_)) == 0	||
		    memcmp(szComPort, _COMPORT_COM4_, strlen(_COMPORT_COM4_)) == 0)
		{
			inRetVal = inRS232_ECR_ReceiveEI(pobTran);
		}
		else if (memcmp(szComPort, _COMPORT_USB1_, strlen(_COMPORT_USB1_)) == 0)
		{
			inRetVal = inUSB_ECR_ReceiveEI(pobTran);
		}
		else if (memcmp(szComPort, _COMPORT_USB2_, strlen(_COMPORT_USB2_)) == 0)
		{
			inRetVal = inBaseUSB_ECR_ReceiveEI(pobTran);
		}
		else if (memcmp(szComPort, _COMPORT_WIFI_, strlen(_COMPORT_WIFI_)) == 0)
		{
			inRetVal = inWiFi_ECR_Receive_EI(pobTran);
		}
		else
		{
			inRetVal = VS_SUCCESS;
		}
	}
	
	return (inRetVal);
}

/*
Function        :inECR_SendEI
Date&Time       :2017/6/1 下午 6:17
Describe        :送錯誤訊息給ECR
*/
int inECR_SendEI(TRANSACTION_OBJECT * pobTran, int inErrorType)
{
	int	inRetVal = VS_ERROR;
	char	szComPort[4 + 1];
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inECR_SendEI() START !");
	}
	
	/* UDP和RS232並存，要分開寫 */
	if (gbECR_UDP_TransBit == VS_TRUE)
	{
		inRetVal = inUDP_ECR_SendEI(pobTran, inErrorType);
	}
	else
	{
		memset(szComPort, 0x00, sizeof(szComPort));
		inGetECRComPort(szComPort);
		if (memcmp(szComPort, _COMPORT_COM1_, strlen(_COMPORT_COM1_)) == 0	||
		    memcmp(szComPort, _COMPORT_COM2_, strlen(_COMPORT_COM2_)) == 0	||
		    memcmp(szComPort, _COMPORT_COM3_, strlen(_COMPORT_COM3_)) == 0	||
		    memcmp(szComPort, _COMPORT_COM4_, strlen(_COMPORT_COM4_)) == 0)
		{
			inRetVal = inRS232_ECR_SendEI(pobTran, inErrorType);
		}
		else if (memcmp(szComPort, _COMPORT_USB1_, strlen(_COMPORT_USB1_)) == 0)
		{
			inRetVal = inUSB_ECR_SendEI(pobTran, inErrorType);
		}
		else if (memcmp(szComPort, _COMPORT_USB2_, strlen(_COMPORT_USB2_)) == 0)
		{
			inRetVal = inBaseUSB_ECR_SendEI(pobTran, inErrorType);
		}
		else if (memcmp(szComPort, _COMPORT_WIFI_, strlen(_COMPORT_WIFI_)) == 0)
		{
			inRetVal = inWiFi_ECR_Send_EI(pobTran, inErrorType);
		}
		else
		{
			inRetVal = VS_SUCCESS;
		}
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inECR_SendEI() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :inECR_DeInitial
Date&Time       :2018/5/22 上午 10:30
Describe        :反初始化
*/
int inECR_DeInitial(void)
{
	int	inRetVal = VS_ERROR;
	char	szTMSOK[2 + 1];
	char	szECREnable[2 + 1];
	char	szComPort[4 + 1];
	char	szTemplate[20 + 1] = {0};
	
	/* 檢查是否做過【參數下載】 */
	memset(szTMSOK, 0x00, sizeof(szTMSOK));
	inGetTMSOK(szTMSOK);
	if (szTMSOK[0] != 'Y')
	{
//		DISPLAY_OBJECT	srDispMsgObj;
//		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
//		strcpy(srDispMsgObj.szDispPic1Name, _ERR_TMS_DWL_);
//		srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
//		srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
//		srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
//		strcpy(srDispMsgObj.szErrMsg1, "");
//		srDispMsgObj.inErrMsgLine1 = 0;
//		inDISP_Msg_BMP(_ERR_TMS_DWL_, _COORDINATE_Y_LINE_8_6_, _CLEAR_KEY_MSG_, _EDC_TIMEOUT_, "", 0);
		
		return (VS_SUCCESS);
	}
	
	/* 檢查是否有支援收銀機連線 */
	memset(szECREnable, 0x00, sizeof(szECREnable));
	inGetECREnable(szECREnable);
	if (szECREnable[0] != 'Y')
	{
		return (VS_SUCCESS);
	}
	
	/* UDP和RS232並存，要分開寫 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetSupECR_UDP(szTemplate);
	if (memcmp(szTemplate, "Y", 1) == 0)
	{
		inRetVal = inUDP_ECR_DeInitial();
	}
	
	memset(szComPort, 0x00, sizeof(szComPort));
	inGetECRComPort(szComPort);
	if (memcmp(szComPort, _COMPORT_COM1_, strlen(_COMPORT_COM1_)) == 0	||
	    memcmp(szComPort, _COMPORT_COM2_, strlen(_COMPORT_COM2_)) == 0	||
	    memcmp(szComPort, _COMPORT_COM3_, strlen(_COMPORT_COM3_)) == 0	||
	    memcmp(szComPort, _COMPORT_COM4_, strlen(_COMPORT_COM4_)) == 0)
	{
		inRetVal = inRS232_ECR_DeInitial();
	}
	else if (memcmp(szComPort, _COMPORT_USB1_, strlen(_COMPORT_USB1_)) == 0)
	{
		inRetVal = inUSB_ECR_DeInitial();
	}
	else if (memcmp(szComPort, _COMPORT_USB2_, strlen(_COMPORT_USB2_)) == 0)
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
		gsrECROb.srSetting.uszSettingOK = VS_FALSE;
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inECR_Data_Send
Date&Time       :2017/11/15 下午 5:43
Describe        :分流
*/
int inECR_Data_Send(unsigned char *uszSendBuff, unsigned short usSendSize)
{
	int	inRetVal = VS_ERROR;
	char	szComPort[4 + 1];
	
	/* UDP和RS232並存，要分開寫 */
	if (gbECR_UDP_TransBit == VS_TRUE)
	{
		inRetVal = inUDP_Data_Send(uszSendBuff, usSendSize);
	}
	else
	{
		memset(szComPort, 0x00, sizeof(szComPort));
		inGetECRComPort(szComPort);
		if (memcmp(szComPort, _COMPORT_COM1_, strlen(_COMPORT_COM1_)) == 0	||
		    memcmp(szComPort, _COMPORT_COM2_, strlen(_COMPORT_COM2_)) == 0	||
		    memcmp(szComPort, _COMPORT_COM3_, strlen(_COMPORT_COM3_)) == 0	||
		    memcmp(szComPort, _COMPORT_COM4_, strlen(_COMPORT_COM4_)) == 0)
		{
			inRetVal = inRS232_Data_Send(gsrECROb.srSetting.uszComPort, uszSendBuff, usSendSize);
		}
		else if (memcmp(szComPort, _COMPORT_USB1_, strlen(_COMPORT_USB1_)) == 0)
		{
			inRetVal = inUSB_Data_Send(uszSendBuff, usSendSize);
		}
		else if (memcmp(szComPort, _COMPORT_USB2_, strlen(_COMPORT_USB2_)) == 0)
		{
			inRetVal = inBaseUSB_Data_Send(uszSendBuff, usSendSize);
		}
		else if (memcmp(szComPort, _COMPORT_WIFI_, strlen(_COMPORT_WIFI_)) == 0)
		{
			inRetVal = inETHERNET_Send_Data_By_Native(ginECR_ResponseFd, uszSendBuff, usSendSize);
		}
		else
		{
			inRetVal = VS_SUCCESS;
		}
	}
	
	return (inRetVal);
}

/*
Function        :inECR_Data_Receive
Date&Time       :2017/11/15 下午 3:51
Describe        :底層function分流
*/
int inECR_Data_Receive(unsigned char *uszReceBuff, unsigned short *usReceSize)
{
	int	inRetVal = VS_ERROR;
	char	szComPort[4 + 1];
	
	/* UDP和RS232並存，要分開寫 */
	if (gbECR_UDP_TransBit == VS_TRUE)
	{
		inRetVal = inUDP_Data_Receive(uszReceBuff, usReceSize);
	}
	else
	{
		memset(szComPort, 0x00, sizeof(szComPort));
		inGetECRComPort(szComPort);
		if (memcmp(szComPort, _COMPORT_COM1_, strlen(_COMPORT_COM1_)) == 0	||
		    memcmp(szComPort, _COMPORT_COM2_, strlen(_COMPORT_COM2_)) == 0	||
		    memcmp(szComPort, _COMPORT_COM3_, strlen(_COMPORT_COM3_)) == 0	||
		    memcmp(szComPort, _COMPORT_COM4_, strlen(_COMPORT_COM4_)) == 0)
		{
			inRetVal = inRS232_Data_Receive(gsrECROb.srSetting.uszComPort, uszReceBuff, usReceSize);
		}
		else if (memcmp(szComPort, _COMPORT_USB1_, strlen(_COMPORT_USB1_)) == 0)
		{
			inRetVal = inUSB_Data_Receive(uszReceBuff, usReceSize);
		}
		else if (memcmp(szComPort, _COMPORT_USB2_, strlen(_COMPORT_USB2_)) == 0)
		{
			inRetVal = inBaseUSB_Data_Receive(uszReceBuff, usReceSize);
		}
		else if (memcmp(szComPort, _COMPORT_WIFI_, strlen(_COMPORT_WIFI_)) == 0)
		{
			inRetVal = inETHERNET_Receive_Data_By_Native(ginECR_ResponseFd, uszReceBuff, usReceSize);
		}
		else
		{
			inRetVal = VS_SUCCESS;
		}
	}
	
	return (inRetVal);
}

/*
Function        :inECR_NCCC_144_To_400
Date&Time       :2018/7/26 下午 2:23
Describe        :144轉換400
*/
int inECR_NCCC_144_To_400(ECR_TABLE* srECROb, unsigned char* uszReceiveBuffer)
{
	int	inRetVal = VS_ERROR;
	char	szComPort[4 + 1] = {0};
	
	/* UDP和RS232並存，要分開寫 */
	if (gbECR_UDP_TransBit == VS_TRUE)
	{
		inRetVal = inUDP_ECR_NCCC_144_To_400(srECROb, uszReceiveBuffer);
	}
	else
	{
		memset(szComPort, 0x00, sizeof(szComPort));
		inGetECRComPort(szComPort);
		if (memcmp(szComPort, _COMPORT_COM1_, strlen(_COMPORT_COM1_)) == 0	||
		    memcmp(szComPort, _COMPORT_COM2_, strlen(_COMPORT_COM2_)) == 0	||
		    memcmp(szComPort, _COMPORT_COM3_, strlen(_COMPORT_COM3_)) == 0	||
		    memcmp(szComPort, _COMPORT_COM4_, strlen(_COMPORT_COM4_)) == 0)
		{
			inRetVal = inRS232_ECR_NCCC_7E1_To_8N1_Cheat(srECROb, uszReceiveBuffer);
		}
		else if (memcmp(szComPort, _COMPORT_USB1_, strlen(_COMPORT_USB1_)) == 0)
		{
			inRetVal = inUSB_ECR_NCCC_144_To_400(srECROb, uszReceiveBuffer);
		}
		else if (memcmp(szComPort, _COMPORT_USB2_, strlen(_COMPORT_USB2_)) == 0)
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
	}
	
	/* (需求單-110175)-端末設備配合實務優化流程需求 by Russell 2021/12/20 下午 4:05 */
	if (inRetVal == VS_SUCCESS)
	{
		unsigned long	ulFileHandle = 0;
		
		inFILE_Create(&ulFileHandle, (unsigned char*)_144_To_400_CHANGED_FILE_NAME_);
	}
		
	return (inRetVal);
}

/*
Function        :inECR_Send_ACKorNAK
Date&Time       :2017/11/15 下午 6:11
Describe        :送ACK OR NAK
*/
int inECR_Send_ACKorNAK (ECR_TABLE * srECROb, int inAckorNak)
{
	char		szDebugMsg[100 + 1] = {0};
	unsigned char	uszSendBuffer[2 + 1];
	unsigned short	usRetVal;
    	
	/* 清send Buffer */
	inECR_FlushTxBuffer();
	
	memset(uszSendBuffer, 0x00, sizeof(uszSendBuffer));
	
	inDISP_Timer_Start(_TIMER_NEXSYS_1_, 2);
	/* 檢查port是否已經準備好要送資料 */
        while (1)
	{
		/* 等TxReady*/
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			if (ginDisplayDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "ECR Send Timeout");
				inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_2_, VS_FALSE);
			}
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "ECR Send Timeout");
			}
			return (VS_TIMEOUT);
		}
		
		if (inECR_Send_Check() == VS_SUCCESS)
		{
			break;
		}
		else
		{
			if (ginDisplayDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "ECR Send Check Not OK");
				inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_2_, VS_FALSE);
			}
		}
	};
	
	if (inAckorNak == _ACK_)
	{	    
		/* 成功，回傳ACK */
		uszSendBuffer[0] = _ACK_;
		uszSendBuffer[1] = _ACK_;
		
		usRetVal = inECR_Data_Send(uszSendBuffer, 2);
		
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
		
		usRetVal = inECR_Data_Send(uszSendBuffer, 2);
		
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

/*
Function        :inECR_Receive_ACKandNAK
Date&Time       :2017/11/15 下午 5:50
Describe        :
*/
int inECR_Receive_ACKandNAK(ECR_TABLE * srECROb)
{
	int		i = 0;
	int		inRetVal = VS_SUCCESS;
	int		inACKTimeout = 0;
	char		szDebugMsg[(_ECR_BUFF_SIZE_ * 2) + 1] = {0};
	char		szCustomerIndicator[3 + 1] = {0};
	unsigned char	uszBuf[_ECR_BUFF_SIZE_] = {0};
	unsigned short	usTwoSize = 2;
	unsigned short	usReceiveLen = 0;
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_082_IKEA_, _CUSTOMER_INDICATOR_SIZE_)	||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_))
	{
		if (srECROb->srSetting.inVersion == atoi(_ECR_RS232_VERSION_082_IKEA_) ||
		    srECROb->srSetting.inUDPVersion == atoi(_ECR_UDP_VERSION_082_IKEA_))
		{
			inACKTimeout = _ECR_RECEIVE_ACK_TIMEOUT_144_;
		}
		else
		{
			/* 設定Timeout */
			if (gbBarCodeECRBit == VS_TRUE)
			{
				inACKTimeout = _ECR_RECEIVE_ACK_TIMEOUT_LONG_;
			}
			else
			{
				inACKTimeout = _ECR_RECEIVE_ACK_TIMEOUT_NORMAL_;
			}
		}
	}
	else
	{
		/* 設定Timeout */
		if (gbBarCodeECRBit == VS_TRUE)
		{
			inACKTimeout = _ECR_RECEIVE_ACK_TIMEOUT_LONG_;
		}
		else
		{
			inACKTimeout = _ECR_RECEIVE_ACK_TIMEOUT_NORMAL_;
		}
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "ACK Receive Timeout:%d(10ms)", inACKTimeout);
	}
	inRetVal = inDISP_Timer_Start_MicroSecond(_TIMER_NEXSYS_2_, inACKTimeout);
	
	/* 客製化005可以只收一個ACK */
	if (1)
	{
		while (1)
		{
			memset(uszBuf, 0x00, sizeof(uszBuf));
			/* 當COMPort中有東西就開始分析 */
			while (usReceiveLen == 0)
			{
				inECR_Receive_Check(&usReceiveLen);

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
				if (inTimerGet(_TIMER_NEXSYS_2_) == VS_SUCCESS)
				{
					return (VS_TIMEOUT);
				}

				usTwoSize = usReceiveLen;
				inRetVal = inECR_Data_Receive(&uszBuf[0], &usTwoSize);

				if (inRetVal == VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
						inFunc_BCD_to_ASCII(szDebugMsg, uszBuf, usTwoSize);
						inLogPrintf(AT, szDebugMsg);
					}
					
					i = 0;
					do
					{
						if (usReceiveLen == 0)
						{
							break;
						}
						
						/* buffer讀出1個byte，長度減1 */
						usReceiveLen -= 1;

						/* 判斷收到資料是否為ACK */	
						if (uszBuf[i] == _ACK_)
						{ 
							if (ginDebug == VS_TRUE)
							{
								inLogPrintf(AT, "Receive ACK!");
							}

							return (_ACK_);				
						}
						/* 判斷收到資料是否為NAK */
						else if (uszBuf[i] == _NAK_)
						{	
							if (ginDebug == VS_TRUE)
							{
								inLogPrintf(AT, "Receive NAK!");
							}

							return (_NAK_);
						}
						else 
						{
							i++;
							if (ginDebug == VS_TRUE)
							{
								inLogPrintf(AT, "Receive Not Ack Neither NAK!");
							}
							continue;
						}
					}
					while(1);

				}/*  */

			}/* while (usReceiveLen > 0) (有收到資料) */

		}/* while(1)...... */
	}
	else
	{
		while (1)
		{
			memset(uszBuf, 0x00, sizeof(uszBuf));
			/* 當COMPort中有東西就開始分析 */
			while (usReceiveLen == 0)
			{
				inECR_Receive_Check(&usReceiveLen);

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
				if (inTimerGet(_TIMER_NEXSYS_2_) == VS_SUCCESS)
				{
					return (VS_TIMEOUT);
				}

				/* 這邊一次只收1個byte */
				usTwoSize = 1;
				uszBuf[0] = uszBuf[1];
				inRetVal = inECR_Data_Receive(&uszBuf[1], &usTwoSize);

				if (inRetVal == VS_SUCCESS)
				{
					/* buffer讀出1個byte，長度減1 */
					usReceiveLen -= 1;

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
							inFunc_BCD_to_ASCII(szDebugMsg, uszBuf, 2);
							inLogPrintf(AT, szDebugMsg);
						}

						continue;
					}

				}/*  */

			}/* while (usReceiveLen > 0) (有收到資料) */

		}/* while(1)...... */
	}
		
}

/*
Function        :inECR_Print_Receive_ISODeBug
Date&Time       :2016/11/1 上午 11:06
Describe        :印收到的ISODeBug
*/
int inECR_Print_Receive_ISODeBug(char *szDataBuffer, unsigned short usReceiveBufferSize, int inDataSize)
{
	int			i;
	int			inPrintPosition, inLeftTag, inISOPositionInLine, inISOPosition;	/* ISODebug使用 */
	char			szDebugMsg[600 + 1];						/* 會用來印400ECR，所以放大一點 */
	char			szTemplate[100 + 1];
	unsigned char		uszBuffer1[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	BufferHandle		srBhandle1;
	FONT_ATTRIB		srFont_Attrib1;
	
	/* ISO example
	 * ===============(400)(標準收_400_BYTE)
	      1234567890 1234567890 1234567890
	  001 I      001 N  000001
	  031              00000000 5100
	  061
	  091
	  121
	  151
	  181
	  211
	  241
	  271
	  301
	  331
	  361
	  391
	==================(標準結束_400_BYTE)
	 */
	if (ginISODebug == VS_TRUE)
	{
		/* 是否有列印功能 */
		if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
		{
			
		}
		else
		{
			/* 初始化 */
			inPRINT_Buffer_Initial(uszBuffer1, _BUFFER_MAX_LINE_, &srFont_Attrib1, &srBhandle1);

			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));

			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetECRVersion(szTemplate);
			if (memcmp(szTemplate, "0", 1) == 0)
			{
				sprintf(szDebugMsg, "===============(%03d)(標準收_%d_BYTE)", usReceiveBufferSize, inDataSize);
			}
			inPRINT_Buffer_PutIn(szDebugMsg, _PRT_ISO_, uszBuffer1, &srFont_Attrib1, &srBhandle1, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("      1234567890 1234567890 1234567890", _PRT_ISO_, uszBuffer1, &srFont_Attrib1, &srBhandle1, _LAST_ENTRY_, _PRINT_LEFT_);

			inISOPositionInLine = 0;			/* 印到該行第幾個Byte */
			inLeftTag = 1;					/* 最左邊的欄位，從1開始，每次加30 */

			/* 最左邊的欄位 */
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "  %03d  ", inLeftTag);

			inPrintPosition = 6;				/* 在紙上要從哪裡開始印，因為前面要空LeftTag的位置 */
			inISOPosition = 0;
			do
			{	
				if ((szDataBuffer[inISOPosition] >= 0x00 && szDataBuffer[inISOPosition] <= 31) || szDataBuffer[inISOPosition] == 127)
				{
					szDebugMsg[inPrintPosition ++] = '(';
					sprintf(&szDebugMsg[inPrintPosition], "%02X", szDataBuffer[inISOPosition]);
					inPrintPosition += 2;
					szDebugMsg[inPrintPosition ++] = ')';
				}
				else
				{
					szDebugMsg[inPrintPosition ++] = szDataBuffer[inISOPosition];
				}


				inISOPositionInLine ++;			/* 電文的第幾個byte */

				/* 每10個一個空格 */
				if ((inISOPositionInLine % 10) == 0)
					szDebugMsg[inPrintPosition ++] = 0x20;

				/* 每30個印出來，歸0並換行 */
				if ((inISOPositionInLine % 30) == 0)
				{
					inPRINT_Buffer_PutIn(szDebugMsg, _PRT_ISO_, uszBuffer1, &srFont_Attrib1, &srBhandle1, _LAST_ENTRY_, _PRINT_LEFT_);
					inISOPositionInLine = 0;
					inLeftTag += 30;

					/* 最左邊的欄位 */
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "  %03d  ", inLeftTag);
					inPrintPosition = 6;
				}

				/* 判斷是否超過 */
				if ((inISOPosition + 1) >= usReceiveBufferSize)
				{
					if (inISOPositionInLine != 0)
					{
						/* 把最後那一行印出來 */
						inPRINT_Buffer_PutIn(szDebugMsg, _PRT_ISO_, uszBuffer1, &srFont_Attrib1, &srBhandle1, _LAST_ENTRY_, _PRINT_LEFT_);
					}
					else
					{
						/* 剛好30的倍數，不用另外印 */
					}
					break;
				}
				else
				{
					inISOPosition++;
				}
			}while(1);

			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));

			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetECRVersion(szTemplate);
			if (memcmp(szTemplate, "0", 1) == 0)
			{
				sprintf(szDebugMsg, "==================(標準結束_%03d_BYTE)", inDataSize);
			}
			inPRINT_Buffer_PutIn(szDebugMsg, _PRT_ISO_, uszBuffer1, &srFont_Attrib1, &srBhandle1, _LAST_ENTRY_, _PRINT_LEFT_);

			for (i = 0; i < 8; i++)
			{
				inPRINT_Buffer_PutIn("", _PRT_ISO_, uszBuffer1, &srFont_Attrib1, &srBhandle1, _LAST_ENTRY_, _PRINT_LEFT_);
			}

			inPRINT_Buffer_OutPut(uszBuffer1, &srBhandle1);
		}
		
	}
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetECRVersion(szTemplate);
		if (memcmp(szTemplate, "0", 1) == 0)
		{
			sprintf(szDebugMsg, "===============(%03d)(標準收_%d_BYTE)", usReceiveBufferSize, inDataSize);
		}
		inLogPrintf(AT, szDebugMsg);
		inLogPrintf(AT, "      1234567890 1234567890 1234567890");
		
		inISOPositionInLine = 0;			/* 印到該行第幾個Byte */
		inLeftTag = 1;					/* 最左邊的欄位，從1開始，每次加30 */
		
		/* 最左邊的欄位 */
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "  %03d  ", inLeftTag);
		
		inPrintPosition = 6;				/* 在紙上要從哪裡開始印，因為前面要空LeftTag的位置 */
		inISOPosition = 0;
		do
		{	
			if ((szDataBuffer[inISOPosition] >= 0x00 && szDataBuffer[inISOPosition] <= 31) || szDataBuffer[inISOPosition] == 127)
			{
				szDebugMsg[inPrintPosition ++] = '(';
				sprintf(&szDebugMsg[inPrintPosition], "%02X", szDataBuffer[inISOPosition]);
				inPrintPosition += 2;
				szDebugMsg[inPrintPosition ++] = ')';
			}
			else
			{
				szDebugMsg[inPrintPosition ++] = szDataBuffer[inISOPosition];
			}
			
			
			inISOPositionInLine ++;			/* 電文的第幾個byte */
                        
			/* 每10個一個空格 */
			if ((inISOPositionInLine % 10) == 0)
				szDebugMsg[inPrintPosition ++] = 0x20;
                        
			/* 每30個印出來，歸0並換行 */
			if ((inISOPositionInLine % 30) == 0)
			{
				inLogPrintf(AT, szDebugMsg);
				inISOPositionInLine = 0;
				inLeftTag += 30;
				
				/* 最左邊的欄位 */
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "  %03d  ", inLeftTag);
				inPrintPosition = 6;
			}
			
			if ((inISOPosition + 1) >= usReceiveBufferSize)
			{
				if (inISOPositionInLine != 0)
				{
					/* 把最後那一行印出來 */
					inLogPrintf(AT, szDebugMsg);
				}
				else
				{
					/* 剛好30的倍數，不用另外印 */
				}
				break;
			}
			else
			{
				inISOPosition++;
			}
		}while(1);
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetECRVersion(szTemplate);
		if (memcmp(szTemplate, "0", 1) == 0)
		{
			sprintf(szDebugMsg, "==================(標準結束_%03d_BYTE)", inDataSize);
		}
		inLogPrintf(AT, szDebugMsg);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inECR_Print_Send_ISODeBug
Date&Time       :2016/11/1 上午 11:06
Describe        :印送出的ISODeBug
*/
int inECR_Print_Send_ISODeBug(char *szDataBuffer, unsigned short usReceiveBufferSize, int inDataSize)
{
	int			i;
	int			inPrintPosition, inLeftTag, inISOPositionInLine, inISOPosition;	/* ISODebug使用 */
	char			szDebugMsg[600 + 1];						/* 會用來印400ECR，所以放大一點 */
	char			szTemplate[100 + 1];
	unsigned char		uszBuffer1[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	BufferHandle		srBhandle1;
	FONT_ATTRIB	srFont_Attrib1;
	
	/* ISO example
	 * ===============(400)(標準送_400_BYTE)
	      1234567890 1234567890 1234567890
	  001 I      001 N  000001
	  031              00000000 5100
	  061
	  091
	  121
	  151
	  181
	  211
	  241
	  271
	  301
	  331
	  361
	  391
	==================(標準結束_400_BYTE)
	 */
	if (ginISODebug == VS_TRUE)
	{
		/* 是否有列印功能 */
		if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
		{
			return (VS_SUCCESS);
		}
		else
		{
			/* 初始化 */
			inPRINT_Buffer_Initial(uszBuffer1, _BUFFER_MAX_LINE_, &srFont_Attrib1, &srBhandle1);

			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));

			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetECRVersion(szTemplate);
			if (memcmp(szTemplate, "0", 1) == 0)
			{
				sprintf(szDebugMsg, "===============(%03d)(標準送_%d_BYTE)", usReceiveBufferSize, inDataSize);
			}
			inPRINT_Buffer_PutIn(szDebugMsg, _PRT_ISO_, uszBuffer1, &srFont_Attrib1, &srBhandle1, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("      1234567890 1234567890 1234567890", _PRT_ISO_, uszBuffer1, &srFont_Attrib1, &srBhandle1, _LAST_ENTRY_, _PRINT_LEFT_);

			inISOPositionInLine = 0;			/* 印到該行第幾個Byte */
			inLeftTag = 1;					/* 最左邊的欄位，從1開始，每次加30 */

			/* 最左邊的欄位 */
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "  %03d  ", inLeftTag);

			inPrintPosition = 6;				/* 在紙上要從哪裡開始印，因為前面要空LeftTag的位置 */
			inISOPosition = 0;
			do
			{	
				szDebugMsg[inPrintPosition ++] = szDataBuffer[inISOPosition];
				inISOPositionInLine ++;			/* 電文的第幾個byte */

				/* 每10個一個空格 */
				if ((inISOPositionInLine % 10) == 0)
					szDebugMsg[inPrintPosition ++] = 0x20;

				/* 每30個印出來，歸0並換行 */
				if ((inISOPositionInLine % 30) == 0)
				{
					inPRINT_Buffer_PutIn(szDebugMsg, _PRT_ISO_, uszBuffer1, &srFont_Attrib1, &srBhandle1, _LAST_ENTRY_, _PRINT_LEFT_);
					inISOPositionInLine = 0;
					inLeftTag += 30;

					/* 最左邊的欄位 */
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "  %03d  ", inLeftTag);
					inPrintPosition = 6;
				}

				/* 判斷是否超過 */
				if ((inISOPosition + 1) >= usReceiveBufferSize)
				{
					if (inISOPositionInLine != 0)
					{
						/* 把最後那一行印出來 */
						inPRINT_Buffer_PutIn(szDebugMsg, _PRT_ISO_, uszBuffer1, &srFont_Attrib1, &srBhandle1, _LAST_ENTRY_, _PRINT_LEFT_);
					}
					else
					{
						/* 剛好30的倍數，不用另外印 */
					}
					break;
				}
				else
				{
					inISOPosition++;
				}
			}while(1); 

			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));

			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetECRVersion(szTemplate);
			if (memcmp(szTemplate, "0", 1) == 0)
			{
				sprintf(szDebugMsg, "==================(標準結束_%03d_BYTE)", inDataSize);
			}
			inPRINT_Buffer_PutIn(szDebugMsg, _PRT_ISO_, uszBuffer1, &srFont_Attrib1, &srBhandle1, _LAST_ENTRY_, _PRINT_LEFT_);

			for (i = 0; i < 8; i++)
			{
				inPRINT_Buffer_PutIn("", _PRT_ISO_, uszBuffer1, &srFont_Attrib1, &srBhandle1, _LAST_ENTRY_, _PRINT_LEFT_);
			}

			inPRINT_Buffer_OutPut(uszBuffer1, &srBhandle1);
		}
	}
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetECRVersion(szTemplate);
		if (memcmp(szTemplate, "0", 1) == 0)
		{
			sprintf(szDebugMsg, "===============(%03d)(標準送_%d_BYTE)", usReceiveBufferSize, inDataSize);
		}
		inLogPrintf(AT, szDebugMsg);
		inLogPrintf(AT, "      1234567890 1234567890 1234567890");
		
		inISOPositionInLine = 0;			/* 印到該行第幾個Byte */
		inLeftTag = 1;					/* 最左邊的欄位，從1開始，每次加30 */
		
		/* 最左邊的欄位 */
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "  %03d  ", inLeftTag);
		
		inPrintPosition = 6;				/* 在紙上要從哪裡開始印，因為前面要空LeftTag的位置 */
		inISOPosition = 0;
		do
		{	
			szDebugMsg[inPrintPosition ++] = szDataBuffer[inISOPosition];
			inISOPositionInLine ++;			/* 電文的第幾個byte */
                        
			/* 每10個一個空格 */
			if ((inISOPositionInLine % 10) == 0)
				szDebugMsg[inPrintPosition ++] = 0x20;
                        
			/* 每30個印出來，歸0並換行 */
			if ((inISOPositionInLine % 30) == 0)
			{
				inLogPrintf(AT, szDebugMsg);
				inISOPositionInLine = 0;
				inLeftTag += 30;
				
				/* 最左邊的欄位 */
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "  %03d  ", inLeftTag);
				inPrintPosition = 6;
			}
			
			if ((inISOPosition + 1) >= usReceiveBufferSize)
			{
				if (inISOPositionInLine != 0)
				{
					/* 把最後那一行印出來 */
					inLogPrintf(AT, szDebugMsg);
				}
				else
				{
					/* 剛好30的倍數，不用另外印 */
				}
				break;
			}
			else
			{
				inISOPosition++;
			}
		}while(1);
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetECRVersion(szTemplate);
		if (memcmp(szTemplate, "0", 1) == 0)
		{
			sprintf(szDebugMsg, "==================(標準結束_%03d_BYTE)", inDataSize);
		}
		inLogPrintf(AT, szDebugMsg);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inECRCardNoTruncateDecision
Date&Time       :2017/5/22 下午 4:33
Describe        :判斷ECR交易回送資料給POS機時,卡號是否要做掩飾
 *		1. VS_TRUE -> 卡號要做掩飾
		2. VS_FALSE -> 卡號不做掩飾
 *		看TMS規格應該不會有CardBin重疊的現象
*/
int inECR_CardNoTruncateDecision(TRANSACTION_OBJECT * pobTran)
{
	int		inRetVal = VS_ERROR;
	int		inTempRetVal = VS_ERROR;
	char		szFuncEnable[2 + 1];
	
	inGetECRCardNoTruncateEnable(szFuncEnable);
	/* 如果TMS 設定 回傳卡號 需要掩飾,才有必要比對活動代碼,否則不需要比對,因為已經是明碼回傳卡號 */
	if (memcmp(szFuncEnable, "Y", strlen("Y")) != 0)
	{
		inRetVal = VS_ERROR;
	}
	else
	{
		inTempRetVal = inNCCC_Func_CheckSpecialCard(pobTran, _CAMPAIGNNUMBER_01_PLAIN_PAN_);
		/* 不在參數檔內，要遮掩 */
		if (inTempRetVal != VS_SUCCESS)
		{
			inRetVal = VS_SUCCESS;
		}
		else
		{
			inRetVal = VS_ERROR;
		}
	}

	return (inRetVal);
}

/*
Function        :inECRCardNoTruncateDecision_HG
Date&Time       :2017/5/22 下午 4:33
Describe        :判斷ECR交易回送資料給POS機時,卡號是否要做掩飾
 *		1. VS_TRUE -> 卡號要做掩飾
		2. VS_FALSE -> 卡號不做掩飾
 *		看TMS規格應該不會有CardBin重疊的現象
*/
int inECR_CardNoTruncateDecision_HG(TRANSACTION_OBJECT * pobTran)
{
	int		inRetVal = VS_ERROR;
	int		i = 0;
	int		inLowBinLen = 0, inHighBinLen = 0;
	int		inPANLen = 0;
	long		lnCampaignAmount = 0;
	char		szFuncEnable[2 + 1];
	char		szLowBinRange[2 + 1];
	char		szHighBinRange[2 + 1];
	char		szTempCampaignNumber[2 + 1];
	char		szTemplate[12 + 1];
	char		szStartDate[8 + 1], szEndDate[8 + 1];
	char		szNowDate[8 + 1];
	char		szDebugMsg[100 + 1];
	unsigned char	uszInCardBin = VS_ERROR;		/* 這表示在卡號內 */
	RTC_NEXSYS	srRTC;

	
	inGetECRCardNoTruncateEnable(szFuncEnable);
	/* 如果TMS 設定 回傳卡號 需要掩飾,才有必要比對活動代碼,否則不需要比對,因為已經是明碼回傳卡號 */
	if (memcmp(szFuncEnable, "Y", strlen("Y")) != 0)
	{
		return (VS_ERROR);
	}
	else
	{
		/* CFGT沒開特殊卡別參數檔，就直接遮掩卡號 */
		memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
		inGetSpecialCardRangeEnable(szFuncEnable);
		if (memcmp(szFuncEnable, "Y", strlen("Y")) != 0)
		{
			return (VS_SUCCESS);
		}
		
		inPANLen = strlen(pobTran->srBRec.szHGPAN);
		if (inPANLen < 0)
		{
			/* debug */
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "PANLength: %d < 0", inPANLen);
				inLogPrintf(AT, szDebugMsg);
			}

			/* 顯示卡號錯誤 */
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_CARD_NO_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);

			return (VS_SUCCESS);
		}

		for (i = 0 ;; i ++)
		{
			if (inLoadSCDTRec(i) < 0)
				break;

			memset(szLowBinRange, 0x00, sizeof(szLowBinRange));
			memset(szHighBinRange, 0x00, sizeof(szHighBinRange));
			inGetLowBinRange(szLowBinRange);
			inLowBinLen = strlen(szLowBinRange);
			inGetHighBinRange(szHighBinRange);
			inHighBinLen = strlen(szHighBinRange);

			memset(szTempCampaignNumber, 0x00, sizeof(szTempCampaignNumber));
			inGetCampaignNumber(szTempCampaignNumber);
			/* 如果不是要判別的活動代碼，就跳過，這裡的設計原理一次只確認一種活動代碼(為了模組化) */
			if (memcmp(szTempCampaignNumber, "01", 2) != 0)
			{
				continue;
			}

			/* CUP卡看CardBin為9999999999就好 */
			if (memcmp(pobTran->srBRec.szHGCardLabel, _CARD_TYPE_CUP_, strlen(_CARD_TYPE_CUP_)) == 0)
			{
				/* 判斷卡號 位在哪個Bin Range(如果卡號大於該卡別最低卡號，而且小於最高卡號) */
				if ((memcmp(szLowBinRange, "9999999999", 10) >= 0) &&
				    (memcmp(szHighBinRange, "9999999999", 10) <= 0))
				{
					uszInCardBin = VS_TRUE;
					break;
				}
			}
			else
			{
				if ((memcmp(pobTran->srBRec.szHGPAN, szLowBinRange, min(inPANLen, inLowBinLen)) >= 0) &&
				    (memcmp(pobTran->srBRec.szHGPAN, szHighBinRange, min(inPANLen, inHighBinLen)) <= 0))
				{
					uszInCardBin = VS_TRUE;
					break;
				}
			}
		}

		/* SCDT內沒有該卡別，回傳要遮掩 */
		if (uszInCardBin != VS_TRUE)
		{
			return (VS_SUCCESS);
		}
		else
		{
			/* 檢查金額 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetCampaignAmount(szTemplate);
			/* 小數點下兩位不記*/
			szTemplate[10] = 0x00;
			szTemplate[11] = 0x00;
			lnCampaignAmount = atol(szTemplate);

			if (pobTran->srBRec.lnTxnAmount <= lnCampaignAmount)
			{		
				/* 取得活動起迄日 */
				/* 活動起日 */
				memset(szStartDate, 0x00, sizeof(szStartDate));
				inGetCampaignStartDate(szStartDate);
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "CampaignStartDate = %s", szStartDate);
					inLogPrintf(AT, szDebugMsg);
				}

				/* 活動迄日 */
				memset(szEndDate, 0x00, sizeof(szEndDate));
				inGetCampaignEndDate(szEndDate);
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "CampaignEndDate = %s", szEndDate);
					inLogPrintf(AT, szDebugMsg);
				}

				/* 現在日期 */
				memset(&srRTC, 0x00, sizeof(RTC_NEXSYS));
				inFunc_GetSystemDateAndTime(&srRTC);
				memset(szNowDate, 0x00, sizeof(szNowDate));
				sprintf(szNowDate, "20%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "NowDate = %s", szNowDate);
					inLogPrintf(AT, szDebugMsg);
				}

				/* 若在範圍內，回傳不用遮掩，若不在範圍內，則要回傳遮掩 */
				if (inFunc_SunDay_Sum_Check_In_Range(szNowDate, szStartDate, szEndDate) == VS_SUCCESS)
				{
					inRetVal = VS_ERROR;
				}
				else
				{
					inRetVal = VS_SUCCESS;
				}
			}
		}
	}

	return (inRetVal);
}


/*
Function        :inECR_Receive
Date&Time       :2017/11/15 下午 3:14
Describe        :接收收銀機傳來的資料
*/
int inECR_Receive(TRANSACTION_OBJECT *pobTran, ECR_TABLE* srECROb, char *szDataBuffer, int inDataSize)
{
	int		i;
	int		inRetVal;
	int		inRetry = 0;							/* 目前已重試次數 */
	int		inRecMaxRetry = _ECR_RETRYTIMES_;				/* 最大重試次數 */
	char		szDebugMsg[1000 + 1];						/* DebugMsg */
	char		szCustomerIndicator[3 + 1] = {0};
	unsigned short	usReceiveBufferSize;						/* uszReceiveBuffer的長度，*/
	unsigned short	usOneSize = 1;							/* 一次只讀一個byte */
	unsigned short	usReceiveLen = 0;
	unsigned char	uszLRC;								/* LRC的值 */
	unsigned char	uszTempBuffer[1024 + 1];					/* 從 Buffer收到的片段 */
	unsigned char	uszReceiveBuffer[_ECR_BUFF_SIZE_];				/* 包含STX 和 ETX的原始電文 */
	unsigned char	uszSTX = VS_FALSE;						/* 是否收到STX */
	unsigned char	uszETX = VS_FALSE;						/* 是否收到ETX */
	unsigned char	uszCheckSpecialLenthBit = VS_FALSE;                             /* 是否已檢查為特殊長度 */
        unsigned char	uszCheckUnyBit = VS_FALSE;					/* 是否已檢查Indicator是否為Q */
        unsigned char	uszCheckEIBit = VS_FALSE;					/* 是否為電子發票 */
        char	szTemp[10 + 1];
	
	/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/28 下午 2:42 */
	gbBarCodeECRBit = VS_FALSE;
        gbEIECRBit = VS_FALSE;

	/* 設定Timeout */
	inRetVal = inDISP_Timer_Start(_ECR_RECEIVE_TIMER_, srECROb->srSetting.inTimeout);
	
	/* 初始化放收到資料的陣列 */
	memset(uszReceiveBuffer, 0x00, sizeof(uszReceiveBuffer));
	usReceiveBufferSize = 0;
	
	/* 客製化 */
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
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
		
		/* 當Comport中有東西就開始分析 */
		while (usReceiveLen == 0)
		{
			/* 如果timeout就跳出去 */
			if (inTimerGet(_ECR_RECEIVE_TIMER_) == VS_SUCCESS)
			{
				return (VS_TIMEOUT);
			}
			
			inECR_Receive_Check(&usReceiveLen);
		}
		
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Len : %d", usReceiveLen);
			inLogPrintf(AT, szDebugMsg);
		}

		/* 如果沒收到STX就會進這隻，直到分析出STX */
		while (uszSTX == VS_FALSE && usReceiveLen > 0)
		{
			/* 如果timeout就跳出去 */
			if (inTimerGet(_ECR_RECEIVE_TIMER_) == VS_SUCCESS)
			{
				return (VS_TIMEOUT);
			}
			
			/* 這邊一次只收一個byte */
			usOneSize = 1;
			memset(uszTempBuffer, 0x00, usOneSize);
			inRetVal = inECR_Data_Receive(uszTempBuffer, &usOneSize);
			if (inRetVal == VS_SUCCESS)
			{	
				/* 避免長度過短的雜訊改到標準400，這邊只少要長度超過64才判斷是否轉換 */
				if (usReceiveLen > 64)
				{
					/* 若為為7E1協定，且收到STX之後的第一個為"I"，則自動轉為8N1 (NCCC ECR特殊規定) */
					if (inECR_NCCC_144_To_400(srECROb, uszTempBuffer) == VS_SUCCESS)
					{
						return (VS_ERROR);
					}
				}
				
				/* buffer讀出一個byte，長度減一 */
				usReceiveLen -= usOneSize;

				if (uszTempBuffer[0] == _STX_)
				{
					/* 收到STX */
					uszSTX = VS_TRUE;
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "Receive STX!");
					}
					if (ginDisplayDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "Receive STX");
						inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_2_, VS_FALSE);
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
						memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
						inFunc_BCD_to_ASCII(szDebugMsg, uszTempBuffer, usOneSize);
						inLogPrintf(AT, szDebugMsg);
					}
					if (ginDisplayDebug == VS_TRUE)
					{
						inDISP_LOGDisplay("Not Receive STX", _FONTSIZE_16X22_, _LINE_16_3_, VS_FALSE);
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						inFunc_BCD_to_ASCII(szDebugMsg, uszTempBuffer, usOneSize);
						inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_4_, VS_FALSE);
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
			if (inTimerGet(_ECR_RECEIVE_TIMER_) == VS_SUCCESS)
			{
				return (VS_TIMEOUT);
			}
			
			usOneSize = usReceiveLen;
			memset(uszTempBuffer, 0x00, usOneSize);
			inRetVal = inECR_Data_Receive(uszTempBuffer, &usOneSize);

			if (inRetVal == VS_SUCCESS)
			{
				/* buffer讀出一個byte，長度減一 */
				memcpy(&uszReceiveBuffer[usReceiveBufferSize], uszTempBuffer, usOneSize);
				usReceiveLen -= usOneSize;
				usReceiveBufferSize += usOneSize;
				                                
                                if (usReceiveBufferSize >= 2)
				{ 
                                        /* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/8/13 下午 4:10 */
                                        if (uszCheckUnyBit != VS_TRUE && uszCheckSpecialLenthBit != VS_TRUE)
                                        {
                                                uszCheckUnyBit = VS_TRUE;
                                                
                                                if (uszReceiveBuffer[1] == 'Q')
                                                {
                                                        uszCheckSpecialLenthBit = VS_TRUE;
                                                        gbBarCodeECRBit = VS_TRUE;
                                                        inDataSize = _ECR_8N1_1000_CUPQRCODE_Size_;
                                                }
                                        }
                                }

                                if (usReceiveBufferSize >= 11)
				{
                                        if (uszCheckEIBit != VS_TRUE && uszCheckSpecialLenthBit != VS_TRUE)
                                        {
                                                uszCheckEIBit = VS_TRUE;

                                                /* 電子發票交易別 "63" */
                                                memset(szTemp, 0x00, sizeof(szTemp));
                                                memcpy(&szTemp[0], &uszReceiveBuffer[9], 2);
                                                if (!memcmp(szTemp, _ECR_8N1_EI_TRANSACTION_, 2))
                                                {
                                                        uszCheckSpecialLenthBit = VS_TRUE;
                                                        gbEIECRBit = VS_TRUE;
                                                        inDataSize = _ECR_8N1_2044_EI_Receive_Size_;
                                                }
                                        }
				}
				
				/* 當已收到401byte 確認下個byte是不是ETX */
				if (usReceiveBufferSize >= (inDataSize + 3) - 1)
				{
					if (uszReceiveBuffer[(inDataSize + 3) - 2] == _ETX_)
					{	
						/* 是ETX */
						uszETX = VS_TRUE;
						if (ginDebug == VS_TRUE)
						{
							inLogPrintf(AT, "Receive ETX!");
						}
						if (ginDisplayDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
							sprintf(szDebugMsg, "Receive ETX");
							inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_2_, VS_FALSE);
						}
						
						if (usReceiveBufferSize < (inDataSize + 3))
						{
							/* 再收LRC */
							usOneSize = 1;
							memset(uszTempBuffer, 0x00, usOneSize);
							inRetVal = inECR_Data_Receive(&uszTempBuffer[0], &usOneSize);
							if (inRetVal == VS_SUCCESS)
							{
								memcpy(&uszReceiveBuffer[usReceiveBufferSize], &uszTempBuffer[0], usOneSize);
								usReceiveLen -= usOneSize;
								usReceiveBufferSize += usOneSize;
							}
							else
							{
								/* 沒收到LRC 資料長度有誤*/
							}
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
						if (ginDisplayDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
							sprintf(szDebugMsg, "Not Receive ETX");
							inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_3_, VS_TRUE);
						}
					}	
				}
			}

		}

		/* 有收到STX、ETX而且收到長度也對，就開始算LRC */
		if (uszSTX == VS_TRUE && uszETX == VS_TRUE)
		{
			/* 資料長度錯誤，清Buffer，送NAK，重收 */
			if (usReceiveBufferSize < inDataSize + 3)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "DataLength error!:%d Retry: %d", usReceiveBufferSize, inRetry);
					inLogPrintf(AT, szDebugMsg);
				}
				if (ginDisplayDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "DataLength error!:%d Retry: %d", usReceiveBufferSize, inRetry);
					inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_3_, VS_TRUE);
				}
				
				if (srECROb->srSetting.inVersion == atoi(_ECR_RS232_VERSION_082_IKEA_))
				{
					inECR_SendError(pobTran, VS_TIMEOUT);
				}
				else
				{
					/* 回傳NAK */
					inECR_Send_ACKorNAK(srECROb, _NAK_);
				}
				/* retry次數+ 1 */
				inRetry ++;
				
				/* 初始化資料 */
				memset(uszReceiveBuffer, 0x00, sizeof(uszReceiveBuffer));
				usReceiveBufferSize = 0;
				uszSTX = VS_FALSE;
				uszETX = VS_FALSE;
				
				continue;
			}
			else if (usReceiveBufferSize > inDataSize + 3)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "DataLength :%d", usReceiveBufferSize);
					inLogPrintf(AT, szDebugMsg);
				}
				if (ginDisplayDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "DataLength :%d", usReceiveBufferSize);
					inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_3_, VS_TRUE);
				}
			}
			
			/* 計算收到DATA的LRC （Stx Not Include）*/
			for (i = 1; i < ((inDataSize + 3) - 1); i++)
			{
				uszLRC ^= uszReceiveBuffer[i];
			}
			
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "Calculate LRC: %02x", uszLRC);
			}
			
			if (uszReceiveBuffer[((inDataSize + 3) - 1)] == uszLRC)
			{
				if (srECROb->srOptionalSetting.uszNotSendAck == VS_TRUE)
				{
					
				}
				else
				{
					/* 比對收到的LRC是否正確，若正確回傳ACK */
					inECR_Send_ACKorNAK(srECROb, _ACK_);
				}
				
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
					sprintf(&szDebugMsg[0], "%X", uszReceiveBuffer[((inDataSize + 3) - 1)]);
					inLogPrintf(AT, szDebugMsg);
				}
				if (ginDisplayDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "LRC error! Retry: %d", inRetry);
					inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_3_, VS_TRUE);
				}
				
				/* 若錯誤回傳NAK */
				inECR_Send_ACKorNAK(srECROb, _NAK_);
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
	/* [114197] 中心收單作業ESG永續發展優化 
	*  [260610] Rs232 電子簽單不支援 Uny交易，過濾掉 ECR Indicator ='Q'的交易，gbEIECRBit 這部分不確定是否包含 */
	if (gbBarCodeECRBit == VS_TRUE || gbEIECRBit == VS_TRUE)
	{	
		/* 跳過詢問持卡人是否同意接收數位化簽帳單程式 */
		pobTran->uszEsgOptrionBit = VS_TRUE;
		/* ESG需求不包含 掃碼交易規格/電子發票交易規格 */
		pobTran->szAgreeEsgBill[0] = 'N';
		pobTran->szEsgSendCR[0] = 'N';
		
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inECR_Receive_UDP
Date&Time       :2021/12/6 下午 5:21
Describe        :接收收銀機傳來的資料
*/
int inECR_Receive_UDP(TRANSACTION_OBJECT *pobTran, ECR_TABLE* srECROb, char *szDataBuffer, int inDataSize)
{
	int		i;
	int		inRetVal;
	int		inRetry = 0;							/* 目前已重試次數 */
	int		inRecMaxRetry = _ECR_RETRYTIMES_;				/* 最大重試次數 */
	int		inReceivePacket = 0;						/* 收到的封包數 */
	int		inChooseIndex = 0;						/* 最後選擇的封包 */
	char		szDebugMsg[1000 + 1];						/* DebugMsg */
	unsigned short	usReceiveBufferSize;						/* uszReceiveBuffer的長度，*/
	unsigned short	usOneSize = 1;							/* 一次只讀一個byte */
	unsigned short	usReceiveLen = 0;
	unsigned char	uszLRC;								/* LRC的值 */
	unsigned char	uszTempBuffer[1024 + 1];					/* 從 Buffer收到的片段 */
	unsigned char	uszReceiveBuffer[_ECR_BUFF_SIZE_];				/* 包含STX 和 ETX的原始電文 */
	unsigned char	uszReceivePacket[_ECR_UDP_PACKET_MAX_LIMIT_][_ECR_BUFF_SIZE_];	/* 收到的封包 */
	unsigned char	uszSTX = VS_FALSE;						/* 是否收到STX */
	unsigned char	uszDATA = VS_FALSE;						/* 是否收滿DATA */
	unsigned char	uszETX = VS_FALSE;						/* 是否收到ETX */
	unsigned char	uszLRCBit = VS_FALSE;						/* 是否收到LRC */
	unsigned char	uszSOH = VS_FALSE;						/* 是否收到SOH */
	unsigned char	uszEOT = VS_FALSE;						/* 是否收到EOT */
	unsigned char	uszPOSTxuniqueNoBit = VS_FALSE;
	unsigned char	uszCheckUnyBit = VS_FALSE;					/* 是否已檢查Indicator是否為Q */
	unsigned char	uszCheck144To400 = VS_FALSE;		
	unsigned char	uszReRunBit = VS_FALSE;
	
	/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/28 下午 2:42 */
	gbBarCodeECRBit = VS_FALSE;

	/* 設定Timeout */
	inRetVal = inDISP_Timer_Start(_ECR_RECEIVE_TIMER_, srECROb->srSetting.inTimeout);
	
	/* 初始化放收到資料的陣列 */
	memset(uszReceiveBuffer, 0x00, sizeof(uszReceiveBuffer));
	memset(uszReceivePacket, 0x00, sizeof(uszReceivePacket));
	inReceivePacket = 0;
	usReceiveBufferSize = 0;
	
	do
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
		
		/* 當Comport中有東西就開始分析 */
		while (usReceiveLen == 0)
		{
			/* 如果timeout就跳出去 */
			if (inTimerGet(_ECR_RECEIVE_TIMER_) == VS_SUCCESS)
			{
				return (VS_TIMEOUT);
			}
			
			inECR_Receive_Check(&usReceiveLen);
		}
		
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Len : %d", usReceiveLen);
			inLogPrintf(AT, szDebugMsg);
		}

		while (usReceiveLen > 0)
		{
			if (uszReRunBit == VS_TRUE)
			{
				memset(uszReceiveBuffer, 0x00, sizeof(uszReceiveBuffer));
				usReceiveBufferSize = 0;
				uszSOH = VS_FALSE;
				uszPOSTxuniqueNoBit = VS_FALSE;
				uszSTX = VS_FALSE;
				uszCheck144To400 = VS_FALSE;
				uszCheckUnyBit = VS_FALSE;
				uszDATA = VS_FALSE;
				uszETX = VS_FALSE;
				uszLRCBit = VS_FALSE;
				uszEOT = VS_FALSE;
				uszReRunBit = VS_FALSE;
			}
			
			/* 這邊一次只收一個byte */
			usOneSize = 1;
			memset(uszTempBuffer, 0x00, usOneSize);
			inRetVal = inECR_Data_Receive(uszTempBuffer, &usOneSize);
			if (inRetVal == VS_SUCCESS)
			{
				/* buffer讀出一個byte，長度減一 */
				usReceiveLen --;
			}
			
			/* 檢查是否收到SOH */
			if (uszSOH == VS_FALSE)
			{
				if (uszTempBuffer[0] == _SOH_)
				{
					/* 收到SOH */
					uszSOH = VS_TRUE;
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "Receive SOH!");
					}
					if (ginDisplayDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "Receive SOH");
						inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_2_, VS_FALSE);
					}

					memcpy(&uszReceiveBuffer[usReceiveBufferSize], &uszTempBuffer[0], 1);
					usReceiveBufferSize++;
					continue;
				}
				else
				{
					/* 繼續收 */
					continue;
				}
			}
			
			/* 檢查是否收到POSTxuniqueNo */
			if (uszPOSTxuniqueNoBit == VS_FALSE)
			{
				memcpy(&uszReceiveBuffer[usReceiveBufferSize], &uszTempBuffer[0], 1);
				usReceiveBufferSize++;
					
				if (usReceiveBufferSize >= (_ECR_UDP_SOH_SIZE_ + _ECR_UDP_HEADER_SIZE_))
				{
					memcpy(gsrECROb.srTransData.szPOSTxUniqueNo, &uszReceiveBuffer[_ECR_UDP_SOH_SIZE_], _ECR_UDP_HEADER_SIZE_);
					uszPOSTxuniqueNoBit = VS_TRUE;
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "POSTxUniqueNo:%s", gsrECROb.srTransData.szPOSTxUniqueNo);
					}
					continue;
				}
				else
				{
					/* 繼續收 */
					continue;
				}
			}
			
			/* 檢查是否收到STX */
			if (uszSTX == VS_FALSE)
			{
				if (uszTempBuffer[0] == _STX_)
				{
					/* 收到STX */
					uszSTX = VS_TRUE;
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "Receive STX!");
					}
					if (ginDisplayDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "Receive STX");
						inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_2_, VS_FALSE);
					}

					memcpy(&uszReceiveBuffer[usReceiveBufferSize], &uszTempBuffer[0], 1);
					usReceiveBufferSize++;
					continue;
				}
				else
				{
					/* 繼續收 */
					continue;
				}
			}
			
			/* 是否檢查過144轉400 */
			if (uszCheck144To400 == VS_FALSE)
			{
				if (usReceiveBufferSize >= (_ECR_UDP_SOH_SIZE_ + _ECR_UDP_HEADER_SIZE_ + _ECR_RS232_STX_SIZE_ + 1))
				{
					/* 若為為7E1協定，且收到STX之後的第一個為"I"，則自動轉為8N1 (NCCC ECR特殊規定) */
					if (inECR_NCCC_144_To_400(srECROb, uszReceiveBuffer) == VS_SUCCESS)
					{
						return (VS_ERROR);
					}
					uszCheck144To400 = VS_TRUE;
				}
			}
			
			/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/8/13 下午 4:10 */
			/* 是否檢查過400轉1000 */
			if (uszCheckUnyBit != VS_TRUE)
			{
				if (usReceiveBufferSize >= (_ECR_UDP_SOH_SIZE_ + _ECR_UDP_HEADER_SIZE_ + _ECR_RS232_STX_SIZE_ + 1))
				{
					if (uszReceiveBuffer[_ECR_UDP_SOH_SIZE_ + _ECR_UDP_HEADER_SIZE_ + _ECR_RS232_STX_SIZE_] == 'Q')
					{
						gbBarCodeECRBit = VS_TRUE;
						inDataSize = _ECR_8N1_1000_CUPQRCODE_Size_;
					}
					uszCheckUnyBit = VS_TRUE;
				}
			}
			
			/* 是否收滿data size */
			if (uszDATA == VS_FALSE)
			{
				/* 收STX到ETX中間的內容 */
				memcpy(&uszReceiveBuffer[usReceiveBufferSize], &uszTempBuffer[0], 1);
				usReceiveBufferSize++;
				if (usReceiveBufferSize >= (_ECR_UDP_SOH_SIZE_ + _ECR_UDP_HEADER_SIZE_ + _ECR_RS232_STX_SIZE_ + inDataSize))
				{
					uszDATA = VS_TRUE;
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "Receive DATA(%d)!", inDataSize);
					}
					continue;
				}
				else
				{
					continue;
				}
			}
			
			/* 是否收到ETX */
			if (uszETX == VS_FALSE)
			{
				if (uszTempBuffer[0] == _ETX_)
				{
					/* 收到ETX */
					uszETX = VS_TRUE;
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "Receive ETX!");
					}
					if (ginDisplayDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "Receive ETX");
						inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_2_, VS_FALSE);
					}
					memcpy(&uszReceiveBuffer[usReceiveBufferSize], &uszTempBuffer[0], 1);
					usReceiveBufferSize++;
					continue;
				}
				else
				{
					/* 繼續收 */
					continue;
				}
			}
			
			if (uszLRCBit == VS_FALSE)
			{
				memcpy(&uszReceiveBuffer[usReceiveBufferSize], &uszTempBuffer[0], 1);
				usReceiveBufferSize++;
				/* 收到LRC */
				uszLRCBit = VS_TRUE;
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "Receive LRC!");
				}
				if (ginDisplayDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "Receive LRC");
					inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_2_, VS_FALSE);
				}
				continue;
			}
			
			if (uszEOT == VS_FALSE)
			{

				if (uszTempBuffer[0] == _EOT_)
				{
					/* 收到EOT */
					uszEOT = VS_TRUE;
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "Receive EOT!");
					}
					if (ginDisplayDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "Receive EOT");
						inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_2_, VS_FALSE);
					}
					memcpy(&uszReceiveBuffer[usReceiveBufferSize], &uszTempBuffer[0], 1);
					usReceiveBufferSize++;
					break;
				}
				else
				{
					/* 繼續收 */
					continue;
				}
			}
		}


		/* 有收到STX、ETX而且收到長度也對，就開始算LRC */
		if (uszEOT == VS_TRUE)
		{
			/* 計算收到DATA的LRC （Stx Not Include）*/
			/* 前面18個不記算SOH + POSTx + STX */
			for (i = (_ECR_UDP_SOH_SIZE_ + _ECR_UDP_HEADER_SIZE_ + _ECR_RS232_STX_SIZE_); i < (inDataSize + (_ECR_UDP_SOH_SIZE_ + _ECR_UDP_HEADER_SIZE_ + _ECR_RS232_STX_SIZE_ + _ECR_RS232_ETX_SIZE_)); i++)
			{
				uszLRC ^= uszReceiveBuffer[i];
			}
			
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "Calculate LRC: %02x", uszLRC);
			}
			
			if (uszReceiveBuffer[((inDataSize + (_ECR_UDP_SOH_SIZE_ + _ECR_UDP_HEADER_SIZE_ + _ECR_RS232_STX_SIZE_ + _ECR_RS232_ETX_SIZE_ + _ECR_RS232_LRC_SIZE_)) - 1)] == uszLRC)
			{
				if (srECROb->srOptionalSetting.uszNotSendAck == VS_TRUE)
				{
					
				}
				else
				{
					/* 比對收到的LRC是否正確，若正確回傳ACK */
					inECR_Send_ACKorNAK(srECROb, _ACK_);
				}
				
				/* 完整收到封包 */
				memcpy(&uszReceivePacket[inReceivePacket][0], &uszReceiveBuffer[_ECR_UDP_SOH_SIZE_ + _ECR_UDP_HEADER_SIZE_ + _ECR_RS232_STX_SIZE_ ], inDataSize);
				memset(uszReceiveBuffer, 0x00, sizeof(uszReceiveBuffer));
				inReceivePacket++;
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
					sprintf(&szDebugMsg[0], "%X", uszReceiveBuffer[((inDataSize + (_ECR_UDP_SOH_SIZE_ + _ECR_UDP_HEADER_SIZE_ + _ECR_RS232_STX_SIZE_ + _ECR_RS232_ETX_SIZE_ + _ECR_RS232_LRC_SIZE_)) - 1)]);
					inLogPrintf(AT, szDebugMsg);
				}
				if (ginDisplayDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "LRC error! Retry: %d", inRetry);
					inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_3_, VS_TRUE);
				}
				
				/* 若錯誤回傳NAK */
				inECR_Send_ACKorNAK(srECROb, _NAK_);
				/* retry次數+ 1 */
				inRetry ++;
				
				/* 初始化資料 */
				uszReRunBit = VS_TRUE;
				continue;
			}
			
		}/* 算LRC END */
		
		/* 收不到，而且收到超過一個封包就跳出 */
		usReceiveLen = 0;
		inECR_Receive_Check(&usReceiveLen);
		if (usReceiveLen > 0)
		{
			continue;
		}
		
		/* 收到超過一個封包 */
		if (inReceivePacket > 0)
		{
			inChooseIndex = inReceivePacket - 1;
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "Last Choose Packet(Index from 0):(%d)", inChooseIndex);
			}
			break;
		}
		
		
	}while (1);/* Receive END */
	
	/* 去除STX、ETX、LRC，把資料放到szrRealReceBuffer */
	memcpy(szDataBuffer, &uszReceivePacket[inChooseIndex][0], inDataSize);
	
	/* 列印紙本電文和顯示電文訊息 */
	inECR_Print_Receive_ISODeBug(szDataBuffer, usReceiveBufferSize - (_ECR_UDP_SOH_SIZE_ + _ECR_UDP_HEADER_SIZE_ + _ECR_RS232_STX_SIZE_ + _ECR_RS232_ETX_SIZE_ + _ECR_RS232_LRC_SIZE_), inDataSize);
	
	/* [114197] 中心收單作業ESG永續發展優化 
	*  [260610] UDP 電子簽單不支援 Uny交易，過濾掉 ECR Indicator ='Q'的交易，gbEIECRBit 這部分不確定是否包含 */
	if (gbBarCodeECRBit == VS_TRUE || gbEIECRBit == VS_TRUE)
	{	
		/* 跳過詢問持卡人是否同意接收數位化簽帳單程式 */
		pobTran->uszEsgOptrionBit = VS_TRUE;
		/* ESG需求不包含 掃碼交易規格/電子發票交易規格 */
		pobTran->szAgreeEsgBill[0] = 'N';
		pobTran->szEsgSendCR[0] = 'N';
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inECR_Not_Check_Sizes_Receive
Date&Time       :2017/11/16 上午 11:45
Describe        :
*/
int inECR_Not_Check_Sizes_Receive(TRANSACTION_OBJECT *pobTran, ECR_TABLE* srECROb, char *szDataBuffer, int inDataSize)
{
	int		i;
	int		inRetVal;
	int		inRetry = 0;							/* 目前已重試次數 */
	int		inRecMaxRetry = _ECR_RETRYTIMES_;				/* 最大重試次數 */
	char		szDebugMsg[1024 + 1];						/* DebugMsg */
	unsigned short	usReceiveBufferSize;						/* uszReceiveBuffer的長度，*/
	unsigned short	usOneSize = 1;							/* 一次只讀一個byte */
	unsigned short	usReceiveLen = 0;
	unsigned char	uszLRC;								/* LRC的值 */
	unsigned char	uszTempBuffer[1024 + 1];					/* 從 Buffer收到的片段 */
	unsigned char	uszReceiveBuffer[_ECR_BUFF_SIZE_];				/* 包含STX 和 ETX的原始電文 */
	unsigned char	uszSTX = VS_FALSE;						/* 是否收到STX */
	unsigned char	uszETX = VS_FALSE;						/* 是否收到ETX */
	
	/* 設定Timeout */
	inRetVal = inDISP_Timer_Start(_ECR_RECEIVE_TIMER_, srECROb->srSetting.inTimeout);
	
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
		
		/* 當Comport中有東西就開始分析 */
		while (usReceiveLen == 0)
		{
			/* 如果timeout就跳出去 */
			if (inTimerGet(_ECR_RECEIVE_TIMER_) == VS_SUCCESS)
			{
				return (VS_TIMEOUT);
			}
			
			inECR_Receive_Check(&usReceiveLen);
		}
		
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Len : %d", usReceiveLen);
			inLogPrintf(AT, szDebugMsg);
		}


		/* 如果沒收到STX就會進這隻，直到分析出STX */
		while (uszSTX == VS_FALSE && usReceiveLen > 0)
		{
			/* 如果timeout就跳出去 */
			if (inTimerGet(_ECR_RECEIVE_TIMER_) == VS_SUCCESS)
			{
				return (VS_TIMEOUT);
			}
			
			/* 這邊一次只收一個byte */
			usOneSize = 1;
			memset(uszTempBuffer, 0x00, usOneSize);
			inRetVal = inECR_Data_Receive(uszTempBuffer, &usOneSize);
			if (inRetVal == VS_SUCCESS)
			{
				/* 若為為7E1協定，且收到STX之後的第一個為"I"，則自動轉為8N1 (NCCC ECR特殊規定) */
				if (inECR_NCCC_144_To_400(srECROb, uszTempBuffer) == VS_SUCCESS)
				{
					return (VS_ERROR);
				}
				
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
			if (inTimerGet(_ECR_RECEIVE_TIMER_) == VS_SUCCESS)
			{
				return (VS_TIMEOUT);
			}
			
			usOneSize = usReceiveLen;
			memset(uszTempBuffer, 0x00, usOneSize);
			inRetVal = inECR_Data_Receive(uszTempBuffer, &usOneSize);

			if (inRetVal == VS_SUCCESS)
			{			
				/* buffer讀出一個byte，長度減一 */
				memcpy(&uszReceiveBuffer[usReceiveBufferSize], uszTempBuffer, usOneSize);
				usReceiveLen -= usOneSize;
				usReceiveBufferSize += usOneSize;

				if (uszReceiveBuffer[usReceiveBufferSize - 1] == _ETX_)
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
					inRetVal = inECR_Data_Receive(uszTempBuffer, &usOneSize);
					if (inRetVal == VS_SUCCESS)
					{
						memcpy(&uszReceiveBuffer[usReceiveBufferSize], uszTempBuffer, usOneSize);
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
					usReceiveBufferSize ++;
				}	
				
			}

		}


		/* 有收到STX、ETX而且收到長度也對，就開始算LRC */
		if (uszSTX == VS_TRUE && uszETX == VS_TRUE)
		{		
			/* 計算收到DATA的LRC （Stx Not Include）*/
			for (i = 1; i <= (usReceiveBufferSize - 2); i++)
			{
				uszLRC ^= uszReceiveBuffer[i];
			}
			
			if (uszReceiveBuffer[usReceiveBufferSize-1] == uszLRC)
			{
				/* 比對收到的LRC是否正確，若正確回傳ACK */
				inECR_Send_ACKorNAK(srECROb, _ACK_);
				
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
				inECR_Send_ACKorNAK(srECROb, _NAK_);
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
	memcpy(szDataBuffer, &uszReceiveBuffer[1], usReceiveBufferSize -3);
	
	/* 列印紙本電文和顯示電文訊息 */
	inECR_Print_Receive_ISODeBug(szDataBuffer, usReceiveBufferSize - 3, usReceiveBufferSize -3);
	
	return (VS_SUCCESS);
}

/*
Function        :inECR_Send
Date&Time       :2017/11/15 下午 5:21
Describe        :傳送要給收銀機的資料
*/
int inECR_Send(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer, int inDataSize)
{
	int		i = 0;
	int		inRetVal = VS_SUCCESS;
	int		inFinalRetVal = VS_ERROR;
	int		inTryTimes = 0;
	int		inTryTimesMax = _ECR_RETRYTIMES_;					/* 最大重試次數 */
	int		inSendLen = 0;
	int		inDataIndex = 0;
	int		inCheckTimeout = 0;
	char		szDebugMsg[100 +1] = {0};
	unsigned char	uszSendBuf[_ECR_BUFF_SIZE_];					/* 包含STX、ETX、LRC的電文 */
	unsigned char	uszLRC = 0;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inECR_Send() START !");
	}
	
	if (srECROb->srSetting.uszNonDefaultRetry == VS_TRUE)
	{
		inTryTimesMax = srECROb->srSetting.inNonDefaultRetryTimes + 1;
	}
	else
	{
		inTryTimesMax = _ECR_RETRYTIMES_ + 1;
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inTryTimesMax:%d", inTryTimesMax);
	}
	
	/* 清send Buffer */
	inECR_FlushTxBuffer();

	/* Send之前清Buffer，避免收到錯的回應 */
	inECR_FlushRxBuffer();
	
	if (gbECR_UDP_TransBit == VS_TRUE)
	{
		inDataIndex = 18;
		uszSendBuf[inSendLen] = _SOH_;
		inSendLen ++;
		memcpy(&uszSendBuf[inSendLen], gsrECROb.srTransData.szPOSTxUniqueNo, 16);
		inSendLen += 16;
	}
	else
	{
		inDataIndex = 1;
	}
	
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
	for (i = inDataIndex; i < (inSendLen); i++)
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
	
	if (gbECR_UDP_TransBit == VS_TRUE)
	{
		uszSendBuf[inSendLen] = _EOT_;
		inSendLen ++;
	}
	
	inFinalRetVal = VS_ERROR;
	for (inTryTimes = 0; inTryTimes < inTryTimesMax; )
	{
		inCheckTimeout = 2;
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, inCheckTimeout);
		/* 檢查port是否已經準備好要送資料 */
		do
		{
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				if (ginDisplayDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "ECR Check Timeout");
					inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_2_, VS_FALSE);
				}
				/* 即使Send Check不成功也嘗試送 */
				break;
			}
			
			/* 檢查port是否已經準備好要送資料 */	
			if (inECR_Send_Check() == VS_SUCCESS)
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "ECR_Send_Check OK");
				}
				if (ginDisplayDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "ECR Send Check OK");
					inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_2_, VS_FALSE);
				}
				break;
			}
			else
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "ECR_Send_Check NOT OK");
				}
				if (ginDisplayDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "ECR Send Check Not OK");
					inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_2_, VS_FALSE);
				}
			}
		}while(1);
		
		/* 經由port傳送資料 */
		inRetVal = inECR_Data_Send(uszSendBuf, (unsigned short)inSendLen);
		/* 不管是否成功，嘗試次數都 +1 */
		inTryTimes++;
		if (inRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "ECR Data send fail");
			}
			inFinalRetVal = VS_ERROR;
		}
		else
		{
			
		/*---------------------------------- 印Debug 開始---------------------------------------------------------------*/
			/* 列印紙本電文和顯示電文訊息 */
			inECR_Print_Send_ISODeBug(szDataBuffer, inSendLen, inDataSize);
		/*---------------------------------- 印Debug 結束---------------------------------------------------------------*/
			if (srECROb->srOptionalSetting.uszNotReceiveAck == VS_TRUE)
			{
				/* 成功 */
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "ECR NO NEED ACK OK!");
				}
				if (ginDisplayDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "ECR NO NEED ACK OK!");
					inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_2_, VS_FALSE);
				}
				inFinalRetVal = VS_SUCCESS;
				break;
			}
			else
			{
				/* 接收ACK OR NAK */
				inRetVal = inECR_Receive_ACKandNAK(srECROb);

				/* 超過一秒沒收到回應 */
				if (inRetVal == VS_TIMEOUT)
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "Not Receive Response,Need Retry");
					}
					if (ginDisplayDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "Not Receive Response,Need Retry");
						inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_2_, VS_FALSE);
					}
					inFinalRetVal = VS_ERROR;
				}
				/* 收到NAK */
				else if (inRetVal == _NAK_)
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "Receive NAK,Need Retry");
					}
					if (ginDisplayDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "Receive NAK,Need Retry");
						inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_2_, VS_FALSE);
					}
					inFinalRetVal = VS_ERROR;
				}
				/* 收到ACK */	
				else
				{	
					/* 成功 */
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "ECR ACK OK!");
					}
					if (ginDisplayDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "ECR ACK OK!");
						inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_2_, VS_FALSE);
					}
					inFinalRetVal = VS_SUCCESS;
					break;
				}
			}/* 需要接收ACK的流程 */
		}/* 送ECR成功的判斷流程*/
		
		/* 超過最大重試次數，仍要完成交易，收銀機提示補登畫面 */
		/* MPAS要沒收到ACK要印簽單 */
		/* 傳送Retry */
		if (inTryTimes >= inTryTimesMax)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "Exceed max retry times!");
			}
			if (ginDisplayDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Exceed max retry times!");
				inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_2_, VS_FALSE);
			}
			inFinalRetVal = VS_ERROR;
			break;
		}
		
	};
	
	if (inFinalRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inECR_Send()Error END !");
			inLogPrintf(AT, "----------------------------------------");
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inECR_Send()Success END !");
			inLogPrintf(AT, "----------------------------------------");
		}
		
		return (VS_SUCCESS);
	}
}
	
/*
Function        :inECR_Send_NotACK
Date&Time       :2017/11/15 下午 5:21
Describe        :傳送要給收銀機的資料
*/
int inECR_Send_NotACK(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer, int inDataSize)
{
	int		i;
	int		inRetVal;
	int		inSendLen = 0;
	char		szDebugMsg[100 +1] = {0};
	unsigned char	uszSendBuf[_ECR_BUFF_SIZE_];					/* 包含STX、ETX、LRC的電文 */
	unsigned char	uszLRC = 0;
	
	/* 清send Buffer */
	inECR_FlushTxBuffer();

	/* Send之前清Buffer，避免收到錯的回應 */
	inECR_FlushRxBuffer();
	
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
				if (ginDisplayDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "ECR Send Timeout");
					inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_2_, VS_FALSE);
				}
				return (VS_TIMEOUT);
			}
			
			if (inECR_Send_Check() == VS_SUCCESS)
			{
				break;
			}
			else
			{
				if (ginDisplayDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "ECR Send Check Not OK");
					inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_2_, VS_FALSE);
				}
			}
		};

		/* 經由port傳送資料 */
		inRetVal = inECR_Data_Send(uszSendBuf, (unsigned short)inSendLen);
                
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
                        /* [113110-信託需求][ECR] 因原有ECR收不到鏡象訊息後的資料，秒數由0.5秒調整為1秒  2025/11/20 */
                        inDISP_Wait(1000);
                        return (VS_SUCCESS);
		}/*  */
	
	}/* while(1) */
}

/*
Function        :inECR_Customer_Flow
Date&Time       :2018/7/25 下午 4:26
Describe        :
*/
int inECR_Customer_Flow(TRANSACTION_OBJECT* pobTran)
{
	int			inRetVal = VS_ERROR;
	char			szCustomerIndicator[3 + 1] = {0};
	char			szECRVersion[2 + 1] = {0};
	char			szUDP_ECRVersion[2 + 1] = {0};
	unsigned char		uszNoNeedInquirBit = VS_FALSE;
	TRANSACTION_OBJECT	pobTempTran = {};
	
        vdUtility_SYSFIN_LogMessage(AT, "inECR_Customer_Flow START!");
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inECR_Customer_Flow() START !");
	}
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
        if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_003_WELLCOME_, _CUSTOMER_INDICATOR_SIZE_))
	{
		/* 兩段式流程 */
		if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
		{
			if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
			{
				inRetVal = inFLOW_RunFunction(pobTran, _TICKET_INQUIRY_CARD_NO_FLOW_);
				/* 不論最後成功與否都要切回NCCC */
				inNCCC_tSAM_SelectAID_NCCC_Flow();
				if (inRetVal != VS_SUCCESS)
				{
					return (inRetVal);
				}
			}
			else if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				inRetVal = inNCCC_Func_Card_Inquery_Issuer_Bank_Fisc(pobTran);
				if (inRetVal != VS_SUCCESS)
				{
					return (inRetVal);
				}
			}
			else
			{
				inRetVal = inFLOW_RunFunction(pobTran, _NCCC_FUNCTION_CARD_INQUERY_ISSUER_BANK_);
				if (inRetVal != VS_SUCCESS)
				{
					return (inRetVal);
				}
			}
			
			inRetVal = inFLOW_RunFunction(pobTran, _FUNCTION_ECR_SEND_INQUIRY_RESULT_);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
			
			memcpy(&pobTempTran, pobTran, sizeof(TRANSACTION_OBJECT));
			
			/* 清空ResponseCode，避免使用到卡號查詢的ResponseCode */
			memset(pobTran->srBRec.szRespCode, 0x00, sizeof(pobTran->srBRec.szRespCode));
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				pobTran->srBRec.inFiscIssuerIDLength = 0;
				pobTran->srBRec.inFiscCardCommentLength = 0;
				pobTran->srBRec.inFiscAccountLength = 0;
				memset(pobTran->srBRec.szFiscIssuerID, 0x00, sizeof(pobTran->srBRec.szFiscIssuerID));
				memset(pobTran->srBRec.szFiscCardComment, 0x00, sizeof(pobTran->srBRec.szFiscCardComment));
				memset(pobTran->srBRec.szFiscAccount, 0x00, sizeof(pobTran->srBRec.szFiscAccount));
                                memset(pobTran->srBRec.szPAN, 0x00, sizeof(pobTran->srBRec.szPAN));
                                pobTran->srBRec.uszContactlessBit = VS_FALSE;
			}
			
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inRetVal = inFLOW_RunFunction(pobTran, _FUNCTION_ECR_RECEIVE_SECOND_TRANSACTION_REQUEST_);
			if (inRetVal != VS_SUCCESS)
			{
				if (inRetVal == VS_TIMEOUT)
				{
					/* 如果超過10秒，要顯示接收資料錯誤 */
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				}
				
				return (inRetVal);
			}
                        
                        /* 若第一次查詢和第二次交易卡號不同，顯示操作錯誤 */
                        if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				if (memcmp(pobTran->srBRec.szPAN, pobTempTran.srBRec.szPAN, strlen(pobTran->srBRec.szPAN)))
                                {
                                        pobTran->inErrorMsg = _ERROR_CODE_V3_OPERATION_;
                                        pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "查詢前後卡號不一致");
                                        }
                                        vdUtility_SYSFIN_LogMessage(AT, "查詢前後卡號不一致");
                                        
                                        return (VS_ERROR);
                                }
                                
                                if ((pobTran->srBRec.uszContactlessBit == VS_TRUE && pobTempTran.srBRec.uszContactlessBit != VS_TRUE) ||
                                    (pobTran->srBRec.uszContactlessBit != VS_TRUE && pobTempTran.srBRec.uszContactlessBit == VS_TRUE))
                                {
                                        pobTran->inErrorMsg = _ERROR_CODE_V3_OPERATION_;
                                        pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "查詢前後交易方式不一致");
                                        }
                                        vdUtility_SYSFIN_LogMessage(AT, "查詢前後交易方式不一致");
                                        
                                        return (VS_ERROR);
                                }
                                        
			}
			
			/* 若回傳70終止卡號查詢，會在上面回傳錯誤，不會顯示確認金額 */
			inRetVal = inFLOW_RunFunction(pobTran, _NCCC_FUNCTION_CHECK_INQUIRY_ISSUER_AMOUNT_);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
		}
		else
		{
			inRetVal = VS_SUCCESS;
		}
	}
	/* 107邦柏要用Echo Host */
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_))
	{
		/* 客製化107查詢卡號要當Echo Test */
		if (!memcmp(gsrECROb.srTransData.szTransType, _ECR_8N1_START_CARD_NO_INQUIRY_, strlen(_ECR_8N1_START_CARD_NO_INQUIRY_)))
		{
			/* 測試網路連線結果，若失敗在最外層回傳ECR_ERROR */
			inRetVal = inNCCC_Func_Customer_107_Bumper_Echo_Test(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_COMM_ERROR_;
				return (inRetVal);
			}
			
			/* 測試感應裝置 */
			if (inCTLS_DevicePooling_Internal(pobTran) != VS_SUCCESS)
			{
				DISPLAY_OBJECT	srDispMsgObj;
				memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
				strcpy(srDispMsgObj.szDispPic1Name, "");
				srDispMsgObj.inDispPic1YPosition = 0;
				srDispMsgObj.inMsgType = _NO_KEY_MSG_;
				srDispMsgObj.inTimeout = 0;
				strcpy(srDispMsgObj.szErrMsg1, "感應器連線失敗");
				srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
				srDispMsgObj.inBeepTimes = 3;
				srDispMsgObj.inBeepInterval = 2000;
			
				inDISP_Msg_BMP(&srDispMsgObj);
				return (inRetVal);
			}
			
			/* 回傳測試結果 */
			inRetVal = inFLOW_RunFunction(pobTran, _FUNCTION_ECR_SEND_TRANSACTION_RESULT_);
			
			/* 清空ResponseCode，避免使用到卡號查詢的ResponseCode */
			memset(pobTran->srBRec.szRespCode, 0x00, sizeof(pobTran->srBRec.szRespCode));
		}
		else
		{
			inRetVal = VS_SUCCESS;
		}
	}
	/* (需求單 - 107276)自助交易標準400做法 不提示「二段式收銀機連線，EDC要按0確認金額」畫面。by Russell 2018/12/27 上午 11:20 */
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_)	||
		 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)			||
		 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
	{
		/* 兩段式流程 */
		if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
		{
			if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
			{
				inRetVal = inFLOW_RunFunction(pobTran, _TICKET_INQUIRY_CARD_NO_FLOW_);
				/* 不論最後成功與否都要切回NCCC */
				inNCCC_tSAM_SelectAID_NCCC_Flow();
				if (inRetVal != VS_SUCCESS)
				{
					return (inRetVal);
				}
			}
			else if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				inRetVal = inNCCC_Func_Card_Inquery_Issuer_Bank_Fisc(pobTran);
				if (inRetVal != VS_SUCCESS)
				{
					return (inRetVal);
				}
			}
			else
			{
				inRetVal = inFLOW_RunFunction(pobTran, _NCCC_FUNCTION_CARD_INQUERY_ISSUER_BANK_);
				if (inRetVal != VS_SUCCESS)
				{
					return (inRetVal);
				}
			}
			
			inRetVal = inFLOW_RunFunction(pobTran, _FUNCTION_ECR_SEND_INQUIRY_RESULT_);
			
			memcpy(&pobTempTran, pobTran, sizeof(TRANSACTION_OBJECT));
			
			/* 清空ResponseCode，避免使用到卡號查詢的ResponseCode */
			memset(pobTran->srBRec.szRespCode, 0x00, sizeof(pobTran->srBRec.szRespCode));
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				pobTran->srBRec.inFiscIssuerIDLength = 0;
				pobTran->srBRec.inFiscCardCommentLength = 0;
				pobTran->srBRec.inFiscAccountLength = 0;
				memset(pobTran->srBRec.szFiscIssuerID, 0x00, sizeof(pobTran->srBRec.szFiscIssuerID));
				memset(pobTran->srBRec.szFiscCardComment, 0x00, sizeof(pobTran->srBRec.szFiscCardComment));
				memset(pobTran->srBRec.szFiscAccount, 0x00, sizeof(pobTran->srBRec.szFiscAccount));
                                memset(pobTran->srBRec.szPAN, 0x00, sizeof(pobTran->srBRec.szPAN));
                                pobTran->srBRec.uszContactlessBit = VS_FALSE;
			}
			
                        /* 若第一次查詢和第二次交易卡號不同，顯示操作錯誤 */
                        if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				if (memcmp(pobTran->srBRec.szPAN, pobTempTran.srBRec.szPAN, strlen(pobTran->srBRec.szPAN)))
                                {
                                        pobTran->inErrorMsg = _ERROR_CODE_V3_OPERATION_;
                                        pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "查詢前後卡號不一致");
                                        }
                                        vdUtility_SYSFIN_LogMessage(AT, "查詢前後卡號不一致");
                                        
                                        return (VS_ERROR);
                                }
                                
                                if ((pobTran->srBRec.uszContactlessBit == VS_TRUE && pobTempTran.srBRec.uszContactlessBit != VS_TRUE) ||
                                    (pobTran->srBRec.uszContactlessBit != VS_TRUE && pobTempTran.srBRec.uszContactlessBit == VS_TRUE))
                                {
                                        pobTran->inErrorMsg = _ERROR_CODE_V3_OPERATION_;
                                        pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "查詢前後交易方式不一致");
                                        }
                                        vdUtility_SYSFIN_LogMessage(AT, "查詢前後交易方式不一致");
                                        
                                        return (VS_ERROR);
                                }
                                        
			}
                        
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inRetVal = inFLOW_RunFunction(pobTran, _FUNCTION_ECR_RECEIVE_SECOND_TRANSACTION_REQUEST_);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
			
			/* (需求單 - 107276)自助交易標準400做法 不提示「二段式收銀機連線，EDC要按0確認金額」畫面。by Russell 2018/12/27 上午 11:20 */
		}
		else
		{
			inRetVal = VS_SUCCESS;
		}
	}
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_033_UNIAIR_, _CUSTOMER_INDICATOR_SIZE_))
	{
		/* 兩段式流程 */
		if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
		{
			if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
			{
				inRetVal = inFLOW_RunFunction(pobTran, _TICKET_INQUIRY_CARD_NO_FLOW_);
				/* 不論最後成功與否都要切回NCCC */
				inNCCC_tSAM_SelectAID_NCCC_Flow();
				if (inRetVal != VS_SUCCESS)
				{
					return (inRetVal);
				}
			}
			else
			{
				/* 客製化033不送卡號查詢電文 */
			}
			
			inRetVal = inFLOW_RunFunction(pobTran, _FUNCTION_ECR_SEND_INQUIRY_RESULT_);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
			
			memcpy(&pobTempTran, pobTran, sizeof(TRANSACTION_OBJECT));
			
			/* 清空ResponseCode，避免使用到卡號查詢的ResponseCode */
			memset(pobTran->srBRec.szRespCode, 0x00, sizeof(pobTran->srBRec.szRespCode));
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				pobTran->srBRec.inFiscIssuerIDLength = 0;
				pobTran->srBRec.inFiscCardCommentLength = 0;
				pobTran->srBRec.inFiscAccountLength = 0;
				memset(pobTran->srBRec.szFiscIssuerID, 0x00, sizeof(pobTran->srBRec.szFiscIssuerID));
				memset(pobTran->srBRec.szFiscCardComment, 0x00, sizeof(pobTran->srBRec.szFiscCardComment));
				memset(pobTran->srBRec.szFiscAccount, 0x00, sizeof(pobTran->srBRec.szFiscAccount));
                                memset(pobTran->srBRec.szPAN, 0x00, sizeof(pobTran->srBRec.szPAN));
                                pobTran->srBRec.uszContactlessBit = VS_FALSE;
			}
			
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inRetVal = inFLOW_RunFunction(pobTran, _FUNCTION_ECR_RECEIVE_SECOND_TRANSACTION_REQUEST_);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
			
                        /* 若第一次查詢和第二次交易卡號不同，顯示操作錯誤 */
                        if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				if (memcmp(pobTran->srBRec.szPAN, pobTempTran.srBRec.szPAN, strlen(pobTran->srBRec.szPAN)))
                                {
                                        pobTran->inErrorMsg = _ERROR_CODE_V3_OPERATION_;
                                        pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "查詢前後卡號不一致");
                                        }
                                        vdUtility_SYSFIN_LogMessage(AT, "查詢前後卡號不一致");
                                        
                                        return (VS_ERROR);
                                }
                                
                                if ((pobTran->srBRec.uszContactlessBit == VS_TRUE && pobTempTran.srBRec.uszContactlessBit != VS_TRUE) ||
                                    (pobTran->srBRec.uszContactlessBit != VS_TRUE && pobTempTran.srBRec.uszContactlessBit == VS_TRUE))
                                {
                                        pobTran->inErrorMsg = _ERROR_CODE_V3_OPERATION_;
                                        pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "查詢前後交易方式不一致");
                                        }
                                        vdUtility_SYSFIN_LogMessage(AT, "查詢前後交易方式不一致");
                                        
                                        return (VS_ERROR);
                                }
                                        
			}
                        
			/* 若回傳70終止卡號查詢，會在上面回傳錯誤，不會顯示確認金額 */
			inRetVal = inFLOW_RunFunction(pobTran, _NCCC_FUNCTION_CHECK_INQUIRY_ISSUER_AMOUNT_);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
		}
		else
		{
			inRetVal = VS_SUCCESS;
		}
	}
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_005_FPG_, _CUSTOMER_INDICATOR_SIZE_))
	{
		/* 兩段式流程 */
		if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
		{
			if (gsrECROb.srTransData.srCus005_Data.uszHappyGoCardBit == VS_TRUE)
			{
				/* 詢HG部份 */
				inRetVal = inFLOW_RunFunction(pobTran, _FUNCTION_CHECK_TERM_STATUS_);
			        if (inRetVal != VS_SUCCESS)
				{
					return (inRetVal);
				}
				
				inRetVal = inFLOW_RunFunction(pobTran, _NCCC_FUNCTION_CHECK_TXN_FUNCTION_WHEN_ECR_);
			        if (inRetVal != VS_SUCCESS)
				{
					return (inRetVal);
				}
				
				inRetVal = inFLOW_RunFunction(pobTran, _FUNCTION_GET_CARD_FIELDS_HG_);
			        if (inRetVal != VS_SUCCESS)
				{
					return (inRetVal);
				}
				
				inRetVal = inFLOW_RunFunction(pobTran, _FUNCTION_CHECK_HG_PAN_EXPDATE_);
			        if (inRetVal != VS_SUCCESS)
				{
					return (inRetVal);
				}
				
				/* 回傳 */
				inRetVal = inFLOW_RunFunction(pobTran, _FUNCTION_ECR_SEND_INQUIRY_RESULT_);
				if (inRetVal != VS_SUCCESS)
				{
					return (inRetVal);
				}
				
				/* 清空ResponseCode，避免使用到卡號查詢的ResponseCode */
				memset(pobTran->srBRec.szRespCode, 0x00, sizeof(pobTran->srBRec.szRespCode));

				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inRetVal = inFLOW_RunFunction(pobTran, _FUNCTION_ECR_RECEIVE_SECOND_TRANSACTION_REQUEST_);
				if (inRetVal != VS_SUCCESS)
				{
					return (inRetVal);
				}
				
				inRetVal = inFLOW_RunOperation(pobTran, _OPERATION_SALE_ICC_);
			        if (inRetVal != VS_SUCCESS)
				{
					return (inRetVal);
				}
			}
			else
			{
				inRetVal = inFLOW_RunOperation(pobTran, _OPERATION_SALE_ICC_);
			        if (inRetVal != VS_SUCCESS)
				{
					return (inRetVal);
				}
			}
			
			inRetVal = inFLOW_RunFunction(pobTran, _FUNCTION_ECR_SEND_INQUIRY_RESULT_);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
			
			memcpy(&pobTempTran, pobTran, sizeof(TRANSACTION_OBJECT));
			
			/* 清空ResponseCode，避免使用到卡號查詢的ResponseCode */
			memset(pobTran->srBRec.szRespCode, 0x00, sizeof(pobTran->srBRec.szRespCode));
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				pobTran->srBRec.inFiscIssuerIDLength = 0;
				pobTran->srBRec.inFiscCardCommentLength = 0;
				pobTran->srBRec.inFiscAccountLength = 0;
				memset(pobTran->srBRec.szFiscIssuerID, 0x00, sizeof(pobTran->srBRec.szFiscIssuerID));
				memset(pobTran->srBRec.szFiscCardComment, 0x00, sizeof(pobTran->srBRec.szFiscCardComment));
				memset(pobTran->srBRec.szFiscAccount, 0x00, sizeof(pobTran->srBRec.szFiscAccount));
                                memset(pobTran->srBRec.szPAN, 0x00, sizeof(pobTran->srBRec.szPAN));
                                pobTran->srBRec.uszContactlessBit = VS_FALSE;
			}
			
			/* 等待資料中 */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_ChineseFont("等待資料中", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
			
			inRetVal = inFLOW_RunFunction(pobTran, _FUNCTION_ECR_RECEIVE_SECOND_TRANSACTION_REQUEST_);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
                        
                        /* 若第一次查詢和第二次交易卡號不同，顯示操作錯誤 */
                        if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				if (memcmp(pobTran->srBRec.szPAN, pobTempTran.srBRec.szPAN, strlen(pobTran->srBRec.szPAN)))
                                {
                                        pobTran->inErrorMsg = _ERROR_CODE_V3_OPERATION_;
                                        pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "查詢前後卡號不一致");
                                        }
                                        vdUtility_SYSFIN_LogMessage(AT, "查詢前後卡號不一致");
                                        
                                        return (VS_ERROR);
                                }
                                
                                if ((pobTran->srBRec.uszContactlessBit == VS_TRUE && pobTempTran.srBRec.uszContactlessBit != VS_TRUE) ||
                                    (pobTran->srBRec.uszContactlessBit != VS_TRUE && pobTempTran.srBRec.uszContactlessBit == VS_TRUE))
                                {
                                        pobTran->inErrorMsg = _ERROR_CODE_V3_OPERATION_;
                                        pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "查詢前後交易方式不一致");
                                        }
                                        vdUtility_SYSFIN_LogMessage(AT, "查詢前後交易方式不一致");
                                        
                                        return (VS_ERROR);
                                }
                                        
			}
		}
		else
		{
			inRetVal = VS_SUCCESS;
		}
	}
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_026_TAKA_, _CUSTOMER_INDICATOR_SIZE_) ||
		 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
	{
		/* 兩段式流程 */
		if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
		{
			memset(szECRVersion, 0x00, sizeof(szECRVersion));
			inGetECRVersion(szECRVersion);
			memset(szUDP_ECRVersion, 0x00, sizeof(szUDP_ECRVersion));
			inGetUDP_ECRVersion(szUDP_ECRVersion);
			
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_026_TAKA_, _CUSTOMER_INDICATOR_SIZE_))
			{
				if (gbECR_UDP_TransBit == VS_FALSE && !memcmp(szECRVersion, _ECR_RS232_VERSION_026_TAKA_, 2))
				{
					uszNoNeedInquirBit = VS_TRUE;
				}
			}
			else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
			{
				if (gbECR_UDP_TransBit == VS_FALSE && !memcmp(szECRVersion, _ECR_RS232_VERSION_021_TAKAWEL_, 2))
				{
					uszNoNeedInquirBit = VS_TRUE;
				}
			}
			
			if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
			{
				inRetVal = inFLOW_RunFunction(pobTran, _TICKET_INQUIRY_CARD_NO_FLOW_);
				/* 不論最後成功與否都要切回NCCC */
				inNCCC_tSAM_SelectAID_NCCC_Flow();
				if (inRetVal != VS_SUCCESS)
				{
					return (inRetVal);
				}
			}
			else if (uszNoNeedInquirBit == VS_TRUE)
			{
				/* 7E1版本ECR不用送電文 */
			}
			else if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				inRetVal = inNCCC_Func_Card_Inquery_Issuer_Bank_Fisc(pobTran);
				if (inRetVal != VS_SUCCESS)
				{
					return (inRetVal);
				}
			}
			else
			{
				inRetVal = inFLOW_RunFunction(pobTran, _NCCC_FUNCTION_CARD_INQUERY_ISSUER_BANK_);
				if (inRetVal != VS_SUCCESS)
				{
					return (inRetVal);
				}
			}
			
			inRetVal = inFLOW_RunFunction(pobTran, _FUNCTION_ECR_SEND_INQUIRY_RESULT_);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
			
			memcpy(&pobTempTran, pobTran, sizeof(TRANSACTION_OBJECT));
			
			/* 清空ResponseCode，避免使用到卡號查詢的ResponseCode */
			memset(pobTran->srBRec.szRespCode, 0x00, sizeof(pobTran->srBRec.szRespCode));
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				pobTran->srBRec.inFiscIssuerIDLength = 0;
				pobTran->srBRec.inFiscCardCommentLength = 0;
				pobTran->srBRec.inFiscAccountLength = 0;
				memset(pobTran->srBRec.szFiscIssuerID, 0x00, sizeof(pobTran->srBRec.szFiscIssuerID));
				memset(pobTran->srBRec.szFiscCardComment, 0x00, sizeof(pobTran->srBRec.szFiscCardComment));
				memset(pobTran->srBRec.szFiscAccount, 0x00, sizeof(pobTran->srBRec.szFiscAccount));
                                memset(pobTran->srBRec.szPAN, 0x00, sizeof(pobTran->srBRec.szPAN));
                                pobTran->srBRec.uszContactlessBit = VS_FALSE;
			}
			
			/* 等待資料中 */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_ChineseFont("等待資料中", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
			
			inRetVal = inFLOW_RunFunction(pobTran, _FUNCTION_ECR_RECEIVE_SECOND_TRANSACTION_REQUEST_);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
                        
                        /* 若第一次查詢和第二次交易卡號不同，顯示操作錯誤 */
                        if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				if (memcmp(pobTran->srBRec.szPAN, pobTempTran.srBRec.szPAN, strlen(pobTran->srBRec.szPAN)))
                                {
                                        pobTran->inErrorMsg = _ERROR_CODE_V3_OPERATION_;
                                        pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "查詢前後卡號不一致");
                                        }
                                        vdUtility_SYSFIN_LogMessage(AT, "查詢前後卡號不一致");
                                        
                                        return (VS_ERROR);
                                }
                                
                                if ((pobTran->srBRec.uszContactlessBit == VS_TRUE && pobTempTran.srBRec.uszContactlessBit != VS_TRUE) ||
                                    (pobTran->srBRec.uszContactlessBit != VS_TRUE && pobTempTran.srBRec.uszContactlessBit == VS_TRUE))
                                {
                                        pobTran->inErrorMsg = _ERROR_CODE_V3_OPERATION_;
                                        pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "查詢前後交易方式不一致");
                                        }
                                        vdUtility_SYSFIN_LogMessage(AT, "查詢前後交易方式不一致");
                                        
                                        return (VS_ERROR);
                                }
                                        
			}
			
			inRetVal = inFLOW_RunFunction(pobTran, _NCCC_FUNCTION_CHECK_INQUIRY_ISSUER_AMOUNT_);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
			
		}
		else
		{
			inRetVal = VS_SUCCESS;
		}
	}
	else
	{
		/* 兩段式流程 */
		if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
		{
			if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
			{
				inRetVal = inFLOW_RunFunction(pobTran, _TICKET_INQUIRY_CARD_NO_FLOW_);
				/* 不論最後成功與否都要切回NCCC */
				inNCCC_tSAM_SelectAID_NCCC_Flow();
				if (inRetVal != VS_SUCCESS)
				{
					return (inRetVal);
				}
			}
			else if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				inRetVal = inNCCC_Func_Card_Inquery_Issuer_Bank_Fisc(pobTran);
				if (inRetVal != VS_SUCCESS)
				{
					return (inRetVal);
				}
			}
			else
			{
				inRetVal = inFLOW_RunFunction(pobTran, _NCCC_FUNCTION_CARD_INQUERY_ISSUER_BANK_);
				if (inRetVal != VS_SUCCESS)
				{
					return (inRetVal);
				}
			}
			
			inRetVal = inFLOW_RunFunction(pobTran, _FUNCTION_ECR_SEND_INQUIRY_RESULT_);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
			
			memcpy(&pobTempTran, pobTran, sizeof(TRANSACTION_OBJECT));
			
			/* 清空ResponseCode，避免使用到卡號查詢的ResponseCode */
			memset(pobTran->srBRec.szRespCode, 0x00, sizeof(pobTran->srBRec.szRespCode));
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				pobTran->srBRec.inFiscIssuerIDLength = 0;
				pobTran->srBRec.inFiscCardCommentLength = 0;
				pobTran->srBRec.inFiscAccountLength = 0;
				memset(pobTran->srBRec.szFiscIssuerID, 0x00, sizeof(pobTran->srBRec.szFiscIssuerID));
				memset(pobTran->srBRec.szFiscCardComment, 0x00, sizeof(pobTran->srBRec.szFiscCardComment));
				memset(pobTran->srBRec.szFiscAccount, 0x00, sizeof(pobTran->srBRec.szFiscAccount));
                                memset(pobTran->srBRec.szPAN, 0x00, sizeof(pobTran->srBRec.szPAN));
                                pobTran->srBRec.uszContactlessBit = VS_FALSE;
			}
			
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inRetVal = inFLOW_RunFunction(pobTran, _FUNCTION_ECR_RECEIVE_SECOND_TRANSACTION_REQUEST_);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
			
			/* 若第一次查詢和第二次交易卡號不同，顯示操作錯誤 */
                        if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				if (memcmp(pobTran->srBRec.szPAN, pobTempTran.srBRec.szPAN, strlen(pobTran->srBRec.szPAN)))
                                {
                                        pobTran->inErrorMsg = _ERROR_CODE_V3_OPERATION_;
                                        pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "查詢前後卡號不一致");
                                        }
                                        vdUtility_SYSFIN_LogMessage(AT, "查詢前後卡號不一致");
                                        
                                        return (VS_ERROR);
                                }
                                
                                if ((pobTran->srBRec.uszContactlessBit == VS_TRUE && pobTempTran.srBRec.uszContactlessBit != VS_TRUE) ||
                                    (pobTran->srBRec.uszContactlessBit != VS_TRUE && pobTempTran.srBRec.uszContactlessBit == VS_TRUE))
                                {
                                        pobTran->inErrorMsg = _ERROR_CODE_V3_OPERATION_;
                                        pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "查詢前後交易方式不一致");
                                        }
                                        vdUtility_SYSFIN_LogMessage(AT, "查詢前後交易方式不一致");
                                        
                                        return (VS_ERROR);
                                }
                                        
			}
			
		}
		else
		{
			inRetVal = VS_SUCCESS;
		}
	}
        
        if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_034_TK3C_EINVOICE_, _CUSTOMER_INDICATOR_SIZE_))
        {
                if (pobTran->srBRec.inCode == _SALE_)
                {
                        /* Sale要進選擇畫面 */
                        inRetVal = inECR_8N1_Customer_034_TK3C_EInvoice_SelectTransType(pobTran);
                        
                        /* Select失敗 */
                        if (inRetVal != VS_SUCCESS)
                        {
                                return (inRetVal);
                        }
                }
        }
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_079_CINEMARK_, _CUSTOMER_INDICATOR_SIZE_))
        {
                if (pobTran->srBRec.inCode == _SALE_)
                {
                        /* Sale要進選擇畫面 */
                        inRetVal = inECR_7E1_Customer_079_CINEMARK_SelectTransType(pobTran);
                        
                        /* Select失敗 */
                        if (inRetVal != VS_SUCCESS)
                        {
                                return (inRetVal);
                        }
                }
        }
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inECR_Customer_Flow() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :inECR_Save_Request
Date&Time       :2018/12/9 下午 1:49
Describe        :新增查詢上一筆的機制，存起POS的電文
*/
int inECR_Save_Request(char* szBuffer, int inBufferLen)
{
	char		szFileName[20 + 1] = {0};
	unsigned long	ulFileHandle = 0;

	memset(szFileName, 0x00, sizeof(szFileName));
	strcpy(szFileName, _ECR_LAST_SEND_FILENAME_);

	inFILE_Delete((unsigned char*)szFileName);
        
	if (inFILE_Create(&ulFileHandle, (unsigned char*)szFileName) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inECR_Save_Request Create File Error");
		}
		return (VS_ERROR);
	}
	
	inFILE_Seek(ulFileHandle, 0, _SEEK_END_);
	inFILE_Write(&ulFileHandle, (unsigned char*)szBuffer, inBufferLen);
	
	inFILE_Close(&ulFileHandle);

	return (VS_SUCCESS);
}

/*
Function        :inECR_Save_Response
Date&Time       :2018/12/9 下午 1:49
Describe        :新增查詢上一筆的機制，存起回給POS的電文
*/
int inECR_Save_Response(char* szBuffer, int inBufferLen)
{
	char		szFileName[20 + 1] = {0};
	unsigned long	ulFileHandle = 0;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inECR_Save_Response() START !");
	}

	memset(szFileName, 0x00, sizeof(szFileName));
	strcpy(szFileName, _ECR_LAST_RECE_FILENAME_);

	inFILE_Delete((unsigned char*)szFileName);
	if (inFILE_Create(&ulFileHandle, (unsigned char*)szFileName) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inECR_Save_Response Create File Error");
		}
		return (VS_ERROR);
	}
	
	inFILE_Seek(ulFileHandle, 0, _SEEK_END_);
	inFILE_Write(&ulFileHandle, (unsigned char*)szBuffer, inBufferLen);
	
	inFILE_Close(&ulFileHandle);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inECR_Save_Response() END !");
		inLogPrintf(AT, "----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inECR_Inquiry_Last_Transction
Date&Time       :2018/12/9 下午 2:59
Describe        :
*/
int inECR_Inquiry_Last_Transction(TRANSACTION_OBJECT * pobTran, ECR_TABLE *srECROb, char *szBuffer, int inBufferLen)
{
	int		i = 0;
	int		inFd = 0;
	long		lnFileSize = 0;
	long		lnFileRemainSize = 0;
	char		szSendFileName[50 + 1] = {0}, szTemplate[64 + 1] = {0};
	char		szReceFileName[50 + 1] = {0};
	char		szECCFileName[50 + 1] = {0};
	char		szLastSendBuff[_ECR_BUFF_SIZE_ + 1] = {0};
	char		szLastReceBuff[_ECR_BUFF_SIZE_ + 1] = {0};
	char		szHostEnable[2 + 1] = {0};
	unsigned long	ulFileHandle1 = 0;	/* _ECR_LAST_SEND_FILENAME_ */
	unsigned long	ulFileHandle2 = 0;	/* _ECR_LAST_RECE_FILENAME_*/

	memset(szSendFileName, 0x00, sizeof(szSendFileName));
	strcpy(szSendFileName, _ECR_LAST_SEND_FILENAME_);

	if (inFILE_OpenReadOnly(&ulFileHandle1, (unsigned char*)szSendFileName) != VS_SUCCESS)
	{
		pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
		
		return (VS_ERROR);
	}
	
	lnFileSize = lnFILE_GetSize(&ulFileHandle1, (unsigned char*)szSendFileName);
	lnFileRemainSize = lnFileSize;
	
        memset(szLastSendBuff, 0x00, sizeof(szLastSendBuff));
        inFILE_Seek(ulFileHandle1, 0L, _SEEK_BEGIN_);
	i = 0;
	while (1)
	{
		if (lnFileRemainSize > 1024)
		{
			inFILE_Read(&ulFileHandle1, (unsigned char*)&szLastSendBuff[i], 1024);
			lnFileRemainSize -= 1024;
			i += 1024;
		}
		else
		{
			inFILE_Read(&ulFileHandle1, (unsigned char*)&szLastSendBuff[i], lnFileRemainSize);
			lnFileRemainSize -= lnFileRemainSize;
			i += lnFileRemainSize;
			break;
		}
		
	}
	inFILE_Close(&ulFileHandle1);
	
	/* 將Start Trans Type置換到TransType */
	/* 模擬成上一筆ECR的電文 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(&szTemplate[0], &szBuffer[220], 2);

	szBuffer[220] = 0x20;
	szBuffer[221] = 0x20;

	memcpy(&szBuffer[8], &szTemplate[0], 2);

	/* 跟原request電文比對 */
	if (!memcmp(&szBuffer[0], &szLastSendBuff[0], inBufferLen))
	{
		ulFileHandle2 = 0;
		/* 打開讀取上次要回傳的電文是否回應碼為0018，是就走重新感應流程 */
		memset(szReceFileName, 0x00, sizeof(szReceFileName));
		strcpy(szReceFileName, _ECR_LAST_RECE_FILENAME_);

		if (inFILE_OpenReadOnly(&ulFileHandle2, (unsigned char*)szReceFileName) != VS_SUCCESS)
		{
			pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;

			return (VS_ERROR);
		}

		lnFileRemainSize = 0;
		lnFileSize = lnFILE_GetSize(&ulFileHandle2, (unsigned char*)szReceFileName);
		lnFileRemainSize = lnFileSize;

		memset(szLastReceBuff, 0x00, sizeof(szLastReceBuff));
		inFILE_Seek(ulFileHandle2, 0L, _SEEK_BEGIN_);
		i = 0;
		while (1)
		{
			if (lnFileRemainSize > 1024)
			{
				inFILE_Read(&ulFileHandle2, (unsigned char*)&szLastReceBuff[i], 1024);
				lnFileRemainSize -= 1024;
				i += 1024;
			}
			else
			{
				inFILE_Read(&ulFileHandle2, (unsigned char*)&szLastReceBuff[i], lnFileRemainSize);
				lnFileRemainSize -= lnFileRemainSize;
				i += lnFileRemainSize;
				break;
			}

		}
		inFILE_Close(&ulFileHandle2);
		/* 0018:請重新感應卡片/重新掃碼/重試交易    */
		/* 若前一筆回應碼為0018要重啟悠遊卡感應交易 */
		if (memcmp(&szLastReceBuff[76], "0018", 4) == 0)
		{
			memset(szECCFileName, 0x00, sizeof(szECCFileName));
			sprintf(szECCFileName, "%s%s", _ECC_FOLDER_PATH_, _ECC_API_REQ_RETRY_FILE_);
			if (inFile_Linux_Open(&inFd, szECCFileName) != VS_SUCCESS)
			{
				/* 上一筆回0018但是沒CMAS檔案 回錯誤 */
				inFile_Linux_Close(inFd);
				pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;		/* 操作錯誤 & 交易流程有誤 */
				
				return (VS_ERROR);
			}
			else
			{
				/* 開成功代表要執行悠遊卡Retry */
				inFile_Linux_Close(inFd);
				
				/* 關檔ECRLAST.txt並移除 */
				inFILE_Delete((unsigned char*)szSendFileName);

				/* 開新檔重存ECR Request電文 */
				inECR_Save_Request(szBuffer, inBufferLen);

				/* 看有沒有票證的Host */
				memset(szHostEnable, 0x00, sizeof(szHostEnable));
				inNCCC_Ticket_GetESVC_Enable(pobTran->srBRec.inHDTIndex, szHostEnable);
				if (memcmp(szHostEnable, "Y", strlen("Y")) != 0)
				{
					pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;		/* 操作錯誤 & 交易流程有誤 */
					return (VS_ERROR);
				}
				
				return (VS_TAP_AGAIN);
			}
		}
		/* 比對成功，回上一筆回傳ECR電文 */
		else
		{
			return (VS_SUCCESS);
		}
	}
	/* 比對失敗，回操作錯誤 */
	else
	{
		/* (需求單-111155)-電子錢包業者收付訊息整合平台需求 by Russell 2023/8/24 下午 5:40 */
		/* "註8.	當收銀機發動’查詢前筆交易別狀態’予EDC，EDC查詢前筆交易為電子錢包(不論成功或失敗交易)，EDC須回覆’0010’予收銀機。" */
		if (szLastSendBuff[10] == 'W')
		{
			pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_FORMAT_CHECK_ERROR_;
		}
		else
		{
			/* 簡汶婷(Natasha Chien) <natasha.chien@nccc.com.tw> 2018/10/2 (週二) 上午 11:09 */
			/* 通展部回覆：若POS送錯交易別或金額，偏向POS操作錯誤，故請回傳0004操作錯誤。 */
			pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;		/* 操作錯誤 & 交易流程有誤 */
		}
		
		return (VS_ERROR);
	}
}

/*
Function        :inECR_098_Inquiry_Last_Transction
Date&Time       :2018/12/9 下午 2:59
Describe        :
*/
int inECR_098_Inquiry_Last_Transction(TRANSACTION_OBJECT * pobTran, ECR_TABLE *srECROb, char *szBuffer, int inBufferLen)
{
	int		i = 0;
	int		inFd = 0;
	long		lnFileSize = 0;
	long		lnFileRemainSize = 0;
	char		szSendFileName[50 + 1] = {0};
	char		szReceFileName[50 + 1] = {0};
	char		szECCFileName[50 + 1] = {0};
	char		szLastSendBuff[_ECR_BUFF_SIZE_ + 1] = {0};
	char		szLastReceBuff[_ECR_BUFF_SIZE_ + 1] = {0};
	char		szHostEnable[2 + 1] = {0};
	unsigned long	ulFileHandle1 = 0;	/* _ECR_LAST_SEND_FILENAME_ */
	unsigned long	ulFileHandle2 = 0;	/* _ECR_LAST_RECE_FILENAME_*/

	memset(szSendFileName, 0x00, sizeof(szSendFileName));
	strcpy(szSendFileName, _ECR_LAST_SEND_FILENAME_);

	if (inFILE_OpenReadOnly(&ulFileHandle1, (unsigned char*)szSendFileName) != VS_SUCCESS)
	{
		pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
		
		return (VS_ERROR);
	}
	
	lnFileSize = lnFILE_GetSize(&ulFileHandle1, (unsigned char*)szSendFileName);
	lnFileRemainSize = lnFileSize;
	
        memset(szLastSendBuff, 0x00, sizeof(szLastSendBuff));
        inFILE_Seek(ulFileHandle1, 0L, _SEEK_BEGIN_);
	i = 0;
	while (1)
	{
		if (lnFileRemainSize > 1024)
		{
			inFILE_Read(&ulFileHandle1, (unsigned char*)&szLastSendBuff[i], 1024);
			lnFileRemainSize -= 1024;
			i += 1024;
		}
		else
		{
			inFILE_Read(&ulFileHandle1, (unsigned char*)&szLastSendBuff[i], lnFileRemainSize);
			lnFileRemainSize -= lnFileRemainSize;
			i += lnFileRemainSize;
			break;
		}
		
	}
	inFILE_Close(&ulFileHandle1);  
        
	/* 跟原request電文比對Unique欄位 */
	if (!memcmp(&szBuffer[174], &szLastSendBuff[174], 10))
	{
		ulFileHandle2 = 0;
		/* 打開讀取上次要回傳的電文是否回應碼為0018，是就走重新感應流程 */
		memset(szReceFileName, 0x00, sizeof(szReceFileName));
		strcpy(szReceFileName, _ECR_LAST_RECE_FILENAME_);

		if (inFILE_OpenReadOnly(&ulFileHandle2, (unsigned char*)szReceFileName) != VS_SUCCESS)
		{
			pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;

			return (VS_ERROR);
		}

		lnFileRemainSize = 0;
		lnFileSize = lnFILE_GetSize(&ulFileHandle2, (unsigned char*)szReceFileName);
		lnFileRemainSize = lnFileSize;

		memset(szLastReceBuff, 0x00, sizeof(szLastReceBuff));
		inFILE_Seek(ulFileHandle2, 0L, _SEEK_BEGIN_);
		i = 0;
		while (1)
		{
			if (lnFileRemainSize > 1024)
			{
				inFILE_Read(&ulFileHandle2, (unsigned char*)&szLastReceBuff[i], 1024);
				lnFileRemainSize -= 1024;
				i += 1024;
			}
			else
			{
				inFILE_Read(&ulFileHandle2, (unsigned char*)&szLastReceBuff[i], lnFileRemainSize);
				lnFileRemainSize -= lnFileRemainSize;
				i += lnFileRemainSize;
				break;
			}

		}
		inFILE_Close(&ulFileHandle2);
		
		/* 若前一筆回應碼為0018要重啟悠遊卡感應交易 */
		if (memcmp(&szLastReceBuff[76], "0018", 4) == 0)
		{
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "inECR_098_Inquiry_Last_Transction() is 0018");
                        }
                        
			memset(szECCFileName, 0x00, sizeof(szECCFileName));
			sprintf(szECCFileName, "%s%s", _ECC_FOLDER_PATH_, _ECC_API_REQ_RETRY_FILE_);
			if (inFile_Linux_Open(&inFd, szECCFileName) != VS_SUCCESS)
			{
				/* 上一筆回0018但是沒CMAS檔案 回錯誤 */
				inFile_Linux_Close(inFd);
				pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;		/* 操作錯誤 & 交易流程有誤 */
				
				return (VS_ERROR);
			}
			else
			{
				/* 開成功代表要執行悠遊卡Retry */
				inFile_Linux_Close(inFd);
				
				/* 關檔ECRLAST.txt並移除 */
				inFILE_Delete((unsigned char*)szSendFileName);
                                
                                /* 使用上一個交易別 */
                                memcpy(&szBuffer[8], &szLastSendBuff[8], 2);
                                
				/* 開新檔重存ECR Request電文 */
				inECR_Save_Request(szBuffer, inBufferLen);

				/* 看有沒有票證的Host */
				memset(szHostEnable, 0x00, sizeof(szHostEnable));
				inNCCC_Ticket_GetESVC_Enable(pobTran->srBRec.inHDTIndex, szHostEnable);
				if (memcmp(szHostEnable, "Y", strlen("Y")) != 0)
				{
					pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;		/* 操作錯誤 & 交易流程有誤 */
					return (VS_ERROR);
				}
				
				return (VS_TAP_AGAIN);
			}
		}
		/* 比對成功，回上一筆回傳ECR電文 */
		else
		{
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "inECR_098_Inquiry_Last_Transction() is not 0018");
                        }
                        
			return (VS_SUCCESS);
		}
	}
	/* 比對失敗，回操作錯誤 */
	else
	{
		/* 簡汶婷(Natasha Chien) <natasha.chien@nccc.com.tw> 2018/10/2 (週二) 上午 11:09 */
		/* 通展部回覆：若POS送錯交易別或金額，偏向POS操作錯誤，故請回傳0004操作錯誤。 */
		pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;		/* 操作錯誤 & 交易流程有誤 */
		
		return (VS_ERROR);
	}
}

/*
Function        :inECR_No_TRT_Return_Flow
Date&Time       :2018/12/13 上午 9:33
Describe        :For 沒有TRT的流程要找個地方回傳
*/
int inECR_No_TRT_Return_Flow(TRANSACTION_OBJECT * pobTran)
{
	int	inRetVal = VS_ERROR;
	
        vdUtility_SYSFIN_LogMessage(AT, "inECR_No_TRT_Return_Flow START!");
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inECR_No_TRT_Return_Flow() START !");
	}
	
	if (pobTran->inRunTRTID != _TRT_NULL_)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "有TRT");
		}
		return (VS_SUCCESS);
	}
	
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
	}
	
	inRetVal = inECR_Send_Transaction(pobTran);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inECR_No_TRT_Return_Flow() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	/* 即使回傳失敗 */
	return (inRetVal);
}

/*
Function        :inECR_Load_TMK_Initial
Date&Time       :2019/1/10 下午 2:19
Describe        :loadTMK ECR initial
*/
int inECR_Load_TMK_Initial(void)
{
	int	inRetVal = VS_ERROR;
	char	szECRVersion[2 + 1];
	char	szComPort[4 + 1];
	char	szDebugMsg[100 + 1];
	
	memset(&gsrECROb, 0x00, sizeof(ECR_TABLE));
	/* 設定 ECR 版本 */
	memset(szECRVersion, 0x00, sizeof(szECRVersion));
	inGetECRVersion(szECRVersion);
	gsrECROb.srSetting.inVersion = atoi(szECRVersion);
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "ECRVerson :%d", gsrECROb.srSetting.inVersion);
	}
	if (ginISODebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "ECRVerson :%d", gsrECROb.srSetting.inVersion);
		inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
	}
	
	memset(szComPort, 0x00, sizeof(szComPort));
	inGetECRComPort(szComPort);
	if (memcmp(szComPort, _COMPORT_COM1_, strlen(_COMPORT_COM1_)) == 0	||
	    memcmp(szComPort, _COMPORT_COM2_, strlen(_COMPORT_COM2_)) == 0	||
	    memcmp(szComPort, _COMPORT_COM3_, strlen(_COMPORT_COM3_)) == 0	||
	    memcmp(szComPort, _COMPORT_COM4_, strlen(_COMPORT_COM4_)) == 0)
	{
		inRetVal = inRS232_ECR_Load_TMK_Initial();
	}
	else if (memcmp(szComPort, _COMPORT_USB1_, strlen(_COMPORT_USB1_)) == 0)
	{
		inRetVal = VS_ERROR;
	}
	else if (memcmp(szComPort, _COMPORT_USB2_, strlen(_COMPORT_USB2_)) == 0)
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
		gsrECROb.srSetting.uszSettingOK = VS_TRUE;
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inECR_Load_TMK
Date&Time       :2017/11/16 下午 1:29
Describe        :
*/
int inECR_Load_TMK(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;
	char	szComPort[4 + 1];
	
        vdUtility_SYSFIN_LogMessage(AT, "inECR_Load_TMK START!");
        
	memset(szComPort, 0x00, sizeof(szComPort));
	inGetECRComPort(szComPort);
	if (memcmp(szComPort, _COMPORT_COM1_, strlen(_COMPORT_COM1_)) == 0	||
	    memcmp(szComPort, _COMPORT_COM2_, strlen(_COMPORT_COM2_)) == 0	||
	    memcmp(szComPort, _COMPORT_COM3_, strlen(_COMPORT_COM3_)) == 0	||
	    memcmp(szComPort, _COMPORT_COM4_, strlen(_COMPORT_COM4_)) == 0)
	{
		inRetVal = inRS232_ECR_Load_TMK_From_Master(pobTran);
	}
	else if (memcmp(szComPort, _COMPORT_USB1_, strlen(_COMPORT_USB1_)) == 0)
	{
		inRetVal = VS_ERROR;
	}
	else if (memcmp(szComPort, _COMPORT_USB2_, strlen(_COMPORT_USB2_)) == 0)
	{
		inRetVal = VS_ERROR;
	}
	else if (memcmp(szComPort, _COMPORT_WIFI_, strlen(_COMPORT_WIFI_)) == 0)
	{
		inRetVal = VS_ERROR;
	}
	else
	{
		inRetVal = VS_SUCCESS;
	}
	
	return (inRetVal);
}

/*
Function        :inECR_Send_TMK
Date&Time       :2019/1/8 下午 5:40
Describe        :
*/
int inECR_Send_TMK(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;
	char	szComPort[4 + 1] = {0};
	
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
	}
	
	memset(szComPort, 0x00, sizeof(szComPort));
	inGetECRComPort(szComPort);
	if (memcmp(szComPort, _COMPORT_COM1_, strlen(_COMPORT_COM1_)) == 0	||
	    memcmp(szComPort, _COMPORT_COM2_, strlen(_COMPORT_COM2_)) == 0	||
	    memcmp(szComPort, _COMPORT_COM3_, strlen(_COMPORT_COM3_)) == 0	||
	    memcmp(szComPort, _COMPORT_COM4_, strlen(_COMPORT_COM4_)) == 0)
	{
		inRetVal = inRS232_ECR_Send_TMK(pobTran);
	}
	else if (memcmp(szComPort, _COMPORT_USB1_, strlen(_COMPORT_USB1_)) == 0)
	{
		
	}
	else if (memcmp(szComPort, _COMPORT_USB2_, strlen(_COMPORT_USB2_)) == 0)
	{
		
	}
	else if (memcmp(szComPort, _COMPORT_WIFI_, strlen(_COMPORT_WIFI_)) == 0)
	{
		
	}
	else
	{
		inRetVal = VS_SUCCESS;
	}
	
	return (inRetVal);
}

/*
Function        :inECR_Load_TMK_DeInitial
Date&Time       :2019/1/10 下午 2:58
Describe        :反初始化
*/
int inECR_Load_TMK_DeInitial(void)
{
	int	inRetVal = VS_ERROR;
	char	szComPort[4 + 1];
		
	memset(szComPort, 0x00, sizeof(szComPort));
	inGetECRComPort(szComPort);
	if (memcmp(szComPort, _COMPORT_COM1_, strlen(_COMPORT_COM1_)) == 0	||
	    memcmp(szComPort, _COMPORT_COM2_, strlen(_COMPORT_COM2_)) == 0	||
	    memcmp(szComPort, _COMPORT_COM3_, strlen(_COMPORT_COM3_)) == 0	||
	    memcmp(szComPort, _COMPORT_COM4_, strlen(_COMPORT_COM4_)) == 0)
	{
		inRetVal = inRS232_ECR_Load_TMK_DeInitial();
	}
	else if (memcmp(szComPort, _COMPORT_USB1_, strlen(_COMPORT_USB1_)) == 0)
	{
		inRetVal = VS_ERROR;
	}
	else if (memcmp(szComPort, _COMPORT_USB2_, strlen(_COMPORT_USB2_)) == 0)
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
		gsrECROb.srSetting.uszSettingOK = VS_FALSE;
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inECR_Check_Exception
Date&Time       :2021/12/15 下午 2:05
Describe        :最後檢查例外的地方，在unpack後，在執行OPT前
 *		以UDP為例就是檢查POS IP
*/
int inECR_Check_Exception(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;
	char	szComPort[4 + 1];
	
	/* 不是ECR連線就跳走 */
	if (pobTran->uszECRBit != VS_TRUE)
	{
		return (VS_SUCCESS);
	}
	
	/* UDP和RS232並存，要分開寫 */
	if (gbECR_UDP_TransBit == VS_TRUE)
	{
		/* 確認是否為POS IP的資料 */
		inRetVal = inUDP_CheckFromPOSIP();
		if (inRetVal != VS_SUCCESS)
		{
			pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
			memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff2));
			sprintf(pobTran->szErrorMsgBuff1, "POS IP Not Match");
			pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
			return (inRetVal);
		}
		
		/* 確認是否為和上筆PosTxUniqueNO不同 */
		inRetVal = inUDP_Check_Not_Same_PosTxUniqueNO();
		if (inRetVal != VS_SUCCESS)
		{
			pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
			memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
			sprintf(pobTran->szErrorMsgBuff1, "PosTxUniqueNO Same");
			memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
			sprintf(pobTran->szErrorMsgBuff2, "%s", gsrECROb.srTransData.szPOSTxUniqueNo);
			pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
			return (inRetVal);
		}
	}
	else
	{
		memset(szComPort, 0x00, sizeof(szComPort));
		inGetECRComPort(szComPort);
		if (memcmp(szComPort, _COMPORT_COM1_, strlen(_COMPORT_COM1_)) == 0	||
		    memcmp(szComPort, _COMPORT_COM2_, strlen(_COMPORT_COM2_)) == 0	||
		    memcmp(szComPort, _COMPORT_COM3_, strlen(_COMPORT_COM3_)) == 0	||
		    memcmp(szComPort, _COMPORT_COM4_, strlen(_COMPORT_COM4_)) == 0)
		{
			inRetVal = VS_SUCCESS;
		}
		else if (memcmp(szComPort, _COMPORT_USB1_, strlen(_COMPORT_USB1_)) == 0)
		{
			inRetVal = VS_SUCCESS;
		}
		else if (memcmp(szComPort, _COMPORT_USB2_, strlen(_COMPORT_USB2_)) == 0)
		{
			inRetVal = VS_SUCCESS;
		}
		else if (memcmp(szComPort, _COMPORT_WIFI_, strlen(_COMPORT_WIFI_)) == 0)
		{
			inRetVal = VS_SUCCESS;
		}
		else
		{
			inRetVal = VS_SUCCESS;
		}
	}
	
	return (inRetVal);
}

/*
Function        :inECR_Choose_Way_Flow
Date&Time       :2022/8/1 下午 3:58
Describe        :用來切換非正規ECR的地方(Ex:iPass直連)
*/
int inECR_Choose_Way_Flow(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	
        vdUtility_SYSFIN_LogMessage(AT, "inECR_Choose_Way_Flow START!");
        
	if (pobTran->uszCus096_iPASS_Direct_Bit == VS_TRUE)
	{
		inRetVal = inFLOW_RunOperation(pobTran, _OPERATION_IPASS_READER_);
	}
	else
	{
		inRetVal = inFLOW_RunOperation(pobTran, _OPERATION_ECR_TRANSACTION_NORMAL_);
	}
	
	return (inRetVal);
}

/*
Function        :inECR_8N1_Standard_Unpack
Date&Time       :2017/11/15 下午 2:44
Describe        :分析收銀機傳來的資料
*/
 int inECR_8N1_Standard_Unpack(TRANSACTION_OBJECT *pobTran, ECR_TABLE* srECROb, char *szDataBuffer)
{
	int	i = 0;
	int	inRetVal = VS_SUCCESS;
	int	inTransType = 0;		/* 電文中的交易別字串轉為數字儲存 */
	int	inCardInquiryTRTType = _CARD_INQUIRY_TYPE_NONE_;
	int	inBarCodeLen = 0;		/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/7 上午 10:28 */
	int	inSize = 0;
	int	inTempTransType = 0;		/* 用來處理啟動卡號查詢流程前面就需要判斷的狀況 */
	int	inHDTIndex = -1;
	char	szTemplate[100 + 1] = {0};
	char	szDebugMsg[100 + 1] = {0};	
	char	szHostLabel[8 + 1] = {0};
	char	szHostEnable[2 + 1] = {0};
	char	szBatchNum[6 + 1] = {0};
	char	szSTANNum[6 + 1] = {0};
	char	szInvoiceNum[6 + 1] = {0};
	char	szCTLSEnable[2 + 1] = {0};
	char	szPayItemFuncEnable[2 + 1] = {0};
	char	szPayItemCodeTemp[5 + 1] = {0};
	char	szTempSendData[_ECR_BUFF_SIZE_ + 1] = {0};
        char	szCustomerIndicator[3 + 1] = {0};
	char	szFiscContactlessEnable[1 + 1] = {0};
	char	szCUPContactlessEnable[1 + 1] = {0};
	
        /* (需求單 - 107276)自助交易標準做法 EDC於發動交易及完成時，需發出提示音 by Russell 2019/3/8 上午 10:46 */
        inDISP_BEEP(1, 0);
                
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
        inGetCustomIndicator(szCustomerIndicator);
                
        if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_055_EINVOICE_, _CUSTOMER_INDICATOR_SIZE_)      ||
            !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_088_TINTIN_, _CUSTOMER_INDICATOR_SIZE_))
        {   
                /* 電子發票功能需要延遲列印簽單 */
                pobTran->uszDelaySendBit = VS_TRUE;
                
                if (gbEIECRBit)
                {     
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memcpy(szTemplate, &szDataBuffer[8], 2);
                        inTransType = atoi(szTemplate);

                        inECR_EI_Flow(pobTran, srECROb, szDataBuffer);
                        return (VS_ERROR);
                }
                else
                {
                        /* 交易中不吃其他交易 */
                        if (pobTran->uszIsTradeBit)
                        {
                                /* 無視交易 */
                                srECROb->srTransData.uszIsResponse = VS_TRUE;
                                return (VS_ERROR);
                        }
                }
        }
	
	/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/10/12 下午 5:55 */
	if (gbBarCodeECRBit == VS_TRUE)
	{
		inSize = _ECR_8N1_1000_CUPQRCODE_Size_;
	}
	else
	{
		inSize = _ECR_8N1_Standard_Data_Size_;
	}
	
	/* 存ECR原始資料 */
	memcpy(srECROb->srTransData.szOrgData, szDataBuffer, inSize);
	/* 把raw data szDataBuffer依據ECR電文規格，更新資料到 srECROb->srTransData */
	inECR_8N1_Standard_Parse_Data(srECROb, szDataBuffer);
	
	/* ECR Indicator :新 ECR連線 Indicator"I""E"(規格新增欄位 )。 */
	/* 【需求單 - 108046】電子發票BIN大於6碼需求 標準400收銀機欄位ECR Indicator新增”E”值 by Russell 2019/7/5 下午 5:55 */
	switch (inTransType)
	{
		default:
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s: %s  ","ECR Indicator", srECROb->srTransData.szECRIndicator);
				inLogPrintf(AT, szDebugMsg);
			}
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(srECROb->srTransData.szECRIndicator) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ","ECR Indicator", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "ECR Indicator Not Exist Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
				return (VS_ERROR);
			}
			
			/* ECR Indicator必為'I' or 'E'，否則error */
			/* 【需求單 - 108046】電子發票BIN大於6碼需求 標準400收銀機欄位ECR Indicator新增”E”值 by Russell 2019/7/5 下午 5:55 */
			/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/8/13 下午 4:10 */
			if (memcmp(&srECROb->srTransData.szECRIndicator[0], "I", 1) != 0	&&
			    memcmp(&srECROb->srTransData.szECRIndicator[0], "E", 1) != 0	&&
			    memcmp(&srECROb->srTransData.szECRIndicator[0], "Q", 1) != 0)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ","ECR Indicator", "Not \"I\",\"E\",\"Q\" Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				/* 根據inTransType 顯示對應的中文，ex:inTransType: _ECR_8N1_SALE_NO_，pobTran->szErrorMsgBuff1:一般交易 */
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "ECR Indicator Not \"I\",\"E\",\"Q\" Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
				return (VS_ERROR);
			}
			break;
	}
	
	/* Trans Type Indicator :支援交易別 :一般交易、補登退貨 ，其餘 交易別，端末機不參考此欄位 。*/
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "%s: %s  ", "Trans Type Indicator", srECROb->srTransData.szTransTypeIndicator);
		inLogPrintf(AT, szDebugMsg);
	}
				
	/* Trans Type (交易別) */
	if (gbBarCodeECRBit == VS_TRUE)
	{
		switch (inTransType)
		{
			default:
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, srECROb->srTransData.szTransType, 2);

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Type", szTemplate);
					inLogPrintf(AT, szDebugMsg);
				}

				/* 欄位為M 卻不存在，回傳錯誤 */
				if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s: %s  ", "Trans Type", "Not Exist Error");
						inLogPrintf(AT, szDebugMsg);
					}
					memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
					inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "Trans Type Not Exist Error");
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

					return (VS_ERROR);
				}

				/* 交易類別轉成數字 */
				inTransType = atoi(szTemplate);

				if (inTransType == _ECR_8N1_SETTLEMENT_NO_)
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s%s  ", "掃碼格式", "不支援結帳");
						inLogPrintf(AT, szDebugMsg);
					}
					memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
					sprintf(pobTran->szErrorMsgBuff1, "掃碼格式");
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "不支援結帳");
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
					
					return (VS_ERROR);
				}
				break;
		}
	}
	else
	{
		switch (inTransType)
		{
			default:			
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, srECROb->srTransData.szTransType, 2);

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Type", szTemplate);
					inLogPrintf(AT, szDebugMsg);
				}

				/* 欄位為M 卻不存在，回傳錯誤 */
				if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s: %s  ", "Trans Type", "Not Exist Error");
						inLogPrintf(AT, szDebugMsg);
					}
					memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
					inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "Trans Type Not Exist Error");
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

					return (VS_ERROR);
				}

				/* 交易類別轉成數字 */
				inTransType = atoi(szTemplate);

				/* 要連動結帳 */
				if (inTransType == _ECR_8N1_SETTLEMENT_NO_)
				{
					pobTran->uszAutoSettleBit = VS_TRUE;
				}

				break;
		}
	}
	
	/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/7 上午 10:28 */
	/* 掃碼交易規格 */
	if (gbBarCodeECRBit == VS_TRUE)
	{
		/* inTempTransType目前只用在前面判斷 */
		if (inTransType == _ECR_8N1_START_CARD_NO_INQUIRY_NO_ ||
		    inTransType == _ECR_8N1_INQUIRY_LAST_TRANSACTION_NO_)
		{
			inTempTransType = atoi(srECROb->srTransData.szStartTransType);
		}
		else
		{
			inTempTransType = inTransType;
		}
		/* 確認是哪一種交易 */
		switch (inTempTransType)
		{
			case _ECR_8N1_END_CARD_NO_INQUIRY_NO_ :
			case _ECR_8N1_SALE_NO_:
			case _ECR_8N1_INSTALLMENT_NO_:
			case _ECR_8N1_REDEEM_NO_:
				if (inNCCC_Func_Check_Uny_Format(srECROb->srTransData.szBarCodeData) == VS_TRUE)
				{
					pobTran->srBRec.uszUnyTransBit = VS_TRUE;
					/* Uny交易一定免簽 */
					pobTran->srBRec.uszNoSignatureBit = VS_TRUE;
				}
				else if (srECROb->srTransData.srEW_Data.uszEWTransBit == VS_TRUE)
				{
					pobTran->srEWRec.uszEWTransBit = VS_TRUE;
				}
				else if (srECROb->srTransData.srTRUST_Data.uszTRUSTTransBit == VS_TRUE)
				{
					pobTran->srTrustRec.uszTrustTransBit = VS_TRUE;
				}
				else if (inNCCC_Func_Is_CUP_UPLAN_ONLY(srECROb->srTransData.szBarCodeData, atoi(srECROb->srTransData.szBarCodeLen)) == VS_TRUE)
				{
					/* (【需求單-113200】CASV3標準版支援銀聯優計劃專案 by Russell 2024/8/27 上午 11:03 */
					pobTran->srBRec.uszUPlanECRBit = VS_TRUE;
					/* 銀聯退貨不送UP Table */
					if (inTempTransType == _ECR_8N1_REFUND_NO_ &&
					    !memcmp(&srECROb->srTransData.szField_05[0], "C", 1))
					{
						
					}
					else
					{
						pobTran->srBRec.uszUPlanTransBit = VS_TRUE;
						memset(pobTran->srBRec.szUPlan_Coupon, 0x00, sizeof(pobTran->srBRec.szUPlan_Coupon));
						memcpy(&pobTran->srBRec.szUPlan_Coupon[0], &srECROb->srTransData.szBarCodeData[0], (atoi(srECROb->srTransData.szBarCodeLen)));
					}
				}
				else if (inNCCC_Func_Analyse_UPlan_QRCODE(pobTran, srECROb->srTransData.szBarCodeData, atoi(srECROb->srTransData.szBarCodeLen)) == VS_SUCCESS)
				{
					/* (【需求單-113200】CASV3標準版支援銀聯優計劃專案 by Russell 2024/8/27 上午 11:03 */
					pobTran->srBRec.uszUPlanECRBit = VS_TRUE;
					pobTran->srBRec.uszCUPEMVQRCodeBit = VS_TRUE;

					/* QRCode支付免簽名 */
					if (inTempTransType == _ECR_8N1_REFUND_NO_ &&
					    !memcmp(&srECROb->srTransData.szField_05[0], "C", 1))
					{
						/* 退貨要簽 */
						pobTran->srBRec.uszNoSignatureBit = VS_FALSE;
					}
					else
					{
						
						pobTran->srBRec.uszNoSignatureBit = VS_TRUE;
					}
					
                                        if (strlen(pobTran->srBRec.szUPlan_Coupon) > 0)
                                        {
                                                /* 銀聯退貨不送UP Table */
                                                if (inTempTransType == _ECR_8N1_REFUND_NO_ &&
						    !memcmp(&srECROb->srTransData.szField_05[0], "C", 1))
						{

						}
                                                else
                                                {
                                                        pobTran->srBRec.uszUPlanTransBit = VS_TRUE;
                                                }
                                        }
				}
				else
				{
					/* 接收資料錯誤 */
					memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
					inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "BarCode Data Error");
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

					return (VS_ERROR);
				}
				break;
			case _ECR_8N1_REFUND_NO_:
			case _ECR_8N1_INSTALLMENT_REFUND_NO_:
			case _ECR_8N1_REDEEM_REFUND_NO_:
				if (strlen(srECROb->srTransData.szUnyTransCode) > 0)
				{
					pobTran->srBRec.uszUnyTransBit = VS_TRUE;
					/* Uny交易一定免簽 */
					pobTran->srBRec.uszNoSignatureBit = VS_TRUE;
				}
				else if (srECROb->srTransData.srEW_Data.uszEWTransBit == VS_TRUE)
				{
					pobTran->srEWRec.uszEWTransBit = VS_TRUE;
				}
				else if (srECROb->srTransData.srTRUST_Data.uszTRUSTTransBit == VS_TRUE)
				{
					/* 接收資料錯誤 */
					memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
					inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "信託交易別錯誤");
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

					return (VS_ERROR);
				}
				else if (inNCCC_Func_Is_CUP_UPLAN_ONLY(srECROb->srTransData.szBarCodeData, atoi(srECROb->srTransData.szBarCodeLen)) == VS_TRUE)
				{
					/* (【需求單-113200】CASV3標準版支援銀聯優計劃專案 by Russell 2024/8/27 上午 11:03 */
					pobTran->srBRec.uszUPlanECRBit = VS_TRUE;
					/* 銀聯退貨不送UP Table */
					if (inTempTransType == _ECR_8N1_REFUND_NO_ &&
					    !memcmp(&srECROb->srTransData.szField_05[0], "C", 1))
					{
						
					}
					else
					{
						pobTran->srBRec.uszUPlanTransBit = VS_TRUE;
						memset(pobTran->srBRec.szUPlan_Coupon, 0x00, sizeof(pobTran->srBRec.szUPlan_Coupon));
						memcpy(&pobTran->srBRec.szUPlan_Coupon[0], &srECROb->srTransData.szBarCodeData[0], (atoi(srECROb->srTransData.szBarCodeLen)));
					}
				}
				else if (inNCCC_Func_Analyse_UPlan_QRCODE(pobTran, srECROb->srTransData.szBarCodeData, atoi(srECROb->srTransData.szBarCodeLen)) == VS_SUCCESS)
				{
					/* (【需求單-113200】CASV3標準版支援銀聯優計劃專案 by Russell 2024/8/27 上午 11:03 */
					pobTran->srBRec.uszUPlanECRBit = VS_TRUE;
					pobTran->srBRec.uszCUPEMVQRCodeBit = VS_TRUE;
					
					/* QRCode支付免簽名 */
					if (inTempTransType == _ECR_8N1_REFUND_NO_ &&
					    !memcmp(&srECROb->srTransData.szField_05[0], "C", 1))
					{
						/* 退貨要簽 */
						pobTran->srBRec.uszNoSignatureBit = VS_FALSE;
					}
					else
					{
						
						pobTran->srBRec.uszNoSignatureBit = VS_TRUE;
					}
					
                                        if (strlen(pobTran->srBRec.szUPlan_Coupon) > 0)
                                        {
                                                /* 銀聯退貨不送UP Table */
                                                if (inTempTransType == _ECR_8N1_REFUND_NO_ &&
						    !memcmp(&srECROb->srTransData.szField_05[0], "C", 1))
						{

						}
                                                else
                                                {
                                                        pobTran->srBRec.uszUPlanTransBit = VS_TRUE;
                                                }
                                        }
				}
				else
				{
					/* 接收資料錯誤 */
					memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
					inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "No Üny TransCode");
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

					return (VS_ERROR);
				}
				break;
			case _ECR_8N1_VOID_NO_:
				/* 直接撈原交易出來看是否是Uny交易 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, srECROb->srTransData.szHostID, 2);
				
				memset(szHostLabel, 0x00, sizeof(szHostLabel));
				if (memcmp(szTemplate, _ECR_8N1_NCCC_HOSTID_NCCC_, 2) == 0)
				{
					memcpy(szHostLabel, _HOST_NAME_CREDIT_NCCC_, strlen(_HOST_NAME_CREDIT_NCCC_));
				}
				else if (memcmp(szTemplate, _ECR_8N1_NCCC_HOSTID_DCC_, 2) == 0)
				{
					memcpy(szHostLabel, _HOST_NAME_DCC_, strlen(_HOST_NAME_DCC_));
				}
				/* 2018/5/8 下午 2:10 DFS需求不再使用大來主機 */
				else if (memcmp(szTemplate, _ECR_8N1_NCCC_HOSTID_LOYALTY_REDEEM_, 2) == 0)
				{
					memcpy(szHostLabel, _HOST_NAME_CREDIT_NCCC_, strlen(_HOST_NAME_CREDIT_NCCC_));
				}
				else if (memcmp(szTemplate, _ECR_8N1_NCCC_HOSTID_HG_, 2) == 0)
				{
					memcpy(szHostLabel, _HOST_NAME_HG_, strlen(_HOST_NAME_HG_));
				}
				else if (memcmp(szTemplate, _ECR_8N1_NCCC_HOSTID_TRUST_, 2) == 0)
				{
					memcpy(szHostLabel, _HOST_NAME_TRUST_, strlen(_HOST_NAME_TRUST_));
					pobTran->srTrustRec.uszTrustTransBit = VS_TRUE;
				}
				else
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "無對應Host ID");
					}
					memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
					inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "%s : HOST ID Not Pair", szTemplate);
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

					return (VS_ERROR);
				}

				/* 初始化 */
				i = 0;
				do
				{
					/* 按順序load每一個HOST */
					if (inLoadHDTRec(i) < 0)
					{
						/* 如果return VS_ERROR 代表table沒有該HOST */
						memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
						inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
						memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
						sprintf(pobTran->szErrorMsgBuff2, "%s : HOST ID Not Found", szHostLabel);
						pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

						return (VS_ERROR);
					}

					memset(szTemplate, 0x00, sizeof(szTemplate));
					inGetHostLabel(szTemplate);
					if (!memcmp(szTemplate, szHostLabel, strlen(szHostLabel)))
					{
						/* 比對成功後，判斷HOST是否開啟  */
						memset(szHostEnable, 0x00, sizeof(szHostEnable));
						inGetHostEnable(szHostEnable);
						if (szHostEnable[0] != 'Y')
						{
							/* Host沒開 */
							if (ginDebug == VS_TRUE) 
							{
								memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
								sprintf(szDebugMsg, "%s Not Open Error", szHostLabel);
								inLogPrintf(AT, szDebugMsg);
							}

							memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
							inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
							memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
							sprintf(pobTran->szErrorMsgBuff2, "%s : HOST ID Not Open", szHostLabel);
							pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

							/* 如果return VS_ERROR 代表Host沒開 */
							return (VS_ERROR);
						}
						else
						{
							/* Host有開 */
							pobTran->srBRec.inHDTIndex = i;

							/* Load HDPT */
							inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
							memset(szBatchNum, 0x00, sizeof(szBatchNum));
							inGetBatchNum(szBatchNum);
							pobTran->srBRec.lnBatchNum = atol(szBatchNum);

							if (ginDebug == VS_TRUE)
							{
								memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
								sprintf(szDebugMsg, "%s Open", szHostLabel);
								inLogPrintf(AT, szDebugMsg);
							}

						}/* Host Enable比對End */

						break;
					}/* Host Label比對 End */
					i++;

				} while (1);
				
				if (pobTran->srTrustRec.uszTrustTransBit == VS_TRUE)
				{
                                        /* 不撈值 */
				}
				else
				{
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memcpy(&szTemplate[0], srECROb->srTransData.szReceiptNo, 6);

					pobTran->srBRec.lnOrgInvNum = atol(szTemplate);
					inBATCH_GetTransRecord_By_Sqlite(pobTran);
				}
				
				break;
			case _ECR_8N1_EW_INQUIRY_TRANSACTION_NO_:
				pobTran->srEWRec.uszEWTransBit = VS_TRUE;
				break;
			case _ECR_8N1_REPRINT_RECEIPT_NO_:
				if (srECROb->srTransData.srTRUST_Data.uszTRUSTTransBit == VS_TRUE)
				{
					pobTran->srTrustRec.uszTrustTransBit = VS_TRUE;
				}
				break;
			default:
				break;
		}
	}
	
	/* (需求單 - 107227)邦柏科技自助作業客製化 查詢上一筆機制 by Russell 2018/12/8 下午 2:18 */
	if (inTransType == _ECR_8N1_INQUIRY_LAST_TRANSACTION_NO_)
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_REVIEW_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第一層顯示 <交易查詢> */
		
		/* 複製一份Send Data進去比對 */
		memset(szTempSendData, 0x00, sizeof(szTempSendData));
		memcpy(szTempSendData, szDataBuffer, inSize);
		
		inRetVal = inECR_Inquiry_Last_Transction(pobTran, srECROb, szTempSendData, inSize);

		/* 如果比對失敗，回操作錯誤 */
		if (inRetVal == VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "查詢上一筆流程");
			}
			pobTran->uszLastTranscationBit = VS_TRUE;
		}
		else if (inRetVal == VS_TAP_AGAIN)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "TAP AGAIN流程");
			}
			/* 0018 重新感應用成之前的ECR電文 */
			/* 主要是前面的交易類別：查詢上一筆要轉換回原交易別，並用pobTran->uszLastTranscationBit來識別悠遊卡感應要用舊資料，
			    OPT跑特殊流程，TRT則跑正常流程 */
			/*
			 * 這邊因為trans typr = 62，
			 * 所以在上述的inECR_Inquiry_Last_Transction會把start trans type的值放到trans type，
			 * 這邊的sprintf類似strcpy更新szDataBuffer的值是szTempSendData。
			 */
			sprintf(szDataBuffer, szTempSendData);
			memset(srECROb->srTransData.szTransType, 0x00, sizeof(srECROb->srTransData.szTransType));
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &szDataBuffer[8], 2);
			memcpy(srECROb->srTransData.szTransType, szTemplate, 2);
			/* 交易類別轉成數字 */
			inTransType = atoi(srECROb->srTransData.szTransType);
			
			pobTran->uszLastTranscationBit = VS_TRUE;
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "比對失敗");
			}
			/* 比對失敗，要回操作錯誤 */
			memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
			inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
			memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
			sprintf(pobTran->szErrorMsgBuff2, "查詢上一筆比對失敗");
			pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
			return (VS_ERROR);
		}
	}
	else if (inTransType == _ECR_8N1_START_CARD_NO_INQUIRY_NO_)
	{
		/* 不是查詢上一筆的話 移除Retry備份REQ */
		if (!memcmp(&srECROb->srTransData.szField_05[0], "E", 1))
		{
			inFile_Unlink_File(_ECC_API_REQ_RETRY_FILE_, _ECC_FOLDER_PATH_);
		}
	}
	else
	{
		/* 要另外存ECR Request電文 提供查詢上一筆時檢核用 */
		inECR_Save_Request(szDataBuffer, inSize);

		/* 不是查詢上一筆的話 移除Retry備份REQ */
		if (!memcmp(&srECROb->srTransData.szField_05[0], "E", 1))
		{
			inFile_Unlink_File(_ECC_API_REQ_RETRY_FILE_, _ECC_FOLDER_PATH_);
		}
	}
	
	/* CUP Indicator */
	/* 若前一次為卡號查詢，標記為第二次ECR */
	if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
	{
		pobTran->uszCardInquiryFirstBit = VS_FALSE;
		pobTran->uszCardInquirysSecondBit = VS_TRUE;
		/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/28 下午 2:42 */
		/* 避免兩段式帶上卡號查詢的交易碼 */
		if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
		{
			memset(pobTran->srBRec.szUnyTransCode, 0x00, sizeof(pobTran->srBRec.szUnyTransCode));
			memset(pobTran->srBRec.szUnyMaskedCardNo, 0x00, sizeof(pobTran->srBRec.szUnyMaskedCardNo));
			memset(pobTran->srBRec.szCheckNO, 0x00, sizeof(pobTran->srBRec.szCheckNO));
			memset(pobTran->srBRec.szEInvoiceHASH, 0x00, sizeof(pobTran->srBRec.szEInvoiceHASH));
			memset(pobTran->srBRec.szUnyCardLabelLen, 0x00, sizeof(pobTran->srBRec.szUnyCardLabelLen));
			memset(pobTran->srBRec.szUnyCardLabel, 0x00, sizeof(pobTran->srBRec.szUnyCardLabel));
		}
		
		/* 重新決定TRT流程 */
                if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
                {
                        /* 金融卡查詢要二次過卡，不依循前次查詢界面 */
                        inCardInquiryTRTType = _CARD_INQUIRY_TYPE_NONE_;
                }
                else
                {
                        if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
                        {
                                inCardInquiryTRTType = _CARD_INQUIRY_TYPE_CHIP_;
                        }
                        else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
                        {
                                inCardInquiryTRTType = _CARD_INQUIRY_TYPE_CTLS_;
                        }
                        else
                        {
                                inCardInquiryTRTType = _CARD_INQUIRY_TYPE_MEG_;
                        }
                }
	}
	
	/* 啟動卡號查詢不受Trans Indicator影響 */
	if (inTransType == _ECR_8N1_START_CARD_NO_INQUIRY_NO_)
	{
		/* 卡號查詢 */
		inTransType = atoi(srECROb->srTransData.szStartTransType);
		switch (inTransType)
		{
			case _ECR_8N1_SALE_NO_ :
			case _ECR_8N1_REFUND_NO_ :
			case _ECR_8N1_PREAUTH_NO_ :
			case _ECR_8N1_PREAUTH_COMPLETE_NO_ :
			case _ECR_8N1_ESVC_BALANCE_INQUIRY_NO_:
			case _ECR_8N1_ESVC_TOP_UP_NO_:
			case _ECR_8N1_ESVC_VOID_TOP_UP_NO_:
			case _ECR_8N1_END_CARD_NO_INQUIRY_NO_:
				if (pobTran->uszCardInquirysSecondBit == VS_TRUE)
				{
					
				}
				else
				{
					pobTran->uszCardInquiryFirstBit = VS_TRUE;
				}
				/* 有值影響到後面判斷  確保都是空格 */
                                memcpy(&srECROb->srTransData.szTransTypeIndicator[0], " ", 1);
				break;
			case _ECR_8N1_OFFLINE_NO_ :
			case _ECR_8N1_INSTALLMENT_NO_ :
			case _ECR_8N1_REDEEM_NO_ :
			case _ECR_8N1_INSTALLMENT_REFUND_NO_ :
			case _ECR_8N1_REDEEM_REFUND_NO_ :
			case _ECR_8N1_INSTALLMENT_ADJUST_NO_ :
			case _ECR_8N1_REDEEM_ADJUST_NO_ :
				if (pobTran->uszCardInquirysSecondBit == VS_TRUE)
				{
					
				}
				else
				{
					pobTran->uszCardInquiryFirstBit = VS_TRUE;
				}
                                memcpy(&srECROb->srTransData.szField_05[0], " ", 1);
                                memcpy(&srECROb->srTransData.szTransTypeIndicator[0], " ", 1);
				break;
			default :
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "非可執行卡號查詢交易別");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				return (VS_ERROR);
		}
		
		switch (inTransType)
		{
			case _ECR_8N1_START_CARD_NO_INQUIRY_NO_:	/* 啟動卡號查詢 */
			case _ECR_8N1_SALE_NO_:				/* 一般交易 */
			case _ECR_8N1_REFUND_NO_:			/* 退貨交易 */
			case _ECR_8N1_VOID_NO_:				/* 取消 */
			case _ECR_8N1_PREAUTH_NO_:			/* 預先授權 */
			case _ECR_8N1_PREAUTH_COMPLETE_NO_:		/* 預先授權完成 */
			case _ECR_8N1_ESVC_TOP_UP_NO_:			/* 電子票證加值 */
			case _ECR_8N1_ESVC_BALANCE_INQUIRY_NO_:		/* 電子票證餘額查詢 */
			case _ECR_8N1_ESVC_VOID_TOP_UP_NO_:		/* 電子票證加值取消 */
			case _ECR_8N1_HG_REWARD_SALE_NO_:		/* HG紅利積點 */
			case _ECR_8N1_HG_ONLINE_REDEEMPTION_NO_:	/* HG點數抵扣 */
			case _ECR_8N1_HG_POINT_CERTAIN_NO_:		/* HG加價購 */
			case _ECR_8N1_PREAUTH_CANCEL_NO_:		/* 預先授權取消 */
			case _ECR_8N1_PREAUTH_COMPLETE_CANCEL_NO_:	/* 預先授權完成取消 */
			case _ECR_8N1_END_CARD_NO_INQUIRY_NO_:		/* 結束卡號查詢 */
				/* 如果是'C'代表是銀聯卡交易 */
				if (memcmp(&srECROb->srTransData.szField_05[0], "C", 1) == 0)
				{
					/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/7 上午 10:28 */
					if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
					{
						/* 註：因Üny 不支援銀聯卡，故針對 Üny 掃碼交易無論
						 *  CUP Indicator 帶什麼值請 EDC 都視為信用卡交易。*/
					}
					else
					{
						pobTran->srBRec.uszCUPTransBit = VS_TRUE;
						/* 銀聯目前沒這些交易別，要回接收資料錯誤 */
						switch (inTransType)
						{
							case _ECR_8N1_INSTALLMENT_NO_:
							case _ECR_8N1_REDEEM_NO_:
							case _ECR_8N1_INSTALLMENT_REFUND_NO_:
							case _ECR_8N1_REDEEM_REFUND_NO_:
							case _ECR_8N1_INSTALLMENT_ADJUST_NO_:
							case _ECR_8N1_REDEEM_ADJUST_NO_:
							case _ECR_8N1_OFFLINE_NO_:
								memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
								inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
								memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
								sprintf(pobTran->szErrorMsgBuff2, "銀聯無此交易別");
								pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
								return (VS_ERROR);
								break;
							default:
								break;
						}
					}
				}

				/* 如果是'S'代表是SmartPay交易 */
				if (memcmp(&srECROb->srTransData.szField_05[0], "S", 1) == 0)
				{
					pobTran->srBRec.uszFiscTransBit = VS_TRUE;
					/* SmartPay不用簽名 */
					pobTran->srBRec.uszNoSignatureBit = VS_TRUE;
				}

				/* 如果是'E'代表是電票交易 */
				if (memcmp(&srECROb->srTransData.szField_05[0], "E", 1) == 0)
				{
					pobTran->srTRec.uszESVCTransBit = VS_TRUE;
					/* 若有票證未結帳，要提示訊息，比照520 2019/8/29 下午 1:46 */
					inNCCC_Ticket_Func_Must_SETTLE_Show_In_Menu();
				}
				break;
			default:
				break;
		}
	}
	else
	{
		/* Trans Type Indicator :支援交易別 :一般交易、補登退貨 ，其餘 交易別，端末機不參考此欄位 。*/
		switch (inTransType)
		{
			case _ECR_8N1_SALE_NO_:				/* 一般交易 */
			case _ECR_8N1_REFUND_NO_:			/* 退貨交易 */
			case _ECR_8N1_OFFLINE_NO_:			/* 交易補登 */
				/* Trans Type Indicator是'S'，要進選擇畫面 */
				if (!memcmp(&srECROb->srTransData.szTransTypeIndicator[0], "S", 1))
				{
					inRetVal = inECR_SelectTransType(srECROb);
					/* 交易類別轉成數字 */
					inTransType = atoi(srECROb->srTransData.szTransType);

					/* Select失敗 */
					if (inRetVal != VS_SUCCESS)
					{
						return (inRetVal);
					}
				}
				break;
			default:
				break;
		}

	
		/* CUP/Smart pay Indicator:銀聯卡 / Smart Pay交易使用
		 * Indicator = 'C'，表示為CUP交易
		 * Indicator = 'N'，表示為一般信用卡交易
		 * Indicator = 'S'，表示為SmartPay交易，(SmartPay的Sale Reversal和Refund，收銀機一定要送'S'。SmartPay的Sale送'N'，因為Sale不需要按Hotkey)
		 */
		switch (inTransType)
		{
			case _ECR_8N1_START_CARD_NO_INQUIRY_NO_:	/* 啟動卡號查詢 */
			case _ECR_8N1_SALE_NO_:				/* 一般交易 */
			case _ECR_8N1_REFUND_NO_:			/* 退貨交易 */
			case _ECR_8N1_VOID_NO_:				/* 取消 */
			case _ECR_8N1_PREAUTH_NO_:			/* 預先授權 */
			case _ECR_8N1_PREAUTH_COMPLETE_NO_:		/* 預先授權完成 */
			case _ECR_8N1_ESVC_TOP_UP_NO_:			/* 電子票證加值 */
			case _ECR_8N1_ESVC_BALANCE_INQUIRY_NO_:		/* 電子票證餘額查詢 */
			case _ECR_8N1_ESVC_VOID_TOP_UP_NO_:		/* 電子票證加值取消 */
			case _ECR_8N1_HG_REWARD_SALE_NO_:		/* HG紅利積點 */
			case _ECR_8N1_HG_ONLINE_REDEEMPTION_NO_:	/* HG點數抵扣 */
			case _ECR_8N1_HG_POINT_CERTAIN_NO_:		/* HG加價購 */
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "CUPIndicator", srECROb->srTransData.szField_05);
					inLogPrintf(AT, szDebugMsg);
				}

				/* 欄位為M 卻不存在，回傳錯誤 */
				if (inFunc_CheckFullSpace(srECROb->srTransData.szField_05) == VS_TRUE)
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s: %s  ", "CUPIndicator", "Not Exist Error");
						inLogPrintf(AT, szDebugMsg);
					}
					memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
					inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "CUPIndicator Not Exist Error");
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

					return (VS_ERROR);
				}

				break;
			case _ECR_8N1_PREAUTH_CANCEL_NO_:		/* 預先授權取消 */
			case _ECR_8N1_PREAUTH_COMPLETE_CANCEL_NO_:	/* 預先授權完成取消 */
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "  CUPIndicator :%s  ", srECROb->srTransData.szField_05);
					inLogPrintf(AT, szDebugMsg);
				}

				/* 欄位為M 卻不存在，回傳錯誤 */
				if (inFunc_CheckFullSpace(srECROb->srTransData.szField_05) == VS_TRUE)
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s: %s  ", "CUPIndicator", "Not Exist Error");
						inLogPrintf(AT, szDebugMsg);
					}
					memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
					inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "CUPIndicator Not Exist Error");
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

					return (VS_ERROR);
				}

				/* 預先授權取消及完成僅支援銀聯卡交易 ，故此二Request之 CUP Indicator欄位僅可放 ”C”*/
				if (memcmp(&srECROb->srTransData.szField_05[0], "C", 1) != 0)
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s: %s  ", "CUPIndicator", "CUP Not\"C\" Error");
						inLogPrintf(AT, szDebugMsg);
					}
					memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
					inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "非銀聯交易別帶CUP Indicator");
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
					return (VS_ERROR);
				}
				break;
			default:
				break;
		}

		switch (inTransType)
		{
			case _ECR_8N1_START_CARD_NO_INQUIRY_NO_:	/* 啟動卡號查詢 */
			case _ECR_8N1_SALE_NO_:				/* 一般交易 */
			case _ECR_8N1_REFUND_NO_:			/* 退貨交易 */
			case _ECR_8N1_VOID_NO_:				/* 取消 */
			case _ECR_8N1_PREAUTH_NO_:			/* 預先授權 */
			case _ECR_8N1_PREAUTH_COMPLETE_NO_:		/* 預先授權完成 */
			case _ECR_8N1_ESVC_TOP_UP_NO_:			/* 電子票證加值 */
			case _ECR_8N1_ESVC_BALANCE_INQUIRY_NO_:		/* 電子票證餘額查詢 */
			case _ECR_8N1_ESVC_VOID_TOP_UP_NO_:		/* 電子票證加值取消 */
			case _ECR_8N1_HG_REWARD_SALE_NO_:		/* HG紅利積點 */
			case _ECR_8N1_HG_ONLINE_REDEEMPTION_NO_:	/* HG點數抵扣 */
			case _ECR_8N1_HG_POINT_CERTAIN_NO_:		/* HG加價購 */
			case _ECR_8N1_PREAUTH_CANCEL_NO_:		/* 預先授權取消 */
			case _ECR_8N1_PREAUTH_COMPLETE_CANCEL_NO_:	/* 預先授權完成取消 */
				/* 如果是'C'代表是銀聯卡交易 */
				if (memcmp(&srECROb->srTransData.szField_05[0], "C", 1) == 0)
				{
					/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/7 上午 10:28 */
					if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
					{
						/* 註：因Üny 不支援銀聯卡，故針對 Üny 掃碼交易無論
						 *  CUP Indicator 帶什麼值請 EDC 都視為信用卡交易。
						 */
					}
					else
					{
						pobTran->srBRec.uszCUPTransBit = VS_TRUE;
						/* 銀聯目前沒這些交易別，要回接收資料錯誤 */
						switch (inTransType)
						{
							case _ECR_8N1_INSTALLMENT_NO_:
							case _ECR_8N1_REDEEM_NO_:
							case _ECR_8N1_INSTALLMENT_REFUND_NO_:
							case _ECR_8N1_REDEEM_REFUND_NO_:
							case _ECR_8N1_INSTALLMENT_ADJUST_NO_:
							case _ECR_8N1_REDEEM_ADJUST_NO_:
							case _ECR_8N1_OFFLINE_NO_:
								memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
								inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
								memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
								sprintf(pobTran->szErrorMsgBuff2, "銀聯無此交易別");
								pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
								return (VS_ERROR);
								break;
							default:
								break;
						}
					}
				}

				/* 如果是'S'代表是SmartPay交易 */
				if (memcmp(&srECROb->srTransData.szField_05[0], "S", 1) == 0)
				{
					pobTran->srBRec.uszFiscTransBit = VS_TRUE;
					/* SmartPay不用簽名 */
					pobTran->srBRec.uszNoSignatureBit = VS_TRUE;
				}

				/* 如果是'E'代表是電票交易 */
				if (memcmp(&srECROb->srTransData.szField_05[0], "E", 1) == 0)
				{
					pobTran->srTRec.uszESVCTransBit = VS_TRUE;
					/* 若有票證未結帳，要提示訊息，比照520 2019/8/29 下午 1:46 */
					inNCCC_Ticket_Func_Must_SETTLE_Show_In_Menu();
				}
				
				/* 如果是'T'代表是信託交易 */
				if (memcmp(&srECROb->srTransData.szField_05[0], "T", 1) == 0)
				{
					pobTran->srTrustRec.uszTrustTransBit = VS_TRUE;
				}
				break;
			default:
				break;
		}
	}
	
	/* 銀行別/付款別 HOST ID*/
	switch (inTransType)
	{
		case _ECR_8N1_VOID_NO_:				/* 取消交易 */
		case _ECR_8N1_TIP_NO_:				/* 小費交易 */
		case _ECR_8N1_PREAUTH_CANCEL_NO_:		/* 預先授權取消 */
		case _ECR_8N1_PREAUTH_COMPLETE_CANCEL_NO_:	/* 預先授權完成取消 */
		case _ECR_8N1_REPRINT_RECEIPT_NO_:		/* 重印簽單 */
		case _ECR_8N1_MENU_REVIEW_DETAIL_NO_:		/* 明細查詢 */
		case _ECR_8N1_MENU_REVIEW_TOTAL_NO_:		/* 總額查詢 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, srECROb->srTransData.szHostID, 2);
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "HOST ID", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "HOST ID Not Exist Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
				return (VS_ERROR);
			}
			
			memset(szHostLabel, 0x00, sizeof(szHostLabel));

			if (memcmp(szTemplate, _ECR_8N1_NCCC_HOSTID_NCCC_, 2) == 0)
			{
				memcpy(szHostLabel, _HOST_NAME_CREDIT_NCCC_, strlen(_HOST_NAME_CREDIT_NCCC_));
			}
			else if (memcmp(szTemplate, _ECR_8N1_NCCC_HOSTID_DCC_, 2) == 0)
			{
				memcpy(szHostLabel, _HOST_NAME_DCC_, strlen(_HOST_NAME_DCC_));
			}
			/* 2018/5/8 下午 2:10 DFS需求不再使用大來主機 */
			else if (memcmp(szTemplate, _ECR_8N1_NCCC_HOSTID_LOYALTY_REDEEM_, 2) == 0)
			{
				memcpy(szHostLabel, _HOST_NAME_CREDIT_NCCC_, strlen(_HOST_NAME_CREDIT_NCCC_));
			}
			else if (memcmp(szTemplate, _ECR_8N1_NCCC_HOSTID_ESVC_, 2) == 0)
			{
				memcpy(szHostLabel, _HOST_NAME_ESVC_, strlen(_HOST_NAME_ESVC_));
				pobTran->srTRec.uszESVCTransBit = VS_TRUE;
			}
			else if (memcmp(szTemplate, _ECR_8N1_NCCC_HOSTID_HG_, 2) == 0)
			{
				memcpy(szHostLabel, _HOST_NAME_HG_, strlen(_HOST_NAME_HG_));
			}
			else if (memcmp(szTemplate, _ECR_8N1_NCCC_HOSTID_TRUST_, 2) == 0)
			{
				memcpy(szHostLabel, _HOST_NAME_TRUST_, strlen(_HOST_NAME_TRUST_));
				pobTran->srTrustRec.uszTrustTransBit = VS_TRUE;
			}
			else
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "無對應Host ID");
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "%s : HOST ID Not Pair", szTemplate);
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
					
				return (VS_ERROR);
			}
			
			/* 初始化 */
			i = 0;
			do
			{
				/* 按順序load每一個HOST */
				if (inLoadHDTRec(i) < 0)
				{
					/* 如果return VS_ERROR 代表table沒有該HOST */
					memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
					inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "%s : HOST ID Not Found", szHostLabel);
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
					
					return (VS_ERROR);
				}

				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetHostLabel(szTemplate);
				if (!memcmp(szTemplate, szHostLabel, strlen(szHostLabel)))
				{
					/* 比對成功後，判斷HOST是否開啟  */
					memset(szHostEnable, 0x00, sizeof(szHostEnable));
					inGetHostEnable(szHostEnable);
					if (szHostEnable[0] != 'Y')
					{
						/* Host沒開 */
						if (ginDebug == VS_TRUE) 
						{
							memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
							sprintf(szDebugMsg, "%s Not Open Error", szHostLabel);
							inLogPrintf(AT, szDebugMsg);
						}
						
						memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
						inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
						memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
						sprintf(pobTran->szErrorMsgBuff2, "%s : HOST ID Not Open", szHostLabel);
						pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
					
						/* 如果return VS_ERROR 代表Host沒開 */
						return (VS_ERROR);
					}
					else
					{
						/* Host有開 */
						pobTran->srBRec.inHDTIndex = i;
						
						/* Load HDPT */
						inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
						memset(szBatchNum, 0x00, sizeof(szBatchNum));
						inGetBatchNum(szBatchNum);
						pobTran->srBRec.lnBatchNum = atol(szBatchNum);
						
						if (ginDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
							sprintf(szDebugMsg, "%s Open", szHostLabel);
							inLogPrintf(AT, szDebugMsg);
						}
						
					}/* Host Enable比對End */

					break;
				}/* Host Label比對 End */
				i++;

			} while (1);
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%d", "HDTindex", pobTran->srBRec.inHDTIndex);
				inLogPrintf(AT, szDebugMsg);
			}
			
			break;
			
		default:
			break;
	}
	
	/* 【需求單-113110】信託資訊交換平台需求 by Russell 2025/10/7 下午 3:48 */
	if (pobTran->srTrustRec.uszTrustTransBit == VS_TRUE) 
	{
		/* 共用收單序號 */
		switch (inTransType)
		{
			case _ECR_8N1_VOID_NO_:				/* 取消交易 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], srECROb->srTransData.srTRUST_Data.szTrustRRN, 12);

				/* 欄位為M 卻不存在，回傳錯誤 */
				if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
				{
					
				}
				else
				{
					memcpy(pobTran->srTrustRec.szTrustRRN, szTemplate, 12);
				}

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s :(%s)", "szTrustRRN", pobTran->srTrustRec.szTrustRRN);
					inLogPrintf(AT, szDebugMsg);
				}
				break;
                        case _ECR_8N1_REPRINT_RECEIPT_NO_:		/* 重印簽單 */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], srECROb->srTransData.srTRUST_Data.szTrustRRN, 12);

				/* 欄位為M 卻不存在，回傳錯誤 */
				if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s: %s  ", "TrustRRN", "Not Exist Error");
						inLogPrintf(AT, szDebugMsg);
					}
					memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
					inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "TrustRRN Not Exist Error");
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
					
					return (VS_ERROR);
				}
				else
				{
					memcpy(pobTran->srTrustRec.szTrustRRN, szTemplate, 12);
				}

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s :(%s)", "szTrustRRN", pobTran->srTrustRec.szTrustRRN);
					inLogPrintf(AT, szDebugMsg);
				}
				break;
		}
	}
	else
	{
		/* EDC簽單序號 Receipt No(InvoiceNumber) */
		switch (inTransType)
		{
			case _ECR_8N1_VOID_NO_:				/* 取消交易 */
			case _ECR_8N1_TIP_NO_:				/* 小費交易 */
			case _ECR_8N1_PREAUTH_CANCEL_NO_:		/* 預先授權取消 */
			case _ECR_8N1_PREAUTH_COMPLETE_CANCEL_NO_:	/* 預先授權完成取消 */
			case _ECR_8N1_REPRINT_RECEIPT_NO_:		/* 重印簽單 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], srECROb->srTransData.szReceiptNo, 6);

				/* 欄位為M 卻不存在，回傳錯誤 */
				if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s: %s  ", "Receipt No", "Not Exist Error");
						inLogPrintf(AT, szDebugMsg);
					}
					memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
					inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "Receipt No Not Exist Error");
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

					return (VS_ERROR);
				}
				else
				{
					pobTran->srBRec.lnOrgInvNum = atol(szTemplate);
				}

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s :%ld", "lnOrgInvNum", pobTran->srBRec.lnOrgInvNum);
					inLogPrintf(AT, szDebugMsg);
				}

				break;
			default:
				break;
		}
	}
	
	/* 處理交易金額 (Trans Amount) */
	switch (inTransType)
	{
		case _ECR_8N1_START_CARD_NO_INQUIRY_NO_:	/* 啟動卡號查詢 */
		case _ECR_8N1_SALE_NO_:				/* 一般交易 */
		case _ECR_8N1_REFUND_NO_:			/* 退貨交易 */
		case _ECR_8N1_OFFLINE_NO_:			/* 交易補登 */
		case _ECR_8N1_PREAUTH_NO_:			/* 預先授權 */
		case _ECR_8N1_PREAUTH_COMPLETE_NO_:		/* 預先授權完成 */
		case _ECR_8N1_INSTALLMENT_NO_:			/* 分期 */
		case _ECR_8N1_INSTALLMENT_REFUND_NO_:		/* 分期退貨 */
		case _ECR_8N1_INSTALLMENT_ADJUST_NO_:		/* 分期調帳 */
		case _ECR_8N1_REDEEM_NO_:			/* 紅利 */		
		case _ECR_8N1_REDEEM_REFUND_NO_:		/* 紅利退貨 */
		case _ECR_8N1_REDEEM_ADJUST_NO_:		/* 紅利調帳 */
		case _ECR_8N1_ESVC_TOP_UP_NO_:			/* 電子票證加值 */
		case _ECR_8N1_ESVC_VOID_TOP_UP_NO_:		/* 電子票證加值取消 */
		case _ECR_8N1_HG_REWARD_SALE_NO_:		/* HG紅利積點 */
		case _ECR_8N1_HG_REWARD_INSTALLMENT_NO_ :	/* 快樂購紅利積點 + 信用卡分期付款 */
		case _ECR_8N1_HG_REWARD_REDEMPTION_NO_ :	/* 快樂購紅利積點 + 信用卡紅利扣抵 */
		case _ECR_8N1_HG_ONLINE_REDEEMPTION_NO_:	/* HG點數抵扣 */
		case _ECR_8N1_HG_POINT_CERTAIN_NO_:		/* HG加價購 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &srECROb->srTransData.szField_10[0], 10);
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Amount", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "Trans Amount Not Exist Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
                                if (pobTran->srTrustRec.uszTrustTransBit == VS_TRUE)
                                {
                                        pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
                                        inECR_SendMirror(pobTran, _MIRROR_MSG_8N1_Standard_OPER_ERR_);
                                }
				
				return (VS_ERROR);
			}
			else if ((!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_076_8_DIGITS_, _CUSTOMER_INDICATOR_SIZE_) && atol(szTemplate) > 99999999) || 
				 (memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_076_8_DIGITS_, _CUSTOMER_INDICATOR_SIZE_) && atol(szTemplate) > 9999999))
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Amount", "Out of bound");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_076_8_DIGITS_, _CUSTOMER_INDICATOR_SIZE_))
				{
					sprintf(pobTran->szErrorMsgBuff2, "金額超過8位數");
				}
				else
				{
					sprintf(pobTran->szErrorMsgBuff2, "金額超過7位數");
				}
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
				
				return (VS_ERROR);
			}
			else
			{
				if (pobTran->srEWRec.uszEWTransBit == VS_TRUE)
				{
					pobTran->srEWRec.lnTxnAmount = atol(szTemplate);
					
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s :%ld", "交易金額", pobTran->srEWRec.lnTxnAmount);
						inLogPrintf(AT, szDebugMsg);
					}
				}
                                else if (pobTran->srTrustRec.uszTrustTransBit == VS_TRUE)
				{
					pobTran->srTrustRec.lnTxnAmount = atol(szTemplate);
					
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s :%ld", "交易金額", pobTran->srTrustRec.lnTxnAmount);
						inLogPrintf(AT, szDebugMsg);
					}
				}
				else
				{
					pobTran->srBRec.lnTxnAmount = atol(szTemplate);
					pobTran->srBRec.lnOrgTxnAmount = atol(szTemplate);
					pobTran->srBRec.lnTotalTxnAmount = atol(szTemplate);
					
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s :%ld", "交易金額", pobTran->srBRec.lnTxnAmount);
						inLogPrintf(AT, szDebugMsg);
					}
				}
			}
			
			if (pobTran->srEWRec.uszEWTransBit == VS_TRUE)
			{
				sprintf(pobTran->srEWRec.szEW_Decimal, "%02d", atoi(&srECROb->srTransData.szField_10[10]));
				
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s :%s", "金額小數位", pobTran->srEWRec.szEW_Decimal);
					inLogPrintf(AT, szDebugMsg);
				}
			}
			
			break;
                case _ECR_8N1_VOID_NO_:
                        /* 僅處理信託取消 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &srECROb->srTransData.szField_10[0], 10);
			
			/* 欄位為M 卻不存在可手動輸入 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{

			}
			else if ((!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_076_8_DIGITS_, _CUSTOMER_INDICATOR_SIZE_) && atol(szTemplate) > 99999999) || 
				 (memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_076_8_DIGITS_, _CUSTOMER_INDICATOR_SIZE_) && atol(szTemplate) > 9999999))
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Amount", "Out of bound");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_076_8_DIGITS_, _CUSTOMER_INDICATOR_SIZE_))
				{
					sprintf(pobTran->szErrorMsgBuff2, "金額超過8位數");
				}
				else
				{
					sprintf(pobTran->szErrorMsgBuff2, "金額超過7位數");
				}
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
				
				return (VS_ERROR);
			}
			else
			{
				if (pobTran->srTrustRec.uszTrustTransBit == VS_TRUE)
				{
					pobTran->srTrustRec.lnTxnAmount = atol(szTemplate);
					
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s :%ld", "交易金額", pobTran->srTrustRec.lnTxnAmount);
						inLogPrintf(AT, szDebugMsg);
					}
				}
			}
                        break;
		default:
			break;
	}
	
	/* 交易日期 Trans Date */
	switch (inTransType)
	{
		case _ECR_8N1_START_CARD_NO_INQUIRY_NO_:	/* 啟動卡號查詢 */
		case _ECR_8N1_END_CARD_NO_INQUIRY_NO_:		/* 結束卡號查詢*/
//		case _ECR_8N1_SALE_NO_:				/* 一般交易 */		/* 不以ECR日期為主，會發生與機器日期不符的狀況 */
		case _ECR_8N1_REFUND_NO_:			/* 退貨交易 */
		case _ECR_8N1_VOID_NO_:				/* 取消交易 */
//		case _ECR_8N1_OFFLINE_NO_:			/* 交易補登 */		/* 不以ECR日期為主，會發生與機器日期不符的狀況 */
		case _ECR_8N1_TIP_NO_:				/* 小費交易 */
//		case _ECR_8N1_PREAUTH_NO_:			/* 預先授權 */		/* 不以ECR日期為主，會發生與機器日期不符的狀況 */
		case _ECR_8N1_PREAUTH_CANCEL_NO_:		/* 預先授權取消 */
		case _ECR_8N1_PREAUTH_COMPLETE_NO_:		/* 預先授權完成 */
		case _ECR_8N1_PREAUTH_COMPLETE_CANCEL_NO_:	/* 預先授權完成取消 */
//		case _ECR_8N1_INSTALLMENT_NO_:			/* 分期 */		/* 不以ECR日期為主，會發生與機器日期不符的狀況 */
		case _ECR_8N1_INSTALLMENT_REFUND_NO_:		/* 分期退貨 */
		case _ECR_8N1_INSTALLMENT_ADJUST_NO_:		/* 分期調帳 */
//		case _ECR_8N1_REDEEM_NO_:			/* 紅利 */		/* 不以ECR日期為主，會發生與機器日期不符的狀況 */	
		case _ECR_8N1_REDEEM_REFUND_NO_:		/* 紅利退貨 */
		case _ECR_8N1_REDEEM_ADJUST_NO_:		/* 紅利調帳 */
		case _ECR_8N1_SETTLEMENT_NO_:			/* 結帳交易 */
		case _ECR_8N1_REPRINT_RECEIPT_NO_:		/* 重印簽單 */
		case _ECR_8N1_MENU_REVIEW_DETAIL_NO_:		/* 明細查詢 */
		case _ECR_8N1_MENU_REVIEW_TOTAL_NO_:		/* 總額查詢 */
		case _ECR_8N1_MENU_REPORT_DETAIL_NO_:		/* 明細列印 */
		case _ECR_8N1_MENU_REPORT_TOTAL_NO_:		/* 總額列印 */
		case _ECR_8N1_ESVC_TOP_UP_NO_:			/* 電子票證加值 */
		case _ECR_8N1_ESVC_BALANCE_INQUIRY_NO_:		/* 電子票證餘額查詢 */
		case _ECR_8N1_ESVC_VOID_TOP_UP_NO_:		/* 電子票證加值取消 */
		case _ECR_8N1_ECHO_NO_:				/* 連線測試 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &srECROb->srTransData.szTransDate[0], 6);
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Date", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
//				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
//				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
//				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
//				sprintf(pobTran->szErrorMsgBuff2, "Trans Date Not Exist Error");
//				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
//				
//				return (VS_ERROR);
			}
			else
			{
				/* 根據安全理由，不採信ECR日期時間 */
			}
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "交易日期", pobTran->srBRec.szDate);
				inLogPrintf(AT, szDebugMsg);
			}
			break;
		default:
			break;
	}
	
	/* 交易時間 Trans Time */
	switch (inTransType)
	{
		case _ECR_8N1_START_CARD_NO_INQUIRY_NO_:	/* 啟動卡號查詢 */
		case _ECR_8N1_END_CARD_NO_INQUIRY_NO_:		/* 結束卡號查詢*/
		case _ECR_8N1_SALE_NO_:				/* 一般交易 */
		case _ECR_8N1_REFUND_NO_:			/* 退貨交易 */
		case _ECR_8N1_VOID_NO_:				/* 取消交易 */
		case _ECR_8N1_OFFLINE_NO_:			/* 交易補登 */
		case _ECR_8N1_TIP_NO_:				/* 小費交易 */
		case _ECR_8N1_PREAUTH_NO_:			/* 預先授權 */
		case _ECR_8N1_PREAUTH_CANCEL_NO_:		/* 預先授權取消 */
		case _ECR_8N1_PREAUTH_COMPLETE_NO_:		/* 預先授權完成 */
		case _ECR_8N1_PREAUTH_COMPLETE_CANCEL_NO_:	/* 預先授權完成取消 */
		case _ECR_8N1_INSTALLMENT_NO_:			/* 分期 */
		case _ECR_8N1_INSTALLMENT_REFUND_NO_:		/* 分期退貨 */
		case _ECR_8N1_INSTALLMENT_ADJUST_NO_:		/* 分期調帳 */
		case _ECR_8N1_REDEEM_NO_:			/* 紅利 */		
		case _ECR_8N1_REDEEM_REFUND_NO_:		/* 紅利退貨 */
		case _ECR_8N1_REDEEM_ADJUST_NO_:		/* 紅利調帳 */
		case _ECR_8N1_SETTLEMENT_NO_:			/* 結帳交易 */
		case _ECR_8N1_REPRINT_RECEIPT_NO_:		/* 重印簽單 */
		case _ECR_8N1_MENU_REVIEW_DETAIL_NO_:		/* 明細查詢 */
		case _ECR_8N1_MENU_REVIEW_TOTAL_NO_:		/* 總額查詢 */
		case _ECR_8N1_MENU_REPORT_DETAIL_NO_:		/* 明細列印 */
		case _ECR_8N1_MENU_REPORT_TOTAL_NO_:		/* 總額列印 */
		case _ECR_8N1_ESVC_TOP_UP_NO_:			/* 電子票證加值 */
		case _ECR_8N1_ESVC_BALANCE_INQUIRY_NO_:		/* 電子票證餘額查詢 */
		case _ECR_8N1_ESVC_VOID_TOP_UP_NO_:		/* 電子票證加值取消 */
		case _ECR_8N1_ECHO_NO_:				/* 連線測試 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szTransTime[0], 6);
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Time", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
//				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
//				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
//				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
//				sprintf(pobTran->szErrorMsgBuff2, "Trans Time Not Exist Error");
//				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
//				
//				return (VS_ERROR);
			}
			else
			{
				/* 根據安全理由，不採信ECR日期時間 */
			}
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "交易時間", pobTran->srBRec.szTime);
				inLogPrintf(AT, szDebugMsg);
			}
			
			break;
			
		default:
			break;
	}
	
	/* 授權碼(Auth Code) ECR電文中給9個字元 但ATS電文只需要6個，所以只抓6個 */
	switch (inTransType)
	{
		case _ECR_8N1_REFUND_NO_:			/* 退貨 */
		case _ECR_8N1_OFFLINE_NO_:			/* 交易補登 */
		case _ECR_8N1_INSTALLMENT_REFUND_NO_:		/* 分期退貨 */
		case _ECR_8N1_INSTALLMENT_ADJUST_NO_:		/* 分期調帳 */
		case _ECR_8N1_REDEEM_REFUND_NO_:		/* 紅利退貨 */
		case _ECR_8N1_REDEEM_ADJUST_NO_:		/* 紅利調帳 */
			/* 處理授權碼(optional) */
			memset(pobTran->srBRec.szAuthCode, 0x00, sizeof(pobTran->srBRec.szAuthCode));
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szApprovalNo[0], 6);
			memcpy(pobTran->srBRec.szAuthCode, szTemplate, 6);
			break;
		case _ECR_8N1_PREAUTH_COMPLETE_NO_:		/* 預先授權完成 */
			/* 處理授權碼(must have) */
			memset(pobTran->srBRec.szAuthCode, 0x00, sizeof(pobTran->srBRec.szAuthCode));
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szApprovalNo[0], 6);
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Auth Code", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "Auth Code Not Exist Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
				return (VS_ERROR);
			}
			else
			{
				memcpy(pobTran->srBRec.szAuthCode, szTemplate, 6);
			}
			
			break;
			
		default :
			break;
	}
	
	/* 其他金額(Exp Amount) */
	switch (inTransType)
	{
		case _ECR_8N1_TIP_NO_:				/* 小費交易 */
			/* 小費金額*/
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetTRTFileName(szTemplate);
			if (!memcmp(szTemplate, _TRT_FILE_NAME_DCC_, 6) && !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TIP_, 2))
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &srECROb->srTransData.szExpAmount[0], 12);
			}
			else
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &srECROb->srTransData.szExpAmount[0], 10);
			}
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Exp Amount", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "Exp Amount Not Exist Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
				return (VS_ERROR);
			}
			
			pobTran->lnOldTaxAmount = atol(szTemplate);
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Tip: %ld", pobTran->lnOldTaxAmount);
				inLogPrintf(AT, szDebugMsg);
			}

			break;
			
		case _ECR_8N1_PREAUTH_COMPLETE_NO_:			/* 預先授權完成 預先授權完成交易之原預先授權金額 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szAuthAmount[0], 10);

			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Exp Amount", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "Exp Amount Not Exist Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
				return (VS_ERROR);
			}

			pobTran->srBRec.lnOrgTxnAmount = atol(szTemplate);
			break;
			
		default :
			break;
	}
	
	/* 處理櫃號(Store ID) */
	switch (inTransType)
	{
		case _ECR_8N1_START_CARD_NO_INQUIRY_NO_:	/* 啟動卡號查詢 */
		case _ECR_8N1_SALE_NO_:				/* 一般交易 */
		case _ECR_8N1_REFUND_NO_:			/* 退貨交易 */
		case _ECR_8N1_OFFLINE_NO_:			/* 交易補登 */
		case _ECR_8N1_PREAUTH_NO_:			/* 預先授權 */
		case _ECR_8N1_PREAUTH_CANCEL_NO_:		/* 預先授權取消 */
		case _ECR_8N1_PREAUTH_COMPLETE_NO_:		/* 預先授權完成 */
		case _ECR_8N1_PREAUTH_COMPLETE_CANCEL_NO_:	/* 預先授權完成取消 */
		case _ECR_8N1_INSTALLMENT_NO_:			/* 分期 */
		case _ECR_8N1_INSTALLMENT_REFUND_NO_:		/* 分期退貨 */
		case _ECR_8N1_INSTALLMENT_ADJUST_NO_:		/* 分期調帳 */
		case _ECR_8N1_REDEEM_NO_:			/* 紅利 */
		case _ECR_8N1_REDEEM_REFUND_NO_:		/* 紅利退貨 */
		case _ECR_8N1_REDEEM_ADJUST_NO_:		/* 紅利調帳 */
		case _ECR_8N1_HG_REWARD_SALE_NO_:		/* HG紅利積點 */
		case _ECR_8N1_HG_REWARD_INSTALLMENT_NO_ :	/* 快樂購紅利積點 + 信用卡分期付款 */
		case _ECR_8N1_HG_REWARD_REDEMPTION_NO_ :	/* 快樂購紅利積點 + 信用卡紅利扣抵 */
		case _ECR_8N1_HG_ONLINE_REDEEMPTION_NO_ :	/* 快樂購點數扣抵 */
		case _ECR_8N1_HG_POINT_CERTAIN_NO_ :		/* 快樂購加價購 */
		case _ECR_8N1_HG_FULL_REDEEMPTION_NO_ :		/* 快樂購點數兌換 */
		case _ECR_8N1_HG_REDEEM_REFUND_NO_ :		/* 快樂購扣抵退貨 */
		case _ECR_8N1_HG_REWARD_REFUND_NO_ :		/* 快樂購回饋退貨 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szStoreId[0], 18);
			
			if (strlen(szTemplate) > 0)
			{
				memcpy(pobTran->srBRec.szStoreID, szTemplate, strlen(szTemplate));
				/* 櫃號不滿18，補空白 */
				if (strlen(szTemplate) < 18)
				{
					memset(&pobTran->srBRec.szStoreID[strlen(szTemplate)], 0x20, 18 - strlen(szTemplate));
				}

			}
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "Store ID", pobTran->srBRec.szStoreID);
				inLogPrintf(AT, szDebugMsg);
			}
			
			break;
		case _ECR_8N1_ESVC_TOP_UP_NO_:			/* 電子票證加值 */
		case _ECR_8N1_ESVC_VOID_TOP_UP_NO_:		/* 電子票證加值取消 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szStoreId[0], 18);
			
			if (strlen(szTemplate) > 0)
			{
				memcpy(pobTran->srBRec.szStoreID, szTemplate, strlen(szTemplate));
				/* 櫃號不滿18，補空白 */
				if (strlen(szTemplate) < 18)
				{
					memset(&pobTran->srBRec.szStoreID[strlen(szTemplate)], 0x20, 18 - strlen(szTemplate));
				}

			}
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "Store ID", pobTran->srBRec.szStoreID);
				inLogPrintf(AT, szDebugMsg);
			}
			
			break;
		default:
			break;
	}
	
	if (inTransType == _ECR_8N1_AWARD_REDEEM_NO_)
	{
		/* 兌換模式 */
		switch (inTransType)
		{
			case _ECR_8N1_AWARD_REDEEM_NO_:		/* 優惠兌換 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &srECROb->srTransData.szField_20[0], 1);	/* (1=條碼兌換，2=卡號兌換) */

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s :%s", "Award Redeem Mode", szTemplate);
					inLogPrintf(AT, szDebugMsg);
				}
				
				/* 條碼兌換 */
				if (memcmp(szTemplate, "1", 1) == 0)
				{
					memcpy(pobTran->szL3_AwardWay, "1", 1);
				}
				else if (memcmp(szTemplate, "2", 1) == 0)
				{
					/* 至EDC過卡流程決定 */
					memcpy(pobTran->szL3_AwardWay, "0", 1);
				}
				else
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "ECR未輸入兌換方式");
					}
					memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
					inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "ECR未輸入兌換方式");
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

					return (VS_ERROR);
				}
				
				/* 兌換方式為條碼才抓條碼*/
				if (memcmp(pobTran->szL3_AwardWay, "1", strlen("1")) == 0)
				{
					if (srECROb->srTransData.szBarCode1[0] == ' ' && srECROb->srTransData.szBarCode2[0] == ' ')
					{
						if (ginDebug == VS_TRUE)
						{
							inLogPrintf(AT, "ECR未輸入條碼");
						}
						memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
						inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
						memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
						sprintf(pobTran->szErrorMsgBuff2, "ECR未輸入條碼");
						pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
						
						return (VS_ERROR);
					}
					
					/* 條碼兌換 防呆&阻擋&存Barcode */
					/* 11 表示只有一個一維條碼，且這是第1 個。 */
					if (srECROb->srTransData.szBarCode1[0] == '1' && srECROb->srTransData.szBarCode1[1] == '1')
					{
						memcpy(&pobTran->szL3_Barcode1[0], &srECROb->srTransData.szBarCode1[0], 20);
						sprintf(pobTran->szL3_Barcode1Len, "%02d", strlen(pobTran->szL3_Barcode1));
					}
					else if (srECROb->srTransData.szBarCode2[0] == '1' && srECROb->srTransData.szBarCode2[0] == '1')
					{
						memcpy(&pobTran->szL3_Barcode1[0], &srECROb->srTransData.szBarCode2[0], 20);
						sprintf(pobTran->szL3_Barcode1Len, "%02d", strlen(pobTran->szL3_Barcode1));
					}
					else if (srECROb->srTransData.szBarCode1[0] == '2' && srECROb->srTransData.szBarCode1[1] == '1')
					{
						if (srECROb->srTransData.szBarCode2[0] == '2' && srECROb->srTransData.szBarCode2[1] == '2')
						{
							memcpy(&pobTran->szL3_Barcode1[0], &srECROb->srTransData.szBarCode1[0], 20);
							sprintf(pobTran->szL3_Barcode1Len, "%02d", strlen(pobTran->szL3_Barcode1));
							memcpy(&pobTran->szL3_Barcode2[0], &srECROb->srTransData.szBarCode2[0], 20);
							sprintf(pobTran->szL3_Barcode2Len, "%02d", strlen(pobTran->szL3_Barcode2));
						}
						else
						{
							if (ginDebug == VS_TRUE)
							{
								inLogPrintf(AT, "ECR條碼邏輯錯誤");
							}
							memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
							inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
							memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
							sprintf(pobTran->szErrorMsgBuff2, "ECR條碼邏輯錯誤");
							pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
						
							return (VS_ERROR);
						}
					}
					else if (srECROb->srTransData.szBarCode1[0] == '2' && srECROb->srTransData.szBarCode1[1] == '2')
					{
						if (srECROb->srTransData.szBarCode2[0] == '2' && srECROb->srTransData.szBarCode2[1] == '1')
						{
							memcpy(&pobTran->szL3_Barcode2[0], &srECROb->srTransData.szBarCode1[0], 20);
							sprintf(pobTran->szL3_Barcode2Len, "%02d", strlen(pobTran->szL3_Barcode2));
							memcpy(&pobTran->szL3_Barcode1[0], &srECROb->srTransData.szBarCode2[0], 20);
							sprintf(pobTran->szL3_Barcode1Len, "%02d", strlen(pobTran->szL3_Barcode1));
						}
						else
						{
							if (ginDebug == VS_TRUE)
							{
								inLogPrintf(AT, "ECR條碼邏輯錯誤");
							}
							memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
							inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
							memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
							sprintf(pobTran->szErrorMsgBuff2, "ECR條碼邏輯錯誤");
							pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
							
							return (VS_ERROR);
						}
					}
					else
					{
						if (ginDebug == VS_TRUE)
						{
							inLogPrintf(AT, "ECR條碼邏輯錯誤");
						}
						memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
						inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
						memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
						sprintf(pobTran->szErrorMsgBuff2, "ECR條碼邏輯錯誤");
						pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
							
						return (VS_ERROR);
					}
				}
				break;
			default:
				break;
		}
	}
	else if (inTransType == _ECR_8N1_VOID_AWARD_REDEEM_NO_)
	{
		/* 兌換模式 */
		switch (inTransType)
		{
			case _ECR_8N1_VOID_AWARD_REDEEM_NO_:		/* 取消優惠兌換 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &srECROb->srTransData.szField_20[0], 1);	/* (1=條碼兌換，2=卡號兌換) */

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s :%s", "Award Redeem Mode", szTemplate);
					inLogPrintf(AT, szDebugMsg);
				}
				
				/* 條碼兌換 */
				if (memcmp(szTemplate, "1", 1) == 0)
				{
					memcpy(pobTran->szL3_AwardWay, "1", 1);
				}
				else
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "ECR取消兌換不接受卡號輸入");
					}
					memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
					inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "ECR取消兌換不接受卡號輸入");
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
							
					return (VS_ERROR);
				}
				
				/* 兌換方式為條碼才抓條碼*/
				if (memcmp(pobTran->szL3_AwardWay, "1", strlen("1")) == 0)
				{
					if (srECROb->srTransData.szBarCode1[0] == ' ' && srECROb->srTransData.szBarCode2[0] == ' ')
					{
						if (ginDebug == VS_TRUE)
						{
							inLogPrintf(AT, "ECR未輸入條碼");
						}
						memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
						inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
						memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
						sprintf(pobTran->szErrorMsgBuff2, "ECR未輸入條碼");
						pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
					
						return (VS_ERROR);
					}
					
					/* 條碼兌換 防呆&阻擋&存Barcode */
					/* 11 表示只有一個一維條碼，且這是第1 個。 */
					if (srECROb->srTransData.szBarCode1[0] == '1' && srECROb->srTransData.szBarCode1[1] == '1')
					{
						memcpy(&pobTran->szL3_Barcode1[0], &srECROb->srTransData.szBarCode1[0], 20);
						sprintf(pobTran->szL3_Barcode1Len, "%02d", strlen(pobTran->szL3_Barcode1));
					}
					else if (srECROb->srTransData.szBarCode2[0] == '1' && srECROb->srTransData.szBarCode2[1] == '1')
					{
						memcpy(&pobTran->szL3_Barcode1[0], &srECROb->srTransData.szBarCode2[0], 20);
						sprintf(pobTran->szL3_Barcode1Len, "%02d", strlen(pobTran->szL3_Barcode1));
					}
					else if (srECROb->srTransData.szBarCode1[0] == '2' && srECROb->srTransData.szBarCode1[1] == '1')
					{
						if (srECROb->srTransData.szBarCode2[0] == '2' && srECROb->srTransData.szBarCode2[1] == '2')
						{
							memcpy(&pobTran->szL3_Barcode1[0], &srECROb->srTransData.szBarCode1[0], 20);
							sprintf(pobTran->szL3_Barcode1Len, "%02d", strlen(pobTran->szL3_Barcode1));
							memcpy(&pobTran->szL3_Barcode2[0], &srECROb->srTransData.szBarCode2[0], 20);
							sprintf(pobTran->szL3_Barcode2Len, "%02d", strlen(pobTran->szL3_Barcode2));
						}
						else
						{
							if (ginDebug == VS_TRUE)
							{
								inLogPrintf(AT, "ECR條碼邏輯錯誤");
							}
							memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
							inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
							memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
							sprintf(pobTran->szErrorMsgBuff2, "ECR條碼邏輯錯誤");
							pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
						
							return (VS_ERROR);
						}
					}
					else if (srECROb->srTransData.szBarCode1[0] == '2' && srECROb->srTransData.szBarCode1[1] == '2')
					{
						if (srECROb->srTransData.szBarCode2[0] == '2' && srECROb->srTransData.szBarCode2[1] == '1')
						{
							memcpy(&pobTran->szL3_Barcode2[0], &srECROb->srTransData.szBarCode1[0], 20);
							sprintf(pobTran->szL3_Barcode2Len, "%02d", strlen(pobTran->szL3_Barcode2));
							memcpy(&pobTran->szL3_Barcode1[0], &srECROb->srTransData.szBarCode2[0], 20);
							sprintf(pobTran->szL3_Barcode1Len, "%02d", strlen(pobTran->szL3_Barcode1));
						}
						else
						{
							if (ginDebug == VS_TRUE)
							{
								inLogPrintf(AT, "ECR條碼邏輯錯誤");
							}
							memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
							inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
							memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
							sprintf(pobTran->szErrorMsgBuff2, "ECR條碼邏輯錯誤");
							pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
							
							return (VS_ERROR);
						}
					}
					else
					{
						if (ginDebug == VS_TRUE)
						{
							inLogPrintf(AT, "ECR條碼邏輯錯誤");
						}
						memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
						inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
						memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
						sprintf(pobTran->szErrorMsgBuff2, "ECR條碼邏輯錯誤");
						pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
							
						return (VS_ERROR);
					}
				}
				break;
			default:
				break;
		}
	}
	else if (srECROb->srTransData.srEW_Data.uszEWTransBit == VS_TRUE)
	{
		switch (inTransType)
		{
			case	_ECR_8N1_SALE_NO_:
			case	_ECR_8N1_REFUND_NO_:
				/* 一般金額/一般退貨金額 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &srECROb->srTransData.srEW_Data.szEW_NormalAmt[0], 10);
				memcpy(pobTran->srEWRec.szEW_NormalAmt, szTemplate, 10);
				/* 代收金額/代收退貨金額 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &srECROb->srTransData.srEW_Data.szEW_CollectionAmt[0], 10);
				memcpy(pobTran->srEWRec.szEW_CollectionAmt, szTemplate, 10);
				/* 代售金額/代售退貨金額 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &srECROb->srTransData.srEW_Data.szEW_ConsignmentAmt[0], 10);
				memcpy(pobTran->srEWRec.szEW_ConsignmentAmt, szTemplate, 10);
				/* 不可折抵金額 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &srECROb->srTransData.srEW_Data.szEW_PromotionRestriction[0], 10);
				memcpy(pobTran->srEWRec.szEW_PromotionRestriction, szTemplate, 10);
				break;
			default:
				break;
		}
		
		/* 特店訂單編號 */
		switch (inTransType)
		{
			case	_ECR_8N1_SALE_NO_:
			case	_ECR_8N1_REFUND_NO_:
			case	_ECR_8N1_EW_INQUIRY_TRANSACTION_NO_:
				/* 欄位為M 卻不存在，回傳錯誤 */
				if (inFunc_CheckFullSpace(srECROb->srTransData.srEW_Data.szEW_OrderId) == VS_TRUE)
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s: %s  ", "EW_OrderId", "Not Exist Error");
						inLogPrintf(AT, szDebugMsg);
					}
					memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
					inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "EW_OrderId Error");
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

					return (VS_ERROR);
				}
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &srECROb->srTransData.srEW_Data.szEW_OrderId[0], 30);
				memcpy(pobTran->srEWRec.szEW_OrderId, szTemplate, 30);
				break;
			default:
				break;
		}
		
		/* 退貨訂單編號 */
		switch (inTransType)
		{
			case	_ECR_8N1_REFUND_NO_:
				/* 欄位為M 卻不存在，回傳錯誤 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &srECROb->srTransData.srEW_Data.szEW_RefundTradeNo[0], 30);
				memcpy(pobTran->srEWRec.szEW_RefundTradeNo, szTemplate, 30);
				break;
			default:
				break;
		}
	}
	else if (pobTran->srTrustRec.uszTrustTransBit == VS_TRUE)
	{
		/* 銷帳編號*/
		switch (inTransType)
		{
			case	_ECR_8N1_SALE_NO_:
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &srECROb->srTransData.srTRUST_Data.szReconciliationNo[0], 20);
				memcpy(pobTran->srTrustRec.szReconciliationNo, szTemplate, 20);
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s :(%s)", "ReconciliationNo", pobTran->srTrustRec.szReconciliationNo);
					inLogPrintf(AT, szDebugMsg);
				}
				break;
			default:
				break;
		}
		/* 信託機構代碼 */
		switch (inTransType)
		{
			case	_ECR_8N1_VOID_NO_:
				/* 欄位為M 卻不存在，回傳錯誤 */
				if (inFunc_CheckFullSpace(srECROb->srTransData.srTRUST_Data.szTrustInstitutionCode) == VS_TRUE)
				{
					
				}
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &srECROb->srTransData.srTRUST_Data.szTrustInstitutionCode[0], 7);
				memcpy(pobTran->srTrustRec.szTrustInstitutionCode, szTemplate, 7);
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s :(%s)", "szTrustInstitutionCode", pobTran->srTrustRec.szTrustInstitutionCode);
					inLogPrintf(AT, szDebugMsg);
				}
				break;
			default:
				break;
		}
		/* 信託原交易日期 */
		switch (inTransType)
		{
			case	_ECR_8N1_VOID_NO_:
				/* 欄位為M 卻不存在可手動輸入 */
				if (inFunc_CheckFullSpace(srECROb->srTransData.srTRUST_Data.szTrustOrgDate) == VS_TRUE)
				{

				}
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &srECROb->srTransData.srTRUST_Data.szTrustOrgDate[0], 4);
				memcpy(pobTran->srBRec.szCUP_TD, szTemplate, 4);
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s :(%s)", "org date", pobTran->srBRec.szCUP_TD);
					inLogPrintf(AT, szDebugMsg);
				}
				break;
			default:
				break;
		}
	}
	else
	{
		/* 實際支付金額RDM Paid Amt (含小數 2位) */
		switch (inTransType)
		{
			case _ECR_8N1_REDEEM_REFUND_NO_:		/* 紅利退貨 */
			case _ECR_8N1_REDEEM_ADJUST_NO_:		/* 紅利調帳 */
			case _ECR_8N1_HG_REWARD_REDEMPTION_NO_ :	/* 快樂購紅利積點 + 信用卡紅利扣抵 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &srECROb->srTransData.szRDMPaidAmt[0], 10);	/* 12位數，這裡只取10位 */
                                
                                /* 支付金額比交易金額大 */
                                if (atol(szTemplate) > pobTran->srBRec.lnTxnAmount)
                                {
                                        memset(srECROb->srTransData.szRDMPaidAmt, 0x00, sizeof(srECROb->srTransData.szRDMPaidAmt));
                                        memcpy(srECROb->srTransData.szRDMPaidAmt, "000000000000", 12);	/* 重輸 */
                                        
                                        memset(szTemplate, 0x00, sizeof(szTemplate));
                                        memcpy(szTemplate, &srECROb->srTransData.szRDMPaidAmt[0], 10);	/* 12位數，這裡只取10位 */
                                }

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s :%s", "RDM Paid Amt", szTemplate);
					inLogPrintf(AT, szDebugMsg);
				}

				pobTran->srBRec.lnRedemptionPaidCreditAmount = atol(szTemplate);

				break;
			default:
				break;
		}

		/* 信用卡紅利扣抵點數 RDM Point */
		switch (inTransType)
		{
			case _ECR_8N1_REDEEM_REFUND_NO_:		/* 紅利退貨 */
			case _ECR_8N1_REDEEM_ADJUST_NO_:		/* 紅利調帳 */
			case _ECR_8N1_HG_REWARD_REDEMPTION_NO_ :	/* 快樂購紅利積點 + 信用卡紅利扣抵 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &srECROb->srTransData.szRDMPoint[0], 8);

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s :%s", "RDM Point", szTemplate);
					inLogPrintf(AT, szDebugMsg);
				}

				pobTran->srBRec.lnRedemptionPoints = atol(szTemplate);

				break;
			default:
				break;
		}

		/* 分期期數 Installment Period */
		switch (inTransType)
		{
			case _ECR_8N1_INSTALLMENT_NO_:			/* 分期 */
			case _ECR_8N1_INSTALLMENT_REFUND_NO_:		/* 分期退貨 */
			case _ECR_8N1_INSTALLMENT_ADJUST_NO_:		/* 分期調帳 */
			case _ECR_8N1_HG_REWARD_INSTALLMENT_NO_ :	/* 快樂購紅利積點 + 信用卡分期付款 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &srECROb->srTransData.szInstallmentPeriod[0], 2);

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s :%s", "Installment Period", szTemplate);
					inLogPrintf(AT, szDebugMsg);
				}

				pobTran->srBRec.lnInstallmentPeriod = atol(szTemplate);

				break;
			default:
				break;
		}

		/* 首期金額 Down Payment Amount (含小數 2位) */
		switch (inTransType)
		{
			case _ECR_8N1_INSTALLMENT_REFUND_NO_:		/* 分期退貨 */
			case _ECR_8N1_INSTALLMENT_ADJUST_NO_:		/* 分期調帳 */
			case _ECR_8N1_HG_REWARD_INSTALLMENT_NO_ :	/* 快樂購紅利積點 + 信用卡分期付款 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &srECROb->srTransData.szField_26[0], 10);	/* 12位數，這裡只取10位 */

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s :%s", "Down Payment Amount", szTemplate);
					inLogPrintf(AT, szDebugMsg);
				}

				pobTran->srBRec.lnInstallmentDownPayment = atol(szTemplate);

				break;
			default:
				break;
		}

		/* 每期金額 Installment Payment Amount (含小數 2位) */
		switch (inTransType)
		{
			case _ECR_8N1_INSTALLMENT_REFUND_NO_:		/* 分期退貨 */
			case _ECR_8N1_INSTALLMENT_ADJUST_NO_:		/* 分期調帳 */
			case _ECR_8N1_HG_REWARD_INSTALLMENT_NO_ :	/* 快樂購紅利積點 + 信用卡分期付款 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &srECROb->srTransData.szField_27[0], 10);	/* 12位數，這裡只取10位 */

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s :%s", "Installment Payment Amount", szTemplate);
					inLogPrintf(AT, szDebugMsg);
				}

				pobTran->srBRec.lnInstallmentPayment = atol(szTemplate);

				break;
			default:
				break;
		}

		/* 分期手續費 Formallity Fee (含小數 2位) */
		switch (inTransType)
		{
			case _ECR_8N1_INSTALLMENT_REFUND_NO_:		/* 分期退貨 */
			case _ECR_8N1_INSTALLMENT_ADJUST_NO_:		/* 分期調帳 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &srECROb->srTransData.szField_28[0], 10);	/* 12位數，這裡只取10位 */

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s :%s", "Formallity Fee", szTemplate);
					inLogPrintf(AT, szDebugMsg);
				}
				pobTran->srBRec.lnInstallmentFormalityFee = atol(szTemplate);
				
				if (pobTran->srBRec.lnInstallmentFormalityFee != 0)
				{
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_INST_FEE_NOT_0_;
					pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;		/* 操作錯誤 */
					return (VS_ERROR);
				}
				
				break;
			case _ECR_8N1_HG_REWARD_INSTALLMENT_NO_ :	/* 快樂購紅利積點 + 信用卡分期付款 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &srECROb->srTransData.szField_28[0], 10);	/* 12位數，這裡只取10位 */

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s :%s", "Formallity Fee", szTemplate);
					inLogPrintf(AT, szDebugMsg);
				}

				pobTran->srBRec.lnInstallmentFormalityFee = atol(szTemplate);

				break;
			default:
				break;
		}

		/* 啟動卡號查詢 Start Trans Type */
		switch (inTransType)
		{
			case _ECR_8N1_START_CARD_NO_INQUIRY_NO_:		/* 卡號查詢 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &srECROb->srTransData.szStartTransType[0], 2);	

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s :%s", "Start Trans Type", szTemplate);
					inLogPrintf(AT, szDebugMsg);
				}

				memcpy(srECROb->srTransData.szStartTransType, szTemplate, 2);

				break;
			default:
				break;
		}

		/* 只有金融卡才要收的欄位 */
		if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
		{
			/* 金融卡原交易日期 SP Origin Date */
			switch (inTransType)
			{
				case _ECR_8N1_REFUND_NO_:				/* 退費 */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memcpy(szTemplate, &srECROb->srTransData.szField_34[0], 8);

					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s :%s", "SP Origin Date", szTemplate);
						inLogPrintf(AT, szDebugMsg);
					}
					
					if (atoi(szTemplate) > 0	&&
					    inFunc_CheckValidDate_Include_Year(szTemplate) == VS_SUCCESS)
					{
						memcpy(pobTran->srBRec.szFiscRefundDate, szTemplate, 8);
					}
					
					break;
				default:
					break;
			}

			/* 金融卡調單編號 SP RRN */
			switch (inTransType)
			{
				case _ECR_8N1_REFUND_NO_:				/* 退費 */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memcpy(szTemplate, &srECROb->srTransData.szField_35[0], 12);

					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s :%s", "SP RRN", szTemplate);
						inLogPrintf(AT, szDebugMsg);
					}

					if (atoi(szTemplate) > 0)
					{
						memcpy(pobTran->srBRec.szFiscRRN, szTemplate, 12);
					}

					break;
				default:
					break;
			}

		}
		/* 只有電票要收的欄位 */
		else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
		{
			if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
			{
				/* 電票兩段式，第一段不檢核日期 */
			}
			else
			{
				/* ESVC Origin Date */
				switch (inTransType)
				{
					case _ECR_8N1_REFUND_NO_:				/* 電票退貨 */
						memset(szTemplate, 0x00, sizeof(szTemplate));
						memcpy(szTemplate, &srECROb->srTransData.szField_34[0], 8);

						if (ginDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
							sprintf(szDebugMsg, "%s :%s", "ESVC Origin Date", szTemplate);
							inLogPrintf(AT, szDebugMsg);
						}

						if (atoi(szTemplate) > 0	&&
						    inFunc_CheckValidDate_Include_Year(szTemplate) == VS_SUCCESS)
						{
							memcpy(pobTran->srTRec.szTicketRefundDate, &szTemplate[4], 4);
						}
						else
						{
/* [20251219_BUG_MDF][ECR][DATA] 修改ECR欄位34資料,因電票資料如果為空白會拒絕，跳過檢查步驟,進行後續人工輸入 */
#ifdef _ECR_CHECK_FILE_34_							
							/* 接收資料錯誤 */
							memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
							inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
							memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
							sprintf(pobTran->szErrorMsgBuff2, "ESVC Origin Date Not Exist");
							pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

							return (VS_ERROR);
#endif
						}

						break;
					default:
						break;
				}

				/* ATS電票交易序號 RF NUMBER 左靠右補空白 */
				switch (inTransType)
				{
					case _ECR_8N1_REFUND_NO_:				/* 電票退貨 */
						memset(szTemplate, 0x00, sizeof(szTemplate));
						memcpy(szTemplate, &srECROb->srTransData.szField_35[0], 12);

						if (ginDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
							sprintf(szDebugMsg, "%s :%s", "RF NUMBER", szTemplate);
							inLogPrintf(AT, szDebugMsg);
						}

						memcpy(pobTran->srTRec.szTicketRefundCode, szTemplate, 12);


						break;
					default:
						break;
				}
			}
		}
		/* 只有銀聯要收的欄位 */
		else if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
		{
			/* CUP Origin Date */
			switch (inTransType)
			{
				case _ECR_8N1_REFUND_NO_:				/* 電票退貨 */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memcpy(szTemplate, &srECROb->srTransData.szField_34[0], 8);

					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s :%s", "CUP Origin Date", szTemplate);
						inLogPrintf(AT, szDebugMsg);
					}

					if (atoi(szTemplate) > 0	&&
					    inFunc_CheckValidDate_Include_Year(szTemplate) == VS_SUCCESS)
					{
						memcpy(pobTran->srBRec.szCUP_TD, &szTemplate[4], 4);
					}

					break;
				default:
					break;
			}
		}
	}
	
	/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/5/30 下午 1:30 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_075_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_) || 
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_103_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_))
	{
		switch (inTransType)
		{
			case _ECR_8N1_SALE_NO_ :
			case _ECR_8N1_REDEEM_NO_ :
			case _ECR_8N1_INSTALLMENT_NO_ :
				/* 'Y' = 昇恆昌聯名卡判斷 */
				if (srECROb->srTransData.szMPFlag[0] == 'Y')
					pobTran->uszCheck_CoBrand_CardBit = VS_TRUE;
				else
					pobTran->uszCheck_CoBrand_CardBit = VS_FALSE;
				break;
			default :
				pobTran->uszCheck_CoBrand_CardBit = VS_FALSE;
				break;
		}
	}
	
	/* 【需求單 - 104014】發卡機構on-us繳費平台需求 +【需求單 - 104152】發卡機構on-us繳費平台 Payitem Code by Russell 2020/3/16 下午 5:38 */
	memset(szPayItemFuncEnable, 0x00, sizeof(szPayItemFuncEnable));
	inGetPayItemEnable(szPayItemFuncEnable);
	if (memcmp(szPayItemFuncEnable, "Y", 1) == 0							&&
	    memcmp(&srECROb->srTransData.szTransType[0], _ECR_8N1_START_CARD_NO_INQUIRY_, 2) != 0)
	{
		/* PayItemFuncEnable有開才進入檢核收銀機送的繳費項目 */
		/* 兩段式收銀機連線 第一段不檢核Payitem Code */
		switch (inTransType)
		{
			case _ECR_8N1_SALE_NO_ :
			case _ECR_8N1_REFUND_NO_ :
			case _ECR_8N1_OFFLINE_NO_ :
			case _ECR_8N1_PREAUTH_NO_ :
			case _ECR_8N1_PREAUTH_COMPLETE_NO_ :
			case _ECR_8N1_INSTALLMENT_NO_ :
			case _ECR_8N1_INSTALLMENT_REFUND_NO_ :
			case _ECR_8N1_REDEEM_NO_ :
			case _ECR_8N1_REDEEM_REFUND_NO_ :
				srECROb->srTransData.uszECRResponsePayitem = VS_TRUE;
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, srECROb->srTransData.szPayItem, 5);

				/* 先判斷是不是送5個空白 */
				if (!memcmp(szTemplate, "     ", 5))
				{
					/* 空白不必回傳payitem */
					if (ginDebug == VS_TRUE)
					{
						srECROb->srTransData.uszECRResponsePayitem = VS_FALSE;
						inLogPrintf(AT, "ECR not input PayItem");
					}
					break;
				}

				/* 比對端末機的PIT Table PayItemCode */
				/* 吳升文(Angus Wu) 2015-09-15 (週二) 下午 04:43 回覆 帶5個0視為有值，所以要比對。 */
				for (i = 0 ;; i++)
				{
					/* 吳升文(Angus Wu) 2015-09-15 (週二) 下午 04:24 回覆
						因為繳費項目的代碼會從授權一直帶到清算再回到發卡行，然後發卡行根據繳費項目代碼跟持卡人收費。
						所以不合法的繳費代碼端末機不可以後送，免得後端勾稽不到而收不到錢。
						因此比對不到繳費代碼則端末機提示”不支援該繳費項目”後，回傳Response Code=0004。
					*/
					/* 比對不到不進入輸入keymap畫面 在ECR連線阻檔 */
					if (inLoadPITRec(i) < 0)
					{
						/* 不支援該繳費項目 */
						pobTran->inErrorMsg = _ERROR_CODE_V3_NOT_SUPPORT_THIS_PAY_ITEM_;
						pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
						
						return (VS_ERROR);
					}

					/* 有比對到端末機的繳費項目 */
					memset(szPayItemCodeTemp, 0x00, sizeof(szPayItemCodeTemp));
					inGetPayItemCode(szPayItemCodeTemp);
					if (memcmp(szTemplate, szPayItemCodeTemp, 5) == 0)
					{
						memset(pobTran->srBRec.szPayItemCode, 0x00, sizeof(pobTran->srBRec.szPayItemCode));
						memcpy(&pobTran->srBRec.szPayItemCode[0], &szTemplate[0], 5);
						break;
					}
				}

				break;
			default :
				break;
		}
	}
	
	/* 客製化和HG互斥 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_126_MASTERCARD_FLIGHT_TICKET_, _CUSTOMER_INDICATOR_SIZE_))
	{
		/* 不檢核的交易別 */
		if (inNCCC_Func_Flight_Ticket_ECR_Not_Allow(pobTran, srECROb) == VS_TRUE)
                {
                        
                }
                else if (inNCCC_Func_Flight_Ticket_Not_Allow_Txn_Type(pobTran) == VS_TRUE)
                {
                        
                }
		else
		{
			/* 是否為機票交易 */
			switch (inTransType)
			{
				case _ECR_8N1_SALE_NO_ :
				case _ECR_8N1_REFUND_NO_ :
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s :%s", "szFlightTicketTransBit", srECROb->srTransData.szFlightTicketTransBit);
						inLogPrintf(AT, szDebugMsg);
					}

					/* 欄位為M 卻不存在，回傳錯誤 */
					if (inFunc_CheckFullSpace(srECROb->srTransData.szFlightTicketTransBit) == VS_TRUE)
					{
						if (ginDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
							sprintf(szDebugMsg, "%s: %s  ", "FlightBit", "Not Exist Error");
							inLogPrintf(AT, szDebugMsg);
						}
						memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
						inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
						memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
						sprintf(pobTran->szErrorMsgBuff2, "FlightBit null Error");
						pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
						pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_FORMAT_CHECK_ERROR_;
						vdUtility_SYSFIN_LogMessage(AT, "%s: %s  ", "FlightBit", "Not Exist Error");

						return (VS_ERROR);
					}
					else
					{
						if (srECROb->srTransData.szFlightTicketTransBit[0] == 'Y')
						{
							pobTran->srBRec.szFlightTicketTransBit[0] = _FLIGHT_TICKET_TRANS_YES_;
						}
						else if (srECROb->srTransData.szFlightTicketTransBit[0] == 'N')
						{
							pobTran->srBRec.szFlightTicketTransBit[0] = _FLIGHT_TICKET_TRANS_NOT_;
						}
						else
						{
							if (ginDebug == VS_TRUE)
							{
								memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
								sprintf(szDebugMsg, "%s: %s (%s)", "szFlightTicketTransBit", "Not Y or N", pobTran->srBRec.szFlightTicketTransBit);
								inLogPrintf(AT, szDebugMsg);
							}
							memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
							inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
							memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
							sprintf(pobTran->szErrorMsgBuff2, "szFlightTicketTransBit Not Y or N");
							pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
							pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_FORMAT_CHECK_ERROR_;
							vdUtility_SYSFIN_LogMessage(AT, "%s: %s (%s)", "szFlightTicketTransBit", "Not Y or N", pobTran->srBRec.szFlightTicketTransBit);

							return (VS_ERROR);
						}
					}

					break;
				default :
					break;
			}

			/* 出發地機場代碼 */
			switch (inTransType)
			{
				case _ECR_8N1_SALE_NO_ :
				case _ECR_8N1_REFUND_NO_ :
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s :%s", "szFlightTicketPDS0523", srECROb->srTransData.szFlightTicketPDS0523);
						inLogPrintf(AT, szDebugMsg);
					}

					if (pobTran->srBRec.szFlightTicketTransBit[0] == _FLIGHT_TICKET_TRANS_YES_)
					{
                                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                                memcpy(szTemplate, srECROb->srTransData.szFlightTicketPDS0523, 5);
                                                inFunc_DiscardSpace(szTemplate);
						if ((inFunc_Check_Validation(szTemplate, 3, _CHECK_VALIDATION_MODE_1_) == VS_TRUE && strlen(szTemplate) == 3)	||
						    (inFunc_Check_Validation(szTemplate, 4, _CHECK_VALIDATION_MODE_1_) == VS_TRUE && strlen(szTemplate) == 4)	||
						    (inFunc_Check_Validation(szTemplate, 5, _CHECK_VALIDATION_MODE_1_) == VS_TRUE && strlen(szTemplate) == 5))
						{
							memcpy(pobTran->srBRec.szFlightTicketPDS0523, szTemplate, 5);
						}
						else
						{
							if (ginDebug == VS_TRUE)
							{
								memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
								sprintf(szDebugMsg, "%s: %s  %s", "szFlightTicketPDS0523", "Illegal", srECROb->srTransData.szFlightTicketPDS0523);
								inLogPrintf(AT, szDebugMsg);
							}
							memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
							sprintf(pobTran->szErrorMsgBuff2, srECROb->srTransData.szFlightTicketPDS0523);
							memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
							sprintf(pobTran->szErrorMsgBuff2, "PDS0523 Illegal");
							pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
							pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_FORMAT_CHECK_ERROR_;
							vdUtility_SYSFIN_LogMessage(AT, "%s: %s  %s", "szFlightTicketPDS0523", "Illegal", srECROb->srTransData.szFlightTicketPDS0523);

							return (VS_ERROR);
						}
					}
					else if (pobTran->srBRec.szFlightTicketTransBit[0] == _FLIGHT_TICKET_TRANS_NOT_)
					{
                                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                                memcpy(szTemplate, srECROb->srTransData.szFlightTicketPDS0523, 5);
                                                inFunc_DiscardSpace(szTemplate);
						if (!memcmp(szTemplate, _FLIGHT_TICKET_PDS0523_SOUVENIR_, 3) ||
						    (inFunc_Check_Validation(szTemplate, 3, _CHECK_VALIDATION_MODE_1_) == VS_TRUE && strlen(szTemplate) == 3))
						{
							memcpy(pobTran->srBRec.szFlightTicketPDS0523, szTemplate, 5);
						}
						else
						{
							if (ginDebug == VS_TRUE)
							{
								memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
								sprintf(szDebugMsg, "%s: %s  %s", "szFlightTicketPDS0523", "Illegal", srECROb->srTransData.szFlightTicketPDS0523);
								inLogPrintf(AT, szDebugMsg);
							}
							memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
							sprintf(pobTran->szErrorMsgBuff2, srECROb->srTransData.szFlightTicketPDS0523);
							memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
							sprintf(pobTran->szErrorMsgBuff2, "PDS0523 Illegal");
							pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
							pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_FORMAT_CHECK_ERROR_;
							vdUtility_SYSFIN_LogMessage(AT, "%s: %s  %s", "szFlightTicketPDS0523", "Illegal", srECROb->srTransData.szFlightTicketPDS0523);

							return (VS_ERROR);
						}
					}

					break;
				default :
					break;
			}

			/* 目的地機場代碼 */
			switch (inTransType)
			{
				case _ECR_8N1_SALE_NO_ :
				case _ECR_8N1_REFUND_NO_ :
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s :%s", "szFlightTicketPDS0524", srECROb->srTransData.szFlightTicketPDS0524);
						inLogPrintf(AT, szDebugMsg);
					}

					if (pobTran->srBRec.szFlightTicketTransBit[0] == _FLIGHT_TICKET_TRANS_YES_)
					{
                                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                                memcpy(szTemplate, srECROb->srTransData.szFlightTicketPDS0524, 5);
                                                inFunc_DiscardSpace(szTemplate);
						if ((inFunc_Check_Validation(szTemplate, 3, _CHECK_VALIDATION_MODE_1_) == VS_TRUE && strlen(szTemplate) == 3)	||
						    (inFunc_Check_Validation(szTemplate, 4, _CHECK_VALIDATION_MODE_1_) == VS_TRUE && strlen(szTemplate) == 4)	||
						    (inFunc_Check_Validation(szTemplate, 5, _CHECK_VALIDATION_MODE_1_) == VS_TRUE && strlen(szTemplate) == 5))
						{
							memcpy(pobTran->srBRec.szFlightTicketPDS0524, szTemplate, 5);
						}
						else
						{
							if (ginDebug == VS_TRUE)
							{
								memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
								sprintf(szDebugMsg, "%s: %s  %s", "szFlightTicketPDS0524", "Illegal", srECROb->srTransData.szFlightTicketPDS0524);
								inLogPrintf(AT, szDebugMsg);
							}
							memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
							sprintf(pobTran->szErrorMsgBuff2, srECROb->srTransData.szFlightTicketPDS0524);
							memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
							sprintf(pobTran->szErrorMsgBuff2, "PDS0524 Illegal");
							pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
							pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_FORMAT_CHECK_ERROR_;
							vdUtility_SYSFIN_LogMessage(AT, "%s: %s  %s", "szFlightTicketPDS0524", "Illegal", srECROb->srTransData.szFlightTicketPDS0524);

							return (VS_ERROR);
						}
					}
					else if (pobTran->srBRec.szFlightTicketTransBit[0] == _FLIGHT_TICKET_TRANS_NOT_)
					{
                                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                                memcpy(szTemplate, srECROb->srTransData.szFlightTicketPDS0524, 5);
                                                inFunc_DiscardSpace(szTemplate);
						if (!memcmp(szTemplate, _FLIGHT_TICKET_PDS0523_SOUVENIR_, 3) ||
						    (inFunc_Check_Validation(szTemplate, 3, _CHECK_VALIDATION_MODE_1_) == VS_TRUE && strlen(szTemplate) == 3))
						{
							memcpy(pobTran->srBRec.szFlightTicketPDS0524, szTemplate, 5);
						}
						else
						{
							if (ginDebug == VS_TRUE)
							{
								memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
								sprintf(szDebugMsg, "%s: %s  %s", "szFlightTicketPDS0524", "Illegal", srECROb->srTransData.szFlightTicketPDS0524);
								inLogPrintf(AT, szDebugMsg);
							}
							memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
							sprintf(pobTran->szErrorMsgBuff2, srECROb->srTransData.szFlightTicketPDS0524);
							memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
							sprintf(pobTran->szErrorMsgBuff2, "PDS0524 Illegal");
							pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
							pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_FORMAT_CHECK_ERROR_;
							vdUtility_SYSFIN_LogMessage(AT, "%s: %s  %s", "szFlightTicketPDS0524", "Illegal", srECROb->srTransData.szFlightTicketPDS0524);

							return (VS_ERROR);
						}
					}

					break;
				default :
					break;
			}

			/* 航班號碼 */
			switch (inTransType)
			{
				case _ECR_8N1_SALE_NO_ :
				case _ECR_8N1_REFUND_NO_ :
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s :%s", "szFlightTicketPDS0530", srECROb->srTransData.szFlightTicketPDS0530);
						inLogPrintf(AT, szDebugMsg);
					}

                                        
					if (pobTran->srBRec.szFlightTicketTransBit[0] == _FLIGHT_TICKET_TRANS_YES_)
					{
                                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                                memcpy(szTemplate, srECROb->srTransData.szFlightTicketPDS0530, 5);
                                                inFunc_DiscardSpace(szTemplate);
						if ((inFunc_Check_Validation(szTemplate, 1, _CHECK_VALIDATION_MODE_2_) == VS_TRUE && strlen(szTemplate) == 1)	||
						    (inFunc_Check_Validation(szTemplate, 2, _CHECK_VALIDATION_MODE_2_) == VS_TRUE && strlen(szTemplate) == 2)	||
						    (inFunc_Check_Validation(szTemplate, 3, _CHECK_VALIDATION_MODE_2_) == VS_TRUE && strlen(szTemplate) == 3)	||
						    (inFunc_Check_Validation(szTemplate, 4, _CHECK_VALIDATION_MODE_2_) == VS_TRUE && strlen(szTemplate) == 4)	||
						    (inFunc_Check_Validation(szTemplate, 5, _CHECK_VALIDATION_MODE_2_) == VS_TRUE && strlen(szTemplate) == 5))
						{
							memcpy(pobTran->srBRec.szFlightTicketPDS0530, szTemplate, 5);
						}
						else
						{
							if (ginDebug == VS_TRUE)
							{
								memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
								sprintf(szDebugMsg, "%s: %s  %s", "szFlightTicketPDS0530", "Illegal", srECROb->srTransData.szFlightTicketPDS0530);
								inLogPrintf(AT, szDebugMsg);
							}
							memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
							sprintf(pobTran->szErrorMsgBuff2, srECROb->srTransData.szFlightTicketPDS0530);
							memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
							sprintf(pobTran->szErrorMsgBuff2, "PDS0530 Illegal");
							pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
							pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_FORMAT_CHECK_ERROR_;
							vdUtility_SYSFIN_LogMessage(AT, "%s: %s  %s", "szFlightTicketPDS0530", "Illegal", srECROb->srTransData.szFlightTicketPDS0530);

							return (VS_ERROR);
						}
					}
					else if (pobTran->srBRec.szFlightTicketTransBit[0] == _FLIGHT_TICKET_TRANS_NOT_)
					{
						unsigned char   uszGetBit = VS_FALSE;
                                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                                memcpy(szTemplate, srECROb->srTransData.szFlightTicketPDS0530, 5);
                                                inFunc_DiscardSpace(szTemplate);
                                                if (!memcmp(pobTran->srBRec.szFlightTicketPDS0523, _FLIGHT_TICKET_PDS0523_SOUVENIR_, max(strlen(pobTran->srBRec.szFlightTicketPDS0523), strlen(_FLIGHT_TICKET_PDS0523_SOUVENIR_))) &&
                                                     inFunc_CheckFullSpaceWithLen(srECROb->srTransData.szFlightTicketPDS0530, 5) == VS_TRUE)
                                                {
                                                        uszGetBit = VS_TRUE;
                                                }
                                                else if (memcmp(pobTran->srBRec.szFlightTicketPDS0523, _FLIGHT_TICKET_PDS0523_SOUVENIR_, max(strlen(pobTran->srBRec.szFlightTicketPDS0523), strlen(_FLIGHT_TICKET_PDS0523_SOUVENIR_))) &&
                                                         ((inFunc_Check_Validation(szTemplate, 1, _CHECK_VALIDATION_MODE_2_) == VS_TRUE && strlen(szTemplate) == 1) ||
                                                          (inFunc_Check_Validation(szTemplate, 2, _CHECK_VALIDATION_MODE_2_) == VS_TRUE && strlen(szTemplate) == 2) ||
                                                          (inFunc_Check_Validation(szTemplate, 3, _CHECK_VALIDATION_MODE_2_) == VS_TRUE && strlen(szTemplate) == 3) ||
                                                          (inFunc_Check_Validation(szTemplate, 4, _CHECK_VALIDATION_MODE_2_) == VS_TRUE && strlen(szTemplate) == 4) ||
                                                          (inFunc_Check_Validation(szTemplate, 5, _CHECK_VALIDATION_MODE_2_) == VS_TRUE && strlen(szTemplate) == 5)))
                                                {
                                                        uszGetBit = VS_TRUE;
                                                }
                                                
                                                if (uszGetBit == VS_TRUE)
                                                {
                                                        memcpy(pobTran->srBRec.szFlightTicketPDS0530, szTemplate, 5);
                                                }
                                                else
                                                {
                                                        if (ginDebug == VS_TRUE)
                                                        {
                                                                memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                                                                sprintf(szDebugMsg, "%s: %s  %s", "szFlightTicketPDS0530", "Illegal", srECROb->srTransData.szFlightTicketPDS0530);
                                                                inLogPrintf(AT, szDebugMsg);
                                                        }
                                                        memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
                                                        sprintf(pobTran->szErrorMsgBuff2, srECROb->srTransData.szFlightTicketPDS0524);
                                                        memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
                                                        sprintf(pobTran->szErrorMsgBuff2, "PDS0530 Illegal");
                                                        pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
                                                        pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_FORMAT_CHECK_ERROR_;
                                                        vdUtility_SYSFIN_LogMessage(AT, "%s: %s  %s", "szFlightTicketPDS0530", "Illegal", srECROb->srTransData.szFlightTicketPDS0530);

                                                        return (VS_ERROR);
                                                }
					}

					break;
				default :
					break;
			}
                        
                        /* 機票資料連貫性檢核 */
                        switch (inTransType)
			{
				case _ECR_8N1_SALE_NO_ :
				case _ECR_8N1_VOID_NO_ :
				case _ECR_8N1_REFUND_NO_ :
					if (pobTran->srBRec.szFlightTicketTransBit[0] == _FLIGHT_TICKET_TRANS_YES_)
					{
						
					}
					else if (pobTran->srBRec.szFlightTicketTransBit[0] == _FLIGHT_TICKET_TRANS_NOT_)
					{
						if (!memcmp(srECROb->srTransData.szFlightTicketPDS0523, _FLIGHT_TICKET_PDS0523_SOUVENIR_, 3)		||
						    !memcmp(srECROb->srTransData.szFlightTicketPDS0523, _FLIGHT_TICKET_TXN_CATEGORY_UPGRADE_, 3)	||
						    !memcmp(srECROb->srTransData.szFlightTicketPDS0523, _FLIGHT_TICKET_TXN_CATEGORY_CLUB_FEE_, 3)	||
						    !memcmp(srECROb->srTransData.szFlightTicketPDS0523, _FLIGHT_TICKET_TXN_CATEGORY_CARGO_, 3)		||
						    !memcmp(srECROb->srTransData.szFlightTicketPDS0523, _FLIGHT_TICKET_TXN_CATEGORY_DUTY_FREE_, 3)	||
						    !memcmp(srECROb->srTransData.szFlightTicketPDS0523, _FLIGHT_TICKET_TXN_CATEGORY_BAGGAGE_CHARGE_, 3)	||
						    !memcmp(srECROb->srTransData.szFlightTicketPDS0523, _FLIGHT_TICKET_TXN_CATEGORY_PREPAID_TICKET_, 3)	||
						    !memcmp(srECROb->srTransData.szFlightTicketPDS0523, _FLIGHT_TICKET_TXN_CATEGORY_DELIVERY_, 3)	||
						    !memcmp(srECROb->srTransData.szFlightTicketPDS0523, _FLIGHT_TICKET_TXN_CATEGORY_PET_CARRIER_, 3)	||
						    !memcmp(srECROb->srTransData.szFlightTicketPDS0523, _FLIGHT_TICKET_TXN_CATEGORY_TICKET_REISSUE_, 3)	||
						    !memcmp(srECROb->srTransData.szFlightTicketPDS0523, _FLIGHT_TICKET_TXN_CATEGORY_TOUR_ORDER_, 3)	||
						    !memcmp(srECROb->srTransData.szFlightTicketPDS0523, _FLIGHT_TICKET_TXN_CATEGORY_OTHERS_, 3))
						{
							
						}
						else
						{
							if (ginDebug == VS_TRUE)
							{
								memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
								sprintf(szDebugMsg, "%s: %s  %s", "PDS0523", "Illegal with N", srECROb->srTransData.szFlightTicketPDS0523);
								inLogPrintf(AT, szDebugMsg);
							}
							memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
							sprintf(pobTran->szErrorMsgBuff2, srECROb->srTransData.szFlightTicketPDS0524);
							memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
							sprintf(pobTran->szErrorMsgBuff2, "PDS0523 Illegal with N");
							pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
							pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_FORMAT_CHECK_ERROR_;
							vdUtility_SYSFIN_LogMessage(AT, "%s: %s  %s", "PDS0523", "Illegal with N", srECROb->srTransData.szFlightTicketPDS0523);
							
							return (VS_ERROR);
						}
						
						if (!memcmp(srECROb->srTransData.szFlightTicketPDS0524, _FLIGHT_TICKET_PDS0523_SOUVENIR_, 3)	||
						    !memcmp(srECROb->srTransData.szFlightTicketPDS0524, _FLIGHT_TICKET_TXN_CATEGORY_UPGRADE_, 3)	||
						    !memcmp(srECROb->srTransData.szFlightTicketPDS0524, _FLIGHT_TICKET_TXN_CATEGORY_CLUB_FEE_, 3)	||
						    !memcmp(srECROb->srTransData.szFlightTicketPDS0524, _FLIGHT_TICKET_TXN_CATEGORY_CARGO_, 3)		||
						    !memcmp(srECROb->srTransData.szFlightTicketPDS0524, _FLIGHT_TICKET_TXN_CATEGORY_DUTY_FREE_, 3)	||
						    !memcmp(srECROb->srTransData.szFlightTicketPDS0524, _FLIGHT_TICKET_TXN_CATEGORY_BAGGAGE_CHARGE_, 3)	||
						    !memcmp(srECROb->srTransData.szFlightTicketPDS0524, _FLIGHT_TICKET_TXN_CATEGORY_PREPAID_TICKET_, 3)	||
						    !memcmp(srECROb->srTransData.szFlightTicketPDS0524, _FLIGHT_TICKET_TXN_CATEGORY_DELIVERY_, 3)	||
						    !memcmp(srECROb->srTransData.szFlightTicketPDS0524, _FLIGHT_TICKET_TXN_CATEGORY_PET_CARRIER_, 3)	||
						    !memcmp(srECROb->srTransData.szFlightTicketPDS0524, _FLIGHT_TICKET_TXN_CATEGORY_TICKET_REISSUE_, 3)	||
						    !memcmp(srECROb->srTransData.szFlightTicketPDS0524, _FLIGHT_TICKET_TXN_CATEGORY_TOUR_ORDER_, 3)	||
						    !memcmp(srECROb->srTransData.szFlightTicketPDS0524, _FLIGHT_TICKET_TXN_CATEGORY_OTHERS_, 3))
						{
							
						}
						else
						{
							if (ginDebug == VS_TRUE)
							{
								memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
								sprintf(szDebugMsg, "%s: %s  %s", "PDS0524", "Illegal with N", srECROb->srTransData.szFlightTicketPDS0524);
								inLogPrintf(AT, szDebugMsg);
							}
							memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
							sprintf(pobTran->szErrorMsgBuff2, srECROb->srTransData.szFlightTicketPDS0524);
							memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
							sprintf(pobTran->szErrorMsgBuff2, "PDS0524 Illegal with N");
							pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
							pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_FORMAT_CHECK_ERROR_;
							vdUtility_SYSFIN_LogMessage(AT, "%s: %s  %s", "PDS0524", "Illegal with N", srECROb->srTransData.szFlightTicketPDS0524);
							
							return (VS_ERROR);
						}
						
						if (memcmp(srECROb->srTransData.szFlightTicketPDS0523, srECROb->srTransData.szFlightTicketPDS0524, max(strlen(srECROb->srTransData.szFlightTicketPDS0523), strlen(srECROb->srTransData.szFlightTicketPDS0524))))
						{
							if (ginDebug == VS_TRUE)
							{
								memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
								sprintf(szDebugMsg, "%s not equal %s %s %s", "PDS0523", "PDS0524", srECROb->srTransData.szFlightTicketPDS0523, srECROb->srTransData.szFlightTicketPDS0524);
								inLogPrintf(AT, szDebugMsg);
							}
							memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
							sprintf(pobTran->szErrorMsgBuff2, srECROb->srTransData.szFlightTicketPDS0524);
							memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
							sprintf(pobTran->szErrorMsgBuff2, "PDS0523 PDS0524 not equal");
							pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
							pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_FORMAT_CHECK_ERROR_;
							vdUtility_SYSFIN_LogMessage(AT, "%s not equal %s %s %s", "PDS0523", "PDS0524", srECROb->srTransData.szFlightTicketPDS0523, srECROb->srTransData.szFlightTicketPDS0524);
							
							return (VS_ERROR);
						}
					}
                                        break;
				default :
					break;
			}
		}
	}
	else
	{
		/* 處理HG */
		/* 支付工具 */
		switch (inTransType)
		{
			case _ECR_8N1_HG_REWARD_SALE_NO_ :
			case _ECR_8N1_HG_ONLINE_REDEEMPTION_NO_ :
			case _ECR_8N1_HG_POINT_CERTAIN_NO_ :
			case _ECR_8N1_HG_REWARD_INSTALLMENT_NO_ :
			case _ECR_8N1_HG_REWARD_REDEMPTION_NO_ :
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s :%s", "HG Payment Tool", srECROb->srTransData.szHGPaymentTool);
					inLogPrintf(AT, szDebugMsg);
				}

				/* 欄位為M 卻不存在，回傳錯誤 */
				if (inFunc_CheckFullSpace(srECROb->srTransData.szHGPaymentTool) == VS_TRUE)
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s: %s  ", "HG Payment Tool", "Not Exist Error");
						inLogPrintf(AT, szDebugMsg);
					}
					memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
					inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "HG Payment Tool Error");
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

					return (VS_ERROR);
				}

				if (memcmp(srECROb->srTransData.szHGPaymentTool, _ECR_8N1_HG_PAYMENT_TOOL_TYPE_EDC_CHOOSE_, 2) == 0)
				{
					pobTran->srBRec.lnHGPaymentType = 0;
				}
				else if (memcmp(srECROb->srTransData.szHGPaymentTool, _ECR_8N1_HG_PAYMENT_TOOL_TYPE_CREDIT_CARD_, 2) == 0)
				{
					pobTran->srBRec.lnHGPaymentType = _HG_PAY_CREDIT_;
				}
				else if (memcmp(srECROb->srTransData.szHGPaymentTool, _ECR_8N1_HG_PAYMENT_TOOL_TYPE_CASH_, 2) == 0)
				{
					pobTran->srBRec.lnHGPaymentType = _HG_PAY_CASH_;
				}
				else if (memcmp(srECROb->srTransData.szHGPaymentTool, _ECR_8N1_HG_PAYMENT_TOOL_TYPE_GIFT_PAPER_, 2) == 0)
				{
					pobTran->srBRec.lnHGPaymentType = _HG_PAY_GIFT_PAPER_;
				}
				else if (memcmp(srECROb->srTransData.szHGPaymentTool, _ECR_8N1_HG_PAYMENT_TOOL_TYPE_HGI_, 2) == 0)
				{
					pobTran->srBRec.lnHGPaymentType = _HG_PAY_CREDIT_INSIDE_;
				}
				else if (memcmp(srECROb->srTransData.szHGPaymentTool, _ECR_8N1_HG_PAYMENT_TOOL_TYPE_CUP_CARD_, 2) == 0)
				{
					pobTran->srBRec.lnHGPaymentType = _HG_PAY_CUP_;
				}
				else
				{
					memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
					inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "HG Payment Tool Error");
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				}
				break;
			default :
				break;
		}
	}
	
	/* Happy Go 扣抵點數 */
	switch (inTransType)
	{
		case _ECR_8N1_HG_POINT_CERTAIN_NO_ :
		case _ECR_8N1_HG_FULL_REDEEMPTION_NO_ :
		case _ECR_8N1_HG_REDEEM_REFUND_NO_ :
		case _ECR_8N1_HG_REWARD_REFUND_NO_ :
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "HG Redeem Point", srECROb->srTransData.szHGRedeemPoint);
				inLogPrintf(AT, szDebugMsg);
			}
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(srECROb->srTransData.szHGRedeemPoint) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "HG Redeem Point", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "HG Redeem Point Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

				return (VS_ERROR);
			}
			
			pobTran->srBRec.lnHGTransactionPoint = atol(srECROb->srTransData.szHGRedeemPoint);
			break;
		default :
			break;
	}
	
	if (gbBarCodeECRBit == VS_TRUE)
	{
		if (srECROb->srTransData.srEW_Data.uszEWTransBit == VS_TRUE)
		{
			/* 退貨訂單編號 */
			switch (inTransType)
			{
				case _ECR_8N1_REFUND_NO_:
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memcpy(szTemplate, &srECROb->srTransData.srEW_Data.szEW_OrderId[0], 30);

					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s :%s", "EW_OrderId", szTemplate);
						inLogPrintf(AT, szDebugMsg);
					}

					memcpy(pobTran->srEWRec.szEW_OrderId, szTemplate, 30);
					break;
				default :
					break;
			}
			
			/* 電子錢包業者交易序號 */
			switch (inTransType)
			{
				case _ECR_8N1_REFUND_NO_:
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memcpy(szTemplate, &srECROb->srTransData.srEW_Data.szEW_TransactionId[0], 64);

					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s :%s", "EW_TransactionId", szTemplate);
						inLogPrintf(AT, szDebugMsg);
					}

					memcpy(pobTran->srEWRec.szEW_TransactionId, szTemplate, 64);
					break;
				default :
					break;
			}
		}
		
                /* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/7 上午 10:28 */
                /* 一維或二維條碼資料 */
                switch (inTransType)
                {
                        case _ECR_8N1_END_CARD_NO_INQUIRY_NO_ :
                        case _ECR_8N1_SALE_NO_:
                        case _ECR_8N1_INSTALLMENT_NO_:
                        case _ECR_8N1_REDEEM_NO_:
                        case _ECR_8N1_EW_INQUIRY_TRANSACTION_NO_:
                                if (ginDebug == VS_TRUE)
                                {
                                        memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                                        sprintf(szDebugMsg, "%s :%s", "BarCode Data Len", srECROb->srTransData.szBarCodeLen);
                                        inLogPrintf(AT, szDebugMsg);
                                        memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                                        sprintf(szDebugMsg, "%s :%s", "BarCode Data", srECROb->srTransData.szBarCodeData);
                                        inLogPrintf(AT, szDebugMsg);
                                }

                                if (pobTran->srBRec.uszUnyTransBit == VS_TRUE ||
                                    srECROb->srTransData.srEW_Data.uszEWTransBit == VS_TRUE ||
				    pobTran->srTrustRec.uszTrustTransBit == VS_TRUE)
                                {
                                        /* 欄位為M 卻不存在，回傳錯誤 */
                                        if (inFunc_CheckFullSpace(srECROb->srTransData.szBarCodeLen) == VS_TRUE)
                                        {
                                                if (ginDebug == VS_TRUE)
                                                {
                                                        memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                                                        sprintf(szDebugMsg, "%s: %s  ", "BarCode Data Len", "Not Exist Error");
                                                        inLogPrintf(AT, szDebugMsg);
                                                }
                                                memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
                                                inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
                                                memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
                                                sprintf(pobTran->szErrorMsgBuff2, "BarCode Data Len Error");
                                                pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
						pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;		/* 操作錯誤 & 交易流程有誤 */
						if (pobTran->srTrustRec.uszTrustTransBit == VS_TRUE)
						{
							inECR_SendMirror(pobTran, _MIRROR_MSG_8N1_Standard_OPER_ERR_);
						}

                                                return (VS_ERROR);
                                        }
                                }

                                /* 欄位為M 卻不存在，回傳錯誤 */
                                if (srECROb->srTransData.srEW_Data.uszEWTransBit == VS_TRUE)
                                {

                                }
                                else if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
                                {
                                        if (inNCCC_Func_Check_Uny_Format(srECROb->srTransData.szBarCodeData) == VS_FALSE)
                                        {
                                                if (ginDebug == VS_TRUE)
                                                {
                                                        memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                                                        sprintf(szDebugMsg, "%s: %s  ", "BarCode Data", "Not Uny Format");
                                                        inLogPrintf(AT, szDebugMsg);
                                                }
                                                memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
                                                inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
                                                memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
                                                sprintf(pobTran->szErrorMsgBuff2, "Not Uny Format");
                                                pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

                                                return (VS_ERROR);
                                        }
                                }
				else if (pobTran->srTrustRec.uszTrustTransBit == VS_TRUE)
				{
					/* 欄位為M 卻不存在，回傳錯誤 */
                                        if (inFunc_CheckFullSpace(srECROb->srTransData.szBarCodeData) == VS_TRUE)
                                        {
                                                if (ginDebug == VS_TRUE)
                                                {
                                                        memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                                                        sprintf(szDebugMsg, "%s: %s  ", "BarCode Data", "Not Exist Error");
                                                        inLogPrintf(AT, szDebugMsg);
                                                }
                                                memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
                                                inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
                                                memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
                                                sprintf(pobTran->szErrorMsgBuff2, "BarCode Data Error");
                                                pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
						pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;		/* 操作錯誤 & 交易流程有誤 */
                                                /* [20260421] 刪除 _MIRROR_MSG_8N1_Standard_OPER_ERR_ 重覆傳送資料,只留以下資料 */
						if (pobTran->srTrustRec.uszTrustTransBit == VS_TRUE)
						{
							inECR_SendMirror(pobTran, _MIRROR_MSG_8N1_Standard_OPER_ERR_);
						}

                                                return (VS_ERROR);
                                        }
				}

                                inBarCodeLen = atoi(srECROb->srTransData.szBarCodeLen);
                                if (inBarCodeLen > 0)
                                {
                                        if (pobTran->srEWRec.uszEWTransBit == VS_TRUE)
                                        {
                                                memcpy(pobTran->srBRec.szBarCodeData, srECROb->srTransData.szBarCodeData, 25);
                                        }
                                        else if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
                                        {
                                                memcpy(pobTran->srBRec.szBarCodeData, srECROb->srTransData.szBarCodeData, inBarCodeLen);
                                        }
                                        else 
                                        {
                                                memcpy(pobTran->srBRec.szBarCodeData, srECROb->srTransData.szBarCodeData, inBarCodeLen);
                                        }
                                }

                                break;
                        case _ECR_8N1_REFUND_NO_:
                                if (srECROb->srTransData.srEW_Data.uszEWTransBit == VS_TRUE)
                                {
                                        if (ginDebug == VS_TRUE)
                                        {
                                                memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                                                sprintf(szDebugMsg, "%s :%s", "BarCode Data Len", srECROb->srTransData.szBarCodeLen);
                                                inLogPrintf(AT, szDebugMsg);
                                                memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                                                sprintf(szDebugMsg, "%s :%s", "BarCode Data", srECROb->srTransData.szBarCodeData);
                                                inLogPrintf(AT, szDebugMsg);
                                        }

                                        /* 欄位為M 卻不存在，回傳錯誤 */
                                        if (inFunc_CheckFullSpace(srECROb->srTransData.szBarCodeLen) == VS_TRUE)
                                        {
                                                if (ginDebug == VS_TRUE)
                                                {
                                                        memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                                                        sprintf(szDebugMsg, "%s: %s  ", "BarCode Data Len", "Not Exist Error");
                                                        inLogPrintf(AT, szDebugMsg);
                                                }
                                                memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
                                                inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
                                                memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
                                                sprintf(pobTran->szErrorMsgBuff2, "BarCode Data Len Error");
                                                pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

                                                return (VS_ERROR);
                                        }
                                        
                                        inBarCodeLen = atoi(srECROb->srTransData.szBarCodeLen);
                                }
                                else if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
                                {
                                        /* uny退貨不檢核barCode*/
                                }
                                else
                                {
                                        if (ginDebug == VS_TRUE)
                                        {
                                                memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                                                sprintf(szDebugMsg, "%s :%s", "BarCode Data Len", srECROb->srTransData.szBarCodeLen);
                                                inLogPrintf(AT, szDebugMsg);
                                                memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                                                sprintf(szDebugMsg, "%s :%s", "BarCode Data", srECROb->srTransData.szBarCodeData);
                                                inLogPrintf(AT, szDebugMsg);
                                        }

                                        inBarCodeLen = atoi(srECROb->srTransData.szBarCodeLen);
                                        if (inBarCodeLen > 0)
                                        {
                                                memcpy(pobTran->srBRec.szBarCodeData, srECROb->srTransData.szBarCodeData, inBarCodeLen);
                                        }
                                }

                                break;
                        default :
                                break;
                }
	
		/* 電子錢包不需要Uny交易碼 */
		if (srECROb->srTransData.srEW_Data.uszEWTransBit == VS_TRUE)
		{
			
		}
		else if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
		{
			/* Uny交易碼 */
			switch (inTransType)
			{
				case _ECR_8N1_REFUND_NO_ :
				case _ECR_8N1_INSTALLMENT_REFUND_NO_ :
				case _ECR_8N1_REDEEM_REFUND_NO_ :
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s :%s", "Uny Trans Code", srECROb->srTransData.szUnyTransCode);
						inLogPrintf(AT, szDebugMsg);
					}

					/* 欄位為M 卻不存在，回傳錯誤 */
					if (inFunc_CheckFullSpace(srECROb->srTransData.szUnyTransCode) == VS_TRUE)
					{
						if (ginDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
							sprintf(szDebugMsg, "%s: %s  ", "Uny Trans Code", "Not Exist Error");
							inLogPrintf(AT, szDebugMsg);
						}
						memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
						inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
						memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
						sprintf(pobTran->szErrorMsgBuff2, "Uny Trans Code Error");
						pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

						return (VS_ERROR);
					}

					memcpy(pobTran->srBRec.szUnyTransCode, srECROb->srTransData.szUnyTransCode, 20);
					break;
				default :
					break;
			}
		}
	}
	
	if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
	{
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
	}
	
	if (srECROb->srTransData.srEW_Data.uszEWTransBit == VS_TRUE)
	{
		inHDTIndex = -1;
		inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_EW_, &inHDTIndex);
		if (inHDTIndex != -1)
		{
			pobTran->srBRec.inHDTIndex = inHDTIndex;
			inLoadHDTRec(pobTran->srBRec.inHDTIndex);
			
			memset(szHostEnable, 0x00, sizeof(szHostEnable));
			inGetHostEnable(szHostEnable);
			if (szHostEnable[0] != 'Y')
			{
				/* Host沒開 */
				if (ginDebug == VS_TRUE) 
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "%s Not Open Error", szHostLabel);
					inLogPrintf(AT, szDebugMsg);
				}

				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "%s : HOST ID Not Open", szHostLabel);
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

				/* 如果return VS_ERROR 代表Host沒開 */
				return (VS_ERROR);
			}
			
			inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
			memset(szBatchNum, 0x00, sizeof(szBatchNum));
			inGetBatchNum(szBatchNum);
			pobTran->srBRec.lnBatchNum = atol(szBatchNum);
			
			memset(szSTANNum, 0x00, sizeof(szSTANNum));
			inGetSTANNum(szSTANNum);
			pobTran->srBRec.lnSTANNum = atol(szSTANNum);

			memset(szInvoiceNum, 0x00, sizeof(szInvoiceNum));
			inGetInvoiceNum(szInvoiceNum);
			pobTran->srBRec.lnOrgInvNum = atol(szInvoiceNum);
		}
		else
		{
			memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
			inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
			memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
			sprintf(pobTran->szErrorMsgBuff2, "%s : HOST ID Not Exist", szHostLabel);
			pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

			return (VS_ERROR);
		}
	}
        
	if (inECR_Check_Exception(pobTran) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	memset(szFiscContactlessEnable, 0x00, sizeof(szFiscContactlessEnable));
	inGetSmartPayContactlessEnable(szFiscContactlessEnable);
	memset(szCTLSEnable, 0x00, sizeof(szCTLSEnable));
	inGetContactlessEnable(szCTLSEnable);
	memset(szCUPContactlessEnable, 0x00, sizeof(szCUPContactlessEnable));
	inGetCUPContactlessEnable(szCUPContactlessEnable);
	/* 跑OPT */
	switch (inTransType)
	{
		case _ECR_8N1_SALE_NO_:
			/* 金融卡和一般Sale跑不同OPT，所以要分 */
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				/* 第三層顯示 ＜消費扣款＞ */
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_FISC_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜消費扣款＞ */

				pobTran->inFunctionID = _FISC_SALE_;
				if (!memcmp(szCTLSEnable, "Y", 1)		&&
				    !memcmp(szFiscContactlessEnable, "Y", 1)	&&
				    guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_FISC_SALE_CTLS_;
					pobTran->inRunTRTID = _TRT_FISC_SALE_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_FISC_SALE_;
					pobTran->inRunTRTID = _TRT_FISC_SALE_ICC_;
				}
		
				pobTran->inTransactionCode = _FISC_SALE_;
				pobTran->srBRec.inCode = _FISC_SALE_;
				pobTran->srBRec.inOrgCode = _FISC_SALE_;
				
				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_CUP_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜銀聯一般交易＞ */

				if (!memcmp(szCTLSEnable, "Y", 1)		&&
				    !memcmp(szCUPContactlessEnable, "Y", 1)	&&
				    guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
					pobTran->inRunTRTID = _TRT_CUP_SALE_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
					pobTran->inRunTRTID = _TRT_CUP_SALE_;
				}

				pobTran->inFunctionID = _CUP_SALE_;
				pobTran->inTransactionCode = _CUP_SALE_;
				pobTran->srBRec.inCode = pobTran->inFunctionID;
				pobTran->srBRec.inOrgCode = pobTran->inFunctionID;
				
				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_TICKET_DEDUCT_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜購貨交易＞ */

				pobTran->inFunctionID = _TICKET_DEDUCT_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_TICKET_DEDUCT_;
				}
				else
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "感應未初始化，不能使用此功能");
					}
					return (VS_ERROR);
				}

				pobTran->inTransactionCode = _TICKET_DEDUCT_;
				pobTran->srTRec.inCode = _TICKET_DEDUCT_;
				
				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else if (pobTran->srEWRec.uszEWTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
                                /* [113110-信託需求][UI] 修改抬頭黑底長度不夠問題,加入 inDISP_Display_Black_Back()  2025/11/20 */
                                inDISP_Display_Black_Back(0, _COORDINATE_Y_LINE_8_3_, _LCD_XSIZE_, _LCD_YSIZE_/ 8);
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned("                ", _FONTSIZE_8X16_, _COLOR_BLACK_, _COLOR_BLACK_, _COORDINATE_Y_LINE_8_3_, VS_FALSE, _DISP_CENTER_);
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned("電子錢包", _FONTSIZE_8X16_, _COLOR_WHITE_, _COLOR_BLACK_, _COORDINATE_Y_LINE_8_3_, VS_FALSE, _DISP_CENTER_);		/* 第三層顯示 ＜電子錢包＞ */
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned("交易處理中", _FONTSIZE_8X16_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_Y_LINE_8_5_, VS_FALSE, _DISP_CENTER_);		/* 第五層顯示 "交易處理中" */
				
				pobTran->inTransactionCode = _EW_SALE_;
				pobTran->srBRec.inCode = _EW_SALE_;
				pobTran->inRunTRTID = _TRT_EW_SALE_;
			}
			else if (pobTran->srTrustRec.uszTrustTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				/* 第三層顯示 ＜信託專區＞ */
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_1_);				/* 第一層顯示 LOGO */
				inDISP_Display_Black_Back(0, _COORDINATE_Y_LINE_8_2_, _LCD_XSIZE_, _LCD_YSIZE_/ 8);
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned("信託專區", _FONTSIZE_8X16_, _COLOR_WHITE_, _COLOR_BLACK_, _COORDINATE_Y_LINE_8_2_, VS_FALSE, _DISP_CENTER_);
				
				pobTran->inTransactionCode = _TRUST_SALE_;
                                pobTran->srTrustRec.inCode = _TRUST_SALE_;
				pobTran->inRunOperationID = _OPERATION_TRUST_SALE_;
				pobTran->inRunTRTID = _TRT_TRUST_SALE_;
				pobTran->srTrustRec.uszTrustTransBit = VS_TRUE;
				
				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			else
			{
				inDISP_ClearAll();
				/* 第三層顯示 ＜一般交易＞ */
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般交易＞ */

				pobTran->inFunctionID = _SALE_;
				
				/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/10 上午 10:32 */
				if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_SALE_BARCODE_;
					pobTran->inRunTRTID = _TRT_SALE_;
				}
				else if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
					pobTran->inRunTRTID = _TRT_SALE_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
					pobTran->inRunTRTID = _TRT_SALE_;
				}

				pobTran->inTransactionCode = _SALE_;
				pobTran->srBRec.inCode = _SALE_;
				pobTran->srBRec.inOrgCode = _SALE_;
				
				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			
			break;
		case _ECR_8N1_VOID_NO_:
			/* 金融卡和一般Sale跑不同OPT，所以要分 */
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_FISC_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜沖正交易＞ */

				pobTran->inFunctionID = _FISC_VOID_;
				pobTran->inRunOperationID = _OPERATION_VOID_;
				pobTran->inRunTRTID = _TRT_FISC_VOID_;

				pobTran->inTransactionCode = _FISC_VOID_;
				pobTran->srBRec.inCode = _FISC_VOID_;
				pobTran->srBRec.inOrgCode = _FISC_VOID_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜取消交易＞ */

				pobTran->inRunOperationID = _OPERATION_VOID_;
				pobTran->inRunTRTID = _TRT_CUP_VOID_;

				pobTran->inFunctionID = _CUP_VOID_;
				pobTran->inTransactionCode = _CUP_VOID_;
				pobTran->srBRec.inCode = pobTran->inTransactionCode;
				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else if (pobTran->srTrustRec.uszTrustTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				/* 第三層顯示 ＜信託專區＞ */
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_1_);				/* 第一層顯示 LOGO */
				inDISP_Display_Black_Back(0, _COORDINATE_Y_LINE_8_2_, _LCD_XSIZE_, _LCD_YSIZE_/ 8);
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned("信託專區", _FONTSIZE_8X16_, _COLOR_WHITE_, _COLOR_BLACK_, _COORDINATE_Y_LINE_8_2_, VS_FALSE, _DISP_CENTER_);
				
				pobTran->inTransactionCode = _TRUST_VOID_;
                                pobTran->srTrustRec.inCode = _TRUST_VOID_;
				pobTran->inRunOperationID = _OPERATION_TRUST_VOID_;
				pobTran->inRunTRTID = _TRT_TRUST_VOID_;
				pobTran->srTrustRec.uszTrustTransBit = VS_TRUE;
				
				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜取消交易＞ */

				pobTran->inFunctionID = _VOID_;
				pobTran->inRunOperationID = _OPERATION_VOID_;
				pobTran->inRunTRTID = _TRT_VOID_;

				pobTran->inTransactionCode = _VOID_;
				pobTran->srBRec.inCode = _VOID_;
				pobTran->srBRec.inOrgCode = _VOID_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			break;
		case _ECR_8N1_REFUND_NO_:
			/* 金融卡和一般Sale跑不同OPT，所以要分 */
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_FISC_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);	/* 第二層顯示 ＜退費交易＞ */

				pobTran->inFunctionID = _FISC_REFUND_;
				if (!memcmp(szCTLSEnable, "Y", 1)		&&
				    !memcmp(szFiscContactlessEnable, "Y", 1)	&&
				    guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_FISC_REFUND_CTLS_;
					pobTran->inRunTRTID = _TRT_FISC_REFUND_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_FISC_REFUND_;
					pobTran->inRunTRTID = _TRT_FISC_REFUND_ICC_;
				}

				pobTran->inTransactionCode = _FISC_REFUND_;
				pobTran->srBRec.inCode = _FISC_REFUND_;
				pobTran->srBRec.inOrgCode = _FISC_REFUND_;
				
				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			else if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般退貨＞ */

				if (!memcmp(szCTLSEnable, "Y", 1)		&&
				    !memcmp(szCUPContactlessEnable, "Y", 1)	&&
				    guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_GET_CARD_FIRST_;
					pobTran->inRunTRTID = _TRT_CUP_REFUND_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_AMOUNT_FIRST_CUP_;
					pobTran->inRunTRTID = _TRT_CUP_REFUND_;
				}

				pobTran->inFunctionID = _CUP_REFUND_;
				pobTran->inTransactionCode = _CUP_REFUND_;
				pobTran->srBRec.inCode = pobTran->inTransactionCode;
				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
				
				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_TICKET_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜退貨交易＞ */

				pobTran->inFunctionID = _TICKET_REFUND_;
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_TICKET_REFUND_;
				}
				else
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "感應未初始化，不能使用此功能");
					}
					return (VS_ERROR);
				}

				pobTran->inTransactionCode = _TICKET_REFUND_;
				pobTran->srTRec.inCode = _TICKET_REFUND_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else if (pobTran->srEWRec.uszEWTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
                                /* [113110-信託需求][UI] 修改抬頭黑底長度不夠問題,加入 inDISP_Display_Black_Back()  2025/11/20 */
                                inDISP_Display_Black_Back(0, _COORDINATE_Y_LINE_8_3_, _LCD_XSIZE_, _LCD_YSIZE_/ 8);
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned("                ", _FONTSIZE_8X16_, _COLOR_BLACK_, _COLOR_BLACK_, _COORDINATE_Y_LINE_8_3_, VS_FALSE, _DISP_CENTER_);
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned("電子錢包", _FONTSIZE_8X16_, _COLOR_WHITE_, _COLOR_BLACK_, _COORDINATE_Y_LINE_8_3_, VS_FALSE, _DISP_CENTER_);		/* 第三層顯示 ＜電子錢包＞ */
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned("交易處理中", _FONTSIZE_8X16_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_Y_LINE_8_5_, VS_FALSE, _DISP_CENTER_);		/* 第五層顯示 "交易處理中" */
				
				pobTran->inTransactionCode = _EW_REFUND_;
				pobTran->srBRec.inCode = _EW_REFUND_;
				pobTran->inRunTRTID = _TRT_EW_REFUND_;
			}
			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般退貨＞ */

				pobTran->inFunctionID = _REFUND_;
				/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/10 上午 10:32 */
				if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_BARCODE_;
				}
				else if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_GET_CARD_FIRST_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_AMOUNT_FIRST_;
				}
				pobTran->inRunTRTID = _TRT_REFUND_;

				pobTran->inTransactionCode = _REFUND_;
				pobTran->srBRec.inCode = _REFUND_;
				pobTran->srBRec.inOrgCode = _REFUND_;
				
				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;
			
		case _ECR_8N1_OFFLINE_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_SALE_OFFLINE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜交易補登＞ */
			
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_041_CASH_, _CUSTOMER_INDICATOR_SIZE_))
			{
				pobTran->inFunctionID = _FORCE_CASH_ADVANCE_;
				pobTran->inRunOperationID = _OPERATION_SALE_OFFLINE_;
				pobTran->inRunTRTID = _TRT_FORCE_CASH_ADVANCE_;

				pobTran->inTransactionCode = _FORCE_CASH_ADVANCE_;
				pobTran->srBRec.inCode = _FORCE_CASH_ADVANCE_;
				pobTran->srBRec.inOrgCode = _FORCE_CASH_ADVANCE_;
			}
			else
			{
				pobTran->inFunctionID = _SALE_OFFLINE_;
				pobTran->inRunOperationID = _OPERATION_SALE_OFFLINE_;
				pobTran->inRunTRTID = _TRT_SALE_OFFLINE_;

				pobTran->inTransactionCode = _SALE_OFFLINE_;
				pobTran->srBRec.inCode = _SALE_OFFLINE_;
				pobTran->srBRec.inOrgCode = _SALE_OFFLINE_;
			}
			
			/* 卡號查詢不過第二次卡，所以要直接決定TRT */
			if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
			{
				inNCCC_Func_Decide_CTLS_TRT(pobTran);
			}
			else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
			{
				inNCCC_Func_Decide_ICC_TRT(pobTran);
			}
			else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
			{
				inNCCC_Func_Decide_MEG_TRT(pobTran);
			}
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			break;
			
		case _ECR_8N1_PREAUTH_NO_:
			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_PRE_AUTH_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜預先授權＞ */

				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_PRE_AUTH_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_PRE_AUTH_;
				}
				pobTran->inRunTRTID = _TRT_CUP_PRE_AUTH_;

				pobTran->inFunctionID = _CUP_PRE_AUTH_;
				pobTran->inTransactionCode = _CUP_PRE_AUTH_;
				pobTran->srBRec.inCode = pobTran->inTransactionCode;
				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
				
				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_PRE_AUTH_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜預先授權＞ */

				pobTran->inFunctionID = _PRE_AUTH_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_PRE_AUTH_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_PRE_AUTH_;
				}
				pobTran->inRunTRTID = _TRT_PRE_AUTH_;

				pobTran->inTransactionCode = _PRE_AUTH_;
				pobTran->srBRec.inCode = _PRE_AUTH_;
				pobTran->srBRec.inOrgCode = _PRE_AUTH_;
				
				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			
			break;
			
		case _ECR_8N1_PREAUTH_CANCEL_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_PRE_AUTH_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜預先授權取消＞ */
			
			pobTran->inFunctionID = _CUP_PRE_AUTH_VOID_;
			pobTran->inRunOperationID = _OPERATION_VOID_;
			pobTran->inRunTRTID = _TRT_CUP_PRE_AUTH_VOID_;

			pobTran->inTransactionCode = _CUP_PRE_AUTH_VOID_;
			pobTran->srBRec.inCode = _CUP_PRE_AUTH_VOID_;
			pobTran->srBRec.inOrgCode = _CUP_PRE_AUTH_VOID_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			break;
			
		case _ECR_8N1_PREAUTH_COMPLETE_NO_:
			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_PRE_COMP_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜預先授權完成＞ */

				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_PRE_COMP_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_PRE_COMP_;
				}
				pobTran->inRunTRTID = _TRT_CUP_PRE_COMP_;

				pobTran->inFunctionID = _CUP_PRE_COMP_;
				pobTran->inTransactionCode = _CUP_PRE_COMP_;
				pobTran->srBRec.inCode = pobTran->inTransactionCode;
				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
				
				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_PRE_COMP_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜預先授權完成＞ */

				pobTran->inFunctionID = _PRE_COMP_;
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_PRE_COMP_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_PRE_COMP_;
				}
				pobTran->inRunTRTID = _TRT_PRE_COMP_;

				pobTran->inTransactionCode = _PRE_COMP_;
				pobTran->srBRec.inCode = _PRE_COMP_;
				pobTran->srBRec.inOrgCode = _PRE_COMP_;
				
				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;
			
		case _ECR_8N1_PREAUTH_COMPLETE_CANCEL_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);					/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_PRE_COMP_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜預先授權完成取消＞ */
			
			pobTran->inFunctionID = _CUP_PRE_COMP_VOID_;
			pobTran->inRunOperationID = _OPERATION_VOID_;
			pobTran->inRunTRTID = _TRT_CUP_PRE_COMP_VOID_;

			pobTran->inTransactionCode = _CUP_PRE_COMP_VOID_;
			pobTran->srBRec.inCode = _CUP_PRE_COMP_VOID_;
			pobTran->srBRec.inOrgCode = _CUP_PRE_COMP_VOID_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			break;
			
		case _ECR_8N1_INSTALLMENT_NO_:
//			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
//			{
//				inDISP_ClearAll();
//				/* 保留給銀聯分期使用 */
//				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
//				inDISP_PutGraphic(_MENU_INST_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜分期付款＞ */
//
//				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
//				{
//					pobTran->inRunOperationID = _OPERATION_INST_SALE_CTLS_;
//				}
//				else
//				{
//					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
//				}
//				pobTran->inRunTRTID = _TRT_CUP_INST_SALE_;
//
//				pobTran->inFunctionID = _CUP_INST_SALE_;
//				pobTran->inTransactionCode = _CUP_INST_SALE_;
//				pobTran->srBRec.inCode = pobTran->inTransactionCode;
//				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
//
//				pobTran->srBRec.uszInstallmentBit = VS_TRUE;
//				
//				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
//				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
//				{
//					inNCCC_Func_Decide_CTLS_TRT(pobTran);
//				}
//				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
//				{
//					inNCCC_Func_Decide_ICC_TRT(pobTran);
//				}
//				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
//				{
//					inNCCC_Func_Decide_MEG_TRT(pobTran);
//				}
//
//				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
//			}
//			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_INST_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜分期付款＞ */

				pobTran->inFunctionID = _INST_SALE_;
				
				/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/10 上午 10:32 */
				if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_INST_SALE_BARCODE_;
				}
				else if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_INST_SALE_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
				}
				pobTran->inRunTRTID = _TRT_INST_SALE_;

				pobTran->inTransactionCode = _INST_SALE_;
				pobTran->srBRec.inCode = _INST_SALE_;
				pobTran->srBRec.inOrgCode = _INST_SALE_;

				pobTran->srBRec.uszInstallmentBit = VS_TRUE;
				
				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;
			
		case _ECR_8N1_INSTALLMENT_REFUND_NO_:
//			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
//			{
//				inDISP_ClearAll();
//				/* 保留給銀聯分期退貨使用 */
//				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
//				inDISP_PutGraphic(_MENU_INST_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜分期退貨＞ */
//
//				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
//				{
//					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_GET_CARD_FIRST_;
//				}
//				else
//				{
//					pobTran->inRunOperationID = _OPERATION_REFUND_;
//				}
//				pobTran->inRunTRTID = _TRT_CUP_INST_REFUND_;
//
//				pobTran->inFunctionID = _CUP_INST_REFUND_;
//				pobTran->inTransactionCode = _CUP_INST_REFUND_;
//				pobTran->srBRec.inCode = pobTran->inTransactionCode;
//				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
//
//				pobTran->srBRec.uszInstallmentBit = VS_TRUE;
//				
//				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
//				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
//				{
//					inNCCC_Func_Decide_CTLS_TRT(pobTran);
//				}
//				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
//				{
//					inNCCC_Func_Decide_ICC_TRT(pobTran);
//				}
//				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
//				{
//					inNCCC_Func_Decide_MEG_TRT(pobTran);
//				}
//
//				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
//			}
//			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_INST_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜分期退貨＞ */

				pobTran->inFunctionID = _INST_REFUND_;
				
				/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/10 上午 10:32 */
				if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_INST_REFUND_BARCODE_;
				}
				else if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_GET_CARD_FIRST_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_;
				}
				pobTran->inRunTRTID = _TRT_INST_REFUND_;

				pobTran->inTransactionCode = _INST_REFUND_;
				pobTran->srBRec.inCode = _INST_REFUND_;
				pobTran->srBRec.inOrgCode = _INST_REFUND_;

				pobTran->srBRec.uszInstallmentBit = VS_TRUE;
				
				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;
			
		case _ECR_8N1_INSTALLMENT_ADJUST_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_INST_ADJUST_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜分期調帳＞ */
			
			pobTran->inFunctionID = _INST_ADJUST_;
			pobTran->inRunOperationID = _OPERATION_INST_ADJUST_;
			pobTran->inRunTRTID = _TRT_INST_ADJUST_;

			pobTran->inTransactionCode = _INST_ADJUST_;
			pobTran->srBRec.inCode = _INST_ADJUST_;
			pobTran->srBRec.inOrgCode = _INST_ADJUST_;
			
			pobTran->srBRec.uszInstallmentBit = VS_TRUE;
			
			/* 卡號查詢不過第二次卡，所以要直接決定TRT */
			if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
			{
				inNCCC_Func_Decide_CTLS_TRT(pobTran);
			}
			else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
			{
				inNCCC_Func_Decide_ICC_TRT(pobTran);
			}
			else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
			{
				inNCCC_Func_Decide_MEG_TRT(pobTran);
			}
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			break;
			
		case _ECR_8N1_REDEEM_NO_:
//			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
//			{
//				inDISP_ClearAll();
//				/* 保留給銀聯紅利扣抵使用 */
//				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
//				inDISP_PutGraphic(_MENU_REDEEM_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利扣抵＞ */
//
//				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
//				{
//					pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
//				}
//				else
//				{
//					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
//				}
//				pobTran->inRunTRTID = _TRT_REDEEM_SALE_;
//
//				pobTran->inFunctionID = _CUP_REDEEM_SALE_;
//				pobTran->inTransactionCode = _CUP_REDEEM_SALE_;
//				pobTran->srBRec.inCode = pobTran->inTransactionCode;
//				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
//
//				pobTran->srBRec.uszRedeemBit = VS_TRUE;
//				
//				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
//				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
//				{
//					inNCCC_Func_Decide_CTLS_TRT(pobTran);
//				}
//				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
//				{
//					inNCCC_Func_Decide_ICC_TRT(pobTran);
//				}
//				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
//				{
//					inNCCC_Func_Decide_MEG_TRT(pobTran);
//				}
//
//				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
//			}
//			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REDEEM_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利扣抵＞ */

				pobTran->inFunctionID = _REDEEM_SALE_;
				
				/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/10 上午 10:32 */
				if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REDEEM_SALE_BARCODE_;
				}
				else if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REDEEM_SALE_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
				}
				pobTran->inRunTRTID = _TRT_REDEEM_SALE_;

				pobTran->inTransactionCode = _REDEEM_SALE_;
				pobTran->srBRec.inCode = _REDEEM_SALE_;
				pobTran->srBRec.inOrgCode = _REDEEM_SALE_;

				pobTran->srBRec.uszRedeemBit = VS_TRUE;
				
				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;
		
		case _ECR_8N1_REDEEM_REFUND_NO_:
//			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
//			{
//				inDISP_ClearAll();
//				/* 保留給銀聯紅利退貨使用 */
//				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
//				inDISP_PutGraphic(_MENU_REDEEM_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利退貨＞ */
//
//				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
//				{
//					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_GET_CARD_FIRST_;
//				}
//				else
//				{
//					pobTran->inRunOperationID = _OPERATION_REFUND_;
//				}
//				pobTran->inRunTRTID = _TRT_CUP_REDEEM_REFUND_;
//
//				pobTran->inFunctionID = _CUP_REDEEM_REFUND_;
//				pobTran->inTransactionCode = _CUP_REDEEM_REFUND_;
//				pobTran->srBRec.inCode = pobTran->inTransactionCode;
//				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
//
//				pobTran->srBRec.uszRedeemBit = VS_TRUE;
//				
//				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
//				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
//				{
//					inNCCC_Func_Decide_CTLS_TRT(pobTran);
//				}
//				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
//				{
//					inNCCC_Func_Decide_ICC_TRT(pobTran);
//				}
//				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
//				{
//					inNCCC_Func_Decide_MEG_TRT(pobTran);
//				}
//
//				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
//			}
//			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REDEEM_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利退貨＞ */

				pobTran->inFunctionID = _REDEEM_REFUND_;
				
				/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/10 上午 10:32 */
				if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REDEEM_REFUND_BARCODE_;
				}
				else if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_GET_CARD_FIRST_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_;
				}
				pobTran->inRunTRTID = _TRT_REDEEM_REFUND_;

				pobTran->inTransactionCode = _REDEEM_REFUND_;
				pobTran->srBRec.inCode = _REDEEM_REFUND_;
				pobTran->srBRec.inOrgCode = _REDEEM_REFUND_;

				pobTran->srBRec.uszRedeemBit = VS_TRUE;
				
				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;
			
		case _ECR_8N1_REDEEM_ADJUST_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_REDEEM_ADJUST_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利調帳＞ */
			
			pobTran->inFunctionID = _REDEEM_ADJUST_;
			pobTran->inRunOperationID = _OPERATION_REDEEM_ADJUST_;
			pobTran->inRunTRTID = _TRT_REDEEM_ADJUST_;

			pobTran->inTransactionCode = _REDEEM_ADJUST_;
			pobTran->srBRec.inCode = _REDEEM_ADJUST_;
			pobTran->srBRec.inOrgCode = _REDEEM_ADJUST_;
			
			pobTran->srBRec.uszRedeemBit = VS_TRUE;
			
			/* 卡號查詢不過第二次卡，所以要直接決定TRT */
			if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
			{
				inNCCC_Func_Decide_CTLS_TRT(pobTran);
			}
			else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
			{
				inNCCC_Func_Decide_ICC_TRT(pobTran);
			}
			else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
			{
				inNCCC_Func_Decide_MEG_TRT(pobTran);
			}
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
			
		case _ECR_8N1_TIP_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_TIP_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜小費交易＞ */
			
			pobTran->inFunctionID = _TIP_;
			pobTran->inRunOperationID = _OPERATION_TIP_;
			pobTran->inRunTRTID = _TRT_TIP_;

			pobTran->inTransactionCode = _TIP_;
			pobTran->srBRec.inCode = _TIP_;
			pobTran->srBRec.inOrgCode = _TIP_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
			
		case _ECR_8N1_AWARD_REDEEM_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_LOYALTY_REDEEM_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜優惠兌換＞ */
			
			pobTran->inFunctionID = _LOYALTY_REDEEM_;
			/* 收銀機以條碼兌換*/
			if (pobTran->szL3_AwardWay[0] == '1')
			{
				pobTran->inRunOperationID = _OPERATION_BARCODE_;
			}
			else
			{
				pobTran->inRunOperationID = _OPERATION_LOYALTY_REDEEM_CTLS_;
			}
			pobTran->inRunTRTID = _TRT_LOYALTY_REDEEM_;

			pobTran->inTransactionCode = _LOYALTY_REDEEM_;
			pobTran->srBRec.inCode = _LOYALTY_REDEEM_;
			pobTran->srBRec.inOrgCode = _LOYALTY_REDEEM_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
			
		case _ECR_8N1_VOID_AWARD_REDEEM_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_VOID_LOYALTY_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜兌換取消＞ */
			
			pobTran->inFunctionID = _VOID_LOYALTY_REDEEM_;
			pobTran->inRunOperationID = _OPERATION_VOID_LOYALTY_REDEEM_;
			pobTran->inRunTRTID = _TRT_LOYALTY_REDEEM_;

			pobTran->inTransactionCode = _VOID_LOYALTY_REDEEM_;
			pobTran->srBRec.inCode = _VOID_LOYALTY_REDEEM_;
			pobTran->srBRec.inOrgCode = _VOID_LOYALTY_REDEEM_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;


		/* 快樂購紅利積點 */
		case _ECR_8N1_HG_REWARD_SALE_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡紅利積點＞ */
			
			pobTran->inFunctionID = _SALE_;
			pobTran->inRunOperationID = _OPERATION_HG_;
			pobTran->inRunTRTID = _TRT_HG_REWARD_CREDIT_CTLS_;

			/* 這邊先設成sale，若之後支付方式選銀聯卡，會在那邊改成CUP_SALE */
			pobTran->inTransactionCode = _SALE_;
			pobTran->srBRec.inCode = _SALE_;
			pobTran->srBRec.inOrgCode = _SALE_;
			pobTran->srBRec.lnHGTransactionType = _HG_REWARD_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		
		/* 快樂購紅利積點─分期付款 */
		case _ECR_8N1_HG_REWARD_INSTALLMENT_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_INS_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡分期付款＞ */
			
			pobTran->inFunctionID = _INST_SALE_;
			pobTran->inRunOperationID = _OPERATION_HG_I_R_;
			pobTran->inRunTRTID = _TRT_HG_REWARD_INSTALLMENT_CTLS_;

			pobTran->inTransactionCode = _INST_SALE_;
			pobTran->srBRec.inCode = _INST_SALE_;
			pobTran->srBRec.inOrgCode = _INST_SALE_;
			pobTran->srBRec.lnHGTransactionType = _HG_REWARD_;
			pobTran->srBRec.uszInstallmentBit = VS_TRUE;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		
		/* 快樂購紅利積點─紅利扣抵 */
		case _ECR_8N1_HG_REWARD_REDEMPTION_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_REDEEM_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡紅利扣抵＞ */
			
			pobTran->inFunctionID = _REDEEM_SALE_;
			pobTran->inRunOperationID = _OPERATION_HG_I_R_;
			pobTran->inRunTRTID = _TRT_HG_REWARD_REDEMPTION_CTLS_;

			pobTran->inTransactionCode = _REDEEM_SALE_;
			pobTran->srBRec.inCode = _REDEEM_SALE_;
			pobTran->srBRec.inOrgCode = _REDEEM_SALE_;
			pobTran->srBRec.lnHGTransactionType = _HG_REWARD_;
			pobTran->srBRec.uszRedeemBit = VS_TRUE;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
			
		/* 快樂購點數扣抵 */
		case _ECR_8N1_HG_ONLINE_REDEEMPTION_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_ONLINE_REDEEM_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡點數扣抵＞ */
			
			pobTran->inFunctionID = _SALE_;
			pobTran->inRunOperationID = _OPERATION_HG_;
			pobTran->inRunTRTID = _TRT_HG_ONLINE_REDEEM_CREDIT_CTLS_;

			pobTran->inTransactionCode = _SALE_;
			pobTran->srBRec.inCode = _SALE_;
			pobTran->srBRec.inOrgCode = _SALE_;
			pobTran->srBRec.lnHGTransactionType = _HG_ONLINE_REDEEM_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		
		/* 快樂購加價購 */
		case _ECR_8N1_HG_POINT_CERTAIN_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_POINT_CERTAIN_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡加價購＞ */
			
			pobTran->inFunctionID = _SALE_;
			pobTran->inRunOperationID = _OPERATION_HG_;
			pobTran->inRunTRTID = _TRT_HG_POINT_CERTAIN_CREDIT_CTLS_;

			pobTran->inTransactionCode = _SALE_;
			pobTran->srBRec.inCode = _SALE_;
			pobTran->srBRec.inOrgCode = _SALE_;
			pobTran->srBRec.lnHGTransactionType = _HG_POINT_CERTAIN_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		
		/* 快樂購點數兌換 */
		case _ECR_8N1_HG_FULL_REDEEMPTION_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_REDEMPTION_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡點數兌換＞ */
			
			pobTran->inFunctionID = _SALE_;
			pobTran->inRunOperationID = _OPERATION_HG_;
			pobTran->inRunTRTID = _TRT_HG_FULL_REDEMPTION_;

			pobTran->inTransactionCode = _SALE_;
			pobTran->srBRec.inCode = _SALE_;
			pobTran->srBRec.inOrgCode = _SALE_;
			pobTran->srBRec.lnHGTransactionType = _HG_FULL_REDEMPTION_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		
		/* 快樂購扣抵退貨 */
		case _ECR_8N1_HG_REDEEM_REFUND_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_REDEEM_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡扣抵退貨＞ */
			
			pobTran->inFunctionID = _HG_REDEEM_REFUND_;
			pobTran->inRunOperationID = _OPERATION_HG_REFUND_;
			pobTran->inRunTRTID = _TRT_HG_REDEEM_REFUND_;

			pobTran->inTransactionCode = _HG_REDEEM_REFUND_;
			pobTran->srBRec.inCode = _HG_REDEEM_REFUND_;
			pobTran->srBRec.inOrgCode = _HG_REDEEM_REFUND_;
			pobTran->srBRec.lnHGTransactionType = _HG_REDEEM_REFUND_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		
		/* 快樂購回饋退貨 */
		case _ECR_8N1_HG_REWARD_REFUND_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡回饋退貨＞ */
			
			pobTran->inFunctionID = _HG_REWARD_REFUND_;
			pobTran->inRunOperationID = _OPERATION_HG_REFUND_;
			pobTran->inRunTRTID = _TRT_HG_REWARD_REFUND_;

			pobTran->inTransactionCode = _HG_REWARD_REFUND_;
			pobTran->srBRec.inCode = _HG_REWARD_REFUND_;
			pobTran->srBRec.inOrgCode = _HG_REWARD_REFUND_;
			pobTran->srBRec.lnHGTransactionType = _HG_REWARD_REFUND_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		
		/* 快樂購點數查詢 */
		case _ECR_8N1_HG_POINT_INQUIRY_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_INQUIRY_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡點數查詢＞ */
			
			pobTran->inFunctionID = _HG_INQUIRY_;
			pobTran->inRunOperationID = _OPERATION_HG_;
			pobTran->inRunTRTID = _TRT_HG_INQUIRY_;

			pobTran->inTransactionCode = _HG_INQUIRY_;
			pobTran->srBRec.inCode = _HG_INQUIRY_;
			pobTran->srBRec.inOrgCode = _HG_INQUIRY_;
			pobTran->srBRec.lnHGTransactionType = _HG_INQUIRY_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
			
		case _ECR_8N1_SETTLEMENT_NO_:
			inDISP_ClearAll();
			/* 第三層顯示 ＜結帳交易＞ */
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_SETTLE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜結帳交易＞ */

			pobTran->inFunctionID = _SETTLE_;
			pobTran->inRunOperationID = _OPERATION_SETTLE_;
			pobTran->inRunTRTID = _TRT_SETTLE_;
			
			pobTran->inTransactionCode = _SETTLE_;
			pobTran->srBRec.inCode = _SETTLE_;
			pobTran->srBRec.inOrgCode = _SETTLE_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		
		case _ECR_8N1_REPRINT_RECEIPT_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_REPRINT_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜重印帳單＞ */

			pobTran->inFunctionID = FALSE;
			pobTran->inRunOperationID = _OPERATION_REPRINT_;
			pobTran->inRunTRTID = FALSE;
			
			pobTran->inTransactionCode = FALSE;
			pobTran->srBRec.inCode = FALSE;
			pobTran->srBRec.inOrgCode = FALSE;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
				
		case _ECR_8N1_MENU_REVIEW_DETAIL_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_REVIEW_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 <交易查詢> */
			
			pobTran->inFunctionID = FALSE;
			pobTran->inRunOperationID = _OPERATION_REVIEW_DETAIL_;
			pobTran->inRunTRTID = FALSE;
			
			pobTran->inTransactionCode = FALSE;
			pobTran->srBRec.inCode = FALSE;
			pobTran->srBRec.inOrgCode = FALSE;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		
		case _ECR_8N1_MENU_REVIEW_TOTAL_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_REVIEW_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜總額查詢＞ */
			
			pobTran->inFunctionID = FALSE;
			pobTran->inRunOperationID = _OPERATION_REVIEW_TOTAL_;
			pobTran->inRunTRTID = FALSE;
			
			pobTran->inTransactionCode = FALSE;
			pobTran->srBRec.inCode = FALSE;
			pobTran->srBRec.inOrgCode = FALSE;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
			
		case _ECR_8N1_MENU_REPORT_DETAIL_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_DETAIL_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 明細列印 */
			
			pobTran->inFunctionID = FALSE;
			pobTran->inRunOperationID = _OPERATION_DETAIL_REPORT_;
			pobTran->inRunTRTID = FALSE;
			
			pobTran->inTransactionCode = FALSE;
			pobTran->srBRec.inCode = FALSE;
			pobTran->srBRec.inOrgCode = FALSE;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
			
		case _ECR_8N1_MENU_REPORT_TOTAL_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_TOTAIL_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 總額列印 */
			
			pobTran->inFunctionID = FALSE;
			pobTran->inRunOperationID = _OPERATION_TOTAL_REPORT_;
			pobTran->inRunTRTID = FALSE;
			
			pobTran->inTransactionCode = FALSE;
			pobTran->srBRec.inCode = FALSE;
			pobTran->srBRec.inOrgCode = FALSE;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		case _ECR_8N1_EDC_REBOOT_NO_:
			inDISP_ClearAll();
			/* 第三層顯示 ＜重新開機＞ */
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_REBOOT_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜重新開機＞ */
			
			pobTran->inFunctionID = FALSE;
			pobTran->inRunOperationID = _OPERATION_EDC_REBOOT_;
			pobTran->inRunTRTID = FALSE;
			
			pobTran->inTransactionCode = FALSE;
			pobTran->srBRec.inCode = FALSE;
			pobTran->srBRec.inOrgCode = FALSE;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			
			break;
		case _ECR_8N1_ESVC_TOP_UP_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_TICKET_TOP_UP_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜加值交易＞ */

			pobTran->inFunctionID = _TICKET_TOP_UP_;

			if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
			{
				pobTran->inRunOperationID = _OPERATION_TICKET_TOP_UP_;
			}
			else
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "感應未初始化，不能使用此功能");
				}
				return (VS_ERROR);
			}

			pobTran->inTransactionCode = _TICKET_TOP_UP_;
			pobTran->srTRec.inCode = _TICKET_TOP_UP_;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		case _ECR_8N1_ESVC_BALANCE_INQUIRY_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0, _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_TICKET_INQUIRY_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜餘額查詢＞ */

			pobTran->inFunctionID = _TICKET_INQUIRY_;

			if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
			{
				pobTran->inRunOperationID = _OPERATION_TICKET_INQUIRY_;
			}
			else
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "感應未初始化，不能使用此功能");
				}
				return (VS_ERROR);
			}

			pobTran->inTransactionCode = _TICKET_INQUIRY_;
			pobTran->srTRec.inCode = _TICKET_INQUIRY_;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		case _ECR_8N1_ESVC_VOID_TOP_UP_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);					/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_TICKET_VOID_TOP_UP_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜加值取消＞ */

			pobTran->inFunctionID = _TICKET_VOID_TOP_UP_;

			if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
			{
				pobTran->inRunOperationID = _OPERATION_TICKET_VOID_TOP_UP_;
			}
			else
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "感應未初始化，不能使用此功能");
				}
				return (VS_ERROR);
			}

			pobTran->inTransactionCode = _TICKET_VOID_TOP_UP_;
			pobTran->srTRec.inCode = _TICKET_VOID_TOP_UP_;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		case _ECR_8N1_END_CARD_NO_INQUIRY_NO_:
			if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
			{
				/* 電子票證兩段式，start trans type填70  */
				pobTran->inFunctionID = _TICKET_INQUIRY_;

				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_TICKET_INQUIRY_;
				}
				else
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "感應未初始化，不能使用此功能");
					}
					return (VS_ERROR);
				}
				pobTran->inTransactionCode = _TICKET_INQUIRY_;
				pobTran->srTRec.inCode = _TICKET_INQUIRY_;
				
				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			else
			{
				pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_END_INQUIRY_SUCCESS_;
				inRetVal = VS_ERROR;
			}
			break;
		/* 查詢上一筆 */
		case _ECR_8N1_INQUIRY_LAST_TRANSACTION_NO_:
			inRetVal = VS_SUCCESS;
			break;
		case _ECR_8N1_ECHO_NO_:
			inNCCC_Func_Disp_EchoTest();
			inRetVal = VS_SUCCESS;
			break;
		case _ECR_8N1_EW_INQUIRY_TRANSACTION_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
                        /* [113110-信託需求][UI] 修改抬頭黑底長度不夠問題,加入 inDISP_Display_Black_Back()  2025/11/20 */
                        inDISP_Display_Black_Back(0, _COORDINATE_Y_LINE_8_3_, _LCD_XSIZE_, _LCD_YSIZE_/ 8);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned("                ", _FONTSIZE_8X16_, _COLOR_BLACK_, _COLOR_BLACK_, _COORDINATE_Y_LINE_8_3_, VS_FALSE, _DISP_CENTER_);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned("電子錢包", _FONTSIZE_8X16_, _COLOR_WHITE_, _COLOR_BLACK_, _COORDINATE_Y_LINE_8_3_, VS_FALSE, _DISP_CENTER_);		/* 第三層顯示 ＜電子錢包＞ */
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned("交易處理中", _FONTSIZE_8X16_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_Y_LINE_8_5_, VS_FALSE, _DISP_CENTER_);		/* 第五層顯示 "交易處理中" */

			pobTran->inTransactionCode = _EW_INQUIRY_;
			pobTran->srBRec.inCode = _EW_INQUIRY_;
			pobTran->inRunTRTID = _TRT_EW_INQUIRY_;
			break;
		case _ECR_8N1_VOID_EDC_TRANS_NO_: /*[115150]*/
		{	
			pobTran->inTransactionCode = _VOID_TRANS_;
			pobTran->srBRec.inCode = _VOID_TRANS_;
			pobTran->srBRec.inOrgCode = _VOID_TRANS_;
			inRetVal = VS_SUCCESS;
			break;
		}
		default:
			pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
			inRetVal = VS_ERROR;
			break;
	}
	
	return (inRetVal);
}

/*
Function        :inRS232_ECR_8N1_Standard_Pack
Date&Time       :2016/7/6 下午 2:40
Describe        :先把要送的資料組好
*/
int inECR_8N1_Standard_Pack(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer)
{
	int	inRetVal = VS_ERROR;
	int	i = 0, inCardLen = 0;
	int	inPacketSizes = 0;
	int	inLen = 0;
	int	inBarCodeLen = 0;
	char	szTemplate[100 + 1];
	char	szHash[44 + 1];
	char	szFESMode[2 + 1];
	char	szTemp[8 + 1] = {0};
	char	szTemp2[8 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
	char szEnableESG[2] = {0};
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);

	/* ECR Indicator (1 Byte) */
	memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szECRIndicator, 1);
	inPacketSizes ++;
	/* ECR Version Date (6 Byte) */
	/* [114197] 中心收單作業ESG永續發展優化 
	 * [260612] 修改版本期 251120 */
	memcpy(&szDataBuffer[inPacketSizes], "251120", 6);
	inPacketSizes += 6;
	/* Trans Type Indicator (1 Byte) */
	inPacketSizes ++;
	/* Trans Type (2 Byte) */
	memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szTransType, 2);
	inPacketSizes += 2;
	/* CUP Indicator (1 Byte) */
	if (!memcmp(srECROb->srTransData.szField_05, _ECR_TRUST_INDICATOR_, 1) || pobTran->srTrustRec.uszTrustTransBit == VS_TRUE)
	{
		memcpy(&szDataBuffer[inPacketSizes], _ECR_TRUST_INDICATOR_, 1);
	}
	else if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_START_CARD_NO_INQUIRY_, 2)	||
		 !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2)			||
		 !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REFUND_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_CANCEL_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_CANCEL_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_TOP_UP_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_BALANCE_INQUIRY_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_VOID_TOP_UP_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_SALE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_CERTAIN_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_EW_INQUIRY_TRANSACTION_, 2))
	{
		if (!memcmp(srECROb->srTransData.szField_05, "C", 1) || pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			memcpy(&szDataBuffer[inPacketSizes], "C", 1);
		else if (!memcmp(srECROb->srTransData.szField_05, "S", 1) || pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			memcpy(&szDataBuffer[inPacketSizes], "S", 1);
		else if (!memcmp(srECROb->srTransData.szField_05, "E", 1) || pobTran->srTRec.uszESVCTransBit == VS_TRUE)
			memcpy(&szDataBuffer[inPacketSizes], "E", 1);
		else if (!memcmp(srECROb->srTransData.szField_05, _ECR_EW_INDICATOR_, 1) || pobTran->srEWRec.uszEWTransBit == VS_TRUE)
			memcpy(&szDataBuffer[inPacketSizes], _ECR_EW_INDICATOR_, 1);
		else
		        memcpy(&szDataBuffer[inPacketSizes], "N", 1);
	}

	inPacketSizes ++;

	/* HOST ID (2 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REFUND_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_OFFLINE_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TIP_, 2)				||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_CANCEL_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_CANCEL_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_REFUND_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_ADJUST_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_REFUND_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_ADJUST_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_TOP_UP_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_VOID_TOP_UP_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_INQUIRY_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_SALE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_INSTALLMENT_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REDEMPTION_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_CERTAIN_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_FULL_REDEEMPTION_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REDEEM_REFUND_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REFUND_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_AWARD_REDEEM_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_AWARD_REDEEM_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_EW_INQUIRY_TRANSACTION_, 2))
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTRTFileName(szTemplate);
		
		if (!memcmp(szTemplate, _TRT_FILE_NAME_CREDIT_, strlen(_TRT_FILE_NAME_CREDIT_)))
			memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_HOSTID_NCCC_, 2);
		/* 2018/5/8 下午 2:10 DFS需求不再使用大來主機 */
		else if (!memcmp(szTemplate, _TRT_FILE_NAME_HG_, strlen(_TRT_FILE_NAME_HG_)))
			memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_HOSTID_HG_, 2);
		else if (!memcmp(szTemplate, _TRT_FILE_NAME_DCC_, strlen(_TRT_FILE_NAME_DCC_)))
			memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_HOSTID_DCC_, 2);
		else if (!memcmp(szTemplate, _TRT_FILE_NAME_ESVC_, strlen(_TRT_FILE_NAME_ESVC_)))
			memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_HOSTID_ESVC_, 2);
		else if (!memcmp(szTemplate, _TRT_FILE_NAME_EW_, strlen(_TRT_FILE_NAME_EW_)))
			memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_HOSTID_EW_, 2);
		else if (!memcmp(szTemplate, _TRT_FILE_NAME_TRUST_, strlen(_TRT_FILE_NAME_TRUST_)))
			memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_HOSTID_TRUST_, 2);
	}

	inPacketSizes += 2;

	if (pobTran->srTrustRec.uszTrustTransBit == VS_TRUE)
	{
		if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2)			||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_, 2))
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%s", pobTran->srTrustRec.szTrustRRN);
			memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
		}
                inPacketSizes += 12;
		
		/* Reserved */
		inPacketSizes += 17;
	}
	else
	{
		/* Receipt No (6 Byte) */
		if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2)			||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REFUND_, 2)			||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_, 2)			||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_OFFLINE_, 2)			||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TIP_, 2)				||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_, 2)			||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_CANCEL_, 2)		||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_, 2)		||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_CANCEL_, 2)	||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_, 2)			||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_REFUND_, 2)		||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_ADJUST_, 2)		||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_, 2)			||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_REFUND_, 2)		||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_ADJUST_, 2)		||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_TOP_UP_, 2)			||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_VOID_TOP_UP_, 2)		||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_INQUIRY_, 2)		||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_SALE_, 2)		||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_INSTALLMENT_, 2)	||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REDEMPTION_, 2)	||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2)	||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_CERTAIN_, 2)		||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_FULL_REDEEMPTION_, 2)		||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REDEEM_REFUND_, 2)		||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REFUND_, 2)		||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_AWARD_REDEEM_, 2)		||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_AWARD_REDEEM_, 2)		||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_EW_INQUIRY_TRANSACTION_, 2))
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
			{
				sprintf(szTemplate, "%06ld", pobTran->srTRec.lnInvNum);
			}
			else
			{
				sprintf(szTemplate, "%06ld", pobTran->srBRec.lnOrgInvNum);
			}
			memcpy(&szDataBuffer[inPacketSizes], szTemplate, 6);
		}

		inPacketSizes += 6;

		/* Card No (19 Byte) */
		if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_END_CARD_NO_INQUIRY_, 2)		||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SETTLEMENT_, 2)			||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REPRINT_RECEIPT_, 2)		||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REVIEW_DETAIL_, 2)		||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REVIEW_TOTAL_, 2)		||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REPORT_DETAIL_, 2)		||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REPORT_TOTAL_, 2)		||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_EDC_REBOOT_, 2)			||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_INQUIRY_, 2)		||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_FULL_REDEEMPTION_, 2)		||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REDEEM_REFUND_, 2)		||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REFUND_, 2)		||
		    (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_AWARD_REDEEM_, 2) && !(!memcmp(pobTran->szL3_AwardWay, "4", 1) || !memcmp(pobTran->szL3_AwardWay, "5", 1)))	||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_AWARD_REDEEM_, 2)		||
		    pobTran->srBRec.uszHappyGoSingle == VS_TRUE)
		{
			/* 優惠兌換先設定不回傳卡號 */
			inPacketSizes += 19;
		}
		else
		{
			if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));

				if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
				{
					if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
					{
						memset(szTemplate, 0x00, sizeof(szTemplate));
						inLen = strlen(pobTran->srTRec.srECCRec.szCardID);
						memcpy(szTemplate, pobTran->srTRec.srECCRec.szCardID, inLen);
					}
					else
					{
						memset(szTemplate, 0x00, sizeof(szTemplate));
						inLen = strlen(pobTran->srTRec.szUID);
						memcpy(szTemplate, pobTran->srTRec.szUID, inLen);
					}
				}
				else
				{
					/* 一卡通：遮掩後2 碼，遮掩字元為”*” */
					if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
					{
						memset(szTemplate, 0x00, sizeof(szTemplate));
						inLen = strlen(pobTran->srTRec.szUID);
						memcpy(szTemplate, pobTran->srTRec.szUID, inLen);
						szTemplate[inLen - 1] = 0x2A;
						szTemplate[inLen - 2] = 0x2A;
					}
					/*	1.簽單卡號列印邏輯
						<T4800>票卡版號(Purse Version Number)=0，<T0200>列印晶片卡號(Card Id)
						<T0200>20 bytes，14th,15th,16th隱碼，17th,18th,19th,20th顯示
						<T4800>票卡版號(Purse Version Number)<>0，<T0211>列印外觀卡號(Purse Id)
						<T0211>16 bytes，6th,7th,8th,9th,10th,11th隱碼，12th,13th,14th,15th,16th顯示
						，遮掩字元為”*”
						以<T0200>悠遊卡卡號遮掩從後往前算第五碼，第六碼，第七碼遮掩
						範例:以8碼卡號為例，卡號11651733，1,6,5要遮掩，實際列印為1***1733
						1th 2th 3th 4th 5th 6th 7th 8th 9th 10th 11th 12th 13th 14th 15th 16th 17th
						0 0 0 0 0 0 0 0 0 1 1 6 5 1 7 3 3
						0 0 0 0 0 0 0 0 0 1 * * * 1 7 3 3
						範例:以10碼卡號為例，卡號6611651733，1,6,5要遮掩，實際列印為661***1733
						1th 2th 3th 4th 5th 6th 7th 8th 9th 10th 11th 12th 13th 14th 15th 16th 17th
						0 0 0 0 0 0 0 6 6 1 1 6 5 1 7 3 3
						0 0 0 0 0 0 0 6 6 1 * * * 1 7 3 3

						2. 電子發票載具卡號為T0200 (不遮掩)
					*/
					else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
					{
						memset(szTemplate, 0x00, sizeof(szTemplate));
						inLen = strlen(pobTran->srTRec.szUID);
						memcpy(szTemplate, pobTran->srTRec.szUID, inLen);

						if (atoi(pobTran->srTRec.srECCRec.szPurseVersionNumber) == 0)
						{
							szTemplate[inLen - 5] = 0x2A;
							szTemplate[inLen - 6] = 0x2A;
							szTemplate[inLen - 7] = 0x2A;
						}
						else
						{
							/* 全部16 第6-11隱碼 */
							szTemplate[inLen - 6] = 0x2A;
							szTemplate[inLen - 7] = 0x2A;
							szTemplate[inLen - 8] = 0x2A;
							szTemplate[inLen - 9] = 0x2A;
							szTemplate[inLen - 10] = 0x2A;
							szTemplate[inLen - 11] = 0x2A;
						}
					}
					else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
					{
						memset(szTemplate, 0x00, sizeof(szTemplate));
						inLen = strlen(pobTran->srTRec.szUID);
						memcpy(szTemplate, pobTran->srTRec.szUID, inLen);
						/* 愛金卡：遮掩9-12 碼，遮掩字元為"*" */
						szTemplate[8] = 0x2A;
						szTemplate[9] = 0x2A;
						szTemplate[10] = 0x2A;
						szTemplate[11] = 0x2A;
					}
				}

				memcpy(&szDataBuffer[inPacketSizes], szTemplate, strlen(szTemplate));
			}
			/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/3 下午 6:18 */
			/* 不參考TMS遮掩開關 */
			else if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
			{
				memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szPAN, strlen(pobTran->srBRec.szPAN));
			}
			else if (pobTran->srEWRec.uszEWTransBit == VS_TRUE)
			{
				if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2))
				{
					/* 悠遊付：當消費者支付工具為”1：信用卡”時，遮掩的信用卡卡號 */
					/* 全支付：遮掩的信用卡卡號或帳戶 */
					memcpy(&szDataBuffer[inPacketSizes], pobTran->srEWRec.szEW_MaskCreditCardNo, strlen(pobTran->srEWRec.szEW_MaskCreditCardNo));
				}
				else if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_EW_INQUIRY_TRANSACTION_, 2))
				{
					/* 悠遊付：當消費者支付工具為”1：信用卡”時，遮掩的信用卡卡號 */
					/* icash Pay：卡號前六、後四碼，中間*號遮蔽 */
					/* 全支付：遮掩的信用卡卡號或帳戶 */
					memcpy(&szDataBuffer[inPacketSizes], pobTran->srEWRec.szEW_MaskCreditCardNo, strlen(pobTran->srEWRec.szEW_MaskCreditCardNo));
				}
			}
			else
			{
				/* 卡號是否遮掩 */
				inRetVal = inECR_CardNoTruncateDecision(pobTran);
				/* 要遮卡號 */
				if (inRetVal == VS_SUCCESS)
				{
					memset(szTemplate, 0x00, sizeof(szTemplate));
					strcpy(szTemplate, pobTran->srBRec.szPAN);

					/* HAPPG_GO 卡不掩飾 */
					if (!memcmp(&pobTran->srBRec.szPAN[0], "9552", 4))
					{

					}
					else
					{
						/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
						/* 卡號長度 */
						inCardLen = strlen(szTemplate);

						if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_088_TINTIN_, _CUSTOMER_INDICATOR_SIZE_))
						{
							/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
							for (i = 8; i < (inCardLen - 4); i ++)
								szTemplate[i] = '*';
						}
						else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
						{
							/* 第一段查詢交易，回覆信用卡前8後2卡號 */
							if (pobTran->uszCardInquiryFirstBit == VS_TRUE ||
							    pobTran->uszCardInquirysSecondBit == VS_TRUE)
							{
								for (i = 8; i < (inCardLen - 2); i ++)
									szTemplate[i] = '*';
							}
							else
							{
								for (i = 6; i < (inCardLen - 4); i ++)
									szTemplate[i] = '*';
							}
						}
						else
						{    
							/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
							for (i = 6; i < (inCardLen - 4); i ++)
								szTemplate[i] = '*';
						}
					}
					memcpy(&szDataBuffer[inPacketSizes], szTemplate, strlen(szTemplate));
				}
				else
				{
					memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szPAN, strlen(pobTran->srBRec.szPAN));
				}
			}
			inPacketSizes += 19;
		}

		/* Card Expire Date (4 Byte) */
		if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_END_CARD_NO_INQUIRY_, 2)	||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SETTLEMENT_, 2)		||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REPRINT_RECEIPT_, 2)	||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REVIEW_DETAIL_, 2)	||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REVIEW_TOTAL_, 2)	||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REPORT_DETAIL_, 2)	||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REPORT_TOTAL_, 2)	||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_EDC_REBOOT_, 2)		||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_INQUIRY_, 2)	||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REDEEM_REFUND_, 2)	||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REFUND_, 2)	||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_FULL_REDEEMPTION_, 2))
		{
			inPacketSizes += 4;
		}
		else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
		{
			inPacketSizes += 4;
		}
		/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/3 下午 6:18 */
		/* Uny交易不回傳有效期 */
		else if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
		{
			inPacketSizes += 4;
		}
		/* 電子錢包交易不回傳有效期 */
		/* (需求單-111155)-電子錢包業者收付訊息整合平台需求 by Russell 2023/7/24 下午 6:17 */
		else if (pobTran->srEWRec.uszEWTransBit == VS_TRUE)
		{
			inPacketSizes += 4;
		}
		else
		{
			/* 有效期是否回傳 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetECRExpDateReturnEnable(szTemplate);

			/* 要回傳有效期 */
			if (!memcmp(&szTemplate, "Y", 1))
				memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szExpDate, strlen(pobTran->srBRec.szExpDate));

			inPacketSizes += 4;
		}
	}

	/* Trans Amount (12 Byte) */
	if (pobTran->srTrustRec.uszTrustTransBit == VS_TRUE)
	{
		/* 因為信託交易必定要回交易金額，所以必須擺在最上面 */
                if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2)			||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_, 2))
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "%010lu00", pobTran->srTrustRec.lnTxnAmount);
                        memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
		}

		inPacketSizes += 12;
	}
	else if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TIP_, 2)			||
		 !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_, 2)			||
		 !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_CANCEL_, 2)		||
		 !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_CANCEL_, 2)	||
		 !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_START_CARD_NO_INQUIRY_, 2)	||
		 !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_END_CARD_NO_INQUIRY_, 2)		||
		 !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SETTLEMENT_, 2)			||
		 !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REPRINT_RECEIPT_, 2)		||
		 !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REVIEW_DETAIL_, 2)		||
		 !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REVIEW_TOTAL_, 2)		||
		 !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REPORT_DETAIL_, 2)		||
		 !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REPORT_TOTAL_, 2)		||
		 !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_EDC_REBOOT_, 2)			||
		 !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_INQUIRY_, 2)		||
		 !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_FULL_REDEEMPTION_, 2)		||
		 !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REFUND_, 2)		||
		 !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REDEEM_REFUND_, 2)		||
		 !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_EDC_TRANS_, 2))/*[115150]*/
	{
	        inPacketSizes += 12;
	}
        else if (pobTran->srBRec.uszUPlanTransBit == VS_TRUE)
        {
                if (pobTran->srBRec.uszVOIDBit == VS_TRUE)
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "%010lu00", pobTran->srBRec.lnTxnAmount);
                        memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
                }
                else
                {
                    if (pobTran->srBRec.lnCUPUPlanDiscountedAmount > 0)
                    {
                            memset(szTemplate, 0x00, sizeof(szTemplate));
                            sprintf(szTemplate, "%010lu00", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnCUPUPlanPreferentialAmount));
                            memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
                    }
                    else
                    {
                            memset(szTemplate, 0x00, sizeof(szTemplate));
                            sprintf(szTemplate, "%010lu00", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnCUPUPlanPreferentialAmount));
                            memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
                    }
                }
                inPacketSizes += 12;
        }
        else
        {
                if (pobTran->srTRec.lnTxnAmount != 0)
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "%010lu00", pobTran->srTRec.lnTxnAmount);
                        memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);

                        inPacketSizes += 12;
                }
                else if (pobTran->srBRec.lnTxnAmount != 0)
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "%010lu00", pobTran->srBRec.lnTxnAmount);
                        memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);

                        inPacketSizes += 12;
                }
                else if (pobTran->srEWRec.uszEWTransBit == VS_TRUE)
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "%010lu", pobTran->srEWRec.lnTxnAmount);
                        memcpy(&szDataBuffer[inPacketSizes], szTemplate, 10);
                        inPacketSizes += 10;

                        memcpy(&szDataBuffer[inPacketSizes], pobTran->srEWRec.szEW_Decimal, 2);
                        inPacketSizes += 2;
                }
                else
                {
                        inPacketSizes += 12;
                }
        }

	/* Trans Date (6 Byte) & Trans Time (6 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REFUND_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_OFFLINE_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TIP_, 2)				||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_CANCEL_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_CANCEL_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_REFUND_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_ADJUST_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_REFUND_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_ADJUST_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_TOP_UP_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_VOID_TOP_UP_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SETTLEMENT_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_INQUIRY_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_SALE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_INSTALLMENT_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REDEMPTION_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_CERTAIN_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_FULL_REDEEMPTION_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REDEEM_REFUND_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REFUND_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_AWARD_REDEEM_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_AWARD_REDEEM_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ECHO_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_EW_INQUIRY_TRANSACTION_, 2) ||
		!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_EDC_TRANS_, 2))/*[115150]*/
	{
		if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
		{
			/* Trans Date */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			strcat(szTemplate, pobTran->srTRec.szDate);
			memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 6);
			inPacketSizes += 6;
                        
			/* Trans Time */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			strcat(szTemplate, pobTran->srTRec.szTime);
			memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 6);
			inPacketSizes += 6;
		}
		else
		{
			/* Trans Date */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			strcat(szTemplate, pobTran->srBRec.szDate);
			memcpy(&szDataBuffer[inPacketSizes], &szTemplate[2], 6);
			inPacketSizes += 6;
			/* Trans Time */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			strcat(szTemplate, pobTran->srBRec.szTime);
			memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 6);
			inPacketSizes += 6;
		}
	}
	else
	{
		inPacketSizes += 6;
		inPacketSizes += 6;
	}

	/* Approval No (9 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REFUND_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_OFFLINE_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TIP_, 2)				||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_CANCEL_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_CANCEL_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_REFUND_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_ADJUST_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_REFUND_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_ADJUST_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_TOP_UP_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_BALANCE_INQUIRY_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_VOID_TOP_UP_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_SALE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_INSTALLMENT_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REDEMPTION_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_CERTAIN_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_FULL_REDEEMPTION_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REDEEM_REFUND_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REFUND_, 2))
	{
		if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			strcpy(szTemplate, pobTran->srTRec.szAuthCode);
			inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 9, _PADDING_RIGHT_);
			memcpy(&szDataBuffer[inPacketSizes], szTemplate, 9);
		}
		/* 電子錢包交易不回傳授權碼 */
		/* (需求單-111155)-電子錢包業者收付訊息整合平台需求 by Russell 2023/7/24 下午 6:17 */
		else if (pobTran->srEWRec.uszEWTransBit == VS_TRUE)
		{
			
		}
		else
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			strcpy(szTemplate, pobTran->srBRec.szAuthCode);
			inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 9, _PADDING_RIGHT_);
			memcpy(&szDataBuffer[inPacketSizes], szTemplate, 9);
		}
	}

	inPacketSizes += 9;

	/* Wave Card Indicator (1 Byte) */
	if (!memcmp(&srECROb->srTransData.szECRIndicator[0], "Q", 1))
	{
		/* 電子錢包交易不回傳Wave Card Indicator */
		/* (需求單-111155)-電子錢包業者收付訊息整合平台需求 by Russell 2023/7/24 下午 6:17 */
		/* Indicator為Q目前不送此欄位 */
	}
	else
	{
		if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
		{
			if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2)			||
			    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_START_CARD_NO_INQUIRY_, 2)	||
			    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_TOP_UP_, 2)			||
			    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_BALANCE_INQUIRY_, 2)	||
			    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_VOID_TOP_UP_, 2))
			{
				if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
				{
					memcpy(&szDataBuffer[inPacketSizes], "P", 1);
				}
				else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
				{
					memcpy(&szDataBuffer[inPacketSizes], "Z", 1);
				}
				else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
				{
					memcpy(&szDataBuffer[inPacketSizes], "G", 1);
				}
				else
				{
					memcpy(&szDataBuffer[inPacketSizes], "O", 1);
				}
			}
		}
		else
		{
			if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2)			||
			    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_START_CARD_NO_INQUIRY_, 2)	||
			    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_TOP_UP_, 2)			||
			    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_BALANCE_INQUIRY_, 2)	||
			    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_VOID_TOP_UP_, 2))
			{
				if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
				{
					/* 規格未寫M 跟單機同步 */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					strcpy(szTemplate, pobTran->srBRec.szCardLabel);

					if (!memcmp(szTemplate, _CARD_TYPE_VISA_, strlen(_CARD_TYPE_VISA_)))
						memcpy(&szDataBuffer[inPacketSizes], "V", 1);
					else if (!memcmp(szTemplate, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)))
						memcpy(&szDataBuffer[inPacketSizes], "M", 1);
					else if (!memcmp(szTemplate, _CARD_TYPE_JCB_, strlen(_CARD_TYPE_JCB_)))
						memcpy(&szDataBuffer[inPacketSizes], "J", 1);
					else if (!memcmp(szTemplate, _CARD_TYPE_CUP_, strlen(_CARD_TYPE_CUP_)))
						memcpy(&szDataBuffer[inPacketSizes], "C", 1);
					else if (!memcmp(szTemplate, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)))
						memcpy(&szDataBuffer[inPacketSizes], "A", 1);
					/* (需求單 - 106349) 2018/10/31 下午 3:41 by Russell */
					else if (!memcmp(szTemplate, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)) ||
						 pobTran->srBRec.uszUCARDTransBit == VS_TRUE)
					{
						if (pobTran->srBRec.uszUCARDTransBit == VS_TRUE)
						{
							memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szMCP_BANKID[4], 1);
						}
						else
						{
							memcpy(&szDataBuffer[inPacketSizes], "D", 1);
						}
					}
					else
						memcpy(&szDataBuffer[inPacketSizes], "O", 1);
				}
			}
		}
	}
	inPacketSizes ++;

	/* ECR Response Code (4 Byte) */
	memcpy(&szDataBuffer[inPacketSizes], "0000", 4);
	if(!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_EDC_TRANS_, 2))
        {
            memcpy(&szDataBuffer[inPacketSizes], "0006", 4);
        }
	inPacketSizes += 4;

	/* Merchant ID (15 Byte) & Terminal ID (8 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REPORT_DETAIL_, 2) ||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REPORT_TOTAL_, 2) ||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SETTLEMENT_, 2))
	{
		inPacketSizes += 15;
		inPacketSizes += 8;
	}
	else
	{
		/* Merchant ID (15 Byte) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetMerchantID(szTemplate);
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 15, _PADDING_RIGHT_);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 15);
		inPacketSizes += 15;
		/* Terminal ID (8 Byte) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTerminalID(szTemplate);
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 8, _PADDING_RIGHT_);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 8);
		inPacketSizes += 8;
	}

	/* Exp Amount (12 Byte) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTRTFileName(szTemplate);
        if (!memcmp(szTemplate, _TRT_FILE_NAME_DCC_, _TRT_FILE_NAME_DCC_LEN_) && !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TIP_, 2))
        {
                if (!memcmp(&pobTran->srBRec.szDCC_FCMU[0], "0", 1))
                {
        	        memset(szTemplate, 0x00, sizeof(szTemplate));
        		sprintf(szTemplate, "%010ld00", atol(pobTran->srBRec.szDCC_TIPFCA));
        		memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 12);
        	}
                else if (!memcmp(&pobTran->srBRec.szDCC_FCMU[0], "1", 1))
                {
        	        memset(szTemplate, 0x00, sizeof(szTemplate));
        		sprintf(szTemplate, "%011ld0", atol(pobTran->srBRec.szDCC_TIPFCA));
        		memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 12);
        	}
        	else if (!memcmp(&pobTran->srBRec.szDCC_FCMU[0], "2", 1))
                {
        	        memset(szTemplate, 0x00, sizeof(szTemplate));
        		sprintf(szTemplate, "%012ld", atol(pobTran->srBRec.szDCC_TIPFCA));
        		memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 12);
        	}
        }
        else if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TIP_, 2))
        {
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010lu00", pobTran->srBRec.lnTipTxnAmount);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
	}

	inPacketSizes += 12;
	
	/* Store Id (18 Byte) */
	inPacketSizes += 18;

        if (pobTran->srTrustRec.uszTrustTransBit == VS_TRUE)
        {
                /* 信託 */
                /* 銷帳編號 */
                if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2)	||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_, 2))
                {
                        if (strlen(pobTran->srTrustRec.szReconciliationNo) > 0)
                        {
                                memcpy(&szDataBuffer[inPacketSizes], &pobTran->srTrustRec.szReconciliationNo[0], 20);
                        }
                }
                inPacketSizes += 20;
                /* 信託機構代碼 */
                if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2)	||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_, 2))
                {
                        memcpy(&szDataBuffer[inPacketSizes], &pobTran->srTrustRec.szTrustInstitutionCode[0], 7);
                }
                inPacketSizes += 7;
                /* 交換平台序號 */
                if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2)	||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_, 2))
                {
                        memcpy(&szDataBuffer[inPacketSizes], &pobTran->srTrustRec.szExchangeTxSerialNumber[0], 32);
                }
                inPacketSizes += 32;
                /* 主機回復中文說明 */
                if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2)	||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_, 2))
                {
                        memcpy(&szDataBuffer[inPacketSizes], &pobTran->szHostResponseMessageBIG5[0], 20);
                }
                inPacketSizes += 20;
                /* 信託原交易日期 */
                inPacketSizes += 4;
                /* 保留 */
                inPacketSizes += 4;
        }
        else
        {
                /* 處理紅利、分期 */
                /* Installment / Redeem Indictor (1 Byte) */
                if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_, 2)		||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_REFUND_, 2)	||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_ADJUST_, 2)	||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_, 2)		||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_REFUND_, 2)	||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_ADJUST_, 2)	||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REDEMPTION_, 2) ||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_INSTALLMENT_, 2))
                {
                        if (pobTran->srBRec.uszInstallmentBit == VS_TRUE)
                                memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szInstallmentIndicator[0], 1);
                        else if (pobTran->srBRec.uszRedeemBit == VS_TRUE)
                                memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szRedeemIndicator[0], 1);
                }

                /* 優惠兌換方式 */
                /* 1.條碼兌換 2.卡號兌換 */
                if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_AWARD_REDEEM_, 2))
                {
                        if (!memcmp(pobTran->szL3_AwardWay, "1", 1)	||
                            !memcmp(pobTran->szL3_AwardWay, "2", 1)	||
                            !memcmp(pobTran->szL3_AwardWay, "3", 1))
                        {
                                memcpy(&szDataBuffer[inPacketSizes], "1", 1);
                        }
                        else if (!memcmp(pobTran->szL3_AwardWay, "4", 1)	||
                                 !memcmp(pobTran->szL3_AwardWay, "5", 1))
                        {
                                memcpy(&szDataBuffer[inPacketSizes], "2", 1);
                        }
                }
                else if(!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_AWARD_REDEEM_, 2))
                {
                        /* (1= 條碼兌換， 2= 卡號兌換 )*/
                        /* 取消優惠兌換只接受 條碼兌換 。 */
                        memcpy(&szDataBuffer[inPacketSizes], "1", 1);
                }
                inPacketSizes ++;

                /* 處理紅利扣抵 */
                if ((!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_, 2)		||
                     !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_REFUND_, 2)	||
                     !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_ADJUST_, 2)	||
                     !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REDEMPTION_, 2)) && pobTran->srBRec.uszRedeemBit == VS_TRUE)
                {
                        /* 支付金額 RDM Paid Amt (12 Byte) */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "%010lu00", (pobTran->srBRec.lnRedemptionPaidCreditAmount + pobTran->srBRec.lnTipTxnAmount));
                        memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
                        inPacketSizes += 12;
                        /* 紅利扣抵點數 RDM Point (8 Byte) */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "%08lu", pobTran->srBRec.lnRedemptionPoints);
                        memcpy(&szDataBuffer[inPacketSizes], szTemplate, 8);
                        inPacketSizes += 8;
                        /* 紅利剩餘點數 Points Of Balance (8 Byte) */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "%08lu", pobTran->srBRec.lnRedemptionPointsBalance);
                        memcpy(&szDataBuffer[inPacketSizes], szTemplate, 8);
                        inPacketSizes += 8;
                        /* 紅利扣抵金額 Redeem Amt (12 Byte) */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "%010lu00", (pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnRedemptionPaidCreditAmount));
                        memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
                        inPacketSizes += 12;
                }
                else if (pobTran->srBRec.uszUPlanTransBit == VS_TRUE)
                {
                        if (pobTran->srBRec.uszVOIDBit == VS_TRUE)
                        {
                                inPacketSizes += 40;
                        }
                        else
                        {
                                /* 銀聯優計劃：優惠後金額(含小數2 位) */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                sprintf(szTemplate, "%010lu00", pobTran->srBRec.lnCUPUPlanDiscountedAmount);
                                memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
                                inPacketSizes += 12;

                                /* reserved */
                                inPacketSizes += 8;

                                /* reserved */
                                inPacketSizes += 8;

                                /* 銀聯優計劃：優惠金額(含小數2 位) */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                sprintf(szTemplate, "%010lu00", pobTran->srBRec.lnCUPUPlanPreferentialAmount);
                                memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
                                inPacketSizes += 12;
                        }
                }
                else
                {
                        inPacketSizes += 40;
                }

                /* 處理分期交易 */
                if ((!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_, 2)	||
                     !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_REFUND_, 2)	||
                     !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_ADJUST_, 2)	||
                     !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_INSTALLMENT_, 2)) && pobTran->srBRec.uszInstallmentBit == VS_TRUE)
                {
                        /* 分期期數 Installment Period (2 Byte) */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "%02ld", pobTran->srBRec.lnInstallmentPeriod);
                        memcpy(&szDataBuffer[inPacketSizes], szTemplate, 2);
                        inPacketSizes += 2;
                        /* 首期金額 Down Payment (12 Byte) */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "%010lu00", pobTran->srBRec.lnInstallmentDownPayment);
                        memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
                        inPacketSizes += 12;
                        /* 每期金額 Installment Payment Amt (12 Byte) */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "%010lu00", pobTran->srBRec.lnInstallmentPayment);
                        memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
                        inPacketSizes += 12;
                        /* 分期手續費 Formlity Fee (12 Byte) */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "%010lu00", pobTran->srBRec.lnInstallmentFormalityFee);
                        memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
                        inPacketSizes += 12;
                }
                else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
                {
                        inPacketSizes += 2;

                        /* 交易前餘額 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_BALANCE_INQUIRY_, 2)	||
                            !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_START_CARD_NO_INQUIRY_, 2)	||
                            !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_END_CARD_NO_INQUIRY_, 2)		||
                            !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REPRINT_RECEIPT_, 2)		||
                            !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REVIEW_TOTAL_, 2)		||
                            !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REVIEW_DETAIL_, 2))
                        {
                                /* 查餘額不帶此欄位 */
                                /* 啟動卡號查詢不帶此欄位 */
                                sprintf(szTemplate, "            ");
                        }
                        else
                        {
                                if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
                                {
                                        if (pobTran->srTRec.lnFinalBeforeAmt > 100000)
                                        {
                                                sprintf(szTemplate, "-%09lu00", (pobTran->srTRec.lnFinalBeforeAmt - 100000));
                                        }
                                        else
                                        {
                                                /* 交易前餘額要帶未加值的金額 */
                                                if ((pobTran->srTRec.lnCardRemainAmount - pobTran->srTRec.lnTotalTopUpAmount) < 0)
                                                        sprintf(szTemplate, "-%09lu00", (unsigned long)(0 - (pobTran->srTRec.lnCardRemainAmount - pobTran->srTRec.lnTotalTopUpAmount)));
                                                else
                                                        sprintf(szTemplate, "+%09lu00", (unsigned long)pobTran->srTRec.lnCardRemainAmount - pobTran->srTRec.lnTotalTopUpAmount);
                                        }
                                }
                                else
                                {
                                        if (pobTran->srTRec.lnFinalBeforeAmt < 0)
                                        {
                                                sprintf(szTemplate, "-%09lu00", (0 - pobTran->srTRec.lnFinalBeforeAmt));
                                        }
                                        else
                                        {
                                                sprintf(szTemplate, "+%09lu00", pobTran->srTRec.lnFinalBeforeAmt);
                                        }
                                }

                        }

                        memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 12);
                        inPacketSizes += 12;

                        /* 交易後餘額 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_START_CARD_NO_INQUIRY_, 2)	||
                            !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_END_CARD_NO_INQUIRY_, 2)		||
                            !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REPRINT_RECEIPT_, 2)		||
                            !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REVIEW_TOTAL_, 2)		||
                            !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REVIEW_DETAIL_, 2))
                        {
                                /* 啟動卡號查詢不帶此欄位 */
                                sprintf(szTemplate, "            ");
                        }
                        else if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_BALANCE_INQUIRY_, 2))
                        {
                                if (pobTran->srTRec.lnCardRemainAmount < 0)
                                {
                                        sprintf(szTemplate, "-%09lu00", (unsigned long)(0 - pobTran->srTRec.lnCardRemainAmount));
                                }
                                else
                                {
                                        sprintf(szTemplate, "+%09lu00", (unsigned long)pobTran->srTRec.lnCardRemainAmount);
                                }
                        }
                        else
                        {
                                if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
                                {
                                        if (pobTran->srTRec.lnFinalAfterAmt > 100000)
                                                sprintf(szTemplate, "-%09lu00", (unsigned long)(pobTran->srTRec.lnFinalAfterAmt - 100000));
                                        else
                                                sprintf(szTemplate, "+%09lu00", (unsigned long)(pobTran->srTRec.lnFinalAfterAmt));
                                }
                                else
                                {
                                        if (pobTran->srTRec.lnFinalAfterAmt < 0)
                                                sprintf(szTemplate, "-%09lu00", (unsigned long)(0 - pobTran->srTRec.lnFinalAfterAmt));
                                        else
                                                sprintf(szTemplate, "+%09lu00", (unsigned long)(pobTran->srTRec.lnFinalAfterAmt));
                                }
                        }

                        memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 12);
                        inPacketSizes += 12;

                        /* 自動加值金額 */
                        if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2))
                        {
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                if (pobTran->srTRec.lnTotalTopUpAmount > 0L)
                                        sprintf(szTemplate, "+%09lu00", (unsigned long)(pobTran->srTRec.lnTotalTopUpAmount));
                                else
                                        sprintf(szTemplate, "+%09lu00", (unsigned long)(0));

                                memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 12);
                        }
                        inPacketSizes += 12;
                }
                else
                        inPacketSizes += 38;

                /* Card Type (2 Byte) */
                if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_START_CARD_NO_INQUIRY_, 2)					||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2)							||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REFUND_, 2)							||
                    (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_, 2) && pobTran->srBRec.uszHappyGoSingle != VS_TRUE)	||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_OFFLINE_, 2)							||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TIP_, 2)								||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_, 2)							||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_CANCEL_, 2)						||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_, 2)						||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_CANCEL_, 2)					||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_, 2)							||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_REFUND_, 2)						||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_ADJUST_, 2)						||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_, 2)							||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_REFUND_, 2)						||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_ADJUST_, 2)						||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_TOP_UP_, 2)							||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_BALANCE_INQUIRY_, 2)					||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_VOID_TOP_UP_, 2)						||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_INSTALLMENT_, 2)					||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REDEMPTION_, 2)					||
                    ((!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_SALE_, 2)						||
                      !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_CERTAIN_, 2)						||
                      !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2)) && pobTran->srBRec.uszHappyGoMulti == VS_TRUE)	||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_EW_INQUIRY_TRANSACTION_, 2))
                {
                        if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
                        {
                                if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
                                {
                                        memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_IPASS_, 2);
                                }
                                else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
                                {
                                        memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_ECC_, 2);
                                }
                                else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
                                {
                                        memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_ICASH_, 2);
                                }
                                else
                                        memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_UNKNOWN_, _ECR_8N1_NCCC_CARDTYPE_UNKNOWN_LEN_);
                        }
                        else if (pobTran->srEWRec.uszEWTransBit == VS_TRUE)
                        {
                                memcpy(&szDataBuffer[inPacketSizes], pobTran->srEWRec.szEW_EWIssuerID, _EW_ISSUERID_LEN_);
                        }
                        else
                        {
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                strcpy(szTemplate, pobTran->srBRec.szCardLabel);

                                /* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
                                if (!memcmp(szTemplate, _CARD_TYPE_VISA_, strlen(_CARD_TYPE_VISA_)))
                                        memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_VISA_, 2);
                                else if (!memcmp(szTemplate, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)))
                                        memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_MASTERCARD_, 2);
                                else if (!memcmp(szTemplate, _CARD_TYPE_JCB_, strlen(_CARD_TYPE_JCB_)))
                                        memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_JCB_, 2);
                                else if (!memcmp(szTemplate, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)))
                                        memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_AMEX_, 2);
                                else if (!memcmp(szTemplate, _CARD_TYPE_CUP_, strlen(_CARD_TYPE_CUP_)))
                                        memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_CUP_, 2);
                                else if (!memcmp(szTemplate, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)) ||
                                         pobTran->srBRec.uszUCARDTransBit == VS_TRUE)
                                {
                                        if (pobTran->srBRec.uszUCARDTransBit == VS_TRUE)
                                        {
                                                memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_UCARD_, 2);
                                        }
                                        else
                                        {
                                                memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_DINERS_, 2);
                                        }
                                }
                                else if (!memcmp(szTemplate, _CARD_TYPE_SMARTPAY_, strlen(_CARD_TYPE_SMARTPAY_)))
                                        memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_SMARTPAY_, 2);
                                else
                                        memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_UNKNOWN_, _ECR_8N1_NCCC_CARDTYPE_UNKNOWN_LEN_);
                        }
                }

                inPacketSizes += 2;

                /* Batch No (6 Byte) */
                if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2)			||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REFUND_, 2)			||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_, 2)			||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_OFFLINE_, 2)			||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TIP_, 2)				||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_, 2)			||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_CANCEL_, 2)		||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_, 2)		||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_PREAUTH_COMPLETE_CANCEL_, 2)	||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_, 2)			||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_REFUND_, 2)		||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_ADJUST_, 2)		||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_, 2)			||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_REFUND_, 2)		||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_ADJUST_, 2)		||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_TOP_UP_, 2)			||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_VOID_TOP_UP_, 2)		||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_SALE_, 2)		||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_INSTALLMENT_, 2)	||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REDEMPTION_, 2)	||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2)	||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_CERTAIN_, 2)		||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_FULL_REDEEMPTION_, 2)		||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REDEEM_REFUND_, 2)		||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REFUND_, 2)		||
                    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_EW_INQUIRY_TRANSACTION_, 2))
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "%06ld", pobTran->srBRec.lnBatchNum);
                        memcpy(&szDataBuffer[inPacketSizes], szTemplate, 6);
                }

                inPacketSizes += 6;
        }/* 和信託做區別 */

	/* (需求單-111155)-電子錢包業者收付訊息整合平台需求 by Russell 2023/7/24 下午 6:17 */
	if (pobTran->srEWRec.uszEWTransBit == VS_TRUE)
	{
		/* 特店訂單編號 */
		memcpy(&szDataBuffer[inPacketSizes], pobTran->srEWRec.szEW_OrderId, 30);
		inPacketSizes += 30;
		
		/* 保留 */
		inPacketSizes += 6;
		
		/* 電子發票載具號碼 */
		if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2)			||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_EW_INQUIRY_TRANSACTION_, 2))
		{
			if (strlen(pobTran->srEWRec.szEW_EinvoiceHASH) > 0)
			{
				memcpy(&szDataBuffer[inPacketSizes], pobTran->srEWRec.szEW_EinvoiceHASH, 50);
			}
		}
		inPacketSizes += 50;
		
		/* 退貨訂單編號*/
		if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REFUND_, 2))
		{
			memcpy(&szDataBuffer[inPacketSizes], pobTran->srEWRec.szEW_RefundTradeNo, 30);
		}
		inPacketSizes += 30;
		
		/* 電子錢包業者交易序號 */
		memcpy(&szDataBuffer[inPacketSizes], pobTran->srEWRec.szEW_TransactionId, 30);
		inPacketSizes += 64;
	}
	else
	{
		/* Start Trans Type (2 Byte) */
		if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_START_CARD_NO_INQUIRY_, 2))
			memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szStartTransType, 2);

		inPacketSizes += 2;

		/* MP Flag(是否小額交易，MPAS小額交易用) (1 Byte) */
		/* 合併MPAS ECR到標準400 */
		memset(szFESMode, 0x00, sizeof(szFESMode));
		inGetNCCCFESMode(szFESMode);
		if (memcmp(szFESMode, _NCCC_04_MPAS_MODE_, strlen(_NCCC_04_MPAS_MODE_)) == 0)
		{
			if (pobTran->srBRec.uszMPASTransBit == VS_TRUE)
				memcpy(&szDataBuffer[inPacketSizes], "M", 1);
			else
				memcpy(&szDataBuffer[inPacketSizes], " ", 1);
		}
		inPacketSizes += 1;

		if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_END_CARD_NO_INQUIRY_, 2)	||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SETTLEMENT_, 2)		||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REPRINT_RECEIPT_, 2)	||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REVIEW_DETAIL_, 2)	||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REVIEW_TOTAL_, 2)	||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REPORT_DETAIL_, 2)	||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_MENU_REPORT_TOTAL_, 2)	||
		    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_EDC_REBOOT_, 2))
		{
			/* 共99Bytes */
			inPacketSizes += 8;
			inPacketSizes += 8;
			inPacketSizes += 12;
			inPacketSizes += 5;
			inPacketSizes += 50;
			inPacketSizes += 6;
			inPacketSizes += 1;
			inPacketSizes += 1;
			inPacketSizes += 3;
			inPacketSizes += 5;
		}
		else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
		{
			inPacketSizes += 8;

			/* ESVC Origin Date */
			/* 電票退貨要帶回原交易日期 */
			if (memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REFUND_, 2) == 0)
			{
				memset(szTemp, 0x00, sizeof(szTemp));
				memset(szTemp2, 0x00, sizeof(szTemp2));
				/* [20251219_BUG_MDF][ECC] 修改電票退貨的判斷日期,由 2位MM改為 4位 MMDD */
				memcpy(szTemp2, pobTran->srTRec.szTicketRefundDate, 4);

				inNCCC_TICKET_Decide_Year(szTemp, szTemp2);
				memcpy(&szDataBuffer[inPacketSizes], szTemp, 4); 
				inPacketSizes += 4;

				memcpy(&szDataBuffer[inPacketSizes], &pobTran->srTRec.szTicketRefundDate[0], 4);
				inPacketSizes += 4;
			}
			else
			{
				inPacketSizes += 8;
			}

			/* 兩段式，第一段不用帶RF Number */
			if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
			{
				inPacketSizes += 12;
			}
			else
			{
				/* 餘額查詢不用帶RF序號 */
				if (memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_BALANCE_INQUIRY_, 2) == 0)
				{
					inPacketSizes += 12;
				}
				else
				{
					/* RF序號 */
					memcpy(&szDataBuffer[inPacketSizes], &pobTran->srTRec.szTicketRefundCode[0], 6);
					inPacketSizes += 12;
				}
			}

			/* Pay item */
			inPacketSizes += 5;

			if (!memcmp(&srECROb->srTransData.szECRIndicator[0], "E", 1))
			{
				/* Card No. Hash Value */
				/* 因為卡號查詢會轉交易別，所以用Flag判斷 */
				if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
				{
					memset(szTemplate, 0x00, sizeof(szTemplate));
				}
				else
				{
					if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_TOP_UP_, 2) ||
					    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_VOID_TOP_UP_, 2) ||
					    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_BALANCE_INQUIRY_, 2))
					{
						memset(szTemplate, 0x00, sizeof(szTemplate));
					}
					else
					{
						if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
						{
							memset(szTemplate, 0x00, sizeof(szTemplate));
							strcpy(szTemplate, pobTran->srTRec.szUID);
						}
						else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
						{
							/* 【需求單-108240】EDC修改電票需求 悠遊卡指定要T0200 by Russell 2020/6/4 下午 2:02 */
							memset(szTemplate, 0x00, sizeof(szTemplate));
							strcpy(szTemplate, pobTran->srTRec.srECCRec.szCardID);
						}
						else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
						{
							memset(szTemplate, 0x00, sizeof(szTemplate));
							strcpy(szTemplate, pobTran->srTRec.szUID);	
						}
					}
				}
			}
			else
			{
				/* Card No. Hash Value */
				if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_TOP_UP_, 2) ||
				    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_VOID_TOP_UP_, 2) ||
				    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_ESVC_BALANCE_INQUIRY_, 2))
				{
					memset(szTemplate, 0x00, sizeof(szTemplate));
				}
				else
				{
					if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
					{
						memset(szTemplate, 0x00, sizeof(szTemplate));
						strcpy(szTemplate, pobTran->srTRec.szUID);
					}
					else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
					{
						/* 【需求單-108240】EDC修改電票需求 悠遊卡指定要T0200 by Russell 2020/6/4 下午 2:02 */
						memset(szTemplate, 0x00, sizeof(szTemplate));
						strcpy(szTemplate, pobTran->srTRec.srECCRec.szCardID);
					}
					else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
					{
						memset(szTemplate, 0x00, sizeof(szTemplate));
						strcpy(szTemplate, pobTran->srTRec.szUID);	
					}
				}
			}
			memcpy(&szDataBuffer[inPacketSizes], szTemplate, strlen(szTemplate));
			inPacketSizes += 50;

			/* MP Response Code */
			inPacketSizes += 6;

			/* ASM award flag */
			/* 【需求單-108215】電票交易支援優惠平台需求 by Russell 2020/5/11 下午 3:53 */
			if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2))
			{
				if ((pobTran->srTRec.uszRewardL1Bit == VS_TRUE	||
				     pobTran->srTRec.uszRewardL2Bit == VS_TRUE	||
				     pobTran->srTRec.uszRewardL5Bit == VS_TRUE)	&&
				     pobTran->srTRec.uszRewardSuspendBit != VS_TRUE)
				{
					memcpy(&szDataBuffer[inPacketSizes], "A", strlen("A"));
				}
				else
				{
					memcpy(&szDataBuffer[inPacketSizes], " ", strlen(" "));
				}
			}
			else
			{
				memcpy(&szDataBuffer[inPacketSizes], " ", strlen(" "));
			}
			inPacketSizes += 1;

			/* MCP Indicator & etc.*/
			inPacketSizes += 9;
		}
		else
		{
			/* SmartPay要回傳這三項資訊 (99 Bytes) */
			if ((!memcmp(&srECROb->srTransData.szField_05[0], "S", 1) || pobTran->srBRec.uszFiscTransBit == VS_TRUE) &&
			     pobTran->uszCardInquiryFirstBit != VS_TRUE)
			{
				/* SP ISSUER ID (8 Byte) */
				memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szFiscIssuerID[0], 8);
				inPacketSizes += 8;
				/* SP Origin Date (8 Byte) */
				if (strlen(pobTran->srBRec.szFiscRefundDate) > 0)
				{
					memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szFiscRefundDate[0], 8);
				}  
				else
				{
					memset(szTemplate, 0x00, sizeof(szTemplate));
					strcat(szTemplate, pobTran->srBRec.szDate);
					memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 8);
				}
				inPacketSizes += 8;
				/* SP RRN (12 Byte) */
				memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szFiscRRN[0], 12);
				inPacketSizes += 12;
			}
			else
			{
				inPacketSizes += 8;
				inPacketSizes += 8;
				inPacketSizes += 12;
			}

			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetPayItemEnable(szTemplate);
			if (memcmp(szTemplate, "Y", strlen("Y")) == 0	&&
			   (strlen(pobTran->srBRec.szPayItemCode) > 0))
			{
				/* Pay Item (5 Bytes) */
				/* ECR一段式收銀機連線 送空白payitem 不必回傳 */
				if (srECROb->srTransData.uszECRResponsePayitem == VS_FALSE)
				{
					inPacketSizes += 5;
				}
				else
				{
					memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szPayItemCode[0], 5);
					inPacketSizes += 5;
				}
			}
			else
			{
				inPacketSizes += 5;
			}
			/* 【需求單 - 105039】信用卡為電子發票載具，端末機將卡號加密後回傳收銀機 add by LingHsiung 2016-04-20 上午 09:56:24 */
			/* 【需求單 - 108046】電子發票BIN大於6碼需求 by Russell 2019/7/8 上午 11:44 */
			/* 收銀機欄位ECR Indicator = “E” : 
			 * (1)銀行代碼為 ”B00999” : Card No. Hash Value = 空白50碼
			 * (2)其他 :Card No. Hash Value = 銀行代碼6碼 + Hash Value 44碼。
			*/
			/* Card No. Hash Value (50 Bytes) */
			/* Smartpay 和 DCC 因為不回傳"NI"，所以不回傳hash value */
			/* 只要沒回傳Table "NI" 一律回50個空白 */
			/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/3 下午 6:18 */
			/* Üny 交易：
			   電子發票加密卡號 (B00xxx ’’+44 碼 HASH 值 xxx為金融機構代碼) */
			/* 【需求單 - 112071】客製化122 京站時尚百貨，調整收銀機規格
			 * 符合下列全部條件時，ECR加回欄位37 Card No. Hash Value，內容為 : “B00000” + 44碼卡號Hash值。
			 * ECR Indicator=‘E’、 有卡號、 電文無NI Table或NI Table為”B00999”(非未加入電子發票載具之銀行或國外卡)。
			 * 2023/6/12 下午 4:03 by Russell */
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_122_QSQUARE_, _CUSTOMER_INDICATOR_SIZE_) &&
			    !memcmp(&srECROb->srTransData.szECRIndicator[0], "E", 1) &&
			     strlen(pobTran->srBRec.szPAN) > 0 &&
			    (strlen(pobTran->srBRec.szEIVI_BANKID) == 0 || memcmp(pobTran->srBRec.szEIVI_BANKID, "B00999", 6) == 0))
			{
				memcpy(&szDataBuffer[inPacketSizes], "B00000", 6);
				inPacketSizes += 6;

				memset(szTemplate, 0x00, sizeof(szTemplate));
				if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_AWARD_REDEEM_, 2))
				{
					
				}
				else
				{
					memcpy(&szTemplate[0], &pobTran->srBRec.szPAN[0], strlen(pobTran->srBRec.szPAN));
				}
				
				if (strlen(szTemplate) > 0)
				{
					memset(szHash, 0x00, sizeof(szHash));
					inNCCC_Func_CardNumber_Hash(szTemplate, szHash);
					memcpy(&szDataBuffer[inPacketSizes], &szHash[0], 44);
				}
				inPacketSizes += 44;
			}
			else
			{
				if (!memcmp(&srECROb->srTransData.szECRIndicator[0], "Q", 1))
				{
					if (pobTran->uszCardInquiryFirstBit == VS_TRUE	||
					    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_END_CARD_NO_INQUIRY_, 2)	||
					    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_AWARD_REDEEM_, 2) ||
                                            pobTran->srBRec.uszUPlanECRBit == VS_TRUE)
					{
						inPacketSizes += 6;
						inPacketSizes += 44;
					}
					else
					{
						memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szEIVI_BANKID[0], 6);
						inPacketSizes += 6;
						memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szEInvoiceHASH[0], 44);
						inPacketSizes += 44;
					}
				}
				else if (!memcmp(&srECROb->srTransData.szECRIndicator[0], "E", 1))
				{
					/* 啟動卡號查詢只有在Indicator = "I" 才送Hash Value("E"一律送50個空白) */
					/* 因為卡號查詢會轉交易別，所以用Flag判斷 */
					if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
					{
						inPacketSizes += 50;
					}
					else
					{
						if (strlen(pobTran->srBRec.szEIVI_BANKID) > 0)
						{
							if (!memcmp(&pobTran->srBRec.szEIVI_BANKID[0], "B00999", 6))
							{
								inPacketSizes += 50;
							}
							else
							{
								memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szEIVI_BANKID[0], 6);
								inPacketSizes += 6;

								memset(szTemplate, 0x00, sizeof(szTemplate));
								if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_AWARD_REDEEM_, 2))
								{
									
								}
								else
								{
									memcpy(&szTemplate[0], &pobTran->srBRec.szPAN[0], strlen(pobTran->srBRec.szPAN));
								}
								
								if (strlen(szTemplate) > 0)
								{
									memset(szHash, 0x00, sizeof(szHash));
									inNCCC_Func_CardNumber_Hash(szTemplate, szHash);
									memcpy(&szDataBuffer[inPacketSizes], &szHash[0], 44);
								}
								inPacketSizes += 44;
							}
						}
						else
						{
							inPacketSizes += 50;
						}
					}
				}
				/* 收銀機欄位ECR Indicator = “I” : Card No. Hash Value = Card number前6碼 + Hash Value 44碼(原規格) */
				else
				{
					memset(szTemplate, 0x00, sizeof(szTemplate));
					if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_AWARD_REDEEM_, 2))
					{
						
					}
					else
					{
						/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
						memcpy(&szTemplate[0], &pobTran->srBRec.szPAN[0], strlen(pobTran->srBRec.szPAN));
					}

					if (strlen(szTemplate) > 0)
					{
						memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 6);
						inPacketSizes += 6;

						memset(szHash, 0x00, sizeof(szHash));
						inNCCC_Func_CardNumber_Hash(szTemplate, szHash);
						memcpy(&szDataBuffer[inPacketSizes], &szHash[0], 44);
						inPacketSizes += 44;
					}
					else
					{
						inPacketSizes += 50;
					}
				}
			}

			/* MP Response Code (6 Bytes) */
			memset(szFESMode, 0x00, sizeof(szFESMode));
			inGetNCCCFESMode(szFESMode);
			if (memcmp(szFESMode, _NCCC_04_MPAS_MODE_, strlen(_NCCC_04_MPAS_MODE_)) == 0)
			{
				if (memcmp(pobTran->srBRec.szRespCode, "00", 2) != 0		&& 
				    memcmp(pobTran->srBRec.szRespCode, "11", 2) != 0		&&
				    pobTran->srBRec.uszMPASTransBit == VS_TRUE			&&
				    (memcmp(&pobTran->srBRec.szMPASAuthCode[0], "CAF", 3) == 0	||
				     memcmp(&pobTran->srBRec.szMPASAuthCode[0], "NEG", 3) == 0	||
				     memcmp(&pobTran->srBRec.szMPASAuthCode[0], "WEB", 3) == 0	|| 
				     memcmp(&pobTran->srBRec.szMPASAuthCode[0], "REJ", 3) == 0))
				{
					memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szMPASAuthCode[0], 6);
				}
			}
			inPacketSizes += 6;

			/* ASM award flag */
			if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2)	||
			    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_OFFLINE_, 2)	||
			    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INSTALLMENT_, 2)	||
			    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REDEEM_, 2))
			{	
				if ((pobTran->srBRec.uszRewardL1Bit == VS_TRUE	||
				     pobTran->srBRec.uszRewardL2Bit == VS_TRUE	||
				     pobTran->srBRec.uszRewardL5Bit == VS_TRUE)	&&
				     pobTran->srBRec.uszRewardSuspendBit != VS_TRUE)
				{
					memcpy(&szDataBuffer[inPacketSizes], "A", strlen("A"));
				}
				else
				{
					memcpy(&szDataBuffer[inPacketSizes], " ", strlen(" "));
				}
			}
			else
			{
				memcpy(&szDataBuffer[inPacketSizes], " ", strlen(" "));
			}
			inPacketSizes += 1;

			/* 【需求單 - 106128】行動支付標記及金融機構代碼 add by LingHsiung 2017-08-23 上午 11:48:57 */
			if (strlen(pobTran->srBRec.szMCP_BANKID) > 0)
			{
				/* MCP Indicator (1 Bytes) */
				memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szMCP_BANKID[0], 1);
				inPacketSizes += 1;

				/* 金融機構代碼 (3 Bytes) */
				memcpy(&szDataBuffer[inPacketSizes], &pobTran->srBRec.szMCP_BANKID[1], 3);
				inPacketSizes += 3;
			}
			else
			{
				inPacketSizes += 4;
			}

			/* Reserve (5 Bytes) */
			inPacketSizes += 5;
		}

		/* HG Data (78 Byte) */
		if (pobTran->srBRec.lnHGTransactionType != 0	&&
		    (pobTran->srBRec.uszHappyGoMulti == VS_TRUE || pobTran->srBRec.uszHappyGoSingle == VS_TRUE))
		{
			/* Payment Tools (2 Byte) */
			if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2) ||
			    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_CERTAIN_, 2))
			{
				if (pobTran->srBRec.lnHGPaymentType == _HG_PAY_CREDIT_)
					memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_HG_PAYMENT_TOOL_TYPE_CREDIT_CARD_, 2);
				else if (pobTran->srBRec.lnHGPaymentType == _HG_PAY_CASH_)
					memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_HG_PAYMENT_TOOL_TYPE_CASH_, 2);
				else if (pobTran->srBRec.lnHGPaymentType == _HG_PAY_GIFT_PAPER_)
					memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_HG_PAYMENT_TOOL_TYPE_GIFT_PAPER_, 2);
				else if (pobTran->srBRec.lnHGPaymentType == _HG_PAY_CREDIT_INSIDE_)
					memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_HG_PAYMENT_TOOL_TYPE_HGI_, 2);
				else if (pobTran->srBRec.lnHGPaymentType == _HG_PAY_CUP_)
					memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_HG_PAYMENT_TOOL_TYPE_CUP_CARD_, 2);
			}

			inPacketSizes += 2;

			/* HG Card Number (18 Byte) */
			/* 重印HG混信用卡的交易簽單不回傳HG卡號 */
			if (memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REPRINT_RECEIPT_, 2) != 0)
			{
				inRetVal = inECR_CardNoTruncateDecision_HG(pobTran);
				/* 要遮卡號 */
				if (inRetVal == VS_SUCCESS)
				{
					memset(szTemplate, 0x00, sizeof(szTemplate));
					strcpy(szTemplate, pobTran->srBRec.szHGPAN);

					/* HAPPG_GO 卡不掩飾 */
					if (!memcmp(&pobTran->srBRec.szHGPAN[0], "9552", 4))
					{

					}
					else
					{
						/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
						/* 卡號長度 */
						inCardLen = strlen(szTemplate);

						if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_088_TINTIN_, _CUSTOMER_INDICATOR_SIZE_))
						{
							/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
							for (i = 8; i < (inCardLen - 4); i ++)
								szTemplate[i] = '*';
						}
						else
						{    
							/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
							for (i = 6; i < (inCardLen - 4); i ++)
								szTemplate[i] = '*';
						}
					}

					memcpy(&szDataBuffer[inPacketSizes], szTemplate, strlen(szTemplate));
				}
				else
				{
					memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szHGPAN, strlen(pobTran->srBRec.szHGPAN));
				}

			}

			inPacketSizes += 18;

			/* HG Pay Amount (12 Byte) 實際支付金額 */
			if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2))
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%010lu00", pobTran->srBRec.lnHGAmount);
				memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 12);
			}

			inPacketSizes += 12;
			/* HG Redeem Amount (12 Byte) 扣抵金額 */
			if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2))
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%010lu00", pobTran->srBRec.lnHGRedeemAmount);
				memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 12);
			}

			inPacketSizes += 12;
			/* HG Redeem Point (8 Byte) 扣抵點數 */
			if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2)	||
			    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_CERTAIN_, 2)		||
			    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_FULL_REDEEMPTION_, 2)		||
			    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REDEEM_REFUND_, 2)		||
			    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REFUND_, 2))
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%08lu", pobTran->srBRec.lnHGTransactionPoint);
				memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 8);
			}

			inPacketSizes += 8;
			/* HG Lack Point (8 Byte) 不足點數*/
			if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REFUND_, 2))
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%08lu", pobTran->srBRec.lnHGRefundLackPoint);
				memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 8);
			}

			inPacketSizes += 8;
			/* HG Balance Point (8 Byte) 剩餘點數 */
			if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_ONLINE_REDEEMPTION_, 2) ||
			    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_CERTAIN_, 2) ||
			    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_FULL_REDEEMPTION_, 2) ||
			    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REDEEM_REFUND_, 2) ||
			    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_REWARD_REFUND_, 2) ||
			    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_HG_POINT_INQUIRY_, 2) ||
			    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_VOID_, 2))
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%08lu", pobTran->srBRec.lnHGBalancePoint);
				memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 8);
			}

			inPacketSizes += 8;
			/* HG Reserve (10 Byte) */
			inPacketSizes += 10;
		}
		else
		{
			/* [114197] 中心收單作業ESG永續發展優化
			 * [20260515] 新增此區塊判斷條件 szAgreeEsgBill 進行判斷是否要回傳ESG資料
			 * [20260615] 新增抓取ESC判斷,不需要判斷回傳值,修改ESB包資料的寫法,改由函式方法包裝 */
			if (inGetCHESG_FLAG(szEnableESG) != VS_SUCCESS)
			{
				if (ginDebug == VS_TRUE)
					inLogPrintf(AT, "Log:ESG Get szEnableESG Error");
			}

			if (szEnableESG[0] == 'Y')
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "inECR_8N1_Standard_Pack() HappyGo 未開啟");
					inLogPrintf(AT, "szAgreeEsgBill is %s", pobTran->szAgreeEsgBill);
				}

				inECR_PackEsgData(pobTran, &inPacketSizes, szDataBuffer);
			} else
			{
				inPacketSizes += 78;
			}
		}
	}
	
	if (gbBarCodeECRBit == VS_TRUE)
	{
		/* Üny 交易碼 */
		if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
		{
			/* 一維或二維條碼資料長度 */
			inBarCodeLen = strlen(pobTran->srBRec.szBarCodeData);
			inPacketSizes += 3;
			/* 一維或二維條碼資料內容 */
			inPacketSizes += inBarCodeLen;

			/* 補滿至980 */
			inPacketSizes = 980;

			/* Üny 交易碼 */
			/* 除結束卡號查詢不用，其他Uny交易都要 */
			if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_END_CARD_NO_INQUIRY_, 2)	||
			    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REPRINT_RECEIPT_, 2))
			{

			}
			else
			{
				memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szUnyTransCode, 20);
			}
			inPacketSizes += 20;
		}
		else if (pobTran->srEWRec.uszEWTransBit == VS_TRUE)
		{
			/* 電子錢包資料長度 */
			inBarCodeLen = strlen(pobTran->srBRec.szBarCodeData);
			sprintf(&szDataBuffer[inPacketSizes], "%03d", 184);
			inPacketSizes += 3;
			/* 電子錢包交易條碼 */
			if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2))
			{
				memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szBarCodeData, inBarCodeLen);
			}
			inPacketSizes += 25;
			
			/* 電子錢包業者特店代號 */
			memcpy(&szDataBuffer[inPacketSizes], pobTran->srEWRec.szEW_ChannelId, 50);
			inPacketSizes += 50;
			/* 電子錢包交易日期時間 */
			memcpy(&szDataBuffer[inPacketSizes], pobTran->srEWRec.szEW_PaymentDate, 24);
			inPacketSizes += 24;
			/* 消費者支付工具 */
			if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2)	||
			    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_EW_INQUIRY_TRANSACTION_, 2))
			{
				memcpy(&szDataBuffer[inPacketSizes], pobTran->srEWRec.szEW_PayMethod, 1);
			}
			inPacketSizes += 1;
			/* 訂單狀態 */
			if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_EW_INQUIRY_TRANSACTION_, 2))
			{
				memcpy(&szDataBuffer[inPacketSizes], pobTran->srEWRec.szEW_TransactionType, 2);
			}
			inPacketSizes += 2;
			/* 付款交易編號 */
			if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2))
			{
				memcpy(&szDataBuffer[inPacketSizes], pobTran->srEWRec.szEW_PaymentNo, 30);
			}
			inPacketSizes += 30;
			/* 是否為重複付款請求 */
			if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2))
			{
				memcpy(&szDataBuffer[inPacketSizes], pobTran->srEWRec.szEW_IsDuplicatedPaymentRequest, 1);
			}
			inPacketSizes += 1;
			/* 聯名卡代碼 */
			if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2) ||
			    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_EW_INQUIRY_TRANSACTION_, 2))
			{
				memcpy(&szDataBuffer[inPacketSizes], pobTran->srEWRec.szEW_CobrandedCode, 11);
			}
			inPacketSizes += 11;
			/* 載具類型 */
			if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_EW_INQUIRY_TRANSACTION_, 2))
			{
				memcpy(&szDataBuffer[inPacketSizes], pobTran->srEWRec.szEW_EinvoiceCarrierType, 6);
			}
			inPacketSizes += 6;
			/* 折抵金額/退還折抵金額 */
			if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SALE_, 2) ||
			    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_EW_INQUIRY_TRANSACTION_, 2))
			{
				memcpy(&szDataBuffer[inPacketSizes], pobTran->srEWRec.szEW_DiscountAmt, 10);
			}
			else if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REFUND_, 2))
			{
				memcpy(&szDataBuffer[inPacketSizes], pobTran->srEWRec.szEW_DiscountAmt, 10);
			}
			inPacketSizes += 10;
			/* 門市代碼 */
			memcpy(&szDataBuffer[inPacketSizes], pobTran->srEWRec.szEW_Store_no, 24);
			inPacketSizes += 24;
			/* 保留 */
			inPacketSizes += 413;
		}
		else if (pobTran->srBRec.uszUPlanECRBit == VS_TRUE)
		{
			/* 一維或二維條碼資料長度 */
			inBarCodeLen = strlen(pobTran->srBRec.szBarCodeData);
			inPacketSizes += 3;
			/* 一維或二維條碼資料內容 */
			inPacketSizes += inBarCodeLen;
			
			/* 補滿至980 */
			inPacketSizes = 980;
			
			inPacketSizes += 20;
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inECR_8N1_Standard_Pack_ResponseCode
Date&Time       :2016/7/6 下午 2:40
Describe        :先把要送的資料組好
*/
int inECR_8N1_Standard_Pack_ResponseCode(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuf)
{
	char	szTRTFileName[12 + 1];
	
	/* 非參加機構卡片判斷 */
	if (!memcmp(pobTran->srBRec.szRespCode, "05", 2) && (!memcmp(&pobTran->srBRec.szMPASAuthCode[0], "REJB01", 6) ||
							     !memcmp(&pobTran->srBRec.szMPASAuthCode[0], "REJB02", 6) ||
							     !memcmp(&pobTran->srBRec.szMPASAuthCode[0], "REJB03", 6)))
	{
		srECROb->srTransData.inErrorType = _ECR_RESPONSE_CODE_NOT_MEMBER_CARD_;
	}
	
	if (srECROb->srTransData.inErrorType == VS_CALLBANK	||
	    srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_CALLBANK_)
	{
		memcpy(&szDataBuf[76], "0002", 4);
	}
	else if (srECROb->srTransData.inErrorType == VS_TIMEOUT		||
		 srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_TIMEOUT_	||
		 srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_EW_HOST_TIMEOUT_)
	{
		memcpy(&szDataBuf[76], "0003", 4);
	}
	else if (srECROb->srTransData.inErrorType == VS_USER_OPER_ERR	||	/* 操作錯誤 & 交易流程有誤 */
		 srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_) 
	{
		memcpy(&szDataBuf[76], "0004", 4);
	}
	else if (srECROb->srTransData.inErrorType == VS_COMM_ERROR		||
		 srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_COMM_ERROR_)
	{
		memcpy(&szDataBuf[76], "0005", 4);
	}
	else if (srECROb->srTransData.inErrorType == VS_USER_CANCEL || 
		 srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_USER_TERMINATE_ERROR_)
	{
		memcpy(&szDataBuf[76], "0006", 4);
	}
	else if (srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_NOT_MEMBER_CARD_)
	{
		memcpy(&szDataBuf[76], "0009", 4); /* 非參加機構卡片 */
	}
	else if (srECROb->srTransData.inErrorType == VS_ISO_PACK_ERR		|| 
		 srECROb->srTransData.inErrorType == VS_ISO_UNPACK_ERROR	||
		 srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_FORMAT_CHECK_ERROR_)
	{
		memcpy(&szDataBuf[76], "0010", 4); /* 電文錯誤 */
	}
	else if (srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_NOT_EVERRICH_CO_BRAND_CARD_)
	{
		memcpy(&szDataBuf[76], "0011", 4); /* 非昇恆昌聯名卡錯誤 */
	}
	else if (srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_END_INQUIRY_SUCCESS_)
	{
		memcpy(&szDataBuf[76], "0000", 4);
	}
	else if (srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_ESVC_INSUFFICIENT_BALANCE_	||
		 srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_EW_INSUFFICIENT_BALANCE_)
	{
		memcpy(&szDataBuf[76], "0016", 4); /* 電票/電子錢包餘額不足 */
	}
	else if (srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_MULTI_CARD_)
	{
		memcpy(&szDataBuf[76], "0017", 4);
	}
	else if (srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_UNY_BARCODE_DATA_ERROR_)
	{
		memcpy(&szDataBuf[76], "2007", 4);
	}
	/* (【需求單-113200】CASV3標準版支援銀聯優計劃專案 by Russell 2024/8/27 上午 11:03 */
	else if (srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_COUPON_USED_)
	{
		memcpy(&szDataBuf[76], "2001", 4);
	}
	else if (srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_COUPON_USED_)
	{
		memcpy(&szDataBuf[76], "2002", 4);
	}
	else if (srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_COUPON_USED_)
	{
		memcpy(&szDataBuf[76], "2003", 4);
	}
	else if (srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_COUPON_USED_)
	{
		memcpy(&szDataBuf[76], "2004", 4);
	}
	else if (srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_COUPON_USED_)
	{
		memcpy(&szDataBuf[76], "2005", 4);
	}
        else if (srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_NOT_ENTER_PATIENTID_)
	{
		memcpy(&szDataBuf[76], "2009", 4);
	}
	else
	{
		memset(szTRTFileName, 0x00, sizeof(szTRTFileName));
		inGetTRTFileName(szTRTFileName);
		/* 2017/9/6 上午 11:21 看verifone code新增1301 */
		if (memcmp(&pobTran->srBRec.szRespCode[0], "00", 2) &&
	            memcmp(&pobTran->srBRec.szRespCode[0], "11", 2) &&
                    memcmp(szTRTFileName, _TRT_FILE_NAME_HG_, strlen(_TRT_FILE_NAME_HG_)) == 0)
		{
                        memcpy(&szDataBuf[76], "1301", 4);
		}
                else
		{
			memcpy(&szDataBuf[76], "0001", 4);
		}
	}
	
	/* 電票的ResponseCode */
	if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
	{
		/* 因為失敗跳出的地方太多，所以加在這 */
		if (pobTran->uszAutoTopUpSuccessBit == VS_TRUE &&
		    pobTran->uszDeductSuccessBit != VS_TRUE)
		{
			sprintf(pobTran->szTicket_ErrorCode, "%s", "E0019");
		}
		
		/* 有值才送 */
		if (pobTran->szTicket_ErrorCode[0] == 'E')
		{
			memcpy(&szDataBuf[76], &pobTran->szTicket_ErrorCode[1], 4);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inECR_7E1_Standard_Unpack
Date&Time       :2017/11/16 上午 10:45
Describe        :分析收銀機傳來的資料
*/
int inECR_7E1_Standard_Unpack(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, ECR_TABLE * srECROb)
{
	int	i = 0;
	int	inRetVal = VS_SUCCESS;
	int	inTransType = 0;		/* 電文中的交易別字串轉為數字儲存 */
	int	inHGPaymetType = 0;
	int	inCardInquiryTRTType = _CARD_INQUIRY_TYPE_NONE_;
	char	szTemplate[100 + 1] = {0};
	char	szDebugMsg[100 + 1] = {0};	
	char	szHostLabel[8 + 1] = {0};
	char	szHostEnable[2 + 1] = {0};
	char	szBatchNum[6 + 1] = {0};
	char	szCTLSEnable[2 + 1] = {0};
	char	szFiscContactlessEnable[1 + 1] = {0};
	char	szCUPContactlessEnable[1 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
	
	/* (需求單 - 107276)自助交易標準做法 EDC於發動交易及完成時，需發出提示音 by Russell 2019/3/8 上午 10:46 */
	inDISP_BEEP(1, 0);
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	/* 存ECR原始資料 */
	memcpy(srECROb->srTransData.szOrgData, szDataBuffer, _ECR_7E1_Standard_Data_Size_);
	
	inECR_7E1_Standard_Parse_Data(srECROb, szDataBuffer);
	
	/* Trans Type (交易別) */
	switch (inTransType)
	{
		default:
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szTransType[0], 2);
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s: %s  ", "Trans Type", szTemplate);
				inLogPrintf(AT, szDebugMsg);
			}
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Type", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				sprintf(pobTran->szErrorMsgBuff1, "%02d", inTransType);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "Trans Type Not Exist Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
				return (VS_ERROR);
			}
			
			/* 交易類別轉成數字 */
			inTransType = atoi(srECROb->srTransData.szTransType);
			
			/* 要連動結帳 */
			if (inTransType == _ECR_7E1_SETTLEMENT_NO_)
			{
				pobTran->uszAutoSettleBit = VS_TRUE;
			}
			
			break;
	}
	
	/* 銀行別/付款別 HOST ID*/
	switch (inTransType)
	{
		case _ECR_7E1_VOID_NO_:				/* 取消交易 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szHostID[0], 2);
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "HOST ID", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "%s: %s  ", "HOST ID", "Not Exist Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
					
				return (VS_ERROR);
			}
			
			memset(szHostLabel, 0x00, sizeof(szHostLabel));

			if (memcmp(szTemplate, _ECR_7E1_NCCC_HOSTID_NCCC_, 2) == 0)
			{
				memcpy(szHostLabel, _HOST_NAME_CREDIT_NCCC_, strlen(_HOST_NAME_CREDIT_NCCC_));
			}
			else if (memcmp(szTemplate, _ECR_7E1_NCCC_HOSTID_DCC_, 2) == 0)
			{
				memcpy(szHostLabel, _HOST_NAME_DCC_, strlen(_HOST_NAME_DCC_));
			}
			/* 2018/5/8 下午 2:10 DFS需求不再使用大來主機 */
			else
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "無對應Host ID");
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				sprintf(pobTran->szErrorMsgBuff1, "%02d", inTransType);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "%s : HOST ID Not Pair", szTemplate);
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
					
				return (VS_ERROR);
			}
			
			/* 初始化 */
			i = 0;
			do
			{
				/* 按順序load每一個HOST */
				if (inLoadHDTRec(i) < 0)
				{
					memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
					sprintf(pobTran->szErrorMsgBuff1, "%02d", inTransType);
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "%s : HOST ID Not Found", szHostLabel);
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
					/* 如果return VS_ERROR 代表table沒有該HOST */
					return (VS_ERROR);
				}

				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetHostLabel(szTemplate);
				if (!memcmp(szTemplate, szHostLabel, strlen(szHostLabel)))
				{
					/* 比對成功後，判斷HOST是否開啟  */
					memset(szHostEnable, 0x00, sizeof(szHostEnable));
					inGetHostEnable(szHostEnable);
					if (szHostEnable[0] != 'Y')
					{
						/* Host沒開 */
						if (ginDebug == VS_TRUE) 
						{
							memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
							sprintf(szDebugMsg, "%s Not Open Error", szHostLabel);
							inLogPrintf(AT, szDebugMsg);
						}
						memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
						sprintf(pobTran->szErrorMsgBuff1, "%02d", inTransType);
						memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
						sprintf(pobTran->szErrorMsgBuff2, "%s : HOST ID Not Open", szHostLabel);
						pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

						/* 如果return VS_ERROR 代表Host沒開 */
						return (VS_ERROR);
					}
					else
					{
						/* Host有開 */
						pobTran->srBRec.inHDTIndex = i;
						
						/* Load HDPT */
						inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
						memset(szBatchNum, 0x00, sizeof(szBatchNum));
						inGetBatchNum(szBatchNum);
						pobTran->srBRec.lnBatchNum = atol(szBatchNum);
						
						if (ginDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
							sprintf(szDebugMsg, "%s Open", szHostLabel);
							inLogPrintf(AT, szDebugMsg);
						}
						
					}/* Host Enable比對End */

					break;
				}/* Host Label比對 End */
				i++;

			} while (1);
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%d", "HDTindex", pobTran->srBRec.inHDTIndex);
				inLogPrintf(AT, szDebugMsg);
			}
			
			break;
			
		default:
			break;
	}
	
	/* EDC簽單序號 Receipt No(InvoiceNumber) */
	switch (inTransType)
	{
		case _ECR_7E1_VOID_NO_:				/* 取消交易 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &srECROb->srTransData.szReceiptNo[0], 6);
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Receipt No", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				sprintf(pobTran->szErrorMsgBuff1, "%02d", inTransType);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "Receipt No Not Exist Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
				return (VS_ERROR);
			}
			else
			{
				pobTran->srBRec.lnOrgInvNum = atol(szTemplate);
			}
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%ld", "lnOrgInvNum", pobTran->srBRec.lnOrgInvNum);
				inLogPrintf(AT, szDebugMsg);
			}
			
			break;
		default:
			break;
	}
		
	/* 處理交易金額 (Trans Amount) */
	switch (inTransType)
	{
		case _ECR_7E1_SALE_NO_:
		case _ECR_7E1_OFFLINE_NO_:
		case _ECR_7E1_REFUND_NO_:
		case _ECR_7E1_INSTALLMENT_NO_:
		case _ECR_7E1_REDEEM_NO_:
		case _ECR_7E1_INSTALLMENT_REFUND_NO_:
		case _ECR_7E1_REDEEM_REFUND_NO_:
		case _ECR_7E1_HG_REWARD_SALE_NO_ :
		case _ECR_7E1_HG_REWARD_INSTALLMENT_NO_ :
		case _ECR_7E1_HG_REWARD_REDEMPTION_NO_ :
		case _ECR_7E1_HG_ONLINE_REDEEMPTION_NO_ :
		case _ECR_7E1_HG_POINT_CERTAIN_NO_ :
		case _ECR_7E1_PREAUTH_NO_:
                case _ECR_7E1_START_CARD_NO_INQUIRY_NO_:
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &srECROb->srTransData.szField_10[0], 10);
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Amount", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "Trans Amount Not Exist Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
				return (VS_ERROR);
			}
			else if ((!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_076_8_DIGITS_, _CUSTOMER_INDICATOR_SIZE_) && atol(szTemplate) > 99999999) || 
				 (memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_076_8_DIGITS_, _CUSTOMER_INDICATOR_SIZE_) && atol(szTemplate) > 9999999))
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Amount", "Out of bound");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_076_8_DIGITS_, _CUSTOMER_INDICATOR_SIZE_))
				{
					sprintf(pobTran->szErrorMsgBuff2, "金額超過8位數");
				}
				else
				{
					sprintf(pobTran->szErrorMsgBuff2, "金額超過7位數");
				}
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
				
				return (VS_ERROR);
			}
			else
			{
				pobTran->srBRec.lnTxnAmount = atol(szTemplate);
				pobTran->srBRec.lnOrgTxnAmount = atol(szTemplate);
				pobTran->srBRec.lnTotalTxnAmount = atol(szTemplate);
			}

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%ld", "交易金額", pobTran->srBRec.lnTxnAmount);
				inLogPrintf(AT, szDebugMsg);
			}
			break;
		default:
			break;
	}
	
	/* 交易時間 Trans Time */
	switch (inTransType)
	{
		default:
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szTransTime[0], 6);
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Time", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
//				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
//				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
//				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
//				sprintf(pobTran->szErrorMsgBuff2, "Trans Time Not Exist Error");
//				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
//				
//				/* 有時候會不送時間 */
//				return (VS_ERROR);
			}
			else
			{
				sprintf(pobTran->srBRec.szTime, "%s", szTemplate);
				sprintf(pobTran->srBRec.szOrgTime, "%s", szTemplate);
			}
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "交易時間", pobTran->srBRec.szTime);
				inLogPrintf(AT, szDebugMsg);
			}
			
			break;
			
		
			break;
	}
	
	/* 授權碼(Auth Code) ECR電文中給9個字元(左靠右補空白) 但ATS電文只需要6個，所以只抓6個(交易補登及所有退貨須送此欄位) */
	switch (inTransType)
	{
		case _ECR_7E1_REFUND_NO_:			/* 退貨 */
		case _ECR_7E1_OFFLINE_NO_:			/* 交易補登 */
		case _ECR_7E1_INSTALLMENT_REFUND_NO_:		/* 分期退貨 */
		case _ECR_7E1_INSTALLMENT_ADJUST_NO_:		/* 分期調帳 */
		case _ECR_7E1_REDEEM_REFUND_NO_:		/* 紅利退貨 */
		case _ECR_7E1_REDEEM_ADJUST_NO_:		/* 紅利調帳 */
			/* 處理授權碼(optional) */
			memset(pobTran->srBRec.szAuthCode, 0x00, sizeof(pobTran->srBRec.szAuthCode));
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szApprovalNo[0], 6);
			memcpy(pobTran->srBRec.szAuthCode, szTemplate, 6);
			
			break;
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Auth Code", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "Auth Code Not Exist Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
				return (VS_ERROR);
			}
			else
			{
				memcpy(pobTran->srBRec.szAuthCode, szTemplate, 6);
			}
			
			break;
			
		default :
			break;
	}
	
	/* szDataBuffer[66] 144規格要看紅利分期來看使用用途  */
	switch (inTransType)
	{
		case _ECR_7E1_REDEEM_REFUND_NO_:		/* 紅利退貨 */
		case _ECR_7E1_REDEEM_ADJUST_NO_:		/* 紅利調帳 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szRDMPaidAmt[0], 10);	/* 12位數，這裡只取10位 */
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "RDM Paid Amt", szTemplate);
				inLogPrintf(AT, szDebugMsg);
			}
			/* 實際支付金額 */
			pobTran->srBRec.lnRedemptionPaidCreditAmount = atol(szTemplate);
			
			break;	
		case _ECR_7E1_INSTALLMENT_REFUND_NO_:		/* 分期退貨 */
		case _ECR_7E1_INSTALLMENT_ADJUST_NO_:		/* 分期調帳 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szField_28[0], 10);	/* 12位數，這裡只取10位 */

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "Formallity Fee", szTemplate);
				inLogPrintf(AT, szDebugMsg);
			}
			pobTran->srBRec.lnInstallmentFormalityFee = atol(szTemplate);

			if (pobTran->srBRec.lnInstallmentFormalityFee != 0)
			{
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_INST_FEE_NOT_0_;
				pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;		/* 操作錯誤 */
				return (VS_ERROR);
			}

			break;
		default :
			break;
	}
	
	/* 分期期數 Installment Period */
	switch (inTransType)
	{
		case _ECR_7E1_INSTALLMENT_NO_:			/* 分期 */
		case _ECR_7E1_INSTALLMENT_REFUND_NO_:		/* 分期退貨 */
		case _ECR_7E1_INSTALLMENT_ADJUST_NO_:		/* 分期調帳 */
		case _ECR_7E1_HG_REWARD_INSTALLMENT_NO_:	/* HG紅利積點(分期)*/
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szInstallmentPeriod[0], 2);
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "Installment Period", szTemplate);
				inLogPrintf(AT, szDebugMsg);
			}
			/* 分期期數(右靠左補0) */
			pobTran->srBRec.lnInstallmentPeriod = atol(szTemplate);
			
			break;
			
		default :
			break;
	}
	
	/* szDataBuffer[102] 144規格要看紅利分期來看使用用途 保留欄位/首期金額／紅利扣抵點數  */
	switch (inTransType)
	{
		case _ECR_7E1_REDEEM_REFUND_NO_:		/* 紅利退貨 */
		case _ECR_7E1_REDEEM_ADJUST_NO_:		/* 紅利調帳 */
			/* 最多八位 所以從106開始 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szRDMPoint[4], 8);
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "RDM Point", szTemplate);
				inLogPrintf(AT, szDebugMsg);
			}
			
			/* 信用卡紅利扣抵點數 RDM Point */
			pobTran->srBRec.lnRedemptionPoints = atol(szTemplate);
			break;
		case _ECR_7E1_INSTALLMENT_REFUND_NO_:		/* 分期退貨 */
		case _ECR_7E1_INSTALLMENT_ADJUST_NO_:		/* 分期調帳 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szField_26[0], 10);	/* 12位數，這裡只取10位 */
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "Down Payment Amount", szTemplate);
				inLogPrintf(AT, szDebugMsg);
			}
			/* 首期金額 Down Payment Amount (含小數 2位) */
			pobTran->srBRec.lnInstallmentDownPayment = atol(szTemplate);
			
			break;
		case _ECR_7E1_HG_POINT_CERTAIN_NO_:		/* 加價購*/
		case _ECR_7E1_HG_FULL_REDEEMPTION_NO_:		/* 點數兌換 */
		case _ECR_7E1_HG_REDEEM_REFUND_NO_:		/* 扣抵退貨 */
		case _ECR_7E1_HG_REWARD_REFUND_NO_:		/* 回饋退貨 */
			/* 最多八位 所以從106開始 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szHGRedeemPoint[0], 8);
			pobTran->srBRec.lnHGTransactionPoint = atol(szTemplate);
			break;
		default :
			break;
	}
	
	
	
	/* 處理櫃號(Store ID) */
	switch (inTransType)
	{
		default:
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szStoreId[0], 18);
			
			if (strlen(szTemplate) > 0)
			{
				memcpy(pobTran->srBRec.szStoreID, szTemplate, strlen(szTemplate));
				/* 櫃號不滿18，補空白 */
				if (strlen(szTemplate) < 18)
				{
					memset(&pobTran->srBRec.szStoreID[strlen(szTemplate)], 0x20, 18 - strlen(szTemplate));
				}

			}
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "Store ID", pobTran->srBRec.szStoreID);
				inLogPrintf(AT, szDebugMsg);
			}
			break;
	}
	
	/* szDataBuffer[132] 144規格要看紅利分期來看使用用途  卡號查詢/每期金額/保留欄位 */
	switch (inTransType)
	{
		case _ECR_7E1_INSTALLMENT_REFUND_NO_:		/* 分期退貨 */
		case _ECR_7E1_INSTALLMENT_ADJUST_NO_:		/* 分期調帳 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szField_27[0], 10);	/* 12位數，這裡只取10位 */
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "Payment Amount", szTemplate);
				inLogPrintf(AT, szDebugMsg);
			}
			/* 每期金額 Installment Payment Amount (含小數 2位) */
			pobTran->srBRec.lnInstallmentPayment = atol(szTemplate);
			
			break;
		default:
			break;
	}
	
	/* 若前一次為卡號查詢，標記為第二次ECR */
	if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
	{
		pobTran->uszCardInquiryFirstBit = VS_FALSE;
		pobTran->uszCardInquirysSecondBit = VS_TRUE;
		
		/* 重新決定TRT流程 */
                if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
                {
                        /* 金融卡查詢要二次過卡，不依循前次查詢界面 */
                        inCardInquiryTRTType = _CARD_INQUIRY_TYPE_NONE_;
                }
                else
                {
                        if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
                        {
                                inCardInquiryTRTType = _CARD_INQUIRY_TYPE_CHIP_;
                        }
                        else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
                        {
                                inCardInquiryTRTType = _CARD_INQUIRY_TYPE_CTLS_;
                        }
                        else
                        {
                                inCardInquiryTRTType = _CARD_INQUIRY_TYPE_MEG_;
                        }
                }
	}
	
	/* szDataBuffer[142] 144規格要看紅利分期來看使用用途  卡號查詢/每期金額/保留欄位 */
	switch (inTransType)
	{
		/* 啟動卡號查詢 Start Trans Type */
		case _ECR_7E1_START_CARD_NO_INQUIRY_NO_:		/* 卡號查詢 */
			/* 卡號查詢 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &srECROb->srTransData.szStartTransType[0], 2);
			inTransType = atoi(szTemplate);
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "Start Trans Type", szTemplate);
				inLogPrintf(AT, szDebugMsg);
			}
			
			switch (inTransType)
			{
				case _ECR_7E1_SALE_NO_ :
				case _ECR_7E1_REFUND_NO_ :
				case _ECR_7E1_PREAUTH_NO_ :
					if (pobTran->uszCardInquirysSecondBit == VS_TRUE)
					{

					}
					else
					{
						pobTran->uszCardInquiryFirstBit = VS_TRUE;
					}
					/* 有值影響到後面判斷  確保都是空格 */
					memcpy(&srECROb->srTransData.szField_05[0], " ", 1);
					memcpy(&srECROb->srTransData.szTransTypeIndicator[0], " ", 1);
					break;
				case _ECR_7E1_OFFLINE_NO_ :
				case _ECR_7E1_INSTALLMENT_NO_ :
				case _ECR_7E1_REDEEM_NO_ :
				case _ECR_7E1_INSTALLMENT_REFUND_NO_ :
				case _ECR_7E1_REDEEM_REFUND_NO_ :
				case _ECR_7E1_INSTALLMENT_ADJUST_NO_ :
				case _ECR_7E1_REDEEM_ADJUST_NO_ :
					if (pobTran->uszCardInquirysSecondBit == VS_TRUE)
					{

					}
					else
					{
						pobTran->uszCardInquiryFirstBit = VS_TRUE;
					}
					memcpy(&srECROb->srTransData.szField_05[0], " ", 1);
					memcpy(&srECROb->srTransData.szTransTypeIndicator[0], " ", 1);
					break;
				default :
					pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
					return (VS_ERROR);
			}
			break;
		case _ECR_7E1_HG_REWARD_SALE_NO_:		/* HG紅利積點*/
		case _ECR_7E1_HG_REWARD_REDEMPTION_NO_:		/* HG紅利積點(紅利)*/
		case _ECR_7E1_HG_REWARD_INSTALLMENT_NO_:	/* HG紅利積點(分期)*/
		case _ECR_7E1_HG_POINT_CERTAIN_NO_:		/* HG加價購 */
		case _ECR_7E1_HG_ONLINE_REDEEMPTION_NO_:	/* HG點數扣抵 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szHGPaymentTool[0], 2);
			inHGPaymetType = atoi(szTemplate);
			if (inHGPaymetType == 0)
			{
				pobTran->srBRec.lnHGPaymentType = 0;
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "_HG_PAY_EDC_SELECT_");
				}
			}
			else if (inHGPaymetType == 1)
			{
				pobTran->srBRec.lnHGPaymentType = _HG_PAY_CREDIT_;
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "_HG_PAY_CREDIT_");
				}
			}
			else if (inHGPaymetType == 2)
			{
				pobTran->srBRec.lnHGPaymentType = _HG_PAY_CASH_;
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "_HG_PAY_CASH_");
				}
			}
			else if (inHGPaymetType == 3)
			{
				pobTran->srBRec.lnHGPaymentType = _HG_PAY_GIFT_PAPER_;
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "_HG_PAY_GIFT_PAPER_");
				}
			}
			else if (inHGPaymetType == 4)
			{
				pobTran->srBRec.lnHGPaymentType = _HG_PAY_CREDIT_INSIDE_;
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "_HG_PAY_CREDIT_INSIDE_");
				}
			}
			else if (inHGPaymetType == 5)
			{
				pobTran->srBRec.lnHGPaymentType = _HG_PAY_CUP_;
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "_HG_PAY_CUP_");
				}
			}
			else
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "HG PaymentType Error, %d", inHGPaymetType);
				}
				pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
				return (VS_ERROR);
			}
			break;
		default:
			break;
	}
	
	if (inECR_Check_Exception(pobTran) != VS_SUCCESS)
	{
		memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
		sprintf(pobTran->szErrorMsgBuff2, "ECR_Check_Exception error");
		return (VS_ERROR);
	}
	
	memset(szFiscContactlessEnable, 0x00, sizeof(szFiscContactlessEnable));
	inGetSmartPayContactlessEnable(szFiscContactlessEnable);
	memset(szCTLSEnable, 0x00, sizeof(szCTLSEnable));
	inGetContactlessEnable(szCTLSEnable);
	memset(szCUPContactlessEnable, 0x00, sizeof(szCUPContactlessEnable));
	inGetCUPContactlessEnable(szCUPContactlessEnable);
	
	/* 跑OPT */
	switch (inTransType)
	{
		case _ECR_7E1_SALE_NO_:
			/* 金融卡和一般Sale跑不同OPT，所以要分 */
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{	
				inDISP_ClearAll();
				/* 第三層顯示 ＜消費扣款＞ */
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_FISC_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜消費扣款＞ */

				pobTran->inFunctionID = _FISC_SALE_;
				if (!memcmp(szCTLSEnable, "Y", 1)		&&
				    !memcmp(szFiscContactlessEnable, "Y", 1)	&&
				    guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_FISC_SALE_CTLS_;
					pobTran->inRunTRTID = _TRT_FISC_SALE_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_FISC_SALE_;
					pobTran->inRunTRTID = _TRT_FISC_SALE_ICC_;
				}
				
				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}
		
				pobTran->inTransactionCode = _FISC_SALE_;
				pobTran->srBRec.inCode = _FISC_SALE_;
				pobTran->srBRec.inOrgCode = _FISC_SALE_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_CUP_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜銀聯一般交易＞ */

				if (!memcmp(szCTLSEnable, "Y", 1)		&&
				    !memcmp(szCUPContactlessEnable, "Y", 1)	&&
				    guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
					pobTran->inRunTRTID = _TRT_CUP_SALE_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
					pobTran->inRunTRTID = _TRT_CUP_SALE_;
				}

				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}
				
				pobTran->inFunctionID = _CUP_SALE_;
				pobTran->inTransactionCode = _CUP_SALE_;
				pobTran->srBRec.inCode = pobTran->inTransactionCode;
				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_TICKET_DEDUCT_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜購貨交易＞ */

				pobTran->inFunctionID = _TICKET_DEDUCT_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_TICKET_DEDUCT_;
				}
				else
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "感應未初始化，不能使用此功能");
					}
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "CTLS not Init");
					return (VS_ERROR);
				}

				pobTran->inTransactionCode = _TICKET_DEDUCT_;
				pobTran->srTRec.inCode = _TICKET_DEDUCT_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else
			{
				inDISP_ClearAll();
				/* 第三層顯示 ＜一般交易＞ */
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般交易＞ */

				pobTran->inFunctionID = _SALE_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
					pobTran->inRunTRTID = _TRT_SALE_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
					pobTran->inRunTRTID = _TRT_SALE_;
				}

				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}
				
				pobTran->inTransactionCode = _SALE_;
				pobTran->srBRec.inCode = _SALE_;
				pobTran->srBRec.inOrgCode = _SALE_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			
			break;
		case _ECR_7E1_VOID_NO_:
			/* 金融卡和一般Sale跑不同OPT，所以要分 */
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_FISC_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜沖正交易＞ */

				pobTran->inFunctionID = _FISC_VOID_;
				pobTran->inRunOperationID = _OPERATION_VOID_;
				pobTran->inRunTRTID = _TRT_FISC_VOID_;

				pobTran->inTransactionCode = _FISC_VOID_;
				pobTran->srBRec.inCode = _FISC_VOID_;
				pobTran->srBRec.inOrgCode = _FISC_VOID_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜取消交易＞ */

				pobTran->inRunOperationID = _OPERATION_VOID_;
				pobTran->inRunTRTID = _TRT_CUP_VOID_;

				pobTran->inFunctionID = _CUP_VOID_;
				pobTran->inTransactionCode = _CUP_VOID_;
				pobTran->srBRec.inCode = pobTran->inTransactionCode;
				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜取消交易＞ */

				pobTran->inFunctionID = _VOID_;
				pobTran->inRunOperationID = _OPERATION_VOID_;
				pobTran->inRunTRTID = _TRT_VOID_;

				pobTran->inTransactionCode = _VOID_;
				pobTran->srBRec.inCode = _VOID_;
				pobTran->srBRec.inOrgCode = _VOID_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			break;
		case _ECR_7E1_REFUND_NO_:
			/* 金融卡和一般Sale跑不同OPT，所以要分 */
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_FISC_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);	/* 第二層顯示 ＜退費交易＞ */

				pobTran->inFunctionID = _FISC_REFUND_;                       
				if (!memcmp(szCTLSEnable, "Y", 1)		&&
				    !memcmp(szFiscContactlessEnable, "Y", 1)	&&
				    guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_FISC_REFUND_CTLS_;
					pobTran->inRunTRTID = _TRT_FISC_REFUND_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_FISC_REFUND_;
					pobTran->inRunTRTID = _TRT_FISC_REFUND_ICC_;
				}

				pobTran->inTransactionCode = _FISC_REFUND_;
				pobTran->srBRec.inCode = _FISC_REFUND_;
				pobTran->srBRec.inOrgCode = _FISC_REFUND_;

				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}
				
				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			else if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般退貨＞ */

				if (!memcmp(szCTLSEnable, "Y", 1)		&&
				    !memcmp(szCUPContactlessEnable, "Y", 1)	&&
				    guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_GET_CARD_FIRST_;
					pobTran->inRunTRTID = _TRT_CUP_REFUND_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_AMOUNT_FIRST_CUP_;
					pobTran->inRunTRTID = _TRT_CUP_REFUND_;
				}

				pobTran->inFunctionID = _CUP_REFUND_;
				pobTran->inTransactionCode = _CUP_REFUND_;
				pobTran->srBRec.inCode = pobTran->inTransactionCode;
				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;

				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}
				
				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_TICKET_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜退貨交易＞ */

				pobTran->inFunctionID = _TICKET_REFUND_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_TICKET_REFUND_;
				}
				else
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "感應未初始化，不能使用此功能");
					}
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "CTLS not Init");
					return (VS_ERROR);
				}

				pobTran->inTransactionCode = _TICKET_REFUND_;
				pobTran->srTRec.inCode = _TICKET_REFUND_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般退貨＞ */

				pobTran->inFunctionID = _REFUND_;

				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_GET_CARD_FIRST_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_AMOUNT_FIRST_;
				}
				pobTran->inRunTRTID = _TRT_REFUND_;

				pobTran->inTransactionCode = _REFUND_;
				pobTran->srBRec.inCode = _REFUND_;
				pobTran->srBRec.inOrgCode = _REFUND_;

				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}
				
				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;
			
		case _ECR_7E1_OFFLINE_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_SALE_OFFLINE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜交易補登＞ */
			
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_041_CASH_, _CUSTOMER_INDICATOR_SIZE_))
			{
				pobTran->inFunctionID = _FORCE_CASH_ADVANCE_;
				pobTran->inRunOperationID = _OPERATION_SALE_OFFLINE_;
				pobTran->inRunTRTID = _TRT_FORCE_CASH_ADVANCE_;

				pobTran->inTransactionCode = _FORCE_CASH_ADVANCE_;
				pobTran->srBRec.inCode = _FORCE_CASH_ADVANCE_;
				pobTran->srBRec.inOrgCode = _FORCE_CASH_ADVANCE_;
			}
			else
			{
				pobTran->inFunctionID = _SALE_OFFLINE_;
				pobTran->inRunOperationID = _OPERATION_SALE_OFFLINE_;
				pobTran->inRunTRTID = _TRT_SALE_OFFLINE_;

				pobTran->inTransactionCode = _SALE_OFFLINE_;
				pobTran->srBRec.inCode = _SALE_OFFLINE_;
				pobTran->srBRec.inOrgCode = _SALE_OFFLINE_;
			}
			
			/* 卡號查詢不過第二次卡，所以要直接決定TRT */
			if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
			{
				inNCCC_Func_Decide_CTLS_TRT(pobTran);
			}
			else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
			{
				inNCCC_Func_Decide_ICC_TRT(pobTran);
			}
			else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
			{
				inNCCC_Func_Decide_MEG_TRT(pobTran);
			}
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			break;
			
		case _ECR_7E1_PREAUTH_NO_:
			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_PRE_AUTH_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜預先授權＞ */

				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_PRE_AUTH_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_PRE_AUTH_;
				}
				pobTran->inRunTRTID = _TRT_PRE_AUTH_;

				pobTran->inFunctionID = _CUP_PRE_AUTH_;
				pobTran->inTransactionCode = _CUP_PRE_AUTH_;
				pobTran->srBRec.inCode = pobTran->inTransactionCode;
				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
                                
                                /* 卡號查詢不過第二次卡，所以要直接決定TRT */
                                if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
                                {
                                        inNCCC_Func_Decide_CTLS_TRT(pobTran);
                                }
                                else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
                                {
                                        inNCCC_Func_Decide_ICC_TRT(pobTran);
                                }
                                else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
                                {
                                        inNCCC_Func_Decide_MEG_TRT(pobTran);
                                }

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_PRE_AUTH_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜預先授權＞ */

				pobTran->inFunctionID = _PRE_AUTH_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_PRE_AUTH_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_PRE_AUTH_;
				}
				pobTran->inRunTRTID = _TRT_PRE_AUTH_;

				pobTran->inTransactionCode = _PRE_AUTH_;
				pobTran->srBRec.inCode = _PRE_AUTH_;
				pobTran->srBRec.inOrgCode = _PRE_AUTH_;
                                
                                /* 卡號查詢不過第二次卡，所以要直接決定TRT */
                                if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
                                {
                                        inNCCC_Func_Decide_CTLS_TRT(pobTran);
                                }
                                else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
                                {
                                        inNCCC_Func_Decide_ICC_TRT(pobTran);
                                }
                                else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
                                {
                                        inNCCC_Func_Decide_MEG_TRT(pobTran);
                                }

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			
			break;
						
		case _ECR_7E1_INSTALLMENT_NO_:
//			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
//			{
//				inDISP_ClearAll();
//				/* 保留給銀聯分期使用 */
//				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
//				inDISP_PutGraphic(_MENU_INST_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜分期付款＞ */
//
//				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
//				{
//					pobTran->inRunOperationID = _OPERATION_INST_SALE_CTLS_;
//				}
//				else
//				{
//					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
//				}
//				pobTran->inRunTRTID = _TRT_INST_SALE_;
//
//				pobTran->inFunctionID = _CUP_INST_SALE_;
//				pobTran->inTransactionCode = _CUP_INST_SALE_;
//				pobTran->srBRec.inCode = pobTran->inTransactionCode;
//				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
//
//				pobTran->srBRec.uszInstallmentBit = VS_TRUE;
//				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
//				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
//				{
//					inNCCC_Func_Decide_CTLS_TRT(pobTran);
//				}
//				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
//				{
//					inNCCC_Func_Decide_ICC_TRT(pobTran);
//				}
//				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
//				{
//					inNCCC_Func_Decide_MEG_TRT(pobTran);
//				}
//
//				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
//			}
//			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_INST_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜分期付款＞ */

				pobTran->inFunctionID = _INST_SALE_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_INST_SALE_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
				}
				pobTran->inRunTRTID = _TRT_INST_SALE_;

				pobTran->inTransactionCode = _INST_SALE_;
				pobTran->srBRec.inCode = _INST_SALE_;
				pobTran->srBRec.inOrgCode = _INST_SALE_;

				pobTran->srBRec.uszInstallmentBit = VS_TRUE;
				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;
			
		case _ECR_7E1_INSTALLMENT_REFUND_NO_:
//			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
//			{
//				inDISP_ClearAll();
//				/* 保留給銀聯分期退貨使用 */
//				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
//				inDISP_PutGraphic(_MENU_INST_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜分期退貨＞ */
//
//				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
//				{
//					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_GET_CARD_FIRST_;
//				}
//				else
//				{
//					pobTran->inRunOperationID = _OPERATION_REFUND_;
//				}
//				pobTran->inRunTRTID = _TRT_INST_REFUND_;
//
//				pobTran->inFunctionID = _CUP_INST_REFUND_;
//				pobTran->inTransactionCode = _CUP_INST_REFUND_;
//				pobTran->srBRec.inCode = pobTran->inTransactionCode;
//				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
//
//				pobTran->srBRec.uszInstallmentBit = VS_TRUE;
//				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
//				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
//				{
//					inNCCC_Func_Decide_CTLS_TRT(pobTran);
//				}
//				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
//				{
//					inNCCC_Func_Decide_ICC_TRT(pobTran);
//				}
//				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
//				{
//					inNCCC_Func_Decide_MEG_TRT(pobTran);
//				}
//
//				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
//			}
//			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_INST_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜分期退貨＞ */

				pobTran->inFunctionID = _INST_REFUND_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_GET_CARD_FIRST_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_;
				}
				pobTran->inRunTRTID = _TRT_INST_REFUND_;

				pobTran->inTransactionCode = _INST_REFUND_;
				pobTran->srBRec.inCode = _INST_REFUND_;
				pobTran->srBRec.inOrgCode = _INST_REFUND_;

				pobTran->srBRec.uszInstallmentBit = VS_TRUE;
				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;
			
		case _ECR_7E1_INSTALLMENT_ADJUST_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_INST_ADJUST_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜分期調帳＞ */
			
			pobTran->inFunctionID = _INST_ADJUST_;
			pobTran->inRunOperationID = _OPERATION_INST_ADJUST_;
			pobTran->inRunTRTID = _TRT_INST_ADJUST_;

			pobTran->inTransactionCode = _INST_ADJUST_;
			pobTran->srBRec.inCode = _INST_ADJUST_;
			pobTran->srBRec.inOrgCode = _INST_ADJUST_;
			
			pobTran->srBRec.uszInstallmentBit = VS_TRUE;
			/* 卡號查詢不過第二次卡，所以要直接決定TRT */
			if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
			{
				inNCCC_Func_Decide_CTLS_TRT(pobTran);
			}
			else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
			{
				inNCCC_Func_Decide_ICC_TRT(pobTran);
			}
			else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
			{
				inNCCC_Func_Decide_MEG_TRT(pobTran);
			}
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			break;
			
		case _ECR_7E1_REDEEM_NO_:
//			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
//			{
//				inDISP_ClearAll();
//				/* 保留給銀聯紅利扣抵使用 */
//				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
//				inDISP_PutGraphic(_MENU_REDEEM_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利扣抵＞ */
//
//				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
//				{
//					pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
//				}
//				else
//				{
//					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
//				}
//				pobTran->inRunTRTID = _TRT_REDEEM_SALE_;
//
//				pobTran->inFunctionID = _CUP_REDEEM_SALE_;
//				pobTran->inTransactionCode = _CUP_REDEEM_SALE_;
//				pobTran->srBRec.inCode = pobTran->inTransactionCode;
//				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
//
//				pobTran->srBRec.uszRedeemBit = VS_TRUE;
//				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
//				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
//				{
//					inNCCC_Func_Decide_CTLS_TRT(pobTran);
//				}
//				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
//				{
//					inNCCC_Func_Decide_ICC_TRT(pobTran);
//				}
//				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
//				{
//					inNCCC_Func_Decide_MEG_TRT(pobTran);
//				}
//
//				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
//			}
//			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REDEEM_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利扣抵＞ */

				pobTran->inFunctionID = _REDEEM_SALE_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
				}
				pobTran->inRunTRTID = _TRT_REDEEM_SALE_;

				pobTran->inTransactionCode = _REDEEM_SALE_;
				pobTran->srBRec.inCode = _REDEEM_SALE_;
				pobTran->srBRec.inOrgCode = _REDEEM_SALE_;

				pobTran->srBRec.uszRedeemBit = VS_TRUE;
				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;
		
		case _ECR_7E1_REDEEM_REFUND_NO_:
//			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
//			{
//				inDISP_ClearAll();
//				/* 保留給銀聯紅利退貨使用 */
//				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
//				inDISP_PutGraphic(_MENU_REDEEM_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利退貨＞ */
//
//				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
//				{
//					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_GET_CARD_FIRST_;
//				}
//				else
//				{
//					pobTran->inRunOperationID = _OPERATION_REFUND_;
//				}
//				pobTran->inRunTRTID = _TRT_REDEEM_REFUND_;
//
//				pobTran->inFunctionID = _CUP_REDEEM_REFUND_;
//				pobTran->inTransactionCode = _CUP_REDEEM_REFUND_;
//				pobTran->srBRec.inCode = pobTran->inTransactionCode;
//				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
//
//				pobTran->srBRec.uszRedeemBit = VS_TRUE;
//				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
//				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
//				{
//					inNCCC_Func_Decide_CTLS_TRT(pobTran);
//				}
//				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
//				{
//					inNCCC_Func_Decide_ICC_TRT(pobTran);
//				}
//				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
//				{
//					inNCCC_Func_Decide_MEG_TRT(pobTran);
//				}
//
//				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
//			}
//			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REDEEM_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利退貨＞ */

				pobTran->inFunctionID = _REDEEM_REFUND_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_GET_CARD_FIRST_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_;
				}
				pobTran->inRunTRTID = _TRT_REDEEM_REFUND_;

				pobTran->inTransactionCode = _REDEEM_REFUND_;
				pobTran->srBRec.inCode = _REDEEM_REFUND_;
				pobTran->srBRec.inOrgCode = _REDEEM_REFUND_;

				pobTran->srBRec.uszRedeemBit = VS_TRUE;
				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;
			
		case _ECR_7E1_REDEEM_ADJUST_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_REDEEM_ADJUST_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利調帳＞ */
			
			pobTran->inFunctionID = _REDEEM_ADJUST_;
			pobTran->inRunOperationID = _OPERATION_REDEEM_ADJUST_;
			pobTran->inRunTRTID = _TRT_REDEEM_ADJUST_;

			pobTran->inTransactionCode = _REDEEM_ADJUST_;
			pobTran->srBRec.inCode = _REDEEM_ADJUST_;
			pobTran->srBRec.inOrgCode = _REDEEM_ADJUST_;
			
			pobTran->srBRec.uszRedeemBit = VS_TRUE;
			/* 卡號查詢不過第二次卡，所以要直接決定TRT */
			if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
			{
				inNCCC_Func_Decide_CTLS_TRT(pobTran);
			}
			else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
			{
				inNCCC_Func_Decide_ICC_TRT(pobTran);
			}
			else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
			{
				inNCCC_Func_Decide_MEG_TRT(pobTran);
			}
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
			
		/* 快樂購紅利積點 */
		case _ECR_7E1_HG_REWARD_SALE_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡紅利積點＞ */
			
			pobTran->inFunctionID = _SALE_;
			pobTran->inRunOperationID = _OPERATION_HG_;
			pobTran->inRunTRTID = _TRT_HG_REWARD_CREDIT_CTLS_;

			/* 這邊先設成sale，若之後支付方式選銀聯卡，會在那邊改成CUP_SALE */
			pobTran->inTransactionCode = _SALE_;
			pobTran->srBRec.inCode = _SALE_;
			pobTran->srBRec.inOrgCode = _SALE_;
			pobTran->srBRec.lnHGTransactionType = _HG_REWARD_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		
		/* 快樂購紅利積點─分期付款 */
		case _ECR_7E1_HG_REWARD_INSTALLMENT_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_INS_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡分期付款＞ */
			
			pobTran->inFunctionID = _INST_SALE_;
			pobTran->inRunOperationID = _OPERATION_HG_I_R_;
			pobTran->inRunTRTID = _TRT_HG_REWARD_INSTALLMENT_CTLS_;

			pobTran->inTransactionCode = _INST_SALE_;
			pobTran->srBRec.inCode = _INST_SALE_;
			pobTran->srBRec.inOrgCode = _INST_SALE_;
			pobTran->srBRec.lnHGTransactionType = _HG_REWARD_;
			pobTran->srBRec.uszInstallmentBit = VS_TRUE;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		
		/* 快樂購紅利積點─紅利扣抵 */
		case _ECR_7E1_HG_REWARD_REDEMPTION_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_REDEEM_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡紅利扣抵＞ */
			
			pobTran->inFunctionID = _REDEEM_SALE_;
			pobTran->inRunOperationID = _OPERATION_HG_I_R_;
			pobTran->inRunTRTID = _TRT_HG_REWARD_REDEMPTION_CTLS_;

			pobTran->inTransactionCode = _REDEEM_SALE_;
			pobTran->srBRec.inCode = _REDEEM_SALE_;
			pobTran->srBRec.inOrgCode = _REDEEM_SALE_;
			pobTran->srBRec.lnHGTransactionType = _HG_REWARD_;
			pobTran->srBRec.uszRedeemBit = VS_TRUE;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
			
		/* 快樂購點數扣抵 */
		case _ECR_7E1_HG_ONLINE_REDEEMPTION_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_ONLINE_REDEEM_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡點數扣抵＞ */
			
			pobTran->inFunctionID = _SALE_;
			pobTran->inRunOperationID = _OPERATION_HG_;
			pobTran->inRunTRTID = _TRT_HG_ONLINE_REDEEM_CREDIT_CTLS_;

			pobTran->inTransactionCode = _SALE_;
			pobTran->srBRec.inCode = _SALE_;
			pobTran->srBRec.inOrgCode = _SALE_;
			pobTran->srBRec.lnHGTransactionType = _HG_ONLINE_REDEEM_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		
		/* 快樂購加價購 */
		case _ECR_7E1_HG_POINT_CERTAIN_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_POINT_CERTAIN_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡加價購＞ */
			
			pobTran->inFunctionID = _SALE_;
			pobTran->inRunOperationID = _OPERATION_HG_;
			pobTran->inRunTRTID = _TRT_HG_POINT_CERTAIN_CREDIT_CTLS_;

			pobTran->inTransactionCode = _SALE_;
			pobTran->srBRec.inCode = _SALE_;
			pobTran->srBRec.inOrgCode = _SALE_;
			pobTran->srBRec.lnHGTransactionType = _HG_POINT_CERTAIN_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		
		/* 快樂購點數兌換 */
		case _ECR_7E1_HG_FULL_REDEEMPTION_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_REDEMPTION_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡點數兌換＞ */
			
			pobTran->inFunctionID = _SALE_;
			pobTran->inRunOperationID = _OPERATION_HG_;
			pobTran->inRunTRTID = _TRT_HG_FULL_REDEMPTION_;

			pobTran->inTransactionCode = _SALE_;
			pobTran->srBRec.inCode = _SALE_;
			pobTran->srBRec.inOrgCode = _SALE_;
			pobTran->srBRec.lnHGTransactionType = _HG_FULL_REDEMPTION_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		
		/* 快樂購扣抵退貨 */
		case _ECR_7E1_HG_REDEEM_REFUND_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);					/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_REDEEM_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡扣抵退貨＞ */
			
			pobTran->inFunctionID = _HG_REDEEM_REFUND_;
			pobTran->inRunOperationID = _OPERATION_HG_REFUND_;
			pobTran->inRunTRTID = _TRT_HG_REDEEM_REFUND_;

			pobTran->inTransactionCode = _HG_REDEEM_REFUND_;
			pobTran->srBRec.inCode = _HG_REDEEM_REFUND_;
			pobTran->srBRec.inOrgCode = _HG_REDEEM_REFUND_;
			pobTran->srBRec.lnHGTransactionType = _HG_REDEEM_REFUND_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		
		/* 快樂購回饋退貨 */
		case _ECR_7E1_HG_REWARD_REFUND_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡回饋退貨＞ */
			
			pobTran->inFunctionID = _HG_REWARD_REFUND_;
			pobTran->inRunOperationID = _OPERATION_HG_REFUND_;
			pobTran->inRunTRTID = _TRT_HG_REWARD_REFUND_;

			pobTran->inTransactionCode = _HG_REWARD_REFUND_;
			pobTran->srBRec.inCode = _HG_REWARD_REFUND_;
			pobTran->srBRec.inOrgCode = _HG_REWARD_REFUND_;
			pobTran->srBRec.lnHGTransactionType = _HG_REWARD_REFUND_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		
		/* 快樂購點數查詢 */
		case _ECR_7E1_HG_POINT_INQUIRY_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_INQUIRY_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡點數查詢＞ */
			
			pobTran->inFunctionID = _HG_INQUIRY_;
			pobTran->inRunOperationID = _OPERATION_HG_;
			pobTran->inRunTRTID = _TRT_HG_INQUIRY_;

			pobTran->inTransactionCode = _HG_INQUIRY_;
			pobTran->srBRec.inCode = _HG_INQUIRY_;
			pobTran->srBRec.inOrgCode = _HG_INQUIRY_;
			pobTran->srBRec.lnHGTransactionType = _HG_INQUIRY_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
			
		case _ECR_7E1_SETTLEMENT_NO_:
			inDISP_ClearAll();
			/* 第三層顯示 ＜結帳交易＞ */
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_SETTLE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜結帳交易＞ */

			pobTran->inFunctionID = _SETTLE_;
			pobTran->inRunOperationID = _OPERATION_SETTLE_;
			pobTran->inRunTRTID = _TRT_SETTLE_;
			
			pobTran->inTransactionCode = _SETTLE_;
			pobTran->srBRec.inCode = _SETTLE_;
			pobTran->srBRec.inOrgCode = _SETTLE_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		case _ECR_7E1_END_CARD_NO_INQUIRY_NO_:
			pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_END_INQUIRY_SUCCESS_;
			inRetVal = VS_ERROR;
			break;
		default:
			pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
			inRetVal = VS_ERROR;
			break;
	}
	
	inECR_ReCheck_144_TRT_Flow(pobTran);
	
	return (inRetVal);
}

/*
Function        :inECR_7E1_Standard_Pack
Date&Time       :2017/11/16 上午 10:45
Describe        :先把要送的資料組好
*/
int inECR_7E1_Standard_Pack(TRANSACTION_OBJECT *pobTran, char *szDataBuffer, ECR_TABLE * srECROb)
{
	int	inRetVal;
	int 	i, inCardLen;
	int	inPacketSizes = 0;
	char 	szTemplate[100 + 1];
	
	if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
	{
		strcpy(srECROb->srTransData.szField_05, "C");
	}
	
	/* Trans Type (2 Byte) */
	memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szTransType, 2);
	inPacketSizes += 2;

	/* HOST ID (2 Byte) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTRTFileName(szTemplate);

	if (!memcmp(szTemplate, _TRT_FILE_NAME_CREDIT_, strlen(_TRT_FILE_NAME_CREDIT_)))
		memcpy(&szDataBuffer[inPacketSizes], _ECR_7E1_NCCC_HOSTID_NCCC_, 2);
	/* 2018/5/8 下午 2:10 DFS需求不再使用大來主機 */
	else if (!memcmp(szTemplate, _TRT_FILE_NAME_DCC_, strlen(_TRT_FILE_NAME_DCC_)))
		memcpy(&szDataBuffer[inPacketSizes], _ECR_7E1_NCCC_HOSTID_DCC_, 2);

	inPacketSizes += 2;

	/* Receipt No (6 Byte) */
	if (pobTran->srBRec.lnOrgInvNum != 0L ||
	   !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_POINT_INQUIRY_, 2)) /* 點數查詢就算是沒有INV也要回傳 */
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%06ld", pobTran->srBRec.lnOrgInvNum);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 6);
	}

	inPacketSizes += 6;

	/* Card No (19 Byte)(左靠右補空白) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_POINT_INQUIRY_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_FULL_REDEEMPTION_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REDEEM_REFUND_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REWARD_REFUND_, 2)	||
	    pobTran->srBRec.inOrgCode == _HG_REWARD_REFUND_				||	/* 取消回饋或扣抵退貨不需回傳信用卡卡號 */
	    pobTran->srBRec.inOrgCode == _HG_REDEEM_REFUND_				||
	    pobTran->srBRec.lnHGPaymentType == _HG_PAY_CASH_				||	/* 若是純HG交易，信用卡卡號不需回傳 */
	    pobTran->srBRec.lnHGPaymentType == _HG_PAY_GIFT_PAPER_			||
	    pobTran->srBRec.inHGCode == _HG_FULL_REDEMPTION_)				/* 修改ECR交易取消點數兌換多印卡號。*/
	{
		inPacketSizes += 19;
	}
	else
	{
		/* 卡號是否遮掩 */
		inRetVal = inECR_CardNoTruncateDecision(pobTran);

		/* 要遮卡號 */
		if (inRetVal == VS_SUCCESS)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			strcpy(szTemplate, pobTran->srBRec.szPAN);

			/* HAPPG_GO 卡不掩飾 */
			if (!memcmp(&pobTran->srBRec.szPAN[0], "9552", 4))
			{

			}
			else
			{
				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
				/* 卡號長度 */
				inCardLen = strlen(szTemplate);

				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
				for (i = 6; i < (inCardLen - 4); i ++)
					szTemplate[i] = '*';
			}

			memcpy(&szDataBuffer[inPacketSizes], szTemplate, strlen(szTemplate));
		}
		else
		{
			memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szPAN, strlen(pobTran->srBRec.szPAN));
		}

		inPacketSizes += 19;
	}

	/* Card Expire Date (4 Byte) */
	/* 這裡回傳條件抄Verifone的Code */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_POINT_INQUIRY_, 2)	||	/* 點數查詢 */
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_FULL_REDEEMPTION_, 2)	||	/* 點數扣抵 */
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REDEEM_REFUND_, 2)	||	/* 扣抵退貨 */
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REWARD_REFUND_, 2)	||	/* 回饋退貨 */
	    pobTran->srBRec.inOrgCode == _HG_REWARD_REFUND_				||	/* 取消回饋或扣抵退貨不需回傳信用卡卡號 */
	    pobTran->srBRec.inOrgCode == _HG_REDEEM_REFUND_				||
	    pobTran->srBRec.lnHGPaymentType == _HG_PAY_CASH_				||	/* 若是純HG交易，信用卡卡號不需回傳 */
	    pobTran->srBRec.lnHGPaymentType == _HG_PAY_GIFT_PAPER_)
	{
		inPacketSizes += 4;
	}
	else
	{
		/* 有效期是否回傳 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetECRExpDateReturnEnable(szTemplate);
		
		/* 要回傳有效期 */
		if (!memcmp(&szTemplate, "Y", 1))
			memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szExpDate, strlen(pobTran->srBRec.szExpDate));

		inPacketSizes += 4;
	}

	/* Trans Amount (12 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_SALE_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REFUND_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_OFFLINE_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_PREAUTH_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_INSTALLMENT_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_INSTALLMENT_REFUND_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_INSTALLMENT_ADJUST_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REDEEM_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REDEEM_REFUND_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REDEEM_ADJUST_, 2))
	{
		if (pobTran->srBRec.lnTxnAmount != 0)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%010ld00", pobTran->srBRec.lnTxnAmount);
			memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
		}
	}
	inPacketSizes += 12;

	/* Trans Date (6 Byte) & Trans Time (6 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_SALE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REFUND_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_VOID_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_OFFLINE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_INSTALLMENT_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_INSTALLMENT_REFUND_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REDEEM_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_REDEEM_REFUND_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_POINT_INQUIRY_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_ONLINE_REDEEMPTION_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_POINT_CERTAIN_, 2)||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_FULL_REDEEMPTION_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REDEEM_REFUND_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REWARD_REFUND_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REWARD_SALE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REWARD_INSTALLMENT_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REWARD_REDEMPTION_, 2))
	{
		/* Trans Date */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		strcat(szTemplate, pobTran->srBRec.szDate);
		memcpy(&szDataBuffer[inPacketSizes], &szTemplate[2], 6);
		inPacketSizes += 6;

		/* Trans Time */
		memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szTime, 6);
		inPacketSizes += 6;
	}
	else
	{
		inPacketSizes += 6;
		inPacketSizes += 6;
	}

	/* Approval No (9 Byte) */
	if (strlen(pobTran->srBRec.szAuthCode) > 0)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		strcpy(szTemplate, pobTran->srBRec.szAuthCode);
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 9, _PADDING_RIGHT_);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 9);
	}

	inPacketSizes += 9;
	
	/* 保留欄位/實際支付金額(紅利交易)/分期期數 + 分期手續費(分期交易) (12 Byte) */
	if (pobTran->srBRec.uszRedeemBit == VS_TRUE && pobTran->srBRec.uszVOIDBit != VS_TRUE)
	{
		/* 實際支付金額 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010ld00", (pobTran->srBRec.lnRedemptionPaidCreditAmount + pobTran->srBRec.lnTipTxnAmount));
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
		
		inPacketSizes += 12;
	}
	else if (pobTran->srBRec.uszInstallmentBit == VS_TRUE && pobTran->srBRec.uszVOIDBit != VS_TRUE)
	{
		/* 分期期數 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%02ld", pobTran->srBRec.lnInstallmentPeriod);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 2);
		
		inPacketSizes += 2;
		
		/* 分期手續費 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010ld", pobTran->srBRec.lnInstallmentFormalityFee);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 10);
		
		inPacketSizes += 10;
	}
	else if (pobTran->srBRec.lnHGTransactionType == _HG_ONLINE_REDEEM_ && pobTran->srBRec.uszVOIDBit != VS_TRUE)
	{
		/* HG點數扣抵要回傳實際支付金額 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010ld00", pobTran->srBRec.lnHGAmount);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
		
		inPacketSizes += 12;
	}
	else
	{
		if (pobTran->srBRec.uszContactlessBit == VS_TRUE || pobTran->srBRec.uszRefundCTLSBit == VS_TRUE)
			memcpy(&szDataBuffer[inPacketSizes], "W", 1);
		
		inPacketSizes ++;
		
		if (!memcmp(&srECROb->srTransData.szField_05[0], "C", 1))
			memcpy(&szDataBuffer[inPacketSizes], &srECROb->srTransData.szField_05[0], 1);
		else
			memcpy(&szDataBuffer[inPacketSizes], "0", 1);

		inPacketSizes ++; /* CUP Indicator */
		inPacketSizes += 10;
	}
	
	/* ECR Response Code (4 Byte) */
	memcpy(&szDataBuffer[inPacketSizes], "0000", 4);
	inPacketSizes += 4;
	
	/* Terminal ID (8 Byte) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTerminalID(szTemplate);
	inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 8, _PADDING_RIGHT_);
	memcpy(&szDataBuffer[inPacketSizes], szTemplate, 8);
	inPacketSizes += 8;

	/* Reference No */
	if (strlen(pobTran->srBRec.szRefNo) > 0)
	{
		memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szRefNo, 12);
	}
	inPacketSizes += 12;
	
	/* 保留欄位/扣抵紅利點數(紅利交易)/首期金額(分期交易) (12 Byte) */
	if (pobTran->srBRec.uszRedeemBit == VS_TRUE)
	{
		/* 實際支付金額 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010ld00", pobTran->srBRec.lnRedemptionPoints);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
	}
	else if (pobTran->srBRec.uszInstallmentBit == VS_TRUE)
	{
		/* 首期金額 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010ld00", pobTran->srBRec.lnInstallmentDownPayment);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
	}
	else
	{
	        /* HG點數扣抵 回傳扣抵點數 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		if (pobTran->srBRec.lnHGTransactionType == _HG_ONLINE_REDEEM_ && pobTran->srBRec.uszVOIDBit != VS_TRUE)
			sprintf(szTemplate, "%012lu", pobTran->srBRec.lnHGTransactionPoint);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
	}
	inPacketSizes += 12;
	
	/* HGPAN(Request:Store Id Response:HappyGo Card Number) (18 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_POINT_INQUIRY_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_FULL_REDEEMPTION_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REDEEM_REFUND_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REWARD_REFUND_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_POINT_CERTAIN_, 2)||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_ONLINE_REDEEMPTION_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REWARD_SALE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REWARD_INSTALLMENT_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_7E1_HG_REWARD_REDEMPTION_, 2))
	{
		inRetVal = inECR_CardNoTruncateDecision_HG(pobTran);
		/* 要遮卡號 */
		if (inRetVal == VS_SUCCESS)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			strcpy(szTemplate, pobTran->srBRec.szHGPAN);

			/* HAPPG_GO 卡不掩飾 */
			if (!memcmp(&pobTran->srBRec.szHGPAN[0], "9552", 4))
			{

			}
			else
			{
				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
				/* 卡號長度 */
				inCardLen = strlen(szTemplate);

				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
				for (i = 6; i < (inCardLen - 4); i ++)
					szTemplate[i] = '*';

			}

			memcpy(&szDataBuffer[inPacketSizes], szTemplate, strlen(szTemplate));
		}
		else
		{
			memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szHGPAN, strlen(pobTran->srBRec.szHGPAN));
		}
	}
	inPacketSizes += 18;
	
	/* 保留欄位(10 Byte)+ START Trans Type (2 Byte) /剩餘紅利點數(紅利交易)/每期金額(分期交易) (12 Byte) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_START_CARD_NO_INQUIRY_, 1))
	{
		inPacketSizes += 10;
		
		/* START Trans Type (2 Byte) */
		memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szStartTransType, 2);
				
		inPacketSizes += 2;
	}
	else if (pobTran->srBRec.uszRedeemBit == VS_TRUE)
	{
		/* 剩餘紅利點數(紅利交易) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010ld00", pobTran->srBRec.lnRedemptionPointsBalance);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
		
		inPacketSizes += 12;
	}
	else if (pobTran->srBRec.uszInstallmentBit == VS_TRUE)
	{
		/* 每期金額(分期交易) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010ld", pobTran->srBRec.lnInstallmentPayment);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
		
		inPacketSizes += 12;
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inECR_7E1_Standard_Pack_ResponseCode
Date&Time       :2017/11/16 上午 10:49
Describe        :
*/
int inECR_7E1_Standard_Pack_ResponseCode(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuf)
{
	char	szTmeplate[12 + 1];
	
	memset(szTmeplate, 0x00, sizeof(szTmeplate));
	inGetTRTFileName(szTmeplate);
	
	/* 不知道Verifone是照哪裡的規格，總之照Verifone的Code */
	if (srECROb->srTransData.inErrorType == VS_TIMEOUT)
	{
	        /* 刷卡前沒有host id  先傳 03 */
	        if (pobTran->srBRec.inHDTIndex == -1)
	                memcpy(&szDataBuf[78], "03", 2);
	        else
		{
			if (memcmp(szTmeplate, _TRT_FILE_NAME_DCC_, strlen(_TRT_FILE_NAME_DCC_)) == 0)
				memcpy(&szDataBuf[78], "04", 2);
			else if (memcmp(szTmeplate, _TRT_FILE_NAME_CREDIT_, strlen(_TRT_FILE_NAME_CREDIT_)) == 0)
				memcpy(&szDataBuf[78], "03", 2);
			else if (memcmp(szTmeplate, _TRT_FILE_NAME_HG_, strlen(_TRT_FILE_NAME_HG_)) == 0)
				memcpy(&szDataBuf[78], "13", 2);
			/* 2018/5/8 下午 2:10 DFS需求不再使用大來主機 */
			else
			{
				memcpy(&szDataBuf[78], &szDataBuf[2], 2);
			}
			
		}

                memcpy(&szDataBuf[80], "0A", 2);
	}
	else if (memcmp(&pobTran->srBRec.szRespCode[0] , "00", 2) &&
	         memcmp(&pobTran->srBRec.szRespCode[0] , "11", 2) &&
		 memcmp(&pobTran->srBRec.szRespCode[0] , "\x00\x00", 2))
	{
	        if (memcmp(szTmeplate, _TRT_FILE_NAME_DCC_, strlen(_TRT_FILE_NAME_DCC_)) == 0)
	                memcpy(&szDataBuf[78], "04", 2);
	        else if (memcmp(szTmeplate, _TRT_FILE_NAME_CREDIT_, strlen(_TRT_FILE_NAME_CREDIT_)) == 0)
	                memcpy(&szDataBuf[78], "03", 2);
	        else if (memcmp(szTmeplate, _TRT_FILE_NAME_HG_, strlen(_TRT_FILE_NAME_HG_)) == 0)
	                memcpy(&szDataBuf[78], "13", 2);
	        /* 2018/5/8 下午 2:10 DFS需求不再使用大來主機 */

                memcpy(&szDataBuf[80], &pobTran->srBRec.szRespCode[0], 2);
        }
	else
		memcpy(&szDataBuf[78], "A000", 4);
	
//	if (srECROb->srTransData.inErrorType == VS_CALLBANK	||
//	    srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_CALLBANK_)
//		memcpy(&szDataBuf[78], "0002", 4);
//	else if (srECROb->srTransData.inErrorType == VS_TIMEOUT)
//		memcpy(&szDataBuf[78], "0003", 4);
//	else if (srECROb->srTransData.inErrorType == VS_USER_CANCEL)
//		memcpy(&szDataBuf[78], "0006", 4);
//	else if (srECROb->srTransData.inErrorType == VS_ISO_PACK_ERR || srECROb->srTransData.inErrorType == VS_ISO_UNPACK_ERROR ||
//		 srECROb->srTransData.inErrorType == VS_COMM_ERROR)
//		memcpy(&szDataBuf[78], "0007", 4);
//	else
//	{
//		memcpy(&szDataBuf[78], "0001", 4);
//	}
	
	return (VS_SUCCESS);
}

/*
Function        :inECR_LOAD_TMK_FROM_520_Standard_Unpack
Date&Time       :2017/11/16 下午 2:45
Describe        :
*/
int inECR_LOAD_TMK_FROM_520_Standard_Unpack(TRANSACTION_OBJECT *pobTran, ECR_TABLE* srECROb, char *szDataBuffer)
{
	int	inKeyMaxLen = 48;
	int	inRetVal = VS_SUCCESS;
	int	i = 0;
	int	inKeyCnt = 0;
	int	inKeyLen = 0;
	int	inPacketSizes = 0;
	char	szTemplate[50 + 1] = {0};
	
	/* key count */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(szTemplate, &szDataBuffer[inPacketSizes], 2);
	gsrTMKdata.uszGET_KCT[0] = atoi(szTemplate);
	inKeyCnt = gsrTMKdata.uszGET_KCT[0];
	inPacketSizes += 2;
	
	/* key data */
	for (i = 0; i < inKeyCnt; i++)
	{
		/* key Index */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(szTemplate, &szDataBuffer[inPacketSizes], 2);
		gsrTMKdata.uszGET_KID[i][0] = atoi(szTemplate);
		inPacketSizes += 2;
		
		/* key Len */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(szTemplate, &szDataBuffer[inPacketSizes], 2);
		inKeyLen = atoi(szTemplate);
		gsrTMKdata.uszGET_KLEN[i][0] = inKeyLen / 2;
		inPacketSizes += 2;
		
		/* key Data (就算只有32還是要補空白到48)*/
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(szTemplate, &szDataBuffer[inPacketSizes], inKeyMaxLen);
		memcpy(&gsrTMKdata.uszTMK_DES_Ascii[i][0], szTemplate, inKeyMaxLen);
		inPacketSizes += inKeyMaxLen;
	}
	
	return (inRetVal);
}

/*
Function        :inECR_LOAD_TMK_FROM_520_Standard_Pack
Date&Time       :2017/11/16 下午 2:46
Describe        :
*/
int inECR_LOAD_TMK_FROM_520_Standard_Pack(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer)
{
	int	inKeyMaxLen = 48;
	int	i = 0;
	int	inKeyCnt = 0;
	int	inKeyLen = 0;
	int	inPacketSizes = 0;
	char	szTemplate[50 + 1] = {0};
	
	inKeyCnt = gsrTMKdata.uszGET_KCT[0];
	
	/* key count */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%02d", inKeyCnt);
	memcpy(&szDataBuffer[inPacketSizes], szTemplate, 2);
	inPacketSizes += 2;
	
	/* key data */
	for (i = 0; i < inKeyCnt; i++)
	{
		/* key Index */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%02d", gsrTMKdata.uszGET_KID[i][0]);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 2);
		inPacketSizes += 2;
		
		/* key Len */
		inKeyLen = gsrTMKdata.uszGET_KLEN[i][0] * 2;
		
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%02d", inKeyLen);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 2);
		inPacketSizes += 2;
		
		
		/* key Data (就算只有32還是要補空白到48)*/
		memset(szTemplate, 0x00, sizeof(szTemplate));
		
		/* 全補滿空白再放key值 */
		memset(szTemplate, ' ', inKeyMaxLen);
		memcpy(szTemplate, &gsrTMKdata.uszTMK_DES_Ascii[i][0], inKeyLen);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, inKeyMaxLen);
		inPacketSizes += inKeyMaxLen;
	}
	
	
	return (VS_SUCCESS);
}


/*
Function        :inECR_8N1_TSB_KIOSK_Unpack
Date&Time       :2017/11/15 下午 2:44
Describe        :分析收銀機傳來的資料
*/
int inECR_8N1_TSB_KIOSK_Unpack(TRANSACTION_OBJECT *pobTran, ECR_TABLE* srECROb, char *szDataBuffer)
{
	int	i = 0;
	int	inRetVal = VS_SUCCESS;
	int	inTransType;		/* 電文中的交易別字串轉為數字儲存 */
	char	szTemplate[100 + 1];
	char	szDebugMsg[100 + 1];	
	char	szHostLabel[8 + 1];
	char	szHostEnable[2 + 1];
	char	szBatchNum[6 + 1];
	char	szCTLSEnable[2 + 1];
	char	szFiscContactlessEnable[1 + 1] = {0};
	char	szCUPContactlessEnable[1 + 1] = {0};
	
	/* 存ECR原始資料 */
	memcpy(srECROb->srTransData.szOrgData, szDataBuffer, _ECR_8N1_Standard_Data_Size_);
	
	/* Trans Type (交易別) */
	switch (inTransType)
	{
		default:
			memset(srECROb->srTransData.szTransType, 0x00, sizeof(srECROb->srTransData.szTransType));
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &szDataBuffer[0], 2);
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s: %s  ", "Trans Type", szTemplate);
				inLogPrintf(AT, szDebugMsg);
			}
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Type", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				if (ginDisplayDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Type", "Not Exist Error");
					inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_3_, VS_TRUE);
				}
				
				return (VS_ERROR);
			}
			else
			{
				memcpy(srECROb->srTransData.szTransType, szTemplate, 2);
			}
			
			/* 交易類別轉成數字 */
			inTransType = atoi(srECROb->srTransData.szTransType);
			
			/* 要連動結帳 */
			if (inTransType == _ECR_8N1_TSB_KIOSK_SETTLEMENT_NO_)
			{
				pobTran->uszAutoSettleBit = VS_TRUE;
			}
			
			break;
	}
		
	/* 銀行別/付款別 HOST ID*/
	switch (inTransType)
	{
		case _ECR_8N1_TSB_KIOSK_VOID_NO_:			/* 取消交易 */
		case _ECR_8N1_TSB_KIOSK_REPRINT_RECEIPT_NO_:		/* 重印簽單 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &szDataBuffer[2], 2);
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "HOST ID", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				if (ginDisplayDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "HOST ID", "Not Exist Error");
					inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_3_, VS_TRUE);
				}
				
				return (VS_ERROR);
			}
			
			memset(szHostLabel, 0x00, sizeof(szHostLabel));

			if (memcmp(szTemplate, _ECR_8N1_TSB_KIOSK_HOSTID_TSB_, 2) == 0)
			{
				memcpy(szHostLabel, _HOST_NAME_CREDIT_NCCC_, strlen(_HOST_NAME_CREDIT_NCCC_));
			}
			else if (memcmp(szTemplate, _ECR_8N1_TSB_KIOSK_HOSTID_INST_, 2) == 0)
			{
				memcpy(szHostLabel, _HOST_NAME_CREDIT_NCCC_, strlen(_HOST_NAME_CREDIT_NCCC_));
			}
			else if (memcmp(szTemplate, _ECR_8N1_TSB_KIOSK_HOSTID_REDEEM_, 2) == 0)
			{
				memcpy(szHostLabel, _HOST_NAME_CREDIT_NCCC_, strlen(_HOST_NAME_CREDIT_NCCC_));
			}
			else if (memcmp(szTemplate, _ECR_8N1_TSB_KIOSK_HOSTID_SMARTPAY_, 2) == 0)
			{
				memcpy(szHostLabel, _HOST_NAME_CREDIT_NCCC_, strlen(_HOST_NAME_CREDIT_NCCC_));
			}
			else if (memcmp(szTemplate, _ECR_8N1_TSB_KIOSK_HOSTID_AMEX_, 2) == 0)
			{
				memcpy(szHostLabel, _HOST_NAME_CREDIT_NCCC_, strlen(_HOST_NAME_CREDIT_NCCC_));
			}
			else if (memcmp(szTemplate, _ECR_8N1_TSB_KIOSK_HOSTID_DCC_, 2) == 0)
			{
				memcpy(szHostLabel, _HOST_NAME_DCC_, strlen(_HOST_NAME_DCC_));
			}
			else if (memcmp(szTemplate, _ECR_8N1_TSB_KIOSK_HOSTID_DINERS_, 2) == 0)
			{
				memcpy(szHostLabel, _HOST_NAME_DINERS_, strlen(_HOST_NAME_DINERS_));
			}
			else
			{
				return (VS_ERROR);
			}
			
			/* 初始化 */
			i = 0;
			do
			{
				/* 按順序load每一個HOST */
				if (inLoadHDTRec(i) < 0)
				{
					/* 如果return VS_ERROR 代表table沒有該HOST */
					return (VS_ERROR);
				}

				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetHostLabel(szTemplate);
				if (!memcmp(szTemplate, szHostLabel, strlen(szHostLabel)))
				{
					/* 比對成功後，判斷HOST是否開啟  */
					memset(szHostEnable, 0x00, sizeof(szHostEnable));
					inGetHostEnable(szHostEnable);
					if (szHostEnable[0] != 'Y')
					{
						/* Host沒開 */
						if (ginDebug == VS_TRUE) 
						{
							memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
							sprintf(szDebugMsg, "%s Not Open Error", szHostLabel);
							inLogPrintf(AT, szDebugMsg);
						}
						if (ginDisplayDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
							sprintf(szDebugMsg, "%s Not Open Error", szHostLabel);
							inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_3_, VS_TRUE);
						}

						/* 如果return VS_ERROR 代表Host沒開 */
						return (VS_ERROR);
					}
					else
					{
						/* Host有開 */
						pobTran->srBRec.inHDTIndex = i;
						
						/* Load HDPT */
						inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
						memset(szBatchNum, 0x00, sizeof(szBatchNum));
						inGetBatchNum(szBatchNum);
						pobTran->srBRec.lnBatchNum = atol(szBatchNum);
						
						if (ginDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
							sprintf(szDebugMsg, "%s Open", szHostLabel);
							inLogPrintf(AT, szDebugMsg);
						}
						
					}/* Host Enable比對End */

					break;
				}/* Host Label比對 End */
				i++;

			} while (1);
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%d", "HDTindex", pobTran->srBRec.inHDTIndex);
				inLogPrintf(AT, szDebugMsg);
			}
			if (ginDisplayDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%d", "HDTindex", pobTran->srBRec.inHDTIndex);
				inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_2_, VS_FALSE);
			}
			
			break;
			
		default:
			break;
	}
	
	/* EDC簽單序號 Receipt No(InvoiceNumber) */
	switch (inTransType)
	{
		case _ECR_8N1_TSB_KIOSK_VOID_NO_:			/* 取消交易 */
		case _ECR_8N1_TSB_KIOSK_REPRINT_RECEIPT_NO_:		/* 重印簽單 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &szDataBuffer[4], 6);
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Receipt No", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				if (ginDisplayDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Receipt No", "Not Exist Error");
					inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_3_, VS_TRUE);
				}
				
				return (VS_ERROR);
			}
			else
			{
				pobTran->srBRec.lnOrgInvNum = atol(szTemplate);
			}
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%ld", "lnOrgInvNum", pobTran->srBRec.lnOrgInvNum);
				inLogPrintf(AT, szDebugMsg);
			}
			if (ginDisplayDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%ld", "lnOrgInvNum", pobTran->srBRec.lnOrgInvNum);
				inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_2_, VS_FALSE);
			}
			
			break;
		default:
			break;
	}
	
	/* 處理交易金額 (Trans Amount) */
	switch (inTransType)
	{
		case _ECR_8N1_TSB_KIOSK_SALE_MULTI_NO_:			/* 一般交易 */
		case _ECR_8N1_TSB_KIOSK_SALE_SINGLE_NO_:		/* 一般交易 */
		case _ECR_8N1_TSB_KIOSK_REFUND_NO_:			/* 退貨交易 */
		case _ECR_8N1_TSB_KIOSK_OFFLINE_NO_:			/* 交易補登 */
		case _ECR_8N1_TSB_KIOSK_INSTALLMENT_NO_:		/* 分期 */
		case _ECR_8N1_TSB_KIOSK_REDEEM_NO_:			/* 紅利 */		
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &szDataBuffer[33], 10);
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Amount", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				if (ginDisplayDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Amount", "Not Exist Error");
					inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_3_, VS_TRUE);
				}
				
				return (VS_ERROR);
			}
			else
			{
				pobTran->srBRec.lnTxnAmount = atol(szTemplate);
				pobTran->srBRec.lnOrgTxnAmount = atol(szTemplate);
				pobTran->srBRec.lnTotalTxnAmount = atol(szTemplate);
			}

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%ld", "交易金額", pobTran->srBRec.lnTxnAmount);
				inLogPrintf(AT, szDebugMsg);
			}
			if (ginDisplayDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%ld", "交易金額", pobTran->srBRec.lnTxnAmount);
				inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_2_, VS_FALSE);
			}
			
			break;
		default:
			break;
	}
	
	/* 授權碼(Auth Code) ECR電文中給9個字元 但ATS電文只需要6個，所以只抓6個 */
	switch (inTransType)
	{
		case _ECR_8N1_TSB_KIOSK_REFUND_NO_:			/* 退貨 */
		case _ECR_8N1_TSB_KIOSK_OFFLINE_NO_:			/* 交易補登 */
			/* 處理授權碼(optional) */
			memset(pobTran->srBRec.szAuthCode, 0x00, sizeof(pobTran->srBRec.szAuthCode));
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &szDataBuffer[57], 9);
			memcpy(pobTran->srBRec.szAuthCode, szTemplate, 6);
			break;
		default :
			break;
	}
	
	/* 處理櫃號(Store ID) */
	switch (inTransType)
	{
		case _ECR_8N1_TSB_KIOSK_SALE_MULTI_NO_:			/* 一般交易 */
		case _ECR_8N1_TSB_KIOSK_SALE_SINGLE_NO_:		/* 一般交易 */
		case _ECR_8N1_TSB_KIOSK_REFUND_NO_:			/* 退貨交易 */
		case _ECR_8N1_TSB_KIOSK_OFFLINE_NO_:			/* 交易補登 */
		case _ECR_8N1_TSB_KIOSK_INSTALLMENT_NO_:			/* 分期 */
		case _ECR_8N1_TSB_KIOSK_REDEEM_NO_:			/* 紅利 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &szDataBuffer[114], 18);
			
			if (strlen(szTemplate) > 0)
			{
				memcpy(pobTran->srBRec.szStoreID, szTemplate, strlen(szTemplate));
				/* 櫃號不滿18，補空白 */
				if (strlen(szTemplate) < 18)
				{
					memset(&pobTran->srBRec.szStoreID[strlen(szTemplate)], 0x20, 18 - strlen(szTemplate));
				}

			}
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "Store ID", pobTran->srBRec.szStoreID);
				inLogPrintf(AT, szDebugMsg);
			}
			if (ginDisplayDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "Store ID", pobTran->srBRec.szStoreID);
				inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_2_, VS_FALSE);
			}
			
			break;
		default:
			break;
	}
	
	/* 分期期數 Installment Period */
	switch (inTransType)
	{
		case _ECR_8N1_TSB_KIOSK_INSTALLMENT_NO_:			/* 分期 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &szDataBuffer[132], 2);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "Installment Period", szTemplate);
				inLogPrintf(AT, szDebugMsg);
			}
			if (ginDisplayDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "Installment Period", szTemplate);
				inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_2_, VS_FALSE);
			}

			pobTran->srBRec.lnInstallmentPeriod = atol(szTemplate);

			break;
		default:
			break;
	}
	
	/* CUP/Smart pay Indicator:銀聯卡 / Smart Pay交易使用
	 * Indicator = 'C'，表示為CUP交易
	 * Indicator = 'N'，表示為一般信用卡交易
	 */
	switch (inTransType)
	{
		case _ECR_8N1_TSB_KIOSK_SALE_MULTI_NO_:			/* 一般交易 */
		case _ECR_8N1_TSB_KIOSK_SALE_SINGLE_NO_:		/* 一般交易 */
		case _ECR_8N1_TSB_KIOSK_REFUND_NO_:			/* 退貨交易 */
		case _ECR_8N1_TSB_KIOSK_VOID_NO_:			/* 取消 */
			memset(srECROb->srTransData.szField_05, 0x00, sizeof(srECROb->srTransData.szField_05));
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &szDataBuffer[136], 1);
		
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s: %s  ", "CUPIndicator", szTemplate);
				inLogPrintf(AT, szDebugMsg);
			}
			if (ginDisplayDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s: %s  ", "CUPIndicator", szTemplate);
				inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_2_, VS_FALSE);
			}
			
			memcpy(srECROb->srTransData.szField_05, szTemplate, 1);
			
			/* 如果是'C'代表是銀聯卡交易 */
			if (memcmp(&srECROb->srTransData.szField_05[0], "C", 1) == 0)
			{
				pobTran->srBRec.uszCUPTransBit = VS_TRUE;
			}
			else if (memcmp(&srECROb->srTransData.szField_05[0], "N", 1) == 0)
			{
				
			}
			else if (memcmp(&srECROb->srTransData.szField_05[0], "E", 1) == 0)
			{
				pobTran->srTRec.uszESVCTransBit = VS_TRUE;
			}
			else
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s", "CUP Indicator Error");
					inLogPrintf(AT, szDebugMsg);
				}
				if (ginDisplayDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s", "CUP Indicator Error");
					inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_3_, VS_TRUE);
				}
				return (VS_ERROR);
			}
			
			break;
		default:
			break;
	}
	
	memset(szFiscContactlessEnable, 0x00, sizeof(szFiscContactlessEnable));
	inGetSmartPayContactlessEnable(szFiscContactlessEnable);
	memset(szCTLSEnable, 0x00, sizeof(szCTLSEnable));
	inGetContactlessEnable(szCTLSEnable);
	memset(szCUPContactlessEnable, 0x00, sizeof(szCUPContactlessEnable));
	inGetCUPContactlessEnable(szCUPContactlessEnable);
	
	/* 跑OPT */
	switch (inTransType)
	{
		case _ECR_8N1_TSB_KIOSK_SALE_MULTI_NO_:
		case _ECR_8N1_TSB_KIOSK_SALE_SINGLE_NO_:		/* 一般交易 */
			/* 金融卡和一般Sale跑不同OPT，所以要分 */
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{	
				inDISP_ClearAll();
				/* 第三層顯示 ＜消費扣款＞ */
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_FISC_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜消費扣款＞ */

				pobTran->inFunctionID = _FISC_SALE_;
				if (!memcmp(szCTLSEnable, "Y", 1)		&&
				    !memcmp(szFiscContactlessEnable, "Y", 1)	&&
				    guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_FISC_SALE_CTLS_;
					pobTran->inRunTRTID = _TRT_FISC_SALE_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_FISC_SALE_;
					pobTran->inRunTRTID = _TRT_FISC_SALE_ICC_;
				}
		
				pobTran->inTransactionCode = _FISC_SALE_;
				pobTran->srBRec.inCode = _FISC_SALE_;
				pobTran->srBRec.inOrgCode = _FISC_SALE_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_CUP_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜銀聯一般交易＞ */

				if (!memcmp(szCTLSEnable, "Y", 1)		&&
				    !memcmp(szCUPContactlessEnable, "Y", 1)	&&
				    guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
					pobTran->inRunTRTID = _TRT_CUP_SALE_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
					pobTran->inRunTRTID = _TRT_CUP_SALE_;
				}

				pobTran->inFunctionID = _CUP_SALE_;
				pobTran->inTransactionCode = _CUP_SALE_;
				pobTran->srBRec.inCode = pobTran->inTransactionCode;
				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_TICKET_DEDUCT_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜購貨交易＞ */

				pobTran->inFunctionID = _TICKET_DEDUCT_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_TICKET_DEDUCT_;
				}
				else
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "感應未初始化，不能使用此功能");
					}
					return (VS_ERROR);
				}

				pobTran->inTransactionCode = _TICKET_DEDUCT_;
				pobTran->srTRec.inCode = _TICKET_DEDUCT_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else
			{
				inDISP_ClearAll();
				/* 第三層顯示 ＜一般交易＞ */
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般交易＞ */

				pobTran->inFunctionID = _SALE_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
					pobTran->inRunTRTID = _TRT_SALE_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
					pobTran->inRunTRTID = _TRT_SALE_;
				}

				pobTran->inTransactionCode = _SALE_;
				pobTran->srBRec.inCode = _SALE_;
				pobTran->srBRec.inOrgCode = _SALE_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			
			break;
		case _ECR_8N1_TSB_KIOSK_VOID_NO_:
			/* 金融卡和一般Sale跑不同OPT，所以要分 */
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_FISC_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜沖正交易＞ */

				pobTran->inFunctionID = _FISC_VOID_;
				pobTran->inRunOperationID = _OPERATION_VOID_;
				pobTran->inRunTRTID = _TRT_FISC_VOID_;

				pobTran->inTransactionCode = _FISC_VOID_;
				pobTran->srBRec.inCode = _FISC_VOID_;
				pobTran->srBRec.inOrgCode = _FISC_VOID_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜取消交易＞ */

				pobTran->inRunOperationID = _OPERATION_VOID_;
				pobTran->inRunTRTID = _TRT_CUP_VOID_;

				pobTran->inFunctionID = _CUP_VOID_;
				pobTran->inTransactionCode = _CUP_VOID_;
				pobTran->srBRec.inCode = pobTran->inTransactionCode;
				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜取消交易＞ */

				pobTran->inFunctionID = _VOID_;
				pobTran->inRunOperationID = _OPERATION_VOID_;
				pobTran->inRunTRTID = _TRT_VOID_;

				pobTran->inTransactionCode = _VOID_;
				pobTran->srBRec.inCode = _VOID_;
				pobTran->srBRec.inOrgCode = _VOID_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			break;
		case _ECR_8N1_TSB_KIOSK_REFUND_NO_:
			/* 金融卡和一般Sale跑不同OPT，所以要分 */
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_FISC_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);	/* 第二層顯示 ＜退費交易＞ */

				pobTran->inFunctionID = _FISC_REFUND_;                     
				if (!memcmp(szCTLSEnable, "Y", 1)		&&
				    !memcmp(szFiscContactlessEnable, "Y", 1)	&&
				    guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_FISC_REFUND_CTLS_;
					pobTran->inRunTRTID = _TRT_FISC_REFUND_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_FISC_REFUND_;
					pobTran->inRunTRTID = _TRT_FISC_REFUND_ICC_;
				}

				pobTran->inTransactionCode = _FISC_REFUND_;
				pobTran->srBRec.inCode = _FISC_REFUND_;
				pobTran->srBRec.inOrgCode = _FISC_REFUND_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			else if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般退貨＞ */

				if (!memcmp(szCTLSEnable, "Y", 1)		&&
				    !memcmp(szCUPContactlessEnable, "Y", 1)	&&
				    guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_CUP_;
					pobTran->inRunTRTID = _TRT_CUP_REFUND_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_AMOUNT_FIRST_CUP_;
					pobTran->inRunTRTID = _TRT_CUP_REFUND_;
				}

				pobTran->inFunctionID = _CUP_REFUND_;
				pobTran->inTransactionCode = _CUP_REFUND_;
				pobTran->srBRec.inCode = pobTran->inTransactionCode ;
				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode ;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般退貨＞ */

				pobTran->inFunctionID = _REFUND_;

				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_AMOUNT_FIRST_;
				}
				pobTran->inRunTRTID = _TRT_REFUND_;

				pobTran->inTransactionCode = _REFUND_;
				pobTran->srBRec.inCode = _REFUND_;
				pobTran->srBRec.inOrgCode = _REFUND_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;
			
		case _ECR_8N1_TSB_KIOSK_OFFLINE_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_SALE_OFFLINE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜交易補登＞ */
			
			pobTran->inFunctionID = _SALE_OFFLINE_;
			pobTran->inRunOperationID = _OPERATION_SALE_OFFLINE_;
			pobTran->inRunTRTID = _TRT_SALE_OFFLINE_;

			pobTran->inTransactionCode = _SALE_OFFLINE_;
			pobTran->srBRec.inCode = _SALE_OFFLINE_;
			pobTran->srBRec.inOrgCode = _SALE_OFFLINE_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			break;
		case _ECR_8N1_TSB_KIOSK_INSTALLMENT_NO_:
//			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
//			{
//				inDISP_ClearAll();
//				/* 保留給銀聯分期使用 */
//				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_8X16_1_);		/* 第一層顯示 LOGO */
//				inDISP_PutGraphic(_MENU_INST_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8X16_3_);	/* 第三層顯示 ＜分期付款＞ */
//
//				pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
//				pobTran->inRunTRTID = _TRT_INST_SALE_;
//				
//				pobTran->inFunctionID = _CUP_INST_SALE_;
//				pobTran->inTransactionCode = _CUP_INST_SALE_;
//				pobTran->srBRec.inCode = pobTran->inTransactionCode;
//				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
//
//				pobTran->srBRec.uszInstallmentBit = VS_TRUE;
//
//				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
//			}
//			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_INST_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜分期付款＞ */

				pobTran->inFunctionID = _INST_SALE_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_INST_SALE_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
				}
				pobTran->inRunTRTID = _TRT_INST_SALE_;

				pobTran->inTransactionCode = _INST_SALE_;
				pobTran->srBRec.inCode = _INST_SALE_;
				pobTran->srBRec.inOrgCode = _INST_SALE_;

				pobTran->srBRec.uszInstallmentBit = VS_TRUE;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;
		case _ECR_8N1_TSB_KIOSK_REDEEM_NO_:
//			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
//			{
//				inDISP_ClearAll();
//				/* 保留給銀聯紅利扣抵使用 */
//				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
//				inDISP_PutGraphic(_MENU_REDEEM_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利扣抵＞ */
//
//				pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
//				pobTran->inRunTRTID = _TRT_REDEEM_SALE_;
//
//				pobTran->inFunctionID = _CUP_REDEEM_SALE_;
//				pobTran->inTransactionCode = _CUP_REDEEM_SALE_;
//				pobTran->srBRec.inCode = pobTran->inTransactionCode;
//				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
//
//				pobTran->srBRec.uszRedeemBit = VS_TRUE;
//
//				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
//			}
//			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REDEEM_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利扣抵＞ */

				pobTran->inFunctionID = _REDEEM_SALE_;
				pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
				pobTran->inRunTRTID = _TRT_REDEEM_SALE_;

				pobTran->inTransactionCode = _REDEEM_SALE_;
				pobTran->srBRec.inCode = _REDEEM_SALE_;
				pobTran->srBRec.inOrgCode = _REDEEM_SALE_;

				pobTran->srBRec.uszRedeemBit = VS_TRUE;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;
		case _ECR_8N1_TSB_KIOSK_SETTLEMENT_NO_:
			inDISP_ClearAll();
			/* 第三層顯示 ＜結帳交易＞ */
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_SETTLE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜結帳交易＞ */

			pobTran->inFunctionID = _SETTLE_;
			pobTran->inRunOperationID = _OPERATION_SETTLE_;
			pobTran->inRunTRTID = _TRT_SETTLE_;
			
			pobTran->inTransactionCode = _SETTLE_;
			pobTran->srBRec.inCode = _SETTLE_;
			pobTran->srBRec.inOrgCode = _SETTLE_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		case _ECR_8N1_TSB_KIOSK_REPRINT_RECEIPT_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_REPRINT_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜重印帳單＞ */

			pobTran->inFunctionID = FALSE;
			pobTran->inRunOperationID = _OPERATION_REPRINT_;
			pobTran->inRunTRTID = FALSE;
			
			pobTran->inTransactionCode = FALSE;
			pobTran->srBRec.inCode = FALSE;
			pobTran->srBRec.inOrgCode = FALSE;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		default:
			return (VS_ERROR);
			break;
	}
	
	return (inRetVal);
}

/*
Function        :inRS232_ECR_8N1_TSB_KIOSK_Pack
Date&Time       :2016/7/6 下午 2:40
Describe        :先把要送的資料組好
*/
int inECR_8N1_TSB_KIOSK_Pack(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer)
{
	int		inRetVal = VS_ERROR;
	int		i = 0, inCardLen = 0;
	int		inPacketSizes = 0;
	char		szTemplate[100 + 1];
	ACCUM_TOTAL_REC	srAccum = {0};
	
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_SETTLEMENT_, 2))
	{
		inRetVal = inACCUM_GetRecord(pobTran, &srAccum);
		if (inRetVal == VS_SUCCESS)
		{
			
		}
		else if (inRetVal == VS_NO_RECORD)
		{
			
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "無帳務");
			}
			return (VS_ERROR);
		}
	}

	/* 1.Trans Type (2 Byte)(0~1) */
	memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szTransType, 2);
	inPacketSizes += 2;
	
	
	/* 2.HOST ID (2 Byte)(2~3) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_SALE_MULTI_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_SALE_SINGLE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_REFUND_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_OFFLINE_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_INSTALLMENT_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_REDEEM_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_VOID_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_SETTLEMENT_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_INQUIRE_TRANSACTION_STAUS_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_REPRINT_RECEIPT_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_SALE_FALLBACK_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_INSTALLMENT_FALLBACK_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_REDEEM_FALLBACK_, 2))
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTRTFileName(szTemplate);
		
		if (!memcmp(szTemplate, _TRT_FILE_NAME_CREDIT_, strlen(_TRT_FILE_NAME_CREDIT_)))
			memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_TSB_KIOSK_HOSTID_TSB_, 2);
		else if (!memcmp(szTemplate, _TRT_FILE_NAME_DINERS_, strlen(_TRT_FILE_NAME_DINERS_)))
			memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_TSB_KIOSK_HOSTID_DINERS_, 2);
		else if (!memcmp(szTemplate, _TRT_FILE_NAME_DCC_, strlen(_TRT_FILE_NAME_DCC_)))
			memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_TSB_KIOSK_HOSTID_DCC_, 2);
	}
	inPacketSizes += 2;

	
	/* 3.Receipt No (6 Byte)(4~9) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_SALE_MULTI_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_SALE_SINGLE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_REFUND_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_OFFLINE_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_INSTALLMENT_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_REDEEM_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_VOID_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_INQUIRE_TRANSACTION_STAUS_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_REPRINT_RECEIPT_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_SALE_FALLBACK_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_INSTALLMENT_FALLBACK_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_REDEEM_FALLBACK_, 2))
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%06ld", pobTran->srBRec.lnOrgInvNum);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 6);
	}
	inPacketSizes += 6;

	
	/* 4.Card No (19 Byte)(10~28) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_SETTLEMENT_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_REPRINT_RECEIPT_, 2))
	{
		/* 優惠兌換先設定不回傳卡號 */
		inPacketSizes += 19;
	}
	else
	{
		/* 卡號是否遮掩 */
		inRetVal = inECR_CardNoTruncateDecision(pobTran);
		/* 要遮卡號 */
		if (inRetVal == VS_SUCCESS)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			strcpy(szTemplate, pobTran->srBRec.szPAN);

			/* HAPPG_GO 卡不掩飾 */
			if (!memcmp(&pobTran->srBRec.szPAN[0], "9552", 4))
			{

			}
			else
			{
				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
				/* 卡號長度 */
				inCardLen = strlen(szTemplate);

				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
				for (i = 6; i < (inCardLen - 4); i ++)
					szTemplate[i] = '*';
			}

			memcpy(&szDataBuffer[inPacketSizes], szTemplate, strlen(szTemplate));
		}
		else
		{
			memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szPAN, strlen(pobTran->srBRec.szPAN));
		}
		inPacketSizes += 19;
	}
	
	
	/* 5.Reserve(29~32) */
	inPacketSizes += 4;
	
	
	/* 6.Trans Amount (12 Byte)(33~44) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_VOID_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_SETTLEMENT_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_REPRINT_RECEIPT_, 2))
	{
	        inPacketSizes += 12;
	}
	else if (pobTran->srBRec.lnTxnAmount != 0)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010lu00", pobTran->srBRec.lnTxnAmount);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
			
		inPacketSizes += 12;
	}
	else
	{
		inPacketSizes += 12;
	}

	
	/* 7.Trans Date (6 Byte) & 8.Trans Time (6 Byte)(45~56) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_SALE_MULTI_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_SALE_SINGLE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_REFUND_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_OFFLINE_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_INSTALLMENT_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_REDEEM_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_VOID_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_INQUIRE_TRANSACTION_STAUS_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_REPRINT_RECEIPT_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_SALE_FALLBACK_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_INSTALLMENT_FALLBACK_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_REDEEM_FALLBACK_, 2))
	{
		/* Trans Date */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		strcat(szTemplate, pobTran->srBRec.szDate);
		memcpy(&szDataBuffer[inPacketSizes], &szTemplate[2], 6);
		inPacketSizes += 6;
		
		/* Trans Time */
		memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szTime, 6);
		inPacketSizes += 6;
	}
	else
	{
		inPacketSizes += 6;
		inPacketSizes += 6;
	}

	
	/* 9.Approval No (9 Byte)(57~65) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_SALE_MULTI_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_SALE_SINGLE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_REFUND_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_OFFLINE_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_INSTALLMENT_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_REDEEM_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_VOID_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_INQUIRE_TRANSACTION_STAUS_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_REPRINT_RECEIPT_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_SALE_FALLBACK_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_INSTALLMENT_FALLBACK_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_REDEEM_FALLBACK_, 2))
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		strcpy(szTemplate, pobTran->srBRec.szAuthCode);
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 9, _PADDING_RIGHT_);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 9);
	}
	inPacketSizes += 9;

	
	/* 10.分期付款總額/紅利抵用點數 (12 Bytes)(66~77) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_INSTALLMENT_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_INSTALLMENT_FALLBACK_, 2))
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010lu00", pobTran->srBRec.lnTxnAmount);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
	}
	else if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_REDEEM_, 2)		||
		 !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_REDEEM_FALLBACK_, 2))
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%012lu", pobTran->srBRec.lnRedemptionPoints);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
	}
	inPacketSizes += 12;
	
	
	/* 11.ECR Response Code (4 Byte)(78~81) */
	memcpy(&szDataBuffer[inPacketSizes], "0000", 4);
	inPacketSizes += 4;

	
	/* 12.Terminal ID (8 Byte)(82~89) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTerminalID(szTemplate);
	inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 8, _PADDING_RIGHT_);
	memcpy(&szDataBuffer[inPacketSizes], szTemplate, 8);
	inPacketSizes += 8;

	
	/* 13.分期付款首期金額/紅利抵用後實際付金額/銷售總金額(12 Byte)(90~ 101) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_INSTALLMENT_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_INSTALLMENT_FALLBACK_, 2))
	{
		/* 首期金額 Down Payment (12 Byte) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010lu00", pobTran->srBRec.lnInstallmentDownPayment);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
	}
	else if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_REDEEM_, 2)		||
		 !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_REDEEM_FALLBACK_, 2))
	{
		/* 支付金額 RDM Paid Amt (12 Byte) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010lu00", (pobTran->srBRec.lnRedemptionPaidCreditAmount + pobTran->srBRec.lnTipTxnAmount));
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
	}
	else if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_SETTLEMENT_, 2))
	{
		/* 銷售總金額 (12 Byte) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010llu00", srAccum.llTotalSaleAmount);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
	}
	inPacketSizes += 12;
	
	
	/* 14.分期付款每期金額/紅利抵用金額/退貨總金額(12 Byte)(102~ 113) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_INSTALLMENT_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_INSTALLMENT_FALLBACK_, 2))
	{
		/* 每期金額 Installment Payment Amt (12 Byte) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010lu00", pobTran->srBRec.lnInstallmentPayment);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
	}
	else if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_REDEEM_, 2)		||
		 !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_REDEEM_FALLBACK_, 2))
	{
		/* 紅利扣抵金額 Redeem Amt (12 Byte) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010lu00", (pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnRedemptionPaidCreditAmount));
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
	}
	else if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_SETTLEMENT_, 2))
	{
		/* 銷售總金額 (12 Byte) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010llu00", srAccum.llTotalRefundAmount);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
	}
	inPacketSizes += 12;
	
	
	/* 15.RRN OR Order NO/銷貨筆數、退貨筆數、剩12碼固定為0 (18 Byte)(114~ 131) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_SETTLEMENT_, 2))
	{
		/* 銷貨筆數、退貨筆數 (18 Byte) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%03lu%03lu000000000000", srAccum.lnTotalSaleCount, srAccum.lnTotalRefundCount);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 18);
	}
	else
	{
		/* RRN (1 Byte) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
		{
			sprintf(szTemplate, "%s", pobTran->srBRec.szFiscRRN);
			if (strlen(pobTran->srBRec.szFiscRRN) < 18)
			{
				memset(&szTemplate[strlen(pobTran->srBRec.szFiscRRN)], ' ', 18 - strlen(pobTran->srBRec.szFiscRRN));
			}
		}
		else
		{
			sprintf(szTemplate, "%s", pobTran->srBRec.szRefNo);
			if (strlen(pobTran->srBRec.szRefNo) < 18)
			{
				memset(&szTemplate[strlen(pobTran->srBRec.szRefNo)], ' ', 18 - strlen(pobTran->srBRec.szRefNo));
			}
		}
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 18);
	}
	inPacketSizes += 18;

	
	/* 16.分期期數(2 Bytes)(132~ 133) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_INSTALLMENT_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_INSTALLMENT_FALLBACK_, 2))
	{
		/* 分期期數 Installment Period (2 Byte) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%02ld", pobTran->srBRec.lnInstallmentPeriod);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 2);
	}
	inPacketSizes += 2;

	
	/* 17.Card Type (2 Byte)(134~ 135) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_SALE_MULTI_, 2)						||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_SALE_SINGLE_, 2)						||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_REFUND_, 2)							||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_OFFLINE_, 2)							||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_INSTALLMENT_, 2)						||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_REDEEM_, 2)							||
	    (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_VOID_, 2) && pobTran->srBRec.uszHappyGoSingle != VS_TRUE)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_INQUIRE_TRANSACTION_STAUS_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_REPRINT_RECEIPT_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_SALE_FALLBACK_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_INSTALLMENT_FALLBACK_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_REDEEM_FALLBACK_, 2))
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		strcpy(szTemplate, pobTran->srBRec.szCardLabel);
		
		/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
		if (!memcmp(szTemplate, _CARD_TYPE_VISA_, strlen(_CARD_TYPE_VISA_)))
			memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_TSB_KIOSK_NCCC_CARDTYPE_VISA_, 2);
		else if (!memcmp(szTemplate, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)))
			memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_TSB_KIOSK_NCCC_CARDTYPE_MASTERCARD_, 2);
		else if (!memcmp(szTemplate, _CARD_TYPE_JCB_, strlen(_CARD_TYPE_JCB_)))
			memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_TSB_KIOSK_NCCC_CARDTYPE_JCB_, 2);
		else if (!memcmp(szTemplate, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)))
			memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_TSB_KIOSK_NCCC_CARDTYPE_AMEX_, 2);
		else if (!memcmp(szTemplate, _CARD_TYPE_CUP_, strlen(_CARD_TYPE_CUP_)))
			memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_TSB_KIOSK_NCCC_CARDTYPE_CUP_, 2);
		else if (!memcmp(szTemplate, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)) ||
			 pobTran->srBRec.uszUCARDTransBit == VS_TRUE)
		{
			if (pobTran->srBRec.uszUCARDTransBit == VS_TRUE)
			{
				memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_TSB_KIOSK_NCCC_CARDTYPE_UCARD_, 2);
			}
			else
			{
				memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_TSB_KIOSK_NCCC_CARDTYPE_DINERS_, 2);
			}
		}
		else if (!memcmp(szTemplate, _CARD_TYPE_SMARTPAY_, strlen(_CARD_TYPE_SMARTPAY_)))
			memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_TSB_KIOSK_NCCC_CARDTYPE_SMARTPAY_, 2);
		else
			memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_CARDTYPE_UNKNOWN_, _ECR_8N1_NCCC_CARDTYPE_UNKNOWN_LEN_);
	}
	inPacketSizes += 2;

	
	/* 18.CUP Indicator (1 Byte)(136~136) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_SALE_MULTI_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_SALE_SINGLE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_REFUND_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_OFFLINE_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_INSTALLMENT_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_REDEEM_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_VOID_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_INQUIRE_TRANSACTION_STAUS_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_REPRINT_RECEIPT_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_SALE_FALLBACK_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_INSTALLMENT_FALLBACK_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_REDEEM_FALLBACK_, 2))
	{
		if (!memcmp(srECROb->srTransData.szField_05, "C", 1) || pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			memcpy(&szDataBuffer[inPacketSizes], "C", 1);
		else
		        memcpy(&szDataBuffer[inPacketSizes], "N", 1);
	}
	inPacketSizes ++;
	
	
	/* 19.Reserve(137~ 143) */
	inPacketSizes += 7;
	
	/* 20.EntryMode (1 Byte)(144~ 144) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_SALE_MULTI_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_SALE_SINGLE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_REFUND_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_OFFLINE_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_INSTALLMENT_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_REDEEM_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_VOID_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_INQUIRE_TRANSACTION_STAUS_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_REPRINT_RECEIPT_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_SALE_FALLBACK_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_INSTALLMENT_FALLBACK_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_REDEEM_FALLBACK_, 2))
	{
		/* 過卡方式 */
		if (pobTran->srBRec.uszFiscTransBit != VS_TRUE)
		{
			if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
				memcpy(&szDataBuffer[inPacketSizes], "C", 1);
			else if (pobTran->srBRec.uszMobilePayBit == VS_TRUE)
				memcpy(&szDataBuffer[inPacketSizes], "T", 1);
			else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
				memcpy(&szDataBuffer[inPacketSizes], "W", 1);
			else
			{
				if (pobTran->srBRec.uszManualBit == VS_TRUE)
				{
					/* 【需求單 - 105244】端末設備支援以感應方式進行退貨交易 */
					/* 電文轉Manual Keyin但是簽單要印感應的W */
					if (pobTran->srBRec.uszRefundCTLSBit == VS_TRUE)
						memcpy(&szDataBuffer[inPacketSizes], "W", 1);
					else
						memcpy(&szDataBuffer[inPacketSizes], "M", 1);
				}
				else
					memcpy(&szDataBuffer[inPacketSizes], "S", 1);
			}
			
		}
		else
		{
			if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
				memcpy(&szDataBuffer[inPacketSizes], "W", 1);
			else
				memcpy(&szDataBuffer[inPacketSizes], "C", 1);
		}
	}
	inPacketSizes ++;
	
	
	/* 21.Batch No (6 Byte)(145~150) */
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_SALE_MULTI_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_SALE_SINGLE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_REFUND_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_OFFLINE_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_INSTALLMENT_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_REDEEM_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_VOID_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_INQUIRE_TRANSACTION_STAUS_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_REPRINT_RECEIPT_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_SALE_FALLBACK_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_INSTALLMENT_FALLBACK_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_REDEEM_FALLBACK_, 2))
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetBatchNum(szTemplate);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 6);
	}
	inPacketSizes += 6;
	
	/* 22.Chip_NO (16 Byte)(151~166)*/
	if (!memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_SALE_MULTI_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_SALE_SINGLE_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_REFUND_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_OFFLINE_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_INSTALLMENT_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_REDEEM_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_VOID_, 2)			||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_INQUIRE_TRANSACTION_STAUS_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_REPRINT_RECEIPT_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_SALE_FALLBACK_, 2)		||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_INSTALLMENT_FALLBACK_, 2)	||
	    !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TSB_KIOSK_REDEEM_FALLBACK_, 2))
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		if (pobTran->srEMVRec.in9F26_ApplCryptogramLen > 0)
		{
			sprintf(szTemplate, "%02X%02X%02X%02X%02X%02X%02X%02X",
							pobTran->srEMVRec.usz9F26_ApplCryptogram[0],
							pobTran->srEMVRec.usz9F26_ApplCryptogram[1],
							pobTran->srEMVRec.usz9F26_ApplCryptogram[2],
							pobTran->srEMVRec.usz9F26_ApplCryptogram[3],
							pobTran->srEMVRec.usz9F26_ApplCryptogram[4],
							pobTran->srEMVRec.usz9F26_ApplCryptogram[5],
							pobTran->srEMVRec.usz9F26_ApplCryptogram[6],
							pobTran->srEMVRec.usz9F26_ApplCryptogram[7]);
		}
		else
		{
			sprintf(szTemplate, "                ");
		}
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 16);
	}
	inPacketSizes += 16;
	
	
	/* 23.檢查碼 (4 Byte)(167~170) */
	inPacketSizes += 4;
	
	
	/* 24.Reserve (7 Byte)(171~177) */
	inPacketSizes += 7;
	
	
	/* 25.Merchant ID (15 Byte)(178~192) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetMerchantID(szTemplate);
	inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 15, _PADDING_RIGHT_);
	memcpy(&szDataBuffer[inPacketSizes], szTemplate, 15);
	inPacketSizes += 15;
	
	
	/* 26.Reserve (12 Byte)(193~204) */
	inPacketSizes += 12;
	
	
	/* 27.Reserve (2 Byte)(205~206) */
	inPacketSizes += 2;
	
	
	/* 28.Issuer ID (8 Byte)(207~214) */
	inPacketSizes += 8;
	
	
	/* 29.Card No. Vehicle (44 Byte)(215~258) */
	inPacketSizes += 44;
	
	
	/* 30.Reserve (141 Byte)(259~399) */
	inPacketSizes += 141;
	
	return (VS_SUCCESS);
}

/*
Function        :inECR_8N1_TSB_KIOSK_Pack_ResponseCode
Date&Time       :2016/7/6 下午 2:40
Describe        :先把要送的資料組好
*/
int inECR_8N1_TSB_KIOSK_Pack_ResponseCode(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuf)
{
	if (srECROb->srTransData.inErrorType == VS_CALLBANK	||
	    srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_CALLBANK_)
	{
		memcpy(&szDataBuf[78], "0002", 4);
	}
	else if (srECROb->srTransData.inErrorType == VS_TIMEOUT		||
		 srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_TIMEOUT_)
	{
		memcpy(&szDataBuf[78], "0003", 4);
	}
	else if (srECROb->srTransData.inErrorType == VS_USER_OPER_ERR	||	/* 操作錯誤 & 交易流程有誤 */
		 srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_) 
	{
		memcpy(&szDataBuf[78], "0004", 4);
	}
	else if (srECROb->srTransData.inErrorType == VS_ISO_PACK_ERR		|| 
		 srECROb->srTransData.inErrorType == VS_ISO_UNPACK_ERROR	||
		 srECROb->srTransData.inErrorType == VS_COMM_ERROR)
	{
		memcpy(&szDataBuf[78], "0006", 4);
	}
	else if (srECROb->srTransData.inErrorType == VS_USER_CANCEL || 
		 srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_USER_TERMINATE_ERROR_)
	{
		memcpy(&szDataBuf[78], "0006", 4);
	}
	else if (srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_AMOUNT_ERROR_)
	{
		memcpy(&szDataBuf[78], "0007", 4);
	}
	else if (srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_END_INQUIRY_SUCCESS_)
	{
		memcpy(&szDataBuf[78], "0000", 4);
	}
	else
	{
		memcpy(&szDataBuf[78], "0001", 4);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inECR_8N1_Customer_107_Bumper_Unpack
Date&Time       :2017/11/15 下午 2:44
Describe        :分析收銀機傳來的資料
*/
int inECR_8N1_Customer_107_Bumper_Unpack(TRANSACTION_OBJECT *pobTran, ECR_TABLE* srECROb, char *szDataBuffer)
{
	int	i = 0;
	int	inRetVal = VS_SUCCESS;
	int	inTransType = 0;		/* 電文中的交易別字串轉為數字儲存 */
	int	inCardInquiryTRTType = _CARD_INQUIRY_TYPE_NONE_;
	char	szTemplate[100 + 1] = {0};
	char	szDebugMsg[100 + 1] = {0};	
	char	szHostLabel[8 + 1] = {0};
	char	szHostEnable[2 + 1] = {0};
	char	szBatchNum[6 + 1] = {0};
	char	szCTLSEnable[2 + 1] = {0};
	char	szTempSendData[_ECR_BUFF_SIZE_ + 1] = {0};
	char	szPayItemFuncEnable[2 + 1] = {0};
	char	szPayItemCodeTemp[5 + 1] = {0};
	char	szFiscContactlessEnable[1 + 1] = {0};
	char	szCUPContactlessEnable[1 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
	
	/* (需求單 - 107276)自助交易標準做法 EDC於發動交易及完成時，需發出提示音 by Russell 2019/3/8 上午 10:46 */
	inDISP_BEEP(1, 0);
	
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
        inGetCustomIndicator(szCustomerIndicator);
	
	/* 存ECR原始資料 */
	memcpy(srECROb->srTransData.szOrgData, szDataBuffer, _ECR_8N1_Standard_Data_Size_);
	
	inECR_8N1_Standard_Parse_Data(srECROb, szDataBuffer);
	
	/* ECR Indicator :新 ECR連線 Indicator"I""E"(規格新增欄位 )。 */
	/* 【需求單 - 108046】電子發票BIN大於6碼需求 標準400收銀機欄位ECR Indicator新增”E”值 by Russell 2019/7/5 下午 5:55 */
	switch (inTransType)
	{
		default:
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s: %s  ","ECR Indicator", srECROb->srTransData.szECRIndicator);
				inLogPrintf(AT, szDebugMsg);
			}
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(srECROb->srTransData.szECRIndicator) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ","ECR Indicator", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "ECR Indicator Not Exist Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
				return (VS_ERROR);
			}
			
			/* ECR Indicator必為'I' or 'E'，否則error */
			/* 【需求單 - 108046】電子發票BIN大於6碼需求 標準400收銀機欄位ECR Indicator新增”E”值 by Russell 2019/7/5 下午 5:55 */
			if (memcmp(&srECROb->srTransData.szECRIndicator[0], "I", 1) != 0	&&
			    memcmp(&srECROb->srTransData.szECRIndicator[0], "E", 1) != 0)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ","ECR Indicator", "Not \"I\" or \"E\" Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "ECR Indicator Not \"I\" or \"E\" Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
				return (VS_ERROR);
			}
			break;
	}
	
	/* Trans Type Indicator :支援交易別 :一般交易、補登退貨 ，其餘 交易別，端末機不參考此欄位 。*/
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "%s: %s  ", "Trans Type Indicator", srECROb->srTransData.szTransTypeIndicator);
		inLogPrintf(AT, szDebugMsg);
	}
				
	/* Trans Type (交易別) */
	switch (inTransType)
	{
		default:
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, srECROb->srTransData.szTransType, 2);
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s: %s  ", "Trans Type", szTemplate);
				inLogPrintf(AT, szDebugMsg);
			}
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Type", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "Trans Type Not Exist Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
				return (VS_ERROR);
			}
			
			/* 交易類別轉成數字 */
			inTransType = atoi(srECROb->srTransData.szTransType);
			
			break;
	}
	
	/* (需求單 - 107227)邦柏科技自助作業客製化 查詢上一筆機制 by Russell 2018/12/8 下午 2:18 */
	if (inTransType == _ECR_8N1_INQUIRY_LAST_TRANSACTION_NO_)
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_REVIEW_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第一層顯示 <交易查詢> */
		
		/* 複製一份Send Data進去比對 */
		memset(szTempSendData, 0x00, sizeof(szTempSendData));
		memcpy(szTempSendData, szDataBuffer, _ECR_8N1_Standard_Data_Size_);
		
		inRetVal = inECR_Inquiry_Last_Transction(pobTran, srECROb, szTempSendData, _ECR_8N1_Standard_Data_Size_);

		/* 如果比對失敗，回操作錯誤 */
		if (inRetVal == VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "查詢上一筆流程");
			}
			pobTran->uszLastTranscationBit = VS_TRUE;
		}
		else if (inRetVal == VS_TAP_AGAIN)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "TAP AGAIN流程");
			}
			/* 0018 重新感應用成之前的ECR電文 */
			/* 主要是前面的交易類別：查詢上一筆要轉換回原交易別，並用pobTran->uszLastTranscationBit來識別悠遊卡感應要用舊資料，
			    OPT跑特殊流程，TRT則跑正常流程 */
			sprintf(szDataBuffer, szTempSendData);
			memset(srECROb->srTransData.szTransType, 0x00, sizeof(srECROb->srTransData.szTransType));
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &szDataBuffer[8], 2);
			memcpy(srECROb->srTransData.szTransType, szTemplate, 2);
			/* 交易類別轉成數字 */
			inTransType = atoi(srECROb->srTransData.szTransType);
			
			pobTran->uszLastTranscationBit = VS_TRUE;
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "比對失敗");
			}
			/* 比對失敗，要回操作錯誤 */
			memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
			inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
			memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
			sprintf(pobTran->szErrorMsgBuff2, "查詢上一筆比對失敗");
			pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
			return (VS_ERROR);
		}
	}
	else if (inTransType == _ECR_8N1_START_CARD_NO_INQUIRY_NO_)
	{
		/* 不是查詢上一筆的話 移除Retry備份REQ */
		if (!memcmp(&srECROb->srTransData.szField_05[0], "E", 1))
		{
			inFile_Unlink_File(_ECC_API_REQ_RETRY_FILE_, _ECC_FOLDER_PATH_);
		}
	}
	else
	{
		/* 要另外存ECR Request電文 提供查詢上一筆時檢核用 */
		inECR_Save_Request(szDataBuffer, _ECR_8N1_Standard_Data_Size_);

		/* 不是查詢上一筆的話 移除Retry備份REQ */
		if (!memcmp(&srECROb->srTransData.szField_05[0], "E", 1))
		{
			inFile_Unlink_File(_ECC_API_REQ_RETRY_FILE_, _ECC_FOLDER_PATH_);
		}
	}
	
	/* CUP Indicator */
	/* Settlement Indicator */
	/* 若前一次為卡號查詢，標記為第二次ECR */
	if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
	{
		pobTran->uszCardInquiryFirstBit = VS_FALSE;
		pobTran->uszCardInquirysSecondBit = VS_TRUE;
		
		/* 重新決定TRT流程 */
                 if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
                {
                        /* 金融卡查詢要二次過卡，不依循前次查詢界面 */
                        inCardInquiryTRTType = _CARD_INQUIRY_TYPE_NONE_;
                }
                else
                {
                        if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
                        {
                                inCardInquiryTRTType = _CARD_INQUIRY_TYPE_CHIP_;
                        }
                        else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
                        {
                                inCardInquiryTRTType = _CARD_INQUIRY_TYPE_CTLS_;
                        }
                        else
                        {
                                inCardInquiryTRTType = _CARD_INQUIRY_TYPE_MEG_;
                        }
                }
	}
	
	/* 啟動卡號查詢不受Trans Indicator影響 */
	/* 客製化107改為Echo Host */
	if (inTransType == _ECR_8N1_START_CARD_NO_INQUIRY_NO_)
	{
		/* 執行Echo Test */
	}
	else if (inTransType == _ECR_8N1_SETTLEMENT_NO_)
	{
		/* 空白：連動結帳(信用卡+電票)
		 * ‘N’：信用卡結帳
		 * ‘Ｅ’：電票結帳 
		 */
		/* (需求單 - 107227)邦柏科技自助作業客製化 by Russell 2018/12/9 下午 12:20 */
		/* 這裡不做特別檢核，由Host Id再決定結哪個批次的帳 */
		if (memcmp(&srECROb->srTransData.szField_05, " ", 1) == 0)
		{
			pobTran->uszAutoSettleBit = VS_TRUE;
		}
		else if (memcmp(&srECROb->srTransData.szField_05, "N", 1) == 0)
		{
			
		}
		else if (memcmp(&srECROb->srTransData.szField_05, "E", 1) == 0)
		{
			
		}
		else
		{
			if (ginDisplayDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Settlement Indicator error");
				inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_1_, VS_FALSE);
				
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, ": %s", srECROb->srTransData.szField_05);
				inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_2_, VS_FALSE);
			}
			memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
			inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
			memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
			sprintf(pobTran->szErrorMsgBuff2, "%s : 結帳Indicator錯誤", srECROb->srTransData.szField_05);
			pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
			
			return (VS_ERROR);
		}
	}
	else
	{
		/* Trans Type Indicator :支援交易別 :一般交易、補登退貨 ，其餘 交易別，端末機不參考此欄位 。*/
		switch (inTransType)
		{
			case _ECR_8N1_SALE_NO_:				/* 一般交易 */
			case _ECR_8N1_REFUND_NO_:			/* 退貨交易 */
			case _ECR_8N1_OFFLINE_NO_:			/* 交易補登 */
				/* Trans Type Indicator是'S'，要進選擇畫面 */
				if (!memcmp(&srECROb->srTransData.szTransTypeIndicator[0], "S", 1))
				{
					inRetVal = inECR_SelectTransType(srECROb);
					/* 交易類別轉成數字 */
					inTransType = atoi(srECROb->srTransData.szTransType);

					/* Select失敗 */
					if (inRetVal != VS_SUCCESS)
					{
						return (inRetVal);
					}
				}
				break;
			default:
				break;
		}

		/* CUP/Smart pay Indicator:銀聯卡 / Smart Pay交易使用
		 * Indicator = 'C'，表示為CUP交易
		 * Indicator = 'N'，表示為一般信用卡交易
		 * Indicator = 'S'，表示為SmartPay交易，(SmartPay的Sale Reversal和Refund，收銀機一定要送'S'。SmartPay的Sale送'N'，因為Sale不需要按Hotkey)
		 */
		switch (inTransType)
		{
			case _ECR_8N1_START_CARD_NO_INQUIRY_NO_:	/* 啟動卡號查詢 */
			case _ECR_8N1_SALE_NO_:				/* 一般交易 */
			case _ECR_8N1_REFUND_NO_:			/* 退貨交易 */
			case _ECR_8N1_VOID_NO_:				/* 取消 */
			case _ECR_8N1_PREAUTH_NO_:			/* 預先授權 */
			case _ECR_8N1_PREAUTH_COMPLETE_NO_:		/* 預先授權完成 */
			case _ECR_8N1_ESVC_TOP_UP_NO_:			/* 電子票證加值 */
			case _ECR_8N1_ESVC_BALANCE_INQUIRY_NO_:		/* 電子票證餘額查詢 */
			case _ECR_8N1_ESVC_VOID_TOP_UP_NO_:		/* 電子票證加值取消 */
			case _ECR_8N1_HG_REWARD_SALE_NO_:		/* HG紅利積點 */
			case _ECR_8N1_HG_ONLINE_REDEEMPTION_NO_:	/* HG點數抵扣 */
			case _ECR_8N1_HG_POINT_CERTAIN_NO_:		/* HG加價購 */
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "CUPIndicator", srECROb->srTransData.szField_05);
					inLogPrintf(AT, szDebugMsg);
				}

				/* 欄位為M 卻不存在，回傳錯誤 */
				if (inFunc_CheckFullSpace(srECROb->srTransData.szField_05) == VS_TRUE)
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s: %s  ", "CUPIndicator", "Not Exist Error");
						inLogPrintf(AT, szDebugMsg);
					}
					memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
					inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "CUPIndicator Not Exist Error");
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

					return (VS_ERROR);
				}

				break;
			case _ECR_8N1_PREAUTH_CANCEL_NO_:		/* 預先授權取消 */
			case _ECR_8N1_PREAUTH_COMPLETE_CANCEL_NO_:	/* 預先授權完成取消 */
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "  CUPIndicator :%s  ", srECROb->srTransData.szField_05);
					inLogPrintf(AT, szDebugMsg);
				}

				/* 欄位為M 卻不存在，回傳錯誤 */
				if (inFunc_CheckFullSpace(srECROb->srTransData.szField_05) == VS_TRUE)
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s: %s  ", "CUPIndicator", "Not Exist Error");
						inLogPrintf(AT, szDebugMsg);
					}
					memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
					inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "CUPIndicator Not Exist Error");
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

					return (VS_ERROR);
				}

				/* 預先授權取消及完成僅支援銀聯卡交易 ，故此二Request之 CUP Indicator欄位僅可放 ”C”*/
				if (memcmp(&srECROb->srTransData.szField_05[0], "C", 1) != 0)
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s: %s  ", "CUPIndicator", "CUP Not\"C\" Error");
						inLogPrintf(AT, szDebugMsg);
					}
					memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
					inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "非銀聯交易別帶CUP Indicator");
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

					return (VS_ERROR);
				}
				break;

			default:				
				break;
		}

		switch (inTransType)
		{
			case _ECR_8N1_START_CARD_NO_INQUIRY_NO_:	/* 啟動卡號查詢 */
			case _ECR_8N1_SALE_NO_:				/* 一般交易 */
			case _ECR_8N1_REFUND_NO_:			/* 退貨交易 */
			case _ECR_8N1_VOID_NO_:				/* 取消 */
			case _ECR_8N1_PREAUTH_NO_:			/* 預先授權 */
			case _ECR_8N1_PREAUTH_COMPLETE_NO_:		/* 預先授權完成 */
			case _ECR_8N1_ESVC_TOP_UP_NO_:			/* 電子票證加值 */
			case _ECR_8N1_ESVC_BALANCE_INQUIRY_NO_:		/* 電子票證餘額查詢 */
			case _ECR_8N1_ESVC_VOID_TOP_UP_NO_:		/* 電子票證加值取消 */
			case _ECR_8N1_HG_REWARD_SALE_NO_:		/* HG紅利積點 */
			case _ECR_8N1_HG_ONLINE_REDEEMPTION_NO_:	/* HG點數抵扣 */
			case _ECR_8N1_HG_POINT_CERTAIN_NO_:		/* HG加價購 */
			case _ECR_8N1_PREAUTH_CANCEL_NO_:		/* 預先授權取消 */
			case _ECR_8N1_PREAUTH_COMPLETE_CANCEL_NO_:	/* 預先授權完成取消 */
				/* 如果是'C'代表是銀聯卡交易 */
				if (memcmp(&srECROb->srTransData.szField_05[0], "C", 1) == 0)
				{
					pobTran->srBRec.uszCUPTransBit = VS_TRUE;
					/* 銀聯目前沒這些交易別，要回接收資料錯誤 */
					switch (inTransType)
					{
						case _ECR_8N1_INSTALLMENT_NO_:
						case _ECR_8N1_REDEEM_NO_:
						case _ECR_8N1_INSTALLMENT_REFUND_NO_:
						case _ECR_8N1_REDEEM_REFUND_NO_:
						case _ECR_8N1_INSTALLMENT_ADJUST_NO_:
						case _ECR_8N1_REDEEM_ADJUST_NO_:
						case _ECR_8N1_OFFLINE_NO_:
							memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
							inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
							memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
							sprintf(pobTran->szErrorMsgBuff2, "銀聯無此交易別");
							pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
							return (VS_ERROR);
							break;
						default:
							break;
					}
				}

				/* 如果是'S'代表是SmartPay交易 */
				if (memcmp(&srECROb->srTransData.szField_05[0], "S", 1) == 0)
				{
					pobTran->srBRec.uszFiscTransBit = VS_TRUE;
					/* SmartPay不用簽名 */
					pobTran->srBRec.uszNoSignatureBit = VS_TRUE;
				}

				/* 如果是'E'代表是電票交易 */
				if (memcmp(&srECROb->srTransData.szField_05[0], "E", 1) == 0)
				{
					pobTran->srTRec.uszESVCTransBit = VS_TRUE;
				}
				break;
			default:
				break;
		}
	}
	
	/* 銀行別/付款別 HOST ID*/
	switch (inTransType)
	{
		case _ECR_8N1_VOID_NO_:				/* 取消交易 */
		case _ECR_8N1_TIP_NO_:				/* 小費交易 */
		case _ECR_8N1_PREAUTH_CANCEL_NO_:		/* 預先授權取消 */
		case _ECR_8N1_PREAUTH_COMPLETE_CANCEL_NO_:	/* 預先授權完成取消 */
		case _ECR_8N1_REPRINT_RECEIPT_NO_:		/* 重印簽單 */
		case _ECR_8N1_MENU_REVIEW_DETAIL_NO_:		/* 明細查詢 */
		case _ECR_8N1_MENU_REVIEW_TOTAL_NO_:		/* 總額查詢 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, srECROb->srTransData.szHostID, 2);
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "HOST ID", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "HOST ID Not Exist Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
				return (VS_ERROR);
			}
			
			memset(szHostLabel, 0x00, sizeof(szHostLabel));

			if (memcmp(szTemplate, _ECR_8N1_NCCC_HOSTID_NCCC_, 2) == 0)
			{
				memcpy(szHostLabel, _HOST_NAME_CREDIT_NCCC_, strlen(_HOST_NAME_CREDIT_NCCC_));
			}
			else if (memcmp(szTemplate, _ECR_8N1_NCCC_HOSTID_DCC_, 2) == 0)
			{
				memcpy(szHostLabel, _HOST_NAME_DCC_, strlen(_HOST_NAME_DCC_));
			}
			/* 2018/5/8 下午 2:10 DFS需求不再使用大來主機 */
			else if (memcmp(szTemplate, _ECR_8N1_NCCC_HOSTID_LOYALTY_REDEEM_, 2) == 0)
			{
				memcpy(szHostLabel, _HOST_NAME_CREDIT_NCCC_, strlen(_HOST_NAME_CREDIT_NCCC_));
			}
			else if (memcmp(szTemplate, _ECR_8N1_NCCC_HOSTID_ESVC_, 2) == 0)
			{
				memcpy(szHostLabel, _HOST_NAME_ESVC_, strlen(_HOST_NAME_ESVC_));
				pobTran->srTRec.uszESVCTransBit = VS_TRUE;
			}
			else if (memcmp(szTemplate, _ECR_8N1_NCCC_HOSTID_HG_, 2) == 0)
			{
				memcpy(szHostLabel, _HOST_NAME_HG_, strlen(_HOST_NAME_HG_));
			}
			else
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "無對應Host ID");
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "%s : HOST ID Not Pair", szTemplate);
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
				return (VS_ERROR);
			}
			
			/* 初始化 */
			i = 0;
			do
			{
				/* 按順序load每一個HOST */
				if (inLoadHDTRec(i) < 0)
				{
					/* 如果return VS_ERROR 代表table沒有該HOST */
					memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
					inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "%s : HOST ID Not Found", szHostLabel);
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
					
					return (VS_ERROR);
				}

				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetHostLabel(szTemplate);
				if (!memcmp(szTemplate, szHostLabel, strlen(szHostLabel)))
				{
					/* 比對成功後，判斷HOST是否開啟  */
					memset(szHostEnable, 0x00, sizeof(szHostEnable));
					inGetHostEnable(szHostEnable);
					if (szHostEnable[0] != 'Y')
					{
						/* Host沒開 */
						if (ginDebug == VS_TRUE) 
						{
							memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
							sprintf(szDebugMsg, "%s Not Open Error", szHostLabel);
							inLogPrintf(AT, szDebugMsg);
						}

						/* 如果return VS_ERROR 代表Host沒開 */
						memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
						inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
						memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
						sprintf(pobTran->szErrorMsgBuff2, "%s : HOST ID Not Open", szHostLabel);
						pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
						
						return (VS_ERROR);
					}
					else
					{
						/* Host有開 */
						pobTran->srBRec.inHDTIndex = i;
						
						/* Load HDPT */
						inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
						memset(szBatchNum, 0x00, sizeof(szBatchNum));
						inGetBatchNum(szBatchNum);
						pobTran->srBRec.lnBatchNum = atol(szBatchNum);
						
						if (ginDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
							sprintf(szDebugMsg, "%s Open", szHostLabel);
							inLogPrintf(AT, szDebugMsg);
						}
						
					}/* Host Enable比對End */

					break;
				}/* Host Label比對 End */
				i++;

			} while (1);
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%d", "HDTindex", pobTran->srBRec.inHDTIndex);
				inLogPrintf(AT, szDebugMsg);
			}
			
			break;
		case _ECR_8N1_SETTLEMENT_NO_:
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, srECROb->srTransData.szHostID, 2);
			
			memset(szHostLabel, 0x00, sizeof(szHostLabel));
			
			/* 
			 * 空白連動結帳(信用卡+電票)
			 * ‘03’：信用卡結帳
			 * ‘06’：電票結帳
			 */
			if (memcmp(szTemplate, _ECR_8N1_NCCC_HOSTID_NCCC_, 2) == 0)
			{
				memcpy(szHostLabel, _HOST_NAME_CREDIT_NCCC_, strlen(_HOST_NAME_CREDIT_NCCC_));
			}
			else if (memcmp(szTemplate, _ECR_8N1_NCCC_HOSTID_ESVC_, 2) == 0)
			{
				memcpy(szHostLabel, _HOST_NAME_ESVC_, strlen(_HOST_NAME_ESVC_));
				pobTran->srTRec.uszESVCTransBit = VS_TRUE;
			}
			else if (memcmp(szTemplate, "  ", 2) == 0)
			{
				pobTran->uszAutoSettleBit = VS_TRUE;
			}
			else
			{
				if (ginDisplayDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "Host ID Err :%s", szTemplate);
					inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_1_, VS_FALSE);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "%s : HOST ID Not Pair", szTemplate);
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
				return (VS_ERROR);
			}
			
			/* 個別結帳 */
			if (strlen(szHostLabel) > 0)
			{
				/* 初始化 */
				i = 0;
				do
				{
					/* 按順序load每一個HOST */
					if (inLoadHDTRec(i) < 0)
					{
						/* 如果return VS_ERROR 代表table沒有該HOST */
						memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
						inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
						memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
						sprintf(pobTran->szErrorMsgBuff2, "%s : HOST ID Not Found", szHostLabel);
						pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
					
						return (VS_ERROR);
					}

					memset(szTemplate, 0x00, sizeof(szTemplate));
					inGetHostLabel(szTemplate);
					if (!memcmp(szTemplate, szHostLabel, strlen(szHostLabel)))
					{
						/* 比對成功後，判斷HOST是否開啟  */
						memset(szHostEnable, 0x00, sizeof(szHostEnable));
						inGetHostEnable(szHostEnable);
						if (szHostEnable[0] != 'Y')
						{
							/* Host沒開 */
							if (ginDebug == VS_TRUE) 
							{
								memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
								sprintf(szDebugMsg, "%s Not Open Error", szHostLabel);
								inLogPrintf(AT, szDebugMsg);
							}
							
							memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
							inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
							memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
							sprintf(pobTran->szErrorMsgBuff2, "%s : HOST ID Not Open", szHostLabel);
							pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

							/* 如果return VS_ERROR 代表Host沒開 */
							return (VS_ERROR);
						}
						else
						{
							/* Host有開 */
							pobTran->srBRec.inHDTIndex = i;

							/* Load HDPT */
							inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
							memset(szBatchNum, 0x00, sizeof(szBatchNum));
							inGetBatchNum(szBatchNum);
							pobTran->srBRec.lnBatchNum = atol(szBatchNum);

							if (ginDebug == VS_TRUE)
							{
								memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
								sprintf(szDebugMsg, "%s Open", szHostLabel);
								inLogPrintf(AT, szDebugMsg);
							}

						}/* Host Enable比對End */

						break;
					}/* Host Label比對 End */
					i++;

				} while (1);
			}
			/* 連動結帳 */
			else
			{
				
			}
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%d", "HDTindex", pobTran->srBRec.inHDTIndex);
				inLogPrintf(AT, szDebugMsg);
			}
			
			break;
		default:
			break;
	}
	
	/* EDC簽單序號 Receipt No(InvoiceNumber) */
	switch (inTransType)
	{
		case _ECR_8N1_VOID_NO_:				/* 取消交易 */
		case _ECR_8N1_TIP_NO_:				/* 小費交易 */
		case _ECR_8N1_PREAUTH_CANCEL_NO_:		/* 預先授權取消 */
		case _ECR_8N1_PREAUTH_COMPLETE_CANCEL_NO_:	/* 預先授權完成取消 */
		case _ECR_8N1_REPRINT_RECEIPT_NO_:		/* 重印簽單 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], srECROb->srTransData.szReceiptNo, 6);
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Receipt No", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "Receipt No Not Exist Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
				return (VS_ERROR);
			}
			else
			{
				pobTran->srBRec.lnOrgInvNum = atol(szTemplate);
			}
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%ld", "lnOrgInvNum", pobTran->srBRec.lnOrgInvNum);
				inLogPrintf(AT, szDebugMsg);
			}
			
			break;
		default:
			break;
	}
	
	/* 處理交易金額 (Trans Amount) */
	switch (inTransType)
	{
		case _ECR_8N1_START_CARD_NO_INQUIRY_NO_:	/* 啟動卡號查詢 */
		case _ECR_8N1_SALE_NO_:				/* 一般交易 */
		case _ECR_8N1_REFUND_NO_:			/* 退貨交易 */
		case _ECR_8N1_OFFLINE_NO_:			/* 交易補登 */
		case _ECR_8N1_PREAUTH_NO_:			/* 預先授權 */
		case _ECR_8N1_PREAUTH_COMPLETE_NO_:		/* 預先授權完成 */
		case _ECR_8N1_INSTALLMENT_NO_:			/* 分期 */
		case _ECR_8N1_INSTALLMENT_REFUND_NO_:		/* 分期退貨 */
		case _ECR_8N1_INSTALLMENT_ADJUST_NO_:		/* 分期調帳 */
		case _ECR_8N1_REDEEM_NO_:			/* 紅利 */		
		case _ECR_8N1_REDEEM_REFUND_NO_:		/* 紅利退貨 */
		case _ECR_8N1_REDEEM_ADJUST_NO_:		/* 紅利調帳 */
		case _ECR_8N1_ESVC_TOP_UP_NO_:			/* 電子票證加值 */
		case _ECR_8N1_ESVC_VOID_TOP_UP_NO_:		/* 電子票證加值取消 */
		case _ECR_8N1_HG_REWARD_SALE_NO_:		/* HG紅利積點 */
		case _ECR_8N1_HG_REWARD_INSTALLMENT_NO_ :	/* 快樂購紅利積點 + 信用卡分期付款 */
		case _ECR_8N1_HG_REWARD_REDEMPTION_NO_ :	/* 快樂購紅利積點 + 信用卡紅利扣抵 */
		case _ECR_8N1_HG_ONLINE_REDEEMPTION_NO_:	/* HG點數抵扣 */
		case _ECR_8N1_HG_POINT_CERTAIN_NO_:		/* HG加價購 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &srECROb->srTransData.szField_10[0], 10);
			if (pobTran->srEWRec.uszEWTransBit == VS_TRUE)
			{
				memcpy(pobTran->srEWRec.szEW_Decimal, &srECROb->srTransData.szField_10[10], 2);
			}
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Amount", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "Trans Amount Not Exist Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
				return (VS_ERROR);
			}
			else if ((!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_076_8_DIGITS_, _CUSTOMER_INDICATOR_SIZE_) && atol(szTemplate) > 99999999) || 
				 (memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_076_8_DIGITS_, _CUSTOMER_INDICATOR_SIZE_) && atol(szTemplate) > 9999999))
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Amount", "Out of bound");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_076_8_DIGITS_, _CUSTOMER_INDICATOR_SIZE_))
				{
					sprintf(pobTran->szErrorMsgBuff2, "金額超過8位數");
				}
				else
				{
					sprintf(pobTran->szErrorMsgBuff2, "金額超過7位數");
				}
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
				
				return (VS_ERROR);
			}
			else
			{
				pobTran->srBRec.lnTxnAmount = atol(szTemplate);
				pobTran->srBRec.lnOrgTxnAmount = atol(szTemplate);
				pobTran->srBRec.lnTotalTxnAmount = atol(szTemplate);
			}

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%ld", "交易金額", pobTran->srBRec.lnTxnAmount);
				inLogPrintf(AT, szDebugMsg);
			}
			
			if (pobTran->srEWRec.uszEWTransBit == VS_TRUE)
			{
				sprintf(pobTran->srEWRec.szEW_Decimal, "%02d", atoi(&srECROb->srTransData.szField_10[10]));
				
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s :%s", "金額小數位", pobTran->srEWRec.szEW_Decimal);
					inLogPrintf(AT, szDebugMsg);
				}
			}
			
			break;
		default:
			break;
	}
	
	/* 交易日期 Trans Date */
	switch (inTransType)
	{
		case _ECR_8N1_START_CARD_NO_INQUIRY_NO_:	/* 啟動卡號查詢 */
		case _ECR_8N1_END_CARD_NO_INQUIRY_NO_:		/* 結束卡號查詢*/
//		case _ECR_8N1_SALE_NO_:				/* 一般交易 */		/* 不以ECR日期為主，會發生與機器日期不符的狀況 */
		case _ECR_8N1_REFUND_NO_:			/* 退貨交易 */
		case _ECR_8N1_VOID_NO_:				/* 取消交易 */
//		case _ECR_8N1_OFFLINE_NO_:			/* 交易補登 */		/* 不以ECR日期為主，會發生與機器日期不符的狀況 */
		case _ECR_8N1_TIP_NO_:				/* 小費交易 */
//		case _ECR_8N1_PREAUTH_NO_:			/* 預先授權 */		/* 不以ECR日期為主，會發生與機器日期不符的狀況 */
		case _ECR_8N1_PREAUTH_CANCEL_NO_:		/* 預先授權取消 */
		case _ECR_8N1_PREAUTH_COMPLETE_NO_:		/* 預先授權完成 */
		case _ECR_8N1_PREAUTH_COMPLETE_CANCEL_NO_:	/* 預先授權完成取消 */
//		case _ECR_8N1_INSTALLMENT_NO_:			/* 分期 */		/* 不以ECR日期為主，會發生與機器日期不符的狀況 */
		case _ECR_8N1_INSTALLMENT_REFUND_NO_:		/* 分期退貨 */
		case _ECR_8N1_INSTALLMENT_ADJUST_NO_:		/* 分期調帳 */
//		case _ECR_8N1_REDEEM_NO_:			/* 紅利 */		/* 不以ECR日期為主，會發生與機器日期不符的狀況 */		
		case _ECR_8N1_REDEEM_REFUND_NO_:		/* 紅利退貨 */
		case _ECR_8N1_REDEEM_ADJUST_NO_:		/* 紅利調帳 */
		case _ECR_8N1_SETTLEMENT_NO_:			/* 結帳交易 */
		case _ECR_8N1_REPRINT_RECEIPT_NO_:		/* 重印簽單 */
		case _ECR_8N1_MENU_REVIEW_DETAIL_NO_:		/* 明細查詢 */
		case _ECR_8N1_MENU_REVIEW_TOTAL_NO_:		/* 總額查詢 */
		case _ECR_8N1_MENU_REPORT_DETAIL_NO_:		/* 明細列印 */
		case _ECR_8N1_MENU_REPORT_TOTAL_NO_:		/* 總額列印 */
		case _ECR_8N1_ESVC_TOP_UP_NO_:			/* 電子票證加值 */
		case _ECR_8N1_ESVC_BALANCE_INQUIRY_NO_:		/* 電子票證餘額查詢 */
		case _ECR_8N1_ESVC_VOID_TOP_UP_NO_:		/* 電子票證加值取消 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &srECROb->srTransData.szTransDate[0], 6);
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Date", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				
//				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
//				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
//				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
//				sprintf(pobTran->szErrorMsgBuff2, "Trans Date Not Exist Error");
//				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
//				
//				return (VS_ERROR);
			}
			else
			{
				/* 根據安全理由，不採信ECR日期時間 */
			}
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "交易日期", pobTran->srBRec.szDate);
				inLogPrintf(AT, szDebugMsg);
			}
			break;
		default:
			break;
	}
	
	/* 交易時間 Trans Time */
	switch (inTransType)
	{
		case _ECR_8N1_START_CARD_NO_INQUIRY_NO_:	/* 啟動卡號查詢 */
		case _ECR_8N1_END_CARD_NO_INQUIRY_NO_:		/* 結束卡號查詢*/
		case _ECR_8N1_SALE_NO_:				/* 一般交易 */
		case _ECR_8N1_REFUND_NO_:			/* 退貨交易 */
		case _ECR_8N1_VOID_NO_:				/* 取消交易 */
		case _ECR_8N1_OFFLINE_NO_:			/* 交易補登 */
		case _ECR_8N1_TIP_NO_:				/* 小費交易 */
		case _ECR_8N1_PREAUTH_NO_:			/* 預先授權 */
		case _ECR_8N1_PREAUTH_CANCEL_NO_:		/* 預先授權取消 */
		case _ECR_8N1_PREAUTH_COMPLETE_NO_:		/* 預先授權完成 */
		case _ECR_8N1_PREAUTH_COMPLETE_CANCEL_NO_:	/* 預先授權完成取消 */
		case _ECR_8N1_INSTALLMENT_NO_:			/* 分期 */
		case _ECR_8N1_INSTALLMENT_REFUND_NO_:		/* 分期退貨 */
		case _ECR_8N1_INSTALLMENT_ADJUST_NO_:		/* 分期調帳 */
		case _ECR_8N1_REDEEM_NO_:			/* 紅利 */		
		case _ECR_8N1_REDEEM_REFUND_NO_:		/* 紅利退貨 */
		case _ECR_8N1_REDEEM_ADJUST_NO_:		/* 紅利調帳 */
		case _ECR_8N1_SETTLEMENT_NO_:			/* 結帳交易 */
		case _ECR_8N1_REPRINT_RECEIPT_NO_:		/* 重印簽單 */
		case _ECR_8N1_MENU_REVIEW_DETAIL_NO_:		/* 明細查詢 */
		case _ECR_8N1_MENU_REVIEW_TOTAL_NO_:		/* 總額查詢 */
		case _ECR_8N1_MENU_REPORT_DETAIL_NO_:		/* 明細列印 */
		case _ECR_8N1_MENU_REPORT_TOTAL_NO_:		/* 總額列印 */
		case _ECR_8N1_ESVC_TOP_UP_NO_:			/* 電子票證加值 */
		case _ECR_8N1_ESVC_BALANCE_INQUIRY_NO_:		/* 電子票證餘額查詢 */
		case _ECR_8N1_ESVC_VOID_TOP_UP_NO_:		/* 電子票證加值取消 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szTransTime[0], 6);
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Time", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				
//				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
//				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
//				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
//				sprintf(pobTran->szErrorMsgBuff2, "Trans Time Not Exist Error");
//				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
//				
//				return (VS_ERROR);
			}
			else
			{
				/* 根據安全理由，不採信ECR日期時間 */
			}
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "交易時間", pobTran->srBRec.szTime);
				inLogPrintf(AT, szDebugMsg);
			}
			
			break;
			
		default:
			break;
	}
	
	/* 授權碼(Auth Code) ECR電文中給9個字元 但ATS電文只需要6個，所以只抓6個 */
	switch (inTransType)
	{
		case _ECR_8N1_REFUND_NO_:			/* 退貨 */
		case _ECR_8N1_OFFLINE_NO_:			/* 交易補登 */
		case _ECR_8N1_INSTALLMENT_REFUND_NO_:		/* 分期退貨 */
		case _ECR_8N1_INSTALLMENT_ADJUST_NO_:		/* 分期調帳 */
		case _ECR_8N1_REDEEM_REFUND_NO_:		/* 紅利退貨 */
		case _ECR_8N1_REDEEM_ADJUST_NO_:		/* 紅利調帳 */
			/* 處理授權碼(optional) */
			memset(pobTran->srBRec.szAuthCode, 0x00, sizeof(pobTran->srBRec.szAuthCode));
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szApprovalNo[0], 6);
			memcpy(pobTran->srBRec.szAuthCode, szTemplate, 6);
			break;
		case _ECR_8N1_PREAUTH_COMPLETE_NO_:		/* 預先授權完成 */
			/* 處理授權碼(must have) */
			memset(pobTran->srBRec.szAuthCode, 0x00, sizeof(pobTran->srBRec.szAuthCode));
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szApprovalNo[0], 6);
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Auth Code", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "Auth Code Not Exist Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
				return (VS_ERROR);
			}
			else
			{
				memcpy(pobTran->srBRec.szAuthCode, szTemplate, 6);
			}
			
			break;
			
		default :
			break;
	}
	
	/* 其他金額(Exp Amount) */
	switch (inTransType)
	{
		case _ECR_8N1_TIP_NO_:				/* 小費交易 */
			/* 小費金額*/
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetTRTFileName(szTemplate);
			if (!memcmp(szTemplate, _TRT_FILE_NAME_DCC_, 6) && !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TIP_, 2))
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &srECROb->srTransData.szExpAmount[0], 12);
			}
			else
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &srECROb->srTransData.szExpAmount[0], 10);
			}
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Exp Amount", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "Exp Amount Not Exist Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
				return (VS_ERROR);
			}
			
			pobTran->lnOldTaxAmount = atol(szTemplate);
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Tip: %ld", pobTran->lnOldTaxAmount);
				inLogPrintf(AT, szDebugMsg);
			}

			break;
			
		case _ECR_8N1_PREAUTH_COMPLETE_NO_:			/* 預先授權完成 預先授權完成交易之原預先授權金額 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szAuthAmount[0], 10);

			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Exp Amount", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "Exp Amount Not Exist Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
				return (VS_ERROR);
			}

			pobTran->srBRec.lnOrgTxnAmount = atol(szTemplate);
			break;
			
		default :
			break;
	}
	
	/* 處理櫃號(Store ID) */
	switch (inTransType)
	{
		case _ECR_8N1_START_CARD_NO_INQUIRY_NO_:	/* 啟動卡號查詢 */
		case _ECR_8N1_SALE_NO_:				/* 一般交易 */
		case _ECR_8N1_REFUND_NO_:			/* 退貨交易 */
		case _ECR_8N1_OFFLINE_NO_:			/* 交易補登 */
		case _ECR_8N1_PREAUTH_NO_:			/* 預先授權 */
		case _ECR_8N1_PREAUTH_CANCEL_NO_:		/* 預先授權取消 */
		case _ECR_8N1_PREAUTH_COMPLETE_NO_:		/* 預先授權完成 */
		case _ECR_8N1_PREAUTH_COMPLETE_CANCEL_NO_:	/* 預先授權完成取消 */
		case _ECR_8N1_INSTALLMENT_NO_:			/* 分期 */
		case _ECR_8N1_INSTALLMENT_REFUND_NO_:		/* 分期退貨 */
		case _ECR_8N1_INSTALLMENT_ADJUST_NO_:		/* 分期調帳 */
		case _ECR_8N1_REDEEM_NO_:			/* 紅利 */
		case _ECR_8N1_REDEEM_REFUND_NO_:		/* 紅利退貨 */
		case _ECR_8N1_REDEEM_ADJUST_NO_:		/* 紅利調帳 */
		case _ECR_8N1_HG_REWARD_SALE_NO_:		/* HG紅利積點 */
		case _ECR_8N1_HG_REWARD_INSTALLMENT_NO_ :	/* 快樂購紅利積點 + 信用卡分期付款 */
		case _ECR_8N1_HG_REWARD_REDEMPTION_NO_ :	/* 快樂購紅利積點 + 信用卡紅利扣抵 */
		case _ECR_8N1_HG_ONLINE_REDEEMPTION_NO_ :	/* 快樂購點數扣抵 */
		case _ECR_8N1_HG_POINT_CERTAIN_NO_ :		/* 快樂購加價購 */
		case _ECR_8N1_HG_FULL_REDEEMPTION_NO_ :		/* 快樂購點數兌換 */
		case _ECR_8N1_HG_REDEEM_REFUND_NO_ :		/* 快樂購扣抵退貨 */
		case _ECR_8N1_HG_REWARD_REFUND_NO_ :		/* 快樂購回饋退貨 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szStoreId[0], 18);
			
			if (strlen(szTemplate) > 0)
			{
				memcpy(pobTran->srBRec.szStoreID, szTemplate, strlen(szTemplate));
				/* 櫃號不滿18，補空白 */
				if (strlen(szTemplate) < 18)
				{
					memset(&pobTran->srBRec.szStoreID[strlen(szTemplate)], 0x20, 18 - strlen(szTemplate));
				}

			}
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "Store ID", pobTran->srBRec.szStoreID);
				inLogPrintf(AT, szDebugMsg);
			}
			
			break;
		case _ECR_8N1_ESVC_TOP_UP_NO_:			/* 電子票證加值 */
		case _ECR_8N1_ESVC_VOID_TOP_UP_NO_:		/* 電子票證加值取消 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szStoreId[0], 18);
			
			if (strlen(szTemplate) > 0)
			{
				memcpy(pobTran->srBRec.szStoreID, szTemplate, strlen(szTemplate));
				/* 櫃號不滿18，補空白 */
				if (strlen(szTemplate) < 18)
				{
					memset(&pobTran->srBRec.szStoreID[strlen(szTemplate)], 0x20, 18 - strlen(szTemplate));
				}

			}
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "Store ID", pobTran->srBRec.szStoreID);
				inLogPrintf(AT, szDebugMsg);
			}
			
			break;
		default:
			break;
	}
	
	if (inTransType == _ECR_8N1_AWARD_REDEEM_NO_)
	{
		/* 兌換模式 */
		switch (inTransType)
		{
			case _ECR_8N1_AWARD_REDEEM_NO_:		/* 優惠兌換 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &srECROb->srTransData.szField_20[0], 1);	/* (1=條碼兌換，2=卡號兌換) */

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s :%s", "Award Redeem Mode", szTemplate);
					inLogPrintf(AT, szDebugMsg);
				}
				
				/* 條碼兌換 */
				if (memcmp(szTemplate, "1", 1) == 0)
				{
					memcpy(pobTran->szL3_AwardWay, "1", 1);
				}
				else
				{
					/* 至EDC過卡流程決定 */
					memcpy(pobTran->szL3_AwardWay, "0", 1);
				}
				
				/* 兌換方式為條碼才抓條碼*/
				if (memcmp(pobTran->szL3_AwardWay, "1", strlen("1")) == 0)
				{
					if (srECROb->srTransData.szBarCode1[0] == ' ' && srECROb->srTransData.szBarCode2[0] == ' ')
					{
						if (ginDebug == VS_TRUE)
						{
							inLogPrintf(AT, "ECR未輸入條碼");
						}
						memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
						inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
						memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
						sprintf(pobTran->szErrorMsgBuff2, "ECR未輸入條碼");
						pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
						
						return (VS_ERROR);
					}
					
					/* 條碼兌換 防呆&阻擋&存Barcode */
					/* 11 表示只有一個一維條碼，且這是第1 個。 */
					if (srECROb->srTransData.szBarCode1[0] == '1' && srECROb->srTransData.szBarCode1[1] == '1')
					{
						memcpy(&pobTran->szL3_Barcode1[0], &srECROb->srTransData.szBarCode1[0], 20);
						sprintf(pobTran->szL3_Barcode1Len, "%02d", strlen(pobTran->szL3_Barcode1));
					}
					else if (srECROb->srTransData.szBarCode2[0] == '1' && srECROb->srTransData.szBarCode2[1] == '1')
					{
						memcpy(&pobTran->szL3_Barcode1[0], &srECROb->srTransData.szBarCode2[0], 20);
						sprintf(pobTran->szL3_Barcode1Len, "%02d", strlen(pobTran->szL3_Barcode1));
					}
					else if (srECROb->srTransData.szBarCode1[0] == '2' && srECROb->srTransData.szBarCode1[1] == '1')
					{
						if (srECROb->srTransData.szBarCode2[0] == '2' && srECROb->srTransData.szBarCode2[1] == '2')
						{
							memcpy(&pobTran->szL3_Barcode1[0], &srECROb->srTransData.szBarCode1[0], 20);
							sprintf(pobTran->szL3_Barcode1Len, "%02d", strlen(pobTran->szL3_Barcode1));
							memcpy(&pobTran->szL3_Barcode2[0], &srECROb->srTransData.szBarCode2[0], 20);
							sprintf(pobTran->szL3_Barcode2Len, "%02d", strlen(pobTran->szL3_Barcode2));
						}
						else
						{
							if (ginDebug == VS_TRUE)
							{
								inLogPrintf(AT, "ECR條碼邏輯錯誤");
							}
							memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
							inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
							memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
							sprintf(pobTran->szErrorMsgBuff2, "ECR條碼邏輯錯誤");
							pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
							
							return (VS_ERROR);
						}
					}
					else if (srECROb->srTransData.szBarCode1[0] == '2' && srECROb->srTransData.szBarCode1[1] == '2')
					{
						if (srECROb->srTransData.szBarCode2[0] == '2' && srECROb->srTransData.szBarCode2[1] == '1')
						{
							memcpy(&pobTran->szL3_Barcode2[0], &srECROb->srTransData.szBarCode1[0], 20);
							sprintf(pobTran->szL3_Barcode2Len, "%02d", strlen(pobTran->szL3_Barcode2));
							memcpy(&pobTran->szL3_Barcode1[0], &srECROb->srTransData.szBarCode2[0], 20);
							sprintf(pobTran->szL3_Barcode1Len, "%02d", strlen(pobTran->szL3_Barcode1));
						}
						else
						{
							if (ginDebug == VS_TRUE)
							{
								inLogPrintf(AT, "ECR條碼邏輯錯誤");
							}
							memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
							inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
							memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
							sprintf(pobTran->szErrorMsgBuff2, "ECR條碼邏輯錯誤");
							pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
							
							return (VS_ERROR);
						}
					}
					else
					{
						if (ginDebug == VS_TRUE)
						{
							inLogPrintf(AT, "ECR條碼邏輯錯誤");
						}
						memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
						inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
						memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
						sprintf(pobTran->szErrorMsgBuff2, "ECR條碼邏輯錯誤");
						pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
						
						return (VS_ERROR);
					}
				}
				break;
			default:
				break;
		}
	}
	else if (inTransType == _ECR_8N1_VOID_AWARD_REDEEM_NO_)
	{
		/* 兌換模式 */
		switch (inTransType)
		{
			case _ECR_8N1_VOID_AWARD_REDEEM_NO_:		/* 優惠兌換 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &srECROb->srTransData.szField_20[0], 1);	/* (1=條碼兌換，2=卡號兌換) */

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s :%s", "Award Redeem Mode", szTemplate);
					inLogPrintf(AT, szDebugMsg);
				}
				
				/* 條碼兌換 */
				if (memcmp(szTemplate, "1", 1) == 0)
				{
					memcpy(pobTran->szL3_AwardWay, "1", 1);
				}
				else
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "ECR取消兌換不接受卡號輸入");
					}
					memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
					inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "ECR取消兌換不接受卡號輸入");
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
					
					return (VS_ERROR);
				}
				
				/* 兌換方式為條碼才抓條碼*/
				if (memcmp(pobTran->szL3_AwardWay, "1", strlen("1")) == 0)
				{
					if (srECROb->srTransData.szBarCode1[0] == ' ' && srECROb->srTransData.szBarCode2[0] == ' ')
					{
						if (ginDebug == VS_TRUE)
						{
							inLogPrintf(AT, "ECR未輸入條碼");
						}
						memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
						inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
						memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
						sprintf(pobTran->szErrorMsgBuff2, "ECR未輸入條碼");
						pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
						
						return (VS_ERROR);
					}
					
					/* 條碼兌換 防呆&阻擋&存Barcode */
					/* 11 表示只有一個一維條碼，且這是第1 個。 */
					if (srECROb->srTransData.szBarCode1[0] == '1' && srECROb->srTransData.szBarCode1[1] == '1')
					{
						memcpy(&pobTran->szL3_Barcode1[0], &srECROb->srTransData.szBarCode1[0], 20);
						sprintf(pobTran->szL3_Barcode1Len, "%02d", strlen(pobTran->szL3_Barcode1));
					}
					else if (srECROb->srTransData.szBarCode2[0] == '1' && srECROb->srTransData.szBarCode2[1] == '1')
					{
						memcpy(&pobTran->szL3_Barcode1[0], &srECROb->srTransData.szBarCode2[0], 20);
						sprintf(pobTran->szL3_Barcode1Len, "%02d", strlen(pobTran->szL3_Barcode1));
					}
					else if (srECROb->srTransData.szBarCode1[0] == '2' && srECROb->srTransData.szBarCode1[1] == '1')
					{
						if (srECROb->srTransData.szBarCode2[0] == '2' && srECROb->srTransData.szBarCode2[1] == '2')
						{
							memcpy(&pobTran->szL3_Barcode1[0], &srECROb->srTransData.szBarCode1[0], 20);
							sprintf(pobTran->szL3_Barcode1Len, "%02d", strlen(pobTran->szL3_Barcode1));
							memcpy(&pobTran->szL3_Barcode2[0], &srECROb->srTransData.szBarCode2[0], 20);
							sprintf(pobTran->szL3_Barcode2Len, "%02d", strlen(pobTran->szL3_Barcode2));
						}
						else
						{
							if (ginDebug == VS_TRUE)
							{
								inLogPrintf(AT, "ECR條碼邏輯錯誤");
							}
							memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
							inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
							memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
							sprintf(pobTran->szErrorMsgBuff2, "ECR條碼邏輯錯誤");
							pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
							
							return (VS_ERROR);
						}
					}
					else if (srECROb->srTransData.szBarCode1[0] == '2' && srECROb->srTransData.szBarCode1[1] == '2')
					{
						if (srECROb->srTransData.szBarCode2[0] == '2' && srECROb->srTransData.szBarCode2[1] == '1')
						{
							memcpy(&pobTran->szL3_Barcode2[0], &srECROb->srTransData.szBarCode1[0], 20);
							sprintf(pobTran->szL3_Barcode2Len, "%02d", strlen(pobTran->szL3_Barcode2));
							memcpy(&pobTran->szL3_Barcode1[0], &srECROb->srTransData.szBarCode2[0], 20);
							sprintf(pobTran->szL3_Barcode1Len, "%02d", strlen(pobTran->szL3_Barcode1));
						}
						else
						{
							if (ginDebug == VS_TRUE)
							{
								inLogPrintf(AT, "ECR條碼邏輯錯誤");
							}
							memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
							inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
							memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
							sprintf(pobTran->szErrorMsgBuff2, "ECR條碼邏輯錯誤");
							pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
							
							return (VS_ERROR);
						}
					}
					else
					{
						if (ginDebug == VS_TRUE)
						{
							inLogPrintf(AT, "ECR條碼邏輯錯誤");
						}
						memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
						inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
						memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
						sprintf(pobTran->szErrorMsgBuff2, "ECR條碼邏輯錯誤");
						pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
							
						return (VS_ERROR);
					}
				}
				break;
			default:
				break;
		}
	}
	else
	{
		/* 實際支付金額RDM Paid Amt (含小數 2位) */
		switch (inTransType)
		{
			case _ECR_8N1_REDEEM_REFUND_NO_:		/* 紅利退貨 */
			case _ECR_8N1_REDEEM_ADJUST_NO_:		/* 紅利調帳 */
			case _ECR_8N1_HG_REWARD_REDEMPTION_NO_ :	/* 快樂購紅利積點 + 信用卡紅利扣抵 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &srECROb->srTransData.szRDMPaidAmt[0], 10);	/* 12位數，這裡只取10位 */
                                
                                /* 支付金額比交易金額大 */
                                if (atol(szTemplate) > pobTran->srBRec.lnTxnAmount)
                                {
                                        memset(srECROb->srTransData.szRDMPaidAmt, 0x00, sizeof(srECROb->srTransData.szRDMPaidAmt));
                                        memcpy(srECROb->srTransData.szRDMPaidAmt, "000000000000", 12);	/* 重輸 */
                                        
                                        memset(szTemplate, 0x00, sizeof(szTemplate));
                                        memcpy(szTemplate, &srECROb->srTransData.szRDMPaidAmt[0], 10);	/* 12位數，這裡只取10位 */
                                }

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s :%s", "RDM Paid Amt", szTemplate);
					inLogPrintf(AT, szDebugMsg);
				}

				pobTran->srBRec.lnRedemptionPaidCreditAmount = atol(szTemplate);

				break;
			default:
				break;
		}

		/* 信用卡紅利扣抵點數 RDM Point */
		switch (inTransType)
		{
			case _ECR_8N1_REDEEM_REFUND_NO_:		/* 紅利退貨 */
			case _ECR_8N1_REDEEM_ADJUST_NO_:		/* 紅利調帳 */
			case _ECR_8N1_HG_REWARD_REDEMPTION_NO_ :	/* 快樂購紅利積點 + 信用卡紅利扣抵 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &srECROb->srTransData.szRDMPoint[0], 8);

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s :%s", "RDM Point", szTemplate);
					inLogPrintf(AT, szDebugMsg);
				}

				pobTran->srBRec.lnRedemptionPoints = atol(szTemplate);

				break;
			default:
				break;
		}

		/* 分期期數 Installment Period */
		switch (inTransType)
		{
			case _ECR_8N1_INSTALLMENT_NO_:			/* 分期 */
			case _ECR_8N1_INSTALLMENT_REFUND_NO_:		/* 分期退貨 */
			case _ECR_8N1_INSTALLMENT_ADJUST_NO_:		/* 分期調帳 */
			case _ECR_8N1_HG_REWARD_INSTALLMENT_NO_ :	/* 快樂購紅利積點 + 信用卡分期付款 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &srECROb->srTransData.szInstallmentPeriod[0], 2);

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s :%s", "Installment Period", szTemplate);
					inLogPrintf(AT, szDebugMsg);
				}

				pobTran->srBRec.lnInstallmentPeriod = atol(szTemplate);

				break;
			default:
				break;
		}

		/* 首期金額 Down Payment Amount (含小數 2位) */
		switch (inTransType)
		{
			case _ECR_8N1_INSTALLMENT_REFUND_NO_:		/* 分期退貨 */
			case _ECR_8N1_INSTALLMENT_ADJUST_NO_:		/* 分期調帳 */
			case _ECR_8N1_HG_REWARD_INSTALLMENT_NO_ :	/* 快樂購紅利積點 + 信用卡分期付款 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &srECROb->srTransData.szField_26[0], 10);	/* 12位數，這裡只取10位 */

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s :%s", "Down Payment Amount", szTemplate);
					inLogPrintf(AT, szDebugMsg);
				}

				pobTran->srBRec.lnInstallmentDownPayment = atol(szTemplate);

				break;
			default:
				break;
		}

		/* 每期金額 Installment Payment Amount (含小數 2位) */
		switch (inTransType)
		{
			case _ECR_8N1_INSTALLMENT_REFUND_NO_:		/* 分期退貨 */
			case _ECR_8N1_INSTALLMENT_ADJUST_NO_:		/* 分期調帳 */
			case _ECR_8N1_HG_REWARD_INSTALLMENT_NO_ :	/* 快樂購紅利積點 + 信用卡分期付款 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &srECROb->srTransData.szField_27[0], 10);	/* 12位數，這裡只取10位 */

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s :%s", "Installment Payment Amount", szTemplate);
					inLogPrintf(AT, szDebugMsg);
				}

				pobTran->srBRec.lnInstallmentPayment = atol(szTemplate);

				break;
			default:
				break;
		}

		/* 分期手續費 Formallity Fee (含小數 2位) */
		switch (inTransType)
		{
			case _ECR_8N1_INSTALLMENT_REFUND_NO_:		/* 分期退貨 */
			case _ECR_8N1_INSTALLMENT_ADJUST_NO_:		/* 分期調帳 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &srECROb->srTransData.szField_28[0], 10);	/* 12位數，這裡只取10位 */

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s :%s", "Formallity Fee", szTemplate);
					inLogPrintf(AT, szDebugMsg);
				}
				pobTran->srBRec.lnInstallmentFormalityFee = atol(szTemplate);
				
				if (pobTran->srBRec.lnInstallmentFormalityFee != 0)
				{
					/* 分期退貨不能輸入手續費 */
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_INST_FEE_NOT_0_;
					pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;		/* 0004 操作錯誤 */
					return (VS_ERROR);
				}
				
				break;
			case _ECR_8N1_HG_REWARD_INSTALLMENT_NO_ :	/* 快樂購紅利積點 + 信用卡分期付款 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &srECROb->srTransData.szField_28[0], 10);	/* 12位數，這裡只取10位 */

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s :%s", "Formallity Fee", szTemplate);
					inLogPrintf(AT, szDebugMsg);
				}

				pobTran->srBRec.lnInstallmentFormalityFee = atol(szTemplate);

				break;
			default:
				break;
		}

		/* 啟動卡號查詢 Start Trans Type */
		switch (inTransType)
		{
			case _ECR_8N1_START_CARD_NO_INQUIRY_NO_:		/* 卡號查詢 */	
				
				break;
			default:
				break;
		}

		/* 只有金融卡才要收的欄位 */
		if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
		{
			/* 金融卡原交易日期 SP Origin Date */
			switch (inTransType)
			{
				case _ECR_8N1_REFUND_NO_:				/* 退費 */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memcpy(szTemplate, &srECROb->srTransData.szField_34[0], 8);

					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s :%s", "SP Origin Date", szTemplate);
						inLogPrintf(AT, szDebugMsg);
					}

					if (atoi(szTemplate) > 0	&&
					    inFunc_CheckValidDate_Include_Year(szTemplate) == VS_SUCCESS)
					{
						memcpy(pobTran->srBRec.szFiscRefundDate, szTemplate, 8);
					}
					
					break;
				default:
					break;
			}

			/* 金融卡調單編號 SP RRN */
			switch (inTransType)
			{
				case _ECR_8N1_REFUND_NO_:				/* 退費 */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memcpy(szTemplate, &srECROb->srTransData.szField_35[0], 12);

					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s :%s", "SP RRN", szTemplate);
						inLogPrintf(AT, szDebugMsg);
					}

					if (atoi(szTemplate) > 0)
					{
						memcpy(pobTran->srBRec.szFiscRRN, szTemplate, 12);
					}

					break;
				default:
					break;
			}

		}
		/* 只有電票要收的欄位 */
		else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
		{
			/* ESVC Origin Date */
			switch (inTransType)
			{
				case _ECR_8N1_REFUND_NO_:				/* 電票退貨 */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memcpy(szTemplate, &srECROb->srTransData.szField_34[0], 8);

					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s :%s", "ESVC Origin Date", szTemplate);
						inLogPrintf(AT, szDebugMsg);
					}

					if (atoi(szTemplate) > 0	&&
					    inFunc_CheckValidDate_Include_Year(szTemplate) == VS_SUCCESS)
					{
						memcpy(pobTran->srTRec.szTicketRefundDate, &szTemplate[4], 4);
					}
					else
					{
/* [20251219_BUG_MDF][ECR][DATA] 修改ECR欄位34資料,因電票資料如果為空白會拒絕，跳過檢查步驟,進行後續人工輸入 */
#ifdef _ECR_CHECK_FILE_34_	
						/* 接收資料錯誤 */
						memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
						sprintf(pobTran->szErrorMsgBuff1, "_ECR_8N1_REFUND_NO_");
						memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
						sprintf(pobTran->szErrorMsgBuff2, "ESVC Origin Date");
						pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
						
						return (VS_ERROR);
#endif
					}

					break;
				default:
					break;
			}
			
			/* ATS電票交易序號 RF NUMBER 左靠右補空白 */
			switch (inTransType)
			{
				case _ECR_8N1_REFUND_NO_:				/* 電票退貨 */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memcpy(szTemplate, &srECROb->srTransData.szField_35[0], 12);

					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s :%s", "RF NUMBER", szTemplate);
						inLogPrintf(AT, szDebugMsg);
					}

					if (atoi(szTemplate) > 0)
					{
						memcpy(pobTran->srTRec.szTicketRefundCode, szTemplate, 12);
					}
					else
					{
						/* 接收資料錯誤 */
						memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
						sprintf(pobTran->szErrorMsgBuff1, "_ECR_8N1_REFUND_NO_");
						memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
						sprintf(pobTran->szErrorMsgBuff2, "ESVC RF NUMBER");
						pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
						
						return (VS_ERROR);
					}

					break;
				default:
					break;
			}
		}
		/* 只有銀聯要收的欄位 */
		else if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
		{
			/* CUP Origin Date */
			switch (inTransType)
			{
				case _ECR_8N1_REFUND_NO_:				/* 電票退貨 */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memcpy(szTemplate, &srECROb->srTransData.szField_34[0], 8);

					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s :%s", "CUP Origin Date", szTemplate);
						inLogPrintf(AT, szDebugMsg);
					}

					if (atoi(szTemplate) > 0	&&
					    inFunc_CheckValidDate_Include_Year(szTemplate) == VS_SUCCESS)
					{
						memcpy(pobTran->srBRec.szCUP_TD, &szTemplate[4], 4);
					}

					break;
				default:
					break;
			}
		}
		
	}
	
	/* 【需求單 - 104014】發卡機構on-us繳費平台需求 +【需求單 - 104152】發卡機構on-us繳費平台 Payitem Code by Russell 2020/3/16 下午 5:38 */
	memset(szPayItemFuncEnable, 0x00, sizeof(szPayItemFuncEnable));
	inGetPayItemEnable(szPayItemFuncEnable);
	if (memcmp(szPayItemFuncEnable, "Y", 1) == 0							&&
	    memcmp(&srECROb->srTransData.szTransType[0], _ECR_8N1_START_CARD_NO_INQUIRY_, 2) != 0)
	{
		/* PayItemFuncEnable有開才進入檢核收銀機送的繳費項目 */
		/* 兩段式收銀機連線 第一段不檢核Payitem Code */
		switch (inTransType)
		{
			case _ECR_8N1_SALE_NO_ :
			case _ECR_8N1_REFUND_NO_ :
			case _ECR_8N1_OFFLINE_NO_ :
			case _ECR_8N1_PREAUTH_NO_ :
			case _ECR_8N1_PREAUTH_COMPLETE_NO_ :
			case _ECR_8N1_INSTALLMENT_NO_ :
			case _ECR_8N1_INSTALLMENT_REFUND_NO_ :
			case _ECR_8N1_REDEEM_NO_ :
			case _ECR_8N1_REDEEM_REFUND_NO_ :
				srECROb->srTransData.uszECRResponsePayitem = VS_TRUE;
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, srECROb->srTransData.szPayItem, 5);

				/* 先判斷是不是送5個空白 */
				if (!memcmp(szTemplate, "     ", 5))
				{
					/* 空白不必回傳payitem */
					if (ginDebug == VS_TRUE)
					{
						srECROb->srTransData.uszECRResponsePayitem = VS_FALSE;
						inLogPrintf(AT, "ECR not input PayItem");
					}
					break;
				}

				/* 比對端末機的PIT Table PayItemCode */
				/* 吳升文(Angus Wu) 2015-09-15 (週二) 下午 04:43 回覆 帶5個0視為有值，所以要比對。 */
				for (i = 0 ;; i++)
				{
					/* 吳升文(Angus Wu) 2015-09-15 (週二) 下午 04:24 回覆
						因為繳費項目的代碼會從授權一直帶到清算再回到發卡行，然後發卡行根據繳費項目代碼跟持卡人收費。
						所以不合法的繳費代碼端末機不可以後送，免得後端勾稽不到而收不到錢。
						因此比對不到繳費代碼則端末機提示”不支援該繳費項目”後，回傳Response Code=0004。
					*/
					/* 比對不到不進入輸入keymap畫面 在ECR連線阻檔 */
					if (inLoadPITRec(i) < 0)
					{
						/* 不支援該繳費項目 */
						pobTran->inErrorMsg = _ERROR_CODE_V3_NOT_SUPPORT_THIS_PAY_ITEM_;
						pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
						
						return (VS_ERROR);
					}

					/* 有比對到端末機的繳費項目 */
					memset(szPayItemCodeTemp, 0x00, sizeof(szPayItemCodeTemp));
					inGetPayItemCode(szPayItemCodeTemp);
					if (memcmp(szTemplate, szPayItemCodeTemp, 5) == 0)
					{
						memset(pobTran->srBRec.szPayItemCode, 0x00, sizeof(pobTran->srBRec.szPayItemCode));
						memcpy(&pobTran->srBRec.szPayItemCode[0], &szTemplate[0], 5);
						break;
					}
				}

				break;
			default :
				break;
		}
	}
	
	/* 處理HG */
	/* 支付工具 */
	switch (inTransType)
	{
		case _ECR_8N1_HG_REWARD_SALE_NO_ :
		case _ECR_8N1_HG_ONLINE_REDEEMPTION_NO_ :
		case _ECR_8N1_HG_POINT_CERTAIN_NO_ :
		case _ECR_8N1_HG_REWARD_INSTALLMENT_NO_ :
		case _ECR_8N1_HG_REWARD_REDEMPTION_NO_ :
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "HG Payment Tool", srECROb->srTransData.szHGPaymentTool);
				inLogPrintf(AT, szDebugMsg);
			}
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(srECROb->srTransData.szHGPaymentTool) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "HG Payment Tool", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "HG Payment Tool Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

				return (VS_ERROR);
			}
			
			if (memcmp(srECROb->srTransData.szHGPaymentTool, _ECR_8N1_HG_PAYMENT_TOOL_TYPE_EDC_CHOOSE_, 2) == 0)
			{
				pobTran->srBRec.lnHGPaymentType = 0;
			}
			else if (memcmp(srECROb->srTransData.szHGPaymentTool, _ECR_8N1_HG_PAYMENT_TOOL_TYPE_CREDIT_CARD_, 2) == 0)
			{
				pobTran->srBRec.lnHGPaymentType = _HG_PAY_CREDIT_;
			}
			else if (memcmp(srECROb->srTransData.szHGPaymentTool, _ECR_8N1_HG_PAYMENT_TOOL_TYPE_CASH_, 2) == 0)
			{
				pobTran->srBRec.lnHGPaymentType = _HG_PAY_CASH_;
			}
			else if (memcmp(srECROb->srTransData.szHGPaymentTool, _ECR_8N1_HG_PAYMENT_TOOL_TYPE_GIFT_PAPER_, 2) == 0)
			{
				pobTran->srBRec.lnHGPaymentType = _HG_PAY_GIFT_PAPER_;
			}
			else if (memcmp(srECROb->srTransData.szHGPaymentTool, _ECR_8N1_HG_PAYMENT_TOOL_TYPE_HGI_, 2) == 0)
			{
				pobTran->srBRec.lnHGPaymentType = _HG_PAY_CREDIT_INSIDE_;
			}
			else if (memcmp(srECROb->srTransData.szHGPaymentTool, _ECR_8N1_HG_PAYMENT_TOOL_TYPE_CUP_CARD_, 2) == 0)
			{
				pobTran->srBRec.lnHGPaymentType = _HG_PAY_CUP_;
			}
			else
			{
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "HG Payment Tool Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
			}
			break;
		default :
			break;
	}
	
	/* Happy Go 扣抵點數 */
	switch (inTransType)
	{
		case _ECR_8N1_HG_POINT_CERTAIN_NO_ :
		case _ECR_8N1_HG_FULL_REDEEMPTION_NO_ :
		case _ECR_8N1_HG_REDEEM_REFUND_NO_ :
		case _ECR_8N1_HG_REWARD_REFUND_NO_ :
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "HG Redeem Point", srECROb->srTransData.szHGRedeemPoint);
				inLogPrintf(AT, szDebugMsg);
			}
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(srECROb->srTransData.szHGRedeemPoint) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "HG Redeem Point", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "HG Redeem Point Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

				return (VS_ERROR);
			}
			
			pobTran->srBRec.lnHGTransactionPoint = atol(srECROb->srTransData.szHGRedeemPoint);
			break;
		default :
			break;
	}
	
	if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
	{
		/* 有開CUP且MACEnable有開但安全認證沒過，不能執行CUP交易 */
		if (inKMS_CheckKey(_TWK_KEYSET_NCCC_, _TWK_KEYINDEX_NCCC_MAC_) != VS_SUCCESS)
		{
			if (inNCCC_Func_CUP_PowerOn_LogOn(pobTran) != VS_SUCCESS)
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "安全認證失敗，不能使用此功能");
				}
				/* 安全認證失敗 */
				pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
				
				return (VS_ERROR);
			}
		}
	}
	
	if (inECR_Check_Exception(pobTran) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	memset(szFiscContactlessEnable, 0x00, sizeof(szFiscContactlessEnable));
	inGetSmartPayContactlessEnable(szFiscContactlessEnable);
	memset(szCTLSEnable, 0x00, sizeof(szCTLSEnable));
	inGetContactlessEnable(szCTLSEnable);
	memset(szCUPContactlessEnable, 0x00, sizeof(szCUPContactlessEnable));
	inGetCUPContactlessEnable(szCUPContactlessEnable);
	
	/* 跑OPT */
	switch (inTransType)
	{
		case _ECR_8N1_SALE_NO_:
			/* 金融卡和一般Sale跑不同OPT，所以要分 */
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				/* 第三層顯示 ＜消費扣款＞ */
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_FISC_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜消費扣款＞ */

				pobTran->inFunctionID = _FISC_SALE_;
				if (!memcmp(szCTLSEnable, "Y", 1)		&&
				    !memcmp(szFiscContactlessEnable, "Y", 1)	&&
				    guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_FISC_SALE_CTLS_;
					pobTran->inRunTRTID = _TRT_FISC_SALE_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_FISC_SALE_;
					pobTran->inRunTRTID = _TRT_FISC_SALE_ICC_;
				}
		
				pobTran->inTransactionCode = _FISC_SALE_;
				pobTran->srBRec.inCode = _FISC_SALE_;
				pobTran->srBRec.inOrgCode = _FISC_SALE_;
                                
                                /* 卡號查詢不過第二次卡，所以要直接決定TRT */
                                if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
                                {
                                        inNCCC_Func_Decide_CTLS_TRT(pobTran);
                                }
                                else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
                                {
                                        inNCCC_Func_Decide_ICC_TRT(pobTran);
                                }
                                else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
                                {
                                        inNCCC_Func_Decide_MEG_TRT(pobTran);
                                }

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_CUP_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜銀聯一般交易＞ */

				if (!memcmp(szCTLSEnable, "Y", 1)		&&
				    !memcmp(szCUPContactlessEnable, "Y", 1)	&&
				    guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
					pobTran->inRunTRTID = _TRT_CUP_SALE_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
					pobTran->inRunTRTID = _TRT_CUP_SALE_;
				}

				pobTran->inFunctionID = _CUP_SALE_;
				pobTran->inTransactionCode = _CUP_SALE_;
				pobTran->srBRec.inCode = pobTran->inTransactionCode;
				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;

                                /* 卡號查詢不過第二次卡，所以要直接決定TRT */
                                if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
                                {
                                        inNCCC_Func_Decide_CTLS_TRT(pobTran);
                                }
                                else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
                                {
                                        inNCCC_Func_Decide_ICC_TRT(pobTran);
                                }
                                else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
                                {
                                        inNCCC_Func_Decide_MEG_TRT(pobTran);
                                }
                                
				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_TICKET_DEDUCT_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜購貨交易＞ */

				pobTran->inFunctionID = _TICKET_DEDUCT_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					/* (需求單 - 107227)邦柏科技自助作業客製化 悠遊卡0018重試功能 by Russell 2018/12/14 上午 9:30 */
					if (pobTran->uszLastTranscationBit == VS_TRUE)
					{
						pobTran->inRunOperationID = _OPERATION_TICKET_RETRY_;
					}
					else
					{
						pobTran->inRunOperationID = _OPERATION_TICKET_DEDUCT_;
					}
				}
				else
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "感應未初始化，不能使用此功能");
					}
					return (VS_ERROR);
				}

				pobTran->inTransactionCode = _TICKET_DEDUCT_;
				pobTran->srTRec.inCode = _TICKET_DEDUCT_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else
			{
				inDISP_ClearAll();
				/* 第三層顯示 ＜一般交易＞ */
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般交易＞ */

				pobTran->inFunctionID = _SALE_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
					pobTran->inRunTRTID = _TRT_SALE_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
					pobTran->inRunTRTID = _TRT_SALE_;
				}

				pobTran->inTransactionCode = _SALE_;
				pobTran->srBRec.inCode = _SALE_;
				pobTran->srBRec.inOrgCode = _SALE_;
                                
                                /* 卡號查詢不過第二次卡，所以要直接決定TRT */
                                if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
                                {
                                        inNCCC_Func_Decide_CTLS_TRT(pobTran);
                                }
                                else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
                                {
                                        inNCCC_Func_Decide_ICC_TRT(pobTran);
                                }
                                else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
                                {
                                        inNCCC_Func_Decide_MEG_TRT(pobTran);
                                }

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			
			break;
		case _ECR_8N1_VOID_NO_:
			/* 金融卡和一般Sale跑不同OPT，所以要分 */
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_FISC_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜沖正交易＞ */

				pobTran->inFunctionID = _FISC_VOID_;
				pobTran->inRunOperationID = _OPERATION_VOID_;
				pobTran->inRunTRTID = _TRT_FISC_VOID_;

				pobTran->inTransactionCode = _FISC_VOID_;
				pobTran->srBRec.inCode = _FISC_VOID_;
				pobTran->srBRec.inOrgCode = _FISC_VOID_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜取消交易＞ */

				pobTran->inRunOperationID = _OPERATION_VOID_;
				pobTran->inRunTRTID = _TRT_CUP_VOID_;

				pobTran->inFunctionID = _CUP_VOID_;
				pobTran->inTransactionCode = _CUP_VOID_;
				pobTran->srBRec.inCode = pobTran->inTransactionCode;
				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜取消交易＞ */

				pobTran->inFunctionID = _VOID_;
				pobTran->inRunOperationID = _OPERATION_VOID_;
				pobTran->inRunTRTID = _TRT_VOID_;

				pobTran->inTransactionCode = _VOID_;
				pobTran->srBRec.inCode = _VOID_;
				pobTran->srBRec.inOrgCode = _VOID_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			break;
		case _ECR_8N1_REFUND_NO_:
			/* 金融卡和一般Sale跑不同OPT，所以要分 */
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_FISC_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);	/* 第二層顯示 ＜退費交易＞ */

				pobTran->inFunctionID = _FISC_REFUND_;                    
				if (!memcmp(szCTLSEnable, "Y", 1)		&&
				    !memcmp(szFiscContactlessEnable, "Y", 1)	&&
				    guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_FISC_REFUND_CTLS_;
					pobTran->inRunTRTID = _TRT_FISC_REFUND_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_FISC_REFUND_;
					pobTran->inRunTRTID = _TRT_FISC_REFUND_ICC_;
				}

				pobTran->inTransactionCode = _FISC_REFUND_;
				pobTran->srBRec.inCode = _FISC_REFUND_;
				pobTran->srBRec.inOrgCode = _FISC_REFUND_;
                                
                                /* 卡號查詢不過第二次卡，所以要直接決定TRT */
                                if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
                                {
                                        inNCCC_Func_Decide_CTLS_TRT(pobTran);
                                }
                                else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
                                {
                                        inNCCC_Func_Decide_ICC_TRT(pobTran);
                                }
                                else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
                                {
                                        inNCCC_Func_Decide_MEG_TRT(pobTran);
                                }

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			else if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般退貨＞ */

				if (!memcmp(szCTLSEnable, "Y", 1)		&&
				    !memcmp(szCUPContactlessEnable, "Y", 1)	&&
				    guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_GET_CARD_FIRST_;
					pobTran->inRunTRTID = _TRT_CUP_REFUND_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_AMOUNT_FIRST_CUP_;
					pobTran->inRunTRTID = _TRT_CUP_REFUND_;
				}

				pobTran->inFunctionID = _CUP_REFUND_;
				pobTran->inTransactionCode = _CUP_REFUND_;
				pobTran->srBRec.inCode = pobTran->inTransactionCode;
				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
                                
                                /* 卡號查詢不過第二次卡，所以要直接決定TRT */
                                if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
                                {
                                        inNCCC_Func_Decide_CTLS_TRT(pobTran);
                                }
                                else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
                                {
                                        inNCCC_Func_Decide_ICC_TRT(pobTran);
                                }
                                else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
                                {
                                        inNCCC_Func_Decide_MEG_TRT(pobTran);
                                }

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_TICKET_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜退貨交易＞ */

				pobTran->inFunctionID = _TICKET_REFUND_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					/* (需求單 - 107227)邦柏科技自助作業客製化 悠遊卡0018重試功能 by Russell 2018/12/14 上午 9:30 */
					if (pobTran->uszLastTranscationBit == VS_TRUE)
					{
						pobTran->inRunOperationID = _OPERATION_TICKET_RETRY_;
					}
					else
					{
						pobTran->inRunOperationID = _OPERATION_TICKET_REFUND_;
					}
				}
				else
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "感應未初始化，不能使用此功能");
					}
					return (VS_ERROR);
				}

				pobTran->inTransactionCode = _TICKET_REFUND_;
				pobTran->srTRec.inCode = _TICKET_REFUND_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般退貨＞ */

				pobTran->inFunctionID = _REFUND_;

				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_GET_CARD_FIRST_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_AMOUNT_FIRST_;
				}
				pobTran->inRunTRTID = _TRT_REFUND_;

				pobTran->inTransactionCode = _REFUND_;
				pobTran->srBRec.inCode = _REFUND_;
				pobTran->srBRec.inOrgCode = _REFUND_;
                                
                                /* 卡號查詢不過第二次卡，所以要直接決定TRT */
                                if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
                                {
                                        inNCCC_Func_Decide_CTLS_TRT(pobTran);
                                }
                                else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
                                {
                                        inNCCC_Func_Decide_ICC_TRT(pobTran);
                                }
                                else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
                                {
                                        inNCCC_Func_Decide_MEG_TRT(pobTran);
                                }

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;
			
		case _ECR_8N1_OFFLINE_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_SALE_OFFLINE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜交易補登＞ */
			
			pobTran->inFunctionID = _SALE_OFFLINE_;
			pobTran->inRunOperationID = _OPERATION_SALE_OFFLINE_;
			pobTran->inRunTRTID = _TRT_SALE_OFFLINE_;

			pobTran->inTransactionCode = _SALE_OFFLINE_;
			pobTran->srBRec.inCode = _SALE_OFFLINE_;
			pobTran->srBRec.inOrgCode = _SALE_OFFLINE_;
                        
                        /* 卡號查詢不過第二次卡，所以要直接決定TRT */
			if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
			{
				inNCCC_Func_Decide_CTLS_TRT(pobTran);
			}
			else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
			{
				inNCCC_Func_Decide_ICC_TRT(pobTran);
			}
			else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
			{
				inNCCC_Func_Decide_MEG_TRT(pobTran);
			}
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			break;
			
		case _ECR_8N1_PREAUTH_NO_:
			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_PRE_AUTH_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜預先授權＞ */

				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_PRE_AUTH_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_PRE_AUTH_;
				}
				pobTran->inRunTRTID = _TRT_PRE_AUTH_;

				pobTran->inFunctionID = _CUP_PRE_AUTH_;
				pobTran->inTransactionCode = _CUP_PRE_AUTH_;
				pobTran->srBRec.inCode = pobTran->inTransactionCode;
				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
                                
                                /* 卡號查詢不過第二次卡，所以要直接決定TRT */
                                if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
                                {
                                        inNCCC_Func_Decide_CTLS_TRT(pobTran);
                                }
                                else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
                                {
                                        inNCCC_Func_Decide_ICC_TRT(pobTran);
                                }
                                else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
                                {
                                        inNCCC_Func_Decide_MEG_TRT(pobTran);
                                }

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_PRE_AUTH_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜預先授權＞ */

				pobTran->inFunctionID = _PRE_AUTH_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_PRE_AUTH_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_PRE_AUTH_;
				}
				pobTran->inRunTRTID = _TRT_PRE_AUTH_;

				pobTran->inTransactionCode = _PRE_AUTH_;
				pobTran->srBRec.inCode = _PRE_AUTH_;
				pobTran->srBRec.inOrgCode = _PRE_AUTH_;
                                
                                /* 卡號查詢不過第二次卡，所以要直接決定TRT */
                                if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
                                {
                                        inNCCC_Func_Decide_CTLS_TRT(pobTran);
                                }
                                else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
                                {
                                        inNCCC_Func_Decide_ICC_TRT(pobTran);
                                }
                                else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
                                {
                                        inNCCC_Func_Decide_MEG_TRT(pobTran);
                                }

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			
			break;
			
		case _ECR_8N1_PREAUTH_CANCEL_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_PRE_AUTH_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜預先授權取消＞ */
			
			pobTran->inFunctionID = _CUP_PRE_AUTH_VOID_;
			pobTran->inRunOperationID = _OPERATION_VOID_;
			pobTran->inRunTRTID = _TRT_CUP_PRE_AUTH_VOID_;

			pobTran->inTransactionCode = _CUP_PRE_AUTH_VOID_;
			pobTran->srBRec.inCode = _CUP_PRE_AUTH_VOID_;
			pobTran->srBRec.inOrgCode = _CUP_PRE_AUTH_VOID_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			break;
			
		case _ECR_8N1_PREAUTH_COMPLETE_NO_:
			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_PRE_COMP_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜預先授權完成＞ */

				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_PRE_COMP_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_PRE_COMP_;
				}
				pobTran->inRunTRTID = _TRT_PRE_COMP_;

				pobTran->inFunctionID = _CUP_PRE_COMP_;
				pobTran->inTransactionCode = _CUP_PRE_COMP_;
				pobTran->srBRec.inCode = pobTran->inTransactionCode;
				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
                                
                                /* 卡號查詢不過第二次卡，所以要直接決定TRT */
                                if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
                                {
                                        inNCCC_Func_Decide_CTLS_TRT(pobTran);
                                }
                                else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
                                {
                                        inNCCC_Func_Decide_ICC_TRT(pobTran);
                                }
                                else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
                                {
                                        inNCCC_Func_Decide_MEG_TRT(pobTran);
                                }

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_PRE_COMP_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜預先授權完成＞ */

				pobTran->inFunctionID = _PRE_COMP_;
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_PRE_COMP_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_PRE_COMP_;
				}
				pobTran->inRunTRTID = _TRT_PRE_COMP_;

				pobTran->inTransactionCode = _PRE_COMP_;
				pobTran->srBRec.inCode = _PRE_COMP_;
				pobTran->srBRec.inOrgCode = _PRE_COMP_;
                                
                                /* 卡號查詢不過第二次卡，所以要直接決定TRT */
                                if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
                                {
                                        inNCCC_Func_Decide_CTLS_TRT(pobTran);
                                }
                                else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
                                {
                                        inNCCC_Func_Decide_ICC_TRT(pobTran);
                                }
                                else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
                                {
                                        inNCCC_Func_Decide_MEG_TRT(pobTran);
                                }

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;
			
		case _ECR_8N1_PREAUTH_COMPLETE_CANCEL_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);					/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_PRE_COMP_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜預先授權完成取消＞ */
			
			pobTran->inFunctionID = _CUP_PRE_COMP_VOID_;
			pobTran->inRunOperationID = _OPERATION_VOID_;
			pobTran->inRunTRTID = _TRT_CUP_PRE_COMP_VOID_;

			pobTran->inTransactionCode = _CUP_PRE_COMP_VOID_;
			pobTran->srBRec.inCode = _CUP_PRE_COMP_VOID_;
			pobTran->srBRec.inOrgCode = _CUP_PRE_COMP_VOID_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			break;
			
		case _ECR_8N1_INSTALLMENT_NO_:
//			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
//			{
//				inDISP_ClearAll();
//				/* 保留給銀聯分期使用 */
//				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
//				inDISP_PutGraphic(_MENU_INST_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜分期付款＞ */
//
//				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
//				{
//					pobTran->inRunOperationID = _OPERATION_INST_SALE_CTLS_;
//				}
//				else
//				{
//					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
//				}
//				pobTran->inRunTRTID = _TRT_INST_SALE_;
//
//				pobTran->inFunctionID = _CUP_INST_SALE_;
//				pobTran->inTransactionCode = _CUP_INST_SALE_;
//				pobTran->srBRec.inCode = pobTran->inTransactionCode;
//				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
//
//				pobTran->srBRec.uszInstallmentBit = VS_TRUE;
//                        
//                                /* 卡號查詢不過第二次卡，所以要直接決定TRT */
//                                if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
//                                {
//                                    inNCCC_Func_Decide_CTLS_TRT(pobTran);
//                                }
//                                else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
//                                {
//                                        inNCCC_Func_Decide_ICC_TRT(pobTran);
//                                }
//                                else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
//                                {
//                                        inNCCC_Func_Decide_MEG_TRT(pobTran);
//                                }
//
//				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
//			}
//			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_INST_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜分期付款＞ */

				pobTran->inFunctionID = _INST_SALE_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_INST_SALE_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
				}
				pobTran->inRunTRTID = _TRT_INST_SALE_;

				pobTran->inTransactionCode = _INST_SALE_;
				pobTran->srBRec.inCode = _INST_SALE_;
				pobTran->srBRec.inOrgCode = _INST_SALE_;

				pobTran->srBRec.uszInstallmentBit = VS_TRUE;
                                /* 卡號查詢不過第二次卡，所以要直接決定TRT */
                                if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
                                {
                                        inNCCC_Func_Decide_CTLS_TRT(pobTran);
                                }
                                else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
                                {
                                        inNCCC_Func_Decide_ICC_TRT(pobTran);
                                }
                                else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
                                {
                                        inNCCC_Func_Decide_MEG_TRT(pobTran);
                                }

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;
			
		case _ECR_8N1_INSTALLMENT_REFUND_NO_:
//			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
//			{
//				inDISP_ClearAll();
//				/* 保留給銀聯分期退貨使用 */
//				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
//				inDISP_PutGraphic(_MENU_INST_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜分期退貨＞ */
//
//				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
//				{
//					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_GET_CARD_FIRST_;
//				}
//				else
//				{
//					pobTran->inRunOperationID = _OPERATION_REFUND_;
//				}
//				pobTran->inRunTRTID = _TRT_INST_REFUND_;
//
//				pobTran->inFunctionID = _CUP_INST_REFUND_;
//				pobTran->inTransactionCode = _CUP_INST_REFUND_;
//				pobTran->srBRec.inCode = pobTran->inTransactionCode;
//				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
//
//				pobTran->srBRec.uszInstallmentBit = VS_TRUE;
//                                /* 卡號查詢不過第二次卡，所以要直接決定TRT */
//                                if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
//                                {
//                                        inNCCC_Func_Decide_CTLS_TRT(pobTran);
//                                }
//                                else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
//                                {
//                                        inNCCC_Func_Decide_ICC_TRT(pobTran);
//                                }
//                                else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
//                                {
//                                        inNCCC_Func_Decide_MEG_TRT(pobTran);
//                                }
//
//				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
//			}
//			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_INST_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜分期退貨＞ */

				pobTran->inFunctionID = _INST_REFUND_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_GET_CARD_FIRST_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_;
				}
				pobTran->inRunTRTID = _TRT_INST_REFUND_;

				pobTran->inTransactionCode = _INST_REFUND_;
				pobTran->srBRec.inCode = _INST_REFUND_;
				pobTran->srBRec.inOrgCode = _INST_REFUND_;

				pobTran->srBRec.uszInstallmentBit = VS_TRUE;
                                /* 卡號查詢不過第二次卡，所以要直接決定TRT */
                                if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
                                {
                                        inNCCC_Func_Decide_CTLS_TRT(pobTran);
                                }
                                else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
                                {
                                        inNCCC_Func_Decide_ICC_TRT(pobTran);
                                }
                                else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
                                {
                                        inNCCC_Func_Decide_MEG_TRT(pobTran);
                                }

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;
			
		case _ECR_8N1_INSTALLMENT_ADJUST_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_INST_ADJUST_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜分期調帳＞ */
			
			pobTran->inFunctionID = _INST_ADJUST_;
			pobTran->inRunOperationID = _OPERATION_INST_ADJUST_;
			pobTran->inRunTRTID = _TRT_INST_ADJUST_;

			pobTran->inTransactionCode = _INST_ADJUST_;
			pobTran->srBRec.inCode = _INST_ADJUST_;
			pobTran->srBRec.inOrgCode = _INST_ADJUST_;
			
			pobTran->srBRec.uszInstallmentBit = VS_TRUE;
                        /* 卡號查詢不過第二次卡，所以要直接決定TRT */
			if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
			{
				inNCCC_Func_Decide_CTLS_TRT(pobTran);
			}
			else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
			{
				inNCCC_Func_Decide_ICC_TRT(pobTran);
			}
			else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
			{
				inNCCC_Func_Decide_MEG_TRT(pobTran);
			}
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			break;
			
		case _ECR_8N1_REDEEM_NO_:
//			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
//			{
//				inDISP_ClearAll();
//				/* 保留給銀聯紅利扣抵使用 */
//				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
//				inDISP_PutGraphic(_MENU_REDEEM_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利扣抵＞ */
//
//				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
//				{
//					pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
//				}
//				else
//				{
//					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
//				}
//				pobTran->inRunTRTID = _TRT_REDEEM_SALE_;
//
//				pobTran->inFunctionID = _CUP_REDEEM_SALE_;
//				pobTran->inTransactionCode = _CUP_REDEEM_SALE_;
//				pobTran->srBRec.inCode = pobTran->inTransactionCode;
//				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
//
//				pobTran->srBRec.uszRedeemBit = VS_TRUE;
//                                /* 卡號查詢不過第二次卡，所以要直接決定TRT */
//                                if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
//                                {
//                                        inNCCC_Func_Decide_CTLS_TRT(pobTran);
//                                }
//                                else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
//                                {
//                                        inNCCC_Func_Decide_ICC_TRT(pobTran);
//                                }
//                                else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
//                                {
//                                        inNCCC_Func_Decide_MEG_TRT(pobTran);
//                                }
//
//				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
//			}
//			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REDEEM_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利扣抵＞ */

				pobTran->inFunctionID = _REDEEM_SALE_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
				}
				pobTran->inRunTRTID = _TRT_REDEEM_SALE_;

				pobTran->inTransactionCode = _REDEEM_SALE_;
				pobTran->srBRec.inCode = _REDEEM_SALE_;
				pobTran->srBRec.inOrgCode = _REDEEM_SALE_;

				pobTran->srBRec.uszRedeemBit = VS_TRUE;
                                /* 卡號查詢不過第二次卡，所以要直接決定TRT */
                                if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
                                {
                                        inNCCC_Func_Decide_CTLS_TRT(pobTran);
                                }
                                else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
                                {
                                        inNCCC_Func_Decide_ICC_TRT(pobTran);
                                }
                                else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
                                {
                                        inNCCC_Func_Decide_MEG_TRT(pobTran);
                                }

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;
		
		case _ECR_8N1_REDEEM_REFUND_NO_:
//			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
//			{
//				inDISP_ClearAll();
//				/* 保留給銀聯紅利退貨使用 */
//				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
//				inDISP_PutGraphic(_MENU_REDEEM_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利退貨＞ */
//
//				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
//				{
//					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_GET_CARD_FIRST_;
//				}
//				else
//				{
//					pobTran->inRunOperationID = _OPERATION_REFUND_;
//				}
//				pobTran->inRunTRTID = _TRT_REDEEM_REFUND_;
//
//				pobTran->inFunctionID = _CUP_REDEEM_REFUND_;
//				pobTran->inTransactionCode = CUP_REDEEM_REFUND_;
//				pobTran->srBRec.inCode = pobTran->inTransactionCode;
//				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
//
//				pobTran->srBRec.uszRedeemBit = VS_TRUE;
//                                /* 卡號查詢不過第二次卡，所以要直接決定TRT */
//                                if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
//                                {
//                                        inNCCC_Func_Decide_CTLS_TRT(pobTran);
//                                }
//                                else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
//                                {
//                                        inNCCC_Func_Decide_ICC_TRT(pobTran);
//                                }
//                                else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
//                                {
//                                        inNCCC_Func_Decide_MEG_TRT(pobTran);
//                                }
//
//				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
//			}
//			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REDEEM_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利退貨＞ */

				pobTran->inFunctionID = _REDEEM_REFUND_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_GET_CARD_FIRST_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_;
				}
				pobTran->inRunTRTID = _TRT_REDEEM_REFUND_;

				pobTran->inTransactionCode = _REDEEM_REFUND_;
				pobTran->srBRec.inCode = _REDEEM_REFUND_;
				pobTran->srBRec.inOrgCode = _REDEEM_REFUND_;

				pobTran->srBRec.uszRedeemBit = VS_TRUE;
                                /* 卡號查詢不過第二次卡，所以要直接決定TRT */
                                if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
                                {
                                        inNCCC_Func_Decide_CTLS_TRT(pobTran);
                                }
                                else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
                                {
                                        inNCCC_Func_Decide_ICC_TRT(pobTran);
                                }
                                else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
                                {
                                        inNCCC_Func_Decide_MEG_TRT(pobTran);
                                }

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;
			
		case _ECR_8N1_REDEEM_ADJUST_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_REDEEM_ADJUST_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利調帳＞ */
			
			pobTran->inFunctionID = _REDEEM_ADJUST_;
			pobTran->inRunOperationID = _OPERATION_REDEEM_ADJUST_;
			pobTran->inRunTRTID = _TRT_REDEEM_ADJUST_;

			pobTran->inTransactionCode = _REDEEM_ADJUST_;
			pobTran->srBRec.inCode = _REDEEM_ADJUST_;
			pobTran->srBRec.inOrgCode = _REDEEM_ADJUST_;
			
			pobTran->srBRec.uszRedeemBit = VS_TRUE;
                        /* 卡號查詢不過第二次卡，所以要直接決定TRT */
                        if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
                        {
                                inNCCC_Func_Decide_CTLS_TRT(pobTran);
                        }
                        else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
                        {
                                inNCCC_Func_Decide_ICC_TRT(pobTran);
                        }
                        else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
                        {
                                inNCCC_Func_Decide_MEG_TRT(pobTran);
                        }
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
			
		case _ECR_8N1_TIP_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_TIP_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜小費交易＞ */
			
			pobTran->inFunctionID = _TIP_;
			pobTran->inRunOperationID = _OPERATION_TIP_;
			pobTran->inRunTRTID = _TRT_TIP_;

			pobTran->inTransactionCode = _TIP_;
			pobTran->srBRec.inCode = _TIP_;
			pobTran->srBRec.inOrgCode = _TIP_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
			
		case _ECR_8N1_AWARD_REDEEM_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_LOYALTY_REDEEM_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜優惠兌換＞ */
			
			pobTran->inFunctionID = _LOYALTY_REDEEM_;
			/* 收銀機以條碼兌換*/
			if (pobTran->szL3_AwardWay[0] == '1')
			{
				pobTran->inRunOperationID = _OPERATION_BARCODE_;
			}
			else
			{
				pobTran->inRunOperationID = _OPERATION_LOYALTY_REDEEM_CTLS_;
			}
			pobTran->inRunTRTID = _TRT_LOYALTY_REDEEM_;

			pobTran->inTransactionCode = _LOYALTY_REDEEM_;
			pobTran->srBRec.inCode = _LOYALTY_REDEEM_;
			pobTran->srBRec.inOrgCode = _LOYALTY_REDEEM_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
			
		case _ECR_8N1_VOID_AWARD_REDEEM_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_VOID_LOYALTY_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜兌換取消＞ */
			
			pobTran->inFunctionID = _VOID_LOYALTY_REDEEM_;
			pobTran->inRunOperationID = _OPERATION_VOID_LOYALTY_REDEEM_;
			pobTran->inRunTRTID = _TRT_LOYALTY_REDEEM_;

			pobTran->inTransactionCode = _VOID_LOYALTY_REDEEM_;
			pobTran->srBRec.inCode = _VOID_LOYALTY_REDEEM_;
			pobTran->srBRec.inOrgCode = _VOID_LOYALTY_REDEEM_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;


		/* 快樂購紅利積點 */
		case _ECR_8N1_HG_REWARD_SALE_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡紅利積點＞ */
			
			pobTran->inFunctionID = _SALE_;
			pobTran->inRunOperationID = _OPERATION_HG_;
			pobTran->inRunTRTID = _TRT_HG_REWARD_CREDIT_CTLS_;

			/* 這邊先設成sale，若之後支付方式選銀聯卡，會在那邊改成CUP_SALE */
			pobTran->inTransactionCode = _SALE_;
			pobTran->srBRec.inCode = _SALE_;
			pobTran->srBRec.inOrgCode = _SALE_;
			pobTran->srBRec.lnHGTransactionType = _HG_REWARD_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		
		/* 快樂購紅利積點─分期付款 */
		case _ECR_8N1_HG_REWARD_INSTALLMENT_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_INS_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡分期付款＞ */
			
			pobTran->inFunctionID = _INST_SALE_;
			pobTran->inRunOperationID = _OPERATION_HG_I_R_;
			pobTran->inRunTRTID = _TRT_HG_REWARD_INSTALLMENT_CTLS_;

			pobTran->inTransactionCode = _INST_SALE_;
			pobTran->srBRec.inCode = _INST_SALE_;
			pobTran->srBRec.inOrgCode = _INST_SALE_;
			pobTran->srBRec.lnHGTransactionType = _HG_REWARD_;
			pobTran->srBRec.uszInstallmentBit = VS_TRUE;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		
		/* 快樂購紅利積點─紅利扣抵 */
		case _ECR_8N1_HG_REWARD_REDEMPTION_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_REDEEM_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡紅利扣抵＞ */
			
			pobTran->inFunctionID = _REDEEM_SALE_;
			pobTran->inRunOperationID = _OPERATION_HG_I_R_;
			pobTran->inRunTRTID = _TRT_HG_REWARD_REDEMPTION_CTLS_;

			pobTran->inTransactionCode = _REDEEM_SALE_;
			pobTran->srBRec.inCode = _REDEEM_SALE_;
			pobTran->srBRec.inOrgCode = _REDEEM_SALE_;
			pobTran->srBRec.lnHGTransactionType = _HG_REWARD_;
			pobTran->srBRec.uszRedeemBit = VS_TRUE;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
			
		/* 快樂購點數扣抵 */
		case _ECR_8N1_HG_ONLINE_REDEEMPTION_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_ONLINE_REDEEM_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡點數扣抵＞ */
			
			pobTran->inFunctionID = _SALE_;
			pobTran->inRunOperationID = _OPERATION_HG_;
			pobTran->inRunTRTID = _TRT_HG_ONLINE_REDEEM_CREDIT_CTLS_;

			pobTran->inTransactionCode = _SALE_;
			pobTran->srBRec.inCode = _SALE_;
			pobTran->srBRec.inOrgCode = _SALE_;
			pobTran->srBRec.lnHGTransactionType = _HG_ONLINE_REDEEM_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		
		/* 快樂購加價購 */
		case _ECR_8N1_HG_POINT_CERTAIN_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_POINT_CERTAIN_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡加價購＞ */
			
			pobTran->inFunctionID = _SALE_;
			pobTran->inRunOperationID = _OPERATION_HG_;
			pobTran->inRunTRTID = _TRT_HG_POINT_CERTAIN_CREDIT_CTLS_;

			pobTran->inTransactionCode = _SALE_;
			pobTran->srBRec.inCode = _SALE_;
			pobTran->srBRec.inOrgCode = _SALE_;
			pobTran->srBRec.lnHGTransactionType = _HG_POINT_CERTAIN_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		
		/* 快樂購點數兌換 */
		case _ECR_8N1_HG_FULL_REDEEMPTION_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_REDEMPTION_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡點數兌換＞ */
			
			pobTran->inFunctionID = _SALE_;
			pobTran->inRunOperationID = _OPERATION_HG_;
			pobTran->inRunTRTID = _TRT_HG_FULL_REDEMPTION_;

			pobTran->inTransactionCode = _SALE_;
			pobTran->srBRec.inCode = _SALE_;
			pobTran->srBRec.inOrgCode = _SALE_;
			pobTran->srBRec.lnHGTransactionType = _HG_FULL_REDEMPTION_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		
		/* 快樂購扣抵退貨 */
		case _ECR_8N1_HG_REDEEM_REFUND_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_REDEEM_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡扣抵退貨＞ */
			
			pobTran->inFunctionID = _HG_REDEEM_REFUND_;
			pobTran->inRunOperationID = _OPERATION_HG_REFUND_;
			pobTran->inRunTRTID = _TRT_HG_REDEEM_REFUND_;

			pobTran->inTransactionCode = _HG_REDEEM_REFUND_;
			pobTran->srBRec.inCode = _HG_REDEEM_REFUND_;
			pobTran->srBRec.inOrgCode = _HG_REDEEM_REFUND_;
			pobTran->srBRec.lnHGTransactionType = _HG_REDEEM_REFUND_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		
		/* 快樂購回饋退貨 */
		case _ECR_8N1_HG_REWARD_REFUND_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡回饋退貨＞ */
			
			pobTran->inFunctionID = _HG_REWARD_REFUND_;
			pobTran->inRunOperationID = _OPERATION_HG_REFUND_;
			pobTran->inRunTRTID = _TRT_HG_REWARD_REFUND_;

			pobTran->inTransactionCode = _HG_REWARD_REFUND_;
			pobTran->srBRec.inCode = _HG_REWARD_REFUND_;
			pobTran->srBRec.inOrgCode = _HG_REWARD_REFUND_;
			pobTran->srBRec.lnHGTransactionType = _HG_REWARD_REFUND_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		
		/* 快樂購點數查詢 */
		case _ECR_8N1_HG_POINT_INQUIRY_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_INQUIRY_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡點數查詢＞ */
			
			pobTran->inFunctionID = _HG_INQUIRY_;
			pobTran->inRunOperationID = _OPERATION_HG_;
			pobTran->inRunTRTID = _TRT_HG_INQUIRY_;

			pobTran->inTransactionCode = _HG_INQUIRY_;
			pobTran->srBRec.inCode = _HG_INQUIRY_;
			pobTran->srBRec.inOrgCode = _HG_INQUIRY_;
			pobTran->srBRec.lnHGTransactionType = _HG_INQUIRY_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
			
		case _ECR_8N1_SETTLEMENT_NO_:
			inDISP_ClearAll();
			/* 第三層顯示 ＜結帳交易＞ */
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_SETTLE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜結帳交易＞ */

			pobTran->inFunctionID = _SETTLE_;
			pobTran->inRunOperationID = _OPERATION_SETTLE_;
			pobTran->inRunTRTID = _TRT_SETTLE_;
			
			pobTran->inTransactionCode = _SETTLE_;
			pobTran->srBRec.inCode = _SETTLE_;
			pobTran->srBRec.inOrgCode = _SETTLE_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		
		case _ECR_8N1_REPRINT_RECEIPT_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_REPRINT_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜重印帳單＞ */

			pobTran->inFunctionID = FALSE;
			pobTran->inRunOperationID = _OPERATION_REPRINT_;
			pobTran->inRunTRTID = FALSE;
			
			pobTran->inTransactionCode = FALSE;
			pobTran->srBRec.inCode = FALSE;
			pobTran->srBRec.inOrgCode = FALSE;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
				
		case _ECR_8N1_MENU_REVIEW_DETAIL_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_REVIEW_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 <交易查詢> */
			
			pobTran->inFunctionID = FALSE;
			pobTran->inRunOperationID = _OPERATION_REVIEW_DETAIL_;
			pobTran->inRunTRTID = FALSE;
			
			pobTran->inTransactionCode = FALSE;
			pobTran->srBRec.inCode = FALSE;
			pobTran->srBRec.inOrgCode = FALSE;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		
		case _ECR_8N1_MENU_REVIEW_TOTAL_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_REVIEW_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜總額查詢＞ */
			
			pobTran->inFunctionID = FALSE;
			pobTran->inRunOperationID = _OPERATION_REVIEW_TOTAL_;
			pobTran->inRunTRTID = FALSE;
			
			pobTran->inTransactionCode = FALSE;
			pobTran->srBRec.inCode = FALSE;
			pobTran->srBRec.inOrgCode = FALSE;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
			
		case _ECR_8N1_MENU_REPORT_DETAIL_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_DETAIL_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 明細列印 */
			
			pobTran->inFunctionID = FALSE;
			pobTran->inRunOperationID = _OPERATION_DETAIL_REPORT_;
			pobTran->inRunTRTID = FALSE;
			
			pobTran->inTransactionCode = FALSE;
			pobTran->srBRec.inCode = FALSE;
			pobTran->srBRec.inOrgCode = FALSE;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
			
		case _ECR_8N1_MENU_REPORT_TOTAL_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_TOTAIL_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 總額列印 */
			
			pobTran->inFunctionID = FALSE;
			pobTran->inRunOperationID = _OPERATION_TOTAL_REPORT_;
			pobTran->inRunTRTID = FALSE;
			
			pobTran->inTransactionCode = FALSE;
			pobTran->srBRec.inCode = FALSE;
			pobTran->srBRec.inOrgCode = FALSE;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		case _ECR_8N1_EDC_REBOOT_NO_:
			inDISP_ClearAll();
			/* 第三層顯示 ＜重新開機＞ */
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_REBOOT_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜重新開機＞ */
			
			pobTran->inFunctionID = FALSE;
			pobTran->inRunOperationID = _OPERATION_EDC_REBOOT_;
			
			pobTran->inRunTRTID = FALSE;
			
			pobTran->inTransactionCode = FALSE;
			pobTran->srBRec.inCode = FALSE;
			pobTran->srBRec.inOrgCode = FALSE;
			
			/* 直接用function 不跑OPT */
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			
			break;
		case _ECR_8N1_ESVC_TOP_UP_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_TICKET_TOP_UP_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜加值交易＞ */

			pobTran->inFunctionID = _TICKET_TOP_UP_;

			if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
			{
				/* (需求單 - 107227)邦柏科技自助作業客製化 悠遊卡0018重試功能 by Russell 2018/12/14 上午 9:30 */
				if (pobTran->uszLastTranscationBit == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_TICKET_RETRY_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_TICKET_TOP_UP_;
				}
			}
			else
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "感應未初始化，不能使用此功能");
				}
				return (VS_ERROR);
			}

			pobTran->inTransactionCode = _TICKET_TOP_UP_;
			pobTran->srTRec.inCode = _TICKET_TOP_UP_;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		case _ECR_8N1_ESVC_BALANCE_INQUIRY_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0, _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_TICKET_INQUIRY_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜餘額查詢＞ */

			pobTran->inFunctionID = _TICKET_INQUIRY_;

			if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
			{
				pobTran->inRunOperationID = _OPERATION_TICKET_INQUIRY_;
			}
			else
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "感應未初始化，不能使用此功能");
				}
				return (VS_ERROR);
			}

			pobTran->inTransactionCode = _TICKET_INQUIRY_;
			pobTran->srTRec.inCode = _TICKET_INQUIRY_;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		case _ECR_8N1_ESVC_VOID_TOP_UP_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);					/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_TICKET_VOID_TOP_UP_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜加值取消＞ */

			pobTran->inFunctionID = _TICKET_VOID_TOP_UP_;

			if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
			{
				/* (需求單 - 107227)邦柏科技自助作業客製化 悠遊卡0018重試功能 by Russell 2018/12/14 上午 9:30 */
				if (pobTran->uszLastTranscationBit == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_TICKET_RETRY_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_TICKET_VOID_TOP_UP_;
				}
			}
			else
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "感應未初始化，不能使用此功能");
				}
				return (VS_ERROR);
			}

			pobTran->inTransactionCode = _TICKET_VOID_TOP_UP_;
			pobTran->srTRec.inCode = _TICKET_VOID_TOP_UP_;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		case _ECR_8N1_END_CARD_NO_INQUIRY_NO_:
			pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_END_INQUIRY_SUCCESS_;
			inRetVal = VS_ERROR;
			break;
		/* 客製化107直接借用來當Echo Test*/
		case _ECR_8N1_START_CARD_NO_INQUIRY_NO_:				/* Echo Test */	
			inRetVal = VS_SUCCESS;
			break;
		/* 查詢上一筆 */
		case _ECR_8N1_INQUIRY_LAST_TRANSACTION_NO_:
			inRetVal = VS_SUCCESS;
			break;
		default:
			pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
			inRetVal = VS_ERROR;
			break;
	}
	
	return (inRetVal);
}

/*
Function        :inECR_8N1_Customer_Self_Trans_Settle_Pack
Date&Time       :2018/12/7 下午 9:20
Describe        :自助無人交易格式
*/
int inECR_8N1_Customer_Self_Trans_Settle_Pack(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer)
{
	int	inPacketSizes = 0;
	char	szTemplate[100 + 1] = {0};
	char	szFuncEnable[2 + 1] = {0};

	/* ECR Indicator (1 Byte) */
	memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szECRIndicator, 1);
	inPacketSizes ++;
	/* ECR Version Date (6 Byte) */
	memcpy(&szDataBuffer[inPacketSizes], "231208", 6);
	inPacketSizes += 6;
	/* Reserved (1 Byte) */
	inPacketSizes ++;
	/* Trans Type (2 Byte) */
	memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szTransType, 2);
	inPacketSizes += 2;
	
	/* Settlement Indicator (1 Byte) */
	memcpy(&szDataBuffer[inPacketSizes], srECROb->srTransData.szField_05, 1);
	inPacketSizes ++;

	/* HOST ID (2 Byte) */
	if (!memcmp(&srECROb->srTransData.szField_05[0], " ", 1))
		memcpy(&szDataBuffer[inPacketSizes], "  ", 2);
	else if (!memcmp(&srECROb->srTransData.szField_05[0], "N", 1))
		memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_HOSTID_NCCC_, 2);
	else if (!memcmp(&srECROb->srTransData.szField_05[0], "E", 1))
		memcpy(&szDataBuffer[inPacketSizes], _ECR_8N1_NCCC_HOSTID_ESVC_, 2);

	inPacketSizes += 2;
	
	/* Reserved (25 Byte) */
	inPacketSizes += 25;
	
	/* 信用卡結帳總筆數 (4 Byte) 信用卡結帳總金額 (12 Byte) */
	/* 有結帳該Host，且有結成功才回數字，否則回空白 */
	/* 【需求單 - 109306】卡人自助交易支援信用卡取消及退貨交易需求 更新收銀機結帳回覆規格(支援正負號)by Russell 2020/10/15 下午 5:27 */
	if ((!memcmp(&srECROb->srTransData.szField_05[0], " ", 1)	||
	     !memcmp(&srECROb->srTransData.szField_05[0], "N", 1))	&&
	     pobTran->uszNCCC_Settle_SuccessBit == VS_TRUE)
	{
		/* 信用卡結帳總筆數 (4 Byte) */
		/* Credit Total Count (4 Byte) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%04ld", pobTran->lnNCCC_SettleTotalCount);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 4);
		inPacketSizes += 4;

		/* 信用卡結帳總金額 (12 Byte)*/
		/* Credit Total Amount (12 Byte) */
		/* 【需求單 - 109306】卡人自助交易支援信用卡取消及退貨交易需求 更新收銀機結帳回覆規格(支援正負號)by Russell 2020/10/15 下午 5:27 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%010lld00", pobTran->llNCCC_SettleTotalAmount);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
		inPacketSizes += 12;
	}
	/* 結電票，信用卡回傳空白 */
	else
	{
		/* 信用卡結帳總筆數 (4 Byte) */
		/* Credit Total Count (4 Byte) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 4, _PADDING_LEFT_);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 4);
		inPacketSizes += 4;

		/* 信用卡結帳總金額 (12 Byte)*/
		/* Credit Total Amount (12 Byte) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 12, _PADDING_LEFT_);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 12);
		inPacketSizes += 12;
	}

	/* Trans Date (6 Byte) & Trans Time (6 Byte) */
	if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
	{
		/* Trans Date */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		strcat(szTemplate, pobTran->srTRec.szDate);
		memcpy(&szDataBuffer[inPacketSizes], &szTemplate[0], 6);
		inPacketSizes += 6;

		/* Trans Time */
		memcpy(&szDataBuffer[inPacketSizes], pobTran->srTRec.szTime, 6);
		inPacketSizes += 6;
	}
	else
	{
		/* Trans Date */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		strcat(szTemplate, pobTran->srBRec.szDate);
		memcpy(&szDataBuffer[inPacketSizes], &szTemplate[2], 6);
		inPacketSizes += 6;

		/* Trans Time */
		memcpy(&szDataBuffer[inPacketSizes], pobTran->srBRec.szTime, 6);
		inPacketSizes += 6;
	}


	/* Reserved (10 Byte) */
	inPacketSizes += 10;

	/* ECR Response Code (4 Byte) */
	memcpy(&szDataBuffer[inPacketSizes], "0000", 4);
	
	inPacketSizes += 4;

	/* Merchant ID (15 Byte) & Terminal ID (8 Byte) */
	/* Merchant ID (15 Byte) */
	inPacketSizes += 15;
	/* Terminal ID (8 Byte) */
	inPacketSizes += 8;

	/* 有結帳該Host，且有結成功才回數字，否則回空白 */
	/* 【需求單 - 109306】卡人自助交易支援信用卡取消及退貨交易需求 更新收銀機結帳回覆規格(支援正負號)by Russell 2020/10/15 下午 5:27 */
	if ((!memcmp(&srECROb->srTransData.szField_05[0], " ", 1)	||
	     !memcmp(&srECROb->srTransData.szField_05[0], "E", 1))	&&
	     pobTran->uszESVC_Settle_SuccessBit == VS_TRUE)
	{
		inLoadTDTRec(_TDT_INDEX_01_ECC_);
		memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
		inGetTicket_HostEnable(szFuncEnable);
		
		if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
		{
			/* 悠遊卡購貨總筆數 (3 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%03ld", pobTran->lnECC_SettleTotalCount);
			memcpy(&szDataBuffer[inPacketSizes], szTemplate, 3);
			inPacketSizes += 3;

			/* 悠遊卡購貨總金額 (10 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%08lld00", pobTran->llECC_SettleTotalAmount);
			memcpy(&szDataBuffer[inPacketSizes], szTemplate, 10);
			inPacketSizes += 10;
		}
		else
		{
			/* 悠遊卡購貨總筆數 (3 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 3, _PADDING_LEFT_);
			memcpy(&szDataBuffer[inPacketSizes], szTemplate, 3);
			inPacketSizes += 3;

			/* 悠遊卡購貨總金額 (10 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 10, _PADDING_LEFT_);
			memcpy(&szDataBuffer[inPacketSizes], szTemplate, 10);
			inPacketSizes += 10;
		}

		inLoadTDTRec(_TDT_INDEX_00_IPASS_);
		memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
		inGetTicket_HostEnable(szFuncEnable);
		
		if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
		{
			/* 一卡通購貨總筆數 (3 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%03ld", pobTran->lnIPASS_SettleTotalCount);
			memcpy(&szDataBuffer[inPacketSizes], szTemplate, 3);
			inPacketSizes += 3;

			/* 一卡通購貨總金額 (10 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%08lld00", pobTran->llIPASS_SettleTotalAmount);
			memcpy(&szDataBuffer[inPacketSizes], szTemplate, 10);
			inPacketSizes += 10;
		}
		else
		{
			/* 一卡通購貨總筆數 (3 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 3, _PADDING_LEFT_);
			memcpy(&szDataBuffer[inPacketSizes], szTemplate, 3);
			inPacketSizes += 3;

			/* 一卡通購貨總金額 (10 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 10, _PADDING_LEFT_);
			memcpy(&szDataBuffer[inPacketSizes], szTemplate, 10);
			inPacketSizes += 10;
		}

		inLoadTDTRec(_TDT_INDEX_02_ICASH_);
		memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
		inGetTicket_HostEnable(szFuncEnable);
		
		if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
		{
			/* 愛金卡購貨總筆數 (3 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%03ld", pobTran->lnICASH_SettleTotalCount);
			memcpy(&szDataBuffer[inPacketSizes], szTemplate, 3);
			inPacketSizes += 3;

			/* 愛金卡購貨總金額 (10 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%08lld00", pobTran->llICASH_SettleTotalAmount);
			memcpy(&szDataBuffer[inPacketSizes], szTemplate, 10);
			inPacketSizes += 10;
		}
		else
		{
			/* 愛金卡購貨總筆數 (3 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 3, _PADDING_LEFT_);
			memcpy(&szDataBuffer[inPacketSizes], szTemplate, 3);
			inPacketSizes += 3;

			/* 愛金卡購貨總金額 (10 Byte) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 10, _PADDING_LEFT_);
			memcpy(&szDataBuffer[inPacketSizes], szTemplate, 10);
			inPacketSizes += 10;
		}
		
		memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
		/* 遠鑫卡購貨總筆數 (3 Byte) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 3, _PADDING_LEFT_);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 3);
		inPacketSizes += 3;

		/* 遠鑫卡購貨總金額 (10 Byte) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 10, _PADDING_LEFT_);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 10);
		inPacketSizes += 10;
	}
	else
	{
		/* 悠遊卡購貨總筆數 (3 Byte) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 3, _PADDING_LEFT_);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 3);
		inPacketSizes += 3;

		/* 悠遊卡購貨總金額 (10 Byte) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 10, _PADDING_LEFT_);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 10);
		inPacketSizes += 10;

		/* 一卡通購貨總筆數 (3 Byte) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 3, _PADDING_LEFT_);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 3);
		inPacketSizes += 3;

		/* 一卡通購貨總金額 (10 Byte) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 10, _PADDING_LEFT_);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 10);
		inPacketSizes += 10;

		/* 愛金卡購貨總筆數 (3 Byte) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 3, _PADDING_LEFT_);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 3);
		inPacketSizes += 3;

		/* 愛金卡購貨總金額 (10 Byte) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 10, _PADDING_LEFT_);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 10);
		inPacketSizes += 10;

		/* 遠鑫卡購貨總筆數 (3 Byte) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 3, _PADDING_LEFT_);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 3);
		inPacketSizes += 3;

		/* 遠鑫卡購貨總金額 (10 Byte) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 10, _PADDING_LEFT_);
		memcpy(&szDataBuffer[inPacketSizes], szTemplate, 10);
		inPacketSizes += 10;
	}
	
	/* Reserved (245 Byte) */
	inPacketSizes += 245;
	
	return (VS_SUCCESS);
}

/*
Function        :inECR_8N1_Customer_Self_Trans_Settle_Pack
Date&Time       :2018/12/7 下午 9:20
Describe        :自助無人交易格式
*/
int inECR_8N1_Inquiry_Last_Transaction_Pack(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer)
{
	int		inPacketSizes = 0;
	int		inSendSize = 0;
	unsigned long	ulHandle = 0;
	
	/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/10 上午 10:32 */
	if (gbBarCodeECRBit == VS_TRUE)
	{
		inSendSize = _ECR_8N1_1000_CUPQRCODE_Size_;
	}
	else
	{
		inSendSize = _ECR_8N1_Standard_Data_Size_;
	}
	
	inFILE_OpenReadOnly(&ulHandle, (unsigned char*)_ECR_LAST_RECE_FILENAME_);
	inFILE_Seek(ulHandle, 0, _SEEK_BEGIN_);
	inFILE_Read(&ulHandle, (unsigned char*)&szDataBuffer[inPacketSizes], inSendSize);
	inFILE_Close(&ulHandle);
	
	return (VS_SUCCESS);
}
/*
Function        :inECR_8N1_Customer_107_Bumper_Pack_ResponseCode
Date&Time       :2016/7/6 下午 2:40
Describe        :先把要送的資料組好
*/
int inECR_8N1_Customer_107_Bumper_Pack_ResponseCode(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuf)
{
	char	szTRTFileName[12 + 1];
	char	szFESMode[2 + 1];
	
	/* 非參加機構卡片判斷 */
	if (!memcmp(pobTran->srBRec.szRespCode, "05", 2) && (!memcmp(&pobTran->srBRec.szMPASAuthCode[0], "REJB01", 6) ||
							     !memcmp(&pobTran->srBRec.szMPASAuthCode[0], "REJB02", 6) ||
							     !memcmp(&pobTran->srBRec.szMPASAuthCode[0], "REJB03", 6)))
	{
		srECROb->srTransData.inErrorType = _ECR_RESPONSE_CODE_NOT_MEMBER_CARD_;
	}
	
	if (srECROb->srTransData.inErrorType == VS_CALLBANK	||
	    srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_CALLBANK_)
	{
		memcpy(&szDataBuf[76], "0002", 4);
	}
	else if (srECROb->srTransData.inErrorType == VS_TIMEOUT		||
		 srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_TIMEOUT_)
	{
		memcpy(&szDataBuf[76], "0003", 4);
	}
	else if (srECROb->srTransData.inErrorType == VS_USER_OPER_ERR	||	/* 操作錯誤 & 交易流程有誤 */
		 srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_) 
	{
		memcpy(&szDataBuf[76], "0004", 4);
	}
	else if (srECROb->srTransData.inErrorType == VS_ISO_PACK_ERR		|| 
		 srECROb->srTransData.inErrorType == VS_ISO_UNPACK_ERROR	||
		 srECROb->srTransData.inErrorType == VS_COMM_ERROR		||
		 srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_COMM_ERROR_)
	{
		memcpy(&szDataBuf[76], "0005", 4);
	}
	else if (srECROb->srTransData.inErrorType == VS_USER_CANCEL || 
		 srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_USER_TERMINATE_ERROR_)
	{
		memcpy(&szDataBuf[76], "0006", 4);
	}
	else if (srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_NOT_MEMBER_CARD_)
	{
		memcpy(&szDataBuf[76], "0009", 4); /* 非參加機構卡片 */
	}
	else if (srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_END_INQUIRY_SUCCESS_)
	{
		memcpy(&szDataBuf[76], "0000", 4);
	}
	else if (srECROb->srTransData.inErrorType == VS_ISO_PACK_ERR		|| 
		 srECROb->srTransData.inErrorType == VS_ISO_UNPACK_ERROR)
	{
		memcpy(&szDataBuf[76], "0010", 4); /* 電文錯誤 */
	}
	else if (srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_MULTI_CARD_)
	{
		memcpy(&szDataBuf[76], "0017", 4);
	}
	else if (srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_ESVC_RE_TAP_CARD_)
	{
		memcpy(&szDataBuf[76], "0018", 4);
	}
	else if (srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_SELF_TRANS_SETTLE_ERROR_)
	{
		if (!memcmp(&srECROb->srTransData.szField_05[0], " ", 1))
		{
			if (pobTran->uszNCCC_Settle_SuccessBit == VS_TRUE && pobTran->uszESVC_Settle_SuccessBit == VS_TRUE)
			{
				memcpy(&szDataBuf[76], "0000", 4);
			}
			else if (pobTran->uszNCCC_Settle_SuccessBit == VS_TRUE && pobTran->uszESVC_Settle_SuccessBit != VS_TRUE)
			{
				memcpy(&szDataBuf[76], "1002", 4);
			}
			else if (pobTran->uszNCCC_Settle_SuccessBit != VS_TRUE && pobTran->uszESVC_Settle_SuccessBit == VS_TRUE)
			{
				memcpy(&szDataBuf[76], "1003", 4);
			}
			else
			{
				memcpy(&szDataBuf[76], "1001", 4);
			}
		}
		else if (!memcmp(&srECROb->srTransData.szField_05[0], "N", 1))
		{
			if (pobTran->uszNCCC_Settle_SuccessBit == VS_TRUE)
			{
				memcpy(&szDataBuf[76], "0000", 4);
			}
			else
			{
				memcpy(&szDataBuf[76], "0001", 4);
			}
		}
		else if (!memcmp(&srECROb->srTransData.szField_05[0], "E", 1))
		{
			if (pobTran->uszESVC_Settle_SuccessBit == VS_TRUE)
			{
				memcpy(&szDataBuf[76], "0000", 4);
			}
			else
			{
				memcpy(&szDataBuf[76], "0001", 4);
			}
		}
	}
	/* (【需求單-113200】CASV3標準版支援銀聯優計劃專案 by Russell 2024/8/27 上午 11:03 */
	else if (srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_COUPON_USED_)
	{
		memcpy(&szDataBuf[76], "2001", 4);
	}
	else if (srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_COUPON_USED_)
	{
		memcpy(&szDataBuf[76], "2002", 4);
	}
	else if (srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_COUPON_USED_)
	{
		memcpy(&szDataBuf[76], "2003", 4);
	}
	else if (srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_COUPON_USED_)
	{
		memcpy(&szDataBuf[76], "2004", 4);
	}
	else if (srECROb->srTransData.inErrorType == _ECR_RESPONSE_CODE_COUPON_USED_)
	{
		memcpy(&szDataBuf[76], "2005", 4);
	}
	else
	{
		memset(szTRTFileName, 0x00, sizeof(szTRTFileName));
		inGetTRTFileName(szTRTFileName);
		/* 2017/9/6 上午 11:21 看verifone code新增1301 */
		if (memcmp(&pobTran->srBRec.szRespCode[0], "00", 2) &&
	            memcmp(&pobTran->srBRec.szRespCode[0], "11", 2) &&
                    memcmp(szTRTFileName, _TRT_FILE_NAME_HG_, strlen(_TRT_FILE_NAME_HG_)) == 0)
		{
                        memcpy(&szDataBuf[76], "1301", 4);
		}
                else
		{
			memcpy(&szDataBuf[76], "0001", 4);
		}
	}
	
	/* 電票的ResponseCode */
	if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
	{
		/* 因為失敗跳出的地方太多，所以加在這 */
		if (pobTran->uszAutoTopUpSuccessBit == VS_TRUE &&
		    pobTran->uszDeductSuccessBit != VS_TRUE)
		{
			sprintf(pobTran->szTicket_ErrorCode, "%s", "E0019");
		}
		
		/* 有值才送 */
		if (pobTran->szTicket_ErrorCode[0] == 'E')
		{
			memcpy(&szDataBuf[76], &pobTran->szTicket_ErrorCode[1], 4);
		}
	}
	
	memset(szFESMode, 0x00, sizeof(szFESMode));
	inGetNCCCFESMode(szFESMode);
	if (memcmp(szFESMode, _NCCC_04_MPAS_MODE_, strlen(_NCCC_04_MPAS_MODE_)))
	{
		if (pobTran->srBRec.uszMPASTransBit == VS_TRUE)
			memcpy(&szDataBuf[222], "M", 1);
		else
			memcpy(&szDataBuf[222], " ", 1);

		/* MP Response Code */
		if ((memcmp(pobTran->srBRec.szRespCode, "00", 2)) &&
		    (!memcmp(&pobTran->srBRec.szMPASAuthCode[0], "CAF", 3) || !memcmp(&pobTran->srBRec.szMPASAuthCode[0], "NEG", 3) ||
		     !memcmp(&pobTran->srBRec.szMPASAuthCode[0], "WEB", 3) || !memcmp(&pobTran->srBRec.szMPASAuthCode[0], "REJ", 3)))
		{
			memcpy(&szDataBuf[306], &pobTran->srBRec.szMPASAuthCode[0], 6);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inECR_Transform_8N1_No_To_String
Date&Time       :2019/1/15 上午 9:27
Describe        :轉換8N1交易類別編號為字串
*/
int inECR_Transform_8N1_No_To_String(int inNo, char *szTransType)
{
	switch (inNo)
	{
		case _ECR_8N1_SALE_NO_:
			strcpy(szTransType, "一般交易");
			break;
		case _ECR_8N1_VOID_NO_:
			strcpy(szTransType, "取消");
			break;
		case _ECR_8N1_REFUND_NO_:
			strcpy(szTransType, "退貨");
			break;
		case _ECR_8N1_INSTALLMENT_NO_:
			strcpy(szTransType, "分期付款");
			break;
		case _ECR_8N1_REDEEM_NO_:
			strcpy(szTransType, "紅利扣抵");
			break;
		case _ECR_8N1_INSTALLMENT_REFUND_NO_:
			strcpy(szTransType, "分期退貨");
			break;
		case _ECR_8N1_REDEEM_REFUND_NO_:
			strcpy(szTransType, "紅利退貨");
			break;
		case _ECR_8N1_INSTALLMENT_ADJUST_NO_:
			strcpy(szTransType, "分期調帳");
			break;
		case _ECR_8N1_REDEEM_ADJUST_NO_:
			strcpy(szTransType, "紅利調帳");
			break;
		case _ECR_8N1_OFFLINE_NO_:
			strcpy(szTransType, "交易補登");
			break;
		case _ECR_8N1_PREAUTH_NO_:
			strcpy(szTransType, "預先授權");
			break;
		case _ECR_8N1_PREAUTH_CANCEL_NO_:
			strcpy(szTransType, "預先授權取消");
			break;
		case _ECR_8N1_PREAUTH_COMPLETE_NO_:
			strcpy(szTransType, "預先授權完成");
			break;
		case _ECR_8N1_PREAUTH_COMPLETE_CANCEL_NO_:
			strcpy(szTransType, "預先授權完成取消");
			break;
		case _ECR_8N1_TIP_NO_:
			strcpy(szTransType, "小費交易");
			break;
		case _ECR_8N1_SETTLEMENT_NO_:
			strcpy(szTransType, "結帳");
			break;
		case _ECR_8N1_START_CARD_NO_INQUIRY_NO_:
			strcpy(szTransType, "啟動卡號查詢");
			break;
		case _ECR_8N1_INQUIRY_LAST_TRANSACTION_NO_:
			strcpy(szTransType, "查詢上一筆");
			break;
		case _ECR_8N1_EI_TRANSACTION_NO_:
			strcpy(szTransType, "電子發票");
			break;
		case _ECR_8N1_END_CARD_NO_INQUIRY_NO_:
			strcpy(szTransType, "結束卡號查詢");
			break;
		case _ECR_8N1_REPRINT_RECEIPT_NO_:
			strcpy(szTransType, "重印簽單");
			break;
		case _ECR_8N1_MENU_REVIEW_DETAIL_NO_:
			strcpy(szTransType, "交易明細查詢");
			break;
		case _ECR_8N1_MENU_REVIEW_TOTAL_NO_:
			strcpy(szTransType, "交易總額查詢");
			break;
		case _ECR_8N1_MENU_REPORT_DETAIL_NO_:
			strcpy(szTransType, "交易明細列印");
			break;
		case _ECR_8N1_MENU_REPORT_TOTAL_NO_:
			strcpy(szTransType, "交易總額列印");
			break;
		case _ECR_8N1_EDC_REBOOT_NO_:
			strcpy(szTransType, "EDC重新開機");
			break;
		case _ECR_8N1_AWARD_REDEEM_NO_:
			strcpy(szTransType, "優惠兌換");
			break;
		case _ECR_8N1_VOID_AWARD_REDEEM_NO_:
			strcpy(szTransType, "取消優惠兌換");
			break;
		case _ECR_8N1_ESVC_TOP_UP_NO_:
			strcpy(szTransType, "電票加值(現金加值)");
			break;
		case _ECR_8N1_ESVC_BALANCE_INQUIRY_NO_:
			strcpy(szTransType, "電票餘額查詢");
			break;
		case _ECR_8N1_ESVC_VOID_TOP_UP_NO_:
			strcpy(szTransType, "電票加值取消(現金加值)");
			break;
		case _ECR_8N1_HG_REWARD_SALE_NO_:
			strcpy(szTransType, "快樂購紅利積點(一般交易)");
			break;
		case _ECR_8N1_HG_REWARD_INSTALLMENT_NO_:
			strcpy(szTransType, "快樂購紅利積點 + 信用卡分期付款");
			break;
		case _ECR_8N1_HG_REWARD_REDEMPTION_NO_:
			strcpy(szTransType, "快樂購紅利積點 + 信用卡紅利扣抵 ");
			break;
		case _ECR_8N1_HG_ONLINE_REDEEMPTION_NO_:
			strcpy(szTransType, "快樂購點數扣抵");
			break;
		case _ECR_8N1_HG_POINT_CERTAIN_NO_:
			strcpy(szTransType, "快樂購加價購");
			break;
		case _ECR_8N1_HG_FULL_REDEEMPTION_NO_:
			strcpy(szTransType, "快樂購點數兌換");
			break;
		case _ECR_8N1_HG_REDEEM_REFUND_NO_:
			strcpy(szTransType, "快樂購扣抵退貨");
			break;
		case _ECR_8N1_HG_REWARD_REFUND_NO_:
			strcpy(szTransType, "快樂購回饋退貨");
			break;
		case _ECR_8N1_HG_POINT_INQUIRY_NO_:
			strcpy(szTransType, "快樂購點數查詢");
			break;
		case _ECR_8N1_ECHO_NO_:
			strcpy(szTransType, "Echo");
			break;
		case _ECR_8N1_EW_INQUIRY_TRANSACTION_NO_:
			strcpy(szTransType, "電子錢包交易查詢");
			break;
		default:
			return (VS_ERROR);
			break;
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inECR_8N1_Customer_111_Kiosk_Standard_Unpack
Date&Time       :2019/2/12 下午 1:51
Describe        :分析收銀機傳來的資料
*/
int inECR_8N1_Customer_111_Kiosk_Standard_Unpack(TRANSACTION_OBJECT *pobTran, ECR_TABLE* srECROb, char *szDataBuffer)
{
	int	i = 0;
	int	inRetVal = VS_SUCCESS;
	int	inTransType = 0;		/* 電文中的交易別字串轉為數字儲存 */
	int	inCardInquiryTRTType = _CARD_INQUIRY_TYPE_NONE_;
	int	inBarCodeLen = 0;
	int	inSize = 0;
	int	inTempTransType = 0;		/* 用來處理啟動卡號查詢流程前面就需要判斷的狀況 */
	char	szTemplate[100 + 1] = {0};
	char	szDebugMsg[100 + 1] = {0};	
	char	szHostLabel[8 + 1] = {0};
	char	szHostEnable[2 + 1] = {0};
	char	szBatchNum[6 + 1] = {0};
	char	szCTLSEnable[2 + 1] = {0};
	char	szTempSendData[_ECR_BUFF_SIZE_ + 1] = {0};
	char	szPayItemFuncEnable[2 + 1] = {0};
	char	szPayItemCodeTemp[5 + 1] = {0};
	char	szFiscContactlessEnable[1 + 1] = {0};
	char	szCUPContactlessEnable[1 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
	
        /* (需求單 - 107276)自助交易標準做法 EDC於發動交易及完成時，需發出提示音 by Russell 2019/3/8 上午 10:46 */
        inDISP_BEEP(1, 0);
                
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
        inGetCustomIndicator(szCustomerIndicator);
	
	/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/10/12 下午 5:55 */
	if (gbBarCodeECRBit == VS_TRUE)
	{
		inSize = _ECR_8N1_1000_CUPQRCODE_Size_;
	}
	else
	{
		inSize = _ECR_8N1_Standard_Data_Size_;
	}
	
	/* 客製化123 需要在簽名後才回送ECR */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_))
        { 
                pobTran->uszDelaySendBit = VS_TRUE;
	}
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
        { 
                pobTran->uszDelaySendBit = VS_TRUE;
	}
	
	/* 存ECR原始資料 */
	memcpy(srECROb->srTransData.szOrgData, szDataBuffer, inSize);
	
	inECR_8N1_Standard_Parse_Data(srECROb, szDataBuffer);
	
	/* ECR Indicator :新 ECR連線 Indicator"I""E"(規格新增欄位 )。 */
	/* 【需求單 - 108046】電子發票BIN大於6碼需求 標準400收銀機欄位ECR Indicator新增”E”值 by Russell 2019/7/5 下午 5:55 */
	switch (inTransType)
	{
		default:
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s: %s  ","ECR Indicator", srECROb->srTransData.szECRIndicator);
				inLogPrintf(AT, szDebugMsg);
			}
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(srECROb->srTransData.szECRIndicator) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ","ECR Indicator", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "ECR Indicator Not Exist Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
				return (VS_ERROR);
			}
			
			/* ECR Indicator必為'I' or 'E'，否則error */
			/* 【需求單 - 108046】電子發票BIN大於6碼需求 標準400收銀機欄位ECR Indicator新增”E”值 by Russell 2019/7/5 下午 5:55 */
			/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/8/13 下午 4:10 */
			if (memcmp(&srECROb->srTransData.szECRIndicator[0], "I", 1) != 0	&&
			    memcmp(&srECROb->srTransData.szECRIndicator[0], "E", 1) != 0	&&
			    memcmp(&srECROb->srTransData.szECRIndicator[0], "Q", 1) != 0)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ","ECR Indicator", "Not \"I\",\"E\",\"Q\" Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "ECR Indicator Not \"I\",\"E\",\"Q\" Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
				return (VS_ERROR);
			}
			break;
	}
	
	/* Trans Type Indicator :支援交易別 :一般交易、補登退貨 ，其餘 交易別，端末機不參考此欄位 。*/
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "%s: %s  ", "Trans Type Indicator", srECROb->srTransData.szTransTypeIndicator);
		inLogPrintf(AT, szDebugMsg);
	}
	
	/* Trans Type (交易別) */
	if (gbBarCodeECRBit == VS_TRUE)
	{
		switch (inTransType)
		{
			default:
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, srECROb->srTransData.szTransType, 2);

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Type", szTemplate);
					inLogPrintf(AT, szDebugMsg);
				}

				/* 欄位為M 卻不存在，回傳錯誤 */
				if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s: %s  ", "Trans Type", "Not Exist Error");
						inLogPrintf(AT, szDebugMsg);
					}
					memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
					inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "Trans Type Not Exist Error");
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

					return (VS_ERROR);
				}

				/* 交易類別轉成數字 */
				inTransType = atoi(szTemplate);

				if (inTransType == _ECR_8N1_SETTLEMENT_NO_)
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s%s  ", "掃碼格式", "不支援結帳");
						inLogPrintf(AT, szDebugMsg);
					}
					memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
					sprintf(pobTran->szErrorMsgBuff1, "掃碼格式");
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "不支援結帳");
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
					
					return (VS_ERROR);
				}
				break;
		}
	}
	else
	{
		/* Trans Type (交易別) */
		switch (inTransType)
		{
			default:
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, srECROb->srTransData.szTransType, 2);

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Type", szTemplate);
					inLogPrintf(AT, szDebugMsg);
				}

				/* 欄位為M 卻不存在，回傳錯誤 */
				if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s: %s  ", "Trans Type", "Not Exist Error");
						inLogPrintf(AT, szDebugMsg);
					}
					memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
					inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "Trans Type Not Exist Error");
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

					return (VS_ERROR);
				}
				else
				{
					memcpy(srECROb->srTransData.szTransType, szTemplate, 2);
				}

				/* 交易類別轉成數字 */
				inTransType = atoi(srECROb->srTransData.szTransType);

				break;
		}
	}
	
	/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/7 上午 10:28 */
	/* 掃碼交易規格 */
	if (gbBarCodeECRBit == VS_TRUE)
	{
		/* inTempTransType目前只用在前面判斷 */
		if (inTransType == _ECR_8N1_START_CARD_NO_INQUIRY_NO_ ||
		    inTransType == _ECR_8N1_INQUIRY_LAST_TRANSACTION_NO_)
		{
			inTempTransType = atoi(srECROb->srTransData.szStartTransType);
		}
		else
		{
			inTempTransType = inTransType;
		}
		/* 確認是哪一種交易 */
		switch (inTempTransType)
		{
			case _ECR_8N1_END_CARD_NO_INQUIRY_NO_ :
			case _ECR_8N1_SALE_NO_:
			case _ECR_8N1_INSTALLMENT_NO_:
			case _ECR_8N1_REDEEM_NO_:
				if (inNCCC_Func_Check_Uny_Format(srECROb->srTransData.szBarCodeData) == VS_TRUE)
				{
					pobTran->srBRec.uszUnyTransBit = VS_TRUE;
					/* Uny交易一定免簽 */
					pobTran->srBRec.uszNoSignatureBit = VS_TRUE;
				}
				else if (srECROb->srTransData.srEW_Data.uszEWTransBit == VS_TRUE)
				{
					pobTran->srEWRec.uszEWTransBit = VS_TRUE;
				}
				else if (inNCCC_Func_Is_CUP_UPLAN_ONLY(srECROb->srTransData.szBarCodeData, atoi(srECROb->srTransData.szBarCodeLen)) == VS_TRUE)
				{
					/* (【需求單-113200】CASV3標準版支援銀聯優計劃專案 by Russell 2024/8/27 上午 11:03 */
					pobTran->srBRec.uszUPlanECRBit = VS_TRUE;
					/* 銀聯退貨不送UP Table */
					if (inTempTransType == _ECR_8N1_REFUND_NO_ &&
					    !memcmp(&srECROb->srTransData.szField_05[0], "C", 1))
					{
						
					}
					else
					{
						pobTran->srBRec.uszUPlanTransBit = VS_TRUE;
						memset(pobTran->srBRec.szUPlan_Coupon, 0x00, sizeof(pobTran->srBRec.szUPlan_Coupon));
						memcpy(&pobTran->srBRec.szUPlan_Coupon[0], &srECROb->srTransData.szBarCodeData[0], (atoi(srECROb->srTransData.szBarCodeLen)));
					}
				}
				else if (inNCCC_Func_Analyse_UPlan_QRCODE(pobTran, srECROb->srTransData.szBarCodeData, atoi(srECROb->srTransData.szBarCodeLen)) == VS_SUCCESS)
				{
					/* (【需求單-113200】CASV3標準版支援銀聯優計劃專案 by Russell 2024/8/27 上午 11:03 */
					pobTran->srBRec.uszUPlanECRBit = VS_TRUE;
					pobTran->srBRec.uszCUPEMVQRCodeBit = VS_TRUE;
					
					/* QRCode支付免簽名 */
					if (inTempTransType == _ECR_8N1_REFUND_NO_ &&
					    !memcmp(&srECROb->srTransData.szField_05[0], "C", 1))
					{
						/* 退貨要簽 */
						pobTran->srBRec.uszNoSignatureBit = VS_FALSE;
					}
					else
					{
						
						pobTran->srBRec.uszNoSignatureBit = VS_TRUE;
					}
					
                                        if (strlen(pobTran->srBRec.szUPlan_Coupon) > 0)
                                        {
                                                /* 銀聯退貨不送UP Table */
                                                if (inTempTransType == _ECR_8N1_REFUND_NO_ &&
						    !memcmp(&srECROb->srTransData.szField_05[0], "C", 1))
						{

						}
                                                else
                                                {
                                                        pobTran->srBRec.uszUPlanTransBit = VS_TRUE;
                                                }
                                        }
				}
				else
				{
					/* 接收資料錯誤 */
					memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
					inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "BarCode Data Error");
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

					return (VS_ERROR);
				}
				break;
			case _ECR_8N1_REFUND_NO_:
			case _ECR_8N1_INSTALLMENT_REFUND_NO_:
			case _ECR_8N1_REDEEM_REFUND_NO_:
				if (strlen(srECROb->srTransData.szUnyTransCode) > 0)
				{
					pobTran->srBRec.uszUnyTransBit = VS_TRUE;
					/* Uny交易一定免簽 */
					pobTran->srBRec.uszNoSignatureBit = VS_TRUE;
				}
				else if (srECROb->srTransData.srEW_Data.uszEWTransBit == VS_TRUE)
				{
					pobTran->srEWRec.uszEWTransBit = VS_TRUE;
				}
				else if (inNCCC_Func_Is_CUP_UPLAN_ONLY(srECROb->srTransData.szBarCodeData, atoi(srECROb->srTransData.szBarCodeLen)) == VS_TRUE)
				{
					/* (【需求單-113200】CASV3標準版支援銀聯優計劃專案 by Russell 2024/8/27 上午 11:03 */
					pobTran->srBRec.uszUPlanECRBit = VS_TRUE;
					/* 銀聯退貨不送UP Table */
					if (inTempTransType == _ECR_8N1_REFUND_NO_ &&
					    !memcmp(&srECROb->srTransData.szField_05[0], "C", 1))
					{
						
					}
					else
					{
						pobTran->srBRec.uszUPlanTransBit = VS_TRUE;
						memset(pobTran->srBRec.szUPlan_Coupon, 0x00, sizeof(pobTran->srBRec.szUPlan_Coupon));
						memcpy(&pobTran->srBRec.szUPlan_Coupon[0], &srECROb->srTransData.szBarCodeData[0], (atoi(srECROb->srTransData.szBarCodeLen)));
					}
				}
				else if (inNCCC_Func_Analyse_UPlan_QRCODE(pobTran, srECROb->srTransData.szBarCodeData, atoi(srECROb->srTransData.szBarCodeLen)) == VS_SUCCESS)
				{
					/* (【需求單-113200】CASV3標準版支援銀聯優計劃專案 by Russell 2024/8/27 上午 11:03 */
					pobTran->srBRec.uszUPlanECRBit = VS_TRUE;
					pobTran->srBRec.uszCUPEMVQRCodeBit = VS_TRUE;
					
					/* QRCode支付免簽名 */
					if (inTempTransType == _ECR_8N1_REFUND_NO_ &&
					    !memcmp(&srECROb->srTransData.szField_05[0], "C", 1))
					{
						/* 退貨要簽 */
						pobTran->srBRec.uszNoSignatureBit = VS_FALSE;
					}
					else
					{
						
						pobTran->srBRec.uszNoSignatureBit = VS_TRUE;
					}
					
                                        if (strlen(pobTran->srBRec.szUPlan_Coupon) > 0)
                                        {
                                                /* 銀聯退貨不送UP Table */
                                                if (pobTran->srBRec.inCode == _CUP_REFUND_)
                                                {

                                                }
                                                else
                                                {
                                                        pobTran->srBRec.uszUPlanTransBit = VS_TRUE;
                                                }
                                        }
				}
				else
				{
					/* 接收資料錯誤 */
					memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
					inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "No Üny TransCode");
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

					return (VS_ERROR);
				}
				break;
			case _ECR_8N1_VOID_NO_:
				/* 直接撈原交易出來看是否是Uny交易 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, srECROb->srTransData.szHostID, 2);
				
				memset(szHostLabel, 0x00, sizeof(szHostLabel));
				if (memcmp(szTemplate, _ECR_8N1_NCCC_HOSTID_NCCC_, 2) == 0)
				{
					memcpy(szHostLabel, _HOST_NAME_CREDIT_NCCC_, strlen(_HOST_NAME_CREDIT_NCCC_));
				}
				else if (memcmp(szTemplate, _ECR_8N1_NCCC_HOSTID_DCC_, 2) == 0)
				{
					memcpy(szHostLabel, _HOST_NAME_DCC_, strlen(_HOST_NAME_DCC_));
				}
				/* 2018/5/8 下午 2:10 DFS需求不再使用大來主機 */
				else if (memcmp(szTemplate, _ECR_8N1_NCCC_HOSTID_LOYALTY_REDEEM_, 2) == 0)
				{
					memcpy(szHostLabel, _HOST_NAME_CREDIT_NCCC_, strlen(_HOST_NAME_CREDIT_NCCC_));
				}
				else if (memcmp(szTemplate, _ECR_8N1_NCCC_HOSTID_HG_, 2) == 0)
				{
					memcpy(szHostLabel, _HOST_NAME_HG_, strlen(_HOST_NAME_HG_));
				}
				else
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "無對應Host ID");
					}
					memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
					inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "%s : HOST ID Not Pair", szTemplate);
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

					return (VS_ERROR);
				}

				/* 初始化 */
				i = 0;
				do
				{
					/* 按順序load每一個HOST */
					if (inLoadHDTRec(i) < 0)
					{
						/* 如果return VS_ERROR 代表table沒有該HOST */
						memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
						inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
						memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
						sprintf(pobTran->szErrorMsgBuff2, "%s : HOST ID Not Found", szHostLabel);
						pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

						return (VS_ERROR);
					}

					memset(szTemplate, 0x00, sizeof(szTemplate));
					inGetHostLabel(szTemplate);
					if (!memcmp(szTemplate, szHostLabel, strlen(szHostLabel)))
					{
						/* 比對成功後，判斷HOST是否開啟  */
						memset(szHostEnable, 0x00, sizeof(szHostEnable));
						inGetHostEnable(szHostEnable);
						if (szHostEnable[0] != 'Y')
						{
							/* Host沒開 */
							if (ginDebug == VS_TRUE) 
							{
								memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
								sprintf(szDebugMsg, "%s Not Open Error", szHostLabel);
								inLogPrintf(AT, szDebugMsg);
							}

							memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
							inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
							memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
							sprintf(pobTran->szErrorMsgBuff2, "%s : HOST ID Not Open", szHostLabel);
							pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

							/* 如果return VS_ERROR 代表Host沒開 */
							return (VS_ERROR);
						}
						else
						{
							/* Host有開 */
							pobTran->srBRec.inHDTIndex = i;

							/* Load HDPT */
							inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
							memset(szBatchNum, 0x00, sizeof(szBatchNum));
							inGetBatchNum(szBatchNum);
							pobTran->srBRec.lnBatchNum = atol(szBatchNum);

							if (ginDebug == VS_TRUE)
							{
								memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
								sprintf(szDebugMsg, "%s Open", szHostLabel);
								inLogPrintf(AT, szDebugMsg);
							}

						}/* Host Enable比對End */

						break;
					}/* Host Label比對 End */
					i++;

				} while (1);
				
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(&szTemplate[0], srECROb->srTransData.szReceiptNo, 6);
				
				pobTran->srBRec.lnOrgInvNum = atol(szTemplate);
				inBATCH_GetTransRecord_By_Sqlite(pobTran);
				
				break;
			default:
				break;
		}
	}
	
	/* (需求單 - 107227)邦柏科技自助作業客製化 查詢上一筆機制 by Russell 2018/12/8 下午 2:18 */
	if (inTransType == _ECR_8N1_INQUIRY_LAST_TRANSACTION_NO_)
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_REVIEW_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第一層顯示 <交易查詢> */
		
		/* 複製一份Send Data進去比對 */
		memset(szTempSendData, 0x00, sizeof(szTempSendData));
		memcpy(szTempSendData, szDataBuffer, inSize);
		
		inRetVal = inECR_Inquiry_Last_Transction(pobTran, srECROb, szTempSendData, inSize);

		/* 如果比對失敗，回操作錯誤 */
		if (inRetVal == VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "查詢上一筆流程");
			}
			pobTran->uszLastTranscationBit = VS_TRUE;
		}
		else if (inRetVal == VS_TAP_AGAIN)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "TAP AGAIN流程");
			}
			/* 0018 重新感應用成之前的ECR電文 */
			/* 主要是前面的交易類別：查詢上一筆要轉換回原交易別，並用pobTran->uszLastTranscationBit來識別悠遊卡感應要用舊資料，
			    OPT跑特殊流程，TRT則跑正常流程 */
			sprintf(szDataBuffer, szTempSendData);
			memset(srECROb->srTransData.szTransType, 0x00, sizeof(srECROb->srTransData.szTransType));
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &szDataBuffer[8], 2);
			memcpy(srECROb->srTransData.szTransType, szTemplate, 2);
			/* 交易類別轉成數字 */
			inTransType = atoi(srECROb->srTransData.szTransType);
			
			pobTran->uszLastTranscationBit = VS_TRUE;
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "比對失敗");
			}
			/* 比對失敗，要回操作錯誤 */
			memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
			inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
			memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
			sprintf(pobTran->szErrorMsgBuff2, "查詢上一筆比對失敗");
			pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
			return (VS_ERROR);
		}
	}
	else if (inTransType == _ECR_8N1_START_CARD_NO_INQUIRY_NO_)
	{
		/* 不是查詢上一筆的話 移除Retry備份REQ */
		if (!memcmp(&srECROb->srTransData.szField_05[0], "E", 1))
		{
			inFile_Unlink_File(_ECC_API_REQ_RETRY_FILE_, _ECC_FOLDER_PATH_);
		}
	}
	else
	{
		/* 要另外存ECR Request電文 提供查詢上一筆時檢核用 */
		inECR_Save_Request(szDataBuffer, inSize);

		/* 不是查詢上一筆的話 移除Retry備份REQ */
		if (!memcmp(&srECROb->srTransData.szField_05[0], "E", 1))
		{
			inFile_Unlink_File(_ECC_API_REQ_RETRY_FILE_, _ECC_FOLDER_PATH_);
		}
	}
	
	/* CUP Indicator */
	/* 若前一次為卡號查詢，標記為第二次ECR */
	if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
	{
		pobTran->uszCardInquiryFirstBit = VS_FALSE;
		pobTran->uszCardInquirysSecondBit = VS_TRUE;
		
                /* 重新決定TRT流程 */
                if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
                {
                        /* 金融卡查詢要二次過卡，不依循前次查詢界面 */
                        inCardInquiryTRTType = _CARD_INQUIRY_TYPE_NONE_;
                }
                else
                {
                        if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
                        {
                                inCardInquiryTRTType = _CARD_INQUIRY_TYPE_CHIP_;
                        }
                        else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
                        {
                                inCardInquiryTRTType = _CARD_INQUIRY_TYPE_CTLS_;
                        }
                        else
                        {
                                inCardInquiryTRTType = _CARD_INQUIRY_TYPE_MEG_;
                        }
                }
	}
	
	/* 啟動卡號查詢不受Trans Indicator影響 */
	/* 客製化107改為Echo Host */
	if (inTransType == _ECR_8N1_START_CARD_NO_INQUIRY_NO_)
	{
		/* 卡號查詢 */
		inTransType = atoi(srECROb->srTransData.szStartTransType);
		switch (inTransType)
		{
			case _ECR_8N1_SALE_NO_ :
			case _ECR_8N1_REFUND_NO_ :
			case _ECR_8N1_PREAUTH_NO_ :
			case _ECR_8N1_PREAUTH_COMPLETE_NO_ :
			case _ECR_8N1_ESVC_BALANCE_INQUIRY_NO_:
			case _ECR_8N1_ESVC_TOP_UP_NO_:
			case _ECR_8N1_ESVC_VOID_TOP_UP_NO_:
			case _ECR_8N1_END_CARD_NO_INQUIRY_NO_:
				if (pobTran->uszCardInquirysSecondBit == VS_TRUE)
				{
					
				}
				else
				{
					pobTran->uszCardInquiryFirstBit = VS_TRUE;
				}
				/* 有值影響到後面判斷  確保都是空格 */
                                memcpy(&srECROb->srTransData.szTransTypeIndicator[0], " ", 1);
				break;
			case _ECR_8N1_OFFLINE_NO_ :
			case _ECR_8N1_INSTALLMENT_NO_ :
			case _ECR_8N1_REDEEM_NO_ :
			case _ECR_8N1_INSTALLMENT_REFUND_NO_ :
			case _ECR_8N1_REDEEM_REFUND_NO_ :
			case _ECR_8N1_INSTALLMENT_ADJUST_NO_ :
			case _ECR_8N1_REDEEM_ADJUST_NO_ :
				if (pobTran->uszCardInquirysSecondBit == VS_TRUE)
				{
					
				}
				else
				{
					pobTran->uszCardInquiryFirstBit = VS_TRUE;
				}
                                memcpy(&srECROb->srTransData.szField_05[0], " ", 1);
                                memcpy(&srECROb->srTransData.szTransTypeIndicator[0], " ", 1);
				break;
			default :
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "非可執行卡號查詢交易別");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				return (VS_ERROR);
		}
		
		switch (inTransType)
		{
			case _ECR_8N1_START_CARD_NO_INQUIRY_NO_:	/* 啟動卡號查詢 */
			case _ECR_8N1_SALE_NO_:				/* 一般交易 */
			case _ECR_8N1_REFUND_NO_:			/* 退貨交易 */
			case _ECR_8N1_VOID_NO_:				/* 取消 */
			case _ECR_8N1_PREAUTH_NO_:			/* 預先授權 */
			case _ECR_8N1_PREAUTH_COMPLETE_NO_:		/* 預先授權完成 */
			case _ECR_8N1_ESVC_TOP_UP_NO_:			/* 電子票證加值 */
			case _ECR_8N1_ESVC_BALANCE_INQUIRY_NO_:		/* 電子票證餘額查詢 */
			case _ECR_8N1_ESVC_VOID_TOP_UP_NO_:		/* 電子票證加值取消 */
			case _ECR_8N1_HG_REWARD_SALE_NO_:		/* HG紅利積點 */
			case _ECR_8N1_HG_ONLINE_REDEEMPTION_NO_:	/* HG點數抵扣 */
			case _ECR_8N1_HG_POINT_CERTAIN_NO_:		/* HG加價購 */
			case _ECR_8N1_PREAUTH_CANCEL_NO_:		/* 預先授權取消 */
			case _ECR_8N1_PREAUTH_COMPLETE_CANCEL_NO_:	/* 預先授權完成取消 */
				/* 如果是'C'代表是銀聯卡交易 */
				if (memcmp(&srECROb->srTransData.szField_05[0], "C", 1) == 0)
				{
					pobTran->srBRec.uszCUPTransBit = VS_TRUE;
					/* 銀聯目前沒這些交易別，要回接收資料錯誤 */
					switch (inTransType)
					{
						case _ECR_8N1_INSTALLMENT_NO_:
						case _ECR_8N1_REDEEM_NO_:
						case _ECR_8N1_INSTALLMENT_REFUND_NO_:
						case _ECR_8N1_REDEEM_REFUND_NO_:
						case _ECR_8N1_INSTALLMENT_ADJUST_NO_:
						case _ECR_8N1_REDEEM_ADJUST_NO_:
						case _ECR_8N1_OFFLINE_NO_:
							memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
							inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
							memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
							sprintf(pobTran->szErrorMsgBuff2, "銀聯無此交易別");
							pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
							return (VS_ERROR);
							break;
						default:
							break;
					}
				}

				/* 如果是'S'代表是SmartPay交易 */
				if (memcmp(&srECROb->srTransData.szField_05[0], "S", 1) == 0)
				{
					pobTran->srBRec.uszFiscTransBit = VS_TRUE;
					/* SmartPay不用簽名 */
					pobTran->srBRec.uszNoSignatureBit = VS_TRUE;
				}

				/* 如果是'E'代表是電票交易 */
				if (memcmp(&srECROb->srTransData.szField_05[0], "E", 1) == 0)
				{
					pobTran->srTRec.uszESVCTransBit = VS_TRUE;
					/* 若有票證未結帳，要提示訊息，比照520 2019/8/29 下午 1:46 */
					inNCCC_Ticket_Func_Must_SETTLE_Show_In_Menu();
				}
				break;
			default:
				break;
		}
	}
	else if (inTransType == _ECR_8N1_SETTLEMENT_NO_)
	{
		/* 空白：連動結帳(信用卡+電票)
		 * ‘N’：信用卡結帳
		 * ‘Ｅ’：電票結帳 
		 */
		/* (需求單 - 107227)邦柏科技自助作業客製化 by Russell 2018/12/9 下午 12:20 */
		/* 這裡不做特別檢核，由Host Id再決定結哪個批次的帳 */
		if (memcmp(&srECROb->srTransData.szField_05, " ", 1) == 0)
		{
			pobTran->uszAutoSettleBit = VS_TRUE;
		}
		else if (memcmp(&srECROb->srTransData.szField_05, "N", 1) == 0)
		{
			
		}
		else if (memcmp(&srECROb->srTransData.szField_05, "E", 1) == 0)
		{
			
		}
		else
		{
			if (ginDisplayDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Settlement Indicator error");
				inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_1_, VS_FALSE);
				
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, ": %s", srECROb->srTransData.szField_05);
				inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_2_, VS_FALSE);
			}
			memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
			inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
			memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
			sprintf(pobTran->szErrorMsgBuff2, "%s : 結帳Indicator錯誤", srECROb->srTransData.szField_05);
			pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
			
			return (VS_ERROR);
		}
	}
	else
	{
		/* Trans Type Indicator :支援交易別 :一般交易、補登退貨 ，其餘 交易別，端末機不參考此欄位 。*/
		switch (inTransType)
		{
			case _ECR_8N1_SALE_NO_:				/* 一般交易 */
			case _ECR_8N1_REFUND_NO_:			/* 退貨交易 */
			case _ECR_8N1_OFFLINE_NO_:			/* 交易補登 */
				/* Trans Type Indicator是'S'，要進選擇畫面 */
				if (!memcmp(&srECROb->srTransData.szTransTypeIndicator[0], "S", 1))
				{
					inRetVal = inECR_SelectTransType(srECROb);
					/* 交易類別轉成數字 */
					inTransType = atoi(srECROb->srTransData.szTransType);

					/* Select失敗 */
					if (inRetVal != VS_SUCCESS)
					{
						return (inRetVal);
					}
				}
				break;
			default:
				break;
		}

		/* CUP/Smart pay Indicator:銀聯卡 / Smart Pay交易使用
		 * Indicator = 'C'，表示為CUP交易
		 * Indicator = 'N'，表示為一般信用卡交易
		 * Indicator = 'S'，表示為SmartPay交易，(SmartPay的Sale Reversal和Refund，收銀機一定要送'S'。SmartPay的Sale送'N'，因為Sale不需要按Hotkey)
		 */
		switch (inTransType)
		{
			case _ECR_8N1_START_CARD_NO_INQUIRY_NO_:	/* 啟動卡號查詢 */
			case _ECR_8N1_SALE_NO_:				/* 一般交易 */
			case _ECR_8N1_REFUND_NO_:			/* 退貨交易 */
			case _ECR_8N1_VOID_NO_:				/* 取消 */
			case _ECR_8N1_PREAUTH_NO_:			/* 預先授權 */
			case _ECR_8N1_PREAUTH_COMPLETE_NO_:		/* 預先授權完成 */
			case _ECR_8N1_ESVC_TOP_UP_NO_:			/* 電子票證加值 */
			case _ECR_8N1_ESVC_BALANCE_INQUIRY_NO_:		/* 電子票證餘額查詢 */
			case _ECR_8N1_ESVC_VOID_TOP_UP_NO_:		/* 電子票證加值取消 */
			case _ECR_8N1_HG_REWARD_SALE_NO_:		/* HG紅利積點 */
			case _ECR_8N1_HG_ONLINE_REDEEMPTION_NO_:	/* HG點數抵扣 */
			case _ECR_8N1_HG_POINT_CERTAIN_NO_:		/* HG加價購 */
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "CUPIndicator", srECROb->srTransData.szField_05);
					inLogPrintf(AT, szDebugMsg);
				}

				/* 欄位為M 卻不存在，回傳錯誤 */
				if (inFunc_CheckFullSpace(srECROb->srTransData.szField_05) == VS_TRUE)
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s: %s  ", "CUPIndicator", "Not Exist Error");
						inLogPrintf(AT, szDebugMsg);
					}
					memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
					inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "CUPIndicator Not Exist Error");
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

					return (VS_ERROR);
				}

				break;
			case _ECR_8N1_PREAUTH_CANCEL_NO_:		/* 預先授權取消 */
			case _ECR_8N1_PREAUTH_COMPLETE_CANCEL_NO_:	/* 預先授權完成取消 */
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "  CUPIndicator :%s  ", srECROb->srTransData.szField_05);
					inLogPrintf(AT, szDebugMsg);
				}

				/* 欄位為M 卻不存在，回傳錯誤 */
				if (inFunc_CheckFullSpace(srECROb->srTransData.szField_05) == VS_TRUE)
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s: %s  ", "CUPIndicator", "Not Exist Error");
						inLogPrintf(AT, szDebugMsg);
					}
					memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
					inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "CUPIndicator Not Exist Error");
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

					return (VS_ERROR);
				}

				/* 預先授權取消及完成僅支援銀聯卡交易 ，故此二Request之 CUP Indicator欄位僅可放 ”C”*/
				if (memcmp(&srECROb->srTransData.szField_05[0], "C", 1) != 0)
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s: %s  ", "CUPIndicator", "CUP Not\"C\" Error");
						inLogPrintf(AT, szDebugMsg);
					}
					memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
					inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "非銀聯交易別帶CUP Indicator");
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

					return (VS_ERROR);
				}
				break;

			default:		
				break;
		}

		switch (inTransType)
		{
			case _ECR_8N1_START_CARD_NO_INQUIRY_NO_:	/* 啟動卡號查詢 */
			case _ECR_8N1_SALE_NO_:				/* 一般交易 */
			case _ECR_8N1_REFUND_NO_:			/* 退貨交易 */
			case _ECR_8N1_VOID_NO_:				/* 取消 */
			case _ECR_8N1_PREAUTH_NO_:			/* 預先授權 */
			case _ECR_8N1_PREAUTH_COMPLETE_NO_:		/* 預先授權完成 */
			case _ECR_8N1_ESVC_TOP_UP_NO_:			/* 電子票證加值 */
			case _ECR_8N1_ESVC_BALANCE_INQUIRY_NO_:		/* 電子票證餘額查詢 */
			case _ECR_8N1_ESVC_VOID_TOP_UP_NO_:		/* 電子票證加值取消 */
			case _ECR_8N1_HG_REWARD_SALE_NO_:		/* HG紅利積點 */
			case _ECR_8N1_HG_ONLINE_REDEEMPTION_NO_:	/* HG點數抵扣 */
			case _ECR_8N1_HG_POINT_CERTAIN_NO_:		/* HG加價購 */
			case _ECR_8N1_PREAUTH_CANCEL_NO_:		/* 預先授權取消 */
			case _ECR_8N1_PREAUTH_COMPLETE_CANCEL_NO_:	/* 預先授權完成取消 */
				/* 如果是'C'代表是銀聯卡交易 */
				if (memcmp(&srECROb->srTransData.szField_05[0], "C", 1) == 0)
				{
					pobTran->srBRec.uszCUPTransBit = VS_TRUE;
					/* 銀聯目前沒這些交易別，要回接收資料錯誤 */
					switch (inTransType)
					{
						case _ECR_8N1_INSTALLMENT_NO_:
						case _ECR_8N1_REDEEM_NO_:
						case _ECR_8N1_INSTALLMENT_REFUND_NO_:
						case _ECR_8N1_REDEEM_REFUND_NO_:
						case _ECR_8N1_INSTALLMENT_ADJUST_NO_:
						case _ECR_8N1_REDEEM_ADJUST_NO_:
						case _ECR_8N1_OFFLINE_NO_:
							memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
							inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
							memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
							sprintf(pobTran->szErrorMsgBuff2, "銀聯無此交易別");
							pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
							return (VS_ERROR);
							break;
						default:
							break;
					}
				}

				/* 如果是'S'代表是SmartPay交易 */
				if (memcmp(&srECROb->srTransData.szField_05[0], "S", 1) == 0)
				{
					pobTran->srBRec.uszFiscTransBit = VS_TRUE;
					/* SmartPay不用簽名 */
					pobTran->srBRec.uszNoSignatureBit = VS_TRUE;
				}

				/* 如果是'E'代表是電票交易 */
				if (memcmp(&srECROb->srTransData.szField_05[0], "E", 1) == 0)
				{
					pobTran->srTRec.uszESVCTransBit = VS_TRUE;
				}
				break;
			default:
				break;
		}
	}
	
	/* 銀行別/付款別 HOST ID*/
	switch (inTransType)
	{
		case _ECR_8N1_VOID_NO_:				/* 取消交易 */
		case _ECR_8N1_TIP_NO_:				/* 小費交易 */
		case _ECR_8N1_PREAUTH_CANCEL_NO_:		/* 預先授權取消 */
		case _ECR_8N1_PREAUTH_COMPLETE_CANCEL_NO_:	/* 預先授權完成取消 */
		case _ECR_8N1_REPRINT_RECEIPT_NO_:		/* 重印簽單 */
		case _ECR_8N1_MENU_REVIEW_DETAIL_NO_:		/* 明細查詢 */
		case _ECR_8N1_MENU_REVIEW_TOTAL_NO_:		/* 總額查詢 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, srECROb->srTransData.szHostID, 2);
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "HOST ID", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "HOST ID Not Exist Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
				return (VS_ERROR);
			}
			
			memset(szHostLabel, 0x00, sizeof(szHostLabel));

			if (memcmp(szTemplate, _ECR_8N1_NCCC_HOSTID_NCCC_, 2) == 0)
			{
				memcpy(szHostLabel, _HOST_NAME_CREDIT_NCCC_, strlen(_HOST_NAME_CREDIT_NCCC_));
			}
			else if (memcmp(szTemplate, _ECR_8N1_NCCC_HOSTID_DCC_, 2) == 0)
			{
				memcpy(szHostLabel, _HOST_NAME_DCC_, strlen(_HOST_NAME_DCC_));
			}
			/* 2018/5/8 下午 2:10 DFS需求不再使用大來主機 */
			else if (memcmp(szTemplate, _ECR_8N1_NCCC_HOSTID_LOYALTY_REDEEM_, 2) == 0)
			{
				memcpy(szHostLabel, _HOST_NAME_CREDIT_NCCC_, strlen(_HOST_NAME_CREDIT_NCCC_));
			}
			else if (memcmp(szTemplate, _ECR_8N1_NCCC_HOSTID_ESVC_, 2) == 0)
			{
				memcpy(szHostLabel, _HOST_NAME_ESVC_, strlen(_HOST_NAME_ESVC_));
				pobTran->srTRec.uszESVCTransBit = VS_TRUE;
			}
			else if (memcmp(szTemplate, _ECR_8N1_NCCC_HOSTID_HG_, 2) == 0)
			{
				memcpy(szHostLabel, _HOST_NAME_HG_, strlen(_HOST_NAME_HG_));
			}
			else
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "無對應Host ID");
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "%s : HOST ID Not Pair", szTemplate);
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
				return (VS_ERROR);
			}
			
			/* 初始化 */
			i = 0;
			do
			{
				/* 按順序load每一個HOST */
				if (inLoadHDTRec(i) < 0)
				{
					/* 如果return VS_ERROR 代表table沒有該HOST */
					memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
					inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "%s : HOST ID Not Found", szHostLabel);
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
					
					return (VS_ERROR);
				}

				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetHostLabel(szTemplate);
				if (!memcmp(szTemplate, szHostLabel, strlen(szHostLabel)))
				{
					/* 比對成功後，判斷HOST是否開啟  */
					memset(szHostEnable, 0x00, sizeof(szHostEnable));
					inGetHostEnable(szHostEnable);
					if (szHostEnable[0] != 'Y')
					{
						/* Host沒開 */
						if (ginDebug == VS_TRUE) 
						{
							memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
							sprintf(szDebugMsg, "%s Not Open Error", szHostLabel);
							inLogPrintf(AT, szDebugMsg);
						}

						/* 如果return VS_ERROR 代表Host沒開 */
						memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
						inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
						memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
						sprintf(pobTran->szErrorMsgBuff2, "%s : HOST ID Not Open", szHostLabel);
						pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
						
						return (VS_ERROR);
					}
					else
					{
						/* Host有開 */
						pobTran->srBRec.inHDTIndex = i;
						
						/* Load HDPT */
						inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
						memset(szBatchNum, 0x00, sizeof(szBatchNum));
						inGetBatchNum(szBatchNum);
						pobTran->srBRec.lnBatchNum = atol(szBatchNum);
						
						if (ginDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
							sprintf(szDebugMsg, "%s Open", szHostLabel);
							inLogPrintf(AT, szDebugMsg);
						}
						
					}/* Host Enable比對End */

					break;
				}/* Host Label比對 End */
				i++;

			} while (1);
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%d", "HDTindex", pobTran->srBRec.inHDTIndex);
				inLogPrintf(AT, szDebugMsg);
			}
			
			break;
		case _ECR_8N1_SETTLEMENT_NO_:
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, srECROb->srTransData.szHostID, 2);
			
			memset(szHostLabel, 0x00, sizeof(szHostLabel));
			
			/* 
			 * 空白連動結帳(信用卡+電票)
			 * ‘03’：信用卡結帳
			 * ‘06’：電票結帳
			 */
			if (memcmp(szTemplate, _ECR_8N1_NCCC_HOSTID_NCCC_, 2) == 0)
			{
				memcpy(szHostLabel, _HOST_NAME_CREDIT_NCCC_, strlen(_HOST_NAME_CREDIT_NCCC_));
			}
			else if (memcmp(szTemplate, _ECR_8N1_NCCC_HOSTID_ESVC_, 2) == 0)
			{
				memcpy(szHostLabel, _HOST_NAME_ESVC_, strlen(_HOST_NAME_ESVC_));
				pobTran->srTRec.uszESVCTransBit = VS_TRUE;
			}
			else if (memcmp(szTemplate, "  ", 2) == 0)
			{
				pobTran->uszAutoSettleBit = VS_TRUE;
			}
			else
			{
				if (ginDisplayDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "Host ID Err :%s", szTemplate);
					inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_1_, VS_FALSE);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "%s : HOST ID Not Pair", szTemplate);
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
				return (VS_ERROR);
			}
			
			/* 個別結帳 */
			if (strlen(szHostLabel) > 0)
			{
				/* 初始化 */
				i = 0;
				do
				{
					/* 按順序load每一個HOST */
					if (inLoadHDTRec(i) < 0)
					{
						/* 如果return VS_ERROR 代表table沒有該HOST */
						memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
						inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
						memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
						sprintf(pobTran->szErrorMsgBuff2, "%s : HOST ID Not Found", szHostLabel);
						pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
					
						return (VS_ERROR);
					}

					memset(szTemplate, 0x00, sizeof(szTemplate));
					inGetHostLabel(szTemplate);
					if (!memcmp(szTemplate, szHostLabel, strlen(szHostLabel)))
					{
						/* 比對成功後，判斷HOST是否開啟  */
						memset(szHostEnable, 0x00, sizeof(szHostEnable));
						inGetHostEnable(szHostEnable);
						if (szHostEnable[0] != 'Y')
						{
							/* Host沒開 */
							if (ginDebug == VS_TRUE) 
							{
								memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
								sprintf(szDebugMsg, "%s Not Open Error", szHostLabel);
								inLogPrintf(AT, szDebugMsg);
							}
							
							memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
							inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
							memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
							sprintf(pobTran->szErrorMsgBuff2, "%s : HOST ID Not Open", szHostLabel);
							pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

							/* 如果return VS_ERROR 代表Host沒開 */
							return (VS_ERROR);
						}
						else
						{
							/* Host有開 */
							pobTran->srBRec.inHDTIndex = i;

							/* Load HDPT */
							inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
							memset(szBatchNum, 0x00, sizeof(szBatchNum));
							inGetBatchNum(szBatchNum);
							pobTran->srBRec.lnBatchNum = atol(szBatchNum);

							if (ginDebug == VS_TRUE)
							{
								memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
								sprintf(szDebugMsg, "%s Open", szHostLabel);
								inLogPrintf(AT, szDebugMsg);
							}

						}/* Host Enable比對End */

						break;
					}/* Host Label比對 End */
					i++;

				} while (1);
			}
			/* 連動結帳 */
			else
			{
				
			}
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%d", "HDTindex", pobTran->srBRec.inHDTIndex);
				inLogPrintf(AT, szDebugMsg);
			}
			
			break;
		default:
			break;
	}
	
	/* EDC簽單序號 Receipt No(InvoiceNumber) */
	switch (inTransType)
	{
		case _ECR_8N1_VOID_NO_:				/* 取消交易 */
		case _ECR_8N1_TIP_NO_:				/* 小費交易 */
		case _ECR_8N1_PREAUTH_CANCEL_NO_:		/* 預先授權取消 */
		case _ECR_8N1_PREAUTH_COMPLETE_CANCEL_NO_:	/* 預先授權完成取消 */
		case _ECR_8N1_REPRINT_RECEIPT_NO_:		/* 重印簽單 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], srECROb->srTransData.szReceiptNo, 6);
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Receipt No", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "Receipt No Not Exist Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
				return (VS_ERROR);
			}
			else
			{
				pobTran->srBRec.lnOrgInvNum = atol(szTemplate);
			}
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%ld", "lnOrgInvNum", pobTran->srBRec.lnOrgInvNum);
				inLogPrintf(AT, szDebugMsg);
			}
			
			break;
		default:
			break;
	}
	
	/* 處理交易金額 (Trans Amount) */
	switch (inTransType)
	{
		case _ECR_8N1_START_CARD_NO_INQUIRY_NO_:	/* 啟動卡號查詢 */
		case _ECR_8N1_SALE_NO_:				/* 一般交易 */
		case _ECR_8N1_REFUND_NO_:			/* 退貨交易 */
		case _ECR_8N1_OFFLINE_NO_:			/* 交易補登 */
		case _ECR_8N1_PREAUTH_NO_:			/* 預先授權 */
		case _ECR_8N1_PREAUTH_COMPLETE_NO_:		/* 預先授權完成 */
		case _ECR_8N1_INSTALLMENT_NO_:			/* 分期 */
		case _ECR_8N1_INSTALLMENT_REFUND_NO_:		/* 分期退貨 */
		case _ECR_8N1_INSTALLMENT_ADJUST_NO_:		/* 分期調帳 */
		case _ECR_8N1_REDEEM_NO_:			/* 紅利 */		
		case _ECR_8N1_REDEEM_REFUND_NO_:		/* 紅利退貨 */
		case _ECR_8N1_REDEEM_ADJUST_NO_:		/* 紅利調帳 */
		case _ECR_8N1_ESVC_TOP_UP_NO_:			/* 電子票證加值 */
		case _ECR_8N1_ESVC_VOID_TOP_UP_NO_:		/* 電子票證加值取消 */
		case _ECR_8N1_HG_REWARD_SALE_NO_:		/* HG紅利積點 */
		case _ECR_8N1_HG_REWARD_INSTALLMENT_NO_ :	/* 快樂購紅利積點 + 信用卡分期付款 */
		case _ECR_8N1_HG_REWARD_REDEMPTION_NO_ :	/* 快樂購紅利積點 + 信用卡紅利扣抵 */
		case _ECR_8N1_HG_ONLINE_REDEEMPTION_NO_:	/* HG點數抵扣 */
		case _ECR_8N1_HG_POINT_CERTAIN_NO_:		/* HG加價購 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &srECROb->srTransData.szField_10[0], 10);
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Amount", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "Trans Amount Not Exist Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
				return (VS_ERROR);
			}
			else if ((!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_076_8_DIGITS_, _CUSTOMER_INDICATOR_SIZE_) && atol(szTemplate) > 99999999) || 
				 (memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_076_8_DIGITS_, _CUSTOMER_INDICATOR_SIZE_) && atol(szTemplate) > 9999999))
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Amount", "Out of bound");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_076_8_DIGITS_, _CUSTOMER_INDICATOR_SIZE_))
				{
					sprintf(pobTran->szErrorMsgBuff2, "金額超過8位數");
				}
				else
				{
					sprintf(pobTran->szErrorMsgBuff2, "金額超過7位數");
				}
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
				
				return (VS_ERROR);
			}
			else
			{
				pobTran->srBRec.lnTxnAmount = atol(szTemplate);
				pobTran->srBRec.lnOrgTxnAmount = atol(szTemplate);
				pobTran->srBRec.lnTotalTxnAmount = atol(szTemplate);
			}

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%ld", "交易金額", pobTran->srBRec.lnTxnAmount);
				inLogPrintf(AT, szDebugMsg);
			}
			
			break;
		default:
			break;
	}
	
	/* 交易日期 Trans Date */
	switch (inTransType)
	{
		case _ECR_8N1_START_CARD_NO_INQUIRY_NO_:	/* 啟動卡號查詢 */
		case _ECR_8N1_END_CARD_NO_INQUIRY_NO_:		/* 結束卡號查詢*/
//		case _ECR_8N1_SALE_NO_:				/* 一般交易 */		/* 不以ECR日期為主，會發生與機器日期不符的狀況 */
		case _ECR_8N1_REFUND_NO_:			/* 退貨交易 */
		case _ECR_8N1_VOID_NO_:				/* 取消交易 */
//		case _ECR_8N1_OFFLINE_NO_:			/* 交易補登 */		/* 不以ECR日期為主，會發生與機器日期不符的狀況 */
		case _ECR_8N1_TIP_NO_:				/* 小費交易 */
//		case _ECR_8N1_PREAUTH_NO_:			/* 預先授權 */		/* 不以ECR日期為主，會發生與機器日期不符的狀況 */
		case _ECR_8N1_PREAUTH_CANCEL_NO_:		/* 預先授權取消 */
		case _ECR_8N1_PREAUTH_COMPLETE_NO_:		/* 預先授權完成 */
		case _ECR_8N1_PREAUTH_COMPLETE_CANCEL_NO_:	/* 預先授權完成取消 */
//		case _ECR_8N1_INSTALLMENT_NO_:			/* 分期 */		/* 不以ECR日期為主，會發生與機器日期不符的狀況 */
		case _ECR_8N1_INSTALLMENT_REFUND_NO_:		/* 分期退貨 */
		case _ECR_8N1_INSTALLMENT_ADJUST_NO_:		/* 分期調帳 */
//		case _ECR_8N1_REDEEM_NO_:			/* 紅利 */		/* 不以ECR日期為主，會發生與機器日期不符的狀況 */		
		case _ECR_8N1_REDEEM_REFUND_NO_:		/* 紅利退貨 */
		case _ECR_8N1_REDEEM_ADJUST_NO_:		/* 紅利調帳 */
		case _ECR_8N1_SETTLEMENT_NO_:			/* 結帳交易 */
		case _ECR_8N1_REPRINT_RECEIPT_NO_:		/* 重印簽單 */
		case _ECR_8N1_MENU_REVIEW_DETAIL_NO_:		/* 明細查詢 */
		case _ECR_8N1_MENU_REVIEW_TOTAL_NO_:		/* 總額查詢 */
		case _ECR_8N1_MENU_REPORT_DETAIL_NO_:		/* 明細列印 */
		case _ECR_8N1_MENU_REPORT_TOTAL_NO_:		/* 總額列印 */
		case _ECR_8N1_ESVC_TOP_UP_NO_:			/* 電子票證加值 */
		case _ECR_8N1_ESVC_BALANCE_INQUIRY_NO_:		/* 電子票證餘額查詢 */
		case _ECR_8N1_ESVC_VOID_TOP_UP_NO_:		/* 電子票證加值取消 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &srECROb->srTransData.szTransDate[0], 6);
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Date", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				
//				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
//				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
//				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
//				sprintf(pobTran->szErrorMsgBuff2, "Trans Date Not Exist Error");
//				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
//				
//				return (VS_ERROR);
			}
			else
			{
				/* 根據安全理由，不採信ECR日期時間 */
			}
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "交易日期", pobTran->srBRec.szDate);
				inLogPrintf(AT, szDebugMsg);
			}
			break;
		default:
			break;
	}
	
	/* 交易時間 Trans Time */
	switch (inTransType)
	{
		case _ECR_8N1_START_CARD_NO_INQUIRY_NO_:	/* 啟動卡號查詢 */
		case _ECR_8N1_END_CARD_NO_INQUIRY_NO_:		/* 結束卡號查詢*/
		case _ECR_8N1_SALE_NO_:				/* 一般交易 */
		case _ECR_8N1_REFUND_NO_:			/* 退貨交易 */
		case _ECR_8N1_VOID_NO_:				/* 取消交易 */
		case _ECR_8N1_OFFLINE_NO_:			/* 交易補登 */
		case _ECR_8N1_TIP_NO_:				/* 小費交易 */
		case _ECR_8N1_PREAUTH_NO_:			/* 預先授權 */
		case _ECR_8N1_PREAUTH_CANCEL_NO_:		/* 預先授權取消 */
		case _ECR_8N1_PREAUTH_COMPLETE_NO_:		/* 預先授權完成 */
		case _ECR_8N1_PREAUTH_COMPLETE_CANCEL_NO_:	/* 預先授權完成取消 */
		case _ECR_8N1_INSTALLMENT_NO_:			/* 分期 */
		case _ECR_8N1_INSTALLMENT_REFUND_NO_:		/* 分期退貨 */
		case _ECR_8N1_INSTALLMENT_ADJUST_NO_:		/* 分期調帳 */
		case _ECR_8N1_REDEEM_NO_:			/* 紅利 */		
		case _ECR_8N1_REDEEM_REFUND_NO_:		/* 紅利退貨 */
		case _ECR_8N1_REDEEM_ADJUST_NO_:		/* 紅利調帳 */
		case _ECR_8N1_SETTLEMENT_NO_:			/* 結帳交易 */
		case _ECR_8N1_REPRINT_RECEIPT_NO_:		/* 重印簽單 */
		case _ECR_8N1_MENU_REVIEW_DETAIL_NO_:		/* 明細查詢 */
		case _ECR_8N1_MENU_REVIEW_TOTAL_NO_:		/* 總額查詢 */
		case _ECR_8N1_MENU_REPORT_DETAIL_NO_:		/* 明細列印 */
		case _ECR_8N1_MENU_REPORT_TOTAL_NO_:		/* 總額列印 */
		case _ECR_8N1_ESVC_TOP_UP_NO_:			/* 電子票證加值 */
		case _ECR_8N1_ESVC_BALANCE_INQUIRY_NO_:		/* 電子票證餘額查詢 */
		case _ECR_8N1_ESVC_VOID_TOP_UP_NO_:		/* 電子票證加值取消 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szTransTime[0], 6);
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Trans Time", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				
//				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
//				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
//				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
//				sprintf(pobTran->szErrorMsgBuff2, "Trans Time Not Exist Error");
//				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
//				
//				return (VS_ERROR);
			}
			else
			{
				/* 根據安全理由，不採信ECR日期時間 */
			}
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "交易時間", pobTran->srBRec.szTime);
				inLogPrintf(AT, szDebugMsg);
			}
			
			break;
			
		default:
			break;
	}
	
	/* 授權碼(Auth Code) ECR電文中給9個字元 但ATS電文只需要6個，所以只抓6個 */
	switch (inTransType)
	{
		case _ECR_8N1_REFUND_NO_:			/* 退貨 */
		case _ECR_8N1_OFFLINE_NO_:			/* 交易補登 */
		case _ECR_8N1_INSTALLMENT_REFUND_NO_:		/* 分期退貨 */
		case _ECR_8N1_INSTALLMENT_ADJUST_NO_:		/* 分期調帳 */
		case _ECR_8N1_REDEEM_REFUND_NO_:		/* 紅利退貨 */
		case _ECR_8N1_REDEEM_ADJUST_NO_:		/* 紅利調帳 */
			/* 處理授權碼(optional) */
			memset(pobTran->srBRec.szAuthCode, 0x00, sizeof(pobTran->srBRec.szAuthCode));
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szApprovalNo[0], 6);
			memcpy(pobTran->srBRec.szAuthCode, szTemplate, 6);
			break;
		case _ECR_8N1_PREAUTH_COMPLETE_NO_:		/* 預先授權完成 */
			/* 處理授權碼(must have) */
			memset(pobTran->srBRec.szAuthCode, 0x00, sizeof(pobTran->srBRec.szAuthCode));
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szApprovalNo[0], 6);
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Auth Code", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "Auth Code Not Exist Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
				return (VS_ERROR);
			}
			else
			{
				memcpy(pobTran->srBRec.szAuthCode, szTemplate, 6);
			}
			
			break;
			
		default :
			break;
	}
	
	/* 其他金額(Exp Amount) */
	switch (inTransType)
	{
		case _ECR_8N1_TIP_NO_:				/* 小費交易 */
			/* 小費金額*/
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetTRTFileName(szTemplate);
			if (!memcmp(szTemplate, "DCCTRT", 6) && !memcmp(srECROb->srTransData.szTransType, _ECR_8N1_TIP_, 2))
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &srECROb->srTransData.szExpAmount[0], 12);
			}
			else
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &srECROb->srTransData.szExpAmount[0], 10);
			}
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Exp Amount", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "Exp Amount Not Exist Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
				return (VS_ERROR);
			}
			
			pobTran->lnOldTaxAmount = atol(szTemplate);
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Tip: %ld", pobTran->lnOldTaxAmount);
				inLogPrintf(AT, szDebugMsg);
			}

			break;
			
		case _ECR_8N1_PREAUTH_COMPLETE_NO_:			/* 預先授權完成 預先授權完成交易之原預先授權金額 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szAuthAmount[0], 10);

			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(szTemplate) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "Exp Amount", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "Exp Amount Not Exist Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
				
				return (VS_ERROR);
			}

			pobTran->srBRec.lnOrgTxnAmount = atol(szTemplate);
			break;
			
		default :
			break;
	}
	
	/* 處理櫃號(Store ID) */
	switch (inTransType)
	{
		case _ECR_8N1_START_CARD_NO_INQUIRY_NO_:	/* 啟動卡號查詢 */
		case _ECR_8N1_SALE_NO_:				/* 一般交易 */
		case _ECR_8N1_REFUND_NO_:			/* 退貨交易 */
		case _ECR_8N1_OFFLINE_NO_:			/* 交易補登 */
		case _ECR_8N1_PREAUTH_NO_:			/* 預先授權 */
		case _ECR_8N1_PREAUTH_CANCEL_NO_:		/* 預先授權取消 */
		case _ECR_8N1_PREAUTH_COMPLETE_NO_:		/* 預先授權完成 */
		case _ECR_8N1_PREAUTH_COMPLETE_CANCEL_NO_:	/* 預先授權完成取消 */
		case _ECR_8N1_INSTALLMENT_NO_:			/* 分期 */
		case _ECR_8N1_INSTALLMENT_REFUND_NO_:		/* 分期退貨 */
		case _ECR_8N1_INSTALLMENT_ADJUST_NO_:		/* 分期調帳 */
		case _ECR_8N1_REDEEM_NO_:			/* 紅利 */
		case _ECR_8N1_REDEEM_REFUND_NO_:		/* 紅利退貨 */
		case _ECR_8N1_REDEEM_ADJUST_NO_:		/* 紅利調帳 */
		case _ECR_8N1_HG_REWARD_SALE_NO_:		/* HG紅利積點 */
		case _ECR_8N1_HG_REWARD_INSTALLMENT_NO_ :	/* 快樂購紅利積點 + 信用卡分期付款 */
		case _ECR_8N1_HG_REWARD_REDEMPTION_NO_ :	/* 快樂購紅利積點 + 信用卡紅利扣抵 */
		case _ECR_8N1_HG_ONLINE_REDEEMPTION_NO_ :	/* 快樂購點數扣抵 */
		case _ECR_8N1_HG_POINT_CERTAIN_NO_ :		/* 快樂購加價購 */
		case _ECR_8N1_HG_FULL_REDEEMPTION_NO_ :		/* 快樂購點數兌換 */
		case _ECR_8N1_HG_REDEEM_REFUND_NO_ :		/* 快樂購扣抵退貨 */
		case _ECR_8N1_HG_REWARD_REFUND_NO_ :		/* 快樂購回饋退貨 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szStoreId[0], 18);
			
			if (strlen(szTemplate) > 0)
			{
				memcpy(pobTran->srBRec.szStoreID, szTemplate, strlen(szTemplate));
				/* 櫃號不滿18，補空白 */
				if (strlen(szTemplate) < 18)
				{
					memset(&pobTran->srBRec.szStoreID[strlen(szTemplate)], 0x20, 18 - strlen(szTemplate));
				}

			}
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "Store ID", pobTran->srBRec.szStoreID);
				inLogPrintf(AT, szDebugMsg);
			}
			
			break;
		case _ECR_8N1_ESVC_TOP_UP_NO_:			/* 電子票證加值 */
		case _ECR_8N1_ESVC_VOID_TOP_UP_NO_:		/* 電子票證加值取消 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &srECROb->srTransData.szStoreId[0], 18);
			
			if (strlen(szTemplate) > 0)
			{
				memcpy(pobTran->srBRec.szStoreID, szTemplate, strlen(szTemplate));
				/* 櫃號不滿18，補空白 */
				if (strlen(szTemplate) < 18)
				{
					memset(&pobTran->srBRec.szStoreID[strlen(szTemplate)], 0x20, 18 - strlen(szTemplate));
				}

			}
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "Store ID", pobTran->srBRec.szStoreID);
				inLogPrintf(AT, szDebugMsg);
			}
			
			break;
		default:
			break;
	}
	
	if (inTransType == _ECR_8N1_AWARD_REDEEM_NO_)
	{
		/* 兌換模式 */
		switch (inTransType)
		{
			case _ECR_8N1_AWARD_REDEEM_NO_:		/* 優惠兌換 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &srECROb->srTransData.szField_20[0], 1);	/* (1=條碼兌換，2=卡號兌換) */

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s :%s", "Award Redeem Mode", szTemplate);
					inLogPrintf(AT, szDebugMsg);
				}
				
				/* 條碼兌換 */
				if (memcmp(szTemplate, "1", 1) == 0)
				{
					memcpy(pobTran->szL3_AwardWay, "1", 1);
				}
				else
				{
					/* 至EDC過卡流程決定 */
					memcpy(pobTran->szL3_AwardWay, "0", 1);
				}
				
				/* 兌換方式為條碼才抓條碼*/
				if (memcmp(pobTran->szL3_AwardWay, "1", strlen("1")) == 0)
				{
					if (srECROb->srTransData.szBarCode1[0] == ' ' && srECROb->srTransData.szBarCode2[0] == ' ')
					{
						if (ginDebug == VS_TRUE)
						{
							inLogPrintf(AT, "ECR未輸入條碼");
						}
						memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
						inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
						memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
						sprintf(pobTran->szErrorMsgBuff2, "ECR未輸入條碼");
						pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
						
						return (VS_ERROR);
					}
					
					/* 條碼兌換 防呆&阻擋&存Barcode */
					/* 11 表示只有一個一維條碼，且這是第1 個。 */
					if (srECROb->srTransData.szBarCode1[0] == '1' && srECROb->srTransData.szBarCode1[1] == '1')
					{
						memcpy(&pobTran->szL3_Barcode1[0], &srECROb->srTransData.szBarCode1[0], 20);
						sprintf(pobTran->szL3_Barcode1Len, "%02d", strlen(pobTran->szL3_Barcode1));
					}
					else if (srECROb->srTransData.szBarCode2[0] == '1' && srECROb->srTransData.szBarCode2[1] == '1')
					{
						memcpy(&pobTran->szL3_Barcode1[0], &srECROb->srTransData.szBarCode2[0], 20);
						sprintf(pobTran->szL3_Barcode1Len, "%02d", strlen(pobTran->szL3_Barcode1));
					}
					else if (srECROb->srTransData.szBarCode1[0] == '2' && srECROb->srTransData.szBarCode1[1] == '1')
					{
						if (srECROb->srTransData.szBarCode2[0] == '2' && srECROb->srTransData.szBarCode2[1] == '2')
						{
							memcpy(&pobTran->szL3_Barcode1[0], &srECROb->srTransData.szBarCode1[0], 20);
							sprintf(pobTran->szL3_Barcode1Len, "%02d", strlen(pobTran->szL3_Barcode1));
							memcpy(&pobTran->szL3_Barcode2[0], &srECROb->srTransData.szBarCode2[0], 20);
							sprintf(pobTran->szL3_Barcode2Len, "%02d", strlen(pobTran->szL3_Barcode2));
						}
						else
						{
							if (ginDebug == VS_TRUE)
							{
								inLogPrintf(AT, "ECR條碼邏輯錯誤");
							}
							memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
							inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
							memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
							sprintf(pobTran->szErrorMsgBuff2, "ECR條碼邏輯錯誤");
							pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
							
							return (VS_ERROR);
						}
					}
					else if (srECROb->srTransData.szBarCode1[0] == '2' && srECROb->srTransData.szBarCode1[1] == '2')
					{
						if (srECROb->srTransData.szBarCode2[0] == '2' && srECROb->srTransData.szBarCode2[1] == '1')
						{
							memcpy(&pobTran->szL3_Barcode2[0], &srECROb->srTransData.szBarCode1[0], 20);
							sprintf(pobTran->szL3_Barcode2Len, "%02d", strlen(pobTran->szL3_Barcode2));
							memcpy(&pobTran->szL3_Barcode1[0], &srECROb->srTransData.szBarCode2[0], 20);
							sprintf(pobTran->szL3_Barcode1Len, "%02d", strlen(pobTran->szL3_Barcode1));
						}
						else
						{
							if (ginDebug == VS_TRUE)
							{
								inLogPrintf(AT, "ECR條碼邏輯錯誤");
							}
							memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
							inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
							memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
							sprintf(pobTran->szErrorMsgBuff2, "ECR條碼邏輯錯誤");
							pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
							
							return (VS_ERROR);
						}
					}
					else
					{
						if (ginDebug == VS_TRUE)
						{
							inLogPrintf(AT, "ECR條碼邏輯錯誤");
						}
						memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
						inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
						memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
						sprintf(pobTran->szErrorMsgBuff2, "ECR條碼邏輯錯誤");
						pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
						
						return (VS_ERROR);
					}
				}
				break;
			default:
				break;
		}
	}
	else if (inTransType == _ECR_8N1_VOID_AWARD_REDEEM_NO_)
	{
		/* 兌換模式 */
		switch (inTransType)
		{
			case _ECR_8N1_VOID_AWARD_REDEEM_NO_:		/* 優惠兌換 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &srECROb->srTransData.szField_20[0], 1);	/* (1=條碼兌換，2=卡號兌換) */

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s :%s", "Award Redeem Mode", szTemplate);
					inLogPrintf(AT, szDebugMsg);
				}
				
				/* 條碼兌換 */
				if (memcmp(szTemplate, "1", 1) == 0)
				{
					memcpy(pobTran->szL3_AwardWay, "1", 1);
				}
				else
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "ECR取消兌換不接受卡號輸入");
					}
					memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
					inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
					memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
					sprintf(pobTran->szErrorMsgBuff2, "ECR取消兌換不接受卡號輸入");
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
					
					return (VS_ERROR);
				}
				
				/* 兌換方式為條碼才抓條碼*/
				if (memcmp(pobTran->szL3_AwardWay, "1", strlen("1")) == 0)
				{
					if (srECROb->srTransData.szBarCode1[0] == ' ' && srECROb->srTransData.szBarCode2[0] == ' ')
					{
						if (ginDebug == VS_TRUE)
						{
							inLogPrintf(AT, "ECR未輸入條碼");
						}
						memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
						inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
						memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
						sprintf(pobTran->szErrorMsgBuff2, "ECR未輸入條碼");
						pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
						
						return (VS_ERROR);
					}
					
					/* 條碼兌換 防呆&阻擋&存Barcode */
					/* 11 表示只有一個一維條碼，且這是第1 個。 */
					if (srECROb->srTransData.szBarCode1[0] == '1' && srECROb->srTransData.szBarCode1[1] == '1')
					{
						memcpy(&pobTran->szL3_Barcode1[0], &srECROb->srTransData.szBarCode1[0], 20);
						sprintf(pobTran->szL3_Barcode1Len, "%02d", strlen(pobTran->szL3_Barcode1));
					}
					else if (srECROb->srTransData.szBarCode2[0] == '1' && srECROb->srTransData.szBarCode2[1] == '1')
					{
						memcpy(&pobTran->szL3_Barcode1[0], &srECROb->srTransData.szBarCode2[0], 20);
						sprintf(pobTran->szL3_Barcode1Len, "%02d", strlen(pobTran->szL3_Barcode1));
					}
					else if (srECROb->srTransData.szBarCode1[0] == '2' && srECROb->srTransData.szBarCode1[1] == '1')
					{
						if (srECROb->srTransData.szBarCode2[0] == '2' && srECROb->srTransData.szBarCode2[1] == '2')
						{
							memcpy(&pobTran->szL3_Barcode1[0], &srECROb->srTransData.szBarCode1[0], 20);
							sprintf(pobTran->szL3_Barcode1Len, "%02d", strlen(pobTran->szL3_Barcode1));
							memcpy(&pobTran->szL3_Barcode2[0], &srECROb->srTransData.szBarCode2[0], 20);
							sprintf(pobTran->szL3_Barcode2Len, "%02d", strlen(pobTran->szL3_Barcode2));
						}
						else
						{
							if (ginDebug == VS_TRUE)
							{
								inLogPrintf(AT, "ECR條碼邏輯錯誤");
							}
							memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
							inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
							memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
							sprintf(pobTran->szErrorMsgBuff2, "ECR條碼邏輯錯誤");
							pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
							
							return (VS_ERROR);
						}
					}
					else if (srECROb->srTransData.szBarCode1[0] == '2' && srECROb->srTransData.szBarCode1[1] == '2')
					{
						if (srECROb->srTransData.szBarCode2[0] == '2' && srECROb->srTransData.szBarCode2[1] == '1')
						{
							memcpy(&pobTran->szL3_Barcode2[0], &srECROb->srTransData.szBarCode1[0], 20);
							sprintf(pobTran->szL3_Barcode2Len, "%02d", strlen(pobTran->szL3_Barcode2));
							memcpy(&pobTran->szL3_Barcode1[0], &srECROb->srTransData.szBarCode2[0], 20);
							sprintf(pobTran->szL3_Barcode1Len, "%02d", strlen(pobTran->szL3_Barcode1));
						}
						else
						{
							if (ginDebug == VS_TRUE)
							{
								inLogPrintf(AT, "ECR條碼邏輯錯誤");
							}
							memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
							inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
							memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
							sprintf(pobTran->szErrorMsgBuff2, "ECR條碼邏輯錯誤");
							pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
							
							return (VS_ERROR);
						}
					}
					else
					{
						if (ginDebug == VS_TRUE)
						{
							inLogPrintf(AT, "ECR條碼邏輯錯誤");
						}
						memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
						inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
						memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
						sprintf(pobTran->szErrorMsgBuff2, "ECR條碼邏輯錯誤");
						pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
							
						return (VS_ERROR);
					}
				}
				break;
			default:
				break;
		}
	}
	else
	{
		/* 實際支付金額RDM Paid Amt (含小數 2位) */
		switch (inTransType)
		{
			case _ECR_8N1_REDEEM_REFUND_NO_:		/* 紅利退貨 */
			case _ECR_8N1_REDEEM_ADJUST_NO_:		/* 紅利調帳 */
			case _ECR_8N1_HG_REWARD_REDEMPTION_NO_ :	/* 快樂購紅利積點 + 信用卡紅利扣抵 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &srECROb->srTransData.szRDMPaidAmt[0], 10);	/* 12位數，這裡只取10位 */
                                
                                /* 支付金額比交易金額大 */
                                if (atol(szTemplate) > pobTran->srBRec.lnTxnAmount)
                                {
                                        memset(srECROb->srTransData.szRDMPaidAmt, 0x00, sizeof(srECROb->srTransData.szRDMPaidAmt));
                                        memcpy(srECROb->srTransData.szRDMPaidAmt, "000000000000", 12);	/* 重輸 */
                                        
                                        memset(szTemplate, 0x00, sizeof(szTemplate));
                                        memcpy(szTemplate, &srECROb->srTransData.szRDMPaidAmt[0], 10);	/* 12位數，這裡只取10位 */
                                }

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s :%s", "RDM Paid Amt", szTemplate);
					inLogPrintf(AT, szDebugMsg);
				}

				pobTran->srBRec.lnRedemptionPaidCreditAmount = atol(szTemplate);

				break;
			default:
				break;
		}

		/* 信用卡紅利扣抵點數 RDM Point */
		switch (inTransType)
		{
			case _ECR_8N1_REDEEM_REFUND_NO_:		/* 紅利退貨 */
			case _ECR_8N1_REDEEM_ADJUST_NO_:		/* 紅利調帳 */
			case _ECR_8N1_HG_REWARD_REDEMPTION_NO_ :	/* 快樂購紅利積點 + 信用卡紅利扣抵 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &srECROb->srTransData.szRDMPoint[0], 8);

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s :%s", "RDM Point", szTemplate);
					inLogPrintf(AT, szDebugMsg);
				}

				pobTran->srBRec.lnRedemptionPoints = atol(szTemplate);

				break;
			default:
				break;
		}

		/* 分期期數 Installment Period */
		switch (inTransType)
		{
			case _ECR_8N1_INSTALLMENT_NO_:			/* 分期 */
			case _ECR_8N1_INSTALLMENT_REFUND_NO_:		/* 分期退貨 */
			case _ECR_8N1_INSTALLMENT_ADJUST_NO_:		/* 分期調帳 */
			case _ECR_8N1_HG_REWARD_INSTALLMENT_NO_ :	/* 快樂購紅利積點 + 信用卡分期付款 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &srECROb->srTransData.szInstallmentPeriod[0], 2);

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s :%s", "Installment Period", szTemplate);
					inLogPrintf(AT, szDebugMsg);
				}

				pobTran->srBRec.lnInstallmentPeriod = atol(szTemplate);

				break;
			default:
				break;
		}

		/* 首期金額 Down Payment Amount (含小數 2位) */
		switch (inTransType)
		{
			case _ECR_8N1_INSTALLMENT_REFUND_NO_:		/* 分期退貨 */
			case _ECR_8N1_INSTALLMENT_ADJUST_NO_:		/* 分期調帳 */
			case _ECR_8N1_HG_REWARD_INSTALLMENT_NO_ :	/* 快樂購紅利積點 + 信用卡分期付款 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &srECROb->srTransData.szField_26[0], 10);	/* 12位數，這裡只取10位 */

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s :%s", "Down Payment Amount", szTemplate);
					inLogPrintf(AT, szDebugMsg);
				}

				pobTran->srBRec.lnInstallmentDownPayment = atol(szTemplate);

				break;
			default:
				break;
		}

		/* 每期金額 Installment Payment Amount (含小數 2位) */
		switch (inTransType)
		{
			case _ECR_8N1_INSTALLMENT_REFUND_NO_:		/* 分期退貨 */
			case _ECR_8N1_INSTALLMENT_ADJUST_NO_:		/* 分期調帳 */
			case _ECR_8N1_HG_REWARD_INSTALLMENT_NO_ :	/* 快樂購紅利積點 + 信用卡分期付款 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &srECROb->srTransData.szField_27[0], 10);	/* 12位數，這裡只取10位 */

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s :%s", "Installment Payment Amount", szTemplate);
					inLogPrintf(AT, szDebugMsg);
				}

				pobTran->srBRec.lnInstallmentPayment = atol(szTemplate);

				break;
			default:
				break;
		}

		/* 分期手續費 Formallity Fee (含小數 2位) */
		switch (inTransType)
		{
			case _ECR_8N1_INSTALLMENT_REFUND_NO_:		/* 分期退貨 */
			case _ECR_8N1_INSTALLMENT_ADJUST_NO_:		/* 分期調帳 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &srECROb->srTransData.szField_28[0], 10);	/* 12位數，這裡只取10位 */

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s :%s", "Formallity Fee", szTemplate);
					inLogPrintf(AT, szDebugMsg);
				}
				pobTran->srBRec.lnInstallmentFormalityFee = atol(szTemplate);
				
				if (pobTran->srBRec.lnInstallmentFormalityFee != 0)
				{
					/* 分期退貨不能輸入手續費 */
					pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_INST_FEE_NOT_0_;
					pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;		/* 0004 操作錯誤 */
					return (VS_ERROR);
				}
				
				break;
			case _ECR_8N1_HG_REWARD_INSTALLMENT_NO_ :	/* 快樂購紅利積點 + 信用卡分期付款 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, &srECROb->srTransData.szField_28[0], 10);	/* 12位數，這裡只取10位 */

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s :%s", "Formallity Fee", szTemplate);
					inLogPrintf(AT, szDebugMsg);
				}

				pobTran->srBRec.lnInstallmentFormalityFee = atol(szTemplate);

				break;
			default:
				break;
		}

		/* 啟動卡號查詢 Start Trans Type */
		switch (inTransType)
		{
			case _ECR_8N1_START_CARD_NO_INQUIRY_NO_:		/* 卡號查詢 */	
				
				break;
			default:
				break;
		}

		/* 只有金融卡才要收的欄位 */
		if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
		{
			/* 金融卡原交易日期 SP Origin Date */
			switch (inTransType)
			{
				case _ECR_8N1_REFUND_NO_:				/* 退費 */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memcpy(szTemplate, &srECROb->srTransData.szField_34[0], 8);

					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s :%s", "SP Origin Date", szTemplate);
						inLogPrintf(AT, szDebugMsg);
					}

					if (atoi(szTemplate) > 0)
					{
						memcpy(pobTran->srBRec.szFiscRefundDate, szTemplate, 8);
					}
					
					break;
				default:
					break;
			}

			/* 金融卡調單編號 SP RRN */
			switch (inTransType)
			{
				case _ECR_8N1_REFUND_NO_:				/* 退費 */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memcpy(szTemplate, &srECROb->srTransData.szField_35[0], 12);	

					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s :%s", "SP RRN", szTemplate);
						inLogPrintf(AT, szDebugMsg);
					}

					if (atoi(szTemplate) > 0)
					{
						memcpy(pobTran->srBRec.szFiscRRN, szTemplate, 12);
					}

					break;
				default:
					break;
			}

		}
		/* 只有電票要收的欄位 */
		else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
		{
			/* ESVC Origin Date */
			switch (inTransType)
			{
				case _ECR_8N1_REFUND_NO_:				/* 電票退貨 */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memcpy(szTemplate, &srECROb->srTransData.szField_34[0], 8);	

					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s :%s", "ESVC Origin Date", szTemplate);
						inLogPrintf(AT, szDebugMsg);
					}

					if (atoi(szTemplate) > 0)
					{
						memcpy(pobTran->srTRec.szTicketRefundDate, &szTemplate[4], 4);
					}
					else
					{
/* [20251219_BUG_MDF][ECR][DATA] 修改ECR欄位34資料,因電票資料如果為空白會拒絕，跳過檢查步驟,進行後續人工輸入 */
#ifdef _ECR_CHECK_FILE_34_	
						/* 接收資料錯誤 */
						memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
						sprintf(pobTran->szErrorMsgBuff1, "_ECR_8N1_REFUND_NO_");
						memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
						sprintf(pobTran->szErrorMsgBuff2, "ESVC Origin Date");
						pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
						
						return (VS_ERROR);
#endif
					}

					break;
				default:
					break;
			}
			
			/* ATS電票交易序號 RF NUMBER 左靠右補空白 */
			switch (inTransType)
			{
				case _ECR_8N1_REFUND_NO_:				/* 電票退貨 */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memcpy(szTemplate, &srECROb->srTransData.szField_35[0], 12);

					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s :%s", "RF NUMBER", szTemplate);
						inLogPrintf(AT, szDebugMsg);
					}

					if (atoi(szTemplate) > 0)
					{
						memcpy(pobTran->srTRec.szTicketRefundCode, szTemplate, 12);
					}
					else
					{
						/* 接收資料錯誤 */
						memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
						sprintf(pobTran->szErrorMsgBuff1, "_ECR_8N1_REFUND_NO_");
						memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
						sprintf(pobTran->szErrorMsgBuff2, "ESVC RF NUMBER");
						pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
						
						return (VS_ERROR);
					}

					break;
				default:
					break;
			}
		}
		/* 只有銀聯要收的欄位 */
		else if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
		{
			/* CUP Origin Date */
			switch (inTransType)
			{
				case _ECR_8N1_REFUND_NO_:				/* 電票退貨 */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memcpy(szTemplate, &srECROb->srTransData.szField_34[0], 8);

					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s :%s", "CUP Origin Date", szTemplate);
						inLogPrintf(AT, szDebugMsg);
					}

					if (atoi(szTemplate) > 0	&&
					    inFunc_CheckValidDate_Include_Year(szTemplate) == VS_SUCCESS)
					{
						memcpy(pobTran->srBRec.szCUP_TD, &szTemplate[4], 4);
					}

					break;
				default:
					break;
			}
		}
	}
	
	/* 【需求單 - 104014】發卡機構on-us繳費平台需求 +【需求單 - 104152】發卡機構on-us繳費平台 Payitem Code by Russell 2020/3/16 下午 5:38 */
	memset(szPayItemFuncEnable, 0x00, sizeof(szPayItemFuncEnable));
	inGetPayItemEnable(szPayItemFuncEnable);
	if (memcmp(szPayItemFuncEnable, "Y", 1) == 0							&&
	    memcmp(&srECROb->srTransData.szTransType[0], _ECR_8N1_START_CARD_NO_INQUIRY_, 2) != 0)
	{
		/* PayItemFuncEnable有開才進入檢核收銀機送的繳費項目 */
		/* 兩段式收銀機連線 第一段不檢核Payitem Code */
		switch (inTransType)
		{
			case _ECR_8N1_SALE_NO_ :
			case _ECR_8N1_REFUND_NO_ :
			case _ECR_8N1_OFFLINE_NO_ :
			case _ECR_8N1_PREAUTH_NO_ :
			case _ECR_8N1_PREAUTH_COMPLETE_NO_ :
			case _ECR_8N1_INSTALLMENT_NO_ :
			case _ECR_8N1_INSTALLMENT_REFUND_NO_ :
			case _ECR_8N1_REDEEM_NO_ :
			case _ECR_8N1_REDEEM_REFUND_NO_ :
				srECROb->srTransData.uszECRResponsePayitem = VS_TRUE;
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memcpy(szTemplate, srECROb->srTransData.szPayItem, 5);

				/* 先判斷是不是送5個空白 */
				if (!memcmp(szTemplate, "     ", 5))
				{
					/* 空白不必回傳payitem */
					if (ginDebug == VS_TRUE)
					{
						srECROb->srTransData.uszECRResponsePayitem = VS_FALSE;
						inLogPrintf(AT, "ECR not input PayItem");
					}
					break;
				}

				/* 比對端末機的PIT Table PayItemCode */
				/* 吳升文(Angus Wu) 2015-09-15 (週二) 下午 04:43 回覆 帶5個0視為有值，所以要比對。 */
				for (i = 0 ;; i++)
				{
					/* 吳升文(Angus Wu) 2015-09-15 (週二) 下午 04:24 回覆
						因為繳費項目的代碼會從授權一直帶到清算再回到發卡行，然後發卡行根據繳費項目代碼跟持卡人收費。
						所以不合法的繳費代碼端末機不可以後送，免得後端勾稽不到而收不到錢。
						因此比對不到繳費代碼則端末機提示”不支援該繳費項目”後，回傳Response Code=0004。
					*/
					/* 比對不到不進入輸入keymap畫面 在ECR連線阻檔 */
					if (inLoadPITRec(i) < 0)
					{
						/* 不支援該繳費項目 */
						pobTran->inErrorMsg = _ERROR_CODE_V3_NOT_SUPPORT_THIS_PAY_ITEM_;
						pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
						
						return (VS_ERROR);
					}

					/* 有比對到端末機的繳費項目 */
					memset(szPayItemCodeTemp, 0x00, sizeof(szPayItemCodeTemp));
					inGetPayItemCode(szPayItemCodeTemp);
					if (memcmp(szTemplate, szPayItemCodeTemp, 5) == 0)
					{
						memset(pobTran->srBRec.szPayItemCode, 0x00, sizeof(pobTran->srBRec.szPayItemCode));
						memcpy(&pobTran->srBRec.szPayItemCode[0], &szTemplate[0], 5);
						break;
					}
				}

				break;
			default :
				break;
		}
	}
	
	/* 處理HG */
	/* 支付工具 */
	switch (inTransType)
	{
		case _ECR_8N1_HG_REWARD_SALE_NO_ :
		case _ECR_8N1_HG_ONLINE_REDEEMPTION_NO_ :
		case _ECR_8N1_HG_POINT_CERTAIN_NO_ :
		case _ECR_8N1_HG_REWARD_INSTALLMENT_NO_ :
		case _ECR_8N1_HG_REWARD_REDEMPTION_NO_ :
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "HG Payment Tool", srECROb->srTransData.szHGPaymentTool);
				inLogPrintf(AT, szDebugMsg);
			}
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(srECROb->srTransData.szHGPaymentTool) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "HG Payment Tool", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "HG Payment Tool Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

				return (VS_ERROR);
			}
			
			if (memcmp(srECROb->srTransData.szHGPaymentTool, _ECR_8N1_HG_PAYMENT_TOOL_TYPE_EDC_CHOOSE_, 2) == 0)
			{
				pobTran->srBRec.lnHGPaymentType = 0;
			}
			else if (memcmp(srECROb->srTransData.szHGPaymentTool, _ECR_8N1_HG_PAYMENT_TOOL_TYPE_CREDIT_CARD_, 2) == 0)
			{
				pobTran->srBRec.lnHGPaymentType = _HG_PAY_CREDIT_;
			}
			else if (memcmp(srECROb->srTransData.szHGPaymentTool, _ECR_8N1_HG_PAYMENT_TOOL_TYPE_CASH_, 2) == 0)
			{
				pobTran->srBRec.lnHGPaymentType = _HG_PAY_CASH_;
			}
			else if (memcmp(srECROb->srTransData.szHGPaymentTool, _ECR_8N1_HG_PAYMENT_TOOL_TYPE_GIFT_PAPER_, 2) == 0)
			{
				pobTran->srBRec.lnHGPaymentType = _HG_PAY_GIFT_PAPER_;
			}
			else if (memcmp(srECROb->srTransData.szHGPaymentTool, _ECR_8N1_HG_PAYMENT_TOOL_TYPE_HGI_, 2) == 0)
			{
				pobTran->srBRec.lnHGPaymentType = _HG_PAY_CREDIT_INSIDE_;
			}
			else if (memcmp(srECROb->srTransData.szHGPaymentTool, _ECR_8N1_HG_PAYMENT_TOOL_TYPE_CUP_CARD_, 2) == 0)
			{
				pobTran->srBRec.lnHGPaymentType = _HG_PAY_CUP_;
			}
			else
			{
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "HG Payment Tool Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;
			}
			break;
		default :
			break;
	}
	
	/* Happy Go 扣抵點數 */
	switch (inTransType)
	{
		case _ECR_8N1_HG_POINT_CERTAIN_NO_ :
		case _ECR_8N1_HG_FULL_REDEEMPTION_NO_ :
		case _ECR_8N1_HG_REDEEM_REFUND_NO_ :
		case _ECR_8N1_HG_REWARD_REFUND_NO_ :
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s :%s", "HG Redeem Point", srECROb->srTransData.szHGRedeemPoint);
				inLogPrintf(AT, szDebugMsg);
			}
			
			/* 欄位為M 卻不存在，回傳錯誤 */
			if (inFunc_CheckFullSpace(srECROb->srTransData.szHGRedeemPoint) == VS_TRUE)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "%s: %s  ", "HG Redeem Point", "Not Exist Error");
					inLogPrintf(AT, szDebugMsg);
				}
				memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
				inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
				memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
				sprintf(pobTran->szErrorMsgBuff2, "HG Redeem Point Error");
				pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

				return (VS_ERROR);
			}
			
			pobTran->srBRec.lnHGTransactionPoint = atol(srECROb->srTransData.szHGRedeemPoint);
			break;
		default :
			break;
	}
	
	if (gbBarCodeECRBit == VS_TRUE)
	{ 
		/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/7 上午 10:28 */
                /* 一維或二維條碼資料 */
                switch (inTransType)
                {
                        case _ECR_8N1_END_CARD_NO_INQUIRY_NO_ :
                        case _ECR_8N1_SALE_NO_:
                        case _ECR_8N1_INSTALLMENT_NO_:
                        case _ECR_8N1_REDEEM_NO_:
                        case _ECR_8N1_EW_INQUIRY_TRANSACTION_NO_:
                                if (ginDebug == VS_TRUE)
                                {
                                        memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                                        sprintf(szDebugMsg, "%s :%s", "BarCode Data Len", srECROb->srTransData.szBarCodeLen);
                                        inLogPrintf(AT, szDebugMsg);
                                        memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                                        sprintf(szDebugMsg, "%s :%s", "BarCode Data", srECROb->srTransData.szBarCodeData);
                                        inLogPrintf(AT, szDebugMsg);
                                }

                                if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
                                {
                                        /* 欄位為M 卻不存在，回傳錯誤 */
                                        if (inFunc_CheckFullSpace(srECROb->srTransData.szBarCodeLen) == VS_TRUE)
                                        {
                                                if (ginDebug == VS_TRUE)
                                                {
                                                        memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                                                        sprintf(szDebugMsg, "%s: %s  ", "BarCode Data Len", "Not Exist Error");
                                                        inLogPrintf(AT, szDebugMsg);
                                                }
                                                memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
                                                inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
                                                memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
                                                sprintf(pobTran->szErrorMsgBuff2, "BarCode Data Len Error");
                                                pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

                                                return (VS_ERROR);
                                        }
                                }

                                /* 欄位為M 卻不存在，回傳錯誤 */
                                if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
                                {
                                        if (inNCCC_Func_Check_Uny_Format(srECROb->srTransData.szBarCodeData) == VS_FALSE)
                                        {
                                                if (ginDebug == VS_TRUE)
                                                {
                                                        memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                                                        sprintf(szDebugMsg, "%s: %s  ", "BarCode Data", "Not Uny Format");
                                                        inLogPrintf(AT, szDebugMsg);
                                                }
                                                memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
                                                inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
                                                memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
                                                sprintf(pobTran->szErrorMsgBuff2, "Not Uny Format");
                                                pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

                                                return (VS_ERROR);
                                        }
                                }

                                inBarCodeLen = atoi(srECROb->srTransData.szBarCodeLen);
                                if (inBarCodeLen > 0)
                                {
                                        if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
                                        {
                                                memcpy(pobTran->srBRec.szBarCodeData, srECROb->srTransData.szBarCodeData, inBarCodeLen);
                                        }
                                        else 
                                        {
                                                memcpy(pobTran->srBRec.szBarCodeData, srECROb->srTransData.szBarCodeData, inBarCodeLen);
                                        }
                                }

                                break;
                        case _ECR_8N1_REFUND_NO_:
                                if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
                                {
                                        /* uny退貨不檢核barCode*/
                                }
                                else
                                {
                                        if (ginDebug == VS_TRUE)
                                        {
                                                memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                                                sprintf(szDebugMsg, "%s :%s", "BarCode Data Len", srECROb->srTransData.szBarCodeLen);
                                                inLogPrintf(AT, szDebugMsg);
                                                memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                                                sprintf(szDebugMsg, "%s :%s", "BarCode Data", srECROb->srTransData.szBarCodeData);
                                                inLogPrintf(AT, szDebugMsg);
                                        }

                                        inBarCodeLen = atoi(srECROb->srTransData.szBarCodeLen);
                                        if (inBarCodeLen > 0)
                                        {
                                                memcpy(pobTran->srBRec.szBarCodeData, srECROb->srTransData.szBarCodeData, inBarCodeLen);
                                        }
                                }

                                break;
                        default :
                                break;
                }
	
                if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
		{
                        /* Uny交易碼 */
                        switch (inTransType)
                        {
                                case _ECR_8N1_REFUND_NO_ :
                                case _ECR_8N1_INSTALLMENT_REFUND_NO_ :
                                case _ECR_8N1_REDEEM_REFUND_NO_ :
                                        if (ginDebug == VS_TRUE)
                                        {
                                                memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                                                sprintf(szDebugMsg, "%s :%s", "Uny Trans Code", srECROb->srTransData.szUnyTransCode);
                                                inLogPrintf(AT, szDebugMsg);
                                        }

                                        /* 欄位為M 卻不存在，回傳錯誤 */
                                        if (inFunc_CheckFullSpace(srECROb->srTransData.szUnyTransCode) == VS_TRUE)
                                        {
                                                if (ginDebug == VS_TRUE)
                                                {
                                                        memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                                                        sprintf(szDebugMsg, "%s: %s  ", "Uny Trans Code", "Not Exist Error");
                                                        inLogPrintf(AT, szDebugMsg);
                                                }
                                                memset(pobTran->szErrorMsgBuff1, 0x00, sizeof(pobTran->szErrorMsgBuff1));
                                                inECR_Transform_8N1_No_To_String(inTransType, pobTran->szErrorMsgBuff1);
                                                memset(pobTran->szErrorMsgBuff2, 0x00, sizeof(pobTran->szErrorMsgBuff2));
                                                sprintf(pobTran->szErrorMsgBuff2, "Uny Trans Code Error");
                                                pobTran->inErrorMsg = _ERROR_CODE_V3_ECR_UNPACK_;

                                                return (VS_ERROR);
                                        }

                                        memcpy(pobTran->srBRec.szUnyTransCode, srECROb->srTransData.szUnyTransCode, 20);
                                        break;
                                default :
                                        break;
                        }
                }
	}
	
	if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
	{
		/* 有開CUP且MACEnable有開但安全認證沒過，不能執行CUP交易 */
		if (inKMS_CheckKey(_TWK_KEYSET_NCCC_, _TWK_KEYINDEX_NCCC_MAC_) != VS_SUCCESS)
		{
			if (inNCCC_Func_CUP_PowerOn_LogOn(pobTran) != VS_SUCCESS)
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "安全認證失敗，不能使用此功能");
				}
				/* 安全認證失敗 */
				pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
				
				return (VS_ERROR);
			}
		}
	}
	
	if (inECR_Check_Exception(pobTran) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	memset(szFiscContactlessEnable, 0x00, sizeof(szFiscContactlessEnable));
	inGetSmartPayContactlessEnable(szFiscContactlessEnable);
	memset(szCTLSEnable, 0x00, sizeof(szCTLSEnable));
	inGetContactlessEnable(szCTLSEnable);
	memset(szCUPContactlessEnable, 0x00, sizeof(szCUPContactlessEnable));
	inGetCUPContactlessEnable(szCUPContactlessEnable);
	
	/* 跑OPT */
	switch (inTransType)
	{
		case _ECR_8N1_SALE_NO_:
			/* 金融卡和一般Sale跑不同OPT，所以要分 */
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				/* 第三層顯示 ＜消費扣款＞ */
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_FISC_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜消費扣款＞ */

				pobTran->inFunctionID = _FISC_SALE_;
				if (!memcmp(szCTLSEnable, "Y", 1)		&&
				    !memcmp(szFiscContactlessEnable, "Y", 1)	&&
				    guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_FISC_SALE_CTLS_;
					pobTran->inRunTRTID = _TRT_FISC_SALE_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_FISC_SALE_;
					pobTran->inRunTRTID = _TRT_FISC_SALE_ICC_;
				}
		
				pobTran->inTransactionCode = _FISC_SALE_;
				pobTran->srBRec.inCode = _FISC_SALE_;
				pobTran->srBRec.inOrgCode = _FISC_SALE_;
				
				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_CUP_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜銀聯一般交易＞ */

				pobTran->inFunctionID = _CUP_SALE_;
				
				if (!memcmp(szCTLSEnable, "Y", 1)		&&
				    !memcmp(szCUPContactlessEnable, "Y", 1)	&&
				    guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
					pobTran->inRunTRTID = _TRT_CUP_SALE_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
					pobTran->inRunTRTID = _TRT_CUP_SALE_;
				}

				pobTran->inTransactionCode = _CUP_SALE_;
				pobTran->srBRec.inCode = _CUP_SALE_;
				pobTran->srBRec.inOrgCode = _CUP_SALE_;
				
				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_TICKET_DEDUCT_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜購貨交易＞ */

				pobTran->inFunctionID = _TICKET_DEDUCT_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					/* (需求單 - 107227)邦柏科技自助作業客製化 悠遊卡0018重試功能 by Russell 2018/12/14 上午 9:30 */
					if (pobTran->uszLastTranscationBit == VS_TRUE)
					{
						pobTran->inRunOperationID = _OPERATION_TICKET_RETRY_;
					}
					else
					{
						pobTran->inRunOperationID = _OPERATION_TICKET_DEDUCT_;
					}
				}
				else
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "感應未初始化，不能使用此功能");
					}
					return (VS_ERROR);
				}

				pobTran->inTransactionCode = _TICKET_DEDUCT_;
				pobTran->srTRec.inCode = _TICKET_DEDUCT_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else
			{
				inDISP_ClearAll();
				/* 第三層顯示 ＜一般交易＞ */
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般交易＞ */

				pobTran->inFunctionID = _SALE_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
					pobTran->inRunTRTID = _TRT_SALE_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
					pobTran->inRunTRTID = _TRT_SALE_;
				}

				pobTran->inTransactionCode = _SALE_;
				pobTran->srBRec.inCode = _SALE_;
				pobTran->srBRec.inOrgCode = _SALE_;
				
				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			
			break;
		case _ECR_8N1_VOID_NO_:
			/* 金融卡和一般Sale跑不同OPT，所以要分 */
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_FISC_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜沖正交易＞ */

				pobTran->inFunctionID = _FISC_VOID_;
				pobTran->inRunOperationID = _OPERATION_VOID_;
				pobTran->inRunTRTID = _TRT_FISC_VOID_;

				pobTran->inTransactionCode = _FISC_VOID_;
				pobTran->srBRec.inCode = _FISC_VOID_;
				pobTran->srBRec.inOrgCode = _FISC_VOID_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜取消交易＞ */

				pobTran->inRunOperationID = _OPERATION_VOID_;
				pobTran->inRunTRTID = _TRT_CUP_VOID_;

				pobTran->inFunctionID = _CUP_VOID_;
				pobTran->inTransactionCode = _CUP_VOID_;
				pobTran->srBRec.inCode = pobTran->inTransactionCode;
				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜取消交易＞ */

				pobTran->inFunctionID = _VOID_;
				pobTran->inRunOperationID = _OPERATION_VOID_;
				pobTran->inRunTRTID = _TRT_VOID_;

				pobTran->inTransactionCode = _VOID_;
				pobTran->srBRec.inCode = _VOID_;
				pobTran->srBRec.inOrgCode = _VOID_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			break;
		case _ECR_8N1_REFUND_NO_:
			/* 金融卡和一般Sale跑不同OPT，所以要分 */
			if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_FISC_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);	/* 第二層顯示 ＜退費交易＞ */

				pobTran->inFunctionID = _FISC_REFUND_;                     
				if (!memcmp(szCTLSEnable, "Y", 1)		&&
				    !memcmp(szFiscContactlessEnable, "Y", 1)	&&
				    guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_FISC_REFUND_CTLS_;
					pobTran->inRunTRTID = _TRT_FISC_REFUND_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_FISC_REFUND_;
					pobTran->inRunTRTID = _TRT_FISC_REFUND_ICC_;
				}

				pobTran->inTransactionCode = _FISC_REFUND_;
				pobTran->srBRec.inCode = _FISC_REFUND_;
				pobTran->srBRec.inOrgCode = _FISC_REFUND_;
				
				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			else if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般退貨＞ */

				if (!memcmp(szCTLSEnable, "Y", 1)		&&
				    !memcmp(szCUPContactlessEnable, "Y", 1)	&&
				    guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_GET_CARD_FIRST_;
					pobTran->inRunTRTID = _TRT_CUP_REFUND_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_AMOUNT_FIRST_CUP_;
					pobTran->inRunTRTID = _TRT_CUP_REFUND_;
				}

				pobTran->inFunctionID = _CUP_REFUND_;
				pobTran->inTransactionCode = _CUP_REFUND_;
				pobTran->srBRec.inCode = pobTran->inTransactionCode;
				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
				
				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_TICKET_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜退貨交易＞ */

				pobTran->inFunctionID = _TICKET_REFUND_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					/* (需求單 - 107227)邦柏科技自助作業客製化 悠遊卡0018重試功能 by Russell 2018/12/14 上午 9:30 */
					if (pobTran->uszLastTranscationBit == VS_TRUE)
					{
						pobTran->inRunOperationID = _OPERATION_TICKET_RETRY_;
					}
					else
					{
						pobTran->inRunOperationID = _OPERATION_TICKET_REFUND_;
					}
				}
				else
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "感應未初始化，不能使用此功能");
					}
					return (VS_ERROR);
				}

				pobTran->inTransactionCode = _TICKET_REFUND_;
				pobTran->srTRec.inCode = _TICKET_REFUND_;

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			}
			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般退貨＞ */

				pobTran->inFunctionID = _REFUND_;

				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_GET_CARD_FIRST_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_AMOUNT_FIRST_;
				}
				pobTran->inRunTRTID = _TRT_REFUND_;

				pobTran->inTransactionCode = _REFUND_;
				pobTran->srBRec.inCode = _REFUND_;
				pobTran->srBRec.inOrgCode = _REFUND_;
				
				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;
			
		case _ECR_8N1_OFFLINE_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_SALE_OFFLINE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜交易補登＞ */
			
			pobTran->inFunctionID = _SALE_OFFLINE_;
			pobTran->inRunOperationID = _OPERATION_SALE_OFFLINE_;
			pobTran->inRunTRTID = _TRT_SALE_OFFLINE_;

			pobTran->inTransactionCode = _SALE_OFFLINE_;
			pobTran->srBRec.inCode = _SALE_OFFLINE_;
			pobTran->srBRec.inOrgCode = _SALE_OFFLINE_;
			
			/* 卡號查詢不過第二次卡，所以要直接決定TRT */
			if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
			{
				inNCCC_Func_Decide_CTLS_TRT(pobTran);
			}
			else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
			{
				inNCCC_Func_Decide_ICC_TRT(pobTran);
			}
			else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
			{
				inNCCC_Func_Decide_MEG_TRT(pobTran);
			}
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			break;
			
		case _ECR_8N1_PREAUTH_NO_:
			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_PRE_AUTH_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜預先授權＞ */

				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_PRE_AUTH_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_PRE_AUTH_;
				}
				pobTran->inRunTRTID = _TRT_CUP_PRE_AUTH_;

				pobTran->inFunctionID = _CUP_PRE_AUTH_;
				pobTran->inTransactionCode = _CUP_PRE_AUTH_;
				pobTran->srBRec.inCode = pobTran->inTransactionCode;
				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
				
				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_PRE_AUTH_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜預先授權＞ */

				pobTran->inFunctionID = _PRE_AUTH_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_PRE_AUTH_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_PRE_AUTH_;
				}
				pobTran->inRunTRTID = _TRT_PRE_AUTH_;

				pobTran->inTransactionCode = _PRE_AUTH_;
				pobTran->srBRec.inCode = _PRE_AUTH_;
				pobTran->srBRec.inOrgCode = _PRE_AUTH_;
				
				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			
			break;
			
		case _ECR_8N1_PREAUTH_CANCEL_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_PRE_AUTH_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜預先授權取消＞ */
			
			pobTran->inFunctionID = _CUP_PRE_AUTH_VOID_;
			pobTran->inRunOperationID = _OPERATION_VOID_;
			pobTran->inRunTRTID = _TRT_CUP_PRE_AUTH_VOID_;

			pobTran->inTransactionCode = _CUP_PRE_AUTH_VOID_;
			pobTran->srBRec.inCode = _CUP_PRE_AUTH_VOID_;
			pobTran->srBRec.inOrgCode = _CUP_PRE_AUTH_VOID_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			break;
			
		case _ECR_8N1_PREAUTH_COMPLETE_NO_:
			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_PRE_COMP_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜預先授權完成＞ */

				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_PRE_COMP_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_PRE_COMP_;
				}
				pobTran->inRunTRTID = _TRT_CUP_PRE_COMP_;

				pobTran->inFunctionID = _CUP_PRE_COMP_;
				pobTran->inTransactionCode = _CUP_PRE_COMP_;
				pobTran->srBRec.inCode = pobTran->inTransactionCode;
				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
				
				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_PRE_COMP_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜預先授權完成＞ */

				pobTran->inFunctionID = _PRE_COMP_;
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_PRE_COMP_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_PRE_COMP_;
				}
				pobTran->inRunTRTID = _TRT_PRE_COMP_;

				pobTran->inTransactionCode = _PRE_COMP_;
				pobTran->srBRec.inCode = _PRE_COMP_;
				pobTran->srBRec.inOrgCode = _PRE_COMP_;
				
				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;
			
		case _ECR_8N1_PREAUTH_COMPLETE_CANCEL_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);					/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_PRE_COMP_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜預先授權完成取消＞ */
			
			pobTran->inFunctionID = _CUP_PRE_COMP_VOID_;
			pobTran->inRunOperationID = _OPERATION_VOID_;
			pobTran->inRunTRTID = _TRT_CUP_PRE_COMP_VOID_;

			pobTran->inTransactionCode = _CUP_PRE_COMP_VOID_;
			pobTran->srBRec.inCode = _CUP_PRE_COMP_VOID_;
			pobTran->srBRec.inOrgCode = _CUP_PRE_COMP_VOID_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			break;
			
		case _ECR_8N1_INSTALLMENT_NO_:
//			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
//			{
//				inDISP_ClearAll();
//				/* 保留給銀聯分期使用 */
//				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
//				inDISP_PutGraphic(_MENU_INST_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜分期付款＞ */
//
//				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
//				{
//					pobTran->inRunOperationID = _OPERATION_INST_SALE_CTLS_;
//				}
//				else
//				{
//					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
//				}
//				pobTran->inRunTRTID = _TRT_INST_SALE_;
//
//				pobTran->inFunctionID = _CUP_INST_SALE_;
//				pobTran->inTransactionCode = _CUP_INST_SALE_;
//				pobTran->srBRec.inCode = pobTran->inTransactionCode;
//				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
//
//				pobTran->srBRec.uszInstallmentBit = VS_TRUE;
//				
//				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
//				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
//				{
//					inNCCC_Func_Decide_CTLS_TRT(pobTran);
//				}
//				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
//				{
//					inNCCC_Func_Decide_ICC_TRT(pobTran);
//				}
//				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
//				{
//					inNCCC_Func_Decide_MEG_TRT(pobTran);
//				}
//
//				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
//			}
//			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_INST_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜分期付款＞ */

				pobTran->inFunctionID = _INST_SALE_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_INST_SALE_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
				}
				pobTran->inRunTRTID = _TRT_INST_SALE_;

				pobTran->inTransactionCode = _INST_SALE_;
				pobTran->srBRec.inCode = _INST_SALE_;
				pobTran->srBRec.inOrgCode = _INST_SALE_;

				pobTran->srBRec.uszInstallmentBit = VS_TRUE;
				
				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;
			
		case _ECR_8N1_INSTALLMENT_REFUND_NO_:
//			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
//			{
//				inDISP_ClearAll();
//				/* 保留給銀聯分期退貨使用 */
//				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
//				inDISP_PutGraphic(_MENU_INST_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜分期退貨＞ */
//
//				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
//				{
//					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_GET_CARD_FIRST_;
//				}
//				else
//				{
//					pobTran->inRunOperationID = _OPERATION_REFUND_;
//				}
//				pobTran->inRunTRTID = _TRT_INST_REFUND_;
//
//				pobTran->inFunctionID = _CUP_INST_REFUND_;
//				pobTran->inTransactionCode = _CUP_INST_REFUND_;
//				pobTran->srBRec.inCode = pobTran->inTransactionCode;
//				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
//
//				pobTran->srBRec.uszInstallmentBit = VS_TRUE;
//				
//				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
//				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
//				{
//					inNCCC_Func_Decide_CTLS_TRT(pobTran);
//				}
//				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
//				{
//					inNCCC_Func_Decide_ICC_TRT(pobTran);
//				}
//				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
//				{
//					inNCCC_Func_Decide_MEG_TRT(pobTran);
//				}
//
//				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
//			}
//			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_INST_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜分期退貨＞ */

				pobTran->inFunctionID = _INST_REFUND_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_GET_CARD_FIRST_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_;
				}
				pobTran->inRunTRTID = _TRT_INST_REFUND_;

				pobTran->inTransactionCode = _INST_REFUND_;
				pobTran->srBRec.inCode = _INST_REFUND_;
				pobTran->srBRec.inOrgCode = _INST_REFUND_;

				pobTran->srBRec.uszInstallmentBit = VS_TRUE;
				
				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;
			
		case _ECR_8N1_INSTALLMENT_ADJUST_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_INST_ADJUST_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜分期調帳＞ */
			
			pobTran->inFunctionID = _INST_ADJUST_;
			pobTran->inRunOperationID = _OPERATION_INST_ADJUST_;
			pobTran->inRunTRTID = _TRT_INST_ADJUST_;

			pobTran->inTransactionCode = _INST_ADJUST_;
			pobTran->srBRec.inCode = _INST_ADJUST_;
			pobTran->srBRec.inOrgCode = _INST_ADJUST_;
			
			pobTran->srBRec.uszInstallmentBit = VS_TRUE;
			
			/* 卡號查詢不過第二次卡，所以要直接決定TRT */
			if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
			{
				inNCCC_Func_Decide_CTLS_TRT(pobTran);
			}
			else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
			{
				inNCCC_Func_Decide_ICC_TRT(pobTran);
			}
			else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
			{
				inNCCC_Func_Decide_MEG_TRT(pobTran);
			}
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID); 
			break;
			
		case _ECR_8N1_REDEEM_NO_:
//			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
//			{
//				inDISP_ClearAll();
//				/* 保留給銀聯紅利扣抵使用 */
//				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
//				inDISP_PutGraphic(_MENU_REDEEM_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利扣抵＞ */
//
//				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
//				{
//					pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
//				}
//				else
//				{
//					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
//				}
//				pobTran->inRunTRTID = _TRT_REDEEM_SALE_;
//
//				pobTran->inFunctionID = _CUP_REDEEM_SALE_;
//				pobTran->inTransactionCode = _CUP_REDEEM_SALE_;
//				pobTran->srBRec.inCode = pobTran->inTransactionCode;
//				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
//
//				pobTran->srBRec.uszRedeemBit = VS_TRUE;
//				
//				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
//				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
//				{
//					inNCCC_Func_Decide_CTLS_TRT(pobTran);
//				}
//				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
//				{
//					inNCCC_Func_Decide_ICC_TRT(pobTran);
//				}
//				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
//				{
//					inNCCC_Func_Decide_MEG_TRT(pobTran);
//				}
//
//				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
//			}
//			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REDEEM_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利扣抵＞ */

				pobTran->inFunctionID = _REDEEM_SALE_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_SALE_CTLS_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_SALE_ICC_;
				}
				pobTran->inRunTRTID = _TRT_REDEEM_SALE_;

				pobTran->inTransactionCode = _REDEEM_SALE_;
				pobTran->srBRec.inCode = _REDEEM_SALE_;
				pobTran->srBRec.inOrgCode = _REDEEM_SALE_;

				pobTran->srBRec.uszRedeemBit = VS_TRUE;
				
				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;
		
		case _ECR_8N1_REDEEM_REFUND_NO_:
//			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
//			{
//				inDISP_ClearAll();
//				/* 保留給銀聯紅利退貨使用 */
//				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
//				inDISP_PutGraphic(_MENU_REDEEM_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利退貨＞ */
//
//				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
//				{
//					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_GET_CARD_FIRST_;
//				}
//				else
//				{
//					pobTran->inRunOperationID = _OPERATION_REFUND_;
//				}
//				pobTran->inRunTRTID = _TRT_REDEEM_REFUND_;
//
//				pobTran->inFunctionID = _CUP_REDEEM_REFUND_;
//				pobTran->inTransactionCode = _CUP_REDEEM_REFUND_;
//				pobTran->srBRec.inCode = pobTran->inTransactionCode;
//				pobTran->srBRec.inOrgCode = pobTran->inTransactionCode;
//
//				pobTran->srBRec.uszRedeemBit = VS_TRUE;
//				
//				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
//				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
//				{
//					inNCCC_Func_Decide_CTLS_TRT(pobTran);
//				}
//				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
//				{
//					inNCCC_Func_Decide_ICC_TRT(pobTran);
//				}
//				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
//				{
//					inNCCC_Func_Decide_MEG_TRT(pobTran);
//				}
//
//				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
//			}
//			else
			{
				inDISP_ClearAll();
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_REDEEM_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利退貨＞ */

				pobTran->inFunctionID = _REDEEM_REFUND_;
				
				if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_CTLS_GET_CARD_FIRST_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_REFUND_;
				}
				pobTran->inRunTRTID = _TRT_REDEEM_REFUND_;

				pobTran->inTransactionCode = _REDEEM_REFUND_;
				pobTran->srBRec.inCode = _REDEEM_REFUND_;
				pobTran->srBRec.inOrgCode = _REDEEM_REFUND_;

				pobTran->srBRec.uszRedeemBit = VS_TRUE;
				
				/* 卡號查詢不過第二次卡，所以要直接決定TRT */
				if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
				{
					inNCCC_Func_Decide_CTLS_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
				{
					inNCCC_Func_Decide_ICC_TRT(pobTran);
				}
				else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
				{
					inNCCC_Func_Decide_MEG_TRT(pobTran);
				}

				inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			}
			break;
			
		case _ECR_8N1_REDEEM_ADJUST_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_REDEEM_ADJUST_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利調帳＞ */
			
			pobTran->inFunctionID = _REDEEM_ADJUST_;
			pobTran->inRunOperationID = _OPERATION_REDEEM_ADJUST_;
			pobTran->inRunTRTID = _TRT_REDEEM_ADJUST_;

			pobTran->inTransactionCode = _REDEEM_ADJUST_;
			pobTran->srBRec.inCode = _REDEEM_ADJUST_;
			pobTran->srBRec.inOrgCode = _REDEEM_ADJUST_;
			
			pobTran->srBRec.uszRedeemBit = VS_TRUE;
			
			/* 卡號查詢不過第二次卡，所以要直接決定TRT */
			if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CTLS_)
			{
				inNCCC_Func_Decide_CTLS_TRT(pobTran);
			}
			else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_CHIP_)
			{
				inNCCC_Func_Decide_ICC_TRT(pobTran);
			}
			else if (inCardInquiryTRTType == _CARD_INQUIRY_TYPE_MEG_)
			{
				inNCCC_Func_Decide_MEG_TRT(pobTran);
			}
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
			
		case _ECR_8N1_TIP_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_TIP_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜小費交易＞ */
			
			pobTran->inFunctionID = _TIP_;
			pobTran->inRunOperationID = _OPERATION_TIP_;
			pobTran->inRunTRTID = _TRT_TIP_;

			pobTran->inTransactionCode = _TIP_;
			pobTran->srBRec.inCode = _TIP_;
			pobTran->srBRec.inOrgCode = _TIP_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
			
		case _ECR_8N1_AWARD_REDEEM_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_LOYALTY_REDEEM_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜優惠兌換＞ */
			
			pobTran->inFunctionID = _LOYALTY_REDEEM_;
			/* 收銀機以條碼兌換*/
			if (pobTran->szL3_AwardWay[0] == '1')
			{
				pobTran->inRunOperationID = _OPERATION_BARCODE_;
			}
			else
			{
				pobTran->inRunOperationID = _OPERATION_LOYALTY_REDEEM_CTLS_;
			}
			pobTran->inRunTRTID = _TRT_LOYALTY_REDEEM_;

			pobTran->inTransactionCode = _LOYALTY_REDEEM_;
			pobTran->srBRec.inCode = _LOYALTY_REDEEM_;
			pobTran->srBRec.inOrgCode = _LOYALTY_REDEEM_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
			
		case _ECR_8N1_VOID_AWARD_REDEEM_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_VOID_LOYALTY_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜兌換取消＞ */
			
			pobTran->inFunctionID = _VOID_LOYALTY_REDEEM_;
			pobTran->inRunOperationID = _OPERATION_VOID_LOYALTY_REDEEM_;
			pobTran->inRunTRTID = _TRT_LOYALTY_REDEEM_;

			pobTran->inTransactionCode = _VOID_LOYALTY_REDEEM_;
			pobTran->srBRec.inCode = _VOID_LOYALTY_REDEEM_;
			pobTran->srBRec.inOrgCode = _VOID_LOYALTY_REDEEM_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;


		/* 快樂購紅利積點 */
		case _ECR_8N1_HG_REWARD_SALE_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡紅利積點＞ */
			
			pobTran->inFunctionID = _SALE_;
			pobTran->inRunOperationID = _OPERATION_HG_;
			pobTran->inRunTRTID = _TRT_HG_REWARD_CREDIT_CTLS_;

			/* 這邊先設成sale，若之後支付方式選銀聯卡，會在那邊改成CUP_SALE */
			pobTran->inTransactionCode = _SALE_;
			pobTran->srBRec.inCode = _SALE_;
			pobTran->srBRec.inOrgCode = _SALE_;
			pobTran->srBRec.lnHGTransactionType = _HG_REWARD_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		
		/* 快樂購紅利積點─分期付款 */
		case _ECR_8N1_HG_REWARD_INSTALLMENT_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_INS_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡分期付款＞ */
			
			pobTran->inFunctionID = _INST_SALE_;
			pobTran->inRunOperationID = _OPERATION_HG_I_R_;
			pobTran->inRunTRTID = _TRT_HG_REWARD_INSTALLMENT_CTLS_;

			pobTran->inTransactionCode = _INST_SALE_;
			pobTran->srBRec.inCode = _INST_SALE_;
			pobTran->srBRec.inOrgCode = _INST_SALE_;
			pobTran->srBRec.lnHGTransactionType = _HG_REWARD_;
			pobTran->srBRec.uszInstallmentBit = VS_TRUE;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		
		/* 快樂購紅利積點─紅利扣抵 */
		case _ECR_8N1_HG_REWARD_REDEMPTION_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_REDEEM_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡紅利扣抵＞ */
			
			pobTran->inFunctionID = _REDEEM_SALE_;
			pobTran->inRunOperationID = _OPERATION_HG_I_R_;
			pobTran->inRunTRTID = _TRT_HG_REWARD_REDEMPTION_CTLS_;

			pobTran->inTransactionCode = _REDEEM_SALE_;
			pobTran->srBRec.inCode = _REDEEM_SALE_;
			pobTran->srBRec.inOrgCode = _REDEEM_SALE_;
			pobTran->srBRec.lnHGTransactionType = _HG_REWARD_;
			pobTran->srBRec.uszRedeemBit = VS_TRUE;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
			
		/* 快樂購點數扣抵 */
		case _ECR_8N1_HG_ONLINE_REDEEMPTION_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_ONLINE_REDEEM_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡點數扣抵＞ */
			
			pobTran->inFunctionID = _SALE_;
			pobTran->inRunOperationID = _OPERATION_HG_;
			pobTran->inRunTRTID = _TRT_HG_ONLINE_REDEEM_CREDIT_CTLS_;

			pobTran->inTransactionCode = _SALE_;
			pobTran->srBRec.inCode = _SALE_;
			pobTran->srBRec.inOrgCode = _SALE_;
			pobTran->srBRec.lnHGTransactionType = _HG_ONLINE_REDEEM_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		
		/* 快樂購加價購 */
		case _ECR_8N1_HG_POINT_CERTAIN_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_POINT_CERTAIN_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡加價購＞ */
			
			pobTran->inFunctionID = _SALE_;
			pobTran->inRunOperationID = _OPERATION_HG_;
			pobTran->inRunTRTID = _TRT_HG_POINT_CERTAIN_CREDIT_CTLS_;

			pobTran->inTransactionCode = _SALE_;
			pobTran->srBRec.inCode = _SALE_;
			pobTran->srBRec.inOrgCode = _SALE_;
			pobTran->srBRec.lnHGTransactionType = _HG_POINT_CERTAIN_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		
		/* 快樂購點數兌換 */
		case _ECR_8N1_HG_FULL_REDEEMPTION_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_REDEMPTION_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡點數兌換＞ */
			
			pobTran->inFunctionID = _SALE_;
			pobTran->inRunOperationID = _OPERATION_HG_;
			pobTran->inRunTRTID = _TRT_HG_FULL_REDEMPTION_;

			pobTran->inTransactionCode = _SALE_;
			pobTran->srBRec.inCode = _SALE_;
			pobTran->srBRec.inOrgCode = _SALE_;
			pobTran->srBRec.lnHGTransactionType = _HG_FULL_REDEMPTION_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		
		/* 快樂購扣抵退貨 */
		case _ECR_8N1_HG_REDEEM_REFUND_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_REDEEM_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡扣抵退貨＞ */
			
			pobTran->inFunctionID = _HG_REDEEM_REFUND_;
			pobTran->inRunOperationID = _OPERATION_HG_REFUND_;
			pobTran->inRunTRTID = _TRT_HG_REDEEM_REFUND_;

			pobTran->inTransactionCode = _HG_REDEEM_REFUND_;
			pobTran->srBRec.inCode = _HG_REDEEM_REFUND_;
			pobTran->srBRec.inOrgCode = _HG_REDEEM_REFUND_;
			pobTran->srBRec.lnHGTransactionType = _HG_REDEEM_REFUND_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		
		/* 快樂購回饋退貨 */
		case _ECR_8N1_HG_REWARD_REFUND_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡回饋退貨＞ */
			
			pobTran->inFunctionID = _HG_REWARD_REFUND_;
			pobTran->inRunOperationID = _OPERATION_HG_REFUND_;
			pobTran->inRunTRTID = _TRT_HG_REWARD_REFUND_;

			pobTran->inTransactionCode = _HG_REWARD_REFUND_;
			pobTran->srBRec.inCode = _HG_REWARD_REFUND_;
			pobTran->srBRec.inOrgCode = _HG_REWARD_REFUND_;
			pobTran->srBRec.lnHGTransactionType = _HG_REWARD_REFUND_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		
		/* 快樂購點數查詢 */
		case _ECR_8N1_HG_POINT_INQUIRY_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_HAPPYGO_INQUIRY_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡點數查詢＞ */
			
			pobTran->inFunctionID = _HG_INQUIRY_;
			pobTran->inRunOperationID = _OPERATION_HG_;
			pobTran->inRunTRTID = _TRT_HG_INQUIRY_;

			pobTran->inTransactionCode = _HG_INQUIRY_;
			pobTran->srBRec.inCode = _HG_INQUIRY_;
			pobTran->srBRec.inOrgCode = _HG_INQUIRY_;
			pobTran->srBRec.lnHGTransactionType = _HG_INQUIRY_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
			
		case _ECR_8N1_SETTLEMENT_NO_:
			inDISP_ClearAll();
			/* 第三層顯示 ＜結帳交易＞ */
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_SETTLE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜結帳交易＞ */

			pobTran->inFunctionID = _SETTLE_;
			pobTran->inRunOperationID = _OPERATION_SETTLE_;
			pobTran->inRunTRTID = _TRT_SETTLE_;
			
			pobTran->inTransactionCode = _SETTLE_;
			pobTran->srBRec.inCode = _SETTLE_;
			pobTran->srBRec.inOrgCode = _SETTLE_;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		
		case _ECR_8N1_REPRINT_RECEIPT_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_REPRINT_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜重印帳單＞ */

			pobTran->inFunctionID = FALSE;
			pobTran->inRunOperationID = _OPERATION_REPRINT_;
			pobTran->inRunTRTID = FALSE;
			
			pobTran->inTransactionCode = FALSE;
			pobTran->srBRec.inCode = FALSE;
			pobTran->srBRec.inOrgCode = FALSE;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
				
		case _ECR_8N1_MENU_REVIEW_DETAIL_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_REVIEW_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 <交易查詢> */
			
			pobTran->inFunctionID = FALSE;
			pobTran->inRunOperationID = _OPERATION_REVIEW_DETAIL_;
			pobTran->inRunTRTID = FALSE;
			
			pobTran->inTransactionCode = FALSE;
			pobTran->srBRec.inCode = FALSE;
			pobTran->srBRec.inOrgCode = FALSE;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		
		case _ECR_8N1_MENU_REVIEW_TOTAL_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_REVIEW_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜總額查詢＞ */
			
			pobTran->inFunctionID = FALSE;
			pobTran->inRunOperationID = _OPERATION_REVIEW_TOTAL_;
			pobTran->inRunTRTID = FALSE;
			
			pobTran->inTransactionCode = FALSE;
			pobTran->srBRec.inCode = FALSE;
			pobTran->srBRec.inOrgCode = FALSE;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
			
		case _ECR_8N1_MENU_REPORT_DETAIL_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_DETAIL_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 明細列印 */
			
			pobTran->inFunctionID = FALSE;
			pobTran->inRunOperationID = _OPERATION_DETAIL_REPORT_;
			pobTran->inRunTRTID = FALSE;
			
			pobTran->inTransactionCode = FALSE;
			pobTran->srBRec.inCode = FALSE;
			pobTran->srBRec.inOrgCode = FALSE;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
			
		case _ECR_8N1_MENU_REPORT_TOTAL_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_TOTAIL_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 總額列印 */
			
			pobTran->inFunctionID = FALSE;
			pobTran->inRunOperationID = _OPERATION_TOTAL_REPORT_;
			pobTran->inRunTRTID = FALSE;
			
			pobTran->inTransactionCode = FALSE;
			pobTran->srBRec.inCode = FALSE;
			pobTran->srBRec.inOrgCode = FALSE;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		case _ECR_8N1_EDC_REBOOT_NO_:
			inDISP_ClearAll();
			/* 第三層顯示 ＜重新開機＞ */
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_REBOOT_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜重新開機＞ */
			
			pobTran->inFunctionID = FALSE;
			pobTran->inRunOperationID = _OPERATION_EDC_REBOOT_;
			pobTran->inRunTRTID = FALSE;
			
			pobTran->inTransactionCode = FALSE;
			pobTran->srBRec.inCode = FALSE;
			pobTran->srBRec.inOrgCode = FALSE;
			
			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			
			break;
		case _ECR_8N1_ESVC_TOP_UP_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_TICKET_TOP_UP_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜加值交易＞ */

			pobTran->inFunctionID = _TICKET_TOP_UP_;

			if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
			{
				/* (需求單 - 107227)邦柏科技自助作業客製化 悠遊卡0018重試功能 by Russell 2018/12/14 上午 9:30 */
				if (pobTran->uszLastTranscationBit == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_TICKET_RETRY_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_TICKET_TOP_UP_;
				}
			}
			else
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "感應未初始化，不能使用此功能");
				}
				return (VS_ERROR);
			}

			pobTran->inTransactionCode = _TICKET_TOP_UP_;
			pobTran->srTRec.inCode = _TICKET_TOP_UP_;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		case _ECR_8N1_ESVC_BALANCE_INQUIRY_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0, _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_TICKET_INQUIRY_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜餘額查詢＞ */

			pobTran->inFunctionID = _TICKET_INQUIRY_;

			if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
			{
				pobTran->inRunOperationID = _OPERATION_TICKET_INQUIRY_;
			}
			else
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "感應未初始化，不能使用此功能");
				}
				return (VS_ERROR);
			}

			pobTran->inTransactionCode = _TICKET_INQUIRY_;
			pobTran->srTRec.inCode = _TICKET_INQUIRY_;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		case _ECR_8N1_ESVC_VOID_TOP_UP_NO_:
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);					/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_TICKET_VOID_TOP_UP_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜加值取消＞ */

			pobTran->inFunctionID = _TICKET_VOID_TOP_UP_;

			if (!memcmp(szCTLSEnable, "Y", 1) && guszCTLSInitiOK == VS_TRUE)
			{
				/* (需求單 - 107227)邦柏科技自助作業客製化 悠遊卡0018重試功能 by Russell 2018/12/14 上午 9:30 */
				if (pobTran->uszLastTranscationBit == VS_TRUE)
				{
					pobTran->inRunOperationID = _OPERATION_TICKET_RETRY_;
				}
				else
				{
					pobTran->inRunOperationID = _OPERATION_TICKET_VOID_TOP_UP_;
				}
			}
			else
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "感應未初始化，不能使用此功能");
				}
				return (VS_ERROR);
			}

			pobTran->inTransactionCode = _TICKET_VOID_TOP_UP_;
			pobTran->srTRec.inCode = _TICKET_VOID_TOP_UP_;

			inRetVal = inFLOW_RunOperation(pobTran, pobTran->inRunOperationID);
			break;
		case _ECR_8N1_END_CARD_NO_INQUIRY_NO_:
			pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_END_INQUIRY_SUCCESS_;
			inRetVal = VS_ERROR;
			break;
		/* 客製化107直接借用來當Echo Test*/
		case _ECR_8N1_START_CARD_NO_INQUIRY_NO_:				/* Echo Test */	
			inRetVal = VS_SUCCESS;
			break;
                case _ECR_8N1_ECHO_NO_:
			inNCCC_Func_Disp_EchoTest();
			inRetVal = VS_SUCCESS;
			break;
		/* 查詢上一筆 */
		case _ECR_8N1_INQUIRY_LAST_TRANSACTION_NO_:
			inRetVal = VS_SUCCESS;
			break;
		default:
			pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
			inRetVal = VS_ERROR;
			break;
	}
	
	return (inRetVal);
}