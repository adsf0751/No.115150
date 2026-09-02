#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <libxml/xmlmemory.h>
#include <curl/curl.h>
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
#include "../SOURCE/PRINT/PrtMsg.h"
#include "../HG/HGsrc.h"
#include "../SOURCE/FUNCTION/Sqlite.h"
#include "../SOURCE/FUNCTION/Accum.h"
#include "../SOURCE/FUNCTION/Batch.h"
#include "../SOURCE/FUNCTION/Function.h"
#include "../SOURCE/FUNCTION/FuncTable.h"
#include "../SOURCE/FUNCTION/HDT.h"
#include "../SOURCE/FUNCTION/CDT.h"
#include "../SOURCE/FUNCTION/CDTX.h"
#include "../SOURCE/FUNCTION/CPT.h"
#include "../SOURCE/FUNCTION/CPT_Backup.h"
#include "../SOURCE/FUNCTION/CFGT.h"
#include "../SOURCE/FUNCTION/EDC.h"
#include "../SOURCE/FUNCTION/HDPT.h"
#include "../SOURCE/FUNCTION/PWD.h"
#include "../SOURCE/FUNCTION/MVT.h"
#include "../SOURCE/FUNCTION/VWT.h"
#include "../SOURCE/FUNCTION/EST.h"
#include "../SOURCE/FUNCTION/SKM.h"
#include "../SOURCE/FUNCTION/ASMC.h"
#include "../SOURCE/FUNCTION/QAT.h"
#include "../SOURCE/FUNCTION/File.h"
#include "../SOURCE/FUNCTION/IPASSDT.h"
#include "../SOURCE/FUNCTION/TDT.h"
#include "../SOURCE/FUNCTION/SCDT.h"
#include "../SOURCE/FUNCTION/PCD.h"
#include "../SOURCE/FUNCTION/PIT.h"
#include "../SOURCE/FUNCTION/SPAY.h"
#include "../SOURCE/FUNCTION/ECCDT.h"
#include "../SOURCE/FUNCTION/Utility.h"
#include "../SOURCE/FUNCTION/EDC_Para_Table_Func.h"
#include "../SOURCE/FUNCTION/ICASHDT.h"
#include "../SOURCE/FUNCTION/TMSIPDT.h"
#include "../SOURCE/FUNCTION/Big5NameT.h"
#include "../SOURCE/EVENT/MenuMsg.h"
#include "../SOURCE/EVENT/Flow.h"
#include "../SOURCE/COMM/Comm.h"
#include "../SOURCE/COMM/Ethernet.h"
#include "../SOURCE/COMM/Modem.h"
#include "../SOURCE/COMM/Ftps.h"
#include "../SOURCE/COMM/TLS.h"
#include "../SOURCE/COMM/GPRS.h"
#include "../CREDIT/Creditfunc.h"
#include "../EMVSRC/EMVsrc.h"
#include "../EMVSRC/EMVxml.h"
#include "../CTLS/CTLS.h"
#include "../CREDIT/CreditprtByBuffer.h"
#include "../ECC/ICER/stdAfx.h"
#include "../ECC/ECC.h"
#include "NCCCsrc.h"
#include "NCCCtmsCPT.h"
#include "NCCCtmsFLT.h"
#include "NCCCtmsSCT.h"
#include "NCCCtmsiso.h"
#include "NCCCtmsFTP.h"
#include "NCCCtmsFTPFLT.h"
#include "NCCCdcc.h"
#include "NCCCTicketSrc.h"
#include "TAKAsrc.h"
#include "NCCCtms.h"

TMS_Field58_REC		gsrTMS_Field58;
FTP_OBJECT		gsrFTP;
unsigned long		gulDemoHappyGoPoint;		/* 註解DEMO用HG點數 */
unsigned long		gulDemoRedemptionPointsBalance;	/* DEMO用紅利點數 */
unsigned long		gulDemoTicketPoint;		/* DEMO用票證金額 */

extern  int			ginDebug;  /* Debug使用 extern */
extern	int			ginISODebug;
extern	int			ginDisplayDebug;
extern	int			ginDCCHostIndex;
extern	int			ginHGHostIndex;
extern	int			ginESVCHostIndex;
extern	int			ginMachineType;
extern	char			gszTermVersionID[16 + 1];
extern	char			gszTermVersionDate[16 + 1];
extern  DCC_DATA		gsrDCC_Download;
extern	char			gszParamDBPath[100 + 1];
extern	SQLITE_TAG_TABLE	TABLE_HDPT_TAG[];
/*
Function        :inNCCCTMS_TMS_Func5SelectFlow
Date&Time       :2016/1/6 下午 2:27
Describe        :功能五 TMS下載分流 ISO8583或FTPS
*/
int inNCCCTMS_TMS_Func5SelectFlow(TRANSACTION_OBJECT *pobTran)
{
	char	szCustomerIndicator[3 + 1] = {0};

        vdUtility_SYSFIN_LogMessage(AT, "inNCCCTMS_TMS_Func5SelectFlow START!");
        
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCCTMS_TMS_Func5SelectFlow() START !!");
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
        /* 先LOAD NCCC HOST */
        inLoadHDTRec(0);
        inLoadHDPTRec(0);
        /* TMS的CPT參數 */
        inLoadTMSCPTRec(0);
        /* EDC參數 */
        inLoadEDCRec(0);
        /* CPT參數 */
        inLoadCPTRec(0);
        /* FTP參數 */
        inLoadTMSFTPRec(0);
        
        pobTran->inTransactionCode = _NCCCTMS_LOGON_; /* 先Hard code */
        pobTran->inTMSDwdMode = _TMS_MANUAL_DOWNLOAD_; /* 手動下載 */
        
        /* 初始化Field 58 */
        inNCCCTMS_Field58_Initial(pobTran);
        
        /* 輸入端末機代號 */
        if (inNCCCTMS_TID_GET(pobTran) != VS_SUCCESS)
                return (VS_ERROR);
        
        /* 輸入商店代號 */
        if (inNCCCTMS_MID_GET(pobTran) != VS_SUCCESS)
                return (VS_ERROR);
        
        /* 版本名稱 版本日期確認畫面 */
        if (inNCCCTMS_Version_Check(pobTran) != VS_SUCCESS)
                return (VS_ERROR);    
        
        /* 通訊模式分流 */
        if (inNCCCTMS_TMS_SetCommParm(pobTran) != VS_SUCCESS)
                return (VS_ERROR);

	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_042_BDAU1_, _CUSTOMER_INDICATOR_SIZE_) ||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_043_BDAU9_, _CUSTOMER_INDICATOR_SIZE_))
	{
		if (inFLOW_RunFunction(pobTran, _NCCC_FUNCTION_BDAU_DELETE_BATCH_BEFORE_TMS_) != VS_SUCCESS)
			return (VS_ERROR);
	}

        /* 輸入下載管理號碼 */
        if (inNCCCTMS_Download_PWD_GET(pobTran) != VS_SUCCESS)
                return (VS_ERROR);

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCCTMS_TMS_Func5SelectFlow() END !!");
        
        return (VS_SUCCESS);
}

/*
Function        :inNCCCTMS_TID_GET
Date&Time       :2016/1/6 下午 2:27
Describe        :輸入Terminal ID
*/
int inNCCCTMS_TID_GET(TRANSACTION_OBJECT *pobTran)
{
        char		szTerminalID[16 + 1];
        int		inRetVal = 0;
        DISPLAY_OBJECT  srDispObj;
        
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCCTMS_TID_GET() START !!");
        
        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        inDISP_PutGraphic(_TMS_ENTER_TID_, 0, _COORDINATE_Y_LINE_8_4_);
        
        /* 顯示目前的TID */
        memset(szTerminalID, 0x00, sizeof(szTerminalID));
        inGetTerminalID(szTerminalID);
        inFunc_PAD_ASCII(szTerminalID, szTerminalID, ' ', 16, _PADDING_LEFT_);
        inDISP_EnglishFont(szTerminalID, _FONTSIZE_8X16_, _LINE_8_7_, _DISP_LEFT_);
        
        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
        srDispObj.inMaxLen = 8;
        srDispObj.inY = _LINE_8_7_;
        srDispObj.inR_L = _DISP_RIGHT_;
	srDispObj.inColor = _COLOR_RED_;
	srDispObj.inTimeout = 30;
	
	memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
	srDispObj.inOutputLen = 0;

        inRetVal = inDISP_Enter8x16(&srDispObj);
        
        if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
                return (VS_ERROR);
        
        if (strlen(srDispObj.szOutput) > 0)
        {
                inSetTerminalID(srDispObj.szOutput);
                inSaveHDTRec(0);
        }

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCCTMS_TID_GET() END !!");
        
        return (VS_SUCCESS);
}

/*
Function        :inNCCCTMS_MID_GET
Date&Time       :2016/1/6 下午 2:27
Describe        :輸入Merchant ID
*/
int inNCCCTMS_MID_GET(TRANSACTION_OBJECT *pobTran)
{
        char    szMerchantID[16 + 1];
        int	inRetVal = 0;
        DISPLAY_OBJECT  srDispObj;

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCCTMS_TID_GET() START !!");
        
        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        inDISP_PutGraphic(_TMS_ENTER_MID_, 0, _COORDINATE_Y_LINE_8_4_);
        
        /* 顯示目前的MID */
        memset(szMerchantID, 0x00, sizeof(szMerchantID));
        inGetMerchantID(szMerchantID);
        inFunc_PAD_ASCII(szMerchantID, szMerchantID, ' ', 16, _PADDING_LEFT_);
        inDISP_EnglishFont(szMerchantID, _FONTSIZE_8X16_, _LINE_8_7_, _DISP_LEFT_);

        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
        srDispObj.inMaxLen = 15;
        srDispObj.inY = _LINE_8_7_;
        srDispObj.inR_L = _DISP_RIGHT_;
	srDispObj.inColor = _COLOR_RED_;
	srDispObj.inTimeout = 30;

	memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
	srDispObj.inOutputLen = 0;
	
        inRetVal = inDISP_Enter8x16(&srDispObj);
        
        if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
                return (VS_ERROR);
        
        if (strlen(srDispObj.szOutput) > 0)
        {
                inSetMerchantID(srDispObj.szOutput);
                inSaveHDTRec(0);
        }        

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCCTMS_TID_GET() END !!");
        
        return (VS_SUCCESS);
}

/*
Function        :inNCCCTMS_Version_Check
Date&Time       :2016/1/6 下午 2:27
Describe        :確認版本名稱及版本日期
*/
int inNCCCTMS_Version_Check(TRANSACTION_OBJECT *pobTran)
{
        char		szDispMsg[16 + 1];
        unsigned char   uszKey;

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCCTMS_Version_Check() START !!");
        
        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        inDISP_PutGraphic(_CHECK_VERSION_, 0, _COORDINATE_Y_LINE_8_4_);
        
        memset(szDispMsg, 0x00, sizeof(szDispMsg));
        if (strlen(gszTermVersionID) > 0)
	{
		memcpy(szDispMsg, gszTermVersionID, strlen(gszTermVersionID));
	}
	else
	{
		inGetTermVersionID(szDispMsg);
	}
        inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);
        memset(szDispMsg, 0x00, sizeof(szDispMsg));
	if (strlen(gszTermVersionDate) > 0)
	{
		memcpy(szDispMsg, gszTermVersionDate, strlen(gszTermVersionDate));
	}
	else
	{
		inGetTermVersionDate(szDispMsg);
	}
        inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_7_, _DISP_LEFT_);
        
        while (1)
        {
                uszKey = uszKBD_GetKey(30);
                
                if (uszKey == _KEY_ENTER_)
                {
                        break;
                }
                else if (uszKey == _KEY_CANCEL_ || uszKey == _KEY_TIMEOUT_)
                {
                        return (VS_ERROR);
                }
                else
                {
                        continue;
                }
        }

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCCTMS_Version_Check() END !!");
        
        return (VS_SUCCESS);
}

/*
Function        :inNCCCTMS_TMS_SetCommParm
Date&Time       :2016/1/6 下午 2:27
Describe        :設定TMS HOST IP及TMS PORT
*/
int inNCCCTMS_TMS_SetCommParm(TRANSACTION_OBJECT *pobTran)
{
	int		i = 0;
        int		inRetVal = 0;
        char		szDispMsg[16 + 1] = {0};
	char		szTemplate[20 + 1] = {0};
	char		szTMSOK[2 + 1] = {0};
	char		szI_FESMode[2 + 1] = {0};
        unsigned char   uszKey;
        DISPLAY_OBJECT  srDispObj;

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCCTMS_TMS_SetCommParm() START !!");
	
	inLoadTMSIPDTRec(0);
	
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetCommMode(szTemplate);
	memset(szI_FESMode, 0x00, sizeof(szI_FESMode));
	inGetI_FES_Mode(szI_FESMode);
	/* 若是撥接，則輸入TMS電話號碼 */
	if (memcmp(szTemplate, _COMM_MODEM_MODE_, strlen(_COMM_MODEM_MODE_)) == 0)
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		/* TMS下載電話號碼 */
		inDISP_ChineseFont("TMS下載電話號碼", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);
		
		pobTran->uszFTP_TMS_Download = _TMS_DOWNLOAD_SECURE_MODE_NONE_;
		
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		inGetTMSPhoneNumber(szDispMsg);
		inFunc_PAD_ASCII(szDispMsg, szDispMsg, ' ', 16, _PADDING_LEFT_);
		inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);
		
		memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
		srDispObj.inMaxLen = 15;
		srDispObj.inY = _LINE_8_6_;
		srDispObj.inR_L = _DISP_RIGHT_;
		srDispObj.inColor = _COLOR_RED_;
		srDispObj.inTimeout = 30;

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;
	
		inRetVal = inDISP_Enter8x16(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (VS_ERROR);

		if (strlen(srDispObj.szOutput) > 0)
		{
			/* 自動將,轉p，避免壞檔 by Russell 2021/6/3 下午 5:34 */
			for (i = 0; i < srDispObj.inOutputLen; i++)
			{
				if (srDispObj.szOutput[i] == ',')
				{
					srDispObj.szOutput[i] = 'p';
				}
			}
			
			inSetTMSPhoneNumber(srDispObj.szOutput);
			inSaveTMSCPTRec(0);
		}
	}
	/* Ethernet才要選iso8583或FTP */
	else
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		/* 選擇FTP下載或ISO8583下載 */
		inDISP_ChineseFont("1.FTP下載", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);
		inDISP_ChineseFont("2.ISO8583下載", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);
		inDISP_ChineseFont("3.SFTP", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_LEFT_);

		while (1)
		{
			uszKey = uszKBD_GetKey(30);

			if (uszKey == _KEY_CANCEL_ || uszKey == _KEY_TIMEOUT_)
			{
				pobTran->uszFTP_TMS_Download = _TMS_DOWNLOAD_SECURE_MODE_NONE_;
				return (VS_ERROR);
			}
			else if (uszKey == _KEY_1_)
			{
				pobTran->uszFTP_TMS_Download = _TMS_DOWNLOAD_SECURE_MODE_FTPS_;
				break;
			}
			else if (uszKey == _KEY_2_)
			{
				pobTran->uszFTP_TMS_Download = _TMS_DOWNLOAD_SECURE_MODE_NONE_;
				break;
			}
			else if (uszKey == _KEY_3_)
			{
				pobTran->uszFTP_TMS_Download = _TMS_DOWNLOAD_SECURE_MODE_SFTP_;
				break;
			}
			else
			{
				continue;
			}
		}
		
		/* (需求單 - 107276)自助交易標準做法 MFES雲端化 by Russell 2019/3/6 上午 10:32 */
		/* TMSOK是0的時候 不選SFTP的話把cloudFES關閉 */
		memset(szTMSOK, 0x00, sizeof(szTMSOK));
		inGetTMSOK(szTMSOK);
		if (szTMSOK[0] != 'Y')
		{
			inLoadTMSFTPRec(0);
			inLoadCFGTRec(0);
			if (pobTran->uszFTP_TMS_Download == _TMS_DOWNLOAD_SECURE_MODE_SFTP_)
			{
				inSetCloud_MFES("Y");
				inSetI_FES_Mode("N");
				inSetFTPPortNum(_SFTP_DEFAULT_PORT_);
			}
			else if (pobTran->uszFTP_TMS_Download == _TMS_DOWNLOAD_SECURE_MODE_FTPS_)
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetI_FES_Mode(szTemplate);
				if (memcmp(szTemplate, "Y", strlen("Y")) == 0)
				{
					/* IFES會詢問帳密 */
				}
				else
				{
					/* 寫入預設值 */
					inSetFTPID(_FTP_DEFAULT_USER_);
					inSetFTPPW(_FTP_DEFAULT_PASSWORD_);
				}
				inSetCloud_MFES("N");
				inSetFTPPortNum(_FTP_DEFAULT_PORT_);
			}
			else
			{
				inSetCloud_MFES("N");
				inSetFTPPortNum(_FTP_DEFAULT_PORT_);
			}
			
			inSaveTMSFTPRec(0);
			inSaveCFGTRec(0);
		}
		else
		{
			inLoadTMSFTPRec(0);
			inLoadCFGTRec(0);
			if (pobTran->uszFTP_TMS_Download == _TMS_DOWNLOAD_SECURE_MODE_SFTP_)
			{
				inSetCloud_MFES("Y");
				inSetI_FES_Mode("N");
				inSetFTPPortNum(_SFTP_DEFAULT_PORT_);
			}
			else if (pobTran->uszFTP_TMS_Download == _TMS_DOWNLOAD_SECURE_MODE_FTPS_)
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetI_FES_Mode(szTemplate);
				if (memcmp(szTemplate, "Y", strlen("Y")) == 0)
				{
					/* IFES會詢問帳密 */
				}
				else
				{
					/* 寫入預設值 */	
					inSetFTPID(_FTP_DEFAULT_USER_);
					inSetFTPPW(_FTP_DEFAULT_PASSWORD_);
				}
				inSetCloud_MFES("N");
				inSetFTPPortNum(_FTP_DEFAULT_PORT_);
			}
			else
			{
				inSetCloud_MFES("N");
				inSetFTPPortNum(_FTP_DEFAULT_PORT_);
			}
			
			inSaveTMSFTPRec(0);
			inSaveCFGTRec(0);
		}

		if (pobTran->uszFTP_TMS_Download == _TMS_DOWNLOAD_SECURE_MODE_FTPS_	||
		    pobTran->uszFTP_TMS_Download == _TMS_DOWNLOAD_SECURE_MODE_SFTP_)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_ChineseFont("FTP HOST IP?", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			inGetTMS_IP_Primary(szDispMsg);
			inFunc_PAD_ASCII(szDispMsg, szDispMsg, ' ', 16, _PADDING_LEFT_);
			inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

			memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
			srDispObj.inMaxLen = 15;
			srDispObj.inY = _LINE_8_6_;
			srDispObj.inR_L = _DISP_RIGHT_;
			srDispObj.inColor = _COLOR_RED_;
			srDispObj.inTimeout = 30;
			
			memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
			srDispObj.inOutputLen = 0;

			inRetVal = inDISP_Enter8x16_NumDot(&srDispObj);

			if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
				return (VS_ERROR);

			if (strlen(srDispObj.szOutput) > 0)
			{
				inSetTMS_IP_Primary(srDispObj.szOutput);
				inSaveTMSIPDTRec(0);
			}
                        
                        if (!memcmp(szI_FESMode, "Y", 1))
                        {
                                /* 輸入FTP IP2 */
                                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                                inDISP_ChineseFont_Color("FTP IP2?", _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_BLACK_, _DISP_CENTER_);
                                memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                inGetTMS_IP_Second(szDispMsg);
                                inFunc_PAD_ASCII(szDispMsg, szDispMsg, ' ', 16, _PADDING_LEFT_);
                                inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

                                memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
                                srDispObj.inMaxLen = 15;
                                srDispObj.inY = _LINE_8_6_;
                                srDispObj.inR_L = _DISP_RIGHT_;
                                srDispObj.inColor = _COLOR_RED_;
                                srDispObj.inTimeout = 30;

                                memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
                                srDispObj.inOutputLen = 0;

                                inRetVal = inDISP_Enter8x16_NumDot(&srDispObj);

                                if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
                                        return (VS_ERROR);

                                if (strlen(srDispObj.szOutput) > 0)
                                {
                                        inSetTMS_IP_Second(srDispObj.szOutput);
                                        inSaveTMSIPDTRec(0);
                                }
                        }
		}

		/* 輸入TMS IP */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_TMS_ENTER_IP_, 0, _COORDINATE_Y_LINE_8_4_);
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		inGetTMS_IP_Primary(szDispMsg);
		inFunc_PAD_ASCII(szDispMsg, szDispMsg, ' ', 16, _PADDING_LEFT_);
		inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

		memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
		srDispObj.inMaxLen = 15;
		srDispObj.inY = _LINE_8_6_;
		srDispObj.inR_L = _DISP_RIGHT_;
		srDispObj.inColor = _COLOR_RED_;
		srDispObj.inTimeout = 30;

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;
	
		inRetVal = inDISP_Enter8x16_NumDot(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (VS_ERROR);

		if (strlen(srDispObj.szOutput) > 0)
		{
			inSetTMS_IP_Primary(srDispObj.szOutput);
			inSaveTMSIPDTRec(0);
		}

		/* 輸入TMS PORT */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_TMS_ENTER_PORT_, 0, _COORDINATE_Y_LINE_8_4_);
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		inGetTMS_PortNo_Primary(szDispMsg);
		inFunc_PAD_ASCII(szDispMsg, szDispMsg, ' ', 16, _PADDING_LEFT_);
		inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

		memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
		srDispObj.inMaxLen = 5;
		srDispObj.inY = _LINE_8_6_;
		srDispObj.inR_L = _DISP_RIGHT_;
		srDispObj.inColor = _COLOR_RED_;
		srDispObj.inTimeout = 30;

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;
	
		inRetVal = inDISP_Enter8x16(&srDispObj);

		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (VS_ERROR);

		if (strlen(srDispObj.szOutput) > 0)
		{
			inSetTMS_PortNo_Primary(srDispObj.szOutput);
			inSaveTMSIPDTRec(0);
		}
		
		if (!memcmp(szI_FESMode, "Y", 1))
		{
			/* 輸入TMS IP2 */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_ChineseFont_Color("TMS IP2?", _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_BLACK_, _DISP_CENTER_);
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			inGetTMS_IP_Second(szDispMsg);
			inFunc_PAD_ASCII(szDispMsg, szDispMsg, ' ', 16, _PADDING_LEFT_);
			inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

			memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
			srDispObj.inMaxLen = 15;
			srDispObj.inY = _LINE_8_6_;
			srDispObj.inR_L = _DISP_RIGHT_;
			srDispObj.inColor = _COLOR_RED_;
			srDispObj.inTimeout = 30;

			memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
			srDispObj.inOutputLen = 0;

			inRetVal = inDISP_Enter8x16_NumDot(&srDispObj);

			if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
				return (VS_ERROR);

			if (strlen(srDispObj.szOutput) > 0)
			{
				inSetTMS_IP_Second(srDispObj.szOutput);
				inSaveTMSIPDTRec(0);
			}

			/* 輸入TMS PORT2 */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_ChineseFont_Color("TMS PORT Num2?", _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_BLACK_, _DISP_CENTER_);
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			inGetTMS_PortNo_Second(szDispMsg);
			inFunc_PAD_ASCII(szDispMsg, szDispMsg, ' ', 16, _PADDING_LEFT_);
			inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

			memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
			srDispObj.inMaxLen = 5;
			srDispObj.inY = _LINE_8_6_;
			srDispObj.inR_L = _DISP_RIGHT_;
			srDispObj.inColor = _COLOR_RED_;
			srDispObj.inTimeout = 30;

			memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
			srDispObj.inOutputLen = 0;

			inRetVal = inDISP_Enter8x16(&srDispObj);

			if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
				return (VS_ERROR);

			if (strlen(srDispObj.szOutput) > 0)
			{
				inSetTMS_PortNo_Second(srDispObj.szOutput);
				inSaveTMSIPDTRec(0);
			}
		}
	}
        
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCCTMS_TMS_SetCommParm() END !!");
        
        return (VS_SUCCESS);
}

/*
Function        :inNCCCTMS_Download_PWD_GET
Date&Time       :2016/1/6 下午 2:27
Describe        :輸入下載管理號碼
*/
int inNCCCTMS_Download_PWD_GET(TRANSACTION_OBJECT *pobTran)
{
        int		inRetVal = 0;
        DISPLAY_OBJECT  srDispObj;

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCCTMS_Download_PWD_GET() START !!");
        
        /* FTP模式不用輸入（不是ISO就不輸入） */
        if (pobTran->uszFTP_TMS_Download != _TMS_DOWNLOAD_SECURE_MODE_NONE_)
	{
                return (VS_SUCCESS);
	}
        
        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        inDISP_PutGraphic(_TMS_ENTER_DWL_PWD_, 0, _COORDINATE_Y_LINE_8_4_);
        
	while (1)
	{
                inDISP_BEEP(1, 0);
		inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
		memset(&srDispObj, 0x00, DISPLAY_OBJECT_SIZE);

                /* 設定顯示變數 */
                srDispObj.inMaxLen = 6;
                srDispObj.inY = _LINE_8_7_;
                srDispObj.inR_L = _DISP_RIGHT_;
                srDispObj.inMask = VS_TRUE;
		srDispObj.inColor = _COLOR_RED_;
		srDispObj.inTimeout = 30;
		
		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;
	
                inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);
		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (VS_ERROR);

		if (srDispObj.inOutputLen > 0)
                {
                        memcpy(&gsrTMS_Field58.szManagementCode[0], &srDispObj.szOutput[0], 6);
                        break;
                }
                else 
                        continue;
	}

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCCTMS_Download_PWD_GET() END !!");
        
        return (VS_SUCCESS);
}

/*
Function        :inNCCCTMS_ConnectToServer
Date&Time       :2016/1/6 下午 2:27
Describe        :連線到TMS主機
*/
int inNCCCTMS_ConnectToServer(TRANSACTION_OBJECT *pobTran)
{
        char    szCommmode[2 + 1] = {0};
	char	szIFESMode[2 + 1] = {0};
	char	szUserName[20 + 1] = {0};
	char	szPassWord[20 + 1] = {0};
	char	szCFESMode[2 + 1] = {0};
	
        vdUtility_SYSFIN_LogMessage(AT, "inNCCCTMS_ConnectToServer START!");
        
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCCTMS_ConnectToServer() START !!");
        
        memset(szCommmode, 0x00, sizeof(szCommmode));
        inGetCommMode(szCommmode);
        
        if (memcmp(szCommmode, _COMM_MODEM_MODE_, 1) == 0)
        {
                /* 撥接 */
		if (inModem_Connect_TMS() != VS_SUCCESS)
		{
			pobTran->inErrorMsg = _ERROR_CODE_V3_COMM_;
			return (VS_ERROR);
		}
        }
        else if (memcmp(szCommmode, _COMM_ETHERNET_MODE_, 1) == 0)
        {
                if (pobTran->uszFTP_TMS_Download == _TMS_DOWNLOAD_SECURE_MODE_FTPS_	||
		    pobTran->uszFTP_TMS_Download == _TMS_DOWNLOAD_SECURE_MODE_SFTP_)
                {
                        if (inLoadTMSFTPRec(0) != VS_SUCCESS)
                        {
				pobTran->inErrorMsg = _ERROR_CODE_V3_COMM_;
                                return (VS_ERROR);
                        }
			
			/* IFES 要先詢問帳號密碼 */
			/* CFES也要 */
			memset(szIFESMode, 0x00, sizeof(szIFESMode));
			inGetI_FES_Mode(szIFESMode);
			memset(szCFESMode, 0x00, sizeof(szCFESMode));
			inGetCloud_MFES(szCFESMode);
			if (memcmp(szIFESMode, "Y", strlen("Y")) == 0	||
			    memcmp(szCFESMode, "Y", strlen("Y")) ==0)
			{
				if (inETHERNET_NCCCTMS_Check() != VS_SUCCESS)
				{
					pobTran->inErrorMsg = _ERROR_CODE_V3_COMM_;
					return (VS_ERROR);
				}
				
				if (inNCCCTMS_IFES_GET_FTP_IDPW(pobTran) != VS_SUCCESS)
					return (VS_ERROR);
				
				inETHERNET_END();
			}
			else
			{
				/* 避免關閉之後連原本的FTPS有問題 設回預設ID和PW */
				memset(szUserName, 0x00, sizeof(szUserName));
				strcpy(szUserName, "edcuser");
				inSetFTPID(szUserName);
				memset(szPassWord, 0x00, sizeof(szPassWord));
				strcpy(szPassWord, "edc@123");
				inSetFTPPW(szPassWord);
			}
			
                }
                else
                {
                        if (inETHERNET_NCCCTMS_Check() != VS_SUCCESS)
                        {
				pobTran->inErrorMsg = _ERROR_CODE_V3_COMM_;
                                return (VS_ERROR);
                        }
                }
        }
	else if (memcmp(szCommmode, _COMM_GPRS_MODE_, 1) == 0	||
		 memcmp(szCommmode, _COMM_3G_MODE_, 1) == 0	||
		 memcmp(szCommmode, _COMM_4G_MODE_, 1) == 0)
        {
                if (pobTran->uszFTP_TMS_Download == _TMS_DOWNLOAD_SECURE_MODE_FTPS_	||
		    pobTran->uszFTP_TMS_Download == _TMS_DOWNLOAD_SECURE_MODE_SFTP_)
                {
                        if (inLoadTMSFTPRec(0) != VS_SUCCESS)
                        {
				pobTran->inErrorMsg = _ERROR_CODE_V3_COMM_;
                                return (VS_ERROR);
                        }
                            
			/* IFES 要先詢問帳號密碼 */
			memset(szIFESMode, 0x00, sizeof(szIFESMode));
			inGetI_FES_Mode(szIFESMode);
			memset(szCFESMode, 0x00, sizeof(szCFESMode));
			inGetCloud_MFES(szCFESMode);
			if (memcmp(szIFESMode, "Y", strlen("Y")) == 0	||
			    memcmp(szCFESMode, "Y", strlen("Y")) ==0)
			{
				if (inGPRS_NCCCTMS_Check() != VS_SUCCESS)
				{
					pobTran->inErrorMsg = _ERROR_CODE_V3_COMM_;
					return (VS_ERROR);
				}
				
				if (inNCCCTMS_IFES_GET_FTP_IDPW(pobTran) != VS_SUCCESS)
					return (VS_ERROR);
				
				inGPRS_END();
			}
			else
			{
				/* 避免關閉之後連原本的FTPS有問題 設回預設ID和PW */
				memset(szUserName, 0x00, sizeof(szUserName));
				strcpy(szUserName, "edcuser");
				inSetFTPID(szUserName);
				memset(szPassWord, 0x00, sizeof(szPassWord));
				strcpy(szPassWord, "edc@123");
				inSetFTPPW(szPassWord);
			}
			
                }
                else
                {
                        if (inGPRS_NCCCTMS_Check() != VS_SUCCESS)
                        {
				pobTran->inErrorMsg = _ERROR_CODE_V3_COMM_;
                                return (VS_ERROR);
                        }
                }
        }
        else
                return (VS_ERROR);

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCCTMS_ConnectToServer() END !!");
        
        return (VS_SUCCESS);
}

/*
Function        :inNCCCTMS_DisConnect_From_Server
Date&Time       :2018/6/27 下午 4:00
Describe        :TMS主機斷線
*/
int inNCCCTMS_DisConnect_From_Server(TRANSACTION_OBJECT *pobTran)
{
        vdUtility_SYSFIN_LogMessage(AT, "inNCCCTMS_DisConnect_From_Server START!");
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCCTMS_DisConnect_From_Server() START !");
	}

	inFLOW_RunFunction(pobTran, _COMM_END_);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCCTMS_DisConnect_From_Server() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	return (VS_SUCCESS);
}

/*
Function        :inNCCCTMS_FuncResultHandle
Date&Time       :2016/1/6 下午 2:27
Describe        :列印TMS下載狀態條及立即更新
*/
int inNCCCTMS_FuncResultHandle(TRANSACTION_OBJECT *pobTran)
{
        int			inRetVal, i = 0, j = 0, inSlash = 0, inFileNameLen = 0, inHeight = 0;
        char			szTemplate[60 + 1] = {0};
	char			szSN[15 + 1] = {0};
        char			szPrintBuf[384 + 1] = {0};
        char			szFileName[60 + 1] = {0};
	char			szCloseBatchBit[2 + 1] = {0};
	unsigned char		uszUpdateBit = VS_FALSE;
	VS_BOOL 		fDWLSuccess = VS_SUCCESS;
	unsigned char		uszBuffer[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	BufferHandle		srBhandle;
	FONT_ATTRIB		srFont_Attrib;
	
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCCTMS_FuncResultHandle() START !!");
        
        /* FTPS下載跑這邊 */
        if (pobTran->uszFTP_TMS_Download == _TMS_DOWNLOAD_SECURE_MODE_FTPS_	||
	    pobTran->uszFTP_TMS_Download == _TMS_DOWNLOAD_SECURE_MODE_SFTP_)
        {
                inNCCCTMS_FTPS_FuncResultHandle(pobTran);
        }
        
        /* 斷線 */
        inCOMM_End(pobTran);

        /* 下載完成才印下載檔案狀態紙條 */
        if (inFILE_Check_Exist((unsigned char *)_TMSFLT_FILE_NAME_) != VS_SUCCESS)
                return (VS_SUCCESS);
        
        /* 自動詢問和下載的不用印 */
        if (pobTran->inTMSDwdMode == _TMS_AUTO_DOWNLOAD_)
        {
                for (i = 0 ;; i++)
                {       
                        /* 下載結果已經存在File Index */
                        if (inLoadTMSFLTRec(i) < 0)
                                break;
                        
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        /* 取得下載結果 Y or N */
                        inGetTMSFileIndex(szTemplate);
                        
                        if (!memcmp(&szTemplate[0], "Y", 1))
                        {
                                /* 下載成功且是排程下載，將指定詢問時間初始化*/
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                inGetTMSInquireMode(szTemplate);
                                        
                                if (!memcmp(&szTemplate[0], _TMS_INQUIRE_02_SCHEDHULE_SETTLE_, 1))
                                {
                                        inSetTMSInquireStartDate("00000000");
                                        inSetTMSInquireTime("000000");
                                        inSaveTMSSCTRec(0);
                                }                                
                        }
                        else
                        {
                                fDWLSuccess = VS_ERROR;
                        }                
                }
        }
	else
	{
		/* 是否有列印功能 */
		if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
		{
			/* 列印TMS下載結果 */
			for (i = 0;; i++)
			{       
				/* 下載結果已經存在File Index，取得檔名後直接列印結果 */
				if (inLoadTMSFLTRec(i) < 0)
					break;

				/* 取得下載結果 Y or N */
				/* 目前會下載未定義參數，但不更新 */
				inGetTMSFileIndex(szTemplate);

				if (!memcmp(&szTemplate[0], "Y", 1))
				{
					
				}
				else
				{
					fDWLSuccess = VS_ERROR;
					break;
				}                
			}
		}
		else
		{
			inPRINT_Buffer_Initial(uszBuffer, _BUFFER_MAX_LINE_, &srFont_Attrib, &srBhandle);

			/* 列印下載結果 */
			inPRINT_Buffer_GetHeight((unsigned char*)"TMS_STATUS.bmp", &inHeight);

			/* Title TMS下載狀態 */
			if (inPRINT_Buffer_PutGraphic((unsigned char*)_TMS_DOWNLOAD_STATUS_, uszBuffer, &srBhandle, inHeight, _APPEND_) != VS_SUCCESS)
				return (VS_ERROR);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			/* 列印時間 */
			inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			inCREDIT_PRINT_Printing_Time(pobTran, _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle);

			/* Get商店代號 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetMerchantID(szTemplate);

			/* 列印商店代號 */
			sprintf(szPrintBuf, "商店代號");
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szTemplate);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			/* Get端末機代號 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetTerminalID(szTemplate);

			/* 列印端末機代號 */
			sprintf(szPrintBuf, "端末機代號");
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szTemplate);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			/* 分隔線 */
			inRetVal = inPRINT_Buffer_PutIn("================================================", _PRT_NORMAL_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			/* Terminal AP Name */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			if (strlen(gszTermVersionID) > 0)
			{
				memcpy(szTemplate, gszTermVersionID, strlen(gszTermVersionID));
			}
			else
			{
				inGetTermVersionID(szTemplate);
			}
			sprintf(szPrintBuf, "VERSION ID = %s", szTemplate);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			/* Terminal AP Version */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			if (strlen(gszTermVersionDate) > 0)
			{
				memcpy(szTemplate, gszTermVersionDate, strlen(gszTermVersionDate));
			}
			else
			{
				inGetTermVersionDate(szTemplate);
			}
			sprintf(szPrintBuf, "VERSION DATE = %s", szTemplate);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			/* Terminal S/N */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			/* 取後12碼，但最後一碼為CheckSum，所以取4~15 */
			inFunc_GetSeriaNumber(szTemplate);
			memset(szSN, 0x00, sizeof(szSN));
			memcpy(szSN, &szTemplate[3], 12);
			sprintf(szPrintBuf, "TERMINAL S/N = %s", szSN);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			strcpy(szPrintBuf, "下載檔案清單：(依下清單列示)");
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			strcpy(szPrintBuf, "　　　　　　　 成功　　失敗");
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			/* 列印TMS下載結果 */
			for (i = 0;; i++)
			{       
				/* 下載結果已經存在File Index，取得檔名後直接列印結果 */
				if (inLoadTMSFLTRec(i) < 0)
					break;

				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetTMSFilePathName(szTemplate);

				/* 取得檔案路徑後 抓取檔名儲存到FileName */
				for (j = 0; j < 60 ; j++)
				{
					if (szTemplate[j] == '/')
					    inSlash = j + 1;

					if (szTemplate[j] == 0x00)
					    break;
				}

				inFileNameLen = j - inSlash;
				memset(szFileName, 0x00, sizeof(szFileName));
				memcpy(&szFileName[0], &szTemplate[inSlash], inFileNameLen);
				inFunc_PAD_ASCII(szFileName, szFileName, ' ', 16, _PADDING_RIGHT_);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				strcpy(szPrintBuf, szFileName);

				/* 取得下載結果 Y or N */
				/* 目前會下載未定義參數，但不更新 */
				inGetTMSFileIndex(szTemplate);

				if (!memcmp(&szTemplate[0], "Y", 1))
					strcat(szPrintBuf, "●");
				else
				{
					strcat(szPrintBuf, "　　　　●");
					fDWLSuccess = VS_ERROR;
				}                

				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}

			if (fDWLSuccess == VS_SUCCESS)
			{
				inPRINT_Buffer_GetHeight((unsigned char*)"TMS_SUCCESS.bmp", &inHeight);

				if (inPRINT_Buffer_PutGraphic((unsigned char*)_TMS_DOWNLOAD_SUCCESS_, uszBuffer, &srBhandle, inHeight, _APPEND_) != VS_SUCCESS)
					return (VS_ERROR);
			}
			else
			{
				inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "Response Code = [  %s ]", pobTran->srBRec.szRespCode);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				inPRINT_Buffer_GetHeight((unsigned char*)"TMS_FAILURE.bmp", &inHeight);

				if (inPRINT_Buffer_PutGraphic((unsigned char*)_TMS_DOWNLOAD_FAILURE_, uszBuffer, &srBhandle, inHeight, _APPEND_) != VS_SUCCESS)
					return (VS_ERROR);
			}

			for (i = 0; i < 8; i++)
			{
				inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}

			inPRINT_Buffer_OutPut(uszBuffer, &srBhandle);
		}
	}
	
	if (fDWLSuccess != VS_SUCCESS)
	{
		if (memcmp(gszTermVersionID, "MD731UAGAS001", strlen("MD731UAGAS001")) == 0)
		{
			/* 當SDK不刪除，原來流程會在更新時，重下TMS時刪除，所以沒關係 */
		}
		else
		{
			/* 刪除File List */
			inFILE_Delete((unsigned char *)_TMSFLT_FILE_NAME_);
		}
	}
	else
	{
		/* 紀錄TMS的下載方式，已因應更新時的不同行為 */
		if (pobTran->inTMSDwdMode == _TMS_MANUAL_DOWNLOAD_)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%d", _TMS_MANUAL_DOWNLOAD_);
			inSetLastDownloadMode(szTemplate);
		}
		else if (pobTran->inTMSDwdMode == _TMS_AUTO_DOWNLOAD_)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%d", _TMS_AUTO_DOWNLOAD_);
			inSetLastDownloadMode(szTemplate);
		}
		else
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%d", _TMS_DEFAULT_DOWNLOAD_);
			inSetLastDownloadMode(szTemplate);
		}
		inSaveEDCRec(0);
		
		/* 下載完，若可以更新立即重開機 */
		if (inNCCCTMS_Schedule_Effective_Date_Time_Check(pobTran) == VS_SUCCESS)
		{
			/* 檢查時間到時，檢查FileList 及 下載檔案是否合法 */
			if (inNCCCTMS_CheckAllDownloadFile_Flow(pobTran) != VS_SUCCESS)
				return (VS_SUCCESS);

			/* 檢查是否有帳 */
			inLoadTMSCPTRec(0);
			inGetTMSEffectiveCloseBatch(szCloseBatchBit);
			if (memcmp(szCloseBatchBit, "Y", strlen("Y")) == 0)
			{
				if (inFLOW_RunFunction(pobTran, _FUNCTION_All_HOST_MUST_SETTLE_) != VS_SUCCESS)
				{
					return (VS_SUCCESS);
				}
				else
				{
					uszUpdateBit = VS_TRUE;
				}
			}
			else
			{
				uszUpdateBit = VS_TRUE;
			}
		}
		/* 可以更新，直接重開機 */
		if (uszUpdateBit == VS_TRUE)
		{
			pobTran->uszTMSDownloadRebootBit = VS_TRUE;
		}
	}

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCCTMS_FuncResultHandle() END !!");
        
        return (VS_SUCCESS);
}

/*
Function        :inNCCCTMS_Field58_Initial
Date&Time       :2016/1/6 下午 2:27
Describe        :Field 58的初始化
*/
int inNCCCTMS_Field58_Initial(TRANSACTION_OBJECT *pobTran)
{
        char szTemplate[60 + 1] = {0};

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCCTMS_Field58_Initial() START !!");
        
        memset(&gsrTMS_Field58, 0x00, sizeof(TMS_Field58_REC));
        
        /* Management Code */
        strcpy(gsrTMS_Field58.szManagementCode, "      "); /* 預設為自動下載 6碼空白 */
        
        /* Download Scope */
        strcpy(gsrTMS_Field58.szDownloadScope, "2"); /* Full Download */
        
        /* Batch Number */
        strcpy(gsrTMS_Field58.szBatchNumber, "        "); /* 作業批號預設空白 */
        
        /* 版本名稱 szTerminalVersionID */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        if (strlen(gszTermVersionID) > 0)
	{
		memcpy(szTemplate, gszTermVersionID, strlen(gszTermVersionID));
	}
	else
	{
		inGetTermVersionID(szTemplate);
	}
        memcpy(&gsrTMS_Field58.szTerminalVersionID[0], &szTemplate[0], 15);
  
        /* 版本日期 szTerminalAPVersionDate */
        memset(szTemplate, 0x00, sizeof(szTemplate));
	if (strlen(gszTermVersionDate) > 0)
	{
		memcpy(szTemplate, gszTermVersionDate, strlen(gszTermVersionDate));
	}
	else
	{
		inGetTermVersionDate(szTemplate);
	}
        memcpy(&gsrTMS_Field58.szTerminalAPVersionDate[0], &szTemplate[0], 8);
       
	/* szTSAMIndicator */
	/* 目前hard code為 "1      " */
	
        /* 端末機可下載Bytes數 szTerminalPacketSize */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTMSPacketSize(szTemplate);
	/* 防呆，填入預設值 */
	if (atoi(szTemplate) <= 0)
	{
		sprintf(szTemplate, "%s", _TMS_TerminalPacketSize_);
	}
	else
	{
		sprintf(szTemplate, "%04d", atoi(szTemplate));
	}
	memcpy(gsrTMS_Field58.szTerminalPacketSize, szTemplate, 4);
        
        /* 端末機上Serial Number */
	/* 取後12碼，但最後一碼為CheckSum，所以取4~15 */
        memset(szTemplate, 0x00, sizeof(szTemplate));
	inFunc_GetSeriaNumber(szTemplate);
        memcpy(&gsrTMS_Field58.szTerminalSN[0], &szTemplate[3], 12);
	
	/* TSAM SLOT SN1 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetSAMSlotSN1(szTemplate);
        memcpy(&gsrTMS_Field58.szTSAMSLOTSN1[0], &szTemplate[0], 16);
	
	/* TSAM SLOT SN2 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetSAMSlotSN2(szTemplate);
        memcpy(&gsrTMS_Field58.szTSAMSLOTSN2[0], &szTemplate[0], 16);
	
	/* TSAM SLOT SN3 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetSAMSlotSN3(szTemplate);
        memcpy(&gsrTMS_Field58.szTSAMSLOTSN3[0], &szTemplate[0], 16);
	
	/* TSAM SLOT SN4 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetSAMSlotSN4(szTemplate);
        memcpy(&gsrTMS_Field58.szTSAMSLOTSN4[0], &szTemplate[0], 16);
	
        /* 端末機上OS的版本 */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        inFunc_GetOSVersion((unsigned char*)szTemplate);
        memcpy(&gsrTMS_Field58.szTerminalOSVersion[0], &szTemplate[0], 16);

        /* EMV Approval Number */
        strcpy(gsrTMS_Field58.szEMVApprovalNumber, "2-02391-2-3C-FIM-1212-4.3.a");
        
        /* EMV Application Kernel */
        strcpy(gsrTMS_Field58.szEMVApplicationKernel, "UPT 6.0.1");
        
        /* EDC IP Address */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetTermIPAddress(szTemplate);
        memcpy(&gsrTMS_Field58.szIPAddress[0], &szTemplate[0], 15);

        /* EDC GateWay Address */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetTermGetewayAddress(szTemplate);
        memcpy(&gsrTMS_Field58.szGatewayAddress[0], &szTemplate[0], 15);

        /* EDC MASK Address */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetTermMASKAddress(szTemplate);
        memcpy(&gsrTMS_Field58.szMASKAddress[0], &szTemplate[0], 15);

	/* PABX CODE */
	memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetPABXCode(szTemplate);
        memcpy(&gsrTMS_Field58.szPABX[0], &szTemplate[0], 1);
	
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCCTMS_Field58_Initial() END !!");
        
        return (VS_SUCCESS);
}

/*
Function        :inNCCCTMS_IdleUpdate
Date&Time       :2016/12/6 下午 5:36
Describe        :IDLE 參數生效(看參數生效日)
*/
int inNCCCTMS_IdleUpdate(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	char	szCloseBatchBit[2 + 1] = {0};
	
        vdUtility_SYSFIN_LogMessage(AT, "inNCCCTMS_IdleUpdate START!");
        
	inRetVal = inNCCCTMS_Check_FileList_Flow(pobTran);
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
	/* 檢查是否有帳 */
	inLoadTMSCPTRec(0);
	inGetTMSEffectiveCloseBatch(szCloseBatchBit);
	if (memcmp(szCloseBatchBit, "Y", strlen("Y")) == 0)
	{
		inRetVal = inFLOW_RunFunction(pobTran, _FUNCTION_All_HOST_MUST_SETTLE_);
		if (inRetVal != VS_SUCCESS)
		{
			return (inRetVal);
		}
	}
	
	inRetVal = inNCCCTMS_UpdateParam_Flow(pobTran);

	if (inRetVal == VS_SUCCESS)
	{
		inNCCCTMS_Deal_With_Things_After_Parameter_Update(pobTran);
		/* 更新完一定重開機 */
		inFunc_Reboot();
	}
	else
	{
		/* 更新失敗 刪除File List */
		inNCCCTMS_Delete_FileList_Flow(pobTran);
		inSetTMSOK("N");
		inSaveEDCRec(0);
	}
	
	return (inRetVal);
}

/*
Function        :inNCCCTMS_UpdateParam_Flow
Date&Time       :2017/3/1 下午 5:56
Describe        :檔案更新分流
*/
int inNCCCTMS_UpdateParam_Flow(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;
	char	szDebugMsg[100 + 1];
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCCTMS_UpdateParam_Flow() START !");
	}
	
	if (pobTran->uszFTP_TMS_Download == _TMS_DOWNLOAD_SECURE_MODE_FTPS_	||
	    pobTran->uszFTP_TMS_Download == _TMS_DOWNLOAD_SECURE_MODE_SFTP_)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "UPDATE CHOOSE FTPS");
			inLogPrintf(AT, szDebugMsg);
		}
		
		inRetVal = inNCCCTMS_UpdateParam(pobTran);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "UPDATE CHOOSE ISO8583");
			inLogPrintf(AT, szDebugMsg);
		}
		
		inRetVal = inNCCCTMS_UpdateParam(pobTran);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCCTMS_UpdateParam_Flow() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :inNCCCTMS_UpdateParam
Date&Time       :2016/1/6 下午 2:27
Describe        :檔案更新
*/
int inNCCCTMS_UpdateParam(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal = VS_ERROR;
	int i = 0, j = 0, k = 0, inFileNameLen = 0, inSlash = 0, inRecord;
	char szTemplate[60 + 1] = {0};
	char szTemplate2[60 + 1] = {0};
	char szTMSAPPUpdateStatus[2 + 1];
	char szDebugMsg[100 + 1];
	char szCustomerIndicator[3 + 1] = {0};
	unsigned char uszFileName[32 + 1];
	unsigned char uszUpdateFailBit = VS_FALSE;
	VS_BOOL fEMVDef1 = VS_FALSE;
	VS_BOOL fEMVCLDef1 = VS_FALSE, fEMVCLDef2 = VS_FALSE;
	VS_BOOL fEMVCAPK1 = VS_FALSE;
	VS_BOOL fCFGT1 = VS_FALSE, fCFGT2 = VS_FALSE;
	VS_BOOL fCDT1 = VS_FALSE, fCDT2 = VS_FALSE, fCDT3 = VS_FALSE, fCDT4 = VS_FALSE;
	VS_BOOL fSCDT1 = VS_FALSE, fSCDT2 = VS_FALSE;
	VS_BOOL fIPASS = VS_FALSE;
	VS_BOOL fECC = VS_FALSE;
	VS_BOOL fICASH = VS_FALSE;

	if (ginDebug == VS_TRUE)
		inLogPrintf(AT, "inNCCCTMS_UpdateParam() START !!");

	inDISP_ClearAll();
	inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_); /* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_TMS_DOWNLOAD_TITLE_, 0, _COORDINATE_Y_LINE_8_3_); /* 第三層顯示 ＜TMS參數下載＞ */

	/* 一律清除 */
	inUtility_ClearFile();
	//	inUtility_OpenTraceLogFile();

	/* 先檢查是否有【APPLICATION】要更新【Y = 需要更新AP】 */
	inLoadTMSCPTRec(0);

	memset(szTMSAPPUpdateStatus, 0x00, sizeof (szTMSAPPUpdateStatus));
	inGetTMSAPPUpdateStatus(szTMSAPPUpdateStatus);
	if (memcmp(szTMSAPPUpdateStatus, "Y", strlen("Y")) == 0)
	{
		/* 程式更新中，請勿關機 */
		inDISP_PutGraphic(_TMS_AP_UPDATING_, 0, _COORDINATE_Y_LINE_8_4_);
		/* 等到參數轉換完才算完成 */
		inSetTMSOK("N");
		inSaveEDCRec(0);

		vdUtility_SYSFIN_LogMessage(AT, "AP Need Download TMSOK='N'");

		/* 重置紀錄TSAM已註冊的開關*/
		inSetTSAMRegisterEnable("N");
		inSaveEDCRec(0);

		/* 更新TMS，DCC狀態重置 */
		inSetDCCSettleDownload("0");
		inSaveEDCRec(0);

		inSetTMSAPPUpdateStatus("N");
		inSaveTMSCPTRec(0);

		/* 先行刪除APPL更新完成確認檔 */
		inFunc_Data_Delete("-r", "*", _APPL_UPDATE_CHECK_PATH_);

		inRetVal = inNCCCTMS_Process_AP();
		if (inRetVal == VS_SUCCESS)
		{
			/* 要先重新開機一次，不然更新參數檔會有問題 */
			uszUpdateFailBit = VS_FALSE;
		} else
		{
			uszUpdateFailBit = VS_TRUE;
		}

		if (uszUpdateFailBit == VS_TRUE)
		{
			vdUtility_SYSFIN_LogMessage(AT, "AP更新失敗");
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "AP更新失敗");
				inLogPrintf(AT, szDebugMsg);
			}
		} else
		{
			vdUtility_SYSFIN_LogMessage(AT, "AP更新完成");
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "AP更新完成");
				inLogPrintf(AT, szDebugMsg);
			}
			inFunc_Reboot();
		}
	} else
	{
		/* 參數更新中，請勿關機 */
		inDISP_PutGraphic(_TMS_UPDATING_, 0, _COORDINATE_Y_LINE_8_4_);
		/* 等到參數轉換完才算完成 */
		inSetTMSOK("N");
		inSaveEDCRec(0);

		vdUtility_SYSFIN_LogMessage(AT, "Need Parameter Update");

		/* 使用do...while(0)的原因是因為想若其中一步驟失敗，就不再繼續執行，但仍要關閉檔案及刪除檔案等處理 */
		do
		{
			if (pobTran->uszFTP_TMS_Download == _TMS_DOWNLOAD_SECURE_MODE_FTPS_ ||
				pobTran->uszFTP_TMS_Download == _TMS_DOWNLOAD_SECURE_MODE_SFTP_)
			{
				inRetVal = inFILE_Check_Exist((unsigned char *) _FTPFLT_FILE_NAME_);
				if (inRetVal == VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "FTPFLT Exist");
					}
					vdUtility_SYSFIN_LogMessage(AT, "FTPFLT Exist");
				} else
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "FTPFLT Not Exist");
					}
					vdUtility_SYSFIN_LogMessage(AT, "FTPFLT Not Exist");
				}
			} else
			{
				inRetVal = inFILE_Check_Exist((unsigned char *) _TMSFLT_FILE_NAME_);
				if (inRetVal == VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "TMSFLT Exist");
					}
					vdUtility_SYSFIN_LogMessage(AT, "TMSFLT Exist");
				} else
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "TMSFLT Not Exist");
					}
					vdUtility_SYSFIN_LogMessage(AT, "TMSFLT Not Exist");
				}
			}

			memset(szCustomerIndicator, 0x00, sizeof (szCustomerIndicator));
			inGetCustomIndicator(szCustomerIndicator);
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_) ||
				!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_026_TAKA_, _CUSTOMER_INDICATOR_SIZE_))
			{
				inTAKA_TAKA_Comm_Preserve();
			}

			/* 需要先判斷要用哪個檔案來轉換 Config2.txt EMVCLDef2.txt */
			for (i = 0;; i++)
			{
				if (pobTran->uszFTP_TMS_Download == _TMS_DOWNLOAD_SECURE_MODE_FTPS_ ||
					pobTran->uszFTP_TMS_Download == _TMS_DOWNLOAD_SECURE_MODE_SFTP_)
				{
					/* Load TMS File List 取得檔名 */
					if (inLoadFTPFLTRec(i) < 0)
						break;

					memset(uszFileName, 0x00, sizeof (uszFileName));
					inGetFTPFileName((char*) uszFileName);
				} else
				{
					/* Load TMS File List 取得檔名 */
					if (inLoadTMSFLTRec(i) < 0)
						break;

					memset(szTemplate, 0x00, sizeof (szTemplate));
					inGetTMSFilePathName(szTemplate);

					/* 取得檔案路徑後 抓取檔名儲存到FileName */
					for (j = 0; j < 60; j++)
					{
						if (szTemplate[j] == '/')
							inSlash = j + 1;

						if (szTemplate[j] == 0x00)
							break;
					}

					inFileNameLen = j - inSlash;
					memset(uszFileName, 0x00, sizeof (uszFileName));
					memcpy(&uszFileName[0], &szTemplate[inSlash], inFileNameLen);
				}

				if (!memcmp(&uszFileName[0], "EMVCLDef.txt", 12))
					fEMVCLDef1 = VS_TRUE;
				else if (!memcmp(&uszFileName[0], "EMVCLDef2.txt", 13))
					fEMVCLDef2 = VS_TRUE;
				else if (!memcmp(&uszFileName[0], "Config.txt", 10))
					fCFGT1 = VS_TRUE;
				else if (!memcmp(&uszFileName[0], "Config2.txt", 11))
					fCFGT2 = VS_TRUE;
				else if (!memcmp(&uszFileName[0], "CardDef.txt", 11))
					fCDT1 = VS_TRUE;
				else if (!memcmp(&uszFileName[0], "CardDef2.txt", 12))
					fCDT2 = VS_TRUE;
				else if (!memcmp(&uszFileName[0], "CardDef3.txt", 12))
					fCDT3 = VS_TRUE;
				else if (!memcmp(&uszFileName[0], "CardDef4.txt", 12))
					fCDT4 = VS_TRUE;
				else if (!memcmp(&uszFileName[0], "SpecCardDef.txt", 15))
					fSCDT1 = VS_TRUE;
				else if (!memcmp(&uszFileName[0], "SpecCardDef2.txt", 16))
					fSCDT2 = VS_TRUE;
				else if (!memcmp(&uszFileName[0], "EMVCAPK.txt", 11))
					fEMVCAPK1 = VS_TRUE;
				else if (!memcmp(&uszFileName[0], "EMVDef.txt", 10))
					fEMVDef1 = VS_TRUE;
			}

			if (fEMVCLDef2 == VS_TRUE && fEMVCLDef1 == VS_TRUE)
				fEMVCLDef1 = VS_FALSE;

			if (fCFGT2 == VS_TRUE && fCFGT1 == VS_TRUE)
				fCFGT1 = VS_FALSE;

			if (fCDT4 == VS_TRUE)
			{
				fCDT3 = VS_FALSE;
				fCDT2 = VS_FALSE;
				fCDT1 = VS_FALSE;
			} else if (fCDT3 == VS_TRUE)
			{
				fCDT2 = VS_FALSE;
				fCDT1 = VS_FALSE;
			} else if (fCDT2 == VS_TRUE)
			{
				fCDT1 = VS_FALSE;
			}


			if (fSCDT2 == VS_TRUE && fSCDT1 == VS_TRUE)
				fSCDT1 = VS_FALSE;

			/* 這裡檢核TMS是否沒給重要檔案 START */
			/* 只檢核是否沒有，不特別去擋 2019/12/25 下午 2:46 */
			if (fCFGT1 == VS_FALSE && fCFGT2 == VS_FALSE)
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "CFGT not exist");
				}
				inUtility_StoreTraceLog_OneStep("CFGT not exist");
			}

			if (fEMVCAPK1 == VS_FALSE)
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "CAPK not exist");
				}
				inUtility_StoreTraceLog_OneStep("CAPK not exist");
			}

			if (fEMVDef1 == VS_FALSE)
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "EMVDef not exist");
				}
				inUtility_StoreTraceLog_OneStep("EMVDef not exist");
			}

			if (fEMVCLDef1 == VS_FALSE && fEMVCLDef2 == VS_FALSE)
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "EMVCLDef2 not exist");
				}
				inUtility_StoreTraceLog_OneStep("EMVCLDef2 not exist");
			}

			/* 這裡檢核TMS是否沒給重要檔案 END */

			for (i = 0;; i++)
			{
				if (pobTran->uszFTP_TMS_Download == _TMS_DOWNLOAD_SECURE_MODE_FTPS_ ||
					pobTran->uszFTP_TMS_Download == _TMS_DOWNLOAD_SECURE_MODE_SFTP_)
				{
					/* Load TMS File List 取得檔名 */
					if (inLoadFTPFLTRec(i) < 0)
						break;

					memset(uszFileName, 0x00, sizeof (uszFileName));
					inGetFTPFileName((char *) uszFileName);
				} else
				{
					/* Load TMS File List 取得檔名 */
					if (inLoadTMSFLTRec(i) < 0)
						break;

					memset(szTemplate, 0x00, sizeof (szTemplate));
					inGetTMSFilePathName(szTemplate);

					/* 取得檔案路徑後 抓取檔名儲存到FileName */
					for (j = 0; j < 60; j++)
					{
						if (szTemplate[j] == '/')
							inSlash = j + 1;

						if (szTemplate[j] == 0x00)
							break;
					}

					inFileNameLen = j - inSlash;
					memset(uszFileName, 0x00, sizeof (uszFileName));
					memcpy(&uszFileName[0], &szTemplate[inSlash], inFileNameLen);
				}

				/* 先確認檔案是否存在 避免誤刪檔案 */
				if (inFILE_Check_Exist(uszFileName) != VS_SUCCESS)
				{
					memset(szTemplate, 0x00, sizeof (szTemplate));
					sprintf(szTemplate, "%s_ERROR", uszFileName);

					vdUtility_SYSFIN_LogMessage(AT, "%s_ERROR", uszFileName);
					if (inUtility_StoreTraceLog_OneStep(szTemplate) != VS_SUCCESS)
					{
						inUtility_CloseTraceLogFile();
						return (VS_ERROR);
					}

					continue;

				}

				inRecord = 0;

				/* 重新命名成我們使用的檔案名稱 */
				if (!memcmp(&uszFileName[0], "CardDef.txt", 11) && (fCDT1 == VS_TRUE))
				{
					/* 刪除TMS下載的檔案並改名 */
					inFILE_Delete((unsigned char *) _CDT_FILE_NAME_);
					inFILE_Rename(uszFileName, (unsigned char *) _CDT_FILE_NAME_);

					/* 計算有幾筆Record */
					for (j = 0;; j++)
					{
						if (inLoadCDTRec(j) < 0)
							break;

						inRecord++;
					}
				} else if (!memcmp(&uszFileName[0], "CardDef2.txt", 12) && (fCDT2 == VS_TRUE))
				{
					/* 刪除TMS下載的檔案並改名 */
					inFILE_Delete((unsigned char *) _CDT_FILE_NAME_);
					inFILE_Rename(uszFileName, (unsigned char *) _CDT_FILE_NAME_);

					/* 計算有幾筆Record */
					for (j = 0;; j++)
					{
						if (inLoadCDTRec(j) < 0)
							break;

						inRecord++;
					}
				} else if (!memcmp(&uszFileName[0], "CardDef3.txt", 12) && (fCDT3 == VS_TRUE))
				{
					/* 刪除TMS下載的檔案並改名 */
					inFILE_Delete((unsigned char *) _CDT_FILE_NAME_);
					inFILE_Rename(uszFileName, (unsigned char *) _CDT_FILE_NAME_);

					/* 計算有幾筆Record */
					for (j = 0;; j++)
					{
						if (inLoadCDTRec(j) < 0)
							break;

						inRecord++;
					}
				} else if (!memcmp(&uszFileName[0], "CardDef4.txt", 12) && (fCDT4 == VS_TRUE))
				{
					/* 刪除TMS下載的檔案並改名 */
					inFILE_Delete((unsigned char *) _CDT_FILE_NAME_);
					inFILE_Rename(uszFileName, (unsigned char *) _CDT_FILE_NAME_);

					/* 計算有幾筆Record */
					for (j = 0;; j++)
					{
						if (inLoadCDTRec(j) < 0)
							break;

						inRecord++;
					}
				} else if ((!memcmp(&uszFileName[0], "Config.txt", 10)) && (fCFGT1 == VS_TRUE))
				{
					inFILE_Delete((unsigned char *) _CFGT_FILE_NAME_);
					inFILE_Rename(uszFileName, (unsigned char *) _CFGT_FILE_NAME_);

					/* 計算有幾筆Record */
					for (j = 0;; j++)
					{
						if (inLoadCFGTRec(j) < 0)
							break;

						inRecord++;
					}
				} else if ((!memcmp(&uszFileName[0], "Config2.txt", 11)) && (fCFGT2 == VS_TRUE))
				{
					inFILE_Delete((unsigned char *) _CFGT_FILE_NAME_);
					inFILE_Rename(uszFileName, (unsigned char *) _CFGT_FILE_NAME_);

					/* 計算有幾筆Record */
					for (j = 0;; j++)
					{
						if (inLoadCFGTRec(j) < 0)
							break;

						inRecord++;
					}
				} else if (!memcmp(&uszFileName[0], "SysConfig.txt", 13))
				{
					inFILE_Delete((unsigned char *) _TMSSCT_FILE_NAME_);
					inFILE_Rename(uszFileName, (unsigned char *) _TMSSCT_FILE_NAME_);

					/* 計算有幾筆Record */
					for (j = 0;; j++)
					{
						if (inLoadTMSSCTRec(j) < 0)
							break;

						inRecord++;
					}
				}
				else if (!memcmp(&uszFileName[0], "PWDef.txt", 9))
				{
					inFILE_Delete((unsigned char *) _PWD_FILE_NAME_);
					inFILE_Rename(uszFileName, (unsigned char *) _PWD_FILE_NAME_);

					/* 計算有幾筆Record */
					for (j = 0;; j++)
					{
						if (inLoadPWDRec(j) < 0)
							break;

						inRecord++;
					}
				}
				else if (!memcmp(&uszFileName[0], "HostDef.txt", 11))
				{

					inFILE_Delete((unsigned char *) _HDT_FILE_NAME_);
					inFILE_Rename(uszFileName, (unsigned char *) _HDT_FILE_NAME_);

					/* 計算有幾筆Record */
					for (j = 0;; j++)
					{
						if (inLoadHDTRec(j) < 0)
							break;

						inRecord++;
					}
				} else if (!memcmp(&uszFileName[0], "CommDef.txt", 11))
				{
					/* 備份CPT參數 */
					if (inNCCCTMS_Backup_CPT_Parameter() != VS_SUCCESS)
					{
						if (ginDebug == VS_TRUE)
						{
							inLogPrintf(AT, "CPT Backup Fail");
						}
					}

					inFILE_Delete((unsigned char *) _CPT_FILE_NAME_);
					inFILE_Rename(uszFileName, (unsigned char *) _CPT_FILE_NAME_);

					/* 計算有幾筆Record */
					for (j = 0;; j++)
					{
						if (inLoadCPTRec(j) < 0)
							break;

						inRecord++;
					}
				}
				else if (!memcmp(&uszFileName[0], "EMVDef.txt", 10))
				{
					inFILE_Delete((unsigned char *) _MVT_FILE_NAME_);
					inFILE_Rename(uszFileName, (unsigned char *) _MVT_FILE_NAME_);

					/* 同步MCCCode */
					inNCCCTMS_Sync_MCCCode();

					/* 計算有幾筆Record */
					for (j = 0;; j++)
					{
						if (inLoadMVTRec(j) < 0)
							break;

						inRecord++;
					}
				}
				else if ((!memcmp(&uszFileName[0], "EMVCLDef.txt", 12)) && (fEMVCLDef1 == VS_TRUE))
				{
					inFILE_Delete((unsigned char *) _VWT_FILE_NAME_);
					inFILE_Rename(uszFileName, (unsigned char *) _VWT_FILE_NAME_);

					/* 計算有幾筆Record */
					for (j = 0;; j++)
					{
						if (inLoadVWTRec(j) < 0)
							break;

						inRecord++;
					}
				}
				else if ((!memcmp(&uszFileName[0], "EMVCLDef2.txt", 13)) && (fEMVCLDef2 == VS_TRUE))
				{
					inFILE_Delete((unsigned char *) _VWT_FILE_NAME_);
					inFILE_Rename(uszFileName, (unsigned char *) _VWT_FILE_NAME_);

					/* 計算有幾筆Record */
					for (j = 0;; j++)
					{
						if (inLoadVWTRec(j) < 0)
							break;

						inRecord++;
					}
				} else if (!memcmp(&uszFileName[0], "EMVCAPK.txt", 11))
				{
					inFILE_Delete((unsigned char *) _EST_FILE_NAME_);
					inFILE_Rename(uszFileName, (unsigned char *) _EST_FILE_NAME_);

					/* 計算有幾筆Record */
					for (j = 0;; j++)
					{
						if (inLoadESTRec(j) < 0)
							break;

						inRecord++;
					}
				}
				else if (!memcmp(&uszFileName[0], "SKMDef.txt", 10))
				{
					inFILE_Delete((unsigned char *) _SKM_FILE_NAME_);
					inFILE_Rename(uszFileName, (unsigned char *) _SKM_FILE_NAME_);

					/* 計算有幾筆Record */
					for (j = 0;; j++)
					{
						if (inLoadSKMRec(j) < 0)
							break;

						inRecord++;
					}
				} else if (!memcmp(&uszFileName[0], "ASMConfig.txt", 13))
				{
					inFILE_Delete((unsigned char *) _ASMC_FILE_NAME_);
					inFILE_Rename(uszFileName, (unsigned char *) _ASMC_FILE_NAME_);

					/* 計算有幾筆Record */
					for (j = 0;; j++)
					{
						if (inLoadASMCRec(j) < 0)
							break;

						inRecord++;
					}
				} else if (!memcmp(&uszFileName[0], "SpecCardDef.txt", 15) && fSCDT1 == VS_TRUE)
				{
					inFILE_Delete((unsigned char *) _SCDT_FILE_NAME_);
					inFILE_Rename(uszFileName, (unsigned char *) _SCDT_FILE_NAME_);

					/* 計算有幾筆Record */
					for (j = 0;; j++)
					{
						if (inLoadASMCRec(j) < 0)
							break;

						inRecord++;
					}
				} else if (!memcmp(&uszFileName[0], "SpecCardDef2.txt", 16) && fSCDT2 == VS_TRUE)
				{
					inFILE_Delete((unsigned char *) _SCDT_FILE_NAME_);
					inFILE_Rename(uszFileName, (unsigned char *) _SCDT_FILE_NAME_);

					/* 計算有幾筆Record */
					for (j = 0;; j++)
					{
						if (inLoadASMCRec(j) < 0)
							break;

						inRecord++;
					}
				} else if (!memcmp(&uszFileName[0], "PCodeDef.txt", 12))
				{
					inFILE_Delete((unsigned char *) _PCD_FILE_NAME_);
					inFILE_Rename(uszFileName, (unsigned char *) _PCD_FILE_NAME_);

					/* 計算有幾筆Record */
					for (j = 0;; j++)
					{
						if (inLoadASMCRec(j) < 0)
							break;

						inRecord++;
					}
				} else if (!memcmp(&uszFileName[0], "PayItem.txt", 11))
				{
					inFILE_Delete((unsigned char *) _PIT_FILE_NAME_);
					inFILE_Rename(uszFileName, (unsigned char *) _PIT_FILE_NAME_);

					/* 計算有幾筆Record */
					for (j = 0;; j++)
					{
						if (inLoadASMCRec(j) < 0)
							break;

						inRecord++;
					}
				}					/* QuickPay已廢除 */
				else if (!memcmp(&uszFileName[0], "QuickPay.txt", 12))
				{
					/* 端末機開發此功能(Table NQ)時，需一併移除原上傳Table ID “NI”及TMS QuickPay.txt等~~ 原端末機判斷VEPS免簽名之邏輯。*/
					inFILE_Delete((unsigned char *) "QuickPay.dat");
					continue;
				} else if (!memcmp(&uszFileName[0], "SPDef.txt", 9))
				{
					inFILE_Delete((unsigned char *) _SPAY_FILE_NAME_);
					inFILE_Rename(uszFileName, (unsigned char *) _SPAY_FILE_NAME_);

					/* 計算有幾筆Record */
					for (j = 0;; j++)
					{
						if (inLoadASMCRec(j) < 0)
							break;

						inRecord++;
					}
				} else if (!memcmp(&uszFileName[0], "IPASSDef.txt", 12))
				{
					inFILE_Delete((unsigned char *) _IPASSDT_FILE_NAME_);
					inFILE_Rename(uszFileName, (unsigned char *) _IPASSDT_FILE_NAME_);

					/* 計算有幾筆Record */
					for (j = 0;; j++)
					{
						if (inLoadIPASSDTRec(j) < 0)
							break;

						inRecord++;
					}
					fIPASS = VS_TRUE;
				} else if (!memcmp(&uszFileName[0], "ECCDef.txt", 10))
				{
					inFILE_Delete((unsigned char *) _ECCDT_FILE_NAME_);
					inFILE_Rename(uszFileName, (unsigned char *) _ECCDT_FILE_NAME_);

					/* 計算有幾筆Record */
					for (j = 0;; j++)
					{
						if (inLoadECCDTRec(j) < 0)
							break;

						inRecord++;
					}
					fECC = VS_TRUE;
				} else if (!memcmp(&uszFileName[0], "ICASHDef.txt", 12))
				{
					inFILE_Delete((unsigned char *) _ICASHDT_FILE_NAME_);
					inFILE_Rename(uszFileName, (unsigned char *) _ICASHDT_FILE_NAME_);

					/* 計算有幾筆Record */
					for (j = 0;; j++)
					{
						if (inLoadICASHDTRec(j) < 0)
							break;

						inRecord++;
					}
					fICASH = VS_TRUE;
				} else if (!memcmp(&uszFileName[0], "NcccLogo.bmp", 12))
				{
					inFILE_Delete((unsigned char *) "NCCCLOGO.bmp");
					inFILE_Rename(uszFileName, (unsigned char *) "NCCCLOGO.bmp");
				}
				else if (!memcmp(&uszFileName[0], "BmpName.bmp", 11))
				{
					inFILE_Delete((unsigned char *) "BMPNAME.bmp");
					inFILE_Rename(uszFileName, (unsigned char *) "BMPNAME.bmp");
				} else if (!memcmp(&uszFileName[0], "Big5Name.txt", 12))
				{
					inFILE_Delete((unsigned char *) _Big5NameT_FILE_NAME_);
					inFILE_Rename(uszFileName, (unsigned char *) _Big5NameT_FILE_NAME_);
					for (k = 0;; k++)
					{
						if (inLoadBig5NameTRec(k) != VS_SUCCESS)
							break;

						memset(szTemplate, 0x00, sizeof (szTemplate));
						inGetMerchant_Name(szTemplate);
						memset(szTemplate2, 0x00, sizeof (szTemplate2));
						inFunc_Big5toUTF8(szTemplate2, szTemplate);

						inSetMerchant_Name(szTemplate2);
						inSaveBig5NameTRec(k);
					}
				} else if (!memcmp(&uszFileName[0], "BmpLogo.bmp", 11))
				{
					inFILE_Delete((unsigned char *) "BMPLOGO.bmp");
					inFILE_Rename(uszFileName, (unsigned char *) "BMPLOGO.bmp");
				} else if (!memcmp(&uszFileName[0], "BmpNotice.bmp", 13))
				{
					inFILE_Delete((unsigned char *) "BMPNOTICE.bmp");
					inFILE_Rename(uszFileName, (unsigned char *) "BMPNOTICE.bmp");
				} else if (!memcmp(&uszFileName[0], "BmpLegal.bmp", 12))
				{
					inFILE_Delete((unsigned char *) "BMPLEGAL.bmp");
					inFILE_Rename(uszFileName, (unsigned char *) "BMPLEGAL.bmp");
				} else if (!memcmp(&uszFileName[0], "BmpSlogan.bmp", 13))
				{
					inFILE_Delete((unsigned char *) "BMPSLOGAN.bmp");
					inFILE_Rename(uszFileName, (unsigned char *) "BMPSLOGAN.bmp");
				} else if (!memcmp(&uszFileName[0], "CUPLegal.bmp", 12))
				{
					inFILE_Delete((unsigned char *) "CUPLEGAL.bmp");
					inFILE_Rename(uszFileName, (unsigned char *) "CUPLEGAL.bmp");
				} else if (!memcmp(&uszFileName[0], "QA_I.txt", 8))
				{
					inFILE_Delete((unsigned char *) "QA_I.dat");
					inFILE_Rename(uszFileName, (unsigned char *) "QA_I.dat");
				} else if (!memcmp(&uszFileName[0], "QA_M.txt", 8))
				{
					inFILE_Delete((unsigned char *) "QA_M.dat");
					inFILE_Rename(uszFileName, (unsigned char *) "QA_M.dat");
				} else if (!memcmp(&uszFileName[0], "QA_S.txt", 8))
				{
					inFILE_Delete((unsigned char *) "QA_S.dat");
					inFILE_Rename(uszFileName, (unsigned char *) "QA_S.dat");
				} else if (!memcmp(&uszFileName[0], _TMSIPDT_ORG_TMS_FILE_NAME_, _TMSIPDT_ORG_TMS_FILE_NAME_LEN_))
				{
					inFILE_Delete((unsigned char *) _TMSIPDT_FILE_NAME_);
					inFILE_Rename(uszFileName, (unsigned char *) _TMSIPDT_FILE_NAME_);
				} else if (!memcmp(&uszFileName[0], _CER_UCA_FILE_NAME_, _CER_UCA_FILE_NAME_LEN_))
				{

				} else if (!memcmp(&uszFileName[0], _CER_ROOT_FILE_NAME_, _CER_ROOT_FILE_NAME_LEN_))
				{

				} else if (!memcmp(&uszFileName[0], _CER_UCA2_FILE_NAME_, _CER_UCA2_FILE_NAME_LEN_))
				{
					/*[260206][114215-TWCA憑證升級] 加入UCA2的檔案下載條件 */
				} else
				{
					inFILE_Delete(uszFileName);
				}

				memset(szTemplate, 0x00, sizeof (szTemplate));
				sprintf(szTemplate, "%s_SUCCESS_[%d]", uszFileName, inRecord);

				vdUtility_SYSFIN_LogMessage(AT, "%s_SUCCESS_[%d]", uszFileName, inRecord);
				if (inUtility_StoreTraceLog_OneStep(szTemplate) != VS_SUCCESS)
				{
					inUtility_CloseTraceLogFile();
					return (VS_ERROR);
				}
			}

			/* 確認TMS參數(防呆) */
			inRetVal = inNCCC_Func_Check_TMS_Setting_Compatible(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				uszUpdateFailBit = VS_TRUE;
				vdUtility_SYSFIN_LogMessage(AT, "TMS_Setting_Compatible Failed");
				break;
			}

			/* 初始化HDPT */
			inRetVal = inNCCCTMS_Initial_HDPT();
			if (inRetVal != VS_SUCCESS)
			{
				uszUpdateFailBit = VS_TRUE;
				vdUtility_SYSFIN_LogMessage(AT, "Initial_HDPT Failed");
				break;
			}

			/* 建立參數的資料夾 */
			inRetVal = inFunc_Dir_Make(_EMV_EMVCL_DIR_NAME_, _FS_DATA_PATH_);
			if (inRetVal != VS_SUCCESS)
			{
				uszUpdateFailBit = VS_TRUE;
				vdUtility_SYSFIN_LogMessage(AT, "Dir_Make_EMV_EMVCL_DIR_NAME_ Failed");
				break;
			}

			/* 更新EMV參數 */
			inRetVal = inEMVXML_Update_EMV_XML(_EMV_CONFIG_FILENAME_, _EMV_EMVCL_DATA_PATH_);
			if (inRetVal != VS_SUCCESS)
			{
				uszUpdateFailBit = VS_TRUE;
				vdUtility_SYSFIN_LogMessage(AT, "Update_EMV_XML_EMV_CONFIG_FILENAME_ Failed");
				break;
			}

			/* 更新CTLS感應參數 */
			inRetVal = inEMVXML_Update_CTLS_XML(_EMVCL_CONFIG_FILENAME_, _EMV_EMVCL_DATA_PATH_);
			if (inRetVal != VS_SUCCESS)
			{
				uszUpdateFailBit = VS_TRUE;
				vdUtility_SYSFIN_LogMessage(AT, "Update_CTLS_XML_EMVCL_CONFIG_FILENAME_ Failed");
				break;
			}

			/* 初始化TDT */
			inRetVal = inNCCCTMS_Initial_Ticket();
			if (inRetVal != VS_SUCCESS)
			{
				uszUpdateFailBit = VS_TRUE;
				vdUtility_SYSFIN_LogMessage(AT, "inNCCCTMS_Initial_Ticket Failed");
				break;
			}

			/* 初始化票證參數 */
			if (fIPASS == VS_TRUE)
			{
				inRetVal = inNCCCTMS_Initial_IPASS();
				if (inRetVal != VS_SUCCESS)
				{
					uszUpdateFailBit = VS_TRUE;
					vdUtility_SYSFIN_LogMessage(AT, "inNCCCTMS_Initial_IPASS Failed");
					break;
				}
			}

			if (fECC == VS_TRUE)
			{
				inRetVal = inNCCCTMS_Initial_ECC();
				if (inRetVal != VS_SUCCESS)
				{
					uszUpdateFailBit = VS_TRUE;
					vdUtility_SYSFIN_LogMessage(AT, "inNCCCTMS_Initial_ECC Failed");
					break;
				}
			}

			if (fICASH == VS_TRUE)
			{
				inRetVal = inNCCCTMS_Initial_ICASH();
				if (inRetVal != VS_SUCCESS)
				{
					uszUpdateFailBit = VS_TRUE;
					vdUtility_SYSFIN_LogMessage(AT, "inNCCCTMS_Initial_ICASH Failed");
					break;
				}
			}

			/* 若TMS下載參數，交易環境為"M-FES"，CommDef.txt IP帶空值，不需更新端末機IP資訊，保留手動輸入之IP資訊。 */
			memset(szTemplate, 0x00, sizeof (szTemplate));
			inGetNCCCFESMode(szTemplate);
			if (memcmp(szTemplate, _NCCC_03_MFES_MODE_, strlen(_NCCC_03_MFES_MODE_)) == 0)
			{
				inNCCCTMS_Recover_CPT_Parameter();
			}

			/* CFES設定防呆 */
			inRetVal = inNCCC_Func_CFES_Poka_Yoke_Setting();
			if (inRetVal != VS_SUCCESS)
			{
				uszUpdateFailBit = VS_TRUE;
				vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Func_CFES_Poka_Yoke_Setting Failed");
				break;
			}

			/* 客製化參數設定 */
			inRetVal = inNCCC_Func_Customer_Setting(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				uszUpdateFailBit = VS_TRUE;
				vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Func_Customer_Setting Failed");
				break;
			}

			/* DEMO參數設定 */
			inRetVal = inNCCC_Func_Demo_TMS_Setting(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				uszUpdateFailBit = VS_TRUE;
				vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Func_Demo_TMS_Setting Failed");
				break;
			}

			/* 【需求單 - 108097】Card BIN由ATS查核 add by LingHsiung 2020/2/3 下午 5:19 */
			/* 自定義CDT參數 */
			inLoadCFGTRec(0);
			memset(szTemplate, 0x00, sizeof (szTemplate));
			inGetBIN_CHECK(szTemplate);
			/* 由ATS主機檢核 */
			if (memcmp(szTemplate, "A", strlen("A")) == 0)
			{
				inRetVal = inNCCCTMS_CDT_Customize();
				if (inRetVal != VS_SUCCESS)
				{
					uszUpdateFailBit = VS_TRUE;
					vdUtility_SYSFIN_LogMessage(AT, "inNCCCTMS_CDT_Customize Failed");
					break;
				}

				inRetVal = inNCCCTMS_CDTX_initial();
				if (inRetVal != VS_SUCCESS)
				{
					uszUpdateFailBit = VS_TRUE;
					vdUtility_SYSFIN_LogMessage(AT, "uszUpdateFailBit Failed");
					break;
				}
			}

			/* 補憑證 */
			inNCCC_Func_Update_PEM_Before_TMS_Manual_Download(pobTran);

			/* 更新憑證 */
			inNCCC_Func_Update_PEM_After_Para_Update();

			break;
		} while (1);
		/* 使用do...while(0)的原因是因為想若其中一步驟失敗，就不再繼續執行，但仍要關閉檔案及刪除檔案等處理 */

		/* TraceLog儲存完關閉 */
		inUtility_CloseTraceLogFile();

		/* 更新完成刪除File List */
		if (pobTran->uszFTP_TMS_Download == _TMS_DOWNLOAD_SECURE_MODE_FTPS_ ||
			pobTran->uszFTP_TMS_Download == _TMS_DOWNLOAD_SECURE_MODE_SFTP_)
		{
			/* 以FTP方式下載參數，更新成功要更新參數時間 */
			if (uszUpdateFailBit != VS_TRUE)
			{
				inNCCCTMS_FTPS_Update_TermParaTime();
			}

			inFILE_Delete((unsigned char *) _FTPFLT_FILE_NAME_);
			/* 刪除TermInfo2 */
			inFILE_Delete((unsigned char *) _FTP_INFO_2_);
		} else
		{
			inFILE_Delete((unsigned char *) _TMSFLT_FILE_NAME_);
		}

		if (uszUpdateFailBit == VS_TRUE)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Parameter更新失敗");
				inLogPrintf(AT, szDebugMsg);
			}

			vdUtility_SYSFIN_LogMessage(AT, "更新失敗");
		} else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Parameter更新完成");
				inLogPrintf(AT, szDebugMsg);
			}
			/* TMS下載計數+1 */
			inEDC_TMSDownloadTimes_Increase();

			vdUtility_SYSFIN_LogMessage(AT, "更新成功");

			inFunc_SysFin_Log_Ls();

			/* [114197] 中心收單作業ESG永續發展優化 
			 * [20260519] 新增ESG轉圖檔要用的流程 MergeAndScaleMultipleBMPUniversal()
			 */
			/* 先刪除已存在的圖片,路徑要使用原廠的方法 */
			inFILE_Delete((unsigned char*)_ESG_CUS_LOGO_NAME_);
			/* [114197] 中心收單作業ESG永續發展優化 
			*  [260610] 反映電子簽單顯示上不用包含商店Logo */
			/* 合併列印時的圖片 */
			MergeBMPItem listToMerge[] = {
				{_NAME_LOGO_, 320, 0}	
			};

			int fileCount = sizeof (listToMerge) / sizeof (listToMerge[0]);

			if (inUtility_SYSFIN_MergeAndScaleMultipleBMPUniversal(listToMerge, fileCount, _ESG_CUS_LOGO_PATH_AND_NAME_, 1) != VS_SUCCESS)
			{
				vdUtility_SYSFIN_LogMessage(AT, "ESG 合併圖檔案失敗");
			}


		}
	}

	if (ginDebug == VS_TRUE)
		inLogPrintf(AT, "inNCCCTMS_UpdateParam() END !!");

	if (uszUpdateFailBit == VS_TRUE)
	{
		return (VS_ERROR);
	} else
	{
		return (VS_SUCCESS);
	}
}

/*
Function        :inNCCCTMS_Func6TMSDownload
Date&Time       :2016/1/22 上午 10:32
Describe        :功能六 TMS下載
*/
int inNCCCTMS_Func6TMSDownload(TRANSACTION_OBJECT *pobTran)
{
        int	inRetVal = VS_ERROR;
	char	szTemplate[2 + 1] = {0};
	char	szCFESMode[2 + 1] = {0};
	char	szNCCCFESMode[2 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
        
        vdUtility_SYSFIN_LogMessage(AT, "inNCCCTMS_Func6TMSDownload START!");
	
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCCTMS_Func6TMSDownload() START !!");
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
        
        inDISP_ClearAll();
        inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_TMS_DOWNLOAD_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜TMS參數下載＞ */
        
        /* 先LOAD NCCC HOST */
        inLoadHDTRec(0);
	inLoadHDPTRec(0);
        /* TMS的CPT參數 */
        inLoadTMSCPTRec(0);
        /* EDC參數 */
        inLoadEDCRec(0);
        /* CPT參數 */
        inLoadCPTRec(0);
        
        inRetVal = inNCCCTMS_CheckTMSOK(pobTran);
        
        if (inRetVal != VS_SUCCESS)
        {
                /* 請執行參數下載 */
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, _ERR_TMS_DWL_);
		srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
		srDispMsgObj.inMsgType = _NO_KEY_MSG_;
		srDispMsgObj.inTimeout = 0;
		strcpy(srDispMsgObj.szErrMsg1, "");
		srDispMsgObj.inErrMsg1Line = 0;
		srDispMsgObj.inBeepTimes = 3;
		srDispMsgObj.inBeepInterval = 1000;
			
                inDISP_Msg_BMP(&srDispMsgObj);
		
                return (VS_ERROR);
        }
        
        pobTran->inTransactionCode = _NCCCTMS_LOGON_; /* 先Hard code */
        pobTran->inTMSDwdMode = _TMS_MANUAL_DOWNLOAD_; /* 手動下載 */
	
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTMSDownloadMode(szTemplate);
	memset(szCFESMode, 0x00, sizeof(szCFESMode));
	inGetCloud_MFES(szCFESMode);
	/* FES Mode */
	memset(szNCCCFESMode, 0x00, sizeof(szNCCCFESMode));
	inGetNCCCFESMode(szNCCCFESMode);
			
	if (memcmp(szTemplate, _TMS_DLMODE_FTPS_, strlen(_TMS_DLMODE_FTPS_)) == 0)
	{
		if (memcmp(szCFESMode, "Y", strlen("Y")) == 0		&& 
		    memcmp(szNCCCFESMode, _NCCC_03_MFES_MODE_, strlen(_NCCC_03_MFES_MODE_)) == 0)
		{
			pobTran->uszFTP_TMS_Download = _TMS_DOWNLOAD_SECURE_MODE_SFTP_;
		}
		else
		{
			pobTran->uszFTP_TMS_Download = _TMS_DOWNLOAD_SECURE_MODE_FTPS_;
		}
	}
	else if (memcmp(szTemplate, _TMS_DLMODE_ISO8583_, strlen(_TMS_DLMODE_ISO8583_)) == 0)
	{
		pobTran->uszFTP_TMS_Download = _TMS_DOWNLOAD_SECURE_MODE_NONE_;
	}
	else
	{
		return (VS_ERROR);
	}
        
        /* 初始化Field 58 */
        inNCCCTMS_Field58_Initial(pobTran);
	
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_042_BDAU1_, _CUSTOMER_INDICATOR_SIZE_) ||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_043_BDAU9_, _CUSTOMER_INDICATOR_SIZE_))
	{
		if (inFLOW_RunFunction(pobTran, _NCCC_FUNCTION_BDAU_DELETE_BATCH_BEFORE_TMS_) != VS_SUCCESS)
			return (VS_ERROR);
	}
        
        /* 輸入下載管理號碼 */
        if (inNCCCTMS_Download_PWD_GET(pobTran) != VS_SUCCESS)
                return (VS_ERROR);

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCCTMS_Func6TMSDownload() END !!");
        
        return (VS_SUCCESS);
}

/*
Function        :inNCCCTMS_CheckTMSOK
Date&Time       :2016/1/6 下午 2:27
Describe        :確認TMSOK
*/
int inNCCCTMS_CheckTMSOK(TRANSACTION_OBJECT *pobTran)
{
        char szTMSOK[2 + 1];

        memset(szTMSOK, 0x00, sizeof(szTMSOK));
        
        inGetTMSOK(szTMSOK);
        
        if (!memcmp(&szTMSOK[0], "Y", 1))
                return (VS_SUCCESS);
        else
                return (VS_ERROR);
}

/*
Function        :inNCCCTMS_Schedule_Effective_Date_Time_Check
Date&Time       :2016/2/1 上午 9:57
Describe        :參數生效日期時間確認
*/
int inNCCCTMS_Schedule_Effective_Date_Time_Check(TRANSACTION_OBJECT * pobTran)
{
        int		inSecond = 0, inEffectiveSecond = 0;
        char		szDate[8 + 1], szTime[6 + 1];
        char		szEffectiveDate[8 + 1], szEffectiveTime[6 + 1];
        char		szTemplate[16 + 1];
        RTC_NEXSYS	srRTC;
        VS_BOOL		fDateCheck = VS_FALSE, fTimeCheck = VS_FALSE;

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCCTMS_Schedule_Effective_Date_Time_Check() START !!");
        
	if (inNCCCTMS_Check_FileList_Flow(pobTran) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "FileList不存在 不需要更新參數");
                
                return (VS_ERROR);
	}
        
        if (inLoadTMSCPTRec(0) < 0)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inLoadTMSCPTRec(0) Error !!");
        }
        
        memset(szEffectiveDate, 0x00, sizeof(szEffectiveDate));
        memset(szEffectiveTime, 0x00, sizeof(szEffectiveTime));
        
        /* 取得參數生效日期 */
        if (inGetTMSEffectiveDate(szEffectiveDate) < 0)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "參數生效日期取得失敗");
                
                return (VS_ERROR);
        }
        
        /* 參數立即生效 */
        if (!memcmp(&szEffectiveDate[0], "00000000", 8))
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "參數立即生效");
                
                return (VS_SUCCESS);
        }
        
        /* 取得參數生效時間 */
        if (inGetTMSEffectiveTime(szEffectiveTime) < 0)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "參數生效時間取得失敗");
                
                return (VS_ERROR);
        }
        
        if (inFunc_GetSystemDateAndTime(&srRTC) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
        memset(szDate, 0x00, sizeof(szDate));
        sprintf(szDate, "20%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
        memset(szTime, 0x00, sizeof(szTime));
        sprintf(szTime, "%02d%02d%02d", srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
        
        /* 端末機秒數 */
        inSecond = ((srRTC.uszHour) * 3600) + ((srRTC.uszMinute) * 60) + srRTC.uszSecond;
        
        /* 生效時間秒數 */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        memcpy(&szTemplate[0], &szEffectiveTime[0], 2);
        inEffectiveSecond = atoi(szTemplate) * 3600;
        memset(szTemplate, 0x00, sizeof(szTemplate));
        memcpy(&szTemplate[0], &szEffectiveTime[2], 2);
        inEffectiveSecond = inEffectiveSecond + (atoi(szTemplate) * 60);
        memset(szTemplate, 0x00, sizeof(szTemplate));
        memcpy(&szTemplate[0], &szEffectiveTime[4], 2);
        inEffectiveSecond = inEffectiveSecond + atoi(szTemplate);
        
        if (!memcmp(&szDate[0], &szEffectiveDate[0], 8))
        {
                fDateCheck = VS_TRUE;
        }
        
        /* 端末機時間大於生效時間 */
        if (inSecond >= inEffectiveSecond)
        {
		/* 小於生效時間3分鐘內，一直跳(因為Idle每五秒檢查一次，所以這裡每五秒跳一次) */
		if ((inSecond - inEffectiveSecond) < 60 * 3)
		{
			fTimeCheck = VS_TRUE;
		}
		/* 大於生效時間每2分鐘跳一次 */
		else
		{
			if ((srRTC.uszMinute % 2 == 0) && (srRTC.uszSecond == 0))
			{
				fTimeCheck = VS_TRUE;
			}
		}
		
        }
        
        if (fDateCheck == VS_TRUE && fTimeCheck == VS_TRUE)
                return (VS_SUCCESS);
        else
                return (VS_ERROR);
}

/*
Function        :inNCCCTMS_Schedule_Inquire_Date_Time_Check
Date&Time       :2017/1/25 下午 2:49
Describe        :詢問時間檢查
*/
int inNCCCTMS_Schedule_Inquire_Date_Time_Check()
{
        int		inScheduleDay, inTermDay, inGapDay;
        int		inScheduleMinute, inTermMinute;
        char		szTMSInquireStartDate[8 + 1], szTMSInquireTime[6 + 1], szTMSInquireGap[2 + 1];
        char		szTermDate[8 + 1], szTermTime[6 + 1], szHour[2 + 1], szMinute[2 + 1];
        VS_BOOL		fDate = VS_FALSE, fTime = VS_FALSE;
        RTC_NEXSYS	srRTC; /* Date & Time */

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCCTMS_Schedule_Date_Time_Check() START!");
        
        /* 取得EDC時間日期 */
        if (inFunc_GetSystemDateAndTime(&srRTC) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
        sprintf(szTermDate, "20%02d%02d%02d",  srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
        sprintf(szTermTime, "%02d%02d%02d",  srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);

        /* 參數詢問生效日 */
        memset(szTMSInquireStartDate, 0x00, sizeof(szTMSInquireStartDate));
        inGetTMSInquireStartDate(szTMSInquireStartDate);
        memset(szTMSInquireGap, 0x00, sizeof(szTMSInquireGap));
        inGetTMSInquireGap(szTMSInquireGap);
        inGapDay = atoi(szTMSInquireGap);
        
        if (!memcmp(&szTMSInquireStartDate[0], "00000000", 8))
                return (VS_ERROR);
        else
        {
                /* 太陽日運算 */
                inTermDay = inFunc_SunDay_Sum(szTermDate);
                inScheduleDay = inFunc_SunDay_Sum(szTMSInquireStartDate);
                
                if (inTermDay == inScheduleDay)
                {
                        fDate = VS_TRUE;
                }
                else if ((inTermDay > inScheduleDay) && (inGapDay > 0))
                {
                        if(((inTermDay - inScheduleDay) % inGapDay) == 0)
                        {
                                fDate = VS_TRUE;
                        }
                }
                else
                {
                        fDate = VS_FALSE;
                }
                
                /* 參數詢問時間 */
                memset(szTMSInquireTime, 0x00, sizeof(szTMSInquireTime));
                inGetTMSInquireTime(szTMSInquireTime);
                
                if (!memcmp(&szTMSInquireTime[0], "000000", 6))
                        return (VS_ERROR);
                else
                {
                        memset(szHour, 0x00, sizeof(szHour));
                        memcpy(&szHour[0], &szTMSInquireTime[0], 2);
                        memset(szMinute, 0x00, sizeof(szMinute));
                        memcpy(&szMinute[0], &szTMSInquireTime[2], 2);
                        inScheduleMinute = atoi(szHour) * 60 + atoi(szMinute);
                    
                        memset(szHour, 0x00, sizeof(szHour));
                        memcpy(&szHour[0], &szTermTime[0], 2);
                        memset(szMinute, 0x00, sizeof(szMinute));
                        memcpy(&szMinute[0], &szTermTime[2], 2);
                        inTermMinute = atoi(szHour) * 60 + atoi(szMinute);
                        
                        if ((inTermMinute >= inScheduleMinute) && ((inTermMinute - inScheduleMinute) <= 15))
                        {
                                fTime = VS_TRUE;
                        }
                        else
                        {
                                fTime = VS_FALSE;
                        }
                }
                
                if ((fDate == VS_TRUE) && (fTime == VS_TRUE))
                        return (VS_SUCCESS);
                else
                        return (VS_ERROR);
        }
}

/*
Function        :inNCCCTMS_Schedule_Download_Date_Time_Check
Date&Time       :2017/1/25 下午 1:20
Describe        :參考Verifone機型inNCCC_TMS_CheckSchedule()，查看是否已到排程時間
*/
int inNCCCTMS_Schedule_Download_Date_Time_Check(TRANSACTION_OBJECT * pobTran)
{
        int		inSecond = 0, inScheduleSecond = 0;
        char		szDate[8 + 1], szTime[6 + 1];
        char		szScheduleDate[8 + 1], szScheduleTime[6 + 1];
        char		szTemplate[16 + 1];
	char		szDebugMsg[100 + 1];
        RTC_NEXSYS	srRTC;
        VS_BOOL		fDateCheck = VS_FALSE, fTimeCheck = VS_FALSE;

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCCTMS_Schedule_Download_Date_Time_Check() START !!");
        
        
        if (inLoadTMSCPTRec(0) < 0)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inLoadTMSCPTRec(0) Error !!");
        }
        
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTMSDownloadFlag(szTemplate);
	
	/* 只有設定為排程下載時才繼續往下做*/
	if (memcmp(szTemplate, _TMS_DOWNLOAD_FLAG_SCHEDULE_, strlen(_TMS_DOWNLOAD_FLAG_SCHEDULE_)) == 0)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTMSScheduleRetry(szTemplate);
		if (atoi(szTemplate) >= 3)
		{
			/* 取消排程 */
			inResetTMSCPT_Schedule();
			
			/* 超過時間，刪除所有下載檔案 */
			inNCCCTMS_DeleteAllDownloadFile_Flow(pobTran);
			
			/* 下載失敗 DCC 排程時間也要歸零 */
			inSetDCCDownloadMode(_NCCC_DCC_DOWNLOAD_MODE_NOARMAL_);
			inSaveEDCRec(0);
		}
		
		memset(szScheduleDate, 0x00, sizeof(szScheduleDate));
		memset(szScheduleTime, 0x00, sizeof(szScheduleTime));
        
		/* 取得排程下載日期 */
		if (inGetTMSScheduleDate(szScheduleDate) < 0)
		{
			if (ginDebug == VS_TRUE)
				inLogPrintf(AT, "排程下載日期取得失敗");

			return (VS_ERROR);
		}

		/* 取得排程下載時間 */
		if (inGetTMSScheduleTime(szScheduleTime) < 0)
		{
			if (ginDebug == VS_TRUE)
				inLogPrintf(AT, "排程下載時間取得失敗");
                
			return (VS_ERROR);
		}

		if (inFunc_GetSystemDateAndTime(&srRTC) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		
		memset(szDate, 0x00, sizeof(szDate));
		sprintf(szDate, "20%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
		memset(szTime, 0x00, sizeof(szTime));
		sprintf(szTime, "%02d%02d%02d", srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);

		/* 端末機秒數 */
		inSecond = ((srRTC.uszHour) * 3600) + ((srRTC.uszMinute) * 60) + srRTC.uszSecond;

		/* 生效時間秒數 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(&szTemplate[0], &szScheduleTime[0], 2);
		inScheduleSecond = atoi(szTemplate) * 3600;
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(&szTemplate[0], &szScheduleTime[2], 2);
		inScheduleSecond = inScheduleSecond + (atoi(szTemplate) * 60);
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(&szTemplate[0], &szScheduleTime[4], 2);
		inScheduleSecond = inScheduleSecond + atoi(szTemplate);

		if (!memcmp(&szDate[0], &szScheduleDate[0], 8))
		{
			fDateCheck = VS_TRUE;
		}

		/* 端末機時間大於排程時間 */
		if (inSecond >= inScheduleSecond)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetTMSScheduleRetry(szTemplate);
			
			/* 超過排程時間，且小於15分鐘內才下載 或者 超過15分鐘但未嘗試下載，則至少下載一次 */
			if ((inSecond - inScheduleSecond) < (15 * 60) || atoi(szTemplate) == 0)
			{
				/* 小於生效時間3分鐘內，一直跳(因為Idle每五秒檢查一次，所以這裡每五秒跳一次) */
				if ((inSecond - inScheduleSecond) < 60 * 3)
				{
					fTimeCheck = VS_TRUE;
				}
				/* 大於生效時間每2分鐘跳一次 */
				else
				{
					if ((srRTC.uszMinute % 2 == 0))
					{
						fTimeCheck = VS_TRUE;
					}
				}
				
				if (fDateCheck == VS_TRUE && fTimeCheck == VS_TRUE)
					return (VS_SUCCESS);
				else
					return (VS_ERROR);
			}
			else
			{
				/* 取消排程 */
				inResetTMSCPT_Schedule();
				/* 超過時間，刪除所有下載檔案 */
				inNCCCTMS_DeleteAllDownloadFile_Flow(pobTran);
				
				return (VS_ERROR);
			}

		}
		else
		{
			/* 時間未到 */
			return (VS_ERROR);
		}

	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "非排程下載不檢核時間");
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	
        
}

/*
Function        :inNCCCTMS_Inquire
Date&Time       :2016/1/30 上午 11:53
Describe        :TMS詢問檢查
*/
int inNCCCTMS_Inquire(TRANSACTION_OBJECT *pobTran)
{
	int     inRetVal = VS_ERROR;
	int	inTransactionCode = 0;		/* 雖然流程已修正過，但為了避免以後用到這隻踩到地雷，還是做還原TransactionCode的動作 */
        char    szTemplate[16 + 1] = {0};
	char	szDemoMode[2 + 1] = {0};
	char	szCFESMode[2 + 1] = {0};
	
/* 沒連線能力 */
#ifndef _COMMUNICATION_CAPBILITY_
	return (VS_SUCCESS);
#endif
        vdUtility_SYSFIN_LogMessage(AT, "inNCCCTMS_Inquire START!");
        
	/* 教育訓練模式 */
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		return (VS_SUCCESS);
	}
	else
	{
		/* 若是連動結帳中就等到所有TRT都跑完再做TMS下載(包含Terminfo2.txt) */
		if (pobTran->uszAutoSettleBit == VS_TRUE)
		{
			return (VS_SUCCESS);
		}
			
		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "inNCCCTMS_Inquire() START!");    

		if (inLoadTMSSCTRec(0) != VS_SUCCESS)
			return (VS_ERROR);

		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTMSDownloadMode(szTemplate);
		memset(szCFESMode, 0x00, sizeof(szCFESMode));
		inGetCloud_MFES(szCFESMode);
		/* FTP不看參數詢問 */
		if (memcmp(szTemplate, _TMS_DLMODE_FTPS_, strlen(_TMS_DLMODE_FTPS_)) == 0)
		{
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);					/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_TMS_SCHEDULE_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 參數詢問 */

			if (memcmp(szCFESMode, "Y", strlen("Y")) == 0)
			{
				pobTran->uszFTP_TMS_Download = _TMS_DOWNLOAD_SECURE_MODE_SFTP_;
			}
			else
			{
				pobTran->uszFTP_TMS_Download = _TMS_DOWNLOAD_SECURE_MODE_FTPS_;
			}
		}
		else if (memcmp(szTemplate, _TMS_DLMODE_ISO8583_, strlen(_TMS_DLMODE_ISO8583_)) == 0)
		{
			/* ISO8583要看參數詢問 */
			memset(szTemplate, 0x00, sizeof(szTemplate)); 
			if (inGetTMSInquireMode(szTemplate) != VS_SUCCESS)
			{
				if (ginDebug == VS_TRUE)
					inLogPrintf(AT, "inGetTMSInquireMode() Error!");  
                                
                                vdUtility_SYSFIN_LogMessage(AT, "inGetTMSInquireMode() Error!");

				return (VS_ERROR);
			}

			/* TMS_Inquire_Mode = 0 結帳詢問  1 每次開機詢問及結帳詢問 */
			if (!memcmp(&szTemplate[0], _TMS_INQUIRE_00_SETTLE_, 1))
			{
				/* 結帳詢問直接從結帳電文中帶 */
                                vdUtility_SYSFIN_LogMessage(AT,"inNCCCTMS_Inquire Mode(%s) end", _TMS_INQUIRE_00_SETTLE_);
                                
				return (VS_SUCCESS);
			}
			else if (!memcmp(&szTemplate[0], _TMS_INQUIRE_01_SETTLE_POWERON_, 1))
			{
				if (pobTran->inRunOperationID == _OPERATION_EDC_BOOTING_	||
				    pobTran->inRunOperationID == _OPERATION_EDC_SDK_INITIAL_BOOTING_)
				{
					inDISP_ClearAll();
					inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);					/* 第一層顯示 LOGO */
					inDISP_PutGraphic(_MENU_TMS_SCHEDULE_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 參數詢問 */
				}
				else
				{
                                        vdUtility_SYSFIN_LogMessage(AT,"inNCCCTMS_Inquire TMSInquireMode(%s) not at booting", szTemplate);
                                        
					return (VS_SUCCESS);
				}	
			}
			else if (!memcmp(&szTemplate[0], _TMS_INQUIRE_02_SCHEDHULE_SETTLE_, 1))
			{
				/* 確認詢問日期和時間 */
				inRetVal = inNCCCTMS_Schedule_Inquire_Date_Time_Check();

				if (inRetVal == VS_SUCCESS)
				{
					inDISP_ClearAll();
					inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
					inDISP_PutGraphic(_MENU_TMS_SCHEDULE_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 參數詢問 */
				}
				else
				{
                                        vdUtility_SYSFIN_LogMessage(AT,"inNCCCTMS_Inquire TMSInquireMode(%s) not at booting", szTemplate);
                                        
					return (VS_SUCCESS);
				}

			}
			else
			{
                                vdUtility_SYSFIN_LogMessage(AT,"inNCCCTMS_Inquire TMSInquireMode(%s) 無此流程", szTemplate);
                                
				return (VS_SUCCESS);
			}

			pobTran->uszFTP_TMS_Download = _TMS_DOWNLOAD_SECURE_MODE_NONE_;
		}
		else
		{
                        vdUtility_SYSFIN_LogMessage(AT,"inNCCCTMS_Inquire TMSDownloadMode(%s) 無此TMS下載流程", szTemplate);
                        
			return (VS_SUCCESS);
		}


		/* 參數詢問 */
		inTransactionCode = pobTran->inTransactionCode;
		pobTran->inTMSDwdMode = _TMS_AUTO_DOWNLOAD_;		/* 自動下載 */

		/* 先initial Field 58 */
		inNCCCTMS_Field58_Initial(pobTran);

		/* LOAD NCCC HOST */
		if (inLoadHDTRec(0) != VS_SUCCESS)
			return (VS_ERROR);
		if (inLoadHDPTRec(0) != VS_SUCCESS)
			return (VS_ERROR);
		/* TMS的CPT參數 */
		if (inLoadTMSCPTRec(0) != VS_SUCCESS)
			return (VS_ERROR);
		/* EDC參數 */
		if (inLoadEDCRec(0) != VS_SUCCESS)
			return (VS_ERROR);
		/* CPT參數 */
		if (inLoadCPTRec(0) != VS_SUCCESS)
			return (VS_ERROR);    

		/* 連線到TMS主機 */
		if (inFLOW_RunFunction(pobTran, _FUNCTION_TMS_CONNECT_) != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
				inLogPrintf(AT, "_FUNCTION_TMS_CONNECT_()Error");

			/* 還原inTransactionCode*/
			pobTran->inTransactionCode = inTransactionCode;

                        vdUtility_SYSFIN_LogMessage(AT, "inNCCCTMS_ConnectToServer()Error");
			return (VS_ERROR);
		}

		/* TMS收送電文(暫時用pobTran->uszFTP_TMS_Download分) */
		if (pobTran->uszFTP_TMS_Download == _TMS_DOWNLOAD_SECURE_MODE_FTPS_	||
		    pobTran->uszFTP_TMS_Download == _TMS_DOWNLOAD_SECURE_MODE_SFTP_)
		{
			inSetFTPInquiryResponseCode(" ");
			inSaveTMSFTPRec(0);

			pobTran->inTransactionCode = _NCCCTMS_FTP_AUTO_INQUIRY_REPORT_;
			/* 只下TermInfo2詢問結果 */
			inFLOW_RunFunction(pobTran, _FUNCTION_TMS_SEND_RECEIVE_FTPS_);

			/* 一律重新連連線 (IFES下 TMS和FTP相同，可以考慮不重連) */
			/* 資訊回報 */
			pobTran->uszFTP_TMS_Download = _TMS_DOWNLOAD_SECURE_MODE_NONE_;
			pobTran->inTransactionCode = _NCCCTMS_FTP_AUTO_INQUIRY_REPORT_;
			if (inFLOW_RunFunction(pobTran, _FUNCTION_TMS_CONNECT_) == VS_SUCCESS)
			{
				/* 回報詢問結果 */
				inFLOW_RunFunction(pobTran, _FUNCTION_TMS_SEND_RECEIVE_);
			}
		}
		else if (pobTran->uszFTP_TMS_Download == _TMS_DOWNLOAD_SECURE_MODE_NONE_)
		{
			pobTran->inTransactionCode = _NCCCTMS_SCHEDULE_;
			inFLOW_RunFunction(pobTran, _FUNCTION_TMS_SEND_RECEIVE_);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "TMS Download Mode Not Set");
			}
		}

		/* 斷線 */
		inFLOW_RunFunction(pobTran, _FUNCTION_TMS_DISCONNECT_);

		/* 還原inTransactionCode*/
		pobTran->inTransactionCode = inTransactionCode;

		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "inNCCCTMS_Schedule_Inquire() END!");    

		return (VS_SUCCESS);
	}
}

/*
Function        :inNCCCTMS_Schedule_Download
Date&Time       :2017/1/25 下午 3:14
Describe        :TMS自動排程下載
*/
int inNCCCTMS_Schedule_Download(TRANSACTION_OBJECT *pobTran)
{
	int	inRetry = 0;
        int     inRetVal = VS_ERROR;
	int	inTransactionCode = 0;		/* 雖然流程已修正過，但為了避免以後用到這隻踩到地雷，還是做還原TransactionCode的動作 */
        char    szTemplate[16 + 1] = {0};
	char	szDebugMsg[100 + 1] = {0};
	char	szDemoMode[2 + 1] = {0};
	char	szCFESMode[2 + 1] = {0};
	
        vdUtility_SYSFIN_LogMessage(AT, "inNCCCTMS_Schedule_Download START!");
        
	/* 教育訓練模式 */
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		return (VS_SUCCESS);
	}
	else
	{
		/* 若是連動結帳中就等到所有TRT都跑完再做TMS下載 */
		if (pobTran->uszAutoSettleBit == VS_TRUE)
		{
			return (VS_SUCCESS);
		}

		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "inNCCCTMS_Schedule_Download() START!");

		/* 先查看是哪一種下載模式，
		 * 若為ISO8583，由結帳電文帶下，
		 * 若為FTP，則要下載FileList看是否有差異
		 */

		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTMSDownloadMode(szTemplate);

		/* ISO8583*/
		if (memcmp(szTemplate, _TMS_DLMODE_ISO8583_, strlen(_TMS_DLMODE_ISO8583_)) == 0)
		{
			if (inLoadTMSCPTRec(0) != VS_SUCCESS)
				return (VS_ERROR);

			memset(szTemplate, 0x00, sizeof(szTemplate));
			/* 確認是否立即下載 */
			if (inGetTMSDownloadFlag(szTemplate) != VS_SUCCESS)
			{
				if (ginDebug == VS_TRUE)
					inLogPrintf(AT, "inGetTMSDownloadFlag() Error!");   

				return (VS_ERROR);
			}

			/* TMS_Download Flag = 1 立即下載 */
			if (!memcmp(&szTemplate[0], _TMS_DOWNLOAD_FLAG_IMMEDIATE_, 1))
			{

			}
			/* TMS_Download Flag = 2 排程下載 */
			else if (!memcmp(&szTemplate[0], _TMS_DOWNLOAD_FLAG_SCHEDULE_, 1))
			{
				/* 確認詢問日期和時間 */
				inRetVal = inNCCCTMS_Schedule_Download_Date_Time_Check(pobTran);

				if (inRetVal != VS_SUCCESS)
				{
					/* 代表時間未到 */
					return (VS_SUCCESS);
				}

			}
			/* 無須下載(0)或無定義 */
			else
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "無須下載或無定義 :%s", szTemplate);
					inLogPrintf(AT, szDebugMsg);
				}

				return (VS_SUCCESS);
			}

			/* 設定不是FTP 下載*/
			pobTran->uszFTP_TMS_Download = _TMS_DOWNLOAD_SECURE_MODE_NONE_;
		}
		else if (memcmp(szTemplate, _TMS_DLMODE_FTPS_, strlen(_TMS_DLMODE_FTPS_)) == 0)
		{
			memset(szCFESMode, 0x00, sizeof(szCFESMode));
			inGetCloud_MFES(szCFESMode);
			/* 設定是FTP 下載*/
			if (memcmp(szCFESMode, "Y", strlen("Y")) == 0)
			{
				pobTran->uszFTP_TMS_Download = _TMS_DOWNLOAD_SECURE_MODE_SFTP_;
			}
			else
			{
				pobTran->uszFTP_TMS_Download = _TMS_DOWNLOAD_SECURE_MODE_FTPS_;
			}

		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "找不到此TMS下載模式 :%s", szTemplate);
				inLogPrintf(AT, szDebugMsg);
			}

			return (VS_ERROR);
		}

		/* 顯示參數下載 */
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_PARAM_DOWNLOAD_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 參數下載 */

		/* 開始下載 */
		/* 參數下載 */
		inTransactionCode = pobTran->inTransactionCode;				/* 紀錄原transaction code用，跑完還原 */
		pobTran->inTMSDwdMode = _TMS_AUTO_DOWNLOAD_;				/* 自動下載 */
		/* 先initial Field 58 */
		inNCCCTMS_Field58_Initial(pobTran);

		/* 看download scope  */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTMSDownloadScope(szTemplate);
		strcpy(gsrTMS_Field58.szDownloadScope ,szTemplate);

		/* LOAD NCCC HOST */
		if (inLoadHDTRec(0) != VS_SUCCESS)
			return (VS_ERROR);
		if (inLoadHDPTRec(0) != VS_SUCCESS)
			return (VS_ERROR);
		/* EDC參數 */
		if (inLoadEDCRec(0) != VS_SUCCESS)
			return (VS_ERROR);
		/* CPT參數 */
		if (inLoadCPTRec(0) != VS_SUCCESS)
			return (VS_ERROR);    

		/* 連線到TMS主機(相當於_FUNCTION_TMS_CONNECTING_) */
		if (inFLOW_RunFunction(pobTran, _FUNCTION_TMS_CONNECT_) != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
				inLogPrintf(AT, "inNCCCTMS_ConnectToServer()Error");

			return (VS_ERROR);
		}

		/* 重試次數先加一，怕產生斷電的情形 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTMSScheduleRetry(szTemplate);
		inRetry = atoi(szTemplate);
		inRetry++;
		sprintf(szTemplate, "%02d",inRetry);
		inSetTMSScheduleRetry(szTemplate);
		inSaveTMSCPTRec(0);

		/* TMS收送電文(暫時用pobTran->uszFTP_TMS_Download分) */
		if (pobTran->uszFTP_TMS_Download == _TMS_DOWNLOAD_SECURE_MODE_FTPS_	||
		    pobTran->uszFTP_TMS_Download == _TMS_DOWNLOAD_SECURE_MODE_SFTP_)
		{
			/* FTP無排程下載 */
		}
		else if (pobTran->uszFTP_TMS_Download == _TMS_DOWNLOAD_SECURE_MODE_NONE_)
		{
			pobTran->inTransactionCode = _NCCCTMS_LOGON_;				/* 這邊直接下載 */
			inFLOW_RunFunction(pobTran, _FUNCTION_TMS_SEND_RECEIVE_);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "TMS Download Mode Not Set");
			}
		}

		if (pobTran->inTransactionResult != _NCCCTMS_AUTHORIZED_)
		{

		}
		else
		{
			inSetTMSScheduleRetry("00");
			inSaveTMSCPTRec(0);
			inNCCCTMS_PRINT_ScheduleMessage(pobTran, _TMS_PRT_SCHEDULE_SUCCESS_);
		}

		/* 列印結果及更新 */
		inFLOW_RunFunction(pobTran, _FUNCTION_TMS_RESULT_HANDLE_);

		/* 斷線 */
		inFLOW_RunFunction(pobTran, _FUNCTION_TMS_DISCONNECT_);

		/* 還原inTransactionCode*/
		pobTran->inTransactionCode = inTransactionCode;

		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "inNCCCTMS_Schedule_Download() END!");    

		return (VS_SUCCESS);
	}
}

/*
Function        :inNCCCTMS_Download_Settle
Date&Time       :2017/1/17 下午 2:04
Describe        :結帳後TMS下載
*/
int inNCCCTMS_Download_Settle(TRANSACTION_OBJECT *pobTran)
{
	int     inRetVal = VS_ERROR;
	int	inTransactionCode = 0;		/* 雖然流程已修正過，但為了避免以後用到這隻踩到地雷，還是做還原TransactionCode的動作 */
        char    szTemplate[16 + 1] = {0};
	char	szDebugMsg[100 + 1] = {0};
	char	szDemoMode[2 + 1] = {0};
	char	szCFESMode[2 + 1] = {0};

	/* 教育訓練模式 */
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		return (VS_SUCCESS);
	}
	else
	{
		/* 若是連動結帳中就等到所有TRT都跑完再做TMS下載 */
		if (pobTran->uszAutoSettleBit == VS_TRUE)
		{
			return (VS_SUCCESS);
		}

		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "inNCCCTMS_Download_Settle() START!");

		/* 先查看是哪一種下載模式，
		 * 若為ISO8583，由結帳電文帶下，
		 * 若為FTP，則要下載FileList看是否有差異
		 */

		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTMSDownloadMode(szTemplate);

		/* ISO8583*/
		if (memcmp(szTemplate, _TMS_DLMODE_ISO8583_, strlen(_TMS_DLMODE_ISO8583_)) == 0)
		{
			if (inLoadTMSCPTRec(0) != VS_SUCCESS)
				return (VS_ERROR);

			memset(szTemplate, 0x00, sizeof(szTemplate));
			/* 確認是否立即下載 */
			if (inGetTMSDownloadFlag(szTemplate) != VS_SUCCESS)
			{
				if (ginDebug == VS_TRUE)
					inLogPrintf(AT, "inGetTMSDownloadFlag() Error!");   

                                vdUtility_SYSFIN_LogMessage(AT, "inGetTMSDownloadFlag() Error");
				return (VS_ERROR);
			}

			/* TMS_Download Flag = 1 立即下載 */
			if (!memcmp(&szTemplate[0], _TMS_DOWNLOAD_FLAG_IMMEDIATE_, 1))
			{

			}
			/* TMS_Download Flag = 2 排程下載 */
			else if (!memcmp(&szTemplate[0], _TMS_DOWNLOAD_FLAG_SCHEDULE_, 1))
			{
				/* 確認詢問日期和時間 */
				inRetVal = inNCCCTMS_Schedule_Inquire_Date_Time_Check();

				if (inRetVal != VS_SUCCESS)
				{
					/* 代表時間未到 */
					return (VS_SUCCESS);
				}

			}
			/* 無須下載(0)或無定義 */
			else
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "無須下載或無定義 :%s", szTemplate);
					inLogPrintf(AT, szDebugMsg);
				}

				return (VS_SUCCESS);
			}

			/* 設定不是FTP 下載*/
			pobTran->uszFTP_TMS_Download = _TMS_DOWNLOAD_SECURE_MODE_NONE_;
		}
		else if (memcmp(szTemplate, _TMS_DLMODE_FTPS_, strlen(_TMS_DLMODE_FTPS_)) == 0)
		{
			memset(szCFESMode, 0x00, sizeof(szCFESMode));
			inGetCloud_MFES(szCFESMode);
			/* 設定是FTP 下載*/
			if (memcmp(szCFESMode, "Y", strlen("Y")) == 0)
			{
				pobTran->uszFTP_TMS_Download = _TMS_DOWNLOAD_SECURE_MODE_SFTP_;
			}
			else
			{
				pobTran->uszFTP_TMS_Download = _TMS_DOWNLOAD_SECURE_MODE_FTPS_;
			}
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "找不到此TMS下載模式 :%s", szTemplate);
				inLogPrintf(AT, szDebugMsg);
			}
                        vdUtility_SYSFIN_LogMessage(AT,"無此TMS下載模式 :%s", szTemplate);

			return (VS_ERROR);
		}

		/* 顯示參數下載 */
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_PARAM_DOWNLOAD_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 參數下載 */
		
		/* 開始下載 */
		/* 參數下載 */
		inTransactionCode = pobTran->inTransactionCode;				/* 紀錄原transaction code用，跑完還原 */
		pobTran->inTMSDwdMode = _TMS_AUTO_DOWNLOAD_;				/* 自動下載 */

		/* 先initial Field 58 */
		inNCCCTMS_Field58_Initial(pobTran);

		/* 看download scope  */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTMSDownloadScope(szTemplate);
		strcpy(gsrTMS_Field58.szDownloadScope ,szTemplate);

		/* LOAD NCCC HOST */
		if (inLoadHDTRec(0) != VS_SUCCESS)
			return (VS_ERROR);
		if (inLoadHDPTRec(0) != VS_SUCCESS)
			return (VS_ERROR);
		/* EDC參數 */
		if (inLoadEDCRec(0) != VS_SUCCESS)
			return (VS_ERROR);
		/* CPT參數 */
		if (inLoadCPTRec(0) != VS_SUCCESS)
			return (VS_ERROR);    

		/* TMS收送電文(暫時用pobTran->uszFTP_TMS_Download分) */
		if (pobTran->uszFTP_TMS_Download == _TMS_DOWNLOAD_SECURE_MODE_FTPS_	||
		    pobTran->uszFTP_TMS_Download == _TMS_DOWNLOAD_SECURE_MODE_SFTP_)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetFTPInquiryResponseCode(szTemplate);
			if (memcmp(szTemplate, _FTP_INQUIRY_REPORT_NO_DOWNLOAD_, 1) == 0)
			{
				/* 不下載，跳過流程 */
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "TMS Download No download");
				}
			}
			else
			{
				/* 連線到TMS主機(相當於_FUNCTION_TMS_CONNECTING_) */
				if (inFLOW_RunFunction(pobTran, _FUNCTION_TMS_CONNECT_) != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
						inLogPrintf(AT, "inNCCCTMS_ConnectToServer()Error");

                                        vdUtility_SYSFIN_LogMessage(AT, "inNCCCTMS_ConnectToServer()Error");
					return (VS_ERROR);
				}
		
				pobTran->inTransactionCode = _NCCCTMS_FTP_DOWNLOAD_TMS_REPORT_;

				/* 下載FileList內的東西 */
				inNCCCTMS_FuncSendReceive_FTPS(pobTran);
			}
		}
		else if (pobTran->uszFTP_TMS_Download == _TMS_DOWNLOAD_SECURE_MODE_NONE_)
		{
			/* 連線到TMS主機(相當於_FUNCTION_TMS_CONNECTING_) */
			if (inFLOW_RunFunction(pobTran, _FUNCTION_TMS_CONNECT_) != VS_SUCCESS)
			{
				if (ginDebug == VS_TRUE)
					inLogPrintf(AT, "inNCCCTMS_ConnectToServer()Error");

                                vdUtility_SYSFIN_LogMessage(AT, "inNCCCTMS_ConnectToServer()Error");
				return (VS_ERROR);
			}
		
			pobTran->inTransactionCode = _NCCCTMS_LOGON_;				/* 這邊直接下載 */
			inFLOW_RunFunction(pobTran, _FUNCTION_TMS_SEND_RECEIVE_);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "TMS Download Mode Not Set");
			}
		}

		/* 列印結果及更新 */
		inFLOW_RunFunction(pobTran, _FUNCTION_TMS_RESULT_HANDLE_);

		/* 斷線 */
		inFLOW_RunFunction(pobTran, _FUNCTION_TMS_DISCONNECT_);

		/* 還原inTransactionCode*/
		pobTran->inTransactionCode = inTransactionCode;

		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "inNCCCTMS_Schedule_Download_Check() END!");    

		return (VS_SUCCESS);
	}
}

/*
Function        :inNCCCTMS_TMS_Return_Report
Date&Time       :2016/2/2 下午 2:32
Describe        :TMS參數生效回報
*/
int inNCCCTMS_TMS_Return_Report(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	int	inRetryTimesMax = 4;
	int	inRetryTimes = 0;
	char	szTemplate[2 + 1] = {0};
	TMS_OBJECT	srTMS = {0};
	
	memset(&srTMS, 0x00, sizeof(srTMS));
	
        /* 參數生效回報 */
        inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
        inDISP_PutGraphic(_MENU_TMS_REPORT_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 參數生效回報 */
        pobTran->inTMSDwdMode = _TMS_AUTO_DOWNLOAD_;					/* 自動下載 */
	pobTran->uszFTP_TMS_Download = _TMS_DOWNLOAD_SECURE_MODE_NONE_;			/* 參數回報一律走ISO8583 */
        /* 先initial Field 58 */
        inNCCCTMS_Field58_Initial(pobTran);
        /* 參數生效回報上傳 */
        strcpy(gsrTMS_Field58.szDownloadScope ,_TMS_DOWNLOAD_SCOPE_PARAMETER_EFFECT_REPORT_);

        /* LOAD NCCC HOST */
        if (inLoadHDTRec(0) != VS_SUCCESS)
	{
		vdUtility_SYSFIN_LogMessage(AT, "inNCCCTMS_TMS_Return_Report inLoadHDTRec Failed");
                return (VS_ERROR);
	}
	if (inLoadHDPTRec(0) != VS_SUCCESS)
	{
		vdUtility_SYSFIN_LogMessage(AT, "inNCCCTMS_TMS_Return_Report inLoadHDPTRec Failed");
                return (VS_ERROR);
	}
        /* TMS的CPT參數 */
        if (inLoadTMSCPTRec(0) != VS_SUCCESS)
	{
		vdUtility_SYSFIN_LogMessage(AT, "inNCCCTMS_TMS_Return_Report inLoadTMSCPTRec Failed");
                return (VS_ERROR);
	}
        /* EDC參數 */
        if (inLoadEDCRec(0) != VS_SUCCESS)
	{
		vdUtility_SYSFIN_LogMessage(AT, "inNCCCTMS_TMS_Return_Report inLoadEDCRec Failed");
                return (VS_ERROR);
	}
        /* CPT參數 */
        if (inLoadCPTRec(0) != VS_SUCCESS)
	{
		vdUtility_SYSFIN_LogMessage(AT, "inNCCCTMS_TMS_Return_Report inLoadCPTRec Failed");
                return (VS_ERROR);
	}

        /* 連線到TMS主機 */
	inRetryTimes = 0;
	do
	{
		inRetVal = inFLOW_RunFunction(pobTran, _FUNCTION_TMS_CONNECT_);
		inRetryTimes++;
		if (inRetVal == VS_SUCCESS)
		{
			vdUtility_SYSFIN_LogMessage(AT, "TMS連線成功");
			break;
		}
		
		if (inRetryTimes > inRetryTimesMax)
		{
			vdUtility_SYSFIN_LogMessage(AT, "TMS連線超過最大重試, inRetryTimes:%d", inRetryTimes);
			break;
		}
		
		if (inRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
				inLogPrintf(AT, "inNCCCTMS_ConnectToServer()Error");
		}
		
	}while(1);

	/* 連線成功才回報 */
	if (inRetVal == VS_SUCCESS)
	{
		/* TMS收送電文 */
		inRetryTimes = 0;
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTMSDownloadMode(szTemplate);
		if (memcmp(szTemplate, _TMS_DLMODE_FTPS_, strlen(_TMS_DLMODE_FTPS_)) == 0)
		{	
			pobTran->inTransactionCode = _NCCCTMS_FTP_DOWNLOAD_TMS_REPORT_;

			do
			{
				inRetVal = inFLOW_RunFunction(pobTran, _FUNCTION_TMS_SEND_RECEIVE_ONLY_PACKET_);
				inRetryTimes++;
				if (inRetVal == VS_SUCCESS)
				{
					vdUtility_SYSFIN_LogMessage(AT, "TMS回報成功 ISO8583");
					break;
				}
				else
				{
					vdUtility_SYSFIN_LogMessage(AT, "TMS回報失敗 Resp:%s", pobTran->srBRec.szRespCode);
				}

				if (inRetryTimes >= inRetryTimesMax)
				{
					vdUtility_SYSFIN_LogMessage(AT, "TMS回報超過最大次數", inRetryTimes);
					break;
				}

			}while(1);

			if (inRetVal != VS_SUCCESS)
			{
				srTMS.inTransactionResult = pobTran->inTransactionResult;
				strcpy(srTMS.szRespCode, pobTran->srBRec.szRespCode);
				inNCCCTMS_DispHostResponseCode(&srTMS);
			}
		}
		else if (memcmp(szTemplate, _TMS_DLMODE_ISO8583_, strlen(_TMS_DLMODE_ISO8583_)) == 0)
		{
			pobTran->inTransactionCode = _NCCCTMS_TRACE_LOG_;

			do
			{
				inRetVal = inFLOW_RunFunction(pobTran, _FUNCTION_TMS_SEND_RECEIVE_ONLY_PACKET_);
				inRetryTimes++;
				if (inRetVal == VS_SUCCESS)
				{
					vdUtility_SYSFIN_LogMessage(AT, "TMS回報成功 ISO8583");
					break;
				}
				else
				{
					vdUtility_SYSFIN_LogMessage(AT, "TMS回報失敗 Resp:%s", pobTran->srBRec.szRespCode);
				}

				if (inRetryTimes >= inRetryTimesMax)
				{
					vdUtility_SYSFIN_LogMessage(AT, "TMS回報超過最大次數", inRetryTimes);
					break;
				}

			}while(1);

			if (inRetVal != VS_SUCCESS)
			{
				srTMS.inTransactionResult = pobTran->inTransactionResult;
				strcpy(srTMS.szRespCode, pobTran->srBRec.szRespCode);
				inNCCCTMS_DispHostResponseCode(&srTMS);
			}
		}
	}
	
	/* 斷線 */
	inFLOW_RunFunction(pobTran, _FUNCTION_TMS_DISCONNECT_);
        
        return (VS_SUCCESS);
}

/*
Function        :inNCCCTMS_Func6TraceLog_Upload
Date&Time       :2017/5/19 下午 3:42
Describe        :TMS資訊回報 or Tracelog upload
*/
int inNCCCTMS_Func6TraceLog_Upload(TRANSACTION_OBJECT *pobTran)
{
        vdUtility_SYSFIN_LogMessage(AT, "inNCCCTMS_Func6TraceLog_Upload START!");
        
        /* 參數生效回報 */
        inDISP_ClearAll();
        inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
        inDISP_PutGraphic(_MENU_TMS_REPORT_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 參數生效回報 */
        pobTran->inTransactionCode = _NCCCTMS_TRACE_LOG_;
        pobTran->inTMSDwdMode = _TMS_AUTO_DOWNLOAD_;					/* 自動下載 */
	pobTran->uszFTP_TMS_Download = _TMS_DOWNLOAD_SECURE_MODE_NONE_;			/* 參數回報一律走ISO8583 */
        /* 先initial Field 58 */
        inNCCCTMS_Field58_Initial(pobTran);
        /* 參數生效回報上傳 */
        strcpy(gsrTMS_Field58.szDownloadScope ,"5");

        /* LOAD NCCC HOST */
        if (inLoadHDTRec(0) != VS_SUCCESS)
                return (VS_ERROR);
	if (inLoadHDPTRec(0) != VS_SUCCESS)
                return (VS_ERROR);
        /* TMS的CPT參數 */
        if (inLoadTMSCPTRec(0) != VS_SUCCESS)
                return (VS_ERROR);
        /* EDC參數 */
        if (inLoadEDCRec(0) != VS_SUCCESS)
                return (VS_ERROR);
        /* CPT參數 */
        if (inLoadCPTRec(0) != VS_SUCCESS)
                return (VS_ERROR);    
        
        return (VS_SUCCESS);
}

/*
Function        :inNCCCTMS_Settle_Check
Date&Time       :2016/2/2 下午 8:02
Describe        :判斷是否要先結帳
*/
int inNCCCTMS_Settle_Check(TRANSACTION_OBJECT *pobTran)
{
        int     i;
        int     inBAKTotalCnt = 0;      /* 檔案總筆數 */
        char    szTemplate[16 + 1];
        unsigned char   uszFileName[15 + 1];
        unsigned long   ulBKEYHandle, lnBAKTotalFileSize = 0;
        
        for (i = 0 ;; i ++)
        {
                if (inLoadHDTRec(i) < 0)
                        break;
                
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetHostEnable(szTemplate);
                
                if (!memcmp(&szTemplate[0], "Y", 1))
                {
                        if (inLoadHDPTRec(i) < 0)
                                return (VS_ERROR);
                }
                else
                {
                        continue;
                }
                
		if (inFunc_ComposeFileName(pobTran, (char*)uszFileName, _BATCH_KEY_FILE_EXTENSION_, 6) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
                
                if (inFILE_Check_Exist(uszFileName) == VS_ERROR)
                        continue;
                else
                {
                        /* 算出 TRANS_BATCH_KEY 總合 */
                        lnBAKTotalFileSize = lnFILE_GetSize(&ulBKEYHandle, uszFileName);
                        /* 算出交易總筆數，因為lnFILE_GetSize回傳值為long，因為此函式回傳int所以強制轉型，因為筆數不會超過int大小 */
                        inBAKTotalCnt = (int)(lnBAKTotalFileSize / _BATCH_KEY_SIZE_);
                        
                        if (inBAKTotalCnt > 0)
                        {
                                inSetMustSettleBit("Y");
                                inSaveHDPTRec(i);
                        }
                }
        }
        
        return (VS_SUCCESS);
}

/*
Function        :inNCCCTMS_TMS_ReturnTaskReport
Date&Time       :2017/5/10 下午 2:13
Describe        :功能五 至現回報
*/
int inNCCCTMS_Func5ReturnTaskReport(TRANSACTION_OBJECT *pobTran)
{
        int		i = 0;
	int		inRetVal = VS_SUCCESS;
	int		inChoice = 0;
	int		inTouchSensorFunc = _Touch_8X16_OPT_;
        char		szTemplate[100 + 1];
        char		szUTF8[1024 + 1];
        unsigned char   uszKey, uszFileName[16 + 1];
        DISPLAY_OBJECT  srDispObj;
        
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCCTMS_TMS_ReturnTaskReport() START!");
	
        /* 先LOAD NCCC HOST */
        inLoadHDTRec(0);
        inLoadHDPTRec(0);
        /* TMS的CPT參數 */
        inLoadTMSCPTRec(0);
        /* EDC參數 */
        inLoadEDCRec(0);
        /* CPT參數 */
        inLoadCPTRec(0);
        
        /* 檢查是否做過參數下載 */
        inRetVal = inNCCCTMS_CheckTMSOK(pobTran);
        
        if (inRetVal != VS_SUCCESS)
        {
                /* 請執行參數下載 */
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, _ERR_TMS_DWL_);
		srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
		srDispMsgObj.inMsgType = _NO_KEY_MSG_;
		srDispMsgObj.inTimeout = 0;
		strcpy(srDispMsgObj.szErrMsg1, "");
		srDispMsgObj.inErrMsg1Line = 0;
		srDispMsgObj.inBeepTimes = 3;
		srDispMsgObj.inBeepInterval = 1000;
			
                inDISP_Msg_BMP(&srDispMsgObj);
		
                return (VS_ERROR);
        }

        pobTran->inTransactionCode = _NCCCTMS_TASK_; /* Hard code至現回報 */
        pobTran->inTMSDwdMode = _TMS_MANUAL_DOWNLOAD_; /* 自動下載 */
        
        /* 初始化Field 58 */
        inNCCCTMS_Field58_Initial(pobTran);
        /* 至現回報上傳 */
        strcpy(gsrTMS_Field58.szDownloadScope ,"4");
        
        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        inDISP_PutGraphic(_MENU_TMS_TASK_OPTION_, 0, _COORDINATE_Y_LINE_8_4_);
        memset(uszFileName, 0x00, sizeof(uszFileName));
        
	inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);
	
        while (1)
        {
                inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
                uszKey = uszKBD_Key();
		
		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			uszKey = _KEY_TIMEOUT_;
		}
                
                if (uszKey == _KEY_1_			||
		    inChoice == _OPTTouch8X16_LINE_5_)
		{
                        /* 安裝至現回報 */
                        inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
                        inDISP_PutGraphic(_MENU_TMS_TASK_QAI_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 <安裝至現回報> */
                        gsrTMS_Field58.szOnsiteReportFlag[0] = 'I';
                        strcpy((char *)uszFileName, "QA_I.dat");
                        
                        if (inFILE_Check_Exist(uszFileName) != VS_SUCCESS)
			{
                                inRetVal = VS_ERROR;
				break;
			}
                        
			inRetVal = VS_SUCCESS;
                        break;
                }
                else if (uszKey == _KEY_2_			||
		         inChoice == _OPTTouch8X16_LINE_6_)
		{
                        /* 維護至現回報 */
                        inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
                        inDISP_PutGraphic(_MENU_TMS_TASK_QAM_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 <維護至現回報> */
                        gsrTMS_Field58.szOnsiteReportFlag[0] = 'M';
                        strcpy((char *)uszFileName, "QA_M.dat");
                        
                        if (inFILE_Check_Exist(uszFileName) != VS_SUCCESS)
                        {
                                inRetVal = VS_ERROR;
				break;
			}
                        
			inRetVal = VS_SUCCESS;
                        break;
                }
                else if (uszKey == _KEY_3_			||
		         inChoice == _OPTTouch8X16_LINE_6_)
		{
                        /* 共用至現回報 */
                        inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
                        inDISP_PutGraphic(_MENU_TMS_TASK_QAS_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 <共用至現回報> */
                        gsrTMS_Field58.szOnsiteReportFlag[0] = 'S';
                        strcpy((char *)uszFileName, "QA_S.dat");
                        
                        if (inFILE_Check_Exist(uszFileName) != VS_SUCCESS)
                        {
                                inRetVal = VS_ERROR;
				break;
			}
                        
			inRetVal = VS_SUCCESS;
                        break;
                }
                else if (uszKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else if (uszKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}
        }
	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
        
        /* 問卷顯示在EDC上 */
        for (i = 0 ;; i ++)
        {
                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                
                if (inLoadQATRec(i, uszFileName) < 0)
                        break;
                
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetQADBIndex(szTemplate);
                /* On Site Report Data 索引3碼+Y or N */
                memcpy(&gsrTMS_Field58.szOnsiteReportData[strlen(gsrTMS_Field58.szOnsiteReportData)], &szTemplate[0], 3);
                
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetQuestion(szTemplate);

                memset(szUTF8, 0x00, sizeof(szUTF8));
                /* BIG5轉UTF-8 */
                inFunc_Big5toUTF8(szUTF8, szTemplate);
                
                inDISP_ChineseFont(szUTF8, _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
                inDISP_ChineseFont("是=１，否=０", _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);

                /* 等待使用者輸入1 or 0 */
                while (1)
                {
                        uszKey = uszKBD_GetKey(30);

                        if (uszKey == _KEY_1_)
                        {
                                strcat(gsrTMS_Field58.szOnsiteReportData, "Y");
                                break;
                        }
                        else if (uszKey == _KEY_0_)
                        {
                                strcat(gsrTMS_Field58.szOnsiteReportData, "N");
                                break;
                        }
                        else if (uszKey == _KEY_CANCEL_ || uszKey == _KEY_TIMEOUT_)
                        {
                                return (VS_ERROR);
                        }
                        else
                        {
                                continue;
                        }
                }
        }
        
        /* 問卷未滿15題，剩餘答案位置則固定填入”000N”，補足60 Bytes */
        for (i = strlen(gsrTMS_Field58.szOnsiteReportData); i < 60; i += 4)
                memcpy(&gsrTMS_Field58.szOnsiteReportData[strlen(gsrTMS_Field58.szOnsiteReportData)], "000N", 4);        

        /* 輸入下載管理號碼 */
        if (inNCCCTMS_Download_PWD_GET(pobTran) != VS_SUCCESS)
                return (VS_ERROR);
        
        /* 輸入作業批號 */
        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        inDISP_PutGraphic(_TMS_BATCH_NUM_, 0, _COORDINATE_Y_LINE_8_4_);
        inDISP_TTF_SetFont(_DISP_ENGLISH_, _FONT_DISPLAY_REGULAR_);
        
	while (1)
	{
                inDISP_BEEP(1, 0);
		inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
		memset(&srDispObj, 0x00, DISPLAY_OBJECT_SIZE);

                /* 設定顯示變數 */
                srDispObj.inMaxLen = 8;
                srDispObj.inY = _LINE_8_7_;
                srDispObj.inR_L = _DISP_RIGHT_;
                srDispObj.inMask = VS_FALSE;	/* 作業批號不遮掩 */
		srDispObj.inColor = _COLOR_RED_;
		srDispObj.inTimeout = 30;

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;
	
                inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);
                
		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (VS_ERROR);

		if (srDispObj.inOutputLen > 0)
                {
			if (srDispObj.inOutputLen >= 8)
			{
				memcpy(&gsrTMS_Field58.szBatchNumber[0], &srDispObj.szOutput[0], 8);
				break;
			}
			else
			{
				continue;
			}
                }
                else
		{
                        continue;
		}
	}

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCCTMS_TMS_ReturnTaskReport() END!");
        
        return (VS_SUCCESS);
}

/*
Function        :inNCCCTMS_FTPS_TermInfo_Download
Date&Time       :2018/6/27 下午 3:45
Describe        :下載TermInfo2並分析
*/
int inNCCCTMS_FTPS_TermInfo_Download(TRANSACTION_OBJECT *pobTran)
{
        int		inFTPPort = 0, inRetVal = VS_ERROR;
        char		szTemplate[128 + 1] = {0}, szTemplate_1[128 + 1] = {0};
        char		szTerminalID[8 + 1] = {0};				/* 端末機代號 */
        char		szFTPIPAddress[16 + 1] = {0};				/* FTP IP Address */
        char		szFTPPortNum[6 + 1] = {0};				/* FTP Port Number */
        char		szFTPID[20 + 1] = {0};				/* FTP ID */
        char		szFTPPW[20 + 1] = {0};				/* FTP PW */
	char		szIFESMode[2 + 1] = {0};
	char		szCFESMode[2 + 1] = {0};
	char		szDemoMode[2 + 1] = {0};
        FTPS_REC        srFtpsObj;

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCCTMS_FTPS_TermInfo_Download() START!");
	
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
        
        /* 取得FTPS需要用的IP PORT ID PW */
	memset(&srFtpsObj, 0x00, sizeof(srFtpsObj));
	memset(szFTPIPAddress, 0x00, sizeof(szFTPIPAddress));
	inGetTMS_IP_Primary(szFTPIPAddress);
	memset(szFTPPortNum, 0x00, sizeof(szFTPPortNum));
	inGetFTPPortNum(szFTPPortNum);
	memset(szFTPID, 0x00, sizeof(szFTPID));
	inGetFTPID(szFTPID);
	memset(szFTPPW, 0x00, sizeof(szFTPPW));
	inGetFTPPW(szFTPPW);
		
        if (inLoadHDTRec(0) != VS_SUCCESS)
                return (VS_ERROR);
        if (inLoadHDPTRec(0) != VS_SUCCESS)
                return (VS_ERROR);
	
        /* FTPS目錄用TID來命名 */
        memset(szTerminalID, 0x00, sizeof(szTerminalID));
        inGetTerminalID(szTerminalID);
        memset(szTemplate_1, 0x00, sizeof(szTemplate_1));
        memcpy(&szTemplate_1[0], &szTerminalID[0], 4);
        
	memset(szIFESMode, 0x00, sizeof(szIFESMode));
	inGetI_FES_Mode(szIFESMode);
	memset(szCFESMode, 0x00, sizeof(szCFESMode));
	inGetCloud_MFES(szCFESMode);
        
        /* FTPS Port */
	inFTPPort = atoi(szFTPPortNum);
        srFtpsObj.inFtpsPort = inFTPPort;
        
        /* FTPS ID */
        strcpy(srFtpsObj.szFtpsID, szFTPID);

        /* FTPS PW */
        strcpy(srFtpsObj.szFtpsPW, szFTPPW);
	
        /* 下載的檔案名 */
        /* 聯合FTPS須先下載TermInfo2.txt 查核檔案及TID MID版本日期等等 */
        strcpy(srFtpsObj.szFtpsFileName, _FTP_INFO_2_);
	
	/* 組FTPS URL */
	/* IFES要用外顯式explicit，先走ftp再讓他自己轉 */
	if (memcmp(szCFESMode, "Y", strlen("Y")) == 0	||
	    pobTran->uszFTP_TMS_Download == _TMS_DOWNLOAD_SECURE_MODE_SFTP_)
	{
		/* 沒有找到原因，但CURL使用SFTP必須將使用者帳戶放到URL的前面 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "sftp://%s@%s/u01/users/tmsap/PARM/%s/%s/%s", srFtpsObj.szFtpsID, szFTPIPAddress, szTemplate_1, szTerminalID, _FTP_INFO_2_);
		strcpy(srFtpsObj.szFtpsURL, szTemplate);
	}
	else if (memcmp(szIFESMode, "Y", strlen("Y")) == 0)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "ftp://%s/u01/users/tmsap/PARM/%s/%s/%s", szFTPIPAddress, szTemplate_1, szTerminalID, _FTP_INFO_2_);
		strcpy(srFtpsObj.szFtpsURL, szTemplate);
	}
	else
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "ftps://%s/PARM/%s/%s/%s", szFTPIPAddress, szTemplate_1, szTerminalID, _FTP_INFO_2_);
		strcpy(srFtpsObj.szFtpsURL, szTemplate);
	}

	/* ======================================================================================================================
	   1.取得 TermInfo2.txt檔
	====================================================================================================================== */
	do
	{
		/* FTPS下載 重試3次 */
		/* 下載檔案 失敗重試3次 */
		/* 使用新憑證 */
		memset(srFtpsObj.szCACertFilePath, 0x00, sizeof(srFtpsObj.szCACertFilePath));
		memset(srFtpsObj.szCACertFileName, 0x00, sizeof(srFtpsObj.szCACertFileName));

	//	/* 設定CA憑證路徑 */
	//	if (strlen(_CA_DATA_PATH_) > 0)
	//	{
	//		strcat(srFtpsObj.szCACertFilePath, _CA_DATA_PATH_);
	//	}

		if (strlen(_PEM_NEW_TLS_FILE_NAME_) > 0)
		{
			strcat(srFtpsObj.szCACertFileName, _CA_DATA_PATH_);
			strcat(srFtpsObj.szCACertFileName, _PEM_NEW_TLS_FILE_NAME_);
			if (ginISODebug == VS_TRUE)
			{
				FILE* fp = fopen(srFtpsObj.szCACertFileName, "r");
				if (!fp) {
					inPRINT_ChineseFont("無法開啟憑證檔案",_PRT_ISO_);
					memset(szTemplate_1, 0x00, sizeof (szTemplate_1));
					snprintf(szTemplate_1, sizeof(szTemplate_1), " TMS PEM(1):%s ", srFtpsObj.szCACertFileName);
					inPRINT_ChineseFont(szTemplate_1, _PRT_ISO_);
				}else
				{

					// 1. 取得檔案總長度 (Total Length)
					fseek(fp, 0, SEEK_END);
					long file_size = ftell(fp);
					fclose(fp);
					
					memset(szTemplate_1, 0x00, sizeof (szTemplate_1));
					snprintf(szTemplate_1, sizeof(szTemplate_1), " TMS PEM(1):%s ", srFtpsObj.szCACertFileName);
					inPRINT_ChineseFont(szTemplate_1, _PRT_ISO_);
					memset(szTemplate_1, 0x00, sizeof (szTemplate_1));
					snprintf(szTemplate_1, sizeof(szTemplate_1), " PEM Size[%ld] ", file_size);
					inPRINT_ChineseFont(szTemplate_1, _PRT_ISO_);
				}
			}
		}

		inRetVal = inNCCCTMS_FTP_TermInfo_Disp_Flow(pobTran, &srFtpsObj);

		if (inRetVal == VS_SUCCESS)
		{
			break;
		}

		/* 使用舊憑證 */
		memset(srFtpsObj.szCACertFilePath, 0x00, sizeof(srFtpsObj.szCACertFilePath));
		memset(srFtpsObj.szCACertFileName, 0x00, sizeof(srFtpsObj.szCACertFileName));

//		/* 設定CA憑證路徑 */
//		if (strlen(_CA_DATA_PATH_) > 0)
//		{
//			strcat(srFtpsObj.szCACertFilePath, _CA_DATA_PATH_);
//		}

		if (strlen(_PEM_PRESERVE_TLS_FILE_NAME_) > 0)
		{
			strcat(srFtpsObj.szCACertFileName, _CA_DATA_PATH_);
			strcat(srFtpsObj.szCACertFileName, _PEM_PRESERVE_TLS_FILE_NAME_);
			if (ginISODebug == VS_TRUE)
			{
				FILE* fp = fopen(srFtpsObj.szCACertFileName, "r");
				if (!fp) {
					inPRINT_ChineseFont("無法開啟憑證檔案",_PRT_ISO_);
					memset(szTemplate_1, 0x00, sizeof (szTemplate_1));
					snprintf(szTemplate_1, sizeof(szTemplate_1), " TMS PEM(2):%s ", srFtpsObj.szCACertFileName);
					inPRINT_ChineseFont(szTemplate_1, _PRT_ISO_);
				}else
				{

					// 1. 取得檔案總長度 (Total Length)
					fseek(fp, 0, SEEK_END);
					long file_size = ftell(fp);
					fclose(fp);
					
					memset(szTemplate_1, 0x00, sizeof (szTemplate_1));
					snprintf(szTemplate_1, sizeof(szTemplate_1), " TMS PEM(2):%s ", srFtpsObj.szCACertFileName);
					inPRINT_ChineseFont(szTemplate_1, _PRT_ISO_);
					memset(szTemplate_1, 0x00, sizeof (szTemplate_1));
					snprintf(szTemplate_1, sizeof(szTemplate_1), " PEM Size[%ld] ", file_size);
					inPRINT_ChineseFont(szTemplate_1, _PRT_ISO_);
				}
			}
		}
		
		inRetVal = inNCCCTMS_FTP_TermInfo_Disp_Flow(pobTran, &srFtpsObj);
		
		
		break;
	}while(1);
	
	if (inRetVal != VS_SUCCESS)
	{
		/* TermInfo2下載失敗 */
		inSetFTPInquiryResponseCode(_FTP_INQUIRY_REPORT_TERMINFO_DOWNLOAD_FAILED_);
		inSaveTMSFTPRec(0);

		return (VS_ERROR);
	}
        
        /* 查核TermInfo2資料 儲存FileList */
        inRetVal = inNCCCTMS_FTPS_TermInfo_Analyze(pobTran);
        if (inRetVal != VS_SUCCESS)
        {
                return (VS_ERROR);
        }
	
	/* 手動下載和AP下載對是否更新AP的判斷不同 */
	if (pobTran->inTMSDwdMode == _TMS_AUTO_DOWNLOAD_)
	{
		/* 自動下載需要比對部份條件 */
		inRetVal = inNCCCTMS_FTPS_Auto_Download_Check(pobTran);
		if (inRetVal != VS_SUCCESS)
		{
			return (inRetVal);
		}
	}
	else
	{
		/* 手動下載一定不下AP */
		/* 手動下載拔掉APP那一行Record */
		inRetVal = inNCCCTMS_FTPS_Manual_Download_Check(pobTran);
		if (inRetVal != VS_SUCCESS)
		{
			return (inRetVal);
		}
	}
        
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCCTMS_FTPS_TermInfo_Download() END!");
        
        return (VS_SUCCESS);
}

int inNCCCTMS_FTPS_TermInfo_Analyze(TRANSACTION_OBJECT *pobTran)
{
        int		i, j, inOffset, inDataSize, inCnt;
        int		inRetVal;
        long		lnFTPFLTLength = 0;
        long		lnReadLength = 0;
        char		szFileName[26 + 1], szDispMsg[100 + 1];
        unsigned long   ulFile_Handle;
        unsigned char   *uszReadData;
        unsigned char   *uszTemp;
        VS_BOOL		fLeave = VS_FALSE, fDataErr = VS_FALSE;

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCCTMS_FTPS_TermInfo_Analyze() START!");
        
        /* 還沒圖片 先用字顯示 */
        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        inDISP_ChineseFont("檔案查核中", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);
        
        /* 初始化FTP Record */
        memset(&gsrFTP, 0x00, sizeof(FTP_OBJECT));
                
        /* 讀TermInfo2檔案 START */
        memset(szFileName, 0x00, sizeof(szFileName));
        strcpy(szFileName, _FTP_INFO_2_);
        
        if (inFILE_Open(&ulFile_Handle, (unsigned char *)szFileName) == VS_ERROR)
        {
                /* 開檔失敗 ，不用關檔 */
                /* 開檔失敗，所以回傳error */
                return (VS_ERROR);
        }

        /*
         * get data size 為了接下來動態宣告記憶體的大小需要知道多少 需先取得檔案Size
         * get size 失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        lnFTPFLTLength = lnFILE_GetSize(&ulFile_Handle, (unsigned char *)szFileName);
        
        if (lnFTPFLTLength == VS_ERROR)
        {
                /* GetSize失敗 ，關檔 */
                inFILE_Close(&ulFile_Handle);
                /* 刪除TermInfo2.txt */
                inFILE_Delete((unsigned char *)szFileName);

                return (VS_ERROR);
        }
        
        /*
         * allocate 記憶體
         * allocate時多分配一個byte以防萬一（ex:換行符號）
         */
        uszReadData = malloc(lnFTPFLTLength + 1);
        uszTemp = malloc(lnFTPFLTLength + 1);
        /* 初始化 uszTemp uszReadData */
        memset(uszReadData, 0x00, lnFTPFLTLength + 1);
        memset(uszTemp, 0x00, lnFTPFLTLength + 1);
        
        /* seek 到檔案開頭 & 從檔案開頭開始read */
        if (inFILE_Seek(ulFile_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnFTPFLTLength;

                for (i = 0;; ++i)
                {
                        /* 剩餘長度大於或等於1024 */
                        if (lnReadLength >= 1024)
                        {
                                if (inFILE_Read(&ulFile_Handle, &uszReadData[1024 * i], 1024) == VS_SUCCESS)
                                {
                                        /* 一次讀1024 */
                                        lnReadLength -= 1024;

                                        /* 當剩餘長度剛好為1024，會剛好讀完 */
                                        if (lnReadLength == 0)
                                                break;
                                }
                                /* 讀失敗時 */
                                else
                                {
                                        /* Close檔案 */
                                        inFILE_Close(&ulFile_Handle);
                                        /* 刪除TermInfo2.txt */
                                        inFILE_Delete((unsigned char *)szFileName);
                                        /* Free pointer */
                                        free(uszReadData);
                                        free(uszTemp);

                                        return (VS_ERROR);
                                }
                        }
                        /* 剩餘長度小於1024 */
                        else if (lnReadLength < 1024)
                        {
                                /* 就只讀剩餘長度 */
                                if (inFILE_Read(&ulFile_Handle, &uszReadData[1024 * i], lnReadLength) == VS_SUCCESS)
                                {
                                        break;
                                }
                                /* 讀失敗時 */
                                else
                                {
                                        /* Close檔案 */
                                        inFILE_Close(&ulFile_Handle);
                                        /* 刪除TermInfo2.txt */
                                        inFILE_Delete((unsigned char *)szFileName);
                                        /* Free pointer */
                                        free(uszReadData);
                                        free(uszTemp);

                                        return (VS_ERROR);
                                }
                        }
                }/* end for loop */
        }
        /* seek不成功時 */
        else
        {
                /* 關檔並回傳 */
                inFILE_Close(&ulFile_Handle);
                /* Free pointer */
                free(uszReadData);
                free(uszTemp);

                /* Seek失敗，所以回傳Error */
                return (VS_ERROR);
        }
        /* 讀TermInfo2檔案 END */
        
        /* 分析ReadData資料，第一行是Header */
        for (i = 0; i < lnFTPFLTLength; i ++)
        {
                if ((uszReadData[i] == 0x0D) && (uszReadData[i + 1] == 0x0A))
                        break;
                
                memcpy(&uszTemp[i], &uszReadData[i], 1);
        }

        /* 防呆 */
        gsrFTP.inHeader_length = strlen((char*)uszTemp);

        /* 目前Header那行最長94不含換行符號 */
        if (gsrFTP.inHeader_length > 100)
        {
                free(uszReadData);
                free(uszTemp);
                inFILE_Close(&ulFile_Handle);
                /* 刪除TermInfo2.txt */
                inFILE_Delete((unsigned char *)szFileName);
                /* 錯誤提示訊息 */
                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                inDISP_ChineseFont("檔案格式有誤", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);
                inDISP_BEEP(3, 500);
                return (VS_ERROR);
        }
        
        inOffset = 0;   /* 起點 */
        inCnt = 0;      /* 終點 */
        
	/* 分析Header */
        for (i = 1; i < 13; i ++)
        {
                inDataSize = 0;  /* 長度 */

                while (1)
                {
                        /* 逗號 */
                        if ((uszTemp[inCnt] == 0x2C) || (uszTemp[inCnt] == 0x00))
                        {
                                inCnt ++;
                                break;
                        }
			else
			{
				inCnt ++;
				inDataSize ++;
			}

                        /* 防呆 */
                        if (inCnt > gsrFTP.inHeader_length)
                        {
                                free(uszReadData);
                                free(uszTemp);
                                inFILE_Close(&ulFile_Handle);
                                /* 刪除TermInfo2.txt */
                                inFILE_Delete((unsigned char *)szFileName);
                                /* 錯誤提示訊息 */
                                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                                inDISP_ChineseFont("檔案格式有誤", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);
                                inDISP_BEEP(3, 500);

                                return (VS_ERROR);
                        }
                }

                switch (i)
                {
                        case 1:
                                memcpy(&gsrFTP.szIndex[0], &uszTemp[inOffset], inDataSize);
                                
                                if (ginDebug == VS_TRUE)
                                {
                                        memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                        sprintf(szDispMsg, "Index      [%02d][%s]", strlen(gsrFTP.szIndex), gsrFTP.szIndex);
                                        inLogPrintf(AT, szDispMsg);
                                }
				if (ginISODebug == VS_TRUE)
				{
					memset(szDispMsg, 0x00, sizeof(szDispMsg));
					sprintf(szDispMsg, "Index      [%02d][%s]", strlen(gsrFTP.szIndex), gsrFTP.szIndex);
					inPRINT_ChineseFont(szDispMsg, _PRT_ISO_);
				}
				
                                if (inDataSize != 2)
                                        fDataErr = VS_TRUE;

                                break;
                        case 2:
                                memcpy(&gsrFTP.szFile_Attribute[0], &uszTemp[inOffset], inDataSize);

				if (ginDebug == VS_TRUE)
                                {
                                        memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                        sprintf(szDispMsg, "Attribute  [%02d][%s]", strlen(gsrFTP.szFile_Attribute), gsrFTP.szFile_Attribute);
                                        inLogPrintf(AT, szDispMsg);
                                }
                                if (ginISODebug == VS_TRUE)
				{
					memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                        sprintf(szDispMsg, "Attribute  [%02d][%s]", strlen(gsrFTP.szFile_Attribute), gsrFTP.szFile_Attribute);
					inPRINT_ChineseFont(szDispMsg, _PRT_ISO_);
				}
				
                                if (inDataSize != 1)
                                        fDataErr = VS_TRUE;

                                break;
                        case 3:
                                memcpy(&gsrFTP.szHeader_TID[0], &uszTemp[inOffset], inDataSize);
                                
				if (ginDebug == VS_TRUE)
                                {
                                        memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                        sprintf(szDispMsg, "TID        [%02d][%s]", strlen(gsrFTP.szHeader_TID), gsrFTP.szHeader_TID);
                                        inLogPrintf(AT, szDispMsg);
                                }
                                if (ginISODebug == VS_TRUE)
				{
					memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                        sprintf(szDispMsg, "TID        [%02d][%s]", strlen(gsrFTP.szHeader_TID), gsrFTP.szHeader_TID);
					inPRINT_ChineseFont(szDispMsg, _PRT_ISO_);
				}
				
                                break;
                        case 4:
                                memcpy(&gsrFTP.szHeader_MID[0], &uszTemp[inOffset], inDataSize);
                                
				if (ginDebug == VS_TRUE)
                                {
                                        memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                        sprintf(szDispMsg, "MID        [%02d][%s]", strlen(gsrFTP.szHeader_MID), gsrFTP.szHeader_MID);
                                        inLogPrintf(AT, szDispMsg);
                                }
                                if (ginISODebug == VS_TRUE)
				{
					memset(szDispMsg, 0x00, sizeof(szDispMsg));
					sprintf(szDispMsg, "MID        [%02d][%s]", strlen(gsrFTP.szHeader_MID), gsrFTP.szHeader_MID);
					inPRINT_ChineseFont(szDispMsg, _PRT_ISO_);
				}
				
                                break;
                        case 5:
                                memcpy(&gsrFTP.szHeader_MFES_ID[0], &uszTemp[inOffset], inDataSize);
                                
				if (ginDebug == VS_TRUE)
                                {
                                        memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                        sprintf(szDispMsg, "MFES_ID    [%02d][%s]", strlen(gsrFTP.szHeader_MFES_ID), gsrFTP.szHeader_MFES_ID);
                                        inLogPrintf(AT, szDispMsg);
                                }
				if (ginISODebug == VS_TRUE)
				{
					memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                        sprintf(szDispMsg, "MFES_ID    [%02d][%s]", strlen(gsrFTP.szHeader_MFES_ID), gsrFTP.szHeader_MFES_ID);
					inPRINT_ChineseFont(szDispMsg, _PRT_ISO_);
				}
				
                                if (inDataSize != 4)
                                        fDataErr = VS_TRUE;
                                
                                break;
                        case 6:
                                memcpy(&gsrFTP.szHeader_AutoDownloadFlag[0], &uszTemp[inOffset], inDataSize);
                                
				if (ginDebug == VS_TRUE)
                                {
                                        memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                        sprintf(szDispMsg, "AutoDL     [%02d][%s]", strlen(gsrFTP.szHeader_AutoDownloadFlag), gsrFTP.szHeader_AutoDownloadFlag);
                                        inLogPrintf(AT, szDispMsg);
                                }
				if (ginISODebug == VS_TRUE)
				{
					memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                        sprintf(szDispMsg, "AutoDL     [%02d][%s]", strlen(gsrFTP.szHeader_AutoDownloadFlag), gsrFTP.szHeader_AutoDownloadFlag);
					inPRINT_ChineseFont(szDispMsg, _PRT_ISO_);
				}
				
                                if (inDataSize != 1)
                                        fDataErr = VS_TRUE;
                                
                                break;
                        case 7:
                                memcpy(&gsrFTP.szHeader_StartDownloadDate[0], &uszTemp[inOffset], inDataSize);
                                
				if (ginDebug == VS_TRUE)
                                {
                                        memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                        sprintf(szDispMsg, "StartDL    [%02d][%s]", strlen(gsrFTP.szHeader_StartDownloadDate), gsrFTP.szHeader_StartDownloadDate);
                                        inLogPrintf(AT, szDispMsg);
                                }
				if (ginISODebug == VS_TRUE)
				{
					memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                        sprintf(szDispMsg, "StartDL    [%02d][%s]", strlen(gsrFTP.szHeader_StartDownloadDate), gsrFTP.szHeader_StartDownloadDate);
					inPRINT_ChineseFont(szDispMsg, _PRT_ISO_);
				}
				
                                if (inDataSize != 12)
                                        fDataErr = VS_TRUE;
                                
                                break;
                        case 8:
                                memcpy(&gsrFTP.szHeader_TermApVersion[0], &uszTemp[inOffset], inDataSize);
                                
				if (ginDebug == VS_TRUE)
                                {
                                        memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                        sprintf(szDispMsg, "TermApVer  [%02d][%s]", strlen(gsrFTP.szHeader_TermApVersion), gsrFTP.szHeader_TermApVersion);
                                        inLogPrintf(AT, szDispMsg);
                                }
				if (ginISODebug == VS_TRUE)
				{
					memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                        sprintf(szDispMsg, "TermApVer  [%02d][%s]", strlen(gsrFTP.szHeader_TermApVersion), gsrFTP.szHeader_TermApVersion);
					inPRINT_ChineseFont(szDispMsg, _PRT_ISO_);
				}
				
                                /* 現在採用寫死的方式，不存成參數了 */
                                break;
                        case 9:
                                memcpy(&gsrFTP.szHeader_TermApVersionDate[0], &uszTemp[inOffset], inDataSize);
                                
				if (ginDebug == VS_TRUE)
				{
					memset(szDispMsg, 0x00, sizeof(szDispMsg));
					sprintf(szDispMsg, "TermApVerD [%02d][%s]", strlen(gsrFTP.szHeader_TermApVersionDate), gsrFTP.szHeader_TermApVersionDate);
					inLogPrintf(AT, szDispMsg);
				}
				if (ginISODebug == VS_TRUE)
				{
					memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                        sprintf(szDispMsg, "TermApVerD [%02d][%s]", strlen(gsrFTP.szHeader_TermApVersionDate), gsrFTP.szHeader_TermApVersionDate);
					inPRINT_ChineseFont(szDispMsg, _PRT_ISO_);
				}
				
				/* 現在採用寫死的方式，不存成參數了 */
                                break;
                        case 10:
                                memcpy(&gsrFTP.szHeader_TermParmDateTime[0], &uszTemp[inOffset], inDataSize);

                                if (ginDebug == VS_TRUE)
                                {
                                        memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                        sprintf(szDispMsg, "TermParmDT [%02d][%s]", strlen(gsrFTP.szHeader_TermParmDateTime), gsrFTP.szHeader_TermParmDateTime);
                                        inLogPrintf(AT, szDispMsg);
                                }
                                if (ginISODebug == VS_TRUE)
				{
					memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                        sprintf(szDispMsg, "TermParmDT [%02d][%s]", strlen(gsrFTP.szHeader_TermParmDateTime), gsrFTP.szHeader_TermParmDateTime);
					inPRINT_ChineseFont(szDispMsg, _PRT_ISO_);
				}
				
                                if (inDataSize != 12)
				{
                                        fDataErr = VS_TRUE;
				}
				else
				{
					
				}
                                
                                break;
                        case 11:
                                memcpy(&gsrFTP.szHeader_CloseBatchFlag[0], &uszTemp[inOffset], inDataSize);

                                if (ginDebug == VS_TRUE)
                                {
                                        memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                        sprintf(szDispMsg, "CloseBatch [%02d][%s]", strlen(gsrFTP.szHeader_CloseBatchFlag), gsrFTP.szHeader_CloseBatchFlag);
                                        inLogPrintf(AT, szDispMsg);
                                }
                                if (ginISODebug == VS_TRUE)
				{
					memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                        sprintf(szDispMsg, "CloseBatch [%02d][%s]", strlen(gsrFTP.szHeader_CloseBatchFlag), gsrFTP.szHeader_CloseBatchFlag);
					inPRINT_ChineseFont(szDispMsg, _PRT_ISO_);
				}
				
                                if (inDataSize != 1)
				{
                                        fDataErr = VS_TRUE;
				}
				else
				{
					inSetFTPEffectiveCloseBatch(gsrFTP.szHeader_CloseBatchFlag);
				}

                                break;
                        case 12:
                                memcpy(&gsrFTP.szHeader_BatchNumber[0], &uszTemp[inOffset], inDataSize);

                                if (ginDebug == VS_TRUE)
                                {
                                        memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                        sprintf(szDispMsg, "BatchNum   [%02d][%s]", strlen(gsrFTP.szHeader_BatchNumber), gsrFTP.szHeader_BatchNumber);
                                        inLogPrintf(AT, szDispMsg);
                                }
				if (ginISODebug == VS_TRUE)
				{
					memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                        sprintf(szDispMsg, "BatchNum   [%02d][%s]", strlen(gsrFTP.szHeader_BatchNumber), gsrFTP.szHeader_BatchNumber);
					inPRINT_ChineseFont(szDispMsg, _PRT_ISO_);
				}
                                
                                if (inDataSize != 8)
				{
                                        fDataErr = VS_TRUE;
				}
				else
				{
					inSetFTPBatchNum(gsrFTP.szHeader_BatchNumber);
				}
                                
                                break;
                        default:
                                fLeave = VS_TRUE;
                                break;

                }
                
                inOffset = inCnt;

                if (fDataErr == VS_TRUE)
                {
                        free(uszReadData);
                        free(uszTemp);
                        inFILE_Close(&ulFile_Handle);
                        /* 刪除TermInfo2.txt */
                        inFILE_Delete((unsigned char *)szFileName);
                        /* 錯誤提示訊息 */
                        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                        inDISP_ChineseFont("檔案資料有誤", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
			inDISP_ChineseFont(szFileName, _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
                        inDISP_BEEP(3, 500);
                        
                        return (VS_ERROR);
                }

                if (fLeave == VS_TRUE)
                        break;
        }
	
	/* 一次設定完再存 */
	inSaveTMSFTPRec(0);
        
        /* 檢查TermInfo2.txt內，TID MID是否相同 */
        inRetVal = inNCCCTMS_FTPS_Check_TID_MID(pobTran);
        if (inRetVal != VS_SUCCESS)
        {
                free(uszReadData);
                free(uszTemp);
                inFILE_Close(&ulFile_Handle);
                /* 刪除TermInfo2.txt */
                inFILE_Delete((unsigned char *)szFileName);
		
		/* 檢核TID、MID不符合 */
		inSetFTPInquiryResponseCode(_FTP_INQUIRY_REPORT_TID_MID_NOT_MATCHED_);
		inSetFTPDownloadResponseCode(_FTP_DOWNLOAD_REPORT_TID_MID_NOT_MATCHED_);
		inSaveTMSFTPRec(0);
				
                return (VS_ERROR);
        }
        
	/* 這裡開始處理要下載的List */
        /* 計算TermInfo2.txt內要下載的檔案總數 */
        memset(uszTemp, 0x00, lnFTPFLTLength + 1);
        
        for (i = 0; i < lnFTPFLTLength; i ++)
        {
                if ((uszReadData[i] == 0x0A) && (uszReadData[i - 1] == 0x0D))
                {
                        i = i + 1;
                        lnFTPFLTLength = lnFTPFLTLength - i;
                        break;
                }
        }
        
        for (j = 0; j < lnFTPFLTLength; j ++)
        {
                memcpy(&uszTemp[j], &uszReadData[i + j], 1);
                
                if (uszReadData[i + j] == 0x00)
                        break;
        }
        
        /* 先將要下載的FileList存成FTPFLT.dat */
        inFILE_Delete((unsigned char *)_FTPFLT_FILE_NAME_);
        inFILE_Close(&ulFile_Handle);
        
        if (inFILE_Create(&ulFile_Handle, (unsigned char *)_FTPFLT_FILE_NAME_) == VS_ERROR)
        {
                free(uszReadData);
                free(uszTemp);
                inFILE_Close(&ulFile_Handle);
                /* 刪除TermInfo2.txt */
                inFILE_Delete((unsigned char *)szFileName);
                return (VS_ERROR);
        }
        
        /* 寫檔 將uszTemp存到FTPFLT.dat */
        inRetVal = inFILE_Write(&ulFile_Handle, &uszTemp[0], lnFTPFLTLength);
        
        if (inRetVal != VS_SUCCESS)
        {
                free(uszReadData);
                free(uszTemp);
                inFILE_Close(&ulFile_Handle);
                /* 刪除TermInfo2.txt */
                inFILE_Delete((unsigned char *)szFileName);
                /* 刪除FTPFLT.dat */
                inFILE_Delete((unsigned char *)_FTPFLT_FILE_NAME_);
                return (VS_ERROR);
        }
        
        free(uszReadData);
        free(uszTemp);
        inFILE_Close(&ulFile_Handle);
	
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCCTMS_FTPS_TermInfo_Analyze() END!");
        
        return (VS_SUCCESS);
}

/*
Function        :inNCCCTMS_FTPS_Auto_Download_Check
Date&Time       :2018/6/4 下午 4:18
Describe        :自動下載要檢核Header
*/
int inNCCCTMS_FTPS_Auto_Download_Check(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_ERROR;
	int		inYear = 0;
	int		inMonth = 0;
	int		inDay = 0;
	int		inHour = 0;
	int		inMinute = 0;
	char		szTimeTemp[4 + 1] = {0};
	char		szTermParameterDateTime[12 + 1] = {0};
	char		szI_FES_Mode[1 + 1] = {0};
	unsigned char	uszDeleteAP = VS_FALSE;		/* 是否要刪掉AP那一行 */
	unsigned char	uszNotUpdateByDate = VS_FALSE;	/* 是否因下載日期未到，而不用下載 */
	RTC_NEXSYS	srRTC;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCCTMS_FTPS_Auto_Download_Check() START !");
	}
	
	if (pobTran->inTMSDwdMode != _TMS_AUTO_DOWNLOAD_)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "Not TMS Download, Not Checking");
		}
		return (VS_SUCCESS);
	}
	
	/* ======================================================================================================================
	   2.比對TermInfo2.txt 的 TermAPVersion 與 EDC 的 TermAPVersion 是否相同(決定是否下AP)
	                          TermVersionDate 與 EDC 的 TermVersionDate 是否相同(決定是否下AP)
	     都相同   : 不用下載AP，檢核TermParmDateTime
	     一個不同 : 檢核AutoDownloadFlag
	====================================================================================================================== */
	memset(szI_FES_Mode, 0x00, sizeof(szI_FES_Mode));
	inGetI_FES_Mode(szI_FES_Mode);
	if (!memcmp(szI_FES_Mode, "Y", 1))
	{
		if ((memcmp(gszTermVersionID, gsrFTP.szHeader_TermApVersion, strlen(gszTermVersionID)) == 0)	&&
		    (atol(gszTermVersionDate) >= atol(gsrFTP.szHeader_TermApVersionDate)))
		{
			uszDeleteAP = VS_TRUE;		/* 不需要下載 AP */

			/* ==============================================================================================================
			   3.比對TermInfo.txt的 TermParmDateTime 與 EDC 的 TermParmDateTime 是否相同(決定是否下Param)
			     相同 : 回報 MFES 本次詢問結果為 不需下載
			     不同 : 檢核AutoDownloadFlag
			============================================================================================================== */
			memset(szTermParameterDateTime, 0x00, sizeof(szTermParameterDateTime));
			inGetFTPTermParemeterDateTime(szTermParameterDateTime);
			if (memcmp(szTermParameterDateTime, gsrFTP.szHeader_TermParmDateTime, strlen(gsrFTP.szHeader_TermParmDateTime))  == 0)
			{
				/* 無須下載，刪除FileList */
				inFILE_Delete((unsigned char *)_FTPFLT_FILE_NAME_);
				inSetFTPInquiryResponseCode(_FTP_INQUIRY_REPORT_NO_DOWNLOAD_);
				inSaveTMSFTPRec(0);

				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "Same ParameterDateTime");
				}

				return (VS_ERROR);
			}


		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "Not Same APVersion Aand APVersion Date");
				inLogPrintf(AT, "Version New:%s OLD:%s", gszTermVersionID, gsrFTP.szHeader_TermApVersion);
				inLogPrintf(AT, "Date New:%s OLD:%s", gszTermVersionDate, gsrFTP.szHeader_TermApVersionDate);
			}
			/* 相同，繼續判斷AutoDownloadFlag */
		}
	}
	else
	{
		if (memcmp(gszTermVersionID, gsrFTP.szHeader_TermApVersion, strlen(gszTermVersionID)) == 0	&&
		    memcmp(gszTermVersionDate, gsrFTP.szHeader_TermApVersionDate, 12) == 0)
		{
			uszDeleteAP = VS_TRUE;		/* 不需要下載 AP */

			/* ==============================================================================================================
			   3.比對TermInfo.txt的 TermParmDateTime 與 EDC 的 TermParmDateTime 是否相同(決定是否下Param)
			     相同 : 回報 MFES 本次詢問結果為 不需下載
			     不同 : 檢核AutoDownloadFlag
			============================================================================================================== */
			memset(szTermParameterDateTime, 0x00, sizeof(szTermParameterDateTime));
			inGetFTPTermParemeterDateTime(szTermParameterDateTime);
			if (memcmp(szTermParameterDateTime, gsrFTP.szHeader_TermParmDateTime, strlen(gsrFTP.szHeader_TermParmDateTime))  == 0)
			{
				/* 無須下載，刪除FileList */
				inFILE_Delete((unsigned char *)_FTPFLT_FILE_NAME_);
				inSetFTPInquiryResponseCode(_FTP_INQUIRY_REPORT_NO_DOWNLOAD_);
				inSaveTMSFTPRec(0);

				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "Same ParameterDateTime");
				}

				return (VS_ERROR);
			}


		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "Not Same APVersion Aand APVersion Date");
				inLogPrintf(AT, "Version New:%s OLD:%s", gszTermVersionID, gsrFTP.szHeader_TermApVersion);
				inLogPrintf(AT, "Date New:%s OLD:%s", gszTermVersionDate, gsrFTP.szHeader_TermApVersionDate);
			}
			/* 相同，繼續判斷AutoDownloadFlag */
		}
	}
	
	/* ======================================================================================================================
	   4.比對 AutoDownloadFlag
             AutoDownloadFlag == Y : 須下載AP and Param
             AutoDownloadFlag == N : 回報 MFES 本次詢問結果為 不需下載
	   =================================================================================================================== */
	if (memcmp(gsrFTP.szHeader_AutoDownloadFlag, "Y", strlen("Y")) != 0)
	{
		/* 無須下載，刪除FileList */
		inFILE_Delete((unsigned char *)_FTPFLT_FILE_NAME_);
		inSetFTPInquiryResponseCode(_FTP_INQUIRY_REPORT_NO_DOWNLOAD_);
		inSaveTMSFTPRec(0);
		
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "AutoDownloadFlag Not 'Y'");
		}
		
		return (VS_ERROR);
	}
	
	/* =====================================================================================================================
	   5.比對 StartDownloadDate 與 EDC 的時間(此參數類似生效日功能)

	     (1)StartDownloadDate == "        " : 回報 MFES 本次詢問結果為 不需下載
	     (2)StartDownloadDate >  EDC時間    : 回報 MFES 本次詢問結果為 不需下載
             (3)StartDownloadDate <= EDC時間    : 須下載AP and Param
	===================================================================================================================== */
	memset(&srRTC, 0x0, sizeof(srRTC));
	inFunc_GetSystemDateAndTime(&srRTC);
	
	if (memcmp(gsrFTP.szHeader_StartDownloadDate, "            ", strlen("            ")) == 0)
	{
		/* 無須下載，刪除FileList */
		inFILE_Delete((unsigned char *)_FTPFLT_FILE_NAME_);
		inSetFTPInquiryResponseCode(_FTP_INQUIRY_REPORT_NO_DOWNLOAD_);
		inSaveTMSFTPRec(0);
		
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "StartDownloadDate = space");
		}
		
		return (VS_ERROR);
	}
	else
	{
		/* 年份只取後兩碼 */
		memset(szTimeTemp, 0x00, sizeof(szTimeTemp));
		memcpy(szTimeTemp, &gsrFTP.szHeader_StartDownloadDate[2], 2);
		inYear = atoi(szTimeTemp);
		
		memset(szTimeTemp, 0x00, sizeof(szTimeTemp));
		memcpy(szTimeTemp, &gsrFTP.szHeader_StartDownloadDate[4], 2);
		inMonth = atoi(szTimeTemp);
		
		memset(szTimeTemp, 0x00, sizeof(szTimeTemp));
		memcpy(szTimeTemp, &gsrFTP.szHeader_StartDownloadDate[6], 2);
		inDay = atoi(szTimeTemp);
		
		memset(szTimeTemp, 0x00, sizeof(szTimeTemp));
		memcpy(szTimeTemp, &gsrFTP.szHeader_StartDownloadDate[8], 2);
		inHour = atoi(szTimeTemp);
		
		memset(szTimeTemp, 0x00, sizeof(szTimeTemp));
		memcpy(szTimeTemp, &gsrFTP.szHeader_StartDownloadDate[10], 2);
		inMinute = atoi(szTimeTemp);
		
		if (srRTC.uszYear < inYear)
		{
			uszNotUpdateByDate = VS_TRUE;
		}
		else if (srRTC.uszYear == inYear)
		{
			if (srRTC.uszMonth < inMonth)
			{
				uszNotUpdateByDate = VS_TRUE;
			}
			else if (srRTC.uszMonth == inMonth)
			{
				if (srRTC.uszDay < inDay)
				{
					uszNotUpdateByDate = VS_TRUE;
				}
				else if (srRTC.uszDay == inDay)
				{
					if (srRTC.uszHour < inHour)
					{
						uszNotUpdateByDate = VS_TRUE;
					}
					else if (srRTC.uszHour == inHour)
					{
						if (srRTC.uszMinute < inMinute)
						{
							uszNotUpdateByDate = VS_TRUE;
						}
						else if (srRTC.uszMinute == inMinute)
						{
							/* 時間到了，要下載 */
						}
					}
				}
			}
		}
		
		if (uszNotUpdateByDate == VS_TRUE)
		{
			/* 無須下載，刪除FileList */
			inFILE_Delete((unsigned char *)_FTPFLT_FILE_NAME_);
			inSetFTPInquiryResponseCode(_FTP_INQUIRY_REPORT_NO_DOWNLOAD_);
			inSaveTMSFTPRec(0);

			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "StartDownloadDate > EDC Date");
			}

			return (VS_ERROR);
		}
	}
	
	if (uszDeleteAP == VS_TRUE)
	{
		/* 刪除A那一行 */
		inRetVal = inFILE_Copy_File((unsigned char*)_FTPFLT_FILE_NAME_, (unsigned char*)_FTPFLT_FILE_NAME_BAK_);
		if (inRetVal != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		
		
		inRetVal = inNCCCTMS_FTPFLT_Delete_AP_Attribute_A(_FTPFLT_FILE_NAME_BAK_);
		if (inRetVal != VS_SUCCESS)
		{
			inFILE_Delete((unsigned char*)_FTPFLT_FILE_NAME_BAK_);
		}
		else
		{
			inFILE_Delete((unsigned char*)_FTPFLT_FILE_NAME_);
			inFILE_Rename((unsigned char*)_FTPFLT_FILE_NAME_BAK_, (unsigned char*)_FTPFLT_FILE_NAME_);
			
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "FTP FileList Attribute A Delete OK");
			}
		}
		
		/* 刪除R那一行 */
		inRetVal = inFILE_Copy_File((unsigned char*)_FTPFLT_FILE_NAME_, (unsigned char*)_FTPFLT_FILE_NAME_BAK_);
		if (inRetVal != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		
		inRetVal = inNCCCTMS_FTPFLT_Delete_AP_Attribute_R(_FTPFLT_FILE_NAME_BAK_);
		if (inRetVal != VS_SUCCESS)
		{
			inFILE_Delete((unsigned char*)_FTPFLT_FILE_NAME_BAK_);
		}
		else
		{
			inFILE_Delete((unsigned char*)_FTPFLT_FILE_NAME_);
			inFILE_Rename((unsigned char*)_FTPFLT_FILE_NAME_BAK_, (unsigned char*)_FTPFLT_FILE_NAME_);
			
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "FTP FileList Attribute R Delete OK");
			}
		}
		
		/* 參數下載 */
		inSetFTPDownloadCategory(_FTP_DOWNLOAD_CATEGORY_PARAMETER_);
		inSetFTPInquiryResponseCode(_FTP_INQUIRY_REPORT_PARAMETER_DOWNLOAD_);
		inSaveTMSFTPRec(0);
	}
	else
	{
		/* AP下載 */
		inSetFTPDownloadCategory(_FTP_DOWNLOAD_CATEGORY_AP_);
		inSetFTPInquiryResponseCode(_FTP_INQUIRY_REPORT_AP_DOWNLOAD_);
		inSaveTMSFTPRec(0);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCCTMS_FTPS_Auto_Download_Check() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCCTMS_FTPS_Manual_Download_Check
Date&Time       :2018/6/26 下午 3:11
Describe        :手動下載在這邊拔掉AP那一行
*/
int inNCCCTMS_FTPS_Manual_Download_Check(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_ERROR;
	unsigned char	uszDeleteAP = VS_FALSE;		/* 是否要刪掉AP那一行 */
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCCTMS_FTPS_Manual_Download_Check() START !");
	}
	
	/* 手動下載一定不下AP */
	uszDeleteAP = VS_TRUE;
	
	if (uszDeleteAP == VS_TRUE)
	{
		/* 刪除A那一行 */
		inRetVal = inFILE_Copy_File((unsigned char*)_FTPFLT_FILE_NAME_, (unsigned char*)_FTPFLT_FILE_NAME_BAK_);
		if (inRetVal != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		
		inRetVal = inNCCCTMS_FTPFLT_Delete_AP_Attribute_A(_FTPFLT_FILE_NAME_BAK_);
		if (inRetVal != VS_SUCCESS)
		{
			inFILE_Delete((unsigned char*)_FTPFLT_FILE_NAME_BAK_);
		}
		else
		{
			inFILE_Delete((unsigned char*)_FTPFLT_FILE_NAME_);
			inFILE_Rename((unsigned char*)_FTPFLT_FILE_NAME_BAK_, (unsigned char*)_FTPFLT_FILE_NAME_);
			
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "FTP FileList Attribute A Delete OK");
			}
		}
		
		/* 刪除R那一行 */
		inRetVal = inFILE_Copy_File((unsigned char*)_FTPFLT_FILE_NAME_, (unsigned char*)_FTPFLT_FILE_NAME_BAK_);
		if (inRetVal != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		
		inRetVal = inNCCCTMS_FTPFLT_Delete_AP_Attribute_R(_FTPFLT_FILE_NAME_BAK_);
		if (inRetVal != VS_SUCCESS)
		{
			inFILE_Delete((unsigned char*)_FTPFLT_FILE_NAME_BAK_);
		}
		else
		{
			inFILE_Delete((unsigned char*)_FTPFLT_FILE_NAME_);
			inFILE_Rename((unsigned char*)_FTPFLT_FILE_NAME_BAK_, (unsigned char*)_FTPFLT_FILE_NAME_);
			
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "FTP FileList Attribute R Delete OK");
			}
		}
		
		/* 手動下載一定不下AP */
		inSetFTPDownloadCategory(_FTP_DOWNLOAD_CATEGORY_PARAMETER_);
		inSetFTPInquiryResponseCode(_FTP_INQUIRY_REPORT_PARAMETER_DOWNLOAD_);
		inSaveTMSFTPRec(0);
	}
	else
	{
		/* AP下載 */
		inSetFTPDownloadCategory(_FTP_DOWNLOAD_CATEGORY_AP_);
		inSetFTPInquiryResponseCode(_FTP_INQUIRY_REPORT_AP_DOWNLOAD_);
		inSaveTMSFTPRec(0);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCCTMS_FTPS_Manual_Download_Check() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

int inNCCCTMS_FTPS_Check_TID_MID(TRANSACTION_OBJECT *pobTran)
{
        char    szTID[8 + 1];
        char    szMID[15 + 1];

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCCTMS_FTPS_Check_TID_MID() START!");
        
        if (inLoadHDTRec(0) < 0)
                return (VS_ERROR);
	if (inLoadHDPTRec(0) < 0)
                return (VS_ERROR);
        
        memset(szTID, 0x00, sizeof(szTID));
        inGetTerminalID(szTID);
        
        memset(szMID, 0x00, sizeof(szMID));
        inGetMerchantID(szMID);
        
        /* 比對是否相同 不同的話提示錯誤 */
        if (memcmp(gsrFTP.szHeader_TID, szTID, strlen(szTID)))
        {
                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                inDISP_ChineseFont("檔案格式有誤", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
		inDISP_ChineseFont("TID不相同", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
                inDISP_BEEP(3, 500);
                return (VS_ERROR);
        }
        
        if (memcmp(gsrFTP.szHeader_MID, szMID, strlen(szMID)))
        {
                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                inDISP_ChineseFont("檔案格式有誤", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
		inDISP_ChineseFont("MID不相同", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
                inDISP_BEEP(3, 500);
                return (VS_ERROR);
        }
        
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCCTMS_FTPS_Check_TID_MID() END!");
        
        return (VS_SUCCESS);
}

int inNCCCTMS_FTPS_APPARM_Download(TRANSACTION_OBJECT *pobTran)
{
        int		i = 0, inFTPPort = 0, inRetVal = VS_ERROR;
        char		szTemplate[128 + 1] = {0};
        char		szFTPFileAttribute[2 + 1] = {0};	/* 檔案屬性 */
        char		szFTPFilePath[60 + 1] = {0};		/* 檔案路徑 */
        char		szFTPFileName[26 + 1] = {0};		/* 檔案名稱 */
        char		szFTPFileSize[10 + 1] = {0};		/* 檔案大小 */
        char		szFTPIPAddress[16 + 1] = {0};		/* FTP IP Address */
        char		szFTPPortNum[6 + 1] = {0};		/* FTP Port Number */
        char		szFTPID[20 + 1] = {0};			/* FTP ID */
        char		szFTPPW[20 + 1] = {0};			/* FTP PW */
	char		szIFESMode[2 + 1] = {0};		/* IFES Mode */
	char		szCFESMode[2 + 1] = {0};
	char		szDemoMode[2 + 1] = {0};
        FTPS_REC        srFtpsObj;

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCCTMS_FTPS_APPARM_Download() START!");
	
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
        
        /* 取得FTPS需要用的IP PORT ID PW */
        memset(&srFtpsObj, 0x00, sizeof(srFtpsObj));
        memset(szFTPIPAddress, 0x00, sizeof(szFTPIPAddress));
        inGetTMS_IP_Primary(szFTPIPAddress);
        memset(szFTPPortNum, 0x00, sizeof(szFTPPortNum));
        inGetFTPPortNum(szFTPPortNum);
        memset(szFTPID, 0x00, sizeof(szFTPID));
        inGetFTPID(szFTPID);
        memset(szFTPPW, 0x00, sizeof(szFTPPW));
        inGetFTPPW(szFTPPW);
        
        /* FTPS Port */
        inFTPPort = atoi(szFTPPortNum);
        srFtpsObj.inFtpsPort = inFTPPort;
        
        /* FTPS ID */
        strcpy(srFtpsObj.szFtpsID, szFTPID);
        
        /* FTPS PW */
        strcpy(srFtpsObj.szFtpsPW, szFTPPW);
        
        /* 下載檔案 用Load FTPFLT Record 來下載檔案 */
        for (i = 0 ;; i ++)
        {       
                if (inLoadFTPFLTRec(i) < 0)
                        break;
                
                memset(szFTPFileAttribute, 0x00, sizeof(szFTPFileAttribute));
                inGetFTPFileAttribute(szFTPFileAttribute);
                memset(szFTPFilePath, 0x00, sizeof(szFTPFilePath));
                inGetFTPFilePath(szFTPFilePath);
                memset(szFTPFileName, 0x00, sizeof(szFTPFileName));
                inGetFTPFileName(szFTPFileName);
                memset(szFTPFileSize, 0x00, sizeof(szFTPFileSize));
                inGetFTPFileSize(szFTPFileSize);
                
                /* 還沒圖片 先用字顯示 */
                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                inDISP_ChineseFont("檔案下載中", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_LEFT_);
        
		/* 下載的檔案名 */
		memset(srFtpsObj.szFtpsFileName, 0x00, sizeof(srFtpsObj.szFtpsFileName));
		strcpy(srFtpsObj.szFtpsFileName, szFTPFileName);
			
                /* 組FTPS URL */
		/* IFES要用外顯式explicit，先走ftp再讓他自己轉 */
		/* By http://www.it1352.com/589266.html
		 * curl显然尝试使用隐式FTP（因为它甚至在任何FTP命令之前初始化TLS/SSL会话，这是因为您指定了 ftps：// 前缀，用于隐式TLS。
		 * 它有特殊的前缀，因为隐式TLS使用特殊端口（990）。但是你用 CURLOPT_PORT 覆盖默认值。
		 * 显式TLS使用标准FTP端口它使用标准的 ftp：// 前缀。要启用显式TLS，请使用  CURLOPT_USE_SSL  （你正在做什么，只是通过错误的值，选项类型是枚举，而不是布尔值）。
		 *  */
		memset(szIFESMode, 0x00, sizeof(szIFESMode));
		inGetI_FES_Mode(szIFESMode);
		memset(szCFESMode, 0x00, sizeof(szCFESMode));
		inGetCloud_MFES(szCFESMode);
		if (memcmp(szCFESMode, "Y", strlen("Y")) == 0	||
		    pobTran->uszFTP_TMS_Download == _TMS_DOWNLOAD_SECURE_MODE_SFTP_)
		{
			/* 沒有找到原因，但CURL使用SFTP必須將使用者帳戶放到URL的前面 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(srFtpsObj.szFtpsURL, 0x00, sizeof(srFtpsObj.szFtpsURL));
			sprintf(szTemplate, "sftp://%s@%s/u01/users/tmsap%s/%s", srFtpsObj.szFtpsID, szFTPIPAddress, szFTPFilePath, szFTPFileName);
			strcpy(srFtpsObj.szFtpsURL, szTemplate);
		}
		else if (memcmp(szIFESMode, "Y", strlen("Y")) == 0)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(srFtpsObj.szFtpsURL, 0x00, sizeof(srFtpsObj.szFtpsURL));
			sprintf(szTemplate, "ftp://%s/u01/users/tmsap%s/%s", szFTPIPAddress, szFTPFilePath, szFTPFileName);
			strcpy(srFtpsObj.szFtpsURL, szTemplate);
		}
		else
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(srFtpsObj.szFtpsURL, 0x00, sizeof(srFtpsObj.szFtpsURL));
			sprintf(szTemplate, "ftps://%s%s/%s", szFTPIPAddress, szFTPFilePath, szFTPFileName);
			strcpy(srFtpsObj.szFtpsURL, szTemplate);
		}
		
                /* 下載檔案 失敗重試3次 */
		/* 使用新憑證 */
		memset(srFtpsObj.szCACertFilePath, 0x00, sizeof(srFtpsObj.szCACertFilePath));
		memset(srFtpsObj.szCACertFileName, 0x00, sizeof(srFtpsObj.szCACertFileName));
//		/* 設定CA憑證路徑 */
//		if (strlen(_CA_DATA_PATH_) > 0)
//		{
//			strcat(srFtpsObj.szCACertFilePath, _CA_DATA_PATH_);
//		}
			
		if (strlen(_PEM_NEW_TLS_FILE_NAME_) > 0)
		{
			strcat(srFtpsObj.szCACertFileName, _CA_DATA_PATH_);
			strcat(srFtpsObj.szCACertFileName, _PEM_NEW_TLS_FILE_NAME_);
		}
		
                inRetVal = inNCCCTMS_FTP_APPARM_Disp_Flow(pobTran, &srFtpsObj);
                
                if (inRetVal == VS_SUCCESS)
		{
                        continue;
		}
		
		/* 下載檔案 失敗重試3次 */
		/* 使用舊憑證 */
		memset(srFtpsObj.szCACertFilePath, 0x00, sizeof(srFtpsObj.szCACertFilePath));
		memset(srFtpsObj.szCACertFileName, 0x00, sizeof(srFtpsObj.szCACertFileName));
//		/* 設定CA憑證路徑 */
//		if (strlen(_CA_DATA_PATH_) > 0)
//		{
//			strcat(srFtpsObj.szCACertFilePath, _CA_DATA_PATH_);
//		}
			
		if (strlen(_PEM_PRESERVE_TLS_FILE_NAME_) > 0)
		{
			strcat(srFtpsObj.szCACertFileName, _CA_DATA_PATH_);
			strcat(srFtpsObj.szCACertFileName, _PEM_PRESERVE_TLS_FILE_NAME_);
		}
		
		inRetVal = inNCCCTMS_FTP_APPARM_Disp_Flow(pobTran, &srFtpsObj);
        }

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCCTMS_FTPS_APPARM_Download() END!");
        
	if (inRetVal == VS_SUCCESS)
	{
		return (VS_SUCCESS);
	}
	else
	{
		return (VS_ERROR);
	}
}
/*
Function        :inNCCCTMS_FTPS_FuncResultHandle
Date&Time       :2016-08-16 下午 02:19:14
Describe        :列印FTPS下載狀態條及立即更新
*/
int inNCCCTMS_FTPS_FuncResultHandle(TRANSACTION_OBJECT *pobTran)
{
        int			inRetVal = 0, i = 0, inHeight = 0;
        unsigned char		uszBuffer[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_] = {0};
        char			szTemplate[60 + 1] = {0};
	char			szSN[15 + 1] = {0};
        char			szPrintBuf[384 + 1] = {0};
        char			szFileName[60 + 1] = {0};
	char			szCloseBatchBit[2 + 1] = {0};
	unsigned char		uszUpdateBit = VS_FALSE;
	BufferHandle		srBhandle;
	FONT_ATTRIB		srFont_Attrib;
        VS_BOOL			fDWLSuccess = VS_SUCCESS;

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCCTMS_FTPS_FuncResultHandle() START !!");
	
	/* 下載完成才印下載檔案狀態紙條 */
        if (inFILE_Check_Exist((unsigned char *)_FTPFLT_FILE_NAME_) != VS_SUCCESS)
                return (VS_SUCCESS);
	
	/* 自動詢問和下載的不用印 */
        if (pobTran->inTMSDwdMode == _TMS_AUTO_DOWNLOAD_)
        {
                for (i = 0 ;; i++)
                {       
                        /* 下載結果已經存在File Index */
                        if (inLoadFTPFLTRec(i) < 0)
                                break;
                        
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        /* 取得下載結果 Y or N */
                        inGetFTPFileIndex(szTemplate);
                        
                        if (!memcmp(&szTemplate[0], "Y", 1))
                        {
                                /* 下載成功且是排程下載，將指定詢問時間初始化*/
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                inGetTMSInquireMode(szTemplate);
                                        
                                if (!memcmp(&szTemplate[0], _TMS_INQUIRE_02_SCHEDHULE_SETTLE_, 1))
                                {
                                        inSetTMSInquireStartDate("00000000");
                                        inSetTMSInquireTime("000000");
                                        inSaveTMSSCTRec(0);
                                }                                
                        }
                        else
                        {
                                fDWLSuccess = VS_ERROR;
                        }                
                }
        }
        else
	{
		/* 是否有列印功能 */
		if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
		{

		}
		else
		{
			inPRINT_Buffer_Initial(uszBuffer, _BUFFER_MAX_LINE_, &srFont_Attrib, &srBhandle);

			/* 印TMS下載狀態的圖 */
			inPRINT_Buffer_GetHeight((unsigned char*)"TMS_STATUS.bmp", &inHeight);

			if (inPRINT_Buffer_PutGraphic((unsigned char*)_TMS_DOWNLOAD_STATUS_, uszBuffer, &srBhandle, inHeight, _APPEND_) != VS_SUCCESS)
				return (VS_ERROR);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			/* 列印時間 */
			inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			inCREDIT_PRINT_Printing_Time(pobTran, _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle);

			/* Get商店代號 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetMerchantID(szTemplate);

			/* 列印商店代號 */
			sprintf(szPrintBuf, "商店代號");
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szTemplate);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			/* Get端末機代號 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetTerminalID(szTemplate);

			/* 列印端末機代號 */
			sprintf(szPrintBuf, "端末機代號");
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szTemplate);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			/* 分隔線 */
			inRetVal = inPRINT_Buffer_PutIn("================================================", _PRT_NORMAL_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);


			/* Terminal AP Name */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			if (strlen(gszTermVersionID) > 0)
			{
				memcpy(szTemplate, gszTermVersionID, strlen(gszTermVersionID));
			}
			else
			{
				inGetTermVersionID(szTemplate);
			}
			sprintf(szPrintBuf, "VERSION ID = %s", szTemplate);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			/* Terminal AP Version */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			if (strlen(gszTermVersionDate) > 0)
			{
				memcpy(szTemplate, gszTermVersionDate, strlen(gszTermVersionDate));
			}
			else
			{
				inGetTermVersionDate(szTemplate);
			}
			sprintf(szPrintBuf, "VERSION DATE = %s", szTemplate);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			/* Terminal S/N */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			/* 取後12碼，但最後一碼為CheckSum，所以取4~15 */
			inFunc_GetSeriaNumber(szTemplate);
			memset(szSN, 0x00, sizeof(szSN));
			memcpy(szSN, &szTemplate[3], 12);
			sprintf(szPrintBuf, "TERMINAL S/N = %s", szSN);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			strcpy(szPrintBuf, "下載檔案清單：(依下清單列示)");
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			strcpy(szPrintBuf, "　　　　　　　 成功　　失敗");
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			/* 列印TMS下載結果 */
			for (i = 0;; i++)
			{       
				/* 下載結果已經存在File Index，取得檔名後直接列印結果 */
				if (inLoadFTPFLTRec(i) < 0)
					break;

				memset(szFileName, 0x00, sizeof(szFileName));
				inGetFTPFileName(szFileName);
				inFunc_PAD_ASCII(szFileName, szFileName, ' ', 16, _PADDING_RIGHT_);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				strcpy(szPrintBuf, szFileName);

				/* 取得下載結果 Y or N */
				inGetFTPFileIndex(szTemplate);

				if (!memcmp(&szTemplate[0], "Y", 1))
					strcat(szPrintBuf, "●");
				else
				{
					strcat(szPrintBuf, "　　　　●");
					fDWLSuccess = VS_ERROR;
				}                

				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}

			if (fDWLSuccess == VS_SUCCESS)
			{
				inPRINT_Buffer_GetHeight((unsigned char*)"TMS_SUCCESS.bmp", &inHeight);

				if (inPRINT_Buffer_PutGraphic((unsigned char*)_TMS_DOWNLOAD_SUCCESS_, uszBuffer, &srBhandle, inHeight, _APPEND_) != VS_SUCCESS)
					return (VS_ERROR);
			}
			else
			{
				inPRINT_Buffer_GetHeight((unsigned char*)"TMS_FAILURE.bmp", &inHeight);

				if (inPRINT_Buffer_PutGraphic((unsigned char*)_TMS_DOWNLOAD_FAILURE_, uszBuffer, &srBhandle, inHeight, _APPEND_) != VS_SUCCESS)
					return (VS_ERROR);
			}

			for (i = 0; i < 8; i++)
			{
				inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}

			inPRINT_Buffer_OutPut(uszBuffer, &srBhandle);
		}
	}
	if (fDWLSuccess != VS_SUCCESS)
	{
		/* 刪除File List */
		inFILE_Delete((unsigned char *)_FTPFLT_FILE_NAME_);
	}
	else
	{
		/* 紀錄TMS的下載方式，已因應更新時的不同行為 */
		if (pobTran->inTMSDwdMode == _TMS_MANUAL_DOWNLOAD_)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%d", _TMS_MANUAL_DOWNLOAD_);
			inSetLastDownloadMode(szTemplate);
		}
		else if (pobTran->inTMSDwdMode == _TMS_AUTO_DOWNLOAD_)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%d", _TMS_AUTO_DOWNLOAD_);
			inSetLastDownloadMode(szTemplate);
		}
		else
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%d", _TMS_DEFAULT_DOWNLOAD_);
			inSetLastDownloadMode(szTemplate);
		}
		inSaveEDCRec(0);
		
		/* 下載完，若可以更新立即重開機 */
		/* 檢查是否有帳 */
		inLoadTMSFTPRec(0);
		inGetFTPEffectiveCloseBatch(szCloseBatchBit);
		if (memcmp(szCloseBatchBit, "Y", strlen("Y")) == 0)
		{
			if (inFLOW_RunFunction(pobTran, _FUNCTION_All_HOST_MUST_SETTLE_) != VS_SUCCESS)
			{
				/* 被結帳擋，直接清空並回報 */
				inLoadTMSFTPRec(0);
				inSetFTPDownloadResponseCode(_FTP_DOWNLOAD_REPORT_EFFECT_FAIED_FOR_NOT_SETTLE_);
				inSaveTMSFTPRec(0);
				inFLOW_RunFunction(pobTran, _EDC_BOOTING_UPDATE_SUCCESS_REPORT_);
				
				return (VS_SUCCESS);
			}
			else
			{
				uszUpdateBit = VS_TRUE;
			}
		}
		else
		{
			uszUpdateBit = VS_TRUE;
		}
	}
	/* 可以更新，直接重開機 */
	if (uszUpdateBit == VS_TRUE)
	{
		pobTran->uszTMSDownloadRebootBit = VS_TRUE;
	}
	
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCCTMS_FTPS_FuncResultHandle() END !!");
        
        return (VS_SUCCESS);
}

/*
Function        :inNCCCTMS_DCC_FuncDownloadParameter
Date&Time       :2016-08-22 下午 02:19:14
Describe        :DCC參數下載進入點
*/
int inNCCCTMS_DCC_FuncDownloadParameter(TRANSACTION_OBJECT *pobTran)
{
        int     inRetVal;
        char    szTMSOK[2 + 1];
        char    szHostEnable[2 + 1];
        char    szDCCFlowVersion[2 + 1];
        
        vdUtility_SYSFIN_LogMessage(AT, "inNCCCTMS_DCC_FuncDownloadParameter START!");
        
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCCTMS_DCC_FuncDownloadParameter() START!!");
        
        memset(&gsrDCC_Download, 0x00, sizeof(DCC_DATA));
        
        /* 先Load EDC.dat */
        inLoadEDCRec(0);

	/* 註1:進行DCC參數下載前，必須先進行TMS參數下載成功，若TMS參數下載失敗，則不可進行DCC參數下載。 (START) */
        memset(szTMSOK, 0x00, sizeof(szTMSOK));
        inGetTMSOK(szTMSOK);
        
        if (szTMSOK[0] != 'Y')
        {
                /* 請執行參數下載 */
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, _ERR_TMS_DWL_);
		srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
		srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
		srDispMsgObj.inTimeout = 1;
		strcpy(srDispMsgObj.szErrMsg1, "");
		srDispMsgObj.inErrMsg1Line = 0;
		srDispMsgObj.inBeepTimes = 1;
		srDispMsgObj.inBeepInterval = 0;
                inDISP_Msg_BMP(&srDispMsgObj);
        }
        /* 註1:進行DCC參數下載前，必須先進行TMS參數下載成功，若TMS參數下載失敗，則不可進行DCC參數下載。(END) */
	
	/* 若端末機不支援DCC，則顯示此功能已關閉 */
	inRetVal = inNCCC_DCC_SwitchToDCC_Host(pobTran->srBRec.inHDTIndex);
        if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	pobTran->srBRec.inHDTIndex = ginDCCHostIndex;
        inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
	
        memset(szHostEnable, 0x00, sizeof(szHostEnable));
        inGetHostEnable(szHostEnable);
        
        if (szHostEnable[0] != 'Y')
        {
                /* 此功能已關閉 */
                pobTran->inErrorMsg = _ERROR_CODE_V3_FUNC_CLOSE_;
                return (VS_ERROR);
        }
        /* 若端末機不支援DCC，則顯示此功能已關閉 */
	
	
        if (inLoadCFGTRec(0) < 0)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inLoadCFGTRec() ERROR!!");
                
                return (VS_ERROR);
        }
        
        memset(szDCCFlowVersion, 0x00, sizeof(szDCCFlowVersion));
        inGetDccFlowVersion(szDCCFlowVersion);
        if (memcmp(szDCCFlowVersion, _NCCC_DCC_FLOW_VER_NOT_SUPORTED_, 1) == 0)
        {
           	/* 此功能已關閉 */
	        pobTran->inErrorMsg = _ERROR_CODE_V3_FUNC_CLOSE_;
		
        	return (VS_ERROR);
        }
	
        /* 連線到DCC主機 */
        inRetVal = inFLOW_RunFunction(pobTran, _COMM_START_);
        
        if (inRetVal != VS_SUCCESS)
        {
                /* 通訊失敗‧‧‧‧ */
                pobTran->inTransactionResult = _TRAN_RESULT_COMM_ERROR_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_COMM_;
		inFunc_Display_Error(pobTran);			/* 通訊失敗 */
                /* 列印狀態條 */
                inNCCCTMS_DCC_PRINT_AllDownloadMessage(pobTran);

                return (VS_COMM_ERROR);
        }
               
        /* 開始下載 */
        inRetVal = inNCCC_DCC_TMSDownload_CheckStatus(pobTran);
        
	/* 斷線 */
        inCOMM_End(pobTran);
	
        /* 列印狀態條(避免印到一半中斷) */
	do
	{
		inRetVal = inNCCCTMS_DCC_PRINT_AllDownloadMessage(pobTran);
	}while (inRetVal != VS_SUCCESS);
        
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCCTMS_DCC_FuncDownloadParameter() END!!");
        
        if (inRetVal != VS_SUCCESS)
        {
		inFunc_Display_Error(pobTran);
        }
        
        return (VS_SUCCESS);
}

/*
Function        :inNCCCTMS_DCC_PRINT_AllDownloadMessage
Date&Time       :2016/9/13 下午 1:32
Describe        :DCC參數下載狀態條
*/
int inNCCCTMS_DCC_PRINT_AllDownloadMessage(TRANSACTION_OBJECT *pobTran)
{
        int			inRetVal, i = 0, inHeight = 0;
        char			szTemplate[60 + 1];
        char			szPrintBuf[384 + 1];
	unsigned char 		uszDLSuccess = VS_SUCCESS;				/* 只要有一個下載敗就算失敗 */
	unsigned char		uszBuffer[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	BufferHandle		srBhandle;
	FONT_ATTRIB		srFont_Attrib;
        
	/* 是否有列印功能 */
	if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
	{
		return (VS_SUCCESS);
	}
	else
	{
		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "inNCCCTMS_DCC_PRINT_AllDownloadMessage() START !!");

		/* 初始化 */
		inPRINT_Buffer_Initial(uszBuffer, _BUFFER_MAX_LINE_, &srFont_Attrib, &srBhandle);

		inPRINT_Buffer_GetHeight((unsigned char*)"DCC_STATUS.bmp", &inHeight);

		if (inPRINT_Buffer_PutGraphic((unsigned char*)_DCC_DOWNLOAD_STATUS_, uszBuffer, &srBhandle, inHeight, _APPEND_) != VS_SUCCESS)
			return (VS_ERROR);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		/* 列印時間 */
		inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		inCREDIT_PRINT_Printing_Time(pobTran, _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle);

		/* Get商店代號 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetMerchantID(szTemplate);

		/* 列印商店代號 */
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 15, _PADDING_LEFT_);
		sprintf(szPrintBuf, "商店代號 %s", szTemplate);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		/* Get端末機代號 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTerminalID(szTemplate);

		/* 列印端末機代號 */
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 14, _PADDING_LEFT_);
		sprintf(szPrintBuf, "端末機代號%s", szTemplate);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		/* 分隔線 */
		inRetVal = inPRINT_Buffer_PutIn("================================================", _PRT_NORMAL_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);


		/* 確認是否0下載(檔案清單中沒有檔案要下載或通訊失敗) */
		if (atoi(&gsrDCC_Download.szDCC_FileList[0]) != 0)
		{
			/* 下載檔案清單：(依下清單列示) */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			strcpy(szPrintBuf, "下載檔案清單：(依下清單列示)");
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			/* 成功/失敗 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			strcpy(szPrintBuf, "成功　　失敗");
			inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, 200);

			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			for (i = 0; i < 7; i ++)
			{
				/* 清空，初始化 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate, 0x00, sizeof(szTemplate));
				
				/* (需求單-111060)-DCC流程修改需求 by Russell 2022/4/11 下午 3:26 */
				/* 	DCCFES不下載支援幣別資料給EDC(維持現行10種幣別)，所以DCC下載狀態結果第5~7項(如下表)不需印 */
				if (i == 4 || i == 5 || i == 6)
				{
					continue;
				}
				
				switch (i)
				{
					/* 1.BIN Table */
					case 0:
						/* 下載清單有這項 */
						if (gsrDCC_Download.szDCC_FileList[i] == '1')
						{
							sprintf(szPrintBuf, "1. BIN Table");
							if (gsrDCC_Download.szDCC_FileDownloadStaus[i] == '1')
							{
								strcat(szTemplate, "●　　　　　　");
							}
							else
							{
								uszDLSuccess = VS_ERROR;
								strcat(szTemplate, "　　　　　　●");
							}
							inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szTemplate, _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, 200);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
						}
						else
						{
							/* 下載清單沒這項，跳過 */
						}
						break;
					/* 2.Exchange Rate Source */
					case 1:
						/* 下載清單有這項 */
						if (gsrDCC_Download.szDCC_FileList[i] == '1')
						{
							sprintf(szPrintBuf, "2. Exchange Rate Source");
							if (gsrDCC_Download.szDCC_FileDownloadStaus[i] == '1')
							{
								strcat(szTemplate, "●　　　　　　");
							}
							else
							{
								uszDLSuccess = VS_ERROR;
								strcat(szTemplate, "　　　　　　●");
							}
							inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szTemplate, _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, 200);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
						}
						else
						{
							/* 下載清單沒這項，跳過 */
						}
						break;
					/* 3.VISA Disclaimer Text*/
					case 2:
						/* 下載清單有這項 */
						if (gsrDCC_Download.szDCC_FileList[i] == '1')
						{
							sprintf(szPrintBuf, "3. VISA Disclaimer Text");
							if (gsrDCC_Download.szDCC_FileDownloadStaus[i] == '1')
							{
								strcat(szTemplate, "●　　　　　　");
							}
							else
							{
								uszDLSuccess = VS_ERROR;
								strcat(szTemplate, "　　　　　　●");
							}
							inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szTemplate, _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, 200);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
						}
						else
						{
							/* 下載清單沒這項，跳過 */
						}
						break;
					/* 4.MasterCard Disclaimer */
					case 3:
						/* 下載清單有這項 */
						if (gsrDCC_Download.szDCC_FileList[i] == '1')
						{
							sprintf(szPrintBuf, "4. M/C Disclaimer Text");
							if (gsrDCC_Download.szDCC_FileDownloadStaus[i] == '1')
							{
								strcat(szTemplate, "●　　　　　　");
							}
							else
							{
								uszDLSuccess = VS_ERROR;
								strcat(szTemplate, "　　　　　　●");
							}
							inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szTemplate, _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, 200);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
						}
						else
						{
							/* 下載清單沒這項，跳過 */
						}
						break;
					/* 5.VISA Supported Currency List */
					case 4:
						/* 下載清單有這項 */
						if (gsrDCC_Download.szDCC_FileList[i] == '1')
						{
							sprintf(szPrintBuf, "5. VISA Currency List");
							if (gsrDCC_Download.szDCC_FileDownloadStaus[i] == '1')
							{
								strcat(szTemplate, "●　　　　　　");
							}
							else
							{
								uszDLSuccess = VS_ERROR;
								strcat(szTemplate, "　　　　　　●");
							}
							inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szTemplate, _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, 200);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
						}
						else
						{
							/* 下載清單沒這項，跳過 */
						}
						break;
					/* 6.MasterCard Supported Currency List */
					case 5:
						/* 下載清單有這項 */
						if (gsrDCC_Download.szDCC_FileList[i] == '1')
						{
							sprintf(szPrintBuf, "6. M/C Currency List");
							if (gsrDCC_Download.szDCC_FileDownloadStaus[i] == '1')
							{
								strcat(szTemplate, "●　　　　　　");
							}
							else
							{
								uszDLSuccess = VS_ERROR;
								strcat(szTemplate, "　　　　　　●");
							}
							inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szTemplate, _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, 200);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
						}
						else
						{
							/* 下載清單沒這項，跳過 */
						}
						break;
					/* 7.DCC Flow Version */
					case 6:
						/* 下載清單有這項 */
						if (gsrDCC_Download.szDCC_FileList[i] == '1')
						{
							sprintf(szPrintBuf, "7. DCC Flow Version");
							if (gsrDCC_Download.szDCC_FileDownloadStaus[i] == '1')
							{
								strcat(szTemplate, "●　　　　　　");
							}
							else
							{
								uszDLSuccess = VS_ERROR;
								strcat(szTemplate, "　　　　　　●");
							}
							inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szTemplate, _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _CURRENT_LINE_, 200);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
						}
						else
						{
							/* 下載清單沒這項，跳過 */
						}
						break;
					default:
						break;
				}

				/* 下載清單有這項才要印，否則會印出空白行 */
				if (gsrDCC_Download.szDCC_FileList[i] == '1')
				{
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
				}
			}

		}
		/* 通訊失敗算下載失敗 */
		else if (pobTran->inTransactionResult == _TRAN_RESULT_COMM_ERROR_)
		{
			uszDLSuccess = VS_ERROR;
		}
		/* 沒有要下載的檔案，而且也不是通訊失敗，算下載成功(0下載) */
		else
		{

		}

		if (uszDLSuccess == VS_SUCCESS)
		{
			inPRINT_Buffer_GetHeight((unsigned char*)"DCC_SUCCESS.bmp", &inHeight);

			if (inPRINT_Buffer_PutGraphic((unsigned char*)_DCC_DOWNLOAD_SUCCESS_, uszBuffer, &srBhandle, inHeight, _APPEND_) != VS_SUCCESS)
				return (VS_ERROR);
		}
		else
		{
			inPRINT_Buffer_GetHeight((unsigned char*)"DCC_FAILURE.bmp", &inHeight);

			if (inPRINT_Buffer_PutGraphic((unsigned char*)_DCC_DOWNLOAD_FAILURE_, uszBuffer, &srBhandle, inHeight, _APPEND_) != VS_SUCCESS)
				return (VS_ERROR);
		}

		for (i = 0; i < 8; i++)
		{
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}

		if (inPRINT_Buffer_OutPut(uszBuffer, &srBhandle) != VS_SUCCESS)
			return (VS_ERROR);

		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "inNCCCTMS_DCC_PRINT_AllDownloadMessage() END !!");

		return (VS_SUCCESS);
	}
}

/*
Function        :inNCCCTMS_PRINT_ScheduleMessage
Date&Time       :2017/1/25 上午 10:16
Describe        :預告排程時間和排程自動下載完成都使用這一隻，
 *		 1.提示時間在inStatus填入_TMS_PRT_SCHEDULE_NOFTFY_
 *		 2.提示下載完成在inStatus填入_TMS_PRT_SCHEDULE_SUCCESS_
*/
int inNCCCTMS_PRINT_ScheduleMessage(TRANSACTION_OBJECT *pobTran, int inStatus)
{
	int			inRetVal;
	char			szPrintBuf[42 + 1], szTemplate[42 + 1], szDateTime[15 + 1];
	char			szCustomerIndicator[3 + 1] = {0};
	unsigned char		uszBuffer[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	unsigned char		uszFileName[30 + 1] = {0};
	BufferHandle		srBhandle;
	FONT_ATTRIB		srFont_Attrib;
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	/* 是否有列印功能 */
	if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
	{
		return (VS_SUCCESS);
	}
	else
	{
		/* 初始化 */
		inPRINT_Buffer_Initial(uszBuffer, _BUFFER_MAX_LINE_, &srFont_Attrib, &srBhandle);

		if (inLoadHDTRec(0) < 0)			/* 主機參數檔【HostDef.txt】 */
		{
			return (VS_ERROR);
		}
		if (inLoadHDPTRec(0) < 0)
		{
			return (VS_ERROR);
		}

		/* 重要訊息通知 */
		inRetVal = inCREDIT_PRINT_Schedule_ByBuffer(pobTran, uszBuffer, &srFont_Attrib, &srBhandle);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		/* 列印時間 */
		inRetVal = inCREDIT_PRINT_Printing_Time(pobTran, _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		/* Print NCCC LOGO 384*60 */
		inRetVal = inCREDIT_PRINT_Logo_ByBuffer(pobTran, uszBuffer, &srFont_Attrib, &srBhandle);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		inRetVal = inCREDIT_PRINT_Tidmid_ByBuffer(pobTran, uszBuffer, &srFont_Attrib, &srBhandle);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		/* 自動排程下載時間提示 */
		if (inStatus == _TMS_PRT_SCHEDULE_NOFTFY_)
		{
			/* 排程日期 */
			memset(szDateTime, 0x00, sizeof(szDateTime));
			inGetTMSScheduleDate(szDateTime);
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &szDateTime[0], 4);
			strcat(szTemplate, "/");
			memcpy(&szTemplate[5], &szDateTime[4], 2);
			strcat(szTemplate, "/");
			memcpy(&szTemplate[8], &szDateTime[6], 2);
			szTemplate[10] = ' ';

			/* 排程時間 */
			memset(szDateTime, 0x00, sizeof(szDateTime));
			inGetTMSScheduleTime(szDateTime);

			memcpy(&szTemplate[11], &szDateTime[0], 2);
			strcat(szTemplate, ":");
			memcpy(&szTemplate[14], &szDateTime[2], 2);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "本機將於 %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("進行資料自動更新作業，屆時【請勿關機】。", _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("此為系統自動化作業，無需人員操作及留守，", _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("僅需注意於上述作業時間本機必須為【開機】", _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("狀態即可。作業開始時，本機螢幕會顯示【", _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("資料更新中，請勿關機】之訊息。", _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		/* 自動排程下載完成提示*/
		else if (inStatus == _TMS_PRT_SCHEDULE_SUCCESS_)
		{
			inPRINT_Buffer_PutIn("本機【已完成自動下載作業】，請確認目前螢幕", _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("顯示為聯合信用卡中心待機畫面即可開始使用。", _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}

		inRetVal = inCREDIT_PRINT_End_ByBuffer(pobTran, uszBuffer, &srFont_Attrib, &srBhandle);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_075_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_103_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_))
		{
			if (inStatus == _TMS_PRT_SCHEDULE_NOFTFY_)
			{
				memset(uszFileName, 0x00, sizeof(uszFileName));
				sprintf((char*)uszFileName, "%s%s", _DUTY_FREE_REPRINT_TMS_NOTIFY_FILE_NAME_, _DUTY_FREE_REPRINT_TOTAL_REPORT_FILE_EXTENSION_);
				if (srBhandle.inYcurrent >= srBhandle.inYcover)
				{
					inNCCC_Func_Duty_Free_Save_Total_Report_Buffer(uszFileName, uszBuffer, PB_CANVAS_X_SIZE * (srBhandle.inYcurrent + 8));
				}
				else
				{
					inNCCC_Func_Duty_Free_Save_Total_Report_Buffer(uszFileName, uszBuffer, PB_CANVAS_X_SIZE * (srBhandle.inYcover + 8));
				}
			}
		}

		inRetVal = inPRINT_Buffer_OutPut(uszBuffer, &srBhandle);
	
		return (VS_SUCCESS);
	}
}

/*
Function        :inNCCCTMS_PRINT_Return_Task
Date&Time       :2017/5/19 下午 2:26
Describe        :列印TMS至現回報簽單
*/
int inNCCCTMS_PRINT_Return_Task(TRANSACTION_OBJECT *pobTran)
{
	int			inRetVal, i = 0, inHeight = 0;
        char			szTemplate[60 + 1];
        char			szPrintBuf[384 + 1];
	unsigned char		uszBuffer[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	BufferHandle		srBhandle;
	FONT_ATTRIB		srFont_Attrib;
	
        vdUtility_SYSFIN_LogMessage(AT, "inNCCCTMS_PRINT_Return_Task START!");
        
	/* 是否有列印功能 */
	if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
	{
		return (VS_SUCCESS);
	}
	else
	{
		inPRINT_Buffer_Initial(uszBuffer, _BUFFER_MAX_LINE_, &srFont_Attrib, &srBhandle);
		/* 列印下載結果 */
		/* 至現回報簽單 */
		inPRINT_Buffer_GetHeight((unsigned char*)"TASK.bmp", &inHeight);

		/* Title 至現回報 */
		if (inPRINT_Buffer_PutGraphic((unsigned char*)_TMS_TASK_TITLE_, uszBuffer, &srBhandle, inHeight, _APPEND_) != VS_SUCCESS)
			return (VS_ERROR);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

		/* 列印時間 */
		inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		inCREDIT_PRINT_Printing_Time(pobTran, _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle);

		/* Get商店代號 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetMerchantID(szTemplate);

		/* 列印商店代號 */
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 15, _PADDING_LEFT_);
		sprintf(szPrintBuf, "商店代號 %s", szTemplate);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		/* Get端末機代號 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTerminalID(szTemplate);

		/* 列印端末機代號 */
		inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 13, _PADDING_LEFT_);
		sprintf(szPrintBuf, "端末機代號 %s", szTemplate);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		/* 分隔線 */
		inRetVal = inPRINT_Buffer_PutIn("================================================", _PRT_NORMAL_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);


		/* Terminal AP Name */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		if (strlen(gszTermVersionID) > 0)
		{
			memcpy(szTemplate, gszTermVersionID, strlen(gszTermVersionID));
		}
		else
		{
			inGetTermVersionID(szTemplate);
		}
		sprintf(szPrintBuf, "VERSION ID = %s", szTemplate);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		/* Terminal AP Version */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		if (strlen(gszTermVersionDate) > 0)
		{
			memcpy(szTemplate, gszTermVersionDate, strlen(gszTermVersionDate));
		}
		else
		{
			inGetTermVersionDate(szTemplate);
		}
		sprintf(szPrintBuf, "VERSION DATE = %s", szTemplate);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		/* BATCH NO. */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		strcpy(szPrintBuf, "BATCH NO. = ");
		strcat(szPrintBuf, gsrTMS_Field58.szBatchNumber);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		inPRINT_Buffer_GetHeight((unsigned char*)"END.bmp", &inHeight);

		if (inPRINT_Buffer_PutGraphic((unsigned char*)_END_LOGO_, uszBuffer, &srBhandle, inHeight, _APPEND_) != VS_SUCCESS)
			return (VS_ERROR);

		for (i = 0; i < 8; i++)
		{
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}

		inPRINT_Buffer_OutPut(uszBuffer, &srBhandle);

		return (VS_SUCCESS);
	}
}

/*
Function        :inNCCCTMS_CheckAllDownloadFile_Flow
Date&Time       :2017/1/25 下午 2:44
Describe        :確認檔案分流
*/
int inNCCCTMS_CheckAllDownloadFile_Flow(TRANSACTION_OBJECT * pobTran)
{
	int	inRetVal;
	char	szDebugMsg[100 + 1];
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCCTMS_CheckAllDownloadFile_Flow() START !");
	}
	
	/* FTPS */
	if (pobTran->uszFTP_TMS_Download == _TMS_DOWNLOAD_SECURE_MODE_FTPS_	||
	    pobTran->uszFTP_TMS_Download == _TMS_DOWNLOAD_SECURE_MODE_SFTP_)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inNCCCTMS_CheckAllDownloadFile Choose FTPS");
			inLogPrintf(AT, szDebugMsg);
		}
		inRetVal = inNCCCTMS_CheckAllDownloadFile_FTPS();
	}
	/* ISO8583 */
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inNCCCTMS_CheckAllDownloadFile Choose ISO8583");
			inLogPrintf(AT, szDebugMsg);
		}
		inRetVal = inNCCCTMS_CheckAllDownloadFile_ISO8583();
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCCTMS_CheckAllDownloadFile_Flow() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :inNCCCTMS_CheckAllDownloadFile_ISO8583
Date&Time       :2016/12/13 下午 2:47
Describe        :檢查檔案完整性，若有下載失敗檔案，檔案全刪
*/
int inNCCCTMS_CheckAllDownloadFile_ISO8583()
{
	int		i = 0, j = 0, inFileNameLen = 0, inSlash = 0;
        long		lnFileSize = 0, lnTMSFLTSize = 0;
	int		inDownloadStatus = VS_SUCCESS;
        char		szTemplate[60 + 1], szFileName[60 + 1];
        unsigned long   ulFile_Handle;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCCTMS_CheckAllDownloadFile_ISO8583() START !");
	}
	
	if (inFILE_Check_Exist((unsigned char *)_TMSFLT_FILE_NAME_) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
        for (i = 0;; i++)
        {
                /* Load TMS File List 取得檔名 */
                if (inLoadTMSFLTRec(i) < 0)
                        break;

                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetTMSFilePathName(szTemplate);

                /* 取得檔案路徑後 抓取檔名儲存到FileName */
                for (j = 0; j < 60 ; j++)
                {
                        if (szTemplate[j] == '/')
                            inSlash = j + 1;

                        if (szTemplate[j] == 0x00)
                            break;
                }

		/* 斜線後的檔名長度*/
                inFileNameLen = j - inSlash;
                memset(szFileName, 0x00, sizeof(szFileName));
                memcpy(&szFileName[0], &szTemplate[inSlash], inFileNameLen);
		
                /* 比對下載的檔案與File List提供的檔案大小是否一致 */
                lnFileSize = lnFILE_GetSize(&ulFile_Handle, (unsigned char *)szFileName);
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetTMSFileSize(szTemplate);
                lnTMSFLTSize = atol(szTemplate);

                /* 下載成功或失敗結果存在TMSFileIndex */
                if (lnFileSize == lnTMSFLTSize)
                {
                        continue;
                }
                else
                {
			inDownloadStatus = VS_ERROR;
                        inFILE_Delete((unsigned char *)szFileName);
                }
		
        }
	
	/* 若有下載失敗，回傳錯誤並刪FileList */
	if (inDownloadStatus == VS_ERROR)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "TMS下載不完全，刪除FileList");
		}
		
		inFILE_Delete((unsigned char *)_TMSFLT_FILE_NAME_);
		
		return (VS_ERROR);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCCTMS_CheckAllDownloadFile_ISO8583() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCCTMS_CheckAllDownloadFile_FTPS
Date&Time       :2017/1/25 下午 2:19
Describe        :檢查檔案完整性，若有下載失敗檔案，檔案全刪
*/
int inNCCCTMS_CheckAllDownloadFile_FTPS()
{
	int		i = 0;
        long		lnFileSize = 0, lnTMSFLTSize = 0;
	int		inDownloadStatus = VS_SUCCESS;
        char		szTemplate[60 + 1], szFileName[60 + 1];
	char		szDebugMsg[100 + 1];
        unsigned long   ulFile_Handle;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCCTMS_CheckAllDownloadFile_FTPS() START !");
	}
	
	if (inFILE_Check_Exist((unsigned char *)_FTPFLT_FILE_NAME_) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
        for (i = 0;; i++)
        {
                /* Load TMS File List 取得檔名 */
                if (inLoadFTPFLTRec(i) < 0)
                        break;

                memset(szFileName, 0x00, sizeof(szFileName));
                inGetFTPFileName(szFileName);
		
		/* AP剛解壓縮完就刪了，所以不檢查 */
		if (memcmp(szFileName, _APPL_NAME_, strlen(_APPL_NAME_)) == 0)
		{
			continue;
		}
		
                /* 比對下載的檔案與File List提供的檔案大小是否一致 */
                lnFileSize = lnFILE_GetSize(&ulFile_Handle, (unsigned char *)szFileName);
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetFTPFileSize(szTemplate);
                lnTMSFLTSize = atol(szTemplate);

                /* 下載成功或失敗結果存在TMSFileIndex */
                if (lnFileSize == lnTMSFLTSize)
                {
                        continue;
                }
                else
                {
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "(FSize)%ld ≠ %ld(FLTSize)", lnFileSize, lnTMSFLTSize);
				inLogPrintf(AT, szDebugMsg);
			}
			inDownloadStatus = VS_ERROR;
                        inFILE_Delete((unsigned char *)szFileName);
                }
	
        }
	
	/* 若有下載失敗，回傳錯誤並刪FileList */
	if (inDownloadStatus == VS_ERROR)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "TMS下載不完全，刪除FileList");
		}
		
		inFILE_Delete((unsigned char *)_FTPFLT_FILE_NAME_);
		
		/* 檔案大小不符合 */
		inLoadTMSFTPRec(0);
		inSetFTPDownloadResponseCode(_FTP_DOWNLOAD_REPORT_FILE_SIZE_NOT_VAILID_);
		inSaveTMSFTPRec(0);
		
		return (VS_ERROR);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCCTMS_CheckAllDownloadFile_FTPS() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCCTMS_DeleteAllDownloadFile_Flow
Date&Time       :2017/1/25 下午 4:40
Describe        :刪除檔案分流
*/
int inNCCCTMS_DeleteAllDownloadFile_Flow(TRANSACTION_OBJECT * pobTran)
{	
	int	inRetVal = VS_SUCCESS;
	char	szDebugMsg[100 + 1];
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCCTMS_DeleteAllDownloadFile_Flow() START !");
	}
	
	/* FTPS */
	if (pobTran->uszFTP_TMS_Download == _TMS_DOWNLOAD_SECURE_MODE_FTPS_	||
	    pobTran->uszFTP_TMS_Download == _TMS_DOWNLOAD_SECURE_MODE_SFTP_)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inNCCCTMS_DeleteAllDownloadFile_Flow Choose FTPS");
			inLogPrintf(AT, szDebugMsg);
		}
		inRetVal = inNCCCTMS_DeleteAllDownloadFile_FTPS();
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inNCCCTMS_DeleteAllDownloadFile_Flow Choose ISO8583");
			inLogPrintf(AT, szDebugMsg);
		}
		inRetVal = inNCCCTMS_DeleteAllDownloadFile_ISO8583();
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCCTMS_DeleteAllDownloadFile_Flow() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :inNCCCTMS_DeleteAllDownloadFile_ISO8583
Date&Time       :2016/12/13 下午 2:47
Describe        :下載檔案全刪
*/
int inNCCCTMS_DeleteAllDownloadFile_ISO8583()
{
	int		i = 0, j = 0, inFileNameLen = 0, inSlash = 0;
        char		szTemplate[60 + 1], szFileName[60 + 1];
	char		szDebugMsg[100 + 1];
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCCTMS_DeleteAllDownloadFile_ISO8583() START !");
	}
	
	if (inFILE_Check_Exist((unsigned char *)_TMSFLT_FILE_NAME_) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
        for (i = 0;; i++)
        {
                /* Load TMS File List 取得檔名 */
                if (inLoadTMSFLTRec(i) < 0)
                        break;

                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetTMSFilePathName(szTemplate);

                /* 取得檔案路徑後 抓取檔名儲存到FileName */
                for (j = 0; j < 60 ; j++)
                {
                        if (szTemplate[j] == '/')
                            inSlash = j + 1;

                        if (szTemplate[j] == 0x00)
                            break;
                }

		/* 斜線後的檔名長度*/
                inFileNameLen = j - inSlash;
                memset(szFileName, 0x00, sizeof(szFileName));
                memcpy(&szFileName[0], &szTemplate[inSlash], inFileNameLen);

		inFILE_Delete((unsigned char *)szFileName);
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "刪除%s", szFileName);
			inLogPrintf(AT, szDebugMsg);
		}
		
        }
	
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "刪除FileList");
	}

	inFILE_Delete((unsigned char *)_TMSFLT_FILE_NAME_);
		
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCCTMS_DeleteAllDownloadFile_ISO8583() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCCTMS_DeleteAllDownloadFile_FTPS
Date&Time       :2017/1/25 下午 4:33
Describe        :下載檔案全刪
*/
int inNCCCTMS_DeleteAllDownloadFile_FTPS()
{
	int		i = 0;
        char		szFileName[60 + 1];
	char		szDebugMsg[100 + 1];
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCCTMS_DeleteAllDownloadFile_FTPS() START !");
	}
	
	if (inFILE_Check_Exist((unsigned char *)_FTPFLT_FILE_NAME_) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
        for (i = 0;; i++)
        {
                /* Load TMS File List 取得檔名 */
                if (inLoadFTPFLTRec(i) < 0)
                        break;

                memset(szFileName, 0x00, sizeof(szFileName));
                inGetFTPFileName(szFileName);

                
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "刪除%s", szFileName);
			inLogPrintf(AT, szDebugMsg);
		}
		inFILE_Delete((unsigned char *)szFileName);
        }
	
	/* 若有下載失敗，回傳錯誤並刪FileList */
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "刪除FileList");
	}
		
	inFILE_Delete((unsigned char *)_FTPFLT_FILE_NAME_);
	inFILE_Delete((unsigned char *)_FTP_INFO_2_);
	
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCCTMS_DeleteAllDownloadFile_FTPS() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCCTMS_Check_FileList_Flow
Date&Time       :2017/1/25 下午 2:53
Describe        :確認FileList，通訊分流，檢查存在哪一個FileList，就切成哪一種，
 *		這是為了避免設定為CFGT設定FTPS卻跑ISO8583流程，唯一有疑慮為若有FileList沒清乾淨會跑錯流程
*/
int inNCCCTMS_Check_FileList_Flow(TRANSACTION_OBJECT * pobTran)
{
	int	inRetVal = VS_SUCCESS;
	char	szDebugMsg[100 + 1] = {0};
	char	szCFESMode[2 + 1] = {0};
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCCTMS_Check_FileList_Flow() START !");
	}
	
	while (1)
	{
		/* ISO8583 */
		if (inFILE_Check_Exist((unsigned char *)_TMSFLT_FILE_NAME_) == VS_SUCCESS)
		{
			pobTran->uszFTP_TMS_Download = _TMS_DOWNLOAD_SECURE_MODE_NONE_;
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "inNCCCTMS_Check_FileList Choose ISO8583");
				inLogPrintf(AT, szDebugMsg);
			}
			break;
		}
		/* FTPS */
		else if (inFILE_Check_Exist((unsigned char *)_FTPFLT_FILE_NAME_) == VS_SUCCESS)
		{
			memset(szCFESMode, 0x00, sizeof(szCFESMode));
			inGetCloud_MFES(szCFESMode);
			if (memcmp(szCFESMode, "Y", strlen("Y")) == 0)
			{
				pobTran->uszFTP_TMS_Download = _TMS_DOWNLOAD_SECURE_MODE_SFTP_;
			}
			else
			{
				pobTran->uszFTP_TMS_Download = _TMS_DOWNLOAD_SECURE_MODE_FTPS_;
			}
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "inNCCCTMS_Check_FileList Choose FTPS");
				inLogPrintf(AT, szDebugMsg);
			}
			break;
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "inNCCCTMS_Check_FileList No Filelist");
				inLogPrintf(AT, szDebugMsg);
			}
			inRetVal = VS_ERROR;
			break;
		}
		
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCCTMS_Check_FileList_Flow() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :inNCCCTMS_Delete_FileList_Flow
Date&Time       :2017/1/25 下午 2:53
Describe        :刪除FileList，通訊分流
*/
int inNCCCTMS_Delete_FileList_Flow(TRANSACTION_OBJECT * pobTran)
{
	int	inRetVal;
	char	szDebugMsg[100 + 1];
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCCTMS_Delete_FileList_Flow() START !");
	}
	
	/* FTPS */
	if (pobTran->uszFTP_TMS_Download == _TMS_DOWNLOAD_SECURE_MODE_FTPS_	||
	    pobTran->uszFTP_TMS_Download == _TMS_DOWNLOAD_SECURE_MODE_SFTP_)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inNCCCTMS_Delete_FileList_Flow Choose FTPS");
			inLogPrintf(AT, szDebugMsg);
		}
		inRetVal = inFILE_Delete((unsigned char *)_FTPFLT_FILE_NAME_);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inNCCCTMS_Delete_FileList_Flow Choose ISO8583");
			inLogPrintf(AT, szDebugMsg);
		}
		inRetVal = inFILE_Delete((unsigned char *)_TMSFLT_FILE_NAME_);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCCTMS_Delete_FileList_Flow() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :inNCCCTMS_Initial_HDPT
Date&Time       :2017/1/3 下午 4:52
Describe        :若參數生效，初始化HDPT
*/
int inNCCCTMS_Initial_HDPT()
{
	char	szCustomerIndicator[3 + 1] = {0};
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	inHDPT_Initial_AllRercord(gszParamDBPath, _HDPT_TABLE_NAME_);

	/* NCCC DCC */
	if (inNCCCTMS_HDT_AND_HDPT_Relist(_HOST_NAME_CREDIT_NCCC_, _HOST_NAME_DCC_, _TRT_FILE_NAME_CREDIT_, _TRT_FILE_NAME_DCC_) == VS_SUCCESS)
	{

	}
	else
	{
		/* 只要Relist失敗就有可能造成 HDT和HDPT的順序不同 */
		inFunc_EDCLock(AT);
	}

	/* DCC ESVC */
	if (inNCCCTMS_HDT_AND_HDPT_Relist(_HOST_NAME_DCC_, _HOST_NAME_ESVC_, _TRT_FILE_NAME_DCC_, _TRT_FILE_NAME_ESVC_) == VS_SUCCESS)
	{

	}
	else
	{
		/* 只要Relist失敗就有可能造成 HDT和HDPT的順序不同 */
		inFunc_EDCLock(AT);
	}

	/* 要清空，否則DCC Download會抓到錯的index */
	ginDCCHostIndex = -1;
	ginHGHostIndex = -1;
	ginESVCHostIndex = -1;
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCCTMS_Initial_HDPT
Date&Time       :2018/5/22 下午 6:08
Describe        :初始化EDC，
 *		2018/7/30 下午 3:31廢棄
*/
int inNCCCTMS_Initial_EDC()
{
	/* 重下TMS要設定回144 */
	inSetECRVersion("01");
	inSaveEDCRec(0);
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCCTMS_Table_Relist
Date&Time       :2017/1/3 下午 5:50
Describe        :為了讓DCC接在HOST後面，所以重新排Record，但不改index
*/
int inNCCCTMS_Table_Relist(char *szFileName, int inOldIndex, int inNewIndex)
{
	/* lnTotalLen : 該檔案全長度 ,		inRecLen : 一個Record的長度 */
	/* lnFinalToalLen :Resort完的長度，用來確認檔案完整性 */
        /* szFile : 該檔案全部內容 ,		szRecord : 該Record全部內容 */

	int			i;
	int			inOldRecLen = 0;		/* Old Record 長度 */
	int			inRecCnt = 0;
	int			inOldRecStartOffset = 0;	/* Old Record 起始位置 */
	int			inNewRecStartOffset = 0;	/* New Record 起始位置 */
	int			inRetVal;
        char			*szFile, *szRecord;
	char			szDebugMsg[100 + 1];
	long			lnTotalLen, lnFinalToalLen;
	long			lnLeftSize = 0;
	unsigned long		ulHandle;
	
	/* index不合法 */
	if (inOldIndex == -1 || inNewIndex == -1)
	{
		return (VS_ERROR);
	}

        inRetVal = inFILE_Open(&ulHandle, (unsigned char*)szFileName);
        if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
        
	/* 算總長度 */
        lnTotalLen = lnFILE_GetSize(&ulHandle, (unsigned char*)szFileName);
	
	/* 讀出HDT所有資料 */
        inFILE_Seek(ulHandle, 0L, _SEEK_BEGIN_);
	szFile = malloc((lnTotalLen + 1) * sizeof(char));
	memset(szFile, 0x00, lnTotalLen * sizeof(char));
	lnLeftSize = lnTotalLen;
	
	for (i = 0;; ++i)
	{
		/* 剩餘長度大於或等於1024 */
		if (lnLeftSize >= 1024)
		{
			if (inFILE_Read(&ulHandle, (unsigned char*)&szFile[1024 * i], 1024) == VS_SUCCESS)
			{
				/* 一次讀1024 */
				lnLeftSize -= 1024;

				/* 當剩餘長度剛好為1024，會剛好讀完 */
				if (lnLeftSize == 0)
					break;
			}
			/* 讀失敗時 */
			else
			{
				/* Close檔案 */
				inFILE_Close(&ulHandle);

				/* Free pointer */
				free(szFile);

				return (VS_ERROR);
			}
		}
		/* 剩餘長度小於1024 */
		else if (lnLeftSize < 1024)
		{
			/* 就只讀剩餘長度 */
			if (inFILE_Read(&ulHandle, (unsigned char*)&szFile[1024 * i], lnLeftSize) == VS_SUCCESS)
			{
				break;
			}
			/* 讀失敗時 */
			else
			{
				/* Close檔案 */
				inFILE_Close(&ulHandle);

				/* Free pointer */
				free(szFile);

				return (VS_ERROR);
			}
			
		}
		
	}

	/* 算出Old Record的長度 */
	for (i = 0, inRecCnt = 0; i < lnTotalLen; i++)
	{
		/* Record 長度 */
		if (inRecCnt == inOldIndex)
		{
			inOldRecLen++;
		}
		/* 若大於，代表已經算完，要跳出 */
		else if (inRecCnt > inOldIndex)
		{
			break;
		}
		
		/* Record 結尾 */
		if (szFile[i] == 0x0A && szFile[i - 1] == 0x0D)
		{
			inRecCnt++;
			
			/* 算出Record起始位置 若是第0 Record，永遠不會進去，則inOldRecStartOffset 為 0 */
			if (inRecCnt == inOldIndex)
			{
				inOldRecStartOffset = i + 1;
			}
		}
		
	}
	
	/* 算出New Record的起始位置 */
	for (i = 0, inRecCnt = 0; i < lnTotalLen; i++)
	{
		/* Record 結尾 */
		if (szFile[i] == 0x0A && szFile[i - 1] == 0x0D)
		{
			inRecCnt++;
			
			/* 算出Record起始位置 若是第0 Record，永遠不會進去，則inOldRecStartOffset 為 0 */
			if (inRecCnt == inNewIndex)
			{
				inNewRecStartOffset = i + 1;
				break;
				
			}
			else if (inNewIndex == 0)
			{
				inNewRecStartOffset = 0;
				break;
			}
			
		}
		
	}
	
	/* 移到Record的起始位置 */
	if (inFILE_Seek(ulHandle, inOldRecStartOffset, _SEEK_BEGIN_) != VS_SUCCESS)
	{
		inFILE_Close(&ulHandle);
		free(szFile);
		
		return (VS_ERROR);
	}
	
	/* 讀出Record的資料 */
	szRecord = malloc((inOldRecLen + 1) * sizeof(char));
	memset(szRecord, 0x00, inOldRecLen * sizeof(char));
	if (inFILE_Read(&ulHandle, (unsigned char*)szRecord, inOldRecLen) != VS_SUCCESS)
	{
		inFILE_Close(&ulHandle);
		free(szRecord);
		free(szFile);
		
		return (VS_ERROR);
	}
	
	/* 因這裡V3模仿Verifone的insert和delete，但用自己的方式實做，所以需要先關檔再由該function操作*/
	if (inFILE_Close(&ulHandle) != VS_SUCCESS)
	{
		free(szRecord);
		free(szFile);
		
		return (VS_ERROR);
	}
	
	/* 釋放記憶體 */
        free(szFile);
	
	if (inFILE_Data_Delete((unsigned char*)szFileName, inOldRecStartOffset, inOldRecLen) != VS_SUCCESS)	/* 刪除DCC那一段 */
	{
		/* 釋放記憶體 */
		free(szRecord);
		
		return (VS_ERROR);
	}
	
	if (inFILE_Data_Insert((unsigned char*)szFileName, inNewRecStartOffset, inOldRecLen, (unsigned char*)szRecord) != VS_SUCCESS)		/* 把DCC的內容插入 */
	{
		/* 釋放記憶體 */
		free(szRecord);
		
		return (VS_ERROR);
	}
	
	/* 釋放記憶體 */
	free(szRecord);
	
	/* 最後做檢查，產生檔案長度是否和原來一樣 */
	lnFinalToalLen = lnFILE_GetSize(&ulHandle, (unsigned char*)szFileName);
	if (lnFinalToalLen != lnTotalLen)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Relist Failed");
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_ERROR);
	}

        return (VS_SUCCESS);
}

/*
Function        :inNCCCTMS_HDT_AND_HDPT_Relist
Date&Time       :2018/3/21 下午 2:31
Describe        :順序為NCCC DCC ESVC HG
 *		:szLeadHostName:填要當基準的HostName
 *		:szFollowHostName:填要調整的HostName
*/
int inNCCCTMS_HDT_AND_HDPT_Relist(char* szLeadHostName, char* szFollowHostName, char* szLeadTRTName, char* szFollowTRTName)
{
	int	inRetVal;
	int	inLeadIndex = -1;
	int	inOldFollowIndex = -1;
	int	inNewFollowIndex = -1;
	char	szDebugMsg[100 + 1] = {0};
	
	inFunc_Find_Specific_HDTindex(0, szFollowHostName, &inOldFollowIndex);
	inFunc_Find_Specific_HDTindex(0, szLeadHostName, &inLeadIndex);
	
	/* index不合法 */
	if (inOldFollowIndex == -1 || inLeadIndex == -1)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "HDT old:%d lead:%d", inOldFollowIndex, inLeadIndex);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_SUCCESS);
	}
	
	if (inOldFollowIndex > inLeadIndex)
	{
		inNewFollowIndex = inLeadIndex + 1;
	}
	else
	{
		inNewFollowIndex = inLeadIndex;
	}
	
	inRetVal = inFILE_Copy_File((unsigned char*)_HDT_FILE_NAME_, (unsigned char*)_HDT_FILE_NAME_BAK_);
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	inRetVal = inNCCCTMS_Table_Relist(_HDT_FILE_NAME_BAK_, inOldFollowIndex, inNewFollowIndex);
	if (inRetVal != VS_SUCCESS)
	{
		inFILE_Delete((unsigned char*)_HDT_FILE_NAME_BAK_);
		
		return (VS_ERROR);
	}
	else
	{
		inFILE_Delete((unsigned char*)_HDT_FILE_NAME_);
		inFILE_Rename((unsigned char*)_HDT_FILE_NAME_BAK_, (unsigned char*)_HDT_FILE_NAME_);
	}
	
	/* HDPT同步 */
	inHDPT_Initial_AllRercord(gszParamDBPath, _HDPT_TABLE_NAME_);
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCCTMS_HDPT_Relist_Flow
Date&Time       :2019/5/22 下午 2:09
Describe        :
*/
int inNCCCTMS_HDPT_Relist_Flow(char* szLeadTRTName, char* szFollowTRTName)
{
	int	inRetVal = VS_SUCCESS;
	
	if (_PARAMETER_SAVE_WAY_DAT_ == _PARAMETER_SAVE_WAY_DAT_SQLITE_)
	{
		inRetVal = inNCCCTMS_HDPT_Relist_SQLite(szLeadTRTName, szFollowTRTName);
	}
	else
	{
		inRetVal = inNCCCTMS_HDPT_Relist(szLeadTRTName, szFollowTRTName);
	}
	
	return (inRetVal);
}

/*
Function        :inNCCCTMS_HDPT_Relist_Flow
Date&Time       :2019/5/22 下午 2:09
Describe        :
*/
int inNCCCTMS_HDPT_Relist(char* szLeadTRTName, char* szFollowTRTName)
{
	int	inRetVal = VS_SUCCESS;
	int	inLeadTRTIndex = -1;
	int	inOldFollowTRTindex = -1;
	int	inNewFollowTRTindex = -1;
	char	szDebugMsg[100 + 1] = {0};
	
	inFunc_Find_Specific_HDPTindex_Flow(0, szFollowTRTName, &inOldFollowTRTindex);
	inFunc_Find_Specific_HDPTindex_Flow(0, szLeadTRTName, &inLeadTRTIndex);

	/* index不合法 */
	if (inOldFollowTRTindex == -1 || inLeadTRTIndex == -1)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "HDPT old:%d lead:%d", inOldFollowTRTindex, inLeadTRTIndex);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}

	inNewFollowTRTindex = inLeadTRTIndex + 1;
	
	/* index位置一樣，HDPT不用調整 */
	if (inOldFollowTRTindex == inNewFollowTRTindex)
	{
		return (VS_SUCCESS);
	}

	inFILE_Delete((unsigned char*)_HDPT_FILE_NAME_BAK_);
	inRetVal = inFILE_Copy_File((unsigned char*)_HDPT_FILE_NAME_, (unsigned char*)_HDPT_FILE_NAME_BAK_);
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	inRetVal = inNCCCTMS_Table_Relist(_HDPT_FILE_NAME_BAK_, inOldFollowTRTindex, inNewFollowTRTindex);
	if (inRetVal != VS_SUCCESS)
	{
		inFILE_Delete((unsigned char*)_HDPT_FILE_NAME_BAK_);

		return (VS_ERROR);
	}
	else
	{
		inFILE_Delete((unsigned char*)_HDPT_FILE_NAME_);
		inFILE_Rename((unsigned char*)_HDPT_FILE_NAME_BAK_, (unsigned char*)_HDPT_FILE_NAME_);
	}
	
	return (inRetVal);
}

/*
Function        :inNCCCTMS_HDPT_Relist_SQLite
Date&Time       :2019/5/22 下午 2:29
Describe        :
*/
int inNCCCTMS_HDPT_Relist_SQLite(char* szLeadTRTName, char* szFollowTRTName)
{
	int	inRetVal = VS_SUCCESS;
	int	inLeadTRTIndex = -1;
	int	inOldFollowTRTindex = -1;
	int	inNewFollowTRTindex = -1;
	char	szDebugMsg[100 + 1] = {0};
	
	inFunc_Find_Specific_HDPTindex_Flow(0, szFollowTRTName, &inOldFollowTRTindex);
	inFunc_Find_Specific_HDPTindex_Flow(0, szLeadTRTName, &inLeadTRTIndex);

	/* index不合法 */
	if (inOldFollowTRTindex == -1 || inLeadTRTIndex == -1)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "HDPT old:%d lead:%d", inOldFollowTRTindex, inLeadTRTIndex);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}

	inNewFollowTRTindex = inLeadTRTIndex + 1;
	
	/* index位置一樣，HDPT不用調整 */
	if (inOldFollowTRTindex == inNewFollowTRTindex)
	{
		return (VS_SUCCESS);
	}

	inSqlite_Drop_Table(gszParamDBPath, _HDPT_TABLE_NAME_BAK_);
	inEDCPara_Create_Table_Flow(gszParamDBPath, _HDPT_TABLE_NAME_BAK_, TABLE_HDPT_TAG);
	inRetVal = inSqlite_Copy_Table_Data(gszParamDBPath, _HDPT_TABLE_NAME_, _HDPT_TABLE_NAME_BAK_);
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	inRetVal = inSqlite_Table_Relist_SQLite(gszParamDBPath, _HDPT_TABLE_NAME_BAK_, inOldFollowTRTindex, inNewFollowTRTindex);
	if (inRetVal != VS_SUCCESS)
	{
		inSqlite_Drop_Table(gszParamDBPath, _HDPT_TABLE_NAME_BAK_);

		return (VS_ERROR);
	}
	else
	{
		inSqlite_Drop_Table(gszParamDBPath, _HDPT_TABLE_NAME_);
		inSqlite_Rename_Table(gszParamDBPath, _HDPT_TABLE_NAME_BAK_, _HDPT_TABLE_NAME_);
	}
	
	return (inRetVal);
}

/*
Function        :inNCCCTMS_FTPFLT_Delete_AP_Attribute_A
Date&Time       :2017/1/3 下午 5:50
Describe        :為了讓FTP拔掉AP那一欄Record
*/
int inNCCCTMS_FTPFLT_Delete_AP_Attribute_A(char *szFileName)
{
	/* lnTotalLen : 該檔案全長度 ,		inRecLen : 一個Record的長度 */
	/* lnFinalToalLen :Resort完的長度，用來確認檔案完整性 */
        /* szFile : 該檔案全部內容 ,		szRecord : 該Record全部內容 */

	int			i;
	int			inOldIndex = -1;
	int			inOldRecLen = 0;		/* Old Record 長度 */
	int			inRecCnt = 0;
	int			inOldRecStartOffset = 0;	/* Old Record 起始位置 */
	int			inRetVal;
        char			*szFile, *szRecord;
	char			szFTPFileAttribute[2 + 1];	/* 檔案屬性 */
	long			lnTotalLen = 0;
	long			lnLeftSize = 0;			/* 剩餘長度 */
	unsigned long		ulHandle;
	
	for (i = 0 ;; i ++)
        {  
		if (inLoadFTPFLTRec(i) < 0)
                        break;
		
		memset(szFTPFileAttribute, 0x00, sizeof(szFTPFileAttribute));
                inGetFTPFileAttribute(szFTPFileAttribute);
		
		/* 須判斷是否下AP 手動下載不下AP */
                if (memcmp(szFTPFileAttribute, "A", 1) == 0)
		{
			inOldIndex = i;
			break;
		}
	}
	
	/* index不合法 */
	if (inOldIndex == -1)
	{
		return (VS_ERROR);
	}

        inRetVal = inFILE_Open(&ulHandle, (unsigned char*)szFileName);
        if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
        
	/* 算總長度 */
        lnTotalLen = lnFILE_GetSize(&ulHandle, (unsigned char*)szFileName);
	
	/* 讀出HDT所有資料 */
        inFILE_Seek(ulHandle, 0L, _SEEK_BEGIN_);
	szFile = malloc((lnTotalLen + 1) * sizeof(char));
	memset(szFile, 0x00, lnTotalLen * sizeof(char));
	lnLeftSize = lnTotalLen;
	
	for (i = 0;; ++i)
	{
		/* 剩餘長度大於或等於1024 */
		if (lnLeftSize >= 1024)
		{
			if (inFILE_Read(&ulHandle, (unsigned char*)&szFile[1024 * i], 1024) == VS_SUCCESS)
			{
				/* 一次讀1024 */
				lnLeftSize -= 1024;

				/* 當剩餘長度剛好為1024，會剛好讀完 */
				if (lnLeftSize == 0)
					break;
			}
			/* 讀失敗時 */
			else
			{
				/* Close檔案 */
				inFILE_Close(&ulHandle);

				/* Free pointer */
				free(szFile);

				return (VS_ERROR);
			}
		}
		/* 剩餘長度小於1024 */
		else if (lnLeftSize < 1024)
		{
			/* 就只讀剩餘長度 */
			if (inFILE_Read(&ulHandle, (unsigned char*)&szFile[1024 * i], lnLeftSize) == VS_SUCCESS)
			{
				break;
			}
			/* 讀失敗時 */
			else
			{
				/* Close檔案 */
				inFILE_Close(&ulHandle);

				/* Free pointer */
				free(szFile);

				return (VS_ERROR);
			}
			
		}
		
	}
	
	/* 算出Old Record的長度 */
	for (i = 0, inRecCnt = 0; i < lnTotalLen; i++)
	{
		/* Record 長度 */
		if (inRecCnt == inOldIndex)
		{
			inOldRecLen++;
		}
		/* 若大於，代表已經算完，要跳出 */
		else if (inRecCnt > inOldIndex)
		{
			break;
		}
		
		/* Record 結尾 */
		if (szFile[i] == 0x0A && szFile[i - 1] == 0x0D)
		{
			inRecCnt++;
			
			/* 算出Record起始位置 若是第0 Record，永遠不會進去，則inOldRecStartOffset 為 0 */
			if (inRecCnt == inOldIndex)
			{
				inOldRecStartOffset = i + 1;
			}
		}
		
	}
	
	/* 移到Record的起始位置 */
	if (inFILE_Seek(ulHandle, inOldRecStartOffset, _SEEK_BEGIN_) != VS_SUCCESS)
	{
		inFILE_Close(&ulHandle);
		free(szFile);
		
		return (VS_ERROR);
	}
	
	/* 讀出Record的資料 */
	szRecord = malloc((inOldRecLen + 1) * sizeof(char));
	memset(szRecord, 0x00, inOldRecLen * sizeof(char));
	if (inFILE_Read(&ulHandle, (unsigned char*)szRecord, inOldRecLen) != VS_SUCCESS)
	{
		inFILE_Close(&ulHandle);
		free(szRecord);
		free(szFile);
		
		return (VS_ERROR);
	}
	
	/* 因這裡V3模仿Verifone的insert和delete，但用自己的方式實做，所以需要先關檔再由該function操作*/
	if (inFILE_Close(&ulHandle) != VS_SUCCESS)
	{
		free(szRecord);
		free(szFile);
		
		return (VS_ERROR);
	}
	
	/* 釋放記憶體 */
        free(szFile);
	
	if (inFILE_Data_Delete((unsigned char*)szFileName, inOldRecStartOffset, inOldRecLen) != VS_SUCCESS)	/* 刪除DCC那一段 */
	{
		/* 釋放記憶體 */
		free(szRecord);
		
		return (VS_ERROR);
	}
	
	/* 釋放記憶體 */
	free(szRecord);

        return (VS_SUCCESS);
}

/*
Function        :inNCCCTMS_FTPFLT_Delete_AP_Attribute_R
Date&Time       :2018/6/26 下午 3:18
Describe        :為了讓FTP拔掉AP那一欄Record，整合型週邊為R，理論上不會有，做起來防呆
*/
int inNCCCTMS_FTPFLT_Delete_AP_Attribute_R(char *szFileName)
{
	/* lnTotalLen : 該檔案全長度 ,		inRecLen : 一個Record的長度 */
	/* lnFinalToalLen :Resort完的長度，用來確認檔案完整性 */
        /* szFile : 該檔案全部內容 ,		szRecord : 該Record全部內容 */

	int			i;
	int			inOldIndex = -1;
	int			inOldRecLen = 0;		/* Old Record 長度 */
	int			inRecCnt = 0;
	int			inOldRecStartOffset = 0;	/* Old Record 起始位置 */
	int			inRetVal;
        char			*szFile, *szRecord;
	char			szFTPFileAttribute[2 + 1];	/* 檔案屬性 */
	long			lnTotalLen = 0;
	long			lnLeftSize = 0;			/* 剩餘長度 */
	unsigned long		ulHandle;
	
	for (i = 0 ;; i ++)
        {  
		if (inLoadFTPFLTRec(i) < 0)
                        break;
		
		memset(szFTPFileAttribute, 0x00, sizeof(szFTPFileAttribute));
                inGetFTPFileAttribute(szFTPFileAttribute);
		
		/* 須判斷是否下AP 手動下載不下AP */
                if (memcmp(szFTPFileAttribute, "R", 1) == 0)
		{
			inOldIndex = i;
			break;
		}
	}
	
	/* index不合法 */
	if (inOldIndex == -1)
	{
		return (VS_ERROR);
	}

        inRetVal = inFILE_Open(&ulHandle, (unsigned char*)szFileName);
        if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
        
	/* 算總長度 */
        lnTotalLen = lnFILE_GetSize(&ulHandle, (unsigned char*)szFileName);
	
	/* 讀出HDT所有資料 */
        inFILE_Seek(ulHandle, 0L, _SEEK_BEGIN_);
	szFile = malloc((lnTotalLen + 1) * sizeof(char));
	memset(szFile, 0x00, lnTotalLen * sizeof(char));
	lnLeftSize = lnTotalLen;
	
	for (i = 0;; ++i)
	{
		/* 剩餘長度大於或等於1024 */
		if (lnLeftSize >= 1024)
		{
			if (inFILE_Read(&ulHandle, (unsigned char*)&szFile[1024 * i], 1024) == VS_SUCCESS)
			{
				/* 一次讀1024 */
				lnLeftSize -= 1024;

				/* 當剩餘長度剛好為1024，會剛好讀完 */
				if (lnLeftSize == 0)
					break;
			}
			/* 讀失敗時 */
			else
			{
				/* Close檔案 */
				inFILE_Close(&ulHandle);

				/* Free pointer */
				free(szFile);

				return (VS_ERROR);
			}
		}
		/* 剩餘長度小於1024 */
		else if (lnLeftSize < 1024)
		{
			/* 就只讀剩餘長度 */
			if (inFILE_Read(&ulHandle, (unsigned char*)&szFile[1024 * i], lnLeftSize) == VS_SUCCESS)
			{
				break;
			}
			/* 讀失敗時 */
			else
			{
				/* Close檔案 */
				inFILE_Close(&ulHandle);

				/* Free pointer */
				free(szFile);

				return (VS_ERROR);
			}
			
		}
		
	}
	
	/* 算出Old Record的長度 */
	for (i = 0, inRecCnt = 0; i < lnTotalLen; i++)
	{
		/* Record 長度 */
		if (inRecCnt == inOldIndex)
		{
			inOldRecLen++;
		}
		/* 若大於，代表已經算完，要跳出 */
		else if (inRecCnt > inOldIndex)
		{
			break;
		}
		
		/* Record 結尾 */
		if (szFile[i] == 0x0A && szFile[i - 1] == 0x0D)
		{
			inRecCnt++;
			
			/* 算出Record起始位置 若是第0 Record，永遠不會進去，則inOldRecStartOffset 為 0 */
			if (inRecCnt == inOldIndex)
			{
				inOldRecStartOffset = i + 1;
			}
		}
		
	}
	
	/* 移到Record的起始位置 */
	if (inFILE_Seek(ulHandle, inOldRecStartOffset, _SEEK_BEGIN_) != VS_SUCCESS)
	{
		inFILE_Close(&ulHandle);
		free(szFile);
		
		return (VS_ERROR);
	}
	
	/* 讀出Record的資料 */
	szRecord = malloc((inOldRecLen + 1) * sizeof(char));
	memset(szRecord, 0x00, inOldRecLen * sizeof(char));
	if (inFILE_Read(&ulHandle, (unsigned char*)szRecord, inOldRecLen) != VS_SUCCESS)
	{
		inFILE_Close(&ulHandle);
		free(szRecord);
		free(szFile);
		
		return (VS_ERROR);
	}
	
	/* 因這裡V3模仿Verifone的insert和delete，但用自己的方式實做，所以需要先關檔再由該function操作*/
	if (inFILE_Close(&ulHandle) != VS_SUCCESS)
	{
		free(szRecord);
		free(szFile);
		
		return (VS_ERROR);
	}
	
	/* 釋放記憶體 */
        free(szFile);
	
	if (inFILE_Data_Delete((unsigned char*)szFileName, inOldRecStartOffset, inOldRecLen) != VS_SUCCESS)	/* 刪除DCC那一段 */
	{
		/* 釋放記憶體 */
		free(szRecord);
		
		return (VS_ERROR);
	}
	
	/* 釋放記憶體 */
	free(szRecord);

        return (VS_SUCCESS);
}

/*
Function        :inNCCCTMS_Process_AP
Date&Time       :2017/5/8 下午 4:22
Describe        :移到pub下並解壓縮
*/
int inNCCCTMS_Process_AP()
{
	int	inRetVal = VS_ERROR;
	char	szPathName[200 + 1] = {0};
	char	szFileName[100 + 1] = {0};
	
	inFunc_Clear_AP_Dump();
	
	if (inFILE_Check_Exist((unsigned char*)_APPL_NAME_) != VS_SUCCESS)
	{
		vdUtility_SYSFIN_LogMessage(AT, "inNCCCTMS_Process_AP inFILE_Check_Exist_failed(%s)", _APPL_NAME_);
		return (VS_ERROR);
	}
	inFunc_Dir_Make(_APP_UPDATE_DIR_NAME_, _FS_DATA_PATH_);
	inFile_Move_File(_APPL_NAME_, _FS_DATA_PATH_, "", _APP_UPDATE_PATH_);
	
	inFunc_Unzip("-o", _APPL_NAME_, _APP_UPDATE_PATH_, "-d", _APP_UPDATE_PATH_);
	inFile_Unlink_File(_APPL_NAME_, _APP_UPDATE_PATH_);
	
	inNCCCTMS_Change_AP_Filesize_To_0();
	
	if (ginDebug == VS_TRUE)
	{
		inFunc_ls("-R", _APP_UPDATE_PATH_);
	}
	
	memset(szFileName, 0x00, sizeof(szFileName));
	inNCCCTMS_Get_Filelist_Name(szFileName);
	
	memset(szPathName, 0x00, sizeof(szPathName));
	sprintf(szPathName, "%s%s", _APP_UPDATE_PATH_, szFileName);
	
	inRetVal = inFunc_Update_AP(szPathName);
	
	return (inRetVal);
}

/*
Function        :inNCCCTMS_IFES_GET_FTP_IDPW
Date&Time       :2017/8/4 下午 5:06
Describe        :
*/
int inNCCCTMS_IFES_GET_FTP_IDPW(TRANSACTION_OBJECT * pobTran)
{
	int		inRetVal;
	char		szTemplate[64 + 1];
	TMS_OBJECT	srTMS;

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCCTMS_IFES_GET_FTP_IDPW() START !");
	}

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_ChineseFont_Color("詢問FTPS ID/PW", _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_BLACK_, _DISP_LEFT_);

	memset((char *)&srTMS, 0x00, sizeof(TMS_OBJECT));

        srTMS.inCode = _NCCCTMS_LOGON_FTPIDPW_;
	srTMS.uszRequest = 'Y'; /* 用來判斷是否要繼續【下載】或【上傳】 */
	srTMS.inTransactionResult = VS_SUCCESS; /* Response Code 使用 */

	srTMS.inPackNo = 1;
        
	strcpy(srTMS.szTotalPacketCount, "0000"); /* 初始化 */

	strcpy(srTMS.szPacketNo, "0001"); /* 初始化 */

        /* TPDU */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        /* TMS NII 固定292 */
	inGetTMSNII(szTemplate);
        memset(srTMS.szTPDU, 0x00, sizeof(srTMS.szTPDU));
        sprintf(srTMS.szTPDU, "600%s0000", szTemplate);
        /* NII */
        memset(szTemplate, 0x00, sizeof(szTemplate));
	/* TMS NII 固定292 */
        inGetTMSNII(szTemplate);
        memset(srTMS.szNII, 0x00, sizeof(srTMS.szNII));
        sprintf(srTMS.szNII, "0%s", szTemplate);
        
        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	
	do
	{
		/* 開始組交易封包，組、送、收、解 */
		inRetVal = inNCCCTMS_SendPackRecvUnPack(pobTran, &srTMS);

		if (inRetVal == VS_ERROR)
		{
			/* 先定義為通訊失敗 */
			srTMS.inTransactionResult = _NCCCTMS_COMM_ERROR_;
			srTMS.uszRequest = 'N';
		}
		else if (inRetVal == _NCCCTMS_PACK_ERR_ || inRetVal == _NCCCTMS_UNPACK_ERR_)
		{
			srTMS.inTransactionResult = inRetVal;
			srTMS.uszRequest = 'N';
		}
		else
		{
			srTMS.inTransactionResult = inNCCCTMS_CheckRespCode(&srTMS); /* 【Field_39】 */

			if (srTMS.inTransactionResult != _NCCCTMS_AUTHORIZED_)
			{
				srTMS.uszRequest = 'N'; /* 主機沒有回回覆碼 */
			}
		}

		/* Analyse */
		inRetVal = inNCCCTMS_ISOAnalyse(&srTMS);
		
		/* 電文分析結果失敗 */
		if (inRetVal == VS_ERROR)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_PROCESS_, 0, _COORDINATE_Y_LINE_8_7_);

			srTMS.uszRequest = 'N';

			/* 下載失敗要補檢查檔案狀態，inNCCCTMS_ISOAnalyse內只有全部檔案下載完才檢查 */
			/* 檢查下載的檔案大小與File List比對 */
			inNCCCTMS_CheckFileSize(&srTMS);
		}
	} while (srTMS.uszRequest == 'Y');
	
	/* 螢幕提示TMS下載訊息 */
        inNCCCTMS_DispHostResponseCode(&srTMS);
	
	/* 不論成功或失敗，都要帶回復碼回來 */
	strcpy(pobTran->srBRec.szRespCode, srTMS.szRespCode);
	pobTran->inTransactionResult = srTMS.inTransactionResult;

	if (srTMS.inTransactionResult == _NCCCTMS_AUTHORIZED_)
	{
		return (VS_SUCCESS);
	}
	else
	{
		return (VS_ERROR);
	}
}

/*
Function        :inNCCCTMS_Backup_CPT_Parameter
Date&Time       :2017/9/15 上午 10:33
Describe        :TMS內CPT的需求，若IP和port沒有值，則用原本的
*/
int inNCCCTMS_Backup_CPT_Parameter()
{
	inFILE_Delete((unsigned char *)_CPT_BACKUP_FILE_NAME_);
	inFILE_Copy_File((unsigned char *)_CPT_FILE_NAME_, (unsigned char *)_CPT_BACKUP_FILE_NAME_);
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCCTMS_Recover_CPT_Parameter
Date&Time       :2017/9/15 上午 10:33
Describe        :檢查CPT內每一個IP 和 port
*/
int inNCCCTMS_Recover_CPT_Parameter()
{
	int	i = 0;
	char	szIP[15 + 1];
	char	szPort[5 + 1];
	char	szIP_Backup[15 + 1];
	char	szPort_Backup[5 + 1];
	char	szDebugMsg[100 + 1];
	
	if (inFILE_Check_Exist((unsigned char *)_CPT_BACKUP_FILE_NAME_) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "%s not exist, Not recover", _CPT_BACKUP_FILE_NAME_);
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	else
	{
		for (i = 0; ; i++)
		{
			if (inLoadCPTRec(i) != VS_SUCCESS)
			{
				break;
			}
			if (inLoadCPT_BackupRec(i) != VS_SUCCESS)
			{
				break;
			}

			/* 第一授權IP */
			memset(szIP, 0x00, sizeof(szIP));
			inGetHostIPPrimary(szIP);
			if (strlen(szIP) <= 0)
			{
				memset(szIP_Backup, 0x00, sizeof(szIP_Backup));
				inGetHostIPPrimary_Backup(szIP_Backup);
				
				if (strlen(szIP_Backup) > 0)
				{
					inSetHostIPPrimary(szIP_Backup);
					inSaveCPTRec(i);
				}
			}

			/* 第一授權Port */
			memset(szPort, 0x00, sizeof(szPort));
			inGetHostPortNoPrimary(szPort);
			if (strlen(szPort) <= 0)
			{
				memset(szPort_Backup, 0x00, sizeof(szPort_Backup));
				inGetHostPortNoPrimary_Backup(szPort_Backup);
				
				if (strlen(szPort_Backup) > 0)
				{
					inSetHostPortNoPrimary(szPort_Backup);
					inSaveCPTRec(i);
				}
			}
			
			/* 第二授權IP */
			memset(szIP, 0x00, sizeof(szIP));
			inGetHostIPSecond(szIP);
			if (strlen(szIP) <= 0)
			{
				memset(szIP_Backup, 0x00, sizeof(szIP_Backup));
				inGetHostIPSecond_Backup(szIP_Backup);
				
				if (strlen(szIP_Backup) > 0)
				{
					inSetHostIPSecond(szIP_Backup);
					inSaveCPTRec(i);
				}
			}

			/* 第二授權Port */
			memset(szPort, 0x00, sizeof(szPort));
			inGetHostPortNoSecond(szPort);
			if (strlen(szPort) <= 0)
			{
				memset(szPort_Backup, 0x00, sizeof(szPort_Backup));
				inGetHostPortNoSecond_Backup(szPort_Backup);
				
				if (strlen(szPort_Backup) > 0)
				{
					inSetHostPortNoSecond(szPort_Backup);
					inSaveCPTRec(i);
				}
			}

		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCCTMS_Sync_MCCCode
Date&Time       :2017/9/15 下午 3:53
Describe        :
*/
int inNCCCTMS_Sync_MCCCode()
{
	/* 將szMVTMerchantCategoryCode 同步到EDC.dat */
	char	szMVTMerchantCategoryCode[4 + 1];

	inLoadMVTRec(0);
	memset(szMVTMerchantCategoryCode, 0x00, sizeof(szMVTMerchantCategoryCode));
	inGetMVTMerchantCategoryCode(szMVTMerchantCategoryCode);

	inSetMCCCode(szMVTMerchantCategoryCode);
	inSaveEDCRec(0);
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCCTMS_DeInitial_Ticket
Date&Time       :2018/1/29 下午 6:02
Describe        :
*/
int inNCCCTMS_Initial_Ticket()
{
	inTDT_Initial_AllRercord(gszParamDBPath, _TDT_TABLE_NAME_);
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCCTMS_Initial_IPASS
Date&Time       :2018/1/29 下午 6:02
Describe        :
*/
int inNCCCTMS_Initial_IPASS()
{
	int	inESVC_HostIndex = -1;
	char	szTemplate[20 + 1];
	char	szTemplate2[20 + 1];
	
	inLoadIPASSDTRec(0);
	/* 有檔案代表有開Host */
	inLoadTDTRec(_TDT_INDEX_00_IPASS_);

	/* Ticket_HostIndex */
	inFunc_Find_Specific_HDTindex(0, _HOST_NAME_ESVC_, &inESVC_HostIndex);
	if (inESVC_HostIndex != -1)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%02d", inESVC_HostIndex);
		inSetTicket_HostIndex(szTemplate);
	}

	/* TransFunc */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetIPASS_Transaction_Function(szTemplate);
	inSetTicket_HostTransFunc(szTemplate);

	/* HostEnable */
	inSetTicket_HostEnable("Y");

	/* LogOn OK */
	inSetTicket_LogOnOK("N");

	/* 將SAM slot更新到TDT */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetIPASS_SAM_Slot(szTemplate);

	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	sprintf(szTemplate2, "%02d", atoi(szTemplate));
	inSetTicket_SAM_Slot(szTemplate2);

	/* ReaderID*/
	inSetTicket_ReaderID("    ");
	
	/* STAN */
	inSetTicket_STAN("000001");
	
	inSaveTDTRec(_TDT_INDEX_00_IPASS_);
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCCTMS_Initial_ECC
Date&Time       :2018/3/21 下午 2:23
Describe        :
*/
int inNCCCTMS_Initial_ECC()
{
	int	inESVC_HostIndex = -1;
	char	szTemplate[20 + 1];
	char	szTemplate2[20 + 1];
	
	inLoadECCDTRec(0);
	/* 有檔案代表有開Host */
	inLoadTDTRec(_TDT_INDEX_01_ECC_);

	/* Ticket_HostIndex */
	inFunc_Find_Specific_HDTindex(0, _HOST_NAME_ESVC_, &inESVC_HostIndex);
	if (inESVC_HostIndex != -1)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%02d", inESVC_HostIndex);
		inSetTicket_HostIndex(szTemplate);
	}

	/* TransFunc */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetECC_Transaction_Function(szTemplate);
	inSetTicket_HostTransFunc(szTemplate);

	/* HostEnable */
	inSetTicket_HostEnable("Y");

	/* LogOn OK */
	inSetTicket_LogOnOK("N");

	/* 將SAM slot更新到TDT */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetECC_SAM_Slot(szTemplate);

	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	sprintf(szTemplate2, "%02d", atoi(szTemplate));
	inSetTicket_SAM_Slot(szTemplate2);

	/* ReaderID*/
	inSetTicket_ReaderID("    ");

	/* STAN */
	inSetTicket_STAN("000001");
	
	inSaveTDTRec(_TDT_INDEX_01_ECC_);
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCCTMS_Initial_ICASH
Date&Time       :2019/10/2 上午 9:58
Describe        :
*/
int inNCCCTMS_Initial_ICASH()
{
	int	inESVC_HostIndex = -1;
	char	szTemplate[20 + 1];
	char	szTemplate2[20 + 1];
	
	inLoadICASHDTRec(0);
	/* 有檔案代表有開Host */
	inLoadTDTRec(_TDT_INDEX_02_ICASH_);

	/* Ticket_HostIndex */
	inFunc_Find_Specific_HDTindex(0, _HOST_NAME_ESVC_, &inESVC_HostIndex);
	if (inESVC_HostIndex != -1)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%02d", inESVC_HostIndex);
		inSetTicket_HostIndex(szTemplate);
	}

	/* TransFunc */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetICASH_Transaction_Function(szTemplate);
	inSetTicket_HostTransFunc(szTemplate);

	/* HostEnable */
	inSetTicket_HostEnable("Y");

	/* LogOn OK */
	inSetTicket_LogOnOK("N");

	/* 將SAM slot更新到TDT */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetICASH_SAM_Slot(szTemplate);

	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	sprintf(szTemplate2, "%02d", atoi(szTemplate));
	inSetTicket_SAM_Slot(szTemplate2);
	
	/* STAN */
	inSetTicket_STAN("000001");
	
	inSaveTDTRec(_TDT_INDEX_02_ICASH_);
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCCTMS_Reboot
Date&Time       :2018/7/4 下午 1:10
Describe        :
*/
int inNCCCTMS_Reboot(TRANSACTION_OBJECT * pobTran)
{
        vdUtility_SYSFIN_LogMessage(AT, "inNCCCTMS_Reboot START!");
        
	if (pobTran->uszTMSDownloadRebootBit == VS_TRUE)
	{
                vdUtility_SYSFIN_LogMessage(AT, "TMS更新完觸發重開機");
		inFunc_Reboot();
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCCTMS_Deal_With_Things_After_APPL_Update
Date&Time       :2018/11/7 下午 1:34
Describe        :處理AP更新之後的事情
*/
int inNCCCTMS_Deal_With_Things_After_APPL_Update(TRANSACTION_OBJECT * pobTran)
{
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCCTMS_Deal_With_Things_After_APPL_Update() START !");
	}
	
	/* 重置TMS排程參數 */
	inResetTMSCPT_Schedule();
	
	/* TMS更新成功 */
	inLoadEDCRec(0);
	inSetTMSOK("Y");
	inSaveEDCRec(0);

	/* 重置紀錄TSAM已註冊的開關*/
	inLoadEDCRec(0);
	inSetTSAMRegisterEnable("N");
	inSaveEDCRec(0);

	/* 確認是否接受CUP、SMARTPAY */
	inFunc_Check_Linkage_Function_Enable(pobTran);
	
	/* TMS連動DCC下載檢查，開機後根據DCCDownloadMode立即DCC下載 */
	if (inNCCC_DCC_TMS_Schedule_Check(pobTran) == VS_SUCCESS)
	{
		inLoadEDCRec(0);
		inSetDCCDownloadMode(_NCCC_DCC_DOWNLOAD_MODE_NOW_);
		inSaveEDCRec(0);
	}

	/* 更新TMS，DCC狀態重置 */
	inLoadEDCRec(0);
	inSetDCCSettleDownload("0");
	inSaveEDCRec(0);
	
	/* 不抓SN，仍要把SN清空，避免造成誤會 */
	inLoadEDCRec(0);
	inSetSAMSlotSN1("                ");
	inSetSAMSlotSN2("                ");
	inSetSAMSlotSN3("                ");
	inSetSAMSlotSN4("                ");
	inSaveEDCRec(0);

	/* 要執行生效回報 */
	if (pobTran->uszFTP_TMS_Download == _TMS_DOWNLOAD_SECURE_MODE_FTPS_	||
	    pobTran->uszFTP_TMS_Download == _TMS_DOWNLOAD_SECURE_MODE_SFTP_)
	{
		inLoadTMSFTPRec(0);
		inSetFTPDownloadResponseCode(_FTP_DOWNLOAD_REPORT_EFFECT_SUCCESS_);
		inSaveTMSFTPRec(0);
	}
	else
	{
		inLoadTMSCPTRec(0);
		inSetTMSEffectiveReportBit("Y");
		inSaveTMSCPTRec(0);
		/* 考慮到有可能外勤裝機有可能ISO轉FTP會沒有回報 */
		inLoadTMSFTPRec(0);
		inSetFTPDownloadResponseCode(_FTP_DOWNLOAD_REPORT_EFFECT_SUCCESS_);
		inSaveTMSFTPRec(0);
	}
	
	/* 回復IP資料 */
	inFunc_UpdateAP_Recover();
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCCTMS_Deal_With_Things_After_APPL_Update() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	return (VS_SUCCESS);
}

/*
Function        :inNCCCTMS_Deal_With_Things_After_Parameter_Update
Date&Time       :2020/3/2 上午 11:03
Describe        :處理參數更新之後的事情，和AP更新做出區別
*/
int inNCCCTMS_Deal_With_Things_After_Parameter_Update(TRANSACTION_OBJECT * pobTran)
{
	char	szTMSDownloadTimes[6 + 1] = {0};
	char	szLastDownloadMode[2 + 1] = {0};
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCCTMS_Deal_With_Things_After_Parameter_Update() START !");
	}
	
	/* 重置TMS排程參數 */
	inResetTMSCPT_Schedule();
	
	/* TMS更新成功 */
	inLoadEDCRec(0);
	inSetTMSOK("Y");
	inSaveEDCRec(0);

	/* 重置紀錄TSAM已註冊的開關*/
	inLoadEDCRec(0);
	inSetTSAMRegisterEnable("N");
	inSaveEDCRec(0);

	/* 確認是否接受CUP、SMARTPAY */
	inFunc_Check_Linkage_Function_Enable(pobTran);

	/* TMS連動DCC下載檢查，開機後根據DCCDownloadMode立即DCC下載 */
	if (inNCCC_DCC_TMS_Schedule_Check(pobTran) == VS_SUCCESS)
	{
		inLoadEDCRec(0);
		inSetDCCDownloadMode(_NCCC_DCC_DOWNLOAD_MODE_NOW_);
		inSaveEDCRec(0);
	}

	/* 更新TMS，DCC狀態重置 */
	inLoadEDCRec(0);
	inSetDCCSettleDownload("0");
	inSaveEDCRec(0);

	/* 要執行生效回報 */
	if (pobTran->uszFTP_TMS_Download == _TMS_DOWNLOAD_SECURE_MODE_FTPS_	||
	    pobTran->uszFTP_TMS_Download == _TMS_DOWNLOAD_SECURE_MODE_SFTP_)
	{
		inLoadTMSFTPRec(0);
		inSetFTPDownloadResponseCode(_FTP_DOWNLOAD_REPORT_EFFECT_SUCCESS_);
		inSaveTMSFTPRec(0);
	}
	else
	{
		inLoadTMSCPTRec(0);
		inSetTMSEffectiveReportBit("Y");
		inSaveTMSCPTRec(0);
                /* 考慮到有可能外勤裝機有可能ISO轉FTP會沒有回報 */
		inLoadTMSFTPRec(0);
		inSetFTPDownloadResponseCode(_FTP_DOWNLOAD_REPORT_EFFECT_SUCCESS_);
		inSaveTMSFTPRec(0);
	}
	
	/* 不是第一次下TMS，下載失敗也不能再擋DCC了 */
	memset(szTMSDownloadTimes, 0x00, sizeof(szTMSDownloadTimes));
	inGetTMSDownloadTimes(szTMSDownloadTimes);
	/* 不是第一次下TMS，DCC Init就設為0 */
	if (atoi(szTMSDownloadTimes) != 1)
	{
		inLoadEDCRec(0);
		inSetDCCInit("0");
		inSaveEDCRec(0);
	}
	
	/* AP自動派板或參數自動派板才要印啟用通知 */
	memset(szLastDownloadMode, 0x00, sizeof(szLastDownloadMode));
	inGetLastDownloadMode(szLastDownloadMode);
	if (atoi(szLastDownloadMode) == _TMS_AUTO_DOWNLOAD_)
	{
		inNCCC_Ticket_PrintBuffer_Inform_Activation_After_TMS_Download();
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCCTMS_Deal_With_Things_After_Parameter_Update() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	return (VS_SUCCESS);
}

/*
Function        :inNCCCTMS_Change_AP_Filesize_To_0
Date&Time       :2019/2/15 下午 4:43
Describe        :因為APPL更新時就會砍檔，所以要將FileSize改為0，啟免檢查下載完成度時判斷錯誤
*/
int inNCCCTMS_Change_AP_Filesize_To_0(void)
{
	int	inRetVal = VS_ERROR;
	int	i = 0, j = 0, inFileNameLen = 0, inSlash = 0;
        char	szTemplate[60 + 1], szFileName[60 + 1];
	
	for (i = 0;; i++)
        {
                /* Load TMS File List 取得檔名 */
                if (inLoadTMSFLTRec(i) < 0)
                        break;

                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetTMSFilePathName(szTemplate);

                /* 取得檔案路徑後 抓取檔名儲存到FileName */
                for (j = 0; j < 60 ; j++)
                {
                        if (szTemplate[j] == '/')
                            inSlash = j + 1;

                        if (szTemplate[j] == 0x00)
                            break;
                }

		/* 斜線後的檔名長度*/
                inFileNameLen = j - inSlash;
                memset(szFileName, 0x00, sizeof(szFileName));
                memcpy(&szFileName[0], &szTemplate[inSlash], inFileNameLen);
		
		if (memcmp(szFileName, _APPL_NAME_, strlen(_APPL_NAME_)) != 0)
		{
			continue;
		}
		else
		{
			inSetTMSFileSize("0");
			inSaveTMSFLTRec(i);
			inRetVal = VS_SUCCESS;
			break;
		}
		
        }
	
	return (inRetVal);
}

/*
Function        :inNCCCTMS_FTPS_Update_TermParaTime
Date&Time       :2019/5/2 上午 9:59
Describe        :更新完才能更新版本參數時間
*/
int inNCCCTMS_FTPS_Update_TermParaTime()
{
	int		inLen = 0;
        int		i = 0, inOffset = 0, inDataSize = 0, inCnt = 0;
        long		lnFTPFLTLength = 0;
        long		lnReadLength = 0;
        char		szFileName[26 + 1], szDispMsg[100 + 1];
	char		szTemplate[50 + 1] = {0};
        unsigned long   ulFile_Handle;
        unsigned char   *uszReadData;
        unsigned char   *uszTemp;
        VS_BOOL		fDataErr = VS_FALSE;

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCCTMS_FTPS_Update_TermParaTime() START!");
                      
        /* 讀TermInfo2檔案 START */
        memset(szFileName, 0x00, sizeof(szFileName));
        strcpy(szFileName, _FTP_INFO_2_);
        
        if (inFILE_Open(&ulFile_Handle, (unsigned char *)szFileName) == VS_ERROR)
        {
                /* 開檔失敗 ，不用關檔 */
                /* 開檔失敗，所以回傳error */
                return (VS_ERROR);
        }

        /*
         * get data size 為了接下來動態宣告記憶體的大小需要知道多少 需先取得檔案Size
         * get size 失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        lnFTPFLTLength = lnFILE_GetSize(&ulFile_Handle, (unsigned char *)szFileName);
        
        if (lnFTPFLTLength == VS_ERROR)
        {
                /* GetSize失敗 ，關檔 */
                inFILE_Close(&ulFile_Handle);
                /* 刪除TermInfo2.txt */
                inFILE_Delete((unsigned char *)szFileName);

                return (VS_ERROR);
        }
        
        /*
         * allocate 記憶體
         * allocate時多分配一個byte以防萬一（ex:換行符號）
         */
        uszReadData = malloc(lnFTPFLTLength + 1);
        uszTemp = malloc(lnFTPFLTLength + 1);
        /* 初始化 uszTemp uszReadData */
        memset(uszReadData, 0x00, lnFTPFLTLength + 1);
        memset(uszTemp, 0x00, lnFTPFLTLength + 1);
        
        /* seek 到檔案開頭 & 從檔案開頭開始read */
        if (inFILE_Seek(ulFile_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnFTPFLTLength;

                for (i = 0;; ++i)
                {
                        /* 剩餘長度大於或等於1024 */
                        if (lnReadLength >= 1024)
                        {
                                if (inFILE_Read(&ulFile_Handle, &uszReadData[1024 * i], 1024) == VS_SUCCESS)
                                {
                                        /* 一次讀1024 */
                                        lnReadLength -= 1024;

                                        /* 當剩餘長度剛好為1024，會剛好讀完 */
                                        if (lnReadLength == 0)
                                                break;
                                }
                                /* 讀失敗時 */
                                else
                                {
                                        /* Close檔案 */
                                        inFILE_Close(&ulFile_Handle);
                                        /* 刪除TermInfo2.txt */
                                        inFILE_Delete((unsigned char *)szFileName);
                                        /* Free pointer */
                                        free(uszReadData);
                                        free(uszTemp);

                                        return (VS_ERROR);
                                }
                        }
                        /* 剩餘長度小於1024 */
                        else if (lnReadLength < 1024)
                        {
                                /* 就只讀剩餘長度 */
                                if (inFILE_Read(&ulFile_Handle, &uszReadData[1024 * i], lnReadLength) == VS_SUCCESS)
                                {
                                        break;
                                }
                                /* 讀失敗時 */
                                else
                                {
                                        /* Close檔案 */
                                        inFILE_Close(&ulFile_Handle);
                                        /* 刪除TermInfo2.txt */
                                        inFILE_Delete((unsigned char *)szFileName);
                                        /* Free pointer */
                                        free(uszReadData);
                                        free(uszTemp);

                                        return (VS_ERROR);
                                }
                        }
                }/* end for loop */
        }
        /* seek不成功時 */
        else
        {
                /* 關檔並回傳 */
                inFILE_Close(&ulFile_Handle);
                /* Free pointer */
                free(uszReadData);
                free(uszTemp);

                /* Seek失敗，所以回傳Error */
                return (VS_ERROR);
        }
        /* 讀TermInfo2檔案 END */
        
        /* 分析ReadData資料，第一行是Header */
        for (i = 0; i < lnFTPFLTLength; i ++)
        {
                if ((uszReadData[i] == 0x0D) && (uszReadData[i + 1] == 0x0A))
                        break;
                
                memcpy(&uszTemp[i], &uszReadData[i], 1);
        }

        /* 防呆 */
        inLen = strlen((char*)uszTemp);

        /* 目前Header那行最長94不含換行符號 */
        if (inLen > 100)
        {
                free(uszReadData);
                free(uszTemp);
                inFILE_Close(&ulFile_Handle);
                /* 刪除TermInfo2.txt */
                inFILE_Delete((unsigned char *)szFileName);
                /* 錯誤提示訊息 */
                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                inDISP_ChineseFont("檔案格式有誤", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);
                inDISP_BEEP(3, 500);
                return (VS_ERROR);
        }
        
        inOffset = 0;   /* 起點 */
        inCnt = 0;      /* 終點 */
        
	inLoadTMSFTPRec(0);
	
	/* 分析Header */
        for (i = 1; i < 13; i ++)
        {
                inDataSize = 0;  /* 長度 */

                while (1)
                {
                        inCnt ++;
                        inDataSize ++;

                        /* 逗號 */
                        if ((uszTemp[inCnt] == 0x2C) || (uszTemp[inCnt] == 0x00))
                        {
                                inCnt ++;
                                break;
                        }

                        /* 防呆 */
                        if (inCnt > inLen)
                        {
                                free(uszReadData);
                                free(uszTemp);
                                inFILE_Close(&ulFile_Handle);
                                /* 刪除TermInfo2.txt */
                                inFILE_Delete((unsigned char *)szFileName);
                                /* 錯誤提示訊息 */
                                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                                inDISP_ChineseFont("檔案格式有誤", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);
                                inDISP_BEEP(3, 500);

                                return (VS_ERROR);
                        }
                }

                switch (i)
                {
                        case 1:
                                break;
                        case 2:
                                break;
                        case 3:
                                break;
                        case 4:
                                break;
                        case 5:
                                break;
                        case 6:
                                break;
                        case 7:
                                break;
                        case 8:
                                break;
                        case 9:
                                break;
                        case 10:
				memset(szTemplate, 0x00, sizeof(szTemplate));
                                memcpy(szTemplate, &uszTemp[inOffset], inDataSize);

                                if (ginDebug == VS_TRUE)
                                {
                                        memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                        sprintf(szDispMsg, "TermParmDT [%02d][%s]", strlen(szTemplate), szTemplate);
                                        inLogPrintf(AT, szDispMsg);
                                }
                                if (ginISODebug == VS_TRUE)
				{
					memset(szDispMsg, 0x00, sizeof(szDispMsg));
                                        sprintf(szDispMsg, "TermParmDT [%02d][%s]", strlen(szTemplate), szTemplate);
					inPRINT_ChineseFont(szDispMsg, _PRT_ISO_);
				}
				
                                if (inDataSize != 12)
				{
                                        fDataErr = VS_TRUE;
				}
				else
				{
					inSetFTPTermParemeterDateTime(szTemplate);
				}
                                
                                break;
                        case 11:
                                break;
                        case 12:
                                break;
                        default:
                                break;

                }
                inOffset = inCnt;
        }
	
	/* 一次設定完再存 */
	inSaveTMSFTPRec(0);
        
	free(uszReadData);
	free(uszTemp);
        inFILE_Close(&ulFile_Handle);
	
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCCTMS_FTPS_TermInfo_Analyze() END!");
        
        return (VS_SUCCESS);
}

/*
Function        :inNCCCTMS_Get_Filelist_Name
Date&Time       :2019/5/23 下午 2:22
Describe        :為了因應各種Download Module，因此不將Filelist Hardcode在程式中，改由讀取
*/
int inNCCCTMS_Get_Filelist_Name(char* szFileName)
{
	int	inFd = 0;
	int	inReadDataLen = 0;
	int	inRetVal = VS_SUCCESS;
	char	szPath[100 + 1] = {0};
	char	szReadData[100 + 1] = {0};
	
	memset(szReadData, 0x00, sizeof(szReadData));
	inReadDataLen = sizeof(szReadData);
	
	memset(szPath, 0x00, sizeof(szPath));
	sprintf(szPath, "%s%s", _APP_UPDATE_PATH_, _MMCI_FILE_NAME_);
	inFile_Linux_Open(&inFd, szPath);
	inFile_Linux_Seek(inFd, 0, _SEEK_BEGIN_);
	inFile_Linux_Read(inFd, szReadData, &inReadDataLen);
	inFile_Linux_Close(inFd);
	
	strcpy(szFileName, szReadData);
	
	return (inRetVal);
}


/*
Function        :inNCCCTMS_CDT_Customize
Date&Time       :2020/2/4 上午 10:31
Describe        :用Hardcode CDT 實際卡別由ATS主機定義
*/
/* 【需求單 - 108097】Card BIN由ATS查核 add by LingHsiung 2020/2/3 下午 5:19 */
int inNCCCTMS_CDT_Customize(void)
{
	int		inRetVal;
	int	        inPackCount = 0;
	long            lnCDTLength = 0;
	unsigned long   uldat_Handle;
	unsigned char	*uszRead_Total_Buff;
	unsigned char   *uszWriteBuff_Record;
        char	szCardIndex[2 + 1] = {0};
        char	szLowBinRange[11 + 1] = {0};
        char	szHighBinRange[11 + 1] = {0};
        char	szHostCDTIndex[2 + 1] = {0};
        char	szMinPANLength[2 + 1] = {0};
        char	szMaxPANLength[2 + 1] = {0};
        char	szModule10Check[1 + 1] = {0};
        char	szExpiredDateCheck[1 + 1] = {0};
        char	sz4DBCEnable[1 + 1] = {0};
        char	szCardLabel[20 + 1] = {0};
	char	szPrint_Tx_No_Check_No[1 + 1] = {0};
	
        /* 開啟CDT.dat */
        inRetVal = inFILE_Open(&uldat_Handle, (unsigned char *)_CDT_FILE_NAME_);
	
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
        /* 取得CDT.dat檔案大小 */
        lnCDTLength = lnFILE_GetSize(&uldat_Handle, (unsigned char *)_CDT_FILE_NAME_);
	
        /* 取得檔案大小失敗 */
        if (lnCDTLength == VS_ERROR)
        {
                inFILE_Close(&uldat_Handle);

                return (VS_ERROR);
        }
	
        /* uszRead_Total_Buff儲存CDT.dat全部資料 */
        uszRead_Total_Buff = malloc(lnCDTLength + 1);
        memset(uszRead_Total_Buff, 0x00, lnCDTLength + 1);
	
        /* 讀CDT.dat檔案存到uszRead_Total_Buff */
        if (inFILE_Seek(uldat_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
	{
		if (inFILE_Read(&uldat_Handle, &uszRead_Total_Buff[0], lnCDTLength) == VS_SUCCESS)
		{
			
		}
		else
		{
			/* Close檔案 */
			inFILE_Close(&uldat_Handle);
			/* Free pointer */
			free(uszRead_Total_Buff);

			return (VS_ERROR);			
		}
	}
	else
	{
                /* Close檔案 */
                inFILE_Close(&uldat_Handle);
		/* Free pointer */
		free(uszRead_Total_Buff);
		
		return (VS_ERROR);
	}
	
        /* 存原CDT.dat的每筆Record 初始化 */
        uszWriteBuff_Record = malloc(_SIZE_CDT_REC_ + _SIZE_CDT_COMMA_0D0A_);
        memset(uszWriteBuff_Record, 0x00, _SIZE_CDT_REC_ + _SIZE_CDT_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */
	
        inPackCount = 0; /* uszWriteBuff_Record的index位置 */

	/* 卡別索引 */
	strcpy(szCardIndex, "00");
        memcpy(&uszWriteBuff_Record[0], &szCardIndex[0], strlen(szCardIndex));
        inPackCount += strlen(szCardIndex);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* 低卡號範圍 */
	strcpy(szLowBinRange, "00000000000");
        memcpy(&uszWriteBuff_Record[inPackCount], &szLowBinRange[0], strlen(szLowBinRange));
        inPackCount += strlen(szLowBinRange);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* 高卡號範圍 */
	strcpy(szHighBinRange, "99999999999");
        memcpy(&uszWriteBuff_Record[inPackCount], &szHighBinRange[0], strlen(szHighBinRange));
        inPackCount += strlen(szHighBinRange);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* 對應交易主機索引 */
	strcpy(szHostCDTIndex, "01");
        memcpy(&uszWriteBuff_Record[inPackCount], &szHostCDTIndex[0], strlen(szHostCDTIndex));
        inPackCount += strlen(szHostCDTIndex);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* 檢查最短卡號長度 */
	strcpy(szMinPANLength, "13");
        memcpy(&uszWriteBuff_Record[inPackCount], &szMinPANLength[0], strlen(szMinPANLength));
        inPackCount += strlen(szMinPANLength);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* 檢查最長卡號長度 */
	strcpy(szMaxPANLength, "19");
        memcpy(&uszWriteBuff_Record[inPackCount], &szMaxPANLength[0], strlen(szMaxPANLength));
        inPackCount += strlen(szMaxPANLength);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* 檢查碼查核 */
	strcpy(szModule10Check, "N");
        memcpy(&uszWriteBuff_Record[inPackCount], &szModule10Check[0], strlen(szModule10Check));
        inPackCount += strlen(szModule10Check);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* 有效期查核 */
	strcpy(szExpiredDateCheck, "N");
        memcpy(&uszWriteBuff_Record[inPackCount], &szExpiredDateCheck[0], strlen(szExpiredDateCheck));
        inPackCount += strlen(szExpiredDateCheck);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* 輸入AMEX 4DBC或MASTER/VISA CVV2。 */
	strcpy(sz4DBCEnable, "N");
        memcpy(&uszWriteBuff_Record[inPackCount], &sz4DBCEnable[0], strlen(sz4DBCEnable));
        inPackCount += strlen(sz4DBCEnable);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;

	/* 卡別名稱(VISA,MASTERCARD,JCB,U CARD,AMEX,DISCOVER,SMARTPAY) */
	strcpy(szCardLabel, "ATS CHECK");
        memcpy(&uszWriteBuff_Record[inPackCount], &szCardLabel[0], strlen(szCardLabel));
        inPackCount += strlen(szCardLabel);
        uszWriteBuff_Record[inPackCount] = 0x2C;
        inPackCount++;
	
	/* 商店自存聯卡號遮掩 卡別判斷 */
	strcpy(szPrint_Tx_No_Check_No, "N");
        memcpy(&uszWriteBuff_Record[inPackCount], &szPrint_Tx_No_Check_No[0], strlen(szPrint_Tx_No_Check_No));
        inPackCount += strlen(szPrint_Tx_No_Check_No);

        /* 最後的data不用逗號 */
        /* 補上換行符號 */

        /* 0D是移至行首 */
        uszWriteBuff_Record[inPackCount] = 0x0D;
        inPackCount++;
        /* 0A是移至下一行 */
        uszWriteBuff_Record[inPackCount] = 0x0A;
        inPackCount++;
	
	
        /* 將組好的record寫入檔案 */
	if (inFILE_Seek(uldat_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
	{
		/* 寫入hard code的第一組CARD BIN */
		inRetVal = inFILE_Write(&uldat_Handle, uszWriteBuff_Record, inPackCount);
		 
		if (inRetVal != VS_SUCCESS)
		{
			/* 關檔 */
			inFILE_Close(&uldat_Handle);

			/* Free pointer */
			free(uszWriteBuff_Record);
			free(uszRead_Total_Buff);

			return (VS_ERROR);
		}
		
		/* 寫入原本的CDT */
		inRetVal = inFILE_Write(&uldat_Handle, uszRead_Total_Buff, lnCDTLength);
		
		if (inRetVal != VS_SUCCESS)
		{
			/* 關檔 */
			inFILE_Close(&uldat_Handle);

			/* Free pointer */
			free(uszWriteBuff_Record);
			free(uszRead_Total_Buff);

			return (VS_ERROR);
		}
	}
	else
	{
                /* 檔案 */
                inFILE_Close(&uldat_Handle);
		/* Free pointer */
		free(uszRead_Total_Buff);
		free(uszWriteBuff_Record);
		
		return (VS_ERROR);
	}
	
	/* 關檔 */
	inFILE_Close(&uldat_Handle);

	/* Free pointer */
	free(uszWriteBuff_Record);
	free(uszRead_Total_Buff);
	
	return (VS_SUCCESS);
}

int inNCCCTMS_CDTX_initial(void)
{
	int		inRetVal;
	int	        inPackCount = 0;
	int		inCDTXIndex = 0;
	unsigned long   uldat_Handle;
	unsigned char   *uszWriteBuff_Record;
        char	szCardIndex[2 + 1] = {0};
        char	szCardLabel[20 + 1] = {0};
	
	/* 刪除CDTX.dat */
	inFILE_Delete((unsigned char *)_CDTX_FILE_NAME_);
	
        /* 新建CDTX.dat檔案 */
        inRetVal = inFILE_Create(&uldat_Handle, (unsigned char *)_CDTX_FILE_NAME_);
	
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
 	
        if (inFILE_Seek(uldat_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
	{

	}
	else
	{
                /* Close檔案 */
                inFILE_Close(&uldat_Handle);
		
		return (VS_ERROR);
	}
	
        /* 存CDTX.dat的每筆Record 初始化 */
        uszWriteBuff_Record = malloc(_SIZE_CDTX_REC_ + _SIZE_CDTX_COMMA_0D0A_);
	
	for (inCDTXIndex = 0 ; inCDTXIndex < 10; inCDTXIndex ++)
	{
		memset(uszWriteBuff_Record, 0x00, _SIZE_CDTX_REC_ + _SIZE_CDTX_COMMA_0D0A_); /* 如果用sizeof指標，會只set 4 byte */
		inPackCount = 0; /* uszWriteBuff_Record的index位置 */

		/* 卡別索引 */
		sprintf(szCardIndex, "%02d", inCDTXIndex);
		memcpy(&uszWriteBuff_Record[0], &szCardIndex[0], strlen(szCardIndex));
		inPackCount += strlen(szCardIndex);
		uszWriteBuff_Record[inPackCount] = 0x2C;
		inPackCount++;

		/* 卡別名稱(預設值20個空白) */
		strcpy(szCardLabel, "                    ");
		memcpy(&uszWriteBuff_Record[inPackCount], &szCardLabel[0], strlen(szCardLabel));
		inPackCount += strlen(szCardLabel);

		/* 最後的data不用逗號 */
		/* 補上換行符號 */
		/* 0D是移至行首 */
		uszWriteBuff_Record[inPackCount] = 0x0D;
		inPackCount++;
		/* 0A是移至下一行 */
		uszWriteBuff_Record[inPackCount] = 0x0A;
		inPackCount++;

		/* 將組好的record寫入檔案 */
		inRetVal = inFILE_Write(&uldat_Handle, uszWriteBuff_Record, inPackCount);

		if (inRetVal != VS_SUCCESS)
		{
			/* 關檔 */
			inFILE_Close(&uldat_Handle);

			/* Free pointer */
			free(uszWriteBuff_Record);

			return (VS_ERROR);
		}
	}
	
	/* 關檔 */
	inFILE_Close(&uldat_Handle);

	/* Free pointer */
	free(uszWriteBuff_Record);
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCCTMS_DUTY_FREE_ReturnTxnReceipt
Date&Time       :2022/5/18 下午 3:06
Describe        :上傳簽單紀錄至TMS
*/
int inNCCCTMS_DUTY_FREE_ReturnTxnReceipt(TRANSACTION_OBJECT* pobTran)
{	
	int	inRetVal = VS_ERROR;
	int	inUploadTimes = 0;
	char	szDEMOMode[2 + 1] = {0};
	TRANSACTION_OBJECT	pobTempTran;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCCTMS_DUTY_FREE_ReturnTxnReceipt() START !");
	}

	memset(szDEMOMode, 0x00, sizeof(szDEMOMode));
	inGetDemoMode(szDEMOMode);
	if (szDEMOMode[0] == 'Y')
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont("重印紀錄上傳中", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
		inDISP_Wait(500);
			
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont("傳送中", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
		inDISP_Wait(200);
		
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont("接收中", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
		inDISP_Wait(500);
		inRetVal = VS_SUCCESS;
	}
	else
	{
		pobTempTran.inTMSDwdMode = _TMS_AUTO_DOWNLOAD_;					/* 自動下載 */
		pobTempTran.uszFTP_TMS_Download = _TMS_DOWNLOAD_SECURE_MODE_NONE_;			/* 參數回報一律走ISO8583 */
		/* 先initial Field 58 */
		inNCCCTMS_Field58_Initial(&pobTempTran);
		/* 昇恆昌重印帳單回報 */
		strcpy(gsrTMS_Field58.szDownloadScope ,_TMS_DOWNLOAD_SCOPE_046_REPRINT_REPORT_);

		do
		{
			/* TMS的CPT參數 */
			if (inLoadTMSCPTRec(0) != VS_SUCCESS)
				break;

			/* 連線到TMS主機 */
			if (inFLOW_RunFunction(&pobTempTran, _FUNCTION_TMS_CONNECT_) != VS_SUCCESS)
			{
				if (ginDebug == VS_TRUE)
					inLogPrintf(AT, "inNCCCTMS_ConnectToServer()Error");
				break;
			}

			inUploadTimes = 0;
			do
			{
				/* TMS收送電文 */
				pobTempTran.inTransactionCode = _NCCCTMS_DUTYFREE_;
				inFLOW_RunFunction(&pobTempTran, _FUNCTION_TMS_SEND_RECEIVE_);
				inUploadTimes ++;
				if (pobTempTran.inTransactionResult != _NCCCTMS_AUTHORIZED_)
				{
					/* 若上傳失敗，要多重試一次 */
					if (inUploadTimes < 2)
					{
						continue;
					}
				}
				break;
			}while(1);
			break;
		}while(1);

		/* 斷線 */
		inFLOW_RunFunction(&pobTempTran, _FUNCTION_TMS_DISCONNECT_);
		
		if (pobTempTran.inTransactionResult != _NCCCTMS_AUTHORIZED_)
		{
			inRetVal = VS_ERROR;
		}
		else
		{
			inRetVal = VS_SUCCESS;
		}

		/* 當筆上傳完成要回覆原來主機的狀態 */
		inLoadHDTRec(pobTran->srBRec.inHDTIndex);
		inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
	} /* DEMO_EDC 教育訓練版 */
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCCTMS_DUTY_FREE_ReturnTxnReceipt() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :inNCCCTMS_PRINT_ScheduleMessage_Flow
Date&Time       :2022/7/5 下午 3:57
Describe        :印小白單
*/
int inNCCCTMS_PRINT_ScheduleMessage_Flow(TRANSACTION_OBJECT *pobTran)
{
	char	szTemplate[1 + 1] = {0};
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCCTMS_PRINT_ScheduleMessage_Flow() START !");
	}
	
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTMSDownloadMode(szTemplate);
	
	/* ISO8583*/
	do
	{
		if (memcmp(szTemplate, _TMS_DLMODE_ISO8583_, strlen(_TMS_DLMODE_ISO8583_)) == 0)
		{
			if (inLoadTMSCPTRec(0) != VS_SUCCESS)
				break;
		}
		
		memset(szTemplate, 0x00, sizeof(szTemplate));
		/* 確認是否立即下載 */
		if (inGetTMSDownloadFlag(szTemplate) != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
				inLogPrintf(AT, "inGetTMSDownloadFlag() Error!");
			break;
		}
		
		/* TMS_Download Flag = 2 排程下載 */
		if (!memcmp(&szTemplate[0], _TMS_DOWNLOAD_FLAG_SCHEDULE_, 1))
		{
			inNCCCTMS_PRINT_ScheduleMessage(pobTran, _TMS_PRT_SCHEDULE_NOFTFY_);
		}
		
		break;
	}while(1);
	
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCCTMS_PRINT_ScheduleMessage_Flow() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCCTMS_FTP_TermInfo_Disp_Flow
Date&Time       :2024/5/23 下午 3:07
Describe        :
*/
int inNCCCTMS_FTP_TermInfo_Disp_Flow(TRANSACTION_OBJECT * pobTran, FTPS_REC* srFtpsObj)
{
	int	i = 0;
	int	inRetVal = VS_SUCCESS;
	char	szCFESMode[2 + 1] = {0};
	
	memset(szCFESMode, 0x00, sizeof(szCFESMode));
	inGetCloud_MFES(szCFESMode);
	
        /* 重試三次，所以要連線4次 */
	for (i = 0 ; i < 4 ; i ++)
	{
		srFtpsObj->inFtpRetryTimes = i;

		if (memcmp(szCFESMode, "Y", strlen("Y")) == 0	||
		    pobTran->uszFTP_TMS_Download == _TMS_DOWNLOAD_SECURE_MODE_SFTP_)
		{
			inRetVal = inFTPS_Download_SFTP(srFtpsObj);
		}
		else
		{
			inRetVal = inFTPS_Download(srFtpsObj);
		}

		if (inRetVal != VS_SUCCESS)
		{
			/* 提示下載失敗錯誤訊息 */
			if (inRetVal == CURLE_REMOTE_ACCESS_DENIED)
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("檔案路徑設定有誤", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_CENTER_);
				inDISP_ChineseFont("下載檔案失敗", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
				inDISP_ChineseFont(srFtpsObj->szFtpsFileName, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
				inDISP_BEEP(3, 500);
			}
			else if (inRetVal == CURLE_REMOTE_FILE_NOT_FOUND)
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("檔案不存在主機上", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_CENTER_);
				inDISP_ChineseFont("下載檔案失敗", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
				inDISP_ChineseFont(srFtpsObj->szFtpsFileName, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
				inDISP_BEEP(3, 500);
			}
			else
			{
				DISPLAY_OBJECT	srDispMsgObj;
				memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
				strcpy(srDispMsgObj.szDispPic1Name, _ERR_CONNECT_);
				srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
				srDispMsgObj.inMsgType = _NO_KEY_MSG_;
				srDispMsgObj.inTimeout = 0;
				if (ginDisplayDebug == VS_TRUE)
				{
					if (inRetVal == CURLE_SSL_CONNECT_ERROR)
					{
						strcpy(srDispMsgObj.szErrMsg1, "SSL 連線失敗");
						srDispMsgObj.inErrMsg1Line = _LINE_8_4_;
					}
					else
					{
						sprintf(srDispMsgObj.szErrMsg1, "errno:%d", inRetVal);
						srDispMsgObj.inErrMsg1Line = _LINE_8_4_;
					}

					sprintf(srDispMsgObj.szErrMsg2, "%s", srFtpsObj->szFtpsURL);
					srDispMsgObj.inErrMsg2Line = _LINE_8_5_;
					srDispMsgObj.inErrMsg2FontSize = _FONTSIZE_8X44_;

					sprintf(srDispMsgObj.szErrMsg3, "%s", srFtpsObj->szCACertFileName);
					srDispMsgObj.inErrMsg3Line = _LINE_8_7_;
					srDispMsgObj.inErrMsg3FontSize = _FONTSIZE_8X44_;
				}
				else
				{
					strcpy(srDispMsgObj.szErrMsg1, "");
					srDispMsgObj.inErrMsg1Line = 0;
				}
				srDispMsgObj.inBeepTimes = 3;
				srDispMsgObj.inBeepInterval = 1000;

				inDISP_Msg_BMP(&srDispMsgObj);
			}

			continue;
		}
		else
		{
			if (inFILE_Check_Exist((unsigned char*)srFtpsObj->szFtpsFileName) != VS_SUCCESS)
			{
				continue;
			}
			else
			{
				break;
			}
		}
	}
	
	return (inRetVal);
}

/*
Function        :inNCCCTMS_FTP_APPARM_Disp_Flow
Date&Time       :2024/5/23 下午 3:07
Describe        :
*/
int inNCCCTMS_FTP_APPARM_Disp_Flow(TRANSACTION_OBJECT * pobTran, FTPS_REC* srFtpsObj)
{
	int		j = 0;
	int		inRetVal = VS_SUCCESS;
	char		szDebugMsg[100 + 1] = {0};
	char		szCFESMode[2 + 1] = {0};
	char		szTemplate[128 + 1] = {0};
	long		lnFileSize = 0;				/* 下載的檔案 */
	long		lnTMSFLTSize = 0;			/* 表上的大小 */
	unsigned long	ulFile_Handle = 0;
	
	memset(szCFESMode, 0x00, sizeof(szCFESMode));
	inGetCloud_MFES(szCFESMode);
	
	/* 重試三次，所以要連線4次 */
	for (j = 0 ; j < 4 ; j ++)
	{
		srFtpsObj->inFtpRetryTimes = j;

		if (memcmp(szCFESMode, "Y", strlen("Y")) == 0	||
		    pobTran->uszFTP_TMS_Download == _TMS_DOWNLOAD_SECURE_MODE_SFTP_)
		{
			inRetVal = inFTPS_Download_SFTP(srFtpsObj);
		}
		else
		{
			inRetVal = inFTPS_Download(srFtpsObj);
		}

		if (inRetVal != VS_SUCCESS)
		{
			/* 提示下載失敗錯誤訊息 */
			if (inRetVal == CURLE_REMOTE_ACCESS_DENIED)
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("檔案路徑設定有誤", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_CENTER_);
				inDISP_ChineseFont("下載檔案失敗", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
				inDISP_ChineseFont(srFtpsObj->szFtpsFileName, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
				inDISP_BEEP(3, 500);
				break;
			}
			else if (inRetVal == CURLE_REMOTE_FILE_NOT_FOUND)
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("檔案不存在主機上", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_CENTER_);
				inDISP_ChineseFont("下載檔案失敗", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
				inDISP_ChineseFont(srFtpsObj->szFtpsFileName, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
				inDISP_BEEP(3, 500);

				break;
			}
			else
			{
				DISPLAY_OBJECT	srDispMsgObj;
				memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
				strcpy(srDispMsgObj.szDispPic1Name, _ERR_CONNECT_);
				srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
				srDispMsgObj.inMsgType = _NO_KEY_MSG_;
				srDispMsgObj.inTimeout = 0;
				strcpy(srDispMsgObj.szErrMsg1, "");
				srDispMsgObj.inErrMsg1Line = 0;
				srDispMsgObj.inBeepTimes = 3;
				srDispMsgObj.inBeepInterval = 1000;

				inDISP_Msg_BMP(&srDispMsgObj);
				continue;
			}
		}
		else
		{
			/* 檢核大小 */
			/* 比對下載的檔案與File List提供的檔案大小是否一致 */
			lnFileSize = lnFILE_GetSize(&ulFile_Handle, (unsigned char *)srFtpsObj->szFtpsFileName);
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetFTPFileSize(szTemplate);
			lnTMSFLTSize = atol(szTemplate);

			/* 檢核成功 */
			if (lnFileSize == lnTMSFLTSize)
			{
				/* 有下AP，且AP下載成功 */
				if (memcmp(srFtpsObj->szFtpsFileName, _APPL_NAME_, strlen(_APPL_NAME_)) == 0)
				{
					/* 要更新APP */
					inSetTMSAPPUpdateStatus("Y");
					inSaveTMSCPTRec(0);
				}

				break;
			}
			else
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "FileList: %ld FileSize: %ld", lnTMSFLTSize, lnFileSize);
					inLogPrintf(AT, szDebugMsg);
				}

				if (srFtpsObj->inFtpRetryTimes == 2)
				{
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_ChineseFont(srFtpsObj->szFtpsFileName, _FONTSIZE_8X16_, _LINE_8_4_, _DISP_CENTER_);
					inDISP_ChineseFont("檔案大小有誤", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
					inDISP_BEEP(3, 500);
				}
				else
				{
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_ChineseFont("檔案大小核對有誤", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_CENTER_);
					inDISP_ChineseFont("重新下載核對檔案", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
					inDISP_ChineseFont(srFtpsObj->szFtpsFileName, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
					inDISP_BEEP(3, 500);
				}
				continue;
			}
		}
	}
		
	return (inRetVal);
}