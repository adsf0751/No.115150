#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
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
#include "../SOURCE/FUNCTION/Sqlite.h"
#include "../SOURCE/FUNCTION/Function.h"
#include "../SOURCE/FUNCTION/FuncTable.h"
#include "../SOURCE/FUNCTION/File.h"
#include "../SOURCE/FUNCTION/Batch.h"
#include "../SOURCE/EVENT/MenuMsg.h"
#include "../SOURCE/EVENT/Flow.h"
#include "../SOURCE/FUNCTION/Accum.h"
#include "../SOURCE/FUNCTION/HDT.h"
#include "../SOURCE/FUNCTION/HDPT.h"
#include "../SOURCE/FUNCTION/CDT.h"
#include "../SOURCE/FUNCTION/CPT.h"
#include "../SOURCE/FUNCTION/CFGT.h"
#include "../SOURCE/FUNCTION/EDC.h"
#include "../SOURCE/FUNCTION/Utility.h"
#include "../SOURCE/COMM/Comm.h"
#include "../SOURCE/COMM/Ethernet.h"
#include "../SOURCE/COMM/Modem.h"
#include "../CREDIT/Creditfunc.h"
#include "../SOURCE/COMM/Ftps.h"
#include "NCCCtms.h"
#include "NCCCtmsCPT.h"
#include "NCCCtmsSCT.h"
#include "NCCCtmsFLT.h"
#include "NCCCtmsFTP.h"
#include "NCCCtmsFTPFLT.h"
#include "NCCCtmsiso.h"

#define max(x, y) (((x) > (y)) ? (x) : (y))

int			giNowDispPercent = 0;
unsigned char		guszNCCCTMS_ISO_Field03[_NCCCTMS_PCODE_SIZE_ + 1];
extern  int		ginDebug;  /* Debug使用 extern */
extern  int		ginISODebug;
extern	int		ginMachineType;
extern	char		gszTermVersionID[16 + 1];
extern	char		gszTermVersionDate[16 + 1];
extern  TMS_Field58_REC gsrTMS_Field58;
extern	char		gszReprintDBPath[100 + 1];

ISO_FIELD_NCCCTMS_TABLE srNCCCTMS_ISOFieldPack[] =
{
        {3,        inNCCCTMS_Pack03},
        {24,       inNCCCTMS_Pack24},
        {39,       inNCCCTMS_Pack39},
        {41,       inNCCCTMS_Pack41},
        {42,       inNCCCTMS_Pack42},
	{58,       inNCCCTMS_Pack58},
        {61,       inNCCCTMS_Pack61},
        {0,        NULL},  /* 最後一組一定要放 0 */
};

ISO_FIELD_NCCCTMS_TABLE srNCCCTMS_ISOFieldUnPack[] =
{
        {39,       inNCCCTMS_UnPack39},
        {58,       inNCCCTMS_UnPack58},
        {61,       inNCCCTMS_UnPack61},
        {0,        NULL},  /* 最後一組一定要放 0 */
};

ISO_CHECK_NCCCTMS_TABLE srNCCCTMS_ISOFieldCheck[] =
{
        {3,        inNCCCTMS_Check03},
        {0,        NULL},  /* 最後一組一定要放 0 */
};

ISO_FIELD_TYPE_NCCCTMS_TABLE srNCCCTMS_ISOFieldType[] =
{
        {2,     _NCCCTMS_ISO_NIBBLE_2_,    VS_FALSE,       0},
        {3,     _NCCCTMS_ISO_BCD_,         VS_FALSE,       6},
        {4,     _NCCCTMS_ISO_BCD_,         VS_FALSE,       12},
        {11,    _NCCCTMS_ISO_BCD_,         VS_FALSE,       6},
        {12,    _NCCCTMS_ISO_BCD_,         VS_FALSE,       6},
        {13,    _NCCCTMS_ISO_BCD_,         VS_FALSE,       4},
        {14,    _NCCCTMS_ISO_BCD_,         VS_FALSE,       4},
        {22,    _NCCCTMS_ISO_BCD_,         VS_FALSE,       4},
        {24,    _NCCCTMS_ISO_BCD_,         VS_FALSE,       4},
        {25,    _NCCCTMS_ISO_BCD_,         VS_FALSE,       2},
        {27,    _NCCCTMS_ISO_BCD_,         VS_FALSE,       2},
        {35,    _NCCCTMS_ISO_NIBBLE_2_,    VS_FALSE,       0},
        {37,    _NCCCTMS_ISO_ASC_,         VS_FALSE,       12},
        {38,    _NCCCTMS_ISO_ASC_,         VS_FALSE,       6},
        {39,    _NCCCTMS_ISO_ASC_,         VS_FALSE,       2},
        {41,    _NCCCTMS_ISO_ASC_,         VS_FALSE,       8},
        {42,    _NCCCTMS_ISO_ASC_,         VS_FALSE,       15},
        {48,    _NCCCTMS_ISO_BYTE_3_,      VS_TRUE,        0},
        {54,    _NCCCTMS_ISO_BYTE_3_,      VS_TRUE,        0},
        {55,    _NCCCTMS_ISO_BYTE_3_,      VS_FALSE,       0},
        {57,    _NCCCTMS_ISO_BYTE_3_,      VS_FALSE,       0},
        {58,    _NCCCTMS_ISO_BYTE_3_,      VS_TRUE,        0},
        {59,    _NCCCTMS_ISO_BYTE_3_,      VS_FALSE,       0},
        {60,    _NCCCTMS_ISO_BYTE_3_,      VS_TRUE,        0},
        {61,    _NCCCTMS_ISO_BYTE_3_,      VS_TRUE,        0},
        {62,    _NCCCTMS_ISO_BYTE_3_,      VS_TRUE,        0},
        {63,    _NCCCTMS_ISO_BYTE_3_,      VS_FALSE,       0},
        {0,     _NCCCTMS_ISO_BCD_,         VS_FALSE,       0},  /* 最後一組一定要放 0 */
};

int inNCCC_TMS_SCHEDULE[] = {3, 24, 41, 42, 0}; /* 最後一組一定要放 0!! */
int inNCCC_TMS_LOGON[] = {3, 24, 41, 42, 58, 0}; /* 最後一組一定要放 0!! */
int inNCCC_TMS_RECORD[] = {3, 24, 39, 41, 42, 58, 0}; /* 最後一組一定要放 0!! */
int inNCCC_TMS_TASK[] = {3, 24, 41, 42, 58, 0}; /* 最後一組一定要放 0!! */
int inNCCC_TMS_TRACE_LOG[] = {3, 24, 41, 42, 58, 61, 0}; /* 最後一組一定要放 0!! */
int inNCCC_TMS_DUTY_FREE[] = {3, 24, 41, 42, 58, 61, 0}; /* 最後一組一定要放 0!! */
int inNCCC_TMS_LOGOFF[] = {3, 24, 39, 41, 42, 0}; /* 最後一組一定要放 0!! */

int inNCCC_FTP_AUTO_INQUIRY_REPORT[] = {3, 24, 41, 42, 58, 61, 0}; /* 最後一組一定要放 0!! */
int inNCCC_FTP_DOWNLOAD_TMS_REPORT[] = {3, 24, 41, 42, 58, 61, 0}; /* 最後一組一定要放 0!! */

int inNCCC_TMS_LOGON_GET_FTPIDPW[] = {3, 24, 41, 42, 58, 0}; /* 最後一組一定要放 0!! */

BIT_MAP_NCCCTMS_TABLE srNCCCTMS_ISOBitMap[] =
{
	{_NCCCTMS_SCHEDULE_,			inNCCC_TMS_SCHEDULE,		"0800",		"000100"},	/* 下載詢問/回覆 */
	{_NCCCTMS_LOGON_,			inNCCC_TMS_LOGON,		"0800",		"000200"},	/* 下(上)載檔案清單請求/回覆 */
	{_NCCCTMS_RECORD_,			inNCCC_TMS_RECORD,		"0800",		"000300"},	/* 下載資料請求/回覆 */
	{_NCCCTMS_TASK_,			inNCCC_TMS_TASK,		"0800",		"000200"},	
	{_NCCCTMS_TRACE_LOG_,			inNCCC_TMS_TRACE_LOG,		"0800",		"000200"},	/* 傳送下(上)載結果/結果回覆 */
	{_NCCCTMS_LOGOFF_,			inNCCC_TMS_LOGOFF,		"0800",		"000400"},
        {_NCCCTMS_FTP_AUTO_INQUIRY_REPORT_,     inNCCC_FTP_AUTO_INQUIRY_REPORT,	"0800",		"000700"},
	{_NCCCTMS_FTP_DOWNLOAD_TMS_REPORT_,     inNCCC_FTP_DOWNLOAD_TMS_REPORT,	"0800",		"000800"},
	{_NCCCTMS_LOGON_FTPIDPW_,		inNCCC_TMS_LOGON_GET_FTPIDPW,	"0800",		"000200"},
	{_NCCCTMS_DUTYFREE_,			inNCCC_TMS_DUTY_FREE,		"0800",		"000200"},
	{_NCCCTMS_NULL_TX_,	                NULL,			        "0000",		"000000"},
};

ISO_TYPE_NCCCTMS_TABLE srNCCCTMS_ISOFunc[] =
{
        {
           srNCCCTMS_ISOFieldPack,
           srNCCCTMS_ISOFieldUnPack,
           srNCCCTMS_ISOFieldCheck,
           srNCCCTMS_ISOFieldType,
           srNCCCTMS_ISOBitMap,
           inNCCCTMS_ISOGetBitMapCode,
           inNCCCTMS_ISOPackMessageType,
           inNCCCTMS_ISOModifyBitMap,
           inNCCCTMS_ISOModifyPackData,
           inNCCCTMS_ISOCheckHeader,
           NULL,
	   NULL
        },
};

int inNCCCTMS_FuncSendReceive(TRANSACTION_OBJECT *pobTran)
{
        int		inRetVal = VS_ERROR;
        char		szTemplate[16 + 1] = {0};
	char		szDispTemplate[50 + 1] = {0};
        TMS_OBJECT	srTMS;

        vdUtility_SYSFIN_LogMessage(AT, "inNCCCTMS_FuncSendReceive");
        
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCCTMS_FuncSendReceive() START!");

	memset((char *)&srTMS, 0x00, sizeof(TMS_OBJECT));
	srTMS.inCode = pobTran->inTransactionCode;
	srTMS.uszRequest = 'Y'; /* 用來判斷是否要繼續【下載】或【上傳】 */
	srTMS.inTransactionResult = VS_SUCCESS; /* Response Code 使用 */

	srTMS.inListIndex = 0; /* 下載清單使用 */
	srTMS.inListTotalCount = 0; /* 下載清單【TMSFLT.txt】的總筆數 */
	srTMS.inPackNo = 1;

	/* Trace Log */
	srTMS.inTraceTotalSizes = 0;
	srTMS.inTraceSearchIndex = 0;
	srTMS.inTraceReadSizes = 0;

	if (srTMS.inCode == _NCCCTMS_TASK_)
		strcpy(srTMS.szTotalPacketCount, "0001"); /* 不帶【0001】系統會回錯誤訊息 */
	else
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
	inDISP_PutGraphic(_SEND_, 0, _COORDINATE_Y_LINE_8_7_);

	do
	{
		if (srTMS.inCode == _NCCCTMS_TRACE_LOG_)
			inNCCCTMS_CheckTraceLogFile(&srTMS, (unsigned char *)_TMS_TRACE_LOG_);
		else if (srTMS.inCode == _NCCCTMS_DUTYFREE_)
			inNCCCTMS_DUTY_FREE_PackData(&srTMS);

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
	/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/5/30 下午 1:30
	 * 客製化075、103重印上傳失敗後不要顯示通訊失敗 */
	if (pobTran->inTransactionCode == _NCCCTMS_DUTYFREE_)
	{
		
	}
	else
	{
		inNCCCTMS_DispHostResponseCode(&srTMS);
	}

	/* 列印 TMS 錯誤訊息報表(分兩種情況，一是有FileList，會顯示下載檔案成功或失敗，二是沒有FileList，要顯示列印錯誤訊息，(這裡是第二種)
	 * 這邊加入!= _NCCCTMS_AUTHORIZED_是因為要避免TMS詢問下載會印錯誤訊息的狀況) */
	/* 資訊回報不用印TMS下載失敗的 */
	/* 這邊用pobTran是為了看原本是什麼電文，否則用srTMS.incode都會變成LOG OFF */
	if (pobTran->inTransactionCode == _NCCCTMS_TRACE_LOG_)
	{
		if (srTMS.inTransactionResult != _NCCCTMS_AUTHORIZED_)
		{
			sprintf(szDispTemplate, "Resp Code = [%s]", srTMS.szRespCode);
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
			strcpy(srDispMsgObj.szErrMsg1, szDispTemplate);
			srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_ChineseFont("資訊回報成功", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
			inDISP_Wait(2000);
		}
	}
	else if (pobTran->inTransactionCode == _NCCCTMS_DUTYFREE_)
	{
		
	}
	else
	{
		if (srTMS.inTransactionResult != _NCCCTMS_AUTHORIZED_)
		{
			if (pobTran->inTMSDwdMode == _TMS_MANUAL_DOWNLOAD_)
			{
				/* 沒下成功FileList */
				if (inNCCCTMS_Check_FileList_Flow(pobTran) != VS_SUCCESS)
				{
					inNCCCTMS_PrintErrorResult(&srTMS);
				}
				/* 下載中間失敗 */
				else
				{

				}
			}
		}
		
	}
	
	/* 不論成功或失敗，都要帶回復碼回來 */
	strcpy(pobTran->srBRec.szRespCode, srTMS.szRespCode);
	pobTran->inTransactionResult = srTMS.inTransactionResult;

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCCTMS_FuncSendReceive() END!");

        return (VS_SUCCESS);
}

/*
Function        :inNCCCTMS_FuncSendReceive_Flow
Date&Time       :2018/6/28 下午 2:12
Describe        :
*/
int inNCCCTMS_FuncSendReceive_Flow(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;
	
	if (pobTran->uszFTP_TMS_Download == _TMS_DOWNLOAD_SECURE_MODE_FTPS_	||
	    pobTran->uszFTP_TMS_Download == _TMS_DOWNLOAD_SECURE_MODE_SFTP_)
	{
		inRetVal = inFLOW_RunFunction(pobTran, _FUNCTION_TMS_SEND_RECEIVE_FTPS_);
	}
	else
	{
		inRetVal = inFLOW_RunFunction(pobTran, _FUNCTION_TMS_SEND_RECEIVE_);
	}
	
	return (inRetVal);
}

/*
Function        :inNCCCTMS_FuncSendReceive_FTPS
Date&Time       :2018/6/27 下午 3:27
Describe        :只處理FTPS下載TermInfo2或檔案
*/
int inNCCCTMS_FuncSendReceive_FTPS(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;
	
	/* 自動詢問 */
	if (pobTran->inTransactionCode == _NCCCTMS_FTP_AUTO_INQUIRY_REPORT_)
	{
		/* 初始化curl庫 */
		if (inFTPS_Initial(pobTran) != VS_SUCCESS)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_CONNECT_);
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
		
		/* 下TermInfo2並分析 */
		inRetVal = inNCCCTMS_FTPS_TermInfo_Download(pobTran);
		/* 不管成功或失敗都要DeInitial */
		inFTPS_Deinitial(pobTran);
		if (inRetVal != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		/* 下載成功 */
		else
		{
			
		}
		
	}
	/* 自動下載*/
	else if (pobTran->inTransactionCode == _NCCCTMS_FTP_DOWNLOAD_TMS_REPORT_)
	{
		/* FTP有下就要回報 */
		inLoadTMSFTPRec(0);
		inSetFTPEffectiveReportBit("Y");
		inSaveTMSFTPRec(0);
		
		/* 初始化curl庫 */
		if (inFTPS_Initial(pobTran) != VS_SUCCESS)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_CONNECT_);
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
		
		/* 下載FTPFLT.dat的檔案 */
		inRetVal = inNCCCTMS_FTPS_APPARM_Download(pobTran);
		/* 不管成功或失敗都要DeInitial */
		inFTPS_Deinitial(pobTran);
		
		if (inRetVal != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		/* 下載成功，開始處理*/
		else
		{
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_TMS_DOWNLOAD_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜TMS參數下載＞ */
			inDISP_PutGraphic(_PROCESS_, 0, _COORDINATE_Y_LINE_8_7_);

			/* 檢查下載的檔案大小 */
			inNCCCTMS_FTPS_CheckFileSize(pobTran);
		}
	}
	else if (pobTran->inTransactionCode == _NCCCTMS_LOGON_)
	{
		/* FTP有下就要回報 */
		inLoadTMSFTPRec(0);
		inSetFTPEffectiveReportBit("Y");
		inSaveTMSFTPRec(0);
		
		/* 初始化curl庫 */
		if (inFTPS_Initial(pobTran) != VS_SUCCESS)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_CONNECT_);
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
		
		/* 下TermInfo2並分析 */
		inRetVal = inNCCCTMS_FTPS_TermInfo_Download(pobTran);
		if (inRetVal != VS_SUCCESS)
		{
			inFTPS_Deinitial(pobTran);
			return (VS_ERROR);
		}
		
		/* 下載FTPFLT.dat的檔案 */
		inRetVal = inNCCCTMS_FTPS_APPARM_Download(pobTran);
		/* 不管成功或失敗都要DeInitial */
		inFTPS_Deinitial(pobTran);
		
		if (inRetVal != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		/* 下載成功，開始處理*/
		else
		{
			inDISP_ClearAll();
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_TMS_DOWNLOAD_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜TMS參數下載＞ */
			inDISP_PutGraphic(_PROCESS_, 0, _COORDINATE_Y_LINE_8_7_);

			/* 檢查下載的檔案大小 */
			inNCCCTMS_FTPS_CheckFileSize(pobTran);
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "FTP 無此流程");
		}
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, "");
		srDispMsgObj.inDispPic1YPosition = 0;
		srDispMsgObj.inMsgType = _NO_KEY_MSG_;
		srDispMsgObj.inTimeout = 1;
		strcpy(srDispMsgObj.szErrMsg1, "FTP無此TMS流程");
		srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
		srDispMsgObj.inBeepTimes = 1;
		srDispMsgObj.inBeepInterval = 0;
		
		inDISP_Msg_BMP(&srDispMsgObj);

		return (VS_ERROR);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCCTMS_FuncSendReceive_FTPS() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

int inNCCCTMS_Pack03(TMS_OBJECT *srTMS, unsigned char *uszPackBuf)
{
        int	inCnt = 0;

	memcpy(&uszPackBuf[inCnt], &guszNCCCTMS_ISO_Field03[0], _NCCCTMS_PCODE_SIZE_);
	inCnt += _NCCCTMS_PCODE_SIZE_;

        return (inCnt);
}

/*
Function        :inNCCCTMS_Pack24
Date&Time       :2017/7/20 下午 3:32
Describe        :
*/
int inNCCCTMS_Pack24(TMS_OBJECT *srTMS, unsigned char *uszPackBuf)
{
        int		inCnt = 0;
	char		szTemplate[10 + 1];
	unsigned char	uszBCD[10 + 1];

	memset(szTemplate, 0x00, sizeof(szTemplate));

	/* TMS NII 固定292 */
        memcpy(&szTemplate[0], srTMS->szNII, strlen(srTMS->szNII));

	memset(uszBCD, 0x00, sizeof(uszBCD));
        inFunc_ASCII_to_BCD(&uszBCD[0], &szTemplate[0], 2);

	memcpy((char *)&uszPackBuf[inCnt], (char *)&uszBCD[0], 2);
	inCnt += 2;

	return (inCnt);
}

int inNCCCTMS_Pack39(TMS_OBJECT *srTMS, unsigned char *uszPackBuf)
{
        int 	inCnt = 0;

	memcpy((char *)&uszPackBuf[inCnt], &srTMS->szRespCode[0], 2);
	inCnt += 2;

	return (inCnt);
}

int inNCCCTMS_Pack41(TMS_OBJECT *srTMS, unsigned char *uszPackBuf)
{
        int	inCnt = 0;
	char	szTemplate[8 + 1];

	memset(szTemplate, 0x00, sizeof(szTemplate));

	if (inGetTerminalID(szTemplate) == VS_ERROR)
		return (VS_ERROR);

	memcpy((char *)&uszPackBuf[inCnt], szTemplate, strlen(szTemplate));
	inCnt += strlen(szTemplate);

	return (inCnt);
}

int inNCCCTMS_Pack42(TMS_OBJECT *srTMS, unsigned char *uszPackBuf)
{
        int	inCnt = 0;
	char	szTemplate[15 + 1];

	memset(szTemplate, 0x00, sizeof(szTemplate));

        if (inGetMerchantID(szTemplate) == VS_ERROR)
		return (VS_ERROR);

	inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 15, _PADDING_RIGHT_);
	memcpy((char *)&uszPackBuf[inCnt], szTemplate, strlen(szTemplate));
	inCnt += strlen(szTemplate);

	return (inCnt);
}

int inNCCCTMS_Pack58(TMS_OBJECT *srTMS, unsigned char *uszPackBuf)
{
	int	inCnt = 0;
        int	inPacketCnt = 0;
        int     i = 0, inSlash = 0, inFileNameLen = 0;
	char	szTemplate[60 + 1], szPacket[512 + 1];
        char	szASCII[4 + 1];
        unsigned char   uszBCD[2 + 1];

        memset(szTemplate, 0x00, sizeof(szTemplate));
        memset(szPacket, 0x00, sizeof(szPacket));

	if (srTMS->inCode == _NCCCTMS_FTP_AUTO_INQUIRY_REPORT_)
        {
                /* Management Code */
                memcpy(szPacket, "      ", 6);
                inPacketCnt +=6;

                /* Batch No */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetFTPBatchNum(szTemplate);
		/* 小於8的時候帶空白 */
		if (strlen(szTemplate) < 8)
		{
			memcpy(&szPacket[inPacketCnt], "        ", 8);
		}
		else
		{
			memcpy(&szPacket[inPacketCnt], szTemplate, 8);
		}
                inPacketCnt += 8;
		
		/* Terminal Boot Inqury Report */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetFTPInquiryResponseCode(szTemplate);
		if (szTemplate[0] == 0x00	||
		    szTemplate[0] == ' ')
		{
			memcpy(&szPacket[inPacketCnt], "X", 1);
		}
		else
		{
			memcpy(&szPacket[inPacketCnt], szTemplate, 1);
		}
		inPacketCnt += 1;
		
		/* Batch NO (至現回報使用，其他時候填空白) */
		memcpy(&szPacket[inPacketCnt], &gsrTMS_Field58.szBatchNumber, 8);
		inPacketCnt += 8;
		
                /* Terminal Version ID */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], &gsrTMS_Field58.szTerminalVersionID[0], 15);
                inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 15, _PADDING_RIGHT_);
                memcpy(&szPacket[inPacketCnt], &szTemplate[0], 15);
                inPacketCnt += 15;

                /* Terminal AP Version Date */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], &gsrTMS_Field58.szTerminalAPVersionDate[0], 8);
                inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 15, _PADDING_RIGHT_);
                memcpy(&szPacket[inPacketCnt], &szTemplate[0], 15);
                inPacketCnt += 15;

                /* Terminal Pactet Size */
                memcpy(&szPacket[inPacketCnt], &gsrTMS_Field58.szTerminalPacketSize[0], 4);
                inPacketCnt += 4;

                /* Total Packet Count */
		memcpy(&szPacket[inPacketCnt], &srTMS->szTotalPacketCount[0], 4);
		inPacketCnt += 4;

                /* Packet No */
                memcpy(&szPacket[inPacketCnt], &srTMS->szPacketNo[0], 4);
                inPacketCnt += 4;

                /* Terminal SN */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], &gsrTMS_Field58.szTerminalSN[0], 16);
                inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 16, _PADDING_RIGHT_);
                memcpy(&szPacket[inPacketCnt], &szTemplate[0], 16);
                inPacketCnt += 16;

                /* TSAM SLOT SN1 */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], &gsrTMS_Field58.szTSAMSLOTSN1[0], 16);
                inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 16, _PADDING_RIGHT_);
                memcpy(&szPacket[inPacketCnt], &szTemplate[0], 16);
                inPacketCnt += 16;
                /* TSAM SLOT SN2 */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], &gsrTMS_Field58.szTSAMSLOTSN2[0], 16);
                inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 16, _PADDING_RIGHT_);
                memcpy(&szPacket[inPacketCnt], &szTemplate[0], 16);
                inPacketCnt += 16;
                /* TSAM SLOT SN3 */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], &gsrTMS_Field58.szTSAMSLOTSN3[0], 16);
                inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 16, _PADDING_RIGHT_);
                memcpy(&szPacket[inPacketCnt], &szTemplate[0], 16);
                inPacketCnt += 16;
                /* TSAM SLOT SN4 */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], &gsrTMS_Field58.szTSAMSLOTSN4[0], 16);
                inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 16, _PADDING_RIGHT_);
                memcpy(&szPacket[inPacketCnt], &szTemplate[0], 16);
                inPacketCnt += 16;
                /* Terminal OS Vresion */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], &gsrTMS_Field58.szTerminalOSVersion[0], 16);
                inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 16, _PADDING_RIGHT_);
                memcpy(&szPacket[inPacketCnt], &szTemplate[0], 16);
                inPacketCnt += 16;
                /* EMV Approval Number */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], &gsrTMS_Field58.szEMVApprovalNumber[0], 32);
                inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 32, _PADDING_RIGHT_);
                memcpy(&szPacket[inPacketCnt], &szTemplate[0], 32);
                inPacketCnt += 32;
                /* EMV Application Kernel */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], &gsrTMS_Field58.szEMVApplicationKernel[0], 32);
                inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 32, _PADDING_RIGHT_);
                memcpy(&szPacket[inPacketCnt], &szTemplate[0], 32);
                inPacketCnt += 32;
                /* IP Address */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], &gsrTMS_Field58.szIPAddress[0], 15);
                inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 15, _PADDING_RIGHT_);
                memcpy(&szPacket[inPacketCnt], &szTemplate[0], 15);
                inPacketCnt += 15;
                /* Gateway Address  */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], &gsrTMS_Field58.szGatewayAddress[0], 15);
                inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 15, _PADDING_RIGHT_);
                memcpy(&szPacket[inPacketCnt], &szTemplate[0], 15);
                inPacketCnt += 15;
                /* MASk Address */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], &gsrTMS_Field58.szMASKAddress[0], 15);
                inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 15, _PADDING_RIGHT_);
                memcpy(&szPacket[inPacketCnt], &szTemplate[0], 15);
                inPacketCnt += 15;
                /* PABX */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], &gsrTMS_Field58.szPABX[0], 10);
                inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 10, _PADDING_RIGHT_);
                memcpy(&szPacket[inPacketCnt], &szTemplate[0], 10);
                inPacketCnt += 10;
                
		/* TermParmDateTime */
		memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetFTPTermParemeterDateTime(szTemplate);
                inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 12, _PADDING_RIGHT_);
                memcpy(&szPacket[inPacketCnt], &szTemplate[0], 12);
                inPacketCnt += 12;
		
		/* Receipt Image File Date */
		
		/* MAC Address */
	}
	else if (srTMS->inCode == _NCCCTMS_FTP_DOWNLOAD_TMS_REPORT_)
        {
                /* Management Code */
                memcpy(szPacket, "      ", 6);
                inPacketCnt +=6;

                /* Batch No */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetFTPBatchNum(szTemplate);
		/* 小於8的時候帶空白 */
		if (strlen(szTemplate) < 8)
		{
			memcpy(&szPacket[inPacketCnt], &gsrTMS_Field58.szBatchNumber[0], 8);
		}
		else
		{
			memcpy(&szPacket[inPacketCnt], szTemplate, 8);
		}
                inPacketCnt += 8;

		/* Download Category */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetFTPDownloadCategory(szTemplate);
		if (strlen(szTemplate) == 0)
		{
			memcpy(&szPacket[inPacketCnt], " ", 1);
		}
		else
		{
			memcpy(&szPacket[inPacketCnt], szTemplate, 1);
		}
		inPacketCnt += 1;
		
		/* Terminal Boot Inqury Report */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetFTPDownloadResponseCode(szTemplate);
		if (szTemplate[0] == 0x00	||
		    szTemplate[0] == ' ')
		{
			memcpy(&szPacket[inPacketCnt], "X", 1);
		}
		else
		{
			memcpy(&szPacket[inPacketCnt], szTemplate, 1);
		}
		inPacketCnt += 1;
		
                /* Batch NO (至現回報使用，其他時候填空白) */
		memcpy(&szPacket[inPacketCnt], &gsrTMS_Field58.szBatchNumber, 8);
		inPacketCnt += 8;
		
                /* Terminal Version ID */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], &gsrTMS_Field58.szTerminalVersionID[0], 15);
                inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 15, _PADDING_RIGHT_);
                memcpy(&szPacket[inPacketCnt], &szTemplate[0], 15);
                inPacketCnt += 15;

                /* Terminal AP Version Date */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], &gsrTMS_Field58.szTerminalAPVersionDate[0], 8);
                inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 15, _PADDING_RIGHT_);
                memcpy(&szPacket[inPacketCnt], &szTemplate[0], 15);
                inPacketCnt += 15;

                /* Terminal Pactet Size */
                memcpy(&szPacket[inPacketCnt], &gsrTMS_Field58.szTerminalPacketSize[0], 4);
                inPacketCnt += 4;

                /* Total Packet Count */
		memcpy(&szPacket[inPacketCnt], &srTMS->szTotalPacketCount[0], 4);
		inPacketCnt += 4;

                /* Packet No */
                memcpy(&szPacket[inPacketCnt], &srTMS->szPacketNo[0], 4);
                inPacketCnt += 4;

                /* Terminal SN */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], &gsrTMS_Field58.szTerminalSN[0], 16);
                inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 16, _PADDING_RIGHT_);
                memcpy(&szPacket[inPacketCnt], &szTemplate[0], 16);
                inPacketCnt += 16;

                /* TSAM SLOT SN1 */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], &gsrTMS_Field58.szTSAMSLOTSN1[0], 16);
                inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 16, _PADDING_RIGHT_);
                memcpy(&szPacket[inPacketCnt], &szTemplate[0], 16);
                inPacketCnt += 16;
                /* TSAM SLOT SN2 */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], &gsrTMS_Field58.szTSAMSLOTSN2[0], 16);
                inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 16, _PADDING_RIGHT_);
                memcpy(&szPacket[inPacketCnt], &szTemplate[0], 16);
                inPacketCnt += 16;
                /* TSAM SLOT SN3 */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], &gsrTMS_Field58.szTSAMSLOTSN3[0], 16);
                inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 16, _PADDING_RIGHT_);
                memcpy(&szPacket[inPacketCnt], &szTemplate[0], 16);
                inPacketCnt += 16;
                /* TSAM SLOT SN4 */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], &gsrTMS_Field58.szTSAMSLOTSN4[0], 16);
                inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 16, _PADDING_RIGHT_);
                memcpy(&szPacket[inPacketCnt], &szTemplate[0], 16);
                inPacketCnt += 16;
                /* Terminal OS Vresion */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], &gsrTMS_Field58.szTerminalOSVersion[0], 16);
                inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 16, _PADDING_RIGHT_);
                memcpy(&szPacket[inPacketCnt], &szTemplate[0], 16);
                inPacketCnt += 16;
                /* EMV Approval Number */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], &gsrTMS_Field58.szEMVApprovalNumber[0], 32);
                inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 32, _PADDING_RIGHT_);
                memcpy(&szPacket[inPacketCnt], &szTemplate[0], 32);
                inPacketCnt += 32;
                /* EMV Application Kernel */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], &gsrTMS_Field58.szEMVApplicationKernel[0], 32);
                inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 32, _PADDING_RIGHT_);
                memcpy(&szPacket[inPacketCnt], &szTemplate[0], 32);
                inPacketCnt += 32;
                /* IP Address */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], &gsrTMS_Field58.szIPAddress[0], 15);
                inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 15, _PADDING_RIGHT_);
                memcpy(&szPacket[inPacketCnt], &szTemplate[0], 15);
                inPacketCnt += 15;
                /* Gateway Address  */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], &gsrTMS_Field58.szGatewayAddress[0], 15);
                inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 15, _PADDING_RIGHT_);
                memcpy(&szPacket[inPacketCnt], &szTemplate[0], 15);
                inPacketCnt += 15;
                /* MASk Address */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], &gsrTMS_Field58.szMASKAddress[0], 15);
                inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 15, _PADDING_RIGHT_);
                memcpy(&szPacket[inPacketCnt], &szTemplate[0], 15);
                inPacketCnt += 15;
                /* PABX */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], &gsrTMS_Field58.szPABX[0], 10);
                inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 10, _PADDING_RIGHT_);
                memcpy(&szPacket[inPacketCnt], &szTemplate[0], 10);
                inPacketCnt += 10;
                
		/* TermParmDateTime */
		memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetFTPTermParemeterDateTime(szTemplate);
                inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 12, _PADDING_RIGHT_);
                memcpy(&szPacket[inPacketCnt], &szTemplate[0], 12);
                inPacketCnt += 12;
	}
        else if (srTMS->inCode == _NCCCTMS_LOGON_		|| 
		 srTMS->inCode == _NCCCTMS_TRACE_LOG_		||
		 srTMS->inCode == _NCCCTMS_TASK_		||
		 srTMS->inCode == _NCCCTMS_LOGON_FTPIDPW_	||
		 srTMS->inCode == _NCCCTMS_DUTYFREE_)
        {
                /* Management Code */
                memcpy(szPacket, gsrTMS_Field58.szManagementCode, 6);
                inPacketCnt +=6;

                /* Download Scope */
		/* 問帳密直接hardcode帶B */
		if (srTMS->inCode == _NCCCTMS_LOGON_FTPIDPW_)
		{
			memcpy(&szPacket[inPacketCnt], "B", 1);
			inPacketCnt ++;
		}
		else
		{
			memcpy(&szPacket[inPacketCnt], &gsrTMS_Field58.szDownloadScope[0], 1);
			inPacketCnt ++;
		}

                /* Batch No */
                memcpy(&szPacket[inPacketCnt], &gsrTMS_Field58.szBatchNumber[0], 8);
                inPacketCnt += 8;

                /* Terminal Version ID */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], &gsrTMS_Field58.szTerminalVersionID[0], 15);
                inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 15, _PADDING_RIGHT_);
                memcpy(&szPacket[inPacketCnt], &szTemplate[0], 15);
                inPacketCnt += 15;

                /* Terminal AP Version Date */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], &gsrTMS_Field58.szTerminalAPVersionDate[0], 8);
                inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 8, _PADDING_RIGHT_);
                memcpy(&szPacket[inPacketCnt], &szTemplate[0], 8);
                inPacketCnt += 8;

                /* TSAM Indicator */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                strcpy(szTemplate, "1      ");
                memcpy(&szPacket[inPacketCnt], &szTemplate[0], 7);
                inPacketCnt += 7;

                /* Terminal Pactet Size */
                memcpy(&szPacket[inPacketCnt], &gsrTMS_Field58.szTerminalPacketSize[0], 4);
                inPacketCnt += 4;

                /* Total Packet Count */
                if (srTMS->inCode == _NCCCTMS_TRACE_LOG_)
			memcpy(&szPacket[inPacketCnt], "0001", 4);
                else
			memcpy(&szPacket[inPacketCnt], &srTMS->szTotalPacketCount[0], 4);
                inPacketCnt += 4;

                /* Packet No */
                memcpy(&szPacket[inPacketCnt], &srTMS->szPacketNo[0], 4);
                inPacketCnt += 4;

                /* Terminal SN */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], &gsrTMS_Field58.szTerminalSN[0], 16);
                inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 16, _PADDING_RIGHT_);
                memcpy(&szPacket[inPacketCnt], &szTemplate[0], 16);
                inPacketCnt += 16;

                /* TSAM SLOT SN1 */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], &gsrTMS_Field58.szTSAMSLOTSN1[0], 16);
                inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 16, _PADDING_RIGHT_);
                memcpy(&szPacket[inPacketCnt], &szTemplate[0], 16);
                inPacketCnt += 16;
                /* TSAM SLOT SN2 */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], &gsrTMS_Field58.szTSAMSLOTSN2[0], 16);
                inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 16, _PADDING_RIGHT_);
                memcpy(&szPacket[inPacketCnt], &szTemplate[0], 16);
                inPacketCnt += 16;
                /* TSAM SLOT SN3 */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], &gsrTMS_Field58.szTSAMSLOTSN3[0], 16);
                inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 16, _PADDING_RIGHT_);
                memcpy(&szPacket[inPacketCnt], &szTemplate[0], 16);
                inPacketCnt += 16;
                /* TSAM SLOT SN4 */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], &gsrTMS_Field58.szTSAMSLOTSN4[0], 16);
                inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 16, _PADDING_RIGHT_);
                memcpy(&szPacket[inPacketCnt], &szTemplate[0], 16);
                inPacketCnt += 16;
                /* Terminal OS Vresion */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], &gsrTMS_Field58.szTerminalOSVersion[0], 16);
                inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 16, _PADDING_RIGHT_);
                memcpy(&szPacket[inPacketCnt], &szTemplate[0], 16);
                inPacketCnt += 16;
                /* EMV Approval Number */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], &gsrTMS_Field58.szEMVApprovalNumber[0], 32);
                inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 32, _PADDING_RIGHT_);
                memcpy(&szPacket[inPacketCnt], &szTemplate[0], 32);
                inPacketCnt += 32;
                /* EMV Application Kernel */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], &gsrTMS_Field58.szEMVApplicationKernel[0], 32);
                inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 32, _PADDING_RIGHT_);
                memcpy(&szPacket[inPacketCnt], &szTemplate[0], 32);
                inPacketCnt += 32;
                /* IP Address */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], &gsrTMS_Field58.szIPAddress[0], 15);
                inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 15, _PADDING_RIGHT_);
                memcpy(&szPacket[inPacketCnt], &szTemplate[0], 15);
                inPacketCnt += 15;
                /* Gateway Address  */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], &gsrTMS_Field58.szGatewayAddress[0], 15);
                inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 15, _PADDING_RIGHT_);
                memcpy(&szPacket[inPacketCnt], &szTemplate[0], 15);
                inPacketCnt += 15;
                /* MASk Address */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], &gsrTMS_Field58.szMASKAddress[0], 15);
                inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 15, _PADDING_RIGHT_);
                memcpy(&szPacket[inPacketCnt], &szTemplate[0], 15);
                inPacketCnt += 15;
                /* PABX */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], &gsrTMS_Field58.szPABX[0], 10);
                inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 10, _PADDING_RIGHT_);
                memcpy(&szPacket[inPacketCnt], &szTemplate[0], 10);
                inPacketCnt += 10;
                /* On site Report Flag (1 Byte) */
                if (srTMS->inCode == _NCCCTMS_TASK_)
		{
                        memcpy(&szPacket[inPacketCnt], &gsrTMS_Field58.szOnsiteReportFlag[0], 1);
                        inPacketCnt ++;
                }
                else
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        strcpy(szTemplate, "0"); /* 0 非至現回報 */
                        memcpy(&szPacket[inPacketCnt], &szTemplate[0], 1);
                        inPacketCnt ++;
                }
                /* On site Report Data (60 Byte) */
                if (srTMS->inCode == _NCCCTMS_TASK_)
		{
                        memcpy(&szPacket[inPacketCnt], &gsrTMS_Field58.szOnsiteReportData[0], 60);
                        inPacketCnt += 60;
                }
                else
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 60, _PADDING_RIGHT_);
                        memcpy(&szPacket[inPacketCnt], &szTemplate[0], 60);
                        inPacketCnt += 60;
                }
        }
        else if (srTMS->inCode == _NCCCTMS_RECORD_)
        {
                if (inLoadTMSFLTRec(srTMS->inListIndex) < 0)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                sprintf(szTemplate, "inLoadTMSFLTRec(%d) Error!", srTMS->inListIndex);
                                inLogPrintf(AT, szTemplate);
                        }

                        return (VS_ERROR);
                }

                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetTMSFileAttribute(szTemplate);

                /* AP/Param Flag */
                if (szTemplate[0] == 'P')
                {
                        /* 參數下載 */
                        szPacket[inPacketCnt] = 0x32; /* Parameter Download */
                        inPacketCnt ++;

                }
                else
                {
                        /* AP下載 */
                        szPacket[inPacketCnt] = 0x31;
                        inPacketCnt ++;
                }

                /* File Path Name */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetTMSFilePathName(szTemplate);

                /* 取得檔案路徑後 抓取檔名儲存到TMS FileName */
                for (i = 0; i < 60 ; i++)
                {
                        if (szTemplate[i] == '/')
                            inSlash = i + 1;

                        if (szTemplate[i] == 0x00)
                            break;
                }

                inFileNameLen = i - inSlash;
                memset(srTMS->szFileName, 0x00, sizeof(srTMS->szFileName));
                memcpy(&srTMS->szFileName[0], &szTemplate[inSlash], inFileNameLen);
                inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 60, _PADDING_RIGHT_);
                memcpy(&szPacket[inPacketCnt], &szTemplate[0], 60);
                inPacketCnt += 60;

                /* Terminal Version ID */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], &gsrTMS_Field58.szTerminalVersionID[0], 15);
                inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 15, _PADDING_RIGHT_);
                memcpy(&szPacket[inPacketCnt], &szTemplate[0], 15);
                inPacketCnt += 15;

                /* Terminal Pactet Size */
                memcpy(&szPacket[inPacketCnt], &gsrTMS_Field58.szTerminalPacketSize[0], 4);
                inPacketCnt += 4;

                /* Total Packet Count */
                strcat(szPacket, srTMS->szTotalPacketCount);
                inPacketCnt += 4;
                /* Packet No */
                strcat(szPacket, srTMS->szPacketNo);
                inPacketCnt += 4;

        }
        else if (srTMS->inCode == _NCCCTMS_LOGOFF_)
        {
		strcpy(szPacket, srTMS->szErrFilePathName); /* File Path Name */
		inPacketCnt += 60;                
        }

        /* Packet Data Length */
	memset(szASCII, 0x00, sizeof(szASCII));
	sprintf(szASCII, "%04d", inPacketCnt);
        memset(uszBCD, 0x00, sizeof(uszBCD));
        inFunc_ASCII_to_BCD(uszBCD, szASCII, 2);
	memset(uszPackBuf, 0x00, sizeof(uszPackBuf));
	memcpy(&uszPackBuf[inCnt], &uszBCD[0], 2);
	inCnt += 2;

        /* Packet Data */
	memcpy(&uszPackBuf[inCnt], &szPacket[0], inPacketCnt);
	inCnt += inPacketCnt;
	return	(inCnt);
}

int inNCCCTMS_Pack61(TMS_OBJECT *srTMS, unsigned char *uszPackBuf)
{
	int	i = 0;
        int	inCnt = 0;
        char    szASCII[4 + 1];
	char	szPrintBuffer[128 + 1] = {0}, szTemplate[512 + 1] = {0};
	unsigned char   uszBCD[2 + 1];
	
        /* Packet Data Length */
        memset(szASCII, 0x00, sizeof(szASCII));
        sprintf(szASCII, "%04d", srTMS->inTraceReadSizes);
        memset(uszBCD, 0x00, sizeof(uszBCD));
        inFunc_ASCII_to_BCD(uszBCD, szASCII, 2);
        memset(uszPackBuf, 0x00, sizeof(uszPackBuf));
        memcpy(&uszPackBuf[inCnt], &uszBCD[0], 2);
        inCnt += 2;

        /* Packet Data */
        memcpy(&uszPackBuf[inCnt], &srTMS->uszField61[0], srTMS->inTraceReadSizes);
        inCnt += srTMS->inTraceReadSizes;
	
	if (ginISODebug == VS_TRUE)
	{
		if (srTMS->inCode == _NCCCTMS_DUTYFREE_)
		{
			inPRINT_ChineseFont("***********重印明細資料Field 61***********", _PRT_ISO_);

			/* 2	欄位長度 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inFunc_BCD_to_ASCII(szTemplate, uszBCD, 2);
			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
			sprintf(szPrintBuffer, "欄位長度 = [%s]", szTemplate);
			inPRINT_ChineseFont(szPrintBuffer, _PRT_ISO_);

			/* 1	「I」代表新格式 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &srTMS->uszField61[i], 1);
			i += 1;
			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
			sprintf(szPrintBuffer, "新格式 = [%s]", szTemplate);
			inPRINT_ChineseFont(szPrintBuffer, _PRT_ISO_);

			/* 15	特店代號(該筆易的MID，左靠右補空白) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &srTMS->uszField61[i], 15);
			i += 15;
			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
			sprintf(szPrintBuffer, "特店代號 = [%s]", szTemplate);
			inPRINT_ChineseFont(szPrintBuffer, _PRT_ISO_);

			/* 6	收銀員工號(櫃號前6碼) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &srTMS->uszField61[i], 6);
			i += 6;
			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
			sprintf(szPrintBuffer, "收銀員工號 = [%s]", szTemplate);
			inPRINT_ChineseFont(szPrintBuffer, _PRT_ISO_);

			/* 8	端末機代號 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &srTMS->uszField61[i], 8);
			i += 8;
			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
			sprintf(szPrintBuffer, "端末機代號 = [%s]", szTemplate);
			inPRINT_ChineseFont(szPrintBuffer, _PRT_ISO_);

			/* 6	批號 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &srTMS->uszField61[i], 6);
			i += 6;
			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
			sprintf(szPrintBuffer, "批號 = [%s]", szTemplate);
			inPRINT_ChineseFont(szPrintBuffer, _PRT_ISO_);

			/* 6	調閱編號 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &srTMS->uszField61[i], 6);
			i += 6;
			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
			sprintf(szPrintBuffer, "調閱編號 = [%s]", szTemplate);
			inPRINT_ChineseFont(szPrintBuffer, _PRT_ISO_);

			/* 19	卡號(保留前六後四，左靠右補空白) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &srTMS->uszField61[i], 19);
			i += 19;
			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
			sprintf(szPrintBuffer, "卡號 = [%s]", szTemplate);
			inPRINT_ChineseFont(szPrintBuffer, _PRT_ISO_);

			/* 10	交易金額(不含小數位，右靠左補空白) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &srTMS->uszField61[i], 10);
			i += 10;
			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
			sprintf(szPrintBuffer, "交易金額 = [%s]", szTemplate);
			inPRINT_ChineseFont(szPrintBuffer, _PRT_ISO_);

			/* 12	原交易日期時間(YYYYMMDDhhmm) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &srTMS->uszField61[i], 12);
			i += 12;
			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
			sprintf(szPrintBuffer, "原交易日期時間 = [%s]", szTemplate);
			inPRINT_ChineseFont(szPrintBuffer, _PRT_ISO_);

			/* 12	重印日期時間(YYYYMMDDhhmm) */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(&szTemplate[0], &srTMS->uszField61[i], 12);
			i += 12;
			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
			sprintf(szPrintBuffer, "重印日期時間 = [%s]", szTemplate);
			inPRINT_ChineseFont(szPrintBuffer, _PRT_ISO_);

			/* 1	LCR */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inFunc_BCD_to_ASCII(&szTemplate[0], &srTMS->uszField61[i], 1);
			i += 1;
			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
			sprintf(szPrintBuffer, "LRC = [0x%s]", szTemplate);
			inPRINT_ChineseFont(szPrintBuffer, _PRT_ISO_);

			inPRINT_ChineseFont(" ", _PRT_ISO_);
			inPRINT_ChineseFont(" ", _PRT_ISO_);
			inPRINT_ChineseFont(" ", _PRT_ISO_);
		}
	}

	return (inCnt);
}

int inNCCCTMS_Check03(TMS_OBJECT *srTMS, unsigned char *uszSendPacket, unsigned char *uszRecePacket)
{
        if (memcmp((char *)uszSendPacket, (char *)uszRecePacket, 3))
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inNCCCTMS_Check03 Error!");

                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

int inNCCCTMS_UnPack39(TMS_OBJECT *srTMS, unsigned char *uszUnPackBuf)
{
        memset(srTMS->szRespCode, 0x00, sizeof(srTMS->szRespCode));
	memcpy(&srTMS->szRespCode[0], (char *)&uszUnPackBuf[0], 2);
	srTMS->szRespCode[2] = 0x00;

        return (VS_SUCCESS);
}

int inNCCCTMS_UnPack58(TMS_OBJECT *srTMS, unsigned char *uszUnPackBuf)
{
	int 	inLen, inRetVal = VS_ERROR;

	if (!memcmp(&srTMS->szRespCode[0], "00", 2))
	{
		inLen = (uszUnPackBuf[0] / 16 * 1000) + (uszUnPackBuf[0] % 16 * 100) +
		        (uszUnPackBuf[1] / 16 * 10) + (uszUnPackBuf[1] % 16);

		/* 儲存FTP帳號密碼 */
		if (srTMS->inCode == _NCCCTMS_LOGON_FTPIDPW_)
		{
			inRetVal = inNCCCTMS_ISO_StoreFTPT(srTMS, &uszUnPackBuf[2], inLen);
		}
		else
		{
			inRetVal = inNCCCTMS_ISO_StoreTMSObject(srTMS, &uszUnPackBuf[2], inLen);
		}

		if (inRetVal == VS_ERROR)
		{
			memset(srTMS->szRespCode, 0x00, sizeof(srTMS->szRespCode));
                        /* S1 下載檔案寫入端末機時，發生錯誤 */
			strcpy(srTMS->szRespCode, "S1");
		}
	}

        return (VS_SUCCESS);
}

int inNCCCTMS_UnPack61(TMS_OBJECT *srTMS, unsigned char *uszUnPackBuf)
{
        int 	inLen, inRetVal = VS_ERROR;

	if (!memcmp(&srTMS->szRespCode[0], "00", 2))
	{
		inLen = (uszUnPackBuf[0] / 16 * 1000) + (uszUnPackBuf[0] % 16 * 100) +
		        (uszUnPackBuf[1] / 16 * 10) + (uszUnPackBuf[1] % 16);

                /* 檢查LRC是否正確 */
                inRetVal = inNCCCTMS_ISO_CheckLRC(srTMS, &uszUnPackBuf[2], (inLen - 1));

		if (inRetVal == VS_SUCCESS)
		{
                        uszUnPackBuf[2 + inLen] = 0x00;
                        memset(srTMS->uszField61, 0x00, sizeof(srTMS->uszField61));
                        memcpy(&srTMS->uszField61[0], &uszUnPackBuf[2], (inLen - 1));

                        /* 儲存Filed 61檔案長度 */
                        srTMS->lnFileSize = inLen - 1;
                }
                else
                {
                        memset(srTMS->szRespCode, 0x00, sizeof(srTMS->szRespCode));
                        /* S7 Checksum檢核錯誤 */
                        strcpy(srTMS->szRespCode, "S7");
                }
	}
        else
        {
                memset(srTMS->uszField61, 0x00, sizeof(srTMS->uszField61));
        }

        return (VS_SUCCESS);
}

int inNCCCTMS_ISOGetBitMapCode(TMS_OBJECT *srTMS, int inTxnType)
{
	int	inBitMapTxnCode = -1;

	switch (inTxnType)
	{
		default :
			inBitMapTxnCode = inTxnType;
			break;
	}

        return (inBitMapTxnCode);
}

int inNCCCTMS_ISOPackMessageType(TMS_OBJECT *srTMS, int inTxnCode, unsigned char *uszPackData, char *szMTI)
{
	int	inCnt = 0;
	unsigned char uszBCD[10 + 1];

	memset(uszBCD, 0x00, sizeof(uszBCD));
        inFunc_ASCII_to_BCD(&uszBCD[0], &szMTI[0], _NCCCTMS_MTI_SIZE_);
	memcpy((char *)&uszPackData[inCnt], (char *)&uszBCD[0], _NCCCTMS_MTI_SIZE_);
	inCnt += _NCCCTMS_MTI_SIZE_;

        return (inCnt);
}

int inNCCCTMS_ISOModifyBitMap(TMS_OBJECT *srTMS, int inTxnType, int *inBitMap)
{
	if (srTMS->inCode == _NCCCTMS_LOGOFF_)
	{
                /* 端末機比對所收到之所有檔案大小與TMS Logon Response通知之檔案大小是否相符 */
                /* Filed 58 帶 Download Fail of Application or Parameter Name */
		if ((strlen(srTMS->szErrFilePathName)) > 0)
			inNCCCTMS_BitMapSet(inBitMap, 58);
	}
        
        return (VS_SUCCESS);
}

int inNCCCTMS_ISOModifyPackData(TMS_OBJECT *srTMS, unsigned char *uszPackData, int *inPackLen)
{
        return (VS_SUCCESS);
}

int inNCCCTMS_ISOCheckHeader(TMS_OBJECT *srTMS, char *szSendISOHeader, char *szReceISOHeader)
{
        return (VS_SUCCESS);
}

int inNCCCTMS_ISOAnalyse(TMS_OBJECT *srTMS)
{
        int		inRetVal = -1, i = 0;
        unsigned long   ulFile_Handle;  /* File Handle */
        unsigned char   uszFileName[16 + 1];
        char		szTemplate[60 + 1];

        /* 回應碼不是00 return Error */
        if (memcmp(&srTMS->szRespCode[0], "00", 2))
        {
                return (VS_ERROR);
        }

        /* TMS LOG ON 存下載清單 */
        if (srTMS->inCode == _NCCCTMS_LOGON_ && srTMS->uszField61[0] != 0x00)
        {
                memset(uszFileName, 0x00, sizeof(uszFileName));
                strcpy((char *)uszFileName, _TMSFLT_FILE_NAME_);

                /* 第一段刪除檔案 */
                if ((srTMS->inPackNo - 1) == 0)
                        inFILE_Delete(uszFileName);

                /* 先判斷檔案是否存在 */
                inRetVal = inFILE_Check_Exist(uszFileName);

                /* 檔案不存在 */
                if (inRetVal == VS_ERROR)
                {
                        inRetVal = inFILE_Create(&ulFile_Handle, uszFileName);

                        if (inRetVal != VS_SUCCESS)
                        {
                                memset(srTMS->szRespCode, 0x00, sizeof(srTMS->szRespCode));
                                /* S1 下載檔案寫入端末機時，發生錯誤 */
                                strcpy(srTMS->szRespCode, "S1");
                                inFILE_Close(&ulFile_Handle);
                                return (VS_ERROR);
                        }

                        inRetVal = inFILE_Write(&ulFile_Handle, srTMS->uszField61, srTMS->lnFileSize);

                        if (inRetVal != VS_SUCCESS)
                        {
                                memset(srTMS->szRespCode, 0x00, sizeof(srTMS->szRespCode));
                                /* S1 下載檔案寫入端末機時，發生錯誤 */
                                strcpy(srTMS->szRespCode, "S1");
                                inFILE_Close(&ulFile_Handle);
                                return (VS_ERROR);
                        }

                        inFILE_Close(&ulFile_Handle);
                }
                else
                {
                        /* 檔案存在 */
                        inRetVal = inFILE_Open(&ulFile_Handle, uszFileName);

                        if (inRetVal != VS_SUCCESS)
                        {
                                memset(srTMS->szRespCode, 0x00, sizeof(srTMS->szRespCode));
                                /* S1 下載檔案寫入端末機時，發生錯誤 */
                                strcpy(srTMS->szRespCode, "S1");
                                inFILE_Close(&ulFile_Handle);
                                return (VS_ERROR);
                        }

                        inRetVal = inFILE_Seek(ulFile_Handle, 0, _SEEK_END_);

                        if (inRetVal != VS_SUCCESS)
                        {
                                memset(srTMS->szRespCode, 0x00, sizeof(srTMS->szRespCode));
                                /* S1 下載檔案寫入端末機時，發生錯誤 */
                                strcpy(srTMS->szRespCode, "S1");
                                inFILE_Close(&ulFile_Handle);
                                return (VS_ERROR);
                        }

                        inRetVal = inFILE_Write(&ulFile_Handle, srTMS->uszField61, srTMS->lnFileSize);

                        if (inRetVal != VS_SUCCESS)
                        {
                                memset(srTMS->szRespCode, 0x00, sizeof(srTMS->szRespCode));
                                /* S1 下載檔案寫入端末機時，發生錯誤 */
                                strcpy(srTMS->szRespCode, "S1");
                                inFILE_Close(&ulFile_Handle);
                                return (VS_ERROR);
                        }

                        inFILE_Close(&ulFile_Handle);
                }

                /* inCode轉成檔案下載 */
                if (srTMS->inPackNo == srTMS->inTotalPacketCount)
                {
                        /* 存參數生效時間日期 */
                        inSaveTMSCPTRec(0);

                        /* 取得File List後開始下載 */
                        srTMS->inCode = _NCCCTMS_RECORD_;
                         /* 初始值從第0個Record下載 */
                        srTMS->inListIndex = 0;
                        srTMS->uszRequest = 'Y';
                        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                        inDISP_PutGraphic(_RECEIVE_, 0, _COORDINATE_Y_LINE_8_5_);

                        for (i = 0 ;; i ++)
                        {
                                if (inLoadTMSFLTRec(i) < 0)
                                {
                                        /* 計算總共要下載的record數量 */
                                        srTMS->inListTotalCount = i - 1;
                                        break;
                                }
                        }

                        strcpy(srTMS->szTotalPacketCount, "0000"); /* 初始化 */
                        strcpy(srTMS->szPacketNo, "0001"); /* 初始化 */
                }

        }
        else if (srTMS->inCode == _NCCCTMS_LOGON_ && srTMS->uszField61[0] == 0x00)
        {
                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                inDISP_PutGraphic(_RECEIVE_, 0, _COORDINATE_Y_LINE_8_5_);
                srTMS->inCode = _NCCCTMS_LOGOFF_;
                srTMS->uszRequest = 'Y';
                inRetVal = VS_SUCCESS;
        }
        /* TMS Record 下載檔案 */
        else if (srTMS->inCode == _NCCCTMS_RECORD_)
        {
                if (ginDebug == VS_TRUE)
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "inListTotalCount = %d", srTMS->inListTotalCount);
                        inLogPrintf(AT, szTemplate);
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "inListIndex = %d", srTMS->inListIndex);
                        inLogPrintf(AT, szTemplate);
                }

                memset(uszFileName, 0x00, sizeof(uszFileName));
                strcpy((char *)uszFileName, srTMS->szFileName);

                /* 第一段刪除檔案 */
                if ((srTMS->inPackNo - 1) == 0)
                        inFILE_Delete(uszFileName);

                /* 先判斷檔案是否存在 */
                inRetVal = inFILE_Check_Exist(uszFileName);

                /* 檔案不存在 */
                if (inRetVal == VS_ERROR)
                {
                        inRetVal = inFILE_Create(&ulFile_Handle, uszFileName);

                        if (inRetVal != VS_SUCCESS)
                        {
                                memset(srTMS->szRespCode, 0x00, sizeof(srTMS->szRespCode));
                                /* S1 下載檔案寫入端末機時，發生錯誤 */
                                strcpy(srTMS->szRespCode, "S1");
                                inFILE_Close(&ulFile_Handle);
                                return (VS_ERROR);
                        }

                        inRetVal = inFILE_Write(&ulFile_Handle, srTMS->uszField61, srTMS->lnFileSize);

                        if (inRetVal != VS_SUCCESS)
                        {
                                memset(srTMS->szRespCode, 0x00, sizeof(srTMS->szRespCode));
                                /* S1 下載檔案寫入端末機時，發生錯誤 */
                                strcpy(srTMS->szRespCode, "S1");
                                inFILE_Close(&ulFile_Handle);
                                return (VS_ERROR);
                        }

                        inFILE_Close(&ulFile_Handle);
                }
                else
                {
                        /* 檔案存在 */
                        inRetVal = inFILE_Open(&ulFile_Handle, uszFileName);

                        if (inRetVal != VS_SUCCESS)
                        {
                                memset(srTMS->szRespCode, 0x00, sizeof(srTMS->szRespCode));
                                /* S1 下載檔案寫入端末機時，發生錯誤 */
                                strcpy(srTMS->szRespCode, "S1");
                                inFILE_Close(&ulFile_Handle);
                                return (VS_ERROR);
                        }

                        inRetVal = inFILE_Seek(ulFile_Handle, 0, _SEEK_END_);

                        if (inRetVal != VS_SUCCESS)
                        {
                                memset(srTMS->szRespCode, 0x00, sizeof(srTMS->szRespCode));
                                /* S1 下載檔案寫入端末機時，發生錯誤 */
                                strcpy(srTMS->szRespCode, "S1");
                                inFILE_Close(&ulFile_Handle);
                                return (VS_ERROR);
                        }

                        inRetVal = inFILE_Write(&ulFile_Handle, srTMS->uszField61, srTMS->lnFileSize);

                        if (inRetVal != VS_SUCCESS)
                        {
                                memset(srTMS->szRespCode, 0x00, sizeof(srTMS->szRespCode));
                                /* S1 下載檔案寫入端末機時，發生錯誤 */
                                strcpy(srTMS->szRespCode, "S1");
                                inFILE_Close(&ulFile_Handle);
                                return (VS_ERROR);
                        }

                        inFILE_Close(&ulFile_Handle);
                }

                /* 判斷檔案封包是否已下載完 */
                if (srTMS->inPackNo == srTMS->inTotalPacketCount)
                {
                        /* 下載TMSFLT下一個Record */
                        srTMS->inListIndex ++;

                        /* 判斷全部檔案下載完 */
                        if (srTMS->inListIndex > srTMS->inListTotalCount)
                        {
                                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                                inDISP_PutGraphic(_PROCESS_, 0, _COORDINATE_Y_LINE_8_7_);
                                
                                srTMS->uszRequest = 'Y';
                                srTMS->inCode = _NCCCTMS_LOGOFF_;

                                /* 檢查下載的檔案大小與File List比對 */
                                inNCCCTMS_CheckFileSize(srTMS);
                        }
                        else
                        {
                                srTMS->uszRequest = 'Y';
                                inRetVal = inLoadTMSFLTRec(srTMS->inListIndex);

                                if (inRetVal != VS_SUCCESS)
                                {
                                        memset(srTMS->szRespCode, 0x00, sizeof(srTMS->szRespCode));
                                        /* S1 下載檔案寫入端末機時，發生錯誤 */
                                        strcpy(srTMS->szRespCode, "S1");
                                        inFILE_Close(&ulFile_Handle);
                                        return (VS_ERROR);
                                }

                                strcpy(srTMS->szTotalPacketCount, "0000"); /* 初始化 */
                                strcpy(srTMS->szPacketNo, "0001"); /* 初始化 */
                        }
                }
        }
        else if (srTMS->inCode == _NCCCTMS_SCHEDULE_)
        {
                inSaveTMSCPTRec(0);
                /* 立即下載 */
                if (srTMS->inDownloadFlag == atoi(_TMS_DOWNLOAD_FLAG_IMMEDIATE_))
                {
                        srTMS->inCode = _NCCCTMS_LOGON_;
                        srTMS->uszRequest = 'Y';
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "%d", srTMS->inDownlaodScope);
                        memset(gsrTMS_Field58.szDownloadScope, 0x00, sizeof(gsrTMS_Field58.szDownloadScope));
                        memcpy(&gsrTMS_Field58.szDownloadScope[0], &szTemplate[0], 1);
                        strcpy(srTMS->szTotalPacketCount, "0000"); /* 初始化 */
                        strcpy(srTMS->szPacketNo, "0001"); /* 初始化 */
                        return (VS_SUCCESS);
                }
                /* 排程下載 */
                else if (srTMS->inDownloadFlag == atoi(_TMS_DOWNLOAD_FLAG_SCHEDULE_))
                {
                        srTMS->uszRequest = 'N';
                        return (VS_SUCCESS);
                }
                /* 不需下載(_TMS_DOWNLOAD_FLAG_NO_) */
                else
                {
                        srTMS->uszRequest = 'N';
                        return (VS_SUCCESS);
                }
        }
	else if (srTMS->inCode == _NCCCTMS_TASK_)
	{
		/* 敏華說至現回報要回 LOGOFF */
		if (!memcmp(&srTMS->szTotalPacketCount[0], &srTMS->szPacketNo[0], 4))
                {
                        srTMS->inCode = _NCCCTMS_LOGOFF_;
                        srTMS->uszRequest = 'Y';
                        inRetVal = VS_SUCCESS;
                }    
                else
                {
                        srTMS->inPackNo = atoi(srTMS->szPacketNo);
                        srTMS->inPackNo ++;
                        memset(srTMS->szPacketNo, 0x00, sizeof(srTMS->szPacketNo));
                        sprintf(srTMS->szPacketNo, "%04d", srTMS->inPackNo);
                        inLogPrintf(AT, srTMS->szPacketNo);
                        srTMS->uszRequest = 'Y';
                        inRetVal = VS_SUCCESS;
                }
	}
        else if (srTMS->inCode == _NCCCTMS_TRACE_LOG_)
        {
                /* Trace Log上傳完畢 */
                if (!memcmp(&srTMS->szTotalPacketCount[0], &srTMS->szPacketNo[0], 4))
                {
                        srTMS->inCode = _NCCCTMS_LOGOFF_;
                        srTMS->uszRequest = 'Y';
                        inRetVal = VS_SUCCESS;
                }    
                else
                {
                        srTMS->inPackNo = atoi(srTMS->szPacketNo);
                        srTMS->inPackNo ++;
                        memset(srTMS->szPacketNo, 0x00, sizeof(srTMS->szPacketNo));
                        sprintf(srTMS->szPacketNo, "%04d", srTMS->inPackNo);
                        inLogPrintf(AT, srTMS->szPacketNo);
                        srTMS->uszRequest = 'Y';
                        inRetVal = VS_SUCCESS;
                }
        }
	else if (srTMS->inCode == _NCCCTMS_LOGON_FTPIDPW_)
	{
		/* 取得FTP帳密後LOGOFF */
		srTMS->inCode = _NCCCTMS_LOGOFF_;
		srTMS->uszRequest = 'Y';
		inRetVal = VS_SUCCESS;
	}
	else if (srTMS->inCode == _NCCCTMS_FTP_AUTO_INQUIRY_REPORT_	||
		 srTMS->inCode == _NCCCTMS_FTP_DOWNLOAD_TMS_REPORT_)
	{
		/* 送完封包之後 就不需要再多送封包 , 在預期裡 也只會有 一個封包的資訊量 , 不做 LOG OFF */
		srTMS->uszRequest = 'N';
		inRetVal = VS_SUCCESS;
	}
        else if (srTMS->inCode == _NCCCTMS_LOGOFF_)
        {
		/* 如果 LOGOFF 失敗則所有下載算失敗，重新來一次 */
		if (srTMS->inTransactionResult != _NCCCTMS_AUTHORIZED_)
		{
			srTMS->uszRequest = 'N';
			inRetVal = VS_ERROR;
		}
		else
		{
			/* AP下載完，做Full參數Download */
			if (!memcmp(&gsrTMS_Field58.szDownloadScope[0], "1", 1))
			{
				/* 要更新APP */
				inSetTMSAPPUpdateStatus("Y");
				inSaveTMSCPTRec(0);

				srTMS->inCode = _NCCCTMS_LOGON_;
				strcpy(gsrTMS_Field58.szDownloadScope, "2");
				strcpy(srTMS->szTotalPacketCount, "0000"); /* 初始化 */
				strcpy(srTMS->szPacketNo, "0001"); /* 初始化 */
				srTMS->uszRequest = 'Y';
				inRetVal = VS_SUCCESS;
			}
			else
			{
				srTMS->uszRequest = 'N';
				inRetVal = VS_SUCCESS;
			}
		}
        }
	
	return (inRetVal);
}

int inNCCCTMS_BitMapSet(int *inBitMap, int inFeild)
{
	int 	i, inBMapCnt, inBitMapCnt;
	int 	inBMap[_NCCCTMS_MAX_BIT_MAP_CNT_];

	memset((char *)inBMap, 0x00, sizeof(inBMap));
	inBMapCnt = inBitMapCnt = 0;

	for (i = 0; i < 64; i ++)
	{
		if (inBitMap[inBitMapCnt] > inFeild)
		{
			if (i == 0)
			{
				/* 第一個BitMap */
				inBMap[inBMapCnt ++] = inFeild;
				break;
			}
		}
		else if (inBitMap[inBitMapCnt] < inFeild)
		{
			inBMap[inBMapCnt ++] = inBitMap[inBitMapCnt ++];
			if (inBitMap[inBitMapCnt] == 0 || inBitMap[inBitMapCnt] > inFeild) /* 最後一個 BitMap */
			{
				inBMap[inBMapCnt ++] = inFeild;
				break;
			}
		}
		else
			break; /* if (*inBitMap == inFeild) */
	}

	for (i = 0 ;; i ++)
	{
		if (inBitMap[inBitMapCnt] != 0)
			inBMap[inBMapCnt ++] = inBitMap[inBitMapCnt ++];
		else
			break;
	}

	memcpy((char *)inBitMap, (char *)inBMap, sizeof(inBMap));

	return (VS_SUCCESS);
}

int inNCCCTMS_BitMapReset(int *inBitMap, int inFeild)
{
	int 	i, inBMapCnt;
	int 	inBMap[_NCCCTMS_MAX_BIT_MAP_CNT_];

	memset((char *)inBMap, 0x00, sizeof(inBMap));
	inBMapCnt = 0;

	for (i = 0; i < 64; i ++)
	{
		if (inBitMap[i] == 0)
			break;
		else if (inBitMap[i] != inFeild)
			inBMap[inBMapCnt ++] = inBitMap[i];
	}

	memcpy((char *)inBitMap, (char *)inBMap, sizeof(inBMap));

	return (VS_SUCCESS);
}

int inNCCCTMS_BitMapCheck(unsigned char *inBitMap, int inFeild)
{
	int	inByteIndex, inBitIndex;

	inFeild --;
	inByteIndex = inFeild / 8;
	inBitIndex  = 7 - (inFeild - inByteIndex * 8);

	if (_NCCCTMS_CHECK_BIT_MAP_(inBitMap[inByteIndex], inBitIndex))
	{
		return (VS_TRUE);
	}

	return (VS_FALSE);
}

int inNCCCTMS_CopyBitMap(int *inBitMap, int *inSearchBitMap)
{
	int 	i;

	for (i = 0; i < _NCCCTMS_MAX_BIT_MAP_CNT_; i ++)
	{
		if (inSearchBitMap[i] == 0)
			break;
		else
			inBitMap[i] = inSearchBitMap[i];
	}

	return (VS_SUCCESS);
}

int inNCCCTMS_GetBitMapTableIndex(ISO_TYPE_NCCCTMS_TABLE *srISOFunc, int inBitMapTxnCode)
{
	int	inBitMapIndex;

	for (inBitMapIndex = 0 ;; inBitMapIndex ++)
	{
		if (srISOFunc->srBitMap[inBitMapIndex].inTxnID == _NCCCTMS_NULL_TX_)
		{
			return (VS_ERROR); /* 找不到相對應交易別的Bit Map */
		}

		if (srISOFunc->srBitMap[inBitMapIndex].inTxnID == inBitMapTxnCode)
			break; /* 找到一樣的交易類別 */
	}

	return (inBitMapIndex);
}

int inNCCCTMS_GetBitMapMessagegTypeField03(TMS_OBJECT *srTMS, ISO_TYPE_NCCCTMS_TABLE *srISOFunc, int inTxnType, int *inTxnBitMap,
                unsigned char *uszSendBuf)
{
	int	i, j, k, inBitMapIndex, *inBitMap, inCnt, inBitMapTxnCode = -1;
        unsigned char 	uszBuf;

	/* 設定交易別 */
	inBitMapTxnCode = srISOFunc->inGetBitMapCode(srTMS, inTxnType);
	if (inBitMapTxnCode == -1)
		return (VS_ERROR);

	/* 要搜尋 BIT_MAP_NCCCTMS_TABLE srNCCCTMS_ISOBitMap 相對應的 inTxnID */
	if ((inBitMapIndex = inNCCCTMS_GetBitMapTableIndex(srISOFunc, inBitMapTxnCode)) == VS_ERROR)
		return (VS_ERROR);

	/* Pack Message Type */
	inCnt = 0;
	inCnt += srISOFunc->inPackMTI(srTMS, inTxnType, &uszSendBuf[inCnt], srISOFunc->srBitMap[inBitMapIndex].szMTI);

	/* 要搜尋 BIT_MAP_NCCCTMS_TABLE srNCCCTMS_ISOBitMap 相對應的 inBitMap */
	inNCCCTMS_CopyBitMap(inTxnBitMap, srISOFunc->srBitMap[inBitMapIndex].inBitMap);

	/* 修改 Bit Map */
	srISOFunc->inModifyBitMap(srTMS, inTxnType, inTxnBitMap);
	/* Pack Bit Map */
	inBitMap = inTxnBitMap;
	for (i = 0 ;; i ++)
	{
		uszBuf = 0x80;
		if (*inBitMap == 0)
			break;

		j = *inBitMap / 8;
		k = *inBitMap % 8;

		if (k == 0)
		{
			j --;
			k = 8;
		}

		k --;

		while (k)
		{
			uszBuf = uszBuf >> 1;
			k --;
		}

		uszSendBuf[inCnt + j] += uszBuf;
		inBitMap ++;
	}

	inCnt += _NCCCTMS_BIT_MAP_SIZE_;

	/* Process Code */
	memset(guszNCCCTMS_ISO_Field03, 0x00, sizeof(guszNCCCTMS_ISO_Field03));
        inFunc_ASCII_to_BCD(&guszNCCCTMS_ISO_Field03[0], srISOFunc->srBitMap[inBitMapIndex].szPCode, 3);

	return (inCnt);
}

int inNCCCTMS_PackISO(TMS_OBJECT *srTMS, unsigned char *uszSendBuf, int inTxnCode)
{
	int	i, inSendCnt, inField, inCnt;
	int 	inBitMap[_NCCCTMS_MAX_BIT_MAP_CNT_ + 1];
	int	inRetVal, inISOFuncIndex = -1;
        char    szLogMessage[40 + 1];
	unsigned char uszBCD[20 + 1];
	ISO_TYPE_NCCCTMS_TABLE srISOFunc;

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCCTMS_PackISO() START!");

	inSendCnt = 0;
	inField = 0;

	/* 加密模式，預設值 = 0。 */
	inISOFuncIndex = 0; /* 不加密 */
	/* 決定要執行第幾個 Function Index */
	memset((char *)&srISOFunc, 0x00, sizeof(srISOFunc));
	memcpy((char *)&srISOFunc, (char *)&srNCCCTMS_ISOFunc[inISOFuncIndex], sizeof(srISOFunc));
	memset((char *)inBitMap, 0x00, sizeof(inBitMap));

	/* 開始組 ISO 電文 */
	/* Pack TPDU */
/*
	memset(szTemplate, 0x00, sizeof(szTemplate));
	if (inGetTPDU(szTemplate) != VS_SUCCESS)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inGetTPDU() ERROR!");

		return (VS_ERROR);
        }
*/

	memset(uszBCD, 0x00, sizeof(uszBCD));
        inFunc_ASCII_to_BCD(&uszBCD[0], &srTMS->szTPDU[0], 5);
	memcpy((char *)&uszSendBuf[inSendCnt], (char *)&uszBCD[0], _NCCCTMS_TPDU_SIZE_);
	inSendCnt += _NCCCTMS_TPDU_SIZE_;
	/* Get Bit Map / Mesage Type / Processing Code */
	inRetVal = inNCCCTMS_GetBitMapMessagegTypeField03(srTMS, &srISOFunc, inTxnCode, inBitMap, &uszSendBuf[inSendCnt]);
	if (inRetVal == VS_ERROR)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inNCCCTMS_GetBitMapMessagegTypeField03() ERROR!");

		return (VS_ERROR);
        }
	else
		inSendCnt += inRetVal; /* Bit Map 長度 */

	for (i = 0 ;; i ++)
	{
		if (inBitMap[i] == 0)
			break;

		while (inBitMap[i] > srISOFunc.srPackISO[inField].inFieldNum)
		{
			inField ++;
		}

		if (inBitMap[i] == srISOFunc.srPackISO[inField].inFieldNum)
		{
			inCnt = srISOFunc.srPackISO[inField].inISOLoad(srTMS, &uszSendBuf[inSendCnt]);
			if (inCnt <= 0)
                        {
                                if (ginDebug == VS_TRUE)
                                {
                                        memset(szLogMessage, 0x00, sizeof(szLogMessage));
                                        sprintf(szLogMessage, "inField = %d Error", inField);
                                        inLogPrintf(AT, szLogMessage);
				}

                                return (VS_ERROR);
                        }
			else
				inSendCnt += inCnt;
		}
	}

	if (srISOFunc.inModifyPackData != _NCCCTMS_NULL_TX_)
		srISOFunc.inModifyPackData(srTMS, uszSendBuf, &inSendCnt);

	return (inSendCnt);
}

int inNCCCTMS_CheckUnPackField(int inField, ISO_FIELD_NCCCTMS_TABLE *srCheckUnPackField)
{
	int 	i;

	for (i = 0; i < 64; i ++)
	{
		if (srCheckUnPackField[i].inFieldNum == 0)
			break;
		else if (srCheckUnPackField[i].inFieldNum == inField)
		{
			return (VS_SUCCESS); /* i 是 ISO_FIELD_TABLE 裡的第幾個 */
		}
	}

	return (VS_ERROR);
}

int inNCCCTMS_GetCheckField(int inField, ISO_CHECK_NCCCTMS_TABLE *ISOFieldCheck)
{
	int 	i;

	for (i = 0; i < 64; i ++)
	{
		if (ISOFieldCheck[i].inFieldNum == 0)
			break;
		else if (ISOFieldCheck[i].inFieldNum == inField)
		{
			return (i); /* i 是 ISO_CHECK_NCCCTMS_TABLE 裡的第幾個 */
		}
	}

	return (VS_ERROR);
}

int inNCCCTMS_GetFieldLen(int inField, unsigned char *uszSendData, ISO_FIELD_TYPE_NCCCTMS_TABLE *srFieldType)
{
	int 	inCnt = 0, i, inLen;

	for (i = 0 ;; i ++)
	{
		if (srFieldType[i].inFieldNum == 0)
			break;

		if (srFieldType[i].inFieldNum != inField)
			continue;

		switch (srFieldType[i].inFieldType)
		{
			case _NCCCTMS_ISO_ASC_ :
				inCnt += srFieldType[i].inFieldLen;
				break;
			case _NCCCTMS_ISO_BCD_ :
				inCnt += srFieldType[i].inFieldLen / 2;
				break;
			case _NCCCTMS_ISO_NIBBLE_2_ :
				inLen = (uszSendData[0] / 16 * 10) + (uszSendData[0] % 16);
				inCnt += ((inLen + 1) / 2) + 1;
				break;
			case _NCCCTMS_ISO_NIBBLE_3_ :
				inLen = (uszSendData[0] / 16 * 1000) + (uszSendData[0] % 16 * 100) + (uszSendData[1] / 16 * 10) + (uszSendData[1] % 16);
				inCnt += ((inLen + 1) / 2) + 2;
				break;
			case _NCCCTMS_ISO_BYTE_2_ :
				inLen = (uszSendData[0] / 16 * 10) + (uszSendData[0] % 16);
				inCnt += inLen + 1;
				break;
			case _NCCCTMS_ISO_BYTE_3_ :
				inLen = (uszSendData[0] / 16 * 1000) + (uszSendData[0] % 16 * 100) + (uszSendData[1] / 16 * 10) + (uszSendData[1] % 16);
				inCnt += inLen + 2;
				break;
			case _NCCCTMS_ISO_BYTE_2_H_ :
				inLen = (int)uszSendData[0];
				inCnt += inLen + 1;
				break;
			case _NCCCTMS_ISO_BYTE_3_H_ :
				inLen = ((int)uszSendData[0] * 0xFF) + (int)uszSendData[1];
				inCnt += inLen + 1;
				break;
			default :
				break;
		}

		break;
	}

	return (inCnt);
}

int inNCCCTMS_UnPackISO(TMS_OBJECT *srTMS, unsigned char *uszSendBuf, unsigned char *uszRecvBuf)
{
	int			i, inRecvCnt, inCnt, inSendCnt, inSendField, inRecvField, inISOFuncIndex = -1;
	char			szBuf[_NCCCTMS_TPDU_SIZE_ + _NCCCTMS_MTI_SIZE_ + _NCCCTMS_BIT_MAP_SIZE_ + 1];
        char 			szErrorMessage[40 + 1];
	unsigned char		uszSendMap[_NCCCTMS_BIT_MAP_SIZE_ + 1], uszReceMap[_NCCCTMS_BIT_MAP_SIZE_ + 1];
	ISO_TYPE_NCCCTMS_TABLE	srISOFunc;

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCCTMS_UnPackISO() START!");

	inSendField = inRecvField = 0;
	inSendCnt = inRecvCnt = 0;

	memset((char *)uszSendMap, 0x00, sizeof(uszSendMap));
	memset((char *)uszReceMap, 0x00, sizeof(uszReceMap));
	memset((char *)szBuf, 0x00, sizeof(szBuf));
	/* 加密模式，預設值 = 0。【0 = 不加密，1 = tSAM 加密，2 = 軟體加密】 */
	inISOFuncIndex = 0; /* 不加密 */
	memset((char *)&srISOFunc, 0x00, sizeof(srISOFunc));
	memcpy((char *)&srISOFunc, (char *)&srNCCCTMS_ISOFunc[inISOFuncIndex], sizeof(srISOFunc));

	if (srISOFunc.inCheckISOHeader != NULL)
	{
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "srISOFunc.inCheckISOHeader != NULL");

		if (srISOFunc.inCheckISOHeader(srTMS, (char *)&uszSendBuf[inRecvCnt], (char *)&uszRecvBuf[inRecvCnt]) != VS_SUCCESS)
                {
                        if (ginDebug == VS_TRUE)
                                inLogPrintf(AT, "srISOFunc.inCheckISOHeader ERROR");

                        return (_NCCCTMS_UNPACK_ERR_);
                }
	}

	inSendCnt += _NCCCTMS_TPDU_SIZE_;
	inRecvCnt += _NCCCTMS_TPDU_SIZE_;
	inSendCnt += _NCCCTMS_MTI_SIZE_;
	inRecvCnt += _NCCCTMS_MTI_SIZE_;

	memcpy((char *)uszSendMap, (char *)&uszSendBuf[inSendCnt], _NCCCTMS_BIT_MAP_SIZE_);
	memcpy((char *)uszReceMap, (char *)&uszRecvBuf[inRecvCnt], _NCCCTMS_BIT_MAP_SIZE_);

	inSendCnt += _NCCCTMS_BIT_MAP_SIZE_;
	inRecvCnt += _NCCCTMS_BIT_MAP_SIZE_;

	/* 先檢查 ISO Field_39 */
	if (inNCCCTMS_BitMapCheck(uszReceMap, 39) == VS_FALSE)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inNCCCTMS_BitMapCheck(39) ERROR");

		return (VS_ERROR);
        }

	for (i = 1; i <= 64; i ++)
	{
		/* 有送出去的 Field 但沒有收回來的 Field */
		if (inNCCCTMS_BitMapCheck(uszSendMap, i) && !inNCCCTMS_BitMapCheck(uszReceMap, i))
		{
			inSendCnt += inNCCCTMS_GetFieldLen(i, &uszSendBuf[inSendCnt], srISOFunc.srISOFieldType);
		}
		else if (inNCCCTMS_BitMapCheck(uszReceMap, i)) /* 收的 BitMap */
		{
			if (inNCCCTMS_BitMapCheck(uszSendMap, i)) /* 送的 BitMap 都有 */
			{
				/* 是否要進行檢查封包資料包含【送】【收】 */
				if ((inSendField = inNCCCTMS_GetCheckField(i, srISOFunc.srCheckISO)) != VS_ERROR)
				{
					if (srISOFunc.srCheckISO[inSendField].inISOCheck(srTMS, &uszSendBuf[inSendCnt], &uszRecvBuf[inRecvCnt]) != VS_SUCCESS)
                                        {
                                                if (ginDebug == VS_TRUE)
                                                {
                                                        memset(szErrorMessage, 0x00, sizeof(szErrorMessage));
                                                        sprintf(szErrorMessage, "inSendField = %d Error!", inSendField);
                                                        inLogPrintf(AT, szErrorMessage);
                                                }

						return (_NCCCTMS_UNPACK_ERR_);
                                        }
				}

				inSendCnt += inNCCCTMS_GetFieldLen(i, &uszSendBuf[inSendCnt], srISOFunc.srISOFieldType);
			}

			while (i > srISOFunc.srUnPackISO[inRecvField].inFieldNum) /* 只有收 */
			{
				inRecvField ++;
			}

			if (i == srISOFunc.srUnPackISO[inRecvField].inFieldNum)
			{
				/* 要先檢查 UnpackISO 是否存在 */
				if (inNCCCTMS_CheckUnPackField(i, srISOFunc.srUnPackISO) == VS_SUCCESS)
					srISOFunc.srUnPackISO[inRecvField].inISOLoad(srTMS, &uszRecvBuf[inRecvCnt]);
			}

			inCnt = inNCCCTMS_GetFieldLen(i, &uszRecvBuf[inRecvCnt], srISOFunc.srISOFieldType);
			if (inCnt == VS_ERROR)
			{
                                if (ginDebug == VS_TRUE)
                                {
                                        memset(szErrorMessage, 0x00, sizeof(szErrorMessage));
                                        sprintf(szErrorMessage, "inRecvField = %d Error!", inRecvField);
                                        inLogPrintf(AT, szErrorMessage);
                                }

				return (_NCCCTMS_UNPACK_ERR_);
			}

			inRecvCnt += inCnt;
		}
	}

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCCTMS_UnPackISO() END!");

	return (VS_SUCCESS);
}

int inNCCCTMS_CommSendRecvToHost(TMS_OBJECT *srTMS, unsigned char *uszSendPacket, int inSendLen, unsigned char *uszRecvPacket)
{
        int		inRetVal;
        int		inSendTimeout = 10;		
        int		inReceiveTimeout = 30;
        int		inReceiveSize = _COMM_RECEIVE_MAX_LENGTH_;		/* 配合Ingenico */
	int		inPacketNo = 0;
	int		inPacketTotalNo = 0;
	int		inNowDownloadPercent = 0;
	char		szTimeOut[2 + 1];
        char		szHostResponseTimeOut[2 + 1];
        char		szDispMsg[16 + 1];
	unsigned char	uszDispBit = VS_TRUE;
        
	inPacketNo = atoi(srTMS->szPacketNo);
	inPacketTotalNo = atoi(srTMS->szTotalPacketCount);
		
        if (srTMS->inCode == _NCCCTMS_RECORD_)
        {
                if (!memcmp(&srTMS->szPacketNo[0], "0001", 4))
		{
                        inDISP_Clear_Area(1, _LINE_8_5_, 16, _LINE_8_5_, _ENGLISH_FONT_8X16_);
                        
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			strcpy(szDispMsg, srTMS->szFileName);
			inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);
		}

		if (inPacketTotalNo > 20)
		{
			/* 0001時尚未回傳總封包數 */
			if (!memcmp(&srTMS->szPacketNo[0], "0002", 4))
			{
				giNowDispPercent = 0;
				inDISP_Clear_Area(1, _LINE_8_6_, 16, _LINE_8_6_, _ENGLISH_FONT_8X16_);

				memset(szDispMsg, 0x00, sizeof(szDispMsg));
				sprintf(szDispMsg, "%d%%", giNowDispPercent);
				inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);
			}
			else
			{
				inNowDownloadPercent = (100 * inPacketNo) / inPacketTotalNo;
				/* 至少每一秒顯示一次，不然會拖慢下載速度 */
				/* 100%時也顯示一次 */
				if (inNowDownloadPercent > giNowDispPercent	||
				    giNowDispPercent == 100)
				{
					giNowDispPercent = inNowDownloadPercent;
					inDISP_Clear_Area(1, _LINE_8_6_, 16, _LINE_8_6_, _ENGLISH_FONT_8X16_);

					memset(szDispMsg, 0x00, sizeof(szDispMsg));
					sprintf(szDispMsg, "%d%%", giNowDispPercent);
					inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);
				}
			}
		}
		else
		{
			if (!memcmp(&srTMS->szPacketNo[0], "0001", 4))
			{
				inDISP_Clear_Area(1, _LINE_8_6_, 16, _LINE_8_6_, _ENGLISH_FONT_8X16_);

				memset(szDispMsg, 0x00, sizeof(szDispMsg));
				sprintf(szDispMsg, "%s/%s", srTMS->szTotalPacketCount, srTMS->szPacketNo);
				inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);
			}
			else
			{
				/* 至少每一秒顯示一次，不然會拖慢下載速度 */
				/* 100%時也顯示一次 */
				if (inTimerGet(_TIMER_NEXSYS_4_) == VS_SUCCESS	||
				    inPacketNo == inPacketTotalNo)
				{
					inDISP_Clear_Area(1, _LINE_8_6_, 16, _LINE_8_6_, _ENGLISH_FONT_8X16_);

					memset(szDispMsg, 0x00, sizeof(szDispMsg));
					sprintf(szDispMsg, "%s/%s", srTMS->szTotalPacketCount, srTMS->szPacketNo);
					inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

					inDISP_Timer_Start(_TIMER_NEXSYS_4_, 1);
				}
			}
		}
        }
        
	/* 如果沒設定TimeOut，就用EDC.dat內的TimeOut */
	if (inSendTimeout <= 0)
	{
		memset(szTimeOut, 0x00, sizeof(szTimeOut));
		inGetIPSendTimeout(szTimeOut);
		inSendTimeout = atoi(szTimeOut);
	}
	
	/* 顯示ISO Debug */
        if (ginDebug == VS_TRUE)
	{
		if (inPacketNo <= 20)
		{
			vdNCCCTMS_ISO_FormatDebug_DISP(srTMS, &uszSendPacket[0], inSendLen);
		}
	}
	/* 列印ISO Debug */
        if (ginISODebug == VS_TRUE)
	{
		if (inPacketNo <= 20)
		{
			vdNCCCTMS_ISO_FormatDebug_PRINT(srTMS, &uszSendPacket[0], inSendLen);
		}
	}
	
	if (memcmp(gszTermVersionID, "MD731UAGAS001", strlen("MD731UAGAS001")) == 0)
	{
		/* 不顯示訊息 */
		uszDispBit = VS_FALSE;
	}
	else
	{
		/* 不顯示訊息 */
		uszDispBit = VS_FALSE;
	}
	inRetVal = inCOMM_Send(uszSendPacket, inSendLen, inSendTimeout, uszDispBit);
	
        if (inRetVal != VS_SUCCESS)
        {
                if (inRetVal == VS_TIMEOUT)
                        strcpy(srTMS->szRespCode, "TO"); /* TimeOut */

                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inETHERNET_NCCCTMS_Send() ERROR");

                return (inRetVal);
        }

        /* CPT的授權等候時間 */
        memset(szHostResponseTimeOut, 0x00, sizeof(szHostResponseTimeOut));
        inGetHostResponseTimeOut(szHostResponseTimeOut);

        if (atoi(szHostResponseTimeOut) > 0)
                inReceiveTimeout = atoi(szHostResponseTimeOut);

	if (memcmp(gszTermVersionID, "MD731UAGAS001", strlen("MD731UAGAS001")) == 0)
	{
		/* 不顯示訊息 */
		uszDispBit = VS_FALSE;
	}
	else
	{
		/* 不顯示訊息 */
		uszDispBit = VS_FALSE;
	}
	inRetVal = inCOMM_Receive(uszRecvPacket, inReceiveSize, inReceiveTimeout, uszDispBit);

        if (inRetVal != VS_SUCCESS)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inETHERNET_NCCCTMS_Receive() ERROR");

                if (inRetVal == VS_TIMEOUT)
                {
                        strcpy(srTMS->szRespCode, "TO"); /* TimeOut */
                        return (VS_TIMEOUT);
                }

                return (VS_ERROR);
        }

	/* 顯示ISO Debug */
	/* APdownload時printf NCCCAPDL.zip的 debug會crash */
        if (ginDebug == VS_TRUE)
	{
		if (inPacketNo <= 20)
		{
			vdNCCCTMS_ISO_FormatDebug_DISP(srTMS, &uszRecvPacket[0], inRetVal);
		}
	}
        /* 列印ISO Debug */
        if (ginISODebug == VS_TRUE)
	{
		if (inPacketNo <= 20)
		{
			vdNCCCTMS_ISO_FormatDebug_PRINT(srTMS, &uszRecvPacket[0], inRetVal);
		}
	}

	return (VS_SUCCESS);
}

int inNCCCTMS_SendPackRecvUnPack(TRANSACTION_OBJECT *pobTran, TMS_OBJECT *srTMS)
{
	int		inSendCnt = 0, inRecvCnt = 0, inRetVal;
	unsigned char	uszSendPacket[_NCCCTMS_ISO_SEND_ + 1], uszRecvPacket[_NCCCTMS_ISO_RECV_ + 1];

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCCTMS_SendPackRecvUnPack() START!");

        memset(uszSendPacket, 0x00, sizeof(uszSendPacket));
        memset(uszRecvPacket, 0x00, sizeof(uszRecvPacket));

        /* 組 ISO 電文 */
        if ((inSendCnt = inNCCCTMS_PackISO(srTMS, uszSendPacket, srTMS->inCode)) <= 0)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inNCCCTMS_PackISO() Error!");

                strcpy(srTMS->szRespCode, "LB"); /* ISO 封包錯誤 */
                return (_NCCCTMS_PACK_ERR_); /* 組電文錯誤 */
        }

        /* 傳送及接收 ISO 電文 */
        if ((inRecvCnt = inNCCCTMS_CommSendRecvToHost(srTMS, uszSendPacket, inSendCnt, uszRecvPacket)) != VS_SUCCESS)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inNCCCTMS_CommSendRecvToHost() Error");

                return (VS_ERROR);
        }

        /* 解 ISO 電文 */
        inRetVal = inNCCCTMS_UnPackISO(srTMS, uszSendPacket, uszRecvPacket);

        if (inRetVal != VS_SUCCESS)
        {
                strcpy(srTMS->szRespCode, "LB"); /* ISO 封包錯誤 */
                inRetVal = _NCCCTMS_UNPACK_ERR_;
        }

	return (inRetVal);
}

int inNCCCTMS_CheckRespCode(TMS_OBJECT *srTMS)
{
	int	inRetVal = _NCCCTMS_COMM_ERROR_;

	if (!memcmp(srTMS->szRespCode, "00", 2))
	{
		inRetVal = _NCCCTMS_AUTHORIZED_;
	}
	else if (!memcmp(srTMS->szRespCode, "  ", 2))
        {
		inRetVal = _NCCCTMS_UNPACK_ERR_; /* 不是定義的 Response Code */
	}
        else if (srTMS->szRespCode[0] == 0x00)
	{
                inRetVal = _NCCCTMS_COMM_ERROR_;
	}
        else
	{
                inRetVal = _NCCCTMS_CANCELLED_;
	}

	return (inRetVal);
}

int inNCCCTMS_DispHostResponseCode(TMS_OBJECT *srTMS)
{
	int		inChoice = _DisTouch_No_Event_;
	int		inPic1Y = _COORDINATE_Y_LINE_8_1_;
	char		szResponseCode[10 + 1] = {0};
	char		szCustomerIndicator[3 + 1] = {0};
	char		szPic1Path[50 + 1] = {0};
        unsigned char   uszKey = 0x00;

	memset(szPic1Path, 0x00, sizeof(szPic1Path));
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);

        if (srTMS->inTransactionResult == _NCCCTMS_AUTHORIZED_)
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		
                return (VS_SUCCESS);
	}
        else if (srTMS->inTransactionResult == _NCCCTMS_COMM_ERROR_)
        {
		strcpy(szPic1Path, _ERR_CONNECT_);
		inPic1Y = _COORDINATE_Y_LINE_8_6_;
        }
        else if (srTMS->inTransactionResult == _NCCCTMS_UNPACK_ERR_ || srTMS->inTransactionResult == _NCCCTMS_PACK_ERR_)
        {
		/* 電文錯誤 */
		strcpy(szPic1Path, _ERR_ISO_);
		inPic1Y = _COORDINATE_Y_LINE_8_6_;
        }
        else if (srTMS->inTransactionResult == _NCCCTMS_CANCELLED_)
        {
                memset(szResponseCode, 0x00, sizeof(szResponseCode));
                sprintf(szResponseCode, "%s", srTMS->szRespCode);
		
		strcpy(szPic1Path, _ERR_TMS_DWL_FAILED_);
		inPic1Y = _COORDINATE_Y_LINE_8_6_;
        }
	
	/* 如果當SDK，不用顯示 */
	if (memcmp(gszTermVersionID, "MD731UAGAS001", strlen("MD731UAGAS001")) == 0)
	{
		
	}
	/* 在客製化107.111的時候將請按確認鍵跟請按清除鍵這兩個字樣移除 */
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)		||
                 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_)	||
		 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)			||
		 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
	{
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, szPic1Path);
		srDispMsgObj.inDispPic1YPosition = inPic1Y;
		srDispMsgObj.inMsgType = _NO_KEY_MSG_;
		srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
		strcpy(srDispMsgObj.szErrMsg1, szResponseCode);
		srDispMsgObj.inErrMsg1Line = _LINE_8_5_;
		srDispMsgObj.inBeepTimes = 1;
		srDispMsgObj.inBeepInterval = 0;
		inDISP_Msg_BMP(&srDispMsgObj);

		/* 為了強調Timeout時間 */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	}
	else
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		
		inDISP_PutGraphic(szPic1Path, 0, inPic1Y);
		inDISP_ChineseFont(szResponseCode, _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);

		inDISP_PutGraphic(_ERR_CLEAR_, 0, _COORDINATE_Y_LINE_8_7_);
		inDISP_BEEP(1, 0);

		inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
		while (1)
		{
			inChoice = inDisTouch_TouchSensor_Click_Slide(_Touch_BATCH_END_);
			uszKey = uszKBD_Key();
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				uszKey = _KEY_TIMEOUT_;
			}

			if (uszKey == _KEY_CANCEL_	||
			    inChoice == _BATCH_END_Touch_ENTER_BUTTON_)
			{
				break;
			}
			else if (uszKey == _KEY_TIMEOUT_)
			{
				break;
			}
			else
			{
				continue;
			}
		}
		/* 為了強調Timeout時間 */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDisTouch_Flush_TouchFile();
	}

	if (srTMS->inTransactionResult == _NCCCTMS_AUTHORIZED_)
	{
                return (VS_SUCCESS);
	}
        else if (srTMS->inTransactionResult == _NCCCTMS_COMM_ERROR_)
        {
                return (VS_ERROR);
        }
        else if (srTMS->inTransactionResult == _NCCCTMS_UNPACK_ERR_ || srTMS->inTransactionResult == _NCCCTMS_PACK_ERR_)
        {
		return (VS_ERROR);
        }
        else if (srTMS->inTransactionResult == _NCCCTMS_CANCELLED_)
        {
		return (VS_ERROR);
        }
	else
	{
		return (VS_ERROR);
	}
}

int inNCCCTMS_SyncHostTerminalDateTime(TMS_OBJECT *srTMS)
{
	return (VS_SUCCESS);
}

int inNCCCTMS_ISO_CheckLRC(TMS_OBJECT *srTMS, unsigned char *szCheckData, int inSizes)
{
	int	i;
	unsigned char ucLRC;

	ucLRC = 0x00;
	for (i = 0; i < inSizes; i ++)
	{
		ucLRC ^= szCheckData[i];
	}

	if (ucLRC != szCheckData[inSizes])
	{
		return (VS_ERROR);
	}
	else
		return (VS_SUCCESS);
}

int inNCCCTMS_ISO_StoreTMSObject(TMS_OBJECT *srTMS, unsigned char *uszUpPack, int inUnPackSizes)
{
        int     inCnt = 0, inTotalPacketCount = 0, inPacketNo = 0;
        char    szTemplate[60 + 1];
        char    szDate[8 + 1];
        char    szTime[6 + 1];

        if (srTMS->inCode == _NCCCTMS_LOGOFF_)
                return (VS_SUCCESS);
        
        if (srTMS->inCode == _NCCCTMS_SCHEDULE_)
        {
                /* Host Date(TMS) */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], &uszUpPack[0], 8);

                /* Host Time(TMS) */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], &uszUpPack[8], 6);
          
                /* DownLoad Flag */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], &uszUpPack[14], 1);
                srTMS->inDownloadFlag = atoi(szTemplate);
                
                /* Download Scope */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], &uszUpPack[15], 1);
                srTMS->inDownlaodScope = atoi(szTemplate);
                
                /* Download TEL */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], &uszUpPack[16], 15);
                
                /* Schedule Date(TMS) */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], &uszUpPack[31], 8);
                
                /* 排程下載 存排程下載日期 */
                if (srTMS->inDownloadFlag == 2)
                        inSetTMSScheduleDate(szTemplate);
                else
                        inSetTMSScheduleDate(szTemplate);
                
                /* Schedule Time(TMS) */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], &uszUpPack[39], 6);
                
                /* 排程下載 存排程下載時間 */
                if (srTMS->inDownloadFlag == 2)
                        inSetTMSScheduleTime(szTemplate);
                else
                        inSetTMSScheduleTime(szTemplate);
        }
        else if (srTMS->inCode == _NCCCTMS_LOGON_)
        {
                /* Host Date (TMS) */
                memset(szDate, 0x00, sizeof(szDate));
                memcpy(&szDate[0], (char *)&uszUpPack[inCnt], 8);
                inCnt += 8;
                /* 更新端末機日期 */
                /* Host Time (TMS) */
                memset(szTime, 0x00, sizeof(szTime));
                memcpy(&szTime[0], (char *)&uszUpPack[inCnt], 6);
                inCnt += 6;
                /* 更新端末機時間 */
                inFunc_SetEDCDateTime(szDate, szTime);

                /* Close Batch Flag */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], (char *)&uszUpPack[inCnt], 1);
                inSetTMSEffectiveCloseBatch(szTemplate);
                inCnt ++;

                /* Effective Date */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], (char *)&uszUpPack[inCnt], 8);
                inSetTMSEffectiveDate(szTemplate);
                inCnt += 8;

                /* Effective Time */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], (char *)&uszUpPack[inCnt], 6);
                inSetTMSEffectiveTime(szTemplate);
                inCnt += 6;

                /* Terminal Packet Size */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], (char *)&uszUpPack[inCnt], 4);
                memcpy(&gsrTMS_Field58.szTerminalPacketSize[0], &szTemplate[0], 4);
                inCnt += 4;

                /* Total Packet Count */
                memset(srTMS->szTotalPacketCount, 0x00, sizeof(srTMS->szTotalPacketCount));
                memcpy(&srTMS->szTotalPacketCount[0], (char *)&uszUpPack[inCnt], 4);
                inCnt += 4;
                inTotalPacketCount = atoi(srTMS->szTotalPacketCount);

                /* 存TotalPacketCount到TMS Record */
                srTMS->inTotalPacketCount = inTotalPacketCount;

                /* Packet No */
                memset(srTMS->szPacketNo, 0x00, sizeof(srTMS->szPacketNo));
                memcpy(&srTMS->szPacketNo[0], (char *)&uszUpPack[inCnt], 4);
                inCnt += 4;
                inPacketNo = atoi(srTMS->szPacketNo);

                /* 存PackNo到TMS Record */
                srTMS->inPackNo = inPacketNo;

                /* 檢查是否加【1】 */
                if (inTotalPacketCount > inPacketNo)
                {
                        memset(srTMS->szPacketNo, 0x00, sizeof(srTMS->szPacketNo));
                        sprintf(srTMS->szPacketNo, "%04d", (inPacketNo + 1));
                }
        }
        else if (srTMS->inCode == _NCCCTMS_RECORD_)
        {
                /* AP/Param Flag */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], (char *)&uszUpPack[inCnt], 1);
                inCnt ++;

                /* File Path Name */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], (char *)&uszUpPack[inCnt], 60);
                inCnt += 60;

                /* Terminal Packet Size */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], (char *)&uszUpPack[inCnt], 4);
                inCnt += 4;

                /* Total Packet Count */
                memset(srTMS->szTotalPacketCount, 0x00, sizeof(srTMS->szTotalPacketCount));
                memcpy(&srTMS->szTotalPacketCount[0], (char *)&uszUpPack[inCnt], 4);
                inCnt += 4;
                inTotalPacketCount = atoi(srTMS->szTotalPacketCount);

                /* 存TotalPacketCount到TMS Record */
                srTMS->inTotalPacketCount = inTotalPacketCount;

                /* Packet No */
                memset(srTMS->szPacketNo, 0x00, sizeof(srTMS->szPacketNo));
                memcpy(&srTMS->szPacketNo[0], (char *)&uszUpPack[inCnt], 4);
                inCnt += 4;
                inPacketNo = atoi(srTMS->szPacketNo);

                /* 存PackNo到TMS Record */
                srTMS->inPackNo = inPacketNo;

                /* 檢查是否加【1】 */
                if (inTotalPacketCount > inPacketNo)
                {
                        memset(srTMS->szPacketNo, 0x00, sizeof(srTMS->szPacketNo));
                        sprintf(srTMS->szPacketNo, "%04d", (inPacketNo + 1));
                }
        }

        if (ginDebug == VS_TRUE)
        {
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szTemplate, "inTotalPacketCount = %d", inTotalPacketCount);
                inLogPrintf(AT, szTemplate);
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szTemplate, "inPacketNo = %d", inPacketNo);
                inLogPrintf(AT, szTemplate);
        }

        /* 判斷是否要繼續下載 */
        if (srTMS->inCode == _NCCCTMS_LOGON_ || srTMS->inCode == _NCCCTMS_RECORD_)
        {
                if (inTotalPacketCount == inPacketNo)
                        srTMS->uszRequest = 'N'; /* 表示是最後一次 */
        }

        return (VS_SUCCESS);
}

/*
Function        :inNCCCTMS_ISO_StoreFTPT
Date&Time       :2017/8/14 下午 3:48
Describe        :儲存FTP ID PW
*/
int inNCCCTMS_ISO_StoreFTPT(TMS_OBJECT *srTMS, unsigned char *uszUpPack, int inUnPackSizes)
{
	int	inCnt = 0, inTotalPacketCount, inPacketNo;
	char	szTemplate[60 + 1];
	char    szDate[8 + 1];
        char    szTime[6 + 1];
	char	szDebugMsg[100 + 1];
	
	if (srTMS->inCode == _NCCCTMS_LOGOFF_)
		return (VS_SUCCESS);

	/* Host Date (TMS) */
	memset(szDate, 0x00, sizeof(szDate));
	memcpy(&szDate[0], (char *)&uszUpPack[inCnt], 8);
	inCnt += 8;
	/* 更新端末機日期 */
	/* Host Time (TMS) */
	memset(szTime, 0x00, sizeof(szTime));
	memcpy(&szTime[0], (char *)&uszUpPack[inCnt], 6);
	inCnt += 6;
	/* 更新端末機時間 */
	inFunc_SetEDCDateTime(szDate, szTime);

	/* Close Batch Flag */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(&szTemplate[0], (char *)&uszUpPack[inCnt], 1);
	inSetTMSEffectiveCloseBatch(szTemplate);
	inCnt ++;

	/* Effective Date */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(&szTemplate[0], (char *)&uszUpPack[inCnt], 8);
	inSetTMSEffectiveDate(szTemplate);
	inCnt += 8;

	/* Effective Time */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(&szTemplate[0], (char *)&uszUpPack[inCnt], 6);
	inSetTMSEffectiveTime(szTemplate);
	inCnt += 6;

	/* Terminal Packet Size */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(&szTemplate[0], (char *)&uszUpPack[inCnt], 4);
	memcpy(&gsrTMS_Field58.szTerminalPacketSize[0], &szTemplate[0], 4);
	inCnt += 4;

	/* Total Packet Count */
	memset(srTMS->szTotalPacketCount, 0x00, sizeof(srTMS->szTotalPacketCount));
	memcpy(&srTMS->szTotalPacketCount[0], (char *)&uszUpPack[inCnt], 4);
	inCnt += 4;
	inTotalPacketCount = atoi(srTMS->szTotalPacketCount);

	/* 存TotalPacketCount到TMS Record */
	srTMS->inTotalPacketCount = inTotalPacketCount;

	/* Packet No */
	memset(srTMS->szPacketNo, 0x00, sizeof(srTMS->szPacketNo));
	memcpy(&srTMS->szPacketNo[0], (char *)&uszUpPack[inCnt], 4);
	inCnt += 4;

	/* I-FES FTP Account */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(&szTemplate[0], (char *)&uszUpPack[inCnt], 8);
	inCnt += 8;
	inSetFTPID(szTemplate);
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "I-FES FTP Account : [%s]", szTemplate);
		inLogPrintf(AT, szDebugMsg);
	}

	/* I-FES FTP Password */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(&szTemplate[0], (char *)&uszUpPack[inCnt], 12);
	inCnt += 12;
	inSetFTPPW(szTemplate);
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "I-FES FTP Password : [%s]", szTemplate);
		inLogPrintf(AT, szDebugMsg);
	}

	if (inSaveTMSFTPRec(0) < 0)
	{
		return (VS_SUCCESS);
	}

	 inPacketNo = atoi(srTMS->szPacketNo);

	/* 存PackNo到TMS Record */
	srTMS->inPackNo = inPacketNo;

	/* 檢查是否加【1】 */
	if (inTotalPacketCount > inPacketNo)
	{
		memset(srTMS->szPacketNo, 0x00, sizeof(srTMS->szPacketNo));
		sprintf(srTMS->szPacketNo, "%04d", (inPacketNo + 1));
	}

	/* 一定是最後一次 */
	srTMS->uszRequest = 'N';

	return (VS_SUCCESS);
}

/*
Function        :vdNCCCTMS_ISO_FormatDebug_DISP
Date&Time       :2016/12/1 上午 10:14
Describe        :
*/
void vdNCCCTMS_ISO_FormatDebug_DISP(TMS_OBJECT *srTMS, unsigned char *uszDebugBuf, int inSendLen)
{
	int			inOneLineLen = 34;
	int			inPrintLineCnt = 0;
	int			i, j, inCnt = 0, inField, inLen, inFieldLen;
	int			inBitMapCnt1, inBitMapCnt2;
	char			szPrintLineData[36 + 1];
	char			szPrtBuf[50], szBuf[3072 + 1], szBitMap[8 + 1];
	unsigned char		uszBitMap[9], uszBitMapDisp[66];
	ISO_TYPE_NCCCTMS_TABLE srISOTypeTable;

	memset((char *)&srISOTypeTable, 0x00, sizeof(srISOTypeTable));
	memcpy((char *)&srISOTypeTable, (char *)&srNCCCTMS_ISOFunc[0], sizeof(srISOTypeTable));

        inLogPrintf(AT, "==========================================");

	if (uszDebugBuf[5 + 1] & 0x10)
	{
                inLogPrintf(AT, "ISO8583 Format  <<Recvive Data>>");
	}
	else
                inLogPrintf(AT, "ISO8583 Format  <<Send Data>>");

	memset(szBuf, 0x00, sizeof(szBuf));
	sprintf(szBuf, "TPDU = [");
        inFunc_BCD_to_ASCII(&szBuf[8], &uszDebugBuf[inCnt], 5);
	strcat(szBuf,"]");
        inLogPrintf(AT, szBuf);
	inCnt += 5;

	memset(szBuf, 0x00, sizeof(szBuf));
	sprintf(szBuf,"MTI  = [");
        inFunc_BCD_to_ASCII(&szBuf[8], &uszDebugBuf[inCnt], 2);
	strcat(szBuf,"]");
        inLogPrintf(AT, szBuf);
	inCnt += 2;

	memset(szBuf, 0x00, sizeof(szBuf));
	memset(szBitMap, 0x00, sizeof(szBitMap));
	memcpy(szBitMap, (char *)&uszDebugBuf[inCnt], 8);
	sprintf(szBuf, "BMap = [%02X %02X %02X %02X %02X %02X %02X %02X]",
						uszDebugBuf[inCnt + 0],
						uszDebugBuf[inCnt + 1],
						uszDebugBuf[inCnt + 2],
						uszDebugBuf[inCnt + 3],
						uszDebugBuf[inCnt + 4],
						uszDebugBuf[inCnt + 5],
						uszDebugBuf[inCnt + 6],
						uszDebugBuf[inCnt + 7]);
        inLogPrintf(AT, szBuf);
	memset(uszBitMapDisp, 0x00, sizeof(uszBitMapDisp));
	memset(uszBitMap, 0x00, sizeof(uszBitMap));
	memcpy(uszBitMap, (char *)&uszDebugBuf[inCnt], 8);

	for (inBitMapCnt1 = 0; inBitMapCnt1 < 8; inBitMapCnt1 ++)
        {
                for (inBitMapCnt2 = 0; inBitMapCnt2 < 8; inBitMapCnt2 ++)
                {
                        if (uszBitMap[inBitMapCnt1] & 0x80)
                                uszBitMapDisp[(inBitMapCnt1*8) + inBitMapCnt2 + 1] = '1';
                        else
                                uszBitMapDisp[(inBitMapCnt1*8) + inBitMapCnt2 + 1] = '0';

                        uszBitMap[inBitMapCnt1] = uszBitMap[inBitMapCnt1] << 1;
            }
        }

        inLogPrintf(AT, "  1234567890   1234567890   1234567890");
	memset(szBuf, 0x00, sizeof(szBuf));
	sprintf(szBuf, "0>%10.10s 1>%10.10s 2>%10.10s", &uszBitMapDisp[1], &uszBitMapDisp[11], &uszBitMapDisp[21]);
        inLogPrintf(AT, szBuf);
        sprintf(szBuf, "3>%10.10s 4>%10.10s 5>%10.10s", &uszBitMapDisp[31], &uszBitMapDisp[41], &uszBitMapDisp[51]);
        inLogPrintf(AT, szBuf);
        sprintf(szBuf, "6>%4.4s", &uszBitMapDisp[61]);
        inLogPrintf(AT, szBuf);

	inCnt += 8;

	for (i = 1; i <= 64; i ++)
	{
		if (!inNCCCTMS_BitMapCheck((unsigned char *)szBitMap, i))
			continue;

		inLen = 0;
		inFieldLen = 0;
		memset(szBuf, 0x00, sizeof(szBuf));
		sprintf(szBuf, "F_%02d ", i);
		inLen = 5;

		inField = inNCCCTMS_ISO_GetFieldIndex(i, srISOTypeTable.srISOFieldType);
		inFieldLen = inNCCCTMS_GetFieldLen(i, &uszDebugBuf[inCnt], srISOTypeTable.srISOFieldType);

		switch (srISOTypeTable.srISOFieldType[inField].inFieldType)
		{
			case _NCCCTMS_ISO_ASC_  :
				memcpy(&szBuf[inLen], &uszDebugBuf[inCnt], inFieldLen);
				break;
			case _NCCCTMS_ISO_NIBBLE_2_  :
			case _NCCCTMS_ISO_BYTE_2_  :
			case _NCCCTMS_ISO_BYTE_3_  :
			case _NCCCTMS_ISO_BYTE_2_H_  :
			case _NCCCTMS_ISO_BYTE_3_H_  :
				strcat(&szBuf[inLen], "[");
                                inLen ++;
                                inFunc_BCD_to_ASCII(&szBuf[inLen], &uszDebugBuf[inCnt ++], 1);
				inLen += 2;
				inFieldLen --;

				if (srISOTypeTable.srISOFieldType[inField].inFieldType == _NCCCTMS_ISO_BYTE_3_ ||
				    srISOTypeTable.srISOFieldType[inField].inFieldType == _NCCCTMS_ISO_BYTE_3_H_)
				{
                                        inFunc_BCD_to_ASCII(&szBuf[inLen], &uszDebugBuf[inCnt ++], 1);
					inLen += 2;
					inFieldLen --;
				}

				strcat(&szBuf[inLen ++], "]");
				
				if (srISOTypeTable.srISOFieldType[inField].uszDispAscii == VS_TRUE)
					memcpy(&szBuf[inLen], (char *)&uszDebugBuf[inCnt], inFieldLen);
				else
					inFunc_BCD_to_ASCII(&szBuf[inLen], &uszDebugBuf[inCnt], inFieldLen);
				
				break;
			case _NCCCTMS_ISO_BCD_  :
				if (srISOTypeTable.srISOFieldType[inField].uszDispAscii == VS_TRUE)
					memcpy(&szBuf[inLen], (char *)&uszDebugBuf[inCnt], inFieldLen);
				else
                                        inFunc_BCD_to_ASCII(&szBuf[inLen], &uszDebugBuf[inCnt], inFieldLen);
				break;
			default :
				break;
		}

		j = 0;

		inPrintLineCnt = 0;
		while ((inPrintLineCnt * inOneLineLen) < strlen(szBuf))
		{
			memset(szPrintLineData, 0x00, sizeof(szPrintLineData));
			memset(szPrtBuf, 0x00, sizeof(szPrtBuf));
			if (((inPrintLineCnt + 1) * inOneLineLen) > strlen(szBuf))
			{
				strcat(szPrintLineData, &szBuf[inPrintLineCnt * inOneLineLen]);
			}
			else
			{
				memcpy(szPrintLineData, &szBuf[inPrintLineCnt * inOneLineLen], inOneLineLen);
			}
			
			sprintf(szPrtBuf, "  %s", szPrintLineData);
			
			inLogPrintf(AT, szPrtBuf);
			inPrintLineCnt ++;
		};

		inCnt += inFieldLen;
	}

        inLogPrintf(AT, "==========================================");
}

/*
Function        :vdNCCCTMS_ISO_FormatDebug_PRINT
Date&Time       :2016/12/1 上午 10:14
Describe        :
*/
void vdNCCCTMS_ISO_FormatDebug_PRINT(TMS_OBJECT *srTMS, unsigned char *uszDebugBuf, int inSendLen)
{
	int			inOneLineLen = 34;
	int			inPrintLineCnt = 0;
	int			i, j, inCnt = 0, inField, inLen, inFieldLen;
	int			inBitMapCnt1, inBitMapCnt2;
	char			szPrtBuf[50], szBuf[1536 + 1], szBitMap[8 + 1];
	char			szPrintLineData[36 + 1];
	unsigned char		uszBitMap[9], uszBitMapDisp[66];
	unsigned char		uszBuffer[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	BufferHandle		srBhandle;
	FONT_ATTRIB		srFont_Attrib;
	ISO_TYPE_NCCCTMS_TABLE	srISOTypeTable;
	
	/* 是否有列印功能 */
	if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
	{
		
	}
	else
	{
	
		/* 初始化 */
		inPRINT_Buffer_Initial(uszBuffer, _BUFFER_MAX_LINE_, &srFont_Attrib, &srBhandle);

		memset((char *)&srISOTypeTable, 0x00, sizeof(srISOTypeTable));
		memcpy((char *)&srISOTypeTable, (char *)&srNCCCTMS_ISOFunc[0], sizeof(srISOTypeTable));

		inPRINT_Buffer_PutIn("==========================================", _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		if (uszDebugBuf[5 + 1] & 0x10)
		{
			inPRINT_Buffer_PutIn("ISO8583 Format  <<Recvive Data>>", _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		else
			inPRINT_Buffer_PutIn("ISO8583 Format  <<Send Data>>", _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szBuf, 0x00, sizeof(szBuf));
		sprintf(szBuf, "TPDU = [");
		inFunc_BCD_to_ASCII(&szBuf[8], &uszDebugBuf[inCnt], 5);
		strcat(szBuf,"]");
		inPRINT_Buffer_PutIn(szBuf, _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inCnt += 5;

		memset(szBuf, 0x00, sizeof(szBuf));
		sprintf(szBuf,"MTI  = [");
		inFunc_BCD_to_ASCII(&szBuf[8], &uszDebugBuf[inCnt], 2);
		strcat(szBuf,"]");
		inPRINT_Buffer_PutIn(szBuf, _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inCnt += 2;

		memset(szBuf, 0x00, sizeof(szBuf));
		memset(szBitMap, 0x00, sizeof(szBitMap));
		memcpy(szBitMap, (char *)&uszDebugBuf[inCnt], 8);
		sprintf(szBuf, "BMap = [%02X %02X %02X %02X %02X %02X %02X %02X]",
							uszDebugBuf[inCnt + 0],
							uszDebugBuf[inCnt + 1],
							uszDebugBuf[inCnt + 2],
							uszDebugBuf[inCnt + 3],
							uszDebugBuf[inCnt + 4],
							uszDebugBuf[inCnt + 5],
							uszDebugBuf[inCnt + 6],
							uszDebugBuf[inCnt + 7]);
		inPRINT_Buffer_PutIn(szBuf, _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		memset(uszBitMapDisp, 0x00, sizeof(uszBitMapDisp));
		memset(uszBitMap, 0x00, sizeof(uszBitMap));
		memcpy(uszBitMap, (char *)&uszDebugBuf[inCnt], 8);

		for (inBitMapCnt1 = 0; inBitMapCnt1 < 8; inBitMapCnt1 ++)
		{
			for (inBitMapCnt2 = 0; inBitMapCnt2 < 8; inBitMapCnt2 ++)
			{
				if (uszBitMap[inBitMapCnt1] & 0x80)
					uszBitMapDisp[(inBitMapCnt1*8) + inBitMapCnt2 + 1] = '1';
				else
					uszBitMapDisp[(inBitMapCnt1*8) + inBitMapCnt2 + 1] = '0';

				uszBitMap[inBitMapCnt1] = uszBitMap[inBitMapCnt1] << 1;
			}
		}

		inPRINT_Buffer_PutIn("  1234567890   1234567890   1234567890", _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		memset(szBuf, 0x00, sizeof(szBuf));
		sprintf(szBuf, "0>%10.10s 1>%10.10s 2>%10.10s", &uszBitMapDisp[1], &uszBitMapDisp[11], &uszBitMapDisp[21]);
		inPRINT_Buffer_PutIn(szBuf, _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		sprintf(szBuf, "3>%10.10s 4>%10.10s 5>%10.10s", &uszBitMapDisp[31], &uszBitMapDisp[41], &uszBitMapDisp[51]);
		inPRINT_Buffer_PutIn(szBuf, _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		sprintf(szBuf, "6>%4.4s", &uszBitMapDisp[61]);
		inPRINT_Buffer_PutIn(szBuf, _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		inCnt += 8;

		for (i = 1; i <= 64; i ++)
		{
			if (!inNCCCTMS_BitMapCheck((unsigned char *)szBitMap, i))
				continue;

			inLen = 0;
			inFieldLen = 0;
			memset(szBuf, 0x00, sizeof(szBuf));
			sprintf(szBuf, "F_%02d ", i);
			inLen = 5;

			inField = inNCCCTMS_ISO_GetFieldIndex(i, srISOTypeTable.srISOFieldType);
			inFieldLen = inNCCCTMS_GetFieldLen(i, &uszDebugBuf[inCnt], srISOTypeTable.srISOFieldType);

			switch (srISOTypeTable.srISOFieldType[inField].inFieldType)
			{
				case _NCCCTMS_ISO_ASC_  :
					memcpy(&szBuf[inLen], &uszDebugBuf[inCnt], inFieldLen);
					break;
				case _NCCCTMS_ISO_NIBBLE_2_  :
				case _NCCCTMS_ISO_BYTE_2_  :
				case _NCCCTMS_ISO_BYTE_3_  :
				case _NCCCTMS_ISO_BYTE_2_H_  :
				case _NCCCTMS_ISO_BYTE_3_H_  :
					strcat(&szBuf[inLen], "[");
					inLen ++;
					inFunc_BCD_to_ASCII(&szBuf[inLen], &uszDebugBuf[inCnt ++], 1);
					inLen += 2;
					inFieldLen --;

					if (srISOTypeTable.srISOFieldType[inField].inFieldType == _NCCCTMS_ISO_BYTE_3_ ||
					    srISOTypeTable.srISOFieldType[inField].inFieldType == _NCCCTMS_ISO_BYTE_3_H_)
					{
						inFunc_BCD_to_ASCII(&szBuf[inLen], &uszDebugBuf[inCnt ++], 1);
						inLen += 2;
						inFieldLen --;
					}

					strcat(&szBuf[inLen ++], "]");
				case _NCCCTMS_ISO_BCD_  :
					if (srISOTypeTable.srISOFieldType[inField].uszDispAscii == VS_TRUE)
						memcpy(&szBuf[inLen], (char *)&uszDebugBuf[inCnt], inFieldLen);
					else
						inFunc_BCD_to_ASCII(&szBuf[inLen], &uszDebugBuf[inCnt], inFieldLen);

					break;
				default :
					break;
			}

			j = 0;

			inPrintLineCnt = 0;
			while ((inPrintLineCnt * inOneLineLen) < strlen(szBuf))
			{
				memset(szPrintLineData, 0x00, sizeof(szPrintLineData));
				memset(szPrtBuf, 0x00, sizeof(szPrtBuf));
				if (((inPrintLineCnt + 1) * inOneLineLen) > strlen(szBuf))
				{
					strcat(szPrintLineData, &szBuf[inPrintLineCnt * inOneLineLen]);
				}
				else
				{
					memcpy(szPrintLineData, &szBuf[inPrintLineCnt * inOneLineLen], inOneLineLen);
				}

				sprintf(szPrtBuf, "  %s", szPrintLineData);

				inPRINT_Buffer_PutIn(szPrtBuf, _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPrintLineCnt ++;
			};

			inCnt += inFieldLen;
		}

		inPRINT_Buffer_PutIn("==========================================", _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		inPRINT_Buffer_OutPut(uszBuffer, &srBhandle);
	}
}

int inNCCCTMS_ISO_GetFieldIndex(int inField, ISO_FIELD_TYPE_NCCCTMS_TABLE *srFieldType)
{
	int 	i;

	for (i = 0 ;; i ++)
	{
		if (srFieldType[i].inFieldNum == 0)
			break;

		if (srFieldType[i].inFieldNum == inField)
			return (i);
	}

	return (VS_ERROR);
}

int inNCCCTMS_CheckFileSize(TMS_OBJECT *srTMS)
{
        int		i = 0, j = 0, inFileNameLen = 0, inSlash = 0;
        long		lnFileSize = 0, lnTMSFLTSize = 0;
        char		szTemplate[60 + 1], szFileName[60 + 1];
        unsigned long   ulFile_Handle;

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
                        inSetTMSFileIndex("Y");
                        inSaveTMSFLTRec(i);
			
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "%s Success", szFileName);
			}
                }
                else
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        inGetTMSFilePathName(szTemplate);

                        /* 記錄下載失敗檔案路徑 */
                        memset(srTMS->szErrFilePathName, 0x00, sizeof(srTMS->szErrFilePathName));
                        memcpy(srTMS->szErrFilePathName, szTemplate, 60);
                        inSetTMSFileIndex("N");
                        inSaveTMSFLTRec(i);
			
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "%s Fail", szFileName);
			}

                        /* 下載檔案大小不符 */
                        strcpy(srTMS->szRespCode, "S6");
                }
        }

        return (VS_SUCCESS);
}

int inNCCCTMS_PrintErrorResult(TMS_OBJECT *srTMS)
{
        char		szPrintBuf[384 + 1];
        char		szTemplate[64 + 1];
	char		szSN[16 + 1];
        RTC_NEXSYS	srRTC; /* Date & Time */

        inPRINT_PutGraphic((unsigned char *)_TMS_DOWNLOAD_STATUS_);

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        /* 列印時間 */
        inPRINT_ChineseFont("", _PRT_HEIGHT_);
        if (inFunc_GetSystemDateAndTime(&srRTC) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
        strcpy(szPrintBuf, "列印時間 : ");
        memset(szTemplate, 0x00, sizeof(szTemplate));
        sprintf(szTemplate, "20%02d/%02d/%02d   ", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
        strcat(szPrintBuf, szTemplate);
        memset(szTemplate, 0x00, sizeof(szTemplate));
        sprintf(szTemplate, "%02d:%02d:%02d", srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
        strcat(szPrintBuf, szTemplate);
        inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);

        /* Get商店代號 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetMerchantID(szTemplate);

        /* 列印商店代號 */
        inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 15, _PADDING_LEFT_);
        sprintf(szPrintBuf, "商店代號 %s", szTemplate);
        inPRINT_ChineseFont(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_);

        /* Get端末機代號 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetTerminalID(szTemplate);

        /* 列印端末機代號 */
        inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 14, _PADDING_LEFT_);
        sprintf(szPrintBuf, "端末機代號%s", szTemplate);
        inPRINT_ChineseFont(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_);

        inPRINT_ChineseFont("================================================", _PRT_HEIGHT_);

        /* Terminal AP Name */
	inPRINT_ChineseFont("", _PRT_HEIGHT_);
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
	inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);
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
	inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);
        /* Terminal S/N */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        /* 取後12碼，但最後一碼為CheckSum，所以取4~15 */
	inFunc_GetSeriaNumber(szTemplate);
	memset(szSN, 0x00, sizeof(szSN));
	memcpy(szSN, &szTemplate[3], 12);
	sprintf(szPrintBuf, "TERMINAL S/N = %s", szSN);
	inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);

        /* Response Code */
        inPRINT_ChineseFont("", _PRT_HEIGHT_);
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "Response Code = [%s]", srTMS->szRespCode);
        inPRINT_ChineseFont(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_);

        inPRINT_PutGraphic((unsigned char *)_TMS_DOWNLOAD_FAILURE_);
        inPRINT_SpaceLine(8);

        return (VS_SUCCESS);
}

int inNCCCTMS_CheckTraceLogFile(TMS_OBJECT *srTMS,unsigned char *uszFileName)
{
        int		inRetVal;
        char		szPacket[1024 + 1];
        RTC_NEXSYS	srRTC; /* Date & Time */
        
        memset(srTMS->uszField61, 0x00, sizeof(srTMS->uszField61));
        memset(szPacket, 0x00, sizeof(szPacket));
        if (inFunc_GetSystemDateAndTime(&srRTC) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
        inRetVal = inFILE_Check_Exist(uszFileName);
        
        if (inRetVal == VS_SUCCESS)
        {
                if (inFILE_Open(&srTMS->ulFile_Handle, (unsigned char *)_TMS_TRACE_LOG_) != VS_SUCCESS)
                {
                        sprintf(szPacket, "20%02d%02d%02d%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay, srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
                        strcat(szPacket, "TRACE_LOG_DATA_ERROR");
                }
                
                srTMS->inTraceTotalSizes = lnFILE_GetSize(&srTMS->ulFile_Handle, (unsigned char *)_TMS_TRACE_LOG_);
                
                if (srTMS->inTraceTotalSizes % 1024 == 0)
                        srTMS->inTotalPacketCount = srTMS->inTraceTotalSizes / 1024;
                else
                        srTMS->inTotalPacketCount = srTMS->inTraceTotalSizes / 1024 + 1;
                
                sprintf(srTMS->szTotalPacketCount, "%04d", srTMS->inPackNo);
                
                if (srTMS->inTraceTotalSizes <= 1024)
                {
                        srTMS->inTraceReadSizes = srTMS->inTraceTotalSizes;
                        
                        if (inFILE_Seek(srTMS->ulFile_Handle, 0, _SEEK_BEGIN_) != VS_SUCCESS)
                        {
                                sprintf(szPacket, "20%02d%02d%02d%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay, srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
                                strcat(szPacket, "TRACE_LOG_DATA_ERROR");
                        }

                        if (inFILE_Read(&srTMS->ulFile_Handle, (unsigned char *)szPacket, srTMS->inTraceReadSizes) != VS_SUCCESS)
                        {
                                sprintf(szPacket, "20%02d%02d%02d%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay, srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
                                strcat(szPacket, "TRACE_LOG_DATA_ERROR");
                        }
                }
                else
                {
                        if (inFILE_Seek(srTMS->ulFile_Handle, srTMS->inTraceSearchIndex, _SEEK_BEGIN_) != VS_SUCCESS)
                        {
                                sprintf(szPacket, "20%02d%02d%02d%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay, srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
                                strcat(szPacket, "TRACE_LOG_DATA_ERROR");
                        }
                        
                        srTMS->inTraceSearchIndex += 1024;
                        
                        if (srTMS->inTraceSearchIndex > srTMS->inTraceTotalSizes)
                        {
                                srTMS->inTraceReadSizes = srTMS->inTraceTotalSizes - 1024 - srTMS->inTraceSearchIndex;
                        }
                        else
                        {
                                srTMS->inTraceReadSizes = 1024;
                        }

                        if (inFILE_Read(&srTMS->ulFile_Handle, (unsigned char *)szPacket, srTMS->inTraceReadSizes) != VS_SUCCESS)
                        {
                                sprintf(szPacket, "20%02d%02d%02d%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay, srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
                                strcat(szPacket, "TRACE_LOG_DATA_ERROR");
                        }                     
                }
                
        }
        else
        {
                strcpy(srTMS->szTotalPacketCount, "0001");
                sprintf(szPacket, "20%02d%02d%02d%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay, srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
                strcat(szPacket, "NO_TRACE_LOG");
        }
        
        memcpy(&srTMS->uszField61, &szPacket[0], 1024);
        
        return (VS_SUCCESS);
}

int inNCCCTMS_FTPS_CheckFileSize(TRANSACTION_OBJECT *pobTran)
{
        int		i = 0;
        long		lnFileSize = 0, lnTMSFLTSize = 0;
        char		szTemplate[60 + 1], szFileName[60 + 1];
	char		szDebugMsg[100 + 1];
        unsigned long   ulFile_Handle;

        for (i = 0;; i++)
        {
                /* Load TMS File List 取得檔名 */
                if (inLoadFTPFLTRec(i) < 0)
                        break;
                
                memset(szFileName, 0x00, sizeof(szFileName));
                inGetFTPFileName(szFileName);
        
                /* 比對下載的檔案與File List提供的檔案大小是否一致 */
                lnFileSize = lnFILE_GetSize(&ulFile_Handle, (unsigned char *)szFileName);
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetFTPFileSize(szTemplate);
                lnTMSFLTSize = atol(szTemplate);
        
                /* 下載成功或失敗結果存在TMSFileIndex */
                if (lnFileSize == lnTMSFLTSize)
                {
                        inSetFTPFileIndex("Y");
                        inSaveFTPFLTRec(i);
                }
                else
                {
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Now size: %ld 不等於 Table size: %ld", lnFileSize, lnTMSFLTSize);
				inLogPrintf(AT, szDebugMsg);
			}
			
                        inSetFTPFileIndex("N");
                        inSaveFTPFLTRec(i);
                }
		inDISP_PutGraphic(_PROCESS_, 0, _COORDINATE_Y_LINE_8_7_);
        }
    
        return (VS_SUCCESS);
}


/*
Function        :inNCCCTMS_DUTY_FREE_PackData
Date&Time       :2022/5/18 下午 5:23
Describe        :
*/
int inNCCCTMS_DUTY_FREE_PackData(TMS_OBJECT *srTMS)
{
	int	i = 0;
	int	inCnt = 0;
	int	inAmtCnt = 0;
        char	szPacket[1024 + 1] = {0};
	char	szTemplate[50 + 1] = {0};
	char	szTemplate2[50 + 1] = {0};
	char	szTableName[20 + 1] = {0};
	char	szTRTFileName[12 + 1] = {0};
	char	szSQL[200 + 1] = {0};
	unsigned char	ucLRC = 0x00;
	unsigned long	ulOrgUnixTime = 0;
	unsigned long	ulReprintUnixTime = 0;
	RTC_NEXSYS	srRTC;
	DUTYFREE_REPRINT_DATA	srReprintData;
	SQLITE_ALL_TABLE	srAll;
	
	inGetTRTFileName(szTRTFileName);
	if (!memcmp(szTRTFileName, _TRT_FILE_NAME_CREDIT_, max(strlen(szTRTFileName), strlen(_TRT_FILE_NAME_CREDIT_))))
	{
		sprintf(szTableName, _TABLE_NAME_REPRINT_NCCC_);
	}
	else if (!memcmp(szTRTFileName, _TRT_FILE_NAME_DCC_, max(strlen(szTRTFileName), strlen(_TRT_FILE_NAME_DCC_))))
	{
		sprintf(szTableName, _TABLE_NAME_REPRINT_DCC_);
	}
	memset(&srReprintData, 0x00, sizeof(DUTYFREE_REPRINT_DATA));
	memset(&srAll, 0x00, sizeof(SQLITE_ALL_TABLE));
	inBatch_Table_Link_Reprint_Data(&srReprintData, &srAll, _LS_READ_);
	sprintf(szSQL, "SELECT * FROM %s WHERE inTableID = (SELECT MAX(inTableID) FROM %s)", szTableName, szTableName);
	inSqlite_Get_Data_By_External_SQL(gszReprintDBPath, &srAll, szSQL);
        
        memset(srTMS->uszField61, 0x00, sizeof(srTMS->uszField61));
        memset(szPacket, 0x00, sizeof(szPacket));
	
	/*
		重印明細資料
		1	「I」代表新格式
		15	特店代號(該筆易的MID，左靠右補空白)
		6	收銀員工號(櫃號前6碼)
		8	端末機代號
		6	批號
		6	調閱編號
		19	卡號(保留前六後四，左靠右補空白)
		10	交易金額(不含小數位，右靠左補空白)
		12	原交易日期時間(YYYYMMDDhhmm)
		12	重印日期時間(YYYYMMDDhhmm)
	*/
	memset(szPacket, 0x00, sizeof(szPacket));
	/* 1	「I」代表新格式 */
		/* TMS下載功能規格V4_4_20140821 因配合昇恆昌要求新格式 2014-08-22 PM 05:02:11 add by TIM */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	strcpy(szTemplate, "I");
	memcpy(&szPacket[inCnt], &szTemplate[0], 1);
	inCnt += 1;
	/* 15	特店代號(該筆易的MID，左靠右補空白) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetMerchantID(szTemplate);
	inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 15, _PADDING_RIGHT_);
	memcpy(&szPacket[inCnt], &szTemplate[0], 15);
	inCnt += 15;
	/* 6	收銀員工號(櫃號前6碼) */
	if (strlen(&srReprintData.szStoreID[10]) > 0)
	{
		/* "STORE ID: %s" */
		memcpy(&szPacket[inCnt], &srReprintData.szStoreID[10], 6);
	}
	else
	{
		memset(szTemplate, ' ', sizeof(szTemplate));
		memcpy(&szPacket[inCnt], &szTemplate[0], 6);
	}
	inCnt += 6;
	/* 8	端末機代號 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTerminalID(szTemplate);
	memcpy(&szPacket[inCnt], &szTemplate[0], 8);
	inCnt += 8;
	/* 6	批號 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetBatchNum(szTemplate);
	sprintf(&szPacket[inCnt], "%s", szTemplate);
	inCnt += 6;
	/* 6	調閱編號 */
	if (!memcmp(srReprintData.szINV_Data, "INV: ", strlen("INV: ")))
	{
		memcpy(&szPacket[inCnt], &srReprintData.szINV_Data[strlen("INV: ")], 6);
	}
	else
	{
		memcpy(&szPacket[inCnt], &srReprintData.szINV_Data[strlen("*INV: ")], 6);
	}
	inCnt += 6;
	/* 19	卡號(保留前六後四，左靠右補空白) */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	strcpy(szTemplate, srReprintData.szPrintPAN);
	inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 19, _PADDING_RIGHT_);
	memcpy(&szPacket[inCnt], &szTemplate[0], 19);
	inCnt += 19;
	/* 10	交易金額(不含小數位，右靠左補空白) */
	/* "NT$..." */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	/* 去除金額中, */
	inAmtCnt = 0;
	for(i = 3; srReprintData.szAmount[i] != 0x00; i++)
	{
		if (srReprintData.szAmount[i] == ',')
		{
			continue;
		}
		else
		{
			szTemplate[inAmtCnt] = srReprintData.szAmount[i];
			inAmtCnt++;
		}
	}
	inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 10, _PADDING_LEFT_);
	memcpy(&szPacket[inCnt], &szTemplate[0], 10);
	inCnt += 10;

	/* 12	原交易日期時間(YYYYMMDDhhmm) */
	/* "DATE: %.4s/%.2s/%.2s" */
	/* "TIME: %.2s:%.2s" */
	memset(&srRTC, 0x00, sizeof(RTC_NEXSYS));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	memcpy(&szTemplate2[0], &srReprintData.szDate[6], 4);
	srRTC.uszYear =  atoi(szTemplate2) - 2000;
	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	memcpy(&szTemplate2[0], &srReprintData.szDate[11], 2);
	srRTC.uszMonth =  atoi(szTemplate2);
	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	memcpy(&szTemplate2[0], &srReprintData.szDate[14], 2);
	srRTC.uszDay =  atoi(szTemplate2);
	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	memcpy(&szTemplate2[0], &srReprintData.szTime[6], 2);
	srRTC.uszHour =  atoi(szTemplate2);
	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	memcpy(&szTemplate2[0], &srReprintData.szTime[9], 2);
	srRTC.uszMinute =  atoi(szTemplate2);
	/* 沒紀錄到秒，直接設為0 */
	srRTC.uszSecond =  0;
	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	inFuncGetTimeToUnix(&srRTC, szTemplate2, 8, 8);
	ulOrgUnixTime = atol(szTemplate2);
	
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memcpy(&szTemplate[0], &srReprintData.szDate[6], 4);
	memcpy(&szTemplate[4], &srReprintData.szDate[11], 2);
	memcpy(&szTemplate[6], &srReprintData.szDate[14], 2);
	memcpy(&szTemplate[8], &srReprintData.szTime[6], 2);
	memcpy(&szTemplate[10], &srReprintData.szTime[9], 2);
	memcpy(&szPacket[inCnt], &szTemplate[0], 12);
	inCnt += 12;
	
	/* 12	重印日期時間(YYYYMMDDhhmm) */
	if (ginISODebug == VS_TRUE)
	{
		inPRINT_ChineseFont("***************重印日期資料***************", _PRT_ISO_);
		memset(szTemplate2, 0x00, sizeof(szTemplate2));
		sprintf(szTemplate2 , "原交易日期[%s]",szTemplate);
		inPRINT_ChineseFont(szTemplate2, _PRT_ISO_);
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(&szTemplate[0], &srReprintData.szReprintDate[6], 4);
		memcpy(&szTemplate[4], &srReprintData.szReprintDate[11], 2);
		memcpy(&szTemplate[6], &srReprintData.szReprintDate[14], 2);
		memcpy(&szTemplate[8], &srReprintData.szReprintTime[6], 2);
		memcpy(&szTemplate[10], &srReprintData.szReprintTime[9], 2);
		memset(szTemplate2, 0x00, sizeof(szTemplate2));
		sprintf(szTemplate2 , "修改前重印交易日期[%s]",szTemplate);
		inPRINT_ChineseFont(szTemplate2, _PRT_ISO_);
	}
	
	memset(&srRTC, 0x00, sizeof(RTC_NEXSYS));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	memcpy(&szTemplate2[0], &srReprintData.szReprintDate[6], 4);
	srRTC.uszYear =  atoi(szTemplate2) - 2000;
	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	memcpy(&szTemplate2[0], &srReprintData.szReprintDate[11], 2);
	srRTC.uszMonth =  atoi(szTemplate2);
	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	memcpy(&szTemplate2[0], &srReprintData.szReprintDate[14], 2);
	srRTC.uszDay =  atoi(szTemplate2);
	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	memcpy(&szTemplate2[0], &srReprintData.szReprintTime[6], 2);
	srRTC.uszHour =  atoi(szTemplate2);
	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	memcpy(&szTemplate2[0], &srReprintData.szReprintTime[9], 2);
	srRTC.uszMinute =  atoi(szTemplate2);
	/* 沒紀錄到秒，直接設為0 */
	srRTC.uszSecond =  0;
	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	inFuncGetTimeToUnix(&srRTC, szTemplate2, 8, 8);
	ulReprintUnixTime = atol(szTemplate2);
	
	if (ulOrgUnixTime > ulReprintUnixTime)
	{
		memset(&srRTC, 0x00, sizeof(RTC_NEXSYS));
		ulReprintUnixTime = ulOrgUnixTime + 60;
		memset(szTemplate2, 0x00, sizeof(szTemplate2));
		sprintf(szTemplate2, "%lu", ulReprintUnixTime);
		inFuncGetUnixTimeToLocalTime(&srRTC, szTemplate2, 8, 8);
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(&szTemplate[0], "20%02d", srRTC.uszYear);
		sprintf(&szTemplate[4], "%02d", srRTC.uszMonth);
		sprintf(&szTemplate[6], "%02d", srRTC.uszDay);
		sprintf(&szTemplate[8], "%02d", srRTC.uszHour);
		sprintf(&szTemplate[10], "%02d", srRTC.uszMinute);
		memcpy(&szPacket[inCnt], &szTemplate[0], 12);
		inCnt += 12;
	}
	else
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(&szTemplate[0], &srReprintData.szReprintDate[6], 4);
		memcpy(&szTemplate[4], &srReprintData.szReprintDate[11], 2);
		memcpy(&szTemplate[6], &srReprintData.szReprintDate[14], 2);
		memcpy(&szTemplate[8], &srReprintData.szReprintTime[6], 2);
		memcpy(&szTemplate[10], &srReprintData.szReprintTime[9], 2);
		memcpy(&szPacket[inCnt], &szTemplate[0], 12);
		inCnt += 12;
	}
	
	if (ginISODebug == VS_TRUE)
	{
		memset(szTemplate2, 0x00, sizeof(szTemplate2));
		sprintf(szTemplate2 , "修改後重印交易日期[%s]",szTemplate);
		inPRINT_ChineseFont(szTemplate2, _PRT_ISO_);
	}
	
	/* 1	LCR */
	ucLRC = 0x00;
	for (i = 0; i < inCnt; i ++)
	{
		ucLRC ^= szPacket[i];
	}
	szPacket[inCnt] = ucLRC;
	inCnt ++;
        
	strcpy(srTMS->szTotalPacketCount, "0001");
	srTMS->inTraceReadSizes = inCnt;
        memcpy(&srTMS->uszField61, &szPacket[0], srTMS->inTraceReadSizes);
        
        return (VS_SUCCESS);
}

/*
Function        :inNCCCTMS_FuncSendReceive_Only_
Date&Time       :2024/2/20 下午 1:54
Describe        :在不動到inNCCCTMS_FuncSendReceive的情形下
*/
int inNCCCTMS_FuncSendReceive_Only_Packet(TRANSACTION_OBJECT *pobTran)
{
        int		inRetVal = VS_ERROR;
        char		szTemplate[16 + 1] = {0};
        TMS_OBJECT	srTMS;

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCCTMS_FuncSendReceive_Only_Packet() START!");

	memset((char *)&srTMS, 0x00, sizeof(TMS_OBJECT));
	srTMS.inCode = pobTran->inTransactionCode;
	srTMS.uszRequest = 'Y'; /* 用來判斷是否要繼續【下載】或【上傳】 */
	srTMS.inTransactionResult = VS_SUCCESS; /* Response Code 使用 */

	srTMS.inListIndex = 0; /* 下載清單使用 */
	srTMS.inListTotalCount = 0; /* 下載清單【TMSFLT.txt】的總筆數 */
	srTMS.inPackNo = 1;

	/* Trace Log */
	srTMS.inTraceTotalSizes = 0;
	srTMS.inTraceSearchIndex = 0;
	srTMS.inTraceReadSizes = 0;

	if (srTMS.inCode == _NCCCTMS_TASK_)
		strcpy(srTMS.szTotalPacketCount, "0001"); /* 不帶【0001】系統會回錯誤訊息 */
	else
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
	inDISP_PutGraphic(_SEND_, 0, _COORDINATE_Y_LINE_8_7_);

	do
	{
		if (srTMS.inCode == _NCCCTMS_TRACE_LOG_)
			inNCCCTMS_CheckTraceLogFile(&srTMS, (unsigned char *)_TMS_TRACE_LOG_);
		else if (srTMS.inCode == _NCCCTMS_DUTYFREE_)
			inNCCCTMS_DUTY_FREE_PackData(&srTMS);

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
	
	/* 不論成功或失敗，都要帶回覆碼回來 */
	strcpy(pobTran->srBRec.szRespCode, srTMS.szRespCode);
	pobTran->inTransactionResult = srTMS.inTransactionResult;

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCCTMS_FuncSendReceive_Only_Packet() END!");

        return (inRetVal);
}