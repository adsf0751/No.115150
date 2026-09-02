#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <ctosapi.h>
#include <ctos_clapi.h>
#include <ctos_qrcode.h>
#include <emv_cl.h>
#include <iconv.h>
#include <pthread.h>
#include <fcntl.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <sys/types.h>		//2019/3/5 上午 10:00 Added by Hachi
#include <sys/stat.h>		//2019/3/5 上午 10:00 Added by Hachi
#include <sqlite3.h>
#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Define_2.h"
#include "../INCLUDES/TransType.h"
#include "../INCLUDES/Transaction.h"
#include "../INCLUDES/AllStruct.h"
#include "../PRINT/Print.h"
#include "../DISPLAY/Display.h"
#include "../DISPLAY/DispMsg.h"
#include "../DISPLAY/DisTouch.h"
#include "../EVENT/MenuMsg.h"
#include "../EVENT/Menu.h"
#include "../EVENT/Flow.h"
#include "../EVENT/Event.h"
#include "../COMM/Comm.h"
#include "../COMM/TLS.h"
#include "../COMM/Bluetooth.h"
#include "../COMM/Ethernet.h"
#include "../COMM/GSM.h"
#include "../COMM/GPRS.h"
#include "../../HG/HGsrc.h"
#include "../../HG/HGiso.h"
#include "Sqlite.h"
#include "Accum.h"
#include "Batch.h"
#include "Card.h"
#include "CDT.h"
#include "CFGT.h"
#include "CPT.h"
#include "EDC.h"
#include "File.h"
#include "Function.h"
#include "FuncTable.h"
#include "HDT.h"
#include "HDPT.h"
#include "KMS.h"
#include "MVT.h"
#include "PWD.h"
#include "ECR.h"
#include "RS232.h"
#include "MultiFunc.h"
#include "PowerManagement.h"
#include "XML.h"
#include "VWT.h"
#include "SCDT.h"
#include "../COMM/WiFi.h"
#include "../../CREDIT/CreditprtByBuffer.h"
#include "../../CREDIT/Creditfunc.h"
#include "../../FISC/NCCCfisc.h"
#include "../../CTLS/CTLS.h"
#include "../../EMVSRC/EMVsrc.h"
#include "../../NCCC/NCCCats.h"
#include "../../NCCC/NCCCesc.h"
#include "../../NCCC/NCCCsrc.h"
#include "../../NCCC/NCCCdcc.h"
#include "../../NCCC/NCCCtmk.h"
#include "../../NCCC/NCCCtms.h"
#include "../../NCCC/NCCCtmsCPT.h"
#include "../../NCCC/NCCCtmsSCT.h"
#include "../../NCCC/NCCCtmsFTP.h"
#include "../../NCCC/NCCCtSAM.h"
#include "../../NCCC/NCCCloyalty.h"
#include "../../NCCC/NCCCTicketSrc.h"
#include "../../DINERS/DINERSsrc.h"
#include "../../ETicket/ticket.h"
#include "../../IPASS/IPASSFunc.h"
#include "../../ECC/ICER/stdAfx.h"
#include "../../ECC/ECC.h"
#include "TDT.h"
#include "Signpad.h"
#include "IPASSDT.h"
#include "ECCDT.h"
#include "ICASHDT.h"
#include "USB.h"
#include "Utility.h"
#include "EDC_Para_Table_Func.h"
#include "../PRINT/PrtMsg.h"

#include "../../NCCC/NCCCescReceipt.h"

#define max(x, y) (((x) > (y)) ? (x) : (y))

extern  int     ginIdleMSRStatus, ginMenuKeyIn, ginIdleICCStatus;
extern  int     ginEventCode;
extern  int     ginDebug;  			/* Debug使用 extern */
extern  int     ginISODebug;  			/* Debug使用 extern */
extern	int	ginDisplayDebug;		/* Debug使用 extern */
extern	int	ginEngineerDebug;
extern	int	ginFindRunTime;
extern	int	ginESCDebug;
extern	int	ginESCHostIndex;
extern	int	ginFallback;
extern	char	gszTermVersionID[16 + 1];
extern	char	gszTermVersionDate[16 + 1];
extern	int	ginMachineType;
extern	int	ginAPVersionType;
extern  int	ginContactlessSupport;
extern  int	ginWiFiSupport;
extern	int	ginHalfLCD;
extern	int	ginTouchEnable;
extern	int	ginIdleDispFlag;
extern	int			ginRuntimeCnt;
extern	RUNTIME_ALL_RECORD	gsrRuntimeRecord;		/* 用來紀錄哪邊花比較多時間用的 */
extern	unsigned long		gulDemoHappyGoPoint;
extern	unsigned long		gulDemoRedemptionPointsBalance;
extern	unsigned long		gulDemoTicketPoint;
extern	char			gszTranDBPath[100 + 1];
extern	char			gszParamDBPath[100 + 1];
extern	ECR_TABLE		gsrECROb;
extern	unsigned char		guszCTLSInitiOK;
extern	char	gszReprintDBPath[100 + 1];
extern	unsigned short		gusPrintFontStyleRegular;
extern	int	ginNCCCDCCInvoice;
extern	unsigned long	gulPCIRebootTime;
extern	unsigned long	gulPCI_IdleTime;
extern	unsigned long	gulTotalROMSize;
extern  int             ginSSLErrCode;
extern	unsigned char	guszAlreadySelectMultiAIDBit;
extern	int ginToggleCardRetention;/* [260318_BUG] 新增控制參數 */

/*
Function	:inFunc_PWM_Mode_Switch
Date&Time	:2024/3/6 上午 11:14
Describe        :切換省電模式，目前僅V3M適用
*/
int inFunc_PWM_Mode_Switch(void)
{
	int		inRetVal = VS_SUCCESS;
        char		szTemplate[64 + 1] = {0};
	char		szDispMsg[64 + 1] = {0};
	unsigned char	uszKey = 0x00;
	DISPLAY_OBJECT  srDispObj;
        
	inDISP_ClearAll();
	
        while (1)
        {
		/* PWM Enable */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetPWMEnable(szTemplate);

		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		sprintf(szDispMsg, "PWM Enable: %s", szTemplate);

		inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
		inDISP_ChineseFont("0 = N", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
		inDISP_ChineseFont("1 = Y", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);
	
                uszKey = uszKBD_GetKey(30);
		if (uszKey == _KEY_0_)
		{
			inSetPWMEnable("N");
			inSaveEDCRec(0);
		}
		else if (uszKey == _KEY_1_)
		{
			inSetPWMEnable("Y");
			inSaveEDCRec(0);
		}
		else if (uszKey == _KEY_TIMEOUT_)
		{
			break;
		}
		else if (uszKey == _KEY_CANCEL_ || uszKey == _KEY_ENTER_)
		{
			break;
		}
		else
		{
			continue;
		}
        }
	
	while (1)
        {
		/* PWM Mode */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetPWMMode(szTemplate);

		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		if (!memcmp(szTemplate, _PWM_MODE_00_STANDBY_, 1))
		{
			sprintf(szDispMsg, "PWM Mode: %s", "STANDBY");
		}
		else if (!memcmp(szTemplate, _PWM_MODE_01_SLEEP_, 1))
		{
			sprintf(szDispMsg, "PWM Mode: %s", "SLEEP");
		}
		else
		{
			sprintf(szDispMsg, "PWM Mode: %s", szTemplate);
		}

		inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
		inDISP_ChineseFont("0 = STANDBY", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
		inDISP_ChineseFont("1 = SLEEP", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);
	
                uszKey = uszKBD_GetKey(30);
		if (uszKey == _KEY_0_)
		{
			inSetPWMMode(_PWM_MODE_00_STANDBY_);
			inSaveEDCRec(0);
		}
		else if (uszKey == _KEY_1_)
		{
			inSetPWMMode(_PWM_MODE_01_SLEEP_);
			inSaveEDCRec(0);
		}
		else if (uszKey == _KEY_TIMEOUT_)
		{
			break;
		}
		else if (uszKey == _KEY_CANCEL_ || uszKey == _KEY_ENTER_)
		{
			break;
		}
		else
		{
			continue;
		}
        }
	
	memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
	srDispObj.inY = _LINE_8_8_;
	srDispObj.inR_L = _DISP_RIGHT_;
	srDispObj.inMaxLen = 3;
	srDispObj.inColor = _COLOR_BLACK_;
	
	while (1)
        {
		/* PWM timeout */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetPWMIdleTimeout(szTemplate);

		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		sprintf(szDispMsg, "PWM Timeout: %s", szTemplate);

		inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
	
                memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;

		inRetVal = inDISP_Enter8x16(&srDispObj);
		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
		{
			break;
		}
		else if (srDispObj.inOutputLen > 0)
		{
			inSetPWMIdleTimeout(srDispObj.szOutput);
			inSaveEDCRec(0);
			break;
		}
		
        }
        
        return (VS_SUCCESS);
}

/*
Function        :inFunc_Booting_Flow_Wait_For_Switch
Date&Time       :2024/4/29 下午 5:51
Describe        :
*/
int inFunc_Booting_Flow_Wait_For_Switch(TRANSACTION_OBJECT *pobTran)
{	
	int	inRetVal = VS_SUCCESS;
	char	szTMSOK[1 + 1] = {0};
	char	szCommMode[2 + 1] = {0};
	char	szTMS_EffectiveBit[1 + 1] = {0};
	char	szTMSFTP_EffectiveBit[1 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
	
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_Booting_Flow_Wait_For_Switch START!");
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_Booting_Flow_Wait_For_Switch() START !");
	}
	
	memset(szTMSOK, 0x00, sizeof(szTMSOK));
	inGetTMSOK(szTMSOK);
	memset(szCommMode, 0x00, sizeof(szCommMode));
	inGetCommMode(szCommMode);
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	inLoadTMSCPTRec(0);
	inLoadTMSFTPRec(0);
	memset(szTMS_EffectiveBit, 0x00, sizeof(szTMS_EffectiveBit));
	inGetTMSEffectiveReportBit(szTMS_EffectiveBit);
	memset(szTMSFTP_EffectiveBit, 0x00, sizeof(szTMSFTP_EffectiveBit));
	inGetFTPEffectiveReportBit(szTMSFTP_EffectiveBit);
	
	do
	{
		if (memcmp(szTMSOK, "Y", 1) != 0)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "TMS未完成，不須檢核");
			}
			break;
		}
		
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_082_IKEA_, _CUSTOMER_INDICATOR_SIZE_) ||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_))
		{
			
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "非客製化082、123，不須檢核");
				inLogPrintf(AT, "CUS:%s", szCustomerIndicator);
			}
			break;
		}
		
		if (memcmp(szTMS_EffectiveBit, "Y", 1) != 0 &&
		    memcmp(szTMSFTP_EffectiveBit, "Y", 1) != 0)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "不須TMS回報，不進行測試");
			}
			break;
		}

		if (!memcmp(szCommMode, _COMM_MODEM_MODE_, 1))
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "撥接不測試網路");
			}
			break;
		}

		inDISP_Timer_Start(_TIMER_NEXSYS_2_, 60);
		inDISP_ClearAll();
		inDISP_ChineseFont_Color("網路連線中", _FONTSIZE_8X16_, _LINE_8_4_, _COLOR_BLACK_, _DISP_CENTER_);
		do
		{
			if (memcmp(szCommMode, _COMM_ETHERNET_MODE_, 1) == 0)
			{
				inRetVal = inETHERNET_NCCCTMS_Check();
				inETHERNET_END();
			}
			else if (memcmp(szCommMode, _COMM_GPRS_MODE_, 1) == 0	||
				 memcmp(szCommMode, _COMM_3G_MODE_, 1) == 0	||
				 memcmp(szCommMode, _COMM_4G_MODE_, 1) == 0)
			{
				inRetVal = inGPRS_NCCCTMS_Check();
				inGPRS_END();
			}
			else
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "未知連線方式");
				}
				inRetVal = VS_ERROR;
			}

			if (inRetVal == VS_SUCCESS)
			{
				break;
			}

			if (inTimerGet(_TIMER_NEXSYS_2_) == VS_SUCCESS)
			{
				break;
			}
		}while(1);
		
		break;
	}while (1);
	
	inDISP_ClearAll();
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_Booting_Flow_Wait_For_Switch() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Compare_Files
Date&Time       :2024/5/20 下午 3:58
Describe        :
*/
int inFunc_Compare_Files(char* szFilePath1, char* szFilePath2)
{
	int		i = 0;
	int		inRetVal = VS_SUCCESS;
	char		szReadData1[1 + 1] = {0};
	char		szReadData2[1 + 1] = {0};
	long		lnFileSize1 = 0;
	long		lnFileSize2 = 0;
	unsigned long	ulFileHandle1 = 0;
	unsigned long	ulFileHandle2 = 0;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_Compare_Files(%s,%s) START !", szFilePath1, szFilePath2);
	}
	
	lnFileSize1 = lnFILE_GetSize(&ulFileHandle1, (unsigned char*)szFilePath1);
	lnFileSize2 = lnFILE_GetSize(&ulFileHandle2, (unsigned char*)szFilePath2);
	
	do
	{
		/* 確認是否存在 */
		if (inFILE_Check_Exist((unsigned char*)szFilePath1) != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "%s not exist", szFilePath1);
			}
			inRetVal = VS_ERROR;
			break;
		}
		
		if (inFILE_Check_Exist((unsigned char*)szFilePath2) != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "%s not exist", szFilePath2);
			}
			
			inRetVal = VS_ERROR;
			break;
		}
		
		
		if (lnFileSize1 != lnFileSize2)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "FileSize not equal %ld != %ld", lnFileSize1, lnFileSize2);
			}
			
			inRetVal = VS_ERROR;
			break;
		}
		
		inFILE_OpenReadOnly(&ulFileHandle1, (unsigned char*)szFilePath1);
		inFILE_OpenReadOnly(&ulFileHandle2, (unsigned char*)szFilePath2);
		
		for (i = 0; i < lnFileSize1; i++)
		{
			inFILE_Read(&ulFileHandle1, (unsigned char*)szReadData1, 1);
			inFILE_Read(&ulFileHandle2, (unsigned char*)szReadData2, 1);
			
			if (szReadData1[0] != szReadData2[0])
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "data not eaual,%d", i);
				}
				
				inRetVal = VS_ERROR;
				break;
			}
		}
		
		break;
	}while (1);
	
	if (ulFileHandle1 != -1 &&
	    ulFileHandle1 != 0)
	{
		inFILE_Close(&ulFileHandle1);
	}
	
	if (ulFileHandle2 != -1 &&
	    ulFileHandle2 != 0)
	{
		inFILE_Close(&ulFileHandle2);
	}
	
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	else
	{
		return (VS_SUCCESS);
	}
}

/*
Function        :inFunc_Check_NexsysLog_To_SD
Date&Time       :2024/11/12 下午 3:29
Describe        :
*/
int inFunc_Check_NexsysLog_To_SD(void)
{
	char		szPath[100 + 1] = {0};
	char		szDirName[100 + 1] = {0};
	char		szModelName[20 + 1] = {0};
	RTC_NEXSYS	srRTC = {0};
	DISPLAY_OBJECT	srDispMsgObj = {0};
	
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);

	if (inFunc_Check_SDCard_Mounted() != VS_SUCCESS)
	{
		memset(&srDispMsgObj, 0x00, sizeof(DISPLAY_OBJECT));
		strcpy(srDispMsgObj.szDispPic1Name, "");
		srDispMsgObj.inDispPic1YPosition = 0;
		srDispMsgObj.inMsgType = _NO_KEY_MSG_;
		srDispMsgObj.inTimeout = 2;
		strcpy(srDispMsgObj.szErrMsg1, "未掛載SD");
		srDispMsgObj.inErrMsg1Line = _LINE_8_7_;
		srDispMsgObj.inBeepTimes = 1;
		srDispMsgObj.inBeepInterval = 0;
		inDISP_Msg_BMP(&srDispMsgObj);
		return (VS_ERROR);
	}
	else
	{
		inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
		inDISP_ChineseFont("SD已掛載", _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);
	}
	
	inDISP_Clear_Line(_LINE_8_8_, _LINE_8_8_);
	inDISP_ChineseFont("複製檔案中", _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
	
	vdUtility_SYSFIN_CloseLogFile();
	inFunc_ls("-R -l", _AP_PUB_PATH_);
	
	memset(szModelName, 0x00, sizeof(szModelName));
	inFunc_Get_Termial_Model_Name(szModelName);
	
	inFunc_GetSystemDateAndTime(&srRTC);
	sprintf(szPath, "%s%s_Check/", _SD_PATH_, szModelName);
	sprintf(szDirName, "%02d%02d%02d_%02d%02d%02d_Nexsys%s_Log", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay, srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond, szModelName);
	
	inFunc_Dir_Make(szDirName, szPath);

	memset(szPath, 0x00, sizeof(szPath));
	sprintf(szPath, "%s%s_Check/%s/", _SD_PATH_, szModelName, szDirName);
	inFunc_Data_Copy("* -r", _AP_PUB_LOG_PATH_, "", szPath);
	
	/* 因為外接設備,仍需要使用同步 */
	sync();
	
	inDISP_Clear_Line(_LINE_8_8_, _LINE_8_8_);
	inDISP_ChineseFont("複製檔案完成", _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Check_NexsysLog_To_USB
Date&Time       :2024/11/12 下午 3:32
Describe        :
*/
int inFunc_Check_NexsysLog_To_USB(void)
{
	int		inOrgUSBMode = 0;
	int		inRetVal = VS_ERROR;
	int		inTimeout = 30;
	char		szPath[100 + 1] = {0};
	char		szDirName[100 + 1] = {0};
	char		szModelName[20 + 1] = {0};
	unsigned char	uszKey = 0x00;
	unsigned char	uszCheckUSBBit = VS_FALSE;
	unsigned char	uszCheckTimeoutBit = VS_FALSE;
	unsigned char	uszCheckKeyBit = VS_FALSE;
	RTC_NEXSYS	srRTC = {0};
	DISPLAY_OBJECT	srDispMsgObj = {0};
	
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_ChineseFont("等待掛載中 Timeout:", _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);
	
	inUSB_Get_Host_Device_Mode(&inOrgUSBMode);
	inUSB_SelectMode(_USB_MODE_HOST_);
	inDISP_TimeoutStart(inTimeout);
	
	while (1)
	{
		inRetVal = inDISP_TimeoutCheck(_FONTSIZE_8X22_, _LINE_8_7_, _DISP_RIGHT_);
		if (inRetVal == VS_TIMEOUT)
		{
			uszCheckTimeoutBit = VS_TRUE;
		}
		
		uszKey = uszKBD_Key();
		if (uszKey == _KEY_CANCEL_)
		{
			uszCheckKeyBit = VS_TRUE;
		}
		
		inRetVal = inFunc_Check_USB_Mounted();
		if (inRetVal == VS_SUCCESS)
		{
			uszCheckUSBBit = VS_TRUE;
		}
		
		if (uszCheckTimeoutBit == VS_TRUE	|| 
		    uszCheckUSBBit == VS_TRUE		||
		    uszCheckKeyBit == VS_TRUE)
		{
			break;
		}
	}
	
	if (uszCheckUSBBit != VS_TRUE)
	{
		memset(&srDispMsgObj, 0x00, sizeof(DISPLAY_OBJECT));
		strcpy(srDispMsgObj.szDispPic1Name, "");
		srDispMsgObj.inDispPic1YPosition = 0;
		srDispMsgObj.inMsgType = _NO_KEY_MSG_;
		srDispMsgObj.inTimeout = 2;
		strcpy(srDispMsgObj.szErrMsg1, "未掛載USB");
		srDispMsgObj.inErrMsg1Line = _LINE_8_7_;
		srDispMsgObj.inBeepTimes = 1;
		srDispMsgObj.inBeepInterval = 0;
		inDISP_Msg_BMP(&srDispMsgObj);
		
		inUSB_SelectMode(inOrgUSBMode);
		
		return (VS_ERROR);
	}
	else
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont("USB已掛載", _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);
	}
	
	memset(szModelName, 0x00, sizeof(szModelName));
	inFunc_Get_Termial_Model_Name(szModelName);
	
	inDISP_Clear_Line(_LINE_8_8_, _LINE_8_8_);
	inDISP_ChineseFont("複製檔案中，請稍後", _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
	
	vdUtility_SYSFIN_CloseLogFile();
	inFunc_ls("-R -l", _AP_PUB_PATH_);
	
	inFunc_GetSystemDateAndTime(&srRTC);
	sprintf(szPath, "%s%s_Check/", _USB_PATH_, szModelName);
	sprintf(szDirName, "%02d%02d%02d_%02d%02d%02d_Nexsys%s_Log", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay, srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond, szModelName);
	
	inFunc_Dir_Make(szDirName, szPath);

	memset(szPath, 0x00, sizeof(szPath));
	sprintf(szPath, "%s%s_Check/%s/", _USB_PATH_, szModelName, szDirName);
	inFunc_Data_Copy("* -r", _AP_PUB_LOG_PATH_, "", szPath);
	
	/* 因為外接設備,仍需要使用同步 */
	sync();
	
	inDISP_Clear_Line(_LINE_8_8_, _LINE_8_8_);
	inDISP_ChineseFont("複製檔案完成", _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
	
	inUSB_SelectMode(inOrgUSBMode);
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_SysFin_Log_Ls
Date&Time       :2024/11/13 上午 11:46
Describe        :
*/
int inFunc_SysFin_Log_Ls(void)
{
	int	inFd = 0;
	int	inFileSize = 0;
	int	inReadTempCnt = 0;
	int	inCurrentCnt = 0;
	char*	szReadBuffer = NULL;		/* 檔名buffer */
	char	szReadTempBuffer[1000 + 1] = {0};	/* 讀取用buffer */
	
	inFunc_ShellCommand_Popen("ls -R -l ./", VS_TRUE);
	inFile_Linux_Open_In_Fs_Data(&inFd, _SHELL_RESPONSE_TEMP_);
	inFile_Linux_Get_FileSize_By_LSeek(inFd, &inFileSize);
	inFile_Linux_Seek(inFd, 0, _SEEK_BEGIN_);
	
	szReadBuffer = malloc(inFileSize + 1);
	
	/* 取得檔案大小 */
	memset(szReadBuffer, 0x00, sizeof(szReadBuffer));
	do
	{
		memset(szReadTempBuffer, 0x00, sizeof(szReadTempBuffer));
		inReadTempCnt = 1000;
		inFile_Linux_Read(inFd, szReadTempBuffer, &inReadTempCnt);
		inCurrentCnt += inReadTempCnt;

		strcat(szReadBuffer, szReadTempBuffer);

	} while (inCurrentCnt < inFileSize);
	
	vdUtility_SYSFIN_LogMessage(AT, szReadBuffer);
		
	free(szReadBuffer);
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Booting_XML_Lib_Init
Date&Time       :2025/1/20 下午 4:03
Describe        :XML lib及global variable初始化 
*/
int inFunc_Booting_Flow_XML_Lib_Init(TRANSACTION_OBJECT *pobTran)
{	
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_Booting_Flow_XML_Lib_Init START!");
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_Booting_Flow_XML_Lib_Init() START !");
	}
        
        inXML_Init();
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_Booting_Flow_XML_Lib_Init() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Booting_Flow_Check_And_Recover_Settlement_XML
Date&Time       :2025/1/23 下午 3:53
Describe        :補未完成的結帳流程
*/
int inFunc_Booting_Flow_Check_And_Recover_Settlement_XML(TRANSACTION_OBJECT *pobTran)
{
	int			inHDTIndex = -1;
	int			inHDPTIndex = -1;
	char			szFileName[256 + 1] = {0};
	char			szFilePath[256 + 1] = {0};
        char                    szBatchNum[6 + 1] = {0};
        char                    szFuncEnable[2 + 1] = {0};
	TRANSACTION_OBJECT	pobTranTemp = {0};
        RTC_NEXSYS              srRTC = {};
	
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_Booting_Flow_Check_And_Recover_Settlement_XML START!");
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_Booting_Flow_Check_And_Recover_Settlement_XML() START !");
	}
        
	/* 確認是否有NCCC未完成流程 */
	memset(&pobTranTemp, 0x00, sizeof(pobTranTemp));
        /* 取得EDC時間日期 */
        memset(&srRTC, 0x00, sizeof(RTC_NEXSYS));
	if (inFunc_GetSystemDateAndTime(&srRTC) != VS_SUCCESS)
	{
                vdUtility_SYSFIN_LogMessage(AT, "inFunc_Booting_Flow_Check_And_Recover_Settlement_XML Get_Time_Failed");
		return (VS_ERROR);
	}
	inFunc_Sync_BRec_Date_Time(&pobTranTemp, &srRTC);     
        pobTranTemp.inFunctionID = _SETTLE_;
	pobTranTemp.inRunOperationID = _OPERATION_SETTLE_;
	pobTranTemp.inRunTRTID = _TRT_SETTLE_;
	pobTranTemp.inTransactionCode = _SETTLE_;
	pobTranTemp.srBRec.inCode = _SETTLE_;
	pobTranTemp.srBRec.inOrgCode = _SETTLE_;
	
	memset(szFileName, 0x00, sizeof(szFileName));
	sprintf(szFileName, "%s", _SETTLENMENT_RECOVER_NCCC_XML_);
	memset(szFilePath, 0x00, sizeof(szFilePath));
	sprintf(szFilePath, "%s", _AP_ROOT_PATH_);
	inHDTIndex = -1;
	inFunc_Find_Specific_HDTindex(pobTranTemp.srBRec.inHDTIndex, _HOST_NAME_CREDIT_NCCC_, &inHDTIndex);
	inHDPTIndex = -1;
	inFunc_Find_Specific_HDPTindex_Flow(pobTranTemp.srBRec.inHDTIndex, _TRT_FILE_NAME_CREDIT_, &inHDPTIndex);
	inLoadHDTRec(inHDTIndex);
	inLoadHDPTRec(inHDPTIndex);
        memset(szBatchNum, 0x00, sizeof(szBatchNum));
        inGetBatchNum(szBatchNum);
	pobTranTemp.srBRec.lnBatchNum = atol(szBatchNum);
        pobTranTemp.srBRec.inHDTIndex = inHDTIndex;
        pobTranTemp.inRunTRTID = _TRT_SETTLE_;
	if (inNCCC_Func_Check_And_Recover_Settlement_XML(&pobTranTemp, szFileName, szFilePath) != VS_SUCCESS)
	{
		vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Func_Check_And_Recover_Settlement_XML failed (%s)(%s)", szFileName, szFilePath);
	}
        
	/* 確認是否有DCC委完成流程 */
	memset(&pobTranTemp, 0x00, sizeof(pobTranTemp));
        /* 取得EDC時間日期 */
        memset(&srRTC, 0x00, sizeof(RTC_NEXSYS));
	if (inFunc_GetSystemDateAndTime(&srRTC) != VS_SUCCESS)
	{
                vdUtility_SYSFIN_LogMessage(AT, "inFunc_Booting_Flow_Check_And_Recover_Settlement_XML Get_Time_Failed");
		return (VS_ERROR);
	}
	inFunc_Sync_BRec_Date_Time(&pobTranTemp, &srRTC);     
        pobTranTemp.inFunctionID = _SETTLE_;
	pobTranTemp.inRunOperationID = _OPERATION_SETTLE_;
	pobTranTemp.inRunTRTID = _TRT_SETTLE_;
	pobTranTemp.inTransactionCode = _SETTLE_;
	pobTranTemp.srBRec.inCode = _SETTLE_;
	pobTranTemp.srBRec.inOrgCode = _SETTLE_;
	
	memset(szFileName, 0x00, sizeof(szFileName));
	sprintf(szFileName, "%s", _SETTLENMENT_RECOVER_DCC_XML_);
	memset(szFilePath, 0x00, sizeof(szFilePath));
	sprintf(szFilePath, "%s", _AP_ROOT_PATH_);
	inHDTIndex = -1;
	inFunc_Find_Specific_HDTindex(pobTranTemp.srBRec.inHDTIndex, _HOST_NAME_DCC_, &inHDTIndex);
	inHDPTIndex = -1;
	inFunc_Find_Specific_HDPTindex_Flow(pobTranTemp.srBRec.inHDTIndex, _TRT_FILE_NAME_DCC_, &inHDPTIndex);
	inLoadHDTRec(inHDTIndex);
	inLoadHDPTRec(inHDPTIndex);
        memset(szBatchNum, 0x00, sizeof(szBatchNum));
        inGetBatchNum(szBatchNum);
	pobTranTemp.srBRec.lnBatchNum = atol(szBatchNum);
        pobTranTemp.srBRec.inHDTIndex = inHDTIndex;
        pobTranTemp.inRunTRTID = _TRT_SETTLE_;
	if (inNCCC_Func_Check_And_Recover_Settlement_XML(&pobTranTemp, szFileName, szFilePath) != VS_SUCCESS)
	{
		vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Func_Check_And_Recover_Settlement_XML failed (%s)(%s)", szFileName, szFilePath);
	}
	
	/* 確認是否有電子票證未完成流程 */
	memset(&pobTranTemp, 0x00, sizeof(pobTranTemp));
        /* 取得EDC時間日期 */
        memset(&srRTC, 0x00, sizeof(RTC_NEXSYS));
	if (inFunc_GetSystemDateAndTime(&srRTC) != VS_SUCCESS)
	{
                vdUtility_SYSFIN_LogMessage(AT, "inFunc_Booting_Flow_Check_And_Recover_Settlement_XML Get_Time_Failed");
		return (VS_ERROR);
	}
	inFunc_Sync_BRec_Date_Time(&pobTranTemp, &srRTC);
	pobTranTemp.inFunctionID = _SETTLE_;
	pobTranTemp.inRunOperationID = _OPERATION_SETTLE_;
	pobTranTemp.inRunTRTID = _TRT_SETTLE_;
	pobTranTemp.inTransactionCode = _SETTLE_;
	pobTranTemp.srBRec.inCode = _SETTLE_;
	pobTranTemp.srBRec.inOrgCode = _SETTLE_;
	
	pobTranTemp.uszPrePrintBit = VS_FALSE;
	memset(szFileName, 0x00, sizeof(szFileName));
	sprintf(szFileName, "%s", _SETTLENMENT_RECOVER_ESVC_XML_);
	memset(szFilePath, 0x00, sizeof(szFilePath));
	sprintf(szFilePath, "%s", _AP_ROOT_PATH_);
	inHDTIndex = -1;
	inFunc_Find_Specific_HDTindex(pobTranTemp.srBRec.inHDTIndex, _HOST_NAME_ESVC_, &inHDTIndex);
	inHDPTIndex = -1;
	inFunc_Find_Specific_HDPTindex_Flow(pobTranTemp.srBRec.inHDTIndex, _TRT_FILE_NAME_ESVC_, &inHDPTIndex);
	inLoadHDTRec(inHDTIndex);
	inLoadHDPTRec(inHDPTIndex);
        inLoadIPASSDTRec(0);
        inLoadECCDTRec(0);;
        inLoadICASHDTRec(0);
        memset(szBatchNum, 0x00, sizeof(szBatchNum));
        inGetBatchNum(szBatchNum);
	pobTranTemp.srBRec.lnBatchNum = atol(szBatchNum);
        pobTranTemp.srBRec.inHDTIndex = inHDTIndex;
        inLoadTDTRec(_TDT_INDEX_01_ECC_);
        memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
        inGetTicket_HostEnable(szFuncEnable);
        if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
        {
                pobTranTemp.inRunTRTID = _TRT_TICKET_ECC_SETTLE_;
        }
        else
        {
                pobTranTemp.inRunTRTID = _TRT_SETTLE_;
        }
	if (inNCCC_Func_Check_And_Recover_Settlement_XML(&pobTranTemp, szFileName, szFilePath) != VS_SUCCESS)
	{
		vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Func_Check_And_Recover_Settlement_XML failed (%s)(%s)", szFileName, szFilePath);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_Booting_Flow_Check_And_Recover_Settlement_XML() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Check_Validation
Date&Time       :2025/3/3 下午 6:19
Describe        :1:檢查全英文大寫
 *               2:檢查全數字
 *               3:檢查全英文或數字
 *               4:檢查全英文大寫和空白
 *               5:檢查全數字和空白
*/
int inFunc_Check_Validation(char* szString, int inStringLen, int inMode)
{
	int	i = 0;
	int	inRetVal = VS_TRUE;
	
	if (szString == NULL)
	{
		return (VS_FALSE);
	}
	
        if (inMode == _CHECK_VALIDATION_MODE_1_)
        {
                for (i = 0; i < inStringLen; i++)
                {
                        if ((szString[i] >= 'A') && (szString[i] <= 'Z'))
                        {
                                continue;
                        }
                        else
                        {
                                inRetVal = VS_FALSE;
                                break;
                        }
                }
        }
        else if (inMode == _CHECK_VALIDATION_MODE_2_)
        {
                for (i = 0; i < inStringLen; i++)
                {
                        if ((szString[i] >= '0') && (szString[i] <= '9'))
                        {
                                continue;
                        }
                        else
                        {
                                inRetVal = VS_FALSE;
                                break;
                        }
                }
        }
        else if (inMode == _CHECK_VALIDATION_MODE_3_)
        {
                for (i = 0; i < inStringLen; i++)
                {
                        if (((szString[i] >= 'A') && (szString[i] <= 'Z')) ||
                            ((szString[i] >= 'a') && (szString[i] <= 'z')) ||
                            ((szString[i] >= '0') && (szString[i] <= '9')))
                        {
                                continue;
                        }
                        else
                        {
                                inRetVal = VS_FALSE;
                                break;
                        }
                }
        }
        else if (inMode == _CHECK_VALIDATION_MODE_4_)
        {
                for (i = 0; i < inStringLen; i++)
                {
                        if (((szString[i] >= 'A') && (szString[i] <= 'Z'))  ||
                            (szString[i] == '\x20'))
                        {
                                continue;
                        }
                        else
                        {
                                inRetVal = VS_FALSE;
                                break;
                        }
                }
        }
        else if (inMode == _CHECK_VALIDATION_MODE_5_)
        {
                for (i = 0; i < inStringLen; i++)
                {
                        if (((szString[i] >= '0') && (szString[i] <= '9'))  ||
                            (szString[i] == '\x20'))
                        {
                                continue;
                        }
                        else
                        {
                                inRetVal = VS_FALSE;
                                break;
                        }
                }
        }
	
	
	return (inRetVal);
}

/*
Function        :inFunc_Booting_Flow_Renew_Auto_Reboot_Time
Date&Time       :2025/4/15 下午 1:56
Describe        :
*/
int inFunc_Booting_Flow_Renew_Auto_Reboot_Time(TRANSACTION_OBJECT *pobTran)
{
	char		szTID[8 + 1] = {0};
	char		szOffsetMin[2 + 1] = {0};
	char		szUnixTime[10 + 1] = {0};
	unsigned long	ulNowTime = 0;
	RTC_NEXSYS	srRTC = {};
	
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_Booting_Flow_Renew_Auto_Reboot_Time START!");
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_Booting_Flow_Renew_Auto_Reboot_Time() START !");
	}
        
        inLoadHDTRec(0);
	ulNowTime = time(NULL);
	memset(szTID, 0x00, sizeof(szTID));
	inGetTerminalID(szTID);
	memset(szOffsetMin, 0x00, sizeof(szOffsetMin));
	memcpy(szOffsetMin, &szTID[6], 2);
	
	gulPCIRebootTime = ulNowTime + (24 * 60 * 60) - (atoi(szOffsetMin) * 60);
	memset(szUnixTime, 0x00, sizeof(szUnixTime));
	sprintf(szUnixTime, "%lu", gulPCIRebootTime);
	
	inFuncGetUnixTimeToLocalTime(&srRTC, szUnixTime, 8, 8);

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_Booting_Flow_Renew_Auto_Reboot_Time(%lu) END !", gulPCIRebootTime);
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "20%u.%u.%u %u:%u:%u", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay, srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
	}
	
	vdUtility_SYSFIN_LogMessage(AT, "inFunc_Booting_Flow_Renew_Auto_Reboot_Time(%lu) END!", gulPCIRebootTime);
	vdUtility_SYSFIN_LogMessage(AT, "20%u.%u.%u %u:%u:%u", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay, srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Check_PCI_Reboot
Date&Time       :2025/4/15 下午 1:56
Describe        :
*/
int inFunc_Check_PCI_Reboot(void)
{
	unsigned long	ulNowUnixTime = 0;
	
        if (inFunc_Check_PCI_6_0() != VS_TRUE)
	{
		return (VS_ERROR);
        }
        
	ulNowUnixTime = time(NULL);
	
	if ((ulNowUnixTime >= (gulPCI_IdleTime + 5 * 60))	&&
	    (ulNowUnixTime >= gulPCIRebootTime))
	{
                inDISP_ClearAll();
		inDISP_ChineseFont("系統檢測中，請稍候", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_CENTER_);
                /* UI顯示的時間目前預設為5秒 */
                inDISP_Wait(5000);
		inFunc_Reboot();
		
		return (VS_SUCCESS);
	}
	else
	{
		return (VS_ERROR);
	}
}

/*
Function        :inFunc_Check_PCI_6_0
Date&Time       :2025/5/6 下午 5:15
Describe        :不是這兩個序號就判定成PCI6.0，"118"V3C序號，"320"V3P序號
*/
int inFunc_Check_PCI_6_0(void)
{
	char	szSN[16 + 1] = {0};
	
	memset(szSN, 0x00, sizeof(szSN));
	inFunc_GetSeriaNumber(szSN);
	/* V3C> SN開頭為118 V3CT3> SN開頭為167 */
	if (!memcmp(&szSN[3], "118", 3)	||
	    !memcmp(&szSN[3], "302", 3))
	{
		/* [LOG顯示][20260211] 記錄TLS使用哪個憑證  */
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "PCI 6.0 要進行憑證驗證 SN[%s]", szSN);
		}
		return (VS_FALSE);
	}

	return (VS_TRUE);
}


/*
Function        :inFunc_Check_ID_Valid
Date&Time       :2025/9/12 上午 10:37
Describe        :
 *              一英文字母：: 代表出生後首次登記戶籍的縣市。
 *              第一個阿拉伯數字：: 為性別碼。 本國籍：1為男性，2為女性。外來人口：8為男性，9為女性。
 *              第二個阿拉伯數字：: 為身分碼。 此碼在2003年（民國92年）後，用來區分設籍前的身分。 0-5是保留給國人，6-9則保留給歸化的外國人與中港澳人民使用。 
 *              第三個至第八個數字：: 是依出生登記順序編號的流水號。
 *              最後一碼：: 是檢查碼，用於驗證身分證字號是否有效。 
 *              檢查碼的驗證公式將字母的十位數乘以1，個位數乘以9，後面9位數字的權重為 8..1,1。
 *              若總和除以10的餘數為0，則該身分證字號為有效證號。
 *              註：舊式居留證性別碼可為英文，舊式居留證的話，需要將性別碼依區域碼的轉換方式轉成數值後取個位數。
*/
int inFunc_Check_ID_Valid(char* szID)
{
        int     i = 0;
	int     inLen = 0;
        int     inID1 = 0;
        int     inID2 = 0;
        int     inSum = 0;
        int     inMapped1 = 0;
        int     inMapped2 = 0;
        int     inLetterMap[26] ={
                10, /* A */
                11, /* B */
                12, /* C */
                13, /* D */
                14, /* E */
                15, /* F */
                16, /* G */
                17, /* H */
                34, /* I */
                18, /* J */
                19, /* K */
                20, /* L */
                21, /* M */
                22, /* N */
                35, /* O */
                23, /* P */
                24, /* Q */
                25, /* R */
                26, /* S */
                27, /* T */
                28, /* U */
                29, /* V */
                32, /* W */
                30, /* X */
                31, /* Y */
                33  /* Z */
                };
        
        inLen = strlen(szID);
        
        /* 長度不滿10 */
        if (inLen != 10)
        {
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "ID Not long(%d)", inLen);
                }
                return (VS_FALSE);
        }
        
        /* 第一個要為英文 */
        if (!isalpha(szID[0]))
        {
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "ID 1 Not alpha(%c)", szID[0]);
                }
                /* 非英文判定不合法 */
                return (VS_FALSE);
        }
        
        /* 第二碼可英文或數字 */
        if (!isalpha(szID[1]) &&
            !isdigit(szID[1]))
        {
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "ID 1 Not alpha and digit(%c)", szID[1]);
                }
                /* 非英文也非數字判定不合法 */
                return (VS_FALSE);
        }
        
        /* 第三到十碼為數字 */
        for (i = 2; i < 10; i++)
        {
                if (!isdigit(szID[i]))
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "ID %d Not digit(%c)", szID[i]);
                        }
                        /* 非阿拉伯數字判定不合法 */
                        return (VS_FALSE);
                }
        }
        
        /* 檢查碼是否合法 */
        /* 取得字母對應的數值（A-Z） */
        inID1 = toupper((int)szID[0]) - 'A';
        inMapped1 = inLetterMap[inID1];   /* 兩位數，例如 A -> 10 */
        
        

        /* 新式演算法：將字母的十位數乘以1，個位數乘以9，後面9位數字的權重為 8..1,1 */
        inSum += (inMapped1 / 10) * 1;
        inSum += (inMapped1 % 10) * 9;

        /* 舊式居留證的話，需要將性別碼依區域碼的轉換方式轉成數值後取個位數。 */
        if (isalpha(szID[1]))
        {
                inID2 = toupper((int)szID[1]) - 'A';
                inMapped2 = inLetterMap[inID2];   /* 兩位數，例如 A -> 10 */
                inSum += (inMapped2 % 10) * 8;
        }
        else
        {
                inSum += (szID[1] - '0') * 8;
        }
        inSum += (szID[2] - '0') * 7;
        inSum += (szID[3] - '0') * 6;
        inSum += (szID[4] - '0') * 5;
        inSum += (szID[5] - '0') * 4;
        inSum += (szID[6] - '0') * 3;
        inSum += (szID[7] - '0') * 2;
        inSum += (szID[8] - '0') * 1;
        inSum += (szID[9] - '0') * 1;
        
        if (inSum % 10 == 0)
        {
                return (VS_TRUE);
        }
        else
        {
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "check not valid(%d)", inSum);
                }
                return (VS_FALSE);
        }
}

/*
Function        :inFunc_PrintReceipt_ByBuffer_Trust
Date&Time       :2025/10/15 下午 5:50
Describe        :For信託使用
*/
int inFunc_PrintReceipt_ByBuffer_Trust(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_ERROR;
	int		inChoice = 0;
	int		inTouchSensorFunc = _Touch_CUST_RECEIPT_;
	char		szDispBuf[_DISP_MSG_SIZE_ + 1] = {0};
	char		szPrtMode[2 + 1] = {0};
	char		szCustomerIndicator[3 + 1] = {0};
	unsigned char   uszKey = 0x00;
	
	/* 為了恢復被signpad清掉的title */
	inFunc_ResetTitle(pobTran);
	
	memset(szPrtMode, 0x00, sizeof(szPrtMode));
	inGetPrtMode(szPrtMode);
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	/* 列印商店聯 */
	/* 客製化123不印商店聯且印簽單要提示音 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)	||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
	{
		inDISP_BEEP(1, 0);
	}
        else if (inNCCC_Func_Check_Print_Mode_4_No_Merchant_For_Txn(pobTran) == VS_TRUE)
        {
                /* TMS參數Print_Mode 4=兩聯免簽不印商店聯 */
        }
	else
	{
		/* 簽單模式不是0就印商店聯 */
		if (memcmp(szPrtMode, "0", strlen("0")) != 0)
		{
			/* 列印帳單中 */
			inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);

			while (1)
			{
				pobTran->srBRec.inPrintOption = _PRT_MERCH_;
				inRetVal = inCREDIT_PRINT_Receipt_ByBuffer(pobTran);

				/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
				if (inRetVal == VS_PRINTER_PAPER_OUT ||
				    inRetVal == VS_PRINTER_OVER_HEAT)
					continue;
				else
				{
					/* 成功就砍簽名圖檔 */
					inFunc_Delete_Signature(pobTran);

					break;
				}
			}
		}


                /* S000 */
                memset(szDispBuf, 0x00, sizeof(szDispBuf));
                sprintf(szDispBuf, "%s", pobTran->srBRec.szAuthCode);
                inDISP_ChineseFont_Color(szDispBuf, _FONTSIZE_8X33_, _LINE_8_5_, _COLOR_BLACK_, _DISP_CENTER_);
                /* 同意交易 */
                memset(szDispBuf, 0x00, sizeof(szDispBuf));
                sprintf(szDispBuf, "%s", pobTran->szHostResponseMessageUTF8);
		inFunc_DiscardSpace(szDispBuf);
                inDISP_ChineseFont_Color(szDispBuf, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_BLACK_, _DISP_CENTER_);

                /* 請按確認或清除 */
                inDISP_PutGraphic(_MSG_ENTER_OR_CANCEL_, 0, _COORDINATE_Y_LINE_8_7_);

		/* 清除keyboad Buffer */
		inFlushKBDBuffer();

		/* 第一聯印完後15秒未按確認或清除鍵則端末機嗶嗶聲提示，響15秒後自動出第二聯 */
		/* 客製化107顯示訊息TimeOut 3秒 */
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inDISP_Timer_Start(_TIMER_NEXSYS_2_, _CUSTOMER_107_BUMPER_DISPLAY_TIMEOUT_);
			inDISP_Timer_Start(_TIMER_NEXSYS_3_, _CUSTOMER_107_BUMPER_DISPLAY_TIMEOUT_);
		}
		/* (需求單 - 107276)自助交易標準400做法 by Russell 2018/12/27 上午 11:20 */
		else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inDISP_Timer_Start(_TIMER_NEXSYS_2_, _CUSTOMER_111_KIOSK_STANDARD_DISPLAY_TIMEOUT_);
			inDISP_Timer_Start(_TIMER_NEXSYS_3_, _CUSTOMER_111_KIOSK_STANDARD_DISPLAY_TIMEOUT_);
		}
		else
		{
			inDISP_Timer_Start(_TIMER_NEXSYS_2_, 30);
			inDISP_Timer_Start(_TIMER_NEXSYS_3_, 15);
		}

		while (1)
		{
			inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
			uszKey = uszKBD_Key();

			if (inTimerGet(_TIMER_NEXSYS_2_) == VS_SUCCESS)
			{
				uszKey = _KEY_TIMEOUT_;
			}

			if (inTimerGet(_TIMER_NEXSYS_3_) == VS_SUCCESS)
			{
				inDISP_BEEP(1, 0);
				inDISP_Wait(250);
			}

			if (uszKey == _KEY_ENTER_			|| 
			    uszKey == _KEY_TIMEOUT_			||
			    inChoice == _CUSTReceipt_Touch_ENTER_)
			{
				inRetVal = VS_SUCCESS;
				break;
			}
			else if (uszKey == _KEY_CANCEL_			||
				 inChoice == _CUSTReceipt_Touch_CANCEL_)
			{
				inRetVal = VS_USER_CANCEL;
				break;
			}
		}
		/* 清空Touch資料 */
		inDisTouch_Flush_TouchFile();

		/* 因為這裡交易已完成，所以一定回傳成功*/
		if (inRetVal == VS_USER_CANCEL)
		{
			return (VS_SUCCESS);
		}
	}
	
        /* 列印顧客聯 */
	/* 三聯式簽單，兩聯以上就要印顧客聯 */
	if (memcmp(szPrtMode, "2", strlen("2")) == 0	||
	    memcmp(szPrtMode, "3", strlen("3")) == 0    ||
            !memcmp(szPrtMode, _PRT_MODE_4_TWO_RECEIPT_NO_MERCHANT_, strlen(_PRT_MODE_4_TWO_RECEIPT_NO_MERCHANT_)))
	{
		while (1)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_041_CASH_, _CUSTOMER_INDICATOR_SIZE_) ||
			    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_043_BDAU9_, _CUSTOMER_INDICATOR_SIZE_))
			{
				/* 041、043不顯示免簽名 */
				/* 提示檢核簽名和授權碼 */
				inDISP_PutGraphic(_CHECK_SIGNATURE_2_, 0, _COORDINATE_Y_LINE_8_4_);
			}
			else
			{
				if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE && pobTran->srBRec.inCode != _TIP_)
				{
					/* 提示免簽名和授權碼 */
					inDISP_PutGraphic(_CHECK_SIGNATURE_6_, 0, _COORDINATE_Y_LINE_8_4_);
				}
				else
				{
                                        /* [113110-信託需求][UI] 修改列印顯提示圖,不再提示持卡人核對簽名字樣 2025/11/27 */
                                        if (pobTran->srTrustRec.uszTrustTransBit == VS_TRUE)
                                        {
                                            /* 顯示授權碼 */
                                            inDISP_PutGraphic(_TR_CHECK_SIGNATURE_2_, 0, _COORDINATE_Y_LINE_8_4_);
                                        }else
                                        {
                                            /* 提示檢核簽名和授權碼 */
                                            inDISP_PutGraphic(_CHECK_SIGNATURE_2_, 0, _COORDINATE_Y_LINE_8_4_);
                                        }
				}
			}
                        
                        /* [113110-信託需求][UI] 信託交易,不用顯示授權碼 2025/11/27 */
                        if (pobTran->srTrustRec.uszTrustTransBit != VS_TRUE)
                        {
                            memset(szDispBuf, 0x00, sizeof(szDispBuf));
                            sprintf(szDispBuf, "%s", pobTran->srBRec.szAuthCode);
                            inDISP_EnglishFont_Point_Color(szDispBuf, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_,_COLOR_WHITE_, 11);
                        }
			pobTran->srBRec.inPrintOption = _PRT_CUST_;
			inRetVal = inCREDIT_PRINT_Receipt_ByBuffer(pobTran);

			/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
			if (inRetVal == VS_PRINTER_PAPER_OUT ||
			    inRetVal == VS_PRINTER_OVER_HEAT)
				continue;
			else
				break;
		}
	}

        return (VS_SUCCESS);
}

/*
Function        : inFunc_PrintReceipt_ByBuffer_Trust
Date&Time       :	 20251217
Describe        : 銀聯列印時提示使用
 [20251215_BUG_MDF][UI] 新增銀聯提示用顯示畫面
*/

int inFunc_DisplayCupWarning(TRANSACTION_OBJECT *pobTran)
{
	char szDispBuf[_DISP_MSG_SIZE_ + 1] = {0};
	char szCustomerIndicator[3 + 1] = {0};
    
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_041_CASH_, _CUSTOMER_INDICATOR_SIZE_) ||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_043_BDAU9_, _CUSTOMER_INDICATOR_SIZE_))
	{
		/* 041、043不顯示免簽名 */
		/* 提示檢核簽名和授權碼 */
		inDISP_ChineseFont_Color("請核對持卡人簽名", _FONTSIZE_8X33_, _LINE_8_4_, _COLOR_BLACK_, _DISP_LEFT_);
	}else
	{
		/* 顯示核對持卡人簽名及免簽名訊息 */
		if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE && pobTran->srBRec.inCode != _TIP_)
		{ 
			inDISP_ChineseFont_Color("本交易免簽名", _FONTSIZE_8X33_, _LINE_8_4_, _COLOR_BLACK_, _DISP_LEFT_);
		}else
		{
			inDISP_ChineseFont_Color("請核對持卡人簽名", _FONTSIZE_8X33_, _LINE_8_4_, _COLOR_BLACK_, _DISP_LEFT_);
		}
	}
	/* 授權碼 */
	inDISP_ChineseFont_Color("授權碼：", _FONTSIZE_8X33_, _LINE_8_5_, _COLOR_BLACK_, _DISP_LEFT_);
	memset(szDispBuf, 0x00, sizeof(szDispBuf));
	snprintf(szDispBuf, sizeof(szDispBuf), "%s", pobTran->srBRec.szAuthCode);
	inDISP_ChineseFont_Color(szDispBuf, _FONTSIZE_8X33_, _LINE_8_5_, _COLOR_BLACK_, _DISP_RIGHT_);

	/* 確認本人指定以銀聯卡支付 */
	inDISP_ChineseFont_Color("本人指定以銀聯卡支付", _FONTSIZE_8X33_, _LINE_8_6_, _COLOR_BLACK_, _DISP_CENTER_);

	/* I chose CUP card to  pay */
	inDISP_ChineseFont_Color("I chose CUP card to pay", _FONTSIZE_8X33_, _LINE_8_7_, _COLOR_BLACK_, _DISP_CENTER_);

	return VS_SUCCESS;
}

/*
Function        : inFunc_PrintReceipt_ByBuffer_Trust
Date&Time       :	 20251217
Describe        : 銀聯列印時提示使用
 [20251215_BUG_MDF][UI] 新增分期交易用警示畫面
*/
int inFunc_DisplayInstallmentWarning(TRANSACTION_OBJECT *pobTran)
{
	char szDispBuf[_DISP_MSG_SIZE_ + 1] = {0};
	char szCustomerIndicator[3 + 1] = {0};
	unsigned char uszKey;
	int inChoice, inRetVal = VS_ERROR, inSignedFlag = _SIGNED_NONE_;
	int	inTouchSensorFunc = _Touch_INST_INFO_CHECK_;
    
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	/* 自助加油機不顯示Title */
	if (memcmp(gszTermVersionID, "MD731UAGAS001", strlen("MD731UAGAS001")) != 0)
	{
		inDISP_ClearAll();

		switch (pobTran->inTransactionCode)
		{			
			case _INST_SALE_:
				inDISP_PutGraphic(_MENU_INST_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_1_);	/* ＜分期付款＞ */
				break;
			case _INST_REFUND_:
				inDISP_PutGraphic(_MENU_INST_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_1_);	/* ＜分期退貨＞ */
				inSignedFlag = _SIGNED_MINUS_;
				break;
			case _INST_ADJUST_:
				/* [20260123_BUG_MDF][UI] 新增分期交易用警示畫面 
				 加入分期調帳判斷 */
				inDISP_PutGraphic(_MENU_INST_ADJUST_TITLE_, 0,  _COORDINATE_Y_LINE_8_1_);	/* ＜分期調帳＞ */
				break;
			default:
				/* 如果不是這二個交易,就不顯示分期付款的資料*/
				if (ginDebug == VS_TRUE)
					inLogPrintf(AT, "Installment TransType() Error", pobTran->inTransactionCode);
				return VS_SUCCESS;
		}
	}
	
	
	/* 顯示 確認鍵 圖示 */
	inDISP_PutGraphic(_CHECK_ENTER_KEY_, 0, _COORDINATE_Y_LINE_16_15_);
	/* 顯示 分期警示 圖示 */
	inDISP_PutGraphic(_INST_BMP_LEGAL_LOGO_, 0, _COORDINATE_Y_LINE_16_10_);

	/* [20260123_BUG_MDF][UI] 新增分期交易用警示畫面 
	調整 inSignedFlag 參數使用,只有金額需要進行負號判斷 
	調整顯示行數 */
	/* 交易金額 */
	memset(szDispBuf, 0x00, sizeof(szDispBuf));
	snprintf(szDispBuf, sizeof(szDispBuf), "%ld", pobTran->srBRec.lnTxnAmount);
	
	inFunc_Amount_Comma(szDispBuf, "NT $", '\x00', inSignedFlag, 13, _PADDING_RIGHT_);
	inDISP_ChineseFont_Color(szDispBuf, _FONTSIZE_16X16_, _LINE_16_3_, _COLOR_BLACK_, _DISP_RIGHT_);

	inDISP_ChineseFont_Color("期數", _FONTSIZE_16X16_, _LINE_16_4_, _COLOR_BLACK_, _DISP_LEFT_);
	memset(szDispBuf, 0x00, sizeof(szDispBuf));
	snprintf(szDispBuf, sizeof(szDispBuf), "%ld", pobTran->srBRec.lnInstallmentPeriod);
	inDISP_ChineseFont_Color(szDispBuf, _FONTSIZE_16X16_, _LINE_16_4_, _COLOR_BLACK_, _DISP_RIGHT_);
	

	inDISP_ChineseFont_Color("首期", _FONTSIZE_16X16_, _LINE_16_5_, _COLOR_BLACK_, _DISP_LEFT_);
	memset(szDispBuf, 0x00, sizeof(szDispBuf));
	snprintf(szDispBuf, sizeof(szDispBuf), "%ld", pobTran->srBRec.lnInstallmentDownPayment);
	inFunc_Amount_Comma(szDispBuf, "NT $", '\x00', _SIGNED_NONE_, 13, _PADDING_RIGHT_);
	inDISP_ChineseFont_Color(szDispBuf, _FONTSIZE_16X16_, _LINE_16_5_, _COLOR_BLACK_, _DISP_RIGHT_);
	
	inDISP_ChineseFont_Color("每期", _FONTSIZE_16X16_, _LINE_16_6_, _COLOR_BLACK_, _DISP_LEFT_);
	memset(szDispBuf, 0x00, sizeof(szDispBuf));
	snprintf(szDispBuf, sizeof(szDispBuf), "%ld", pobTran->srBRec.lnInstallmentPayment);
	inFunc_Amount_Comma(szDispBuf, "NT $", '\x00', _SIGNED_NONE_, 13, _PADDING_RIGHT_);
	inDISP_ChineseFont_Color(szDispBuf, _FONTSIZE_16X16_, _LINE_16_6_, _COLOR_BLACK_, _DISP_RIGHT_);
	
	inDISP_ChineseFont_Color("手續費", _FONTSIZE_16X16_, _LINE_16_7_, _COLOR_BLACK_, _DISP_LEFT_);
	memset(szDispBuf, 0x00, sizeof(szDispBuf));
	snprintf(szDispBuf, sizeof(szDispBuf), "%ld", pobTran->srBRec.lnInstallmentFormalityFee);
	inFunc_Amount_Comma(szDispBuf, "NT $", '\x00', _SIGNED_NONE_, 13, _PADDING_RIGHT_);
	inDISP_ChineseFont_Color(szDispBuf, _FONTSIZE_16X16_, _LINE_16_7_, _COLOR_BLACK_, _DISP_RIGHT_);
	
	/* 客製化107顯示訊息TimeOut 3秒 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_))
	{
		inDISP_Timer_Start(_TIMER_NEXSYS_2_, _CUSTOMER_107_BUMPER_DISPLAY_TIMEOUT_);
		inDISP_Timer_Start(_TIMER_NEXSYS_3_, _CUSTOMER_107_BUMPER_DISPLAY_TIMEOUT_);
	}
	/* (需求單 - 107276)自助交易標準400做法 by Russell 2018/12/27 上午 11:20 */
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_))
	{
		inDISP_Timer_Start(_TIMER_NEXSYS_2_, _CUSTOMER_111_KIOSK_STANDARD_DISPLAY_TIMEOUT_);
		inDISP_Timer_Start(_TIMER_NEXSYS_3_, _CUSTOMER_111_KIOSK_STANDARD_DISPLAY_TIMEOUT_);
	}
	else
	{
		inDISP_Timer_Start(_TIMER_NEXSYS_2_, 30);
		inDISP_Timer_Start(_TIMER_NEXSYS_3_, 15);
	}
	
	while (1)
	{
		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
		uszKey = uszKBD_Key();

		if (inTimerGet(_TIMER_NEXSYS_2_) == VS_SUCCESS)
		{
			uszKey = _KEY_TIMEOUT_;
		}

		if (inTimerGet(_TIMER_NEXSYS_3_) == VS_SUCCESS)
		{
			inDISP_BEEP(1, 0);
			inDISP_Wait(250);
		}

		if (uszKey == _KEY_ENTER_			|| 
			uszKey == _KEY_TIMEOUT_			||
			inChoice == _Touch_INST_INFO_CHECK_)
		{
			inRetVal = VS_SUCCESS;
			break;
		}
	}
	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();
	
	return inRetVal;

}

/* [260318_BUG] 新增控制參數 */
int inFunc_SwitchCardRetentionFlag(void)
{
	int inRetVal;
	char szTemplate[64 + 1];
	DISPLAY_OBJECT  srDispObj;

	/* Debug */
	inDISP_Clear_Line(_LINE_8_1_, _LINE_8_8_);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDISP_ChineseFont("是否開啟檢查", _FONTSIZE_8X22_, _LINE_8_3_, _DISP_LEFT_);
	if (ginToggleCardRetention == VS_TRUE)
	{
		inDISP_ChineseFont("Check: On", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
	}
	else
	{
		inDISP_ChineseFont("Check: Off", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
	}
	inDISP_ChineseFont("0 = OFF,1 = On", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);

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
				ginToggleCardRetention = VS_FALSE;
				break;
			}
			else if (srDispObj.szOutput[0] == '1')
			{
				ginToggleCardRetention = VS_TRUE;
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

/*
Function        :inFunc_Display_CHESG
Date&Time       :2026/4/21 下午 6:07
Describe        :
 * [114197] 中心收單作業ESG永續發展優化
 * [20260515] 新增螢幕顯示數位化簽帳單
 * 功能說明:
 * 此函式目前只能回傳 VS_SUCCESS 不能回傳錯誤,不然會中止後面的流程 
 */
int inFunc_Display_CHESG(TRANSACTION_OBJECT* pobTran)
{	
	int inRetVal;
	int inMaskCount = 6; /* 預設為 6,使用前6後4規則 */
	int inLoopFlag = VS_TRUE;
	int inOptional = 0;
	char szTemplate[128 + 1];
	char szDispAmount[52 + 1];
	char szCustomerIndicator[3];
	char szEnableESG[3] = {0};
	char szDemoMode[2 + 1] = {0};
	unsigned char uszkey;
	  
	if (ginMachineType == _CASTLE_TYPE_V3P_)
	{
		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "Suc:ESG V3P is not support ESG Function ");
		return VS_SUCCESS;
	}
	
	if(inGetCHESG_FLAG(szEnableESG) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "Err:ESG Get szEnableESG Error");
		return VS_SUCCESS;
	}
	
	if(szEnableESG[0] != 'Y')
	{	
		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "Err:ESG Function is not enable");
		return VS_SUCCESS;
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_Display_CHESG() START !");
		inLogPrintf(AT, "ESG szAgreeEsgBill[%s]", pobTran->szAgreeEsgBill);
	}
	
	/* 不同意顯示電子簽帳單，跳過 */
	if (pobTran->szAgreeEsgBill[0] == 'N')
	{
		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "Suc:ESG Value is %s", pobTran->szAgreeEsgBill);
		return VS_SUCCESS;
	}
	
	CTOS_LCDTClearDisplay();
	/* [114197] 中心收單作業ESG永續發展優化 
	 * [260610] 調整電子簽單的畫面，包含只秀商店表頭名、拉開行距、縮小某段文字內容、Qrcode改從主機回覆而來*/
	inDISP_ChineseFont_Point_Color_By_Graphic_Mode(_NCCC_TEXT_LOGO_, _FONTSIZE_16X26_, _COLOR_RED_, _COLOR_WHITE_, 4, _COORDINATE_Y_LINE_16_1_, VS_FALSE);

	/* 轉圖檔成功就使用圖檔，無合併圖檔就顯示LOG就好 */
	if(inFILE_Check_Exist((unsigned char *)_ESG_CUS_LOGO_NAME_) == VS_SUCCESS)
	{	
		if (inDISP_PutGraphic(_ESG_CUS_LOGO_PATH_AND_NAME_, 0, _COORDINATE_Y_LINE_16_2_) != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
				inLogPrintf(AT, "Log:ESG DispLog %s is Error", _ESG_CUS_LOGO_PATH_AND_NAME_);
		}
	}else
	{
		if (inDISP_PutGraphic(_NCCC_LOGO_, 0, _COORDINATE_Y_LINE_16_3_) != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
				inLogPrintf(AT, "Log:ESG DispLog %s is Error", _ESG_CUS_LOGO_PATH_AND_NAME_);
		}
	}
	
	memset(szTemplate, 0x00, sizeof (szTemplate));
	snprintf(szTemplate, sizeof(szTemplate), "卡號/卡別:%-16s授權碼:%s", pobTran->srBRec.szCardLabel, pobTran->srBRec.szAuthCode);
	inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szTemplate, _FONTSIZE_16X44_, _COLOR_BLACK_, _COLOR_WHITE_, 0, _COORDINATE_Y_LINE_24_6_, VS_FALSE);
	
	memset(szTemplate, 0x00, sizeof (szTemplate));
	snprintf(szTemplate, sizeof(szTemplate), pobTran->srBRec.szPAN);
	
	memset(szCustomerIndicator, 0x00, sizeof (szCustomerIndicator));
	inRetVal = inGetCustomIndicator(szCustomerIndicator);
	if(inRetVal == VS_SUCCESS)
	{	
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_039_CARD_NO_HIDE_F8_B4_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inMaskCount = 8;
		}	
	}
	
	/* 如果讀取客制化失敗,使用預設的前6後4 */
	for ( ; inMaskCount < (strlen(szTemplate) - 4); inMaskCount++)
	{
		szTemplate[inMaskCount] = 0x2A;
	}
	
	/* 依照卡片使用方式進行判別 */
	if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
		strcat(szTemplate,"(C)");
	else if (pobTran->srBRec.uszMobilePayBit == VS_TRUE)
		strcat(szTemplate, "(T)");
	else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
		strcat(szTemplate, "(W)");
	else
	{
		if (pobTran->srBRec.uszManualBit == VS_TRUE)
			strcat(szTemplate,"(M)");
		else
			strcat(szTemplate,"(S)");
	}

	inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szTemplate, _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, 0, _COORDINATE_Y_LINE_24_8_, VS_FALSE);

	memset(szTemplate, 0x00, sizeof (szTemplate));
	snprintf(szTemplate, sizeof(szTemplate), "日期/時間:%.4s/%.2s/%.2s %.2s:%.2s", pobTran->srBRec.szDate, pobTran->srBRec.szDate + 4, pobTran->srBRec.szDate + 6, pobTran->srBRec.szTime, pobTran->srBRec.szTime + 2);
	inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szTemplate, _FONTSIZE_16X33_, _COLOR_BLACK_, _COLOR_WHITE_, 0, _COORDINATE_Y_LINE_24_10_, VS_FALSE);
	
	memset(szDispAmount, 0x00, sizeof (szDispAmount));
	memset(szTemplate, 0x00, sizeof (szTemplate));
	snprintf(szDispAmount, sizeof(szDispAmount), "%ld", pobTran->srBRec.lnTxnAmount);
	inFunc_Amount_Comma(szDispAmount, "NT$", 0x00, _SIGNED_NONE_, 16, _PADDING_RIGHT_);

	snprintf(szTemplate, sizeof(szTemplate), "總計(Total) %17s", szDispAmount);
	inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szTemplate, _FONTSIZE_8X33_, _COLOR_BLACK_, _COLOR_WHITE_, 0, _COORDINATE_Y_LINE_16_9_, VS_FALSE);

	memset(szTemplate, 0x00, sizeof (szTemplate));
	strcpy(szTemplate, "持卡人存根聯");
	inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szTemplate, _FONTSIZE_16X26_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_16_9_, _COORDINATE_Y_LINE_16_11_, VS_FALSE);
	memset(szTemplate, 0x00, sizeof (szTemplate));
	strcpy(szTemplate, "I AGREE TO PAY");
	inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szTemplate, _FONTSIZE_24X44_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_16_9_, _COORDINATE_Y_LINE_16_12_+_COORDINATE_Y_LINE_24_2_, VS_FALSE);

	memset(szTemplate, 0x00, sizeof (szTemplate));
	strcpy(szTemplate, "TOTAL  AMOUNT");
	inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szTemplate, _FONTSIZE_24X44_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_16_9_, _COORDINATE_Y_LINE_16_12_+_COORDINATE_Y_LINE_24_3_, VS_FALSE);

	memset(szTemplate, 0x00, sizeof (szTemplate));
	strcpy(szTemplate, "ACCORDING TO CARD");
	inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szTemplate, _FONTSIZE_24X44_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_16_9_, _COORDINATE_Y_LINE_16_12_+_COORDINATE_Y_LINE_24_4_, VS_FALSE);

	memset(szTemplate, 0x00, sizeof (szTemplate));
	strcpy(szTemplate, "ISSUER AGREEMENT");
	inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szTemplate, _FONTSIZE_24X44_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_16_9_, _COORDINATE_Y_LINE_16_12_+_COORDINATE_Y_LINE_24_5_, VS_FALSE);

	memset(szTemplate, 0x00, sizeof (szTemplate));
	strcpy(szTemplate, "掃描 QR Code 取得數位簽單");
	inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szTemplate, _FONTSIZE_16X44_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_16_4_, _COORDINATE_Y_LINE_16_16_, VS_FALSE);
	
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		memset(szTemplate, 0x00, sizeof (szTemplate));
		snprintf(szTemplate, sizeof(szTemplate),"%s", pobTran->szCHESGGuid);
		inLogPrintf(AT,"QRCode is CHESGUrl[%s] + CHESGGuid[%s]",pobTran->szCHESGUrl,pobTran->szCHESGGuid);
	}else
	{
		memset(szTemplate, 0x00, sizeof (szTemplate));
		snprintf(szTemplate, sizeof(szTemplate),"%s%s",pobTran->szCHESGUrl, pobTran->szCHESGGuid);
		inLogPrintf(AT,"QRCode is CHESGUrl[%s] + CHESGGuid[%s]",pobTran->szCHESGUrl,pobTran->szCHESGGuid);
	}
	if (strlen(szTemplate) <= 100)
		inDISP_DisplayEsgQRCode(szTemplate, 0, _COORDINATE_Y_LINE_16_11_, 3, QR_VERSION49X49);
	else
		inDISP_DisplayEsgQRCode(szTemplate, 0, _COORDINATE_Y_LINE_16_11_, 2, QR_VERSION73X73);

	inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);
	/* 清鍵盤Buffer */
	inFlushKBDBuffer();
	do
	{
		uszkey = -1;
		uszkey = uszKBD_Key();
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			inLoopFlag = VS_FALSE;
			break;
		}
		
		switch (uszkey)
		{
			case _KEY_0_:
				//此畫面Timeout 時間30秒或按數字【0】鍵回待機畫面，
				inLoopFlag = VS_FALSE;
				break;
			case _KEY_1_:
				//因為szCHESGEnable為Y會擋列印簽帳單，所以這邊要改設定為N
				pobTran->szAgreeEsgBill[0] = 'N';
				/* [20260624] 重印字樣需要判斷此條件,所以備分原來檔再恢復,
				 * 目前適用於一般重印流程,如昇恆昌有客制化需再調整 */
				inOptional = pobTran->inRunOperationID;
				pobTran->inRunOperationID = _OPERATION_REPRINT_;
				
				/* [20260623] 列印時需要重新顯示抬頭及清畫面 */
				inDISP_ClearAll();
				inFunc_ResetTitle(pobTran);
				//按數字【1】鍵重印上一筆帳單
				inRetVal = inFLOW_RunFunction(pobTran, _FUNCTION_REPRINT_RECEIPT_BY_BUFFER_);
				
				/* 暫存資料恢復 */
				pobTran->inRunOperationID = inOptional;
				if(inRetVal != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "Err:ESG Run Print Receipt Error");
					}
				}
				inLoopFlag = VS_FALSE;
				break;
			default:
				continue;
		}
	}while(inLoopFlag == VS_TRUE);
	
	inDisTouch_Flush_TouchFile();
	inDISP_ClearAll();
	inFunc_ResetTitle(pobTran);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_Display_CHESG() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	return VS_SUCCESS;
}
/*
Function        :inFunc_CheckUseEsgFunction
Date&Time       :2026/4/21 下午 6:07
Describe        :
 * [114197] 中心收單作業ESG永續發展優化
 * [20260515] 新增輸入金額後 詢問持卡人是否同意接收數位化簽帳單
 * 功能需求說明:
 * 此函式目前只能回傳 VS_SUCCESS 不能回傳錯誤,不然會中止後面的流程 
 * 一般銷貨及正向交易含分期、紅利(不含負向交易退貨、取消/預授權/預授權
 * 完成/交易補登/預借現金/郵購/小費/DCC交易/遇Uny一維條碼交易/HAPPY GO/電票交易/SmartPay)
 */
int inFunc_CheckUseEsgFunction(TRANSACTION_OBJECT* pobTran)
{
	int inChoice = 0, inLoopFlag = VS_TRUE;
	int inRetVal = VS_ERROR;
	unsigned char uszkey = -1;
	char szCustomerIndicator[3 + 1] = {0};
	char szEnableESG[3] = {0};
	
	/* 使用者已進行過選項，就視為已選擇 */
	if(pobTran->uszEsgOptrionBit == VS_TRUE)
	{
		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "Log:ESG has been seletion ");
		return VS_SUCCESS;
	}
	
	/* 非V3C機型,不進入此選項  */
	if (ginMachineType != _CASTLE_TYPE_V3C_)
	{
		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "Log:ESG Machhine Type is not V3C  [%d]", ginMachineType);
		return VS_SUCCESS;
	}
	
	if(inGetCHESG_FLAG(szEnableESG) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "Log:ESG Get szEnableESG Error");
		return VS_SUCCESS;
	}
	
	if(szEnableESG[0] != 'Y')
	{	
		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "Log:ESG Function is not enable");
		return VS_SUCCESS;
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_CheckUseEsgFunction() START !");
	}
	
	/* 防呆，之前選擇過同意，不再修改參數 */
	if (pobTran->szAgreeEsgBill[0] == 'Y')
	{
		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "Log:ESG szAgreeEsgBill is %c", pobTran->szAgreeEsgBill[0]);
		return VS_SUCCESS;
	}
	
	/* 只要進入此功能,參數初始值就設定成 N */
	pobTran->szAgreeEsgBill[0] = 'N';
		
	//負向交易 或是 非一般交易/紅利/分期 則跳過。
	if (pobTran->srBRec.uszVOIDBit ||
		!(pobTran->srBRec.inCode == _SALE_ ||
		pobTran->srBRec.inCode == _INST_SALE_ ||
		pobTran->srBRec.inCode == _REDEEM_SALE_))
	{
		
		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "Log:ESG inCode IS NOT SUPPORTED Code[%d] VoidBit[%d]", pobTran->srBRec.inCode, pobTran->srBRec.uszVOIDBit );
		return VS_SUCCESS;
	}

	memset(szCustomerIndicator, 0x00, sizeof (szCustomerIndicator));
	inRetVal = inGetCustomIndicator(szCustomerIndicator);
	if(inRetVal == VS_SUCCESS)
	{			
		if(!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_000_, _CUSTOMER_INDICATOR_SIZE_) ||
			!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_038_BELLAVITA_, _CUSTOMER_INDICATOR_SIZE_) ||
			!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_039_CARD_NO_HIDE_F8_B4_, _CUSTOMER_INDICATOR_SIZE_) ||
			!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_076_8_DIGITS_, _CUSTOMER_INDICATOR_SIZE_) ||
			!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_099_SINYA_, _CUSTOMER_INDICATOR_SIZE_) ||
			!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_113_H_AND_M_, _CUSTOMER_INDICATOR_SIZE_) ||
			!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_067_KOBAY, _CUSTOMER_INDICATOR_SIZE_))
		{
			/* 執行後續流程 */
		}else
		{
			/* 因額制化不支援,參數需設定成 N,並回傳成功,不能中斷交易 */
			if (ginDebug == VS_TRUE)
				inLogPrintf(AT, "Log:ESG CustomerIndicator is [%s] ", szCustomerIndicator);
			pobTran->szEsgSendCR[0] = 'N';
			return VS_SUCCESS;
		}
		
	}else
	{
		/* 參數抓取失敗,設定不接受數位化簽單,參數需設定為 N,並回傳成功,不能中斷交易 */
		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "Log:ESG Get szCustomerIndicator Value Error ");
		return VS_SUCCESS;
	}
	
	inDISP_ClearAll();
	inDISP_PutGraphic(_MENU_HOST_098_NCCC_, 0, _COORDINATE_Y_LINE_16_2_);
	inDISP_Display_Black_Back(0, _COORDINATE_Y_LINE_8_3_, _LCD_XSIZE_, _LCD_YSIZE_/ 8);
	inDISP_ChineseFont_Point_Color_By_Graphic_Mode("   請 詢 問 持 卡 人 ", _FONTSIZE_8X22_,
								_COLOR_WHITE_, _COLOR_BLACK_, 0, _COORDINATE_Y_LINE_8_3_, VS_FALSE);
	inDISP_ChineseFont_Point_Color_By_Graphic_Mode("是否同意接收數位化簽帳單?", _FONTSIZE_8X33_,
								_COLOR_BLACK_, _COLOR_WHITE_, 0, _COORDINATE_Y_LINE_8_5_, VS_FALSE);
	inDISP_ChineseFont_Point_Color_By_Graphic_Mode("【同意(1)】", _FONTSIZE_8X33_,
								_COLOR_BLACK_, _COLOR_WHITE_, 0, _COORDINATE_Y_LINE_8_6_, VS_FALSE);
	inDISP_ChineseFont_Point_Color_By_Graphic_Mode("【不同意(0)】", _FONTSIZE_8X33_,
								_COLOR_BLACK_, _COLOR_WHITE_, 0, _COORDINATE_Y_LINE_8_8_, VS_FALSE);

	inDISP_Timer_Start(_TIMER_NEXSYS_1_, 7);
	/* 清鍵盤Buffer */
	inFlushKBDBuffer();
	do
	{
		uszkey = uszKBD_Key();
		inChoice = inDisTouch_TouchSensor_Click_Slide(_APPROVAL_CHECK_Touch_MENU_);
		
		switch (inChoice)
		{
			case _AGREE_TOUCH_YES_:
				uszkey = _KEY_1_;
				break;
			case _AGREE_TOUCH_NO_:
				uszkey = _KEY_0_;
				break;
		}
		
		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{			
			if (ginDebug == VS_TRUE)
				inLogPrintf(AT, "Log:ESG VS_TIMEOUT");
			/* 持卡人未選時,一般(ATS 及雲端 MFES)版本流程、小額(MPAS)版本流程可bypass 列紙本簽帳單。 */
			inLoopFlag = VS_FALSE;
			break;
		}

		switch (uszkey)
		{
			case _KEY_0_:
				if (ginDebug == VS_TRUE)
					inLogPrintf(AT, "Log:ESG Click Disagree");
				inLoopFlag = VS_FALSE;
				break;
			case _KEY_1_:
				if (ginDebug == VS_TRUE)
					inLogPrintf(AT, "Log:ESG Click Agree");
				pobTran->szAgreeEsgBill[0] = 'Y';
				inLoopFlag = VS_FALSE;
				break;
		}
		
	}while(inLoopFlag == VS_TRUE);
	
	inDisTouch_Flush_TouchFile();
	inDISP_ClearAll();
	inFunc_ResetTitle(pobTran);
	/* 只要有進行過此功能，就視為已選擇，不再顯示選項 */
	pobTran->uszEsgOptrionBit = VS_TRUE;
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_CheckUseEsgFunction() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return VS_SUCCESS;
}

/* 測試用,暫停後只能按確認鍵離開  */
int inFunc_GetKeyEnterOut()
{
	unsigned char uszkey;
	while(1)
	{
		uszkey = uszKBD_Key();
		
		if(uszkey == _KEY_ENTER_)
			break;
		
	}

	return VS_SUCCESS;
}

int inFunc_PrintEsgCustReceipt(TRANSACTION_OBJECT* pobTran)
{
	char szEnableESG[3] = {0};
	char szCustomerIndicator[4] = {0};

	/* [114197] 中心收單作業ESG永續發展優化 
	 * [20260515] 詢問持卡人是否同意顯示電子簽帳單，如果同意跳過紙本列印
	 * [20260623] 移動判斷條件,並加上ESC功能判斷及選擇ESG重印簽單的條件 */
	if (pobTran->szAgreeEsgBill[0] == 'Y')
	{
		/* 防呆,如果ESG功能沒開啟,誤設定 szAgreeEsgBill 時的判斷 */
		if (inGetCHESG_FLAG(szEnableESG) == VS_SUCCESS)
		{
			if (szEnableESG[0] == 'Y')
			{
				if (ginDebug == VS_TRUE)
					inLogPrintf(AT, "Suc: agree run ESG flow");

				/* 抓客制化,如果不符合,就依照規則列印簽單 */
				inGetCustomIndicator(szCustomerIndicator);
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_000_, _CUSTOMER_INDICATOR_SIZE_) ||
					!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_038_BELLAVITA_, _CUSTOMER_INDICATOR_SIZE_) ||
					!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_039_CARD_NO_HIDE_F8_B4_, _CUSTOMER_INDICATOR_SIZE_) ||
					!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_076_8_DIGITS_, _CUSTOMER_INDICATOR_SIZE_) ||
					!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_099_SINYA_, _CUSTOMER_INDICATOR_SIZE_) ||
					!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_113_H_AND_M_, _CUSTOMER_INDICATOR_SIZE_) ||
					!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_067_KOBAY, _CUSTOMER_INDICATOR_SIZE_))
				{
					/* 回傳不用列印簽單 */
					return VS_TRUE;
				}
			}
			vdUtility_SYSFIN_LogMessage(AT, "Log: ESG Not Enable And Agree is [Y]");
		} else
		{ /* 抓取參數失敗,視為進行列印 */
			if (ginDebug == VS_TRUE)
				inLogPrintf(AT, "Log: Get szEnableESG Error and print paper bill");
		}
	}
	return VS_FALSE;
}

