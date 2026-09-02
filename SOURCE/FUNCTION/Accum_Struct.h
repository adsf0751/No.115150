/* 
 * File:   Accum_Struct.h
 * Author: RussellBai
 *
 * Created on 2023年8月9日, 上午 11:49
 */

#ifndef ACCUM_STRUCT_H
#define	ACCUM_STRUCT_H

#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* ACCUM_STRUCT_H */

/* VEGA3000 為32位元系統
 * 在32位元系統下，int、long都用4個byte longlong為8個byte
 * 在64位元系統下，int 使用4個byte long、longlong為8個byte
 * 
 * 至於double為浮點數，使用8個byte，有效位10位，但是超過10位精度可能會被吃掉，要特別注意。
 * 
 * 除了*与long随操作系统子长变化而变化外，其他的都固定不变(32位和64位相比)
 */

typedef struct Account_Total_File
{
        /* 全部交易筆數 */
        long		lnTotalSaleCount;
        long		lnTotalRefundCount;
        long		lnTotalTipsCount;
        long		lnTotalAdjustCount;
        long		lnTotalCount;
        /* 全部交易金額 */
        long long	llTotalSaleAmount;
        long long	llTotalRefundAmount;
        long long	llTotalTipsAmount;
        long long	llTotalAmount;

        /* VISA 全部交易筆數*/
        long		lnVisaTotalSaleCount;
        long		lnVisaTotalRefundCount;
        long		lnVisaTotalTipsCount;
        long		lnVisaTotalCount;
        /* VISA 全部交易金額 */
        long long	llVisaTotalSaleAmount;
        long long	llVisaTotalRefundAmount;
        long long	llVisaTotalTipsAmount;
        long long	llVisaTotalAmount;
        /* MASTER 全部交易筆數*/
        long		lnMasterTotalSaleCount;
        long		lnMasterTotalRefundCount;
        long		lnMasterTotalTipsCount;
        long		lnMasterTotalCount;
        /* MASTER 全部交易金額 */
        long long	llMasterTotalSaleAmount;
        long long	llMasterTotalRefundAmount;
        long long	llMasterTotalTipsAmount;
        long long	llMasterTotalAmount;
        /* JCB 全部交易筆數*/
        long		lnJcbTotalSaleCount;
        long		lnJcbTotalRefundCount;
        long		lnJcbTotalTipsCount;
        long		lnJcbTotalCount;
        /* JCB 全部交易金額 */
        long long	llJcbTotalSaleAmount;
        long long	llJcbTotalRefundAmount;
        long long	llJcbTotalTipsAmount;
        long long	llJcbTotalAmount;
        /* U_CARD 全部交易筆數*/
        long		lnUCardTotalSaleCount;
        long		lnUCardTotalRefundCount;
        long		lnUCardTotalTipsCount;
        long		lnUCardTotalCount;
        /* U_CARD 全部交易金額 */
        long long	llUCardTotalSaleAmount;
        long long	llUCardTotalRefundAmount;
        long long	llUCardTotalTipsAmount;
        long long	llUCardTotalAmount;
        /* AMEX 全部交易筆數*/
        long		lnAmexTotalSaleCount;
        long		lnAmexTotalRefundCount;
        long		lnAmexTotalTipsCount;
        long		lnAmexTotalCount;
        /* AMEX 全部交易金額 */
        long long	llAmexTotalSaleAmount;
        long long	llAmexTotalRefundAmount;
        long long	llAmexTotalTipsAmount;
        long long	llAmexTotalAmount;
        /* DINERS 全部交易筆數*/
        long		lnDinersTotalSaleCount;
        long		lnDinersTotalRefundCount;
        long		lnDinersTotalTipsCount;
        long		lnDinersTotalCount;
        /* DINERS 全部交易金額 */
        long long	llDinersTotalSaleAmount;
        long long	llDinersTotalRefundAmount;
        long long	llDinersTotalTipsAmount;
        long long	llDinersTotalAmount;
        /* CUP 全部交易筆數*/
        long		lnCupTotalSaleCount;
        long		lnCupTotalRefundCount;
        long		lnCupTotalTipsCount;
        long		lnCupTotalCount;
        /* CUP 全部交易金額 */
        long long	llCupTotalSaleAmount;
        long long	llCupTotalRefundAmount;
        long long	llCupTotalTipsAmount;
        long long	llCupTotalAmount;
        /* Smart Pay 全部交易筆數 */
        long		lnFiscTotalSaleCount;
        long		lnFiscTotalRefundCount;
        long		lnFiscTotalCount;
        /* Smart Pay 全部交易金額 */
        long long	llFiscTotalSaleAmount;
        long long	llFiscTotalRefundAmount;
        long long	llFiscTotalAmount;

	
	/* 【需求單 - 108097】Card BIN由ATS查核 add by LingHsiung 2020/2/3 下午 5:19 */
	/* 新增未知卡別小計 add by LingHsiung 2020/7/7 下午 2:26 */
	/* unknown card X0 全部交易筆數 */
        long		lnX0TotalSaleCount;
        long		lnX0TotalRefundCount;
	long		lnX0TotalTipsCount;
        long		lnX0TotalCount;
	/* unknown card X0 全部交易金額 */
        long long	llX0TotalSaleAmount;
        long long	llX0TotalRefundAmount;
	long long	llX0TotalTipsAmount;
        long long	llX0TotalAmount;
	
	/* unknown card X1 全部交易筆數 */
        long		lnX1TotalSaleCount;
        long		lnX1TotalRefundCount;
	long		lnX1TotalTipsCount;
        long		lnX1TotalCount;
	/* unknown card X1 全部交易金額 */
        long long	llX1TotalSaleAmount;
        long long	llX1TotalRefundAmount;
	long long	llX1TotalTipsAmount;
        long long	llX1TotalAmount;
	
	/* unknown card X2 全部交易筆數 */
        long		lnX2TotalSaleCount;
        long		lnX2TotalRefundCount;
	long		lnX2TotalTipsCount;
        long		lnX2TotalCount;
	/* unknown card X2 全部交易金額 */
        long long	llX2TotalSaleAmount;
        long long	llX2TotalRefundAmount;
	long long	llX2TotalTipsAmount;
        long long	llX2TotalAmount;
	
	/* unknown card X3 全部交易筆數 */
        long		lnX3TotalSaleCount;
        long		lnX3TotalRefundCount;
	long		lnX3TotalTipsCount;
        long		lnX3TotalCount;
	/* unknown card X3 全部交易金額 */
        long long	llX3TotalSaleAmount;
        long long	llX3TotalRefundAmount;
	long long	llX3TotalTipsAmount;
        long long	llX3TotalAmount;
	
	/* unknown card X4 全部交易筆數 */
        long		lnX4TotalSaleCount;
        long		lnX4TotalRefundCount;
	long		lnX4TotalTipsCount;
        long		lnX4TotalCount;
	/* unknown card X4 全部交易金額 */
        long long	llX4TotalSaleAmount;
        long long	llX4TotalRefundAmount;
	long long	llX4TotalTipsAmount;
        long long	llX4TotalAmount;
	
	/* unknown card X5 全部交易筆數 */
        long		lnX5TotalSaleCount;
        long		lnX5TotalRefundCount;
	long		lnX5TotalTipsCount;
        long		lnX5TotalCount;
	/* unknown card X5 全部交易金額 */
        long long	llX5TotalSaleAmount;
        long long	llX5TotalRefundAmount;
	long long	llX5TotalTipsAmount;
        long long	llX5TotalAmount;
	
	/* unknown card X6 全部交易筆數 */
        long		lnX6TotalSaleCount;
        long		lnX6TotalRefundCount;
	long		lnX6TotalTipsCount;
        long		lnX6TotalCount;
	/* unknown card X6 全部交易金額 */
        long long	llX6TotalSaleAmount;
        long long	llX6TotalRefundAmount;
	long long	llX6TotalTipsAmount;
        long long	llX6TotalAmount;
	
	/* unknown card X7 全部交易筆數 */
        long		lnX7TotalSaleCount;
        long		lnX7TotalRefundCount;
	long		lnX7TotalTipsCount;
        long		lnX7TotalCount;
	/* unknown card X7 全部交易金額 */
        long long	llX7TotalSaleAmount;
        long long	llX7TotalRefundAmount;
	long long	llX7TotalTipsAmount;
        long long	llX7TotalAmount;
	
	/* unknown card X8 全部交易筆數 */
        long		lnX8TotalSaleCount;
        long		lnX8TotalRefundCount;
	long		lnX8TotalTipsCount;
        long		lnX8TotalCount;
	/* unknown card X8 全部交易金額 */
        long long	llX8TotalSaleAmount;
        long long	llX8TotalRefundAmount;
	long long	llX8TotalTipsAmount;
        long long	llX8TotalAmount;
	
	/* unknown card X9 全部交易筆數 */
        long		lnX9TotalSaleCount;
        long		lnX9TotalRefundCount;
	long		lnX9TotalTipsCount;
        long		lnX9TotalCount;
	/* unknown card X9 全部交易金額 */
        long long	llX9TotalSaleAmount;
        long long	llX9TotalRefundAmount;
	long long	llX9TotalTipsAmount;
        long long	llX9TotalAmount;
	
	

	
        /* 紅利抵扣筆數 */
        long		lnRedeemSaleCount;
        long		lnRedeemRefundCount;
        long		lnRedeemTipsCount;
        long		lnRedeemTotalCount;
        long		lnRedeemTotalPoint;
        /* 紅利抵扣金額 */
        long long	llRedeemSaleAmount;
        long long	llRedeemRefundAmount;
        long long	llRedeemTipsAmount;
        long long	llRedeemTotalAmount;

        /* 分期付款筆數 */
        long		lnInstSaleCount;
        long		lnInstRefundCount;
        long		lnInstTipsCount;
        long		lnInstTotalCount;
        /* 分期付款金額 */
        long long	llInstSaleAmount;
        long long	llInstRefundAmount;
        long long	llInstTipsAmount;
        long long	llInstTotalAmount;
	
	/* 優惠兌換數量 */
	long		lnLoyaltyRedeemSuccessCount;	/* 兌換成功總數量 */
	long		lnLoyaltyRedeemCancelCount;	/* 兌換取消總數量 */
	long		lnLoyaltyRedeemTotalCount;	/* 兌換合計 */
	
	/* ESC */
	long		lnESC_BypassNum;		/* ESC上傳時，因Bypasst出簽單的次數 */
	long		lnESC_TotalFailULNum;	        /* ESC上傳時，有出簽單的次數 */
	long		lnESC_FailUploadNum;	        /* ESC上傳時，因上傳失敗出簽單的次數 */
	long		lnESC_SuccessNum;	        /* ESC上傳時，成功的次數 */
	long		lnESC_PreAuthNum;		/* ESC上傳時，預先授權的次數 */
        
 	long long	llESC_BypassAmount;		/* ESC上傳時，因Bypasst出簽單的總金額 */
	long long	llESC_TotalFailULAmount;        /* ESC上傳時，有出簽單的總金額 */
	long long	llESC_FailUploadAmount;	        /* ESC上傳時，因上傳失敗出簽單的總金額 */
	long long	llESC_SuccessAmount;            /* ESC上傳時，成功的總金額 */
	long long	llESC_PreAuthAmount;		/* ESC上傳時，預先授權的總金額 */
	
	long		lnTotalSale_SignCount;		/* 【需求單-113172】免簽商店存根聯參數及短簽單調整需求 需簽名筆數 by Russell 2025/5/15 下午 2:41 */
	long		lnTotalSale_NoSignCount;	/* 【需求單-113172】免簽商店存根聯參數及短簽單調整需求 不需簽名筆數 by Russell 2025/5/15 下午 2:41 */
        long long	llTotalSale_SignAmount;		/* 【需求單-113172】免簽商店存根聯參數及短簽單調整需求 需簽名金額 by Russell 2025/5/15 下午 2:41 */
	long long	llTotalSale_NoSignAmount;	/* 【需求單-113172】免簽商店存根聯參數及短簽單調整需求 不需簽名金額 by Russell 2025/5/15 下午 2:41 */
        long long	llTotalTips_SignAmount;		/* 【需求單-113172】免簽商店存根聯參數及短簽單調整需求 需簽名金額 by Russell 2025/5/15 下午 2:41 */
	long long	llTotalTips_NoSignAmount;	/* 【需求單-113172】免簽商店存根聯參數及短簽單調整需求 不需簽名金額 by Russell 2025/5/15 下午 2:41 */
} ACCUM_TOTAL_REC;


typedef struct
{
	/* 全部交易筆數 */
	long		lnTotalCount;
	long		lnDeductTotalCount;
	long		lnVoidDeductTotalCount;
	long		lnADDTotalCount;
	long		lnAutoADDTotalCount;
	long		lnVoidADDTotalCount;
	long		lnRefundTotalCount;
	
	long long	llTotalAmount;
	long long	llDeductTotalAmount;
	long long	llVoidDeductTotalAmount;
	long long	llADDTotalAmount;
	long long	llAutoADDTotalAmount;
	long long	llVoidADDTotalAmount;
        long long	llRefundTotalAmount;
        
        /* IPASS */
        long		lnIPASS_TotalCount;
	long		lnIPASS_DeductTotalCount;
	long		lnIPASS_VoidDeductTotalCount;
	long		lnIPASS_ADDTotalCount;
	long		lnIPASS_AutoADDTotalCount;
	long		lnIPASS_VoidADDTotalCount;
	long		lnIPASS_RefundTotalCount;
	long		lnIPASS_RewardTotalCount;
	
	long long	llIPASS_TotalAmount;
	long long	llIPASS_DeductTotalAmount;
	long long	llIPASS_VoidDeductTotalAmount;
	long long	llIPASS_ADDTotalAmount;
	long long	llIPASS_AutoADDTotalAmount;
	long long	llIPASS_VoidADDTotalAmount;
        long long	llIPASS_RefundTotalAmount;
        
        /* ECC */
        long		lnEASYCARD_TotalCount;
	long		lnEASYCARD_DeductTotalCount;
	long		lnEASYCARD_VoidDeductTotalCount;
	long		lnEASYCARD_ADDTotalCount;
	long		lnEASYCARD_AutoADDTotalCount;
	long		lnEASYCARD_VoidADDTotalCount;
	long		lnEASYCARD_RefundTotalCount;
	long		lnEASYCARD_RewardTotalCount;
	
	long long	llEASYCARD_TotalAmount;
	long long	llEASYCARD_DeductTotalAmount;
	long long	llEASYCARD_VoidDeductTotalAmount;
	long long	llEASYCARD_ADDTotalAmount;
	long long	llEASYCARD_AutoADDTotalAmount;
	long long	llEASYCARD_VoidADDTotalAmount;
        long long	llEASYCARD_RefundTotalAmount;
	
	/* ICASH */
        long		lnICASH_TotalCount;
	long		lnICASH_DeductTotalCount;
	long		lnICASH_VoidDeductTotalCount;
	long		lnICASH_ADDTotalCount;
	long		lnICASH_AutoADDTotalCount;
	long		lnICASH_VoidADDTotalCount;
	long		lnICASH_RefundTotalCount;
	long		lnICASH_RewardTotalCount;
	
	long long	llICASH_TotalAmount;
	long long	llICASH_DeductTotalAmount;
	long long	llICASH_VoidDeductTotalAmount;
	long long	llICASH_ADDTotalAmount;
	long long	llICASH_AutoADDTotalAmount;
	long long	llICASH_VoidADDTotalAmount;
        long long	llICASH_RefundTotalAmount;
} TICKET_ACCUM_TOTAL_REC;

typedef struct
{
        /* 全部交易筆數 */
        long		lnTotalSaleCount;
        long		lnTotalRefundCount;
        long		lnTotalCount;
        /* 全部交易金額 */
        long long	llTotalSaleAmount;
        long long	llTotalRefundAmount;
        long long	llTotalAmount;

        /* employee 全部交易筆數*/
        long		lnTAKA_EMPLOYEE_TotalSaleCount;
        long		lnTAKA_EMPLOYEE_TotalRefundCount;
        long		lnTAKA_EMPLOYEE_TotalCount;
        /* empolyee 全部交易金額 */
        long long	llTAKA_EMPLOYEE_TotalSaleAmount;
        long long	llTAKA_EMPLOYEE_TotalRefundAmount;
        long long	llTAKA_EMPLOYEE_TotalAmount;
	
	/* T_DAYEH 全部交易筆數*/
        long		lnTAKA_T_DAYEH_TotalSaleCount;
        long		lnTAKA_T_DAYEH_TotalRefundCount;
        long		lnTAKA_T_DAYEH_TotalCount;
        /* T_DAYEH 全部交易金額 */
        long long	llTAKA_T_DAYEH_TotalSaleAmount;
        long long	llTAKA_T_DAYEH_TotalRefundAmount;
        long long	llTAKA_T_DAYEH_TotalAmount;
	
	/* T_CARD 全部交易筆數*/
        long		lnTAKA_T_CARD_TotalSaleCount;
        long		lnTAKA_T_CARD_TotalRefundCount;
        long		lnTAKA_T_CARD_TotalCount;
        /* T_CARD 全部交易金額 */
        long long	llTAKA_T_CARD_TotalSaleAmount;
        long long	llTAKA_T_CARD_TotalRefundAmount;
        long long	llTAKA_T_CARD_TotalAmount;
	
	/* ESC */
	long		lnESC_BypassNum;		/* ESC上傳時，因Bypasst出簽單的次數 */
	long		lnESC_TotalFailULNum;	        /* ESC上傳時，有出簽單的次數 */
	long		lnESC_FailUploadNum;	        /* ESC上傳時，因上傳失敗出簽單的次數 */
	long		lnESC_SuccessNum;	        /* ESC上傳時，成功的次數 */
        
 	long long	llESC_BypassAmount;		/* ESC上傳時，因Bypasst出簽單的總金額 */
	long long	llESC_TotalFailULAmount;        /* ESC上傳時，有出簽單的總金額 */
	long long	llESC_SuccessAmount;            /* ESC上傳時，成功的總金額 */
} TAKA_ACCUM_TOTAL_REC;

#define _ACCUM_REC_SIZE_                sizeof(ACCUM_TOTAL_REC)
#define _TICKET_ACCUM_REC_SIZE_		sizeof(TICKET_ACCUM_TOTAL_REC)
#define _TAKA_ACCUM_REC_SIZE_		sizeof(TAKA_ACCUM_TOTAL_REC)