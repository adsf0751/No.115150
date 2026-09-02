#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <sqlite3.h>
#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Define_2.h"
#include "../INCLUDES/Transaction.h"
#include "../INCLUDES/TransType.h"
#include "../INCLUDES/AllStruct.h"
#include "../PRINT/Print.h"
#include "../DISPLAY/Display.h"
#include "../DISPLAY/DisTouch.h"
#include "../EVENT/Flow.h"
#include "../EVENT/MenuMsg.h"
#include "Sqlite.h"
#include "Accum.h"
#include "Card.h"
#include "HDT.h"
#include "HDPT.h"
#include "File.h"
#include "FuncTable.h"
#include "Function.h"
#include "EDC.h"
#include "CFGT.h"
#include "Utility.h"
#include "CDTX.h"
#include "../../CREDIT/CreditFunc.h"
#include "../../CREDIT/CreditCard.h"
#include "../../FISC/NCCCfisc.h"
#include "../../NCCC/NCCCesc.h"
#include "../../NCCC/NCCCsrc.h"
#include "../../NCCC/NCCCTicketSrc.h"
#include "../../DINERS/DINERSsrc.h"


extern	int	ginDebug; /* Debug使用 extern */
extern	int	ginFindRunTime;

/*
Function        :inACCUM_UpdateTotalAmount
Date&Time       :2015/9/4 下午 14:00
Describe        :每當有交易時，到此fuction更新筆數和總額，回傳(VS_SUCCESS) or (VS_ERROR)
*/
int inACCUM_UpdateTotalAmount(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec)
{
        char            szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

        /* inACCUM_UpdateTotalAmount()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inACCUM_UpdateTotalAmount()_START");
        }

        /* 不是取消的交易 */
        if (pobTran->srBRec.uszVOIDBit == VS_FALSE)
        {
                switch (pobTran->srBRec.inCode)
                {
                        case _SALE_:
                        case _SALE_OFFLINE_:/* Offline Sale */
			case _CASH_ADVANCE_ :
			case _FORCE_CASH_ADVANCE_ :
			case _PRE_COMP_ :
			case _CUP_SALE_ :
			case _CUP_PRE_COMP_ :
			case _FISC_SALE_ :
			case _MAIL_ORDER_ :
			case _CUP_MAIL_ORDER_ :
                                /* 計算筆數 */
                                srAccumRec->lnTotalSaleCount++;
                                /* 【需求單-113172】免簽商店存根聯參數及短簽單調整需求 by Russell 2025/5/15 下午 2:41 */
                                if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE)
                                {
                                        srAccumRec->lnTotalSale_NoSignCount++;
                                }
                                else
                                {
                                        srAccumRec->lnTotalSale_SignCount++;
                                }
                                srAccumRec->lnTotalCount++;
                                /* 計算金額 */
                                srAccumRec->llTotalSaleAmount += pobTran->srBRec.lnTxnAmount;
                                /* 【需求單-113172】免簽商店存根聯參數及短簽單調整需求 by Russell 2025/5/15 下午 2:41 */
                                if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE)
                                {
                                        srAccumRec->llTotalSale_NoSignAmount += pobTran->srBRec.lnTxnAmount;
                                }
                                else
                                {
                                        srAccumRec->llTotalSale_SignAmount += pobTran->srBRec.lnTxnAmount;
                                }
                                srAccumRec->llTotalAmount += pobTran->srBRec.lnTxnAmount;
                                break;
                        case _REFUND_:
			case _CUP_REFUND_:
			case _CUP_MAIL_ORDER_REFUND_ :
			case _FISC_REFUND_ :
                                /* 計算筆數 */
                                srAccumRec->lnTotalRefundCount++;
                                srAccumRec->lnTotalCount++;
                                /* 計算金額 */
                                srAccumRec->llTotalRefundAmount += (0 - pobTran->srBRec.lnTxnAmount);
                                srAccumRec->llTotalAmount += (0 - pobTran->srBRec.lnTxnAmount);
                                break;
                        case _TIP_:
                                /* 計算筆數 */
                                srAccumRec->lnTotalTipsCount++;
                                /* 計算金額 */
                                srAccumRec->llTotalTipsAmount += pobTran->srBRec.lnTipTxnAmount;
                                if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE)
                                {
                                        srAccumRec->llTotalTips_NoSignAmount += pobTran->srBRec.lnTipTxnAmount;
                                }
                                else
                                {
                                        srAccumRec->llTotalTips_SignAmount += pobTran->srBRec.lnTipTxnAmount;
                                }
                                srAccumRec->llTotalAmount += pobTran->srBRec.lnTipTxnAmount;
				
				/* 如果是分期小費 */
				if (pobTran->srBRec.uszInstallmentBit == VS_TRUE)
				{
					/* 計算筆數 */
					srAccumRec->lnInstTipsCount ++;
					/* 計算金額 */
					srAccumRec->llInstTipsAmount += pobTran->srBRec.lnTipTxnAmount;
					srAccumRec->llInstTotalAmount += pobTran->srBRec.lnTipTxnAmount;
				}
				
				/* 如果是紅利小費 */
				if (pobTran->srBRec.uszRedeemBit == VS_TRUE)
				{
					/* 計算筆數 */
					srAccumRec->lnRedeemTipsCount ++;
					/* 計算金額 */
					srAccumRec->llRedeemTipsAmount += pobTran->srBRec.lnTipTxnAmount;
					srAccumRec->llRedeemTotalAmount += pobTran->srBRec.lnTipTxnAmount;
				}				
                                break;
                        case _ADJUST_:
                                /* 計算筆數 */
                                srAccumRec->lnTotalAdjustCount ++;
                                /* 計算金額 */
                                srAccumRec->llTotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                srAccumRec->llTotalSaleAmount += pobTran->srBRec.lnAdjustTxnAmount;
                                /* 【需求單-113172】免簽商店存根聯參數及短簽單調整需求 by Russell 2025/5/15 下午 2:41 */
                                if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE)
                                {
                                        srAccumRec->llTotalSale_NoSignAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llTotalSale_NoSignAmount += pobTran->srBRec.lnAdjustTxnAmount;
                                }
                                else
                                {
                                        srAccumRec->llTotalSale_SignAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llTotalSale_SignAmount += pobTran->srBRec.lnAdjustTxnAmount;
                                }
                                srAccumRec->llTotalAmount -= pobTran->srBRec.lnTxnAmount;
                                srAccumRec->llTotalAmount += pobTran->srBRec.lnAdjustTxnAmount;
                                break;
                        case _INST_SALE_:
                        	/* 計算筆數 */
				srAccumRec->lnInstSaleCount ++;
				srAccumRec->lnInstTotalCount ++;
				srAccumRec->lnTotalSaleCount ++;
                                /* 【需求單-113172】免簽商店存根聯參數及短簽單調整需求 by Russell 2025/5/15 下午 2:41 */
                                if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE)
                                {
                                        srAccumRec->lnTotalSale_NoSignCount++;
                                }
                                else
                                {
                                        srAccumRec->lnTotalSale_SignCount++;
                                }
				srAccumRec->lnTotalCount ++;
				/* 計算金額 */
				srAccumRec->llInstSaleAmount += pobTran->srBRec.lnTxnAmount;
				srAccumRec->llInstTotalAmount += pobTran->srBRec.lnTxnAmount;
				srAccumRec->llTotalSaleAmount += pobTran->srBRec.lnTxnAmount;
                                if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE)
                                {
                                        srAccumRec->llTotalSale_NoSignAmount += pobTran->srBRec.lnTxnAmount;
                                }
                                else
                                {
                                        srAccumRec->llTotalSale_SignAmount += pobTran->srBRec.lnTxnAmount;
                                }
				srAccumRec->llTotalAmount += pobTran->srBRec.lnTxnAmount;
				break;
                        case _INST_REFUND_:
				/* 計算筆數 */
				srAccumRec->lnInstRefundCount ++;
				srAccumRec->lnInstTotalCount ++;
				srAccumRec->lnTotalRefundCount ++;
				srAccumRec->lnTotalCount ++;
				/* 計算金額 */
				srAccumRec->llInstRefundAmount += (0 - pobTran->srBRec.lnTxnAmount);
				srAccumRec->llInstTotalAmount += (0 - pobTran->srBRec.lnTxnAmount);
				srAccumRec->llTotalRefundAmount += (0 - pobTran->srBRec.lnTxnAmount);
				srAccumRec->llTotalAmount += (0 - pobTran->srBRec.lnTxnAmount);
				break;
			case _INST_ADJUST_:
				/* 計算筆數 */
				srAccumRec->lnInstSaleCount ++;
				srAccumRec->lnInstTotalCount ++;
				srAccumRec->lnTotalSaleCount ++;
                                /* 【需求單-113172】免簽商店存根聯參數及短簽單調整需求 by Russell 2025/5/15 下午 2:41 */
                                if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE)
                                {
                                        srAccumRec->lnTotalSale_NoSignCount++;
                                }
                                else
                                {
                                        srAccumRec->lnTotalSale_SignCount++;
                                }
				srAccumRec->lnTotalCount ++;
				/* 計算金額 */
				srAccumRec->llInstSaleAmount += pobTran->srBRec.lnTxnAmount;
				srAccumRec->llInstTotalAmount += pobTran->srBRec.lnTxnAmount;
				srAccumRec->llTotalSaleAmount += pobTran->srBRec.lnTxnAmount;
                                if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE)
                                {
                                        srAccumRec->llTotalSale_NoSignAmount += pobTran->srBRec.lnTxnAmount;
                                }
                                else
                                {
                                        srAccumRec->llTotalSale_SignAmount += pobTran->srBRec.lnTxnAmount;
                                }
				srAccumRec->llTotalAmount += pobTran->srBRec.lnTxnAmount;
				break;
			case _REDEEM_SALE_:
				/* 計算筆數 */
				srAccumRec->lnRedeemSaleCount ++;
				srAccumRec->lnRedeemTotalCount ++;
				srAccumRec->lnTotalSaleCount ++;
                                /* 【需求單-113172】免簽商店存根聯參數及短簽單調整需求 by Russell 2025/5/15 下午 2:41 */
                                if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE)
                                {
                                        srAccumRec->lnTotalSale_NoSignCount++;
                                }
                                else
                                {
                                        srAccumRec->lnTotalSale_SignCount++;
                                }
				srAccumRec->lnTotalCount ++;
				/* 計算金額 */
				srAccumRec->llRedeemSaleAmount += pobTran->srBRec.lnTxnAmount;
				srAccumRec->llRedeemTotalAmount += pobTran->srBRec.lnTxnAmount;
				srAccumRec->llTotalSaleAmount += pobTran->srBRec.lnTxnAmount;
                                if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE)
                                {
                                        srAccumRec->llTotalSale_NoSignAmount += pobTran->srBRec.lnTxnAmount;
                                }
                                else
                                {
                                        srAccumRec->llTotalSale_SignAmount += pobTran->srBRec.lnTxnAmount;
                                }
				srAccumRec->llTotalAmount += pobTran->srBRec.lnTxnAmount;
				/* 計算總扣抵點數 */
				srAccumRec->lnRedeemTotalPoint += pobTran->srBRec.lnRedemptionPoints;
				break;
			case _REDEEM_REFUND_:
				/* 計算筆數 */
				srAccumRec->lnRedeemRefundCount ++;
				srAccumRec->lnRedeemTotalCount ++;
				srAccumRec->lnTotalRefundCount ++;
				srAccumRec->lnTotalCount ++;
				/* 計算金額 */
				srAccumRec->llRedeemRefundAmount += (0 - pobTran->srBRec.lnTxnAmount);
				srAccumRec->llRedeemTotalAmount += (0 - pobTran->srBRec.lnTxnAmount);
				srAccumRec->llTotalRefundAmount += (0 - pobTran->srBRec.lnTxnAmount);
				srAccumRec->llTotalAmount += (0 - pobTran->srBRec.lnTxnAmount);
				/* 計算總扣抵點數 */
				srAccumRec->lnRedeemTotalPoint += (0 - pobTran->srBRec.lnRedemptionPoints);
				break;
			case _REDEEM_ADJUST_:
				/* 計算筆數 */
				srAccumRec->lnRedeemSaleCount ++;
				srAccumRec->lnRedeemTotalCount ++;
				srAccumRec->lnTotalSaleCount ++;
                                /* 【需求單-113172】免簽商店存根聯參數及短簽單調整需求 by Russell 2025/5/15 下午 2:41 */
                                if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE)
                                {
                                        srAccumRec->lnTotalSale_NoSignCount++;
                                }
                                else
                                {
                                        srAccumRec->lnTotalSale_SignCount++;
                                }
				srAccumRec->lnTotalCount ++;
				/* 計算金額 */
				srAccumRec->llRedeemSaleAmount += pobTran->srBRec.lnTxnAmount;
				srAccumRec->llRedeemTotalAmount += pobTran->srBRec.lnTxnAmount;
				srAccumRec->llTotalSaleAmount += pobTran->srBRec.lnTxnAmount;
                                if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE)
                                {
                                        srAccumRec->llTotalSale_NoSignAmount += pobTran->srBRec.lnTxnAmount;
                                }
                                else
                                {
                                        srAccumRec->llTotalSale_SignAmount += pobTran->srBRec.lnTxnAmount;
                                }
				srAccumRec->llTotalAmount += pobTran->srBRec.lnTxnAmount;
				/* 計算總扣抵點數 */
				srAccumRec->lnRedeemTotalPoint += pobTran->srBRec.lnRedemptionPoints;
				break;
                        default:
                                /* 交易類別錯誤，(沒有此交易類別) */
                                /* debug */
                                if (ginDebug == VS_TRUE)
                                {
                                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                        sprintf(szErrorMsg, "inACCUM_UpdateTotalAmount ERROR!!");
                                        inLogPrintf(AT, szErrorMsg);

                                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                        sprintf(szErrorMsg, "Trans code = %d", pobTran->srBRec.inCode);
                                        inLogPrintf(AT, szErrorMsg);
                                }

                                return (VS_ERROR);
                }
        }
        /* 取消的時候 */
        else
        {
                switch (pobTran->srBRec.inOrgCode)
                {
                        case _SALE_:
                        case _SALE_OFFLINE_: /* Offline Sale */
			case _CASH_ADVANCE_ :
			case _FORCE_CASH_ADVANCE_ :
			case _PRE_COMP_ :
			case _CUP_SALE_ :
			case _CUP_PRE_COMP_ :
			case _FISC_SALE_ :
			case _MAIL_ORDER_ :
			case _CUP_MAIL_ORDER_ :
                                /* 計算筆數 */
                                srAccumRec->lnTotalSaleCount--;
                                /* 【需求單-113172】免簽商店存根聯參數及短簽單調整需求 by Russell 2025/5/15 下午 2:41 */
                                if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE)
                                {
                                        srAccumRec->lnTotalSale_NoSignCount--;
                                }
                                else
                                {
                                        srAccumRec->lnTotalSale_SignCount--;
                                }
                                srAccumRec->lnTotalCount--;
                                /* 計算金額 */
                                srAccumRec->llTotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE)
                                {
                                        srAccumRec->llTotalSale_NoSignAmount -= pobTran->srBRec.lnTxnAmount;
                                }
                                else
                                {
                                        srAccumRec->llTotalSale_SignAmount -= pobTran->srBRec.lnTxnAmount;
                                }
                                srAccumRec->llTotalAmount -= pobTran->srBRec.lnTxnAmount;
                                break;
                        case _REFUND_:
			case _CUP_REFUND_:
			case _CUP_MAIL_ORDER_REFUND_ :
				/* SmartPay退費不能取消 */
                                /* 計算筆數 */
                                srAccumRec->lnTotalRefundCount--;
                                srAccumRec->lnTotalCount--;
                                /* 計算金額 */
                                srAccumRec->llTotalRefundAmount += pobTran->srBRec.lnTxnAmount;
                                srAccumRec->llTotalAmount += pobTran->srBRec.lnTxnAmount;
                                break;
                        case _TIP_:
                                /* 取消整筆交易，計算筆數 */
                                srAccumRec->lnTotalTipsCount--;
                                srAccumRec->lnTotalSaleCount--;
                                /* 【需求單-113172】免簽商店存根聯參數及短簽單調整需求 by Russell 2025/5/15 下午 2:41 */
                                if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE)
                                {
                                        srAccumRec->lnTotalSale_NoSignCount--;
                                }
                                else
                                {
                                        srAccumRec->lnTotalSale_SignCount--;
                                }
                                srAccumRec->lnTotalCount --; /* 這裡取消的是原來一般交易產生的筆數 */
                                /* 取消整筆交易，計算金額 */
                                srAccumRec->llTotalTipsAmount -= pobTran->srBRec.lnTipTxnAmount;
                                if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE)
                                {
                                        srAccumRec->llTotalTips_NoSignAmount -= pobTran->srBRec.lnTipTxnAmount;
                                }
                                else
                                {
                                        srAccumRec->llTotalTips_SignAmount -= pobTran->srBRec.lnTipTxnAmount;
                                }
                                srAccumRec->llTotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE)
                                {
                                        srAccumRec->llTotalSale_NoSignAmount -= pobTran->srBRec.lnTxnAmount;
                                }
                                else
                                {
                                        srAccumRec->llTotalSale_SignAmount -= pobTran->srBRec.lnTxnAmount;
                                }
                                srAccumRec->llTotalAmount -= (pobTran->srBRec.lnTipTxnAmount + pobTran->srBRec.lnTxnAmount);
				
				/* 如果是分期小費 */
				if (pobTran->srBRec.uszInstallmentBit == VS_TRUE)
				{
					/* 取消整筆交易，計算筆數 */
					srAccumRec->lnInstTipsCount --;
					srAccumRec->lnInstSaleCount --;
					/* 取消整筆交易，計算金額 */
					srAccumRec->llInstTipsAmount -= pobTran->srBRec.lnTipTxnAmount;
					srAccumRec->llInstSaleAmount -= pobTran->srBRec.lnTxnAmount;
					srAccumRec->llInstTotalAmount -= (pobTran->srBRec.lnTipTxnAmount + pobTran->srBRec.lnTxnAmount);
				}
				
				/* 如果是紅利小費 */
				if (pobTran->srBRec.uszRedeemBit == VS_TRUE)
				{
					/* 取消整筆交易，計算筆數 */
					srAccumRec->lnRedeemTipsCount --;
					srAccumRec->lnRedeemSaleCount --;
					/* 取消整筆交易，計算金額 */
					srAccumRec->llRedeemTipsAmount -= pobTran->srBRec.lnTipTxnAmount;
					srAccumRec->llRedeemSaleAmount -= pobTran->srBRec.lnTxnAmount;
					srAccumRec->llRedeemTotalAmount -= (pobTran->srBRec.lnTipTxnAmount + pobTran->srBRec.lnTxnAmount);
				}
                                break;
                        case _ADJUST_:
                                /* 計算筆數 */
                                srAccumRec->lnTotalAdjustCount--;
                                srAccumRec->lnTotalSaleCount--;
                                /* 【需求單-113172】免簽商店存根聯參數及短簽單調整需求 by Russell 2025/5/15 下午 2:41 */
                                if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE)
                                {
                                        srAccumRec->lnTotalSale_NoSignCount--;
                                }
                                else
                                {
                                        srAccumRec->lnTotalSale_SignCount--;
                                }
                                srAccumRec->lnTotalCount --; /* 這裡取消的是原來一般交易產生的筆數 */
                                /* 計算金額 */
                                srAccumRec->llTotalSaleAmount -= (0 - pobTran->srBRec.lnTxnAmount);
                                srAccumRec->llTotalSaleAmount += (0 - pobTran->srBRec.lnAdjustTxnAmount);
                                if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE)
                                {
                                        srAccumRec->llTotalSale_NoSignAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llTotalSale_NoSignAmount += (0 - pobTran->srBRec.lnAdjustTxnAmount);
                                }
                                else
                                {
                                        srAccumRec->llTotalSale_SignAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llTotalSale_SignAmount += (0 - pobTran->srBRec.lnAdjustTxnAmount);
                                }
                                srAccumRec->llTotalAmount -= (0 - pobTran->srBRec.lnTxnAmount);
                                srAccumRec->llTotalAmount += (0 - pobTran->srBRec.lnAdjustTxnAmount);
                                break;
                        case _INST_SALE_:
				/* 計算筆數 */
				srAccumRec->lnInstSaleCount --;
				srAccumRec->lnInstTotalCount --;
				srAccumRec->lnTotalSaleCount --;
                                /* 【需求單-113172】免簽商店存根聯參數及短簽單調整需求 by Russell 2025/5/15 下午 2:41 */
                                if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE)
                                {
                                        srAccumRec->lnTotalSale_NoSignCount--;
                                }
                                else
                                {
                                        srAccumRec->lnTotalSale_SignCount--;
                                }
				srAccumRec->lnTotalCount --;
				/* 計算金額 */
				srAccumRec->llInstSaleAmount -= pobTran->srBRec.lnTxnAmount;
				srAccumRec->llInstTotalAmount -= pobTran->srBRec.lnTxnAmount;
				srAccumRec->llTotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE)
                                {
                                        srAccumRec->llTotalSale_NoSignAmount -= pobTran->srBRec.lnTxnAmount;
                                }
                                else
                                {
                                        srAccumRec->llTotalSale_SignAmount -= pobTran->srBRec.lnTxnAmount;
                                }
				srAccumRec->llTotalAmount -= pobTran->srBRec.lnTxnAmount;
				break;
			case _INST_REFUND_:
				/* 計算筆數 */
				srAccumRec->lnInstRefundCount --;
				srAccumRec->lnInstTotalCount --;
				srAccumRec->lnTotalRefundCount --;
				srAccumRec->lnTotalCount --;
				/* 計算金額 */
				srAccumRec->llInstRefundAmount += pobTran->srBRec.lnTxnAmount;
				srAccumRec->llInstTotalAmount += pobTran->srBRec.lnTxnAmount;
				srAccumRec->llTotalRefundAmount += pobTran->srBRec.lnTxnAmount;
				srAccumRec->llTotalAmount += pobTran->srBRec.lnTxnAmount;
				break;
			case _INST_ADJUST_:
				/* 取消整筆交易 */
				srAccumRec->lnInstTotalCount --;
				srAccumRec->lnInstSaleCount --;
				srAccumRec->lnTotalSaleCount --;
                                /* 【需求單-113172】免簽商店存根聯參數及短簽單調整需求 by Russell 2025/5/15 下午 2:41 */
                                if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE)
                                {
                                        srAccumRec->lnTotalSale_NoSignCount--;
                                }
                                else
                                {
                                        srAccumRec->lnTotalSale_SignCount--;
                                }
				srAccumRec->lnTotalCount --;
				/* 計算金額 */
				srAccumRec->llInstSaleAmount -= pobTran->srBRec.lnTxnAmount;
				srAccumRec->llInstTotalAmount -= pobTran->srBRec.lnTxnAmount;
				srAccumRec->llTotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE)
                                {
                                        srAccumRec->llTotalSale_NoSignAmount -= pobTran->srBRec.lnTxnAmount;
                                }
                                else
                                {
                                        srAccumRec->llTotalSale_SignAmount -= pobTran->srBRec.lnTxnAmount;
                                }
				srAccumRec->llTotalAmount -= pobTran->srBRec.lnTxnAmount;
				break;
			case _REDEEM_SALE_:
				/* 計算筆數 */
				srAccumRec->lnRedeemSaleCount --;
				srAccumRec->lnRedeemTotalCount --;
				srAccumRec->lnTotalSaleCount --;
                                /* 【需求單-113172】免簽商店存根聯參數及短簽單調整需求 by Russell 2025/5/15 下午 2:41 */
                                if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE)
                                {
                                        srAccumRec->lnTotalSale_NoSignCount--;
                                }
                                else
                                {
                                        srAccumRec->lnTotalSale_SignCount--;
                                }
				srAccumRec->lnTotalCount --;
				/* 計算金額 */
				srAccumRec->llRedeemSaleAmount -= pobTran->srBRec.lnTxnAmount;
				srAccumRec->llRedeemTotalAmount -= pobTran->srBRec.lnTxnAmount;
				srAccumRec->llTotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE)
                                {
                                        srAccumRec->llTotalSale_NoSignAmount -= pobTran->srBRec.lnTxnAmount;
                                }
                                else
                                {
                                        srAccumRec->llTotalSale_SignAmount -= pobTran->srBRec.lnTxnAmount;
                                }
				srAccumRec->llTotalAmount -= pobTran->srBRec.lnTxnAmount;
				/* 計算總扣抵點數 */
				srAccumRec->lnRedeemTotalPoint -= pobTran->srBRec.lnRedemptionPoints;
				break;
			case _REDEEM_REFUND_:
				/* 計算筆數 */
				srAccumRec->lnRedeemRefundCount --;
				srAccumRec->lnRedeemTotalCount --;
				srAccumRec->lnTotalRefundCount --;
				srAccumRec->lnTotalCount --;
				/* 計算金額 */
				srAccumRec->llRedeemRefundAmount += pobTran->srBRec.lnTxnAmount;
				srAccumRec->llRedeemTotalAmount += pobTran->srBRec.lnTxnAmount;
				srAccumRec->llTotalRefundAmount += pobTran->srBRec.lnTxnAmount;
				srAccumRec->llTotalAmount += pobTran->srBRec.lnTxnAmount;
				/* 計算總扣抵點數 */
				srAccumRec->lnRedeemTotalPoint += pobTran->srBRec.lnRedemptionPoints;
				break;
			case _REDEEM_ADJUST_:
				/* 計算筆數 */
				srAccumRec->lnRedeemSaleCount --;
				srAccumRec->lnRedeemTotalCount --;
				srAccumRec->lnTotalSaleCount --;
                                /* 【需求單-113172】免簽商店存根聯參數及短簽單調整需求 by Russell 2025/5/15 下午 2:41 */
                                if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE)
                                {
                                        srAccumRec->lnTotalSale_NoSignCount--;
                                }
                                else
                                {
                                        srAccumRec->lnTotalSale_SignCount--;
                                }
				srAccumRec->lnTotalCount --;
				/* 計算金額 */
				srAccumRec->llRedeemSaleAmount -= pobTran->srBRec.lnTxnAmount;
				srAccumRec->llRedeemTotalAmount -= pobTran->srBRec.lnTxnAmount;
				srAccumRec->llTotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE)
                                {
                                        srAccumRec->llTotalSale_NoSignAmount -= pobTran->srBRec.lnTxnAmount;
                                }
                                else
                                {
                                        srAccumRec->llTotalSale_SignAmount -= pobTran->srBRec.lnTxnAmount;
                                }
				srAccumRec->llTotalAmount -= pobTran->srBRec.lnTxnAmount;
				/* 計算總扣抵點數 */
				srAccumRec->lnRedeemTotalPoint -= pobTran->srBRec.lnRedemptionPoints;
				break;
                        default:
                                /* 交易類別錯誤，(沒有此交易類別) */
                                /* debug */
                                if (ginDebug == VS_TRUE)
                                {
                                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                        sprintf(szErrorMsg, "inACCUM_UpdateTotalAmount ERROR!!");
                                        inLogPrintf(AT, szErrorMsg);

                                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                        sprintf(szErrorMsg, "OrgTrans code = %d", pobTran->srBRec.inOrgCode);
                                        inLogPrintf(AT, szErrorMsg);
                                }

                                return (VS_ERROR);
                }
        }

        /* inACCUM_UpdateTotalAmount()_END */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inACCUM_UpdateTotalAmount()_END");
        }

        return (VS_SUCCESS);
}

/*
Function        :inACCUM_UpdateTotalAmountByCard
Date&Time       :2015/9/4 下午 15:00
Describe        :每當有交易時，到此fuction更新"卡別"的筆數和總額，回傳(VS_SUCCESS) or (VS_ERROR)
*/
int inACCUM_UpdateTotalAmountByCard(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec)
{
	int		inRetVal = VS_SUCCESS;
        char            szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

        /* inACCUM_UpdateTotalAmountByCard()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inACCUM_UpdateTotalAmountByCard()_START");
        }

        /* 不是取消的交易 */
        if (pobTran->srBRec.uszVOIDBit == VS_FALSE)
        {
                switch (pobTran->srBRec.inCode)
                {
                        case _SALE_:
                        case _SALE_OFFLINE_:/* Offline Sale */
                        case _INST_SALE_:
                        case _REDEEM_SALE_:
			case _CASH_ADVANCE_ :
			case _FORCE_CASH_ADVANCE_ :
			case _PRE_COMP_ :
			case _CUP_SALE_ :
			case _CUP_PRE_COMP_ :
			case _FISC_SALE_ :
			case _INST_ADJUST_ :
			case _REDEEM_ADJUST_ :
			case _MAIL_ORDER_ :
			case _CUP_MAIL_ORDER_ :
                                if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_VISA_, 4))
                                {
                                        /* 計算筆數 */
                                        srAccumRec->lnVisaTotalSaleCount ++;
                                        srAccumRec->lnVisaTotalCount ++;
                                        /* 計算金額 */
                                        srAccumRec->llVisaTotalSaleAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llVisaTotalAmount += pobTran->srBRec.lnTxnAmount;
                                }
                                else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_MASTERCARD_, 10))
                                {
                                        /* 計算筆數 */
                                        srAccumRec->lnMasterTotalSaleCount ++;
                                        srAccumRec->lnMasterTotalCount ++;
                                        /* 計算金額 */
                                        srAccumRec->llMasterTotalSaleAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llMasterTotalAmount += pobTran->srBRec.lnTxnAmount;
                                }
                                else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_JCB_, 3))
                                {
                                        /* 計算筆數 */
                                        srAccumRec->lnJcbTotalSaleCount ++;
                                        srAccumRec->lnJcbTotalCount ++;
                                        /* 計算金額 */
                                        srAccumRec->llJcbTotalSaleAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llJcbTotalAmount += pobTran->srBRec.lnTxnAmount;
                                }
				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
                                else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_AMEX_, 4))
                                {
                                        /* 計算筆數 */
                                        srAccumRec->lnAmexTotalSaleCount ++;
                                        srAccumRec->lnAmexTotalCount ++;
                                        /* 計算金額 */
                                        srAccumRec->llAmexTotalSaleAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llAmexTotalAmount += pobTran->srBRec.lnTxnAmount;
                                }
                                else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_CUP_, 3))
                                {
                                        /* 計算筆數 */
                                        srAccumRec->lnCupTotalSaleCount ++;
                                        srAccumRec->lnCupTotalCount ++;
                                        /* 計算金額 */
                                        srAccumRec->llCupTotalSaleAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llCupTotalAmount += pobTran->srBRec.lnTxnAmount;
                                }
                                else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_DINERS_, 6) ||
					 pobTran->srBRec.uszUCARDTransBit == VS_TRUE)
                                {
					if (pobTran->srBRec.uszUCARDTransBit == VS_TRUE)
					{
						/* 計算筆數 */
						srAccumRec->lnUCardTotalSaleCount ++;
						srAccumRec->lnUCardTotalCount ++;
						/* 計算金額 */
						srAccumRec->llUCardTotalSaleAmount += pobTran->srBRec.lnTxnAmount;
						srAccumRec->llUCardTotalAmount += pobTran->srBRec.lnTxnAmount;
					}
					else
					{
						/* 計算筆數 */
						srAccumRec->lnDinersTotalSaleCount ++;
						srAccumRec->lnDinersTotalCount ++;
						/* 計算金額 */
						srAccumRec->llDinersTotalSaleAmount += pobTran->srBRec.lnTxnAmount;
						srAccumRec->llDinersTotalAmount += pobTran->srBRec.lnTxnAmount;
					}
                                }
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_SMARTPAY_, 8))
                                {
                                        /* 計算筆數 */
                                        srAccumRec->lnFiscTotalSaleCount ++;
                                        srAccumRec->lnFiscTotalCount ++;
                                        /* 計算金額 */
                                        srAccumRec->llFiscTotalSaleAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llFiscTotalAmount += pobTran->srBRec.lnTxnAmount;
                                }
                                else
                                {
					/* 【需求單 - 108097】Card BIN由ATS查核 add by LingHsiung 2020/2/3 下午 5:19 */
					/* 新增未知卡別小計 add by LingHsiung 2020/7/7 下午 2:26 */
					inRetVal = inACCUM_UpdateTotalAmountByUnknownCard(pobTran, srAccumRec);
					if (inRetVal != VS_SUCCESS)
					{
						return (VS_ERROR);
					}
                                }

                                break;
                        case _REFUND_:
			case _CUP_REFUND_:
			case _CUP_MAIL_ORDER_REFUND_ :
                        case _INST_REFUND_:
                        case _REDEEM_REFUND_:
			case _FISC_REFUND_ :
                                if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_VISA_, 4))
                                {
                                        /* 計算筆數 */
                                        srAccumRec->lnVisaTotalRefundCount ++;
                                        srAccumRec->lnVisaTotalCount ++;
                                        /* 計算金額 */
                                        srAccumRec->llVisaTotalRefundAmount += (0 - pobTran->srBRec.lnTxnAmount);
                                        srAccumRec->llVisaTotalAmount += (0 - pobTran->srBRec.lnTxnAmount);
                                }
                                else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_MASTERCARD_, 10))
                                {
                                        /* 計算筆數 */
                                        srAccumRec->lnMasterTotalRefundCount ++;
                                        srAccumRec->lnMasterTotalCount ++;
                                        /* 計算金額 */
                                        srAccumRec->llMasterTotalRefundAmount += (0 - pobTran->srBRec.lnTxnAmount);
                                        srAccumRec->llMasterTotalAmount += (0 - pobTran->srBRec.lnTxnAmount);
                                }
                                else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_JCB_, 3))
                                {
                                        /* 計算筆數 */
                                        srAccumRec->lnJcbTotalRefundCount ++;
                                        srAccumRec->lnJcbTotalCount ++;
                                        /* 計算金額 */
                                        srAccumRec->llJcbTotalRefundAmount += (0 - pobTran->srBRec.lnTxnAmount);
                                        srAccumRec->llJcbTotalAmount += (0 - pobTran->srBRec.lnTxnAmount);
                                }
				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
                                else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_AMEX_, 4))
                                {
                                        /* 計算筆數 */
                                        srAccumRec->lnAmexTotalRefundCount ++;
                                        srAccumRec->lnAmexTotalCount ++;
                                        /* 計算金額 */
                                        srAccumRec->llAmexTotalRefundAmount += (0 - pobTran->srBRec.lnTxnAmount);
                                        srAccumRec->llAmexTotalAmount += (0 - pobTran->srBRec.lnTxnAmount);
                                }
                                else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_CUP_, 3))
                                {
                                        /* 計算筆數 */
                                        srAccumRec->lnCupTotalRefundCount ++;
                                        srAccumRec->lnCupTotalCount ++;
                                        /* 計算金額 */
                                        srAccumRec->llCupTotalRefundAmount += (0 - pobTran->srBRec.lnTxnAmount);
                                        srAccumRec->llCupTotalAmount += (0 - pobTran->srBRec.lnTxnAmount);
                                }
                                else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_DINERS_, 6) ||
					 pobTran->srBRec.uszUCARDTransBit == VS_TRUE)
                                {
					if (pobTran->srBRec.uszUCARDTransBit == VS_TRUE)
					{
						/* 計算筆數 */
						srAccumRec->lnUCardTotalRefundCount ++;
						srAccumRec->lnUCardTotalCount ++;
						/* 計算金額 */
						srAccumRec->llUCardTotalRefundAmount += (0 - pobTran->srBRec.lnTxnAmount);
						srAccumRec->llUCardTotalAmount += (0 - pobTran->srBRec.lnTxnAmount);
					}
					else
					{
						/* 計算筆數 */
						srAccumRec->lnDinersTotalRefundCount ++;
						srAccumRec->lnDinersTotalCount ++;
						/* 計算金額 */
						srAccumRec->llDinersTotalRefundAmount += (0 - pobTran->srBRec.lnTxnAmount);
						srAccumRec->llDinersTotalAmount += (0 - pobTran->srBRec.lnTxnAmount);
					}
                                }
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_SMARTPAY_, 8))
                                {
                                        /* 計算筆數 */
                                        srAccumRec->lnFiscTotalRefundCount ++;
                                        srAccumRec->lnFiscTotalCount ++;
                                        /* 計算金額 */
                                        srAccumRec->llFiscTotalRefundAmount += (0 - pobTran->srBRec.lnTxnAmount);
                                        srAccumRec->llFiscTotalAmount += (0 - pobTran->srBRec.lnTxnAmount);
                                }
                                else
                                {
					/* 【需求單 - 108097】Card BIN由ATS查核 add by LingHsiung 2020/2/3 下午 5:19 */
					/* 新增未知卡別小計 add by LingHsiung 2020/7/7 下午 2:26 */
					inRetVal = inACCUM_UpdateTotalAmountByUnknownCard(pobTran, srAccumRec);
					if (inRetVal != VS_SUCCESS)
					{
						return (VS_ERROR);
					}
                                }

                                break;
                        case _TIP_:
                                if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_VISA_, 4))
                                {
                                        /* 計算筆數 */
                                        srAccumRec->lnVisaTotalTipsCount++;
                                        /* 計算金額 */
                                        srAccumRec->llVisaTotalTipsAmount += pobTran->srBRec.lnTipTxnAmount;
                                        srAccumRec->llVisaTotalAmount += pobTran->srBRec.lnTipTxnAmount;
                                }
                                else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_MASTERCARD_, 10))
                                {
                                        /* 計算筆數 */
                                        srAccumRec->lnMasterTotalTipsCount++;
                                        /* 計算金額 */
                                        srAccumRec->llMasterTotalTipsAmount += pobTran->srBRec.lnTipTxnAmount;
                                        srAccumRec->llMasterTotalAmount += pobTran->srBRec.lnTipTxnAmount;
                                }
                                else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_JCB_, 3))
                                {
                                        /* 計算筆數 */
                                        srAccumRec->lnJcbTotalTipsCount++;
                                        /* 計算金額 */
                                        srAccumRec->llJcbTotalTipsAmount += pobTran->srBRec.lnTipTxnAmount;
                                        srAccumRec->llJcbTotalAmount += pobTran->srBRec.lnTipTxnAmount;
                                }
				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
                                else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_AMEX_, 4))
                                {
                                        /* 計算筆數 */
                                        srAccumRec->lnAmexTotalTipsCount++;
                                        /* 計算金額 */
                                        srAccumRec->llAmexTotalTipsAmount += pobTran->srBRec.lnTipTxnAmount;
                                        srAccumRec->llAmexTotalAmount += pobTran->srBRec.lnTipTxnAmount;
                                }
                                else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_CUP_, 3))
                                {
                                        /* 計算筆數 */
                                        srAccumRec->lnCupTotalTipsCount++;
                                        /* 計算金額 */
                                        srAccumRec->llCupTotalTipsAmount += pobTran->srBRec.lnTipTxnAmount;
                                        srAccumRec->llCupTotalAmount += pobTran->srBRec.lnTipTxnAmount;
                                }
                                else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_DINERS_, 6) ||
					 pobTran->srBRec.uszUCARDTransBit == VS_TRUE)
                                {
					if (pobTran->srBRec.uszUCARDTransBit == VS_TRUE)
					{
						/* 計算筆數 */
						srAccumRec->lnUCardTotalTipsCount++;
						/* 計算金額 */
						srAccumRec->llUCardTotalTipsAmount += pobTran->srBRec.lnTipTxnAmount;
						srAccumRec->llUCardTotalAmount += pobTran->srBRec.lnTipTxnAmount;
					}
					else
					{
						/* 計算筆數 */
						srAccumRec->lnDinersTotalTipsCount++;
						/* 計算金額 */
						srAccumRec->llDinersTotalTipsAmount += pobTran->srBRec.lnTipTxnAmount;
						srAccumRec->llDinersTotalAmount += pobTran->srBRec.lnTipTxnAmount;
					}
                                }
                                else
                                {
					/* 修改Checkbin卡別空白，取消會鎖機的Bug 2022/5/9 下午 2:41 */
					inRetVal = inACCUM_UpdateTotalAmountByUnknownCard(pobTran, srAccumRec);
					if (inRetVal != VS_SUCCESS)
					{
						return (VS_ERROR);
					}
                                }

                                break;
                        case _ADJUST_:
                                if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_VISA_, 4))
                                {
                                        /* 計算金額 */
                                        srAccumRec->llVisaTotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llVisaTotalAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llVisaTotalSaleAmount += pobTran->srBRec.lnAdjustTxnAmount;
                                        srAccumRec->llVisaTotalAmount += pobTran->srBRec.lnAdjustTxnAmount;
                                }
                                else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_MASTERCARD_, 10))
                                {
                                        /* 計算金額 */
                                        srAccumRec->llMasterTotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llMasterTotalAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llMasterTotalSaleAmount += pobTran->srBRec.lnAdjustTxnAmount;
                                        srAccumRec->llMasterTotalAmount += pobTran->srBRec.lnAdjustTxnAmount;
                                }
                                else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_JCB_, 3))
                                {
                                        /* 計算金額 */
                                        srAccumRec->llJcbTotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llJcbTotalAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llJcbTotalSaleAmount += pobTran->srBRec.lnAdjustTxnAmount;
                                        srAccumRec->llJcbTotalAmount += pobTran->srBRec.lnAdjustTxnAmount;
                                }
				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
                                else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_AMEX_, 4))
                                {
                                        /* 計算金額 */
                                        srAccumRec->llAmexTotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llAmexTotalAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llAmexTotalSaleAmount += pobTran->srBRec.lnAdjustTxnAmount;
                                        srAccumRec->llAmexTotalAmount += pobTran->srBRec.lnAdjustTxnAmount;
                                }
                                else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_CUP_, 3))
                                {
                                        /* 計算金額 */
                                        srAccumRec->llCupTotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llCupTotalAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llCupTotalSaleAmount += pobTran->srBRec.lnAdjustTxnAmount;
                                        srAccumRec->llCupTotalAmount += pobTran->srBRec.lnAdjustTxnAmount;
                                }
                                else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_DINERS_, 6) ||
					 pobTran->srBRec.uszUCARDTransBit == VS_TRUE)
                                {
					if (pobTran->srBRec.uszUCARDTransBit == VS_TRUE)
					{
						/* 計算金額 */
						srAccumRec->llUCardTotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
						srAccumRec->llUCardTotalAmount -= pobTran->srBRec.lnTxnAmount;
						srAccumRec->llUCardTotalSaleAmount += pobTran->srBRec.lnAdjustTxnAmount;
						srAccumRec->llUCardTotalAmount += pobTran->srBRec.lnAdjustTxnAmount;
					}
					else
					{
						/* 計算金額 */
						srAccumRec->llDinersTotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
						srAccumRec->llDinersTotalAmount -= pobTran->srBRec.lnTxnAmount;
						srAccumRec->llDinersTotalSaleAmount += pobTran->srBRec.lnAdjustTxnAmount;
						srAccumRec->llDinersTotalAmount += pobTran->srBRec.lnAdjustTxnAmount;
					}
                                }
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_SMARTPAY_, 8))
                                {
                                        /* 計算金額 */
                                        srAccumRec->llFiscTotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llFiscTotalAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llFiscTotalSaleAmount += pobTran->srBRec.lnAdjustTxnAmount;
                                        srAccumRec->llFiscTotalAmount += pobTran->srBRec.lnAdjustTxnAmount;
                                }
                                else
                                {
					/* 修改Checkbin卡別空白，取消會鎖機的Bug 2022/5/9 下午 2:41 */
					inRetVal = inACCUM_UpdateTotalAmountByUnknownCard(pobTran, srAccumRec);
					if (inRetVal != VS_SUCCESS)
					{
						return (VS_ERROR);
					}
                                }

                                break;
                        default :
                                /* 交易類別錯誤，(沒有此交易類別) */
                                /* debug */
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
				sprintf(szErrorMsg, "inACCUM_UpdateTotalAmountByCard : ERROR!! Trans code = %d", pobTran->srBRec.inCode);
				/* 鎖機記Log */
				inUtility_StoreTraceLog_OneStep(szErrorMsg);
				
				if (ginDebug == VS_TRUE)
					inLogPrintf(AT, szErrorMsg);

                                return (VS_ERROR);
                }
        }
        /* 取消的時候 */
        else
        {
                switch (pobTran->srBRec.inOrgCode)
                {
                        case _SALE_:
                        case _SALE_OFFLINE_:/* Offline Sale */
                        case _INST_SALE_:
                        case _REDEEM_SALE_:
			case _CASH_ADVANCE_ :
			case _FORCE_CASH_ADVANCE_ :
			case _PRE_COMP_ :
			case _CUP_SALE_ :
			case _CUP_PRE_COMP_ :
			case _FISC_SALE_ :
			case _INST_ADJUST_ :
			case _REDEEM_ADJUST_ :
			case _MAIL_ORDER_ :
			case _CUP_MAIL_ORDER_ :
                                if (!memcmp(&pobTran->srBRec.szCardLabel, _CARD_TYPE_VISA_, 4))
                                {
                                        /* 計算筆數 */
                                        srAccumRec->lnVisaTotalSaleCount --;
                                        srAccumRec->lnVisaTotalCount --;
                                        /* 計算金額 */
                                        srAccumRec->llVisaTotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llVisaTotalAmount -= pobTran->srBRec.lnTxnAmount;
                                }
                                else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_MASTERCARD_, 10))
                                {
                                        /* 計算筆數 */
                                        srAccumRec->lnMasterTotalSaleCount --;
                                        srAccumRec->lnMasterTotalCount --;
                                        /* 計算金額 */
                                        srAccumRec->llMasterTotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llMasterTotalAmount -= pobTran->srBRec.lnTxnAmount;
                                }
                                else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_JCB_, 3))
                                {
                                        /* 計算筆數 */
                                        srAccumRec->lnJcbTotalSaleCount --;
                                        srAccumRec->lnJcbTotalCount --;
                                        /* 計算金額 */
                                        srAccumRec->llJcbTotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llJcbTotalAmount -= pobTran->srBRec.lnTxnAmount;
                                }
				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
                                else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_AMEX_, 4))
                                {
                                        /* 計算筆數 */
                                        srAccumRec->lnAmexTotalSaleCount --;
                                        srAccumRec->lnAmexTotalCount --;
                                        /* 計算金額 */
                                        srAccumRec->llAmexTotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llAmexTotalAmount -= pobTran->srBRec.lnTxnAmount;
                                }
                                else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_CUP_, 3))
                                {
                                        /* 計算筆數 */
                                        srAccumRec->lnCupTotalSaleCount --;
                                        srAccumRec->lnCupTotalCount --;
                                        /* 計算金額 */
                                        srAccumRec->llCupTotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llCupTotalAmount -= pobTran->srBRec.lnTxnAmount;
                                }
                                else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_DINERS_, 6) ||
					 pobTran->srBRec.uszUCARDTransBit == VS_TRUE)
                                {
					if (pobTran->srBRec.uszUCARDTransBit == VS_TRUE)
					{
						/* 計算筆數 */
						srAccumRec->lnUCardTotalSaleCount --;
						srAccumRec->lnUCardTotalCount --;
						/* 計算金額 */
						srAccumRec->llUCardTotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
						srAccumRec->llUCardTotalAmount -= pobTran->srBRec.lnTxnAmount;
					}
					else
					{
						/* 計算筆數 */
						srAccumRec->lnDinersTotalSaleCount --;
						srAccumRec->lnDinersTotalCount --;
						/* 計算金額 */
						srAccumRec->llDinersTotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
						srAccumRec->llDinersTotalAmount -= pobTran->srBRec.lnTxnAmount;
					}
                                }
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_SMARTPAY_, 8))
                                {
                                        /* 計算筆數 */
                                        srAccumRec->lnFiscTotalSaleCount --;
                                        srAccumRec->lnFiscTotalCount --;
                                        /* 計算金額 */
                                        srAccumRec->llFiscTotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llFiscTotalAmount -= pobTran->srBRec.lnTxnAmount;
                                }
                                else
                                {
					/* 修改Checkbin卡別空白，取消會鎖機的Bug 2022/5/9 下午 2:41 */
					inRetVal = inACCUM_UpdateTotalAmountByUnknownCard(pobTran, srAccumRec);
					if (inRetVal != VS_SUCCESS)
					{
						return (VS_ERROR);
					}
                                }

                                break;
                        case _REFUND_:
			case _CUP_REFUND_:
			case _CUP_MAIL_ORDER_REFUND_ :
                        case _INST_REFUND_:
                        case _REDEEM_REFUND_:
			/* SmartPay退貨不能取消 */
                                if (!memcmp(&pobTran->srBRec.szCardLabel, _CARD_TYPE_VISA_, 4))
                                {
                                        /* 計算筆數 */
                                        srAccumRec->lnVisaTotalRefundCount --;
                                        srAccumRec->lnVisaTotalCount --;
                                        /* 計算金額 */
                                        srAccumRec->llVisaTotalRefundAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llVisaTotalAmount += pobTran->srBRec.lnTxnAmount;
                                }
                                else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_MASTERCARD_, 10))
                                {
                                        /* 計算筆數 */
                                        srAccumRec->lnMasterTotalRefundCount --;
                                        srAccumRec->lnMasterTotalCount --;
                                        /* 計算金額 */
                                        srAccumRec->llMasterTotalRefundAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llMasterTotalAmount += pobTran->srBRec.lnTxnAmount;
                                }
                                else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_JCB_, 3))
                                {
                                        /* 計算筆數 */
                                        srAccumRec->lnJcbTotalRefundCount --;
                                        srAccumRec->lnJcbTotalCount --;
                                        /* 計算金額 */
                                        srAccumRec->llJcbTotalRefundAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llJcbTotalAmount += pobTran->srBRec.lnTxnAmount;
                                }
				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
                                else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_AMEX_, 4))
                                {
                                        /* 計算筆數 */
                                        srAccumRec->lnAmexTotalRefundCount --;
                                        srAccumRec->lnAmexTotalCount --;
                                        /* 計算金額 */
                                        srAccumRec->llAmexTotalRefundAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llAmexTotalAmount += pobTran->srBRec.lnTxnAmount;
                                }
                                else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_CUP_, 3))
                                {
                                        /* 計算筆數 */
                                        srAccumRec->lnCupTotalRefundCount --;
                                        srAccumRec->lnCupTotalCount --;
                                        /* 計算金額 */
                                        srAccumRec->llCupTotalRefundAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llCupTotalAmount += pobTran->srBRec.lnTxnAmount;
                                }
                                else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_DINERS_, 6) ||
					 pobTran->srBRec.uszUCARDTransBit == VS_TRUE)
                                {
					if (pobTran->srBRec.uszUCARDTransBit == VS_TRUE)
					{
						/* 計算筆數 */
						srAccumRec->lnUCardTotalRefundCount --;
						srAccumRec->lnUCardTotalCount --;
						/* 計算金額 */
						srAccumRec->llUCardTotalRefundAmount += pobTran->srBRec.lnTxnAmount;
						srAccumRec->llUCardTotalAmount += pobTran->srBRec.lnTxnAmount;
					}
					else
					{
						/* 計算筆數 */
						/* 計算筆數 */
						srAccumRec->lnDinersTotalRefundCount --;
						srAccumRec->lnDinersTotalCount --;
						/* 計算金額 */
						srAccumRec->llDinersTotalRefundAmount += pobTran->srBRec.lnTxnAmount;
						srAccumRec->llDinersTotalAmount += pobTran->srBRec.lnTxnAmount;
					}
                                }
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_SMARTPAY_, 8))
                                {
                                        /* 計算筆數 */
                                        /* 計算筆數 */
                                        srAccumRec->lnFiscTotalRefundCount --;
                                        srAccumRec->lnFiscTotalCount --;
                                        /* 計算金額 */
                                        srAccumRec->llFiscTotalRefundAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llFiscTotalAmount += pobTran->srBRec.lnTxnAmount;
                                }
                                else
                                {
					/* 修改Checkbin卡別空白，取消會鎖機的Bug 2022/5/9 下午 2:41 */
					inRetVal = inACCUM_UpdateTotalAmountByUnknownCard(pobTran, srAccumRec);
					if (inRetVal != VS_SUCCESS)
					{
						return (VS_ERROR);
					}
                                }

                                break;
                        case _TIP_:
                                if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_VISA_, 4))
                                {
                                        /* 取消整筆交易，計算筆數 */
                                        srAccumRec->lnVisaTotalTipsCount --;
                                        srAccumRec->lnVisaTotalSaleCount --;
                                        srAccumRec->lnVisaTotalCount --;
                                        /* 取消整筆交易，計算金額 */
                                        srAccumRec->llVisaTotalTipsAmount -= pobTran->srBRec.lnTipTxnAmount;
                                        srAccumRec->llVisaTotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llVisaTotalAmount -= (pobTran->srBRec.lnTipTxnAmount + pobTran->srBRec.lnTxnAmount);
                                }
                                else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_MASTERCARD_, 10))
                                {
                                        /* 取消整筆交易，計算筆數 */
                                        srAccumRec->lnMasterTotalTipsCount --;
                                        srAccumRec->lnMasterTotalSaleCount --;
                                        srAccumRec->lnMasterTotalCount --;
                                        /* 取消整筆交易，計算金額 */
                                        srAccumRec->llMasterTotalTipsAmount -= pobTran->srBRec.lnTipTxnAmount;
                                        srAccumRec->llMasterTotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llMasterTotalAmount -= (pobTran->srBRec.lnTipTxnAmount + pobTran->srBRec.lnTxnAmount);
                                }
                                else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_JCB_, 3))
                                {
                                        /* 取消整筆交易，計算筆數 */
                                        srAccumRec->lnJcbTotalTipsCount --;
                                        srAccumRec->lnJcbTotalSaleCount --;
                                        srAccumRec->lnJcbTotalCount --;
                                        /* 取消整筆交易，計算金額 */
                                        srAccumRec->llJcbTotalTipsAmount -= pobTran->srBRec.lnTipTxnAmount;
                                        srAccumRec->llJcbTotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llJcbTotalAmount -= (pobTran->srBRec.lnTipTxnAmount + pobTran->srBRec.lnTxnAmount);
                                }
				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
                                else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_AMEX_, 4))
                                {
                                        /* 取消整筆交易，計算筆數 */
                                        srAccumRec->lnAmexTotalTipsCount --;
                                        srAccumRec->lnAmexTotalSaleCount --;
                                        srAccumRec->lnAmexTotalCount --;
                                        /* 取消整筆交易，計算金額 */
                                        srAccumRec->llAmexTotalTipsAmount -= pobTran->srBRec.lnTipTxnAmount;
                                        srAccumRec->llAmexTotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llAmexTotalAmount -= (pobTran->srBRec.lnTipTxnAmount + pobTran->srBRec.lnTxnAmount);
                                }
                                else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_CUP_, 3))
                                {
                                        /* 取消整筆交易，計算筆數 */
                                        srAccumRec->lnCupTotalTipsCount --;
                                        srAccumRec->lnCupTotalSaleCount --;
                                        srAccumRec->lnCupTotalCount --;
                                        /* 取消整筆交易，計算金額 */
                                        srAccumRec->llCupTotalTipsAmount -= pobTran->srBRec.lnTipTxnAmount;
                                        srAccumRec->llCupTotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llCupTotalAmount -= (pobTran->srBRec.lnTipTxnAmount + pobTran->srBRec.lnTxnAmount);
                                }
                                else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_DINERS_, 6) ||
					 pobTran->srBRec.uszUCARDTransBit == VS_TRUE)
                                {
					if (pobTran->srBRec.uszUCARDTransBit == VS_TRUE)
					{
						/* 取消整筆交易，計算筆數 */
						srAccumRec->lnUCardTotalTipsCount --;
						srAccumRec->lnUCardTotalSaleCount --;
						srAccumRec->lnUCardTotalCount --;
						/* 取消整筆交易，計算金額 */
						srAccumRec->llUCardTotalTipsAmount -= pobTran->srBRec.lnTipTxnAmount;
						srAccumRec->llUCardTotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
						srAccumRec->llUCardTotalAmount -= (pobTran->srBRec.lnTipTxnAmount + pobTran->srBRec.lnTxnAmount);
					}
					else
					{
						/* 計算筆數 */
						srAccumRec->lnDinersTotalTipsCount --;
						srAccumRec->lnDinersTotalSaleCount --;
						srAccumRec->lnDinersTotalCount --;
						/* 計算金額 */
						srAccumRec->llDinersTotalTipsAmount -= pobTran->srBRec.lnTipTxnAmount;
						srAccumRec->llDinersTotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
						srAccumRec->llDinersTotalAmount -= (pobTran->srBRec.lnTipTxnAmount + pobTran->srBRec.lnTxnAmount);
					}
                                }
                                else
                                {
					/* 修改Checkbin卡別空白，取消會鎖機的Bug 2022/5/9 下午 2:41 */
					inRetVal = inACCUM_UpdateTotalAmountByUnknownCard(pobTran, srAccumRec);
					if (inRetVal != VS_SUCCESS)
					{
						return (VS_ERROR);
					}
                                }

                                break;
                        case _ADJUST_:
                                if (!memcmp(&pobTran->srBRec.szCardLabel, _CARD_TYPE_VISA_, 4))
                                {
                                        /* 計算金額 */
                                        srAccumRec->llVisaTotalSaleAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llVisaTotalAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llVisaTotalSaleAmount += (0 - pobTran->srBRec.lnAdjustTxnAmount);
                                        srAccumRec->llVisaTotalAmount += (0 - pobTran->srBRec.lnAdjustTxnAmount);


                                }
                                else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_MASTERCARD_, 10))
                                {
                                        /* 計算金額 */
                                        srAccumRec->llMasterTotalSaleAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llMasterTotalAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llMasterTotalSaleAmount += (0 - pobTran->srBRec.lnAdjustTxnAmount);
                                        srAccumRec->llMasterTotalAmount += (0 - pobTran->srBRec.lnAdjustTxnAmount);

                                }
                                else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_JCB_, 3))
                                {
                                        /* 計算金額 */
                                        srAccumRec->llJcbTotalSaleAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llJcbTotalAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llJcbTotalSaleAmount += (0 - pobTran->srBRec.lnAdjustTxnAmount);
                                        srAccumRec->llJcbTotalAmount += (0 - pobTran->srBRec.lnAdjustTxnAmount);


                                }
				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
                                else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_AMEX_, 4))
                                {
                                        /* 計算金額 */
                                        srAccumRec->llAmexTotalSaleAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llAmexTotalAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llAmexTotalSaleAmount += (0 - pobTran->srBRec.lnAdjustTxnAmount);
                                        srAccumRec->llAmexTotalAmount += (0 - pobTran->srBRec.lnAdjustTxnAmount);

                                }
                                else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_CUP_, 3))
                                {
                                        /* 計算金額 */
                                        srAccumRec->llCupTotalSaleAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llCupTotalAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llCupTotalSaleAmount += (0 - pobTran->srBRec.lnAdjustTxnAmount);
                                        srAccumRec->llCupTotalAmount += (0 - pobTran->srBRec.lnAdjustTxnAmount);
                                }
                                else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_DINERS_, 6) ||
					 pobTran->srBRec.uszUCARDTransBit == VS_TRUE)
                                {
					if (pobTran->srBRec.uszUCARDTransBit == VS_TRUE)
					{
						/* 計算金額 */
						srAccumRec->llUCardTotalSaleAmount += pobTran->srBRec.lnTxnAmount;
						srAccumRec->llUCardTotalAmount += pobTran->srBRec.lnTxnAmount;
						srAccumRec->llUCardTotalSaleAmount += (0 - pobTran->srBRec.lnAdjustTxnAmount);
						srAccumRec->llUCardTotalAmount += (0 - pobTran->srBRec.lnAdjustTxnAmount);
					}
					else
					{
						/* 計算金額 */
						srAccumRec->llDinersTotalSaleAmount += pobTran->srBRec.lnTxnAmount;
						srAccumRec->llDinersTotalAmount += pobTran->srBRec.lnTxnAmount;
						srAccumRec->llDinersTotalSaleAmount += (0 - pobTran->srBRec.lnAdjustTxnAmount);
						srAccumRec->llDinersTotalAmount += (0 - pobTran->srBRec.lnAdjustTxnAmount);
					}
                                }
				else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_SMARTPAY_, 8))
                                {
                                        /* 計算金額 */
                                        srAccumRec->llFiscTotalSaleAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llFiscTotalAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llFiscTotalSaleAmount += (0 - pobTran->srBRec.lnAdjustTxnAmount);
                                        srAccumRec->llFiscTotalAmount += (0 - pobTran->srBRec.lnAdjustTxnAmount);
                                }
                                else
                                {
					/* 修改Checkbin卡別空白，取消會鎖機的Bug 2022/5/9 下午 2:41 */
					inRetVal = inACCUM_UpdateTotalAmountByUnknownCard(pobTran, srAccumRec);
					if (inRetVal != VS_SUCCESS)
					{
						return (VS_ERROR);
					}
                                }

                                break;
                        default:
                                /* 交易類別錯誤，(沒有此交易類別) */
                                /* debug */
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
				sprintf(szErrorMsg, "inACCUM_UpdateTotalAmountByCard : ERROR!!");
				/* 鎖機記Log */
				inUtility_StoreTraceLog_OneStep(szErrorMsg);
				
				if (ginDebug == VS_TRUE)
                                        inLogPrintf(AT, szErrorMsg);

				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
				sprintf(szErrorMsg, "OrgTrans code = %d", pobTran->srBRec.inOrgCode);
				inUtility_StoreTraceLog_OneStep(szErrorMsg);
				
				if (ginDebug == VS_TRUE)
                                        inLogPrintf(AT, szErrorMsg);

                                return (VS_ERROR);
                }
        }

        /* inACCUM_UpdateTotalAmountByCard()_END */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inACCUM_UpdateTotalAmountByCard()_END");
        }

        return (VS_SUCCESS);
}

/*
Function        :inACCUM_StoreRecord
Date&Time       :2015/9/4 下午 15:30
Describe        :再做加總後，將資料存回record，回傳(VS_SUCCESS) or (VS_ERROR)
*/
int inACCUM_StoreRecord(ACCUM_TOTAL_REC *srAccumRec,unsigned char *uszFileName)
{
        unsigned long   ulHandlePtr;    /* File Handle，type為pointer */
        int inRetVal; /* return value，來判斷是否回傳error */

        /* inACCUM_StoreRecord()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inACCUM_StoreRecord()_START");
        }

        /*開檔*/
        inRetVal = inFILE_Open(&ulHandlePtr, uszFileName);
        /*開檔成功*/
        if (inRetVal != VS_SUCCESS)
        {
                /* 開檔失敗時，不必關檔(開檔失敗，handle回傳NULL) */
                /* 開檔失敗，所以回傳error */
                return(VS_ERROR);
        }

        /* 把指針指到開頭*/
        inRetVal = inFILE_Seek(ulHandlePtr, 0, _SEEK_BEGIN_);

        /* seek不成功時 */
        if (inRetVal != VS_SUCCESS)
        {
                /* inFILE_Seek失敗時 */
                /* 關檔並回傳VS_ERROR */
                inFILE_Close(&ulHandlePtr);
                /* seek失敗，所以回傳error。(關檔不論成功與否都要回傳(VS_ERROR)) */
                return (VS_ERROR);
        }

        /* 寫檔 */
        if (inFILE_Write(&ulHandlePtr,(unsigned char*)srAccumRec,_ACCUM_REC_SIZE_) != VS_SUCCESS)
        {
                /*寫檔失敗時*/
                /* 關檔  */
                inFILE_Close(&ulHandlePtr);
                /* 寫檔失敗，所以回傳error。(關檔不論成功與否都要回傳(VS_ERROR)) */
                return (VS_ERROR);
        }

        /* 關檔  */
        if (inFILE_Close(&ulHandlePtr) != VS_SUCCESS)
        {
                /*關檔失敗時*/
                /* 關檔失敗，所以回傳error */
                return (VS_ERROR);
        }

        /* inACCUM_StoreRecord()_END */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inACCUM_StoreRecord()_END");
        }

        return (VS_SUCCESS);
}

/*
Function        :inACCUM_GetRecord
Date&Time       :2015/9/4 下午 16:00
Describe        :將.amt中的資料讀取到srAccumRec來做使用，回傳(VS_SUCCESS) or (VS_ERROR)
*/
int inACCUM_GetRecord(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec)
{
        unsigned long   ulHandlePtr;				/* File Handle */
        int             inRetVal;                               /* return value，來判斷是否回傳error */
        char            szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
        unsigned char   uszFileName[14 + 1];                    /* 儲存交易金額檔案的檔案名稱(最大為15) */
        
        /* inACCUM_GetRecord()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inACCUM_GetRecord()_START");
        }

	if (inFunc_ComposeFileName(pobTran, (char*)uszFileName, _ACCUM_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

        /* 純開檔不建檔  */
        if (inFILE_OpenReadOnly(&ulHandlePtr, uszFileName) == (VS_ERROR))
        {
                /* 開檔失敗時 */
                /* 開檔錯誤，確認是否有檔案，若有檔案仍錯誤，則可能是handle的問題 */
                if (inFILE_Check_Exist(uszFileName) != (VS_ERROR))
                {
                        /* 開啟失敗，但檔案存在回傳error */
                        return (VS_ERROR);
                }
                /* 如果沒有檔案時，則為沒有交易紀錄，回傳NO_RECORD並印空簽單 */
                else
                {
                        return (VS_NO_RECORD);
                }
		
        }
        /* 開檔成功 */
        else
        {
                /*先清空srAccumRec 為讀檔案作準備  */
                memset(srAccumRec, 0x00, sizeof(srAccumRec));

                /* 把指針指到開頭*/
                inRetVal = inFILE_Seek(ulHandlePtr, 0, _SEEK_BEGIN_);

                /* inFILE_Seek，成功時 */
                if (inRetVal != VS_SUCCESS)
                {
                        /* inFILE_Seek失敗時 */
                        /* 關檔並回傳VS_ERROR */
                        inFILE_Close(&ulHandlePtr);

                        /* Seek檔失敗，所以回傳VS_ERROR。(關檔不論成功與否都要回傳(VS_ERROR)) */
                        return (VS_ERROR);
                }

                /*確認檔案大小*/
                if (lnFILE_GetSize(&ulHandlePtr, uszFileName) == 0)
                {
                        /* 長度為0，不必讀 */
                        /* debug */
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "lnFILE_GetSize ＝＝ 0 ：");
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "(FName = %s), (Ptr = %d)", uszFileName, (int)ulHandlePtr);
                                inLogPrintf(AT, szErrorMsg);
                        }
                        /* 關檔並回傳VS_NO_RECORD */
                        inFILE_Close(&ulHandlePtr);
                        return (VS_NO_RECORD);
                }
                /* Get不到Size的時候 */
                else if (lnFILE_GetSize(&ulHandlePtr, uszFileName) == VS_ERROR)
                {
                        /* 關檔並回傳VS_ERROR */
                        inFILE_Close(&ulHandlePtr);
                        return (VS_ERROR);
                }
                /* 檔案大小大於0(裡面有資料時) */
                else
                {
                        /* 讀檔 */
                        if (inFILE_Read(&ulHandlePtr, (unsigned char *) srAccumRec, _ACCUM_REC_SIZE_) == VS_ERROR)
                        {
                                /* 讀檔失敗就關檔 */
                                inFILE_Close(&ulHandlePtr);
                                /* 讀檔失敗，所以回傳（VS_ERROR）。(關檔不論成功與否都要回傳(VS_ERROR)) */
                                return (VS_ERROR);
                        }
                }

                /* 將檔案關閉 */
                if (inFILE_Close(&ulHandlePtr) == VS_ERROR)
                {
                        /*關檔失敗*/
                        /* 關檔失敗，所以回傳VS_ERROR */
                        return (VS_ERROR);
                }
        }
        /* inACCUM_GetRecord()_END */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inACCUM_GetRecord()_END");
        }

        return (VS_SUCCESS);
}

/*
Function        :inACCUM_GetRecord_ESVC
Date&Time       :2018/1/18 下午 4:54
Describe        :將.amt中的資料讀取到srAccumRec來做使用，回傳(VS_SUCCESS) or (VS_ERROR)
*/
int inACCUM_GetRecord_ESVC(TRANSACTION_OBJECT *pobTran, TICKET_ACCUM_TOTAL_REC *srAccumRec)
{
        unsigned long   ulHandlePtr;				/* File Handle */
        int             inRetVal;                               /* return value，來判斷是否回傳error */
        char            szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
        unsigned char   uszFileName[14 + 1];                    /* 儲存交易金額檔案的檔案名稱(最大為15) */
        
        /* inACCUM_GetRecord()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inACCUM_GetRecord_ESVC()_START");
        }

	if (inFunc_ComposeFileName(pobTran, (char*)uszFileName, _ACCUM_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

        /* 純開檔不建檔  */
        if (inFILE_OpenReadOnly(&ulHandlePtr, uszFileName) == (VS_ERROR))
        {
                /* 開檔失敗時 */
                /* 開檔錯誤，確認是否有檔案，若有檔案仍錯誤，則可能是handle的問題 */
                if (inFILE_Check_Exist(uszFileName) != (VS_ERROR))
                {
                        /* 開啟失敗，但檔案存在回傳error */
                        return (VS_ERROR);
                }
                /* 如果沒有檔案時，則為沒有交易紀錄，回傳NO_RECORD並印空簽單 */
                else
                {
                        return (VS_NO_RECORD);
                }
		
        }
        /* 開檔成功 */
        else
        {
                /*先清空srAccumRec 為讀檔案作準備  */
                memset(srAccumRec, 0x00, sizeof(srAccumRec));

                /* 把指針指到開頭*/
                inRetVal = inFILE_Seek(ulHandlePtr, 0, _SEEK_BEGIN_);

                /* inFILE_Seek，成功時 */
                if (inRetVal != VS_SUCCESS)
                {
                        /* inFILE_Seek失敗時 */
                        /* 關檔並回傳VS_ERROR */
                        inFILE_Close(&ulHandlePtr);

                        /* Seek檔失敗，所以回傳VS_ERROR。(關檔不論成功與否都要回傳(VS_ERROR)) */
                        return (VS_ERROR);
                }

                /*確認檔案大小*/
                if (lnFILE_GetSize(&ulHandlePtr, uszFileName) == 0)
                {
                        /* 長度為0，不必讀 */
                        /* debug */
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "lnFILE_GetSize ＝＝ 0 ：");
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "(FName = %s), (Ptr = %d)", uszFileName, (int)ulHandlePtr);
                                inLogPrintf(AT, szErrorMsg);
                        }
                        /* 關檔並回傳VS_NO_RECORD */
                        inFILE_Close(&ulHandlePtr);
                        return (VS_NO_RECORD);
                }
                /* Get不到Size的時候 */
                else if (lnFILE_GetSize(&ulHandlePtr, uszFileName) == VS_ERROR)
                {
                        /* 關檔並回傳VS_ERROR */
                        inFILE_Close(&ulHandlePtr);
                        return (VS_ERROR);
                }
                /* 檔案大小大於0(裡面有資料時) */
                else
                {
                        /* 讀檔 */
                        if (inFILE_Read(&ulHandlePtr, (unsigned char *) srAccumRec, _TICKET_ACCUM_REC_SIZE_) == VS_ERROR)
                        {
                                /* 讀檔失敗就關檔 */
                                inFILE_Close(&ulHandlePtr);
                                /* 讀檔失敗，所以回傳（VS_ERROR）。(關檔不論成功與否都要回傳(VS_ERROR)) */
                                return (VS_ERROR);
                        }
                }

                /* 將檔案關閉 */
                if (inFILE_Close(&ulHandlePtr) == VS_ERROR)
                {
                        /*關檔失敗*/
                        /* 關檔失敗，所以回傳VS_ERROR */
                        return (VS_ERROR);
                }
        }
        /* inACCUM_GetRecord()_END */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inACCUM_GetRecord_ESVC()_END");
        }

        return (VS_SUCCESS);
}

/*
Function        :inACCUM_UpdateFlow
Date&Time       :2015/9/4 下午 13:30
Describe        :做交易後計算總額的flow，回傳(VS_SUCCESS) or (VS_ERROR)
*/
int inACCUM_UpdateFlow(TRANSACTION_OBJECT *pobTran)
{
        unsigned long   ulHandlePtr = 0;				/* File Handle，type為pointer */
        int		inRetVal = VS_SUCCESS;				/* return value，來判斷是否回傳error */
	char		szTRTFileName[12 + 1] = {};
        unsigned char   uszFileName[14 + 1] = {0}; /* 儲存交易金額檔案的檔案名稱(最大為15) */
        ACCUM_TOTAL_REC srAccumRec; /*用來放總筆數、總金額的結構體*/
        
        vdUtility_SYSFIN_LogMessage(AT, "inACCUM_UpdateFlow START!");
	
	memset(szTRTFileName, 0x00, sizeof(szTRTFileName));
	inGetTRTFileName(szTRTFileName);
	if (!memcmp(szTRTFileName, _TRT_FILE_NAME_TAKA_, strlen(_TRT_FILE_NAME_TAKA_)))
	{
		inRetVal = inACCUM_UpdateFlow_TAKA(pobTran);
                
                if (inRetVal != VS_SUCCESS)
                {
                        vdUtility_SYSFIN_LogMessage(AT, "inACCUM_UpdateFlow inACCUM_UpdateFlow_TAKA failed");
                }
                
		return (inRetVal);
	}
	else
	{
		/* inACCUM_UpdateFlow()_START */
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "----------------------------------------");
			inLogPrintf(AT, "inACCUM_UpdateFlow()_START");
		}
		/* 若斷電導致批次無法正確儲存時方便解釋 */
		inUtility_StoreTraceLog_OneStep("_UPDATE_ACCUM_START_($:%ld)", pobTran->srBRec.lnTxnAmount);

		if (ginFindRunTime == VS_TRUE)
		{
			inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
		}

		if (inFunc_ComposeFileName(pobTran, (char*)uszFileName, _ACCUM_FILE_EXTENSION_, 6) != VS_SUCCESS)
		{
                        vdUtility_SYSFIN_LogMessage(AT, "inACCUM_UpdateFlow inFunc_ComposeFileName failed");
			/* Update Accum失敗 鎖機*/
			inFunc_EDCLock(AT);

			return (VS_ERROR);
		}

		/* 開檔 create檔(若沒有檔案則創建檔案) */
		if (inFILE_Open(&ulHandlePtr, uszFileName) == (VS_ERROR))
		{
			/* 開檔失敗時，不必關檔(開檔失敗，handle回傳NULL) */
			/* 若檔案不存在時，Create檔案 */
			if (inFILE_Check_Exist(uszFileName) == (VS_ERROR))
			{
				inRetVal = inFILE_Create(&ulHandlePtr, uszFileName);
			}
			/* 檔案存在還是開檔失敗，回傳錯誤跳出 */
			else
			{
                                vdUtility_SYSFIN_LogMessage(AT, "inACCUM_UpdateFlow file_exist_but_opened_failed");
				/* Update Accum失敗 鎖機*/
				inFunc_EDCLock(AT);

				return (VS_ERROR);
			}

			/* Create檔 */
			if (inRetVal != VS_SUCCESS)
			{
                                vdUtility_SYSFIN_LogMessage(AT, "inACCUM_UpdateFlow create_failed");
				/* Update Accum失敗 鎖機*/
				inFunc_EDCLock(AT);

				/* Create檔失敗時不關檔，因為create檔失敗handle回傳NULL */
				return (VS_ERROR);
			}
			/* create檔成功就繼續(因為create檔已經把handle指向檔案，所以不用在開檔) */
		}

		/* 開檔成功或create檔成功後 */
		/*先清空srAccumRec 為讀檔案作準備  */
		memset(&srAccumRec, 0x00, sizeof(srAccumRec));

		/* 把指針指到開頭*/
		inRetVal = inFILE_Seek(ulHandlePtr, 0, _SEEK_BEGIN_);

		/* inFile_seek */
		if (inRetVal != VS_SUCCESS)
		{
                        vdUtility_SYSFIN_LogMessage(AT, "inACCUM_UpdateFlow seek_failed");
			/* Update Accum失敗 鎖機*/
			inFunc_EDCLock(AT);

			/* inFILE_Seek失敗時 */
			/* Seek檔失敗，所以關檔 */
			inFILE_Close(&ulHandlePtr);

			/* Seek檔失敗，所以回傳VS_ERROR。(關檔不論成功與否都要回傳(VS_ERROR)) */
			return (VS_ERROR);
		}

		/* 2015/11/20 修改 */
		/* 當文件內容為空時，read會回傳error;所以當read失敗， 而且檔案長度不等於0的時候，才是真的出錯 */
		if (inFILE_Read(&ulHandlePtr, (unsigned char *)&srAccumRec, _ACCUM_REC_SIZE_) == VS_ERROR && lnFILE_GetSize(&ulHandlePtr, (unsigned char *) &srAccumRec) != 0)
		{
                        vdUtility_SYSFIN_LogMessage(AT, "inACCUM_UpdateFlow read failed");
			/* Update Accum失敗 鎖機*/
			inFunc_EDCLock(AT);

			/* 讀檔失敗 */
			/* Read檔失敗，所以關檔 */
			inFILE_Close(&ulHandlePtr);

			/* Read檔失敗，所以回傳VS_ERROR。(關檔不論成功與否都要回傳(VS_ERROR)) */
			return (VS_ERROR);
		}
		else
		{
			if (pobTran->srBRec.inCode == _LOYALTY_REDEEM_	||
			    pobTran->srBRec.inCode == _VOID_LOYALTY_REDEEM_)
			{
				/* 計算優惠兌換成功數量 */
				if (inACCUM_UpdateLoyaltyRedeem(pobTran, &srAccumRec) == VS_ERROR)
				{
                                        vdUtility_SYSFIN_LogMessage(AT, "inACCUM_UpdateFlow inACCUM_UpdateLoyaltyRedeem failed");
					/* Update Accum失敗 鎖機*/
					inFunc_EDCLock(AT);

					/* 計算卡別全部交易金額、筆數失敗 */
					/* 計算卡別全部交易金額、筆數失敗，所以關檔 */
					inFILE_Close(&ulHandlePtr);

					/* 計算卡別全部交易金額、筆數失敗，所以回傳VS_ERROR。(關檔不論成功與否都要回傳(VS_ERROR)) */
					return (VS_ERROR);
				}
			}
			else
			{
				/* 計算全部交易金額、筆數 */
				if (inACCUM_UpdateTotalAmount(pobTran, &srAccumRec) == VS_ERROR)
				{
                                        vdUtility_SYSFIN_LogMessage(AT, "inACCUM_UpdateFlow inACCUM_UpdateTotalAmount failed");
					/* Update Accum失敗 鎖機*/
					inFunc_EDCLock(AT);

					/* 計算全部交易金額、筆數失敗 */
					/* 計算全部交易金額、筆數失敗，所以關檔 */
					inFILE_Close(&ulHandlePtr);

					/* 計算全部交易金額、筆數失敗，所以回傳VS_ERROR。(關檔不論成功與否都要回傳(VS_ERROR)) */
					return (VS_ERROR);
				}

				/* 計算卡別全部交易金額、筆數 */
				if (inACCUM_UpdateTotalAmountByCard(pobTran, &srAccumRec) == VS_ERROR)
				{
                                        vdUtility_SYSFIN_LogMessage(AT, "inACCUM_UpdateFlow inACCUM_UpdateTotalAmountByCard failed");
					/* Update Accum失敗 鎖機*/
					inFunc_EDCLock(AT);

					/* 計算卡別全部交易金額、筆數失敗 */
					/* 計算卡別全部交易金額、筆數失敗，所以關檔 */
					inFILE_Close(&ulHandlePtr);

					/* 計算卡別全部交易金額、筆數失敗，所以回傳VS_ERROR。(關檔不論成功與否都要回傳(VS_ERROR)) */
					return (VS_ERROR);
				}
			}

			/* 先將檔案關閉 */
			if (inFILE_Close(&ulHandlePtr) == (VS_ERROR))
			{
                                vdUtility_SYSFIN_LogMessage(AT, "inACCUM_UpdateFlow inFILE_Close failed");
				/* Update Accum失敗 鎖機*/
				inFunc_EDCLock(AT);

				/* 關檔失敗 */
				/* 回傳VS_ERROR */
				return (VS_ERROR);
			}

			/* 存檔案 */
			if (inACCUM_StoreRecord(&srAccumRec, uszFileName) == VS_ERROR)
			{
                                vdUtility_SYSFIN_LogMessage(AT, "inACCUM_UpdateFlow inACCUM_StoreRecord failed");
				/* Update Accum失敗 鎖機*/
				inFunc_EDCLock(AT);

				/* 存檔失敗 */
				/* 因為inACCUM_StoreRecord失敗，所以回傳VS_ERROR */
				return (VS_ERROR);
			}

		}/* read、計算總額 和 存檔 成功 */

		/* 若斷電導致批次無法正確儲存時方便解釋 */
		inUtility_StoreTraceLog_OneStep("_UPDATE_ACCUM_END_($:%ld)", pobTran->srBRec.lnTxnAmount);
		/* inACCUM_UpdateFlow()_END */
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "----------------------------------------");
			inLogPrintf(AT, "inACCUM_UpdateFlow()_END");
		}

		if (ginFindRunTime == VS_TRUE)
		{
			inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
		}
	}
        
        vdUtility_SYSFIN_LogMessage(AT, "inACCUM_UpdateFlow END!");
	
        return (VS_SUCCESS);
}

/*
Function        :inACCUM_ReviewReport_Total
Date&Time       :2016/2/25 下午 4:06
Describe        :螢幕顯示總額
*/
int inACCUM_ReviewReport_Total(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	char	szHostLabel[8 + 1];
	
        vdUtility_SYSFIN_LogMessage(AT, "inACCUM_ReviewReport_Total START!");
        
	/* 票證顯示格式不同 */
	if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
	{
		inRetVal = inNCCC_Ticket_Func_Display_Review(pobTran);
	}
	else
	{
		memset(szHostLabel, 0x00, sizeof(szHostLabel));
		inGetHostLabel(szHostLabel);
		if (!memcmp(szHostLabel, _HOST_NAME_CREDIT_NCCC_, strlen(_HOST_NAME_CREDIT_NCCC_)))
		{
			inRetVal = inNCCC_Func_Display_Review(pobTran);
			if (inRetVal == VS_SUCCESS)
			{
				inRetVal = inNCCC_Func_Display_Review_ESC_Reinforce(pobTran);
			}
		}
		else if (!memcmp(szHostLabel, _HOST_NAME_DCC_, strlen(_HOST_NAME_DCC_)))
		{
			inRetVal = inNCCC_Func_Display_Review_DCC(pobTran);
			if (inRetVal == VS_SUCCESS)
			{
				inRetVal = inNCCC_Func_Display_Review_ESC_Reinforce(pobTran);
			}
		}
		else if (!memcmp(szHostLabel, _HOST_NAME_TAKA_, strlen(_HOST_NAME_TAKA_)))
		{
			inRetVal = inNCCC_Func_Display_Review_TAKA(pobTran);
		}
		/* 2018/5/8 下午 2:10 DFS需求不再使用大來主機 */
		else
		{
			inRetVal = VS_ERROR;
			pobTran->inErrorMsg = _ERROR_CODE_V3_OPERATION_;
		}
	}
	
	return (inRetVal);
	
}

/*
Function        :inACCUM_ReviewReport_Total_Settle
Date&Time       :2016/10/12 上午 11:45
Describe        :交易確認畫面，for 結帳用
*/
int inACCUM_ReviewReport_Total_Settle(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_SUCCESS;
	int		inChoice = 0;
	int		inTouchSensorFunc = _Touch_BATCH_END_;
	char		szHostLabel[8 + 1] = {0};
	char		szCustomerIndicator[3 + 1] = {0};
	unsigned char	uszKey;
	
	vdUtility_SYSFIN_LogMessage(AT, "inACCUM_ReviewReport_Total_Settle START!");
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	/* (需求單 - 107227)邦柏科技自助作業客製化 結帳確認畫面要跳過 by Russell 2018/12/7 下午 2:36 */
	/* (需求單 - 107276)自助交易標準400做法 結帳確認畫面要跳過 by Russell 2018/12/27 上午 11:20 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)		||
            !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_)	||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)		||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
	{
		inRetVal = VS_SUCCESS;
	}
	else
	{
		/* 連動結帳的話 */
		if (pobTran->uszAutoSettleBit == VS_TRUE)
		{
			/* 如果是ECR發動不用再按0確認 */
			if (pobTran->uszECRBit == VS_TRUE)
			{
				inRetVal = VS_SUCCESS;
			}
			/* (需求單-109327)-vx520客製化需求 by Russell 2022/4/13 上午 10:46 */
			/* 昇恆昌客製化： 連動結帳移除請按(0)確認提示畫面 */
			else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_046_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)         ||
				 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_047_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	     ||
				 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_075_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	     ||
				 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_103_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_))
			{
				inRetVal = VS_SUCCESS;
			}
			else
			{
				inDISP_Clear_Line(_LINE_8_6_, _LINE_8_8_);
				inDISP_PutGraphic(_ERR_0_, 0, _COORDINATE_Y_LINE_8_7_);

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
			}
		}
		else
		{
			/* 美麗華客製化(035)： ECR結帳移除請按(0)確認提示畫面 */
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_035_MIRAMAR_CINEMAS_, _CUSTOMER_INDICATOR_SIZE_)	&&
			    pobTran->uszECRBit == VS_TRUE)
			{
				inRetVal = VS_SUCCESS;
			}
			else
			{
				/* 票證顯示格式不同 */
				if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
				{
					inRetVal = inNCCC_Ticket_Func_Display_Review_Settle(pobTran);
				}
				else
				{
					memset(szHostLabel, 0x00, sizeof(szHostLabel));
					inGetHostLabel(szHostLabel);
					if (!memcmp(szHostLabel, _HOST_NAME_CREDIT_NCCC_, strlen(_HOST_NAME_CREDIT_NCCC_)))
					{
						inRetVal = inNCCC_Func_Display_Review_Settle(pobTran);
					}
					else if (!memcmp(szHostLabel, _HOST_NAME_DCC_, strlen(_HOST_NAME_DCC_)))
					{
						inRetVal = inNCCC_Func_Display_Review_Settle_DCC(pobTran);
					}
					else if (!memcmp(szHostLabel, _HOST_NAME_TAKA_, strlen(_HOST_NAME_TAKA_)))
					{
						inRetVal = inNCCC_Func_Display_Review_Settle_TAKA(pobTran);
					}else if (!memcmp(szHostLabel, _HOST_NAME_TRUST_, strlen(_HOST_NAME_TRUST_)))
					{	/* [20251219_BUG_MDF][UI] 信託主機在結帳時,需要顯示按0確認的頁面 */
						inRetVal = inNCCC_Func_Display_Review_Settle_TRUST(pobTran);
					}
                    
					/* 2018/5/8 下午 2:10 DFS需求不再使用大來主機 */
				}
			}
		}
	}
		
	return (inRetVal);
}

/*
Function        :inACCUM_UpdateFlow_ESC
Date&Time       :2018/4/17 下午 3:34
Describe        :
*/
int inACCUM_UpdateFlow_ESC(TRANSACTION_OBJECT *pobTran, int inUpdateType)
{
	unsigned long   ulHandlePtr;			/* File Handle，type為pointer */
        int		inRetVal;			/* return value，來判斷是否回傳error */
        unsigned char   uszFileName[14 + 1] = {0};	/* 儲存交易金額檔案的檔案名稱(最大為15) */
        ACCUM_TOTAL_REC srAccumRec;			/*用來放總筆數、總金額的結構體*/

        /* inACCUM_UpdateFlow()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inACCUM_UpdateFlow_ESC()_START");
        }

	memset(uszFileName, 0x00, sizeof(uszFileName));
	if (inFunc_ComposeFileName(pobTran, (char*)uszFileName, _ACCUM_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		vdUtility_SYSFIN_LogMessage(AT, "inACCUM_UpdateFlow_ESC _ACCUM_FILE_EXTENSION_inFunc_ComposeFileName_ fail");
		return (VS_ERROR);
	}

        /* 開檔 create檔(若沒有檔案則創建檔案) */
        if (inFILE_Open(&ulHandlePtr, uszFileName) == (VS_ERROR))
        {
                /* 開檔失敗時，不必關檔(開檔失敗，handle回傳NULL) */
                /* 若檔案不存在時，Create檔案 */
                if (inFILE_Check_Exist(uszFileName) == (VS_ERROR))
                {
                        inRetVal = inFILE_Create(&ulHandlePtr, uszFileName);
                }
                /* 檔案存在還是開檔失敗，回傳錯誤跳出 */
                else
                {
			vdUtility_SYSFIN_LogMessage(AT, "inACCUM_UpdateFlow_ESC File_exist_open_failed");
                        return (VS_ERROR);
                }

                /* Create檔 */
                if (inRetVal != VS_SUCCESS)
                {
			vdUtility_SYSFIN_LogMessage(AT, "inACCUM_UpdateFlow_ESC File_create_fail");
                        /* Create檔失敗時不關檔，因為create檔失敗handle回傳NULL */
                        return (VS_ERROR);
                }
                /* create檔成功就繼續(因為create檔已經把handle指向檔案，所以不用在開檔) */
        }

        /* 開檔成功或create檔成功後 */
        /*先清空srAccumRec 為讀檔案作準備  */
        memset(&srAccumRec, 0x00, sizeof(srAccumRec));

        /* 把指針指到開頭*/
        inRetVal = inFILE_Seek(ulHandlePtr, 0, _SEEK_BEGIN_);

        /* inFile_seek */
        if (inRetVal != VS_SUCCESS)
        {
		vdUtility_SYSFIN_LogMessage(AT, "inACCUM_UpdateFlow_ESC ");
                /* inFILE_Seek失敗時 */
                /* Seek檔失敗，所以關檔 */
                inFILE_Close(&ulHandlePtr);

                /* Seek檔失敗，所以回傳VS_ERROR。(關檔不論成功與否都要回傳(VS_ERROR)) */
                return (VS_ERROR);
        }

        
        /* 當文件內容為空時，read會回傳error;所以當read失敗， 而且檔案長度不等於0的時候，才是真的出錯 */
        if (inFILE_Read(&ulHandlePtr, (unsigned char *)&srAccumRec, _ACCUM_REC_SIZE_) == VS_ERROR && lnFILE_GetSize(&ulHandlePtr, (unsigned char *) &srAccumRec) != 0)
        {
		vdUtility_SYSFIN_LogMessage(AT, "inACCUM_UpdateFlow_ESC inFILE_Read failed ptr:(%lu)", ulHandlePtr);
                /* 讀檔失敗 */
                /* Read檔失敗，所以關檔 */
                inFILE_Close(&ulHandlePtr);

                /* Read檔失敗，所以回傳VS_ERROR。(關檔不論成功與否都要回傳(VS_ERROR)) */
                return (VS_ERROR);
        }
        else
        {
                /* 計算ESC全部交易金額、筆數 */
                if (inACCUM_Update_ESC_TotalAmount(pobTran, &srAccumRec, inUpdateType) == VS_ERROR)
                {
			vdUtility_SYSFIN_LogMessage(AT, "inACCUM_UpdateFlow_ESC inACCUM_Update_ESC_TotalAmount failed");
                        /* 計算ESC全部交易金額、筆數失敗 */
                        /* 計算ESC全部交易金額、筆數失敗，所以關檔 */
                        inFILE_Close(&ulHandlePtr);

                        /* 計算全部交易金額、筆數失敗，所以回傳VS_ERROR。(關檔不論成功與否都要回傳(VS_ERROR)) */
                        return (VS_ERROR);
                }

                /* 先將檔案關閉 */
                if (inFILE_Close(&ulHandlePtr) == (VS_ERROR))
                {
			vdUtility_SYSFIN_LogMessage(AT, "inACCUM_UpdateFlow_ESC inFILE_Close failed ptr:(%lu)", ulHandlePtr);
                        /* 關檔失敗 */
                        /* 回傳VS_ERROR */
                        return (VS_ERROR);
                }

                /* 存檔案 */
                if (inACCUM_StoreRecord(&srAccumRec, uszFileName) == VS_ERROR)
                {
			vdUtility_SYSFIN_LogMessage(AT, "inACCUM_UpdateFlow_ESC inACCUM_StoreRecord failed");
                        /* 存檔失敗 */
                        /* 因為inACCUM_StoreRecord失敗，所以回傳VS_ERROR */
                        return (VS_ERROR);
                }

        }/* read、計算總額 和 存檔 成功 */

        /* inACCUM_UpdateFlow()_END */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inACCUM_UpdateFlow_ESC()_END");
        }
        return (VS_SUCCESS);
}

int inACCUM_Update_ESC_TotalAmount(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec, int inUpdateType)
{
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inACCUM_Update_ESC_TotalAmount()_START");
        }

	if (inUpdateType == _ESC_ACCUM_STATUS_BYPASS_)	/* ESC_BYPASS */
	{
		/* 計算筆數 */
		srAccumRec->lnESC_BypassNum ++;
		srAccumRec->lnESC_TotalFailULNum ++;
		
		if (pobTran->srBRec.uszVOIDBit == VS_FALSE)
        	{
        		switch (pobTran->srBRec.inCode)
        		{
        			case _SALE_ :
                                case _CUP_SALE_ :
                                case _INST_SALE_ :
                                case _REDEEM_SALE_ :
                                case _SALE_OFFLINE_ :
                                case _INST_ADJUST_ :    
                                case _REDEEM_ADJUST_ :  
                                case _PRE_COMP_ :
                                case _CUP_PRE_COMP_ :   
                                case _MAIL_ORDER_ :
                                case _CUP_MAIL_ORDER_ :    
                                case _CASH_ADVANCE_ :
                                case _FORCE_CASH_ADVANCE_ :  
                                case _FISC_SALE_ :              
        				/* 計算金額 */       
        				srAccumRec->llESC_BypassAmount += pobTran->srBRec.lnTxnAmount;
		                        srAccumRec->llESC_TotalFailULAmount += pobTran->srBRec.lnTxnAmount;
        				break;
        			case _REFUND_ :
        			case _REDEEM_REFUND_ :
        			case _INST_REFUND_ :
        			case _CUP_REFUND_ :
        			case _CUP_MAIL_ORDER_REFUND_ :  
        			case _FISC_REFUND_ :       
        				/* 計算金額 */
        				srAccumRec->llESC_BypassAmount -= pobTran->srBRec.lnTxnAmount;
		                        srAccumRec->llESC_TotalFailULAmount -= pobTran->srBRec.lnTxnAmount;
        				break;
        			case _TIP_ :
        			        /* 計算金額 */
                                        srAccumRec->llESC_BypassAmount += pobTran->srBRec.lnTipTxnAmount;
		                        srAccumRec->llESC_TotalFailULAmount += pobTran->srBRec.lnTipTxnAmount;
        				break;
        			case _PRE_AUTH_ :	
        			case _CUP_PRE_AUTH_ :         
        			        srAccumRec->lnESC_BypassNum --;
		                        srAccumRec->lnESC_TotalFailULNum --;
					
					/* ESC預先授權列印紙本及預先授權不納入結帳總額 */
		                        srAccumRec->lnESC_PreAuthNum ++;
		                        srAccumRec->llESC_PreAuthAmount += pobTran->srBRec.lnTxnAmount;
        			        break;
        			default :
        				return (VS_ERROR);
        		}
			
        	}
        	else
        	{
        		switch (pobTran->srBRec.inOrgCode)
        		{
        			case _SALE_ :
                                case _CUP_SALE_ :
                                case _INST_SALE_ :
                                case _REDEEM_SALE_ :
                                case _SALE_OFFLINE_ :
                                case _INST_ADJUST_ :    
                                case _REDEEM_ADJUST_ :  
                                case _PRE_COMP_ :
                                case _CUP_PRE_COMP_ :   
                                case _MAIL_ORDER_ :
                                case _CUP_MAIL_ORDER_ :    
                                case _CASH_ADVANCE_ :
                                case _FORCE_CASH_ADVANCE_ :  
                                case _FISC_SALE_ :           
        				/* 計算金額 */       
        				srAccumRec->llESC_BypassAmount += (0 - pobTran->srBRec.lnTxnAmount);
		                        srAccumRec->llESC_TotalFailULAmount += (0 - pobTran->srBRec.lnTxnAmount);
        				break;
        			case _REFUND_ :
        			case _REDEEM_REFUND_ :
        			case _INST_REFUND_ :
        			case _CUP_REFUND_ :
				case _CUP_MAIL_ORDER_REFUND_ :
        			case _FISC_REFUND_ :        
        			        /* 計算金額 */   
        				srAccumRec->llESC_BypassAmount -= (0 - pobTran->srBRec.lnTxnAmount);
		                        srAccumRec->llESC_TotalFailULAmount -= (0 - pobTran->srBRec.lnTxnAmount);
        				break;
        			case _PRE_AUTH_ :	
        			case _CUP_PRE_AUTH_ : 
        			        srAccumRec->lnESC_BypassNum --;
		                        srAccumRec->lnESC_TotalFailULNum --;
					
					/* ESC預先授權列印紙本及預先授權不納入結帳總額 */
		                        srAccumRec->lnESC_PreAuthNum ++;
		                        srAccumRec->llESC_PreAuthAmount += pobTran->srBRec.lnTxnAmount;
        			        break;	
        			default :
        				return (VS_ERROR);
        		}
        	}
	}
	else if (inUpdateType == _ESC_ACCUM_STATUS_FAIL_)		/* ESC_FAIL */
	{
		/* 計算筆數 */
		srAccumRec->lnESC_FailUploadNum ++;
		srAccumRec->lnESC_TotalFailULNum ++;
		
		if (pobTran->srBRec.uszVOIDBit == VS_FALSE)
        	{
        		switch (pobTran->srBRec.inCode)
        		{
        			case _SALE_ :
                                case _CUP_SALE_ :
                                case _INST_SALE_ :
                                case _REDEEM_SALE_ :
                                case _SALE_OFFLINE_ :
                                case _INST_ADJUST_ :    
                                case _REDEEM_ADJUST_ :  
                                case _PRE_COMP_ :
                                case _CUP_PRE_COMP_ :   
                                case _MAIL_ORDER_ :
                                case _CUP_MAIL_ORDER_ :    
                                case _CASH_ADVANCE_ :
                                case _FORCE_CASH_ADVANCE_ :  
                                case _FISC_SALE_ :      
        				/* 計算金額 */
		                        srAccumRec->llESC_FailUploadAmount += pobTran->srBRec.lnTxnAmount;
		                        srAccumRec->llESC_TotalFailULAmount += pobTran->srBRec.lnTxnAmount;
        				break;
        			case _REFUND_ :
        			case _REDEEM_REFUND_ :
        			case _INST_REFUND_ :
        			case _CUP_REFUND_ :
        			case _CUP_MAIL_ORDER_REFUND_ :  
        			case _FISC_REFUND_ :
        				/* 計算金額 */
		                        srAccumRec->llESC_FailUploadAmount -= pobTran->srBRec.lnTxnAmount;
		                        srAccumRec->llESC_TotalFailULAmount -= pobTran->srBRec.lnTxnAmount;
        				break;
        			case _TIP_ :
        			        /* 計算金額 */
        				srAccumRec->llESC_FailUploadAmount += pobTran->srBRec.lnTipTxnAmount;
                                        srAccumRec->llESC_TotalFailULAmount += pobTran->srBRec.lnTipTxnAmount;
        				break;
        			case _PRE_AUTH_ :	
        			case _CUP_PRE_AUTH_ :         
        			        srAccumRec->lnESC_FailUploadNum --;
		                        srAccumRec->lnESC_TotalFailULNum --;
					
					/* ESC預先授權列印紙本及預先授權不納入結帳總額 */
		                        srAccumRec->lnESC_PreAuthNum ++;
		                        srAccumRec->llESC_PreAuthAmount += pobTran->srBRec.lnTxnAmount;
        			        break;	
        			default :
        				return (VS_ERROR);
        		}
        	}
        	else
        	{
        		switch (pobTran->srBRec.inOrgCode)
        		{
        			case _SALE_ :
                                case _CUP_SALE_ :
                                case _INST_SALE_ :
                                case _REDEEM_SALE_ :
                                case _SALE_OFFLINE_ :
                                case _INST_ADJUST_ :    
                                case _REDEEM_ADJUST_ :  
                                case _PRE_COMP_ :
                                case _CUP_PRE_COMP_ :   
                                case _MAIL_ORDER_ :
                                case _CUP_MAIL_ORDER_ :    
                                case _CASH_ADVANCE_ :
                                case _FORCE_CASH_ADVANCE_ :  
                                case _FISC_SALE_ :
        				/* 計算金額 */
		                        srAccumRec->llESC_FailUploadAmount += (0 - pobTran->srBRec.lnTxnAmount);
		                        srAccumRec->llESC_TotalFailULAmount += (0 - pobTran->srBRec.lnTxnAmount);
        				break;
        			case _REFUND_ :
        			case _REDEEM_REFUND_ :
        			case _INST_REFUND_ :
        			case _CUP_REFUND_ :
				case _CUP_MAIL_ORDER_REFUND_ :
        			case _FISC_REFUND_ :        
        				/* 計算金額 */
		                        srAccumRec->llESC_FailUploadAmount -= (0 - pobTran->srBRec.lnTxnAmount);
		                        srAccumRec->llESC_TotalFailULAmount -= (0 - pobTran->srBRec.lnTxnAmount);
        				break;
        			case _PRE_AUTH_ :	
        			case _CUP_PRE_AUTH_ :         
        			        srAccumRec->lnESC_FailUploadNum --;
		                        srAccumRec->lnESC_TotalFailULNum --;
					
					/* ESC預先授權列印紙本及預先授權不納入結帳總額 */
		                        srAccumRec->lnESC_PreAuthNum ++;
		                        srAccumRec->llESC_PreAuthAmount += pobTran->srBRec.lnTxnAmount;
        			        break;	
        			default :
        				return (VS_ERROR);
        		}
        	}
	}
        else		/* ESC_SUCCESS */
	{
		/* 計算筆數 */
		srAccumRec->lnESC_SuccessNum ++;
		
		if (pobTran->srBRec.uszVOIDBit == VS_FALSE)
        	{
        		switch (pobTran->srBRec.inCode)
        		{
        			case _SALE_ :
                                case _CUP_SALE_ :
                                case _INST_SALE_ :
                                case _REDEEM_SALE_ :
                                case _SALE_OFFLINE_ :
                                case _INST_ADJUST_ :    
                                case _REDEEM_ADJUST_ :  
                                case _PRE_COMP_ :
                                case _CUP_PRE_COMP_ :   
                                case _MAIL_ORDER_ :
                                case _CUP_MAIL_ORDER_ :    
                                case _CASH_ADVANCE_ :
                                case _FORCE_CASH_ADVANCE_ :  
                                case _FISC_SALE_ :        
        				/* 計算金額 */
		                        srAccumRec->llESC_SuccessAmount += pobTran->srBRec.lnTxnAmount;
        				break;
        			case _REFUND_ :
        			case _REDEEM_REFUND_ :
        			case _INST_REFUND_ :
        			case _CUP_REFUND_ :
        			case _CUP_MAIL_ORDER_REFUND_ :  
        			case _FISC_REFUND_ :
        				/* 計算金額 */
		                        srAccumRec->llESC_SuccessAmount -= pobTran->srBRec.lnTxnAmount;
        				break;
        			case _TIP_ :
        				/* 計算金額 */
        				srAccumRec->llESC_SuccessAmount += pobTran->srBRec.lnTipTxnAmount;
        				break;
        			case _PRE_AUTH_ :	
        			case _CUP_PRE_AUTH_ :         
        			        srAccumRec->lnESC_SuccessNum --;
					
					/* ESC預先授權列印紙本及預先授權不納入結帳總額 */
		                        srAccumRec->lnESC_PreAuthNum ++;
		                        srAccumRec->llESC_PreAuthAmount += pobTran->srBRec.lnTxnAmount;
        			        break;	
        			default :
        				return (VS_ERROR);
        		}
        	}
        	else
        	{
        		switch (pobTran->srBRec.inOrgCode)
        		{
        			case _SALE_ :
                                case _CUP_SALE_ :
                                case _INST_SALE_ :
                                case _REDEEM_SALE_ :
                                case _SALE_OFFLINE_ :
                                case _INST_ADJUST_ :    
                                case _REDEEM_ADJUST_ :  
                                case _PRE_COMP_ :
                                case _CUP_PRE_COMP_ :   
                                case _MAIL_ORDER_ :
                                case _CUP_MAIL_ORDER_ :    
                                case _CASH_ADVANCE_ :
                                case _FORCE_CASH_ADVANCE_ :  
                                case _FISC_SALE_ :        
        				/* 計算金額 */
		                        srAccumRec->llESC_SuccessAmount += (0 - pobTran->srBRec.lnTxnAmount);
        				break;
        			case _REFUND_ :
        			case _REDEEM_REFUND_ :
        			case _INST_REFUND_ :
        			case _CUP_REFUND_ :
				case _CUP_MAIL_ORDER_REFUND_ :
        			case _FISC_REFUND_ :         
        				/* 計算金額 */
		                        srAccumRec->llESC_SuccessAmount -= (0 - pobTran->srBRec.lnTxnAmount);
        				break;
        			case _PRE_AUTH_ :	
        			case _CUP_PRE_AUTH_ :         
        			        srAccumRec->lnESC_SuccessNum --;
					
					/* ESC預先授權列印紙本及預先授權不納入結帳總額 */
		                        srAccumRec->lnESC_PreAuthNum ++;
		                        srAccumRec->llESC_PreAuthAmount += pobTran->srBRec.lnTxnAmount;
        			        break;		
        			default :
        				return (VS_ERROR);
        		}
			
        	}
		
	}
	
	/* inACCUM_UpdateFlow()_END */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inACCUM_Update_ESC_TotalAmount()_END");
        }
        return (VS_SUCCESS);
}

/*
Function        :inACCUM_Check_Specific_Accum
Date&Time       :2017/1/10 下午 3:42
Describe        :原來用來確認該Host有沒有做過交易，但後來想到更好的辦法
*/
int inACCUM_Check_Specific_Accum(TRANSACTION_OBJECT *pobTran, char *szHostName, ACCUM_TOTAL_REC* srACCUMRec)
{
	int	inHostIndex = -1;
	
	inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, szHostName, &inHostIndex);
	
	if (inHostIndex == -1)
	{
		return (VS_ERROR);
	}
	
	if (inLoadHDPTRec(inHostIndex) != VS_SUCCESS)
	{
		/* load回來 */
		inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
			
		return (VS_ERROR);
	}
	
	if (inACCUM_GetRecord(pobTran, srACCUMRec) != VS_SUCCESS)
	{
		/* load回來 */
		inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
		
		return (VS_ERROR);
	}
	
	/* load回來 */
	inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
	
	return (VS_SUCCESS);
}

/*
Function        :inACCUM_Check_Transaction_Count
Date&Time       :2017/1/10 下午 4:13
Describe        :確認是否有交易紀錄，若沒做交易，連Accum檔都不會產生
*/
int inACCUM_Check_Transaction_Count(TRANSACTION_OBJECT *pobTran, char *szHostName, char *szTrans)
{
	int	inHostIndex = -1;
	char	szFileName[15 + 1];
	
	inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, szHostName, &inHostIndex);
	
	if (inHostIndex == -1)
	{
		return (VS_ERROR);
	}
	
	if (inLoadHDPTRec(inHostIndex) != VS_SUCCESS)
	{
		/* load回來 */
		inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
			
		return (VS_ERROR);
	}
	
	memset(szFileName, 0x00, sizeof(szFileName));
	if (inFunc_ComposeFileName(pobTran, szFileName, _ACCUM_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		/* load回來 */
		inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
		
		return (VS_ERROR);
	}
	
	if (inFILE_Check_Exist((unsigned char*)szFileName) != VS_SUCCESS)
	{
		*szTrans = 'N';
	}
	else
	{
		*szTrans = 'Y';
	}
	
	/* load回來 */
	inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
		
	return (VS_SUCCESS);
}

/*
Function        :inACCUM_UpdateLoyaltyRedeem
Date&Time       :2017/2/21 下午 1:41
Describe        :計算優惠兌換數量
*/
int inACCUM_UpdateLoyaltyRedeem(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec)
{
	/* 兌換成功或兌換取消 */
	if (pobTran->srBRec.inCode == _LOYALTY_REDEEM_)
	{
		srAccumRec->lnLoyaltyRedeemSuccessCount ++;
		srAccumRec->lnLoyaltyRedeemTotalCount ++;
	}
	else if (pobTran->srBRec.inCode == _VOID_LOYALTY_REDEEM_)
	{
		srAccumRec->lnLoyaltyRedeemCancelCount ++;
		srAccumRec->lnLoyaltyRedeemTotalCount --;
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inACCUM_Update_Ticket_Flow
Date&Time       :2018/1/12 下午 2:27
Describe        :紀錄電票交易的帳
*/
int inACCUM_Update_Ticket_Flow(TRANSACTION_OBJECT *pobTran)
{
        unsigned long		ulHandlePtr = 0;	/* File Handle，type為pointer */
        int			inRetVal;		/* return value，來判斷是否回傳error */
        unsigned char		uszFileName[14 + 1];	/* 儲存交易金額檔案的檔案名稱(最大為15) */
        TICKET_ACCUM_TOTAL_REC	srAccumRec;		/*用來放總筆數、總金額的結構體*/

        /* inACCUM_UpdateFlow()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inACCUM_Update_Ticket_Flow()_START");
        }
	/* 若斷電導致批次無法正確儲存時方便解釋 */
	inUtility_StoreTraceLog_OneStep("_TICKET_UPDATE_ACCUM_START_($:%ld)", pobTran->srTRec.lnTxnAmount);
	
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
	}
	
	/*  黑名單鎖卡不應存帳 */
	if (pobTran->srTRec.uszBlackListBit == VS_TRUE)
	{
	        return (VS_ERROR);
	}

	if (inFunc_ComposeFileName(pobTran, (char*)uszFileName, _ACCUM_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		/* Update Accum失敗 鎖機*/
		inFunc_EDCLock(AT);
			
		return (VS_ERROR);
	}

        /* 開檔 create檔(若沒有檔案則創建檔案) */
        if (inFILE_Open(&ulHandlePtr, uszFileName) == (VS_ERROR))
        {
                /* 開檔失敗時，不必關檔(開檔失敗，handle回傳NULL) */
                /* 若檔案不存在時，Create檔案 */
                if (inFILE_Check_Exist(uszFileName) == (VS_ERROR))
                {
                        inRetVal = inFILE_Create(&ulHandlePtr, uszFileName);
                }
                /* 檔案存在還是開檔失敗，回傳錯誤跳出 */
                else
                {
			/* Update Accum失敗 鎖機*/
			inFunc_EDCLock(AT);
			
                        return (VS_ERROR);
                }

                /* Create檔 */
                if (inRetVal != VS_SUCCESS)
                {
			/* Update Accum失敗 鎖機*/
			inFunc_EDCLock(AT);
		
                        /* Create檔失敗時不關檔，因為create檔失敗handle回傳NULL */
                        return (VS_ERROR);
                }
                /* create檔成功就繼續(因為create檔已經把handle指向檔案，所以不用在開檔) */
        }

        /* 開檔成功或create檔成功後 */
        /*先清空srAccumRec 為讀檔案作準備  */
        memset(&srAccumRec, 0x00, sizeof(srAccumRec));

        /* 把指針指到開頭*/
        inRetVal = inFILE_Seek(ulHandlePtr, 0, _SEEK_BEGIN_);

        /* inFile_seek */
        if (inRetVal != VS_SUCCESS)
        {
		/* Update Accum失敗 鎖機*/
		inFunc_EDCLock(AT);
		
                /* inFILE_Seek失敗時 */
                /* Seek檔失敗，所以關檔 */
                inFILE_Close(&ulHandlePtr);

                /* Seek檔失敗，所以回傳VS_ERROR。(關檔不論成功與否都要回傳(VS_ERROR)) */
                return (VS_ERROR);
        }

        /* 2015/11/20 修改 */
        /* 當文件內容為空時，read會回傳error;所以當read失敗， 而且檔案長度不等於0的時候，才是真的出錯 */
        if (inFILE_Read(&ulHandlePtr, (unsigned char *)&srAccumRec, _TICKET_ACCUM_REC_SIZE_) == VS_ERROR && lnFILE_GetSize(&ulHandlePtr, (unsigned char *) &srAccumRec) != 0)
        {
		/* Update Accum失敗 鎖機*/
		inFunc_EDCLock(AT);
		
                /* 讀檔失敗 */
                /* Read檔失敗，所以關檔 */
                inFILE_Close(&ulHandlePtr);

                /* Read檔失敗，所以回傳VS_ERROR。(關檔不論成功與否都要回傳(VS_ERROR)) */
                return (VS_ERROR);
        }
        else
        {
		/* 計算全部交易金額、筆數 */
		if (inACCUM_UpdateTotalAmount_Ticket(pobTran, &srAccumRec) == VS_ERROR)
		{
			/* Update Accum失敗 鎖機*/
			inFunc_EDCLock(AT);

			/* 計算全部交易金額、筆數失敗 */
			/* 計算全部交易金額、筆數失敗，所以關檔 */
			inFILE_Close(&ulHandlePtr);

			/* 計算全部交易金額、筆數失敗，所以回傳VS_ERROR。(關檔不論成功與否都要回傳(VS_ERROR)) */
			return (VS_ERROR);
		}

		
                /* 先將檔案關閉 */
                if (inFILE_Close(&ulHandlePtr) == (VS_ERROR))
                {
			/* Update Accum失敗 鎖機*/
			inFunc_EDCLock(AT);
			
                        /* 關檔失敗 */
                        /* 回傳VS_ERROR */
                        return (VS_ERROR);
                }

                /* 存檔案 */
                if (inACCUM_StoreRecord_Ticket(&srAccumRec, uszFileName) == VS_ERROR)
                {
			/* Update Accum失敗 鎖機*/
			inFunc_EDCLock(AT);
		
                        /* 存檔失敗 */
                        /* 因為inACCUM_StoreRecord失敗，所以回傳VS_ERROR */
                        return (VS_ERROR);
                }

        }/* read、計算總額 和 存檔 成功 */

	/* 若斷電導致批次無法正確儲存時方便解釋 */
	inUtility_StoreTraceLog_OneStep("_TICKET_UPDATE_ACCUM_END_($:%ld)", pobTran->srTRec.lnTxnAmount);
        /* inACCUM_UpdateFlow()_END */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inACCUM_UpdateFlow()_END");
        }
        return (VS_SUCCESS);
}

/*
Function        :inACCUM_UpdateTotalAmount_Ticket
Date&Time       :2018/1/12 下午 3:28
Describe        :更新電票交易金額
*/
int inACCUM_UpdateTotalAmount_Ticket(TRANSACTION_OBJECT *pobTran, TICKET_ACCUM_TOTAL_REC *srAccumRec)
{
	char	szDebugMsg[100 + 1];

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "inACCUM_UpdateTotalAmount_Ticket(%d) START !", pobTran->srBRec.inCode);
		inLogPrintf(AT, szDebugMsg);
	}

        switch (pobTran->srTRec.inCode)
        {
                case _TICKET_IPASS_DEDUCT_ :
                        /* 計算筆數 */
                	srAccumRec->lnTotalCount ++;
                	srAccumRec->lnIPASS_TotalCount ++;
                	srAccumRec->lnDeductTotalCount ++;
                	srAccumRec->lnIPASS_DeductTotalCount ++;
                	/* 計算金額 */
                	srAccumRec->llTotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llIPASS_TotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llDeductTotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llIPASS_DeductTotalAmount += pobTran->srTRec.lnTxnAmount;
			
			/* 【需求單-108215】電票交易支援優惠平台需求 by Russell 2020/5/11 下午 3:53 */
			if ((pobTran->srTRec.uszRewardL1Bit == VS_TRUE	||
			     pobTran->srTRec.uszRewardL2Bit == VS_TRUE	||
			     pobTran->srTRec.uszRewardL5Bit == VS_TRUE)	&&
			     pobTran->srTRec.uszRewardSuspendBit != VS_TRUE)
			{
				srAccumRec->lnIPASS_RewardTotalCount ++;
			}
			
                        break;
                case _TICKET_IPASS_VOID_TOP_UP_ :
                        /* 計算筆數 */
                	srAccumRec->lnTotalCount ++;
                	srAccumRec->lnIPASS_TotalCount ++;
                	srAccumRec->lnVoidADDTotalCount ++;
                	srAccumRec->lnIPASS_VoidADDTotalCount ++;
                	/* 計算金額 */
                	srAccumRec->llTotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llIPASS_TotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llVoidADDTotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llIPASS_VoidADDTotalAmount += pobTran->srTRec.lnTxnAmount;
                        break;
                case _TICKET_IPASS_REFUND_ :
                        /* 計算筆數 */
                	srAccumRec->lnTotalCount ++;
                	srAccumRec->lnIPASS_TotalCount ++;
                	srAccumRec->lnRefundTotalCount ++;
                	srAccumRec->lnIPASS_RefundTotalCount ++;
                	/* 計算金額 */
                	srAccumRec->llTotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llIPASS_TotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llRefundTotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llIPASS_RefundTotalAmount += pobTran->srTRec.lnTxnAmount;
                        break;
                case _TICKET_IPASS_TOP_UP_ :
                        /* 計算筆數 */
                	srAccumRec->lnTotalCount ++;
                	srAccumRec->lnIPASS_TotalCount ++;
                	srAccumRec->lnADDTotalCount ++;
                	srAccumRec->lnIPASS_ADDTotalCount ++;
                	/* 計算金額 */
                	srAccumRec->llTotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llIPASS_TotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llADDTotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llIPASS_ADDTotalAmount += pobTran->srTRec.lnTxnAmount;
                        break;
                case _TICKET_IPASS_AUTO_TOP_UP_ :
                        /* 計算筆數 */
                        srAccumRec->lnTotalCount ++;
                	srAccumRec->lnIPASS_TotalCount ++;
                	srAccumRec->lnAutoADDTotalCount ++;
                	srAccumRec->lnIPASS_AutoADDTotalCount ++;
                	/* 計算金額 */
                	srAccumRec->llTotalAmount += pobTran->srTRec.lnTotalTopUpAmount;
                	srAccumRec->llIPASS_TotalAmount += pobTran->srTRec.lnTotalTopUpAmount;
                	srAccumRec->llAutoADDTotalAmount += pobTran->srTRec.lnTotalTopUpAmount;
                	srAccumRec->llIPASS_AutoADDTotalAmount += pobTran->srTRec.lnTotalTopUpAmount;
                        break;
                case _TICKET_EASYCARD_DEDUCT_ :
                        /* 計算筆數 */
                	srAccumRec->lnTotalCount ++;
                	srAccumRec->lnEASYCARD_TotalCount ++;
                	srAccumRec->lnDeductTotalCount ++;
                	srAccumRec->lnEASYCARD_DeductTotalCount ++;
                	/* 計算金額 */
                	srAccumRec->llTotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llEASYCARD_TotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llDeductTotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llEASYCARD_DeductTotalAmount += pobTran->srTRec.lnTxnAmount;
			
			/* 【需求單-108215】電票交易支援優惠平台需求 by Russell 2020/5/11 下午 3:53 */
			if ((pobTran->srTRec.uszRewardL1Bit == VS_TRUE	||
			     pobTran->srTRec.uszRewardL2Bit == VS_TRUE	||
			     pobTran->srTRec.uszRewardL5Bit == VS_TRUE)	&&
			     pobTran->srTRec.uszRewardSuspendBit != VS_TRUE)
			{
				srAccumRec->lnEASYCARD_RewardTotalCount ++;
			}
			
                        break;      
                case _TICKET_EASYCARD_VOID_TOP_UP_ :
                        /* 計算筆數 */
                	srAccumRec->lnTotalCount ++;
                	srAccumRec->lnEASYCARD_TotalCount ++;
                	srAccumRec->lnVoidADDTotalCount ++;
                	srAccumRec->lnEASYCARD_VoidADDTotalCount ++;
                	/* 計算金額 */
                	srAccumRec->llTotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llEASYCARD_TotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llVoidADDTotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llEASYCARD_VoidADDTotalAmount += pobTran->srTRec.lnTxnAmount;
                        break;         
                case _TICKET_EASYCARD_REFUND_ :
                        /* 計算筆數 */
                	srAccumRec->lnTotalCount ++;
                	srAccumRec->lnEASYCARD_TotalCount ++;
                	srAccumRec->lnRefundTotalCount ++;
                	srAccumRec->lnEASYCARD_RefundTotalCount ++;
                	/* 計算金額 */
                	srAccumRec->llTotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llEASYCARD_TotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llRefundTotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llEASYCARD_RefundTotalAmount += pobTran->srTRec.lnTxnAmount;
                        break;  
                case _TICKET_EASYCARD_TOP_UP_ :
                        /* 計算筆數 */
                	srAccumRec->lnTotalCount ++;
                	srAccumRec->lnEASYCARD_TotalCount ++;
                	srAccumRec->lnADDTotalCount ++;
                	srAccumRec->lnEASYCARD_ADDTotalCount ++;
                	/* 計算金額 */
                	srAccumRec->llTotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llEASYCARD_TotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llADDTotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llEASYCARD_ADDTotalAmount += pobTran->srTRec.lnTxnAmount;
                        break;           
                case _TICKET_EASYCARD_AUTO_TOP_UP_ :
                        /* 計算筆數 */
                        srAccumRec->lnTotalCount ++;
                	srAccumRec->lnEASYCARD_TotalCount ++;
                	srAccumRec->lnAutoADDTotalCount ++;
                	srAccumRec->lnEASYCARD_AutoADDTotalCount ++;
                	/* 計算金額 */
                	srAccumRec->llTotalAmount += pobTran->srTRec.lnTotalTopUpAmount;
                	srAccumRec->llEASYCARD_TotalAmount += pobTran->srTRec.lnTotalTopUpAmount;
                	srAccumRec->llAutoADDTotalAmount += pobTran->srTRec.lnTotalTopUpAmount;
                	srAccumRec->llEASYCARD_AutoADDTotalAmount += pobTran->srTRec.lnTotalTopUpAmount;
                        break;
                case _TICKET_ICASH_DEDUCT_ :
                        /* 計算筆數 */
                	srAccumRec->lnTotalCount ++;
                	srAccumRec->lnICASH_TotalCount ++;
                	srAccumRec->lnDeductTotalCount ++;
                	srAccumRec->lnICASH_DeductTotalCount ++;
                	/* 計算金額 */
                	srAccumRec->llTotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llICASH_TotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llDeductTotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llICASH_DeductTotalAmount += pobTran->srTRec.lnTxnAmount;
			
			/* 【需求單-108215】電票交易支援優惠平台需求 by Russell 2020/5/11 下午 3:53 */
			if ((pobTran->srTRec.uszRewardL1Bit == VS_TRUE	||
			     pobTran->srTRec.uszRewardL2Bit == VS_TRUE	||
			     pobTran->srTRec.uszRewardL5Bit == VS_TRUE)	&&
			     pobTran->srTRec.uszRewardSuspendBit != VS_TRUE)
			{
				srAccumRec->lnICASH_RewardTotalCount ++;
			}
			
                        break;
                case _TICKET_ICASH_VOID_TOP_UP_ :
                        /* 計算筆數 */
                	srAccumRec->lnTotalCount ++;
                	srAccumRec->lnICASH_TotalCount ++;
                	srAccumRec->lnVoidADDTotalCount ++;
                	srAccumRec->lnICASH_VoidADDTotalCount ++;
                	/* 計算金額 */
                	srAccumRec->llTotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llICASH_TotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llVoidADDTotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llICASH_VoidADDTotalAmount += pobTran->srTRec.lnTxnAmount;
                        break;
                case _TICKET_ICASH_REFUND_ :
                        /* 計算筆數 */
                	srAccumRec->lnTotalCount ++;
                	srAccumRec->lnICASH_TotalCount ++;
                	srAccumRec->lnRefundTotalCount ++;
                	srAccumRec->lnICASH_RefundTotalCount ++;
                	/* 計算金額 */
                	srAccumRec->llTotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llICASH_TotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llRefundTotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llICASH_RefundTotalAmount += pobTran->srTRec.lnTxnAmount;
                        break;
                case _TICKET_ICASH_TOP_UP_ :
                        /* 計算筆數 */
                	srAccumRec->lnTotalCount ++;
                	srAccumRec->lnICASH_TotalCount ++;
                	srAccumRec->lnADDTotalCount ++;
                	srAccumRec->lnICASH_ADDTotalCount ++;
                	/* 計算金額 */
                	srAccumRec->llTotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llICASH_TotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llADDTotalAmount += pobTran->srTRec.lnTxnAmount;
                	srAccumRec->llICASH_ADDTotalAmount += pobTran->srTRec.lnTxnAmount;
                        break;
                case _TICKET_ICASH_AUTO_TOP_UP_ :
                        /* 計算筆數 */
                        srAccumRec->lnTotalCount ++;
                	srAccumRec->lnICASH_TotalCount ++;
                	srAccumRec->lnAutoADDTotalCount ++;
                	srAccumRec->lnICASH_AutoADDTotalCount ++;
                	/* 計算金額 */
                	srAccumRec->llTotalAmount += pobTran->srTRec.lnTotalTopUpAmount;
                	srAccumRec->llICASH_TotalAmount += pobTran->srTRec.lnTotalTopUpAmount;
                	srAccumRec->llAutoADDTotalAmount += pobTran->srTRec.lnTotalTopUpAmount;
                	srAccumRec->llICASH_AutoADDTotalAmount += pobTran->srTRec.lnTotalTopUpAmount;
                        break;
                default :
                        return (VS_ERROR);
        }

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inACCUM_UpdateTotalAmount_Ticket() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inACCUM_StoreRecord_Ticket
Date&Time       :2018/1/12 下午 3:42
Describe        :再做加總後，將資料存回record，回傳(VS_SUCCESS) or (VS_ERROR)
*/
int inACCUM_StoreRecord_Ticket(TICKET_ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszFileName)
{
        unsigned long   ulHandlePtr;    /* File Handle，type為pointer */
        int		inRetVal; /* return value，來判斷是否回傳error */

        /* inACCUM_StoreRecord()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inACCUM_StoreRecord()_START");
        }

        /*開檔*/
        inRetVal = inFILE_Open(&ulHandlePtr, uszFileName);
        /*開檔成功*/
        if (inRetVal != VS_SUCCESS)
        {
                /* 開檔失敗時，不必關檔(開檔失敗，handle回傳NULL) */
                /* 開檔失敗，所以回傳error */
                return(VS_ERROR);
        }

        /* 把指針指到開頭*/
        inRetVal = inFILE_Seek(ulHandlePtr, 0, _SEEK_BEGIN_);

        /* seek不成功時 */
        if (inRetVal != VS_SUCCESS)
        {
                /* inFILE_Seek失敗時 */
                /* 關檔並回傳VS_ERROR */
                inFILE_Close(&ulHandlePtr);
                /* seek失敗，所以回傳error。(關檔不論成功與否都要回傳(VS_ERROR)) */
                return (VS_ERROR);
        }

        /* 寫檔 */
        if (inFILE_Write(&ulHandlePtr,(unsigned char*)srAccumRec, _TICKET_ACCUM_REC_SIZE_) != VS_SUCCESS)
        {
                /*寫檔失敗時*/
                /* 關檔  */
                inFILE_Close(&ulHandlePtr);
                /* 寫檔失敗，所以回傳error。(關檔不論成功與否都要回傳(VS_ERROR)) */
                return (VS_ERROR);
        }

        /* 關檔  */
        if (inFILE_Close(&ulHandlePtr) != VS_SUCCESS)
        {
                /*關檔失敗時*/
                /* 關檔失敗，所以回傳error */
                return (VS_ERROR);
        }

        /* inACCUM_StoreRecord()_END */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inACCUM_StoreRecord()_END");
        }

        return (VS_SUCCESS);
}

/* 【需求單 - 108097】Card BIN由ATS查核 add by LingHsiung 2020/2/3 下午 5:19 */
/* 新增未知卡別小計 add by LingHsiung 2020/7/7 下午 2:26 */
int inACCUM_UpdateTotalAmountByUnknownCard(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec)
{
	int		inCDTXIndex;
	char		szUnknownCardLabel[20 + 1];
	char            szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
	unsigned char	uszCheckCard = VS_FALSE;

        /* inACCUM_UpdateTotalAmountByUnknownCard()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inACCUM_UpdateTotalAmountByUnknownCard()_START");
        }
	
	/* 先比對未知卡別要存的Accum位置 */
	for (inCDTXIndex = 0; inCDTXIndex < 10; inCDTXIndex ++)
	{
		inLoadCDTXRec(inCDTXIndex);
		memset(szUnknownCardLabel, 0x00, sizeof(szUnknownCardLabel));
		inGetUnknownCardLabel(szUnknownCardLabel);

		/* 比對卡別 */
		if (!memcmp(&szUnknownCardLabel[0], &pobTran->srBRec.szCardLabel[0], strlen(szUnknownCardLabel)))
		{
			uszCheckCard = VS_TRUE;
			break;
		}
	}
	
	if (uszCheckCard != VS_TRUE)
	{
		memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
		sprintf(szErrorMsg, "CDTX Undefine Cardlabel ERROR!!");
		/* 記Log */
		inUtility_StoreTraceLog_OneStep(szErrorMsg);
		memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
		sprintf(szErrorMsg, "Trans code = %d Card Label = %s", pobTran->srBRec.inCode, pobTran->srBRec.szCardLabel);
		inUtility_StoreTraceLog_OneStep(szErrorMsg);
		return (VS_ERROR);
	}
	
        /* 不是取消的交易 */
        if (pobTran->srBRec.uszVOIDBit == VS_FALSE)
        {
                switch (pobTran->srBRec.inCode)
                {
                        case _SALE_:
                        case _SALE_OFFLINE_:/* Offline Sale */
                        case _INST_SALE_:
                        case _REDEEM_SALE_:
			case _CASH_ADVANCE_ :
			case _FORCE_CASH_ADVANCE_ :
			case _PRE_COMP_ :
			case _CUP_SALE_ :
			case _CUP_PRE_COMP_ :
			case _FISC_SALE_ :
			case _INST_ADJUST_ :
			case _REDEEM_ADJUST_ :
			case _MAIL_ORDER_ :
			case _CUP_MAIL_ORDER_ :
				if (inCDTXIndex == 0)
				{
					/* 計算筆數 */
					srAccumRec->lnX0TotalSaleCount ++;
					srAccumRec->lnX0TotalCount ++;
					/* 計算金額 */
					srAccumRec->llX0TotalSaleAmount += pobTran->srBRec.lnTxnAmount;
					srAccumRec->llX0TotalAmount += pobTran->srBRec.lnTxnAmount;
				}
				else if (inCDTXIndex == 1)
				{
					/* 計算筆數 */
					srAccumRec->lnX1TotalSaleCount ++;
					srAccumRec->lnX1TotalCount ++;
					/* 計算金額 */
					srAccumRec->llX1TotalSaleAmount += pobTran->srBRec.lnTxnAmount;
					srAccumRec->llX1TotalAmount += pobTran->srBRec.lnTxnAmount;
				}
				else if (inCDTXIndex == 2)
				{
					/* 計算筆數 */
					srAccumRec->lnX2TotalSaleCount ++;
					srAccumRec->lnX2TotalCount ++;
					/* 計算金額 */
					srAccumRec->llX2TotalSaleAmount += pobTran->srBRec.lnTxnAmount;
					srAccumRec->llX2TotalAmount += pobTran->srBRec.lnTxnAmount;
				}
				else if (inCDTXIndex == 3)
				{
					/* 計算筆數 */
					srAccumRec->lnX3TotalSaleCount ++;
					srAccumRec->lnX3TotalCount ++;
					/* 計算金額 */
					srAccumRec->llX3TotalSaleAmount += pobTran->srBRec.lnTxnAmount;
					srAccumRec->llX3TotalAmount += pobTran->srBRec.lnTxnAmount;
				}
				else if (inCDTXIndex == 4)
				{
					/* 計算筆數 */
					srAccumRec->lnX4TotalSaleCount ++;
					srAccumRec->lnX4TotalCount ++;
					/* 計算金額 */
					srAccumRec->llX4TotalSaleAmount += pobTran->srBRec.lnTxnAmount;
					srAccumRec->llX4TotalAmount += pobTran->srBRec.lnTxnAmount;
				}
				else if (inCDTXIndex == 5)
				{
					/* 計算筆數 */
					srAccumRec->lnX5TotalSaleCount ++;
					srAccumRec->lnX5TotalCount ++;
					/* 計算金額 */
					srAccumRec->llX5TotalSaleAmount += pobTran->srBRec.lnTxnAmount;
					srAccumRec->llX5TotalAmount += pobTran->srBRec.lnTxnAmount;
				}						
				else if (inCDTXIndex == 6)
				{
					/* 計算筆數 */
					srAccumRec->lnX6TotalSaleCount ++;
					srAccumRec->lnX6TotalCount ++;
					/* 計算金額 */
					srAccumRec->llX6TotalSaleAmount += pobTran->srBRec.lnTxnAmount;
					srAccumRec->llX6TotalAmount += pobTran->srBRec.lnTxnAmount;
				}
				else if (inCDTXIndex == 7)
				{
					/* 計算筆數 */
					srAccumRec->lnX7TotalSaleCount ++;
					srAccumRec->lnX7TotalCount ++;
					/* 計算金額 */
					srAccumRec->llX7TotalSaleAmount += pobTran->srBRec.lnTxnAmount;
					srAccumRec->llX7TotalAmount += pobTran->srBRec.lnTxnAmount;
				}
				else if (inCDTXIndex == 8)
				{
					/* 計算筆數 */
					srAccumRec->lnX8TotalSaleCount ++;
					srAccumRec->lnX8TotalCount ++;
					/* 計算金額 */
					srAccumRec->llX8TotalSaleAmount += pobTran->srBRec.lnTxnAmount;
					srAccumRec->llX8TotalAmount += pobTran->srBRec.lnTxnAmount;
				}
				else if (inCDTXIndex ==9)
				{
					/* 計算筆數 */
					srAccumRec->lnX9TotalSaleCount ++;
					srAccumRec->lnX9TotalCount ++;
					/* 計算金額 */
					srAccumRec->llX9TotalSaleAmount += pobTran->srBRec.lnTxnAmount;
					srAccumRec->llX9TotalAmount += pobTran->srBRec.lnTxnAmount;
				}
				else
				{
					memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
					sprintf(szErrorMsg, "inACCUM_UpdateTotalAmountByUnknownCard ERROR!!");
					/* 記Log */
					inUtility_StoreTraceLog_OneStep(szErrorMsg);
					memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
					sprintf(szErrorMsg, "Trans code = %d Card Label = %s", pobTran->srBRec.inCode, pobTran->srBRec.szCardLabel);
					inUtility_StoreTraceLog_OneStep(szErrorMsg);
				}
				break;
			case _REFUND_:
			case _CUP_REFUND_:
			case _CUP_MAIL_ORDER_REFUND_ :
                        case _INST_REFUND_:
                        case _REDEEM_REFUND_:
			case _FISC_REFUND_ :
				if (inCDTXIndex == 0)
				{
					/* 計算筆數 */
					srAccumRec->lnX0TotalRefundCount ++;
					srAccumRec->lnX0TotalCount ++;
					/* 計算金額 */
					srAccumRec->llX0TotalRefundAmount += (0 - pobTran->srBRec.lnTxnAmount);
					srAccumRec->llX0TotalAmount += (0 - pobTran->srBRec.lnTxnAmount);
				}
				else if (inCDTXIndex == 1)
				{
					/* 計算筆數 */
					srAccumRec->lnX1TotalRefundCount ++;
					srAccumRec->lnX1TotalCount ++;
					/* 計算金額 */
					srAccumRec->llX1TotalRefundAmount += (0 - pobTran->srBRec.lnTxnAmount);
					srAccumRec->llX1TotalAmount += (0 - pobTran->srBRec.lnTxnAmount);					
				}
				else if (inCDTXIndex == 2)
				{
					/* 計算筆數 */
					srAccumRec->lnX2TotalRefundCount ++;
					srAccumRec->lnX2TotalCount ++;
					/* 計算金額 */
					srAccumRec->llX2TotalRefundAmount += (0 - pobTran->srBRec.lnTxnAmount);
					srAccumRec->llX2TotalAmount += (0 - pobTran->srBRec.lnTxnAmount);					
				}
				else if (inCDTXIndex == 3)
				{
					/* 計算筆數 */
					srAccumRec->lnX3TotalRefundCount ++;
					srAccumRec->lnX3TotalCount ++;
					/* 計算金額 */
					srAccumRec->llX3TotalRefundAmount += (0 - pobTran->srBRec.lnTxnAmount);
					srAccumRec->llX3TotalAmount += (0 - pobTran->srBRec.lnTxnAmount);					
				}
				else if (inCDTXIndex == 4)
				{
					/* 計算筆數 */
					srAccumRec->lnX4TotalRefundCount ++;
					srAccumRec->lnX4TotalCount ++;
					/* 計算金額 */
					srAccumRec->llX4TotalRefundAmount += (0 - pobTran->srBRec.lnTxnAmount);
					srAccumRec->llX4TotalAmount += (0 - pobTran->srBRec.lnTxnAmount);					
				}
				else if (inCDTXIndex == 5)
				{
					/* 計算筆數 */
					srAccumRec->lnX5TotalRefundCount ++;
					srAccumRec->lnX5TotalCount ++;
					/* 計算金額 */
					srAccumRec->llX5TotalRefundAmount += (0 - pobTran->srBRec.lnTxnAmount);
					srAccumRec->llX5TotalAmount += (0 - pobTran->srBRec.lnTxnAmount);					
				}						
				else if (inCDTXIndex == 6)
				{	
					/* 計算筆數 */
					srAccumRec->lnX6TotalRefundCount ++;
					srAccumRec->lnX6TotalCount ++;
					/* 計算金額 */
					srAccumRec->llX6TotalRefundAmount += (0 - pobTran->srBRec.lnTxnAmount);
					srAccumRec->llX6TotalAmount += (0 - pobTran->srBRec.lnTxnAmount);
				}
				else if (inCDTXIndex == 7)
				{
					/* 計算筆數 */
					srAccumRec->lnX7TotalRefundCount ++;
					srAccumRec->lnX7TotalCount ++;
					/* 計算金額 */
					srAccumRec->llX7TotalRefundAmount += (0 - pobTran->srBRec.lnTxnAmount);
					srAccumRec->llX7TotalAmount += (0 - pobTran->srBRec.lnTxnAmount);					
				}
				else if (inCDTXIndex == 8)
				{
					/* 計算筆數 */
					srAccumRec->lnX8TotalRefundCount ++;
					srAccumRec->lnX8TotalCount ++;
					/* 計算金額 */
					srAccumRec->llX8TotalRefundAmount += (0 - pobTran->srBRec.lnTxnAmount);
					srAccumRec->llX8TotalAmount += (0 - pobTran->srBRec.lnTxnAmount);					
				}
				else if (inCDTXIndex == 9)
				{
					/* 計算筆數 */
					srAccumRec->lnX9TotalRefundCount ++;
					srAccumRec->lnX9TotalCount ++;
					/* 計算金額 */
					srAccumRec->llX9TotalRefundAmount += (0 - pobTran->srBRec.lnTxnAmount);
					srAccumRec->llX9TotalAmount += (0 - pobTran->srBRec.lnTxnAmount);					
				}
				else
				{
					memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
					sprintf(szErrorMsg, "inACCUM_UpdateTotalAmountByUnknownCard ERROR!!");
					/* 記Log */
					inUtility_StoreTraceLog_OneStep(szErrorMsg);
					memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
					sprintf(szErrorMsg, "Trans code = %d Card Label = %s", pobTran->srBRec.inCode, pobTran->srBRec.szCardLabel);
					inUtility_StoreTraceLog_OneStep(szErrorMsg);
				}
				break;
			case _TIP_:
				if (inCDTXIndex == 0)
				{
					/* 計算筆數 */
					srAccumRec->lnX0TotalTipsCount++;
					/* 計算金額 */
					srAccumRec->llX0TotalTipsAmount += pobTran->srBRec.lnTipTxnAmount;
					srAccumRec->llX0TotalAmount += pobTran->srBRec.lnTipTxnAmount;
				}
				else if (inCDTXIndex == 1)
				{
					/* 計算筆數 */
					srAccumRec->lnX1TotalTipsCount++;
					/* 計算金額 */
					srAccumRec->llX1TotalTipsAmount += pobTran->srBRec.lnTipTxnAmount;
					srAccumRec->llX1TotalAmount += pobTran->srBRec.lnTipTxnAmount;
				}
				else if (inCDTXIndex == 2)
				{
					/* 計算筆數 */
					srAccumRec->lnX2TotalTipsCount++;
					/* 計算金額 */
					srAccumRec->llX2TotalTipsAmount += pobTran->srBRec.lnTipTxnAmount;
					srAccumRec->llX2TotalAmount += pobTran->srBRec.lnTipTxnAmount;
				}
				else if (inCDTXIndex == 3)
				{
					/* 計算筆數 */
					srAccumRec->lnX3TotalTipsCount++;
					/* 計算金額 */
					srAccumRec->llX3TotalTipsAmount += pobTran->srBRec.lnTipTxnAmount;
					srAccumRec->llX3TotalAmount += pobTran->srBRec.lnTipTxnAmount;
				}
				else if (inCDTXIndex == 4)
				{
					/* 計算筆數 */
					srAccumRec->lnX4TotalTipsCount++;
					/* 計算金額 */
					srAccumRec->llX4TotalTipsAmount += pobTran->srBRec.lnTipTxnAmount;
					srAccumRec->llX4TotalAmount += pobTran->srBRec.lnTipTxnAmount;
				}
				else if (inCDTXIndex == 5)
				{
					/* 計算筆數 */
					srAccumRec->lnX5TotalTipsCount++;
					/* 計算金額 */
					srAccumRec->llX5TotalTipsAmount += pobTran->srBRec.lnTipTxnAmount;
					srAccumRec->llX5TotalAmount += pobTran->srBRec.lnTipTxnAmount;
				}						
				else if (inCDTXIndex == 6)
				{
					/* 計算筆數 */
					srAccumRec->lnX6TotalTipsCount++;
					/* 計算金額 */
					srAccumRec->llX6TotalTipsAmount += pobTran->srBRec.lnTipTxnAmount;
					srAccumRec->llX6TotalAmount += pobTran->srBRec.lnTipTxnAmount;
				}
				else if (inCDTXIndex == 7)
				{
					/* 計算筆數 */
					srAccumRec->lnX7TotalTipsCount++;
					/* 計算金額 */
					srAccumRec->llX7TotalTipsAmount += pobTran->srBRec.lnTipTxnAmount;
					srAccumRec->llX7TotalAmount += pobTran->srBRec.lnTipTxnAmount;
				}
				else if (inCDTXIndex == 8)
				{
					/* 計算筆數 */
					srAccumRec->lnX8TotalTipsCount++;
					/* 計算金額 */
					srAccumRec->llX8TotalTipsAmount += pobTran->srBRec.lnTipTxnAmount;
					srAccumRec->llX8TotalAmount += pobTran->srBRec.lnTipTxnAmount;
				}
				else if (inCDTXIndex == 9)
				{
					/* 計算筆數 */
					srAccumRec->lnX9TotalTipsCount++;
					/* 計算金額 */
					srAccumRec->llX9TotalTipsAmount += pobTran->srBRec.lnTipTxnAmount;
					srAccumRec->llX9TotalAmount += pobTran->srBRec.lnTipTxnAmount;
				}
				else
				{
					memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
					sprintf(szErrorMsg, "inACCUM_UpdateTotalAmountByUnknownCard ERROR!!");
					/* 記Log */
					inUtility_StoreTraceLog_OneStep(szErrorMsg);
					memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
					sprintf(szErrorMsg, "Trans code = %d Card Label = %s", pobTran->srBRec.inCode, pobTran->srBRec.szCardLabel);
					inUtility_StoreTraceLog_OneStep(szErrorMsg);
				}				
				break;
			case _ADJUST_:
				if (inCDTXIndex == 0)
				{
                                        /* 計算金額 */
                                        srAccumRec->llX0TotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX0TotalAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX0TotalSaleAmount += pobTran->srBRec.lnAdjustTxnAmount;
                                        srAccumRec->llX0TotalAmount += pobTran->srBRec.lnAdjustTxnAmount;
				}
				else if (inCDTXIndex == 1)
				{
                                        srAccumRec->llX1TotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX1TotalAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX1TotalSaleAmount += pobTran->srBRec.lnAdjustTxnAmount;
                                        srAccumRec->llX1TotalAmount += pobTran->srBRec.lnAdjustTxnAmount;
				}
				else if (inCDTXIndex == 2)
				{
                                        srAccumRec->llX2TotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX2TotalAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX2TotalSaleAmount += pobTran->srBRec.lnAdjustTxnAmount;
                                        srAccumRec->llX2TotalAmount += pobTran->srBRec.lnAdjustTxnAmount;
				}
				else if (inCDTXIndex == 3)
				{
                                        srAccumRec->llX3TotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX3TotalAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX3TotalSaleAmount += pobTran->srBRec.lnAdjustTxnAmount;
                                        srAccumRec->llX3TotalAmount += pobTran->srBRec.lnAdjustTxnAmount;
				}
				else if (inCDTXIndex == 4)
				{
                                        srAccumRec->llX4TotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX4TotalAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX4TotalSaleAmount += pobTran->srBRec.lnAdjustTxnAmount;
                                        srAccumRec->llX4TotalAmount += pobTran->srBRec.lnAdjustTxnAmount;
				}
				else if (inCDTXIndex == 5)
				{
                                        srAccumRec->llX5TotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX5TotalAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX5TotalSaleAmount += pobTran->srBRec.lnAdjustTxnAmount;
                                        srAccumRec->llX5TotalAmount += pobTran->srBRec.lnAdjustTxnAmount;
				}						
				else if (inCDTXIndex == 6)
				{
                                        srAccumRec->llX6TotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX6TotalAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX6TotalSaleAmount += pobTran->srBRec.lnAdjustTxnAmount;
                                        srAccumRec->llX6TotalAmount += pobTran->srBRec.lnAdjustTxnAmount;
				}
				else if (inCDTXIndex == 7)
				{
                                        srAccumRec->llX7TotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX7TotalAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX7TotalSaleAmount += pobTran->srBRec.lnAdjustTxnAmount;
                                        srAccumRec->llX7TotalAmount += pobTran->srBRec.lnAdjustTxnAmount;
				}
				else if (inCDTXIndex == 8)
				{
                                        srAccumRec->llX8TotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX8TotalAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX8TotalSaleAmount += pobTran->srBRec.lnAdjustTxnAmount;
                                        srAccumRec->llX8TotalAmount += pobTran->srBRec.lnAdjustTxnAmount;
				}
				else if (inCDTXIndex == 9)
				{
                                        srAccumRec->llX9TotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX9TotalAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX9TotalSaleAmount += pobTran->srBRec.lnAdjustTxnAmount;
                                        srAccumRec->llX9TotalAmount += pobTran->srBRec.lnAdjustTxnAmount;
				}
				else
				{
					memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
					sprintf(szErrorMsg, "inACCUM_UpdateTotalAmountByUnknownCard ERROR!!");
					/* 記Log */
					inUtility_StoreTraceLog_OneStep(szErrorMsg);
					memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
					sprintf(szErrorMsg, "Trans code = %d Card Label = %s", pobTran->srBRec.inCode, pobTran->srBRec.szCardLabel);
					inUtility_StoreTraceLog_OneStep(szErrorMsg);
				}
				break;
			default :
                                /* 交易類別錯誤，(沒有此交易類別) */
                                /* debug */
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
				sprintf(szErrorMsg, "inACCUM_UpdateTotalAmountByUnknownCard : ERROR!! Trans code = %d", pobTran->srBRec.inCode);
				/* 鎖機記Log */
				inUtility_StoreTraceLog_OneStep(szErrorMsg);
				
				if (ginDebug == VS_TRUE)
					inLogPrintf(AT, szErrorMsg);

                                return (VS_ERROR);
		}

	}
        /* 取消的時候 */
        else
        {
                switch (pobTran->srBRec.inOrgCode)
                {
                        case _SALE_:
                        case _SALE_OFFLINE_:/* Offline Sale */
                        case _INST_SALE_:
                        case _REDEEM_SALE_:
			case _CASH_ADVANCE_ :
			case _FORCE_CASH_ADVANCE_ :
			case _PRE_COMP_ :
			case _CUP_SALE_ :
			case _CUP_PRE_COMP_ :
			case _FISC_SALE_ :
			case _INST_ADJUST_ :
			case _REDEEM_ADJUST_ :
			case _MAIL_ORDER_ :
			case _CUP_MAIL_ORDER_ :
				if (inCDTXIndex == 0)
				{
                                        /* 計算筆數 */
                                        srAccumRec->lnX0TotalSaleCount --;
                                        srAccumRec->lnX0TotalCount --;
                                        /* 計算金額 */
                                        srAccumRec->llX0TotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX0TotalAmount -= pobTran->srBRec.lnTxnAmount;
				}
				else if (inCDTXIndex == 1)
				{
                                        /* 計算筆數 */
                                        srAccumRec->lnX1TotalSaleCount --;
                                        srAccumRec->lnX1TotalCount --;
                                        /* 計算金額 */
                                        srAccumRec->llX1TotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX1TotalAmount -= pobTran->srBRec.lnTxnAmount;
				}
				else if (inCDTXIndex == 2)
				{
                                        /* 計算筆數 */
                                        srAccumRec->lnX2TotalSaleCount --;
                                        srAccumRec->lnX2TotalCount --;
                                        /* 計算金額 */
                                        srAccumRec->llX2TotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX2TotalAmount -= pobTran->srBRec.lnTxnAmount;
				}
				else if (inCDTXIndex == 3)
				{
                                        /* 計算筆數 */
                                        srAccumRec->lnX3TotalSaleCount --;
                                        srAccumRec->lnX3TotalCount --;
                                        /* 計算金額 */
                                        srAccumRec->llX3TotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX3TotalAmount -= pobTran->srBRec.lnTxnAmount;
				}
				else if (inCDTXIndex == 4)
				{
                                        /* 計算筆數 */
                                        srAccumRec->lnX4TotalSaleCount --;
                                        srAccumRec->lnX4TotalCount --;
                                        /* 計算金額 */
                                        srAccumRec->llX4TotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX4TotalAmount -= pobTran->srBRec.lnTxnAmount;
				}
				else if (inCDTXIndex == 5)
				{
                                        /* 計算筆數 */
                                        srAccumRec->lnX5TotalSaleCount --;
                                        srAccumRec->lnX5TotalCount --;
                                        /* 計算金額 */
                                        srAccumRec->llX5TotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX5TotalAmount -= pobTran->srBRec.lnTxnAmount;
				}						
				else if (inCDTXIndex == 6)
				{
                                        /* 計算筆數 */
                                        srAccumRec->lnX6TotalSaleCount --;
                                        srAccumRec->lnX6TotalCount --;
                                        /* 計算金額 */
                                        srAccumRec->llX6TotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX6TotalAmount -= pobTran->srBRec.lnTxnAmount;
				}
				else if (inCDTXIndex == 7)
				{
                                        /* 計算筆數 */
                                        srAccumRec->lnX7TotalSaleCount --;
                                        srAccumRec->lnX7TotalCount --;
                                        /* 計算金額 */
                                        srAccumRec->llX7TotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX7TotalAmount -= pobTran->srBRec.lnTxnAmount;
				}
				else if (inCDTXIndex == 8)
				{
                                        /* 計算筆數 */
                                        srAccumRec->lnX8TotalSaleCount --;
                                        srAccumRec->lnX8TotalCount --;
                                        /* 計算金額 */
                                        srAccumRec->llX8TotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX8TotalAmount -= pobTran->srBRec.lnTxnAmount;
				}
				else if (inCDTXIndex == 9)
				{
                                        /* 計算筆數 */
                                        srAccumRec->lnX9TotalSaleCount --;
                                        srAccumRec->lnX9TotalCount --;
                                        /* 計算金額 */
                                        srAccumRec->llX9TotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX9TotalAmount -= pobTran->srBRec.lnTxnAmount;
				}
				else
				{
					memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
					sprintf(szErrorMsg, "inACCUM_UpdateTotalAmountByUnknownCard ERROR!!");
					/* 記Log */
					inUtility_StoreTraceLog_OneStep(szErrorMsg);
					memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
					sprintf(szErrorMsg, "Trans code = %d Card Label = %s", pobTran->srBRec.inCode, pobTran->srBRec.szCardLabel);
					inUtility_StoreTraceLog_OneStep(szErrorMsg);
				}
                                break;
                        case _REFUND_:
			case _CUP_REFUND_:
			case _CUP_MAIL_ORDER_REFUND_ :
                        case _INST_REFUND_:
                        case _REDEEM_REFUND_:
				if (inCDTXIndex == 0)
				{
                                        /* 計算筆數 */
                                        srAccumRec->lnX0TotalRefundCount --;
                                        srAccumRec->lnX0TotalCount --;
                                        /* 計算金額 */
                                        srAccumRec->llX0TotalRefundAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX0TotalAmount += pobTran->srBRec.lnTxnAmount;
				}
				else if (inCDTXIndex == 1)
				{
                                        /* 計算筆數 */
                                        srAccumRec->lnX1TotalRefundCount --;
                                        srAccumRec->lnX1TotalCount --;
                                        /* 計算金額 */
                                        srAccumRec->llX1TotalRefundAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX1TotalAmount += pobTran->srBRec.lnTxnAmount;
				}
				else if (inCDTXIndex == 2)
				{
                                        /* 計算筆數 */
                                        srAccumRec->lnX2TotalRefundCount --;
                                        srAccumRec->lnX2TotalCount --;
                                        /* 計算金額 */
                                        srAccumRec->llX2TotalRefundAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX2TotalAmount += pobTran->srBRec.lnTxnAmount;
				}
				else if (inCDTXIndex == 3)
				{
                                        /* 計算筆數 */
                                        srAccumRec->lnX3TotalRefundCount --;
                                        srAccumRec->lnX3TotalCount --;
                                        /* 計算金額 */
                                        srAccumRec->llX3TotalRefundAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX3TotalAmount += pobTran->srBRec.lnTxnAmount;
				}
				else if (inCDTXIndex == 4)
				{
                                        /* 計算筆數 */
                                        srAccumRec->lnX4TotalRefundCount --;
                                        srAccumRec->lnX4TotalCount --;
                                        /* 計算金額 */
                                        srAccumRec->llX4TotalRefundAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX4TotalAmount += pobTran->srBRec.lnTxnAmount;
				}
				else if (inCDTXIndex == 5)
				{
                                        /* 計算筆數 */
                                        srAccumRec->lnX5TotalRefundCount --;
                                        srAccumRec->lnX5TotalCount --;
                                        /* 計算金額 */
                                        srAccumRec->llX5TotalRefundAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX5TotalAmount += pobTran->srBRec.lnTxnAmount;
				}						
				else if (inCDTXIndex == 6)
				{
                                        /* 計算筆數 */
                                        srAccumRec->lnX6TotalRefundCount --;
                                        srAccumRec->lnX6TotalCount --;
                                        /* 計算金額 */
                                        srAccumRec->llX6TotalRefundAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX6TotalAmount += pobTran->srBRec.lnTxnAmount;
				}
				else if (inCDTXIndex == 7)
				{
                                        /* 計算筆數 */
                                        srAccumRec->lnX7TotalRefundCount --;
                                        srAccumRec->lnX7TotalCount --;
                                        /* 計算金額 */
                                        srAccumRec->llX7TotalRefundAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX7TotalAmount += pobTran->srBRec.lnTxnAmount;
				}
				else if (inCDTXIndex == 8)
				{
                                        /* 計算筆數 */
                                        srAccumRec->lnX8TotalRefundCount --;
                                        srAccumRec->lnX8TotalCount --;
                                        /* 計算金額 */
                                        srAccumRec->llX8TotalRefundAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX8TotalAmount += pobTran->srBRec.lnTxnAmount;
				}
				else if (inCDTXIndex == 9)
				{
                                        /* 計算筆數 */
                                        srAccumRec->lnX9TotalRefundCount --;
                                        srAccumRec->lnX9TotalCount --;
                                        /* 計算金額 */
                                        srAccumRec->llX9TotalRefundAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX9TotalAmount += pobTran->srBRec.lnTxnAmount;
				}
				else
				{
					memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
					sprintf(szErrorMsg, "inACCUM_UpdateTotalAmountByUnknownCard ERROR!!");
					/* 記Log */
					inUtility_StoreTraceLog_OneStep(szErrorMsg);
					memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
					sprintf(szErrorMsg, "Trans code = %d Card Label = %s", pobTran->srBRec.inCode, pobTran->srBRec.szCardLabel);
					inUtility_StoreTraceLog_OneStep(szErrorMsg);
				}
                                break;
                        case _TIP_:
				if (inCDTXIndex == 0)
				{
                                        /* 取消整筆交易，計算筆數 */
                                        srAccumRec->lnX0TotalTipsCount --;
                                        srAccumRec->lnX0TotalSaleCount --;
                                        srAccumRec->lnX0TotalCount --;
                                        /* 取消整筆交易，計算金額 */
                                        srAccumRec->llX0TotalTipsAmount -= pobTran->srBRec.lnTipTxnAmount;
                                        srAccumRec->llX0TotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX0TotalAmount -= (pobTran->srBRec.lnTipTxnAmount + pobTran->srBRec.lnTxnAmount);
				}
				else if (inCDTXIndex == 1)
				{
                                        /* 取消整筆交易，計算筆數 */
                                        srAccumRec->lnX1TotalTipsCount --;
                                        srAccumRec->lnX1TotalSaleCount --;
                                        srAccumRec->lnX1TotalCount --;
                                        /* 取消整筆交易，計算金額 */
                                        srAccumRec->llX1TotalTipsAmount -= pobTran->srBRec.lnTipTxnAmount;
                                        srAccumRec->llX1TotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX1TotalAmount -= (pobTran->srBRec.lnTipTxnAmount + pobTran->srBRec.lnTxnAmount);
				}
				else if (inCDTXIndex == 2)
				{
                                        /* 取消整筆交易，計算筆數 */
                                        srAccumRec->lnX2TotalTipsCount --;
                                        srAccumRec->lnX2TotalSaleCount --;
                                        srAccumRec->lnX2TotalCount --;
                                        /* 取消整筆交易，計算金額 */
                                        srAccumRec->llX2TotalTipsAmount -= pobTran->srBRec.lnTipTxnAmount;
                                        srAccumRec->llX2TotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX2TotalAmount -= (pobTran->srBRec.lnTipTxnAmount + pobTran->srBRec.lnTxnAmount);
				}
				else if (inCDTXIndex == 3)
				{
                                        /* 取消整筆交易，計算筆數 */
                                        srAccumRec->lnX3TotalTipsCount --;
                                        srAccumRec->lnX3TotalSaleCount --;
                                        srAccumRec->lnX3TotalCount --;
                                        /* 取消整筆交易，計算金額 */
                                        srAccumRec->llX3TotalTipsAmount -= pobTran->srBRec.lnTipTxnAmount;
                                        srAccumRec->llX3TotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX3TotalAmount -= (pobTran->srBRec.lnTipTxnAmount + pobTran->srBRec.lnTxnAmount);
				}
				else if (inCDTXIndex == 4)
				{
                                        /* 取消整筆交易，計算筆數 */
                                        srAccumRec->lnX4TotalTipsCount --;
                                        srAccumRec->lnX4TotalSaleCount --;
                                        srAccumRec->lnX4TotalCount --;
                                        /* 取消整筆交易，計算金額 */
                                        srAccumRec->llX4TotalTipsAmount -= pobTran->srBRec.lnTipTxnAmount;
                                        srAccumRec->llX4TotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX4TotalAmount -= (pobTran->srBRec.lnTipTxnAmount + pobTran->srBRec.lnTxnAmount);
				}
				else if (inCDTXIndex == 5)
				{
                                        /* 取消整筆交易，計算筆數 */
                                        srAccumRec->lnX5TotalTipsCount --;
                                        srAccumRec->lnX5TotalSaleCount --;
                                        srAccumRec->lnX5TotalCount --;
                                        /* 取消整筆交易，計算金額 */
                                        srAccumRec->llX5TotalTipsAmount -= pobTran->srBRec.lnTipTxnAmount;
                                        srAccumRec->llX5TotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX5TotalAmount -= (pobTran->srBRec.lnTipTxnAmount + pobTran->srBRec.lnTxnAmount);
				}						
				else if (inCDTXIndex == 6)
				{
                                        /* 取消整筆交易，計算筆數 */
                                        srAccumRec->lnX6TotalTipsCount --;
                                        srAccumRec->lnX6TotalSaleCount --;
                                        srAccumRec->lnX6TotalCount --;
                                        /* 取消整筆交易，計算金額 */
                                        srAccumRec->llX6TotalTipsAmount -= pobTran->srBRec.lnTipTxnAmount;
                                        srAccumRec->llX6TotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX6TotalAmount -= (pobTran->srBRec.lnTipTxnAmount + pobTran->srBRec.lnTxnAmount);
				}
				else if (inCDTXIndex == 7)
				{
                                        /* 取消整筆交易，計算筆數 */
                                        srAccumRec->lnX7TotalTipsCount --;
                                        srAccumRec->lnX7TotalSaleCount --;
                                        srAccumRec->lnX7TotalCount --;
                                        /* 取消整筆交易，計算金額 */
                                        srAccumRec->llX7TotalTipsAmount -= pobTran->srBRec.lnTipTxnAmount;
                                        srAccumRec->llX7TotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX7TotalAmount -= (pobTran->srBRec.lnTipTxnAmount + pobTran->srBRec.lnTxnAmount);
				}
				else if (inCDTXIndex == 8)
				{
                                        /* 取消整筆交易，計算筆數 */
                                        srAccumRec->lnX8TotalTipsCount --;
                                        srAccumRec->lnX8TotalSaleCount --;
                                        srAccumRec->lnX8TotalCount --;
                                        /* 取消整筆交易，計算金額 */
                                        srAccumRec->llX8TotalTipsAmount -= pobTran->srBRec.lnTipTxnAmount;
                                        srAccumRec->llX8TotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX8TotalAmount -= (pobTran->srBRec.lnTipTxnAmount + pobTran->srBRec.lnTxnAmount);
				}
				else if (inCDTXIndex == 9)
				{
                                        /* 取消整筆交易，計算筆數 */
                                        srAccumRec->lnX9TotalTipsCount --;
                                        srAccumRec->lnX9TotalSaleCount --;
                                        srAccumRec->lnX9TotalCount --;
                                        /* 取消整筆交易，計算金額 */
                                        srAccumRec->llX9TotalTipsAmount -= pobTran->srBRec.lnTipTxnAmount;
                                        srAccumRec->llX9TotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX9TotalAmount -= (pobTran->srBRec.lnTipTxnAmount + pobTran->srBRec.lnTxnAmount);
				}
				else
				{
					memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
					sprintf(szErrorMsg, "inACCUM_UpdateTotalAmountByUnknownCard ERROR!!");
					/* 記Log */
					inUtility_StoreTraceLog_OneStep(szErrorMsg);
					memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
					sprintf(szErrorMsg, "Trans code = %d Card Label = %s", pobTran->srBRec.inCode, pobTran->srBRec.szCardLabel);
					inUtility_StoreTraceLog_OneStep(szErrorMsg);
				}
                                break;
                        case _ADJUST_:
				if (inCDTXIndex == 0)
				{
                                        /* 計算金額 */
                                        srAccumRec->llX0TotalSaleAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX0TotalAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX0TotalSaleAmount += (0 - pobTran->srBRec.lnAdjustTxnAmount);
                                        srAccumRec->llX0TotalAmount += (0 - pobTran->srBRec.lnAdjustTxnAmount);
				}
				else if (inCDTXIndex == 1)
				{
                                        /* 計算金額 */
                                        srAccumRec->llX1TotalSaleAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX1TotalAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX1TotalSaleAmount += (0 - pobTran->srBRec.lnAdjustTxnAmount);
                                        srAccumRec->llX1TotalAmount += (0 - pobTran->srBRec.lnAdjustTxnAmount);
				}
				else if (inCDTXIndex == 2)
				{
                                        /* 計算金額 */
                                        srAccumRec->llX2TotalSaleAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX2TotalAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX2TotalSaleAmount += (0 - pobTran->srBRec.lnAdjustTxnAmount);
                                        srAccumRec->llX2TotalAmount += (0 - pobTran->srBRec.lnAdjustTxnAmount);
				}
				else if (inCDTXIndex == 3)
				{
                                        /* 計算金額 */
                                        srAccumRec->llX3TotalSaleAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX3TotalAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX3TotalSaleAmount += (0 - pobTran->srBRec.lnAdjustTxnAmount);
                                        srAccumRec->llX3TotalAmount += (0 - pobTran->srBRec.lnAdjustTxnAmount);
				}
				else if (inCDTXIndex == 4)
				{
                                        /* 計算金額 */
                                        srAccumRec->llX4TotalSaleAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX4TotalAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX4TotalSaleAmount += (0 - pobTran->srBRec.lnAdjustTxnAmount);
                                        srAccumRec->llX4TotalAmount += (0 - pobTran->srBRec.lnAdjustTxnAmount);
				}
				else if (inCDTXIndex == 5)
				{
                                        /* 計算金額 */
                                        srAccumRec->llX5TotalSaleAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX5TotalAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX5TotalSaleAmount += (0 - pobTran->srBRec.lnAdjustTxnAmount);
                                        srAccumRec->llX5TotalAmount += (0 - pobTran->srBRec.lnAdjustTxnAmount);
				}						
				else if (inCDTXIndex == 6)
				{
                                        /* 計算金額 */
                                        srAccumRec->llX6TotalSaleAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX6TotalAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX6TotalSaleAmount += (0 - pobTran->srBRec.lnAdjustTxnAmount);
                                        srAccumRec->llX6TotalAmount += (0 - pobTran->srBRec.lnAdjustTxnAmount);
				}
				else if (inCDTXIndex == 7)
				{
                                        /* 計算金額 */
                                        srAccumRec->llX7TotalSaleAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX7TotalAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX7TotalSaleAmount += (0 - pobTran->srBRec.lnAdjustTxnAmount);
                                        srAccumRec->llX7TotalAmount += (0 - pobTran->srBRec.lnAdjustTxnAmount);
				}
				else if (inCDTXIndex == 8)
				{
                                        /* 計算金額 */
                                        srAccumRec->llX8TotalSaleAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX8TotalAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX8TotalSaleAmount += (0 - pobTran->srBRec.lnAdjustTxnAmount);
                                        srAccumRec->llX8TotalAmount += (0 - pobTran->srBRec.lnAdjustTxnAmount);
				}
				else if (inCDTXIndex == 9)
				{
                                        /* 計算金額 */
                                        srAccumRec->llX9TotalSaleAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX9TotalAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llX9TotalSaleAmount += (0 - pobTran->srBRec.lnAdjustTxnAmount);
                                        srAccumRec->llX9TotalAmount += (0 - pobTran->srBRec.lnAdjustTxnAmount);
				}
				else
				{
					memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
					sprintf(szErrorMsg, "inACCUM_UpdateTotalAmountByUnknownCard ERROR!!");
					/* 記Log */
					inUtility_StoreTraceLog_OneStep(szErrorMsg);
					memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
					sprintf(szErrorMsg, "Trans code = %d Card Label = %s", pobTran->srBRec.inCode, pobTran->srBRec.szCardLabel);
					inUtility_StoreTraceLog_OneStep(szErrorMsg);
				}
                                break;
                        default:
                                /* 交易類別錯誤，(沒有此交易類別) */
                                /* debug */
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
				sprintf(szErrorMsg, "inACCUM_UpdateTotalAmountByUnknownCard : ERROR!!");
				/* 鎖機記Log */
				inUtility_StoreTraceLog_OneStep(szErrorMsg);
				
				if (ginDebug == VS_TRUE)
                                        inLogPrintf(AT, szErrorMsg);

				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
				sprintf(szErrorMsg, "OrgTrans code = %d", pobTran->srBRec.inOrgCode);
				inUtility_StoreTraceLog_OneStep(szErrorMsg);
				
				if (ginDebug == VS_TRUE)
                                        inLogPrintf(AT, szErrorMsg);

                                return (VS_ERROR);
                }
        }
	
        /* inACCUM_UpdateTotalAmountByUnknownCard()_END */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inACCUM_UpdateTotalAmountByUnknownCard()_END");
        }
	
	return (VS_SUCCESS);
}

/*
Function        :inACCUM_UpdateFlow_TAKA
Date&Time       :2022/10/17 上午 10:32
Describe        :
*/
int inACCUM_UpdateFlow_TAKA(TRANSACTION_OBJECT *pobTran)
{
        unsigned long   ulHandlePtr = 0;				/* File Handle，type為pointer */
        int		inRetVal = VS_SUCCESS;				/* return value，來判斷是否回傳error */
        unsigned char   uszFileName[14 + 1] = {0}; /* 儲存交易金額檔案的檔案名稱(最大為15) */
	TAKA_ACCUM_TOTAL_REC srAccumRec; /*用來放總筆數、總金額的結構體*/

        /* inACCUM_UpdateFlow_TAKA()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inACCUM_UpdateFlow_TAKA()_START");
        }
	/* 若斷電導致批次無法正確儲存時方便解釋 */
	inUtility_StoreTraceLog_OneStep("_UPDATE_ACCUM_START_($:%ld)", pobTran->srBRec.lnTxnAmount);
	
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
	}

	if (inFunc_ComposeFileName(pobTran, (char*)uszFileName, _ACCUM_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
                vdUtility_SYSFIN_LogMessage(AT, "inACCUM_UpdateFlow_TAKA inFunc_ComposeFileName failed");
		/* Update Accum失敗 鎖機*/
		inFunc_EDCLock(AT);
			
		return (VS_ERROR);
	}

        /* 開檔 create檔(若沒有檔案則創建檔案) */
        if (inFILE_Open(&ulHandlePtr, uszFileName) == (VS_ERROR))
        {
                /* 開檔失敗時，不必關檔(開檔失敗，handle回傳NULL) */
                /* 若檔案不存在時，Create檔案 */
                if (inFILE_Check_Exist(uszFileName) == (VS_ERROR))
                {
                        inRetVal = inFILE_Create(&ulHandlePtr, uszFileName);
                }
                /* 檔案存在還是開檔失敗，回傳錯誤跳出 */
                else
                {
                        vdUtility_SYSFIN_LogMessage(AT, "inACCUM_UpdateFlow_TAKA file_exist_but_opened_failed ");
			/* Update Accum失敗 鎖機*/
			inFunc_EDCLock(AT);
			
                        return (VS_ERROR);
                }

                /* Create檔 */
                if (inRetVal != VS_SUCCESS)
                {
                        vdUtility_SYSFIN_LogMessage(AT, "inACCUM_UpdateFlow_TAKA create_file_failed");
			/* Update Accum失敗 鎖機*/
			inFunc_EDCLock(AT);
		
                        /* Create檔失敗時不關檔，因為create檔失敗handle回傳NULL */
                        return (VS_ERROR);
                }
                /* create檔成功就繼續(因為create檔已經把handle指向檔案，所以不用在開檔) */
        }

        /* 開檔成功或create檔成功後 */
        /*先清空srAccumRec 為讀檔案作準備  */
        memset(&srAccumRec, 0x00, sizeof(srAccumRec));

        /* 把指針指到開頭*/
        inRetVal = inFILE_Seek(ulHandlePtr, 0, _SEEK_BEGIN_);

        /* inFile_seek */
        if (inRetVal != VS_SUCCESS)
        {
                vdUtility_SYSFIN_LogMessage(AT, "inACCUM_UpdateFlow_TAKA inFILE_Seek failed");
		/* Update Accum失敗 鎖機*/
		inFunc_EDCLock(AT);
		
                /* inFILE_Seek失敗時 */
                /* Seek檔失敗，所以關檔 */
                inFILE_Close(&ulHandlePtr);

                /* Seek檔失敗，所以回傳VS_ERROR。(關檔不論成功與否都要回傳(VS_ERROR)) */
                return (VS_ERROR);
        }

        /* 2015/11/20 修改 */
        /* 當文件內容為空時，read會回傳error;所以當read失敗， 而且檔案長度不等於0的時候，才是真的出錯 */
        if (inFILE_Read(&ulHandlePtr, (unsigned char *)&srAccumRec, _TAKA_ACCUM_REC_SIZE_) == VS_ERROR && lnFILE_GetSize(&ulHandlePtr, (unsigned char *) &srAccumRec) != 0)
        {
                vdUtility_SYSFIN_LogMessage(AT, "inACCUM_UpdateFlow_TAKA inFILE_Read failed");
		/* Update Accum失敗 鎖機*/
		inFunc_EDCLock(AT);
		
                /* 讀檔失敗 */
                /* Read檔失敗，所以關檔 */
                inFILE_Close(&ulHandlePtr);

                /* Read檔失敗，所以回傳VS_ERROR。(關檔不論成功與否都要回傳(VS_ERROR)) */
                return (VS_ERROR);
        }
        else
        {
		/* 計算全部交易金額、筆數 */
		if (inACCUM_UpdateTotalAmount_TAKA(pobTran, &srAccumRec) == VS_ERROR)
		{
                        vdUtility_SYSFIN_LogMessage(AT, "inACCUM_UpdateFlow_TAKA inACCUM_UpdateTotalAmount_TAKA failed");
			/* Update Accum失敗 鎖機*/
			inFunc_EDCLock(AT);

			/* 計算全部交易金額、筆數失敗 */
			/* 計算全部交易金額、筆數失敗，所以關檔 */
			inFILE_Close(&ulHandlePtr);

			/* 計算全部交易金額、筆數失敗，所以回傳VS_ERROR。(關檔不論成功與否都要回傳(VS_ERROR)) */
			return (VS_ERROR);
		}

		/* 計算卡別全部交易金額、筆數 */
		if (inACCUM_UpdateTotalAmountByCard_TAKA(pobTran, &srAccumRec) == VS_ERROR)
		{
                        vdUtility_SYSFIN_LogMessage(AT, "inACCUM_UpdateFlow_TAKA inACCUM_UpdateTotalAmountByCard_TAKA failed");
			/* Update Accum失敗 鎖機*/
			inFunc_EDCLock(AT);

			/* 計算卡別全部交易金額、筆數失敗 */
			/* 計算卡別全部交易金額、筆數失敗，所以關檔 */
			inFILE_Close(&ulHandlePtr);

			/* 計算卡別全部交易金額、筆數失敗，所以回傳VS_ERROR。(關檔不論成功與否都要回傳(VS_ERROR)) */
			return (VS_ERROR);
		}
		
                /* 先將檔案關閉 */
                if (inFILE_Close(&ulHandlePtr) == (VS_ERROR))
                {
                        vdUtility_SYSFIN_LogMessage(AT, "inACCUM_UpdateFlow_TAKA inFILE_Close failed");
			/* Update Accum失敗 鎖機*/
			inFunc_EDCLock(AT);
			
                        /* 關檔失敗 */
                        /* 回傳VS_ERROR */
                        return (VS_ERROR);
                }

                /* 存檔案 */
                if (inACCUM_StoreRecord_General(&srAccumRec, uszFileName, sizeof(srAccumRec)) == VS_ERROR)
                {
                        vdUtility_SYSFIN_LogMessage(AT, "inACCUM_UpdateFlow_TAKA inACCUM_StoreRecord_General failed");
			/* Update Accum失敗 鎖機*/
			inFunc_EDCLock(AT);
		
                        /* 存檔失敗 */
                        /* 因為inACCUM_StoreRecord失敗，所以回傳VS_ERROR */
                        return (VS_ERROR);
                }

        }/* read、計算總額 和 存檔 成功 */

	/* 若斷電導致批次無法正確儲存時方便解釋 */
	inUtility_StoreTraceLog_OneStep("_UPDATE_ACCUM_END_($:%ld)", pobTran->srBRec.lnTxnAmount);
        /* inACCUM_UpdateFlow_TAKA()_END */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inACCUM_UpdateFlow_TAKA()_END");
        }
	
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inACCUM_UpdateTotalAmount_TAKA
Date&Time       :2022/10/17 上午 10:45
Describe        :每當有交易時，到此fuction更新筆數和總額，回傳(VS_SUCCESS) or (VS_ERROR)
*/
int inACCUM_UpdateTotalAmount_TAKA(TRANSACTION_OBJECT *pobTran, TAKA_ACCUM_TOTAL_REC *srAccumRec)
{
        char            szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */

        /* inACCUM_UpdateTotalAmount_TAKA()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inACCUM_UpdateTotalAmount_TAKA()_START");
        }

        /* 不是取消的交易 */
        if (pobTran->srBRec.uszVOIDBit == VS_FALSE)
        {
                switch (pobTran->srBRec.inCode)
                {
                        case _SALE_:
                                /* 計算筆數 */
                                srAccumRec->lnTotalSaleCount++;
                                srAccumRec->lnTotalCount++;
                                /* 計算金額 */
                                srAccumRec->llTotalSaleAmount += pobTran->srBRec.lnTxnAmount;
                                srAccumRec->llTotalAmount += pobTran->srBRec.lnTxnAmount;
                                break;
                        case _REFUND_:
                                /* 計算筆數 */
                                srAccumRec->lnTotalRefundCount++;
                                srAccumRec->lnTotalCount++;
                                /* 計算金額 */
                                srAccumRec->llTotalRefundAmount += (0 - pobTran->srBRec.lnTxnAmount);
                                srAccumRec->llTotalAmount += (0 - pobTran->srBRec.lnTxnAmount);
                                break;
                        default:
                                /* 交易類別錯誤，(沒有此交易類別) */
                                /* debug */
                                if (ginDebug == VS_TRUE)
                                {
                                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                        sprintf(szErrorMsg, "inACCUM_UpdateTotalAmount_TAKA ERROR!!");
                                        inLogPrintf(AT, szErrorMsg);

                                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                        sprintf(szErrorMsg, "Trans code = %d", pobTran->srBRec.inCode);
                                        inLogPrintf(AT, szErrorMsg);
                                }

                                return (VS_ERROR);
                }
        }
        /* 取消的時候 */
        else
        {
                switch (pobTran->srBRec.inOrgCode)
                {
                        case _SALE_:
                                /* 計算筆數 */
                                srAccumRec->lnTotalSaleCount--;
                                srAccumRec->lnTotalCount--;
                                /* 計算金額 */
                                srAccumRec->llTotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                srAccumRec->llTotalAmount -= pobTran->srBRec.lnTxnAmount;
                                break;
                        case _REFUND_:
                                /* 計算筆數 */
                                srAccumRec->lnTotalRefundCount--;
                                srAccumRec->lnTotalCount--;
                                /* 計算金額 */
                                srAccumRec->llTotalRefundAmount += pobTran->srBRec.lnTxnAmount;
                                srAccumRec->llTotalAmount += pobTran->srBRec.lnTxnAmount;
                                break;
                      
                        default:
                                /* 交易類別錯誤，(沒有此交易類別) */
                                /* debug */
                                if (ginDebug == VS_TRUE)
                                {
                                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                        sprintf(szErrorMsg, "inACCUM_UpdateTotalAmount_TAKA ERROR!!");
                                        inLogPrintf(AT, szErrorMsg);

                                        memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                        sprintf(szErrorMsg, "OrgTrans code = %d", pobTran->srBRec.inOrgCode);
                                        inLogPrintf(AT, szErrorMsg);
                                }

                                return (VS_ERROR);
                }
        }

        /* inACCUM_UpdateTotalAmount_TAKA()_END */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inACCUM_UpdateTotalAmount_TAKA()_END");
        }

        return (VS_SUCCESS);
}

/*
Function        :inACCUM_UpdateTotalAmountByCard_TAKA
Date&Time       :2022/10/17 上午 11:33
Describe        :每當有交易時，到此fuction更新"卡別"的筆數和總額，回傳(VS_SUCCESS) or (VS_ERROR)
*/
int inACCUM_UpdateTotalAmountByCard_TAKA(TRANSACTION_OBJECT *pobTran, TAKA_ACCUM_TOTAL_REC *srAccumRec)
{
        char	szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1] = {0};   /* debug message */

        /* inACCUM_UpdateTotalAmountByCard()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inACCUM_UpdateTotalAmountByCard()_START");
        }

        /* 不是取消的交易 */
        if (pobTran->srBRec.uszVOIDBit == VS_FALSE)
        {
                switch (pobTran->srBRec.inCode)
                {
                        case _SALE_:
                                if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_TAKA_EMPLOYEE_, 8))
                                {
                                        /* 計算筆數 */
                                        srAccumRec->lnTAKA_EMPLOYEE_TotalSaleCount ++;
                                        srAccumRec->lnTAKA_EMPLOYEE_TotalCount ++;
                                        /* 計算金額 */
                                        srAccumRec->llTAKA_EMPLOYEE_TotalSaleAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llTAKA_EMPLOYEE_TotalAmount += pobTran->srBRec.lnTxnAmount;
                                }
                                else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_TAKA_T_DAYEH_, 7))
                                {
                                        /* 計算筆數 */
                                        srAccumRec->lnTAKA_T_DAYEH_TotalSaleCount ++;
                                        srAccumRec->lnTAKA_T_DAYEH_TotalCount ++;
                                        /* 計算金額 */
                                        srAccumRec->llTAKA_T_DAYEH_TotalSaleAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llTAKA_T_DAYEH_TotalAmount += pobTran->srBRec.lnTxnAmount;
                                }
                                else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_TAKA_T_CARD_, 6))
                                {
                                        /* 計算筆數 */
                                        srAccumRec->lnTAKA_T_CARD_TotalSaleCount ++;
                                        srAccumRec->lnTAKA_T_CARD_TotalCount ++;
                                        /* 計算金額 */
                                        srAccumRec->llTAKA_T_CARD_TotalSaleAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llTAKA_T_CARD_TotalAmount += pobTran->srBRec.lnTxnAmount;
                                }
                                break;
                        case _REFUND_:
                                if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_TAKA_EMPLOYEE_, 8))
                                {
                                        /* 計算筆數 */
                                        srAccumRec->lnTAKA_EMPLOYEE_TotalRefundCount ++;
                                        srAccumRec->lnTAKA_EMPLOYEE_TotalCount ++;
                                        /* 計算金額 */
                                        srAccumRec->llTAKA_EMPLOYEE_TotalRefundAmount += (0 - pobTran->srBRec.lnTxnAmount);
                                        srAccumRec->llTAKA_EMPLOYEE_TotalAmount += (0 - pobTran->srBRec.lnTxnAmount);
                                }
                                else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_TAKA_T_DAYEH_, 7))
                                {
                                        /* 計算筆數 */
                                        srAccumRec->lnTAKA_T_DAYEH_TotalRefundCount ++;
                                        srAccumRec->lnTAKA_T_DAYEH_TotalCount ++;
                                        /* 計算金額 */
                                        srAccumRec->llTAKA_T_DAYEH_TotalRefundAmount += (0 - pobTran->srBRec.lnTxnAmount);
                                        srAccumRec->llTAKA_T_DAYEH_TotalAmount += (0 - pobTran->srBRec.lnTxnAmount);
                                }
                                else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_TAKA_T_CARD_, 6))
                                {
                                        /* 計算筆數 */
                                        srAccumRec->lnTAKA_T_CARD_TotalRefundCount ++;
                                        srAccumRec->lnTAKA_T_CARD_TotalCount ++;
                                        /* 計算金額 */
                                        srAccumRec->llTAKA_T_CARD_TotalRefundAmount += (0 - pobTran->srBRec.lnTxnAmount);
                                        srAccumRec->llTAKA_T_CARD_TotalAmount += (0 - pobTran->srBRec.lnTxnAmount);
                                }
                                break;
                        default :
                                /* 交易類別錯誤，(沒有此交易類別) */
                                /* debug */
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
				sprintf(szErrorMsg, "inACCUM_UpdateTotalAmountByCard_TAKA : ERROR!! Trans code = %d", pobTran->srBRec.inCode);
				/* 鎖機記Log */
				inUtility_StoreTraceLog_OneStep(szErrorMsg);
				
				if (ginDebug == VS_TRUE)
					inLogPrintf(AT, szErrorMsg);

                                return (VS_ERROR);
                }
        }
        /* 取消的時候 */
        else
        {
                switch (pobTran->srBRec.inOrgCode)
                {
                        case _SALE_:
                                if (!memcmp(&pobTran->srBRec.szCardLabel, _CARD_TYPE_TAKA_EMPLOYEE_, 8))
                                {
                                        /* 計算筆數 */
                                        srAccumRec->lnTAKA_EMPLOYEE_TotalSaleCount --;
                                        srAccumRec->lnTAKA_EMPLOYEE_TotalCount --;
                                        /* 計算金額 */
                                        srAccumRec->llTAKA_EMPLOYEE_TotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llTAKA_EMPLOYEE_TotalAmount -= pobTran->srBRec.lnTxnAmount;
                                }
                                else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_TAKA_T_DAYEH_, 7))
                                {
                                        /* 計算筆數 */
                                        srAccumRec->lnTAKA_T_DAYEH_TotalSaleCount --;
                                        srAccumRec->lnTAKA_T_DAYEH_TotalCount --;
                                        /* 計算金額 */
                                        srAccumRec->llTAKA_T_DAYEH_TotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llTAKA_T_DAYEH_TotalAmount -= pobTran->srBRec.lnTxnAmount;
                                }
                                else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_TAKA_T_CARD_, 6))
                                {
                                        /* 計算筆數 */
                                        srAccumRec->lnTAKA_T_CARD_TotalSaleCount --;
                                        srAccumRec->lnTAKA_T_CARD_TotalCount --;
                                        /* 計算金額 */
                                        srAccumRec->llTAKA_T_CARD_TotalSaleAmount -= pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llTAKA_T_CARD_TotalAmount -= pobTran->srBRec.lnTxnAmount;
                                }
                                break;
                        case _REFUND_:
			/* SmartPay退貨不能取消 */
                                if (!memcmp(&pobTran->srBRec.szCardLabel, _CARD_TYPE_TAKA_EMPLOYEE_, 4))
                                {
                                        /* 計算筆數 */
                                        srAccumRec->lnTAKA_EMPLOYEE_TotalRefundCount --;
                                        srAccumRec->lnTAKA_EMPLOYEE_TotalCount --;
                                        /* 計算金額 */
                                        srAccumRec->llTAKA_EMPLOYEE_TotalRefundAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llTAKA_EMPLOYEE_TotalAmount += pobTran->srBRec.lnTxnAmount;
                                }
                                else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_TAKA_T_DAYEH_, 10))
                                {
                                        /* 計算筆數 */
                                        srAccumRec->lnTAKA_T_DAYEH_TotalRefundCount --;
                                        srAccumRec->lnTAKA_T_DAYEH_TotalCount --;
                                        /* 計算金額 */
                                        srAccumRec->llTAKA_T_DAYEH_TotalRefundAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llTAKA_T_DAYEH_TotalAmount += pobTran->srBRec.lnTxnAmount;
                                }
                                else if (!memcmp(&pobTran->srBRec.szCardLabel[0], _CARD_TYPE_TAKA_T_CARD_, 3))
                                {
                                        /* 計算筆數 */
                                        srAccumRec->lnTAKA_T_CARD_TotalRefundCount --;
                                        srAccumRec->lnTAKA_T_CARD_TotalCount --;
                                        /* 計算金額 */
                                        srAccumRec->llTAKA_T_CARD_TotalRefundAmount += pobTran->srBRec.lnTxnAmount;
                                        srAccumRec->llTAKA_T_CARD_TotalAmount += pobTran->srBRec.lnTxnAmount;
                                }
                                break;
                        default:
                                /* 交易類別錯誤，(沒有此交易類別) */
                                /* debug */
				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
				sprintf(szErrorMsg, "inACCUM_UpdateTotalAmountByCard_TAKA : ERROR!!");
				/* 鎖機記Log */
				inUtility_StoreTraceLog_OneStep(szErrorMsg);
				
				if (ginDebug == VS_TRUE)
                                        inLogPrintf(AT, szErrorMsg);

				memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
				sprintf(szErrorMsg, "OrgTrans code = %d", pobTran->srBRec.inOrgCode);
				inUtility_StoreTraceLog_OneStep(szErrorMsg);
				
				if (ginDebug == VS_TRUE)
                                        inLogPrintf(AT, szErrorMsg);

                                return (VS_ERROR);
                }
        }

        /* inACCUM_UpdateTotalAmountByCard()_END */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inACCUM_UpdateTotalAmountByCard_TAKA()_END");
        }

        return (VS_SUCCESS);
}

/*
Function        :inACCUM_StoreRecord_General
Date&Time       :2022/10/17 上午 11:28
Describe        :做成通用型，不只限於特殊客製化，標準板理論上可取代，不過先保留
*/
int inACCUM_StoreRecord_General(void *srAccumRec, unsigned char *uszFileName, int inAccumRecSize)
{
        unsigned long   ulHandlePtr;    /* File Handle，type為pointer */
        int inRetVal; /* return value，來判斷是否回傳error */

        /* inACCUM_StoreRecord_General()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inACCUM_StoreRecord_General()_START");
        }

        /*開檔*/
        inRetVal = inFILE_Open(&ulHandlePtr, uszFileName);
        /*開檔成功*/
        if (inRetVal != VS_SUCCESS)
        {
                /* 開檔失敗時，不必關檔(開檔失敗，handle回傳NULL) */
                /* 開檔失敗，所以回傳error */
                return(VS_ERROR);
        }

        /* 把指針指到開頭*/
        inRetVal = inFILE_Seek(ulHandlePtr, 0, _SEEK_BEGIN_);

        /* seek不成功時 */
        if (inRetVal != VS_SUCCESS)
        {
                /* inFILE_Seek失敗時 */
                /* 關檔並回傳VS_ERROR */
                inFILE_Close(&ulHandlePtr);
                /* seek失敗，所以回傳error。(關檔不論成功與否都要回傳(VS_ERROR)) */
                return (VS_ERROR);
        }

        /* 寫檔 */
        if (inFILE_Write(&ulHandlePtr, (unsigned char*)srAccumRec, inAccumRecSize) != VS_SUCCESS)
        {
                /*寫檔失敗時*/
                /* 關檔  */
                inFILE_Close(&ulHandlePtr);
                /* 寫檔失敗，所以回傳error。(關檔不論成功與否都要回傳(VS_ERROR)) */
                return (VS_ERROR);
        }

        /* 關檔  */
        if (inFILE_Close(&ulHandlePtr) != VS_SUCCESS)
        {
                /*關檔失敗時*/
                /* 關檔失敗，所以回傳error */
                return (VS_ERROR);
        }

        /* inACCUM_StoreRecord_General()_END */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inACCUM_StoreRecord_General()_END");
        }

        return (VS_SUCCESS);
}

/*
Function        :inACCUM_GetRecord_General
Date&Time       :2022/10/17 下午 2:06
Describe        :將.amt中的資料讀取到srAccumRec來做使用，回傳(VS_SUCCESS) or (VS_ERROR)
*/
int inACCUM_GetRecord_General(TRANSACTION_OBJECT *pobTran, void *srAccumRec, int inAccumRecSize)
{
        unsigned long   ulHandlePtr;				/* File Handle */
        int             inRetVal;                               /* return value，來判斷是否回傳error */
        char            szErrorMsg[_DEBUG_MESSAGE_SIZE_ + 1];   /* debug message */
        unsigned char   uszFileName[14 + 1];                    /* 儲存交易金額檔案的檔案名稱(最大為15) */
        
        /* inACCUM_GetRecord_General()_START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inACCUM_GetRecord_General()_START");
        }

	if (inFunc_ComposeFileName(pobTran, (char*)uszFileName, _ACCUM_FILE_EXTENSION_, 6) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

        /* 純開檔不建檔  */
        if (inFILE_OpenReadOnly(&ulHandlePtr, uszFileName) == (VS_ERROR))
        {
                /* 開檔失敗時 */
                /* 開檔錯誤，確認是否有檔案，若有檔案仍錯誤，則可能是handle的問題 */
                if (inFILE_Check_Exist(uszFileName) != (VS_ERROR))
                {
                        /* 開啟失敗，但檔案存在回傳error */
                        return (VS_ERROR);
                }
                /* 如果沒有檔案時，則為沒有交易紀錄，回傳NO_RECORD並印空簽單 */
                else
                {
                        return (VS_NO_RECORD);
                }
		
        }
        /* 開檔成功 */
        else
        {
                /*先清空srAccumRec 為讀檔案作準備  */
                memset(srAccumRec, 0x00, inAccumRecSize);

                /* 把指針指到開頭*/
                inRetVal = inFILE_Seek(ulHandlePtr, 0, _SEEK_BEGIN_);

                /* inFILE_Seek，成功時 */
                if (inRetVal != VS_SUCCESS)
                {
                        /* inFILE_Seek失敗時 */
                        /* 關檔並回傳VS_ERROR */
                        inFILE_Close(&ulHandlePtr);

                        /* Seek檔失敗，所以回傳VS_ERROR。(關檔不論成功與否都要回傳(VS_ERROR)) */
                        return (VS_ERROR);
                }

                /*確認檔案大小*/
                if (lnFILE_GetSize(&ulHandlePtr, uszFileName) == 0)
                {
                        /* 長度為0，不必讀 */
                        /* debug */
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "lnFILE_GetSize ＝＝ 0 ：");
                                memset(szErrorMsg, 0x00, sizeof(szErrorMsg));
                                sprintf(szErrorMsg, "(FName = %s), (Ptr = %d)", uszFileName, (int)ulHandlePtr);
                                inLogPrintf(AT, szErrorMsg);
                        }
                        /* 關檔並回傳VS_NO_RECORD */
                        inFILE_Close(&ulHandlePtr);
                        return (VS_NO_RECORD);
                }
                /* Get不到Size的時候 */
                else if (lnFILE_GetSize(&ulHandlePtr, uszFileName) == VS_ERROR)
                {
                        /* 關檔並回傳VS_ERROR */
                        inFILE_Close(&ulHandlePtr);
                        return (VS_ERROR);
                }
                /* 檔案大小大於0(裡面有資料時) */
                else
                {
                        /* 讀檔 */
                        if (inFILE_Read(&ulHandlePtr, (unsigned char *) srAccumRec, inAccumRecSize) == VS_ERROR)
                        {
                                /* 讀檔失敗就關檔 */
                                inFILE_Close(&ulHandlePtr);
                                /* 讀檔失敗，所以回傳（VS_ERROR）。(關檔不論成功與否都要回傳(VS_ERROR)) */
                                return (VS_ERROR);
                        }
                }

                /* 將檔案關閉 */
                if (inFILE_Close(&ulHandlePtr) == VS_ERROR)
                {
                        /*關檔失敗*/
                        /* 關檔失敗，所以回傳VS_ERROR */
                        return (VS_ERROR);
                }
        }
        /* inACCUM_GetRecord_General()_END */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inACCUM_GetRecord_General()_END");
        }

        return (VS_SUCCESS);
}