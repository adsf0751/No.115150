/* 
 * File:   ECR_Struct.h
 * Author: RussellBai
 *
 * Created on 2022年8月12日, 下午 3:50
 */
	
#include "ECR_Define.h"

typedef struct
{
	char		szTransType[2 + 1];
	char		szResponseCode[4 + 1];
	char		szCommandDataLength[4 + 1];
	char		szAPIResponseCode[2 + 1];
	char		szCommandData[500 + 1];
	unsigned char	uszIsError;	/* On起來代表API沒有值 */
	int		inErrorType;	/* 0:no error 1:Timeout 2:Cancel*/
}ECR_TRANSACTION_DATA_CUS_096;

typedef struct
{
	char		szFPG_EFID[4 + 1];
	char		szSVC_EFID[4 + 1];
	char		szFPG_FTC_Invoice[10 + 1];
	unsigned char	uszFPG_FTC_CardBit;
	unsigned char	uszHappyGoCardBit;
}ECR_TRANSACTION_DATA_CUS_005;

typedef struct
{
	char		szEW_NormalAmt[10 + 1];			/* 一般交易金額 *//* 一般退貨金額（不含小數位）。,iCashPay：RefundItemAmt,（右靠左補0、非適用業者全帶空白space） */
	char		szEW_CollectionAmt[10 + 1];		/* 代收交易金額 *//* 代收交易金額 代收退貨金額（不含小數位）。悠遊付：rebateNotApplicableAmount,iCashPay：RefundUtilityAmt（右靠左補0、非適用業者全帶空白space）*/
	char		szEW_ConsignmentAmt[10 + 1];		/* 代售交易金額 *//* 代售退貨金額（不含小數位）。悠遊付：rebateNotApplicableAmount,ICash Pay：CommAmt、RefundCommAmt（右靠左補0、非適用業者全帶空白space） */
	char		szEW_PromotionRestriction[10 + 1];	/* 不可折抵金額(LinePay：extras.promotionRestriction,悠遊付：discountNotApplicableAmount,iCashPay：ItemNonRedeemAmt,全盈+PAY：NonRebatesAmount,全支付：none_discount_amount,Pi 錢包：nondiscount_amt,（右靠左補0）) */
	char		szEW_EWIssuerID[2 + 1];			/* 電子錢包業者代碼,LinePay： 21,悠遊付：22,iCashPay：23,全盈+PAY：24,全支付：25,Pi 錢包：26 */
	char		szEW_OrderId[30 + 1];			/* 特店訂單編號(LinePay：orderId,悠遊付：merchantOrderNo,iCashPay：MerchantTradeNo,全盈+PAY：OrderNo,全支付：mer_trade_no,Pi 錢包：bill_id) （左靠右補空白）*//* 退貨訂單編號。,iCashPay：MerchantTradeNo,全支付：mer_trade_no,（左靠右補空白） */
	char		szEW_EinvoiceHASH[50 + 1];		/* 電子發票HASH 值,LinePay：info[].merchantReference.affiliateCards[].cardId,悠遊付：einvoiceCarrierNo,iCashPay：MobileInvoiceCarry,全盈+PAY：InvoiceVehicle,全支付：invo_carrier,Pi 錢包：carrier_id_2,（左靠右補空白）*/
	char		szEW_RefundTradeNo[30 + 1];		/* 退貨訂單編號。,iCashPay：MerchantTradeNo,全支付：mer_trade_no,（左靠右補空白） */
	char		szEW_TransactionId[64 + 1];		/* 錢包業者交易序號。LinePay：transactionId,悠遊付：orderNo,iCashPay：TransactionID,全盈+PAY：BankOrderNo,全支付：px_trade_no,Pi 錢包：wallet_transaction_id,（左靠右補空白）*/
	char		szEW_ChannelId[50 + 1];			/* 錢包業者特店代號(預設由ATS 抓主檔回覆)。LinePay：channelId,悠遊付：sellerMemberUid,iCashPay：MerchantID,全盈+PAY：StoreID,全支付：store_id,Pi 錢包：channel_id,（左靠右補空白）*/
	char		szEW_PayMethod[1 + 1];			/* 消費者支付工具，ATS 判斷後回覆給EDC。,LinePay：info.payInfo[].method,悠遊付：paymentMethod,iCashPay：PaymentType,全盈+PAY：PaymentMethod,全支付：pay_tool,Pi 錢包：payment_type */
	char		szEW_TransactionType[2 + 1];		/* 訂單狀態。LinePay：info.transactionType,悠遊付：transactionAction,iCashPay：TradeType,全盈+PAY：OrderStatus,全支付：order_type,Pi 錢包：trans_status,訂單狀態詳下說明三 */
	char		szEW_PaymentNo[30 + 1];			/* 付款交易編號。,悠遊付：paymentNo */
	char		szEW_IsDuplicatedPaymentRequest[1 + 1];	/* 重複付款請求。Y/N,悠遊付：isDuplicatedPaymentRequest */
	char		szEW_CobrandedCode[11 + 1];		/* 聯名卡代碼。,悠遊付：cobrandedCode */
	char		szEW_EinvoiceCarrierType[6 + 1];	/* 載具類型。LinePay：info.merchantReference.affiliateCards[].cardType,悠遊付：einvoiceCarrierType（左靠右補空白）*/
	char		szEW_DiscountAmt[10 + 1];		/* 折抵金額（不含小數位）。,悠遊付：orderAmount 減去paymentAmount,iCashPay：BonusAmt,全盈+PAY：DiscountAmount,全支付：discount_amount,Pi 錢包：psp_bonuspoint_amount（右靠左補0、無資料請全帶0）*//* 退還折抵金額（不含小數位）。iCashPay：RefundBonusAmt,全盈+PAY：ReturnRewardAmount,全支付：discount_amount,Pi 錢包：psp_bonuspoint_amount（右靠左補0、無資料請全帶0）*/
	char		szEW_Store_no[24 + 1];			/* 門市代碼。,Pi 錢包：store_no */
	char		szEW_MaskCreditCardNo[19 + 1];		/* 遮罩帳號/卡號。悠遊付：maskCreditCardNo,iCashPay：MaskedPan,全支付：identity */
	unsigned char	uszEWTransBit;				/* 是否為電子錢包交易 */
}ECR_TRANSACTION_DATA_EW;

typedef struct
{
	char		szTrustRRN[12 + 1];			/* 共用收單序號 */
	char		szReconciliationNo[20 + 1];		/* 銷帳編號，若無則全帶空白。左靠右補空白 */
	char		szTrustInstitutionCode[7 + 1];		/* 信託機構代碼。 左靠右補空白 */
	char		szTrustOrgDate[4 + 1];			/* 信託原交易日期 MMDD */
	unsigned char	uszTRUSTTransBit;			/* 是否為信託交易 */
}ECR_TRANSACTION_DATA_TRUST;

typedef struct
{
	int		inVersion;
	int		inUDPVersion;
	int		inTimeout;
        int             inCustomerLen;
	int		inNonDefaultRetryTimes;
	unsigned char	uszComPort;		/* Verifone用handle紀錄，Castle用Port紀錄 */
	unsigned char	uszSettingOK;		/* 用來標注是否已設定好RS232 */
	unsigned char	uszNonDefaultRetry;	/* 是否不要套用預設Retry次數 */
}ECR_SETTING;

typedef struct
{
	int		inErrorType;
	int		inSendPacketSize;		/* 客製化079使用，回傳非固定長度 */
	char		szECRIndicator[2 + 1];
	char		szTransTypeIndicator[2 + 1];
	char		szTransType[2 + 1];
	char		szField_05[2 + 1];		/* CUP / Smart pay / ESVC Indicator */
	char		szHostID[2 + 1];
	char		szReceiptNo[6 + 1];
	char		szCardNo[19 + 1];
	char		szField_09[4 + 1];		/* Card Expire Date/結帳總筆數 */
	char		szField_10[12 + 1];		/* Trans Amount / 結帳總金額 */
	char		szTransDate[6 + 1];
	char		szTransTime[6 + 1];
	char		szApprovalNo[6 + 1];
	char		szWaveCardIndicator[1 + 1];
	char		szECRResponseCode[4 + 1];
	char		szMerchantID[15 + 1];
	char		szTerminalID[8 + 1];
	char		szExpAmount[12 + 1];
	char		szStoreId[20 + 1];
	char		szField_20[1 + 1];		/* Installment/Redeem Indicator */
	char		szRDMPaidAmt[12 + 1];
	char		szRDMPoint[12 + 1];
	char		szPointsOfBalance[8 + 1];
	char		szRedeemAmt[12 + 1];
	char		szInstallmentPeriod[2 + 1];
	char		szField_26[12 + 1];	/* Down Payment Amount /ESVC Balance before Tx. */
	char		szField_27[12 + 1];	/* Installment Payment Amount/ESVC Balance after Tx. */
	char		szField_28[12 + 1];	/* Formality Fee/ESVC Autoload Amount */
	char		szCardType[2 + 1];
	char		szBatchNo[6 + 1];
	char		szStartTransType[2 + 1];
	char		szMPFlag[1 + 1];
	char		szSPIssuerID[8 + 1];
	char		szField_34[8 + 1];	/* SP / 信用卡/ESVC Origin Date */
	char		szField_35[12 + 1];	/* SP Origin RRN / 信用卡OriginRRN/ATS電票交易序號 */
	char		szPayItem[5 + 1];
	char		szCardNoHashValue[50 + 1];
	char		szMPResponseCode[6 + 1];
	char		szASMAwardFlag[1 + 1];
	char		szMCPIndicator[1 + 1];
	char		szIssuerBankID[3 + 1];
	char		szBarCode1[20 + 1];
	char		szBarCode2[20 + 1];
	char		szHGPaymentTool[2 + 1];
	char		szHGRedeemPoint[8 + 1];
	char		szAuthAmount[12 + 1];		/* 預先授權金額 */
	char		szOrgData[1000 + 1];		/* 用來存原始資料做備份 */
	char		szBarCodeLen[3 + 1];		/* 一維或二維條碼資料長度 */
	char		szBarCodeData[600 + 1];		/* 一維或二維條碼資料內容(最大值設600一定夠) */
	char		szUnyTransCode[20 + 1];		/* 銀聯優計劃：space(18)Üny 退貨交易需傳送此欄位 */
	char		szPOSTxUniqueNo[_ECR_UDP_HEADER_SIZE_ + 1];	/* UDP使用 */
	char		szCus027IdentifyCode[4 + 1];	/* 4碼識別碼 */
	char            szFlightTicketTransBit[1 + 1];                   /* 判斷是否是機票交易 不設定為空值 機票交易為1 非機票交易為2 */
        char            szFlightTicketPDS0523[5 + 1];                   /* 出發地機場代碼（PDS 0523）左靠右補空白 */
        char            szFlightTicketPDS0524[5 + 1];                   /* 目的地機場代碼（PDS 0524）左靠右補空白 */
        char            szFlightTicketPDS0530[5 + 1];                   /* 航班號碼（PDS 0530） */
	unsigned char	uszIsResponse;			/* 用來確認是否回過ECR，避免送兩次回覆(例如:Call Bank是Send Error卻繼續跑流程) */
        unsigned char	uszUseOrgData;			/* 用原始資料發動ECR，客製化098使用 */
	unsigned char	uszECRResponsePayitem;
	ECR_TRANSACTION_DATA_CUS_096	srCus096_Data;	/* 直連Ipass，客製化096使用 */
	ECR_TRANSACTION_DATA_CUS_005	srCus005_Data;	/* 客製化005使用 */
	ECR_TRANSACTION_DATA_EW		srEW_Data;	/* 電子錢包專用 */
	ECR_TRANSACTION_DATA_TRUST	srTRUST_Data;	/* 信託專用 */
}ECR_TRANSACTION_DATA;

typedef struct
{
	int		inErrorType;
	char		szTransType[2 + 1];
	char		szDataLen[4 + 1];
	char		szCRC[4 + 1];
	char		szSQNo[1];
	unsigned char	uszIsResponse;			/* 用來確認是否回過ECR，避免送兩次回覆(例如:Call Bank是Send Error卻繼續跑流程) */
}ECR_TRANSACTION_DATA_TSB;

typedef struct
{
	unsigned char	uszNotSendAck;			/* 若On起來，代表接收完對方Request不用送ACK給對方 */
	unsigned char	uszNotReceiveAck;		/* 若On起來，代表送完Response不用等對方回傳ACK */
}ECR_OPTIONAL_SETTING;

typedef struct
{
	ECR_SETTING			srSetting;	/* initial完不會動的 */
	ECR_TRANSACTION_DATA		srTransData;	/* 每次接收都要清空 */
	ECR_TRANSACTION_DATA_TSB	srTransDataTSB;	/* 台新專用結構 */
	ECR_OPTIONAL_SETTING		srOptionalSetting;
}ECR_TABLE;


/* 用來決定要跑那一個ECR版本 */
typedef struct
{
	int (*inEcrInitial)(ECR_TABLE *srECROb);
	int (*inEcrRece)(TRANSACTION_OBJECT *, ECR_TABLE *srECROb);
	int (*inEcrSend)(TRANSACTION_OBJECT *, ECR_TABLE *srECROb);
        int (*inEcrMirror)(TRANSACTION_OBJECT *, ECR_TABLE *srECROb);
	int (*inEcrSendError)(TRANSACTION_OBJECT *, ECR_TABLE *srECROb);
        int (*inEcrEIRece)(TRANSACTION_OBJECT *, ECR_TABLE *srECROb);
	int (*inEcrEISend)(TRANSACTION_OBJECT *, ECR_TABLE *srECROb);
	int (*inEcrEnd)(ECR_TABLE *);
}ECR_TRANS_TABLE;

typedef struct
{
	int     inTotalPacket;
        int     inCurrentPacket;
        long    lnTotalSize;
        long    lnDataSize;
        char    szPrintTime[9 + 1];     /* 比對，防止封包重複 */
        char    szHeader[23 + 1];       /* 回傳用 */
        unsigned char	uszCreatReadData;
        char	*szReadData;
}EI_TABLE;

typedef struct TagFTC_INFO_REC
{
	char szEFID_1[4 + 1];			/* FPG 點數的錢包 */
	char szEFID_2[4 + 1];			/* 各通路儲值金的錢包 */
	char szFE01_DATA[63 + 1];		/* 卡片基本資料檔 1 */
	char szFE02_DATA[54 + 1];		/* 卡片基本資料檔 2 */
	unsigned long lnOil;			/* 油量 */
	unsigned long lnFPG_Credit_Point;	/* FPG 加點 */
	unsigned long lnFPG_Debit_Point;	/* FPG 扣點 */
	unsigned long lnSVC_Credit_Amt;		/* SVC 加點 */
	unsigned long lnSVC_Debit_Amt;		/* SVC 扣點 */
} FTC_INFO_REC;

typedef struct TagFPG_FTC_EFIDREC
{
	unsigned fActive		: 1;
	unsigned fTxnOK			: 1;
	unsigned fCredit		: 1;
	unsigned fDebit			: 1;
	long lnTxnAmt;
	long lnBalanceAmt;
	unsigned char bEFID[FPG_FTC_EFID_SIZE + 1];
	unsigned char bSignture[SIGNTURE_SIZE + 1];
} FPG_FTC_EFID_REC;

typedef struct TagFPG_FTC_REC
{
	unsigned char usEFID[FPG_FTC_EFID_SIZE + 1];
	unsigned char usMemberNO[FPG_FTC_MEMBER_ID_SIZE + 1];		/* FE01_會員編號 */
	unsigned char usCardType[FPG_FTC_CARD_TYPE_SIZE + 1];		/* FE01_卡別 */
	unsigned char usMemberCode[FPG_FTC_MEMBER_CODE_SIZE + 1];	/* FE01_身分代號 */
	unsigned char usRange[FPG_FTC_RANGE_SIZE + 1];			/* FE01_等級 */
	unsigned char usInvoiceNO[FPG_FTC_INVOICE_NO_SIZE + 1];		/* FE01_發票列印統編 */
	unsigned char usCardNum[FPG_FTC_CARD_NUM_SIZE + 1];		/* FE01_車號 */
	unsigned char usOilCode[FPG_FTC_OIL_CODE_SIZE + 1];		/* FE01_油品代號 */
	unsigned char usBasicData1[FPG_FTC_BASIC_DATA_SIZE + 1];	/* FE01_Basic_Data_1 */
	unsigned char usCustName[FPG_FTC_FE02_CUSTOMER_NAME_SIZE + 1];	/* FE02_姓名 */
	unsigned char usBirthday[FPG_FTC_BIRTHDAY_SIZE + 1];		/* FE02_生日 */
	unsigned char usMemberID[FPG_FTC_MEMBER_ID_SIZE + 1];		/* FE02_會員卡號 */

	unsigned char usEFID1[10 + 1];
	unsigned char usEFID2[10 + 1];
	long lnOilQuan; /* A Data */
	unsigned fUpdata : 1;
        unsigned fUpdataOK : 1;
        int inTxnIndex;
        long lnLSN;
	long lnDSN;
	short inAIDLen;
	unsigned char usPID[FPG_FTC_PID_SIZE + 1];
	unsigned char usAuthCode[SIGNTURE_SIZE + 1];
	unsigned char usRandomNO[SIGNTURE_SIZE + 1];
	FPG_FTC_EFID_REC srFPG_FTC_EFIDRec;  /* 2013-04-11 AM 08:55:28 add by kakab 舊TMS轉置新TMS台亞版收銀機連線開發 */
}FPG_FTC_REC;
