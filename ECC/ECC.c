#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <sqlite3.h>
#include <unistd.h>
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
#include "../SOURCE/FUNCTION/Accum.h"
#include "../SOURCE/FUNCTION/Function.h"
#include "../SOURCE/FUNCTION/FuncTable.h"
#include "../SOURCE/FUNCTION/CFGT.h"
#include "../SOURCE/FUNCTION/CPT.h"
#include "../SOURCE/FUNCTION/EDC.h"
#include "../SOURCE/FUNCTION/HDT.h"
#include "../SOURCE/FUNCTION/HDPT.h"
#include "../SOURCE/FUNCTION/ECR.h"
#include "../SOURCE/FUNCTION/ECCDT.h"
#include "../SOURCE/FUNCTION/TDT.h"
#include "../SOURCE/FUNCTION/File.h"
#include "../SOURCE/FUNCTION/XML.h"
#include "../SOURCE/FUNCTION/Utility.h"
#include "../SOURCE/EVENT/Flow.h"
#include "../SOURCE/EVENT/MenuMsg.h"
#include "../SOURCE/COMM/Comm.h"
#include "../SOURCE/COMM/Ethernet.h"
#include "../CTLS/CTLS.h"
#include "../NCCC/NCCCTicketSrc.h"
#include "../NCCC/NCCCTicketIso.h"
#include "../NCCC/NCCCsrc.h"
#include "../IPASS/IPASSFunc.h"
#include "ICER/stdAfx.h"
#include "ICER/libutil.h"
#include "ECC.h"

extern  int		ginDebug;	/* Debug使用 extern */
extern	int		ginISODebug;
extern	int		ginEngineerDebug;
extern	int		ginAPVersionType;
extern	unsigned long	gulDemoTicketPoint;		/* DEMO用 */
extern	int		ginMacError;
int			ginECC_F57_Len;
int			ginECC_F59_ET_Len = 0;
int			ginECC_F63_Len = 0;
int			ginBeforeIndex = 0;
unsigned char		guszECCRetryBit = VS_FALSE;
ECC_RETRY_DATA		gszECCRetryData;
/*
Function        :inECC_Init_Flow
Date&Time       :2018/4/23 下午 4:03
Describe        :
*/
int inECC_Init_Flow(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_SUCCESS;
	int		inESVCIndex = -1;
	char		szDebugMsg[100 + 1] = {0};
	char		szTemplate[50 + 1] = {0};
	char		szTID[16] = {0};
	char		szAscii[6 + 1] = {0};
	unsigned char	uszECCVersion[3 + 1] = {0};

#if READER_MANUFACTURERS == LINUX_API
	/* 建立資料夾 */
	inFunc_Dir_Make(_ECC_FOLDER_NAME_, _ECC_ROOT_UPPER_PATH_);
#else
	#if defined _NEW_ECC_
		/* 建立資料夾 */
		inFunc_Dir_Make(_ECC_SUB_FOLDER_NAME_, _ECC_ROOT_UPPER_PATH_);
		inFunc_Dir_Make(_ECC_FOLDER_NAME_, _ECC_ROOT_UPPER_PATH_);
	#endif
#endif

	/* 版本確認使用 */
	unsigned char	uszCheckVersionBit = VS_FALSE;
	if (ginAPVersionType == _APVERSION_TYPE_NCCC_)
	{
		if (READER_MANUFACTURERS == NEXSYS_CASTLES)
		{
			#if defined _NEW_ECC_NEXSYS_
				uszCheckVersionBit = VS_TRUE;
			#endif
		}
	}
	
	if (uszCheckVersionBit == VS_FALSE)
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont("悠遊卡版本錯誤", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
		inDISP_ChineseFont("請重新安裝", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
		inDISP_ChineseFont("請按任意鍵", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_CENTER_);
		inDISP_BEEP(1, 0);
		do
		{
			if (uszKBD_Key() != 0x00)
			{
				break;
			}
		}while(1);
	}
		
	/* 初始化 */
	inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_4_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_ChineseFont("悠遊卡初始化", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
	
	/* 確認version */
	ECC_ReadLibVersion_Lib(uszECCVersion);
	if (ginDebug == VS_TRUE)
	{
		memset(szAscii, 0x00, sizeof(szAscii));
		inFunc_BCD_to_ASCII(szAscii, uszECCVersion, 3);
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "ECC Version: %s", szAscii);
		inLogPrintf(AT, szDebugMsg);
	}
	
	/* 聯合版不能reset SAM卡，不然會reset其他票證 */
	if (ginAPVersionType == _APVERSION_TYPE_NCCC_)
	{
		ECC_MSamEnable_Lib(TRUE);
	}
	else
	{
		ECC_MSamEnable_Lib(FALSE);
	}
	
	/* 電子票證 */
	/* T4104 = TID */
	/* TID8碼 */
        /* Load ECC TDT */
	pobTran->srTRec.inTicketType = _TICKET_TYPE_ECC_;
	pobTran->srTRec.inTDTIndex = _TDT_INDEX_01_ECC_;
        if (inLoadTDTRec(pobTran->srTRec.inTDTIndex) != VS_SUCCESS)
		return (VS_ERROR);

	/* 存API Version到TDT */
	inSetTicket_API_Version(API_VERSION_CMAS);
	inSaveTDTRec(_TDT_INDEX_01_ECC_);
	
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTicket_HostIndex(szTemplate);
	inESVCIndex = atoi(szTemplate);
	pobTran->srBRec.inHDTIndex = inESVCIndex;
	
	if (inLoadHDTRec(pobTran->srBRec.inHDTIndex) != VS_SUCCESS)
		return (VS_ERROR);
	if (inLoadHDPTRec(pobTran->srBRec.inHDTIndex) != VS_SUCCESS)
		return (VS_ERROR);
	
        memset(szTID, 0x00, sizeof(szTID));
	inGetTerminalID(szTID);
        ECC_SetReaderID_Lib((unsigned char *)szTID);
	
	/* 初始化是否已重試 */
	guszECCRetryBit = VS_FALSE;
	
        /* 產生參數檔 */
        inECC_PACK_ICERINI(pobTran);
        
        /* 組電文(生成ICERAPI.REQ) */
	inECC_PACK_Init_ICERAPI(pobTran);
	
	/* AdditionalTcpipData */
        inECC_PACK_Init_ICERAPI_NCCC(pobTran);

	/* 票證沒撥接，自行防呆 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetCommMode(szTemplate);
	if (memcmp(szTemplate, _COMM_MODEM_MODE_, strlen(_COMM_MODEM_MODE_)) == 0)
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont("此交易不支援撥接", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
		inDISP_Wait(2000);
		
		inRetVal = VS_ERROR;
	}
	else
	{
		/* CMAS_API_REQ_CHECK 要有 */
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "ICER Start");
			inLogPrintf(AT, szDebugMsg);
		}

		inRetVal = ICERApi_exe();

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "ICER End");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	inNCCC_TICKET_SetSTAN(pobTran);

	if (inRetVal != VS_SUCCESS)
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_);
		inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_4_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	        inDISP_ChineseFont("認證失敗", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_); 
	        inDISP_BEEP(3, 1000);
		
	        inECC_UNPACK_ICERAPI(pobTran, _ECC_API_RES_FILE_);
		return (VS_ERROR);
	}
        else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_);
		inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_4_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);
		
		inECC_UNPACK_ICERAPI(pobTran, _ECC_API_RES_FILE_);
		inLoadTDTRec(_TDT_INDEX_01_ECC_);
		inSetTicket_LogOnOK("Y");
		inSaveTDTRec(_TDT_INDEX_01_ECC_);
		
		return (VS_SUCCESS);
	}
}

/*
Function        :inECC_PACK_ICERINI
Date&Time       :2018/3/21 下午 4:05
Describe        :
*/
int inECC_PACK_ICERINI(TRANSACTION_OBJECT *pobTran)
{
	char		szHostIndex[2 + 1]= {0};
	char		szCommunicationIndex[2 + 1] = {0};
	char		szHostIP[16 + 1] = {0};
	char		szPort[5 + 1] = {0};
	char		szECC_SP_ID[8 + 1] = {0};
	char		szSAMSlot[2 + 1] = {0};
	char		szIFESMode[2 + 1] = {0};
	char		szTemplate[100 + 1] = {0};
	char		szFuncEnable[2 + 1] = {0};
	char		szCFESMode[2 + 1] = {0};
	char		szNCCCMode[2 + 1] = {0};
	xmlDocPtr       srDoc = NULL;
        xmlNodePtr      srRoot_Node = NULL;
	
        if (inLoadTDTRec(_TDT_INDEX_01_ECC_) < 0)
                return (VS_ERROR);

        /* MFT index 紀錄 HDT index */
	memset(szHostIndex, 0x00, sizeof(szHostIndex));
	inGetTicket_HostIndex(szHostIndex);
	pobTran->srBRec.inHDTIndex = atoi(szHostIndex);
	
        if (inLoadHDTRec(pobTran->srBRec.inHDTIndex) != VS_SUCCESS)
                return (VS_ERROR);
	if (inLoadHDPTRec(pobTran->srBRec.inHDTIndex) != VS_SUCCESS)
                return (VS_ERROR);
	
        if (inLoadECCDTRec(0) != VS_SUCCESS)
                return (VS_ERROR);             
        
        /* 刪除原本的ICERINI */
#if	READER_MANUFACTURERS == LINUX_API
	inFile_Unlink_File(_ECC_PARA_FILE_, "");
#else
	inFile_Unlink_File(_ECC_PARA_FILE_, _ECC_FOLDER_PATH_);
#endif
	
	/* 建立一個新的檔案 */
        srDoc = xmlNewDoc(BAD_CAST "1.0");
        srRoot_Node = xmlNewNode(NULL, BAD_CAST "TransXML");
        xmlDocSetRootElement (srDoc, srRoot_Node);
	
	/* LogFlag */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "LogFlag", BAD_CAST "1");
	
	/* DLLVersion */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "DLLVersion", BAD_CAST "2");
	
	/* LogCnt */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "LogCnt", BAD_CAST "30");
	
	/* ComPort */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "ComPort", BAD_CAST "1");
	
	/* ICER_IP */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "ICER_IP", BAD_CAST "172.25.17.95");
	
	/* ICER_Port */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "ICER_Port", BAD_CAST "8303");
	
	/* ECC_IP */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "ECC_IP", BAD_CAST "172.16.11.20");
	
	/* ECC_Port */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "ECC_Port", BAD_CAST "8902");
	
	/* CMAS_IP */
	memset(szCommunicationIndex, 0x00, sizeof(szCommunicationIndex));
	inGetCommunicationIndex(szCommunicationIndex);
	inLoadCPTRec(atoi(szCommunicationIndex) - 1);
	memset(szHostIP, 0x00, sizeof(szHostIP));
	inGetHostIPPrimary(szHostIP);
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "CMAS_IP", BAD_CAST szHostIP);
	
	/* CMAS_Port */
	memset(szPort, 0x00, sizeof(szPort));
	inGetHostPortNoPrimary(szPort);
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "CMAS_Port", BAD_CAST szPort);
	
	/* TCPIPTimeOut */
	/* 【需求單-108265】修改電票交易timeout時間需求 by Russell 2020/6/4 下午 2:02 */
	/* READER_MANUFACTURERS==LINUX_API 悠遊卡timeout已經harcode成10秒，改xml沒用 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%d", _NCCC_TICKET_COMM_TIMEOUT_);
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "TCPIPTimeOut", BAD_CAST szTemplate);
	
	/* TMLocationID */
	memset(szECC_SP_ID, 0x00, sizeof(szECC_SP_ID));
	inGetECC_New_SP_ID(szECC_SP_ID);
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "TMLocationID", BAD_CAST szECC_SP_ID);
	
	/* TMID */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "TMID", BAD_CAST "01");
	
	/* TMAgentNumber */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "TMAgentNumber", BAD_CAST "0001");
	
	/* LocationID */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "LocationID", BAD_CAST "0");
	
	/* NewLocationID */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "NewLocationID", BAD_CAST "0");
	
	/* SPID */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "SPID", BAD_CAST "0");
	
	/* NewSPID */
	memset(szECC_SP_ID, 0x00, sizeof(szECC_SP_ID));
	inGetECC_New_SP_ID(szECC_SP_ID);
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "NewSPID", BAD_CAST szECC_SP_ID);
	
	/* Slot */
	memset(szSAMSlot, 0x00, sizeof(szSAMSlot));
	inGetTicket_SAM_Slot(szSAMSlot);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	if (memcmp(szSAMSlot, "01", strlen("01")) == 0)
	{
		sprintf(szTemplate, "11");
	}
	else if (memcmp(szSAMSlot, "02", strlen("02")) == 0)
	{
		sprintf(szTemplate, "22");
	}
	else if (memcmp(szSAMSlot, "03", strlen("03")) == 0)
	{
		sprintf(szTemplate, "33");
	}
	else if (memcmp(szSAMSlot, "04", strlen("04")) == 0)
	{
		sprintf(szTemplate, "44");
	}
	else
	{
		sprintf(szTemplate, "11");
	}
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "Slot", BAD_CAST szTemplate);
	
	/* BaudRate */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "BaudRate", BAD_CAST "115200");
	
	/* OpenCom */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "OpenCom", BAD_CAST "1");
	
	/* MustSettleDate */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "MustSettleDate", BAD_CAST "10");
	
	/* ReaderMode */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "ReaderMode", BAD_CAST "2");
	
	/* DLLMode */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "DLLMode", BAD_CAST "0");
	
	/* BatchFlag(批次檔案參數):0=EZHOST / 1=CMAS / 2=SIS2 */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "BatchFlag", BAD_CAST "1");
	
	/* OnlineFlag */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "OnlineFlag", BAD_CAST "1");
	
	/* ICERDataFlag */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "ICERDataFlag", BAD_CAST "1");
	
	/* MessageHeader */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "MessageHeader", BAD_CAST "99909020");
	
	/* AutoLoadMode */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "AutoLoadMode", BAD_CAST "2");
	
	/* MaxALAmt */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "MaxALAmt", BAD_CAST "1500");
	
	/* Dev_Info */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "Dev_Info", BAD_CAST "1122334455");
		
	/* TCPIP_SSL */
	memset(szIFESMode, 0x00, sizeof(szIFESMode));
	inGetI_FES_Mode(szIFESMode);
	memset(szCFESMode, 0x00, sizeof(szCFESMode));
	inGetCloud_MFES(szCFESMode);
	memset(szNCCCMode, 0x00, sizeof(szNCCCMode));
	inGetNCCCFESMode(szNCCCMode);
	memset(szTemplate, 0x00, sizeof(szIFESMode));
	if (memcmp(szIFESMode, "Y", strlen("Y")) == 0	||
	   (memcmp(szCFESMode, "Y", strlen("Y")) == 0	&&
	    memcmp(szNCCCMode, _NCCC_03_MFES_MODE_, strlen(_NCCC_03_MFES_MODE_)) == 0))
	{
		strcpy(szTemplate, "1");
	}
	else
	{
		strcpy(szTemplate, "0");
	}
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "TCPIP_SSL", BAD_CAST szTemplate);
	
	/* CMASAdviceVerify */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "CMASAdviceVerify", BAD_CAST "0");
	
	/* AutoSignOnPercnet */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "AutoSignOnPercnet", BAD_CAST "0");
	
	/* AutoLoadFunction */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "AutoLoadFunction", BAD_CAST "0");
	
	/* VerificationCode */
	/* ICER內沒似乎沒抓這個值，2020/3/9 下午 12:02 */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "VerificationCode", BAD_CAST "0");
	
	/* ReSendReaderAVR */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "ReSendReaderAVR", BAD_CAST "0");
	
	/* XMLHeaderFlag */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "XMLHeaderFlag", BAD_CAST "1");
	
	/* FolderCreatFlag */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "FolderCreatFlag", BAD_CAST "1");
	
	/* <BLCName> : 黑名單 */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "BLCName", BAD_CAST "BLC01095BIG");
	
	/* CMASMode */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "CMASMode", BAD_CAST "1");
	
	/* POS_ID */
	/* (需求單 - 107276)自助交易標準做法  	悠遊卡交易時，悠遊卡POS ID帶入POS櫃號數值 by Russell 2019/1/21 下午 2:02 */
	/* 【需求單-108240】EDC修改電票需求 by Russell 2020/6/4 下午 2:02 */
	memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
	inGetStoreIDEnable(szFuncEnable);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	if (pobTran->uszECRBit == VS_TRUE		&&
	    !memcmp(szFuncEnable, "Y", strlen("Y"))	&&
	    inFunc_CheckFullSpace(pobTran->srBRec.szStoreID) != VS_TRUE)
	{
		/* 櫃號如沒輸入會直接在輸入那一隻產生18個空白 */
		/* 避免因特殊字元無法輸入，要先檢核轉換 */
		inXML_Escape_Character_Transform(pobTran->srBRec.szStoreID, strlen(pobTran->srBRec.szStoreID), szTemplate);
	}
	else
	{
		inGetECC_POS_ID(szTemplate);
	}
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "POS_ID", BAD_CAST szTemplate);
	
	/* PacketLenFlag */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "PacketLenFlag", BAD_CAST "1");
	
	/* AdditionalTcpipData */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "AdditionalTcpipData", BAD_CAST "1");
	
	/* CRT_FileName */
	/* NCCC不驗所以填空值，若要驗憑證應該要填 */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "CRT_FileName", BAD_CAST " ");
	
	/* Key_FileName */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "Key_FileName", BAD_CAST " ");
	
	/* ICERFlowDebug */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "ICERFlowDebug", BAD_CAST "0");
	
	/* ReaderUartDebug 悠遊卡測FW使用 */
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "ReaderUartDebug", BAD_CAST "0");
	
	xmlNewChild(srRoot_Node, NULL, BAD_CAST "SetCardValidDate", BAD_CAST "0");
	/* 儲存xml檔 xml檔現在會自動換行了 */
        inXML_SaveFile(_ECC_PARA_FILE_, &srDoc, "utf-8", 1);
#if	READER_MANUFACTURERS == LINUX_API
	/* 參數檔放根目錄 */
#else
	inFile_Move_File(_ECC_PARA_FILE_, _AP_ROOT_PATH_, "", _ECC_FOLDER_PATH_);
#endif	
	/* 清空佔用記憶體 */
	inXML_End(&srDoc);
	
	return (VS_SUCCESS);
}

/*
Function        :inECC_PACK_Init_ICERAPI
Date&Time       :2018/3/22 上午 11:00
Describe        :
*/
int inECC_PACK_Init_ICERAPI(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_ERROR;
	char		szHostIndex[2 + 1]= {0};
	char		szSTAN[6 + 1] = {0};
	char		szInvoiceNum[6 + 1] = {0};
	char		szTemplate[20 + 1] = {0};
	RTC_NEXSYS	srRTC = {0};
        xmlDocPtr       srDoc = NULL;
        xmlNodePtr      srRoot_Node = NULL, srTRANS_Node = NULL;
#if 	READER_MANUFACTURERS == LINUX_API
	unsigned long	ulHandle = 0;
#else
	int		inFd = 0;
#endif
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inECC_PACK_Init_ICERAPI() START !");
	}
        
        if (inLoadTDTRec(_TDT_INDEX_01_ECC_) < 0)
                return (VS_ERROR);
        
	/* MFT index 紀錄 HDT index */
	memset(szHostIndex, 0x00, sizeof(szHostIndex));
	inGetTicket_HostIndex(szHostIndex);
	pobTran->srBRec.inHDTIndex = atoi(szHostIndex);
	
        if (inLoadHDTRec(pobTran->srBRec.inHDTIndex) != VS_SUCCESS)
                return (VS_ERROR);
	if (inLoadHDPTRec(pobTran->srBRec.inHDTIndex) != VS_SUCCESS)
                return (VS_ERROR);
	   
         if (inLoadECCDTRec(0) != VS_SUCCESS)
                return (VS_ERROR);         
               
        /* 刪除原本的ICERAPI */
        inFile_Unlink_File(_ECC_API_REQ_CHECK_, _ECC_FOLDER_PATH_);    /* 檢核用，有表示組ICERAPI成功 */
	inFile_Unlink_File(_ECC_API_REQ_FILE_, _ECC_FOLDER_PATH_);     /* 悠遊卡自用，交易資料，每次都要重組 */
	inFile_Unlink_File(_ECC_API_REQ_FILE2_, _ECC_FOLDER_PATH_);    /* NCCC用，每次重組，但裡面Logon不用 */
	inFile_Unlink_File(_ECC_API_REQ_TEMP_, _ECC_FOLDER_PATH_);     /* Logon用，因為CMAS_API_REQ_FILE2交易都要組，所以改寫到暫存檔 */
	
	inFile_Unlink_File(_ECC_API_RES_FILE_, _ECC_FOLDER_PATH_);     /* 防止用到上次資料結果 */
	/* Retry時需要參考RES2，不能砍 */
	if (guszECCRetryBit == VS_TRUE)
	{
		
	}
	else
	{
		inFile_Unlink_File(_ECC_API_RES_FILE2_, _ECC_FOLDER_PATH_);
	}
	
	/* 建立一個新的檔案 */
        srDoc = xmlNewDoc(BAD_CAST "1.0");
        srRoot_Node = xmlNewNode(NULL, BAD_CAST "TransXML");
        xmlDocSetRootElement (srDoc, srRoot_Node);
 
        /* <TRANS> */
	srTRANS_Node = xmlNewNode(NULL, BAD_CAST "TRANS");
	xmlAddChild(srRoot_Node, srTRANS_Node);

        /* <T0100> */
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T0100", BAD_CAST "0800");

        /* <T0300> */
        xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T0300", BAD_CAST "881999");
        
        /* <T1100> : STAN 不知道用在哪，先比照NCCC STAN */
	memset(szSTAN, 0x00, sizeof(szSTAN));
	inGetSTANNum(szSTAN);
        xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1100", BAD_CAST szSTAN);
        
        /* <T1101> : CMAS自用，區分上筆交易(會回傳) */
	memset(szSTAN, 0x00, sizeof(szSTAN));
	inGetTicket_STAN(szSTAN);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%06d", atoi(szSTAN) + 1);
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1101", BAD_CAST szTemplate);
	inSetTicket_STAN(szTemplate);
	inSaveTDTRec(_TDT_INDEX_01_ECC_);

        /* <T1102> : NCCC STAN */
        memset(szSTAN, 0x00, sizeof(szSTAN));
	inGetSTANNum(szSTAN);
        xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1102", BAD_CAST szSTAN);
        
        /* <T1103> : NCCC RRN(後三碼) */
	memset(szInvoiceNum, 0x00, sizeof(szInvoiceNum));
	inGetTicket_InvNum(szInvoiceNum);
        xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1103", BAD_CAST szInvoiceNum);

        /* <T1200> : hhmmss */
	memset(&srRTC, 0x00, sizeof(srRTC));
	inFunc_GetSystemDateAndTime(&srRTC);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%02d%02d%02d", srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1200", BAD_CAST szTemplate);
        
	/* <T1300> : yyyymmdd */
	/* hhmmss就抓過一次時間了，不用重抓 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "20%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1300", BAD_CAST szTemplate);
    
	/* 儲存xml檔 xml檔現在會自動換行了 */
        inXML_SaveFile(_ECC_API_REQ_FILE_, &srDoc, "utf-8", 1);
	inFile_Move_File(_ECC_API_REQ_FILE_, _AP_ROOT_PATH_, "", _ECC_FOLDER_PATH_);
	
	/* 清空佔用記憶體 */
	inXML_End(&srDoc);
	
	/* 步驟完成，建立檢核擋 */
#if 	READER_MANUFACTURERS == LINUX_API
	inRetVal = inFILE_Create(&ulHandle, (unsigned char*)_ECC_API_REQ_CHECK_);
	/* 不管成功或失敗都要關檔 */
	inFILE_Close(&ulHandle);
	/* 抓到ICERData資料夾 */
	inFile_Move_File(_ECC_API_REQ_CHECK_, _FS_DATA_PATH_, "", _ECC_FOLDER_PATH_);
#else
	inRetVal = inFile_Linux_Create(&inFd, _ECC_API_REQ_CHECK_);
	/* 不管成功或失敗都要關檔 */
	inFile_Linux_Close(inFd);
	/* 會有權限問題，要先改權限 */
	inFunc_Data_Chmod("u+rwx", _ECC_API_REQ_CHECK_, _AP_ROOT_PATH_);
	/* 抓到ICERData資料夾 */
	inFile_Move_File(_ECC_API_REQ_CHECK_, _AP_ROOT_PATH_, "", _ECC_FOLDER_PATH_);
#endif

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inECC_PACK_Init_ICERAPI() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inECC_PACK_Init_ICERAPI_NCCC
Date&Time       :2018/3/22 下午 1:56
Describe        :
*/
int inECC_PACK_Init_ICERAPI_NCCC(TRANSACTION_OBJECT *pobTran)
{
	int		inDataLen = 0;
	int		inTxnCode = _TRANS_TYPE_NULL_;
	int		inSendLen = 0;
	int		inField32 = 0;
	char		szFESMode[2 + 1] = {0};
	char		szIFESMode[2 + 1] = {0};
	char		szSendPacket[_NCCC_TICKET_ISO_SEND_ + 1];
	char		szTemplate[500 + 1] = {0};
	xmlDocPtr       srDoc = NULL;
        xmlNodePtr      srRoot_Node = NULL, sr881999_Node = NULL, sr881999adv_Node = NULL, sr882999_Node = NULL, sr882999rev_Node = NULL,  sr596100adv_Node = NULL;
	
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inECC_PACK_Init_ICERAPI_NCCC() START !");
	}
        
	memset(szFESMode, 0x00, sizeof(szFESMode));
	inGetNCCCFESMode(szFESMode);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetCloud_MFES(szTemplate);
	memset(szIFESMode, 0x00, sizeof(szIFESMode));
	inGetI_FES_Mode(szIFESMode);
	
        if (memcmp(szFESMode, _NCCC_05_ATS_MODE_, 2) == 0	||
	    memcmp(szFESMode, _NCCC_04_MPAS_MODE_, 2) == 0	||
	   (memcmp(szFESMode, _NCCC_03_MFES_MODE_, 2) == 0	&&
	    memcmp(szTemplate, "Y", strlen("Y")) == 0))
	{
                inField32 = _ECC_F32_LENTH_;
	}

        pobTran->srTRec.inTicketType = _TICKET_TYPE_ECC_;
        inNCCC_Ticket_Get_ParamValue(pobTran);
       
        memset(szSendPacket, 0x00, sizeof(szSendPacket));
        inTxnCode = _TICKET_EASYCARD_LOGON_;
        pobTran->inISOTxnCode = _TICKET_EASYCARD_LOGON_;
        
	/* 要Load CPT */
	inETHERNET_Begin(pobTran);
	
        /* 組 ISO 電文 */
	ginBeforeIndex = 0;
	if ((inSendLen = inNCCC_TICKET_PackISO(pobTran, (unsigned char *)szSendPacket, inTxnCode)) <= 0)
	{
		/* 上傳不能Show錯誤訊息 */
		pobTran->inErrorMsg = _ERROR_CODE_V3_NONE_;
		
		return (VS_ISO_PACK_ERR);
	}
	
	/* 沒找到 */
	if (ginBeforeIndex == 0)
	{
	        return (VS_ERROR);
	}

        /* 刪除原本的ICERAPI */
	inFile_Unlink_File(_ECC_API_REQ_FILE2_, _ECC_FOLDER_PATH_);
	inFile_Unlink_File(_ECC_API_REQ_TEMP_, _ECC_FOLDER_PATH_);
	
	/* 建立一個新的檔案 */
        srDoc = xmlNewDoc(BAD_CAST "1.0");
        srRoot_Node = xmlNewNode(NULL, BAD_CAST "TransXML");
        xmlDocSetRootElement (srDoc, srRoot_Node);
 
        /* <881999> */
	sr881999_Node = xmlNewNode(NULL, BAD_CAST "881999");
	xmlAddChild(srRoot_Node, sr881999_Node);
        
        /* <BefData> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%s", "0000");
	inFunc_BCD_to_ASCII(&szTemplate[4], (unsigned char*)&szSendPacket[0], ginBeforeIndex);
        xmlNewChild(sr881999_Node, NULL, BAD_CAST "BefData", BAD_CAST szTemplate);
             
        /* <AftData> : 扣掉取代的Len + @@@@@ */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inFunc_BCD_to_ASCII(&szTemplate[0], (unsigned char*)&szSendPacket[ginBeforeIndex + 7], inSendLen - ginBeforeIndex - 7);
        xmlNewChild(sr881999_Node, NULL, BAD_CAST "AftData", BAD_CAST szTemplate);

        /* <F11Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr881999_Node, NULL, BAD_CAST "F11Adr", BAD_CAST szTemplate);
         
        /* <F37Adr> */
	/* 抓後三碼，要指到我們用的第9 10 11碼 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F11_LENTH_ + _ECC_F24_LENTH_ + _ECC_F25_LENTH_ + inField32 + 8;;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr881999_Node, NULL, BAD_CAST "F37Adr", BAD_CAST szTemplate);
        
	memset(szFESMode, 0x00, sizeof(szFESMode));
	inGetNCCCFESMode(szFESMode);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetCloud_MFES(szTemplate);
        if (memcmp(szFESMode, _NCCC_05_ATS_MODE_, 2) == 0	||
	    memcmp(szFESMode, _NCCC_04_MPAS_MODE_, 2) == 0	||
	   (memcmp(szFESMode, _NCCC_03_MFES_MODE_, 2) == 0	&&
	    memcmp(szTemplate, "Y", strlen("Y")) == 0))
	{
		/* <F39Adr> */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + _ECC_F24_LENTH_ + 
			    inField32 + _ECC_F37_LENTH_ + _ECC_F38_LENTH_;
		sprintf(szTemplate, "%d", inDataLen);
		xmlNewChild(sr881999_Node, NULL, BAD_CAST "F39Adr", BAD_CAST szTemplate);

		/* <ECCAdr> */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + _ECC_F24_LENTH_ + 
			    inField32 + _ECC_F37_LENTH_ + _ECC_F38_LENTH_ + _ECC_F39_LENTH_ + _ECC_F41_LENTH_ + _ECC_F56_LENTH_;
		sprintf(szTemplate, "%d", inDataLen);
		xmlNewChild(sr881999_Node, NULL, BAD_CAST "ECCAdr", BAD_CAST szTemplate);
	}
	else
	{
		/* <F39Adr> */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + _ECC_F24_LENTH_;
		sprintf(szTemplate, "%d", inDataLen);
		xmlNewChild(sr881999_Node, NULL, BAD_CAST "F39Adr", BAD_CAST szTemplate);

		/* <ECCAdr> */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + _ECC_F24_LENTH_ + 
			    _ECC_F39_LENTH_ + _ECC_F41_LENTH_ + _ECC_F56_LENTH_;
		sprintf(szTemplate, "%d", inDataLen);
		xmlNewChild(sr881999_Node, NULL, BAD_CAST "ECCAdr", BAD_CAST szTemplate);
	}
        /* ======================================================================================= */
	/* <881999_Adv> */
	sr881999adv_Node = xmlNewNode(NULL, BAD_CAST "881999_Adv");
	xmlAddChild(srRoot_Node, sr881999adv_Node);

        /* <BefData> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%s", "0000");
	inFunc_BCD_to_ASCII(&szTemplate[4], (unsigned char*)&szSendPacket[0], ginBeforeIndex);
	szTemplate[37] = 0x31;      /* Advice修改Pcode */
        xmlNewChild(sr881999adv_Node, NULL, BAD_CAST "BefData", BAD_CAST szTemplate);
        
        /* <AftData> : 扣掉取代的Len + @@@@@ */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inFunc_BCD_to_ASCII(&szTemplate[0], (unsigned char*)&szSendPacket[ginBeforeIndex + 7], inSendLen - ginBeforeIndex - 7);
        xmlNewChild(sr881999adv_Node, NULL, BAD_CAST "AftData", BAD_CAST szTemplate);
        
        /* <F11Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr881999adv_Node, NULL, BAD_CAST "F11Adr", BAD_CAST szTemplate);
         
        /* <F37Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F11_LENTH_ + _ECC_F24_LENTH_ + _ECC_F25_LENTH_ + inField32 + 8;;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr881999adv_Node, NULL, BAD_CAST "F37Adr", BAD_CAST szTemplate);
	
	memset(szFESMode, 0x00, sizeof(szFESMode));
	inGetNCCCFESMode(szFESMode);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetCloud_MFES(szTemplate);
        if (memcmp(szFESMode, _NCCC_05_ATS_MODE_, 2) == 0	||
	    memcmp(szFESMode, _NCCC_04_MPAS_MODE_, 2) == 0	||
	   (memcmp(szFESMode, _NCCC_03_MFES_MODE_, 2) == 0	&&
	    memcmp(szTemplate, "Y", strlen("Y")) == 0))
	{
		/* <F39Adr> */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + _ECC_F24_LENTH_ + 
			    inField32 + _ECC_F37_LENTH_ + _ECC_F38_LENTH_;
		sprintf(szTemplate, "%d", inDataLen);
		xmlNewChild(sr881999adv_Node, NULL, BAD_CAST "F39Adr", BAD_CAST szTemplate);

		/* <ECCAdr> */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + _ECC_F24_LENTH_ + 
			    inField32 + _ECC_F37_LENTH_ + _ECC_F38_LENTH_ + _ECC_F39_LENTH_ + _ECC_F41_LENTH_ + _ECC_F56_LENTH_;
		sprintf(szTemplate, "%d", inDataLen);
		xmlNewChild(sr881999adv_Node, NULL, BAD_CAST "ECCAdr", BAD_CAST szTemplate);
	}
	else
	{
		/* <F39Adr> */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + _ECC_F24_LENTH_;
		sprintf(szTemplate, "%d", inDataLen);
		xmlNewChild(sr881999adv_Node, NULL, BAD_CAST "F39Adr", BAD_CAST szTemplate);

		/* <ECCAdr> */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + _ECC_F24_LENTH_ + 
			    _ECC_F39_LENTH_ + _ECC_F41_LENTH_ + _ECC_F56_LENTH_;
		sprintf(szTemplate, "%d", inDataLen);
		xmlNewChild(sr881999adv_Node, NULL, BAD_CAST "ECCAdr", BAD_CAST szTemplate);
	}
        
        /* ======================================================================================= */
        memset(szSendPacket, 0x00, sizeof(szSendPacket));
        inTxnCode = _TICKET_EASYCARD_REGISTER_;
        pobTran->inISOTxnCode = _TICKET_EASYCARD_REGISTER_;
  
        /* 組 ISO 電文 */
	ginBeforeIndex = 0;
	if ((inSendLen = inNCCC_TICKET_PackISO(pobTran, (unsigned char *)szSendPacket, inTxnCode)) <= 0)
	{
		/* 清空佔用記憶體 */
		inXML_End(&srDoc);
		
		/* 上傳不能Show錯誤訊息 */
		pobTran->inErrorMsg = _ERROR_CODE_V3_NONE_;
		
		return (VS_ISO_PACK_ERR);
	}
	
	/* 沒找到 */
	if (ginBeforeIndex == 0)
	{
		/* 清空佔用記憶體 */
		inXML_End(&srDoc);
		
	        return (VS_ERROR);
	}

        /* <882999> */
	sr882999_Node = xmlNewNode(NULL, BAD_CAST "882999");
	xmlAddChild(srRoot_Node, sr882999_Node);
        
        /* <BefData> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%s", "0000");
	inFunc_BCD_to_ASCII(&szTemplate[4], (unsigned char*)&szSendPacket[0], ginBeforeIndex);
        xmlNewChild(sr882999_Node, NULL, BAD_CAST "BefData", BAD_CAST szTemplate);
       
        /* <AftData> : 扣掉取代的Len + @@@@@ */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inFunc_BCD_to_ASCII(&szTemplate[0], (unsigned char*)&szSendPacket[ginBeforeIndex + 7], inSendLen - ginBeforeIndex - 7);
        xmlNewChild(sr882999_Node, NULL, BAD_CAST "AftData", BAD_CAST szTemplate);
        
        /* <F11Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr882999_Node, NULL, BAD_CAST "F11Adr", BAD_CAST szTemplate);
         
        /* <F37Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F11_LENTH_ + _ECC_F24_LENTH_ + _ECC_F25_LENTH_ + inField32 + 8;;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr882999_Node, NULL, BAD_CAST "F37Adr", BAD_CAST szTemplate);
       
	memset(szFESMode, 0x00, sizeof(szFESMode));
	inGetNCCCFESMode(szFESMode);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetCloud_MFES(szTemplate);
        if (memcmp(szFESMode, _NCCC_05_ATS_MODE_, 2) == 0	||
	    memcmp(szFESMode, _NCCC_04_MPAS_MODE_, 2) == 0	||
	   (memcmp(szFESMode, _NCCC_03_MFES_MODE_, 2) == 0	&&
	    memcmp(szTemplate, "Y", strlen("Y")) == 0))
	{
		/* <F39Adr> */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + _ECC_F24_LENTH_ + 
			    inField32 + _ECC_F37_LENTH_ + _ECC_F38_LENTH_;
		sprintf(szTemplate, "%d", inDataLen);
		xmlNewChild(sr882999_Node, NULL, BAD_CAST "F39Adr", BAD_CAST szTemplate);

		/* <ECCAdr> */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + _ECC_F24_LENTH_ + 
			    inField32 + _ECC_F37_LENTH_ + _ECC_F38_LENTH_ + _ECC_F39_LENTH_ + _ECC_F41_LENTH_ + _ECC_F56_LENTH_;
		sprintf(szTemplate, "%d", inDataLen);
		xmlNewChild(sr882999_Node, NULL, BAD_CAST "ECCAdr", BAD_CAST szTemplate);
	}
	else
	{
		/* <F39Adr> */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + _ECC_F24_LENTH_;
		sprintf(szTemplate, "%d", inDataLen);
		xmlNewChild(sr882999_Node, NULL, BAD_CAST "F39Adr", BAD_CAST szTemplate);

		/* <ECCAdr> */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + _ECC_F24_LENTH_ + 
			    _ECC_F39_LENTH_ + _ECC_F41_LENTH_ + _ECC_F56_LENTH_;
		sprintf(szTemplate, "%d", inDataLen);
		xmlNewChild(sr882999_Node, NULL, BAD_CAST "ECCAdr", BAD_CAST szTemplate);
	}

        /* ======================================================================================= */
        memset(szSendPacket, 0x00, sizeof(szSendPacket));
        inTxnCode = _TICKET_EASYCARD_REG_REV_;
        pobTran->inISOTxnCode = _TICKET_EASYCARD_REG_REV_;
  
        /* 組 ISO 電文 */
	ginBeforeIndex = 0;
	if ((inSendLen = inNCCC_TICKET_PackISO(pobTran, (unsigned char *)szSendPacket, inTxnCode)) <= 0)
	{
		/* 清空佔用記憶體 */
		inXML_End(&srDoc);
		
		/* 上傳不能Show錯誤訊息 */
		pobTran->inErrorMsg = _ERROR_CODE_V3_NONE_;
		
		return (VS_ISO_PACK_ERR);
	}
	
	/* 沒找到 */
	if (ginBeforeIndex == 0)
	{
		/* 清空佔用記憶體 */
		inXML_End(&srDoc);
		
	        return (VS_ERROR);
	}
  
	/* <882999_Rev> */
	sr882999rev_Node = xmlNewNode(NULL, BAD_CAST "882999_Rev");
	xmlAddChild(srRoot_Node, sr882999rev_Node);
        
        /* <BefData> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%s", "0000");
	inFunc_BCD_to_ASCII(&szTemplate[4], (unsigned char*)&szSendPacket[0], ginBeforeIndex);
        xmlNewChild(sr882999rev_Node, NULL, BAD_CAST "BefData", BAD_CAST szTemplate);
       
        /* <AftData> : 扣掉取代的Len + @@@@@ */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inFunc_BCD_to_ASCII(&szTemplate[0], (unsigned char*)&szSendPacket[ginBeforeIndex + 7], inSendLen - ginBeforeIndex - 7);
        xmlNewChild(sr882999rev_Node, NULL, BAD_CAST "AftData", BAD_CAST szTemplate);
        
        /* <F11Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr882999rev_Node, NULL, BAD_CAST "F11Adr", BAD_CAST szTemplate);
         
        /* <F37Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F11_LENTH_ + _ECC_F24_LENTH_ + _ECC_F25_LENTH_ + inField32 + 8;;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr882999rev_Node, NULL, BAD_CAST "F37Adr", BAD_CAST szTemplate);
       
	memset(szFESMode, 0x00, sizeof(szFESMode));
	inGetNCCCFESMode(szFESMode);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetCloud_MFES(szTemplate);
        if (memcmp(szFESMode, _NCCC_05_ATS_MODE_, 2) == 0	||
	    memcmp(szFESMode, _NCCC_04_MPAS_MODE_, 2) == 0	||
	   (memcmp(szFESMode, _NCCC_03_MFES_MODE_, 2) == 0	&&
	    memcmp(szTemplate, "Y", strlen("Y")) == 0))
	{
		/* <F39Adr> */
		/* SignOn Reversal 不要F38 2019/1/9 下午 4:20 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + _ECC_F24_LENTH_ + 
			    inField32 + _ECC_F37_LENTH_;
		sprintf(szTemplate, "%d", inDataLen);
		xmlNewChild(sr882999rev_Node, NULL, BAD_CAST "F39Adr", BAD_CAST szTemplate);

		/* <ECCAdr> */
		/* SignOn Reversal 不要F38 2019/1/9 下午 4:20 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + _ECC_F24_LENTH_ + 
			    inField32 + _ECC_F37_LENTH_+ _ECC_F39_LENTH_ + _ECC_F41_LENTH_ + _ECC_F56_LENTH_;
		sprintf(szTemplate, "%d", inDataLen);
		xmlNewChild(sr882999rev_Node, NULL, BAD_CAST "ECCAdr", BAD_CAST szTemplate);
	}
	else
	{
		/* <F39Adr> */
		/* SignOn Reversal 不要F38 2019/1/9 下午 4:20 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + _ECC_F24_LENTH_ + 
			    inField32 + _ECC_F37_LENTH_;
		sprintf(szTemplate, "%d", inDataLen);
		xmlNewChild(sr882999rev_Node, NULL, BAD_CAST "F39Adr", BAD_CAST szTemplate);

		/* <ECCAdr> */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + _ECC_F24_LENTH_ + 
			    _ECC_F39_LENTH_ + _ECC_F41_LENTH_ + _ECC_F56_LENTH_;
		sprintf(szTemplate, "%d", inDataLen);
		xmlNewChild(sr882999rev_Node, NULL, BAD_CAST "ECCAdr", BAD_CAST szTemplate);
	}
         
        /* ======================================================================================= */
        memset(szSendPacket, 0x00, sizeof(szSendPacket));
        inTxnCode = _ADVICE_;
        pobTran->inISOTxnCode = _ADVICE_;
        pobTran->srTRec.uszBlackListBit = VS_TRUE;
	
	/* 組 ISO 電文 */
	ginBeforeIndex = 0;
	if ((inSendLen = inNCCC_TICKET_PackISO(pobTran, (unsigned char *)szSendPacket, inTxnCode)) <= 0)
	{
		/* 清空佔用記憶體 */
		inXML_End(&srDoc);
		
		/* 上傳不能Show錯誤訊息 */
		pobTran->inErrorMsg = _ERROR_CODE_V3_NONE_;
		
		return (VS_ISO_PACK_ERR);
	}
	
	pobTran->srTRec.uszBlackListBit = VS_FALSE;
	
	/* 沒找到 */
	if (ginBeforeIndex == 0)
	{
		/* 清空佔用記憶體 */
		inXML_End(&srDoc);
		
	        return (VS_ERROR);
	}
	
	/* <596100_Adv> */
	sr596100adv_Node = xmlNewNode(NULL, BAD_CAST "596100_Adv");
	xmlAddChild(srRoot_Node, sr596100adv_Node);
        
        /* <BefData> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%s", "0000");
	inFunc_BCD_to_ASCII(&szTemplate[4], (unsigned char*)&szSendPacket[0], ginBeforeIndex);
        xmlNewChild(sr596100adv_Node, NULL, BAD_CAST "BefData", BAD_CAST szTemplate);
       
        /* <AftData> : 扣掉取代的Len + @@@@@ */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inFunc_BCD_to_ASCII(&szTemplate[0], (unsigned char*)&szSendPacket[ginBeforeIndex + 7], inSendLen - ginBeforeIndex - 7);
        xmlNewChild(sr596100adv_Node, NULL, BAD_CAST "AftData", BAD_CAST szTemplate);
        
	/* <F03Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr596100adv_Node, NULL, BAD_CAST "F03Adr", BAD_CAST szTemplate);
	
	/* <F04Adr> - 鎖卡金額要為零，不帶就是零 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr596100adv_Node, NULL, BAD_CAST "F04Adr", BAD_CAST szTemplate);
	
        /* <F11Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr596100adv_Node, NULL, BAD_CAST "F11Adr", BAD_CAST szTemplate);
        
	/* <F12Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr596100adv_Node, NULL, BAD_CAST "F12Adr", BAD_CAST szTemplate);
	
	/* <F13Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr596100adv_Node, NULL, BAD_CAST "F13Adr", BAD_CAST szTemplate);
	      
        /* <F35Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ +
                    _ECC_F22_LENTH_ + _ECC_F24_LENTH_ + _ECC_F25_LENTH_ + inField32 + 1;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr596100adv_Node, NULL, BAD_CAST "F35Adr", BAD_CAST szTemplate);
	
        /* <F37Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_+ _ECC_F03_LENTH_+ _ECC_F04_LENTH_+ _ECC_F11_LENTH_+ _ECC_F12_LENTH_+ _ECC_F13_LENTH_+
                    _ECC_F22_LENTH_+ _ECC_F24_LENTH_+ _ECC_F25_LENTH_+ inField32 + _ECC_F35_LENTH_+ 8;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr596100adv_Node, NULL, BAD_CAST "F37Adr", BAD_CAST szTemplate);
       
	if (memcmp(szIFESMode, "Y", 1) == 0)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = ginECC_F57_Len;
		sprintf(szTemplate, "-%d", inDataLen);
		xmlNewChild(sr596100adv_Node, NULL, BAD_CAST "F57Adr", BAD_CAST szTemplate);
	}
	
	/* <F59Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = ginECC_F59_ET_Len;
	sprintf(szTemplate, "-%d", inDataLen);
        xmlNewChild(sr596100adv_Node, NULL, BAD_CAST "F59Adr", BAD_CAST szTemplate);
	
//	memset(szFESMode, 0x00, sizeof(szFESMode));
//	inGetNCCCFESMode(szFESMode);
//	memset(szTemplate, 0x00, sizeof(szTemplate));
//	inGetCloud_MFES(szTemplate);
//        if (memcmp(szFESMode, _NCCC_05_ATS_MODE_, 2) == 0	||
//	    memcmp(szFESMode, _NCCC_04_MPAS_MODE_, 2) == 0	||
//	   (memcmp(szFESMode, _NCCC_03_MFES_MODE_, 2) == 0	&&
//	    memcmp(szTemplate, "Y", strlen("Y")) == 0))
//	{
		/* <F39Adr> */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F24_LENTH_ + inField32 + 
			    _ECC_F37_LENTH_ + _ECC_F38_LENTH_;
		sprintf(szTemplate, "%d", inDataLen);
		xmlNewChild(sr596100adv_Node, NULL, BAD_CAST "F39Adr", BAD_CAST szTemplate);

		/* <ECCAdr> */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F24_LENTH_ + inField32 + 
			    _ECC_F37_LENTH_ + _ECC_F38_LENTH_ + _ECC_F39_LENTH_ + _ECC_F41_LENTH_ + _ECC_F56_LENTH_;
		sprintf(szTemplate, "%d", inDataLen);
		xmlNewChild(sr596100adv_Node, NULL, BAD_CAST "ECCAdr", BAD_CAST szTemplate);
//	}
//	else
//	{
//		/* <F39Adr> */
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F24_LENTH_;
//		sprintf(szTemplate, "%d", inDataLen);
//		xmlNewChild(sr596100adv_Node, NULL, BAD_CAST "F39Adr", BAD_CAST szTemplate);
//
//		/* <ECCAdr> */
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F24_LENTH_ + _ECC_F39_LENTH_ + 
//			    _ECC_F41_LENTH_ + _ECC_F56_LENTH_;
//		sprintf(szTemplate, "%d", inDataLen);
//		xmlNewChild(sr596100adv_Node, NULL, BAD_CAST "ECCAdr", BAD_CAST szTemplate);
//	}
	
	/* 儲存xml檔 xml檔現在會自動換行了 */
        inXML_SaveFile(_ECC_API_REQ_FILE2_, &srDoc, "utf-8", 1);
	inFile_Move_File(_ECC_API_REQ_FILE2_, _AP_ROOT_PATH_, "", _ECC_FOLDER_PATH_);
	inFunc_Data_Copy(_ECC_API_REQ_FILE2_, _ECC_FOLDER_PATH_, _ECC_API_REQ_TEMP_, _ECC_FOLDER_PATH_);
	
	/* 清空佔用記憶體 */
	inXML_End(&srDoc);
	
	/* 處理暫存檔 */
	if (inECC_Process_Temp() != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	/* 會有權限問題，要先改權限 */
	inFunc_Data_Chmod("u+rwx", _ECC_API_REQ_TEMP_, _ECC_FOLDER_PATH_);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inECC_PACK_Init_ICERAPI_NCCC() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inECC_UNPACK_ICERAPI
Date&Time       :2018/3/27 下午 2:20
Describe        :
*/
int inECC_UNPACK_ICERAPI(TRANSACTION_OBJECT *pobTran, char *szFileName)
{
	int		i = 0;
	int		inRetVal = VS_ERROR;
	int		inESVCIndex = -1;
	int		inOffset = 0;
	char		szTemplate[20 + 1] = {0};
	char		szTagData[1536 + 1] = {0};
	char		szDebugMsg[100 + 1];
	char		szLastTransDate[8 + 1] = {0};
	char		szLastTransRRN[20 + 1] = {0};
	char		szT1101[6 + 1] = {0};
	char		szT1102[6 + 1] = {0};
	char		szT4100[20 + 1] = {0};
	char		szT4109[20 + 1] = {0};
	char		szT4110[20 + 1] = {0};
        char            szCustomerIndicator[3 + 1] = {0};
	xmlChar*	szTagPtr = NULL;
	xmlDocPtr	srDoc = NULL;
	xmlNodePtr	srCur = NULL;		/* (第二層) */
	xmlNodePtr	srTagNode = NULL;	/* 用來移動Tag那一層(第三層) */
	xmlNodePtr	srTextNode = NULL;	/* 因為ElementNode的子節點TextNode才有內容(第四層) */
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inECC_UNPACK_ICERAPI() START !");
	}
        
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
       
	inLoadTDTRec(_TDT_INDEX_01_ECC_);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTicket_HostIndex(szTemplate);
	inESVCIndex = atoi(szTemplate);
	inLoadHDTRec(inESVCIndex);
	inLoadHDPTRec(inESVCIndex);
				
	inRetVal = inECC_ParseXML_In_ICERData(szFileName, &srDoc);
	/* 異常 */
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	inRetVal = inXML_Get_RootElement(&srDoc, &srCur);
	/* 異常 */
	if (inRetVal != VS_SUCCESS)
	{
		inXML_End(&srDoc);
		
		return (VS_ERROR);
	}
		
	/* 先走到TransXML的下一層 */
	while (srCur->children != NULL)
	{
		srCur = srCur->children;
		if (memcmp(srCur->parent->name , "TransXML", strlen("TransXML")) == 0)
		{
			break;
		}
	}
	
	memset(szT1101, 0x00, sizeof(szT1101));
	memset(szT1102, 0x00, sizeof(szT1102));
	memset(szT4100, 0x00, sizeof(szT4100));
	memset(szT4109, 0x00, sizeof(szT4109));
	memset(szT4110, 0x00, sizeof(szT4110));
	
	/* 換下一個節點檢查，往下走到TRANS的子節點，接著開始往橫走 */
	while (srCur != NULL)
	{
		/* 每次都設為NULL，如果有要查看的話才設定位置 */
		srTagNode = NULL;
		if (memcmp(srCur->name, "TRANS", strlen("TRANS")) == 0	&&
	            srCur->children != NULL)
		{
			srTagNode = srCur ->children;
		}
		
		/* 推移第三層 */
		while (srTagNode != NULL)
		{
			/* 有分XML_TEXT_NODE和XML_ELEMENT_NODE 才有content可抓 */
			if (srTagNode->type == XML_ELEMENT_NODE	&&
			   (srTagNode->children != NULL		&&
			    srTagNode->children->type == XML_TEXT_NODE))
			{
				srTextNode = srTagNode->children;
				memset(szTagData, 0x00, sizeof(szTagData));
				szTagPtr = xmlNodeListGetString(srDoc, srTextNode, srTextNode->line);
				if (szTagPtr != NULL	&&
				    xmlStrlen(szTagPtr) != 0)
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s : %s", srTagNode->name, szTagPtr);
						inLogPrintf(AT, szDebugMsg);
					}

					strcat(szTagData, (char*)szTagPtr);
					xmlFree(szTagPtr);
				}
			}

			/* 悠遊卡號內碼(改列印這個) */
			if (xmlStrcmp(srTagNode->name, (const xmlChar*)"T0200") == 0)
			{
				inOffset = 0;
				for (i = 0; i < strlen(szTagData); i++)
				{
					/* 非0位置 */
					if (szTagData[i] != '0')
					{
						inOffset = i;
						break;
					}
				}
				
				/* 悠遊卡號內碼 */
				memset(pobTran->srTRec.srECCRec.szCardID, 0x00, sizeof(pobTran->srTRec.srECCRec.szCardID));
				memcpy(&pobTran->srTRec.srECCRec.szCardID[0], &szTagData[inOffset], (strlen(szTagData) - inOffset)); 
			}
			else if (xmlStrcmp(srTagNode->name, (const xmlChar*)"T0211") == 0)
			{
				inOffset = 0;
				for (i = 0; i < strlen(szTagData); i++)
				{
					/* 非0位置 */
					if (szTagData[i] != '0')
					{
						inOffset = i;
						break;
					}
				}
				
				/* 悠遊卡外觀卡號 */
				memset(pobTran->srTRec.srECCRec.szPurseID, 0x00, sizeof(pobTran->srTRec.srECCRec.szPurseID));
				memcpy(&pobTran->srTRec.srECCRec.szPurseID[0], &szTagData[inOffset], (strlen(szTagData) - inOffset)); 
			}
			else if (xmlStrcmp(srTagNode->name, (const xmlChar*)"T0215") == 0)
			{
				
			}
			else if (xmlStrcmp(srTagNode->name, (const xmlChar*)"T0400") == 0)
			{
				/* 金額 */
			}
			else if (xmlStrcmp(srTagNode->name, (const xmlChar*)"T0409") == 0)
			{
				/* 自動加值金額 */
				if (pobTran->srTRec.inCode == _TICKET_EASYCARD_DEDUCT_)
				{
					memset(szTemplate, 0x00, sizeof(szTemplate));    
					memcpy(&szTemplate[0], &szTagData[0], (strlen(szTagData) - 2)); 
					pobTran->srTRec.lnTotalTopUpAmount = atol(szTemplate);
				}
			}
			else if (xmlStrcmp(srTagNode->name, (const xmlChar*)"T0410") == 0)
			{
				/* 交易後餘額 */
				memset(szTemplate, 0x00, sizeof(szTemplate));    
				memcpy(&szTemplate[0], &szTagData[0], (strlen(szTagData) - 2)); 
				pobTran->srTRec.lnFinalAfterAmt = atol(szTemplate);
				pobTran->srTRec.lnCardRemainAmount = pobTran->srTRec.lnFinalAfterAmt;  
			}
			else if (xmlStrcmp(srTagNode->name, (const xmlChar*)"T0415") == 0)
			{
				/* 交易前餘額 */
				memset(szTemplate, 0x00, sizeof(szTemplate));    
				memcpy(&szTemplate[0], &szTagData[0], (strlen(szTagData) - 2)); 
				pobTran->srTRec.lnFinalBeforeAmt = atol(szTemplate); 
			}
			else if (xmlStrcmp(srTagNode->name, (const xmlChar*)"T1101") == 0)
			{
				/* Host Serial Number */
				if (atol(szTagData) != 0)
				{       
					sprintf(szT1101, "%06ld", atol(szTagData));
				}
			}
			else if (xmlStrcmp(srTagNode->name, (const xmlChar*)"T1102") == 0)
			{
				/* NCCC STAN */
				if (atol(szTagData) != 0)
				{
					sprintf(szT1102, "%06ld", atol(szTagData));
				} 
			}
			else if (xmlStrcmp(srTagNode->name, (const xmlChar*)"T1103") == 0)
			{
				/* NCCC RRN */
				if (atol(szTagData) != 0)
				{
					/* 正常情況，悠遊卡會累加成下一筆，有自動加值或自動SignOn序號會再各多加一 */
					pobTran->srTRec.lnECCInvNum = atol(szTagData);
					pobTran->srTRec.lnCountInvNum = pobTran->srTRec.lnECCInvNum;
				}
			}
			else if (xmlStrcmp(srTagNode->name, (const xmlChar*)"T1200") == 0)
			{
				/* 交易時間 HHMMSS */
				memset(pobTran->srTRec.srECCRec.szTime, 0x00, sizeof(pobTran->srTRec.srECCRec.szTime));    
				memcpy(&pobTran->srTRec.srECCRec.szTime[0], &szTagData[0], 6);  
			}
			else if (xmlStrcmp(srTagNode->name, (const xmlChar*)"T1300") == 0)
			{
				/* 交易日期 YYYYMMDD */
				memset(pobTran->srTRec.srECCRec.szDate, 0x00, sizeof(pobTran->srTRec.srECCRec.szDate));    
				memcpy(&pobTran->srTRec.srECCRec.szDate[0], &szTagData[0], 8);  

				memset(szLastTransDate, 0x00, sizeof(szLastTransDate));
				memcpy(szLastTransDate, &szTagData[0], 8);
			}
			else if (xmlStrcmp(srTagNode->name, (const xmlChar*)"T3700") == 0)
			{
				/* RRN */
				memset(pobTran->srTRec.srECCRec.szRRN, 0x00, sizeof(pobTran->srTRec.srECCRec.szRRN)); 
				memcpy(&pobTran->srTRec.srECCRec.szRRN[0], &szTagData[0], strlen(szTagData));
				
				memset(szLastTransRRN, 0x00, sizeof(szLastTransRRN)); 
				memcpy(szLastTransRRN, &szTagData[0], strlen(szTagData));  
			}
			else if (xmlStrcmp(srTagNode->name, (const xmlChar*)"T3800") == 0)
			{
				/* AuthCode */
				memset(pobTran->srTRec.szAuthCode, 0x00, sizeof(pobTran->srTRec.szAuthCode)); 
				memcpy(&pobTran->srTRec.szAuthCode[0], &szTagData[0], strlen(szTagData));   
			}
			else if (xmlStrcmp(srTagNode->name, (const xmlChar*)"T3900") == 0)
			{
				/* ICER RespCode */
				memset(pobTran->srTRec.srECCRec.szICER_RespCode, 0x00, sizeof(pobTran->srTRec.srECCRec.szICER_RespCode)); 
				memcpy(&pobTran->srTRec.srECCRec.szICER_RespCode[0], &szTagData[0], strlen(szTagData));  
			}
			else if (xmlStrcmp(srTagNode->name, (const xmlChar*)"T3901") == 0)
			{
				/* API RespCode */
				memset(pobTran->srTRec.srECCRec.szAPI_RespCode, 0x00, sizeof(pobTran->srTRec.srECCRec.szAPI_RespCode)); 
				memcpy(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], &szTagData[0], strlen(szTagData));
			}
			else if (xmlStrcmp(srTagNode->name, (const xmlChar*)"T3902") == 0)
			{
				/* EZHOST RespCode */
				memset(pobTran->srTRec.srECCRec.szEZ_RespCode, 0x00, sizeof(pobTran->srTRec.srECCRec.szEZ_RespCode)); 
				memcpy(&pobTran->srTRec.srECCRec.szEZ_RespCode[0], &szTagData[0], strlen(szTagData));   
			}
			else if (xmlStrcmp(srTagNode->name, (const xmlChar*)"T3903") == 0)
			{
				/* SVCS RespCode */
				memset(pobTran->srTRec.srECCRec.szSVCS_RespCode, 0x00, sizeof(pobTran->srTRec.srECCRec.szSVCS_RespCode)); 
				memcpy(&pobTran->srTRec.srECCRec.szSVCS_RespCode[0], &szTagData[0], strlen(szTagData));
			}
			else if (xmlStrcmp(srTagNode->name, (const xmlChar*)"T3904") == 0)
			{
				/* Reader RespCode */
				memset(pobTran->srTRec.srECCRec.szReader_RespCode, 0x00, sizeof(pobTran->srTRec.srECCRec.szReader_RespCode)); 
				memcpy(&pobTran->srTRec.srECCRec.szReader_RespCode[0], &szTagData[0], strlen(szTagData));   
			}
			else if (xmlStrcmp(srTagNode->name, (const xmlChar*)"T3908") == 0)
			{
				/* NCCC Host RespCode */
				memset(pobTran->srTRec.szRespCode, 0x00, sizeof(pobTran->srTRec.szRespCode));
				memcpy(&pobTran->srTRec.szRespCode[0], &szTagData[0], strlen(szTagData));
			}
			else if (xmlStrcmp(srTagNode->name, (const xmlChar*)"T4109") == 0)
			{
				/* 一代設備編號 */
				strcpy(szT4109, szTagData);
			}
			else if (xmlStrcmp(srTagNode->name, (const xmlChar*)"T4100") == 0)
			{
				/* T5581使用 */
				strcpy(szT4100, szTagData);
			}
			else if (xmlStrcmp(srTagNode->name, (const xmlChar*)"T4110") == 0)
			{
				/* 二代設備編號 */
				strcpy(szT4110, szTagData);
			}
			else if (xmlStrcmp(srTagNode->name, (const xmlChar*)"T4800") == 0)
			{
				/* 票卡版號 */
				strcpy(pobTran->srTRec.srECCRec.szPurseVersionNumber, szTagData);
			}

			srTagNode = srTagNode->next;
		}
		
		/* 推移第二層 */
		srCur = srCur->next;
	}
	
	/* free */
	inXML_End(&srDoc);
	
	if (strlen(szT1101) > 0)
	{
		inSetTicket_STAN(szT1101);
	}
	if (strlen(szT1102) > 0)
	{
		inSetSTANNum(szT1102);
	}
	if (strlen(szT4109) > 0)
	{
		inSetTicket_Device1(szT4109);
	}
	if (strlen(szT4100) > 0)
	{
		inSetTicket_Device3(szT4100);
	}
	if (strlen(szT4110) > 0)
	{
		inSetTicket_Device2(szT4110);
	}
	
	/* <T4800>票卡版號(Purse Version Number)=0，
	 * <T0200>列印晶片卡號(Card Id)<T0200>20 bytes，14th,15th,16th隱碼，17th,18th,19th,20th顯示
	 * 
	 * <T4800>票卡版號(Purse Version Number)<>0，
	 * <T0211>列印外觀卡號(Purse Id)
	 * <T0211>16 bytes，6th,7th,8th,9th,10th,11th隱碼，12th,13th,14th,15th,16th顯示，
	 * 遮掩字元為”*”

*/
        if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)     ||
            !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_))
        {
                /* 客製化098，悠遊卡因為使用T0200，加密遮掩為倒數5、6、7 */
                if (strlen(pobTran->srTRec.srECCRec.szCardID) > 0)
                {
                        memset(pobTran->srTRec.szUID, 0x00, sizeof(pobTran->srTRec.szUID));
                        strcpy(pobTran->srTRec.szUID, pobTran->srTRec.srECCRec.szCardID);
                }
        }
        else
        {    
                if (atoi(pobTran->srTRec.srECCRec.szPurseVersionNumber) == 0)
                {
                        if (strlen(pobTran->srTRec.srECCRec.szCardID) > 0)
                        {
                                memset(pobTran->srTRec.szUID, 0x00, sizeof(pobTran->srTRec.szUID));
                                strcpy(pobTran->srTRec.szUID, pobTran->srTRec.srECCRec.szCardID);
                        }
                }
                else
                {
                        if (strlen(pobTran->srTRec.srECCRec.szPurseID) > 0)
                        {
                                memset(pobTran->srTRec.szUID, 0x00, sizeof(pobTran->srTRec.szUID));
                                strcpy(pobTran->srTRec.szUID, pobTran->srTRec.srECCRec.szPurseID);
                        }
                }
        }
				
	/* 邊解邊存的話，有可能這筆購貨失敗但上傳鎖卡advice，此時會存到鎖卡的RRN和Lastdate，導致取消加值帶錯資料 */
	if (memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "0", 1) == 0)
	{
		
		if (strlen(szLastTransDate) > 0)
		{
			if (pobTran->srTRec.inCode == _TICKET_EASYCARD_DEDUCT_	||
			    pobTran->srTRec.inCode == _TICKET_EASYCARD_REFUND_	||
			    pobTran->srTRec.inCode == _TICKET_EASYCARD_TOP_UP_	||
			    pobTran->srTRec.inCode == _TICKET_EASYCARD_INQUIRY_ ||
			    pobTran->srTRec.inCode == _TICKET_EASYCARD_VOID_TOP_UP_)
			{
				/* 存最後一筆交易的時間 */
				inSetTicket_LastTransDate(szLastTransDate);
			}
		}
		
		if (strlen(szLastTransRRN) > 0)
		{
			inSetTicket_LastRRN(szLastTransRRN);
		}
		
		
	}
	
	inSaveHDPTRec(inESVCIndex);
	inSaveTDTRec(_TDT_INDEX_01_ECC_);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inECC_UNPACK_ICERAPI() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	       
	return (VS_SUCCESS);
}

/*
Function        :inECC_UNPACK_ICERAPI_NCCC
Date&Time       :2018/3/30 下午 1:34
Describe        :
*/
int inECC_UNPACK_ICERAPI_NCCC(TRANSACTION_OBJECT *pobTran, char *szFileName, char* szSource)
{
	int		inFd = 0;
	int		i = 0;
	int		j = 0;
	int		inRetVal = VS_ERROR;
	int		inESVCIndex = -1;
	int		inFileSize = 0;
	int		inLastTagType = _XML_TAG_TYPE_NONE_;
	int		inCnt = 0;
	int		inPackCnt = 0;
	int		inTagDataBCDLen = 0;
	int		inBitMap = 0;
	char		szTemplate[20 + 1] = {0};
	char		szTagData[1536 + 1] = {0};
	char		szSearch[50 + 1] = {0};
	char		szDebugMsg[2500 + 1] = {0};	/* szDebugMsg 容量要比 szTagData 大 */
	char		szPath[50 + 1] = {0};
	char		szData[5120] = {0};
	char		szDataTemp[10 + 1] = {0};
	char		szTagName[100 + 1] = {0};
	char		szParentTagName[50 + 1] = {0};
	char		szBitMap[16 + 1] = {0};
	unsigned char	uszTagDataBCD[1536 + 1] = {0};
	unsigned char	uszTagBit = VS_FALSE;
	unsigned char	uszStartTagBit = VS_FALSE;
	unsigned char	uszEndTagBit = VS_FALSE;
	unsigned char	uszF59Bit = VS_FALSE;
	unsigned char	uszF60Bit = VS_FALSE;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inECC_UNPACK_ICERAPI_NCCC() START !");
	}
       
	inLoadTDTRec(_TDT_INDEX_01_ECC_);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTicket_HostIndex(szTemplate);
	inESVCIndex = atoi(szTemplate);
	inLoadHDTRec(inESVCIndex);
	inLoadHDPTRec(inESVCIndex);
	
	memset(szPath, 0x00, sizeof(szPath));
	sprintf(szPath, "%s%s", szSource, szFileName);
	
	inRetVal = inFile_Linux_Get_FileSize_By_Stat(szPath, &inFileSize);
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	inRetVal = inFile_Linux_Open(&inFd, szPath);
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	inRetVal = inFile_Linux_Seek(inFd, 0, _SEEK_BEGIN_);
	if (inRetVal != VS_SUCCESS)
	{
		inFile_Linux_Close(inFd);
		
		return (VS_ERROR);
	}
	
	memset(szData, 0x00, sizeof(szData));
	inRetVal = inFile_Linux_Read(inFd, szData, &inFileSize);
	if (inRetVal != VS_SUCCESS)
	{
		inFile_Linux_Close(inFd);
		
		return (VS_ERROR);
	}
	
	inFile_Linux_Close(inFd);
	
	/* 1.先判斷是開頭tag或結尾Tag，
	 *   如果是開頭Tag，
	 *   uszTagBit代表現在紀錄的內容為Tag的名稱，
	 *   uszStartTagBit和uszEndTagBit在遇到'>'時用來判斷前面是開頭還是結尾Tag
	 *   如果是結尾Tag，
	 *   
	 */
	for (i = 0; i < inFileSize; i++)
	{
		/* 有可能是Tag開頭或結尾 */
		if (szData[i] == '<')
		{
			/* 是結尾Tag */
			if (szData[i + 1] == '/')
			{
				/* 代表一個Tag關閉了 */
				if (inLastTagType == _XML_TAG_TYPE_START_)
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "<Parent> : %s", szParentTagName);
						inLogPrintf(AT, szDebugMsg);
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "<%s> : %s", szTagName, szTagData);
						inLogPrintf(AT, szDebugMsg);
					}
					
					if (ginISODebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "<Parent> : %s", szParentTagName);
						inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "<%s> : %s", szTagName, szTagData);
						inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
					}
					
					if (memcmp(szTagName, "BefData", strlen("BefData")) == 0)
					{
						/* 只有購貨要抓資料 */
						if (memcmp(szParentTagName, "606100", strlen("606100")) == 0)
						{
							/* <BefData> 尋找 */
							for (j = 0; j < strlen(szTagData); j++)
							{
								memset(szTemplate, 0x00, sizeof(szTemplate)); 
								memcpy(&szTemplate[0], &szTagData[j], 16);  

								/* RRN 前八碼找Auth code */
								memset(szSearch, 0x00, sizeof(szSearch)); 
								inFunc_BCD_to_ASCII(&szSearch[0], (unsigned char*)pobTran->srTRec.szRefNo, 8); 

								if (memcmp(szTemplate, szSearch, 16) == 0)
								{
									memset(pobTran->srTRec.szECCAuthCode, 0x00, sizeof(pobTran->srTRec.szECCAuthCode)); 
									inFunc_ASCII_to_BCD((unsigned char*)pobTran->srTRec.szECCAuthCode, &szTagData[j + 24], 6);  
									j += 24;

									break;
								}
							}
						}
						
						/* 取得BMAP 16Byte,前面有 4(長度) + 10(TPDU) + 4 (MTI) = 18*/
						memset(szBitMap, 0x00, sizeof(szBitMap));
						memcpy(szBitMap, &szTagData[18], 16);
					}
					else if (memcmp(szTagName, "AftData", strlen("AftData")) == 0)
					{
						if (szBitMap[14] - '0' >= 0 && szBitMap[14] - '0' <= 9)
						{
							inBitMap = szBitMap[14] - '0';
						}
						else
						{
							inBitMap = szBitMap[14] - 'A' + 10;
						}
						
						if (inBitMap / 8 == 1)
						{
							
						}
						inBitMap = inBitMap % 8;
						if (inBitMap / 4 == 1)
						{
							
						}
						inBitMap = inBitMap % 4;
						if (inBitMap / 2 == 1)
						{
							uszF59Bit = VS_TRUE;
						}
						else
						{
							uszF59Bit = VS_FALSE;
						}
						inBitMap = inBitMap % 2;
						if (inBitMap > 0)
						{
							uszF60Bit = VS_TRUE;
						}
						else
						{
							uszF60Bit = VS_FALSE;
						}
						
						/* 購貨要抓的內容 */
						if (memcmp(szParentTagName, "606100", strlen(szParentTagName)) == 0)
						{
							/* <AftData> 尋找 */
							/* 解析F_59 */
							if (uszF59Bit == VS_TRUE	&&
							    inTagDataBCDLen > 0)
							{
								inNCCC_TICKET_UnPack59(pobTran, &uszTagDataBCD[inCnt]);
								inPackCnt = 0;
								inPackCnt += 2;
								inPackCnt += ((uszTagDataBCD[inCnt] / 16 * 10 + uszTagDataBCD[inCnt] % 16) * 100);
								inPackCnt += (uszTagDataBCD[inCnt + 1] / 16 * 10) + uszTagDataBCD[inCnt + 1] % 16;
								inCnt += inPackCnt;
								inTagDataBCDLen -= inPackCnt;
							}
							
							/* 解析F_60 */
							if (uszF60Bit == VS_TRUE	&&
							    inTagDataBCDLen > 0)
							{
								inRetVal = inNCCC_TICKET_UnPack60(pobTran, &uszTagDataBCD[inCnt]);
								inPackCnt = 0;
								inPackCnt += 2;
								inPackCnt += ((uszTagDataBCD[inCnt] / 16 * 10 + uszTagDataBCD[inCnt] % 16) * 100);
								inPackCnt += (uszTagDataBCD[inCnt + 1] / 16 * 10) + uszTagDataBCD[inCnt + 1] % 16;
								inCnt += inPackCnt;
								inTagDataBCDLen -= inPackCnt;
							}
						}
						/* 退貨 */
						else if (memcmp(szParentTagName, "620061", strlen(szParentTagName)) == 0)
						{
							/* <AftData> 尋找 */
							/* 解析F_59 */
							if (uszF59Bit == VS_TRUE	&&
							    inTagDataBCDLen > 0)
							{
								inNCCC_TICKET_UnPack59(pobTran, &uszTagDataBCD[inCnt]);
								inPackCnt = 0;
								inPackCnt += 2;
								inPackCnt += ((uszTagDataBCD[inCnt] / 16 * 10 + uszTagDataBCD[inCnt] % 16) * 100);
								inPackCnt += (uszTagDataBCD[inCnt + 1] / 16 * 10) + uszTagDataBCD[inCnt + 1] % 16;
								inCnt += inPackCnt;
								inTagDataBCDLen -= inPackCnt;
							}
							
							/* 解析F_60 */
							if (uszF60Bit == VS_TRUE	&&
							    inTagDataBCDLen > 0)
							{
								inRetVal = inNCCC_TICKET_UnPack60(pobTran, &uszTagDataBCD[inCnt]);
								inPackCnt = 0;
								inPackCnt += 2;
								inPackCnt += ((uszTagDataBCD[inCnt] / 16 * 10 + uszTagDataBCD[inCnt] % 16) * 100);
								inPackCnt += (uszTagDataBCD[inCnt + 1] / 16 * 10) + uszTagDataBCD[inCnt + 1] % 16;
								inCnt += inPackCnt;
								inTagDataBCDLen -= inPackCnt;
							}
						}
						/* 加值 */
						else if (memcmp(szParentTagName, "801061", strlen(szParentTagName)) == 0)
						{
							/* <AftData> 尋找 */
							/* 解析F_59 */
							if (uszF59Bit == VS_TRUE	&&
							    inTagDataBCDLen > 0)
							{
								inNCCC_TICKET_UnPack59(pobTran, &uszTagDataBCD[inCnt]);
								inPackCnt = 0;
								inPackCnt += 2;
								inPackCnt += ((uszTagDataBCD[inCnt] / 16 * 10 + uszTagDataBCD[inCnt] % 16) * 100);
								inPackCnt += (uszTagDataBCD[inCnt + 1] / 16 * 10) + uszTagDataBCD[inCnt + 1] % 16;
								inCnt += inPackCnt;
								inTagDataBCDLen -= inPackCnt;
							}
							
							/* 解析F_60 */
							if (uszF60Bit == VS_TRUE	&&
							    inTagDataBCDLen > 0)
							{
								inRetVal = inNCCC_TICKET_UnPack60(pobTran, &uszTagDataBCD[inCnt]);
								inPackCnt = 0;
								inPackCnt += 2;
								inPackCnt += ((uszTagDataBCD[inCnt] / 16 * 10 + uszTagDataBCD[inCnt] % 16) * 100);
								inPackCnt += (uszTagDataBCD[inCnt + 1] / 16 * 10) + uszTagDataBCD[inCnt + 1] % 16;
								inCnt += inPackCnt;
								inTagDataBCDLen -= inPackCnt;
							}
						}
						/* 加值取消 */
						else if (memcmp(szParentTagName, "816101", strlen(szParentTagName)) == 0)
						{
							/* <AftData> 尋找 */
							/* 解析F_59 */
							if (uszF59Bit == VS_TRUE	&&
							    inTagDataBCDLen > 0)
							{
								inNCCC_TICKET_UnPack59(pobTran, &uszTagDataBCD[inCnt]);
								inPackCnt = 0;
								inPackCnt += 2;
								inPackCnt += ((uszTagDataBCD[inCnt] / 16 * 10 + uszTagDataBCD[inCnt] % 16) * 100);
								inPackCnt += (uszTagDataBCD[inCnt + 1] / 16 * 10) + uszTagDataBCD[inCnt + 1] % 16;
								inCnt += inPackCnt;
								inTagDataBCDLen -= inPackCnt;
							}
							
							/* 解析F_60 */
							if (uszF60Bit == VS_TRUE	&&
							    inTagDataBCDLen > 0)
							{
								inRetVal = inNCCC_TICKET_UnPack60(pobTran, &uszTagDataBCD[inCnt]);
								inPackCnt = 0;
								inPackCnt += 2;
								inPackCnt += ((uszTagDataBCD[inCnt] / 16 * 10 + uszTagDataBCD[inCnt] % 16) * 100);
								inPackCnt += (uszTagDataBCD[inCnt + 1] / 16 * 10) + uszTagDataBCD[inCnt + 1] % 16;
								inCnt += inPackCnt;
								inTagDataBCDLen -= inPackCnt;
							}
						}
						else
						{
							/* 避免重複解析sale和advice重複解析 */
						}
					}
				}
				/* 連續兩個結束 代表回上一層 */
				else if (inLastTagType == _XML_TAG_TYPE_END_)
				{
					memset(szParentTagName, 0x00, sizeof(szParentTagName));
					inCnt = 0;
				}
				
				memset(szTagName, 0x00, sizeof(szTagName));
				uszTagBit = VS_TRUE;
				uszEndTagBit = VS_TRUE;
				/* 要跳過斜線 */
				i += 1;
				continue;
			}
			/* 是開頭Tag */
			else
			{
				/* 連續兩個開頭Tag，代表前一個是後一個的Parent */
				if (inLastTagType == _XML_TAG_TYPE_START_)
				{
					memset(szParentTagName, 0x00, sizeof(szParentTagName));
					strcpy(szParentTagName, szTagName);
				}
				
				memset(szTagName, 0x00, sizeof(szTagName));
				uszTagBit = VS_TRUE;
				uszStartTagBit = VS_TRUE;
				/* '<'不用紀錄 */
				continue;
			}
		}
		else if (szData[i] == '>')
		{
			if (uszStartTagBit == VS_TRUE)
			{
				inLastTagType = _XML_TAG_TYPE_START_;
			}
			else if (uszEndTagBit == VS_TRUE)
			{
				inLastTagType = _XML_TAG_TYPE_END_;
			}
			
			uszTagBit = VS_FALSE;
			uszStartTagBit = VS_FALSE;
			uszEndTagBit = VS_FALSE;
			/* 準備紀錄TagData */
			memset(szTagData, 0x00, sizeof(szTagData));
			memset(uszTagDataBCD, 0x00, sizeof(uszTagDataBCD));
			/* '>'不用紀錄 */
			continue;
		}
		
		szDataTemp[0] = szData[i];
		/* 需要紀錄Tag */
		if (uszTagBit == VS_TRUE)
		{
			strcat(szTagName, szDataTemp);
		}
		else
		{
			strcat(szTagData, szDataTemp);
			inTagDataBCDLen = strlen(szTagData) / 2;
			inFunc_ASCII_to_BCD(uszTagDataBCD, szTagData, inTagDataBCDLen);
		}
		
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inECC_UNPACK_ICERAPI_NCCC() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inECC_ResponeCode
Date&Time       :2018/3/30 下午 3:51
Describe        :
*/
int inECC_ResponeCode(TRANSACTION_OBJECT *pobTran)
{
	int	inRespCodeLen = 0;
        char    szLine1[66 + 1] = {0}, szLine2[66 + 1] = {0}, szLine3[66 + 1] = {0};
	char	szOther[10 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
	char	szFuncEnable[2 + 1] = {0};
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_) ||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_))
	{
		inECC_ResponeCode_MCDONALDS(pobTran);
		
		return (VS_SUCCESS);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inECC_ResponeCode() START !");
	}
	
	memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
	inGetIntegrate_Device(szFuncEnable);
	if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
	{

	}
	else
	{
		inCTLS_Set_LED(_CTLS_LIGHT_RED_);
	}
	
        memset(szLine1, 0x00, sizeof(szLine1)); 
        memset(szLine2, 0x00, sizeof(szLine2)); 
        memset(szLine3, 0x00, sizeof(szLine3)); 
        inRespCodeLen = atoi(pobTran->srTRec.srECCRec.szAPI_RespCode);
	
	/* <T3901> */
        if (!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-101", 4))
        {
                /* <T3900> */
                if (!memcmp(&pobTran->srTRec.srECCRec.szICER_RespCode[0], "06", 2))
		{
			/* <T3903> */
			if (!memcmp(&pobTran->srTRec.srECCRec.szSVCS_RespCode[0], "04", 2) ||
			    !memcmp(&pobTran->srTRec.srECCRec.szSVCS_RespCode[0], "05", 2) ||
			    !memcmp(&pobTran->srTRec.srECCRec.szSVCS_RespCode[0], "N3", 2) ||
			    !memcmp(&pobTran->srTRec.srECCRec.szSVCS_RespCode[0], "N4", 2))
			{
				sprintf(szLine3, "%s", "票卡異常");
	                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
			}
			else if (!memcmp(&pobTran->srTRec.srECCRec.szSVCS_RespCode[0], "13", 2))
			{
				sprintf(szLine3, "%s", "請確認交易金額");
	                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
			}
			else if (!memcmp(&pobTran->srTRec.srECCRec.szSVCS_RespCode[0], "19", 2))
			{
				sprintf(szLine3, "%s", "交易重複");
	                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
			}
			else if (!memcmp(&pobTran->srTRec.srECCRec.szSVCS_RespCode[0], "51", 2) ||
			         !memcmp(&pobTran->srTRec.srECCRec.szSVCS_RespCode[0], "57", 2) ||
			         !memcmp(&pobTran->srTRec.srECCRec.szSVCS_RespCode[0], "Q1", 2) ||
			         !memcmp(&pobTran->srTRec.srECCRec.szSVCS_RespCode[0], "C0", 2))
			{
				sprintf(szLine3, "%s", "請與發卡銀行確認");
	                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0002");
			}
			else if (!memcmp(&pobTran->srTRec.srECCRec.szSVCS_RespCode[0], "54", 2))
			{
				sprintf(szLine3, "%s", "卡片過期");
	                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
			}
			else if (!memcmp(&pobTran->srTRec.srECCRec.szSVCS_RespCode[0], "55", 2))
			{
				sprintf(szLine3, "%s", "無需展期");
	                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
			}
	                else
			{
			        sprintf(szLine3, "%s", "交易失敗");
	                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
			}
			
			sprintf(szLine2, "[T3903 = %s]", pobTran->srTRec.srECCRec.szSVCS_RespCode);
		}
		else if (!memcmp(&pobTran->srTRec.srECCRec.szICER_RespCode[0], "13", 2))
		{
		        sprintf(szLine3, "%s", "非實名卡");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
		}
                else if (!memcmp(&pobTran->srTRec.srECCRec.szICER_RespCode[0], "14", 2))
		{
		        sprintf(szLine3, "%s", "卡號不合法");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
		}
		else if (!memcmp(&pobTran->srTRec.srECCRec.szICER_RespCode[0], "17", 2))
		{
		        sprintf(szLine3, "%s", "超過交易次限額");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
		}
		else if (!memcmp(&pobTran->srTRec.srECCRec.szICER_RespCode[0], "19", 2))
		{
		        sprintf(szLine3, "%s", "交易重複");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
		}
		else if (!memcmp(&pobTran->srTRec.srECCRec.szICER_RespCode[0], "60", 2))
		{
		        sprintf(szLine3, "%s", "配對失敗");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
		}
		else if (!memcmp(&pobTran->srTRec.srECCRec.szICER_RespCode[0], "76", 2))
		{
		        sprintf(szLine3, "%s", "交易失敗");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0014");
		}
                else
		{
		        sprintf(szLine3, "%s", "交易失敗");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
		}

		sprintf(szOther, "%s", pobTran->srTRec.srECCRec.szICER_RespCode);
        }
        else if (!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-102", 4))
        {
                /* <T3902> */
                if (!memcmp(&pobTran->srTRec.srECCRec.szEZ_RespCode[0], "13", 2))
		{
		        sprintf(szLine3, "%s", "請確認交易金額");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
		}
                else if (!memcmp(&pobTran->srTRec.srECCRec.szEZ_RespCode[0], "14", 2))
		{
		        sprintf(szLine3, "%s", "票卡異常");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
		}
                else if (!memcmp(&pobTran->srTRec.srECCRec.szEZ_RespCode[0], "19", 2))
		{
		        sprintf(szLine3, "%s", "交易重複");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
		}
                else if (!memcmp(&pobTran->srTRec.srECCRec.szEZ_RespCode[0], "51", 2))
		{
		        sprintf(szLine3, "%s", "請與發卡銀行確認");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0002");
		}
                else if (!memcmp(&pobTran->srTRec.srECCRec.szEZ_RespCode[0], "54", 2))
		{
		        sprintf(szLine3, "%s", "票卡異常");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
		}
                else if (!memcmp(&pobTran->srTRec.srECCRec.szEZ_RespCode[0], "55", 2))
		{
		        sprintf(szLine3, "%s", "無需展期");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
		}
                else if (!memcmp(&pobTran->srTRec.srECCRec.szEZ_RespCode[0], "57", 2))
		{
		        sprintf(szLine3, "%s", "請與發卡銀行確認");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0002");
		}
                else if (!memcmp(&pobTran->srTRec.srECCRec.szEZ_RespCode[0], "61", 2))
		{
		        sprintf(szLine3, "%s", "超過金額上限");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0012");
		}
                else if (!memcmp(&pobTran->srTRec.srECCRec.szEZ_RespCode[0], "65", 2))
		{
		        sprintf(szLine3, "%s", "超過次數上限");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0012");
		}
                else 
		{
		        sprintf(szLine3, "%s", "交易失敗");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
		}
		
		sprintf(szOther, "%s", pobTran->srTRec.srECCRec.szEZ_RespCode);
        }
        else if (!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-110", 4))
        {
        	sprintf(szLine3, "%s", "網路傳送逾時");
                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
        }	
        else if (!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-111", 4))
        {
        	sprintf(szLine3, "%s", "網路接收逾時");
                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
        }	
        else if (!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-117", 4))
        {
        	sprintf(szLine3, "%s", "請先結帳");
                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
        }	
	else if (!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-119", 4))
        {
        	/* <T3904> */
        	if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "61", 2))
                {
                        if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6101", 4))
                        {
                	        sprintf(szLine3, "%s", "票卡不適用");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                        }
                        else if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6103", 4))
                        {
                	        sprintf(szLine3, "%s", "票卡已失效");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                        }
                        else if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6108", 4))
                        {
                	        sprintf(szLine3, "%s", "票卡過期");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                        }
                        else if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6109", 4))
                        {
                	        sprintf(szLine3, "%s", "票卡已鎖洽悠遊卡");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0013");
                        }
                        else if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "610A", 4))
                        {
                	        sprintf(szLine3, "%s", "檢核碼錯誤");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                        }
                        else
                        {
                	        sprintf(szLine3, "%s", "票卡異常");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                        }
                }
                else if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "62", 2))
                {
                        if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6201", 4))
                        {
                	        sprintf(szLine3, "%s", "找不到卡片");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                        }
                        else if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6202", 4))
                        {
                	        sprintf(szLine3, "%s", "讀卡失敗");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                        }
                        else if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6203", 4))
                        {
                	        sprintf(szLine3, "%s", "寫卡失敗");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                        }
                        else if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6204", 4))
                        {
                	        sprintf(szLine3, "%s", "多張票卡讀取失敗");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0017");
                        }
                        else
                        {
                	        sprintf(szLine3, "%s", "交易失敗");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                        }
                }
                else if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "64", 2))
                {
                        if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6401", 4))
                        {
                	        sprintf(szLine3, "%s", "取消與上一筆不符");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0014");
                        }
                        else if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6402", 4))
                        {
                	        sprintf(szLine3, "%s", "交易金額超過額度");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0012");
                        }
                        else if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6403", 4))
                        {
                	        sprintf(szLine3, "%s", "票卡餘額不足");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0016");
                        }
                        else if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6404", 4))
                        {
                	        sprintf(szLine3, "%s", "卡號錯誤");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                        }
                        else if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6406", 4))
                        {
                	        sprintf(szLine3, "%s", "票卡異常");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                        }
                        else if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6409", 4))
                        {
                	        sprintf(szLine3, "%s", "自動加值未啟用");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0015");
                        }
                        else if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "640A", 4))
                        {
                	        sprintf(szLine3, "%s", "自動加值金額為零");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                        }
                        else if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "640C", 4))
                        {
                	        sprintf(szLine3, "%s", "超過交易日限額");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                        }
                        else if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "640D", 4))
                        {
                	        sprintf(szLine3, "%s", "超過交易次限額");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                        }
                        else if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "640E", 4))
                        {
                	        sprintf(szLine3, "%s", "票卡餘額異常");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                        }
                        else if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6410", 4) ||
                        	 !memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6411", 4) ||
                        	 !memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6412", 4) ||
                        	 !memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6413", 4) ||
                        	 !memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6414", 4) ||
                        	 !memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6417", 4) ||
                        	 !memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6419", 4))
                        {
                	        sprintf(szLine3, "%s", "不適用於此交易");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                        }
                        else if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6416", 4))
                        {
                	        sprintf(szLine3, "%s", "自動加值已開啟");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                        }
                        else if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6418", 4))
                        {
                	        sprintf(szLine3, "%s", "票卡限制使用");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                        }
                        else if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "641D", 4))
                        {
                	        sprintf(szLine3, "%s", "超過交易日限次");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                        }
                        else
                        {
                	        sprintf(szLine3, "%s", "交易失敗");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                        }
                }
                else
                {
        	        sprintf(szLine3, "%s", "交易失敗");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                }

                sprintf(szOther, "%s", pobTran->srTRec.srECCRec.szReader_RespCode);
	}
        else if (!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-120", 4))
        {
        	sprintf(szLine3, "%s", "網路傳送逾時");
                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
        }	
        else if (!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-121", 4))
        {
        	sprintf(szLine3, "%s", "網路接收逾時");
                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
        }
        else if (!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-123", 4))
        {
	        sprintf(szLine3, "%s", "票卡餘額不足");
                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0016");
	}
	else if (!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-125", 4))
        {
        	/* 後面檢查 */
	        sprintf(szLine3, "%s", "請重新感應卡片");
                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
	}
        else if (!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-130", 4))
        {
	        sprintf(szLine3, "%s", "票卡已退");
                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
	}
	else if (!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-131", 4))
        {
	        sprintf(szLine3, "%s", "卡別錯誤");
                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
	}
	else if (!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-132", 4))
        {
	        sprintf(szLine3, "%s", "票卡異常");
                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0013");
	}
	else if (!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-133", 4))
        {
	        sprintf(szLine3, "%s", "票卡已鎖洽悠遊卡");
                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0013");
	}
        else if (!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-134", 4))
        {
	        sprintf(szLine3, "%s", "票卡未開卡");
                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
	}
	else if (!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-138", 4))
        {
        	/* <T3908> */
                if (!memcmp(&pobTran->srTRec.szRespCode[0], "XT", 2))
                {
                        sprintf(szLine3, "%s", "無法比對原交易");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0014");
                }
                else
                {
        	        sprintf(szLine3, "%s", "交易失敗");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                }
	}
	else if (!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-139", 4))
        {
	        sprintf(szLine3, "%s", "卡片已展期或設定");
                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
	}
	else
	{
	        sprintf(szLine3, "%s", "交易失敗");
                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
        }
	
	if (strlen(szOther) > 0)
	{
		if (!memcmp(&pobTran->srTRec.szRespCode[0], "XT", 2))
		{
	                sprintf(szLine1, "[%s][%s]", pobTran->srTRec.szRespCode, szOther);
			sprintf(pobTran->szTicket_ErrorCode, "%s", "E0014");
		}
		else
		{
			sprintf(szLine1, "[%s][%s]", pobTran->srTRec.srECCRec.szAPI_RespCode, szOther);
		}
	}
        else
	{
		if (!memcmp(&pobTran->srTRec.szRespCode[0], "XT", 2))
		{
			sprintf(szLine1, "[%s]", pobTran->srTRec.szRespCode);
			sprintf(pobTran->szTicket_ErrorCode, "%s", "E0014");
		}
		else
		{
			sprintf(szLine1, "[%s]", pobTran->srTRec.srECCRec.szAPI_RespCode);
		}
	}
	
	DISPLAY_OBJECT	srDispMsgObj;
	memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
	strcpy(srDispMsgObj.szDispPic1Name, "");
	srDispMsgObj.inDispPic1YPosition = 0;
	srDispMsgObj.inMsgType = _ANY_KEY_MSG_;
	
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)     ||
            !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_))  
        {
                srDispMsgObj.inTimeout = _CUSTOMER_105_MCDONALDS_DISPLAY_TIMEOUT_;
        }
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)	||
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
	srDispMsgObj.inMsgPosition1 = _MSG_BMP_DISP_POSITION_CENTER_;
	strcpy(srDispMsgObj.szErrMsg2, szLine2);
	srDispMsgObj.inErrMsg2Line = _LINE_8_5_;
	srDispMsgObj.inMsgPosition2 = _MSG_BMP_DISP_POSITION_CENTER_;
	strcpy(srDispMsgObj.szErrMsg3, szLine3);
	srDispMsgObj.inErrMsg3Line = _LINE_8_6_;
	srDispMsgObj.inMsgPosition3 = _MSG_BMP_DISP_POSITION_CENTER_;
	srDispMsgObj.inBeepTimes = 3;
	srDispMsgObj.inBeepInterval = 500;
	
	inDISP_Msg_BMP(&srDispMsgObj);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inECC_ResponeCode() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);        
}

/*
Function        :inECC_ResponeCode_MCDONALDS
Date&Time       :2022/11/16 下午 3:26
Describe        :
*/
int inECC_ResponeCode_MCDONALDS(TRANSACTION_OBJECT *pobTran)
{
	int	inRespCodeLen = 0;
        char    szLine1[66 + 1] = {0}, szLine2[66 + 1] = {0}, szLine3[66 + 1] = {0};
	char	szOther[10 + 1] = {0};
	char	szFuncEnable[2 + 1] = {0};
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inECC_ResponeCode_MCDONALDS() START !");
	}
	
	memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
	inGetIntegrate_Device(szFuncEnable);
	if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
	{

	}
	else
	{
		inCTLS_Set_LED(_CTLS_LIGHT_RED_);
	}
	
        memset(szLine1, 0x00, sizeof(szLine1)); 
        memset(szLine2, 0x00, sizeof(szLine2)); 
        memset(szLine3, 0x00, sizeof(szLine3)); 
        inRespCodeLen = atoi(pobTran->srTRec.srECCRec.szAPI_RespCode);
	
	/* <T3901> */
        if (!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-101", 4))
        {
                /* <T3900> */
                if (!memcmp(&pobTran->srTRec.srECCRec.szICER_RespCode[0], "06", 2))
		{
			/* <T3903> */
			if (!memcmp(&pobTran->srTRec.srECCRec.szSVCS_RespCode[0], "04", 2) ||
			    !memcmp(&pobTran->srTRec.srECCRec.szSVCS_RespCode[0], "05", 2) ||
			    !memcmp(&pobTran->srTRec.srECCRec.szSVCS_RespCode[0], "N3", 2) ||
			    !memcmp(&pobTran->srTRec.srECCRec.szSVCS_RespCode[0], "N4", 2))
			{
				sprintf(szLine3, "%s", "票卡異常");
	                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
			}
			else if (!memcmp(&pobTran->srTRec.srECCRec.szSVCS_RespCode[0], "13", 2))
			{
				sprintf(szLine3, "%s", "請確認交易金額");
	                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
			}
			else if (!memcmp(&pobTran->srTRec.srECCRec.szSVCS_RespCode[0], "19", 2))
			{
				sprintf(szLine3, "%s", "交易重複");
	                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
			}
			else if (!memcmp(&pobTran->srTRec.srECCRec.szSVCS_RespCode[0], "51", 2) ||
			         !memcmp(&pobTran->srTRec.srECCRec.szSVCS_RespCode[0], "57", 2) ||
			         !memcmp(&pobTran->srTRec.srECCRec.szSVCS_RespCode[0], "Q1", 2) ||
			         !memcmp(&pobTran->srTRec.srECCRec.szSVCS_RespCode[0], "C0", 2))
			{
				sprintf(szLine3, "%s", "請與發卡銀行確認");
	                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0002");
			}
			else if (!memcmp(&pobTran->srTRec.srECCRec.szSVCS_RespCode[0], "54", 2))
			{
				sprintf(szLine3, "%s", "卡片過期");
	                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
			}
			else if (!memcmp(&pobTran->srTRec.srECCRec.szSVCS_RespCode[0], "55", 2))
			{
				sprintf(szLine3, "%s", "無需展期");
	                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
			}
	                else
			{
			        sprintf(szLine3, "%s", "交易失敗");
	                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
			}
			
			sprintf(szLine2, "[T3903 = %s]", pobTran->srTRec.srECCRec.szSVCS_RespCode);
		}
                else if (!memcmp(&pobTran->srTRec.srECCRec.szICER_RespCode[0], "13", 2))
		{
		        sprintf(szLine3, "%s", "非實名卡");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
		}
                else if (!memcmp(&pobTran->srTRec.srECCRec.szICER_RespCode[0], "14", 2))
		{
		        sprintf(szLine3, "%s", "卡號不合法");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
		}
		else if (!memcmp(&pobTran->srTRec.srECCRec.szICER_RespCode[0], "17", 2))
		{
		        sprintf(szLine3, "%s", "超過交易次限額");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
		}
		else if (!memcmp(&pobTran->srTRec.srECCRec.szICER_RespCode[0], "19", 2))
		{
		        sprintf(szLine3, "%s", "交易重複");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
		}
		else if (!memcmp(&pobTran->srTRec.srECCRec.szICER_RespCode[0], "60", 2))
		{
		        sprintf(szLine3, "%s", "配對失敗");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
		}
		else if (!memcmp(&pobTran->srTRec.srECCRec.szICER_RespCode[0], "76", 2))
		{
		        sprintf(szLine3, "%s", "交易失敗");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0013");
		}
                else
		{
		        sprintf(szLine3, "%s", "交易失敗");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
		}

		sprintf(szOther, "%s", pobTran->srTRec.srECCRec.szICER_RespCode);
        }
        else if (!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-102", 4))
        {
                /* <T3902> */
                if (!memcmp(&pobTran->srTRec.srECCRec.szEZ_RespCode[0], "13", 2))
		{
		        sprintf(szLine3, "%s", "請確認交易金額");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
		}
                else if (!memcmp(&pobTran->srTRec.srECCRec.szEZ_RespCode[0], "14", 2))
		{
		        sprintf(szLine3, "%s", "票卡異常");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
		}
                else if (!memcmp(&pobTran->srTRec.srECCRec.szEZ_RespCode[0], "19", 2))
		{
		        sprintf(szLine3, "%s", "交易重複");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
		}
                else if (!memcmp(&pobTran->srTRec.srECCRec.szEZ_RespCode[0], "51", 2))
		{
		        sprintf(szLine3, "%s", "請與發卡銀行確認");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0002");
		}
                else if (!memcmp(&pobTran->srTRec.srECCRec.szEZ_RespCode[0], "54", 2))
		{
		        sprintf(szLine3, "%s", "票卡異常");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
		}
                else if (!memcmp(&pobTran->srTRec.srECCRec.szEZ_RespCode[0], "55", 2))
		{
		        sprintf(szLine3, "%s", "無需展期");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
		}
                else if (!memcmp(&pobTran->srTRec.srECCRec.szEZ_RespCode[0], "57", 2))
		{
		        sprintf(szLine3, "%s", "請與發卡銀行確認");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0002");
		}
                else if (!memcmp(&pobTran->srTRec.srECCRec.szEZ_RespCode[0], "61", 2))
		{
		        sprintf(szLine3, "%s", "超過金額上限");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0011");
		}
                else if (!memcmp(&pobTran->srTRec.srECCRec.szEZ_RespCode[0], "65", 2))
		{
		        sprintf(szLine3, "%s", "超過次數上限");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0011");
		}
                else 
		{
		        sprintf(szLine3, "%s", "交易失敗");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
		}
		
		sprintf(szOther, "%s", pobTran->srTRec.srECCRec.szEZ_RespCode);
        }
        else if (!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-110", 4))
        {
        	sprintf(szLine3, "%s", "網路傳送逾時");
                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
        }	
        else if (!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-111", 4))
        {
        	sprintf(szLine3, "%s", "網路接收逾時");
                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
        }	
        else if (!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-117", 4))
        {
        	sprintf(szLine3, "%s", "請先結帳");
                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
        }	
	else if (!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-119", 4))
        {
        	/* <T3904> */
        	if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "61", 2))
                {
                        if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6101", 4))
                        {
                	        sprintf(szLine3, "%s", "票卡不適用");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                        }
                        else if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6103", 4))
                        {
                	        sprintf(szLine3, "%s", "票卡已失效");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                        }
                        else if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6108", 4))
                        {
                	        sprintf(szLine3, "%s", "票卡過期");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                        }
                        else if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6109", 4))
                        {
                	        sprintf(szLine3, "%s", "票卡已鎖洽悠遊卡");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0012");
                        }
                        else if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "610A", 4))
                        {
                	        sprintf(szLine3, "%s", "檢核碼錯誤");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                        }
                        else
                        {
                	        sprintf(szLine3, "%s", "票卡異常");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                        }
                }
                else if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "62", 2))
                {
                        if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6201", 4))
                        {
                	        sprintf(szLine3, "%s", "找不到卡片");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                        }
                        else if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6202", 4))
                        {
                	        sprintf(szLine3, "%s", "讀卡失敗");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                        }
                        else if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6203", 4))
                        {
                	        sprintf(szLine3, "%s", "寫卡失敗");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                        }
                        else if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6204", 4))
                        {
                	        sprintf(szLine3, "%s", "多張票卡讀取失敗");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0016");
                        }
                        else
                        {
                	        sprintf(szLine3, "%s", "交易失敗");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                        }
                }
                else if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "64", 2))
                {
                        if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6401", 4))
                        {
                	        sprintf(szLine3, "%s", "取消與上一筆不符");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0013");
                        }
                        else if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6402", 4))
                        {
                	        sprintf(szLine3, "%s", "交易金額超過額度");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0011");
                        }
                        else if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6403", 4))
                        {
                	        sprintf(szLine3, "%s", "票卡餘額不足");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0015");
                        }
                        else if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6404", 4))
                        {
                	        sprintf(szLine3, "%s", "卡號錯誤");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                        }
                        else if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6406", 4))
                        {
                	        sprintf(szLine3, "%s", "票卡異常");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                        }
                        else if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6409", 4))
                        {
                	        sprintf(szLine3, "%s", "自動加值未啟用");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0014");
                        }
                        else if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "640A", 4))
                        {
                	        sprintf(szLine3, "%s", "自動加值金額為零");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                        }
                        else if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "640C", 4))
                        {
                	        sprintf(szLine3, "%s", "超過交易日限額");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                        }
                        else if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "640D", 4))
                        {
                	        sprintf(szLine3, "%s", "超過交易次限額");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                        }
                        else if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "640E", 4))
                        {
                	        sprintf(szLine3, "%s", "票卡餘額異常");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                        }
                        else if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6410", 4) ||
                        	 !memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6411", 4) ||
                        	 !memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6412", 4) ||
                        	 !memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6413", 4) ||
                        	 !memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6414", 4) ||
                        	 !memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6417", 4) ||
                        	 !memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6419", 4))
                        {
                	        sprintf(szLine3, "%s", "不適用於此交易");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                        }
                        else if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6416", 4))
                        {
                	        sprintf(szLine3, "%s", "自動加值已開啟");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                        }
                        else if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "6418", 4))
                        {
                	        sprintf(szLine3, "%s", "票卡限制使用");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                        }
                        else if (!memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "641D", 4))
                        {
                	        sprintf(szLine3, "%s", "超過交易日限次");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                        }
                        else
                        {
                	        sprintf(szLine3, "%s", "交易失敗");
                                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                        }
                }
                else
                {
        	        sprintf(szLine3, "%s", "交易失敗");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                }

                sprintf(szOther, "%s", pobTran->srTRec.srECCRec.szReader_RespCode);
	}
        else if (!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-120", 4))
        {
        	sprintf(szLine3, "%s", "網路傳送逾時");
                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
        }	
        else if (!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-121", 4))
        {
        	sprintf(szLine3, "%s", "網路接收逾時");
                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
        }
        else if (!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-123", 4))
        {
	        sprintf(szLine3, "%s", "票卡餘額不足");
                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0015");
	}
	else if (!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-125", 4))
        {
        	/* 後面檢查 */
	        sprintf(szLine3, "%s", "請重新感應卡片");
                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
	}
        else if (!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-130", 4))
        {
	        sprintf(szLine3, "%s", "票卡已退");
                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
	}
	else if (!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-131", 4))
        {
	        sprintf(szLine3, "%s", "卡別錯誤");
                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
	}
	else if (!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-132", 4))
        {
	        sprintf(szLine3, "%s", "票卡異常");
                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0012");
	}
	else if (!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-133", 4))
        {
	        sprintf(szLine3, "%s", "票卡已鎖洽悠遊卡");
                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0012");
	}
        else if (!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-134", 4))
        {
	        sprintf(szLine3, "%s", "票卡未開卡");
                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
	}
	else if (!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-138", 4))
        {
        	/* <T3908> */
                if (!memcmp(&pobTran->srTRec.szRespCode[0], "XT", 2))
                {
                        sprintf(szLine3, "%s", "無法比對原交易");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0013");
                }
                else
                {
        	        sprintf(szLine3, "%s", "交易失敗");
                        sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
                }
	}
	else if (!memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-139", 4))
        {
	        sprintf(szLine3, "%s", "卡片已展期或設定");
                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
	}
	else
	{
	        sprintf(szLine3, "%s", "交易失敗");
                sprintf(pobTran->szTicket_ErrorCode, "%s", "E0001");
        }
	
	if (strlen(szOther) > 0)
	{
		if (!memcmp(&pobTran->srTRec.szRespCode[0], "XT", 2))
		{
			sprintf(szLine1, "[%s][%s]", pobTran->srTRec.szRespCode, szOther);
			sprintf(pobTran->szTicket_ErrorCode, "%s", "E0013");
		}
		else
		{
			sprintf(szLine1, "[%s][%s]", pobTran->srTRec.srECCRec.szAPI_RespCode, szOther);
		}
	}
        else
	{
		if (!memcmp(&pobTran->srTRec.szRespCode[0], "XT", 2))
		{
			sprintf(szLine1, "[%s]", pobTran->srTRec.szRespCode);
			sprintf(pobTran->szTicket_ErrorCode, "%s", "E0013");
		}
		else
		{
			sprintf(szLine1, "[%s]", pobTran->srTRec.srECCRec.szAPI_RespCode);
		}
	}
	
	DISPLAY_OBJECT	srDispMsgObj;
	memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
	strcpy(srDispMsgObj.szDispPic1Name, "");
	srDispMsgObj.inDispPic1YPosition = 0;
	srDispMsgObj.inMsgType = _ANY_KEY_MSG_;
	srDispMsgObj.inTimeout = _CUSTOMER_105_MCDONALDS_DISPLAY_TIMEOUT_;
        
	strcpy(srDispMsgObj.szErrMsg1, szLine1);
	srDispMsgObj.inErrMsg1Line = _LINE_8_4_;
	srDispMsgObj.inMsgPosition1 = _MSG_BMP_DISP_POSITION_CENTER_;
	strcpy(srDispMsgObj.szErrMsg2, szLine2);
	srDispMsgObj.inErrMsg2Line = _LINE_8_5_;
	srDispMsgObj.inMsgPosition2 = _MSG_BMP_DISP_POSITION_CENTER_;
	strcpy(srDispMsgObj.szErrMsg3, szLine3);
	srDispMsgObj.inErrMsg3Line = _LINE_8_6_;
	srDispMsgObj.inMsgPosition3 = _MSG_BMP_DISP_POSITION_CENTER_;
	srDispMsgObj.inBeepTimes = 3;
	srDispMsgObj.inBeepInterval = 500;
	
	inDISP_Msg_BMP(&srDispMsgObj);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inECC_ResponeCode_MCDONALDS() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);        
}

/*
Function        :inECC_APIflow
Date&Time       :2018/3/28 下午 4:38
Describe        :
*/
int inECC_APIflow(TRANSACTION_OBJECT *pobTran)
{
        int     inRetVal = VS_ERROR;
	int	inRetVal2 = VS_ERROR;
	int	inOrgCode = _TRANS_TYPE_NULL_;
	char	szDebugMsg[100 + 1] = {0};
	char	szDemoMode[2 + 1] = {0};
	char	szTxnType[20 + 1] = {0};
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inECC_APIflow() START !");
	}
	
	/* 刪LOG，
	 * 為了穩固性把流程加在這裡
	 * 會速度慢也是理所當然的。 */
	inECC_Delete_Log(5 * 1024 * 1024, 1);
	
	inDISP_ClearAll();
	inCTLS_Set_LED(_CTLS_LIGHT_YELLOW_);
	
	inDISP_ChineseFont("交易進行中", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
	inDISP_ChineseFont("請勿移動票卡", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
	
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
        {
                memset(pobTran->srTRec.szUID, 0x00, sizeof(pobTran->srTRec.szUID));
                memcpy(&pobTran->srTRec.szUID[0], "B123456789", 10);

                pobTran->srTRec.lnCardRemainAmount = gulDemoTicketPoint;
                pobTran->srTRec.lnFinalBeforeAmt = gulDemoTicketPoint;
                pobTran->srTRec.lnTopUpAmount = 500;			/* 自動加值金額(元/次) */
		
		/* 悠遊卡簽單有RRN及自己的Date Time */
		memcpy(&pobTran->srTRec.srECCRec.szRRN[0], &pobTran->srTRec.szRefNo[0], 12);
		strcpy(pobTran->srTRec.srECCRec.szDate, "20");
		memcpy(&pobTran->srTRec.srECCRec.szDate[2], &pobTran->srTRec.szDate[0], 6);
		memcpy(&pobTran->srTRec.srECCRec.szTime[0], &pobTran->srTRec.szTime[0], 6);

                if (pobTran->srTRec.inCode == _TICKET_EASYCARD_DEDUCT_)
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
						inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
						inDISP_ChineseFont("餘額不足", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);  
						inDISP_BEEP(3, 500);

						if (ginDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
							sprintf(szDebugMsg, "inNCCC_Ticket_IPASS_Query_Flow()_自動加值金額過大");
							inLogPrintf(AT, szDebugMsg);
						}
						inRetVal = VS_ERROR;
                                	}

					if (inRetVal == VS_SUCCESS)
					{
						/* 負值不可交易 */
						inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
						inDISP_ChineseFont("餘額不足", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);  
						inDISP_ChineseFont("進行自動加值", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_); 

						pobTran->inTransactionCode = _TICKET_EASYCARD_AUTO_TOP_UP_;
						pobTran->srTRec.inCode = _TICKET_EASYCARD_AUTO_TOP_UP_;

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
						pobTran->inTransactionCode = _TICKET_EASYCARD_DEDUCT_;
						pobTran->srTRec.inCode = _TICKET_EASYCARD_DEDUCT_;

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
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_ChineseFont("餘額不足", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);  
					inDISP_BEEP(3, 500);
                        	        inRetVal = VS_ERROR;
                        	}
                        }

                        memset(pobTran->srTRec.szTicketRefundCode, 0x00, sizeof(pobTran->srTRec.szTicketRefundCode));
                        memcpy(&pobTran->srTRec.szTicketRefundCode[0], "333333", 6);
                }
                else if (pobTran->srTRec.inCode == _TICKET_EASYCARD_REFUND_ || pobTran->srTRec.inCode == _TICKET_EASYCARD_TOP_UP_)
                {
                        pobTran->srTRec.lnFinalAfterAmt = pobTran->srTRec.lnFinalBeforeAmt + pobTran->srTRec.lnTxnAmount;
                        gulDemoTicketPoint = pobTran->srTRec.lnFinalAfterAmt;
			inRetVal = VS_SUCCESS;
                }
                else if (pobTran->srTRec.inCode == _TICKET_EASYCARD_VOID_TOP_UP_)
                {
                        if (pobTran->srTRec.lnFinalBeforeAmt - pobTran->srTRec.lnTxnAmount < 0)
                        {
                                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("餘額不足", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);  
				inDISP_BEEP(3, 500);
                	        inRetVal = VS_ERROR;
                        }
                        else
                        {
                                pobTran->srTRec.lnFinalAfterAmt = pobTran->srTRec.lnFinalBeforeAmt - pobTran->srTRec.lnTxnAmount;
                                gulDemoTicketPoint = pobTran->srTRec.lnFinalAfterAmt;
				inRetVal = VS_SUCCESS;
                        }
                }
		else if (pobTran->srTRec.inCode == _TICKET_EASYCARD_INQUIRY_)
		{
			inRetVal = VS_SUCCESS;
		}
        }
        else
	{
		if (pobTran->srTRec.inCode == _TICKET_EASYCARD_DEDUCT_)
		{
			pobTran->srTRec.lnTotalTopUpAmount = 0;

			inRetVal = inECC_Deduct_Flow(pobTran);
			if (inRetVal == VS_SUCCESS)
			{
				/* 購貨成功 */
				pobTran->uszDeductSuccessBit = VS_TRUE;
			}

			/* 有可能購貨失敗但自動加值成功，所以不能在這裡跳出 */
			/* 檢核是否有自動加值 */
			if (pobTran->srTRec.lnTotalTopUpAmount > 0)
			{
				/* 存原交易別 */
				inOrgCode = pobTran->srTRec.inCode;
				inRetVal2 = inECC_AutoAdd_Flow(pobTran);
				/* 回復原交易別 */
				pobTran->srTRec.inCode = inOrgCode;
				if (inRetVal2 != VS_SUCCESS)
				{
					inRetVal = VS_ERROR;
				}
				else
				{
					/* 自動加值成功 */
					pobTran->uszAutoTopUpSuccessBit = VS_TRUE;
				}
			}

		}
		else if (pobTran->srTRec.inCode == _TICKET_EASYCARD_REFUND_)
		{
			inRetVal = inECC_Refund_Flow(pobTran);   
		}
		else if (pobTran->srTRec.inCode == _TICKET_EASYCARD_TOP_UP_)
		{
			inRetVal = inECC_TOP_UP_Flow(pobTran);
		}
		else if (pobTran->srTRec.inCode == _TICKET_EASYCARD_INQUIRY_)
		{
			inRetVal = inECC_Inquiry_Flow(pobTran);
		}
		else if (pobTran->srTRec.inCode == _TICKET_EASYCARD_VOID_TOP_UP_)
		{
			inRetVal = inECC_Void_TOP_UP_Flow(pobTran);   
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "無對應ECC incode: %d", pobTran->srTRec.inCode);
				inLogPrintf(AT, szDebugMsg);
			}

			inRetVal = VS_ERROR;
		}
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inECC_APIflow() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	if (inRetVal == VS_SUCCESS)
	{
		if (pobTran->srTRec.inCode == _TICKET_EASYCARD_INQUIRY_)
		{
			/* 和其他票證同步（詢卡不嗶 */
		}
		else
		{
			/* 成功 */
			inNCCC_Ticket_Trans_Success_Beep(pobTran);
		}
		
		return (VS_SUCCESS);
	}
	else
	{
		return (VS_ERROR);
	}
}

/*
Function        :inECC_AutoAdd_Flow
Date&Time       :2018/3/30 下午 5:36
Describe        :
*/
int inECC_AutoAdd_Flow(TRANSACTION_OBJECT *pobTran)
{
        int     inType = 0;
        long    lnAmt = 0l;
        char    szTemplate[7] = {0};
	char	szInvNum[6 + 1] = {0};
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inECC_AutoAdd_Flow() START !");
	}
        
        inType = pobTran->srTRec.inCode;
        pobTran->srTRec.inCode = _TICKET_EASYCARD_AUTO_TOP_UP_;
        
        /* 金額要調整成自動加值的 */
        lnAmt = pobTran->srTRec.lnTxnAmount;
        pobTran->srTRec.lnTxnAmount = pobTran->srTRec.lnTotalTopUpAmount;

        /* AuthCode要調整成自動加值的 */
        memset(szTemplate, 0x00, sizeof(szTemplate)); 
        memcpy(&szTemplate[0], &pobTran->srTRec.szAuthCode[0], 6);
        memset(pobTran->srTRec.szAuthCode, 0x00, sizeof(pobTran->srTRec.szAuthCode)); 
        memcpy(&pobTran->srTRec.szAuthCode[0], &pobTran->srTRec.szECCAuthCode[0], 6);
	         
	if (inFLOW_RunFunction(pobTran, _TICKET_UPDATE_ACCUM_) != VS_SUCCESS)
                return (VS_ERROR);
        
	if (inFLOW_RunFunction(pobTran, _TICKET_UPDATE_BATCH_) != VS_SUCCESS)
                return (VS_ERROR);
	
	if (inFLOW_RunFunction(pobTran, _TICKET_UPDATE_INV_) != VS_SUCCESS)
                return (VS_ERROR);
	
        /* 還原成購貨 */
        pobTran->srTRec.inCode = inType;
        pobTran->srTRec.lnTxnAmount = lnAmt;
        
        memset(pobTran->srTRec.szAuthCode, 0x00, sizeof(pobTran->srTRec.szAuthCode)); 
        memcpy(&pobTran->srTRec.szAuthCode[0], &szTemplate[0], 6);
        
        /* 購貨的簽單序號要在自動加值後加一號，上面已累加簽單調閱編號，所以直接再取出 */
	memset(szInvNum, 0x00, sizeof(szInvNum));
	inGetInvoiceNum(szInvNum);
        pobTran->srTRec.lnInvNum = atol(szInvNum);
	
        /* RRN - 購貨會帶成n，要重組成n+1 */
        inNCCC_Ticket_Func_MakeRefNo(pobTran);
	
	/*
	 * T5501批次號碼之規則為yymmddss+流水號兩碼
	 * 以當批第一筆成功交易時間為準
	 * 如果NeedNewBatch是'Y'，代表是第一筆，更新狀態為不用更新批號
	 * 如果NeedNewBatch是'N'，代表有做過交易
	*/
	inECC_Batch_Update();
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inECC_AutoAdd_Flow() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
                 
        return (VS_SUCCESS);             
}

/*
Function        :inECC_Deduct_Flow
Date&Time       :2018/3/30 上午 11:38
Describe        :
*/
int inECC_Deduct_Flow(TRANSACTION_OBJECT *pobTran)
{
     	int		inRetVal = VS_ERROR;
	int		inRetry = 1;
	char		szKey = 0x00;
	char		szCustomerIndicator[3 + 1] = {0};
	char		szDebugMsg[100 + 1] = {0};
	unsigned char	uszECRAlreadyBit = VS_FALSE;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inECC_Deduct_Flow() START !");
	}
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	/* 初始化是否已重試 */
	guszECCRetryBit = VS_FALSE;
	
	while (inRetry <= 5)
	{
		/* (需求單 - 107227)邦柏科技自助作業客製化 by Russell 2018/12/13 下午 8:33 */
		/* (需求單 - 107276)自助交易標準400做法 by Russell 2018/12/27 上午 11:20 */
		if ((!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)			||
		     !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_)		||
                     !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)		||
		     !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)		||
		     !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)			||
		     !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))	&&
		     pobTran->uszLastTranscationBit == VS_TRUE)
		{
			uszECRAlreadyBit = VS_TRUE;
			inECC_PACK_RETRY_ICERAPI(pobTran);
		}
		else
		{
			/* 組電文(生成ICERAPI.REQ) */
			inECC_PACK_Deduct_ICERAPI(pobTran);
		}
	        
	        /* AdditionalTcpipData */
	        inECC_PACK_Deduct_ICERAPI_NCCC(pobTran);
		
                if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "ICER Start");
			inLogPrintf(AT, szDebugMsg);
		}
		
		inRetVal = ICERApi_exe();
		
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "ICER End");
			inLogPrintf(AT, szDebugMsg);
		}
		
		/* 解電文(分析ICERAPI.RES) */
		inRetVal = inECC_UNPACK_ICERAPI(pobTran, _ECC_API_RES_FILE_);	
		
		if (inRetVal != VS_SUCCESS)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inCTLS_Set_LED(_CTLS_LIGHT_RED_);
                        inDISP_ChineseFont("悠遊卡檔案異常", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
                        inDISP_BEEP(3, 1000);
			
                        return (VS_ERROR);
                }
		
		/* 檢核回應碼 <T3901>開始 */
	        if (memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "0", 1) != 0)
	        {
			if (!memcmp(pobTran->srTRec.szRespCode, "A0", 2)	||
			    !memcmp(pobTran->srTRec.szRespCode, "A1", 2))
			{
				ginMacError++;
				inECC_UNPACK_ICERAPI_NCCC(pobTran, _ECC_API_RES_FILE2_, _ECC_FOLDER_PATH_);
			}
			
			/* (需求單 - 107227)邦柏科技自助作業客製化 by Russell 2018/12/9 下午 2:42 */
			/* (需求單 - 107276)自助交易標準400做法 by Russell 2018/12/27 上午 11:20 */
			if ((!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)		||
			     !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_)	||
			     !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)		||
			     !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))	&&
			     pobTran->uszECRBit == VS_TRUE)
			{
				/* 第二次發生RETRY 回傳失敗 */
	        		if (uszECRAlreadyBit == VS_TRUE)
				{
	        			return (VS_ERROR);
				}
				
				if (memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-125", 4) == 0)
				{
	                                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_ChineseFont("請重新交易", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
					inDISP_BEEP(3, 1000);

	                                /* 要回錯誤碼0018不是0001 所以Reset */
	                                memset(pobTran->szTicket_ErrorCode, 0x00, sizeof(pobTran->szTicket_ErrorCode));
					pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_ESVC_RE_TAP_CARD_;

					/* 將現在組好的REQ 備份成ICERRETRY.REQ */
					inFunc_Data_Copy(_ECC_API_REQ_FILE_, _ECC_FOLDER_PATH_, _ECC_API_REQ_RETRY_FILE_, _ECC_FOLDER_PATH_);

//					/* 將pobTran的MFRec備份部分資料 */
//					inECC_SAVE_TREC(pobTran);
                                        
	                                return (VS_ERROR);
	                        }
				else
				{	
					/* 其他錯誤Display */
					inECC_ResponeCode(pobTran);

					return (VS_ERROR);
				}
			}
                        else if ((!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)       ||
			          !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_))	&&
			         pobTran->uszECRBit == VS_TRUE)
			{
				/* 第二次發生RETRY 回傳失敗 */
	        		if (uszECRAlreadyBit == VS_TRUE)
				{
	        			return (VS_ERROR);
				}
				
				if (memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-125", 4) == 0)
				{
	                                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_ChineseFont("請重新交易", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
					inDISP_BEEP(3, 1000);

	                                /* 要回錯誤碼0018不是0001 所以Reset */
	                                memset(pobTran->szTicket_ErrorCode, 0x00, sizeof(pobTran->szTicket_ErrorCode));
					pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_ESVC_RE_TAP_CARD_;

					/* 將現在組好的REQ 備份成ICERRETRY.REQ */
					inFunc_Data_Copy(_ECC_API_REQ_FILE_, _ECC_FOLDER_PATH_, _ECC_API_REQ_RETRY_FILE_, _ECC_FOLDER_PATH_);

//					/* 將pobTran的MFRec備份部分資料 */
//					inECC_SAVE_TREC(pobTran);
                                        
                                        pobTran->uszECCRetryBit = VS_TRUE;
                                        
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "ECC Retry....");
                                        }
                                        
	                                return (VS_ERROR);
	                        }
				else
				{	
					/* 其他錯誤Display */
					inECC_ResponeCode(pobTran);

					return (VS_ERROR);
				}
			}
			else
			{
				/* -125 = Retry */
				/* -119 : Call相關DLL回應錯誤 ; 請參考T3904 */
				/* 62 : RC531錯誤，交易中止，請重新交易*/
				if ((memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-125", 4) == 0)	||
				    (memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-119", 4) == 0	&&
				     memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "62", 2) == 0	&&
				     guszECCRetryBit == VS_TRUE))
				{
					if (inRetry >= 4)
					{
						inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
						inDISP_ChineseFont("請重新交易", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
						inDISP_BEEP(3, 1000);
						return (VS_ERROR);
					}

					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_ChineseFont("連線失敗", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
					inDISP_ChineseFont("請重新感應卡片", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
					inDISP_ChineseFont("並按確認鍵重試", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_CENTER_);

					szKey = 0x00;
					inDISP_Timer_Start(_TIMER_NEXSYS_1_, 60);

					while (1)
					{
						szKey = uszKBD_Key();
						if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
						{
							szKey = _KEY_TIMEOUT_;
						}

						if (szKey == _KEY_ENTER_)
						{
							inRetry++;
							guszECCRetryBit = VS_TRUE;
							break;
						}
						else if (szKey == _KEY_TIMEOUT_)
						{
							return (VS_ERROR);
						}
					}

					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_ChineseFont("交易處理中", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
					inDISP_ChineseFont("請重新感應卡片", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_CENTER_);

					continue;
				}
				else
				{	
					/* 其他錯誤Display */
					inECC_ResponeCode(pobTran);

					return (VS_ERROR);
				}
			}
                }
		else
		{
			/* 每次unpack(交易，詢卡、查額、SignOn不算)就同步電文序號
			 * 成功交易才更新電文序號 */
			inNCCC_Ticket_Func_UpdatetTicketInvNum(pobTran);
			
			/* 未滿三次，授權成功要重置次數 */
			if (ginMacError < 3)
			{
				ginMacError = 0;
			}
		
		}
                
                /* 抓必要資料 */
		if (inECC_UNPACK_ICERAPI_NCCC(pobTran, _ECC_API_RES_FILE2_, _ECC_FOLDER_PATH_) != VS_SUCCESS)
		        return (VS_ERROR);     
		        
                break;
        }
        
        /*
	 * T5501批次號碼之規則為yymmddss+流水號兩碼
	 * 以當批第一筆成功交易時間為準
	 * 如果NeedNewBatch是'Y'，代表是第一筆，更新狀態為不用更新批號
	 * 如果NeedNewBatch是'N'，代表有做過交易
	*/
	inECC_Batch_Update();

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inECC_Deduct_Flow() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inECC_PACK_Deduct_ICERAPI
Date&Time       :2018/3/28 下午 5:26
Describe        :
*/
int inECC_PACK_Deduct_ICERAPI(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_ERROR;
	int		inESVCIndex = 0;
	char		szTemplate[20 + 1] = {0};
	char		szSTAN[6 + 1] = {0};
	char		szInvoiceNum[6 + 1] = {0};
	RTC_NEXSYS	srRTC = {0};
	xmlDocPtr       srDoc = NULL;
        xmlNodePtr      srRoot_Node = NULL, srTRANS_Node = NULL;
#if 	READER_MANUFACTURERS == LINUX_API
	unsigned long	ulHandle = 0;
#else
	int		inFd = 0;
#endif

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inECC_PACK_Deduct_ICERAPI() START !");
	}
        
        if (inLoadTDTRec(_TDT_INDEX_01_ECC_) < 0)
                return (VS_ERROR);
	
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTicket_HostIndex(szTemplate);
	inESVCIndex = atoi(szTemplate);
        
        if (inLoadHDTRec(inESVCIndex) != VS_SUCCESS)
                return (VS_ERROR);
	if (inLoadHDPTRec(inESVCIndex) != VS_SUCCESS)
                return (VS_ERROR);
                
        if (inLoadECCDTRec(0) != VS_SUCCESS)
	{
                return (VS_ERROR);
	}
        
        /* 刪除原本的ICERAPI */
        inFile_Unlink_File(_ECC_API_REQ_CHECK_, _ECC_FOLDER_PATH_);	/* 檢核用 */
	inFile_Unlink_File(_ECC_API_REQ_FILE_, _ECC_FOLDER_PATH_);
	inFile_Unlink_File(_ECC_API_REQ_FILE2_, _ECC_FOLDER_PATH_);
	inFile_Unlink_File(_ECC_API_RES_FILE_, _ECC_FOLDER_PATH_);	/* 防止用到上次資料結果 */
	/* Retry時需要參考RES2，不能砍 */
	if (guszECCRetryBit == VS_TRUE)
	{
		
	}
	else
	{
		inFile_Unlink_File(_ECC_API_RES_FILE2_, _ECC_FOLDER_PATH_);
	}
	
	/* 建立一個新的檔案 */
        srDoc = xmlNewDoc(BAD_CAST "1.0");
        srRoot_Node = xmlNewNode(NULL, BAD_CAST "TransXML");
        xmlDocSetRootElement (srDoc, srRoot_Node);
 
        /* <TRANS> */
	srTRANS_Node = xmlNewNode(NULL, BAD_CAST "TRANS");
	xmlAddChild(srRoot_Node, srTRANS_Node);

        /* <T0100> */
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T0100", BAD_CAST "0200");

        /* <T0300> */
        xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T0300", BAD_CAST "606100");
        
	/* <T0400> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%lu00", pobTran->srTRec.lnTxnAmount);
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T0400", BAD_CAST szTemplate);

	if (guszECCRetryBit == VS_TRUE)
	{
		/* Retry 要同第一次，除了時間 */
                /* <T1100> : STAN 不知道用在哪，先比照NCCC STAN */
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1100", BAD_CAST gszECCRetryData.szT1100);

		/* <T1101> : CMAS自用，區分上筆交易(會回傳) */
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1101", BAD_CAST gszECCRetryData.szT1101);

		/* <T1102> : NCCC STAN */
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1102", BAD_CAST gszECCRetryData.szT1102);

		/* <T1103> : NCCC RRN(後三碼) */
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1103", BAD_CAST gszECCRetryData.szT1103);

                /* <T1200> : hhmmss */
		memset(&srRTC, 0x00, sizeof(srRTC));
		inFunc_GetSystemDateAndTime(&srRTC);
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%02d%02d%02d", srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1200", BAD_CAST szTemplate);

		/* <T1300> : yyyymmdd */
		/* hhmmss就抓過一次時間了，不用重抓 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "20%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1300", BAD_CAST szTemplate);
	}
	else
	{
		/* For Retry 重試時使用 */
		memset(&gszECCRetryData, 0x00, sizeof(gszECCRetryData));
		
		/* <T1100> : STAN 不知道用在哪，先比照NCCC STAN */
		memset(szSTAN, 0x00, sizeof(szSTAN));
		inGetSTANNum(szSTAN);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1100", BAD_CAST szSTAN);
		
		/* For Retry 重試時使用 */
		memcpy(gszECCRetryData.szT1100, szSTAN, strlen(szSTAN));

		/* <T1101> : CMAS自用，區分上筆交易(會回傳) */
		memset(szSTAN, 0x00, sizeof(szSTAN));
		inGetTicket_STAN(szSTAN);
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%06d", atoi(szSTAN) + 1);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1101", BAD_CAST szTemplate);
		inSetTicket_STAN(szTemplate);
		inSaveTDTRec(_TDT_INDEX_01_ECC_);

		/* For Retry 重試時使用 */
		memcpy(gszECCRetryData.szT1101, szTemplate, strlen(szTemplate));
		
		/* <T1102> : NCCC STAN */
		memset(szSTAN, 0x00, sizeof(szSTAN));
		inGetSTANNum(szSTAN);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1102", BAD_CAST szSTAN);
		
		/* For Retry 重試時使用 */
		memcpy(gszECCRetryData.szT1102, szSTAN, strlen(szSTAN));

		/* <T1103> : NCCC RRN(後三碼) */
		memset(szInvoiceNum, 0x00, sizeof(szInvoiceNum));
		inGetTicket_InvNum(szInvoiceNum);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1103", BAD_CAST szInvoiceNum);
		
		/* For Retry 重試時使用 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%06ld", atol(szInvoiceNum) + 1);
		memcpy(gszECCRetryData.szT1103, szTemplate, strlen(szTemplate));

		/* <T1200> : hhmmss */
		memset(&srRTC, 0x00, sizeof(srRTC));
		inFunc_GetSystemDateAndTime(&srRTC);
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%02d%02d%02d", srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1200", BAD_CAST szTemplate);

		/* <T1300> : yyyymmdd */
		/* hhmmss就抓過一次時間了，不用重抓 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "20%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1300", BAD_CAST szTemplate);
	}

        /* <T4108> : LCD Control Flag 單純顯示畫面用 */
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T4108", BAD_CAST "0");

	/* <T4830> : Read Purse Flag 
	             0：不需同一票卡(default)
		     1：需為同一票卡
	*/
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T4830", BAD_CAST "0");
        
        /* <T5501> : Batch Number - 悠遊卡特有，同一天流水號累加，隔天就要歸1 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inECC_Batch_Check(szTemplate);
        xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T5501", BAD_CAST szTemplate);
        
	/* 儲存xml檔 xml檔現在會自動換行了 */
        inXML_SaveFile(_ECC_API_REQ_FILE_, &srDoc, "utf-8", 1);
	inFile_Move_File(_ECC_API_REQ_FILE_, _AP_ROOT_PATH_, "", _ECC_FOLDER_PATH_);
	
	/* 清空佔用記憶體 */
	inXML_End(&srDoc);
	
	/* 步驟完成，建立檢核擋 */
#if 	READER_MANUFACTURERS == LINUX_API
	inRetVal = inFILE_Create(&ulHandle, (unsigned char*)_ECC_API_REQ_CHECK_);
	/* 不管成功或失敗都要關檔 */
	inFILE_Close(&ulHandle);
	/* 抓到ICERData資料夾 */
	inFile_Move_File(_ECC_API_REQ_CHECK_, _FS_DATA_PATH_, "", _ECC_FOLDER_PATH_);
#else
	inRetVal = inFile_Linux_Create(&inFd, _ECC_API_REQ_CHECK_);
	/* 不管成功或失敗都要關檔 */
	inFile_Linux_Close(inFd);
	/* 會有權限問題，要先改權限 */
	inFunc_Data_Chmod("u+rwx", _ECC_API_REQ_CHECK_, _AP_ROOT_PATH_);
	/* 抓到ICERData資料夾 */
	inFile_Move_File(_ECC_API_REQ_CHECK_, _AP_ROOT_PATH_, "", _ECC_FOLDER_PATH_);
#endif
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inECC_PACK_Deduct_ICERAPI() END !");
		inLogPrintf(AT, "----------------------------------------");
	}

        return (VS_SUCCESS);
}

/*
Function        :inECC_PACK_Deduct_ICERAPI_NCCC
Date&Time       :2018/3/28 下午 6:13
Describe        :
*/
int inECC_PACK_Deduct_ICERAPI_NCCC(TRANSACTION_OBJECT *pobTran)
{
	int		inDataLen = 0;
	int		inTxnCode = 0;
	int		inOrgCode = 0;
	int		inSendLen = 0;
	int		inField32 = 0;
	int		inField48 = 0;
	int		inField62 = 0;
	char		szFESMode[2 + 1] = {0};
	char		szIFESMode[2 + 1] = {0};
	char		szSendPacket[_NCCC_TICKET_ISO_SEND_ + 1] = {0};
	char		szTemplate[500 + 1] = {0};
	xmlDocPtr       srDoc = NULL;
        xmlNodePtr      srRoot_Node = NULL, sr606100_Node = NULL, sr606100adv_Node = NULL, sr606100rev_Node = NULL, sr825799_Node = NULL,  sr825799adv_Node = NULL, sr825799rev_Node = NULL;

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inECC_PACK_Deduct_ICERAPI_NCCC() START !");
	}
        
	memset(szFESMode, 0x00, sizeof(szFESMode));
	inGetNCCCFESMode(szFESMode);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetCloud_MFES(szTemplate);
	memset(szIFESMode, 0x00, sizeof(szIFESMode));
	inGetI_FES_Mode(szIFESMode);
	
        if (memcmp(szFESMode, _NCCC_05_ATS_MODE_, 2) == 0	||
	    memcmp(szFESMode, _NCCC_04_MPAS_MODE_, 2) == 0	||
	   (memcmp(szFESMode, _NCCC_03_MFES_MODE_, 2) == 0	&&
	    memcmp(szTemplate, "Y", strlen("Y")) == 0))
	{
                inField32 = _ECC_F32_LENTH_;
	}
                
        if (strlen(pobTran->srTRec.szStoreID) > 0)
	{
                inField48 = _ECC_F48_LENTH_;
	}
                
        pobTran->srTRec.inTicketType = _TICKET_TYPE_ECC_;
       
        memset(szSendPacket, 0x00, sizeof(szSendPacket));
        inTxnCode = _TICKET_EASYCARD_DEDUCT_;
        pobTran->inISOTxnCode = inTxnCode;
        
	/* 要Load CPT */
	inETHERNET_Begin(pobTran);
	
        /* 組 ISO 電文 */
	ginBeforeIndex = 0;
	if ((inSendLen = inNCCC_TICKET_PackISO(pobTran, (unsigned char *)szSendPacket, inTxnCode)) <= 0)
	{
		/* 上傳不能Show錯誤訊息 */
		pobTran->inErrorMsg = _ERROR_CODE_V3_NONE_;
		return (VS_ISO_PACK_ERR);
	}
	
	/* 回復交易碼 */
	pobTran->inISOTxnCode = pobTran->srTRec.inCode;
	
	/* 沒找到 */
	if (ginBeforeIndex == 0)
	{
	        return (VS_ERROR);
	}

        /* 刪除原本的ICERAPI */
	inFile_Unlink_File(_ECC_API_REQ_FILE2_, _ECC_FOLDER_PATH_);
	
	/* 建立一個新的檔案 */
        srDoc = xmlNewDoc(BAD_CAST "1.0");
        srRoot_Node = xmlNewNode(NULL, BAD_CAST "TransXML");
        xmlDocSetRootElement (srDoc, srRoot_Node);
	
	/* 從既有檔案擴增TAG */
        /* <606100> =================================================================================購貨0100 */
	sr606100_Node = xmlNewNode(NULL, BAD_CAST "606100");
	xmlAddChild(srRoot_Node, sr606100_Node);
        
	/* <BefData> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%s", "0000");
	inFunc_BCD_to_ASCII(&szTemplate[4], (unsigned char*)&szSendPacket[0], ginBeforeIndex);
        xmlNewChild(sr606100_Node, NULL, BAD_CAST "BefData", BAD_CAST szTemplate);
             
        /* <AftData> : 扣掉取代的Len + @@@@@ */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inFunc_BCD_to_ASCII(&szTemplate[0], (unsigned char*)&szSendPacket[ginBeforeIndex + 7], inSendLen - ginBeforeIndex - 7);
        xmlNewChild(sr606100_Node, NULL, BAD_CAST "AftData", BAD_CAST szTemplate);
	inField62 = (inSendLen - ginBeforeIndex - 7) - 6;
	
	/* <F03Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr606100_Node, NULL, BAD_CAST "F03Adr", BAD_CAST szTemplate);
	
	/* <F04Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr606100_Node, NULL, BAD_CAST "F04Adr", BAD_CAST szTemplate);
        
	/* <F11Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr606100_Node, NULL, BAD_CAST "F11Adr", BAD_CAST szTemplate);
	
	/* <F35Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F22_LENTH_ + _ECC_F24_LENTH_ +
                    _ECC_F25_LENTH_ + inField32 + 1;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr606100_Node, NULL, BAD_CAST "F35Adr", BAD_CAST szTemplate);
        
        /* <F37Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F22_LENTH_ + _ECC_F24_LENTH_ +
                    _ECC_F25_LENTH_ + inField32 + _ECC_F35_LENTH_ + 8;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr606100_Node, NULL, BAD_CAST "F37Adr", BAD_CAST szTemplate);
        
	/* <F57Adr> */
	if (memcmp(szIFESMode, "Y", 1) == 0)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = ginECC_F57_Len;
		sprintf(szTemplate, "-%d", inDataLen);
		xmlNewChild(sr606100_Node, NULL, BAD_CAST "F57Adr", BAD_CAST szTemplate);
	}
	
        /* <F59Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = ginECC_F59_ET_Len;
	sprintf(szTemplate, "-%d", inDataLen);
        xmlNewChild(sr606100_Node, NULL, BAD_CAST "F59Adr", BAD_CAST szTemplate);

        /* <F62Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = inField62;
	sprintf(szTemplate, "-%d", inDataLen);
        xmlNewChild(sr606100_Node, NULL, BAD_CAST "F62Adr", BAD_CAST szTemplate);
	
//	memset(szFESMode, 0x00, sizeof(szFESMode));
//	inGetNCCCFESMode(szFESMode);
//	memset(szTemplate, 0x00, sizeof(szTemplate));
//	inGetCloud_MFES(szTemplate);
//        if (memcmp(szFESMode, _NCCC_05_ATS_MODE_, 2) == 0	||
//	    memcmp(szFESMode, _NCCC_04_MPAS_MODE_, 2) == 0	||
//	   (memcmp(szFESMode, _NCCC_03_MFES_MODE_, 2) == 0	&&
//	    memcmp(szTemplate, "Y", strlen("Y")) == 0))
//	{
		/* <F39Adr> */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
			    _ECC_F24_LENTH_ + inField32 + _ECC_F37_LENTH_ + _ECC_F38_LENTH_;
		sprintf(szTemplate, "%d", inDataLen);
		xmlNewChild(sr606100_Node, NULL, BAD_CAST "F39Adr", BAD_CAST szTemplate);

		/* <ECCAdr> */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_  + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
			    _ECC_F24_LENTH_ + inField32 + _ECC_F37_LENTH_ + _ECC_F38_LENTH_ + _ECC_F39_LENTH_ + _ECC_F41_LENTH_ + _ECC_F56_LENTH_;
		sprintf(szTemplate, "%d", inDataLen);
		xmlNewChild(sr606100_Node, NULL, BAD_CAST "ECCAdr", BAD_CAST szTemplate);
//	}
//	else
//	{
//		/* <F39Adr> */
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
//			    _ECC_F24_LENTH_;
//		sprintf(szTemplate, "%d", inDataLen);
//		xmlNewChild(sr606100_Node, NULL, BAD_CAST "F39Adr", BAD_CAST szTemplate);
//
//		/* <ECCAdr> */
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_  + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
//			    _ECC_F24_LENTH_ + _ECC_F39_LENTH_ + _ECC_F41_LENTH_ + _ECC_F56_LENTH_;
//		sprintf(szTemplate, "%d", inDataLen);
//		xmlNewChild(sr606100_Node, NULL, BAD_CAST "ECCAdr", BAD_CAST szTemplate);
//	}
	
        /* <606100_Adv>==============================================================================購貨0220 */
        memset(szSendPacket, 0x00, sizeof(szSendPacket));
        inTxnCode = _ADVICE_;
        pobTran->inISOTxnCode = _ADVICE_;
            
        /* 組 ISO 電文 */
	ginBeforeIndex = 0;
	if ((inSendLen = inNCCC_TICKET_PackISO(pobTran, (unsigned char *)szSendPacket, inTxnCode)) <= 0)
	{
		/* 清空佔用記憶體 */
		inXML_End(&srDoc);
	
		/* 上傳不能Show錯誤訊息 */
		pobTran->inErrorMsg = _ERROR_CODE_V3_NONE_;
		return (VS_ISO_PACK_ERR);
	}
	
	/* 回復交易碼 */
	pobTran->inISOTxnCode = pobTran->srTRec.inCode;
	
	/* 沒找到 */
	if (ginBeforeIndex == 0)
	{
		/* 清空佔用記憶體 */
		inXML_End(&srDoc);
	
	        return (VS_ERROR);
	}

        /* <606100_Adv> */
	sr606100adv_Node = xmlNewNode(NULL, BAD_CAST "606100_Adv");
	xmlAddChild(srRoot_Node, sr606100adv_Node);
        
	/* <BefData> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%s", "0000");
	inFunc_BCD_to_ASCII(&szTemplate[4], (unsigned char*)&szSendPacket[0], ginBeforeIndex);
        xmlNewChild(sr606100adv_Node, NULL, BAD_CAST "BefData", BAD_CAST szTemplate);
             
        /* <AftData> : 扣掉取代的Len + @@@@@ */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inFunc_BCD_to_ASCII(&szTemplate[0], (unsigned char*)&szSendPacket[ginBeforeIndex + 7], inSendLen - ginBeforeIndex - 7);
        xmlNewChild(sr606100adv_Node, NULL, BAD_CAST "AftData", BAD_CAST szTemplate);
	inField62 = (inSendLen - ginBeforeIndex - 7) - 6;

	/* <F03Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr606100adv_Node, NULL, BAD_CAST "F03Adr", BAD_CAST szTemplate);
	
	/* <F04Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr606100adv_Node, NULL, BAD_CAST "F04Adr", BAD_CAST szTemplate);
        
	/* <F11Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr606100adv_Node, NULL, BAD_CAST "F11Adr", BAD_CAST szTemplate);

        /* <F12Adr> */
        memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr606100adv_Node, NULL, BAD_CAST "F12Adr", BAD_CAST szTemplate);
        
        /* <F13Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr606100adv_Node, NULL, BAD_CAST "F13Adr", BAD_CAST szTemplate);
        
	/* <F35Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
		    _ECC_F22_LENTH_ + _ECC_F24_LENTH_ + _ECC_F25_LENTH_ + inField32 + 1;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr606100adv_Node, NULL, BAD_CAST "F35Adr", BAD_CAST szTemplate);
        
        /* <F37Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
		    _ECC_F22_LENTH_ + _ECC_F24_LENTH_ + _ECC_F25_LENTH_ + inField32 + _ECC_F35_LENTH_ + 8;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr606100adv_Node, NULL, BAD_CAST "F37Adr", BAD_CAST szTemplate);

	/* <F57Adr> */
	if (memcmp(szIFESMode, "Y", 1) == 0)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = ginECC_F57_Len;
		sprintf(szTemplate, "-%d", inDataLen);
		xmlNewChild(sr606100adv_Node, NULL, BAD_CAST "F57Adr", BAD_CAST szTemplate);
	}
	
	 /* <F59Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = ginECC_F59_ET_Len;
	sprintf(szTemplate, "-%d", inDataLen);
        xmlNewChild(sr606100adv_Node, NULL, BAD_CAST "F59Adr", BAD_CAST szTemplate);

        /* <F62Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = inField62;
	sprintf(szTemplate, "-%d", inDataLen);
        xmlNewChild(sr606100adv_Node, NULL, BAD_CAST "F62Adr", BAD_CAST szTemplate);
	
//	memset(szFESMode, 0x00, sizeof(szFESMode));
//	inGetNCCCFESMode(szFESMode);
//	memset(szTemplate, 0x00, sizeof(szTemplate));
//	inGetCloud_MFES(szTemplate);
//        if (memcmp(szFESMode, _NCCC_05_ATS_MODE_, 2) == 0	||
//	    memcmp(szFESMode, _NCCC_04_MPAS_MODE_, 2) == 0	||
//	   (memcmp(szFESMode, _NCCC_03_MFES_MODE_, 2) == 0	&&
//	    memcmp(szTemplate, "Y", strlen("Y")) == 0))
//	{
		/* <F39Adr> */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F24_LENTH_ + inField32 + 
			    _ECC_F37_LENTH_ + _ECC_F38_LENTH_;
		sprintf(szTemplate, "%d", inDataLen);
		xmlNewChild(sr606100adv_Node, NULL, BAD_CAST "F39Adr", BAD_CAST szTemplate);

		/* <ECCAdr> */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_  + _ECC_F11_LENTH_ + _ECC_F24_LENTH_ + inField32 + 
			    _ECC_F37_LENTH_ + _ECC_F38_LENTH_ + _ECC_F39_LENTH_ + _ECC_F41_LENTH_ + _ECC_F56_LENTH_;
		sprintf(szTemplate, "%d", inDataLen);
		xmlNewChild(sr606100adv_Node, NULL, BAD_CAST "ECCAdr", BAD_CAST szTemplate);
//	}
//	else
//	{
//		/* <F39Adr> */
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F24_LENTH_;
//		sprintf(szTemplate, "%d", inDataLen);
//		xmlNewChild(sr606100adv_Node, NULL, BAD_CAST "F39Adr", BAD_CAST szTemplate);
//
//		/* <ECCAdr> */
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_  + _ECC_F11_LENTH_ + _ECC_F24_LENTH_ + _ECC_F39_LENTH_ + 
//			    _ECC_F41_LENTH_ + _ECC_F56_LENTH_;
//		sprintf(szTemplate, "%d", inDataLen);
//		xmlNewChild(sr606100adv_Node, NULL, BAD_CAST "ECCAdr", BAD_CAST szTemplate);
//	}

        /* <606100_Rev>==============================================================================購貨0400 */
        memset(szSendPacket, 0x00, sizeof(szSendPacket));
        inTxnCode = _REVERSAL_;
        pobTran->inISOTxnCode = _REVERSAL_;
  
        /* 組 ISO 電文 */
	ginBeforeIndex = 0;
	if ((inSendLen = inNCCC_TICKET_PackISO(pobTran, (unsigned char *)szSendPacket, inTxnCode)) <= 0)
	{
		/* 清空佔用記憶體 */
		inXML_End(&srDoc);
		
		/* 上傳不能Show錯誤訊息 */
		pobTran->inErrorMsg = _ERROR_CODE_V3_NONE_;
		
		return (VS_ISO_PACK_ERR);
	}
	
	/* 回復交易碼 */
	pobTran->inISOTxnCode = pobTran->srTRec.inCode;
	
	/* 沒找到 */
	if (ginBeforeIndex == 0)
	{
		/* 清空佔用記憶體 */
		inXML_End(&srDoc);
		
	        return (VS_ERROR);
	}
	
        /* <606100_Rev> */
	sr606100rev_Node = xmlNewNode(NULL, BAD_CAST "606100_Rev");
	xmlAddChild(srRoot_Node, sr606100rev_Node);

	/* <BefData> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%s", "0000");
	inFunc_BCD_to_ASCII(&szTemplate[4], (unsigned char*)&szSendPacket[0], ginBeforeIndex);
        xmlNewChild(sr606100rev_Node, NULL, BAD_CAST "BefData", BAD_CAST szTemplate);
             
        /* <AftData> : 扣掉取代的Len + @@@@@ */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inFunc_BCD_to_ASCII(&szTemplate[0], (unsigned char*)&szSendPacket[ginBeforeIndex + 7], inSendLen - ginBeforeIndex - 7);
        xmlNewChild(sr606100rev_Node, NULL, BAD_CAST "AftData", BAD_CAST szTemplate);
	inField62 = (inSendLen - ginBeforeIndex - 7) - 6;

	/* <F03Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr606100rev_Node, NULL, BAD_CAST "F03Adr", BAD_CAST szTemplate);
	
	/* <F04Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr606100rev_Node, NULL, BAD_CAST "F04Adr", BAD_CAST szTemplate);
        
	/* <F11Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr606100rev_Node, NULL, BAD_CAST "F11Adr", BAD_CAST szTemplate);
	
	/* <F35Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F22_LENTH_ + _ECC_F24_LENTH_ +
                    _ECC_F25_LENTH_ + inField32 + 1;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr606100rev_Node, NULL, BAD_CAST "F35Adr", BAD_CAST szTemplate);
        
	/* <F37Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F22_LENTH_ + _ECC_F24_LENTH_ +
                    _ECC_F25_LENTH_ + inField32 + _ECC_F35_LENTH_ + 8;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr606100rev_Node, NULL, BAD_CAST "F37Adr", BAD_CAST szTemplate);
        
	/* <F57Adr> */
	if (memcmp(szIFESMode, "Y", 1) == 0)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = ginECC_F57_Len;
		sprintf(szTemplate, "-%d", inDataLen);
		xmlNewChild(sr606100rev_Node, NULL, BAD_CAST "F57Adr", BAD_CAST szTemplate);
	}
	
        /* <F59Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = ginECC_F59_ET_Len;
	sprintf(szTemplate, "-%d", inDataLen);
        xmlNewChild(sr606100rev_Node, NULL, BAD_CAST "F59Adr", BAD_CAST szTemplate);

        /* <F62Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = inField62;
	sprintf(szTemplate, "-%d", inDataLen);
        xmlNewChild(sr606100rev_Node, NULL, BAD_CAST "F62Adr", BAD_CAST szTemplate);
	
//	memset(szFESMode, 0x00, sizeof(szFESMode));
//	inGetNCCCFESMode(szFESMode);
//	memset(szTemplate, 0x00, sizeof(szTemplate));
//	inGetCloud_MFES(szTemplate);
//        if (memcmp(szFESMode, _NCCC_05_ATS_MODE_, 2) == 0	||
//	    memcmp(szFESMode, _NCCC_04_MPAS_MODE_, 2) == 0	||
//	   (memcmp(szFESMode, _NCCC_03_MFES_MODE_, 2) == 0	&&
//	    memcmp(szTemplate, "Y", strlen("Y")) == 0))
//	{
		/* ATS規格 - 回覆跟Host長度有關，目前一樣 */
		/* <F39Adr> */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
			    _ECC_F24_LENTH_ + inField32 + _ECC_F37_LENTH_ + _ECC_F38_LENTH_;
		sprintf(szTemplate, "%d", inDataLen);
		xmlNewChild(sr606100rev_Node, NULL, BAD_CAST "F39Adr", BAD_CAST szTemplate);

		/* <ECCAdr> */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_  + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
			    _ECC_F24_LENTH_ + inField32 + _ECC_F37_LENTH_ + _ECC_F39_LENTH_ + _ECC_F41_LENTH_ + _ECC_F56_LENTH_;
		sprintf(szTemplate, "%d", inDataLen);
		xmlNewChild(sr606100rev_Node, NULL, BAD_CAST "ECCAdr", BAD_CAST szTemplate);
//	}
//	else
//	{
//		/* <F39Adr> */
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
//			    _ECC_F24_LENTH_;
//		sprintf(szTemplate, "%d", inDataLen);
//		xmlNewChild(sr606100rev_Node, NULL, BAD_CAST "F39Adr", BAD_CAST szTemplate);
//
//		/* <ECCAdr> */
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_  + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
//			    _ECC_F24_LENTH_ + inField32 + _ECC_F37_LENTH_ + _ECC_F38_LENTH_ + _ECC_F39_LENTH_ + _ECC_F41_LENTH_;
//		sprintf(szTemplate, "%d", inDataLen);
//		xmlNewChild(sr606100rev_Node, NULL, BAD_CAST "ECCAdr", BAD_CAST szTemplate);
//	}

        /* <825799> =============================================================================自動加值0100 */
        memset(szSendPacket, 0x00, sizeof(szSendPacket));
        inTxnCode = _TICKET_EASYCARD_AUTO_TOP_UP_;
        pobTran->inISOTxnCode = pobTran->srTRec.inCode;
	inOrgCode = pobTran->srTRec.inCode;
	pobTran->srTRec.inCode = _TICKET_EASYCARD_AUTO_TOP_UP_;
        
        /* 組 ISO 電文 */
	ginBeforeIndex = 0;
	if ((inSendLen = inNCCC_TICKET_PackISO(pobTran, (unsigned char *)szSendPacket, inTxnCode)) <= 0)
	{
		/* 清空佔用記憶體 */
		inXML_End(&srDoc);
		
		/* 上傳不能Show錯誤訊息 */
		pobTran->inErrorMsg = _ERROR_CODE_V3_NONE_;
		
		return (VS_ISO_PACK_ERR);
	}
	
	/* 回復交易碼 */
	pobTran->srTRec.inCode = inOrgCode;
	pobTran->inISOTxnCode = pobTran->srTRec.inCode;
	
	/* 沒找到 */
	if (ginBeforeIndex == 0)
	{
		/* 清空佔用記憶體 */
		inXML_End(&srDoc);
		
	        return (VS_ERROR);
	}

        /* <825799> */
	sr825799_Node = xmlNewNode(NULL, BAD_CAST "825799");
	xmlAddChild(srRoot_Node, sr825799_Node);

	/* <BefData> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%s", "0000");
	inFunc_BCD_to_ASCII(&szTemplate[4], (unsigned char*)&szSendPacket[0], ginBeforeIndex);
        xmlNewChild(sr825799_Node, NULL, BAD_CAST "BefData", BAD_CAST szTemplate);
             
        /* <AftData> : 扣掉取代的Len + @@@@@ */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inFunc_BCD_to_ASCII(&szTemplate[0], (unsigned char*)&szSendPacket[ginBeforeIndex + 7], inSendLen - ginBeforeIndex - 7);
        xmlNewChild(sr825799_Node, NULL, BAD_CAST "AftData", BAD_CAST szTemplate);
	inField62 = (inSendLen - ginBeforeIndex - 7) - 6;
	
	/* <F03Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr825799_Node, NULL, BAD_CAST "F03Adr", BAD_CAST szTemplate);
	
	/* <F04Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr825799_Node, NULL, BAD_CAST "F04Adr", BAD_CAST szTemplate);
        
	/* <F11Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr825799_Node, NULL, BAD_CAST "F11Adr", BAD_CAST szTemplate);
	
	/* <F35Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F22_LENTH_ + _ECC_F24_LENTH_ +
                    _ECC_F25_LENTH_ + inField32 + 1;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr825799_Node, NULL, BAD_CAST "F35Adr", BAD_CAST szTemplate);
        
        /* <F37Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F22_LENTH_ + _ECC_F24_LENTH_ +
                    _ECC_F25_LENTH_ + inField32 + _ECC_F35_LENTH_ + 8;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr825799_Node, NULL, BAD_CAST "F37Adr", BAD_CAST szTemplate);
        
	/* <F57Adr> */
	if (memcmp(szIFESMode, "Y", 1) == 0)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = ginECC_F57_Len;
		sprintf(szTemplate, "-%d", inDataLen);
		xmlNewChild(sr825799_Node, NULL, BAD_CAST "F57Adr", BAD_CAST szTemplate);
	}
	
        /* <F59Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = ginECC_F59_ET_Len;
	sprintf(szTemplate, "-%d", inDataLen);
        xmlNewChild(sr825799_Node, NULL, BAD_CAST "F59Adr", BAD_CAST szTemplate);

        /* <F62Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = inField62;
	sprintf(szTemplate, "-%d", inDataLen);
        xmlNewChild(sr825799_Node, NULL, BAD_CAST "F62Adr", BAD_CAST szTemplate);
	
	//	memset(szFESMode, 0x00, sizeof(szFESMode));
//	inGetNCCCFESMode(szFESMode);
//	memset(szTemplate, 0x00, sizeof(szTemplate));
//	inGetCloud_MFES(szTemplate);
//        if (memcmp(szFESMode, _NCCC_05_ATS_MODE_, 2) == 0	||
//	    memcmp(szFESMode, _NCCC_04_MPAS_MODE_, 2) == 0	||
//	   (memcmp(szFESMode, _NCCC_03_MFES_MODE_, 2) == 0	&&
//	    memcmp(szTemplate, "Y", strlen("Y")) == 0))
//	{
		/* <F39Adr> */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
			    _ECC_F24_LENTH_ + inField32 + _ECC_F37_LENTH_ + _ECC_F38_LENTH_;
		sprintf(szTemplate, "%d", inDataLen);
		xmlNewChild(sr825799_Node, NULL, BAD_CAST "F39Adr", BAD_CAST szTemplate);

		/* <ECCAdr> */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_  + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
			    _ECC_F24_LENTH_ + inField32 + _ECC_F37_LENTH_ + _ECC_F38_LENTH_ + _ECC_F39_LENTH_ + _ECC_F41_LENTH_ + _ECC_F56_LENTH_;
		sprintf(szTemplate, "%d", inDataLen);
		xmlNewChild(sr825799_Node, NULL, BAD_CAST "ECCAdr", BAD_CAST szTemplate);
//	}
//	else
//	{
//		/* <F39Adr> */
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
//			    _ECC_F24_LENTH_;
//		sprintf(szTemplate, "%d", inDataLen);
//		xmlNewChild(sr825799_Node, NULL, BAD_CAST "F39Adr", BAD_CAST szTemplate);
//
//		/* <ECCAdr> */
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_  + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
//			    _ECC_F24_LENTH_ + _ECC_F39_LENTH_ + _ECC_F41_LENTH_ + _ECC_F56_LENTH_;
//		sprintf(szTemplate, "%d", inDataLen);
//		xmlNewChild(sr825799_Node, NULL, BAD_CAST "ECCAdr", BAD_CAST szTemplate);
//	}
	
        /* <825799_Adv>==========================================================================自動加值0220 */
        memset(szSendPacket, 0x00, sizeof(szSendPacket));
        inTxnCode = _ADVICE_;
        pobTran->inISOTxnCode = _ADVICE_;
	inOrgCode = pobTran->srTRec.inCode;
	pobTran->srTRec.inCode = _TICKET_EASYCARD_AUTO_TOP_UP_;
        
        /* 組 ISO 電文 */
	ginBeforeIndex = 0;
	if ((inSendLen = inNCCC_TICKET_PackISO(pobTran, (unsigned char *)szSendPacket, inTxnCode)) <= 0)
	{
		/* 清空佔用記憶體 */
		inXML_End(&srDoc);
		
		/* 上傳不能Show錯誤訊息 */
		pobTran->inErrorMsg = _ERROR_CODE_V3_NONE_;
		
		return (VS_ISO_PACK_ERR);
	}
	
	/* 回復交易碼 */
	pobTran->srTRec.inCode = inOrgCode;
	pobTran->inISOTxnCode = pobTran->srTRec.inCode;
	
	/* 沒找到 */
	if (ginBeforeIndex == 0)
	{
		/* 清空佔用記憶體 */
		inXML_End(&srDoc);
	
	        return (VS_ERROR);
	}
	        
        /* <825799_Adv> */
	sr825799adv_Node = xmlNewNode(NULL, BAD_CAST "825799_Adv");
	xmlAddChild(srRoot_Node, sr825799adv_Node);
        
	/* <BefData> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%s", "0000");
	inFunc_BCD_to_ASCII(&szTemplate[4], (unsigned char*)&szSendPacket[0], ginBeforeIndex);
        xmlNewChild(sr825799adv_Node, NULL, BAD_CAST "BefData", BAD_CAST szTemplate);
             
        /* <AftData> : 扣掉取代的Len + @@@@@ */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inFunc_BCD_to_ASCII(&szTemplate[0], (unsigned char*)&szSendPacket[ginBeforeIndex + 7], inSendLen - ginBeforeIndex - 7);
        xmlNewChild(sr825799adv_Node, NULL, BAD_CAST "AftData", BAD_CAST szTemplate);
	inField62 = (inSendLen - ginBeforeIndex - 7) - 6;

	/* <F03Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr825799adv_Node, NULL, BAD_CAST "F03Adr", BAD_CAST szTemplate);
	
	/* <F04Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr825799adv_Node, NULL, BAD_CAST "F04Adr", BAD_CAST szTemplate);
        
	/* <F11Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr825799adv_Node, NULL, BAD_CAST "F11Adr", BAD_CAST szTemplate);

        /* <F12Adr> */
        memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr825799adv_Node, NULL, BAD_CAST "F12Adr", BAD_CAST szTemplate);
        
        /* <F13Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr825799adv_Node, NULL, BAD_CAST "F13Adr", BAD_CAST szTemplate);
        
	/* <F35Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
		    _ECC_F22_LENTH_ + _ECC_F24_LENTH_ + _ECC_F25_LENTH_ + inField32 + 1;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr825799adv_Node, NULL, BAD_CAST "F35Adr", BAD_CAST szTemplate);
        
        /* <F37Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
		    _ECC_F22_LENTH_ + _ECC_F24_LENTH_ + _ECC_F25_LENTH_ + inField32 + _ECC_F35_LENTH_ + 8;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr825799adv_Node, NULL, BAD_CAST "F37Adr", BAD_CAST szTemplate);

	/* <F57Adr> */
	if (memcmp(szIFESMode, "Y", 1) == 0)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = ginECC_F57_Len;
		sprintf(szTemplate, "-%d", inDataLen);
		xmlNewChild(sr825799adv_Node, NULL, BAD_CAST "F57Adr", BAD_CAST szTemplate);
	}
	
	 /* <F59Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = ginECC_F59_ET_Len;
	sprintf(szTemplate, "-%d", inDataLen);
        xmlNewChild(sr825799adv_Node, NULL, BAD_CAST "F59Adr", BAD_CAST szTemplate);

        /* <F62Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = inField62;
	sprintf(szTemplate, "-%d", inDataLen);
        xmlNewChild(sr825799adv_Node, NULL, BAD_CAST "F62Adr", BAD_CAST szTemplate);
	
//	memset(szFESMode, 0x00, sizeof(szFESMode));
//	inGetNCCCFESMode(szFESMode);
//	memset(szTemplate, 0x00, sizeof(szTemplate));
//	inGetCloud_MFES(szTemplate);
//        if (memcmp(szFESMode, _NCCC_05_ATS_MODE_, 2) == 0	||
//	    memcmp(szFESMode, _NCCC_04_MPAS_MODE_, 2) == 0	||
//	   (memcmp(szFESMode, _NCCC_03_MFES_MODE_, 2) == 0	&&
//	    memcmp(szTemplate, "Y", strlen("Y")) == 0))
//	{
		/* ATS規格 - 回覆跟Host長度有關，目前一樣 */
		/* <F39Adr> */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F24_LENTH_ + inField32 + 
			    _ECC_F37_LENTH_ + _ECC_F38_LENTH_;
		sprintf(szTemplate, "%d", inDataLen);
		xmlNewChild(sr825799adv_Node, NULL, BAD_CAST "F39Adr", BAD_CAST szTemplate);

		/* <ECCAdr> */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_  + _ECC_F11_LENTH_ + _ECC_F24_LENTH_ + inField32 + 
			    _ECC_F37_LENTH_ + _ECC_F38_LENTH_ + _ECC_F39_LENTH_ + _ECC_F41_LENTH_ + _ECC_F56_LENTH_;
		sprintf(szTemplate, "%d", inDataLen);
		xmlNewChild(sr825799adv_Node, NULL, BAD_CAST "ECCAdr", BAD_CAST szTemplate);
//	}
//	else
//	{
//		/* <F39Adr> */
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F24_LENTH_;
//		sprintf(szTemplate, "%d", inDataLen);
//		xmlNewChild(sr825799adv_Node, NULL, BAD_CAST "F39Adr", BAD_CAST szTemplate);
//
//		/* <ECCAdr> */
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_  + _ECC_F11_LENTH_ + _ECC_F24_LENTH_ + 
//		            _ECC_F39_LENTH_ + _ECC_F41_LENTH_ + _ECC_F56_LENTH_;
//		sprintf(szTemplate, "%d", inDataLen);
//		xmlNewChild(sr825799adv_Node, NULL, BAD_CAST "ECCAdr", BAD_CAST szTemplate);
//	}

        /* <825799_Rev>==========================================================================自動加值0400 */
        memset(szSendPacket, 0x00, sizeof(szSendPacket));
        inTxnCode = _REVERSAL_;
        pobTran->inISOTxnCode = _REVERSAL_;
	inOrgCode = pobTran->srTRec.inCode;
	pobTran->srTRec.inCode = _TICKET_EASYCARD_AUTO_TOP_UP_;
  
        /* 組 ISO 電文 */
	ginBeforeIndex = 0;
	if ((inSendLen = inNCCC_TICKET_PackISO(pobTran, (unsigned char *)szSendPacket, inTxnCode)) <= 0)
	{
		/* 清空佔用記憶體 */
		inXML_End(&srDoc);
	
		/* 上傳不能Show錯誤訊息 */
		pobTran->inErrorMsg = _ERROR_CODE_V3_NONE_;
		
		return (VS_ISO_PACK_ERR);
	}
	
	/* 回復交易碼 */
	pobTran->srTRec.inCode = inOrgCode;
	pobTran->inISOTxnCode = pobTran->srTRec.inCode;
	
	/* 沒找到 */
	if (ginBeforeIndex == 0)
	{
		/* 清空佔用記憶體 */
		inXML_End(&srDoc);
	
	        return (VS_ERROR);
	}
	        
        /* <825799_Rev> */
	sr825799rev_Node = xmlNewNode(NULL, BAD_CAST "825799_Rev");
	xmlAddChild(srRoot_Node, sr825799rev_Node);

	/* <BefData> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%s", "0000");
	inFunc_BCD_to_ASCII(&szTemplate[4], (unsigned char*)&szSendPacket[0], ginBeforeIndex);
        xmlNewChild(sr825799rev_Node, NULL, BAD_CAST "BefData", BAD_CAST szTemplate);
             
        /* <AftData> : 扣掉取代的Len + @@@@@ */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inFunc_BCD_to_ASCII(&szTemplate[0], (unsigned char*)&szSendPacket[ginBeforeIndex + 7], inSendLen - ginBeforeIndex - 7);
        xmlNewChild(sr825799rev_Node, NULL, BAD_CAST "AftData", BAD_CAST szTemplate);
	inField62 = (inSendLen - ginBeforeIndex - 7) - 6;
	
	/* <F03Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr825799rev_Node, NULL, BAD_CAST "F03Adr", BAD_CAST szTemplate);
	
	/* <F04Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr825799rev_Node, NULL, BAD_CAST "F04Adr", BAD_CAST szTemplate);
        
	/* <F11Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr825799rev_Node, NULL, BAD_CAST "F11Adr", BAD_CAST szTemplate);
	
	/* <F35Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F22_LENTH_ + _ECC_F24_LENTH_ +
                    _ECC_F25_LENTH_ + inField32 + 1;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr825799rev_Node, NULL, BAD_CAST "F35Adr", BAD_CAST szTemplate);
        
	/* <F37Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F22_LENTH_ + _ECC_F24_LENTH_ +
                    _ECC_F25_LENTH_ + inField32 + _ECC_F35_LENTH_ + 8;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr825799rev_Node, NULL, BAD_CAST "F37Adr", BAD_CAST szTemplate);
        
	/* <F57Adr> */
	if (memcmp(szIFESMode, "Y", 1) == 0)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = ginECC_F57_Len;
		sprintf(szTemplate, "-%d", inDataLen);
		xmlNewChild(sr825799rev_Node, NULL, BAD_CAST "F57Adr", BAD_CAST szTemplate);
	}
	
        /* <F59Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = ginECC_F59_ET_Len;
	sprintf(szTemplate, "-%d", inDataLen);
        xmlNewChild(sr825799rev_Node, NULL, BAD_CAST "F59Adr", BAD_CAST szTemplate);

        /* <F62Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = inField62;
	sprintf(szTemplate, "-%d", inDataLen);
        xmlNewChild(sr825799rev_Node, NULL, BAD_CAST "F62Adr", BAD_CAST szTemplate);
	
//	memset(szFESMode, 0x00, sizeof(szFESMode));
//	inGetNCCCFESMode(szFESMode);
//	memset(szTemplate, 0x00, sizeof(szTemplate));
//	inGetCloud_MFES(szTemplate);
//        if (memcmp(szFESMode, _NCCC_05_ATS_MODE_, 2) == 0	||
//	    memcmp(szFESMode, _NCCC_04_MPAS_MODE_, 2) == 0	||
//	   (memcmp(szFESMode, _NCCC_03_MFES_MODE_, 2) == 0	&&
//	    memcmp(szTemplate, "Y", strlen("Y")) == 0))
//	{
		/* <F39Adr> */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
			    _ECC_F24_LENTH_ + inField32 + _ECC_F37_LENTH_ + _ECC_F38_LENTH_;
		sprintf(szTemplate, "%d", inDataLen);
		xmlNewChild(sr825799rev_Node, NULL, BAD_CAST "F39Adr", BAD_CAST szTemplate);

		/* <ECCAdr> */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_  + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
			    _ECC_F24_LENTH_ + inField32 + _ECC_F37_LENTH_ + _ECC_F39_LENTH_ + _ECC_F41_LENTH_ + _ECC_F56_LENTH_;
		sprintf(szTemplate, "%d", inDataLen);
		xmlNewChild(sr825799rev_Node, NULL, BAD_CAST "ECCAdr", BAD_CAST szTemplate);
//	}
//	else
//	{
//		/* <F39Adr> */
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
//			    _ECC_F24_LENTH_;
//		sprintf(szTemplate, "%d", inDataLen);
//		xmlNewChild(sr825799rev_Node, NULL, BAD_CAST "F39Adr", BAD_CAST szTemplate);
//
//		/* <ECCAdr> */
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_  + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
//			    _ECC_F24_LENTH_ + _ECC_F39_LENTH_ + _ECC_F41_LENTH_ + _ECC_F56_LENTH_;
//		sprintf(szTemplate, "%d", inDataLen);
//		xmlNewChild(sr825799rev_Node, NULL, BAD_CAST "ECCAdr", BAD_CAST szTemplate);
//	}

	/* 儲存xml檔 xml檔現在會自動換行了 */
        inXML_SaveFile(_ECC_API_REQ_FILE2_, &srDoc, "utf-8", 1);
	inFile_Move_File(_ECC_API_REQ_FILE2_, _AP_ROOT_PATH_, "", _ECC_FOLDER_PATH_);
	
	/* 清空佔用記憶體 */
	inXML_End(&srDoc);
	
	if (inECC_Insert_Temp() != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inECC_PACK_Deduct_ICERAPI_NCCC() END !");
		inLogPrintf(AT, "----------------------------------------");
	}

        return (VS_SUCCESS);
}

/*
Function        :inECC_Refund_Flow
Date&Time       :2018/4/2 下午 3:36
Describe        :
*/
int inECC_Refund_Flow(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_ERROR;
	int		inRetry = 1;
	char		szKey = 0x00;
	char		szCustomerIndicator[3 + 1] = {0};
	char		szDebugMsg[100 + 1] = {0};
	unsigned char	uszECRAlreadyBit = VS_FALSE;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inECC_Refund_Flow() START !");
	}
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	guszECCRetryBit = VS_FALSE;
	
	while (inRetry <= 5)
	{
		/* (需求單 - 107227)邦柏科技自助作業客製化 by Russell 2018/12/13 下午 8:33 */
		/* (需求單 - 107276)自助交易標準400做法 by Russell 2018/12/27 上午 11:20 */
		if ((!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)           ||
		     !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_)   ||
                     !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)        ||
		     !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)	||
		     !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)		||
		     !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))	&&
		     pobTran->uszLastTranscationBit == VS_TRUE)
		{
			uszECRAlreadyBit = VS_TRUE;
			inECC_PACK_RETRY_ICERAPI(pobTran);
		}
		else
		{
			/* 組電文(生成ICERAPI.REQ) */
			inECC_PACK_Refund_ICERAPI(pobTran);
		}

		/* AdditionalTcpipData */
	        inECC_PACK_Refund_ICERAPI_NCCC(pobTran);

                if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "ICER Start");
			inLogPrintf(AT, szDebugMsg);
		}
		
		inRetVal = ICERApi_exe();
		
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "ICER End");
			inLogPrintf(AT, szDebugMsg);
		}

		/* 解電文(分析ICERAPI.RES) */
		inRetVal = inECC_UNPACK_ICERAPI(pobTran, _ECC_API_RES_FILE_);	
		
		if (inRetVal != VS_SUCCESS)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inCTLS_Set_LED(_CTLS_LIGHT_RED_);
                        inDISP_ChineseFont("悠遊卡檔案異常", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
                        inDISP_BEEP(3, 1000);
			
                        return (VS_ERROR);
                }

		/* 檢核回應碼 <T3901>開始 */
	        if (memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "0", 1) != 0)
	        {
			if (!memcmp(pobTran->srTRec.szRespCode, "A0", 2)	||
			    !memcmp(pobTran->srTRec.szRespCode, "A1", 2))
			{
				ginMacError++;
				inECC_UNPACK_ICERAPI_NCCC(pobTran, _ECC_API_RES_FILE2_, _ECC_FOLDER_PATH_);
			}
			
			/* (需求單 - 107227)邦柏科技自助作業客製化 by Russell 2018/12/9 下午 2:42 */
			/* (需求單 - 107276)自助交易標準400做法 by Russell 2018/12/27 上午 11:20 */
			if ((!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)        ||
			     !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_)||
			     !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)	     ||
			     !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))	&&
			     pobTran->uszECRBit == VS_TRUE)
			{
				/* 第二次發生RETRY 回傳失敗 */
	        		if (uszECRAlreadyBit == VS_TRUE)
				{
	        			return (VS_ERROR);
				}
				
				if (memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-125", 4) == 0)
				{
	                                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_ChineseFont("請重新交易", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
					inDISP_BEEP(3, 1000);

	                                /* 要回錯誤碼0018不是0001 所以Reset */
	                                memset(pobTran->szTicket_ErrorCode, 0x00, sizeof(pobTran->szTicket_ErrorCode));
					pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_ESVC_RE_TAP_CARD_;

					/* 將現在組好的REQ 備份成ICERRETRY.REQ */
					inFunc_Data_Copy(_ECC_API_REQ_FILE_, _ECC_FOLDER_PATH_, _ECC_API_REQ_RETRY_FILE_, _ECC_FOLDER_PATH_);

//					/* 將pobTran的MFRec備份部分資料 */
//					inECC_SAVE_TREC(pobTran);
					
	                                return (VS_ERROR);
	                        }
				else
				{	
					/* 其他錯誤Display */
					inECC_ResponeCode(pobTran);

					return (VS_ERROR);
				}
			}
                        else if ((!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)       ||
			          !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_))	&&
			         pobTran->uszECRBit == VS_TRUE)
			{
				/* 第二次發生RETRY 回傳失敗 */
	        		if (uszECRAlreadyBit == VS_TRUE)
				{
	        			return (VS_ERROR);
				}
				
				if (memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-125", 4) == 0)
				{
	                                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_ChineseFont("請重新交易", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
					inDISP_BEEP(3, 1000);

	                                /* 要回錯誤碼0018不是0001 所以Reset */
	                                memset(pobTran->szTicket_ErrorCode, 0x00, sizeof(pobTran->szTicket_ErrorCode));
					pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_ESVC_RE_TAP_CARD_;

					/* 將現在組好的REQ 備份成ICERRETRY.REQ */
					inFunc_Data_Copy(_ECC_API_REQ_FILE_, _ECC_FOLDER_PATH_, _ECC_API_REQ_RETRY_FILE_, _ECC_FOLDER_PATH_);

//					/* 將pobTran的MFRec備份部分資料 */
//					inECC_SAVE_TREC(pobTran);
                                        
                                        pobTran->uszECCRetryBit = VS_TRUE;
                                        
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "ECC Retry....");
                                        }
					
	                                return (VS_ERROR);
	                        }
				else
				{	
					/* 其他錯誤Display */
					inECC_ResponeCode(pobTran);

					return (VS_ERROR);
				}
			}
			else
			{
				/* -125 = Retry */
				/* -119 : Call相關DLL回應錯誤 ; 請參考T3904 */
				/* 62 : RC531錯誤，交易中止，請重新交易*/
				if ((memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-125", 4) == 0)	||
				    (memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-119", 4) == 0	&&
				     memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "62", 2) == 0	&&
				     guszECCRetryBit == VS_TRUE))
				{
					if (inRetry >= 4)
					{
						inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
						inDISP_ChineseFont("請重新交易", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
						inDISP_BEEP(3, 1000);
						return (VS_ERROR);
					}

					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_ChineseFont("連線失敗", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
					inDISP_ChineseFont("請重新感應卡片", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
					inDISP_ChineseFont("並按確認鍵重試", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_CENTER_);

					szKey = 0x00;
					inDISP_Timer_Start(_TIMER_NEXSYS_1_, 60);

					while (1)
					{
						szKey = uszKBD_Key();
						if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
						{
							szKey = _KEY_TIMEOUT_;
						}

						if (szKey == _KEY_ENTER_)
						{
							inRetry++;
							guszECCRetryBit = VS_TRUE;
							break;
						}
						else if (szKey == _KEY_TIMEOUT_)
						{
							return (VS_ERROR);
						}
					}

					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_ChineseFont("交易處理中", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
					inDISP_ChineseFont("請重新感應卡片", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_CENTER_);

					continue;
				}
				else
				{
					/* 其他錯誤Display */
					inECC_ResponeCode(pobTran);

					return (VS_ERROR);
				}
			}/* 客製化 */
                }
		else
		{
			/* 每次unpack(交易，詢卡、查額、SignOn不算)就同步電文序號
			 * 成功交易才更新電文序號 */
			inNCCC_Ticket_Func_UpdatetTicketInvNum(pobTran);
		
			/* 未滿三次，授權成功要重置次數 */
			if (ginMacError < 3)
			{
				ginMacError = 0;
			}
		}
		
                /* 抓必要資料 */
		if (inECC_UNPACK_ICERAPI_NCCC(pobTran, _ECC_API_RES_FILE2_, _ECC_FOLDER_PATH_) != VS_SUCCESS)
		        return (VS_ERROR);    
		
                break;
        }
        
	/*
	 * T5501批次號碼之規則為yymmddss+流水號兩碼
	 * 以當批第一筆成功交易時間為準
	 * 如果NeedNewBatch是'Y'，代表是第一筆，更新狀態為不用更新批號
	 * 如果NeedNewBatch是'N'，代表有做過交易
	*/
	inECC_Batch_Update();

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inECC_Refund_Flow() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inECC_PACK_Refund_ICERAPI
Date&Time       :2018/4/2 下午 3:36
Describe        :
*/
int inECC_PACK_Refund_ICERAPI(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_ERROR;
	int		inESVCIndex = 0;
	char		szTemplate[20 + 1] = {0};
	char		szSTAN[6 + 1] = {0};
	char		szInvoiceNum[6 + 1] = {0};
	RTC_NEXSYS	srRTC = {0};
	xmlDocPtr       srDoc = NULL;
        xmlNodePtr      srRoot_Node = NULL, srTRANS_Node = NULL;
#if 	READER_MANUFACTURERS == LINUX_API
	unsigned long	ulHandle = 0;
#else
	int		inFd = 0;
#endif

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inECC_PACK_Refund_ICERAPI() START !");
	}
        
        if (inLoadTDTRec(_TDT_INDEX_01_ECC_) < 0)
                return (VS_ERROR);
	
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTicket_HostIndex(szTemplate);
	inESVCIndex = atoi(szTemplate);
        
        if (inLoadHDTRec(inESVCIndex) != VS_SUCCESS)
                return (VS_ERROR);
	if (inLoadHDPTRec(inESVCIndex) != VS_SUCCESS)
                return (VS_ERROR);
                
        if (inLoadECCDTRec(0) != VS_SUCCESS)
	{
                return (VS_ERROR);
	}
        
        /* 刪除原本的ICERAPI */
        inFILE_Delete((unsigned char*)_ECC_API_REQ_CHECK_);	/* 檢核用 */
	inFILE_Delete((unsigned char*)_ECC_API_REQ_FILE_);
	inFILE_Delete((unsigned char*)_ECC_API_REQ_FILE2_);
	inFILE_Delete((unsigned char*)_ECC_API_RES_FILE_);	/* 防止用到上次資料結果 */
	inFILE_Delete((unsigned char*)_ECC_API_RES_FILE2_);
	
	/* 建立一個新的檔案 */
        srDoc = xmlNewDoc(BAD_CAST "1.0");
        srRoot_Node = xmlNewNode(NULL, BAD_CAST "TransXML");
        xmlDocSetRootElement (srDoc, srRoot_Node);
 
        /* <TRANS> */
	srTRANS_Node = xmlNewNode(NULL, BAD_CAST "TRANS");
	xmlAddChild(srRoot_Node, srTRANS_Node);
	
	/* <T0100> */
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T0100", BAD_CAST "0200");

        /* <T0300> */
        xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T0300", BAD_CAST "620061");
        
	/* <T0400> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%lu00", pobTran->srTRec.lnTxnAmount);
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T0400", BAD_CAST szTemplate);

	if (guszECCRetryBit == VS_TRUE)
	{
		/* Retry 要同第一次，除了時間 */
                /* <T1100> : STAN 不知道用在哪，先比照NCCC STAN */
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1100", BAD_CAST gszECCRetryData.szT1100);

		/* <T1101> : CMAS自用，區分上筆交易(會回傳) */
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1101", BAD_CAST gszECCRetryData.szT1101);

		/* <T1102> : NCCC STAN */
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1102", BAD_CAST gszECCRetryData.szT1102);

		/* <T1103> : NCCC RRN(後三碼) */
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1103", BAD_CAST gszECCRetryData.szT1103);
                                            
                /* <T1200> : hhmmss */
		memset(&srRTC, 0x00, sizeof(srRTC));
		inFunc_GetSystemDateAndTime(&srRTC);
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%02d%02d%02d", srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1200", BAD_CAST szTemplate);

		/* <T1300> : yyyymmdd */
		/* hhmmss就抓過一次時間了，不用重抓 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "20%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1300", BAD_CAST szTemplate);
	}
	else
	{
		/* For Retry 重試時使用 */
		memset(&gszECCRetryData, 0x00, sizeof(gszECCRetryData));
		
		/* <T1100> : STAN 不知道用在哪，先比照NCCC STAN */
		memset(szSTAN, 0x00, sizeof(szSTAN));
		inGetSTANNum(szSTAN);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1100", BAD_CAST szSTAN);
		
		/* For Retry 重試時使用 */
		memcpy(gszECCRetryData.szT1100, szSTAN, strlen(szSTAN));

		/* <T1101> : CMAS自用，區分上筆交易(會回傳) */
		memset(szSTAN, 0x00, sizeof(szSTAN));
		inGetTicket_STAN(szSTAN);
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%06d", atoi(szSTAN) + 1);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1101", BAD_CAST szTemplate);
		inSetTicket_STAN(szTemplate);
		inSaveTDTRec(_TDT_INDEX_01_ECC_);

		/* For Retry 重試時使用 */
		memcpy(gszECCRetryData.szT1101, szTemplate, strlen(szTemplate));
		
		/* <T1102> : NCCC STAN */
		memset(szSTAN, 0x00, sizeof(szSTAN));
		inGetSTANNum(szSTAN);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1102", BAD_CAST szSTAN);
		
		/* For Retry 重試時使用 */
		memcpy(gszECCRetryData.szT1102, szSTAN, strlen(szSTAN));

		/* <T1103> : NCCC RRN(後三碼) */
		memset(szInvoiceNum, 0x00, sizeof(szInvoiceNum));
		inGetTicket_InvNum(szInvoiceNum);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1103", BAD_CAST szInvoiceNum);
		
		/* For Retry 重試時使用 */
		memcpy(gszECCRetryData.szT1103, szInvoiceNum, strlen(szInvoiceNum));

		/* <T1200> : hhmmss */
		memset(&srRTC, 0x00, sizeof(srRTC));
		inFunc_GetSystemDateAndTime(&srRTC);
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%02d%02d%02d", srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1200", BAD_CAST szTemplate);

		/* <T1300> : yyyymmdd */
		/* hhmmss就抓過一次時間了，不用重抓 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "20%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1300", BAD_CAST szTemplate);
	}
	
        /* <T4108> : LCD Control Flag 單純顯示畫面用 */
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T4108", BAD_CAST "0");

	/* <T4830> : Read Purse Flag 
	             0：不需同一票卡(default)
		     1：需為同一票卡
	*/
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T4830", BAD_CAST "0");
        
        /* <T5501> : Batch Number - 悠遊卡特有，同一天流水號累加，隔天就要歸1 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inECC_Batch_Check(szTemplate);
        xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T5501", BAD_CAST szTemplate);

	/* 儲存xml檔 xml檔現在會自動換行了 */
        inXML_SaveFile(_ECC_API_REQ_FILE_, &srDoc, "utf-8", 1);
	inFile_Move_File(_ECC_API_REQ_FILE_, _AP_ROOT_PATH_, "", _ECC_FOLDER_PATH_);
	
	/* 清空佔用記憶體 */
	inXML_End(&srDoc);
	
	/* 步驟完成，建立檢核擋 */
	#if 	READER_MANUFACTURERS == LINUX_API
	inRetVal = inFILE_Create(&ulHandle, (unsigned char*)_ECC_API_REQ_CHECK_);
	/* 不管成功或失敗都要關檔 */
	inFILE_Close(&ulHandle);
	/* 抓到ICERData資料夾 */
	inFile_Move_File(_ECC_API_REQ_CHECK_, _FS_DATA_PATH_, "", _ECC_FOLDER_PATH_);
#else
	inRetVal = inFile_Linux_Create(&inFd, _ECC_API_REQ_CHECK_);
	/* 不管成功或失敗都要關檔 */
	inFile_Linux_Close(inFd);
	/* 會有權限問題，要先改權限 */
	inFunc_Data_Chmod("u+rwx", _ECC_API_REQ_CHECK_, _AP_ROOT_PATH_);
	/* 抓到ICERData資料夾 */
	inFile_Move_File(_ECC_API_REQ_CHECK_, _AP_ROOT_PATH_, "", _ECC_FOLDER_PATH_);
#endif
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inECC_PACK_Refund_ICERAPI() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
        
        return (VS_SUCCESS);
}

/*
Function        :inECC_PACK_Refund_ICERAPI_NCCC
Date&Time       :2018/4/2 下午 3:35
Describe        :
*/
int inECC_PACK_Refund_ICERAPI_NCCC(TRANSACTION_OBJECT *pobTran)
{
	int		inDataLen = 0;
	int		inTxnCode = 0;
	int		inSendLen = 0;
	int		inField32 = 0;
	int		inField48 = 0;
	char		szFESMode[2 + 1] = {0};
	char		szIFESMode[2 + 1] = {0};
	char		szSendPacket[_NCCC_TICKET_ISO_SEND_ + 1] = {0};
	char		szTemplate[500 + 1] = {0};
	xmlDocPtr       srDoc = NULL;
        xmlNodePtr      srRoot_Node = NULL, sr620061_Node = NULL, sr620061adv_Node = NULL, sr620061rev_Node = NULL;

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inECC_PACK_Refund_ICERAPI_NCCC() START !");
	}
	
	memset(szFESMode, 0x00, sizeof(szFESMode));
	inGetNCCCFESMode(szFESMode);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetCloud_MFES(szTemplate);
	memset(szIFESMode, 0x00, sizeof(szIFESMode));
	inGetI_FES_Mode(szIFESMode);
        if (memcmp(szFESMode, _NCCC_05_ATS_MODE_, 2) == 0	||
	    memcmp(szFESMode, _NCCC_04_MPAS_MODE_, 2) == 0	||
	   (memcmp(szFESMode, _NCCC_03_MFES_MODE_, 2) == 0	&&
	    memcmp(szTemplate, "Y", strlen("Y")) == 0))
	{
                inField32 = _ECC_F32_LENTH_;
	}
                
        if (strlen(pobTran->srTRec.szStoreID) > 0)
	{
                inField48 = _ECC_F48_LENTH_;
	}
                
        pobTran->srTRec.inTicketType = _TICKET_TYPE_ECC_;
       
        memset(szSendPacket, 0x00, sizeof(szSendPacket));
        inTxnCode = _TICKET_EASYCARD_REFUND_;
        pobTran->inISOTxnCode = inTxnCode;
        
	/* 要Load CPT */
	inETHERNET_Begin(pobTran);
	
        /* 組 ISO 電文 */
	ginBeforeIndex = 0;
	if ((inSendLen = inNCCC_TICKET_PackISO(pobTran, (unsigned char *)szSendPacket, inTxnCode)) <= 0)
	{
		/* 上傳不能Show錯誤訊息 */
		pobTran->inErrorMsg = _ERROR_CODE_V3_NONE_;
		return (VS_ISO_PACK_ERR);
	}
	
	/* 回復交易碼 */
	pobTran->inISOTxnCode = pobTran->srTRec.inCode;
	
	/* 沒找到 */
	if (ginBeforeIndex == 0)
	{
	        return (VS_ERROR);
	}

        /* 刪除原本的ICERAPI */
	inFile_Unlink_File(_ECC_API_REQ_FILE2_, _ECC_FOLDER_PATH_);
	
	/* 建立一個新的檔案 */
        srDoc = xmlNewDoc(BAD_CAST "1.0");
        srRoot_Node = xmlNewNode(NULL, BAD_CAST "TransXML");
        xmlDocSetRootElement (srDoc, srRoot_Node);
	
        /* <620061>==================================================================================退貨0100 */
	sr620061_Node = xmlNewNode(NULL, BAD_CAST "620061");
	xmlAddChild(srRoot_Node, sr620061_Node);
        
	/* <BefData> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%s", "0000");
	inFunc_BCD_to_ASCII(&szTemplate[4], (unsigned char*)&szSendPacket[0], ginBeforeIndex);
        xmlNewChild(sr620061_Node, NULL, BAD_CAST "BefData", BAD_CAST szTemplate);
             
        /* <AftData> : 扣掉取代的Len + @@@@@ */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inFunc_BCD_to_ASCII(&szTemplate[0], (unsigned char*)&szSendPacket[ginBeforeIndex + 7], inSendLen - ginBeforeIndex - 7);
        xmlNewChild(sr620061_Node, NULL, BAD_CAST "AftData", BAD_CAST szTemplate);

	/* <F03Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr620061_Node, NULL, BAD_CAST "F03Adr", BAD_CAST szTemplate);
	
	/* <F04Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr620061_Node, NULL, BAD_CAST "F04Adr", BAD_CAST szTemplate);
        
	/* <F11Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr620061_Node, NULL, BAD_CAST "F11Adr", BAD_CAST szTemplate);
	
	/* <F35Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F22_LENTH_ + _ECC_F24_LENTH_ +
                    _ECC_F25_LENTH_ + inField32 + 1;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr620061_Node, NULL, BAD_CAST "F35Adr", BAD_CAST szTemplate);
        
        /* <F37Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F22_LENTH_ + _ECC_F24_LENTH_ +
                    _ECC_F25_LENTH_ + inField32 + _ECC_F35_LENTH_ + 8;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr620061_Node, NULL, BAD_CAST "F37Adr", BAD_CAST szTemplate);
        
	/* <F57Adr> */
	if (memcmp(szIFESMode, "Y", 1) == 0)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = ginECC_F57_Len;
		sprintf(szTemplate, "-%d", inDataLen);
		xmlNewChild(sr620061_Node, NULL, BAD_CAST "F57Adr", BAD_CAST szTemplate);
	}
	
        /* <F59Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = ginECC_F59_ET_Len;
	sprintf(szTemplate, "-%d", inDataLen);
        xmlNewChild(sr620061_Node, NULL, BAD_CAST "F59Adr", BAD_CAST szTemplate);

//	memset(szFESMode, 0x00, sizeof(szFESMode));
//	inGetNCCCFESMode(szFESMode);
//	memset(szTemplate, 0x00, sizeof(szTemplate));
//	inGetCloud_MFES(szTemplate);
//	if (memcmp(szFESMode, _NCCC_05_ATS_MODE_, 2) == 0	||
//	    memcmp(szFESMode, _NCCC_04_MPAS_MODE_, 2) == 0	||
//	   (memcmp(szFESMode, _NCCC_03_MFES_MODE_, 2) == 0	&&
//	    memcmp(szTemplate, "Y", strlen("Y")) == 0))
//	{
		/* ATS規格 - 回覆跟Host長度有關，目前一樣 */
		/* <F39Adr> */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
			    _ECC_F24_LENTH_ + inField32 + _ECC_F37_LENTH_ + _ECC_F38_LENTH_;
		sprintf(szTemplate, "%d", inDataLen);
		xmlNewChild(sr620061_Node, NULL, BAD_CAST "F39Adr", BAD_CAST szTemplate);

		/* <ECCAdr> */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_  + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
			    _ECC_F24_LENTH_ + inField32 + _ECC_F37_LENTH_ + _ECC_F38_LENTH_ + _ECC_F39_LENTH_ + _ECC_F41_LENTH_ + _ECC_F56_LENTH_;
		sprintf(szTemplate, "%d", inDataLen);
//	}
//	else
//	{
//		/* <F39Adr> */
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
//			    _ECC_F24_LENTH_;
//		sprintf(szTemplate, "%d", inDataLen);
//		xmlNewChild(sr620061_Node, NULL, BAD_CAST "F39Adr", BAD_CAST szTemplate);
//
//		/* <ECCAdr> */
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_  + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
//			    _ECC_F24_LENTH_ + _ECC_F39_LENTH_ + _ECC_F41_LENTH_ + _ECC_F56_LENTH_;
//		sprintf(szTemplate, "%d", inDataLen);
//	}
	
        xmlNewChild(sr620061_Node, NULL, BAD_CAST "ECCAdr", BAD_CAST szTemplate);

        /* <620061_Adv>==============================================================================退貨0220 */
        memset(szSendPacket, 0x00, sizeof(szSendPacket));
        inTxnCode = _ADVICE_;
        pobTran->inISOTxnCode = _ADVICE_;
            
        /* 組 ISO 電文 */
	ginBeforeIndex = 0;
	if ((inSendLen = inNCCC_TICKET_PackISO(pobTran, (unsigned char *)szSendPacket, inTxnCode)) <= 0)
	{
		/* 清空佔用記憶體 */
		inXML_End(&srDoc);
	
		/* 上傳不能Show錯誤訊息 */
		pobTran->inErrorMsg = _ERROR_CODE_V3_NONE_;
		return (VS_ISO_PACK_ERR);
	}
	
	/* 回復交易碼 */
	pobTran->inISOTxnCode = pobTran->srTRec.inCode;
	
	/* 沒找到 */
	if (ginBeforeIndex == 0)
	{
	        return (VS_ERROR);
	}
	        
        /* <620061_Adv> */
	sr620061adv_Node = xmlNewNode(NULL, BAD_CAST "620061_Adv");
	xmlAddChild(srRoot_Node, sr620061adv_Node);
        
	/* <BefData> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%s", "0000");
	inFunc_BCD_to_ASCII(&szTemplate[4], (unsigned char*)&szSendPacket[0], ginBeforeIndex);
        xmlNewChild(sr620061adv_Node, NULL, BAD_CAST "BefData", BAD_CAST szTemplate);
             
        /* <AftData> : 扣掉取代的Len + @@@@@ */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inFunc_BCD_to_ASCII(&szTemplate[0], (unsigned char*)&szSendPacket[ginBeforeIndex + 7], inSendLen - ginBeforeIndex - 7);
        xmlNewChild(sr620061adv_Node, NULL, BAD_CAST "AftData", BAD_CAST szTemplate);

	/* <F03Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr620061adv_Node, NULL, BAD_CAST "F03Adr", BAD_CAST szTemplate);
	
	/* <F04Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr620061adv_Node, NULL, BAD_CAST "F04Adr", BAD_CAST szTemplate);
        
	/* <F11Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr620061adv_Node, NULL, BAD_CAST "F11Adr", BAD_CAST szTemplate);

        /* <F12Adr> */
        memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr620061adv_Node, NULL, BAD_CAST "F12Adr", BAD_CAST szTemplate);
        
        /* <F13Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr620061adv_Node, NULL, BAD_CAST "F13Adr", BAD_CAST szTemplate);
        
	/* <F35Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
		    _ECC_F22_LENTH_ + _ECC_F24_LENTH_ + _ECC_F25_LENTH_ + inField32 + 1;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr620061adv_Node, NULL, BAD_CAST "F35Adr", BAD_CAST szTemplate);
        
        /* <F37Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
		    _ECC_F22_LENTH_ + _ECC_F24_LENTH_ + _ECC_F25_LENTH_ + inField32 + _ECC_F35_LENTH_ + 8;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr620061adv_Node, NULL, BAD_CAST "F37Adr", BAD_CAST szTemplate);

	/* <F57Adr> */
	if (memcmp(szIFESMode, "Y", 1) == 0)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = ginECC_F57_Len;
		sprintf(szTemplate, "-%d", inDataLen);
		xmlNewChild(sr620061adv_Node, NULL, BAD_CAST "F57Adr", BAD_CAST szTemplate);
	}
	
	 /* <F59Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = ginECC_F59_ET_Len;
	sprintf(szTemplate, "-%d", inDataLen);
        xmlNewChild(sr620061adv_Node, NULL, BAD_CAST "F59Adr", BAD_CAST szTemplate);

//	memset(szFESMode, 0x00, sizeof(szFESMode));
//	inGetNCCCFESMode(szFESMode);
//	memset(szTemplate, 0x00, sizeof(szTemplate));
//	inGetCloud_MFES(szTemplate);
//        if (memcmp(szFESMode, _NCCC_05_ATS_MODE_, 2) == 0	||
//	    memcmp(szFESMode, _NCCC_04_MPAS_MODE_, 2) == 0	||
//	   (memcmp(szFESMode, _NCCC_03_MFES_MODE_, 2) == 0	&&
//	    memcmp(szTemplate, "Y", strlen("Y")) == 0))
//	{
		/* <F39Adr> */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F24_LENTH_ + inField32 + 
			    _ECC_F37_LENTH_ + _ECC_F38_LENTH_;
		sprintf(szTemplate, "%d", inDataLen);
		xmlNewChild(sr620061adv_Node, NULL, BAD_CAST "F39Adr", BAD_CAST szTemplate);

		/* <ECCAdr> */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_  + _ECC_F11_LENTH_ + _ECC_F24_LENTH_ + inField32 + 
			    _ECC_F37_LENTH_ + _ECC_F38_LENTH_ + _ECC_F39_LENTH_ + _ECC_F41_LENTH_ + _ECC_F56_LENTH_;
		sprintf(szTemplate, "%d", inDataLen);
		xmlNewChild(sr620061adv_Node, NULL, BAD_CAST "ECCAdr", BAD_CAST szTemplate);
//	}
//	else
//	{
//		/* <F39Adr> */
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F24_LENTH_;
//		sprintf(szTemplate, "%d", inDataLen);
//		xmlNewChild(sr620061adv_Node, NULL, BAD_CAST "F39Adr", BAD_CAST szTemplate);
//
//		/* <ECCAdr> */
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_  + _ECC_F11_LENTH_ + _ECC_F24_LENTH_ + 
//			    _ECC_F39_LENTH_ + _ECC_F41_LENTH_ + _ECC_F56_LENTH_;
//		sprintf(szTemplate, "%d", inDataLen);
//		xmlNewChild(sr620061adv_Node, NULL, BAD_CAST "ECCAdr", BAD_CAST szTemplate);
//	}
	
        /* <620061_Rev>==============================================================================退貨0400 */
        memset(szSendPacket, 0x00, sizeof(szSendPacket));
        inTxnCode = _REVERSAL_;
        pobTran->inISOTxnCode = _REVERSAL_;
  
        /* 組 ISO 電文 */
	ginBeforeIndex = 0;
	if ((inSendLen = inNCCC_TICKET_PackISO(pobTran, (unsigned char *)szSendPacket, inTxnCode)) <= 0)
	{
		/* 清空佔用記憶體 */
		inXML_End(&srDoc);
	
		/* 上傳不能Show錯誤訊息 */
		pobTran->inErrorMsg = _ERROR_CODE_V3_NONE_;
		return (VS_ISO_PACK_ERR);
	}
	
	/* 回復交易碼 */
	pobTran->inISOTxnCode = pobTran->srTRec.inCode;
	
	/* 沒找到 */
	if (ginBeforeIndex == 0)
	        return (VS_ERROR);
	        
        /* <620061_Rev> */
	sr620061rev_Node = xmlNewNode(NULL, BAD_CAST "620061_Rev");
	xmlAddChild(srRoot_Node, sr620061rev_Node);
        
	/* <BefData> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%s", "0000");
	inFunc_BCD_to_ASCII(&szTemplate[4], (unsigned char*)&szSendPacket[0], ginBeforeIndex);
        xmlNewChild(sr620061rev_Node, NULL, BAD_CAST "BefData", BAD_CAST szTemplate);
             
        /* <AftData> : 扣掉取代的Len + @@@@@ */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inFunc_BCD_to_ASCII(&szTemplate[0], (unsigned char*)&szSendPacket[ginBeforeIndex + 7], inSendLen - ginBeforeIndex - 7);
        xmlNewChild(sr620061rev_Node, NULL, BAD_CAST "AftData", BAD_CAST szTemplate);

	/* <F03Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr620061rev_Node, NULL, BAD_CAST "F03Adr", BAD_CAST szTemplate);
	
	/* <F04Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr620061rev_Node, NULL, BAD_CAST "F04Adr", BAD_CAST szTemplate);
        
	/* <F11Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr620061rev_Node, NULL, BAD_CAST "F11Adr", BAD_CAST szTemplate);
	
	/* <F35Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F22_LENTH_ + _ECC_F24_LENTH_ +
                    _ECC_F25_LENTH_ + inField32 + 1;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr620061rev_Node, NULL, BAD_CAST "F35Adr", BAD_CAST szTemplate);
        
        /* <F37Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F22_LENTH_ + _ECC_F24_LENTH_ +
                    _ECC_F25_LENTH_ + inField32 + _ECC_F35_LENTH_ + 8;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr620061rev_Node, NULL, BAD_CAST "F37Adr", BAD_CAST szTemplate);
        
	/* <F57Adr> */
	if (memcmp(szIFESMode, "Y", 1) == 0)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = ginECC_F57_Len;
		sprintf(szTemplate, "-%d", inDataLen);
		xmlNewChild(sr620061rev_Node, NULL, BAD_CAST "F57Adr", BAD_CAST szTemplate);
	}
	
        /* <F59Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = ginECC_F59_ET_Len;
	sprintf(szTemplate, "-%d", inDataLen);
        xmlNewChild(sr620061rev_Node, NULL, BAD_CAST "F59Adr", BAD_CAST szTemplate);

//	memset(szFESMode, 0x00, sizeof(szFESMode));
//	inGetNCCCFESMode(szFESMode);
//	memset(szTemplate, 0x00, sizeof(szTemplate));
//	inGetCloud_MFES(szTemplate);
//	if (memcmp(szFESMode, _NCCC_05_ATS_MODE_, 2) == 0	||
//	    memcmp(szFESMode, _NCCC_04_MPAS_MODE_, 2) == 0	||
//	   (memcmp(szFESMode, _NCCC_03_MFES_MODE_, 2) == 0	&&
//	    memcmp(szTemplate, "Y", strlen("Y")) == 0))
//	{
		/* <F39Adr> */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
			    _ECC_F24_LENTH_ + inField32 + _ECC_F37_LENTH_ + _ECC_F38_LENTH_;
		sprintf(szTemplate, "%d", inDataLen);
		xmlNewChild(sr620061rev_Node, NULL, BAD_CAST "F39Adr", BAD_CAST szTemplate);

		/* <ECCAdr> */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_  + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
			    _ECC_F24_LENTH_ + inField32 + _ECC_F37_LENTH_ + _ECC_F39_LENTH_ + _ECC_F41_LENTH_ + _ECC_F56_LENTH_;
		sprintf(szTemplate, "%d", inDataLen);
		xmlNewChild(sr620061rev_Node, NULL, BAD_CAST "ECCAdr", BAD_CAST szTemplate);
//	}
//	else
//	{
//		/* <F39Adr> */
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
//			    _ECC_F24_LENTH_;
//		sprintf(szTemplate, "%d", inDataLen);
//		xmlNewChild(sr620061rev_Node, NULL, BAD_CAST "F39Adr", BAD_CAST szTemplate);
//
//		/* <ECCAdr> */
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_  + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
//			    _ECC_F24_LENTH_ + _ECC_F39_LENTH_ + _ECC_F41_LENTH_ + _ECC_F56_LENTH_;
//		sprintf(szTemplate, "%d", inDataLen);
//		xmlNewChild(sr620061rev_Node, NULL, BAD_CAST "ECCAdr", BAD_CAST szTemplate);
//	}
	
	/* 儲存xml檔 xml檔現在會自動換行了 */
        inXML_SaveFile(_ECC_API_REQ_FILE2_, &srDoc, "utf-8", 1);
	inFile_Move_File(_ECC_API_REQ_FILE2_, _AP_ROOT_PATH_, "", _ECC_FOLDER_PATH_);
	
	/* 清空佔用記憶體 */
	inXML_End(&srDoc);
        
	if (inECC_Insert_Temp() != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inECC_PACK_Refund_ICERAPI_NCCC() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inECC_TOP_UP_Flow
Date&Time       :2018/4/9 下午 4:12
Describe        :
*/
int inECC_TOP_UP_Flow(TRANSACTION_OBJECT *pobTran)
{
        int		inRetVal = VS_ERROR;
	int		inRetry = 1;
	char		szKey = 0x00;
	char		szCustomerIndicator[3 + 1] = {0};
	char		szDebugMsg[100 + 1] = {0};
	unsigned char	uszECRAlreadyBit = VS_FALSE;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inECC_TOP_UP_Flow() START !");
	}
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	guszECCRetryBit = VS_FALSE;
	
	while (inRetry <= 5)
	{
		/* (需求單 - 107227)邦柏科技自助作業客製化 by Russell 2018/12/13 下午 8:33 */
		/* (需求單 - 107276)自助交易標準400做法 by Russell 2018/12/27 上午 11:20 */
		if ((!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)           ||
		     !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_)   ||
                     !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)        ||
		     !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)	||
		     !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)		||
		     !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))	&&
		     pobTran->uszLastTranscationBit == VS_TRUE)
		{
			uszECRAlreadyBit = VS_TRUE;
			inECC_PACK_RETRY_ICERAPI(pobTran);
		}
		else
		{
			/* 組電文(生成ICERAPI.REQ) */
			inECC_PACK_TOP_UP_ICERAPI(pobTran);
		}
	        
	        /* AdditionalTcpipData */
	        inECC_PACK_TOP_UP_ICERAPI_NCCC(pobTran);

                if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "ICER Start");
			inLogPrintf(AT, szDebugMsg);
		}
		
		inRetVal = ICERApi_exe();
		
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "ICER End");
			inLogPrintf(AT, szDebugMsg);
		}
                
		/* 解電文(分析ICERAPI.RES) */
		inRetVal = inECC_UNPACK_ICERAPI(pobTran, _ECC_API_RES_FILE_);	
		
		if (inRetVal != VS_SUCCESS)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inCTLS_Set_LED(_CTLS_LIGHT_RED_);
                        inDISP_ChineseFont("悠遊卡檔案異常", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
                        inDISP_BEEP(3, 1000);
			
                        return (VS_ERROR);
                }
                
		/* 檢核回應碼 <T3901>開始 */
	        if (memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "0", 1) != 0)
	        {
			if (!memcmp(pobTran->srTRec.szRespCode, "A0", 2)	||
			    !memcmp(pobTran->srTRec.szRespCode, "A1", 2))
			{
				ginMacError++;
				inECC_UNPACK_ICERAPI_NCCC(pobTran, _ECC_API_RES_FILE2_, _ECC_FOLDER_PATH_);
			}
			
			/* (需求單 - 107227)邦柏科技自助作業客製化 by Russell 2018/12/9 下午 2:42 */
			/* (需求單 - 107276)自助交易標準400做法 by Russell 2018/12/27 上午 11:20 */
			if ((!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)           ||
			     !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_)	||
			     !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)		||
			     !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))	&&
			     pobTran->uszECRBit == VS_TRUE)
			{
				/* 第二次發生RETRY 回傳失敗 */
	        		if (uszECRAlreadyBit == VS_TRUE)
				{
	        			return (VS_ERROR);
				}
				
				if (memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-125", 4) == 0)
				{
	                                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_ChineseFont("請重新交易", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
					inDISP_BEEP(3, 1000);

	                                /* 要回錯誤碼0018不是0001 所以Reset */
	                                memset(pobTran->szTicket_ErrorCode, 0x00, sizeof(pobTran->szTicket_ErrorCode));
					pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_ESVC_RE_TAP_CARD_;

					/* 將現在組好的REQ 備份成ICERRETRY.REQ */
					inFunc_Data_Copy(_ECC_API_REQ_FILE_, _ECC_FOLDER_PATH_, _ECC_API_REQ_RETRY_FILE_, _ECC_FOLDER_PATH_);

//					/* 將pobTran的MFRec備份部分資料 */
//					inECC_SAVE_TREC(pobTran);
					
	                                return (VS_ERROR);
	                        }
				else
				{	
					/* 其他錯誤Display */
					inECC_ResponeCode(pobTran);

					return (VS_ERROR);
				}
			}
                        else if ((!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)       ||
			          !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_))	&&
			         pobTran->uszECRBit == VS_TRUE)
			{
				/* 第二次發生RETRY 回傳失敗 */
	        		if (uszECRAlreadyBit == VS_TRUE)
				{
	        			return (VS_ERROR);
				}
				
				if (memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-125", 4) == 0)
				{
	                                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_ChineseFont("請重新交易", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
					inDISP_BEEP(3, 1000);

	                                /* 要回錯誤碼0018不是0001 所以Reset */
	                                memset(pobTran->szTicket_ErrorCode, 0x00, sizeof(pobTran->szTicket_ErrorCode));
					pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_ESVC_RE_TAP_CARD_;

					/* 將現在組好的REQ 備份成ICERRETRY.REQ */
					inFunc_Data_Copy(_ECC_API_REQ_FILE_, _ECC_FOLDER_PATH_, _ECC_API_REQ_RETRY_FILE_, _ECC_FOLDER_PATH_);

//					/* 將pobTran的MFRec備份部分資料 */
//					inECC_SAVE_TREC(pobTran);
                                        
                                        pobTran->uszECCRetryBit = VS_TRUE;
                                        
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "ECC Retry....");
                                        }
					
	                                return (VS_ERROR);
	                        }
				else
				{	
					/* 其他錯誤Display */
					inECC_ResponeCode(pobTran);

					return (VS_ERROR);
				}
			}
			else
			{
				/* -125 = Retry */
				/* -119 : Call相關DLL回應錯誤 ; 請參考T3904 */
				/* 62 : RC531錯誤，交易中止，請重新交易*/
				if ((memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-125", 4) == 0)	||
				    (memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-119", 4) == 0	&&
				     memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "62", 2) == 0	&&
				     guszECCRetryBit == VS_TRUE))
				{
					if (inRetry >= 4)
					{
						inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
						inDISP_ChineseFont("請重新交易", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
						inDISP_BEEP(3, 1000);
						return (VS_ERROR);
					}

					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_ChineseFont("連線失敗", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
					inDISP_ChineseFont("請重新感應卡片", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
					inDISP_ChineseFont("並按確認鍵重試", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_CENTER_);

					szKey = 0x00;
					inDISP_Timer_Start(_TIMER_NEXSYS_1_, 60);

					while (1)
					{
						szKey = uszKBD_Key();
						if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
						{
							szKey = _KEY_TIMEOUT_;
						}

						if (szKey == _KEY_ENTER_)
						{
							inRetry++;
							guszECCRetryBit = VS_TRUE;
							break;
						}
						else if (szKey == _KEY_TIMEOUT_)
						{
							return (VS_ERROR);
						}
					}

					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_ChineseFont("交易處理中", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
					inDISP_ChineseFont("請重新感應卡片", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_CENTER_);

					continue;
				}
				else
				{
					/* 其他錯誤Display */
					inECC_ResponeCode(pobTran);

					return (VS_ERROR);
				}
			}/* 客製化 */
                }
		else
		{
			/* 每次unpack(交易，詢卡、查額、SignOn不算)就同步電文序號
			 * 成功交易才更新電文序號 */
			inNCCC_Ticket_Func_UpdatetTicketInvNum(pobTran);
		
			/* 未滿三次，授權成功要重置次數 */
			if (ginMacError < 3)
			{
				ginMacError = 0;
			}
		}
		
                /* 抓必要資料 */
		if (inECC_UNPACK_ICERAPI_NCCC(pobTran, _ECC_API_RES_FILE2_, _ECC_FOLDER_PATH_) != VS_SUCCESS)
		        return (VS_ERROR);    
		
                break;
        }
	
        /*
	 * T5501批次號碼之規則為yymmddss+流水號兩碼
	 * 以當批第一筆成功交易時間為準
	 * 如果NeedNewBatch是'Y'，代表是第一筆，更新狀態為不用更新批號
	 * 如果NeedNewBatch是'N'，代表有做過交易
	*/
	inECC_Batch_Update();
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inECC_TOP_UP_Flow() END !");
		inLogPrintf(AT, "----------------------------------------");
	}

        return (VS_SUCCESS);
}

/*
Function        :inECC_PACK_TOP_UP_ICERAPI
Date&Time       :2018/4/9 下午 4:27
Describe        :
*/
int inECC_PACK_TOP_UP_ICERAPI(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_ERROR;
	int		inESVCIndex = 0;
	char		szTemplate[20 + 1] = {0};
	char		szSTAN[6 + 1] = {0};
	char		szInvoiceNum[6 + 1] = {0};
	RTC_NEXSYS	srRTC = {0};
	xmlDocPtr       srDoc = NULL;
        xmlNodePtr      srRoot_Node = NULL, srTRANS_Node = NULL;
#if 	READER_MANUFACTURERS == LINUX_API
	unsigned long	ulHandle = 0;
#else
	int		inFd = 0;
#endif

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inECC_PACK_TOP_UP_ICERAPI() START !");
	}
        
	if (inLoadTDTRec(_TDT_INDEX_01_ECC_) < 0)
                return (VS_ERROR);
	
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTicket_HostIndex(szTemplate);
	inESVCIndex = atoi(szTemplate);
        
        if (inLoadHDTRec(inESVCIndex) != VS_SUCCESS)
                return (VS_ERROR);
	if (inLoadHDPTRec(inESVCIndex) != VS_SUCCESS)
                return (VS_ERROR);
                
        if (inLoadECCDTRec(0) != VS_SUCCESS)
	{
                return (VS_ERROR);
	}
        
        /* 刪除原本的ICERAPI */
        inFile_Unlink_File(_ECC_API_REQ_CHECK_, _ECC_FOLDER_PATH_);	/* 檢核用 */
	inFile_Unlink_File(_ECC_API_REQ_FILE_, _ECC_FOLDER_PATH_);
	inFile_Unlink_File(_ECC_API_REQ_FILE2_, _ECC_FOLDER_PATH_);
	inFile_Unlink_File(_ECC_API_RES_FILE_, _ECC_FOLDER_PATH_);	/* 防止用到上次資料結果 */
	/* Retry時需要參考RES2，不能砍 */
	if (guszECCRetryBit == VS_TRUE)
	{
		
	}
	else
	{
		inFile_Unlink_File(_ECC_API_RES_FILE2_, _ECC_FOLDER_PATH_);
	}
	
	/* 建立一個新的檔案 */
        srDoc = xmlNewDoc(BAD_CAST "1.0");
        srRoot_Node = xmlNewNode(NULL, BAD_CAST "TransXML");
        xmlDocSetRootElement (srDoc, srRoot_Node);
	
        /* <TRANS> */
	srTRANS_Node = xmlNewNode(NULL, BAD_CAST "TRANS");
	xmlAddChild(srRoot_Node, srTRANS_Node);

        /* <T0100> */
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T0100", BAD_CAST "0200");

        /* <T0300> */
        xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T0300", BAD_CAST "801061");
        
	/* <T0400> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%lu00", pobTran->srTRec.lnTxnAmount);
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T0400", BAD_CAST szTemplate);

	if (guszECCRetryBit == VS_TRUE)
	{
		/* Retry 要同第一次，除了時間 */
                /* <T1100> : STAN 不知道用在哪，先比照NCCC STAN */
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1100", BAD_CAST gszECCRetryData.szT1100);

		/* <T1101> : CMAS自用，區分上筆交易(會回傳) */
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1101", BAD_CAST gszECCRetryData.szT1101);

		/* <T1102> : NCCC STAN */
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1102", BAD_CAST gszECCRetryData.szT1102);

		/* <T1103> : NCCC RRN(後三碼) */
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1103", BAD_CAST gszECCRetryData.szT1103);
                                            
                /* <T1200> : hhmmss */
		memset(&srRTC, 0x00, sizeof(srRTC));
		inFunc_GetSystemDateAndTime(&srRTC);
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%02d%02d%02d", srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1200", BAD_CAST szTemplate);

		/* <T1300> : yyyymmdd */
		/* hhmmss就抓過一次時間了，不用重抓 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "20%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1300", BAD_CAST szTemplate);
	}
	else
	{
		/* For Retry 重試時使用 */
		memset(&gszECCRetryData, 0x00, sizeof(gszECCRetryData));
		
		/* <T1100> : STAN 不知道用在哪，先比照NCCC STAN */
		memset(szSTAN, 0x00, sizeof(szSTAN));
		inGetSTANNum(szSTAN);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1100", BAD_CAST szSTAN);
		
		/* For Retry 重試時使用 */
		memcpy(gszECCRetryData.szT1100, szSTAN, strlen(szSTAN));

		/* <T1101> : CMAS自用，區分上筆交易(會回傳) */
		memset(szSTAN, 0x00, sizeof(szSTAN));
		inGetTicket_STAN(szSTAN);
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%06d", atoi(szSTAN) + 1);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1101", BAD_CAST szTemplate);
		inSetTicket_STAN(szTemplate);
		inSaveTDTRec(_TDT_INDEX_01_ECC_);

		/* For Retry 重試時使用 */
		memcpy(gszECCRetryData.szT1101, szTemplate, strlen(szTemplate));
		
		/* <T1102> : NCCC STAN */
		memset(szSTAN, 0x00, sizeof(szSTAN));
		inGetSTANNum(szSTAN);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1102", BAD_CAST szSTAN);
		
		/* For Retry 重試時使用 */
		memcpy(gszECCRetryData.szT1102, szSTAN, strlen(szSTAN));

		/* <T1103> : NCCC RRN(後三碼) */
		memset(szInvoiceNum, 0x00, sizeof(szInvoiceNum));
		inGetTicket_InvNum(szInvoiceNum);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1103", BAD_CAST szInvoiceNum);
		
		/* For Retry 重試時使用 */
		memcpy(gszECCRetryData.szT1103, szInvoiceNum, strlen(szInvoiceNum));

		/* <T1200> : hhmmss */
		memset(&srRTC, 0x00, sizeof(srRTC));
		inFunc_GetSystemDateAndTime(&srRTC);
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%02d%02d%02d", srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1200", BAD_CAST szTemplate);

		/* <T1300> : yyyymmdd */
		/* hhmmss就抓過一次時間了，不用重抓 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "20%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1300", BAD_CAST szTemplate);
	}

        /* <T4108> : LCD Control Flag 單純顯示畫面用 */
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T4108", BAD_CAST "0");

	/* <T4830> : Read Purse Flag 
	             0：不需同一票卡(default)
		     1：需為同一票卡
	*/
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T4830", BAD_CAST "0");
        
        /* <T5501> : Batch Number - 悠遊卡特有，同一天流水號累加，隔天就要歸1 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inECC_Batch_Check(szTemplate);
        xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T5501", BAD_CAST szTemplate);
        
	/* 儲存xml檔 xml檔現在會自動換行了 */
        inXML_SaveFile(_ECC_API_REQ_FILE_, &srDoc, "utf-8", 1);
	inFile_Move_File(_ECC_API_REQ_FILE_, _AP_ROOT_PATH_, "", _ECC_FOLDER_PATH_);
	
	/* 清空佔用記憶體 */
	inXML_End(&srDoc);
	
	/* 步驟完成，建立檢核擋 */
#if 	READER_MANUFACTURERS == LINUX_API
	inRetVal = inFILE_Create(&ulHandle, (unsigned char*)_ECC_API_REQ_CHECK_);
	/* 不管成功或失敗都要關檔 */
	inFILE_Close(&ulHandle);
	/* 抓到ICERData資料夾 */
	inFile_Move_File(_ECC_API_REQ_CHECK_, _FS_DATA_PATH_, "", _ECC_FOLDER_PATH_);
#else
	inRetVal = inFile_Linux_Create(&inFd, _ECC_API_REQ_CHECK_);
	/* 不管成功或失敗都要關檔 */
	inFile_Linux_Close(inFd);
	/* 會有權限問題，要先改權限 */
	inFunc_Data_Chmod("u+rwx", _ECC_API_REQ_CHECK_, _AP_ROOT_PATH_);
	/* 抓到ICERData資料夾 */
	inFile_Move_File(_ECC_API_REQ_CHECK_, _AP_ROOT_PATH_, "", _ECC_FOLDER_PATH_);
#endif
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inECC_PACK_TOP_UP_ICERAPI() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inECC_PACK_TOP_UP_ICERAPI_NCCC
Date&Time       :2018/4/9 下午 5:46
Describe        :
*/
int inECC_PACK_TOP_UP_ICERAPI_NCCC(TRANSACTION_OBJECT *pobTran)
{
	int		inDataLen = 0;
	int		inTxnCode = 0;
	int		inSendLen = 0;
	int		inField32 = 0;
	int		inField48 = 0;
	char		szFESMode[2 + 1] = {0};
	char		szIFESMode[2 + 1] = {0};
	char		szSendPacket[_NCCC_TICKET_ISO_SEND_ + 1] = {0};
	char		szTemplate[500 + 1] = {0};
	xmlDocPtr       srDoc = NULL;
        xmlNodePtr      srRoot_Node = NULL, sr801061_Node = NULL, sr801061adv_Node = NULL, sr801061rev_Node = NULL;

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inECC_PACK_TOP_UP_ICERAPI_NCCC() START !");
	}
        
	memset(szFESMode, 0x00, sizeof(szFESMode));
	inGetNCCCFESMode(szFESMode);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetCloud_MFES(szTemplate);
	memset(szIFESMode, 0x00, sizeof(szIFESMode));
	inGetI_FES_Mode(szIFESMode);
        if (memcmp(szFESMode, _NCCC_05_ATS_MODE_, 2) == 0	||
	    memcmp(szFESMode, _NCCC_04_MPAS_MODE_, 2) == 0	||
	   (memcmp(szFESMode, _NCCC_03_MFES_MODE_, 2) == 0	&&
	    memcmp(szTemplate, "Y", strlen("Y")) == 0))
	{
                inField32 = _ECC_F32_LENTH_;
	}
                
        if (strlen(pobTran->srTRec.szStoreID) > 0)
	{
                inField48 = _ECC_F48_LENTH_;
	}
                
        pobTran->srTRec.inTicketType = _TICKET_TYPE_ECC_;
       
        memset(szSendPacket, 0x00, sizeof(szSendPacket));
        inTxnCode = _TICKET_EASYCARD_TOP_UP_;
        pobTran->inISOTxnCode = inTxnCode;
        
	/* 要Load CPT */
	inETHERNET_Begin(pobTran);
	
        /* 組 ISO 電文 */
	ginBeforeIndex = 0;
	if ((inSendLen = inNCCC_TICKET_PackISO(pobTran, (unsigned char *)szSendPacket, inTxnCode)) <= 0)
	{
		/* 上傳不能Show錯誤訊息 */
		pobTran->inErrorMsg = _ERROR_CODE_V3_NONE_;
		return (VS_ISO_PACK_ERR);
	}
	
	/* 回復交易碼 */
	pobTran->inISOTxnCode = pobTran->srTRec.inCode;
	
	/* 沒找到 */
	if (ginBeforeIndex == 0)
	{
	        return (VS_ERROR);
	}

        /* 刪除原本的ICERAPI */
	inFile_Unlink_File(_ECC_API_REQ_FILE2_, _ECC_FOLDER_PATH_);
	
	/* 建立一個新的檔案 */
        srDoc = xmlNewDoc(BAD_CAST "1.0");
        srRoot_Node = xmlNewNode(NULL, BAD_CAST "TransXML");
        xmlDocSetRootElement (srDoc, srRoot_Node);

        /* <801061> =================================================================================手動加值0100 */
	sr801061_Node = xmlNewNode(NULL, BAD_CAST "801061");
	xmlAddChild(srRoot_Node, sr801061_Node);
        
	/* <BefData> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%s", "0000");
	inFunc_BCD_to_ASCII(&szTemplate[4], (unsigned char*)&szSendPacket[0], ginBeforeIndex);
        xmlNewChild(sr801061_Node, NULL, BAD_CAST "BefData", BAD_CAST szTemplate);
             
        /* <AftData> : 扣掉取代的Len + @@@@@ */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inFunc_BCD_to_ASCII(&szTemplate[0], (unsigned char*)&szSendPacket[ginBeforeIndex + 7], inSendLen - ginBeforeIndex - 7);
        xmlNewChild(sr801061_Node, NULL, BAD_CAST "AftData", BAD_CAST szTemplate);

	/* <F03Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr801061_Node, NULL, BAD_CAST "F03Adr", BAD_CAST szTemplate);
	
	/* <F04Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr801061_Node, NULL, BAD_CAST "F04Adr", BAD_CAST szTemplate);
        
	/* <F11Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr801061_Node, NULL, BAD_CAST "F11Adr", BAD_CAST szTemplate);
	
	/* <F35Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F22_LENTH_ + _ECC_F24_LENTH_ +
                    _ECC_F25_LENTH_ + inField32 + 1;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr801061_Node, NULL, BAD_CAST "F35Adr", BAD_CAST szTemplate);
        
        /* <F37Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F22_LENTH_ + _ECC_F24_LENTH_ +
                    _ECC_F25_LENTH_ + inField32 + _ECC_F35_LENTH_ + 8;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr801061_Node, NULL, BAD_CAST "F37Adr", BAD_CAST szTemplate);
        
	/* <F57Adr> */
	if (memcmp(szIFESMode, "Y", 1) == 0)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = ginECC_F57_Len;
		sprintf(szTemplate, "-%d", inDataLen);
		xmlNewChild(sr801061_Node, NULL, BAD_CAST "F57Adr", BAD_CAST szTemplate);
	}
	
        /* <F59Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = ginECC_F59_ET_Len;
	sprintf(szTemplate, "-%d", inDataLen);
        xmlNewChild(sr801061_Node, NULL, BAD_CAST "F59Adr", BAD_CAST szTemplate);

//	memset(szFESMode, 0x00, sizeof(szFESMode));
//	inGetNCCCFESMode(szFESMode);
//	memset(szTemplate, 0x00, sizeof(szTemplate));
//	inGetCloud_MFES(szTemplate);
//	if (memcmp(szFESMode, _NCCC_05_ATS_MODE_, 2) == 0	||
//	    memcmp(szFESMode, _NCCC_04_MPAS_MODE_, 2) == 0	||
//	   (memcmp(szFESMode, _NCCC_03_MFES_MODE_, 2) == 0	&&
//	    memcmp(szTemplate, "Y", strlen("Y")) == 0))
//	{
		/* ATS規格 - 回覆跟Host長度有關，目前一樣 */
		/* <F39Adr> */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
			    _ECC_F24_LENTH_ + inField32 + _ECC_F37_LENTH_ + _ECC_F38_LENTH_;
		sprintf(szTemplate, "%d", inDataLen);
		xmlNewChild(sr801061_Node, NULL, BAD_CAST "F39Adr", BAD_CAST szTemplate);

		/* <ECCAdr> */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_  + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
			    _ECC_F24_LENTH_ + inField32 + _ECC_F37_LENTH_ + _ECC_F38_LENTH_ + _ECC_F39_LENTH_ + _ECC_F41_LENTH_ + _ECC_F56_LENTH_;
		sprintf(szTemplate, "%d", inDataLen);
		xmlNewChild(sr801061_Node, NULL, BAD_CAST "ECCAdr", BAD_CAST szTemplate);
//	}
//	else
//	{
//		/* MFES規格 */
//		/* <F39Adr> */
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
//			    _ECC_F24_LENTH_;
//		sprintf(szTemplate, "%d", inDataLen);
//		xmlNewChild(sr801061_Node, NULL, BAD_CAST "F39Adr", BAD_CAST szTemplate);
//
//		/* <ECCAdr> */
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_  + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
//			    _ECC_F24_LENTH_ + _ECC_F39_LENTH_ + _ECC_F41_LENTH + _ECC_F56_LENTH_;
//		sprintf(szTemplate, "%d", inDataLen);
//		xmlNewChild(sr801061_Node, NULL, BAD_CAST "ECCAdr", BAD_CAST szTemplate);
//	}
	
        /* <801061_Adv>==============================================================================手動加值0220 */
        memset(szSendPacket, 0x00, sizeof(szSendPacket));
        inTxnCode = _ADVICE_;
        pobTran->inISOTxnCode = _ADVICE_;
            
        /* 組 ISO 電文 */
	ginBeforeIndex = 0;
	if ((inSendLen = inNCCC_TICKET_PackISO(pobTran, (unsigned char *)szSendPacket, inTxnCode)) <= 0)
	{
		/* 清空佔用記憶體 */
		inXML_End(&srDoc);
	
		/* 上傳不能Show錯誤訊息 */
		pobTran->inErrorMsg = _ERROR_CODE_V3_NONE_;
		return (VS_ISO_PACK_ERR);
	}
	
	/* 回復交易碼 */
	pobTran->inISOTxnCode = pobTran->srTRec.inCode;
	
	/* 沒找到 */
	if (ginBeforeIndex == 0)
	{
		/* 清空佔用記憶體 */
		inXML_End(&srDoc);
	
	        return (VS_ERROR);
	}

        /* <801061_Adv> */
	sr801061adv_Node = xmlNewNode(NULL, BAD_CAST "801061_Adv");
	xmlAddChild(srRoot_Node, sr801061adv_Node);
        
	/* <BefData> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%s", "0000");
	inFunc_BCD_to_ASCII(&szTemplate[4], (unsigned char*)&szSendPacket[0], ginBeforeIndex);
        xmlNewChild(sr801061adv_Node, NULL, BAD_CAST "BefData", BAD_CAST szTemplate);
             
        /* <AftData> : 扣掉取代的Len + @@@@@ */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inFunc_BCD_to_ASCII(&szTemplate[0], (unsigned char*)&szSendPacket[ginBeforeIndex + 7], inSendLen - ginBeforeIndex - 7);
        xmlNewChild(sr801061adv_Node, NULL, BAD_CAST "AftData", BAD_CAST szTemplate);

	/* <F03Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr801061adv_Node, NULL, BAD_CAST "F03Adr", BAD_CAST szTemplate);
	
	/* <F04Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr801061adv_Node, NULL, BAD_CAST "F04Adr", BAD_CAST szTemplate);
        
	/* <F11Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr801061adv_Node, NULL, BAD_CAST "F11Adr", BAD_CAST szTemplate);

        /* <F12Adr> */
        memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr801061adv_Node, NULL, BAD_CAST "F12Adr", BAD_CAST szTemplate);
        
        /* <F13Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr801061adv_Node, NULL, BAD_CAST "F13Adr", BAD_CAST szTemplate);
        
	/* <F35Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
		    _ECC_F22_LENTH_ + _ECC_F24_LENTH_ + _ECC_F25_LENTH_ + inField32 + 1;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr801061adv_Node, NULL, BAD_CAST "F35Adr", BAD_CAST szTemplate);
        
        /* <F37Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
		    _ECC_F22_LENTH_ + _ECC_F24_LENTH_ + _ECC_F25_LENTH_ + inField32 + _ECC_F35_LENTH_ + 8;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr801061adv_Node, NULL, BAD_CAST "F37Adr", BAD_CAST szTemplate);

	/* <F57Adr> */
	if (memcmp(szIFESMode, "Y", 1) == 0)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = ginECC_F57_Len;
		sprintf(szTemplate, "-%d", inDataLen);
		xmlNewChild(sr801061adv_Node, NULL, BAD_CAST "F57Adr", BAD_CAST szTemplate);
	}
	
	 /* <F59Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = ginECC_F59_ET_Len;
	sprintf(szTemplate, "-%d", inDataLen);
        xmlNewChild(sr801061adv_Node, NULL, BAD_CAST "F59Adr", BAD_CAST szTemplate);

//	memset(szFESMode, 0x00, sizeof(szFESMode));
//	inGetNCCCFESMode(szFESMode);
//	memset(szTemplate, 0x00, sizeof(szTemplate));
//	inGetCloud_MFES(szTemplate);
//	if (memcmp(szFESMode, _NCCC_05_ATS_MODE_, 2) == 0	||
//	    memcmp(szFESMode, _NCCC_04_MPAS_MODE_, 2) == 0	||
//	   (memcmp(szFESMode, _NCCC_03_MFES_MODE_, 2) == 0	&&
//	    memcmp(szTemplate, "Y", strlen("Y")) == 0))
//	{
		/* ATS規格 - 回覆跟Host長度有關，目前一樣 */
		/* <F39Adr> */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F24_LENTH_ + inField32 + 
			    _ECC_F37_LENTH_ + _ECC_F38_LENTH_;
		sprintf(szTemplate, "%d", inDataLen);
		xmlNewChild(sr801061adv_Node, NULL, BAD_CAST "F39Adr", BAD_CAST szTemplate);

		/* <ECCAdr> */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_  + _ECC_F11_LENTH_ + _ECC_F24_LENTH_ + inField32 + 
			    _ECC_F37_LENTH_ + _ECC_F38_LENTH_ + _ECC_F39_LENTH_ + _ECC_F41_LENTH_ + _ECC_F56_LENTH_;
		sprintf(szTemplate, "%d", inDataLen);
		xmlNewChild(sr801061adv_Node, NULL, BAD_CAST "ECCAdr", BAD_CAST szTemplate);
//	}
//	else
//	{
//		/* ATS規格 - 回覆跟Host長度有關，目前一樣 */
//		/* <F39Adr> */
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F24_LENTH_;
//		sprintf(szTemplate, "%d", inDataLen);
//		xmlNewChild(sr801061adv_Node, NULL, BAD_CAST "F39Adr", BAD_CAST szTemplate);
//
//		/* <ECCAdr> */
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_  + _ECC_F11_LENTH_ + _ECC_F24_LENTH_ + 
//			    _ECC_F39_LENTH_ + _ECC_F41_LENTH_ + _ECC_F56_LENTH_;
//		sprintf(szTemplate, "%d", inDataLen);
//		xmlNewChild(sr801061adv_Node, NULL, BAD_CAST "ECCAdr", BAD_CAST szTemplate);
//	}

        /* <801061_Rev>==============================================================================手動加值0400 */
        memset(szSendPacket, 0x00, sizeof(szSendPacket));
        inTxnCode = _REVERSAL_;
        pobTran->inISOTxnCode = _REVERSAL_;
  
        /* 組 ISO 電文 */
	ginBeforeIndex = 0;
	if ((inSendLen = inNCCC_TICKET_PackISO(pobTran, (unsigned char *)szSendPacket, inTxnCode)) <= 0)
	{
		/* 清空佔用記憶體 */
		inXML_End(&srDoc);
		
		/* 上傳不能Show錯誤訊息 */
		pobTran->inErrorMsg = _ERROR_CODE_V3_NONE_;
		
		return (VS_ISO_PACK_ERR);
	}
	
	/* 回復交易碼 */
	pobTran->inISOTxnCode = pobTran->srTRec.inCode;
	
	/* 沒找到 */
	if (ginBeforeIndex == 0)
	{
		/* 清空佔用記憶體 */
		inXML_End(&srDoc);
		
	        return (VS_ERROR);
	}
	
        /* <801061_Rev> */
	sr801061rev_Node = xmlNewNode(NULL, BAD_CAST "801061_Rev");
	xmlAddChild(srRoot_Node, sr801061rev_Node);

	/* <BefData> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%s", "0000");
	inFunc_BCD_to_ASCII(&szTemplate[4], (unsigned char*)&szSendPacket[0], ginBeforeIndex);
        xmlNewChild(sr801061rev_Node, NULL, BAD_CAST "BefData", BAD_CAST szTemplate);
             
        /* <AftData> : 扣掉取代的Len + @@@@@ */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inFunc_BCD_to_ASCII(&szTemplate[0], (unsigned char*)&szSendPacket[ginBeforeIndex + 7], inSendLen - ginBeforeIndex - 7);
        xmlNewChild(sr801061rev_Node, NULL, BAD_CAST "AftData", BAD_CAST szTemplate);

	/* <F03Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr801061rev_Node, NULL, BAD_CAST "F03Adr", BAD_CAST szTemplate);
	
	/* <F04Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr801061rev_Node, NULL, BAD_CAST "F04Adr", BAD_CAST szTemplate);
        
	/* <F11Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr801061rev_Node, NULL, BAD_CAST "F11Adr", BAD_CAST szTemplate);
	
	/* <F35Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F22_LENTH_ + _ECC_F24_LENTH_ +
                    _ECC_F25_LENTH_ + inField32 + 1;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr801061rev_Node, NULL, BAD_CAST "F35Adr", BAD_CAST szTemplate);
        
	/* <F37Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F22_LENTH_ + _ECC_F24_LENTH_ +
                    _ECC_F25_LENTH_ + inField32 + _ECC_F35_LENTH_ + 8;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr801061rev_Node, NULL, BAD_CAST "F37Adr", BAD_CAST szTemplate);
        
	/* <F57Adr> */
	if (memcmp(szIFESMode, "Y", 1) == 0)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = ginECC_F57_Len;
		sprintf(szTemplate, "-%d", inDataLen);
		xmlNewChild(sr801061rev_Node, NULL, BAD_CAST "F57Adr", BAD_CAST szTemplate);
	}
	
        /* <F59Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = ginECC_F59_ET_Len;
	sprintf(szTemplate, "-%d", inDataLen);
        xmlNewChild(sr801061rev_Node, NULL, BAD_CAST "F59Adr", BAD_CAST szTemplate);

//	memset(szFESMode, 0x00, sizeof(szFESMode));
//	inGetNCCCFESMode(szFESMode);
//	memset(szTemplate, 0x00, sizeof(szTemplate));
//	inGetCloud_MFES(szTemplate);
//	if (memcmp(szFESMode, _NCCC_05_ATS_MODE_, 2) == 0	||
//	    memcmp(szFESMode, _NCCC_04_MPAS_MODE_, 2) == 0	||
//	   (memcmp(szFESMode, _NCCC_03_MFES_MODE_, 2) == 0	&&
//	    memcmp(szTemplate, "Y", strlen("Y")) == 0))
//	{
		/* ATS規格 - 回覆跟Host長度有關，目前一樣 */
		/* <F39Adr> */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
			    _ECC_F24_LENTH_ + inField32 + _ECC_F37_LENTH_ + _ECC_F38_LENTH_;
		sprintf(szTemplate, "%d", inDataLen);
		xmlNewChild(sr801061rev_Node, NULL, BAD_CAST "F39Adr", BAD_CAST szTemplate);

		/* <ECCAdr> */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_  + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
			    _ECC_F24_LENTH_ + inField32 + _ECC_F37_LENTH_ + _ECC_F39_LENTH_ + _ECC_F41_LENTH_ + _ECC_F56_LENTH_;
		sprintf(szTemplate, "%d", inDataLen);
		xmlNewChild(sr801061rev_Node, NULL, BAD_CAST "ECCAdr", BAD_CAST szTemplate);
//	}
//	else
//	{
//		/* MFES規格 */
//		/* <F39Adr> */
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
//			    _ECC_F24_LENTH_;
//		sprintf(szTemplate, "%d", inDataLen);
//		xmlNewChild(sr801061rev_Node, NULL, BAD_CAST "F39Adr", BAD_CAST szTemplate);
//
//		/* <ECCAdr> */
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_  + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
//			    _ECC_F24_LENTH_ + _ECC_F39_LENTH_ + _ECC_F41_LENTH_ + _ECC_F56_LENTH_;
//		sprintf(szTemplate, "%d", inDataLen);
//		xmlNewChild(sr801061rev_Node, NULL, BAD_CAST "ECCAdr", BAD_CAST szTemplate);
//	}

        /* 儲存xml檔 xml檔現在會自動換行了 */
        inXML_SaveFile(_ECC_API_REQ_FILE2_, &srDoc, "utf-8", 1);
	inFile_Move_File(_ECC_API_REQ_FILE2_, _AP_ROOT_PATH_, "", _ECC_FOLDER_PATH_);
	
	/* 清空佔用記憶體 */
	inXML_End(&srDoc);
	
	if (inECC_Insert_Temp() != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inECC_PACK_TOP_UP_ICERAPI_NCCC() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inECC_Void_TOP_UP_Flow
Date&Time       :2018/4/10 上午 9:55
Describe        :
*/
int inECC_Void_TOP_UP_Flow(TRANSACTION_OBJECT *pobTran)
{
        int		inRetVal = VS_ERROR;
	int		inRetry = 1;
	char		szKey = 0x00;
	char		szCustomerIndicator[3 + 1] = {0};
	char		szDebugMsg[100 + 1] = {0};
	unsigned char	uszECRAlreadyBit = VS_FALSE;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inECC_Void_TOP_UP_Flow() START !");
	}
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	guszECCRetryBit = VS_FALSE;
	
	while (inRetry <= 5)
	{
		/* (需求單 - 107227)邦柏科技自助作業客製化 by Russell 2018/12/13 下午 8:33 */
		/* (需求單 - 107276)自助交易標準400做法 by Russell 2018/12/27 上午 11:20 */
		if ((!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)           ||
		     !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_)   ||
                     !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)        ||
		     !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)	||
		     !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)		||
		     !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))	&&
		     pobTran->uszLastTranscationBit == VS_TRUE)
		{
			uszECRAlreadyBit = VS_TRUE;
			inECC_PACK_RETRY_ICERAPI(pobTran);
		}
		else
		{
			/* 組電文(生成ICERAPI.REQ) */
			inECC_PACK_Void_TOP_UP_ICERAPI(pobTran);
		}
	        
	        /* AdditionalTcpipData */
	        inECC_PACK_Void_TOP_UP_ICERAPI_NCCC(pobTran);

                if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "ICER Start");
			inLogPrintf(AT, szDebugMsg);
		}
		
		inRetVal = ICERApi_exe();
		
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "ICER End");
			inLogPrintf(AT, szDebugMsg);
		}
                
		/* 解電文(分析ICERAPI.RES) */
		inRetVal = inECC_UNPACK_ICERAPI(pobTran, _ECC_API_RES_FILE_);	
				
		if (inRetVal != VS_SUCCESS)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inCTLS_Set_LED(_CTLS_LIGHT_RED_);
                        inDISP_ChineseFont("悠遊卡檔案異常", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
                        inDISP_BEEP(3, 1000);
			
                        return (VS_ERROR);
                }
                
		/* 檢核回應碼 <T3901>開始 */
	        if (memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "0", 1) != 0)
	        {
			if (!memcmp(pobTran->srTRec.szRespCode, "A0", 2)	||
			    !memcmp(pobTran->srTRec.szRespCode, "A1", 2))
			{
				ginMacError++;
				inECC_UNPACK_ICERAPI_NCCC(pobTran, _ECC_API_RES_FILE2_, _ECC_FOLDER_PATH_);
			}
			
			/* (需求單 - 107227)邦柏科技自助作業客製化 by Russell 2018/12/9 下午 2:42 */
			if ((!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)           ||
			     !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_)	||
			     !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)		||
			     !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))	&&
			     pobTran->uszECRBit == VS_TRUE)
			{
				/* 第二次發生RETRY 回傳失敗 */
	        		if (uszECRAlreadyBit == VS_TRUE)
				{
	        			return (VS_ERROR);
				}
				
				if (memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-125", 4) == 0)
				{
	                                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_ChineseFont("請重新交易", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
					inDISP_BEEP(3, 1000);

	                                /* 要回錯誤碼0018不是0001 所以Reset */
	                                memset(pobTran->szTicket_ErrorCode, 0x00, sizeof(pobTran->szTicket_ErrorCode));
					pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_ESVC_RE_TAP_CARD_;

					/* 將現在組好的REQ 備份成ICERRETRY.REQ */
					inFunc_Data_Copy(_ECC_API_REQ_FILE_, _ECC_FOLDER_PATH_, _ECC_API_REQ_RETRY_FILE_, _ECC_FOLDER_PATH_);

//					/* 將pobTran的MFRec備份部分資料 */
//					inECC_SAVE_TREC(pobTran);
					
	                                return (VS_ERROR);
	                        }
				else
				{	
					/* 其他錯誤Display */
					inECC_ResponeCode(pobTran);

					return (VS_ERROR);
				}
			}
                        else if ((!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)   ||
			          !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_))	&&
			         pobTran->uszECRBit == VS_TRUE)
			{
				/* 第二次發生RETRY 回傳失敗 */
	        		if (uszECRAlreadyBit == VS_TRUE)
				{
	        			return (VS_ERROR);
				}
				
				if (memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-125", 4) == 0)
				{
	                                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_ChineseFont("請重新交易", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
					inDISP_BEEP(3, 1000);

	                                /* 要回錯誤碼0018不是0001 所以Reset */
	                                memset(pobTran->szTicket_ErrorCode, 0x00, sizeof(pobTran->szTicket_ErrorCode));
					pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_ESVC_RE_TAP_CARD_;

					/* 將現在組好的REQ 備份成ICERRETRY.REQ */
					inFunc_Data_Copy(_ECC_API_REQ_FILE_, _ECC_FOLDER_PATH_, _ECC_API_REQ_RETRY_FILE_, _ECC_FOLDER_PATH_);

//					/* 將pobTran的MFRec備份部分資料 */
//					inECC_SAVE_TREC(pobTran);
					
                                        pobTran->uszECCRetryBit = VS_TRUE;
                                        
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "ECC Retry....");
                                        }
                                        
	                                return (VS_ERROR);
	                        }
				else
				{	
					/* 其他錯誤Display */
					inECC_ResponeCode(pobTran);

					return (VS_ERROR);
				}
			}
			else
			{
				/* -125 = Retry */
				/* -119 : Call相關DLL回應錯誤 ; 請參考T3904 */
				/* 62 : RC531錯誤，交易中止，請重新交易*/
				if ((memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-125", 4) == 0)	||
				    (memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "-119", 4) == 0	&&
				     memcmp(&pobTran->srTRec.srECCRec.szReader_RespCode[0], "62", 2) == 0	&&
				     guszECCRetryBit == VS_TRUE))
				{
					if (inRetry >= 4)
					{
						inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
						inDISP_ChineseFont("請重新交易", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
						inDISP_BEEP(3, 1000);
						return (VS_ERROR);
					}

					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_ChineseFont("連線失敗", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
					inDISP_ChineseFont("請重新感應卡片", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
					inDISP_ChineseFont("並按確認鍵重試", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_CENTER_);

					szKey = 0x00;
					inDISP_Timer_Start(_TIMER_NEXSYS_1_, 60);

					while (1)
					{
						szKey = uszKBD_Key();
						if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
						{
							szKey = _KEY_TIMEOUT_;
						}

						if (szKey == _KEY_ENTER_)
						{
							inRetry++;
							guszECCRetryBit = VS_TRUE;
							break;
						}
						else if (szKey == _KEY_TIMEOUT_)
						{
							return (VS_ERROR);
						}
					}

					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_ChineseFont("交易處理中", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
					inDISP_ChineseFont("請重新感應卡片", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_CENTER_);

					continue;
				}
				else
				{
					/* 其他錯誤Display */
					inECC_ResponeCode(pobTran);

					return (VS_ERROR);
				}
			}/* 客製化 */
                }
		else
		{
			/* 每次unpack(交易，詢卡、查額、SignOn不算)就同步電文序號
			 * 成功交易才更新電文序號 */
			inNCCC_Ticket_Func_UpdatetTicketInvNum(pobTran);
		
			/* 未滿三次，授權成功要重置次數 */
			if (ginMacError < 3)
			{
				ginMacError = 0;
			}
		}
		
		/* 抓必要資料 */
		if (inECC_UNPACK_ICERAPI_NCCC(pobTran, _ECC_API_RES_FILE2_, _ECC_FOLDER_PATH_) != VS_SUCCESS)
		        return (VS_ERROR);
                
                break;
        }
	
        /*
	 * T5501批次號碼之規則為yymmddss+流水號兩碼
	 * 以當批第一筆成功交易時間為準
	 * 如果NeedNewBatch是'Y'，代表是第一筆，更新狀態為不用更新批號
	 * 如果NeedNewBatch是'N'，代表有做過交易
	*/
	inECC_Batch_Update();
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inECC_Void_TOP_UP_Flow() END !");
		inLogPrintf(AT, "----------------------------------------");
	}

        return (VS_SUCCESS);
}

/*
Function        :inECC_PACK_Void_TOP_UP_ICERAPI
Date&Time       :2018/4/10 上午 10:04
Describe        :
*/
int inECC_PACK_Void_TOP_UP_ICERAPI(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_ERROR;
	int		inESVCIndex = 0;
	char		szTemplate[20 + 1] = {0};
	char		szSTAN[6 + 1] = {0};
	char		szInvoiceNum[6 + 1] = {0};
	RTC_NEXSYS	srRTC = {0};
	xmlDocPtr       srDoc = NULL;
        xmlNodePtr      srRoot_Node = NULL, srTRANS_Node = NULL;
#if 	READER_MANUFACTURERS == LINUX_API
	unsigned long	ulHandle = 0;
#else
	int		inFd = 0;
#endif

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inECC_PACK_Void_TOP_UP_ICERAPI() START !");
	}
	
	if (inLoadTDTRec(_TDT_INDEX_01_ECC_) < 0)
                return (VS_ERROR);
	
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTicket_HostIndex(szTemplate);
	inESVCIndex = atoi(szTemplate);
        
        if (inLoadHDTRec(inESVCIndex) != VS_SUCCESS)
                return (VS_ERROR);
	if (inLoadHDPTRec(inESVCIndex) != VS_SUCCESS)
                return (VS_ERROR);
                
        if (inLoadECCDTRec(0) != VS_SUCCESS)
	{
                return (VS_ERROR);
	}
        
        /* 刪除原本的ICERAPI */
        inFILE_Delete((unsigned char*)_ECC_API_REQ_CHECK_);	/* 檢核用 */
	inFILE_Delete((unsigned char*)_ECC_API_REQ_FILE_);
	inFILE_Delete((unsigned char*)_ECC_API_REQ_FILE2_);
	inFILE_Delete((unsigned char*)_ECC_API_RES_FILE_);	/* 防止用到上次資料結果 */
	inFILE_Delete((unsigned char*)_ECC_API_RES_FILE2_);
	
	/* 建立一個新的檔案 */
        srDoc = xmlNewDoc(BAD_CAST "1.0");
        srRoot_Node = xmlNewNode(NULL, BAD_CAST "TransXML");
        xmlDocSetRootElement (srDoc, srRoot_Node);
	
        /* <TRANS> */
	srTRANS_Node = xmlNewNode(NULL, BAD_CAST "TRANS");
	xmlAddChild(srRoot_Node, srTRANS_Node);

        /* <T0100> */
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T0100", BAD_CAST "0200");

        /* <T0300> */
        xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T0300", BAD_CAST "816100");
        
	/* <T0400> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%lu00", pobTran->srTRec.lnTxnAmount);
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T0400", BAD_CAST szTemplate);

	if (guszECCRetryBit == VS_TRUE)
	{
		/* Retry 要同第一次，除了時間 */
                /* <T1100> : STAN 不知道用在哪，先比照NCCC STAN */
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1100", BAD_CAST gszECCRetryData.szT1100);

		/* <T1101> : CMAS自用，區分上筆交易(會回傳) */
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1101", BAD_CAST gszECCRetryData.szT1101);

		/* <T1102> : NCCC STAN */
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1102", BAD_CAST gszECCRetryData.szT1102);

		/* <T1103> : NCCC RRN(後三碼) */
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1103", BAD_CAST gszECCRetryData.szT1103);
                                            
                /* <T1200> : hhmmss */
		memset(&srRTC, 0x00, sizeof(srRTC));
		inFunc_GetSystemDateAndTime(&srRTC);
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%02d%02d%02d", srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1200", BAD_CAST szTemplate);

		/* <T1300> : yyyymmdd */
		/* hhmmss就抓過一次時間了，不用重抓 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "20%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1300", BAD_CAST szTemplate);
	}
	else
	{
		/* For Retry 重試時使用 */
		memset(&gszECCRetryData, 0x00, sizeof(gszECCRetryData));
		
		/* <T1100> : STAN 不知道用在哪，先比照NCCC STAN */
		memset(szSTAN, 0x00, sizeof(szSTAN));
		inGetSTANNum(szSTAN);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1100", BAD_CAST szSTAN);
		
		/* For Retry 重試時使用 */
		memcpy(gszECCRetryData.szT1100, szSTAN, strlen(szSTAN));

		/* <T1101> : CMAS自用，區分上筆交易(會回傳) */
		memset(szSTAN, 0x00, sizeof(szSTAN));
		inGetTicket_STAN(szSTAN);
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%06d", atoi(szSTAN) + 1);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1101", BAD_CAST szTemplate);
		inSetTicket_STAN(szTemplate);
		inSaveTDTRec(_TDT_INDEX_01_ECC_);

		/* For Retry 重試時使用 */
		memcpy(gszECCRetryData.szT1101, szTemplate, strlen(szTemplate));
		
		/* <T1102> : NCCC STAN */
		memset(szSTAN, 0x00, sizeof(szSTAN));
		inGetSTANNum(szSTAN);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1102", BAD_CAST szSTAN);
		
		/* For Retry 重試時使用 */
		memcpy(gszECCRetryData.szT1102, szSTAN, strlen(szSTAN));

		/* <T1103> : NCCC RRN(後三碼) */
		memset(szInvoiceNum, 0x00, sizeof(szInvoiceNum));
		inGetTicket_InvNum(szInvoiceNum);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1103", BAD_CAST szInvoiceNum);
		
		/* For Retry 重試時使用 */
		memcpy(gszECCRetryData.szT1103, szInvoiceNum, strlen(szInvoiceNum));

		/* <T1200> : hhmmss */
		memset(&srRTC, 0x00, sizeof(srRTC));
		inFunc_GetSystemDateAndTime(&srRTC);
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%02d%02d%02d", srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1200", BAD_CAST szTemplate);

		/* <T1300> : yyyymmdd */
		/* hhmmss就抓過一次時間了，不用重抓 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "20%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
		xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1300", BAD_CAST szTemplate);
	}

        /* <T4108> : LCD Control Flag 單純顯示畫面用 */
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T4108", BAD_CAST "0");

	/* <T4830> : Read Purse Flag 
	             0：不需同一票卡(default)
		     1：需為同一票卡
	*/
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T4830", BAD_CAST "0");
        
        /* <T5501> : Batch Number - 悠遊卡特有，同一天流水號累加，隔天就要歸1 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inECC_Batch_Check(szTemplate);
        xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T5501", BAD_CAST szTemplate);
        
	/* <T5581> : 取消交易要帶<T4100> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTicket_Device3(szTemplate);
        xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T5581", BAD_CAST szTemplate);
        
        /* <T5582> : 最後的RRN */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTicket_LastRRN(szTemplate);
        xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T5582", BAD_CAST szTemplate);
        
        /* <T5583> : 最後的DATE */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTicket_LastTransDate(szTemplate);
        xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T5583", BAD_CAST szTemplate);
	
	/* 儲存xml檔 xml檔現在會自動換行了 */
        inXML_SaveFile(_ECC_API_REQ_FILE_, &srDoc, "utf-8", 1);
	inFile_Move_File(_ECC_API_REQ_FILE_, _AP_ROOT_PATH_, "", _ECC_FOLDER_PATH_);
	
	/* 清空佔用記憶體 */
	inXML_End(&srDoc);
	
	/* 步驟完成，建立檢核擋 */
#if 	READER_MANUFACTURERS == LINUX_API
	inRetVal = inFILE_Create(&ulHandle, (unsigned char*)_ECC_API_REQ_CHECK_);
	/* 不管成功或失敗都要關檔 */
	inFILE_Close(&ulHandle);
	/* 抓到ICERData資料夾 */
	inFile_Move_File(_ECC_API_REQ_CHECK_, _FS_DATA_PATH_, "", _ECC_FOLDER_PATH_);
#else
	inRetVal = inFile_Linux_Create(&inFd, _ECC_API_REQ_CHECK_);
	/* 不管成功或失敗都要關檔 */
	inFile_Linux_Close(inFd);
	/* 會有權限問題，要先改權限 */
	inFunc_Data_Chmod("u+rwx", _ECC_API_REQ_CHECK_, _AP_ROOT_PATH_);
	/* 抓到ICERData資料夾 */
	inFile_Move_File(_ECC_API_REQ_CHECK_, _AP_ROOT_PATH_, "", _ECC_FOLDER_PATH_);
#endif
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inECC_PACK_Void_TOP_UP_ICERAPI() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inECC_PACK_Void_TOP_UP_ICERAPI_NCCC
Date&Time       :2018/4/10 上午 10:41
Describe        :
*/
int inECC_PACK_Void_TOP_UP_ICERAPI_NCCC(TRANSACTION_OBJECT *pobTran)
{
	int		inDataLen = 0;
	int		inTxnCode = 0;
	int		inSendLen = 0;
	int		inField32 = 0;
	int		inField48 = 0;
	char		szFESMode[2 + 1] = {0};
	char		szIFESMode[2 + 1] = {0};
	char		szSendPacket[_NCCC_TICKET_ISO_SEND_ + 1] = {0};
	char		szTemplate[500 + 1] = {0};
	xmlDocPtr       srDoc = NULL;
        xmlNodePtr      srRoot_Node = NULL, sr816100_Node = NULL, sr816100adv_Node = NULL, sr816100rev_Node = NULL,
		        sr816101_Node = NULL, sr816101adv_Node = NULL, sr816101rev_Node = NULL;

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inECC_PACK_Void_TOP_UP_ICERAPI_NCCC() START !");
	}
	
	memset(szFESMode, 0x00, sizeof(szFESMode));
	inGetNCCCFESMode(szFESMode);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetCloud_MFES(szTemplate);
	memset(szIFESMode, 0x00, sizeof(szIFESMode));
	inGetI_FES_Mode(szIFESMode);
        if (memcmp(szFESMode, _NCCC_05_ATS_MODE_, 2) == 0	||
	    memcmp(szFESMode, _NCCC_04_MPAS_MODE_, 2) == 0	||
	   (memcmp(szFESMode, _NCCC_03_MFES_MODE_, 2) == 0	&&
	    memcmp(szTemplate, "Y", strlen("Y")) == 0))
	{
                inField32 = _ECC_F32_LENTH_;
	}
                
        if (strlen(pobTran->srTRec.szStoreID) > 0)
	{
                inField48 = _ECC_F48_LENTH_;
	}
                
        pobTran->srTRec.inTicketType = _TICKET_TYPE_ECC_;
       
        memset(szSendPacket, 0x00, sizeof(szSendPacket));
        inTxnCode = _TICKET_EASYCARD_VOID_TOP_UP_;
        pobTran->inISOTxnCode = inTxnCode;
        
	/* 要Load CPT */
	inETHERNET_Begin(pobTran);
	
        /* 組 ISO 電文 */
	ginBeforeIndex = 0;
	if ((inSendLen = inNCCC_TICKET_PackISO(pobTran, (unsigned char *)szSendPacket, inTxnCode)) <= 0)
	{
		/* 上傳不能Show錯誤訊息 */
		pobTran->inErrorMsg = _ERROR_CODE_V3_NONE_;
		return (VS_ISO_PACK_ERR);
	}
	
	/* 回復交易碼 */
	pobTran->inISOTxnCode = pobTran->srTRec.inCode;
	
	/* 沒找到 */
	if (ginBeforeIndex == 0)
	{
	        return (VS_ERROR);
	}

        /* 刪除原本的ICERAPI */
	inFile_Unlink_File(_ECC_API_REQ_FILE2_, _ECC_FOLDER_PATH_);
	
	/* 建立一個新的檔案 */
        srDoc = xmlNewDoc(BAD_CAST "1.0");
        srRoot_Node = xmlNewNode(NULL, BAD_CAST "TransXML");
        xmlDocSetRootElement (srDoc, srRoot_Node);

        /* <816100> =================================================================================加值取消0100 */
	sr816100_Node = xmlNewNode(NULL, BAD_CAST "816100");
	xmlAddChild(srRoot_Node, sr816100_Node);
        
	/* <BefData> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%s", "0000");
	inFunc_BCD_to_ASCII(&szTemplate[4], (unsigned char*)&szSendPacket[0], ginBeforeIndex);
        xmlNewChild(sr816100_Node, NULL, BAD_CAST "BefData", BAD_CAST szTemplate);
             
        /* <AftData> : 扣掉取代的Len + @@@@@ */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inFunc_BCD_to_ASCII(&szTemplate[0], (unsigned char*)&szSendPacket[ginBeforeIndex + 7], inSendLen - ginBeforeIndex - 7);
        xmlNewChild(sr816100_Node, NULL, BAD_CAST "AftData", BAD_CAST szTemplate);

	/* <F03Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr816100_Node, NULL, BAD_CAST "F03Adr", BAD_CAST szTemplate);
	
	/* <F04Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr816100_Node, NULL, BAD_CAST "F04Adr", BAD_CAST szTemplate);
        
	/* <F11Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr816100_Node, NULL, BAD_CAST "F11Adr", BAD_CAST szTemplate);
	
	/* <F35Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F22_LENTH_ + _ECC_F24_LENTH_ +
                    _ECC_F25_LENTH_ + inField32 + 1;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr816100_Node, NULL, BAD_CAST "F35Adr", BAD_CAST szTemplate);
        
        /* <F37Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F22_LENTH_ + _ECC_F24_LENTH_ +
                    _ECC_F25_LENTH_ + inField32 + _ECC_F35_LENTH_ + 8;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr816100_Node, NULL, BAD_CAST "F37Adr", BAD_CAST szTemplate);
        
	/* <F57Adr> */
	if (memcmp(szIFESMode, "Y", 1) == 0)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = ginECC_F57_Len;
		sprintf(szTemplate, "-%d", inDataLen);
		xmlNewChild(sr816100_Node, NULL, BAD_CAST "F57Adr", BAD_CAST szTemplate);
	}
	
        /* <F59Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = ginECC_F59_ET_Len;
	sprintf(szTemplate, "-%d", inDataLen);
        xmlNewChild(sr816100_Node, NULL, BAD_CAST "F59Adr", BAD_CAST szTemplate);

//	memset(szFESMode, 0x00, sizeof(szFESMode));
//	inGetNCCCFESMode(szFESMode);
//	memset(szTemplate, 0x00, sizeof(szTemplate));
//	inGetCloud_MFES(szTemplate);
//	if (memcmp(szFESMode, _NCCC_05_ATS_MODE_, 2) == 0	||
//	    memcmp(szFESMode, _NCCC_04_MPAS_MODE_, 2) == 0	||
//	   (memcmp(szFESMode, _NCCC_03_MFES_MODE_, 2) == 0	&&
//	    memcmp(szTemplate, "Y", strlen("Y")) == 0))
//	{
		/* ATS規格 - 回覆跟Host長度有關，目前一樣 */
		/* <F39Adr> */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
			    _ECC_F24_LENTH_ + inField32 + _ECC_F37_LENTH_ + _ECC_F38_LENTH_;
		sprintf(szTemplate, "%d", inDataLen);
		xmlNewChild(sr816100_Node, NULL, BAD_CAST "F39Adr", BAD_CAST szTemplate);

		/* <ECCAdr> */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_  + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
			    _ECC_F24_LENTH_ + inField32 + _ECC_F37_LENTH_ + _ECC_F38_LENTH_ + _ECC_F39_LENTH_ + _ECC_F41_LENTH_ + _ECC_F56_LENTH_;
		sprintf(szTemplate, "%d", inDataLen);
		xmlNewChild(sr816100_Node, NULL, BAD_CAST "ECCAdr", BAD_CAST szTemplate);
//	}
//	else
//	{
//		/* ATS規格 - 回覆跟Host長度有關，目前一樣 */
//		/* <F39Adr> */
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
//			    _ECC_F24_LENTH_;
//		sprintf(szTemplate, "%d", inDataLen);
//		xmlNewChild(sr816100_Node, NULL, BAD_CAST "F39Adr", BAD_CAST szTemplate);
//
//		/* <ECCAdr> */
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_  + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
//			    _ECC_F24_LENTH_ + _ECC_F39_LENTH_ + _ECC_F41_LENTH_ + _ECC_F56_LENTH_;
//		sprintf(szTemplate, "%d", inDataLen);
//		xmlNewChild(sr816100_Node, NULL, BAD_CAST "ECCAdr", BAD_CAST szTemplate);
//	}
	
        /* <816100_Adv>==============================================================================手動加值0220 */
        memset(szSendPacket, 0x00, sizeof(szSendPacket));
        inTxnCode = _ADVICE_;
        pobTran->inISOTxnCode = _ADVICE_;
            
        /* 組 ISO 電文 */
	ginBeforeIndex = 0;
	if ((inSendLen = inNCCC_TICKET_PackISO(pobTran, (unsigned char *)szSendPacket, inTxnCode)) <= 0)
	{
		/* 清空佔用記憶體 */
		inXML_End(&srDoc);
	
		/* 上傳不能Show錯誤訊息 */
		pobTran->inErrorMsg = _ERROR_CODE_V3_NONE_;
		return (VS_ISO_PACK_ERR);
	}
	
	/* 回復交易碼 */
	pobTran->inISOTxnCode = pobTran->srTRec.inCode;
	
	/* 沒找到 */
	if (ginBeforeIndex == 0)
	{
		/* 清空佔用記憶體 */
		inXML_End(&srDoc);
	
	        return (VS_ERROR);
	}

        /* <816100_Adv> */
	sr816100adv_Node = xmlNewNode(NULL, BAD_CAST "816100_Adv");
	xmlAddChild(srRoot_Node, sr816100adv_Node);
        
	/* <BefData> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%s", "0000");
	inFunc_BCD_to_ASCII(&szTemplate[4], (unsigned char*)&szSendPacket[0], ginBeforeIndex);
        xmlNewChild(sr816100adv_Node, NULL, BAD_CAST "BefData", BAD_CAST szTemplate);
             
        /* <AftData> : 扣掉取代的Len + @@@@@ */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inFunc_BCD_to_ASCII(&szTemplate[0], (unsigned char*)&szSendPacket[ginBeforeIndex + 7], inSendLen - ginBeforeIndex - 7);
        xmlNewChild(sr816100adv_Node, NULL, BAD_CAST "AftData", BAD_CAST szTemplate);

	/* <F03Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr816100adv_Node, NULL, BAD_CAST "F03Adr", BAD_CAST szTemplate);
	
	/* <F04Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr816100adv_Node, NULL, BAD_CAST "F04Adr", BAD_CAST szTemplate);
        
	/* <F11Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr816100adv_Node, NULL, BAD_CAST "F11Adr", BAD_CAST szTemplate);

        /* <F12Adr> */
        memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr816100adv_Node, NULL, BAD_CAST "F12Adr", BAD_CAST szTemplate);
        
        /* <F13Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr816100adv_Node, NULL, BAD_CAST "F13Adr", BAD_CAST szTemplate);
        
	/* <F35Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
		    _ECC_F22_LENTH_ + _ECC_F24_LENTH_ + _ECC_F25_LENTH_ + inField32 + 1;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr816100adv_Node, NULL, BAD_CAST "F35Adr", BAD_CAST szTemplate);
        
        /* <F37Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
		    _ECC_F22_LENTH_ + _ECC_F24_LENTH_ + _ECC_F25_LENTH_ + inField32 + _ECC_F35_LENTH_ + 8;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr816100adv_Node, NULL, BAD_CAST "F37Adr", BAD_CAST szTemplate);

	/* <F57Adr> */
	if (memcmp(szIFESMode, "Y", 1) == 0)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = ginECC_F57_Len;
		sprintf(szTemplate, "-%d", inDataLen);
		xmlNewChild(sr816100adv_Node, NULL, BAD_CAST "F57Adr", BAD_CAST szTemplate);
	}
	
	 /* <F59Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = ginECC_F59_ET_Len;
	sprintf(szTemplate, "-%d", inDataLen);
        xmlNewChild(sr816100adv_Node, NULL, BAD_CAST "F59Adr", BAD_CAST szTemplate);

//	memset(szFESMode, 0x00, sizeof(szFESMode));
//	inGetNCCCFESMode(szFESMode);
//	memset(szTemplate, 0x00, sizeof(szTemplate));
//	inGetCloud_MFES(szTemplate);
//	if (memcmp(szFESMode, _NCCC_05_ATS_MODE_, 2) == 0	||
//	    memcmp(szFESMode, _NCCC_04_MPAS_MODE_, 2) == 0	||
//	   (memcmp(szFESMode, _NCCC_03_MFES_MODE_, 2) == 0	&&
//	    memcmp(szTemplate, "Y", strlen("Y")) == 0))
//	{
		/* ATS規格 - 回覆跟Host長度有關，目前一樣 */
		/* <F39Adr> */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F24_LENTH_ + inField32 + 
			    _ECC_F37_LENTH_ + _ECC_F38_LENTH_;
		sprintf(szTemplate, "%d", inDataLen);
		xmlNewChild(sr816100adv_Node, NULL, BAD_CAST "F39Adr", BAD_CAST szTemplate);

		/* <ECCAdr> */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_  + _ECC_F11_LENTH_ + _ECC_F24_LENTH_ + inField32 + 
			    _ECC_F37_LENTH_ + _ECC_F38_LENTH_ + _ECC_F39_LENTH_ + _ECC_F41_LENTH_ + _ECC_F56_LENTH_;
		sprintf(szTemplate, "%d", inDataLen);
		xmlNewChild(sr816100adv_Node, NULL, BAD_CAST "ECCAdr", BAD_CAST szTemplate);
//	}
//	else
//	{
//		/* MFES規格 */
//		/* <F39Adr> */
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F24_LENTH_;
//		sprintf(szTemplate, "%d", inDataLen);
//		xmlNewChild(sr816100adv_Node, NULL, BAD_CAST "F39Adr", BAD_CAST szTemplate);
//
//		/* <ECCAdr> */
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_  + _ECC_F11_LENTH_ + _ECC_F24_LENTH_ +
//			    _ECC_F39_LENTH_ + _ECC_F41_LENTH_ + _ECC_F56_LENTH_;
//		sprintf(szTemplate, "%d", inDataLen);
//		xmlNewChild(sr816100adv_Node, NULL, BAD_CAST "ECCAdr", BAD_CAST szTemplate);
//	}

        /* <816100_Rev>==============================================================================手動加值0400 */
        memset(szSendPacket, 0x00, sizeof(szSendPacket));
        inTxnCode = _REVERSAL_;
        pobTran->inISOTxnCode = _REVERSAL_;
  
        /* 組 ISO 電文 */
	ginBeforeIndex = 0;
	if ((inSendLen = inNCCC_TICKET_PackISO(pobTran, (unsigned char *)szSendPacket, inTxnCode)) <= 0)
	{
		/* 清空佔用記憶體 */
		inXML_End(&srDoc);
		
		/* 上傳不能Show錯誤訊息 */
		pobTran->inErrorMsg = _ERROR_CODE_V3_NONE_;
		
		return (VS_ISO_PACK_ERR);
	}
	
	/* 回復交易碼 */
	pobTran->inISOTxnCode = pobTran->srTRec.inCode;
	
	/* 沒找到 */
	if (ginBeforeIndex == 0)
	{
		/* 清空佔用記憶體 */
		inXML_End(&srDoc);
		
	        return (VS_ERROR);
	}
	
        /* <816100_Rev> */
	sr816100rev_Node = xmlNewNode(NULL, BAD_CAST "816100_Rev");
	xmlAddChild(srRoot_Node, sr816100rev_Node);

	/* <BefData> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%s", "0000");
	inFunc_BCD_to_ASCII(&szTemplate[4], (unsigned char*)&szSendPacket[0], ginBeforeIndex);
        xmlNewChild(sr816100rev_Node, NULL, BAD_CAST "BefData", BAD_CAST szTemplate);
             
        /* <AftData> : 扣掉取代的Len + @@@@@ */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inFunc_BCD_to_ASCII(&szTemplate[0], (unsigned char*)&szSendPacket[ginBeforeIndex + 7], inSendLen - ginBeforeIndex - 7);
        xmlNewChild(sr816100rev_Node, NULL, BAD_CAST "AftData", BAD_CAST szTemplate);

	/* <F03Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr816100rev_Node, NULL, BAD_CAST "F03Adr", BAD_CAST szTemplate);
	
	/* <F04Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr816100rev_Node, NULL, BAD_CAST "F04Adr", BAD_CAST szTemplate);
        
	/* <F11Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr816100rev_Node, NULL, BAD_CAST "F11Adr", BAD_CAST szTemplate);
	
	/* <F35Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F22_LENTH_ + _ECC_F24_LENTH_ +
                    _ECC_F25_LENTH_ + inField32 + 1;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr816100rev_Node, NULL, BAD_CAST "F35Adr", BAD_CAST szTemplate);
        
	/* <F37Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F22_LENTH_ + _ECC_F24_LENTH_ +
                    _ECC_F25_LENTH_ + inField32 + _ECC_F35_LENTH_ + 8;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr816100rev_Node, NULL, BAD_CAST "F37Adr", BAD_CAST szTemplate);
        
	/* <F57Adr> */
	if (memcmp(szIFESMode, "Y", 1) == 0)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = ginECC_F57_Len;
		sprintf(szTemplate, "-%d", inDataLen);
		xmlNewChild(sr816100rev_Node, NULL, BAD_CAST "F57Adr", BAD_CAST szTemplate);
	}
	
        /* <F59Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = ginECC_F59_ET_Len;
	sprintf(szTemplate, "-%d", inDataLen);
        xmlNewChild(sr816100rev_Node, NULL, BAD_CAST "F59Adr", BAD_CAST szTemplate);

//	memset(szFESMode, 0x00, sizeof(szFESMode));
//	inGetNCCCFESMode(szFESMode);
//	memset(szTemplate, 0x00, sizeof(szTemplate));
//	inGetCloud_MFES(szTemplate);
//	if (memcmp(szFESMode, _NCCC_05_ATS_MODE_, 2) == 0	||
//	    memcmp(szFESMode, _NCCC_04_MPAS_MODE_, 2) == 0	||
//	   (memcmp(szFESMode, _NCCC_03_MFES_MODE_, 2) == 0	&&
//	    memcmp(szTemplate, "Y", strlen("Y")) == 0))
//	{
		/* ATS規格 - 回覆跟Host長度有關，目前一樣 */
	
		/* <F39Adr> */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
			    _ECC_F24_LENTH_ + inField32 + _ECC_F37_LENTH_ + _ECC_F38_LENTH_;
		sprintf(szTemplate, "%d", inDataLen);
		xmlNewChild(sr816100rev_Node, NULL, BAD_CAST "F39Adr", BAD_CAST szTemplate);

		/* <ECCAdr> */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_  + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
			    _ECC_F24_LENTH_ + inField32 + _ECC_F37_LENTH_ + _ECC_F39_LENTH_ + _ECC_F41_LENTH_ + _ECC_F56_LENTH_;
		sprintf(szTemplate, "%d", inDataLen);
		xmlNewChild(sr816100rev_Node, NULL, BAD_CAST "ECCAdr", BAD_CAST szTemplate);
//	}
//	else
//	{
//		/* MFES規格 */
//		/* <F39Adr> */
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
//			    _ECC_F24_LENTH_;
//		sprintf(szTemplate, "%d", inDataLen);
//		xmlNewChild(sr816100rev_Node, NULL, BAD_CAST "F39Adr", BAD_CAST szTemplate);
//
//		/* <ECCAdr> */
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_  + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
//			    _ECC_F24_LENTH_ + _ECC_F39_LENTH_ + _ECC_F41_LENTH_ + _ECC_F56_LENTH_;
//		sprintf(szTemplate, "%d", inDataLen);
//		xmlNewChild(sr816100rev_Node, NULL, BAD_CAST "ECCAdr", BAD_CAST szTemplate);
//	}

	/* <816101> =================================================================================加值取消0100 */
	memset(szSendPacket, 0x00, sizeof(szSendPacket));
        inTxnCode = _TICKET_EASYCARD_VOID_TOP_UP_;
        pobTran->inISOTxnCode = _TICKET_EASYCARD_VOID_TOP_UP_;
            
        /* 組 ISO 電文 */
	ginBeforeIndex = 0;
	if ((inSendLen = inNCCC_TICKET_PackISO(pobTran, (unsigned char *)szSendPacket, inTxnCode)) <= 0)
	{
		/* 清空佔用記憶體 */
		inXML_End(&srDoc);
	
		/* 上傳不能Show錯誤訊息 */
		pobTran->inErrorMsg = _ERROR_CODE_V3_NONE_;
		return (VS_ISO_PACK_ERR);
	}
	
	/* 回復交易碼 */
	pobTran->inISOTxnCode = pobTran->srTRec.inCode;
	
	/* 沒找到 */
	if (ginBeforeIndex == 0)
	{
		/* 清空佔用記憶體 */
		inXML_End(&srDoc);
	
	        return (VS_ERROR);
	}
	
	/* <816101> */
	sr816101_Node = xmlNewNode(NULL, BAD_CAST "816101");
	xmlAddChild(srRoot_Node, sr816101_Node);
        
	/* <BefData> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%s", "0000");
	inFunc_BCD_to_ASCII(&szTemplate[4], (unsigned char*)&szSendPacket[0], ginBeforeIndex);
        xmlNewChild(sr816101_Node, NULL, BAD_CAST "BefData", BAD_CAST szTemplate);
             
        /* <AftData> : 扣掉取代的Len + @@@@@ */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inFunc_BCD_to_ASCII(&szTemplate[0], (unsigned char*)&szSendPacket[ginBeforeIndex + 7], inSendLen - ginBeforeIndex - 7);
        xmlNewChild(sr816101_Node, NULL, BAD_CAST "AftData", BAD_CAST szTemplate);

	/* <F03Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr816101_Node, NULL, BAD_CAST "F03Adr", BAD_CAST szTemplate);
	
	/* <F04Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr816101_Node, NULL, BAD_CAST "F04Adr", BAD_CAST szTemplate);
        
	/* <F11Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr816101_Node, NULL, BAD_CAST "F11Adr", BAD_CAST szTemplate);
	
	/* <F35Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F22_LENTH_ + _ECC_F24_LENTH_ +
                    _ECC_F25_LENTH_ + inField32 + 1;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr816101_Node, NULL, BAD_CAST "F35Adr", BAD_CAST szTemplate);
        
        /* <F37Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F22_LENTH_ + _ECC_F24_LENTH_ +
                    _ECC_F25_LENTH_ + inField32 + _ECC_F35_LENTH_ + 8;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr816101_Node, NULL, BAD_CAST "F37Adr", BAD_CAST szTemplate);
        
	/* <F57Adr> */
	if (memcmp(szIFESMode, "Y", 1) == 0)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = ginECC_F57_Len;
		sprintf(szTemplate, "-%d", inDataLen);
		xmlNewChild(sr816101_Node, NULL, BAD_CAST "F57Adr", BAD_CAST szTemplate);
	}
	
        /* <F59Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = ginECC_F59_ET_Len;
	sprintf(szTemplate, "-%d", inDataLen);
        xmlNewChild(sr816101_Node, NULL, BAD_CAST "F59Adr", BAD_CAST szTemplate);

//	memset(szFESMode, 0x00, sizeof(szFESMode));
//	inGetNCCCFESMode(szFESMode);
//	memset(szTemplate, 0x00, sizeof(szTemplate));
//	inGetCloud_MFES(szTemplate);
//	if (memcmp(szFESMode, _NCCC_05_ATS_MODE_, 2) == 0	||
//	    memcmp(szFESMode, _NCCC_04_MPAS_MODE_, 2) == 0	||
//	   (memcmp(szFESMode, _NCCC_03_MFES_MODE_, 2) == 0	&&
//	    memcmp(szTemplate, "Y", strlen("Y")) == 0))
//	{
		/* ATS規格 - 回覆跟Host長度有關，目前一樣 */
		/* <F39Adr> */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
			    _ECC_F24_LENTH_ + inField32 + _ECC_F37_LENTH_ + _ECC_F38_LENTH_;
		sprintf(szTemplate, "%d", inDataLen);
		xmlNewChild(sr816101_Node, NULL, BAD_CAST "F39Adr", BAD_CAST szTemplate);

		/* <ECCAdr> */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_  + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
			    _ECC_F24_LENTH_ + inField32 + _ECC_F37_LENTH_ + _ECC_F38_LENTH_ + _ECC_F39_LENTH_ + _ECC_F41_LENTH_ + _ECC_F56_LENTH_;
		sprintf(szTemplate, "%d", inDataLen);
		xmlNewChild(sr816101_Node, NULL, BAD_CAST "ECCAdr", BAD_CAST szTemplate);
//	}
//	else
//	{
//		/* MFES規格 */
//		/* <F39Adr> */
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
//			    _ECC_F24_LENTH_;
//		sprintf(szTemplate, "%d", inDataLen);
//		xmlNewChild(sr816101_Node, NULL, BAD_CAST "F39Adr", BAD_CAST szTemplate);
//
//		/* <ECCAdr> */
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_  + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
//			    _ECC_F24_LENTH_ + _ECC_F39_LENTH_ + _ECC_F41_LENTH_ + _ECC_F56_LENTH_;
//		sprintf(szTemplate, "%d", inDataLen);
//		xmlNewChild(sr816101_Node, NULL, BAD_CAST "ECCAdr", BAD_CAST szTemplate);
//	}
	
        /* <816101_Adv>==============================================================================手動加值0220 */
        memset(szSendPacket, 0x00, sizeof(szSendPacket));
        inTxnCode = _ADVICE_;
        pobTran->inISOTxnCode = _ADVICE_;
            
        /* 組 ISO 電文 */
	ginBeforeIndex = 0;
	if ((inSendLen = inNCCC_TICKET_PackISO(pobTran, (unsigned char *)szSendPacket, inTxnCode)) <= 0)
	{
		/* 清空佔用記憶體 */
		inXML_End(&srDoc);
	
		/* 上傳不能Show錯誤訊息 */
		pobTran->inErrorMsg = _ERROR_CODE_V3_NONE_;
		return (VS_ISO_PACK_ERR);
	}
	
	/* 回復交易碼 */
	pobTran->inISOTxnCode = pobTran->srTRec.inCode;
	
	/* 沒找到 */
	if (ginBeforeIndex == 0)
	{
		/* 清空佔用記憶體 */
		inXML_End(&srDoc);
	
	        return (VS_ERROR);
	}

        /* <816101_Adv> */
	sr816101adv_Node = xmlNewNode(NULL, BAD_CAST "816101_Adv");
	xmlAddChild(srRoot_Node, sr816101adv_Node);
        
	/* <BefData> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%s", "0000");
	inFunc_BCD_to_ASCII(&szTemplate[4], (unsigned char*)&szSendPacket[0], ginBeforeIndex);
        xmlNewChild(sr816101adv_Node, NULL, BAD_CAST "BefData", BAD_CAST szTemplate);
             
        /* <AftData> : 扣掉取代的Len + @@@@@ */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inFunc_BCD_to_ASCII(&szTemplate[0], (unsigned char*)&szSendPacket[ginBeforeIndex + 7], inSendLen - ginBeforeIndex - 7);
        xmlNewChild(sr816101adv_Node, NULL, BAD_CAST "AftData", BAD_CAST szTemplate);

	/* <F03Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr816101adv_Node, NULL, BAD_CAST "F03Adr", BAD_CAST szTemplate);
	
	/* <F04Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr816101adv_Node, NULL, BAD_CAST "F04Adr", BAD_CAST szTemplate);
        
	/* <F11Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr816101adv_Node, NULL, BAD_CAST "F11Adr", BAD_CAST szTemplate);

        /* <F12Adr> */
        memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr816101adv_Node, NULL, BAD_CAST "F12Adr", BAD_CAST szTemplate);
        
        /* <F13Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr816101adv_Node, NULL, BAD_CAST "F13Adr", BAD_CAST szTemplate);
        
	/* <F35Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
		    _ECC_F22_LENTH_ + _ECC_F24_LENTH_ + _ECC_F25_LENTH_ + inField32 + 1;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr816101adv_Node, NULL, BAD_CAST "F35Adr", BAD_CAST szTemplate);
        
        /* <F37Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
		    _ECC_F22_LENTH_ + _ECC_F24_LENTH_ + _ECC_F25_LENTH_ + inField32 + _ECC_F35_LENTH_ + 8;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr816101adv_Node, NULL, BAD_CAST "F37Adr", BAD_CAST szTemplate);

	/* <F57Adr> */
	if (memcmp(szIFESMode, "Y", 1) == 0)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = ginECC_F57_Len;
		sprintf(szTemplate, "-%d", inDataLen);
		xmlNewChild(sr816101adv_Node, NULL, BAD_CAST "F57Adr", BAD_CAST szTemplate);
	}
	
	 /* <F59Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = ginECC_F59_ET_Len;
	sprintf(szTemplate, "-%d", inDataLen);
        xmlNewChild(sr816101adv_Node, NULL, BAD_CAST "F59Adr", BAD_CAST szTemplate);

//	memset(szFESMode, 0x00, sizeof(szFESMode));
//	inGetNCCCFESMode(szFESMode);
//	memset(szTemplate, 0x00, sizeof(szTemplate));
//	inGetCloud_MFES(szTemplate);
//	if (memcmp(szFESMode, _NCCC_05_ATS_MODE_, 2) == 0	||
//	    memcmp(szFESMode, _NCCC_04_MPAS_MODE_, 2) == 0	||
//	   (memcmp(szFESMode, _NCCC_03_MFES_MODE_, 2) == 0	&&
//	    memcmp(szTemplate, "Y", strlen("Y")) == 0))
//	{
		/* ATS規格 - 回覆跟Host長度有關，目前一樣 */
		/* <F39Adr> */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F24_LENTH_ + inField32 + 
			    _ECC_F37_LENTH_ + _ECC_F38_LENTH_;
		sprintf(szTemplate, "%d", inDataLen);
		xmlNewChild(sr816101adv_Node, NULL, BAD_CAST "F39Adr", BAD_CAST szTemplate);

		/* <ECCAdr> */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_  + _ECC_F11_LENTH_ + _ECC_F24_LENTH_ + inField32 + 
			    _ECC_F37_LENTH_ + _ECC_F38_LENTH_ + _ECC_F39_LENTH_ + _ECC_F41_LENTH_ + _ECC_F56_LENTH_;
		sprintf(szTemplate, "%d", inDataLen);
		xmlNewChild(sr816101adv_Node, NULL, BAD_CAST "ECCAdr", BAD_CAST szTemplate);
//	}
//	else
//	{
//		/* MFES規格 */
//		/* <F39Adr> */
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F24_LENTH_;
//		sprintf(szTemplate, "%d", inDataLen);
//		xmlNewChild(sr816101adv_Node, NULL, BAD_CAST "F39Adr", BAD_CAST szTemplate);
//
//		/* <ECCAdr> */
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_  + _ECC_F11_LENTH_ + _ECC_F24_LENTH_ + 
//			    _ECC_F39_LENTH_ + _ECC_F41_LENTH_ + + _ECC_F56_LENTH_;
//		sprintf(szTemplate, "%d", inDataLen);
//		xmlNewChild(sr816101adv_Node, NULL, BAD_CAST "ECCAdr", BAD_CAST szTemplate);
//	}

        /* <816101_Rev>==============================================================================手動加值0400 */
        memset(szSendPacket, 0x00, sizeof(szSendPacket));
        inTxnCode = _REVERSAL_;
        pobTran->inISOTxnCode = _REVERSAL_;
  
        /* 組 ISO 電文 */
	ginBeforeIndex = 0;
	if ((inSendLen = inNCCC_TICKET_PackISO(pobTran, (unsigned char *)szSendPacket, inTxnCode)) <= 0)
	{
		/* 清空佔用記憶體 */
		inXML_End(&srDoc);
		
		/* 上傳不能Show錯誤訊息 */
		pobTran->inErrorMsg = _ERROR_CODE_V3_NONE_;
		
		return (VS_ISO_PACK_ERR);
	}
	
	/* 回復交易碼 */
	pobTran->inISOTxnCode = pobTran->srTRec.inCode;
	
	/* 沒找到 */
	if (ginBeforeIndex == 0)
	{
		/* 清空佔用記憶體 */
		inXML_End(&srDoc);
		
	        return (VS_ERROR);
	}
	
        /* <816101_Rev> */
	sr816101rev_Node = xmlNewNode(NULL, BAD_CAST "816101_Rev");
	xmlAddChild(srRoot_Node, sr816101rev_Node);

	/* <BefData> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%s", "0000");
	inFunc_BCD_to_ASCII(&szTemplate[4], (unsigned char*)&szSendPacket[0], ginBeforeIndex);
        xmlNewChild(sr816101rev_Node, NULL, BAD_CAST "BefData", BAD_CAST szTemplate);
             
        /* <AftData> : 扣掉取代的Len + @@@@@ */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inFunc_BCD_to_ASCII(&szTemplate[0], (unsigned char*)&szSendPacket[ginBeforeIndex + 7], inSendLen - ginBeforeIndex - 7);
        xmlNewChild(sr816101rev_Node, NULL, BAD_CAST "AftData", BAD_CAST szTemplate);

	/* <F03Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr816101rev_Node, NULL, BAD_CAST "F03Adr", BAD_CAST szTemplate);
	
	/* <F04Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr816101rev_Node, NULL, BAD_CAST "F04Adr", BAD_CAST szTemplate);
        
	/* <F11Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr816101rev_Node, NULL, BAD_CAST "F11Adr", BAD_CAST szTemplate);
	
	/* <F35Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F22_LENTH_ + _ECC_F24_LENTH_ +
                    _ECC_F25_LENTH_ + inField32 + 1;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr816101rev_Node, NULL, BAD_CAST "F35Adr", BAD_CAST szTemplate);
        
	/* <F37Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F22_LENTH_ + _ECC_F24_LENTH_ +
                    _ECC_F25_LENTH_ + inField32 + _ECC_F35_LENTH_ + 8;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr816101rev_Node, NULL, BAD_CAST "F37Adr", BAD_CAST szTemplate);
        
	/* <F57Adr> */
	if (memcmp(szIFESMode, "Y", 1) == 0)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = ginECC_F57_Len;
		sprintf(szTemplate, "-%d", inDataLen);
		xmlNewChild(sr816101rev_Node, NULL, BAD_CAST "F57Adr", BAD_CAST szTemplate);
	}
	
        /* <F59Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = ginECC_F59_ET_Len;
	sprintf(szTemplate, "-%d", inDataLen);
        xmlNewChild(sr816101rev_Node, NULL, BAD_CAST "F59Adr", BAD_CAST szTemplate);

//	memset(szFESMode, 0x00, sizeof(szFESMode));
//	inGetNCCCFESMode(szFESMode);
//	memset(szTemplate, 0x00, sizeof(szTemplate));
//	inGetCloud_MFES(szTemplate);
//	if (memcmp(szFESMode, _NCCC_05_ATS_MODE_, 2) == 0	||
//	    memcmp(szFESMode, _NCCC_04_MPAS_MODE_, 2) == 0	||
//	   (memcmp(szFESMode, _NCCC_03_MFES_MODE_, 2) == 0	&&
//	    memcmp(szTemplate, "Y", strlen("Y")) == 0))
//	{
		/* ATS規格 - 回覆跟Host長度有關，目前一樣 */
		/* <F39Adr> */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
			    _ECC_F24_LENTH_ + inField32 + _ECC_F37_LENTH_ + _ECC_F38_LENTH_;
		sprintf(szTemplate, "%d", inDataLen);
		xmlNewChild(sr816101rev_Node, NULL, BAD_CAST "F39Adr", BAD_CAST szTemplate);

		/* <ECCAdr> */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_  + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
			    _ECC_F24_LENTH_ + inField32 + _ECC_F37_LENTH_ + _ECC_F39_LENTH_ + _ECC_F41_LENTH_ + _ECC_F56_LENTH_;
		sprintf(szTemplate, "%d", inDataLen);
		xmlNewChild(sr816101rev_Node, NULL, BAD_CAST "ECCAdr", BAD_CAST szTemplate);
//	}
//	else
//	{
//		/* MFES規格 */
//		/* <F39Adr> */
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
//			    _ECC_F24_LENTH_;
//		sprintf(szTemplate, "%d", inDataLen);
//		xmlNewChild(sr816101rev_Node, NULL, BAD_CAST "F39Adr", BAD_CAST szTemplate);
//
//		/* <ECCAdr> */
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F04_LENTH_  + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + 
//			    _ECC_F24_LENTH_ + _ECC_F39_LENTH_ + _ECC_F41_LENTH_ + _ECC_F56_LENTH_;
//		sprintf(szTemplate, "%d", inDataLen);
//		xmlNewChild(sr816101rev_Node, NULL, BAD_CAST "ECCAdr", BAD_CAST szTemplate);
//	}
	
        /* 儲存xml檔 xml檔現在會自動換行了 */
        inXML_SaveFile(_ECC_API_REQ_FILE2_, &srDoc, "utf-8", 1);
	inFile_Move_File(_ECC_API_REQ_FILE2_, _AP_ROOT_PATH_, "", _ECC_FOLDER_PATH_);
	
	/* 清空佔用記憶體 */
	inXML_End(&srDoc);
	
	if (inECC_Insert_Temp() != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inECC_PACK_Void_TOP_UP_ICERAPI_NCCC() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inECC_Inquiry_Flow
Date&Time       :2018/4/2 下午 1:13
Describe        :
*/
int inECC_Inquiry_Flow(TRANSACTION_OBJECT *pobTran)
{
        int     inRetVal = 0;
	char	szDebugMsg[100 + 1] = {0};
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inECC_Inquiry_Flow() START !");
	}
	
	/* 初始化是否已重試(避免少砍ICERAPI2.res) */
	guszECCRetryBit = VS_FALSE;

        /* 組電文(生成ICERAPI.REQ) */
        inECC_PACK_Inquiry_ICERAPI(pobTran);
        
        /* AdditionalTcpipData */
        inECC_PACK_Inquiry_ICERAPI_NCCC(pobTran);

        if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "ICER Start");
		inLogPrintf(AT, szDebugMsg);
	}

	inRetVal = ICERApi_exe();

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "ICER End");
		inLogPrintf(AT, szDebugMsg);
	}

	/* 解電文(分析ICERAPI.RES) */
	inRetVal = inECC_UNPACK_ICERAPI(pobTran, _ECC_API_RES_FILE_);	
	
	if (inRetVal != VS_SUCCESS)
	{
	        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inCTLS_Set_LED(_CTLS_LIGHT_RED_);
		inDISP_ChineseFont("悠遊卡檔案異常", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
		inDISP_BEEP(3, 1000);

		return (VS_ERROR);
	}
        
        /* 檢核回應碼 <T3901>開始 */
        if (memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "0", 1) != 0)
        {
		if (!memcmp(pobTran->srTRec.szRespCode, "A0", 2)	||
		    !memcmp(pobTran->srTRec.szRespCode, "A1", 2))
		{
			ginMacError++;
			inECC_UNPACK_ICERAPI_NCCC(pobTran, _ECC_API_RES_FILE2_, _ECC_FOLDER_PATH_);
		}
		
                /* 其他錯誤Display */
		inECC_ResponeCode(pobTran);

		return (VS_ERROR);
        }
	
	/* 抓必要資料 */
	if (inECC_UNPACK_ICERAPI_NCCC(pobTran, _ECC_API_RES_FILE2_, _ECC_FOLDER_PATH_) != VS_SUCCESS)
		return (VS_ERROR);

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inECC_Inquiry_Flow() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inECC_PACK_Inquiry_ICERAPI
Date&Time       :2018/4/2 下午 1:29
Describe        :
*/
int inECC_PACK_Inquiry_ICERAPI(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_ERROR;
	int		inESVCIndex = 0;
	char		szTemplate[20 + 1] = {0};
	char		szSTAN[6 + 1] = {0};
	char		szInvoiceNum[6 + 1] = {0};
	RTC_NEXSYS	srRTC = {0};
	xmlDocPtr       srDoc = NULL;
        xmlNodePtr      srRoot_Node = NULL, srTRANS_Node = NULL;
#if 	READER_MANUFACTURERS == LINUX_API
	unsigned long	ulHandle = 0;
#else
	int		inFd = 0;
#endif
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inECC_PACK_Inquiry_ICERAPI() START !");
	}

	if (inLoadTDTRec(_TDT_INDEX_01_ECC_) < 0)
                return (VS_ERROR);
	
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTicket_HostIndex(szTemplate);
	inESVCIndex = atoi(szTemplate);
        
        if (inLoadHDTRec(inESVCIndex) != VS_SUCCESS)
                return (VS_ERROR);
	if (inLoadHDPTRec(inESVCIndex) != VS_SUCCESS)
                return (VS_ERROR);
                
        if (inLoadECCDTRec(0) != VS_SUCCESS)
	{
                return (VS_ERROR);
	}                    

	/* 刪除原本的ICERAPI */
        inFile_Unlink_File(_ECC_API_REQ_CHECK_, _ECC_FOLDER_PATH_);	/* 檢核用 */
	inFile_Unlink_File(_ECC_API_REQ_FILE_, _ECC_FOLDER_PATH_);
	inFile_Unlink_File(_ECC_API_REQ_FILE2_, _ECC_FOLDER_PATH_);
	inFile_Unlink_File(_ECC_API_RES_FILE_, _ECC_FOLDER_PATH_);	/* 防止用到上次資料結果 */
	/* Retry時需要參考RES2，不能砍 */
	if (guszECCRetryBit == VS_TRUE)
	{
		
	}
	else
	{
		inFile_Unlink_File(_ECC_API_RES_FILE2_, _ECC_FOLDER_PATH_);
	}
	
	/* 建立一個新的檔案 */
        srDoc = xmlNewDoc(BAD_CAST "1.0");
        srRoot_Node = xmlNewNode(NULL, BAD_CAST "TransXML");
        xmlDocSetRootElement (srDoc, srRoot_Node);
 
        /* <TRANS> */
	srTRANS_Node = xmlNewNode(NULL, BAD_CAST "TRANS");
	xmlAddChild(srRoot_Node, srTRANS_Node);

        /* <T0100> */
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T0100", BAD_CAST "0200");

        /* <T0300> */
        xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T0300", BAD_CAST "296000");
        
        /* <T1100> : STAN 不知道用在哪，先比照NCCC STAN */
	memset(szSTAN, 0x00, sizeof(szSTAN));
	inGetSTANNum(szSTAN);
        xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1100", BAD_CAST szSTAN);
        
        /* <T1101> : CMAS自用，區分上筆交易(會回傳) */
	memset(szSTAN, 0x00, sizeof(szSTAN));
	inGetTicket_STAN(szSTAN);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%06d", atoi(szSTAN) + 1);
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1101", BAD_CAST szTemplate);
	inSetTicket_STAN(szTemplate);
	inSaveTDTRec(_TDT_INDEX_01_ECC_);

        /* <T1102> : NCCC STAN */
        memset(szSTAN, 0x00, sizeof(szSTAN));
	inGetSTANNum(szSTAN);
        xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1102", BAD_CAST szSTAN);
        
        /* <T1103> : NCCC RRN(後三碼) */
	memset(szInvoiceNum, 0x00, sizeof(szInvoiceNum));
	inGetTicket_InvNum(szInvoiceNum);
        xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1103", BAD_CAST szInvoiceNum);

        /* <T1200> : hhmmss */
	memset(&srRTC, 0x00, sizeof(srRTC));
	inFunc_GetSystemDateAndTime(&srRTC);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%02d%02d%02d", srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1200", BAD_CAST szTemplate);
        
	/* <T1300> : yyyymmdd */
	/* hhmmss就抓過一次時間了，不用重抓 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "20%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1300", BAD_CAST szTemplate);
	
        /* <T4108> : LCD Control Flag 單純顯示畫面用 */
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T4108", BAD_CAST "0");
        
	/* 儲存xml檔 xml檔現在會自動換行了 */
        inXML_SaveFile(_ECC_API_REQ_FILE_, &srDoc, "utf-8", 1);
	inFile_Move_File(_ECC_API_REQ_FILE_, _AP_ROOT_PATH_, "", _ECC_FOLDER_PATH_);
	
	/* 清空佔用記憶體 */
	inXML_End(&srDoc);
	
	/* 步驟完成，建立檢核擋 */
#if 	READER_MANUFACTURERS == LINUX_API
	inRetVal = inFILE_Create(&ulHandle, (unsigned char*)_ECC_API_REQ_CHECK_);
	/* 不管成功或失敗都要關檔 */
	inFILE_Close(&ulHandle);
	/* 抓到ICERData資料夾 */
	inFile_Move_File(_ECC_API_REQ_CHECK_, _FS_DATA_PATH_, "", _ECC_FOLDER_PATH_);
#else
	inRetVal = inFile_Linux_Create(&inFd, _ECC_API_REQ_CHECK_);
	/* 不管成功或失敗都要關檔 */
	inFile_Linux_Close(inFd);
	/* 會有權限問題，要先改權限 */
	inFunc_Data_Chmod("u+rwx", _ECC_API_REQ_CHECK_, _AP_ROOT_PATH_);
	/* 抓到ICERData資料夾 */
	inFile_Move_File(_ECC_API_REQ_CHECK_, _AP_ROOT_PATH_, "", _ECC_FOLDER_PATH_);
#endif

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inECC_PACK_Inquiry_ICERAPI() END !");
		inLogPrintf(AT, "----------------------------------------");
	}

        return (VS_SUCCESS);
}

/*
Function        :inECC_PACK_Inquiry_ICERAPI_NCCC
Date&Time       :2018/4/2 下午 3:05
Describe        :
*/
int inECC_PACK_Inquiry_ICERAPI_NCCC(TRANSACTION_OBJECT *pobTran)
{
	xmlDocPtr       srDoc = NULL;
        xmlNodePtr      srRoot_Node = NULL;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inECC_PACK_Inquiry_ICERAPI_NCCC() START !");
	}
        
        /* 刪除原本的ICERAPI */
	inFile_Unlink_File(_ECC_API_REQ_FILE2_, _ECC_FOLDER_PATH_);
	/* 建立一個新的檔案 */
        srDoc = xmlNewDoc(BAD_CAST "1.0");
        srRoot_Node = xmlNewNode(NULL, BAD_CAST "TransXML");
        xmlDocSetRootElement (srDoc, srRoot_Node);

	/* 儲存xml檔 xml檔現在會自動換行了 */
        inXML_SaveFile(_ECC_API_REQ_FILE2_, &srDoc, "utf-8", 1);
	inFile_Move_File(_ECC_API_REQ_FILE2_, _AP_ROOT_PATH_, "", _ECC_FOLDER_PATH_);
	
	/* 清空佔用記憶體 */
	inXML_End(&srDoc);
	
	if (inECC_Insert_Temp() != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inECC_PACK_Inquiry_ICERAPI_NCCC() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inECC_Settle_Flow
Date&Time       :2018/4/12 下午 4:05
Describe        :
*/
int inECC_Settle_Flow(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;
	char	szDemoMode[2 + 1] = {0};
	char	szDebugMsg[100 + 1] = {0};
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inECC_Settle_Flow() START !");
	}
	
	/* 先送其他票證的ADV和REV */
	inRetVal = inFLOW_RunFunction(pobTran, _TICKET_CONNECT_);
	if (inRetVal == VS_SUCCESS)
	{
		inRetVal = inECC_Settle_Other_REV_ADV(pobTran);
		
		inFLOW_RunFunction(pobTran, _TICKET_DISCONNECT_);
		
		/* 結帳前reversal 或 adv失敗*/
		if (inRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "結帳前reversal 或 adv失敗");
			}
			
			return (inRetVal);
		}
	}
	else
	{
		return (inRetVal);
	}
	
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		
	}
	else
	{
		/* 初始化是否已重試(避免少砍ICERAPI2.res) */
		guszECCRetryBit = VS_FALSE;
	
		/* 組電文(生成ICERAPI.REQ) */
		inECC_PACK_Settle_ICERAPI(pobTran);

		/* AdditionalTcpipData */
		inECC_PACK_Settle_ICERAPI_NCCC(pobTran);

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "ICER Start");
			inLogPrintf(AT, szDebugMsg);
		}
		
		inRetVal = ICERApi_exe();
		
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "ICER End");
			inLogPrintf(AT, szDebugMsg);
		}

		/* 解電文(分析ICERAPI.RES) */
		inRetVal = inECC_UNPACK_ICERAPI(pobTran, _ECC_API_RES_FILE_);	

		if (inRetVal != VS_SUCCESS)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inCTLS_Set_LED(_CTLS_LIGHT_RED_);
			inDISP_ChineseFont("悠遊卡檔案異常", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
			inDISP_BEEP(3, 1000);

			return (VS_ERROR);
		}

		/* 檢核回應碼 <T3901>開始 */
		if (memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "0", 1) != 0)
		{
			if (!memcmp(pobTran->srTRec.szRespCode, "A0", 2)	||
			    !memcmp(pobTran->srTRec.szRespCode, "A1", 2))
			{
				ginMacError++;
				inECC_UNPACK_ICERAPI_NCCC(pobTran, _ECC_API_RES_FILE2_, _ECC_FOLDER_PATH_);
			}
			
			/* 其他錯誤Display */
			inECC_ResponeCode(pobTran);

			return (VS_ERROR);
		}
		else
		{
			if (inNCCC_TICKET_SetReversalCnt(pobTran, _RESET_) == VS_ERROR)
			{
				return (VS_ERROR);
			}

			/* 結帳成功 把請先結帳的bit關掉 */
			inNCCC_TICKET_SetMustSettleBit(pobTran, "N");
		}

		/* 抓必要資料 */
		if (inECC_UNPACK_ICERAPI_NCCC(pobTran, _ECC_API_RES_FILE2_, _ECC_FOLDER_PATH_) != VS_SUCCESS)
                {
			/* 不回傳錯誤，可能會導致沒清批次 */
                        vdUtility_SYSFIN_LogMessage(AT, "inECC_Settle_Flow inECC_UNPACK_ICERAPI_NCCC failed");
                }
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inECC_Settle_Flow() END !");
		inLogPrintf(AT, "----------------------------------------");
	}

        return (VS_SUCCESS);
} 

/*
Function        :inECC_PACK_Settle_ICERAPI
Date&Time       :2018/4/12 下午 4:24
Describe        :
*/
int inECC_PACK_Settle_ICERAPI(TRANSACTION_OBJECT *pobTran)
{
	int			inRetVal = VS_ERROR;
	int			inESVCIndex = 0;
	char			szTemplate[20 + 1] = {0};
	char			szSTAN[6 + 1] = {0};
	char			szInvoiceNum[6 + 1] = {0};
	RTC_NEXSYS		srRTC = {0};
	xmlDocPtr		srDoc = NULL;
        xmlNodePtr		srRoot_Node = NULL, srTRANS_Node = NULL, sr5592_Node = NULL;
	TICKET_ACCUM_TOTAL_REC	srTicketAccum;
#if 	READER_MANUFACTURERS == LINUX_API
	unsigned long	ulHandle = 0;
#else
	int		inFd = 0;
#endif

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inECC_PACK_Settle_ICERAPI() START !");
	}
        
        if (inLoadTDTRec(_TDT_INDEX_01_ECC_) < 0)
                return (VS_ERROR);
	
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTicket_HostIndex(szTemplate);
	inESVCIndex = atoi(szTemplate);
        
        if (inLoadHDTRec(inESVCIndex) != VS_SUCCESS)
                return (VS_ERROR);
	if (inLoadHDPTRec(inESVCIndex) != VS_SUCCESS)
                return (VS_ERROR);
                
        if (inLoadECCDTRec(0) != VS_SUCCESS)
	{
                return (VS_ERROR);
	}
        
        /* 刪除原本的ICERAPI */
        inFile_Unlink_File(_ECC_API_REQ_CHECK_, _ECC_FOLDER_PATH_);	/* 檢核用 */
	inFile_Unlink_File(_ECC_API_REQ_FILE_, _ECC_FOLDER_PATH_);
	inFile_Unlink_File(_ECC_API_REQ_FILE2_, _ECC_FOLDER_PATH_);
	inFile_Unlink_File(_ECC_API_RES_FILE_, _ECC_FOLDER_PATH_);	/* 防止用到上次資料結果 */
	/* Retry時需要參考RES2，不能砍 */
	if (guszECCRetryBit == VS_TRUE)
	{
		
	}
	else
	{
		inFile_Unlink_File(_ECC_API_RES_FILE2_, _ECC_FOLDER_PATH_);
	}
	
	/* 建立一個新的檔案 */
        srDoc = xmlNewDoc(BAD_CAST "1.0");
        srRoot_Node = xmlNewNode(NULL, BAD_CAST "TransXML");
        xmlDocSetRootElement (srDoc, srRoot_Node);
 
        /* <TRANS> */
	srTRANS_Node = xmlNewNode(NULL, BAD_CAST "TRANS");
	xmlAddChild(srRoot_Node, srTRANS_Node);

        /* <T0100> */
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T0100", BAD_CAST "0200");

        /* <T0300> */
        xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T0300", BAD_CAST "900099");
        
	/* <T0400> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%lu00", pobTran->srTRec.lnTxnAmount);
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T0400", BAD_CAST szTemplate);

	/* <T1100> : STAN 不知道用在哪，先比照NCCC STAN */
	memset(szSTAN, 0x00, sizeof(szSTAN));
	inGetSTANNum(szSTAN);
        xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1100", BAD_CAST szSTAN);

	/* <T1101> : CMAS自用，區分上筆交易(會回傳) */
	memset(szSTAN, 0x00, sizeof(szSTAN));
	inGetTicket_STAN(szSTAN);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%06d", atoi(szSTAN) + 1);
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1101", BAD_CAST szTemplate);
	inSetTicket_STAN(szTemplate);
	inSaveTDTRec(_TDT_INDEX_01_ECC_);
        
	/* <T1102> : NCCC STAN */
        memset(szSTAN, 0x00, sizeof(szSTAN));
	inGetSTANNum(szSTAN);
        xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1102", BAD_CAST szSTAN);
        
        /* <T1103> : NCCC RRN(後三碼) */
	memset(szInvoiceNum, 0x00, sizeof(szInvoiceNum));
	inGetTicket_InvNum(szInvoiceNum);
        xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1103", BAD_CAST szInvoiceNum);

        /* <T1200> : hhmmss */
	memset(&srRTC, 0x00, sizeof(srRTC));
	inFunc_GetSystemDateAndTime(&srRTC);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%02d%02d%02d", srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1200", BAD_CAST szTemplate);
	
	/* <T1300> : yyyymmdd */
	/* hhmmss就抓過一次時間了，不用重抓 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "20%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1300", BAD_CAST szTemplate);
        
        /* <T5501> : Batch Number - 悠遊卡特有，同一天流水號累加，隔天就要歸1 */
	/* 如果szTicket_NeedNewBatch在結帳時為Y */
	/* 修改為空白結帳也會遞增 by Russell 2020/1/31 上午 10:06 */
	inECC_Batch_Update();
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inECC_Batch_Check(szTemplate);
        xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T5501", BAD_CAST szTemplate);
        
	/* <T5591> : 總筆數 */
	memset(&srTicketAccum, 0x00, sizeof(srTicketAccum));
	if (inACCUM_GetRecord_ESVC(pobTran, &srTicketAccum) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "GET ECC ACCUM FAIL");
		}
	}
	
	memset(szTemplate, 0x00, sizeof(szTemplate));
        sprintf(szTemplate, "%ld", srTicketAccum.lnEASYCARD_TotalCount);
        xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T5591", BAD_CAST szTemplate);
        
        /* <T5592> : 總金額 */
	/* <559201>要包在<T5592>內 */
	sr5592_Node = xmlNewNode(NULL, BAD_CAST "T5592");
	xmlAddChild(srTRANS_Node, sr5592_Node);
	
	memset(szTemplate, 0x00, sizeof(szTemplate));
        sprintf(szTemplate, "%lld00", srTicketAccum.llEASYCARD_TotalAmount);
        xmlNewChild(sr5592_Node, NULL, BAD_CAST "T559201", BAD_CAST szTemplate);
	
	/* 儲存xml檔 xml檔現在會自動換行了 */
        inXML_SaveFile(_ECC_API_REQ_FILE_, &srDoc, "utf-8", 1);
	inFile_Move_File(_ECC_API_REQ_FILE_, _AP_ROOT_PATH_, "", _ECC_FOLDER_PATH_);
	
	/* 清空佔用記憶體 */
	inXML_End(&srDoc);
	
	/* 步驟完成，建立檢核擋 */
#if 	READER_MANUFACTURERS == LINUX_API
	inRetVal = inFILE_Create(&ulHandle, (unsigned char*)_ECC_API_REQ_CHECK_);
	/* 不管成功或失敗都要關檔 */
	inFILE_Close(&ulHandle);
	/* 抓到ICERData資料夾 */
	inFile_Move_File(_ECC_API_REQ_CHECK_, _FS_DATA_PATH_, "", _ECC_FOLDER_PATH_);
#else
	inRetVal = inFile_Linux_Create(&inFd, _ECC_API_REQ_CHECK_);
	/* 不管成功或失敗都要關檔 */
	inFile_Linux_Close(inFd);
	/* 會有權限問題，要先改權限 */
	inFunc_Data_Chmod("u+rwx", _ECC_API_REQ_CHECK_, _AP_ROOT_PATH_);
	/* 抓到ICERData資料夾 */
	inFile_Move_File(_ECC_API_REQ_CHECK_, _AP_ROOT_PATH_, "", _ECC_FOLDER_PATH_);
#endif
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inECC_PACK_Settle_ICERAPI() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inECC_PACK_Settle_ICERAPI_NCCC
Date&Time       :2018/4/12 下午 4:36
Describe        :
*/
int inECC_PACK_Settle_ICERAPI_NCCC(TRANSACTION_OBJECT *pobTran)
{
	int		inDataLen = 0;
	int		inTxnCode = 0;
	int		inOrgCode = 0;
	int		inSendLen = 0;
	int		inField32 = 0;
	int		inField48 = 0;
	char		szFESMode[2 + 1] = {0};
	char		szSendPacket[_NCCC_TICKET_ISO_SEND_ + 1] = {0};
	char		szTemplate[1024 + 1] = {0};
	xmlDocPtr       srDoc = NULL;
        xmlNodePtr      srRoot_Node = NULL, sr900099_Node = NULL;

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inECC_PACK_Settle_ICERAPI_NCCC() START !");
	}
        
	memset(szFESMode, 0x00, sizeof(szFESMode));
	inGetNCCCFESMode(szFESMode);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetCloud_MFES(szTemplate);
        if (memcmp(szFESMode, _NCCC_05_ATS_MODE_, 2) == 0	||
	    memcmp(szFESMode, _NCCC_04_MPAS_MODE_, 2) == 0	||
	   (memcmp(szFESMode, _NCCC_03_MFES_MODE_, 2) == 0	&&
	    memcmp(szTemplate, "Y", strlen("Y")) == 0))
	{
                inField32 = _ECC_F32_LENTH_;
	}
                
        if (strlen(pobTran->srTRec.szStoreID) > 0)
	{
                inField48 = _ECC_F48_LENTH_;
	}
                
        pobTran->srTRec.inTicketType = _TICKET_TYPE_ECC_;
       
        memset(szSendPacket, 0x00, sizeof(szSendPacket));
        inTxnCode = _SETTLE_;
	inOrgCode = pobTran->srTRec.inCode;
        pobTran->inISOTxnCode = inTxnCode;
	pobTran->srTRec.inCode = inTxnCode;
        
	/* 要Load CPT */
	inETHERNET_Begin(pobTran);
	
        /* 組 ISO 電文 */
	ginBeforeIndex = 0;
	if ((inSendLen = inNCCC_TICKET_PackISO(pobTran, (unsigned char *)szSendPacket, inTxnCode)) <= 0)
	{
		/* 上傳不能Show錯誤訊息 */
		pobTran->inErrorMsg = _ERROR_CODE_V3_NONE_;
		return (VS_ISO_PACK_ERR);
	}
	
	/* 回復交易碼 */
	pobTran->srTRec.inCode = inOrgCode;
	pobTran->inISOTxnCode = inOrgCode;
	
	/* 沒找到 */
	if (ginBeforeIndex == 0)
	{
	        return (VS_ERROR);
	}

        /* 刪除原本的ICERAPI */
	inFile_Unlink_File(_ECC_API_REQ_FILE2_, _ECC_FOLDER_PATH_);
	
	/* 建立一個新的檔案 */
        srDoc = xmlNewDoc(BAD_CAST "1.0");
        srRoot_Node = xmlNewNode(NULL, BAD_CAST "TransXML");
        xmlDocSetRootElement (srDoc, srRoot_Node);

        /* <900099> =================================================================================結帳 */
	sr900099_Node = xmlNewNode(NULL, BAD_CAST "900099");
	xmlAddChild(srRoot_Node, sr900099_Node);
        
	/* <BefData> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%s", "0000");
	inFunc_BCD_to_ASCII(&szTemplate[4], (unsigned char*)&szSendPacket[0], ginBeforeIndex);
        xmlNewChild(sr900099_Node, NULL, BAD_CAST "BefData", BAD_CAST szTemplate);
             
        /* <AftData> : 扣掉取代的Len + @@@@@ */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inFunc_BCD_to_ASCII(&szTemplate[0], (unsigned char*)&szSendPacket[ginBeforeIndex + 7], inSendLen - ginBeforeIndex - 7);
        xmlNewChild(sr900099_Node, NULL, BAD_CAST "AftData", BAD_CAST szTemplate);
	
	/* <F03Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr900099_Node, NULL, BAD_CAST "F03Adr", BAD_CAST szTemplate);
	
	/* <F11Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_;
	sprintf(szTemplate, "+%d", inDataLen);
        xmlNewChild(sr900099_Node, NULL, BAD_CAST "F11Adr", BAD_CAST szTemplate);
	
	/* <F63Adr> */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inDataLen = ginECC_F63_Len;
	sprintf(szTemplate, "-%d", inDataLen);
        xmlNewChild(sr900099_Node, NULL, BAD_CAST "F63Adr", BAD_CAST szTemplate);
	
//	memset(szFESMode, 0x00, sizeof(szFESMode));
//	inGetNCCCFESMode(szFESMode);
//	memset(szTemplate, 0x00, sizeof(szTemplate));
//	inGetCloud_MFES(szTemplate);
//	if (memcmp(szFESMode, _NCCC_05_ATS_MODE_, 2) == 0	||
//	    memcmp(szFESMode, _NCCC_04_MPAS_MODE_, 2) == 0	||
//	   (memcmp(szFESMode, _NCCC_03_MFES_MODE_, 2) == 0	&&
//	    memcmp(szTemplate, "Y", strlen("Y")) == 0))
//	{
		/* ATS規格 - 回覆跟Host長度有關，目前一樣 */
		/* <F39Adr> */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + _ECC_F24_LENTH_ + 
			    inField32;
		sprintf(szTemplate, "%d", inDataLen);
		xmlNewChild(sr900099_Node, NULL, BAD_CAST "F39Adr", BAD_CAST szTemplate);

		/* <ECCAdr> */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + _ECC_F24_LENTH_ + 
			    inField32 + _ECC_F39_LENTH_ + _ECC_F41_LENTH_ + _ECC_F56_LENTH_;
		sprintf(szTemplate, "%d", inDataLen);
		xmlNewChild(sr900099_Node, NULL, BAD_CAST "ECCAdr", BAD_CAST szTemplate);
//	}
//	else
//	{
//		/* MFES規格 */
//		/* <F39Adr> */
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + _ECC_F24_LENTH_ + 
//			    inField32;
//		sprintf(szTemplate, "%d", inDataLen);
//		xmlNewChild(sr900099_Node, NULL, BAD_CAST "F39Adr", BAD_CAST szTemplate);
//
//		/* <ECCAdr> */
//		memset(szTemplate, 0x00, sizeof(szTemplate));
//		inDataLen = _ECC_HEADER_LENTH_ + _ECC_F03_LENTH_ + _ECC_F11_LENTH_ + _ECC_F12_LENTH_ + _ECC_F13_LENTH_ + _ECC_F24_LENTH_ + 
//			    inField32 + _ECC_F39_LENTH_ + _ECC_F41_LENTH_ + _ECC_F56_LENTH_;
//		sprintf(szTemplate, "%d", inDataLen);
//		xmlNewChild(sr900099_Node, NULL, BAD_CAST "ECCAdr", BAD_CAST szTemplate);
//	}

	/* 儲存xml檔 xml檔現在會自動換行了 */
        inXML_SaveFile(_ECC_API_REQ_FILE2_, &srDoc, "utf-8", 1);
	inFile_Move_File(_ECC_API_REQ_FILE2_, _AP_ROOT_PATH_, "", _ECC_FOLDER_PATH_);
	
	/* 清空佔用記憶體 */
	inXML_End(&srDoc);
	
	if (inECC_Insert_Temp() != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inECC_PACK_Settle_ICERAPI_NCCC() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inECC_POLL_Flow
Date&Time       :2018/3/30 下午 4:15
Describe        :
*/
int inECC_POLL_Flow(TRANSACTION_OBJECT *pobTran)
{
        int     inRetVal = 0;
	char	szDemoMode[2 + 1] = {0};
	char	szDebugMsg[100 + 1] = {0};
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inECC_POLL_Flow() START !");
	}
        
	/* 教育訓練模式 */
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inECC_POLL_Flow() END !");
			inLogPrintf(AT, "----------------------------------------");
		}
		
		return (VS_SUCCESS);
	}
	else
	{
		/* 初始化是否已重試(避免少砍ICERAPI2.res) */
		guszECCRetryBit = VS_FALSE;
		
		/* 組電文(生成ICERAPI.REQ) */
		inECC_PACK_POLL_ICERAPI(pobTran);

		/* AdditionalTcpipData */
		inECC_PACK_POLL_ICERAPI_NCCC(pobTran);

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "ICER Start");
			inLogPrintf(AT, szDebugMsg);
		}
		
		inRetVal = ICERApi_exe();
		
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "ICER End");
			inLogPrintf(AT, szDebugMsg);
		}

		/* 解電文(分析ICERAPI.RES) */
		inRetVal = inECC_UNPACK_ICERAPI(pobTran, _ECC_API_RES_FILE_);	 

		if (inRetVal != VS_SUCCESS)
		{
			return (VS_ERROR);
		}

		/* 檢核回應碼 <T3901>開始 */
		if (memcmp(&pobTran->srTRec.srECCRec.szAPI_RespCode[0], "0", 1) != 0)
		{
			if (!memcmp(pobTran->srTRec.szRespCode, "A0", 2)	||
			    !memcmp(pobTran->srTRec.szRespCode, "A1", 2))
			{
				ginMacError++;
				inECC_UNPACK_ICERAPI_NCCC(pobTran, _ECC_API_RES_FILE2_, _ECC_FOLDER_PATH_);
			}
			
			return (VS_ERROR);
		}
		
		/* 抓必要資料 */
		if (inECC_UNPACK_ICERAPI_NCCC(pobTran, _ECC_API_RES_FILE2_, _ECC_FOLDER_PATH_) != VS_SUCCESS)
			return (VS_ERROR);

		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inECC_POLL_Flow() END !");
			inLogPrintf(AT, "----------------------------------------");
		}

		return (VS_SUCCESS);
	}
}

/*
Function        :inECC_PACK_POLL_ICERAPI
Date&Time       :2018/3/30 下午 4:46
Describe        :
*/
int inECC_PACK_POLL_ICERAPI(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_ERROR;
	int		inESVCIndex = 0;
	char		szTemplate[20 + 1] = {0};
	char		szSTAN[6 + 1] = {0};
	char		szInvoiceNum[6 + 1] = {0};
	RTC_NEXSYS	srRTC = {0};
	xmlDocPtr       srDoc = NULL;
        xmlNodePtr      srRoot_Node = NULL, srTRANS_Node = NULL;
#if 	READER_MANUFACTURERS == LINUX_API
	unsigned long	ulHandle = 0;
#else
	int		inFd = 0;
#endif
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inECC_PACK_POLL_ICERAPI() START !");
	}
	
	if (inLoadTDTRec(_TDT_INDEX_01_ECC_) < 0)
                return (VS_ERROR);
	
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTicket_HostIndex(szTemplate);
	inESVCIndex = atoi(szTemplate);
        
        if (inLoadHDTRec(inESVCIndex) != VS_SUCCESS)
                return (VS_ERROR);
	if (inLoadHDPTRec(inESVCIndex) != VS_SUCCESS)
                return (VS_ERROR);
                
        if (inLoadECCDTRec(0) != VS_SUCCESS)
	{
                return (VS_ERROR);
	}
           
        /* 刪除原本的ICERAPI */
        inFile_Unlink_File(_ECC_API_REQ_CHECK_, _ECC_FOLDER_PATH_);	/* 檢核用 */
	inFile_Unlink_File(_ECC_API_REQ_FILE_, _ECC_FOLDER_PATH_);
	inFile_Unlink_File(_ECC_API_REQ_FILE2_, _ECC_FOLDER_PATH_);
	inFile_Unlink_File(_ECC_API_RES_FILE_, _ECC_FOLDER_PATH_);	/* 防止用到上次資料結果 */
	/* Retry時需要參考RES2，不能砍 */
	if (guszECCRetryBit == VS_TRUE)
	{
		
	}
	else
	{
		inFile_Unlink_File(_ECC_API_RES_FILE2_, _ECC_FOLDER_PATH_);
	}
	
	/* 建立一個新的檔案 */
        srDoc = xmlNewDoc(BAD_CAST "1.0");
        srRoot_Node = xmlNewNode(NULL, BAD_CAST "TransXML");
        xmlDocSetRootElement (srDoc, srRoot_Node);
 
        /* <TRANS> */
	srTRANS_Node = xmlNewNode(NULL, BAD_CAST "TRANS");
	xmlAddChild(srRoot_Node, srTRANS_Node);

        /* <T0100> */
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T0100", BAD_CAST "0200");

        /* <T0300> */
        xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T0300", BAD_CAST "296062");
        
	/* <T1100> : STAN 不知道用在哪，先比照NCCC STAN */
	memset(szSTAN, 0x00, sizeof(szSTAN));
	inGetSTANNum(szSTAN);
        xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1100", BAD_CAST szSTAN);

	/* <T1101> : CMAS自用，區分上筆交易(會回傳) */
	memset(szSTAN, 0x00, sizeof(szSTAN));
	inGetTicket_STAN(szSTAN);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%06d", atoi(szSTAN) + 1);
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1101", BAD_CAST szTemplate);
	inSetTicket_STAN(szTemplate);
	inSaveTDTRec(_TDT_INDEX_01_ECC_);
        
	/* <T1102> : NCCC STAN */
        memset(szSTAN, 0x00, sizeof(szSTAN));
	inGetSTANNum(szSTAN);
        xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1102", BAD_CAST szSTAN);
        
        /* <T1103> : NCCC RRN(後三碼) */
	memset(szInvoiceNum, 0x00, sizeof(szInvoiceNum));
	inGetTicket_InvNum(szInvoiceNum);
        xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1103", BAD_CAST szInvoiceNum);

        /* <T1200> : hhmmss */
	memset(&srRTC, 0x00, sizeof(srRTC));
	inFunc_GetSystemDateAndTime(&srRTC);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%02d%02d%02d", srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1200", BAD_CAST szTemplate);
	
	/* <T1300> : yyyymmdd */
	/* hhmmss就抓過一次時間了，不用重抓 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "20%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
	xmlNewChild(srTRANS_Node, NULL, BAD_CAST "T1300", BAD_CAST szTemplate);

	/* 儲存xml檔 xml檔現在會自動換行了 */
        inXML_SaveFile(_ECC_API_REQ_FILE_, &srDoc, "utf-8", 1);
	inFile_Move_File(_ECC_API_REQ_FILE_, _AP_ROOT_PATH_, "", _ECC_FOLDER_PATH_);
	
	/* 清空佔用記憶體 */
	inXML_End(&srDoc);
	
	/* 步驟完成，建立檢核擋 */
#if 	READER_MANUFACTURERS == LINUX_API
	inRetVal = inFILE_Create(&ulHandle, (unsigned char*)_ECC_API_REQ_CHECK_);
	/* 不管成功或失敗都要關檔 */
	inFILE_Close(&ulHandle);
	/* 抓到ICERData資料夾 */
	inFile_Move_File(_ECC_API_REQ_CHECK_, _FS_DATA_PATH_, "", _ECC_FOLDER_PATH_);
#else
	inRetVal = inFile_Linux_Create(&inFd, _ECC_API_REQ_CHECK_);
	/* 不管成功或失敗都要關檔 */
	inFile_Linux_Close(inFd);
	/* 會有權限問題，要先改權限 */
	inFunc_Data_Chmod("u+rwx", _ECC_API_REQ_CHECK_, _AP_ROOT_PATH_);
	/* 抓到ICERData資料夾 */
	inFile_Move_File(_ECC_API_REQ_CHECK_, _AP_ROOT_PATH_, "", _ECC_FOLDER_PATH_);
#endif
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inECC_PACK_POLL_ICERAPI() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
        return (VS_SUCCESS);
}        

/*
Function        :inECC_PACK_POLL_ICERAPI_NCCC
Date&Time       :2018/3/30 下午 4:46
Describe        :
*/
int inECC_PACK_POLL_ICERAPI_NCCC(TRANSACTION_OBJECT *pobTran)
{
	xmlDocPtr       srDoc = NULL;
        xmlNodePtr      srRoot_Node = NULL;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inECC_PACK_POLL_ICERAPI_NCCC() START !");
	}
	
	/* 刪除原本的ICERAPI */
	inFile_Unlink_File(_ECC_API_REQ_FILE2_, _ECC_FOLDER_PATH_);
	
	/* 建立一個新的檔案 */
        srDoc = xmlNewDoc(BAD_CAST "1.0");
        srRoot_Node = xmlNewNode(NULL, BAD_CAST "TransXML");
        xmlDocSetRootElement (srDoc, srRoot_Node);
	
	/* 儲存xml檔 xml檔現在會自動換行了 */
        inXML_SaveFile(_ECC_API_REQ_FILE2_, &srDoc, "utf-8", 1);
	inFile_Move_File(_ECC_API_REQ_FILE2_, _AP_ROOT_PATH_, "", _ECC_FOLDER_PATH_);
	
	/* 清空佔用記憶體 */
	inXML_End(&srDoc);
	
	if (inECC_Insert_Temp() != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inECC_PACK_POLL_ICERAPI_NCCC() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inECC_Batch_Check
Date&Time       :2018/3/29 下午 2:03
Describe        :確認是否進批，並回傳批號
*/
int inECC_Batch_Check(char* szECCBatchOut)
{
	int		inCount = 0;
	char		szECCBatch[8 + 1] = {0};
	char		szTermDateTime[8 + 1] = {0};
	char		szTicket_NeedNewBatch[2 + 1] = {0};
	char		szTemplate[2 + 1] = {0};
	RTC_NEXSYS	srRTC = {0};
        
	memset(szTicket_NeedNewBatch, 0x00, sizeof(szTicket_NeedNewBatch));
	inGetTicket_NeedNewBatch(szTicket_NeedNewBatch);
        /* 先檢查是否是第一筆，不是就沿用 */
        if (memcmp(szTicket_NeedNewBatch, "Y", strlen("Y")) == 0)
	{
	        memset(&srRTC, 0x00, sizeof(srRTC));
		inFunc_GetSystemDateAndTime(&srRTC);
                sprintf(szTermDateTime, "%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
		
		inLoadTDTRec(_TDT_INDEX_01_ECC_);  
                memset(szECCBatch, 0x00, sizeof(szECCBatch));
		inGetTicket_Batch(szECCBatch);
		
		/* 是否是同一天 */
                if (memcmp(szTermDateTime, szECCBatch, 6) == 0)
                {
                        /* 同一天流水號+1 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memcpy(szTemplate, &szECCBatch[6], 2);   
                        inCount = atoi(szTemplate) + 1;
                        
                        if (inCount >= 100)
			{
                                inCount = 1;
			}

                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "%02d", inCount);
                }       
                else
                {
                        /* 不同天更新日期，並回到第一批 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "%02d", 1);
                } 
                
                memcpy(&szTermDateTime[6], szTemplate, 2);
		strcpy(szECCBatchOut, szTermDateTime);
        }
	else
	{
		inLoadTDTRec(_TDT_INDEX_01_ECC_);
                memset(szECCBatch, 0x00, sizeof(szECCBatch));
		inGetTicket_Batch(szECCBatch);
		
		strcpy(szECCBatchOut, szECCBatch);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inECC_Batch_Update
Date&Time       :2019/2/26 上午 10:50
Describe        :
*/
int inECC_Batch_Update(void)
{
	char		szECCBatch[8 + 1] = {0};
	char		szTicket_NeedNewBatch[2 + 1] = {0};
        
	memset(szTicket_NeedNewBatch, 0x00, sizeof(szTicket_NeedNewBatch));
	inGetTicket_NeedNewBatch(szTicket_NeedNewBatch);
        /* 檢查是否要更新批次 */
        if (memcmp(szTicket_NeedNewBatch, "Y", strlen("Y")) == 0)
	{
		memset(szECCBatch, 0x00, sizeof(szECCBatch));
		inECC_Batch_Check(szECCBatch);
		inSetTicket_Batch(szECCBatch);
		inSetTicket_NeedNewBatch("N");
		inSaveTDTRec(_TDT_INDEX_01_ECC_);
        }
	else
	{
		/* 不更新 */
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inECC_ParseXML_In_fs_data
Date&Time       :2018/4/3 下午 2:35
Describe        :
*/
int inECC_ParseXML_In_fs_data(char* szFileName, xmlDocPtr *srDoc)
{
	int	inRetVal = VS_ERROR;
	char	szFileName_New[50 + 1] = {0};
	
	sprintf(szFileName_New, "./fs_data/%s", szFileName);
	inRetVal = inXML_ParseFile(szFileName_New, srDoc);
	
	return (inRetVal);
}

/*
Function        :inECC_ParseXML_In_ICERData
Date&Time       :2018/4/23 下午 4:45
Describe        :
*/
int inECC_ParseXML_In_ICERData(char* szFileName, xmlDocPtr *srDoc)
{
	int	inRetVal = VS_ERROR;
	char	szFileName_New[50 + 1] = {0};
	char	szDebugMsg[100 + 1] = {0};
	
	sprintf(szFileName_New, "%s%s", _ECC_FOLDER_PATH_, szFileName);
	inRetVal = inXML_ReadFile(szFileName_New, srDoc, NULL, XML_PARSE_RECOVER | XML_PARSE_PEDANTIC);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Parse Failed, %s", szFileName);
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	return (inRetVal);
}

/*
Function        :inECC_Delete_Log
Date&Time       :2018/7/3 上午 9:37
Describe        :inRemainSize(用Byte計)
 *		 uszCanOverBit:0:不能超過 1:可以超過
*/
int inECC_Delete_Log(int inRemainSize, unsigned char uszCanOverBit)
{
	int		inFd = 0;				/* 檔案列表使用handle */
	int		inReadTempCnt = 0;			/* 單次讀取長度 */
	int		inFileSize = 0;				/* 檔案列表長度 */
	int		inTempFileSize = 0;			/* 個別檔案長度 */
	int		inCurrentCnt = 0;			/* 現已讀取長度 */
	int		inTotalSize = 0;			/* 加總長度 */
	int		inMaxSize = 0;				/* 最大長度 */
	int		inActionMode = 1;			/* 1:計算長度累計 2:刪除該檔案 */
	char		szReadBuffer[100 + 1] = {0};		/* 檔名buffer */
	char		szReadTempBuffer[100 + 1] = {0};	/* 讀取用buffer */
	char		szPath[200 + 1] = {0};			/* 路徑用buffer */
	char		szShellCommand[100 + 1] = {0};
	
	/* 取得檔案列表(照檔案修改時間排序，時間由近到久) */
	/* ls ICER*.log 回傳不含路徑 */
	/* ls 路徑/ 回傳不含路徑 */
	/* ls 路徑/ICER*.log 回傳含路徑 */
	memset(szShellCommand, 0x00, sizeof(szShellCommand));
	sprintf(szShellCommand, "ls %sICER*.log  -t", _ECC_ICER_LOG_FOLDER_PATH_);
	inFunc_ShellCommand_Popen(szShellCommand, VS_TRUE);
	inFILE_Delete((unsigned char*)_SHELL_RESPONSE_);
	inFILE_Copy_File((unsigned char*)_SHELL_RESPONSE_TEMP_, (unsigned char*)_SHELL_RESPONSE_);
	
	memset(szPath, 0x00, sizeof(szPath));
	sprintf(szPath, "%s%s", _FS_DATA_PATH_, _SHELL_RESPONSE_);
	inFile_Linux_Get_FileSize_By_Stat(szPath, &inFileSize);
	
	inFile_Linux_Open_In_Fs_Data(&inFd, _SHELL_RESPONSE_);
	inFile_Linux_Seek(inFd, 0, _SEEK_BEGIN_);
	
	/* 5MB(5242880 Byte) */
	inMaxSize = inRemainSize;
	
	do
	{
		/* 取得檔名 */
		memset(szReadBuffer, 0x00, sizeof(szReadBuffer));
		do
		{
			memset(szReadTempBuffer, 0x00, sizeof(szReadTempBuffer));
			inReadTempCnt = 1;
			inFile_Linux_Read(inFd, szReadTempBuffer, &inReadTempCnt);
			inCurrentCnt++;

			/* 行的尾部或是檔案的尾部 */
			if (szReadTempBuffer[0] == 0x0A	||
			    szReadTempBuffer[0] == 0x00)
			{
				break;
			}
			else
			{
				strcat(szReadBuffer, szReadTempBuffer);
			}
			
		} while (inCurrentCnt < inFileSize);
		
		/* 達到檔案尾部 */
		if (szReadTempBuffer[0] == 0x00)
		{
			break;
		}

		/* 加總長度 */
		if (inActionMode == 1)
		{
			/* 取得檔案長度 */
			inTempFileSize = 0;
			memset(szPath, 0x00, sizeof(szPath));
			sprintf(szPath, "%s", szReadBuffer);
			inFile_Linux_Get_FileSize_By_Stat(szPath, &inTempFileSize);
			inTotalSize += inTempFileSize;

			/* 達到5MB，刪除剩餘檔案 */
			if (inTotalSize >= inMaxSize)
			{
				/* 超過的那個就要砍 */
				if (uszCanOverBit == 0)
				{
					/* 大小已達5MB，開始刪除檔案 */
					inFunc_Data_Delete("", szReadBuffer, "");
				}
				
				/* 改變行為模式為刪除檔案 */
				inActionMode = 2;
			}
		}
		else
		{
			/* 大小已達5MB，開始刪除檔案 */
			inFunc_Data_Delete("", szReadBuffer, "");
		}
	}while (1);
	
	inFile_Linux_Close(inFd);
	
	return (VS_SUCCESS);
}

/*
Function        :inECC_Process_Temp
Date&Time       :2018/7/13 下午 2:20
Describe        :
*/
int inECC_Process_Temp(void)
{
	int	i = 0;
	int	inRetVal = VS_ERROR;
	int	inFd = 0;
	int	inFd2 = 0;
	int	inFileSize = 0;
	int	inStartOffset = 0;
	int	inWriteLen = 0;
	int	inLine = 0;
	char	szPath[50 + 1] = {0};
	char*	szTemp;
	
	/* 先刪除備份檔 */
	inFile_Unlink_File(_ECC_API_REQ_TEMP_BAK_, _ECC_FOLDER_PATH_);
	
	/* 開檔 */
	memset(szPath, 0x00, sizeof(szPath));
	sprintf(szPath, "%s%s", _ECC_FOLDER_PATH_, _ECC_API_REQ_TEMP_);
	
	/* 取得檔案總大小 */
	inRetVal = inFile_Linux_Get_FileSize_By_Stat(szPath, &inFileSize);
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	inRetVal = inFile_Linux_Open(&inFd, szPath);
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	/* 讀取檔案 */
	szTemp = malloc(inFileSize + 1);
	memset(szTemp, 0x00, inFileSize + 1);
	inFile_Linux_Seek(inFd, 0, _SEEK_BEGIN_);
	inRetVal = inFile_Linux_Read(inFd, szTemp, &inFileSize);
	if (inRetVal != VS_SUCCESS)
	{
		free(szTemp);
		inFile_Linux_Close(inFd);
		return (VS_ERROR);
	}
	
	/* 找到起始offset */
	inLine = 0;
	for (i = 0; i <= inFileSize; i++)      /* "<=" 是為了抓到最後一個0x00 */
        {
                /* 讀完一筆record  */
                if (szTemp[i] == 0x0A)
                {
                        inLine++;
                        /* 為了跳過 0x0D 0x0A */
                        i += 1;
                }
		
		/* 跳過前兩行 */
		if (inLine == 2)
		{
			inStartOffset = i;
			break;
		}
        }
	
	fdatasync(inFd);
	/* 關檔 */
	inFile_Linux_Close(inFd);
	
	/* 存到備份檔 */
	memset(szPath, 0x00, sizeof(szPath));
	sprintf(szPath, "%s%s", _ECC_FOLDER_PATH_, _ECC_API_REQ_TEMP_BAK_);
	inRetVal = inFile_Linux_Create(&inFd2, szPath);
	if (inRetVal != VS_SUCCESS)
	{
		free(szTemp);
		return (VS_ERROR);
	}
	
	inWriteLen = inFileSize - inStartOffset;
	inFile_Linux_Write(inFd2, &szTemp[inStartOffset], &inWriteLen);
	if (inRetVal != VS_SUCCESS)
	{
		free(szTemp);
		inFile_Linux_Close(inFd2);
		return (VS_ERROR);
	}
	
	free(szTemp);
	fdatasync(inFd2);
	inFile_Linux_Close(inFd2);
	
	/* 刪除原檔 */
	inFile_Unlink_File(_ECC_API_REQ_TEMP_, _ECC_FOLDER_PATH_);
	
	/* 改名 */
	inFunc_Data_Rename(_ECC_API_REQ_TEMP_BAK_, _ECC_FOLDER_PATH_, _ECC_API_REQ_TEMP_, _ECC_FOLDER_PATH_);
	
	return (VS_SUCCESS);
}

/*
Function        :inECC_Insert_Temp
Date&Time       :2018/7/13 下午 2:20
Describe        :
*/
int inECC_Insert_Temp(void)
{
	int	inRetVal = VS_ERROR;
	int	inFd = 0;
	int	inFd2 = 0;
	int	inFileSize = 0;
	int	inOffset = 0;
	int	inWriteLen = 0;
	char	szPath[50 + 1] = {0};
	char*	szTemp;
	
	/* 先讀出檔案 */
	memset(szPath, 0x00, sizeof(szPath));
	sprintf(szPath, "%s%s", _ECC_FOLDER_PATH_, _ECC_API_REQ_TEMP_);
	
	/* 取得檔案總大小 */
	inRetVal = inFile_Linux_Get_FileSize_By_Stat(szPath, &inFileSize);
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	inRetVal = inFile_Linux_Open(&inFd, szPath);
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	/* 讀取檔案 */
	szTemp = malloc(inFileSize + 1);
	memset(szTemp, 0x00, inFileSize + 1);
	inFile_Linux_Seek(inFd, 0, _SEEK_BEGIN_);
	inRetVal = inFile_Linux_Read(inFd, szTemp, &inFileSize);
	if (inRetVal != VS_SUCCESS)
	{
		free(szTemp);
		inFile_Linux_Close(inFd);
		return (VS_ERROR);
	}
	/* 關閉暫存檔案 */
	inFile_Linux_Close(inFd);
	
	
	
	/* 開要塞入的檔案 */
	memset(szPath, 0x00, sizeof(szPath));
	sprintf(szPath, "%s%s", _ECC_FOLDER_PATH_, _ECC_API_REQ_FILE2_);
	inRetVal = inFile_Linux_Open(&inFd2, szPath);
	if (inRetVal != VS_SUCCESS)
	{
		free(szTemp);
		return (VS_ERROR);
	}
	
	inOffset = 0 - (strlen("</TransXML>") + 1);
	/* 找到最後一行位置 */
	inRetVal = inFile_Linux_Seek(inFd2, inOffset, _SEEK_END_);
	if (inRetVal != VS_SUCCESS)
	{
		free(szTemp);
		inFile_Linux_Close(inFd2);
		return (VS_ERROR);
	}
	
	/* 塞進去 */
	inWriteLen = inFileSize;
	inRetVal = inFile_Linux_Write(inFd2, szTemp, &inWriteLen);
	if (inRetVal != VS_SUCCESS)
	{
		free(szTemp);
		inFile_Linux_Close(inFd2);
		return (VS_ERROR);
	}
	
	/* 關閉檔案 */
	free(szTemp);
	inFile_Linux_Close(inFd2);
	
	return (VS_SUCCESS);
}

/*
Function        :inECC_Settle_Other_REV_ADV
Date&Time       :2018/11/22 下午 6:12
Describe        :
*/
int inECC_Settle_Other_REV_ADV(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;
	char 	szSendReversalBit[2 + 1] = {0};
	
	/* 處理Reversal */
	/* IPASS */
	pobTran->srTRec.inTDTIndex = _TDT_INDEX_00_IPASS_;
	inLoadTDTRec(pobTran->srTRec.inTDTIndex);

	memset(szSendReversalBit, 0x00, sizeof(szSendReversalBit));
	inGetTicket_ReversalBit(szSendReversalBit);

	if (!memcmp(szSendReversalBit, "Y", 1))
	{
		if ((inRetVal = inNCCC_TICKET_ReversalSendRecvPacket(pobTran)) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}

		if (inNCCC_TICKET_SetReversalCnt(pobTran, _ADD_) == VS_ERROR)
		{
			return (VS_ERROR);
		}
	}
		
	/* ICASH */
	pobTran->srTRec.inTDTIndex = _TDT_INDEX_02_ICASH_;
	inLoadTDTRec(pobTran->srTRec.inTDTIndex);

	memset(szSendReversalBit, 0x00, sizeof(szSendReversalBit));
	inGetTicket_ReversalBit(szSendReversalBit);

	if (!memcmp(szSendReversalBit, "Y", 1))
	{
		if ((inRetVal = inNCCC_TICKET_ReversalSendRecvPacket(pobTran)) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}

		if (inNCCC_TICKET_SetReversalCnt(pobTran, _ADD_) == VS_ERROR)
		{
			return (VS_ERROR);
		}
	}
		
	/* 處理advice */
	/* IPASS */
	pobTran->srTRec.inTDTIndex = _TDT_INDEX_00_IPASS_;
	inLoadTDTRec(pobTran->srTRec.inTDTIndex);
	
	if ((inRetVal = inNCCC_TICKET_ProcessAdvice(pobTran)) != VS_SUCCESS)
	{
		/* 只有結帳時前送advice錯誤時要顯示錯誤訊息，在inNCCC_TICKET_ISOAdviceAnalyse裡顯示錯誤訊息 */
		pobTran->inErrorMsg = _ERROR_CODE_V3_SETTLE_NOT_SUCCESS_;
		return (inRetVal);
	}
	
	/* ICASH */
	pobTran->srTRec.inTDTIndex = _TDT_INDEX_02_ICASH_;
	inLoadTDTRec(pobTran->srTRec.inTDTIndex);
	
	if ((inRetVal = inNCCC_TICKET_ProcessAdvice(pobTran)) != VS_SUCCESS)
	{
		/* 只有結帳時前送advice錯誤時要顯示錯誤訊息，在inNCCC_TICKET_ISOAdviceAnalyse裡顯示錯誤訊息 */
		pobTran->inErrorMsg = _ERROR_CODE_V3_SETTLE_NOT_SUCCESS_;
		return (inRetVal);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inECC_PACK_RETRY_ICERAPI
Date&Time       :2018/12/13 下午 8:40
Describe        :
*/
int inECC_PACK_RETRY_ICERAPI(TRANSACTION_OBJECT *pobTran)
{
	int		inHDTIndex = -1;
	int		inRetVal = VS_ERROR;
	char		szTemplate[50 + 1] = {0};
	char		szTagData[1536 + 1] = {0};
	char		szHDTIndex[2 + 1] = {0};
	char		szDebugMsg[100 + 1] = {0};
	xmlChar*	szTagPtr = NULL;
	xmlDocPtr	srDoc = NULL;
	xmlNodePtr	srCur = NULL;		/* (第二層) */
	xmlNodePtr	srTagNode = NULL;	/* 用來移動Tag那一層(第三層) */
	xmlNodePtr	srTextNode = NULL;	/* 因為ElementNode的子節點TextNode才有內容(第四層) */
	RTC_NEXSYS	srRTC = {};

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inECC_PACK_RETRY_ICERAPI() START !");
	}
	
	if (inLoadTDTRec(_TDT_INDEX_01_ECC_) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

        /* MFT index 紀錄 HDT index */
	memset(szHDTIndex, 0x00, sizeof(szHDTIndex));
	inGetTicket_HostIndex(szHDTIndex);
	inHDTIndex = atoi(szHDTIndex);
        if (inLoadHDTRec(inHDTIndex) != VS_SUCCESS)
	{
                return (VS_ERROR);
	}

        if (inLoadECCDTRec(0) < 0)
		return (VS_ERROR);

        /* 刪除原本的ICERAPI */
        /* 檢核用 Retry不能刪除 _ECC_API_REQ_CHECK_*/

	inFile_Unlink_File(_ECC_API_REQ_FILE_, _ECC_FOLDER_PATH_);
	inFile_Unlink_File(_ECC_API_REQ_FILE2_, _ECC_FOLDER_PATH_);
	inFile_Unlink_File(_ECC_API_RES_FILE_, _ECC_FOLDER_PATH_);
	/* Retry時需要參考RES2，不能砍 */
	if (guszECCRetryBit == VS_TRUE)
	{
		
	}
	else
	{
		inFile_Unlink_File(_ECC_API_RES_FILE2_, _ECC_FOLDER_PATH_);
	}

	/* 先抓EDC日期時間 */
	memset(&srRTC, 0x00, sizeof(RTC_NEXSYS));
	inFunc_GetSystemDateAndTime(&srRTC);

        memset(pobTran->srTRec.szDate, 0x00, sizeof(pobTran->srTRec.szDate));
        memset(pobTran->srTRec.szTime, 0x00, sizeof(pobTran->srTRec.szTime));
	sprintf(pobTran->srTRec.szDate, "%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
	sprintf(pobTran->srTRec.szTime, "%02d%02d%02d", srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);

	inRetVal = inECC_ParseXML_In_ICERData(_ECC_API_REQ_RETRY_FILE_, &srDoc);
	/* 異常 */
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	inRetVal = inXML_Get_RootElement(&srDoc, &srCur);
	/* 異常 */
	if (inRetVal != VS_SUCCESS)
	{
		inXML_End(&srDoc);
		
		return (VS_ERROR);
	}
		
	/* 先走到TransXML的下一層 */
	while (srCur->children != NULL)
	{
		srCur = srCur->children;
		if (memcmp(srCur->parent->name , "TransXML", strlen("TransXML")) == 0)
		{
			break;
		}
	}
	
	/* 換下一個節點檢查，往下走到TRANS的子節點，接著開始往橫走 */
	while (srCur != NULL)
	{
		/* 每次都設為NULL，如果有要查看的話才設定位置 */
		srTagNode = NULL;
		if (memcmp(srCur->name, "TRANS", strlen("TRANS")) == 0	&&
	            srCur->children != NULL)
		{
			srTagNode = srCur ->children;
		}
		
		/* 推移第三層 */
		while (srTagNode != NULL)
		{
			/* 有分XML_TEXT_NODE和XML_ELEMENT_NODE 才有content可抓 */
			if (srTagNode->type == XML_ELEMENT_NODE	&&
			   (srTagNode->children != NULL		&&
			    srTagNode->children->type == XML_TEXT_NODE))
			{
				srTextNode = srTagNode->children;
				memset(szTagData, 0x00, sizeof(szTagData));
				szTagPtr = xmlNodeListGetString(srDoc, srTextNode, srTextNode->line);
				if (szTagPtr != NULL	&&
				    xmlStrlen(szTagPtr) != 0)
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s : %s", srTagNode->name, szTagPtr);
						inLogPrintf(AT, szDebugMsg);
					}

					strcat(szTagData, (char*)szTagPtr);
					xmlFree(szTagPtr);
				}
			}

			/* 修改時間 */
			if (xmlStrcmp(srTagNode->name, (const xmlChar*)"T1200") == 0)
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%02d%02d%02d", srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
				xmlNodeSetContent(srTagNode, BAD_CAST szTemplate);
			}
			/* 修改日期 */
			else if (xmlStrcmp(srTagNode->name, (const xmlChar*)"T1300") == 0)
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "20%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
				xmlNodeSetContent(srTagNode, BAD_CAST szTemplate);
			}
//			else if (xmlStrcmp(srTagNode->name, (const xmlChar*)"T1100") == 0)
//			{
//				memset(szSTAN, 0x00, sizeof(szSTAN));
//				inGetSTANNum(szSTAN);
//				memset(szTemplate, 0x00, sizeof(szTemplate));
//				sprintf(szTemplate, "%s", szSTAN);
//				xmlNodeSetContent(srTagNode, BAD_CAST szTemplate);
//			}
//			else if (xmlStrcmp(srTagNode->name, (const xmlChar*)"T1101") == 0)
//			{
//				memset(szSTAN, 0x00, sizeof(szSTAN));
//				inGetTicket_STAN(szSTAN);
//				memset(szTemplate, 0x00, sizeof(szTemplate));
//				sprintf(szTemplate, "%06d", atoi(szSTAN) + 1);
//				xmlNodeSetContent(srTagNode, BAD_CAST szTemplate);
//				inSetTicket_STAN(szTemplate);
//				inSaveTDTRec(_TDT_INDEX_01_ECC_);
//			}
//			else if (xmlStrcmp(srTagNode->name, (const xmlChar*)"T1102") == 0)
//			{
//				memset(szSTAN, 0x00, sizeof(szSTAN));
//				inGetSTANNum(szSTAN);
//				memset(szTemplate, 0x00, sizeof(szTemplate));
//				sprintf(szTemplate, "%s", szSTAN);
//				xmlNodeSetContent(srTagNode, BAD_CAST szTemplate);
//			}
			else if (xmlStrcmp(srTagNode->name, (const xmlChar*)"T1103") == 0)
			{
				xmlNodeSetContent(srTagNode, BAD_CAST gszECCRetryData.szT1103);
			}

			srTagNode = srTagNode->next;
		}
		
		/* 推移第二層 */
		srCur = srCur->next;
	}
	
	/* 儲存xml檔 xml檔現在會自動換行了 */
        inXML_SaveFile(_ECC_API_REQ_FILE_, &srDoc, "utf-8", 1);
	inFile_Move_File(_ECC_API_REQ_FILE_, _AP_ROOT_PATH_, "", _ECC_FOLDER_PATH_);
	
	/* 清空佔用記憶體 */
	inXML_End(&srDoc);

	/* 移除RETRY備份的REQ檔 */
	inFile_Unlink_File(_ECC_API_REQ_RETRY_FILE_, _ECC_FOLDER_PATH_);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inECC_PACK_RETRY_ICERAPI() END !");
		inLogPrintf(AT, "----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inECC_Set_Update_Batch_Flag
Date&Time       :2019/2/26 下午 4:24
Describe        :將悠遊卡更新批次的Flag放到和NCCC批次同樣的地方，以求印單時批號邏輯一致
*/
int inECC_Set_Update_Batch_Flag(TRANSACTION_OBJECT *pobTran)
{
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inECC_Set_Update_Batch_Flag() START !");
	}
	
	/*
	 * T5501批次號碼之規則為yymmddss+流水號兩碼
	 * 以當批第一筆成功交易時間為準
	 * 如果NeedNewBatch是'Y'，代表是第一筆，更新狀態為不用更新批號
	 * 如果NeedNewBatch是'N'，代表有做過交易
	*/
	inLoadTDTRec(_TDT_INDEX_01_ECC_);
	/* 結帳，所以要更新批號 */
	inSetTicket_NeedNewBatch("Y");
	inSaveTDTRec(_TDT_INDEX_01_ECC_);
	
	/* 標記為已做過 */
        if (pobTran->inRunTRTID == _TRT_SETTLE_ ||
	    pobTran->inRunTRTID == _TRT_TICKET_ECC_SETTLE_)
        {
            inNCCC_Func_Settlement_XML_Edit(_SETTLENMENT_RECOVER_XML_TAG_TICKET_ECC_SET_UPDATE_BATCH_FLAG_, "Y");
        }
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inECC_Set_Update_Batch_Flag() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inECC_Check_ICER_LOG_SIZE
Date&Time       :2020/12/18 下午 4:58
Describe        :
*/
int inECC_Check_ICER_LOG_SIZE(void)
{
	int	inFd = 0;				/* 檔案列表使用handle */
	int	inReadTempCnt = 0;			/* 單次讀取長度 */
	int	inFileSize = 0;				/* 檔案列表長度 */
	int	inCurrentCnt = 0;			/* 現已讀取長度 */
	char	szDispMsg[200 + 1] = {0};
	char	szReadBuffer[100 + 1] = {0};		/* 檔名buffer */
	char	szReadTempBuffer[100 + 1] = {0};	/* 讀取用buffer */
	char	szShellCommand[100 + 1] = {0};
	char	szPath[200 + 1] = {0};			/* 路徑用buffer */
	
	/* 用awk算大小一定要加上-l(顯示詳細資料) */
	memset(szShellCommand, 0x00, sizeof(szShellCommand));
	sprintf(szShellCommand, "ls -l %sICER*.log | awk '{ SUM += $5 } END { print SUM/1024/1024 }'", _ECC_ICER_LOG_FOLDER_PATH_);
	inFunc_ShellCommand_Popen(szShellCommand, VS_TRUE);
	inFILE_Delete((unsigned char*)_SHELL_RESPONSE_);
	inFILE_Copy_File((unsigned char*)_SHELL_RESPONSE_TEMP_, (unsigned char*)_SHELL_RESPONSE_);
	memset(szPath, 0x00, sizeof(szPath));
	sprintf(szPath, "%s%s", _FS_DATA_PATH_, _SHELL_RESPONSE_);
	inFile_Linux_Get_FileSize_By_Stat(szPath, &inFileSize);
	inFile_Linux_Open_In_Fs_Data(&inFd, _SHELL_RESPONSE_);
	inFile_Linux_Seek(inFd, 0, _SEEK_BEGIN_);
	do
	{
		/* 取得檔名 */
		memset(szReadBuffer, 0x00, sizeof(szReadBuffer));
		do
		{
			memset(szReadTempBuffer, 0x00, sizeof(szReadTempBuffer));
			inReadTempCnt = 1;
			inFile_Linux_Read(inFd, szReadTempBuffer, &inReadTempCnt);
			inCurrentCnt++;

			/* 行的尾部或是檔案的尾部 */
			if (szReadTempBuffer[0] == 0x0A	||
			    szReadTempBuffer[0] == 0x00)
			{
				break;
			}
			else
			{
				strcat(szReadBuffer, szReadTempBuffer);
			}
			
		} while (inCurrentCnt < inFileSize);
		
		break;
	}while (1);
	inFile_Linux_Close(inFd);
	
	inDISP_ClearAll();
	memset(szDispMsg, 0x00, sizeof(szDispMsg));
	sprintf(szDispMsg, "%s Size: %s MB", _ECC_ICER_LOG_FOLDER_NAME_, szReadBuffer);
	inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_4_, _DISP_CENTER_);
	inDISP_Wait(5000);

	return (VS_SUCCESS);
}

/*
Function        :inECC_HAL_SetDebug
Date&Time       :2021/11/18 下午 3:15
Describe        :
*/
int inECC_HAL_SetDebug(void)
{
#ifdef _NEW_ECC_NEXSYS_
	int		inRetVal;
	int		inFd = 0;
	char		szPath[100 + 1] = {0};
        char		szTemplate[64 + 1];
        DISPLAY_OBJECT  srDispObj;
        
        /* Debug */
	inDISP_Clear_Line(_LINE_8_1_, _LINE_8_8_);
        memset(szTemplate, 0x00, sizeof(szTemplate));
        
	strcpy(szTemplate, "ECC HAL Debug 開關");
	memset(szPath, 0x00, sizeof(szPath));
	sprintf(szPath, "%s%s", _ECC_FOLDER_PATH_, _ECC_DEBUG_SWITCH_FILE_NAME_);
	
        inDISP_ChineseFont(szTemplate, _FONTSIZE_8X22_, _LINE_8_3_, _DISP_LEFT_);
	if (inFile_Linux_Open(&inFd, szPath) == VS_SUCCESS)
	{
		inDISP_ChineseFont("Now: On", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
	}
	else
	{
		inDISP_ChineseFont("Now: Off", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
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
				inFile_Unlink_File(_ECC_DEBUG_SWITCH_FILE_NAME_, _ECC_FOLDER_PATH_);
				ECC_Debug_Off_Lib();
                                break;
                        }
                        else if (srDispObj.szOutput[0] == '1')
                        {
				inFile_Linux_Create(&inFd, szPath);
				inFunc_Data_Chmod("u+rwx", _ECC_DEBUG_SWITCH_FILE_NAME_, _ECC_FOLDER_PATH_);
                                ECC_Debug_On_Lib();
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
#endif
	
	return (VS_SUCCESS);
}
