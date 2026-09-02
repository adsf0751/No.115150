#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <sqlite3.h>
#include "../SOURCE/INCLUDES/Define_1.h"
#include "../SOURCE/INCLUDES/Define_2.h"
#include "../SOURCE/INCLUDES/TransType.h"
#include "../SOURCE/INCLUDES/Transaction.h"
#include "../SOURCE/INCLUDES/AllStruct.h"
#include "../SOURCE/DISPLAY/Display.h"
#include "../SOURCE/DISPLAY/DispMsg.h"
#include "../SOURCE/PRINT/Print.h"
#include "../SOURCE/PRINT/PrtMsg.h"
#include "../SOURCE/FUNCTION/Sqlite.h"
#include "../SOURCE/FUNCTION/Function.h"
#include "../SOURCE/FUNCTION/File.h"
#include "../SOURCE/FUNCTION/Accum.h"
#include "../SOURCE/FUNCTION/CDT.h"
#include "../SOURCE/FUNCTION/Card.h"
#include "../SOURCE/FUNCTION/CFGT.h"
#include "../SOURCE/FUNCTION/HDT.h"
#include "../SOURCE/FUNCTION/HDPT.h"
#include "../SOURCE/FUNCTION/EDC.h"
#include "../SOURCE/FUNCTION/Utility.h"
#include "../SOURCE/FUNCTION/Big5NameT.h"
#include "../FISC/NCCCfisc.h"
#include "../HG/HGsrc.h"
#include "../CTLS/CTLS.h"
#include "../CREDIT/CreditprtByBuffer.h"
#include "NCCCsrc.h"
#include "NCCCdcc.h"
#include "NCCCloyalty.h"
#include "NCCCescReceipt.h"
#include "TAKAsrc.h"

#define max(x, y) (((x) > (y)) ? (x) : (y))
#define min(x, y) (((x) < (y)) ? (x) : (y))

extern int	ginDebug;
extern long	glnESC_PokaYoke_Amt;		/* 避免再度產生送錯電子簽單的機制 */
extern long	glnESC_PokaYoke_TestAmt;		/* 測試ESC防呆機制使用金額 */

/* 重點提示:一定要轉成Big5 */

/* ESC 帳單使用 (START) */
ESC_RECEIPT_TYPE_TABLE srESCReceiptType[] =
{
	/* 00.信用卡 */
	{
		inNCCC_ESC_PRINT_LOGO,
		inNCCC_ESC_PRINT_TIDMID,
		inNCCC_ESC_PRINT_DATA,
		inNCCC_ESC_PRINT_CUP_AMOUNT,
		inNCCC_ESC_PRINT_AMOUNT,
		inNCCC_ESC_PRINT_INST,
		inNCCC_ESC_PRINT_REDEEM,
		_NULL_CH_,
		inNCCC_ESC_PRINT_ReceiptEND
	},
	
	/* 1.DCC 簽單 For Sale */
	{
		inNCCC_ESC_PRINT_LOGO,
		inNCCC_ESC_PRINT_TIDMID,
		inNCCC_ESC_PRINT_DCC_DATA,
		_NULL_CH_,
		inNCCC_ESC_PRINT_DCC_AMOUNT_FOR_SALE,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inNCCC_ESC_PRINT_DCC_ReceiptEND
	},
	
	/* 2.DCC 簽單 Not For Sale */
	{
		inNCCC_ESC_PRINT_LOGO,
		inNCCC_ESC_PRINT_TIDMID,
		inNCCC_ESC_PRINT_DCC_DATA,
		_NULL_CH_,
		inNCCC_ESC_PRINT_DCC_Amount_NOT_FOR_SALE,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inNCCC_ESC_PRINT_DCC_ReceiptEND
	},

	/* 3.DCC 簽單 轉台幣 */
	{
		inNCCC_ESC_PRINT_LOGO,
		inNCCC_ESC_PRINT_TIDMID,
		inNCCC_ESC_PRINT_DCC_DATA,
		_NULL_CH_,
		inNCCC_ESC_PRINT_DCC_Amount_CHANGE_NTD,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inNCCC_ESC_PRINT_DCC_ReceiptEND
	},
	
	/* 4.SmartPay簽單 */
	{
		inNCCC_ESC_PRINT_LOGO,
		inNCCC_ESC_PRINT_TIDMID,
		inNCCC_ESC_PRINT_FISC_Data,
		_NULL_CH_,
		inNCCC_ESC_PRINT_FISC_Amount,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inNCCC_ESC_PRINT_ReceiptEND
	},
	
       /* 5.HappyGo混合交易簽單 */
	{
		inNCCC_ESC_PRINT_LOGO,
		inNCCC_ESC_PRINT_TIDMID,
		inNCCC_ESC_PRINT_DATA,
		inNCCC_ESC_PRINT_CUP_AMOUNT,
		inNCCC_ESC_PRINT_AMOUNT,
		inNCCC_ESC_PRINT_INST,
		inNCCC_ESC_PRINT_REDEEM,
		inNCCC_ESC_PRINT_HG_Multi_Data,
		inNCCC_ESC_PRINT_ReceiptEND
	},
	
	/* 縮小版簽單 */
	{
		inNCCC_ESC_PRINT_LOGO,
		inNCCC_ESC_PRINT_TIDMID_Small,
		inNCCC_ESC_PRINT_DATA_Small,
		inNCCC_ESC_PRINT_CUP_AMOUNT_Small,
		inNCCC_ESC_PRINT_AMOUNT_Small,
		inNCCC_ESC_PRINT_INST_Small,
		inNCCC_ESC_PRINT_REDEEM_Small,
		_NULL_CH_,
		inNCCC_ESC_PRINT_ReceiptEND_Small
	},
	
	/* 縮小版DCC 簽單 For Sale */
	{
		inNCCC_ESC_PRINT_LOGO,
		inNCCC_ESC_PRINT_TIDMID_Small,
		inNCCC_ESC_PRINT_DCC_DATA_Small,
		_NULL_CH_,
		inNCCC_ESC_PRINT_DCC_AMOUNT_FOR_SALE_Small,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inNCCC_ESC_PRINT_DCC_ReceiptEND_Small
	},
	
	/* 縮小版DCC 簽單 Not For Sale */
	{
		inNCCC_ESC_PRINT_LOGO,
		inNCCC_ESC_PRINT_TIDMID_Small,
		inNCCC_ESC_PRINT_DCC_DATA_Small,
		_NULL_CH_,
		inNCCC_ESC_PRINT_DCC_Amount_NOT_FOR_SALE_Small,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inNCCC_ESC_PRINT_DCC_ReceiptEND_Small
	},
	
	/* 縮小版DCC 簽單 轉台幣 */
	{
		inNCCC_ESC_PRINT_LOGO,
		inNCCC_ESC_PRINT_TIDMID_Small,
		inNCCC_ESC_PRINT_DCC_DATA_Small,
		_NULL_CH_,
		inNCCC_ESC_PRINT_DCC_Amount_CHANGE_NTD_Small,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inNCCC_ESC_PRINT_DCC_ReceiptEND_Small
	},
	
	/* 縮小版SmartPay簽單 */
	{
		inNCCC_ESC_PRINT_LOGO,
		inNCCC_ESC_PRINT_TIDMID_Small,
		inNCCC_ESC_PRINT_FISC_Data_Small,
		_NULL_CH_,
		inNCCC_ESC_PRINT_FISC_Amount_Small,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inNCCC_ESC_PRINT_ReceiptEND_Small
	},
	
	/* 縮小版HappyGo混合交易簽單 */
	{
		inNCCC_ESC_PRINT_LOGO,
		inNCCC_ESC_PRINT_TIDMID_Small,
		inNCCC_ESC_PRINT_DATA_Small,
		inNCCC_ESC_PRINT_CUP_AMOUNT_Small,
		inNCCC_ESC_PRINT_AMOUNT_Small,
		inNCCC_ESC_PRINT_INST_Small,
		inNCCC_ESC_PRINT_REDEEM_Small,
		inNCCC_ESC_PRINT_HG_Multi_Data,
		inNCCC_ESC_PRINT_ReceiptEND_Small
	},
	
	/* 信用卡(027) */
	{
		inNCCC_ESC_PRINT_LOGO,
		inNCCC_ESC_PRINT_TIDMID,
		inNCCC_ESC_PRINT_DATA,
		inNCCC_ESC_PRINT_CUP_AMOUNT,
		inNCCC_ESC_PRINT_AMOUNT,
		inNCCC_ESC_PRINT_INST,
		inNCCC_ESC_PRINT_REDEEM,
		_NULL_CH_,
		inNCCC_ESC_PRINT_ReceiptEND_027
	},
	
	/* SmartPay簽單 */
	{
		inNCCC_ESC_PRINT_LOGO,
		inNCCC_ESC_PRINT_TIDMID,
		inNCCC_ESC_PRINT_FISC_Data,
		_NULL_CH_,
		inNCCC_ESC_PRINT_FISC_Amount,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inNCCC_ESC_PRINT_ReceiptEND_027
	},
	
       /* HappyGo混合交易簽單 */
	{
		inNCCC_ESC_PRINT_LOGO,
		inNCCC_ESC_PRINT_TIDMID,
		inNCCC_ESC_PRINT_DATA,
		inNCCC_ESC_PRINT_CUP_AMOUNT,
		inNCCC_ESC_PRINT_AMOUNT,
		inNCCC_ESC_PRINT_INST,
		inNCCC_ESC_PRINT_REDEEM,
		inNCCC_ESC_PRINT_HG_Multi_Data,
		inNCCC_ESC_PRINT_ReceiptEND_027
	},
       
	/* 客製化【061】 SALE 交易 */
	{
		inNCCC_ESC_PRINT_LOGO,
		inNCCC_ESC_PRINT_TIDMID,
		inNCCC_ESC_PRINT_DCC_DATA,
		_NULL_CH_,
		inNCCC_ESC_PRINT_DCC_AMOUNT_FOR_SALE,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inNCCC_ESC_PRINT_DCC_ReceiptEND_061
	},
	
	/* 客製化【061】 Not For SALE 交易 */
	{
		inNCCC_ESC_PRINT_LOGO,
		inNCCC_ESC_PRINT_TIDMID,
		inNCCC_ESC_PRINT_DCC_DATA,
		_NULL_CH_,
		inNCCC_ESC_PRINT_DCC_Amount_NOT_FOR_SALE,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inNCCC_ESC_PRINT_DCC_ReceiptEND_061
	},
	
	/* 客製化【061】 DCC 簽單 轉台幣 */
	{
		inNCCC_ESC_PRINT_LOGO,
		inNCCC_ESC_PRINT_TIDMID,
		inNCCC_ESC_PRINT_DCC_DATA,
		_NULL_CH_,
		inNCCC_ESC_PRINT_DCC_Amount_CHANGE_NTD,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inNCCC_ESC_PRINT_DCC_ReceiptEND_061
	},
	
	/* 客製化【043】  */
	{
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_
	},
	
	/* 信用卡(客製化005-FPG) */
	{
		inNCCC_ESC_PRINT_LOGO,
		inNCCC_ESC_PRINT_TIDMID,
		inNCCC_ESC_PRINT_DATA_005,
		inNCCC_ESC_PRINT_CUP_AMOUNT,
		inNCCC_ESC_PRINT_AMOUNT,
		inNCCC_ESC_PRINT_INST,
		inNCCC_ESC_PRINT_REDEEM,
		_NULL_CH_,
		inNCCC_ESC_PRINT_ReceiptEND
	},
	
	/* HappyGo混合交易簽單(客製化005-FPG) */
	{
		inNCCC_ESC_PRINT_LOGO,
		inNCCC_ESC_PRINT_TIDMID,
		inNCCC_ESC_PRINT_DATA_005,
		inNCCC_ESC_PRINT_CUP_AMOUNT,
		inNCCC_ESC_PRINT_AMOUNT,
		inNCCC_ESC_PRINT_INST,
		inNCCC_ESC_PRINT_REDEEM,
		inNCCC_ESC_PRINT_HG_Multi_Data,
		inNCCC_ESC_PRINT_ReceiptEND
	},
	
	/* 縮小版S 簽單 */
	{
		inNCCC_ESC_PRINT_LOGO,
		inNCCC_ESC_PRINT_TIDMID_Small,
		inNCCC_ESC_PRINT_DATA_Small_S,
		inNCCC_ESC_PRINT_CUP_AMOUNT_Small,
		inNCCC_ESC_PRINT_AMOUNT_Small_S,
		inNCCC_ESC_PRINT_INST_Small,
		inNCCC_ESC_PRINT_REDEEM_Small,
		_NULL_CH_,
		inNCCC_ESC_PRINT_ReceiptEND_Small_S
	},
	
	/* 縮小版S DCC 簽單 For Sale */
	{
		inNCCC_ESC_PRINT_LOGO,
		inNCCC_ESC_PRINT_TIDMID_Small,
		inNCCC_ESC_PRINT_DCC_DATA_Small_S,
		_NULL_CH_,
		inNCCC_ESC_PRINT_DCC_AMOUNT_FOR_SALE_Small,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inNCCC_ESC_PRINT_DCC_ReceiptEND_Small_S
	},
	
	/* 縮小版S DCC 簽單 Not For Sale */
	{
		inNCCC_ESC_PRINT_LOGO,
		inNCCC_ESC_PRINT_TIDMID_Small,
		inNCCC_ESC_PRINT_DCC_DATA_Small_S,
		_NULL_CH_,
		inNCCC_ESC_PRINT_DCC_Amount_NOT_FOR_SALE_Small,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inNCCC_ESC_PRINT_DCC_ReceiptEND_Small_S
	},
	
	/* 縮小版S DCC 簽單 轉台幣 */
	{
		inNCCC_ESC_PRINT_LOGO,
		inNCCC_ESC_PRINT_TIDMID_Small,
		inNCCC_ESC_PRINT_DCC_DATA_Small_S,
		_NULL_CH_,
		inNCCC_ESC_PRINT_DCC_Amount_CHANGE_NTD_Small,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inNCCC_ESC_PRINT_DCC_ReceiptEND_Small_S
	},
	
	/* 縮小版S SmartPay簽單 */
	{
		inNCCC_ESC_PRINT_LOGO,
		inNCCC_ESC_PRINT_TIDMID_Small,
		inNCCC_ESC_PRINT_FISC_Data_Small,
		_NULL_CH_,
		inNCCC_ESC_PRINT_FISC_Amount_Small,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inNCCC_ESC_PRINT_ReceiptEND_Small_S
	},
	
	/* 縮小版S HappyGo混合交易簽單 */
	{
		inNCCC_ESC_PRINT_LOGO,
		inNCCC_ESC_PRINT_TIDMID_Small,
		inNCCC_ESC_PRINT_DATA_Small,
		inNCCC_ESC_PRINT_CUP_AMOUNT_Small,
		inNCCC_ESC_PRINT_AMOUNT_Small,
		inNCCC_ESC_PRINT_INST_Small,
		inNCCC_ESC_PRINT_REDEEM_Small,
		inNCCC_ESC_PRINT_HG_Multi_Data,
		inNCCC_ESC_PRINT_ReceiptEND_Small_S
	},
};
/* ESC帳單使用 (END) */

/*
Function        :inNCCC_ESC_Make_E1Data
Date&Time       :2016/4/18 上午 11:18
Describe        :組E1
*/
int inNCCC_ESC_Make_E1Data(TRANSACTION_OBJECT *pobTran)
{
	int		inDataLen = 0;		/* 各部份的檔案長度 */
	int		inPrintIndex = 0;
	char		szE1Temp[3072 + 1] = {0};
	char		szDebugMsg[100 + 1] = {0};
	char		szCustomerIndicator[3 + 1] = {0};
	char		szTemplate[1 + 1] = {0};
	long		lnOrgTxnAmt = 0;
	unsigned long	ulHandle = 0;
	unsigned char	uszUseDefaultBit = VS_FALSE;
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_Make_E1Data() START!");
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	/* ESC上傳防呆機制測試使用 */
	if (glnESC_PokaYoke_TestAmt > 0)
	{
		lnOrgTxnAmt = pobTran->srBRec.lnTxnAmount;
		pobTran->srBRec.lnTxnAmount = glnESC_PokaYoke_TestAmt;
	}
	
	/* 組的是商店聯 */
	pobTran->srBRec.inPrintOption = _PRT_MERCH_;
	
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_027_CHUNGHWA_TELECOM_, _CUSTOMER_INDICATOR_SIZE_))
	{
		/* 信用卡DCC */
		if (pobTran->srBRec.uszDCCTransBit == VS_TRUE)
		{
			uszUseDefaultBit = VS_TRUE;
		}
		/* DCC 當筆轉台幣 */
		else if (pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE && (pobTran->srBRec.inCode == _SALE_ || pobTran->srBRec.inCode == _PRE_COMP_))
		{
			uszUseDefaultBit = VS_TRUE;
		}
		/* 信用卡SmartPay */
		else if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
		{
			inPrintIndex = _REPORT_INDEX_ESC_NORMAL_FISC_027_;
		}
		/* Happy GO混合交易 */
		else if (pobTran->srBRec.uszHappyGoMulti == VS_TRUE)
		{
			inPrintIndex = _REPORT_INDEX_ESC_NORMAL_HG_MULTIPLE_027_;
		}
		/* 信用卡一般 */
		else
		{
			inPrintIndex = _REPORT_INDEX_ESC_NORMAL_027_;
		}
	}
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_061_GRAND_HOTEL_, _CUSTOMER_INDICATOR_SIZE_))
	{
		/* 信用卡DCC */
		if (pobTran->srBRec.uszDCCTransBit == VS_TRUE)
		{
			/* For Sale */
			if ((pobTran->srBRec.inCode == _SALE_ || pobTran->srBRec.inCode == _PRE_COMP_))
			{
				inPrintIndex = _REPORT_INDEX_ESC_NORMAL_DCC_FOR_SALE_061_;
			}
			/* 信用卡DCC Not For Sale */
			else
			{
				inPrintIndex = _REPORT_INDEX_ESC_NORMAL_DCC_NOT_FOR_SALE_061_;
			}
		}
		/* DCC 當筆轉台幣 */
		else if (pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE && (pobTran->srBRec.inCode == _SALE_ || pobTran->srBRec.inCode == _PRE_COMP_))
		{
			inPrintIndex = _REPORT_INDEX_ESC_NORMAL_DCC_CHANGE_TWD_061_;
		}
		else
		{
			uszUseDefaultBit = VS_TRUE;
		}
	}
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_043_BDAU9_, _CUSTOMER_INDICATOR_SIZE_))
	{
		inPrintIndex = _REPORT_INDEX_ESC_NORMAL_043_;
	}
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_005_FPG_, _CUSTOMER_INDICATOR_SIZE_))
	{
		if (pobTran->srBRec.uszHappyGoMulti == VS_TRUE)
		{
			inPrintIndex = _REPORT_INDEX_ESC_NORMAL_HG_MULTIPLE_005;
		}
		/* 信用卡一般 */
		else
		{
			inPrintIndex = _REPORT_INDEX_ESC_NORMAL_005_;
		}
	}
	else
	{
		uszUseDefaultBit = VS_TRUE;
	}
	
	/* 如果客製化下不需改變，就會跑Default */
	if (uszUseDefaultBit == VS_TRUE)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetShort_Receipt_Mode(szTemplate);
		if (memcmp(szTemplate, "Y", strlen("Y")) == 0)
		{
			/* 信用卡DCC */
			if (pobTran->srBRec.uszDCCTransBit == VS_TRUE)
			{
				/* For Sale */
				if ((pobTran->srBRec.inCode == _SALE_ || pobTran->srBRec.inCode == _PRE_COMP_))
				{
					inPrintIndex = _REPORT_INDEX_ESC_DCC_FOR_SALE_SMALL_;
				}
				/* 信用卡DCC Not For Sale */
				else
				{
					inPrintIndex = _REPORT_INDEX_ESC_DCC_NOT_FOR_SALE_SMALL_;
				}

			}
			/* DCC 當筆轉台幣 */
			else if (pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE && (pobTran->srBRec.inCode == _SALE_ || pobTran->srBRec.inCode == _PRE_COMP_))
			{
				inPrintIndex = _REPORT_INDEX_ESC_DCC_CHANGE_TWD_SMALL_;
			}
			/* 信用卡SmartPay */
			else if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				inPrintIndex = _REPORT_INDEX_ESC_FISC_SMALL_;
			}
			/* Happy GO混合交易 */
			else if (pobTran->srBRec.uszHappyGoMulti == VS_TRUE)
			{
				inPrintIndex = _REPORT_INDEX_ESC_HG_MULTIPLE_SMALL_;
			}
			/* 信用卡縮小一般 */
			else
			{
				inPrintIndex = _REPORT_INDEX_ESC_SMALL_;
			}
		}
		else if (!memcmp(szTemplate, _SHORT_RECEIPT_S_, strlen(_SHORT_RECEIPT_S_))	||
			 !memcmp(szTemplate, _SHORT_RECEIPT_U_, strlen(_SHORT_RECEIPT_U_)))
		{
			/* 信用卡DCC */
			if (pobTran->srBRec.uszDCCTransBit == VS_TRUE)
			{
				/* For Sale */
				if ((pobTran->srBRec.inCode == _SALE_ || pobTran->srBRec.inCode == _PRE_COMP_))
				{
					inPrintIndex = _REPORT_INDEX_ESC_DCC_FOR_SALE_SMALL_S_;
				}
				/* 信用卡DCC Not For Sale */
				else
				{
					inPrintIndex = _REPORT_INDEX_ESC_DCC_NOT_FOR_SALE_SMALL_S_;
				}

			}
			/* DCC 當筆轉台幣 */
			else if (pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE && (pobTran->srBRec.inCode == _SALE_ || pobTran->srBRec.inCode == _PRE_COMP_))
			{
				inPrintIndex = _REPORT_INDEX_ESC_DCC_CHANGE_TWD_SMALL_S_;
			}
			/* 信用卡SmartPay */
			else if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				inPrintIndex = _REPORT_INDEX_ESC_FISC_SMALL_S_;
			}
			/* Happy GO混合交易 */
			else if (pobTran->srBRec.uszHappyGoMulti == VS_TRUE)
			{
				inPrintIndex = _REPORT_INDEX_ESC_HG_MULTIPLE_SMALL_S_;
			}
			/* 信用卡縮小一般 */
			else
			{
				inPrintIndex = _REPORT_INDEX_ESC_SMALL_S_;
			}
		}
		else
		{
			/* 信用卡DCC */
			if (pobTran->srBRec.uszDCCTransBit == VS_TRUE)
			{
				/* For Sale */
				if ((pobTran->srBRec.inCode == _SALE_ || pobTran->srBRec.inCode == _PRE_COMP_))
				{
					inPrintIndex = _REPORT_INDEX_ESC_NORMAL_DCC_FOR_SALE_;
				}
				/* 信用卡DCC Not For Sale */
				else
				{
					inPrintIndex = _REPORT_INDEX_ESC_NORMAL_DCC_NOT_FOR_SALE_;
				}

			}
			/* DCC 當筆轉台幣 */
			else if (pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE && (pobTran->srBRec.inCode == _SALE_ || pobTran->srBRec.inCode == _PRE_COMP_))
			{
				inPrintIndex = _REPORT_INDEX_ESC_NORMAL_DCC_CHANGE_TWD_;
			}
			/* 信用卡SmartPay */
			else if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				inPrintIndex = _REPORT_INDEX_ESC_NORMAL_FISC_;
			}
			/* Happy GO混合交易 */
			else if (pobTran->srBRec.uszHappyGoMulti == VS_TRUE)
			{
				inPrintIndex = _REPORT_INDEX_ESC_NORMAL_HG_MULTIPLE_;
			}
			/* 信用卡一般 */
			else
			{
				inPrintIndex = _REPORT_INDEX_ESC_NORMAL_;
			}
		}
	}
	
	if (ginDebug == VS_TRUE)
	{
		char	szDebugMsg[100 + 1];

		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "PrintIndex : %d", inPrintIndex);
		inLogPrintf(AT, szDebugMsg);
	}
	/* [BUG] 移除檔案同步API SYNC()指令 
    * [20260506] 加上ESC選擇的記錄 */
	vdUtility_SYSFIN_LogMessage(AT, "ESC PrintIndex : %d", inPrintIndex);
	
	/* 初始化，防呆送錯電子簽單 */
	glnESC_PokaYoke_Amt = 0;
	
	/*inNCCC_ESC_Open_File的參數三 inDelete 控制是否刪除檔案*/
	/* 刪除檔案並開啟新檔 */
	inNCCC_ESC_Open_File(&ulHandle, _ESC_FILE_RECEIPT_, VS_TRUE);
	
	/* E1開頭 */
	memset(szE1Temp, 0x00, sizeof(szE1Temp));
        sprintf(szE1Temp, "%s\x0D\x0A", "[@@]");
        if (inNCCC_ESC_Append_E1(&ulHandle, &szE1Temp[0], 6) != VS_SUCCESS)
	{
		vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_Make_E1Data inNCCC_ESC_Append_E1");
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "E1 Head doesn't make");
			inLogPrintf(AT, szDebugMsg);
		}
		
		if (inNCCC_ESC_Close_File(&ulHandle) != VS_SUCCESS)
		{
			vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_Make_E1Data inNCCC_ESC_Close_File close failed");
			return (VS_ERROR);
		}
		
		return (VS_ERROR);
	}
	
	/* Bank & Merchant Logo */
	if (srESCReceiptType[inPrintIndex].inLogo != NULL)
	{
	        memset(szE1Temp, 0x00, sizeof(szE1Temp));
                inDataLen = srESCReceiptType[inPrintIndex].inLogo(pobTran, &szE1Temp[0]);
		
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "inDataLen: %d", inDataLen);
			inLogPrintf(AT, szDebugMsg);
		}

		if (inDataLen > 0)
		        inNCCC_ESC_Append_E1(&ulHandle, szE1Temp, inDataLen);
		else
		{
			vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_Make_E1Data .inLogo_failed_index(%d)", index);
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "E1 logo doesn't make");
				inLogPrintf(AT, szDebugMsg);
			}
			
		}
		
        }
	
	/* TID & MID */
	if (srESCReceiptType[inPrintIndex].inTop != NULL)
	{
		memset(szE1Temp, 0x00, sizeof(szE1Temp));
                inDataLen = srESCReceiptType[inPrintIndex].inTop(pobTran, szE1Temp);

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "inDataLen: %d", inDataLen);
			inLogPrintf(AT, szDebugMsg);
		}
		
		if (inDataLen > 0)
		        inNCCC_ESC_Append_E1(&ulHandle, szE1Temp, inDataLen);
		else
		{
			vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_Make_E1Data .inTop_failed_index(%d)", index);
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "E1 TID doesn't make");
				inLogPrintf(AT, szDebugMsg);
			}
			
		}
		
	}

	/* Data */
	if (srESCReceiptType[inPrintIndex].inData != NULL)
	{
	        memset(szE1Temp, 0x00, sizeof(szE1Temp));
                inDataLen = srESCReceiptType[inPrintIndex].inData(pobTran, szE1Temp);

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "inDataLen: %d", inDataLen);
			inLogPrintf(AT, szDebugMsg);
		}
		
		if (inDataLen > 0)
		        inNCCC_ESC_Append_E1(&ulHandle, szE1Temp, inDataLen);
		else
		{
			vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_Make_E1Data .inData_failed_index(%d)", index);
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "E1 Print Data doesn't make");
				inLogPrintf(AT, szDebugMsg);
			}
			
		}
		
	}

	/* Process CUP Flow (START) */
	if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
	{
		/* CUP Amount */
		if (srESCReceiptType[inPrintIndex].inCUPAmount != NULL)
		{
		        memset(szE1Temp, 0x00, sizeof(szE1Temp));
                        inDataLen = srESCReceiptType[inPrintIndex].inCUPAmount(pobTran, szE1Temp);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "inDataLen: %d", inDataLen);
				inLogPrintf(AT, szDebugMsg);
			}
			
        		if (inDataLen > 0)
        		        inNCCC_ESC_Append_E1(&ulHandle, szE1Temp, inDataLen);
			else
			{
				vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_Make_E1Data .inCUPAmount_failed_index(%d)", index);
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "E1 CUP doesn't make");
					inLogPrintf(AT, szDebugMsg);
				}
				
			}
			
	        }
		
	}
	else
	{
		/* Amount */
		if (srESCReceiptType[inPrintIndex].inAmount != NULL)
		{
			memset(szE1Temp, 0x00, sizeof(szE1Temp));
			inDataLen = srESCReceiptType[inPrintIndex].inAmount(pobTran, szE1Temp);

			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "inDataLen: %d", inDataLen);
				inLogPrintf(AT, szDebugMsg);
			}
			
			if (inDataLen > 0)
				inNCCC_ESC_Append_E1(&ulHandle, szE1Temp, inDataLen);
			else
			{
				vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_Make_E1Data .inAmount_failed_index(%d)", index);
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "E1 Amount doesn't make");
					inLogPrintf(AT, szDebugMsg);
				}
				
			}
			
		}

		/* Installment & Redemption */
		if (pobTran->srBRec.uszInstallmentBit == VS_TRUE)
		{
			if (srESCReceiptType[inPrintIndex].inInstallment != NULL)
			{
				memset(szE1Temp, 0x00, sizeof(szE1Temp));
				inDataLen = srESCReceiptType[inPrintIndex].inInstallment(pobTran, szE1Temp);

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "inDataLen: %d", inDataLen);
					inLogPrintf(AT, szDebugMsg);
				}
				
				if (inDataLen > 0)
					inNCCC_ESC_Append_E1(&ulHandle, szE1Temp, inDataLen);
				else
				{
					vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_Make_E1Data .inInstallment_failed_index(%d)", index);
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
						sprintf(szDebugMsg, "E1 Inst. doesn't make");
						inLogPrintf(AT, szDebugMsg);
					}
					
				}
				
			}
			
		}
		else if (pobTran->srBRec.uszRedeemBit == VS_TRUE)
		{
			if (srESCReceiptType[inPrintIndex].inRedemption != NULL)
			{
				memset(szE1Temp, 0x00, sizeof(szE1Temp));
				inDataLen = srESCReceiptType[inPrintIndex].inRedemption(pobTran, szE1Temp);

				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "inDataLen: %d", inDataLen);
					inLogPrintf(AT, szDebugMsg);
				}
				
				if (inDataLen > 0)
					inNCCC_ESC_Append_E1(&ulHandle, szE1Temp, inDataLen);
				else
				{
					vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_Make_E1Data .inRedemption_failed_index(%d)", index);
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
						sprintf(szDebugMsg, "E1 Redeem doesn't make");
						inLogPrintf(AT, szDebugMsg);
					}
					
				}
				
			}
			
		}
		
	}
	
	/* 防呆送錯電子簽單，紀錄簽單金額 */
	glnESC_PokaYoke_Amt = pobTran->srBRec.lnTxnAmount;
	
	/* OTHER資料 */
	if (srESCReceiptType[inPrintIndex].inHappyGo != NULL)
	{
		memset(szE1Temp, 0x00, sizeof(szE1Temp));
		inDataLen = srESCReceiptType[inPrintIndex].inHappyGo(pobTran, szE1Temp);

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "inDataLen: %d", inDataLen);
			inLogPrintf(AT, szDebugMsg);
		}

		if (inDataLen > 0)
			inNCCC_ESC_Append_E1(&ulHandle, szE1Temp, inDataLen);
		else
		{
			vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_Make_E1Data .inHappyGo_failed_index(%d)", index);
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "E1 HappyGo doesn't make");
				inLogPrintf(AT, szDebugMsg);
			}

		}

	}
	
	/* End */
	if (srESCReceiptType[inPrintIndex].inEnd != NULL)
	{
	        memset(szE1Temp, 0x00, sizeof(szE1Temp));
                inDataLen = srESCReceiptType[inPrintIndex].inEnd(pobTran, szE1Temp);

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "inDataLen: %d", inDataLen);
			inLogPrintf(AT, szDebugMsg);
		}
		
		if (inDataLen > 0)
		        inNCCC_ESC_Append_E1(&ulHandle, szE1Temp, inDataLen);
		else
		{
			vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_Make_E1Data .inEnd_failed_index(%d)", index);
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "E1 Print End doesn't make");
				inLogPrintf(AT, szDebugMsg);
			}
			
		}
		
        }

        memset(szE1Temp, 0x00, sizeof(szE1Temp));
        sprintf(szE1Temp, "%s\x0D\x0A", "[$$]");
        if (inNCCC_ESC_Append_E1(&ulHandle, szE1Temp, 6) != VS_SUCCESS)
	{
		vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_Make_E1Data inNCCC_ESC_Append_E1_failed");
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "E1 Tail doesn't make");
			inLogPrintf(AT, szDebugMsg);
		}
		
		if (inNCCC_ESC_Close_File(&ulHandle) != VS_SUCCESS)
		{
			vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_Make_E1Data inNCCC_ESC_Close_File");
			return (VS_ERROR);
		}
		
		return (VS_ERROR);
	}

        if (inNCCC_ESC_Close_File(&ulHandle) != VS_SUCCESS)
	{
		vdUtility_SYSFIN_LogMessage(AT, "inNCCC_ESC_Make_E1Data inNCCC_ESC_Close_File");
		return (VS_ERROR);
	}
	
	/* ESC上傳防呆機制測試使用 */
	if (glnESC_PokaYoke_TestAmt > 0)
	{
		pobTran->srBRec.lnTxnAmount = lnOrgTxnAmt;
	}
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_Make_E1Data() END!");
	
	return (VS_SUCCESS);
}

/* 檔案功能 (START) */
/*
Function        :inNCCC_ESC_Open_File
Date&Time       :2016/4/20 下午 4:20
Describe        :Open E1
*/
int inNCCC_ESC_Open_File(unsigned long *ulHandle, char *szFileName, int inDelete)
{
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_Open_File() START!");
	
        /* 移除原本的檔案 */
        if (inDelete == VS_TRUE)
        {
                inFILE_Delete((unsigned char*)szFileName);
        }

	/*
	   O_CREAT         若打開的檔案並不存在,則核心就會建立一個新的檔案. 若檔案已經存在則不會有影響
	   O_APPEND        在寫入之前,檔案的位置會指向在檔案的結尾,新寫入的內容就會附加在檔案結尾之後
	   O_RDWR          允許讀取與寫入模式
	   O_TRUNC         為假如檔案已經存在, 且是一個普通檔案,打開模式又是可寫(即檔案是用O_RDWR 或O_WRONLY 模式打開的)
	                   就把檔案的長度設置為零, 丟棄其中的現有內容.
	*/

	if (inFILE_Create(ulHandle, (unsigned char*)szFileName) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_Open_File() END!");
	
        return (VS_SUCCESS);
}

/*
Function        :inNCCC_ESC_Close_File
Date&Time       :2016/4/20 下午 4:20
Describe        :Close E1
*/
int inNCCC_ESC_Close_File(unsigned long *ulHandle)
{
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_Close_File() START!");
	
	if (inFILE_Close(ulHandle) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_Close_File() END!");
	
        return (VS_SUCCESS);
}

/*
Function        :inNCCC_ESC_Append_E1
Date&Time       :2016/4/18 上午 11:00
Describe        :組E1
*/
int inNCCC_ESC_Append_E1(unsigned long *ulESCHandle, char *szE1Temp, int inDataLen)
{
	int	inRetVal;
	char	szBig5[2048 + 1];
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_Append_E1() START!");
	
	

        if (ulESCHandle <= 0)
                return (VS_ERROR);

        /* 移到檔案最後 */
        inRetVal = inFILE_Seek(*ulESCHandle, 0L, SEEK_END);
	if (inRetVal != VS_SUCCESS)
        {
		return (VS_ERROR);
	}
	
	/* 轉成Big5 */
	memset(szBig5, 0x00, sizeof(szBig5));
	inFunc_UTF8toBig5(szBig5, szE1Temp);
		
        inRetVal = inFILE_Write(ulESCHandle, (unsigned char*)szBig5, strlen(szBig5));
	
        if (inRetVal != VS_SUCCESS)
        {
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inNCCC_ESC_Append_E1() failed!");
		}
        	inFILE_Close(ulESCHandle);
		
        	return (VS_ERROR);
        }
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_Append_E1() END!");

        return (VS_SUCCESS);
}

/* 檔案功能 (END) */

/*
Function        :inESC_PRINT_LOGO
Date&Time       :2016/4/18 下午 1:10
Describe        :組E1 LOGO部份
*/
int inNCCC_ESC_PRINT_LOGO(TRANSACTION_OBJECT *pobTran, char *szPackData)
{
	int	i = 0;
	int     inDataLen = 0;
        char    szPrintBuf[200 + 1];		/* inESC_PRINT_LOGO裡用的buffer，每組完一個東西就清空一次 */
	char	szPrtMerchantLogo[2 + 1];
	char	szPrtMerchantName[2 + 1];
	char    szTempBuff[100 + 1] = {0};	/* 暫放get出來的值 */

	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_PRINT_LOGO() START!");
	
	/* Print NCCC LOGO 384*60 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "G::%s\x0D\x0A", _ESC_BMP_NCCC_LOGO_);
	/* 丟給外面的Buffer */
        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
	/* 位置移到szPackData最後面，以便放其他欄位 */
        inDataLen += strlen(szPrintBuf);

	/* Print MERCHANT LOGO 384*180 */
	memset(szPrtMerchantLogo, 0x00, sizeof(szPrtMerchantLogo));
	inGetPrtMerchantLogo(szPrtMerchantLogo);
		
	if (memcmp(&szPrtMerchantLogo[0], "Y", 1) == 0)
	{
	        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "G::%s\x0D\x0A", _ESC_BMP_MERCHANT_LOGO_);
                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
	}

	/* Print MERCHANT NAME 384*90 */
	/* 【需求單-113278】新增ESC簽單上傳表頭資料需求 by Russell 2025/5/21 下午 5:16 */
	memset(szPrtMerchantName, 0x00, sizeof(szPrtMerchantName));
	inGetPrtMerchantName(szPrtMerchantName);
	if (memcmp(&szPrtMerchantName[0], "Y", 1) == 0)
	{	/* 這邊inLoadBig5NameTRec兩次 分別是商店LOGO 和 NAME ? */
		for (i = 0; i < 2; i++)
		{
			if (inLoadBig5NameTRec(i) != VS_SUCCESS)
				break;
			
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTempBuff, 0x00, sizeof(szTempBuff));
			inGetMerchant_Name(szTempBuff);
			sprintf(szPrintBuf, "13::%s\x0D\x0A", szTempBuff);
			memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
		}
	}
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_PRINT_LOGO() END!");

	return (inDataLen);
}

/*
Function        :inNCCC_ESC_PRINT_TIDMID
Date&Time       :2016/4/18 下午 1:57
Describe        :組E1 TID、MID部份
*/
int inNCCC_ESC_PRINT_TIDMID(TRANSACTION_OBJECT *pobTran, char *szPackData)
{
        int     inDataLen = 0;
	char    szTempBuff[100 + 1];	/* 暫放get出來的值 */
	char	szPrintBuf[200 + 1];

	/* 商店代號 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTempBuff, 0x00, sizeof(szTempBuff));
	inGetMerchantID(szTempBuff);
	sprintf(szPrintBuf, "22::商店代號%s%s\x0D\x0A", _DATA_FLAG_, szTempBuff);

	memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 端末機代號 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTempBuff, 0x00, sizeof(szTempBuff));
	inGetTerminalID(szTempBuff);
	sprintf(szPrintBuf, "22::端末機代號%s%s\x0D\x0A", _DATA_FLAG_, szTempBuff);

	memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

        /* ============ */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	strcpy(szPrintBuf, "13::============================\x0D\x0A");

        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	return (inDataLen);
}

/*
Function        :inNCCC_ESC_PRINT_DATA
Date&Time       :2016/4/19 上午 10:58
Describe        :組E1 DATA部份
*/
int inNCCC_ESC_PRINT_DATA(TRANSACTION_OBJECT *pobTran, char *szPackData)
{
	int	i = 0;
	int	inDataLen = 0;
	int	inTempLen = 0;
	int	inUpperAmount = 0;
	int	inLowerAmount = 0;
	char	szPrintBuf[100 + 1] = {0}, szPrintBuf1[100 + 1] = {0}, szPrintBuf2[100 + 1] = {0}, szTemplate[100 + 1] = {0}, szTemplate1[100 + 1] = {0}, szTemplate2[100 + 1] = {0};
	char	szProductCodeEnable[1 + 1];
	char	szStore_Stub_CardNo_Truncate_Enable[2 + 1];
	char	szExamBit[10 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);

	/* 城市別 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", "城市別(City)");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 城市名 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetCityName(szTemplate);
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", szTemplate);

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 卡別 & 檢查碼 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	/* 大高卡只能顯示有效期 */
	if (pobTran->srBRec.uszTakaTransBit == VS_TRUE)
	{
		sprintf(szPrintBuf, "29::%s%s%s\x0D\x0A", "卡別(Card Type)", _DATA_FLAG_, "有效期(Exp. Date)");
	}
	else
	{
		sprintf(szPrintBuf, "29::%s%s%s\x0D\x0A", "卡別(Card Type)", _DATA_FLAG_, "檢查碼(Check No.)");
	}

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 卡別值 & 檢查碼值 */
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	/* 【需求單 - 106349】自有品牌判斷需求 */
	/* unpack電文就存存CardLabel modify by LingHsiung 2020/2/14 上午 11:07 */
	strcpy(szTemplate1, pobTran->srBRec.szCardLabel);

	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/28 下午 2:42 */
	if (pobTran->srBRec.uszTakaTransBit == VS_TRUE)
	{
		sprintf(szTemplate2, "%.2s/'%.2s", pobTran->srBRec.szExpDate + 2, pobTran->srBRec.szExpDate);
	}
	else
	{
		if (strlen(pobTran->srBRec.szCheckNO) > 0)
		{
			strcpy(szTemplate2, pobTran->srBRec.szCheckNO);
		}
		else
		{
			inCARD_ExpDateEncryptAndDecrypt(pobTran, "", szTemplate2, _EXP_ENCRYPT_);
		}
	}

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "29::%s%s%s\x0D\x0A", szTemplate1, _DATA_FLAG_, szTemplate2);

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 卡號掩飾 NCCC 規格以為準 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", "卡號(Card No.)");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	memset(szTemplate, 0x00, sizeof(szTemplate));
	strcpy(szTemplate, pobTran->srBRec.szPAN);
	
	/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
	/* 商店聯卡號遮掩 */
	memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
	inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
	/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/8 下午 1:55 */
	if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
	{
		/* Uny交易不受TMS開關限制，且獲取卡號時已遮掩 */
	}
	else if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0 && pobTran->srBRec.uszTxNoCheckBit == VS_TRUE)
	{
		/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
		for (i = 6; i < (strlen(szTemplate) - 4); i ++)
			szTemplate[i] = 0x2A;
	}

	/* 過卡方式 */
	if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
	{
		if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
			strcat(szTemplate, "(W)");
		else
			strcat(szTemplate, "(C)");
	}
	else if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
	{
		strcat(szTemplate, "(Q)");
	}
	else if (pobTran->srBRec.uszCUPEMVQRCodeBit == VS_TRUE)
	{
		strcat(szTemplate, "(Q)");
	}
	else
	{
		if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
			strcat(szTemplate,"(C)");
		else if (pobTran->srBRec.uszMobilePayBit == VS_TRUE)
			strcat(szTemplate, "(T)");
		else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
			strcat(szTemplate, "(W)");
		else
		{
			if (pobTran->srBRec.uszManualBit == VS_TRUE)
			{
				/* 【需求單 - 105244】端末設備支援以感應方式進行退貨交易 */
				/* 電文轉Manual Keyin但是簽單要印感應的W */
				if (pobTran->srBRec.uszRefundCTLSBit == VS_TRUE)
					strcat(szTemplate, "(W)");
				else
					strcat(szTemplate,"(M)");
			}
			else
				strcat(szTemplate,"(S)");
		}
	}

       	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", szTemplate);

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 主機別 & 交易類別 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", "主機別/交易類別(Host/Trans. Type)");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 主機別值 & 交易類別值*/
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	inGetHostLabel(szTemplate);
	inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);

	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	sprintf(szPrintBuf1, "%s %s %s", szTemplate, szTemplate1, szTemplate2);
	
	if (strlen(szTemplate1) > 0)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "11::%s\x0D\x0A", szPrintBuf1);

		memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
	}
		
	
	/* 批次號碼(Batch No.) & 授權碼 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "29::%s%s%s\x0D\x0A", "批次號碼(Batch No.)", _DATA_FLAG_, "授權碼(Auth Code)");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%03ld", pobTran->srBRec.lnBatchNum);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "29::%s%s%s\x0D\x0A", szTemplate, _DATA_FLAG_, pobTran->srBRec.szAuthCode);

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	/* 回覆碼 */
	if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
	{
	        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "11::%s\x0D\x0A", "回覆碼(Resp. Code)");

                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);

		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(&szTemplate[0], &pobTran->srBRec.szRespCode[0], 2);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "11::%s\x0D\x0A", szTemplate);

                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
	}

	/* 日期 & 時間 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", "日期/時間(Date/Time)");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 日期值 時間值 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%.4s/%.2s/%.2s %.2s:%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6], &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
		
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "14::%s\x0D\x0A", szTemplate);

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 序號 & 調閱編號 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "29::%s%s%s\x0D\x0A", "序號(Ref. No.)", _DATA_FLAG_, "調閱編號(Inv. No.)");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 序號值 & 調閱編號值 */
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	inFunc_PAD_ASCII(szTemplate1, pobTran->srBRec.szRefNo, ' ', 25, _PADDING_RIGHT_);

	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	sprintf(szTemplate2, "%06ld", pobTran->srBRec.lnOrgInvNum);

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "29::%s%s%s\x0D\x0A", szTemplate1, _DATA_FLAG_, szTemplate2);

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 櫃號 */
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	inGetStoreIDEnable(szTemplate1);
	if ((memcmp(&szTemplate1[0], "Y", 1) == 0) && (strlen(pobTran->srBRec.szStoreID) > 0))
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_026_TAKA_, _CUSTOMER_INDICATOR_SIZE_) ||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
		{
			sprintf(szPrintBuf, "11::%s\x0D\x0A", "品群碼(Store ID)");
		}
		else
		{
			sprintf(szPrintBuf, "11::%s\x0D\x0A", "櫃號(Store ID)");
		}

		memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
		
		/* 櫃號值 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
		{
			/* 原品群碼*/
			sprintf(szTemplate1, "%s", pobTran->srBRec.szStoreID);
			
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "11::%s\x0D\x0A", szTemplate1);

			memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
			
			/* hardcode */
			char	szPOS_ID[6 + 1] = {0};
			memset(szPOS_ID, 0x00, sizeof(szPOS_ID));
			inGetPOS_ID(szPOS_ID);
			
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			memcpy(szTemplate1, _HARDCODE_STORE_ID_CUS_021_TAKAWEL_, 12);
			memcpy(&szTemplate1[12], szPOS_ID, 6);
		}
		else
		{
			sprintf(szTemplate1, "%s", pobTran->srBRec.szStoreID);
		}

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "11::%s\x0D\x0A", szTemplate1);

		memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
	}
	
	/* 產品代碼 */
	inGetProductCodeEnable(szProductCodeEnable);
	if (memcmp(szProductCodeEnable, "Y", 1) == 0)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "11::%s\x0D\x0A", "產品代碼(Product Code)");

                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "11::%s\x0D\x0A", pobTran->srBRec.szProductCode);

                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
	}
	
        /* 【需求單-113260】MasterCard機票交易需求 by Russell 2025/3/3 上午 11:55 */
        /* 取消維持原交易不列印機票資訊 */
        if (inNCCC_Func_Flight_Ticket_Allow_Print(pobTran) == VS_TRUE)
        {
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "27::%s%s%s\x0D\x0A", "出發地機場(departure)", _DATA_FLAG_, pobTran->srBRec.szFlightTicketPDS0523);

                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "27::%s%s%s\x0D\x0A", "目的地機場(arrival)", _DATA_FLAG_, pobTran->srBRec.szFlightTicketPDS0524);

                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);

                memset(szTemplate2, 0x00, sizeof(szTemplate2));
                if (strlen(pobTran->srBRec.szFlightTicketPDS0530) > 0)
                {
                        sprintf(szTemplate2, "%d", atoi(pobTran->srBRec.szFlightTicketPDS0530));
                }
                else
                {
                        sprintf(szTemplate2, "%s", " ");
                }
                sprintf(szPrintBuf, "%s", szTemplate2);
                
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "27::%s%s%s\x0D\x0A", "航班號碼(Flight No.)", _DATA_FLAG_, szTemplate2);

                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
                
                memset(szTemplate1, 0x00, sizeof(szTemplate1));
		inNCCC_Func_Get_Flight_Ticket_Txn_Category_PrintName(pobTran, szTemplate1);

		if (!memcmp(szTemplate1, _FLIGHT_TICKET_TXN_CATEGORY_PRINT_BAGGAGE_CHARGE_, max(strlen(szTemplate1), strlen(_FLIGHT_TICKET_TXN_CATEGORY_PRINT_BAGGAGE_CHARGE_)))		||
		    !memcmp(szTemplate1, _FLIGHT_TICKET_TXN_CATEGORY_PRINT_PREPAID_TICKET_, max(strlen(szTemplate1), strlen(_FLIGHT_TICKET_TXN_CATEGORY_PRINT_PREPAID_TICKET_)))		||
		    !memcmp(szTemplate1, _FLIGHT_TICKET_TXN_CATEGORY_PRINT_MISCELLANEOUS_CHARGE_ , max(strlen(szTemplate1), strlen(_FLIGHT_TICKET_TXN_CATEGORY_PRINT_MISCELLANEOUS_CHARGE_)))	||
		    !memcmp(szTemplate1, _FLIGHT_TICKET_TXN_CATEGORY_PRINT_TICKET_REISSUE_, max(strlen(szTemplate1), strlen(_FLIGHT_TICKET_TXN_CATEGORY_PRINT_TICKET_REISSUE_))))
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "27::%s%s%s\x0D\x0A", "交易類型(transaction category)", _DATA_FLAG_, " ");

			memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "25::%s%s%s\x0D\x0A", " ", _DATA_FLAG_, szTemplate1);

			memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
		}
		else
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "27::%s%s%s\x0D\x0A", "交易類型(transaction category)", _DATA_FLAG_, szTemplate1);

			memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
		}
        }
        
	/* TC */
	/* 驗測開關 */
	memset(szExamBit, 0x00, sizeof(szExamBit));
	inGetExamBit(szExamBit);
	/* Uny交易 */
	/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/8 下午 1:55 */
	if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
	{
		/* 取消或退貨，只印數字交易碼 */
		/* 最小字體 */
		if (pobTran->srBRec.uszVOIDBit == VS_TRUE ||
		    pobTran->srBRec.inCode == _REFUND_ || pobTran->srBRec.inCode == _INST_REFUND_ || pobTran->srBRec.inCode == _REDEEM_REFUND_)
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			sprintf(szTemplate1, "交易碼 %s", pobTran->srBRec.szUnyTransCode);
			sprintf(szPrintBuf, "11::%s\x0D\x0A", szTemplate1);

			memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
		}
		/* 正向交易，要印條碼和數字 */
		else
		{

			if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
			{
				/* 一維條碼 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%s", pobTran->srBRec.szUnyTransCode);
				sprintf(szPrintBuf, "U::%s\x0D\x0A", szTemplate1);
				
				memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);

				/* 數字 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "交易碼(退貨請掃描) %s", pobTran->srBRec.szUnyTransCode);
				sprintf(szPrintBuf, "11::%s\x0D\x0A", szTemplate1);

				memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);
			}
			else
			{
				/* 一維條碼 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%s", pobTran->srBRec.szUnyTransCode);
				sprintf(szPrintBuf, "U::%s\x0D\x0A", szTemplate1);

				memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);

				/* 數字 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "交易碼 %s", pobTran->srBRec.szUnyTransCode);
				sprintf(szPrintBuf, "11::%s\x0D\x0A", szTemplate1);

				memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);
			}
		}
	}
	/* 實體卡交易和掃碼交易互斥 */
	else
	{
		if (pobTran->srBRec.inChipStatus == _EMV_CARD_ || pobTran->srBRec.uszContactlessBit == VS_TRUE)
		{
			/* 感應磁條 */
			if (!memcmp(pobTran->srBRec.szAuthCode, "VLP", 3)				|| 
			    !memcmp(pobTran->srBRec.szAuthCode, "JCB", 3)				||
			     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE		|| 
			     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_64_NEWJSPEEDY_MSD		||
			     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE)
			{
				/* 商店聯卡號遮掩 */
				memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
				inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
				if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
				    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
				    strlen(pobTran->srBRec.szTxnNo) > 0)
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "11::%s\x0D\x0A", "交易編號(Transaction No.):");
					memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
					inDataLen += strlen(szPrintBuf);

					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "11::%s\x0D\x0A", pobTran->srBRec.szTxnNo);
					memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
					inDataLen += strlen(szPrintBuf);
				}
			}
			else
			{
				if (pobTran->srBRec.inPrintOption == _PRT_MERCH_)
				{
					if (pobTran->srEMVRec.in9F26_ApplCryptogramLen > 0)
					{
						memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
						sprintf(szPrintBuf1, "%02X%02X%02X%02X%02X%02X%02X%02X",
										pobTran->srEMVRec.usz9F26_ApplCryptogram[0],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[1],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[2],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[3],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[4],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[5],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[6],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[7]);

						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "23::TC%s%s\x0D\x0A", _DATA_FLAG_, szPrintBuf1);

						memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
						inDataLen += strlen(szPrintBuf);
					}

					/* 商店聯卡號遮掩 */
					memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
					inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
					if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
					    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
					    strlen(pobTran->srBRec.szTxnNo) > 0)
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "11::%s\x0D\x0A", "交易編號(Transaction No.):");
						memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
						inDataLen += strlen(szPrintBuf);

						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "11::%s\x0D\x0A", pobTran->srBRec.szTxnNo);
						memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
						inDataLen += strlen(szPrintBuf);
					}

					/* M/C交易列印AP Lable (START) */
					if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)))
					{
						if (pobTran->srEMVRec.in50_APLabelLen > 0)
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							strcat(szPrintBuf, "AP Label:");
							memset(szTemplate, 0x00, sizeof(szTemplate));
							strcpy(szTemplate, (char*)pobTran->srEMVRec.usz50_APLabel);
							inUpperAmount = inFunc_Get_UpperCase_Char(szTemplate);
							inLowerAmount = strlen(szTemplate) - inUpperAmount;
							inTempLen =  _ESC_AP_LABEL_TOTAL_SPACE_ - (_ESC_UPPER_MULTIPLE_ * inUpperAmount + _ESC_LOWER_MULTIPLE_ * inLowerAmount);
							for (i = 0; i < inTempLen; i++)
							{
								strcat(szPrintBuf, " ");
							}
							strcat(szPrintBuf, szTemplate);
						}
						else
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							strcat(szPrintBuf, "AP Label:");
							strcat(szPrintBuf, "\x20");
						}
					}
					/* M/C交易列印AP Lable (END) */
					else if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)))
					{
						if (pobTran->srEMVRec.in50_APLabelLen > 0)
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							strcat(szPrintBuf, "AP Label:");
							memset(szTemplate, 0x00, sizeof(szTemplate));
							strcpy(szTemplate, (char*)pobTran->srEMVRec.usz50_APLabel);
							inUpperAmount = inFunc_Get_UpperCase_Char(szTemplate);
							inLowerAmount = strlen(szTemplate) - inUpperAmount;
							inTempLen =  _ESC_AP_LABEL_TOTAL_SPACE_ - (_ESC_UPPER_MULTIPLE_ * inUpperAmount + _ESC_LOWER_MULTIPLE_ * inLowerAmount);
							for (i = 0; i < inTempLen; i++)
							{
								strcat(szPrintBuf, " ");
							}
							strcat(szPrintBuf, szTemplate);
						}
						else
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							strcat(szPrintBuf, "AP Label:");
							strcat(szPrintBuf, "\x20");
						}
					}
					/* 驗測要印AP Label 和 AID */
					else if (szExamBit[0] == '1')
					{
						if (pobTran->srEMVRec.in50_APLabelLen > 0)
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							strcat(szPrintBuf, "AP Label:");
							memset(szTemplate, 0x00, sizeof(szTemplate));
							strcpy(szTemplate, (char*)pobTran->srEMVRec.usz50_APLabel);
							inUpperAmount = inFunc_Get_UpperCase_Char(szTemplate);
							inLowerAmount = strlen(szTemplate) - inUpperAmount;
							inTempLen =  _ESC_AP_LABEL_TOTAL_SPACE_ - (_ESC_UPPER_MULTIPLE_ * inUpperAmount + _ESC_LOWER_MULTIPLE_ * inLowerAmount);
							for (i = 0; i < inTempLen; i++)
							{
								strcat(szPrintBuf, " ");
							}
							strcat(szPrintBuf, szTemplate);
						}
						else
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							strcat(szPrintBuf, "AP Label:");
							strcat(szPrintBuf, "\x20");
						}
					}
					else
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						strcat(szPrintBuf, "\x20");
						strcat(szPrintBuf, "\x20");
					}
					memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
					sprintf(szPrintBuf2, szPrintBuf);
					
					/* AID */
					if (strlen(pobTran->srBRec.szCUP_EMVAID) > 0)
					{
						memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
						sprintf(szPrintBuf1, "AID:%s", pobTran->srBRec.szCUP_EMVAID);
						inFunc_PAD_ASCII(szPrintBuf1, szPrintBuf1, ' ', _ESC_AID_TOTAL_LEN_, _PADDING_RIGHT_);
						
						if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
						{
							/* CUP晶片要印 */
							if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
							else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)) == 0)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
							/* 【需求單-109121】Mastercard交易簽單列印AID需求 by Russell 2020/7/23 下午 4:23 */
							else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)) == 0)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
							/* 【需求單-111290】VISA置換Logo、DISCOVER簽單新增AID欄位、AE簽單新增AID和AP Label欄位需求 by Russell 2022/12/29 下午 5:57 */
							else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)) == 0 &&
								pobTran->srBRec.uszUCARDTransBit != VS_TRUE)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
							/* 驗測要印AP Label 和 AID */
							else if (szExamBit[0] == '1')
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
						}
						/* 銀聯閃付 */
						else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
						{
							if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
							/* AE晶片要印出AID */
							else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_50_EXPRESSSPAY || 
								 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
							/* 【需求單-109121】Mastercard交易簽單列印AID需求 by Russell 2020/7/23 下午 4:23 */
							else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE || 
								 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_21_PAYPASS_MCHIP)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
							else if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_42_DPAS	|| pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_43_DISCOVER_DPAS_MAG_STRIPE) &&
								pobTran->srBRec.uszUCARDTransBit != VS_TRUE)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
							/* 驗測要印AP Label 和 AID */
							else if (szExamBit[0] == '1')
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
						}
					}
				}
			}

		}
		else
		{
			/* 商店聯卡號遮掩 */
			/* 磁條卡列印交易編號 */
			memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
			inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
			if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
			    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
			    strlen(pobTran->srBRec.szTxnNo) > 0)
			{
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "11::%s\x0D\x0A", "交易編號(Transaction No.):");
				memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "11::%s\x0D\x0A", pobTran->srBRec.szTxnNo);
				memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);
			}
		}
	}
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        strcpy(szPrintBuf, "L::1\x0D\x0A");
        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	return (inDataLen);
}

/*
Function        :inNCCC_ESC_PRINT_CUP_AMOUNT
Date&Time       :2016/4/19 上午 10:58
Describe        :組E1 Amount部份
*/
int inNCCC_ESC_PRINT_CUP_AMOUNT(TRANSACTION_OBJECT *pobTran, char *szPackData)
{
	int     inDataLen = 0;
	char	szTemplate[42 + 1];
	char	szPrintBuf[100];
        long    lnTempTxnAmt = 0;

	if (pobTran->srBRec.uszVOIDBit == VS_TRUE)
	{
                /* (【需求單-113200】CASV3標準版支援銀聯優計劃專案 by Russell 2024/8/27 上午 11:03 */
                if (pobTran->srBRec.uszUPlanTransBit == VS_TRUE)
                {
                        lnTempTxnAmt = pobTran->srBRec.lnCUPUPlanDiscountedAmount + pobTran->srBRec.lnCUPUPlanPreferentialAmount;

                }
                else
                {
                        lnTempTxnAmt = pobTran->srBRec.lnTxnAmount;
                }
                
		/* 橫式 */
                /* 金額 */
                /* 取消退貨是正數 */
                if (pobTran->srBRec.inOrgCode == _CUP_REFUND_	||
		    pobTran->srBRec.inOrgCode == _CUP_MAIL_ORDER_REFUND_)
                {
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
			sprintf(szTemplate, "%ld",  lnTempTxnAmt);
			inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
                }
		else
		{
			/* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
			sprintf(szTemplate, "%ld",  (0 - lnTempTxnAmt));
			inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
		}
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "22::總計(Total)%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
                
                /* (【需求單-113200】CASV3標準版支援銀聯優計劃專案 by Russell 2024/8/27 上午 11:03 */
		if (pobTran->srBRec.uszUPlanTransBit == VS_TRUE)
		{
	                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        strcpy(szPrintBuf, "L::1\x0D\x0A");
                        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                        inDataLen += strlen(szPrintBuf);

			memset(szTemplate, 0x00, sizeof(szTemplate));
			if (pobTran->srBRec.inOrgCode == _CUP_REFUND_		||
			    pobTran->srBRec.inOrgCode == _CUP_MAIL_ORDER_REFUND_)
			{
				sprintf(szTemplate, "%ld", pobTran->srBRec.lnCUPUPlanDiscountedAmount);
			}
			else
			{
				sprintf(szTemplate, "%ld", 0 - pobTran->srBRec.lnCUPUPlanDiscountedAmount);
			}
			inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	                sprintf(szPrintBuf, "22::優惠後金額%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);
	                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
	                inDataLen += strlen(szPrintBuf);

	                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        strcpy(szPrintBuf, "L::1\x0D\x0A");
                        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                        inDataLen += strlen(szPrintBuf);

			memset(szTemplate, 0x00, sizeof(szTemplate));
			if (pobTran->srBRec.inOrgCode == _CUP_REFUND_		||
			    pobTran->srBRec.inOrgCode == _CUP_MAIL_ORDER_REFUND_)
			{
				sprintf(szTemplate, "%ld", pobTran->srBRec.lnCUPUPlanPreferentialAmount);
			}
			else
			{
				sprintf(szTemplate, "%ld", 0 - pobTran->srBRec.lnCUPUPlanPreferentialAmount);
			}
			inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	                sprintf(szPrintBuf, "22::優惠金額%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);
	                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
	                inDataLen += strlen(szPrintBuf);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			strcpy(szPrintBuf, "13::============================\x0D\x0A");
		        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
		        inDataLen += strlen(szPrintBuf);

	                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        sprintf(szPrintBuf, "11::%s\x0D\x0A", "備註欄(Reference)");
                        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                        inDataLen += strlen(szPrintBuf);

                        if (strlen(pobTran->srBRec.szUPlan_RemarksInformation) > 0)
                        {
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                strcpy(szPrintBuf, "L::1\x0D\x0A");
                                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                                inDataLen += strlen(szPrintBuf);
                        }
                        else
                        {
		                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	                        sprintf(szPrintBuf, "11:: \x0D\x0A");
	                        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
	                        inDataLen += strlen(szPrintBuf);
                        }

	                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        sprintf(szPrintBuf, "11::%s\x0D\x0A", "優惠券號(Coupon ID)");
                        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                        inDataLen += strlen(szPrintBuf);

	                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        sprintf(szPrintBuf, "11::%s\x0D\x0A", pobTran->srBRec.szUPlan_Coupon);
                        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                        inDataLen += strlen(szPrintBuf);
        	}
	}
	else
	{
		/* 初始化 */
		memset(szTemplate, 0x00, sizeof(szTemplate));

		/* 將NT$ ＋數字塞到szTemplate中來inpad，退貨要負數 */
		if (pobTran->srBRec.inCode == _CUP_REFUND_||
		    pobTran->srBRec.inOrgCode == _CUP_MAIL_ORDER_REFUND_)
		{
			sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
		}
		else
		{
			sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
		}
		
		inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "22::總計(Total)%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
                
                /* (【需求單-113200】CASV3標準版支援銀聯優計劃專案 by Russell 2024/8/27 上午 11:03 */
		if (pobTran->srBRec.uszUPlanTransBit == VS_TRUE)
		{
	                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        strcpy(szPrintBuf, "L::1\x0D\x0A");
                        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                        inDataLen += strlen(szPrintBuf);

			memset(szTemplate, 0x00, sizeof(szTemplate));
			if (pobTran->srBRec.inOrgCode == _CUP_REFUND_		||
			    pobTran->srBRec.inOrgCode == _CUP_MAIL_ORDER_REFUND_)
			{
				sprintf(szTemplate, "%ld", 0 - (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnCUPUPlanPreferentialAmount));
			}
			else
			{
				sprintf(szTemplate, "%ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnCUPUPlanPreferentialAmount));
			}
			inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	                sprintf(szPrintBuf, "22::優惠前金額%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);
	                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
	                inDataLen += strlen(szPrintBuf);

			memset(szTemplate, 0x00, sizeof(szTemplate));
			if (pobTran->srBRec.inOrgCode == _CUP_REFUND_		||
			    pobTran->srBRec.inOrgCode == _CUP_MAIL_ORDER_REFUND_)
			{
				sprintf(szTemplate, "%ld", 0 - pobTran->srBRec.lnCUPUPlanPreferentialAmount);
			}
			else
			{
				sprintf(szTemplate, "%ld", pobTran->srBRec.lnCUPUPlanPreferentialAmount);
			}
			inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	                sprintf(szPrintBuf, "22::優惠金額%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);
	                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
	                inDataLen += strlen(szPrintBuf);

			memset(szTemplate, 0x00, sizeof(szTemplate));
			if (pobTran->srBRec.inOrgCode == _CUP_REFUND_		||
			    pobTran->srBRec.inOrgCode == _CUP_MAIL_ORDER_REFUND_)
			{
				sprintf(szTemplate, "%ld", 0 - pobTran->srBRec.lnCUPUPlanDiscountedAmount);
			}
			else
			{
				sprintf(szTemplate, "%ld", pobTran->srBRec.lnCUPUPlanDiscountedAmount);
			}
			inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	                sprintf(szPrintBuf, "22::優惠後金額%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);
	                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
	                inDataLen += strlen(szPrintBuf);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			strcpy(szPrintBuf, "13::============================\x0D\x0A");
		        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
		        inDataLen += strlen(szPrintBuf);

	                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        sprintf(szPrintBuf, "11::%s\x0D\x0A", "備註欄(Reference)");
                        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                        inDataLen += strlen(szPrintBuf);

                        if (strlen(pobTran->srBRec.szUPlan_RemarksInformation) > 0)
                        {
		                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	                        sprintf(szPrintBuf, "11::%s\x0D\x0A", pobTran->srBRec.szUPlan_RemarksInformation);
	                        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
	                        inDataLen += strlen(szPrintBuf);
                        }
                        else
                        {
		                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	                        sprintf(szPrintBuf, "11:: \x0D\x0A");
	                        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
	                        inDataLen += strlen(szPrintBuf);
                        }

	                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        sprintf(szPrintBuf, "11::%s\x0D\x0A", "優惠券號(Coupon ID)");
                        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                        inDataLen += strlen(szPrintBuf);

	                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        sprintf(szPrintBuf, "11::%s\x0D\x0A", pobTran->srBRec.szUPlan_Coupon);
                        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                        inDataLen += strlen(szPrintBuf);
        	}
	}

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        strcpy(szPrintBuf, "L::1\x0D\x0A");
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 列印銀聯交易提示文字 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "G::%s\x0D\x0A", _ESC_BMP_CUP_LEGAL_LOGO_);
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        strcpy(szPrintBuf, "L::1\x0D\x0A");
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	return (inDataLen);
}

/*
Function        :inNCCC_ESC_PRINT_AMOUNT
Date&Time       :2016/4/19 下午 1:43
Describe        :組E1 Amount部份
*/
int inNCCC_ESC_PRINT_AMOUNT(TRANSACTION_OBJECT *pobTran, char *szPackData)
{
        int     inDataLen = 0;
	char	szPrintBuf[200 + 1], szTemplate[100 + 1];
	
	if (pobTran->srBRec.uszVOIDBit == VS_TRUE)
	{
		if (pobTran->srBRec.inOrgCode == _REFUND_	|| 
		    pobTran->srBRec.inOrgCode == _INST_REFUND_	|| 
		    pobTran->srBRec.inOrgCode == _REDEEM_REFUND_)
		{
		        /* 將NT$ ＋數字塞到szTemplate中來inpad */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
			inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, _PADDING_RIGHT_);

		        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        sprintf(szPrintBuf, "22::總計(Total)%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

                        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
                        inDataLen += strlen(szPrintBuf);
		}
		else
		{
			/* 將NT$ ＋數字塞到szTemplate中來inpad */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
			inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, _PADDING_RIGHT_);

		        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        sprintf(szPrintBuf, "22::總計(Total)%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

                        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
                        inDataLen += strlen(szPrintBuf);
		}
	}
	else
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTransFunc(szTemplate);
		if (szTemplate[6] == 'Y')	/* 檢查是否有開小費 */
		{
			/* 退貨金額為負數 */
			if (pobTran->srBRec.inCode == _REFUND_ || pobTran->srBRec.inCode == _INST_REFUND_ || pobTran->srBRec.inCode == _REDEEM_REFUND_)
			{
				 /* 初始化 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
				inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, _PADDING_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "22::總計(Total)%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

                                memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
                                inDataLen += strlen(szPrintBuf);
			}
			/* 預授不會有小費，所以拉出來 */
			else if (pobTran->srBRec.inCode == _PRE_AUTH_ || pobTran->srBRec.inCode == _PRE_COMP_)
			{
				/* 初始化 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
				inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, _PADDING_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "22::總計(Total)%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

                                memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
                                inDataLen += strlen(szPrintBuf);
			}
			else
			{
				/* 金額 */
				/* 初始化 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
				inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, _PADDING_RIGHT_);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "22::金額(Amount)%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

                                memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
                                inDataLen += strlen(szPrintBuf);

				/* lnTipAmount為0表示非小費 */
				if (pobTran->srBRec.lnTipTxnAmount == 0L)
				{
				        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				        sprintf(szPrintBuf, "22::小費(Tips)%s%s\x0D\x0A", _DATA_FLAG_, "_____________");

                                        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
                                        inDataLen += strlen(szPrintBuf);

                                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                        sprintf(szPrintBuf, "22::總計(Total)%s%s\x0D\x0A", _DATA_FLAG_, "_____________");

                                        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
                                        inDataLen += strlen(szPrintBuf);
				}
				else
				{
					/* 小費 */
					/* 初始化 */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

					/* 將NT$ ＋數字塞到szTemplate中來inpad */
					sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTipTxnAmount);
					inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, _PADDING_RIGHT_);

					/* 把前面的字串和數字結合起來 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "22::小費(Tips)%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

                                        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
                                        inDataLen += strlen(szPrintBuf);

					/* 總計 */
					/* 初始化 */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

					/* 將NT$ ＋數字塞到szTemplate中來inpad */
					sprintf(szTemplate, "%ld",  (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
					inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 15, _PADDING_RIGHT_);

					/* 把前面的字串和數字結合起來 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                        sprintf(szPrintBuf, "22::總計(Total)%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

                                        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
                                        inDataLen += strlen(szPrintBuf);
				}
			}
		}
		else
		{
			/* 初始化 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

			/* 將NT$ ＋數字塞到szTemplate中來inpad，退貨要負數 */
			if (pobTran->srBRec.inCode == _REFUND_ || pobTran->srBRec.inCode == _INST_REFUND_ || pobTran->srBRec.inCode == _REDEEM_REFUND_)
				sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
			else
				sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
			
			inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, VS_TRUE);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        sprintf(szPrintBuf, "22::總計(Total)%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

                        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
                        inDataLen += strlen(szPrintBuf);
		}
		
	}
		
	/* 斷行 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        strcpy(szPrintBuf, "L::1\x0D\x0A");
        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	return (inDataLen);
}

/*
Function        :inNCCC_ESC_PRINT_INST
Date&Time       :2016/4/19 下午 1:43
Describe        :組E1 分期
*/
int inNCCC_ESC_PRINT_INST(TRANSACTION_OBJECT *pobTran, char *szPackData)
{
	int	inDataLen = 0;
	char    szPrintBuf[84 + 1], szTemplate[42 + 1];

	if (pobTran->srBRec.uszInstallmentBit == VS_TRUE)
	{
		/* 分期期數 */
		/* 將NT$ ＋數字塞到szTemplate中來inpad */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%02ld 期", pobTran->srBRec.lnInstallmentPeriod);

		/* 把前面的字串和數字結合起來 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "22::分期期數%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

		memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);

		/* 首期金額 */
		/* 將NT$ ＋數字塞到szTemplate中來inpad */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		if (pobTran->srBRec.lnTipTxnAmount > 0L)
			sprintf(szTemplate, "%ld", (pobTran->srBRec.lnInstallmentDownPayment + pobTran->srBRec.lnTipTxnAmount));
		else
			sprintf(szTemplate, "%ld", (pobTran->srBRec.lnInstallmentDownPayment));
		inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, _PADDING_RIGHT_);

		/* 把前面的字串和數字結合起來 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "22::首期金額%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

		memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);

		/* 每期金額 */
		/* 將NT$ ＋數字塞到szTemplate中來inpad */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%ld", (pobTran->srBRec.lnInstallmentPayment));
		inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, _PADDING_RIGHT_);

		/* 把前面的字串和數字結合起來 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "22::每期金額%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

		memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);

		/* 分期手續費 */
		/* 將NT$ ＋數字塞到szTemplate中來inpad */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%ld", (pobTran->srBRec.lnInstallmentFormalityFee));
		inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, _PADDING_RIGHT_);

		/* 把前面的字串和數字結合起來 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "22::分期手續費%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

		memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
		
		/* 斷行 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		strcpy(szPrintBuf, "L::1\x0D\x0A");

		memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);

		/* 分期警語 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "G::%s\x0D\x0A", _ESC_BMP_LEGAL_LOGO_);
                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);

		/* 斷行 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		strcpy(szPrintBuf, "L::1\x0D\x0A");

		memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
	}
		
        return (inDataLen);
}

/*
Function        :inNCCC_ESC_PRINT_REDEEM
Date&Time       :2016/4/19 下午 5:54
Describe        :組E1 紅利
*/
int inNCCC_ESC_PRINT_REDEEM(TRANSACTION_OBJECT *pobTran, char *szPackData)
{
	int	inDataLen = 0;
        char    szPrintBuf[84 + 1], szTemplate[42 + 1], szTemplate1[42 + 1];

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));
		
	if (pobTran->srBRec.uszRedeemBit == VS_TRUE)
	{
		if (pobTran->srBRec.uszVOIDBit == VS_TRUE)
		{
			if (pobTran->srBRec.inOrgCode == _REDEEM_SALE_ || pobTran->srBRec.inOrgCode == _REDEEM_ADJUST_)
			{
				/* 支付金額 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", (0 - pobTran->srBRec.lnRedemptionPaidCreditAmount + pobTran->srBRec.lnTipTxnAmount));
				inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, _PADDING_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "22::支付金額%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

				memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);

				/* 紅利扣抵金額 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", (0 - (pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnRedemptionPaidCreditAmount)));
				inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, _PADDING_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "22::紅利扣抵金額%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

				memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);
				/* 紅利扣抵點數 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%ld 點", (0 - pobTran->srBRec.lnRedemptionPoints));

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "22::紅利扣抵點數%s%s\x0D\x0A", _DATA_FLAG_, szTemplate1);

				memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);
			}
			else if (pobTran->srBRec.inOrgCode == _REDEEM_REFUND_)
			{
				/* 支付金額 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", pobTran->srBRec.lnRedemptionPaidCreditAmount);
				inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, _PADDING_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "22::支付金額%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

				memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);

				/* 紅利扣抵金額 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", (pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnRedemptionPaidCreditAmount));
				inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, _PADDING_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "22::紅利扣抵金額%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

				memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);
				/* 紅利扣抵點數 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%ld 點", (pobTran->srBRec.lnRedemptionPoints));

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "22::紅利扣抵點數%s%s\x0D\x0A", _DATA_FLAG_, szTemplate1);

				memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);
			}
		}
		/* 正向交易 */
		else
		{
			if (pobTran->srBRec.inCode == _REDEEM_REFUND_)
			{
				/* 支付金額 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", (0 - pobTran->srBRec.lnRedemptionPaidCreditAmount));
				inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, _PADDING_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "22::支付金額%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

				memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);

				/* 紅利扣抵金額 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", (0 - (pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnRedemptionPaidCreditAmount)));
				inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, _PADDING_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "22::紅利扣抵金額%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

				memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);
				/* 紅利扣抵點數 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%ld 點", (0 - (pobTran->srBRec.lnRedemptionPoints)));

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "22::紅利扣抵點數%s%s\x0D\x0A", _DATA_FLAG_, szTemplate1);

				memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);
			}
			else if (pobTran->srBRec.inCode == _REDEEM_ADJUST_)
			{
				/* 支付金額 因為是調帳所以不會有小費*/
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", pobTran->srBRec.lnRedemptionPaidCreditAmount);
				inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, _PADDING_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "22::支付金額%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

				memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);

				/* 紅利扣抵金額 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", (pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnRedemptionPaidCreditAmount));
				inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, _PADDING_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "22::紅利扣抵金額%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

				memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);
				/* 紅利扣抵點數 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%ld 點", (pobTran->srBRec.lnRedemptionPoints));

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "22::紅利扣抵點數%s%s\x0D\x0A", _DATA_FLAG_, szTemplate1);

				memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);
			}
			else
			{
				/* 支付金額 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", pobTran->srBRec.lnRedemptionPaidCreditAmount + pobTran->srBRec.lnTipTxnAmount);
				inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, _PADDING_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "22::支付金額%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

				memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);

				/* 紅利扣抵金額 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", (pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnRedemptionPaidCreditAmount));
				inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, _PADDING_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "22::紅利扣抵金額%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

				memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);
				/* 紅利扣抵點數 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%ld 點", (pobTran->srBRec.lnRedemptionPoints));

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "22::紅利扣抵點數%s%s\x0D\x0A", _DATA_FLAG_, szTemplate1);

				memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);
				
				
				/* 紅利剩餘點數 */
				if (pobTran->inTransactionCode == _REDEEM_SALE_ || pobTran->srBRec.inCode == _REDEEM_SALE_)
				{
					/* 紅利扣抵才印 */
					/* 將NT$ ＋數字塞到szTemplate中來inpad */
					memset(szTemplate1, 0x00, sizeof(szTemplate1));
					sprintf(szTemplate1, "%ld 點", (pobTran->srBRec.lnRedemptionPointsBalance));

					/* 把前面的字串和數字結合起來 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "22::紅利剩餘點數%s%s\x0D\x0A", _DATA_FLAG_, szTemplate1);

					memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
					inDataLen += strlen(szPrintBuf);
				}
				
			}
			
		}
		/* 斷行 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		strcpy(szPrintBuf, "L::1\x0D\x0A");

		memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
	}
		
        return (inDataLen);
}

/*
Function        :inNCCC_ESC_PRINT_Receiptend
Date&Time       :2016/4/19 下午 5:54
Describe        :組E1 結尾
*/
int inNCCC_ESC_PRINT_ReceiptEND(TRANSACTION_OBJECT *pobTran, char *szPackData)
{
	int	inDataLen = 0;
	char	szPrintBuf[84 + 1], szTemplate[42 + 1] = {0};
	char	szPrtNotice[1 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
        
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	/* 簽名欄 */
	/* 免簽名 */
	/* 小費一律簽名 */
	if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE && pobTran->srBRec.inCode != _TIP_)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "12::%s\x0D\x0A", "免　　簽　　名");

		memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
	}
	/* (需求單 - 107227)邦柏科技自助作業客製化 by Russell 2018/12/7 下午 2:36 */
	/* (需求單 - 107276)自助交易標準400做法 by Russell 2018/12/27 上午 11:20 */
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)         ||
                 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_))
	{
		/* 一律送免簽名 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "12::%s\x0D\x0A", "免　　簽　　名");

		memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
	}
	/* 要簽名 */
	else
	{
		/* 有在signpad簽名 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
		inFunc_ComposeFileName_InvoiceNumber(pobTran, szTemplate, _PICTURE_FILE_EXTENSION_, 6);

		if (inFILE_Check_Exist((unsigned char *)szTemplate) == VS_SUCCESS)
		{
			/* 電子簽名 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "G::%s\x0D\x0A", _ESC_BMP_SIGNATURE_);
			memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
		}

	}

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "14::%s\x0D\x0A", "X:__________________________________");
	memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
	inDataLen += strlen(szPrintBuf);

	/* 持卡人姓名 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "11::%s\x0D\x0A", pobTran->srBRec.szCardHolder);
	memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
	inDataLen += strlen(szPrintBuf);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "12::%s\x0D\x0A", "持卡人簽名(特店存根聯)");
	memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
	inDataLen += strlen(szPrintBuf);
		
	/* 列印警示語 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "12::%s\x0D\x0A", "I AGREE TO PAY TOTAL AMOUNT");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "12::%s\x0D\x0A", "ACCORDING TO CARD ISSUER AGREEMENT");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* Print Notice */
	memset(szPrtNotice, 0x00, sizeof(szPrtNotice));
	inGetPrtNotice(szPrtNotice);
	if(memcmp(szPrtNotice, "Y", strlen("Y")) == 0)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "G::%s\x0D\x0A", _ESC_BMP_NOTICE_LOGO_);
                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
	}
	
        return (inDataLen);
}

/*
Function        :inNCCC_ESC_PRINT_DCC_DATA
Date&Time       :2017/4/21 下午 5:22
Describe        :組E1 DCC DATA部份
*/
int inNCCC_ESC_PRINT_DCC_DATA(TRANSACTION_OBJECT *pobTran, char *szPackData)
{
	int	i = 0;
	int	inDataLen = 0;
	int	inTempLen = 0;
	int	inUpperAmount = 0;
	int	inLowerAmount = 0;
	char	szPrintBuf[200 + 1] = {0}, szPrintBuf1[100 + 1] = {0}, szPrintBuf2[100 + 1] = {0}, szTemplate[100 + 1] = {0}, szTemplate1[100 + 1] = {0}, szTemplate2[100 + 1] = {0};
	char	szProductCodeEnable[1 + 1];
	char	szStore_Stub_CardNo_Truncate_Enable[2 + 1];
	char	szExamBit[10 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);

	/* 城市別 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", "城市別(City)");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 城市名 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetCityName(szTemplate);
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", szTemplate);

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 卡別 & 檢查碼 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "29::%s%s%s\x0D\x0A", "卡別(Card Type)", _DATA_FLAG_, "檢查碼(Check No.)");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 卡別值 & 檢查碼值 */
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	strcpy(szTemplate1, pobTran->srBRec.szCardLabel);

	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	if (strlen(pobTran->srBRec.szCheckNO) > 0)
	{
		strcpy(szTemplate2, pobTran->srBRec.szCheckNO);
	}
	else
	{
		inCARD_ExpDateEncryptAndDecrypt(pobTran, "", szTemplate2, _EXP_ENCRYPT_);
	}

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "29::%s%s%s\x0D\x0A", szTemplate1, _DATA_FLAG_, szTemplate2);

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 卡號掩飾 NCCC 規格以為準 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", "卡號(Card No.)");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	memset(szTemplate, 0x00, sizeof(szTemplate));
	strcpy(szTemplate, pobTran->srBRec.szPAN);
	
	/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
	/* 商店聯卡號遮掩 */
	memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
	inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
	if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0 && pobTran->srBRec.uszTxNoCheckBit == VS_TRUE)
	{
		/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
		for (i = 6; i < (strlen(szTemplate) - 4); i ++)
			szTemplate[i] = 0x2A;
	}
	
	/* 過卡方式 */
	if (pobTran->srBRec.uszFiscTransBit != VS_TRUE)
	{
		if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
			strcat(szTemplate,"(C)");
		else if (pobTran->srBRec.uszMobilePayBit == VS_TRUE)
			strcat(szTemplate, "(T)");
		else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
			strcat(szTemplate, "(W)");
		else
		{
			if (pobTran->srBRec.uszManualBit == VS_TRUE)
			{
				/* 【需求單 - 105244】端末設備支援以感應方式進行退貨交易 */
				/* 電文轉Manual Keyin但是簽單要印感應的W */
				if (pobTran->srBRec.uszRefundCTLSBit == VS_TRUE)
					strcat(szTemplate, "(W)");
				else
					strcat(szTemplate,"(M)");
			}
			else
				strcat(szTemplate,"(S)");
		}

	}
	else
	{
		if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
			strcat(szTemplate, "(W)");
		else
			strcat(szTemplate, "(C)");
	}

       	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", szTemplate);

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 主機別 & 交易類別 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", "主機別/交易類別(Host/Trans. Type)");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 主機別值 & 交易類別值*/
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	inGetHostLabel(szTemplate);
	inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);

	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	sprintf(szPrintBuf1, "%s %s %s", szTemplate, szTemplate1, szTemplate2);
	
	if (strlen(szTemplate1) > 0)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "11::%s\x0D\x0A", szPrintBuf1);

		memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
	}
		
	
	/* 批次號碼(Batch No.) & 授權碼 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "29::%s%s%s\x0D\x0A", "批次號碼(Batch No.)", _DATA_FLAG_, "授權碼(Auth Code)");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%03ld", pobTran->srBRec.lnBatchNum);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "29::%s%s%s\x0D\x0A", szTemplate, _DATA_FLAG_, pobTran->srBRec.szAuthCode);

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	/* 回覆碼 */
	if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
	{
	        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "11::%s\x0D\x0A", "回覆碼(Resp. Code)");

                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);

		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(&szTemplate[0], &pobTran->srBRec.szRespCode[0], 2);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "11::%s\x0D\x0A", szTemplate);

                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
	}

	/* 日期 & 時間 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", "日期/時間(Date/Time)");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 日期值 時間值 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%.4s/%.2s/%.2s %.2s:%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6], &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
		
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", szTemplate);

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 序號 & 調閱編號 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "29::%s%s%s\x0D\x0A", "序號(Ref. No.)", _DATA_FLAG_, "調閱編號(Inv. No.)");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 序號值 & 調閱編號值 */
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	inFunc_PAD_ASCII(szTemplate1, pobTran->srBRec.szRefNo, ' ', 25, _PADDING_RIGHT_);

	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	sprintf(szTemplate2, "%06ld", pobTran->srBRec.lnOrgInvNum);

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "29::%s%s%s\x0D\x0A", szTemplate1, _DATA_FLAG_, szTemplate2);

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 櫃號 */
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	inGetStoreIDEnable(szTemplate1);
	if ((memcmp(&szTemplate1[0], "Y", 1) == 0) && (strlen(pobTran->srBRec.szStoreID) > 0))
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_026_TAKA_, _CUSTOMER_INDICATOR_SIZE_) ||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
		{
			sprintf(szPrintBuf, "11::%s\x0D\x0A", "品群碼(Store ID)");
		}
		else
		{
			sprintf(szPrintBuf, "11::%s\x0D\x0A", "櫃號(Store ID)");
		}

		memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
		
		/* 櫃號值 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
		{
			/* 原品群碼*/
			sprintf(szTemplate1, "%s", pobTran->srBRec.szStoreID);
			
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "11::%s\x0D\x0A", szTemplate1);

			memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
			
			/* hardcode */
			char	szPOS_ID[6 + 1] = {0};
			memset(szPOS_ID, 0x00, sizeof(szPOS_ID));
			inGetPOS_ID(szPOS_ID);
			
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			memcpy(szTemplate1, _HARDCODE_STORE_ID_CUS_021_TAKAWEL_, 12);
			memcpy(&szTemplate1[12], szPOS_ID, 6);
		}
		else
		{
			sprintf(szTemplate1, "%s", pobTran->srBRec.szStoreID);
		}

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "11::%s\x0D\x0A", szTemplate1);

		memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
	}
	
	/* 產品代碼 */
	inGetProductCodeEnable(szProductCodeEnable);
	if (memcmp(szProductCodeEnable, "Y", 1) == 0)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "11::%s\x0D\x0A", "產品代碼(Product Code)");

                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "11::%s\x0D\x0A", pobTran->srBRec.szProductCode);

                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
	}
        
        /* 【需求單-113260】MasterCard機票交易需求 by Russell 2025/3/3 上午 11:55 */
        /* 取消維持原交易不列印機票資訊 */
        if (inNCCC_Func_Flight_Ticket_Allow_Print(pobTran) == VS_TRUE)
        {
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "27::%s%s%s\x0D\x0A", "出發地機場(departure)", _DATA_FLAG_, pobTran->srBRec.szFlightTicketPDS0523);

                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "27::%s%s%s\x0D\x0A", "目的地機場(arrival)", _DATA_FLAG_, pobTran->srBRec.szFlightTicketPDS0524);

                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);

                memset(szTemplate2, 0x00, sizeof(szTemplate2));
                if (strlen(pobTran->srBRec.szFlightTicketPDS0530) > 0)
                {
                        sprintf(szTemplate2, "%d", atoi(pobTran->srBRec.szFlightTicketPDS0530));
                }
                else
                {
                        sprintf(szTemplate2, "%s", " ");
                }
                sprintf(szPrintBuf, "%s", szTemplate2);
                
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "27::%s%s%s\x0D\x0A", "航班號碼(Flight No.)", _DATA_FLAG_, szTemplate2);

                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
                
                memset(szTemplate1, 0x00, sizeof(szTemplate1));
		inNCCC_Func_Get_Flight_Ticket_Txn_Category_PrintName(pobTran, szTemplate1);

		if (!memcmp(szTemplate1, _FLIGHT_TICKET_TXN_CATEGORY_PRINT_BAGGAGE_CHARGE_, max(strlen(szTemplate1), strlen(_FLIGHT_TICKET_TXN_CATEGORY_PRINT_BAGGAGE_CHARGE_)))		||
		    !memcmp(szTemplate1, _FLIGHT_TICKET_TXN_CATEGORY_PRINT_PREPAID_TICKET_, max(strlen(szTemplate1), strlen(_FLIGHT_TICKET_TXN_CATEGORY_PRINT_PREPAID_TICKET_)))		||
		    !memcmp(szTemplate1, _FLIGHT_TICKET_TXN_CATEGORY_PRINT_MISCELLANEOUS_CHARGE_ , max(strlen(szTemplate1), strlen(_FLIGHT_TICKET_TXN_CATEGORY_PRINT_MISCELLANEOUS_CHARGE_)))	||
		    !memcmp(szTemplate1, _FLIGHT_TICKET_TXN_CATEGORY_PRINT_TICKET_REISSUE_, max(strlen(szTemplate1), strlen(_FLIGHT_TICKET_TXN_CATEGORY_PRINT_TICKET_REISSUE_))))
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "27::%s%s%s\x0D\x0A", "交易類型(transaction category)", _DATA_FLAG_, " ");

			memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "25::%s%s%s\x0D\x0A", " ", _DATA_FLAG_, szTemplate1);

			memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
		}
		else
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "27::%s%s%s\x0D\x0A", "交易類型(transaction category)", _DATA_FLAG_, szTemplate1);

			memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
		}
        }
	
	/* TC */
	/* 驗測開關 */
	memset(szExamBit, 0x00, sizeof(szExamBit));
	inGetExamBit(szExamBit);
	if (pobTran->srBRec.inChipStatus == _EMV_CARD_ || pobTran->srBRec.uszContactlessBit == VS_TRUE)
	{
		if (pobTran->srBRec.inPrintOption == _PRT_MERCH_)
		{
			if (pobTran->srEMVRec.in9F26_ApplCryptogramLen > 0)
			{
				memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
				sprintf(szPrintBuf1, "%02X%02X%02X%02X%02X%02X%02X%02X",
								pobTran->srEMVRec.usz9F26_ApplCryptogram[0],
								pobTran->srEMVRec.usz9F26_ApplCryptogram[1],
								pobTran->srEMVRec.usz9F26_ApplCryptogram[2],
								pobTran->srEMVRec.usz9F26_ApplCryptogram[3],
								pobTran->srEMVRec.usz9F26_ApplCryptogram[4],
								pobTran->srEMVRec.usz9F26_ApplCryptogram[5],
								pobTran->srEMVRec.usz9F26_ApplCryptogram[6],
								pobTran->srEMVRec.usz9F26_ApplCryptogram[7]);
				
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "23::TC%s%s\x0D\x0A", _DATA_FLAG_, szPrintBuf1);
				
				memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                                inDataLen += strlen(szPrintBuf);
			}
			
			/* 商店聯卡號遮掩 */
			memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
			inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
			if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
			    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
			    strlen(pobTran->srBRec.szTxnNo) > 0)
			{
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			        sprintf(szPrintBuf, "11::%s\x0D\x0A", "交易編號(Transaction No.):");
			        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
			        inDataLen += strlen(szPrintBuf);

			       	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			        sprintf(szPrintBuf, "11::%s\x0D\x0A", pobTran->srBRec.szTxnNo);
			        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
		 		inDataLen += strlen(szPrintBuf);
			}
			
			/* M/C交易列印AP Lable (START) */
			if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)))
			{
				if (pobTran->srEMVRec.in50_APLabelLen > 0)
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					strcat(szPrintBuf, "AP Label:");
					memset(szTemplate, 0x00, sizeof(szTemplate));
					strcpy(szTemplate, (char*)pobTran->srEMVRec.usz50_APLabel);
					inUpperAmount = inFunc_Get_UpperCase_Char(szTemplate);
					inLowerAmount = strlen(szTemplate) - inUpperAmount;
					inTempLen =  _ESC_AP_LABEL_TOTAL_SPACE_ - (_ESC_UPPER_MULTIPLE_ * inUpperAmount + _ESC_LOWER_MULTIPLE_ * inLowerAmount);
					for (i = 0; i < inTempLen; i++)
					{
						strcat(szPrintBuf, " ");
					}
					strcat(szPrintBuf, szTemplate);
				}
				else
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					strcat(szPrintBuf, "AP Label:");
					strcat(szPrintBuf, "\x20");
				}
			}
			/* M/C交易列印AP Lable (END) */
			else if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)))
			{
				if (pobTran->srEMVRec.in50_APLabelLen > 0)
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					strcat(szPrintBuf, "AP Label:");
					memset(szTemplate, 0x00, sizeof(szTemplate));
					strcpy(szTemplate, (char*)pobTran->srEMVRec.usz50_APLabel);
					inUpperAmount = inFunc_Get_UpperCase_Char(szTemplate);
					inLowerAmount = strlen(szTemplate) - inUpperAmount;
					inTempLen =  _ESC_AP_LABEL_TOTAL_SPACE_ - (_ESC_UPPER_MULTIPLE_ * inUpperAmount + _ESC_LOWER_MULTIPLE_ * inLowerAmount);
					for (i = 0; i < inTempLen; i++)
					{
						strcat(szPrintBuf, " ");
					}
					strcat(szPrintBuf, szTemplate);
				}
				else
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					strcat(szPrintBuf, "AP Label:");
					strcat(szPrintBuf, "\x20");
				}
			}
			/* 驗測要印AP Label 和 AID */
			else if (szExamBit[0] == '1')
			{
				if (pobTran->srEMVRec.in50_APLabelLen > 0)
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					strcat(szPrintBuf, "AP Label:");
					memset(szTemplate, 0x00, sizeof(szTemplate));
					strcpy(szTemplate, (char*)pobTran->srEMVRec.usz50_APLabel);
					inUpperAmount = inFunc_Get_UpperCase_Char(szTemplate);
					inLowerAmount = strlen(szTemplate) - inUpperAmount;
					inTempLen =  _ESC_AP_LABEL_TOTAL_SPACE_ - (_ESC_UPPER_MULTIPLE_ * inUpperAmount + _ESC_LOWER_MULTIPLE_ * inLowerAmount);
					for (i = 0; i < inTempLen; i++)
					{
						strcat(szPrintBuf, " ");
					}
					strcat(szPrintBuf, szTemplate);
				}
				else
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					strcat(szPrintBuf, "AP Label:");
					strcat(szPrintBuf, "\x20");
				}
			}
			else
			{
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				strcat(szPrintBuf, "\x20");
				strcat(szPrintBuf, "\x20");
			}

			memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
			sprintf(szPrintBuf2, szPrintBuf);
			
			/* AID */
			if (strlen(pobTran->srBRec.szCUP_EMVAID) > 0)
			{
				memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
				sprintf(szPrintBuf1, "AID:%s", pobTran->srBRec.szCUP_EMVAID);
				inFunc_PAD_ASCII(szPrintBuf1, szPrintBuf1, ' ', _ESC_AID_TOTAL_LEN_, _PADDING_RIGHT_);
						
				if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
				{
					/* 【需求單-109121】Mastercard交易簽單列印AID需求 by Russell 2020/7/23 下午 4:23 */
					if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)) == 0)
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						strcat(szPrintBuf, "11");
						strcat(szPrintBuf, "::");
						strcat(szPrintBuf, szPrintBuf1);
						strcat(szPrintBuf, szPrintBuf2);
						strcat(szPrintBuf, "\x0D\x0A");

						memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
						inDataLen += strlen(szPrintBuf);
					}
					/* 驗測要印AP Label 和 AID */
					else if (szExamBit[0] == '1')
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						strcat(szPrintBuf, "11");
						strcat(szPrintBuf, "::");
						strcat(szPrintBuf, szPrintBuf1);
						strcat(szPrintBuf, szPrintBuf2);
						strcat(szPrintBuf, "\x0D\x0A");

						memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
						inDataLen += strlen(szPrintBuf);
					}
				}
				else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
				{
					/* 【需求單-109121】Mastercard交易簽單列印AID需求 by Russell 2020/7/23 下午 4:23 */
					if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)) == 0)
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						strcat(szPrintBuf, "11");
						strcat(szPrintBuf, "::");
						strcat(szPrintBuf, szPrintBuf1);
						strcat(szPrintBuf, szPrintBuf2);
						strcat(szPrintBuf, "\x0D\x0A");

						memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
						inDataLen += strlen(szPrintBuf);
					}
					/* 驗測要印AP Label 和 AID */
					else if (szExamBit[0] == '1')
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						strcat(szPrintBuf, "11");
						strcat(szPrintBuf, "::");
						strcat(szPrintBuf, szPrintBuf1);
						strcat(szPrintBuf, szPrintBuf2);
						strcat(szPrintBuf, "\x0D\x0A");

						memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
						inDataLen += strlen(szPrintBuf);
					}
				}
			}
		}
	}
	else
	{
		/* 商店聯卡號遮掩 */
		/* 磁條卡列印交易編號 */
		memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
		inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
		if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
		    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
		    strlen(pobTran->srBRec.szTxnNo) > 0)
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		        sprintf(szPrintBuf, "11::%s\x0D\x0A", "交易編號(Transaction No.):");
		        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
		        inDataLen += strlen(szPrintBuf);

		       	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		        sprintf(szPrintBuf, "11::%s\x0D\x0A", pobTran->srBRec.szTxnNo);
		        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
	 		inDataLen += strlen(szPrintBuf);
		}
	}
	
	return (inDataLen);
}

/*
Function        :inNCCC_ESC_PRINT_DCC_AMOUNT_FOR_SALE
Date&Time       :2017/4/21 下午 5:33
Describe        :
*/
int inNCCC_ESC_PRINT_DCC_AMOUNT_FOR_SALE(TRANSACTION_OBJECT *pobTran, char *szPackData)
{
	int	inDataLen = 0;
	int	inIRDU = 0;
	char	szPrintBuf[200 + 1] = {0}, szPrintBuf1[100 + 1] = {0}, szPrintBuf2[100 + 1] = {0};
	char	szIRDU[12 + 1] = {0};			/* Inverted Rate Display Unit */
	char	szExchangeAmt[50 + 1] = {0};		/* 轉換金額 */

	/* ------------------------------------------ */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "12::%s\x0D\x0A", "------------------------------------------");
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* Please select */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "12::%s\x0D\x0A", "Please select");
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* Transaction currency as below */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "12::%s\x0D\x0A", "Transaction currency as below");
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 外幣 */
	memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
	inFunc_Amount_Comma_DCC(pobTran->srBRec.szDCC_FCA, "", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, pobTran->srBRec.szDCC_FCMU, "", szPrintBuf2);

	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	sprintf(szPrintBuf1, "1.【X】%s %s", pobTran->srBRec.szDCC_FCAC, szPrintBuf2);						/* Foreign currcncy Alphabetic Code */
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", szPrintBuf1);
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 匯率 */
	/* Inverted Rate Display Unit */
	inIRDU = atoi(pobTran->srBRec.szDCC_IRDU); 
	memset(szIRDU, 0x00, sizeof(szIRDU));
	strcpy(szIRDU, "1");
	inFunc_PAD_ASCII(szIRDU, szIRDU, '0', (inIRDU + 1), _PADDING_RIGHT_);
	
	memset(szExchangeAmt, 0x00, sizeof(szExchangeAmt));
	inFunc_Amount_Comma_DCC(pobTran->srBRec.szDCC_IRV, "", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, pobTran->srBRec.szDCC_IRMU, "", szExchangeAmt);
	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	sprintf(szPrintBuf1, "Exchange Rate:");
	memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
	sprintf(szPrintBuf2, "%s %s = %s NTD", szIRDU, pobTran->srBRec.szDCC_FCAC, szExchangeAmt);
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s%s\x0D\x0A", szPrintBuf1, szPrintBuf2);
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 轉換費率 */
	/* XX.XX %*/
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	inFunc_Currency_Conversion_Fee(pobTran->srBRec.szDCC_MPV, pobTran->srBRec.szDCC_MPDP, szPrintBuf1);
	strcat(szPrintBuf1, " %");
	
        sprintf(szPrintBuf, "11::%s%s\x0D\x0A", "轉換費率(Currency Conversion Fee)", szPrintBuf1);
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	/* (Currency Conversion Fee on the exchange rate over a wholesale rate.) */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", "(Currency Conversion Fee on the");
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* exchange rate over a wholesale rate.) */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", "exchange rate over a wholesale rate.)");
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 台幣 */
	memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
	sprintf(szPrintBuf2, "%ld", pobTran->srBRec.lnTxnAmount);
	inFunc_Amount_Comma_DCC(szPrintBuf2, "", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, "0", "", szPrintBuf2);

	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	/* 轉台幣跑另外一隻 */
	sprintf(szPrintBuf1, "2.【 】%s %s", "NTD", szPrintBuf2);									/* Foreign currcncy Alphabetic Code */
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", szPrintBuf1);
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	/* 有開小費要多印 */
	memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
	inGetTransFunc(szPrintBuf2);
	/* 檢查是否有開小費 */
	/* 預先授權完成必定不印小費 */
	if (szPrintBuf2[6] == 'Y'	&&
	    pobTran->srBRec.inCode == _SALE_)
	{
		/* 小費跟總計不需列印外幣 by Russell 2020/10/29 下午 3:29 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	        sprintf(szPrintBuf, "20::小費(Tips)%s%s%s\x0D\x0A", _DATA_FLAG_, "", "____________");
                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "20::總計(Total)%s%s%s\x0D\x0A", _DATA_FLAG_, "", "____________");
                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
	}
	
	/* 斷行 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        strcpy(szPrintBuf, "L::1\x0D\x0A");
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	return (inDataLen);
}

/*
Function        :inNCCC_ESC_PRINT_DCC_ReceiptEND
Date&Time       :2017/4/24 下午 5:25
Describe        :
*/
int inNCCC_ESC_PRINT_DCC_ReceiptEND(TRANSACTION_OBJECT *pobTran, char *szPackData)
{
	int	inTempLen = 0;
	int	inDataLen = 0;
	char	szPrtNotice[1 + 1];
	char	szPrintBuf[200 + 1], szTemplate[100 + 1];
	char	szTempBuff[1000] = {};

	/* Disclaimer */
	if ((!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_VISA_, strlen(_CARD_TYPE_VISA_))) && 
	    (pobTran->srBRec.inCode == _SALE_		|| 
	     pobTran->srBRec.inCode == _TIP_		|| 
	     pobTran->srBRec.inCode == _PRE_COMP_))
	{
		memset(szTempBuff, 0x00, sizeof(szTempBuff));
		inTempLen = inNCCC_ESC_PRINT_DCC_Disclaimer(_ESC_DCC_V_, szTempBuff);
		if (inTempLen > 0)
		{
		        memcpy(&szPackData[inDataLen], &szTempBuff[0], inTempLen);
                        inDataLen += inTempLen;
		}
	}
	else if ((!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_))) && 
		 (pobTran->srBRec.inCode == _SALE_	|| 
		  pobTran->srBRec.inCode == _TIP_	|| 
		  pobTran->srBRec.inCode == _PRE_COMP_))
	{
		memset(szTempBuff, 0x00, sizeof(szTempBuff));
		inTempLen = inNCCC_ESC_PRINT_DCC_Disclaimer(_ESC_DCC_M_, szTempBuff);
		if (inTempLen > 0)
		{
		        memcpy(&szPackData[inDataLen], &szTempBuff[0], inTempLen);
                        inDataLen += inTempLen;
		}
	}
	
	if (pobTran->srBRec.inPrintOption == _PRT_MERCH_	||
	    pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
        {
		if (pobTran->srBRec.uszDCCTransBit == VS_TRUE && pobTran->srBRec.inCode == _TIP_)
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "11::%s\x0D\x0A", "[ ] ACCEPT");
			memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
		}
	}

	/* 簽名欄 */
	/* 免簽名 */
	/* 小費一律簽名 */
	if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE && pobTran->srBRec.inCode != _TIP_)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "12::%s\x0D\x0A", "免　　簽　　名");

		memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
	}
	/* 要簽名 */
	else
	{
		/* 有在signpad簽名 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
		inFunc_ComposeFileName_InvoiceNumber(pobTran, szTemplate, _PICTURE_FILE_EXTENSION_, 6);

		if (inFILE_Check_Exist((unsigned char *)szTemplate) == VS_SUCCESS)
		{
			/* 電子簽名 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "G::%s\x0D\x0A", _ESC_BMP_SIGNATURE_);
			memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
		}

	}

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "14::%s\x0D\x0A", "X:__________________________________");
	memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
	inDataLen += strlen(szPrintBuf);

	/* 持卡人姓名 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "11::%s\x0D\x0A", pobTran->srBRec.szCardHolder);
	memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
	inDataLen += strlen(szPrintBuf);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "12::%s\x0D\x0A", "持卡人簽名(特店存根聯)");
	memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
	inDataLen += strlen(szPrintBuf);
		
	/* 列印警示語 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "12::%s\x0D\x0A", "I AGREE TO PAY TOTAL AMOUNT");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "12::%s\x0D\x0A", "ACCORDING TO CARD ISSUER AGREEMENT");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* Print Notice */
	memset(szPrtNotice, 0x00, sizeof(szPrtNotice));
	inGetPrtNotice(szPrtNotice);
	if(memcmp(szPrtNotice, "Y", strlen("Y")) == 0)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "G::%s\x0D\x0A", _ESC_BMP_NOTICE_LOGO_);
                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
	}

	return (inDataLen);
}

/*
Function        :inNCCC_ESC_PRINT_DCC_Disclaimer
Date&Time       :2017/4/24 下午 5:29
Describe        :
*/
int inNCCC_ESC_PRINT_DCC_Disclaimer(char *szFilename, char *szPackData)
{
	int		i;
	int		inOffset;				/* 偏移 */
	int		inDataSegament;				/* 一次讀多少 */
	int		inPrintHandle = 0;			/* put in 到哪裡 */
	int		inLineLength = 0;			/* 一行印得下多少char */
	int		inDataLen = 0;				/* For ESC */
	long		lnDataLength;				/* 資料長度 */
	long            lnReadLength;                           /* 剩餘讀取長度 */
	char		szPrintTemp[200 + 1];			/* 放印一行的陣列 */
	char		szPrintBuf[250 + 1];			/* For ESC */
	unsigned long	ulFile_Handle;
        unsigned char   *uszTemp;                               /* 暫存，放整筆CDT檔案 */

	/* [BUG] 移除檔案同步API SYNC()指令
	 * [20260506] 刪除 不需使用inFILE_OpenReadOnly() */
	
	/*
         * open disclaimer file
         * open失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        if (inFILE_Open(&ulFile_Handle, (unsigned char *)szFilename) == VS_ERROR)
        {
                /* 開檔失敗 ，不用關檔 */
                /* 開檔失敗，所以回傳error */
                return (VS_ERROR);
        }

        /*
         * get data size 為了接下來動態宣告記憶體的大小需要知道多少 需先取得檔案Size
         * get size 失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        lnDataLength = lnFILE_GetSize(&ulFile_Handle, (unsigned char *)szFilename);

        if (lnDataLength == VS_ERROR)
        {
                /* GetSize失敗 ，關檔 */
                inFILE_Close(&ulFile_Handle);

                return (VS_ERROR);
        }

        /*
         * allocate 記憶體
         * allocate時多分配一個byte以防萬一（ex:換行符號）
         */
        uszTemp = malloc(lnDataLength + 1);
        /* 初始化 uszTemp uszReadData */
        memset(uszTemp, 0x00, lnDataLength + 1);

	/* ESC V3一行用70會超過，Vx520用64，跟著用64 by Russell 2020/10/29 下午 3:55 */
	inLineLength = 64;
	
	/*一次讀1024 */
	inDataSegament = 1024;
	
        /* seek 到檔案開頭 & 從檔案開頭開始read */
        if (inFILE_Seek(ulFile_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnDataLength;

                for (i = 0;; ++i)
                {
                        /* 剩餘長度大於或等於inLineLength */
                        if (lnReadLength >= inDataSegament)
                        {
                                if (inFILE_Read(&ulFile_Handle, &uszTemp[inDataSegament * i], inDataSegament) == VS_SUCCESS)
                                {
                                        /* 一次讀inDataSegament */
                                        lnReadLength -= inDataSegament;

                                        /* 當剩餘長度剛好為inLineLength，會剛好讀完 */
                                        if (lnReadLength == 0)
                                                break;
                                }
                                /* 讀失敗時 */
                                else
                                {
                                        /* Close檔案 */
                                        inFILE_Close(&ulFile_Handle);

                                        /* Free pointer */
                                        free(uszTemp);

                                        return (VS_ERROR);
                                }
                        }
                        /* 剩餘長度小於inLineLength */
                        else if (lnReadLength < inDataSegament)
                        {
                                /* 就只讀剩餘長度 */
                                if (inFILE_Read(&ulFile_Handle, &uszTemp[inDataSegament * i], inDataSegament) == VS_SUCCESS)
                                {
                                        break;
                                }
                                /* 讀失敗時 */
                                else
                                {
                                        /* Close檔案 */
                                        inFILE_Close(&ulFile_Handle);

                                        /* Free pointer */
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
                free(uszTemp);

                /* Seek失敗，所以回傳Error */
                return (VS_ERROR);
        }

	/* 剩餘讀取長度 */
	lnReadLength = lnDataLength;
	
	while (inPrintHandle < (int)lnDataLength)
	{
		/* 為了去除句首的空白 */
		while (uszTemp[inPrintHandle] == 0x20)
		{
			inPrintHandle++;
			lnReadLength--;
		}
		
		/* 確認句末是否會斷到字*/
		if (lnReadLength >= inLineLength)
		{
			/* 不能斷行的狀況 */
			if (uszTemp[inPrintHandle + inLineLength] != 0x20 && uszTemp[inPrintHandle + inLineLength - 1] != 0x20)
			{
				memset(szPrintTemp, 0x00, sizeof(szPrintTemp));
				inOffset = 0;
				
				/* 若句末為非空白，則往前找直到可以段句不斷字 */
				while(uszTemp[inPrintHandle + inLineLength - inOffset] != 0x20)
				{
					inOffset++;
					/* 萬一整行都沒空白，直接印 */
					if ((inLineLength - inOffset) == 0)
					{
						inOffset = 0;
						break;
					}
				}
				memcpy(szPrintTemp, &uszTemp[inPrintHandle], (int)inLineLength - inOffset);
				inPrintHandle += inLineLength - inOffset;
				lnReadLength -= inLineLength - inOffset;
			}
			/* 可以直接斷行的狀況 */
			else
			{
				memset(szPrintTemp, 0x00, sizeof(szPrintTemp));
				memcpy(szPrintTemp, &uszTemp[inPrintHandle], (int)inLineLength);
				inPrintHandle += inLineLength;
				lnReadLength -= inLineLength;
			}
		}
		/* 只剩不到一行的狀況 */
		else
		{
			memset(szPrintTemp, 0x00, sizeof(szPrintTemp));
			memcpy(szPrintTemp, &uszTemp[inPrintHandle], (int)lnReadLength);
			inPrintHandle += lnReadLength;
			lnReadLength -= lnReadLength;
		}
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "11::%s\x0D\x0A", szPrintTemp);

                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
		
	}
	
	
	/* release */
        /* 關檔 */
        inFILE_Close(&ulFile_Handle);

        /* Free pointer */
        free(uszTemp);
	
	return (inDataLen);
}

/*
Function        :inNCCC_ESC_PRINT_DCC_Amount_NOT_FOR_SALE
Date&Time       :2017/4/24 下午 5:50
Describe        :DCC 一段式 非SALE的其他交易
*/
int inNCCC_ESC_PRINT_DCC_Amount_NOT_FOR_SALE(TRANSACTION_OBJECT *pobTran, char *szPackData)
{
	int	inDataLen = 0;
	int	inIRDU = 0;
	char	szIRDU[12 + 1] = {0};		/* Inverted Rate Display Unit */
	char	szPrintBuf[200 + 1] = {0}, szPrintBuf1[100 + 1] = {0}, szPrintBuf2[100 + 1] = {0}, szTemplate[42 + 1] = {0};
	char	szExchangeAmt[50 + 1] = {0};		/* 轉換金額 */
	long	lnTipTotalAmt = 0l;
	
	/* 台幣 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", "台幣(Local Amount)");

        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	if (pobTran->srBRec.inCode == _TIP_)
	{
		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
		if ((pobTran->srBRec.uszVOIDBit == VS_TRUE && pobTran->srBRec.inOrgCode != _REFUND_)	|| 
		     pobTran->srBRec.inCode == _REFUND_)
		{
			sprintf(szPrintBuf2, "%ld", 0 - pobTran->srBRec.lnTxnAmount);
		}
		else
		{
			sprintf(szPrintBuf2, "%ld", pobTran->srBRec.lnTxnAmount);
		}
		inFunc_Amount_Comma_DCC(szPrintBuf2, "", '\x00', _SIGNED_NONE_, 13, _PADDING_RIGHT_, "0", "", szPrintBuf2);

		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szPrintBuf1, "%s", szPrintBuf2);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "23::Amount%s:NTD %s\x0D\x0A", _DATA_FLAG_, szPrintBuf1);
                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);

		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
		if ((pobTran->srBRec.uszVOIDBit == VS_TRUE && pobTran->srBRec.inOrgCode != _REFUND_)	|| 
		     pobTran->srBRec.inCode == _REFUND_)
		{
			sprintf(szPrintBuf2, "%ld", 0 - pobTran->srBRec.lnTipTxnAmount);
		}
		else
		{
			sprintf(szPrintBuf2, "%ld", pobTran->srBRec.lnTipTxnAmount);
		}
		inFunc_Amount_Comma_DCC(szPrintBuf2, "", '\x00', _SIGNED_NONE_, 13, _PADDING_RIGHT_, "0", "", szPrintBuf2);

		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szPrintBuf1, "%s", szPrintBuf2);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "23::Tips%s:NTD %s\x0D\x0A", _DATA_FLAG_, szPrintBuf1);
                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);

		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
		if ((pobTran->srBRec.uszVOIDBit == VS_TRUE && pobTran->srBRec.inOrgCode != _REFUND_)	|| 
		     pobTran->srBRec.inCode == _REFUND_)
		{
			sprintf(szPrintBuf2, "%ld", 0 - pobTran->srBRec.lnTotalTxnAmount);
		}
		else
		{
			sprintf(szPrintBuf2, "%ld", pobTran->srBRec.lnTotalTxnAmount);
		}
		inFunc_Amount_Comma_DCC(szPrintBuf2, "", '\x00', _SIGNED_NONE_, 13, _PADDING_RIGHT_, "0", "", szPrintBuf2);

		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szPrintBuf1, "%s", szPrintBuf2);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "23::Total%s:NTD %s\x0D\x0A", _DATA_FLAG_, szPrintBuf1);
                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
	}
	else
	{
		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
		if ((pobTran->srBRec.uszVOIDBit == VS_TRUE && pobTran->srBRec.inOrgCode != _REFUND_)	|| 
		     pobTran->srBRec.inCode == _REFUND_)
		{
			sprintf(szPrintBuf2, "%ld", 0 - pobTran->srBRec.lnTotalTxnAmount);
		}
		else
		{
			sprintf(szPrintBuf2, "%ld", pobTran->srBRec.lnTotalTxnAmount);
		}
		inFunc_Amount_Comma_DCC(szPrintBuf2, "", '\x00', _SIGNED_NONE_, 13, _PADDING_RIGHT_, "0", "", szPrintBuf2);

		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szPrintBuf1, "%s", szPrintBuf2);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "23::Total%s:NTD %s\x0D\x0A", _DATA_FLAG_, szPrintBuf1);
                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
	}

	if (pobTran->srBRec.inCode == _TIP_)
	{
		/* 金額 */
		/* 初始化 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "11::金額(Amount)\x0D\x0A");
                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
		
		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
		if ((pobTran->srBRec.uszVOIDBit == VS_TRUE && pobTran->srBRec.inOrgCode != _REFUND_) || 
		     pobTran->srBRec.inCode == _REFUND_)
		{
			inFunc_Amount_Comma_DCC(pobTran->srBRec.szDCC_FCA, "-", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, pobTran->srBRec.szDCC_FCMU, pobTran->srBRec.szDCC_FCAC, szPrintBuf2);
		}
		else
		{
			inFunc_Amount_Comma_DCC(pobTran->srBRec.szDCC_FCA, "", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, pobTran->srBRec.szDCC_FCMU, pobTran->srBRec.szDCC_FCAC, szPrintBuf2);
		}

		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szPrintBuf1, "%s", szPrintBuf2);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "20::\x20%s%s\x0D\x0A", _DATA_FLAG_, szPrintBuf1);
                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
		
		/* 小費 */
		/* 初始化 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "11::小費(Tips)\x0D\x0A");
                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
		
		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
		if ((pobTran->srBRec.uszVOIDBit == VS_TRUE && pobTran->srBRec.inOrgCode != _REFUND_) || 
		     pobTran->srBRec.inCode == _REFUND_)
		{
			inFunc_Amount_Comma_DCC(pobTran->srBRec.szDCC_TIPFCA, "-", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, pobTran->srBRec.szDCC_FCMU, pobTran->srBRec.szDCC_FCAC, szPrintBuf2);
		}
		else
		{
			inFunc_Amount_Comma_DCC(pobTran->srBRec.szDCC_TIPFCA, "", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, pobTran->srBRec.szDCC_FCMU, pobTran->srBRec.szDCC_FCAC, szPrintBuf2);
		}

		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szPrintBuf1, "%s", szPrintBuf2);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "20::\x20%s%s\x0D\x0A", _DATA_FLAG_, szPrintBuf1);
                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
		
		/* 總計 */
		/* 初始化 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "20::總計(Total)%s%s\x0D\x0A", _DATA_FLAG_, "\x20");
                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "11::%s\x0D\x0A", "Total amount of Transaction Currency");
                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
		
		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
		/* 初始化 */
		lnTipTotalAmt = 0;
		memset(szTemplate, 0x00, sizeof(szTemplate));
		lnTipTotalAmt += atol(pobTran->srBRec.szDCC_FCA);
		lnTipTotalAmt += atol(pobTran->srBRec.szDCC_TIPFCA);
		sprintf(szTemplate, "%ld", lnTipTotalAmt);
		
		if ((pobTran->srBRec.uszVOIDBit == VS_TRUE && pobTran->srBRec.inOrgCode != _REFUND_) || 
		     pobTran->srBRec.inCode == _REFUND_)
		{
			inFunc_Amount_Comma_DCC(szTemplate, "-", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, pobTran->srBRec.szDCC_FCMU, pobTran->srBRec.szDCC_FCAC, szPrintBuf2);
		}
		else
		{
			inFunc_Amount_Comma_DCC(szTemplate, "", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, pobTran->srBRec.szDCC_FCMU, pobTran->srBRec.szDCC_FCAC, szPrintBuf2);
		}

		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szPrintBuf1, "%s", szPrintBuf2);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "20::\x20%s%s\x0D\x0A", _DATA_FLAG_, szPrintBuf1);
                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
	}
	else
	{
		/* 總計 */
		/* 初始化 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "20::總計(Total)%s%s\x0D\x0A", _DATA_FLAG_, "\x20");
                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "11::%s\x0D\x0A", "Total amount of Transaction Currency");
                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
		
		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
		if ((pobTran->srBRec.uszVOIDBit == VS_TRUE && pobTran->srBRec.inOrgCode != _REFUND_) || 
		     pobTran->srBRec.inCode == _REFUND_)
		{
			inFunc_Amount_Comma_DCC(pobTran->srBRec.szDCC_FCA, "-", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, pobTran->srBRec.szDCC_FCMU, pobTran->srBRec.szDCC_FCAC, szPrintBuf2);
		}
		else
		{
			inFunc_Amount_Comma_DCC(pobTran->srBRec.szDCC_FCA, "", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, pobTran->srBRec.szDCC_FCMU, pobTran->srBRec.szDCC_FCAC, szPrintBuf2);
		}
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szPrintBuf1, "%s", szPrintBuf2);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "20::\x20%s%s\x0D\x0A", _DATA_FLAG_, szPrintBuf1);
		memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
	}
	
	/* 列印換算匯率比【一】 */
	/* Inverted Rate Display Unit */
	inIRDU = atoi(pobTran->srBRec.szDCC_IRDU); 
	memset(szIRDU, 0x00, sizeof(szIRDU));
	strcpy(szIRDU, "1");
	inFunc_PAD_ASCII(szIRDU, szIRDU, '0', (inIRDU + 1), _PADDING_RIGHT_);
	/* 列印換算匯率比【二】 */
	inFunc_Amount_Comma_DCC(pobTran->srBRec.szDCC_IRV, "", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, pobTran->srBRec.szDCC_IRMU, "", szExchangeAmt);
	/* 匯率 */
	/* Ex:1 USD = 30.0000 NTD */
	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	sprintf(szPrintBuf1, "Exchange Rate:");
	memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
	sprintf(szPrintBuf2, "%s %s = %s NTD", szIRDU, pobTran->srBRec.szDCC_FCAC, szExchangeAmt);
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s%s\x0D\x0A", szPrintBuf1, szPrintBuf2);
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	/* 轉換費率 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	/* XX.XX % */
	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	inFunc_Currency_Conversion_Fee(pobTran->srBRec.szDCC_MPV, pobTran->srBRec.szDCC_MPDP, szPrintBuf1);
	strcat(szPrintBuf1, " %");
	
        sprintf(szPrintBuf, "11::%s%s\x0D\x0A", "轉換費率(Currency Conversion Fee)", szPrintBuf1);
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	/* (Currency Conversion Fee on the */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", "(Currency Conversion Fee on the");
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* exchange rate over a wholesale rate.) */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", "exchange rate over a wholesale rate.)");
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	/* 斷行 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        strcpy(szPrintBuf, "L::1\x0D\x0A");
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	return (inDataLen);
}

/*
Function        :inNCCC_ESC_PRINT_DCC_Amount_CHANGE_NTD
Date&Time       :2017/4/25 上午 9:05
Describe        :DCC 一段式 轉台幣
*/
int inNCCC_ESC_PRINT_DCC_Amount_CHANGE_NTD(TRANSACTION_OBJECT *pobTran, char *szPackData)
{
	int	inDataLen = 0;
	int	inIRDU = 0;
	char	szPrintBuf[200 + 1] = {0}, szPrintBuf1[50 + 1] = {0}, szPrintBuf2[50 + 1] = {0};
	char	szIRDU[12 + 1] = {0};			/* Inverted Rate Display Unit */
	char	szExchangeAmt[50 + 1] = {0};		/* 轉換金額 */
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "12::%s\x0D\x0A", "------------------------------------------");
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "12::%s\x0D\x0A", "Please select");
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "12::%s\x0D\x0A", "Transaction currency as below");
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 外幣 */
	memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
	inFunc_Amount_Comma_DCC(pobTran->srBRec.szDCC_FCA, "", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, pobTran->srBRec.szDCC_FCMU, "", szPrintBuf2);

	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	sprintf(szPrintBuf1, "1.【 】%s %s", pobTran->srBRec.szDCC_FCAC, szPrintBuf2);						/* Foreign currcncy Alphabetic Code */
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", szPrintBuf1);
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 匯率 */
	/* Inverted Rate Display Unit */
	inIRDU = atoi(pobTran->srBRec.szDCC_IRDU); 
	memset(szIRDU, 0x00, sizeof(szIRDU));
	strcpy(szIRDU, "1");
	inFunc_PAD_ASCII(szIRDU, szIRDU, '0', (inIRDU + 1), _PADDING_RIGHT_);
	
	memset(szExchangeAmt, 0x00, sizeof(szExchangeAmt));
	inFunc_Amount_Comma_DCC(pobTran->srBRec.szDCC_IRV, "", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, pobTran->srBRec.szDCC_IRMU, "", szExchangeAmt);
	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	sprintf(szPrintBuf1, "Exchange Rate:");
	memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
	sprintf(szPrintBuf2, "%s %s = %s NTD", szIRDU, pobTran->srBRec.szDCC_FCAC, szExchangeAmt);
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s%s\x0D\x0A", szPrintBuf1, szPrintBuf2);
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	/* 轉換費率 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	/* XX.XX % */
	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	inFunc_Currency_Conversion_Fee(pobTran->srBRec.szDCC_MPV, pobTran->srBRec.szDCC_MPDP, szPrintBuf1);
	strcat(szPrintBuf1, " %");
	
        sprintf(szPrintBuf, "11::%s%s\x0D\x0A", "轉換費率(Currency Conversion Fee)", szPrintBuf1);
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	/* (Currency Conversion Fee on the */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", "(Currency Conversion Fee on the");
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* exchange rate over a wholesale rate.) */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", "exchange rate over a wholesale rate.)");
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 台幣 */
	memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
	sprintf(szPrintBuf2, "%ld", pobTran->srBRec.lnTxnAmount);
	inFunc_Amount_Comma_DCC(szPrintBuf2, "", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, "0", "", szPrintBuf2);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf1, "2.【X】%s %s", "NTD", szPrintBuf2);									/* Foreign currcncy Alphabetic Code */

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", szPrintBuf1);
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	/* 有開小費要多印 */
	memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
	inGetTransFunc(szPrintBuf2);
	/* 檢查是否有開小費 */
	/* 預先授權完成必定不印小費 */
	if (szPrintBuf2[6] == 'Y'	&&
	    (pobTran->srBRec.inCode == _SALE_ && pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE))
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	        sprintf(szPrintBuf, "20::小費(Tips)%s%s\x0D\x0A", _DATA_FLAG_, "____________");
                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "20::總計(Total)%s%s\x0D\x0A", _DATA_FLAG_, "____________");
                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
	}

	/* 斷行 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        strcpy(szPrintBuf, "L::1\x0D\x0A");
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	return (inDataLen);
}

/*
Function        :inNCCC_ESC_PRINT_LOYALTY_REDEEM_DATA
Date&Time       :2017/4/25 上午 10:38
Describe        :組E1 DATA部份
*/
int inNCCC_ESC_PRINT_LOYALTY_REDEEM_DATA(TRANSACTION_OBJECT *pobTran, char *szPackData)
{
	int			inTempLen = 0;
	int			inDataLen = 0;
	char			szRewardL1L2L3[1650 + 1];	/* L5最常可到1618Bytes 取1650 */
	char			szPrintBuf[300 + 1];
	char			szPrintBuf1[100 + 1];
	char			szTempBuff[500];
	unsigned long		ulBufferSize;
	LOYALTY_L1L2L3_OBJECT	srLoyaltyData;
	
	ulBufferSize = sizeof(szRewardL1L2L3);
	memset(szRewardL1L2L3, 0x00, sizeof(szRewardL1L2L3));
	if (inNCCC_Loyalty_Read_Reward_Data(&ulBufferSize, szRewardL1L2L3, _REWARD_FILE_NAME_) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	/* 放到結構中 */
	if (inNCCC_Loyalty_Data_Format(&srLoyaltyData, (int)ulBufferSize, szRewardL1L2L3) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	/* 日期 & 時間 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", "日期/時間(Date/Time)");
        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 日期值 時間值 */
	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	sprintf(szPrintBuf1, "%.4s/%.2s/%.2s %.2s:%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6], &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
		
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "14::%s\x0D\x0A", szPrintBuf1);
        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 是否列印兌換核銷資訊(一) */
	if (memcmp(srLoyaltyData.srL3DATA.srL3_CANCEL_DATA[0].szCancelFlag, "1", strlen("1")) == 0)
	{
		/* 兌換核銷資訊(一)長度。(最大長度限制100 Bytes，右靠左補零) */
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		memcpy(szPrintBuf1, srLoyaltyData.srL3DATA.srL3_CANCEL_DATA[0].szCancelLen, 3);

		/* 兌換核銷資訊(一)之內容。(內容可支援英數字、中文、換行。需補足空白至100 Bytes。) */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memcpy(szPrintBuf, srLoyaltyData.srL3DATA.srL3_CANCEL_DATA[0].szCancelContent, atoi(srLoyaltyData.srL3DATA.srL3_CANCEL_DATA[0].szCancelLen));
		
		memset(szTempBuff, 0x00, sizeof(szTempBuff));
		inTempLen = inNCCC_ESC_PRINT_Format_Auto_Change_Line(szPrintBuf, szTempBuff);	
		if (inTempLen > 0)
		{
		        memcpy(&szPackData[inDataLen], &szTempBuff[0], inTempLen);
                        inDataLen += inTempLen;
		}
	}

	/* 是否列印兌換核銷資訊(二) */
	if (memcmp(srLoyaltyData.srL3DATA.srL3_CANCEL_DATA[0].szCancelFlag, "1", strlen("1")) == 0)
	{
		/* 兌換核銷資訊(二)長度。(最大長度限制100 Bytes，右靠左補零) */
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		memcpy(szPrintBuf1, srLoyaltyData.srL3DATA.srL3_CANCEL_DATA[1].szCancelLen, 3);

		/* 兌換核銷資訊(二)之內容。(內容可支援英數字、中文、換行。需補足空白至100 Bytes。) */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memcpy(szPrintBuf, srLoyaltyData.srL3DATA.srL3_CANCEL_DATA[1].szCancelContent, atoi(srLoyaltyData.srL3DATA.srL3_CANCEL_DATA[1].szCancelLen));
		
		memset(szTempBuff, 0x00, sizeof(szTempBuff));
		inTempLen = inNCCC_ESC_PRINT_Format_Auto_Change_Line(szPrintBuf, szTempBuff);
		if (inTempLen > 0)
		{
		        memcpy(&szPackData[inDataLen], &szTempBuff[0], inTempLen);
                        inDataLen += inTempLen;
		}
	}

	/* 是否列印補充資訊 */
	if (memcmp(srLoyaltyData.srL3DATA.szSupInfFlag, "1", strlen("1")) == 0)
	{
		/* 補充資訊實際長度。(右靠左補零) */
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		memcpy(szPrintBuf1, srLoyaltyData.srL3DATA.szSupInfLen, 3);

		/* 補充資訊內容 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memcpy(szPrintBuf, srLoyaltyData.srL3DATA.szSupInfContent, atoi(srLoyaltyData.srL3DATA.szSupInfLen));
		
		memset(szTempBuff, 0x00, sizeof(szTempBuff));
		inTempLen = inNCCC_ESC_PRINT_Format_Auto_Change_Line(szPrintBuf, szTempBuff);	
		if (inTempLen > 0)
		{
		        memcpy(&szPackData[inDataLen], &szTempBuff[0], inTempLen);
                        inDataLen += inTempLen;
		}
	}
		
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        strcpy(szPrintBuf, "L::2\x0D\x0A");
        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	return (inDataLen);
}

/*
Function	:inPRINT_Buffer_PutIn_Format_Auto_Change_Line
Date&Time	:2017/2/15 下午 5:05
Describe	:For 優惠兌換自動換行訊息，一律換行，inNextLine無作用，不知道會不會有Bug，建議少用
 *szString:		put in 的字串
 *inFontSize:		put in的字型大小	
 *uszBuffer:		用來列印的Buffer
 *srBhandle:		用來管理buffer現在寫到哪裡，是否寫超過紙張或Buffer最底部
 *srFont_Attrib:	字的大小，字與字之間的間距
 *inNextLine:		下次在Put in會在這一行還是下一行，放_LAST_ENTRY_表示下一次put in會放下一行
 *uszPrintPosition:	有靠左、置中、靠右三種選擇(_PRINT_LEFT_、_PRINT_CENTER_、_PRINT_RIGHT_）
 * 
 * inYLength:		高度有多少像素單位(8像素為一單位)
 *
*/
int inNCCC_ESC_PRINT_Format_Auto_Change_Line(char* szString, char *szPackData)
{
	int	inDataLen = 0;
	int	inFontXLen = 0;		/* 該大小每個字x寬度*/
	int	inLineLen = 0;
	int	inStart = 0;
	int	inCurrentLen = 0;
	int	inTotalLen;
	int	inOffset;
	char	szPart[200 + 1];
	char    szPrintBuf[256];
	BOOL	fChinese;
	
	/* 總Byte數 */
	inTotalLen = strlen(szString);
	inFontXLen = 8;
	
	do
	{
		/* 先判斷是否為Ascii Code  char > 127 會是負數 */
		if (szString[inCurrentLen] < 0)
		{
			fChinese = VS_TRUE;
			inLineLen += 2 * inFontXLen;
			inOffset = 3;
		}
		else
		{
			fChinese = VS_FALSE;
			inLineLen += inFontXLen;
			inOffset = 1;
		}
		
		/* 往前移動 */
		inCurrentLen += inOffset;
		
		/* PB_CANVAS_X_SIZE = 384 */
		if (inLineLen > PB_CANVAS_X_SIZE	||	/* 超過邊緣 */
		    inCurrentLen == inTotalLen)			/* 印完全部 */
		{
			/* 要超過邊緣了 先退回一格 */
			if (inLineLen > PB_CANVAS_X_SIZE)
			{
				inCurrentLen -= inOffset;
			}
			
			/* 行內長度歸0 */
			inLineLen = 0;
			
			memset(szPart, 0x00, sizeof(szPart));
			memcpy(szPart, &szString[inStart], inCurrentLen - inStart);
			
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "11::%s\x0D\x0A", szPart);
			memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
			
			/* 起始點移動 */
			inStart = inCurrentLen;
		}
		else
		{
			
		}
			
	} while (inCurrentLen < inTotalLen);

	return (inDataLen);
}

/*
Function        :inNCCC_ESC_PRINT_FISC_Data
Date&Time       :2017/4/25 上午 11:19
Describe        :
*/
int inNCCC_ESC_PRINT_FISC_Data(TRANSACTION_OBJECT *pobTran, char *szPackData)
{
	int	i = 0;
	int	inDataLen = 0;
	char 	szPrintBuf[200 + 1] = {0}, szPrintBuf1[42 + 1] = {0}, szPrintBuf2[42 + 1] = {0}, szTemplate1[42 + 1] = {0}, szTemplate2[42 + 1] = {0};
	char	szProductCodeEnable[1 + 1] = {0};
	char	szStore_Stub_CardNo_Truncate_Enable[2 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
        memset(szTemplate1, 0x00, sizeof(szTemplate1));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	
	/* 發卡行代碼 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::發卡行代碼\x0D\x0A");
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	/* 發卡行代碼值 */
	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	memcpy(szPrintBuf1, &pobTran->srBRec.szFiscIssuerID[0], 3);
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", szPrintBuf1);
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	
	/* 卡別 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", "卡別(Card Type)");
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 卡別值 */
	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	/* 【需求單 - 106349】自有品牌判斷需求 */
	/* unpack電文就存存CardLabel modify by LingHsiung 2020/2/14 上午 11:07 */
	inFunc_PAD_ASCII(szPrintBuf1, pobTran->srBRec.szCardLabel, ' ', 12, _PADDING_RIGHT_);
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", szPrintBuf1);
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 卡號 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", "卡號(Card No.)");
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 卡號值 */
	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	strcpy(szPrintBuf1, pobTran->srBRec.szPAN);
	/* 商店聯卡號遮掩 */
	memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
	inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
	if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0 && pobTran->srBRec.uszTxNoCheckBit == VS_TRUE)
	{
		/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
		for (i = 6; i < (strlen(szPrintBuf1) - 4); i ++)
			szPrintBuf1[i] = 0x2A;
	}

	/* 過卡方式 */
	if (pobTran->srBRec.uszFiscTransBit != VS_TRUE)
	{
		if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
			strcat(szPrintBuf1,"(C)");
		else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
			strcat(szPrintBuf1, "(W)");
		else
		{
			if (pobTran->srBRec.uszManualBit == VS_TRUE)
			{
				/* 【需求單 - 105244】端末設備支援以感應方式進行退貨交易 */
				/* 電文轉Manual Keyin但是簽單要印感應的W */
				if (pobTran->srBRec.uszRefundCTLSBit == VS_TRUE)
					strcat(szPrintBuf1, "(W)");
				else
					strcat(szPrintBuf1,"(M)");
			}
			else
				strcat(szPrintBuf1,"(S)");
		}

	}
	else
	{
		if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
			strcat(szPrintBuf1, "(W)");
		else
			strcat(szPrintBuf1, "(C)");
	}

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", szPrintBuf1);
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 主機別 & 交易別 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", "主機別/交易類別(Host/Trans. Type)");
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 主機別值 & 交易別值 */
	memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
	inGetHostLabel(szPrintBuf2);
	
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);
	
	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	sprintf(szPrintBuf1, "%s %s %s", szPrintBuf2, szTemplate1, szTemplate2);
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", szPrintBuf1);
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 批次號碼 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", "批次號碼(Batch No.)");
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* Batch Num */
	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	sprintf(szPrintBuf1, "%03ld", pobTran->srBRec.lnBatchNum);
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", szPrintBuf1);

        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	/* 日期時間 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", "日期/時間(Date/Time)");
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	/* 日期時間值 */
	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	sprintf(szPrintBuf1, "%.4s/%.2s/%.2s %.2s:%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6], &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", szPrintBuf1);
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 序號 & 調閱編號 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "29::%s%s%s\x0D\x0A", "序號(Ref. No.)", _DATA_FLAG_, "調閱編號(Inv. No.)");
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 序號 & 調閱編號值 */
	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	/* 雖然電文RRN送12個byte，但RRN最後一碼是0x00，所以只看到11碼 */
	inFunc_PAD_ASCII(szPrintBuf1, pobTran->srBRec.szRefNo, ' ', 12, _PADDING_RIGHT_);
	memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
	sprintf(szPrintBuf2, "%06ld", pobTran->srBRec.lnOrgInvNum);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "29::%s%s%s\x0D\x0A", szPrintBuf1, _DATA_FLAG_, szPrintBuf2);
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	/* 調單編號 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", "調單編號(RRN NO.)");
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	/* 調單編號值 */
	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	memcpy(szPrintBuf1, pobTran->srBRec.szFiscRRN, _FISC_RRN_SIZE_);
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", szPrintBuf1);
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 櫃號 */
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	inGetStoreIDEnable(szTemplate1);
	if ((memcmp(&szTemplate1[0], "Y", 1) == 0) && (strlen(pobTran->srBRec.szStoreID) > 0))
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_026_TAKA_, _CUSTOMER_INDICATOR_SIZE_) ||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
		{
			sprintf(szPrintBuf, "11::%s\x0D\x0A", "品群碼(Store ID)");
		}
		else
		{
			sprintf(szPrintBuf, "11::%s\x0D\x0A", "櫃號(Store ID)");
		}
		
		memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);

		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
		{
			/* 原品群碼*/
			sprintf(szPrintBuf1, "%s", pobTran->srBRec.szStoreID);
			
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "11::%s\x0D\x0A", szPrintBuf1);

			memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
			
			/* hardcode */
			char	szPOS_ID[6 + 1] = {0};
			memset(szPOS_ID, 0x00, sizeof(szPOS_ID));
			inGetPOS_ID(szPOS_ID);
			
			memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
			memcpy(szPrintBuf1, _HARDCODE_STORE_ID_CUS_021_TAKAWEL_, 12);
			memcpy(&szPrintBuf1[12], szPOS_ID, 6);
		}
		else
		{
			if (strlen(pobTran->srBRec.szStoreID) > 42)
				memcpy(szPrintBuf1, pobTran->srBRec.szStoreID, 42);
			else
				strcpy(szPrintBuf1, pobTran->srBRec.szStoreID);
		}
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "11::%s\x0D\x0A", szPrintBuf1);
		memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
	}

	/* 產品代碼 */
	inGetProductCodeEnable(szProductCodeEnable);
	if (memcmp(szProductCodeEnable, "Y", 1) == 0)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "11::%s\x0D\x0A", "產品代碼(Product Code)");
                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "11::%s\x0D\x0A", pobTran->srBRec.szProductCode);
                memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
	}
	
	/* 商店聯卡號遮掩 */
	memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
	inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
	if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
	    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
	    strlen(pobTran->srBRec.szTxnNo) > 0)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "11::%s\x0D\x0A", "交易編號(Transaction No.):");
		memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "11::%s\x0D\x0A", pobTran->srBRec.szTxnNo);
		memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
	}

        /* 斷行 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        strcpy(szPrintBuf, "L::1\x0D\x0A");
        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	return (inDataLen);
}

/*
Function        :inNCCC_ESC_PRINT_FISC_Amount
Date&Time       :2017/4/25 下午 2:11
Describe        :
 */
int inNCCC_ESC_PRINT_FISC_Amount(TRANSACTION_OBJECT *pobTran, char *szPackData)
{
	int	inDataLen = 0;
        char    szPrintBuf[84 + 1], szTemplate[42 + 1];

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));
	
	/* 橫式 */
        /* 負向交易 */
	if(pobTran->srBRec.uszVOIDBit == VS_TRUE)
	{
		/* 橫式 */
                /* 金額 */
                /* 取消退貨是正數 */
                if (pobTran->srBRec.inOrgCode == _FISC_REFUND_)
                {
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
			sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
			inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, _PADDING_LEFT_);

                        /* 把前面的字串和數字結合起來 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "22::總計(Total)%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);
			memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
                }
		else
		{
			/* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
			sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
			inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, _PADDING_LEFT_);

                        /* 把前面的字串和數字結合起來 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "22::總計(Total)%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);
			memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
		}
	}
	else
	{
		
		/* 初始化 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

		/* 將NT$ ＋數字塞到szTemplate中來inpad，退貨要負數 */
		if (pobTran->srBRec.inCode == _FISC_REFUND_)
		{
			sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
		}
		else
		{
			sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
		}

		inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, _PADDING_LEFT_);

		/* 把前面的字串和數字結合起來 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "22::總計(Total)%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);
		memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
		
	}
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        strcpy(szPrintBuf, "L::1\x0D\x0A");
        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	return (inDataLen);
}

/*
Function        :inCREDIT_PRINT_HG_Multi_Data_ByBuffer
Date&Time       :2017/4/26 下午 2:19
Describe        :
*/
int inNCCC_ESC_PRINT_HG_Multi_Data(TRANSACTION_OBJECT *pobTran, char *szPackData)
{
	int	inDataLen = 0;
        char    szPrintBuf[84 + 1], szPrintBuf1[84 + 1], szTemplate[42 + 1], szTemplate1[42 + 1];
	
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));
        
	/* 快樂購聯合集點卡 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "14::%s\x0D\x0A", "快樂購聯合集點卡");
	memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
	inDataLen += strlen(szPrintBuf);
	
        /* 交易類別 */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        memset(szTemplate1, 0x00, sizeof(szTemplate1));
        memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
        vdCREDIT_PRINT_HG_GetTransactionType(pobTran, szTemplate, szTemplate1);
        sprintf(szPrintBuf1, "%s%s", szTemplate, szTemplate1);
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "14::%s\x0D\x0A", szPrintBuf1);
	memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
	inDataLen += strlen(szPrintBuf);
	
	/* HG卡號 */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        strcpy(szTemplate, pobTran->srBRec.szHGPAN);
	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
        sprintf(szPrintBuf1, "ＨＧ卡號　 %s", szTemplate);
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "14::%s\x0D\x0A", szPrintBuf1);
	memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
	inDataLen += strlen(szPrintBuf);
	
	if (pobTran->srBRec.uszVOIDBit == VS_FALSE)
	{
		switch (pobTran->srBRec.inHGCode)
		{
			case _HG_POINT_CERTAIN_CREDIT_:
			case _HG_POINT_CERTAIN_CUP_:
			case _HG_POINT_CERTAIN_CREDIT_INSIDE_:
				/* 扣抵點數 */                               
				/* 將數字塞到szTemplate中來inpad */
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%ld 點", pobTran->srBRec.lnHGTransactionPoint);
				
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "22::扣抵點數%s%s\x0D\x0A", _DATA_FLAG_, szTemplate1);
                                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                                inDataLen += strlen(szPrintBuf);
				/* 剩餘點數 */
				/* 將數字塞到szTemplate中來inpad */
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%ld 點", pobTran->srBRec.lnHGBalancePoint);
				
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "22::剩餘點數%s%s\x0D\x0A", _DATA_FLAG_, szTemplate1);
				memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);
				break;
			case _HG_ONLINE_REDEEM_CREDIT_:
			case _HG_ONLINE_REDEEM_CREDIT_INSIDE_:
			case _HG_ONLINE_REDEEM_CUP_:
				/* 商品金額 */
				/* 將數字塞到szTemplate中來inpad */
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnHGRedeemAmount));
				inFunc_Amount_Comma(szTemplate1, "NT$", ' ', _SIGNED_NONE_, 16, _PADDING_LEFT_);
				
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "22::商品金額%s%s\x0D\x0A", _DATA_FLAG_, szTemplate1);
				memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);
				/* 點數扣抵金額 */
				/* 將數字塞到szTemplate中來inpad */
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGRedeemAmount);
				inFunc_Amount_Comma(szTemplate1, "NT$", ' ', _SIGNED_NONE_, 16, _PADDING_LEFT_);
				
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "22::點數扣抵金額%s%s\x0D\x0A", _DATA_FLAG_, szTemplate1);
				memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);
				/* 扣抵點數 */
				/* 將數字塞到szTemplate中來inpad */
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGTransactionPoint);
				
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "22::扣抵點數%s%s\x0D\x0A", _DATA_FLAG_, szTemplate1);
				memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);
				/* 剩餘點數 */
				/* 將數字塞到szTemplate中來inpad */
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGBalancePoint);
				
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "22::剩餘點數%s%s\x0D\x0A", _DATA_FLAG_, szTemplate1);
				memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);
				break;

			default:
				break;
		}
	}
	/* 負向交易時 */
	else
	{
		switch (pobTran->srBRec.inHGCode)
		{
			case _HG_POINT_CERTAIN_CREDIT_:
			case _HG_POINT_CERTAIN_CUP_:
			case _HG_POINT_CERTAIN_CREDIT_INSIDE_:
				/* 扣抵點數 */                               
				/* 將數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "-%ld", pobTran->srBRec.lnHGTransactionPoint);
				
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "22::扣抵點數%s%s\x0D\x0A", _DATA_FLAG_, szTemplate1);
				memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);
				/* 剩餘點數 */
				/* 將數字塞到szTemplate中來inpad */
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGBalancePoint);
				
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "22::剩餘點數%s%s\x0D\x0A", _DATA_FLAG_, szTemplate1);
				memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);
				break;
			case _HG_ONLINE_REDEEM_CREDIT_:
			case _HG_ONLINE_REDEEM_CREDIT_INSIDE_:
			case _HG_ONLINE_REDEEM_CUP_:
				/* 商品金額 */
				/* 將數字塞到szTemplate中來inpad */
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%ld", (0 - (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnHGRedeemAmount)));
				inFunc_Amount_Comma(szTemplate1, "NT$", ' ', _SIGNED_MINUS_, 16, _PADDING_LEFT_);
				
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "22::商品金額%s%s\x0D\x0A", _DATA_FLAG_, szTemplate1);
				memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);
				/* 點數扣抵金額 */
				/* 將數字塞到szTemplate中來inpad */
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%ld", (0 - pobTran->srBRec.lnHGRedeemAmount));
				inFunc_Amount_Comma(szTemplate1, "NT$", ' ', _SIGNED_MINUS_, 16, _PADDING_LEFT_);
				
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "22::點數扣抵金額%s%s\x0D\x0A", _DATA_FLAG_, szTemplate1);
				memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);
				/* 扣抵點數 */
				/* 將數字塞到szTemplate中來inpad */
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "-%ld", pobTran->srBRec.lnHGTransactionPoint);
				
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "22::扣抵點數%s%s\x0D\x0A", _DATA_FLAG_, szTemplate1);
				memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);
				/* 剩餘點數 */
				/* 將數字塞到szTemplate中來inpad */
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGBalancePoint);
				
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "22::剩餘點數%s%s\x0D\x0A", _DATA_FLAG_, szTemplate1);
				memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);
				break;

			default:
				break;
		}
	}
        
        /* 斷行 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        strcpy(szPrintBuf, "L::1\x0D\x0A");
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
        
        return (inDataLen);
}

/*
Function        :inNCCC_ESC_PRINT_ReceiptEND_027
Date&Time       :2022/7/11 下午 3:12
Describe        :中華電信要列印警語
*/
int inNCCC_ESC_PRINT_ReceiptEND_027(TRANSACTION_OBJECT *pobTran, char *szPackData)
{
	int	inDataLen = 0;
	char	szPrintBuf[84 + 1], szTemplate[42 + 1] = {0};
	char	szPrtNotice[1 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
        
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	if (pobTran->srBRec.inCode == _VOID_ || pobTran->srBRec.inCode == _CUP_VOID_ ||
	    pobTran->srBRec.inCode == _REFUND_ || pobTran->srBRec.inCode == _INST_REFUND_ ||
	    pobTran->srBRec.inCode == _REDEEM_REFUND_ || pobTran->srBRec.inCode == _CUP_REFUND_ ||
	    pobTran->srBRec.inCode == _CUP_PRE_AUTH_VOID_ || pobTran->srBRec.inCode == _CUP_PRE_COMP_VOID_ ||
	    pobTran->srBRec.inCode == _CUP_MAIL_ORDER_REFUND_) 
	/* 取消銀聯預授與取消銀聯預授完成不需印警語  */
	{

	}
	else
	{
		/* 斷行 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		strcpy(szPrintBuf, "L::1\x0D\x0A");
		memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
	
		/* 退貨請攜帶本簽單及原交易卡片與商品至營業櫃檯辦理 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "12::%s\x0D\x0A", "退貨請攜帶本簽單及原交易");
		memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "12::%s\x0D\x0A", "卡片與商品至營業櫃檯辦理");
		memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
		
		/* 斷行 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		strcpy(szPrintBuf, "L::1\x0D\x0A");
		memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
	}
	
	/* 簽名欄 */
	/* 免簽名 */
	/* 小費一律簽名 */
	if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE && pobTran->srBRec.inCode != _TIP_)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "12::%s\x0D\x0A", "免　　簽　　名");

		memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
	}
	/* 要簽名 */
	else
	{
		/* 有在signpad簽名 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
		inFunc_ComposeFileName_InvoiceNumber(pobTran, szTemplate, _PICTURE_FILE_EXTENSION_, 6);

		if (inFILE_Check_Exist((unsigned char *)szTemplate) == VS_SUCCESS)
		{
			/* 電子簽名 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "G::%s\x0D\x0A", _ESC_BMP_SIGNATURE_);
			memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
		}

	}

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "14::%s\x0D\x0A", "X:__________________________________");
	memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
	inDataLen += strlen(szPrintBuf);

	/* 持卡人姓名 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "11::%s\x0D\x0A", pobTran->srBRec.szCardHolder);
	memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
	inDataLen += strlen(szPrintBuf);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "12::%s\x0D\x0A", "持卡人簽名(特店存根聯)");
	memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
	inDataLen += strlen(szPrintBuf);
		
	/* 列印警示語 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "12::%s\x0D\x0A", "I AGREE TO PAY TOTAL AMOUNT");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "12::%s\x0D\x0A", "ACCORDING TO CARD ISSUER AGREEMENT");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* Print Notice */
	memset(szPrtNotice, 0x00, sizeof(szPrtNotice));
	inGetPrtNotice(szPrtNotice);
	if(memcmp(szPrtNotice, "Y", strlen("Y")) == 0)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "G::%s\x0D\x0A", _ESC_BMP_NOTICE_LOGO_);
                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
	}
	
        return (inDataLen);
}

/*
Function        :inNCCC_ESC_PRINT_DCC_ReceiptEND_061
Date&Time       :2022/9/6 下午 4:50
Describe        :
*/
int inNCCC_ESC_PRINT_DCC_ReceiptEND_061(TRANSACTION_OBJECT *pobTran, char *szPackData)
{
	int	inTempLen = 0;
	int	inDataLen = 0;
	char	szPrtNotice[1 + 1];
	char	szPrintBuf[200 + 1], szTemplate[100 + 1];
	char	szTempBuff[1000] = {};

	/* Disclaimer */
	if ((!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_VISA_, strlen(_CARD_TYPE_VISA_))) && 
	    (pobTran->srBRec.inCode == _SALE_		|| 
	     pobTran->srBRec.inCode == _TIP_		|| 
	     pobTran->srBRec.inCode == _PRE_COMP_))
	{
		memset(szTempBuff, 0x00, sizeof(szTempBuff));
		inTempLen = inNCCC_ESC_PRINT_DCC_Disclaimer_061_VISA(pobTran, szTempBuff);
		if (inTempLen > 0)
		{
		        memcpy(&szPackData[inDataLen], &szTempBuff[0], inTempLen);
                        inDataLen += inTempLen;
		}
	}
	else if ((!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_))) && 
		 (pobTran->srBRec.inCode == _SALE_	|| 
		  pobTran->srBRec.inCode == _TIP_	|| 
		  pobTran->srBRec.inCode == _PRE_COMP_))
	{
		memset(szTempBuff, 0x00, sizeof(szTempBuff));
		inTempLen = inNCCC_ESC_PRINT_DCC_Disclaimer_061_MASTERCARD(pobTran, szTempBuff);
		if (inTempLen > 0)
		{
		        memcpy(&szPackData[inDataLen], &szTempBuff[0], inTempLen);
                        inDataLen += inTempLen;
		}
	}
	
	if (pobTran->srBRec.inPrintOption == _PRT_MERCH_)
        {
		if (pobTran->srBRec.uszDCCTransBit == VS_TRUE && pobTran->srBRec.inCode == _TIP_)
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "11::%s\x0D\x0A", "");
			memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "11::%s\x0D\x0A", "Cardholder expressly agrees to the Transaction ");
			memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "11::%s\x0D\x0A", "Receipt Information by marking the “accept box”");
			memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "11::%s\x0D\x0A", "below.");
			memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
			
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "11::%s\x0D\x0A", "[ ] ACCEPT");
			memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
		}
	}
	else if (pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
	{
		if (pobTran->srBRec.uszDCCTransBit == VS_TRUE && pobTran->srBRec.inCode == _TIP_)
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "11::%s\x0D\x0A", "");
			memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "11::%s\x0D\x0A", "Cardholder expressly agrees to the Transaction ");
			memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "11::%s\x0D\x0A", "Receipt Information by marking the “accept box”");
			memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "11::%s\x0D\x0A", "below.");
			memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
			
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "11::%s\x0D\x0A", "[ ] ACCEPT");
			memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
		}
	}
	else
	{
		
	}

	/* 簽名欄 */
	/* 免簽名 */
	/* 小費一律簽名 */
	if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE && pobTran->srBRec.inCode != _TIP_)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "12::%s\x0D\x0A", "免　　簽　　名");

		memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
	}
	/* 要簽名 */
	else
	{
		/* 有在signpad簽名 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
		inFunc_ComposeFileName_InvoiceNumber(pobTran, szTemplate, _PICTURE_FILE_EXTENSION_, 6);

		if (inFILE_Check_Exist((unsigned char *)szTemplate) == VS_SUCCESS)
		{
			/* 電子簽名 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "G::%s\x0D\x0A", _ESC_BMP_SIGNATURE_);
			memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
		}

	}

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "14::%s\x0D\x0A", "X:__________________________________");
	memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
	inDataLen += strlen(szPrintBuf);

	/* 持卡人姓名 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "11::%s\x0D\x0A", pobTran->srBRec.szCardHolder);
	memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
	inDataLen += strlen(szPrintBuf);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "12::%s\x0D\x0A", "持卡人簽名(特店存根聯)");
	memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
	inDataLen += strlen(szPrintBuf);
		
	/* 列印警示語 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "12::%s\x0D\x0A", "I AGREE TO PAY TOTAL AMOUNT");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "12::%s\x0D\x0A", "ACCORDING TO CARD ISSUER AGREEMENT");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* Print Notice */
	memset(szPrtNotice, 0x00, sizeof(szPrtNotice));
	inGetPrtNotice(szPrtNotice);
	if(memcmp(szPrtNotice, "Y", strlen("Y")) == 0)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "G::%s\x0D\x0A", _ESC_BMP_NOTICE_LOGO_);
                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
	}

	return (inDataLen);
}

/*
Function        :inNCCC_ESC_PRINT_DCC_Disclaimer_061_VISA
Date&Time       :2022/9/6 下午 4:57
Describe        :
*/
int inNCCC_ESC_PRINT_DCC_Disclaimer_061_VISA(TRANSACTION_OBJECT *pobTran, char *szPackData)
{
	int	inDataLen = 0;
	char	szPrintBuf[200 + 1] = {0};
	
	inDataLen = 0;
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "11::%s\x0D\x0A", "VISA Legal Notice(Disclaimer Text): I have");
	memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
	inDataLen += strlen(szPrintBuf);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "11::%s\x0D\x0A", "been offered a choice of currencies and");
	memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
	inDataLen += strlen(szPrintBuf);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "11::%s\x0D\x0A", "agree to pay in the selected Transaction");
	memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
	inDataLen += strlen(szPrintBuf);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "11::%s\x0D\x0A", "Currency. Dynamic Currency Conversion");
	memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
	inDataLen += strlen(szPrintBuf);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "11::%s\x0D\x0A", "(DCC) is offered by the Merchant.");
	memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
	inDataLen += strlen(szPrintBuf);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "11::%s\x0D\x0A", "Cardholder expressly agrees to the");
	memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
	inDataLen += strlen(szPrintBuf);
	
	if (pobTran->srBRec.inPrintOption == _PRT_MERCH_ || pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
	{	
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "11::%s\x0D\x0A", "Transaction Receipt Information by marking");
		memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "11::%s\x0D\x0A", "the “accept box” below.");
		memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
	}
	else
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "11::%s\x0D\x0A", "Transaction Receipt Information");
		memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
	}
			
	return (inDataLen);
}

/*
Function        :inNCCC_ESC_PRINT_DCC_Disclaimer_061_MASTERCARD
Date&Time       :2022/9/6 下午 4:57
Describe        :
*/
int inNCCC_ESC_PRINT_DCC_Disclaimer_061_MASTERCARD(TRANSACTION_OBJECT *pobTran, char *szPackData)
{
	int	inDataLen = 0;
	char	szPrintBuf[200 + 1] = {0};
	
	inDataLen = 0;
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "11::%s\x0D\x0A", "M/C Legal Notice: I have chosen not to use");
	memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
	inDataLen += strlen(szPrintBuf);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "11::%s\x0D\x0A", "the MasterCard currency conversion process");
	memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
	inDataLen += strlen(szPrintBuf);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "11::%s\x0D\x0A", "and I will have no recourse against");
	memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
	inDataLen += strlen(szPrintBuf);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "11::%s\x0D\x0A", "MasterCard concerning the currency");
	memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
	inDataLen += strlen(szPrintBuf);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "11::%s\x0D\x0A", "conversion or its disclosure. Cardholder");
	memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
	inDataLen += strlen(szPrintBuf);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "11::%s\x0D\x0A", "expressly agrees to the Transaction");
	memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
	inDataLen += strlen(szPrintBuf);
	
	if (pobTran->srBRec.inPrintOption == _PRT_MERCH_ || pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
	{	
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "11::%s\x0D\x0A", "Receipt Information by marking the");
		memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "11::%s\x0D\x0A", "accept box” below.");
		memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
	}
	else
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "11::%s\x0D\x0A", "Receipt Information.");
		memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
	}
			
	return (inDataLen);
}

/*
Function        :inNCCC_ESC_PRINT_LOGO_041_043
Date&Time       :2022/9/29 下午 4:11
Describe        :
*/
int inNCCC_ESC_PRINT_LOGO_041_043(TRANSACTION_OBJECT *pobTran, char *szPackData)
{
	int     inDataLen = 0;
        char    szPrintBuf[200 + 1];	/* inESC_PRINT_LOGO裡用的buffer，每組完一個東西就清空一次 */
	char	szPrtMerchantLogo[2 + 1];
	char	szPrtMerchantName[2 + 1];

	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_PRINT_LOGO() START!");
	
	/* Print NCCC LOGO 384*60 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "G::%s\x0D\x0A", _ESC_BMP_NCCC_LOGO_);
	/* 丟給外面的Buffer */
        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
	/* 位置移到szPackData最後面，以便放其他欄位 */
        inDataLen += strlen(szPrintBuf);

	/* Print MERCHANT LOGO 384*180 */
	memset(szPrtMerchantLogo, 0x00, sizeof(szPrtMerchantLogo));
	inGetPrtMerchantLogo(szPrtMerchantLogo);
		
	if (memcmp(&szPrtMerchantLogo[0], "Y", 1) == 0)
	{
	        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "G::%s\x0D\x0A", _ESC_BMP_MERCHANT_LOGO_);
                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
	}

	/* Print MERCHANT NAME 384*90 */
	memset(szPrtMerchantName, 0x00, sizeof(szPrtMerchantName));
	inGetPrtMerchantName(szPrtMerchantName);
	if (memcmp(&szPrtMerchantName[0], "Y", 1) == 0)
	{
	        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "G::%s\x0D\x0A", _ESC_BMP_NAME_LOGO_);
                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
	}
	
	if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inNCCC_ESC_PRINT_LOGO() END!");

	return (inDataLen);
}

/*
Function        :inNCCC_ESC_PRINT_DATA_041_043
Date&Time       :2022/9/29 下午 4:13
Describe        :
*/
int inNCCC_ESC_PRINT_DATA_041_043(TRANSACTION_OBJECT *pobTran, char *szPackData)
{
	int	i = 0;
	int	inDataLen = 0;
	int	inTempLen = 0;
	int	inUpperAmount = 0;
	int	inLowerAmount = 0;
	char	szPrintBuf[200 + 1] = {0}, szPrintBuf1[100 + 1] = {0}, szPrintBuf2[100 + 1] = {0}, szTemplate[100 + 1] = {0}, szTemplate1[100 + 1] = {0}, szTemplate2[100 + 1] = {0};
	char	szProductCodeEnable[1 + 1];
	char	szStore_Stub_CardNo_Truncate_Enable[2 + 1];
	char	szExamBit[10 + 1] = {0};

	/* 城市別 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", "城市別(City)");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 城市名 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetCityName(szTemplate);
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", szTemplate);

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 卡別 & 檢查碼 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "29::%s%s%s\x0D\x0A", "卡別(Card Type)", _DATA_FLAG_, "檢查碼(Check No.)");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 卡別值 & 檢查碼值 */
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	/* 【需求單 - 106349】自有品牌判斷需求 */
	/* unpack電文就存存CardLabel modify by LingHsiung 2020/2/14 上午 11:07 */
	strcpy(szTemplate1, pobTran->srBRec.szCardLabel);

	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/28 下午 2:42 */
	if (strlen(pobTran->srBRec.szCheckNO) > 0)
	{
		strcpy(szTemplate2, pobTran->srBRec.szCheckNO);
	}
	else
	{
		inCARD_ExpDateEncryptAndDecrypt(pobTran, "", szTemplate2, _EXP_ENCRYPT_);
	}

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "29::%s%s%s\x0D\x0A", szTemplate1, _DATA_FLAG_, szTemplate2);

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 卡號掩飾 NCCC 規格以為準 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", "卡號(Card No.)");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	memset(szTemplate, 0x00, sizeof(szTemplate));
	strcpy(szTemplate, pobTran->srBRec.szPAN);
	
	/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
	/* 商店聯卡號遮掩 */
	memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
	inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
	/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/8 下午 1:55 */
	if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
	{
		/* Uny交易不受TMS開關限制，且獲取卡號時已遮掩 */
	}
	else if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0 && pobTran->srBRec.uszTxNoCheckBit == VS_TRUE)
	{
		/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
		for (i = 6; i < (strlen(szTemplate) - 4); i ++)
			szTemplate[i] = 0x2A;
	}

	/* 過卡方式 */
	if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
	{
		if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
			strcat(szTemplate, "(W)");
		else
			strcat(szTemplate, "(C)");
	}
	else if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
	{
		strcat(szTemplate, "(Q)");
	}
	else if (pobTran->srBRec.uszCUPEMVQRCodeBit == VS_TRUE)
	{
		strcat(szTemplate, "(Q)");
	}
	else
	{
		if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
			strcat(szTemplate,"(C)");
		else if (pobTran->srBRec.uszMobilePayBit == VS_TRUE)
			strcat(szTemplate, "(T)");
		else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
			strcat(szTemplate, "(W)");
		else
		{
			if (pobTran->srBRec.uszManualBit == VS_TRUE)
			{
				/* 【需求單 - 105244】端末設備支援以感應方式進行退貨交易 */
				/* 電文轉Manual Keyin但是簽單要印感應的W */
				if (pobTran->srBRec.uszRefundCTLSBit == VS_TRUE)
					strcat(szTemplate, "(W)");
				else
					strcat(szTemplate,"(M)");
			}
			else
				strcat(szTemplate,"(S)");
		}
	}

       	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", szTemplate);

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 主機別 & 交易類別 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", "主機別/交易類別(Host/Trans. Type)");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 主機別值 & 交易類別值*/
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	inGetHostLabel(szTemplate);
	inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);

	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	sprintf(szPrintBuf1, "%s %s %s", szTemplate, szTemplate1, szTemplate2);
	
	if (strlen(szTemplate1) > 0)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "11::%s\x0D\x0A", szPrintBuf1);

		memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
	}
		
	
	/* 批次號碼(Batch No.) & 授權碼 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "29::%s%s%s\x0D\x0A", "批次號碼(Batch No.)", _DATA_FLAG_, "授權碼(Auth Code)");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%03ld", pobTran->srBRec.lnBatchNum);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "29::%s%s%s\x0D\x0A", szTemplate, _DATA_FLAG_, pobTran->srBRec.szAuthCode);

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	/* 回覆碼 */
	if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
	{
	        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "11::%s\x0D\x0A", "回覆碼(Resp. Code)");

                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);

		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(&szTemplate[0], &pobTran->srBRec.szRespCode[0], 2);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "11::%s\x0D\x0A", szTemplate);

                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
	}

	/* 日期 & 時間 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", "日期/時間(Date/Time)");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 日期值 時間值 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%.4s/%.2s/%.2s %.2s:%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6], &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
		
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "14::%s\x0D\x0A", szTemplate);

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 序號 & 調閱編號 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "29::%s%s%s\x0D\x0A", "序號(Ref. No.)", _DATA_FLAG_, "調閱編號(Inv. No.)");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 序號值 & 調閱編號值 */
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	inFunc_PAD_ASCII(szTemplate1, pobTran->srBRec.szRefNo, ' ', 25, _PADDING_RIGHT_);

	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	sprintf(szTemplate2, "%06ld", pobTran->srBRec.lnOrgInvNum);

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "29::%s%s%s\x0D\x0A", szTemplate1, _DATA_FLAG_, szTemplate2);

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 櫃號 */
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	inGetStoreIDEnable(szTemplate1);
	if ((memcmp(&szTemplate1[0], "Y", 1) == 0) && (strlen(pobTran->srBRec.szStoreID) > 0))
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "11::%s\x0D\x0A", "櫃號(Store ID)");

		memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
		
		/* 櫃號值 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		sprintf(szTemplate1, "%s", pobTran->srBRec.szStoreID);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "11::%s\x0D\x0A", szTemplate1);

		memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
	}
	
	/* 產品代碼 */
	inGetProductCodeEnable(szProductCodeEnable);
	if (memcmp(szProductCodeEnable, "Y", 1) == 0)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "11::%s\x0D\x0A", "產品代碼(Product Code)");

                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "11::%s\x0D\x0A", pobTran->srBRec.szProductCode);

                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
	}
	
	/* TC */
	/* 驗測開關 */
	memset(szExamBit, 0x00, sizeof(szExamBit));
	inGetExamBit(szExamBit);
	/* Uny交易 */
	/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/8 下午 1:55 */
	if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
	{
		/* 取消或退貨，只印數字交易碼 */
		/* 最小字體 */
		if (pobTran->srBRec.uszVOIDBit == VS_TRUE ||
		    pobTran->srBRec.inCode == _REFUND_ || pobTran->srBRec.inCode == _INST_REFUND_ || pobTran->srBRec.inCode == _REDEEM_REFUND_)
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			sprintf(szTemplate1, "交易碼 %s", pobTran->srBRec.szUnyTransCode);
			sprintf(szPrintBuf, "11::%s\x0D\x0A", szTemplate1);

			memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
		}
		/* 正向交易，要印條碼和數字 */
		else
		{

			if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
			{
				/* 一維條碼 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%s", pobTran->srBRec.szUnyTransCode);
				sprintf(szPrintBuf, "U::%s\x0D\x0A", szTemplate1);
				
				memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);

				/* 數字 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "交易碼(退貨請掃描) %s", pobTran->srBRec.szUnyTransCode);
				sprintf(szPrintBuf, "11::%s\x0D\x0A", szTemplate1);

				memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);
			}
			else
			{
				/* 一維條碼 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%s", pobTran->srBRec.szUnyTransCode);
				sprintf(szPrintBuf, "U::%s\x0D\x0A", szTemplate1);

				memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);

				/* 數字 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "交易碼 %s", pobTran->srBRec.szUnyTransCode);
				sprintf(szPrintBuf, "11::%s\x0D\x0A", szTemplate1);

				memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);
			}
		}
	}
	/* 實體卡交易和掃碼交易互斥 */
	else
	{
		if (pobTran->srBRec.inChipStatus == _EMV_CARD_ || pobTran->srBRec.uszContactlessBit == VS_TRUE)
		{
			/* 感應磁條 */
			if (!memcmp(pobTran->srBRec.szAuthCode, "VLP", 3)				|| 
			    !memcmp(pobTran->srBRec.szAuthCode, "JCB", 3)				||
			     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE		|| 
			     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_64_NEWJSPEEDY_MSD		||
			     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE)
			{
				/* 商店聯卡號遮掩 */
				memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
				inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
				if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
				    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
				    strlen(pobTran->srBRec.szTxnNo) > 0)
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "11::%s\x0D\x0A", "交易編號(Transaction No.):");
					memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
					inDataLen += strlen(szPrintBuf);

					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "11::%s\x0D\x0A", pobTran->srBRec.szTxnNo);
					memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
					inDataLen += strlen(szPrintBuf);
				}
			}
			else
			{
				if (pobTran->srBRec.inPrintOption == _PRT_MERCH_)
				{
					if (pobTran->srEMVRec.in9F26_ApplCryptogramLen > 0)
					{
						memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
						sprintf(szPrintBuf1, "%02X%02X%02X%02X%02X%02X%02X%02X",
										pobTran->srEMVRec.usz9F26_ApplCryptogram[0],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[1],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[2],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[3],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[4],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[5],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[6],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[7]);

						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "23::TC%s%s\x0D\x0A", _DATA_FLAG_, szPrintBuf1);

						memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
						inDataLen += strlen(szPrintBuf);
					}

					/* 商店聯卡號遮掩 */
					memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
					inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
					if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
					    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
					    strlen(pobTran->srBRec.szTxnNo) > 0)
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "11::%s\x0D\x0A", "交易編號(Transaction No.):");
						memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
						inDataLen += strlen(szPrintBuf);

						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "11::%s\x0D\x0A", pobTran->srBRec.szTxnNo);
						memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
						inDataLen += strlen(szPrintBuf);
					}

					/* M/C交易列印AP Lable (START) */
					if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)))
					{
						if (pobTran->srEMVRec.in50_APLabelLen > 0)
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							strcat(szPrintBuf, "AP Label:");
							memset(szTemplate, 0x00, sizeof(szTemplate));
							strcpy(szTemplate, (char*)pobTran->srEMVRec.usz50_APLabel);
							inUpperAmount = inFunc_Get_UpperCase_Char(szTemplate);
							inLowerAmount = strlen(szTemplate) - inUpperAmount;
							inTempLen =  _ESC_AP_LABEL_TOTAL_SPACE_ - (_ESC_UPPER_MULTIPLE_ * inUpperAmount + _ESC_LOWER_MULTIPLE_ * inLowerAmount);
							for (i = 0; i < inTempLen; i++)
							{
								strcat(szPrintBuf, " ");
							}
							strcat(szPrintBuf, szTemplate);
						}
						else
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							strcat(szPrintBuf, "AP Label:");
							strcat(szPrintBuf, "\x20");
						}
					}
					/* M/C交易列印AP Lable (END) */
					else if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)))
					{
						if (pobTran->srEMVRec.in50_APLabelLen > 0)
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							strcat(szPrintBuf, "AP Label:");
							memset(szTemplate, 0x00, sizeof(szTemplate));
							strcpy(szTemplate, (char*)pobTran->srEMVRec.usz50_APLabel);
							inUpperAmount = inFunc_Get_UpperCase_Char(szTemplate);
							inLowerAmount = strlen(szTemplate) - inUpperAmount;
							inTempLen =  _ESC_AP_LABEL_TOTAL_SPACE_ - (_ESC_UPPER_MULTIPLE_ * inUpperAmount + _ESC_LOWER_MULTIPLE_ * inLowerAmount);
							for (i = 0; i < inTempLen; i++)
							{
								strcat(szPrintBuf, " ");
							}
							strcat(szPrintBuf, szTemplate);
						}
						else
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							strcat(szPrintBuf, "AP Label:");
							strcat(szPrintBuf, "\x20");
						}
					}
					/* 驗測要印AP Label 和 AID */
					else if (szExamBit[0] == '1')
					{
						if (pobTran->srEMVRec.in50_APLabelLen > 0)
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							strcat(szPrintBuf, "AP Label:");
							memset(szTemplate, 0x00, sizeof(szTemplate));
							strcpy(szTemplate, (char*)pobTran->srEMVRec.usz50_APLabel);
							inUpperAmount = inFunc_Get_UpperCase_Char(szTemplate);
							inLowerAmount = strlen(szTemplate) - inUpperAmount;
							inTempLen =  _ESC_AP_LABEL_TOTAL_SPACE_ - (_ESC_UPPER_MULTIPLE_ * inUpperAmount + _ESC_LOWER_MULTIPLE_ * inLowerAmount);
							for (i = 0; i < inTempLen; i++)
							{
								strcat(szPrintBuf, " ");
							}
							strcat(szPrintBuf, szTemplate);
						}
						else
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							strcat(szPrintBuf, "AP Label:");
							strcat(szPrintBuf, "\x20");
						}
					}
					else
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						strcat(szPrintBuf, "\x20");
						strcat(szPrintBuf, "\x20");
					}
					memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
					sprintf(szPrintBuf2, szPrintBuf);
					
					/* AID */
					if (strlen(pobTran->srBRec.szCUP_EMVAID) > 0)
					{
						memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
						sprintf(szPrintBuf1, "AID:%s", pobTran->srBRec.szCUP_EMVAID);
						inFunc_PAD_ASCII(szPrintBuf1, szPrintBuf1, ' ', _ESC_AID_TOTAL_LEN_, _PADDING_RIGHT_);
						
						if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
						{
							/* CUP晶片要印 */
							if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
							else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)) == 0)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
							/* 【需求單-109121】Mastercard交易簽單列印AID需求 by Russell 2020/7/23 下午 4:23 */
							else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)) == 0)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
							/* 【需求單-111290】VISA置換Logo、DISCOVER簽單新增AID欄位、AE簽單新增AID和AP Label欄位需求 by Russell 2022/12/29 下午 5:57 */
							else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)) == 0 &&
								pobTran->srBRec.uszUCARDTransBit != VS_TRUE)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
							/* 驗測要印AP Label 和 AID */
							else if (szExamBit[0] == '1')
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
						}
						/* 銀聯閃付 */
						else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
						{
							if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
							/* AE晶片要印出AID */
							else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_50_EXPRESSSPAY || 
								 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
							else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE || 
							         pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_21_PAYPASS_MCHIP)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
							else if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_42_DPAS	|| pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_43_DISCOVER_DPAS_MAG_STRIPE) &&
								pobTran->srBRec.uszUCARDTransBit != VS_TRUE)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
							/* 驗測要印AP Label 和 AID */
							else if (szExamBit[0] == '1')
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
						}
					}
				}
			}

		}
		else
		{
			/* 商店聯卡號遮掩 */
			/* 磁條卡列印交易編號 */
			memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
			inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
			if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
			    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
			    strlen(pobTran->srBRec.szTxnNo) > 0)
			{
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "11::%s\x0D\x0A", "交易編號(Transaction No.):");
				memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "11::%s\x0D\x0A", pobTran->srBRec.szTxnNo);
				memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);
			}
		}
	}
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        strcpy(szPrintBuf, "L::1\x0D\x0A");
        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	return (inDataLen);
}

/*
Function        :inNCCC_ESC_PRINT_AMOUNT_041_043
Date&Time       :
Describe        :
*/
int inNCCC_ESC_PRINT_AMOUNT_041_043(TRANSACTION_OBJECT *pobTran, char *szPackData)
{
        int     inDataLen = 0;
	char	szPrintBuf[200 + 1], szTemplate[100 + 1];
	
	if (pobTran->srBRec.uszVOIDBit == VS_TRUE)
	{
		if (pobTran->srBRec.inOrgCode == _REFUND_	|| 
		    pobTran->srBRec.inOrgCode == _INST_REFUND_	|| 
		    pobTran->srBRec.inOrgCode == _REDEEM_REFUND_)
		{
		        /* 將NT$ ＋數字塞到szTemplate中來inpad */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
			inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, _PADDING_RIGHT_);

		        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        sprintf(szPrintBuf, "22::總計(Total)%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

                        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
                        inDataLen += strlen(szPrintBuf);
		}
		else
		{
			/* 將NT$ ＋數字塞到szTemplate中來inpad */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
			inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, _PADDING_RIGHT_);

		        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        sprintf(szPrintBuf, "22::總計(Total)%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

                        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
                        inDataLen += strlen(szPrintBuf);
		}
	}
	else
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTransFunc(szTemplate);
		if (szTemplate[6] == 'Y')	/* 檢查是否有開小費 */
		{
			/* 退貨金額為負數 */
			if (pobTran->srBRec.inCode == _REFUND_ || pobTran->srBRec.inCode == _INST_REFUND_ || pobTran->srBRec.inCode == _REDEEM_REFUND_)
			{
				 /* 初始化 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
				inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, _PADDING_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "22::總計(Total)%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

                                memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
                                inDataLen += strlen(szPrintBuf);
			}
			/* 預授不會有小費，所以拉出來 */
			else if (pobTran->srBRec.inCode == _PRE_AUTH_ || pobTran->srBRec.inCode == _PRE_COMP_)
			{
				/* 初始化 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
				inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, _PADDING_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "22::總計(Total)%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

                                memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
                                inDataLen += strlen(szPrintBuf);
			}
			else
			{
				/* 金額 */
				/* 初始化 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
				inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, _PADDING_RIGHT_);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "22::金額(Amount)%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

                                memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
                                inDataLen += strlen(szPrintBuf);

				/* lnTipAmount為0表示非小費 */
				if (pobTran->srBRec.lnTipTxnAmount == 0L)
				{
				        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				        sprintf(szPrintBuf, "22::小費(Tips)%s%s\x0D\x0A", _DATA_FLAG_, "_____________");

                                        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
                                        inDataLen += strlen(szPrintBuf);

                                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                        sprintf(szPrintBuf, "22::總計(Total)%s%s\x0D\x0A", _DATA_FLAG_, "_____________");

                                        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
                                        inDataLen += strlen(szPrintBuf);
				}
				else
				{
					/* 小費 */
					/* 初始化 */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

					/* 將NT$ ＋數字塞到szTemplate中來inpad */
					sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTipTxnAmount);
					inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, _PADDING_RIGHT_);

					/* 把前面的字串和數字結合起來 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "22::小費(Tips)%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

                                        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
                                        inDataLen += strlen(szPrintBuf);

					/* 總計 */
					/* 初始化 */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

					/* 將NT$ ＋數字塞到szTemplate中來inpad */
					sprintf(szTemplate, "%ld",  (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
					inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 15, _PADDING_RIGHT_);

					/* 把前面的字串和數字結合起來 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                        sprintf(szPrintBuf, "22::總計(Total)%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

                                        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
                                        inDataLen += strlen(szPrintBuf);
				}
			}
		}
		else
		{
			/* 初始化 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

			/* 將NT$ ＋數字塞到szTemplate中來inpad，退貨要負數 */
			if (pobTran->srBRec.inCode == _REFUND_ || pobTran->srBRec.inCode == _INST_REFUND_ || pobTran->srBRec.inCode == _REDEEM_REFUND_)
				sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
			else
				sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
			
			inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, VS_TRUE);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        sprintf(szPrintBuf, "22::總計(Total)%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

                        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
                        inDataLen += strlen(szPrintBuf);
		}
		
	}
		
	/* 斷行 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        strcpy(szPrintBuf, "L::1\x0D\x0A");
        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	return (inDataLen);
}

/*
Function        :inNCCC_ESC_PRINT_Receiptend_041_043
Date&Time       :2022/9/29 下午 4:14
Describe        :組E1 結尾
*/
int inNCCC_ESC_PRINT_ReceiptEND_041_043(TRANSACTION_OBJECT *pobTran, char *szPackData)
{
	int	inDataLen = 0;
	char	szPrintBuf[84 + 1], szTemplate[42 + 1] = {0};
	char	szPrtNotice[1 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
        
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	/* 簽名欄 */
	/* 免簽名 */
	/* 小費一律簽名 */
	if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE && pobTran->srBRec.inCode != _TIP_)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "12::%s\x0D\x0A", "免　　簽　　名");

		memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
	}
	/* (需求單 - 107227)邦柏科技自助作業客製化 by Russell 2018/12/7 下午 2:36 */
	/* (需求單 - 107276)自助交易標準400做法 by Russell 2018/12/27 上午 11:20 */
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)         ||
                 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_))
	{
		/* 一律送免簽名 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "12::%s\x0D\x0A", "免　　簽　　名");

		memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
	}
	/* 要簽名 */
	else
	{
		/* 有在signpad簽名 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
		inFunc_ComposeFileName_InvoiceNumber(pobTran, szTemplate, _PICTURE_FILE_EXTENSION_, 6);

		if (inFILE_Check_Exist((unsigned char *)szTemplate) == VS_SUCCESS)
		{
			/* 電子簽名 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "G::%s\x0D\x0A", _ESC_BMP_SIGNATURE_);
			memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
		}

	}

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "14::%s\x0D\x0A", "X:__________________________________");
	memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
	inDataLen += strlen(szPrintBuf);

	/* 持卡人姓名 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "11::%s\x0D\x0A", pobTran->srBRec.szCardHolder);
	memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
	inDataLen += strlen(szPrintBuf);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "12::%s\x0D\x0A", "持卡人簽名(特店存根聯)");
	memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
	inDataLen += strlen(szPrintBuf);
		
	/* 列印警示語 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "12::%s\x0D\x0A", "I AGREE TO PAY TOTAL AMOUNT");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "12::%s\x0D\x0A", "ACCORDING TO CARD ISSUER AGREEMENT");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* Print Notice */
	memset(szPrtNotice, 0x00, sizeof(szPrtNotice));
	inGetPrtNotice(szPrtNotice);
	if(memcmp(szPrtNotice, "Y", strlen("Y")) == 0)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "G::%s\x0D\x0A", _ESC_BMP_NOTICE_LOGO_);
                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
	}
	
        return (inDataLen);
}

/*
Function        :inNCCC_ESC_PRINT_DATA_005
Date&Time       :2022/10/5 下午 8:58
Describe        :多了發票號碼
*/
int inNCCC_ESC_PRINT_DATA_005(TRANSACTION_OBJECT *pobTran, char *szPackData)
{
	int	i = 0;
	int	inDataLen = 0;
	int	inTempLen = 0;
	int	inUpperAmount = 0;
	int	inLowerAmount = 0;
	char	szPrintBuf[200 + 1] = {0}, szPrintBuf1[100 + 1] = {0}, szPrintBuf2[100 + 1] = {0}, szTemplate[100 + 1] = {0}, szTemplate1[100 + 1] = {0}, szTemplate2[100 + 1] = {0};
	char	szProductCodeEnable[1 + 1];
	char	szStore_Stub_CardNo_Truncate_Enable[2 + 1];
	char	szExamBit[10 + 1] = {0};

	/* 城市別 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", "城市別(City)");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 城市名 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetCityName(szTemplate);
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", szTemplate);

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 卡別 & 檢查碼 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "29::%s%s%s\x0D\x0A", "卡別(Card Type)", _DATA_FLAG_, "檢查碼(Check No.)");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 卡別值 & 檢查碼值 */
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	/* 【需求單 - 106349】自有品牌判斷需求 */
	/* unpack電文就存存CardLabel modify by LingHsiung 2020/2/14 上午 11:07 */
	strcpy(szTemplate1, pobTran->srBRec.szCardLabel);

	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/28 下午 2:42 */
	if (strlen(pobTran->srBRec.szCheckNO) > 0)
	{
		strcpy(szTemplate2, pobTran->srBRec.szCheckNO);
	}
	else
	{
		inCARD_ExpDateEncryptAndDecrypt(pobTran, "", szTemplate2, _EXP_ENCRYPT_);
	}

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "29::%s%s%s\x0D\x0A", szTemplate1, _DATA_FLAG_, szTemplate2);

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 卡號掩飾 NCCC 規格以為準 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", "卡號(Card No.)");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	memset(szTemplate, 0x00, sizeof(szTemplate));
	strcpy(szTemplate, pobTran->srBRec.szPAN);
	
	/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
	/* 商店聯卡號遮掩 */
	memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
	inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
	/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/8 下午 1:55 */
	if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
	{
		/* Uny交易不受TMS開關限制，且獲取卡號時已遮掩 */
	}
	else if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0 && pobTran->srBRec.uszTxNoCheckBit == VS_TRUE)
	{
		/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
		for (i = 6; i < (strlen(szTemplate) - 4); i ++)
			szTemplate[i] = 0x2A;
	}

	/* 過卡方式 */
	if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
	{
		if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
			strcat(szTemplate, "(W)");
		else
			strcat(szTemplate, "(C)");
	}
	else if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
	{
		strcat(szTemplate, "(Q)");
	}
	else if (pobTran->srBRec.uszCUPEMVQRCodeBit == VS_TRUE)
	{
		strcat(szTemplate, "(Q)");
	}
	else
	{
		if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
			strcat(szTemplate,"(C)");
		else if (pobTran->srBRec.uszMobilePayBit == VS_TRUE)
			strcat(szTemplate, "(T)");
		else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
			strcat(szTemplate, "(W)");
		else
		{
			if (pobTran->srBRec.uszManualBit == VS_TRUE)
			{
				/* 【需求單 - 105244】端末設備支援以感應方式進行退貨交易 */
				/* 電文轉Manual Keyin但是簽單要印感應的W */
				if (pobTran->srBRec.uszRefundCTLSBit == VS_TRUE)
					strcat(szTemplate, "(W)");
				else
					strcat(szTemplate,"(M)");
			}
			else
				strcat(szTemplate,"(S)");
		}
	}

       	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", szTemplate);

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 主機別 & 交易類別 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", "主機別/交易類別(Host/Trans. Type)");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 主機別值 & 交易類別值*/
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	inGetHostLabel(szTemplate);
	inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);

	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	sprintf(szPrintBuf1, "%s %s %s", szTemplate, szTemplate1, szTemplate2);
	
	if (strlen(szTemplate1) > 0)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "11::%s\x0D\x0A", szPrintBuf1);

		memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
	}
		
	
	/* 批次號碼(Batch No.) & 授權碼 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "29::%s%s%s\x0D\x0A", "批次號碼(Batch No.)", _DATA_FLAG_, "授權碼(Auth Code)");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%03ld", pobTran->srBRec.lnBatchNum);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "29::%s%s%s\x0D\x0A", szTemplate, _DATA_FLAG_, pobTran->srBRec.szAuthCode);

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	/* 回覆碼 */
	if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
	{
	        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "11::%s\x0D\x0A", "回覆碼(Resp. Code)");

                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);

		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(&szTemplate[0], &pobTran->srBRec.szRespCode[0], 2);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "11::%s\x0D\x0A", szTemplate);

                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
	}

	/* 日期 & 時間 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", "日期/時間(Date/Time)");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 日期值 時間值 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szTemplate, "%.4s/%.2s/%.2s %.2s:%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6], &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
		
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "14::%s\x0D\x0A", szTemplate);

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 序號 & 調閱編號 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "29::%s%s%s\x0D\x0A", "序號(Ref. No.)", _DATA_FLAG_, "調閱編號(Inv. No.)");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 序號值 & 調閱編號值 */
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	inFunc_PAD_ASCII(szTemplate1, pobTran->srBRec.szRefNo, ' ', 25, _PADDING_RIGHT_);

	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	sprintf(szTemplate2, "%06ld", pobTran->srBRec.lnOrgInvNum);

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "29::%s%s%s\x0D\x0A", szTemplate1, _DATA_FLAG_, szTemplate2);

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 櫃號 */
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	inGetStoreIDEnable(szTemplate1);
	if ((memcmp(&szTemplate1[0], "Y", 1) == 0) && (strlen(pobTran->srBRec.szStoreID) > 0))
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "11::%s\x0D\x0A", "櫃號(Store ID)");

		memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
		
		/* 櫃號值 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		sprintf(szTemplate1, "%s", pobTran->srBRec.szStoreID);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "11::%s\x0D\x0A", szTemplate1);

		memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
	}
	
	if (strlen(pobTran->srBRec.szFPG_FTC_Invoice) > 0)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "11::%s\x0D\x0A", "發票號碼");

		memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "11::%s\x0D\x0A", pobTran->srBRec.szFPG_FTC_Invoice);

		memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
	}
	
	/* 產品代碼 */
	inGetProductCodeEnable(szProductCodeEnable);
	if (memcmp(szProductCodeEnable, "Y", 1) == 0)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "11::%s\x0D\x0A", "產品代碼(Product Code)");

                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "11::%s\x0D\x0A", pobTran->srBRec.szProductCode);

                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
	}
	
	/* TC */
	/* 驗測開關 */
	memset(szExamBit, 0x00, sizeof(szExamBit));
	inGetExamBit(szExamBit);
	/* Uny交易 */
	/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/8 下午 1:55 */
	if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
	{
		/* 取消或退貨，只印數字交易碼 */
		/* 最小字體 */
		if (pobTran->srBRec.uszVOIDBit == VS_TRUE ||
		    pobTran->srBRec.inCode == _REFUND_ || pobTran->srBRec.inCode == _INST_REFUND_ || pobTran->srBRec.inCode == _REDEEM_REFUND_)
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			sprintf(szTemplate1, "交易碼 %s", pobTran->srBRec.szUnyTransCode);
			sprintf(szPrintBuf, "11::%s\x0D\x0A", szTemplate1);

			memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
		}
		/* 正向交易，要印條碼和數字 */
		else
		{

			if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
			{
				/* 一維條碼 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%s", pobTran->srBRec.szUnyTransCode);
				sprintf(szPrintBuf, "U::%s\x0D\x0A", szTemplate1);
				
				memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);

				/* 數字 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "交易碼(退貨請掃描) %s", pobTran->srBRec.szUnyTransCode);
				sprintf(szPrintBuf, "11::%s\x0D\x0A", szTemplate1);

				memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);
			}
			else
			{
				/* 一維條碼 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%s", pobTran->srBRec.szUnyTransCode);
				sprintf(szPrintBuf, "U::%s\x0D\x0A", szTemplate1);

				memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);

				/* 數字 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "交易碼 %s", pobTran->srBRec.szUnyTransCode);
				sprintf(szPrintBuf, "11::%s\x0D\x0A", szTemplate1);

				memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);
			}
		}
	}
	/* 實體卡交易和掃碼交易互斥 */
	else
	{
		if (pobTran->srBRec.inChipStatus == _EMV_CARD_ || pobTran->srBRec.uszContactlessBit == VS_TRUE)
		{
			/* 感應磁條 */
			if (!memcmp(pobTran->srBRec.szAuthCode, "VLP", 3)				|| 
			    !memcmp(pobTran->srBRec.szAuthCode, "JCB", 3)				||
			     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE		|| 
			     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_64_NEWJSPEEDY_MSD		||
			     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE)
			{
				/* 商店聯卡號遮掩 */
				memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
				inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
				if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
				    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
				    strlen(pobTran->srBRec.szTxnNo) > 0)
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "11::%s\x0D\x0A", "交易編號(Transaction No.):");
					memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
					inDataLen += strlen(szPrintBuf);

					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "11::%s\x0D\x0A", pobTran->srBRec.szTxnNo);
					memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
					inDataLen += strlen(szPrintBuf);
				}
			}
			else
			{
				if (pobTran->srBRec.inPrintOption == _PRT_MERCH_)
				{
					if (pobTran->srEMVRec.in9F26_ApplCryptogramLen > 0)
					{
						memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
						sprintf(szPrintBuf1, "%02X%02X%02X%02X%02X%02X%02X%02X",
										pobTran->srEMVRec.usz9F26_ApplCryptogram[0],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[1],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[2],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[3],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[4],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[5],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[6],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[7]);

						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "23::TC%s%s\x0D\x0A", _DATA_FLAG_, szPrintBuf1);

						memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
						inDataLen += strlen(szPrintBuf);
					}

					/* 商店聯卡號遮掩 */
					memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
					inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
					if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
					    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
					    strlen(pobTran->srBRec.szTxnNo) > 0)
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "11::%s\x0D\x0A", "交易編號(Transaction No.):");
						memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
						inDataLen += strlen(szPrintBuf);

						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "11::%s\x0D\x0A", pobTran->srBRec.szTxnNo);
						memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
						inDataLen += strlen(szPrintBuf);
					}

					/* M/C交易列印AP Lable (START) */
					if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)))
					{
						if (pobTran->srEMVRec.in50_APLabelLen > 0)
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							strcat(szPrintBuf, "AP Label:");
							memset(szTemplate, 0x00, sizeof(szTemplate));
							strcpy(szTemplate, (char*)pobTran->srEMVRec.usz50_APLabel);
							inUpperAmount = inFunc_Get_UpperCase_Char(szTemplate);
							inLowerAmount = strlen(szTemplate) - inUpperAmount;
							inTempLen =  _ESC_AP_LABEL_TOTAL_SPACE_ - (_ESC_UPPER_MULTIPLE_ * inUpperAmount + _ESC_LOWER_MULTIPLE_ * inLowerAmount);
							for (i = 0; i < inTempLen; i++)
							{
								strcat(szPrintBuf, " ");
							}
							strcat(szPrintBuf, szTemplate);
						}
						else
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							strcat(szPrintBuf, "AP Label:");
							strcat(szPrintBuf, "\x20");
						}
					}
					/* M/C交易列印AP Lable (END) */
					else if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)))
					{
						if (pobTran->srEMVRec.in50_APLabelLen > 0)
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							strcat(szPrintBuf, "AP Label:");
							memset(szTemplate, 0x00, sizeof(szTemplate));
							strcpy(szTemplate, (char*)pobTran->srEMVRec.usz50_APLabel);
							inUpperAmount = inFunc_Get_UpperCase_Char(szTemplate);
							inLowerAmount = strlen(szTemplate) - inUpperAmount;
							inTempLen =  _ESC_AP_LABEL_TOTAL_SPACE_ - (_ESC_UPPER_MULTIPLE_ * inUpperAmount + _ESC_LOWER_MULTIPLE_ * inLowerAmount);
							for (i = 0; i < inTempLen; i++)
							{
								strcat(szPrintBuf, " ");
							}
							strcat(szPrintBuf, szTemplate);
						}
						else
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							strcat(szPrintBuf, "AP Label:");
							strcat(szPrintBuf, "\x20");
						}
					}
					/* 驗測要印AP Label 和 AID */
					else if (szExamBit[0] == '1')
					{
						if (pobTran->srEMVRec.in50_APLabelLen > 0)
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							strcat(szPrintBuf, "AP Label:");
							memset(szTemplate, 0x00, sizeof(szTemplate));
							strcpy(szTemplate, (char*)pobTran->srEMVRec.usz50_APLabel);
							inUpperAmount = inFunc_Get_UpperCase_Char(szTemplate);
							inLowerAmount = strlen(szTemplate) - inUpperAmount;
							inTempLen =  _ESC_AP_LABEL_TOTAL_SPACE_ - (_ESC_UPPER_MULTIPLE_ * inUpperAmount + _ESC_LOWER_MULTIPLE_ * inLowerAmount);
							for (i = 0; i < inTempLen; i++)
							{
								strcat(szPrintBuf, " ");
							}
							strcat(szPrintBuf, szTemplate);
						}
						else
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							strcat(szPrintBuf, "AP Label:");
							strcat(szPrintBuf, "\x20");
						}
					}
					else
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						strcat(szPrintBuf, "\x20");
						strcat(szPrintBuf, "\x20");
					}
					
					memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
					sprintf(szPrintBuf2, szPrintBuf);
					
					/* AID */
					if (strlen(pobTran->srBRec.szCUP_EMVAID) > 0)
					{
						memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
						sprintf(szPrintBuf1, "AID:%s", pobTran->srBRec.szCUP_EMVAID);
						inFunc_PAD_ASCII(szPrintBuf1, szPrintBuf1, ' ', _ESC_AID_TOTAL_LEN_, _PADDING_RIGHT_);
						
						if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
						{
							/* CUP晶片要印 */
							if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
							else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)) == 0)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
							/* 【需求單-109121】Mastercard交易簽單列印AID需求 by Russell 2020/7/23 下午 4:23 */
							else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)) == 0)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
							/* 【需求單-111290】VISA置換Logo、DISCOVER簽單新增AID欄位、AE簽單新增AID和AP Label欄位需求 by Russell 2022/12/29 下午 5:57 */
							else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)) == 0 &&
								pobTran->srBRec.uszUCARDTransBit != VS_TRUE)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
							/* 驗測要印AP Label 和 AID */
							else if (szExamBit[0] == '1')
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
						}
						/* 銀聯閃付 */
						else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
						{
							if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
							/* AE晶片要印出AID */
							else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_50_EXPRESSSPAY || 
								 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
							else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE || 
							         pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_21_PAYPASS_MCHIP)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
							else if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_42_DPAS	|| pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_43_DISCOVER_DPAS_MAG_STRIPE) &&
								pobTran->srBRec.uszUCARDTransBit != VS_TRUE)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
							/* 驗測要印AP Label 和 AID */
							else if (szExamBit[0] == '1')
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
						}
					}/* AID END */
				}
			}
		}
		else
		{
			/* 商店聯卡號遮掩 */
			/* 磁條卡列印交易編號 */
			memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
			inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
			if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
			    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
			    strlen(pobTran->srBRec.szTxnNo) > 0)
			{
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "11::%s\x0D\x0A", "交易編號(Transaction No.):");
				memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "11::%s\x0D\x0A", pobTran->srBRec.szTxnNo);
				memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);
			}
		}
	}
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        strcpy(szPrintBuf, "L::1\x0D\x0A");
        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	return (inDataLen);
}

/*
Function        :inNCCC_ESC_PRINT_TIDMID_SMALL
Date&Time       :2023/3/7 下午 4:36
Describe        :組E1 TID、MID部份
*/
int inNCCC_ESC_PRINT_TIDMID_Small(TRANSACTION_OBJECT* pobTran, char* szPackData)
{
        int     inDataLen = 0;
	char    szTempBuff[100 + 1];	/* 暫放get出來的值 */
	char	szPrintBuf[200 + 1];

	/* 商店代號 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTempBuff, 0x00, sizeof(szTempBuff));
	inGetMerchantID(szTempBuff);
	sprintf(szPrintBuf, "20::商店代號%s%s\x0D\x0A", _DATA_FLAG_, szTempBuff);

	memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 端末機代號 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTempBuff, 0x00, sizeof(szTempBuff));
	inGetTerminalID(szTempBuff);
	sprintf(szPrintBuf, "20::端末機代號%s%s\x0D\x0A", _DATA_FLAG_, szTempBuff);

	memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	return (inDataLen);
}

/*
Function        :inNCCC_ESC_PRINT_DATA_Small
Date&Time       :2023/3/8 上午 11:19
Describe        :組E1 DATA部份
*/
int inNCCC_ESC_PRINT_DATA_Small(TRANSACTION_OBJECT *pobTran, char *szPackData)
{
	int	i = 0;
	int	inDataLen = 0;
	int	inTempLen = 0;
	int	inUpperAmount = 0;
	int	inLowerAmount = 0;
	char	szPrintBuf[200 + 1] = {0}, szPrintBuf1[100 + 1] = {0}, szPrintBuf2[100 + 1] = {0}, szTemplate[100 + 1] = {0}, szTemplate1[100 + 1] = {0}, szTemplate2[100 + 1] = {0};
	char	szProductCodeEnable[1 + 1];
	char	szStore_Stub_CardNo_Truncate_Enable[2 + 1];
	char	szExamBit[10 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);

	/* "卡號 卡別" 卡別值 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	strcpy(szTemplate, pobTran->srBRec.szCardLabel);
	
	inTempLen =  ((20 - strlen(szTemplate)) * _ESC_SMALL_CARD_TYPE_MULTIPLE_) + _ESC_SMALL_CARD_TYPE_OFFSET_;
	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	for (i = 0; i < inTempLen; i++)
	{
		strcat(szTemplate2, " ");
	}
	strcat(szTemplate2, szTemplate);
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	strcat(szPrintBuf, "29::");
	strcat(szPrintBuf, "卡號");
	strcat(szPrintBuf, "\x20");
	strcat(szPrintBuf, _DATA_FLAG_);
	strcat(szPrintBuf, "卡別");
	strcat(szPrintBuf, "\x20");
	strcat(szPrintBuf, szTemplate2);
	strcat(szPrintBuf, "\x0D\x0A");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	/* 卡號值 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	strcpy(szTemplate, pobTran->srBRec.szPAN);
	
	/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
	/* 商店聯卡號遮掩 */
	memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
	inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
	/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/8 下午 1:55 */
	if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
	{
		/* Uny交易不受TMS開關限制，且獲取卡號時已遮掩 */
	}
	else if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0 && pobTran->srBRec.uszTxNoCheckBit == VS_TRUE)
	{
		/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
		for (i = 6; i < (strlen(szTemplate) - 4); i ++)
			szTemplate[i] = 0x2A;
	}

	/* 過卡方式 */
	if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
	{
		if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
			strcat(szTemplate, "(W)");
		else
			strcat(szTemplate, "(C)");
	}
	else if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
	{
		strcat(szTemplate, "(Q)");
	}
	else if (pobTran->srBRec.uszCUPEMVQRCodeBit == VS_TRUE)
	{
		strcat(szTemplate, "(Q)");
	}
	else
	{
		if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
			strcat(szTemplate,"(C)");
		else if (pobTran->srBRec.uszMobilePayBit == VS_TRUE)
			strcat(szTemplate, "(T)");
		else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
			strcat(szTemplate, "(W)");
		else
		{
			if (pobTran->srBRec.uszManualBit == VS_TRUE)
			{
				/* 【需求單 - 105244】端末設備支援以感應方式進行退貨交易 */
				/* 電文轉Manual Keyin但是簽單要印感應的W */
				if (pobTran->srBRec.uszRefundCTLSBit == VS_TRUE)
					strcat(szTemplate, "(W)");
				else
					strcat(szTemplate,"(M)");
			}
			else
				strcat(szTemplate,"(S)");
		}
	}
       	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", szTemplate);

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	/* 交易類別 */
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s %s\x0D\x0A", "交易", szTemplate1);

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	if (strlen(szTemplate2) > 0)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "11::%s\x0D\x0A", szTemplate2);

		memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
	}
	
	/* 城市 主機 */
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	inGetCityName(szTemplate1);
	
	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	inGetHostLabel(szTemplate2);
	inFunc_DiscardSpace(szTemplate2);
	inFunc_PAD_ASCII(szTemplate2, szTemplate2, ' ', 8, _PADDING_LEFT_);
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	strcat(szPrintBuf, "40::");
	strcat(szPrintBuf, "城市");
	strcat(szPrintBuf, _DATA_FLAG_);
	strcat(szPrintBuf, szTemplate1);
	strcat(szPrintBuf, _DATA_FLAG_);
	strcat(szPrintBuf, "主機");
	strcat(szPrintBuf, _DATA_FLAG_);
	strcat(szPrintBuf, szTemplate2);
	strcat(szPrintBuf, "\x0D\x0A");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	/* "日期/時間" 日期值 "批號" 批號值 */
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	sprintf(szTemplate1, "%.4s/%.2s/%.2s %.2s:%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6], &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
	
	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	sprintf(szTemplate2, "%03ld", pobTran->srBRec.lnBatchNum);
	inFunc_PAD_ASCII(szTemplate2, szTemplate2, ' ', 8, _PADDING_LEFT_);
		
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	strcat(szPrintBuf, "40::");
	strcat(szPrintBuf, "日期/時間");
	strcat(szPrintBuf, _DATA_FLAG_);
	strcat(szPrintBuf, szTemplate1);
	strcat(szPrintBuf, _DATA_FLAG_);
	strcat(szPrintBuf, "批號");
	strcat(szPrintBuf, _DATA_FLAG_);
	strcat(szPrintBuf, szTemplate2);
	strcat(szPrintBuf, "\x0D\x0A");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* "授權碼" 授權碼值 "檢查碼" 檢查碼值 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	memcpy(szTemplate1, &pobTran->srBRec.szAuthCode[0], 12);
	
	if (pobTran->srBRec.uszTakaTransBit == VS_TRUE)
	{
		memset(szTemplate2, 0x00, sizeof(szTemplate2));
		sprintf(szTemplate2, "%.2s/'%.2s", pobTran->srBRec.szExpDate + 2, pobTran->srBRec.szExpDate);
	}
	else
	{
		memset(szTemplate2, 0x00, sizeof(szTemplate2));
		/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/28 下午 2:42 */
		if (strlen(pobTran->srBRec.szCheckNO) > 0)
		{
			strcpy(szTemplate2, pobTran->srBRec.szCheckNO);
		}
		else
		{
			inCARD_ExpDateEncryptAndDecrypt(pobTran, "", szTemplate2, _EXP_ENCRYPT_);
		}
		inFunc_PAD_ASCII(szTemplate2, szTemplate2, ' ', 9, _PADDING_LEFT_);
	}
	
	/* 大高卡只能顯示有效期 */
	if (pobTran->srBRec.uszTakaTransBit == VS_TRUE)
	{
		strcat(szPrintBuf, "40::");
		strcat(szPrintBuf, "授權碼");
		strcat(szPrintBuf, _DATA_FLAG_);
		strcat(szPrintBuf, szTemplate1);
		strcat(szPrintBuf, _DATA_FLAG_);
		strcat(szPrintBuf, "有效期");
		strcat(szPrintBuf, _DATA_FLAG_);
		strcat(szPrintBuf, szTemplate2);
		strcat(szPrintBuf, "\x0D\x0A");
	}
	else
	{
		strcat(szPrintBuf, "40::");
		strcat(szPrintBuf, "授權碼");
		strcat(szPrintBuf, _DATA_FLAG_);
		strcat(szPrintBuf, szTemplate1);
		strcat(szPrintBuf, _DATA_FLAG_);
		strcat(szPrintBuf, "檢查碼");
		strcat(szPrintBuf, _DATA_FLAG_);
		strcat(szPrintBuf, szTemplate2);
		strcat(szPrintBuf, "\x0D\x0A");
	}

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* "序號" 序號值 "調閱號" 調閱編號值 */
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	inFunc_PAD_ASCII(szTemplate1, pobTran->srBRec.szRefNo, ' ', 25, _PADDING_RIGHT_);

	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	sprintf(szTemplate2, "%06ld", pobTran->srBRec.lnOrgInvNum);
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	strcat(szPrintBuf, "40::");
	strcat(szPrintBuf, "序號");
	strcat(szPrintBuf, _DATA_FLAG_);
	strcat(szPrintBuf, szTemplate1);
	strcat(szPrintBuf, _DATA_FLAG_);
	strcat(szPrintBuf, "調閱號");
	strcat(szPrintBuf, _DATA_FLAG_);
	strcat(szPrintBuf, szTemplate2);
	strcat(szPrintBuf, "\x0D\x0A");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 櫃號 */
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	inGetStoreIDEnable(szTemplate1);
	if ((memcmp(&szTemplate1[0], "Y", 1) == 0) && (strlen(pobTran->srBRec.szStoreID) > 0))
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memcpy(szTemplate1, &pobTran->srBRec.szStoreID[0], 23);
		
		memset(szTemplate2, 0x00, sizeof(szTemplate2));
		sprintf(szTemplate2, "%s", pobTran->srBRec.szRespCode);
		inFunc_PAD_ASCII(szTemplate2, szTemplate2, ' ', 9, _PADDING_LEFT_);
		
		if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
		{
			strcat(szPrintBuf, "40::");
			strcat(szPrintBuf, "櫃號");
			strcat(szPrintBuf, _DATA_FLAG_);
			strcat(szPrintBuf, szTemplate1);
			strcat(szPrintBuf, _DATA_FLAG_);
			strcat(szPrintBuf, "回覆碼");
			strcat(szPrintBuf, _DATA_FLAG_);
			strcat(szPrintBuf, szTemplate2);
			strcat(szPrintBuf, "\x0D\x0A");
		}
		else
		{
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_026_TAKA_, _CUSTOMER_INDICATOR_SIZE_) ||
			    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
			{
				strcat(szPrintBuf, "40::");
				strcat(szPrintBuf, "品群碼");
				strcat(szPrintBuf, _DATA_FLAG_);
				strcat(szPrintBuf, szTemplate1);
				strcat(szPrintBuf, _DATA_FLAG_);
				strcat(szPrintBuf, "\x20");
				strcat(szPrintBuf, _DATA_FLAG_);
				strcat(szPrintBuf, "\x20");
				strcat(szPrintBuf, "\x0D\x0A");
			}
			else
			{
				strcat(szPrintBuf, "40::");
				strcat(szPrintBuf, "櫃號");
				strcat(szPrintBuf, _DATA_FLAG_);
				strcat(szPrintBuf, szTemplate1);
				strcat(szPrintBuf, _DATA_FLAG_);
				strcat(szPrintBuf, "\x20");
				strcat(szPrintBuf, _DATA_FLAG_);
				strcat(szPrintBuf, "\x20");
				strcat(szPrintBuf, "\x0D\x0A");
			}	
		}
		
		memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
		
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			/* hardcode */
			char	szPOS_ID[6 + 1] = {0};
			memset(szPOS_ID, 0x00, sizeof(szPOS_ID));
			inGetPOS_ID(szPOS_ID);

			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			memcpy(szTemplate1, _HARDCODE_STORE_ID_CUS_021_TAKAWEL_, 12);
			memcpy(&szTemplate1[12], szPOS_ID, 6);
			
			memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
		}
	}
	else
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memcpy(szTemplate1, &pobTran->srBRec.szStoreID[0], 23);
		
		memset(szTemplate2, 0x00, sizeof(szTemplate2));
		sprintf(szTemplate2, "%s", pobTran->srBRec.szRespCode);
		inFunc_PAD_ASCII(szTemplate2, szTemplate2, ' ', 9, _PADDING_LEFT_);
		
		if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
		{
			strcat(szPrintBuf, "40::");
			strcat(szPrintBuf, "\x20");
			strcat(szPrintBuf, _DATA_FLAG_);
			strcat(szPrintBuf, "\x20");
			strcat(szPrintBuf, _DATA_FLAG_);
			strcat(szPrintBuf, "回覆碼");
			strcat(szPrintBuf, _DATA_FLAG_);
			strcat(szPrintBuf, szTemplate2);
			strcat(szPrintBuf, "\x0D\x0A");
			
			memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
		}
	}
	
	/* 產品代碼 */
	inGetProductCodeEnable(szProductCodeEnable);
	if (memcmp(szProductCodeEnable, "Y", 1) == 0)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		strcat(szPrintBuf, "40::");
		strcat(szPrintBuf, "產品代碼");
		strcat(szPrintBuf, _DATA_FLAG_);
		strcat(szPrintBuf, pobTran->srBRec.szProductCode);
		strcat(szPrintBuf, _DATA_FLAG_);
		strcat(szPrintBuf, "\x20");
		strcat(szPrintBuf, _DATA_FLAG_);
		strcat(szPrintBuf, "\x20");
		strcat(szPrintBuf, "\x0D\x0A");

                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
	}
        
        /* 【需求單-113260】MasterCard機票交易需求 by Russell 2025/3/3 上午 11:55 */
        /* 取消維持原交易不列印機票資訊 */
        if (inNCCC_Func_Flight_Ticket_Allow_Print(pobTran) == VS_TRUE)
        {
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "27::%s%s%s\x0D\x0A", "出發地機場(departure)", _DATA_FLAG_, pobTran->srBRec.szFlightTicketPDS0523);

                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "27::%s%s%s\x0D\x0A", "目的地機場(arrival)", _DATA_FLAG_, pobTran->srBRec.szFlightTicketPDS0524);

                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);

                memset(szTemplate2, 0x00, sizeof(szTemplate2));
		if (strlen(pobTran->srBRec.szFlightTicketPDS0530) > 0)
                {
                        sprintf(szTemplate2, "%d", atoi(pobTran->srBRec.szFlightTicketPDS0530));
                }
                else
                {
                        sprintf(szTemplate2, "%s", " ");
                }
                sprintf(szPrintBuf, "%s", szTemplate2);
                
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "27::%s%s%s\x0D\x0A", "航班號碼(Flight No.)", _DATA_FLAG_, szTemplate2);

                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
                
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		inNCCC_Func_Get_Flight_Ticket_Txn_Category_PrintName(pobTran, szTemplate1);
                        
		if (!memcmp(szTemplate1, _FLIGHT_TICKET_TXN_CATEGORY_PRINT_BAGGAGE_CHARGE_, max(strlen(szTemplate1), strlen(_FLIGHT_TICKET_TXN_CATEGORY_PRINT_BAGGAGE_CHARGE_)))		||
		    !memcmp(szTemplate1, _FLIGHT_TICKET_TXN_CATEGORY_PRINT_PREPAID_TICKET_, max(strlen(szTemplate1), strlen(_FLIGHT_TICKET_TXN_CATEGORY_PRINT_PREPAID_TICKET_)))		||
		    !memcmp(szTemplate1, _FLIGHT_TICKET_TXN_CATEGORY_PRINT_MISCELLANEOUS_CHARGE_ , max(strlen(szTemplate1), strlen(_FLIGHT_TICKET_TXN_CATEGORY_PRINT_MISCELLANEOUS_CHARGE_)))	||
		    !memcmp(szTemplate1, _FLIGHT_TICKET_TXN_CATEGORY_PRINT_TICKET_REISSUE_, max(strlen(szTemplate1), strlen(_FLIGHT_TICKET_TXN_CATEGORY_PRINT_TICKET_REISSUE_))))
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "27::%s%s%s\x0D\x0A", "交易類型(transaction category)", _DATA_FLAG_, " ");

			memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "25::%s%s%s\x0D\x0A", " ", _DATA_FLAG_, szTemplate1);

			memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
		}
		else
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "27::%s%s%s\x0D\x0A", "交易類型(transaction category)", _DATA_FLAG_, szTemplate1);

			memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
		}
        }
	
	/* TC */
	/* 驗測開關 */
	memset(szExamBit, 0x00, sizeof(szExamBit));
	inGetExamBit(szExamBit);
	/* Uny交易 */
	/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/8 下午 1:55 */
	if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
	{
		/* 取消或退貨，只印數字交易碼 */
		/* 最小字體 */
		if (pobTran->srBRec.uszVOIDBit == VS_TRUE ||
		    pobTran->srBRec.inCode == _REFUND_ || pobTran->srBRec.inCode == _INST_REFUND_ || pobTran->srBRec.inCode == _REDEEM_REFUND_)
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			sprintf(szTemplate1, "交易碼 %s", pobTran->srBRec.szUnyTransCode);
			sprintf(szPrintBuf, "11::%s\x0D\x0A", szTemplate1);

			memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
		}
		/* 正向交易，要印條碼和數字 */
		else
		{

			if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
			{
				/* 一維條碼 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%s", pobTran->srBRec.szUnyTransCode);
				sprintf(szPrintBuf, "U::%s\x0D\x0A", szTemplate1);
				
				memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);

				/* 數字 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "交易碼(退貨請掃描) %s", pobTran->srBRec.szUnyTransCode);
				sprintf(szPrintBuf, "11::%s\x0D\x0A", szTemplate1);

				memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);
			}
			else
			{
				/* 一維條碼 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%s", pobTran->srBRec.szUnyTransCode);
				sprintf(szPrintBuf, "U::%s\x0D\x0A", szTemplate1);

				memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);

				/* 數字 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "交易碼 %s", pobTran->srBRec.szUnyTransCode);
				sprintf(szPrintBuf, "11::%s\x0D\x0A", szTemplate1);

				memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);
			}
		}
	}
	/* 實體卡交易和掃碼交易互斥 */
	else
	{
		if (pobTran->srBRec.inChipStatus == _EMV_CARD_ || pobTran->srBRec.uszContactlessBit == VS_TRUE)
		{
			/* 感應磁條 */
			if (!memcmp(pobTran->srBRec.szAuthCode, "VLP", 3)				|| 
			    !memcmp(pobTran->srBRec.szAuthCode, "JCB", 3)				||
			     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE		|| 
			     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_64_NEWJSPEEDY_MSD		||
			     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE)
			{
				/* 商店聯卡號遮掩 */
				memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
				inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
				if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
				    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
				    strlen(pobTran->srBRec.szTxnNo) > 0)
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "11::%s %s\x0D\x0A", "交易編號", pobTran->srBRec.szTxnNo);
					memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
					inDataLen += strlen(szPrintBuf);
				}
			}
			else
			{
				if (pobTran->srBRec.inPrintOption == _PRT_MERCH_)
				{
					if (pobTran->srEMVRec.in9F26_ApplCryptogramLen > 0)
					{
						memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
						sprintf(szPrintBuf1, "%02X%02X%02X%02X%02X%02X%02X%02X",
										pobTran->srEMVRec.usz9F26_ApplCryptogram[0],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[1],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[2],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[3],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[4],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[5],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[6],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[7]);

						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "23::TC%s%s\x0D\x0A", _DATA_FLAG_, szPrintBuf1);

						memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
						inDataLen += strlen(szPrintBuf);
					}

					/* 商店聯卡號遮掩 */
					memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
					inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
					if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
					    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
					    strlen(pobTran->srBRec.szTxnNo) > 0)
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "11::%s %s\x0D\x0A", "交易編號", pobTran->srBRec.szTxnNo);
						memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
						inDataLen += strlen(szPrintBuf);
					}

					/* M/C交易列印AP Lable (START) */
					if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)))
					{
						if (pobTran->srEMVRec.in50_APLabelLen > 0)
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							strcat(szPrintBuf, "AP Label:");
							memset(szTemplate, 0x00, sizeof(szTemplate));
							strcpy(szTemplate, (char*)pobTran->srEMVRec.usz50_APLabel);
							inUpperAmount = inFunc_Get_UpperCase_Char(szTemplate);
							inLowerAmount = strlen(szTemplate) - inUpperAmount;
							inTempLen =  _ESC_AP_LABEL_TOTAL_SPACE_ - (_ESC_UPPER_MULTIPLE_ * inUpperAmount + _ESC_LOWER_MULTIPLE_ * inLowerAmount);
							for (i = 0; i < inTempLen; i++)
							{
								strcat(szPrintBuf, " ");
							}
							strcat(szPrintBuf, szTemplate);
						}
						else
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							strcat(szPrintBuf, "AP Label:");
							strcat(szPrintBuf, "\x20");
						}
					}
					/* M/C交易列印AP Lable (END) */
					else if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)))
					{
						if (pobTran->srEMVRec.in50_APLabelLen > 0)
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							strcat(szPrintBuf, "AP Label:");
							memset(szTemplate, 0x00, sizeof(szTemplate));
							strcpy(szTemplate, (char*)pobTran->srEMVRec.usz50_APLabel);
							inUpperAmount = inFunc_Get_UpperCase_Char(szTemplate);
							inLowerAmount = strlen(szTemplate) - inUpperAmount;
							inTempLen =  _ESC_AP_LABEL_TOTAL_SPACE_ - (_ESC_UPPER_MULTIPLE_ * inUpperAmount + _ESC_LOWER_MULTIPLE_ * inLowerAmount);
							for (i = 0; i < inTempLen; i++)
							{
								strcat(szPrintBuf, " ");
							}
							strcat(szPrintBuf, szTemplate);
						}
						else
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							strcat(szPrintBuf, "AP Label:");
							strcat(szPrintBuf, "\x20");
						}
					}
					/* 驗測要印AP Label 和 AID */
					else if (szExamBit[0] == '1')
					{
						if (pobTran->srEMVRec.in50_APLabelLen > 0)
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							strcat(szPrintBuf, "AP Label:");
							memset(szTemplate, 0x00, sizeof(szTemplate));
							strcpy(szTemplate, (char*)pobTran->srEMVRec.usz50_APLabel);
							inUpperAmount = inFunc_Get_UpperCase_Char(szTemplate);
							inLowerAmount = strlen(szTemplate) - inUpperAmount;
							inTempLen =  _ESC_AP_LABEL_TOTAL_SPACE_ - (_ESC_UPPER_MULTIPLE_ * inUpperAmount + _ESC_LOWER_MULTIPLE_ * inLowerAmount);
							for (i = 0; i < inTempLen; i++)
							{
								strcat(szPrintBuf, " ");
							}
							strcat(szPrintBuf, szTemplate);
						}
						else
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							strcat(szPrintBuf, "AP Label:");
							strcat(szPrintBuf, "\x20");
						}
					}
					else
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						strcat(szPrintBuf, "\x20");
						strcat(szPrintBuf, "\x20");
					}
					memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
					sprintf(szPrintBuf2, szPrintBuf);
					
					/* AID */
					if (strlen(pobTran->srBRec.szCUP_EMVAID) > 0)
					{
						memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
						sprintf(szPrintBuf1, "AID:%s", pobTran->srBRec.szCUP_EMVAID);
						inFunc_PAD_ASCII(szPrintBuf1, szPrintBuf1, ' ', _ESC_AID_TOTAL_LEN_, _PADDING_RIGHT_);
						
						if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
						{
							/* CUP晶片要印 */
							if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
							else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)) == 0)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
							/* 【需求單-109121】Mastercard交易簽單列印AID需求 by Russell 2020/7/23 下午 4:23 */
							else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)) == 0)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
							/* 【需求單-111290】VISA置換Logo、DISCOVER簽單新增AID欄位、AE簽單新增AID和AP Label欄位需求 by Russell 2022/12/29 下午 5:57 */
							else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)) == 0 &&
								pobTran->srBRec.uszUCARDTransBit != VS_TRUE)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
							/* 驗測要印AP Label 和 AID */
							else if (szExamBit[0] == '1')
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
						}
						/* 銀聯閃付 */
						else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
						{
							if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
							/* AE晶片要印出AID */
							else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_50_EXPRESSSPAY || 
								 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
							/* 【需求單-109121】Mastercard交易簽單列印AID需求 by Russell 2020/7/23 下午 4:23 */
							else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE || 
								 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_21_PAYPASS_MCHIP)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
							else if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_42_DPAS	|| pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_43_DISCOVER_DPAS_MAG_STRIPE) &&
								pobTran->srBRec.uszUCARDTransBit != VS_TRUE)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
							/* 驗測要印AP Label 和 AID */
							else if (szExamBit[0] == '1')
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
						}
					}
				}
			}

		}
		else
		{
			/* 商店聯卡號遮掩 */
			/* 磁條卡列印交易編號 */
			memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
			inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
			if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
			    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
			    strlen(pobTran->srBRec.szTxnNo) > 0)
			{
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "11::%s %s\x0D\x0A", "交易編號", pobTran->srBRec.szTxnNo);
				memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);
			}
		}
	}
	
	return (inDataLen);
}

/*
Function        :inNCCC_ESC_PRINT_CUP_AMOUNT_Small
Date&Time       :2023/3/8 下午 5:21
Describe        :組E1 Amount部份
*/
int inNCCC_ESC_PRINT_CUP_AMOUNT_Small(TRANSACTION_OBJECT *pobTran, char *szPackData)
{
	int     inDataLen = 0;
	char	szTemplate[42 + 1];
	char	szPrintBuf[100];
        long    lnTempTxnAmt = 0;

	if (pobTran->srBRec.uszVOIDBit == VS_TRUE)
	{
                /* (【需求單-113200】CASV3標準版支援銀聯優計劃專案 by Russell 2024/8/27 上午 11:03 */
                if (pobTran->srBRec.uszUPlanTransBit == VS_TRUE)
                {
                        lnTempTxnAmt = pobTran->srBRec.lnCUPUPlanDiscountedAmount + pobTran->srBRec.lnCUPUPlanPreferentialAmount;

                }
                else
                {
                        lnTempTxnAmt = pobTran->srBRec.lnTxnAmount;
                }
                
		/* 橫式 */
                /* 金額 */
                /* 取消退貨是正數 */
                if (pobTran->srBRec.inOrgCode == _CUP_REFUND_	||
		    pobTran->srBRec.inOrgCode == _CUP_MAIL_ORDER_REFUND_)
                {
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
			sprintf(szTemplate, "%ld",  lnTempTxnAmt);
			inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
                }
		else
		{
			/* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
			sprintf(szTemplate, "%ld",  (0 - lnTempTxnAmt));
			inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
		}
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "22::總計(Total)%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
                
                /* (【需求單-113200】CASV3標準版支援銀聯優計劃專案 by Russell 2024/8/27 上午 11:03 */
		if (pobTran->srBRec.uszUPlanTransBit == VS_TRUE)
		{
	                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        strcpy(szPrintBuf, "L::1\x0D\x0A");
                        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                        inDataLen += strlen(szPrintBuf);

			memset(szTemplate, 0x00, sizeof(szTemplate));
			if (pobTran->srBRec.inOrgCode == _CUP_REFUND_		||
			    pobTran->srBRec.inOrgCode == _CUP_MAIL_ORDER_REFUND_)
			{
				sprintf(szTemplate, "%ld", pobTran->srBRec.lnCUPUPlanDiscountedAmount);
			}
			else
			{
				sprintf(szTemplate, "%ld", 0 - pobTran->srBRec.lnCUPUPlanDiscountedAmount);
			}
			inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	                sprintf(szPrintBuf, "22::優惠後金額%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);
	                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
	                inDataLen += strlen(szPrintBuf);

	                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        strcpy(szPrintBuf, "L::1\x0D\x0A");
                        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                        inDataLen += strlen(szPrintBuf);

			memset(szTemplate, 0x00, sizeof(szTemplate));
			if (pobTran->srBRec.inOrgCode == _CUP_REFUND_		||
			    pobTran->srBRec.inOrgCode == _CUP_MAIL_ORDER_REFUND_)
			{
				sprintf(szTemplate, "%ld", pobTran->srBRec.lnCUPUPlanPreferentialAmount);
			}
			else
			{
				sprintf(szTemplate, "%ld", 0 - pobTran->srBRec.lnCUPUPlanPreferentialAmount);
			}
			inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	                sprintf(szPrintBuf, "22::優惠金額%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);
	                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
	                inDataLen += strlen(szPrintBuf);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			strcpy(szPrintBuf, "13::============================\x0D\x0A");
		        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
		        inDataLen += strlen(szPrintBuf);

	                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        sprintf(szPrintBuf, "11::%s\x0D\x0A", "備註欄(Reference)");
                        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                        inDataLen += strlen(szPrintBuf);

                        if (strlen(pobTran->srBRec.szUPlan_RemarksInformation) > 0)
                        {
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                strcpy(szPrintBuf, "L::1\x0D\x0A");
                                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                                inDataLen += strlen(szPrintBuf);
                        }
                        else
                        {
		                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	                        sprintf(szPrintBuf, "11:: \x0D\x0A");
	                        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
	                        inDataLen += strlen(szPrintBuf);
                        }

	                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        sprintf(szPrintBuf, "11::%s\x0D\x0A", "優惠券號(Coupon ID)");
                        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                        inDataLen += strlen(szPrintBuf);

	                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        sprintf(szPrintBuf, "11::%s\x0D\x0A", pobTran->srBRec.szUPlan_Coupon);
                        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                        inDataLen += strlen(szPrintBuf);
        	}
	}
	else
	{
		/* 初始化 */
		memset(szTemplate, 0x00, sizeof(szTemplate));

		/* 將NT$ ＋數字塞到szTemplate中來inpad，退貨要負數 */
		if (pobTran->srBRec.inCode == _CUP_REFUND_||
		    pobTran->srBRec.inOrgCode == _CUP_MAIL_ORDER_REFUND_)
		{
			sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
		}
		else
		{
			sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
		}
		
		inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "22::總計(Total)%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
                
                 
                /* (【需求單-113200】CASV3標準版支援銀聯優計劃專案 by Russell 2024/8/27 上午 11:03 */
		if (pobTran->srBRec.uszUPlanTransBit == VS_TRUE)
		{
	                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        strcpy(szPrintBuf, "L::1\x0D\x0A");
                        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                        inDataLen += strlen(szPrintBuf);

			memset(szTemplate, 0x00, sizeof(szTemplate));
			if (pobTran->srBRec.inOrgCode == _CUP_REFUND_		||
			    pobTran->srBRec.inOrgCode == _CUP_MAIL_ORDER_REFUND_)
			{
				sprintf(szTemplate, "%ld", 0 - (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnCUPUPlanPreferentialAmount));
			}
			else
			{
				sprintf(szTemplate, "%ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnCUPUPlanPreferentialAmount));
			}
			inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	                sprintf(szPrintBuf, "22::優惠前金額%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);
	                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
	                inDataLen += strlen(szPrintBuf);

			memset(szTemplate, 0x00, sizeof(szTemplate));
			if (pobTran->srBRec.inOrgCode == _CUP_REFUND_		||
			    pobTran->srBRec.inOrgCode == _CUP_MAIL_ORDER_REFUND_)
			{
				sprintf(szTemplate, "%ld", 0 - pobTran->srBRec.lnCUPUPlanPreferentialAmount);
			}
			else
			{
				sprintf(szTemplate, "%ld", pobTran->srBRec.lnCUPUPlanPreferentialAmount);
			}
			inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	                sprintf(szPrintBuf, "22::優惠金額%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);
	                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
	                inDataLen += strlen(szPrintBuf);

			memset(szTemplate, 0x00, sizeof(szTemplate));
			if (pobTran->srBRec.inOrgCode == _CUP_REFUND_		||
			    pobTran->srBRec.inOrgCode == _CUP_MAIL_ORDER_REFUND_)
			{
				sprintf(szTemplate, "%ld", 0 - pobTran->srBRec.lnCUPUPlanDiscountedAmount);
			}
			else
			{
				sprintf(szTemplate, "%ld", pobTran->srBRec.lnCUPUPlanDiscountedAmount);
			}
			inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	                sprintf(szPrintBuf, "22::優惠後金額%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);
	                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
	                inDataLen += strlen(szPrintBuf);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			strcpy(szPrintBuf, "13::============================\x0D\x0A");
		        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
		        inDataLen += strlen(szPrintBuf);

	                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        sprintf(szPrintBuf, "11::%s\x0D\x0A", "備註欄(Reference)");
                        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                        inDataLen += strlen(szPrintBuf);

                        if (strlen(pobTran->srBRec.szUPlan_RemarksInformation) > 0)
                        {
		                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	                        sprintf(szPrintBuf, "11::%s\x0D\x0A", pobTran->srBRec.szUPlan_RemarksInformation);
	                        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
	                        inDataLen += strlen(szPrintBuf);
                        }
                        else
                        {
		                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	                        sprintf(szPrintBuf, "11:: \x0D\x0A");
	                        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
	                        inDataLen += strlen(szPrintBuf);
                        }

	                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        sprintf(szPrintBuf, "11::%s\x0D\x0A", "優惠券號(Coupon ID)");
                        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                        inDataLen += strlen(szPrintBuf);

	                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        sprintf(szPrintBuf, "11::%s\x0D\x0A", pobTran->srBRec.szUPlan_Coupon);
                        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                        inDataLen += strlen(szPrintBuf);
        	}
	}

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        strcpy(szPrintBuf, "L::1\x0D\x0A");
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 列印銀聯交易提示文字 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "G::%s\x0D\x0A", _ESC_BMP_CUP_LEGAL_LOGO_);
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        strcpy(szPrintBuf, "L::1\x0D\x0A");
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	return (inDataLen);
}

/*
Function        :inNCCC_ESC_PRINT_AMOUNT_Small
Date&Time       :2023/3/8 下午 5:27
Describe        :組E1 Amount部份
*/
int inNCCC_ESC_PRINT_AMOUNT_Small(TRANSACTION_OBJECT *pobTran, char *szPackData)
{
        int     inDataLen = 0;
	char	szPrintBuf[200 + 1], szTemplate[100 + 1];
	
	if (pobTran->srBRec.uszVOIDBit == VS_TRUE)
	{
		if (pobTran->srBRec.inOrgCode == _REFUND_	|| 
		    pobTran->srBRec.inOrgCode == _INST_REFUND_	|| 
		    pobTran->srBRec.inOrgCode == _REDEEM_REFUND_)
		{
		        /* 將NT$ ＋數字塞到szTemplate中來inpad */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
			inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, _PADDING_RIGHT_);

		        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        sprintf(szPrintBuf, "22::總計(Total)%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

                        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
                        inDataLen += strlen(szPrintBuf);
		}
		else
		{
			/* 將NT$ ＋數字塞到szTemplate中來inpad */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
			inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, _PADDING_RIGHT_);

		        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        sprintf(szPrintBuf, "22::總計(Total)%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

                        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
                        inDataLen += strlen(szPrintBuf);
		}
	}
	else
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTransFunc(szTemplate);
		if (szTemplate[6] == 'Y')	/* 檢查是否有開小費 */
		{
			/* 退貨金額為負數 */
			if (pobTran->srBRec.inCode == _REFUND_ || pobTran->srBRec.inCode == _INST_REFUND_ || pobTran->srBRec.inCode == _REDEEM_REFUND_)
			{
				 /* 初始化 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
				inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, _PADDING_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "22::總計(Total)%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

                                memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
                                inDataLen += strlen(szPrintBuf);
			}
			/* 預授不會有小費，所以拉出來 */
			else if (pobTran->srBRec.inCode == _PRE_AUTH_ || pobTran->srBRec.inCode == _PRE_COMP_)
			{
				/* 初始化 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
				inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, _PADDING_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "22::總計(Total)%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

                                memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
                                inDataLen += strlen(szPrintBuf);
			}
			else
			{
				/* 金額 */
				/* 初始化 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
				inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, _PADDING_RIGHT_);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "22::金額(Amount)%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

                                memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
                                inDataLen += strlen(szPrintBuf);

				/* lnTipAmount為0表示非小費 */
				if (pobTran->srBRec.lnTipTxnAmount == 0L)
				{
				        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				        sprintf(szPrintBuf, "22::小費(Tips)%s%s\x0D\x0A", _DATA_FLAG_, "_____________");

                                        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
                                        inDataLen += strlen(szPrintBuf);

                                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                        sprintf(szPrintBuf, "22::總計(Total)%s%s\x0D\x0A", _DATA_FLAG_, "_____________");

                                        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
                                        inDataLen += strlen(szPrintBuf);
				}
				else
				{
					/* 小費 */
					/* 初始化 */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

					/* 將NT$ ＋數字塞到szTemplate中來inpad */
					sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTipTxnAmount);
					inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, _PADDING_RIGHT_);

					/* 把前面的字串和數字結合起來 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "22::小費(Tips)%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

                                        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
                                        inDataLen += strlen(szPrintBuf);

					/* 總計 */
					/* 初始化 */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

					/* 將NT$ ＋數字塞到szTemplate中來inpad */
					sprintf(szTemplate, "%ld",  (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
					inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 15, _PADDING_RIGHT_);

					/* 把前面的字串和數字結合起來 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                        sprintf(szPrintBuf, "22::總計(Total)%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

                                        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
                                        inDataLen += strlen(szPrintBuf);
				}
			}
		}
		else
		{
			/* 初始化 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

			/* 將NT$ ＋數字塞到szTemplate中來inpad，退貨要負數 */
			if (pobTran->srBRec.inCode == _REFUND_ || pobTran->srBRec.inCode == _INST_REFUND_ || pobTran->srBRec.inCode == _REDEEM_REFUND_)
				sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
			else
				sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
			
			inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, VS_TRUE);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        sprintf(szPrintBuf, "22::總計(Total)%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

                        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
                        inDataLen += strlen(szPrintBuf);
		}
		
	}
		
	/* 斷行 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        strcpy(szPrintBuf, "L::1\x0D\x0A");
        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	return (inDataLen);
}

/*
Function        :inNCCC_ESC_PRINT_INST_Small
Date&Time       :2023/3/8 下午 5:34
Describe        :組E1 分期
*/
int inNCCC_ESC_PRINT_INST_Small(TRANSACTION_OBJECT *pobTran, char *szPackData)
{
	int	inDataLen = 0;
	char    szPrintBuf[84 + 1], szTemplate[42 + 1];

	if (pobTran->srBRec.uszInstallmentBit == VS_TRUE)
	{
		/* 分期期數 */
		/* 將NT$ ＋數字塞到szTemplate中來inpad */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%02ld 期", pobTran->srBRec.lnInstallmentPeriod);

		/* 把前面的字串和數字結合起來 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "22::分期期數%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

		memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);

		/* 首期金額 */
		/* 將NT$ ＋數字塞到szTemplate中來inpad */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		if (pobTran->srBRec.lnTipTxnAmount > 0L)
			sprintf(szTemplate, "%ld", (pobTran->srBRec.lnInstallmentDownPayment + pobTran->srBRec.lnTipTxnAmount));
		else
			sprintf(szTemplate, "%ld", (pobTran->srBRec.lnInstallmentDownPayment));
		inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, _PADDING_RIGHT_);

		/* 把前面的字串和數字結合起來 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "22::首期金額%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

		memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);

		/* 每期金額 */
		/* 將NT$ ＋數字塞到szTemplate中來inpad */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%ld", (pobTran->srBRec.lnInstallmentPayment));
		inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, _PADDING_RIGHT_);

		/* 把前面的字串和數字結合起來 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "22::每期金額%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

		memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);

		/* 分期手續費 */
		/* 將NT$ ＋數字塞到szTemplate中來inpad */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%ld", (pobTran->srBRec.lnInstallmentFormalityFee));
		inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, _PADDING_RIGHT_);

		/* 把前面的字串和數字結合起來 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "22::分期手續費%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

		memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);

		/* 分期警語 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "G::%s\x0D\x0A", _ESC_BMP_LEGAL_LOGO_);
                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
	}
		
        return (inDataLen);
}

/*
Function        :inNCCC_ESC_PRINT_REDEEM_Small
Date&Time       :2023/3/8 下午 5:36
Describe        :組E1 紅利
*/
int inNCCC_ESC_PRINT_REDEEM_Small(TRANSACTION_OBJECT *pobTran, char *szPackData)
{
	int	inDataLen = 0;
        char    szPrintBuf[84 + 1], szTemplate[42 + 1], szTemplate1[42 + 1];

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));
		
	if (pobTran->srBRec.uszRedeemBit == VS_TRUE)
	{
		if (pobTran->srBRec.uszVOIDBit == VS_TRUE)
		{
			if (pobTran->srBRec.inOrgCode == _REDEEM_SALE_ || pobTran->srBRec.inOrgCode == _REDEEM_ADJUST_)
			{
				/* 支付金額 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", (0 - pobTran->srBRec.lnRedemptionPaidCreditAmount + pobTran->srBRec.lnTipTxnAmount));
				inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, _PADDING_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "22::支付金額%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

				memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);

				/* 紅利扣抵金額 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", (0 - (pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnRedemptionPaidCreditAmount)));
				inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, _PADDING_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "22::紅利扣抵金額%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

				memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);
				/* 紅利扣抵點數 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%ld 點", (0 - pobTran->srBRec.lnRedemptionPoints));

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "22::紅利扣抵點數%s%s\x0D\x0A", _DATA_FLAG_, szTemplate1);

				memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);
			}
			else if (pobTran->srBRec.inOrgCode == _REDEEM_REFUND_)
			{
				/* 支付金額 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", pobTran->srBRec.lnRedemptionPaidCreditAmount);
				inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, _PADDING_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "22::支付金額%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

				memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);

				/* 紅利扣抵金額 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", (pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnRedemptionPaidCreditAmount));
				inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, _PADDING_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "22::紅利扣抵金額%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

				memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);
				/* 紅利扣抵點數 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%ld 點", (pobTran->srBRec.lnRedemptionPoints));

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "22::紅利扣抵點數%s%s\x0D\x0A", _DATA_FLAG_, szTemplate1);

				memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);
			}
		}
		/* 正向交易 */
		else
		{
			if (pobTran->srBRec.inCode == _REDEEM_REFUND_)
			{
				/* 支付金額 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", (0 - pobTran->srBRec.lnRedemptionPaidCreditAmount));
				inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, _PADDING_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "22::支付金額%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

				memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);

				/* 紅利扣抵金額 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", (0 - (pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnRedemptionPaidCreditAmount)));
				inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, _PADDING_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "22::紅利扣抵金額%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

				memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);
				/* 紅利扣抵點數 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%ld 點", (0 - (pobTran->srBRec.lnRedemptionPoints)));

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "22::紅利扣抵點數%s%s\x0D\x0A", _DATA_FLAG_, szTemplate1);

				memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);
			}
			else if (pobTran->srBRec.inCode == _REDEEM_ADJUST_)
			{
				/* 支付金額 因為是調帳所以不會有小費*/
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", pobTran->srBRec.lnRedemptionPaidCreditAmount);
				inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, _PADDING_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "22::支付金額%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

				memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);

				/* 紅利扣抵金額 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", (pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnRedemptionPaidCreditAmount));
				inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, _PADDING_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "22::紅利扣抵金額%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

				memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);
				/* 紅利扣抵點數 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%ld 點", (pobTran->srBRec.lnRedemptionPoints));

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "22::紅利扣抵點數%s%s\x0D\x0A", _DATA_FLAG_, szTemplate1);

				memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);
			}
			else
			{
				/* 支付金額 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", pobTran->srBRec.lnRedemptionPaidCreditAmount + pobTran->srBRec.lnTipTxnAmount);
				inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, _PADDING_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "22::支付金額%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

				memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);

				/* 紅利扣抵金額 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", (pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnRedemptionPaidCreditAmount));
				inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, _PADDING_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "22::紅利扣抵金額%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

				memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);
				/* 紅利扣抵點數 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%ld 點", (pobTran->srBRec.lnRedemptionPoints));

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "22::紅利扣抵點數%s%s\x0D\x0A", _DATA_FLAG_, szTemplate1);

				memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);
				
				
				/* 紅利剩餘點數 */
				if (pobTran->inTransactionCode == _REDEEM_SALE_ || pobTran->srBRec.inCode == _REDEEM_SALE_)
				{
					/* 紅利扣抵才印 */
					/* 將NT$ ＋數字塞到szTemplate中來inpad */
					memset(szTemplate1, 0x00, sizeof(szTemplate1));
					sprintf(szTemplate1, "%ld 點", (pobTran->srBRec.lnRedemptionPointsBalance));

					/* 把前面的字串和數字結合起來 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "22::紅利剩餘點數%s%s\x0D\x0A", _DATA_FLAG_, szTemplate1);

					memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
					inDataLen += strlen(szPrintBuf);
				}
				
			}
			
		}
	}
		
        return (inDataLen);
}

/*
Function        :inNCCC_ESC_PRINT_ReceiptEND_Small
Date&Time       :2023/3/8 下午 5:37
Describe        :組E1 結尾
*/
int inNCCC_ESC_PRINT_ReceiptEND_Small(TRANSACTION_OBJECT *pobTran, char *szPackData)
{
	int	inDataLen = 0;
	char	szPrintBuf[84 + 1], szTemplate[42 + 1] = {0};
	char	szPrtNotice[1 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
        
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	/* 簽名欄 */
	/* 免簽名 */
	/* 小費一律簽名 */
	if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE && pobTran->srBRec.inCode != _TIP_)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "11::%s\x0D\x0A", "X:                                          免簽名");

		memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
	}
	/* (需求單 - 107227)邦柏科技自助作業客製化 by Russell 2018/12/7 下午 2:36 */
	/* (需求單 - 107276)自助交易標準400做法 by Russell 2018/12/27 上午 11:20 */
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)         ||
                 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_))
	{
		/* 一律送免簽名 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "11::%s\x0D\x0A", "X:                                          免簽名");

		memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
	}
	/* 要簽名 */
	else
	{
		/* 有在signpad簽名 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
		inFunc_ComposeFileName_InvoiceNumber(pobTran, szTemplate, _PICTURE_FILE_EXTENSION_, 6);

		if (inFILE_Check_Exist((unsigned char *)szTemplate) == VS_SUCCESS)
		{
			/* 電子簽名 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "G::%s\x0D\x0A", _ESC_BMP_SIGNATURE_);
			memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
		}
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "14::%s\x0D\x0A", "X:");
		memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
	}
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "14::%s\x0D\x0A", "----------------------------");
	memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
	inDataLen += strlen(szPrintBuf);

	/* "持卡人姓名" */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "12::%s\x0D\x0A", "持卡人簽名(特店存根聯)");
	memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
	inDataLen += strlen(szPrintBuf);
	
	/* 持卡人姓名值 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "11::%s\x0D\x0A", pobTran->srBRec.szCardHolder);
	memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
	inDataLen += strlen(szPrintBuf);
		
	/* 列印警示語 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "12::%s\x0D\x0A", "I AGREE TO PAY TOTAL AMOUNT");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "12::%s\x0D\x0A", "ACCORDING TO CARD ISSUER AGREEMENT");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* Print Notice */
	memset(szPrtNotice, 0x00, sizeof(szPrtNotice));
	inGetPrtNotice(szPrtNotice);
	if(memcmp(szPrtNotice, "Y", strlen("Y")) == 0)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "G::%s\x0D\x0A", _ESC_BMP_NOTICE_LOGO_);
                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
	}
	
        return (inDataLen);
}

/*
Function        :inNCCC_ESC_PRINT_DCC_DATA_Small
Date&Time       :2023/3/9 上午 10:41
Describe        :組E1 DCC DATA部份
*/
int inNCCC_ESC_PRINT_DCC_DATA_Small(TRANSACTION_OBJECT *pobTran, char *szPackData)
{
	int	i = 0;
	int	inDataLen = 0;
	int	inTempLen = 0;
	int	inUpperAmount = 0;
	int	inLowerAmount = 0;
	char	szPrintBuf[200 + 1] = {0}, szPrintBuf1[100 + 1] = {0}, szPrintBuf2[100 + 1] = {0}, szTemplate[100 + 1] = {0}, szTemplate1[100 + 1] = {0}, szTemplate2[100 + 1] = {0};
	char	szProductCodeEnable[1 + 1];
	char	szStore_Stub_CardNo_Truncate_Enable[2 + 1];
	char	szExamBit[10 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);

	/* "卡號 卡別" 卡別值 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	strcpy(szTemplate, pobTran->srBRec.szCardLabel);
	
	inTempLen =  ((20 - strlen(szTemplate)) * _ESC_SMALL_CARD_TYPE_MULTIPLE_) + _ESC_SMALL_CARD_TYPE_OFFSET_;
	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	for (i = 0; i < inTempLen; i++)
	{
		strcat(szTemplate2, " ");
	}
	strcat(szTemplate2, szTemplate);
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	strcat(szPrintBuf, "29::");
	strcat(szPrintBuf, "卡號");
	strcat(szPrintBuf, "\x20");
	strcat(szPrintBuf, _DATA_FLAG_);
	strcat(szPrintBuf, "卡別");
	strcat(szPrintBuf, "\x20");
	strcat(szPrintBuf, szTemplate2);
	strcat(szPrintBuf, "\x0D\x0A");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	/* 卡號值 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	strcpy(szTemplate, pobTran->srBRec.szPAN);
	
	/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
	/* 商店聯卡號遮掩 */
	memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
	inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
	/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/8 下午 1:55 */
	if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
	{
		/* Uny交易不受TMS開關限制，且獲取卡號時已遮掩 */
	}
	else if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0 && pobTran->srBRec.uszTxNoCheckBit == VS_TRUE)
	{
		/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
		for (i = 6; i < (strlen(szTemplate) - 4); i ++)
			szTemplate[i] = 0x2A;
	}

	/* 過卡方式 */
	if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
	{
		if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
			strcat(szTemplate, "(W)");
		else
			strcat(szTemplate, "(C)");
	}
	else if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
	{
		strcat(szTemplate, "(Q)");
	}
	else if (pobTran->srBRec.uszCUPEMVQRCodeBit == VS_TRUE)
	{
		strcat(szTemplate, "(Q)");
	}
	else
	{
		if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
			strcat(szTemplate,"(C)");
		else if (pobTran->srBRec.uszMobilePayBit == VS_TRUE)
			strcat(szTemplate, "(T)");
		else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
			strcat(szTemplate, "(W)");
		else
		{
			if (pobTran->srBRec.uszManualBit == VS_TRUE)
			{
				/* 【需求單 - 105244】端末設備支援以感應方式進行退貨交易 */
				/* 電文轉Manual Keyin但是簽單要印感應的W */
				if (pobTran->srBRec.uszRefundCTLSBit == VS_TRUE)
					strcat(szTemplate, "(W)");
				else
					strcat(szTemplate,"(M)");
			}
			else
				strcat(szTemplate,"(S)");
		}
	}
       	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", szTemplate);

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	/* 交易類別 */
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s %s\x0D\x0A", "交易", szTemplate1);

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	if (strlen(szTemplate2) > 0)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "11::%s\x0D\x0A", szTemplate2);

		memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
	}
	
	/* 城市 主機 */
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	inGetCityName(szTemplate1);
	
	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	inGetHostLabel(szTemplate2);
	inFunc_DiscardSpace(szTemplate2);
	inFunc_PAD_ASCII(szTemplate2, szTemplate2, ' ', 8, _PADDING_LEFT_);
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	strcat(szPrintBuf, "40::");
	strcat(szPrintBuf, "城市");
	strcat(szPrintBuf, _DATA_FLAG_);
	strcat(szPrintBuf, szTemplate1);
	strcat(szPrintBuf, _DATA_FLAG_);
	strcat(szPrintBuf, "主機");
	strcat(szPrintBuf, _DATA_FLAG_);
	strcat(szPrintBuf, szTemplate2);
	strcat(szPrintBuf, "\x0D\x0A");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	/* "日期/時間" 日期值 "批號" 批號值 */
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	sprintf(szTemplate1, "%.4s/%.2s/%.2s %.2s:%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6], &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
	
	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	sprintf(szTemplate2, "%03ld", pobTran->srBRec.lnBatchNum);
	inFunc_PAD_ASCII(szTemplate2, szTemplate2, ' ', 8, _PADDING_LEFT_);
		
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	strcat(szPrintBuf, "40::");
	strcat(szPrintBuf, "日期/時間");
	strcat(szPrintBuf, _DATA_FLAG_);
	strcat(szPrintBuf, szTemplate1);
	strcat(szPrintBuf, _DATA_FLAG_);
	strcat(szPrintBuf, "批號");
	strcat(szPrintBuf, _DATA_FLAG_);
	strcat(szPrintBuf, szTemplate2);
	strcat(szPrintBuf, "\x0D\x0A");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* "授權碼" 授權碼值 "檢查碼" 檢查碼值 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	memcpy(szTemplate1, &pobTran->srBRec.szAuthCode[0], 12);
	
	if (pobTran->srBRec.uszTakaTransBit == VS_TRUE)
	{
		memset(szTemplate2, 0x00, sizeof(szTemplate2));
		sprintf(szTemplate2, "%.2s/'%.2s", pobTran->srBRec.szExpDate + 2, pobTran->srBRec.szExpDate);
	}
	else
	{
		memset(szTemplate2, 0x00, sizeof(szTemplate2));
		/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/28 下午 2:42 */
		if (strlen(pobTran->srBRec.szCheckNO) > 0)
		{
			strcpy(szTemplate2, pobTran->srBRec.szCheckNO);
		}
		else
		{
			inCARD_ExpDateEncryptAndDecrypt(pobTran, "", szTemplate2, _EXP_ENCRYPT_);
		}
		inFunc_PAD_ASCII(szTemplate2, szTemplate2, ' ', 9, _PADDING_LEFT_);
	}
	
	/* 大高卡只能顯示有效期 */
	if (pobTran->srBRec.uszTakaTransBit == VS_TRUE)
	{
		strcat(szPrintBuf, "40::");
		strcat(szPrintBuf, "授權碼");
		strcat(szPrintBuf, _DATA_FLAG_);
		strcat(szPrintBuf, szTemplate1);
		strcat(szPrintBuf, _DATA_FLAG_);
		strcat(szPrintBuf, "有效期");
		strcat(szPrintBuf, _DATA_FLAG_);
		strcat(szPrintBuf, szTemplate2);
		strcat(szPrintBuf, "\x0D\x0A");
	}
	else
	{
		strcat(szPrintBuf, "40::");
		strcat(szPrintBuf, "授權碼");
		strcat(szPrintBuf, _DATA_FLAG_);
		strcat(szPrintBuf, szTemplate1);
		strcat(szPrintBuf, _DATA_FLAG_);
		strcat(szPrintBuf, "檢查碼");
		strcat(szPrintBuf, _DATA_FLAG_);
		strcat(szPrintBuf, szTemplate2);
		strcat(szPrintBuf, "\x0D\x0A");
	}

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* "序號" 序號值 "調閱號" 調閱編號值 */
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	inFunc_PAD_ASCII(szTemplate1, pobTran->srBRec.szRefNo, ' ', 25, _PADDING_RIGHT_);

	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	sprintf(szTemplate2, "%06ld", pobTran->srBRec.lnOrgInvNum);
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	strcat(szPrintBuf, "40::");
	strcat(szPrintBuf, "序號");
	strcat(szPrintBuf, _DATA_FLAG_);
	strcat(szPrintBuf, szTemplate1);
	strcat(szPrintBuf, _DATA_FLAG_);
	strcat(szPrintBuf, "調閱號");
	strcat(szPrintBuf, _DATA_FLAG_);
	strcat(szPrintBuf, szTemplate2);
	strcat(szPrintBuf, "\x0D\x0A");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 櫃號 */
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	inGetStoreIDEnable(szTemplate1);
	if ((memcmp(&szTemplate1[0], "Y", 1) == 0) && (strlen(pobTran->srBRec.szStoreID) > 0))
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memcpy(szTemplate1, &pobTran->srBRec.szStoreID[0], 23);
		
		memset(szTemplate2, 0x00, sizeof(szTemplate2));
		sprintf(szTemplate2, "%s", pobTran->srBRec.szRespCode);
		inFunc_PAD_ASCII(szTemplate2, szTemplate2, ' ', 9, _PADDING_LEFT_);
		
		if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
		{
			strcat(szPrintBuf, "40::");
			strcat(szPrintBuf, "櫃號");
			strcat(szPrintBuf, _DATA_FLAG_);
			strcat(szPrintBuf, szTemplate1);
			strcat(szPrintBuf, _DATA_FLAG_);
			strcat(szPrintBuf, "回覆碼");
			strcat(szPrintBuf, _DATA_FLAG_);
			strcat(szPrintBuf, szTemplate2);
			strcat(szPrintBuf, "\x0D\x0A");
		}
		else
		{
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_026_TAKA_, _CUSTOMER_INDICATOR_SIZE_) ||
			    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
			{
				strcat(szPrintBuf, "40::");
				strcat(szPrintBuf, "品群碼");
				strcat(szPrintBuf, _DATA_FLAG_);
				strcat(szPrintBuf, szTemplate1);
				strcat(szPrintBuf, _DATA_FLAG_);
				strcat(szPrintBuf, "\x20");
				strcat(szPrintBuf, _DATA_FLAG_);
				strcat(szPrintBuf, "\x20");
				strcat(szPrintBuf, "\x0D\x0A");
			}
			else
			{
				strcat(szPrintBuf, "40::");
				strcat(szPrintBuf, "櫃號");
				strcat(szPrintBuf, _DATA_FLAG_);
				strcat(szPrintBuf, szTemplate1);
				strcat(szPrintBuf, _DATA_FLAG_);
				strcat(szPrintBuf, "\x20");
				strcat(szPrintBuf, _DATA_FLAG_);
				strcat(szPrintBuf, "\x20");
				strcat(szPrintBuf, "\x0D\x0A");
			}	
		}
		
		memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
		
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			/* hardcode */
			char	szPOS_ID[6 + 1] = {0};
			memset(szPOS_ID, 0x00, sizeof(szPOS_ID));
			inGetPOS_ID(szPOS_ID);

			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			memcpy(szTemplate1, _HARDCODE_STORE_ID_CUS_021_TAKAWEL_, 12);
			memcpy(&szTemplate1[12], szPOS_ID, 6);
			
			memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
		}
	}
	else
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memcpy(szTemplate1, &pobTran->srBRec.szStoreID[0], 23);
		
		memset(szTemplate2, 0x00, sizeof(szTemplate2));
		sprintf(szTemplate2, "%s", pobTran->srBRec.szRespCode);
		inFunc_PAD_ASCII(szTemplate2, szTemplate2, ' ', 9, _PADDING_LEFT_);
		
		if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
		{
			strcat(szPrintBuf, "40::");
			strcat(szPrintBuf, "\x20");
			strcat(szPrintBuf, _DATA_FLAG_);
			strcat(szPrintBuf, "\x20");
			strcat(szPrintBuf, _DATA_FLAG_);
			strcat(szPrintBuf, "回覆碼");
			strcat(szPrintBuf, _DATA_FLAG_);
			strcat(szPrintBuf, szTemplate2);
			strcat(szPrintBuf, "\x0D\x0A");
			
			memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
		}
	}
        
        /* 【需求單-113260】MasterCard機票交易需求 by Russell 2025/3/3 上午 11:55 */
        /* 取消維持原交易不列印機票資訊 */
        if (inNCCC_Func_Flight_Ticket_Allow_Print(pobTran) == VS_TRUE)
        {
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "27::%s%s%s\x0D\x0A", "出發地機場(departure)", _DATA_FLAG_, pobTran->srBRec.szFlightTicketPDS0523);

                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "27::%s%s%s\x0D\x0A", "目的地機場(arrival)", _DATA_FLAG_, pobTran->srBRec.szFlightTicketPDS0524);

                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);

                memset(szTemplate2, 0x00, sizeof(szTemplate2));
		if (strlen(pobTran->srBRec.szFlightTicketPDS0530) > 0)
                {
                        sprintf(szTemplate2, "%d", atoi(pobTran->srBRec.szFlightTicketPDS0530));
                }
                else
                {
                        sprintf(szTemplate2, "%s", " ");
                }
                sprintf(szPrintBuf, "%s", szTemplate2);
                
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "27::%s%s%s\x0D\x0A", "航班號碼(Flight No.)", _DATA_FLAG_, szTemplate2);

                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
                
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		inNCCC_Func_Get_Flight_Ticket_Txn_Category_PrintName(pobTran, szTemplate1);
                        
		if (!memcmp(szTemplate1, _FLIGHT_TICKET_TXN_CATEGORY_PRINT_BAGGAGE_CHARGE_, max(strlen(szTemplate1), strlen(_FLIGHT_TICKET_TXN_CATEGORY_PRINT_BAGGAGE_CHARGE_)))		||
		    !memcmp(szTemplate1, _FLIGHT_TICKET_TXN_CATEGORY_PRINT_PREPAID_TICKET_, max(strlen(szTemplate1), strlen(_FLIGHT_TICKET_TXN_CATEGORY_PRINT_PREPAID_TICKET_)))		||
		    !memcmp(szTemplate1, _FLIGHT_TICKET_TXN_CATEGORY_PRINT_MISCELLANEOUS_CHARGE_ , max(strlen(szTemplate1), strlen(_FLIGHT_TICKET_TXN_CATEGORY_PRINT_MISCELLANEOUS_CHARGE_)))	||
		    !memcmp(szTemplate1, _FLIGHT_TICKET_TXN_CATEGORY_PRINT_TICKET_REISSUE_, max(strlen(szTemplate1), strlen(_FLIGHT_TICKET_TXN_CATEGORY_PRINT_TICKET_REISSUE_))))
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "27::%s%s%s\x0D\x0A", "交易類型(transaction category)", _DATA_FLAG_, " ");

			memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "25::%s%s%s\x0D\x0A", " ", _DATA_FLAG_, szTemplate1);

			memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
		}
		else
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "27::%s%s%s\x0D\x0A", "交易類型(transaction category)", _DATA_FLAG_, szTemplate1);

			memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
		}
        }
	
	/* TC */
	/* 驗測開關 */
	memset(szExamBit, 0x00, sizeof(szExamBit));
	inGetExamBit(szExamBit);
	/* Uny交易 */
	/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/8 下午 1:55 */
	if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
	{
		/* 取消或退貨，只印數字交易碼 */
		/* 最小字體 */
		if (pobTran->srBRec.uszVOIDBit == VS_TRUE ||
		    pobTran->srBRec.inCode == _REFUND_ || pobTran->srBRec.inCode == _INST_REFUND_ || pobTran->srBRec.inCode == _REDEEM_REFUND_)
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			sprintf(szTemplate1, "交易碼 %s", pobTran->srBRec.szUnyTransCode);
			sprintf(szPrintBuf, "11::%s\x0D\x0A", szTemplate1);

			memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
		}
		/* 正向交易，要印條碼和數字 */
		else
		{

			if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
			{
				/* 一維條碼 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%s", pobTran->srBRec.szUnyTransCode);
				sprintf(szPrintBuf, "U::%s\x0D\x0A", szTemplate1);
				
				memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);

				/* 數字 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "交易碼(退貨請掃描) %s", pobTran->srBRec.szUnyTransCode);
				sprintf(szPrintBuf, "11::%s\x0D\x0A", szTemplate1);

				memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);
			}
			else
			{
				/* 一維條碼 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%s", pobTran->srBRec.szUnyTransCode);
				sprintf(szPrintBuf, "U::%s\x0D\x0A", szTemplate1);

				memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);

				/* 數字 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "交易碼 %s", pobTran->srBRec.szUnyTransCode);
				sprintf(szPrintBuf, "11::%s\x0D\x0A", szTemplate1);

				memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);
			}
		}
	}
	/* 實體卡交易和掃碼交易互斥 */
	else
	{
		if (pobTran->srBRec.inChipStatus == _EMV_CARD_ || pobTran->srBRec.uszContactlessBit == VS_TRUE)
		{
			/* 感應磁條 */
			if (!memcmp(pobTran->srBRec.szAuthCode, "VLP", 3)				|| 
			    !memcmp(pobTran->srBRec.szAuthCode, "JCB", 3)				||
			     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE		|| 
			     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_64_NEWJSPEEDY_MSD		||
			     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE)
			{
				/* 商店聯卡號遮掩 */
				memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
				inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
				if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
				    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
				    strlen(pobTran->srBRec.szTxnNo) > 0)
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "11::%s %s\x0D\x0A", "交易編號", pobTran->srBRec.szTxnNo);
					memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
					inDataLen += strlen(szPrintBuf);
				}
			}
			else
			{
				if (pobTran->srBRec.inPrintOption == _PRT_MERCH_)
				{
					if (pobTran->srEMVRec.in9F26_ApplCryptogramLen > 0)
					{
						memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
						sprintf(szPrintBuf1, "%02X%02X%02X%02X%02X%02X%02X%02X",
										pobTran->srEMVRec.usz9F26_ApplCryptogram[0],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[1],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[2],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[3],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[4],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[5],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[6],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[7]);

						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "23::TC%s%s\x0D\x0A", _DATA_FLAG_, szPrintBuf1);

						memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
						inDataLen += strlen(szPrintBuf);
					}

					/* 商店聯卡號遮掩 */
					memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
					inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
					if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
					    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
					    strlen(pobTran->srBRec.szTxnNo) > 0)
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "11::%s %s\x0D\x0A", "交易編號", pobTran->srBRec.szTxnNo);
						memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
						inDataLen += strlen(szPrintBuf);
					}

					/* M/C交易列印AP Lable (START) */
					if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)))
					{
						if (pobTran->srEMVRec.in50_APLabelLen > 0)
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							strcat(szPrintBuf, "AP Label:");
							memset(szTemplate, 0x00, sizeof(szTemplate));
							strcpy(szTemplate, (char*)pobTran->srEMVRec.usz50_APLabel);
							inUpperAmount = inFunc_Get_UpperCase_Char(szTemplate);
							inLowerAmount = strlen(szTemplate) - inUpperAmount;
							inTempLen =  _ESC_AP_LABEL_TOTAL_SPACE_ - (_ESC_UPPER_MULTIPLE_ * inUpperAmount + _ESC_LOWER_MULTIPLE_ * inLowerAmount);
							for (i = 0; i < inTempLen; i++)
							{
								strcat(szPrintBuf, " ");
							}
							strcat(szPrintBuf, szTemplate);
						}
						else
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							strcat(szPrintBuf, "AP Label:");
							strcat(szPrintBuf, "\x20");
						}
					}
					/* M/C交易列印AP Lable (END) */
					else if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)))
					{
						if (pobTran->srEMVRec.in50_APLabelLen > 0)
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							strcat(szPrintBuf, "AP Label:");
							memset(szTemplate, 0x00, sizeof(szTemplate));
							strcpy(szTemplate, (char*)pobTran->srEMVRec.usz50_APLabel);
							inUpperAmount = inFunc_Get_UpperCase_Char(szTemplate);
							inLowerAmount = strlen(szTemplate) - inUpperAmount;
							inTempLen =  _ESC_AP_LABEL_TOTAL_SPACE_ - (_ESC_UPPER_MULTIPLE_ * inUpperAmount + _ESC_LOWER_MULTIPLE_ * inLowerAmount);
							for (i = 0; i < inTempLen; i++)
							{
								strcat(szPrintBuf, " ");
							}
							strcat(szPrintBuf, szTemplate);
						}
						else
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							strcat(szPrintBuf, "AP Label:");
							strcat(szPrintBuf, "\x20");
						}
					}
					/* 驗測要印AP Label 和 AID */
					else if (szExamBit[0] == '1')
					{
						if (pobTran->srEMVRec.in50_APLabelLen > 0)
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							strcat(szPrintBuf, "AP Label:");
							memset(szTemplate, 0x00, sizeof(szTemplate));
							strcpy(szTemplate, (char*)pobTran->srEMVRec.usz50_APLabel);
							inUpperAmount = inFunc_Get_UpperCase_Char(szTemplate);
							inLowerAmount = strlen(szTemplate) - inUpperAmount;
							inTempLen =  _ESC_AP_LABEL_TOTAL_SPACE_ - (_ESC_UPPER_MULTIPLE_ * inUpperAmount + _ESC_LOWER_MULTIPLE_ * inLowerAmount);
							for (i = 0; i < inTempLen; i++)
							{
								strcat(szPrintBuf, " ");
							}
							strcat(szPrintBuf, szTemplate);
						}
						else
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							strcat(szPrintBuf, "AP Label:");
							strcat(szPrintBuf, "\x20");
						}
					}
					else
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						strcat(szPrintBuf, "\x20");
						strcat(szPrintBuf, "\x20");
					}
					memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
					sprintf(szPrintBuf2, szPrintBuf);
					
					/* AID */
					if (strlen(pobTran->srBRec.szCUP_EMVAID) > 0)
					{
						memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
						sprintf(szPrintBuf1, "AID:%s", pobTran->srBRec.szCUP_EMVAID);
						inFunc_PAD_ASCII(szPrintBuf1, szPrintBuf1, ' ', _ESC_AID_TOTAL_LEN_, _PADDING_RIGHT_);
						
						if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
						{
							/* CUP晶片要印 */
							if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
							else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)) == 0)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
							/* 【需求單-109121】Mastercard交易簽單列印AID需求 by Russell 2020/7/23 下午 4:23 */
							else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)) == 0)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
							/* 【需求單-111290】VISA置換Logo、DISCOVER簽單新增AID欄位、AE簽單新增AID和AP Label欄位需求 by Russell 2022/12/29 下午 5:57 */
							else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)) == 0 &&
								pobTran->srBRec.uszUCARDTransBit != VS_TRUE)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
							/* 驗測要印AP Label 和 AID */
							else if (szExamBit[0] == '1')
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
						}
						/* 銀聯閃付 */
						else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
						{
							if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
							/* AE晶片要印出AID */
							else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_50_EXPRESSSPAY || 
								 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
							/* 【需求單-109121】Mastercard交易簽單列印AID需求 by Russell 2020/7/23 下午 4:23 */
							else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE || 
								 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_21_PAYPASS_MCHIP)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
							else if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_42_DPAS	|| pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_43_DISCOVER_DPAS_MAG_STRIPE) &&
								pobTran->srBRec.uszUCARDTransBit != VS_TRUE)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
							/* 驗測要印AP Label 和 AID */
							else if (szExamBit[0] == '1')
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
						}
					}
				}
			}

		}
		else
		{
			/* 商店聯卡號遮掩 */
			/* 磁條卡列印交易編號 */
			memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
			inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
			if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
			    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
			    strlen(pobTran->srBRec.szTxnNo) > 0)
			{
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "11::%s %s\x0D\x0A", "交易編號", pobTran->srBRec.szTxnNo);
				memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);
			}
		}
	}
	
	/* 產品代碼 */
	/* 吳升文回覆 產品代碼列印於TC欄位下方 */
	inGetProductCodeEnable(szProductCodeEnable);
	if (memcmp(szProductCodeEnable, "Y", 1) == 0)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		strcat(szPrintBuf, "40::");
		strcat(szPrintBuf, "產品代碼");
		strcat(szPrintBuf, _DATA_FLAG_);
		strcat(szPrintBuf, pobTran->srBRec.szProductCode);
		strcat(szPrintBuf, _DATA_FLAG_);
		strcat(szPrintBuf, "\x20");
		strcat(szPrintBuf, _DATA_FLAG_);
		strcat(szPrintBuf, "\x20");
		strcat(szPrintBuf, "\x0D\x0A");

                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
	}
	
	return (inDataLen);
}

/*
Function        :inNCCC_ESC_PRINT_DCC_AMOUNT_FOR_SALE_Small
Date&Time       :2023/3/10 下午 3:07
Describe        :
*/
int inNCCC_ESC_PRINT_DCC_AMOUNT_FOR_SALE_Small(TRANSACTION_OBJECT *pobTran, char *szPackData)
{
	int	inDataLen = 0;
	int	inIRDU = 0;
	char	szPrintBuf[200 + 1] = {0}, szPrintBuf1[100 + 1] = {0}, szPrintBuf2[100 + 1] = {0};
	char	szIRDU[12 + 1] = {0};			/* Inverted Rate Display Unit */
	char	szExchangeAmt[50 + 1] = {0};		/* 轉換金額 */

	/*   */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "12::%s\x0D\x0A", " ");
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* Please select */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "12::%s\x0D\x0A", "Please select");
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* Transaction currency as below */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "12::%s\x0D\x0A", "Transaction currency as below");
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 外幣     1.  [ X ]  EUR 8.18 上需空一行 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "12::%s\x0D\x0A", " ");
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	/* 外幣 */
	memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
	inFunc_Amount_Comma_DCC(pobTran->srBRec.szDCC_FCA, "", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, pobTran->srBRec.szDCC_FCMU, "", szPrintBuf2);

	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	sprintf(szPrintBuf1, "1.【X】%s %s", pobTran->srBRec.szDCC_FCAC, szPrintBuf2);						/* Foreign currcncy Alphabetic Code */
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", szPrintBuf1);
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 匯率 */
	/* Inverted Rate Display Unit */
	inIRDU = atoi(pobTran->srBRec.szDCC_IRDU); 
	memset(szIRDU, 0x00, sizeof(szIRDU));
	strcpy(szIRDU, "1");
	inFunc_PAD_ASCII(szIRDU, szIRDU, '0', (inIRDU + 1), _PADDING_RIGHT_);
	
	memset(szExchangeAmt, 0x00, sizeof(szExchangeAmt));
	inFunc_Amount_Comma_DCC(pobTran->srBRec.szDCC_IRV, "", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, pobTran->srBRec.szDCC_IRMU, "", szExchangeAmt);
	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	sprintf(szPrintBuf1, "Exchange Rate:");
	memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
	sprintf(szPrintBuf2, "%s %s = %s NTD", szIRDU, pobTran->srBRec.szDCC_FCAC, szExchangeAmt);
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s%s\x0D\x0A", szPrintBuf1, szPrintBuf2);
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 轉換費率 */
	/* XX.XX %*/
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	inFunc_Currency_Conversion_Fee(pobTran->srBRec.szDCC_MPV, pobTran->srBRec.szDCC_MPDP, szPrintBuf1);
	strcat(szPrintBuf1, " %");
	
	sprintf(szPrintBuf, "29::%s%s%s\x0D\x0A", "轉換費率(Currency Conversion Fee)", _DATA_FLAG_, szPrintBuf1);
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	/* (Currency Conversion Fee on the exchange rate over a wholesale rate.) */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", "(Currency Conversion Fee on the");
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* exchange rate over a wholesale rate.) */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", "exchange rate over a wholesale rate.)");
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 台幣 */
	memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
	sprintf(szPrintBuf2, "%ld", pobTran->srBRec.lnTxnAmount);
	inFunc_Amount_Comma_DCC(szPrintBuf2, "", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, "0", "", szPrintBuf2);

	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	/* 轉台幣跑另外一隻 */
	sprintf(szPrintBuf1, "2.【 】%s %s", "NTD", szPrintBuf2);									/* Foreign currcncy Alphabetic Code */
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", szPrintBuf1);
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	/* 有開小費要多印 */
	memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
	inGetTransFunc(szPrintBuf2);
	/* 檢查是否有開小費 */
	/* 預先授權完成必定不印小費 */
	if (szPrintBuf2[6] == 'Y'	&&
	    pobTran->srBRec.inCode == _SALE_)
	{
		/* 小費     小費（Tips）： EUR  _______________ 上需空一行 by Russell 2020/10/29 下午 3:20 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "12::%s\x0D\x0A", " ");
		memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
		
		/* 小費跟總計不需列印外幣 by Russell 2020/10/29 下午 3:29 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	        sprintf(szPrintBuf, "20::小費(Tips)%s%s%s\x0D\x0A", _DATA_FLAG_, "", "____________");
                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "20::總計(Total)%s%s%s\x0D\x0A", _DATA_FLAG_, "", "____________");
                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
	}
	
	return (inDataLen);
}

/*
Function        :inNCCC_ESC_PRINT_DCC_ReceiptEND_Small
Date&Time       :2023/3/10 下午 4:20
Describe        :
*/
int inNCCC_ESC_PRINT_DCC_ReceiptEND_Small(TRANSACTION_OBJECT *pobTran, char *szPackData)
{
	int	inTempLen = 0;
	int	inDataLen = 0;
	char	szPrtNotice[1 + 1];
	char	szPrintBuf[200 + 1], szTemplate[100 + 1];
	char	szTempBuff[1000] = {};

	/* 免責宣言上需空一行 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "12::%s\x0D\x0A", " ");
	memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
	inDataLen += strlen(szPrintBuf);
	
	/* Disclaimer */
	if ((!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_VISA_, strlen(_CARD_TYPE_VISA_))) && 
	    (pobTran->srBRec.inCode == _SALE_		|| 
	     pobTran->srBRec.inCode == _TIP_		|| 
	     pobTran->srBRec.inCode == _PRE_COMP_	||
	    (pobTran->srBRec.inCode == _SALE_ && pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE)))
	{
		memset(szTempBuff, 0x00, sizeof(szTempBuff));
		inTempLen = inNCCC_ESC_PRINT_DCC_Disclaimer(_ESC_DCC_V_, szTempBuff);
		if (inTempLen > 0)
		{
		        memcpy(&szPackData[inDataLen], &szTempBuff[0], inTempLen);
                        inDataLen += inTempLen;
		}
	}
	else if ((!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_))) && 
		 (pobTran->srBRec.inCode == _SALE_	|| 
		  pobTran->srBRec.inCode == _TIP_	|| 
		  pobTran->srBRec.inCode == _PRE_COMP_	||
		 (pobTran->srBRec.inCode == _SALE_ && pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE)))
	{
		memset(szTempBuff, 0x00, sizeof(szTempBuff));
		inTempLen = inNCCC_ESC_PRINT_DCC_Disclaimer(_ESC_DCC_M_, szTempBuff);
		if (inTempLen > 0)
		{
		        memcpy(&szPackData[inDataLen], &szTempBuff[0], inTempLen);
                        inDataLen += inTempLen;
		}
	}
	
	if (pobTran->srBRec.inPrintOption == _PRT_MERCH_	||
	    pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
        {
		if (pobTran->srBRec.uszDCCTransBit == VS_TRUE && pobTran->srBRec.inCode == _TIP_)
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "11::%s\x0D\x0A", "[ ] ACCEPT");
			memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
		}
	}
	
	if (pobTran->srBRec.uszDCCTransBit == VS_TRUE && pobTran->srBRec.inCode == _TIP_)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "11::%s\x0D\x0A", " ");
		memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "11::%s\x0D\x0A", "Cardholder expressly agrees to the Transaction ");
		memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "11::%s\x0D\x0A", "Receipt Information by marking the “accept box");
		memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "11::%s\x0D\x0A", "below.");
		memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "14::%s\x0D\x0A", "[ ] ACCEPT");
		memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
	}

	/* 簽名欄 */
	/* 免簽名 */
	/* 小費一律簽名 */
	if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE && pobTran->srBRec.inCode != _TIP_)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "12::%s\x0D\x0A", "免　　簽　　名");

		memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
	}
	/* 要簽名 */
	else
	{
		/* 有在signpad簽名 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
		inFunc_ComposeFileName_InvoiceNumber(pobTran, szTemplate, _PICTURE_FILE_EXTENSION_, 6);

		if (inFILE_Check_Exist((unsigned char *)szTemplate) == VS_SUCCESS)
		{
			/* 電子簽名 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "G::%s\x0D\x0A", _ESC_BMP_SIGNATURE_);
			memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
		}

	}

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "14::%s\x0D\x0A", "X:__________________________________");
	memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
	inDataLen += strlen(szPrintBuf);

	/* 持卡人姓名 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "11::%s\x0D\x0A", pobTran->srBRec.szCardHolder);
	memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
	inDataLen += strlen(szPrintBuf);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "12::%s\x0D\x0A", "持卡人簽名(特店存根聯)");
	memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
	inDataLen += strlen(szPrintBuf);
		
	/* 列印警示語 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "12::%s\x0D\x0A", "I AGREE TO PAY TOTAL AMOUNT");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "12::%s\x0D\x0A", "ACCORDING TO CARD ISSUER AGREEMENT");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* Print Notice */
	memset(szPrtNotice, 0x00, sizeof(szPrtNotice));
	inGetPrtNotice(szPrtNotice);
	if(memcmp(szPrtNotice, "Y", strlen("Y")) == 0)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "G::%s\x0D\x0A", _ESC_BMP_NOTICE_LOGO_);
                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
	}

	return (inDataLen);
}

/*
Function        :inNCCC_ESC_PRINT_DCC_Amount_NOT_FOR_SALE_Small
Date&Time       :2023/3/10 下午 6:07
Describe        :DCC 一段式 非SALE的其他交易
*/
int inNCCC_ESC_PRINT_DCC_Amount_NOT_FOR_SALE_Small(TRANSACTION_OBJECT *pobTran, char *szPackData)
{
	int	inDataLen = 0;
	int	inIRDU = 0;
	char	szIRDU[12 + 1] = {0};		/* Inverted Rate Display Unit */
	char	szPrintBuf[200 + 1] = {0}, szPrintBuf1[100 + 1] = {0}, szPrintBuf2[100 + 1] = {0}, szTemplate[42 + 1] = {0};
	char	szExchangeAmt[50 + 1] = {0};		/* 轉換金額 */
	long	lnTipTotalAmt = 0l;
	
	/* 台幣(Local Amour)  上需空一行 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", " ");
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	/* 台幣 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", "台幣(Local Amount)");

        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	if (pobTran->srBRec.inCode == _TIP_)
	{
		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
		if ((pobTran->srBRec.uszVOIDBit == VS_TRUE && pobTran->srBRec.inOrgCode != _REFUND_)	|| 
		     pobTran->srBRec.inCode == _REFUND_)
		{
			sprintf(szPrintBuf2, "%ld", 0 - pobTran->srBRec.lnTxnAmount);
		}
		else
		{
			sprintf(szPrintBuf2, "%ld", pobTran->srBRec.lnTxnAmount);
		}
		inFunc_Amount_Comma_DCC(szPrintBuf2, "", '\x00', _SIGNED_NONE_, 13, _PADDING_RIGHT_, "0", "", szPrintBuf2);

		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szPrintBuf1, "%s", szPrintBuf2);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "23::Amount%s:NTD %s\x0D\x0A", _DATA_FLAG_, szPrintBuf1);
                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);

		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
		if ((pobTran->srBRec.uszVOIDBit == VS_TRUE && pobTran->srBRec.inOrgCode != _REFUND_)	|| 
		     pobTran->srBRec.inCode == _REFUND_)
		{
			sprintf(szPrintBuf2, "%ld", 0 - pobTran->srBRec.lnTipTxnAmount);
		}
		else
		{
			sprintf(szPrintBuf2, "%ld", pobTran->srBRec.lnTipTxnAmount);
		}
		inFunc_Amount_Comma_DCC(szPrintBuf2, "", '\x00', _SIGNED_NONE_, 13, _PADDING_RIGHT_, "0", "", szPrintBuf2);

		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szPrintBuf1, "%s", szPrintBuf2);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "23::Tips%s:NTD %s\x0D\x0A", _DATA_FLAG_, szPrintBuf1);
                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);

		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
		if ((pobTran->srBRec.uszVOIDBit == VS_TRUE && pobTran->srBRec.inOrgCode != _REFUND_)	|| 
		     pobTran->srBRec.inCode == _REFUND_)
		{
			sprintf(szPrintBuf2, "%ld", 0 - pobTran->srBRec.lnTotalTxnAmount);
		}
		else
		{
			sprintf(szPrintBuf2, "%ld", pobTran->srBRec.lnTotalTxnAmount);
		}
		inFunc_Amount_Comma_DCC(szPrintBuf2, "", '\x00', _SIGNED_NONE_, 13, _PADDING_RIGHT_, "0", "", szPrintBuf2);

		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szPrintBuf1, "%s", szPrintBuf2);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "23::Total%s:NTD %s\x0D\x0A", _DATA_FLAG_, szPrintBuf1);
                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
	}
	else
	{
		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
		if ((pobTran->srBRec.uszVOIDBit == VS_TRUE && pobTran->srBRec.inOrgCode != _REFUND_)	|| 
		     pobTran->srBRec.inCode == _REFUND_)
		{
			sprintf(szPrintBuf2, "%ld", 0 - pobTran->srBRec.lnTotalTxnAmount);
		}
		else
		{
			sprintf(szPrintBuf2, "%ld", pobTran->srBRec.lnTotalTxnAmount);
		}
		inFunc_Amount_Comma_DCC(szPrintBuf2, "", '\x00', _SIGNED_NONE_, 13, _PADDING_RIGHT_, "0", "", szPrintBuf2);

		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szPrintBuf1, "%s", szPrintBuf2);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "23::Total%s:NTD %s\x0D\x0A", _DATA_FLAG_, szPrintBuf1);
                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
	}
	
	/* 空一行 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", " ");
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	if (pobTran->srBRec.inCode == _TIP_)
	{
		/* 金額 */
		/* 初始化 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "11::金額(Amount)\x0D\x0A");
                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
		
		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
		if ((pobTran->srBRec.uszVOIDBit == VS_TRUE && pobTran->srBRec.inOrgCode != _REFUND_) || 
		     pobTran->srBRec.inCode == _REFUND_)
		{
			inFunc_Amount_Comma_DCC(pobTran->srBRec.szDCC_FCA, "-", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, pobTran->srBRec.szDCC_FCMU, pobTran->srBRec.szDCC_FCAC, szPrintBuf2);
		}
		else
		{
			inFunc_Amount_Comma_DCC(pobTran->srBRec.szDCC_FCA, "", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, pobTran->srBRec.szDCC_FCMU, pobTran->srBRec.szDCC_FCAC, szPrintBuf2);
		}

		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szPrintBuf1, "%s", szPrintBuf2);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "20::\x20%s%s\x0D\x0A", _DATA_FLAG_, szPrintBuf1);
                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
		
		/* 小費 */
		/* 初始化 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "11::小費(Tips)\x0D\x0A");
                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
		
		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
		if ((pobTran->srBRec.uszVOIDBit == VS_TRUE && pobTran->srBRec.inOrgCode != _REFUND_) || 
		     pobTran->srBRec.inCode == _REFUND_)
		{
			inFunc_Amount_Comma_DCC(pobTran->srBRec.szDCC_TIPFCA, "-", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, pobTran->srBRec.szDCC_FCMU, pobTran->srBRec.szDCC_FCAC, szPrintBuf2);
		}
		else
		{
			inFunc_Amount_Comma_DCC(pobTran->srBRec.szDCC_TIPFCA, "", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, pobTran->srBRec.szDCC_FCMU, pobTran->srBRec.szDCC_FCAC, szPrintBuf2);
		}

		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szPrintBuf1, "%s", szPrintBuf2);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "20::\x20%s%s\x0D\x0A", _DATA_FLAG_, szPrintBuf1);
                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
		
		/* 總計 */
		/* 初始化 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "20::總計(Total)%s%s\x0D\x0A", _DATA_FLAG_, "\x20");
                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "11::%s\x0D\x0A", "Total amount of Transaction Currency");
                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
		
		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
		/* 初始化 */
		lnTipTotalAmt = 0;
		memset(szTemplate, 0x00, sizeof(szTemplate));
		lnTipTotalAmt += atol(pobTran->srBRec.szDCC_FCA);
		lnTipTotalAmt += atol(pobTran->srBRec.szDCC_TIPFCA);
		sprintf(szTemplate, "%ld", lnTipTotalAmt);
		
		if ((pobTran->srBRec.uszVOIDBit == VS_TRUE && pobTran->srBRec.inOrgCode != _REFUND_) || 
		     pobTran->srBRec.inCode == _REFUND_)
		{
			inFunc_Amount_Comma_DCC(szTemplate, "-", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, pobTran->srBRec.szDCC_FCMU, pobTran->srBRec.szDCC_FCAC, szPrintBuf2);
		}
		else
		{
			inFunc_Amount_Comma_DCC(szTemplate, "", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, pobTran->srBRec.szDCC_FCMU, pobTran->srBRec.szDCC_FCAC, szPrintBuf2);
		}

		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szPrintBuf1, "%s", szPrintBuf2);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "20::\x20%s%s\x0D\x0A", _DATA_FLAG_, szPrintBuf1);
                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
	}
	else
	{
		/* 總計 */
		/* 初始化 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "20::總計(Total)%s%s\x0D\x0A", _DATA_FLAG_, "\x20");
                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "11::%s\x0D\x0A", "Total amount of Transaction Currency");
                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
		
		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
		if ((pobTran->srBRec.uszVOIDBit == VS_TRUE && pobTran->srBRec.inOrgCode != _REFUND_) || 
		     pobTran->srBRec.inCode == _REFUND_)
		{
			inFunc_Amount_Comma_DCC(pobTran->srBRec.szDCC_FCA, "-", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, pobTran->srBRec.szDCC_FCMU, pobTran->srBRec.szDCC_FCAC, szPrintBuf2);
		}
		else
		{
			inFunc_Amount_Comma_DCC(pobTran->srBRec.szDCC_FCA, "", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, pobTran->srBRec.szDCC_FCMU, pobTran->srBRec.szDCC_FCAC, szPrintBuf2);
		}
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szPrintBuf1, "%s", szPrintBuf2);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "20::\x20%s%s\x0D\x0A", _DATA_FLAG_, szPrintBuf1);
		memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
	}
	/* 空一行 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", " ");
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	/* 列印換算匯率比【一】 */
	/* Inverted Rate Display Unit */
	inIRDU = atoi(pobTran->srBRec.szDCC_IRDU); 
	memset(szIRDU, 0x00, sizeof(szIRDU));
	strcpy(szIRDU, "1");
	inFunc_PAD_ASCII(szIRDU, szIRDU, '0', (inIRDU + 1), _PADDING_RIGHT_);
	/* 列印換算匯率比【二】 */
	inFunc_Amount_Comma_DCC(pobTran->srBRec.szDCC_IRV, "", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, pobTran->srBRec.szDCC_IRMU, "", szExchangeAmt);
	/* 匯率 */
	/* Ex:1 USD = 30.0000 NTD */
	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	sprintf(szPrintBuf1, "Exchange Rate:");
	memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
	sprintf(szPrintBuf2, "%s %s = %s NTD", szIRDU, pobTran->srBRec.szDCC_FCAC, szExchangeAmt);
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s%s\x0D\x0A", szPrintBuf1, szPrintBuf2);
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	/* 轉換費率 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	/* XX.XX % */
	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	inFunc_Currency_Conversion_Fee(pobTran->srBRec.szDCC_MPV, pobTran->srBRec.szDCC_MPDP, szPrintBuf1);
	strcat(szPrintBuf1, " %");
	
	sprintf(szPrintBuf, "29::%s%s%s\x0D\x0A", "轉換費率(Currency Conversion Fee)", _DATA_FLAG_, szPrintBuf1);
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	/* (Currency Conversion Fee on the */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", "(Currency Conversion Fee on the");
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* exchange rate over a wholesale rate.) */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", "exchange rate over a wholesale rate.)");
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	/* 斷行 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        strcpy(szPrintBuf, "L::1\x0D\x0A");
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	return (inDataLen);
}

/*
Function        :inNCCC_ESC_PRINT_DCC_Amount_CHANGE_NTD_Small
Date&Time       :2023/3/14 下午 3:46
Describe        :DCC 一段式 轉台幣
*/
int inNCCC_ESC_PRINT_DCC_Amount_CHANGE_NTD_Small(TRANSACTION_OBJECT *pobTran, char *szPackData)
{
	int	inDataLen = 0;
	int	inIRDU = 0;
	char	szPrintBuf[200 + 1] = {0}, szPrintBuf1[50 + 1] = {0}, szPrintBuf2[50 + 1] = {0};
	char	szIRDU[12 + 1] = {0};			/* Inverted Rate Display Unit */
	char	szExchangeAmt[50 + 1] = {0};		/* 轉換金額 */
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "12::%s\x0D\x0A", " ");
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "12::%s\x0D\x0A", "Please select");
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "12::%s\x0D\x0A", "Transaction currency as below");
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 外幣     1.  [ X ]  EUR 8.18 上需空一行 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "12::%s\x0D\x0A", " ");
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	/* 外幣 */
	memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
	inFunc_Amount_Comma_DCC(pobTran->srBRec.szDCC_FCA, "", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, pobTran->srBRec.szDCC_FCMU, "", szPrintBuf2);

	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	sprintf(szPrintBuf1, "1.【 】%s %s", pobTran->srBRec.szDCC_FCAC, szPrintBuf2);						/* Foreign currcncy Alphabetic Code */
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", szPrintBuf1);
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 匯率 */
	/* Inverted Rate Display Unit */
	inIRDU = atoi(pobTran->srBRec.szDCC_IRDU); 
	memset(szIRDU, 0x00, sizeof(szIRDU));
	strcpy(szIRDU, "1");
	inFunc_PAD_ASCII(szIRDU, szIRDU, '0', (inIRDU + 1), _PADDING_RIGHT_);
	
	memset(szExchangeAmt, 0x00, sizeof(szExchangeAmt));
	inFunc_Amount_Comma_DCC(pobTran->srBRec.szDCC_IRV, "", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, pobTran->srBRec.szDCC_IRMU, "", szExchangeAmt);
	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	sprintf(szPrintBuf1, "Exchange Rate:");
	memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
	sprintf(szPrintBuf2, "%s %s = %s NTD", szIRDU, pobTran->srBRec.szDCC_FCAC, szExchangeAmt);
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s%s\x0D\x0A", szPrintBuf1, szPrintBuf2);
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	/* 轉換費率 XX.XX %*/
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	inFunc_Currency_Conversion_Fee(pobTran->srBRec.szDCC_MPV, pobTran->srBRec.szDCC_MPDP, szPrintBuf1);
	strcat(szPrintBuf1, " %");
	
        sprintf(szPrintBuf, "29::%s%s%s\x0D\x0A", "轉換費率(Currency Conversion Fee)", _DATA_FLAG_, szPrintBuf1);
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	/* (Currency Conversion Fee on the */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", "(Currency Conversion Fee on the");
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* exchange rate over a wholesale rate.) */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", "exchange rate over a wholesale rate.)");
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 台幣     2.  [    ]  NTD 264  上需空一行 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "12::%s\x0D\x0A", " ");
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	/* 台幣 */
	memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
	sprintf(szPrintBuf2, "%ld", pobTran->srBRec.lnTxnAmount);
	inFunc_Amount_Comma_DCC(szPrintBuf2, "", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, "0", "", szPrintBuf2);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf1, "2.【X】%s %s", "NTD", szPrintBuf2);									/* Foreign currcncy Alphabetic Code */

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", szPrintBuf1);
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	/* 有開小費要多印 */
	memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
	inGetTransFunc(szPrintBuf2);
	/* 檢查是否有開小費 */
	/* 預先授權完成必定不印小費 */
	if (szPrintBuf2[6] == 'Y'	&&
	    (pobTran->srBRec.inCode == _SALE_ && pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE))
	{
		/* 小費     小費（Tips）： EUR  _______________ 上需空一行 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "12::%s\x0D\x0A", " ");
		memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	        sprintf(szPrintBuf, "20::小費(Tips)%s%s\x0D\x0A", _DATA_FLAG_, "____________");
                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "20::總計(Total)%s%s\x0D\x0A", _DATA_FLAG_, "____________");
                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
	}
	
	return (inDataLen);
}

/*
Function        :inNCCC_ESC_PRINT_FISC_Data_Small
Date&Time       :2023/3/14 下午 4:29
Describe        :
*/
int inNCCC_ESC_PRINT_FISC_Data_Small(TRANSACTION_OBJECT *pobTran, char *szPackData)
{
	int	i = 0;
	int	inDataLen = 0;
	int	inTempLen = 0;
	char	szTemplate[100 + 1] = {0};
	char 	szPrintBuf[200 + 1] = {0}, szPrintBuf1[42 + 1] = {0}, szPrintBuf2[42 + 1] = {0}, szTemplate1[42 + 1] = {0}, szTemplate2[42 + 1] = {0};
	char	szProductCodeEnable[1 + 1] = {0};
	char	szStore_Stub_CardNo_Truncate_Enable[2 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
        memset(szTemplate1, 0x00, sizeof(szTemplate1));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	
	/* "卡號 卡別" 卡別值 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	strcpy(szTemplate, pobTran->srBRec.szCardLabel);
	
	inTempLen =  ((20 - strlen(szTemplate)) * _ESC_SMALL_CARD_TYPE_MULTIPLE_) + _ESC_SMALL_CARD_TYPE_OFFSET_;
	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	for (i = 0; i < inTempLen; i++)
	{
		strcat(szTemplate2, " ");
	}
	strcat(szTemplate2, szTemplate);
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	strcat(szPrintBuf, "29::");
	strcat(szPrintBuf, "卡號");
	strcat(szPrintBuf, "\x20");
	strcat(szPrintBuf, _DATA_FLAG_);
	strcat(szPrintBuf, "卡別");
	strcat(szPrintBuf, "\x20");
	strcat(szPrintBuf, szTemplate2);
	strcat(szPrintBuf, "\x0D\x0A");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	/* 卡號值 */
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	strcpy(szTemplate1, pobTran->srBRec.szPAN);
	
	/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
	/* 商店聯卡號遮掩 */
	memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
	inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
	/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/8 下午 1:55 */
	if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
	{
		/* Uny交易不受TMS開關限制，且獲取卡號時已遮掩 */
	}
	else if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0 && pobTran->srBRec.uszTxNoCheckBit == VS_TRUE)
	{
		/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
		for (i = 6; i < (strlen(szTemplate1) - 4); i ++)
			szTemplate1[i] = 0x2A;
	}

	/* 過卡方式 */
	if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
	{
		if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
			strcat(szTemplate1, "(W)");
		else
			strcat(szTemplate1, "(C)");
	}
	else if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
	{
		strcat(szTemplate1, "(Q)");
	}
	else if (pobTran->srBRec.uszCUPEMVQRCodeBit == VS_TRUE)
	{
		strcat(szTemplate, "(Q)");
	}
	else
	{
		if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
			strcat(szTemplate1,"(C)");
		else if (pobTran->srBRec.uszMobilePayBit == VS_TRUE)
			strcat(szTemplate1, "(T)");
		else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
			strcat(szTemplate1, "(W)");
		else
		{
			if (pobTran->srBRec.uszManualBit == VS_TRUE)
			{
				/* 【需求單 - 105244】端末設備支援以感應方式進行退貨交易 */
				/* 電文轉Manual Keyin但是簽單要印感應的W */
				if (pobTran->srBRec.uszRefundCTLSBit == VS_TRUE)
					strcat(szTemplate1, "(W)");
				else
					strcat(szTemplate1,"(M)");
			}
			else
				strcat(szTemplate1,"(S)");
		}
	}
       	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", szTemplate1);
        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	/* 交易類別 */
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s %s\x0D\x0A", "交易", szTemplate1);
        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	if (strlen(szTemplate2) > 0)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "11::%s\x0D\x0A", szTemplate2);
		memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
	}
	
	/* 發卡行代碼 主機 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	/* 發卡行代碼 */
	/* 發卡行代碼值 */
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	memcpy(szTemplate1, &pobTran->srBRec.szFiscIssuerID[0], 3);
	/* 【需求單 - 106349】自有品牌判斷需求 */
	/* unpack電文就存存CardLabel modify by LingHsiung 2020/2/14 上午 11:07 */
	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	inGetHostLabel(szTemplate2);
	
	strcat(szPrintBuf, "40::");
	strcat(szPrintBuf, "發卡行代碼");
	strcat(szPrintBuf, _DATA_FLAG_);
	strcat(szPrintBuf, szTemplate1);
	strcat(szPrintBuf, _DATA_FLAG_);
	strcat(szPrintBuf, "主機");
	strcat(szPrintBuf, _DATA_FLAG_);
	strcat(szPrintBuf, szTemplate2);
	strcat(szPrintBuf, "\x0D\x0A");
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	/* 日期時間 批號 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	/* 日期時間 */
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	sprintf(szTemplate1, "%.4s/%.2s/%.2s %.2s:%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6], &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
	/* 批號值 */
	sprintf(szTemplate2, "%03ld", pobTran->srBRec.lnBatchNum);
	inFunc_PAD_ASCII(szTemplate2, szTemplate2, ' ', 8, _PADDING_LEFT_);
	
	strcat(szPrintBuf, "40::");
	strcat(szPrintBuf, "日期/時間");
	strcat(szPrintBuf, _DATA_FLAG_);
	strcat(szPrintBuf, szTemplate1);
	strcat(szPrintBuf, _DATA_FLAG_);
	strcat(szPrintBuf, "批號");
	strcat(szPrintBuf, _DATA_FLAG_);
	strcat(szPrintBuf, szTemplate2);
	strcat(szPrintBuf, "\x0D\x0A");
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 序號 調閱號 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	/* 序號 */
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	/* 雖然電文RRN送12個byte，但RRN最後一碼是0x00，所以只看到11碼 */
	inFunc_PAD_ASCII(szTemplate1, pobTran->srBRec.szRefNo, ' ', 12, _PADDING_RIGHT_);
	/* 調閱號值 */
	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	sprintf(szTemplate2, "%06ld", pobTran->srBRec.lnOrgInvNum);
	
	strcat(szPrintBuf, "40::");
	strcat(szPrintBuf, "序號");
	strcat(szPrintBuf, _DATA_FLAG_);
	strcat(szPrintBuf, szTemplate1);
	strcat(szPrintBuf, _DATA_FLAG_);
	strcat(szPrintBuf, "調閱號");
	strcat(szPrintBuf, _DATA_FLAG_);
	strcat(szPrintBuf, szTemplate2);
	strcat(szPrintBuf, "\x0D\x0A");
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	/* 調單編號 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	/* 調單編號值 */
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	memcpy(szTemplate1, pobTran->srBRec.szFiscRRN, _FISC_RRN_SIZE_);
	strcat(szPrintBuf, "40::");
	strcat(szPrintBuf, "調單編號");
	strcat(szPrintBuf, _DATA_FLAG_);
	strcat(szPrintBuf, pobTran->srBRec.szFiscRRN);
	strcat(szPrintBuf, _DATA_FLAG_);
	strcat(szPrintBuf, "\x20");
	strcat(szPrintBuf, _DATA_FLAG_);
	strcat(szPrintBuf, "\x20");
	strcat(szPrintBuf, "\x0D\x0A");
        memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 櫃號 */
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	inGetStoreIDEnable(szTemplate1);
	if ((memcmp(&szTemplate1[0], "Y", 1) == 0) && (strlen(pobTran->srBRec.szStoreID) > 0))
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memcpy(szTemplate1, &pobTran->srBRec.szStoreID[0], 23);
		
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_026_TAKA_, _CUSTOMER_INDICATOR_SIZE_) ||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
		{
			strcat(szPrintBuf, "40::");
			strcat(szPrintBuf, "品群碼");
			strcat(szPrintBuf, _DATA_FLAG_);
			strcat(szPrintBuf, szTemplate1);
			strcat(szPrintBuf, _DATA_FLAG_);
			strcat(szPrintBuf, "\x20");
			strcat(szPrintBuf, _DATA_FLAG_);
			strcat(szPrintBuf, "\x20");
			strcat(szPrintBuf, "\x0D\x0A");
		}
		else
		{
			strcat(szPrintBuf, "40::");
			strcat(szPrintBuf, "櫃號");
			strcat(szPrintBuf, _DATA_FLAG_);
			strcat(szPrintBuf, szTemplate1);
			strcat(szPrintBuf, _DATA_FLAG_);
			strcat(szPrintBuf, "\x20");
			strcat(szPrintBuf, _DATA_FLAG_);
			strcat(szPrintBuf, "\x20");
			strcat(szPrintBuf, "\x0D\x0A");
		}
		memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);

		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			/* hardcode */
			char	szPOS_ID[6 + 1] = {0};
			memset(szPOS_ID, 0x00, sizeof(szPOS_ID));
			inGetPOS_ID(szPOS_ID);

			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			memcpy(szTemplate1, _HARDCODE_STORE_ID_CUS_021_TAKAWEL_, 12);
			memcpy(&szTemplate1[12], szPOS_ID, 6);
			
			memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
		}
	}

	/* 商店聯卡號遮掩 */
	memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
	inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
	if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
	    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
	    strlen(pobTran->srBRec.szTxnNo) > 0)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "11::%s %s\x0D\x0A", "交易編號", pobTran->srBRec.szTxnNo);
		memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
	}
	
	/* 產品代碼 */
	inGetProductCodeEnable(szProductCodeEnable);
	if (memcmp(szProductCodeEnable, "Y", 1) == 0)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "11::%s\x0D\x0A", "產品代碼(Product Code)");
                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "11::%s\x0D\x0A", pobTran->srBRec.szProductCode);
                memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
	}

	return (inDataLen);
}

/*
Function        :inNCCC_ESC_PRINT_FISC_Amount_Small
Date&Time       :2023/3/15 上午 10:13
Describe        :
 */
int inNCCC_ESC_PRINT_FISC_Amount_Small(TRANSACTION_OBJECT *pobTran, char *szPackData)
{
	int	inDataLen = 0;
        char    szPrintBuf[84 + 1], szTemplate[42 + 1];

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));
	
	/* 橫式 */
        /* 負向交易 */
	if(pobTran->srBRec.uszVOIDBit == VS_TRUE)
	{
		/* 橫式 */
                /* 金額 */
                /* 取消退貨是正數 */
                if (pobTran->srBRec.inOrgCode == _FISC_REFUND_)
                {
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
			sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
			inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, _PADDING_LEFT_);

                        /* 把前面的字串和數字結合起來 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "22::總計(Total)%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);
			memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
                }
		else
		{
			/* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
			sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
			inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, _PADDING_LEFT_);

                        /* 把前面的字串和數字結合起來 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "22::總計(Total)%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);
			memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
		}
	}
	else
	{
		
		/* 初始化 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

		/* 將NT$ ＋數字塞到szTemplate中來inpad，退貨要負數 */
		if (pobTran->srBRec.inCode == _FISC_REFUND_)
		{
			sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
		}
		else
		{
			sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
		}

		inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, _PADDING_LEFT_);

		/* 把前面的字串和數字結合起來 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "22::總計(Total)%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);
		memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
		
	}
	
	return (inDataLen);
}

/*
Function        :inNCCC_ESC_PRINT_DATA_Small_S
Date&Time       :2023/3/23 下午 6:31
Describe        :組E1 DATA部份
*/
int inNCCC_ESC_PRINT_DATA_Small_S(TRANSACTION_OBJECT *pobTran, char *szPackData)
{
	int	i = 0;
	int	inDataLen = 0;
	int	inTempLen = 0;
	int	inUpperAmount = 0;
	int	inLowerAmount = 0;
	char	szPrintBuf[200 + 1] = {0}, szPrintBuf1[100 + 1] = {0}, szPrintBuf2[100 + 1] = {0}, szTemplate[100 + 1] = {0}, szTemplate1[100 + 1] = {0}, szTemplate2[100 + 1] = {0};
	char	szProductCodeEnable[1 + 1];
	char	szStore_Stub_CardNo_Truncate_Enable[2 + 1];
	char	szExamBit[10 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);

	/* "卡號 卡別" 卡別值 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	strcpy(szTemplate, pobTran->srBRec.szCardLabel);
	
	inTempLen =  ((20 - strlen(szTemplate)) * _ESC_SMALL_CARD_TYPE_MULTIPLE_) + _ESC_SMALL_CARD_TYPE_OFFSET_;
	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	for (i = 0; i < inTempLen; i++)
	{
		strcat(szTemplate2, " ");
	}
	strcat(szTemplate2, szTemplate);
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	strcat(szPrintBuf, "29::");
	strcat(szPrintBuf, "卡號");
	strcat(szPrintBuf, "\x20");
	strcat(szPrintBuf, _DATA_FLAG_);
	strcat(szPrintBuf, "卡別");
	strcat(szPrintBuf, "\x20");
	strcat(szPrintBuf, szTemplate2);
	strcat(szPrintBuf, "\x0D\x0A");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	/* 卡號值 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	strcpy(szTemplate, pobTran->srBRec.szPAN);
	
	/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
	/* 商店聯卡號遮掩 */
	memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
	inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
	/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/8 下午 1:55 */
	if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
	{
		/* Uny交易不受TMS開關限制，且獲取卡號時已遮掩 */
	}
	else if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0 && pobTran->srBRec.uszTxNoCheckBit == VS_TRUE)
	{
		/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
		for (i = 6; i < (strlen(szTemplate) - 4); i ++)
			szTemplate[i] = 0x2A;
	}

	/* 過卡方式 */
	if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
	{
		if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
			strcat(szTemplate, "(W)");
		else
			strcat(szTemplate, "(C)");
	}
	else if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
	{
		strcat(szTemplate, "(Q)");
	}
	else if (pobTran->srBRec.uszCUPEMVQRCodeBit == VS_TRUE)
	{
		strcat(szTemplate, "(Q)");
	}
	else
	{
		if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
			strcat(szTemplate,"(C)");
		else if (pobTran->srBRec.uszMobilePayBit == VS_TRUE)
			strcat(szTemplate, "(T)");
		else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
			strcat(szTemplate, "(W)");
		else
		{
			if (pobTran->srBRec.uszManualBit == VS_TRUE)
			{
				/* 【需求單 - 105244】端末設備支援以感應方式進行退貨交易 */
				/* 電文轉Manual Keyin但是簽單要印感應的W */
				if (pobTran->srBRec.uszRefundCTLSBit == VS_TRUE)
					strcat(szTemplate, "(W)");
				else
					strcat(szTemplate,"(M)");
			}
			else
				strcat(szTemplate,"(S)");
		}
	}
       	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", szTemplate);

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	/* 交易類別 */
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s %s\x0D\x0A", "交易", szTemplate1);

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	if (strlen(szTemplate2) > 0)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "11::%s\x0D\x0A", szTemplate2);

		memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
	}
	
	/* 城市 主機 */
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	inGetCityName(szTemplate1);
	
	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	inGetHostLabel(szTemplate2);
	inFunc_DiscardSpace(szTemplate2);
	inFunc_PAD_ASCII(szTemplate2, szTemplate2, ' ', 8, _PADDING_LEFT_);
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	strcat(szPrintBuf, "40::");
	strcat(szPrintBuf, "城市");
	strcat(szPrintBuf, _DATA_FLAG_);
	strcat(szPrintBuf, szTemplate1);
	strcat(szPrintBuf, _DATA_FLAG_);
	strcat(szPrintBuf, "主機");
	strcat(szPrintBuf, _DATA_FLAG_);
	strcat(szPrintBuf, szTemplate2);
	strcat(szPrintBuf, "\x0D\x0A");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	/* "日期/時間" 日期值 "批號" 批號值 */
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	sprintf(szTemplate1, "%.4s/%.2s/%.2s %.2s:%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6], &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
	
	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	sprintf(szTemplate2, "%03ld", pobTran->srBRec.lnBatchNum);
	inFunc_PAD_ASCII(szTemplate2, szTemplate2, ' ', 8, _PADDING_LEFT_);
		
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	strcat(szPrintBuf, "40::");
	strcat(szPrintBuf, "日期/時間");
	strcat(szPrintBuf, _DATA_FLAG_);
	strcat(szPrintBuf, szTemplate1);
	strcat(szPrintBuf, _DATA_FLAG_);
	strcat(szPrintBuf, "批號");
	strcat(szPrintBuf, _DATA_FLAG_);
	strcat(szPrintBuf, szTemplate2);
	strcat(szPrintBuf, "\x0D\x0A");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* "授權碼" 授權碼值 "檢查碼" 檢查碼值 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	memcpy(szTemplate1, &pobTran->srBRec.szAuthCode[0], 12);
	
	if (pobTran->srBRec.uszTakaTransBit == VS_TRUE)
	{
		memset(szTemplate2, 0x00, sizeof(szTemplate2));
		sprintf(szTemplate2, "%.2s/'%.2s", pobTran->srBRec.szExpDate + 2, pobTran->srBRec.szExpDate);
	}
	else
	{
		memset(szTemplate2, 0x00, sizeof(szTemplate2));
		/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/28 下午 2:42 */
		if (strlen(pobTran->srBRec.szCheckNO) > 0)
		{
			strcpy(szTemplate2, pobTran->srBRec.szCheckNO);
		}
		else
		{
			inCARD_ExpDateEncryptAndDecrypt(pobTran, "", szTemplate2, _EXP_ENCRYPT_);
		}
		inFunc_PAD_ASCII(szTemplate2, szTemplate2, ' ', 9, _PADDING_LEFT_);
	}
	
	/* 大高卡只能顯示有效期 */
	if (pobTran->srBRec.uszTakaTransBit == VS_TRUE)
	{
		strcat(szPrintBuf, "40::");
		strcat(szPrintBuf, "授權碼");
		strcat(szPrintBuf, _DATA_FLAG_);
		strcat(szPrintBuf, szTemplate1);
		strcat(szPrintBuf, _DATA_FLAG_);
		strcat(szPrintBuf, "有效期");
		strcat(szPrintBuf, _DATA_FLAG_);
		strcat(szPrintBuf, szTemplate2);
		strcat(szPrintBuf, "\x0D\x0A");
	}
	else
	{
		strcat(szPrintBuf, "40::");
		strcat(szPrintBuf, "授權碼");
		strcat(szPrintBuf, _DATA_FLAG_);
		strcat(szPrintBuf, szTemplate1);
		strcat(szPrintBuf, _DATA_FLAG_);
		strcat(szPrintBuf, "檢查碼");
		strcat(szPrintBuf, _DATA_FLAG_);
		strcat(szPrintBuf, szTemplate2);
		strcat(szPrintBuf, "\x0D\x0A");
	}

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* "序號" 序號值 "調閱號" 調閱編號值 */
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	inFunc_PAD_ASCII(szTemplate1, pobTran->srBRec.szRefNo, ' ', 25, _PADDING_RIGHT_);

	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	sprintf(szTemplate2, "%06ld", pobTran->srBRec.lnOrgInvNum);
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	strcat(szPrintBuf, "40::");
	strcat(szPrintBuf, "序號");
	strcat(szPrintBuf, _DATA_FLAG_);
	strcat(szPrintBuf, szTemplate1);
	strcat(szPrintBuf, _DATA_FLAG_);
	strcat(szPrintBuf, "調閱號");
	strcat(szPrintBuf, _DATA_FLAG_);
	strcat(szPrintBuf, szTemplate2);
	strcat(szPrintBuf, "\x0D\x0A");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 櫃號 */
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	inGetStoreIDEnable(szTemplate1);
	if ((memcmp(&szTemplate1[0], "Y", 1) == 0) && (strlen(pobTran->srBRec.szStoreID) > 0))
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memcpy(szTemplate1, &pobTran->srBRec.szStoreID[0], 23);
		
		memset(szTemplate2, 0x00, sizeof(szTemplate2));
		sprintf(szTemplate2, "%s", pobTran->srBRec.szRespCode);
		inFunc_PAD_ASCII(szTemplate2, szTemplate2, ' ', 9, _PADDING_LEFT_);
		
		if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
		{
			strcat(szPrintBuf, "40::");
			strcat(szPrintBuf, "櫃號");
			strcat(szPrintBuf, _DATA_FLAG_);
			strcat(szPrintBuf, szTemplate1);
			strcat(szPrintBuf, _DATA_FLAG_);
			strcat(szPrintBuf, "回覆碼");
			strcat(szPrintBuf, _DATA_FLAG_);
			strcat(szPrintBuf, szTemplate2);
			strcat(szPrintBuf, "\x0D\x0A");
		}
		else
		{
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_026_TAKA_, _CUSTOMER_INDICATOR_SIZE_) ||
			    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
			{
				strcat(szPrintBuf, "40::");
				strcat(szPrintBuf, "品群碼");
				strcat(szPrintBuf, _DATA_FLAG_);
				strcat(szPrintBuf, szTemplate1);
				strcat(szPrintBuf, _DATA_FLAG_);
				strcat(szPrintBuf, "\x20");
				strcat(szPrintBuf, _DATA_FLAG_);
				strcat(szPrintBuf, "\x20");
				strcat(szPrintBuf, "\x0D\x0A");
			}
			else
			{
				strcat(szPrintBuf, "40::");
				strcat(szPrintBuf, "櫃號");
				strcat(szPrintBuf, _DATA_FLAG_);
				strcat(szPrintBuf, szTemplate1);
				strcat(szPrintBuf, _DATA_FLAG_);
				strcat(szPrintBuf, "\x20");
				strcat(szPrintBuf, _DATA_FLAG_);
				strcat(szPrintBuf, "\x20");
				strcat(szPrintBuf, "\x0D\x0A");
			}	
		}
		
		memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
		
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			/* hardcode */
			char	szPOS_ID[6 + 1] = {0};
			memset(szPOS_ID, 0x00, sizeof(szPOS_ID));
			inGetPOS_ID(szPOS_ID);

			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			memcpy(szTemplate1, _HARDCODE_STORE_ID_CUS_021_TAKAWEL_, 12);
			memcpy(&szTemplate1[12], szPOS_ID, 6);
			
			memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
		}
	}
	else
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memcpy(szTemplate1, &pobTran->srBRec.szStoreID[0], 23);
		
		memset(szTemplate2, 0x00, sizeof(szTemplate2));
		sprintf(szTemplate2, "%s", pobTran->srBRec.szRespCode);
		inFunc_PAD_ASCII(szTemplate2, szTemplate2, ' ', 9, _PADDING_LEFT_);
		
		if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
		{
			strcat(szPrintBuf, "40::");
			strcat(szPrintBuf, "\x20");
			strcat(szPrintBuf, _DATA_FLAG_);
			strcat(szPrintBuf, "\x20");
			strcat(szPrintBuf, _DATA_FLAG_);
			strcat(szPrintBuf, "回覆碼");
			strcat(szPrintBuf, _DATA_FLAG_);
			strcat(szPrintBuf, szTemplate2);
			strcat(szPrintBuf, "\x0D\x0A");
			
			memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
		}
	}
	
	/* 產品代碼 */
	inGetProductCodeEnable(szProductCodeEnable);
	if (memcmp(szProductCodeEnable, "Y", 1) == 0)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		strcat(szPrintBuf, "40::");
		strcat(szPrintBuf, "產品代碼");
		strcat(szPrintBuf, _DATA_FLAG_);
		strcat(szPrintBuf, pobTran->srBRec.szProductCode);
		strcat(szPrintBuf, _DATA_FLAG_);
		strcat(szPrintBuf, "\x20");
		strcat(szPrintBuf, _DATA_FLAG_);
		strcat(szPrintBuf, "\x20");
		strcat(szPrintBuf, "\x0D\x0A");

                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
	}
	
        /* 【需求單-113260】MasterCard機票交易需求 by Russell 2025/3/3 上午 11:55 */
        /* 取消維持原交易不列印機票資訊 */
        if (inNCCC_Func_Flight_Ticket_Allow_Print(pobTran) == VS_TRUE)
        {
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "41::%s%s%s%s%s%s%s\x0D\x0A", "出發地機場", _DATA_FLAG_, pobTran->srBRec.szFlightTicketPDS0523, _DATA_FLAG_, "目的地機場", _DATA_FLAG_, pobTran->srBRec.szFlightTicketPDS0524);

                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);

                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                inNCCC_Func_Get_Flight_Ticket_Txn_Category_PrintName(pobTran, szTemplate1);
                memset(szTemplate2, 0x00, sizeof(szTemplate2));
                if (strlen(pobTran->srBRec.szFlightTicketPDS0530) > 0)
                {
                        sprintf(szTemplate2, "%d", atoi(pobTran->srBRec.szFlightTicketPDS0530));
                }
                else
                {
                        sprintf(szTemplate2, "%s", " ");
                }
                sprintf(szPrintBuf, "%s", szTemplate2);
                
		if (!memcmp(szTemplate1, _FLIGHT_TICKET_TXN_CATEGORY_PRINT_BAGGAGE_CHARGE_, max(strlen(szTemplate1), strlen(_FLIGHT_TICKET_TXN_CATEGORY_PRINT_BAGGAGE_CHARGE_)))		||
		    !memcmp(szTemplate1, _FLIGHT_TICKET_TXN_CATEGORY_PRINT_PREPAID_TICKET_, max(strlen(szTemplate1), strlen(_FLIGHT_TICKET_TXN_CATEGORY_PRINT_PREPAID_TICKET_)))		||
		    !memcmp(szTemplate1, _FLIGHT_TICKET_TXN_CATEGORY_PRINT_MISCELLANEOUS_CHARGE_ , max(strlen(szTemplate1), strlen(_FLIGHT_TICKET_TXN_CATEGORY_PRINT_MISCELLANEOUS_CHARGE_)))	||
		    !memcmp(szTemplate1, _FLIGHT_TICKET_TXN_CATEGORY_PRINT_TICKET_REISSUE_, max(strlen(szTemplate1), strlen(_FLIGHT_TICKET_TXN_CATEGORY_PRINT_TICKET_REISSUE_))))
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "41::%s%s%s%s%s%s%s\x0D\x0A", "航班號碼", _DATA_FLAG_, szTemplate2, _DATA_FLAG_, "交易類型", _DATA_FLAG_, " ");

			memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);

			/* 根據業務要求交易類型換行靠右 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "24::%s%s%s\x0D\x0A", " ", _DATA_FLAG_, szTemplate1);

			memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
		}
		else
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "41::%s%s%s%s%s%s%s\x0D\x0A", "航班號碼", _DATA_FLAG_, szTemplate2, _DATA_FLAG_, "交易類型", _DATA_FLAG_, szTemplate1);

			memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
		}
        }
            
        
	/* TC */
	/* 驗測開關 */
	memset(szExamBit, 0x00, sizeof(szExamBit));
	inGetExamBit(szExamBit);
	/* Uny交易 */
	/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/8 下午 1:55 */
	if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
	{
		/* 取消或退貨，只印數字交易碼 */
		/* 最小字體 */
		if (pobTran->srBRec.uszVOIDBit == VS_TRUE ||
		    pobTran->srBRec.inCode == _REFUND_ || pobTran->srBRec.inCode == _INST_REFUND_ || pobTran->srBRec.inCode == _REDEEM_REFUND_)
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			sprintf(szTemplate1, "交易碼 %s", pobTran->srBRec.szUnyTransCode);
			sprintf(szPrintBuf, "11::%s\x0D\x0A", szTemplate1);

			memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
		}
		/* 正向交易，要印條碼和數字 */
		else
		{

			if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
			{
				/* 一維條碼 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%s", pobTran->srBRec.szUnyTransCode);
				sprintf(szPrintBuf, "U::%s\x0D\x0A", szTemplate1);
				
				memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);

				/* 數字 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "交易碼(退貨請掃描) %s", pobTran->srBRec.szUnyTransCode);
				sprintf(szPrintBuf, "11::%s\x0D\x0A", szTemplate1);

				memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);
			}
			else
			{
				/* 一維條碼 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%s", pobTran->srBRec.szUnyTransCode);
				sprintf(szPrintBuf, "U::%s\x0D\x0A", szTemplate1);

				memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);

				/* 數字 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "交易碼 %s", pobTran->srBRec.szUnyTransCode);
				sprintf(szPrintBuf, "11::%s\x0D\x0A", szTemplate1);

				memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);
			}
		}
	}
	/* 實體卡交易和掃碼交易互斥 */
	else
	{
		if (pobTran->srBRec.inChipStatus == _EMV_CARD_ || pobTran->srBRec.uszContactlessBit == VS_TRUE)
		{
			/* 感應磁條 */
			if (!memcmp(pobTran->srBRec.szAuthCode, "VLP", 3)				|| 
			    !memcmp(pobTran->srBRec.szAuthCode, "JCB", 3)				||
			     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE		|| 
			     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_64_NEWJSPEEDY_MSD		||
			     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE)
			{
				/* 商店聯卡號遮掩 */
				memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
				inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
				if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
				    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
				    strlen(pobTran->srBRec.szTxnNo) > 0)
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "11::%s %s\x0D\x0A", "交易編號", pobTran->srBRec.szTxnNo);
					memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
					inDataLen += strlen(szPrintBuf);
				}
			}
			else
			{
				if (pobTran->srBRec.inPrintOption == _PRT_MERCH_)
				{
					if (pobTran->srEMVRec.in9F26_ApplCryptogramLen > 0)
					{
						memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
						sprintf(szPrintBuf1, "%02X%02X%02X%02X%02X%02X%02X%02X",
										pobTran->srEMVRec.usz9F26_ApplCryptogram[0],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[1],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[2],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[3],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[4],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[5],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[6],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[7]);

						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "23::TC%s%s\x0D\x0A", _DATA_FLAG_, szPrintBuf1);

						memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
						inDataLen += strlen(szPrintBuf);
					}

					/* 商店聯卡號遮掩 */
					memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
					inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
					if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
					    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
					    strlen(pobTran->srBRec.szTxnNo) > 0)
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "11::%s %s\x0D\x0A", "交易編號", pobTran->srBRec.szTxnNo);
						memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
						inDataLen += strlen(szPrintBuf);
					}

					/* M/C交易列印AP Lable (START) */
					if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)))
					{
						if (pobTran->srEMVRec.in50_APLabelLen > 0)
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							strcat(szPrintBuf, "AP Label:");
							memset(szTemplate, 0x00, sizeof(szTemplate));
							strcpy(szTemplate, (char*)pobTran->srEMVRec.usz50_APLabel);
							inUpperAmount = inFunc_Get_UpperCase_Char(szTemplate);
							inLowerAmount = strlen(szTemplate) - inUpperAmount;
							inTempLen =  _ESC_AP_LABEL_TOTAL_SPACE_ - (_ESC_UPPER_MULTIPLE_ * inUpperAmount + _ESC_LOWER_MULTIPLE_ * inLowerAmount);
							for (i = 0; i < inTempLen; i++)
							{
								strcat(szPrintBuf, " ");
							}
							strcat(szPrintBuf, szTemplate);
						}
						else
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							strcat(szPrintBuf, "AP Label:");
							strcat(szPrintBuf, "\x20");
						}
					}
					/* M/C交易列印AP Lable (END) */
					else if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)))
					{
						if (pobTran->srEMVRec.in50_APLabelLen > 0)
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							strcat(szPrintBuf, "AP Label:");
							memset(szTemplate, 0x00, sizeof(szTemplate));
							strcpy(szTemplate, (char*)pobTran->srEMVRec.usz50_APLabel);
							inUpperAmount = inFunc_Get_UpperCase_Char(szTemplate);
							inLowerAmount = strlen(szTemplate) - inUpperAmount;
							inTempLen =  _ESC_AP_LABEL_TOTAL_SPACE_ - (_ESC_UPPER_MULTIPLE_ * inUpperAmount + _ESC_LOWER_MULTIPLE_ * inLowerAmount);
							for (i = 0; i < inTempLen; i++)
							{
								strcat(szPrintBuf, " ");
							}
							strcat(szPrintBuf, szTemplate);
						}
						else
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							strcat(szPrintBuf, "AP Label:");
							strcat(szPrintBuf, "\x20");
						}
					}
					/* 驗測要印AP Label 和 AID */
					else if (szExamBit[0] == '1')
					{
						if (pobTran->srEMVRec.in50_APLabelLen > 0)
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							strcat(szPrintBuf, "AP Label:");
							memset(szTemplate, 0x00, sizeof(szTemplate));
							strcpy(szTemplate, (char*)pobTran->srEMVRec.usz50_APLabel);
							inUpperAmount = inFunc_Get_UpperCase_Char(szTemplate);
							inLowerAmount = strlen(szTemplate) - inUpperAmount;
							inTempLen =  _ESC_AP_LABEL_TOTAL_SPACE_ - (_ESC_UPPER_MULTIPLE_ * inUpperAmount + _ESC_LOWER_MULTIPLE_ * inLowerAmount);
							for (i = 0; i < inTempLen; i++)
							{
								strcat(szPrintBuf, " ");
							}
							strcat(szPrintBuf, szTemplate);
						}
						else
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							strcat(szPrintBuf, "AP Label:");
							strcat(szPrintBuf, "\x20");
						}
					}
					else
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						strcat(szPrintBuf, "\x20");
						strcat(szPrintBuf, "\x20");
					}
					memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
					sprintf(szPrintBuf2, szPrintBuf);
					
					/* AID */
					if (strlen(pobTran->srBRec.szCUP_EMVAID) > 0)
					{
						memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
						sprintf(szPrintBuf1, "AID:%s", pobTran->srBRec.szCUP_EMVAID);
						inFunc_PAD_ASCII(szPrintBuf1, szPrintBuf1, ' ', _ESC_AID_TOTAL_LEN_, _PADDING_RIGHT_);
						
						if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
						{
							/* CUP晶片要印 */
							if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
							else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)) == 0)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
							/* 【需求單-109121】Mastercard交易簽單列印AID需求 by Russell 2020/7/23 下午 4:23 */
							else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)) == 0)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
							/* 【需求單-111290】VISA置換Logo、DISCOVER簽單新增AID欄位、AE簽單新增AID和AP Label欄位需求 by Russell 2022/12/29 下午 5:57 */
							else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)) == 0 &&
								pobTran->srBRec.uszUCARDTransBit != VS_TRUE)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
							/* 驗測要印AP Label 和 AID */
							else if (szExamBit[0] == '1')
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
						}
						/* 銀聯閃付 */
						else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
						{
							if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
							/* AE晶片要印出AID */
							else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_50_EXPRESSSPAY || 
								 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
							/* 【需求單-109121】Mastercard交易簽單列印AID需求 by Russell 2020/7/23 下午 4:23 */
							else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE || 
								 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_21_PAYPASS_MCHIP)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
							else if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_42_DPAS	|| pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_43_DISCOVER_DPAS_MAG_STRIPE) &&
								pobTran->srBRec.uszUCARDTransBit != VS_TRUE)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
							/* 驗測要印AP Label 和 AID */
							else if (szExamBit[0] == '1')
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
						}
					}
				}
			}

		}
		else
		{
			/* 商店聯卡號遮掩 */
			/* 磁條卡列印交易編號 */
			memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
			inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
			if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
			    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
			    strlen(pobTran->srBRec.szTxnNo) > 0)
			{
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "11::%s %s\x0D\x0A", "交易編號", pobTran->srBRec.szTxnNo);
				memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);
			}
		}
	}
	
	return (inDataLen);
}

/*
Function        :inNCCC_ESC_PRINT_AMOUNT_Small_S
Date&Time       :2023/3/27 下午 5:42
Describe        :組E1 Amount部份
*/
int inNCCC_ESC_PRINT_AMOUNT_Small_S(TRANSACTION_OBJECT *pobTran, char *szPackData)
{
        int     inDataLen = 0;
	char	szPrintBuf[200 + 1], szTemplate[100 + 1];
	
	if (pobTran->srBRec.uszVOIDBit == VS_TRUE)
	{
		if (pobTran->srBRec.inOrgCode == _REFUND_	|| 
		    pobTran->srBRec.inOrgCode == _INST_REFUND_	|| 
		    pobTran->srBRec.inOrgCode == _REDEEM_REFUND_)
		{
		        /* 將NT$ ＋數字塞到szTemplate中來inpad */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
			inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, _PADDING_RIGHT_);

		        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        sprintf(szPrintBuf, "22::總計(Total)%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

                        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
                        inDataLen += strlen(szPrintBuf);
		}
		else
		{
			/* 將NT$ ＋數字塞到szTemplate中來inpad */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
			inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, _PADDING_RIGHT_);

		        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        sprintf(szPrintBuf, "22::總計(Total)%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

                        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
                        inDataLen += strlen(szPrintBuf);
		}
	}
	else
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTransFunc(szTemplate);
		if (szTemplate[6] == 'Y')	/* 檢查是否有開小費 */
		{
			/* 退貨金額為負數 */
			if (pobTran->srBRec.inCode == _REFUND_ || pobTran->srBRec.inCode == _INST_REFUND_ || pobTran->srBRec.inCode == _REDEEM_REFUND_)
			{
				 /* 初始化 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
				inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, _PADDING_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "22::總計(Total)%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

                                memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
                                inDataLen += strlen(szPrintBuf);
			}
			/* 預授不會有小費，所以拉出來 */
			else if (pobTran->srBRec.inCode == _PRE_AUTH_ || pobTran->srBRec.inCode == _PRE_COMP_)
			{
				/* 初始化 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
				inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, _PADDING_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "22::總計(Total)%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

                                memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
                                inDataLen += strlen(szPrintBuf);
			}
			else
			{
				/* 金額 */
				/* 初始化 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
				inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, _PADDING_RIGHT_);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "22::金額(Amount)%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

                                memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
                                inDataLen += strlen(szPrintBuf);

				/* lnTipAmount為0表示非小費 */
				if (pobTran->srBRec.lnTipTxnAmount == 0L)
				{
				        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				        sprintf(szPrintBuf, "22::小費(Tips)%s%s\x0D\x0A", _DATA_FLAG_, "_____________");

                                        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
                                        inDataLen += strlen(szPrintBuf);

                                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                        sprintf(szPrintBuf, "22::總計(Total)%s%s\x0D\x0A", _DATA_FLAG_, "_____________");

                                        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
                                        inDataLen += strlen(szPrintBuf);
				}
				else
				{
					/* 小費 */
					/* 初始化 */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

					/* 將NT$ ＋數字塞到szTemplate中來inpad */
					sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTipTxnAmount);
					inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, _PADDING_RIGHT_);

					/* 把前面的字串和數字結合起來 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "22::小費(Tips)%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

                                        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
                                        inDataLen += strlen(szPrintBuf);

					/* 總計 */
					/* 初始化 */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

					/* 將NT$ ＋數字塞到szTemplate中來inpad */
					sprintf(szTemplate, "%ld",  (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
					inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 15, _PADDING_RIGHT_);

					/* 把前面的字串和數字結合起來 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                        sprintf(szPrintBuf, "22::總計(Total)%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

                                        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
                                        inDataLen += strlen(szPrintBuf);
				}
			}
		}
		else
		{
			/* 初始化 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

			/* 將NT$ ＋數字塞到szTemplate中來inpad，退貨要負數 */
			if (pobTran->srBRec.inCode == _REFUND_ || pobTran->srBRec.inCode == _INST_REFUND_ || pobTran->srBRec.inCode == _REDEEM_REFUND_)
				sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
			else
				sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
			
			inFunc_Amount_Comma(szTemplate, "NT$" , ' ', _SIGNED_NONE_, 20, VS_TRUE);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        sprintf(szPrintBuf, "22::總計(Total)%s%s\x0D\x0A", _DATA_FLAG_, szTemplate);

                        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
                        inDataLen += strlen(szPrintBuf);
		}
		
	}
	
	return (inDataLen);
}

/*
Function        :inNCCC_ESC_PRINT_ReceiptEND_Small_S
Date&Time       :2023/3/27 下午 5:50
Describe        :組E1 結尾
*/
int inNCCC_ESC_PRINT_ReceiptEND_Small_S(TRANSACTION_OBJECT *pobTran, char *szPackData)
{
	int	inDataLen = 0;
	char	szPrintBuf[200 + 1], szTemplate[42 + 1] = {0};
	char	szPrtNotice[1 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
        
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	/* 簽名欄 */
	/* 免簽名 */
	/* 小費一律簽名 */
	if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE && pobTran->srBRec.inCode != _TIP_)
	{
		
	}
	/* (需求單 - 107227)邦柏科技自助作業客製化 by Russell 2018/12/7 下午 2:36 */
	/* (需求單 - 107276)自助交易標準400做法 by Russell 2018/12/27 上午 11:20 */
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)         ||
                 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_))
	{
		
	}
	/* 要簽名 */
	else
	{
		/* 有在signpad簽名 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
		inFunc_ComposeFileName_InvoiceNumber(pobTran, szTemplate, _PICTURE_FILE_EXTENSION_, 6);

		if (inFILE_Check_Exist((unsigned char *)szTemplate) == VS_SUCCESS)
		{
			/* 電子簽名 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "G::%s\x0D\x0A", _ESC_BMP_SIGNATURE_);
			memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
		}
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "14::%s\x0D\x0A", "X:");
		memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "14::%s\x0D\x0A", "----------------------------");
		memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
	}

	/* "持卡人姓名" */
	if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE && pobTran->srBRec.inCode != _TIP_)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "11::%s\x0D\x0A", "                                     持卡人簽名(特店存根聯)               免簽名");
		memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
	}
	/* (需求單 - 107227)邦柏科技自助作業客製化 by Russell 2018/12/7 下午 2:36 */
	/* (需求單 - 107276)自助交易標準400做法 by Russell 2018/12/27 上午 11:20 */
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)         ||
                 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_))
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "11::%s\x0D\x0A", "                                     持卡人簽名(特店存根聯)               免簽名");
		memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
	}
	else
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "12::%s\x0D\x0A", "持卡人簽名(特店存根聯)");
		memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
	}
	
	/* 持卡人姓名值 */
	if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)) == 0 &&
	    pobTran->srBRec.uszUCARDTransBit != VS_TRUE)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "11::%s\x0D\x0A", pobTran->srBRec.szCardHolder);
		memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
	}
		
	/* 列印警示語 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "12::%s\x0D\x0A", "I AGREE TO PAY TOTAL AMOUNT");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "12::%s\x0D\x0A", "ACCORDING TO CARD ISSUER AGREEMENT");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* Print Notice */
	memset(szPrtNotice, 0x00, sizeof(szPrtNotice));
	inGetPrtNotice(szPrtNotice);
	if(memcmp(szPrtNotice, "Y", strlen("Y")) == 0)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "G::%s\x0D\x0A", _ESC_BMP_NOTICE_LOGO_);
                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
	}
	
        return (inDataLen);
}

/*
Function        :inNCCC_ESC_PRINT_DCC_ReceiptEND_Small_S
Date&Time       :2023/4/7 下午 5:25
Describe        :
*/
int inNCCC_ESC_PRINT_DCC_ReceiptEND_Small_S(TRANSACTION_OBJECT *pobTran, char *szPackData)
{
	int	inTempLen = 0;
	int	inDataLen = 0;
	char	szPrtNotice[1 + 1];
	char	szPrintBuf[200 + 1], szTemplate[100 + 1];
	char	szTempBuff[1000] = {};

	/* 免責宣言上需空一行 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "12::%s\x0D\x0A", " ");
	memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
	inDataLen += strlen(szPrintBuf);
	
	/* Disclaimer */
	if ((!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_VISA_, strlen(_CARD_TYPE_VISA_))) && 
	    (pobTran->srBRec.inCode == _SALE_		|| 
	     pobTran->srBRec.inCode == _TIP_		|| 
	     pobTran->srBRec.inCode == _PRE_COMP_	||
	    (pobTran->srBRec.inCode == _SALE_ && pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE)))
	{
		memset(szTempBuff, 0x00, sizeof(szTempBuff));
		inTempLen = inNCCC_ESC_PRINT_DCC_Disclaimer(_ESC_DCC_V_, szTempBuff);
		if (inTempLen > 0)
		{
		        memcpy(&szPackData[inDataLen], &szTempBuff[0], inTempLen);
                        inDataLen += inTempLen;
		}
	}
	else if ((!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_))) && 
		 (pobTran->srBRec.inCode == _SALE_	|| 
		  pobTran->srBRec.inCode == _TIP_	|| 
		  pobTran->srBRec.inCode == _PRE_COMP_	||
		 (pobTran->srBRec.inCode == _SALE_ && pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE)))
	{
		memset(szTempBuff, 0x00, sizeof(szTempBuff));
		inTempLen = inNCCC_ESC_PRINT_DCC_Disclaimer(_ESC_DCC_M_, szTempBuff);
		if (inTempLen > 0)
		{
		        memcpy(&szPackData[inDataLen], &szTempBuff[0], inTempLen);
                        inDataLen += inTempLen;
		}
	}
	
	if (pobTran->srBRec.inPrintOption == _PRT_MERCH_	||
	    pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
        {
		if (pobTran->srBRec.uszDCCTransBit == VS_TRUE && pobTran->srBRec.inCode == _TIP_)
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "11::%s\x0D\x0A", "[ ] ACCEPT");
			memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
		}
	}
	
	if (pobTran->srBRec.uszDCCTransBit == VS_TRUE && pobTran->srBRec.inCode == _TIP_)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "11::%s\x0D\x0A", " ");
		memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "11::%s\x0D\x0A", "Cardholder expressly agrees to the Transaction ");
		memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "11::%s\x0D\x0A", "Receipt Information by marking the “accept box");
		memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "11::%s\x0D\x0A", "below.");
		memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "14::%s\x0D\x0A", "[ ] ACCEPT");
		memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
	}

	/* 簽名欄 */
	/* 免簽名 */
	/* 小費一律簽名 */
	if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE && pobTran->srBRec.inCode != _TIP_)
	{
		
	}
	/* 要簽名 */
	else
	{
		/* 有在signpad簽名 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
		inFunc_ComposeFileName_InvoiceNumber(pobTran, szTemplate, _PICTURE_FILE_EXTENSION_, 6);

		if (inFILE_Check_Exist((unsigned char *)szTemplate) == VS_SUCCESS)
		{
			/* 電子簽名 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "G::%s\x0D\x0A", _ESC_BMP_SIGNATURE_);
			memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
		}
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "14::%s\x0D\x0A", "X:");
		memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "14::%s\x0D\x0A", "----------------------------");
		memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
	}

	/* "持卡人簽名" */
	if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE && pobTran->srBRec.inCode != _TIP_)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "11::%s\x0D\x0A", "                                     持卡人簽名(特店存根聯)               免簽名");
		memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
	}
	else
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "12::%s\x0D\x0A", "持卡人簽名(特店存根聯)");
		memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
	}
	
	/* 持卡人姓名值 */
	if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)) == 0 &&
	    pobTran->srBRec.uszUCARDTransBit != VS_TRUE)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "11::%s\x0D\x0A", pobTran->srBRec.szCardHolder);
		memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
	}
		
	/* 列印警示語 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "12::%s\x0D\x0A", "I AGREE TO PAY TOTAL AMOUNT");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "12::%s\x0D\x0A", "ACCORDING TO CARD ISSUER AGREEMENT");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* Print Notice */
	memset(szPrtNotice, 0x00, sizeof(szPrtNotice));
	inGetPrtNotice(szPrtNotice);
	if(memcmp(szPrtNotice, "Y", strlen("Y")) == 0)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "G::%s\x0D\x0A", _ESC_BMP_NOTICE_LOGO_);
                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
	}

	return (inDataLen);
}

/*
Function        :inNCCC_ESC_PRINT_DCC_DATA_Small_S
Date&Time       :2025/3/29 下午 5:48
Describe        :
*/
int inNCCC_ESC_PRINT_DCC_DATA_Small_S(TRANSACTION_OBJECT *pobTran, char *szPackData)
{
	int	i = 0;
	int	inDataLen = 0;
	int	inTempLen = 0;
	int	inUpperAmount = 0;
	int	inLowerAmount = 0;
	char	szPrintBuf[200 + 1] = {0}, szPrintBuf1[100 + 1] = {0}, szPrintBuf2[100 + 1] = {0}, szTemplate[100 + 1] = {0}, szTemplate1[100 + 1] = {0}, szTemplate2[100 + 1] = {0};
	char	szProductCodeEnable[1 + 1];
	char	szStore_Stub_CardNo_Truncate_Enable[2 + 1];
	char	szExamBit[10 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);

	/* "卡號 卡別" 卡別值 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	strcpy(szTemplate, pobTran->srBRec.szCardLabel);
	
	inTempLen =  ((20 - strlen(szTemplate)) * _ESC_SMALL_CARD_TYPE_MULTIPLE_) + _ESC_SMALL_CARD_TYPE_OFFSET_;
	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	for (i = 0; i < inTempLen; i++)
	{
		strcat(szTemplate2, " ");
	}
	strcat(szTemplate2, szTemplate);
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	strcat(szPrintBuf, "29::");
	strcat(szPrintBuf, "卡號");
	strcat(szPrintBuf, "\x20");
	strcat(szPrintBuf, _DATA_FLAG_);
	strcat(szPrintBuf, "卡別");
	strcat(szPrintBuf, "\x20");
	strcat(szPrintBuf, szTemplate2);
	strcat(szPrintBuf, "\x0D\x0A");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	/* 卡號值 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	strcpy(szTemplate, pobTran->srBRec.szPAN);
	
	/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
	/* 商店聯卡號遮掩 */
	memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
	inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
	/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/8 下午 1:55 */
	if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
	{
		/* Uny交易不受TMS開關限制，且獲取卡號時已遮掩 */
	}
	else if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0 && pobTran->srBRec.uszTxNoCheckBit == VS_TRUE)
	{
		/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
		for (i = 6; i < (strlen(szTemplate) - 4); i ++)
			szTemplate[i] = 0x2A;
	}

	/* 過卡方式 */
	if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
	{
		if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
			strcat(szTemplate, "(W)");
		else
			strcat(szTemplate, "(C)");
	}
	else if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
	{
		strcat(szTemplate, "(Q)");
	}
	else if (pobTran->srBRec.uszCUPEMVQRCodeBit == VS_TRUE)
	{
		strcat(szTemplate, "(Q)");
	}
	else
	{
		if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
			strcat(szTemplate,"(C)");
		else if (pobTran->srBRec.uszMobilePayBit == VS_TRUE)
			strcat(szTemplate, "(T)");
		else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
			strcat(szTemplate, "(W)");
		else
		{
			if (pobTran->srBRec.uszManualBit == VS_TRUE)
			{
				/* 【需求單 - 105244】端末設備支援以感應方式進行退貨交易 */
				/* 電文轉Manual Keyin但是簽單要印感應的W */
				if (pobTran->srBRec.uszRefundCTLSBit == VS_TRUE)
					strcat(szTemplate, "(W)");
				else
					strcat(szTemplate,"(M)");
			}
			else
				strcat(szTemplate,"(S)");
		}
	}
       	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s\x0D\x0A", szTemplate);

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	/* 交易類別 */
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "11::%s %s\x0D\x0A", "交易", szTemplate1);

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	if (strlen(szTemplate2) > 0)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "11::%s\x0D\x0A", szTemplate2);

		memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
	}
	
	/* 城市 主機 */
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	inGetCityName(szTemplate1);
	
	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	inGetHostLabel(szTemplate2);
	inFunc_DiscardSpace(szTemplate2);
	inFunc_PAD_ASCII(szTemplate2, szTemplate2, ' ', 8, _PADDING_LEFT_);
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	strcat(szPrintBuf, "40::");
	strcat(szPrintBuf, "城市");
	strcat(szPrintBuf, _DATA_FLAG_);
	strcat(szPrintBuf, szTemplate1);
	strcat(szPrintBuf, _DATA_FLAG_);
	strcat(szPrintBuf, "主機");
	strcat(szPrintBuf, _DATA_FLAG_);
	strcat(szPrintBuf, szTemplate2);
	strcat(szPrintBuf, "\x0D\x0A");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);
	
	/* "日期/時間" 日期值 "批號" 批號值 */
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	sprintf(szTemplate1, "%.4s/%.2s/%.2s %.2s:%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6], &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
	
	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	sprintf(szTemplate2, "%03ld", pobTran->srBRec.lnBatchNum);
	inFunc_PAD_ASCII(szTemplate2, szTemplate2, ' ', 8, _PADDING_LEFT_);
		
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	strcat(szPrintBuf, "40::");
	strcat(szPrintBuf, "日期/時間");
	strcat(szPrintBuf, _DATA_FLAG_);
	strcat(szPrintBuf, szTemplate1);
	strcat(szPrintBuf, _DATA_FLAG_);
	strcat(szPrintBuf, "批號");
	strcat(szPrintBuf, _DATA_FLAG_);
	strcat(szPrintBuf, szTemplate2);
	strcat(szPrintBuf, "\x0D\x0A");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* "授權碼" 授權碼值 "檢查碼" 檢查碼值 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	memcpy(szTemplate1, &pobTran->srBRec.szAuthCode[0], 12);
	
	if (pobTran->srBRec.uszTakaTransBit == VS_TRUE)
	{
		memset(szTemplate2, 0x00, sizeof(szTemplate2));
		sprintf(szTemplate2, "%.2s/'%.2s", pobTran->srBRec.szExpDate + 2, pobTran->srBRec.szExpDate);
	}
	else
	{
		memset(szTemplate2, 0x00, sizeof(szTemplate2));
		/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/28 下午 2:42 */
		if (strlen(pobTran->srBRec.szCheckNO) > 0)
		{
			strcpy(szTemplate2, pobTran->srBRec.szCheckNO);
		}
		else
		{
			inCARD_ExpDateEncryptAndDecrypt(pobTran, "", szTemplate2, _EXP_ENCRYPT_);
		}
		inFunc_PAD_ASCII(szTemplate2, szTemplate2, ' ', 9, _PADDING_LEFT_);
	}
	
	/* 大高卡只能顯示有效期 */
	if (pobTran->srBRec.uszTakaTransBit == VS_TRUE)
	{
		strcat(szPrintBuf, "40::");
		strcat(szPrintBuf, "授權碼");
		strcat(szPrintBuf, _DATA_FLAG_);
		strcat(szPrintBuf, szTemplate1);
		strcat(szPrintBuf, _DATA_FLAG_);
		strcat(szPrintBuf, "有效期");
		strcat(szPrintBuf, _DATA_FLAG_);
		strcat(szPrintBuf, szTemplate2);
		strcat(szPrintBuf, "\x0D\x0A");
	}
	else
	{
		strcat(szPrintBuf, "40::");
		strcat(szPrintBuf, "授權碼");
		strcat(szPrintBuf, _DATA_FLAG_);
		strcat(szPrintBuf, szTemplate1);
		strcat(szPrintBuf, _DATA_FLAG_);
		strcat(szPrintBuf, "檢查碼");
		strcat(szPrintBuf, _DATA_FLAG_);
		strcat(szPrintBuf, szTemplate2);
		strcat(szPrintBuf, "\x0D\x0A");
	}

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* "序號" 序號值 "調閱號" 調閱編號值 */
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	inFunc_PAD_ASCII(szTemplate1, pobTran->srBRec.szRefNo, ' ', 25, _PADDING_RIGHT_);

	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	sprintf(szTemplate2, "%06ld", pobTran->srBRec.lnOrgInvNum);
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	strcat(szPrintBuf, "40::");
	strcat(szPrintBuf, "序號");
	strcat(szPrintBuf, _DATA_FLAG_);
	strcat(szPrintBuf, szTemplate1);
	strcat(szPrintBuf, _DATA_FLAG_);
	strcat(szPrintBuf, "調閱號");
	strcat(szPrintBuf, _DATA_FLAG_);
	strcat(szPrintBuf, szTemplate2);
	strcat(szPrintBuf, "\x0D\x0A");

        memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
        inDataLen += strlen(szPrintBuf);

	/* 櫃號 */
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	inGetStoreIDEnable(szTemplate1);
	if ((memcmp(&szTemplate1[0], "Y", 1) == 0) && (strlen(pobTran->srBRec.szStoreID) > 0))
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memcpy(szTemplate1, &pobTran->srBRec.szStoreID[0], 23);
		
		memset(szTemplate2, 0x00, sizeof(szTemplate2));
		sprintf(szTemplate2, "%s", pobTran->srBRec.szRespCode);
		inFunc_PAD_ASCII(szTemplate2, szTemplate2, ' ', 9, _PADDING_LEFT_);
		
		if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
		{
			strcat(szPrintBuf, "40::");
			strcat(szPrintBuf, "櫃號");
			strcat(szPrintBuf, _DATA_FLAG_);
			strcat(szPrintBuf, szTemplate1);
			strcat(szPrintBuf, _DATA_FLAG_);
			strcat(szPrintBuf, "回覆碼");
			strcat(szPrintBuf, _DATA_FLAG_);
			strcat(szPrintBuf, szTemplate2);
			strcat(szPrintBuf, "\x0D\x0A");
		}
		else
		{
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_026_TAKA_, _CUSTOMER_INDICATOR_SIZE_) ||
			    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
			{
				strcat(szPrintBuf, "40::");
				strcat(szPrintBuf, "品群碼");
				strcat(szPrintBuf, _DATA_FLAG_);
				strcat(szPrintBuf, szTemplate1);
				strcat(szPrintBuf, _DATA_FLAG_);
				strcat(szPrintBuf, "\x20");
				strcat(szPrintBuf, _DATA_FLAG_);
				strcat(szPrintBuf, "\x20");
				strcat(szPrintBuf, "\x0D\x0A");
			}
			else
			{
				strcat(szPrintBuf, "40::");
				strcat(szPrintBuf, "櫃號");
				strcat(szPrintBuf, _DATA_FLAG_);
				strcat(szPrintBuf, szTemplate1);
				strcat(szPrintBuf, _DATA_FLAG_);
				strcat(szPrintBuf, "\x20");
				strcat(szPrintBuf, _DATA_FLAG_);
				strcat(szPrintBuf, "\x20");
				strcat(szPrintBuf, "\x0D\x0A");
			}	
		}
		
		memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
		inDataLen += strlen(szPrintBuf);
		
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			/* hardcode */
			char	szPOS_ID[6 + 1] = {0};
			memset(szPOS_ID, 0x00, sizeof(szPOS_ID));
			inGetPOS_ID(szPOS_ID);

			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			memcpy(szTemplate1, _HARDCODE_STORE_ID_CUS_021_TAKAWEL_, 12);
			memcpy(&szTemplate1[12], szPOS_ID, 6);
			
			memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
		}
	}
	else
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memcpy(szTemplate1, &pobTran->srBRec.szStoreID[0], 23);
		
		memset(szTemplate2, 0x00, sizeof(szTemplate2));
		sprintf(szTemplate2, "%s", pobTran->srBRec.szRespCode);
		inFunc_PAD_ASCII(szTemplate2, szTemplate2, ' ', 9, _PADDING_LEFT_);
		
		if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
		{
			strcat(szPrintBuf, "40::");
			strcat(szPrintBuf, "\x20");
			strcat(szPrintBuf, _DATA_FLAG_);
			strcat(szPrintBuf, "\x20");
			strcat(szPrintBuf, _DATA_FLAG_);
			strcat(szPrintBuf, "回覆碼");
			strcat(szPrintBuf, _DATA_FLAG_);
			strcat(szPrintBuf, szTemplate2);
			strcat(szPrintBuf, "\x0D\x0A");
			
			memcpy(&szPackData[inDataLen], szPrintBuf, strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
		}
	}
        
        /* 【需求單-113260】MasterCard機票交易需求 by Russell 2025/3/3 上午 11:55 */
        /* 取消維持原交易不列印機票資訊 */
        if (inNCCC_Func_Flight_Ticket_Allow_Print(pobTran) == VS_TRUE)
        {
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "41::%s%s%s%s%s%s%s\x0D\x0A", "出發地機場", _DATA_FLAG_, pobTran->srBRec.szFlightTicketPDS0523, _DATA_FLAG_, "目的地機場", _DATA_FLAG_, pobTran->srBRec.szFlightTicketPDS0524);

                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);

                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                inNCCC_Func_Get_Flight_Ticket_Txn_Category_PrintName(pobTran, szTemplate1);
                memset(szTemplate2, 0x00, sizeof(szTemplate2));
                if (strlen(pobTran->srBRec.szFlightTicketPDS0530) > 0)
                {
                        sprintf(szTemplate2, "%d", atoi(pobTran->srBRec.szFlightTicketPDS0530));
                }
                else
                {
                        sprintf(szTemplate2, "%s", " ");
                }
                sprintf(szPrintBuf, "%s", szTemplate2);
                
		if (!memcmp(szTemplate1, _FLIGHT_TICKET_TXN_CATEGORY_PRINT_BAGGAGE_CHARGE_, max(strlen(szTemplate1), strlen(_FLIGHT_TICKET_TXN_CATEGORY_PRINT_BAGGAGE_CHARGE_)))		||
		    !memcmp(szTemplate1, _FLIGHT_TICKET_TXN_CATEGORY_PRINT_PREPAID_TICKET_, max(strlen(szTemplate1), strlen(_FLIGHT_TICKET_TXN_CATEGORY_PRINT_PREPAID_TICKET_)))		||
		    !memcmp(szTemplate1, _FLIGHT_TICKET_TXN_CATEGORY_PRINT_MISCELLANEOUS_CHARGE_ , max(strlen(szTemplate1), strlen(_FLIGHT_TICKET_TXN_CATEGORY_PRINT_MISCELLANEOUS_CHARGE_)))	||
		    !memcmp(szTemplate1, _FLIGHT_TICKET_TXN_CATEGORY_PRINT_TICKET_REISSUE_, max(strlen(szTemplate1), strlen(_FLIGHT_TICKET_TXN_CATEGORY_PRINT_TICKET_REISSUE_))))
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "41::%s%s%s%s%s%s%s\x0D\x0A", "航班號碼", _DATA_FLAG_, szTemplate2, _DATA_FLAG_, "交易類型", _DATA_FLAG_, " ");

			memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);

			/* 根據業務要求交易類型換行靠右 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "24::%s%s%s\x0D\x0A", " ", _DATA_FLAG_, szTemplate1);

			memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
		}
		else
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "41::%s%s%s%s%s%s%s\x0D\x0A", "航班號碼", _DATA_FLAG_, szTemplate2, _DATA_FLAG_, "交易類型", _DATA_FLAG_, szTemplate1);

			memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
		}
        }
	
	/* TC */
	/* 驗測開關 */
	memset(szExamBit, 0x00, sizeof(szExamBit));
	inGetExamBit(szExamBit);
	/* Uny交易 */
	/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/8 下午 1:55 */
	if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
	{
		/* 取消或退貨，只印數字交易碼 */
		/* 最小字體 */
		if (pobTran->srBRec.uszVOIDBit == VS_TRUE ||
		    pobTran->srBRec.inCode == _REFUND_ || pobTran->srBRec.inCode == _INST_REFUND_ || pobTran->srBRec.inCode == _REDEEM_REFUND_)
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			sprintf(szTemplate1, "交易碼 %s", pobTran->srBRec.szUnyTransCode);
			sprintf(szPrintBuf, "11::%s\x0D\x0A", szTemplate1);

			memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
			inDataLen += strlen(szPrintBuf);
		}
		/* 正向交易，要印條碼和數字 */
		else
		{

			if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
			{
				/* 一維條碼 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%s", pobTran->srBRec.szUnyTransCode);
				sprintf(szPrintBuf, "U::%s\x0D\x0A", szTemplate1);
				
				memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);

				/* 數字 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "交易碼(退貨請掃描) %s", pobTran->srBRec.szUnyTransCode);
				sprintf(szPrintBuf, "11::%s\x0D\x0A", szTemplate1);

				memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);
			}
			else
			{
				/* 一維條碼 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%s", pobTran->srBRec.szUnyTransCode);
				sprintf(szPrintBuf, "U::%s\x0D\x0A", szTemplate1);

				memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);

				/* 數字 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "交易碼 %s", pobTran->srBRec.szUnyTransCode);
				sprintf(szPrintBuf, "11::%s\x0D\x0A", szTemplate1);

				memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);
			}
		}
	}
	/* 實體卡交易和掃碼交易互斥 */
	else
	{
		if (pobTran->srBRec.inChipStatus == _EMV_CARD_ || pobTran->srBRec.uszContactlessBit == VS_TRUE)
		{
			/* 感應磁條 */
			if (!memcmp(pobTran->srBRec.szAuthCode, "VLP", 3)				|| 
			    !memcmp(pobTran->srBRec.szAuthCode, "JCB", 3)				||
			     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE		|| 
			     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_64_NEWJSPEEDY_MSD		||
			     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE)
			{
				/* 商店聯卡號遮掩 */
				memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
				inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
				if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
				    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
				    strlen(pobTran->srBRec.szTxnNo) > 0)
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "11::%s %s\x0D\x0A", "交易編號", pobTran->srBRec.szTxnNo);
					memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
					inDataLen += strlen(szPrintBuf);
				}
			}
			else
			{
				if (pobTran->srBRec.inPrintOption == _PRT_MERCH_)
				{
					if (pobTran->srEMVRec.in9F26_ApplCryptogramLen > 0)
					{
						memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
						sprintf(szPrintBuf1, "%02X%02X%02X%02X%02X%02X%02X%02X",
										pobTran->srEMVRec.usz9F26_ApplCryptogram[0],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[1],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[2],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[3],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[4],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[5],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[6],
										pobTran->srEMVRec.usz9F26_ApplCryptogram[7]);

						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "23::TC%s%s\x0D\x0A", _DATA_FLAG_, szPrintBuf1);

						memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
						inDataLen += strlen(szPrintBuf);
					}

					/* 商店聯卡號遮掩 */
					memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
					inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
					if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
					    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
					    strlen(pobTran->srBRec.szTxnNo) > 0)
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "11::%s %s\x0D\x0A", "交易編號", pobTran->srBRec.szTxnNo);
						memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
						inDataLen += strlen(szPrintBuf);
					}

					/* M/C交易列印AP Lable (START) */
					if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)))
					{
						if (pobTran->srEMVRec.in50_APLabelLen > 0)
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							strcat(szPrintBuf, "AP Label:");
							memset(szTemplate, 0x00, sizeof(szTemplate));
							strcpy(szTemplate, (char*)pobTran->srEMVRec.usz50_APLabel);
							inUpperAmount = inFunc_Get_UpperCase_Char(szTemplate);
							inLowerAmount = strlen(szTemplate) - inUpperAmount;
							inTempLen =  _ESC_AP_LABEL_TOTAL_SPACE_ - (_ESC_UPPER_MULTIPLE_ * inUpperAmount + _ESC_LOWER_MULTIPLE_ * inLowerAmount);
							for (i = 0; i < inTempLen; i++)
							{
								strcat(szPrintBuf, " ");
							}
							strcat(szPrintBuf, szTemplate);
						}
						else
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							strcat(szPrintBuf, "AP Label:");
							strcat(szPrintBuf, "\x20");
						}
					}
					/* M/C交易列印AP Lable (END) */
					else if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)))
					{
						if (pobTran->srEMVRec.in50_APLabelLen > 0)
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							strcat(szPrintBuf, "AP Label:");
							memset(szTemplate, 0x00, sizeof(szTemplate));
							strcpy(szTemplate, (char*)pobTran->srEMVRec.usz50_APLabel);
							inUpperAmount = inFunc_Get_UpperCase_Char(szTemplate);
							inLowerAmount = strlen(szTemplate) - inUpperAmount;
							inTempLen =  _ESC_AP_LABEL_TOTAL_SPACE_ - (_ESC_UPPER_MULTIPLE_ * inUpperAmount + _ESC_LOWER_MULTIPLE_ * inLowerAmount);
							for (i = 0; i < inTempLen; i++)
							{
								strcat(szPrintBuf, " ");
							}
							strcat(szPrintBuf, szTemplate);
						}
						else
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							strcat(szPrintBuf, "AP Label:");
							strcat(szPrintBuf, "\x20");
						}
					}
					/* 驗測要印AP Label 和 AID */
					else if (szExamBit[0] == '1')
					{
						if (pobTran->srEMVRec.in50_APLabelLen > 0)
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							strcat(szPrintBuf, "AP Label:");
							memset(szTemplate, 0x00, sizeof(szTemplate));
							strcpy(szTemplate, (char*)pobTran->srEMVRec.usz50_APLabel);
							inUpperAmount = inFunc_Get_UpperCase_Char(szTemplate);
							inLowerAmount = strlen(szTemplate) - inUpperAmount;
							inTempLen =  _ESC_AP_LABEL_TOTAL_SPACE_ - (_ESC_UPPER_MULTIPLE_ * inUpperAmount + _ESC_LOWER_MULTIPLE_ * inLowerAmount);
							for (i = 0; i < inTempLen; i++)
							{
								strcat(szPrintBuf, " ");
							}
							strcat(szPrintBuf, szTemplate);
						}
						else
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							strcat(szPrintBuf, "AP Label:");
							strcat(szPrintBuf, "\x20");
						}
					}
					else
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						strcat(szPrintBuf, "\x20");
						strcat(szPrintBuf, "\x20");
					}
					memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
					sprintf(szPrintBuf2, szPrintBuf);
					
					/* AID */
					if (strlen(pobTran->srBRec.szCUP_EMVAID) > 0)
					{
						memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
						sprintf(szPrintBuf1, "AID:%s", pobTran->srBRec.szCUP_EMVAID);
						inFunc_PAD_ASCII(szPrintBuf1, szPrintBuf1, ' ', _ESC_AID_TOTAL_LEN_, _PADDING_RIGHT_);
						
						if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
						{
							/* CUP晶片要印 */
							if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
							else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)) == 0)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
							/* 【需求單-109121】Mastercard交易簽單列印AID需求 by Russell 2020/7/23 下午 4:23 */
							else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)) == 0)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
							/* 【需求單-111290】VISA置換Logo、DISCOVER簽單新增AID欄位、AE簽單新增AID和AP Label欄位需求 by Russell 2022/12/29 下午 5:57 */
							else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)) == 0 &&
								pobTran->srBRec.uszUCARDTransBit != VS_TRUE)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
							/* 驗測要印AP Label 和 AID */
							else if (szExamBit[0] == '1')
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
						}
						/* 銀聯閃付 */
						else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
						{
							if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
							/* AE晶片要印出AID */
							else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_50_EXPRESSSPAY || 
								 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
							/* 【需求單-109121】Mastercard交易簽單列印AID需求 by Russell 2020/7/23 下午 4:23 */
							else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE || 
								 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_21_PAYPASS_MCHIP)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
							else if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_42_DPAS	|| pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_43_DISCOVER_DPAS_MAG_STRIPE) &&
								pobTran->srBRec.uszUCARDTransBit != VS_TRUE)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
							/* 驗測要印AP Label 和 AID */
							else if (szExamBit[0] == '1')
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								strcat(szPrintBuf, "11");
								strcat(szPrintBuf, "::");
								strcat(szPrintBuf, szPrintBuf1);
								strcat(szPrintBuf, szPrintBuf2);
								strcat(szPrintBuf, "\x0D\x0A");

								memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
								inDataLen += strlen(szPrintBuf);
							}
						}
					}
				}
			}

		}
		else
		{
			/* 商店聯卡號遮掩 */
			/* 磁條卡列印交易編號 */
			memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
			inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
			if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
			    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
			    strlen(pobTran->srBRec.szTxnNo) > 0)
			{
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "11::%s %s\x0D\x0A", "交易編號", pobTran->srBRec.szTxnNo);
				memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
				inDataLen += strlen(szPrintBuf);
			}
		}
	}
	
	/* 產品代碼 */
	/* 吳升文回覆 產品代碼列印於TC欄位下方 */
	inGetProductCodeEnable(szProductCodeEnable);
	if (memcmp(szProductCodeEnable, "Y", 1) == 0)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		strcat(szPrintBuf, "40::");
		strcat(szPrintBuf, "產品代碼");
		strcat(szPrintBuf, _DATA_FLAG_);
		strcat(szPrintBuf, pobTran->srBRec.szProductCode);
		strcat(szPrintBuf, _DATA_FLAG_);
		strcat(szPrintBuf, "\x20");
		strcat(szPrintBuf, _DATA_FLAG_);
		strcat(szPrintBuf, "\x20");
		strcat(szPrintBuf, "\x0D\x0A");

                memcpy(&szPackData[inDataLen], &szPrintBuf[0], strlen(szPrintBuf));
                inDataLen += strlen(szPrintBuf);
	}
	
	return (inDataLen);
}