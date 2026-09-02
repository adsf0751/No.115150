#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <ctosapi.h>
#include <curl/curl.h>
#include <sqlite3.h>
#include <libxml/xpath.h>
#include "../SOURCE/INCLUDES/Define_1.h"
#include "../SOURCE/INCLUDES/Define_2.h"
#include "../SOURCE/INCLUDES/TransType.h"
#include "../SOURCE/INCLUDES/Transaction.h"
#include "../SOURCE/INCLUDES/AllStruct.h"
#include "../SOURCE/DISPLAY/Display.h"
#include "../SOURCE/DISPLAY/DispMsg.h"
#include "../SOURCE/DISPLAY/DisTouch.h"
#include "../SOURCE/PRINT/Print.h"
#include "../SOURCE/COMM/Comm.h"
#include "../SOURCE/FUNCTION/HDT.h"
#include "../SOURCE/FUNCTION/HDPT.h"
#include "../SOURCE/FUNCTION/Function.h"
#include "../SOURCE/FUNCTION/FuncTable.h"
#include "../SOURCE/FUNCTION/File.h"
#include "../SOURCE/FUNCTION/XML.h"
#include "../SOURCE/FUNCTION/CPT.h"
#include "../SOURCE/EVENT/MenuMsg.h"
#include "../SOURCE/EVENT/Flow.h"
#include "TAKAsrc.h"

extern	int	ginTAKAHostIndex;
extern	int	ginTAKACommIndex;
extern  int	ginDebug;		/* Debug使用 extern */
extern  int	ginISODebug;		/* Debug使用 extern */
extern	int	ginFindRunTime;

int SALE_TRT_TAKA_TABLE[] =
{
	_TAKA_FUNCTION_CHECK_TRANS_FUNCTION_FLOW_,
        _TAKA_FUNCTION_MUST_SETTLE_CHECK_,
	_FUNCTION_DUPLICATE_CHECK_,
	_COMM_MODEM_PREDIAL_,
        _CREDIT_FUNCTION_GET_AMOUNT_,
        _CREDIT_FUNCTION_GET_STOREID_,
	_CREDIT_FUNCTION_GET_PRODUCT_CODE_,
	_NCCC_FUNCTION_MAKE_REFNO_,
	_NCCC_FUNCTION_CHECK_SPECIALCARD_FLOW_,
	_NCCC_FUNCTION_GET_PAY_ITEM_,			/* 若判定是DCC流程，就不跑 */
	_NCCC_FUNCTION_SET_TXN_ONLINE_OFFLINE_FLOW_,
        _NCCC_FUNCTION_BUILD_AND_SEND_PACKET_FLOW_,
	_NCCC_FUNCTION_GET_TRANSACTION_NO_FROM_PAN_,
	_UPDATE_ACCUM_,
        _UPDATE_BATCH_,
	_FUNCTION_DUPLICATE_SAVE_,
        _FUNCTION_UPDATE_INV_,
	_FUNCTION_ECR_SEND_TRANSACTION_RESULT_,
        _CREDIT_FUNCTION_CHECKRESULT_,
        _FUNCTION_PRINT_RECEIPT_BY_BUFFER_FLOW_,
	_NCCC_FUNCTION_SEND_ECR_AFTER_PRINT_RECEIPT_,
	_NCCC_FUNCTION_RECEIVE_EI_FLOW_,
        _FLOW_NULL_,
};

int VOID_TRT_TAKA_TABLE[] =
{
	_TAKA_FUNCTION_CHECK_TRANS_FUNCTION_FLOW_,
        _TAKA_FUNCTION_MUST_SETTLE_CHECK_,
	_COMM_MODEM_PREDIAL_,
        _NCCC_FUNCTION_VOID_CHECK_,
        _NCCC_FUNCTION_VOID_CONFIRM_,
	_NCCC_FUNCTION_CHECK_SPECIALCARD_FLOW_,
        _HG_VOID_RUN_MULTI_FLOW_,
	_NCCC_FUNCTION_GET_TRANSACTION_NO_FROM_PAN_,
        /*
	_NCCC_FUNCTION_SET_TXN_ONLINE_OFFLINE_FLOW_,
        _NCCC_FUNCTION_BUILD_AND_SEND_PACKET_FLOW_,
        _UPDATE_ACCUM_,
        _UPDATE_BATCH_,
        */
	_FUNCTION_ECR_SEND_TRANSACTION_RESULT_,
        _CREDIT_FUNCTION_CHECKRESULT_,
        _FUNCTION_PRINT_RECEIPT_BY_BUFFER_FLOW_,
	_NCCC_FUNCTION_SEND_ECR_AFTER_PRINT_RECEIPT_,
	_NCCC_FUNCTION_RECEIVE_EI_FLOW_,
	_HG_FUNCTION_DISPLAY_COMPLETE_,
	_FLOW_NULL_,
};

int REFUND_TRT_TAKA_TABLE[] =
{
	_TAKA_FUNCTION_CHECK_TRANS_FUNCTION_FLOW_,
        _TAKA_FUNCTION_MUST_SETTLE_CHECK_,
	_COMM_MODEM_PREDIAL_,
        _CREDIT_FUNCTION_GET_REFUND_AMOUNT_,
	_CREDIT_FUNCTION_GET_AUTHCODE_,
        _CREDIT_FUNCTION_GET_STOREID_,
	_CREDIT_FUNCTION_GET_PRODUCT_CODE_,
	_NCCC_FUNCTION_MAKE_REFNO_,
	_NCCC_FUNCTION_CHECK_SPECIALCARD_FLOW_,
	_NCCC_FUNCTION_GET_PAY_ITEM_,			/* 若判定是DCC流程，就不跑 */
	_NCCC_FUNCTION_SET_TXN_ONLINE_OFFLINE_FLOW_,
        _NCCC_FUNCTION_BUILD_AND_SEND_PACKET_FLOW_,
	_NCCC_FUNCTION_GET_TRANSACTION_NO_FROM_PAN_,
        _UPDATE_ACCUM_,
        _UPDATE_BATCH_,
        _FUNCTION_UPDATE_INV_,
	_FUNCTION_ECR_SEND_TRANSACTION_RESULT_,
        _CREDIT_FUNCTION_CHECKRESULT_,
        _FUNCTION_PRINT_RECEIPT_BY_BUFFER_FLOW_,
	_NCCC_FUNCTION_SEND_ECR_AFTER_PRINT_RECEIPT_,
	_NCCC_FUNCTION_RECEIVE_EI_FLOW_,
	_FLOW_NULL_,
};

int SETTLE_TRT_TAKA_TABLE[] =
{
	_TAKA_FUNCTION_CHECK_TRANS_FUNCTION_FLOW_,
        _FUNCTION_PREPRINT_AUTO_REPORT_BY_BUFFER_,
	_NCCC_FUNCTION_SET_TXN_ONLINE_OFFLINE_FLOW_,
        _NCCC_FUNCTION_BUILD_AND_SEND_PACKET_FLOW_,
	_NCCC_FUNCTION_SETTLEMENT_XML_NCCC_START_,
        _FUNCTION_DELETE_BATCH_FLOW_,
        _FUNCTION_PRINT_TOTAL_REPORT_BY_BUFFER_,
        _FUNCTION_DELETE_ACCUM_FLOW_,
        _FUNCTION_REST_BATCH_INV_,
	_FUNCTION_UPDATE_BATCH_NUM_,
	_NCCC_FUNCTION_SETTLEMENT_XML_NCCC_END_,
        _FLOW_NULL_,
};

TRT_TABLE TRANSACTION_TAKA_TABLE[] =
{
        {_TRT_SALE_,				SALE_TRT_TAKA_TABLE},
        {_TRT_VOID_,				VOID_TRT_TAKA_TABLE},
        {_TRT_REFUND_,				REFUND_TRT_TAKA_TABLE},
        {_TRT_SETTLE_,				SETTLE_TRT_TAKA_TABLE},
        {_FLOW_NULL_,				NULL}
};

/*
Function        :inNCCC_RunTRT
Date&Time       :2016/9/19 下午 3:44
Describe        :
*/
int inTAKA_Func_RunTRT(TRANSACTION_OBJECT *pobTran, int inTRTCode)
{
        int     *inTRTID = NULL;
        int     i, inRetVal = VS_ERROR ;
	
        for (i = 0 ;; i ++)
        {
		if (TRANSACTION_TAKA_TABLE[i].inTRTCode == inTRTCode)
		{
			inTRTID = TRANSACTION_TAKA_TABLE[i].inTRTID;
			break;
		}
		else if (TRANSACTION_TAKA_TABLE[i].inTRTCode == -1)
		{
			break;
		}
        }

        if (inTRTID == NULL)
        {
                return (VS_ERROR);
        }

        for (i = 0 ; ; i ++)
        {
                if (inTRTID[i] == VS_ERROR)
                        break;

                inRetVal = inFLOW_RunFunction(pobTran, inTRTID[i]);

                if (inRetVal != VS_SUCCESS)
                {
                        break;
                }
        }

	/* 斷線 */
	inCOMM_End(pobTran);

        return (inRetVal);
}

/*
Function        :inTAKA_Func_Check_Transaction_Function
Date&Time       :2022/10/14 下午 1:58
Describe        :確認交易功能是否打開，
*/
int inTAKA_Func_Check_Transaction_Function(int inCode)
{
	char		szTransFunc[20 + 1] = {0};
	unsigned char	uszTxnEnable = VS_TRUE;
	
	inTAKA_Func_SwitchToTAKA_Host(0);
	
	memset(szTransFunc, 0x00, sizeof(szTransFunc));
	if (inGetTransFunc(szTransFunc) != VS_SUCCESS)
		return (VS_ERROR);
	
	if (inCode == _SALE_)
	{
		if (memcmp(&szTransFunc[0], "Y", 1) != 0)
		{
			uszTxnEnable = VS_FALSE;
		}
	}
	else if (inCode == _VOID_)
	{
		if (memcmp(&szTransFunc[1], "Y", 1) != 0)
		{
			uszTxnEnable = VS_FALSE;
		}
	}
	else if (inCode == _SETTLE_)
	{
		if (memcmp(&szTransFunc[2], "Y", 1) != 0)
		{
			uszTxnEnable = VS_FALSE;
		}
	}
	else if (inCode == _REFUND_)
	{
		if (memcmp(&szTransFunc[3], "Y", 1) != 0)
		{
			uszTxnEnable = VS_FALSE;
		}
	}
	
	/* 此功能已關閉 */
	if (uszTxnEnable == VS_FALSE)
	{
		return (VS_ERROR);
	}
	else
	{
		return (VS_SUCCESS);
	}
}

/*
Function        :inTAKA_Func_SwitchToTAKA_Host
Date&Time       :2022/10/14 下午 1:57
Describe        :切換到HDT中DCC的REC上，之後要回覆就靠inOrgHDTIndex（在此function中回傳錯誤也會回覆原host）
*/
int inTAKA_Func_SwitchToTAKA_Host(int inOrgHDTIndex)
{
	/* 開機後只找一次TAKA */
	if (ginTAKAHostIndex != -1)
	{
		/* 直接使用 */
	}
	else
	{
		if (inFunc_Find_Specific_HDTindex(inOrgHDTIndex, _HOST_NAME_TAKA_, &ginTAKAHostIndex) != VS_SUCCESS)
		{
			/* 找不到直接return VS_ERROR */
			return (VS_ERROR);
		}

	}
	
	/* 如果Load失敗，回復原Host並回傳VS_ERROR(理論上不會有此情況) */
	if (inLoadHDTRec(ginTAKAHostIndex) < 0)
	{
		if (inOrgHDTIndex >= 0)
		{
			/* 回覆原本的Host */
			inLoadHDTRec(inOrgHDTIndex);
		}
		else
		{
			/* load回信用卡主機 */
			inLoadHDTRec(0);
		}

		return (VS_ERROR);
	}
	else
	{
		return (VS_SUCCESS);
	}
}

/*
Function        :inTAKA_Func_Check_Transaction_Function_Flow
Date&Time       :2022/10/14 下午 2:07
Describe        :確認交易功能是否打開，因為根據Host，所以每個Host一個Function
*/
int inTAKA_Func_Check_Transaction_Function_Flow(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_SUCCESS;
	unsigned char	uszTxnEnable = VS_TRUE;

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inTAKA_Func_Check_Transaction_Function_Flow() START !");
	}
	
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
	}
	
	
	inRetVal = inTAKA_Func_Check_Transaction_Function(pobTran->inTransactionCode);
	if (inRetVal != VS_SUCCESS)
	{
		uszTxnEnable = VS_FALSE;
	}

	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inTAKA_Func_Check_Transaction_Function_Flow(%d) END !", uszTxnEnable);
		inLogPrintf(AT, "----------------------------------------");
	}
	
	/* 此功能已關閉 */
	if (uszTxnEnable == VS_FALSE)
	{
		pobTran->inErrorMsg = _ERROR_CODE_V3_FUNC_CLOSE_;
		
		return (VS_ERROR);
	}
	else
	{
		return (VS_SUCCESS);
	}
}

/*
Function        :inTAKA_Func_Must_SETTLE
Date&Time       :2022/10/14 下午 2:11
Describe        :確認是否要先結帳
*/
int inTAKA_Func_Must_SETTLE(TRANSACTION_OBJECT *pobTran)
{
        char    szMustSettleBit[2 + 1] = {0};
	
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
	}
	
	/* 開機後只找一次TAKA */
	if (ginTAKAHostIndex != -1)
	{
		/* 直接使用 */
	}
	else
	{
		if (inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_TAKA_, &ginTAKAHostIndex) != VS_SUCCESS)
		{
			/* 找不到直接return VS_ERROR */
			return (VS_ERROR);
		}

	}
	
        inLoadHDPTRec(ginTAKAHostIndex);
        memset(szMustSettleBit, 0x00, sizeof(szMustSettleBit));

        if (inGetMustSettleBit(szMustSettleBit) == VS_ERROR)
        {
                return (VS_ERROR);
        }

	/* Load完了要load回原來的HDPT避免意外，如果有的話 */
	inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
	
        if (!memcmp(szMustSettleBit, "Y", 1))
        {
                /* 表示要結帳 */
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, _ERR_MUST_SETTLE_);
		srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
		srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
		srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
		strcpy(srDispMsgObj.szErrMsg1, _HOST_NAME_TAKA_);
		srDispMsgObj.inErrMsg1Line = _LINE_8_5_;
		srDispMsgObj.inBeepTimes = 1;
		srDispMsgObj.inBeepInterval = 0;
                inDISP_Msg_BMP(&srDispMsgObj);
		
		return (VS_ERROR);
	}
        else
	{
                return (VS_SUCCESS);
	}
}

/*
Function        :inTAKA_Func_Find_Comm_Index
Date&Time       :2024/4/30 下午 2:56
Describe        :切換到HDT中TAKA的REC上，之後要回覆就靠inOrgHDTIndex（在此function中回傳錯誤也會回覆原host）
*/
int inTAKA_Func_Find_Comm_Index(int inOrgHDTIndex, int *inCommIndex)
{
	int	inRetVal = VS_SUCCESS;
	char	szCommunicationIndex[2 + 1] = {0};
	
	/* 開機後只找一次TAKA */
	if (ginTAKACommIndex != -1)
	{
		/* 直接使用 */
		*inCommIndex = ginTAKACommIndex;
		
		return (VS_SUCCESS);
	}
	else
	{
		if (inFunc_Find_Specific_HDTindex(inOrgHDTIndex, _HOST_NAME_TAKA_, &ginTAKAHostIndex) != VS_SUCCESS)
		{
			/* 找不到直接return VS_ERROR */
			return (VS_ERROR);
		}
		
		do
		{
			/* 如果Load失敗，回復原Host並回傳VS_ERROR(理論上不會有此情況) */
			if (inLoadHDTRec(ginTAKAHostIndex) < 0)
			{
				inRetVal = VS_ERROR;
				break;
			}
			
			memset(szCommunicationIndex, 0x00, sizeof(szCommunicationIndex));
			inGetCommunicationIndex(szCommunicationIndex);
			*inCommIndex = atoi(szCommunicationIndex) - 1;
			ginTAKACommIndex = *inCommIndex;
			
			break;
		}while(1);
		
		
		if (inOrgHDTIndex >= 0)
		{
			/* 回覆原本的Host */
			inLoadHDTRec(inOrgHDTIndex);
		}
		else
		{
			/* load回信用卡主機 */
			inLoadHDTRec(0);
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
}


/*
Function        :inTAKA_TAKA_Comm_Preserve
Date&Time       :2024/4/30 下午 3:26
Describe        :儲存TAKA Comm參數
*/
int inTAKA_TAKA_Comm_Preserve()
{
	int		inCommIndex = 0;
	char		szTemplate[50 + 1] = {0};
	xmlDocPtr       srDoc = NULL;
        xmlNodePtr      srRoot_Node = NULL;
	
	/* 刪除舊有xml */
	inFile_Unlink_File(_TAKA_COMM_PRESERVE_XML_FILE_NAME_, _FS_DATA_PATH_);
	
	/* 建立一個新的檔案 */
        srDoc = xmlNewDoc(BAD_CAST "1.0");
        srRoot_Node = xmlNewNode(NULL, BAD_CAST "NEXSYS");
        xmlDocSetRootElement (srDoc, srRoot_Node);
	
	inTAKA_Func_Find_Comm_Index(0, &inCommIndex);
	inLoadCPTRec(inCommIndex);
	
	/* CommIndex */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetCommIndex(szTemplate);
	xmlNewChild(srRoot_Node, NULL, BAD_CAST _TAKA_COMM_PRESERVE_TAG_CommIndex_, BAD_CAST szTemplate);
	
	/* TPDU */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTPDU(szTemplate);
	xmlNewChild(srRoot_Node, NULL, BAD_CAST _TAKA_COMM_PRESERVE_TAG_TPDU_, BAD_CAST szTemplate);
	
	/* NII */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetNII(szTemplate);
	xmlNewChild(srRoot_Node, NULL, BAD_CAST _TAKA_COMM_PRESERVE_TAG_NII_, BAD_CAST szTemplate);
	
	/* HostTelPrimary */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetHostTelPrimary(szTemplate);
	xmlNewChild(srRoot_Node, NULL, BAD_CAST _TAKA_COMM_PRESERVE_TAG_HostTelPrimary_, BAD_CAST szTemplate);
	
	/* HostTelSecond */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetHostTelSecond(szTemplate);
	xmlNewChild(srRoot_Node, NULL, BAD_CAST _TAKA_COMM_PRESERVE_TAG_HostTelSecond_, BAD_CAST szTemplate);
	
	/* ReferralTel */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetReferralTel(szTemplate);
	xmlNewChild(srRoot_Node, NULL, BAD_CAST _TAKA_COMM_PRESERVE_TAG_ReferralTel_, BAD_CAST szTemplate);
	
	/* HostIPPrimary */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetHostIPPrimary(szTemplate);
	xmlNewChild(srRoot_Node, NULL, BAD_CAST _TAKA_COMM_PRESERVE_TAG_HostIPPrimary_, BAD_CAST szTemplate);
	
	/* HostPortNoPrimary */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetHostPortNoPrimary(szTemplate);
	xmlNewChild(srRoot_Node, NULL, BAD_CAST _TAKA_COMM_PRESERVE_TAG_HostPortNoPrimary_, BAD_CAST szTemplate);
	
	/* HostIPSecond */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetHostIPSecond(szTemplate);
	xmlNewChild(srRoot_Node, NULL, BAD_CAST _TAKA_COMM_PRESERVE_TAG_HostIPSecond_, BAD_CAST szTemplate);
	
	/* HostPortNoSecond */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetHostPortNoSecond(szTemplate);
	xmlNewChild(srRoot_Node, NULL, BAD_CAST _TAKA_COMM_PRESERVE_TAG_HostPortNoSecond_, BAD_CAST szTemplate);
	
	/* TCPHeadFormat */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTCPHeadFormat(szTemplate);
	xmlNewChild(srRoot_Node, NULL, BAD_CAST _TAKA_COMM_PRESERVE_TAG_TCPHeadFormat_, BAD_CAST szTemplate);
	
	/* CarrierTimeOut */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetCarrierTimeOut(szTemplate);
	xmlNewChild(srRoot_Node, NULL, BAD_CAST _TAKA_COMM_PRESERVE_TAG_CarrierTimeOut_, BAD_CAST szTemplate);
	
	/* HostResponseTimeOut */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetHostResponseTimeOut(szTemplate);
	xmlNewChild(srRoot_Node, NULL, BAD_CAST _TAKA_COMM_PRESERVE_TAG_HostResponseTimeOut_, BAD_CAST szTemplate);
			
	/* 儲存xml檔 xml檔現在會自動換行了 */
        inXML_SaveFile(_TAKA_COMM_PRESERVE_XML_FILE_NAME_, &srDoc, "utf-8", 1);
	
	/* 清空佔用記憶體 */
	inXML_End(&srDoc);
	
	inFile_Move_File(_TAKA_COMM_PRESERVE_XML_FILE_NAME_, _AP_ROOT_PATH_, "", _FS_DATA_PATH_);
	
	return (VS_SUCCESS);
}

/*
Function        :inTAKA_TAKA_Comm_Get_In_Record
Date&Time       :2024/5/2 上午 11:19
Describe        :回復TAKA Comm參數
*/
int inTAKA_TAKA_Comm_Get_In_Record()
{
	int		inRetVal = VS_ERROR;
	char		szTagData[1536 + 1] = {0};
	char		szFileName_New[50 + 1] = {0};
	char		szDebugMsg[100 + 1] = {0};
	xmlChar*	szTagPtr = NULL;
	xmlDocPtr       srDoc = NULL;
	xmlNodePtr	srCur = NULL;		/* (第二層) */
	xmlNodePtr	srTagNode = NULL;	/* 用來移動Tag那一層(第三層) */
	xmlNodePtr	srTextNode = NULL;	/* 因為ElementNode的子節點TextNode才有內容(第四層) */
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inTAKA_TAKA_Comm_Get_In_Record() START !");
	}
	
	if (inFILE_Check_Exist((unsigned char*)_TAKA_COMM_PRESERVE_XML_FILE_NAME_) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inFunc_UpdateAP_Recover file not exist");
		}
		return (VS_ERROR);
	}
	
	sprintf(szFileName_New, "%s%s", _FS_DATA_PATH_, _TAKA_COMM_PRESERVE_XML_FILE_NAME_);
	inRetVal = inXML_ReadFile(szFileName_New, &srDoc, NULL, XML_PARSE_RECOVER | XML_PARSE_PEDANTIC);
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
	
	/* 換下一個節點檢查，往下走到TRANS的子節點，接著開始往橫走 */
	while (srCur != NULL)
	{
		/* 每次都設為NULL，如果有要查看的話才設定位置 */
		srTagNode = NULL;
		if (memcmp(srCur->name, _TAKA_COMM_PRESERVE_TAG_NEXSYS_, _TAKA_COMM_PRESERVE_TAG_NEXSYS_LEN_) == 0	&&
	            srCur->children != NULL)
		{
			srTagNode = srCur ->children;
		}
		
		/* 推移第三層 */
		while (srTagNode != NULL)
		{
			memset(szTagData, 0x00, sizeof(szTagData));
			
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

			/* HostTelPrimary */
			if (xmlStrcmp(srTagNode->name, (const xmlChar*)_TAKA_COMM_PRESERVE_TAG_HostTelPrimary_) == 0)
			{
				if (strlen(szTagData) > 0)
				{
					inSetHostTelPrimary(szTagData);
				}
			}
			/* HostTelSecond */
			else if (xmlStrcmp(srTagNode->name, (const xmlChar*)_TAKA_COMM_PRESERVE_TAG_HostTelSecond_) == 0)
			{
				if (strlen(szTagData) > 0)
				{
					inSetHostTelSecond(szTagData);
				}
			}
			/* ReferralTel */
			else if (xmlStrcmp(srTagNode->name, (const xmlChar*)_TAKA_COMM_PRESERVE_TAG_ReferralTel_) == 0)
			{
				if (strlen(szTagData) > 0)
				{
					inGetReferralTel(szTagData);
				}
			}
			/* HostIPPrimary */
			else if (xmlStrcmp(srTagNode->name, (const xmlChar*)_TAKA_COMM_PRESERVE_TAG_HostIPPrimary_) == 0)
			{
				if (strlen(szTagData) > 0)
				{
					inSetHostIPPrimary(szTagData);
				}
			}
			/* HostPortNoPrimary */
			else if (xmlStrcmp(srTagNode->name, (const xmlChar*)_TAKA_COMM_PRESERVE_TAG_HostPortNoPrimary_) == 0)
			{
				if (strlen(szTagData) > 0)
				{
					inSetHostPortNoPrimary(szTagData);
				}
			}
			/* HostIPSecond */
			else if (xmlStrcmp(srTagNode->name, (const xmlChar*)_TAKA_COMM_PRESERVE_TAG_HostIPSecond_) == 0)
			{
				if (strlen(szTagData) > 0)
				{
					inSetHostIPSecond(szTagData);
				}
			}
			/* HostPortNoSecond */
			else if (xmlStrcmp(srTagNode->name, (const xmlChar*)_TAKA_COMM_PRESERVE_TAG_HostPortNoSecond_) == 0)
			{
				if (strlen(szTagData) > 0)
				{
					inSetHostPortNoSecond(szTagData);
				}
			}
			
			srTagNode = srTagNode->next;
		}
			
		/* 推移第二層 */
		srCur = srCur->next;
	}
	
	/* free */
	inXML_End(&srDoc);
	
	/* 回存完刪除檔案 */
	inFile_Unlink_File(_TAKA_COMM_PRESERVE_XML_FILE_NAME_, _FS_DATA_PATH_);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inTAKA_TAKA_Comm_Get_In_Record() START !");
	}
	
	return (VS_SUCCESS);
}