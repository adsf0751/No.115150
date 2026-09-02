/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Transaction_struct.h
 * Author: 1500414
 *
 * Created on 2025年10月3日, 下午 5:53
 */

#ifndef TRANSACTION_STRUCT_H
#define TRANSACTION_STRUCT_H

#ifdef __cplusplus
extern "C" {
#endif




#ifdef __cplusplus
}
#endif

#endif /* TRANSACTION_STRUCT_H */

typedef struct TagBATCH_REC
{
	int inCode; /* Trans Code */
	int inOrgCode; /* Original Trans Code  */
	int inPrintOption; /* Print Option Flag */
	int inHDTIndex; /* 紀錄HDTindex */
	int inCDTIndex; /* 紀錄CDTindex */
	int inCPTIndex; /* 紀錄CPTindex */
	int inTxnResult; /* 紀錄交易結果 */
	int inChipStatus; /* 0 NOT_USING_CHIP, 1 EMV_CARD, 2 EMV_EASY_ENTRY_CARD */
	int inFiscIssuerIDLength; /* 金融卡發卡單位代號長度 */
	int inFiscCardCommentLength; /* 金融卡備註欄長度 */
	int inFiscAccountLength; /* 金融卡帳號長度 */
	int inFiscSTANLength; /* 金融卡交易序號長度 */
	int inESCTransactionCode; /* ESC組ISO使用 重新上傳使用 Transaction Code沒辦法存在Batch */
	int inESCUploadMode; /* 標示支不支援ESC */
	int inESCUploadStatus; /* 標示ESC上傳狀態 */
	int inSignStatus; /* 簽名檔狀態(有 免簽 或 Bypass) ESC電文使用 */
	int inHGCreditHostIndex; /* 聯合_HAPPY_GO_信用卡主機 */
	int inHGCode; /* 聯合_HAPPY_GO_交易碼 */
	long lnTxnAmount; /* The transaction amount, such as a SALE */
	long lnOrgTxnAmount; /* The ORG transaction amount, such as a SALE */
	long lnTipTxnAmount; /* The transaction amount, such as a TIP */
	long lnAdjustTxnAmount; /* The transaction amount, such as a ADJUST */
	long lnTotalTxnAmount; /* The transaction amount, such as a TOTAL */
	long lnInvNum; /* 調閱編號  */
	long lnOrgInvNum; /* Original 調閱編號  */
	long lnBatchNum; /* Batch Number */
	long lnOrgBatchNum; /* Original Batch Number */
	long lnSTANNum; /* Stan Number */
	long lnOrgSTANNum; /* Original Stan Number，離線或混合交易使用 */
	long lnInstallmentPeriod; /* 分期付款_期數 */
	long lnInstallmentDownPayment; /* 分期付款_頭期款 */
	long lnInstallmentPayment; /* 分期付款_每期款 */
	long lnInstallmentFormalityFee; /* 分期付款_手續費 */
	long lnRedemptionPoints; /* 紅利扣抵_扣抵紅利點數 */
	long lnRedemptionPointsBalance; /* 紅利扣抵_剩餘紅利點數 */
	long lnRedemptionPaidCreditAmount; /* 紅利扣抵_實際支付金額（消費者付的金額） */
	long lnHGTransactionType; /* 聯合_HAPPY GO_交易類別 */
	long lnHGPaymentType; /* 聯合_HAPPY_GO_支付工具 */
	long lnHGPaymentTeam; /* 聯合_HAPPY_GO_支付工具_主機回_*/
	long lnHGBalancePoint; /* 聯合_HAPPY_GO_剩餘點數 */
	long lnHGTransactionPoint; /* 聯合_HAPPY_GO_交易點數  合計 */
	long lnHGAmount; /* 聯合_HAPPY_GO_扣抵後金額  (商品金額 = lnHGAmount + lnHGRedeemAmt) */
	long lnHGRedeemAmount; /* 聯合_HAPPY_GO_扣抵金額 */
	long lnHGRefundLackPoint; /* 聯合_HAPPY_GO_不足點數 */
	long lnHGBatchIndex; /* 聯合_HAPPY_GO_主機當下批次號碼 */
	long lnHG_SPDH_OrgInvNum; /* HAPPY_GO取消用INV */
	long lnHGSTAN; /* HAPPY_GO STAN */
	long lnCUPUPlanDiscountedAmount; /* 銀聯優計畫折價後金額 */
	long lnCUPUPlanPreferentialAmount; /* 銀聯優計畫優惠金額 */
	char szAuthCode[_AUTH_CODE_SIZE_ + 1]; /* Auth Code */
	char szMPASAuthCode[_AUTH_CODE_SIZE_ + 1]; /* MPAS Auth Code */
	char szRespCode[4 + 1]; /* Response Code */
	char szStoreID[50 + 1]; /* StoreID */
	char szCardLabel[20 + 1]; /* 卡別  */
	char szPAN[20 + 1]; /* 卡號  */
	char szDate[8 + 1]; /* YYYYMMDD */
	char szOrgDate[8 + 1]; /* YYYYMMDD */
	char szTime[6 + 1]; /* HHMMSS */
	char szOrgTime[6 + 1]; /* HHMMSS */
	char szCardTime[15 + 1]; /* 晶片卡讀卡時間 , YYYYMMDDHHMMSS */
	char szRefNo[12 + 1]; /* 序號  */
	char szExpDate[_EXP_DATE_SIZE_ + 1]; /* Expiration date YYMM */
	char szServiceCode[_SVC_CODE_SIZE_ + 1]; /* Service code from track */
	char szCardHolder[_CARD_HOLDER_NAME_SIZE_ + 1]; /* 持卡人名字 */
	char szAMEX4DBC[4 + 1];
	char szFiscIssuerID[8 + 1]; /* 發卡單位代號 */
	char szFiscCardComment[30 + 1]; /* 金融卡備註欄 */
	char szFiscAccount[16 + 1]; /* 金融卡帳號 */
	char szFiscOutAccount[16 + 1]; /* 金融卡轉出帳號 */
	char szFiscSTAN[8 + 1]; /* 金融卡交易序號 */
	char szFiscTacLength[2 + 1]; /* 金融卡Tac長度 */
	char szFiscTac[32 + 1]; /* 金融卡Tac */
	char szFiscTCC[8 + 1]; /* 端末設備查核碼 */
	char szFiscMCC[16 + 1]; /* 金融卡MCC */
	char szFiscRRN[12 + 1]; /* 金融卡調單編號 */
	char szFiscRefundDate[8 + 1]; /* 金融卡退貨原始交易日期(YYYYMMDD) */
	char szFiscDateTime[14 + 1]; /* 計算TAC(S2)的交易日期時間 */
	char szFiscPayDevice[2 + 1]; /* 金融卡付款裝置 1 = 手機 2 = 卡片 */
	char szFiscMobileDevice[2 + 1]; /* SE 類型，0x05：雲端卡片(Cloud-Based) */
	char szFiscMobileNFType[2 + 1]; /* 近端交易類型，行動金融卡是否需輸入密碼 00不需要 01視情況 02一定要 */
	char szFiscMobileNFSetting[2 + 1]; /* 近端交易類型設定 0x00：Single Issuer Wallet 0x01：國內Third-Party Wallet 0x02~9：保留 0x0A：其他 */
	char szInstallmentIndicator[2 + 1];
	char szRedeemIndicator[2 + 1];
	char szRedeemSignOfBalance[1 + 1];
	char szHGCardLabel[20 + 1]; /* HAPPY_GO 卡別 */
	char szHGPAN[20 + 1]; /* HAPPY_GO Account number */
	char szHGAuthCode[6 + 1]; /* HAPPY_GO 授權碼 */
	char szHGRefNo[12 + 1]; /* HAPPY_GO Reference Number */
	char szHGRespCode[6 + 1]; /* HG Response Code */
	char szCUP_TN[6 + 1]; /* CUP Trace Number (TN) */
	char szCUP_TD[4 + 1]; /* CUP Transaction Date (MMDD) */
	char szCUP_TT[6 + 1]; /* CUP Transaction Time (hhmmss) */
	char szCUP_RRN[12 + 1]; /* CUP Retrieve Reference Number (CRRN) */
	char szCUP_STD[4 + 1]; /* CUP Settlement Date(MMDD) Of Host Response */
	char szCUP_EMVAID[32 + 1]; /* CUP晶片交易存AID帳單列印使用 */
	char szTranAbbrev[2 + 1]; /* Tran abbrev for reports */
	char szIssueNumber[2 + 1]; /* */
	char szStore_DREAM_MALL[36 + 1]; /* 存Dream_Mall Account Number And Member ID*/
	char szDCC_FCNFR[3 + 1]; /* Foreign Currency No. For Rate */
	char szDCC_AC[4 + 1]; /* Action Code */
	char szDCC_FCN[3 + 1]; /* Foreign Currency Number */
	char szDCC_FCA[12 + 1]; /* Foreign Currency Amount */
	char szDCC_FCMU[2 + 1]; /* Foreign Currency Minor Unit */
	char szDCC_FCAC[3 + 1]; /* Foreign currcncy Alphabetic Code */
	char szDCC_ERMU[2 + 1]; /* Exchange Rate Minor Unit */
	char szDCC_ERV[9 + 1]; /* Exchange Rate Value */
	char szDCC_IRMU[2 + 1]; /* Inverted Rate Minor Unit */
	char szDCC_IRV[9 + 1]; /* Inverted Rate Value */
	char szDCC_IRDU[2 + 1]; /* Inverted Rate Display Unit */
	char szDCC_MPV[8 + 1]; /* Markup Percentage Value */
	char szDCC_MPDP[2 + 1]; /* Markup Percentage Decimal Point */
	char szDCC_CVCN[3 + 1]; /* Commissino Value Currency Number */
	char szDCC_CVCA[12 + 1]; /* Commission Value Currency Amount */
	char szDCC_CVCMU[2 + 1]; /* Commission Value Currency Minor Unit */
	char szDCC_TIPFCA[12 + 1]; /* Tip Foreign Currency Amount */
	char szDCC_OTD[4 + 1]; /* Original Transaction Date & Time (MMDD) */
	char szDCC_OTA[12 + 1]; /* Original Transaction Amount */
	char szProductCode[42 + 1]; /* 產品代碼 */
	char szAwardNum[2 + 1]; /* 優惠個數 */
	char szAwardSN[22 + 1]; /* 優惠序號(Award S/N) TID(8Bytes)+YYYYMMDDhhmmss(16 Bytes)，共22Bytes */
	char szTxnNo[24 + 1]; /* 交易編號 */
	char szMCP_BANKID[32 + 1]; /* 行動支付標記 金融機構代碼 */
	char szPayItemCode[5 + 1]; /* 繳費項目代碼 */
	char szTableTD_Data[15 + 1]; /* Table TD的資料， */
	char szDFSTraceNum[6 + 1]; /* DFS交易系統追蹤號 */
	char szCheckNO[5 + 1]; /* 檢查碼，目前FOR SDK使用 */
	char szEIVI_BANKID[6 + 1]; /* E Invoice vehicle I ssue Bank ID 支援電子發票載具之發卡行代碼 */
	char szBarCodeData[600 + 1]; /* 掃碼交易資料內容 */
	char szUnyTransCode[20 + 1]; /* Uny交易碼 */
	char szUnyMaskedCardNo[20 + 1]; /* 遮掩交易卡號資料(非完整卡號，有遮掩，故另設欄位) */
	char szEInvoiceHASH[44 + 1]; /* 電子發票HASH 值（發卡銀行若無參加電子發票載具，則此欄位回覆值 為空白 SPACE 。） */
	char szUnyCardLabelLen[2 + 1]; /* 卡別名稱長度，szMCP_BANKID為Twin卡專用，故另設欄位 */
	char szUnyCardLabel[20 + 1]; /* 卡別名稱，szMCP_BANKID為Twin卡專用，故另設欄位 */
	char szFPG_FTC_Invoice[10 + 1]; /* 台塑生醫生技客製化專屬【005】　台亞福懋加油站客製化專屬【006】發票號碼 */
	char szUPlan_Coupon[30 + 1]; /* 銀聯優計畫的優惠訊息 */
	char szUPlan_RemarksInformation[120 + 1]; /* 銀聯優計畫-備註訊息 */
	char szFlightTicketTransBit[1 + 1]; /* 判斷是否是機票交易 不設定為空值 機票交易為1 非機票交易為2 */
	char szFlightTicketPDS0523[5 + 1]; /* 出發地機場代碼（PDS 0523）左靠右補空白 */
	char szFlightTicketPDS0524[5 + 1]; /* 目的地機場代碼（PDS 0524）左靠右補空白 */
	char szFlightTicketPDS0530[5 + 1]; /* 航班號碼（PDS 0530） */
	unsigned char uszWAVESchemeID; /* WAVE 使用用於組電文 Field_22 */
	unsigned char uszVOIDBit; /* 負向交易 */
	unsigned char uszUpload1Bit; /* Offline交易使用 (原交易advice是否未上傳)*/
	unsigned char uszUpload2Bit; /* Offline交易使用 (原交易advice調帳是否未上傳)*/
	unsigned char uszUpload3Bit; /* Offline交易使用 (原交易advice調帳的取消是否未上傳)*/
	unsigned char uszReferralBit; /* ISO Response Code 【01】【02】使用 */
	unsigned char uszOfflineBit; /* 離線交易 */
	unsigned char uszManualBit; /* Manual Keyin */
	unsigned char uszNoSignatureBit; /* 免簽名使用 (免簽名則為true)*/
	unsigned char uszCUPTransBit; /* 是否為CUP */
	unsigned char uszFiscTransBit; /* SmartPay交易，是否為金融卡 */
	unsigned char uszContactlessBit; /* 是否為非接觸式 */
	unsigned char uszEMVFallBackBit; /* 是否要啟動fallback */
	unsigned char uszInstallmentBit; /* Installment */
	unsigned char uszRedeemBit; /* Redemption */
	unsigned char uszForceOnlineBit; /* 組電文使用 Field_25 Point of Service Condition Code */
	unsigned char uszMail_OrderBit; /* 組電文使用 Field_25 Point of Service Condition Code */
	unsigned char uszDCCTransBit; /* 是否為DCC交易 */
	unsigned char uszNCCCDCCRateBit; /* 詢價後轉台幣 */
	unsigned char uszCVV2Bit; /* 有輸入CVV2 */
	unsigned char uszRewardSuspendBit; /* 暫停優惠服務(優惠活動為0且要印補充資訊) */
	unsigned char uszRewardL1Bit; /* 要印L1 */
	unsigned char uszRewardL2Bit; /* 要印L2 */
	unsigned char uszRewardL5Bit; /* 要印L5 */
	unsigned char uszField24NPSBit;
	unsigned char uszVEPS_SignatureBit; /* VEPS 免簽名是否成立 */
	unsigned char uszTCUploadBit; /* TCUpload是否已上傳 */
	unsigned char uszFiscConfirmBit; /* SmartPay 0220 是否已上傳 */
	unsigned char uszFiscVoidConfirmBit; /* SmartPay Void 0220 是否已上傳 */
	unsigned char uszESCMerchantCopyBit; /* 是否加印商店聯 */
	unsigned char uszPinEnterBit; /* 此筆交易是否有鍵入密碼(只能確認原交易，若該筆之後的調整或取消不會將此Bit Off) */
	unsigned char uszL2PrintADBit; /* L2是否印AD，因L2原交易取消要判斷，只好增加 */
	unsigned char uszInstallment; /* HappyGo分期交易 */
	unsigned char uszRedemption; /* HappyGo點數兌換 */
	unsigned char uszHappyGoSingle; /* HappyGo交易 */
	unsigned char uszHappyGoMulti; /* HappyGo混合交易 */
	unsigned char uszCLSBatchBit; /* 是否已接續上傳 此Bit設定時值為'1' */
	unsigned char uszTxNoCheckBit; /* 商店自存聯卡號遮掩開關 */
	unsigned char uszSpecial00Bit; /* 特殊卡別參數檔，活動代碼00表示免簽(只紀錄，主要看uszNoSignatureBit) */
	unsigned char uszSpecial01Bit; /* 特殊卡別參數檔，活動代碼01表示ECR回傳明碼(先決條件ECR卡號遮掩有開才做判斷) */
	unsigned char uszRefundCTLSBit; /* 用在簽單印(W) 因為送電文contactless bit已OFF轉Manual Key in */
	unsigned char uszMPASTransBit; /* 標示為小額交易 */
	unsigned char uszMPASReprintBit; /* 標示該小額交易是否可重印 */
	unsigned char uszMobilePayBit; /* 判斷是不是行動支付交易 由主機回傳 */
	unsigned char uszUCARDTransBit; /* 判斷是否是自由品牌卡 */
	unsigned char uszUnyTransBit; /* 是否是Uny交易 */
	unsigned char uszESCOrgUploadBit; /* 用於標明原交易的ESC上傳狀態 */
	unsigned char uszESCTipUploadBit; /* 用於標明Tip的ESC上傳狀態 */
	unsigned char uszTakaTransBit; /* 客製化026、021 Taka社員卡交易 */
	unsigned char uszUPlanECRBit; /* 判斷是否為優計畫ECR規格(用以決定是否回傳hash value) */
	unsigned char uszCUPEMVQRCodeBit; /* 判斷是否使用銀聯QRCode支付 */
	unsigned char uszUPlanTransBit; /* 判斷是否為優計畫交易(使用銀聯優惠券) */
} BATCH_REC;

typedef struct TagEMV_REC
{
        int		inEMVCardDecision;
        int		in50_APLabelLen;
        int		in5A_ApplPanLen;
        int		in5F20_CardholderNameLen;
        int		in5F24_ExpireDateLen;
	int		in5F28_IssuerCountryCodeLen;
        int		in5F2A_TransCurrCodeLen;
        int		in5F34_ApplPanSeqnumLen;
        int		in71_IssuerScript1Len;
        int		in72_IssuerScript2Len;
        int		in82_AIPLen;
        int		in84_DFNameLen;
        int		in8A_AuthRespCodeLen;
        int		in91_IssuerAuthDataLen;
        int		in95_TVRLen;
        int		in9A_TranDateLen;
        int		in9B_TSILen;
        int		in9C_TranTypeLen;
        int		in9F02_AmtAuthNumLen;
        int		in9F03_AmtOtherNumLen;
        int		in9F08_AppVerNumICCLen;
        int		in9F09_TermVerNumLen;
        int		in9F10_IssuerAppDataLen;
        int		in9F18_IssuerScriptIDLen;
        int		in9F1A_TermCountryCodeLen;
        int		in9F1E_IFDNumLen;
        int		in9F26_ApplCryptogramLen;
        int		in9F27_CIDLen;
        int		in9F33_TermCapabilitiesLen;
        int		in9F34_CVMLen;
        int		in9F35_TermTypeLen;
        int		in9F36_ATCLen;
        int		in9F37_UnpredictNumLen;
        int		in9F41_TransSeqCounterLen;
	int		in9F42_ApplicationCurrencyCodeLen;
        int		in9F5A_Application_Program_IdentifierLen;
        int		in9F5B_ISRLen;
        int		in9F60_CouponInformationLen;
        int		in9F63_CardProductLabelInformationLen;
        int		in9F66_QualifiersLen;
        int		in9F6C_Card_Transaction_QualifiersLen;
        int		in9F6E_From_Factor_IndicatorLen;
        int		in9F74_TLVLen;
	int		in9F7C_PartnerDiscretionaryDataLen;
        int		inDF69_NewJspeedyModeLen;
        int		inDF8F4F_TransactionResultLen;
        int		inDFEC_FallBackIndicatorLen;
        int		inDFED_ChipConditionCodeLen;
        int		inDFEE_TerEntryCapLen;
        int		inDFEF_ReasonOnlineCodeLen;
        unsigned char	usz50_APLabel[16 + 1];
        unsigned char	usz5A_ApplPan[10 + 1];
        unsigned char	usz5F20_CardholderName[26 + 1];
        unsigned char	usz5F24_ExpireDate[4 + 1];
        unsigned char	usz5F2A_TransCurrCode[2 + 1];
	unsigned char	usz5F28_IssuerCountryCode[2 + 1];
        unsigned char	usz5F34_ApplPanSeqnum[2 + 1];
        unsigned char	usz71_IssuerScript1[262 + 1];
        unsigned char	usz72_IssuerScript2[262 + 1];
        unsigned char	usz82_AIP[2 + 1];
        unsigned char	usz84_DF_NAME[16 + 1];
        unsigned char	usz8A_AuthRespCode[2 + 1];
        unsigned char	usz91_IssuerAuthData[16 + 1];
        unsigned char	usz95_TVR[6 + 1];
        unsigned char	usz9A_TranDate[4 + 1];
        unsigned char	usz9B_TSI[2 + 1];
        unsigned char	usz9C_TranType[2 + 1];
        unsigned char	usz9F02_AmtAuthNum[6 + 1];
        unsigned char	usz9F03_AmtOtherNum[6 + 1];
        unsigned char	usz9F08_AppVerNumICC[2 + 1];
        unsigned char	usz9F09_TermVerNum[2 + 1];
        unsigned char	usz9F10_IssuerAppData[32 + 1];
        unsigned char	usz9F18_IssuerScriptID[4 + 1];
        unsigned char	usz9F1A_TermCountryCode[2 + 1];
        unsigned char	usz9F1E_IFDNum[8 + 1];
        unsigned char	usz9F26_ApplCryptogram[8 + 1];
        unsigned char	usz9F27_CID[2 + 1];
        unsigned char	usz9F33_TermCapabilities[4 + 1];
        unsigned char	usz9F34_CVM[4 + 1];
        unsigned char	usz9F35_TermType[2 + 1];
        unsigned char	usz9F36_ATC[2 + 1];
        unsigned char	usz9F37_UnpredictNum[4 + 1];
        unsigned char	usz9F41_TransSeqCounter[4 + 1];
	unsigned char	usz9F42_ApplicationCurrencyCode[2 + 1];
        unsigned char	usz9F5A_Application_Program_Identifier[32 + 1];
        unsigned char	usz9F5B_ISR[6 + 1];
        unsigned char	usz9F60_CouponInformation[30 + 1];
        unsigned char	usz9F63_CardProductLabelInformation[16 + 1];
        unsigned char	usz9F66_Qualifiers[4 + 1];
        unsigned char	usz9F6C_Card_Transaction_Qualifiers[2 + 1];
        unsigned char	usz9F6E_From_Factor_Indicator[32 + 1];
        unsigned char	usz9F74_TLV[6 + 1];
	unsigned char	usz9F7C_PartnerDiscretionaryData[32 + 1];
        unsigned char	uszDF69_NewJspeedyMode[2 + 1];
        unsigned char	uszDF8F4F_TransactionResult[2 + 1];
        unsigned char	uszDFEC_FallBackIndicator[2 + 1];
        unsigned char	uszDFED_ChipConditionCode[2 + 1];
        unsigned char	uszDFEE_TerEntryCap[2 + 1];
        unsigned char	uszDFEF_ReasonOnlineCode[4 + 1];
        
} EMV_REC;

typedef struct TagIPASS_REC
{
        int		inStepNum;			/* 紀錄交易階段 */
        long		lnCardInvNum;			/* Card Transaction Invoice # */
        long		lnUnixTime;			/* GMT Unix Time */
        long		lnSign_Len;			/* 簽章長度 */
	long		lnLastDeductOneDayValue;	/* 最後消費日累計金額 */
	char		szDAVTITxn[280];		/* Advice用，API回傳取得 */
	char		szSign_Data[300];		/* 收送用簽章資料 */
	char		szCardInfo[100];		/* 詢卡結果 */
	char		szLastDeductDate[8 + 1];	/* 最後消費日日期 */
        unsigned char	uszCB_CardBit;			/* 是否為聯名卡 */   
        unsigned char	uszQueryBit;			/* 是否為詢卡 */        
} IPASS_REC;

typedef struct TagECC_REC
{
        long		lnCardInvNum;			/* Card Transaction Invoice # */
        char		szRespCode[2 + 1];		/* Response Code */
        char		szDate[8 + 1];			/* YYYYMMDD */
	char		szTime[6 + 1];			/* HHMMSS */
	char		szICER_RespCode[4 + 1];		/* Response Code T3900 */
	char		szAPI_RespCode[4 + 1];		/* Response Code T3901 */
	char		szEZ_RespCode[4 + 1];		/* Response Code T3902 */
	char		szSVCS_RespCode[6 + 1];		/* Response Code T3903 */
	char		szReader_RespCode[4 + 1];	/* Response Code T3904 */
	char		szPurseVersionNumber[2 + 1];	/* Response Code T4800 */
	char		szCardID[20 + 1];		/* T0200 */
	char		szPurseID[16 + 1];		/* T0211 */
	char		szRRN[15 + 1];
        unsigned char	uszCB_CardBit;		/* 是否為聯名卡 */   
} ECC_REC;

typedef struct TagICASH_REC
{
	int		inStepNum;                              /* 紀錄交易階段 */
        long		lnSign_Len;                             /* 簽章長度 */
	char		szDate[8 + 1];			        /* YYYYMMDD */
	char		szTime[6 + 1];			        /* HHMMSS */
	char		szCardInfo[100];	                /* 詢卡結果 */
	char		szTxLog[400];                           /* Advice用，API回傳取得 */
	char		szTxLog_Other[350];                     /* 一次吐兩筆用的 */
	char		szSign_Data[500];                       /* 收送用簽章資料 */
	unsigned char	uszCB_CardBit;				/* 是否為聯名卡 */
	unsigned char	uszTxLogBit;				/* 兩個txlog */
	unsigned char	uszAPI_FailBit;				/* API寫入失敗 */
} ICASH_REC;

typedef struct TagTicket_BATCH_REC
{
        int		inCode;                                 /* 交易類別 */
	int		inPrintOption;                          /* Print Option Flag (also in TCT) */
        int		inTicketType;				/* 票證種類 - 交易或明細使用 */
	int		inTDTIndex;				/* 存TDT的index */
	long		lnTxnAmount;				/* 交易金額 */
	long		lnTopUpAmount;				/* 加值金額(基底) */
	long		lnTotalTopUpAmount;			/* 加值金額 */
	long		lnCardRemainAmount;			/* 卡片餘額 */
	long		lnInvNum;                               /* For簽單使用，簽單序號 */
	long		lnECCInvNum;                            /* ECC Transaction Invoice # */
	long		lnSTAN;
	long		lnFinalBeforeAmt;                       /* 最後交易結構，交易前卡片餘額 */
	long		lnFinalAfterAmt;                        /* 最後交易結構，交易後卡片餘額 */
	long		lnMainInvNum;                           /* Confirm use 電文序號，因電票中一筆交易可能有數筆電文，所以簽單序號和電文序號分開 */
	long		lnCountInvNum;                          /* Confirm use 若有advice，要預先跳過的電文序號 */
	char		szUID[_UID_SIZE_ + 1];		        /* 卡號 or UID number */
	char		szAPUID[_UID_SIZE_ + 1];		/* 卡號 or UID number(Hex) */
	char		szDate[6 + 1];			        /* YYMMDD */
	char		szOrgDate[6 + 1];		        /* YYMMDD */
	char		szTime[6 + 1];			        /* HHMMSS */
	char		szOrgTime[6 + 1];		        /* HHMMSS */
	char		szAuthCode[_AUTH_CODE_SIZE_ + 1];
	char		szECCAuthCode[_AUTH_CODE_SIZE_ + 1];	
	char		szRespCode[4 + 1];			/* Response Code */
	char		szProductCode[42 + 1];			/* 產品代碼 */
	char		szTicketRefundCode[12 + 1];		/* 退貨序號 */
	char		szTicketRefundDate[4 + 1];		/* MMDD */
	char		szStoreID[50 + 1];
	char		szRefNo[12 + 1];			/* Reference Number(RRN) */
	char		szAwardNum[2 + 1];			/* 優惠個數 */
	char		szAwardSN[22 + 1];			/* 優惠序號(Award S/N) TID(8Bytes)+YYYYMMDDhhmmss(16 Bytes)，共22Bytes */
	unsigned char	uszAutoTopUpBit;			/* 是否自動加值 */ 
	unsigned char	uszBlackListBit;			/* 是否在黑名單中 */     
	unsigned char	uszOfflineBit;				/* 離線交易 */
	unsigned char	uszTicketConnectBit;			/* 是否連線中 */   
	unsigned char	uszResponseBit;				/* 票值回覆用 */          
	unsigned char	uszCloseAutoTopUpBit;			/* 關閉自動加值用 */  
	unsigned char	uszStopPollBit;				/* Mifare Stop */ 
	unsigned char	uszConfirmBit;				/* IPASS Confirm Inv use */
	unsigned char	uszESVCTransBit;			/* 代表是電票交易 */
	unsigned char	uszRewardSuspendBit;			/* 暫停優惠服務(優惠活動為0且要印補充資訊) */
	unsigned char	uszRewardL1Bit;				/* 要印L1 */
	unsigned char	uszRewardL2Bit;				/* 要印L2 */
	unsigned char	uszRewardL5Bit;				/* 要印L5 */
	unsigned char	uszMPASTransBit;			/* 是否為小額交易 */
	unsigned char	uszMPASReprintBit;			/* 是否可重印 */
	IPASS_REC       srIPASSRec;   
	ECC_REC         srECCRec;
	ICASH_REC	srICASHRec;
} TICKET_REC;

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
	char		szEW_PaymentDate[24 + 1];		/* 錢包業者交易日期時間。LinePay：info.transactionDate,悠遊付：orderCreateDateTime,iCashPay：PaymentDate,全盈+PAY：BankOrderDT,全支付：px_trade_time,Pi 錢包：transaction_time*/
	char		szEW_PayMethod[1 + 1];			/* 消費者支付工具，ATS 判斷後回覆給EDC。,LinePay：info.payInfo[].method,悠遊付：paymentMethod,iCashPay：PaymentType,全盈+PAY：PaymentMethod,全支付：pay_tool,Pi 錢包：payment_type */
	char		szEW_TransactionType[2 + 1];		/* 訂單狀態。LinePay：info.transactionType,悠遊付：transactionAction,iCashPay：TradeType,全盈+PAY：OrderStatus,全支付：order_type,Pi 錢包：trans_status,訂單狀態詳下說明三 */
	char		szEW_PaymentNo[30 + 1];			/* 付款交易編號。,悠遊付：paymentNo */
	char		szEW_IsDuplicatedPaymentRequest[1 + 1];	/* 重複付款請求。Y/N,悠遊付：isDuplicatedPaymentRequest */
	char		szEW_CobrandedCode[11 + 1];		/* 聯名卡代碼。,悠遊付：cobrandedCode */
	char		szEW_EinvoiceCarrierType[6 + 1];	/* 載具類型。LinePay：info.merchantReference.affiliateCards[].cardType,悠遊付：einvoiceCarrierType（左靠右補空白）*/
	char		szEW_DiscountAmt[10 + 1];		/* 折抵金額（不含小數位）。,悠遊付：orderAmount 減去paymentAmount,iCashPay：BonusAmt,全盈+PAY：DiscountAmount,全支付：discount_amount,Pi 錢包：psp_bonuspoint_amount（右靠左補0、無資料請全帶0）*//* 退還折抵金額（不含小數位）。iCashPay：RefundBonusAmt,全盈+PAY：ReturnRewardAmount,全支付：discount_amount,Pi 錢包：psp_bonuspoint_amount（右靠左補0、無資料請全帶0）*/
	char		szEW_Store_no[24 + 1];			/* 門市代碼。,Pi 錢包：store_no */
	char		szEW_MaskCreditCardNo[25 + 1];		/* 遮罩帳號/卡號。悠遊付：maskCreditCardNo,iCashPay：MaskedPan,全支付：identity */
	char		szEW_Decimal[2 + 1];			/* 儲存ECR傳來的小數位 */
	long		lnTxnAmount;				/* 交易金額 */
	unsigned char	uszEWTransBit;				/* 是否為電子錢包交易 */
} EW_REC;

typedef struct
{
	int		inCode;						/* */
	long		lnTxnAmount;					/* */
	char		szReconciliationNo[20 + 1];			/* 銷帳編號，若無則全帶空白。左靠右補空白 */
	char		szTrustInstitutionCode[7 + 1];			/* 信託機構代碼。 左靠右補空白 */
	char		szExchangeTxSerialNumber[32 + 1];		/* 交換平台交易序號（平台提供唯一值）左靠右補空白 */
	char		szMaskedBeneficiaryId[10 + 1];			/* 遮掩後的 信託受益人ID */
	char		szTrustRRN[12 + 1];				/* 共用收單序號 */
	unsigned char	uszTrustTransBit;				/* 是否是信託交易 */
} TRUST_REC;

/* 交易pobTran參數，srBRec存交易設定值，srEMVRec存EMV設定值，srTRTRec存TRT設定值 */
typedef struct {
	short shTrack1Len; /* track 1 len */
	short shTrack2Len; /* track 2 len */
	short shTrack3Len; /* track 3 len */
	int inFunctionID; /* 交易的類型 */
	int inRunOperationID; /* 執行 OPT.txt */
	int inRunTRTID; /* 執行 xxxxTRT.txt */
	int inTransactionCode; /* Transaction Code */
	int inTransactionResult;
	int inISOTxnCode; /* ISO code */
	int inCreditCard; /* 判斷是否為信用卡 */
	int inUICCCard; /* 判斷是否為銀聯卡 */
	int inFISCCard; /* 判斷是否為金融卡 */
	int in57_Track2Len; /* track 2 Len */
	int inTMSDwdMode; /* 判斷自動下載或手動下載 */
	int inEFID; /* 金融卡EFID */
	int inMenuKeyin; /* 手動輸入顯示的第一個字 */
	int inHGTransactionCode; /* HappyGo用 */
	int inTableID; /* SQLite使用，在插入資料庫後從資料庫中抓出的PrimaryKey(主要用於Foreign Key) */
	int inEMVResult; /* EMV kernel approve or decline(沒有分first失敗還是second 失敗) 目前沒用這個值判斷 */
	int inCVMResult; /* EMV kernel判斷是否要簽名 目前沒用這個值判斷 */
	int inMultiTransactionCode; /* 只用在當外接設備時，因兩種機型之incode定義不同，所以特別開一個參數來存 */
	int inMultiNormalPayRule; /* 只用在當外接設備時，手續費 for NP */
	int inErrorMsg; /* 錯誤訊息代碼 */
	int inErrorMsgVersion; /* 錯誤訊息代碼版本 */
	int inECRErrorMsgVersion; /* 預設為0，若做不同的銀行ECR版本時，可能會遇上定義的錯誤碼差很遠難以整合，可以用版本區分 */
	int inECRErrorMsg; /* 錯誤訊息代碼 */
	int inEMVDecision; /* 用來分EMV Online or Offline */
	int inESC_Sale_UploadCnt; /* 結帳前儲存用 */
	int inESC_Sale_PaperCnt; /* 結帳前儲存用 */
	int inESC_Refund_UploadCnt; /* 結帳前儲存用 */
	int inESC_Refund_PaperCnt; /* 結帳前儲存用 */
	int inFisc1004Len; /* 只用在外接設備時使用，1004資料長度 */
	int inCTLSLightStatus; /* 用來確認目前感應燈號的狀態，以減少顯示燈號帶來的延遲 */
	int inTicketMsgStatus; /* 用來確認目前顯示的訊息，以減少顯示訊息帶來的延遲 */
	int inMirrorMsgType; /* 收銀機映射訊息代碼 */
	int inTagNYtSAMEncryptLen; /* 用來標示szIDNumber_tSAM加密的長度 */
	long lnCVM_CheckAmt; /* 客製化098，用來檢核是否出簽單 */
	long lnMultiFdtFeeAmt; /* 只用在外接設備時使用，手續費 for NP */
	long lnVoidCheckAmt; /* 客製化098，ECR取消時的金額須比對原交易金額 */
	long lnESC_Sale_UploadAmt; /* 結帳前儲存用 */
	long lnESC_Sale_PaperAmt; /* 結帳前儲存用 */
	long lnESC_Refund_UploadAmt; /* 結帳前儲存用 */
	long lnESC_Refund_PaperAmt; /* 結帳前儲存用 */
	long lnNCCC_SettleTotalCount; /* 信用卡結帳總筆數 *//* 邦柏結帳統計 (START) */ /* 【需求單 - 109306】卡人自助交易支援信用卡取消及退貨交易需求 更新收銀機結帳回覆規格(支援正負號)by Russell 2020/10/15 下午 5:27 */
	long lnECC_SettleTotalCount; /* 悠遊卡結帳總筆數 */
	long lnIPASS_SettleTotalCount; /* 一卡通結帳總筆數 */
	long lnICASH_SettleTotalCount; /* 愛金卡結帳總筆數 */
	long lnNCCC_SettleSaleCount; /* 信用卡一般交易筆數 035客製化使用 */
	long lnNCCC_SettleRefundCount; /* 信用卡退貨交易筆數 035客製化使用 */
	long long llNCCC_SettleTotalAmount; /* 信用卡結帳總金額 */
	long long llECC_SettleTotalAmount; /* 悠遊卡結帳總金額 */
	long long llIPASS_SettleTotalAmount; /* 一卡通結帳總金額 */
	long long llICASH_SettleTotalAmount; /* 愛金卡結帳總金額 */
	long long llNCCC_SettleSaleAmount; /* 信用卡一般交易金額 035客製化使用 */
	long long llNCCC_SettleRefundAmount; /* 信用卡退貨交易金額 035客製化使用 */
	long lnOldTaxAmount; /* ECR小費使用，避免因讀取批次而使得ECR小費被蓋掉 */
	char szTrack1[128 + 1]; /* track 1 buffer */
	char szTrack2[128 + 1]; /* track 2 buffer */
	char szTrack3[128 + 1]; /* track 3 buffer */
	char szT2Data[256 + 1];
	char szCVV2Value[4 + 1];
	char szL3_AwardWay[2 + 1]; /* 兌換方式， ‘1’=於收銀機上輸入條碼‘2’=於端末機上接BarcodeReader‘3’=卡號於端末機上輸入兌換條碼‘4’=卡號於端末機上刷卡‘5’=卡號於端末機上手動輸入 */
	char szL3_Barcode1[20 + 1]; /* 優惠兌換用一維條碼 (一)*/
	char szL3_Barcode1Len[2 + 1]; /* 優惠兌換用一維條碼長度 (一)*/
	char szL3_Barcode2[20 + 1]; /* 優惠兌換用一維條碼 (二)*/
	char szL3_Barcode2Len[2 + 1]; /* 優惠兌換用一維條碼長度 (二)*/
	char szReferralPhoneNum[30 + 1]; /* Call Bank 的電話號碼 */
	char szPIN[16 + 1]; /* Pin data pin size = 16 */
	char szFiscPin[16 + 1]; /* 金融卡Pin */
	char szMAC_HEX[8 + 1]; /* CUP 交易 */
	char szMultiFuncTID[10 + 1]; /* 只用在當外接設備時，因存進HDT太花時間 */
	char szMultiFuncMID[15 + 1]; /* 只用在當外接設備時，因存進HDT太花時間 */
	char szMultiNaitionalPay[2 + 1]; /* 只用在當外接設備時，是否有全國繳 */
	char szMultiEdcFiscIssuerId[8 + 1]; /* 只用在當外接設備時，發卡單位代號 for NP */
	char szMultiEdcFdtFeeFlag[2 + 1]; /* 只用在當外接設備時，手續費開關 for NP */
	char szMultiEdcTccCode[8 + 1]; /* 只用在當外接設備時，Tcc code for NP */
	char szFisc1004Data[256 + 1]; /* 只用在外接設備時使用，1004資料 */
	char szTicket_ErrorCode[10 + 1]; /* 票證使用錯誤碼 */
	char szManagementCode[6 + 1]; /* 下載管理號碼 */
	char szErrorMsgBuff1[50 + 1]; /* 發生錯誤時的自訂訊息1 */
	char szErrorMsgBuff2[50 + 1]; /* 發生錯誤時的自訂訊息2 */
	char szIDNumber[10 + 1]; /* 身分證字號 統一編號 【需求單 - 104014】發卡機構on-us繳費平台需求 */
	char szIDNumber_tSAM[24 + 1]; /* 身分證字號 統一編號 加密後 【需求單 - 104014】發卡機構on-us繳費平台需求 *//* 信託共用加密區塊長度增加至24 */
	char szUniqueNo[10 + 1]; /* 收銀機交易的唯一碼 */
	char szPatientId[10 + 1]; /* 病患/入住者ID(信託使用) */
	char szHostResponseMessageBIG5[20 + 1]; /* 主機回覆中文說明（big5）註1 左靠右補空白 */
	char szHostResponseMessageUTF8[30 + 1]; /* 主機回覆中文說明（UTF8）註1 左靠右補空白 */
	char szTrustQRCodeType[2 + 1]; /* 02靜態 03動態 */
	char szTrustVerificationCode[3 + 1]; /* 信託交易驗證碼 */
	char szUnpredictErrorLogMsg[48 + 1];
	char szEsgSendCR[1 + 1];
	char szAgreeEsgBill[1 + 1]; /* [114197] 中心收單作業ESG永續發展優化 判斷是否同意 持卡人存根聯數位化  同意為Y 不同意為N*/
	char szCHESGUrl[50 + 1];    /* [114197] 中心收單作業ESG永續發展優化 [260610]存放主機回覆的網址前綴 */
    char szCHESGGuid[108 + 1];   /* [114197] 中心收單作業ESG永續發展優化 持卡人同意數位化簽帳單時的簽帳單網址，sys_guid()目前最長可以達 36個字元 */
	unsigned char uszUpdateBatchBit;
	unsigned char uszReversalBit;
	unsigned char uszLastBatchUploadBit; /* 最後一筆 Batch Upload 使用 */
	unsigned char uszConnectionBit; /* 判斷是否已經是連結狀態 */
	unsigned char uszTCUploadBit; /* EMV 0220 上傳使用(標示為當筆TC UPLOAD) */
	unsigned char uszFiscConfirmBit; /* SmartPay 0220 使用 */
	unsigned char uszManualUCardBit; /* 因為U card在manualkeyin 可能會和其他卡的卡號重複 */
	unsigned char uszCreditBit; /* 判斷是否為信用卡 */
	unsigned char uszUICCBit; /* 判斷是否為銀聯卡 */
	unsigned char uszFISCBit; /* 判斷是否為金融卡 */
	unsigned char uszUCardBit; /* 判斷是否自有品牌卡(晶片卡選單使用) */
	unsigned char uszPayWave3Tag55Bit; /* Wave3免簽名條件 */
	unsigned char usz57_Track2[40 + 1]; /* track 2 buffer */
	unsigned char uszFTP_TMS_Download; /* 判斷是否為FTP下載 修改0為不加密 1為FTPS 2為SFTP */
	unsigned char uszMACBit; /* 是否要送 MAC */
	unsigned char uszECRBit; /* 當筆是否ECR發動 */
	unsigned char uszAutoSettleBit; /* 當筆是否連動結帳 */
	unsigned char uszReferralBit; /* 當筆是否CallBank(在當筆送非advice) */
	unsigned char uszQuickPassTag99; /* QuickPass OnlinePIN */
	unsigned char uszDialBackup; /* 撥接備援使用 */
	unsigned char uszEMVProcessDisconectBit; /* 此Bit用以判斷是否等到TC UPload後才斷線 */
	unsigned char uszHGManualBit; /* 表示集點用的HG卡使用人工輸入方式 */
	unsigned char uszMultiFuncSlaveBit; /* 表示是被外接設備 */
	unsigned char uszTwoTapBit; /* 表示是TwoTap流程 */
	unsigned char uszTicketADVOnBit; /* 票證發生票值回覆 */
	unsigned char uszESVCTxnNotEndBit; /* 這個bit On起來代表，票證交易還在進行，不響嗶聲和亮綠燈 */
	unsigned char uszInputCheckNoBit; /* 這個bit On起來代表，有輸入檢查碼，確認卡號時要顯示檢查碼 */
	unsigned char uszInputTxnoBit; /* 這個bit On起來代表，有輸入交易編號 */
	unsigned char uszSelfServiceBit; /* 這個Bit On起來表示是自助式設備，不能隨便讓人按按鍵或取消(先不使用) */
	unsigned char uszSettleLOGONFailedBit; /* 表示結帳完的安全認證失敗，要顯示請先安全認證 */
	unsigned char uszN4TableBit; /* 代表有回傳Table ID "N4", for 修改分期及紅利交易防呆機制 */
	unsigned char uszN5TableBit; /* 代表有回傳Table ID "N5", for 修改分期及紅利交易防呆機制 */
	unsigned char uszCardInquiryFirstBit; /* 詢卡第一次流程 */
	unsigned char uszCardInquirysSecondBit; /* 詢卡第二次流程 */
	unsigned char uszECRPreferCreditBit; /* 只用在當外接設備時，ECR預設COMBO時，信用卡優先 */
	unsigned char uszECRPreferFiscBit; /* 只用在當外接設備時，ECR預設COMBO時，金融卡優先(育德說不會和uszECRPreferCreditBit同時On，湘信他) */
	unsigned char uszTMSDownloadRebootBit; /* 表示TMS下載完要重開機，因為可能下載還有後續要處理，所以用Bit紀錄，直到回Idle前再重開機 */
	unsigned char uszMenuSelectCancelBit; /* 只用在當外接設備時，表示在出感應選單時取消，不在進行感應流程 */
	unsigned char uszSDKTransBit; /* 表示以SDK使用 */
	unsigned char uszDFSNoAuthCodeBit; /* 表示DFS卡未帶授權碼，須reversal */
	unsigned char uszESVCFileNameByCardBit; /* 表示組合票證檔案名稱這時要以卡別組 */
	unsigned char uszESVCLogOnDispBit; /* 表示已顯示過票證LogOn狀態 */
	unsigned char uszESVC_IC_SAM_Unlock_Bit; /* MF iCash init use (是否已做過SAM unlock) */
	unsigned char uszESVC_Get_PIN_Bit; /* MF iCash init use (是否已做過Get PIN) */
	unsigned char uszNCCC_Settle_SuccessBit; /* 判斷NCCC HOST結帳成功 */ /* 邦柏需求 */
	unsigned char uszESVC_Settle_SuccessBit; /* 判斷ESVC HOST結帳成功 */ /* 邦柏需求 */
	unsigned char uszLastTranscationBit; /* 當筆交易由查詢上一筆發動 */
	unsigned char uszFileNameNoNeedHostBit; /* On起來時，組成檔案名不參考Host，而以All代替 */
	unsigned char uszFileNameNoNeedNumBit; /* On起來時，組成檔案名不參考批次號碼 */
	unsigned char uszEnterDownPaymentBit; /* On起來時，代表首期金額已輸入過 */
	unsigned char uszEnterInstFeeBit; /* On起來時，代表分期手續費已輸入過 */
	unsigned char uszEnterRDMPaidAmtBit; /* On起來時，代表支付金額已輸入過 */
	unsigned char uszDeductSuccessBit; /* 成功購貨 */
	unsigned char uszAutoTopUpSuccessBit; /* 成功自動加值 */
	unsigned char uszPrePrintBit; /* On起來時，表示目前在預先印結帳簽單前的報表 */
	unsigned char uszRemoveChecECRkBit; /* On起來時，只接收取消與查詢最後一筆，並儲存內容 */
	unsigned char uszECCRetryBit; /* On起來時，客製化098會儲存0018 */
	unsigned char uszTK3C_NoHotkeyBit; /* On起來時，會遮掩電票交易，因為ECR規格未定 */
	unsigned char uszDelaySendBit; /* On起來時，列印完簽單才會回傳ECR結果(有用到此Bit要注意ECR是否有正確回傳，有可能會被此Bit跳過流程) */
	unsigned char uszIsTradeBit; /* On起來時，表示還在交易中，只接受EI */
	unsigned char uszEverRich_Settle_RepeintBit; /* 用來標示現在要存重印的結帳報表，結帳報表的交易類別要印"(重印)" */
	unsigned char uszCheck_CoBrand_CardBit; /* 用來判斷要不要進入判斷聯名卡流程 */
	unsigned char uszCheck_OPT_Special_CardBit; /* 用來判斷是否OPT跑過確認特殊卡，避免跑兩次浪費時間 */
	unsigned char uszEverRich_NoDataBit; /* 沒有初始資料，但仍要印出空白結帳單用 */
	unsigned char uszCus099_Already_Decide_Reversal_Bit; /* 客製化099，代表已選擇過reversal，避免再選一次 */
	unsigned char uszCus096_iPASS_Direct_Bit; /* 客製化096，代表進入當作ipass讀卡機模式 */
	unsigned char uszNotDispMsgBit; /* 不更改螢幕上顯示，目前客製化042使用 */
	unsigned char uszCardInquirysThirdBit; /* 客製化005，可能有三段式交易 */
	unsigned char uszUpdateBatchNumBit; /* 用來紀錄以調閱編號超過999觸發批號加一的狀況 */
	unsigned char uszTagNYtSAMEncryptBit; /* 用來標示szIDNumber_tSAM是否tSam加密 */
	unsigned char uszEsgOptrionBit; /* [114197] 中心收單作業ESG永續發展優化 新增固定方法不受其它參數更改 */
	BATCH_REC srBRec;
	EMV_REC srEMVRec;
	TICKET_REC srTRec;
	EW_REC srEWRec;
	TRUST_REC srTrustRec;
} TRANSACTION_OBJECT;

/* TRANS_BATCH_KEY存batch資料 */
typedef struct TagTRANS_BATCH_KEY
{
        long    lnSearchIndex;
        long    lnInvoiceNo;            /* 原交易的 Invoice */
        long    lnBatchRecStart;        /* 要搜尋 BATCH_REC 的啟始位置 */
        long    lnBatchRecSize;         /* 要存的檔案大小 */
        long    lnAdviceOrgSTAN;        /* Advice交易的【STAN】 */
} TRANS_BATCH_KEY;

typedef int (*FUNC_POINT)(TRANSACTION_OBJECT *);