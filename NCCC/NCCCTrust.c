#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <ctosapi.h>
#include <curl/curl.h>
#include <sqlite3.h>
#include <libxml/tree.h>
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
#include "../SOURCE/COMM/Modem.h"
#include "../SOURCE/COMM/Ftps.h"
#include "../SOURCE/COMM/TLS.h"
#include "../SOURCE/FUNCTION/Sqlite.h"
#include "../SOURCE/FUNCTION/ASMC.h"
#include "../SOURCE/FUNCTION/Accum.h"
#include "../SOURCE/FUNCTION/Batch.h"
#include "../SOURCE/FUNCTION/File.h"
#include "../SOURCE/FUNCTION/Function.h"
#include "../SOURCE/FUNCTION/FuncTable.h"
#include "../SOURCE/FUNCTION/CDT.h"
#include "../SOURCE/FUNCTION/CDTX.h"
#include "../SOURCE/FUNCTION/Card.h"
#include "../SOURCE/FUNCTION/CPT.h"
#include "../SOURCE/FUNCTION/HDT.h"
#include "../SOURCE/FUNCTION/CFGT.h"
#include "../SOURCE/FUNCTION/EDC.h"
#include "../SOURCE/FUNCTION/HDPT.h"
#include "../SOURCE/FUNCTION/ECR.h"
#include "../SOURCE/FUNCTION/RS232.h"
#include "../SOURCE/FUNCTION/SCDT.h"
#include "../SOURCE/FUNCTION/VWT.h"
#include "../SOURCE/FUNCTION/TDT.h"
#include "../SOURCE/FUNCTION/PIT.h"
#include "../SOURCE/FUNCTION/NexsysSDK.h"
#include "../SOURCE/FUNCTION/CastlePWD.h"
#include "../SOURCE/FUNCTION/Utility.h"
#include "../SOURCE/FUNCTION/APDU.h"
#include "../SOURCE/FUNCTION/TMSIPDT.h"
#include "../SOURCE/FUNCTION/XML.h"
#include "../SOURCE/EVENT/MenuMsg.h"
#include "../SOURCE/EVENT/Flow.h"
#include "../SOURCE/EVENT/Menu.h"
#include "../HG/HGsrc.h"
#include "../HG/HGiso.h"
#include "../CREDIT/Creditfunc.h"
#include "../CREDIT/CreditprtByBuffer.h"
#include "../FISC/NCCCfisc.h"
#include "../EMVSRC/EMVsrc.h"
#include "../CTLS/CTLS.h"
#include "NCCCsrc.h"
#include "NCCCats.h"
#include "NCCCmfes.h"
#include "NCCCtmk.h"
#include "NCCCdcc.h"
#include "NCCCesc.h"
#include "NCCCloyalty.h"
#include "NCCCTicketSrc.h"
#include "NCCCtmsCPT.h"
#include "NCCCtmsFTPFLT.h"
#include "NCCCtmsFTP.h"
#include "NCCCtms.h"
#include "NCCCtSAM.h"
#include "TAKAsrc.h"
#include "TAKAiso.h"
#include "NCCCTrust.h"

#define max(x, y) (((x) > (y)) ? (x) : (y))
#define min(x, y) (((x) < (y)) ? (x) : (y))

DEMO_TRUST_REC          gsrDEMO_TRUSTRec;
SQLITE_TAG_TABLE	TABLE_TRUST_TAG[] = 
{
	{"inTableID"			,"INTEGER"	,"PRIMARY KEY"	,""},	/* Table ID Primary key, sqlite table專用避免PRIMARY KEY重複 */
	{"inCode"			,"INTEGER"	,""		,""},	
	{"lnTxnAmount"			,"INTEGER"	,""		,""},	
	{"lnBatchNum"			,"INTEGER"	,""		,""},	/* Batch Number */
	{"szReconciliationNo"           ,"BLOB"         ,""             ,""},	/* 銷帳編號，若無則全帶空白。左靠右補空白 */
	{"szTrustInstitutionCode"       ,"BLOB"         ,""             ,""},	/* 信託機構代碼。 左靠右補空白 */
	{"szExchangeTxSerialNumber"     ,"BLOB"         ,""             ,""},	/* 交換平台交易序號（平台提供唯一值）左靠右補空白 */
	{"szMaskedBeneficiaryId"        ,"BLOB"         ,""             ,""},	/* 遮掩後的 信託受益人ID */
	{"szTrustRRN"                   ,"BLOB"         ,""             ,""},	/* 共用收單序號 */
        {"szAuthCode"                   ,"BLOB"         ,""             ,""},	/* 授權碼 */
	{"szDate"			,"BLOB"		,""		,""},	/* YYYYMMDD */
	{"szTime"			,"BLOB"		,""		,""},	/* HHMMSS */
	{"uszTrustTransBit"             ,"BLOB"         ,""             ,""},	/* 是否是信託交易 */
	{"uszUpdated"			,"BLOB"		,""		,"DEFAULT 0"},	/* For SQLite使用，pobTran中不存，若設為1則代表該紀錄已不用，初始值設為0 */
	{""},
};

int TRUST_SALE_TRT_NCCC_TRUST_TABLE[] =
{
	_NCCC_TRUST_CHECK_TRANS_FUNCTION_FLOW_,
        _NCCC_TRUST_MUST_SETTLE_CHECK_,
        _FUNCTION_CHECK_BATCH_LIMIT_,
	_NCCC_FUNCTION_MAKE_REFNO_,
	_NCCC_ESC_CHECK_,
	_NCCC_FUNCTION_SET_TXN_ONLINE_OFFLINE_FLOW_,
        _NCCC_FUNCTION_BUILD_AND_SEND_PACKET_FLOW_,
	_NCCC_FUNCTION_BEEP_AFTER_AUTH_,
	_TRUST_UPDATE_BATCH_,
        _FUNCTION_UPDATE_INV_,
        _FUNCTION_ECR_SEND_TRANSACTION_RESULT_,
	_CREDIT_FUNCTION_CHECKRESULT_,
        _FUNCTION_PRINT_RECEIPT_BY_BUFFER_FLOW_,
	_NCCC_FUNCTION_SEND_ECR_AFTER_PRINT_RECEIPT_,
        _FLOW_NULL_,
};

int TRUST_VOID_TRT_NCCC_TRUST_TABLE[] =
{
	_NCCC_TRUST_CHECK_TRANS_FUNCTION_FLOW_,
        _NCCC_TRUST_MUST_SETTLE_CHECK_,
        _FUNCTION_CHECK_BATCH_LIMIT_,
	_NCCC_ESC_CHECK_,
	_NCCC_FUNCTION_SET_TXN_ONLINE_OFFLINE_FLOW_,
        _NCCC_FUNCTION_BUILD_AND_SEND_PACKET_FLOW_,
	_NCCC_FUNCTION_BEEP_AFTER_AUTH_,
	_TRUST_UPDATE_BATCH_,
        _FUNCTION_UPDATE_INV_,
        _FUNCTION_ECR_SEND_TRANSACTION_RESULT_,
	_CREDIT_FUNCTION_CHECKRESULT_,
        _FUNCTION_PRINT_RECEIPT_BY_BUFFER_FLOW_,
	_NCCC_FUNCTION_SEND_ECR_AFTER_PRINT_RECEIPT_,
        _FLOW_NULL_,
};

int SETTLE_TRT_NCCC_TRUST_TABLE[] =
{
	_NCCC_TRUST_CHECK_TRANS_FUNCTION_FLOW_,
        _NCCC_TRUST_SEND_REVERSAL_,
        _FUNCTION_PREPRINT_AUTO_REPORT_BY_BUFFER_,
        _FUNCTION_PRINT_TOTAL_REPORT_BY_BUFFER_,
        _FUNCTION_DELETE_BATCH_FLOW_,
        _FUNCTION_REST_BATCH_INV_,
	_FUNCTION_UPDATE_BATCH_NUM_,
        _FLOW_NULL_,
};

TRT_TABLE TRANSACTION_NCCC_TRUST_TABLE[] =
{
        {_TRT_SETTLE_,				SETTLE_TRT_NCCC_TRUST_TABLE},
        {_TRT_TRUST_SALE_,			TRUST_SALE_TRT_NCCC_TRUST_TABLE},
	{_TRT_TRUST_VOID_,			TRUST_VOID_TRT_NCCC_TRUST_TABLE},
        {_FLOW_NULL_,				NULL}
};

extern  int		ginDebug;		/* Debug使用 extern */
extern  int		ginISODebug;		/* Debug使用 extern */
/*
 Function        :inNCCC_Trust_Deal_Trust_Txn_Parameter_OPT
 Date&Time       :2025/9/16 下午 3:36
 Describe        :【需求單-113172】TMS系統新增「TMS參數Print_Mode 4=兩聯免簽不印商店聯」，免簽名交易不列印商店存根聯
 *                用於是否顯示銷售免。

 */
int inNCCC_Trust_Deal_Trust_Txn_Parameter_OPT(TRANSACTION_OBJECT *pobTran)
{
	int	inTrustIndex = -1;
	char	szBatchNum[6 + 1] = {0};
	char	szInvNum[6 + 1] = {0};
        char    szHostEnable[1 + 1] = {0};
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_Trust_Deal_Trust_Txn_Parameter_OPT() START !");
	}
	vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Trust_Deal_Trust_Txn_Parameter_OPT START!");
	
	
	inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_TRUST_, &inTrustIndex);
	if (inTrustIndex != -1)
	{
		pobTran->srBRec.inHDTIndex = inTrustIndex;
		if (inLoadHDTRec(pobTran->srBRec.inHDTIndex) != VS_SUCCESS)
		{
			vdUtility_SYSFIN_LogMessage(AT, "inLoadHDTRec(%d) failed", pobTran->srBRec.inHDTIndex);
			return (VS_ERROR);
		}
                
                memset(szHostEnable, 0x00, sizeof(szHostEnable));
                inGetHostEnable(szHostEnable);
                if (szHostEnable[0] != 'Y')
                {
                        vdUtility_SYSFIN_LogMessage(AT, "Trust Not Open(%s)", szHostEnable);
                        pobTran->inErrorMsg = _ERROR_CODE_V3_FUNC_CLOSE_;
			return (VS_ERROR);
                }
		
		if (inLoadHDPTRec(pobTran->srBRec.inHDTIndex) != VS_SUCCESS)
		{
			vdUtility_SYSFIN_LogMessage(AT, "inLoadHDPTRec(%d) failed", pobTran->srBRec.inHDTIndex);
			return (VS_ERROR);
		}
		
		memset(szBatchNum, 0x00, sizeof(szBatchNum));
		inGetBatchNum(szBatchNum);
                pobTran->srBRec.lnBatchNum = atol(szBatchNum);
                
		memset(szInvNum, 0x00, sizeof(szInvNum));
		inGetInvoiceNum(szInvNum);
		pobTran->srBRec.lnOrgInvNum = atol(szInvNum);
	}
	else
	{
		vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Func_Deal_Trust_Txn_Parameter_OPT Failed Lock(%d)", inTrustIndex);
                pobTran->inErrorMsg = _ERROR_CODE_V3_FUNC_NOT_SUPPORT_;
                
		return (VS_ERROR);
	}
        
        /* ECR */
        if (pobTran->uszECRBit == VS_TRUE)
        {
                inNCCC_Trust_Func_Unpack_QRCode(pobTran);
        }
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_Trust_Deal_Trust_Txn_Parameter_OPT() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Trust_Deal_Trust_Txn_Parameter_OPT END!");
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_Trust_Table_Link_TrustRec
Date&Time       :2025/10/7 下午 3:53
Describe        :將pobTran變數pointer位置放到Table中(用以解決每一個function都要放一個table的問題)
 *		 這邊直接把pobTran的pointer直接指到srAll(之後可能要考慮給動態記憶體)，TagName因為是寫在這個Function內的Table，所以要給實體位置儲存
*/
int inNCCC_Trust_Table_Link_TrustRec(TRANSACTION_OBJECT *pobTran, SQLITE_ALL_TABLE *srAll, int inLinkState)
{
	SQLITE_INT32T_TABLE TABLE_BATCH_INT[] = 
	{
		{0	,"inTableID"			,&pobTran->inTableID			},	/* inTableID */
                {0	,"inCode"			,&pobTran->srTrustRec.inCode            },	
                {0	,"lnTxnAmount"			,&pobTran->srTrustRec.lnTxnAmount       },	
                {0	,"lnBatchNum"			,&pobTran->srBRec.lnBatchNum            },	/* Batch Number */
		{0	,""				,NULL					}	/* 這行用Null用來知道尾端在哪 */
		
	};
	
	SQLITE_INT32T_TABLE TABLE_BATCH_INT64T[] = 
	{

		{0	,""				,NULL				}	/* 這行用Null用來知道尾端在哪 */
	};
	
	SQLITE_CHAR_TABLE TABLE_BATCH_CHAR[] =
	{
                {0	,"szReconciliationNo"           ,&pobTran->srTrustRec.szReconciliationNo        ,sizeof(pobTran->srTrustRec.szReconciliationNo)         },	/* 銷帳編號，若無則全帶空白。左靠右補空白 */
                {0	,"szTrustInstitutionCode"       ,&pobTran->srTrustRec.szTrustInstitutionCode    ,sizeof(pobTran->srTrustRec.szTrustInstitutionCode)     },	/* 信託機構代碼。 左靠右補空白 */
                {0	,"szExchangeTxSerialNumber"     ,&pobTran->srTrustRec.szExchangeTxSerialNumber  ,sizeof(pobTran->srTrustRec.szExchangeTxSerialNumber)   },	/* 交換平台交易序號（平台提供唯一值）左靠右補空白 */
                {0	,"szMaskedBeneficiaryId"        ,&pobTran->srTrustRec.szMaskedBeneficiaryId     ,sizeof(pobTran->srTrustRec.szMaskedBeneficiaryId)      },	/* 遮掩後的 信託受益人ID */
                {0	,"szTrustRRN"                   ,&pobTran->srTrustRec.szTrustRRN                ,sizeof(pobTran->srTrustRec.szTrustRRN)                 },	/* 共用收單序號 */
                {0	,"szAuthCode"                   ,&pobTran->srBRec.szAuthCode                    ,sizeof(pobTran->srBRec.szAuthCode)                     },	/* 授權碼 */
		{0	,"szDate"			,pobTran->srBRec.szDate				,strlen(pobTran->srBRec.szDate)				},	/* YYYYMMDD */
		{0	,"szTime"			,pobTran->srBRec.szTime				,strlen(pobTran->srBRec.szTime)				},	/* HHMMSS */
                {0	,"uszTrustTransBit"             ,&pobTran->srTrustRec.uszTrustTransBit          ,sizeof(pobTran->srTrustRec.uszTrustTransBit)           },	/* 是否是信託交易 */
		{0	,""				,NULL                                           ,0                                                      }	/* 這行用Null用來知道尾端在哪 */
	};
	
	if (ginDebug == VS_TRUE)
        {
        	inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inNCCC_Trust_Table_Link_TrustRec()_START");
        }
	
	SQLITE_LINK_TABLE	srLink;
	memset(&srLink, 0x00, sizeof(SQLITE_LINK_TABLE));
	srLink.psrInt = TABLE_BATCH_INT;
	srLink.psrInt64t = TABLE_BATCH_INT64T;
	srLink.psrChar = TABLE_BATCH_CHAR;
	srLink.psrText = NULL;
	
	inSqlite_Table_Link(srAll, inLinkState, &srLink);
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inNCCC_Trust_Table_Link_TrustRec()_END");
                inLogPrintf(AT, "----------------------------------------");
        }
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_Trust_VOID_Confirm
Date&Time       :2025/10/9 下午 3:44
Describe        :按0確認是否要進行取消交易
*/
int inNCCC_Trust_VOID_Confirm(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;
        char    szDispMsg[_DISP_MSG_SIZE_ + 1] = {0};
        char    szAmountMsg[_DISP_MSG_SIZE_ + 1] = {0};
        char    szKey = 0x00;
	char	szCustomerIndicator[3 + 1] = {0};

	vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Trust_VOID_Confirm START!");
	
        /* 確認金額 */
        if (pobTran->srTrustRec.lnTxnAmount <= 0)
                return (VS_ERROR);

	/* (需求單 - 107227)邦柏科技自助作業客製化 客製化107不顯示確認畫面亦不用按0確認 by Russell 2018/12/7 下午 2:35 */
	/* (需求單 - 107276)自助交易標準400做法 by Russell 2018/12/27 上午 11:20 */
	/* 客製化123比照客製化119比照客製化111，取消不用按0 */
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	if ((!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)           ||
	     !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_)   ||
             !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)        ||
             !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)        ||
             !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)		||
	     !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))	&&
	     pobTran->uszECRBit == VS_TRUE)
	{
		inRetVal = VS_SUCCESS;
	}
	/* (需求單 - 109327)V3機型開發支援Vx520的客製化參數需求 (002-耐斯廣場/王子大飯店) 取消不按零確認 by Russell 2020/10/6 下午 4:46 */
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_002_NICE_PLAZA_, _CUSTOMER_INDICATOR_SIZE_))
	{
		inRetVal = VS_SUCCESS;
	}
	/* (需求單 - 109327)V3機型開發支援Vx520的客製化參數需求 (005-FPG) 取消不按零確認 by Russell 2022/10/5 下午 7:38 */
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_005_FPG_, _CUSTOMER_INDICATOR_SIZE_))
	{
		inRetVal = VS_SUCCESS;
	}
	else
	{
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		memset(szAmountMsg, 0x00, sizeof(szAmountMsg));
	
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		/* 顯示共用收單序號 */
		inDISP_ChineseFont("共用收單序號", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_CENTER_);
		/* 顯示共用收單序號值 */
		sprintf(szDispMsg, "%s", pobTran->srTrustRec.szTrustRRN);
		inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _COLOR_RED_, _DISP_RIGHT_);

		sprintf(szAmountMsg, "%ld", pobTran->srTrustRec.lnTxnAmount);
		inFunc_Amount_Comma(szAmountMsg, "NT$" , ' ', _SIGNED_NONE_, 13, _PADDING_LEFT_);
		
		inDISP_ChineseFont("金額", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

                if (strlen(szAmountMsg) > 13)
                        inDISP_EnglishFont_Point_Color(szAmountMsg, _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_RED_,_COLOR_WHITE_, 7);
                else
                        inDISP_EnglishFont_Point_Color(szAmountMsg, _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_RED_,_COLOR_WHITE_, 8);
		
		inDISP_ChineseFont("請按[0]進行取消", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_CENTER_);

		while (1)
		{
			szKey = uszKBD_GetKey(_EDC_TIMEOUT_);

			if (szKey == _KEY_0_)
			{
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
	}

        return (inRetVal);
}

/*
Function        :inNCCC_Trust_RunTRT
Date&Time       :2025/10/14 下午 4:05
Describe        :
*/
int inNCCC_Trust_RunTRT(TRANSACTION_OBJECT *pobTran, int inTRTCode)
{
        int     *inTRTID = NULL;
        int     i, inRetVal = VS_ERROR ;
	char	szFESMode[2 + 1] = {0};
	char	szCFESMode[2 + 1] = {0};
	
	memset(szFESMode, 0x00, sizeof(szFESMode));
	inGetNCCCFESMode(szFESMode);
	memset(szCFESMode, 0x00, sizeof(szCFESMode));
	inGetCloud_MFES(szCFESMode);
        for (i = 0 ;; i ++)
        {
		if (memcmp(szFESMode, _NCCC_03_MFES_MODE_, 2) == 0)
		{
			/* (需求單 - 107276)自助交易標準做法 by Russell 2019/3/5 下午 4:39 */
			if (memcmp(szCFESMode, "Y", strlen("Y")) == 0)
			{
				if (TRANSACTION_NCCC_TRUST_TABLE[i].inTRTCode == inTRTCode)
				{
					inTRTID = TRANSACTION_NCCC_TRUST_TABLE[i].inTRTID;
					break;
				}
				else if (TRANSACTION_NCCC_TRUST_TABLE[i].inTRTCode == -1)
				{
					break;
				}
			}
			else
			{
				if (TRANSACTION_NCCC_TRUST_TABLE[i].inTRTCode == inTRTCode)
				{
					inTRTID = TRANSACTION_NCCC_TRUST_TABLE[i].inTRTID;
					break;
				}
				else if (TRANSACTION_NCCC_TRUST_TABLE[i].inTRTCode == -1)
				{
					break;
				}
			}
			
		}/* 現在MPAS和ATS共用電文 */
		else if (memcmp(szFESMode, _NCCC_05_ATS_MODE_, 2) == 0	||
			 memcmp(szFESMode, _NCCC_04_MPAS_MODE_, 2) == 0)
		{
			if (TRANSACTION_NCCC_TRUST_TABLE[i].inTRTCode == inTRTCode)
			{
				inTRTID = TRANSACTION_NCCC_TRUST_TABLE[i].inTRTID;
				break;
			}
			else if (TRANSACTION_NCCC_TRUST_TABLE[i].inTRTCode == -1)
			{
				break;
			}
			
		}
		else
		{
			inTRTID = NULL;
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
	
	/* 如果有插SmartPay錯誤要Power Off */
	if (pobTran->uszFISCBit == VS_TRUE)
	{
		inFLOW_RunFunction(pobTran, _FISC_CARD_POWER_OFF_);
	}

	/* 斷線 */
	inCOMM_End(pobTran);

        return (inRetVal);
}

/*
Function        :inNCCC_Trust_Check_Transaction_Function_Flow
Date&Time       :2025/10/14 下午 7:05
Describe        :確認交易功能是否打開，因為根據Host，所以每個Host一個Function
*/
int inNCCC_Trust_Check_Transaction_Function_Flow(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_SUCCESS;
	unsigned char	uszTxnEnable = VS_TRUE;

        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Trust_Check_Transaction_Function_Flow START!");
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_Trust_Check_Transaction_Function_Flow() START !");
	}
	
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
	}
	
        inRetVal = inNCCC_Trust_Check_Transaction_Function(pobTran->inTransactionCode);
        if (inRetVal != VS_SUCCESS)
        {
                vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Trust_Check_Transaction_Function_Flow inNCCC_Func_Check_Transaction_Function failed");
                uszTxnEnable = VS_FALSE;
        }
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_Trust_Check_Transaction_Function_Flow(%d) END !", uszTxnEnable);
		inLogPrintf(AT, "----------------------------------------");
	}
	
	/* 此功能已關閉 */
	if (uszTxnEnable == VS_FALSE)
	{
                vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Trust_Check_Transaction_Function_Flow func_close");
		pobTran->inErrorMsg = _ERROR_CODE_V3_FUNC_CLOSE_;
		
		return (VS_ERROR);
	}
	else
	{
                vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Trust_Check_Transaction_Function_Flow END!");
                
		return (VS_SUCCESS);
	}
}

/*
Function        :inNCCC_Trust_Check_Transaction_Function
Date&Time       :2025/10/14 下午 7:08
Describe        :確認交易功能是否打開，
*/
int inNCCC_Trust_Check_Transaction_Function(int inCode)
{
        int             inHDTIndex = -1;
	char		szTransFunc[20 + 1] = {0};
	char		szFunEnable[2 + 1] = {0};
	unsigned char	uszTxnEnable = VS_TRUE;
	
	/* 只檢查TRUST的功能 */
	inHDTIndex = -1;
	inFunc_Find_Specific_HDTindex(-1, _HOST_NAME_TRUST_, &inHDTIndex);
	if (inHDTIndex != -1)
	{
		inLoadHDTRec(inHDTIndex);
	}
	else
	{
		return (VS_ERROR);
	}
	
	memset(szTransFunc, 0x00, sizeof(szTransFunc));
	if (inGetTransFunc(szTransFunc) != VS_SUCCESS)
		return (VS_ERROR);
	
	if (inCode == _SALE_	||
	    inCode == _CUP_SALE_||
	    inCode == _FISC_SALE_)
	{
		if (memcmp(&szTransFunc[0], "Y", 1) != 0)
		{
			uszTxnEnable = VS_FALSE;
		}
	}
	else if (inCode == _VOID_		||
		 inCode == _CUP_VOID_		||
		 inCode == _CUP_PRE_AUTH_VOID_	||
		 inCode == _CUP_PRE_COMP_VOID_	||
		 inCode == _FISC_VOID_)
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
	else if (inCode == _REFUND_		||
		 inCode == _CUP_REFUND_		||
		 inCode == _FISC_REFUND_)
	{
		if (memcmp(&szTransFunc[3], "Y", 1) != 0)
		{
			uszTxnEnable = VS_FALSE;
		}
	}
	/* NCCC(含MPAS)的退貨功能一併管理分期、紅利退貨 */
	/* 分期退貨要看兩個開關 */
	else if (inCode == _INST_REFUND_	||
		 inCode == _CUP_INST_REFUND_)
	{
		if ((memcmp(&szTransFunc[3], "Y", 1) != 0) || (memcmp(&szTransFunc[7], "Y", 1) != 0))
		{
			uszTxnEnable = VS_FALSE;
		}
	}
	/* 紅利退貨要看兩個開關 */
	else if (inCode == _REDEEM_REFUND_	||
		 inCode == _CUP_REDEEM_REFUND_)
	{
		if ((memcmp(&szTransFunc[3], "Y", 1) != 0) || (memcmp(&szTransFunc[8], "Y", 1) != 0))
		{
			uszTxnEnable = VS_FALSE;
		}
	}
	/* 該Byte Map=0為關閉交易補登功能， Byte Map=1為開啟 Online 交易補登，Byte Map = 2為開啟 Offline交易補登。 */
	else if (inCode == _SALE_OFFLINE_)
	{
		if (memcmp(&szTransFunc[4], "0", 1) == 0)
		{
			uszTxnEnable = VS_FALSE;
		}
	}
	else if (inCode == _PRE_AUTH_	||
		 inCode == _PRE_COMP_	||
		 inCode == _CUP_PRE_AUTH_	||
		 inCode == _CUP_PRE_COMP_)
	{
		if (memcmp(&szTransFunc[5], "Y", 1) != 0)
		{
			uszTxnEnable = VS_FALSE;
		}
	}
	else if (inCode == _TIP_)
	{
		if (memcmp(&szTransFunc[6], "Y", 1) != 0)
		{
			uszTxnEnable = VS_FALSE;
		}
	}
	else if (inCode == _INST_SALE_	||
		 inCode == _CUP_INST_SALE_)
	{
		if (memcmp(&szTransFunc[7], "Y", 1) != 0)
		{
			uszTxnEnable = VS_FALSE;
		}
	}
	else if (inCode == _REDEEM_SALE_	||
		 inCode == _CUP_REDEEM_SALE_)
	{
		if (memcmp(&szTransFunc[8], "Y", 1) != 0)
		{
			uszTxnEnable = VS_FALSE;
		}
	}
	else if (inCode == _INST_ADJUST_)
	{
		if (memcmp(&szTransFunc[9], "Y", 1) != 0)
		{
			uszTxnEnable = VS_FALSE;
		}
	}
	else if (inCode == _REDEEM_ADJUST_)
	{
		if (memcmp(&szTransFunc[10], "Y", 1) != 0)
		{
			uszTxnEnable = VS_FALSE;
		}
	}
	else if (inCode == _MAIL_ORDER_	||
		 inCode == _CUP_MAIL_ORDER_)
	{
		if (memcmp(&szTransFunc[11], "Y", 1) != 0)
		{
			uszTxnEnable = VS_FALSE;
		}
	}
	else if (inCode == _CUP_MAIL_ORDER_REFUND_)
	{
		/* 郵購開關 && 退貨開關 */
		if (memcmp(&szTransFunc[3], "Y", 1) != 0 || memcmp(&szTransFunc[11], "Y", 1) != 0)
		{
			uszTxnEnable = VS_FALSE;
		}

	}

	/* 關閉銀聯分期紅利 */
	if (inCode == _CUP_INST_SALE_	||
	    inCode == _CUP_REDEEM_SALE_)
	{
		uszTxnEnable = VS_FALSE;
	}
	
	/* 交易別是銀聯的，銀聯功能未開要擋(For ECR) */
	if (inCode == _CUP_SALE_		||
	    inCode == _CUP_REFUND_		||
	    inCode == _CUP_VOID_		||
	    inCode == _CUP_INST_SALE_		||
	    inCode == _CUP_INST_REFUND_		||
	    inCode == _CUP_REDEEM_SALE_		||
	    inCode == _CUP_REDEEM_REFUND_	||
	    inCode == _CUP_PRE_AUTH_		||
	    inCode == _CUP_PRE_AUTH_VOID_	||
	    inCode == _CUP_PRE_COMP_		||
	    inCode == _CUP_PRE_COMP_VOID_	||
	    inCode == _CUP_MAIL_ORDER_		||
	    inCode == _CUP_MAIL_ORDER_REFUND_)
	{
		memset(szFunEnable, 0x00, sizeof(szFunEnable));
		inGetCUPFuncEnable(szFunEnable);
		if (memcmp(szFunEnable, "Y", 1) != 0)
		{
			uszTxnEnable = VS_FALSE;
		}
	}
	
	/* 交易別是金融卡的，金融卡功能未開要擋(For ECR) */
	if (inCode == _FISC_SALE_		||
	    inCode == _FISC_REFUND_		||
	    inCode == _FISC_VOID_)
	{
		memset(szFunEnable, 0x00, sizeof(szFunEnable));
		inGetFiscFuncEnable(szFunEnable);
		if (memcmp(szFunEnable, "Y", 1) != 0)
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
Function        :inNCCC_Trust_Func_Must_SETTLE
Date&Time       :2025/10/14 下午 7:12
Describe        :確認是否要先結帳
*/
int inNCCC_Trust_Func_Must_SETTLE(TRANSACTION_OBJECT *pobTran)
{
	int	inHostIndex = -1;
        char    szMustSettleBit[2 + 1] = {0};
	
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
	}
	
	if (inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_TRUST_, &inHostIndex) != VS_SUCCESS)
	{
		/* 找不到直接return VS_ERROR */
		return (VS_ERROR);
	}
        inLoadHDPTRec(inHostIndex);
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
		strcpy(srDispMsgObj.szErrMsg1, "信託");
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
Function        :inNCCC_Trust_Func_Unpack_QRCode
Date&Time       :2025/10/17 下午 6:26
Describe        :拆出信託機構代碼
*/
int inNCCC_Trust_Func_Unpack_QRCode(TRANSACTION_OBJECT *pobTran)
{
        int     inCnt = 0;
        int     inLen = 0;
        char    szLen[4 + 1] = {0};
        
	if (strlen(pobTran->srBRec.szBarCodeData) > 0)
	{
		/* 取得type */
		inCnt += 2;

		memset(szLen, 0x00, sizeof(szLen));
		memcpy(szLen, &pobTran->srBRec.szBarCodeData[inCnt], 4);
		inCnt += 4;

		inLen = atoi(szLen);
		memcpy(pobTran->szTrustQRCodeType, &pobTran->srBRec.szBarCodeData[inCnt], inLen);
		inCnt += inLen;
		
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "szTrustQRCodeType:(%s)", pobTran->szTrustQRCodeType);
		}

		/* 取得機構代碼 */
		inCnt += 2;

		memset(szLen, 0x00, sizeof(szLen));
		memcpy(szLen, &pobTran->srBRec.szBarCodeData[inCnt], 4);
		inCnt += 4;

		inLen = atoi(szLen);
		memcpy(pobTran->srTrustRec.szTrustInstitutionCode, &pobTran->srBRec.szBarCodeData[inCnt], inLen);
		inCnt += inLen;

		/* 補空白補足七碼 */
		memset(&pobTran->srTrustRec.szTrustInstitutionCode[inLen], ' ', 7 - inLen);
		
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "szTrustInstitutionCode:(%s)", pobTran->srTrustRec.szTrustInstitutionCode);
		}
	}
        
        return (VS_SUCCESS);
}

/*
Function        :inNCCC_Trust_Func_GenMAC
Date&Time       :2025/10/23 下午 7:03
Describe        :
*/
int inNCCC_Trust_Func_GenMAC(TRANSACTION_OBJECT *pobTran, char *szF_03, char *szF_04, char *szF_11, char *szF_41)
{
	int 	inMACLen = 0;
	int 	inRetVal;
	char	szDebugMsg[128 + 1], szDataBuf[128 + 1];
	char	szAscii[16 + 1];
	

	/* 處理中‧‧‧‧‧ */
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_PutGraphic(_PROCESS_, 0, _COORDINATE_Y_LINE_8_7_); 

	
		/*
			1. ISO8583 F_03（Processing Code）前2 碼。
                        2. ISO8583 F_04_Data（交易金額）
                        例：NT 187 → 則DATA 為 「000000018700」共12 碼。
                        3. ISO8583 F_11（STAN）後2 碼。
                        4. ISO8583 F_41（TID）8 碼。
		*/
		memset(szDataBuf, 0x00, sizeof(szDataBuf));
		
		/* Process Code前兩碼 2 碼 (BCD to ASCII) */
		strcat(szDataBuf, szF_03);
                /* ISO8583 F_04 Data */
		strcat(szDataBuf, szF_04);
                /* ISO8583 F_11(STAN)後2碼(BCD to ASCII) */
		strcat(szDataBuf, szF_11);
                /* Terminal ID */
		strcat(szDataBuf, szF_41);
		
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "F03: %s", szF_03);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "F04: %s", szF_04);
			inLogPrintf(AT, szDebugMsg);
                        
                        memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "F11: %s", szF_11);
			inLogPrintf(AT, szDebugMsg);
                        
                        memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "F41: %s", szF_41);
			inLogPrintf(AT, szDebugMsg);
		}

	inMACLen = strlen(szDataBuf);

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "==========================================");
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "GenMAC Length [%d]", inMACLen);
		inLogPrintf(AT, szDebugMsg);
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "GenMAC Data [%s]", szDataBuf);
		inDISP_LogPrintf_Format(szDebugMsg, "  ", 34);
		
		inLogPrintf(AT, "==========================================");
	}
	
	if (ginISODebug == VS_TRUE)
	{
		inPRINT_ChineseFont("==========================================", _PRT_ISO_);
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "GenMAC Length [%d]", inMACLen);
		inPRINT_ChineseFont(szDebugMsg, _PRT_ISO_);
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "GenMAC Data [%s]", szDataBuf);
		inPRINT_ChineseFont_Format(szDebugMsg, "  ", 34, _PRT_ISO_);
		
		inPRINT_ChineseFont("==========================================", _PRT_ISO_);
	}

	inRetVal = inNCCC_TMK_CalculateMac(szDataBuf, pobTran->szMAC_HEX);
		
	if (ginDebug == VS_TRUE)
	{
                memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		memset(szAscii, 0x00, sizeof(szAscii));
		inFunc_BCD_to_ASCII(szAscii, (unsigned char*)pobTran->szMAC_HEX, 8);
        	sprintf(szDebugMsg, "MAC RESULT [%d][%s]", strlen(pobTran->szMAC_HEX), szAscii);
                inDISP_LogPrintf_Format(szDebugMsg, "  ", 34);
        }
	
	if (ginISODebug == VS_TRUE)
	{
                memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		memset(szAscii, 0x00, sizeof(szAscii));
		inFunc_BCD_to_ASCII(szAscii, (unsigned char*)pobTran->szMAC_HEX, 8);
        	sprintf(szDebugMsg, "MAC RESULT [%d][%s]", strlen(pobTran->szMAC_HEX), szAscii);
                inPRINT_ChineseFont_Format(szDebugMsg, "  ", 34, _PRT_ISO_);
        }

        if (inRetVal != VS_SUCCESS)
	        return (VS_ERROR);

        return (VS_SUCCESS);
}

/*
Function        :inNCCC_Trust_SendReversal
Date&Time       :2025/10/30 上午 11:29
Describe        :此function僅用來送信託reversal，不包含Reversal產生
*/
int inNCCC_Trust_SendReversal(TRANSACTION_OBJECT *pobTran)
{
        int 	inRetVal = VS_SUCCESS;
        int     inHostIndex = -1;
        char 	szSendReversalBit[1 + 1] = {0};
        char    szHostEnable[1 + 1] = {0};
	
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_Trust_SendReversal() START!");
        
        /* 先確認是否有開信託 */
        if (inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_TRUST_, &inHostIndex) != VS_SUCCESS)
	{
		return (VS_SUCCESS);
	}
        else
        {
                pobTran->srBRec.inHDTIndex = inHostIndex;
                
                if (inLoadHDTRec(pobTran->srBRec.inHDTIndex) != VS_SUCCESS)
                        return (VS_ERROR);
                
                if (inLoadHDPTRec(pobTran->srBRec.inHDTIndex) != VS_SUCCESS)
                {
                        pobTran->inErrorMsg = _ERROR_CODE_V3_GET_HDPT_TAG_FAIL_;
                        return (VS_ERROR);
                }
                
                memset(szHostEnable, 0x00, sizeof(szHostEnable));
                inGetHostEnable(szHostEnable);
                if (memcmp(szHostEnable, "Y", 1))
                {
                        return (VS_SUCCESS);
                }
        }
	
        memset(szSendReversalBit, 0x00, sizeof(szSendReversalBit));
        if (inGetSendReversalBit(szSendReversalBit) == VS_ERROR)
	{
		inUtility_StoreTraceLog_OneStep("inGetSendReversalBit Error");
		
		pobTran->inErrorMsg = _ERROR_CODE_V3_GET_HDPT_TAG_FAIL_;
                return (VS_ERROR);
	}

        if (!memcmp(szSendReversalBit, "Y", 1))
        {
                /* 若沒有連線要連線 */
                if (pobTran->uszConnectionBit != VS_TRUE)
		{
			if (inFLOW_RunFunction(pobTran, _COMM_START_) != VS_SUCCESS)
			{
				/* 通訊失敗‧‧‧‧ */
				pobTran->inTransactionResult = _TRAN_RESULT_COMM_ERROR_;
				pobTran->inErrorMsg = _ERROR_CODE_V3_COMM_;
				inFunc_Display_Error(pobTran);			/* 通訊失敗 */

				inCOMM_End(pobTran);

				return (VS_COMM_ERROR);
			}
		}
                
                if ((inRetVal = inNCCC_ATS_ReversalSendRecvPacket(pobTran)) != VS_SUCCESS)
                {
			inUtility_StoreTraceLog_OneStep("inNCCC_ATS_ReversalSendRecvPacket Error");
			
                        return (VS_ERROR);
                }

                if (inNCCC_ATS_SetReversalCnt(pobTran, _ADD_) == VS_ERROR)
                {
			pobTran->inErrorMsg = _ERROR_CODE_V3_SET_HDPT_TAG_FAIL_;
                        return (VS_ERROR);
                }
        }
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_Trust_SendReversal() END!");
	
        return (VS_SUCCESS);
}


/*
Function        :inNCCC_Func_Display_Review_Settle_TRUST
Date&Time       :20251218
Describe        :
 [20251219_BUG_MDF][UI] 信託主機在結帳時,需要顯示按0確認的頁面
*/
int inNCCC_Func_Display_Review_Settle_TRUST(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_SUCCESS;
	int		inChoice = 0;
	int		inTouchSensorFunc = _Touch_BATCH_END_;
	unsigned char	uszKey = 0x00;
	
	/* [20251219_BUG_MDF][UI] 信託主機在結帳時,需要顯示按0確認的頁面 
	 加入請按0顯示頁面,圖片顯示的方法 */
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_PutGraphic(_REVIEW_SETTLE_TRUST_, 0, _COORDINATE_Y_LINE_8_4_);
	
	/* [20251219_BUG_MDF][UI] 信託主機在結帳時,需要顯示按0確認的頁面 
	 加入請按0顯示頁面*/
	//inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned("請按[0]確認", _FONTSIZE_8X16_, _COLOR_WHITE_, _COLOR_BLACK_, _COORDINATE_Y_LINE_8_7_, VS_FALSE, _DISP_CENTER_);
	
	inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
	
	uszKey = 0x00;
	while (1)
	{
		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
		uszKey = uszKBD_Key();

		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			uszKey = _KEY_TIMEOUT_;
		}

		if (uszKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else if (uszKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}
		else if (inChoice == _BATCH_END_Touch_ENTER_BUTTON_	||
			 uszKey == _KEY_0_)
		{
			inRetVal = VS_SUCCESS;
			break;
		}
	}
	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();
	
	return (inRetVal);
}