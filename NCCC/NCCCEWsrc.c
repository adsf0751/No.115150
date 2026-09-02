#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <ctosapi.h>
#include <curl/curl.h>
#include <sqlite3.h>
#include "../SOURCE/INCLUDES/Define_1.h"
#include "../SOURCE/INCLUDES/Define_2.h"
#include "../SOURCE/INCLUDES/TransType.h"
#include "../SOURCE/INCLUDES/Transaction.h"
#include "../SOURCE/INCLUDES/AllStruct.h"
#include "../SOURCE/DISPLAY/Display.h"
#include "../SOURCE/PRINT/Print.h"
#include "../SOURCE/FUNCTION/Sqlite.h"
#include "../SOURCE/FUNCTION/CFGT.h"
#include "../SOURCE/FUNCTION/EDC.h"
#include "../SOURCE/FUNCTION/FuncTable.h"
#include "../SOURCE/FUNCTION/Function.h"
#include "../SOURCE/FUNCTION/HDT.h"
#include "../SOURCE/FUNCTION/HDPT.h"
#include "../SOURCE/COMM/Comm.h"
#include "../SOURCE/EVENT/Flow.h"
#include "../NCCC/NCCCEWsrc.h"

extern  int	ginDebug;		/* Debug使用 extern */
extern	int	ginFindRunTime;
DEMO_EW_REC	gsrDEMO_EWRec[6] = {};

int EW_SALE_TRT_TABLE[] =
{
	_NCCC_EW_FUNCTION_CHECK_TRANS_FUNCTION_FLOW_,
	_COMM_MODEM_PREDIAL_,
        _CREDIT_FUNCTION_GET_STOREID_,
	_NCCC_FUNCTION_MAKE_REFNO_,
	_NCCC_FUNCTION_SET_TXN_ONLINE_OFFLINE_FLOW_,
        _NCCC_FUNCTION_BUILD_AND_SEND_PACKET_FLOW_,
        _FUNCTION_UPDATE_INV_,
	_FUNCTION_ECR_SEND_TRANSACTION_RESULT_,
	_NCCC_FUNCTION_CHECK_UPDATE_BATCH_NUM_,
        _FLOW_NULL_,
};

int EW_REFUND_TRT_TABLE[] =
{
	_NCCC_EW_FUNCTION_CHECK_TRANS_FUNCTION_FLOW_,
	_COMM_MODEM_PREDIAL_,
        _CREDIT_FUNCTION_GET_STOREID_,
	_NCCC_FUNCTION_MAKE_REFNO_,
	_NCCC_FUNCTION_SET_TXN_ONLINE_OFFLINE_FLOW_,
        _NCCC_FUNCTION_BUILD_AND_SEND_PACKET_FLOW_,
        _FUNCTION_UPDATE_INV_,
	_FUNCTION_ECR_SEND_TRANSACTION_RESULT_,
	_NCCC_FUNCTION_CHECK_UPDATE_BATCH_NUM_,
        _FLOW_NULL_,
};

int EW_INQUIRY_TRT_TABLE[] =
{
	_NCCC_EW_FUNCTION_CHECK_TRANS_FUNCTION_FLOW_,
	_COMM_MODEM_PREDIAL_,
	_NCCC_FUNCTION_MAKE_REFNO_,
	_NCCC_FUNCTION_SET_TXN_ONLINE_OFFLINE_FLOW_,
	_FUNCTION_UPDATE_INV_,
	_NCCC_FUNCTION_CHECK_UPDATE_BATCH_NUM_,
        _NCCC_FUNCTION_BUILD_AND_SEND_PACKET_FLOW_,
	_FUNCTION_ECR_SEND_TRANSACTION_RESULT_,
        _FLOW_NULL_,
};

TRT_TABLE TRANSACTION_NCCC_EW_TABLE[] =
{
        {_TRT_EW_SALE_,				EW_SALE_TRT_TABLE},
        {_TRT_EW_REFUND_,			EW_REFUND_TRT_TABLE},
	{_TRT_EW_INQUIRY_,			EW_INQUIRY_TRT_TABLE},
        {_FLOW_NULL_,				NULL}
};

/*
Function        :inNCCC_EW_RunTRT
Date&Time       :2023/7/21 下午 4:54
Describe        :
*/
int inNCCC_EW_RunTRT(TRANSACTION_OBJECT *pobTran, int inTRTCode)
{
        int     *inTRTID = NULL;
        int     i, inRetVal = VS_ERROR ;

        for (i = 0 ;; i ++)
        {
		if (TRANSACTION_NCCC_EW_TABLE[i].inTRTCode == inTRTCode)
		{
			inTRTID = TRANSACTION_NCCC_EW_TABLE[i].inTRTID;
			break;
		}
		else if (TRANSACTION_NCCC_EW_TABLE[i].inTRTCode == -1)
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
Function        :inNCCC_EW_Func_Check_Transaction_Function
Date&Time       :2023/8/17 上午 11:03
Describe        :確認交易功能是否打開，
*/
int inNCCC_EW_Func_Check_Transaction_Function(int inCode)
{
	int		inHDTIndex = -1;
	char		szTransFunc[20 + 1] = {0};
	unsigned char	uszTxnEnable = VS_TRUE;
	
	/* 只檢查EW的功能 */
	inHDTIndex = -1;
	inFunc_Find_Specific_HDTindex(-1, _HOST_NAME_EW_, &inHDTIndex);
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
	
	if (inCode == _EW_SALE_)
	{
		if (memcmp(&szTransFunc[0], "Y", 1) != 0)
		{
			uszTxnEnable = VS_FALSE;
		}
	}
	else if (inCode == _EW_REFUND_)
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
Function        :inNCCC_EW_Func_Check_Transaction_Function_Flow
Date&Time       :2023/8/17 上午 11:05
Describe        :確認交易功能是否打開，因為根據Host，所以每個Host一個Function
*/
int inNCCC_EW_Func_Check_Transaction_Function_Flow(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_SUCCESS;
	unsigned char	uszTxnEnable = VS_TRUE;

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inNCCC_EW_Func_Check_Transaction_Function_Flow() START !");
	}
	
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
	}
	
	inRetVal = inNCCC_EW_Func_Check_Transaction_Function(pobTran->inTransactionCode);
	if (inRetVal != VS_SUCCESS)
	{
		uszTxnEnable = VS_FALSE;
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inNCCC_Func_Check_Transaction_Function_Flow(%d) END !", uszTxnEnable);
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